// FollowUpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "FollowUpDlg.h"
#include "NxTabView.h"
#include "GlobalUtils.h"
#include "ToDoAlarmDlg.h"
#include "MainFrm.h"
#include "TaskEditDlg.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "NoteCategories.h"
#include "AuditTrail.h"
#include "PhaseTracking.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "TodoUtils.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (c.haag 2008-06-10 10:09) - PLID 11599 - Split out assign-to columns now that we support
// having multiple assign-to's per task
#define COLUMN_TASK_ID			0
#define COLUMN_TASK_TYPE		1
#define COLUMN_TASK				2
#define COLUMN_PRIORITY			3
#define COLUMN_NOTES			4
#define COLUMN_ASSIGN_TO_NAMES	5
#define COLUMN_ASSIGN_TO_IDS	6
#define COLUMN_COMPLETE_BY		7
#define COLUMN_COMPLETE_DATE	8
#define COLUMN_ENTERED_BY		9


// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDADD         52345
#define IDMODIFY      52346
#define IDDELETE      52347
#define IDCOMPLETED   52348
#define IDUNCOMPLETED 52349

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CFollowUpDlg dialog
CFollowUpDlg::CFollowUpDlg(CWnd* pParent)
	: CPatientDialog(CFollowUpDlg::IDD, pParent), m_tblCheckTask(NetUtils::TodoList)
{
	//{{AFX_DATA_INIT(CFollowUpDlg)
		// (a.walling 2010-10-13 10:37) - PLID 40977 - Dead code, m_IsEditing is never set anywhere, nor m_IsAdding, nor m_boAllowUpdate
		//m_IsEditing = FALSE;
		//m_IsAdding = FALSE;
		//m_boAllowUpdate = TRUE;
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/Follow_Up/create_a_to-do_reminder.htm";

	// (a.walling 2010-10-13 10:35) - PLID 40977
	m_id = -1;
}

void CFollowUpDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFollowUpDlg)
	DDX_Control(pDX, IDC_FOLLOWUP_SHOW_GRID, m_btnShowGrid);
	DDX_Control(pDX, IDC_EDIT_CATEGORIES, m_editCategoriesButton);
	DDX_Control(pDX, IDC_DELETE_COMPLETED, m_deleteCompletedButton);
	DDX_Control(pDX, IDC_CREATE_TODO, m_createTodoButton);
	DDX_Control(pDX, IDC_RADIO_ALL, m_all);
	DDX_Control(pDX, IDC_RADIO_INCOMPLETE, m_incomplete);
	DDX_Control(pDX, IDC_RADIO_COMPLETED, m_completed);
	DDX_Control(pDX, IDC_BKG, m_bkg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFollowUpDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFollowUpDlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_RADIO_ALL, OnRadioAll)
	ON_BN_CLICKED(IDC_RADIO_COMPLETED, OnRadioCompleted)
	ON_BN_CLICKED(IDC_RADIO_INCOMPLETE, OnRadioIncomplete)
	ON_BN_CLICKED(IDC_CREATE_TODO, OnCreateTodo)
	ON_BN_CLICKED(IDC_TODO_LIST, OnTodoList)
	ON_BN_CLICKED(IDC_DELETE_COMPLETED, OnDeleteCompleted)
	ON_BN_CLICKED(IDC_EDIT_CATEGORIES, OnEditCategories)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)// (s.tullis 2014-08-26 09:27) - PLID 63226 
	ON_BN_CLICKED(IDC_FOLLOWUP_SHOW_GRID, OnFollowupShowGrid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CFollowUpDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFollowUpDlg)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 7 /* RButtonUp */, OnRButtonUpListTodo, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 4 /* LButtonDown */, OnLButtonDownListTodo, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 10 /* EditingFinished */, OnEditingFinishedListTodo, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 9 /* EditingFinishing */, OnEditingFinishingListTodo, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 18 /* RequeryFinished */, OnRequeryFinishedListTodo, VTS_I2)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 8 /* EditingStarting */, OnEditingStartingListTodo, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CFollowUpDlg, IDC_LIST_TODO, 3 /* DblClickCell */, OnDblClickCellListTodo, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CFollowUpDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	//(e.lally 2009-11-16) PLID 36304 - added try/catch
	try{
		// (a.walling 2010-10-13 10:37) - PLID 40977 - Dead code, m_IsEditing is never set anywhere, nor m_IsAdding, nor m_boAllowUpdate
		//if (!m_boAllowUpdate) return;

		// (a.walling 2010-10-13 10:36) - PLID 40977
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();

		{
			m_colorComplete = (COLORREF)GetRemotePropertyInt("ToDoColorComplete", RGB(210, 255, 210), 0, GetCurrentUserName(), true);
			m_colorIncompleteHigh = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteHigh", RGB(240, 200, 200), 0, GetCurrentUserName(), true);
			m_colorIncompleteMedium = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteMedium", RGB(240, 210, 210), 0, GetCurrentUserName(), true);
			m_colorIncompleteLow = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteLow", RGB(240, 220, 220), 0, GetCurrentUserName(), true);
		}

		// If the user adds a new item, it will make combos pop
		// from left to right. We cannot requery because it will try
		// to sort the new record, which should always be at the bottom.
		// (a.walling 2010-10-13 10:37) - PLID 40977 - Dead code, m_IsEditing is never set anywhere.
		/*
		if (m_IsEditing)
			m_List->Requery();
		else
			Requery();
		*/
		
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (m_tblCheckTask.Changed()) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			Requery();
		}
		m_ForceRefresh = false;
	}NxCatchAll(__FUNCTION__);
}

void CFollowUpDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
	CPatientDialog::SetColor(nNewColor);
}

void CFollowUpDlg::Requery()
{
	CWaitCursor pWait;

	try {
		// Force SQL to reflect record of only one patient where
		// tasks have yet to be completed
		CString strSql, strWhere;
		if (m_incomplete.GetCheck()) 
		{	
			strWhere.Format("ToDoListQ.Done Is NULL AND ToDoListQ.PersonID = %li",m_id);
		}
		else 
		{	if (m_completed.GetCheck()) 
			{	strWhere.Format("ToDoListQ.Done Is Not NULL AND ToDoListQ.PersonID = %li",m_id);
			}
			else 
				strWhere.Format("ToDoListQ.PersonID = %li",m_id);
		}

		//CAH 5/5/2003 - Modify the query to reflect the ability to read todos
		// (c.haag 2008-07-03 12:25) - PLID 30615 - Use new TodoAssignTo table structure. Do it in such a way that
		// if you don't have access to see the todo's of person X, then you filter out todo's that
		// are assigned to person X and not to you. If they're assigned to both, you can still see yours.
		if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)))
		{
			CString str;
			str.Format(" AND (TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> %d))", GetCurrentUserID());
			strWhere += str;
		}
		if (!(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT__R_________ANDPASS)))
		{
			CString str;
			str.Format(" AND (TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = %d))", GetCurrentUserID());
			strWhere += str;
		}

		m_List->WhereClause = _bstr_t(strWhere);
		//when you switch the where clause, you requery the data
		m_List->Requery();
		m_ForceRefresh = false;
	}NxCatchAll("Error in FollowUpDlg::Requery");
}

void CFollowUpDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	//(e.lally 2009-11-16) PLID 36304 - added try/catch
	try{
		CNxDialog::OnShowWindow(bShow, nStatus);

		m_createTodoButton.SetFocus();
	}NxCatchAll(__FUNCTION__);
}

