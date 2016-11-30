// AllowedAmountSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AllowedAmountSearchDlg.h"
#include "InternationalUtils.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "FinancialRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

#define FILTER_ALL_DATES				1
#define FILTER_PAYMENT_SERVICE_DATE		2
#define FILTER_PAYMENT_INPUT_DATE		3
#define FILTER_CHARGE_SERVICE_DATE		4
#define FILTER_CHARGE_INPUT_DATE		5

#define DATE_COLUMN		3

/////////////////////////////////////////////////////////////////////////////
// CAllowedAmountSearchDlg dialog

// (j.jones 2006-11-29 15:51) - PLID 22293 - Created


CAllowedAmountSearchDlg::CAllowedAmountSearchDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAllowedAmountSearchDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAllowedAmountSearchDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAllowedAmountSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAllowedAmountSearchDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_SEARCH_ALLOWABLES, m_btnSearch);
	DDX_Control(pDX, IDC_BTN_PREVIEW_ALLOWABLES, m_btnPreview);
	DDX_Control(pDX, IDC_ALLOWABLE_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_ALLOWABLE_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_EDIT_ALLOWABLE_AMOUNT, m_nxeditEditAllowableAmount);
	DDX_Control(pDX, IDC_RADIO_SERVICE_CODES, m_radioServiceCodes);
	DDX_Control(pDX, IDC_RADIO_INV_ITEMS, m_radioInvItems);
	DDX_Control(pDX, IDC_SERVICE_CODE_LABEL, m_staticServiceCodeLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAllowedAmountSearchDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAllowedAmountSearchDlg)
	ON_BN_CLICKED(IDC_BTN_SEARCH_ALLOWABLES, OnBtnSearchAllowables)
	ON_BN_CLICKED(IDC_BTN_PREVIEW_ALLOWABLES, OnBtnPreviewAllowables)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO_SERVICE_CODES, OnRadioServiceCodes)
	ON_BN_CLICKED(IDC_RADIO_INV_ITEMS, OnRadioInvItems)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAllowedAmountSearchDlg message handlers

