#pragma once


// CCreditCardReceiptPromptDlg dialog
// (z.manning 2015-09-08 10:20) - PLID 67226 - Created

class CCreditCardReceiptPromptDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCreditCardReceiptPromptDlg)

public:
	CCreditCardReceiptPromptDlg(long nPaymentID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreditCardReceiptPromptDlg();

// Dialog Data
	enum { IDD = IDD_CREDIT_CARD_RECEIPT_PROMPT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog() override;

	CNxIconButton m_btnPrint;
	CNxIconButton m_btnCancel;

	long m_nPaymentID;

	void UpdateControls();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedMerchantCopyCheck();
	afx_msg void OnBnClickedCustomerCopyCheck();
	afx_msg void OnBnClickedOk();
};
