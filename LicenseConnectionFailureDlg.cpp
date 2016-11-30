// LicenseConnectionFailureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LicenseConnectionFailureDlg.h"
#include "InternationalUtils.h"
#include "PracticeRc.h"
#include "GlobalUtils.h"

// (j.jones 2016-03-03 09:01) - PLID 68479 - created

// CLicenseConnectionFailureDlg dialog

#define DEFAULT_DAYS_UNTIL_LOCKOUT	3

CLicenseConnectionFailureDlg::CLicenseConnectionFailureDlg(CWnd* pParent, CString strSubkey, CPracticeLicense* pLicense)
	: CNxDialog(CLicenseConnectionFailureDlg::IDD, pParent)
{
	m_strSubkey = strSubkey;
	m_pLicense = pLicense;
	m_nDaysRemaining = DEFAULT_DAYS_UNTIL_LOCKOUT;
	m_bPreventLogin = false;
}

CLicenseConnectionFailureDlg::~CLicenseConnectionFailureDlg()
{
}

void CLicenseConnectionFailureDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LICENSE_WARNING_LABEL1, m_nxstaticLicenseWarningLabel1);
	DDX_Control(pDX, IDC_LICENSE_WARNING_LABEL2, m_nxstaticLicenseWarningLabel2);
	DDX_Control(pDX, IDC_PHONE_LABEL, m_nxstaticPhoneLabel);
	DDX_Control(pDX, IDC_WEBSITE_LABEL, m_nxlstaticWebSite);
	DDX_Control(pDX, IDC_EMAIL_LABEL, m_nxlstaticEmail);
	DDX_Control(pDX, IDC_RETRY_LABEL, m_nxstaticRetryLabel);
	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_RETRY, m_btnRetry);
}


BEGIN_MESSAGE_MAP(CLicenseConnectionFailureDlg, CNxDialog)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_RETRY, OnBtnRetry)
END_MESSAGE_MAP()


// CLicenseConnectionFailureDlg message handlers

