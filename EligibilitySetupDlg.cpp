// EligibilitySetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilitySetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-06-23 09:00) - PLID 30434 - created

/////////////////////////////////////////////////////////////////////////////
// CEligibilitySetupDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ProviderComboColumns {

	pccID = 0,
	pccName,
};

enum LocationComboColumns {

	lccID = 0,
	lccName,
};

enum Combo2100C_Columns {

	c2100CcID = 0,
	c2100CcName,
};

enum Combo2100D_Columns {

	c2100DcID = 0,
	c2100DcName,
};

CEligibilitySetupDlg::CEligibilitySetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilitySetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEligibilitySetupDlg)
		m_nGroupID = -1;
		m_nProviderID = -1;
		m_nLocationID = -1;
		m_bIsLoading = FALSE;
		m_bHasChanged = FALSE;
	//}}AFX_DATA_INIT
}

void CEligibilitySetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEligibilitySetupDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_2100B_VALUE_2, m_nxedit2100B_Value_2);
	DDX_Control(pDX, IDC_2100B_VALUE_1, m_nxedit2100B_Value_1);
	DDX_Control(pDX, IDC_2100B_QUAL_2, m_nxedit2100B_Qual_2);
	DDX_Control(pDX, IDC_2100B_QUAL_1, m_nxedit2100B_Qual_1);
	DDX_Control(pDX, IDC_CHECK_USE_2100B_2, m_checkUse2100B_2);
	DDX_Control(pDX, IDC_CHECK_USE_2100B_1, m_checkUse2100B_1);
	DDX_Control(pDX, IDC_GROUP_LABEL, m_nxstaticGroupLabel);
	DDX_Control(pDX, IDC_2100B_QUAL_NM108, m_nxedit2100B_NM109_Qual);
	DDX_Control(pDX, IDC_2100B_VALUE_NM109, m_nxedit2100B_NM109_Value);
	DDX_Control(pDX, IDC_CHECK_USE_2100B_NM109, m_checkUse2100B_NM109);
	DDX_Control(pDX, IDC_RADIO_USE_PROV_NPI_ELIG, m_radioProvNPI);
	DDX_Control(pDX, IDC_RADIO_USE_LOC_NPI_ELIG, m_radioLocNPI);
	DDX_Control(pDX, IDC_CHECK_USE_TITLE_IN_LAST, m_checkUseTitleInLast);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEligibilitySetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEligibilitySetupDlg)
	ON_BN_CLICKED(IDC_CHECK_USE_2100B_1, OnCheckUse2100b1)
	ON_BN_CLICKED(IDC_CHECK_USE_2100B_2, OnCheckUse2100b2)
	ON_BN_CLICKED(IDC_CHECK_USE_2100B_NM109, OnCheckUse2100bNm109)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEligibilitySetupDlg message handlers

