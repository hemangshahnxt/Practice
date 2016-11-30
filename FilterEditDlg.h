// FilterEditDlg.h : header file
//
//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#include "NxStandard.h"
#include "WhereClause.h"
#include "FilterFieldInfo.h"
#include "AuditTrail.h"

#if !defined(AFX_FILTEREDITDLG_H__1344E1B6_FA5E_11D2_935B_00104B318376__INCLUDED_)
#define AFX_FILTEREDITDLG_H__1344E1B6_FA5E_11D2_935B_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum EnumFilterBaseJoin {
	fbjPatient = 1,
	fbjContact = 2,
	fbjProvider = 4,
	fbjRefPhys = 8,
	fbjSupplier = 16,
	fbjInsParty = 32,
	fbjInsCo = 64,
	fbjUser = 128,
	fbjRefSource = 256,
	
	fbjDefault = fbjPatient,
};


class CFilterDlg;

/////////////////////////////////////////////////////////////////////////////
// CFilterEditDlg dialog
class CFilterEditDlg : public CNxDialog
{
// Construction
public:
	CFilterEditDlg(CWnd* pParent, long nFilterType, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&) = NULL, LPCTSTR strTitle = NULL);
	virtual  ~CFilterEditDlg();

public:
	int EditFilter(long nFilterId, LPCTSTR strCurrentFilterString = NULL);
	int NewFilter();
	// (z.manning 2009-06-03 10:29) - PLID 34430 - Added optional audit transaction parameter
	bool ValidateFilter(CAuditTransaction *pAuditTransaction = NULL);
	bool Save();
	bool Load();

	// (z.manning, 04/25/2008) - PLID 29795 - NxIconButtons
// Dialog Data
	//{{AFX_DATA(CFilterEditDlg)
	enum { IDD = IDD_FILTER_EDIT_DLG };
	CNxEdit	m_nxeditFilterNameEdit;
	CNxStatic	m_nxstaticFilterBkgLabel;
	CNxStatic	m_nxstaticFilterBkgLabel2;
	CNxStatic	m_nxstaticFilterBkgLabel3;
	CNxStatic	m_nxstaticFilterBkgLabel4;
	CNxStatic	m_nxstaticFilterBkgLabel5;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSaveFilterAs;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterEditDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFilterDlg &m_dlgFilter;

	CString m_strTitle;

	//Callback functions for writing to data.
	BOOL (WINAPI* m_pfnIsActionSupported)(SupportedActionsEnum, long);
	BOOL (WINAPI* m_pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*);
	//Callback function for handling obsolete filter fields.
	BOOL (WINAPI* m_pfnGetNewFilterString)(long, long, LPCTSTR, CString&);

	// Generated message map functions
	//{{AFX_MSG(CFilterEditDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClearBtn();
	afx_msg void OnSaveFilterAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Letter Writing 5.0
public:
	// Interface
	long GetFilterId();
	CString m_strFilterName;
	CString m_strFilterString;
	CString m_strOldFilterName;	//for auditing
	long m_nFilterType;

	// For immediate access to the FROM and WHERE clauses (only valid after ValidateFilter() has returned true)
	CString m_strSqlFrom;
	CString m_strSqlWhere;

	// (z.manning 2009-06-02 15:12) - PLID 34430
	BOOL m_bAuditForLookup;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILTEREDITDLG_H__1344E1B6_FA5E_11D2_935B_00104B318376__INCLUDED_)
