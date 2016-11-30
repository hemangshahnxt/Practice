// SupportExpiredDlg.cpp: implementation of the CSupportExpiredDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SupportExpiredDlg.h"
#include "InternationalUtils.h"
#include "GlobalUtils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSupportExpiredDlg::CSupportExpiredDlg(CWnd* pParent /*=NULL*/)
	: CSizeableTextDlg(CSupportExpiredDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSupportExpiredDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}

CSupportExpiredDlg::~CSupportExpiredDlg()
{

}

void CSupportExpiredDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSupportExpiredDlg)
	DDX_Control(pDX, IDC_DONTSHOW_CHECK, m_btnDontRemind);
	DDX_Control(pDX, IDC_TEXT_EDIT, m_nxstaticTextEdit);
	DDX_Control(pDX, IDC_TEXT_EDIT2, m_nxstaticTextEdit2);// (a.vengrofski 2009-11-04 17:49) - PLID <36091> - Second text box 
	DDX_Control(pDX, IDC_TEXT_EDIT3, m_nxstaticTextEdit3);// (a.vengrofski 2009-11-04 17:49) - PLID <36091> - Third text box 
	DDX_Control(pDX, IDC_STATIC_EMAIL, m_nxlstaticEmail);// (a.vengrofski 2009-11-04 17:49) - PLID <36091> - EMail link 
	DDX_Control(pDX, IDC_STATIC_WEBSITE, m_nxlstaticWebSite);// (a.vengrofski 2009-11-04 17:49) - PLID <36091> - Website link 
	DDX_Control(pDX, IDYES, m_btnOk);
	DDX_Control(pDX, IDNO, m_btnUpdate);
	//}}AFX_DATA_MAP
}

//TES 1/11/2007 - PLID 24189 - Note that, since Bob implemented 18294 on 11/11/2005, this is function is ALWAYS called
// on program initialization, assuming they have a valid license.  The comments in CLicense::Initialize() already point
// that out, but I just wanted to clarify it here, because from the name of this class you might think this function
// would only be called when the support had expired.
void CSupportExpiredDlg::Show(IN const COleDateTime &dtExpirationDate)
{
	// (j.armen 2011-10-24 14:02) - PLID 46139 - GetPracPath references the MDE path
	// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
	// than the license object.
	m_strCurUser = g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT);
	m_strName = "TechSupport Expired";
	m_strTitle = "";

	// Remember the expiration date because we're going to use it below, AND in the 
	// UserNeedsRemind() function.
	m_dtExpirationDate.SetDate(dtExpirationDate.GetYear(), dtExpirationDate.GetMonth(), dtExpirationDate.GetDay());

	// (b.cardillo 2005-11-11 11:52) - PLID 18290 - Use local time rather than GMT because that's 
	// the convention we've adopted; even though it's not as ideal as GMT, it's the way we do 
	// the comparisons throughout CSizeableTextDlg classes, so if we did GMT time here, we run 
	// the risk of having the comparisons be off by several hours (which was part of the bug of 
	// this pl item).
	m_dtOverrideCurrentTime = COleDateTime::GetCurrentTime();
	
	m_nStyle |= CSizeableTextDlg::drsDontRemindUntil|CSizeableTextDlg::bsYesNoOnly|CSizeableTextDlg::msMsgBoxText|CSizeableTextDlg::tsFromString;
	
	// (b.cardillo 2005-11-11 16:10) - PLID 18294 - We now start the reminding a month before 
	// their tech support is due to expire, so as to give them time.  But it's a friendlier 
	// message that says "Your support will expire in XXX days", until it actually does expire 
	// at which point it says it has expired.
	COleDateTime dtNow;
	dtNow.SetDate(m_dtOverrideCurrentTime.GetYear(), m_dtOverrideCurrentTime.GetMonth(), m_dtOverrideCurrentTime.GetDay());

	// (a.walling 2007-02-20 17:10) - PLID 24462 - Get the last time that the license was attempted to download.
	COleDateTime dtLastAttemptDay;
	{
		// (j.jones 2016-03-08 13:45) - PLID 68479 - this is now encrypted to prevent tampering,
		// this function will give us the real date
		COleDateTime dtLastAttempt = GetLastLicenseUpdateAttempt();
		dtLastAttemptDay = COleDateTime(dtLastAttempt.GetYear(), dtLastAttempt.GetMonth(), dtLastAttempt.GetDay(), 0, 0, 0);
	}

	if (dtNow >= (m_dtExpirationDate - COleDateTimeSpan(30,0,0,0))) {
		//Try and update the license.
		// (a.walling 2007-02-20 17:13) - PLID 24462 - At least limit this to once a day, rather than every login, since
		// users with expired support will create an audit entry on internal each time a user logs on.
		if (dtNow >= dtLastAttemptDay + COleDateTimeSpan(1, 0, 0, 0)) {
			m_pLicense->TryToUpdateLicense();
			m_dtExpirationDate = m_pLicense->GetExpirationDate();
			if(dtNow < (m_dtExpirationDate - COleDateTimeSpan(30,0,0,0))) {
				//We successfully downloaded an updated license.
				MsgBox("Your technical support has been renewed through %s.  Thank you!", FormatDateTimeForInterface(m_dtExpirationDate));
				return;
			}
		}

		if (dtNow > m_dtExpirationDate) {
			// Expired, so give the stern message
			 m_strTextEdit = "Your tech support has expired.";
		} else {
			// Not quite expired yet, so give the friendly message
			long nDays = (m_dtExpirationDate - dtNow).GetDays();
			if (nDays == 0) {
				m_strTextEdit = "Your tech support expires today.";
			} else if (nDays == 1) {
				m_strTextEdit = "Your tech support expires tomorrow.";
			} else {
				m_strTextEdit.Format("Your tech support will expire in %li days.", nDays);
			}
		}
		m_strTextEdit += "\n\nNexTech is constantly updating and adding new features to Practice which help your office "
						"run at peak efficiency.\n\nCall, click, or email to keep up to date and to see the exciting features that you may be missing.";
		m_strTextEdit2 = "(800)490-0821";
		m_strTextEdit3 = "If you feel you are getting this message in error, please click Update to update your license information.";

		// And pop up the dialog
		DoModal();
	} else {
		// No need for any warning, because their support is more than a month from expiration
		//TES 1/11/2007 - PLID 24189 - However, if it's been more than 10 days since we last updated the license, let's go 
		// ahead and update it, just in case anything got missed.
		//We'll default this to the distant past, so the first time they run Practice after upgrading to this version, it
		// will update their license.

		// (b.spivey, November 30, 2012) - PLID 53901 - Once every 3 days instead of 10 days. 
		if(dtNow - dtLastAttemptDay > COleDateTimeSpan(3,0,0,0)) {
			//It's been more than 3 days, let's give it a try.
			m_pLicense->TryToUpdateLicense();
		}
		
	}
}

