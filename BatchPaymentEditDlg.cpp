// BatchPaymentEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BatchPaymentEditDlg.h"
#include "GlobalFinancialUtils.h"
#include "EditComboBox.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalDrawingUtils.h"
#include "PayCatDlg.h" // (j.gruber 2012-11-15 13:55) - PLID 53752
#include "FinancialRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CBatchPaymentEditDlg dialog

// (j.jones 2008-07-11 13:51) - PLID 28756 - added enum for the insurance combo
enum InsuranceComboColumns {
	iccID = 0,
	iccName,
	iccAddress,
	iccDefaultPayDesc,
	iccDefaultPayCategoryID,
	iccDefaultAdjDesc,
	iccDefaultAdjCategoryID
};

CBatchPaymentEditDlg::CBatchPaymentEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBatchPaymentEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBatchPaymentEditDlg)
		m_ID = -1;
		m_nType = 1;
		// (j.jones 2014-06-26 17:23) - PLID 62546 - added PayType
		m_ePayType = eMedicalPayment;
		m_nAppliedPayID = -1;
		m_cyMaxApplyAmt = COleCurrency(999999,99);
		m_bIsInUse = FALSE;
		m_bBatchDateChanged = FALSE;
		m_strBatchDate = "";
	//}}AFX_DATA_INIT
}


void CBatchPaymentEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchPaymentEditDlg)
	DDX_Control(pDX, IDC_BATCH_PAY_CAT, m_nxstaticPayCat);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PAY_DATE, m_dtPaymentDate);
	DDX_Control(pDX, IDC_BATCH_PAY_COLOR, m_bkg);
	DDX_Control(pDX, IDC_EDIT_TOTAL, m_nxeditEditTotal);
	DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_nxeditEditDescription);
	DDX_Control(pDX, IDC_BANK_NAME, m_nxeditBankName);
	DDX_Control(pDX, IDC_BANK_NO, m_nxeditBankNo);
	DDX_Control(pDX, IDC_CHECK_NO, m_nxeditCheckNo);
	DDX_Control(pDX, IDC_CHECK_ACCT_NO, m_nxeditCheckAcctNo);
	DDX_Control(pDX, IDC_CURRENCY_SYMBOL_BATCHPAY, m_nxstaticCurrencySymbolBatchpay);	
	DDX_Control(pDX, IDC_ORIGINAL_CURRENCY_SYMBOL_BATCHPAY, m_nxstaticOriginalCurrencySymbolBatchpay);
	DDX_Control(pDX, IDC_LABEL_ORIGINAL_AMOUNT, m_nxstaticOriginalAmountLabel);
	DDX_Control(pDX, IDC_EDIT_ORIGINAL_TOTAL, m_nxeditOriginalAmount);
	DDX_Control(pDX, IDC_CHECK_CAPITATION, m_checkCapitation);
	DDX_Control(pDX, IDC_SERVICE_DATE_FROM, m_dtServiceDateFrom);
	DDX_Control(pDX, IDC_SERVICE_DATE_TO, m_dtServiceDateTo);
	DDX_Control(pDX, IDC_LABEL_SERVICE_DATE, m_nxstaticLabelServiceDate);
	DDX_Control(pDX, IDC_LABEL_SERVICE_DATE_TO, m_nxstaticLabelServiceDateTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBatchPaymentEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EDIT_PAY_CAT, OnEditPayCat)
	ON_BN_CLICKED(IDC_EDIT_PAY_DESC, OnEditPayDesc)
	ON_BN_CLICKED(IDC_CHECK_CAPITATION, OnCheckCapitation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchPaymentEditDlg message handlers

BEGIN_EVENTSINK_MAP(CBatchPaymentEditDlg, CNxDialog)
	ON_EVENT(CBatchPaymentEditDlg, IDC_DESCRIPTION_COMBO, 16 /* SelChosen */, OnSelChosenDescriptionCombo, VTS_I4)
	ON_EVENT(CBatchPaymentEditDlg, IDC_LOCATIONS, 20 /* TrySetSelFinished */, OnTrySetSelFinishedLocations, VTS_I4 VTS_I4)
	ON_EVENT(CBatchPaymentEditDlg, IDC_PROVIDER_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedProviderCombo, VTS_I4 VTS_I4)
	ON_EVENT(CBatchPaymentEditDlg, IDC_INSURANCE_COMPANIES, 16 /* SelChosen */, OnSelChosenInsuranceCompanies, VTS_I4)
END_EVENTSINK_MAP()

BOOL CBatchPaymentEditDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-07 11:07) - PLID 29854 - added nxiconbuttons for modernization
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//TES 5/28/2008 - PLID 26189 - bulk cache our configrt calls.
	g_propManager.CachePropertiesInBulk("BatchPaymentEditDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'DisablePaymentDate' OR "
		"Name = 'BatchPay_DefaultCategory' OR "
		"Name = 'BatchPay_DefaultDescription' "
		// (j.jones 2011-07-08 16:58) - PLID 44497 - added RequireProviderOnPayments
		"OR Name = 'RequireProviderOnPayments' "
		// (j.jones 2015-10-26 14:03) - PLID 67462 - added pref. to enable capitation payments
		"OR Name = 'BatchPayments_EnableCapitation' "
		")",
		_Q(GetCurrentUserName()));

	// (j.jones 2006-12-07 12:47) - PLID 19467 - added option to disable payment date changing
	long nDisablePaymentDate = GetRemotePropertyInt("DisablePaymentDate", 0, 0, "<None>", true);
	if(nDisablePaymentDate == 1) {
		//simply disable the ability to change the payment date,
		//the code will fill it any number of ways, however
		GetDlgItem(IDC_PAY_DATE)->EnableWindow(FALSE);
	}

	if(m_nType == 3) {
		m_color = 0x0066CCCC;
		m_bkg.SetColor(m_color);
		SetWindowText("Batch Payment Refund");
		// (j.jones 2008-09-08 13:31) - PLID 26689 - added m_nxstaticPayCat
		m_nxstaticPayCat.SetWindowText("Refund Category");
	}
	else if(m_nType == 2) {
		m_color = 0xFFB9A8;
		m_bkg.SetColor(m_color);
		SetWindowText("Batch Payment Adjustment");
		// (j.jones 2008-09-08 13:31) - PLID 26689 - added m_nxstaticPayCat
		m_nxstaticPayCat.SetWindowText("Adjustment Category");
	}
	else {
		//payment
		m_color = 0x0085A43C;	

		//for Payments, the window text will change in Load()
	}

	m_brush.CreateSolidBrush(PaletteColor(m_color));

	CString strICurr;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, strICurr.GetBuffer(2), 2, TRUE);
	strICurr.ReleaseBuffer();
	strICurr.TrimRight();
	if(strICurr == "2") {
		SetDlgItemText(IDC_CURRENCY_SYMBOL_BATCHPAY,GetCurrencySymbol() + " ");
		// (j.jones 2009-06-26 12:11) - PLID 33856 - added original amount
		SetDlgItemText(IDC_ORIGINAL_CURRENCY_SYMBOL_BATCHPAY, GetCurrencySymbol() + " ");
	}
	else {
		SetDlgItemText(IDC_CURRENCY_SYMBOL_BATCHPAY,GetCurrencySymbol());
		// (j.jones 2009-06-26 12:11) - PLID 33856 - added original amount
		SetDlgItemText(IDC_ORIGINAL_CURRENCY_SYMBOL_BATCHPAY, GetCurrencySymbol());		
	}
	
	m_InsuranceCoCombo = BindNxDataListCtrl(this,IDC_INSURANCE_COMPANIES,GetRemoteData(),true);
	m_DescriptionCombo = BindNxDataListCtrl(this,IDC_DESCRIPTION_COMBO,GetRemoteData(),true);
	m_LocationCombo = BindNxDataListCtrl(this,IDC_LOCATIONS,GetRemoteData(),true);
	m_PayCatsCombo = BindNxDataListCtrl(this,IDC_PAY_CATEGORY,GetRemoteData(),true);
	m_DoctorCombo = BindNxDataListCtrl(this,IDC_PROVIDER_COMBO,GetRemoteData(),true);

	IRowSettingsPtr pRow;
	pRow = m_DoctorCombo->GetRow(-1);
	pRow->PutValue(0,long(-1));
	pRow->PutValue(1,_bstr_t("<No Provider Selected>"));
	m_DoctorCombo->InsertRow(pRow,0);

	pRow = m_PayCatsCombo->GetRow(-1);
	pRow->PutValue(0,long(0));
	pRow->PutValue(1,_bstr_t("<No Category Selected>"));
	m_PayCatsCombo->AddRow(pRow);

	Load();	

	// (j.jones 2009-06-09 15:41) - PLID 34549 - filter on payments only
	// (j.jones 2009-06-10 17:48) - PLID 34592 - disallow editing if there is another batch payment
	// (j.jones 2011-06-24 13:03) - PLID 41863 - we now allow editing the amount, but need to track later
	// if we need to validate saving changes to the amount
	m_bIsInUse = FALSE;
	if(m_ID != -1) {
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 PaymentsT.ID FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID WHERE BatchPaymentID = {INT} AND Deleted = 0 AND Type <> 2"
			""
			"SELECT TOP 1 ID FROM BatchPaymentsT WHERE Deleted = 0 AND AppliedBatchPayID = {INT}", m_ID, m_ID);		
		if(!rs->eof) {
			AfxMessageBox("You have already begun to apply this payment to patient accounts. Some of these fields cannot be changed.");
			m_bIsInUse = TRUE;
		}

		// (j.jones 2009-06-10 17:48) - PLID 34592 - disallow editing if there is another batch payment
		rs = rs->NextRecordset(NULL);

		if(!rs->eof && !m_bIsInUse) {
			AfxMessageBox("This batch payment currently has applied adjustments or refunds. Some of these fields may not be changed.");
			m_bIsInUse = TRUE;
		}
		rs->Close();
	}

	if(m_bIsInUse) {
		DisableItems_BatchPayInUse();
	}
	else {
		((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetSel(0, -1);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBatchPaymentEditDlg::OnOK() 
{
	if(!Save())
		return;
	
	CDialog::OnOK();
}

void CBatchPaymentEditDlg::OnSelChosenDescriptionCombo(long nRow) 
{
	try {
		if(nRow==-1)
			return;

		CString str;
		str = CString(m_DescriptionCombo->GetValue(nRow,0).bstrVal);
		GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText(str);
	}NxCatchAll(__FUNCTION__);
}

void CBatchPaymentEditDlg::OnEditPayCat() 
{
	try {
		_variant_t value;
		long curSel = m_PayCatsCombo->CurSel;
		if (curSel != -1)
			value = m_PayCatsCombo->Value[curSel][0];

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// (j.gruber 2012-11-15 13:55) - PLID 53752 - change the dialog
		//CEditComboBox(this, 3, m_PayCatsCombo, "Edit Combo Box").DoModal();			
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			m_PayCatsCombo->Requery();			
			m_PayCatsCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}

		IRowSettingsPtr pRow;
		pRow = m_PayCatsCombo->GetRow(-1);
		pRow->PutValue(0,long(0));
		pRow->PutValue(1,_bstr_t("<No Category Selected>"));
		m_PayCatsCombo->AddRow(pRow);

		if (curSel != -1)
			m_PayCatsCombo->SetSelByColumn(0, value);
	}NxCatchAll(__FUNCTION__);
}

void CBatchPaymentEditDlg::OnEditPayDesc() 
{
	try {
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 2, m_DescriptionCombo, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}

BOOL CBatchPaymentEditDlg::Save()
{
	try {
		BOOL bIsNew = TRUE;

		CString str, strAmount, strDescription, strCheckNo, strCheckAcctNo, strBankName, strBankRoutingNum;

		long InsCo, LocationID, ProviderID = -1, PaymentGroupID = 0;

		_variant_t var;

		if(m_ID == -1)
			bIsNew = TRUE;
		else
			bIsNew = FALSE;

		//get the data

		//Amount

		////////////////////////////////////////////////////////
		// Make sure total amount is not blank, is less than
		// the maximum allowable amount, has no bad characters,
		// and has no more than two places to the right of the
		// decimal point.
		////////////////////////////////////////////////////////
		GetDlgItem(IDC_EDIT_TOTAL)->GetWindowText(str);
		if (str.GetLength() == 0) {
			MsgBox("Please fill in the 'Total Amount' box.");
			return FALSE;
		}
		// (j.jones 2012-02-13 15:36) - PLID 48121 - negative applies are now permitted for adjustments
		if(!IsValidCurrencyText(str)) {
			MsgBox("Please fill correct information in the 'Total Amount' box.");
			return FALSE;
		}

		COleCurrency cy = ParseCurrencyFromInterface(str);

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cy.Format().Find(strDecimal) != -1 && cy.Format().Find(strDecimal) + (nDigits+1) < cy.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the 'Total Amount' box.",nDigits,strDecimal == "," ? "comma" : "decimal");
			return FALSE;
		}
		
		// (j.jones 2004-06-30 14:01) - changed to allow $0.00 batch payments
		// (j.jones 2012-02-13 15:39) - PLID 48121 - adjustments can be negative only if
		// the balance of the batch pament (m_cyMaxApplyAmt) is negative
		// (j.jones 2013-07-02 16:37) - PLID 57420 - made these warnings actually stop saving
		if(m_nType == 3 && cy <= COleCurrency(0,0)) {
			MsgBox("You cannot enter a refund that is negative or zero.");
			return FALSE;
		}
		else if(m_nType == 2 && cy == COleCurrency(0,0)) {
			MsgBox("You cannot enter a %s adjustment.", FormatCurrencyForInterface(cy));
			return FALSE;
		}
		else if(m_nType == 2 && cy < COleCurrency(0,0) && m_cyMaxApplyAmt >= COleCurrency(0,0)) {
			MsgBox("You cannot enter a negative adjustment unless the batch payment has a negative balance.", FormatCurrencyForInterface(cy));
			return FALSE;
		}
		else if(m_nType == 1 && cy < COleCurrency(0,0)) {
			MsgBox("You cannot enter a negative batch payment.");
			return FALSE;
		}

		// (j.jones 2015-10-08 13:01) - PLID 67307 - only payments can be capitation
		bool bCapitation = false;
		_variant_t varServiceDateFrom = g_cvarNull;
		_variant_t varServiceDateTo = g_cvarNull;
		if (m_nType == 1 && m_checkCapitation.GetCheck()) {
			COleDateTime dtFrom = (COleDateTime)m_dtServiceDateFrom.GetValue();
			COleDateTime dtTo = (COleDateTime)m_dtServiceDateTo.GetValue();

			//verify the dates are valid - they can be in the future (though that is stupid),
			//and they can be the same dates, they just can't be backwards
			if (dtFrom > dtTo) {
				MsgBox("The capitation Service Date From is later than the Service Date To. Please correct these dates before saving.");
				return FALSE;
			}

			bCapitation = true;
			varServiceDateFrom = _variant_t(dtFrom, VT_DATE);
			varServiceDateTo = _variant_t(dtTo, VT_DATE);

			//yell if the payment is zero
			if (cy <= COleCurrency(0, 0)) {
				MsgBox("Capitation payments must have an amount greater than zero.");
				return FALSE;
			}
		}

		//now check to see if we are refunding / adjusting more than the batch payment's balance
		if(m_nType != 1) {
			// (j.jones 2012-02-13 15:43) - PLID 48121 - m_cyMaxApplyAmt will be negative if there is a negative balance
			// on the batch payment, in that case we cannot apply a negative adjustment greater than that balance.
			// In all cases you can only apply in such a way that brings the balance towards zero, never beyond zero, in either direction.
			if((m_cyMaxApplyAmt >= COleCurrency(0,0) && cy > m_cyMaxApplyAmt)
				||
				(m_cyMaxApplyAmt < COleCurrency(0,0) && (cy < m_cyMaxApplyAmt || cy > COleCurrency(0,0)))) {
				str.Format("The balance of the batch payment is %s. You may not refund more than this amount.", FormatCurrencyForInterface(m_cyMaxApplyAmt,TRUE,TRUE));
				if(m_nType == 2)
					str.Replace("refund", "adjust");
				MsgBox(str);
				return FALSE;
			}
		}

		// (j.jones 2011-06-24 13:06) - PLID 41863 - if the batch payment is already applied,
		// we let them change the amount, but we cannot make the new balance negative
		// (j.jones 2012-02-13 15:53) - PLID 48121 - it might already be negative, if so,
		// don't do anything if they didn't change the amount, and otherwise permit negative balances,
		// but warn that they are doing so
		if(m_nType == 1 && !bIsNew && m_ID != -1 && m_bIsInUse) {
			// (j.jones 2012-04-25 10:53) - PLID 48032 - Supported line item corrections that were takebacks,
			// returning the value of the original payment to the batch payment. Also fixed to ignore
			// payments that were voided (unless part of another batch payment's takeback).
			_RecordsetPtr rs = CreateParamRecordset("SELECT NewRemainingAmount "
				"FROM (SELECT BatchPaymentsT.ID, "
				"	Convert(money, {STRING}) "
				"	 - Coalesce(LineItemsInUseQ.TotalApplied, Convert(money,0)) "
				"	 + Coalesce(LineItemsReversedQ.TotalReversed, Convert(money,0)) "
				"	 - Coalesce(AppliedPaymentsT.TotalApplied, Convert(money,0)) "
				"	AS NewRemainingAmount "
				"	FROM BatchPaymentsT "
				""
				//find child payments that are not voided, but include them if they are part of a takeback
				"	LEFT JOIN (SELECT PaymentsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalApplied "
				"		FROM LineItemT "
				"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT WHERE BatchPaymentID Is Null) AS LineItemCorrections_OriginalPaymentQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentQ.VoidingLineItemID "
				"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
				"		AND LineItemCorrections_OriginalPaymentQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingPaymentQ.VoidingLineItemID Is Null "
				"		AND PaymentsT.BatchPaymentID Is Not Null "
				"		GROUP BY PaymentsT.BatchPaymentID "
				"	) AS LineItemsInUseQ ON BatchPaymentsT.ID = LineItemsInUseQ.BatchPaymentID "
				""
				//find payments that were part of takebacks, crediting this batch payment
				"	LEFT JOIN (SELECT LineItemCorrectionsT.BatchPaymentID, Sum(LineItemT.Amount) AS TotalReversed "
				"		FROM LineItemT "
				"		INNER JOIN LineItemCorrectionsT ON LineItemT.ID = LineItemCorrectionsT.OriginalLineItemID "
				"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
				"		AND LineItemCorrectionsT.BatchPaymentID Is Not Null "
				"		GROUP BY LineItemCorrectionsT.BatchPaymentID "
				"	) AS LineItemsReversedQ ON BatchPaymentsT.ID = LineItemsReversedQ.BatchPaymentID "
				""
				//find the batch payment's adjustments or refunds
				"	LEFT JOIN (SELECT Sum(Amount) AS TotalApplied, AppliedBatchPayID "
				"		FROM BatchPaymentsT "
				"		WHERE Type <> 1 AND Deleted = 0 "
				"		GROUP BY AppliedBatchPayID, Deleted "
				"	) AS AppliedPaymentsT ON BatchPaymentsT.ID = AppliedPaymentsT.AppliedBatchPayID "
				""
				"	WHERE BatchPaymentsT.ID = {INT} AND BatchPaymentsT.Deleted = 0 AND BatchPaymentsT.Type = 1 "
				"	AND BatchPaymentsT.Amount <> Convert(money, {STRING}) "
				"	GROUP BY BatchPaymentsT.ID, BatchPaymentsT.Amount, LineItemsInUseQ.TotalApplied,LineItemsReversedQ.TotalReversed, AppliedPaymentsT.TotalApplied, BatchPaymentsT.Type, BatchPaymentsT.Deleted "
				") AS BatchPaymentsQ "
				"WHERE NewRemainingAmount < Convert(money,'$0.00')", FormatCurrencyForSql(cy), m_ID, FormatCurrencyForSql(cy));
			if(!rs->eof) {
				CString strWarn;
				strWarn.Format("The new amount you have entered would reduce the batch payment's balance to be less than %s.\n\n"
					"Are you sure you wish to save this batch payment with a negative balance?", FormatCurrencyForInterface(COleCurrency(0,0)));
				if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
			rs->Close();
		}

		//Used ONLY for auditing.
		strAmount = FormatCurrencyForInterface(cy);

		//Provider

		if(m_DoctorCombo->GetCurSel()!=-1) {
			var = m_DoctorCombo->GetValue(m_DoctorCombo->GetCurSel(),0);
			if(var.vt != VT_NULL)
				ProviderID = var.lVal;
		}

		_variant_t varProviderID = g_cvarNull;
		if(ProviderID == -1 && !m_DoctorCombo->IsComboBoxTextInUse) {
			//warn them before saving
			
			if(bIsNew) {
				// (j.jones 2011-07-08 17:16) - PLID 44497 - added pref. to force selection of a provider
				if(GetRemotePropertyInt("RequireProviderOnPayments", 1, 0, "<None>", true) == 1) {
					str = "You have not selected a provider for this batch payment.\n"
						"All new batch payments must have providers selected.";

					if(m_nType == 3)
						str.Replace("batch payment","refund");
					else if(m_nType == 2)
						str.Replace("batch payment","adjustment");

					MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_OK);
					return FALSE;
				}
				else {
					str = "You have not selected a provider for this batch payment.\n"
						"Are you sure you wish to save this batch payment without a provider?";
					if(m_nType == 3)
						str.Replace("batch payment","refund");
					else if(m_nType == 2)
						str.Replace("batch payment","adjustment");
					if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
						return FALSE;
					}
				}
			}
		}
		else if(!m_DoctorCombo->IsComboBoxTextInUse) {
			varProviderID = (long)ProviderID;
		}

		//Date

		// (c.haag 2009-03-10 16:17) - PLID 32433 - Fail if the user is trying to backdate an existing batch payment more days than is
		// acceptable. Because users are allowed to backdate existing items they entered in the same day, there's no point in checking
		// this for new items, or items entered on the same day.
		//
		// (c.haag 2009-05-18 12:04) - PLID 34273 - We no longer let new batch payments, or batch payments created today, override
		// the backdating validation.
		COleDateTime dtPayment = (COleDateTime)m_dtPaymentDate.GetValue();
		{
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtPayment.SetDate(dtPayment.GetYear(), dtPayment.GetMonth(), dtPayment.GetDay());
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			if (dtPayment < dtToday) {
				if (!CanChangeHistoricFinancial_ByServiceDate("BatchPayment", dtPayment, TRUE)) {
					// The user cannot save the payment as is, and they've already been prompted about it.
					return FALSE;
				}
			} else {
				// The payment is today or in the future
			}
		}

		//Insurance Company

		if(m_InsuranceCoCombo->GetCurSel()==-1) {
			MsgBox("Please choose an insurance company.");
			return FALSE;
		}

		InsCo = VarLong(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->GetCurSel(),0));


		//Description

		GetDlgItem(IDC_EDIT_DESCRIPTION)->GetWindowText(strDescription);
		if (strDescription.GetLength() == 0)
			strDescription = "(No description)";

		//Location

		if(m_LocationCombo->GetCurSel()==-1) {
			if(CString((LPCTSTR)m_LocationCombo->ComboBoxText) == "" || bIsNew) {
				LocationID = GetCurrentLocationID();
			}
			else {
				//It's the same inactive location it started as.
				_RecordsetPtr rsLoc = CreateRecordset("SELECT ID FROM LocationsT WHERE ID = (SELECT Location FROM BatchPaymentsT WHERE ID = %li)", m_ID);
				if(!rsLoc->eof) {
					LocationID = AdoFldLong(rsLoc, "ID");
				}
				else {
					LocationID = GetCurrentLocationID();
				}
			}
		}
		else {
			var = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0);
			if(var.vt == VT_I4)
				LocationID = var.lVal;
			else
				LocationID = GetCurrentLocationID();
		}

		//Payment Category

		if(m_PayCatsCombo->GetCurSel() != -1) {
			var = m_PayCatsCombo->GetValue(m_PayCatsCombo->GetCurSel(),0);
			if (var.vt != VT_NULL)
				PaymentGroupID = var.lVal;
			else
				PaymentGroupID = 0;
		}

		//Check No.		
		GetDlgItemText(IDC_CHECK_NO,strCheckNo);

		//Check Acct. No.
		GetDlgItemText(IDC_CHECK_ACCT_NO,strCheckAcctNo);

		//Bank Name
		GetDlgItemText(IDC_BANK_NAME,strBankName);
		
		//Bank Routing Num.
		GetDlgItemText(IDC_BANK_NO,strBankRoutingNum);

		_variant_t varAppliedPayID = g_cvarNull;
		if (m_nAppliedPayID != -1) {
			varAppliedPayID = (long)m_nAppliedPayID;
		}

		//save the data
		if(bIsNew) {
			//save new
			m_ID = NewNumber("BatchPaymentsT","ID");
			// (j.jones 2014-06-27 08:40) - PLID 62546 - added PayType, and parameterized
			// (j.jones 2015-10-08 13:25) - PLID 67307 - added capitation fields
			ExecuteParamSql("INSERT INTO BatchPaymentsT (ID, InsuranceCoID, Description, Amount, "
				"	Date, InputDate, UserID, PayCatID, "
				"	Location, ProviderID, CheckNo, BankRoutingNum, "
				"	BankName, CheckAcctNo, Type, AppliedBatchPayID, "
				"	PayType, Capitation, ServiceDateFrom, ServiceDateTo "
				") "
				""
				"VALUES ({INT}, {INT}, {STRING}, {OLECURRENCY}, "
				"	{OLEDATETIME}, GetDate(), {INT}, {INT}, "
				"	{INT}, {VT_I4}, {STRING}, {STRING}, "
				"	{STRING}, {STRING}, {INT}, {VT_I4}, "
				"	{INT}, {INT}, {VT_DATE}, {VT_DATE} "
				")",
				m_ID, InsCo, strDescription, cy,
				dtPayment, GetCurrentUserID(), PaymentGroupID,
				LocationID, varProviderID, strCheckNo, strBankRoutingNum,
				strBankName, strCheckAcctNo, m_nType, varAppliedPayID,
				(long)m_ePayType, (long)bCapitation ? 1 : 0, varServiceDateFrom, varServiceDateTo);

			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			if (AuditID != -1) {				
				CString strInsCoName = VarString(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->GetCurSel(), iccName), "");

				// (j.jones 2015-10-14 11:18) - PLID 67307 - reworked the auditing so normal batch payments
				// say Batch Payment, and capitation say Capitation Batch Payment and the dates
				CString strNewValue, strType;
				if (m_nType == 2) {
					strType = " Adjustment";
				}
				else if (m_nType == 3) {
					strType = " Refund";
				}
				strNewValue.Format("Batch Payment%s for %s", strType, strInsCoName);

				if (bCapitation) {
					strNewValue.Format("Capitation Batch Payment%s for %s (%s - %s)", strType, strInsCoName,
						FormatDateTimeForInterface(VarDateTime(varServiceDateFrom), NULL, dtoDate),
						FormatDateTimeForInterface(VarDateTime(varServiceDateTo), NULL, dtoDate));
				}

				// (j.jones 2014-06-27 08:40) - PLID 62546 - if the vision payments license exists, audit the type of payment
				if (m_nType == 1 && g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
					if (m_ePayType == eVisionPayment) {
						strNewValue = "Vision " + strNewValue;
					}
					else {
						strNewValue = "Medical " + strNewValue;
					}
				}

				AuditEvent(-1, strInsCoName, AuditID, (m_nType != 1 ? aeiBatchPayRefundCreated : aeiBatchPaymentCreated), m_ID, "", strNewValue, aepHigh, aetCreated);
			}
		}
		else {
			CString strInsName = VarString(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->GetCurSel(), iccName), "");
			// (j.jones 2015-10-14 11:25) - PLID 67307 - supported capitation fields
			_RecordsetPtr rsAudit = CreateParamRecordset("SELECT Date, Description, Amount, InsuranceCoID, Name, "
				"Capitation, ServiceDateFrom, ServiceDateTo "
				"FROM BatchPaymentsT "
				"LEFT JOIN InsuranceCoT ON BatchPaymentsT.InsuranceCoID = InsuranceCoT.PersonID WHERE ID = {INT}", m_ID);
			if (!rsAudit->eof) {
				CString oldVal;
				_variant_t var;

				// (j.jones 2015-10-14 11:48) - PLID 67307 - added better audit desc.
				CString strBatchPayInfo;
				strBatchPayInfo.Format("%s payment for %s (%s)", strAmount, strInsName, FormatDateTimeForInterface(dtPayment, 0, dtoDate));

				long AuditID = -1;

				var = rsAudit->Fields->Item["Description"]->Value;
				oldVal = CString(var.bstrVal);
				if (oldVal != strDescription) {
					if (AuditID == -1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(-1, strInsName, AuditID, aeiBatchPayDescription, m_ID, oldVal + " - " + strBatchPayInfo, strDescription, aepLow);
				}

				// (b.savon 2012-06-14 16:05) - PLID 49879 - Format this for the interface
				var = rsAudit->Fields->Item["Date"]->Value;
				COleDateTime dt = var.date;
				oldVal = FormatDateTimeForInterface(dt, 0, dtoDate);

				CString strDate = FormatDateTimeForInterface(dtPayment, 0, dtoDate);

				if (oldVal != strDate) {
					// (b.savon 2012-06-13 15:36) - PLID 49879 - Warn the user, if they changed their mind, get out.
					if (IDYES != MessageBox("WARNING!\r\n\r\nThe date has been edited for this batch payment.  This will also "
						"change all the payment dates to reflect the new batch payment date.\r\n\r\n"
						"Are you sure you want to continue?", "Batch Payment Date Changed", MB_YESNO | MB_ICONWARNING)){
						// Reset
						m_dtPaymentDate.SetValue(var);
						return FALSE;
					}
					m_bBatchDateChanged = TRUE;
					m_strBatchDate = strDate;

					if (AuditID == -1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(-1, strInsName, AuditID, aeiBatchPayDate, m_ID, oldVal + " - " + strBatchPayInfo, strDate, aepMedium);
				}

				var = rsAudit->Fields->Item["Amount"]->Value;
				oldVal = FormatCurrencyForInterface(var.cyVal);
				if (oldVal != strAmount) {
					if (AuditID == -1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(-1, strInsName, AuditID, aeiBatchPayAmount, m_ID, oldVal + " - " + strBatchPayInfo, strAmount, aepHigh);
				}

				var = rsAudit->Fields->Item["Name"]->Value;
				oldVal = CString(var.bstrVal);
				if (oldVal != strInsName) {
					if (AuditID == -1)
						AuditID = BeginNewAuditEvent();
					AuditEvent(-1, strInsName, AuditID, aeiBatchPayInsCo, m_ID, oldVal + " - " + strBatchPayInfo, strInsName, aepHigh);
				}

				// (j.jones 2015-10-14 11:25) - PLID 67307 - supported capitation fields
				var = rsAudit->Fields->Item["Capitation"]->Value;
				if (!!VarBool(var) != bCapitation) {
					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}
					AuditEvent(-1, strInsName, AuditID, aeiBatchPayCapitation, m_ID, CString(bCapitation ? "Not a Capitation Payment" : "Capitation Payment") + " - " + strBatchPayInfo, bCapitation ? "Capitation Payment" : "Not a Capitation Payment", aepHigh);
				}

				var = rsAudit->Fields->Item["ServiceDateFrom"]->Value;
				if (var.vt != varServiceDateFrom.vt
					|| (var.vt == VT_DATE && varServiceDateFrom.vt == VT_DATE && VarDateTime(var) != VarDateTime(varServiceDateFrom))) {

					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}

					CString strOld = "No Date", strNew = "No Date";
					if (var.vt == VT_DATE) {
						strOld = FormatDateTimeForInterface(VarDateTime(var), NULL, dtoDate);
					}
					if (varServiceDateFrom.vt == VT_DATE) {
						strNew = FormatDateTimeForInterface(VarDateTime(varServiceDateFrom), NULL, dtoDate);
					}

					AuditEvent(-1, strInsName, AuditID, aeiBatchPayServiceDateFrom, m_ID, strOld + " - " + strBatchPayInfo, strNew, aepHigh);
				}

				var = rsAudit->Fields->Item["ServiceDateTo"]->Value;
				if (var.vt != varServiceDateTo.vt
					|| (var.vt == VT_DATE && varServiceDateTo.vt == VT_DATE && VarDateTime(var) != VarDateTime(varServiceDateTo))) {

					if (AuditID == -1) {
						AuditID = BeginNewAuditEvent();
					}

					CString strOld = "No Date", strNew = "No Date";
					if (var.vt == VT_DATE) {
						strOld = FormatDateTimeForInterface(VarDateTime(var), NULL, dtoDate);
					}
					if (varServiceDateTo.vt == VT_DATE) {
						strNew = FormatDateTimeForInterface(VarDateTime(varServiceDateTo), NULL, dtoDate);
					}

					AuditEvent(-1, strInsName, AuditID, aeiBatchPayServiceDateTo, m_ID, strOld + " - " + strBatchPayInfo, strNew, aepHigh);
				}
			}
			rsAudit->Close();

			//in case provider is inactive, we don't want to save over it
			CSqlFragment sqlProviderUpdate("");
			if (!m_DoctorCombo->IsComboBoxTextInUse) {
				sqlProviderUpdate = CSqlFragment("ProviderID = {VT_I4}, ", varProviderID);
			}

			// (j.jones 2014-06-27 08:40) - PLID 62546 - parameterized
			// (j.jones 2015-10-08 13:25) - PLID 67307 - added capitation fields
			ExecuteParamSql("UPDATE BatchPaymentsT SET InsuranceCoID = {INT}, Description = {STRING}, Amount = {OLECURRENCY}, "
				"Date = {OLEDATETIME}, PayCatID = {INT}, Location = {INT}, {SQL} "
				"CheckNo = {STRING}, BankRoutingNum = {STRING}, "
				"BankName = {STRING}, CheckAcctNo = {STRING}, "
				"Capitation = {INT}, ServiceDateFrom = {VT_DATE}, ServiceDateTo = {VT_DATE} "
				"WHERE ID = {INT}",
				InsCo, strDescription, cy,
				dtPayment, PaymentGroupID, LocationID, sqlProviderUpdate,
				strCheckNo, strBankRoutingNum, strBankName, strCheckAcctNo,
				(long)bCapitation ? 1 : 0, varServiceDateFrom, varServiceDateTo,
				m_ID);
		}

		if(bIsNew) {
			// (j.jones 2004-06-30 14:03) - if they are saving a $0.00 payment, enable the ability to show $0.00 payments
			if(cy == COleCurrency(0,0)) {
				SetRemotePropertyInt("ShowZeroBalanceBatchPays",1,0,GetCurrentUserName());
			}
		}

		return TRUE;

	}NxCatchAll("Error saving batch payment.");

	return FALSE;
}