BOOL CFollowUpDlg::OnInitDialog() 
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch
	try{
		CNxDialog::OnInitDialog();

		m_editCategoriesButton.AutoSet(NXB_MODIFY);
		m_deleteCompletedButton.AutoSet(NXB_DELETE);
		m_createTodoButton.AutoSet(NXB_NEW);

		m_List = BindNxDataListCtrl(IDC_LIST_TODO,false);

		// (a.walling 2010-10-13 10:35) - PLID 40977 - Don't load the id until UpdateView is called
		/*
		m_id = GetActivePatientID();
		*/

		// Check radio button
		m_incomplete.SetCheck(TRUE);
		m_completed.SetCheck(FALSE);
		m_all.SetCheck(FALSE);

		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to create or edit a todo alarm
		/*
		if (!(GetCurrentUserPermissions(bioPatient) & (SPT___W________ANDPASS)))
		{
			GetDlgItem(IDC_CREATE_TODO)->EnableWindow(FALSE);
			GetDlgItem(IDC_DELETE_COMPLETED)->EnableWindow(FALSE);
			GetDlgItem(IDC_LIST_TODO)->EnableWindow(FALSE);
		}
		*/

		if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT____C_______ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT____C_______ANDPASS))){
			GetDlgItem(IDC_CREATE_TODO)->EnableWindow(FALSE);
		}
		if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT_____D______ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT_____D______ANDPASS))){
			GetDlgItem(IDC_DELETE_COMPLETED)->EnableWindow(FALSE);
		}
		if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT__R_________ANDPASS))){
			GetDlgItem(IDC_LIST_TODO)->EnableWindow(FALSE);
			GetDlgItem(IDC_TODO_LIST)->EnableWindow(FALSE);
		}

		if(GetRemotePropertyInt("FollowupShowGrid", 0, 0, GetCurrentUserName(), true)) {
			CheckDlgButton(IDC_FOLLOWUP_SHOW_GRID, BST_CHECKED);
			m_List->GridVisible = true;
		}
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CFollowUpDlg::OnRButtonUpListTodo(long nRow, short nCol, long x, long y, long nFlags) 
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch
	try{
		m_List->CurSel = nRow;
		m_nRowSel = nRow;

		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to edit a todo alarm
		/*
		if (!(GetCurrentUserPermissions(bioPatient) & (SPT___W________ANDPASS)))
			return;
		*/
		
		// Build a popup menu to change task characteristics
		CMenu Popup;
		Popup.m_hMenu = CreatePopupMenu();
		Popup.InsertMenu(1, MF_BYPOSITION, IDADD, "Add Task");
		if(nRow!=-1) {
			Popup.InsertMenu(2, MF_BYPOSITION, IDMODIFY, "Modify Task");
			Popup.InsertMenu(3, MF_BYPOSITION, IDDELETE, "Delete Task");

			// Allow the user to change task completion status
			_variant_t varCompleteDate = m_List->GetValue(nRow, COLUMN_COMPLETE_DATE);

			if (varCompleteDate.vt == VT_NULL) {
				Popup.InsertMenu(4, MF_BYPOSITION, IDCOMPLETED, "Mark As Completed");
			}
			else {
				Popup.InsertMenu(4, MF_BYPOSITION, IDUNCOMPLETED, "Mark As Incomplete");
			}
		}
		

		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* pwnd = GetDlgItem(IDC_LIST_TODO);
		if (pwnd != NULL) {
			pwnd->ClientToScreen(&pt);
			Popup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2008-06-10 10:49) - PLID 11599 - Populates a row in the list with data values
void CFollowUpDlg::PopulateListRow(long nRow, FieldsPtr& f)
{
	IRowSettingsPtr pRow = m_List->GetRow(nRow);
	PopulateListRow(pRow, f);
}

// (c.haag 2008-06-10 10:49) - PLID 11599 - Populates a row in the list with data values
void CFollowUpDlg::PopulateListRow(IRowSettingsPtr& pRow, FieldsPtr& f)
{
	if (NULL != pRow) {
		pRow->PutValue(COLUMN_TASK_TYPE, f->Item["CategoryID"]->Value);
		pRow->PutValue(COLUMN_TASK, f->Item["Task"]->Value);
		pRow->PutValue(COLUMN_PRIORITY, f->Item["Priority"]->Value);
		pRow->PutValue(COLUMN_NOTES, f->Item["Notes"]->Value);
		pRow->PutValue(COLUMN_ASSIGN_TO_NAMES, f->Item["AssignedToNames"]->Value);
		pRow->PutValue(COLUMN_ASSIGN_TO_IDS, f->Item["AssignedToIDs"]->Value);
		pRow->PutValue(COLUMN_COMPLETE_BY, f->Item["DeadLine"]->Value);
		_variant_t varFinishDate = f->Item["Done"]->Value;
		if (varFinishDate.vt == VT_DATE) {
			COleDateTime dtDate = VarDateTime(varFinishDate);
			CString strDate = FormatDateTimeForInterface(dtDate, DTF_STRIP_SECONDS, dtoDate);
			pRow->PutValue(COLUMN_COMPLETE_DATE, _variant_t(strDate));
		}
		else {
			//thats ok
			pRow->PutValue(COLUMN_COMPLETE_DATE, varFinishDate);
		}
		pRow->PutValue(COLUMN_ENTERED_BY, f->Item["EnteredByName"]->Value);

	} else {
		ASSERT(FALSE);
	}
}

BOOL CFollowUpDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {
		case IDADD:
			//(e.lally 2009-11-16) PLID 36304 - Added try/catch and new CreateTodo function
			try{
				if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT____C_______ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT____C_______ANDPASS))){
					MsgBox("You do not have permission to create Follow-Ups");
				}
				else {
					CreateTodo();
				}
			}NxCatchAll(" Error in CFollowUpDlg::OnCommand - Cannot Modify Task");
		break;
	
		case IDMODIFY:
			//(e.lally 2009-11-16) PLID 36304 - Added try/catch and new ModifyItem function
			try{
				ModifyItem();
			}NxCatchAll(" Error in CFollowUpDlg::OnCommand - Cannot Modify Task");
		break;

		case IDDELETE:
			// make sure the user has permission to delete
			//(e.lally 2009-11-16) PLID 36304 - Combined to one try/catch, if the permission check failed, we can't still allow the delete
			try {
				// (c.haag 2008-07-03 12:25) - PLID 30615 - Use the new member function to do permission checks
				CArray<long,long> arynAssignTo;
				GetAssignToArrayByRow(m_List->GetCurSel(), arynAssignTo);
				if (!CheckAssignToPermissions(arynAssignTo, sptDelete)) {
					break;
				}

				if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to delete this task?")) {
					long nCurSel = m_List->GetCurSel();
					if(nCurSel == -1)
						//Shouldn't be possible
						return TRUE;

					long nTaskID = VarLong(m_List->GetValue(nCurSel,COLUMN_TASK_ID));
					CString strOld = VarString(m_List->GetValue(nCurSel, COLUMN_NOTES), "");
					// (j.gruber 2008-04-02 11:04) - PLID 29296 - update clientimplemtnationstepsT
					if (IsNexTechInternal()) {
						ExecuteSql("UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL WHERE ToDoTaskID = %li", nTaskID);	
					}
					// (a.walling 2008-06-25 17:47) - PLID 30515 - This should not affect any actual EMN records, since this appears to
					// be old code for Custom Records only
					ExecuteSql("UPDATE EMRMasterT SET ProcedureTaskID = NULL WHERE ProcedureTaskID = %d;UPDATE EMRMasterT SET FollowUpTaskID = NULL WHERE FollowUpTaskID = %d", nTaskID, nTaskID);
					// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
					TodoDelete(nTaskID);
					//auditing
					CString strAssignTo(""), strCategory("");
					// (c.haag 2008-06-10 10:21) - PLID 11599 - We can now just get the assign to names from the list
					// (a.walling 2011-08-10 11:25) - PLID 44739 - This might be NULL
					strAssignTo = VarString(m_List->GetValue(nCurSel,COLUMN_ASSIGN_TO_NAMES), "");

					_RecordsetPtr prsCategory = CreateRecordset("SELECT ID, Description AS Text "
						"FROM NoteCatsF WHERE ID = %li ORDER BY Text", VarLong(m_List->GetValue(nCurSel,COLUMN_TASK_TYPE),-1));
					if (!prsCategory->eof)
						strCategory = AdoFldString(prsCategory, "Text", "");
					CString strTemp;
					strTemp.Format("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", strAssignTo, FormatDateTimeForInterface(m_List->GetValue(nCurSel, COLUMN_COMPLETE_BY), 0, dtoDate), strCategory, strOld);
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1)
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientTodoTaskDelete, GetActivePatientID(), strTemp, "<Deleted>", aepMedium, aetDeleted);
					
					
					

					

					// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
					if (arynAssignTo.GetSize() == 1){
						CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), arynAssignTo[0], TableCheckerDetailIndex::tddisDeleted);
					}
					else{
						CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisDeleted);
					}
					m_List->RemoveRow(nCurSel);
					// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
					/*
					CMainFrame *pMain = GetMainFrame();
					if(pMain) {
						if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
							pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, nTaskID);
						}
					}
					*/
				}

			}NxCatchAll(" Error in CFollowUpDlg::OnCommand \n Cannot Delete Task");
				
		break;

		case IDUNCOMPLETED: {
			//(e.lally 2009-11-16) PLID 36304 - Moved try up to top
			try{
				// (c.haag 2008-07-03 12:25) - PLID 30615 - Use the new member function to do permission checks
				CArray<long,long> arynAssignTo;
				GetAssignToArrayByRow(m_List->GetCurSel(), arynAssignTo);
				if (!CheckAssignToPermissions(arynAssignTo, sptWrite)) {
					break;
				}

				long nCurSel;
				nCurSel = m_List->GetCurSel();
				// (a.walling 2006-10-31 16:53) - PLID 23299 - The VarLong below is just waiting for a problem.
				if (nCurSel == -1) break;
				long nTaskID = VarLong(m_List->GetValue(nCurSel,COLUMN_TASK_ID));
				ExecuteSql("UPDATE ToDoList SET ToDoList.Done = NULL WHERE TaskID = %li", nTaskID);

				// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
				if (arynAssignTo.GetSize() == 1){
					CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), arynAssignTo[0], TableCheckerDetailIndex::tddisChanged);
				}
				else{
					CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisChanged);
				}
				//if the incompleted radio button is selected then by marking this task completed, we need to remove
				//the item from this list
				if (m_completed.GetCheck()) {
					m_List->RemoveRow(nCurSel);
				}
				else if (m_all.GetCheck()) {
					_variant_t var;
					var.vt = VT_NULL;	// the list checks to see if this is null when it decides whether to prompt you to mark complete or incomplete
					m_List->PutValue(nCurSel, COLUMN_COMPLETE_DATE, var);
				}

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientTodoTaskUncomplete, GetActivePatientID(), "Completed", "Uncompleted", aepMedium, aetChanged);

				//update the todoalarm
				PhaseTracking::SyncLadderWithTodo(nTaskID);
			

				
				

				// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
				/*
				CMainFrame *pMain = GetMainFrame();
				if(pMain) {
					if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
						pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, nTaskID);
					}
				}
				*/
			
			}NxCatchAll("Error in CFollowUpDlg::OnCommand \n Cannot Change Completion Status");
		}			
		break;
		
		case IDCOMPLETED: {
			//(e.lally 2009-11-16) PLID 36304 - Moved try up to top
			try{
				// (c.haag 2008-07-03 12:25) - PLID 30615 - Use the new member function to do permission checks
				CArray<long,long> arynAssignTo;
				GetAssignToArrayByRow(m_List->GetCurSel(), arynAssignTo);
				if (!CheckAssignToPermissions(arynAssignTo, sptWrite)) {
					break;
				}
				long nCurSel;
				nCurSel = m_List->GetCurSel();
				long nTaskID = VarLong(m_List->GetValue(nCurSel,COLUMN_TASK_ID));
				ExecuteSql("UPDATE ToDoList SET Done = GetDate() WHERE TaskID = %li", nTaskID );
				// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
				CString strAssignTo = VarString(m_List->GetValue(nCurSel, COLUMN_ASSIGN_TO_IDS), "");
				

				//if the completed radio button is checked then we need to remove this item from this list 
				if (m_incomplete.GetCheck()) {
					m_List->RemoveRow(nCurSel);
				}
				else if (m_all.GetCheck()) {
					COleDateTime date;
					m_List->PutValue(nCurSel, COLUMN_COMPLETE_DATE, _variant_t(FormatDateTimeForInterface(date, DTF_STRIP_SECONDS, dtoDate)));
				}

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientTodoTaskComplete, GetActivePatientID(), "Uncompleted", "Completed", aepMedium, aetChanged);

				//update tracking
				PhaseTracking::SyncLadderWithTodo(nTaskID);
				// (z.manning 2008-10-29 12:13) - PLID 31667 - Update labs
				SyncLabWithTodo(nTaskID);
				//update the ToDo alarm (if it's open)
				// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
			


				CArray < long, long> arrAssignedID;
				ParseDelimitedStringToLongArray(strAssignTo, " ", arrAssignedID);


				if (arrAssignedID.GetSize() == 1){
					CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
				}
				else{
					CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisChanged);
				}

				// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
				/*
				CMainFrame *pMain = GetMainFrame();
				if(pMain) {
					if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
						pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, nTaskID);
					}
				}
				*/

				//Update ourselves
				m_List->Requery();

			}NxCatchAll("Error in CFollowUpDlg::OnCommand \n Cannot change Completion Status");
		}			
		break;

		return TRUE;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

