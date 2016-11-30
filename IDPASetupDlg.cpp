// IDPASetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IDPASetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CIDPASetupDlg dialog


CIDPASetupDlg::CIDPASetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CIDPASetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIDPASetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIDPASetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIDPASetupDlg)
	DDX_Control(pDX, IDC_RADIO_4_DIGIT_YEAR, m_btn4Digit);
	DDX_Control(pDX, IDC_RADIO_DATES_WITH_NO_SPACES, m_btnDoNotIncludeSpaces);
	DDX_Control(pDX, IDC_RADIO_DATES_WITH_SPACES, m_btnIncludeSpaces);
	DDX_Control(pDX, IDC_RADIO_2_DIGIT_YEAR, m_btn2Digit);
	DDX_Control(pDX, IDC_CHECK_SHOW_CHARGE_COUNT_IN_BOX_34, m_btnShowCount34);
	DDX_Control(pDX, IDC_CHECK_SHOW_LOC_NAME_CITY_IN_BOX_21, m_btmShowPOSBox21);
	DDX_Control(pDX, IDC_CHECK_HIDE_BOX_21_WHEN_BILL_LOCATION, m_btnDoNotFill21POS);
	DDX_Control(pDX, IDC_IDPA_PAYEE_NUMBER, m_nxeditIdpaPayeeNumber);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIDPASetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CIDPASetupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIDPASetupDlg message handlers

