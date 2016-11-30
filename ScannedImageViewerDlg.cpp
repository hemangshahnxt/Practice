// ScannedImageViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practiceRc.h"
#include "ScannedImageViewerDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2008-07-25 12:26) - PLID 30836 - Made this dialog modeless and resizable
/////////////////////////////////////////////////////////////////////////////
// CScannedImageViewerDlg dialog


CScannedImageViewerDlg::CScannedImageViewerDlg(CString strFilePath, CWnd* pParent /*=NULL*/)
	: CNxDialog(CScannedImageViewerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScannedImageViewerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strFilePath = strFilePath;
	m_hImage = NULL;
	m_bPromptMode = FALSE;
}

CScannedImageViewerDlg::~CScannedImageViewerDlg()
{
	FreeImage();
}



void CScannedImageViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScannedImageViewerDlg)
	DDX_Control(pDX, IDC_CAPTION, m_lblCaption);
	DDX_Control(pDX, IDC_CANCEL_DLG, m_btnCancel);
	DDX_Control(pDX, IDC_CLOSE_DLG, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScannedImageViewerDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CLOSE_DLG, OnClose)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CANCEL_DLG, OnCancelDlg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScannedImageViewerDlg message handlers

BOOL CScannedImageViewerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (a.walling 2008-09-11 09:02) - PLID 31334 - Rearrange some buttons and show the prompt if we need to
	if (m_bPromptMode) {
		m_lblCaption.ShowWindow(SW_SHOWNA);
		m_btnCancel.ShowWindow(SW_SHOWNA);
		m_lblCaption.SetWindowText(m_strPrompt);

		CRect rcClient;
		GetClientRect(rcClient);

		CRect rcClose;
		m_btnClose.GetWindowRect(rcClose);
		ScreenToClient(rcClose);

		CRect rcCancel;
		m_btnClose.GetWindowRect(rcCancel);
		ScreenToClient(rcCancel);

		CRect rcNewClose = rcClose;
		rcNewClose.left = rcClient.right - rcCancel.right;
		rcNewClose.right = rcNewClose.left + rcClose.Width();

		m_btnClose.MoveWindow(rcNewClose);
		
		m_btnClose.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
	} else {
		m_btnClose.AutoSet(NXB_CLOSE);
	}

	PrepareImage();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScannedImageViewerDlg::SetImage(const CString& strPath)
{
	if (m_strFilePath.CompareNoCase(strPath) != 0) {
		m_strFilePath = strPath;

		PrepareImage();
	}
}

void CScannedImageViewerDlg::PrepareImage()
{	
	FreeImage();
	if (!m_strFilePath.IsEmpty()) {
		LoadImageFile(m_strFilePath, m_hImage, -1);
	}
	Invalidate();
}

void CScannedImageViewerDlg::FreeImage()
{
	if (m_hImage) {
		DeleteObject(m_hImage);
		m_hImage = NULL;
	}
}

void CScannedImageViewerDlg::OnClose() 
{
	CDialog::OnOK();
	FreeImage();
}

void CScannedImageViewerDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	//lets load this image
	//first, get the rect of the button so that we don't go over it

	/*
	GetWindowRect(rctImage);
	rctImage.bottom = rctButton.top - 10;
	ScreenToClient(rctImage);
	*/

	CRect rctButton, rctImage, rctLabel;
	// (a.walling 2008-09-08 13:11) - PLID 31294 - We need to use client rects otherwise the rctImage will
	// have coordinates outside the client area.
	GetClientRect(rctImage);

	// (a.walling 2008-09-11 09:08) - PLID 31334 - Paint relative to label if we are showing it.
	if (m_bPromptMode) {
		m_lblCaption.GetWindowRect(rctLabel);
		ScreenToClient(rctLabel);

		rctImage.bottom = rctLabel.top - 10;
	} else {
		m_btnClose.GetWindowRect(rctButton);		
		ScreenToClient(rctButton);

		rctImage.bottom = rctButton.top - 10;
	}
	
	if (m_hImage) {
		DrawBitmapInRect(&dc, rctImage, m_hImage);
	} else {
		CString strError;
		if (!m_strFilePath.IsEmpty()) {
			strError = "Image could not be loaded!";
		} else {
			strError = "No image currently selected.";
		}
		CRect rcText = rctImage;
		dc.DrawText(strError, rcText, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	}
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CScannedImageViewerDlg::OnCancelDlg() 
{
	CDialog::OnCancel();
	FreeImage();
}
