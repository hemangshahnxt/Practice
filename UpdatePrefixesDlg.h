#if !defined(AFX_UPDATEPREFIXESDLG_H__5CD7CDC2_5F6A_453C_BECD_94826FF1E4AD__INCLUDED_)
#define AFX_UPDATEPREFIXESDLG_H__5CD7CDC2_5F6A_453C_BECD_94826FF1E4AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdatePrefixesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdatePrefixesDlg dialog

class CUpdatePrefixesDlg : public CNxDialog
{
// Construction
public:
	CUpdatePrefixesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUpdatePrefixesDlg)
	enum { IDD = IDD_UPDATE_PREFIXES };
	NxButton	m_btnUpdateMales;
	NxButton	m_btnUpdateFemales;
	CComboBox	m_MaleType;
	CComboBox	m_FemaleType;
	CNxIconButton	m_btnUpdatePrefix;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdatePrefixesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pMaleTitle;
	NXDATALISTLib::_DNxDataListPtr m_pFemaleTitle;

	// Generated message map functions
	//{{AFX_MSG(CUpdatePrefixesDlg)
	afx_msg void OnUpdatePrefix();
	afx_msg void OnCloseUpdatePrefixes();
	afx_msg void OnUpdateFemales();
	afx_msg void OnUpdateMales();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	CString GetUpdateString(long nGender);
	CString GetNewPrefix(long nGender);
	CString GetWhereClause(long nGender);
	long GetBoxID(long nGender);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEPREFIXESDLG_H__5CD7CDC2_5F6A_453C_BECD_94826FF1E4AD__INCLUDED_)
