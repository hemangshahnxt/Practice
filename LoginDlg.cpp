// CLoginDlg.cpp : implementation file
 
#include "stdafx.h"
#include "LoginDlg.h"
#include "GlobalUtils.h"
//#include "UserListTSet.h"
#include "PracProps.h"
#include "LicenseDlg.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"
#include "client.h"
#include "NxServer.h"
#include "PracticeRc.h"
#include "NxSecurity.h"
#include "AuditTrail.h"

#include "VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (d.thompson 2012-02-17) - PLID 48194 - This was defined in numerous places, I standardized it
const UINT cNoFocus = (UINT)-1;


// (j.jones 2009-06-01 11:45) - PLID 34426 - added enums for the username dropdown
enum UserNameComboColumns {
	unccUserID = 0,
	unccTrueUserName,
	unccDisplayUserName,
	unccAutoLogoff,
	unccAutoLogoffDuration,
};

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog
#define  INVALID_PASSWORD  "Invalid password.  This password is invalid because the database could not be opened.  There is probably an invalid connection to the server, but I only include that information here to make it even more unlikely that the user chooses this string as their password"

extern CPracticeApp theApp;

// (a.walling 2010-07-29 09:58) - PLID 39871 - Moved GetLicenseAgreementAcceptedName to CLicense

//DRT 5/4/2004 - Moved to globalutils, renamed to 
//GetAdvancedUsername() to allow more generic use.
//CString GetLicenseAgreementAcceptedUsername()


CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLoginDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_Password = _T("");
	//}}AFX_DATA_INIT
	m_image = NULL;
	m_nBackColor = 0xE0E0E0;
	//this displays as C5 on QUIGON even if the bitmap is correct, I am still investigating
	//should be C0C0C0 - I checked the bitmap and it seems correct - BVB
	m_bAllowLogin = TRUE;
	m_bNexTechLoginAvailable = FALSE;
	m_bRelogin = FALSE;

	// (a.walling 2008-04-16 17:42) - PLID 29691 - Get the animate window function pointer
	m_User32 = ::LoadLibrary("USER32.DLL");
	if (m_User32) {
		m_pfnAnimateWindow = (AnimateWindowType)GetProcAddress(m_User32, "AnimateWindow");
	} else {	
		m_pfnAnimateWindow = NULL;
	}
	
	// (a.walling 2009-12-16 16:07) - PLID 35784 - Are we in the process of logging in?
	m_bIsLoggingIn = false;
}

CLoginDlg::~CLoginDlg()
{
	if (m_image) {
		DeleteObject(m_image);
	}

	// (a.walling 2008-04-16 17:43) - PLID 29691 - Free our library reference
	if (m_User32) {
		::FreeLibrary(m_User32);
	}
}