BOOL CAllowedAmountSearchDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_FINANCIAL, 0)));

	// (j.jones 2008-05-07 10:43) - PLID 29854 - added nxiconbuttons for modernization
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnSearch.AutoSet(NXB_INSPECT);
	m_btnPreview.AutoSet(NXB_PRINT_PREV);
	
	// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
	m_pDateFilterCombo = BindNxDataList2Ctrl(this,IDC_ALLOWABLE_DATE_FILTER_COMBO,GetRemoteData(),false);
	m_pInsuranceCoCombo = BindNxDataList2Ctrl(this,IDC_ALLOWABLE_INS_CO_COMBO,GetRemoteData(),true);
	m_pInsurancePlanCombo = BindNxDataList2Ctrl(this,IDC_ALLOWABLE_INS_PLAN_COMBO,GetRemoteData(),false);
	m_pLocationCombo = BindNxDataList2Ctrl(this,IDC_ALLOWABLE_LOCATION_COMBO,GetRemoteData(),true);
	m_pServiceCodeCombo = BindNxDataList2Ctrl(this,IDC_ALLOWABLE_CPT_COMBO,GetRemoteData(),true);
	
	// (j.jones 2016-01-18 13:16) - PLID 67501 - added inventory item dropdown
	m_pInventoryItemCombo = BindNxDataList2Ctrl(IDC_ALLOWABLE_INV_ITEM_COMBO, false);

	//show service codes by default
	m_radioServiceCodes.SetCheck(TRUE);
	GetDlgItem(IDC_ALLOWABLE_INV_ITEM_COMBO)->ShowWindow(SW_HIDE);

	//if they don't have inventory, hide this interface
	if (!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
		m_pInventoryItemCombo->PutEnabled(FALSE);
		m_radioServiceCodes.ShowWindow(SW_HIDE);
		m_radioInvItems.ShowWindow(SW_HIDE);
		m_staticServiceCodeLabel.ShowWindow(SW_SHOW);
	}
	else {
		m_staticServiceCodeLabel.ShowWindow(SW_HIDE);

		m_pInventoryItemCombo->FromClause = _bstr_t("(SELECT ProductT.ID, ProductT.InsCode, ServiceT.Name, ProductT.LastCost, ServiceT.Price, ServiceT.Active "
			"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID) AS Query");
		m_pInventoryItemCombo->Requery();
	}
	
	
	m_pResultList = BindNxDataList2Ctrl(this,IDC_ALLOWED_AMOUNT_RESULT_LIST,GetRemoteData(),false);

	//we only set the clauses here for the sake of readability and
	//to facilitate easier changes in the future

	// (j.jones 2011-08-17 11:00) - PLID 44888 - ignore "original" and "void" line items
	m_pResultList->PutFromClause("ChargesT "
		"INNER JOIN (SELECT LineItemT.* FROM LineItemT "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID " 
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID " 
		"	WHERE Deleted = 0 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	) AS ChargeLineItemT ON ChargesT.ID = ChargeLineItemT.ID "
		"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
		"INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
		"INNER JOIN PersonT ON ChargeLineItemT.PatientID = PersonT.ID "
		"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"INNER JOIN (SELECT LineItemT.* FROM LineItemT "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalPaymentsQ ON LineItemT.ID = LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID " 
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingPaymentsQ ON LineItemT.ID = LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID " 
		"	WHERE Type = 1 AND Deleted = 0 "
		"	AND LineItemCorrections_OriginalPaymentsQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingPaymentsQ.VoidingLineItemID Is Null "
		"	) AS PaymentLineItemT ON PaymentsT.ID = PaymentLineItemT.ID "
		"INNER JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
		"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID ");

	m_pResultList->PutGroupByClause("PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, "
		"InsuranceCoT.Name, ChargesT.ID, ItemCode, ItemSubCode, ChargeLineItemT.Amount, ChargesT.Quantity, "
		"ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, "
		"InsuranceCoT.PersonID, PaymentLineItemT.Date, PaymentLineItemT.InputDate, "
		"ChargeLineItemT.Date, ChargeLineItemT.InputDate");

	//add rows to the date filter combo
	IRowSettingsPtr pRow = m_pDateFilterCombo->GetNewRow();
	pRow->PutValue(0, (long)FILTER_ALL_DATES);
	pRow->PutValue(1, "<All Dates>");
	m_pDateFilterCombo->AddRowAtEnd(pRow, NULL);

	//select all dates
	m_pDateFilterCombo->PutCurSel(pRow);

	//add remaining date options
	pRow = m_pDateFilterCombo->GetNewRow();
	pRow->PutValue(0, (long)FILTER_PAYMENT_SERVICE_DATE);
	pRow->PutValue(1, "Payment Service Date");
	m_pDateFilterCombo->AddRowAtEnd(pRow, NULL);
	pRow = m_pDateFilterCombo->GetNewRow();
	pRow->PutValue(0, (long)FILTER_PAYMENT_INPUT_DATE);
	pRow->PutValue(1, "Payment Input Date");
	m_pDateFilterCombo->AddRowAtEnd(pRow, NULL);
	pRow = m_pDateFilterCombo->GetNewRow();
	pRow->PutValue(0, (long)FILTER_CHARGE_SERVICE_DATE);
	pRow->PutValue(1, "Charge Service Date");
	m_pDateFilterCombo->AddRowAtEnd(pRow, NULL);
	pRow = m_pDateFilterCombo->GetNewRow();
	pRow->PutValue(0, (long)FILTER_CHARGE_INPUT_DATE);
	pRow->PutValue(1, "Charge Input Date");
	m_pDateFilterCombo->AddRowAtEnd(pRow, NULL);

	//add an "all insurance companies" row, and select it
	pRow = m_pInsuranceCoCombo->GetNewRow();
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, "<All Insurance Companies>");
	m_pInsuranceCoCombo->AddRowBefore(pRow, m_pInsuranceCoCombo->GetFirstRow());
	m_pInsuranceCoCombo->PutCurSel(pRow);

	//add an "all locations" row, and select it
	pRow = m_pLocationCombo->GetNewRow();
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, "<All Locations>");
	m_pLocationCombo->AddRowBefore(pRow, m_pLocationCombo->GetFirstRow());
	m_pLocationCombo->PutCurSel(pRow);

	//disable the plan combo
	m_pInsurancePlanCombo->Enabled = false;

	//set the dates to be today
	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	m_dtFrom.SetValue(_variant_t(dtNow));
	m_dtTo.SetValue(_variant_t(dtNow));
	
	//disable the dates
	GetDlgItem(IDC_ALLOWABLE_DATE_FROM)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALLOWABLE_DATE_TO)->EnableWindow(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAllowedAmountSearchDlg::OnBtnSearchAllowables() 
{
	try {

		CString strWhere, strHaving;
		COleCurrency cyAllowable;

		if(!ValidateAndBuildWhereClause(strWhere, strHaving, cyAllowable)) {
			return;
		}

		//update the date column
		long nDateFilterType = FILTER_ALL_DATES;		
		IRowSettingsPtr pRow = m_pDateFilterCombo->GetCurSel();
		nDateFilterType = VarLong(pRow->GetValue(0), FILTER_ALL_DATES);

		IColumnSettingsPtr pCol = m_pResultList->GetColumn(DATE_COLUMN);

		switch(nDateFilterType) {			
			case FILTER_PAYMENT_INPUT_DATE:
				pCol->ColumnTitle = "Payment Input Date";
				pCol->FieldName = "PaymentLineItemT.InputDate";
				break;
			case FILTER_CHARGE_SERVICE_DATE:
				pCol->ColumnTitle = "Charge Service Date";
				pCol->FieldName = "ChargeLineItemT.Date";
				break;
			case FILTER_CHARGE_INPUT_DATE:
				pCol->ColumnTitle = "Charge Input Date";
				pCol->FieldName = "ChargeLineItemT.InputDate";
				break;
			default:
			case FILTER_ALL_DATES:
			case FILTER_PAYMENT_SERVICE_DATE:
				pCol->ColumnTitle = "Payment Service Date";
				pCol->FieldName = "PaymentLineItemT.Date";
				break;
		}

		//ValidateAndBuildWhereClause will have ensured the search
		//data is clean, and have given us our where clause,
		//so perform the search with this information

		m_pResultList->PutWhereClause(_bstr_t(strWhere));
		m_pResultList->PutHavingClause(_bstr_t(strHaving));
		m_pResultList->Requery();		
	
	}NxCatchAll("Error searching for payments under the allowed amount.");
}

BEGIN_EVENTSINK_MAP(CAllowedAmountSearchDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAllowedAmountSearchDlg)
	ON_EVENT(CAllowedAmountSearchDlg, IDC_ALLOWABLE_DATE_FILTER_COMBO, 16 /* SelChosen */, OnSelChosenAllowableDateFilterCombo, VTS_DISPATCH)
	ON_EVENT(CAllowedAmountSearchDlg, IDC_ALLOWABLE_INS_CO_COMBO, 16 /* SelChosen */, OnSelChosenAllowableInsCoCombo, VTS_DISPATCH)
	ON_EVENT(CAllowedAmountSearchDlg, IDC_ALLOWED_AMOUNT_RESULT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedAllowedAmountResultList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAllowedAmountSearchDlg::OnSelChosenAllowableDateFilterCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		long nFilterType = FILTER_ALL_DATES;

		if(pRow != NULL) {
			nFilterType = VarLong(pRow->GetValue(0), FILTER_ALL_DATES);
		}

		if(nFilterType == FILTER_ALL_DATES) {
			//disable the date filters
			GetDlgItem(IDC_ALLOWABLE_DATE_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALLOWABLE_DATE_TO)->EnableWindow(FALSE);
		}
		else {
			//enable the date filters
			GetDlgItem(IDC_ALLOWABLE_DATE_FROM)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALLOWABLE_DATE_TO)->EnableWindow(TRUE);
		}

	}NxCatchAll("CAllowedAmountSearchDlg::OnSelChosenAllowableDateFilterCombo");	
}

