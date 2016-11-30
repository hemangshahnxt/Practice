// TaskEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TaskEditDlg.h"
#include "MainFrm.h"
#include "MarketUtils.h"
#include "Client.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "TodoUtils.h"
#include "MultiSelectDlg.h"
#include "practicerc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



typedef enum {
	eclat_ID = 0,
	eclat_Username = 1
} EColAssignTo;

/////////////////////////////////////////////////////////////////////////////
// CTaskEditDlg dialog

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
// (a.walling 2008-07-07 17:47) - PLID 29900 - Use m_nPersonID instead of GetActive[Patient,Contact][Name,ID]

CTaskEditDlg::CTaskEditDlg(CWnd* pParent)
	: CNxDialog(CTaskEditDlg::IDD, pParent), m_tblCheckTask(NetUtils::TodoList)
{
	//{{AFX_DATA_INIT(CTaskEditDlg)
		m_iTaskID = -1;
	//}}AFX_DATA_INIT
		m_strDefaultNote.Empty();
		m_nDefaultCategoryID = -1;

		// (m.hancock 2006-10-26 17:00) - PLID 21730 - Set default deadline and reminding date to an invalid date.
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		COleDateTime m_dtDefaultDeadline = dtInvalid;
		COleDateTime m_dtDefaultRemindingDate = dtInvalid;
		m_bUseDefaultDeadline = FALSE;
		m_bUseDefaultRemindingDate = FALSE;

		m_nPersonID = -1;
		m_isPatient = true; // (a.walling 2008-07-07 17:51) - PLID 29900 - this was never initialized! though it was set in DoModal

		m_bReadOnly = FALSE; // (c.haag 2008-07-08 09:07) - PLID 30641

		m_RegardingType = ttPatientContact; // (c.haag 2008-07-11 09:55) - PLID 30550
		m_bInvokedFromEMN = FALSE;
		m_nEMNID = -1;
		m_bWasLinkedToEMNWhenSaved = FALSE;
		m_bWasDeleted = FALSE;
		m_nPatientCoordID = -1;

		// (c.haag 2010-05-21 13:48) - PLID 38827 - Be able to override the regarding type and ID
		m_nRegardingIDOverride = -1;
		m_RegardingTypeOverride = ttPatientContact;
}

// (c.haag 2008-06-02 14:56) - PLID 11599 - Based on the current content of m_anAssignTo,
// update the visibility of the Assign To combo
void CTaskEditDlg::RefreshAssignToCombo()
{
	if (0 == m_anAssignTo.GetSize()) {
		m_user->CurSel = -1;
		GetDlgItem(IDC_STATIC_TODO_ASSIGN_TO_MULTIPLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSIGNTO)->ShowWindow(SW_SHOW);

		// (j.gruber 2010-01-12 14:12) - PLID 20916 - patient coordinator
		if (m_isPatient) {
			if (m_nPatientCoordID == -1) {
				m_stUserDescription.ShowWindow(SW_HIDE);
			}
			else {
				m_stUserDescription.ShowWindow(SW_SHOW);
			}
		}
		else {
			m_stUserDescription.ShowWindow(SW_HIDE);
		}

	}
	else if (1 == m_anAssignTo.GetSize()) {
		m_user->TrySetSelByColumn(eclat_ID, m_anAssignTo[0]);
		GetDlgItem(IDC_STATIC_TODO_ASSIGN_TO_MULTIPLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ASSIGNTO)->ShowWindow(SW_SHOW);

		// (j.gruber 2010-01-12 14:12) - PLID 20916 - patient coordinator
		if (m_isPatient) {
			if (m_nPatientCoordID == -1) {
				m_stUserDescription.ShowWindow(SW_HIDE);
			}
			else {
				m_stUserDescription.ShowWindow(SW_SHOW);
			}
		}
		else {
			m_stUserDescription.ShowWindow(SW_HIDE);
		}
	}
	else {
		// Calculate the Assign To text from data based on the selected values
		_RecordsetPtr prs = CreateRecordset("SELECT Username FROM UsersT WHERE PersonID IN (%s) ORDER BY Username",
			ArrayAsString(m_anAssignTo));
		FieldsPtr f = prs->Fields;
		CString strAssignTo;
		while (!prs->eof) {
			strAssignTo += AdoFldString(f, "UserName") + ", ";
			prs->MoveNext();
		}
		prs->Close();
		strAssignTo = strAssignTo.Left( strAssignTo.GetLength() - 2 );
		m_nxlAssignToMultiple.SetText(strAssignTo);

		// Update the dialog
		_variant_t vNull;
		vNull.vt = VT_NULL;
		m_user->TrySetSelByColumn(eclat_ID, vNull);
		GetDlgItem(IDC_ASSIGNTO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_TODO_ASSIGN_TO_MULTIPLE)->ShowWindow(SW_SHOW);
		m_stUserDescription.ShowWindow(SW_HIDE);
	}
}

// (c.haag 2008-07-03 12:25) - PLID 30615 - This function encapsulates security checking, and saves
// us from having to repeat code.
BOOL CTaskEditDlg::CheckAssignToPermissions(ESecurityPermissionType type)
{
	// (a.walling 2006-10-31 17:24) - PLID 23299 - Still check the permissions even though the assign to is
	//		an inactive user.
	// (c.haag 2008-06-10 10:30) - I updated the logic to keep consistent with Adam's change
	// (c.haag 2008-07-03 11:25) - Need to be careful how we consider permissions now that todos can be
	// assigned to multiple users. Here are the rules:
	//
	// For new tasks:
	//   - If you can't assign a todo to another user, then you will be forbidden from creating a todo
	//   assigned to someone besides you
	// For existing tasks:
	//   - If you can't assign a todo to another user, then you will be forbidden from assigning a user
	//   to this todo if it was previously unassigned. You will also be forbidden from unassigning a user
	//   from this todo if it was previously assigned.
	//
	int i;

	// Handle permissions for new tasks
	if (sptCreate == type) {
		if (0 == m_anAssignTo.GetSize()) {
			// The todo is assgined to nobody -- bad data or a bug entering new tasks!
			return FALSE;
		}
		BOOL bNewSelfAssign = FALSE;
		BOOL bNewRemoteAssign = FALSE;

		for (i=0; i < m_anAssignTo.GetSize(); i++) {
			if (m_anAssignTo[i] == GetCurrentUserID()) {
				bNewSelfAssign = TRUE;
			} else {
				bNewRemoteAssign = TRUE;
			}
		}

		if (bNewSelfAssign && bNewRemoteAssign) {
			// If we get here, the todo is joint owned. Allow the user to create unless
			// they fail both permission tests
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, type) && !CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
			{
				return FALSE;
			}
		}
		else {
			if (bNewSelfAssign)
			{
				if(!CheckCurrentUserPermissions(bioSelfFollowUps, type))
					return FALSE;
			}
			if (bNewRemoteAssign)
			{
				if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
					return FALSE;
			}
		}	
	}
	// Handle permissions for modifying existing tasks
	else if (sptWrite == type) {
		return TodoCheckItemReassignmentPermissions(m_anExistingAssignTo, m_anAssignTo);
	}
	// Handle for deleting tasks. This is just like for creating, except we check
	// the original assignees of the item
	else if (sptDelete == type) {
		BOOL bOldSelfAssign = FALSE;
		BOOL bOldRemoteAssign = FALSE;

		for (i=0; i < m_anExistingAssignTo.GetSize(); i++) {
			if (m_anExistingAssignTo[i] == GetCurrentUserID()) {
				bOldSelfAssign = TRUE;
			} else {
				bOldRemoteAssign = TRUE;
			}
		}

		if (bOldSelfAssign && bOldRemoteAssign) {
			// If we get here, the todo is joint owned. Allow the user to delete unless
			// they fail both permission tests
			if(!CheckCurrentUserPermissions(bioSelfFollowUps, type) && !CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
			{
				return FALSE;
			}
		}
		else {
			if (bOldSelfAssign)
			{
				if(!CheckCurrentUserPermissions(bioSelfFollowUps, type))
					return FALSE;
			}
			if (bOldRemoteAssign)
			{
				if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, type))
					return FALSE;
			}
		}	
	}

	return TRUE;
}

