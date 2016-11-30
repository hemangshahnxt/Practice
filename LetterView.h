#if !defined(AFX_LETTERVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_LETTERVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdminView.h : header file
//503-525-6620

#include "NxTabView.h"
#include "LetterWriting.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterView view

class CLetterView : public CNxView
{
protected:
	CLetterView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLetterView)
	//virtual void OnSize(UINT nType, int cx, int cy);
// Attributes
public:
	// (a.walling 2011-08-04 14:36) - PLID 44788 - Get the active CNxDialog sheet - Also we don't necessarily want a const pointer
	virtual CNxDialog* GetActiveSheet()
	{
		return reinterpret_cast<CNxDialog*>(&m_frame);
	}

	virtual const CNxDialog* GetActiveSheet() const
	{
		return reinterpret_cast<const CNxDialog*>(&m_frame);
	}

	CLetterWriting m_frame;

// Operations
public:
	BOOL CheckPermissions();	
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLetterView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Implementation
protected:
	virtual ~CLetterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLetterView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateView();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMergeAllFields();
	afx_msg void OnUpdateMergeAllFields(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADMINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