// (a.walling 2009-01-21 15:54) - PLID 32657 - CNxIconButtons for login screen buttons
void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Text(pDX, IDC_PASSWORD_BOX, m_Password);
	DDX_Control(pDX, IDC_LOGIN_PROGRESS, m_progLogin);
	DDX_Control(pDX, IDC_LOGIN_LOCATION_LABEL, m_nxstaticLoginLocationLabel);
	DDX_Control(pDX, IDC_USERNAME_LABEL, m_nxstaticUsernameLabel);
	DDX_Control(pDX, IDC_PASSWORD_LABEL, m_nxstaticPasswordLabel);
	DDX_Control(pDX, IDC_LOGIN_STATUS_TEXT, m_nxstaticLoginStatusText);
	DDX_Control(pDX, IDOK, m_nxibLogin);
	DDX_Control(pDX, IDC_VIEW_LICENSE, m_nxibViewLicense);
	DDX_Control(pDX, IDC_EXIT_PRAC_BTN, m_nxibExit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	ON_BN_CLICKED(IDC_EXIT_PRAC_BTN, OnExitPracBtn)
	ON_BN_CLICKED(IDC_VIEW_LICENSE, OnViewLicense)
	ON_WM_ACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_DRAWITEM()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(NXM_LOGIN_PROGRESS, OnLoginProgress)
	ON_REGISTERED_MESSAGE(NXM_PRINTCLIENTINRECT, OnPrintClientInRect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLoginDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLoginDlg)
	ON_EVENT(CLoginDlg, IDC_USER_COMBO, 16 /* SelChosen */, OnSelectionChangeUserNameCmb, VTS_I4)
	ON_EVENT(CLoginDlg, IDC_LOCATION_COMBO, 15 /* SelSet */, OnSelSetLocationCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers

#define SHIFT_CONTROL(idc, n) {	CRect rcTmp;	GetDlgItem(idc)->GetWindowRect(&rcTmp);	ScreenToClient(&rcTmp);	rcTmp.top += n;	rcTmp.bottom += n;	GetDlgItem(idc)->SetWindowPos(NULL, rcTmp.left, rcTmp.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);	}

BOOL CLoginDlg::OnInitDialog() 
{
	try {

		// First do base class init
		CNxDialog::OnInitDialog();

		// (a.walling 2009-01-23 10:46) - PLID 32657 - Text is included in the background image for now
		m_nxstaticUsernameLabel.ShowWindow(SW_HIDE);
		m_nxstaticPasswordLabel.ShowWindow(SW_HIDE);
		m_nxstaticLoginLocationLabel.ShowWindow(SW_HIDE);

		// (a.walling 2008-05-23 13:53) - PLID 30099 - Remove clipchildren style
#ifndef NXDIALOG_NOCLIPCHILDEN
		ModifyStyle(WS_CLIPCHILDREN, 0);
#endif

		SetWindowText("Practice - Log In");
		HICON hIcon = theApp.LoadIcon(IDR_MAINFRAME);
		SetIcon(hIcon, TRUE);			// Set big icon
		SetIcon(hIcon, FALSE);		// Set small icon

		//DRT 6/3/2004 - PLID 12786 - We have worked out a deal with a person
		//	who sells out franchise spas that will be including our software.  
		//	Part of the deal included putting his logo on the splash screen of
		//	Practice.  I modified this code to do the following:
		//	 - Detect if this is a special office (currently a ConfigRT entry)
		//	 - If not, just display the default login with no modifications.
		//	 - If so, we need to expand the size of the login screen (the
		//		splash image is larger), put the new image (IDB_NEXTECH_MEDSPA)
		//		onto the screen, and shift all the controls down a little 
		//		so they are off of the larger image
		// (a.walling 2009-01-21 16:33) - PLID 32657 - Get rid of deprecated code
		//BOOL bDefaultLogin = TRUE;

		//DRT TODO - For now, this will use a ConfigRT setting - I don't feel comfortable
		//	changing the license file in the patch, when its already changed in main in 
		//	other ways.  In the future, we need to make this read from a license file.
		// (c.haag 2005-01-31 17:32) - PLID 15480 - No more MedSpa.
		//if(GetRemotePropertyInt("MedSpaBoutique", 0, 0, "<None>", true) == 1) 
		//They should be using the special login image
		//	bDefaultLogin = FALSE;

		CRect rc;
		GetClientRect(&rc);

		// (a.walling 2009-01-21 16:33) - PLID 32657 - Get rid of deprecated code
		/*
		if(!bDefaultLogin) {
		//if we're not on the default, we need to increase the size of the dialog by a set amount,
		//	and slide all the controls down by the same
		int nMove = 50;
		rc.bottom += nMove;

		//resize the dialog to our new coords and set the z order
		SetWindowPos(&wndTop, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE);

		//Shift all controls on this dialog appropriately
		SHIFT_CONTROL(IDC_LOGIN_PROGRESS, nMove);
		SHIFT_CONTROL(IDC_LOGIN_STATUS_TEXT, nMove);
		SHIFT_CONTROL(IDC_REGISTER, nMove);
		SHIFT_CONTROL(IDC_LOCATION_COMBO, nMove);
		SHIFT_CONTROL(IDC_USER_COMBO, nMove);
		SHIFT_CONTROL(IDC_PASSWORD_BOX, nMove);
		SHIFT_CONTROL(IDOK, nMove);
		SHIFT_CONTROL(IDC_VIEW_LICENSE, nMove);
		SHIFT_CONTROL(IDC_EXIT_PRAC_BTN, nMove);
		SHIFT_CONTROL(IDC_LOGIN_LOCATION_LABEL, nMove);
		SHIFT_CONTROL(IDC_USERNAME_LABEL, nMove);
		SHIFT_CONTROL(IDC_PASSWORD_LABEL, nMove);
		}
		*/

		//On the default dialog, the splash logo is actually the size of the entire dialog.
		//	We need to use a shorter version if we're on this special setup

		// (j.jones 2013-06-24 17:29) - PLID 57281 - the logo now has a TM instead of (R).
		// IDB_SPLASH is res\nextech.jpg
		// Instructions for updating the logo are in:
		// http://yoda/developers/doku.php?id=generating_the_nextech_practice_logo

		int idb = IDB_SPLASH;
		// (c.haag 2005-01-31 17:16) - PLID 15480 - We no longer have a
		// medspa client.
		//if(!bDefaultLogin)
		//	idb = IDB_NEXTECH_MEDSPA;

		// (a.walling 2009-01-21 16:38) - PLID 32657 - We don't need a DIB section; all we are doing is blitting to the screen.

		// (a.walling 2009-08-12 16:06) - PLID 35136 - Need to specify to load as a DIB section, otherwise it will try to map the colors
		// into the current physical palette.

		m_image = NULL;
		Gdiplus::Bitmap* pLoginPicture = NxGdi::BitmapFromJPGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(idb));

		if (pLoginPicture) {
			pLoginPicture->GetHBITMAP(Gdiplus::Color::Black, &m_image);
			delete pLoginPicture;
		}

		// An annoying work-around to make the two combo boxes the same height
	{
		CWnd *pWnd1 = GetDlgItem(IDC_USER_COMBO);
		CWnd *pWnd2 = GetDlgItem(IDC_LOCATION_COMBO);
		CRect rc;
		pWnd1->GetWindowRect(&rc);
		pWnd2->SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOMOVE);
	}

	// (a.walling 2009-01-21 16:33) - PLID 32657 - Get rid of deprecated code
	/*
	DWORD dwVersion = GetVersion();
	DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
	*/

	/*	DRT 4/11/2008 - PLID 29608 - Now that we have implemented a manifest file, the password box
			is doing some very strange things due to the fact that we put 2 boxes, 1 within the other.
			This approach doesn't seem to actually do anything -- I remove the code and cannot find
			any situation in which there is a display issue with the passwords.  The comments do not
			help at all, they don't tell us why we are doing this, and no PLID is referenced.  b.cardillo
			thinks there might have once been an issue with the text not being properly indented.  I've
			removed the outer control, all references and handlers for it, and moved the inner control
			to the proper position, gave it a border, etc.

			if (dwWindowsMajorVersion == 5 && dwWindowsMinorVersion == 1)
			//Windows XP, use the correct password box
			{
			CWnd *pWndPassEnclosing = GetDlgItem(IDC_PASSWORD_BOX2);
			CWnd *pWndPass = GetDlgItem(IDC_PASSWORD_BOX);
			if (pWndPassEnclosing && pWndPass)
			{
			CRect rcPass;
			pWndPassEnclosing->GetWindowRect(&rcPass);
			ScreenToClient(&rcPass);
			rcPass.DeflateRect(7, 3, 3, 3);
			pWndPass->MoveWindow(rcPass);
			//			pWndPassEnclosing->ShowWindow(SW_HIDE);
			pWndPass->ModifyStyle(0, WS_BORDER);
			pWndPass->ModifyStyleEx(0, WS_EX_TRANSPARENT);
			// (b.cardillo 2005-03-21 18:50) - PLID 16018 - We used to enable pWndPass here, and not
			// pWndPassEnclosing, which resulted in an irritating grey block inside the otherwise clean
			// password box.  Initially I added a pWndPassEnclosing->EnableWindow() but then I couldn't
			// figure out why we were enabling here in the first place since we do it later on, so
			// instead I ended up taking the pWndPass->EnableWindow() out, and replacing it with this
			// comment.  I checked sourcesafe history and found that this EnableWindow() was originally
			// added at the same time this branch of the if-statement was added (version 41 in sourcesafe
			// history).  I think it might have just been a mistake at the time, but it's strange because
			// the whole point of that check-in was to try to resolve drawing issues on Windows XP...to
			// me this qualifies as a drawing issue.  But it seems quite strange to enable only one of
			// the password boxes, and even more strange to only do so in this branch of the if-statement.
			// I've tested on Windows XP to try to ensure my change doesn't cause problems.
			}
			}
			else
			{
			// Adjust the position of the real password box to be inside the enclosing one
			CWnd *pWndPassEnclosing = GetDlgItem(IDC_PASSWORD_BOX2);
			CWnd *pWndPass = GetDlgItem(IDC_PASSWORD_BOX);
			if (pWndPassEnclosing && pWndPass) {
			CRect rcPass;
			pWndPassEnclosing->GetWindowRect(&rcPass);
			ScreenToClient(&rcPass);
			rcPass.DeflateRect(7, 3, 3, 3);
			pWndPass->MoveWindow(rcPass);
			//pWndPass->SetWindowPos(pWndPassEnclosing, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED|SWP_SHOWWINDOW|SWP_NOZORDER);
			}
			}
			*/

	// Clear the login status text
	SetDlgItemText(IDC_LOGIN_STATUS_TEXT, "");
	m_progLogin.SetRange(0, 100);

	//*	// (b.cardillo 2005-03-21 18:48) - I'm just putting this in to see how we like it.  I think we 
	// should do something like this to make our login screen more fresh and new and exciting.  
	// This may not be the best attempt we can come up with, but we ought to do SOMETHING.  To turn 
	// this off just take out this block of code.)

	// (a.walling 2009-01-21 16:33) - PLID 32657 - Get rid of deprecated code. We now do this part just by creating our
	// window without a border.
	/*
	if (bDefaultLogin) {
	CRgn rgn;
	{
	//* (b.cardillo 2005-09-13 17:30) - Exclude the window border (this is a request
	// from k.majeed, but I suspect he just wants to see how it looks and we'll take
	// it back out sometime soon).
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	CRect rc;
	GetClientRect(rc);
	ClientToScreen(&rc);
	rc.OffsetRect(-rcWindow.left, -rcWindow.top);
	rgn.CreateRectRgnIndirect(rc);
	//*/

	/* Encircle the ball
	CRect rc;
	GetWindowRect(&rc);
	ScreenToClient(&rc);
	CRgn rgnEllipse;
	{
	CRect rcRegister;
	GetDlgItem(IDC_REGISTER)->GetWindowRect(&rcRegister);
	ScreenToClient(&rcRegister);
	rgn.CreateRectRgn(rc.left, rcRegister.top - 25, rc.right, rc.bottom);
	CRect rcEllipse;
	rcEllipse.SetRect(rc.left, rc.top, rc.right, rcRegister.top);
	rcEllipse.DeflateRect(11, 22, 5, 16);
	rgnEllipse.CreateEllipticRgnIndirect(rcEllipse);
	}
	rgn.CombineRgn(&rgn, &rgnEllipse, RGN_OR);
	//*/
	/*
}
SetWindowRgn(rgn, FALSE);
}
*/
	//*/

	ShowWindow(SW_SHOW);//show here, license may take a while
	Invalidate(TRUE);
	UpdateWindow();

	// (a.walling 2007-05-04 10:09) - PLID 4850 - Everything is already initialized when switching users,
	// so enable the login immediately
	if (m_bRelogin) {
		LoadInitialLoginInfo();
	}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}
	
// (j.jones 2015-04-20 09:11) - PLID 63694 - renamed for clarity
bool CLoginDlg::LoadInitialLoginInfo()
//actually show the combobox, as well as anything we need to log in a user
{
	// Helps for error reporting (in case the login dialog has problems)
	LoadErrorInfoStrings();

	const UINT cNoFocus = (UINT)-1;
	UINT nGiveFocusTo = cNoFocus;
	try {

		// Before we do anything else see prompt the user to agree to the license if they haven't been reminded on this version		
		// (a.walling 2010-07-29 09:58) - PLID 39871 - Moved GetLicenseAgreementAcceptedName to CLicense
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Undecided about the license agreement yet. Should it change to prompt per user now
		// or the remoting machine? Waiting on Ron for an answer to that and I am to leave this as the legacy behavior until I get an answer.
		if (GetRemotePropertyDateTime(GetLicenseAgreementAcceptedPropertyName(), NULL, 0, GetLicenseAgreementUsername(), false).GetStatus() != COleDateTime::valid) {
			if (!DoPromptForLicenseAgreement()) {
				return FALSE;
			}
		}

		
		// (a.walling 2009-12-16 14:44) - PLID 35784 - Display a banner
		// (a.walling 2010-01-18 17:29) - PLID 35784 - No default text
		CString strLoginBanner = GetRemotePropertyText("LoginBanner", "", 0, "<None>", true);
		SetDlgItemText(IDC_LOGIN_STATUS_TEXT, strLoginBanner);

		//DRT 5/16/2008 - PLID 30089 - For the Adv Inventory beta, we need to popup the NDA form on each logon.  We don't need to bother
		//	tracking a separate ConfigRT property, we'll just bounce off of this one.  If they disagree to this one, the next login
		//	will get prompted with both again.
		//DRT 5/21/2008 - PLID 30117 - After we saw it, minds were changed, and we decided not to prompt on login.

		// Attach to the datalists without requerying the userlist
		m_lstLocation = BindNxDataListCtrl(this, IDC_LOCATION_COMBO, GetRemoteData(), true);
		m_lstUser = BindNxDataListCtrl(this, IDC_USER_COMBO, GetRemoteData(), false);
		if (!IsKeyDown(VK_CONTROL)) {
			m_lstUser->PutFromClause(
				"UsersT INNER JOIN UserLocationT ON UsersT.PersonID = UserLocationT.PersonID "
				"INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID "
				"INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID AND UsersT.PersonID > 0");
		} else {
			// Make sure the built-in nextech user is there (this can't be in a mod, because we need to 
			// make sure every time that the user exists and that the user is allowed in any location and 
			// is connected to every resource)
			ExecuteSql(
				"DECLARE @strBuiltInUserName NVARCHAR(500) \r\n"
				"SET @strBuiltInUserName = '%s' \r\n"
				"DECLARE @nBuiltInUserID INT \r\n"
				"SET @nBuiltInUserID = %li \r\n"
				"IF NOT EXISTS (SELECT * FROM UsersT WHERE PersonID = @nBuiltInUserID) BEGIN \r\n"
				"  -- The ID isn't already in use so we're expecting the username to also not be in use because we're going to create it \r\n"
				"  IF NOT EXISTS (SELECT * FROM UsersT WHERE Username = @strBuiltInUserName) BEGIN \r\n"
				"    -- This is the base case, the built-in user hasn't been created yet, so let's create it and ensure the locations and resources \r\n"
				"    BEGIN TRAN \r\n"
				"      -- Create it \r\n"
				"      INSERT INTO PersonT (ID) VALUES (@nBuiltInUserID) \r\n"
				"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
				"      INSERT INTO UsersT (PersonID, Username) VALUES (@nBuiltInUserID, @strBuiltInUserName) \r\n"
				"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
				"      -- And ensure the locations and resources \r\n"
				"      INSERT INTO UserLocationT (PersonID, LocationID) SELECT @nBuiltInUserID, ID FROM LocationsT WHERE Managed = 1 \r\n"
				"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
				"      INSERT INTO UserResourcesT (UserID, ResourceID, Relevence) SELECT @nBuiltInUserID, ID, 0 FROM ResourceT \r\n"
				"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
				"    COMMIT TRAN \r\n"
				"    RETURN \r\n"
				"  END ELSE BEGIN \r\n"
				"    -- The ID doesn't exist but there's a user with the built-in username, so we have to fail \r\n"
				"    RAISERROR('A person with a username of ''%%s'' already exists!', 16, 1, @strBuiltInUserName) \r\n"
				"    ROLLBACK TRAN \r\n" // (a.walling 2011-01-27 13:34) - PLID 34813 - Rollback
				"    RETURN \r\n"
				"  END \r\n"
				"END ELSE BEGIN \r\n"
				"  -- The built-in ID already exists, so we're expecting that user to be the built-in username \r\n"
				"  IF (SELECT Username FROM UsersT WHERE PersonID = @nBuiltInUserID) = @strBuiltInUserName BEGIN \r\n"
				"    -- Good, this is the most common case: We created the built-in user at some earlier date so just ensure the locations and all resources \r\n"
				"    BEGIN TRAN \r\n"
				"      -- Ensure the locations and resources \r\n"
				"      INSERT INTO UserLocationT (PersonID, LocationID) \r\n"
				"        SELECT @nBuiltInUserID, ID FROM LocationsT WHERE Managed = 1 AND ID NOT IN (SELECT LocationID FROM UserLocationT WHERE PersonID = @nBuiltInUserID) \r\n"
				"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
				"      INSERT INTO UserResourcesT (UserID, ResourceID, Relevence) \r\n"
				"        SELECT @nBuiltInUserID, ID, 0 FROM ResourceT WHERE ID NOT IN (SELECT ResourceID FROM UserResourcesT WHERE UserID = @nBuiltInUserID) \r\n"
				"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
				"    COMMIT TRAN \r\n"
				"    RETURN \r\n"
				"  END ELSE BEGIN \r\n"
				"    -- This should never happen because if the @nBuiltInUserID record got added, it should have been added with the correct username \r\n"
				"    RAISERROR('A person with an ID of %%d already exists!', 16, 1, @nBuiltInUserID) \r\n"
				"    ROLLBACK TRAN \r\n" // (a.walling 2011-01-27 13:34) - PLID 34813 - Rollback
				"    RETURN \r\n"
				"  END \r\n"
				"END\r\n", BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME, BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID);

			// Make sure it includes the built-in user
			m_lstUser->PutFromClause(
				"UsersT INNER JOIN UserLocationT ON UsersT.PersonID = UserLocationT.PersonID "
				"INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID "
				"INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID");

			m_bNexTechLoginAvailable = TRUE;
		}
		// Choose the default location
		long nLocId = ChooseDefaultLocation();
		ApplyLocationFilter(nLocId);

		// (d.thompson 2012-02-17) - PLID 48194 - Use shared code to determine the default user
		nGiveFocusTo = ChooseDefaultUser();
	} NxCatchAllCall("CLoginDlg::OnInitDialog Error 1", {
		// Failure
		OnCancel();
		return FALSE;
	});

	// Enable everything
	GetDlgItem(IDC_LOCATION_COMBO)->EnableWindow();
	GetDlgItem(IDC_USER_COMBO)->EnableWindow();
	GetDlgItem(IDOK)->EnableWindow();
	GetDlgItem(IDC_VIEW_LICENSE)->EnableWindow();
	GetDlgItem(IDC_EXIT_PRAC_BTN)->EnableWindow();
	GetDlgItem(IDC_PASSWORD_BOX)->EnableWindow();

	// If the focus needs to be set, try to set it
	try {
		if (nGiveFocusTo != cNoFocus) {	
			GiveFocus(nGiveFocusTo);
		}
	} NxCatchAllCall("CLoginDlg::OnInitDialog Error 1", {
		// Failure
		OnCancel();
		return FALSE;
	});

	// Success!
	m_bAllowLogin = TRUE;
	return TRUE;
}

// (z.manning 2015-12-03 10:45) - PLID 67637 - Added password param
void CLoginDlg::DisallowLogin(const CString &strPassword)
{
	m_bAllowLogin = FALSE;

	GetDlgItem(IDC_LOCATION_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_USER_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_VIEW_LICENSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EXIT_PRAC_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PASSWORD_BOX)->EnableWindow(FALSE);

	if (!m_bRelogin) {
		BOOL bSuccess = theApp.PostLogin(strPassword);
		if (!bSuccess) {
			OnCancel();
		}
	} else {
		// (a.walling 2007-05-04 10:10) - PLID 4850 - Switching users, we need to tell PostLogin this.
		// and also HandleUserLogin.
		if (GetMainFrame())
			GetMainFrame()->LoadTitleBarText();

		BOOL bSuccess = theApp.PostLogin(strPassword, FALSE, this); // not initial login
		if (bSuccess) {
			if (GetMainFrame())
				GetMainFrame()->HandleUserLogin(FALSE);
			else
				ASSERT(FALSE);

			CDialog::OnOK(); // As a modal dialog, this will also 'delete this' in PostNcDestroy
		} else {
			ASSERT(FALSE);
			MessageBox("There was an error logging in! Practice will now close.");
			
			CDialog::OnCancel();

			if (GetMainFrame())
				GetMainFrame()->DestroyWindow();

			PostQuitMessage(0);
		}
	}
}

void CLoginDlg::OnSelectionChangeUserNameCmb(long iNewRow) 
{
	if (iNewRow != -1) {
		try {
			// (j.jones 2009-06-01 11:49) - PLID 34426 - use the actual username,
			// not the displayed one where we may have artificially changed the case
			ChooseUser(VarString(m_lstUser->Value[iNewRow][unccTrueUserName]));
		} NxCatchAll("CLoginDlg::OnSelectionChangeUserNameCmb");
	}
}

CString CLoginDlg::GetSystemName()
{
	HKEY KeyHandle;
	CString strSystemName;
	DWORD ReturnType;
	DWORD ReturnLen;

	KeyHandle = NULL;

	RegOpenKeyEx(HKEY_LOCAL_MACHINE , "System\\CurrentControlSet\\control\\ComputerName\\ComputerName",0,KEY_READ, &KeyHandle);

    RegQueryValueEx(KeyHandle, "ComputerName" , 0 , &ReturnType , (unsigned char *)strSystemName.GetBuffer(MAX_PATH), &ReturnLen);
	strSystemName.ReleaseBuffer();

	RegCloseKey(KeyHandle);

	return strSystemName;
}

// (a.walling 2007-11-06 17:19) - PLID 27998 - VS2008 - This is obsolete and unreferenced
/*
CString CLoginDlg::ConvertText(CString InputString, CString Direction)
{
	CString OutString;
	char AddNum;
	int x;
	int len;

	OutString = "";

	Direction.MakeUpper();

	if (Direction == "TOGOOD") {AddNum = -100;}
	else {AddNum = 100;}

	len = InputString.GetLength();
	for(x = 0; x < len; x++)
	{
		OutString += CString(InputString.GetAt(x) + AddNum);
//		OutString.Format("%s%s",OutString, CString(InputString.GetAt(x) + AddNum));
	}

	return OutString;
}
*/

void CLoginDlg::OnExitPracBtn() 
{
	OnCancel();
}

// Returns the selected location ID or -1 on failure
long CLoginDlg::ChooseDefaultLocation()
{
	// Try to get the last selected location	
	// (d.thompson 2012-02-17) - PLID 48194 - This has always saved the value per-path.  Now that we have concurrent 
	//	TS licensing, the path is going to be the same for everyone on that TS, so everyone shares this setting, and
	//	it just bounces around a lot.  We should instead save it per-Windows user per-Computer
	//Default to whatever value got saved last in the "LastUser" (this may or may not be what they want, depending
	//	how settings have changed, but, much like Obi-wan before it, it's our only hope).
	// (d.thompson 2012-05-29) - PLID 48194 - On good advice, I changed the default to -2, and if we get that, we know
	//	this is the first time the user has checked for this new property.  Then and only then do we do the second
	//	round trip to get the old ID
	long nLastLocId = GetRemotePropertyInt("LastLocationIDPerWinUser", -2, 0, GetAdvancedUsername(), true);
	if(nLastLocId == -2) {
		// (d.thompson 2012-05-29) - PLID 48194 - This was the old property
		nLastLocId = GetPropertyInt("LastLocationID", -1);
	}

	if (nLastLocId == -1) {
		// There was no last selected location so try to default to the first one in the list
		m_lstLocation->CurSel = 0;
		if (m_lstLocation->CurSel != -1) {
			nLastLocId = VarLong(m_lstLocation->Value[m_lstLocation->CurSel][0]);
		} else {
			// Nothing in the list
			return -1;
		}
	} else {
		// Got the last selected location so try to select it now
		m_lstLocation->SetSelByColumn(0, nLastLocId);
		long nPos = m_lstLocation->CurSel;
		if (nPos == -1) {
			// Couldn't set the selection for some reason so 
			// try to default to the first item in the list
			m_lstLocation->CurSel = 0;
			long nCount = m_lstLocation->GetRowCount();
			if (m_lstLocation->CurSel != -1) {
				nLastLocId = VarLong(m_lstLocation->Value[m_lstLocation->CurSel][0]);
			} else {
				// Nothing in the list
				return -1;
			}
		}
	}

	// If we made it here, we have selected a default 
	// location so filter the user list based on it
	return nLastLocId;
}

bool CLoginDlg::ChooseUser(const CString &strUsername)
{
	try 
	{
		// Try to find the specified user

		// (j.jones 2009-06-01 11:49) - PLID 34426 - use the actual username,
		// not the displayed one where we may have artificially changed the case
		long nUserIndex = m_lstUser->FindByColumn(unccTrueUserName, _bstr_t(strUsername), 0, FALSE);
		if (nUserIndex != -1) 
		{
			// Find out what name the list thinks is selected
			CString strLocUser = VarString(m_lstUser->Value[nUserIndex][unccTrueUserName]);

			// Get the currently selected location
			long nLocLocationID;
			long nLocIndex = m_lstLocation->GetCurSel();
			if (nLocIndex != -1) {
				nLocLocationID = VarLong(m_lstLocation->GetValue(nLocIndex, 0));
			} else {
				nLocLocationID = -1;
			}

			// Found requested username so look for the password
			CString strPassword;
			bool bAutoRecall = false;
			// (j.jones 2008-11-19 10:51) - PLID 28578 - added bIsPasswordVerified as a parameter, but it is NULL here
			if (LoadUserInfo(strLocUser, nLocLocationID, &strPassword, NULL, &bAutoRecall)) 
			{
				if (bAutoRecall) 
					m_Password = strPassword;
				else 
					m_Password.Empty();

				// (j.jones 2009-06-01 12:39) - PLID 34426 - if the current user ID is the same as
				// the user ID we're about to select, don't bother changing the selection
				BOOL bChangeSelection = TRUE;
				if(m_lstUser->CurSel != -1 && nUserIndex != -1
					&& VarLong(m_lstUser->Value[m_lstUser->CurSel][unccUserID]) == VarLong(m_lstUser->Value[nUserIndex][unccUserID])) {
					
					bChangeSelection = FALSE;
				}

				if(bChangeSelection) {
					m_lstUser->CurSel = nUserIndex;
				}
				UpdateData(FALSE);
				return true;
			} 
			else return false;
		} 
		else return false;
	} NxCatchAllCall("CLoginDlg::ChooseUser Error 1", return false);
}

void CLoginDlg::GiveFocus(UINT nID)
{
	CWnd *pWnd = GetDlgItem(nID);
	pWnd->SetFocus();
	if (nID == IDC_PASSWORD_BOX) {
		CString str;
		pWnd->GetWindowText(str);
		((CEdit *)pWnd)->SetSel(0, str.GetLength());
	}
}

BOOL CLoginDlg::DoPromptForLicenseAgreement()
{
	// They haven't been reminded
	CLicenseDlg dlg(this);
	if (dlg.DoModal() == IDOK) {
		// They accepted the agreement, so write that to the data (this is a machine-based setting, so use SetProperty, not SetRemoteProperty)		
		// (a.walling 2010-07-29 09:58) - PLID 39871 - Moved GetLicenseAgreementAcceptedName to CLicense
		// (r.gonet 2016-05-19 18:13) - NX-100689 - Force the license prompt to be per the local machine.
		SetRemotePropertyDateTime(GetLicenseAgreementAcceptedPropertyName(), COleDateTime::GetCurrentTime(), 0, GetLicenseAgreementUsername());
		return TRUE;
	} else {
		// They did not accept the agreement, cancel the login dialog
		OnCancel();
		return FALSE;
	}
}

void CLoginDlg::OnViewLicense() 
{
	try {
		//DRT 5/26/2004 - PLID 12612 - Just pop up the license
		//	and do not give them the options to agree.
		CLicenseDlg dlg(this);
		dlg.m_bRequireAgreement = FALSE;
		dlg.DoModal();
	} NxCatchAll("CLoginDlg::OnViewLicense");
}

void CLoginDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	
	if (nState == WA_INACTIVE && m_bAllowLogin) {
		SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);	
	}
}