// (c.haag 2008-06-10 12:30) - PLID 11599 - Returns the "Assign To" names
CString CTaskEditDlg::GetAssignToNames()
{
	if (m_anAssignTo.GetSize() > 1) {
		return m_nxlAssignToMultiple.GetText();
	} else {
		return VarString(m_user->GetComboBoxText());
	}
}

void CTaskEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTaskEditDlg)
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDOK, m_okButton);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_EDIT_NOTES, m_editNotes);
	DDX_Control(pDX, IDC_COMPLETED, m_completed);
	DDX_Control(pDX, IDC_DEADLINE, m_deadline);
	DDX_Control(pDX, IDC_START, m_remind);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_bkg1);
	DDX_Control(pDX, IDC_STATIC_TODO_ASSIGN_TO_MULTIPLE, m_nxlAssignToMultiple);
	DDX_Control(pDX, IDC_CHECK_LINK_TASK_WITH_EMN, m_checkLinkTaskWithEMN);
	DDX_Control(pDX, IDC_STATIC_USER_DESC, m_stUserDescription);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CTaskEditDlg, IDC_DEADLINE, 2 /* Change */, OnChangeDeadline, VTS_NONE)
//	ON_EVENT(CTaskEditDlg, IDC_START, 2 /* Change */, OnChangeStart, VTS_NONE)

BEGIN_MESSAGE_MAP(CTaskEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTaskEditDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DEADLINE, OnChangeDeadline)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_START, OnChangeStart)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelLButtonDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTaskEditDlg message handlers

