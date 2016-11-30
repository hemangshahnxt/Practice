// ChargeDiscountDlg.cpp : implementation file
//
// (j.gruber 2009-03-18 14:19) - PLID 33411 - created for
#include "stdafx.h"
#include "Practice.h"
#include "ChargeDiscountDlg.h"
#include "DiscountCategorySelectDlg.h"
#include "NewDiscountDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "Barcode.h"
#include "BillingDlg.h"

// CChargeDiscountDlg dialog

enum DiscountListColumns {
	dlcID = 0,
	dlcPercentOff,
	dlcDiscount,
	dlcCouponID,
	dlcDiscountCategoryID,
	dlcCustomDiscountDesc,
	dlcDiscountCategoryLink,
};

IMPLEMENT_DYNAMIC(CChargeDiscountDlg, CNxDialog)


// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
// (j.gruber 2009-04-22 10:41) - PLID 34042 - added description
// (j.jones 2011-01-21 14:38) - PLID 42156 - this dialog now requires the charge ID
CChargeDiscountDlg::CChargeDiscountDlg(CWnd* pParent, long nChargeID, DiscountList *pDiscountList, BOOL bUseCoupons, BOOL bIsBill, BOOL bCheckPermissions,
		COleCurrency cyUnitCost, COleCurrency cyOtherFee, double dblQuantity, CString strChargeDescription, 
		double dblMultiplier1 /*= -1.0*/, double dblMultiplier2 /*= -1.0*/, double dblMultiplier3 /*= -1.0*/, double dblMultiplier4 /*= -1.0*/,
		double dblTax1 /*= -1.0*/, double dblTax2 /*= -1.0*/)
	: CNxDialog(CChargeDiscountDlg::IDD, pParent)
{
	m_nChargeID = nChargeID;
	m_bUseCoupons = bUseCoupons;
	m_pDiscountList = pDiscountList;
	m_cyUnitCost = cyUnitCost;
	m_cyOtherCost = cyOtherFee;
	m_dblMultiplier1 = dblMultiplier1;
	m_dblMultiplier2 = dblMultiplier2;
	m_dblMultiplier3 = dblMultiplier3;
	m_dblMultiplier4 = dblMultiplier4;
	m_dblQuantity = dblQuantity;
	m_dblTax1 = dblTax1;
	m_dblTax2 = dblTax2;
	m_bIsBill = bIsBill;
	m_bCheckPermissions = bCheckPermissions;
	m_strChargeDescription = strChargeDescription;
	m_bScanning = false;

}

CChargeDiscountDlg::~CChargeDiscountDlg()
{
	try {

		// (j.jones 2009-04-07 10:00) - PLID 33474 - supported barcoding
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in CChargeDiscountDlg::~CChargeDiscountDlg");
}

void CChargeDiscountDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CDD_ADD_DISCOUNT, m_btnAddDiscount);
	DDX_Control(pDX, IDC_CDD_DELETE_DISCOUNT, m_btnDeleteDiscount);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CChargeDiscountDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CDD_ADD_DISCOUNT, &CChargeDiscountDlg::OnBnClickedAddDiscount)	
	ON_BN_CLICKED(IDC_CDD_DELETE_DISCOUNT, &CChargeDiscountDlg::OnBnClickedDeleteDiscount)
	ON_BN_CLICKED(IDOK, &CChargeDiscountDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CChargeDiscountDlg::OnBnClickedCancel)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
END_MESSAGE_MAP()



// CChargeDiscountDlg message handlers
CString CChargeDiscountDlg::GetDiscountCategoryName(long nDiscountCategoryID) {

	try {
		if (nDiscountCategoryID > 0) {
			ADODB::_RecordsetPtr rsName = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {INT}", nDiscountCategoryID);
			if (! rsName->eof) {
				return AdoFldString(rsName, "Description", "");
			}
		}
	}NxCatchAll("Error in CChargeDiscountDlg::GetDiscountCategoryName");

	return "";
}


CString CChargeDiscountDlg::GetCouponName(long nCouponID) {

	try {
		ADODB::_RecordsetPtr rsName = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", nCouponID);
		if (! rsName->eof) {
			return AdoFldString(rsName, "Description", "");
		}
	}NxCatchAll("Error in CChargeDiscountDlg::GetCouponName");

	return "";
}

