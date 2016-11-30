#if !defined(AFX_MDICLIENTWND_H__A2ED9D46_A40F_4AD3_AFA5_BE782A9665FF__INCLUDED_)
#define AFX_MDICLIENTWND_H__A2ED9D46_A40F_4AD3_AFA5_BE782A9665FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MDIClientWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMDIClientWnd window

#include "NxGdiPlusUtils.h"
// (a.walling 2008-04-04 09:37) - PLID 29544 - Helper class for subclassing MDIClientWnd of MainFrame; fills with a subtle light gradient.

class CMDIClientWnd : public CWnd
{
// Construction
public:
	CMDIClientWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDIClientWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMDIClientWnd();

	// Generated message map functions
protected:
	long m_nMins;
	//{{AFX_MSG(CMDIClientWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDICLIENTWND_H__A2ED9D46_A40F_4AD3_AFA5_BE782A9665FF__INCLUDED_)
