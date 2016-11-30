// Packages.cpp : implementation file
//

#include "stdafx.h"
#include "Packages.h"
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

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

#define ROW_BLANK		0
#define ROW_HEADER		1
#define ROW_COST		2
#define ROW_UNBILLED	3
#define ROW_BALANCE		4
#define ROW_TOTAL_COUNT	5
#define ROW_REM_COUNT	6
#define ROW_PREPAYS		7
#define ROW_COMMANDS	8
#define ROW_USES_LABEL	9
#define ROW_MULTI_USES	10
#define ROW_SEPARATOR	11

#define	COLUMN_ID		0
#define	COLUMN_LABEL	1
#define	COLUMN_VALUE	2
#define COLUMN_ROWTYPE	3
#define COLUMN_OKTOBILL	4

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CPackages dialog


CPackages::CPackages(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPackages::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPackages)
		m_pParent = pParent;
		m_nID = CPackages::IDD;
		m_DisableRefCount = 0;
		m_bIsScreenEnabled = TRUE;
		m_bShowAll = FALSE;
	//}}AFX_DATA_INIT
}


void CPackages::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPackages)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPackages, CNxDialog)
	//{{AFX_MSG_MAP(CPackages)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SHOW_ALL, OnShowAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPackages message handlers

BOOL CPackages::Create()
{
	return CNxDialog::Create(m_nID, m_pParent);
}

void CPackages::OnCancel() 
{
	ShowWindow(SW_HIDE);
}

