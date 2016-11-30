// ContactTodo.cpp : implementation file
#include "stdafx.h"
#include "practice.h"
#include "ContactTodo.h"
#include "MainFrm.h"
#include "TaskEditDlg.h"
#include "ToDoAlarmDlg.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "NoteCategories.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "TodoUtils.h"
#include "multiselectdlg.h"

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (c.haag 2008-06-10 10:09) - PLID 11599 - Split out assign-to columns now that we support
// having multiple assign-to's per task
#define COLUMN_TASK_ID			0
#define COLUMN_CATEGORY_ID		1
#define COLUMN_TASK				2
#define COLUMN_PRIORITY			3
#define COLUMN_NOTES			4
#define COLUMN_ASSIGN_TO_NAMES	5
#define COLUMN_ASSIGN_TO_IDS	6
#define COLUMN_COMPLETE_BY		7
#define COLUMN_COMPLETE_DATE	8
#define COLUMN_ENTERED_BY		9


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_ADDTASK	42305
#define ID_MODIFY	42306
#define ID_DELETE	42307
#define ID_MARKDONE	42308
#define ID_MARKNOTDONE 42309

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CContactTodo dialog
CContactTodo::CContactTodo(CWnd* pParent)
	: CNxDialog(CContactTodo::IDD, pParent), m_tblCheckToDoList(NetUtils::TodoList)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Contacts_Module/create_a_to_do_reminder_for_a_contact.htm";
	//{{AFX_DATA_INIT(CContactTodo)
	//}}AFX_DATA_INIT
}

// (c.haag 2008-07-03 12:23) - PLID 30615 - This function encapsulates security checking, and saves
// us from having to repeat a lot of code.
BOOL CContactTodo::CheckAssignToPermissions(long nRow, ESecurityPermissionType type)
{
	//
	// (c.haag 2008-07-03 12:23) - PLID 30615 - Now that we're dealing with todo's that have multiple assignees,
	// we may have to check more that one permission to commit an action of some kind.
	//
	CArray<long,long> anAssignTo;
	ParseDelimitedStringToLongArray(
		VarString(m_list->GetValue(nRow,COLUMN_ASSIGN_TO_IDS)), " ", anAssignTo
		);
	BOOL bSelfAssign = FALSE;
	BOOL bRemoteAssign = FALSE;

	// (a.walling 2006-10-31 17:24) - PLID 23299 - Still check the permissions even though the assign to is
	//		an inactive user.
	// (c.haag 2008-06-10 10:30) - I updated the logic to keep consistent with Adam's change
	// (c.haag 2008-07-03 10:38) - For editing or deleting todo alarms, the rules apply just as they
	// did before with one exception: that is when the todo is assigned to both one self and to another user.
	// Because the todo has joint ownership, an affirmative same-user or other-user permission trumps the 
	// restriction of the other permission. That is, of course, unless both fail the test (in which case, 
	// by definition, you can't edit or delete anything). Consider the possibilities with the alternative -- 
	// you can't edit a todo assigned to you because someone else owns it?
	if (0 == anAssignTo.GetSize()) {
		// The todo is assgined to nobody -- bad data! Return TRUE so that the user may have the chance
		// to fix it.
		return TRUE;
	}
	else {
		for (int i=0; i < anAssignTo.GetSize(); i++) {
			if (anAssignTo[i] == GetCurrentUserID()) {
				bSelfAssign = TRUE;
			} else {
				bRemoteAssign = TRUE;
			}
		}
	}

	if (bSelfAssign && bRemoteAssign) {
		// If we get here, the todo is joint owned. Allow the user to do whatever unless
		// they fail both permission tests
		if(!CheckCurrentUserPermissions(bioSelfFollowUps, type) &&
			!CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
		{
			return FALSE;
		}
	}
	else {
		if (bSelfAssign)
		{
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, type))
				return FALSE;
		}
		if (bRemoteAssign)
		{
			if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
				return FALSE;
		}
	}
	return TRUE;
}

// (c.haag 2008-06-10 15:31) - PLID 11599 - Populates a row in the list with data values
void CContactTodo::PopulateListRow(long nRow, FieldsPtr& f)
{
	IRowSettingsPtr pRow = m_list->GetRow(nRow);
	PopulateListRow(pRow, f);
}

// (c.haag 2008-06-10 15:31) - PLID 11599 - Populates a row in the list with data values
void CContactTodo::PopulateListRow(IRowSettingsPtr& pRow, FieldsPtr& f)
{
	if (NULL != pRow) {
		pRow->PutValue(COLUMN_CATEGORY_ID, f->Item["CategoryID"]->Value);
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

void CContactTodo::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactTodo)
	DDX_Control(pDX, IDC_CONTACTS_FOLLOWUP_SHOW_GRID, m_btnShowGridlines);
	DDX_Control(pDX, IDC_VIEW, m_viewButton);
	DDX_Control(pDX, IDC_CREATE, m_createButton);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_EDIT, m_editButton);
	DDX_Control(pDX, IDC_COMPLETED, m_finished);
	DDX_Control(pDX, IDC_INCOMPLETE, m_unfinished);
	DDX_Control(pDX, IDC_ALL, m_all);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CContactTodo, CNxDialog)
	//{{AFX_MSG_MAP(CContactTodo)
	ON_BN_CLICKED(IDC_COMPLETED, OnCompleted)
	ON_BN_CLICKED(IDC_INCOMPLETE, OnIncomplete)
	ON_BN_CLICKED(IDC_ALL, OnAll)
	ON_BN_CLICKED(IDC_VIEW, OnView)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_CREATE, OnCreateTodo)
	ON_BN_CLICKED(IDC_CONTACTS_FOLLOWUP_SHOW_GRID, OnContactsFollowupShowGrid)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChangedEx)// (s.tullis 2014-08-26 11:50) - PLID 63226 -
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CContactTodo, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CContactTodo)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 6 /* RButtonDown */, OnRButtonDownListTodo, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 7 /* RButtonUp */, OnRButtonUpListTodo, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 4 /* LButtonDown */, OnLButtonDownListTodo, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 9 /* EditingFinishing */, OnEditingFinishingListTodo, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 10 /* EditingFinished */, OnEditingFinishedListTodo, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 8 /* EditingStarting */, OnEditingStartingListTodo, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 3 /* DblClickCell */, OnDblClickCellListTodo, VTS_I4 VTS_I2)
	ON_EVENT(CContactTodo, IDC_LIST_TODO, 18 /* RequeryFinished */, OnRequeryFinishedListTodo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactTodo message handlers
BOOL CContactTodo::OnInitDialog() 
{
	m_viewButton.SetTextColor(0x000000);
	m_createButton.AutoSet(NXB_NEW);
	m_deleteButton.AutoSet(NXB_DELETE);
	m_editButton.AutoSet(NXB_MODIFY);

	m_list = BindNxDataListCtrl(IDC_LIST_TODO,false);
	CNxDialog::OnInitDialog();

	m_unfinished.SetCheck(true);
	m_id = GetActiveContactID();

	if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT____C_______ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT____C_______ANDPASS))){
		m_createButton.EnableWindow(FALSE);
	}
	if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT_____D______ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT_____D______ANDPASS))){
		m_deleteButton.EnableWindow(FALSE);
	}
	if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT__R_________ANDPASS))){
		GetDlgItem(IDC_LIST_TODO)->EnableWindow(FALSE);
		m_viewButton.EnableWindow(FALSE);
	}

	if(GetRemotePropertyInt("ContactsFollowupShowGrid", 0, 0, GetCurrentUserName(), true)) {
		CheckDlgButton(IDC_CONTACTS_FOLLOWUP_SHOW_GRID, BST_CHECKED);
		m_list->GridVisible = true;
	}

	UpdateView();
	return TRUE;
}

