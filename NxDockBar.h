#if !defined(AFX_NXDOCKBAR_H__B701B252_8CA1_498A_9CEA_6F89C6FAACB0__INCLUDED_)
#define AFX_NXDOCKBAR_H__B701B252_8CA1_498A_9CEA_6F89C6FAACB0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxDockBar.h : header file
//

// (a.walling 2008-04-14 14:21) - PLID 29642 - Overridden CDockBar for custom painting

/////////////////////////////////////////////////////////////////////////////
// CNxDockBar frame

#include "afxpriv.h"
#include "NxGdiPlus.h"

class CNxDockBar : public CDockBar
{
public:
	CNxDockBar();

// Attributes

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxDockBar)
	//}}AFX_VIRTUAL
    CSize CalcDynamicLayout(int nLength, DWORD nMode);


// Implementation
public:
	virtual ~CNxDockBar();

protected:
	// (a.walling 2009-01-14 12:33) - PLID 32734 - Use GDI primitives rather than a Gdiplus::Bitmap*
	//Gdiplus::Bitmap* m_pCachedGradient;
	CBitmap m_bmpGradient;
	CBrush m_brushPattern;

	// Generated message map functions
	//{{AFX_MSG(CNxDockBar)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXDOCKBAR_H__B701B252_8CA1_498A_9CEA_6F89C6FAACB0__INCLUDED_)
