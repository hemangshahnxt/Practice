// GCEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GCEntryDlg.h"
#include "GCTypeDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "NewPatient.h"
#include "barcode.h"
#include "PaymentDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "AuditTrail.h"
#include "SecurityGroupsDlg.h"
#include "PaymentDlg.h"
#include "BillingDlg.h"
#include "BillingRc.h"

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CGCEntryDlg dialog

enum TypeListColumns {
	tlcID = 0,
	tlcName,
	tlcValue, // (r.gonet 2015-03-24 18:05) - PLID 65276 - The value column of the GC types list
	tlcPrice, // (r.gonet 2015-03-24 18:05) - PLID 65276 - The price column of the GC types list. Usually the same as Value, but can be different, eg $100 GC for $80.
	tlcExpires,
	tlcDays,
};

enum PatientListColumns {
	plcPersonID = 0,
	plcUserID, 
	plcName,
	plcPhone,
	plcAddress,
	plcCity,
	plcState,
	plcZip,
};

// (j.jones 2015-05-11 16:57) - PLID 65714 - added GCCreationStyle as an optional parameter
CGCEntryDlg::CGCEntryDlg(CWnd* pParent, GCCreationStyle eCreationStyle /*= GCCreationStyle::eCreateNewGC*/)
	: CNxDialog(CGCEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGCEntryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bShowingRec = true;
	m_nID = -1;
	
	// (j.jones 2015-05-11 17:04) - PLID 65714 - added m_eCreationStyle
	m_eCreationStyle = eCreationStyle;
	
	m_bEditing = false;
	// (r.gonet 2015-03-24 18:05) - PLID 65276 - Initialize the sync control flag to false to prevent changes to the
	// Value field from syncing with the Price field.
	m_bSyncPriceWithValue = false;
}


void CGCEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGCEntryDlg)
	DDX_Control(pDX, IDC_EXP_DATE_CHECK, m_btnExpDate);
	DDX_Control(pDX, IDC_REC_TYPE_UNKNOWN, m_btnUnknown);
	DDX_Control(pDX, IDC_REC_TYPE_PAT, m_btnPat);
	DDX_Control(pDX, IDC_PURCHASE_DATE, m_dtPurchase);
	DDX_Control(pDX, IDC_EXP_DATE, m_dtExpire);
	DDX_Control(pDX, IDC_CERT_NUMBER, m_nxeditCertNumber);
	DDX_Control(pDX, IDC_CERT_VALUE_EDIT, m_nxeditCertValue);
	DDX_Control(pDX, IDC_CERT_PRICE_EDIT, m_nxeditCertPrice);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGCEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CGCEntryDlg)
	ON_BN_CLICKED(IDC_REC_TYPE_PAT, OnRecTypePat)
	ON_BN_CLICKED(IDC_OPEN_GC_TYPES, OnOpenGcTypes)
	ON_BN_CLICKED(IDC_ADD_NEW_PURCHASER, OnAddNewPurchaser)
	ON_BN_CLICKED(IDC_ADD_NEW_RECEIVER, OnAddNewReceiver)
	ON_BN_CLICKED(IDC_REC_TYPE_UNKNOWN, OnRecTypeUnknown)
	ON_BN_CLICKED(IDC_EXP_DATE_CHECK, OnExpDateCheck)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_EN_KILLFOCUS(IDC_CERT_PRICE_EDIT, OnKillfocusCertPrice)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(IDC_CERT_VALUE_EDIT, &CGCEntryDlg::OnEnKillfocusCertValueEdit)
	ON_EN_SETFOCUS(IDC_CERT_VALUE_EDIT, &CGCEntryDlg::OnEnSetfocusCertValueEdit)
	ON_EN_CHANGE(IDC_CERT_VALUE_EDIT, &CGCEntryDlg::OnEnChangeCertValueEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCEntryDlg message handlers

BOOL CGCEntryDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 16:49) - PLID 29876 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxeditCertNumber.SetLimitText(50);

		g_propManager.CachePropertiesInBulk("GCEntryDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewPatientOpenSecurityGroups' "
			// (j.jones 2011-09-23 09:12) - PLID 32067 - added RequireProviderOnCharges
			"OR Name = 'DefaultChargesNoProvider' "
			"OR Name = 'EnableBillInvoiceNumbers' " // (j.jones 2013-07-12 10:00) - PLID 57539
			")",
			_Q(GetCurrentUserName()));

		//set default dates
		COleDateTime dt = COleDateTime::GetCurrentTime();
		m_dtPurchase.SetValue(_variant_t(dt));
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(365, 0, 0, 0);
		m_dtExpire.SetValue(_variant_t(dt + dtSpan));
		OnExpDateCheck();

		//setup datalists
		m_pPurchList = BindNxDataListCtrl(IDC_PURCH_PAT_LIST);
		m_pRecList = BindNxDataListCtrl(IDC_REC_PAT_LIST);
		m_pTypeList = BindNxDataListCtrl(IDC_GC_TYPE_LIST, false);
		m_pProvList = BindNxDataListCtrl(IDC_GC_PROVIDER_LIST);
		m_pLocationList = BindNxDataListCtrl(IDC_GC_LOCATIONS_LIST);

		// (r.gonet 2015-05-14 18:05) - PLID 66033 - If this gift certificate is being created as part of a refund
		// then choose the type for them and don't let them change it.
		// (a.walling 2016-06-07 8:14) - NX-100745 - Insert into ServiceLocationInfoT whenever inserting into ServiceT
		long nTypeID = -1;
		if (m_eCreationStyle == GCCreationStyle::eRefundToNewGC) {
			_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), R"(
SET NOCOUNT ON;
DECLARE @RefundTypeID INT SET @RefundTypeID = (SELECT TOP 1 ServiceID FROM GCTypesT WHERE IsRefund = 1 ORDER BY ServiceID ASC)
IF @RefundTypeID IS NULL
BEGIN
	SET @RefundTypeID = (SELECT CONVERT(INT, COALESCE(MAX(ID), 0) + 1) FROM ServiceT);

	INSERT INTO ServiceT(ID, Name, Category, Price)
	VALUES(@RefundTypeID, 'Refund to New Gift Certificate', NULL, CONVERT(MONEY, 0));

	INSERT INTO GCTypesT(ServiceID, DefaultExpires, DefaultDays, IsRecharge, Redeemable, Points, DiscountCategoryID, Value, IsRefund)
	VALUES(@RefundTypeID, 0, NULL, 0, 0, CONVERT(MONEY, 0), NULL, CONVERT(MONEY, 0), 1);
	
	INSERT INTO ServiceLocationInfoT (ServiceID, LocationID)
	SELECT @RefundTypeID, LocationsT.ID FROM LocationsT WHERE LocationsT.Managed = 1

