//billingDlgDiscounts.cpp

// (j.gruber 2009-03-10 11:10) - PLID 33351 - all functions in this file created for that unless otherwise noted

#include "stdafx.h"
#include "BillingModuledlg.h"
#include "BillingDlg.h"
#include "InternationalUtils.h"
#include "globalFinancialUtils.h"
#include "ChargeDiscountDlg.h"
#include "AuditTrail.h"
#include "AdvDiscountDlg.h"
#include "DiscountBillDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2014-02-24 11:27) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems.


using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (j.gruber 2009-03-05 15:47) - PLID 33351 - setup the discount list
// (j.dinatale 2012-04-10 16:34) - PLID 49519 - need to know if we are loading discounts from a glasses/contacts order
void CBillingDlg::LoadDiscountList(BOOL bIsNew, long nChargeID, long nServiceID, DiscountList *pDiscountList, 
								   COleCurrency cyPointCost, COleCurrency cyRewardDiscountDollars, long nRewardDiscountPercent,
								   BOOL &bUsingPoints, COleCurrency &cyPointsUsed, BOOL bMarkExistingDiscounts /*=FALSE*/,
								   BOOL bLoadFromSurgeries /*=FALSE*/, BOOL bLoadFromGlassesOrder /*=FALSE*/) {
	
	// (j.jones 2008-05-19 12:15) - PLID 28359 - defaulted to on
	// (j.gruber 2009-03-06 08:39) - the KeepDiscounts setting is a little odd, but basically if it is true,
	//then 
	
	try {

		ASSERT(pDiscountList != NULL);
		BOOL bKeepDiscounts = (GetRemotePropertyInt("KeepQuoteDiscounts",1,0,"<None>",TRUE) == 1);

		_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		_variant_t varNull;
		varNull.vt = VT_NULL;

		BOOL bIgnoreDiscountPreference = FALSE;

		if (! bMarkExistingDiscounts) {
			//set it to be false
			bIgnoreDiscountPreference = TRUE;
		}

		//load the discounts
		stDiscount disc;	
		_RecordsetPtr rsDiscounts;
		if (bLoadFromSurgeries) {
			//the chargeID field is filled with the surgeryID
			rsDiscounts = CreateParamRecordset("SELECT ID, PercentOff, Discount, DiscountCategoryID, CustomDiscountDesc FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = {INT} ", nChargeID);		
		}
		else {
			// (j.dinatale 2012-04-10 16:34) - PLID 49519 - need to check for glasses order discounts
			// (s.dhole 2012-05-24 09:50) - PLID 43785 Remove DELETED Column
			if(bLoadFromGlassesOrder){
				//s.dhole  We drop DELETED  from this table
				rsDiscounts = CreateParamRecordset("SELECT ID, PercentOff, Discount, CouponID, DiscountCategoryID, CustomDiscountDesc FROM GlassesOrderServiceDiscountsT WHERE GlassesOrderServiceID = {INT} ", nChargeID);
			}else{
				if (nChargeID != -1) {
					rsDiscounts = CreateParamRecordset("SELECT ID, PercentOff, Discount, CouponID, DiscountCategoryID, CustomDiscountDesc FROM ChargeDiscountsT WHERE ChargeID = {INT} AND DELETED = 0", nChargeID);		
				}
			} 
		}

		
		if (nChargeID != -1) {
			_variant_t var;
			while (! rsDiscounts->eof) {
				
				if (!bIsNew) {
					var = rsDiscounts->Fields->Item["ID"]->Value;
					if (var.vt != VT_NULL) {
						disc.ID = var.lVal;
					}
				}
				else {
					disc.ID = varNull;
				}


				//for a quote, only keep it if they have the preference set to retain it
				//if(!bIsNew || bLoadFromSurgeries) {
					var = rsDiscounts->Fields->Item["PercentOff"]->Value;
					if (var.vt != VT_NULL)
						disc.PercentOff = var.lVal;
				//}		

				//if(!bIsNew || bLoadFromSurgeries) {
					var = rsDiscounts->Fields->Item["Discount"]->Value;
					if (var.vt != VT_NULL)
						disc.Discount  = var.cyVal;
				//}

				disc.DiscountCategoryID = rsDiscounts->Fields->Item["DiscountCategoryID"]->Value;

				var = rsDiscounts->Fields->Item["CustomDiscountDesc"]->Value;
				if (var.vt == VT_NULL || var.vt == VT_EMPTY) {
					disc.CustomDiscountDescription = "";
				}
				else {
					disc.CustomDiscountDescription = CString(var.bstrVal);
				}

				// (j.gruber 2007-04-03 17:28) - PLID 9796 - adding coupons
				//only do this if its not a surgery
				if (!bLoadFromSurgeries) {
					disc.CouponID = rsDiscounts->Fields->Item["CouponID"]->Value;
				}
			
				if (bKeepDiscounts) {
					disc.DiscountPreference = dpKeepDiscountSeparate;
				}
				else {
					if (bIgnoreDiscountPreference) {
						disc.DiscountPreference = dpIgnorePreference;
					}
					else {
						disc.DiscountPreference = dpIncludeDiscountInTotal;
					}
				}

				AddToDiscountList(pDiscountList, disc.ID, disc.PercentOff, disc.Discount, disc.CustomDiscountDescription, disc.DiscountCategoryID, disc.CouponID, disc.DiscountPreference);

				rsDiscounts->MoveNext();
			}
						
		}

		//only if we are billing a quote (not a package)
		if(bIsNew && !m_bPaymentPlan) {
			long nNewPercentOff = 0;
			CString strCode = "";
			//check procedure discounts
			// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
			if(CheckProcedureDiscount(nServiceID, m_nPatientID, nNewPercentOff, (((CBillingModuleDlg*)m_pBillingModuleWnd)->m_date.GetValue()), GetBillID()) && nNewPercentOff > 0) {
				CString str;
				_RecordsetPtr rsCode = CreateRecordset("SELECT CASE WHEN Code Is Not Null THEN Code + ' - ' + Name ELSE Name END AS Description FROM ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = %li", nServiceID);
				if(!rsCode->eof) {
					strCode = AdoFldString(rsCode, "Description","");
				}
				rsCode->Close();
				
				str.Format("This patient has earned a discount of %li%% off the procedure:\n%s.\n\n"
					"Would you like to apply this discount now?",nNewPercentOff, strCode);			

				if(IDYES == MessageBox(str,"Practice",MB_YESNO|MB_ICONQUESTION)) {
					//add a new discount				
					AddToDiscountList(pDiscountList, varNull, nNewPercentOff, varNull, varNull, varNull, varNull, dpKeepDiscountSeparate);

				}
			}

			// (a.walling 2007-05-30 10:32) - PLID 26172
			BOOL bUseRewardDiscount = TRUE;
			COleCurrency cyPoints;
			cyPoints.SetCurrency(0, 0);		
			CString strRewardDiscountText;
			if (nRewardDiscountPercent != 0) {
				strRewardDiscountText.Format("%li%%", nRewardDiscountPercent);
			} else if (cyRewardDiscountDollars != COleCurrency(0, 0)) {
				strRewardDiscountText = FormatCurrencyForInterface(cyRewardDiscountDollars);
			} else {
				// both are zero, no discounts set.
				bUseRewardDiscount = FALSE;
			}

			// (a.wetta 2007-05-08 10:41) - PLID 25959 - Check to see if the item is on sale
			double dblPercentDiscount = 0;
			COleCurrency cyMoneyDiscount = COleCurrency(0,0), cyNewDiscount = COleCurrency(0,0);
			CString strSaleName = "";
			long nDiscountCategoryID = -1;
			BOOL bUsingDiscount = FALSE;

			// (a.wetta 2007-05-17 09:30) - PLID 25960 - Make sure they have the NexSpa license also
			if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
				if (GetServiceItemSaleDiscount(nServiceID, dblPercentDiscount, cyMoneyDiscount, strSaleName, nDiscountCategoryID)) {
					// Make sure we have the name of the service item
					if (strCode == "") {
						_RecordsetPtr rsCode = CreateRecordset("SELECT CASE WHEN Code Is Not Null THEN Code + ' - ' + Name ELSE Name END AS Description FROM ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = %li", nServiceID);
						if(!rsCode->eof) {
							strCode = AdoFldString(rsCode, "Description","");
						}
					}
					CString strMsg;

					// (j.gruber 2009-12-31 13:45) - PLID 36480 - -1 is special for discount categories, but its just null for sales
					_variant_t varDiscountCategory;
					if (nDiscountCategoryID == -1) {
						varDiscountCategory = varNull;
					}
					else {
						varDiscountCategory = (long)nDiscountCategoryID;
					}

					// Handle a percent off sale
					if (dblPercentDiscount > 0) {
						strMsg.Format("This item \'%s\' is on sale for a %.0f%% discount as part of the \'%s\' sale. Would you like to apply the sale discount?", 
							strCode,dblPercentDiscount, strSaleName);
						

						if (IDYES == MessageBox(strMsg, "Practice", MB_YESNO|MB_ICONQUESTION)) {
							//set it to false because we don't want to calculate these in since they didn't come from the quote
							// (j.gruber 2009-12-31 13:37) - PLID 36480 - send in the discount category
							AddToDiscountList(pDiscountList, varNull, (long)dblPercentDiscount, varNull, varNull, varDiscountCategory, varNull, dpKeepDiscountSeparate);
							bUsingDiscount = TRUE;
						}
					}
					else {
						// This must be a dollar discount sale
						// (j.gruber 2009-03-05 12:21) - PLID 33351 - just add it
						strMsg.Format("This item \'%s\' is on sale for a %s discount as part of the \'%s\' sale. Would you like to apply the sale discount?", 
							strCode, FormatCurrencyForInterface(cyMoneyDiscount, TRUE, TRUE), strSaleName);
						
						if (IDYES == MessageBox(strMsg, "Practice", MB_YESNO|MB_ICONQUESTION)) {						
							//set it to false because we don't want to calculate these in since they didn't come from the quote
							// (j.gruber 2009-12-31 13:38) - PLID 36480 - send in the discount category
							AddToDiscountList(pDiscountList, varNull, varNull, _variant_t(cyMoneyDiscount), varNull, varDiscountCategory, varNull, dpKeepDiscountSeparate);
							bUsingDiscount = TRUE;
						}
					}
				}
				
				if (bUseRewardDiscount && m_EntryType == 1) {
					// (a.walling 2007-05-30 10:59) - PLID 26172
					COleCurrency cyAdjustedPoints = ((CBillingModuleDlg*)m_pBillingModuleWnd)->GetAdjustedRewardPoints();

					// (a.walling 2008-06-16 16:34) - PLID 30407 - Should be <=, not <
					if (cyPointCost != COleCurrency(0, 0) && cyPointCost <= cyAdjustedPoints) {
						// Make sure we have the name of the service item
						if (strCode == "") {
							_RecordsetPtr rsCode = CreateRecordset("SELECT CASE WHEN Code Is Not Null THEN Code + ' - ' + Name ELSE Name END AS Description FROM ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE ServiceT.ID = %li", nServiceID);
							if(!rsCode->eof) {
								strCode = AdoFldString(rsCode, "Description","");
							}
						}

						CString strMsg, strDiscount, strDollarDiscount, strPercentDiscount;
						/*COleCurrency* pcyActiveDiscount = NULL;
						long* pnActivePercent = NULL;

						if (bUsingDiscount) {
							// already a discount applied, probably from sales
							// only percent or dollar will be set.
							if (nNewPercentOff > 0) {
								pnActivePercent = &nNewPercentOff;
								pcyActiveDiscount = &cyDiscount;
							} else if (cyNewDiscount > COleCurrency(0, 0)) {
								pcyActiveDiscount = &cyNewDiscount;
								pnActivePercent = &nNewPercentOff;
							}
						} else {
							// discounts that already exist on the quote or surgery
							pnActivePercent = &nNewPercentOff;
							pcyActiveDiscount = &cyDiscount;
						}

						if (pcyActiveDiscount != NULL && *pcyActiveDiscount > COleCurrency(0, 0)) {
							strDollarDiscount = FormatCurrencyForInterface(*pcyActiveDiscount);
						}

						if (pnActivePercent != NULL && *pnActivePercent > 0) {
							strPercentDiscount.Format("%li%%", *pnActivePercent);
						}

						if (nRewardDiscountPercent > 0 && pnActivePercent != NULL && *pnActivePercent > 0) {
							strDiscount = strPercentDiscount;
						} else if (cyRewardDiscountDollars > COleCurrency(0, 0) && *pcyActiveDiscount > COleCurrency(0, 0)) {
							strDiscount = strDollarDiscount;
						}

						if (nNewPercentOff > 0) {
							// This item already has a discount prompt the user about what should be done
							strMsg.Format("The item \'%s\' is redeemable for %s of this patient\'s reward points, but already has a %s discount on it. Would you like to exchange these points for a %s discount on this product? If you choose YES, the previous discount will be cleared.",
								strCode, FormatCurrencyForInterface(cyPointCost, FALSE), strDiscount, strRewardDiscountText);
						}
						else if (iPercentOff > 0) {
							strMsg.Format("The item \'%s\' is redeemable for %s of this patient\'s reward points. However, the quote you are loading already has a %s discount on it. Would you like to exchange these points for a %s discount on this product? If you choose YES, the previous discount will be cleared.",
								strCode, FormatCurrencyForInterface(cyPointCost, FALSE), strDiscount, strRewardDiscountText);
						}
						else {*/
							strMsg.Format("The item \'%s\' is redeemable for %s of this patient\'s reward points. Would you like to exchange these points for a %s discount on this product?",
								strCode, FormatCurrencyForInterface(cyPointCost, FALSE), strRewardDiscountText);
						//}
						
						if (IDYES == MessageBox(strMsg, "Practice", MB_YESNO | MB_ICONQUESTION)) {
							// if this is -1, we will be prompted to select one
							// for now, we'll always prompt to select a discount category.
							// if we set this up elsewhere, we can grab it here
							// nDiscountCategoryID = VarLong(m_GiftCombo->GetValue(nRow, PRODUCTS_COLUMN_DISCOUNTCATEGORYID), -1);
							nDiscountCategoryID = -1;
							
							((CBillingModuleDlg*)m_pBillingModuleWnd)->AddAdjustedRewardPoints(cyPointCost);

							bUsingPoints = TRUE;
							cyPointsUsed = cyPointCost;						
							//set it to false because we don't want to calculate these in since they didn't come from the quote						
							AddToDiscountList(pDiscountList, varNull, nRewardDiscountPercent, _variant_t(cyRewardDiscountDollars), varNull, varNull, varNull, dpKeepDiscountSeparate);
							/*bUsingDiscount = TRUE;
							nNewPercentOff = nRewardDiscountPercent;
							cyNewDiscount = cyRewardDiscountDollars;*/
						}
					}			
				}
			}

			//if we kept the new one, replace the one from the quote
		/*	BOOL bDiscountChanged = FALSE;
			if(nNewPercentOff > 0) {
				iPercentOff = nNewPercentOff;
				pNew->PercentOff = _variant_t(iPercentOff);
				bDiscountChanged = TRUE;
			}
			if(cyNewDiscount > COleCurrency(0,0)) {
				cyDiscount = cyNewDiscount;
				pNew->Discount = _variant_t(cyDiscount);
				bDiscountChanged = TRUE;
			}
			if(bDiscountChanged) {
				if (bUsingDiscount && nDiscountCategoryID != -1) {
					// (a.wetta 2007-05-08 14:41) - PLID 25959 - Use the pre-set discount category for the sale
					pNew->DiscountCategoryID = (long)nDiscountCategoryID;
					pNew->CustomDiscountDescription = "";
					pNew->HasDiscountCategory.boolVal = TRUE;
					pNew->CouponID.vt = VT_NULL;
				}
				else {
					// (j.gruber 2007-03-28 12:39) - PLID 25352 - check to see if they want to add a category
					long nSelection = GetRemotePropertyInt("BillShowDiscountCatScreenWhenAddDiscount", 2, 0, "<None>", true);
					if (nSelection == 1) {
						//always show it
						// (a.wetta 2007-05-23 09:25) - PLID 26104 - Don't show coupons
						ShowDiscountCategoryList(pNew, TRUE, FALSE);
					}
					else if (nSelection == 2) {
						//prompt
						if (MsgBox(MB_YESNO, "Would you like to set a discount category?") == IDYES) {
							// (a.wetta 2007-05-23 09:25) - PLID 26104 - Don't show coupons
							ShowDiscountCategoryList(pNew, TRUE, FALSE);
						}
					}
				}
			}*/
		}

	}NxCatchAll("Error in CBillingDlg::LoadDiscountList");

}