#define COMPARE_DATETIME_INEXACT(a, b, marginseconds) \
	(((a) > ((b) + COleDateTimeSpan(0, 0, 0, marginseconds))) ? 1 : \
	 ((a) < ((b) - COleDateTimeSpan(0, 0, 0, marginseconds))) ? -1 : 0)

BOOL CSupportExpiredDlg::UserNeedsRemind()
{
	try {
		// (b.cardillo 2005-11-11 12:58) - PLID 18290 - We used to check the last modified date 
		// of the NxCntrl.dll in the shared path here, but since it doesn't exist anymore with 
		// the new licensing, we decided to drop that behavior, and do a more normal "remind me 
		// later" approach.

		// Determine when we are currently required to remind the user
		// (b.cardillo 2005-11-11 11:49) - PLID 18292 - It has been decided that this would be 
		// 3 days, rather than 7 which it used to be.  Now if we decide to change it (yet again) 
		// we can do so very easily by just changing this initialization.
		// (a.vengrofski 2009-11-12 13:01) - PLID <36091> - The decision has come to be 36 hours, or 1 day 12 hours.
		const COleDateTimeSpan dtsRemindAgainAfter(1, 12, 0, 0);
		COleDateTime dtRemindAgain;
		{
			// When was the last time we showed this?
			COleDateTime dtLast = GetRemotePropertyDateTime("timestamp " + m_strName, 
				&COleDateTime(1901, 1, 1, 1, 1, 1), 0, m_strCurUser, false);
			// Use that to determine the next time we have to show it
			dtRemindAgain = dtLast + dtsRemindAgainAfter;

			// (b.cardillo 2005-11-11 18:03) - PLID 18294 - BUT if the last time the user was 
			// reminded was BEFORE their tech support expired, and now we're AFTER the tech 
			// support has expired, then we have to remind them now, even if the last time they 
			// were reminded was less than dtsRemindAgainAfter ago.
			COleDateTime dtExpEnd = m_dtExpirationDate + COleDateTimeSpan(1,0,0,0);
			if (dtLast < dtExpEnd && m_dtOverrideCurrentTime >= dtExpEnd) {
				// Gotta remind them, so short-circuit the rest of the function
				return TRUE;
			}
		}
		
		// Are we past that date
		if(COMPARE_DATETIME_INEXACT(m_dtOverrideCurrentTime, dtRemindAgain, 5) > 0) {
			// We're past it, so we need to remind the user
			return TRUE;
		} else {
			// We're not past it, so no need to remind the user
			return FALSE;
		}

	}NxCatchAll("Error in CSupportExpiredDlg::UserNeedsRemind()");
	return TRUE;
}