void CAllowedAmountSearchDlg::OnSelChosenAllowableInsCoCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		long nInsuranceCoID = -1;

		if(pRow != NULL) {
			nInsuranceCoID = VarLong(pRow->GetValue(0), -1);
		}

		if(nInsuranceCoID == -1) {
			//disable the plan filter
			m_pInsurancePlanCombo->Clear();
			m_pInsurancePlanCombo->Enabled = false;			
		}
		else {
			//enable the plan filter
			m_pInsurancePlanCombo->Enabled = true;

			//query the plans for this company
			CString strWhere;
			strWhere.Format("InsurancePlansT.InsCoID = %li", nInsuranceCoID);
			m_pInsurancePlanCombo->PutWhereClause(_bstr_t(strWhere));
			m_pInsurancePlanCombo->Requery();

			//add an "all plans" option
			IRowSettingsPtr pRow = m_pInsurancePlanCombo->GetNewRow();
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, "<All Insurance Plans>");
			m_pInsurancePlanCombo->AddRowBefore(pRow, m_pInsurancePlanCombo->GetFirstRow());
			m_pInsurancePlanCombo->PutCurSel(pRow);
		}

	}NxCatchAll("CAllowedAmountSearchDlg::OnSelChosenAllowableInsCoCombo");	
}