BOOL CPackages::OnInitDialog() 
{ 
	try {

		CNxDialog::OnInitDialog();
		
		m_List = BindNxDataListCtrl(this,IDC_LIST,GetRemoteData(),false);

		// (j.jones 2009-10-26 09:23) - PLID 32904 - added bulk caching
		g_propManager.CachePropertiesInBulk("CPackages", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PackageTaxEstimation' OR "
			"Name = 'IncludeAllPrePaysInPopUps' OR "
			"Name = 'DoNotBillNonPrepaidPackages' "
			")",
			_Q(GetCurrentUserName()));

	}NxCatchAll("Error in CPackages::OnInitDialog(");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPackages::Requery()
{
	DisablePackageScreen();

	long TopRow = m_List->TopRowIndex;	

	m_List->Clear();
	FillList();

	m_List->TopRowIndex = TopRow;

	EnablePackageScreen();
}

void CPackages::FillList()
{
	IRowSettingsPtr pRow;

	DisablePackageScreen();

	try {

		CString str, strFilter = "";

		if(!m_bShowAll) {
			//filter out completed packages
			//our query will cast the proper amount remaining into CurrentCount regardless of the Type
			strFilter = " AND CurrentCount > 0 ";
		}

		_variant_t varNull;
		varNull.vt = VT_NULL;

		_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
		// (d.thompson 2009-09-01) - PLID 16758 - Added exp date flag
		str.Format("SELECT PackagesT.*, BillsT.Description, BillsT.Date, "
			"	convert(bit, CASE WHEN BillsT.UseExp = 1 THEN CASE WHEN DATEADD(d, BillsT.ExpDays, BillsT.Date) < getdate() then 1 else 0 end ELSE 0 END) AS Expired "
			"FROM "
			"	(SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, OriginalCurrentAmount, Type, "
			"	(CASE WHEN Type = 1 THEN PackagesT.TotalCount WHEN Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
			"	(CASE WHEN Type = 1 THEN PackagesT.CurrentCount WHEN Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
			"	FROM PackagesT "
			"	LEFT JOIN "
			"		(SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
			"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
			"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
			"		GROUP BY BillID "
			"		) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
			"	) AS PackagesT "
			"INNER JOIN BillsT ON PackagesT.QuoteID = BillsT.ID WHERE "
			"BillsT.Deleted = 0 AND BillsT.PatientID = %li AND BillsT.Active = 1 %s ORDER BY BillsT.Date DESC", GetActivePatientID(), strFilter);

		_RecordsetPtr rs = CreateRecordset(str);

		if(rs->eof) {
			InsertBlankRow(-1);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)-1);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("<No Available Packages>"));
			pRow->PutValue(COLUMN_VALUE,_bstr_t(""));			
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_HEADER);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			pRow->PutCellForeColor(COLUMN_LABEL,RGB(192,0,0));
			m_List->AddRow(pRow);
		}

		while(!rs->eof) {

			// (d.thompson 2009-09-01) - PLID 16758 - Flag if expired
			BOOL bExpired = AdoFldBool(rs, "Expired");

			long PackageID, Type;
			double TotalCount, RemCount;
			COleCurrency cyTotalCostNoTax, cyTotalCostWithTax;
			COleCurrency cyRemBalanceNoTax, cyRemBalanceWithTax;
			COleCurrency cyUnBilledNoTax, cyUnBilledWithTax;
			COleCurrency cyPrePays, cyOneUse;
			CString strDesc, strDate;
			COleDateTime dtQuoteDate;
			_variant_t var;

			//load package info
			PackageID = AdoFldLong(rs, "QuoteID", -1);

			Type = AdoFldLong(rs, "Type", -1);

			strDesc = AdoFldString(rs, "Description","<No Description>");

			var = rs->Fields->Item["Date"]->Value;
			if(var.vt == VT_DATE) {
				dtQuoteDate = var.date;
				strDate = FormatDateTimeForInterface(dtQuoteDate, NULL, dtoDate);
			}
			else
				strDate = "";

			cyTotalCostNoTax = AdoFldCurrency(rs, "TotalAmount",COleCurrency(0,0));			

			cyUnBilledNoTax = AdoFldCurrency(rs, "CurrentAmount",COleCurrency(0,0));

			var = rs->Fields->Item["TotalCount"]->Value;
			if(var.vt == VT_I4)
				TotalCount = (double)VarLong(var);
			else if(var.vt == VT_R8)
				TotalCount = VarDouble(var);
			else
				TotalCount = 0.0;

			var = rs->Fields->Item["CurrentCount"]->Value;
			if(var.vt == VT_I4)
				RemCount = (double)VarLong(var);
			else if(var.vt == VT_R8)
				RemCount = VarDouble(var);
			else
				RemCount = 0.0;

			cyPrePays = CalculatePrePayments(GetActivePatientID(), PackageID, GetRemotePropertyInt("IncludeAllPrePaysInPopUps", 1, 0, "<None>", TRUE) == 1);

			// (j.jones 2008-02-14 09:24) - PLID 28794 - I combined the "remaining package balance" functions
			// together, since they can be handled with one function and return both values at once
			// (j.jones 2008-08-12 10:54) - PLID 24624 - this now requires the patient ID
			CalculateRemainingPackageBalance(PackageID, GetActivePatientID(), rs->Fields->Item["OriginalCurrentAmount"]->Value, cyRemBalanceNoTax, cyRemBalanceWithTax);
			
			BOOL bHasTax = FALSE;

			cyTotalCostWithTax = CalculateTotalPackageValueWithTax(PackageID);

			//if the balances are different, tax exists, so make sure we also grab the other amounts with tax
			if(cyTotalCostNoTax != cyTotalCostWithTax) {
				bHasTax = TRUE;
				
				// (j.jones 2008-02-14 09:24) - PLID 28794 - cyRemBalanceWithTax was calculated
				// earlier
				//cyRemBalanceWithTax = CalculateRemainingPackageBalanceWithTax(PackageID);

				cyUnBilledWithTax = CalculateRemainingPackageValueWithTax(PackageID);
			}
			else {
				// (j.jones 2008-02-14 09:24) - PLID 28794 - cyRemBalanceWithTax was calculated
				// earlier, so if we aren't using it, it needs reset to the no-tax value
				cyRemBalanceWithTax = cyRemBalanceNoTax;
				cyUnBilledWithTax = cyUnBilledNoTax;
			}

			//calculate the value of one instance of the package
			if(RemCount <= 1)
				//if there is 1 use left, it is the remaining balance
				//if there is 0 left, then the balance is 0 anyways
				if(bHasTax) {
					cyOneUse = cyUnBilledWithTax;
				}
				else {
					cyOneUse = cyUnBilledNoTax;
				}
			else {
				//one use is the package cost / package count
				if(bHasTax) {
					cyOneUse = (cyTotalCostWithTax/TotalCount);
				}
				else {
					cyOneUse = (cyTotalCostNoTax/TotalCount);
				}
				RoundCurrency(cyOneUse);
			}

			// (j.jones 2011-07-15 15:21) - PLID 38334 - if there are uses left,
			// but the unbilled amount is zero, then one use is also zero
			if(bHasTax) {
				if(cyUnBilledWithTax <= COleCurrency(0,0)) {
					cyOneUse = COleCurrency(0,0);
				}
			}
			else {
				if(cyUnBilledNoTax <= COleCurrency(0,0)) {
					cyOneUse = COleCurrency(0,0);
				}
			}

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t(strDesc));
			pRow->PutValue(COLUMN_VALUE,_bstr_t(strDate));			
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_HEADER);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			pRow->PutCellForeColor(COLUMN_LABEL,RGB(0,0,192));
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Package Cost:"));
			pRow->PutValue(COLUMN_VALUE,_variant_t(cyTotalCostNoTax));			
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_COST);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			if(bHasTax) {
				pRow = m_List->GetRow(-1);
				pRow->PutValue(COLUMN_ID,(long)PackageID);
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Cost With Tax:"));
				pRow->PutValue(COLUMN_VALUE,_variant_t(cyTotalCostWithTax));			
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_COST);
				pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
				// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
				if(bExpired) {
					pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
				}
				m_List->AddRow(pRow);
			}

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Unbilled Amount:"));
			pRow->PutValue(COLUMN_VALUE,_variant_t(cyUnBilledNoTax));
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_UNBILLED);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			if(bHasTax) {
				pRow = m_List->GetRow(-1);
				pRow->PutValue(COLUMN_ID,(long)PackageID);
				pRow->PutValue(COLUMN_LABEL,_bstr_t("With Tax:"));
				pRow->PutValue(COLUMN_VALUE,_variant_t(cyUnBilledWithTax));
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_UNBILLED);
				pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
				// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
				if(bExpired) {
					pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
				}
				m_List->AddRow(pRow);
			}

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Remaining Balance:"));
			pRow->PutValue(COLUMN_VALUE,_variant_t(cyRemBalanceNoTax));
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_BALANCE);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			if(bHasTax) {
				pRow = m_List->GetRow(-1);
				pRow->PutValue(COLUMN_ID,(long)PackageID);
				pRow->PutValue(COLUMN_LABEL,_bstr_t("With Tax:"));
				pRow->PutValue(COLUMN_VALUE,_variant_t(cyRemBalanceWithTax));
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_BALANCE);
				pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
				// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
				if(bExpired) {
					pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
				}
				m_List->AddRow(pRow);
			}

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			if(Type == 1) {
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Package Quantity:"));
			}
			else {
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Services Purchased:"));
			}
			pRow->PutValue(COLUMN_VALUE,(double)TotalCount);
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_TOTAL_COUNT);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			if(Type == 1) {
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Remaining Quantity:"));
			}
			else {
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Services Remaining:"));
			}
			pRow->PutValue(COLUMN_VALUE,(double)RemCount);
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_REM_COUNT);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(PackageID);			

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Available PrePayments:"));
			pRow->PutValue(COLUMN_VALUE,_variant_t(cyPrePays));
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_PREPAYS);
			pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
			if(cyPrePays >= cyOneUse) {
				pRow->PutCellForeColor(COLUMN_VALUE,RGB(0,192,0));
			}
			else {
				pRow->PutCellForeColor(COLUMN_VALUE,RGB(192,0,0));
			}
			// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
			if(bExpired) {
				pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
			}
			m_List->AddRow(pRow);

			InsertBlankRow(PackageID);

			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_ID,(long)PackageID);
			pRow->PutValue(COLUMN_LABEL,_bstr_t("Make a PrePayment"));
			pRow->PutCellLinkStyle(1,dlLinkStyleTrue);
			pRow->PutValue(COLUMN_VALUE,_bstr_t("Bill This Package"));
			pRow->PutCellLinkStyle(2,dlLinkStyleTrue);
			pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_COMMANDS);
			
			// (j.jones 2007-08-10 09:21) - PLID 23769 - cache the info. that we can bill this package,
			// base on their preferences

			long nDoNotBillNonPrepaidPackages = GetRemotePropertyInt("DoNotBillNonPrepaidPackages",0,0,"<None>",TRUE);
			if(nDoNotBillNonPrepaidPackages == 0) {
				//we aren't stopped from billing any package
				pRow->PutValue(COLUMN_OKTOBILL, varTrue);
			}
			else if(nDoNotBillNonPrepaidPackages == 1 && cyPrePays >= cyUnBilledWithTax) {
				//we have enough prepaid to cover the unbilled amount
				pRow->PutValue(COLUMN_OKTOBILL, varTrue);
			} 
			else if(nDoNotBillNonPrepaidPackages == 2 && cyPrePays >= cyOneUse) {
				//we have enough prepaid to cover one use
				pRow->PutValue(COLUMN_OKTOBILL, varTrue);
			}
			else {
				//we can't bill this package
				pRow->PutValue(COLUMN_OKTOBILL, varFalse);
			}
			pRow->PutForeColor(RGB(0,0,192));
			m_List->AddRow(pRow);

			InsertBlankRow(PackageID);

			//if a multi-use package, show the charges and their remaining quantities
			if(Type == 2) {
				pRow = m_List->GetRow(-1);
				pRow->PutValue(COLUMN_ID,(long)PackageID);
				pRow->PutValue(COLUMN_LABEL,_bstr_t("Services In Package"));
				pRow->PutValue(COLUMN_VALUE, _bstr_t("Uses Remaining"));
				pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_USES_LABEL);
				pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
				pRow->PutForeColor(RGB(92,92,92));
				m_List->AddRow(pRow);

				//InsertBlankRow(PackageID);

				// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
				_RecordsetPtr rsUses = CreateParamRecordset("SELECT Description, Quantity, PackageQtyRemaining "
					"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"WHERE Deleted = 0 AND BillID = {INT} "
					"AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) ",
					PackageID);
				while(!rsUses->eof) {

					CString strDescription = AdoFldString(rsUses, "Description","<No Description>");
					CString strUses;
					double dblQty = AdoFldDouble(rsUses, "Quantity",0.0);
					double dblQtyRem = AdoFldDouble(rsUses, "PackageQtyRemaining",0.0);
					strUses.Format("%g of %g remaining",dblQtyRem,dblQty);
						
					pRow = m_List->GetRow(-1);
					pRow->PutValue(COLUMN_ID,(long)PackageID);
					pRow->PutValue(COLUMN_LABEL,_bstr_t(strDescription));
					pRow->PutValue(COLUMN_VALUE, _bstr_t(strUses));
					pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_MULTI_USES);
					pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
					// (d.thompson 2009-09-01) - PLID 16758 - Color expired quotes
					if(bExpired) {
						pRow->PutForeColor(EXPIRED_QUOTE_FORECOLOR);
					}
					m_List->AddRow(pRow);

					//InsertBlankRow(PackageID);

					rsUses->MoveNext();
				}
				rsUses->Close();

				InsertBlankRow(PackageID);
			}

			rs->MoveNext();

			if(!rs->eof)
				InsertSeparatorLine(PackageID);
		}

		rs->Close();

	}NxCatchAll("Error in FillList");

	EnablePackageScreen();
}

