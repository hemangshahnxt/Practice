// ImageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageDlg dialog


CImageDlg::CImageDlg(CWnd* pParent)
	: CNxDialog(CImageDlg::IDD, pParent)
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
}


void CImageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageDlg)
	DDX_Control(pDX, IDC_IMAGE, m_imageButton);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImageDlg, CNxDialog)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageDlg message handlers

int CImageDlg::DoModal(HBITMAP image, const CString &text)
{
	m_title = text;

	m_imageButton.m_image = image;
	m_imageButton.m_bAutoDeleteImage = false;

	BITMAP tmpBmp;
	GetObject(image, sizeof(tmpBmp), &tmpBmp);
	x = tmpBmp.bmWidth;
	y = tmpBmp.bmHeight;

	return CNxDialog::DoModal();
}

void CImageDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CImageDlg::OnOK() 
{
	CDialog::OnOK();
}

BOOL CImageDlg::OnInitDialog() 
{
	//TES 3/1/2004: It's already not redrawing, silly!
	//SetRedraw(FALSE);
	CNxDialog::OnInitDialog();

	SetWindowText(m_title);

	long border = 2 * GetSystemMetrics(SM_CYSIZEFRAME); //border width
	long nc = 2 * GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION); //non-client height

	if (x + border <= GetSystemMetrics(SM_CXSCREEN) 
		&& y + nc <= GetSystemMetrics(SM_CYSCREEN))
	{	MoveWindow(0, 0, x + border, y + nc, FALSE);
		CenterWindow();
	}
	else { 
		// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
		SetWindowPos(NULL, 0, 0, 994, 738, 0);
		ShowWindow(SW_SHOWMAXIMIZED); 
	}
	//SetRedraw(TRUE);
	
	return TRUE;
}