void CLoginDlg::ApplyLocationFilter(long nLocId)
{
	CString strUserFilter;
//	strUserFilter.Format("LocationID = -1 OR LocationID = %li", nLocId);
	strUserFilter.Format("UserLocationT.LocationID = %li AND PersonT.Archived = 0", nLocId);
	// (a.walling 2007-11-06 17:21) - PLID 27998 - VS2008 - Disambiguate this comparison
	if (strUserFilter.Compare(m_lstUser->WhereClause) == 0) {
		//do not requery the list
		return;
	}

	long nCurSel = m_lstUser->CurSel;
	long nCurUserID = -1;
	if (nCurSel != -1) {
		nCurUserID = VarLong(m_lstUser->Value[nCurSel][unccUserID]);
	}
	m_lstUser->WhereClause = _bstr_t(strUserFilter);

	// (j.jones 2009-06-01 12:00) - PLID 34426 - This is likely only needed for CCHIT, as it
	// doesn't make any sense, but if the DisplayLoginUsersInUpperAndLowerCases setting is enabled
	// (can only be done through a query), then show each username once in all uppercase, and once
	// in all lowercase, ignoring whether or not the actual name is a mixed case.
	if(GetRemotePropertyInt("DisplayLoginUsersInUpperAndLowerCases", 0, 0, "<None>", true) == 1) {
		//we've enabled this silly preference, so rather than requerying, load up the results
		//in a recordset so we can manually manipulate the usernames and add rows
		CString strFromClause = (LPCTSTR)m_lstUser->FromClause;
		CString strWhereClause = (LPCTSTR)m_lstUser->WhereClause;
		strFromClause.TrimRight(" ");
		strWhereClause.TrimRight(" ");
		if(strFromClause.IsEmpty() || strWhereClause.IsEmpty()) {
			//safety check, should be impossible
			ASSERT(FALSE);
			//simply requery
			m_lstUser->Requery();
		}
		else {
			CString strSql;
			strSql.Format("SELECT UsersT.PersonID, UsersT.UserName, "
				"UsersT.AutoLogoff, UsersT.AutoLogoffDuration "
				"FROM %s WHERE %s", strFromClause, strWhereClause);
			ADODB::_RecordsetPtr rs = CreateRecordsetStd(strSql);
				
			m_lstUser->Clear();

			while(!rs->eof) {
				long nPersonID = AdoFldLong(rs, "PersonID");
				CString strUserName = AdoFldString(rs, "UserName", "");
				_variant_t varAutoLogoff = rs->Fields->Item["AutoLogoff"]->Value;
				long nAutoLogoffDuration = AdoFldLong(rs, "AutoLogoffDuration", 0);

				//add the default case
				{
					NXDATALISTLib::IRowSettingsPtr pRow = m_lstUser->GetRow(-1);
					pRow->PutValue(unccUserID, nPersonID);
					pRow->PutValue(unccTrueUserName, (LPCTSTR)strUserName);
					pRow->PutValue(unccDisplayUserName, (LPCTSTR)strUserName);
					pRow->PutValue(unccAutoLogoff, varAutoLogoff);
					pRow->PutValue(unccAutoLogoffDuration, nAutoLogoffDuration);
					m_lstUser->AddRow(pRow);
				}

				//add with lowercase name
				{
					CString strLower = strUserName;
					strLower.MakeLower();
					// (j.jones 2013-08-08 11:39) - PLID 57915 - don't add if this case
					// is exactly the same as our regular case
					if(strLower != strUserName) {
						NXDATALISTLib::IRowSettingsPtr pRow = m_lstUser->GetRow(-1);
						pRow->PutValue(unccUserID, nPersonID);
						pRow->PutValue(unccTrueUserName, (LPCTSTR)strUserName);
						pRow->PutValue(unccDisplayUserName, (LPCTSTR)strLower);
						pRow->PutValue(unccAutoLogoff, varAutoLogoff);
						pRow->PutValue(unccAutoLogoffDuration, nAutoLogoffDuration);
						m_lstUser->AddRow(pRow);
					}
				}

				//add with uppercase name
				{
					CString strUpper = strUserName;
					strUpper.MakeUpper();
					// (j.jones 2013-08-08 11:39) - PLID 57915 - don't add if this case
					// is exactly the same as our regular case
					if(strUpper != strUserName) {
						NXDATALISTLib::IRowSettingsPtr pRow = m_lstUser->GetRow(-1);
						pRow->PutValue(unccUserID, nPersonID);
						pRow->PutValue(unccTrueUserName, (LPCTSTR)strUserName);
						pRow->PutValue(unccDisplayUserName, (LPCTSTR)strUpper);
						pRow->PutValue(unccAutoLogoff, varAutoLogoff);
						pRow->PutValue(unccAutoLogoffDuration, nAutoLogoffDuration);
						m_lstUser->AddRow(pRow);
					}
				}

				rs->MoveNext();
			}
			rs->Close();

			//ensure the list is sorted
			m_lstUser->Sort();
		}
	}
	else {
		//normal requery
		m_lstUser->Requery();
	}

	if(nCurUserID != -1) {

		m_lstUser->SetSelByColumn(unccUserID, nCurUserID);

		// (j.jones 2015-04-20 09:06) - PLID 63694 - if we remembered the last selected user,
		// we will not call ChooseUser to try and load their password, it should have
		// already been loaded
	}
	else {

		// (j.jones 2015-04-20 09:10) - PLID 63694 - refactored this so ChooseUser
		// is only called if we arbitrarily set the first user in the list
		m_lstUser->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
		if (m_lstUser->GetRowCount() > 0) {
			m_lstUser->PutCurSel(0);

			//DRT 9/25/03 - PLID 9629 - We might have set a default user (the first person in the list). If so, 
			//		we need to set the user (including the "remember my password" stuff).
			// (j.jones 2009-06-01 11:49) - PLID 34426 - use the actual username,
			// not the displayed one where we may have artificially changed the case
			ChooseUser(VarString(m_lstUser->Value[0][unccTrueUserName]));
		}
	}

	//DRT 9/25/03 - Is there any case in which this can ever be executed?
	// (j.jones 2015-04-20 09:19) - PLID 63694 - you can get here if the previously
	// selected user no longer exists in the list after switching locations
	if (m_lstUser->CurSel == -1 && m_lstUser->GetRowCount() > 0) {
		m_lstUser->PutCurSel(0);
		// (j.jones 2009-06-01 11:49) - PLID 34426 - use the actual username,
		// not the displayed one where we may have artificially changed the case
		ChooseUser(VarString(m_lstUser->Value[0][unccTrueUserName]));
	}
}

