// DoesExist.cpp: implementation of the CDoesExist class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "DoesExist.h"
#include "ShowConnectingFeedbackDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UINT DoesFileExist(LPVOID pParam)
{
	BSTR bstr = (BSTR)pParam;
	CString str(bstr);
	SysFreeString(bstr);
	// (c.haag 2006-08-02 12:27) - PLID 21743 - Auto-fail if there is no path
	if (str.IsEmpty()) {
		return false;
	} else {
		return DoesExist(str);
	}
}

CDoesExist::CDoesExist()
{
	m_pThread = NULL;
	SetPath("");
}

CDoesExist::CDoesExist(CString strFilePath, BOOL bTryAccess /* = TRUE */)
{
	m_pThread = NULL;
	SetPath(strFilePath);
	
	// If we want to try to access the file now, do it
	if (bTryAccess)
		TryAccess();	
}

CDoesExist::~CDoesExist()
{
	Stop();
}

void CDoesExist::SetPath(const CString& strPath)
{
	Stop();
	m_strFilePath = strPath;
}

void CDoesExist::TryAccess()
{
	if (GetStatus() == ePending) {
		return;
	} else if (m_strFilePath.IsEmpty()) {
		return;
	}

	// (c.haag 2003-11-04 12:17) - This seems to be much faster than using
	// the AFX thread management code.
	// (c.haag 2006-08-02 12:09) - PLID 21743 - We now use AfxBeginThread; the right way for MFC apps
	//m_hThread = CreateThread(NULL, 0, DoesFileExist, (LPVOID)m_strFilePath.AllocSysString(), 0, &m_dwThreadId);
	m_pThread = AfxBeginThread(DoesFileExist, (LPVOID)m_strFilePath.AllocSysString(), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_pThread->m_bAutoDelete = false;
	m_pThread->ResumeThread();
}

void CDoesExist::Stop()
{
	// Destroy our thread object
	if (m_pThread)
	{
		// (z.manning, 5/5/2006, PLID 20460) - We used to wait 5 seconds here for
		// no real good reason because if we're stopping it, then we must not care
		// about what it's doing anymore.
		//WaitForSingleObject(m_hThread, 5000); // Wait for the thread to terminate

		// (c.haag 2006-08-02 12:12) - PLID 21743 - We now use a CWinThread object
		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so post a quit message to it and let it delete itself
			m_pThread->m_bAutoDelete = TRUE;
			PostThreadMessage(m_pThread->m_nThreadID, WM_QUIT, 0, 0);
		} else {
			// The thread is finished, so just delete it
			delete m_pThread;
		}
		
		// Either we just deleted the thread or it will automatically be deleted later
		m_pThread = NULL;
	}
}

CDoesExist::EStatus CDoesExist::WaitForAccess(const CString& strWaitMessage)
{
	if (!m_pThread) return eNoAction;
	CShowConnectingFeedbackDlg dlgConnecting;
	dlgConnecting.SetWaitMessage(strWaitMessage);
	WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	return GetStatus();
}

CDoesExist::EStatus CDoesExist::GetStatus()
{
	// (c.haag 2006-08-02 12:27) - PLID 21743 - Auto-fail if there is no path
	if (m_strFilePath.IsEmpty())
		return eFailed;
	else if (!m_pThread)
		return eNoAction;

	// Get the exit code
	DWORD dwExitCode = 0;
	::GetExitCodeThread(m_pThread->m_hThread, &dwExitCode);
	// See if the thread is still active
	if (dwExitCode == STILL_ACTIVE)
		return ePending;
	else if (dwExitCode == 0)
		return eFailed;
	return eSuccess;
}