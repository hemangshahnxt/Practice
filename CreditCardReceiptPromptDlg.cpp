// CreditCardReceiptPromptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillingRc.h"
#include "CreditCardReceiptPromptDlg.h"
#include "GlobalFinancialUtils.h"


// CCreditCardReceiptPromptDlg dialog
// (z.manning 2015-09-08 10:20) - PLID 67226 - Created

IMPLEMENT_DYNAMIC(CCreditCardReceiptPromptDlg, CNxDialog)

CCreditCardReceiptPromptDlg::CCreditCardReceiptPromptDlg(long nPaymentID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CCreditCardReceiptPromptDlg::IDD, pParent)
	, m_nPaymentID(nPaymentID)
{

}

CCreditCardReceiptPromptDlg::~CCreditCardReceiptPromptDlg()
{
}

void CCreditCardReceiptPromptDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnPrint);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CCreditCardReceiptPromptDlg, CNxDialog)
	ON_BN_CLICKED(IDC_MERCHANT_COPY_CHECK, &CCreditCardReceiptPromptDlg::OnBnClickedMerchantCopyCheck)
	ON_BN_CLICKED(IDC_CUSTOMER_COPY_CHECK, &CCreditCardReceiptPromptDlg::OnBnClickedCustomerCopyCheck)
	ON_BN_CLICKED(IDOK, &CCreditCardReceiptPromptDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CCreditCardReceiptPromptDlg message handlers

BOOL CCreditCardReceiptPromptDlg::OnInitDialog()
{
	try
	{
		__super::OnInitDialog();

		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnCancel.AutoSet(NXB_CANCEL);

		ADODB::_RecordsetPtr prsPayment = CreateParamRecordset(R"(
SELECT CT.WasSignedElectronically
FROM CardConnect_CreditTransactionT CT
WHERE CT.ID = {INT}
)"
, m_nPaymentID);
		BOOL bWasSignedElectronically = FALSE;
		if (!prsPayment->eof) {
			bWasSignedElectronically = AdoFldBool(prsPayment, "WasSignedElectronically");
		}

		if (!bWasSignedElectronically) {
			CheckDlgButton(IDC_MERCHANT_COPY_CHECK, BST_CHECKED);
		}
		CheckDlgButton(IDC_CUSTOMER_COPY_CHECK, BST_CHECKED);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CCreditCardReceiptPromptDlg::UpdateControls()
{
	BOOL bMerchantCopy = (IsDlgButtonChecked(IDC_MERCHANT_COPY_CHECK) == BST_CHECKED);
	BOOL bCustomperCopy = (IsDlgButtonChecked(IDC_CUSTOMER_COPY_CHECK) == BST_CHECKED);

	m_btnPrint.EnableWindow(bMerchantCopy || bCustomperCopy);
}

void CCreditCardReceiptPromptDlg::OnBnClickedMerchantCopyCheck()
{
	try
	{
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}

void CCreditCardReceiptPromptDlg::OnBnClickedCustomerCopyCheck()
{
	try
	{
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}

void CCreditCardReceiptPromptDlg::OnBnClickedOk()
{
	try
	{
		BOOL bMerchantCopy = (IsDlgButtonChecked(IDC_MERCHANT_COPY_CHECK) == BST_CHECKED);
		BOOL bCustomperCopy = (IsDlgButtonChecked(IDC_CUSTOMER_COPY_CHECK) == BST_CHECKED);

		PrintICCPReceipts(this, m_nPaymentID, bMerchantCopy, bCustomperCopy);

		__super::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}