void CLoginDlg::OnSelSetLocationCombo(long nRow) 
{
	long nNewLocId = -1;
	if (nRow != -1) {
		try {
			nNewLocId = VarLong(m_lstLocation->Value[nRow][0]);
		} NxCatchAll("CLoginDlg::OnSelSetLocationCombo 1");
	}

	try {
		ApplyLocationFilter(nNewLocId);
	} NxCatchAll("CLoginDlg::OnSelSetLocationCombo 2");
}

// (a.walling 2007-11-06 17:16) - PLID 27998 - VS2008 - NcHitTest needs an LRESULT
LRESULT CLoginDlg::OnNcHitTest(CPoint point)
{
//	UINT nAns = CDialog::OnNcHitTest(point);
//	return nAns;
	return HTCAPTION;
}

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int nID = pWnd->GetDlgCtrlID();

	if (nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_BTN || nCtlColor == CTLCOLOR_DLG) {

		//DRT 4/11/2008 - PLID 29608 - Apparently, when a CEdit is read only or disabled, it comes through as CTLCOLOR_STATIC!  Thus, 
		//	since our dialog starts off disabled everything, the edit box is trying to be transparent, which
		//	the manifest implementation of all controls really doesn't like, and just makes it black while it's in the disabled state.
		//	We'll have to manually watch for the specific box and make sure not to make it transparent.
		
		if(nID == IDC_PASSWORD_BOX) {
			//Do nothing
		}
		else {
			pDC->SetBkColor(m_nBackColor);
			// (b.cardillo 2005-09-14 10:42) - PLID 17523 - We used to set it to transparent for 
			// everything EXCEPT the IDC_LOGIN_STATUS_TEXT control.  But now that control handles 
			// itself better so it can be transparent too.  See this pl item's other comment for 
			// more info.
			pDC->SetBkMode(TRANSPARENT);
			if (!m_brBrush.m_hObject) {
				m_brBrush.CreateSolidBrush(m_nBackColor);
			}
			hbr = (HBRUSH)::GetStockObject(NULL_BRUSH);//m_hBrush;

		}
	}

	// (a.walling 2009-01-27 10:38) - PLID 32657 - Use yellow for login progress text color
	// (a.walling 2009-02-03 16:55) - PLID 32657 - Not anymore
	//E9F7AA
	// (a.walling 2009-12-16 14:42) - PLID 35784 - And it's back
	// (r.gonet 04/03/2014) - PLID 61628 - Gone again! Login background changed. The color makes it unreadable.
	if (!m_bIsLoggingIn && nID == IDC_LOGIN_STATUS_TEXT) {
		pDC->SetTextColor(RGB(0, 0, 0));
	} else {
		pDC->SetTextColor(RGB(0, 0, 0));
	}
	return hbr;
}

