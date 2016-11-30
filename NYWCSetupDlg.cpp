// NYWCSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NYWCSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNYWCSetupDlg dialog

CNYWCSetupDlg::CNYWCSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNYWCSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNYWCSetupDlg)
	//}}AFX_DATA_INIT
}


void CNYWCSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNYWCSetupDlg)
	DDX_Control(pDX, IDC_RADIO_USE_FIRST_SIMILAR_ILLNESS, m_btnUseIllnessDate);
	DDX_Control(pDX, IDC_RADIO_USE_DATE_OF_CURRENT_ACC, m_btnUseAccidentDate);
	DDX_Control(pDX, IDC_RADIO_USEBILLPROV, m_radioBillProvider);
	DDX_Control(pDX, IDC_RADIO_USEOVRRDPROV, m_radioOverride);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_ADDRESS, m_nxeditAddress);
	DDX_Control(pDX, IDC_CITY, m_nxeditCity);
	DDX_Control(pDX, IDC_STATE, m_nxeditState);
	DDX_Control(pDX, IDC_ZIP, m_nxeditZip);
	DDX_Control(pDX, IDC_PHONE, m_nxeditPhone);
	DDX_Control(pDX, IDC_BOX33_NAME_LABEL, m_nxstaticBox33NameLabel);
	DDX_Control(pDX, IDC_O2STATIC, m_nxstaticO2static);
	DDX_Control(pDX, IDC_O3STATIC, m_nxstaticO3static);
	DDX_Control(pDX, IDC_O5STATIC, m_nxstaticO5static);
	DDX_Control(pDX, IDC_O6STATIC, m_nxstaticO6static);
	DDX_Control(pDX, IDC_O4STATIC, m_nxstaticO4static);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNYWCSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNYWCSetupDlg)
	ON_BN_CLICKED(IDC_RADIO_USEBILLPROV, OnRadioUsebillprov)
	ON_BN_CLICKED(IDC_RADIO_USEOVRRDPROV, OnRadioUseovrrdprov)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNYWCSetupDlg message handlers

BOOL CNYWCSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	long nDateOfInjury = GetRemotePropertyInt("NYWCDateOfInjury",1,0,"<None>",true);
	if(nDateOfInjury == 0)
		CheckDlgButton(IDC_RADIO_USE_DATE_OF_CURRENT_ACC, TRUE);
	else
		CheckDlgButton(IDC_RADIO_USE_FIRST_SIMILAR_ILLNESS, TRUE);
	
	long nUseProvider = GetRemotePropertyInt("NYWCUseProvider",0,0,"<None>",true);

	if(nUseProvider == 0) {
		m_radioBillProvider.SetCheck(1);
		EnableOverride(FALSE);
	}
	else {
		m_radioOverride.SetCheck(1);
		EnableOverride(TRUE);
	}

	((CNxEdit*)GetDlgItem(IDC_NAME))->LimitText(255);
	((CNxEdit*)GetDlgItem(IDC_ADDRESS))->LimitText(255);
	((CNxEdit*)GetDlgItem(IDC_CITY))->LimitText(255);
	((CNxEdit*)GetDlgItem(IDC_STATE))->LimitText(255);
	((CNxEdit*)GetDlgItem(IDC_ZIP))->LimitText(255);
	((CNxEdit*)GetDlgItem(IDC_PHONE))->LimitText(255);

	SetDlgItemText(IDC_NAME,GetRemotePropertyText("NYWCOverrideName","",0,"<None>"));
	SetDlgItemText(IDC_ADDRESS,GetRemotePropertyText("NYWCOverrideAddress","",0,"<None>"));
	SetDlgItemText(IDC_CITY,GetRemotePropertyText("NYWCOverrideCity","",0,"<None>"));
	SetDlgItemText(IDC_STATE,GetRemotePropertyText("NYWCOverrideState","",0,"<None>"));
	SetDlgItemText(IDC_ZIP,GetRemotePropertyText("NYWCOverrideZip","",0,"<None>"));
	SetDlgItemText(IDC_PHONE,GetRemotePropertyText("NYWCOverridePhone","",0,"<None>"));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNYWCSetupDlg::OnOK() 
{
	long nDateOfInjury = 1;
	if(IsDlgButtonChecked(IDC_RADIO_USE_DATE_OF_CURRENT_ACC))
		nDateOfInjury = 0;

	SetRemotePropertyInt("NYWCDateOfInjury",nDateOfInjury,0,"<None>");

	long UseProvider = 0;
	if(m_radioOverride.GetCheck())
		UseProvider = 1;

	SetRemotePropertyInt("NYWCUseProvider",UseProvider,0,"<None>");

	CString str;
	GetDlgItemText(IDC_NAME,str);
	SetRemotePropertyText("NYWCOverrideName",str,0,"<None>");
	GetDlgItemText(IDC_ADDRESS,str);
	SetRemotePropertyText("NYWCOverrideAddress",str,0,"<None>");
	GetDlgItemText(IDC_CITY,str);
	SetRemotePropertyText("NYWCOverrideCity",str,0,"<None>");
	GetDlgItemText(IDC_STATE,str);
	SetRemotePropertyText("NYWCOverrideState",str,0,"<None>");
	GetDlgItemText(IDC_ZIP,str);
	SetRemotePropertyText("NYWCOverrideZip",str,0,"<None>");
	GetDlgItemText(IDC_PHONE,str);
	SetRemotePropertyText("NYWCOverridePhone",str,0,"<None>");
	
	CDialog::OnOK();
}

void CNYWCSetupDlg::OnRadioUsebillprov() 
{
	if(m_radioBillProvider.GetCheck())
		EnableOverride(FALSE);
	else
		EnableOverride(TRUE);
}

void CNYWCSetupDlg::OnRadioUseovrrdprov() 
{
	if(m_radioOverride.GetCheck())
		EnableOverride(TRUE);
	else
		EnableOverride(FALSE);	
}

void CNYWCSetupDlg::EnableOverride(BOOL bEnable)
{
	GetDlgItem(IDC_NAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADDRESS)->EnableWindow(bEnable);
	GetDlgItem(IDC_CITY)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_ZIP)->EnableWindow(bEnable);
	GetDlgItem(IDC_PHONE)->EnableWindow(bEnable);
}