BOOL CChargeDiscountDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//set the buttons
		m_btnAddDiscount.AutoSet(NXB_NEW);
		m_btnDeleteDiscount.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.gruber 2009-04-22 10:43) - PLID 34042 - if they sent in a charge description, add it
		if (!m_strChargeDescription.IsEmpty()) {
			SetWindowText("Discounts for " + m_strChargeDescription);
		}		

		// (j.jones 2009-04-07 10:01) - PLID 33474 - supported barcoding
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
				MsgBox("Error registering for barcode scans.  You may not be able to scan in this screen.");
			}
		}

		//load the list
		m_pDiscountDataList = BindNxDataList2Ctrl(IDC_CHARGE_DISCOUNT_LIST, false);

		//load the discount list
		if (m_pDiscountList) {

			for(int i = 0; i < m_pDiscountList->aryDiscounts.GetCount(); i++) {

				stDiscount disc = m_pDiscountList->aryDiscounts.GetAt(i);
				
				NXDATALIST2Lib::IRowSettingsPtr pRow;
				pRow = m_pDiscountDataList->GetNewRow();		

				pRow->PutValue(dlcID, disc.ID);
				pRow->PutValue(dlcPercentOff, VarLong(disc.PercentOff, 0));
				pRow->PutValue(dlcDiscount, _variant_t(VarCurrency(disc.Discount, COleCurrency(0,0))));			
				pRow->PutValue(dlcCouponID, disc.CouponID);			
				pRow->PutValue(dlcDiscountCategoryID, disc.DiscountCategoryID);
				pRow->PutValue(dlcCustomDiscountDesc, disc.CustomDiscountDescription);
				
				if (disc.DiscountCategoryID.vt == VT_I4) {
					if (disc.DiscountCategoryID.lVal == -2) {
						//its a coupon, get the name
						if (disc.CouponID.vt == VT_I4) {
							pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<Coupon> - " + GetCouponName(disc.CouponID.lVal)));						
						}
						else if (disc.CouponID.vt == VT_NULL) {
							//it's an unpecified couponID
							pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<Coupon> - <Unspecified>"));											
						}
					}
					else if (disc.DiscountCategoryID.lVal == -1) {

						//its a custom description
						CString strTemp = VarString(disc.CustomDiscountDescription, "");
						pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<Custom> - " + strTemp));
						
					}				
					else {
						//get the name of the discount category 
						pRow->PutValue(dlcDiscountCategoryLink, _variant_t(GetDiscountCategoryName(disc.DiscountCategoryID.lVal)));
					}
				}
				else {
					//its null so it doesn't have anything
					pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<No Category>"));
				}

				m_pDiscountDataList->AddRowAtEnd(pRow, NULL);
			}
		}
	}NxCatchAll("Error in CChargeDiscountDlg::OnInitDialog()"); 

	return TRUE;
	
}

// (j.jones 2009-04-07 09:53) - PLID 33474 - created for barcoding purposes
void CChargeDiscountDlg::AddNewDiscount(long nCouponID /*= -1*/, CString strCouponName /*= ""*/)
{
	try {

		// (j.jones 2011-01-21 14:35) - PLID 42156 - this is protected data, always call 
		// CanChangeHistoricFinancial if this is an existing charge
		if(m_bCheckPermissions && m_bIsBill && m_nChargeID != -2
			&& !CanChangeHistoricFinancial("Charge", m_nChargeID, bioBill, sptWrite)) {
			return;
		}

		//moved from OnBnClickedAddDiscount
		CNewDiscountDlg dlg(m_bUseCoupons, m_bIsBill, m_bCheckPermissions, NULL, nCouponID, strCouponName);
		long nResult = dlg.DoModal();
		if (nResult == IDOK) {

			//add it to our list

			long nPercent = dlg.m_nPercentDiscount;
			COleCurrency cyDiscount = dlg.m_cyDollarDiscount;
			long nDiscountCatID = dlg.m_nDiscountCategoryID;
			long nCouponID = dlg.m_nCouponID;
			CString strCustomDiscountDesc = dlg.m_strCustomDiscountDesc;
			CString strDesc = dlg.m_strCategoryDescription;

			//if the percent being applied to the line total or just regular
			if (nPercent > 0 && dlg.m_bApplyFromLineTotal) {
				
				//get the line total first and then calculate a dollar discount off that						
				long nTotalPercentOff = GetTotalPercent();
				COleCurrency cyTotalDiscount = GetTotalDiscount();
				COleCurrency cyCurrentLineTotal = CalculateLineTotal(nTotalPercentOff, cyTotalDiscount);

				//now calculate what the dollar discount will be
				COleCurrency cyTempDiscount;
				cyTempDiscount = cyCurrentLineTotal * (nPercent * 1000); 
				cyTempDiscount = cyTempDiscount / long(100000);

				cyDiscount += cyTempDiscount;

				//now clear the percent off
				nPercent= 0;
			}


			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pDiscountDataList->GetNewRow();

			pRow->PutValue(dlcID, -1);
			pRow->PutValue(dlcPercentOff, nPercent);
			pRow->PutValue(dlcDiscount, _variant_t(cyDiscount));
			pRow->PutValue(dlcCouponID, nCouponID);
			pRow->PutValue(dlcDiscountCategoryID, nDiscountCatID);		
			pRow->PutValue(dlcCustomDiscountDesc, _variant_t(strCustomDiscountDesc));
			pRow->PutValue(dlcDiscountCategoryLink, _variant_t(strDesc));

			m_pDiscountDataList->AddRowAtEnd(pRow, NULL);
		}

	}NxCatchAll("Error in CChargeDiscountDlg::AddNewDiscount");
}