BEGIN_EVENTSINK_MAP(CTaskEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTaskEditDlg)
	ON_EVENT(CTaskEditDlg, IDC_PRIORITY, 1 /* SelChanging */, OnSelChangingPriority, VTS_PI4)
	ON_EVENT(CTaskEditDlg, IDC_ASSIGNTO, 1 /* SelChanging */, OnSelChangingAssignto, VTS_PI4)
	ON_EVENT(CTaskEditDlg, IDC_METHOD, 1 /* SelChanging */, OnSelChangingMethod, VTS_PI4)
	ON_EVENT(CTaskEditDlg, IDC_ASSIGNTO, 18 /* RequeryFinished */, OnRequeryFinishedAssignto, VTS_I2)
	ON_EVENT(CTaskEditDlg, IDC_ASSIGNTO, 16 /* SelChosen */, OnSelChosenAssignto, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

int CTaskEditDlg::DoModal(bool setIsPatient/*=true*/) 
{
	m_isPatient = setIsPatient;
	return CNxDialog::DoModal();
}

BOOL CTaskEditDlg::OnInitDialog() 
{
	try {	
		CNxDialog::OnInitDialog();

		//DRT 4/15/2008 - PLID 29663 - Cleanup improper creation of NxIconButtons
		m_deleteButton.AutoSet(NXB_DELETE);
		m_okButton.AutoSet(NXB_OK);
		// (c.haag 2008-04-25 12:28) - PLID 29793 - NxIconified the cancel button
		m_cancelButton.AutoSet(NXB_CANCEL);

		// (c.haag 2008-06-10 11:52) - PLID 11599 - Initialize the new label
		m_nxlAssignToMultiple.SetType(dtsHyperlink);
		m_nxlAssignToMultiple.SetSingleLine(true);
		GetDlgItem(IDC_STATIC_TODO_ASSIGN_TO_MULTIPLE)->GetWindowRect(&m_rcAssignToMultiple);
		ScreenToClient(&m_rcAssignToMultiple);

		// (c.haag 2008-07-11 09:58) - PLID 30550 - If this is a new task invoked from
		// the EMN "New ToDo" button, then show the Link to EMN checkbox
		if (m_iTaskID == -1 && m_bInvokedFromEMN) {
			m_checkLinkTaskWithEMN.ShowWindow(SW_SHOW);
			m_checkLinkTaskWithEMN.SetWindowText(FormatString("Link this task with the EMN '%s'", m_strEMNDescription));
			m_checkLinkTaskWithEMN.SetCheck(1);
		}

		EnsureRemoteData();
		m_user = BindNxDataListCtrl(IDC_ASSIGNTO);
		m_description = BindNxDataListCtrl(IDC_CATEGORY);
		IRowSettingsPtr pRow = m_description->GetRow(-1);
		pRow->PutValue(0, long(-1));
		pRow->PutValue(1, _bstr_t(""));
		m_description->AddRow(pRow);

		m_priority = BindNxDataListCtrl(IDC_PRIORITY, false);
		pRow = m_priority->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Low"));
		m_priority->AddRow(pRow);
		pRow = m_priority->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Medium"));
		m_priority->AddRow(pRow);
		pRow = m_priority->GetRow(-1);
		pRow->PutValue(0, _bstr_t("High"));
		m_priority->AddRow(pRow);
		m_priority->CurSel = 0;

		m_method = BindNxDataListCtrl(IDC_METHOD, false);
		pRow = m_method->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Phone"));
		m_method->AddRow(pRow);
		pRow = m_method->GetRow(-1);
		pRow->PutValue(0, _bstr_t("E-Mail"));
		m_method->AddRow(pRow);
		pRow = m_method->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Fax"));
		m_method->AddRow(pRow);
		pRow = m_method->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Letter"));
		m_method->AddRow(pRow);
		pRow = m_method->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Other"));
		m_method->AddRow(pRow);
		m_method->CurSel = 0;

		m_nxtRemind = BindNxTimeCtrl(this, IDC_REMIND_TIME);

		if (m_iTaskID == -1)//new task
		{	COleVariant var = COleDateTime::GetCurrentTime();
			m_deleteButton.EnableWindow(FALSE);
			m_completed.EnableWindow(FALSE);
			m_completed.SetValue(var);
			m_completed.SetValue(COleVariant());			
			//Set the priority to the default.
			
			// (m.hancock 2006-10-26 17:05) - PLID 21730 - Set default deadline date; otherwise, use today's date.
			if(m_bUseDefaultDeadline)
				m_deadline.SetValue(COleVariant(m_dtDefaultDeadline));
			else
				m_deadline.SetValue(var);

			// (m.hancock 2006-10-26 17:05) - PLID 21730 - Set default reminding date; otherwise, use today's date.
			if(m_bUseDefaultRemindingDate)
				m_remind.SetValue(COleVariant(m_dtDefaultRemindingDate));
			else
				m_remind.SetValue(var);

			//if it's a new item, just check for contact, we don't know the status from here
			//set the color of the NxColor
			if(!m_isPatient) {
				m_bkg1.SetColor(GetNxColor(GNC_CONTACT, 0)); //contact
			}
			else {
				// (b.spivey, May 21, 2012) - PLID 50558 - We use the default patient blue always.
				// (a.walling 2008-07-07 17:46) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
				//_RecordsetPtr rsPat = CreateRecordset("SELECT PatientsT.CurrentStatus FROM PatientsT WHERE PersonID = %li", m_nPersonID);
				//if(rsPat->eof) {
				//	rsPat->Close();
				//	return FALSE;
				//}

				//set the color of the NxColor
				m_bkg1.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));	//patient 
			}

			// check and see if there is a default follow up category
			// (m.hancock 2006-07-12 11:20) - PLID 21353 - Added a member variable to assign a category.
			// This will override the default preference.
			if(m_nDefaultCategoryID > -1)
				m_description->SetSelByColumn(0, (long)m_nDefaultCategoryID);
			else {
				long nDefaultCatID = GetRemotePropertyInt("DefaultFollowUpCatID", NULL, 0, "<None>", TRUE);
				if(nDefaultCatID > 0){
					m_description->SetSelByColumn(0, (long)nDefaultCatID);
				}
			}

			// (m.hancock 2006-07-12 11:26) - PLID 21353 - Set the note if m_strDefaultNote has been set
			if(!m_strDefaultNote.IsEmpty()) {
				m_editNotes.SetWindowText(m_strDefaultNote);
			}

			// (j.gruber 2010-01-12 12:49) - PLID 20916 - need to load patient coord
			if (m_isPatient) {
				_RecordsetPtr rsCoord = CreateParamRecordset("SELECT EmployeeID FROM PatientsT WHERE PersonID = {INT}", m_nPersonID);

				if (! rsCoord->eof) {
					m_nPatientCoordID = AdoFldLong(rsCoord, "EmployeeID", -1);
				}

				if (m_nPatientCoordID == -1) {
					//hide the description
					m_stUserDescription.ShowWindow(SW_HIDE);
				}
				else {
					m_stUserDescription.ShowWindow(SW_SHOW);
				}
			}
			else {
				m_stUserDescription.ShowWindow(SW_HIDE);
			}

			
		}
		else //Load info from an existing task
		{
			COleVariant var;

			// (c.haag 2008-06-10 11:39) - PLID 11599 - Use the new todo assignment structure in the query
			// (c.haag 2008-07-11 09:54) - PLID 30550 - Read in the todo type
			// (j.gruber 2010-01-12 12:45) - PLID 20916 - added patient coordID
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT ToDoList.*, PatientsT.CurrentStatus, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, RegardingType, PatientsT.EmployeeID as PatCoordID "
				"FROM ToDoList "
				"LEFT JOIN PatientsT ON ToDoList.PersonID = PatientsT.PersonID "
				"WHERE TaskID = {INT}", m_iTaskID);
			if(rs->eof) {
				rs->Close();
				return FALSE;
			}

			//set the color of the NxColor
			if(m_isPatient) {
				long nStatus = AdoFldShort(rs, "CurrentStatus",1);
				m_bkg1.SetColor(GetNxColor(GNC_PATIENT_STATUS, nStatus));	//patient 
			}
			else
				m_bkg1.SetColor(GetNxColor(GNC_CONTACT, 0)); //contact

			// (c.haag 2008-07-11 09:54) - PLID 30550 - Regarding type
			m_RegardingType = (TodoType)AdoFldLong(rs, "RegardingType");

			// Category
			var = rs->Fields->Item["CategoryID"]->Value;
			if(var.vt != VT_NULL)
				m_description->SetSelByColumn(0,var);

			// Method
			var = rs->Fields->Item["Task"]->Value;
			if (var.vt == VT_BSTR)
				m_method->SetSelByColumn(0, var);

			// Priority
			var = rs->Fields->Item["Priority"]->Value;
			if (var.vt == VT_I2 || var.vt == VT_UI1) {
				switch(var.iVal) {
				case 1:
					m_priority->CurSel = 2;
					break;
				case 2:
					m_priority->CurSel = 1;
					break;
				default:
					m_priority->CurSel = 0;
					break;
				}
			}

			// Assigned to
			// (c.haag 2008-06-10 11:41) - PLID 11599 - We now update a member array
			// (a.walling 2011-08-09 16:08) - PLID 44739 - Handle unassigned tasks without crashing
			ParseDelimitedStringToLongArray(AdoFldString(rs, "AssignedToIDs", ""), " ", m_anAssignTo);
			// (c.haag 2008-07-03 12:29) - PLID 30615 - Track the original assign to's
			m_anExistingAssignTo.Copy(m_anAssignTo);

			// Target date
			if (rs->Fields->Item["Deadline"]->Value.vt == VT_DATE) {
				COleDateTime dt = rs->Fields->Item["Deadline"]->Value;
				m_deadline.SetValue(rs->Fields->Item["Deadline"]->Value);
		//		m_nxtDeadlineTime.SetDateTime(dt);
			}
			else m_completed.SetValue(COleVariant());

			// Completion date
			if(rs->Fields->Item["Done"]->Value.vt == VT_DATE) {
				m_completed.SetValue(rs->Fields->Item["Done"]->Value);
			}
			else {
				m_completed.SetValue((COleVariant)COleDateTime::GetCurrentTime());
				m_completed.SetValue(COleVariant());
			}

			// Remind by date
			//get just the date
			COleDateTime dtRemind = AdoFldDateTime(rs, "Remind");
			m_remind.SetValue(_variant_t(COleDateTime(dtRemind.GetYear(), dtRemind.GetMonth(), dtRemind.GetDay(), 0, 0, 0)));

			//DRT 3/11/03 - This is shady, but here's the deal:
			//		If the remind time is midnight, we're not going to put anything in the box.  This, of course, 
			//		removes any ability to set a real followup to a time of midnight.  But I think for the general
			//		usage of taks, that is never going to happen.  And on the very very very rare case it actually 
			//		might happen, we can just tell them to set it to 12:01.
			//get just the time
			if(dtRemind.GetHour() != 0 || dtRemind.GetMinute() != 0)
				m_nxtRemind->SetDateTime(COleDateTime(1899,12,30,dtRemind.GetHour(),dtRemind.GetMinute(),0));

			// Notes
			var = rs->Fields->Item["Notes"]->Value;
			if (var.vt == VT_BSTR)
				m_editNotes.SetWindowText(CString(var.bstrVal));

			// (j.gruber 2010-01-12 12:46) - PLID 20916 - patcoordID
			if (m_isPatient) {
				m_nPatientCoordID = AdoFldLong(rs, "PatCoordID", -1);
				if (m_nPatientCoordID == -1) {
					//hide the description
					m_stUserDescription.ShowWindow(SW_HIDE);
				}
				else {
					m_stUserDescription.ShowWindow(SW_SHOW);
				}

			}
			else {
				m_stUserDescription.ShowWindow(SW_HIDE);
			}

			rs->Close();

		}//end else

		// (c.haag 2008-07-08 12:01) - PLID 30641 - Determine if this dialog should be read-only
		// (c.haag 2008-07-08 12:34) - If this is new, then only the create permission matters. In this
		// implementation, we only care about the write permission.
		if (m_iTaskID != -1) {
			BOOL bSelfAssign = FALSE;
			BOOL bRemoteAssign = FALSE;
			for (int i=0; i < m_anExistingAssignTo.GetSize(); i++) {
				if (m_anExistingAssignTo[i] == GetCurrentUserID()) {
					bSelfAssign = TRUE;
				} else {
					bRemoteAssign = TRUE;
				}
			}
			if (bSelfAssign && bRemoteAssign) {
				// If we get here, the todo is joint owned. Allow the user to write unless
				// they fail both permission tests
				if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite, FALSE, 0, TRUE))
				{
					m_bReadOnly = TRUE;
				}
			}
			else {
				if (bSelfAssign)
				{
					if(!CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite, FALSE, 0, TRUE))
						m_bReadOnly = TRUE;
				}
				if (bRemoteAssign)
				{
					if(!CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite, FALSE, 0, TRUE))
						m_bReadOnly = TRUE;
				}
			}	

			// (c.haag 2008-07-08 09:07) - PLID 30641 - If this dialog is read-only, set all the
			// controls as such
			if (m_bReadOnly) {
				MsgBox(MB_ICONWARNING | MB_OK, "You do not have permission to change this todo alarm, but you may view its details.");
				m_description->Enabled = VARIANT_FALSE;
				m_method->Enabled = VARIANT_FALSE;
				m_priority->Enabled = VARIANT_FALSE;
				m_user->Enabled = VARIANT_FALSE;
				GetDlgItem(IDC_DEADLINE)->EnableWindow(FALSE);
				GetDlgItem(IDC_COMPLETED)->EnableWindow(FALSE);
				GetDlgItem(IDC_START)->EnableWindow(FALSE);
				GetDlgItem(IDC_REMIND_TIME)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_NOTES)->EnableWindow(FALSE);
				m_nxlAssignToMultiple.SetType(dtsDisabledHyperlink);
				// Do not disable the delete button. That falls under a different permission.
			}
		}

		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		ApplyIndividualPermissions();

		// (c.haag 2008-06-10 11:39) - PLID 11599 - Refresh the appearance of the AssignTo combo
		RefreshAssignToCombo();

		// (c.haag 2008-08-22 12:40) - Try to select the current user for new tasks
		if (-1 == m_iTaskID) {
			m_user->TrySetSelByColumn(0,_variant_t(GetCurrentUserID()));
			m_anAssignTo.RemoveAll();
			m_anAssignTo.Add(GetCurrentUserID());
		}


	}NxCatchAll("Could not load to-do task")
	
	return TRUE;
}

