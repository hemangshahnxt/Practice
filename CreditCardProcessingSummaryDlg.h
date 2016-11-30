#if !defined(AFX_CREDITCARDPROCESSINGSUMMARYDLG_H__B8F15B5D_1BA1_4BF2_A0CD_E83293F4EA91__INCLUDED_)
#define AFX_CREDITCARDPROCESSINGSUMMARYDLG_H__B8F15B5D_1BA1_4BF2_A0CD_E83293F4EA91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CreditCardProcessingSummaryDlg.h : header file
//
#include "PaymentechUtils.h"
/////////////////////////////////////////////////////////////////////////////
// CCreditCardProcessingSummaryDlg dialog
// (j.gruber 2007-08-22 10:47) - PLID 26584 - created for
class CCreditCardProcessingSummaryDlg : public CNxDialog
{
// Construction
public:
	CCreditCardProcessingSummaryDlg(CWnd* pParent);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CCreditCardProcessingSummaryDlg)
	enum { IDD = IDD_CREDIT_CARD_PROCESSING_SUMMARY };
	CNxStatic	m_NotProcessedBlock;
	CNxStatic	m_NotInBatchBlock;
	CNxStatic	m_ManuallyApprovedBlock;
	CNxStatic	m_ApprovedBlock;
	CNxEdit	m_nxeditBatchNumber;
	CNxStatic	m_nxstaticBatchStatus;
	NxButton	m_btnGeneratedTotalsGroupbox;
	NxButton	m_btnPaymentechTotalsGroupbox;
	NxButton	m_btnLegendGroupbox;
	CNxIconButton	m_btnCommitBatch;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreditCardProcessingSummaryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL SendBatchInquiry();
	void OnVoidTransaction();
	void OnProcessTransaction();
	BOOL VoidPayment(long nPaymentID, long &nVoidRetRefNum);
	BOOL ProcessTransaction(long nPaymentID, PaymentechUtils::ResponseStruct &rsResp, PaymentechUtils::TransactionCode &tcCode);
	void UpdateScreen();
	long GetSortFromPayType(CString strPayType);
	BOOL CheckBatch();
	// (j.gruber 2007-08-03 09:34) - PLID 26926 - added for auditing
	CString GetDescFromTransactionType(PaymentechUtils::TransactionCode tcCode);

	CString m_strBatchID;


	NXDATALIST2Lib::_DNxDataListPtr m_pTransList;
	NXDATALIST2Lib::_DNxDataListPtr m_pGeneratedList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPaymentechList;

	NXTIMELib::_DNxTimePtr m_pdtBatchClose;
	NXTIMELib::_DNxTimePtr m_pdtBatchOpen;

	CBrush m_brushNotProcessed;
	CBrush m_brushManuallyApproved;
	CBrush m_brushApproved;
	CBrush m_brushNotInBatch;



	// Generated message map functions
	//{{AFX_MSG(CCreditCardProcessingSummaryDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnRButtonUpTransactionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnCommitBatch();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedTransactionList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CREDITCARDPROCESSINGSUMMARYDLG_H__B8F15B5D_1BA1_4BF2_A0CD_E83293F4EA91__INCLUDED_)
