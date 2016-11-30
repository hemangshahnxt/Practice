// EMREMChecklistCodeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMREMChecklistCodeSetupDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-08-17 15:32) - PLID 27104 - created

// (c.haag 2007-09-11 15:55) - PLID 27353 - Changed all message boxes
// to use this dialog as their parent rather than the main window

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum ServiceCodeComboColumn {

	scccID = 0,
	scccCode,
	scccSubCode,
	scccDescription,
};

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistCodeSetupDlg dialog


CEMREMChecklistCodeSetupDlg::CEMREMChecklistCodeSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMREMChecklistCodeSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMREMChecklistCodeSetupDlg)
		m_nCodingLevelID = -1;
		m_nServiceID = -1;
		m_strServiceCode = "";
		m_nColumnsRequired = 1;
		m_nMinimumTimeRequired = 0;	// (j.jones 2007-09-17 10:05) - PLID 27396 - the minimum time required to satisfy this row (0 for not required)
		m_strDescription = "";
		m_bApproved = FALSE;
		m_nApprovalUserID = -1;
		m_strApprovalUserName = "";
		m_pChecklist = NULL;
		m_nPendingServiceID = -1;
		m_nCurApprovalUserID = -1;
		m_strCurApprovalUserName = "";
	//}}AFX_DATA_INIT
}


void CEMREMChecklistCodeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMREMChecklistCodeSetupDlg)
	DDX_Control(pDX, IDC_CHECK_CODING_LEVEL_APPROVED, m_btnApproved);
	DDX_Control(pDX, IDC_CHECK_REQUIRE_MINIMUM_TIME, m_btnRequireMin);
	DDX_Control(pDX, IDC_EDIT_COLUMNS_REQUIRED, m_nxeditEditColumnsRequired);
	DDX_Control(pDX, IDC_EDIT_TIME_REQUIRED, m_nxeditEditTimeRequired);
	DDX_Control(pDX, IDC_EDIT_CODE_DESCRIPTION, m_nxeditEditCodeDescription);
	DDX_Control(pDX, IDC_DESCRIPTION_CHANGED_LABEL, m_nxstaticDescriptionChangedLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMREMChecklistCodeSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMREMChecklistCodeSetupDlg)
	ON_EN_CHANGE(IDC_EDIT_COLUMNS_REQUIRED, OnChangeEditColumnsRequired)
	ON_BN_CLICKED(IDC_CHECK_CODING_LEVEL_APPROVED, OnCheckCodingLevelApproved)
	ON_EN_CHANGE(IDC_EDIT_CODE_DESCRIPTION, OnChangeEditCodeDescription)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_REQUIRE_MINIMUM_TIME, OnCheckRequireMinimumTime)
	ON_EN_CHANGE(IDC_EDIT_TIME_REQUIRED, OnChangeEditTimeRequired)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistCodeSetupDlg message handlers

BOOL CEMREMChecklistCodeSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-29 17:00) - PLID 29837 - NxIconify the OK and Cancel buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_ServiceCodeCombo = BindNxDataList2Ctrl(this, IDC_SERVICE_CODE_COMBO, GetRemoteData(), true);

		//load any passed-in information

		//set the service code
		if(m_nServiceID != -1) {

			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			long nSel = m_ServiceCodeCombo->TrySetSelByColumn_Deprecated(scccID,m_nServiceID);
			if(nSel == -1) {
				//maybe it's inactive?
				_RecordsetPtr rsCode = CreateRecordset("SELECT Code FROM CPTCodeT WHERE ID = %li", m_nServiceID);
				if(!rsCode->eof) {
					m_ServiceCodeCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCode, "Code", "")));
				}
				else 
					m_ServiceCodeCombo->CurSel = NULL;
			}
			else if(nSel == sriNoRowYet_WillFireEvent) {
				//store the pending service ID
				m_nPendingServiceID = m_nServiceID;
			}
		}

		//set the required columns
		SetDlgItemInt(IDC_EDIT_COLUMNS_REQUIRED, m_nColumnsRequired);

		// (j.jones 2007-09-17 10:09) - PLID 27396 - set the minimum time
		// 0 means it is not required
		if(m_nMinimumTimeRequired > 0) {
			CheckDlgButton(IDC_CHECK_REQUIRE_MINIMUM_TIME, TRUE);
			SetDlgItemInt(IDC_EDIT_TIME_REQUIRED, m_nMinimumTimeRequired);
		}
		else {
			GetDlgItem(IDC_EDIT_TIME_REQUIRED)->EnableWindow(FALSE);
		}

		//set the description
		SetDlgItemText(IDC_EDIT_CODE_DESCRIPTION, m_strDescription);

		//fill in the approved status
		if(m_bApproved) {
			
			CheckDlgButton(IDC_CHECK_CODING_LEVEL_APPROVED, TRUE);

			m_dtCurApproved = m_dtApproved;
			m_nCurApprovalUserID = m_nApprovalUserID;
			m_strCurApprovalUserName = m_strApprovalUserName;
			
			CString strApproved;
			strApproved.Format("Approved By %s on %s", m_strApprovalUserName, FormatDateTimeForInterface(m_dtApproved, NULL, dtoDateTime));

			SetDlgItemText(IDC_CHECK_CODING_LEVEL_APPROVED, strApproved);
		}
		else {
			m_dtCurApproved.SetStatus(COleDateTime::invalid);
			CheckDlgButton(IDC_CHECK_CODING_LEVEL_APPROVED, FALSE);
			SetDlgItemText(IDC_CHECK_CODING_LEVEL_APPROVED, "Approved");
		}

		//will show a warning if the description does not contain the service code
		CheckShowDescriptionChangedWarning(TRUE);

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMREMChecklistCodeSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMREMChecklistCodeSetupDlg)
	ON_EVENT(CEMREMChecklistCodeSetupDlg, IDC_SERVICE_CODE_COMBO, 16 /* SelChosen */, OnSelChosenServiceCodeCombo, VTS_DISPATCH)
	ON_EVENT(CEMREMChecklistCodeSetupDlg, IDC_SERVICE_CODE_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedServiceCodeCombo, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMREMChecklistCodeSetupDlg::OnSelChosenServiceCodeCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		//update our description
		RecalculateAndApplyNewDescription();

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnSelChosenServiceCodeCombo");
}

void CEMREMChecklistCodeSetupDlg::OnChangeEditColumnsRequired() 
{
	try {

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnChangeEditColumnsRequired");
}

void CEMREMChecklistCodeSetupDlg::OnCheckCodingLevelApproved() 
{
	try {
		// (j.jones 2007-08-29 11:38) - PLID 27135 - added permissions

		if(IsDlgButtonChecked(IDC_CHECK_CODING_LEVEL_APPROVED)) {

			//if they checked the box, check permissions, and uncheck if permission denied

			if(!CheckCurrentUserPermissions(bioAdminEMChecklist, sptDynamic0))
				CheckDlgButton(IDC_CHECK_CODING_LEVEL_APPROVED, FALSE);
		}

		// (j.jones 2007-08-29 11:38) - PLID 27135 - moved the interface setup to ReflectCodingLevelApprovalChange
		ReflectCodingLevelApprovalChange();

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnCheckCodingLevelApproved");
}

void CEMREMChecklistCodeSetupDlg::OnChangeEditCodeDescription() 
{
	try {

		//will show a warning if the description does not contain the service code
		CheckShowDescriptionChangedWarning(FALSE);

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnChangeEditCodeDescription");
}

void CEMREMChecklistCodeSetupDlg::OnTrySetSelFinishedServiceCodeCombo(long nRowEnum, long nFlags) 
{
	try {

		//don't need to keep our pending ID now
		m_nPendingServiceID = -1;

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsCode = CreateRecordset("SELECT Code FROM CPTCodeT WHERE ID = %li", m_nServiceID);
			if(!rsCode->eof) {
				m_ServiceCodeCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCode, "Code", "")));
			}
			else 
				m_ServiceCodeCombo->CurSel = NULL;
		}

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnTrySetSelFinishedServiceCodeCombo");
}

