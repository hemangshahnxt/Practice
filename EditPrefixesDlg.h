#if !defined(AFX_EDITPREFIXESDLG_H__C6FB41FA_95B6_4AB0_8A5F_E9DC37D88375__INCLUDED_)
#define AFX_EDITPREFIXESDLG_H__C6FB41FA_95B6_4AB0_8A5F_E9DC37D88375__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditPrefixesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditPrefixesDlg dialog

class CEditPrefixesDlg : public CNxDialog
{
// Construction
public:
	CEditPrefixesDlg(CWnd* pParent);   // standard constructor
	bool m_bChangeInformIds;

// Dialog Data
	//{{AFX_DATA(CEditPrefixesDlg)
	enum { IDD = IDD_EDIT_PREFIXES };
	CNxIconButton	m_btnAddPrefix;
	CNxIconButton	m_btnRemovePrefix;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditPrefixesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pPrefixes;

	void RefreshButtons();

	// Generated message map functions
	//{{AFX_MSG(CEditPrefixesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddPrefix();
	afx_msg void OnRemovePrefix();
	virtual void OnOK();
	afx_msg void OnEditingFinishingEditPrefix(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEditPrefix(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChosenEditPrefix(long nRow);
	afx_msg void OnLeftClickEditPrefix(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITPREFIXESDLG_H__C6FB41FA_95B6_4AB0_8A5F_E9DC37D88375__INCLUDED_)
