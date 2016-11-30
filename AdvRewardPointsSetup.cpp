// AdvRewardPointsSetup.cpp : implementation file
//

#include "stdafx.h"
#include "AdvRewardPointsSetup.h"
#include "AdministratorRc.h"
#include "internationalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.wetta 2007-05-21 16:53) - PLID 26078 - Created AdvRewardPointsSetup dialog

/////////////////////////////////////////////////////////////////////////////
// CAdvRewardPointsSetup dialog


CAdvRewardPointsSetup::CAdvRewardPointsSetup(CWnd* pParent)
	: CNxDialog(CAdvRewardPointsSetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvRewardPointsSetup)
	//}}AFX_DATA_INIT
	m_bChanged = false;
	m_erpstType = erpstAccumulate;
}


void CAdvRewardPointsSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvRewardPointsSetup)
	DDX_Control(pDX, IDC_SHOW_PRICE, m_btnShowPrice);
	DDX_Control(pDX, IDC_RADIO_REWARD_DOLLARS, m_btnDollars);
	DDX_Control(pDX, IDC_RADIO_REWARD_PERCENT, m_btnPercent);
	DDX_Control(pDX, IDC_APPLY_QUICK_REWARD_DISCOUNTS, m_btnApplyDiscount);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_APPLY_QUICK_REWARD_VALUE, m_btnApply);
	DDX_Control(pDX, IDC_QUICK_REWARD_VALUE, m_nxeditQuickRewardValue);
	DDX_Control(pDX, IDC_QUICK_REWARD_DISCOUNT, m_nxeditQuickRewardDiscount);
	DDX_Control(pDX, IDC_SET_REWARD_VALUE, m_nxstaticSetRewardValue);
	DDX_Control(pDX, IDC_REWARD_VALUES_FOR_ALL, m_nxstaticRewardValuesForAll);
	DDX_Control(pDX, IDC_SET_REWARD_DISCOUNT, m_nxstaticSetRewardDiscount);
	DDX_Control(pDX, IDC_REWARD_DISCOUNTS_FOR_ALL, m_nxstaticRewardDiscountsForAll);
	DDX_Control(pDX, IDC_CATEGORY_REWARD_TEXT, m_nxstaticCategoryRewardText);
	DDX_Control(pDX, IDC_SUPPLIER_REWARD_TEXT, m_nxstaticSupplierRewardText);
	DDX_Control(pDX, IDC_REWARDS_LIST_HEADER, m_nxstaticRewardsListHeader);
	DDX_Control(pDX, IDC_ARPS_CHECK_DISCOUNT, m_chkDiscount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvRewardPointsSetup, CNxDialog)
	//{{AFX_MSG_MAP(CAdvRewardPointsSetup)
	ON_EN_KILLFOCUS(IDC_QUICK_REWARD_VALUE, OnKillfocusQuickRewardValue)
	ON_BN_CLICKED(IDC_APPLY_QUICK_REWARD_VALUE, OnApplyQuickRewardValue)
	ON_BN_CLICKED(IDC_APPLY_QUICK_REWARD_DISCOUNTS, OnApplyQuickRewardDiscounts)
	ON_EN_KILLFOCUS(IDC_QUICK_REWARD_DISCOUNT, OnKillfocusQuickRewardDiscount)
	ON_BN_CLICKED(IDC_SHOW_PRICE, OnShowPrice)
	ON_BN_CLICKED(IDC_RADIO_REWARD_PERCENT, OnRadioRewardPercent)
	ON_BN_CLICKED(IDC_RADIO_REWARD_DOLLARS, OnRadioRewardDollars)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvRewardPointsSetup message handlers