void CPackages::InsertBlankRow(long QuoteID)
{
	_variant_t varNull;
	varNull.vt = VT_NULL;

	IRowSettingsPtr pRow;
	pRow = m_List->GetRow(-1);
	pRow->PutValue(COLUMN_ID,(long)QuoteID);
	pRow->PutValue(COLUMN_LABEL,_bstr_t(""));
	pRow->PutValue(COLUMN_VALUE,_bstr_t(""));
	pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_BLANK);
	pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
	m_List->AddRow(pRow);
}

void CPackages::InsertSeparatorLine(long QuoteID)
{
	_variant_t varNull;
	varNull.vt = VT_NULL;

	IRowSettingsPtr pRow;
	pRow = m_List->GetRow(-1);
	pRow->PutValue(COLUMN_ID,(long)QuoteID);
	pRow->PutValue(COLUMN_LABEL,_bstr_t(""));
	pRow->PutValue(COLUMN_VALUE,_bstr_t(""));
	pRow->PutValue(COLUMN_ROWTYPE,(long)ROW_SEPARATOR);	
	pRow->PutValue(COLUMN_OKTOBILL, varNull); //irrelevant on this row
	pRow->PutBackColor(GetSysColor(COLOR_3DFACE));
	m_List->AddRow(pRow);
}

