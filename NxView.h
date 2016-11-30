#if !defined(AFX_NXVIEW_H__24372404_B9B7_11D1_B2DD_00001B4B970B__INCLUDED_)
#define AFX_NXVIEW_H__24372404_B9B7_11D1_B2DD_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NxView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxView view

#include "PracticeDoc.h"
#include "GlobalUtils.h"

class CNxView : public CView
{
protected:
	CNxView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CNxView)

	// (a.walling 2008-05-22 11:45) - PLID 27648 - Need to handle WM_CTLCOLOR messages for 'dialog' backgrounds.
	// (a.walling 2008-06-05 10:01) - PLID 30289 - Changed to a reference to the static shared NxDialog background brush
	const CBrush	&m_brBackground;

// Attributes
public:
	
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Defined here, since an CNxView may implement tab-like behavior but not necessary be a CNxTabView.
	virtual short GetActiveTab() {return -1;};
	virtual BOOL SetActiveTab(short tab) {return FALSE;};

	// (c.haag 2003-10-01 17:49) - If this is true, we MAY need to check
	// for the password.
	BOOL m_bCheckSecurityWhenActivated;

	// (c.haag 2003-10-01 17:50) - The active state of the window the
	// previous time OnActivateView was called.
	BOOL m_bPreviousActiveState;

	// (c.haag 2003-10-03 09:37) - Returns true if we are explicitly
	// closing this view.
	BOOL m_bIsExplicitlyClosing;

	// (c.haag 2003-10-02 15:39) - The last time we passed a security check
	COleDateTime m_dtLastSecurityCheck;

	// (c.haag 2003-11-13 10:13) - Reset the "security timer" in case a
	// user has password protection on a view, and the view is deactivated,
	// but not because the user activated a third party application.
	void ResetSecurityTimer();

	// (c.haag 2004-03-16 09:48) - Show the preferences dialog in its own way
	virtual int ShowPrefsDlg();
// Operations
public:
	// (a.walling 2011-08-04 14:36) - PLID 44788 - Get the active CNxDialog sheet - Also we don't necessarily want a const pointer
	virtual CNxDialog* GetActiveSheet()
	{
		return NULL;
	}

	virtual const CNxDialog* GetActiveSheet() const
	{
		return NULL;
	}

	virtual CPracticeDoc *GetDocument();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual int Hotkey(int key){ return 1; /*unhandled*/ }
	virtual void Delete(){}
	virtual void ShowToolBars();
	virtual BOOL CheckPermissions();
	bool m_bMarketToolBar;
	bool m_bPatientsToolBar;
	bool m_bContactsToolBar;
	bool m_bDateToolBar;
	CString m_strManualLocation;
	CString m_strManualBookmark;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	// (a.walling 2012-03-02 10:43) - PLID 48589 - Remove the client edge style to get rid of borders
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CNxView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	// (a.walling 2008-05-22 11:45) - PLID 27648 - Need to handle WM_CTLCOLOR messages for 'dialog' backgrounds.
	//{{AFX_MSG(CNxView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in NxView.cpp
inline CPracticeDoc* CNxView::GetDocument()
   { return (CPracticeDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXVIEW_H__24372404_B9B7_11D1_B2DD_00001B4B970B__INCLUDED_)
