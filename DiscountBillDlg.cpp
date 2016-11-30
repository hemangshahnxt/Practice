// DiscountBillDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingRc.h"
#include "DiscountBillDlg.h"
#include "internationalutils.h"
#include "CouponSelectDlg.h"
#include "Barcode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.gruber 2007-05-02 16:57) - PLID 14202 - Class created for
/////////////////////////////////////////////////////////////////////////////
// CDiscountBillDlg dialog


CDiscountBillDlg::CDiscountBillDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDiscountBillDlg::IDD, pParent)
{

	m_nPercentOff = 0;
	m_cyDiscount = COleCurrency(0,0);
	m_nDiscountCatID = -3;
	m_bScanning = false;
	m_bCouponSet = FALSE;
	m_bHasDiscountsAlready = FALSE;
	m_bIsBill = TRUE;
	m_edboLastDiscountBillOverwriteOption = edboUnspecified;
	// (j.gruber 2009-03-24 10:35) - PLID 33355 - radio button to apply from lineTotal or not
	m_bApplyFromLineTotal = FALSE;
	//{{AFX_DATA_INIT(CDiscountBillDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CDiscountBillDlg::~CDiscountBillDlg()
{
	try {

		// (j.jones 2008-12-30 15:15) - PLID 32584 - unregister from barcode scanning
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in ~CDiscountBillDlg");
}


void CDiscountBillDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiscountBillDlg)
	DDX_Control(pDX, IDC_RADIO_ADD_DISCOUNTS, m_radioAddDiscounts);
	DDX_Control(pDX, IDC_RADIO_REPLACE_DISCOUNTS, m_radioReplaceDiscounts);
	DDX_Control(pDX, IDC_RADIO_SKIP_DISCOUNTS, m_radioSkipDiscounts);
	DDX_Control(pDX, IDC_EXISTING_DISCOUNT_LABEL, m_nxstaticExistingDiscountsLabel);
	DDX_Control(pDX, IDC_EXISTING_DISCOUNT_CATEGORY_LABEL, m_nxstaticExistingDiscountCategoryLabel);
	DDX_Control(pDX, IDC_BILL_DISCOUNT_USE_CUSTOM, m_btnCustom);
	DDX_Control(pDX, IDC_BILL_CUSTOM_DISCOUNT_CAT, m_nxeditBillCustomDiscountCat);
	DDX_Control(pDX, IDC_DISCOUNT_BILL_PERCENT_OFF, m_nxeditDiscountBillPercentOff);
	DDX_Control(pDX, IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, m_nxeditDiscountBillDollarDiscount);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DISCOUNT_CATEGORY_GROUPBOX, m_btnDiscountCategoryGroupbox);
	DDX_Control(pDX, IDC_DISCOUNT_BILL_USE_LINE_TOTAL, m_radioApplyUsingLineTotal);
	DDX_Control(pDX, IDC_DISCOUNT_BILL_APPLY_USE_UNIT_COST, m_radioApplyUsingUnitCost);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiscountBillDlg, CNxDialog)
	//{{AFX_MSG_MAP(CDiscountBillDlg)
	ON_EN_KILLFOCUS(IDC_DISCOUNT_BILL_PERCENT_OFF, OnKillfocusDiscountBillPercentOff)
	ON_EN_KILLFOCUS(IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, OnKillfocusDiscountBillDollarDiscount)
	ON_BN_CLICKED(IDC_BILL_DISCOUNT_USE_CUSTOM, OnBillDiscountUseCustom)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DISCOUNT_BILL_USE_LINE_TOTAL, &CDiscountBillDlg::OnBnClickedDiscountBillUseLineTotal)
	ON_BN_CLICKED(IDC_DISCOUNT_BILL_APPLY_USE_UNIT_COST, &CDiscountBillDlg::OnBnClickedDiscountBillApplyUseUnitCost)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscountBillDlg message handlers