void CFollowUpDlg::OnLButtonDownListTodo(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-06-30 10:18) - PLID 11599 - Let users edit the todo assignee list of
	// the selected row
	try {
		if(m_List->GetCurSel()==-1)
			return;

		if (COLUMN_ASSIGN_TO_NAMES == nCol) {
			//check to see if the user has permission to edit this row
			// (a.walling 2006-10-31 16:53) - PLID 23299 - Only proceed if we get a valid variant
			// (c.haag 2008-07-03 12:25) - PLID 30615 - This is now encapsulated in a member utility function
			CArray<long,long> arynAssignTo;
			GetAssignToArrayByRow(m_List->GetCurSel(), arynAssignTo);

			//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
			if (!CheckIndividualPermissions(m_List->GetCurSel(), nCol, arynAssignTo)) {
				return;
			}

			if (!CheckAssignToPermissions(arynAssignTo, sptWrite)) {
				return;
			}

			// (a.walling 2011-08-10 11:25) - PLID 44739 - These might be NULL
			const CString strOldIDs = VarString(m_List->GetValue(nRow, COLUMN_ASSIGN_TO_IDS), "");
			const CString strOldNames = VarString(m_List->GetValue(nRow, COLUMN_ASSIGN_TO_NAMES), "");
			CArray<long,long> anOldAssignTo;
			BOOL bRetry = FALSE;
			int i;

			do {
				bRetry = FALSE;

				// Invoke the multi-select dialog
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "UsersT");
				ParseDelimitedStringToLongArray(strOldIDs, " ", anOldAssignTo);
				for (i=0; i < anOldAssignTo.GetSize(); i++) {
					dlg.PreSelect(anOldAssignTo[i]);
				}
				dlg.m_strNameColTitle = "Username";

				if (IDOK == dlg.Open("PersonT INNER JOIN UsersT ON UsersT.PersonID = PersonT.ID",
					"Archived = 0 AND ID > 0",
					"ID", "Username", "Please choose one or more users from the list.", 1))
				{
					// If we get here, the user chose at least one user.					
					CArray<long,long> anNewAssignTo;
					dlg.FillArrayWithIDs(anNewAssignTo);
					CString strNewIDs = ArrayAsString(anNewAssignTo);
					strNewIDs.Replace(",", " ");

					// Check permissions before saving the selection.
					m_List->PutValue(nRow, COLUMN_ASSIGN_TO_IDS, _bstr_t(strNewIDs));
	
					// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
					//to edit a todo alarm
					if (/*!CheckCurrentUserPermissions(bioPatient, sptWrite) ||*/
						!TodoCheckItemReassignmentPermissions(anOldAssignTo, anNewAssignTo)) {

						// Permissions fail. Undo the change and let the user try again.
						m_List->PutValue(nRow, COLUMN_ASSIGN_TO_IDS, _bstr_t(strOldIDs));
						bRetry = TRUE;
					}
					else {
						const long nTaskID = VarLong(m_List->GetValue(nRow, COLUMN_TASK_ID));

						// (j.jones 2008-12-02 10:18) - PLID 30830 - multiple users per tracking step is now allowed
						/*
						// Permissions check out. Make sure it's not bound to tracking ladders.
						{
							_RecordsetPtr prs = CreateParamRecordset("SELECT RegardingType FROM TodoList WHERE TaskID = {INT}", nTaskID);
							if (!prs->eof) {
								TodoType tt = (TodoType)AdoFldLong(prs, "RegardingType");
								if (ttTrackingStep == tt && anNewAssignTo.GetSize() > 1) {
									AfxMessageBox("You may not assign multiple users to a todo alarm linked to a tracking ladder.", MB_ICONERROR);
									m_List->PutValue(nRow, COLUMN_ASSIGN_TO_IDS, _bstr_t(strOldIDs));
									bRetry = TRUE;
									continue;
								}
							} 
						}
						*/
						
						// Update the data
						TodoChangeAssignTo(nTaskID, anNewAssignTo);
						PhaseTracking::SyncLadderWithTodo(nTaskID);

						// Audit the change
						CString strNewNames;
						_RecordsetPtr prs = CreateParamRecordset("SELECT dbo.GetTodoAssignToNamesString({INT}) AS TodoNames, RegardingID, RegardingType FROM TodoList WHERE Todolist.TaskID = {INT}", nTaskID,nTaskID);
						strNewNames = AdoFldString(prs, "TodoNames");
						
						const CString strOld = FormatString("Assigned To: %s", strOldNames);
						const CString strNew = FormatString("Assigned To: %s", strNewNames);
						if (strOld != strNew) {
							long nAuditID = BeginNewAuditEvent();
							AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
						}

						// Now update the names on the screen.
						m_List->PutValue(nRow, COLUMN_ASSIGN_TO_NAMES, _bstr_t(strNewNames));

						// Now send a table checker
						// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
						if (anNewAssignTo.GetSize() == 1){
							CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), anNewAssignTo[0], TableCheckerDetailIndex::tddisChanged);
						}
						else{
							CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisChanged);
						}

						prs->Close();
					}				
				}
				else {
					// The user changed their mind. Don't change anything.
				}
			}
			while (bRetry);

		} // if (COLUMN_ASSIGN_TO_NAMES == nCol) {
	}
	NxCatchAll("Error in CFollowUpDlg::OnLButtonDownListTodo");
}

