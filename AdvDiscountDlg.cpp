// AdvDiscountDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvDiscountDlg.h"
#include "BillingRc.h"
#include "BillingDlg.h"

// (j.gruber 2009-03-31 13:11) - PLID 33356 - created for

// CAdvDiscountDlg dialog

enum DiscountListColumns {
	dlcLineID = 0,
	dlcID,
	dlcChargeDesc,
	dlcPercentOff,
	dlcDiscount,
};

IMPLEMENT_DYNAMIC(CAdvDiscountDlg, CNxDialog)

// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
// (j.jones 2011-08-24 08:41) - PLID 44868 - added a map for original/void charges
CAdvDiscountDlg::CAdvDiscountDlg(CMap<long, long, DiscountList*, DiscountList*> *mapDiscounts,
								 CMap<long, long, CString, CString> *mapChargeDescs,
								 CMap<long, long, BOOL, BOOL> *mapCorrectedCharges,
								 BOOL bIsBill,
								 CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvDiscountDlg::IDD, pParent)
{

	m_mapDiscounts = mapDiscounts;
	m_mapChargeDescs = mapChargeDescs;
	m_mapCorrectedCharges = mapCorrectedCharges;
	m_bIsBill = bIsBill;

}

CAdvDiscountDlg::~CAdvDiscountDlg()
{
}

void CAdvDiscountDlg::DoDataExchange(CDataExchange* pDX)
{

	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADV_DISCOUNT_DELETE_ALL, m_btnDelete);
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAdvDiscountDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CAdvDiscountDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CAdvDiscountDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_ADV_DISCOUNT_DELETE_ALL, &CAdvDiscountDlg::OnBnClickedAdvDiscountDeleteAll)
END_MESSAGE_MAP()


// CAdvDiscountDlg message handlers


BOOL CAdvDiscountDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnDelete.AutoSet(NXB_DELETE);

		m_pCategoryList = BindNxDataList2Ctrl(IDC_ADV_DISCOUNT_CATEGORY_LIST, false);
		m_pDiscountList = BindNxDataList2Ctrl(IDC_ADV_DISCOUNT_DISCOUNT_LIST, false);

		m_pCategoryList->DisplayColumn = "1";	

		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pCategoryList->GetColumn(1);
		pCol->SortPriority = 0;	

		// (j.gruber 2009-03-31 16:13) - PLID 33554 - if they don't have both edit percent and discount then grey out the button
		//assume they know the password
		if (m_bIsBill) {
			if ((GetCurrentUserPermissions(bioChargePercentOff) & (SPT___W_______)) && (GetCurrentUserPermissions(bioChargeAmountOff) & (SPT___W_______))) {
				GetDlgItem(IDC_ADV_DISCOUNT_DELETE_ALL)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_ADV_DISCOUNT_DELETE_ALL)->EnableWindow(FALSE);
			}
		}
		else {
			if ((GetCurrentUserPermissions(bioQuotePercentOff) & (SPT___W_______)) && (GetCurrentUserPermissions(bioQuoteAmountOff) & (SPT___W_______))) {
				GetDlgItem(IDC_ADV_DISCOUNT_DELETE_ALL)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_ADV_DISCOUNT_DELETE_ALL)->EnableWindow(FALSE);
			}
		}
	
		LoadCategoryList();	
	}NxCatchAll("Error in CAdvDiscountDlg::OnInitDialog");

	return TRUE;
}