void CBillingDlg::CalculateTotalDiscount(DiscountList *pDiscountList, COleCurrency cyCurrentPracticeTotal, COleCurrency cyCurrentOutsideTotal, long &nPercentOff, COleCurrency &cyTotalDollarDiscount, COleCurrency &cyTotalLineDiscount, KeepQuoteDiscountPref discountPreference) {

	try {
		nPercentOff = 0;
		cyTotalDollarDiscount = COleCurrency(0,0);

		//check the discount count list is not null
		ASSERT(pDiscountList);
		if (pDiscountList) {

			for (int i = 0; i < pDiscountList->aryDiscounts.GetCount(); i++) {
				stDiscount disc = pDiscountList->aryDiscounts[i];
				if ((discountPreference == dpIgnorePreference) || (discountPreference == disc.DiscountPreference)) {
					if (disc.Discount.vt == VT_CY) {
						cyTotalDollarDiscount += disc.Discount.cyVal;
					}

					if (disc.PercentOff.vt == VT_I4) {
						nPercentOff += disc.PercentOff.lVal;
					}
				}
			}
		}
		
		//calculate the total line discount	
		cyTotalLineDiscount = COleCurrency(0,0);
		COleCurrency cyTemp = COleCurrency(0,0);

		//first the practice total
		cyTemp = cyCurrentPracticeTotal;
		cyCurrentPracticeTotal = (cyCurrentPracticeTotal * (100000 - nPercentOff * 1000));
		cyCurrentPracticeTotal/= 100000;
		cyTotalLineDiscount = cyTemp - cyCurrentPracticeTotal;
		cyTotalLineDiscount += cyTotalDollarDiscount;

		//now the outside total if applicable
		if (cyCurrentOutsideTotal.GetStatus() != COleCurrency::invalid) {
			cyTemp = cyCurrentOutsideTotal;
			cyCurrentOutsideTotal = (cyCurrentOutsideTotal * (100000 - nPercentOff * 1000));
			cyCurrentOutsideTotal /= 100000;
			cyTotalLineDiscount = cyTemp - cyCurrentOutsideTotal;
			cyTotalLineDiscount += cyTotalDollarDiscount;
		}	
	}NxCatchAll("Error in CBillingDlg::CalculateTotalDiscount");
		
}