void CBatchPaymentEditDlg::Load()
{
	try {

		bool bIsNew = true;

		if(m_ID == -1)
			bIsNew = true;
		else
			bIsNew = false;

		if(bIsNew) {

			// (j.jones 2009-06-26 12:07) - PLID 33856 - the original amount is always hidden on new payments
			GetDlgItem(IDC_ORIGINAL_CURRENCY_SYMBOL_BATCHPAY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LABEL_ORIGINAL_AMOUNT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_ORIGINAL_TOTAL)->ShowWindow(SW_HIDE);

			if(m_nType == 1) {

				//initialize a new payment

				//set the data

				//Amount
				SetDlgItemText(IDC_EDIT_TOTAL,"0.00");

				//Date
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				_variant_t varNow = dtNow;
				m_dtPaymentDate.SetValue(varNow);

				//Insurance Company

				//Description
				//TES 5/28/2008 - PLID 26189 - We now pull their preferred default.
				// (j.jones 2008-07-11 14:02) - PLID 28756 - this may be overridden later by the insurance company default,
				// but fill it in now anyways
				SetDlgItemText(IDC_EDIT_DESCRIPTION, GetRemotePropertyText("BatchPay_DefaultDescription", "", 0, "<None>", true));

				//Location
				m_LocationCombo->SetSelByColumn(0,GetCurrentLocationID());

				//Provider

				// (j.jones 2011-07-08 17:04) - if we ever want to default to a provider,
				// don't forget that we have a preference called "DefaultPaymentsNoProvider" that
				// would need to be respected

				//Payment Category
				//TES 5/28/2008 - PLID 26189 - We now pull their preferred default.
				// (j.jones 2008-07-11 14:02) - PLID 28756 - this may be overridden later by the insurance company default,
				// but fill it in now anyways
				m_PayCatsCombo->TrySetSelByColumn(0, GetRemotePropertyInt("BatchPay_DefaultCategory", -1, 0, "<None>", true));

				// (j.jones 2015-10-08 13:22) - PLID 67307 - set the capitation dates to today
				m_dtServiceDateFrom.SetValue(varNow);
				m_dtServiceDateTo.SetValue(varNow);
			}
			else {	//refund or adjustment

				//load information from the batch payment

				_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM BatchPaymentsT WHERE ID = {INT}",m_nAppliedPayID);

				//load the data		

				//Amount
				//use the passed in m_cyMaxRefundAmt
				CString strAmount = FormatCurrencyForInterface(m_cyMaxApplyAmt, FALSE);
				SetDlgItemText(IDC_EDIT_TOTAL,strAmount);

				//Date
				COleDateTime dt = COleDateTime::GetCurrentTime();
				_variant_t var = dt;
				m_dtPaymentDate.SetValue(var);

				//Insurance Company

				var = rs->Fields->Item["InsuranceCoID"]->Value;
				m_InsuranceCoCombo->SetSelByColumn(0,var); // Insurance company
				m_InsuranceCoCombo->Enabled = FALSE; //don't let them change the company

				// (j.jones 2008-07-11 13:58) - PLID 28756 - set our default category and description
				TrySetDefaultInsuranceDescriptions();

				//Description

				//Location

				var = rs->Fields->Item["Location"]->Value; // Pay ment Location
				if(m_LocationCombo->TrySetSelByColumn(0,var) == -1) {
					_RecordsetPtr rsLocation = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM BatchPaymentsT WHERE ID = %li)", m_ID);
					if(!rsLocation->eof) {
						m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocation, "Name", "")));
					}
				}

				//Provider

				var = rs->Fields->Item["ProviderID"]->Value; // Payment Provider
				if(m_DoctorCombo->TrySetSelByColumn(0,var) == -1) {
					//they may have an inactive provider
					_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT ProviderID FROM BatchPaymentsT WHERE ID = %li)", m_ID);
					if(!rsProv->eof) {
						m_DoctorCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
					}
					else 
						m_DoctorCombo->PutCurSel(-1);
				}

				// (j.jones 2015-10-08 12:59) - PLID 67307 - the capitation checkbox is not an option on refunds/adjustments
				m_checkCapitation.SetCheck(FALSE);
				m_checkCapitation.ShowWindow(SW_HIDE);

				// (j.jones 2015-10-08 13:33) - PLID 67307 - rename the "create" button accordingly
				if (m_nType == 2) {
					m_btnOK.SetWindowText("Create Adjustment");
				}
				else if (m_nType == 3) {
					m_btnOK.SetWindowText("Create Refund");
				}
			}
		}
		else {

			//existing batch payment (could be a refund or adjustment)

			_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM BatchPaymentsT WHERE ID = {INT}",m_ID);

			//load the data

			//Amount

			_variant_t var = AdoFldCurrency(rs, "Amount", COleCurrency(0, 0));
			CString strAmount = FormatCurrencyForInterface(var.cyVal, FALSE);
			SetDlgItemText(IDC_EDIT_TOTAL,strAmount);

			//Date
			COleDateTime dtPaymentDate = VarDateTime(rs->Fields->Item["Date"]->Value);	// Payment date
			m_dtPaymentDate.SetValue(_variant_t(dtPaymentDate));

			//Insurance Company

			var = rs->Fields->Item["InsuranceCoID"]->Value;
			m_InsuranceCoCombo->SetSelByColumn(0,var); // Insurance company			

			//Description
			var = rs->Fields->Item["Description"]->Value;
			// Set description box
			GetDlgItem(IDC_EDIT_DESCRIPTION)->SetWindowText(CString(var.bstrVal));
			m_DescriptionCombo->SetSelByColumn(0,var);

			//Location

			var = rs->Fields->Item["Location"]->Value; // Payment Location
			if(m_LocationCombo->TrySetSelByColumn(0,var) == -1) {
				_RecordsetPtr rsLocation = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM BatchPaymentsT WHERE ID = %li)", m_ID);
				if(!rsLocation->eof) {
					m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocation, "Name", "")));
				}
			}

			//Provider

			var = rs->Fields->Item["ProviderID"]->Value; // Payment Provider
			if(m_DoctorCombo->TrySetSelByColumn(0,var) == -1) {
				//they may have an inactive provider
				_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT ProviderID FROM BatchPaymentsT WHERE ID = %li)", m_ID);
				if(!rsProv->eof) {
					m_DoctorCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
				}
				else 
					m_DoctorCombo->PutCurSel(-1);
			}


			//Payment Category

			var = rs->Fields->Item["PayCatID"]->Value;
			m_PayCatsCombo->SetSelByColumn(0,var); // Payment category

			//Check No.
			SetDlgItemText(IDC_CHECK_NO,AdoFldString(rs, "CheckNo","")); // Check No.

			//Check Acct. No.
			SetDlgItemText(IDC_CHECK_ACCT_NO,AdoFldString(rs, "CheckAcctNo",""));

			//Bank Name
			SetDlgItemText(IDC_BANK_NAME,AdoFldString(rs, "BankName",""));
			
			//Bank Routing Num.
			SetDlgItemText(IDC_BANK_NO,AdoFldString(rs, "BankRoutingNum",""));

			// (j.jones 2009-06-26 12:07) - PLID 33856 - the original amount is only shown if its value is not null
			_variant_t varOriginalAmt = rs->Fields->Item["OriginalAmount"]->Value;
			if(varOriginalAmt.vt == VT_CY) {
				SetDlgItemText(IDC_EDIT_ORIGINAL_TOTAL, FormatCurrencyForInterface(VarCurrency(varOriginalAmt), FALSE));
			}
			else {
				GetDlgItem(IDC_ORIGINAL_CURRENCY_SYMBOL_BATCHPAY)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_LABEL_ORIGINAL_AMOUNT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_EDIT_ORIGINAL_TOTAL)->ShowWindow(SW_HIDE);
			}

			// (j.jones 2014-06-27 08:40) - PLID 62546 - load the PayType
			m_ePayType = (EBatchPaymentPayType)VarLong(rs->Fields->Item["PayType"]->Value, (long)eMedicalPayment);

			// (j.jones 2015-10-08 12:59) - PLID 67307 - load the capitation checkbox,
			// if this is a remit payment the capitation checkbox is not an option
			if (VarBool(rs->Fields->Item["ERemittance"]->Value, FALSE)) {
				m_checkCapitation.SetCheck(FALSE);
				m_checkCapitation.EnableWindow(FALSE);
			}
			else {
				m_checkCapitation.SetCheck(VarBool(rs->Fields->Item["Capitation"]->Value, FALSE));
				m_dtServiceDateFrom.SetValue(VarDateTime(rs->Fields->Item["ServiceDateFrom"]->Value, dtPaymentDate));
				m_dtServiceDateTo.SetValue(VarDateTime(rs->Fields->Item["ServiceDateTo"]->Value, dtPaymentDate));
			}

			// (j.jones 2015-10-08 13:33) - PLID 67307 - rename the "create" button accordingly
			if (m_nType == 1) {
				m_btnOK.SetWindowText("Save Payment");
			}
			//technically you can't get to this following code - you can't edit adjustments or refunds
			else if (m_nType == 2) {
				m_btnOK.SetWindowText("Save Adjustment");
			}
			else if (m_nType == 3) {
				m_btnOK.SetWindowText("Save Refund");
			}
		}

		//if a payment, set the window text based on payment type
		if (m_nType == 1) {

			// (j.jones 2014-06-26 17:23) - PLID 62546 - the PayType defines our title,
			// in addition to the Vision Payment license
			if (g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
				if (m_ePayType == eVisionPayment) {
					SetWindowText("Vision Batch Payment");

					// (j.jones 2015-10-16 15:58) - PLID 67307 - the capitation checkbox is not an option on vision payments
					m_checkCapitation.SetCheck(FALSE);
					m_checkCapitation.ShowWindow(SW_HIDE);
				}
				else {
					SetWindowText("Medical Batch Payment");
				}
			}
			else {
				//ignore the payment type, always call it a batch payment
				SetWindowText("Batch Payment");
			}

			// (j.jones 2015-10-26 14:03) - PLID 67462 - If capitation payments are disabled,
			// hide the controls on new payments or existing non-capitation payments.
			// Loading an existing capitation payment will show the controls and ignore the preference.
			if (GetRemotePropertyInt("BatchPayments_EnableCapitation", 0, 0, "<None>", true) == 0) {
				//hide the capitation option if this is a new payment or a non-capitation payment
				if (bIsNew || m_checkCapitation.GetCheck() == FALSE) {
					m_checkCapitation.SetCheck(FALSE);
					m_checkCapitation.ShowWindow(SW_HIDE);
				}
			}
		}

		// (j.jones 2015-10-08 10:25) - PLID 67307 - this will show/hide the capitation service date range
		// based on the value of the capitation checkbox
		OnCheckCapitation();

	}NxCatchAll("Error loading batch payment.");
}