void CAdvDiscountDlg::LoadCategoryList()
{

	try {
		CString strWhere;
		//pull all the categories that exist on this bill, inactive or otherwise

		//loop through the map and get the categories associated
		CMap<CString, LPCTSTR, CString, LPCTSTR> mapCategories;
		POSITION pos = m_mapDiscounts->GetStartPosition();
		long nLineID;
		DiscountList *pDiscountList;
		while (pos != NULL) {
			m_mapDiscounts->GetNextAssoc(pos, nLineID, pDiscountList);

			for (int i = 0; i < pDiscountList->aryDiscounts.GetCount(); i++) {
				stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);

				CString strID;
				CString strName;
				if (Disc.DiscountCategoryID.vt == VT_I4) {
					if (Disc.DiscountCategoryID.lVal == -2) {
						//its a coupon
						if (Disc.CouponID.vt == VT_I4) {
							strID = "-2&" + AsString(Disc.CouponID);
						}
						else {
							//its a coupon unspecified
							strName = "<Coupon> - <Unspecified>";
						}
					}
					else if (Disc.DiscountCategoryID.lVal == -1) {
						//its a custom discount category
						if (Disc.CustomDiscountDescription.vt == VT_BSTR) {
							strID = "-1";
							strName = Disc.CustomDiscountDescription;
						}
					}
					else if (Disc.DiscountCategoryID.lVal != -3) {
						//a regular discount category
						strID.Format("%li", Disc.DiscountCategoryID.lVal);
					}
					else {
						//it a no category
						strID = "-3";
						strName = "<No Category>";
					}

					//add the item to the map
					CString strTemp;
					if (! mapCategories.Lookup(strID, strTemp)) {
					
						//get the name of the category or coupon
						if (strName.IsEmpty()) {
							long nResult = strID.Find("&");
							if (nResult != -1) {
								//we have a coupon
								//get the coupon name
								ADODB::_RecordsetPtr rsCoupon = CreateParamRecordset("SELECT Description FROM CouponsT WHERE ID = {INT}", 
									atoi(strID.Right(strID.GetLength() - (nResult +1))));
								if (! rsCoupon->eof) {
									strName = "<Coupon> - " + AdoFldString(rsCoupon, "Description", "");
								}
							}
							else {
								//its a discount category
								ADODB::_RecordsetPtr rsDiscount = CreateParamRecordset("SELECT Description FROM DiscountCategoriesT WHERE ID = {INT}", atoi(strID));
								if (! rsDiscount->eof) {
									strName = AdoFldString(rsDiscount, "Description", "");
								}
							}
						}
						mapCategories.SetAt(strID, strName);
					}
				}
			}
		}

		//add a no category
		mapCategories.SetAt("-3", "<No Category>");


		//now loop through our map and add the Rows
		pos = mapCategories.GetStartPosition();
		CString strID;
		CString strName;
		while (pos != NULL) {
			
			mapCategories.GetNextAssoc(pos, strID, strName);

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pCategoryList->GetNewRow();
			if (pRow) {
				pRow->PutValue(0, _variant_t(strID));
				pRow->PutValue(1, _variant_t(strName));

				m_pCategoryList->AddRowSorted(pRow, NULL);
			}

		}

		//set selection to no category
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCategoryList->SetSelByColumn(0, _variant_t("-3"));
		if (pRow) {
			SelChosenAdvDiscountCategoryList(pRow);
		}
		else {
			ASSERT(FALSE);
		}
	}NxCatchAll("Error in CAdvDiscountDlg::LoadCategoryList()");
}

void CAdvDiscountDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}