BEGIN_EVENTSINK_MAP(CDiscountBillDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDiscountBillDlg)
	ON_EVENT(CDiscountBillDlg, IDC_BILL_DISCOUNT_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenBillDiscountCategoryList, VTS_DISPATCH)
	ON_EVENT(CDiscountBillDlg, IDC_BILL_DISCOUNT_CATEGORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedBillDiscountCategoryList, VTS_I2)
	ON_EVENT(CDiscountBillDlg, IDC_BILL_DISCOUNT_CATEGORY_LIST, 1 /* SelChanging */, OnSelChangingBillDiscountCategoryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CDiscountBillDlg, IDC_BILL_DISCOUNT_CATEGORY_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedBillDiscountCategoryList, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CDiscountBillDlg::OnOK() 
{
	
	//make sure we same the value in the currently selected box in case they hit enter instead of click

	// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
	CheckFocus();


	//check to make sure something is filled in
	if (m_nPercentOff <= 0 && m_cyDiscount == COleCurrency(0,0)) {
		MsgBox("Please enter either a percent or dollar discount or click cancel");
		return;
	}

	// (j.jones 2008-12-23 11:10) - PLID 32492 - if they have discounts already,
	// get their choice for how to handle them, and warn if no selection was made
	if(m_bHasDiscountsAlready) {		
		EDiscountBillOverwriteOption edboLastDiscountBillOverwriteOption = edboUnspecified;
		if(m_radioAddDiscounts.GetCheck()) {
			edboLastDiscountBillOverwriteOption = edboAdd;
		}
		else if(m_radioReplaceDiscounts.GetCheck()) {
			edboLastDiscountBillOverwriteOption = edboReplace;
		}
		else if(m_radioSkipDiscounts.GetCheck()) {
			edboLastDiscountBillOverwriteOption = edboSkip;
		}
		
		//don't let them continue without a selection
		if(edboLastDiscountBillOverwriteOption == edboUnspecified) {
			MsgBox("Because you have discounts on existing charges, you must make a selection for how to update these charges.");
			return;
		}

		//now save their choice
		SetRemotePropertyInt("LastDiscountBillOverwriteOption", (long)edboLastDiscountBillOverwriteOption, 0, GetCurrentUserName());
		m_edboLastDiscountBillOverwriteOption = edboLastDiscountBillOverwriteOption;
	}

	//(e.lally 2009-08-18) PLID 28825 - Added preference for requiring a discount category
	BOOL bRequireDiscCategory = GetRemotePropertyInt("RequireDiscountCategory", 0, 0, "<None>", true) == 0 ? FALSE: TRUE;

	//first see if we are using a custom discount category
	if (IsDlgButtonChecked(IDC_BILL_DISCOUNT_USE_CUSTOM)) {
		m_nDiscountCatID = -1;
		GetDlgItemText(IDC_BILL_CUSTOM_DISCOUNT_CAT, m_strCustomDescription);
		//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, only allow a custom description if it is not blank
		CString strDescriptionTest = m_strCustomDescription;
		strDescriptionTest.TrimRight();
		if(bRequireDiscCategory && strDescriptionTest.IsEmpty()){
			MsgBox("A Discount Category or Discount Description is required before saving this discount.");
			return;
		}
	}
	else {

		//see what the current selection is
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->CurSel;
		if (pRow) {

			long nID = pRow->GetValue(0);

			if (nID == -2) {

				//we already have the couponId, so we should be good
				m_nDiscountCatID = -2;
				ASSERT(m_nCouponID != -1);
			}
			else {
				//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, check that one is selected
				if(bRequireDiscCategory && nID == -3){
					MsgBox("A Discount Category must be selected before saving this discount.");
					return;
				}

				m_nDiscountCatID = nID;
			}
		}
		else {
			//it could be an inactive item
			if (m_pCatList->IsComboBoxTextInUse) {
				//it's inactive, so we aren't going to change anything
			}
			else {
				//they didn't choose anything
				//(e.lally 2009-08-18) PLID 28825 - When requiring a discount category, check that one is selected
				if(bRequireDiscCategory){
					MsgBox("A Discount Category must be selected before saving this discount.");
					return;
				}
				m_nDiscountCatID = -3;
			}
		}
	}

	// (j.jones 2016-05-26 10:59) - PLID-67910 - if a percent was entered and
	// 'calculate on line total' is checked, warn the user of what will happen
	if (m_nPercentOff > 0 && m_radioApplyUsingLineTotal.GetCheck()) {
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
	
	CDialog::OnOK();
}

void CDiscountBillDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDiscountBillDlg::OnKillfocusDiscountBillPercentOff() 
{
	try {
		//make sure that it is a percent
		CString strPercent;

		GetDlgItemText(IDC_DISCOUNT_BILL_PERCENT_OFF, strPercent);

		strPercent.TrimLeft();
		strPercent.TrimRight();

		//check to see if there is a decimal and take off the rest of it
		long nResult = strPercent.Find(".");
		if (nResult != -1) {
			//take off everything from the decimal on
			strPercent = strPercent.Left(nResult);
		}

		
		if (!strPercent.IsEmpty()) {
			if (strPercent.SpanIncluding("0123456789%").GetLength() != strPercent.GetLength()) {

				MsgBox("Please enter a valid percent off.");
				SetDlgItemText(IDC_DISCOUNT_BILL_PERCENT_OFF, AsString(m_nPercentOff));
				return;
			}
			if (atoi(strPercent) > 100 || atoi(strPercent) < 0) {
				MsgBox("Please enter a positive percent amount no greater then 100");
				SetDlgItemText(IDC_DISCOUNT_BILL_PERCENT_OFF, AsString(m_nPercentOff));
				return;
			}
		}
		
		m_nPercentOff = atoi(strPercent);
		SetDlgItemInt(IDC_DISCOUNT_BILL_PERCENT_OFF, atoi(strPercent));
	}NxCatchAll("Error in OnKillFocusDiscountBillPercentOff");
	
	
}

void CDiscountBillDlg::OnKillfocusDiscountBillDollarDiscount() 
{
	try {
		//make sure it is a valid amount
		CString strDiscount;

		GetDlgItemText(IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, strDiscount);
		if (!strDiscount.IsEmpty()) {
			COleCurrency cyDiscount = ParseCurrencyFromInterface(strDiscount);
			if (cyDiscount.GetStatus() == COleCurrency::invalid) {
				MsgBox("Please enter a valid amount.");
				SetDlgItemText(IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, FormatCurrencyForInterface(m_cyDiscount));
			}
			else {
				SetDlgItemText(IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, FormatCurrencyForInterface(cyDiscount));
				m_cyDiscount = cyDiscount;
			}
		}
	}NxCatchAll("Error in OnKillFocusDiscountBillDollarDiscount");
	
}

void CDiscountBillDlg::OnBillDiscountUseCustom() 
{
	try {
		if (IsDlgButtonChecked(IDC_BILL_DISCOUNT_USE_CUSTOM)) {
			//gray out the list
			GetDlgItem(IDC_BILL_DISCOUNT_CATEGORY_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BILL_CUSTOM_DISCOUNT_CAT)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_BILL_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_BILL_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

			//see if we have a category
			if (m_nDiscountCatID != -3) {
				NXDATALIST2Lib::IRowSettingsPtr pRow;
				pRow = m_pCatList->SetSelByColumn(0, m_nDiscountCatID);
				if (pRow == NULL) {
					m_pCatList->SetSelByColumn(0, (long)-3);
				}
			}
			else {
				m_pCatList->SetSelByColumn(0, (long)-3);
			}
		}	
	}NxCatchAll("Error in OnBillDiscountUseCustom");
	
}

void CDiscountBillDlg::OnSelChosenBillDiscountCategoryList(LPDISPATCH lpRow) 
{
	try {
		//see if it is a coupon
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {

			long nID = pRow->GetValue(0);

			if (nID == -2) {
				ApplyCoupon();
			}
		}
		
	}NxCatchAll("Error in OnSelChosenBillDiscountCategoryList");
	
}

// (a.walling 2007-05-10 16:00) - PLID 25171 - Apply a given coupon, prompting for one if -1
void CDiscountBillDlg::ApplyCoupon(long nID /*=-1*/)
{
	// (j.jones 2008-12-30 15:39) - PLID 32588 - this code needs to find the coupon row, not the current selection
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->SetSelByColumn(0, _variant_t((long)-2));
	if(pRow == NULL) {
		ThrowNxException("CDiscountBillDlg::ApplyCoupon could not find a coupon row!");
	}

	// (a.wetta 2007-05-17 10:00) - PLID 25960 - Need a NexSpa license to apply coupons
	if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent) || pRow == NULL)
		return;

	//coupons have discounts associated with them already
	if (m_nPercentOff > 0 || m_cyDiscount > COleCurrency(0,0)) {
		if (IDNO == MsgBox(MB_YESNO, "By choosing a coupon, this will overwrite any discounts you've already entered with the discount associated with the coupon.\n"
			"Are you sure you wish to continue?"))  {
			return;
		}
	}

	if (nID == -1) { // select a coupon
		CCouponSelectDlg dlg(this);
		if (dlg.DoModal() == IDCANCEL) {
/*			if (m_nCouponID == -1) {
				//none selected
				pRow->PutValue(1, _variant_t("<Coupon>"));
			}*/

			return;
		} else {
			nID = dlg.m_nCouponID;
		}
	}
		
	//Get the discount information from the coupon
	CString strCouponName;
	ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Description, DiscountType, PercentOff, DiscountAmount FROM CouponsT WHERE ID = %li", nID);
	if (! rs->eof) {
		strCouponName = AdoFldString(rs, "Description", "");
		long nDiscountType = AdoFldLong(rs, "DiscountType");
		if (nDiscountType == 1) {
			//its a percent
			long nPercent = AdoFldLong(rs, "PercentOff", 0);

			SetDlgItemInt(IDC_DISCOUNT_BILL_PERCENT_OFF, nPercent);
			m_nPercentOff = nPercent;

			//clear out the discount
			COleCurrency cyDiscount = COleCurrency(0,0);
			SetDlgItemText(IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, FormatCurrencyForInterface(cyDiscount, TRUE, TRUE));
			m_cyDiscount = cyDiscount;
			
		}
		else {
			//its a dollar discount
			// (j.gruber 2007-05-15 08:52) - PLID 14202 - fixed bad variable type error
			COleCurrency cyDiscount = AdoFldCurrency(rs, "DiscountAmount", COleCurrency(0,0));
			SetDlgItemText(IDC_DISCOUNT_BILL_DOLLAR_DISCOUNT, FormatCurrencyForInterface(cyDiscount, TRUE, TRUE));
			m_cyDiscount = cyDiscount;	
			
			//clear out the percent
			SetDlgItemText(IDC_DISCOUNT_BILL_PERCENT_OFF, 0);
			m_nPercentOff = 0;
		}	

		m_nCouponID = nID;
	} else {
		ThrowNxException(FormatString("The coupon id %li could not be found!", nID));
	}
	
	//add it to the datalist
	pRow->PutValue(1, _variant_t("<Coupon> - " + strCouponName));
}

