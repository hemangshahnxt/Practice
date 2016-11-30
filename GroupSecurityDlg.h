#if !defined(AFX_GROUPSECURITYDLG_H__A8CED7F5_DA1F_4DED_B988_3F85769CBC9E__INCLUDED_)
#define AFX_GROUPSECURITYDLG_H__A8CED7F5_DA1F_4DED_B988_3F85769CBC9E__INCLUDED_

#include <afxtempl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GroupSecurityDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGroupSecurityDlg dialog

class CGroupSecurityDlg : public CNxDialog
{
// Construction
public:
	CGroupSecurityDlg(CWnd* pParent);   // standard constructor

	int Open(long nPersonID);

// Dialog Data
	//{{AFX_DATA(CGroupSecurityDlg)
	enum { IDD = IDD_GROUP_SECURITY };
	CNxIconButton	m_btnNewTemplate;
	CNxIconButton	m_btnDeleteTemplate;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnEditTemplatePermissions;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroupSecurityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlGroups;

	// The person whose groups we are looking at
	long m_lPersonID;

	// Generated message map functions
	//{{AFX_MSG(CGroupSecurityDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnEditGroupPermissions();
	afx_msg void OnSelChangedMemberofList(long nNewSel);
	afx_msg void OnEditingFinishedMemberofList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnBtnHelp();
	afx_msg void OnBtnNewTemplate();
	afx_msg void OnBtnDeleteTemplate();
	afx_msg void OnDblClickCellMemberofList(long nRowIndex, short nColIndex);
	afx_msg void OnEditingFinishingMemberofList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPSECURITYDLG_H__A8CED7F5_DA1F_4DED_B988_3F85769CBC9E__INCLUDED_)
