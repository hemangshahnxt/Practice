// SizeableTextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SizeableTextDlg.h"
#include "pracProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSizeableTextDlg dialog

CSizeableTextDlg::CSizeableTextDlg(CWnd* pParentWnd /*= NULL*/)
	: CNxDialog(IDD, pParentWnd)
{
	m_nStyle = 0;
	m_strCurUser.Empty();
	m_strTextOrPath.Empty();
	m_strName.Empty();
	m_strTitle.Empty();
	m_dtOverrideCurrentTime.SetStatus(COleDateTime::invalid);
	m_dtNextTime.SetStatus(COleDateTime::invalid);

	m_szTextEdit.cx = m_szTextEdit.cy = 0;
	m_ptDontShowCheck.x = m_ptDontShowCheck.y = 0;
	m_szOKBtn.cx = m_szOKBtn.cy = 0;
	m_szPrintBtn.cx = m_szPrintBtn.cy = 0;
	m_szYesBtn.cx = m_szYesBtn.cy = 0;
	m_szNoBtn.cx = m_szNoBtn.cy = 0;

	m_bDontShowChecked = FALSE;
}

CSizeableTextDlg::CSizeableTextDlg(UINT nIDTemplate, CWnd* pParentWnd /*= NULL*/)
	: CNxDialog(nIDTemplate, pParentWnd)
{
	//{{AFX_DATA_INIT(CSizeableTextDlg)
	//}}AFX_DATA_INIT

	m_nStyle = 0;
	m_strCurUser.Empty();
	m_strTextOrPath.Empty();
	m_strName.Empty();
	m_strTitle.Empty();
	m_dtOverrideCurrentTime.SetStatus(COleDateTime::invalid);
	m_dtNextTime.SetStatus(COleDateTime::invalid);

	m_szTextEdit.cx = m_szTextEdit.cy = 0;
	m_ptDontShowCheck.x = m_ptDontShowCheck.y = 0;
	m_szOKBtn.cx = m_szOKBtn.cy = 0;
	m_szPrintBtn.cx = m_szPrintBtn.cy = 0;
	m_szYesBtn.cx = m_szYesBtn.cy = 0;
	m_szNoBtn.cx = m_szNoBtn.cy = 0;

	m_bDontShowChecked = FALSE;
}

void CSizeableTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSizeableTextDlg)
	DDX_Control(pDX, IDC_TEXT_EDIT, m_nxeditTextEdit);
	DDX_Control(pDX, IDC_PRINT_BTN, m_btnPrint);
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSizeableTextDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSizeableTextDlg)
	ON_BN_CLICKED(IDC_PRINT_BTN, OnPrintBtn)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDNO, OnNo)
	ON_BN_CLICKED(IDYES, OnYes)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSizeableTextDlg message handlers