void CContactTodo::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	m_id = GetActiveContactID();

	{
		m_colorComplete = (COLORREF)GetRemotePropertyInt("ToDoColorComplete", RGB(210, 255, 210), 0, GetCurrentUserName(), true);
		m_colorIncompleteHigh = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteHigh", RGB(240, 200, 200), 0, GetCurrentUserName(), true);
		m_colorIncompleteMedium = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteMedium", RGB(240, 210, 210), 0, GetCurrentUserName(), true);
		m_colorIncompleteLow = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteLow", RGB(240, 220, 220), 0, GetCurrentUserName(), true);
	}

	Requery(m_id);
}

void CContactTodo::RecallDetails()
{
//	UpdateView();
}

void CContactTodo::Requery(long id)
{
	CString strSQL;

	strSQL.Format ("PersonID = %li", id);
	if (m_finished.GetCheck())
		strSQL += " AND (Done Is Not Null OR Done <> '')";
	else if (m_unfinished.GetCheck())
		strSQL += " AND (Done Is Null OR Done = '')";

	//TES 6/17/2003 - Modify the query to reflect the ability to read todos
	// (c.haag 2008-07-03 12:23) - PLID 30615 - Use TodoAssignToT. Do it in such a way that
	// if you don't have access to see the todo's of person X, then you filter out todo's that
	// are assigned to person X and not to you. If they're assigned to both, you can still see yours.
	if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT__R_________ANDPASS)))
	{
		CString str;
		str.Format(" AND (TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> %d))", GetCurrentUserID());
		strSQL += str;
	}
	if (!(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT__R_________ANDPASS)))
	{
		CString str;
		str.Format(" AND (TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = %d))", GetCurrentUserID());
		strSQL += str;
	}

	if (m_list->GetWhereClause() != _bstr_t(strSQL))
	{
		m_list->WhereClause = _bstr_t(strSQL);
	}

	m_list->Requery();
}

void CContactTodo::OnCompleted() 
{
	UpdateView();	
}

void CContactTodo::OnIncomplete() 
{
	UpdateView();	
}

void CContactTodo::OnAll() 
{
	UpdateView();	
}

void CContactTodo::Delete()
{
	try {
		long nCurSel = m_list->GetCurSel();
		if (nCurSel == -1) {
			MsgBox("Please Select a Task");
			return;
		}
			// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
			if(IsNexTechInternal()) {
				ExecuteParamSql("DELETE FROM AttendanceToDoLinkT WHERE TodoID = {INT}", VarLong(m_list->GetValue(nCurSel,COLUMN_TASK_ID)));
			}
			// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
			TodoDelete(VarLong(m_list->GetValue(nCurSel,COLUMN_TASK_ID)));

			//for audit
			CString strOld = CString(m_list->GetValue(nCurSel, COLUMN_NOTES).bstrVal);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactFollowupDelete, GetActiveContactID(), strOld, "<Deleted>", aepMedium, aetDeleted);

			m_list->RemoveRow(nCurSel);
	}NxCatchAll("Error in CContactTodo::Delete()");
}

void CContactTodo::OnRButtonDownListTodo(long nRow, short nCol, long x, long y, long nFlags) 
{
	
}

