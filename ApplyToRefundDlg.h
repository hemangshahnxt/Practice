#if !defined(AFX_APPLYTOREFUNDDLG_H__B0F3325C_95F8_475A_9609_5EEA2945634C__INCLUDED_)
#define AFX_APPLYTOREFUNDDLG_H__B0F3325C_95F8_475A_9609_5EEA2945634C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ApplyToRefundDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CApplyToRefundDlg dialog

class CApplyToRefundDlg : public CNxDialog
{
// Construction
public:
	CApplyToRefundDlg(CWnd* pParent);   // standard constructor

	long m_nPayID;
	COleCurrency m_cyPayAmount;
	CBrush m_brush;

	long m_nPatientID; // (a.walling 2008-07-07 17:13) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
// Dialog Data
	//{{AFX_DATA(CApplyToRefundDlg)
	enum { IDD = IDD_APPLY_TO_REFUND };
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApplyToRefundDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pPayList;

	// Generated message map functions
	//{{AFX_MSG(CApplyToRefundDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellAvalPaysList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLYTOREFUNDDLG_H__B0F3325C_95F8_475A_9609_5EEA2945634C__INCLUDED_)
