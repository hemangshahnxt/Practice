// ShowNxErrorFeedbackDlg.cpp: implementation of the CShowNxErrorFeedbackDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "nxerrordialog.h"
#include "ShowNxErrorFeedbackDlg.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct CShowConnectingFeedbackDlgResources
{
	HANDLE m_hEventWaitForTerm;
	CWinThread *m_pThread;
};

extern UINT AFX_CDECL Thread_ReleaseConnectingFeedbackDlgResources(LPVOID lpParam);

UINT AFX_CDECL Thread_ShowNxErrorFeedbackDlg(LPVOID lpParam)
{
	CShowNxErrorFeedbackDlg* pSource = (CShowNxErrorFeedbackDlg*)lpParam;
	HANDLE hEventWaitForTerm = pSource->m_hEventWaitForTerm;

	// Create and display the NxError dialog
	CNxErrorDialog dlg;
	// Added Continue; email is now retry.
	dlg.Create(CWnd::GetDesktopWindow(), pSource->m_strMessage, pSource->m_strTitle, pSource->m_level,
		"&Continue", "&E-mail NexTech", "E&xit Practice", "", "");
	dlg.ShowWindow(SW_SHOW);
	dlg.CenterWindow();
	dlg.UpdateWindow();
	dlg.BringWindowToTop();

	// Pump the message queue, waiting for either a quit message, or the event to be set
	while (true) {
		DWORD dwResult = MsgWaitForMultipleObjectsEx(1, &hEventWaitForTerm, INFINITE, QS_ALLEVENTS, 0);
		if (dwResult == WAIT_OBJECT_0 + 1) {
			// There's a message in the queue, pump until there are none or the event is set
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message != WM_QUIT) {
					if (msg.message == NXM_NXERRDLG_IDOK) {
						// (a.walling 2010-08-04 16:29) - PLID 38964 - This is now 'continue'
						SetEvent(pSource->m_hEventUserContinue);
						CWnd* pWnd = dlg.GetDlgItem(IDOK);
						if (pWnd) {
							pWnd->EnableWindow(FALSE);
							pWnd->SetWindowText("Waiting...");
						}
					} else if (msg.message == NXM_NXERRDLG_IDRETRY) {
						// Send an e-mail and close the program
						SendErrorToNexTech(&dlg, pSource->m_strTitle, "Critical failure", pSource->m_strMessage);
					} else if (msg.message == NXM_NXERRDLG_IDCANCEL) {
						// Close the program
						SetEvent(pSource->m_hEventUserAbort);
						if(dlg.GetSafeHwnd())
							dlg.DestroyWindow();
						return 1;
					}

					// Just pump the message the normal way
					if (!TranslateMessage(&msg)) {
						DispatchMessage(&msg);
					}
				} else {
					// Got the quit message, so quit
					if(dlg.GetSafeHwnd())
						dlg.DestroyWindow();
					return 1;
				}

				// We're pumping messages, but still let's just check to see if the event was set
				if (WaitForSingleObject(hEventWaitForTerm, 0) == WAIT_OBJECT_0) {
					// Event was set so return					
					if(dlg.GetSafeHwnd())
						dlg.DestroyWindow();
					return 2;
				}
			}
		} 
		else {
			// Either the event was set or the wait failed in some unexpected way
			ASSERT(dwResult == WAIT_OBJECT_0); // I don't know how it could be a WAIT_TIMEOUT, WAIT_IO_COMPLETION, or WAIT_ABANDONED_0+
			if(dlg.GetSafeHwnd())
				dlg.DestroyWindow();
			return 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShowNxErrorFeedbackDlg::CShowNxErrorFeedbackDlg(const CString &message, const CString &title, ErrorLevel level) :
m_strMessage(message),
m_strTitle(title),
m_level(level)
{
	// Create the event
	m_hEventWaitForTerm = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hEventUserAbort = CreateEvent(NULL, TRUE, FALSE, NULL);
	// (a.walling 2010-08-04 16:28) - PLID 38964
	m_hEventUserContinue = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Spawn the thread (make sure it isn't set to auto-delete)
	m_pThread = AfxBeginThread(Thread_ShowNxErrorFeedbackDlg, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED, NULL);
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();
}

CShowNxErrorFeedbackDlg::~CShowNxErrorFeedbackDlg()
{
	// Set the event, which tells the thread it's allowed to close
	SetEvent(m_hEventWaitForTerm);
	m_pThread->PostThreadMessage(WM_QUIT,0,0);

	// This seems weird but it's really not.  See we could just sit here and wait for the m_pThread to finish 
	// but we don't really need to.  But if we ever want to call CloseHandle on our event handle, we need to 
	// wait until the m_pThread returns.  So instead of waiting for it here, we spawn a different thread (a 
	// thread that MFC will auto-delete for us) that will take responsibility for destroying our member 
	// variables for us.  That way it can run in it's own time without stopping us from returning.  But in 
	// the end everything gets properly destroyed in as timely a manner as possible. (notice the secondary 
	// thread also deallocates the CShowConnectingFeedbackDlgResources that we create right here so we don't
	// have to call delete on it ourselfs)
	CShowConnectingFeedbackDlgResources *pParam = new CShowConnectingFeedbackDlgResources;
	pParam->m_hEventWaitForTerm = m_hEventWaitForTerm;
	pParam->m_pThread = m_pThread;
	AfxBeginThread(Thread_ReleaseConnectingFeedbackDlgResources, (LPVOID)pParam); // note this is autodeleted
	// Tempting though it is, do not call delete on the pParam we just created, the thread does that for us

	// (a.walling 2010-08-04 16:30) - PLID 38964
	CloseHandle(m_hEventUserContinue);

	// We've passed our parameters into the "release resources" thread, so we're no longer responsible for them
	CloseHandle(m_hEventUserAbort);
	m_hEventWaitForTerm = NULL;
	m_pThread = NULL;

}

BOOL CShowNxErrorFeedbackDlg::UserAborted()
{
	return (WAIT_OBJECT_0 == WaitForSingleObject(m_hEventUserAbort, 0)) ? TRUE : FALSE;
}

// (a.walling 2010-08-04 16:37) - PLID 38964 - This blocks until one of our events are set. 
void CShowNxErrorFeedbackDlg::WaitForUser()
{
	HANDLE arHandles[2] = {m_hEventUserContinue, m_hEventUserAbort};
	WaitForMultipleObjects(2, arHandles, FALSE, INFINITE);
}