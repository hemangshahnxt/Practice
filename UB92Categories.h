#if !defined(AFX_UB92CATEGORIES_H__5016D8BC_73C9_4D8F_9023_3AA2EC788D8F__INCLUDED_)
#define AFX_UB92CATEGORIES_H__5016D8BC_73C9_4D8F_9023_3AA2EC788D8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UB92Categories.h : header file
//

#include "Client.h"

/////////////////////////////////////////////////////////////////////////////
// CUB92Categories dialog

class CUB92Categories : public CNxDialog
{
// Construction
public:
	CTableChecker m_UB92CatChecker;
	NXDATALISTLib::_DNxDataListPtr m_CategoryList;
	CUB92Categories(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUB92Categories)
	enum { IDD = IDD_UB92_CATEGORIES };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUB92Categories)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUB92Categories)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAddCat();
	afx_msg void OnDelCat();
	afx_msg void OnRButtonDownUb92CategoryList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingUb92CategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedUb92CategoryList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UB92CATEGORIES_H__5016D8BC_73C9_4D8F_9023_3AA2EC788D8F__INCLUDED_)
