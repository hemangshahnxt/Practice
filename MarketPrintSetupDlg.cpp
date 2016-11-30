// MarketPrintSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "marketingrc.h"
#include "MarketPrintSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketPrintSetupDlg dialog


CMarketPrintSetupDlg::CMarketPrintSetupDlg(CWnd* pParent /*=NULL*/, BOOL bPreview /*= TRUE*/)
	: CNxDialog(CMarketPrintSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketPrintSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bPreview = bPreview;
}


void CMarketPrintSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketPrintSetupDlg)
	DDX_Control(pDX, IDC_PREVIEW_BTN, m_btnPreview);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRINT_STYLE_GROUPBOX, m_btnPrintStyleGroupbox);
	DDX_Control(pDX, IDC_SUMMARY_RADIO, m_radioSummary);
	DDX_Control(pDX, IDC_DETAILED_RADIO, m_radioDetailed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketPrintSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketPrintSetupDlg)
	ON_BN_CLICKED(IDC_PREVIEW_BTN, OnPreviewBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketPrintSetupDlg message handlers

void CMarketPrintSetupDlg::OnPreviewBtn() 
{

	if (IsDlgButtonChecked(IDC_DETAILED_RADIO)) {
		SetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>");
	}
	else {
		SetRemotePropertyInt("MarketPrintStyleOption", 0, 0, "<None>");
	}

	if (IsDlgButtonChecked(IDC_LANDSCAPE_RADIO)) {
		SetRemotePropertyInt("MarketPrintLayoutOption", 1, 0, "<None>");
	}
	else {
		SetRemotePropertyInt("MarketPrintLayoutOption", 0, 0, "<None>");
	}



	
	OnOK();

}

BOOL CMarketPrintSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-04-29 12:41) - PLID 29824 - NxIconify the buttons
		m_btnPreview.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>")) {
			CheckDlgButton(IDC_DETAILED_RADIO, 1);
		}
		else {
			CheckDlgButton(IDC_SUMMARY_RADIO, 1);
		}

		/*if (GetRemotePropertyInt("MarketPrintLayoutOption", 1, 0, "<None>")) {
			CheckDlgButton(IDC_LANDSCAPE_RADIO, 1);
		}
		else {
			CheckDlgButton(IDC_PORTRAIT_RADIO, 1);
		}*/

		CString strTemp;
		if (m_bPreview) {
			strTemp = "Preview";
		} else {
			strTemp = "Print";
		}
		SetDlgItemText(IDC_PREVIEW_BTN, "&" + strTemp);
	}
	NxCatchAll("Error in CMarketPrintSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
