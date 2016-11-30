// EMNWaitForAccessDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "EMNWaitForAccessDlg.h"
#include "EmrTreeWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMNWaitForAccessDlg dialog
// (a.walling 2008-07-03 12:58) - PLID 30498 - Dialog to show waiting for user to save/release write access

// (j.jones 2013-05-16 14:39) - PLID 56596 - m_wtInfo is now a reference, needs to be created here
CEMNWaitForAccessDlg::CEMNWaitForAccessDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMNWaitForAccessDlg::IDD, pParent),
	m_wtInfo(*(new CWriteTokenInfo()))
{
	//{{AFX_DATA_INIT(CEMNWaitForAccessDlg)
	//}}AFX_DATA_INIT
	m_nEmnID = -1;
}

// (j.jones 2013-05-16 14:40) - PLID 56596 - added a destructor
CEMNWaitForAccessDlg::~CEMNWaitForAccessDlg()
{
	try {

		// (j.jones 2013-05-16 14:39) - PLID 56596 - m_wtInfo is now a reference, it is never null,
		// and always is filled in the constructor, and must be cleared here
		delete &m_wtInfo;

	}NxCatchAll(__FUNCTION__);
}

void CEMNWaitForAccessDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMNWaitForAccessDlg)
	DDX_Control(pDX, IDC_BTN_FORCE_NOW, m_nxibForceNow);
	DDX_Control(pDX, IDC_WAIT_TIMER, m_nxstaticTimer);
	DDX_Control(pDX, IDC_WAIT_INFO, m_nxstaticInfo);
	DDX_Control(pDX, IDOK, m_nxibOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMNWaitForAccessDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMNWaitForAccessDlg)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_FORCE_NOW, OnBtnForceNow)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMNWaitForAccessDlg message handlers

#define IDT_WAIT_TIMER 1001

BOOL CEMNWaitForAccessDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		CString str;

		str.Format("Requesting write access from user %s for EMN: %s...",
			m_wtInfo.strHeldByUserName, m_strDescription);

		m_nxstaticInfo.SetWindowText(str);
		m_dtInitialTime = COleDateTime::GetCurrentTime();

		m_nxstaticTimer.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_nxibForceNow.EnableWindow(FALSE);

		UpdateTimer();

		SetTimer(1001, 1000, NULL);
	} NxCatchAll("Error in CEMNWaitForAccessDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMNWaitForAccessDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);	
}

void CEMNWaitForAccessDlg::OnTimer(UINT nIDEvent) 
{
	switch (nIDEvent) {
	case IDT_WAIT_TIMER: {
		UpdateTimer();
		break;
						 }
	}
	
	CNxDialog::OnTimer(nIDEvent);
}

void CEMNWaitForAccessDlg::OnBtnForceNow() 
{
	try {
		// prompt to make sure they are absolutely aware of what they are doing!
		// (c.haag 2012-06-11) - PLID 50806 - Moved into a utility function
		if (IDOK == DoForcedWriteAcquisitionPrompt(this, m_wtInfo)) {
			// luke.. use the force
			CWnd* pOwner = GetOwner();
			ASSERT(pOwner);
			ASSERT(pOwner->IsKindOf(RUNTIME_CLASS(CEmrTreeWnd)));
			if (pOwner) {
				pOwner->PostMessage(NXM_TRY_ACQUIRE_WRITE_ACCESS, (WPARAM)m_nEmnID, MAKELPARAM(1, 1));
			}

			ShowWindow(SW_HIDE);
		}
	} NxCatchAll("Error in CEMNWaitForAccessDlg::OnBtnForceNow");
}

void CEMNWaitForAccessDlg::OnOK() 
{	
	ShowWindow(SW_HIDE);
}

void CEMNWaitForAccessDlg::OnCancel() 
{	
	ShowWindow(SW_HIDE);
}

void CEMNWaitForAccessDlg::SetInfo(long nID, const CString &strDescription, const CWriteTokenInfo& wtInfo)
{
	m_nEmnID = nID;
	m_wtInfo = wtInfo;
	m_strDescription = strDescription;
}

void CEMNWaitForAccessDlg::UpdateTimer()
{
	try {
		CString str;
		COleDateTime dtCur = COleDateTime::GetCurrentTime();

		COleDateTimeSpan dtsDiff = dtCur - m_dtInitialTime;

		long nSeconds = (long)dtsDiff.GetTotalSeconds();

		if (nSeconds <= 0) {
			str.Format("Waiting...");
		} else {
			str.Format("Waiting... (%li seconds)", nSeconds);
		}

		m_nxstaticTimer.SetWindowText(str);

		// they must wait 5 seconds!
		if (nSeconds >= 5) {
			m_nxibForceNow.EnableWindow(TRUE);
		}

	} NxCatchAll("CEMNWaitForAccessDlg::UpdateTimer");
}

void CEMNWaitForAccessDlg::OnDestroy() 
{	
	KillTimer(IDT_WAIT_TIMER);	

	CNxDialog::OnDestroy();
}
