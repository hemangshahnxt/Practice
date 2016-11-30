#if !defined(AFX_QBDEPOSITACCOUNTSDLG_H__F1E365CD_53F0_4DB7_8B96_FC1D05AC8F49__INCLUDED_)
#define AFX_QBDEPOSITACCOUNTSDLG_H__F1E365CD_53F0_4DB7_8B96_FC1D05AC8F49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QBDepositAccountsDlg.h : header file
//

#include "QuickbooksUtils.h"


struct QBSourceAccounts {

	long ID;
	CString strAcctID;
	CString strAcctName;
};

/////////////////////////////////////////////////////////////////////////////
// CQBDepositAccountsDlg dialog

class CQBDepositAccountsDlg : public CNxDialog
{
// Construction
public:
	CQBDepositAccountsDlg(CWnd* pParent);   // standard constructor
	~CQBDepositAccountsDlg();	//destructor

	CPtrArray *m_paryPaymentIDs;
	CPtrArray *m_paryRefundIDs;	// (j.jones 2009-02-18 08:49) - PLID 33136
	CPtrArray *m_paryBatchPaymentIDs;
	CPtrArray *m_paryPaymentTipIDs;

	NXDATALISTLib::_DNxDataListPtr m_SourceAccountList;
	NXDATALISTLib::_DNxDataListPtr m_DestAccountCombo;

	IQBSessionManagerPtr qb;

	CPtrArray m_parySourceAccounts;
	CString m_strToAccount;

	// (j.jones 2008-05-08 10:42) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CQBDepositAccountsDlg)
	enum { IDD = IDD_QB_DEPOSIT_ACCOUNTS_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQBDepositAccountsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQBDepositAccountsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QBDEPOSITACCOUNTSDLG_H__F1E365CD_53F0_4DB7_8B96_FC1D05AC8F49__INCLUDED_)