void CChargeDiscountDlg::OnBnClickedAddDiscount()
{
	try {

		// (j.jones 2009-04-07 09:54) - PLID 33474 - now this calls AddNewDiscount
		AddNewDiscount();	

	}NxCatchAll("Error in CChargeDiscountDlg::OnBnClickedAddDiscount()");
}

void CChargeDiscountDlg::OnBnClickedDeleteDiscount()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountDataList->CurSel;

		if (pRow) {
			//just remove the row because we will rebuild the list when we close the dialog

			//we need to check to see if they have permission to write
			if (m_bCheckPermissions) {

				// (j.jones 2011-01-21 14:35) - PLID 42156 - this is protected data, always call 
				// CanChangeHistoricFinancial if this is an existing charge
				if(m_bIsBill && m_nChargeID != -2
					&& !CanChangeHistoricFinancial("Charge", m_nChargeID, bioBill, sptWrite)) {
					return;
				}

				long nPercentOff = VarLong(pRow->GetValue(dlcPercentOff), 0);
				if (nPercentOff > 0) {
					// (j.gruber 2009-03-31 15:35) - PLID 33554 - check permissions
					if (m_bIsBill) {
						if  (!CheckCurrentUserPermissions(bioChargePercentOff, sptWrite)) {
							//they can't remove it
							return;
						}
					}
					else {
						if  (!CheckCurrentUserPermissions(bioQuotePercentOff, sptWrite)) {
							//they can't remove it
							return;
						}
					}

				}

				COleCurrency cyDiscount = VarCurrency(pRow->GetValue(dlcDiscount), COleCurrency(0,0));
				if (cyDiscount > COleCurrency(0,0)) {
					// (j.gruber 2009-03-31 15:35) - PLID 33554 - check permissions
					if (m_bIsBill) {
						if (!CheckCurrentUserPermissions(bioChargeAmountOff, sptWrite)) {
							return;
						}
					}
					else {
						if (!CheckCurrentUserPermissions(bioQuoteAmountOff, sptWrite)) {
							return;
						}
					}

				}
			}

			m_pDiscountDataList->RemoveRow(pRow);
		}
		else {
			MsgBox("Please select a discount to delete");
		}
	}NxCatchAll("Error in CChargeDiscountDlg::OnBnClickedDeleteDiscount()");
}
BEGIN_EVENTSINK_MAP(CChargeDiscountDlg, CNxDialog)
	ON_EVENT(CChargeDiscountDlg, IDC_CHARGE_DISCOUNT_LIST, 19, CChargeDiscountDlg::LeftClickChargeDiscountList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CChargeDiscountDlg, IDC_CHARGE_DISCOUNT_LIST, 8, CChargeDiscountDlg::EditingStartingChargeDiscountList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)	
	ON_EVENT(CChargeDiscountDlg, IDC_CHARGE_DISCOUNT_LIST, 9, CChargeDiscountDlg::EditingFinishingChargeDiscountList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CChargeDiscountDlg::ChangePercentDiscountFieldsFromCoupon(NXDATALIST2Lib::IRowSettingsPtr pRow, long nCouponID) {

	try {
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT DiscountType, PercentOff, DiscountAmount FROM CouponsT WHERE ID = %li", nCouponID);
		if (! rs->eof) {
			long nDiscountType = AdoFldLong(rs, "DiscountType");
			if (nDiscountType == 1) {
				//its a percent
				long nPercent = AdoFldLong(rs, "PercentOff", 0);

				//check to see if they already have a discount
				long nExistingPercentOff = VarLong(pRow->GetValue(dlcPercentOff), 0);
				if (nExistingPercentOff > 0) {
					CString strMessage;
					strMessage.Format("This coupon has a percentage discount of %li%%, \n"
						"however this charge already has a percentage discount on it of %li%%.\n"
						"Would you like to overwrite the existing percentage discount with the coupon percentage discount?", nPercent, nExistingPercentOff);
					if (IDYES == MessageBox(strMessage,"NexTech Practice", MB_YESNO)) {
						pRow->PutValue(dlcPercentOff, (long)nPercent);						
					}	
				}
				else {
					//must not be a discount yet
					pRow->PutValue(dlcPercentOff, (long)nPercent);
				}
			}
			else {
				//its a dollar discount
				//check to see if they already have a dollar discount
				COleCurrency cyDollar = AdoFldCurrency(rs, "DiscountAmount", COleCurrency(0,0));
				COleCurrency cyExistingDollarDiscount = VarCurrency(pRow->GetValue(dlcDiscount), COleCurrency(0,0));
				if (cyExistingDollarDiscount > COleCurrency(0,0)) {
					//they already have a dollar disount
					CString strMessage;
					strMessage.Format("This coupon has a dollar discount of %s, \n"
						"however this charge already has a dollar discount on it of %s.\n"
						"Would you like to overwrite the existing dollar discount with the coupon dollar discount?", 
						FormatCurrencyForInterface(cyDollar, TRUE, TRUE), 
						FormatCurrencyForInterface(cyExistingDollarDiscount, TRUE, TRUE));
					if (IDYES == MessageBox(strMessage,"NexTech Practice", MB_YESNO)) {
						pRow->PutValue(dlcDiscount, _variant_t(cyDollar));
					}
				}
				else {
					//just add the dollar discount on since there is no percent
					pRow->PutValue(dlcDiscount, _variant_t(cyDollar));
				}			
			}
		}
	}NxCatchAll("Error in CChargeDiscountDlg::ChangePercentDiscountFieldsFromCoupon");

}


void CChargeDiscountDlg::LeftClickChargeDiscountList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			switch(nCol) {

				case dlcDiscountCategoryLink:
					{
						// (j.jones 2011-01-21 14:35) - PLID 42156 - this is protected data, always call 
						// CanChangeHistoricFinancial if this is an existing charge
						if(m_bIsBill && m_nChargeID != -2
							&& !CanChangeHistoricFinancial("Charge", m_nChargeID, bioBill, sptWrite)) {
							return;
						}

						long nDiscountCatID = VarLong(pRow->GetValue(dlcDiscountCategoryID), -3);
						CString strCustomCat;
						if (nDiscountCatID == -1) {						
							strCustomCat = VarString(pRow->GetValue(dlcCustomDiscountDesc), "");						
						}

						long nCouponID = -1;
						if (m_bUseCoupons) {
							nCouponID = VarLong(pRow->GetValue(dlcCouponID), -1);
						}
						
						//open the discount category select dialog
						CDiscountCategorySelectDlg dlg(this, nDiscountCatID, strCustomCat, nCouponID, m_bUseCoupons);
						long nResult = dlg.DoModal();
						if (nResult == IDOK) {
							//see if anything changed
							long nReturnedCouponID = dlg.m_nCouponID;
							long nReturnedDiscountCategoryID = dlg.m_nDiscountCatID;
							CString strReturnedDiscCatDesc = dlg.m_strCustomDescription;

							if (nReturnedDiscountCategoryID == -1) {
								//its a custom description
								pRow->PutValue(dlcDiscountCategoryID, (long)-1);
								pRow->PutValue(dlcCouponID, (long)-1);
								pRow->PutValue(dlcCustomDiscountDesc, _variant_t(strReturnedDiscCatDesc));
								pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<Custom> - " + strReturnedDiscCatDesc));
							}
							else if (nReturnedDiscountCategoryID == -2) {
								//its a coupon
								if (nReturnedCouponID > 0) {
									pRow->PutValue(dlcDiscountCategoryID, (long)-2);
									pRow->PutValue(dlcCouponID, nReturnedCouponID);
									pRow->PutValue(dlcCustomDiscountDesc, _variant_t(""));
									pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<Coupon> - " + GetCouponName(nReturnedCouponID)));

									//we need to change the percent off or discount according to the coupon
									ChangePercentDiscountFieldsFromCoupon(pRow, nReturnedCouponID);								
								}
								else {
									//they didn't select a specific coupon
									pRow->PutValue(dlcDiscountCategoryID, (long)-2);
									pRow->PutValue(dlcCouponID, (long)-1);
									pRow->PutValue(dlcCustomDiscountDesc, _variant_t(""));
									pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<Coupon> - <Unspecified>"));
								}

							}
							else if (nReturnedDiscountCategoryID == -3) {
								//they chose no category
								pRow->PutValue(dlcDiscountCategoryID, (long) -3);
								pRow->PutValue(dlcCouponID, (long)-1);
								pRow->PutValue(dlcCustomDiscountDesc, _variant_t(""));
								pRow->PutValue(dlcDiscountCategoryLink, _variant_t("<No Category>"));
							}
							else {
								//they picked something
								pRow->PutValue(dlcDiscountCategoryID, nReturnedDiscountCategoryID);
								pRow->PutValue(dlcCouponID, (long)-1);
								pRow->PutValue(dlcCustomDiscountDesc, _variant_t(""));
								pRow->PutValue(dlcDiscountCategoryLink, _variant_t(GetDiscountCategoryName(nReturnedDiscountCategoryID)));
							}
						}
					}
			}
		}

	}NxCatchAll("Error in CChargeDiscountDlg::LeftClickChargeDiscountList");
}


