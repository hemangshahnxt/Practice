// EMRTodoActionConfigDlg.cpp : implementation file
//
// (c.haag 2008-06-02 16:16) - PLID 30221 - Initial implementation
//
#include "stdafx.h"
#include "administratorrc.h"
#include "EMRTodoActionConfigDlg.h"
#include "multiselectdlg.h"
#include "emrutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;
using namespace ADODB;

// The sentinel ID for the {Multiple User} selection
#define MULTIPLE_SELECTIONS		(long)-99999

typedef enum {
	eclcat_ID = 0,
	eclcat_Description = 1
} EColCategories;

typedef enum {
	eclat_ID = 0,
	eclat_Username = 1
} EColAssignTo;

typedef enum {
	eclmth_Text = 0
} EColMethod;

typedef enum {
	eclpri_ID = 0,
	eclpri_Text = 1
} EColPriority;

/////////////////////////////////////////////////////////////////////////////
// CEMRTodoActionConfigDlg dialog


CEMRTodoActionConfigDlg::CEMRTodoActionConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRTodoActionConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRTodoActionConfigDlg)
	//}}AFX_DATA_INIT
	m_bIsNew = TRUE;
}

// (c.haag 2008-05-30 16:31) - Returns TRUE if this is a new todo action
BOOL CEMRTodoActionConfigDlg::IsNew() const
{
	return m_bIsNew;
}

// (c.haag 2008-05-30 16:33) - Load the default settings for a new todo action
void CEMRTodoActionConfigDlg::LoadDefaultValues()
{
	SetDlgItemCheck(IDC_DEADLINE_IMMEDIATE, 1);
	SetDlgItemCheck(IDC_REMIND_IMMEDIATE, 1);
	SetDlgItemInt(IDC_DEADLINE_INTERVAL_NUMBER, 1);
	SetDlgItemInt(IDC_REMIND_INTERVAL_NUMBER, 1);
	m_comboDeadlineIntervalType.SetCurSel(0);
	m_comboRemindType.SetCurSel(0);
	GetDlgItem(IDC_DEADLINE_INTERVAL_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_DEADLINE_INTERVAL_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMIND_INTERVAL_NUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMIND_TYPE)->EnableWindow(FALSE);
	// Update control appearances
	RefreshAssignToCombo();
}

// (c.haag 2008-05-30 16:36) - Loads an existing todo action
void CEMRTodoActionConfigDlg::LoadTodoAction()
{
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	if (m_nCommit_Category > 0) {
		m_dlCategory->TrySetSelByColumn_Deprecated(eclcat_ID, m_nCommit_Category);
	} else {
		_variant_t vNull;
		vNull.vt = VT_NULL;
		m_dlCategory->SetSelByColumn(eclcat_ID, vNull);
	}

	m_dlMethod->SetSelByColumn(eclmth_Text, _bstr_t(m_strCommit_Method));
	m_dlPriority->SetSelByColumn(eclpri_ID, m_nCommit_Priority);
	m_anAssignTo.Copy( m_anCommit_AssignTo );
	RefreshAssignToCombo();
	SetDlgItemText(IDC_EDIT_NOTES, m_strCommit_Notes);

	// Deadline type
	if (-1 == m_nCommit_DeadlineType) {
		GetDlgItem(IDC_DEADLINE_INTERVAL_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEADLINE_INTERVAL_TYPE)->EnableWindow(FALSE);
		SetDlgItemCheck(IDC_DEADLINE_IMMEDIATE, 1);
		SetDlgItemInt(IDC_DEADLINE_INTERVAL_NUMBER, 1);
		m_comboDeadlineIntervalType.SetCurSel(0);
	}
	else {
		GetDlgItem(IDC_DEADLINE_INTERVAL_NUMBER)->EnableWindow(TRUE);
		GetDlgItem(IDC_DEADLINE_INTERVAL_TYPE)->EnableWindow(TRUE);
		SetDlgItemCheck(IDC_DEADLINE_TIME_AFTER, 1);
		SetDlgItemInt(IDC_DEADLINE_INTERVAL_NUMBER, m_nCommit_DeadlineInterval);
		m_comboDeadlineIntervalType.SetCurSel(m_nCommit_DeadlineType);
	}

	// Remind type
	if (-1 == m_nCommit_RemindType) {
		GetDlgItem(IDC_REMIND_INTERVAL_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMIND_TYPE)->EnableWindow(FALSE);
		SetDlgItemCheck(IDC_REMIND_IMMEDIATE, 1);
		SetDlgItemInt(IDC_REMIND_INTERVAL_NUMBER, 1);
		m_comboRemindType.SetCurSel(0);
	}
	else {
		GetDlgItem(IDC_REMIND_INTERVAL_NUMBER)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMIND_TYPE)->EnableWindow(TRUE);
		SetDlgItemCheck(IDC_REMIND_AFTER, 1);
		SetDlgItemInt(IDC_REMIND_INTERVAL_NUMBER, m_nCommit_RemindInterval);
		m_comboRemindType.SetCurSel(m_nCommit_RemindType);
	}
}

