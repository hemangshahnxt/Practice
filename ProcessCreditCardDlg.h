#if !defined(AFX_PROCESSCREDITCARDDLG_H__5FA78900_B32E_4F67_A978_D38BC98C1D5C__INCLUDED_)
#define AFX_PROCESSCREDITCARDDLG_H__5FA78900_B32E_4F67_A978_D38BC98C1D5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessCreditCardDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPAYMENTECH_ProcessCreditCardDlg dialog

#include "PaymentechUtils.h"

struct CPaymentSaveInfo;
enum SwipeType;

// (j.gruber 2007-08-03 16:20) - PLID 26620 - credit card processing
// (d.thompson 2009-04-10) - PLID 33957 - Renamed to CPAYMENTECH_ProcessCreditCardDlg for clarity (we now have other types)

class CPAYMENTECH_ProcessCreditCardDlg : public CNxDialog
{
// Construction
public:
	// (a.walling 2007-09-21 15:56) - PLID 27468 - Added the payment info struct
	CPAYMENTECH_ProcessCreditCardDlg(long nPaymentID, CString strCCNumber, CString strCCName, CString CCExpDate, 
		long nCCCardID, COleCurrency cyPayAmount, long nType, long nPatientID, long nProviderID, BOOL bSwiped, 
		SwipeType swType, CString strTrack1, CString strTrack2, CString strTrack3, long nDrawerID, long nLocationID,
		CWnd* pParent, CPaymentSaveInfo* pInfo = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPAYMENTECH_ProcessCreditCardDlg)
	enum { IDD = IDD_PAYMENTECH_PROCESS_CC_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditTotalTransAmt;
	CNxEdit	m_nxeditCcAuthNumber;
	CNxEdit	m_nxeditCcAuthName;
	CNxEdit	m_nxeditCcAuthExpDate;
	CNxEdit	m_nxeditTotalTipAmt;
	CNxEdit	m_nxeditAuthorizationNumber;
	CNxStatic	m_nxstaticMessageLabel;
	CNxStatic	m_nxstaticAuthNumberLabel;
	CNxIconButton m_btnProcess;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAddTip;
	NxButton	m_btnAuthManually;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPAYMENTECH_ProcessCreditCardDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pTipList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCardList;

	CString m_strCCNumber, m_strCCName, m_strCCExpDate;
	long m_nCCCardID;
	long m_nType;
	COleCurrency m_cyPayAmount;
	long m_nPatientID;
	CString m_strPatientName; // (a.walling 2008-07-07 17:16) - PLID 29900 - Cached patient name
	long m_nPaymentID;
	long m_nProviderID;
	BOOL m_bSwiped;
	SwipeType m_swtType;
	CString m_strMagStripe;
	CString m_strTrack1;
	CString m_strTrack2;
	CString m_strTrack3;
	long m_nLocationID;	

	long m_nDrawerID;

	CBrush m_brush;

	CDWordArray m_aryDeleted;

	void OnDeleteTip();
	void UpdateTotalTipAmt();
	void UpdateTotalTransAmt();
	COleCurrency GetChargeTipTotal();

	// (a.walling 2007-09-21 17:08) - PLID 27468 - pointer to a payment info struct
	CPaymentSaveInfo* m_pInfo; 

	enum EApprovalCode {
		IsApproved = 0,
		IsDeclined,
		IsManuallyApproved,
	};

	// (j.gruber 2007-07-03 15:25) - PLID 15416 - CCProcessing
	BOOL ProcessTransaction();
	BOOL SaveChanges();
	PaymentechUtils::TransStruct  m_tsTransaction;
	// (j.gruber 2007-08-02 17:15) - PLID 26650 - customer copy
	BOOL PrintReceipt(PaymentechUtils::ResponseStruct &tsResponse);
	BOOL PrintReceiptManuallyProcessed();
	//TES 12/6/2007 - PLID 28192 - Added the POS Printer as a parameter to these functions, so it can just be claimed
	// once, and then passed into these functions for printing.
	BOOL PrintReceiptHeader(COPOSPrinterDevice *pPOSPrinter, EApprovalCode appCode);
	BOOL PrintReceiptMiddle(COPOSPrinterDevice *pPOSPrinter, CString strOutputMessage);
	BOOL PrintReceiptFooter(COPOSPrinterDevice *pPOSPrinter);
	CString LeftJustify(CString strText, long nLineWidth);
	CString LeftRightJustify(CString strTextLeft, CString strTextRight, long nLineWidth);
	CString CenterLine(CString strText, long nLineWidth);
	BOOL DoAddressVerification();
	void SaveTransaction(PaymentechUtils::ResponseStruct *ptsResponse, long nIsProcessed, CString strExtraAuditMessage);
	// (j.gruber 2007-08-03 10:05) - PLID 26926 - added for auditing
	CString GetDescFromTransactionType(PaymentechUtils::TransactionCode tcCode);
	BOOL CheckFields();
	void HandleOnKillfocusCCNumber();
	
	BOOL m_bIsDeleted;

	CString m_strCardHolderAddress;
	CString m_strCardHolderAddress2;
	CString m_strCardHolderZip;

	BOOL m_bHasChanged;



	// Generated message map functions
	//{{AFX_MSG(CPAYMENTECH_ProcessCreditCardDlg)
	afx_msg void OnProcessCcCard();
	virtual void OnCancel();
	afx_msg void OnAddTipBtn();
	afx_msg void OnTrySetSelFinishedCardTypeList(long nRowEnum, long nFlags);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnRequeryFinishedCardTypeList(short nFlags);
	afx_msg void OnRequeryFinishedCcAuthTipList(short nFlags);
	afx_msg void OnRButtonDownCcAuthTipList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedCcAuthTipList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual void OnOK();
	afx_msg void OnAuthorizeManually();
	afx_msg void OnUpdateCcAuthExpDate();
	afx_msg void OnKillfocusCCNumber();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSCREDITCARDDLG_H__5FA78900_B32E_4F67_A978_D38BC98C1D5C__INCLUDED_)