BOOL CLicenseConnectionFailureDlg::OnInitDialog()
{	
	try {

		CNxDialog::OnInitDialog();

		//this should never be greater than DEFAULT_DAYS_UNTIL_LOCKOUT (3)
		ASSERT(m_nDaysRemaining <= DEFAULT_DAYS_UNTIL_LOCKOUT);

		CString strWarningText;

		// (j.jones 2016-03-03 13:32) - PLID 68478 - if the lockout period has passed, change the warning
		// to tell the user they will not be allowed to login until the connectivity issue is resolved		
		if (m_nDaysRemaining <= 0) {
			
			if (!m_bPreventLogin) {
				//anything that set the days remaining to <= 0
				//should have already set m_bPreventLogin to true,
				//why didn't that happen?
				ASSERT(FALSE);				
				m_bPreventLogin = true;
			}
			
			//report how many days this has been going on, and tell them they can't log in
			strWarningText.Format("Your license server has been unable to communicate with Nextech for %li days.\n\n"
				"Access to this system has been be denied until this connectivity problem has been resolved.", -m_nDaysRemaining + DEFAULT_DAYS_UNTIL_LOCKOUT);
		}
		else {
			//this text dynamically counts down how many days remain before being locked out		
			strWarningText.Format("Your license server has been unable to communicate with Nextech.\n\n"
				"Access to this system will be denied in %li days unless this connectivity problem has been resolved.", m_nDaysRemaining);
		}
		m_nxstaticLicenseWarningLabel1.SetWindowText(strWarningText);

		m_nxlstaticWebSite.SetType(dtsHyperlink);
		m_nxlstaticWebSite.SetText("http://www.nextech.com");
		m_nxlstaticEmail.SetType(dtsHyperlink);
		m_nxlstaticEmail.SetText("Email Nextech");

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnRetry.AutoSet(NXB_REFRESH);
	
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

//checks to see if we have connectivity, shows the dialog if we do not
void CLicenseConnectionFailureDlg::CheckShowDialog()
{
	COleDateTime dtNowTime = COleDateTime::GetCurrentTime();
	COleDateTime dtNow;
	dtNow.SetDate(dtNowTime.GetYear(), dtNowTime.GetMonth(), dtNowTime.GetDay());

	//Get the last time that the license was attempted to download,
	//and the current lockout date (if any)
	COleDateTime dtLastAttemptDay;
	{
		// (j.jones 2016-03-08 13:45) - PLID 68479 - this is now encrypted to prevent tampering,
		// this function will give us the real date
		COleDateTime dtLastAttempt = GetLastLicenseUpdateAttempt();		
		dtLastAttemptDay = COleDateTime(dtLastAttempt.GetYear(), dtLastAttempt.GetMonth(), dtLastAttempt.GetDay(), 0, 0, 0);
	}

	COleDateTime dtLockoutDate = m_pLicense->GetSubscriptionLicenseConnectivityLockoutDate();

	//if no license update was attempted on today's calendar day, try to update now
	if (dtNow >= dtLastAttemptDay + COleDateTimeSpan(1, 0, 0, 0)) {

		CWaitCursor pWait;

		if (!m_pLicense->TryToUpdateLicense()) {

			//connection failed - begin the lockout period if we do not have one
			if (dtLockoutDate.GetStatus() != COleDateTime::valid) {

				//set the default days remaining (3)
				m_nDaysRemaining = DEFAULT_DAYS_UNTIL_LOCKOUT;
				dtLockoutDate = dtNow + COleDateTimeSpan(m_nDaysRemaining, 0, 0, 0);

				m_pLicense->UpdateLicenseConnectivityLockoutDate(dtLockoutDate);
			}
		}
		else {
			//update the local lockout date
			dtLockoutDate = m_pLicense->GetSubscriptionLicenseConnectivityLockoutDate();

			//this ought to have updated to null
			ASSERT(dtLockoutDate == g_cdtNull);
		}
	}

	//if the LicenseConnectivityLockoutDate is a valid date then that
	//means the license update has failed due to connectivity issues,
	//a successful license check would have wiped this date
	if(dtLockoutDate.GetStatus() == COleDateTime::valid) {
		
		//calculate the days remaining until lockout,
		//if <= 0, the lockout will be enforced
		COleDateTimeSpan dtSpan = dtLockoutDate - dtNow;
		m_nDaysRemaining = dtSpan.GetDays();

		// (j.jones 2016-03-03 13:32) - PLID 68478 - if the lockout day is today or in the past,
		// prevent the user from logging in
		if (m_nDaysRemaining <= 0) {
			m_bPreventLogin = true;
		}

		//pop up the dialog
		DoModal();
	}
}

void CLicenseConnectionFailureDlg::OnBtnClose()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CLicenseConnectionFailureDlg::OnBtnRetry()
{
	try {

		CWaitCursor pWait;

		//attempt to reconnect to the license activation server
		bool bSuccess = m_pLicense->TryToUpdateLicense() ? true : false;

		//if this succeeded, tell the user so, and return IDOK
		if (bSuccess) {

			// (j.jones 2016-03-03 13:32) - PLID 68478 - clear the prevent login boolean
			m_bPreventLogin = false;

			CNxDialog::OnOK();

			//use the green checkmark icon in the messagebox
			MSGBOXPARAMS mp;
			::ZeroMemory(&mp, sizeof(MSGBOXPARAMS));
			mp.cbSize = sizeof(MSGBOXPARAMS);
			mp.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
			mp.hInstance = AfxGetApp()->m_hInstance;
			mp.lpszText = "The license server connection to Nextech has been successfully restored.";
			mp.lpszCaption = "Practice";
			mp.dwStyle = MB_USERICON | MB_OK;
			mp.lpszIcon = MAKEINTRESOURCE(IDI_CHECKMARK);
			::MessageBoxIndirect((LPMSGBOXPARAMS)&mp);
			return;
		}
		else {
			//give a technical reason why it failed, such that a competent IT person could resolve it
			CString strFailure;
			CString strServerAddress = NxRegUtils::ReadString(NxRegUtils::GetRegistryBase(m_strSubkey) + "LicenseActivationServer", LICENSE_ACTIVATION_SERVER_ADDRESS);
			strFailure.Format("The license server connection to Nextech failed. Please check your server's internet connection and firewall settings.\n\n"
				"Please ensure that your network can communicate with %s on port %li.\n\n"
				"For further assistance with this issue, please contact Nextech.",
				strServerAddress, STANDARD_NXLICENSEACTIVATIONSERVER_PORT);
			MessageBox(strFailure, "Practice", MB_OK | MB_ICONERROR);
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CLicenseConnectionFailureDlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	try
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcEmail;
		CRect rcWebSite;
		m_nxlstaticEmail.GetWindowRect(rcEmail);
		m_nxlstaticWebSite.GetWindowRect(rcWebSite);
		ScreenToClient(&rcEmail);
		ScreenToClient(&rcWebSite);

		if (rcEmail.PtInRect(pt) && m_nxlstaticEmail.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
		if (rcWebSite.PtInRect(pt) && m_nxlstaticWebSite.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CLicenseConnectionFailureDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcEmail;
		CRect rcWebSite;
		m_nxlstaticEmail.GetWindowRect(rcEmail);
		m_nxlstaticWebSite.GetWindowRect(rcWebSite);
		ScreenToClient(&rcEmail);
		ScreenToClient(&rcWebSite);

		if (rcEmail.PtInRect(pt) && m_nxlstaticEmail.GetType() == dtsHyperlink) {	//Launch email
			ShellExecute(GetSafeHwnd(), NULL, "mailto:allsupport@nextech.com?Subject=License Server unable to communicate with Nextech", NULL, "", SW_MAXIMIZE);
		}
		if (rcWebSite.PtInRect(pt) && m_nxlstaticWebSite.GetType() == dtsHyperlink) {	//Launch browser.
			ShellExecute(NULL, NULL, "http://www.nextech.com/", NULL, NULL, SW_SHOW);
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}