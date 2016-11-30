#if !defined(AFX_FINANCECHARGESDLG_H__E3C46F4F_4DCE_455E_9F8D_5F7C4205F8CB__INCLUDED_)
#define AFX_FINANCECHARGESDLG_H__E3C46F4F_4DCE_455E_9F8D_5F7C4205F8CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FinanceChargesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFinanceChargesDlg dialog

class CFinanceChargesDlg : public CNxDialog
{
// Construction
public:
	CFinanceChargesDlg(CWnd* pParent);   // standard constructor

	long AddFinanceCharge(long nServiceID, BOOL bIncrementExistingFinanceCharges, long nPatientID, long nBillID, long nProviderID, long nLocationID, COleCurrency cyTotalFCAmount);

	ADODB::_RecordsetPtr GetOverdueChargeRecordset(long nChargeType, long nDaysUntilOverdue, long nChargeInterestDayInterval, BOOL bCompoundExistingFinanceCharges);

	// (j.jones 2009-06-11 16:05) - PLID 34577 - removed m_strTempTableName because
	// we no longer filter on patients prior to this dialog being created
	//CString m_strTempTableName;

// Dialog Data
	//{{AFX_DATA(CFinanceChargesDlg)
	enum { IDD = IDD_FINANCE_CHARGES_DLG };
	CProgressCtrl	m_progress;
	CNxEdit	m_nxeditProgressBarStatus;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFinanceChargesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFinanceChargesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINANCECHARGESDLG_H__E3C46F4F_4DCE_455E_9F8D_5F7C4205F8CB__INCLUDED_)