// (z.manning 2011-03-28 16:53) - PLID 41423 - Changed the type of eExistingDiscount to be an enum rather than
// a variant to prevent different types of variants from being used for it.
void CBillingDlg::AddToDiscountList(DiscountList *pDiscountList, _variant_t varDiscountID, _variant_t varPercentOff, _variant_t varDiscountAmt, _variant_t varCustomDiscountDesc, _variant_t varDiscountCategoryID, _variant_t varCouponID, KeepQuoteDiscountPref eExistingDiscount)
{
	try {
		if (pDiscountList) {

			_variant_t varNull;
			varNull.vt = VT_NULL;

			if (varDiscountID.vt == VT_NULL ) {

				//this is a completely new discount, so figure out the new ID
				varDiscountID = (long)NEW_DISCOUNT_ID - (pDiscountList->aryDiscounts.GetCount());

				stDiscount Disc;
				Disc.ID = varDiscountID.lVal;

				if (varCouponID.vt== VT_I4){
					Disc.CouponID = varCouponID.lVal;
				}
				else {
					Disc.CouponID = varNull;
				}			
				
				if (varCustomDiscountDesc.vt == VT_BSTR)  {
					Disc.CustomDiscountDescription = varCustomDiscountDesc.bstrVal;
				}
				else {
					Disc.CustomDiscountDescription = varNull;
				}		

				if (varDiscountAmt.vt == VT_CY) {
					Disc.Discount = varDiscountAmt.cyVal;
				}
				else {
					Disc.Discount = varNull;
				}

				if (varPercentOff.vt == VT_I4) {
					Disc.PercentOff = varPercentOff.lVal;
				}
				else {
					Disc.PercentOff = varNull;
				}


				if (varDiscountCategoryID.vt == VT_I4) {
					Disc.DiscountCategoryID = varDiscountCategoryID.lVal;
				}
				else {
					Disc.DiscountCategoryID = varNull;
				}

				Disc.DiscountPreference = eExistingDiscount;

				//add it to the list
				pDiscountList->aryDiscounts.Add(Disc);
			}
			else {
				//we could be editing, so try to get it out of the list
				BOOL bFound = FALSE;
				for (int i = 0; i< pDiscountList->aryDiscounts.GetCount(); i++) {
					stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);

					if (Disc.ID.lVal == varDiscountID.lVal) {
						//we found it, remove it from the list
						pDiscountList->aryDiscounts.RemoveAt(i);
						bFound = TRUE;

						//now fill it back in
						if (varCouponID.vt== VT_I4){
							Disc.CouponID = varCouponID.lVal;
						}
						else {
							Disc.CouponID = varNull;
						}

						if (varCustomDiscountDesc.vt == VT_BSTR)  {
							Disc.CustomDiscountDescription = varCustomDiscountDesc.bstrVal;
						}
						else {
							Disc.CouponID = varNull;
						}

						if (varDiscountAmt.vt == VT_CY) {
							Disc.Discount = varDiscountAmt.cyVal;
						}
						else {
							Disc.Discount = varNull;
						}

						if (varPercentOff.vt == VT_I4) {
							Disc.PercentOff = varPercentOff.lVal;
						}
						else {
							Disc.PercentOff = varNull;
						}

						if (varDiscountCategoryID.vt == VT_I4) {
							Disc.DiscountCategoryID = varDiscountCategoryID.lVal;
						}
						else {
							Disc.DiscountCategoryID = varNull;
						}

						Disc.DiscountPreference = eExistingDiscount;

						//add it back to the list
						pDiscountList->aryDiscounts.Add(Disc);
					}
				}

				if (! bFound) {
					//we didn't find it to update in the list, so add it
					stDiscount Disc;
					Disc.ID = varDiscountID.lVal;

					if (varCouponID.vt== VT_I4){
						Disc.CouponID = varCouponID.lVal;
					}
					else {
						Disc.CouponID = varNull;
					}			
					
					if (varCustomDiscountDesc.vt == VT_BSTR)  {
						Disc.CustomDiscountDescription = varCustomDiscountDesc.bstrVal;
					}
					else {
						Disc.CustomDiscountDescription = varNull;
					}		

					if (varDiscountAmt.vt == VT_CY) {
						Disc.Discount = varDiscountAmt.cyVal;
					}
					else {
						Disc.Discount = varNull;
					}

					if (varPercentOff.vt == VT_I4) {
						Disc.PercentOff = varPercentOff.lVal;
					}
					else {
						Disc.PercentOff = varNull;
					}


					if (varDiscountCategoryID.vt == VT_I4) {
						Disc.DiscountCategoryID = varDiscountCategoryID.lVal;
					}
					else {
						Disc.DiscountCategoryID = varNull;
					}

					Disc.DiscountPreference = eExistingDiscount;

					//add it to the list
					pDiscountList->aryDiscounts.Add(Disc);
				}

			}
		}
	}NxCatchAll("Error in CBillingDlg::AddToDiscountList");
}


void CBillingDlg::ShowDiscountDetailDialog(BillingItemPtr pItem, BOOL bIsBill) {

	try {
		COleCurrency cyCurrentLineTotal;
		if (pItem->LineTotal.vt == VT_CY) {
			cyCurrentLineTotal = pItem->LineTotal.cyVal;
		}
		else {
			ASSERT(FALSE);
		}

		COleCurrency cyUnitCost, cyOtherCost;
		double dblMultiplier1, dblMultiplier2, dblMultiplier3, dblMultiplier4, dblQuantity, dblTax1, dblTax2;

		_variant_t var;

		cyUnitCost = pItem->UnitCost.cyVal;
		
		var = pItem->OthrUnitCost;
		if(var.vt != VT_NULL && var.vt != VT_EMPTY)
			cyOtherCost = var.cyVal;
		else
			cyOtherCost = COleCurrency(0,0);

		dblQuantity = pItem->Quantity.dblVal;	

		var = pItem->Multiplier1;
		dblMultiplier1 = (var.vt == VT_EMPTY) ? 1.0 : VarDouble(var,1.0);

		var = pItem->Multiplier2;
		dblMultiplier2 = (var.vt == VT_EMPTY) ? 1.0 : VarDouble(var,1.0);

		var = pItem->Multiplier3;
		dblMultiplier3 = (var.vt == VT_EMPTY) ? 1.0 : VarDouble(var,1.0);

		var = pItem->Multiplier4;
		dblMultiplier4 = (var.vt == VT_EMPTY) ? 1.0 : VarDouble(var,1.0);

		var = pItem->TaxRate1;
		if(var.vt == VT_R8)
			dblTax1 = var.dblVal;
		else
			dblTax1 = 0.0;
		
		/* Convert the rate from 'Visible rate' to 'Scott rate' */
		dblTax1 /= 100.0;
		dblTax1 += 1.0;


		var = pItem->TaxRate2;
		if(var.vt == VT_R8)
			dblTax2 = var.dblVal;
		else
			dblTax2 = 0.0;

		/* Convert the rate from 'Visible rate' to 'Scott rate' */
		dblTax2 /= 100.0;
		dblTax2 += 1.0;

		long nExistingDiscountCount = pItem->DiscountList->aryDiscounts.GetCount();

		// (j.gruber 2009-04-22 10:45) - PLID 34042 - added charge description to dialog title
		CString strDescription;
		if (pItem->Description.vt == VT_BSTR) {
			strDescription = pItem->Description.bstrVal;
		}
		else {
			strDescription = "";
		}

		// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
		// (j.jones 2011-01-21 14:38) - PLID 42156 - this dialog now requires the charge ID
		CChargeDiscountDlg dlg(this, pItem->ChargeID, pItem->DiscountList, IsSpa(FALSE), bIsBill, TRUE, cyUnitCost, cyOtherCost, dblQuantity, strDescription,
			dblMultiplier1, dblMultiplier2, dblMultiplier3, dblMultiplier4, dblTax1, dblTax2);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {

			//everything is accounted for in the dialog, so just refresh the totals
			COleCurrency cyInvalid;
			cyInvalid.SetStatus(COleCurrency::invalid);
			m_varBoundItem = pItem->LineID;

			CalculateLineTotal(cyInvalid, cyInvalid, -1, -1, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, cyInvalid);

			if (pItem->ChargeID.vt == VT_I4 && pItem->ChargeID.lVal != -2) {
				AddToModifiedList(pItem->ChargeID.lVal);
			}

			// (j.gruber 2009-03-24 16:56) - PLID 33473 - see what their preference is for the rest of the charges
			//only do this if its they have no discount before
			if (nExistingDiscountCount == 0) {

				//make sure they added some discounts and there are other charges
				// (j.jones 2011-08-24 08:41) - PLID 44868 - we won't prompt if no other charges are writeable
				// (r.gonet 2016-02-06 16:46) - PLID 68193 - Make that we won't prompt if no other discountable
				// charges are writable.
				if (pItem->DiscountList->aryDiscounts.GetCount() > 0 && GetDiscountableChargeCount() > 1
					&& HasUncorrectedCharge(VarLong(pItem->LineID))) {
				
					long nSelection = GetRemotePropertyInt("BillApplyDiscountCategoryToCharges", 2, 0, "<None>", true);

					long nResult = 0;
					if (nSelection == 2) {
						//ask them if they want to continue
						if (pItem->DiscountList->aryDiscounts.GetCount() > 1) {
							nResult = MsgBox(MB_YESNO, "Would you like to add these discounts/categories to all other charges?");
						}
						else if (pItem->DiscountList->aryDiscounts.GetCount() == 1) {					
							nResult = MsgBox(MB_YESNO, "Would you like to add this discount/category to all other charges?");
						}
					}

					BOOL bOutputMessage = FALSE;
					CString strOutputDescription;
					if (nResult == IDYES || nSelection == 1) {

						// (r.gonet 2016-02-06 16:46) - PLID 68193 - Warn them that the discount
						// will not be applied to the gift certificate.
						if (GetGiftCertificateCount() > 0) {
							AfxMessageBox("Discounts will not be applied to Gift Certificates.");
						}

						//we are adding the discounts to all other charges, excluding this one
						for(int i=0;i<(int)m_billingItems.size();i++) {

							BillingItemPtr pItemToAdd = m_billingItems[i];

							long nChargeID = VarLong(pItemToAdd->ChargeID, -2);
							// (j.jones 2011-08-24 08:41) - PLID 44868 - if original/void, skip this charge
							// (r.gonet 2016-02-06 16:46) - PLID 68193 - Skip applying the discount to
							// gift certificate charges.
							if(IsOriginalOrVoidCharge(nChargeID) || pItemToAdd->GiftID.vt == VT_I4) {
								continue;
							}
						
							if(pItemToAdd->LineID.lVal != pItem->LineID.lVal) {

								for (int j = 0; j < pItem->DiscountList->aryDiscounts.GetCount(); j++) {
									stDiscount Disc = pItem->DiscountList->aryDiscounts.GetAt(j);

									_variant_t varNull;
									varNull.vt = VT_NULL;

									// (j.gruber 2009-04-03 17:31) - PLID 33246 - check to see that we can do that
									//don't check if its 0 already
									COleCurrency cyPracticeFee = pItemToAdd->UnitCost.cyVal;
									COleCurrency cyOtherFee = pItemToAdd->OthrUnitCost.cyVal;
									if (cyPracticeFee > COleCurrency(0,0) || cyOtherFee > COleCurrency(0,0)) {
										if (CanAddDiscount(pItemToAdd, FALSE, atapNotApplicable, Disc.PercentOff.lVal, Disc.Discount.cyVal)) {
											AddToDiscountList(pItemToAdd->DiscountList, varNull, Disc.PercentOff, Disc.Discount,
											Disc.CustomDiscountDescription, Disc.DiscountCategoryID, Disc.CouponID, dpIgnorePreference);
										}				
										else {
											bOutputMessage = TRUE;
											CString strTemp;
											strTemp.Format("%s with a total of %s\r\n", (CString)pItemToAdd->Description.bstrVal, FormatCurrencyForInterface(pItemToAdd->LineTotal.cyVal));
											strOutputDescription += "Charge: " + strTemp;
										}
									}
								}
							}
						}

						//now loop again and update the totals
						for(int i=0;i<(int)m_billingItems.size();i++) {
						
							BillingItemPtr pItemToAdd = m_billingItems[i];

							long nChargeID = VarLong(pItemToAdd->ChargeID, -2);
							// (j.jones 2011-08-24 08:41) - PLID 44868 - if original/void, skip this charge
							if(IsOriginalOrVoidCharge(nChargeID)) {
								continue;
							}
						
							if(pItemToAdd->LineID.lVal != pItem->LineID.lVal) {
								COleCurrency cyInvalid;
								cyInvalid.SetStatus(COleCurrency::invalid);
								m_varBoundItem = pItemToAdd->LineID;

								CalculateLineTotal(cyInvalid, cyInvalid, -1, -1, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, cyInvalid);
								AddToModifiedList(pItemToAdd->ChargeID.lVal);

							}
						}

						if (bOutputMessage) {
							MsgBox("The following charges did not get the discount applied as doing so would create negative charges \n%s", strOutputDescription);
						}					
					}
				}
			}


		}
	}NxCatchAll("Error in CBillingDlg::ShowDiscountDetailDialog");
}