BEGIN_EVENTSINK_MAP(CPackages, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPackages)
	ON_EVENT(CPackages, IDC_LIST, 19 /* LeftClick */, OnLeftClickList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPackages::OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags) 
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

		// (j.jones 2007-08-10 09:16) - PLID 23769 - if the preference is enabled to not bill non-prepaid packages,
		// check our previously calculated value to determine if it is allowed
		long nDoNotBillNonPrepaidPackages = GetRemotePropertyInt("DoNotBillNonPrepaidPackages",0,0,"<None>",TRUE);
		if(nDoNotBillNonPrepaidPackages > 0) {

			BOOL bOKToBill = VarBool(m_List->GetValue(nRow,COLUMN_OKTOBILL), FALSE);
			if(!bOKToBill) {
				//nope, not enough prepays

				CString str, strType;
				if(nDoNotBillNonPrepaidPackages == 1) {
					strType = "the full unbilled balance";
				}
				else {
					strType = "one use";
				}
				str.Format("This patient does not have enough available prepayments to cover %s of this package.\n"
					"Your preferences are set to disallow billing any package without enough prepayments to cover %s.",
					strType, strType);
				AfxMessageBox(str);
				return;
			}
		}

		BillPackage(QuoteID);
	}	
}

void CPackages::MakePrePayment(long QuoteID)
{
	COleCurrency cyRemBalance;
	
	if(IsRecordsetEmpty("SELECT BillsT.ID FROM BillsT "
						"INNER JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
						"WHERE BillsT.EntryType = 2 AND BillsT.Deleted = 0 AND BillsT.ID = %li ",QuoteID)) {
		AfxMessageBox("This package no longer exists. The list will now be reloaded.");
		m_List->Clear();
		FillList();
		return;
	}

	//this is more exact than searching the datalist
	cyRemBalance = CalculateRemainingPackageValueWithTax(QuoteID);

	cyRemBalance -= CalculatePrePayments(GetActivePatientID(), QuoteID, false);

	if(UserPermission(NewPayment)) {
		CPaymentDlg dlg(this);
		dlg.m_varBillID = (long)QuoteID;
		dlg.m_bIsPrePayment = TRUE;
		dlg.m_cyFinalAmount = cyRemBalance;
		dlg.m_QuoteID = QuoteID;

		if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
			Requery();

			m_pParent->PostMessage(NXM_POST_PACKAGE_PAYMENT);	
		}
	}
}

