// MarketReferralsPrintSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "marketingRc.h"
#include "MarketReferralsPrintSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketReferralsPrintSetupDlg dialog


CMarketReferralsPrintSetupDlg::CMarketReferralsPrintSetupDlg(CWnd* pParent /*=NULL*/, BOOL bPreview /*= TRUE*/, BOOL bEnableAll /*=TRUE*/)
	: CNxDialog(CMarketReferralsPrintSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketReferralsPrintSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bPreview = bPreview;
	m_bEnableAll = bEnableAll;
}


void CMarketReferralsPrintSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketReferralsPrintSetupDlg)
	DDX_Control(pDX, IDC_PREVIEW_BTN, m_btnPreview);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRINT_STYLE_GROUPBOX, m_btnPrintStyleGroupbox);
	DDX_Control(pDX, IDC_REPORT_ON, m_btnReportOn);
	DDX_Control(pDX, IDC_SUMMARY_RADIO, m_radioSummary);
	DDX_Control(pDX, IDC_DETAILED_RADIO, m_radioDetailed);
	DDX_Control(pDX, IDC_REFERRAL_SOURCE_RADIO, m_radioReferralSource);
	DDX_Control(pDX, IDC_REF_PHYS_RADIO, m_radioRefPhys);
	DDX_Control(pDX, IDC_REF_PATIENT_RADIO, m_radioRefPatient);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketReferralsPrintSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketReferralsPrintSetupDlg)
	ON_BN_CLICKED(IDC_PREVIEW_BTN, OnPreviewBtn)
	ON_BN_CLICKED(IDC_DETAILED_RADIO, OnDetailedRadio)
	ON_BN_CLICKED(IDC_SUMMARY_RADIO, OnSummaryRadio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketReferralsPrintSetupDlg message handlers

void CMarketReferralsPrintSetupDlg::OnPreviewBtn() 
{
	
	if (IsDlgButtonChecked(IDC_DETAILED_RADIO)) {
		SetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>");
	}
	else {
		SetRemotePropertyInt("MarketPrintStyleOption", 0, 0, "<None>");
	}


	if (IsDlgButtonChecked(IDC_REFERRAL_SOURCE_RADIO)) {
		SetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>");
	}
	else if (IsDlgButtonChecked(IDC_REF_PHYS_RADIO)) {
		SetRemotePropertyInt("MarketReferralPrintOption", 2, 0, "<None>");
	}
	else {
		SetRemotePropertyInt("MarketReferralPrintOption", 3, 0, "<None>");
	}

	CNxDialog::OnOK();

	
}

BOOL CMarketReferralsPrintSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 12:49) - PLID 29824 - NxIconify the buttons
		m_btnPreview.AutoSet(NXB_PRINT_PREV);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if (! m_bEnableAll) {
			CheckDlgButton(IDC_REFERRAL_SOURCE_RADIO, 1);
		}
		else  {
			if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
				CheckDlgButton(IDC_REFERRAL_SOURCE_RADIO, 1);
			}
			else if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 2) {
				CheckDlgButton(IDC_REF_PHYS_RADIO, 1);
			}
			else {
				CheckDlgButton(IDC_REF_PATIENT_RADIO, 1);
			}
		}



		if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>")) {
			CheckDlgButton(IDC_DETAILED_RADIO, 1);
			//only enable referral sources defaultly, the other ones are passed in
			GetDlgItem(IDC_REFERRAL_SOURCE_RADIO)->EnableWindow(TRUE);
			GetDlgItem(IDC_REF_PHYS_RADIO)->EnableWindow(m_bEnableAll);
			GetDlgItem(IDC_REF_PATIENT_RADIO)->EnableWindow(m_bEnableAll);
		}
		else {
			CheckDlgButton(IDC_SUMMARY_RADIO, 1);

			//disable the check boxes
			GetDlgItem(IDC_REFERRAL_SOURCE_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_REF_PHYS_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_REF_PATIENT_RADIO)->EnableWindow(FALSE);
		}
		

		CString strTemp;
		if (m_bPreview) {
			strTemp = "Preview";
		} else {
			strTemp = "Print";
		}
		SetDlgItemText(IDC_PREVIEW_BTN, "&" + strTemp);

		
	}
	NxCatchAll("Error in CMarketReferralsPrintSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarketReferralsPrintSetupDlg::OnDetailedRadio() 
{
	if (IsDlgButtonChecked(IDC_DETAILED_RADIO)) {
		GetDlgItem(IDC_REFERRAL_SOURCE_RADIO)->EnableWindow(TRUE);
		GetDlgItem(IDC_REF_PHYS_RADIO)->EnableWindow(m_bEnableAll);
		GetDlgItem(IDC_REF_PATIENT_RADIO)->EnableWindow(m_bEnableAll);
	}

	
}

void CMarketReferralsPrintSetupDlg::OnSummaryRadio() 
{
	if (IsDlgButtonChecked(IDC_SUMMARY_RADIO)) {
		GetDlgItem(IDC_REFERRAL_SOURCE_RADIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_REF_PHYS_RADIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_REF_PATIENT_RADIO)->EnableWindow(FALSE);
	}

	
	
}
