// NewDiscountDlg.cpp : implementation file
//

// (j.gruber 2009-03-18 14:20) - PLID 33351 - created for
#include "stdafx.h"
#include "Practice.h"
#include "NewDiscountDlg.h"
#include "InternationalUtils.h"
#include "CouponSelectDlg.h"
#include "BillingRc.h"
#include "Barcode.h"

// CNewDiscountDlg dialog

IMPLEMENT_DYNAMIC(CNewDiscountDlg, CNxDialog)

// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
// (j.jones 2009-04-07 10:09) - PLID 33474 - added fields for default coupons
CNewDiscountDlg::CNewDiscountDlg(BOOL bUseCoupons, BOOL bIsBill, BOOL bCheckPermissions, CWnd* pParent /*=NULL*/,
								 long nDefaultCouponID /*= -1*/, CString strDefaultCouponName /*= ""*/)
	: CNxDialog(CNewDiscountDlg::IDD, pParent)
{
	m_bUseCoupons = bUseCoupons;

	m_nDiscountCategoryID = -3;
	m_nPercentDiscount = 0;
	m_cyDollarDiscount = COleCurrency(0,0);

	m_bIsBill = bIsBill;
	m_bCheckPermissions = bCheckPermissions;

	m_bApplyFromLineTotal = FALSE;

	m_bScanning = false;

	m_nCouponID = -1;
	
	// (j.jones 2009-04-07 10:09) - PLID 33474 - added fields for default coupons
	m_nDefaultCouponID = nDefaultCouponID;
	m_strDefaultCouponName = strDefaultCouponName;
}

CNewDiscountDlg::~CNewDiscountDlg()
{
	try {

		// (j.jones 2009-04-07 10:00) - PLID 33474 - supported barcoding
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in CNewDiscountDlg::~CNewDiscountDlg");
}

void CNewDiscountDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewDiscountDlg)	
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEW_DISCOUNT_CALCULATE_LINE_TOTAL, m_radioApplyUsingLineTotal);
	DDX_Control(pDX, IDC_NEW_DISCOUNT_CALCULATE_UNIT_COST, m_radioApplyUsingUnitCost);
	DDX_Control(pDX, IDC_NEW_DISCOUNT_CUSTOM_DESC, m_nxeCustomDiscountDesc);
	DDX_Control(pDX, IDC_NEW_DISCOUNT_PERCENT_OFF, m_nxePercentOff);
	DDX_Control(pDX, IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, m_nxeDiscount);
	DDX_Control(pDX, IDC_NEW_DISCOUNT_USE_CUSTOM, m_chkCustomDiscount);
	//}}AFX_DATA_MAP
	
}


BEGIN_MESSAGE_MAP(CNewDiscountDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNewDiscountDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNewDiscountDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_NEW_DISCOUNT_USE_CUSTOM, &CNewDiscountDlg::OnBnClickedNewDiscountUseCustom)
	ON_EN_KILLFOCUS(IDC_NEW_DISCOUNT_PERCENT_OFF, &CNewDiscountDlg::OnEnKillfocusNewDiscountPercentOff)
	ON_EN_KILLFOCUS(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, &CNewDiscountDlg::OnEnKillfocusNewDiscountDollarDiscount)
	ON_BN_CLICKED(IDC_NEW_DISCOUNT_CALCULATE_LINE_TOTAL, &CNewDiscountDlg::OnBnClickedNewDiscountCalculateLineTotal)
	ON_BN_CLICKED(IDC_NEW_DISCOUNT_CALCULATE_UNIT_COST, &CNewDiscountDlg::OnBnClickedNewDiscountCalculateUnitCost)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
END_MESSAGE_MAP()