BOOL CContactTodo::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	COleVariant var;

	switch (wParam) {
	case ID_ADDTASK:
		if (!(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT____C_______ANDPASS)) && !(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT____C_______ANDPASS))){
			MsgBox("You do not have permission to create Follow-Ups");
		}
		else {
			OnCreateTodo(); 
		}
			break;
		case ID_DELETE:
			try {
				if (m_iRow!=-1) {
					// TES 6/17/2003 - Make sure we have permission to delete this task
					// (c.haag 2008-07-03 12:23) - PLID 30615 - This is now done in a member utility function
					if (!CheckAssignToPermissions(m_list->GetCurSel(), sptDelete)) {
						break;
					}

					// (a.walling 2006-10-31 16:53) - PLID 23299 - Check for -1 here just in case
					if (m_iRow == -1) break;

					if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to delete this task?")) {
						long nTaskID = m_list->GetValue(m_iRow,COLUMN_TASK_ID).lVal;
						// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
						if(IsNexTechInternal()) {
							ExecuteParamSql("DELETE FROM AttendanceToDoLinkT WHERE TodoID = {INT}", nTaskID);
						}
						// (c.haag 2008-06-09 17:11) - PLID 30328 - Use global utilities to delete todos
						TodoDelete(nTaskID);

						//for audit
						CString strOld = CString(m_list->GetValue(m_iRow, COLUMN_NOTES).bstrVal);

						//auditing
						long nCurSel = m_list->GetCurSel();
						CString strAssignTo(""), strCategory(""), strAssignToIDs("");// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
						// (c.haag 2008-06-10 15:26) - PLID 11599 - We can now just get the assign to names from the list
						strAssignTo = VarString(m_list->GetValue(nCurSel,COLUMN_ASSIGN_TO_NAMES));
						

						_RecordsetPtr prsCategory = CreateRecordset("SELECT ID, Description AS Text "
							"FROM NoteCatsF WHERE ID = %li ORDER BY Text", VarLong(m_list->GetValue(nCurSel,COLUMN_CATEGORY_ID),-1));
						if (!prsCategory->eof)
							strCategory = AdoFldString(prsCategory, "Text", "");
						CString strTemp;
						strTemp.Format("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", strAssignTo, FormatDateTimeForInterface(m_list->GetValue(nCurSel, COLUMN_COMPLETE_BY), 0, dtoDate), strCategory, strOld);
						long nAuditID = -1;
						nAuditID = BeginNewAuditEvent();
						if(nAuditID != -1)
							AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactTodoTaskDelete, GetActiveContactID(), strTemp, "<Deleted>", aepMedium, aetDeleted);

						
						strAssignToIDs = VarString(m_list->GetValue(nCurSel, COLUMN_ASSIGN_TO_IDS));
						CArray < long, long> arrAssignedID;
						ParseDelimitedStringToLongArray(strAssignToIDs, " ", arrAssignedID);

						if (arrAssignedID.GetSize() == 1)
							CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), arrAssignedID[0], TableCheckerDetailIndex::tddisDeleted);
						else {// more than one User Assigned send the regular TableChecker
							CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisDeleted);
						}





						// (c.haag 2003-11-26 15:08) - NOW delete the row!
						m_list->RemoveRow(m_iRow);

						
					}
				}
			}NxCatchAll("Error in CContactToDo::OnCommand");
		break;
		case ID_MARKDONE:
			try {
				// TES 6/17/2003 - Make sure we have permission to modify this task
				// (c.haag 2008-07-03 12:23) - PLID 30615 - This is now done in a member utility function
				if (!CheckAssignToPermissions(m_list->GetCurSel(), sptWrite)) {
					break;
				}

				long nTaskID = VarLong(m_list->GetValue(m_iRow,COLUMN_TASK_ID));
				ExecuteSql("UPDATE ToDoList SET Done = GetDate() WHERE TaskID = %li", nTaskID);
				// (s.tullis 2014-08-21 10:09) 63674- Changed to Support Ex Todo
				
				CString strAssignedIDs = VarString(m_list->GetValue(m_iRow, COLUMN_ASSIGN_TO_IDS));
				//if the completed radio button is checked then we need to remove this item from this list 
				if (m_unfinished.GetCheck()) {
					m_list->RemoveRow(m_iRow);
				}
				else if (m_all.GetCheck()) {
					COleDateTime date;
					date = COleDateTime::GetCurrentTime();
					m_list->PutValue(m_iRow, COLUMN_COMPLETE_DATE, _variant_t(FormatDateTimeForInterface(date, DTF_STRIP_SECONDS, dtoDate)));
					m_list->SetRedraw(TRUE);
				}	

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactFollowupComplete, GetActiveContactID(), "Uncompleted", "Completed", aepMedium, aetChanged);

				// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
				
				CArray < long, long> arrAssignedID;
				ParseDelimitedStringToLongArray(strAssignedIDs, " ", arrAssignedID);

				if (arrAssignedID.GetSize() == 1)
					CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
				else {// more than one User Assigned send the regular TableChecker
					CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisChanged);
				}
				

			}NxCatchAll("Error in CContactToDo::OnCommand \n Cannot change Completion Status");
		break;
		case ID_MARKNOTDONE:
			try {
				// TES 6/17/2003 - Make sure we have permission to modify this task
				// (c.haag 2008-07-03 12:23) - PLID 30615 - This is now done in a member utility function
				if (!CheckAssignToPermissions(m_list->GetCurSel(), sptWrite)) {
					break;
				}

				// (a.walling 2006-10-31 17:04) - PLID 23299 - Check for invalid row before using VarLong
				if (m_iRow == -1) break;
				long nTaskID = VarLong(m_list->GetValue(m_iRow,COLUMN_TASK_ID));
				ExecuteSql("UPDATE ToDoList SET ToDoList.Done = NULL WHERE TaskID = %li", nTaskID);

				// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
				CString strAssignedIDs = VarString(m_list->GetValue(m_iRow, COLUMN_ASSIGN_TO_IDS));
				CArray<long, long> anAssignTo;
				ParseDelimitedStringToLongArray(strAssignedIDs, " ", anAssignTo);
				if (anAssignTo.GetSize() == 1)
					CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), anAssignTo[0], TableCheckerDetailIndex::tddisChanged);
				else {// more than one User Assigned send the regular TableChecker
					CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisChanged);
				}


				//if the incompleted radio button is selected then by marking this task completed, we need to remove
				//the item from this list
				if (m_finished.GetCheck()) {
					m_list->RemoveRow(m_iRow);
				}
				else if (m_all.GetCheck()) {
					_variant_t var;
					var.vt = VT_NULL;
					m_list->PutValue(m_iRow, COLUMN_COMPLETE_DATE, var);
					m_list->SetRedraw(TRUE);
				}

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactFollowupUncomplete, GetActiveContactID(), "Completed", "Uncompleted", aepMedium, aetChanged);



			}NxCatchAll("Error in CContactToDo::OnCommand \n Cannot Change Completion Status");
		break;
		case ID_MODIFY:
			OnModifyItem();			
			break;
	}
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CContactTodo::OnRButtonUpListTodo(long nRow, short nCol, long x, long y, long nFlags) 
{
// Build a popup menu to change task characteristics
	CMenu pMenu;
	m_iRow = nRow;
	m_list->PutCurSel(nRow);

	if (nRow == -1)
	{	pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_ADDTASK, "Add Task");
	}
	else
	{
		pMenu.CreatePopupMenu();
		pMenu.InsertMenu(0, MF_BYPOSITION, ID_ADDTASK, "Add Task");
		pMenu.InsertMenu(1, MF_BYPOSITION, ID_MODIFY, "Modify Task");
		pMenu.InsertMenu(2, MF_BYPOSITION, ID_DELETE, "Delete Task");

		// Allow the user to change task completion status
		if (m_list->GetValue(nRow,COLUMN_COMPLETE_DATE).vt == VT_NULL || m_list->GetValue(nRow,COLUMN_COMPLETE_DATE).vt == VT_EMPTY)
			pMenu.InsertMenu(3, MF_BYPOSITION, ID_MARKDONE, "Mark As Completed");
		else pMenu.InsertMenu(3, MF_BYPOSITION, ID_MARKNOTDONE, "Mark As Incomplete");
	}

	CPoint pt;
	pt.x = x;
	pt.y = y;
	CWnd* pwnd = GetDlgItem(IDC_LIST_TODO);
	if (pwnd != NULL) {
		pwnd->ClientToScreen(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
	
}

void CContactTodo::OnLButtonDownListTodo(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-06-30 10:18) - PLID 11599 - Let users edit the todo assignee list of
	// the selected row
	try {
		if(m_list->GetCurSel()==-1)
			return;

		if (COLUMN_ASSIGN_TO_NAMES == nCol) {
			//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
			CArray<long,long> arynAssignTo;
			ParseDelimitedStringToLongArray(
				VarString(m_list->GetValue(nRow,COLUMN_ASSIGN_TO_IDS)), " ", arynAssignTo
			);
			if (!CheckIndividualPermissions(nCol, arynAssignTo)) {
				return;
			}
			//check to see if the user has permission to edit this row
			// (a.walling 2006-10-31 16:53) - PLID 23299 - Only proceed if we get a valid variant
			// (c.haag 2008-07-03 12:23) - PLID 30615 - This is now encapsulated in a member utility function
			if (!CheckAssignToPermissions(nRow, sptWrite)) {
				return;
			}

			const CString strOldIDs = VarString(m_list->GetValue(nRow, COLUMN_ASSIGN_TO_IDS));
			const CString strOldNames = VarString(m_list->GetValue(nRow, COLUMN_ASSIGN_TO_NAMES));
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
					m_list->PutValue(nRow, COLUMN_ASSIGN_TO_IDS, _bstr_t(strNewIDs));
					if (!TodoCheckItemReassignmentPermissions(anOldAssignTo, anNewAssignTo)) {

						// Permissions fail. Undo the change and let the user try again.
						m_list->PutValue(nRow, COLUMN_ASSIGN_TO_IDS, _bstr_t(strOldIDs));
						bRetry = TRUE;
					}
					else {
						const long nTaskID = VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID));

						// (j.jones 2008-12-02 10:18) - PLID 30830 - multiple users per tracking step is now allowed
						/*
						// Permissions check out. Make sure it's not bound to tracking ladders.
						{
							_RecordsetPtr prs = CreateParamRecordset("SELECT RegardingType FROM TodoList WHERE TaskID = {INT}", nTaskID);
							if (!prs->eof) {
								TodoType tt = (TodoType)AdoFldLong(prs, "RegardingType");
								if (ttTrackingStep == tt && anNewAssignTo.GetSize() > 1) {
									AfxMessageBox("You may not assign multiple users to a todo alarm linked to a tracking ladder.", MB_ICONERROR);
									m_list->PutValue(nRow, COLUMN_ASSIGN_TO_IDS, _bstr_t(strOldIDs));
									bRetry = TRUE;
									continue;
								}
							} 
						}
						*/

						// Update the data.
						TodoChangeAssignTo(nTaskID, anNewAssignTo);
						PhaseTracking::SyncLadderWithTodo(nTaskID);

						// Audit the change
						CString strNewNames;
						_RecordsetPtr prs = CreateParamRecordset("SELECT dbo.GetTodoAssignToNamesString({INT}) AS TodoNames ", nTaskID);
						strNewNames = AdoFldString(prs, "TodoNames");
						prs->Close();
						const CString strOld = FormatString("Assigned To: %s", strOldNames);
						const CString strNew = FormatString("Assigned To: %s", strNewNames);
						if (strOld != strNew) {
							long nAuditID = BeginNewAuditEvent();
							AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, nTaskID, strOld, strNew, aepMedium, aetChanged);
						}



						// Now update the names on the screen.
						m_list->PutValue(nRow, COLUMN_ASSIGN_TO_NAMES, _bstr_t(strNewNames));

						// Now send a table checker
						// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
						
						if (anNewAssignTo.GetSize() == 1)
							CClient::RefreshTodoTable(nTaskID, GetActiveContactID() , anNewAssignTo[0], TableCheckerDetailIndex::tddisChanged);
						else {// more than one User Assigned send the regular TableChecker
							CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisChanged);
						}
					}				
				}
				else {
					// The user changed their mind. Don't change anything.
				}
			}
			while (bRetry);

		} // if (COLUMN_ASSIGN_TO_NAMES == nCol) {
	}
	NxCatchAll("Error in CContactTodo::OnLButtonDownListTodo");
}