void CAllowedAmountSearchDlg::OnRequeryFinishedAllowedAmountResultList(short nFlags) 
{
	if(m_pResultList->GetRowCount() == 0) {
		AfxMessageBox("There are no applied payments for the given filters that are less than your entered allowed amount.");
		return;
	}
}

BOOL CAllowedAmountSearchDlg::ValidateAndBuildWhereClause(CString &strWhere, CString &strHaving, COleCurrency &cyAllowable)
{
	//validate all the filters, and if they are all clean, build the where clause
	//for the datalist and report

	try {

		IRowSettingsPtr pRow;

		//be sure to disallow unselected filters

		long nDateFilterType = FILTER_ALL_DATES;		
		pRow = m_pDateFilterCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select a date filter type before searching.");
			return FALSE;
		}
		nDateFilterType = VarLong(pRow->GetValue(0), FILTER_ALL_DATES);

		long nInsuranceCoID = -1;
		pRow = m_pInsuranceCoCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select an insurance company, or all insurance companies, before searching.");
			return FALSE;
		}
		nInsuranceCoID = VarLong(pRow->GetValue(0), -1);

		long nInsurancePlanID = -1;
		//insurance plan is not used if a specific insco is not selected
		if(nInsuranceCoID != -1) {
			pRow = m_pInsurancePlanCombo->GetCurSel();
			if(pRow == NULL) {
				AfxMessageBox("You must select an insurance plan, or all insurance plans, before searching.");
				return FALSE;
			}
			nInsurancePlanID = VarLong(pRow->GetValue(0), -1);
		}

		long nLocationID = -1;
		pRow = m_pLocationCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select a location, or all locations, before searching.");
			return FALSE;
		}
		nLocationID = VarLong(pRow->GetValue(0), -1);

		long nServiceID = -1;
		pRow = NULL;
		// (j.jones 2016-01-18 13:28) - PLID 67501 - supported inventory items
		if (m_radioServiceCodes.GetCheck() || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			pRow = m_pServiceCodeCombo->GetCurSel();
		}
		else {
			pRow = m_pInventoryItemCombo->GetCurSel();
		}
		if (pRow == NULL) {
			AfxMessageBox("You must select a service code or inventory item before searching.");
			return FALSE;
		}
		else {
			nServiceID = VarLong(pRow->GetValue(0), -1);
		}

		//disallow "to" dates before a "fore" date
		COleDateTime dtFrom = VarDateTime(m_dtFrom.GetValue());
		COleDateTime dtTo = VarDateTime(m_dtTo.GetValue());

		if(dtTo < dtFrom) {
			AfxMessageBox("You cannot search with a 'from' date later than your 'to' date.");
			return FALSE;
		}

		//and lastly grab the allowed amount and verify it is a valid currency
		CString strAllowed;
		GetDlgItemText(IDC_EDIT_ALLOWABLE_AMOUNT, strAllowed);

		strAllowed.TrimLeft();
		strAllowed.TrimRight();

		if (strAllowed.GetLength() == 0) {
			AfxMessageBox("You must enter an allowable amount to search with.");
			return FALSE;
		}

		cyAllowable = ParseCurrencyFromInterface(strAllowed);
		if(cyAllowable.GetStatus() == COleCurrency::invalid) {
			MsgBox("Please enter a valid allowable amount.");		
			return FALSE;
		}

		//see how much the regional settings allows to the right of the decimal
		CString strICurr;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRDIGITS, strICurr.GetBuffer(3), 3, TRUE);
		int nDigits = atoi(strICurr);
		CString strDecimal;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, strDecimal.GetBuffer(4), 4, TRUE);	
		if(cyAllowable.Format().Find(strDecimal) != -1 && cyAllowable.Format().Find(strDecimal) + (nDigits+1) < cyAllowable.Format().GetLength()) {
			MsgBox("Please fill only %li places to the right of the %s in the allowable amount.",nDigits,strDecimal == "," ? "comma" : "decimal");			
			return FALSE;
		}

		//has to be greater than zero
		if(cyAllowable <= COleCurrency(0,0)) {
			MsgBox("Please enter an allowable amount greater than zero.");			
			return FALSE;
		}

		//if we're still here, we have a good value, so set it back on screen
		SetDlgItemText(IDC_EDIT_ALLOWABLE_AMOUNT,FormatCurrencyForInterface(cyAllowable,FALSE,TRUE));

		//////////////////////////////////////////////////////////////////////////////////////////////////

		//now we know our data is clean, so build the where clause

		CString str;
		
		//dates - based on date filter
		CString strFromDate, strToDate;
		strFromDate = FormatDateTimeForSql(VarDateTime(m_dtFrom.GetValue()), dtoDate);
		strToDate = FormatDateTimeForSql(VarDateTime(m_dtTo.GetValue()), dtoDate);
		switch(nDateFilterType) {			
			case FILTER_PAYMENT_SERVICE_DATE:
				str.Format("(PaymentLineItemT.Date >= '%s' AND PaymentLineItemT.Date < DATEADD(day, 1, '%s'))", strFromDate, strToDate);
				if(!strWhere.IsEmpty())
					strWhere += " AND ";
				strWhere += str;
				break;
			case FILTER_PAYMENT_INPUT_DATE:
				str.Format("(PaymentLineItemT.InputDate >= '%s' AND PaymentLineItemT.InputDate < DATEADD(day, 1, '%s'))", strFromDate, strToDate);
				if(!strWhere.IsEmpty())
					strWhere += " AND ";
				strWhere += str;
				break;
			case FILTER_CHARGE_SERVICE_DATE:
				str.Format("(ChargeLineItemT.Date >= '%s' AND ChargeLineItemT.Date < DATEADD(day, 1, '%s'))", strFromDate, strToDate);
				if(!strWhere.IsEmpty())
					strWhere += " AND ";
				strWhere += str;
				break;
			case FILTER_CHARGE_INPUT_DATE:
				str.Format("(ChargeLineItemT.InputDate >= '%s' AND ChargeLineItemT.InputDate < DATEADD(day, 1, '%s'))", strFromDate, strToDate);
				if(!strWhere.IsEmpty())
					strWhere += " AND ";
				strWhere += str;
				break;
			default:
			case FILTER_ALL_DATES:
				//do nothing
				break;
		}

		//location
		if(nLocationID != -1) {
			str.Format("PaymentLineItemT.LocationID = %li", nLocationID);
			if(!strWhere.IsEmpty())
				strWhere += " AND ";
			strWhere += str;
		}

		//insurance company
		if(nInsuranceCoID != -1) {
			str.Format("InsuredPartyT.InsuranceCoID = %li", nInsuranceCoID);
			if(!strWhere.IsEmpty())
				strWhere += " AND ";
			strWhere += str;
		}

		//insurance plan
		if(nInsurancePlanID != -1) {
			str.Format("InsuredPartyT.InsPlan = %li", nInsurancePlanID);
			if(!strWhere.IsEmpty())
				strWhere += " AND ";
			strWhere += str;
		}

		//service code
		str.Format("ChargesT.ServiceID = %li", nServiceID);
		if(!strWhere.IsEmpty())
			strWhere += " AND ";
		strWhere += str;

		// (j.jones 2015-11-23 15:56) - PLID 67640 - ignore all payments that are not for the same insured party
		// as the allowable insured party (e.g. ignore all secondary payments)
		if (!strWhere.IsEmpty())
			strWhere += " AND ";
		strWhere += "IsNull(PaymentsT.InsuredPartyID, -1) = IsNull(ChargesT.AllowableInsuredPartyID, -1)";

		//and finally the allowable
		str.Format("Sum(AppliesT.Amount) > 0 AND Sum(AppliesT.Amount) < Round(Convert(money, '%s' * Quantity * "
			"(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END) * "
			"(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END) * "
			"(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			"), 2)", FormatCurrencyForSql(cyAllowable));
		if(!strHaving.IsEmpty())
			strHaving += " AND ";
		strHaving += str;

		return TRUE;

	}NxCatchAll("CAllowedAmountSearchDlg::ValidateAndBuildWhereClause");

	return FALSE;
}

