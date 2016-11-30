#if !defined(AFX_REFUNDEDBATCHPAYSDLG_H__DEE859BF_5EB8_4B9F_88FD_C748FE17A767__INCLUDED_)
#define AFX_REFUNDEDBATCHPAYSDLG_H__DEE859BF_5EB8_4B9F_88FD_C748FE17A767__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RefundedBatchPaysDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRefundedBatchPaysDlg dialog

class CRefundedBatchPaysDlg : public CNxDialog
{
// Construction
public:
	CRefundedBatchPaysDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_RefundedPaysList;

	// (j.jones 2008-05-08 10:44) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CRefundedBatchPaysDlg)
	enum { IDD = IDD_REFUNDED_BATCH_PAYS_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnUnapplyItem;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRefundedBatchPaysDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRefundedBatchPaysDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnUnapplyRefundedBatchPay();
	afx_msg void OnDblClickCellRefundedBatchPaymentsList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFUNDEDBATCHPAYSDLG_H__DEE859BF_5EB8_4B9F_88FD_C748FE17A767__INCLUDED_)