// CNewDiscountDlg message handlers
BOOL CNewDiscountDlg::OnInitDialog() {
	
	CNxDialog::OnInitDialog();

	try {

		// (s.dhole 2013-06-07 15:15) - PLID 56368 Load properties into the NxPropManager cache
		g_propManager.CachePropertiesInBulk("FinancialDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'BillNewDiscountDefaultCalculation' "
			"OR Name = 'QuoteNewDiscountDefaultCalculation'	" 
			"OR Name = 'dontshow PercentDiscountOnLineTotal' "
			")",
			_Q(GetCurrentUserName()));

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);



		// (j.jones 2009-04-07 10:01) - PLID 33474 - supported barcoding
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
				MsgBox("Error registering for barcode scans.  You may not be able to scan in this screen.");
			}
		}

		// (j.gruber 2009-03-20 14:29) - PLID 33554 - check the permissions
		if (m_bCheckPermissions) {
			if (m_bIsBill) {
				if (!(GetCurrentUserPermissions(bioChargePercentOff) & (SPT___W_______))) {
					//maybe they have with pass
					if (!(GetCurrentUserPermissions(bioChargePercentOff) & (SPT___W________ANDPASS))) {
						//they don't have permission for percent off, grey out the box and the radio buttons
						GetDlgItem(IDC_NEW_DISCOUNT_PERCENT_OFF)->EnableWindow(FALSE);
						GetDlgItem(IDC_NEW_DISCOUNT_CALCULATE_UNIT_COST)->EnableWindow(FALSE);
						GetDlgItem(IDC_NEW_DISCOUNT_CALCULATE_LINE_TOTAL)->EnableWindow(FALSE);
					}		

				}
			}
			else {
				if (!(GetCurrentUserPermissions(bioQuotePercentOff) & (SPT___W_______))) {
					//maybe they have with pass
					if (!(GetCurrentUserPermissions(bioQuotePercentOff) & (SPT___W________ANDPASS))) {
						//they don't have permission for percent off, grey out the box and the radio buttons
						GetDlgItem(IDC_NEW_DISCOUNT_PERCENT_OFF)->EnableWindow(FALSE);
						GetDlgItem(IDC_NEW_DISCOUNT_CALCULATE_UNIT_COST)->EnableWindow(FALSE);
						GetDlgItem(IDC_NEW_DISCOUNT_CALCULATE_LINE_TOTAL)->EnableWindow(FALSE);
					}		

				}
			}
		

			if (m_bIsBill) {
				if (!(GetCurrentUserPermissions(bioChargeAmountOff) & (SPT___W_______))) {
					if (!(GetCurrentUserPermissions(bioChargeAmountOff) & (SPT___W________ANDPASS))) {
						//they don't have permission for discount, grey out the box 
						GetDlgItem(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT)->EnableWindow(FALSE);		
					}
				}
			}
			else {
					if (!(GetCurrentUserPermissions(bioQuoteAmountOff) & (SPT___W_______))) {
						if (!(GetCurrentUserPermissions(bioQuoteAmountOff) & (SPT___W________ANDPASS))) {
						//they don't have permission for discount, grey out the box 
						GetDlgItem(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT)->EnableWindow(FALSE);		
					}
				}
			}

		}

		//m_radioApplyUsingUnitCost.SetCheck(1);
		// (s.dhole 2013-06-07 15:15) - PLID 56368 Set option value
		BOOL bDiscountCalculationByLineTotal = FALSE;
		if (m_bIsBill){
			bDiscountCalculationByLineTotal = GetRemotePropertyInt("BillNewDiscountDefaultCalculation", 0, 0, "<None>", true);
		}
		else{
			bDiscountCalculationByLineTotal = GetRemotePropertyInt("QuoteNewDiscountDefaultCalculation", 0, 0, "<None>", true);
		}
		// (s.dhole 2013-06-07 15:15) - PLID 56368
		if (bDiscountCalculationByLineTotal){
			m_radioApplyUsingLineTotal.SetCheck(TRUE); 
			m_radioApplyUsingUnitCost.SetCheck(FALSE);
		}
		else{ 
			m_radioApplyUsingLineTotal.SetCheck(FALSE);	
			m_radioApplyUsingUnitCost.SetCheck(TRUE);			
		}

		GetDlgItem(IDC_NEW_DISCOUNT_CUSTOM_DESC)->EnableWindow(FALSE);

		m_pCatList = BindNxDataList2Ctrl(IDC_NEW_DISCOUNT_DISCOUNT_CATEGORY, true);

		// (z.manning 2016-01-15 11:10) - PLID 67909 - Make sure to update the flag for the way to apply discounts
		HandleCalculateDiscountTypeChanged();

	}NxCatchAll("Error in CNewDiscountDlg::OnInitDialog()");
	return TRUE;
}


