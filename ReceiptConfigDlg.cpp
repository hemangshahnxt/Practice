// ReceiptConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "ReceiptConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReceiptConfigDlg dialog


CReceiptConfigDlg::CReceiptConfigDlg(CWnd* pParent)
	: CNxDialog(CReceiptConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReceiptConfigDlg)
	//}}AFX_DATA_INIT
}


void CReceiptConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReceiptConfigDlg)
	DDX_Control(pDX, IDC_RECEIPT_SHOW_CHARGE_INFO, m_btnHideApplyInfo);
	DDX_Control(pDX, IDC_RECEIPT_SHOW_TAX, m_btnShowTax);
	DDX_Control(pDX, IDC_RECEIPT_CUSTOM_TEXT, m_nxeditReceiptCustomText);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReceiptConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CReceiptConfigDlg)
	ON_BN_CLICKED(IDC_RECEIPT_SHOW_CHARGE_INFO, OnReceiptShowChargeInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReceiptConfigDlg message handlers

void CReceiptConfigDlg::OnReceiptShowChargeInfo() 
{
	// TODO: Add your control notification handler code here
	
}

void CReceiptConfigDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CReceiptConfigDlg::OnOK() 
{
	CString str;
	SetRemotePropertyInt("ReceiptShowChargeInfo", IsDlgButtonChecked(IDC_RECEIPT_SHOW_CHARGE_INFO), 0, "<None>");
	SetRemotePropertyInt("ReceiptShowTax", IsDlgButtonChecked(IDC_RECEIPT_SHOW_TAX), 0, "<None>");
	GetDlgItemText(IDC_RECEIPT_CUSTOM_TEXT, str);
	SetRemotePropertyText("ReceiptCustomInfo", str, 0, "<None>");
	
	CDialog::OnOK();
}

BOOL CReceiptConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	((CNxEdit*)GetDlgItem(IDC_RECEIPT_CUSTOM_TEXT))->LimitText(255);
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	//load all the settings
	SetDlgItemText (IDC_RECEIPT_CUSTOM_TEXT, GetRemotePropertyText ("ReceiptCustomInfo", "", 0, "<None>", true));
	CheckDlgButton(IDC_RECEIPT_SHOW_CHARGE_INFO,GetRemotePropertyInt("ReceiptShowChargeInfo",0,0,"<None>",true));
	CheckDlgButton(IDC_RECEIPT_SHOW_TAX,GetRemotePropertyInt("ReceiptShowTax",0,0,"<None>",true));

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
