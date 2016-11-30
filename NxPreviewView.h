//CNxPreviewView.h

#if !defined(AFX_NXPREVIEWVIEW_H__F2B94DB7_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
#define AFX_NXPREVIEWVEW_H__F2B94DB7_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_

#pragma once

#include <afxpriv.h>

class CNxPreviewView : public CPreviewView
{
protected:
	CNxPreviewView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CNxPreviewView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxPreviewView)
	protected:
	//virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//virtual void OnInitialUpdate();     // first time after construct
	//virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CNxPreviewView();
#ifdef _DEBUG
	//virtual void AssertValid() const;
	//virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CNxPreviewView)
	afx_msg void OnPreviewClose();
	afx_msg void OnPreviewPrint();
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_NXPREVIEWVIEW_H__F2B94DB7_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)