void CTaskEditDlg::OnChangeDeadline(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch
	try{
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTaskEditDlg::OnChangeStart(NMHDR* pNMHDR, LRESULT* pResult)
{
	//(e.lally 2009-11-16) PLID 36304 - Added try/catch
	try{
		*pResult = 0;
	}NxCatchAll(__FUNCTION__);
}

void CTaskEditDlg::OnDelete() 
{
	//(e.lally 2009-11-16) PLID 36304 - moved try to top, we only need one set otherwise we could still attempt to delete or audit
		//when part or nothing was performed
	try{
		CString sql;

		if (m_iTaskID == -1)
			return;
		// (s.tullis 2015-01-06 09:21) - 64440 Should not be able to delete a todo if associated with a scheduling mix rule
		if (ReturnsRecordsParam(" Select ID FROM SCheduleMixRulesT WHERE SCheduleMixRulesT.TaskID = {INT} ",m_iTaskID))
		{
			MsgBox("This task could not be deleted because it is associated with a scheduling mix rule.");
			return;
		}
		// (c.haag 2008-07-03 12:25) - PLID 30615 - Use the new todo table structure. We also no longer
		// need to select the assign to field in the first place; that is done in CheckAssignToPermissions
		_RecordsetPtr prs = CreateParamRecordset("SELECT 1 FROM ToDoList WHERE TaskID = {INT}", m_iTaskID);
		if (prs->eof)
		{
			MsgBox("This task could not be deleted because it no longer exists in the database.");
			return;
		}
	
		// CAH 5/5/2003 - Make sure we have permission to delete this task
		// (c.haag 2008-07-03 12:25) - PLID 30615 - Check permissions within the member function
		if (!CheckAssignToPermissions(sptDelete)) {
			return;
		}
		
		if(MessageBox("Are you sure you wish to delete this item?", "Delete?", MB_YESNO) == IDNO)
			return;
		
		// (z.manning, 02/19/2008) - PLID 28216 - Need to delete from attendance-todo linking table
		// (j.gruber 2008-04-02 11:01) - PLID 29296 - get rid of the link to the implementation ladder
		if(IsNexTechInternal()) {
			ExecuteParamSql("DELETE FROM AttendanceToDoLinkT WHERE TodoID = {INT}", m_iTaskID);
			ExecuteParamSql("UPDATE ClientImplementationStepsT SET ToDoTaskID = NULL WHERE TodoTaskID = {INT}", m_iTaskID);
		}
		// (c.haag 2008-06-09 17:16) - PLID 30328 - Use global utilities to delete todos
		TodoDelete(m_iTaskID);
		

		

		// (s.tullis 2014-09-08 09:10) - PLID 63344 - 
		if (m_anAssignTo.GetSize() == 1){
			CClient::RefreshTodoTable(m_iTaskID,m_nPersonID, m_anAssignTo[0], TableCheckerDetailIndex::tddisDeleted);
		}
		else{
			CClient::RefreshTodoTable(m_iTaskID,m_nPersonID, -1, TableCheckerDetailIndex::tddisDeleted);
		}

	
	
		//auditing
		COleDateTime dtDeadline = m_deadline.GetValue().date;
		long nAuditID = BeginNewAuditEvent();
		CString strPerson, strOld, strTemp;
		GetDlgItemText(IDC_EDIT_NOTES, strTemp);
		strOld.Format("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", GetAssignToNames(), FormatDateTimeForInterface(dtDeadline, 0, dtoDate), VarString(m_description->GetComboBoxText()), strTemp);
		if (m_isPatient)
			AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditID, aeiPatientTodoTaskDelete, m_iTaskID, strOld, "<Deleted>", aepMedium, aetDeleted);
		else 
			AuditEvent(-1, GetExistingContactName(m_nPersonID), nAuditID, aeiContactTodoTaskDelete, m_iTaskID, strOld, "<Deleted>", aepMedium, aetDeleted);

		// (c.haag 2008-08-18 16:47) - PLID 30607 - Flag the item as having been deleted
		m_bWasDeleted = TRUE;

	}NxCatchAll("Could not delete task");
	
	CDialog::OnOK();	
}

void CTaskEditDlg::OnOK() 
{
	// (c.haag 2008-07-08 09:12) - PLID 30641 - Quit right away if this dialog
	// is read-only
	if (m_bReadOnly) {
		CDialog::OnOK();
		return;
	}

	_RecordsetPtr rs(__uuidof(Recordset)),
				  rsUserNameInfo(__uuidof(Recordset));
	CString	categoryID = "",
			task="", 
			priority="",
			deadlineDate, 
			completeDate,
			remindByDate,
			notes,
			strSQL,
			sql;

	int
			priority_level,
			loginID=0;

	try
	{
		if(m_priority->CurSel == -1) {
			MsgBox("You must select a priority for this task.");
			return;
		}
		// (c.haag 2008-06-10 12:05) - PLID 11599 - Check to ensure at least one
		// assign to is selected
		if(0 == m_anAssignTo.GetSize()) {
			MsgBox("You must select a user for this task to be assigned to.");
			return;
		}

		if(m_description->GetCurSel()==-1 || VarLong(m_description->GetValue(m_description->GetCurSel(), 0)) == -1)
			categoryID = "NULL";
		else
			categoryID.Format("%li",m_description->GetValue(m_description->GetCurSel(),0).lVal);

		//DRT 4/25/03 - Even though newer data forces m_method and m_priority to be selected, there may well 
		//		be older data that does not have it.
		if(m_method->CurSel != -1)
			task = VarString(m_method->GetValue(m_method->CurSel, 0));

		if(m_priority->CurSel != -1)
			priority = VarString(m_priority->GetValue(m_priority->CurSel, 0));

		if (priority == "") 
			priority = "Low";
		if(priority=="Low")
			priority_level=3;
		else if(priority=="Medium")
			priority_level=2;
		else if(priority=="High")
			priority_level=1;

		
		COleDateTime dtCompleted, dtDeadline, dtRemind;


		dtDeadline = m_deadline.GetValue().date;
		deadlineDate = "'" + _Q(FormatDateTimeForSql(dtDeadline, dtoDate)) + "'";// + ((COleDateTime)m_nxtDeadlineTime.GetDateTime()).Format("%I:%M:%S %p'");

		dtRemind = m_remind.GetValue().date;
		remindByDate = "'" + _Q(FormatDateTimeForSql(dtRemind, dtoDate) + " " + FormatDateTimeForSql(((COleDateTime)m_nxtRemind->GetDateTime()), dtoTime)) + "'";

		_variant_t var = m_completed.GetValue();
		if(var.vt == VT_NULL)
			completeDate = "NULL";
		else {
			dtCompleted = var.date;
			if(dtCompleted.m_status != COleDateTime::invalid && dtCompleted != COleDateTime())
				completeDate = "'" + _Q(FormatDateTimeForSql(dtCompleted, dtoDate)) + "'";
			else
				completeDate = "NULL";
		}

		//check to see that it is not a invalid SQL date
		COleDateTime dtMin, dtMax;
		dtMin.SetDate(1753, 1, 1);
		dtMax.SetDate(9999, 12, 31);

		if (dtCompleted < dtMin || dtCompleted > dtMax || 
			dtRemind < dtMin || dtRemind > dtMax ||
			dtDeadline < dtMin || dtDeadline > dtMax) {

			//let them know
			MsgBox("You may not enter a date before 01/01/1753 or a date after 12/31/9999");
			return;
		}

		if (m_iTaskID == -1) {
			//On NEW alarms only, warn when the deadline is in the past, or if the remind time is in the past.
			COleDateTime dtNow;

			//DRT 1/5/2005 - PLID 15192 - The below line was attempting to force the comparison to only be done using
			//	the date (no time).  However, this is horrible non-internationally compliant, and on formats where the
			//	date is in a different format, would do wildly crazy things.  ParseDateTime uses the current regional
			//	settings, but we were passing it a forcibly english (US) formatted date.
			//dtNow.ParseDateTime((COleDateTime::GetCurrentTime()).Format("%m/%d/%Y"));
			dtNow = COleDateTime::GetCurrentTime();
			dtNow.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
			//

			if(dtDeadline < dtNow) {
				if(IDNO == MessageBox("The 'Deadline' date is in the past. Do you still wish to save?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			if(dtRemind < dtNow) {
				if(IDNO == MessageBox("The 'Start Reminding Me' date is in the past. Do you still wish to save?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
		}

		if (dtDeadline < dtRemind)
		{	m_remind.SetValue(COleVariant(dtDeadline));
			AfxMessageBox("The \'Start Reminding Me\' date cannot be after the deadline.\nIt has been set to the 'Deadline' date.");
			return;
		}

		if (dtCompleted > COleDateTime::GetCurrentTime())
		{
			if (IDNO == MsgBox(MB_YESNO, "You have entered a completion date that exists in the future. Are you sure you wish to do this?"))
			{
				return;								
			}
		}

		if(m_nxtRemind->GetStatus() == 2) {
			AfxMessageBox("You have entered an invalid remind time");
			return;
		}

		//DRT 4/25/03 - We have this exact same line above!  Plus it can cause an exception
		//task = VarString(m_method->GetValue(m_method->CurSel, 0));

		GetDlgItemText(IDC_EDIT_NOTES, notes);
		if(notes.GetLength() > 2000) {
			notes = notes.Left(2000);
			AfxMessageBox("The note you entered is larger than the maximum amount (2000) and has been shortened.\n"
				"Please double-check your note and edit it as needed.");
			SetDlgItemText(IDC_EDIT_NOTES, notes);
			return;
		}

		//CAH 5/5/2003 - Examine the assign-to user and decide if we have
		//permission to make todo's for that person
		//TES 6/17/2003: We no longer care if this is a patient followup.
		//if (m_isPatient)
		// (c.haag 2008-07-03 12:25) - PLID 30615 - Check permissions within the member function
		// (c.haag 2008-07-03 11:02) - PLID 30615 - Need to discern create and write permissions
		// (c.haag 2008-07-11 09:37) - PLID 30550 - If this is a new todo alarm opened from the EMN's
		// "New ToDo" button, or the todo was opened from the More Info todo list, then don't check
		// permissions if this task  is directly tied to the EMN in any way.
		BOOL bCheckPermissions = TRUE;
		if (m_iTaskID == -1 && m_bInvokedFromEMN && m_checkLinkTaskWithEMN.GetCheck()) {
			bCheckPermissions = FALSE;
		}
		else if (m_iTaskID != -1 && m_bInvokedFromEMN && (ttEMN == m_RegardingType || ttEMNDetail == m_RegardingType)) {
			bCheckPermissions = FALSE;
		}

		if (bCheckPermissions) {
			if (m_iTaskID == -1) {
				if (!CheckAssignToPermissions(sptCreate)) {
					return;
				}
			} else {
				if (!CheckAssignToPermissions(sptWrite)) {
					return;
				}	
			}
		}

		// (c.haag 2008-06-13 12:57) - PLID 30321 - Before changing an existing todo alarm, we
		// must ensure that only one user is assigned to it
		// (j.jones 2008-12-02 10:18) - PLID 30830 - multiple users per tracking step is now allowed
						/*
		if (m_iTaskID != -1) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT RegardingType FROM TodoList WHERE TaskID = {INT}", m_iTaskID);
			if (!prs->eof) {
				TodoType tt = (TodoType)AdoFldLong(prs, "RegardingType");
				if (ttTrackingStep == tt && m_anAssignTo.GetSize() > 1) {
					AfxMessageBox("You may not assign multiple users to a todo alarm linked to a tracking ladder.", MB_ICONERROR);
					return;					
				}
			} else {
				// This should never happen; of course it also means our test doesn't fail
			}
		}
		*/

		// (a.walling 2008-07-15 12:44) - PLID 29900 - PersonID no longer necessary
		/*
		if (m_isPatient)
			personID = m_nPersonID;
		else 
			personID = m_nPersonID;*/

		bool bNewTask = false;
		_RecordsetPtr rsOldInfo;
	


		if (m_iTaskID != -1) 
		{
			//for auditing
			// (c.haag 2008-06-10 12:06) - PLID 30321 - Account for new TodoAssignTo table
			//(c.copits 2010-12-02) PLID 40794 - Permissions for individual todo alarm fields
			rsOldInfo = CreateParamRecordset("SELECT NoteCatsF.Description, dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedTo, Deadline, Notes, Remind, Done, Priority, Task "
												"FROM ToDoList "
												"LEFT JOIN NoteCatsF ON NoteCatsF.ID = ToDoList.CategoryID "
												"WHERE TaskID = {INT}", m_iTaskID);
			//TS 11/1/02: I see no reason to update the enteredby field, since we didn't enter this step, just modified it.
			sql.Format (
				"UPDATE ToDoList SET "
					"CategoryID = %s, Task = '%s', "
					"Priority = %li, Deadline = %s, "
					"Done = %s, Remind = %s, Notes = '%s' "
				"WHERE TaskID = %i\r\n"
				"DELETE FROM TodoAssignToT WHERE TaskID = %d\r\n"
				"INSERT INTO TodoAssignToT (TaskID, AssignTo) SELECT %d, PersonID FROM UsersT WHERE PersonID IN (%s)"
				,
				categoryID, _Q(task),
				priority_level, deadlineDate,
				completeDate, remindByDate, _Q(notes),
				m_iTaskID,
				m_iTaskID,
				m_iTaskID, ArrayAsString(m_anAssignTo)
				);
			ExecuteSql("%s", sql);
		}
		else 
		{
			COleDateTime dtRemindTime = (COleDateTime)m_nxtRemind->GetDateTime();
			COleDateTime dtRemindDateTime;
			dtRemindDateTime.SetDateTime(dtRemind.GetYear(), dtRemind.GetMonth(), dtRemind.GetDay(),
				dtRemindTime.GetHour(), dtRemindTime.GetMinute(), dtRemindTime.GetSecond());
			long nCategoryID;
			if(m_description->GetCurSel()==-1 || VarLong(m_description->GetValue(m_description->GetCurSel(), 0)) == -1) {
				nCategoryID = -1;
			} else {
				nCategoryID = VarLong(m_description->GetValue(m_description->GetCurSel(),0).lVal, -1);
			}

			// (c.haag 2008-06-09 12:24) - PLID 30321 - Use a global utility function to create the todo
			// (c.haag 2008-07-11 09:39) - PLID 30550 - If this dialog was invoked from an EMN, and the user
			// wants to link the task with the EMN, then we need to define the regarding type and ID.
			TodoType RegardingType = ttPatientContact;
			long nRegardingID = m_nPersonID;
			if (m_bInvokedFromEMN && m_checkLinkTaskWithEMN.GetCheck()) {
				RegardingType = ttEMN;
				// (c.haag 2008-07-14 12:03) - PLID 30550 - Ensure the regarding ID is -1, not the EMN ID,
				// because we don't want to allow users to delete todo's created by EMR entities before they
				// are saved (unless they're an administrator).
				nRegardingID = -1;
				m_bWasLinkedToEMNWhenSaved = TRUE;
				// Format the notes
				CString str = FormatString("EMN: %s\r\n%s", m_strEMNDescription, notes);
				notes = str;
			}

			// (c.haag 2010-05-21 13:48) - PLID 38827 - Be able to override the regarding type and ID
			if (-1 != m_nRegardingIDOverride) {
				nRegardingID = m_nRegardingIDOverride;
				RegardingType = m_RegardingTypeOverride;
			}
		
			
			m_iTaskID = TodoCreate(dtRemindDateTime, dtDeadline, m_anAssignTo, notes, task, nRegardingID, RegardingType,
				m_nPersonID, -1, (TodoPriority)priority_level, nCategoryID, dtCompleted);
			bNewTask = true;
		}		

		// (s.tullis 2014-09-08 09:10) - PLID 63344 - 
		TableCheckerDetailIndex::Todo_Status status;
		if (bNewTask){
			status = TableCheckerDetailIndex::tddisAdded;
		}
		else{
			status = TableCheckerDetailIndex::tddisChanged;
		}

		if (m_anAssignTo.GetSize() == 1){
			CClient::RefreshTodoTable(m_iTaskID, m_nPersonID, m_anAssignTo[0], status);
		}
		else{
			CClient::RefreshTodoTable(m_iTaskID, m_nPersonID, -1,status);
		}



		//auditing
		long nAuditID = BeginNewAuditEvent();
		CString strNew = "", strOld = "", strTemp;
		BOOL bFirst = TRUE;
		GetDlgItemText(IDC_EDIT_NOTES, strTemp);
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields 
		// Note: if (bNewTask == true), m_iTaskID was created in this class; if (m_bIsNew == TRUE),
		// m_iTaskID was created (externally to this class) right before. In both cases, we
		// really have a new to-do alarm created that needs to be auditied as such.
		if (bNewTask == true || m_bIsNew) {
			// If completed date not set, FormatDateTimeForInterface will return some date in 1899
			CString strCompletedDate;
			if ((dtCompleted.m_status == COleDateTime::invalid) ||
				(!dtCompleted))
				strCompletedDate = "<No Date>";
			else
				strCompletedDate = FormatDateTimeForInterface(dtCompleted, 0, dtoDate);

				strNew.Format("Created by: %s, Assigned To: %s, Deadline: %s, "
							"Remind Time: %s %s, Category: %s, Note: %s, Method: %s, Priority: %s, "
							"Completed: %s", 
				GetCurrentUserName(), 
				GetAssignToNames(), 
				FormatDateTimeForInterface(dtDeadline, 0, dtoDate), 
				FormatDateTimeForInterface((COleDateTime)m_remind.GetValue().date, 0, dtoDate), 
				FormatDateTimeForInterface((COleDateTime)m_nxtRemind->GetDateTime(), 0, dtoTime), 
				VarString(m_description->GetComboBoxText()), 
				strTemp, 
				VarString(m_method->GetComboBoxText()), 
				CString(m_priority->GetValue(m_priority->CurSel, 0)),
				strCompletedDate
				);
			if (m_isPatient)
				AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditID, aeiPatientToDoCreated, m_iTaskID, "", strNew, aepMedium, aetCreated);
			else 
				AuditEvent(-1, GetExistingContactName(m_nPersonID), nAuditID, aeiContactToDoCreated, m_iTaskID, "", strNew, aepMedium, aetCreated);
		}
		else {
			if (!rsOldInfo->eof) {
				//Find what has changed and create an appropriate message
				if (AdoFldString(rsOldInfo, "AssignedTo", "") != GetAssignToNames()) {
					strOld += "Assigned To: " + AdoFldString(rsOldInfo, "AssignedTo", "");
					strNew += "Assigned To: " + GetAssignToNames();
					bFirst = FALSE;
				}
				if (FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Deadline"), 0, dtoDate) != FormatDateTimeForInterface(dtDeadline, 0, dtoDate)) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Deadline: " + FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Deadline"), 0, dtoDate);
					strNew += "Deadline: " + FormatDateTimeForInterface(dtDeadline, 0, dtoDate);
					bFirst = FALSE;
				}
				if (FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Remind"), 0, dtoTime) != FormatDateTimeForInterface((COleDateTime)m_nxtRemind->GetDateTime(), 0, dtoTime)) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Remind Time: " + FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Remind"), 0, dtoTime);
					strNew += "Remind Time: " + FormatDateTimeForInterface((COleDateTime)m_nxtRemind->GetDateTime(), 0, dtoTime);
					bFirst = FALSE;
				}
				if (AdoFldString(rsOldInfo, "Description", "") != VarString(m_description->GetComboBoxText())) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Category: " + AdoFldString(rsOldInfo, "Description", "");
					strNew += "Category: " + VarString(m_description->GetComboBoxText());
					bFirst = FALSE;
				}
				if (AdoFldString(rsOldInfo, "Notes", "") != strTemp) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Note: " + AdoFldString(rsOldInfo, "Notes", "");
					strNew += "Note: " + strTemp;
					bFirst = FALSE;
				}
				//(c.copits 2010-12-02) PLID 40794 - Permissions for individual todo alarm fields (audit method, priority, completed, start reminding)
				// Method
				if (AdoFldString(rsOldInfo, "Task", "") != VarString(m_method->GetComboBoxText())) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					if (AdoFldString(rsOldInfo, "Task", "") == "")
						strOld += "Task: <No Task>";
					else
						strOld += "Task: " + AdoFldString(rsOldInfo, "Task", "");

					if (CString(VarString(m_method->GetComboBoxText())) == "")
						strNew += "Task: <No Task>";
					else
						strNew += "Task: " + VarString(m_method->GetComboBoxText());
					bFirst = FALSE;
				}
				// Priority
				_variant_t varBYTE = rsOldInfo->Fields->Item["Priority"]->Value;
				if (varBYTE.vt == VT_NULL) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Priority: <No Priority>";
					strNew += "Priority: " + CString(VarString(m_priority->GetValue(m_priority->CurSel, 0)));
					bFirst = FALSE;
				}
				else if (AdoFldByte(rsOldInfo, "Priority", -1) != BytePriorityString(m_priority->CurSel)) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Priority: " + StrPriorityString(AdoFldByte(rsOldInfo, "Priority", -1));
					strNew += "Priority: " + CString(m_priority->GetValue(m_priority->CurSel, 0));
					bFirst = FALSE;
				}
				// Completed
				// If todo just created, "Done" will be NULL
				_variant_t varDT;
				varDT = rsOldInfo->Fields->Item["Done"]->Value;
				if( varDT.vt == VT_NULL) {
					// There can be no completed date set
					// So don't bother having old=<No Date> and new=<No Date>
					// If completed date not set, FormatDateTimeForInterface will return some date in 1899
					if (dtCompleted) {
						if (bFirst == FALSE) {
							strOld += ", ";
							strNew += ", ";
						}
						strOld += "Completed: <No Date>";
						strNew += "Completed: " + FormatDateTimeForInterface(dtCompleted, 0, dtoDate);
						bFirst = FALSE;
					}
				}
				else if (FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Done"), 0, dtoDate) != FormatDateTimeForInterface(dtCompleted, 0, dtoDate)) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Completed: " + FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Done"), 0, dtoDate);
					strNew += "Completed: " + FormatDateTimeForInterface(dtCompleted, 0, dtoDate);
					bFirst = FALSE;
				}

				// Start Reminding (date)
				varDT = rsOldInfo->Fields->Item["Remind"]->Value;
				if (varDT.vt == VT_NULL) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Remind: <No Date>";
					strNew += "Remind: " + FormatDateTimeForInterface(dtRemind, 0, dtoDate);
					bFirst = FALSE;
				}
				else if (FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Remind"), 0, dtoDate) != FormatDateTimeForInterface(dtRemind, 0, dtoDate)) {
					if (bFirst == FALSE) {
						strOld += ", ";
						strNew += ", ";
					}
					strOld += "Remind: " + FormatDateTimeForInterface(AdoFldDateTime(rsOldInfo, "Remind"), 0, dtoDate);
					strNew += "Remind: " + FormatDateTimeForInterface(dtRemind, 0, dtoDate);
					bFirst = FALSE;
				}
			}
			if (strNew != "") {
				if (m_isPatient)		
					AuditEvent(m_nPersonID, GetExistingPatientName(m_nPersonID), nAuditID, aeiPatientToDoChanged, m_iTaskID, strOld, strNew, aepMedium, aetChanged);
				else 
					AuditEvent(-1, GetExistingContactName(m_nPersonID), nAuditID, aeiContactToDoChanged, m_iTaskID, strOld, strNew, aepMedium, aetChanged);
			}
		}			
	}
	NxCatchAll("Could not save todo task");

	CDialog::OnOK();
}

void CTaskEditDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CTaskEditDlg::OnSelChangingPriority(long FAR* nNewSel) 
{
	//(e.lally 2009-11-16) PLID 36304 - added try/catch
	try{
		if(*nNewSel == -1) {
			*nNewSel = m_priority->CurSel;
		}
	}NxCatchAll(__FUNCTION__);
}

void CTaskEditDlg::OnSelChangingAssignto(long FAR* nNewSel) 
{
	//(e.lally 2009-11-16) PLID 36304 - added try/catch
	try{
		if(*nNewSel == -1) {
			*nNewSel = m_user->CurSel;
		}
	}NxCatchAll(__FUNCTION__);
}

void CTaskEditDlg::OnSelChangingMethod(long FAR* nNewSel) 
{
	//(e.lally 2009-11-16) PLID 36304 - added try/catch
	try{
		if(*nNewSel == -1) {
			*nNewSel = m_method->CurSel;
		}
	}NxCatchAll(__FUNCTION__);
}

void CTaskEditDlg::SetDeadlineDate(COleDateTime dtDeadline)
{
	try {
		// (m.hancock 2006-10-30 10:00) - PLID 21730 - Allow a default deadline and reminder date to be set.
		m_bUseDefaultDeadline = TRUE;
		m_dtDefaultDeadline = dtDeadline;

	} NxCatchAll("Error in CTaskEditDlg::SetDeadlineDate()");
}

