// PendingQuotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PendingQuotesDlg.h"
#include "PaymentDlg.h"
#include "GlobalFinancialUtils.h"
#include "nxmessagedef.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ROW_BLANK		0
#define ROW_HEADER		1
#define ROW_COST		2
#define ROW_OUTSIDE_FEES		3 // (j.gruber 2010-02-02 12:43) - PLID 33784 -  added outside fees
#define ROW_BALANCE		4
#define ROW_PREPAYS		5
#define ROW_COMMANDS	6
#define ROW_SEPARATOR	7

#define	COLUMN_ID		0
#define	COLUMN_LABEL	1
#define	COLUMN_VALUE	2
#define COLUMN_ROWTYPE	3

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CPendingQuotesDlg dialog

using namespace ADODB;

CPendingQuotesDlg::CPendingQuotesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPendingQuotesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPendingQuotesDlg)
		m_pParent = pParent;
		m_nID = CPendingQuotesDlg::IDD;
		m_DisableRefCount = 0;
		m_bIsScreenEnabled = TRUE;
		m_bShowAll = FALSE;
	//}}AFX_DATA_INIT
}


void CPendingQuotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPendingQuotesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPendingQuotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPendingQuotesDlg)
	ON_BN_CLICKED(IDC_SHOW_ALL_QUOTES, OnShowAllQuotes)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPendingQuotesDlg message handlers

BOOL CPendingQuotesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_List = BindNxDataListCtrl(this,IDC_LIST,GetRemoteData(),false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPendingQuotesDlg::OnShowAllQuotes() 
{
	if(m_bShowAll) {
		m_bShowAll = FALSE;
		GetDlgItem(IDC_SHOW_ALL_QUOTES)->SetWindowText("Include Billed Quotes");
	}
	else {
		m_bShowAll = TRUE;
		GetDlgItem(IDC_SHOW_ALL_QUOTES)->SetWindowText("Hide Billed Quotes");
	}

	m_List->Clear();
	FillList();
}

void CPendingQuotesDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if(bShow) {

		if(m_bShowAll)
			GetDlgItem(IDC_SHOW_ALL_QUOTES)->SetWindowText("Hide Billed Quotes");
		else
			GetDlgItem(IDC_SHOW_ALL_QUOTES)->SetWindowText("Include Billed Quotes");

		m_List->Clear();
		FillList();
	}
	else
		m_List->Clear();
}

void CPendingQuotesDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
}