BOOL CChargeDiscountDlg::ValidateDiscounts() {
	//this will be done for 33246, just putting in the structure for it now

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountDataList->GetFirstRow();
		long nTotalPercentOff = 0;
		COleCurrency cyTotalDiscounts;

		while (pRow) {

			//check the percent
			long nPercentOff = VarLong(pRow->GetValue(dlcPercentOff), 0);
			COleCurrency cyDiscount = VarCurrency(pRow->GetValue(dlcDiscount), COleCurrency(0,0));
			long nDiscountCategoryID = VarLong(pRow->GetValue(dlcDiscountCategoryID), -3);
			
			
			if (nPercentOff > 100) {
				MsgBox("At least one of the percent discounts is greater then 100, please enter a valid percent discount.");
				return FALSE;
			}

			if (nPercentOff < 0) {
				MsgBox("At least one of the percent discounts is less then 0, please enter a valid percent discount.");
				return FALSE;
			}

			if(cyDiscount > COleCurrency(100000000,0)) {				
				CString str;
				MsgBox("At least one of the discounts is greater than %s, please enter a valid discount.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));			
				return FALSE;
			}

			//also check for negatives
			if (cyDiscount < COleCurrency(0,0)) {
				MsgBox("Please enter a valid discount");
				return FALSE;
			}

			//check to make sure something is filled out
			if (nPercentOff == 0 && cyDiscount == COleCurrency(0,0) && nDiscountCategoryID == -3) {
				MsgBox("At least one of the discounts contains no information, please either delete this discount or add information to the discount line.");
				return FALSE;
			}

			nTotalPercentOff  += nPercentOff;
			cyTotalDiscounts += cyDiscount;

			pRow = pRow->GetNextRow();
		}


		//run through the datalist and make sure that these discounts won't make a negative charge
		//time to calculate the discount total
		COleCurrency cyLineTotal = CalculateLineTotal(nTotalPercentOff, cyTotalDiscounts);
		
		if (cyLineTotal < COleCurrency(0,0)) {

			MsgBox("The discount(s) entered calculates to a negative charge. Please fix this before continuing.");
			return FALSE;
		}

	
		return TRUE;
	}NxCatchAll("Error in CChargeDiscountDlg::ValidateDiscounts()");

	return FALSE;
}