END
SET NOCOUNT OFF;
SELECT @RefundTypeID AS RefundTypeID;
)");
			if (!prs->eof) {
				nTypeID = AdoFldLong(prs->Fields, "RefundTypeID", -1);
			}
			if (nTypeID == -1) {
				ThrowNxException("%s : Could not find or create the built-in refund gift certificate type.", __FUNCTION__);
			}
			m_pTypeList->WhereClause = _bstr_t(FormatString("ServiceID = %li", nTypeID));
			// (r.gonet 2015-05-14 16:49) - PLID 66033 - Don't let them change the selection or edit the type.
			EnableDlgItem(IDC_GC_TYPE_LIST, FALSE);
			EnableDlgItem(IDC_OPEN_GC_TYPES, FALSE);

		} else {
			// (r.gonet 2015-05-14 16:49) - PLID 66033 - Exclude refund types.
			m_pTypeList->WhereClause = _bstr_t("IsRecharge = 0 AND Active = 1 AND IsRefund = 0");
		}
		m_pTypeList->Requery();

		//We must auto-generate a number to start.  This is editable, because they may use pre-printed gift
		//	cards or certificates and need to enter that number.
		GenerateNewGCID();

		//default the value and price to nothing
		COleCurrency cyDisplayedValue, cyPrice;
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Add a Value field and default it to 0.
		cyDisplayedValue = g_ccyZero;
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Rename Amount to Price
		cyPrice = g_ccyZero;

		//if we're coming from a bill, the purchaser MUST be the current patient.
		// (r.gonet 2015-04-29 10:00) - PLID 65327 - If this gift certificate is being created from
		// the billing dialog, then certain fields must be disabled.
		// (j.jones 2015-05-11 17:04) - PLID 65714 - changed this to follow m_eCreationStyle instead
		if (m_eCreationStyle == GCCreationStyle::eCreateNewGC_FromBill) {

			//the user manually added a GC to a bill, we do not need to create a bill

			if (m_nPresetPurchasedByPatientID == -1) {
				//we're on a bill, and were not given the patient to use.  We'll go with the current (since you can
				//only bill the current patient)
				m_nPresetPurchasedByPatientID = GetActivePatientID();
			}

			//If we're coming from a bill, all of this info is pulled from the bill.  We just need to 
			//	disable these fields, we can't have them changing the info now.
			((CWnd*)GetDlgItem(IDC_PURCH_PAT_LIST))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_GC_TYPE_LIST))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_GC_PROVIDER_LIST))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_PURCHASE_DATE))->EnableWindow(FALSE);
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Disable the new Value field.
			((CWnd*)GetDlgItem(IDC_CERT_VALUE_EDIT))->EnableWindow(FALSE);
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Rename Amount to Price
			((CWnd*)GetDlgItem(IDC_CERT_PRICE_EDIT))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_GC_LOCATIONS_LIST))->EnableWindow(FALSE);
			// (v.maida 2015-03-10 12:33) - PLID 60126 - If we're coming from a bill, the Purchaser should remain set to the patient on the bill we're coming from,
			// so disable the add purchaser button.
			((CWnd*)GetDlgItem(IDC_ADD_NEW_PURCHASER))->EnableWindow(FALSE);
		}
		// (j.jones 2015-05-11 17:04) - PLID 65714 - changed this to follow m_eCreationStyle instead
		else if (m_eCreationStyle == GCCreationStyle::eRefundToNewGC) {

			//this GC is being created from a refund

			if (m_nPresetPurchasedByPatientID == -1) {
				//we're on a payment dialog (refund), and were not given the patient to use.  We'll go with the current (since you can
				//only bill the current patient)
				m_nPresetPurchasedByPatientID = GetActivePatientID();
			}

			// (r.gonet 2015-04-29 10:00) - PLID 65327 - If we're coming from a refund, all of this info 
			// is pulled from the payment dialog.  We just need to disable these fields, we can't have them 
			// changing the info now.
			((CWnd*)GetDlgItem(IDC_PURCH_PAT_LIST))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_GC_PROVIDER_LIST))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_PURCHASE_DATE))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_CERT_VALUE_EDIT))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_CERT_PRICE_EDIT))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_GC_LOCATIONS_LIST))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_ADD_NEW_PURCHASER))->EnableWindow(FALSE);
			GetDlgItem(IDC_REC_TYPE_UNKNOWN)->EnableWindow(FALSE);
			GetDlgItem(IDC_REC_TYPE_PAT)->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_REC_PAT_LIST))->EnableWindow(FALSE);
			GetDlgItem(IDC_ADD_NEW_RECEIVER)->EnableWindow(FALSE);
		}
		// (j.jones 2015-05-11 17:04) - PLID 65714 - supported balance transfers
		else if (m_eCreationStyle == GCCreationStyle::eTransferToNewGC) {

			//this GC is being created from a balance transfer

			if (m_nPresetPurchasedByPatientID == -1) {
				//the calling code should not have allowed this
				ASSERT(FALSE);
				m_nPresetPurchasedByPatientID = GetActivePatientID();

				//let them change this patient if they got here
			} else {
				//normally this is disabled
				((CWnd*)GetDlgItem(IDC_PURCH_PAT_LIST))->EnableWindow(FALSE);
				((CWnd*)GetDlgItem(IDC_ADD_NEW_PURCHASER))->EnableWindow(FALSE);
			}

			((CWnd*)GetDlgItem(IDC_PURCHASE_DATE))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_CERT_VALUE_EDIT))->EnableWindow(FALSE);
			((CWnd*)GetDlgItem(IDC_CERT_PRICE_EDIT))->EnableWindow(FALSE);
		}

		// (r.gonet 2015-03-23 16:20) - PLID 65272 - Disable the types editor if they don't have permission to edit types.
		if (!CheckCurrentUserPermissions(bioEditGiftCertificateTypes, sptWrite, FALSE, 0, TRUE)) {
			GetDlgItem(IDC_OPEN_GC_TYPES)->EnableWindow(FALSE);
		}

		// (r.gonet 2015-03-24 16:20) - PLID 65276 - Make the Value field read only if they don't have permission to edit Values
		if (!CheckCurrentUserPermissions(bioEditGiftCertificateValue, sptWrite, FALSE, 0, TRUE)) {
			((CNxEdit*)GetDlgItem(IDC_CERT_VALUE_EDIT))->SetReadOnly(TRUE);
		}

		// (r.gonet 2015-03-24 16:20) - PLID 65276 - Make the Price field read only if they don't have permission to edit Prices.
		if (!CheckCurrentUserPermissions(bioEditGiftCertificatePrice, sptWrite, FALSE, 0, TRUE)) {
			((CNxEdit*)GetDlgItem(IDC_CERT_PRICE_EDIT))->SetReadOnly(TRUE);
		}

		// (r.gonet 2015-04-29 10:00) - PLID 65327 - If the caller has preset the certificate number, then use that in place
		// of the auto-generated one.
		if (m_strPresetCertNumber != "") {
			SetDlgItemText(IDC_CERT_NUMBER, m_strPresetCertNumber);
		}

		//if we had anything set before opening, try to select them in the datalist now
		// (r.gonet 2015-04-29 10:00) - PLID 65327 - Renamed to m_nPresetProviderID for consistency
		if (m_nPresetProviderID != -1) {
			m_pProvList->SetSelByColumn(0, (long)m_nPresetProviderID);
		}
		// (r.gonet 2015-04-29 10:00) - PLID 65327 - Renamed to m_nPresetPurchasedByPatientID for consistency.
		if (m_nPresetPurchasedByPatientID != -1) {
			m_pPurchList->SetSelByColumn(plcPersonID, (long)m_nPresetPurchasedByPatientID);
		}

		// (r.gonet 2015-04-29 10:00) - PLID 65327 - The caller wants the value of the gift certificate prefilled.
		if (m_cyPresetValue != g_ccyNull) {
			cyDisplayedValue = m_cyPresetValue;
			if (m_cyPresetPrice == g_ccyNull) {
				cyPrice = m_cyPresetValue;
			}
		}

		// (r.gonet 2015-04-29 10:00) - PLID 65327 - The caller wants the price of the gift certificate prefilled.
		if (m_cyPresetPrice != g_ccyNull) {
			cyPrice = m_cyPresetPrice;
			if (m_cyPresetValue == g_ccyNull) {
				cyDisplayedValue = m_cyPresetPrice;
			}
		}

		// (r.gonet 2015-04-29 10:00) - PLID 65327 - The caller wants to lie to the user and tell them that
		// the value is something it is not. This is used in refunds where the value is credited by the refund 
		// line item existing rather than by gift certificate charge line item's GCValue. This avoids having to
		// show 0.00 for the Value, which would be misleading.
		if (m_cyPresetDisplayedValue != g_ccyNull) {
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Init the Value field to be the preset value.
			cyDisplayedValue = m_cyPresetDisplayedValue;
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Checke if there is a Price preset too, otherwise we'll just use the Value field's value for the Price field.
			if (m_cyPresetPrice == g_ccyNull) {
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Use the same amount as the Value field preset.
				cyPrice = m_cyPresetDisplayedValue;
			}
		}

		// (r.gonet 2015-04-29 10:00) - PLID 65327 - The caller wants to preset the purchase date.
		if (m_bPurchaseDatePreset) {
			m_dtPurchase.SetValue(_variant_t(m_dtPresetPurchaseDate));
		}

		// (r.gonet 2015-04-29 10:00) - PLID 65327 - The caller wants to preset the expiration date. This needs to
		// have a separate flag variable because there is a checkbox controlling whether the expiration date is enabled
		// or not. If there was just the one COleDateTime variable, then its value of g_cdtNull would be ambiguous.
		if (m_bExpirationDatePreset) {
			if (m_dtPresetExpirationDate != g_cdtNull) {
				CheckDlgButton(IDC_EXP_DATE_CHECK, BST_CHECKED);
				m_dtExpire.SetValue(_variant_t(m_dtPresetExpirationDate));
			} else {
				CheckDlgButton(IDC_EXP_DATE_CHECK, BST_UNCHECKED);
			}
		}

		// (r.gonet 2015-03-24 18:05) - PLID 66033 - If this isn't a refund, then set the type using the preset if there is one.
		if (m_eCreationStyle != GCCreationStyle::eRefundToNewGC) {
			if (m_nPresetServiceID != -1) {
				nTypeID = m_nPresetServiceID;
			}
		}

		// (r.gonet 2015-03-24 18:05) - PLID 66033 - Select the type.
		if (nTypeID != -1) {
			long nRow = m_pTypeList->SetSelByColumn(0, nTypeID);
			OnSelChosenGcTypeList(nRow);
		}

		// (r.gonet 2015-04-20) - PLID 65327 - If the caller has given us a location to preselect, select the location.
		if (m_nPresetLocationID != -1) {
			m_pLocationList->SetSelByColumn(0, (long)m_nPresetLocationID);
		} else {
			//auto select the location to the current one
			m_pLocationList->SetSelByColumn(0, (long)GetCurrentLocationID());
		}

		//set the default patient to the current one
		// (v.maida 2015-03-10 12:33) - PLID 60126 - Only do this if the purchaser list hasn't already been set, which is the case if m_nPresetPurch == -1.
		if (m_nPresetPurchasedByPatientID == -1) {
			m_pPurchList->SetSelByColumn(0, (long)GetActivePatientID());
		}

		// (r.gonet 2015-04-29 10:23) - PLID 65327 - The caller wants to preset the patient the gift certificate is
		// received by.
		if (m_bReceivedByPatientIDPreset) {
			if (m_pRecList->TrySetSelByColumn(plcPersonID, m_nPresetReceivedByPatientID) == NXDATALISTLib::sriNoRow) {
				// (r.gonet 2015-04-29 10:23) - PLID 65327 - Failed to select the patient. Revert to Unknown.
				CheckDlgButton(IDC_REC_TYPE_UNKNOWN, BST_CHECKED);
				CheckDlgButton(IDC_REC_TYPE_PAT, BST_UNCHECKED);
			} else {
				CheckDlgButton(IDC_REC_TYPE_UNKNOWN, BST_UNCHECKED);
				CheckDlgButton(IDC_REC_TYPE_PAT, BST_CHECKED);
			}
		} else {
			//set the receiver to "unknown"
			CheckDlgButton(IDC_REC_TYPE_UNKNOWN, BST_CHECKED);
			CheckDlgButton(IDC_REC_TYPE_PAT, BST_UNCHECKED);
		}
		EnsurePatInfo();

		// (r.gonet 2015-05-13 14:26) - PLID 65276 - Load the Value and Price fields in one spot. Also this way we have 
		// access to the final value and price at the end of the function.
		SetDlgItemText(IDC_CERT_VALUE_EDIT, FormatCurrencyForInterface(cyDisplayedValue));
		SetDlgItemText(IDC_CERT_PRICE_EDIT, FormatCurrencyForInterface(cyPrice));
		if (!CheckCurrentUserPermissions(bioEditGiftCertificatePrice, sptWrite, FALSE, 0, TRUE)
			&& cyDisplayedValue != cyPrice) {
			// (r.gonet 2015-05-13 14:26) - PLID 65276 - If the user doesn't have permission to change the Price and the Price
			// started out as different from the value, then flag the price never, ever to sync with the value. Decided by product 
			// management.
			m_bSyncPriceAllowed = false;
		} else {
			m_bSyncPriceAllowed = true;
		}

		//register for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this))
				MsgBox("Error registering for barcode scans.  You may not be able to scan.");
		}
	}
	NxCatchAll("Error in CGCEntryDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGCEntryDlg::GenerateNewGCID()
{
	try {
		//TODO:  This is not safe for multiple - if you hit new, then someone else adds one, they'll take your number.

		CString strNewID = "1";

		//we can't calculate a new number if there is a number more than 15 characters long
		//(ok, I think it's actually 18 nines in a row, but let's just cut it off at 18)

		//also we can't properly calculate a new number if not all of the GCs are proper numbers,
		//for example, if they don't use a barcode scanner and have 1, 2, 3, 4, 5, etc. then 
		//we can calculate a new number, but if they have 0123450 and 0123460, then we can't accurately
		//handle those, so we can't calculate new numbers
		//(you must add e0 to the end of anything you run isnumeric() on to avoid valid fields like decimals and dollar signs)
		if(ReturnsRecords("SELECT ID FROM GiftCertificatesT WHERE Len(GiftID) > 15 OR GiftID = '' OR "
			"isnumeric(GiftID + 'e0') = 0 OR Convert(nvarchar,(Convert(numeric, GiftID))) <> GiftID")) {
			SetDlgItemText(IDC_CERT_NUMBER, "");
			return;
		}


		_RecordsetPtr rs = CreateRecordset("SELECT Convert(nvarchar, Max(Convert(numeric, GiftID)) + 1) AS MaxID FROM GiftCertificatesT");
		if(!rs->eof) {
			strNewID = AdoFldString(rs, "MaxID","1");	
		}
		rs->Close();

		SetDlgItemText(IDC_CERT_NUMBER, strNewID);

	} NxCatchAll("Error generating new gift ID.");
}

void CGCEntryDlg::OnOK() 
{
	try {
		// check permissions (Ability to create a bill)
		if(!CheckCurrentUserPermissions(bioBill, sptCreate))
			return;


		//get the data out
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Rename Amount to Price. Added a variable for the new Value field.
		CString strValue, strPrice, strRec, str, strExpFormat, strProv, strGiftID;
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - When I parameterized the queries below, these needed some variants rather than doing the NULL thing in the a string.
		_variant_t varExpFormat, varRec;
		COleCurrency cyValue, cyPrice;
		COleDateTime dtPurch, dtExp;
		long nPurch = -1, nRec = -1, nTypeID = -1, nProv = 0, nLocID = -1;

		GetDlgItemText(IDC_CERT_NUMBER, strGiftID);
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Get the value of the Value field.
		// (r.gonet 2015-04-29 10:23) - PLID 65327 - If there is a displayed value in place, then
		// the real value is hidden from the user and we should use that instead of what is in the 
		// edit box.
		if (m_cyPresetDisplayedValue != g_ccyNull && m_cyPresetValue != g_ccyNull) {
			cyValue = m_cyPresetValue;
		} else {
			GetDlgItemText(IDC_CERT_VALUE_EDIT, strValue);
			cyValue = ParseCurrencyFromInterface(strValue);
		}
		
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Rename Amount to Price.
		GetDlgItemText(IDC_CERT_PRICE_EDIT, strPrice);
		cyPrice = ParseCurrencyFromInterface(strPrice);
		dtExp = (COleDateTime)m_dtExpire.GetValue();
		dtPurch = (COleDateTime)m_dtPurchase.GetValue();

		strGiftID.TrimLeft();
		strGiftID.TrimRight();

		//ensure that the gift ID is numeric, and under 50 characters
		{
			if(strGiftID.GetLength() == 0) {
				MsgBox("You must enter a gift ID.");
				return;
			}

			if(strGiftID.GetLength() > 50) {
				MsgBox("You cannot have a gift ID that is over 50 characters long.");
				return;
			}

			// (j.jones 2009-10-07 10:47) - PLID 35835 - removed restriction for numeric only,
			// instead required that there are no spaces
			/*
			if(strGiftID.SpanIncluding("0123456789") != strGiftID) {
				MsgBox("You have entered an invalid gift ID.  A gift ID must be numeric only.");
				return;
			}
			*/
			if(strGiftID.SpanExcluding(" ") != strGiftID) {
				MsgBox("You have entered an invalid gift ID. A gift ID must not have spaces in it.");
				return;
			}
		}

		if(m_pLocationList->GetCurSel() == sriNoRow) {
			MsgBox("You must select a location.");
			return;
		}

		nLocID = VarLong(m_pLocationList->GetValue(m_pLocationList->GetCurSel(), 0));

		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Ensure the Value field's value is valid...vvv
		if (cyValue.GetStatus() != COleCurrency::valid) {
			MsgBox("You must specify a valid value for the Cert. Value field.");
			return;
		}
		RoundCurrency(cyValue);

		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Rename Amount to Price.
		if(cyPrice.GetStatus() != COleCurrency::valid) {
			MsgBox("You must specify a valid value for the Cert. Price field.");
			return;
		}
		RoundCurrency(cyPrice);

		//get the persons
		{
			long nCurSel;

			nCurSel = m_pPurchList->GetCurSel();
			if(nCurSel == -1) {
				MsgBox("You must select a purchaser.");
				return;
			}

			nPurch = VarLong(m_pPurchList->GetValue(nCurSel, plcPersonID));

			if(IsDlgButtonChecked(IDC_REC_TYPE_PAT)) {
				nCurSel = m_pRecList->GetCurSel();
				if(nCurSel == -1) {
					MsgBox("You must select a receiver.");
					return;
				}

				nRec = VarLong(m_pRecList->GetValue(nCurSel, plcPersonID));
			}
			else if(IsDlgButtonChecked(IDC_REC_TYPE_UNKNOWN))
				nRec = -1;	//-1 = unknown

			nCurSel = m_pTypeList->GetCurSel();
			if(nCurSel == -1) {
				MsgBox("You must select a gift certificate type before continuing.");
				return;
			}
			nTypeID = VarLong(m_pTypeList->GetValue(nCurSel, 0));
		}

		//if they uncheck the expiration date box then GetValue() will return an invalid date
		if(dtExp.GetStatus() != COleDateTime::valid && IsDlgButtonChecked(IDC_EXP_DATE_CHECK)) {
			MsgBox("You must choose a valid expiration date.");
			return;
		}

		if(dtPurch.GetStatus() != COleDateTime::valid) {
			MsgBox("You must choose a valid purchase date.");
			return;
		}

		//make sure this certificate is unique (there is a constraint that forces this, but we obviously don't want an error)
		{
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM GiftCertificatesT WHERE GiftID = {STRING}", strGiftID);
			if(!prs->eof) {

				// (j.jones 2005-11-10 12:03) - PLID 17710 - don't ask to generate a new ID if we can't do so
				if(ReturnsRecords("SELECT ID FROM GiftCertificatesT WHERE Len(GiftID) > 15 OR GiftID = '' OR "
					"isnumeric(GiftID + 'e0') = 0 OR Convert(nvarchar,(Convert(numeric, GiftID))) <> GiftID")) {
					MsgBox("This certificate ID is already in use.  You must use a unique ID.");
				}
				else {
					if(IDYES == MsgBox(MB_YESNO, "This certificate ID is already in use.  You must use a unique ID.\r\n"
						"Would you like to reset the ID?")) {
						GenerateNewGCID();
					}
				}

				return;
			}
		}

		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Use a variant rather than a string to record a NULL value.
		if (nRec == -1) {
			varRec = g_cvarNull;
		} else {
			varRec = _variant_t(nRec, VT_I4);
		}

		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Use a variant rather than a string to cerdo a NULL value.
		if (IsDlgButtonChecked(IDC_EXP_DATE_CHECK)) {
			varExpFormat = _variant_t(dtExp, VT_DATE);
		} else {
			varExpFormat = g_cvarNull;
		}

		if(m_pProvList->GetCurSel() == -1) {
			strProv.Format("NULL");

			// (j.jones 2011-07-08 17:36) - PLID 32067 - added pref. to force selection of a provider
			if(GetRemotePropertyInt("RequireProviderOnCharges", 1, 0, "<None>", true) == 1) {
				MessageBox("You have not selected a provider. All new gift certificates must have providers selected.","Practice",MB_ICONEXCLAMATION|MB_OK);
				return;
			}
			else {
				// (j.jones 2013-07-12 10:00) - PLID 57539 - if invoice numbers are turned on, give a different warning
				CString strWarn = "You have not selected a provider. Are you sure you wish to continue saving this gift certificate?";
				if(GetRemotePropertyInt("EnableBillInvoiceNumbers", 0, 0, "<None>", true) == 1) {
					strWarn = "You have not selected a provider. If you continue, this bill will not have an invoice number created.\n\n"
						"Are you sure you wish to continue saving this gift certificate?";
				}
				if(IDNO == MessageBox(strWarn,"Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
		}
		else {
			nProv = VarLong(m_pProvList->GetValue(m_pProvList->GetCurSel(), 0));
			strProv.Format("%li", nProv);
		}
		// (r.gonet 2015-04-29 10:23) - PLID 65327 - Get the certificate number so the dialog's callers can see what was entered.
		m_nxeditCertNumber.GetWindowText(m_strCertNumber);

		// set up auditing variables

		if(m_nID == -1) {
			//new GC
			_variant_t var;
			// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here.
			// If we do, then if NOCOUNT is on, then the call to NextRecordset will return NULL
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Parameterized.
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT OFF \r\n"
				"INSERT INTO GiftCertificatesT (GiftID,  ExpDate, PurchaseDate, PurchasedBy, ReceivedBy, DefaultTypeID, LocationID) values \r\n"
				"({STRING}, dbo.AsDateNoTime({VT_DATE}), {OLEDATETIME}, {INT}, {VT_I4}, {INT}, {INT}); \r\n"
				"SELECT Convert(int, SCOPE_IDENTITY()) AS NewNum; \r\n", 
				strGiftID, varExpFormat, dtPurch, nPurch, varRec, nTypeID, nLocID);
			prs = prs->NextRecordset(&var);	//move to next recordset to get the id we just created
			if (NULL == prs) {
				// (c.haag 2006-02-15 09:04) - PLID 19051 - If this is NULL, something bad happened
				AfxThrowNxException("The gift certificate could not be made. Please try saving your changes again or restart NexTech Practice if the problem persists.");
			}
			m_nID = AdoFldLong(prs, "NewNum");	//get the id out

			// a.walling 4/12/06 audit the new GC
			CString oldVal = "";
			CString newVal;
			CString strWhom;
			CString strType;

			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Parameterized.
			_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM GCTypesT INNER JOIN ServiceT ON ServiceID = ID WHERE ServiceID = {INT}", nTypeID);
			if (!rs->eof) {
				strType = AdoFldString(rs, "Name");
			} else {
				strType = "Undefined";
			}

			if (nRec == -1) {
				strWhom = "Anyone";
			} else {
				strWhom = GetExistingPatientName(nRec);
			}

			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Replaced the Amount with the Value, which is more useful than the price originally paid for it.
			newVal.Format("Gift Certificate ($%s %s for %s, ID %s)", cyValue.Format(0), strType, strWhom, strGiftID);
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(nPurch, GetExistingPatientName(nPurch), nAuditID, aeiGiftCCreated, m_nID, oldVal, newVal, aepHigh, aetChanged);
		}
		else {
			// editing an existing, save any changes
			// a.walling 4/13/06 todo can this be done? we need to audit
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Parameterized.
			ExecuteParamSql(
				"UPDATE GiftCertificatesT SET "
					"GiftID = {STRING}, "
					"ExpDate = dbo.AsDateNoTime({VT_DATE}), "
					"PurchaseDate = dbo.AsDateNoTime({OLEDATETIME}), "
					"PurchasedBy = {INT}, "
					"ReceivedBy = {VT_I4}, "
					"DefaultTypeID = {INT}, "
					"LocationID = {INT} "
				"WHERE ID = {INT}", 
				strGiftID, varExpFormat, dtPurch, nPurch, varRec, nTypeID, nLocID, m_nID);
		}

		/////////////////////////
		//OK, we are fully saved, now we need to make a bill for this

		// (r.gonet 2015-04-29 10:23) - PLID 65327 - Changed the billing dialog flag to a special caller enum.
		// (j.jones 2015-05-11 17:04) - PLID 65714 - changed this to follow m_eCreationStyle instead
		if (m_eCreationStyle != GCCreationStyle::eCreateNewGC_FromBill) {

			//The user did not make a bill first, so need to create the bill/charge, and prompt for a payment.

			long nBillID = -1;
			try {
				CString strGCType = FormatString("Gift Certificate - %s", VarString(m_pTypeList->GetValue(m_pTypeList->GetCurSel(), 1)));	//we checked for -1 sel above

				long nServiceSel = m_pTypeList->GetCurSel();
				CString strItemCode = VarString(m_pTypeList->GetValue(nServiceSel, 1), "");	//we checked above for a -1 cursel
				// (j.jones 2015-03-18 14:24) - PLID 64974 - Category and SubCategory are now nullable
				_variant_t varCategoryID = g_cvarNull;
				_variant_t varSubCategoryID = g_cvarNull;
				GetCategoriesFromServiceID(nTypeID, varCategoryID, varSubCategoryID);

				CParamSqlBatch sql;
				// (j.jones 2009-09-21 11:37) - PLID 35564 - -1 is no longer allowed for insured party IDs, so set to NULL
				// (j.armen 2013-06-28 10:37) - PLID 57367 - Idenitate BillsT
				sql.Declare(
					"SET NOCOUNT ON\r\n"
					"DECLARE @BillID INT\r\n"
					"DECLARE @LineItemID INT\r\n"
					"DECLARE @ChargeRespID INT\r\n"
					"DECLARE @NewInvoiceID INT\r\n");
				
				// (r.gonet 2015-05-14 18:05) - PLID 66033 - Append the refund amount if this is a refund to new gift certificate.
				CString strBillDescription = strGCType;
				if (m_eCreationStyle == GCCreationStyle::eRefundToNewGC) {
					strBillDescription += FormatString(" - %s", FormatCurrencyForInterface(m_cyPresetDisplayedValue));
				}
				sql.Add(
					"INSERT INTO BillsT (\r\n"
					"	PatientID, Date, EntryType, Location, InputName, Description\r\n"
					") VALUES (\r\n"
					"	{INT}, dbo.AsDateNoTime({OLEDATETIME}), 1, {INT}, {INT}, {STRING})\r\n"
					"SET @BillID = SCOPE_IDENTITY()\r\n\r\n",
					nPurch, dtPurch, nLocID, GetCurrentUserID(), strBillDescription);

				// (j.jones 2013-07-12 10:00) - PLID 57539 - if invoice numbers are turned on, auto-generate one
				if(GetRemotePropertyInt("EnableBillInvoiceNumbers", 0, 0, "<None>", true) == 1 && nProv > 0) {
					sql.Add("SELECT @NewInvoiceID = COALESCE(MAX(InvoiceID), 0) + 1 FROM BillInvoiceNumbersT WITH (UPDLOCK, HOLDLOCK) WHERE ProviderID = {INT} \r\n"
						"INSERT INTO BillInvoiceNumbersT (BillID, ProviderID, InvoiceID) VALUES (@BillID, {INT}, @NewInvoiceID) \r\n",
						nProv, nProv);
				}

				// (j.armen 2013-06-28 12:47) - PLID 57373 - Idenitate LineItemT
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Renamed Amount to Price.
				// (r.gonet 2015-03-27 18:58) - PLID 65279 - Save the value as well.
				sql.Add(
					"INSERT INTO LineItemT (\r\n"
					"	PatientID, Type, Amount, Description, Date,\r\n"
					"	InputName, LocationID, GiftID, GCValue\r\n"
					") VALUES (\r\n"
					"	{INT}, {INT}, {OLECURRENCY}, {STRING}, dbo.AsDateNoTime({OLEDATETIME}),\r\n"
					"	{STRING}, {INT}, {INT}, {OLECURRENCY})\r\n"
					"SET @LineItemID = SCOPE_IDENTITY()\r\n\r\n",
					nPurch, 10, cyPrice, strGCType, dtPurch,
					GetCurrentUserName(), nLocID, m_nID, cyValue);

				// (j.jones 2015-03-18 14:24) - PLID 64974 - Category and SubCategory are now nullable, 0 is no longer a valid sentinel value
				// (c.haag 2016-05-02 10:36) - NX-100403 - DoctorsProviders is nullable now
				sql.Add(
					"INSERT INTO ChargesT (\r\n"
					"	ID, BillID, ServiceID, ItemCode, \r\n"
					"	Category, SubCategory, \r\n"
					"	TaxRate, Quantity, DoctorsProviders, LineID,\r\n"
					"	TaxRate2, ServiceDateFrom, ServiceDateTo\r\n"
					") VALUES (\r\n"
					"	@LineItemID, @BillID, {INT}, {STRING}, \r\n"
					"	{VT_I4}, {VT_I4}, \r\n"
					"	{INT}, {INT}, {VT_I4}, {INT},\r\n"
					"	{INT}, dbo.AsDateNoTime({OLEDATETIME}), dbo.AsDateNoTime({OLEDATETIME}))\r\n\r\n",
					nTypeID, strItemCode,
					varCategoryID, varSubCategoryID,
					1, 1, (nProv > 0) ? _variant_t((long)nProv) : g_cvarNull, 1,
					1, dtPurch, dtPurch);

				// (j.armen 2013-06-28 15:03) - PLID 57383 - Idenitate ChargeRespT
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Renamed Gift Cert Amount to Price.
				sql.Add(
					"INSERT INTO ChargeRespT (ChargeID, Amount) VALUES (@LineItemID, {OLECURRENCY})\r\n"
					"SET @ChargeRespID = SCOPE_IDENTITY()\r\n\r\n",
					cyPrice);

				// (j.armen 2013-06-28 16:49) - PLID 57384 - Idenitate ChargeRespDetailT
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Renamed Gift Cert Amount to Price.
				sql.Add(
					"INSERT INTO ChargeRespDetailT (ChargeRespID, Amount, Date)\r\n"
					"	VALUES (@ChargeRespID, {OLECURRENCY}, dbo.AsDateNoTime({OLEDATETIME}))\r\n",
					cyPrice, dtPurch);

				sql.Declare(
					"SELECT @BillID AS BillID\r\n"
					"SET NOCOUNT OFF");

				_RecordsetPtr prs = sql.CreateRecordset(GetRemoteData());

				nBillID = AdoFldLong(prs, "BillID");

			} NxCatchAllCall("Error creating a bill for the current gift certificate.", { return; });

			//Done creating a bill
			/////////////////////////

			// (r.gonet 2015-04-20) - PLID 65327 - Launch the payments dialog. If the caller was the 
			// payments dialog, then we should not launch it because the line item will come into existence
			// with the saving of the refund.
			// (j.jones 2015-05-11 17:04) - PLID 65714 - do not prompt for a payment on refunds and transfers
			if (m_eCreationStyle != GCCreationStyle::eRefundToNewGC && m_eCreationStyle != GCCreationStyle::eTransferToNewGC) {
				/////////////////////////
				//Now we need to create the payment dialog, and tell it that it is
				//	applied to the bill we just created.
				CPaymentDlg dlg(this);
				COleCurrency cyCharges, cyPays, cyAdj, cyRef, cyIns;

				if (!GetBillTotals(nBillID, nPurch, &cyCharges, &cyPays, &cyAdj, &cyRef, &cyIns)) {
					//this should not be able to happen
					MsgBox("The bill could not be found, you will have to manually create a payment for this gift certificate.");
					return;
				}

				//set the MaxBalance to the patient balance
				COleCurrency cyMaxBalance;
				cyMaxBalance = cyCharges - cyPays - cyAdj - cyRef - cyIns;
				_variant_t var(nBillID);

				dlg.m_cyFinalAmount = cyMaxBalance;
				dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
				dlg.m_varBillID = var;
				dlg.m_ApplyOnOK = TRUE;
				dlg.m_nGiftID = m_nID;

				dlg.DoModal(__FUNCTION__, __LINE__);
			} else {
				// (r.gonet 2015-04-20) - PLID 65327 - If we are coming from the payment dialog, then there is no price, so don't show the payment dialog. Plus that would be re-entrant.
			}

			//update the views so this gets shown correctly
			GetMainFrame()->UpdateAllViews();
			//
			/////////////////////////
		}	//end of !m_bFromBilling
		else {
			//DRT TODO:  This report is ugly.  Very ugly.  We're operating on the current
			//	assumption that most people are going to use pre-printed GC's.  We need
			//	a preference to show this automatically, but for now, it's commented out.
			//dlg.ViewReport();
		}

	} NxCatchAll("Error in OnOK()");

	//unregister for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->UnregisterForBarcodeScan(this))
			MsgBox("Error unregistering for barcode scans.");
	}

	CDialog::OnOK();
}

void CGCEntryDlg::OnCancel() 
{
	// (r.gonet 2015-04-29 10:23) - PLID 65327 - Use a caller enum rather than the billing flag.
	// (j.jones 2015-05-11 17:04) - PLID 65714 - changed this to follow m_eCreationStyle instead
	if (m_eCreationStyle == GCCreationStyle::eCreateNewGC_FromBill) {
		if (MsgBox(MB_YESNO, "If you cancel, this line item will not be saved on the bill.  Are you sure you wish to continue?") != IDYES)
			return;
	}
	else if (m_eCreationStyle == GCCreationStyle::eRefundToNewGC) {
		// (r.gonet 2015-04-29 10:23) - PLID 65327 - If this has been called by the payments dialog, then be more specific about
		// what is being aborted. Currently only refunds can launch this dialog.
		if (MsgBox(MB_YESNO, "If you cancel, the refund will not be saved.  Are you sure you wish to continue?") != IDYES)
			return;
	}
	// (j.jones 2015-05-11 17:04) - PLID 65714 - supported balance transfers
	else if (m_eCreationStyle == GCCreationStyle::eTransferToNewGC) {
		if (MsgBox(MB_YESNO, "If you cancel, the transfer will not be saved.  Are you sure you wish to continue?") != IDYES)
			return;
	}
	else {
		//only warn if they've already selected a type
		if(m_pTypeList->GetCurSel() != sriNoRow) {
			if(MsgBox(MB_YESNO, "Are you sure you wish to cancel?") != IDYES)
				return;
		}
	}

	//unregister for barcode messages
	if(GetMainFrame()) {
		if(!GetMainFrame()->UnregisterForBarcodeScan(this))
			MsgBox("Error unregistering for barcode scans.");
	}

	CDialog::OnCancel();
}

void CGCEntryDlg::OnRecTypePat() 
{
	EnsurePatInfo();
}

#define ENABLE_WND(idc, bEnable)	((CWnd*)GetDlgItem(idc))->EnableWindow(bEnable);

void CGCEntryDlg::EnsurePatInfo()
{
	BOOL bEnable = TRUE;

	if (m_eCreationStyle == GCCreationStyle::eRefundToNewGC) {
		// (r.gonet 2015-07-06 17:03) - PLID 65327 - The user should not be able to select a different patient when refunding to a new gift certificate.
		bEnable = FALSE;
	}
	//enable or disable receiver windows
	else if(!IsDlgButtonChecked(IDC_REC_TYPE_PAT))
		bEnable = FALSE;

	ENABLE_WND(IDC_REC_PAT_LIST, bEnable);
}

void CGCEntryDlg::OnOpenGcTypes() 
{
	try {
		CGCTypeDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//requery the list of types
			long nSel = m_pTypeList->GetCurSel();
			long nID = -1;
			if(nSel != -1)
				nID = VarLong(m_pTypeList->GetValue(nSel, tlcID));

			m_pTypeList->Requery();

			//reset selection
			m_pTypeList->SetSelByColumn(tlcID, (long)nID);
		}
	} NxCatchAll("Error opening type dialog.");
}

BEGIN_EVENTSINK_MAP(CGCEntryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGCEntryDlg)
	ON_EVENT(CGCEntryDlg, IDC_GC_TYPE_LIST, 16 /* SelChosen */, OnSelChosenGcTypeList, VTS_I4)
	ON_EVENT(CGCEntryDlg, IDC_REC_PAT_LIST, 20 /* TrySelSetFinished */, OnTrySetSelFinishedRecPatList, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CGCEntryDlg::OnSelChosenGcTypeList(long nRow) 
{
	if(nRow == -1) 
		return;

	try {
		//once a type is chosen, default the information in the value, price and expiration fields
		//all the info we need is stored in the datalist, just pull it out

		//set the value and the price
		COleCurrency cyDisplayedValue = g_ccyZero; 
		COleCurrency cyPrice = g_ccyZero;
		//m.hancock - 2/3/2006 - PLID 14624 - GC made from billing use the standard value and/or price instead of the value and/or price from the charge
		// (r.gonet 2015-04-29 10:23) - PLID 65327 - Use the caller enum. Also I suppose the preset displayed value, if set, would also count.
		// (j.jones 2015-05-11 17:04) - PLID 65714 - changed this to follow m_eCreationStyle instead
		if (m_eCreationStyle == GCCreationStyle::eCreateNewGC_FromBill && (m_cyPresetValue != g_ccyNull || m_cyPresetDisplayedValue != g_ccyNull)) { //Check if we're coming from a bill and have a set value
			if (m_cyPresetDisplayedValue != g_ccyNull) {
				// (r.gonet 2015-03-24 18:05) - PLID 65327 - Init the Value field to be the preset displayed value (lie to the user).
				cyDisplayedValue = m_cyPresetDisplayedValue;
			} else {
				// (r.gonet 2015-04-28 18:05) - PLID 65276 - Init the Value field to be the preset value.
				cyDisplayedValue = m_cyPresetValue;
			}
			
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Check if there is a Price preset too, otherwise we'll just use the Value field's value for the Price field.
			if (m_cyPresetPrice != g_ccyNull) {
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Use the preset Price.
				cyPrice = m_cyPresetPrice;
			} else {
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Use the same amount as the Value field preset.
				cyPrice = cyDisplayedValue;
			}
		}
		// (j.jones 2015-05-11 17:04) - PLID 65714 - supported balance transfers, same logic here as refunds
		else if (m_eCreationStyle == GCCreationStyle::eRefundToNewGC || m_eCreationStyle == GCCreationStyle::eTransferToNewGC) {
			// (r.gonet 2015-04-20) - PLID 65327 - We had better have a value and price preset since we have disabled those fields and are not allowing them to be changed by the type combo box.
			ASSERT((m_cyPresetValue != g_ccyNull || m_cyPresetDisplayedValue != g_ccyNull) && m_cyPresetPrice != g_ccyNull);
			if (m_cyPresetDisplayedValue != g_ccyNull) {
				// (r.gonet 2015-04-28 18:05) - PLID 65327 - Init the Value field to be the preset displayed value (lie to the user).
				cyDisplayedValue = m_cyPresetDisplayedValue;
			} else {
				// (r.gonet 2015-03-24 18:05) - PLID 65276 - Init the Value field to be the preset value.
				cyDisplayedValue = m_cyPresetValue;
			}
			cyPrice = m_cyPresetPrice;
		}
		else { //default
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Load the Value field from the combo.
			cyDisplayedValue = VarCurrency(m_pTypeList->GetValue(nRow, tlcValue), COleCurrency(0, 0));
			// (r.gonet 2015-03-24 18:05) - PLID 65276 - Renamed Amount to Price.
			cyPrice = VarCurrency(m_pTypeList->GetValue(nRow, tlcPrice), COleCurrency(0, 0));
		}

		//update the expiration appropriately
		if(VarShort(m_pTypeList->GetValue(nRow, tlcExpires)) == 0) {
			//does not expire
			CheckDlgButton(IDC_EXP_DATE_CHECK, FALSE);
			OnExpDateCheck();
		}
		else {
			//it does expire!
			CheckDlgButton(IDC_EXP_DATE_CHECK, TRUE);
			OnExpDateCheck();

			//and set the date to x days from now
			long nDays = VarLong(m_pTypeList->GetValue(nRow, tlcDays), 0);

			COleDateTime dt = COleDateTime(m_dtPurchase.GetValue());
			COleDateTimeSpan dtSpan(nDays, 0, 0, 0);

			dt += dtSpan;

			m_dtExpire.SetValue(_variant_t(dt));
		}

		// (r.gonet 2015-05-13 14:26) - PLID 65276 - Load the Value and Price fields in one spot. Also this way we have 
		// access to the final value and price at the end of the function.
		SetDlgItemText(IDC_CERT_VALUE_EDIT, FormatCurrencyForInterface(cyDisplayedValue));
		SetDlgItemText(IDC_CERT_PRICE_EDIT, FormatCurrencyForInterface(cyPrice));
		if (!CheckCurrentUserPermissions(bioEditGiftCertificatePrice, sptWrite, FALSE, 0, TRUE)
			&& cyDisplayedValue != cyPrice) {
			// (r.gonet 2015-05-13 14:26) - PLID 65276 - If the user doesn't have permission to change the Price and the Price
			// started out as different from the value, then flag the price never, ever to sync with the value. Decided by product 
			// management.
			m_bSyncPriceAllowed = false;
		} else {
			m_bSyncPriceAllowed = true;
		}
	} NxCatchAll("Error after choosing gift certificate type.");
}

void CGCEntryDlg::OnAddNewPurchaser() 
{
	AddNewPatient(m_pPurchList);
}

void CGCEntryDlg::OnAddNewReceiver() 
{
	AddNewPatient(m_pRecList);
}

void CGCEntryDlg::OnRecTypeUnknown() 
{
	EnsurePatInfo();
}

void CGCEntryDlg::AddNewPatient(_DNxDataListPtr pList)
{
	try {
		if (!UserPermission(NewPatient))
			return;

		// (z.manning, 04/16/2008) - PLID 29680 - Removed all references to obsolete 640x480 dialogs
		CNewPatient dlg(NULL, IDD_NEW_PATIENT);
		dlg.m_bForGC = true;

		long id = -1;
		long choice = dlg.DoModal(&id);

		// (j.gruber 2010-10-04 13:19) - PLID 40413 - check to see whether we are opening the security groups dialog
		long nOpen = GetRemotePropertyInt("NewPatientOpenSecurityGroups", 0, 0, GetCurrentUserName(), true);

		//they want to open it and they didn't cancel and they have permission		
		// (j.gruber 2010-10-26 13:56) - PLID 40416 - change to assign permission, instead of write
		BOOL bHasPerms = CheckCurrentUserPermissions(bioSecurityGroup, sptDynamic0, 0, 0, TRUE, TRUE);		
		if (id > -1 && nOpen == 1 && bHasPerms && choice != 0) {		
			
			CSecurityGroupsDlg dlg(this);
			dlg.m_nPatientID = id;
			dlg.DoModal();
		}

		if(choice == 0) {
			//cancelled, do nothing for now
		}
		else if(choice == 1) {
			//new patient added
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			GetMainFrame()->m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient);

			//and we need to add them to the datalist (both have the same columns)
			_RecordsetPtr prs = CreateRecordset("SELECT "
				"PatientsT.PersonID, UserDefinedID, Last + ', ' + First + ' ' + Middle AS Name, HomePhone, Address1, City, State, Zip "
				"FROM PersonT INNER JOIN PatientsT ON ID = PersonID WHERE PersonID = %li", id);

			if(!prs->eof) {
				IRowSettingsPtr pRow = pList->GetRow(-1);
				pRow->PutValue(plcPersonID, prs->Fields->Item["PersonID"]->Value);
				pRow->PutValue(plcUserID, prs->Fields->Item["UserDefinedID"]->Value);
				pRow->PutValue(plcName, prs->Fields->Item["Name"]->Value);
				pRow->PutValue(plcPhone, prs->Fields->Item["HomePhone"]->Value);
				pRow->PutValue(plcAddress, prs->Fields->Item["Address1"]->Value);
				pRow->PutValue(plcCity, prs->Fields->Item["City"]->Value);
				pRow->PutValue(plcState, prs->Fields->Item["State"]->Value);
				pRow->PutValue(plcZip, prs->Fields->Item["Zip"]->Value);

				pList->AddRow(pRow);
			}
			else {
				//something wierd happened here and we couldnt find the pt we 
				//just added... so just requery the whole list
				pList->Requery();
			}

			//now try to select the patient we just added
			pList->SetSelByColumn(plcPersonID, (long)id);

			//TES 8/13/2014 - PLID 63194 - We already sent this up above
			//send a table checker message so everyone knows
			//CClient::RefreshTable(NetUtils::PatCombo, id);
		}
	} NxCatchAll("Error in AddNewPatient()");
}

void CGCEntryDlg::OnExpDateCheck() 
{
	if(IsDlgButtonChecked(IDC_EXP_DATE_CHECK)) 
		((CWnd*)GetDlgItem(IDC_EXP_DATE))->EnableWindow(TRUE);
	else
		((CWnd*)GetDlgItem(IDC_EXP_DATE))->EnableWindow(FALSE);
}

LRESULT CGCEntryDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	CString strNumber = (LPCTSTR)_bstr_t((BSTR)lParam);

	SetDlgItemText(IDC_CERT_NUMBER, strNumber);

	return 0;
}