void CContactTodo::OnEditingFinishingListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		if(*pbCommit == FALSE) {
			return;
		}

		switch (nCol) {
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
			case COLUMN_COMPLETE_BY:  //Deadline Column
			case COLUMN_COMPLETE_DATE:  //Finished Column
				if (pvarNewValue->vt != VT_DATE) {
					if(pvarNewValue->vt == VT_BSTR) {
						if(CString(pvarNewValue->bstrVal) == "") {
							if(nCol == 6) 
								*pbCommit = FALSE;
							else 
								pvarNewValue->vt = VT_NULL;
						}
						else
						{
							COleDateTime dt,dttemp;
							dt.ParseDateTime(CString(pvarNewValue->bstrVal));
							// (a.walling 2007-11-05 16:07) - PLID 27977 - VS2008 - We were comparing a datetime to a bool!!!
							dttemp.ParseDateTime("01/01/1800");
							if(dt.m_status==COleDateTime::invalid || dt < dttemp) {
								AfxMessageBox("You entered an invalid date value.");
								*pbCommit = FALSE;
								*pbContinue = FALSE;
								return;
							}
							pvarNewValue->vt = VT_DATE;
							pvarNewValue->date = dt;
						}
					}
					else {
						AfxMessageBox("You entered an invalid date value.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
				break;
		}
	} NxCatchAll("Error in CContactTodo::OnEditingFinishingListTodo()");
}

void CContactTodo::OnEditingFinishedListTodo(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	long nAuditID = -1;
	CString strNew, strOld;

	// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers

	IRowSettingsPtr pRow;
	pRow = m_list->GetRow(nRow);
	CString strAssignTo="";
	long nTaskID=-1;
	CArray<long, long> anAssignTo;

	if (pRow != NULL){
		
		nTaskID = VarLong(pRow->GetValue( COLUMN_TASK_ID));
		strAssignTo = VarString(pRow->GetValue( COLUMN_ASSIGN_TO_IDS));
		
		  ParseDelimitedStringToLongArray(strAssignTo, " ", anAssignTo);
	}
	else{
		return;
	}
	
	
	switch (nCol) {
		case COLUMN_CATEGORY_ID:		//category
			if(bCommit) {
				try {
					if(varNewValue.vt == VT_I4) {
						CString strID;
						if(varNewValue.lVal == -1)
							strID = "NULL";
						else
							strID.Format("%li",varNewValue.lVal);
						ExecuteSql("Update ToDoList Set CategoryID = %s WHERE TaskID = %li", strID, VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
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
						AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)), strOld, strNew, aepMedium, aetChanged);
				}NxCatchAll("Error in CFollowUpDlg::OnEditingFinishedListTodo");
			}
			break;
		case COLUMN_TASK:		//method
			if(bCommit) {
				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Get old value for auditing
				_RecordsetPtr rsMethodTypeOld = CreateRecordset("SELECT Task FROM ToDoList "
															"WHERE TaskID = %li", VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
				if (!rsMethodTypeOld->eof) {
					strOld.Format("Method: %s", AdoFldString(rsMethodTypeOld, "Task", ""));
					if (Trim(AdoFldString(rsMethodTypeOld, "Task", "")) == "") {
						strOld.Format("Method: <none>");
					}
				}
				else {
					strOld.Format("Method: <none>");
				}

				ExecuteSql("UPDATE ToDoList SET Task = '%s' WHERE TaskID = %li", _Q(VarString(varNewValue)), VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
				
				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Audit new value
				strNew.Format("Method: %s", _Q(VarString(varNewValue)));

				nAuditID = BeginNewAuditEvent();
				if (strOld != strNew) {
					AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)), strOld, strNew, aepMedium, aetChanged);
				}

			}
			break;
		case COLUMN_PRIORITY:		//priority
			if(bCommit)
			{
				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Get old value for auditing
				_RecordsetPtr rsPriorityOld = CreateRecordset("SELECT Priority FROM ToDoList "
															"WHERE TaskID = %li", VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
				if (!rsPriorityOld->eof) {
					strOld.Format("Priority: %s", StrPriorityString(AdoFldByte(rsPriorityOld, "Priority", -1)));
				}
				else {
					strOld.Format("Priority: <none>");
				}

				ExecuteSql("UPDATE ToDoList SET Priority = %li WHERE TaskID = %li", VarShort(varNewValue), VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
				ColorizeList(nRow);

				//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
				// Audit new value
				strNew.Format("Priority: %s", _Q(StrPriorityString(VarShort(varNewValue))));
				nAuditID = BeginNewAuditEvent();
				if (strOld != strNew) {
					AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)), strOld, strNew, aepMedium, aetChanged);
				}
			}
			break;	
		case COLUMN_NOTES:		//Notes
			try {
				if (bCommit) {
					ExecuteSql("Update ToDoList Set Notes = '%s' WHERE TaskID = %li", _Q(VarString(varNewValue)), VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
					//auditing
					nAuditID = BeginNewAuditEvent();
					strOld.Format("Note: %s", VarString(varOldValue));
					strNew.Format("Note: %s", VarString(varNewValue));
					if (strOld != strNew)
						AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)), strOld, strNew, aepMedium, aetChanged);
				}
			}NxCatchAll("Error in CContactToDo::OnEditingFinishedNxdlnotes");
			break;
			// (c.haag 2008-06-10 15:42) - PLID 11599 - This is no longer an editable column
			/*
		case 5:		//assigned to
			if(bCommit)
				if(varNewValue.vt == VT_I4) {
					// (c.haag 2008-06-09 12:12) - PLID 30321 - We now use a global function to change the assignee
					TodoChangeAssignTo(VarLong(m_list->GetValue(nRow, 0)), VarLong(varNewValue));
					//for auditing
					_RecordsetPtr rsAssignToNew = CreateRecordset("SELECT UserName FROM UsersT "
															"WHERE PersonID = %li", VarLong(varNewValue));
					if (!rsAssignToNew->eof)
						strNew.Format("Assigned To: %s", AdoFldString(rsAssignToNew, "UserName", ""));
				}
				//auditing
				if(varOldValue.vt == VT_I4) {
					_RecordsetPtr rsAssignToOld = CreateRecordset("SELECT UserName FROM UsersT "
															"WHERE PersonID = %li", VarLong(varOldValue));
					if (!rsAssignToOld->eof)
						strOld.Format("Assigned To: %s", AdoFldString(rsAssignToOld, "UserName", ""));
				}
				nAuditID = BeginNewAuditEvent();
				if (strOld != strNew)
					AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, VarLong(m_list->GetValue(nRow, 0)), strOld, strNew, aepMedium, aetChanged);
			break;*/
		case COLUMN_COMPLETE_BY:		//deadline
			try {
				if (bCommit) {
					ExecuteSql("Update ToDoList Set Deadline = '%s' WHERE TaskID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
					//auditing
					nAuditID = BeginNewAuditEvent();
					strOld.Format("Deadline: %s", FormatDateTimeForInterface(VarDateTime(varOldValue), dtoDate));
					strNew.Format("Deadline: %s", FormatDateTimeForInterface(VarDateTime(varNewValue), dtoDate));
					if (strOld != strNew)
						AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactToDoChanged, VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)), strOld, strNew, aepMedium, aetChanged);
				}
			}NxCatchAll("Error in CContactToDo::OnEditingFinishedNxdlnotes");
			break;
		case COLUMN_COMPLETE_DATE:		//done
			try {
				if (bCommit) {
					if(varNewValue.vt == VT_NULL) {
						//This will now not be done.
						ExecuteSql("Update ToDoList SET Done = NULL WHERE TaskID = %li", VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
						if (m_finished.GetCheck()) {
							

							m_list->RemoveRow(nRow);
							
						}
					}
					else {
						//This is now done.
						ExecuteSql("Update ToDoList SET Done = '%s' WHERE TaskID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), VarLong(m_list->GetValue(nRow, COLUMN_TASK_ID)));
						if (m_unfinished.GetCheck()) {
							
							
							m_list->RemoveRow(nRow);
							
						}
					}
				}
			}NxCatchAll("Error in CContactToDo::OnEditingFinishedNxdlnotes");
			break;
	}	
	// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers
	if (anAssignTo.GetSize() == 1)
		CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), anAssignTo[0], TableCheckerDetailIndex::tddisChanged, FALSE);
	else {// more than one User Assigned send the regular TableChecker
		CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisChanged,FALSE);
	}
	
	
}