void CTaskEditDlg::SetReminderDate(COleDateTime dtReminder)
{
	try {
		// (m.hancock 2006-10-30 10:00) - PLID 21730 - Allow a default deadline and reminder date to be set.
		m_bUseDefaultRemindingDate = TRUE;
		m_dtDefaultRemindingDate = dtReminder;

	} NxCatchAll("Error in CTaskEditDlg::SetReminderDate()");
}

// (c.haag 2010-05-21 13:48) - PLID 38827 - Be able to override the regarding type and ID
void CTaskEditDlg::SetNewTaskRegardingOverrides(long nID, TodoType type)
{
	m_nRegardingIDOverride = nID;
	m_RegardingTypeOverride = type;
}

void CTaskEditDlg::OnRequeryFinishedAssignto(short nFlags) 
{
	// (c.haag 2008-06-10 11:49) - PLID 11599 - Add a multiple select row
	// to the assign to combo
	try {
		_variant_t vNull;
		vNull.vt = VT_NULL;
		IRowSettingsPtr pRow = m_user->GetRow(-1);
		pRow->Value[eclat_ID] = vNull;
		pRow->Value[eclat_Username] = _bstr_t(" { Multiple Users } ");
		m_user->AddRow(pRow);	

		// (j.gruber 2010-01-12 12:53) - PLID 20916 - colorize the row that is the patient coordinator
		long nRow = m_user->FindByColumn(eclat_ID, m_nPatientCoordID, -1, FALSE);
		if (nRow != -1) {
			pRow = m_user->GetRow(nRow);
			if (pRow) {
				pRow->PutForeColor(RGB(255,0,0));
				pRow->PutForeColorSel(RGB(255,0,0));
			}
		}
	}
	NxCatchAll("Error in CTaskEditDlg::OnRequeryFinishedAssignto");
}

