#if !defined(AFX_NXTOOLBAR_H__82191EAA_97EB_4FEB_8D1E_B64E26896A3B__INCLUDED_)
#define AFX_NXTOOLBAR_H__82191EAA_97EB_4FEB_8D1E_B64E26896A3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxToolbar.h : header file
//
#include "dynuxtheme.h"
#include "NxGdiPlus.h"

// (a.walling 2008-04-16 09:41) - PLID 29673 - Helper class to handle our own drawing of the toolbar

/////////////////////////////////////////////////////////////////////////////
// CNxToolBar window

class CNxToolBar : public CToolBar
{
// Construction
public:
	CNxToolBar();

// Attributes
public:

// Operations
public:
	void DrawHighlight(Gdiplus::Graphics& g, const Gdiplus::Rect& rect, const Gdiplus::Color& colorA, const Gdiplus::Color& colorB);
	// (a.walling 2009-08-13 10:30) - PLID 35214 - Let us know if the screen uses indexed color so we can disable smoothing
	void DrawButtonText(Gdiplus::Graphics& g, const CString& strLabel, Gdiplus::RectF rectLabel, BOOL bIsActive, WORD nFlatState, BOOL bIndexedColor);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNxToolBar();

	// Generated message map functions
protected:
	// (a.walling 2008-06-17 16:32) - PLID 30420 - Override WM_ERASEBKGND
	// (a.walling 2008-07-02 13:59) - PLID 29757 - Handle mousedown/up messages to catch clicking on disabled buttons
	// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
	// (a.walling 2008-10-08 16:33) - PLID 31575 - Eat RButton messages
	//{{AFX_MSG(CNxToolBar)
		afx_msg BOOL OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		afx_msg void OnNcPaint();
	//}}AFX_MSG

	BOOL m_bThemeInit;
	// (a.walling 2008-07-02 14:00) - PLID 29757 - Last button index that got a mousedown message
	long m_nLButtonDownIndex;
	UXTheme &m_uxtTheme;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXTOOLBAR_H__82191EAA_97EB_4FEB_8D1E_B64E26896A3B__INCLUDED_)