void CPendingQuotesDlg::FillList()
{
	IRowSettingsPtr pRow;

	DisableQuotesScreen();

	try {

		CString str;

		if(m_bShowAll) {
			//show all quotes for this patient

			// (j.jones 2007-07-31 14:49) - PLID 24808 - billed quotes need to use MaxBillID, the most recent non-deleted bill referencing the quote, and the bill date
			// (j.jones 2007-08-10 09:54) - PLID 27043 - this screen no longer shows packages, only non-package quotes
			// (j.gruber 2009-03-18 12:40) - PLID 33574 - discount structure
			// (d.thompson 2009-08-18) - PLID 16758 - Added 'Expired' flag
			// (j.gruber 2010-02-02 12:39) - PLID 33784 - include other fees
			// (j.jones 2011-06-17 14:09) - PLID 38347 - fixed quote total calculation to account for modifiers
			str.Format("SELECT [PatientBillsQ].ID, [PatientBillsQ].Description, [PatientBillsQ].Date, CASE WHEN HasBeenBilled Is Null THEN 0 ELSE 1 END AS HasBeenBilled, "
					"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
					"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
					"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
					")),2)) AS Total, "
					"Sum(Round(Convert(money,((([OthrBillFee]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR Amount > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
					"(([OthrBillFee]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR Amount > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
					"(([OthrBillFee]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR Amount > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
					")),2)) AS OutsideFee, "
					" Coalesce(HasBeenBilled, 0) AS MaxBillID, (SELECT Date FROM BillsT WHERE ID = Coalesce(HasBeenBilled, 0)) AS DateBilled, "
					"convert(bit, CASE WHEN PatientBillsQ.UseExp = 1 THEN CASE WHEN DATEADD(d, PatientBillsQ.ExpDays, PatientBillsQ.Date) < getdate() then 1 else 0 end ELSE 0 END) AS Expired "
					"FROM ((SELECT BillsT.*, (SELECT Max(BillID) FROM BilledQuotesT WHERE BilledQuotesT.QuoteID = BillsT.ID AND BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) AS HasBeenBilled FROM BillsT WHERE (((BillsT.PatientID)=%li) AND ((BillsT.Deleted)=0)) AND BillsT.Active = 1) AS PatientBillsQ LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"WHERE (((LineItemT.PatientID)=%li) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))) AS PatientChargesQ ON [PatientBillsQ].ID = [PatientChargesQ].BillID) "
					"LEFT JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
					"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID  "
					"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
					"WHERE ((([PatientBillsQ].EntryType)=2)) AND PatientBillsQ.ID NOT IN (SELECT QuoteID FROM PackagesT) "
					"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, PatientBillsQ.HasBeenBilled, "
					"convert(bit, CASE WHEN PatientBillsQ.UseExp = 1 THEN CASE WHEN DATEADD(d, PatientBillsQ.ExpDays, PatientBillsQ.Date) < getdate() then 1 else 0 end ELSE 0 END)",GetActivePatientID(),GetActivePatientID());
		}
		else {
			//only show unbilled quotes

			// (j.jones 2007-07-31 14:49) - PLID 24808 - unbilled quotes do not need to use MaxBillID or DateBilled, but they still need to be fields
			// (j.jones 2007-08-10 09:54) - PLID 27043 - this screen no longer shows packages, only non-package quotes
			// (j.gruber 2009-03-18 12:44) - PLID 33574 - discount structure
			// (d.thompson 2009-08-18) - PLID 16758 - Added 'Expired' flag
			// (j.gruber 2010-02-02 12:46) - PLID 33784 - Ourside Fees
			// (j.jones 2011-06-17 14:09) - PLID 38347 - fixed quote total calculation to account for modifiers
			str.Format("SELECT [PatientBillsQ].ID, [PatientBillsQ].Description, [PatientBillsQ].Date, CASE WHEN HasBeenBilled Is Null THEN 0 ELSE 1 END AS HasBeenBilled, "
					"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
					"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
					"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
					")),2)) AS Total, "
					"Sum(Round(Convert(money,((([OthrBillFee]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR Amount > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
					"(([OthrBillFee]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR Amount > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
					"(([OthrBillFee]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (OthrBillFee = 0 OR  Amount > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
					")),2)) AS OutsideFee, "
					" Coalesce(HasBeenBilled, 0) AS MaxBillID, NULL AS DateBilled, "
					"convert(bit, CASE WHEN PatientBillsQ.UseExp = 1 THEN CASE WHEN DATEADD(d, PatientBillsQ.ExpDays, PatientBillsQ.Date) < getdate() then 1 else 0 end ELSE 0 END) AS Expired "
					"FROM ((SELECT BillsT.*, (SELECT Max(BillID) FROM BilledQuotesT WHERE BilledQuotesT.QuoteID = BillsT.ID AND BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) AS HasBeenBilled FROM BillsT WHERE (((BillsT.PatientID)=%li) AND ((BillsT.Deleted)=0)) AND BillsT.Active = 1) AS PatientBillsQ LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"WHERE (((LineItemT.PatientID)=%li) AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))) AS PatientChargesQ ON [PatientBillsQ].ID = [PatientChargesQ].BillID) "
					"LEFT JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
					"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID  "
					" LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID  "
					"WHERE [PatientBillsQ].EntryType=2 AND (HasBeenBilled = 0 OR HasBeenBilled Is Null) AND PatientBillsQ.ID NOT IN (SELECT QuoteID FROM PackagesT) "
					"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, PatientBillsQ.HasBeenBilled, "
					"convert(bit, CASE WHEN PatientBillsQ.UseExp = 1 THEN CASE WHEN DATEADD(d, PatientBillsQ.ExpDays, PatientBillsQ.Date) < getdate() then 1 else 0 end ELSE 0 END)",GetActivePatientID(),GetActivePatientID());
		}

		_RecordsetPtr rs = CreateRecordset(str);

		if(rs->eof) {
			InsertBlankRow(-1);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)-1);
			if(m_bShowAll)
				pRow->PutValue(COLUMN_LABEL,_bstr_t("<No Available Quotes>"));
			else
				pRow->PutValue(COLUMN_LABEL,_bstr_t("<No Unbilled Quotes>"));
			pRow->PutValue(COLUMN_VALUE,_bstr_t(""));			
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_HEADER);
			pRow->PutCellForeColor(COLUMN_LABEL,RGB(192,0,0));
			m_List->AddRow(pRow);
		}

		while(!rs->eof) {

			long QuoteID;
			COleCurrency cyTotal, cyRemBalance, cyPrePays, cyOutsideFees;
			CString strDesc, strDate;
			COleDateTime dtQuoteDate;
			_variant_t var;

			//load quote info

			var = rs->Fields->Item["ID"]->Value;
			if(var.vt == VT_I4)
				QuoteID = var.lVal;
			else
				QuoteID = -1; //not likely to ever happen, but hey, gotta be safe

			var = rs->Fields->Item["Description"]->Value;
			if(var.vt == VT_BSTR)
				strDesc = CString(var.bstrVal);
			else
				strDesc = "<No Description>";

			var = rs->Fields->Item["Date"]->Value;
			if(var.vt == VT_DATE) {
				dtQuoteDate = var.date;
				strDate = FormatDateTimeForInterface(dtQuoteDate, NULL, dtoDate);
			}
			else
				strDate = "";

			var = rs->Fields->Item["Total"]->Value;
			if(var.vt == VT_CY)
				cyTotal = var.cyVal;
			else
				cyTotal = COleCurrency(0,0);

			
			// (j.gruber 2010-02-02 12:50) - PLID 33784 - outside fees
			var = rs->Fields->Item["OutsideFee"]->Value;
			if(var.vt == VT_CY)
				cyOutsideFees = var.cyVal;
			else
				cyOutsideFees = COleCurrency(0,0);

			// (d.thompson 2009-08-18) - PLID 16758 - Is it expired?
			BOOL bExpired = AdoFldBool(rs, "Expired");

			cyPrePays = CalculatePrePayments(GetActivePatientID(), QuoteID, GetRemotePropertyInt("IncludeAllPrePaysInPopUps", 1, 0, "<None>", TRUE) == 1);

			// (j.jones 2007-07-31 14:47) - PLID 24808 - for remaining balance, show the bill balance if the quote is on
			// a bill (if on multiple bills, which would be atypical, use the newest non-deleted bill ID)
			long nMaxBillID = AdoFldLong(rs, "MaxBillID",0);
			if(nMaxBillID > 0) {
				COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
				GetBillTotals(nMaxBillID, GetActivePatientID(), &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
				cyRemBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;
			}
			else {
				//normal quote calculation: total - prepayments
				cyRemBalance = cyTotal - cyPrePays;
			}
			
			if(cyRemBalance < COleCurrency(0,0)) {
				cyRemBalance = COleCurrency(0,0);
			}

			InsertBlankRow(QuoteID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)QuoteID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t(strDesc));
			pRow->PutValue(COLUMN_VALUE,_bstr_t(strDate));			
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_HEADER);
			pRow->PutCellForeColor(COLUMN_LABEL,RGB(0,0,192));
			// (d.thompson 2009-08-18) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(QuoteID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)QuoteID);
			// (j.gruber 2010-02-02 12:49) - PLID 33784 - changed to Practice Fee
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Practice Fees:"));
			pRow->PutValue(COLUMN_VALUE,_variant_t(cyTotal));			
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_COST);
			// (d.thompson 2009-08-18) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(QuoteID);
			
			// (j.gruber 2010-02-02 12:49) - PLID 33784 - added outside fee
			if (cyOutsideFees > COleCurrency(0,0)) {
				pRow = m_List->GetRow(-1);
				pRow->PutValue(COLUMN_ID,(long)QuoteID);
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Outside Fees:"));
				pRow->PutValue(COLUMN_VALUE,_variant_t(cyOutsideFees));			
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_OUTSIDE_FEES);
				// (d.thompson 2009-08-18) - PLID 16758 - Color expired quotes
				if(bExpired) {
					pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
				}
				m_List->AddRow(pRow);

				InsertBlankRow(QuoteID);
			}

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)QuoteID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Remaining Balance:"));
			pRow->PutValue(COLUMN_VALUE,_variant_t(cyRemBalance));
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_BALANCE);
			// (d.thompson 2009-08-18) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(QuoteID);			

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)QuoteID);

			// (j.jones 2007-07-31 14:55) - PLID 24808 - if billed, don't show the available prepayments,
			// instead show the date billed
			if(nMaxBillID > 0) {
				//it is a billed quote
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Date Billed:"));
				COleDateTime dtBillDate = AdoFldDateTime(rs, "DateBilled");
				pRow->PutValue(COLUMN_VALUE,_bstr_t(FormatDateTimeForInterface(dtBillDate, dtoDate)));
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_PREPAYS);
			}
			else {
				//normal, unbilled quote
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Available PrePayments:"));
				pRow->PutValue(COLUMN_VALUE,_variant_t(cyPrePays));
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_PREPAYS);
				if(cyPrePays >= cyTotal)
					pRow->PutCellForeColor(COLUMN_VALUE,RGB(0,192,0));
				else
					pRow->PutCellForeColor(COLUMN_VALUE,RGB(192,0,0));
			}
			// (d.thompson 2009-08-18) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(QuoteID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)QuoteID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Make a PrePayment"));
			pRow->PutCellLinkStyle(1,dlLinkStyleTrue);
			pRow->PutValue(COLUMN_VALUE,_bstr_t("Bill This Quote"));
			pRow->PutCellLinkStyle(2,dlLinkStyleTrue);
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_COMMANDS);
			pRow->PutForeColor(RGB(0,0,192));
			m_List->AddRow(pRow);

			InsertBlankRow(QuoteID);

			rs->MoveNext();

			if(!rs->eof)
				InsertSeparatorLine(QuoteID);
		}

		rs->Close();

	}NxCatchAll("Error in FillList");

	EnableQuotesScreen();
}