void CEMREMChecklistCodeSetupDlg::OnOK() 
{
	long nAuditTransactionID = -1;

	try {

		if(m_pChecklist == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//get the data, validate as needed

		long nServiceID = -1;
		CString strServiceCode = "";

		//pull and validate the service code
		{
			if(m_ServiceCodeCombo->GetIsTrySetSelPending()) {
				//TrySetSel is only called on load, so if we reach here, we are trying to save our changes
				//before it has finished loading the datalist.  This can happen on slow connections.
				if(m_nPendingServiceID != -1) {
					nServiceID = m_nPendingServiceID;
					strServiceCode = m_strServiceCode;
				}
			}
			else {
				IRowSettingsPtr pRow = m_ServiceCodeCombo->CurSel;
				if(pRow) {
					nServiceID = VarLong(pRow->GetValue(scccID),-1);
					strServiceCode = VarString(pRow->GetValue(scccCode),"");
				}
				else if(m_ServiceCodeCombo->IsComboBoxTextInUse) {
					//this would mean they didn't change the value,
					//so our member variable is unchanged
					nServiceID = m_nServiceID;
					strServiceCode = AsString(m_ServiceCodeCombo->ComboBoxText);
				}
			}

			//now, is it valid?
			if(nServiceID == -1) {
				MessageBox("You must select a service code for this coding level.", "Practice", MB_OK | MB_ICONERROR);
				return;
			}

			//is it already in use?
			for(int i=0;i<m_pChecklist->paryCodingLevelRows.GetSize();i++) {
				ChecklistCodingLevelInfo *pInfo = (ChecklistCodingLevelInfo*)(m_pChecklist->paryCodingLevelRows.GetAt(i));
				//obviously don't compare to the level we are editing, if we are editing an existing level
				if(pInfo->nID != m_nCodingLevelID) {
					if(pInfo->nServiceID == nServiceID) {
						MessageBox("The selected service code is already in use by another coding level.\n"
							"You may not have two coding level rows with the same service code.", "Practice", MB_OK | MB_ICONERROR);
						return;
					}
				}
			}
		}

		//pull and validate the number of columns required
		long nColumnsRequired = -1;
		{
			nColumnsRequired = GetDlgItemInt(IDC_EDIT_COLUMNS_REQUIRED);
			
			if(nColumnsRequired < 1) {
				MessageBox("You must have at least 1 column required to complete this coding level.", "Practice", MB_OK | MB_ICONERROR);
				return;
			}

			long nColumnCount = m_pChecklist->paryColumns.GetSize();
			if(nColumnsRequired > nColumnCount) {
				CString strWarning;
				strWarning.Format("The checklist currently has %li columns, and you have indicated %li columns are required for this coding level to be completed.\n"
					"Please reduce this requirement to no more than %li.", nColumnCount, nColumnsRequired, nColumnCount);
				MessageBox(strWarning, "Practice", MB_OK | MB_ICONERROR);
				return;
			}
		}

		// (j.jones 2007-09-17 10:13) - PLID 27396 - pull and validate the minimum time required
		long nMinimumTimeRequired = 0;
		if(IsDlgButtonChecked(IDC_CHECK_REQUIRE_MINIMUM_TIME)) {
			nMinimumTimeRequired = GetDlgItemInt(IDC_EDIT_TIME_REQUIRED);
			
			if(nMinimumTimeRequired < 1) {
				MessageBox("You must have a positive value in the 'minimum time required' field to complete this coding level.", "Practice", MB_OK | MB_ICONERROR);
				return;
			}

			if(nMinimumTimeRequired > 1440) {
				MessageBox("Please enter a 'minimum time required' no greater than 1440 minutes (24 hours).", "Practice", MB_OK | MB_ICONERROR);
				return;
			}
		}

		//pull and validate the description
		CString strDescription;
		{
			GetDlgItemText(IDC_EDIT_CODE_DESCRIPTION, strDescription);

			strDescription.TrimLeft();
			strDescription.TrimRight();

			if(strDescription.IsEmpty()) {
				MessageBox("The description for this coding level is blank. You must type in a description that contains at least the service code, to serve as a label for this row.", "Practice", MB_OK | MB_ICONERROR);
				return;
			}

			if(strDescription.Find(strServiceCode) == -1) {
				CString strWarning;
				strWarning.Format("The description for this coding level does not contain the service code '%s'.\n"
					"You must type in a description that contains the service code, to serve as a label for this row.", strServiceCode);
				MessageBox(strWarning, "Practice", MB_OK | MB_ICONERROR);
				return;
			}
		}

		//pull and validate the approval info
		BOOL bApproved = IsDlgButtonChecked(IDC_CHECK_CODING_LEVEL_APPROVED);
		CString strApprovedBy = "NULL";
		CString strApprovedDate = "NULL";

		if(!bApproved) {
			if(IDNO == MessageBox("This coding level must be marked as approved before the checklist can be used on a patient's EMN.\n"
				"Are you sure you wish to save this unapproved coding level?","Practice",MB_ICONQUESTION|MB_YESNO))
				return;
		}
		else {
			strApprovedBy.Format("%li", m_nCurApprovalUserID);
			strApprovedDate.Format("'%s'", FormatDateTimeForSql(m_dtCurApproved));
		}

		//save the data

		long nCodingLevelID = m_nCodingLevelID;

		if(nCodingLevelID == -1) {

			//new record
			nCodingLevelID = NewNumber("EMChecklistCodingLevelsT", "ID");

			// (j.jones 2007-09-17 10:20) - PLID 27396 - added MinimumTime
			ExecuteSql("INSERT INTO EMChecklistCodingLevelsT (ID, ChecklistID, ServiceID, MinColumns, Description, "
				"Approved, ApprovedBy, ApprovedDate, MinimumTime) "
				"VALUES (%li, %li, %li, %li, '%s', %li, %s, %s, %li) ",
				nCodingLevelID, m_pChecklist->nID, nServiceID, nColumnsRequired, _Q(strDescription),
				bApproved ? 1 : 0, strApprovedBy, strApprovedDate, nMinimumTimeRequired);

			//audit the creation
			long nAuditID = BeginNewAuditEvent();

			//replace newlines with spaces
			CString strNewValue = strDescription;
			strNewValue.Replace("\r\n", "  ");

			AuditEvent(-1, "", nAuditID, aeiEMChecklistCodingLevelCreated, m_pChecklist->nID, "", strNewValue, aepMedium, aetCreated);
		}
		else {
			//update the existing record

			// (j.jones 2007-09-17 10:20) - PLID 27396 - added MinimumTime
			ExecuteSql("UPDATE EMChecklistCodingLevelsT SET ServiceID = %li, MinColumns = %li, Description = '%s', "
				"Approved = %li, ApprovedBy = %s, ApprovedDate = %s, MinimumTime = %li "
				"WHERE ID = %li",
				nServiceID, nColumnsRequired, _Q(strDescription),
				bApproved ? 1 : 0, strApprovedBy, strApprovedDate, nMinimumTimeRequired,
				nCodingLevelID);

			//audit the changes
			
			if(m_nServiceID != nServiceID) {

				//changed service IDs

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelServiceID, m_pChecklist->nID, m_strServiceCode, strServiceCode, aepMedium, aetChanged);
			}

			if(m_nColumnsRequired != nColumnsRequired) {

				//changed the number of columns required

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldDesc = m_strDescription;
				strOldDesc.Replace("\r\n", "  ");
				CString strNewDesc = strDescription;
				strNewDesc.Replace("\r\n", "  ");

				CString strOldValue, strNewValue;
				strOldValue.Format("%li columns (%s)", m_nColumnsRequired, strOldDesc);
				strNewValue.Format("%li columns (%s)", nColumnsRequired, strNewDesc);

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelMinColumns, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			// (j.jones 2007-09-17 10:09) - PLID 27396 - audit the minimum time
			// 0 means it is not required
			if(m_nMinimumTimeRequired != nMinimumTimeRequired) {

				//changed the minimum time required

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldDesc = m_strDescription;
				strOldDesc.Replace("\r\n", "  ");
				CString strNewDesc = strDescription;
				strNewDesc.Replace("\r\n", "  ");

				CString strOldTimeRequired, strNewTimeRequired;
				if(m_nMinimumTimeRequired <= 0)
					strOldTimeRequired = "No time required";
				else
					strOldTimeRequired.Format("%li minutes required", m_nMinimumTimeRequired);
				if(nMinimumTimeRequired <= 0)
					strNewTimeRequired = "No time required";
				else
					strNewTimeRequired.Format("%li minutes required", nMinimumTimeRequired);

				CString strOldValue, strNewValue;
				strOldValue.Format("%s (%s)", strOldTimeRequired, strOldDesc);
				strNewValue.Format("%s (%s)", strNewTimeRequired, strNewDesc);

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelMininumTime, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			if(m_strDescription != strDescription) {

				//changed the description

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldValue = m_strDescription;
				strOldValue.Replace("\r\n", "  ");
				CString strNewValue = strDescription;
				strNewValue.Replace("\r\n", "  ");

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelDesc, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			if(m_bApproved != bApproved || m_nApprovalUserID != m_nCurApprovalUserID || m_dtApproved != m_dtCurApproved) {

				//approval information changed

				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				//replace newlines with spaces
				CString strOldDesc = m_strDescription;
				strOldDesc.Replace("\r\n", "  ");
				CString strNewDesc = strDescription;
				strNewDesc.Replace("\r\n", "  ");

				CString strOldValue, strNewValue;
				if(!m_bApproved) {
					strOldValue.Format("Not Approved (%s)", strOldDesc);
				}
				else {
					strOldValue.Format("Approved by %s on %s (%s)", m_strApprovalUserName, FormatDateTimeForInterface(m_dtApproved, NULL, dtoDateTime), strOldDesc);
				}
				if(!bApproved) {
					strNewValue.Format("Not Approved (%s)", strNewDesc);
				}
				else {
					strNewValue.Format("Approved by %s on %s (%s)", m_strCurApprovalUserName, FormatDateTimeForInterface(m_dtCurApproved, NULL, dtoDateTime), strNewDesc);
				}

				AuditEvent(-1, "", nAuditTransactionID, aeiEMChecklistCodingLevelApproved, m_pChecklist->nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			if(nAuditTransactionID != -1)
				CommitAuditTransaction(nAuditTransactionID);
		}

		//now update our member variables with the changes

		m_nCodingLevelID = nCodingLevelID;
		m_nServiceID = nServiceID;
		m_strServiceCode = strServiceCode;
		m_nColumnsRequired = nColumnsRequired;
		// (j.jones 2007-09-17 10:17) - PLID 27396 - store the new time
		m_nMinimumTimeRequired = nMinimumTimeRequired;
		m_strDescription = strDescription;
		m_bApproved = bApproved;		
		m_dtApproved = m_dtCurApproved;
		m_nApprovalUserID = m_nCurApprovalUserID;
		m_strApprovalUserName = m_strCurApprovalUserName;

		// (j.jones 2007-10-01 12:36) - PLID 27104 - close and return now, if successful
		CDialog::OnOK();
		return;
	
	}NxCatchAllCall("Error in CEMREMChecklistCodeSetupDlg::OnOK", 
		// (j.jones 2007-10-01 12:36) - PLID 27104 - rollback the audit transaction if we have one, and had an exception
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

//will uncheck the approved box if checked
void CEMREMChecklistCodeSetupDlg::CheckUndoApproval()
{
	if(IsDlgButtonChecked(IDC_CHECK_CODING_LEVEL_APPROVED)) {

		//uncheck the box
		CheckDlgButton(IDC_CHECK_CODING_LEVEL_APPROVED, FALSE);
		ReflectCodingLevelApprovalChange();
	}
}

//generates a description of the contents of this coding level
void CEMREMChecklistCodeSetupDlg::GenerateCodingLevelDescription()
{
	try {

		CString strServiceCode = "";

		//pull the service code
		{
			if(m_ServiceCodeCombo->GetIsTrySetSelPending()) {
				//TrySetSel is only called on load, so if we reach here, we are trying to save our changes
				//before it has finished loading the datalist.  This can happen on slow connections.
				if(m_nPendingServiceID != -1) {
					strServiceCode = m_strServiceCode;
				}
			}
			else {
				IRowSettingsPtr pRow = m_ServiceCodeCombo->CurSel;
				if(pRow) {
					strServiceCode = VarString(pRow->GetValue(scccCode),"");
				}
				else if(m_ServiceCodeCombo->IsComboBoxTextInUse) {
					//this would mean they didn't change the value,
					//so our member variable is unchanged
					strServiceCode = AsString(m_ServiceCodeCombo->ComboBoxText);
				}
			}
		}

		//now assign the service code to our stored value
		m_strCurGeneratedCodingLevelDesc = strServiceCode;

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::GenerateCodingLevelDescription");
}

//determines if we show or hide the description warning label
void CEMREMChecklistCodeSetupDlg::DisplayDescriptionChangedWarning(BOOL bShow)
{
	try {

		m_bShowDescriptionChangedWarning = bShow;
		GetDlgItem(IDC_DESCRIPTION_CHANGED_LABEL)->ShowWindow(m_bShowDescriptionChangedWarning ? SW_SHOW : SW_HIDE);

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::DisplayDescriptionChangedWarning");
}

//compares our stored description to the displayed description, optionally recalculating the stored description
void CEMREMChecklistCodeSetupDlg::CheckShowDescriptionChangedWarning(BOOL bRecalculateDesc)
{
	try {
	
		//see if we need to recalculate the coding level description
		if(bRecalculateDesc)
			GenerateCodingLevelDescription();

		CString strGetCurDisplayDesc;
		GetDlgItemText(IDC_EDIT_CODE_DESCRIPTION, strGetCurDisplayDesc);

		//show/hide the warning based on whether the user-entered description does not contain the service code
		DisplayDescriptionChangedWarning(strGetCurDisplayDesc.Find(m_strCurGeneratedCodingLevelDesc) == -1);
	
	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::CheckShowDescriptionChangedWarning");
}

//potentially updates the description with our changes
void CEMREMChecklistCodeSetupDlg::RecalculateAndApplyNewDescription()
{
	try {

		//first get the current display desc
		CString strGetCurDisplayDesc;
		GetDlgItemText(IDC_EDIT_CODE_DESCRIPTION, strGetCurDisplayDesc);

		BOOL bHasCurDescription = strGetCurDisplayDesc.Find(m_strCurGeneratedCodingLevelDesc) != -1;

		CString strOldCalculatedDesc = m_strCurGeneratedCodingLevelDesc;

		//recalculate the description based on the current data
		GenerateCodingLevelDescription();

		//if the current description doesn't have our calculated one inside it,
		//just warn, otherwise replace it
		if(!bHasCurDescription)
			CheckShowDescriptionChangedWarning(FALSE);
		else {

			strGetCurDisplayDesc.Replace(strOldCalculatedDesc, m_strCurGeneratedCodingLevelDesc);
			SetDlgItemText(IDC_EDIT_CODE_DESCRIPTION, m_strCurGeneratedCodingLevelDesc);
		}

		CheckShowDescriptionChangedWarning(FALSE);

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::RecalculateAndApplyNewDescription");
}

HBRUSH CEMREMChecklistCodeSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(m_bShowDescriptionChangedWarning && pWnd->GetDlgCtrlID() == IDC_DESCRIPTION_CHANGED_LABEL)
	{
		pDC->SetTextColor(RGB(174,0,0));	//this maroon color works in 256-color TS sessions
	}

	return hbr;
}

// (j.jones 2007-08-29 11:36) - PLID 27135 - changed the approval code for permission purposes
void CEMREMChecklistCodeSetupDlg::ReflectCodingLevelApprovalChange()
{
	if(IsDlgButtonChecked(IDC_CHECK_CODING_LEVEL_APPROVED)) {

		//they checked the box

		//store the time this occurred
		m_dtCurApproved = COleDateTime::GetCurrentTime();;

		//and the user that did this
		m_nCurApprovalUserID = GetCurrentUserID();
		m_strCurApprovalUserName = GetCurrentUserName();

		//update the display
		CString strApproved;
		strApproved.Format("Approved By %s on %s", m_strCurApprovalUserName, FormatDateTimeForInterface(m_dtCurApproved, NULL, dtoDateTime));
		SetDlgItemText(IDC_CHECK_CODING_LEVEL_APPROVED, strApproved);
	}
	else {
		//the unchecked the box

		//update the current member variables
		m_dtCurApproved.SetStatus(COleDateTime::invalid);
		m_nCurApprovalUserID = -1;
		m_strCurApprovalUserName = "";

		//update the display
		SetDlgItemText(IDC_CHECK_CODING_LEVEL_APPROVED, "Approved");
	}
}

// (j.jones 2007-09-17 10:08) - PLID 27396 - added minimum time
void CEMREMChecklistCodeSetupDlg::OnCheckRequireMinimumTime() 
{
	try {

		//enable/disable the minimum time field
		GetDlgItem(IDC_EDIT_TIME_REQUIRED)->EnableWindow(IsDlgButtonChecked(IDC_CHECK_REQUIRE_MINIMUM_TIME));

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnCheckRequireMinimumTime");
}

// (j.jones 2007-09-17 10:08) - PLID 27396 - added minimum time
void CEMREMChecklistCodeSetupDlg::OnChangeEditTimeRequired() 
{
	try {

		//undo the approval
		CheckUndoApproval();

	}NxCatchAll("Error in CEMREMChecklistCodeSetupDlg::OnChangeEditTimeRequired");
}