BOOL CSizeableTextDlg::OnInitDialog()
{
	// Call the base class
	CNxDialog::OnInitDialog();

	m_btnPrint.AutoSet(NXB_PRINT);
	m_btnOk.AutoSet(NXB_OK);
	//DRT 6/9/2008 - PLID 30331 - Added cancel button
	m_btnCancel.AutoSet(NXB_CANCEL);

	// Useful for control position calculation
	CRect rcDialogClient;
	GetClientRect(&rcDialogClient);

	CWnd *pWnd = NULL; // General purpose variable

	//  Decide how we're gonna go about printing
	if ((m_nStyle & (tsFromFile|bsPrintOnly|bsOKPrint|bsYesNoPrint)) != 0) {
		pWnd = GetDlgItem(IDC_PRINT_BTN);
		if (pWnd) {
			pWnd->SetWindowText("Note&pad");
			pWnd->ShowWindow(SW_SHOW);
		}
	}

	// Load the text
	CString strText;
	if ((m_nStyle & (tsFromFile|bsPrintOnly|bsOKPrint|bsYesNoPrint)) != 0) {
		// Pull from file, interpret m_strTextOrPath as a path
		try {
			//PLID 18069: extended it to read 64K instead of 16K
			const int LEN_64_KB = 65536;
			// Open the file
			CFile changes(m_strTextOrPath, CFile::modeRead|CFile::shareDenyNone);
			// Read up to 64 KB
			long nActualLen = changes.Read(strText.GetBuffer(LEN_64_KB), LEN_64_KB);
			// Release the string
			strText.ReleaseBuffer(nActualLen);

		} NxCatchAll("Could not read from file");
	} else {
		strText = m_strTextOrPath;
	}

	// Init the textbox
	pWnd = GetDlgItem(IDC_TEXT_EDIT);
	if (pWnd) {
		// Remember where the top of the textbox is
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_szTextEdit.cx = rcDialogClient.Width() - rc.Width();
		m_szTextEdit.cy = rcDialogClient.Height() - rc.Height();

		// Set the text
		pWnd->SetWindowText(strText);

		// Set the visual elements
		if ((m_nStyle & msFixedFontText) == msFixedFontText) {
			extern CPracticeApp theApp;
			pWnd->SetFont(&theApp.m_fixedFont, FALSE);
		}

		if ((m_nStyle & msMsgBoxText) == msMsgBoxText) {
			// TODO: make the dialog autosize and think about making the edit box a static control instead
			pWnd->ModifyStyle(WS_VSCROLL, ES_AUTOVSCROLL);
			pWnd->ModifyStyleEx(WS_EX_STATICEDGE, 0);
		}
	}

	// Init the don't show checkbox
	pWnd = GetDlgItem(IDC_DONTSHOW_CHECK);
	if (pWnd) {
		CRect rc;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_ptDontShowCheck.x = rc.Width();
		m_ptDontShowCheck.y = rcDialogClient.bottom - rc.top;
	}

	// Init the ok button
	if ((m_nStyle & (bsOKOnly|bsOKPrint)) != 0)  {
		pWnd = GetDlgItem(IDOK);
		if (pWnd) {
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_szOKBtn.cx = rc.Width();
			m_szOKBtn.cy = rcDialogClient.bottom - rc.top;
		}
	}
	else {
		GetDlgItem(IDOK)->ShowWindow(FALSE);
	}

	// Init the print button
	if ((m_nStyle & (bsPrintOnly|bsOKPrint|bsYesNoPrint)) != 0)  {
		pWnd = GetDlgItem(IDC_PRINT_BTN);
		if (pWnd) {
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_szPrintBtn.cx = rcDialogClient.right - rc.left;
			m_szPrintBtn.cy = rcDialogClient.bottom - rc.top;
		}
	}
	
	//Init the Yes/No button
	//	DRT 6/9/2008 - PLID 30331 - Added the Cancel option
	if ((m_nStyle & (bsYesNoOnly|bsYesNoPrint|bsYesNoCancel)) != 0) {
		pWnd = GetDlgItem(IDYES);
		if (pWnd) {
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_szYesBtn.cx = rc.Width();
			m_szYesBtn.cy = rcDialogClient.bottom - rc.top;
		}


		pWnd = GetDlgItem(IDNO);
		if (pWnd) {
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_szNoBtn.cx = rc.Width();
			m_szNoBtn.cy = rcDialogClient.bottom - rc.top;
		}

	}
	else {
		GetDlgItem(IDYES)->ShowWindow(FALSE);
		GetDlgItem(IDNO)->ShowWindow(FALSE);
	}

	//DRT 6/9/2008 - PLID 30331 - Init the Cancel button
	if((m_nStyle & (bsYesNoCancel)) != 0) {
		pWnd = GetDlgItem(IDCANCEL);
		if(pWnd) {
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_szCancelBtn.cx = rc.Width();
			m_szCancelBtn.cy = rcDialogClient.bottom - rc.top;
		}
	}
	else {
		//Hide the cancel button
		GetDlgItem(IDCANCEL)->ShowWindow(FALSE);
	}

	// Set the dialog title
	if (!m_strTitle.IsEmpty()) {
		SetWindowText(m_strTitle);
	} else {
		SetWindowText(AfxGetApp()->m_pszAppName);
	}

	return TRUE;
}

// if a > b+m, return 1; if a < b-m, return -1; else return 0
#define COMPARE_DATETIME_INEXACT(a, b, marginseconds) \
	(((a) > ((b) + COleDateTimeSpan(0, 0, 0, marginseconds))) ? 1 : \
	 ((a) < ((b) - COleDateTimeSpan(0, 0, 0, marginseconds))) ? -1 : 0)

