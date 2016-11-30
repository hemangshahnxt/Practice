#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "QBMSProcessingUtils.h"
#include "ChaseProcessingUtils.h"
#include "PaymentechUtils.h"
// (d.lange 2010-09-01 11:41) - PLID 40310 - A complete copy of QBMS_ProcessCreditCardDlg, but with a more generic name
// CCreditCardProcessingDlg dialog
struct CPaymentSaveInfo;

class CCreditCardProcessingDlg : public CNxDialog
{

public:
	// (a.walling 2007-09-21 15:56) - PLID 27468 - Added the payment info struct
	// (d.thompson 2009-04-16) - PLID 33957 - Added Payment Trans ID.  This is only used if the type is
	//	refund.  It will specify the AnaTransID of the payment you want to refund.  ApprovalCode as well.
	CCreditCardProcessingDlg(long nPaymentID, CString strCCNumber, CString strCCName, CString CCExpDate, 
		long nCCCardID, COleCurrency cyPayAmount, long nType, long nPatientID, long nProviderID, 
		long nDrawerID, long nLocationID, bool bCCSwiped, CString strTrack2Data, CWnd* pParent, CPaymentSaveInfo* pInfo,
		bool bIsNew, CString strApplyToPaymentTransID);

// Dialog Data
	enum { IDD = IDD_PROCESS_CC_DLG };

	CNxEdit	m_nxeditTotalTransAmt;
	CNxEdit	m_nxeditCcAuthNumber;
	CNxEdit	m_nxeditCcAuthName;
	CNxEdit	m_nxeditCcAuthExpDate;
	CNxEdit	m_nxeditTotalTipAmt;
	CNxEdit m_nxeditCcSecurityCode;
	CNxEdit m_nxeditBillingAddress;
	CNxEdit m_nxeditBillingZip;
	CNxStatic	m_nxstaticMessageLabel;
	CNxIconButton m_btnProcess;
	CNxIconButton m_btnSaveAndProcessLater;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAddTip;
	CNxIconButton m_btnKeyboardCardSwipe; // (b.spivey, October 04, 2011) - PLID 40567 - Keyboard card swipe button. 
	NxButton m_btnIsCardPresent;
	NxButton m_btnCardIsSwiped;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQBMS_ProcessCreditCardDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pTipList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCardList;
	// (d.thompson 2010-11-08) - PLID 41367 - Added account list
	NXDATALIST2Lib::_DNxDataListPtr m_pAccountList;
	CString m_strCCNumber, m_strCCName, m_strCCExpDate;
	long m_nCCCardID;
	long m_nType;
	COleCurrency m_cyPayAmount;
	long m_nPatientID;
	CString m_strPatientName; // (a.walling 2008-07-07 17:16) - PLID 29900 - Cached patient name
	long m_nPaymentID;
	long m_nProviderID;
	long m_nLocationID;	
	bool m_bIsSwiped;
	CString m_strTrack2Data;
	long m_nDrawerID;
	CBrush m_brush;
	CDWordArray m_aryDeleted;
	void OnDeleteTip();
	void UpdateTotalTipAmt();
	void UpdateTotalTransAmt();
	COleCurrency GetChargeTipTotal();
	bool m_bIsNew;
	CString m_strApplyToPaymentTransID;

	// (a.walling 2007-09-21 17:08) - PLID 27468 - pointer to a payment info struct
	CPaymentSaveInfo* m_pInfo; 

	// (d.thompson 2010-09-02) - PLID 40351 - Split out to handle QBMS and Chase both
	bool CCreditCardProcessingDlg::GetDialogData(OUT CString &strCleanCCNumber, OUT CString &strExpDate, OUT CString &strMM, OUT CString &strYY, 
											 OUT COleCurrency &cyAmount, OUT CString &strNameOnCard, OUT CString &strSecurityCode, 
											 OUT CString &strAddress, OUT CString &strZip, OUT bool &bCardPresent);