BOOL CNewDiscountDlg::ValidateInformation() {

	try {
		//make sure the percent is not greater then 100
		long nPercent = GetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF);
		if (nPercent > 100 || nPercent < 0) {
			MsgBox("Please enter a valid percent off");
			return FALSE;
		}

		return TRUE;
	}NxCatchAll("Error in CNewDiscountDlg::ValidateInformation()");
	return FALSE;
	
}

void CNewDiscountDlg::OnBnClickedOk()
{
	try {
		//check to make sure everything is valid
		if (ValidateInformation()) {

			//(e.lally 2009-08-18) PLID 28825 - Added preference for requiring a discount category
			BOOL bRequireDiscCategory = GetRemotePropertyInt("RequireDiscountCategory", 0, 0, "<None>", true) == 0 ? FALSE: TRUE;

			//set our variables
			m_nPercentDiscount = GetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF);
			CString strTemp;
			GetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, strTemp);
			if (strTemp.IsEmpty()) {
				m_cyDollarDiscount = COleCurrency(0,0);
			}
			else {
				m_cyDollarDiscount = ParseCurrencyFromInterface(strTemp);
			}

			//check the custom discount
			if (IsDlgButtonChecked(IDC_NEW_DISCOUNT_USE_CUSTOM) ) {
				//set the custom discount
				m_nDiscountCategoryID = -1;
				GetDlgItemText(IDC_NEW_DISCOUNT_CUSTOM_DESC, m_strCustomDiscountDesc);
				//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, only allow a custom description if it is not blank
				CString strDescriptionTest = m_strCustomDiscountDesc;
				strDescriptionTest.TrimRight();
				if(bRequireDiscCategory && strDescriptionTest.IsEmpty()){
					MsgBox("A Discount Category or Discount Description is required before saving this discount.");
					return;
				}

				m_strCategoryDescription = "<Custom> - " + m_strCustomDiscountDesc;
			}
			else {
				//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, check that one is selected
				if(bRequireDiscCategory && m_nDiscountCategoryID == -3){
					MsgBox("A Discount Category must be selected before saving this discount.");
					return;
				}
			}

			// (j.jones 2016-05-26 10:59) - PLID-67910 - if a percent was entered and
			// 'calculate on line total' is checked, warn the user of what will happen
			long nPercent = GetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF);
			if (nPercent > 0 && m_radioApplyUsingLineTotal.GetCheck()) {
				if (IDNO == DontShowMeAgain(this, FormatString("When calculating a percent discount on the current line total, "
					"a fixed discount amount will be added to the charge. Additional changes to the charge will not affect this value.\n\n"
					"Example: a 10%% discount on a %s charge will make a fixed %s discount.\n\n"
					"Do you wish to continue and create a fixed discount amount using the current line total?",
					FormatCurrencyForInterface(COleCurrency(150, 0)),
					FormatCurrencyForInterface(COleCurrency(15, 0))),
					"PercentDiscountOnLineTotal", "Practice", false, true, false)) {
					return;
				}
			}
			
			OnOK();
		}
		//otherwise it gives messages they'll need to fix
	}NxCatchAll("Error in CNewDiscountDlg::OnBnClickedOk");
}

void CNewDiscountDlg::OnBnClickedCancel()
{
	
	OnCancel();
}
BEGIN_EVENTSINK_MAP(CNewDiscountDlg, CNxDialog)
	ON_EVENT(CNewDiscountDlg, IDC_NEW_DISCOUNT_DISCOUNT_CATEGORY, 16, CNewDiscountDlg::SelChosenNewDiscountDiscountCategory, VTS_DISPATCH)
	ON_EVENT(CNewDiscountDlg, IDC_NEW_DISCOUNT_DISCOUNT_CATEGORY, 18, CNewDiscountDlg::RequeryFinishedNewDiscountDiscountCategory, VTS_I2)