// (r.gonet 2015-03-24 18:05) - PLID 65276 - Handler for when the Value field gains focus.
void CGCEntryDlg::OnEnSetfocusCertValueEdit()
{
	try {
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - The user may be about to edit the Value field now.
		// Get the two field values so we can know whether to sync them or not, if the user has permission
		// and the two fields are currently the same value.
		CString strValue, strPrice;
		GetDlgItemText(IDC_CERT_PRICE_EDIT, strPrice);
		GetDlgItemText(IDC_CERT_VALUE_EDIT, strValue);
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Allow even with no permission to change the price.
		if (strValue == strPrice) {
			m_bSyncPriceWithValue = true;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-03-24 18:05) - PLID 65276 - Added a handler for when the Value field is changed.
void CGCEntryDlg::OnEnChangeCertValueEdit()
{
	try {
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Check the control flag for whether we should sync
		// the value from Value with Price. This flag should only be set while the user is editing the
		// Value field, ie it has focus. This prevents programmatic SetDlgItemText calls from triggering
		// the sync, which is undesired.
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Product Management determined that the syncing between Value and Price
		// should be done even if the user doesn't have permission to change the Price. However, if the Price started off
		// as different from the Value and we don't have permission to change the Price, we don't want it to sync, we want it 
		// to be constant. Even if the Value changes to be the same as the Price, it won't start syncing suddenly since the
		// intent is the Price to be different and fixed.
		if (m_bSyncPriceAllowed && m_bSyncPriceWithValue) {
			CString str;
			GetDlgItemText(IDC_CERT_VALUE_EDIT, str);

			COleCurrency cyNewValue = ParseCurrencyFromInterface(str);
			RoundCurrency(cyNewValue);	//round it to 2 digits
			CString strNewPrice = FormatCurrencyForInterface(cyNewValue);

			SetDlgItemText(IDC_CERT_PRICE_EDIT, strNewPrice);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-03-24 18:05) - PLID 65276 - Handler for when the Value field loses focus.
void CGCEntryDlg::OnEnKillfocusCertValueEdit()
{
	try {
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Format the value entered by the user as currency.
		CString str;
		GetDlgItemText(IDC_CERT_VALUE_EDIT, str);

		COleCurrency cy;
		cy = ParseCurrencyFromInterface(str);
		RoundCurrency(cy);

		SetDlgItemText(IDC_CERT_VALUE_EDIT, FormatCurrencyForInterface(cy));
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Stop syncing the Value field with the Price field.
		m_bSyncPriceWithValue = false;
	} NxCatchAll(__FUNCTION__);
}

void CGCEntryDlg::OnKillfocusCertPrice()
{
	try {
		CString str;
		// (r.gonet 2015-03-24 18:05) - PLID 65276 - Renamed Amount to Price.
		GetDlgItemText(IDC_CERT_PRICE_EDIT, str);

		COleCurrency cy;
		cy = ParseCurrencyFromInterface(str);
		RoundCurrency(cy);

		SetDlgItemText(IDC_CERT_PRICE_EDIT, FormatCurrencyForInterface(cy));
	} NxCatchAll(__FUNCTION__);
}

//looks up the category and subcategory for a given service id
//values are set to 0 if no category exists
// (j.jones 2015-03-18 14:24) - PLID 64974 - Category and SubCategory are now nullable
void CGCEntryDlg::GetCategoriesFromServiceID(long nServiceID, _variant_t &varCategoryID, _variant_t &varSubCategoryID)
{
	try {
		_RecordsetPtr prs = CreateParamRecordset("SELECT "
			"CASE WHEN CategoriesT.Parent = 0 OR CategoriesT.Parent IS NULL THEN ServiceT.Category ELSE CategoriesT.Parent END AS Category, "
			"CASE WHEN CategoriesT.Parent = 0 OR CategoriesT.Parent IS NULL THEN 0 ELSE ServiceT.Category END AS SubCategory "
			"FROM ServiceT LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"WHERE ServiceT.ID = {INT}", nServiceID);
		if(prs->eof) {
			varCategoryID = g_cvarNull;
			varSubCategoryID = g_cvarNull;
		}
		else {
			long nCategoryID = AdoFldLong(prs, "Category", -1);
			//ignore 0 and -1
			if (nCategoryID > 0) {
				varCategoryID = (long)nCategoryID;
			}
			long nSubCategoryID = AdoFldLong(prs, "SubCategory", -1);
			if (nSubCategoryID > 0) {
				varSubCategoryID = (long)nSubCategoryID;
			}
		}

	} NxCatchAll("Error determining category for service.");
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the certificate number text field in the dialog.
void CGCEntryDlg::SetCertNumber(CString strCertNumber)
{
	m_strPresetCertNumber = strCertNumber;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the provider dropdown field in the dialog.
void CGCEntryDlg::SetProvider(long nID)
{
	m_nPresetProviderID = nID;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the location dropdown field in the dialog.
void CGCEntryDlg::SetLocation(long nID)
{
	m_nPresetLocationID = nID;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the purchased by patient dropdown field in the dialog.
void CGCEntryDlg::SetPurchasedByPatientID(long nID)
{
	m_nPresetPurchasedByPatientID = nID;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the received by patient dropdown field in the dialog.
// If nID is not a valid patient ID, then the Received By will be set to Unknown.
void CGCEntryDlg::SetReceivedByPatientID(long nID)
{
	m_bReceivedByPatientIDPreset = true;
	m_nPresetReceivedByPatientID = nID;
}

void CGCEntryDlg::SetService(long nID)
{
	m_nPresetServiceID = nID;
}

// (r.gonet 2015-03-25 09:38) - PLID 65276 - Sets the default value of the Value field in the dialog.
// If SetPrice is not called as well, then the price will be filled by default with the preset Value
// field value.
void CGCEntryDlg::SetValue(COleCurrency cy)
{
	m_cyPresetValue = cy;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - If called, then the amount shown in the Value field
// will use this rather than the amount set by SetValue. Should be called with SetValue because that
// amount will be what is saved with the gift certificate to LineItemT.GCValue.
void CGCEntryDlg::SetDisplayedValue(COleCurrency cy)
{
	m_cyPresetDisplayedValue = cy;
}

// (r.gonet 2015-03-25 09:38) - PLID 65276 - Renamed Amount to Price.
void CGCEntryDlg::SetPrice(COleCurrency cy)
{
	m_cyPresetPrice = cy;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Renamed slightly for consistency.
void CGCEntryDlg::SetPurchaseDate(COleDateTime dt)
{
	m_bPurchaseDatePreset = true;
	m_dtPresetPurchaseDate = dt;
}

// (r.gonet 2015-04-29 10:23) - PLID 65327 - Presets the expiraton date datetimepicker field.
// If dt is g_cdtNull, then there will be no expiration date.
void CGCEntryDlg::SetExpirationDate(COleDateTime dt)
{
	m_bExpirationDatePreset = true;
	m_dtPresetExpirationDate = dt;
}

void CGCEntryDlg::ViewReport()
{
	try {
		//preview the GC report
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(497)]);
		//set the filter for the ID to the current GC
		infReport.nExtraID = m_nID;

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, true, (CWnd *)this, "Gift Certificate");
	} NxCatchAll("Error viewing gift certificate report.");
}

// (r.gonet 2015-07-06 18:02) - PLID 65327 - In the dialog initialization, we may have tried to preset
// the receiving patient. This is the async end to that.
void CGCEntryDlg::OnTrySetSelFinishedRecPatList(long nRowEnum, long nFlags)
{
	try {
		if (nFlags == dlTrySetSelFinishedFailure) {
			// (r.gonet 2015-07-06 18:02) - PLID 65327 - select unknown receiver
			CheckDlgButton(IDC_REC_TYPE_UNKNOWN, BST_CHECKED);
			CheckDlgButton(IDC_REC_TYPE_PAT, BST_UNCHECKED);
			EnsurePatInfo();
		}
	}NxCatchAll(__FUNCTION__);
}