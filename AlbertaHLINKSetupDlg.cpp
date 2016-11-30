// AlbertaHLINKSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlbertaHLINKSetupDlg.h"

// CAlbertaHLINKSetupDlg dialog

// (j.jones 2010-10-12 14:52) - PLID 40901 - created

using namespace NXDATALIST2Lib;

//each dropdown only has two columns
enum AlbertaColumns {

	acID = 0,
	acName = 1,
};

IMPLEMENT_DYNAMIC(CAlbertaHLINKSetupDlg, CNxDialog)

CAlbertaHLINKSetupDlg::CAlbertaHLINKSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAlbertaHLINKSetupDlg::IDD, pParent)
{
	m_nHealthNumberCustomField = -1;
	m_bHealthNumberCustomFieldChanged = FALSE;
}

CAlbertaHLINKSetupDlg::~CAlbertaHLINKSetupDlg()
{
}

void CAlbertaHLINKSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_ALBERTA_SUBMITTER_PREFIX, m_nxeditSubmitterPrefix);
}


BEGIN_MESSAGE_MAP(CAlbertaHLINKSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDC_HLINK_SETUP_HELP, &CAlbertaHLINKSetupDlg::OnBnClickedHlinkSetupHelp)
END_MESSAGE_MAP()

// CAlbertaHLINKSetupDlg message handlers

BOOL CAlbertaHLINKSetupDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CAlbertaHLINKSetupDlg-Number", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'Alberta_PatientULICustomField' OR "
				"Name = 'Alberta_PatientRegNumberCustomField' OR "
				"Name = 'Alberta_ProviderBAIDCustomField' OR "
				"Name = 'Alberta_SubmitterPrefixCustomField' "
				")",
				_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CAlbertaHLINKSetupDlg-Text", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'Alberta_SubmitterPrefix' OR "
				"Name = 'Alberta_PayToCode' "
				")",
				_Q(GetCurrentUserName()));

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.dinatale 2012-12-28 10:00) - PLID 54370 - allow users to choose the submitter prefix, and add the none row
		m_SubmitterPrefixCombo = BindNxDataList2Ctrl(IDC_ALBERTA_SUBMITPREFIX_COMBO, true);
		m_PatientULICombo = BindNxDataList2Ctrl(IDC_ALBERTA_PATIENT_ULI_COMBO, true);
		m_RegistrationNumberCombo = BindNxDataList2Ctrl(IDC_ALBERTA_REGISTRATION_NUMBER_COMBO, true);
		m_ProviderBAIDCombo = BindNxDataList2Ctrl(IDC_ALBERTA_PROV_BA_ID_COMBO, true);
		m_PayToCodeCombo = BindNxDataList2Ctrl(IDC_ALBERTA_PAY_TO_CODE_COMBO, false);

		//fill the Pay To Codes
		IRowSettingsPtr pRow = m_PayToCodeCombo->GetNewRow();
		pRow->PutValue(acID, (LPCTSTR)"BAPY");
		pRow->PutValue(acName, (LPCTSTR)"Business Arrangement");
		m_PayToCodeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_PayToCodeCombo->GetNewRow();
		pRow->PutValue(acID, (LPCTSTR)"PRVD");
		pRow->PutValue(acName, (LPCTSTR)"Service Provider");
		m_PayToCodeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_PayToCodeCombo->GetNewRow();
		pRow->PutValue(acID, (LPCTSTR)"RECP");
		pRow->PutValue(acName, (LPCTSTR)"Service Recipient");
		m_PayToCodeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_PayToCodeCombo->GetNewRow();
		pRow->PutValue(acID, (LPCTSTR)"CONT");
		pRow->PutValue(acName, (LPCTSTR)"Contract Holder");
		m_PayToCodeCombo->AddRowAtEnd(pRow, NULL);		
		pRow = m_PayToCodeCombo->GetNewRow();
		pRow->PutValue(acID, (LPCTSTR)"OTHR");
		pRow->PutValue(acName, (LPCTSTR)"Other");
		m_PayToCodeCombo->AddRowAtEnd(pRow, NULL);

		//prefixes cannot exceed 3 characters
		m_nxeditSubmitterPrefix.SetLimitText(3);

		CString strSubmitterPrefix = GetRemotePropertyText("Alberta_SubmitterPrefix", "", 0, "<None>", true);
		m_nxeditSubmitterPrefix.SetWindowText(strSubmitterPrefix);

		// (j.dinatale 2012-12-28 10:27) - PLID 54370 - submitter custom field combo
		long nSubmitterPrefixCustomField = GetRemotePropertyInt("Alberta_SubmitterPrefixCustomField", -1, 0, "<None>", true);
		if(nSubmitterPrefixCustomField > 0){
			m_SubmitterPrefixCombo->SetSelByColumn(acID, nSubmitterPrefixCustomField);
		}
		
		//internally it's called ULI for Unique Lifetime Identifier, but for Patients it is more commonly
		//known as the Patienr Health Number, PHN, so we display PHN for the label
		m_nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
		m_PatientULICombo->SetSelByColumn(acID, m_nHealthNumberCustomField);

		long nPatientRegNumberCustomField = GetRemotePropertyInt("Alberta_PatientRegNumberCustomField", 2, 0, "<None>", true);
		m_RegistrationNumberCombo->SetSelByColumn(acID, nPatientRegNumberCustomField);

		long nProviderBAIDCustomField = GetRemotePropertyInt("Alberta_ProviderBAIDCustomField", 6, 0, "<None>", true);
		m_ProviderBAIDCombo->SetSelByColumn(acID, nProviderBAIDCustomField);

		CString strPayToCode = GetRemotePropertyText("Alberta_PayToCode", "BAPY", 0, "<None>", true);
		if(m_PayToCodeCombo->SetSelByColumn(acID, (LPCTSTR)strPayToCode) == NULL) {
			//just incase we ever enter in some as-yet-unknown override in ConfigRT, show that code anyways
			m_PayToCodeCombo->PutComboBoxText((LPCTSTR)strPayToCode);
		}

	}NxCatchAll("Error in CAlbertaHLINKSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAlbertaHLINKSetupDlg::OnOK() 
{
	try {

		CString strSubmitterPrefix = "";
		m_nxeditSubmitterPrefix.GetWindowText(strSubmitterPrefix);
		strSubmitterPrefix.TrimLeft();
		strSubmitterPrefix.TrimRight();
		m_nxeditSubmitterPrefix.SetWindowText(strSubmitterPrefix);

		// (j.dinatale 2013-01-14 16:16) - PLID 54370 - need to change our error to refer to the default submitter prefix
		//prefixes cannot exceed 3 characters (we will allow them to be blank though)
		if(strSubmitterPrefix.GetLength() != 3) {
			AfxMessageBox("The default submitter prefix must be 3 characters.");
			return;
		}
	
		IRowSettingsPtr pPatientULIRow = m_PatientULICombo->GetCurSel();
		if(pPatientULIRow == NULL) {
			AfxMessageBox("A value must be selected for the Patient Health Number.");
			return;
		}

		IRowSettingsPtr pRegNumberRow = m_RegistrationNumberCombo->GetCurSel();
		if(pRegNumberRow == NULL) {
			AfxMessageBox("A value must be selected for the Registration Number.");
			return;
		}

		IRowSettingsPtr pProvBAIDRow = m_ProviderBAIDCombo->GetCurSel();
		if(pProvBAIDRow == NULL) {
			AfxMessageBox("A value must be selected for the Provider Business Arrangement ID.");
			return;
		}

		IRowSettingsPtr pPayToRow = m_PayToCodeCombo->GetCurSel();
		if(pPayToRow == NULL && !m_PayToCodeCombo->IsComboBoxTextInUse) {
			//only validate the row if the combo box text was not in use
			AfxMessageBox("A value must be selected for the Pay To Code.");
			return;
		}

		SetRemotePropertyText("Alberta_SubmitterPrefix", strSubmitterPrefix, 0, "<None>");

		// (j.dinatale 2012-12-28 10:34) - PLID 54370 - set the preference for the submitter combo
		IRowSettingsPtr pSubmitterPrefixRow = m_SubmitterPrefixCombo->GetCurSel();
		long nSubPrefixCustomField = -1;
		if(pSubmitterPrefixRow){
			nSubPrefixCustomField = VarLong(pSubmitterPrefixRow->GetValue(acID), -1);
		}
		SetRemotePropertyInt("Alberta_SubmitterPrefixCustomField", nSubPrefixCustomField, 0, "<None>");

		long nPatientULICustomField = VarLong(pPatientULIRow->GetValue(acID), -1);

		// (j.jones 2010-11-08 14:06) - PLID 39620 - did this field change?
		if(m_nHealthNumberCustomField != nPatientULICustomField) {
			m_bHealthNumberCustomFieldChanged = TRUE;
			m_nHealthNumberCustomField = nPatientULICustomField;
		}

		SetRemotePropertyInt("Alberta_PatientULICustomField", nPatientULICustomField, 0, "<None>");

		long nPatientRegNumberCustomField = VarLong(pRegNumberRow->GetValue(acID), -1);
		SetRemotePropertyInt("Alberta_PatientRegNumberCustomField", nPatientRegNumberCustomField, 0, "<None>");

		long nProviderBAIDCustomField = VarLong(pProvBAIDRow->GetValue(acID), -1);
		SetRemotePropertyInt("Alberta_ProviderBAIDCustomField", nProviderBAIDCustomField, 0, "<None>");

		if(pPayToRow) {
			CString strPayToCode = VarString(pPayToRow->GetValue(acID), "");
			SetRemotePropertyText("Alberta_PayToCode", strPayToCode, 0, "<None>");
		}

		CNxDialog::OnOK();
	
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-12-28 10:19) - PLID 54370 - submitter prefix datalist
BEGIN_EVENTSINK_MAP(CAlbertaHLINKSetupDlg, CNxDialog)
ON_EVENT(CAlbertaHLINKSetupDlg, IDC_ALBERTA_SUBMITPREFIX_COMBO, 16, CAlbertaHLINKSetupDlg::SelChosenAlbertaSubmitprefixCombo, VTS_DISPATCH)
ON_EVENT(CAlbertaHLINKSetupDlg, IDC_ALBERTA_SUBMITPREFIX_COMBO, 18, CAlbertaHLINKSetupDlg::RequeryFinishedAlbertaSubmitprefixCombo, VTS_I2)
END_EVENTSINK_MAP()

// (j.dinatale 2012-12-28 10:19) - PLID 54370 - submitter prefix datalist
void CAlbertaHLINKSetupDlg::SelChosenAlbertaSubmitprefixCombo(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow = IRowSettingsPtr(lpRow);
		if(pRow){
			long nID = VarLong(pRow->GetValue(acID), -1);
			if(nID == -1){
				m_SubmitterPrefixCombo->CurSel = NULL;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-12-28 10:19) - PLID 54370 - submitter prefix datalist
void CAlbertaHLINKSetupDlg::RequeryFinishedAlbertaSubmitprefixCombo(short nFlags)
{
	try{
		IRowSettingsPtr pNoneRow = m_SubmitterPrefixCombo->GetNewRow();
		pNoneRow->PutValue(acID, (long)-1);
		pNoneRow->PutValue(acName, _bstr_t("<None>"));
		m_SubmitterPrefixCombo->AddRowBefore(pNoneRow, m_SubmitterPrefixCombo->GetFirstRow());
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-12-31 14:40) - PLID 54370 - help button to explain the new setup
void CAlbertaHLINKSetupDlg::OnBnClickedHlinkSetupHelp()
{
	try{
		this->MessageBox(
			"The Default Submitter Prefix is used when the custom field representing the Submitter Prefix is "
			"blank or when there is no custom field selected. If you wish all providers to have the same Submitter "
			"Prefix, then select <None> from the Submitter Prefix dropdown and fill in the Default Submitter Prefix. "
			"If you wish to have some or all providers to have a different Submitter Prefix, select the custom field that contains the prefix."
			, "Alberta Submitter Prefix");
	}NxCatchAll(__FUNCTION__);
}