// (j.gruber 2009-03-19 15:56) - PLID 33246 - pretty much copied from CalculateLineTotal in BillingDlg
COleCurrency CChargeDiscountDlg::CalculateLineTotal(long nTotalPercentOff, COleCurrency cyTotalDiscount) {

	try {
		COleCurrency cyLineTotal = m_cyUnitCost + m_cyOtherCost;	

		cyLineTotal = CalculateAmtQuantity(cyLineTotal,m_dblQuantity);

		//highly unlikely anyone would use modifiers and discounts together,
		//but if so, modify first
		if (m_dblMultiplier1 != -1) {
			cyLineTotal = CalculateAmtQuantity(cyLineTotal,m_dblMultiplier1);
		}
		if (m_dblMultiplier2 != -1) {
			cyLineTotal = CalculateAmtQuantity(cyLineTotal,m_dblMultiplier2);
		}
		if (m_dblMultiplier3 != -1) {
			cyLineTotal = CalculateAmtQuantity(cyLineTotal,m_dblMultiplier3);
		}
		if (m_dblMultiplier4 != -1) {
			cyLineTotal = CalculateAmtQuantity(cyLineTotal,m_dblMultiplier4);
		}

		cyLineTotal = (cyLineTotal * (100000 - nTotalPercentOff * 1000));
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - Operator / is ambiguous
		cyLineTotal = cyLineTotal / long(100000);
		cyLineTotal -= cyTotalDiscount;
		
		//Equivalent to cy = cy + (cy * (double)(dblTax / 100.0));
		COleCurrency taxResult1, taxResult2;

		if (m_dblTax1 != -1) {
			taxResult1 = CalculateTax(cyLineTotal,m_dblTax1);
		}

		if (m_dblTax2 != -1) {
			taxResult2 = CalculateTax(cyLineTotal,m_dblTax2);
		}

		if (m_dblTax1 != -1 && m_dblTax2 != -1) {
			cyLineTotal += taxResult1;
			cyLineTotal += taxResult2;
		}

		RoundCurrency(cyLineTotal);

		return cyLineTotal;
	}NxCatchAll("Error in CChargeDiscountDlg::CalculateLineTotal");

	return COleCurrency(0,0);

}