int CSizeableTextDlg::DoModal() 
{	
	if (Init()) {
		if (UserNeedsRemind()) {
			int result = CDialog::DoModal();
			return result;
		} else {
			return IDOK;
		}
	} else {
		return IDOK;
	}
}

void CSizeableTextDlg::OnPrintBtn() 
{
	if ((m_nStyle & tsFromFile) == tsFromFile) {
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		int nResult = (int)ShellExecute(GetSafeHwnd(), NULL, 
			"notepad.exe", ("\"" + m_strTextOrPath + "\""), NULL, SW_SHOW);
		if (nResult < 32) {
			CString str;
			str.Format("Error %li while trying to load '%s' in notepad.", nResult, m_strTextOrPath);
			HandleException(NULL, str);
		}
	}
}

BOOL CSizeableTextDlg::UserNeedsRemind()
{
	if (!(m_nStyle & drsIgnoreDontRemind)) {
		// We need to see if the user should be given this dialog, whatever it is
		ASSERT(!m_strCurUser.IsEmpty()); // We use ConfigRT so a user has to be logged in
		ASSERT(!m_strName.IsEmpty()); // If we don't have an m_strName, what are we supposed to look up in ConfigRT?
		if (!m_strCurUser.IsEmpty() && !m_strName.IsEmpty()) {
			// See what kind of reminding we're doing: by date, or just boolean
			if ((m_nStyle & (drsDontRemindUntil|drsDontRemindUntilFileChange)) != 0) {
				
				// Either drsDontRemindUntil or drsDontRemindUntilFileChange
				if (((m_nStyle & tsFromFile) == tsFromFile) && ((m_nStyle & drsDontRemindUntilFileChange) == drsDontRemindUntilFileChange)) {
					// Don't remind until the file referenced by m_strTextOrPath has been modified

					// We're just getting the file's  modified time so the caller doesn't have to
					COleDateTime dtFileTime;
					bool bGotFileTime = GetFileSystemTime(m_strTextOrPath, dtFileTime);
					if (bGotFileTime) {
						// Got the file time so set both the "next time" and the "current time" to the file time
						m_dtNextTime = dtFileTime;
						m_dtOverrideCurrentTime = dtFileTime;
						// And then let the standard "don't remind until" functionality do its thing
					} else {
						// Couldn't get the file time, so fail
						return FALSE;
					}
				} else {
					// Don't remind until m_dtNextTime
					
					// If this assertion fails, it might be because the caller set style 
					// drsDontRemindUntilFileChange but failed to set style tsFromFile
					ASSERT((m_nStyle & drsDontRemindUntil) == drsDontRemindUntil);
					
					// Make sure we have a valid current time
					if (m_dtOverrideCurrentTime.GetStatus() != COleDateTime::valid) {
						m_dtOverrideCurrentTime = COleDateTime::GetCurrentTime();
					}
				}

				// If "current time" is >= m_dtNextTime then we really want to show it
				if (COMPARE_DATETIME_INEXACT(m_dtOverrideCurrentTime, m_dtNextTime, 5) >= 0) {
					// But find out when we last showed it
					COleDateTime dtLast = GetRemotePropertyDateTime("timestamp " + m_strName, 
						&COleDateTime(1901, 1, 1, 1, 1, 1), 0, m_strCurUser, false);
					// If "last time" is >= m_dtNexTime too then we've shown it before and the user decided not to see it again
					if (COMPARE_DATETIME_INEXACT(dtLast, m_dtNextTime, 5) >= 0) {
						// User doesn't want our dialog
						return FALSE;
					} else {
						// Last time was before "next time" which means we have yet to show it, let's show it now
						return TRUE;
					}
				} else {
					// "current time" was not greater than or equal to "next time" so don't show it
					return FALSE;
				}
			} else {
				// Simple boolean
				if (GetRemotePropertyInt("dontshow " + m_strName, 0, 0, m_strCurUser, false)) {
					// We've been told not to show
					return FALSE;
				} else {
					return TRUE;
				}
			}
		} else {
			// We're not doing any "don't show me again" stuff
			CWnd *pWnd = GetDlgItem(IDC_DONTSHOW_CHECK);
			if (pWnd) {
				CheckDlgButton(IDC_DONTSHOW_CHECK, 0);
				pWnd->ShowWindow(SW_HIDE);
			}
			return TRUE;
		}
	} else {
		//TES 8/12/2009 - PLID 35201 - This style (which was never previously used) now means that the caller
		// will be handling the "Don't Show Me Again" functionality.
		return TRUE;
	}
}