void CDiscountBillDlg::OnRequeryFinishedBillDiscountCategoryList(short nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCatList->GetNewRow();
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pCatList->GetFirstRow();

		// (a.wetta 2007-05-17 10:02) - PLID 25960 - Need NexSpa license for coupons
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
			//add the coupon line
			pRow->PutValue(0, (long)-2);
			pRow->PutValue(1, _variant_t("<Coupon>"));
			m_pCatList->AddRowBefore(pRow, pFirstRow);
		}

		pFirstRow = m_pCatList->GetFirstRow();

		//add a no line
		pRow = m_pCatList->GetNewRow();
		pRow->PutValue(0, (long)-3);
		pRow->PutValue(1, _variant_t("<No Category>"));
		m_pCatList->AddRowBefore(pRow, pFirstRow);


		//now set the default value
		if (m_nDiscountCatID == -1) {

			GetDlgItem(IDC_BILL_DISCOUNT_CATEGORY_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_BILL_CUSTOM_DISCOUNT_CAT)->EnableWindow(TRUE);

			CheckDlgButton(IDC_BILL_DISCOUNT_USE_CUSTOM, 1);		
			
			//they are using a custom value
			SetDlgItemText(IDC_BILL_CUSTOM_DISCOUNT_CAT, m_strCustomDescription);
		}
		else if (m_nDiscountCatID == -2) {
			//its a coupon
			GetDlgItem(IDC_BILL_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_BILL_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

			CheckDlgButton(IDC_BILL_DISCOUNT_USE_CUSTOM, 0);		

			pRow = m_pCatList->SetSelByColumn(0, (long)-2);

			if (!m_bCouponSet) {
				//we just put it in, so we know its there
				if (pRow) {
					ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Description FROM CouponsT WHERE ID = %li", m_nCouponID);
					if (!rs->eof) {
						pRow->PutValue(1, _variant_t("<Coupon> - " + AdoFldString(rs, "Description", "")));
					}
				}
			} else {
				// our coupon ID was set outside the dialog
				ApplyCoupon(m_nCouponID);
			}
		}
		else {

			GetDlgItem(IDC_BILL_DISCOUNT_CATEGORY_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_BILL_CUSTOM_DISCOUNT_CAT)->EnableWindow(FALSE);

			CheckDlgButton(IDC_BILL_DISCOUNT_USE_CUSTOM, 0);		

			//account for inactives
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			long nSel = m_pCatList->TrySetSelByColumn_Deprecated(0, m_nDiscountCatID);
			if (nSel == -1) {
				//we have an inactive
				CString strDiscName = VarString(GetTableField("DiscountCategoriesT", "Description", "ID", m_nDiscountCatID),"");
				if (! strDiscName.IsEmpty()) {
					m_pCatList->PutComboBoxText(_bstr_t(strDiscName));
				}
			}
			else if (nSel == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
				//we don't really need to do anything
			}
		}	
	}NxCatchAll("Error In OnRequeryFinishedBillDiscountCategoryList");
	
}