// (c.haag 2008-07-01 09:11) - Moves up a todo date given an interval. Used for validation testing only.
void CEMRTodoActionConfigDlg::MoveTodoDate(COleDateTime& dt, long nType, long nInterval)
{
	switch (nType) {
	case 0: // Days
		dt += COleDateTimeSpan(1 * nInterval,0,0,0);
		break;
	case 1: // Weeks
		dt += COleDateTimeSpan(7 * nInterval,0,0,0);
		break;
	case 2: // Months
		{
			// (s.dhole 2012-08-01 09:11) PLID 51912 code change  to support date  as 31 or leap year date 29
			// COleDateTime month range is 1-12
			long nDay = dt.GetDay();
			long nYears =dt.GetYear() + (nInterval / 12);
			long nMonths = nInterval % 12;
			long nCurrentMonths = dt.GetMonth();
			dt.SetDate( nYears , nCurrentMonths , dt.GetDay() );
			// for leap year  day > 28 then check valid day of month 
			long nNewDay = nDay;
			if (nDay>28){
				// set correct month day
				while ( dt.GetStatus() == COleDateTime::invalid && nNewDay >28){
					nNewDay --;
					dt.SetDate(nYears, nCurrentMonths , nNewDay );
				}
			}
			while (nMonths > 0) {
				long nTempMonths =1;
				if (dt.GetMonth() == 12) {
					nYears = dt.GetYear() + 1;
					nTempMonths = 1;
				} else {
					nYears = dt.GetYear() ;	
					nTempMonths =dt.GetMonth() +1;
				}
				dt.SetDate( nYears , nTempMonths, nDay );
				if (nDay>28){
					nNewDay = nDay;
					while (dt.GetStatus() == COleDateTime::invalid && nNewDay>28){
						nNewDay--;
						dt.SetDate(nYears, nTempMonths, nNewDay);
					}
				}
				nMonths--;
			}
		}
		break;
	case 3: // Years
		{
			// (s.dhole 2012-08-01 09:11) PLID 51912 code change  to support date  as 31 or leap year date 29
			// use breackpoit 
			long nDay = dt.GetDay();
			long nMonth= dt.GetMonth();
			long nYears =dt.GetYear() + nInterval;
			dt.SetDate( nYears , nMonth, dt.GetDay() );
			// for leap year  day > 28 then check valid day of month 
			while (dt.GetStatus() == COleDateTime::invalid && nDay >28){
				nDay --;
				dt.SetDate(nYears, nMonth, nDay );
			}
		}
		break;
	default:
		break;
	}
}