void CChargeDiscountDlg::OnBnClickedOk()
{

	try {
		if (ValidateDiscounts() ) {

			//clear the existing array and rebuild it from the datalist
			m_pDiscountList->aryDiscounts.RemoveAll();

			//now rebuild it
			NXDATALIST2Lib::IRowSettingsPtr pRow;

			pRow = m_pDiscountDataList->GetFirstRow();

			while (pRow) {

				stDiscount Disc;
				Disc.PercentOff = pRow->GetValue(dlcPercentOff);
				Disc.Discount = pRow->GetValue(dlcDiscount);		
				Disc.CouponID = pRow->GetValue(dlcCouponID);
				long nDiscCatID = VarLong(pRow->GetValue(dlcDiscountCategoryID), -3);
				if (nDiscCatID == 0) {
					Disc.DiscountCategoryID = (long)-3;
				}
				else {
					Disc.DiscountCategoryID = pRow->GetValue(dlcDiscountCategoryID);
				}
				Disc.CustomDiscountDescription = pRow->GetValue(dlcCustomDiscountDesc);
				Disc.ID = (long)NEW_DISCOUNT_ID - (m_pDiscountList->aryDiscounts.GetCount());
				//this can always be false here because this field is only for the initial loading
				// (z.manning 2011-03-28 16:52) - PLID 41423 - Removed the typecast here as the varible is now of the enum type.
				Disc.DiscountPreference = dpIgnorePreference;

				m_pDiscountList->aryDiscounts.Add(Disc);

				pRow = pRow->GetNextRow();
			}

			OnOK();
		}
	}NxCatchAll("Error in CChargeDiscountDlg::OnBnClickedOk()");
}


long CChargeDiscountDlg::GetTotalPercent()
{	
	try {
		//now rebuild it
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pDiscountDataList->GetFirstRow();
		long nTotalPercent = 0;

		while (pRow) {

			nTotalPercent += VarLong(pRow->GetValue(dlcPercentOff), 0);

			pRow = pRow->GetNextRow();
		}
		return nTotalPercent;
	}NxCatchAll("Error in CChargeDiscountDlg::GetTotalPercent()");
	return -1;
}

