// AlertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "AlertDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlertDlg dialog


CAlertDlg::CAlertDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAlertDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAlertDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwVisibleAlert = 0;
}


void CAlertDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlertDlg)
	DDX_Control(pDX, IDC_BTN_PREV, m_btnPrev);
	DDX_Control(pDX, IDC_BTN_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_BTN_LAST, m_btnLast);
	DDX_Control(pDX, IDC_BTN_FIRST, m_btnFirst);
	DDX_Control(pDX, IDC_STATIC_ALERT, m_nxeditStaticAlert);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_STATIC_GROUPBOX, m_btnGroupbox);
	DDX_Control(pDX, IDC_ALERT_ACTION_LABEL, m_nxlActionLabel); // (a.walling 2010-10-11 17:27) - PLID 40731
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlertDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAlertDlg)
	ON_BN_CLICKED(IDC_BTN_FIRST, OnBtnFirst)
	ON_BN_CLICKED(IDC_BTN_PREV, OnBtnPrev)
	ON_BN_CLICKED(IDC_BTN_NEXT, OnBtnNext)
	ON_BN_CLICKED(IDC_BTN_LAST, OnBtnLast)
	ON_WM_ACTIVATE()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CAlertDlg::OnLabelClick)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlertDlg message handlers

// (a.walling 2010-10-11 17:11) - PLID 40731 - Pass in the associated ID and alert type
void CAlertDlg::SetAlertMessage(const char* szMsg, long nAssociatedID, EAlert alertType)
{
	// (a.walling 2010-10-11 17:13) - PLID 40731 - Use a structure to keep track of alerts
	m_arAlerts.push_back(AlertItem(szMsg, COleDateTime::GetCurrentTime(), nAssociatedID, alertType));
	m_dwVisibleAlert = (DWORD)(m_arAlerts.size() - 1);
	UpdateVisibleAlerts();
}

void CAlertDlg::UpdateVisibleAlerts()
{
	CString str;

	////////////////////////////////////////
	// Set the current alert
	// (a.walling 2010-10-11 17:13) - PLID 40731
	
	AlertItem alertItem;
	if (!m_arAlerts.empty())
	{
		if (m_dwVisibleAlert >= (DWORD)m_arAlerts.size())
			m_dwVisibleAlert = (DWORD)(m_arAlerts.size() - 1);

		alertItem = m_arAlerts[m_dwVisibleAlert];

		SetDlgItemText(IDC_STATIC_ALERT, alertItem.m_strMessage);
	}
	
	// (a.walling 2010-10-11 17:27) - PLID 40731
	if (alertItem.m_alertType == PatientIsIn && alertItem.m_nAssociatedID != -1) {
		m_nxlActionLabel.SetType(dtsHyperlink);
		m_nxlActionLabel.SetHzAlign(DT_CENTER);
		m_nxlActionLabel.SetText("Go to patient");
		m_nxlActionLabel.EnableWindow(TRUE);
		m_nxlActionLabel.ShowWindow(SW_SHOWNA);
	} else {
		m_nxlActionLabel.ShowWindow(SW_HIDE);
		m_nxlActionLabel.EnableWindow(FALSE);
		m_nxlActionLabel.SetText("");
	}

	////////////////////////////////////////
	// Update the alert count
	
	// (a.walling 2010-10-11 17:13) - PLID 40731
	str.Format("Message %d of %d sent at %s", m_dwVisibleAlert+1, m_arAlerts.size(), alertItem.m_dt.Format("%I:%M %p"));
	// (a.walling 2010-10-12 12:29) - PLID 40897 - Invalidate the group box to ensure the background is erased.
	InvalidateDlgItem(IDC_STATIC_GROUPBOX);
	SetDlgItemText(IDC_STATIC_GROUPBOX, str);

	////////////////////////////////////////
	// Update the buttons
	GetDlgItem(IDC_BTN_FIRST)->EnableWindow( m_dwVisibleAlert != 0 );
	GetDlgItem(IDC_BTN_PREV)->EnableWindow( m_dwVisibleAlert != 0 );
	GetDlgItem(IDC_BTN_NEXT)->EnableWindow( m_dwVisibleAlert != ((DWORD)m_arAlerts.size() - 1) );
	GetDlgItem(IDC_BTN_LAST)->EnableWindow( m_dwVisibleAlert != ((DWORD)m_arAlerts.size() - 1) );
}