void CContactTodo::OnCreateTodo() 
{
	try {
		CTaskEditDlg dlg(this);
		dlg.m_isPatient = false;	//we're looking at a contact
		dlg.m_nPersonID = GetActiveContactID(); // (a.walling 2008-07-07 17:50) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = TRUE;
		long nResult = dlg.DoModal(0);

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
	
		// (c.haag 2008-06-10 17:29) - PLID 11599 - Changed to use new data structure
		_RecordsetPtr rs = CreateParamRecordset("SELECT ToDoList.PersonID, ToDoList.TaskID, Task, CategoryID, ToDoList.Priority, ToDoList.Notes, "
			"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
			"Deadline, Done, UsersT.UserName AS EnteredByName "
			"FROM ToDoList "
			"LEFT JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
			"WHERE TaskID = {INT}", dlg.m_iTaskID);
		IRowSettingsPtr pRow;
		FieldsPtr fields;
		fields  = rs->Fields;
		pRow = m_list->GetRow(-1);
		pRow->PutValue(COLUMN_TASK_ID, dlg.m_iTaskID);
		pRow->PutValue(COLUMN_CATEGORY_ID, fields->Item["CategoryID"]->Value);
		pRow->PutValue(COLUMN_TASK, fields->Item["Task"]->Value);
				//for some reason this field is not being read correctly as a VT_UI1, so we have to manually change it to a VT_I2
		_variant_t var = fields->Item["Priority"]->Value;
		var.vt = VT_I2;
//		pRow->PutValue(3, fields->Item["Priority"]->Value);
		pRow->PutValue(COLUMN_PRIORITY, var);

		pRow->PutValue(COLUMN_NOTES, fields->Item["Notes"]->Value);
		pRow->PutValue(COLUMN_ASSIGN_TO_NAMES, fields->Item["AssignedToNames"]->Value);
		pRow->PutValue(COLUMN_ASSIGN_TO_IDS, fields->Item["AssignedToIDs"]->Value);
		pRow->PutValue(COLUMN_COMPLETE_BY, fields->Item["DeadLine"]->Value);
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
		if((fields->Item["Done"]->Value.vt==VT_NULL && !m_finished.GetCheck()) || (fields->Item["Done"]->Value.vt!=VT_NULL && !m_unfinished.GetCheck()))
		{
			if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
				ColorizeItem(pRow);
			}
			m_list->InsertRow(pRow, -1);
		}
		/*// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers-- Removed Redundant Tablechecker send
		CString strAssignedIDs = VarString(fields->Item["AssignedToIDs"]->Value);
		CArray < long, long> arrAssignedID;
		ParseDelimitedStringToLongArray(strAssignedIDs, ",", arrAssignedID);

		if (arrAssignedID.GetSize() == 1)
			CClient::RefreshTodoTable(dlg.m_iTaskID, GetActiveContactID(), arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
		else {// more than one User Assigned send the regular TableChecker
			CClient::RefreshTodoTable(dlg.m_iTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisChanged);
		}*/
	}NxCatchAll("Error in CContactToDo::OnClickBtnTodo() ");	
}

void CContactTodo::OnView() 
{
	CMainFrame *pMainFrame = GetMainFrame();
	if (pMainFrame->GetSafeHwnd()) {
		pMainFrame->ShowTodoList();
		pMainFrame->SelectTodoListUser(m_id);
	}
}

