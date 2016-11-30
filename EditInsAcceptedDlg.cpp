// EditInsAcceptedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "EditInsAcceptedDlg.h"
#include "AuditTrail.h"
#include "OHIPUtils.h"
#include "GlobalFinancialUtils.h"
#include "BlankAssignmentBenefitsDlg.h"
#include "MarkAllInsAcceptedDlg.h"
#include "AlbertaHLINKUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

#define	ID_ACCEPT_ALL_INS	42345
#define	ID_ACCEPT_NO_INS	42346

enum ProviderColumns {

	pcID = 0,
	pcName,
	pcAccepted,
};

/////////////////////////////////////////////////////////////////////////////
// CEditInsAcceptedDlg dialog


CEditInsAcceptedDlg::CEditInsAcceptedDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditInsAcceptedDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditInsAcceptedDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditInsAcceptedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditInsAcceptedDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_RADIO_ACCEPT_ALL_INS, m_radioAlwaysAccept);
	DDX_Control(pDX, IDC_RADIO_ACCEPT_NO_INS, m_radioNeverAccept);	
	DDX_Control(pDX, IDC_LABEL_WARN_ASSIGNMENT_OF_BENEFITS_ACCEPTASSIGN, m_nxstaticAssignmentOfBenefitsWarningLabel);
	DDX_Control(pDX, IDC_BTN_WARN_ASSIGNMENT_OF_BENEFITS_ACCEPTASSIGN, m_btnAssignmentOfBenefitsWarning);
	DDX_Control(pDX, IDC_BTN_UPDATE_ALL_ACCEPTED, m_btnUpdateAllAccepted);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditInsAcceptedDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditInsAcceptedDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO_ACCEPT_ALL_INS, OnRadioAcceptAllIns)
	ON_BN_CLICKED(IDC_RADIO_ACCEPT_NO_INS, OnRadioAcceptNoIns)
	ON_BN_CLICKED(IDC_BTN_WARN_ASSIGNMENT_OF_BENEFITS_ACCEPTASSIGN, OnBtnWarnAssignmentOfBenefitsAcceptAssignment)
	ON_BN_CLICKED(IDC_BTN_UPDATE_ALL_ACCEPTED, OnBtnUpdateAllAccepted)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditInsAcceptedDlg message handlers

void CEditInsAcceptedDlg::OnOK() 
{	
	CDialog::OnOK();
}

BOOL CEditInsAcceptedDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 09:53) - PLID 29817 - NxIconify the Close button
		m_btnOK.AutoSet(NXB_CLOSE);

		// (j.jones 2010-07-30 14:34) - PLID 39917 - moved the ability to mark all accepted / not accepted
		// into a separate, permissioned dialog
		m_btnUpdateAllAccepted.AutoSet(NXB_MODIFY);

		//check the dynamic permission
		if(!(GetCurrentUserPermissions(bioInsuranceCo) & (sptDynamic0|sptDynamic0WithPass))) {
			m_btnUpdateAllAccepted.EnableWindow(FALSE);
		}

		// (j.jones 2010-07-23 17:20) - PLID 34105 - set up the assignment of benefits warning label
		m_nxstaticAssignmentOfBenefitsWarningLabel.SetColor(RGB(255,0,0));
		extern CPracticeApp theApp;
		m_nxstaticAssignmentOfBenefitsWarningLabel.SetFont(&theApp.m_boldFont);
		m_btnAssignmentOfBenefitsWarning.SetIcon(IDI_BLUE_QUESTION);
		UpdateAssignmentOfBenefitsWarningLabel();

		BOOL bAcceptByDefault = (GetRemotePropertyInt("DefaultInsAcceptAssignment",1,0,"<None>",TRUE) == 1);
		m_radioAlwaysAccept.SetCheck(bAcceptByDefault);
		m_radioNeverAccept.SetCheck(!bAcceptByDefault);

		m_ProviderList = BindNxDataListCtrl(this,IDC_PROVIDER_ACCEPTED_LIST,GetRemoteData(),true);
	}
	NxCatchAll("Error in CEditInsAcceptedDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditInsAcceptedDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditInsAcceptedDlg)
	ON_EVENT(CEditInsAcceptedDlg, IDC_PROVIDER_ACCEPTED_LIST, 10 /* EditingFinished */, OnEditingFinishedProviderAcceptedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditInsAcceptedDlg, IDC_PROVIDER_ACCEPTED_LIST, 18 /* RequeryFinished */, OnRequeryFinishedProviderAcceptedList, VTS_I2)
	ON_EVENT(CEditInsAcceptedDlg, IDC_PROVIDER_ACCEPTED_LIST, 6 /* RButtonDown */, OnRButtonDownProviderAcceptedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditInsAcceptedDlg::OnEditingFinishedProviderAcceptedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if(nRow == -1)
			return;

		long nProviderID = m_ProviderList->GetValue(m_ProviderList->GetCurSel(),pcID).lVal;

		long data = 0;
		if(m_ProviderList->GetValue(m_ProviderList->GetCurSel(),pcAccepted).boolVal)
			data = 1;

		long nRecordsAffected = 0;

		ExecuteSql(&nRecordsAffected, adCmdText, 
			// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here. If we
			// do, then nRecordsAffected may never be updated.
			"SET NOCOUNT OFF "
			"UPDATE InsuranceAcceptedT SET Accepted = %li WHERE InsuranceCoID = %li AND ProviderID = %li", data, m_iInsuranceCoID, nProviderID);

		if(nRecordsAffected == 0)
			ExecuteSql("INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) VALUES (%li,%li,%li)", m_iInsuranceCoID, nProviderID, data);

		//auditing
		CString strInsCoName, strProviderName, strOld, strNew;
		strProviderName = VarString(m_ProviderList->GetValue(m_ProviderList->GetCurSel(),pcName),"");
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM InsuranceCoT WHERE PersonID = %li",m_iInsuranceCoID);
		if(!rs->eof) {
			strInsCoName = AdoFldString(rs, "Name","");
		}
		rs->Close();

		strOld.Format("%sAccepted by %s", (data == 1) ? "Not " : "", strProviderName);
		strNew.Format("%sAccepted by %s", (data == 0) ? "Not " : "", strProviderName);

		long AuditID = BeginNewAuditEvent();
		AuditEvent(-1, strInsCoName,AuditID,aeiInsCoAccepted,m_iInsuranceCoID,strOld,strNew,aepMedium,aetChanged);

		// (j.jones 2010-07-23 17:20) - PLID 34105 - update the assignment of benefits warning label
		UpdateAssignmentOfBenefitsWarningLabel();

	}NxCatchAll("Error editing accepted list.");
}