END_EVENTSINK_MAP()


void CNewDiscountDlg::ChangePercentDiscountFieldsFromCoupon(long nCouponID) {

	try {
		//we aren't warning here if we overwrite a filled in field because its a new discount
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT DiscountType, PercentOff, DiscountAmount FROM CouponsT WHERE ID = %li", nCouponID);
		if (! rs->eof) {
			long nDiscountType = AdoFldLong(rs, "DiscountType");
			if (nDiscountType == 1) {
				//its a percent
				long nPercent = AdoFldLong(rs, "PercentOff", 0);

				//check to see if they already have a discount					
				SetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF, nPercent);
				
			}
			else {
				//its a dollar discount
				//check to see if they already have a dollar discount
				COleCurrency cyDollar = AdoFldCurrency(rs, "DiscountAmount", COleCurrency(0,0));
							
				//just add the dollar discount on since there is no percent
				SetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, FormatCurrencyForInterface(cyDollar));					
			}
		}

	}NxCatchAll("Error in CNewDiscountDlg::ChangePercentDiscountFieldsFromCoupon");
}

void CNewDiscountDlg::SelChosenNewDiscountDiscountCategory(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			//check to see what the value is
			long nID = pRow->GetValue(0);

			if (nID == -2) {
				//its a coupon, do something
				CCouponSelectDlg dlg(this);
				if (dlg.DoModal() == IDOK) {

					//we should be able to get a coupon ID
					m_nCouponID = dlg.m_nCouponID;
					CString strCouponName = dlg.m_strCouponName;
					m_nDiscountCategoryID = -2;

					//add it to the datalist
					pRow->PutValue(1, _variant_t("<Coupon> - " + strCouponName));				

					//set the description to send back to the dialog
					m_strCategoryDescription = "<Coupon> - " + strCouponName;

					//add the discounts associated with the coupon
					ChangePercentDiscountFieldsFromCoupon(m_nCouponID);
				}
				else {
					if (m_nCouponID == -1) {
						//none selected
						pRow->PutValue(1, _variant_t("<Coupon>"));				
					}				
				}
			}
			else {
				//set the categoryID
				m_nDiscountCategoryID = VarLong(pRow->GetValue(0));
				m_strCategoryDescription= VarString(pRow->GetValue(1));
			}
		}
	}NxCatchAll("Error in CNewDiscountDlg::SelChosenNewDiscountDiscountCategory");
}


void CNewDiscountDlg::OnBnClickedNewDiscountUseCustom()
{
	try {
		if (IsDlgButtonChecked(IDC_NEW_DISCOUNT_USE_CUSTOM)) {
			//gray out the list
			GetDlgItem(IDC_NEW_DISCOUNT_DISCOUNT_CATEGORY)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_DISCOUNT_CUSTOM_DESC)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_NEW_DISCOUNT_DISCOUNT_CATEGORY)->EnableWindow(TRUE);
			GetDlgItem(IDC_NEW_DISCOUNT_CUSTOM_DESC)->EnableWindow(FALSE);

			m_strCustomDiscountDesc = "";

			//see if we have a category
			if (m_nDiscountCategoryID != -3) {
				NXDATALIST2Lib::IRowSettingsPtr pRow;
				pRow = m_pCatList->SetSelByColumn(0, m_nDiscountCategoryID);
				if (pRow == NULL) {
					m_pCatList->SetSelByColumn(0, (long)-3);
				}
			}
			else {
				m_pCatList->SetSelByColumn(0, (long)-3);
			}
		}	
	}NxCatchAll("Error in CNewDiscountDlg::OnBnClickedNewDiscountUseCustom()");
}

