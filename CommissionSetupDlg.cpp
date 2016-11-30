// CommissionSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "CommissionSetupDlg.h"
#include "globalfinancialutils.h"
#include "InternationalUtils.h"
#include "GlobalUtils.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.wetta 2007-03-28 17:34) - PLID 25360 - Redid the commission dialog to support the advance commission control

/////////////////////////////////////////////////////////////////////////////
// CCommissionSetupDlg dialog


CCommissionSetupDlg::CCommissionSetupDlg(CWnd* pParent)
	: CNxDialog(CCommissionSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommissionSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nProviderID = -1;
	m_CommissionSetupWnd = NULL;
}


void CCommissionSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommissionSetupDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_WND_PLACEHOLDER, m_nxstaticWndPlaceholder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommissionSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCommissionSetupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommissionSetupDlg message handlers

BEGIN_EVENTSINK_MAP(CCommissionSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCommissionSetupDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CCommissionSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// Create the commission window
	m_CommissionSetupWnd = new CCommissionSetupWnd(this);
	m_CommissionSetupWnd->m_nProviderID = m_nProviderID;
	// (a.wetta 2007-03-30 10:59) - PLID 24872 - Set the background color of the dialog match the contacts module
	m_CommissionSetupWnd->m_backgroundColor = GetNxColor(GNC_CONTACT, 0);
	m_CommissionSetupWnd->Create(IDD_COMMISSION_SETUP_WND, this);

	// Get the setup window's size
	CRect rcCommissionWnd, rcNew;
	m_CommissionSetupWnd->GetWindowRect(&rcCommissionWnd);
	ScreenToClient(&rcCommissionWnd);

	// (a.wetta 2007-03-29 14:30) - PLID 25407 - Because the Retail tab needs this dialog to have no border,
	// we have account for it when creating this pop up dialog
	long nCommissionWndBorder = 5;

	// Put the commission window on the dialog
	m_CommissionSetupWnd->SetWindowPos(&wndTop, nCommissionWndBorder, nCommissionWndBorder, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE);

	// Get the dimensions of the buttons
	CRect rcOK, rcCancel;
	GetDlgItem(IDOK)->GetWindowRect(&rcOK);
	GetDlgItem(IDCANCEL)->GetWindowRect(&rcCancel);

	long nWidth, nHeight, nWidthApart;
	//reset the width
	nWidth = 80;
	nHeight = rcOK.Height();
	nWidthApart = 30;

	rcNew = rcCommissionWnd;
	rcNew.bottom += nHeight + 10 + (2*nCommissionWndBorder);
	rcNew.right += (2*nCommissionWndBorder);
	
	//now move the OK and Cancel buttons; center them at the bottom of the dialog
	rcOK.left = (rcNew.Width()/2) - (nWidth + (nWidthApart/2));
	rcOK.right = (rcNew.Width()/2) - (nWidthApart/2);
	rcOK.top = rcNew.bottom - nHeight - 5;
	rcOK.bottom = rcNew.bottom - 5;

	rcCancel.left = (rcNew.Width()/2) + (nWidthApart/2);
	rcCancel.right = (rcNew.Width()/2) + (nWidth + (nWidthApart/2));
	rcCancel.top = rcNew.bottom - nHeight - 5;
	rcCancel.bottom = rcNew.bottom - 5;

	GetDlgItem(IDOK)->MoveWindow(rcOK);
	GetDlgItem(IDCANCEL)->MoveWindow(rcCancel);

	CSize szBorder;
	CalcWindowBorderSize(this, &szBorder);

	rcNew.right += szBorder.cx;
	rcNew.bottom += szBorder.cy;

	// Size the dialog
	SetWindowPos(NULL,0,0,rcNew.Width(),rcNew.Height(),SWP_NOZORDER);
	CenterWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCommissionSetupDlg::OnOK() 
{
	if (!m_CommissionSetupWnd->CloseWindow(TRUE))
		return;

	CDialog::OnOK();
}

void CCommissionSetupDlg::OnCancel() 
{
	if (!m_CommissionSetupWnd->CloseWindow(FALSE))
		return;

	CDialog::OnCancel();
}

BOOL CCommissionSetupDlg::DestroyWindow() 
{
	m_CommissionSetupWnd->DestroyWindow();
	delete m_CommissionSetupWnd;
	
	return CNxDialog::DestroyWindow();
}