void CAdvDiscountDlg::OnBnClickedCancel()
{
	OnCancel();
}
BEGIN_EVENTSINK_MAP(CAdvDiscountDlg, CNxDialog)
	ON_EVENT(CAdvDiscountDlg, IDC_ADV_DISCOUNT_DISCOUNT_LIST, 8, CAdvDiscountDlg::EditingStartingAdvDiscountDiscountList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CAdvDiscountDlg, IDC_ADV_DISCOUNT_DISCOUNT_LIST, 10, CAdvDiscountDlg::EditingFinishedAdvDiscountDiscountList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAdvDiscountDlg, IDC_ADV_DISCOUNT_CATEGORY_LIST, 16, CAdvDiscountDlg::SelChosenAdvDiscountCategoryList, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CAdvDiscountDlg::EditingStartingAdvDiscountDiscountList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	// (j.gruber 2009-03-31 15:47) - PLID 33554 - check permissions
	switch (nCol) {
		case dlcPercentOff:
			// (j.gruber 2009-03-31 15:35) - PLID 33554 - check permissions
			if (m_bIsBill) {

				long nLineID = VarLong(pRow->GetValue(dlcLineID));

				// (j.jones 2011-08-24 08:41) - PLID 44868 - disallow editing original/void charges
				BOOL bIsCorrected = FALSE;
				if(m_mapCorrectedCharges->Lookup(nLineID, bIsCorrected) && bIsCorrected) {
					//could be an original or void charge, but give the same message for each
					MessageBox("This charge has been voided, and is no longer editable. If a corrected charge was created, you can edit that charge in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
					*pbContinue = FALSE;
					m_pCategoryList->StopEditing(FALSE);
					return;
				}

				if (!CheckCurrentUserPermissions(bioChargePercentOff, sptWrite)) {
					*pbContinue = FALSE;
					m_pCategoryList->StopEditing(FALSE);
					return;
				}		
			}
			else {
				//its a quote
				if (!CheckCurrentUserPermissions(bioQuotePercentOff, sptWrite)) {
					*pbContinue = FALSE;
					m_pCategoryList->StopEditing(FALSE);
					return;
				}
			}


		break;

		case dlcDiscount:
			// (j.gruber 2009-03-31 15:35) - PLID 33554 - check permissions
			if (m_bIsBill) {

				long nLineID = VarLong(pRow->GetValue(dlcLineID));

				// (j.jones 2011-08-24 08:41) - PLID 44868 - disallow editing original/void charges
				BOOL bIsCorrected = FALSE;
				if(m_mapCorrectedCharges->Lookup(nLineID, bIsCorrected) && bIsCorrected) {
					//could be an original or void charge, but give the same message for each
					MessageBox("This charge has been voided, and is no longer editable. If a corrected charge exists, you can edit that charge in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
					*pbContinue = FALSE;
					m_pCategoryList->StopEditing(FALSE);
					return;
				}

				if (!CheckCurrentUserPermissions(bioChargeAmountOff, sptWrite)) {
					*pbContinue = FALSE;
					m_pCategoryList->StopEditing(FALSE);				
					return;
				}
			}
			else {
				if (!CheckCurrentUserPermissions(bioQuoteAmountOff, sptWrite)) {
					*pbContinue = FALSE;
					m_pCategoryList->StopEditing(FALSE);				
					return;
				}
			}		

		break;
	}
}

void CAdvDiscountDlg::EditingFinishedAdvDiscountDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if (bCommit) {

			//get the id out of our map
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {

				long nLineID = VarLong(pRow->GetValue(dlcLineID));
				DiscountList *pDiscountList;
				if (m_mapDiscounts->Lookup(nLineID, pDiscountList)) {

					//remove it from the map
					m_mapDiscounts->RemoveKey(nLineID);

					long nDiscountID = VarLong(pRow->GetValue(dlcID));

					for (int i=0; i < pDiscountList->aryDiscounts.GetCount(); i++) {
						stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);

						if (Disc.ID.lVal == nDiscountID) {

							pDiscountList->aryDiscounts.RemoveAt(i);

							if (nCol == dlcPercentOff) {
								if (varNewValue.vt == VT_I4) {
									Disc.PercentOff = varNewValue;
								}
							}
							else if (nCol == dlcDiscount) {
								if (varNewValue.vt == VT_CY) {
									Disc.Discount = varNewValue;
								}
							}

							//add it back to the array
							pDiscountList->aryDiscounts.Add(Disc);
						}

						//add it back to the map
						m_mapDiscounts->SetAt(nLineID, pDiscountList);
					}
				}
			}
		}
	}NxCatchAll("Error in CAdvDiscountDlg::EditingFinishedAdvDiscountDiscountList");
}