void CContactTodo::OnDelete() 
{
	bool bDeleteSelf = false;
	bool bDeleteOthers = false;

	if(GetCurrentUserPermissions(bioSelfFollowUps) & (SPT_____D______ANDPASS)){
		bDeleteSelf = true;
	}
	if(GetCurrentUserPermissions(bioNonSelfFollowUps) & (SPT_____D______ANDPASS)){
		bDeleteOthers = true;
	}

	CString strSql, strMessage, strAuditValue;

	try {
		if(!bDeleteSelf && !bDeleteOthers) {
			// user cannot delete any follow-ups
			AfxMessageBox("You do not have permission to delete any follow-ups.");
			return;
		}
		else if(bDeleteSelf && !bDeleteOthers) {
			// user can delete own follow-ups but not others
			// (c.haag 2008-06-13 11:10) - PLID 11599 - Use new todo structure. If you do not have permission to delete
			// others' todo alarms, then you can only delete finished todos assigned to you (even if they are also assigned
			// to others)
			_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(TaskID) AS NumItems FROM ToDoList "
				"WHERE (Done IS NOT NULL) AND PersonID = {INT} "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = {INT})" // Assigned to you
				, m_id, GetCurrentUserID(), GetCurrentUserID());
			long nCount = AdoFldLong(rsCount, "NumItems");
			if(nCount == 0) {
				MsgBox("You only have permission to delete your own follow-ups, but you have no completed follow-ups for this contact assigned to you.  "
					"No action will be taken.");
				return;
			}
			strMessage.Format("You only have permission to delete your own follow-ups.  Are you sure you wish "
				" to delete these %li follow-up(s)?", nCount);
			// (c.haag 2008-06-13 11:26) - PLID 11599 - This is easier if we just create a temp table with all the tasks to delete
			strSql += FormatString("DECLARE @tblTasks TABLE (ID INT NOT NULL)\r\n"
				"INSERT INTO @tblTasks (ID)\r\n "
				"SELECT TaskID FROM ToDoList\r\n "
				"WHERE (Done IS NOT NULL) AND PersonID = %d\r\n "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo = %d)\r\n" // Assigned to you
				, m_id, GetCurrentUserID(), GetCurrentUserID());
			// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
			if(IsNexTechInternal()) {
				strSql += FormatString("DELETE FROM AttendanceToDoLinkT WHERE TodoID IN (SELECT ID FROM @tblTasks)");
			}
			// (c.haag 2008-06-11 10:52) - PLID 30328 - Use new todo structure
			// (c.haag 2008-07-10 15:30) - PLID 30674 - Delete EMR todo table records
			strSql += FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT ID FROM @tblTasks);\r\n");
			strSql += FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT ID FROM @tblTasks);\r\n");
			strSql += FormatString("DELETE FROM ToDoList WHERE TaskID IN (SELECT ID FROM @tblTasks);\r\n");
			strAuditValue.Format("Cleared Own Items");
		}
		else if(!bDeleteSelf && bDeleteOthers) {
			// user can only delete other's follow-ups
			// (c.haag 2008-06-13 11:29) - PLID 11599 - Use new todo structure. The user can only delete follow-ups
			// assigned to other users (even if the user is also assigned to it)
			_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(TaskID) AS NumItems FROM ToDoList "
				"WHERE (Done IS NOT NULL) AND PersonID = {INT} "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> {INT}) "
				";\r\n", m_id, GetCurrentUserID(), GetCurrentUserID());
			long nCount = AdoFldLong(rsCount, "NumItems");
			if(nCount == 0) {
				MsgBox("You only have permission to delete other users' follow-ups, but no other users have completed follow-ups for this contact which are not assigned to you.  "
					"No action will be taken.");
				return;
			}
			strMessage.Format("You only have permission to delete other users' follow-ups.  Are you sure you wish "
				" to delete these %li follow-up(s)?", nCount);
			// (c.haag 2008-06-13 11:34) - PLID 11599 - This is easier if we just create a temp table with all the tasks to delete
			strSql += FormatString("DECLARE @tblTasks TABLE (ID INT NOT NULL)\r\n"
				"INSERT INTO @tblTasks (ID)\r\n "
				"SELECT TaskID FROM ToDoList\r\n "
				"WHERE (Done IS NOT NULL) AND PersonID = %d\r\n "
				"AND TaskID IN (SELECT TaskID FROM TodoAssignToT WHERE AssignTo <> %d)\r\n" // Assigned to other users
				, m_id, GetCurrentUserID(), GetCurrentUserID());
			// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
			if(IsNexTechInternal()) {
				strSql += FormatString("DELETE FROM AttendanceToDoLinkT WHERE TodoID IN (SELECT ID FROM @tblTasks);\r\n");
			}
			// (c.haag 2008-06-11 10:52) - PLID 30328 - Use new todo structure
			// (c.haag 2008-07-10 15:30) - PLID 30674 - Delete EMR todo table records
			strSql += FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT ID FROM @tblTasks);\r\n");
			strSql += FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT ID FROM @tblTasks);\r\n");
			strSql += FormatString("DELETE FROM ToDoList WHERE TaskID IN (SELECT ID FROM @tblTasks);\r\n");
			strAuditValue.Format("Cleared Others' Items");
		}
		else{
			// the user can delete all follow-ups
			_RecordsetPtr rsCount = CreateRecordset("SELECT Count(TaskID) AS NumItems FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li", m_id);
			long nCount = AdoFldLong(rsCount, "NumItems");
			if(nCount == 0) {
				MsgBox("There are no completed follow-ups for this contact.  No action will be taken.");
				return;
			}
			strMessage.Format("Are you sure you wish to delete the %li completed task(s)?", nCount);
			// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
			if(IsNexTechInternal()) {
				strSql += FormatString("DELETE FROM AttendanceToDoLinkT WHERE TodoID IN (SELECT TaskID FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li);\r\n", m_id);
			}
			// (c.haag 2008-06-11 10:52) - PLID 30328 - Use new todo structure
			// (c.haag 2008-07-10 15:30) - PLID 30674 - Delete EMR todo table records
			strSql += FormatString("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li);\r\n", m_id);
			strSql += FormatString("DELETE FROM EMRTodosT WHERE TaskID IN (SELECT TaskID FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li);\r\n", m_id);
			strSql += FormatString("DELETE FROM ToDoList WHERE (Done IS NOT NULL) AND PersonID = %li;\r\n", m_id);
			strAuditValue.Format("Cleared All Items");
		}
		
		if (IDNO == MessageBox(strMessage, "NexTech", MB_YESNO))
			return;

	
		ExecuteSqlStd(strSql);

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, GetActiveContactName(), nAuditID, aeiContactFollowupClear, GetActiveContactID(), "", strAuditValue, aepMedium, aetDeleted);
		
		m_list->Requery();

	}NxCatchAll("Error in CFollowUpDlg::OnClickBtnDeleteDone");	
}

void CContactTodo::OnEdit() 
{
	CNoteCategories	dlg(this);
	dlg.DoModal();
	
	m_list->Requery();
}

LRESULT CContactTodo::OnTableChangedEx(WPARAM wParam, LPARAM lParam){

	try {

		CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
		switch (wParam)
		{

		
		case NetUtils::TodoList:

			ReflectchangedTodo(pDetails);
			break;
		default:
			break;
		}


	}NxCatchAll(__FUNCTION__)


		return 0;

}