void CLoginDlg::OnCancel()
{
//	GracefullyCrash(); doesn't work :(
//	PostQuitMessage(0);
	// (a.walling 2007-05-04 10:14) - PLID 4850 - When switching users, it is similar to user switching in windows. At the moment, the
	// previous user is logged off, and no one is logged in. Cancelling to a completely disabled application is pointless; they may as
	// well leave the dialog open. So if the user decides to cancel, close the application entirely.
	if (m_bRelogin) {
		long nResult = MessageBox("The current session has been locked to switch users. You must log in to continue using the program. Are you sure you wish to close NexTech Practice?", "Practice", MB_YESNO | MB_ICONEXCLAMATION);

		if (nResult == IDYES) {
			CDialog::OnCancel();
			if (GetMainFrame()) {
				GetMainFrame()->SendMessage(NXM_FORCIBLY_CLOSE, NULL, NULL);
			}
			PostQuitMessage(0);
		} else {
			return;
		}
	}
	else
		DestroyWindow();
}

void CLoginDlg::OnOK() 
{
	if (UpdateData()) {
		try {
			long nUserID, nCurSel, nInactivityMinutes;
			CString strUsername, strLocationName;
			CString strPassword = m_Password;
			nCurSel = m_lstUser->GetCurSel();
			if(nCurSel==-1) {
				MsgBox("Invalid username.  Please choose one from the drop-down list");
				GiveFocus(IDC_USER_COMBO);
				return;
			}
			// (j.jones 2009-06-01 11:49) - PLID 34426 - use the actual username,
			// not the displayed one where we may have artificially changed the case
			_variant_t var = m_lstUser->Value[nCurSel][unccTrueUserName];
			nUserID = VarLong(m_lstUser->GetValue(nCurSel, unccUserID));
			if (var.vt == VT_BSTR) {
				strUsername = (LPCTSTR)_bstr_t(var.bstrVal);
			}
			long nLocIndex = m_lstLocation->CurSel;
			if(nLocIndex==-1) {
				MsgBox("Invalid location.  Please choose one from the drop-down list");
				GiveFocus(IDC_LOCATION_COMBO);
				return;
			}
			long nLocId = -1;
			if (nLocIndex != -1) {
				nLocId = VarLong(m_lstLocation->Value[nLocIndex][0]);
			}
			var = m_lstLocation->Value[nLocIndex][1];
			if (var.vt == VT_BSTR) {
				strLocationName = (LPCTSTR)_bstr_t(var.bstrVal);
			}
			if (VarBool(m_lstUser->Value[nCurSel][unccAutoLogoff]))
				nInactivityMinutes = VarLong(m_lstUser->Value[nCurSel][unccAutoLogoffDuration]);
			else
				nInactivityMinutes = -1;

			int nFailureReason = 0;
			HANDLE hUserHandle = LogInUser(strUsername, strPassword, nLocId, nInactivityMinutes, strLocationName, &nFailureReason);

			// (z.manning 2009-06-10 15:23) - PLID 34585 - If we loaded a user successfully then the user's
			// permissions are loaded as well. Let's check and see if the user has at least one permission
			// before we permit login.
			if (hUserHandle != NULL && !DoesCurrentUserHaveAnyPermissions()) {
				hUserHandle = NULL;
			}

			if (hUserHandle) {
				// Success, close the login dialog

				// (a.walling 2009-12-16 16:07) - PLID 35784 - Are we in the process of logging in?
				m_bIsLoggingIn = true;
				// (z.manning 2015-12-03 11:07) - PLID 67637 - Pass in the password
				DisallowLogin(strPassword);
			}
			else
			{
				// (z.manning 2009-04-30 10:57) - PLID 28576 - CCHIT requires that login messages be as generic
				// as possible and not give the reason for failed authentication. Note: testing script 5.42
				// specifically mentions user privileges which is why I don't handle that case either even
				// though authentication was successful.
				//TES 4/30/2009 - PLID 28573 - Actually, there is one case in which we will give a different 
				// error message, and that's if their account is locked (nFailureReason == 5) {
				if (nFailureReason == 5) {
					MessageBox("This account has exceeded the maximum number of login attempts, and has been locked.  Please contact an Administrator in order to unlock this account.");
				}
				else {
					MessageBox("Your login attempt was unsuccessful.", "Login Failed", MB_ICONERROR);
				}

				// (b.savon 2016-03-14 14:45) - PLID 67718 - Supplemental -- Reset to Welcome! text
				CString strLoginBanner = GetRemotePropertyText("LoginBanner", "", 0, "<None>", true);
				BSTR bstrProgress = strLoginBanner.AllocSysString();
				OnLoginProgress(0, (LPARAM)bstrProgress);

				// (e.lally 2009-06-12) PLID 34608 - Audit the failed login attempt.
				int nAuditID;
				nAuditID = BeginNewAuditEvent();
				if (nAuditID != -1) {
					//Don't need to include new value / old value, the current location and username will be blank since we
					//	aren't actually logged in.
					AuditEvent(-1, strUsername, nAuditID, aeiFailedLogin, nUserID, "", "", aepMedium);
				}
				// (z.manning 2009-04-30 11:01) - PLID 28576 - No reason to put focus anywhere since we don't
				// know what failed.  Plus CCHIT may interpret that as giving our users a hint and heaven
				// forbid we make things user-friendly.

				/* (z.manning 2009-04-30 11:08) - PLID 28576 - Keeping the old code here in case we ever
				decide to bring this back after certification.
				// Failure of some sort
				ASSERT(nFailureReason != 0);
				// Handle the failure appropriately
				switch (nFailureReason) {
				case 1:
				MsgBox("Invalid username.  Please choose one from the drop-down list");
				GiveFocus(IDC_USER_COMBO);
				break;
				case 2:
				// (j.jones 2008-11-19 12:46) - PLID 28578 - passwords are now case-sensitive, so we must warn accordingly
				MsgBox("You have entered an invalid password.  Please re-type the correct password.\n\n"
				"Be sure to use the correct uppercase and lowercase characters in your password.");
				GiveFocus(IDC_PASSWORD_BOX);
				break;
				case 3: // The user had to change their password, but they changed their mind
				break;
				case 4: // The user has no permissions
				MsgBox("There are no permissions configured for this user name. \n"
				"Please contact an administrator to assign permissions to this user in the Contacts module, in 'User Permissions'.");
				GiveFocus(IDC_PASSWORD_BOX);
				break;
				default:
				// Something was wrong
				ASSERT(FALSE);
				MsgBox("Invalid username or password.");
				return;
				}*/
			}
			return;
		} NxCatchAll("CLoginDlg::OnOK Error 1");
		if (!m_bRelogin) // (a.walling 2007-05-04 10:15) - PLID 4850 - Don't post this if we are switching users
			PostQuitMessage(0);
	}
}

void CLoginDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
}

BOOL CLoginDlg::OnEraseBkgnd(CDC* pDC) 
{
	if (m_image) {
		// Do the normal erase (doesn't hurt and gives us a nice clean background to work with)
		// (a.walling 2009-01-21 16:18) - PLID 32567 - This will cause flickering; we are already filling
		// the entire background with a bitmap anyway.
		//CNxDialog::OnEraseBkgnd(pDC);

		// Draw our splash bitmap on the clean background
		{
			// Make sure we are drawing under the correct color palette
			CPalette *newPal, *oldPal;
			newPal = &theApp.m_palette;
			oldPal = pDC->SelectPalette(newPal, FALSE);
			pDC->RealizePalette();

			// Get the dialog dimensions
			CRect rc;
			GetClientRect(&rc);

			// Draw the bitmap
			// (a.walling 2009-01-21 16:17) - PLID 32567 - This is dead code
			/*
			BITMAP bmpTmp;
			GetObject(m_image, sizeof(bmpTmp), &bmpTmp);
			*/
			DrawBitmap(pDC, (HBITMAP)m_image, 0, 0, rc.Width(), rc.Height());//BVB - align left, since the background colors don't match, and we don't need to center since bitmap and dialog are the same width

			// Go back to our previous palette
			pDC->SelectPalette(oldPal, FALSE);
		}

		return TRUE;
	} else {
		return CNxDialog::OnEraseBkgnd(pDC);
	}
}