void CEditInsAcceptedDlg::OnRequeryFinishedProviderAcceptedList(short nFlags) 
{
	//we have to take in to account that there may not be records for a provider

	try {
		
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProviderID, Accepted FROM InsuranceAcceptedT WHERE InsuranceCoID = {INT}", m_iInsuranceCoID);
		while(!rs->eof) {
			BOOL bAccepted = AdoFldBool(rs, "Accepted",TRUE);
			long nProviderID = AdoFldLong(rs, "ProviderID",-1);

			long row = m_ProviderList->FindByColumn(pcID,nProviderID,0,FALSE);
			if(row != -1)
				m_ProviderList->PutValue(row,pcAccepted,_variant_t(long(bAccepted ? 1 : 0)));

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error loading list.");
}

void CEditInsAcceptedDlg::OnRButtonDownProviderAcceptedList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_ProviderList->CurSel = nRow;

	if(nRow == -1)
		return;

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();
	if(nRow != -1) {
		CString str, strProv;
		strProv = VarString(m_ProviderList->GetValue(nRow, pcName),"");
		str.Format("Mark all Insurance Companies Accepted for %s",strProv);
		mnu.InsertMenu(0, MF_BYPOSITION, ID_ACCEPT_ALL_INS, str);
		str.Format("Mark all Insurance Companies as Not Accepted for %s",strProv);
		mnu.InsertMenu(1, MF_BYPOSITION, ID_ACCEPT_NO_INS, str);
	}

	CPoint pt;
	GetCursorPos(&pt);
	mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, this, NULL);
}