// (j.jones 2011-10-04 11:40) - PLID 45799 - now takes in a DL2 row
void CBillingDlg::ShowDiscountDetailDialog(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bIsBill) {

	try {
		for(int i=0;i<(int)m_billingItems.size();i++) {

			BillingItemPtr bi = m_billingItems[i];

			if(VarLong(bi->LineID) == VarLong(pRow->GetValue(COLUMN_LINE_ID))) {

				if(VarBool(bi->IsOriginalCharge, FALSE)) {
					MessageBox("This charge has been voided, and is no longer editable. If a corrected charge exists, you can edit that charge in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
					return;
				} 
				else if ( bi->GiftID.vt == VT_I4) { 
					// (r.farnworth 2013-08-02 16:14) - PLID 45994 - No longer allow discounts to be used on Gift Certificates
					MessageBox("This charge is a Gift Certificate and cannot be discounted", "Practice", MB_ICONINFORMATION|MB_OK);
					return;
				}	
				else if(VarBool(bi->IsVoidingCharge, FALSE)) {
					MessageBox("This charge voids another charge, and is not editable. If a corrected charge exists, you can edit that charge in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
					return;
				}

				//call the billingitem function
				ShowDiscountDetailDialog(m_billingItems[i], bIsBill);
				return;
			}
		}
	}NxCatchAll("Error in CBillingDlg::ShowDiscountDetailDialog (long nRow, BOOL bIsBill)");
}

// (s.dhole 2012-04-05 15:34) - PLID 43785 removed CBillingDlg refrence
BOOL FindDiscountInArray(DiscountList *pDiscountList, long nDiscountID, stDiscount &Disc) {

	try {
		for (int i = 0; i < pDiscountList->aryDiscounts.GetSize(); i++) {

			stDiscount tempDisc = pDiscountList->aryDiscounts.GetAt(i);

			if (tempDisc.ID.lVal == nDiscountID) {
				Disc = tempDisc;
				return TRUE;
			}
		}

	}NxCatchAll("Error in BillingDlg::FindDiscountInArray");
	//if we got here, we couldn't find it
	return FALSE;
}
// (s.dhole 2012-04-05 15:34) - PLID 43785 removed CBillingDlg refrence

CString GetAuditDiscountDescription(stDiscount Disc) {

	try {
		CString strDescription;

		if (Disc.PercentOff.vt == VT_I4) {
			strDescription += "Percent Off: " + AsString(Disc.PercentOff) + ";";
		}

		if (Disc.Discount.vt == VT_CY) {
			strDescription += "Discount: " + AsString(Disc.Discount) + ";";
		}

		if (Disc.DiscountCategoryID.vt == VT_I4) {
			if (Disc.DiscountCategoryID.lVal == -1) {
				//custom discount
				strDescription += "Custom Category: " + AsString(Disc.CustomDiscountDescription) + ";";
			}
			else if (Disc.DiscountCategoryID.lVal == -2) {
				//coupon
				if (Disc.CouponID.vt == VT_I4) {
					if (Disc.CouponID.lVal <= 0) {
						strDescription += "Coupon;";
					}
					else {
						strDescription += "Coupon: " + GetCouponName(Disc.CouponID.lVal) + ";";
					}
				}
				else {
					ASSERT(FALSE);
				}
			}
			else if (Disc.DiscountCategoryID.lVal == -3) {
				strDescription += "Discount Category: <No Category>;";
			}
			else {
				strDescription += "Discount Category: " + GetDiscountCategoryName(Disc.DiscountCategoryID.lVal) + ";";
			}
		}

		return strDescription;
	}NxCatchAll("Error in CBillingDlg::GetAuditDiscountDescription");

	return "";
}
// (s.dhole 2012-04-05 15:34) - PLID 43785 removed CBillingDlg refrence
long GetPercentOffSaveValue(_variant_t varPercentOff) {
	
	long nPercentOff = 0;

	if (varPercentOff.vt != VT_NULL && varPercentOff.vt != VT_EMPTY) {
		nPercentOff = varPercentOff.lVal;
	}
	else { 
		nPercentOff = 0;
	}

	return nPercentOff;
}

// (s.dhole 2012-04-05 15:34) - PLID 43785 removed CBillingDlg refrence
CString GetDiscountSaveValue(_variant_t varDiscount) {

	CString strDiscount; 

	if (varDiscount.vt != VT_NULL && varDiscount.vt != VT_EMPTY) {
		if(varDiscount.vt == VT_I4) {//TES 6/13/03: I've seen this happen when you bill a package.
			strDiscount = FormatCurrencyForSql(COleCurrency(VarLong(varDiscount),0));
		}
		else {
			strDiscount = FormatCurrencyForSql(VarCurrency(varDiscount));
		}
	}
	else {
		strDiscount.Format("%s",FormatCurrencyForSql(COleCurrency(0,0)));
	}

	return strDiscount;
}
// (s.dhole 2012-04-05 15:34) - PLID 43785 removed CBillingDlg refrence
void GetDiscountCategorySaveValues(_variant_t varDiscountCategoryID, _variant_t varCouponID, _variant_t varCustomDiscountDesc,
												CString &strDiscountCategoryID, CString &strCouponID, CString &strCustomDiscountDescription) {

	
	if (varDiscountCategoryID.vt != VT_NULL && varDiscountCategoryID.vt != VT_EMPTY) {

		strDiscountCategoryID.Format("%li", varDiscountCategoryID.lVal);

		//check to see if they are using a custom description
		if (varDiscountCategoryID.lVal == -1) {
			
			strCouponID = " NULL ";
			//they are indeed			
			if (varCustomDiscountDesc.vt != VT_NULL && varCustomDiscountDesc.vt != VT_EMPTY) {
				strCustomDiscountDescription = CString(varCustomDiscountDesc.bstrVal);
			}
			else {
				strCustomDiscountDescription = "";
			}
		}
		else if (varDiscountCategoryID.lVal == -2) {
			//its a coupon			
			if (varCouponID.vt != VT_NULL && varCouponID.vt != VT_EMPTY) {
				if (varCouponID.lVal == -1) {
					strCouponID = " NULL ";
				}
				else {
					strCouponID.Format("%li", varCouponID.lVal);
				}
			}
			else {
				strCouponID = " NULL ";
			}
		}
		else if (varDiscountCategoryID.lVal == -3) {
			strCouponID = " NULL ";
			strDiscountCategoryID = " NULL ";
			strCustomDiscountDescription = "";
		}
		else {
			strCouponID = " NULL ";			
		}
	}
	else {
		strDiscountCategoryID = " NULL ";
		strCustomDiscountDescription = "";
		strCouponID = " NULL ";
	}
}


void CBillingDlg::AuditAndGenerateSaveStringForNewDiscounts(BOOL bNewCharge, long ChargeID, DiscountList *pDiscountList, CString &strSaveString) {
	
	CString strPatientName = GetBillPatientName();
	long nAuditID = -1;
	//now we have to deal with all the discounts they created
	for (int i = 0; i < pDiscountList->aryDiscounts.GetSize(); i++) {

		stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);
		if (Disc.ID.lVal < 0) {

			if (!bNewCharge) {
				//its a new one, so audit
				if (nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}

				CString strAuditDesc = GetAuditDiscountDescription(Disc);
				AuditEvent(m_nPatientID, strPatientName, nAuditID, m_EntryType == 1 ? aeiChargeDiscountAdded : aeiQuoteChargeDiscountAdded, ChargeID, "<Created>", strAuditDesc, aepMedium);
			}

			long nPercentOff = GetPercentOffSaveValue(Disc.PercentOff);
			CString strDiscount = GetDiscountSaveValue(Disc.Discount);
			CString strDiscountCategoryID, strCouponID, strCustomDiscountDesc;
			GetDiscountCategorySaveValues(Disc.DiscountCategoryID, Disc.CouponID, Disc.CustomDiscountDescription,
				strDiscountCategoryID, strCouponID, strCustomDiscountDesc);

			CString strTemp;
			strTemp.Format("INSERT INTO ChargeDiscountsT (ChargeID, PercentOff, Discount, CouponID, DiscountCategoryID, CustomDiscountDesc) VALUES "
				"(%s, %li, Convert(money,'%s'), %s, %s, '%s')", 
				bNewCharge ? "{CHARGEID}" : AsString(ChargeID), nPercentOff, _Q(strDiscount), _Q(strCouponID), _Q(strDiscountCategoryID), _Q(strCustomDiscountDesc));
			strSaveString += strTemp + ";\r\n";
		}
	}

}
// (s.dhole 2012-04-05 15:34) - PLID 43785 removed CBillingDlg refrence
CString GetDiscountCategoryName(long nDiscountCategoryID) {

	try {
		ADODB::_RecordsetPtr rsName = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {INT}", nDiscountCategoryID);
		if (! rsName->eof) {
			return AdoFldString(rsName, "Description", "");
		}
	}NxCatchAll("Error in CBillingDlg::GetDiscountCategoryName");

	return "";
}

// (s.dhole 2012-04-05 15:34) - PLID 43785 remove CBillingDlg refrence
CString GetCouponName(long nCouponID) {

	try {
		ADODB::_RecordsetPtr rsName = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", nCouponID);
		if (! rsName->eof) {
			return AdoFldString(rsName, "Description", "");
		}
	}NxCatchAll("Error in CBillingDlg::GetCouponName");

	return "";
}

void CBillingDlg::AuditAndGenerateSaveStringForDiscounts(BOOL bNewCharge, BillingItemPtr pItem, CString &strSaveString) {


	CString strPatientName = GetBillPatientName();

	//initialize the save string
	strSaveString = "";

	if (!bNewCharge) {

		long ChargeID = pItem->ChargeID.lVal;
		long nAuditID = -1;

		//we'll have to audit
		_RecordsetPtr rs = CreateParamRecordset("SELECT ChargeDiscountsT.*, "
			" CASE WHEN ChargeDiscountsT.DiscountCategoryID IS NULL THEN '<No Category>' ELSE CASE WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN '<Custom Discount Description>' ELSE  CASE WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN '<Coupon>' ELSE DiscountCategoriesT.Description END END END AS DiscountCategoryName, "
			" CASE WHEN ChargeDiscountsT.CouponID IS NULL THEN '' ELSE CouponsT.Description END AS CouponName "
			" FROM ChargeDiscountsT "
			" LEFT JOIN CouponsT ON ChargeDiscountsT.CouponID = CouponsT.ID "
			" LEFT JOIN DiscountCategoriesT ON ChargeDiscountsT.DiscountCategoryID = DiscountCategoriesT.ID "
			" WHERE ChargeID = {INT} AND DELETED = 0", ChargeID);

		while (!rs->eof) {

			long nDiscountID = AdoFldLong(rs, "ID");			
			CString oldVal, newVal;

			//get the corresponding Discount structure
			stDiscount Disc;
			if (FindDiscountInArray(pItem->DiscountList, nDiscountID, Disc)) {

				long nPercentOff;
				CString strDiscount;
				CString strDiscountCategoryID;
				CString strCustomDiscountDescription;
				CString strCouponID;

				//check the discount against the data
				nPercentOff = GetPercentOffSaveValue(Disc.PercentOff);

				_variant_t var = rs->Fields->Item["PercentOff"]->Value;
				if(var.vt != VT_NULL && var.lVal != nPercentOff) {
					if(nAuditID==-1)
						nAuditID = BeginNewAuditEvent();
					oldVal.Format("%li",var.lVal);
					newVal.Format("%li",nPercentOff);
					AuditEvent(m_nPatientID, strPatientName,nAuditID,m_EntryType == 1 ? aeiChargePercentOff : aeiQuoteChargePercentOff,ChargeID,oldVal,newVal,aepMedium);
				}
				

				strDiscount = GetDiscountSaveValue(Disc.Discount);
				
				var = rs->Fields->Item["Discount"]->Value;
				if(var.vt != VT_NULL && FormatCurrencyForSql(COleCurrency(var.cyVal)) != strDiscount) {
					if(nAuditID==-1)
						nAuditID = BeginNewAuditEvent();
					AuditEvent(m_nPatientID, strPatientName,nAuditID,m_EntryType == 1 ? aeiChargeDiscount : aeiQuoteChargeDiscount,ChargeID,FormatCurrencyForInterface(COleCurrency(var.cyVal)),FormatCurrencyForInterface(ParseCurrencyFromSql(strDiscount)),aepMedium);
				}
				
				// (j.gruber 2007-03-23 09:31) - PLID 24870 - adding discount categories
				// (j.gruber 2007-04-05 14:17) - PLID 9796 - Adding support for Coupons
				GetDiscountCategorySaveValues(Disc.DiscountCategoryID, Disc.CouponID, Disc.CustomDiscountDescription,
					strDiscountCategoryID, strCouponID, strCustomDiscountDescription);
				
				var = rs->Fields->Item["DiscountCategoryID"]->Value;
				if (var.vt != VT_NULL && var.vt == VT_I4) {
					if ((var.lVal != atoi(strDiscountCategoryID)) || (var.lVal == -2 && atoi(strDiscountCategoryID) == -2)) {
						if (nAuditID == -1) {
							nAuditID = BeginNewAuditEvent();
						}
						if (var.lVal == -2) {

							// (j.gruber 2007-04-03 17:32) - PLID 9796 - Coupons
							//they changed from a coupon to a non coupon
							_variant_t var2 = rs->Fields->Item["CouponName"]->Value;
							if(var2.vt != VT_NULL && var2.lVal != atoi(strCouponID)) {
								if(nAuditID==-1)
									nAuditID = BeginNewAuditEvent();								
								
								long nOldCouponID = AdoFldLong(rs, "CouponID", -1);
								if (nOldCouponID == -1) {
									oldVal = "<Coupon> - <Unspecified>";
								}
								else {
									oldVal = "<Coupon> - " + CString(var2.bstrVal);
								}
									
								//see what the new one is
								if (strDiscountCategoryID == " NULL ") {
									newVal = "<No Category>";
								}
								else if (strDiscountCategoryID == "-1") {
									newVal = "<Custom Discount Description>";
								}
								else if (strDiscountCategoryID == "-2") {
									//from a coupon to a coupon!
									if (strCouponID == " NULL ") {
										newVal = "<Coupon> - <Unspecified>";
									}
									else {
										_RecordsetPtr rsDesc = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", atoi(strCouponID));
										if (!rsDesc->eof) {
											newVal = "<Coupon> - " + GetCouponName(atoi(strCouponID));
										}											
									}
								}
								else {
									//it must be a discount category
									_RecordsetPtr rsDisCat = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {STRING}", strDiscountCategoryID);
									if (!rsDisCat->eof) {
										newVal = AdoFldString(rsDisCat, "Description", "<No Category>");
									}
								}								
							}
							else if (var2.vt == VT_NULL && strCouponID != " NULL ") {
								//they changed it from nothing to something
								if (nAuditID == -1) {
									nAuditID = BeginNewAuditEvent();
								}
								//maybe it was a different description
								if (var.vt != NULL) {
									oldVal = AdoFldString(rs, "DiscountCategoryName");
								}
								else {
									ASSERT(FALSE);
									oldVal = "<No Category>";
								}								
								_RecordsetPtr rsDesc = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", atoi(strCouponID));
								if (!rsDesc->eof) {
									newVal = "<Coupon> - " + AdoFldString(rs, "Description");
								}
								AuditEvent(m_nPatientID, strPatientName, nAuditID, m_EntryType == 1 ? aeiChargeDiscountCategory : aeiQuoteDiscountCategory, ChargeID, oldVal, newVal, aepMedium);
							}							
						}		
						else {
							oldVal = AdoFldString(rs, "DiscountCategoryName", "<No Category>");
							if (strDiscountCategoryID == "-1" ) {
								newVal = "<Custom Discount Description>";
							}
							else if (strDiscountCategoryID == " NULL ") {
								newVal = "<No Category>";
							}
							else if (strDiscountCategoryID == "-2") {
								//we changed from a non coupon to a coupon
								_variant_t var2 = Disc.CouponID; 
								if (strCouponID == " NULL ") {
									newVal = "<Coupon> - <Unspecified>";
								}
								else {
									_RecordsetPtr rsDesc = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", atoi(strCouponID));
									if (!rsDesc->eof) {
										newVal = "<Coupon> - " + AdoFldString(rsDesc, "Description");
									}
									else {
										ASSERT(FALSE);
									}
								}								
							}
							else {
								_RecordsetPtr rsDisCat = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {STRING}", strDiscountCategoryID);
								if (!rsDisCat->eof) {
									newVal = AdoFldString(rsDisCat, "Description", "<No Category>");
								}
							}
						}
						AuditEvent(m_nPatientID, strPatientName, nAuditID, m_EntryType == 1 ? aeiChargeDiscountCategory : aeiQuoteDiscountCategory, ChargeID, oldVal, newVal, aepMedium);
					}
				}
				else {
					//they couldn've still changed it
					if (strDiscountCategoryID != " NULL ") {

						//they changed it from blank to something
						if (nAuditID == -1) {
							nAuditID = BeginNewAuditEvent();
						}
						oldVal = "<No Category>";
						if (atoi(strDiscountCategoryID) == -2) {
							//they changed from nothing to a coupon
							if (strCouponID == " NULL ") {
								newVal = "<Coupon> - <Unspecified>";
							}
							else {
								_RecordsetPtr rsDesc = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", atoi(strCouponID));
								if (!rsDesc->eof) {
									newVal = "<Coupon> - " + AdoFldString(rs, "Description");
								}
							}
						}
						else if (atoi(strDiscountCategoryID) == -1) {
							newVal = "<Custom Discount Description>";
						}
						else {
							_RecordsetPtr rsDisCat = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {STRING}", strDiscountCategoryID);
							if (!rsDisCat->eof) {
								newVal = AdoFldString(rsDisCat, "Description", "<No Category>");
							}
						}
						AuditEvent(m_nPatientID, strPatientName, nAuditID, m_EntryType == 1 ? aeiChargeDiscountCategory : aeiQuoteDiscountCategory, ChargeID, oldVal, newVal, aepMedium);
					}
				}
			

				var = rs->Fields->Item["CustomDiscountDesc"]->Value;
				if (var.vt != VT_NULL && var.vt != VT_EMPTY) {
					if (CString(var.bstrVal).CompareNoCase(strCustomDiscountDescription) != 0) {
						if (nAuditID == -1) {
							nAuditID = BeginNewAuditEvent();
						}
						oldVal = CString(var.bstrVal);
						newVal = strCustomDiscountDescription;
						AuditEvent(m_nPatientID, strPatientName, nAuditID, m_EntryType == 1 ? aeiChargeCustomDiscountDescription : aeiQuoteCustomDiscountDescription, ChargeID, oldVal, newVal, aepMedium);
					}
				}

				CString strTemp;
				strTemp.Format("UPDATE ChargeDiscountsT SET PercentOff = %li, Discount = Convert(money,'%s'), CouponID = %s, DiscountCategoryID = %s, CustomDiscountDesc = '%s' WHERE ID = %li",
					nPercentOff, _Q(strDiscount), _Q(strCouponID), _Q(strDiscountCategoryID), _Q(strCustomDiscountDescription), nDiscountID);

				strSaveString += strTemp + ";\r\n";
			} //end if found
			else {
				
				//we didn't find it, they must've deleted it
				if (nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}
				//we'll need to load it from data
				CString strDescription;

				strDescription += "Percent Off: " + AsString(AdoFldLong(rs, "PercentOff", 0)) + ";";
				strDescription += "Discount: " + FormatCurrencyForInterface(AdoFldCurrency(rs, "Discount", COleCurrency(0,0))) + ";";
			
				long nDiscountCatID = AdoFldLong(rs, "DiscountCategoryID", -3);
				if (nDiscountCatID == -1) {
					//custom discount
					strDescription += "Custom Category: " + AdoFldString(rs, "CustomDiscountDesc", "") + ";";
				}
				else if (nDiscountCatID == -2) {
					//coupon
					long nCouponID = AdoFldLong(rs, "CouponID", -1);
					if (nCouponID <= 0) {
						strDescription += "Coupon;";
					}
					else {
						strDescription += "Coupon: " + GetCouponName(nCouponID) + ";";
					}					
				}
				else if (nDiscountCatID == -3) {
					strDescription += "Discount Category: <No Category>;";
				}
				else {
					strDescription += "Discount Category: " + GetDiscountCategoryName(nDiscountCatID) + ";";
				}
				

				CString strAuditDesc = strDescription;
				AuditEvent(m_nPatientID, strPatientName, nAuditID, m_EntryType == 1 ? aeiChargeDiscountDeleted : aeiQuoteChargeDiscountDeleted, ChargeID, strAuditDesc, "<Deleted>", aepMedium);

				CString strTemp;
				strTemp.Format("UPDATE ChargeDiscountsT SET DELETED = 1, DELETEDBy = '%s', DeleteDate = getDate() WHERE ID = %li", _Q(GetCurrentUserName()), AdoFldLong(rs, "ID"));
				strSaveString += strTemp + ";\r\n";
			}

			rs->MoveNext();
		}//end recordset
		
		//now do new charges
		CString strTemp;
		AuditAndGenerateSaveStringForNewDiscounts(FALSE, ChargeID, pItem->DiscountList, strTemp);
		strSaveString += strTemp;		

	}//end if NewCharge
	else {

		//its a new charge, we don't audit the discounts
		AuditAndGenerateSaveStringForNewDiscounts(TRUE, -1, pItem->DiscountList, strSaveString);

	}

}


void CBillingDlg::OnBnClickedBtnAdvancedDiscounting()
{

	try {

		// (j.jones 2011-01-25 14:58) - PLID 42156 - check historical permissions
		if(m_EntryType == 1 && GetBillID() != -1
			&& !CanChangeHistoricFinancial("Bill", GetBillID(), bioBill, sptWrite)) {
			return;
		}

		//build an map of discounts
		CMap<long, long, DiscountList*, DiscountList*> mapDiscounts;
		CMap<long, long, CString, CString> mapChargeDescs;
		// (j.jones 2011-08-24 08:41) - PLID 44868 - added a map for original/void charges
		CMap<long, long, BOOL, BOOL> mapCorrectedCharges;

		//loop through the billing array and setup the maps
		for(int i=0;i<(int)m_billingItems.size();i++) {
			BillingItemPtr pItem = m_billingItems[i];

			DiscountList *pDiscountList = new DiscountList;
			//loop through and set our list to the new list
			for (int j=0; j < pItem->DiscountList->aryDiscounts.GetCount(); j++) {
				stDiscount Disc = pItem->DiscountList->aryDiscounts.GetAt(j);

				pDiscountList->aryDiscounts.Add(Disc);
			}

			mapDiscounts.SetAt(VarLong(pItem->LineID), pDiscountList);
			mapChargeDescs.SetAt(VarLong(pItem->LineID), pItem->Description.bstrVal);

			// (j.jones 2011-08-24 08:41) - PLID 44868 - added a map for original/void charges
			BOOL bIsCorrected = IsOriginalOrVoidCharge(VarLong(pItem->ChargeID, -2));
			mapCorrectedCharges.SetAt(VarLong(pItem->LineID), bIsCorrected);
		}

		// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bIsBill
		// (j.jones 2011-08-24 08:41) - PLID 44868 - added a map for original/void charges
		CAdvDiscountDlg dlg(&mapDiscounts, &mapChargeDescs, &mapCorrectedCharges, m_EntryType == 1 ? true : false);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {

			POSITION pos = mapDiscounts.GetStartPosition();
			long nLineID;
			DiscountList *pDiscountList;
			while (pos != NULL) {
				mapDiscounts.GetNextAssoc(pos, nLineID, pDiscountList);

				//now loop through the array and find the correct item
				BillingItemPtr pItem;
				for(int i=0;i<(int)m_billingItems.size();i++) {
					pItem = m_billingItems[i];

					if (pItem->LineID.lVal == nLineID) {
						//we found the one we want, now break out of the inner loop
						break;
					}
				}

				ASSERT(pItem);
				
				//remove everything in this charge's list
				pItem->DiscountList->aryDiscounts.RemoveAll();
				//set the map's discount list to be the arrays
				for (int j=0; j < pDiscountList->aryDiscounts.GetCount(); j++) {
				
					stDiscount Disc = pDiscountList->aryDiscounts.GetAt(j);

					pItem->DiscountList->aryDiscounts.Add(Disc);
					
				}
				
				if (pDiscountList) {
					pDiscountList->aryDiscounts.RemoveAll();
					delete pDiscountList;
				}
			}

			//now loop through the array again and calculate the line total
			for(int i=0;i<(int)m_billingItems.size();i++) {
				BillingItemPtr pItem = m_billingItems[i];

				// (j.jones 2011-08-24 08:41) - PLID 44868 - skip original/void charges
				if(IsOriginalOrVoidCharge(VarLong(pItem->ChargeID, -2))) {
					continue;
				}

				COleCurrency cyInvalid;
				cyInvalid.SetStatus(COleCurrency::invalid);
				m_varBoundItem = pItem->LineID;			
				CalculateLineTotal(cyInvalid, cyInvalid, -1, -1, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, cyInvalid);

				//add all other charges to the modified list since they potentially modified all of them
				AddToModifiedList(VarLong(pItem->ChargeID, -2));
			}

			
		}
		else {

			//we still need to clear our array
			POSITION pos = mapDiscounts.GetStartPosition();
			long nLineID;
			DiscountList *pDiscountList;
			while (pos != NULL) {
				mapDiscounts.GetNextAssoc(pos, nLineID, pDiscountList);
				if (pDiscountList) {
					pDiscountList->aryDiscounts.RemoveAll();
					delete pDiscountList;
				}
			}
		}
	}NxCatchAll("Error in CBillingDlg::OnBnClickedBtnAdvancedDiscounting()");
}


// (j.gruber 2007-05-02 16:25) - PLID 14202 - button to discount entire bill
void CBillingDlg::OnBtnApplyDiscountToAll() 
{	
	try {

		// (j.jones 2011-01-25 14:58) - PLID 42156 - check historical permissions;
		// in the unlikely event that they need a password for both historical changes
		// and discounting the entire bill, they will have to enter it twice
		if(m_EntryType == 1 && GetBillID() != -1
			&& !CanChangeHistoricFinancial("Bill", GetBillID(), bioBill, sptWrite)) {
			return;
		}

		if (m_EntryType == 1) {
			if (CheckCurrentUserPermissions(bioDiscountEntireBill, sptWrite)) {
				ApplyDiscountToAll(atapNotApplicable);
			}
		}
		else {
			//quote
			if (CheckCurrentUserPermissions(bioDiscountEntireQuote, sptWrite)) {

				// (j.gruber 2009-03-23 16:50) - PLID 33355 - check the preference and pop up a menu if necessary
				long nPreference = GetRemotePropertyInt("QuoteDiscountApplyToAllChargesWhichLines", 1, 0, "<None>", true);
				if (nPreference == 2) {
					//we need to pop up a menu
					CMenu pMenu;
					pMenu.CreatePopupMenu();
					pMenu.InsertMenu(0, MF_BYPOSITION, ID_PRACTICEFEES, "Apply to lines with Practice Fees only");
					pMenu.InsertMenu(1, MF_BYPOSITION, ID_BOTHPRACTICEANDOTHERFEES, "Apply to All Lines");

					CPoint pt;
					GetCursorPos(&pt);
					pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);				
				}
				else if (nPreference == 0) {
					ApplyDiscountToAll(atapPracticeFeesOnly);
				}
				else {
					//use all
					ApplyDiscountToAll(atapAllLines);
				}
			}
		}
	}NxCatchAll("Error in OnBtnApplyDiscountToAll");
}