void CPackages::BillPackage(long QuoteID)
{
	double dblRemAmount;
	
	//this is more exact than searching the datalist
	// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
	_RecordsetPtr rs = CreateRecordset("SELECT CurrentCount FROM "
		"	(SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, Type, "
		"	(CASE WHEN Type = 1 THEN Convert(float,PackagesT.TotalCount) WHEN Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
		"	(CASE WHEN Type = 1 THEN Convert(float,PackagesT.CurrentCount) WHEN Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
		"	FROM PackagesT "
		"	LEFT JOIN "
		"		(SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
		"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
		"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
		"		GROUP BY BillID "
		"		) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
		"	) AS PackagesT "
		"WHERE QuoteID = %li",QuoteID);

	if(rs->eof) {
		rs->Close();
		AfxMessageBox("This package no longer exists. The list will now be reloaded.");
		m_List->Clear();
		FillList();
		return;
	}

	_variant_t var = rs->Fields->Item["CurrentCount"]->Value;
	if(var.vt == VT_I4)
		dblRemAmount = (double)VarLong(var,0);
	else if(var.vt == VT_R8)
		dblRemAmount = VarDouble(var,0.0);
	else
		dblRemAmount = 0.0;

	if(dblRemAmount <= 0.0) {
		AfxMessageBox("This package has been used up. Please go to the Quotes tab to delete this package or add more uses.");
		return;
	}

	rs->Close();

	//bill this package
	m_pParent->PostMessage(NXM_BILL_PACKAGE, QuoteID, 0);
}

void CPackages::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
	
	if(bShow) {

		if(m_bShowAll)
			GetDlgItem(IDC_SHOW_ALL)->SetWindowText("Hide Completed Packages");
		else
			GetDlgItem(IDC_SHOW_ALL)->SetWindowText("Show Completed Packages");

		m_List->Clear();
		FillList();
	}
	else
		m_List->Clear();
}

//use these functions to cleanly redraw the screen, like the financial tab
void CPackages::DisablePackageScreen() {

	m_DisableRefCount++;

	if(!m_bIsScreenEnabled)
		return;
	else {

		m_List->SetRedraw(FALSE);
	
		m_bIsScreenEnabled = FALSE;
	}
}

void CPackages::EnablePackageScreen() {

	m_DisableRefCount--;

	if(m_DisableRefCount > 0 || m_bIsScreenEnabled) {
		return;
	}
	else {
		
		m_List->SetRedraw(TRUE);

		m_bIsScreenEnabled = TRUE;
	}
}

void CPackages::OnShowAll() 
{
	if(m_bShowAll) {
		m_bShowAll = FALSE;
		GetDlgItem(IDC_SHOW_ALL)->SetWindowText("Show Completed Packages");
	}
	else {
		m_bShowAll = TRUE;
		GetDlgItem(IDC_SHOW_ALL)->SetWindowText("Hide Completed Packages");
	}

	m_List->Clear();
	FillList();
}