BOOL CIDPASetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29860 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	m_ProviderNumberCombo = BindNxDataListCtrl(this,IDC_IDPA_PROVIDER_NUMBER_COMBO,GetRemoteData(),false);
	m_ProviderNameCombo = BindNxDataListCtrl(this,IDC_IDPA_PROVIDER_NAME_COMBO,GetRemoteData(),false);

	((CNxEdit*)GetDlgItem(IDC_IDPA_PAYEE_NUMBER))->LimitText(255);

	IRowSettingsPtr pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)1);
	pRow->PutValue(1,_bstr_t("Medicaid Number"));
	m_ProviderNumberCombo->AddRow(pRow);
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)2);
	pRow->PutValue(1,_bstr_t("UPIN"));
	m_ProviderNumberCombo->AddRow(pRow);
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)3);
	pRow->PutValue(1,_bstr_t("Medicare Number"));
	m_ProviderNumberCombo->AddRow(pRow);	
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)4);
	pRow->PutValue(1,_bstr_t("BCBS Number"));
	m_ProviderNumberCombo->AddRow(pRow);
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)5);
	pRow->PutValue(1,_bstr_t("Social Security Number"));
	m_ProviderNumberCombo->AddRow(pRow);
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)6);
	pRow->PutValue(1,_bstr_t("DEA Number"));
	m_ProviderNumberCombo->AddRow(pRow);
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)7);
	pRow->PutValue(1,_bstr_t("Workers Comp Number"));
	m_ProviderNumberCombo->AddRow(pRow);
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)8);
	pRow->PutValue(1,_bstr_t("Other ID Number"));
	m_ProviderNumberCombo->AddRow(pRow);
	// (j.jones 2007-06-07 12:11) - PLID 26248 - added NPI number
	pRow = m_ProviderNumberCombo->GetRow(-1);
	pRow->PutValue(0,(long)9);
	pRow->PutValue(1,_bstr_t("NPI Number"));
	m_ProviderNumberCombo->AddRow(pRow);

	pRow = m_ProviderNameCombo->GetRow(-1);
	pRow->PutValue(0,(long)1);
	pRow->PutValue(1,_bstr_t("First Middle Last Title"));
	m_ProviderNameCombo->AddRow(pRow);
	pRow = m_ProviderNameCombo->GetRow(-1);
	pRow->PutValue(0,(long)2);
	pRow->PutValue(1,_bstr_t("Last, First Middle Title"));
	m_ProviderNameCombo->AddRow(pRow);
	pRow = m_ProviderNameCombo->GetRow(-1);
	pRow->PutValue(0,(long)3);
	pRow->PutValue(1,_bstr_t("Last First Title"));
	m_ProviderNameCombo->AddRow(pRow);
	pRow = m_ProviderNameCombo->GetRow(-1);
	pRow->PutValue(0,(long)4);
	pRow->PutValue(1,_bstr_t("First Middle Last"));
	m_ProviderNameCombo->AddRow(pRow);
	pRow = m_ProviderNameCombo->GetRow(-1);
	pRow->PutValue(0,(long)5);
	pRow->PutValue(1,_bstr_t("Last, First Middle"));
	m_ProviderNameCombo->AddRow(pRow);
	pRow = m_ProviderNameCombo->GetRow(-1);
	pRow->PutValue(0,(long)6);
	pRow->PutValue(1,_bstr_t("Last First"));
	m_ProviderNameCombo->AddRow(pRow);
		
	int nProviderNumber = GetRemotePropertyInt("IDPAProviderNumber",2,0,"<None>",TRUE);
	int nProviderName = GetRemotePropertyInt("IDPAProviderName",1,0,"<None>",TRUE);

	m_ProviderNumberCombo->SetSelByColumn(0,(long)nProviderNumber);
	m_ProviderNameCombo->SetSelByColumn(0,(long)nProviderName);

	SetDlgItemText(IDC_IDPA_PAYEE_NUMBER,GetRemotePropertyText("IDPAPayeeNumber","",0,"<None>",TRUE));

	CheckDlgButton(IDC_CHECK_SHOW_CHARGE_COUNT_IN_BOX_34, GetRemotePropertyInt("IDPAShowChargeCountInBox34",0,0,"<None>",TRUE) == 1 ? TRUE : FALSE);

	long nYearDigits = GetRemotePropertyInt("IDPADatesYearDigits",1,0,"<None>",TRUE);

	if(nYearDigits == 0)
		CheckDlgButton(IDC_RADIO_2_DIGIT_YEAR,TRUE);
	else
		CheckDlgButton(IDC_RADIO_4_DIGIT_YEAR,TRUE);

	long nIncludeSpaces = GetRemotePropertyInt("IDPADatesIncludeSpaces",1,0,"<None>",TRUE);

	if(nIncludeSpaces == 1)
		CheckDlgButton(IDC_RADIO_DATES_WITH_SPACES,TRUE);
	else
		CheckDlgButton(IDC_RADIO_DATES_WITH_NO_SPACES,TRUE);

	CheckDlgButton(IDC_CHECK_HIDE_BOX_21_WHEN_BILL_LOCATION, GetRemotePropertyInt("IDPAHideBox21WhenPOSEqual",0,0,"<None>",TRUE) == 1 ? TRUE : FALSE);

	CheckDlgButton(IDC_CHECK_SHOW_LOC_NAME_CITY_IN_BOX_21, GetRemotePropertyInt("IDPAOnlyShowPOSNameAndCity",0,0,"<None>",TRUE) == 1 ? TRUE : FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIDPASetupDlg::OnOK() 
{
	if(m_ProviderNumberCombo->CurSel == -1) {
		AfxMessageBox("Please select a provider number.");
		return;
	}

	if(m_ProviderNameCombo->CurSel == -1) {
		AfxMessageBox("Please select a provider name.");
		return;
	}

	int nProviderNumber = m_ProviderNumberCombo->GetValue(m_ProviderNumberCombo->CurSel,0).lVal;
	SetRemotePropertyInt("IDPAProviderNumber",nProviderNumber,0,"<None>");
	int nProviderName = m_ProviderNameCombo->GetValue(m_ProviderNameCombo->CurSel,0).lVal;
	SetRemotePropertyInt("IDPAProviderName",nProviderName,0,"<None>");

	CString strPayee;
	GetDlgItemText(IDC_IDPA_PAYEE_NUMBER,strPayee);
	SetRemotePropertyText("IDPAPayeeNumber",strPayee,0,"<None>");

	SetRemotePropertyInt("IDPAShowChargeCountInBox34", IsDlgButtonChecked(IDC_CHECK_SHOW_CHARGE_COUNT_IN_BOX_34) ? 1 : 0, 0, "<None>");

	long nYearDigits = 1;

	if(IsDlgButtonChecked(IDC_RADIO_2_DIGIT_YEAR))
		nYearDigits = 0;

	SetRemotePropertyInt("IDPADatesYearDigits",nYearDigits,0,"<None>");

	long nIncludeSpaces = 1;

	if(IsDlgButtonChecked(IDC_RADIO_DATES_WITH_NO_SPACES))
		nIncludeSpaces = 0;

	SetRemotePropertyInt("IDPADatesIncludeSpaces",nIncludeSpaces,0,"<None>");

	SetRemotePropertyInt("IDPAHideBox21WhenPOSEqual", IsDlgButtonChecked(IDC_CHECK_HIDE_BOX_21_WHEN_BILL_LOCATION) ? 1 : 0, 0, "<None>");

	SetRemotePropertyInt("IDPAOnlyShowPOSNameAndCity", IsDlgButtonChecked(IDC_CHECK_SHOW_LOC_NAME_CITY_IN_BOX_21) ? 1 : 0, 0, "<None>");
	
	CDialog::OnOK();
}