// (r.gonet 2016-02-06 16:46) - PLID 68193 - Gets the number of unvoided gift certificate
// charges on the bill.
long CBillingDlg::GetGiftCertificateCount() const
{
	long nGiftCertificateCount = 0;
	for (int i = 0; i < (int)m_billingItems.size(); i++) {
		BillingItemPtr pItem = m_billingItems[i];
		if (pItem->GiftID.vt == VT_I4 && !IsOriginalOrVoidCharge(VarLong(pItem->ChargeID, -2))) {
			nGiftCertificateCount++;
		}
	}
	return nGiftCertificateCount;
}

// (r.gonet 2016-02-06 16:46) - PLID 68193 - Gets the number of unvoided, discountable charges
// on the bill.
long CBillingDlg::GetDiscountableChargeCount() const
{
	long nDiscountableChargeCount = 0;
	for (int i = 0; i < (int)m_billingItems.size(); i++) {
		BillingItemPtr pItem = m_billingItems[i];
		// (r.gonet 2016-02-06 16:46) - PLID 68193 - Gift certificates are not discountable.
		// Neither are voided charges.
		if (pItem->GiftID.vt != VT_I4 && !IsOriginalOrVoidCharge(VarLong(pItem->ChargeID, -2))) {
			nDiscountableChargeCount++;
		}
	}
	return nDiscountableChargeCount;
}