BOOL CAdvRewardPointsSetup::OnInitDialog() 
{
	try {
		using namespace NXDATALIST2Lib;

		CNxDialog::OnInitDialog();

		// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnApplyDiscount.AutoSet(NXB_MODIFY);

		m_chkDiscount.SetWindowTextA("Decrement point value when charge is discounted. (Note: If a charge has both patient and insurance responsibility the entire charge amount will be used when calculating the percentage to decrement the point value.  If a charge is insurance only, the point value will be 0.)");
		
		m_pItemTypeList = BindNxDataList2Ctrl(IDC_REWARD_ITEM_TYPE_COMBO, false);
		CString strFrom;
		if (IsRedeem()) {
			strFrom = "(SELECT 0 AS ID, 'Service Codes' AS Type UNION SELECT 1 AS ID, 'Inventory Items' AS Type) TypeQ";
		} else if (IsAccumulate()) {
			strFrom = "(SELECT 0 AS ID, 'Service Codes' AS Type UNION SELECT 1 AS ID, 'Inventory Items' AS Type UNION SELECT 2 AS ID, 'Gift Certificates' AS Type) TypeQ";
		} else {
			ASSERT(FALSE);
		}

		g_propManager.CachePropertiesInBulk("AdvRewardPointsSetup", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'RewardCalcDiscountInPoints' OR "
				"Name = 'RewardPointsSetup_ShowPrice' "
				")",
				_Q(GetCurrentUserName()));

		// (j.gruber 2010-07-28 15:59) - PLID 31147 - added checkbox for decrementing when discounted
		CheckDlgButton(IDC_ARPS_CHECK_DISCOUNT, GetRemotePropertyInt("RewardCalcDiscountInPoints", 0, 0, "<None>"));

		m_pItemTypeList->PutFromClause((LPCTSTR)strFrom);

		m_pItemTypeList->Requery();

		m_pItemTypeList2 = BindNxDataList2Ctrl(IDC_REWARD_ITEM_TYPE_COMBO2, true);

		// Set up the category list
		m_pCategoryList = BindNxDataList2Ctrl(IDC_REWARD_CATEGORY_COMBO, true);

		// Set up the supplier list
		m_pSupplierList = BindNxDataList2Ctrl(IDC_REWARD_SUPPLIER_COMBO, true);

		m_pRewardValueList = BindNxDataList2Ctrl(IDC_REWARD_VALUE_LIST, false);

		// (a.walling 2007-09-21 13:43) - PLID 26172 - Recall previous 'show price' state
		BOOL bShowPrice = GetRemotePropertyInt("RewardPointsSetup_ShowPrice", FALSE, 0, GetCurrentUserName(), true);
		CheckDlgButton(IDC_SHOW_PRICE, bShowPrice);

		// (a.walling 2007-09-21 13:36) - PLID 26172 - Store the current price in a column as well.
		IColumnSettingsPtr(m_pRewardValueList->GetColumn(m_pRewardValueList->InsertColumn(rlfPrice, _T("Price"), _T("Price"), bShowPrice ? 80 : 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;

		// (a.walling 2007-05-29 17:46) - PLID 26172 - Handle being used to setup the point costs to redeem for services.
		if (IsRedeem()) {
			// set up the dialog text

			SetWindowText("Reward Points Redemption Setup");
			SetDlgItemText(IDC_SET_REWARD_VALUE, "Set the point cost to be");
			SetDlgItemText(IDC_REWARDS_LIST_HEADER, "Service/Inventory Item Reward Point Costs");

			SetDlgItemText(IDC_APPLY_QUICK_REWARD_VALUE, "Apply Costs");

			CheckDlgButton(IDC_RADIO_REWARD_PERCENT, TRUE);

			SetDlgItemText(IDC_RADIO_REWARD_DOLLARS, GetCurrencySymbol());

			// set up the columns
			{
				IColumnSettingsPtr pCol = m_pRewardValueList->GetColumn(rlfPoints);

				pCol->FieldName = _bstr_t("PointCost");
				pCol->ColumnTitle = _bstr_t("Point Cost");
				pCol->PutColumnStyle(csVisible|csFixedWidth|csEditable); // should be editable

				pCol = m_pRewardValueList->GetColumn(rlfOldPoints);

				pCol->FieldName = _bstr_t("PointCost");
				pCol->ColumnTitle = _bstr_t("Old Point Cost");
				
				// setup some new columns
				IColumnSettingsPtr(m_pRewardValueList->GetColumn(m_pRewardValueList->InsertColumn(rlfDiscountID, _T("RewardDiscountID"), _T("RewardDiscountID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_pRewardValueList->GetColumn(m_pRewardValueList->InsertColumn(rlfPercent, _T("DiscountPercent"), _T("Discount %"), 80, csVisible|csFixedWidth|csEditable)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_pRewardValueList->GetColumn(m_pRewardValueList->InsertColumn(rlfOldPercent, _T("DiscountPercent"), _T("Old Discount %"), 0, csVisible|csFixedWidth|csEditable)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_pRewardValueList->GetColumn(m_pRewardValueList->InsertColumn(rlfDollars, _T("DiscountDollars"), _T(_bstr_t(CString("Discount ") + GetCurrencySymbol())), 80, csVisible|csFixedWidth|csEditable)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_pRewardValueList->GetColumn(m_pRewardValueList->InsertColumn(rlfOldDollars, _T("DiscountDollars"), _T("Old Discount"), 0, csVisible|csFixedWidth|csEditable)))->FieldType = cftTextSingleLine;
			};

		} else if (IsAccumulate()) {
			// perhaps move the dialog items down a bit

			const long nAccControls = 5;
			long arAccControls[] = {
				IDC_SET_REWARD_VALUE,
				IDC_QUICK_REWARD_VALUE,
				IDC_REWARD_VALUES_FOR_ALL,
				IDC_REWARD_ITEM_TYPE_COMBO,
				IDC_APPLY_QUICK_REWARD_VALUE
			};

			const long nRedeemControls = 7;
			long arRedeemControls[] = {
				IDC_SET_REWARD_DISCOUNT,
				IDC_QUICK_REWARD_DISCOUNT,
				IDC_RADIO_REWARD_PERCENT,
				IDC_RADIO_REWARD_DOLLARS,
				IDC_REWARD_DISCOUNTS_FOR_ALL,
				IDC_REWARD_ITEM_TYPE_COMBO2,
				IDC_APPLY_QUICK_REWARD_DISCOUNTS
			};

			for (int i = 0; i < nRedeemControls; i++) {
				CWnd* pWnd = GetDlgItem(arRedeemControls[i]);

				pWnd->ShowWindow(SW_HIDE);
			}

			for (int j = 0; j < nAccControls; j++) {
				CWnd* pWnd = GetDlgItem(arAccControls[j]);

				CRect rc;
				pWnd->GetWindowRect(rc);

				ScreenToClient(rc);

				rc.top += 15;
				rc.bottom += 15;

				pWnd->MoveWindow(rc);
			}
		}

		RefreshRewardValueList();

		// Set the default reward value
		SetDlgItemText(IDC_QUICK_REWARD_VALUE, "0");
		SetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, "0");

		// Hide the supplier info
		GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(FALSE);

		// Hide the category info
		GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(FALSE);

	}NxCatchAll("Error in CAdvRewardPointsSetup::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvRewardPointsSetup::RefreshRewardValueList()
{
	try {
		CString str;
		if (IsRedeem()) {
			// (a.walling 2007-05-30 09:10) - PLID 26172 - Ignore GCs, include Point Costs.
			// (r.farnworth 2013-08-07 16:33) - PLID 45994 - Removed some unnecesary left joining on the GCTypes Table
			str.Format("(SELECT ServiceT.ID AS ID, "
						"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 'Service Code' WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' ELSE 'Service Code' END AS Type, "
						"CASE WHEN CPTCodeT.ID IS NOT NULL THEN CPTCodeT.Code WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' END AS Code, ServiceT.Name AS Name, CategoriesT.ID AS CatID, CategoriesT.Name AS Category, convert(float, ServiceT.PointCost) AS PointCost, "
						"RewardDiscountsT.DiscountPercent, RewardDiscountsT.DiscountDollars, RewardDiscountsT.ID AS RewardDiscountID, "
						"ServiceT.Price AS Price " // (a.walling 2007-09-21 13:39) - PLID 26172 - Added price
						"FROM ServiceT "
						"LEFT JOIN RewardDiscountsT ON ServiceT.ID = RewardDiscountsT.ServiceID "
						"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
						"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
						"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
						"WHERE ServiceT.ID NOT IN (SELECT ServiceID FROM GCTypesT)) RewardQ");
		} else {
			str.Format("(SELECT ServiceT.ID AS ID, "
						"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 'Service Code' WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' ELSE 'Service Code' END AS Type, "
						"CASE WHEN CPTCodeT.ID IS NOT NULL THEN CPTCodeT.Code WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' END AS Code, ServiceT.Name AS Name, CategoriesT.ID AS CatID, CategoriesT.Name AS Category, convert(float, ServiceT.Points) AS Points, "
						"ServiceT.Price AS Price " // (a.walling 2007-09-21 13:42) - PLID 26172 - Added price here too
						"FROM ServiceT "
						"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
						"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
						"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
						"WHERE ServiceT.ID NOT IN (SELECT ServiceID FROM GCTypesT)) RewardQ");
		}
		m_pRewardValueList->PutFromClause(_bstr_t(str));
		m_pRewardValueList->Requery();

		m_bChanged = false;
	}NxCatchAll("Error in CAdvRewardPointsSetup::RefreshRewardValueList");
}

void CAdvRewardPointsSetup::OnCancel() 
{
	if (m_bChanged == true && MessageBox("Are you sure you wish to close without saving your changes?", NULL, MB_YESNO|MB_ICONQUESTION) != IDYES)
		return;
	
	CDialog::OnCancel();
}

void CAdvRewardPointsSetup::OnOK() 
{
	if (!Save())
		return;
	
	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CAdvRewardPointsSetup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvRewardPointsSetup)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_ITEM_TYPE_COMBO, 2 /* SelChanged */, OnSelChangedRewardItemTypeCombo, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_VALUE_LIST, 10 /* EditingFinished */, OnEditingFinishedRewardValueList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_VALUE_LIST, 9 /* EditingFinishing */, OnEditingFinishingRewardValueList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_SUPPLIER_COMBO, 1 /* SelChanging */, OnSelChangingRewardSupplierCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_CATEGORY_COMBO, 1 /* SelChanging */, OnSelChangingRewardCategoryCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_ITEM_TYPE_COMBO2, 2 /* SelChanged */, OnSelChangedRewardItemTypeCombo2, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_CATEGORY_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCategoryCombo, VTS_I2)
	ON_EVENT(CAdvRewardPointsSetup, IDC_REWARD_SUPPLIER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedSupplierCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvRewardPointsSetup::OnSelChangedRewardItemTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try{
		if (lpNewSel) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

			long nSelection = VarLong(pRow->GetValue(0));

			// Show the	supplier info if needed
			if (nSelection == ritInventoryItem) {
				// Enable the supplier information
				// For some reason the text has to be hidden and then re-shown or it will not redraw correctly, rather annoying
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->ShowWindow(FALSE);
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(TRUE);
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->ShowWindow(TRUE);
				GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(FALSE);
				GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(FALSE);
			}

			// Show the	category info if needed
			if (nSelection != ritGiftCertificate) {
				// Enable the category information
				// For some reason the text has to be hidden and then re-shown or it will not redraw correctly, rather annoying
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->ShowWindow(FALSE);
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(TRUE);
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->ShowWindow(TRUE);
				GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(FALSE);
				GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(FALSE);
			}
		}
		else {
			// Hide the supplier info
			GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(FALSE);
			// Hide the category info
			GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnSelChangedRewardItemTypeCombo");		
}

void CAdvRewardPointsSetup::OnEditingFinishingRewardValueList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try{
		CString strTemp;
		CString strEntered = strUserEntered;

		if (!(*pbCommit)) {
			*pbContinue = TRUE;
			return;
		}

		if (nCol == rlfPoints) {

			// (d.singleton 2012-03-21 10:27) - PLID 48264 - if you have french regional settings and put a period in the value ( 20.25 )
			//it will clear out the value as it is invalid, so check for invalid and handle accordingly.
			COleCurrency cyPoints;
			if(strEntered.IsEmpty()) {
				cyPoints = COleCurrency(0,0);
			}
			else {
				cyPoints.ParseCurrency(strEntered);
			}
			if (cyPoints.GetStatus() == COleCurrency::invalid && *pbCommit) {
				MessageBox("The entered value is invalid!.", NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}
			// Do not allow them to assign negative amounts
			if (LooseCompareDouble(VarDouble(*pvarNewValue, 0), 0.0, 0.001) == -1 && *pbCommit) {
				CString strMsg;
				if (IsRedeem())
					strMsg.Format("The point cost of a service or item cannot be less than zero.");
				else
					strMsg.Format("You may not assign a reward point value less than 0.");
				MessageBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}

			if (LooseCompareDouble(VarDouble(*pvarNewValue, 0), 0.0, 0.001) == 0) {
				// If the value is 0, it should just be null
				pvarNewValue->vt = VT_NULL;
			} else {
				// Let's round the point value
				strTemp.Format("%.2f", VarDouble(*pvarNewValue, 0));
				pvarNewValue->dblVal = atof(strTemp);
			}
		} else if (nCol == rlfPercent) {
			long nPercent = atol(strUserEntered);

			if (nPercent < 0 && *pbCommit) {
				MessageBox("You may not assign a percentage discount less than 0.", NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			} else if (nPercent > 100 && *pbCommit) {
				MessageBox("You may not assign a percentage discount greater than 100.", NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}

			VariantClear(pvarNewValue);

			if (nPercent == 0) {
				*pvarNewValue = g_cvarNull;
			} else {
				*pvarNewValue = _variant_t(nPercent, VT_I4);
			}
		} else if (nCol == rlfDollars) {
			COleCurrency cyDollars;
			if (strEntered.IsEmpty())
				cyDollars = COleCurrency(0, 0);
			else
				cyDollars.ParseCurrency(strUserEntered);

			if (cyDollars.GetStatus() == COleCurrency::invalid && *pbCommit) {
				MessageBox("The entered value is invalid!.", NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}

			if (cyDollars < COleCurrency(0, 0) && *pbCommit) {
				MessageBox("You may not assign a discount less than 0.", NULL, MB_OK|MB_ICONEXCLAMATION);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {
				COleCurrency cyCurrentPrice = VarCurrency(pRow->GetValue(rlfPrice));

				// (a.walling 2007-09-21 13:56) - PLID 26172 - Warn when trying to use a higher discount than price, but still allow in case they want to. Perhaps for tax.
				//(c.copits 2011-10-11) PLID 36337 - Disallow a larger redeemable discount when it exceeds the price of that item
				if (cyDollars > cyCurrentPrice) {
					CString strMessage;
					strMessage.Format("A discount of %s will exceed this item's current price of %s.\n"
								"Discounts may not exceed the item's price.",
								FormatCurrencyForInterface(cyDollars), FormatCurrencyForInterface(cyCurrentPrice));
					MessageBox(strMessage);
					/*
					if (IDNO == MessageBox(FormatString("A discount of %s will exceed this item's current price of %s! Any excess discount will need to be adjusted on the bill when redeemed. Do you want to continue?", 
												FormatCurrencyForInterface(cyDollars), FormatCurrencyForInterface(cyCurrentPrice)),
												NULL, MB_YESNO | MB_ICONASTERISK)) {
					*/
						*pbContinue = FALSE;
						*pbCommit = FALSE;
						return;	
					//}
				}
			}

			VariantClear(pvarNewValue);

			if (cyDollars == COleCurrency(0, 0)) {
				*pvarNewValue = g_cvarNull;
			} else {
				*pvarNewValue = _variant_t((CURRENCY)cyDollars);
			}
		}
	
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnEditingFinishingRewardValueList");	
}

void CAdvRewardPointsSetup::OnEditingFinishedRewardValueList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {		
		if (lpRow && bCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			_variant_t varNull;
			varNull.vt = VT_NULL;

			if (nCol == rlfPoints) {
				if (IsRedeem() && varNewValue.vt == VT_NULL) {
					// (a.walling 2007-06-11 10:33) - PLID 26172 - if we are clearing the cost, we should
					// also clear out the discounts
					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
					if (pRow) {
						pRow->PutValue(rlfPercent, g_cvarNull);
						pRow->PutValue(rlfDollars, g_cvarNull);
					}
				}

				if (varOldValue.vt == VT_NULL && LooseCompareDouble(VarDouble(varNewValue, 0), 0.0, 0.001) == 0) {
					// Nothing was actually changed
					pRow->PutValue(nCol, varNull);
					return;
				}
			} else if (nCol == rlfPercent) {
				if (varOldValue.vt == VT_NULL && VarLong(varNewValue, 0) == 0) {
					pRow->PutValue(nCol, varNull);
					return;
				} else {
					// clear the opposing discount
					pRow->PutValue(rlfDollars, varNull);
				}
			} else if (nCol == rlfDollars) {
				if (varOldValue.vt == VT_NULL && VarCurrency(varNewValue, COleCurrency(0, 0)) == COleCurrency(0, 0)) {
					pRow->PutValue(nCol, varNull);
					return;
				} else {
					// clear the opposing discount
					pRow->PutValue(rlfPercent, varNull);
				}
			}

			m_bChanged = true;
		}

	}NxCatchAll("Error in CAdvRewardPointsSetup::OnEditingFinishedRewardValueList");	
}

void CAdvRewardPointsSetup::OnKillfocusQuickRewardValue() 
{
	try {
		// Get the discount amount
		CString strPointValue;
		GetDlgItemText(IDC_QUICK_REWARD_VALUE, strPointValue);
		strPointValue.TrimLeft();
		strPointValue.TrimRight();

		// Make sure that it is a valid currency
		if (!strPointValue.IsEmpty() ) {
			COleCurrency cyPointValue = ParseCurrencyFromInterface(strPointValue);
			if (cyPointValue.GetStatus() == COleCurrency::invalid) {
				MessageBox(IsRedeem() ? "Please enter a valid point cost." : "Please enter a valid point value.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_QUICK_REWARD_VALUE, "");
				GetDlgItem(IDC_QUICK_REWARD_VALUE)->SetFocus();
				return;
			}

			// Do not allow them to assign negative amounts
			if (cyPointValue < COleCurrency(0,0)) {
				MessageBox(IsRedeem() ? "The point cost of a service or item cannot be less than zero." : "You may not assign a reward point value less than 0.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_QUICK_REWARD_VALUE, "");
				GetDlgItem(IDC_QUICK_REWARD_VALUE)->SetFocus();
				return;
			}

			CString strTemp = FormatCurrencyForInterface(cyPointValue, FALSE, TRUE);

			SetDlgItemText(IDC_QUICK_REWARD_VALUE, strTemp);
		}	
		else {
			SetDlgItemText(IDC_QUICK_REWARD_VALUE, "");
		}
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnKillfocusQuickRewardValue");	
}

void CAdvRewardPointsSetup::OnSelChangingRewardSupplierCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnSelChangingRewardSupplierCombo");	
}

void CAdvRewardPointsSetup::OnSelChangingRewardCategoryCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnSelChangingRewardCategoryCombo");	
}

void CAdvRewardPointsSetup::OnApplyQuickRewardValue() 
{
	try {
		CWaitCursor wait;

		/////////////////////////////
		// First check to make sure that everything that is selected has been selected
		// Check the item type
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pItemTypeList->GetCurSel();
		long nItemType = -1;
		if (pRow) {	
			nItemType = VarLong(pRow->GetValue(0));
		}
		else {
			if (!IsRedeem()) {
				MessageBox("No item type has been selected (Service Code, Inventory Item, or Gift Certificate).\n"
							"Please select an item type.", NULL, MB_OK|MB_ICONEXCLAMATION);
			} else {
				MessageBox("No item type has been selected (Service Code or Inventory Item).\n"
							"Please select an item type.", NULL, MB_OK|MB_ICONEXCLAMATION);
			}
			return;
		}

		// Get the point value
		CString strPoints;
		GetDlgItemText(IDC_QUICK_REWARD_VALUE, strPoints);

		if(strPoints.IsEmpty()) {
			MessageBox(IsRedeem() ? "You must enter a point cost before applying." : "You must enter a point value before applying.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Round the point value
		double dblPointValue = 0;
		CString strPointValue = "";
		// (j.gruber 2009-07-10 16:06) - PLID 34175 - fix it so they can enter more then 3 places
		COleCurrency cy = ParseCurrencyFromInterface(strPoints);
		strPoints = FormatCurrencyForSql(cy);
		strPointValue.Format("%.2f", atof(strPoints));
		strPointValue.TrimRight("0");
		strPointValue.TrimRight(".");
		dblPointValue = atof(strPointValue);

		// Check the supplier
		if (nItemType == ritInventoryItem && m_pSupplierList->GetCurSel() == NULL) {
			// Somehow no supplier row is selected
			MessageBox("No supplier has been selected.  Please select a supplier.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Check the category
		if (nItemType != ritGiftCertificate && m_pCategoryList->GetCurSel() == NULL) {
			// Somehow no category row is selected
			MessageBox("No category has been selected.  Please select a category.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		///////////////////////////////////

		///////////////////////////////////
		// Ok, everything checks out, now let's apply the discount 
		// Get the list of inventory items for a particular supplier, if this is an inventory item
		CMap<long, long, long, long> mapSupplierItems;
		long nSupplierID = -1;
		CString strSupplierName = "";
		if (nItemType == ritInventoryItem) {
			pRow = m_pSupplierList->GetCurSel();
			nSupplierID = VarLong(pRow->GetValue(0), -1);
			strSupplierName = VarString(pRow->GetValue(1));
			if (nSupplierID != -1) {
				// Get the inventory items that have this supplier
				_RecordsetPtr rs = CreateRecordset("select ProductID from MultiSupplierT where SupplierID = %li", nSupplierID);
				while (!rs->eof) {
					mapSupplierItems[AdoFldLong(rs, "ProductID")] = TRUE;
					rs->MoveNext();
				}
				rs->Close();
			}
		}

		// Get the category
		long nCategoryID = -1;
		CString strCategoryName = "";
		if (nItemType != ritGiftCertificate) {
			pRow = m_pCategoryList->GetCurSel();
			nCategoryID = VarLong(pRow->GetValue(0), -1);
			strCategoryName = VarString(pRow->GetValue(1));
		}

		// Warn the user before updating
		CString strMsg, strItemType;
		strItemType = nItemType == ritServiceCode ? "Service Code" : (nItemType == ritInventoryItem ? "Inventory Item" : "Gift Certificate");
		if (IsRedeem()) {
			strMsg.Format("Are you sure you want to update all of the %ss%s%s to have a point cost of %s?  Any point cost these items "
				"currently have will be removed.", strItemType, 
				(nItemType != ritGiftCertificate && nCategoryID != -1) ? " in the category \'" + strCategoryName + "\'" : "",
				(nItemType == ritInventoryItem && nSupplierID != -1) ? " with the supplier \'" + strSupplierName + "\'" : "", strPointValue);
		} else {
			strMsg.Format("Are you sure you want to update all of the %ss%s%s to have a point value of %s?  Any point value these items "
				"currently have will be removed.", strItemType, 
				(nItemType != ritGiftCertificate && nCategoryID != -1) ? " in the category \'" + strCategoryName + "\'" : "",
				(nItemType == ritInventoryItem && nSupplierID != -1) ? " with the supplier \'" + strSupplierName + "\'" : "", strPointValue);
		}
		if(MessageBox(strMsg, NULL, MB_YESNO|MB_ICONQUESTION) != IDYES)
			return;

		wait.Restore();

		// Go through all of the service items in the list and update them accordingly
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow = m_pRewardValueList->GetFirstRow();
		while (pRow) {
			// Make sure that this inventory item is offered by the selected supplier or is not an inventory item and is in the selected category
			if (VarString(pRow->GetValue(rlfType)) == strItemType && (nSupplierID == -1 || mapSupplierItems[VarLong(pRow->GetValue(rlfID))]) &&
				(nCategoryID == -1 || VarLong(pRow->GetValue(rlfCategoryID), -1) == nCategoryID)) {
				if (dblPointValue == 0)
					pRow->PutValue(rlfPoints, _variant_t(varNull));
				else {
					pRow->PutValue(rlfPoints, _variant_t(dblPointValue));
				}
			}
			pRow = pRow->GetNextRow();
		}

		/////////////////////////////////

		m_bChanged = true;

	}NxCatchAll("Error in CAdvRewardPointsSetup::OnApplyQuickRewardValue");	
}

BOOL CAdvRewardPointsSetup::Save()
{
	try {
		CWaitCursor wait;

		// Start the save query
		CString strSql = BeginSqlBatch();
		CString strSqlStatement = "";

		// (a.walling 2011-07-22 13:19) - PLID 44681 - Keep track of whether we made any changes
		int nChangedPoints = 0;
		int nChangedRedeem = 0;

		// Go through all of the service items in the list and determine what needs to be done
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRewardValueList->GetFirstRow();
		while (pRow) {
			strSqlStatement = "";

			// Get the discount values
			double dblPoints = VarDouble(pRow->GetValue(rlfPoints), 0);
			double dblOldPoints = VarDouble(pRow->GetValue(rlfOldPoints), 0);
			CString strTemp;
			strTemp.Format("%.2f", dblPoints);
			COleCurrency cyPoints;
			// (d.singleton 2012-03-20 17:36) - PLID 48262 - Canada french system settings cause error so format for regional settings
			cyPoints.ParseCurrency(FormatNumberForInterface(strTemp, FALSE, TRUE, 2));			
			long nServiceID = VarLong(pRow->GetValue(rlfID));

			// Update the values in the database that have been changed
			if (dblOldPoints != dblPoints) {
				if (dblPoints != 0) {
					strSqlStatement.Format("UPDATE ServiceT SET %s = %s WHERE ID = %li", 
						IsRedeem() ? "PointCost" : "Points", FormatCurrencyForSql(cyPoints), nServiceID);
				}
				else {
					strSqlStatement.Format("UPDATE ServiceT SET %s = NULL WHERE ID = %li", 
						IsRedeem() ? "PointCost" : "Points", nServiceID);
				}
				AddStatementToSqlBatch(strSql, strSqlStatement);

				++nChangedPoints;
			}

			if (IsRedeem()) {
				long nPercent = VarLong(pRow->GetValue(rlfPercent), 0);
				long nOldPercent = pRow->GetValue(rlfOldPercent).vt == VT_EMPTY ? 0 : VarLong(pRow->GetValue(rlfOldPercent), 0);

				COleCurrency cyDollars = VarCurrency(pRow->GetValue(rlfDollars), COleCurrency(0, 0));
				COleCurrency cyOldDollars = pRow->GetValue(rlfOldDollars).vt == VT_EMPTY ? COleCurrency(0, 0) : VarCurrency(pRow->GetValue(rlfOldDollars), COleCurrency(0, 0));

				long nDiscountID = pRow->GetValue(rlfDiscountID).vt == VT_EMPTY ? -1 : VarLong(pRow->GetValue(rlfDiscountID), -1);

				CString strPercentValue, strDollarValue;

				ASSERT(cyDollars == COleCurrency(0, 0) || nPercent == 0);

				if ( (cyDollars != cyOldDollars) || (nPercent != nOldPercent) ) {
					// a value has changed, add to the batch.

					if (nPercent == 0)
						strPercentValue = "NULL";
					else
						strPercentValue = AsStringForSql(pRow->GetValue(rlfPercent));

					if (cyDollars == COleCurrency(0, 0))
						strDollarValue = "NULL";
					else
						strDollarValue = FormatCurrencyForSql(cyDollars);

					if (nDiscountID == -1) {
						// create a new record
						if (cyDollars != COleCurrency(0, 0) || nPercent != 0) // at least one needs to be set
							AddStatementToSqlBatch(strSql, "INSERT INTO RewardDiscountsT (ServiceID, DiscountDollars, DiscountPercent) VALUES (%li, %s, %s)", nServiceID, strDollarValue, strPercentValue);
					} else {
						if (cyDollars == COleCurrency(0, 0) && nPercent == 0) // both null, remove this record
							AddStatementToSqlBatch(strSql, "DELETE FROM RewardDiscountsT WHERE ServiceID = %li", nServiceID);
						else
							AddStatementToSqlBatch(strSql, "UPDATE RewardDiscountsT SET DiscountDollars = %s, DiscountPercent = %s WHERE ServiceID = %li", strDollarValue, strPercentValue, nServiceID);
					}

					++nChangedRedeem;
				}
			}

			pRow = pRow->GetNextRow();
		}

		ExecuteSqlBatch(strSql);

		// (j.gruber 2010-07-28 15:59) - PLID 31147 - added checkbox for decrementing when discounted
		SetRemotePropertyInt("RewardCalcDiscountInPoints", IsDlgButtonChecked(IDC_ARPS_CHECK_DISCOUNT), 0, "<None>");

		// (a.walling 2011-07-22 13:19) - PLID 44681 - Fire the CPTCodeT table checker if we have modified any reward points
		if ( (0 != nChangedPoints) || (0 != nChangedRedeem) ) {
			CClient::RefreshTable(NetUtils::CPTCodeT);
		}

		m_bChanged = false;

		return TRUE;
	}NxCatchAll("Error in CAdvRewardPointsSetup::Save");
	return FALSE;
}

// (a.walling 2007-05-30 15:53) - PLID 26172
void CAdvRewardPointsSetup::OnApplyQuickRewardDiscounts() 
{
	try {
		CWaitCursor wait;

		/////////////////////////////
		// First check to make sure that everything that is selected has been selected
		// Check the item type
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pItemTypeList2->GetCurSel();
		long nItemType = -1;
		if (pRow) {	
			nItemType = VarLong(pRow->GetValue(0));
		}
		else {
			MessageBox("No item type has been selected (Service Code or Inventory Item).\n"
						"Please select an item type.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Get the user-entered value
		CString strDiscount;
		GetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, strDiscount);

		if(strDiscount.IsEmpty()) {
			MessageBox("You must enter a value before applying.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Round the point value
		long nDiscountPercent = atol(strDiscount);
		CString strDiscountText = "";

		COleCurrency cyDiscountDollars;

		BOOL bPercent = IsDlgButtonChecked(IDC_RADIO_REWARD_PERCENT);

		if (bPercent) {
			strDiscountText = strDiscount + "%";
			if ( (nDiscountPercent > 100) || (nDiscountPercent < 0) ) {
				MessageBox("You must use a valid discount percentage (0 - 100) before applying.", NULL, MB_OK|MB_ICONEXCLAMATION);
				return;
			}
		} else {
			cyDiscountDollars.ParseCurrency(strDiscount);
			strDiscountText = FormatCurrencyForInterface(cyDiscountDollars);
		}

		// Check the supplier
		if (nItemType == ritInventoryItem && m_pSupplierList->GetCurSel() == NULL) {
			// Somehow no supplier row is selected
			MessageBox("No supplier has been selected.  Please select a supplier.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Check the category
		if (nItemType != ritGiftCertificate && m_pCategoryList->GetCurSel() == NULL) {
			// Somehow no category row is selected
			MessageBox("No category has been selected.  Please select a category.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		///////////////////////////////////

		///////////////////////////////////
		// Ok, everything checks out, now let's apply the discount 
		// Get the list of inventory items for a particular supplier, if this is an inventory item
		CMap<long, long, long, long> mapSupplierItems;
		long nSupplierID = -1;
		CString strSupplierName = "";
		if (nItemType == ritInventoryItem) {
			pRow = m_pSupplierList->GetCurSel();
			nSupplierID = VarLong(pRow->GetValue(0), -1);
			strSupplierName = VarString(pRow->GetValue(1));
			if (nSupplierID != -1) {
				// Get the inventory items that have this supplier
				_RecordsetPtr rs = CreateRecordset("select ProductID from MultiSupplierT where SupplierID = %li", nSupplierID);
				while (!rs->eof) {
					mapSupplierItems[AdoFldLong(rs, "ProductID")] = TRUE;
					rs->MoveNext();
				}
				rs->Close();
			}
		}

		// Get the category
		long nCategoryID = -1;
		CString strCategoryName = "";
		if (nItemType != ritGiftCertificate) {
			pRow = m_pCategoryList->GetCurSel();
			nCategoryID = VarLong(pRow->GetValue(0), -1);
			strCategoryName = VarString(pRow->GetValue(1));
		}

		// Warn the user before updating
		CString strMsg, strItemType;
		strItemType = nItemType == ritServiceCode ? "Service Code" : (nItemType == ritInventoryItem ? "Inventory Item" : "Gift Certificate");

		strMsg.Format("Are you sure you want to update all of the %ss%s%s to have a discount of %s?  Any discount properties these items "
			"currently have will be removed.", strItemType, 
			(nItemType != ritGiftCertificate && nCategoryID != -1) ? " in the category \'" + strCategoryName + "\'" : "",
			(nItemType == ritInventoryItem && nSupplierID != -1) ? " with the supplier \'" + strSupplierName + "\'" : "", strDiscountText);

		if(MessageBox(strMsg, NULL, MB_YESNO|MB_ICONQUESTION) != IDYES)
			return;

		wait.Restore();

		// Go through all of the service items in the list and update them accordingly
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow = m_pRewardValueList->GetFirstRow();
		long nExceeded = 0;
		while (pRow) {
			// Make sure that this inventory item is offered by the selected supplier or is not an inventory item and is in the selected category
			if (VarString(pRow->GetValue(rlfType)) == strItemType && (nSupplierID == -1 || mapSupplierItems[VarLong(pRow->GetValue(rlfID))]) &&
				(nCategoryID == -1 || VarLong(pRow->GetValue(rlfCategoryID), -1) == nCategoryID)) {

				if (bPercent) {
					if (nDiscountPercent == 0)
						pRow->PutValue(rlfPercent, g_cvarNull);
					else
						pRow->PutValue(rlfPercent, _variant_t(nDiscountPercent));

					pRow->PutValue(rlfDollars, g_cvarNull);
				} else {
					// (a.walling 2007-09-21 13:48) - PLID 26172 - Get the current price
					COleCurrency cyCurrentPrice = VarCurrency(pRow->GetValue(rlfPrice));

					// (a.walling 2007-09-21 14:18) - PLID 26172 - Use NULL if the current price is 0
					if ((cyDiscountDollars == COleCurrency(0, 0)) || (cyCurrentPrice == COleCurrency(0, 0)))
						pRow->PutValue(rlfDollars, _variant_t(varNull));
					else {
						// (a.walling 2007-09-21 13:48) - PLID 26172 - Don't put more than the current price as the discount
						if (cyDiscountDollars > cyCurrentPrice) {
							nExceeded++;
							pRow->PutValue(rlfDollars, _variant_t(cyCurrentPrice));	
						} else {
							pRow->PutValue(rlfDollars, _variant_t(cyDiscountDollars));	
						}
						
					}

					pRow->PutValue(rlfPercent, g_cvarNull);
				}
			}
			pRow = pRow->GetNextRow();
		}

		// (a.walling 2007-09-21 13:51) - PLID 26172 - If we had to adjust the price for a couple items, tell the user.
		if (nExceeded > 0) {
			MessageBox(FormatString("The dollar discount exceeded the current price of %li items. For those items, the discount was modified to equal the price.", nExceeded), NULL, MB_OK | MB_ICONHAND);
		}

		/////////////////////////////////

		m_bChanged = true;

	}NxCatchAll("Error in CAdvRewardPointsSetup::OnApplyQuickRewardDiscounts");		
}

void CAdvRewardPointsSetup::OnKillfocusQuickRewardDiscount() 
{
	try {
		// Get the discount amount
		CString strDiscount;
		GetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, strDiscount);
		strDiscount.TrimLeft();
		strDiscount.TrimRight();

		BOOL bPercent = IsDlgButtonChecked(IDC_RADIO_REWARD_PERCENT);

		// Make sure that it is a valid currency
		if (!strDiscount.IsEmpty() ) {
			COleCurrency cyPointValue = ParseCurrencyFromInterface(strDiscount);
			if (cyPointValue.GetStatus() == COleCurrency::invalid) {
				MessageBox("Please enter a valid discount.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, "");
				GetDlgItem(IDC_QUICK_REWARD_DISCOUNT)->SetFocus();
				return;
			}

			// Do not allow them to assign negative amounts
			if (cyPointValue < COleCurrency(0,0)) {
				MessageBox("You may not assign a discount value less than 0.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, "");
				GetDlgItem(IDC_QUICK_REWARD_DISCOUNT)->SetFocus();
				return;
			}

			CString strTemp;
			if (bPercent) {
				strTemp.Format("%.2f", atof(strDiscount));
				strTemp.TrimRight("0");
				strTemp.TrimRight(".");
			} else {				
				strTemp = FormatCurrencyForInterface(cyPointValue, FALSE, TRUE);
			}
			SetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, strTemp);
		}	
		else {
			SetDlgItemText(IDC_QUICK_REWARD_DISCOUNT, "");
		}
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnKillfocusQuickRewardDiscount");		
}

void CAdvRewardPointsSetup::OnSelChangedRewardItemTypeCombo2(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try{
		if (lpNewSel) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

			long nSelection = VarLong(pRow->GetValue(0));

			// Show the	supplier info if needed
			if (nSelection == ritInventoryItem) {
				// Enable the supplier information
				// For some reason the text has to be hidden and then re-shown or it will not redraw correctly, rather annoying
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->ShowWindow(FALSE);
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(TRUE);
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->ShowWindow(TRUE);
				GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(FALSE);
				GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(FALSE);
			}

			// Show the	category info if needed
			if (nSelection != ritGiftCertificate) {
				// Enable the category information
				// For some reason the text has to be hidden and then re-shown or it will not redraw correctly, rather annoying
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->ShowWindow(FALSE);
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(TRUE);
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->ShowWindow(TRUE);
				GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(FALSE);
				GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(FALSE);
			}
		}
		else {
			// Hide the supplier info
			GetDlgItem(IDC_SUPPLIER_REWARD_TEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REWARD_SUPPLIER_COMBO)->EnableWindow(FALSE);
			// Hide the category info
			GetDlgItem(IDC_CATEGORY_REWARD_TEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REWARD_CATEGORY_COMBO)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CAdvRewardPointsSetup::OnSelChangedRewardItemTypeCombo2");
}

void CAdvRewardPointsSetup::OnRequeryFinishedCategoryCombo(short nFlags)
{
	try
	{
		// Add an { All } row
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pCategoryList->GetNewRow();
		pNewRow->PutValue(0, _variant_t((long)-1));
		pNewRow->PutValue(1, _variant_t("{ All }"));
		m_pCategoryList->AddRowBefore(pNewRow, m_pCategoryList->GetFirstRow());
		// Select the { All } row
		m_pCategoryList->SetSelByColumn(0, _variant_t((long)-1));

	}NxCatchAll("CAdvRewardPointsSetup::OnRequeryFinishedCategoryCombo");
}

void CAdvRewardPointsSetup::OnRequeryFinishedSupplierCombo(short nFlags)
{
	try
	{
		// Add an { All } row
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pSupplierList->GetNewRow();
		pNewRow->PutValue(0, _variant_t((long)-1));
		pNewRow->PutValue(1, _variant_t("{ All }"));
		m_pSupplierList->AddRowBefore(pNewRow, m_pSupplierList->GetFirstRow());
		// Select the { All } row
		m_pSupplierList->SetSelByColumn(0, _variant_t((long)-1));

	}NxCatchAll("CAdvRewardPointsSetup::OnRequeryFinishedSupplierCombo");
}

void CAdvRewardPointsSetup::OnShowPrice() 
{
	try {
		BOOL bChecked = IsDlgButtonChecked(IDC_SHOW_PRICE);

		SetRemotePropertyInt("RewardPointsSetup_ShowPrice", bChecked, 0, GetCurrentUserName());

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pRewardValueList->GetColumn(rlfPrice);
		if (pCol) {
			pCol->PutStoredWidth(bChecked ? 80 : 0);
		}
	} NxCatchAll("Error in CAdvRewardPointsSetup::OnShowPrice()");
}

// (a.walling 2007-09-21 14:17) - PLID 26172
void CAdvRewardPointsSetup::OnRadioRewardPercent() 
{
	OnKillfocusQuickRewardDiscount();
}

void CAdvRewardPointsSetup::OnRadioRewardDollars() 
{
	OnKillfocusQuickRewardDiscount();
}