	// (d.thompson 2010-11-08) - PLID 41367
	void LoadAccountsList();
	// (d.thompson 2010-11-08) - PLID 41367
	void SelectDefaultAccount();
	// (j.gruber 2007-07-03 15:25) - PLID 15416 - CCProcessing
	// (d.thompson 2010-09-02) - PLID 40371 - Renamed to denote QBMS-specific function
	BOOL QBMS_ProcessTransaction();
	// (d.thompson 2010-09-07) - PLID 40351 - Chase-specific, similar to qbms-specific one
	BOOL Chase_ProcessTransaction();
	BOOL SaveChanges();
	PaymentechUtils::TransStruct  m_tsTransaction;
	// (j.gruber 2007-08-02 17:15) - PLID 26650 - customer copy
	// (d.thompson 2011-01-05) - PLID 41991 - Added sale type and is swiped flag.
	BOOL CCreditCardProcessingDlg::PrintReceipt(CString strApprovalCode, CString strSaleType, bool bIsSwiped);
	//TES 12/6/2007 - PLID 28192 - Added the POS Printer as a parameter to these functions, so it can just be claimed
	// once, and then passed into these functions for printing.
	BOOL PrintReceiptHeader(COPOSPrinterDevice *pPOSPrinter);
	// (d.thompson 2011-01-05) - PLID 41994 - Added sale type parameter.  Prints on output.
	BOOL PrintReceiptMiddle(COPOSPrinterDevice *pPOSPrinter, CString strOutputMessage, CString strSaleType);
	BOOL PrintReceiptFooter(COPOSPrinterDevice *pPOSPrinter);
	CString LeftJustify(CString strText, long nLineWidth);
	CString LeftRightJustify(CString strTextLeft, CString strTextRight, long nLineWidth);
	CString CenterLine(CString strText, long nLineWidth);
	// (d.thompson 2010-09-02) - PLID 40371 - Renamed to denote QBMS-specific behavior
	// (d.thompson 2010-11-08) - PLID 41367 - Added nAccountID for multi-account support
	bool QBMS_SaveTransaction(bool bApproved, QBMS_TransTypes qbTransType, CString strCreditCardTransID, CString strAuthorizationCode, CString strAVSStreet, 
		CString strAVSZip, CString strCardSecurityCodeMatch, CString strPaymentStatus, COleDateTime dtTxnAuthorizationTime, long nAccountID);
	// (d.thompson 2010-09-07) - PLID 40351 - Implemented Chase-specific functions
	// (d.thompson 2010-11-11) - PLID 40351 - Added AccountID
	// (d.thompson 2010-12-20) - PLID 41895 - added auth code
	bool Chase_SaveTransaction(bool bApproved, ChaseProcessingUtils::Chase_TransTypes cpTransType, CString strOrderID, 
							CString strTxRefNum, COleDateTime dtProcDateTime, CString strProcStatus, CString strApprovalStatus, 
							CString strRespCode, CString strAVSRespCode, CString strCVVRespCode, CString strRetryTrace, 
							CString strRetryAttemptCount, COleCurrency cyReversalAmtVoided, COleCurrency cyReversalOutstandingAmt, 
							long nAccountID, CString strAuthorizationCode);
	BOOL CheckFields();
	void HandleOnKillfocusCCNumber();
	CString m_strCardHolderAddress;
	CString m_strCardHolderAddress2;
	CString m_strCardHolderZip;
	BOOL m_bHasChanged;
	void OfferSaveForLater(CString strPreText);
	bool GetTotalTransactionAmount(OUT COleCurrency& cyAmount);
	void DisableAfterSwipe();
	// (d.thompson 2010-09-02) - PLID 40371 - Renamed to denote QBMS-specific behavior
	QBMS_TransTypes QBMS_GetCurrentTransType();
	// (d.thompson 2010-09-07) - PLID 40351 - Implemented chase-specific behavior
	ChaseProcessingUtils::Chase_TransTypes Chase_GetCurrentTransType();
	// (d.thompson 2013-06-06) - PLID 56334
	void CloseAfterSuccess();
	// (d.thompson 2013-06-06) - PLID 56334
	void SaveLastUsedAccount();

	// Generated message map functions
	//{{AFX_MSG(CQBMS_ProcessCreditCardDlg)
	afx_msg void OnProcessCcCard();
	virtual void OnCancel();
	afx_msg void OnAddTipBtn();
	afx_msg void OnTrySetSelFinishedCardTypeList(long nRowEnum, long nFlags);
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedCardTypeList(short nFlags);
	afx_msg void OnRequeryFinishedCcAuthTipList(short nFlags);
	afx_msg void OnRButtonDownCcAuthTipList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedCcAuthTipList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual void OnOK();
	afx_msg void OnUpdateCcAuthExpDate();
	afx_msg void OnKillfocusCCNumber();
	
	//}}AFX_MSG
	
public:
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedSaveAndProcessLater();
	void SelChosenAccountsList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedKeyboardCardSwipe();
};