COleCurrency CChargeDiscountDlg::GetTotalDiscount()
{	
	try {
		//now rebuild it
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		pRow = m_pDiscountDataList->GetFirstRow();
		COleCurrency cyTotalDiscount;

		while (pRow) {

			cyTotalDiscount += VarCurrency(pRow->GetValue(dlcDiscount), COleCurrency(0,0));

			pRow = pRow->GetNextRow();
		}

		return cyTotalDiscount;
	}NxCatchAll("Error in CChargeDiscountDlg::GetTotalDiscount()"); 
	return COleCurrency(0,0);
}

void CChargeDiscountDlg::OnBnClickedCancel()
{
	//everything stays the same, so just cancel
	OnCancel();
}

void CChargeDiscountDlg::EditingStartingChargeDiscountList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		if (m_bCheckPermissions) {

			// (j.jones 2011-01-21 14:35) - PLID 42156 - this is protected data, always call 
			// CanChangeHistoricFinancial if this is an existing charge
			if(m_bIsBill && m_nChargeID != -2
				&& !CanChangeHistoricFinancial("Charge", m_nChargeID, bioBill, sptWrite)) {
				*pbContinue = FALSE;
				m_pDiscountDataList->StopEditing(FALSE);
				return;
			}

			switch (nCol) {
				case dlcPercentOff:
					// (j.gruber 2009-03-31 15:35) - PLID 33554 - check permissions
					if (m_bIsBill) {
						if (!CheckCurrentUserPermissions(bioChargePercentOff, sptWrite)) {
							*pbContinue = FALSE;
							m_pDiscountDataList->StopEditing(FALSE);
							return;
						}		
					}
					else {
						if (!CheckCurrentUserPermissions(bioQuotePercentOff, sptWrite)) {
							*pbContinue = FALSE;
							m_pDiscountDataList->StopEditing(FALSE);
							return;
						}	
					}


				break;

				case dlcDiscount:
					// (j.gruber 2009-03-31 15:35) - PLID 33554 - check permissions
					if (m_bIsBill) {
						if (!CheckCurrentUserPermissions(bioChargeAmountOff, sptWrite)) {
							*pbContinue = FALSE;
							m_pDiscountDataList->StopEditing(FALSE);				
							return;
						}
					}
					else {
						if (!CheckCurrentUserPermissions(bioQuoteAmountOff, sptWrite)) {
							*pbContinue = FALSE;
							m_pDiscountDataList->StopEditing(FALSE);				
							return;
						}
					}			

				break;
			}
		}
	}NxCatchAll("Error in CChargeDiscountDlg::EditingStartingChargeDiscountList");
}

void CChargeDiscountDlg::EditingFinishingChargeDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			if (nCol == dlcPercentOff) {

				//check to make sure its valid
				long nPercent = VarLong(*pvarNewValue, 0);
				if (nPercent > 100 || nPercent < 0) {
					MsgBox("Please enter a valid percentage discount.");
					*pvarNewValue = varOldValue;
					*pbCommit = false;
				}
			}

			if (nCol == dlcDiscount) {

				//make sure its valid
				//we don't allow a unit cost higher then 100 million, so we will limit to that here also
				COleCurrency cyDiscount = VarCurrency(*pvarNewValue, COleCurrency(0,0));
				if(cyDiscount > COleCurrency(100000000,0)) {				
					CString str;
					str.Format("You cannot have any charges greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
					MsgBox(str);
					*pvarNewValue = varOldValue;
					*pbCommit = false;
				}

				//also check for negatives
				if (cyDiscount < COleCurrency(0,0)) {
					MsgBox("Please enter a valid discount");
					*pvarNewValue = varOldValue;
					*pbCommit = false;
				}
			}
		}


	}NxCatchAll("Error in CChargeDiscountDlg::EditingFinishingChargeDiscountList");
	
}

// (j.jones 2009-04-07 09:43) - PLID 33474 - supported barcode scanning
LRESULT CChargeDiscountDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
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

				AddNewDiscount(nID, strCouponName);
			}
		}
	} NxCatchAll("Error in CChargeDiscountDlg::OnBarcodeScan");

	m_bScanning = false;

	return 0;
}