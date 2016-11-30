#include "stdafx.h"
#include "PracticeLicenseSocket.h"


// (z.manning 2015-05-19 16:48) - PLID 65971 - Created. Has Practice-specific license functionality as
// the core license code was moved to NxNetworkLib



CPracticeLicenseSocket::CPracticeLicenseSocket(CPracticeLicense* pParent)
	: CLicenseSocket(pParent)
{
}

#define IDT_DISCONNECTED	1
LRESULT CALLBACK CPracticeLicenseSocket::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		KillTimer(hWnd, IDT_DISCONNECTED);
		TryToReconnect();
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

//DRT 6/15/2006 - PLID 21038 - Copied from CLicense, renamed, and slightly modified
bool CPracticeLicenseSocket::TryToReconnect()
{
	//TES 9/27/2006 - If we've given up on reconnecting, then repost the closing message (sometimes it gets lost when the main
	// window is transferred from the login dialog to CMainFrame), and then return.
	CMainFrame *pMain = GetMainFrame();
	if (m_bClosedPractice) {
		if (pMain) {
			pMain->PostMessage(NXM_FORCIBLY_CLOSE);
		}
		else {
			AfxGetMainWnd()->PostMessage(WM_CLOSE);
		}
		return false;
	}

	//reset our handle
	if (m_hClient) {
		NxSocketUtils::Disconnect(m_hClient);
	}
	m_hClient = NULL;

	//OK, let's try to reconnect.  Here's the plan: loop through trying to re-connect until we succeed or we have been 
	//trying for a certain amount of time (let's use the ConnectionTimeout registry key).  If we succeed, reset all our
	//cached license values, and try to re-obtain a license for our current workstation.  If that succeeds, then just move on.
	//If it fails, close Practice.  If we timeout, ask the user if they want to keep trying.  If they do, then repeat, 
	//if not, close Practice.

	//OK, actually, if a report is running, all this will cause problems.  So wait for it to finish.
	if (pMain) {
		if (pMain->m_bIsReportRunning) {
			EnsureReportsTimerHwnd();
			::SetTimer(m_hWndReportsTimer, IDT_DISCONNECTED, 100, NULL);
			return true;
		}
	}

	//First, give the user some feedback that there's a problem connecting.
	CShowConnectingFeedbackDlg *pDlg = NULL;

	while (!m_hClient && !m_bClosedPractice) {
		if (!pDlg) {
			pDlg = new CShowConnectingFeedbackDlg;
			pDlg->SetWaitMessage("Please wait while Practice connects to your license server...");
		}

		DWORD dwTimeoutMs = GetConnectionTimeout() * 1000;
		DWORD dwStartingTickCount = GetTickCount();
		while (!m_hClient && GetTickCount() < dwStartingTickCount + dwTimeoutMs) {
			try {
				m_hClient = NxSocketUtils::Connect(NULL, NxRegUtils::ReadString(NxRegUtils::GetRegistryBase(m_pParentLicense->m_strSubkey) + "NxServerIP"), STANDARD_NXLICENSESERVER_PORT);
			}
			catch (CNxException *e) {
				e->Delete();
			}
			catch (...){}
			if (!NxSocketUtils::FromHandle(m_hClient)) {
				m_hClient = NULL;
			}
		}

		if (m_hClient) {
			//Great!  We reconnected.
			//Re-initialize our socket.

			//Notify our parent license that we lost our connection, but it has since been regained.  This will
			//	perform the act of re-initializing our given working path in the license.  This function
			//	will return whether or not the program should continue.
			// (j.armen 2011-10-26 16:33) - PLID 45796 - Register will return the result of LicenseConnectionLostAndRegained();
			bool bContinueProgram = Register();

			//cleanup the connection feedback dialog
			//(e.lally 2008-07-28) - Only delete pDlg if it is not null
			if (pDlg){
				delete pDlg;
			}
			pDlg = NULL;

			//Handle the act of shutting down the program
			// (j.armen 2011-11-02 11:13) - PLID 45796 - The license won't be valid unless we got a license for the folder
			// (j.armen 2014-01-31 16:13) - PLID 60603 - This should have checked to make sure the license is NOT valid before prompting to force exit practice
			if (!bContinueProgram || !g_pLicense->IsValid()) {
				CString strOutput;
				strOutput.Format("Practice connected to the license server, but was unable to allocate a license for the currently logged in location.  Practice will now close.\n"
					"Would you like to e-mail NexTech Technical Support?");
				UINT response = IDNO;
				response = m_pParentLicense->OnMessageBox(strOutput, MB_YESNO);
				if (IDYES == response) {
					SendErrorEmail(GetMainFrame(), "Could not re-obtain license after disconnect");
				}
				m_bClosedPractice = true;
				if (pMain) {
					pMain->PostMessage(NXM_FORCIBLY_CLOSE);
				}
				else {
					AfxGetMainWnd()->PostMessage(WM_CLOSE);
				}
			}
		}
		else {
			//OK, we've timed out.  Let's see if they want to keep trying.
			//(e.lally 2008-07-28) - Only delete pDlg if it is not null
			if (pDlg){
				delete pDlg;
			}
			pDlg = NULL;

			CString strOutput;
			strOutput.Format("Practice has lost its connection to your license server.  Would you like to continue trying to reconnect?\n"
				"If No, Practice will immediately close.");
			UINT response = IDNO;
			// (j.armen 2014-01-31 16:13) - PLID 60603 - This should have assigned to handle the user's response.  We don't always want to exit.
			// (z.manning 2015-06-10 17:00) - PLID 66361 - Let the license handle the message
			response = m_pParentLicense->OnMessageBox(strOutput, MB_YESNO);
			if (IDYES != response) {
				//OK, we're definitely closing Practice.  Give them a chance to fire off an error report.
				strOutput.Format("Would you like to e-mail Nextech Technical Support?");
				UINT response2 = IDNO;
				g_pLicense->OnMessageBox(strOutput, MB_YESNO);
				if (IDYES == response2) {
					SendErrorEmail(GetMainFrame(), "Disconnected from NxLicenseServer");
				}
				m_bClosedPractice = true;
				if (pMain) {
					pMain->PostMessage(NXM_FORCIBLY_CLOSE);
				}
				else {
					AfxGetMainWnd()->PostMessage(WM_CLOSE);
				}
			}
		}
	}
	return !m_bClosedPractice;
}