BOOL CDiscountBillDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (s.dhole 2013-06-11 12:52) - PLID  Load properties into the NxPropManager cache
		g_propManager.CachePropertiesInBulk("FinancialDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'BillNewDiscountDefaultCalculation' "
			"OR Name = 'QuoteNewDiscountDefaultCalculation'	" 
			"OR Name = 'dontshow PercentDiscountOnLineTotal' "
			")",
			_Q(GetCurrentUserName()));

		// (c.haag 2008-05-01 15:49) - PLID 29871 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if(!m_bIsBill) {
			//rename the dialog if a quote
			SetWindowText("Discount Entire Quote");
		}

		m_pCatList = BindNxDataList2Ctrl(this, IDC_BILL_DISCOUNT_CATEGORY_LIST, GetRemoteData(), true);

		// (j.jones 2008-12-30 15:15) - PLID 32584 - this dialog needs to register for barcode scans
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
				MsgBox("Error registering for barcode scans.  You may not be able to scan in this screen.");
			}
		}

		// (j.jones 2008-12-23 10:55) - PLID 32492 - if we don't already have discounts, disable
		// the existing discount controls, and change the label
		if(!m_bHasDiscountsAlready) {
			m_radioAddDiscounts.EnableWindow(FALSE);
			m_radioReplaceDiscounts.EnableWindow(FALSE);
			m_radioSkipDiscounts.EnableWindow(FALSE);
			CString str;
			str.Format("The current %s does not have any discounts, the new discounts will be added to each charge.", m_bIsBill ? "bill" : "quote");
			m_nxstaticExistingDiscountsLabel.SetWindowText(str);
			m_nxstaticExistingDiscountCategoryLabel.SetWindowText("");
		}
		else {
			//load their last selection, if one exists
			EDiscountBillOverwriteOption edboLastDiscountBillOverwriteOption = (EDiscountBillOverwriteOption)GetRemotePropertyInt("LastDiscountBillOverwriteOption", (long)edboUnspecified, 0, GetCurrentUserName(), true);
			if(edboLastDiscountBillOverwriteOption == edboAdd) { //add
				m_radioAddDiscounts.SetCheck(TRUE);
			}
			else if(edboLastDiscountBillOverwriteOption == edboReplace) { //replace
				m_radioReplaceDiscounts.SetCheck(TRUE);
			}
			else if(edboLastDiscountBillOverwriteOption == edboSkip) { //skip
				m_radioSkipDiscounts.SetCheck(TRUE);
			}
			//leave all unchecked if they've never used this before
		}


		// (j.gruber 2009-03-24 11:50) - PLID 33355 - added a option to apply to line total
		//m_radioApplyUsingLineTotal.SetCheck(0);
		//m_radioApplyUsingUnitCost.SetCheck(1);
		// (s.dhole 2013-06-11 12:52) - PLID 56368 Set option value
		BOOL bDiscountCalculationByLineTotal = FALSE;
		if (m_bIsBill){
			bDiscountCalculationByLineTotal = GetRemotePropertyInt("BillNewDiscountDefaultCalculation", 0, 0, "<None>", true);
		}
		else{
			bDiscountCalculationByLineTotal = GetRemotePropertyInt("QuoteNewDiscountDefaultCalculation", 0, 0, "<None>", true);
		}
		// (s.dhole 2013-06-11 12:52) - PLID  56368
		if (bDiscountCalculationByLineTotal){
			m_radioApplyUsingLineTotal.SetCheck(TRUE); 
			m_radioApplyUsingUnitCost.SetCheck(FALSE);
		}
		else{ 
			m_radioApplyUsingLineTotal.SetCheck(FALSE);	
			m_radioApplyUsingUnitCost.SetCheck(TRUE);			
		}

		// (z.manning 2016-01-15 11:10) - PLID 67909 - Make sure to update the flag for the way to apply discounts
		HandleCalculateDiscountTypeChanged();

		if (m_bCouponSet)
			m_pCatList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}
	NxCatchAll("Error in CDiscountBillDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDiscountBillDlg::OnSelChangingBillDiscountCategoryList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}

void CDiscountBillDlg::OnTrySetSelFinishedBillDiscountCategoryList(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			//It must be inactive.
			CString strDiscName = VarString(GetTableField("DiscountCategoriesT", "Description", "ID", m_nDiscountCatID),"");
			if (! strDiscName.IsEmpty()) {
				m_pCatList->PutComboBoxText(_bstr_t(strDiscName));
			}
		}
	}NxCatchAll("Error in OnTrySetSelFinishedBillDiscountCategoryList");

	
}