// (a.walling 2010-06-09 16:03) - PLID 39087 - Prints the background and any NxColors into HDC (wParam) using the screen rect (lParam)
LRESULT CLoginDlg::OnPrintClientInRect(WPARAM wParam, LPARAM lParam)
{
	CRect rcTargetScreen((LPRECT)lParam);
	
	if (m_image) {
		// Draw our splash bitmap on the clean background
		{
			CDC dc;
			dc.Attach((HDC)wParam);

			// lParam is LPCRECT -- the rect to draw in, in screen coordinates

			CRect rcScreen;
			GetWindowRect(rcScreen);

			CRect rcTarget = rcTargetScreen;

			ScreenToClient(rcTarget);

			CRect rc;
			GetClientRect(rc);

			int nOffsetX = rc.left - rcTarget.left;
			int nOffsetY = rc.top - rcTarget.top;

			// Draw the bitmap
			// (a.walling 2009-01-21 16:17) - PLID 32567 - This is dead code
			/*
			BITMAP bmpTmp;
			GetObject(m_image, sizeof(bmpTmp), &bmpTmp);
			*/
			DrawBitmap(&dc, (HBITMAP)m_image, nOffsetX, nOffsetY, rc.Width(), rc.Height());//BVB - align left, since the background colors don't match, and we don't need to center since bitmap and dialog are the same width
	
			dc.Detach();
		}

		return 0x7FFFFFFF; // let caller know we handled it.
	} else {
		return CNexTechDialog::OnPrintClientInRect(wParam, lParam);
	}
}

void CLoginDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// (a.walling 2009-01-21 15:54) - PLID 32657 - CNxIconButtons for login screen buttons, so this is now obsolete
	/*
	CRect rect = &(lpDrawItemStruct->rcItem);
	CDC *pDC	= CDC::FromHandle(lpDrawItemStruct->hDC);
	CBrush	brBlack;
	CButton *pWnd;

	pWnd = (CButton *)GetDlgItem(nIDCtl);//unsafe cast

	if (!pWnd->IsWindowEnabled())
	{	brBlack.Attach((HBRUSH)GetStockObject(GRAY_BRUSH));
		pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else brBlack.Attach((HBRUSH)GetStockObject(BLACK_BRUSH));

	if (pWnd->GetState() & 0x4)
		pDC->DrawEdge(rect, EDGE_SUNKEN, BF_RECT);
	else
		pDC->DrawEdge(rect, BDR_RAISEDINNER, BF_RECT);

	rect.DeflateRect(1,1,1,1);
	pDC->FillSolidRect(rect, m_nBackColor);
	pDC->SetBkColor(m_nBackColor);
	
	switch (nIDCtl)
	{
		case IDOK:
			pDC->DrawText("Log In", rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		break;
		case IDC_VIEW_LICENSE:
			pDC->DrawText("License &Agreement", rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		break;
		case IDC_EXIT_PRAC_BTN:
			pDC->DrawText("Cancel", rect, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		break;
	}
	brBlack.Detach();
	*/
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

LRESULT CLoginDlg::OnLoginProgress(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	// (a.walling 2008-05-06 17:15) - PLID 29940 - Less ambiguous conversion
	_bstr_t bstrProgress((BSTR)lParam, false);
	m_progLogin.SetPos(wParam);
	// (c.haag 2003-09-06 11:31) - The static control resizes itself, so we
	// need to force it to clear the area.
	SetDlgItemText(IDC_LOGIN_STATUS_TEXT, "                                                                                                                  ");
	// (a.walling 2007-11-08 16:17) - PLID 27476 - We are sending a BSTR, so we need to ensure we are converting it properly. Previously we sent an
	// LPCSTR improperly formatted as a BSTR, and this improper conversion below worked with that. But now we are correctly sending the BSTR, so
	// we need to ensure it is converted correctly as well.
	SetDlgItemText(IDC_LOGIN_STATUS_TEXT, (LPCTSTR)bstrProgress);
	// (a.walling 2008-05-06 17:11) - PLID 29940 - We created the _bstr_t holder to NOT use a copy, so it will free the underlying BSTR in its dtor.
	//SysFreeString(bstrProgress);
	
	// (b.cardillo 2005-09-14 10:39) - PLID 17523 - Invalidate and redraw the whole status 
	// text area.  I'm pretty sure this accounts for the "resizing" thing above because I 
	// don't actually think it resizes itself because that would be amazing.  But since 
	// c.haag's comment says so and it's not a performance hit or anything, I'm leaving 
	// that code as is for now.  We can change it later if necessary.
	{
		CRect rc;
		GetDlgItem(IDC_LOGIN_STATUS_TEXT)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		RedrawWindow(rc);
	}

	return 0;
}