// (j.jones 2015-10-08 13:11) - PLID 67307 - renamed this function and removed its parameter, it only disables items
void CBatchPaymentEditDlg::DisableItems_BatchPayInUse()
{
	// (j.jones 2011-06-24 13:05) - PLID 41863 - you can now change the amount on in-use batch payments
	//((CNxEdit*)GetDlgItem(IDC_EDIT_TOTAL))->SetReadOnly(true);
	m_InsuranceCoCombo->Enabled = false;
	m_DoctorCombo->Enabled = false;
	m_LocationCombo->Enabled = false;

	// (b.savon 2012-06-13 15:11) - PLID 49879 - Enable based on the preference
	// (j.jones 2015-10-08 13:15) - PLID 67307 - removed this code, the date is disabled
	// at all times in OnInitDialog if a preference says so, and is otherwise always
	// editable on in-use payments, which is what PLID 49879 asked for
	//GetDlgItem(IDC_PAY_DATE)->EnableWindow(FALSE);

	// (j.jones 2015-10-08 13:10) - PLID 67307 - disable all capitation fields if in use
	m_checkCapitation.EnableWindow(FALSE);
	m_dtServiceDateFrom.EnableWindow(FALSE);
	m_dtServiceDateTo.EnableWindow(FALSE);
}

