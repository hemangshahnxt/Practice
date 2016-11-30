// BillDiscountOverviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillDiscountOverviewDlg.h"
#include "InternationalUtils.h"

// CBillDiscountOverviewDlg dialog

// (j.jones 2010-06-10 17:42) - PLID 39109 - created

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum ChargeListColumns {

	clcID = 0,
	clcParentID,
	clcChargeID,
	clcLineID,
	clcDescription,
	clcDate,
	clcAmount,
	clcQuantity,
	clcDiscountTotal,
	clcTotal,
};

IMPLEMENT_DYNAMIC(CBillDiscountOverviewDlg, CNxDialog)

CBillDiscountOverviewDlg::CBillDiscountOverviewDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillDiscountOverviewDlg::IDD, pParent)
{
	m_nBillID = -1;
	m_nDefChargeID = -1;
}

void CBillDiscountOverviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE_DISCOUNT_OVERVIEW_DLG, m_btnClose);
}


BEGIN_MESSAGE_MAP(CBillDiscountOverviewDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE_DISCOUNT_OVERVIEW_DLG, OnBtnClose)
END_MESSAGE_MAP()

// CBillDiscountOverviewDlg message handlers

BOOL CBillDiscountOverviewDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);

		m_ChargeList = BindNxDataList2Ctrl(IDC_CHARGE_DISCOUNT_LIST, false);

		//manually add rows, in part so we can properly format the discount amount internationally,
		//and in part so we full control over what is expanded when the screen is initially displayed

		_RecordsetPtr rs = CreateParamRecordset(""
			//charges
			"SELECT BillsT.Description AS BillDesc, BillsT.Date, Last + ', ' + First + ' ' + Middle AS PatientName, "
			"LineItemT.ID, ChargesT.LineID, LineItemT.Description AS ChargeDesc, ChargesT.ServiceDateFrom, LineItemT.Amount, ChargesT.Quantity, "
			"Convert(money,SUM(Round(Convert(money, ((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 0 ELSE [TotalPercentOff] END)/100)+(CASE WHEN([TotalDiscount] Is Null) THEN 0 ELSE [TotalDiscount] END)))),2))) AS PreTaxDiscountTotal, "
			"dbo.GetChargeTotal(LineItemT.ID) AS Total "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"INNER JOIN PersonT ON BillsT.PatientID = PersonT.ID "
			"LEFT JOIN (SELECT ChargeID, Sum(PercentOff) as TotalPercentOff, Sum(Discount) AS TotalDiscount FROM ChargeDiscountsT WHERE Deleted = 0 GROUP BY ChargeID) ChargeTotalDiscountsT ON ChargesT.ID = ChargeTotalDiscountsT.ChargeID "
			"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} "
			"GROUP BY BillsT.Date, BillsT.Description, PersonT.Last, PersonT.First, PersonT.Middle, "
			"LineItemT.Deleted, LineItemT.ID, ChargesT.LineID, LineItemT.Description, ChargesT.ServiceDateFrom, LineItemT.Amount, ChargesT.Quantity "
			"ORDER BY ChargesT.LineID "
			""
			//discounts
			"SELECT ChargeDiscountsT.ChargeID, ChargeDiscountsT.PercentOff, ChargeDiscountsT.Discount, "
			"Coalesce(CASE WHEN ChargeDiscountsT.DiscountCategoryID Is Null OR ChargeDiscountsT.DiscountCategoryID = -3 THEN '<None Selected>' "
			"			WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN '<Custom> - ' + ChargeDiscountsT.CustomDiscountDesc "
			"			WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN '<Coupon> - ' + Coalesce(CouponsT.Description, 'Unspecified') "
			"			ELSE DiscountCategoriesT.Description END, '<None Selected>') AS CategoryName "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ChargeDiscountsT ON ChargesT.ID = ChargeDiscountsT.ChargeID "
			"LEFT JOIN DiscountCategoriesT ON ChargeDiscountsT.DiscountCategoryID = DiscountCategoriesT.ID "
			"LEFT JOIN CouponsT ON ChargeDiscountsT.CouponID = CouponsT.ID "
			"WHERE LineItemT.Deleted = 0 AND ChargesT.BillID = {INT} AND ChargeDiscountsT.Deleted = 0 "
			"ORDER BY Coalesce(CASE WHEN ChargeDiscountsT.DiscountCategoryID Is Null OR ChargeDiscountsT.DiscountCategoryID = -3 THEN '<None Selected>' "
			"			WHEN ChargeDiscountsT.DiscountCategoryID = -1 THEN '<Custom> - ' + ChargeDiscountsT.CustomDiscountDesc "
			"			WHEN ChargeDiscountsT.DiscountCategoryID = -2 THEN '<Coupon> - ' + Coalesce(CouponsT.Description, 'Unspecified') "
			"			ELSE DiscountCategoriesT.Description END, '<None Selected>')",
			m_nBillID, m_nBillID);

		if(!rs->eof) {
			//load the bill information
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strDescription = AdoFldString(rs, "BillDesc", "");
			CString strPatientName = AdoFldString(rs, "PatientName", "");
			strPatientName.TrimRight();

			CString strWindowText;
			strWindowText.Format("Discount Overview for %s - Bill: %s (%s)", strPatientName, strDescription, FormatDateTimeForInterface(dtDate, NULL, dtoDate));
			SetWindowText(strWindowText);
		}

		//charges
		while(!rs->eof) {
			IRowSettingsPtr pRow = m_ChargeList->GetNewRow();

			long nChargeID = AdoFldLong(rs, "ID");

			pRow->PutValue(clcID, nChargeID);
			pRow->PutValue(clcParentID, g_cvarNull);
			pRow->PutValue(clcChargeID, nChargeID);
			pRow->PutValue(clcLineID, rs->Fields->Item["LineID"]->Value);
			pRow->PutValue(clcDescription, rs->Fields->Item["ChargeDesc"]->Value);
			pRow->PutValue(clcDate, rs->Fields->Item["ServiceDateFrom"]->Value);
			pRow->PutValue(clcAmount, rs->Fields->Item["Amount"]->Value);
			pRow->PutValue(clcQuantity, rs->Fields->Item["Quantity"]->Value);
			pRow->PutValue(clcDiscountTotal, rs->Fields->Item["PreTaxDiscountTotal"]->Value);
			pRow->PutValue(clcTotal, rs->Fields->Item["Total"]->Value);

			//this is a top-level row
			m_ChargeList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}

		rs = rs->NextRecordset(NULL);

		//discounts
		while(!rs->eof) {
			IRowSettingsPtr pRow = m_ChargeList->GetNewRow();

			long nChargeID = AdoFldLong(rs, "ChargeID");

			IRowSettingsPtr pParentRow = m_ChargeList->FindByColumn(clcChargeID, (long)nChargeID, NULL, FALSE);
			if(pParentRow == NULL) {
				ThrowNxException("Found a discount for missing charge ID %li!", nChargeID);
			}

			long nPercentOff = AdoFldLong(rs, "PercentOff", 0);
			COleCurrency cyDiscount = AdoFldCurrency(rs, "Discount", COleCurrency(0,0));
			CString strDiscountCategory = AdoFldString(rs, "CategoryName", "");
			strDiscountCategory.TrimLeft();
			strDiscountCategory.TrimRight();

			CString strDiscountDescription;
			if(nPercentOff != 0) {
				CString str;
				str.Format("Percent Off: %li%%", nPercentOff);
				strDiscountDescription += str;
			}
			if(cyDiscount != COleCurrency(0,0)) {
				CString str;
				str.Format("Discount: %s", FormatCurrencyForInterface(cyDiscount));
				if(!strDiscountDescription.IsEmpty()) {
					strDiscountDescription += ", ";
				}
				strDiscountDescription += str;
			}
			CString str;
			str.Format("Category: %s", strDiscountCategory);
			if(!strDiscountDescription.IsEmpty()) {
				strDiscountDescription += ", ";
			}
			strDiscountDescription += str;

			pRow->PutValue(clcID, g_cvarNull);
			pRow->PutValue(clcParentID, nChargeID);
			pRow->PutValue(clcChargeID, nChargeID);
			pRow->PutValue(clcLineID, g_cvarNull);
			pRow->PutValue(clcDescription, (LPCTSTR)strDiscountDescription);
			pRow->PutValue(clcDate, g_cvarNull);
			pRow->PutValue(clcAmount, g_cvarNull);
			pRow->PutValue(clcQuantity, g_cvarNull);
			pRow->PutValue(clcDiscountTotal, g_cvarNull);
			pRow->PutValue(clcTotal, g_cvarNull);

			//this is a child row
			m_ChargeList->AddRowSorted(pRow, pParentRow);

			//expand if this is our default charge or if we have no default charge
			if(m_nDefChargeID == -1 || m_nDefChargeID == nChargeID) {
				pParentRow->PutExpanded(VARIANT_TRUE);
			}

			rs->MoveNext();
		}
		rs->Close();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CBillDiscountOverviewDlg::OnBtnClose()
{
	try {

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}