LRESULT CDiscountBillDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	if (m_bScanning) return 0;

	m_bScanning = true;
	try {
		// (a.wetta 2007-05-17 09:58) - PLID 25960 - Can only scan coupons with the NexSpa license
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
			// (a.walling 2007-05-09 12:59) - PLID 25171 - We have a barcode; select the coupon if we can
			// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
			_bstr_t bstr = (BSTR)lParam; // We can typecast this to an ANSI string
			CString str = (LPCTSTR)bstr;

			ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID, Description, CASE WHEN CONVERT(datetime, Convert(nvarchar, StartDate, 1)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 1 ELSE 0 END AS Started, CASE WHEN EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 0 ELSE 1 END AS Expired FROM CouponsT WHERE BarCode = '%s'", _Q(str));
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

				ApplyCoupon(nID);
			}
		}
	} NxCatchAll("Error in CDiscountBillDlg::OnBarcodeScan");

	m_bScanning = false;

	return 0;
}

void CDiscountBillDlg::OnBnClickedDiscountBillUseLineTotal()
{
	try
	{
		HandleCalculateDiscountTypeChanged();
	}
	NxCatchAll(__FUNCTION__);
}

void CDiscountBillDlg::OnBnClickedDiscountBillApplyUseUnitCost()
{
	try
	{
		HandleCalculateDiscountTypeChanged();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2016-01-15 11:08) - PLID 67909
void CDiscountBillDlg::HandleCalculateDiscountTypeChanged()
{
	if (IsDlgButtonChecked(IDC_DISCOUNT_BILL_USE_LINE_TOTAL) == BST_CHECKED) {
		//its checked, set our variable
		m_bApplyFromLineTotal = TRUE;		
	}
	else {
		m_bApplyFromLineTotal = FALSE;
	}
}