// (a.walling 2007-05-10 15:22) - PLID 25171 - Apply a coupon to all (or prompt if -1)
void CBillingDlg::ApplyDiscountToAll(ApplyToAllPref atapPref, long nCouponID /* = -1 */)
{
	
	try {
		// (j.gruber 2009-03-23 17:44) - PLID 33355 - change to new discount structure
		BOOL bHasExistingDiscount = FALSE;

		// (r.gonet 2016-02-06 16:46) - PLID 68193 - Initialize using the new function.
		BOOL bHasWriteableCharge = GetDiscountableChargeCount() > 0 ? TRUE : FALSE;

		// (r.farnworth 2013-08-02 16:14) - PLID 45994 - No longer allow discounts to be used on Gift Certificates
		// (r.gonet 2016-02-06 16:46) - PLID 68193 - Initialize using the new function.
		BOOL bHasGiftCertificate = GetGiftCertificateCount() > 0 ? TRUE : FALSE;
		
		CMap<long, long, long, long> mapIDs;
		//check to see if any charges on this bill already have discounts or discount categories
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		int i = 0;
		for(i=0;i<(int)m_billingItems.size();i++) {

			BillingItemPtr pItem = m_billingItems[i];

			long nLineID = pItem->LineID.lVal;
		
			// (j.jones 2011-08-24 08:41) - PLID 44868 - skip original & void charges
			if(IsOriginalOrVoidCharge(VarLong(pItem->ChargeID, -2))) {
				continue;
			}
			
			if (pItem->DiscountList) {
				if (pItem->DiscountList->aryDiscounts.GetCount()) {
					bHasExistingDiscount = TRUE;
					mapIDs.SetAt(nLineID, nLineID);
				}
			}			
		}

		// (r.farnworth 2013-08-02 16:14) - PLID 45994 - If all charges are discounts, warn and leave.
		if(!bHasWriteableCharge && bHasGiftCertificate) {
			AfxMessageBox("All charges on this bill are Gift Certificates and cannot be discounted.");
			return;
		}

		// (j.jones 2011-08-24 08:41) - PLID 44868 - if all charges are read-only,
		// warn and leave
		if(!bHasWriteableCharge) {
			AfxMessageBox("All charges on this bill are original or void charges, and cannot be edited.");
			return;
		}

		// (r.farnworth 2013-08-02 16:14) - PLID 45994 - If they have Gift Certificates in addition to other charges,
		//	continue the discount but warn that they will not be applied to GCs
		if(bHasGiftCertificate) {
			AfxMessageBox("Discounts will not be applied to Gift Certificates.");
		}

		//they want to continue
		BOOL bContinue = FALSE;

		BOOL bApplyFromLineTotal = FALSE;

		// prompt for coupon info
		m_pDiscountBillDlg.reset(new CDiscountBillDlg(this));

		// (j.jones 2008-12-23 10:52) - PLID 32492 - tell this dialog if we already have discounts
		m_pDiscountBillDlg->m_bHasDiscountsAlready = bHasExistingDiscount;

		m_pDiscountBillDlg->m_bIsBill = m_EntryType == 1;

		long nPercentOff = 0, nDiscountCategory = -2;
		COleCurrency cyDiscount;
		CString strCustomDiscountCategory;
		
		if (nCouponID != -1) {
			m_pDiscountBillDlg->m_nDiscountCatID = -2; // coupon
			m_pDiscountBillDlg->m_nCouponID = nCouponID;
			m_pDiscountBillDlg->m_bCouponSet = TRUE;
		}

		if (IDOK == m_pDiscountBillDlg->DoModal()) {
			nPercentOff = m_pDiscountBillDlg->m_nPercentOff;
			cyDiscount = m_pDiscountBillDlg->m_cyDiscount;
			nDiscountCategory = m_pDiscountBillDlg->m_nDiscountCatID;
			nCouponID = m_pDiscountBillDlg->m_nCouponID;
			strCustomDiscountCategory = m_pDiscountBillDlg->m_strCustomDescription;
			bApplyFromLineTotal = m_pDiscountBillDlg->m_bApplyFromLineTotal;
			bContinue = TRUE;
		} else {
			bContinue = FALSE;
		}

		// (j.jones 2008-12-23 11:20) - PLID 32492 - track the user's selection for discount overwriting
		EDiscountBillOverwriteOption edboLastDiscountBillOverwriteOption = edboUnspecified;
		if(m_pDiscountBillDlg->m_bHasDiscountsAlready) {
			edboLastDiscountBillOverwriteOption = m_pDiscountBillDlg->m_edboLastDiscountBillOverwriteOption;
		}

		m_pDiscountBillDlg.reset();

		if (bContinue) {

			_variant_t varNull;
			varNull.vt = VT_NULL;

			BOOL bOutputMessage = FALSE;
			CString strOutputDescription;
		
			// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
			int i = 0;
			for(i=0;i<(int)m_billingItems.size();i++) {

				COleCurrency cyChargeDiscount = cyDiscount;
				long nChargePercentOff = nPercentOff;

				BillingItemPtr pItem = m_billingItems[i];

				long nID = pItem->LineID.lVal;

				// (j.jones 2011-08-24 08:41) - PLID 44868 - skip original & void charges
				// (r.farnworth 2013-08-02 17:03) - PLID 45994 - Skip Gift Certificates
				if((IsOriginalOrVoidCharge(VarLong(pItem->ChargeID, -2))) || (pItem->GiftID.vt == VT_I4)) {
					continue;
				}

				BOOL bHasDiscounts = mapIDs.Lookup(nID, nID) != 0;

				// (j.jones 2008-12-23 11:30) - PLID 32492 - this code replaces discounts, so if it is
				// not in our map of tracked IDs, or the user's choice was to replace discounts, then do so
				if(!bHasDiscounts
					|| edboLastDiscountBillOverwriteOption == edboReplace) {

				
					// (j.gruber 2009-03-23 17:52) - PLID 33355 - update to new structure
					//we are clearing the discounts
					
					pItem->DiscountList->aryDiscounts.RemoveAll();

					if (nChargePercentOff > 0 && bApplyFromLineTotal) {
						//get the line total first and then calculate a dollar discount off that												
						COleCurrency cyCurrentLineTotal = GetLineTotal(pItem, FALSE, atapPref);

						//now calculate what the dollar discount will be
						COleCurrency cyTempDiscount;
						cyTempDiscount = cyCurrentLineTotal * (nChargePercentOff * 1000); 
						cyTempDiscount = cyTempDiscount / long(100000);

						cyChargeDiscount += cyTempDiscount;

						//now clear the percent off
						nChargePercentOff = 0;
					}
					
					//now add the new discount back in
					//check our preference
					if (atapPref == atapPracticeFeesOnly ) {
						//check to make sure there is a practice fee
						COleCurrency cyPracticeFee = pItem->UnitCost.cyVal;
						if ( cyPracticeFee > COleCurrency(0,0)) {
							// (j.gruber 2009-03-24 13:14) - PLID 33246 - make sure we aren't going to go negative
							//it will only happen with a dollar discount
							//actually as long as there are any discounts, we have to check
							if (CanAddDiscount(pItem, FALSE, atapPref, nChargePercentOff, cyChargeDiscount) ) {
									AddToDiscountList(pItem->DiscountList, varNull, nChargePercentOff, _variant_t(cyChargeDiscount),
									_variant_t(strCustomDiscountCategory), nDiscountCategory, nCouponID, dpIgnorePreference);
							}
							else {									
								//we are going to go below 0, don't add it
								bOutputMessage = TRUE;
								CString strTemp;
								strTemp.Format("%s with a total of %s\r\n", (CString)pItem->Description.bstrVal, FormatCurrencyForInterface(pItem->LineTotal.cyVal));
								strOutputDescription += "Charge: " + strTemp;
								
							}						
						}
					}
					else {
						// (j.gruber 2009-03-24 13:14) - PLID 33246 - make sure we aren't going to go negative
							//it will only happen with a dollar discount
						//actually it can happen if there exists a dollar discount, so check it either way
						COleCurrency cyPracticeFee = pItem->UnitCost.cyVal;
						COleCurrency cyOtherFee = pItem->OthrUnitCost.cyVal;
						if (cyPracticeFee > COleCurrency(0,0) || cyOtherFee > COleCurrency(0,0)) {

							if (CanAddDiscount(pItem, FALSE, atapPref, nChargePercentOff, cyChargeDiscount) ) {
									AddToDiscountList(pItem->DiscountList, varNull, nChargePercentOff, _variant_t(cyChargeDiscount),
									_variant_t(strCustomDiscountCategory), nDiscountCategory, nCouponID, dpIgnorePreference);
							}
							else {
								bOutputMessage = TRUE;
								CString strTemp;
								strTemp.Format("%s with a total of %s\r\n", (CString)pItem->Description.bstrVal, FormatCurrencyForInterface(pItem->LineTotal.cyVal));
								strOutputDescription += "Charge: " + strTemp;
							}						
						}
					}

					
					//update the row
					m_varBoundItem = pItem->LineID;
					AddToModifiedList(pItem->ChargeID);

					COleCurrency cyInvalid;
					cyInvalid.SetStatus(COleCurrency::invalid);
					// (j.gruber 2009-03-05 17:39) - PLID 33351 - changed to use totaldiscount
					CalculateLineTotal(cyInvalid, cyInvalid, -1, -1, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, cyInvalid);
				}
				// (j.jones 2008-12-23 11:32) - PLID 32492 - this code will add to discounts, so if it is
				// in our map of tracked IDs, and the user's choice was to add discounts, then do so now
				else if(bHasDiscounts
					&& edboLastDiscountBillOverwriteOption == edboAdd) {


					if (nChargePercentOff > 0 && bApplyFromLineTotal) {
						//get the line total first and then calculate a dollar discount off that						

						COleCurrency cyCurrentLineTotal = GetLineTotal(pItem, FALSE, atapPref);

						//now calculate what the dollar discount will be
						COleCurrency cyTempDiscount;
						cyTempDiscount = cyCurrentLineTotal * (nChargePercentOff * 1000); 
						cyTempDiscount = cyTempDiscount / long(100000);

						cyChargeDiscount += cyTempDiscount;

						//now clear the percent off
						nChargePercentOff = 0;
					}

					//we are just adding on to the discount list
					//check our preference
					if (atapPref == atapPracticeFeesOnly ) {
						//check to make sure there is a practice fee
						COleCurrency cyPracticeFee = pItem->UnitCost.cyVal;
						if (cyPracticeFee > COleCurrency(0,0)) {
							// (j.gruber 2009-03-24 13:14) - PLID 33246 - make sure we aren't going to go negative
							//it will only happen with a dollar discount
							//not true, it can happen if a dollar discount already exists
							if (CanAddDiscount(pItem, FALSE, atapPref, nChargePercentOff, cyChargeDiscount)) {
								AddToDiscountList(pItem->DiscountList, varNull, nChargePercentOff, _variant_t(cyChargeDiscount),
									_variant_t(strCustomDiscountCategory), nDiscountCategory, nCouponID, dpIgnorePreference);
							}
							else {
									bOutputMessage = TRUE;
									CString strTemp;
									strTemp.Format("%s with a total of %s\r\n", (CString)pItem->Description.bstrVal, FormatCurrencyForInterface(pItem->LineTotal.cyVal));
									strOutputDescription += "Charge: " + strTemp;
							}						
						}
					}
					else {
						COleCurrency cyPracticeFee = pItem->UnitCost.cyVal;
						COleCurrency cyOtherFee = pItem->OthrUnitCost.cyVal;
						if (cyPracticeFee > COleCurrency(0,0) || cyOtherFee > COleCurrency(0,0)) {
							// (j.gruber 2009-03-24 13:14) - PLID 33246 - make sure we aren't going to go negative
								//it will only happen with a dollar discount
								//actually, that's not the case, it could happen if there is  a dollar discount already
							if (CanAddDiscount(pItem, FALSE, atapPref, nChargePercentOff, cyChargeDiscount)) {
									AddToDiscountList(pItem->DiscountList, varNull, nChargePercentOff, _variant_t(cyChargeDiscount),
										_variant_t(strCustomDiscountCategory), nDiscountCategory, nCouponID, dpIgnorePreference);
							}
							else {
									bOutputMessage = TRUE;
									CString strTemp;
									strTemp.Format("%s with a total of %s\r\n", (CString)pItem->Description.bstrVal, FormatCurrencyForInterface(pItem->LineTotal.cyVal));
									strOutputDescription += "Charge: " + strTemp;
							}							
						}
					
					}

					//update the row
					m_varBoundItem = pItem->LineID;
					AddToModifiedList(pItem->ChargeID);

					COleCurrency cyInvalid;
					cyInvalid.SetStatus(COleCurrency::invalid);
					// (j.gruber 2009-03-05 17:39) - PLID 33351 - changed to use totaldiscount
					CalculateLineTotal(cyInvalid, cyInvalid, -1, -1, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, cyInvalid);
				}
			}

			// (j.gruber 2009-03-24 13:29) - PLID 33246 - check to see if we need to popup a message
			if (bOutputMessage) {
				MsgBox("The following charges did not get the discount applied as doing so would create negative charges \n%s", strOutputDescription);
			}
			
		}

		mapIDs.RemoveAll();
	}NxCatchAll("Error in OnBtnApplyDiscountToAll");

	m_pDiscountBillDlg.reset();	
}