void CNewDiscountDlg::RequeryFinishedNewDiscountDiscountCategory(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->GetNewRow();
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pCatList->GetFirstRow();


		if (m_bUseCoupons) {
			//add the coupon line
			pRow->PutValue(0, (long)-2);
			pRow->PutValue(1, _variant_t("<Coupon>"));
			m_pCatList->AddRowBefore(pRow, pFirstRow);

			pFirstRow = m_pCatList->GetFirstRow();
		}

		//add a no line
		pRow = m_pCatList->GetNewRow();
		pRow->PutValue(0, (long)-3);
		pRow->PutValue(1, _variant_t("<No Category>"));
		m_pCatList->AddRowBefore(pRow, pFirstRow);


		//now set no Category to be selected
		m_pCatList->CurSel = pRow;

		// (j.jones 2009-04-07 10:07) - PLID 33474 - if coupons are allowed and we have
		// a default coupon ID, load it up now
		if(m_bUseCoupons && m_nDefaultCouponID != -1) {

			NXDATALIST2Lib::IRowSettingsPtr pCouponRow = m_pCatList->SetSelByColumn(0, (long)-2);
			if(pCouponRow == NULL) {
				ThrowNxException("No coupon row available!");
			}
			
			m_nCouponID = m_nDefaultCouponID;
			m_nDiscountCategoryID = -2;

			//add it to the datalist
			pCouponRow->PutValue(1, _variant_t("<Coupon> - " + m_strDefaultCouponName));				

			//set the description to send back to the dialog
			m_strCategoryDescription = "<Coupon> - " + m_strDefaultCouponName;

			//add the discounts associated with the coupon
			ChangePercentDiscountFieldsFromCoupon(m_nCouponID);
		}
	
	}NxCatchAll("Error in CNewDiscountDlg::RequeryFinishedNewDiscountDiscountCategory");
}


// (j.gruber 2009-03-20 14:19) - PLID 33554 - check permissions
void CNewDiscountDlg::OnEnKillfocusNewDiscountPercentOff()
{
	try {
		//see if its changed
		if (GetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF) != m_nPercentDiscount) {
			if (m_bCheckPermissions) {
				if (m_bIsBill) {
					if (!CheckCurrentUserPermissions(bioChargePercentOff, sptWrite)) {
						//set it back to what it was before
						SetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF, m_nPercentDiscount);
					}
					else {
						m_nPercentDiscount = GetDlgItemInt(IDC_NEW_DISCOUNT_PERCENT_OFF);
					}
				}
				else {
					if (!(GetCurrentUserPermissions(bioQuotePercentOff) & (SPT___W_______))) {
						if (!(GetCurrentUserPermissions(bioQuotePercentOff) & (SPT___W________ANDPASS))) {
							//they don't have permission for discount, grey out the box 
							GetDlgItem(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT)->EnableWindow(FALSE);		
						}
					}
				}
			}
		}
	}NxCatchAll("Error in CNewDiscountDlg::OnEnKillfocusNewDiscountPercentOff()");
}

// (j.gruber 2009-03-20 14:19) - PLID 33554 - check permissions
void CNewDiscountDlg::OnEnKillfocusNewDiscountDollarDiscount()
{

	try {
		CString strTemp;
		GetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, strTemp);
		if (strTemp.IsEmpty()) {
			strTemp = "0";
		}

		COleCurrency cyTemp = ParseCurrencyFromInterface(strTemp);
		if (cyTemp.GetStatus() == COleCurrency::invalid) {
			SetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, FormatCurrencyForInterface(m_cyDollarDiscount));
			return;
		}

		if (ParseCurrencyFromInterface(strTemp) != m_cyDollarDiscount) {	
			if (m_bCheckPermissions) {
				if (m_bIsBill) {
					if (!CheckCurrentUserPermissions(bioChargeAmountOff, sptWrite)) {
						//set it back to what it was before
						SetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, FormatCurrencyForInterface(m_cyDollarDiscount));
					}
					else {
						//set the member variable			
						m_cyDollarDiscount = ParseCurrencyFromInterface(strTemp);

					}
				}
				else {
					if (!CheckCurrentUserPermissions(bioQuoteAmountOff, sptWrite)) {
						//set it back to what it was before
						SetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, FormatCurrencyForInterface(m_cyDollarDiscount));
					}
					else {
						//set the member variable			
						m_cyDollarDiscount = ParseCurrencyFromInterface(strTemp);

					}
				}
			}
			else {

				//format it to look nice
				m_cyDollarDiscount = ParseCurrencyFromInterface(strTemp);
			}
			SetDlgItemText(IDC_NEW_DISCOUNT_DOLLAR_DISCOUNT, FormatCurrencyForInterface(m_cyDollarDiscount));		
			
		}
	}NxCatchAll("Error in CNewDiscountDlg::OnEnKillfocusNewDiscountDollarDiscount()");
		
}