void CFollowUpDlg::OnEditingFinishedListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)  
{
	try {
		// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
		IRowSettingsPtr pRow;
		pRow = m_List->GetRow(nRow);
		CString strAssignTo="";
		long nTaskID = -1;
		CArray < long, long> arrAssignedID;
		if (pRow != NULL){
			 strAssignTo = VarString(pRow->GetValue(COLUMN_ASSIGN_TO_IDS));
			 nTaskID = VarLong(pRow->GetValue( COLUMN_TASK_ID));
			 
			 ParseDelimitedStringToLongArray(strAssignTo, " ", arrAssignedID);

		}
		else{
			return;
		}
		
	


		//(e.lally 2011-07-13) PLID 44543 - We can stop right here if nothing is being committed.
		if(bCommit == FALSE){
			return;
		}
		 
		long nAuditID = -1;
		CString strNew, strOld;
		switch (nCol) {
			case COLUMN_TASK_TYPE:	// Categories
				try {
					if(varNewValue.vt == VT_I4) {
						CString strID;
						if(varNewValue.lVal == -1)
							strID = "NULL";
						else
							strID.Format("%li",varNewValue.lVal);
						ExecuteSql("Update ToDoList Set CategoryID = %s WHERE TaskID = %li", strID, nTaskID);
						//for auditing
						_RecordsetPtr rsTaskTypeNew = CreateRecordset("SELECT Description FROM NoteCatsF "
																"WHERE ID = %li", varNewValue.lVal);
						if (!rsTaskTypeNew->eof)
							strNew.Format("Category: %s", AdoFldString(rsTaskTypeNew, "Description", ""));
						else
							strNew = "Category: <None>";
					}
					else
						strNew = "Category: <None>";
					//auditing
					if(varOldValue.vt == VT_I4 && varOldValue.lVal != -1) {
						_RecordsetPtr rsTaskTypeOld = CreateRecordset("SELECT Description FROM NoteCatsF "
																"WHERE ID = %li", varOldValue.lVal);
						if (!rsTaskTypeOld->eof)
							strOld.Format("Category: %s", AdoFldString(rsTaskTypeOld, "Description", ""));
					}
					else
						strOld = "Category: <None>";
					nAuditID = BeginNewAuditEvent();
					if (strOld != strNew)
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;	
			case COLUMN_TASK:		// Methods
				try {

					//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
					// Get old value for auditing
					_RecordsetPtr rsMethodTypeOld = CreateRecordset("SELECT Task FROM ToDoList "
																"WHERE TaskID = %li", nTaskID);
					if (!rsMethodTypeOld->eof) {
						strOld.Format("Method: %s", AdoFldString(rsMethodTypeOld, "Task", ""));
						if (Trim(AdoFldString(rsMethodTypeOld, "Task", "")) == "") {
							strOld.Format("Method: <none>");
						}
					}
					else {
						strOld.Format("Method: <none>");
					}

					if(varNewValue.vt == VT_BSTR) {
						ExecuteSql("Update ToDoList Set Task = '%s' WHERE TaskID = %li", _Q(VarString(varNewValue)), nTaskID);
					
						//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
						// Audit new value
						strNew.Format("Method: %s", _Q(VarString(varNewValue)));
					}
					else {
						//(e.lally 2011-07-13) PLID 44543 - The method is not selected! We don't support committing this.
						return;
					}

					nAuditID = BeginNewAuditEvent();
					if (strOld != strNew) {
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
					}
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;	
			case COLUMN_PRIORITY:
				try {
					//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
					// Get old value for auditing
					_RecordsetPtr rsPriorityOld = CreateRecordset("SELECT Priority FROM ToDoList "
																"WHERE TaskID = %li", nTaskID);
					if (!rsPriorityOld->eof) {
						strOld.Format("Priority: %s", StrPriorityString(AdoFldByte(rsPriorityOld, "Priority", -1)));
					}
					else {
						strOld.Format("Priority: <none>");
					}

					if(varNewValue.vt == VT_I2)
						ExecuteSql("Update ToDoList Set Priority = %i WHERE TaskID = %li", VarShort(varNewValue), nTaskID);
					ColorizeList(nRow);

					//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
					// Audit new value
					strNew.Format("Priority: %s", _Q(StrPriorityString(VarShort(varNewValue))));
					nAuditID = BeginNewAuditEvent();
					if (strOld != strNew) {
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
					}

				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;	
			// (c.haag 2008-06-10 10:41) - PLID 11599 - This is obselete. We now use a pop-up window.
			/*case COLUMN_ASSIGN_TO:
				try {
					if(varNewValue.vt == VT_I4) {
						// (c.haag 2008-06-09 12:12) - PLID 30321 - We now use a global function to change the assignee
						TodoChangeAssignTo(nTaskID,VarLong(varNewValue));
						//for auditing
						_RecordsetPtr rsAssignToNew = CreateRecordset("SELECT UserName FROM UsersT "
																"WHERE PersonID = %li", VarLong(varNewValue));
						if (!rsAssignToNew->eof)
							strNew.Format("Assigned To: %s", AdoFldString(rsAssignToNew, "UserName", ""));
					}
					PhaseTracking::SyncLadderWithTodo(nTaskID);
					//auditing
					if(varOldValue.vt == VT_I4) {
						_RecordsetPtr rsAssignToOld = CreateRecordset("SELECT UserName FROM UsersT "
																"WHERE PersonID = %li", VarLong(varOldValue));
						if (!rsAssignToOld->eof)
							strOld.Format("Assigned To: %s", AdoFldString(rsAssignToOld, "UserName", ""));
					}
					nAuditID = BeginNewAuditEvent();
					if (strOld != strNew)
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;	*/
			case COLUMN_NOTES:
				try {
					if (bCommit) {
						ExecuteSql("Update ToDoList Set Notes = '%s' WHERE TaskID = %li", _Q(VarString(varNewValue)), nTaskID);
						//auditing
						nAuditID = BeginNewAuditEvent();
						strOld.Format("Note: %s", VarString(varOldValue));
						strNew.Format("Note: %s", VarString(varNewValue));
						if (strOld != strNew)
							AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
					}
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;	
			case COLUMN_COMPLETE_BY:
				try {
					if (bCommit) {
						ExecuteSql("Update ToDoList Set Deadline = '%s', Remind = CASE WHEN Remind > '%s' THEN '%s' ELSE Remind END WHERE TaskID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), nTaskID);
						PhaseTracking::SyncLadderWithTodo(nTaskID);
						//auditing
						nAuditID = BeginNewAuditEvent();
						strOld.Format("Deadline: %s", FormatDateTimeForInterface(VarDateTime(varOldValue), dtoDate));
						strNew.Format("Deadline: %s", FormatDateTimeForInterface(VarDateTime(varNewValue), dtoDate));
						if (strOld != strNew)
							AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
					}
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;
			case COLUMN_COMPLETE_DATE:
				try {
					if (bCommit) {
						if(varNewValue.vt == VT_NULL) {
							//This isn't done any more.
							ExecuteSql("Update ToDoList SET Done = NULL WHERE TaskID = %li", nTaskID);
							if(m_completed.GetCheck()) {
								m_List->RemoveRow(nRow);
							}
						}
						else {
							ExecuteSql("Update ToDoList SET Done = '%s' WHERE TaskID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), nTaskID);
							if (m_incomplete.GetCheck()) {
								m_List->RemoveRow(nRow);
							}
						}
						PhaseTracking::SyncLadderWithTodo(nTaskID);
						// (z.manning 2008-10-29 12:13) - PLID 31667 - Update labs
						SyncLabWithTodo(nTaskID);
					}
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
				break;
		}	
		
		

		
		
		// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
		if (arrAssignedID.GetSize() == 1){
			CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), arrAssignedID[0]  , TableCheckerDetailIndex::tddisChanged,FALSE);
		}
		else{
			CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisChanged,FALSE);
		}

		// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
		/*
		CMainFrame *pMain = GetMainFrame();
		if(pMain) {
			if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
				pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, nTaskID);
			}
		}
		*/

	} NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
}

BOOL CFollowUpDlg::PreTranslateMessage(MSG* pMsg) 
{
	return CNxDialog::PreTranslateMessage(pMsg);
}

void CFollowUpDlg::OnEditingFinishingListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(*pbCommit == FALSE) {
			return;
		}

		switch (nCol) {
			case COLUMN_COMPLETE_BY:  //Deadline Column
				if (pvarNewValue->vt != VT_DATE) {
					if(pvarNewValue->vt == VT_BSTR) {
						if(CString(pvarNewValue->bstrVal) == "")
							*pbCommit = FALSE;
						else
						{
							COleDateTime dt;
							dt.ParseDateTime(CString(pvarNewValue->bstrVal));
							//TES 11/6/2007 - PLID 28007 - ParseDateTime() returns a bool, not, as whoever wrote
							// this seems to have believed, a date value.  This mostly worked, however, because
							// it was comparing dt to "true", aka 1, and a dt value less than 1 is also less than
							// 1/1/1900.  I fixed the code to do what the original creator seems to have intended, 
							// and also resolve a compile warning in VS 2008.
							if(dt.m_status==COleDateTime::invalid || dt < COleDateTime(1800,1,1,0,0,0)/*dttemp.ParseDateTime("01/01/1800")*/) {
								AfxMessageBox("Please enter a valid date.");
								*pbCommit = FALSE;
								*pbContinue = FALSE;
								return;
							}
							pvarNewValue->vt = VT_DATE;
							pvarNewValue->date = dt;
						}
					}
					else {
						AfxMessageBox("Please enter a valid date.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
				break;
			case COLUMN_COMPLETE_DATE: //Finished Column
				if (pvarNewValue->vt != VT_DATE ) {
					if(pvarNewValue->vt == VT_BSTR) {
						if(CString(pvarNewValue->bstrVal) == "") 
							pvarNewValue->vt = VT_NULL;
						else
						{
							COleDateTime dt;
							dt.ParseDateTime(CString(pvarNewValue->bstrVal));
							//TES 11/6/2007 - PLID 28007 - ParseDateTime() returns a bool, not, as whoever wrote
							// this seems to have believed, a date value.  This mostly worked, however, because
							// it was comparing dt to "true", aka 1, and a dt value less than 1 is also less than
							// 1/1/1900.  I fixed the code to do what the original creator seems to have intended, 
							// and also resolve a compile warning in VS 2008.
							if(dt.m_status==COleDateTime::invalid || dt < COleDateTime(1800,1,1,0,0,0)/*dttemp.ParseDateTime("01/01/1800")*/) {
								AfxMessageBox("Please enter a valid date.");
								*pbCommit = FALSE;
								*pbContinue = FALSE;
								return;
							}
							else if (dt > COleDateTime::GetCurrentTime())
							{
								if (IDNO == MsgBox(MB_YESNO, "You have entered a completion time that exists in the future. Are you sure you wish to do this?"))
								{
									*pbCommit = FALSE;
									*pbContinue = FALSE;
									return;								
								}
							}
							pvarNewValue->vt = VT_DATE;
							pvarNewValue->date = dt;
						}
					}
					else {
						AfxMessageBox("Please enter a valid date.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
				break;
			case COLUMN_NOTES:
				{			
					// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks		
					CString strNewValue = VarString(pvarNewValue, "");
					if(strNewValue.GetLength() > 2000) {
						MsgBox("The text you entered is longer than the maximum amount (2000) and has been shortened.\n"
							"Please double-check your note and make changes as needed.");
						::VariantClear(pvarNewValue);
						*pvarNewValue = _variant_t(strNewValue.Left(2000)).Detach();
					}
				}
				break;
		}
	} NxCatchAll("Error in CFollowUpDlg::OnEditingFinishingListTodo()");	
}

void CFollowUpDlg::OnRadioAll() 
{
	//(e.lally 2009-11-16) PLID 36304 - Requery contains the try/catch
	Requery();
}

void CFollowUpDlg::OnRadioCompleted() 
{
	//(e.lally 2009-11-16) PLID 36304 - Requery contains the try/catch
	Requery();
}

void CFollowUpDlg::OnRadioIncomplete() 
{
	//(e.lally 2009-11-16) PLID 36304 - Requery contains the try/catch
	Requery();
}

void CFollowUpDlg::OnRequeryFinishedListTodo(short nFlags) 
{
	// Go through list coloring completed tasks blue and uncompleteable tasks red
	//m_List->ResetColors();
	try {
		ColorizeList();
	} NxCatchAll("Error changing colors.");
}

void CFollowUpDlg::OnCreateTodo() 
{
	//(e.lally 2009-11-16) PLID 36304 - Moved actual code into its own CreateTodo function
	try {
		CreateTodo();
	}NxCatchAll("Error in CFollowUpDlg::OnClickBtnTodo() ");	
}

void CFollowUpDlg::CreateTodo()
{
	/* Create a new todo from the task edit dialog */

	// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
	//to create a todo alarm
	/*
	if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
		return;
	*/
	
		CTaskEditDlg dlg(this);
		dlg.m_nPersonID = GetActivePatientID(); // (a.walling 2008-07-07 17:53) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = TRUE;

		long nResult = dlg.DoModal();

		if (nResult != IDOK) return;

		// Added by CH 1/17: Force the next remind
		// time to be 5 minutes if a new task is
		// added so the "Don't remind me again"
		// option will not cause the new task to
		// be forgotten.
		{
			COleDateTime dt = COleDateTime::GetCurrentTime();
			dt += COleDateTimeSpan(0,0,5,0);
			SetPropertyDateTime("TodoTimer", dt);
			// (j.dinatale 2012-10-22 17:59) - PLID 52393 - set our user preference
			SetRemotePropertyInt("LastTimeOption_User", 5, 0, GetCurrentUserName());
			SetTimer(IDT_TODO_TIMER, 5*60*1000, NULL);
		}
	
		// (c.haag 2008-06-10 10:41) - PLID 11599 - Changed query and list population to use new multi-assign-to structure
		// (z.manning 2008-10-13 11:21) - PLID 31667 - Added lab based to-do actions
		// (j.gruber 2010-02-24 12:59) - PLID 37510 - added clinical decision support rules
		_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT ToDoList.PersonID, ToDoList.TaskID, Task, CategoryID, Priority, ToDoList.Notes, "
			"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
			"Deadline, Done, "
			"CASE WHEN UsersT.UserName Is Null THEN CASE WHEN RegardingType = %i THEN '<Tracking>' WHEN RegardingType = %i THEN '<Custom Record>' WHEN RegardingType = %i THEN '<Billing>' WHEN RegardingType = %i THEN '<Labs>' WHEN RegardingType = %i THEN '<Clinical Decision Support Rules>'  ELSE '' END ELSE UsersT.UserName END AS EnteredByName "
			"FROM ToDoList "
			"LEFT JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID WHERE TaskID = {INT}",
			ttTrackingStep, ttCustomRecord, ttGlobalPeriod, ttLabStep, ttDecisionRule), dlg.m_iTaskID);
		IRowSettingsPtr pRow;
		FieldsPtr fields;
		fields  = rs->Fields;
		pRow = m_List->GetRow(-1);
		pRow->PutValue(COLUMN_TASK_ID, dlg.m_iTaskID);
		pRow->PutValue(COLUMN_TASK_TYPE, fields->Item["CategoryID"]->Value);
		pRow->PutValue(COLUMN_TASK, fields->Item["Task"]->Value);
		//for some reason this field is not being read correctly as a VT_UI1, so we have to manually change it to a VT_I2
		_variant_t var = fields->Item["Priority"]->Value;
		var.vt = VT_I2;
//		pRow->PutValue(3, fields->Item["Priority"]->Value);
		pRow->PutValue(COLUMN_PRIORITY, var);
		pRow->PutValue(COLUMN_NOTES, fields->Item["Notes"]->Value);
		pRow->PutValue(COLUMN_ASSIGN_TO_NAMES, fields->Item["AssignedToNames"]->Value);
		pRow->PutValue(COLUMN_ASSIGN_TO_IDS, fields->Item["AssignedToIDs"]->Value);
		pRow->PutValue(COLUMN_COMPLETE_BY, fields->Item["Deadline"]->Value);
		_variant_t varFinishDate = fields->Item["Done"]->Value;
		if (varFinishDate.vt == VT_DATE) {
			COleDateTime dtDate = VarDateTime(varFinishDate);
			CString strDate = FormatDateTimeForInterface(dtDate, DTF_STRIP_SECONDS, dtoDate);
			pRow->PutValue(COLUMN_COMPLETE_DATE, _variant_t(strDate));
		}
		else {
			//thats ok
			pRow->PutValue(COLUMN_COMPLETE_DATE, varFinishDate);
		}
		pRow->PutValue(COLUMN_ENTERED_BY, fields->Item["EnteredByName"]->Value);
		if((fields->Item["Done"]->Value.vt==VT_NULL && !m_completed.GetCheck()) || (fields->Item["Done"]->Value.vt!=VT_NULL && !m_incomplete.GetCheck()))
		{
			if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
				ColorizeItem(pRow);
			}
			m_List->AddRow(pRow);
		}
	/*	// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers -- Removed Redundant tablechecker send
		long nCurSel = m_List->GetCurSel();
		CString strAssignTo = VarString(fields->Item["AssignedToIDs"]->Value, "");


		CArray < long, long> arrAssignedID;
		ParseDelimitedStringToLongArray(strAssignTo, ",", arrAssignedID);


		if (arrAssignedID.GetSize() == 1){
			CClient::RefreshTodoTable(dlg.m_iTaskID, GetActivePatientID(), arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
		}
		else{
			CClient::RefreshTodoTable(dlg.m_iTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisChanged);
		}*/

	

		// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
		/*
		CMainFrame *pMain = GetMainFrame();
		if(pMain) {
			if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
				pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, dlg.m_iTaskID);
			}
		}
		*/

}

void CFollowUpDlg::OnTodoList() 
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch
	try{
		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame->GetSafeHwnd()) {
			pMainFrame->ShowTodoList();
		}


		//I don't think we need this now that we have the network code in here
		//they could have deleted items!
		//m_List->Requery();
	}NxCatchAll(__FUNCTION__);
}

void CFollowUpDlg::OnDeleteCompleted() 
{
	//(e.lally 2009-11-16) PLID 36304 - Moved try to the top
	try{
		bool bDeleteSelf = false;
		bool bDeleteOthers = false;

		if(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT_____D______ANDPASS)){
			bDeleteSelf = true;
		}
		if(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT_____D______ANDPASS)){
			bDeleteOthers = true;
		}

		CString strSql, strMessage, strAuditValue, strInternalUpdate;
	
		if(!bDeleteSelf && !bDeleteOthers) {
			// user cannot delete any follow-ups
			AfxMessageBox("You do not have permission to delete any follow-ups.");
			return;
		}
		else if(bDeleteSelf && !bDeleteOthers) {
			// user can delete own follow-ups but not others
			// (c.haag 2008-06-13 11:10) - PLID 11599 - Use new todo structure. If you do not have permission to delete
			// others' todo alarms, then you can only delete finished todos assigned to you (even if they are also assigned
			// to other users)
			_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(TaskID) AS NumItems FROM ToDoList "
				"WHERE (Done IS NOT NULL) AND PersonID = {INT} "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = {INT})" // Assigned to you
				, m_id, GetCurrentUserID(), GetCurrentUserID());
			long nCount = AdoFldLong(rsCount, "NumItems");
			if(nCount == 0) {
				MsgBox("You only have permission to delete your own follow-ups, but you have no completed follow-ups for this patient assigned to you.  "
					"No action will be taken.");
				return;
			}
			strMessage.Format("You only have permission to delete your own follow-ups.  Are you sure you wish "
				" to delete these %li follow-up(s)?", nCount);
			// (c.haag 2008-06-13 11:26) - PLID 11599 - This is easier if we just create a temp table with all the tasks to delete
			CString strTempTable = FormatString("DECLARE @tblTasks TABLE (ID INT NOT NULL)\r\n"
				"INSERT INTO @tblTasks (ID)\r\n "
				"SELECT TaskID FROM ToDoList\r\n "
				"WHERE (Done IS NOT NULL) AND PersonID = %d\r\n "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = %d)\r\n" // Assigned to you
				, m_id, GetCurrentUserID(), GetCurrentUserID());
			strSql += strTempTable;
			// (j.gruber 2008-04-02 11:11) - PLID 29296 - update clientimplementationstepsT
			// (c.haag 2008-06-13 11:40) - PLID 11599 - Use new todo structure
			strInternalUpdate.Format("%s UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL "
					" FROM ClientImplementationStepsT INNER JOIN TodoList ON ClientImplementationStepsT.ToDoTaskID = ToDoList.TaskID "
					" WHERE TodoList.TaskID IN (SELECT ID FROM @tblTasks) ",
					strTempTable);
			// (c.haag 2008-06-11 10:55) - PLID 30328 - Consider the new data structure when deleting todos
			// (c.haag 2008-07-10 15:33) - PLID 30674 - Include the EMR todo list table
			strSql += FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT ID FROM @tblTasks)") +
					FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT ID FROM @tblTasks)") +
					FormatString("DELETE FROM ToDoList WHERE TaskID IN (SELECT ID FROM @tblTasks)");
			strAuditValue.Format("Cleared Own Items");
		}
		else if(!bDeleteSelf && bDeleteOthers) {
			// user can only delete other's follow-ups
			// (c.haag 2008-06-13 11:29) - PLID 11599 - Use new todo structure. The user can only delete follow-ups
			// assigned to other users (even if they are also assigned to you)
			_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(TaskID) AS NumItems FROM ToDoList "
				"WHERE (Done IS NOT NULL) AND PersonID = {INT} "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> {INT}) " // Assigned to other users
				";\r\n", m_id, GetCurrentUserID(), GetCurrentUserID());
			long nCount = AdoFldLong(rsCount, "NumItems");
			if(nCount == 0) {
				MsgBox("You only have permission to delete other users' follow-ups, but no other users have completed follow-ups for this patient which are not assigned to you.  "
					"No action will be taken.");
				return;
			}
					
			strMessage.Format("You only have permission to delete other users' follow-ups.  Are you sure you wish "
				" to delete these %li follow-up(s)?", nCount);
			// (j.gruber 2008-04-02 11:11) - PLID 29296 - update clientimplementationstepsT
			// (c.haag 2008-06-13 11:34) - PLID 11599 - This is easier if we just create a temp table with all the tasks to delete
			CString strTempTable = FormatString("DECLARE @tblTasks TABLE (ID INT NOT NULL)\r\n"
				"INSERT INTO @tblTasks (ID)\r\n "
				"SELECT TaskID FROM ToDoList\r\n "
				"WHERE (Done IS NOT NULL) AND PersonID = %d\r\n "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> %d)\r\n" // Assigned to other users
				, m_id, GetCurrentUserID(), GetCurrentUserID());
			strSql += strTempTable;
			// (c.haag 2008-06-13 11:41) - PLID 11599 - Use new todo structure
			strInternalUpdate.Format("%s UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL "
					" FROM ClientImplementationStepsT INNER JOIN TodoList ON ClientImplementationStepsT.ToDoTaskID = ToDoList.TaskID "
					" WHERE TodoList.TaskID IN (SELECT ID FROM @tblTasks) ", strTempTable);
			// (c.haag 2008-06-11 10:55) - PLID 30328 - Consider the new data structure when deleting todos
			// (c.haag 2008-07-10 15:33) - PLID 30674 - Include the EMR todo list table
			strSql += FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT ID FROM @tblTasks)") +
					FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT ID FROM @tblTasks)") +
					FormatString("DELETE FROM ToDoList WHERE TaskID IN (SELECT ID FROM @tblTasks)");
			strAuditValue.Format("Cleared Others' Items");
		}
		else{
			// the user can delete all follow-ups
			_RecordsetPtr rsCount = CreateRecordset("SELECT Count(TaskID) AS NumItems FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li", m_id);
			long nCount = AdoFldLong(rsCount, "NumItems");
			if(nCount == 0) {
				MsgBox("There are no completed follow-ups for this patient.  No action will be taken.");
				return;
			}
			strMessage.Format("Are you sure you wish to delete the %li completed task(s)?", nCount);
			// (j.gruber 2008-04-02 11:11) - PLID 29296 - update clientimplementationstepsT
			strInternalUpdate.Format("UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL "
					" FROM ClientImplementationStepsT INNER JOIN TodoList ON ClientImplementationStepsT.ToDoTaskID = ToDoList.TaskID "
					" WHERE TodoList.Done IS NOT NULL AND PersonID = %li ", 
					m_id);
			// (c.haag 2008-06-11 10:55) - PLID 30328 - Consider the new data structure when deleting todos
			// (c.haag 2008-07-10 15:33) - PLID 30674 - Include the EMR todo list table
			strSql = FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li)\r\n", m_id) +
				FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT TaskID FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li)\r\n", m_id) +
				FormatString("DELETE FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li", m_id);
			strAuditValue.Format("Cleared All Items");
		}
	
		if (IDNO == MessageBox(strMessage, "NexTech", MB_YESNO))
			return;

		// (j.gruber 2008-04-02 11:11) - PLID 29296 - update clientimplementationstepsT
		if (IsNexTechInternal()) {
			ExecuteSqlStd(strInternalUpdate);
		}

		ExecuteSqlStd(strSql);

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientTodoTaskClear, GetActivePatientID(), "", strAuditValue, aepMedium, aetDeleted);


		m_List->Requery();

	}NxCatchAll("Error in CFollowUpDlg::OnClickBtnDeleteDone");	
}