void CAdvDiscountDlg::OnBnClickedAdvDiscountDeleteAll()
{
	try {

		// (j.jones 2011-08-24 08:41) - PLID 44868 - disallow if any entry is an original/void charge
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountList->GetFirstRow();
			while(pRow) {

				long nLineID = VarLong(pRow->GetValue(dlcLineID));

				BOOL bIsCorrected = FALSE;
				if(m_mapCorrectedCharges->Lookup(nLineID, bIsCorrected) && bIsCorrected) {
					//could be an original or void charge, but give the same message for each
					MessageBox("At least one charge using this discount category has been corrected, and is no longer editable.\n"
						"You cannot delete all the discounts for this category.", "Practice", MB_ICONINFORMATION|MB_OK);
					return;
				}

				pRow = pRow->GetNextRow();
			}
		}

		//first get the current category selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCategoryList->CurSel;
		if (pRow == NULL ){
			MsgBox("Please select a category to delete");
			return;
		}	

		//get the ID
		CString strID = VarString(pRow->GetValue(0));
		CString strName = VarString(pRow->GetValue(1));

		//loop through the map looking for the discount to delete
		POSITION pos = m_mapDiscounts->GetStartPosition();
		DiscountList *pDiscountList;
		long nLineID;
		while (pos != NULL) {
			m_mapDiscounts->GetNextAssoc(pos, nLineID, pDiscountList);

			for (int i=0; i < pDiscountList->aryDiscounts.GetCount(); i++) {
				stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);

				long nResult = strID.Find("&");
				if (nResult != -1) {

					//its a coupon
					if (Disc.CouponID.vt == VT_I4) {
						long nCouponID = atoi(strID.Right(strID.GetLength() - (nResult + 1)));
						if (nCouponID == Disc.CouponID.lVal) {
							//it's a match, remove the discount
							pDiscountList->aryDiscounts.RemoveAt(i);						
						}
					}
				}
				else {
					if (strID == "-2") {

						//check for coupon unspecified 
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == -2) {
								
								if (Disc.CouponID.vt == VT_I4) {
									if (Disc.CouponID.lVal == -1) {
										//add the Row
										pDiscountList->aryDiscounts.RemoveAt(i);
									}
								}
								else {
									if (Disc.CouponID.vt == VT_NULL) {
										//add the row
										pDiscountList->aryDiscounts.RemoveAt(i);
									}
								}
							}
						}
					}
					else if (strID == "-3") {
						//no category
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == -3) {
								//add the row
								pDiscountList->aryDiscounts.RemoveAt(i);
							}
						}
						else {					
							//add it
							pDiscountList->aryDiscounts.RemoveAt(i);
						}
					}
					else if (strID == "-1") {
						//custom
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == -1) {
								if (Disc.CustomDiscountDescription.vt == VT_BSTR) {
									CString strTemp = Disc.CustomDiscountDescription.bstrVal;
									if (strName.CompareNoCase(strTemp) == 0) {
										pDiscountList->aryDiscounts.RemoveAt(i);
									}
								}
							}
						}
					}
					else {
						//normal category
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == atoi(strID)) {
								//add it
								pDiscountList->aryDiscounts.RemoveAt(i);
							}
						}
					}
				}
			}			
		}

		//clear the discount and category lists
		m_pDiscountList->Clear();
		m_pCategoryList->Clear();
		//reload the category list
		LoadCategoryList();

	}NxCatchAll("Error in CAdvDiscountDlg::OnBnClickedAdvDiscountDeleteAll()");
}

void CAdvDiscountDlg::SelChosenAdvDiscountCategoryList(LPDISPATCH lpRow)
{
	try {

		//clear the discount list
		m_pDiscountList->Clear();
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			//get the ID string
			CString strID = VarString(pRow->GetValue(0));
			CString strName = VarString(pRow->GetValue(1));		

			//now loop through the map and get all the discounts with this ID
			LoadDiscountsForCategory(strID, strName);
		}		

	}NxCatchAll("Error in CAdvDiscountDlg::SelChosenAdvDiscountCategoryList");

}