void CAllowedAmountSearchDlg::OnBtnPreviewAllowables() 
{
	try {

		CString strWhere, strHaving;
		COleCurrency cyAllowable;
	

		if(!ValidateAndBuildWhereClause(strWhere, strHaving, cyAllowable)) {
			return;
		}

		//ValidateAndBuildWhereClause will have ensured the search
		//data is clean, and have given us our where clause,
		//so run the report with this information

		// (j.jones 2006-12-04 09:52) - PLID 23703 - added this report

		CPtrArray aryParams;
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(579)]);

		// (j.jones 2016-01-18 13:28) - PLID 67501 - supported inventory items
		CString strServiceType, strCodeOrName;
		if (m_radioServiceCodes.GetCheck() || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			IRowSettingsPtr pRow = m_pServiceCodeCombo->GetCurSel();
			if (pRow == NULL) {
				//should be impossible to get here with no service code
				ASSERT(FALSE);
				return;
			}
			strServiceType = "Service Code";
			strCodeOrName = VarString(pRow->GetValue(1), "");
		}
		else {
			IRowSettingsPtr pRow = m_pInventoryItemCombo->GetCurSel();
			if (pRow == NULL) {
				//should be impossible to get here with no product
				ASSERT(FALSE);
				return;
			}
			strServiceType = "Inventory Item";
			strCodeOrName = VarString(pRow->GetValue(2), "");
		}

		CRParameterInfo* pParam = new CRParameterInfo;
		pParam->m_Name = "ServiceType";
		pParam->m_Data = strServiceType;
		aryParams.Add((void*)pParam); 
		
		pParam = new CRParameterInfo;
		pParam->m_Name = "CPTCode";
		pParam->m_Data = strCodeOrName;
		aryParams.Add((void*)pParam);

		pParam = new CRParameterInfo;
		pParam->m_Name = "AllowedAmount";
		pParam->m_Data = FormatCurrencyForInterface(cyAllowable, TRUE, TRUE);
		aryParams.Add((void*)pParam);

		if(!strWhere.IsEmpty())
			infReport.strExtraText = "WHERE " + strWhere;

		if(!strHaving.IsEmpty())
			infReport.strExtraField = "HAVING " + strHaving;

		//infReport.nPatient = m_nPatientID;	//If we ever want to allow 1 form for multiple patients, just remove this.
		//infReport.AddExtraValue(strForm);	//We're hijacking the "extra values" parameter, which is the "use extended" list.
		RunReport(&infReport, &aryParams, TRUE, (CWnd *)this, "Payments Under Allowed Amount Preview", NULL);
		ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done

		//close up the lab entry
		CDialog::OnOK();
	
	}NxCatchAll("Error previewing report.");
}

// (j.jones 2016-01-18 13:24) - PLID 67501 - added service/product radio buttons
void CAllowedAmountSearchDlg::OnRadioServiceCodes()
{
	try {

		if(m_radioInvItems.GetCheck()) {
			GetDlgItem(IDC_ALLOWABLE_CPT_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ALLOWABLE_INV_ITEM_COMBO)->ShowWindow(SW_SHOW);
		}
		else {
			GetDlgItem(IDC_ALLOWABLE_INV_ITEM_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ALLOWABLE_CPT_COMBO)->ShowWindow(SW_SHOW);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-18 13:24) - PLID 67501 - added service/product radio buttons
void CAllowedAmountSearchDlg::OnRadioInvItems()
{
	try {

		OnRadioServiceCodes();

	}NxCatchAll(__FUNCTION__);
}