void CBillingDlg::OnApplyDiscountToPracticeFeeLinesOnly() {


	try {
		//pass in our preference selection to the apply discount dialog
		ApplyDiscountToAll(atapPracticeFeesOnly);
	}NxCatchAll("CBillingDlg::OnApplyDiscountToPracticeFeeLinesOnly()");

}



void CBillingDlg::OnApplyDiscountToAllLines() {

	try {
		ApplyDiscountToAll(atapAllLines);
	}NxCatchAll("CBillingDlg::OnApplyDiscountToAllLines()");
}


// (j.gruber 2009-03-23 17:32) - PLID 33474 - moved from BillingDlg.cpp
BOOL CBillingDlg::ApplyDiscountFromBarCode(LPARAM lParam) {
	
	CString strCouponName;
	long nCouponID = -1;

	// (a.walling 2007-05-09 12:59) - PLID 25171 - We have a barcode; select the coupon if we can
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	_bstr_t bstr = (BSTR)lParam; // We can typecast this to an ANSI string
	CString str = (LPCTSTR)bstr;

	ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID, Description, CASE WHEN CONVERT(datetime, Convert(nvarchar, StartDate, 1)) <= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 1 ELSE 0 END AS Started, CASE WHEN EndDate >= CONVERT(datetime, Convert(nvarchar, getDate(), 1)) THEN 0 ELSE 1 END AS Expired FROM CouponsT WHERE BarCode = '%s'", _Q(str));
	if (!prs->eof) {
		strCouponName = AdoFldString(prs, "Description", "");
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
			//re-enable barcodes in billing
			EnableBarcoding();
			return 0;
		}

		ApplyDiscountToAll(atapNotApplicable, nID);

		return TRUE;

	} else {
		// ignore
	}

	return FALSE;

}

