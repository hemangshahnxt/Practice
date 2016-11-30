#if !defined(AFX_COMMISSIONSETUPDLG_H__F8094C42_408F_43F5_9A60_1B746855A7B9__INCLUDED_)
#define AFX_COMMISSIONSETUPDLG_H__F8094C42_408F_43F5_9A60_1B746855A7B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommissionSetupDlg.h : header file
//

#include "CommissionSetupWnd.h"

// (a.wetta 2007-03-28 17:34) - PLID 25360 - Redid the commission dialog to support the advance commission control

class CCommissionSetupWnd;

#define IDC_COMMISSION_SETUP_WND		1001

/////////////////////////////////////////////////////////////////////////////
// CCommissionSetupDlg dialog

class CCommissionSetupDlg : public CNxDialog
{
// Construction
public:
	CCommissionSetupDlg(CWnd* pParent);   // standard constructor

	long m_nProviderID;

// Dialog Data
	//{{AFX_DATA(CCommissionSetupDlg)
	enum { IDD = IDD_COMMISSION_SETUP_DLG };
	CNxIconButton	m_btnOk;  // (a.wetta 2007-03-29 14:46) - PLID 25407 - Make the buttons match the commission window
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticWndPlaceholder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommissionSetupDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CCommissionSetupWnd *m_CommissionSetupWnd;

	// Generated message map functions
	//{{AFX_MSG(CCommissionSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();;
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMISSIONSETUPDLG_H__F8094C42_408F_43F5_9A60_1B746855A7B9__INCLUDED_)