void CAdvDiscountDlg::LoadDiscountsForCategory(CString strID, CString strName) {

	try {
		POSITION pos = m_mapDiscounts->GetStartPosition();
		DiscountList *pDiscountList;
		long nLineID;
		while (pos != NULL) {
			m_mapDiscounts->GetNextAssoc(pos, nLineID, pDiscountList);

			for (int i=0; i < pDiscountList->aryDiscounts.GetCount(); i++) {
				stDiscount Disc = pDiscountList->aryDiscounts.GetAt(i);

				long nResult = strID.Find("&");
				if (nResult != -1) {

					//its a coupon
					if (Disc.CouponID.vt == VT_I4) {
						long nCouponID = atoi(strID.Right(strID.GetLength() - (nResult + 1)));
						if (nCouponID == Disc.CouponID.lVal) {
							//it's a match, add the row
							AddRowToDatalist(nLineID, Disc);						
						}
					}
				}
				else {
					if (strID == "-2") {

						//check for coupon unspecified 
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == -2) {
								
								if (Disc.CouponID.vt == VT_I4) {
									if (Disc.CouponID.lVal == -1) {
										//add the Row
										AddRowToDatalist(nLineID, Disc);
									}
								}
								else {
									if (Disc.CouponID.vt == VT_NULL) {
										//add the row
										AddRowToDatalist(nLineID, Disc);
									}
								}
							}
						}
					}
					else if (strID == "-3") {
						//no category
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == -3) {
								//add the row
								AddRowToDatalist(nLineID, Disc);
							}
						}
						else {					
							//add it
							AddRowToDatalist(nLineID, Disc);						
						}
					}
					else if (strID == "-1") {
						//custom
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == -1) {
								if (Disc.CustomDiscountDescription.vt == VT_BSTR) {
									CString strTemp = Disc.CustomDiscountDescription.bstrVal;
									if (strName.CompareNoCase(strTemp) == 0) {
										AddRowToDatalist(nLineID, Disc);
									}
								}
							}
						}
					}
					else {
						//normal category
						if (Disc.DiscountCategoryID.vt == VT_I4) {
							if (Disc.DiscountCategoryID.lVal == atoi(strID)) {
								//add it
								AddRowToDatalist(nLineID, Disc);
							}
						}
					}
				}
			}
		}

	}NxCatchAll("Error in CAdvDiscountDlg::LoadDiscountsForCategory");
}

void CAdvDiscountDlg::AddRowToDatalist(long nLineID, stDiscount Disc) {

	try {
		//first find the charge description
		CString strChargeDesc;
		if (m_mapChargeDescs->Lookup(nLineID, strChargeDesc)) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountList->GetNewRow();
			pRow->PutValue(dlcLineID, nLineID);
			pRow->PutValue(dlcID, Disc.ID.lVal);
			pRow->PutValue(dlcChargeDesc, _variant_t(strChargeDesc));
			if (Disc.PercentOff.vt == VT_I4) {
				pRow->PutValue(dlcPercentOff, Disc.PercentOff.lVal);
			}

			if (Disc.Discount.vt == VT_CY) {
				pRow->PutValue(dlcDiscount, Disc.Discount);
			}

			// (j.jones 2011-08-24 08:41) - PLID 44868 - color read-only charges gray
			BOOL bIsCorrected = FALSE;
			if(m_mapCorrectedCharges->Lookup(nLineID, bIsCorrected) && bIsCorrected) {
				pRow->PutForeColor(CORRECTED_CHARGE_FOREGROUND_COLOR);
			}

			m_pDiscountList->AddRowAtEnd(pRow, NULL);
		}
		else {
			ASSERT(FALSE);
		}
	}NxCatchAll("Error in CAdvDiscountDlg::AddRowToDatalist");
	
}