void CContactTodo::ReflectchangedTodo(CTableCheckerDetails* pDetails){


	try{
		long nTaskID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTaskID), -1);
		long nPersonID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiPersonID), -1);

		long nAssignedID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiAssignedUser), -1);


		TableCheckerDetailIndex::Todo_Status todoStatus = (TableCheckerDetailIndex::Todo_Status)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Todos_DetailIndex::tddiTodoStatus), -1);

		NXDATALISTLib::IRowSettingsPtr pRow;

	
		// not current patient.. return 
		if (m_id != nPersonID){
			return;
		}

		if (m_list->IsRequerying()){
			return;
		}

		// if the todo was deleted

		long nRow = m_list->FindByColumn(COLUMN_TASK_ID, nTaskID, -1, VARIANT_FALSE);

		if (nRow == -1){

			pRow = m_list->GetRow(-1);

		}
		else{
			pRow = m_list->GetRow(nRow);
		}

		if (TableCheckerDetailIndex::tddisDeleted == todoStatus){

			if (nRow == -1){

				// not in the list don't need to worry about removing it
			}
			else{

				m_list->RemoveRow(nRow);
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
			pRow = m_list->GetRow(-1);
			pRow->PutValue(COLUMN_TASK_ID, nTaskID);
			PopulateListRow(pRow, fields);

			if ((fields->Item["Done"]->Value.vt == VT_NULL && !m_finished.GetCheck()) || (fields->Item["Done"]->Value.vt != VT_NULL && !m_unfinished.GetCheck()))
			{
				if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
					ColorizeItem(pRow);
				}

			}

			if (pRow != NULL){
				if (fields->Item["Done"]->Value.vt == VT_NULL){
					//if the radio button is set to uncompleted or all we can show this todo
					if (!m_finished.GetCheck()){

						// not in the list going to need to add the todo
						if (nRow == -1){
							m_list->AddRow(pRow);
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
							m_list->RemoveRow(nRow);
						}
					}
				}// we have a todo that has been completed 
				else if ((fields->Item["Done"]->Value.vt != VT_NULL)){
					//check to see that were not showing only uncompleted todos
					if (!m_unfinished.GetCheck()){
						// not in the list going to need to add the todo
						if (nRow == -1){
							m_list->AddRow(pRow);
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
							m_list->RemoveRow(nRow);
						}
					}


				}


			}


		}




	}NxCatchAll(__FUNCTION__)






}

LRESULT CContactTodo::OnTableChanged(WPARAM wParam, LPARAM lParam) {
	try {

		if (wParam == NetUtils::TodoList) {
			try {
				//the TaskID is in the lParam, so get that first
				long nTaskID = lParam;

				if (m_unfinished.GetCheck()) {

					//Get the item out of the data and either update, delete, or add it to the datalist
					// (c.haag 2008-06-10 15:33) - PLID 11599 - Use the new todo assign structure
					_RecordsetPtr rsTask = CreateParamRecordset("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, UsersT.UserName AS EnteredByName, "
						"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
						"UsersT.UserName AS AssignedToName, ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
						"FROM ToDoList "
						"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
						"WHERE (TaskID = {INT}) AND (Done IS NULL) AND (ToDoList.PersonID = {INT}) ", nTaskID, GetActiveContactID());
					FieldsPtr fields = rsTask->Fields;

					if (rsTask->eof) {

						//the task is not in the datalist at the moment and shouldn't be, try to remove it if it is there
						long nRow = m_list->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow != -1) {

						//it is there, so remove
						m_list->RemoveRow(nRow);

						}
						else {
						
							//it is not there and it shouldn't be there, so we are all good
						}

					}
					else {

				
						//there is something in the recordset, so we have to determine whether to add it or just update it
						long nRow = m_list->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow == -1) {

							//the row is not there, so just add it
							IRowSettingsPtr pRow = m_list->GetRow(-1);

							pRow->PutValue(COLUMN_TASK_ID, nTaskID);
							PopulateListRow(pRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
								ColorizeItem(pRow);
							}
							//add the row to the datalist	
							m_list->AddRow(pRow);
						}
						else {

							//the row is already there, we just have to update the values in it
							PopulateListRow(nRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							ColorizeList(nRow);
						}
						
					}
				}else if (m_finished.GetCheck() ) {
					//Get the item out of the data and either update, delete, or add it to the datalist
					// (c.haag 2008-06-10 15:33) - PLID 11599 - Use the new todo assign structure
					_RecordsetPtr rsTask = CreateParamRecordset("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, UsersT.UserName AS EnteredByName, "
						"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
						"UsersT.UserName AS AssignedToName, ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
						"FROM ToDoList "
						"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
						"WHERE (TaskID = {INT}) AND (Done IS NOT NULL) AND (ToDoList.PersonID = {INT}) ", nTaskID, GetActiveContactID());
					FieldsPtr fields = rsTask->Fields;

					if (rsTask->eof) {

						//the task shouldn't be in the data list, try to remove it if it is there
						long nRow = m_list->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow != -1) {

							//it is there, so remove
							m_list->RemoveRow(nRow);

						}
						else {
						
							//it is not there and it shouldn't be there, so we are all good
						}

					}
					else {

				
						//there is something in the recordset, so we have to determine whether to add it or just update it
						long nRow = m_list->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow == -1) {

							//the row is not there, so just add it
							IRowSettingsPtr pRow = m_list->GetRow(-1);

							pRow->PutValue(COLUMN_TASK_ID, nTaskID);
							PopulateListRow(pRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
								ColorizeItem(pRow);
							}

							//add the row to the datalist	
							m_list->AddRow(pRow);
						}
						else {

							//the row is already there, we just have to update the values in it
							PopulateListRow(nRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							ColorizeList(nRow);

						}
						
					}
				}
				else { //we are showing all

					
					//Get the item out of the data and either update, delete, or add it to the datalist
					// (c.haag 2008-06-10 15:33) - PLID 11599 - Use the new todo assign structure
					_RecordsetPtr rsTask = CreateParamRecordset("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, UsersT.UserName AS EnteredByName, "
						"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
						"UsersT.UserName AS AssignedToName, ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
						"FROM ToDoList "
						"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
						"WHERE (TaskID = {INT}) AND (ToDoList.PersonID = {INT}) ", nTaskID, GetActiveContactID());
					FieldsPtr fields = rsTask->Fields;

					if (rsTask->eof) {

						//the task shouldn't be in the data list, try to remove it if it is there
						long nRow = m_list->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow != -1) {

							//it is there, so remove
							m_list->RemoveRow(nRow);

						}
						else {
						
							//it is not there and it shouldn't be there, so we are all good
						}

					}
					else {

				
						//there is something in the recordset, so we have to determine whether to add it or just update it
						long nRow = m_list->FindByColumn(0, nTaskID, -1, FALSE);

						if (nRow == -1) {

							//the row is not there, so just add it
							IRowSettingsPtr pRow = m_list->GetRow(-1);

							pRow->PutValue(COLUMN_TASK_ID, nTaskID);
							PopulateListRow(pRow, fields);

							// (a.walling 2006-09-11 12:28) - PLID 20419 - Colorize items added via table checker
							if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1) { // color coding is enabled
								ColorizeItem(pRow);
							}

							//add the row to the datalist
							m_list->AddRow(pRow);
						}
						else {

							//the row is already there, we just have to update the values in it
							m_list->PutValue(nRow, COLUMN_TASK_ID, nTaskID);
							PopulateListRow(nRow, fields);

							ColorizeList(nRow);							
						}
						
					}
				}
			}NxCatchAll("Error in CContactTodo::OnTableChanged:Todolist");
		}
		else if(wParam == NetUtils::NoteCatsF) {
			try {
				//requery the combo source
				IColumnSettingsPtr pCol = m_list->GetColumn(1);
				pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT -1, '{No Category}' ORDER BY Description"));
			} NxCatchAll("Error in CContactTodo::OnTableChanged:NoteCatsF");
		}

	} NxCatchAll("Error in CContactTodo::OnTableChanged");

	return 0;

}