void CPendingQuotesDlg::InsertBlankRow(long QuoteID)
{
	IRowSettingsPtr pRow;
	pRow = m_List->GetRow(-1);
	pRow->PutValue(COLUMN_ID,(long)QuoteID);
	pRow->PutValue(COLUMN_LABEL,_bstr_t(""));
	pRow->PutValue(COLUMN_VALUE,_bstr_t(""));
	pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_BLANK);
	m_List->AddRow(pRow);
}

void CPendingQuotesDlg::InsertSeparatorLine(long QuoteID)
{
	IRowSettingsPtr pRow;
	pRow = m_List->GetRow(-1);
	pRow->PutValue(COLUMN_ID,(long)QuoteID);
	pRow->PutValue(COLUMN_LABEL,_bstr_t(""));
	pRow->PutValue(COLUMN_VALUE,_bstr_t(""));
	pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_SEPARATOR);	
	pRow->PutBackColor(GetSysColor(COLOR_3DFACE));
	m_List->AddRow(pRow);
}

void CPendingQuotesDlg::Requery()
{
	DisableQuotesScreen();

	long TopRow = m_List->TopRowIndex;	

	m_List->Clear();
	FillList();

	m_List->TopRowIndex = TopRow;

	EnableQuotesScreen();
}

//use these functions to cleanly redraw the screen, like the financial tab
void CPendingQuotesDlg::DisableQuotesScreen() {

	m_DisableRefCount++;

	if(!m_bIsScreenEnabled)
		return;
	else {

		m_List->SetRedraw(FALSE);
	
		m_bIsScreenEnabled = FALSE;
	}
}