void CTaskEditDlg::OnSelChosenAssignto(long nRow) 
{
	// (c.haag 2008-06-10 11:51) - PLID 11599 - Added support for multiple selections
	try {
		long nID = (-1 != nRow) ? VarLong(m_user->Value[nRow][eclat_ID], -1) : -1;
		if (-1 == nID) {
			// Multiple assignee selection
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "UsersT");
			for (int i=0; i < m_anAssignTo.GetSize(); i++) {
				dlg.PreSelect(m_anAssignTo[i]);
			}
			dlg.m_strNameColTitle = "Username";
			if (IDOK == dlg.Open("PersonT INNER JOIN UsersT ON UsersT.PersonID = PersonT.ID",
				"Archived = 0 AND ID > 0",
				"ID", "Username", "Please choose one or more users from the list.", 1))
			{
				dlg.FillArrayWithIDs(m_anAssignTo);
			}
			else {
				// The user changed their mind. Don't change m_anAssignTo.
			}
		} else {
			// Single assignee
			m_anAssignTo.RemoveAll();
			m_anAssignTo.Add(nID);
		}

		// Now refresh the visible state of the Assign To combo
		RefreshAssignToCombo();
	}
	NxCatchAll("Error in CTaskEditDlg::OnSelChosenAssignto");
	
}

