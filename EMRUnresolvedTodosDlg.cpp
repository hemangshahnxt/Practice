// EMRUnresolvedTodosDlg.cpp : implementation file
//
// (c.haag 2008-07-10 09:23) - PLID 30648 - Initial implementation
// (c.haag 2008-07-14 15:49) - PLID 30696 - Changed scope from only spawned/unspawned
// todo's to manually created/deleted todos

#include "stdafx.h"
#include "patientsrc.h"
#include "EMRUnresolvedTodosDlg.h"
#include "TodoUtils.h"
#include "GlobalAuditUtils.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "EMNTodo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define COLUMN_TASK_ID			0L
#define COLUMN_TASK_TYPE		1L
#define COLUMN_TASK				2L
#define COLUMN_PRIORITY			3L
#define COLUMN_NOTES			4L
#define COLUMN_ASSIGN_TO_NAMES	5L
#define COLUMN_ASSIGN_TO_IDS	6L
#define COLUMN_COMPLETE_BY		7L
#define COLUMN_OPTION			8L
#define COLUMN_REGARDING_TYPE	9L
#define COLUMN_REGARDING_ID		10L
#define COLUMN_REMIND			11L
#define COLUMN_PERSON_ID		12L
#define COLUMN_LOCATION_ID		13L
#define COLUMN_DONE				14L
#define COLUMN_ENTERED_BY		15L
#define COLUMN_SOURCE_ACTION_ID	16L

/////////////////////////////////////////////////////////////////////////////
// CEMRUnresolvedTodosDlg dialog


CEMRUnresolvedTodosDlg::CEMRUnresolvedTodosDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRUnresolvedTodosDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRUnresolvedTodosDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// (c.haag 2008-07-10 10:22) - Adds a row to either list
void CEMRUnresolvedTodosDlg::AddTodoRow(NXDATALIST2Lib::_DNxDataListPtr& pList, EMNTodo* pTodo)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->GetNewRow();
	CString strIDs;
	pRow->Value[COLUMN_TASK_ID] = pTodo->nTodoID;
	pRow->Value[COLUMN_TASK_TYPE] = pTodo->vCategoryID;
	pRow->Value[COLUMN_TASK] = pTodo->vTask;
	pRow->Value[COLUMN_PRIORITY] = pTodo->vPriority;
	pRow->Value[COLUMN_NOTES] = pTodo->vNotes;
	pRow->Value[COLUMN_ASSIGN_TO_NAMES] = pTodo->vAssignToNames;
	for (int j=0; j < pTodo->anAssignTo.GetSize(); j++) {
		strIDs += FormatString("%d ", pTodo->anAssignTo[j]);
	}
	strIDs.TrimRight(" ");
	pRow->Value[COLUMN_ASSIGN_TO_IDS] = _bstr_t(strIDs);
	pRow->Value[COLUMN_COMPLETE_BY] = pTodo->vDeadline;
	pRow->Value[COLUMN_OPTION] = _variant_t(VARIANT_TRUE, VT_BOOL);
	pRow->Value[COLUMN_REGARDING_TYPE] = pTodo->vRegardingType;
	pRow->Value[COLUMN_REGARDING_ID] = pTodo->vRegardingID;
	pRow->Value[COLUMN_REMIND] = pTodo->vRemind;
	pRow->Value[COLUMN_PERSON_ID] = pTodo->vPersonID;
	pRow->Value[COLUMN_LOCATION_ID] = pTodo->vLocationID;
	pRow->Value[COLUMN_DONE] = pTodo->vDone;
	pRow->Value[COLUMN_ENTERED_BY] = pTodo->vEnteredBy;
	// (z.manning 2009-02-26 16:06) - PLID 33141 - Use the new source action info class
	pRow->Value[COLUMN_SOURCE_ACTION_ID] = pTodo->sai.nSourceActionID;
	pList->AddRowSorted(pRow, NULL);
}

void CEMRUnresolvedTodosDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRUnresolvedTodosDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRUnresolvedTodosDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRUnresolvedTodosDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRUnresolvedTodosDlg message handlers

BOOL CEMRUnresolvedTodosDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
	
		// (c.haag 2008-07-11 15:55) - Using the OK style. If the use cancels out, then
		// the EMN will not be cancelled.
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (c.haag 2008-07-10 09:31) - Bind the datalists
		m_dlCreatedTodos = BindNxDataList2Ctrl(IDC_LIST_UNRESOLVED_CREATED_TODO_ALARMS, false);
		m_dlDeletedTodos = BindNxDataList2Ctrl(IDC_LIST_UNRESOLVED_DELETED_TODO_ALARMS, false);

		// (c.haag 2008-07-10 10:11) - Requery embedded combos
		CString strCategoryCombo = "SELECT -1 AS ID, '' AS Text UNION ALL SELECT ID, Description AS Text FROM NoteCatsF ORDER BY Text";
		CString strMethodCombo = "SELECT 'Phone', 'Phone' UNION SELECT 'E-Mail', 'E-Mail' UNION SELECT 'Fax', 'Fax' UNION SELECT 'Letter', 'Letter' UNION SELECT 'Other', 'Other'";
		m_dlDeletedTodos->GetColumn(COLUMN_TASK_TYPE)->ComboSource = _bstr_t(strCategoryCombo);
		m_dlDeletedTodos->GetColumn(COLUMN_TASK)->ComboSource = _bstr_t(strMethodCombo);

		// (c.haag 2008-07-10 09:32) - Add the spawned todos
		// (c.haag 2008-07-14 12:11) - PLID 30696 - Include both spawned and manually created
		// todos (from the "New ToDo" button)
		CString strWhere;
		int i;
		if (m_apCreatedEMNTodos.GetSize() > 0) {
			CArray<long,long> anTodoIDs;
			for (i=0; i < m_apCreatedEMNTodos.GetSize(); i++) {
				EMNTodo* pTodo = m_apCreatedEMNTodos[i];
				anTodoIDs.Add(pTodo->nTodoID);
			}
			strWhere.Format("TodoList.TaskID IN (%s)", ArrayAsString(anTodoIDs));
			m_dlCreatedTodos->WhereClause = _bstr_t(strWhere);
			m_dlCreatedTodos->Requery();
			// (c.haag 2008-07-14 17:44) - The todos must be visible before the user can start to make a decision
			m_dlCreatedTodos->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		// (c.haag 2008-07-10 09:50) - Add the unspawned todos
		// (c.haag 2008-07-14 12:11) - PLID 30696 - Include both unspawned and manually deleted
		// todos (from the More Info topic)
		for (i=0; i < m_apDeletedEMNTodos.GetSize(); i++) {
			EMNTodo* pTodo = m_apDeletedEMNTodos[i];
			AddTodoRow(m_dlDeletedTodos, pTodo);
		}
	}
	NxCatchAll("Error in CEMRUnresolvedTodosDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRUnresolvedTodosDlg::OnOK()
{
	try {
		// (c.haag 2008-07-10 09:46) - Go through the created list, look for everything that
		// is flagged to be deleted, and delete the todos
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlCreatedTodos->GetFirstRow();
		while (NULL != pRow) {
			const long nTaskID = VarLong(pRow->Value[COLUMN_TASK_ID]);
			const BOOL bDelete = VarBool(pRow->Value[COLUMN_OPTION]);
			if (bDelete) {
				TodoDelete(nTaskID, TRUE);
				// Audit the change
				CString strAssignTo = VarString(pRow->Value[COLUMN_ASSIGN_TO_NAMES]);
				_variant_t vCategory = pRow->GetOutputValue(COLUMN_TASK_TYPE);
				if (VT_EMPTY == vCategory.vt) {
					vCategory = "";
				}
				CString strCategory = VarString(vCategory, "");
				CString strNotes = VarString(pRow->Value[COLUMN_NOTES]);
				long nAuditID = BeginNewAuditEvent();
				// CR and LF's don't show up in the audit trail elegantly; replace them with spaces or else the note will look ugly
				strNotes.Replace(10, ' ');
				strNotes.Replace(13, ' ');
				CString strOld = FormatString("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", 
					strAssignTo, FormatDateTimeForInterface(VarDateTime(pRow->Value[COLUMN_COMPLETE_BY]), 0, dtoDate), strCategory, strNotes);
				AuditEvent(VarLong(pRow->Value[COLUMN_PERSON_ID]), GetExistingPatientName(VarLong(pRow->Value[COLUMN_PERSON_ID])), nAuditID, aeiPatientTodoTaskDelete, nTaskID, strOld, "<Deleted>", aepMedium, aetDeleted);
				// Fire a table checker
				CArray<long, long> anAssignTo;
				ParseDelimitedStringToLongArray(VarString(pRow->Value[COLUMN_ASSIGN_TO_IDS]), " ", anAssignTo);

				// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo

				if (anAssignTo.GetSize() == 1){
					CClient::RefreshTodoTable(nTaskID, VarLong(pRow->Value[COLUMN_PERSON_ID],-1), anAssignTo[0], TableCheckerDetailIndex::tddisDeleted);
				}
				else{
					CClient::RefreshTodoTable(nTaskID, VarLong(pRow->Value[COLUMN_PERSON_ID], -1), -1, TableCheckerDetailIndex::tddisDeleted);
				}
			}
			pRow = pRow->GetNextRow();
		}

		// (c.haag 2008-07-10 09:49) - Go through the deleted list, look for everything that
		// is flagged to be restored, and restore the todos
		pRow = m_dlDeletedTodos->GetFirstRow();
		while (NULL != pRow) {
			const BOOL bCreate = VarBool(pRow->Value[COLUMN_OPTION]);
			if (bCreate) {
				CArray<long,long> anAssignTo;
				ParseDelimitedStringToLongArray(VarString(pRow->Value[COLUMN_ASSIGN_TO_IDS]), " ", anAssignTo);
				// (c.haag 2008-07-14 16:07) - PLID 30696 - This should never be null or -1
				if (-1 == VarLong(pRow->Value[COLUMN_REGARDING_TYPE])) {
					ASSERT(FALSE);
					ThrowNxException("Attempted to restore a todo alarm that could not be linked to an EMR entity.");
				}
				long nTaskID = TodoCreate(VarDateTime(pRow->Value[COLUMN_REMIND]),
					VarDateTime(pRow->Value[COLUMN_COMPLETE_BY]),
					anAssignTo,
					VarString(pRow->Value[COLUMN_NOTES]),
					VarString(pRow->Value[COLUMN_TASK]),					
					VarLong(pRow->Value[COLUMN_REGARDING_ID]),
					(TodoType)VarLong(pRow->Value[COLUMN_REGARDING_TYPE]),
					VarLong(pRow->Value[COLUMN_PERSON_ID]), // This should *never* be null
					VarLong(pRow->Value[COLUMN_LOCATION_ID], -1),
					(TodoPriority)VarByte(pRow->Value[COLUMN_PRIORITY]),
					VarLong(pRow->Value[COLUMN_TASK_TYPE], -1),
					VarDateTime(pRow->Value[COLUMN_DONE], (DATE)0),
					VarLong(pRow->Value[COLUMN_ENTERED_BY]),
					VarLong(pRow->Value[COLUMN_SOURCE_ACTION_ID])
					);

				// Audit the change
				CString strAssignTo = VarString(pRow->Value[COLUMN_ASSIGN_TO_NAMES]);
				_variant_t vCategory = pRow->GetOutputValue(COLUMN_TASK_TYPE);
				if (VT_EMPTY == vCategory.vt) {
					vCategory = "";
				}
				CString strNotes = VarString(pRow->Value[COLUMN_NOTES]);
				long nAuditID = BeginNewAuditEvent();
				// CR and LF's don't show up in the audit trail elegantly; replace them with spaces or else the note will look ugly
				strNotes.Replace(10, ' ');
				strNotes.Replace(13, ' ');
				CString strNew = FormatString("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", 
					strAssignTo, FormatDateTimeForInterface(VarDateTime(pRow->Value[COLUMN_COMPLETE_BY]), 0, dtoDate), VarString(vCategory,""), strNotes);
				AuditEvent(VarLong(pRow->Value[COLUMN_PERSON_ID]), GetExistingPatientName(VarLong(pRow->Value[COLUMN_PERSON_ID])), nAuditID, aeiPatientToDoCreated, nTaskID, "", strNew, aepMedium, aetCreated);
				// Fire a table checker
				// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
				if (anAssignTo.GetSize() == 1){
					CClient::RefreshTodoTable(nTaskID, VarLong(pRow->Value[COLUMN_PERSON_ID], -1), anAssignTo[0], TableCheckerDetailIndex::tddisAdded);
				}
				else{
					CClient::RefreshTodoTable(nTaskID, VarLong(pRow->Value[COLUMN_PERSON_ID], -1), -1, TableCheckerDetailIndex::tddisAdded);
				}
			}
			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}
	NxCatchAll("Error in CEMRUnresolvedTodosDlg::OnOK");
}
