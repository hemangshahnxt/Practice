#if !defined(AFX_MIRRORIMAGEBUTTON_H__98736811_7801_11D3_AD6F_00104B318376__INCLUDED_)
#define AFX_MIRRORIMAGEBUTTON_H__98736811_7801_11D3_AD6F_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MirrorImageButton.h : header file
//

#define MIRRORIMAGEBUTTON_PENDING		0x00000001

/////////////////////////////////////////////////////////////////////////////
// CMirrorImageButton window

typedef enum {
	eImageSrcMirror,
	eImageSrcUnited,
	eImageSrcPractice,
	eImageSrcDiagram,
	eImageSrcRSIMMS, // (c.haag 2009-07-07 12:59) - PLID 34379
} EImageSource;

typedef enum {
	eNoError,
	eErrorNoPermission,
	eErrorUnspecified,
	// (c.haag 2009-04-01 17:23) - PLID 33630 - Status when the
	// Mirror link is being initialized asynchronously
	eImageBtnInitializingMirror,
} EImageError;

class CMirrorImageButton : public NxButton
{
// Construction
public:
	CMirrorImageButton();

// Attributes
public:
	CProgressCtrl m_progress;
	HBITMAP m_image;
	EImageSource m_source;
	bool m_bAutoDeleteImage;
	EImageError m_nError;

	// Applies only if EImageSource is eImageSrcPractice.
	CString m_strPracticeFileName;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorImageButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMirrorImageButton();
protected:
	void CalcProgressRect(CRect& rc);
	void Draw(CDC *pdc);

	// (j.jones 2008-06-17 16:00) - PLID 30419 - added a function to safely ensure the progress control exists
	void CheckEnsureProgressControl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMirrorImageButton)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIRRORIMAGEBUTTON_H__98736811_7801_11D3_AD6F_00104B318376__INCLUDED_)
