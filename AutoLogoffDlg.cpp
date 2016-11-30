// AutoLogoffDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "AutoLogoffDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CAutoLogoffDlg dialog


CAutoLogoffDlg::CAutoLogoffDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAutoLogoffDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAutoLogoffDlg)
	m_bIgnoreExtensions = FALSE;
	m_nSeconds = 30;
	//}}AFX_DATA_INIT
}


void CAutoLogoffDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoLogoffDlg)
	DDX_Control(pDX, IDC_BTN_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_STATIC_TIMELEFT, m_nxstaticTimeleft);
	DDX_Control(pDX, IDC_AUTOLOGOFF_OPTIONS, m_nxstaticAutologoffOptions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoLogoffDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAutoLogoffDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_DONTCLOSE, OnBtnDontclose)
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoLogoffDlg message handlers

// (z.manning, 05/13/2008) - PLID 29902 - Added OnInitDialog
BOOL CAutoLogoffDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

	}NxCatchAll("CAutoLogoffDlg::OnInitDialog");

	return TRUE;
}

void CAutoLogoffDlg::OnTimer(UINT nIDEvent) 
{
	//TES 5/12/2009 - PLID 34237 - I noticed this function didn't have error handling, so I added it.
	try {
		CString str;

		DWORD dwTick = GetTickCount();
		// (a.walling 2007-06-21 11:38) - PLID 26443 - I've gotten this into a state (and witnessed it initially)
		// where m_dwExpiration > GetTickCount() / 1000, then when we set nNewInterval it was not (GetTickCount() had
		// increased by 15ms due to clock resolution). So the nNewInterval then was set to 4,294,967,294 ms, then
		// multiplied by 1000, leaving us with a new interval of 4,294,965,296 ms.
		// Additionally, I am removing everything that is dividing then mutliplying by 1000. This just loses resolution
		// and can introduce bugs during the rollover.

		long nUnsavedEMRs = 0;

		switch (nIDEvent)
		{
			case IDT_AUTOLOGOFF_APPEAR_TIMER:
				KillTimer(IDT_AUTOLOGOFF_APPEAR_TIMER);
				if (m_dwExpiration > dwTick)
				{
					unsigned long nNewInterval = m_dwExpiration - dwTick; // subtraction is always safe, even if rollover occurred
					if (nNewInterval > 0)
					{
						SetTimer(IDT_AUTOLOGOFF_APPEAR_TIMER, nNewInterval, NULL);
						break;
					}
				}
				m_bIgnoreExtensions = TRUE;
				str.Format("NexTech Practice will automatically close in %d second(s) due to inactivity.", m_nSeconds);

				// (a.walling 2007-08-27 09:11) - PLID 26195 - Prevent close if unsaved EMRs are around.
				
				nUnsavedEMRs = GetMainFrame()->GetUnsavedEMRCount();

				if (nUnsavedEMRs > 0) {
					SetDlgItemText(IDC_AUTOLOGOFF_OPTIONS, FormatString("Practice cannot be closed due to %li unsaved EMR%s.", nUnsavedEMRs, nUnsavedEMRs > 1 ? "s" : ""));
					GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
				} else {
					SetDlgItemText(IDC_AUTOLOGOFF_OPTIONS, "Please select one of the following options to close this window");
					GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(TRUE);
				}

				SetDlgItemText(IDC_STATIC_TIMELEFT, str);
				ShowWindow(SW_SHOW);
				KillTimer(IDT_AUTOLOGOFF_COUNTDOWN_TIMER);
				SetTimer(IDT_AUTOLOGOFF_COUNTDOWN_TIMER, 1000, NULL);
				break;
			case IDT_AUTOLOGOFF_COUNTDOWN_TIMER:
				// (a.walling 2007-06-21 11:55) - PLID 26443 - Need to kill the timers here, otherwise any message box
				// that pops up will cause this message to be continually processed due to a modal dialog idle loop,
				// and eventually Practice will crash.

				nUnsavedEMRs = GetMainFrame()->GetUnsavedEMRCount();

				KillTimer(IDT_AUTOLOGOFF_APPEAR_TIMER);
				if (--m_nSeconds <= 0) {
					KillTimer(IDT_AUTOLOGOFF_COUNTDOWN_TIMER);

					// (a.walling 2009-06-08 10:24) - PLID 34520 - Ensure no PHI is on screen
					MinimizeAll();
					// (a.walling 2007-08-27 09:11) - PLID 26195 - Prevent close if unsaved EMRs are around.
					SetDlgItemText(IDC_STATIC_TIMELEFT, "");
					if (nUnsavedEMRs > 0) {
						SetDlgItemText(IDC_AUTOLOGOFF_OPTIONS, FormatString("Practice cannot be closed due to %li unsaved EMR%s.", nUnsavedEMRs, nUnsavedEMRs > 1 ? "s" : ""));
						GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
						// set the timer again so we can refresh the close button status
						SetTimer(IDT_AUTOLOGOFF_COUNTDOWN_TIMER, 1000, NULL);

						if (m_nSeconds == 0) {
							AuditEvent(-1, GetCurrentUserName(), BeginNewAuditEvent(), aeiInactivityTimeoutFired, -1, "", CString("Session minimized for ") + GetCurrentUserName(), aepMedium);
						}
					} else {
						SetDlgItemText(IDC_AUTOLOGOFF_OPTIONS, "Please select one of the following options to close this window");
						GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(TRUE);
						//TES 5/12/2009 - PLID 34237 - Audit
						AuditEvent(-1, GetCurrentUserName(), BeginNewAuditEvent(), aeiInactivityTimeoutFired, -1, "", CString("Session terminated for ") + GetCurrentUserName(), aepMedium);
						ClosePractice();
					}
				} else {
					str.Format("NexTech Practice will automatically close in %d second(s) due to inactivity.", m_nSeconds);

					if (nUnsavedEMRs > 0) {
						SetDlgItemText(IDC_AUTOLOGOFF_OPTIONS, FormatString("Practice cannot be closed due to %li unsaved EMR%s.", nUnsavedEMRs, nUnsavedEMRs > 1 ? "s" : ""));
						GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
					} else {
						SetDlgItemText(IDC_AUTOLOGOFF_OPTIONS, "Please select one of the following options to close this window");
						GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(TRUE);
					}

					SetDlgItemText(IDC_STATIC_TIMELEFT, str);		
				}

				// (a.walling 2007-09-13 17:50) - PLID 26195
				if (m_nSeconds < 0)
					m_nSeconds = -1; // it is possible, if they decide to wait a couple billion seconds, that m_nSeconds could become positive again.

				break;
		}
	}NxCatchAll("Error in CAutoLogoffDlg::OnTimer()");

	CNxDialog::OnTimer(nIDEvent);
}

