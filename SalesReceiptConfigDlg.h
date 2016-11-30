#if !defined(AFX_SALESRECEIPTCONFIGDLG_H__177F0A0F_3284_43AC_9EF0_D2C1E631A2D6__INCLUDED_)
#define AFX_SALESRECEIPTCONFIGDLG_H__177F0A0F_3284_43AC_9EF0_D2C1E631A2D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SalesReceiptConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSalesReceiptConfigDlg dialog
// (j.gruber 2007-03-15 13:36) - PLID 25020 - class created
class CSalesReceiptConfigDlg : public CNxDialog
{
// Construction
public:
	// (j.gruber 2007-03-29 14:43) - PLID 9802 - receipt printer 
	// (a.walling 2008-07-07 17:18) - PLID 29900 - included patientID
	CSalesReceiptConfigDlg(COleDateTime dtPay, long nNumber, long nPaymentID, BOOL bPreview, long nReportID, long nPatientID, CWnd* pParent);   // standard constructor  

	// (j.gruber 2007-03-15 14:47) - PLID 25223 - need to account for this being run from the right click
	
	// (a.walling 2008-07-07 17:18) - PLID 29900 - included patientID
	CSalesReceiptConfigDlg(COleDateTime dtPay, long nPaymentID, BOOL bPreview, long nReportID, long nPatientID, CWnd* pParent);   

// Dialog Data
	//{{AFX_DATA(CSalesReceiptConfigDlg)
	enum { IDD = IDD_SALES_RECEIPT_CONFIG };
	CNxIconButton	m_btnSalesReceiptPrint;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSalesReceiptConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	

	COleDateTime m_dtPay;
	long m_nNumber;
	BOOL m_bPreview;
	long m_nPayID;
	long m_nReportID;
	long m_nPatientID; // (a.walling 2008-07-07 17:18) - PLID 29900 - Stored patientID

	NXDATALIST2Lib::_DNxDataListPtr m_pCharges;
	NXDATALIST2Lib::_DNxDataListPtr m_pPayments;

	CString GenerateIDList();

	// (j.gruber 2007-07-16 10:38) - PLID 26686 - all sales receipt printer functionality moved to GlobalReportUitls
		
	// Generated message map functions
	//{{AFX_MSG(CSalesReceiptConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedSalesReceiptPayments(short nFlags);
	afx_msg void OnRequeryFinishedSalesReceiptCharges(short nFlags);
	afx_msg void OnSalesReceiptPrint();
	virtual void OnCancel();
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SALESRECEIPTCONFIGDLG_H__177F0A0F_3284_43AC_9EF0_D2C1E631A2D6__INCLUDED_)