BOOL CSupportExpiredDlg::OnInitDialog()
{
	CSizeableTextDlg::OnInitDialog();
	try{
		CWnd *pWnd = NULL; // General purpose variable
		// (a.vengrofski 2009-11-05 13:55) - PLID <36091> - Added a new dialog for Support Expired, so no need to "Hijack" anything.
		// (b.cardillo 2005-11-11 15:16) - PLID 18292 - Part of this pl item is to change the text 
		// here to say "Remind me later".  It used to say "Don't remind me".
		//SetDlgItemText(IDC_DONTSHOW_CHECK, "&Remind me later");

		//TES 3/113/2006 - Hijacking the Yes and No buttons to be OK and Update...
		//SetDlgItemText(IDYES, "Close");
		//SetDlgItemText(IDNO, "Update...");

		pWnd = GetDlgItem(IDC_TEXT_EDIT);		// (a.vengrofski 2009-11-05 13:56) - PLID <36091> - Get the first text box
		if (pWnd){
			pWnd->SetWindowText(m_strTextEdit);	// (a.vengrofski 2009-11-05 13:56) - PLID <36091> - Set it
		}
		pWnd = GetDlgItem(IDC_TEXT_EDIT2);		// (a.vengrofski 2009-11-05 13:56) - PLID <36091> - Get the second text box
		if (pWnd){
			pWnd->SetWindowText(m_strTextEdit2);// (a.vengrofski 2009-11-05 13:56) - PLID <36091> - Set it
		}
		pWnd = GetDlgItem(IDC_TEXT_EDIT3);		// (a.vengrofski 2009-11-05 13:57) - PLID <36091> - Get the third text box
		if (pWnd){
			pWnd->SetWindowText(m_strTextEdit3);// (a.vengrofski 2009-11-05 13:57) - PLID <36091> - Set it
		}
		m_nxlstaticWebSite.SetType(dtsHyperlink);	// (a.vengrofski 2009-11-05 14:24) - PLID <36091> - Set it as a hyperlink
		m_nxlstaticWebSite.SetText("See The Changes");// (a.vengrofski 2009-11-05 14:24) - PLID <36091> - Set the text 
		m_nxlstaticEmail.SetType(dtsHyperlink);	// (a.vengrofski 2009-11-05 14:24) - PLID <36091> - Set it as a hyperlink
		m_nxlstaticEmail.SetText("Email Nextech");// (a.vengrofski 2009-11-05 14:24) - PLID <36091> - Set the text //NOTE see CSupportExpiredDlg::OnLabelClick for destinations

		m_btnOk.AutoSet(NXB_CLOSE);
		m_btnUpdate.AutoSet(NXB_MODIFY);
	}NxCatchAll("Error in CSupportExpiredDlg::OnInitDialog()");
	return TRUE;
}

BEGIN_MESSAGE_MAP(CSupportExpiredDlg,CSizeableTextDlg)
	//{{AFX_MSG_MAP(CSizeableTextDlg)
	ON_BN_CLICKED(IDNO, OnNo)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)// (a.vengrofski 2009-11-05 14:55) - PLID <36091> - Get the left click.
	ON_WM_SETCURSOR()// (a.vengrofski 2009-11-09 10:33) - PLID <36091> - Get the hover.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSupportExpiredDlg::OnNo()
{
	m_pLicense->PromptForUpdateCode(FALSE);

	//Check whether they're still expired.
	COleDateTime dtNewExpire = m_pLicense->GetExpirationDate();
	if(dtNewExpire != m_dtExpirationDate) {
		m_dtExpirationDate = dtNewExpire;
		COleDateTime dtNow;
		dtNow.SetDate(m_dtOverrideCurrentTime.GetYear(), m_dtOverrideCurrentTime.GetMonth(), m_dtOverrideCurrentTime.GetDay());
		if (dtNow < (m_dtExpirationDate - COleDateTimeSpan(30,0,0,0))) {
			MsgBox("Your technical support has been renewed through %s.  Thank you!", FormatDateTimeForInterface(m_dtExpirationDate));
			OnOK();
		}
		else {
			OnOK();
			CSupportExpiredDlg dlg(this);
			dlg.m_pLicense = m_pLicense;
			dlg.Show(m_dtExpirationDate);
		}
	}
}

// (a.vengrofski 2009-11-05 14:51) - PLID <36091> - Handle hover of links
BOOL CSupportExpiredDlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
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

		if(rcEmail.PtInRect(pt) && m_nxlstaticEmail.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
		if(rcWebSite.PtInRect(pt) && m_nxlstaticWebSite.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.vengrofski 2009-11-05 14:51) - PLID <36091> - Handle clicking on labels
LRESULT CSupportExpiredDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
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

		if(rcEmail.PtInRect(pt) && m_nxlstaticEmail.GetType() == dtsHyperlink) {//Launch email
			ShellExecute(GetSafeHwnd(), NULL, "mailto:insidesales@nextech.com?Subject=Reactivation of NexTech Support", NULL, "", SW_MAXIMIZE);
		}
		if(rcWebSite.PtInRect(pt) && m_nxlstaticWebSite.GetType() == dtsHyperlink) {//Launch browser.
			ShellExecute(NULL, NULL, "http://www.nextech.com/Training/Changes.aspx", NULL, NULL, SW_SHOW); 
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

