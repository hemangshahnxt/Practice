#if !defined(AFX_SINGLESELECTDLG_H__6EF06A82_B7EC_4CC6_A4DB_B5BEBBE949C9__INCLUDED_)
#define AFX_SINGLESELECTDLG_H__6EF06A82_B7EC_4CC6_A4DB_B5BEBBE949C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SingleSelectDlg.h : header file
//
#include "schedulerrc.h"
/////////////////////////////////////////////////////////////////////////////
// CSingleSelectDlg dialog

class CSingleSelectDlg : public CNxDialog
{
// Construction
public:
	CSingleSelectDlg(CWnd* pParent);   // standard constructor

	//TES 10/13/2008 - PLID 21093 - Added a parameter to tell the dialog to not let them leave the selection blank,
	// they will have to either select a record, or Cancel.
	HRESULT Open(CString strFrom, CString strWhere,
		CString strIDField, CString strValueField, CString strDescription, bool bForceSelection = false);
	// (c.haag 2010-09-24 10:41) - PLID 40677 - Support for memory values. The ID will be the index in the array.
	HRESULT Open(CStringArray* pastrValues, CString strDescription, bool bForceSelection = false);

	void PreSelect(long ID);
	// (c.haag 2010-09-24 10:41) - PLID 40677 - Support for pre-selecting a string (for memory values only)
	void PreSelect(const CString& str);
	long GetSelectedID();
	// (j.jones 2007-08-23 09:17) - PLID 27148 - added so we could get
	// the display name, but a variant since we can't assume it's a string
	_variant_t GetSelectedDisplayValue();

	// (z.manning 2010-05-11 13:02) - PLID 37416 - Added a checkbox to the single select dialog to have 2
	// different ways of filtering the list.
	void UseAdditionalFilter(const CString strAdditionalWhere, const CString strDisplayText, const BOOL bCheckedByDefault);

	// (z.manning 2010-05-11 14:43) - PLID 37416 - Added a function to set different text and style to the cancel button
	void SetCancelButtonStyle(const CString strText, const NXB_TYPE eStyle);

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CSingleSelectDlg)
	enum { IDD = IDD_SINGLESELECT_DLG };
	CNxStatic	m_nxstaticDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnAdditionalFilterCheck;
	//}}AFX_DATA
	long m_nSelectedID;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSingleSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strFrom, m_strWhere, m_strIDField, m_strValueField, m_strDescription;
	// (c.haag 2010-09-24 10:41) - PLID 40677 - Used to track memory values. We now support selecting from
	// an array of strings.
	CStringArray* m_pastrValues;
	// (j.jones 2007-08-23 09:17) - PLID 27148 - added so we could get
	// the display name, but a variant since we can't assume it's a string
	_variant_t m_varSelectedDisplayValue;
	long m_nPreSelectID;
	// (c.haag 2010-09-24 10:41) - PLID 40677 - Support for pre-selecting a string (for memory values only)
	CString m_strPreSelect;
	NXDATALISTLib::_DNxDataListPtr m_dlList;

	//TES 10/13/2008 - PLID 21093 - Added a parameter to tell the dialog to not let them leave the selection blank,
	// they will have to either select a record, or Cancel.
	bool m_bForceSelection;


	// (z.manning 2010-05-11 13:05) - PLID 37416 - Variables for the optional additional filter
	CString m_strAdditionalWhere;
	CString m_strFilterCheckDisplayText;
	BOOL m_bCheckAdditionalFilter;

	CString m_strCancelButtonText;
	NXB_TYPE m_eCancelButtonStyle;

	// (z.manning 2010-05-11 14:14) - PLID 37416 - Moved the requery logic to its own function
	void RequeryList();

	// Generated message map functions
	//{{AFX_MSG(CSingleSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedSingleSelectFilterCheck();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SINGLESELECTDLG_H__6EF06A82_B7EC_4CC6_A4DB_B5BEBBE949C9__INCLUDED_)