void CContactTodo::OnEditingStartingListTodo(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	if(nRow != -1) {

		//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
		CArray<long,long> arynAssignTo;
		ParseDelimitedStringToLongArray(
			VarString(m_list->GetValue(nRow,COLUMN_ASSIGN_TO_IDS)), " ", arynAssignTo
		);
		if (!CheckIndividualPermissions(nCol, arynAssignTo)) {
			*pbContinue = FALSE;
		}

		// (c.haag 2008-07-03 12:23) - PLID 30615 - Check for permission in a member utility function
		if (!CheckAssignToPermissions(nRow, sptWrite)) {
			*pbContinue = FALSE;
		}
	}
}

void CContactTodo::OnDblClickCellListTodo(long nRowIndex, short nColIndex) 
{
	if(nRowIndex != -1) {
		m_iRow = nRowIndex;
		OnModifyItem();
	}
}

void CContactTodo::OnModifyItem()
{
	try {
		// TES 6/17/2003 - Make sure we have permission to modify this task
		// (c.haag 2008-07-03 12:23) - PLID 30615 - This is now done in a member utility function
		if (!CheckAssignToPermissions(m_list->GetCurSel(), sptWrite)) {
			return;
		}
		
		long nTaskID = VarLong(m_list->GetValue(m_iRow, COLUMN_TASK_ID));
		CTaskEditDlg dlg(this);
		dlg.m_iTaskID = nTaskID;
		dlg.m_nPersonID = GetActiveContactID(); // (a.walling 2008-07-07 17:50) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = FALSE;

		if(dlg.DoModal(false)==IDCANCEL)
			return;
		_RecordsetPtr  rs;
		
		// (c.haag 2008-06-10 15:36) - PLID 11599 - Changed the query to use the new to do list multi-assignee structure
		rs = CreateParamRecordset("SELECT ToDoList.TaskID, ToDoList.Done, ToDoList.Deadline, UsersT.UserName AS EnteredByName, "
		"dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedToNames, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, "
		"ToDoList.Notes, ToDoList.Priority, ToDoList.CategoryID, ToDoList.Task "
		"FROM ToDoList "
		"LEFT OUTER JOIN UsersT ON ToDoList.EnteredBy = UsersT.PersonID "
		"WHERE TaskID = {INT}", dlg.m_iTaskID);
		if(rs->eof) {
			rs->Close();
			m_list->RemoveRow(m_iRow);
			return;
		}
		FieldsPtr fields;
		fields  = rs->Fields;

	/*	// (s.tullis 2014-09-15 09:17) - PLID 63674 - Support for Ex Todo tablecheckers--removed Redundant Tablechecker send
		CString strAssignedIDs = VarString(fields->Item["AssignedToIDs"]->Value);
		CArray<long, long> anAssignTo;
		ParseDelimitedStringToLongArray(strAssignedIDs, " ", anAssignTo);
		if (anAssignTo.GetSize() == 1)
			CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), anAssignTo[0], TableCheckerDetailIndex::tddisChanged);
		else {// more than one User Assigned send the regular TableChecker
			CClient::RefreshTodoTable(nTaskID, GetActiveContactID(), -1, TableCheckerDetailIndex::tddisChanged);
		}*/
		if (m_unfinished.GetCheck()) {
			if (fields->Item["Done"]->Value.vt == VT_NULL) {
				//m_list->SetRedraw(FALSE);
				PopulateListRow(m_iRow, fields);
				m_list->SetRedraw(TRUE);
			}
			else {
				m_list->RemoveRow(m_iRow);
			}
		}
		else if (m_finished.GetCheck()) {
			if (fields->Item["Done"]->Value.vt == VT_NULL) {
				m_list->RemoveRow(m_iRow);
			}
			else {
				//m_list->SetRedraw(FALSE);
				PopulateListRow(m_iRow, fields);
				m_list->SetRedraw(TRUE);
			}
		}

		//update the color
		ColorizeList(m_iRow);

		
	}NxCatchAll("Error in CContactToDo::OnCommand - Modify");
}
void CContactTodo::OnContactsFollowupShowGrid() 
{
	try {
		if(IsDlgButtonChecked(IDC_CONTACTS_FOLLOWUP_SHOW_GRID)) {
			SetRemotePropertyInt("ContactsFollowupShowGrid", 1, 0, GetCurrentUserName());
			m_list->GridVisible = true;
		}
		else {
			SetRemotePropertyInt("ContactsFollowupShowGrid", 0, 0, GetCurrentUserName());
			m_list->GridVisible = false;
		}
	}NxCatchAll("Error in CContactTodo::OnContactsFollowupShowGrid()");
}

bool CContactTodo::UnfinishedChecked()
{
	return m_unfinished.GetCheck() == BST_CHECKED;
}

bool CContactTodo::FinishedChecked()
{
	return m_finished.GetCheck() == BST_CHECKED;
}

void CContactTodo::ColorizeList(OPTIONAL long nRow /* = -1 */)
{
	const int nPriorityAdj = 10;

	try {
		if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 0) { // color coding is disabled
			return;
		}

		if (nRow < 0) { // color the whole list
			for (int i = 0; i < m_list->GetRowCount(); i++) {
				IRowSettingsPtr pRow = m_list->GetRow(i);
				if (pRow) {
					ColorizeItem(pRow);
				}
			}
		}
		else // just this row...
		{
			IRowSettingsPtr pRow = m_list->GetRow(nRow);
			if (pRow) {
				ColorizeItem(pRow);
			}
		}
	} NxCatchAll("Error in CContactTodo::ColorizeList");
}

void CContactTodo::ColorizeItem(IRowSettingsPtr &pRow)
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
	} NxCatchAll("Error in CContactTodo::ColorizeItem");
}

void CContactTodo::OnRequeryFinishedListTodo(short nFlags) 
{
	try {
		ColorizeList();
	} NxCatchAll("Error changing colors");
}

void CContactTodo::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
}

//(c.copits 2011-02-22) PLID 40794 - Permissions for individual todo alarm fields
BOOL CContactTodo::CheckIndividualPermissions(short nCol, CArray<long,long> &arynAssignTo)
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
				if (nCol == COLUMN_CATEGORY_ID) {
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
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsAssignTo, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_ASSIGN_TO_NAMES) {
					bValid = FALSE;
				}
			}

		}
	// Permission if assigned to other and have other write permissions
	if (bRemoteAssign && CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_CATEGORY_ID) {
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
			if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsAssignTo, sptWrite, FALSE, 0, TRUE)) {
				if (nCol == COLUMN_ASSIGN_TO_NAMES) {
					bValid = FALSE;
				}
			}
	}

	return bValid;
}