BOOL CEditInsAcceptedDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch(wParam) {
		case ID_ACCEPT_ALL_INS:

			try {

				CString strProvName = VarString(m_ProviderList->GetValue(m_ProviderList->GetCurSel(), pcName),"");

				CString str;
				str.Format("This will mark all insurance companies as being accepted for provider %s.\n"
					"HCFA Box 27, Accept Assignment, will send 'No'.\n\n"
					"Are you sure you wish to do this?", strProvName);

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					break;
				}

				long nProvID = VarLong(m_ProviderList->GetValue(m_ProviderList->GetCurSel(), pcID));

				ExecuteSql("UPDATE InsuranceAcceptedT SET Accepted = 1 WHERE ProviderID = %li", nProvID);
				ExecuteSql("INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) "
					"SELECT PersonID, %li, 1 FROM InsuranceCoT WHERE PersonID NOT IN ("
					"SELECT InsuranceCoID FROM InsuranceAcceptedT WHERE ProviderID = %li"
					")", nProvID, nProvID);	

				// (j.jones 2010-07-23 10:06) - PLID 25217 - audit that this button was clicked
				CString strNew;
				strNew.Format("Added Accept Assignment to all Insurance Companies for Provider '%s'", strProvName);
				long AuditID = BeginNewAuditEvent();
				AuditEvent(-1, "<All Companies>", AuditID, aeiInsCoAccepted, -1, "", strNew, aepHigh, aetChanged);

				m_ProviderList->PutValue(m_ProviderList->GetCurSel(),pcAccepted,_variant_t(long(1)));

				// (j.jones 2010-07-23 17:20) - PLID 34105 - update the assignment of benefits warning label
				UpdateAssignmentOfBenefitsWarningLabel();

			}NxCatchAll("Error marking all insurance companies accepted for a provider");
			break;

		case ID_ACCEPT_NO_INS:

			try {

				CString strProvName = VarString(m_ProviderList->GetValue(m_ProviderList->GetCurSel(), pcName),"");

				CString str;
				str.Format("This will mark all insurance companies as not being accepted for provider %s.\n"
					"HCFA Box 27, Accept Assignment, will send 'Yes'.\n\n"
					"Are you sure you wish to do this?", strProvName);

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					break;
				}

				long nProvID = VarLong(m_ProviderList->GetValue(m_ProviderList->GetCurSel(), pcID));

				// (j.jones 2010-07-23 15:18) - PLID 39783 - changed this code to ensure
				// we always try to fill InsuranceAcceptedT
				ExecuteSql("UPDATE InsuranceAcceptedT SET Accepted = 0 WHERE ProviderID = %li", nProvID);
				ExecuteSql("INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) "
					"SELECT PersonID, %li, 0 FROM InsuranceCoT WHERE PersonID NOT IN ("
					"SELECT InsuranceCoID FROM InsuranceAcceptedT WHERE ProviderID = %li"
					")", nProvID, nProvID);	

				// (j.jones 2010-07-23 10:06) - PLID 25217 - audit that this button was clicked
				CString strNew;
				strNew.Format("Removed Accept Assignment from all Insurance Companies for Provider '%s'", strProvName);
				long AuditID = BeginNewAuditEvent();
				AuditEvent(-1, "<All Companies>", AuditID, aeiInsCoAccepted, -1, "", strNew, aepHigh, aetChanged);

				m_ProviderList->PutValue(m_ProviderList->GetCurSel(),pcAccepted,_variant_t(long(0)));

				// (j.jones 2010-07-23 17:20) - PLID 34105 - update the assignment of benefits warning label
				UpdateAssignmentOfBenefitsWarningLabel();

			}NxCatchAll("Error marking no insurance companies accepted for a provider");
			break;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

// (j.jones 2010-07-23 09:10) - PLID 25217 - moved preference features into Practice
void CEditInsAcceptedDlg::OnRadioAcceptAllIns()
{
	try {

		BOOL bAcceptByDefault = TRUE;

		if(m_radioNeverAccept.GetCheck()) {
			bAcceptByDefault = FALSE;
		}

		SetRemotePropertyInt("DefaultInsAcceptAssignment", bAcceptByDefault ? 1 : 0, 0, "<None>");

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-23 09:10) - PLID 25217 - moved preference features into Practice
void CEditInsAcceptedDlg::OnRadioAcceptNoIns()
{
	try {

		//handle both in one function
		OnRadioAcceptAllIns();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-23 17:16) - PLID 34105 - called to show/hide the assignment of benefits warning label
void CEditInsAcceptedDlg::UpdateAssignmentOfBenefitsWarningLabel()
{
	try {

		// (j.jones 2010-07-23 17:17) - PLID 34105 - if not OHIP,
		// warn if any setup can have assignment of benefits not filled
		// (this does not account for individual bills with Box 13 overridden)
		// (j.jones 2010-07-27 09:29) - PLID 39854 - CanAssignmentOfBenefitsBeBlank can
		// now filter for HCFAs or UBs, but in this dialog we want to show both
		// (j.jones 2010-11-08 13:58) - PLID 39620 - skip if Alberta
		if(!UseOHIP() && !UseAlbertaHLINK() && CanAssignmentOfBenefitsBeBlank(babtBothForms)) {
			m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_SHOW);
			m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_SHOW);
		}
		else {
			m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_HIDE);
			m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-23 17:18) - PLID 34105 - only shown if assignment of benefits can be blank,
// clicking this button should explain why the warning is displayed
void CEditInsAcceptedDlg::OnBtnWarnAssignmentOfBenefitsAcceptAssignment()
{
	try {

		CBlankAssignmentBenefitsDlg dlg(this);
		// (j.jones 2010-07-27 09:47) - PLID 39854 - show info. for both form types
		dlg.m_babtType = babtBothForms;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-30 14:34) - PLID 39917 - moved the ability to mark all accepted / not accepted
// into a separate, permissioned dialog
void CEditInsAcceptedDlg::OnBtnUpdateAllAccepted()
{
	try {

		//check the dynamic permission
		if(!CheckCurrentUserPermissions(bioInsuranceCo, sptDynamic0)) {
			return;
		}

		CMarkAllInsAcceptedDlg dlg(this);
		if(dlg.DoModal() != IDCANCEL) {

			//refresh the providers
			m_ProviderList->Requery();

			//update our warning label
			UpdateAssignmentOfBenefitsWarningLabel();
		}

	}NxCatchAll(__FUNCTION__);
}