void CPendingQuotesDlg::EnableQuotesScreen() {

	m_DisableRefCount--;

	if(m_DisableRefCount > 0 || m_bIsScreenEnabled) {
		return;
	}
	else {
		
		m_List->SetRedraw(TRUE);

		m_bIsScreenEnabled = TRUE;
	}
}

BOOL CPendingQuotesDlg::Create() {

	return CDialog::Create(m_nID, m_pParent);
}

BEGIN_EVENTSINK_MAP(CPendingQuotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPendingQuotesDlg)
	ON_EVENT(CPendingQuotesDlg, IDC_LIST, 19 /* LeftClick */, OnLeftClickList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPendingQuotesDlg::OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1)
		return;

	if(m_List->GetValue(nRow,COLUMN_ROWTYPE).lVal != ROW_COMMANDS)
		return;

	long QuoteID = m_List->GetValue(nRow,COLUMN_ID).lVal;

	if(nCol == COLUMN_LABEL) {
		MakePrePayment(QuoteID);
	}
	else if(nCol == COLUMN_VALUE) {
		BillQuote(QuoteID);
	}	
}

void CPendingQuotesDlg::MakePrePayment(long QuoteID)
{
	try {

		COleCurrency cyRemBalance;

		//this is more exact than searching the datalist
		// (j.jones 2007-08-10 09:54) - PLID 27043 - this screen no longer shows packages, only non-package quotes
		// (j.gruber 2009-03-18 12:45) - PLID 33574 - discount structure
		// (j.jones 2011-06-17 14:09) - PLID 38347 - fixed quote total calculation to account for modifiers
		_RecordsetPtr rs = CreateRecordset("SELECT "
						"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
						"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
						"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
						")),2)) AS Total "
						"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID  "
						"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
						"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
						"AND LineItemT.Type = 11 AND BillsT.ID NOT IN (SELECT QuoteID FROM PackagesT) "
						"AND BillsT.ID = %li ",QuoteID);

		if(rs->eof) {
			rs->Close();
			AfxMessageBox("This quote no longer exists. The list will now be reloaded.");
			m_List->Clear();
			FillList();
			return;
		}

		if(rs->Fields->Item["Total"]->Value.vt == VT_CY)
			cyRemBalance = rs->Fields->Item["Total"]->Value.cyVal;
		else
			cyRemBalance = COleCurrency(0,0);

		rs->Close();

		cyRemBalance -= CalculatePrePayments(GetActivePatientID(), QuoteID, false);

		if(UserPermission(NewPayment)) {
			CPaymentDlg dlg(this);
			dlg.m_varBillID = (long)QuoteID;
			dlg.m_bIsPrePayment = TRUE;
			dlg.m_cyFinalAmount = cyRemBalance;
			dlg.m_QuoteID = QuoteID;

			if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
				Requery();

				//the package message is re-used here
				m_pParent->PostMessage(NXM_POST_PACKAGE_PAYMENT);
			}
		}

	}NxCatchAll("Error creating prepayment.");
}

void CPendingQuotesDlg::BillQuote(long QuoteID)
{	
	//this is more exact than searching the datalist
	// (j.jones 2007-08-10 09:54) - PLID 27043 - this screen no longer shows packages, only non-package quotes
	_RecordsetPtr rs = CreateRecordset("SELECT ID FROM BillsT WHERE Deleted = 0 AND ID NOT IN (SELECT QuoteID FROM PackagesT) AND ID = %li",QuoteID);

	if(rs->eof) {
		rs->Close();
		AfxMessageBox("This quote no longer exists. The list will now be reloaded.");
		m_List->Clear();
		FillList();
		return;
	}
	rs->Close();

	//bill this quote (using the BILL_PACKAGE message)
	m_pParent->PostMessage(NXM_BILL_PACKAGE, QuoteID, 0);
}