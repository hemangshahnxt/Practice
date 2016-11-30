// CareCreditMarketingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CareCreditMarketingDlg.h"
#include "CareCreditUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CCareCreditMarketingDlg dialog


CCareCreditMarketingDlg::CCareCreditMarketingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCareCreditMarketingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCareCreditMarketingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCareCreditMarketingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCareCreditMarketingDlg)
	DDX_Control(pDX, IDC_DONT_SHOW_CARECREDIT_MARKETING, m_btnDontShow);
	DDX_Control(pDX, IDC_OPEN_CARECREDIT, m_btnOpenCareCredit);
	DDX_Control(pDX, IDC_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_UPDATE_LICENSE, m_btnUpdateLicense);
	DDX_Control(pDX, IDC_EMAIL_NEXTECH, m_btnEmailNextech);
	DDX_Control(pDX, IDC_OPEN_CARECREDIT_WEBPAGE, m_btnOpenCareCreditWebsite);
	DDX_Control(pDX, IDC_CARECREDIT_MARKETING_TEXT, m_nxstaticCarecreditMarketingText);
	DDX_Control(pDX, IDC_UPDATE_LICENSE_TEXT, m_nxstaticUpdateLicenseText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCareCreditMarketingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCareCreditMarketingDlg)
	ON_BN_CLICKED(IDC_OPEN_CARECREDIT, OnOpenCarecredit)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_UPDATE_LICENSE, OnUpdateLicense)
	ON_BN_CLICKED(IDC_EMAIL_NEXTECH, OnEmailNexTech)
	ON_BN_CLICKED(IDC_OPEN_CARECREDIT_WEBPAGE, OnOpenCareCreditWebpage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCareCreditMarketingDlg message handlers

BOOL CCareCreditMarketingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_btnCancel.AutoSet(NXB_CLOSE);

		UpdateView();

	}NxCatchAll("CCareCreditMarketingDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCareCreditMarketingDlg::OnCancel() 
{
	try {

		if(IsDlgButtonChecked(IDC_DONT_SHOW_CARECREDIT_MARKETING)) {
			SetRemotePropertyInt("DontShowCareCreditMarketingScreen", 1, 0, GetCurrentUserName());
		}

	}NxCatchAll("CCareCreditMarketingDlg::OnCancel");
	CDialog::OnCancel();	
}

void CCareCreditMarketingDlg::OnOpenCarecredit() 
{
	try {

		if(IsDlgButtonChecked(IDC_DONT_SHOW_CARECREDIT_MARKETING)) {
			SetRemotePropertyInt("DontShowCareCreditMarketingScreen", 1, 0, GetCurrentUserName());
		}

	}NxCatchAll("CCareCreditMarketingDlg::OnOpenCarecredit");
	CDialog::OnOK();	
}

void CCareCreditMarketingDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	RedrawWindow();

	// (z.manning, 01/09/2007) - PLID 24057 - There are 3 possible states for CareCredit licensing:
	//  1. They are licensed for it and may use it freely.
	//  2. They are not licensed for it, but there's no expiration date. This means we are still offering
	//     the CareCredit link, but they need to contact us in order to have their link enabled.
	//  3. They have an expiration date before the current date (whether licensed or not). This means we have
	//     stopped offering the CareCredit link, so just popup a message telling them usage of it has expired.
	CString strText;
	CareCreditLicenseStatus eLicenseStatus = NxCareCredit::GetCareCreditLicenseStatus();
	GetDlgItem(IDC_OPEN_CARECREDIT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_UPDATE_LICENSE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_UPDATE_LICENSE_TEXT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_DONT_SHOW_CARECREDIT_MARKETING)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EMAIL_NEXTECH)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_OPEN_CARECREDIT_WEBPAGE)->ShowWindow(SW_HIDE);
	if(eLicenseStatus == cclsLicensed)
	{
		// (z.manning, 01/04/2007) - PLID 24056 - Make sure the didn't set the don't show me again option.
		if(GetRemotePropertyInt("DontShowCareCreditMarketingScreen", 0, 0, GetCurrentUserName(), true) == 1) {
			EndDialog(IDOK);
		}
		strText = "NexTech offers without charge the use of this integration module with "
			"CareCredit® patient financing. However, NexTech reserves all rights of ownership "
			"and availability of use as detailed in your NexTech Software License Agreement.";
	}
	else
	{
		// (z.manning, 01/10/2007) - PLID 24057 - Ok, they don't have a license for it. Check whether or not it's expired.
		if(eLicenseStatus == cclsExpired) {
			strText = "The free usage period for the CareCredit® link has expired.";
		}
		else {
			strText = "The NexTech integration module to CareCredit® allows you to apply for patient financing "
				"and pre-populate patient information from NexTech to CareCredit®. This link is not currently "
				"enabled for your account. Please contact NexTech at (800) 490-0821 for more details.";
			GetDlgItem(IDC_UPDATE_LICENSE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UPDATE_LICENSE_TEXT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMAIL_NEXTECH)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_OPEN_CARECREDIT_WEBPAGE)->ShowWindow(SW_SHOW);
		}
		GetDlgItem(IDC_OPEN_CARECREDIT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DONT_SHOW_CARECREDIT_MARKETING)->ShowWindow(SW_HIDE);
	}
	SetDlgItemText(IDC_CARECREDIT_MARKETING_TEXT, strText);
}

void CCareCreditMarketingDlg::OnUpdateLicense() 
{
	// (z.manning, 01/10/2007) - PLID 24056 - Intentionally keeping this simple (as in not bringing up
	// the download, ok, cancel dialog) to minimize support calls here.
	CWaitCursor wc;
	try {
		
		if(!g_pLicense->TryToUpdateLicense()) {
			MessageBox("Unable to update your license. Please make sure you are connected to the Internet. "
				"Contact NexTech Technical Support if you need help.");
			return;
		}

		// (z.manning, 01/10/2007) - Ok, they've updated the license successfully, see if we have CareCredit
		// license now.
		if(g_pLicense->CheckForLicense(CLicense::lcCareCredit, CLicense::cflrSilent)) {
			MessageBox("You may now start using the CareCredit link!");
			UpdateView();
		}		
		else {
			MessageBox("License updated successfully, but you still do not have a license for CareCredit. "
				"Contact NexTech for further assistance.");
		}

	}NxCatchAll("CCareCreditMarketingDlg::OnUpdateLicense");
}

void CCareCreditMarketingDlg::OnEmailNexTech()
{
	try {

		// (z.manning, 01/31/2007) - PLID 24424 - Send to the special email account we've setup.
		SendEmail(this, "ccredit@nextech.com", "Care Credit Integration", "");

	}NxCatchAll("CCareCreditMarketingDlg::OnEmailNexTech");
}

void CCareCreditMarketingDlg::OnOpenCareCreditWebpage()
{
	try {

		// (z.manning, 02/08/2007) - PLID 24424 - Open the CareCredit page on our website.
		ShellExecute(NULL, NULL, "http://www.nextech.com/CareCredit.htm", NULL, NULL, SW_SHOW);

	}NxCatchAll("CCareCreditMarketingDlg::OnOpenCareCreditWebpage");
}