void CBatchPaymentEditDlg::OnTrySetSelFinishedLocations(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//OK, they must have an inactive location selected.
			if(m_ID != -1) {
				_RecordsetPtr rsLocation = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM BatchPaymentsT WHERE ID = %li)", m_ID);
				if(!rsLocation->eof) {
					m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocation, "Name", "")));
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CBatchPaymentEditDlg::OnTrySetSelFinishedProviderCombo(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//they may have an inactive provider
			_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT ProviderID FROM BatchPaymentsT WHERE ID = %li)", m_ID);
			if(!rsProv->eof) {
				m_DoctorCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
			else 
				m_DoctorCombo->PutCurSel(-1);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2008-07-11 13:45) - PLID 28756 - this function will check
// which insurance company is selected, and override the 
// payment description and payment category 
void CBatchPaymentEditDlg::TrySetDefaultInsuranceDescriptions()
{
	try {

		//return if this is a refund
		if(m_nType == 3) {
			return;
		}

		//return if no insurance is selected
		if(m_InsuranceCoCombo->CurSel == -1) {
			return;
		}

		CString strDefaultDesc = "";
		long nDefaultCategoryID = -1;

		if(m_nType == 1) {	//payment

			strDefaultDesc = VarString(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->CurSel, iccDefaultPayDesc), "");
			nDefaultCategoryID = VarLong(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->CurSel, iccDefaultPayCategoryID), -1);
		}
		else if(m_nType == 2) {	//adjustment
		
			strDefaultDesc = VarString(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->CurSel, iccDefaultAdjDesc), "");
			nDefaultCategoryID = VarLong(m_InsuranceCoCombo->GetValue(m_InsuranceCoCombo->CurSel, iccDefaultAdjCategoryID), -1);		
		}
		else {
			return;
		}

		if(nDefaultCategoryID != -1) {
			m_PayCatsCombo->TrySetSelByColumn(0, (long)nDefaultCategoryID);
			//normally when we manually select a category, it fills the description, but we do NOT do this on defaults
		}

		if(!strDefaultDesc.IsEmpty()) {
			m_nxeditEditDescription.SetWindowText(strDefaultDesc);
		}

	}NxCatchAll("Error in CBatchPaymentEditDlg::TrySetDefaultInsuranceDescriptions");
}

void CBatchPaymentEditDlg::OnSelChosenInsuranceCompanies(long nRow) 
{
	try {
		// (j.jones 2008-07-11 13:58) - PLID 28756 - set our default category and description
		TrySetDefaultInsuranceDescriptions();
	}NxCatchAll("Error in CBatchPaymentEditDlg::OnSelChosenInsuranceCompanies");
}

// (j.jones 2015-10-08 10:25) - PLID 67307 - added capitation payment checkbox & date range
void CBatchPaymentEditDlg::OnCheckCapitation()
{
	try {

		//show/hide the service date controls
		bool bIsCapitation = m_checkCapitation.GetCheck() ? true : false;
		
		m_dtServiceDateFrom.ShowWindow(bIsCapitation ? SW_SHOW : SW_HIDE);
		m_dtServiceDateTo.ShowWindow(bIsCapitation ? SW_SHOW : SW_HIDE);
		m_nxstaticLabelServiceDate.ShowWindow(bIsCapitation ? SW_SHOW : SW_HIDE);
		m_nxstaticLabelServiceDateTo.ShowWindow(bIsCapitation ? SW_SHOW : SW_HIDE);

	}NxCatchAll(__FUNCTION__);
}