BOOL CSizeableTextDlg::DoModeless()
{
	if (Init()) {
		if (UserNeedsRemind()) {
			if (Create(m_lpszTemplateName, m_pParentWnd)) {
				ShowWindow(SW_SHOW);
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			return FALSE;
		}
	} else {
		return FALSE;
	}
}

void CSizeableTextDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	CWnd *pWnd = NULL;

	pWnd = GetDlgItem(IDC_TEXT_EDIT);
	if (pWnd) {
		pWnd->SetWindowPos(NULL, 0, 0, cx - m_szTextEdit.cx, cy - m_szTextEdit.cy, SWP_NOMOVE | SWP_NOZORDER);
		pWnd->Invalidate();
	}
	
	pWnd = GetDlgItem(IDC_DONTSHOW_CHECK);
	if (pWnd) {
		// (b.cardillo 2005-05-02 14:53) - PLID 15477 - Made it so the .x is the WIDTH of the 
		// checkbox, rather than it's original left position, since now we center the check 
		// when the window is sized (we used to just leave it where it was horizontally).
		pWnd->SetWindowPos(NULL, max(cx / 2 - m_ptDontShowCheck.x / 2, 0), cy - m_ptDontShowCheck.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		pWnd->Invalidate();
	}
	
	pWnd = GetDlgItem(IDOK);
	if (pWnd) {
		pWnd->SetWindowPos(NULL, (cx / 2) - (m_szOKBtn.cx / 2), cy - m_szOKBtn.cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		pWnd->Invalidate();
	}
	
	pWnd = GetDlgItem(IDC_PRINT_BTN);
	if (pWnd) {
		pWnd->SetWindowPos(NULL, cx - m_szPrintBtn.cx, cy - m_szPrintBtn.cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		pWnd->Invalidate();
	}

	//DRT 6/9/2008 - PLID 30331 - For the Yes, No, Cancel, depending on options we'll have either 2 or 3 buttons
	long nNumButtons = 2;
	if(m_nStyle & bsYesNoCancel)
		nNumButtons = 3;

	pWnd = GetDlgItem(IDYES);
	if (pWnd) {
		pWnd->SetWindowPos(NULL, cx / nNumButtons - ((m_szYesBtn.cx + m_szNoBtn.cx + 5)/nNumButtons), cy - m_szYesBtn.cy, 0,0, SWP_NOSIZE | SWP_NOZORDER);
		pWnd->Invalidate();
	}

	pWnd = GetDlgItem(IDNO);
	if (pWnd) {
		pWnd->SetWindowPos(NULL, cx / nNumButtons - ((m_szYesBtn.cx + m_szNoBtn.cx + 5)/nNumButtons) + m_szYesBtn.cx + 5, cy - m_szNoBtn.cy, 0,0, SWP_NOSIZE | SWP_NOZORDER);
		pWnd->Invalidate();
	}

	//DRT 6/9/2008 - PLID 30331 - Added the cancel button
	pWnd = GetDlgItem(IDCANCEL);
	if(pWnd) {
		//This sizing looks a little random -- The problem is that the resource file isn't wide enough for 3 buttons, and I did not
		//	want to mess up everywhere else for this little addition.  So we have to start off by bumping it half a button (because it's
		//	halfway under the 'No' button in the resource file).  This would work better if the dialog did "absolute sizing", but instead it
		//	tries to shift things from the resource positioning, which I can't quite change.
		pWnd->SetWindowPos(NULL, (m_szNoBtn.cx / 2) - (5*nNumButtons) + cx / nNumButtons + (m_szYesBtn.cx + m_szNoBtn.cx + m_szCancelBtn.cx + 5)/nNumButtons, cy - m_szCancelBtn.cy, 0,0, SWP_NOSIZE | SWP_NOZORDER);
		pWnd->Invalidate();
	}
}

void CSizeableTextDlg::OnOK()
{
	CWnd *pWnd = GetDlgItem
		(IDC_DONTSHOW_CHECK);
	if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible() && IsDlgButtonChecked(IDC_DONTSHOW_CHECK)) {
		if (m_nStyle & (drsDontRemindUntil|drsDontRemindUntilFileChange)) {
			// A time-related don't show
			SetRemotePropertyDateTime("timestamp " + m_strName, m_dtOverrideCurrentTime, 0, m_strCurUser);
		} else if (m_nStyle & drsIgnoreDontRemind) {
			// Not a no-show at all (this shouldn't be possible, because it would have been set 
			// this way when the dialog was created, so the don't show checkbox would have been 
			// hidden, which means we shouldn't have been able to get into this logic)
			ASSERT(FALSE);
		} else {
			// Simple boolean don't show
			SetRemotePropertyInt("dontshow " + m_strName, true, 0, m_strCurUser);
		}
	}

	CDialog::OnOK();
}

void CSizeableTextDlg::OnCancel() 
{
	// Users tend to think that hitting escape when there's only an OK button is the same as clicking OK
	//DRT 6/9/2008 - PLID 30331 - This code previously checked if the cancel button existed to decide if
	//	we should cancel.  But I've had to add it as a general ability now, even though it's still hidden
	//	most of the time.  Instead of checking for existence, we'll just see if it's in use.
	if(m_nStyle & (bsYesNoCancel)) {
		//There is a cancel button, so use it.
		CDialog::OnCancel();
	}
	else {
		//DRT 6/9/2008 - PLID 30332 - If OK doesn't exist, we cannot call OnOK!  In that case just ignore
		//	the cancel command.
		if(m_nStyle & (bsOKOnly|bsOKPrint)) {
			//OK exists, we can call it.
			OnOK();
		}
		else {
			//OK does not exist, don't try it.
			return;
		}
	}
}

BOOL CSizeableTextDlg::Init()
{
	if (m_strCurUser.IsEmpty()) {
		m_strCurUser = GetCurrentUserName();
	}
	return TRUE;
}

void CSizeableTextDlg::OnNo() 
{
	CWnd *pWnd = GetDlgItem
		(IDC_DONTSHOW_CHECK);
	if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible() && IsDlgButtonChecked(IDC_DONTSHOW_CHECK)) {
		if (m_nStyle & (drsDontRemindUntil|drsDontRemindUntilFileChange)) {
			// A time-related don't show
			SetRemotePropertyDateTime("timestamp " + m_strName, m_dtOverrideCurrentTime, 0, m_strCurUser);
		} else if (m_nStyle & drsIgnoreDontRemind) {
			//TES 8/12/2009 - PLID 35201 - This style (which was never previously used) now means that the caller
			// will be handling the "Don't Show Me Again" functionality, so remember that the box was checked so
			// that we can output it to the caller.
			m_bDontShowChecked = TRUE;
		} else {
			// Simple boolean don't show
			SetRemotePropertyInt("dontshow " + m_strName, true, 0, m_strCurUser);
		}
	}

	CDialog::EndDialog(IDNO);
	
	
}

void CSizeableTextDlg::OnYes() 
{
	CWnd *pWnd = GetDlgItem
		(IDC_DONTSHOW_CHECK);
	if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible() && IsDlgButtonChecked(IDC_DONTSHOW_CHECK)) {
		if (m_nStyle & (drsDontRemindUntil|drsDontRemindUntilFileChange)) {
			// A time-related don't show
			SetRemotePropertyDateTime("timestamp " + m_strName, m_dtOverrideCurrentTime, 0, m_strCurUser);
		} else if (m_nStyle & drsIgnoreDontRemind) {
			//TES 8/12/2009 - PLID 35201 - This style (which was never previously used) now means that the caller
			// will be handling the "Don't Show Me Again" functionality, so remember that the box was checked so
			// that we can output it to the caller.
			m_bDontShowChecked = TRUE;
		} else {
			// Simple boolean don't show
			SetRemotePropertyInt("dontshow " + m_strName, true, 0, m_strCurUser);
		}
	}

	CDialog::EndDialog(IDYES);
	
}

HBRUSH CSizeableTextDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_TEXT_EDIT:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;
	}

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}