void CAutoLogoffDlg::ResetInactivityTimer()
{
	if (IsWindow(GetSafeHwnd()))
	{
		KillTimer(IDT_AUTOLOGOFF_APPEAR_TIMER);
		KillTimer(IDT_AUTOLOGOFF_COUNTDOWN_TIMER);
		SetDlgItemText(IDC_STATIC_TIMELEFT, "NexTech Practice has detected inactivity for an extensive period of time");
	}

	if (GetInactivityTimeout() == -1)
		return;

	m_bIgnoreExtensions = FALSE;
	m_nSeconds = 30;
	// (a.walling 2007-06-25 11:14) - PLID 26443 - The previous calculation could be incorrect 
	// when gettickcount rolls over from 0xFFFF,FFFF to 0x0.
	// (a.walling 2009-06-08 09:44) - PLID 34520 - The countdown must be included in the timeout.

	m_dwExpiration = GetTickCount() + GetAdjustedTimeoutMs();
	if (IsWindow(GetSafeHwnd()))
	{
		SetTimer(IDT_AUTOLOGOFF_APPEAR_TIMER, GetAdjustedTimeoutMs(), NULL);
	}
}

void CAutoLogoffDlg::ExtendInactivityTimer()
{
	// (a.walling 2007-06-25 11:14) - PLID 26443 - The previous calculation could be incorrect 
	// when gettickcount rolls over from 0xFFFF,FFFF to 0x0.
	
	// (a.walling 2009-06-08 09:44) - PLID 34520 - The countdown must be included in the timeout.
	if (!m_bIgnoreExtensions)
		m_dwExpiration = GetTickCount() + GetAdjustedTimeoutMs();
}

void CAutoLogoffDlg::OnBtnDontclose() 
{
	ShowWindow(SW_HIDE);
	ResetInactivityTimer();
}

void CAutoLogoffDlg::OnBtnClose() 
{
	ClosePractice();
}

void CAutoLogoffDlg::OnOK()
{

}

void CAutoLogoffDlg::OnCancel()
{

}

void CAutoLogoffDlg::ClosePractice()
{
	// (a.walling 2007-08-24 17:59) - PLID 26195 - Don't close if there are any unsaved EMRs
	if (GetMainFrame()->GetUnsavedEMRCount() <= 0) {
		GetMainFrame()->PostMessage(NXM_FORCIBLY_CLOSE);
	}
}


// (a.walling 2009-06-08 09:44) - PLID 34520 - The countdown must be included in the timeout.
DWORD CAutoLogoffDlg::GetAdjustedTimeoutMs()
{
	return 
		(
			(GetInactivityTimeout() * 60) // total inactivity timeout
			-
			(m_nSeconds) // seconds the countdown is display
		) * 1000; // in milliseconds
}

// (a.walling 2009-06-08 10:24) - PLID 34520 - Ensure no PHI is on screen
void CAutoLogoffDlg::MinimizeAll()
{
	MinimizeAllInfo mai;
	mai.hwndExclude = GetSafeHwnd();
	mai.dwProcessID = GetCurrentProcessId();
	mai.dwThreadID = GetCurrentThreadId();

	EnumWindows(MinimizeAllProc, (LPARAM)&mai);
}

BOOL CALLBACK CAutoLogoffDlg::MinimizeAllProc(HWND hwnd, LPARAM lParam)
{
	MinimizeAllInfo* pMai = (MinimizeAllInfo*)lParam;

	if (hwnd == pMai->hwndExclude)
		return TRUE;

	DWORD dwProcessID = 0;
	DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);

	if (dwProcessID == pMai->dwProcessID) {
		if (::IsWindowVisible(hwnd)) {
			if (dwThreadID == pMai->dwThreadID) {
				::ShowWindow(hwnd, SW_MINIMIZE);
			} else {
				::ShowWindow(hwnd, SW_FORCEMINIMIZE);
			}
		}
	}

	return TRUE;
}