void CAlertDlg::OnBtnFirst() 
{
	m_dwVisibleAlert = 0;
	UpdateVisibleAlerts();
}

void CAlertDlg::OnBtnPrev() 
{
	if (m_dwVisibleAlert == 0) return;
	m_dwVisibleAlert--;
	UpdateVisibleAlerts();
}

void CAlertDlg::OnBtnNext() 
{
	// (a.walling 2010-10-11 17:13) - PLID 40731
	if (m_dwVisibleAlert == ((DWORD)m_arAlerts.size() - 1)) return;
	m_dwVisibleAlert++;
	UpdateVisibleAlerts();
}

void CAlertDlg::OnBtnLast() 
{
	// (a.walling 2010-10-11 17:13) - PLID 40731
	m_dwVisibleAlert = (DWORD)m_arAlerts.size() - 1;
	UpdateVisibleAlerts();
}

BOOL CAlertDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (a.walling 2008-06-09 16:48) - PLID 22049 - This lacks a taskbar icon style
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			ModifyStyleEx(0, WS_EX_APPWINDOW);
		}
		
		m_btnFirst.AutoSet(NXB_LLEFT);
		m_btnPrev.AutoSet(NXB_LEFT);
		m_btnNext.AutoSet(NXB_RIGHT);
		m_btnLast.AutoSet(NXB_RRIGHT);
		// (c.haag 2008-04-28 11:10) - PLID 29793 - NxIconize the OK button
		m_btnOK.AutoSet(NXB_CLOSE);
	}
	NxCatchAll("Error in CAlertDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2007-05-04 09:52) - PLID 4850 - Clear all alerts (called when switching users)
void CAlertDlg::Clear()
{
	// (a.walling 2010-10-11 17:13) - PLID 40731
	m_arAlerts.clear();
	m_dwVisibleAlert = 0;
}

void CAlertDlg::OnOK()
{
	//Do not call CDialog::OnOK() for a modeless window!
	ShowWindow(SW_HIDE);
}

//returns the number of alerts currently stored
long CAlertDlg::GetAlertCount()
{
	// (a.walling 2010-10-11 17:13) - PLID 40731
	return m_arAlerts.size();
}

void CAlertDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);
	
	if (nState == WA_ACTIVE) {
		// (a.walling 2012-10-05 09:36) - PLID 53027 - Fix mainframe activation
		//CMainFrame *pMainWnd = GetMainFrame();
		//if (pMainWnd->GetSafeHwnd() && pMainWnd->IsIconic()) {
		//	pMainWnd->BringWindowToTop();
		//}
	}	
	else if (nState == WA_INACTIVE)
	{
		if (GetMainFrame() && GetMainFrame()->GetSafeHwnd())
		{
			CNxTabView* pView = GetMainFrame()->GetActiveView();

			// Reset the timer before the view gets its activation message
			if (pView && pView->GetSafeHwnd()) pView->ResetSecurityTimer();
		}
	}
}

HBRUSH CAlertDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC_ALERT:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;
	}

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (a.walling 2010-10-11 17:27) - PLID 40731
BOOL CAlertDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		CRect rc;
		m_nxlActionLabel.GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2010-10-11 17:27) - PLID 40731
LRESULT CAlertDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		if (m_dwVisibleAlert < 0 || m_dwVisibleAlert >= m_arAlerts.size()) {
			return 0;
		}

		if (!m_nxlActionLabel.IsWindowEnabled() || !m_nxlActionLabel.IsWindowVisible()) {
			return 0;
		}

		AlertItem alertItem = m_arAlerts[m_dwVisibleAlert];
		if (alertItem.m_alertType == PatientIsIn && alertItem.m_nAssociatedID != -1) {
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT PatientID FROM AppointmentsT WHERE ID = {INT}", alertItem.m_nAssociatedID);

			if (!prs->eof) {
				long nPatientID = AdoFldLong(prs, "PatientID", -1);

				if (nPatientID >= 0 && GetMainFrame() != NULL) {					
					// Seems better to not hide in this case
					//ShowWindow(SW_HIDE);
					GetMainFrame()->GotoPatient(nPatientID);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}
