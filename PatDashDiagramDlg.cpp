// GroupPictureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PatDashDiagramDlg.h"
#include "patientsrc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
// (j.gruber 2012-07-02 09:45) - PLID 51210 - created for

/////////////////////////////////////////////////////////////////////////////
// CPatDashDiagramDlg dialog


CPatDashDiagramDlg::CPatDashDiagramDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatDashDiagramDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPatDashDiagramDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPatDashDiagramDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatDashDiagramDlg)
	DDX_Control(pDX, IDOK, m_btnClose);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatDashDiagramDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatDashDiagramDlg)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatDashDiagramDlg message handlers

BOOL CPatDashDiagramDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		m_btnClose.AutoSet(NXB_CLOSE);

		m_image = NULL;
		Gdiplus::Bitmap* pGroupPicture = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_PATDASHDIAGRAM));

		if (pGroupPicture) {
			pGroupPicture->GetHBITMAP(Gdiplus::Color::Black, &m_image);
			delete pGroupPicture;
		}
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CPatDashDiagramDlg::OnPaint() 
{
	try {
		CPaintDC dc(this); // device context for painting
		CPalette *newPal, *oldPal;

		newPal = &theApp.m_palette;
		oldPal = dc.SelectPalette(newPal, FALSE);
		dc.RealizePalette();

		DrawBitmap(&dc, (HBITMAP)m_image, 0, 0);

		dc.SelectPalette(oldPal, FALSE);
	}NxCatchAll(__FUNCTION__);
}

void CPatDashDiagramDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		CNxDialog::OnShowWindow(bShow, nStatus);
		
		CMainFrame* pMain = GetMainFrame();
		if(pMain) {
			pMain->ActivateFrame();
			pMain->BringWindowToTop();
		}

		SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}NxCatchAll(__FUNCTION__);
}


void CPatDashDiagramDlg::OnDestroy() 
{
	try {
		if (m_image)
			DeleteObject(m_image);

		CNxDialog::OnDestroy();
	}NxCatchAll(__FUNCTION__);
}