// (j.gruber 2009-03-19 15:56) - PLID 33355 - pretty much copied from CalculateLineTotal in BillingDlg
COleCurrency CBillingDlg::GetLineTotal(_variant_t varUnitCost, _variant_t varOtherFee, _variant_t varQuantity, 
		_variant_t varMultiplier1, _variant_t varMultiplier2, _variant_t varMultiplier3, _variant_t varMultiplier4,
		_variant_t varTax1, _variant_t varTax2, DiscountList *pDiscountList,
		BOOL bIncludeTax, BOOL bOnlyPracticeFee/*= FALSE*/, BOOL bOnlyOtherFee/*=FALSE*/) {


		COleCurrency cyUnitCost = varUnitCost.cyVal;		

		
		COleCurrency cyOtherFee;		
		if(varOtherFee.vt != VT_NULL && varOtherFee.vt != VT_EMPTY)
			cyOtherFee = varOtherFee.cyVal;
		else
			cyOtherFee = COleCurrency(0,0);
	

		double dblQuantity = varQuantity.dblVal;		

		
		double dblMultiplier1 = (varMultiplier1.vt == VT_EMPTY) ? 1.0 : VarDouble(varMultiplier1,1.0);
		double dblMultiplier2 = (varMultiplier2.vt == VT_EMPTY) ? 1.0 : VarDouble(varMultiplier2,1.0);
		double dblMultiplier3 = (varMultiplier3.vt == VT_EMPTY) ? 1.0 : VarDouble(varMultiplier3,1.0);
		double dblMultiplier4 = (varMultiplier4.vt == VT_EMPTY) ? 1.0 : VarDouble(varMultiplier4,1.0);

		
		double dblTax1;
		if(varTax1.vt == VT_R8)
			dblTax1 = varTax1.dblVal;
		else
			dblTax1 = 0.0;
		
		dblTax1 /= 100.0;
		dblTax1 += 1.0;


		double dblTax2;
		if(varTax2.vt == VT_R8)
			dblTax2 = varTax2.dblVal;
		else
			dblTax2 = 0.0;

		dblTax2 /= 100.0;
		dblTax2 += 1.0;

	COleCurrency cyLineTotal;
	if (bOnlyPracticeFee) {
		cyLineTotal = cyUnitCost;
	}
	else if (bOnlyOtherFee) {
		cyLineTotal = cyOtherFee;	
	}
	else {
		cyLineTotal = cyUnitCost + cyOtherFee;	
	}

	cyLineTotal = CalculateAmtQuantity(cyLineTotal,dblQuantity);

	//highly unlikely anyone would use modifiers and discounts together,
	//but if so, modify first
	cyLineTotal = CalculateAmtQuantity(cyLineTotal,dblMultiplier1);
	cyLineTotal = CalculateAmtQuantity(cyLineTotal,dblMultiplier2);
	cyLineTotal = CalculateAmtQuantity(cyLineTotal,dblMultiplier3);
	cyLineTotal = CalculateAmtQuantity(cyLineTotal,dblMultiplier4);

	//loop through and get the total discounts
	long nTotalPercentOff = 0;
	COleCurrency cyTotalDiscount;

	for (int i= 0; i < pDiscountList->aryDiscounts.GetCount(); i++) {
		stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);

		if (Disc.PercentOff.vt == VT_I4) {
			nTotalPercentOff += Disc.PercentOff.lVal;
		}

		if (Disc.Discount.vt == VT_CY) {
			cyTotalDiscount += Disc.Discount.cyVal;
		}
	}

	cyLineTotal = (cyLineTotal * (100000 - nTotalPercentOff * 1000));
	// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - Operator / is ambiguous
	cyLineTotal = cyLineTotal / long(100000);
	cyLineTotal -= cyTotalDiscount;

	//if they don't want tax then calculate here
	if (!bIncludeTax) {
		RoundCurrency(cyLineTotal);
		return cyLineTotal;
	}
	
	//Equivalent to cy = cy + (cy * (double)(dblTax / 100.0));
	COleCurrency taxResult1, taxResult2;

	taxResult1 = CalculateTax(cyLineTotal,dblTax1);
	taxResult2 = CalculateTax(cyLineTotal,dblTax2);

	cyLineTotal += taxResult1;
	cyLineTotal += taxResult2;

	RoundCurrency(cyLineTotal);

	return cyLineTotal;

}

// (j.gruber 2009-04-03 17:23) - PLID 33246 - created to check new discounts before adding
BOOL CBillingDlg::CanAddDiscount(BillingItemPtr pItem, BOOL bIncludeTax, ApplyToAllPref atapPref, long nPercentOff, COleCurrency cyDiscount) {

	//make a new discount list with our new discount and get the line total
	DiscountList *pNewDiscountList = new DiscountList;

	for (int i = 0; i < pItem->DiscountList->aryDiscounts.GetCount(); i++) {
		stDiscount disc = ((stDiscount)pItem->DiscountList->aryDiscounts.GetAt(i));

		pNewDiscountList->aryDiscounts.Add(disc);
	}

	//now add our new one
	stDiscount DiscToAdd;
	DiscToAdd.Discount = _variant_t(cyDiscount);
	DiscToAdd.PercentOff = (long)nPercentOff;

	pNewDiscountList->aryDiscounts.Add(DiscToAdd);

	//now get the current line total
	BOOL bUseOnlyPracticeFee = FALSE, bUseOnlyOutsideFee = FALSE;
	if (m_EntryType != 1) {
		//its a quote, check our preference
		if (atapPref == atapPracticeFeesOnly) {
			bUseOnlyPracticeFee = TRUE;
		}
	}

	COleCurrency cyAmount = GetLineTotal(pItem->UnitCost, pItem->OthrUnitCost, pItem->Quantity, 
		pItem->Multiplier1, pItem->Multiplier2, pItem->Multiplier3, pItem->Multiplier4,
		pItem->TaxRate1, pItem->TaxRate2, pNewDiscountList, 
		bIncludeTax, bUseOnlyPracticeFee, bUseOnlyOutsideFee);


	BOOL bReturn = FALSE;

	if (cyAmount < COleCurrency(0,0)) {
		//its negative, can't add
		bReturn = FALSE;
	}
	else {
		bReturn = TRUE;
	}

	//delete from our array
	pNewDiscountList->aryDiscounts.RemoveAll();
	if (pNewDiscountList) {
		delete pNewDiscountList;
	}

	return bReturn;
}

COleCurrency CBillingDlg::GetLineTotal(BillingItemPtr pItem, BOOL bIncludeTax, ApplyToAllPref atapPref) {

	BOOL bUseOnlyPracticeFee = FALSE, bUseOnlyOutsideFee = FALSE;
	if (m_EntryType != 1) {
		//its a quote, check our preference
		if (atapPref == atapPracticeFeesOnly) {
			bUseOnlyPracticeFee = TRUE;
		}
	}

	return GetLineTotal(pItem->UnitCost, pItem->OthrUnitCost, pItem->Quantity, 
		pItem->Multiplier1, pItem->Multiplier2, pItem->Multiplier3, pItem->Multiplier4,
		pItem->TaxRate1, pItem->TaxRate2, pItem->DiscountList, 
		bIncludeTax, bUseOnlyPracticeFee, bUseOnlyOutsideFee);
}
