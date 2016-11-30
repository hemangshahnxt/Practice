// NxProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NxProgressDlg.h"
#include "NxStandard.h"
#include "NxStandardRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// (a.walling 2008-09-19 09:53) - PLID 28040 - NxStandard has been consolidated into Practice

/////////////////////////////////////////////////////////////////////////////
// CNxProgressDlg dialog


CNxProgressDlg::CNxProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PROGRESS_DLG, pParent)
{
	//{{AFX_DATA_INIT(CNxProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nThreadMinPos = 0;
	m_nThreadMaxPos = 100;
	m_nThreadStepLength = 100000;
	m_pCurrentThread = NULL;
}


void CNxProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxProgressDlg)
	DDX_Control(pDX, IDC_CURRENT_THREAD_PROGRESS, m_CurrentThreadProgress);
	DDX_Control(pDX, IDC_OVERALL_PROGRESS, m_OverallProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNxProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CNxProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxProgressDlg message handlers

void CNxProgressDlg::StartNextProgress(CWinThread *pThread, int nMin, int nMax, LPCTSTR strDescription /* = NULL */)
{
	// Bump up thread counter
	m_nCurrentThreadIndex++;
	m_pCurrentThread = pThread;
	
	// Remember current thread properties
	m_nThreadMinPos = nMin;
	m_nThreadMaxPos = nMax;
	
	// Tell current thread properties to the progress bar
	m_CurrentThreadProgress.SetRange32(nMin, nMax);
	m_CurrentThreadProgress.SetPos(nMin);
	
	// Set the overall current position for this new thread
	m_OverallProgress.SetPos(m_nCurrentThreadIndex * m_nThreadStepLength);

	// If a string is specified to describe this overall precess, enter it here, otherwise just say "Overall Progress"
	SetDlgItemText(IDC_OVERALL_PROGRESS_LABEL, strDescription ? strDescription : "Overall Progress");
}

void CNxProgressDlg::ResetProgress(int nThreadCount /* = 1 */)
{
	// Only show individual progress if it will be different from the overall progress
	if (nThreadCount == 1) {
		m_CurrentThreadProgress.ShowWindow(SW_HIDE);
	} else {
		m_CurrentThreadProgress.ShowWindow(SW_SHOW);
	}
	
	// Initialize all values based on thread count
	m_nCurrentThreadIndex = -1;
	m_nThreadStepLength = 100000 / nThreadCount;
	m_OverallProgress.SetRange32(0, 100000);
	m_OverallProgress.SetPos(0);
	m_CurrentThreadProgress.SetPos(0);
}

void CNxProgressDlg::SetProgress(int nNewProgress, LPCTSTR strDescription /* = NULL */)
{
	// Set the current progress bar appropriately (easy)
	m_CurrentThreadProgress.SetPos(nNewProgress);

	// Set the overall progress bar (some calculations necessary)
	long nStartPos = m_nCurrentThreadIndex * m_nThreadStepLength;
	long nCurMidPos = (long)((double)m_nThreadStepLength * ((double)(nNewProgress - m_nThreadMinPos) / (double)(m_nThreadMaxPos - m_nThreadMinPos)));
	m_OverallProgress.SetPos(nStartPos + nCurMidPos);

	// If a description is specified display it
	SetDescription(strDescription);
}

void CNxProgressDlg::OnCancel() 
{
	if (m_pCurrentThread) {
		m_pCurrentThread->SuspendThread();
		if (MessageBox("Are you sure you want to cancel the install?", "Cancel?", MB_YESNO) == IDYES) {
			SetDescription("Please Wait -- Install is Being Cancelled");
			m_pCurrentThread->PostThreadMessage(NXM_CANCEL, 0, 0);
		}
		m_pCurrentThread->ResumeThread();
	}
	//	CDialog::OnCancel();
}

// If a description is specified display it
void CNxProgressDlg::SetDescription(LPCTSTR strNewDesc)
{
	if (strNewDesc) {
		SetDlgItemText(IDC_CURRENT_THREAD_PROGRESS_LABEL, strNewDesc);
	}
}

BOOL CNxProgressDlg::Create(int nProcessCnt /* = 1 */, LPCTSTR strInitDesc /* = NULL */, bool bVisible /* = true */, CWnd *pParentWnd /* = NULL */)
{
	// Make sure we look to our dll for the resources
	//HINSTANCE hOld = AfxGetResourceHandle();
	//AfxSetResourceHandle((HINSTANCE)::GetModuleHandle(DLL_MODULE_NAME));
	
	// Handle unspecified parent
	if (pParentWnd == NULL) {
		pParentWnd = GetDesktopWindow();
	}

	// Try to create the window
	BOOL bAns = CDialog::Create(IDD_PROGRESS_DLG, pParentWnd);
	
	// If Creation was successful
	if (bAns) {
		// Center the as yet invisible window
		CenterWindow();
		
		// Set the appropriate count of progresses to be
		ResetProgress(nProcessCnt);
		
		// Make visible or not depending on request
		if (bVisible) {
			ShowWindow(SW_SHOW);
		} else {
			ShowWindow(SW_HIDE);
		}
	}

	// Okay, we can now look back to the exe for future resources
	//AfxSetResourceHandle(hOld);

	// Return result
	return bAns;
}
