#if !defined(AFX_LOCATIONSELECTIMAGEDLG_H__BF2BA922_C8F1_43A5_BA9D_1F21B63DFCB7__INCLUDED_)
#define AFX_LOCATIONSELECTIMAGEDLG_H__BF2BA922_C8F1_43A5_BA9D_1F21B63DFCB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocationSelectImageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLocationSelectImageDlg dialog

// (d.moore 2007-07-16 15:16) - PLID 14799 - This dialog was created to handle 
//  adding images for locations. Currently images may be either a logo for the 
//  location or a general picture (such as a picture of the building or of the 
//  staff).

#include "mirrorimagebutton.h"

enum EImageType {
	eitLogo, 
	eitGeneralImage
};

class CLocationSelectImageDlg : public CNxDialog
{
// Construction
public:
	// Require both a location ID and a value to indicate if we are selecting
	//  a logo or other image.
	CLocationSelectImageDlg(long nLocationID, EImageType nType, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLocationSelectImageDlg)
	enum { IDD = IDD_LOCATION_SELECT_IMAGE };
	CMirrorImageButton	m_btnLocationImage;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSelectImage;
	CNxIconButton	m_btnClearImage;
	CNxStatic	m_nxstaticLocationImageLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocationSelectImageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (d.moore 2007-07-16 17:06) - PLID 14799 - Prevent this dialog from
	//  being called without specifying a location an image type (logo, etc...).
	CLocationSelectImageDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	BOOL Save();

	long m_nLocationID;
	long m_nImageType;
	CString m_strImageFile;
	CString m_strImagePathDbField;
	CString m_strImagePath;

	// (a.walling 2010-09-16 10:51) - PLID 31435
	long m_nLogoWidth;
	bool SaveLogoWidth();

	// Generated message map functions
	//{{AFX_MSG(CLocationSelectImageDlg)
	afx_msg void OnSelectImage();
	virtual BOOL OnInitDialog();
	afx_msg void OnClearImage();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnKillfocusLocationLogoWidthEdit();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATIONSELECTIMAGEDLG_H__BF2BA922_C8F1_43A5_BA9D_1F21B63DFCB7__INCLUDED_)