void CFollowUpDlg::OnEditCategories() 
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch
	try{
		CNoteCategories	dlg(this);
		dlg.m_bEditingFollowUpCat = true;
		dlg.DoModal();

		m_List->Requery();
	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2014-08-26 11:50) - PLID 63226 -
void CFollowUpDlg::ReflectChangedTodo(CTableCheckerDetails* pDetails){

	try{
		long nTaskID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTaskID), -1);
		long nPersonID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiPersonID), -1);		
		TableCheckerDetailIndex::Todo_Status todoStatus = (TableCheckerDetailIndex::Todo_Status)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTodoStatus), -1);

		NXDATALISTLib::IRowSettingsPtr pRow;
		// not current patient.. return 
		if (m_id != nPersonID){
			return;
		}
		// if the todo was deleted
		
		if (m_List->IsRequerying()){
			return;
		}

		long nRow= m_List->FindByColumn(COLUMN_TASK_ID, nTaskID, -1, VARIANT_FALSE);

		if (nRow == -1){
			
			pRow = m_List->GetRow(-1);

		}
		else{
			pRow = m_List->GetRow(nRow);

		}



		if (TableCheckerDetailIndex::tddisDeleted== todoStatus){

			if (nRow==-1){

				// not in the list don't need to worry about removing it
			}
			else{

				m_List->RemoveRow(pRow->Index);
			}

			return;
		}

		if (TableCheckerDetailIndex::tddisChanged == todoStatus || TableCheckerDetailIndex::tddisAdded == todoStatus){

			// whether added or updating we need the new todo infomation
			_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT ToDoList.PersonID, ToDoList.TaskID, Task, CategoryID, Priority, ToDoList.Notes, "
				"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
				"Deadline, Done, "
				"CASE WHEN UsersT.UserName Is Null THEN CASE WHEN RegardingType = %i THEN '<Tracking>' WHEN RegardingType = %i THEN '<Custom Record>' WHEN RegardingType = %i THEN '<Billing>' WHEN RegardingType = %i THEN '<Labs>' WHEN RegardingType = %i THEN '<Clinical Decision Support Rules>'  ELSE '' END ELSE UsersT.UserName END AS EnteredByName "
				"FROM ToDoList "
				"LEFT JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID WHERE TaskID = {INT}",
				ttTrackingStep, ttCustomRecord, ttGlobalPeriod, ttLabStep, ttDecisionRule), nTaskID);

			FieldsPtr fields;
			fields = rs->Fields;
			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_TASK_ID, nTaskID);
			PopulateListRow(pRow, fields);

			if ((fields->Item["Done"]->Value.vt == VT_NULL && !m_completed.GetCheck()) || (fields->Item["Done"]->Value.vt != VT_NULL && !m_incomplete.GetCheck()))
			{
				if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
					ColorizeItem(pRow);
				}

			}

			if (pRow != NULL){
				if (fields->Item["Done"]->Value.vt == VT_NULL){
					//if the radio button is set to uncompleted or all we can show this todo
					if (!m_completed.GetCheck()){

						// not in the list going to need to add the todo
						if (nRow == -1){
							m_List->AddRow(pRow);
						}
						else // row is in the list we need to update the row
						{
							// (r.goldschmidt 2015-07-06 16:50) - PLID 65472 - update row in place instead of removing and re-adding
							PopulateListRow(nRow, fields);
							ColorizeList(nRow);
						}
					}
					else{// we have todo that has been updated or be uncompleted need to remove it from the completed list
						//Todo is not completed 
						// check if it was there in the first place
						if (nRow != -1){
							m_List->RemoveRow(nRow);
						}
					}
				}// we have a todo that has been completed 
				else if ((fields->Item["Done"]->Value.vt != VT_NULL)){
					//check to see that were not showing only uncompleted todos
					if (!m_incomplete.GetCheck()){
						// not in the list going to need to add the todo
						if (nRow == -1){
							m_List->AddRow(pRow);
						}
						else // row is in the list we need to update the row with the new infomations
						{
							// (r.goldschmidt 2015-07-06 16:50) - PLID 65472 - update row in place instead of removing and re-adding
							PopulateListRow(nRow, fields);
							ColorizeList(nRow);
						}
					}
					else{// seems this todo has been marked done and is in the incomplete list remove it
						if (nRow != -1){
							m_List->RemoveRow(nRow);
						}
					}


				}


			}


		}
		
		

		
	}NxCatchAll(__FUNCTION__)
	


}



