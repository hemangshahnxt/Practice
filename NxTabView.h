#if !defined(AFX_NXTABVIEW_H__0A1E70E3_CE49_11D1_804C_00104B2FE914__INCLUDED_)
#define AFX_NXTABVIEW_H__0A1E70E3_CE49_11D1_804C_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NxTabView.h : header file
//

#include "NxView.h"
#include "Modules.h"
/////////////////////////////////////////////////////////////////////////////
// CNxTabView view

#define INVALID_SHEET	-1

class CNxTabView : public CNxView
{
protected:
	CNxTabView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CNxTabView)

// Attributes
public:
//	CNxTabCtl m_Tabs; someday use our own instead of that annoying TabPro thingy
// Operations
public:
	short m_defaultTab;

	// (a.walling 2011-08-04 14:36) - PLID 44788 - Get the active CNxDialog sheet (now virtual) - Also we don't necessarily want a const pointer
	virtual CNxDialog* GetActiveSheet()
	{
		return m_pActiveSheet;
	}

	virtual const CNxDialog* GetActiveSheet() const		
	{
		return m_pActiveSheet;
	}

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual void RecallDetails(CNxDialog *newSheet);
	virtual void StoreDetails(CNxDialog *oldSheet);
	// (a.walling 2008-07-02 17:43) - PLID 27648 - Support the alternate Short Title
	short CreateSheet(CNxDialog *pNewSheet, LPCTSTR strTitle = NULL, LPCTSTR strShortTitle = NULL);
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Create a sheet by calling into the modules code to get the tab info
	short CreateSheet(CNxDialog *pNewSheet, Modules::Tabs& tabs, int ordinal);
	virtual void SetDefaultControlFocus();
	virtual int Hotkey (int key);

	// (a.walling 2010-11-26 13:08) - PLID 40444 - these are now in CNxView as well, since they may implement their own tab-related thing
	virtual short GetActiveTab();
	virtual BOOL SetActiveTab(short tab);

	CString GetTabLabel(short tab);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxTabView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CNxDialog * m_pActiveSheet;
	CRect m_rectSheet;
	NxTab::_DNxTabPtr m_tab;
	CWnd m_wndTab;
	CBrush m_brush;
	HANDLE m_hSetSheetFocus;
	//Keep Practice from closing when you've got a PP screen up.
	bool m_bIsPrintPreviewRunning;

	virtual ~CNxTabView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	CPtrArray m_pSheet;

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	//{{AFX_MSG(CNxTabView)
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg virtual void OnSelectTab(short newTab, short oldTab);
	afx_msg void OnUpdateViewUI(CCmdUI* pCmdUI); // (z.manning 2010-07-19 15:59) - PLID 39222
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXTABVIEW_H__0A1E70E3_CE49_11D1_804C_00104B2FE914__INCLUDED_)