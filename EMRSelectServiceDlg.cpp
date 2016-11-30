// EMRSelectServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRSelectServiceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CPT_COLUMN_ID		0
#define CPT_COLUMN_CODE		1
#define CPT_COLUMN_SUBCODE	2
#define CPT_COLUMN_DESC		3
#define CPT_COLUMN_FEE		4
#define CPT_COLUMN_CATEGORY	5
#define CPT_COLUMN_BILLABLE	6
#define CPT_COLUMN_CATEGORY_COUNT 7// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 

#define PRODUCT_COLUMN_ID		0
#define PRODUCT_COLUMN_CATEGORY	1
#define PRODUCT_COLUMN_SUPPLIER	2
#define PRODUCT_COLUMN_DESC		3
#define PRODUCT_COLUMN_BARCODE	4	// (j.jones 2008-09-08 13:03) - PLID 15345 - added barcode column, it isn't scannable, just viewable
#define PRODUCT_COLUMN_FEE		5
#define PRODUCT_COLUMN_COST		6
#define PRODUCT_COLUMN_CATEGORY_COUNT 7// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
#define PRODUCT_COLUMN_CATEGORY_ID	8

// (j.jones 2008-06-03 11:23) - PLID 30062 - added enums for the Quote combo
enum QuoteComboColumns {

	qccID = 0,
	qccDescription,
	qccDate,
	qccHasBeenBilled,
	qccIsPackage,
	qccTotal
};

/////////////////////////////////////////////////////////////////////////////
// CEMRSelectServiceDlg dialog

// (b.savon 2014-02-26 10:13) - PLID 60805 - Added DefaultRadioSelction
CEMRSelectServiceDlg::CEMRSelectServiceDlg(CWnd* pParent, DefaultRadioSelection drsDefaultSelection /*= drsServiceCode*/)
	: CNxDialog(CEMRSelectServiceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRSelectServiceDlg)
		m_ServiceID = -1;
		m_strCode = "";
		m_strSubCode = "";
		m_strDescription = "";
		m_cyPrice = COleCurrency(0,0);
		m_bProductComboLoaded = FALSE;
		m_bGeneric = FALSE;
		m_bQuoteComboLoaded = FALSE;
		m_nPatientID = -1;
		m_nQuoteID = -1;
		m_bBillable = TRUE;
		m_bCptCodesOnly = FALSE; // (z.manning 2011-07-06 12:02) - PLID 44421
		m_bShowRespList = FALSE; // (j.dinatale 2012-01-11 11:51) - PLID 47464
		m_nAssignedInsuredPartyID = -2; // (j.dinatale 2012-01-11 12:00) - PLID 47464 - -2 represents an unassigned resp
		m_drsDefaultSelection = drsDefaultSelection; // (b.savon 2014-02-26 10:13) - PLID 60805 - Added DefaultRadioSelction
		m_bCPTCodeComboLoaded = FALSE;
		m_nCategory = -1;// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
		m_nCategoryCount = 0;
	//}}AFX_DATA_INIT
}


void CEMRSelectServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRSelectServiceDlg)
	DDX_Control(pDX, IDC_RADIO_QUOTES, m_radioQuotes);
	DDX_Control(pDX, IDC_RADIO_SERVICE_CODE, m_radioSvcCode);
	DDX_Control(pDX, IDC_RADIO_PRODUCT, m_radioProductCode);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRSelectServiceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRSelectServiceDlg)
	ON_BN_CLICKED(IDC_RADIO_SERVICE_CODE, OnRadioServiceCode)
	ON_BN_CLICKED(IDC_RADIO_PRODUCT, OnRadioProduct)
	ON_BN_CLICKED(IDC_RADIO_QUOTES, OnRadioQuotes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRSelectServiceDlg message handlers

BOOL CEMRSelectServiceDlg::OnInitDialog() 
{
	try {
		
		CNxDialog::OnInitDialog();

		// (z.manning 2011-07-06 12:02) - PLID 44421 - Have an option to hide all the radio buttons and make it CPT codes only
		if(m_bCptCodesOnly)
		{
			SetWindowText("Select a service code");
			GetDlgItem(IDC_RADIO_QUOTES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_SERVICE_CODE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_PRODUCT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_QUOTES)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_SERVICE_CODE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_PRODUCT)->EnableWindow(FALSE);
		}
		else if (m_bGeneric) {
			SetWindowText("Select a product or service code");

			// (j.jones 2008-06-03 11:09) - PLID 30062 - hide the quote option if called from outside the EMR
			// (don't ask me why an EMR dialog is used outside of the EMR...)
			GetDlgItem(IDC_RADIO_QUOTES)->ShowWindow(SW_HIDE);
		}
		
		if(m_nPatientID == -1) {
			// (j.jones 2008-06-03 11:09) - PLID 30062 - disable the quote option if no patient ID was given,
			// which will happen if called from a template
			GetDlgItem(IDC_RADIO_QUOTES)->EnableWindow(FALSE);
		}

		// (c.haag 2008-04-25 16:53) - PLID 29796 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (b.savon 2014-02-26 10:35) - PLID 60805 - Set the selection
		switch(m_drsDefaultSelection){
			case drsServiceCode:
				{
					m_radioSvcCode.SetCheck(BST_CHECKED);
					m_bCPTCodeComboLoaded = TRUE;
				}
				break;
			case drsInventoryItem:
				{
					m_radioProductCode.SetCheck(BST_CHECKED);
					m_bProductComboLoaded = TRUE;
					SetWindowText("Add An Inventory Item");
				}
				break;
			case drsQuote:
				{
					m_radioQuotes.SetCheck(BST_CHECKED);
					m_bQuoteComboLoaded = TRUE;
					SetWindowText("Add An Existing Quote");
				}
				break;
		}
		
		// (b.savon 2014-02-26 10:51) - PLID 60805 - Requery what we need initially
		m_CPTCombo = BindNxDataListCtrl(this,IDC_CPT_CODE_COMBO,GetRemoteData(), m_drsDefaultSelection == drsServiceCode ? true : false);
		m_ProductCombo = BindNxDataListCtrl(this,IDC_PRODUCT_COMBO,GetRemoteData(), false);
		m_ProductCombo->PutWhereClause(_bstr_t("ServiceT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE Billable = 1) AND ServiceT.Active = 1"));
		// (j.jones 2006-05-22 12:53) - do not requery yet, until they select products
		// (b.savon 2014-02-26 10:51) - PLID 60805 - Requery what we need initially
		if( m_drsDefaultSelection == drsInventoryItem ){
			m_ProductCombo->Requery();
		}

		// (j.jones 2008-06-03 10:20) - PLID 30062 - added the quote combo
		m_QuotesCombo = BindNxDataListCtrl(this,IDC_EMR_QUOTE_COMBO,GetRemoteData(),false);

		if(m_nPatientID != -1) {
			CString str;
			//copied from the billing dialog and modified slightly
			// (j.gruber 2009-03-17 17:39) - PLID 33360 - changed for new discount structure
			str.Format("(SELECT [PatientBillsQ].ID, [PatientBillsQ].Description, [PatientBillsQ].Date, "
				"Convert(bit, (CASE WHEN HasBeenBilled Is Null THEN 0 ELSE 1 END)) AS HasBeenBilled, "
				"Convert(bit, (CASE WHEN PackagesT.QuoteID Is Null THEN 0 ELSE 1 END)) AS IsPackage, "
				"(CASE WHEN PackagesT.QuoteID Is Not Null THEN PackagesT.CurrentAmount ELSE "
				"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
				"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
				"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
				")),2)) END) AS Total "
				"FROM ((SELECT BillsT.*, (SELECT TOP 1 ID FROM BilledQuotesT WHERE BilledQuotesT.QuoteID = BillsT.ID AND BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) AS HasBeenBilled FROM BillsT "
				"WHERE BillsT.PatientID=%li AND BillsT.Deleted=0 AND BillsT.Active = 1) AS PatientBillsQ "
				"INNER JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, TotalPercentageQ.TotalPercentOff, TotalDiscountQ.TotalDiscount FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID "
				"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
				"WHERE LineItemT.PatientID=%li AND LineItemT.Deleted=0 AND LineItemT.Type >= 10 AND (ChargesT.OthrBillFee = 0 OR LineItemT.Amount > 0)) AS PatientChargesQ ON [PatientBillsQ].ID = [PatientChargesQ].BillID) "
				"INNER JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
				"LEFT JOIN PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID "
				"WHERE (([PatientBillsQ].EntryType)=2) "
				"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, PatientBillsQ.HasBeenBilled, PackagesT.QuoteID, PackagesT.CurrentAmount) AS Q",m_nPatientID,m_nPatientID);
			m_QuotesCombo->FromClause = _bstr_t(str);
			// (b.savon 2014-02-26 10:51) - PLID 60805 - Requery what we need initially
			if( m_drsDefaultSelection == drsQuote ){
				m_QuotesCombo->Requery();
			}
		}

		// (j.dinatale 2012-01-11 11:52) - PLID 47464 - need to hide or show the resp list that was added because
		//	we need this list for the emr
		m_pRespList = BindNxDataList2Ctrl(IDC_ASSIGN_RESP_TO_CHARGE, GetRemoteData(), false);

		if(m_bShowRespList && m_nPatientID != -1){
			GetDlgItem(IDC_ASSIGN_RESP_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ASSIGN_RESP_TO_CHARGE)->ShowWindow(SW_SHOW);

			CString strRespListFrom;
			strRespListFrom.Format(
				"(SELECT InsuredPartyT.PersonID AS InsuredPartyID, InsuranceCoT.Name AS InsCoName, "
				"RespTypeT.TypeName AS TypeName, RespTypeT.Priority AS Priority "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = %li AND RespTypeT.Priority <> -1 "
				"UNION "
				"SELECT -1, 'Patient Resp.', '', -1 "
				"UNION "
				"SELECT -2, '< Unassigned >', '', -2 "
				") InsRespInfoQ ", m_nPatientID);

			m_pRespList->FromClause = _bstr_t(strRespListFrom);
			m_pRespList->Requery();
			m_pRespList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
			// (j.jones 2012-08-22 09:34) - PLID 50486 - the insured party ID can now take in a default
			// value, which we should auto-select
			m_pRespList->SetSelByColumn(arlcInsPartyID, m_nAssignedInsuredPartyID);
		}
		else{
			GetDlgItem(IDC_ASSIGN_RESP_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ASSIGN_RESP_TO_CHARGE)->ShowWindow(SW_HIDE);
		}

		// (j.jones 2013-10-14 15:20) - PLID 58997 - call ChangeChargeList,
		// which ensures everything is drawn properly, and also sets focus
		// to the dropdown datalist
		ChangeChargeList();
	}
	NxCatchAll("Error in CEMRSelectServiceDlg::OnInitDialog");
	
	// (j.jones 2013-10-14 15:21) - PLID 58997 - return FALSE,
	// because ChangeChargeList() does set focus to a control
	return FALSE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRSelectServiceDlg::OnOK() 
{
	if(IsDlgButtonChecked(IDC_RADIO_PRODUCT)) {
		if(m_ProductCombo->GetCurSel() == -1) {
			MessageBox(m_bGeneric ? "Please select a product" : "Please select a product to add to the bill.");
			return;
		}

		m_ServiceID = VarLong(m_ProductCombo->GetValue(m_ProductCombo->GetCurSel(),PRODUCT_COLUMN_ID));
		m_strCode = "";
		m_strSubCode = "";
		m_strDescription = VarString(m_ProductCombo->GetValue(m_ProductCombo->GetCurSel(),PRODUCT_COLUMN_DESC), "");
		m_cyPrice = VarCurrency(m_ProductCombo->GetValue(m_ProductCombo->GetCurSel(),PRODUCT_COLUMN_FEE), COleCurrency(0,0));
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
		m_nCategory = VarLong(m_ProductCombo->GetValue(m_ProductCombo->GetCurSel(), PRODUCT_COLUMN_CATEGORY_ID), -1);
		m_nCategoryCount = VarLong(m_ProductCombo->GetValue(m_ProductCombo->GetCurSel(), PRODUCT_COLUMN_CATEGORY_COUNT), 0);
		m_nQuoteID = -1;

		// (j.dinatale 2012-01-11 13:38) - PLID 47464 - set the insured party id
		if(m_bShowRespList && m_nPatientID != -1 && m_pRespList->GetCurSel()){
			m_nAssignedInsuredPartyID = VarLong(m_pRespList->GetCurSel()->GetValue(arlcInsPartyID), -2);
		}else{
			m_nAssignedInsuredPartyID = -2;
		}
	}
	else if(IsDlgButtonChecked(IDC_RADIO_SERVICE_CODE)) {
		if(m_CPTCombo->GetCurSel() == -1) {
			MessageBox(m_bGeneric || m_bCptCodesOnly ? "Please select a service" : "Please select a service to add to the bill.");
			return;
		}

		m_ServiceID = VarLong(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),CPT_COLUMN_ID));
		m_strCode = VarString(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),CPT_COLUMN_CODE), "");
		m_strSubCode = VarString(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),CPT_COLUMN_SUBCODE), "");
		m_strDescription = VarString(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),CPT_COLUMN_DESC), "");
		m_cyPrice = VarCurrency(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),CPT_COLUMN_FEE), COleCurrency(0,0));
		m_nQuoteID = -1;
		// (j.jones 2011-03-28 15:20) - PLID 42575 - added m_bBillable
		m_bBillable = VarBool(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(),CPT_COLUMN_BILLABLE), TRUE);
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
		m_nCategory = VarLong(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(), CPT_COLUMN_CATEGORY), -1);
		m_nCategoryCount = VarLong(m_CPTCombo->GetValue(m_CPTCombo->GetCurSel(), CPT_COLUMN_CATEGORY_COUNT), 0);
		// (j.dinatale 2012-01-11 13:38) - PLID 47464 - set the insured party id
		if(m_bShowRespList && m_nPatientID != -1 &&m_pRespList->GetCurSel()){
			m_nAssignedInsuredPartyID = VarLong(m_pRespList->GetCurSel()->GetValue(arlcInsPartyID), -2);
		}else{
			m_nAssignedInsuredPartyID = -2;
		}
	}
	// (j.jones 2008-06-03 11:11) - PLID 30062 - added quote option
	else if(IsDlgButtonChecked(IDC_RADIO_QUOTES)) {
		long nSel = m_QuotesCombo->GetCurSel();
		if(nSel == -1) {
			MessageBox("Please select a quote.");
			return;
		}

		//warn if it has been billed before
		BOOL bHasBeenBilled = VarBool(m_QuotesCombo->GetValue(nSel, qccHasBeenBilled));
		if(bHasBeenBilled &&
			IDNO == MessageBox("This quote has already been billed, do you still wish to select this quote?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		m_nQuoteID = VarLong(m_QuotesCombo->GetValue(nSel, qccID));
		m_ServiceID = -1;

		// (j.dinatale 2012-01-11 13:38) - PLID 47464 - set the insured party id to be unassigned here since its a quote
		m_nAssignedInsuredPartyID = -2;
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
		m_nCategory = 0;
		m_nCategoryCount = -1;
	}
	else {
		//should be impossible to have no options checked
		MessageBox("You must select a service to continue.");
		return;
	}	
	
	CDialog::OnOK();
}

void CEMRSelectServiceDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CEMRSelectServiceDlg::OnRadioServiceCode() 
{
	ChangeChargeList();
}

void CEMRSelectServiceDlg::OnRadioProduct() 
{
	ChangeChargeList();
}

void CEMRSelectServiceDlg::ChangeChargeList()
{
	if(IsDlgButtonChecked(IDC_RADIO_PRODUCT)) {
		GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CPT_CODE_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EMR_QUOTE_COMBO)->ShowWindow(SW_HIDE);

		// (j.dinatale 2012-01-11 11:56) - PLID 47464 - need to re-enable the resp list
		if(m_bShowRespList && m_nPatientID != -1){
			GetDlgItem(IDC_ASSIGN_RESP_TO_CHARGE)->EnableWindow(TRUE);
		}

		if(!m_bProductComboLoaded) {
			m_ProductCombo->Requery();
			m_bProductComboLoaded = TRUE;
		}

		// (j.jones 2013-10-14 15:22) - PLID 58997 - set focus to the product list
		GetDlgItem(IDC_PRODUCT_COMBO)->SetFocus();
	}
	// (j.jones 2008-06-03 10:23) - PLID 30062 - added Quote option
	else if(IsDlgButtonChecked(IDC_RADIO_QUOTES)) {
		GetDlgItem(IDC_EMR_QUOTE_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CPT_CODE_COMBO)->ShowWindow(SW_HIDE);

		// (j.dinatale 2012-01-11 11:56) - PLID 47464 - need to disable the resp list for quotes
		if(m_bShowRespList && m_nPatientID != -1){
			GetDlgItem(IDC_ASSIGN_RESP_TO_CHARGE)->EnableWindow(FALSE);
		}

		if(!m_bQuoteComboLoaded) {
			m_QuotesCombo->Requery();
			m_bQuoteComboLoaded = TRUE;
		}

		// (j.jones 2013-10-14 15:22) - PLID 58997 - set focus to the quotes list
		GetDlgItem(IDC_EMR_QUOTE_COMBO)->SetFocus();
	}
	else {
		GetDlgItem(IDC_CPT_CODE_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EMR_QUOTE_COMBO)->ShowWindow(SW_HIDE);

		// (j.dinatale 2012-01-11 11:56) - PLID 47464 - need to re-enable the resp list
		if(m_bShowRespList && m_nPatientID != -1){
			GetDlgItem(IDC_ASSIGN_RESP_TO_CHARGE)->EnableWindow(TRUE);
		}

		// (b.savon 2014-02-26 11:05) - PLID 60805 - CPT is no longer always the default
		if(!m_bCPTCodeComboLoaded){
			m_CPTCombo->Requery();
			m_bCPTCodeComboLoaded = TRUE;
		}

		// (j.jones 2013-10-14 15:22) - PLID 58997 - set focus to the service list
		GetDlgItem(IDC_CPT_CODE_COMBO)->SetFocus();
	}
}

// (j.jones 2008-06-03 10:22) - PLID 30062 - added OnRadioQuotes
void CEMRSelectServiceDlg::OnRadioQuotes() 
{
	ChangeChargeList();
}

BEGIN_EVENTSINK_MAP(CEMRSelectServiceDlg, CNxDialog)
	ON_EVENT(CEMRSelectServiceDlg, IDC_ASSIGN_RESP_TO_CHARGE, 16, CEMRSelectServiceDlg::SelChosenAssignRespToCharge, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (j.dinatale 2012-01-11 13:37) - PLID 47464 - if for some reason we didnt select something, just select unassigned
void CEMRSelectServiceDlg::SelChosenAssignRespToCharge(LPDISPATCH lpRow)
{
	try{
		if(!lpRow){
			m_pRespList->SetSelByColumn(arlcInsPartyID, -2);
		}
	}NxCatchAll(__FUNCTION__);
}