// (s.tullis 2014-08-26 11:50) - PLID 63226 -
LRESULT CFollowUpDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {


			try {
					
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
					switch (wParam)
					{

						case NetUtils::TodoList:
						
						ReflectChangedTodo(pDetails);
						default:
						break;
					}
				
				
			}NxCatchAll(__FUNCTION__)


		  return 0;
}

LRESULT CFollowUpDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {
	try {

		if (wParam == NetUtils::TodoList) {
			try {
				//the TaskID is in the lParam, so get that first
				long nTaskID = lParam;

				if (m_incomplete.GetCheck()) {

					//Get the item out of the data and either update, delete, or add it to the datalist
					// (c.haag 2008-06-10 11:00) - PLID 11599 - Use the new todo assign structure
					// (z.manning 2008-10-13 11:23) - PLID 31667 - Lab based todos
					// (j.gruber 2010-02-24 12:59) - PLID 37510 - added clinical decision support rules
					_RecordsetPtr rsTask = CreateParamRecordset(FormatString("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, "
						"CASE WHEN UsersT.UserName Is Null THEN CASE WHEN RegardingType = %i THEN '<Tracking>' WHEN RegardingType = %i THEN '<Custom Record>' WHEN RegardingType = %i THEN '<Billing>' WHEN RegardingType = %i THEN '<Labs>' WHEN RegardingType = %i THEN '<Clinical Decision Support Rules>' ELSE '' END ELSE UsersT.UserName END AS EnteredByName, "
						"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
						"ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
						"FROM ToDoList "
						"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
						"WHERE (TaskID = {INT}) AND (Done IS NULL) AND (ToDoList.PersonID = {INT}) ", ttTrackingStep, ttCustomRecord, ttGlobalPeriod, ttLabStep, ttDecisionRule), nTaskID, GetActivePatientID());
					FieldsPtr fields = rsTask->Fields;

					if (rsTask->eof) {

						//the task is not in the datalist at the moment and shouldn't be, try to remove it if it is there
						long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow != -1) {

						//it is there, so remove
						m_List->RemoveRow(nRow);

						}
						else {
						
							//it is not there and it shouldn't be there, so we are all good
						}

					}
					else {

				
						//there is something in the recordset, so we have to determine whether to add it or just update it
						long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow == -1) {

							//the row is not there, so just add it
							IRowSettingsPtr pRow = m_List->GetRow(-1);
							pRow->PutValue(COLUMN_TASK_ID, nTaskID);
							PopulateListRow(pRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
								ColorizeItem(pRow);
							}
							//add the row to the datalist	
							m_List->AddRow(pRow);

							
							
						}
						else {

							//the row is already there, we just have to update the values in it
							PopulateListRow(nRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							ColorizeList(nRow);
						}
						
					}
				}else if (m_completed.GetCheck() ) {

					
					//Get the item out of the data and either update, delete, or add it to the datalist
					// (c.haag 2008-06-10 11:03) - PLID 11599 - Use the new todo assign structure
					// (z.manning 2008-10-13 11:23) - PLID 31667 - Lab based todos
					// (j.gruber 2010-02-24 13:00) - PLID 37510 - CDSR
					_RecordsetPtr rsTask = CreateParamRecordset(FormatString("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, CASE WHEN UsersT.UserName Is Null THEN CASE WHEN RegardingType = %i THEN '<Tracking>' WHEN RegardingType = %i THEN '<Custom Record>' WHEN RegardingType = %i THEN '<Billing>' WHEN RegardingType = %i THEN '<Labs>' WHEN RegardingType = %i THEN '<Clinical Decision Support Rules>' ELSE '' END ELSE UsersT.UserName END AS EnteredByName, "
						"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
						"ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
						"FROM ToDoList "
						"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
						"WHERE (TaskID = {INT}) AND (Done IS NOT NULL) AND (ToDoList.PersonID = {INT}) ", ttTrackingStep, ttCustomRecord, ttGlobalPeriod, ttLabStep, ttDecisionRule), nTaskID, GetActivePatientID());
					FieldsPtr fields = rsTask->Fields;

					if (rsTask->eof) {

						//the task shouldn't be in the data list, try to remove it if it is there
						long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow != -1) {

							//it is there, so remove
							m_List->RemoveRow(nRow);

						}
						else {
						
							//it is not there and it shouldn't be there, so we are all good
						}

					}
					else {

				
						//there is something in the recordset, so we have to determine whether to add it or just update it
						long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow == -1) {

							//the row is not there, so just add it
							IRowSettingsPtr pRow = m_List->GetRow(-1);

							pRow->PutValue(COLUMN_TASK_ID, nTaskID);
							PopulateListRow(pRow, fields);

							//color the row
							//pRow->PutForeColor(RGB(0,0,128));

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
								ColorizeItem(pRow);
							}

							//add the row to the datalist	
							m_List->AddRow(pRow);
						}
						else {

							//the row is already there, we just have to update the values in it
							PopulateListRow(nRow, fields);

							//color the row
							/*
							IRowSettingsPtr pRow = m_List->GetRow(nRow);
							pRow->PutForeColor(RGB(0,0,128));
							*/

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							ColorizeList(nRow);

						}
						
					}
				}
				else { //we are showing all

					
					//Get the item out of the data and either update, delete, or add it to the datalist
					// (z.manning 2008-10-13 11:24) - PLID 31667 - Lab based todos
					// (j.gruber 2010-02-24 13:01) - PLID 37510 - CDSR
					_RecordsetPtr rsTask = CreateParamRecordset(FormatString("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, CASE WHEN UsersT.UserName Is Null THEN CASE WHEN RegardingType = %i THEN '<Tracking>' WHEN RegardingType = %i THEN '<Custom Record>' WHEN RegardingType = %i THEN '<Billing>' WHEN RegardingType = %i THEN '<Labs>' WHEN RegardingType = %i THEN '<Clinical Decision Support Rules>' ELSE '' END ELSE UsersT.UserName END AS EnteredByName, "
						"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
						"ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
						"FROM ToDoList "
						"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
						"WHERE (TaskID = {INT}) AND (ToDoList.PersonID = {INT}) ", ttTrackingStep, ttCustomRecord, ttGlobalPeriod, ttLabStep, ttDecisionRule), nTaskID, GetActivePatientID());
					FieldsPtr fields = rsTask->Fields;

					if (rsTask->eof) {

						//the task shouldn't be in the data list, try to remove it if it is there
						long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow != -1) {

							//it is there, so remove
							m_List->RemoveRow(nRow);

						}
						else {
						
							//it is not there and it shouldn't be there, so we are all good
						}

					}
					else {

				
						//there is something in the recordset, so we have to determine whether to add it or just update it
						long nRow = m_List->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow == -1) {

							//the row is not there, so just add it
							IRowSettingsPtr pRow = m_List->GetRow(-1);

							pRow->PutValue(COLUMN_TASK_ID, nTaskID);
							PopulateListRow(pRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
								ColorizeItem(pRow);
							}

							//add the row to the datalist
							m_List->AddRow(pRow);
						}
						else {

							//the row is already there, we just have to update the values in it
							m_List->PutValue(nRow, COLUMN_TASK_ID, nTaskID);
							PopulateListRow(nRow, fields);

							/*
							//color the row
							_variant_t varDone;
							varDone = fields->Item["Done"]->Value;
							if (varDone.vt == VT_DATE) {
								IRowSettingsPtr pRow = m_List->GetRow(nRow);
								pRow->PutForeColor(RGB(0,0,128));
							}
							else {
								IRowSettingsPtr pRow = m_List->GetRow(nRow);
								pRow->PutForeColor(RGB(0,0,0));
							}
							*/

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							ColorizeList(nRow);
						}
					}
				}
			} NxCatchAll("Error in CFollowUpDlg::OnTableChanged:TodoList");
		}
		else if (wParam == NetUtils::NoteCatsF) {
			try {
				//requery the combo source
				IColumnSettingsPtr pCol = m_List->GetColumn(COLUMN_TASK_TYPE);
				pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT -1, '{No Category}' ORDER BY Description"));
			} NxCatchAll("Error in CFollowUpDlg::OnTableChanged:NoteCatsF");
		}


	} NxCatchAll("Error in CFollowUpDlg::OnTableChanged");

	return 0;

}

