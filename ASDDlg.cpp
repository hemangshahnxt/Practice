// ASDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "ASDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CASDDlg dialog
// (a.walling 2008-07-03 15:44) - PLID 30498 - Generic dialog to warn and require a checkbox be selected before continuing


CASDDlg::CASDDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CASDDlg::IDD, pParent),
	m_bAllowCancel(false)
{
	//{{AFX_DATA_INIT(CASDDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// (a.walling 2010-01-13 14:16) - PLID 31253 - Option to allow cancel
// (a.walling 2010-01-13 14:51) - PLID 31253 - Set parameters for this dialog
void CASDDlg::SetParams(LPCTSTR strText, LPCTSTR strWindowTitle, LPCTSTR strTitle, LPCTSTR strAgree, bool bAllowCancel)
{
	m_strText = strText;
	m_strText.Replace("\r", "");
	m_strTitle = strTitle;
	m_strWindowTitle = strWindowTitle;
	m_strAgree = strAgree;
	m_bAllowCancel = bAllowCancel;
}

void CASDDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CASDDlg)
	DDX_Control(pDX, IDOK, m_nxibOK);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	// (a.walling 2010-01-13 14:16) - PLID 31253 - 'Other' button
	DDX_Control(pDX, IDC_ASD_OTHER, m_nxibOther);
	DDX_Control(pDX, IDC_ASD_TEXT, m_nxsText);
	DDX_Control(pDX, IDC_ASD_TITLE, m_nxsTitle);
	DDX_Control(pDX, IDC_CHECK_AGREE, m_nxbAgree);
	DDX_Control(pDX, IDC_ASD_COLOR, m_nxcolor);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CASDDlg, CNxDialog)
	//{{AFX_MSG_MAP(CASDDlg)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_CHECK_AGREE, OnCheckAgree)
	ON_BN_CLICKED(IDC_ASD_OTHER, OnOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CASDDlg message handlers

int CASDDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

BOOL CASDDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		SetWindowText(m_strWindowTitle);
		m_nxsTitle.SetWindowText(m_strTitle);
		m_nxsTitle.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_nxsText.SetWindowText(ConvertToControlText(m_strText));
		if (!m_strAgree.IsEmpty()) {
			m_nxbAgree.SetWindowText(m_strAgree);
		}
		
		// (a.walling 2010-01-13 14:28) - PLID 31253
		if (!m_bAllowCancel) {
			m_nxibCancel.EnableWindow(FALSE);
			m_nxibCancel.ShowWindow(SW_HIDE);
		}

		// (a.walling 2008-08-25 12:43) - PLID 30498 - Need OK/Cancel styles
		m_nxibOK.AutoSet(NXB_OK);
		m_nxibCancel.AutoSet(NXB_CANCEL);

		m_nxibOK.EnableWindow(FALSE);

		UpdateOtherButton();

		CRect rc;
		m_nxsText.GetClientRect(rc);

		CRect rcText = rc;
		rcText.bottom = rcText.top; // 0 height
		
		CDC* pDC = GetDC();
		CGdiObject* pOldObject = pDC->SelectObject(GetFont());
		pDC->DrawText(m_strText, rcText, DT_CALCRECT|DT_WORDBREAK|DT_LEFT);
		pDC->SelectObject(pOldObject);
		ReleaseDC(pDC);

		long nOffset = rcText.Height() - rc.Height();
		// (j.jones 2015-03-03 10:56) - PLID 65106 - support negative offsets to shrink the dialog as needed
		if (nOffset != 0) {
			CRect rcAdj;
			GetWindowRect(rcAdj);
			rcAdj.bottom += nOffset;
			SetWindowPos(NULL, 0, 0, rcAdj.Width(), rcAdj.Height(), SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

			m_nxcolor.GetWindowRect(rcAdj);
			ScreenToClient(rcAdj);
			rcAdj.bottom += nOffset;
			m_nxcolor.MoveWindow(rcAdj);

			m_nxsText.GetWindowRect(rcAdj);
			ScreenToClient(rcAdj);
			rcAdj.bottom += nOffset;
			m_nxsText.MoveWindow(rcAdj);

			m_nxbAgree.GetWindowRect(rcAdj);
			ScreenToClient(rcAdj);
			rcAdj.top += nOffset;
			rcAdj.bottom += nOffset;
			m_nxbAgree.MoveWindow(rcAdj);

			m_nxibOK.GetWindowRect(rcAdj);
			ScreenToClient(rcAdj);
			rcAdj.top += nOffset;
			rcAdj.bottom += nOffset;
			m_nxibOK.MoveWindow(rcAdj);

			m_nxibCancel.GetWindowRect(rcAdj);
			ScreenToClient(rcAdj);
			rcAdj.top += nOffset;
			rcAdj.bottom += nOffset;
			m_nxibCancel.MoveWindow(rcAdj);

			// (a.walling 2010-01-13 15:20) - PLID 31253 - Move the Other button
			m_nxibOther.GetWindowRect(rcAdj);
			ScreenToClient(rcAdj);
			rcAdj.top += nOffset;
			rcAdj.bottom += nOffset;
			m_nxibOther.MoveWindow(rcAdj);
		}

		m_nxibCancel.SetFocus();
	} NxCatchAll("Error in CASDDlg::OnInitDialog");
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CASDDlg::OnOK() 
{
	CNxDialog::OnOK();
}

void CASDDlg::OnCancel() 
{
	if (m_bAllowCancel) {
		CNxDialog::OnCancel();
	}
}

// (a.walling 2010-01-13 14:55) - PLID 31253 - Other button was clicked
void CASDDlg::OnOther()
{
	OnOtherButtonClicked();
}

void CASDDlg::OnCheckAgree() 
{
	if (m_nxbAgree.GetCheck() == BST_CHECKED) {
		m_nxibOK.EnableWindow(TRUE);
	} else {
		m_nxibOK.EnableWindow(FALSE);
	}
}

// (a.walling 2010-01-13 14:47) - PLID 31253 - Update the other button text / style
void CASDDlg::UpdateOtherButton()
{	
	m_nxibOther.ShowWindow(SW_HIDE);
	m_nxibOther.EnableWindow(FALSE);
}

// (a.walling 2010-01-13 14:55) - PLID 31253 - Other button was clicked
void CASDDlg::OnOtherButtonClicked()
{
}