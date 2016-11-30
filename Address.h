#if !defined(AFX_ADDRESS_H__C5C62077_0620_4305_88CE_7CED25B2639A__INCLUDED_)
#define AFX_ADDRESS_H__C5C62077_0620_4305_88CE_7CED25B2639A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Address.h : header file
//

#include "globalutils.h"

/////////////////////////////////////////////////////////////////////////////
// CAddress dialog

class CAddress : public CNxDialog
{
// Construction
public:
	void OK();
	CAddress(CWnd* pParent);   // standard constructor
	void ItemChange();
	long GetUserID();
	void SetUserID(long ID);

// Dialog Data
	//{{AFX_DATA(CAddress)
	enum { IDD = IDD_ADDRESS };
	NxButton	m_btnRadAddContacts;
	NxButton	m_btnRadDupMatch;
	NxButton	m_btnRadDupMerge;
	NxButton	m_chkAllowRestoreSyncImports;
	NxButton	m_chkSuppliers;
	NxButton	m_chkEmployees;
	NxButton	m_chkEmployee;
	NxButton	m_chkOther;
	NxButton	m_chkReferring;
	NxButton	m_chkDoctors;
	CComboBox	m_cmbPhone5;
	CComboBox	m_cmbPhone4;
	CComboBox	m_cmbPhone3;
	CComboBox	m_cmbPhone2;
	CComboBox	m_cmbPhone1;
	int		m_nPhone1;
	int		m_nPhone2;
	int		m_nPhone5;
	int		m_nPhone4;
	int		m_nPhone3;
	BOOL	m_bDoctors;
	BOOL	m_bOther;
	BOOL	m_bReferring;
	BOOL	m_bEmployees;
	BOOL	m_bSuppliers;
	BOOL	m_bPrimary;
	BOOL	m_bAllowRestoreSyncImports;
	int		m_iDuplicateHandling;
	NxButton	m_btnCategoriesGroupbox;
	NxButton	m_btnFieldsGroupbox;
	NxButton	m_btnDuplicateGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	ADODB::_RecordsetPtr		m_rs;
	long m_nPUserID;

	void InitDlg();

	void OnOK();
	void OnCancel();
	// Generated message map functions
	//{{AFX_MSG(CAddress)
	afx_msg void OnSelchangeComboPhone1();
	afx_msg void OnSelchangeComboPhone2();
	afx_msg void OnSelchangeComboPhone3();
	afx_msg void OnSelchangeComboPhone4();
	afx_msg void OnSelchangeComboPhone5();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDRESS_H__C5C62077_0620_4305_88CE_7CED25B2639A__INCLUDED_)
