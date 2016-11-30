#if !defined(AFX_NXTREE_H__08871B64_E1FB_11D2_9350_00104B318376__INCLUDED_)
#define AFX_NXTREE_H__08871B64_E1FB_11D2_9350_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxTree window

class CNxTree : public CTreeCtrl
{
// Construction
public:
	CNxTree();
// Attributes
public:
	CString		GetSelectedItemText();
	void		SetSelectedItemText(CString set);
	void		SelectFirstVisible();
	HTREEITEM	GetSelectedItemParent();
	bool		m_add;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxTree)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CNxTree();

	// Generated message map functions
protected:
	//{{AFX_MSG(CNxTree)
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXTREE_H__08871B64_E1FB_11D2_9350_00104B318376__INCLUDED_)