BOOL CLoginDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message) {
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_CONTROL || pMsg->wParam == VK_SHIFT || pMsg->wParam == 'T') {

			if(!m_bAllowLogin)
				return TRUE;

			// (j.jones 2005-01-28 10:05) - if one holds down Ctrl-Shift-T, bring up the NexTech Login
			if((GetAsyncKeyState(VK_CONTROL) & 0xE000) && (GetAsyncKeyState(VK_SHIFT) & 0xE000) && (GetAsyncKeyState('T') & 0xE000)) {
				//but obviously don't bother if it is already available
				if(m_bNexTechLoginAvailable)
					return TRUE;

				// (j.jones 2010-05-03 11:03) - PLID 38457 - added exception handling
				try {

					// Make sure the built-in nextech user is there (this can't be in a mod, because we need to 
					// make sure every time that the user exists and that the user is allowed in any location and 
					// is connected to every resource)
					ExecuteSql(
						"DECLARE @strBuiltInUserName NVARCHAR(500) \r\n"
						"SET @strBuiltInUserName = '%s' \r\n"
						"DECLARE @nBuiltInUserID INT \r\n"
						"SET @nBuiltInUserID = %li \r\n"
						"IF NOT EXISTS (SELECT * FROM UsersT WHERE PersonID = @nBuiltInUserID) BEGIN \r\n"
						"  -- The ID isn't already in use so we're expecting the username to also not be in use because we're going to create it \r\n"
						"  IF NOT EXISTS (SELECT * FROM UsersT WHERE Username = @strBuiltInUserName) BEGIN \r\n"
						"    -- This is the base case, the built-in user hasn't been created yet, so let's create it and ensure the locations and resources \r\n"
						"    BEGIN TRAN \r\n"
						"      -- Create it \r\n"
						"      INSERT INTO PersonT (ID) VALUES (@nBuiltInUserID) \r\n"
						"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
						"      INSERT INTO UsersT (PersonID, Username) VALUES (@nBuiltInUserID, @strBuiltInUserName) \r\n"
						"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
						"      -- And ensure the locations and resources \r\n"
						"      INSERT INTO UserLocationT (PersonID, LocationID) SELECT @nBuiltInUserID, ID FROM LocationsT WHERE Managed = 1 \r\n"
						"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
						"      INSERT INTO UserResourcesT (UserID, ResourceID, Relevence) SELECT @nBuiltInUserID, ID, 0 FROM ResourceT \r\n"
						"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
						"    COMMIT TRAN \r\n"
						"    RETURN \r\n"
						"  END ELSE BEGIN \r\n"
						"    -- The ID doesn't exist but there's a user with the built-in username, so we have to fail \r\n"
						"    RAISERROR('A person with a username of ''%%s'' already exists!', 16, 1, @strBuiltInUserName) \r\n"
						"    ROLLBACK TRAN \r\n" // (a.walling 2011-01-27 13:34) - PLID 34813 - Rollback
						"    RETURN \r\n"
						"  END \r\n"
						"END ELSE BEGIN \r\n"
						"  -- The built-in ID already exists, so we're expecting that user to be the built-in username \r\n"
						"  IF (SELECT Username FROM UsersT WHERE PersonID = @nBuiltInUserID) = @strBuiltInUserName BEGIN \r\n"
						"    -- Good, this is the most common case: We created the built-in user at some earlier date so just ensure the locations and all resources \r\n"
						"    BEGIN TRAN \r\n"
						"      -- Ensure the locations and resources \r\n"
						"      INSERT INTO UserLocationT (PersonID, LocationID) \r\n"
						"        SELECT @nBuiltInUserID, ID FROM LocationsT WHERE Managed = 1 AND ID NOT IN (SELECT LocationID FROM UserLocationT WHERE PersonID = @nBuiltInUserID) \r\n"
						"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
						"      INSERT INTO UserResourcesT (UserID, ResourceID, Relevence) \r\n"
						"        SELECT @nBuiltInUserID, ID, 0 FROM ResourceT WHERE ID NOT IN (SELECT ResourceID FROM UserResourcesT WHERE UserID = @nBuiltInUserID) \r\n"
						"      IF (@@ERROR <> 0) BEGIN ROLLBACK TRAN RETURN END \r\n"
						"    COMMIT TRAN \r\n"
						"    RETURN \r\n"
						"  END ELSE BEGIN \r\n"
						"    -- This should never happen because if the @nBuiltInUserID record got added, it should have been added with the correct username \r\n"
						"    RAISERROR('A person with an ID of %%d already exists!', 16, 1, @nBuiltInUserID) \r\n"
						"    ROLLBACK TRAN \r\n" // (a.walling 2011-01-27 13:34) - PLID 34813 - Rollback
						"    RETURN \r\n"
						"  END \r\n"
						"END\r\n", BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME, BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID);

					m_lstUser->PutFromClause(
						"UsersT INNER JOIN UserLocationT ON UsersT.PersonID = UserLocationT.PersonID "
						"INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID "
						"INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID");

					//this will force a requery
					m_lstUser->WhereClause = "";

					m_bNexTechLoginAvailable = TRUE;

					// Choose the default location
					long nLocId = ChooseDefaultLocation();
					ApplyLocationFilter(nLocId);

					UINT nGiveFocusTo = cNoFocus;

					// (d.thompson 2012-02-17) - PLID 48194 - Use shared code to determine the default user
					nGiveFocusTo = ChooseDefaultUser();
				
				}NxCatchAll(__FUNCTION__);
			
				return TRUE;
			}
		}
		break;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CLoginDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	// (a.walling 2008-04-16 17:43) - PLID 29691 - If they animate window function is ready and we are not remote, animate
	if (bShow && (m_pfnAnimateWindow != NULL) && (0 == GetSystemMetrics(SM_REMOTESESSION)) ) {
		if (0 == m_pfnAnimateWindow(GetSafeHwnd(), 100, 0x00080000 /*AW_BLEND*/ | 0x00020000 /*AW_ACTIVATE*/)) {
			// 0 is a failure state, so call the default handler
			CDialog::OnShowWindow(bShow, nStatus);
		}
	} else {
		CDialog::OnShowWindow(bShow, nStatus);
	}
}

	// (a.walling 2010-07-15 14:18) - PLID 38608 - Handle WM_TIMECHANGE
LRESULT CLoginDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_TIMECHANGE) {
		// (a.walling 2010-07-12 13:50) - PLID 39608 - Our offset from the server time is no longer reliable
		InvalidateRemoteServerTime();		
	}

	return CNxDialog::WindowProc(message, wParam, lParam);
}


// (d.thompson 2012-02-17) - PLID 48194 - Use shared code instead of repeating this chunk of code.  Returns the control
//	we should give focus to.
UINT CLoginDlg::ChooseDefaultUser()
{
	UINT nGiveFocusTo = 0;

	CString strLoginName = GetCurrentWinLoginName();

	//changed 4/20/01 - if last logged in user doesn't exist, then try the windows username
	// (d.thompson 2012-02-17) - PLID 48194 - This has always saved the value per-path.  Now that we have concurrent 
	//	TS licensing, the path is going to be the same for everyone on that TS, so everyone shares this setting, and
	//	it just bounces around a lot.  We should instead save it per-Windows user per-Computer
	//Default to whatever value got saved last in the "LastUser" (this may or may not be what they want, depending
	//	how settings have changed, but, much like Obi-wan before it, it's our only hope).
	// (d.thompson 2012-05-29) - PLID 48194 - On good advice, I set the default value check to a (hopefully) absurd
	//	string value for a non-user.  Only if we get that do we do a data lookup for the old property.
	CString strUserToChoose = GetRemotePropertyText("LastUserPerWinUser", "415c4f30-a997-11e1-afa6-0800200c9a66", 0, GetAdvancedUsername(), true);
	if(strUserToChoose == "415c4f30-a997-11e1-afa6-0800200c9a66") {
		// (d.thompson 2012-05-29) - PLID 48194 - This was the old property
		strUserToChoose = GetPropertyText("LastUser");
	}
	if (ChooseUser(strUserToChoose) || ChooseUser(strLoginName)) {
		nGiveFocusTo = IDC_PASSWORD_BOX;
	} else {
		nGiveFocusTo = cNoFocus;
	}

	return nGiveFocusTo;
}

// (d.thompson 2012-02-17) - PLID 48194 - Pulled out of several copy/pastes and made its own function
CString CLoginDlg::GetCurrentWinLoginName()
{
	// Get the name of the user currently logged into windows
	CString strLoginName;
	DWORD nLenStr = 2;
	char *pStr = strLoginName.GetBuffer(nLenStr);
	if (!GetUserName(pStr, &nLenStr)) {
		strLoginName.ReleaseBuffer();
		pStr = strLoginName.GetBuffer(nLenStr);
		if (!GetUserName(pStr, &nLenStr)) {
			pStr[0] = NULL;
		}
	}
	strLoginName.ReleaseBuffer();
	strLoginName.MakeUpper();

	return strLoginName;
}