BOOL CEligibilitySetupDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetDlgItemText(IDC_GROUP_LABEL, m_strGroupName);

		ASSERT(m_nGroupID != -1);

		m_ProviderCombo = BindNxDataList2Ctrl(IDC_ELIG_PROVIDER_COMBO, true);
		m_LocationCombo = BindNxDataList2Ctrl(IDC_ELIG_LOCATION_COMBO, true);
		m_2100C_Combo = BindNxDataList2Ctrl(IDC_2100C_REF_COMBO, false);
		// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
		m_2100D_Combo = BindNxDataList2Ctrl(IDC_2100D_REF_COMBO, false);

		//set our limits
		m_nxedit2100B_Qual_1.SetLimitText(10);
		m_nxedit2100B_Value_1.SetLimitText(50);
		m_nxedit2100B_Qual_2.SetLimitText(10);
		m_nxedit2100B_Value_2.SetLimitText(50);
		// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
		m_nxedit2100B_NM109_Qual.SetLimitText(10);
		m_nxedit2100B_NM109_Value.SetLimitText(50);

		//build the 2100C combo
		IRowSettingsPtr pRow = m_2100C_Combo->GetNewRow();
		pRow->PutValue(c2100CcID, (long)value2100C_None),
		pRow->PutValue(c2100CcName, "<Do Not Fill>"),
		m_2100C_Combo->AddRowAtEnd(pRow, NULL);
		pRow = m_2100C_Combo->GetNewRow();
		pRow->PutValue(c2100CcID, (long)value2100C_SSN),
		pRow->PutValue(c2100CcName, "Social Security Number"),
		m_2100C_Combo->AddRowAtEnd(pRow, NULL);
		pRow = m_2100C_Combo->GetNewRow();
		pRow->PutValue(c2100CcID, (long)value2100C_PolicyGroupNum),
		pRow->PutValue(c2100CcName, "Policy Group Number"),
		m_2100C_Combo->AddRowAtEnd(pRow, NULL);

		// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
		//build the 2100D combo
		pRow = m_2100D_Combo->GetNewRow();
		pRow->PutValue(c2100DcID, (long)value2100D_None),
		pRow->PutValue(c2100DcName, "<Do Not Fill>"),
		m_2100D_Combo->AddRowAtEnd(pRow, NULL);
		pRow = m_2100D_Combo->GetNewRow();
		pRow->PutValue(c2100DcID, (long)value2100D_SSN),
		pRow->PutValue(c2100DcName, "Social Security Number"),
		m_2100D_Combo->AddRowAtEnd(pRow, NULL);
		pRow = m_2100D_Combo->GetNewRow();
		pRow->PutValue(c2100DcID, (long)value2100D_PolicyGroupNum),
		pRow->PutValue(c2100DcName, "Policy Group Number"),
		m_2100D_Combo->AddRowAtEnd(pRow, NULL);

		//default our settings
		m_checkUse2100B_1.SetCheck(FALSE);
		m_nxedit2100B_Qual_1.SetReadOnly(TRUE);
		m_nxedit2100B_Value_1.SetReadOnly(TRUE);
		OnCheckUse2100b1();
		m_checkUse2100B_2.SetCheck(FALSE);
		m_nxedit2100B_Qual_2.SetReadOnly(TRUE);
		m_nxedit2100B_Value_2.SetReadOnly(TRUE);		
		OnCheckUse2100b2();
		// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
		m_checkUse2100B_NM109.SetCheck(FALSE);
		m_nxedit2100B_NM109_Qual.SetReadOnly(TRUE);
		m_nxedit2100B_NM109_Value.SetReadOnly(TRUE);		
		OnCheckUse2100bNm109();

		long nEligibility_2100C_REF_Option = value2100C_SSN;
		// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
		long nEligibility_2100D_REF = value2100D_PolicyGroupNum;
		// (j.jones 2011-06-15 14:33) - PLID 42181 - added NPI toggle
		long nEligNPIUsage = 1;
		// (j.jones 2013-07-08 15:08) - PLID 57469 - added EligTitleInLast
		long nEligTitleInLast = 0;

		//doesn't need to be in the Load() since it will only happen once
		// (j.jones 2011-06-15 14:33) - PLID 42181 - added NPI toggle
		// (j.jones 2013-07-08 15:08) - PLID 57469 - added EligTitleInLast
		_RecordsetPtr rs = CreateParamRecordset("SELECT Eligibility_2100C_REF_Option, "
			"Eligibility_2100D_REF, EligNPIUsage, EligTitleInLast "
			"FROM HCFASetupT WHERE ID = {INT} ", m_nGroupID);
		if(!rs->eof) {
			nEligibility_2100C_REF_Option = AdoFldLong(rs, "Eligibility_2100C_REF_Option", value2100C_SSN);
			nEligibility_2100D_REF = AdoFldLong(rs, "Eligibility_2100D_REF", value2100D_PolicyGroupNum);
			nEligNPIUsage = AdoFldLong(rs, "EligNPIUsage", 1);
			nEligTitleInLast = AdoFldLong(rs, "EligTitleInLast", 0);
		}
		rs->Close();

		m_2100C_Combo->SetSelByColumn(c2100CcID, (long)nEligibility_2100C_REF_Option);
		m_2100D_Combo->SetSelByColumn(c2100DcID, (long)nEligibility_2100D_REF);

		// (j.jones 2011-06-15 14:33) - PLID 42181 - added NPI toggle
		if(nEligNPIUsage == 1) {
			//location NPI
			m_radioLocNPI.SetCheck(TRUE);
		}
		else {
			//provider NPI
			m_radioProvNPI.SetCheck(TRUE);
		}

		// (j.jones 2013-07-08 15:08) - PLID 57469 - added UseTitleInLast
		m_checkUseTitleInLast.SetCheck(nEligTitleInLast);

		m_bHasChanged = FALSE;

	}NxCatchAll("Error in CEligibilitySetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEligibilitySetupDlg::Load()
{
	try {

		if(m_nProviderID == -1) {
			return;
		}

		if(m_nLocationID == -1) {
			return;
		}

		m_bIsLoading = TRUE;

		BOOL bUse2100B_REF_1 = FALSE;
		CString strANSI_2100B_REF_Qual_1 = "";
		CString strANSI_2100B_REF_Value_1 = "";
		BOOL bUse2100B_REF_2 = FALSE;
		CString strANSI_2100B_REF_Qual_2 = "";
		CString strANSI_2100B_REF_Value_2 = "";
		// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
		BOOL bUse2100B_NM109 = FALSE;
		CString strANSI_2100B_NM109_Qual = "";
		CString strANSI_2100B_NM109_Value = "";

		_RecordsetPtr rs = CreateParamRecordset("SELECT Use2100B_REF_1, ANSI_2100B_REF_Qual_1, ANSI_2100B_REF_Value_1, "
			"Use2100B_REF_2, ANSI_2100B_REF_Qual_2, ANSI_2100B_REF_Value_2, "
			"Use2100B_NM109, ANSI_2100B_NM109_Qual, ANSI_2100B_NM109_Value "
			"FROM EligibilitySetupT "
			"WHERE HCFASetupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}", m_nGroupID, m_nProviderID, m_nLocationID);

		if(!rs->eof) {

			bUse2100B_REF_1 = AdoFldBool(rs, "Use2100B_REF_1", FALSE);
			strANSI_2100B_REF_Qual_1 = AdoFldString(rs, "ANSI_2100B_REF_Qual_1", "");
			strANSI_2100B_REF_Value_1 = AdoFldString(rs, "ANSI_2100B_REF_Value_1", "");
			bUse2100B_REF_2 = AdoFldBool(rs, "Use2100B_REF_2", FALSE);
			strANSI_2100B_REF_Qual_2 = AdoFldString(rs, "ANSI_2100B_REF_Qual_2", "");
			strANSI_2100B_REF_Value_2 = AdoFldString(rs, "ANSI_2100B_REF_Value_2", "");

			// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
			bUse2100B_NM109 = AdoFldBool(rs, "Use2100B_NM109", FALSE);
			strANSI_2100B_NM109_Qual = AdoFldString(rs, "ANSI_2100B_NM109_Qual", "");
			strANSI_2100B_NM109_Value = AdoFldString(rs, "ANSI_2100B_NM109_Value", "");
		}
		rs->Close();

		m_checkUse2100B_1.SetCheck(bUse2100B_REF_1);
		SetDlgItemText(IDC_2100B_QUAL_1, strANSI_2100B_REF_Qual_1);
		SetDlgItemText(IDC_2100B_VALUE_1, strANSI_2100B_REF_Value_1);
		OnCheckUse2100b1();

		m_checkUse2100B_2.SetCheck(bUse2100B_REF_2);
		SetDlgItemText(IDC_2100B_QUAL_2, strANSI_2100B_REF_Qual_2);
		SetDlgItemText(IDC_2100B_VALUE_2, strANSI_2100B_REF_Value_2);
		OnCheckUse2100b2();

		m_checkUse2100B_NM109.SetCheck(bUse2100B_NM109);
		SetDlgItemText(IDC_2100B_QUAL_NM108, strANSI_2100B_NM109_Qual);
		SetDlgItemText(IDC_2100B_VALUE_NM109, strANSI_2100B_NM109_Value);
		OnCheckUse2100bNm109();

		m_bHasChanged = FALSE;

	}NxCatchAll("Error in CEligibilitySetupDlg::Load");

	m_bIsLoading = FALSE;
}

BOOL CEligibilitySetupDlg::Save()
{
	try {

		if(m_nProviderID == -1) {
			return FALSE;
		}

		if(m_nLocationID == -1) {
			return FALSE;
		}
		
		long nEligibility_2100C_REF_Option = value2100C_None;
		IRowSettingsPtr pRow = m_2100C_Combo->GetCurSel();
		if(pRow) {
			nEligibility_2100C_REF_Option = VarLong(pRow->GetValue(c2100CcID), value2100C_None);
		}

		// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
		long nEligibility_2100D_REF = value2100D_None;
		pRow = m_2100D_Combo->GetCurSel();
		if(pRow) {
			nEligibility_2100D_REF = VarLong(pRow->GetValue(c2100DcID), value2100D_None);
		}

		BOOL bUse2100B_REF_1 = m_checkUse2100B_1.GetCheck();
		CString strANSI_2100B_REF_Qual_1 = "";
		GetDlgItemText(IDC_2100B_QUAL_1, strANSI_2100B_REF_Qual_1);
		CString strANSI_2100B_REF_Value_1 = "";
		GetDlgItemText(IDC_2100B_VALUE_1, strANSI_2100B_REF_Value_1);

		BOOL bUse2100B_REF_2 = m_checkUse2100B_2.GetCheck();
		CString strANSI_2100B_REF_Qual_2 = "";
		GetDlgItemText(IDC_2100B_QUAL_2, strANSI_2100B_REF_Qual_2);
		CString strANSI_2100B_REF_Value_2 = "";
		GetDlgItemText(IDC_2100B_VALUE_2, strANSI_2100B_REF_Value_2);

		// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
		BOOL bUse2100B_NM109 = m_checkUse2100B_NM109.GetCheck();
		CString strANSI_2100B_NM109_Qual = "";
		GetDlgItemText(IDC_2100B_QUAL_NM108, strANSI_2100B_NM109_Qual);
		CString strANSI_2100B_NM109_Value = "";
		GetDlgItemText(IDC_2100B_VALUE_NM109, strANSI_2100B_NM109_Value);

		strANSI_2100B_REF_Qual_1.TrimLeft();
		strANSI_2100B_REF_Qual_1.TrimRight();
		strANSI_2100B_REF_Value_1.TrimLeft();
		strANSI_2100B_REF_Value_1.TrimRight();
		strANSI_2100B_REF_Qual_2.TrimLeft();
		strANSI_2100B_REF_Qual_2.TrimRight();
		strANSI_2100B_REF_Value_2.TrimLeft();
		strANSI_2100B_REF_Value_2.TrimRight();
		strANSI_2100B_NM109_Qual.TrimLeft();
		strANSI_2100B_NM109_Qual.TrimRight();
		strANSI_2100B_NM109_Value.TrimLeft();
		strANSI_2100B_NM109_Value.TrimRight();

		//disallow saving blank values
		if(bUse2100B_REF_1) {
			if(strANSI_2100B_REF_Qual_1.IsEmpty() || strANSI_2100B_REF_Value_1.IsEmpty()) {
				AfxMessageBox("You cannot send blank values in the 2100B REF segment. Please correct this before saving.");
				return FALSE;
			}
		}

		if(bUse2100B_REF_2) {
			if(strANSI_2100B_REF_Qual_2.IsEmpty() || strANSI_2100B_REF_Value_2.IsEmpty()) {
				AfxMessageBox("You cannot send blank values in the Additional 2100B REF segment. Please correct this before saving.");
				return FALSE;
			}
		}

		if(bUse2100B_NM109) {
			if(strANSI_2100B_NM109_Qual.IsEmpty() || strANSI_2100B_NM109_Value.IsEmpty()) {
				AfxMessageBox("You cannot send blank values in the 2100B NM109 segment. Please correct this before saving.");
				return FALSE;
			}
		}

		//disallow using the same qualifier twice, warn if they use the same number twice
		if(bUse2100B_REF_1 && bUse2100B_REF_2) {

			if(strANSI_2100B_REF_Qual_1.CompareNoCase(strANSI_2100B_REF_Qual_2) == 0) {
				AfxMessageBox("You cannot send the same qualifier twice for the 2100B REF segments. Please correct this before saving.");
				return FALSE;
			}

			//they may not have changed these values, but it is never really going to be correct,
			//so warn if anything changed, and the values are equal
			if(m_bHasChanged && strANSI_2100B_REF_Value_1.CompareNoCase(strANSI_2100B_REF_Value_2) == 0) {
				if(IDNO == MessageBox("You are sending the same value twice for the 2100B REF segments. Are you sure you wish to save this information?",
					"Practice", MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}

		// (j.jones 2011-06-15 14:33) - PLID 42181 - added NPI toggle
		long nEligNPIUsage = 1;
		if(m_radioProvNPI.GetCheck()) {
			nEligNPIUsage = 2;
		}

		// (j.jones 2013-07-08 15:08) - PLID 57469 - added UseTitleInLast
		long nEligTitleInLast = m_checkUseTitleInLast.GetCheck() ? 1 : 0;

		//first save the HCFASetup option
		// (j.jones 2010-05-25 15:43) - PLID 38868 - added Eligibility_2100D_REF
		// (j.jones 2011-06-15 14:33) - PLID 42181 - added NPI toggle
		// (j.jones 2013-07-08 15:08) - PLID 57469 - added UseTitleInLast
		ExecuteParamSql("UPDATE HCFASetupT SET Eligibility_2100C_REF_Option = {INT}, "
			"Eligibility_2100D_REF = {INT}, EligNPIUsage = {INT}, EligTitleInLast = {INT} "
			"WHERE ID = {INT}", nEligibility_2100C_REF_Option,
			nEligibility_2100D_REF, nEligNPIUsage, nEligTitleInLast,
			m_nGroupID);

		long nRecordsAffected = 0;
		//can't use parameters and get the records affected
		// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
		// (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT - You can use parameters and get the records affected now
		ExecuteParamSql(GetRemoteData(), &nRecordsAffected,
			"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
			"UPDATE EligibilitySetupT SET Use2100B_REF_1 = {INT}, ANSI_2100B_REF_Qual_1 = {STRING}, ANSI_2100B_REF_Value_1 = {STRING}, "
			"Use2100B_REF_2 = {INT}, ANSI_2100B_REF_Qual_2 = {STRING}, ANSI_2100B_REF_Value_2 = {STRING}, "
			"Use2100B_NM109 = {INT}, ANSI_2100B_NM109_Qual = {STRING}, ANSI_2100B_NM109_Value = {STRING} "
			"WHERE HCFASetupID = {INT} AND ProviderID = {INT} AND LocationID = {INT} ",
			bUse2100B_REF_1 ? 1 : 0, strANSI_2100B_REF_Qual_1, strANSI_2100B_REF_Value_1, 
			bUse2100B_REF_2 ? 1 : 0, strANSI_2100B_REF_Qual_2, strANSI_2100B_REF_Value_2, 
			bUse2100B_NM109 ? 1 : 0, strANSI_2100B_NM109_Qual, strANSI_2100B_NM109_Value, 
			m_nGroupID, m_nProviderID, m_nLocationID);

		if(nRecordsAffected == 0) {
			//record doesn't exist, so we must create it
			ExecuteParamSql("INSERT INTO EligibilitySetupT (Use2100B_REF_1, ANSI_2100B_REF_Qual_1, ANSI_2100B_REF_Value_1, "
				"Use2100B_REF_2, ANSI_2100B_REF_Qual_2, ANSI_2100B_REF_Value_2, "
				"Use2100B_NM109, ANSI_2100B_NM109_Qual, ANSI_2100B_NM109_Value, "
				"HCFASetupID, ProviderID, LocationID) "
				"VALUES ({INT}, {STRING}, {STRING}, "
				"{INT}, {STRING}, {STRING}, "
				"{INT}, {STRING}, {STRING}, "
				"{INT}, {INT}, {INT}) ",
				bUse2100B_REF_1 ? 1 : 0, strANSI_2100B_REF_Qual_1, strANSI_2100B_REF_Value_1, 
				bUse2100B_REF_2 ? 1 : 0, strANSI_2100B_REF_Qual_2, strANSI_2100B_REF_Value_2, 
				bUse2100B_NM109 ? 1 : 0, strANSI_2100B_NM109_Qual, strANSI_2100B_NM109_Value, 
				m_nGroupID, m_nProviderID, m_nLocationID);
		}

		return TRUE;

	}NxCatchAll("Error in CEligibilitySetupDlg::Save");

	return FALSE;
}

void CEligibilitySetupDlg::OnOK() 
{
	try {

		if(!Save()) {
			return;
		}
	
		CNxDialog::OnOK();

	}NxCatchAll("Error in CEligibilitySetupDlg::OnOK");
}

void CEligibilitySetupDlg::OnCancel() 
{
	try {
	
		CNxDialog::OnCancel();

	}NxCatchAll("Error in CEligibilitySetupDlg::OnCancel");
}

void CEligibilitySetupDlg::OnCheckUse2100b1() 
{
	try {

		BOOL bEnabled = m_checkUse2100B_1.GetCheck();
		m_nxedit2100B_Qual_1.SetReadOnly(!bEnabled);
		m_nxedit2100B_Value_1.SetReadOnly(!bEnabled);

		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}
	
	}NxCatchAll("Error in CEligibilitySetupDlg::OnCheckUse2100b1");
}

void CEligibilitySetupDlg::OnCheckUse2100b2() 
{
	try {

		BOOL bEnabled = m_checkUse2100B_2.GetCheck();
		m_nxedit2100B_Qual_2.SetReadOnly(!bEnabled);
		m_nxedit2100B_Value_2.SetReadOnly(!bEnabled);

		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}
	
	}NxCatchAll("Error in CEligibilitySetupDlg::OnCheckUse2100b2");
}

BEGIN_EVENTSINK_MAP(CEligibilitySetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEligibilitySetupDlg)
	ON_EVENT(CEligibilitySetupDlg, IDC_ELIG_PROVIDER_COMBO, 16 /* SelChosen */, OnSelChosenEligProviderCombo, VTS_DISPATCH)
	ON_EVENT(CEligibilitySetupDlg, IDC_ELIG_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenEligLocationCombo, VTS_DISPATCH)
	ON_EVENT(CEligibilitySetupDlg, IDC_ELIG_PROVIDER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedEligProviderCombo, VTS_I2)
	ON_EVENT(CEligibilitySetupDlg, IDC_ELIG_LOCATION_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedEligLocationCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEligibilitySetupDlg::OnSelChosenEligProviderCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//try to re-select our provider if we have one
			if(m_nProviderID != -1) {
				pRow = m_ProviderCombo->SetSelByColumn(pccID, (long)m_nProviderID);
				//if still NULL, how is it we have a valid provider ID?
				if(pRow == NULL) {
					//don't reset the ID, just assert and return
					ASSERT(FALSE);
					return;
				}
			}
			else {
				return;
			}
		}

		if(m_nProviderID != -1) {
			//check to see if they are still on the same provider
			long nProvID = VarLong(pRow->GetValue(pccID));
			if (nProvID != m_nProviderID) {
				if (m_bHasChanged) {
					if(IDYES == MessageBox("Any changes made to the previous provider / location combination will be saved.\n"
						"Do you still wish to switch providers?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		
						if(!Save()) {
							//the save failed, don't load and overwrite their changes yet
							m_ProviderCombo->SetSelByColumn(pccID, (long)m_nProviderID);
							return;
						}
					}
					else {
						//don't load and overwrite their changes yet
						m_ProviderCombo->SetSelByColumn(pccID, (long)m_nProviderID);
						return;
					}
				}
			}
			else {
				//don't load and overwrite the changes
				return;
			}
		}

		m_nProviderID = VarLong(pRow->GetValue(pccID));

		Load();
	
	}NxCatchAll("Error in CEligibilitySetupDlg::OnSelChosenEligProviderCombo");
}

void CEligibilitySetupDlg::OnSelChosenEligLocationCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//try to re-select our location if we have one
			if(m_nLocationID != -1) {
				pRow = m_LocationCombo->SetSelByColumn(lccID, (long)m_nLocationID);
				//if still NULL, how is it we have a valid location ID?
				if(pRow == NULL) {
					//don't reset the ID, just assert and return
					ASSERT(FALSE);
					return;
				}
			}
			else {
				return;
			}
		}

		if(m_nLocationID != -1) {
			//check to see if they are still on the same location
			long nLocID = VarLong(pRow->GetValue(lccID));
			if (nLocID != m_nLocationID) {
				if (m_bHasChanged) {
					if(IDYES == MessageBox("Any changes made to the previous provider / location combination will be saved.\n"
						"Do you still wish to switch locations?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		
						if(!Save()) {
							//the save failed, don't load and overwrite their changes yet
							m_LocationCombo->SetSelByColumn(lccID, (long)m_nLocationID);
							return;
						}
					}
					else {
						//don't load and overwrite their changes yet
						m_LocationCombo->SetSelByColumn(lccID, (long)m_nLocationID);
						return;
					}
				}
			}
			else {
				//don't load and overwrite the changes
				return;
			}
		}

		m_nLocationID = VarLong(pRow->GetValue(lccID));

		Load();
	
	}NxCatchAll("Error in CEligibilitySetupDlg::OnSelChosenEligLocationCombo");
}