// (c.haag 2008-05-30 16:41) - Returns TRUE if we may save the data
// for this dialog, or false if not
BOOL CEMRTodoActionConfigDlg::Validate()
{
	if (NULL == m_dlMethod->CurSel) {
		MessageBox("You must choose a method before saving your changes.", "NexTech Practice", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	else if (NULL == m_dlPriority->CurSel) {
		MessageBox("You must choose a priority before saving your changes.", "NexTech Practice", MB_ICONERROR | MB_OK);
		return FALSE;
	}
	else if (0 == m_anAssignTo.GetSize()) {
		MessageBox("You must choose at least one assignee before saving your changes.", "NexTech Practice", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Now validate the durations, if applicable
	if (m_radioDeadlineAfter.GetCheck()) {
		int n = GetDlgItemInt(IDC_DEADLINE_INTERVAL_NUMBER);
		if (n <= 0 || n > 1000) {
			MessageBox("Please choose a deadline interval between or including 1 and 1000 before saving.", "NexTech Practice", MB_ICONERROR | MB_OK);
			GetDlgItem(IDC_DEADLINE_INTERVAL_NUMBER)->SetFocus();
			return FALSE;				
		}
	}
	if (m_radioRemindAfter.GetCheck()) {
		int n = GetDlgItemInt(IDC_REMIND_INTERVAL_NUMBER);
		if (n <= 0 || n > 1000) {
			MessageBox("Please choose a reminder interval between or including 1 and 1000 before saving.", "NexTech Practice", MB_ICONERROR | MB_OK);
			GetDlgItem(IDC_REMIND_INTERVAL_NUMBER)->SetFocus();
			return FALSE;				
		}
	}

	// Make sure that the reminder is on or before the deadline. To do this, we take two
	// dummy pivot dates, increase them by the time span based on the current selection, and then
	// compare the two dates
	COleDateTime dtDeadlinePivot = COleDateTime::GetCurrentTime();
	dtDeadlinePivot.SetDate(dtDeadlinePivot.GetYear(), dtDeadlinePivot.GetMonth(), dtDeadlinePivot.GetDay());
	COleDateTime dtRemindPivot = dtDeadlinePivot;

	MoveTodoDate(dtDeadlinePivot, (m_radioDeadlineAfter.GetCheck()) ? m_comboDeadlineIntervalType.GetCurSel() : -1,
		GetDlgItemInt(IDC_DEADLINE_INTERVAL_NUMBER));

	MoveTodoDate(dtRemindPivot, (m_radioRemindAfter.GetCheck()) ? m_comboRemindType.GetCurSel() : -1,
		GetDlgItemInt(IDC_REMIND_INTERVAL_NUMBER));

	if (dtRemindPivot > dtDeadlinePivot) {
		MessageBox("The remind interval extends beyond the deadline interval. Please correct this before saving.", "NexTech Practice", MB_ICONERROR | MB_OK);
		return FALSE;
	}

	return TRUE;
}

// (c.haag 2008-05-30 16:43) - Called to write the form data to member variables
void CEMRTodoActionConfigDlg::Save()
{
	if (NULL != m_dlCategory->CurSel) {
		m_nCommit_Category = VarLong( ((IRowSettingsPtr)m_dlCategory->CurSel)->Value[eclcat_ID], -1 );
	} else {
		m_nCommit_Category = -1;
	}

	// Validation would have prevented the following from being null
	long nCheck;
	m_strCommit_Method = VarString( ((IRowSettingsPtr)m_dlMethod->CurSel)->Value[eclmth_Text] );
	m_nCommit_Priority = VarLong( ((IRowSettingsPtr)m_dlPriority->CurSel)->Value[eclpri_ID] );
	m_anCommit_AssignTo.Copy(m_anAssignTo);
	GetDlgItemText(IDC_EDIT_NOTES, m_strCommit_Notes);
	GetDlgItemCheck(IDC_DEADLINE_IMMEDIATE, nCheck);
	if (nCheck) {
		m_nCommit_DeadlineType = -1;
		m_nCommit_DeadlineInterval = 0;
	} else {
		m_nCommit_DeadlineType = m_comboDeadlineIntervalType.GetCurSel();
		m_nCommit_DeadlineInterval = GetDlgItemInt(IDC_DEADLINE_INTERVAL_NUMBER);
	}
	
	GetDlgItemCheck(IDC_REMIND_IMMEDIATE, nCheck);
	if (nCheck) {
		m_nCommit_RemindType = -1;
		m_nCommit_RemindInterval = 0;
	} else {
		m_nCommit_RemindType = m_comboRemindType.GetCurSel();
		m_nCommit_RemindInterval = GetDlgItemInt(IDC_REMIND_INTERVAL_NUMBER);
	}
}

void CEMRTodoActionConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRTodoActionConfigDlg)
	DDX_Control(pDX, IDC_STATIC_ASSIGN_TO_MULTIPLE, m_nxlAssignToMultiple);
	DDX_Control(pDX, IDC_REMIND_TYPE, m_comboRemindType);
	DDX_Control(pDX, IDC_DEADLINE_INTERVAL_TYPE, m_comboDeadlineIntervalType);
	DDX_Control(pDX, IDC_DEADLINE_IMMEDIATE, m_radioDeadlineImmediate);
	DDX_Control(pDX, IDC_DEADLINE_TIME_AFTER, m_radioDeadlineAfter);
	DDX_Control(pDX, IDC_REMIND_IMMEDIATE, m_radioRemindImmediate);
	DDX_Control(pDX, IDC_REMIND_AFTER, m_radioRemindAfter);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_DEADLINE_INTERVAL_NUMBER, m_editDeadlineIntervalNumber);
	DDX_Control(pDX, IDC_REMIND_INTERVAL_NUMBER, m_editRemindIntervalNumber);
	DDX_Control(pDX, IDC_EDIT_NOTES, m_editNotes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRTodoActionConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRTodoActionConfigDlg)
	ON_BN_CLICKED(IDC_DEADLINE_IMMEDIATE, OnDeadlineImmediate)
	ON_BN_CLICKED(IDC_DEADLINE_TIME_AFTER, OnDeadlineTimeAfter)
	ON_BN_CLICKED(IDC_REMIND_IMMEDIATE, OnRemindImmediate)
	ON_BN_CLICKED(IDC_REMIND_AFTER, OnRemindAfter)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelLButtonDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRTodoActionConfigDlg message handlers

BOOL CEMRTodoActionConfigDlg::OnInitDialog() 
{
	try {
		IRowSettingsPtr pRow;
		CNxDialog::OnInitDialog();
	
		// (c.haag 2008-05-30 12:35) - Set the button icons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		// Bind the datalists
		m_dlCategory = BindNxDataList2Ctrl(IDC_COMBO_TODO_CATEGORY);
		_variant_t vNull;
		vNull.vt = VT_NULL;
		pRow = m_dlCategory->GetNewRow();
		pRow->Value[eclcat_ID] = vNull;
		pRow->Value[eclcat_Description] = _bstr_t(" { No Category } ");
		IRowSettingsPtr pNullCategoryRow = m_dlCategory->AddRowSorted(pRow, NULL);
		if (IsNew()) {
			m_dlCategory->CurSel = pNullCategoryRow;
		}
		m_dlAssignTo = BindNxDataList2Ctrl(IDC_COMBO_TODO_ASSIGN_TO);

		// Add the multiple user selection
		pRow = m_dlAssignTo->GetNewRow();
		pRow->Value[eclat_ID] = MULTIPLE_SELECTIONS;
		pRow->Value[eclat_Username] = _bstr_t("  { Multiple Users } ");
		m_dlAssignTo->AddRowSorted(pRow, NULL);
		
		// (c.haag 2008-06-25 09:24) - PLID 30505 - Add special sentinel values for TBD users
		pRow = m_dlAssignTo->GetNewRow();
		pRow->Value[eclat_ID] = (long)etsaLoggedInUser;
		pRow->Value[eclat_Username] = _bstr_t(" { Logged-In User }");
		m_dlAssignTo->AddRowSorted(pRow, NULL);

		pRow = m_dlAssignTo->GetNewRow();
		pRow->Value[eclat_ID] = (long)etsaPatientCoordinator;
		pRow->Value[eclat_Username] = _bstr_t(" { Patient Coordinator }");
		m_dlAssignTo->AddRowSorted(pRow, NULL);

		m_dlPriority = BindNxDataList2Ctrl(IDC_COMBO_TODO_PRIORITY, false);
		pRow = m_dlPriority->GetNewRow();
		pRow->PutValue(eclpri_ID, (long)3);
		pRow->PutValue(eclpri_Text, _bstr_t("Low"));
		m_dlPriority->AddRowAtEnd(pRow, NULL);
		pRow = m_dlPriority->GetNewRow();
		pRow->PutValue(eclpri_ID, (long)2);
		pRow->PutValue(eclpri_Text, _bstr_t("Medium"));
		m_dlPriority->AddRowAtEnd(pRow, NULL);
		pRow = m_dlPriority->GetNewRow();
		pRow->PutValue(eclpri_ID, (long)1);
		pRow->PutValue(eclpri_Text, _bstr_t("High"));
		m_dlPriority->AddRowAtEnd(pRow, NULL);
		
		m_dlMethod = BindNxDataList2Ctrl(IDC_COMBO_TODO_METHOD, false);
		pRow = m_dlMethod->GetNewRow();
		pRow->PutValue(0, _bstr_t("Phone"));
		m_dlMethod->AddRowAtEnd(pRow, NULL);
		pRow = m_dlMethod->GetNewRow();
		pRow->PutValue(0, _bstr_t("E-Mail"));
		m_dlMethod->AddRowAtEnd(pRow, NULL);
		pRow = m_dlMethod->GetNewRow();
		pRow->PutValue(0, _bstr_t("Fax"));
		m_dlMethod->AddRowAtEnd(pRow, NULL);
		pRow = m_dlMethod->GetNewRow();
		pRow->PutValue(0, _bstr_t("Letter"));
		m_dlMethod->AddRowAtEnd(pRow, NULL);
		pRow = m_dlMethod->GetNewRow();
		pRow->PutValue(0, _bstr_t("Other"));
		m_dlMethod->AddRowAtEnd(pRow, NULL);

		GetDlgItem(IDC_STATIC_ASSIGN_TO_MULTIPLE)->GetWindowRect(&m_rcAssignToMultiple);
		ScreenToClient(&m_rcAssignToMultiple);

		m_nxlAssignToMultiple.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxlAssignToMultiple.SetType(dtsHyperlink);
		m_nxlAssignToMultiple.SetSingleLine(true);

		// Populate initial dialog data
		if (IsNew()) {
			LoadDefaultValues();
		} else {
			LoadTodoAction();
		}
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (c.haag 2008-05-30 16:42) - Called to save changes to the todo action
void CEMRTodoActionConfigDlg::OnOK() 
{
	try {
		// Validate the form data before saving
		if (!Validate()) {
			return;
		}

		// Now save the form data
		Save();

		CNxDialog::OnOK();
	} NxCatchAll("Error in CEMRTodoActionConfigDlg::OnOK");
}

void CEMRTodoActionConfigDlg::OnDeadlineImmediate() 
{
	try {
		GetDlgItem(IDC_DEADLINE_INTERVAL_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEADLINE_INTERVAL_TYPE)->EnableWindow(FALSE);
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnDeadlineImmediate");
}

void CEMRTodoActionConfigDlg::OnDeadlineTimeAfter() 
{
	try {
		GetDlgItem(IDC_DEADLINE_INTERVAL_NUMBER)->EnableWindow(TRUE);
		GetDlgItem(IDC_DEADLINE_INTERVAL_TYPE)->EnableWindow(TRUE);
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnDeadlineTimeAfter");	
}

void CEMRTodoActionConfigDlg::OnRemindImmediate() 
{
	try {
		GetDlgItem(IDC_REMIND_INTERVAL_NUMBER)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMIND_TYPE)->EnableWindow(FALSE);
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnRemindImmediate");
}

void CEMRTodoActionConfigDlg::OnRemindAfter() 
{
	try {
		GetDlgItem(IDC_REMIND_INTERVAL_NUMBER)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMIND_TYPE)->EnableWindow(TRUE);

	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnRemindAfter");
}

BEGIN_EVENTSINK_MAP(CEMRTodoActionConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRTodoActionConfigDlg)
	ON_EVENT(CEMRTodoActionConfigDlg, IDC_COMBO_TODO_ASSIGN_TO, 16 /* SelChosen */, OnSelChosenComboTodoAssignTo, VTS_DISPATCH)
	ON_EVENT(CEMRTodoActionConfigDlg, IDC_COMBO_TODO_ASSIGN_TO, 1 /* SelChanging */, OnSelChangingComboTodoAssignTo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMRTodoActionConfigDlg, IDC_COMBO_TODO_CATEGORY, 1 /* SelChanging */, OnSelChangingComboTodoCategory, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMRTodoActionConfigDlg, IDC_COMBO_TODO_METHOD, 1 /* SelChanging */, OnSelChangingComboTodoMethod, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMRTodoActionConfigDlg, IDC_COMBO_TODO_PRIORITY, 1 /* SelChanging */, OnSelChangingComboTodoPriority, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (c.haag 2008-06-02 14:56) - Based on the current content of m_anAssignTo,
// update the visibility of the Assign To combo
void CEMRTodoActionConfigDlg::RefreshAssignToCombo()
{
	if (0 == m_anAssignTo.GetSize()) {
		m_dlAssignTo->CurSel = NULL;
		GetDlgItem(IDC_STATIC_ASSIGN_TO_MULTIPLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_TODO_ASSIGN_TO)->ShowWindow(SW_SHOW);
	}
	else if (1 == m_anAssignTo.GetSize()) {
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_dlAssignTo->TrySetSelByColumn_Deprecated(eclat_ID, m_anAssignTo[0]);
		GetDlgItem(IDC_STATIC_ASSIGN_TO_MULTIPLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_TODO_ASSIGN_TO)->ShowWindow(SW_SHOW);
	}
	else {
		// Calculate the Assign To text from data based on the selected values
		// (c.haag 2008-06-25 10:31) - PLID 30505 - Include sentinel values
		_RecordsetPtr prs = CreateRecordset(
			"SELECT Username FROM ("
			"   SELECT PersonID, Username FROM UsersT "
			"   UNION SELECT -1, ' { Logged-In User }' "
			"   UNION SELECT -2, ' { Patient Coordinator }' "
			") SubQ "
			"WHERE PersonID IN (%s) "
			"ORDER BY Username"
			,ArrayAsString(m_anAssignTo));
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
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_dlAssignTo->TrySetSelByColumn_Deprecated(eclat_ID, MULTIPLE_SELECTIONS);
		GetDlgItem(IDC_COMBO_TODO_ASSIGN_TO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_ASSIGN_TO_MULTIPLE)->ShowWindow(SW_SHOW);
	}
}

void CEMRTodoActionConfigDlg::OnSelChosenComboTodoAssignTo(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != lpRow) {
			// (c.haag 2008-06-27 12:49) - Use MULTIPLE_SELECTIONS instead of the traditional
			// -1 because -1 is reserved for the sentinel "Logged-In" user value.
			long nID = VarLong(pRow->Value[eclat_ID]);
			if (MULTIPLE_SELECTIONS == nID) {
				// Multiple assignee selection
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "UsersT");
				for (int i=0; i < m_anAssignTo.GetSize(); i++) {
					dlg.PreSelect(m_anAssignTo[i]);
				}
				dlg.m_strNameColTitle = "Username";
				// (c.haag 2008-06-25 10:31) - PLID 30505 - Include sentinel values
				if (IDOK == dlg.Open("(SELECT PersonT.ID, Username FROM PersonT INNER JOIN UsersT ON UsersT.PersonID = PersonT.ID WHERE PersonT.ID > 0 AND Archived = 0"
					"UNION SELECT -1, ' { Logged-In User }' "
					"UNION SELECT -2, ' { Patient Coordinator }' "
					") SubQ",
					"",
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
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnSelChosenComboTodoAssignTo");
}

void CEMRTodoActionConfigDlg::OnSelChangingComboTodoAssignTo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnSelChangingComboTodoAssignTo");
}

void CEMRTodoActionConfigDlg::OnSelChangingComboTodoCategory(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnSelChangingComboTodoCategory");
}

void CEMRTodoActionConfigDlg::OnSelChangingComboTodoMethod(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnSelChangingComboTodoMethod");
}

void CEMRTodoActionConfigDlg::OnSelChangingComboTodoPriority(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnSelChangingComboTodoPriority");
}

BOOL CEMRTodoActionConfigDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	// (c.haag 2008-06-02 15:30) - Ensure we use the correct hyperlink cursor
	if (m_anAssignTo.GetSize() > 1) {
		if (m_rcAssignToMultiple.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CEMRTodoActionConfigDlg::OnLabelLButtonDown(WPARAM wParam, LPARAM lParam) 
{
	// (c.haag 2008-06-02 15:29) - If the user clicked on the
	// Assign To hyperlink, invoke a selection change
	try {
		OnSelChosenComboTodoAssignTo(m_dlAssignTo->CurSel);
	}
	NxCatchAll("Error in CEMRTodoActionConfigDlg::OnLButtonDown");
	return 0;
}