void CNewDiscountDlg::OnBnClickedNewDiscountCalculateLineTotal()
{
	try {
		HandleCalculateDiscountTypeChanged();
	}NxCatchAll("Error in CNewDiscountDlg::OnBnClickedNewDiscountCalculateLineTotal");
}

void CNewDiscountDlg::OnBnClickedNewDiscountCalculateUnitCost()
{
	try {
		HandleCalculateDiscountTypeChanged();
	}NxCatchAll("Error in CNewDiscountDlg::OnBnClickedNewDiscountCalculateUnitCost()");
}

// (z.manning 2016-01-15 11:08) - PLID 67909
void CNewDiscountDlg::HandleCalculateDiscountTypeChanged()
{
	if (IsDlgButtonChecked(IDC_NEW_DISCOUNT_CALCULATE_LINE_TOTAL) == BST_CHECKED) {
		//its checked, set our variable
		m_bApplyFromLineTotal = TRUE;
	}
	else {
		m_bApplyFromLineTotal = FALSE;
	}
}

// (j.jones 2009-04-07 09:43) - PLID 33474 - supported barcode scanning
LRESULT CNewDiscountDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	//if we can't use coupons, ignore the scan
	if(!m_bUseCoupons) {
		return 0;
	}

	if (m_bScanning) {
		return 0;
	}

	m_bScanning = true;
	try {
		//we can only scan coupons with the NexSpa license
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {

			_bstr_t bstr = (BSTR)lParam; // We can typecast this to an ANSI string
			CString str = (LPCTSTR)bstr;

			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT ID, Description, CASE WHEN CONVERT(datetime, Convert(nvarchar, StartDate, 1)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 1 ELSE 0 END AS Started, CASE WHEN EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 0 ELSE 1 END AS Expired FROM CouponsT WHERE BarCode = {STRING}", str);
			if (!prs->eof) {
				CString strCouponName = AdoFldString(prs, "Description", "");
				long nID = AdoFldLong(prs, "ID");
				BOOL bStarted = AdoFldLong(prs, "Started");
				BOOL bExpired = AdoFldLong(prs, "Expired");

				if (strCouponName.IsEmpty())
					strCouponName = "<No Coupon Description>";

				if (bExpired) {
					// since the coupon select allows this, we can continue with a warning.
					MessageBox(FormatString("(%s) - You have scanned an expired coupon!", strCouponName), NULL, MB_OK | MB_ICONEXCLAMATION);
					//return 0;
				} else if (!bStarted) {
					MessageBox(FormatString("(%s) - You have scanned a coupon that has not yet become active! Coupon will not be used.", strCouponName), NULL, MB_OK | MB_ICONHAND);
					return 0;
				}

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->SetSelByColumn(0, (long)-2);
				if(pRow == NULL) {
					ThrowNxException("No coupon row available!");
				}

				m_nCouponID = nID;
				m_nDiscountCategoryID = -2;

				//add it to the datalist
				pRow->PutValue(1, _variant_t("<Coupon> - " + strCouponName));				

				//set the description to send back to the dialog
				m_strCategoryDescription = "<Coupon> - " + strCouponName;

				//add the discounts associated with the coupon
				ChangePercentDiscountFieldsFromCoupon(m_nCouponID);
			}
		}
	} NxCatchAll("Error in CNewDiscountDlg::OnBarcodeScan");

	m_bScanning = false;

	return 0;
}