BOOL CEligibilitySetupDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (HIWORD(wParam)) {
		
			case EN_CHANGE: {

				//doesn't matter which edit box was changed
				if (!m_bIsLoading) {
					m_bHasChanged = TRUE;
				}

				break;
			}			
		}

	}NxCatchAll("Error in CEligibilitySetupDlg::OnCommand");
	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CEligibilitySetupDlg::OnRequeryFinishedEligProviderCombo(short nFlags) 
{
	try {

		IRowSettingsPtr pProvRow = m_ProviderCombo->GetFirstRow();		
		m_ProviderCombo->PutCurSel(pProvRow);

		IRowSettingsPtr pLocRow = m_LocationCombo->GetCurSel();

		if(pProvRow != NULL && pLocRow != NULL) {
			m_nProviderID = VarLong(pProvRow->GetValue(pccID),-1);
			m_nLocationID = VarLong(pLocRow->GetValue(lccID),-1);

			Load();
		}

	}NxCatchAll("Error in CEligibilitySetupDlg::OnRequeryFinishedEligProviderCombo");
}

void CEligibilitySetupDlg::OnRequeryFinishedEligLocationCombo(short nFlags) 
{
	try {

		IRowSettingsPtr pLocRow = m_LocationCombo->GetFirstRow();		
		m_LocationCombo->PutCurSel(pLocRow);

		IRowSettingsPtr pProvRow = m_ProviderCombo->GetCurSel();

		if(pProvRow != NULL && pLocRow != NULL) {
			m_nProviderID = VarLong(pProvRow->GetValue(pccID),-1);
			m_nLocationID = VarLong(pLocRow->GetValue(lccID),-1);

			Load();
		}

	}NxCatchAll("Error in CEligibilitySetupDlg::OnRequeryFinishedEligLocationCombo");
}

// (j.jones 2010-03-01 17:28) - PLID 37585 - added override for 2100B NM109
void CEligibilitySetupDlg::OnCheckUse2100bNm109()
{
	try {

		BOOL bEnabled = m_checkUse2100B_NM109.GetCheck();
		m_nxedit2100B_NM109_Qual.SetReadOnly(!bEnabled);
		m_nxedit2100B_NM109_Value.SetReadOnly(!bEnabled);

		if(!m_bIsLoading) {
			m_bHasChanged = TRUE;
		}
	
	}NxCatchAll("Error in CEligibilitySetupDlg::OnCheckUse2100bNm109");
}