BOOL CTaskEditDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	// (c.haag 2008-06-10 12:00) - PLID 11599 - Ensure we use the correct hyperlink cursor
	if (m_anAssignTo.GetSize() > 1) {
		if (m_rcAssignToMultiple.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CTaskEditDlg::OnLabelLButtonDown(WPARAM wParam, LPARAM lParam) 
{
	// (c.haag 2008-06-10 12:00) - PLID 11599 - If the user clicked on the
	// Assign To hyperlink, invoke a selection change
	try {
		OnSelChosenAssignto(m_user->CurSel);
	}
	NxCatchAll("Error in CTaskEditDlg::OnLButtonDown");
	return 0;
}


//(c.copits 2010-11-30) PLID 40794 - Permissions for individual todo alarm fields
// This method accepts a datalist index and returns the index corresponding to the
// value in the table ToDoList for auditing purposes.
// Input: long int corresponding to datalist number priority.
// Output: byte (unsigned short) corresponding to TodoList table priority.
unsigned short CTaskEditDlg::BytePriorityString(long nPriorityIn)
{
	unsigned short nPriority;
	
	// Medium
	if (nPriorityIn == 1) {
		nPriority = 2;
	}

	// High
	else if (nPriorityIn == 2) {
		nPriority = 1;
	}

	// Low
	//if (nPriorityIn == 0)
	else {
		nPriority = 3;
	}

return nPriority;
}

//(c.copits 2010-12-02) PLID 40794 - Permissions for individual todo alarm fields
// This method applies individual (per-field) permissions to the task editing dialog.
void CTaskEditDlg::ApplyIndividualPermissions()
{
	BOOL bSelfAssign = FALSE;
	BOOL bRemoteAssign = FALSE;

	for (int i=0; i < m_anExistingAssignTo.GetSize(); i++) {
		if (m_anExistingAssignTo[i] == GetCurrentUserID()) {
			bSelfAssign = TRUE;
		} else {
			bRemoteAssign = TRUE;
		}
	}

	// If user can create a todo, then the permissions don't apply.
	if (!m_bIsNew) {
			// Permissions if assigned to self and have self write permissions		
			if (bSelfAssign && CheckCurrentUserPermissions(bioSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_CATEGORY)->EnableWindow(FALSE);
						m_description->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsMethod, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_METHOD)->EnableWindow(FALSE);
						m_method->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsPriority, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_PRIORITY)->EnableWindow(FALSE);
						m_priority->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsAssignTo, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_ASSIGNTO)->EnableWindow(FALSE);
						m_user->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsNotes, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_EDIT_NOTES)->EnableWindow(FALSE);
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsDeadline, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_DEADLINE)->EnableWindow(FALSE);
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsRemindTime, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_REMIND_TIME)->EnableWindow(FALSE);
						m_nxtRemind->Enabled = VARIANT_FALSE;
					}
				}
			// Permission if assigned to other and have other write permissions
			if (bRemoteAssign && CheckCurrentUserPermissions(bioNonSelfFollowUps, sptWrite, FALSE, 0, TRUE)) {
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsCategory, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_CATEGORY)->EnableWindow(FALSE);
						m_description->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsMethod, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_METHOD)->EnableWindow(FALSE);
						m_method->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsPriority, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_PRIORITY)->EnableWindow(FALSE);
						m_priority->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsAssignTo, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_ASSIGNTO)->EnableWindow(FALSE);
						m_user->Enabled = VARIANT_FALSE;
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsNotes, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_EDIT_NOTES)->EnableWindow(FALSE);
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsDeadline, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_DEADLINE)->EnableWindow(FALSE);
					}
					if(!CheckCurrentUserPermissions(bioTodoAlarmFieldsRemindTime, sptWrite, FALSE, 0, TRUE)) {
						GetDlgItem(IDC_REMIND_TIME)->EnableWindow(FALSE);
						m_nxtRemind->Enabled = VARIANT_FALSE;
					}
			}
	}
}