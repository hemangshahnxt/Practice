#if !defined(AFX_PHOTOVIEWERDLG_H__DDCC8A77_3271_43CC_80A8_3577A6B47B1C__INCLUDED_)
#define AFX_PHOTOVIEWERDLG_H__DDCC8A77_3271_43CC_80A8_3577A6B47B1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhotoViewerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPhotoViewerDlg dialog
#include "PhotoViewerCtrl.h"

class CPhotoViewerDlg : public CNxDialog
{
// Construction
public:
	CPhotoViewerDlg(CWnd* pParent);   // standard constructor
	~CPhotoViewerDlg();

	// (j.jones 2009-10-13 13:10) - PLID 35894 - converted to be pointers
	// (a.walling 2010-12-20 16:05) - PLID 41917 - use smart pointers
	CArray<ImageInformationPtr,ImageInformationPtr> m_arImages;
	ImageLayout m_ilLayout;

	CString m_strPatientName;

// Dialog Data
	//{{AFX_DATA(CPhotoViewerDlg)
	enum { IDD = IDD_PHOTO_VIEWER_DLG };
	CNxIconButton	m_btnPrint;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnRight;
	CNxIconButton	m_btnPageRight;
	CNxIconButton	m_btnLeft;
	CNxIconButton	m_btnPageLeft;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhotoViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Takes in the notes and the image rectangle, modifies the image rectangle and fills in the text rectangle.
	void CalcRects(CDC *pDC, const CString &strNotes, CRect &rImage, CRect &rText);

	CFont *m_pIconFont;
	CFont *m_pIconFontPrint;

	NXDATALIST2Lib::_DNxDataListPtr m_pFontSizeList; //(e.lally 2012-04-23) PLID 29583
	long m_nFontSize; //(e.lally 2012-04-23) PLID 29583
	int m_nIndex;

	// Generated message map functions
	//{{AFX_MSG(CPhotoViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnPhotoPageLeft();
	afx_msg void OnPhotoLeft();
	afx_msg void OnPhotoPageRight();
	afx_msg void OnPhotoRight();
	afx_msg void OnPrintPhotos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	//(e.lally 2012-04-23) PLID 29583
	void OnSelChangingFontSizeList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	void OnSelChosenFontSizeList(LPDISPATCH lpRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHOTOVIEWERDLG_H__DDCC8A77_3271_43CC_80A8_3577A6B47B1C__INCLUDED_)