void CFollowUpDlg::OnEditingStartingListTodo(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		if(m_List->GetCurSel()==-1)
			return;

		//check to see if the user has permission to edit this row
		// (a.walling 2006-10-31 16:53) - PLID 23299 - Only proceed if we get a valid variant
		// (c.haag 2008-07-03 12:25) - PLID 30615 - This is now encapsulated in a member utility function
		CArray<long,long> arynAssignTo;
		GetAssignToArrayByRow(m_List->GetCurSel(), arynAssignTo);
		if (!CheckAssignToPermissions(arynAssignTo, sptWrite)) {
			*pbContinue = FALSE;
		}
		//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
		if (!CheckIndividualPermissions(m_List->GetCurSel(), nCol, arynAssignTo)) {
			*pbContinue = FALSE;
		}
	} NxCatchAll("Error in CFollowUpDlg::OnEditingStartingListTodo()");
}

void CFollowUpDlg::OnDblClickCellListTodo(long nRowIndex, short nColIndex) 
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch, call ModifyItem function instead of OnModifyItem
	try{
		if(nRowIndex != -1) {
			m_nRowSel = nRowIndex;
			ModifyItem();
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2009-11-16) PLID 36304 - Renamed to ModifyItem, moved try/catches into callers
void CFollowUpDlg::ModifyItem()
{
		if(m_List->GetCurSel() == -1){
				return;
		}

		// make sure the user has permission to edit this task
		// (c.haag 2008-07-03 12:25) - PLID 30615 - Use a member function to get these permissions
		CArray<long,long> arynAssignTo;
		GetAssignToArrayByRow(m_List->GetCurSel(), arynAssignTo);
		if (!CheckAssignToPermissions(arynAssignTo, sptWrite)) {
			return;
		}

		CTaskEditDlg dlg(this);					
		dlg.m_nPersonID = GetActivePatientID(); // (a.walling 2008-07-07 17:53) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		long nTaskID = VarLong(m_List->GetValue(m_nRowSel,COLUMN_TASK_ID));
		dlg.m_iTaskID = m_List->GetValue(m_nRowSel,COLUMN_TASK_ID).lVal;
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = FALSE;
		if(dlg.DoModal()==IDCANCEL)
			return;
		// (c.haag 2008-06-10 11:10) - PLID 11599 - Use the new assign to structure for populating the list
		// (z.manning 2008-10-13 11:25) - PLID 31667 - Lab based todos
		// (j.gruber 2010-02-24 13:02) - PLID 37510 - CDSR
		_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, "
			"CASE WHEN UsersT.UserName Is Null THEN CASE WHEN RegardingType = %i THEN '<Tracking>' WHEN RegardingType = %i THEN '<Custom Record>' WHEN RegardingType = %i THEN '<Billing>' WHEN RegardingType = %i THEN '<Labs>' WHEN RegardingType = %i THEN '<Clinical Decision Support Rules>'  ELSE '' END ELSE UsersT.UserName END AS EnteredByName, "
			"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
			"ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task, TodoList.RegardingType "
			"FROM ToDoList "
			"LEFT JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
			"WHERE TaskID = {INT}", ttTrackingStep, ttCustomRecord, ttGlobalPeriod, ttLabStep, ttDecisionRule), dlg.m_iTaskID);
		if(rs->eof) {
			rs->Close();
			m_List->RemoveRow(m_nRowSel);
			return;
		}
		FieldsPtr fields;

		fields  = rs->Fields;
		/*// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers -- Removed Redundant tablechecker
		if (arynAssignTo.GetSize() == 1){
			CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), arynAssignTo[0], TableCheckerDetailIndex::tddisChanged);
		}
		else{
			CClient::RefreshTodoTable(nTaskID, GetActivePatientID(), -1, TableCheckerDetailIndex::tddisChanged);
		}*/


		if (m_incomplete.GetCheck()) {
			if (fields->Item["Done"]->Value.vt == VT_NULL) {
				//m_List->SetRedraw(FALSE);
				PopulateListRow(m_nRowSel, fields);
				m_List->SetRedraw(TRUE);
			}
			else {
				m_List->RemoveRow(m_nRowSel);
			}
		}
		else if (m_completed.GetCheck()) {
			if (fields->Item["Done"]->Value.vt == VT_NULL) {
				m_List->RemoveRow(m_nRowSel);
			}
			else {
				//m_List->SetRedraw(FALSE);
				PopulateListRow(m_nRowSel, fields);
				m_List->SetRedraw(TRUE);
			}
		}
		else {
			//m_List->SetRedraw(FALSE);
			PopulateListRow(m_nRowSel, fields);
			m_List->SetRedraw(TRUE);
		}
		
		// (a.walling 2006-09-11 12:25) - PLID 22387 - update the color
		ColorizeList(m_nRowSel);

		//update the todoalarm
		PhaseTracking::SyncLadderWithTodo(nTaskID);
		// (z.manning 2008-10-29 12:13) - PLID 31667 - Update labs
		SyncLabWithTodo(nTaskID);
		

		
		

	
		// (j.jones 2008-11-06 17:30) - PLID 31947 - mainframe will update the ToDo alarm, we should not have to
		/*
		CMainFrame *pMain = GetMainFrame();
		if(pMain) {
			if(pMain->m_dlgToDoAlarm.GetSafeHwnd()) {
				pMain->m_dlgToDoAlarm.OnTableChanged(NetUtils::TodoList, nTaskID);
			}
		}
		*/
		
}
void CFollowUpDlg::OnFollowupShowGrid() 
{
	try {
		if(IsDlgButtonChecked(IDC_FOLLOWUP_SHOW_GRID)) {
			SetRemotePropertyInt("FollowupShowGrid", 1, 0, GetCurrentUserName());
			m_List->GridVisible = true;
		}
		else {
			SetRemotePropertyInt("FollowupShowGrid", 0, 0, GetCurrentUserName());
			m_List->GridVisible = false;
		}
	}NxCatchAll("Error in CFollowUpDlg::OnFollowupShowGrid()");
}

