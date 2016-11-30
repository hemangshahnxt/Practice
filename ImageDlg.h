#pragma once

// ImageDlg.h : header file
//

#include "MirrorImageButton.h"
/////////////////////////////////////////////////////////////////////////////
// CImageDlg dialog

class CImageDlg : public CNxDialog
{
// Construction
public:
	CImageDlg(CWnd* pParent);   // standard constructor

	int DoModal(HBITMAP image, const CString &text);

// Dialog Data
	//{{AFX_DATA(CImageDlg)
	enum { IDD = IDD_IMAGE_DLG };
	CMirrorImageButton	m_imageButton;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_title;
	long x, y;

	// Generated message map functions
	//{{AFX_MSG(CImageDlg)
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