void CFollowUpDlg::ColorizeList(OPTIONAL long nRow /* = -1 */)
{
	const int nPriorityAdj = 10;
	try {
		if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 0) { // color coding is disabled
			return;
		}

		if (nRow < 0) { // color the whole list
			for (int i = 0; i < m_List->GetRowCount(); i++) {
				IRowSettingsPtr pRow = m_List->GetRow(i);
				if (pRow) {
					ColorizeItem(pRow);
				}
			}
		}
		else // just this row...
		{
			IRowSettingsPtr pRow = m_List->GetRow(nRow);
			if (pRow) {
				ColorizeItem(pRow);
			}
		}
	} NxCatchAll("Error in CFollowUpDlg::ColorizeList");
}

void CFollowUpDlg::ColorizeItem(IRowSettingsPtr &pRow)
{
	try {
		if (pRow) {
			_variant_t varCompleted = pRow->GetValue(COLUMN_COMPLETE_DATE);

			if (varCompleted.vt == VT_NULL) {
				int nPriority = VarByte(pRow->GetValue(COLUMN_PRIORITY), 0);

				COLORREF colorAdj;
				switch(nPriority) {
					case 1: colorAdj = m_colorIncompleteHigh; break;
					case 2: colorAdj = m_colorIncompleteMedium; break;
					case 3: default: colorAdj = m_colorIncompleteLow; break;
				}

				pRow->PutBackColor(colorAdj);
			}
			else {
				pRow->PutBackColor(m_colorComplete);
			}
		}
	} NxCatchAll("Error in CFollowUpDlg::ColorizeItem");
}

// (z.manning 2009-09-11 18:11) - PLID 32048 - Moved this logic here from CheckAssignToPermissions
void CFollowUpDlg::GetAssignToArrayByRow(long nRow, CArray<long,long> &arynAssignTo)
{
	//
	// (c.haag 2008-07-03 12:25) - PLID 30615 - Now that we're dealing with todo's that have multiple assignees,
	// we may have to check more that one permission to commit an action of some kind.
	//
	// (a.walling 2011-07-28 14:38) - PLID 44739 - Some bad data can cause this to be NULL.
	ParseDelimitedStringToLongArray(
		VarString(m_List->GetValue(nRow,COLUMN_ASSIGN_TO_IDS), ""), " ", arynAssignTo
		);
}

//(c.copits 2011-02-10) PLID 40794 - Permissions for individual todo alarm fields
BOOL CFollowUpDlg::CheckIndividualPermissions(long nRow, short nCol, CArray<long,long> &arynAssignTo)
{
	BOOL bSelfAssign = FALSE;
	BOOL bRemoteAssign = FALSE;
	BOOL bValid = TRUE;

	for (int i=0; i < arynAssignTo.GetSize(); i++) {
			if (arynAssignTo[i] == GetCurrentUserID()) {
				bSelfAssign = TRUE;
			} else {
				bRemoteAssign = TRUE;
			}
		}

	// Permissions if assigned to self and have self write permissions		
	if (bSelfAssign && CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_TASK_TYPE) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsMethod, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_TASK) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsPriority, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_PRIORITY) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsAssignTo, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_ASSIGN_TO_NAMES) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsNotes, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_NOTES) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsDeadline, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_COMPLETE_BY) {
					bValid = FALSE;
				}
			}
		}
	// Permission if assigned to other and have other write permissions
	if (bRemoteAssign && CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_TASK_TYPE) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsMethod, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_TASK) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsPriority, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_PRIORITY) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsAssignTo, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_ASSIGN_TO_NAMES) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsNotes, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_NOTES) {
					bValid = FALSE;
				}
			}
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsDeadline, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_COMPLETE_BY) {
					bValid = FALSE;
				}
			}
	}

	return bValid;
}
