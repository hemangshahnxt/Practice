// EMNQuoteSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMNQuoteSelectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_QUOTE_ID			0
#define COLUMN_QUOTE_DESC		1
#define COLUMN_QUOTE_DATE		2
#define COLUMN_QUOTE_ISPACKAGE	3
#define COLUMN_QUOTE_TOTAL		4

using namespace NXDATALIST2Lib;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEMNQuoteSelectionDlg dialog


CEMNQuoteSelectionDlg::CEMNQuoteSelectionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMNQuoteSelectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMNQuoteSelectionDlg)
		m_nPatientID = -1;
		m_nEMNID = -1;
		m_nLinkedToQuoteID = -1;
		m_bLinkedQuoteIsPackage = FALSE;
		m_nLinkedQuoteBillType = elqbtQuoteAndEMN;
		m_varInsuredPartyID = g_cvarNull;
	//}}AFX_DATA_INIT
}


void CEMNQuoteSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMNQuoteSelectionDlg)
	DDX_Control(pDX, IDC_RADIO_BILL_QUOTE_AND_EMN, m_btnQuoteAndEMN);
	DDX_Control(pDX, IDC_RADIO_BILL_QUOTE_ONLY, m_btnQuoteOnly);
	DDX_Control(pDX, IDC_RADIO_BILL_EMN_CHARGES_ONLY, m_btnChargesOnly);
	DDX_Control(pDX, IDC_RADIO_BILL_EMN_CHARGES_NO_QUOTE, m_btnNoQuote);
	DDX_Control(pDX, IDC_QUOTE_LABEL, m_nxstaticQuoteLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMNQuoteSelectionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMNQuoteSelectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMNQuoteSelectionDlg message handlers

BOOL CEMNQuoteSelectionDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-05-01 16:03) - PLID 29871 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_QuoteCombo = BindNxDataList2Ctrl(this, IDC_QUOTE_COMBO, GetRemoteData(), false);
		m_EMNCharges = BindNxDataList2Ctrl(this, IDC_EMN_CHARGE_LIST, GetRemoteData(), false);
		m_QuoteCharges = BindNxDataList2Ctrl(this, IDC_QUOTE_CHARGE_LIST, GetRemoteData(), false);

		//query the combo of available quotes / packages

		long nEMNLinkToQuote = GetRemotePropertyInt("EMNLinkToQuote", 0, 0, "<None>",TRUE);
		//0 - filtered, 1 - all unbilled quotes

		CString strEMRChargeFromClause, strEMRChargeWhereClause;

		// (j.jones 2012-01-19 09:24) - PLID 47597 - m_varInsuredPartyID is an optional filter for EMR charges,
		// NULL will mean no filter, -1 means to filter by patient resp, else insured party ID		
		if(m_varInsuredPartyID.vt == VT_I4) {
			strEMRChargeFromClause = "EMRChargesT INNER JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				"INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID";

			long nInsuredPartyID = VarLong(m_varInsuredPartyID);
			if(nInsuredPartyID == -1){
				strEMRChargeWhereClause.Format("Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
					"AND EMRChargeRespT.InsuredPartyID IS NULL AND EMRID = %li", m_nEMNID);
			}else{
				strEMRChargeWhereClause.Format("Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
					"AND EMRChargeRespT.InsuredPartyID = %li AND EMRID = %li", nInsuredPartyID, m_nEMNID);
			}
		}
		else {
			strEMRChargeFromClause = "EMRChargesT INNER JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID";
			strEMRChargeWhereClause.Format("Deleted = 0 AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) AND EMRID = %li", m_nEMNID);
		}
		
		m_EMNCharges->PutFromClause(_bstr_t(strEMRChargeFromClause));
		m_EMNCharges->PutWhereClause(_bstr_t(strEMRChargeWhereClause));
		m_EMNCharges->Requery();

		CString strEMNChargeFilter;
		if(nEMNLinkToQuote == 0) {
			// (j.jones 2011-03-28 16:50) - PLID 42575 - do not match on non-billable CPT codes
			// (j.jones 2012-01-17 17:05) - PLID 47600 - fixed bug where it didn't ignore deleted EMR charges

			// (j.jones 2012-01-19 09:24) - PLID 47597 - m_varInsuredPartyID is an optional filter for EMR charges,
			// NULL will mean no filter, -1 means to filter by patient resp, else insured party ID		
			if(m_varInsuredPartyID.vt == VT_I4) {

				long nInsuredPartyID = VarLong(m_varInsuredPartyID);
				if(nInsuredPartyID == -1){
					strEMNChargeFilter.Format(" AND [PatientBillsQ].ID IN ("
						"	SELECT BillID FROM ChargesT "
						"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"	WHERE Deleted = 0 AND "
						"	ServiceID IN ("
						"		SELECT ServiceID FROM EMRChargesT "
						"		LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
						"		INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
						"		WHERE EMRID = %li AND Deleted = 0 AND EMRChargeRespT.InsuredPartyID IS NULL "
						"		AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
						"	)"
						") ", m_nEMNID);
				}
				else {
					strEMNChargeFilter.Format(" AND [PatientBillsQ].ID IN ("
						"	SELECT BillID FROM ChargesT "
						"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"	WHERE Deleted = 0 AND "
						"	ServiceID IN ("
						"		SELECT ServiceID FROM EMRChargesT "
						"		LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
						"		INNER JOIN EMRChargeRespT ON EMRChargesT.ID = EMRChargeRespT.EMRChargeID "
						"		WHERE EMRID = %li AND Deleted = 0 AND EMRChargeRespT.InsuredPartyID = %li "
						"		AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
						"	)"
						") ", m_nEMNID, nInsuredPartyID);
				}
			}
			else {
				strEMNChargeFilter.Format(" AND [PatientBillsQ].ID IN ("
					"	SELECT BillID FROM ChargesT "
					"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"	WHERE Deleted = 0 AND "
					"	ServiceID IN ("
					"		SELECT ServiceID FROM EMRChargesT "
					"		LEFT JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
					"		WHERE EMRID = %li AND Deleted = 0 "
					"		AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
					"	)"
					") ", m_nEMNID);
			}
		}

		CString strFrom;
		// (j.gruber 2009-03-17 17:29) - PLID 33360 - change discount structure
		// (j.jones 2012-01-17 17:05) - PLID 47600 - fixed bug where it didn't ignore deleted EMR charges
		strFrom.Format("(SELECT [PatientBillsQ].ID AS QuoteID, [PatientBillsQ].Description, [PatientBillsQ].Date, "
			"CASE WHEN PackagesT.QuoteID Is Not Null THEN (CASE WHEN CurrentCount > 0 THEN 0 ELSE 1 END) ELSE Coalesce(HasBeenBilled,0) END AS HasBeenBilled, "
			"Convert(bit, CASE WHEN PackagesT.QuoteID Is Not Null THEN 1 ELSE 0 END) AS IsPackage, "
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
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID "
			"WHERE [PatientBillsQ].EntryType = 2 "
			"AND (CASE WHEN PackagesT.QuoteID Is Not Null THEN (CASE WHEN CurrentCount > 0 THEN 0 ELSE 1 END) ELSE Coalesce(HasBeenBilled,0) END) = 0 "
			"AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
			"%s "
			"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, CASE WHEN PackagesT.QuoteID Is Not Null THEN (CASE WHEN CurrentCount > 0 THEN 0 ELSE 1 END) ELSE Coalesce(HasBeenBilled,0) END, PackagesT.QuoteID, PackagesT.CurrentAmount) AS Q",m_nPatientID,m_nPatientID, strEMNChargeFilter);
		
		m_QuoteCombo->PutFromClause(_bstr_t(strFrom));
		m_QuoteCombo->Requery();

		// (j.jones 2012-01-19 09:44) - PLID 47597 - if filtering by insured party ID,
		// the first two options in this screen are disallowed
		if(m_varInsuredPartyID.vt == VT_I4) {
			GetDlgItem(IDC_RADIO_BILL_QUOTE_AND_EMN)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_BILL_QUOTE_ONLY)->EnableWindow(FALSE);
			CheckDlgButton(IDC_RADIO_BILL_EMN_CHARGES_ONLY, TRUE);
		}
		else {
			//set the "bill Quote and EMN" option as the default
			CheckDlgButton(IDC_RADIO_BILL_QUOTE_AND_EMN, TRUE);
		}
	}
	NxCatchAll("Error in CEMNQuoteSelectionDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEMNQuoteSelectionDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMNQuoteSelectionDlg)
	ON_EVENT(CEMNQuoteSelectionDlg, IDC_QUOTE_COMBO, 16 /* SelChosen */, OnSelChosenQuoteCombo, VTS_DISPATCH)
	ON_EVENT(CEMNQuoteSelectionDlg, IDC_QUOTE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedQuoteCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMNQuoteSelectionDlg::OnSelChosenQuoteCombo(LPDISPATCH lpRow) 
{
	try {

		//fill the quote charge list
		
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_QuoteCharges->Clear();
			return;
		}

		long nQuoteID = VarLong(pRow->GetValue(COLUMN_QUOTE_ID),-1);
		BOOL bIsPackage = VarBool(pRow->GetValue(COLUMN_QUOTE_ISPACKAGE),FALSE);

		//if a package, we need to disable certain options
		// (j.jones 2012-01-19 09:44) - PLID 47597 - if filtering by insured party ID,
		// the first two options in this screen are disallowed
		GetDlgItem(IDC_RADIO_BILL_QUOTE_AND_EMN)->EnableWindow(!bIsPackage && m_varInsuredPartyID.vt != VT_I4);
		GetDlgItem(IDC_RADIO_BILL_EMN_CHARGES_ONLY)->EnableWindow(!bIsPackage);

		///also unselect any potentially selected options
		if(bIsPackage) {
			if(IsDlgButtonChecked(IDC_RADIO_BILL_QUOTE_AND_EMN)) {
				CheckDlgButton(IDC_RADIO_BILL_QUOTE_AND_EMN, FALSE);
				if(m_varInsuredPartyID.vt == VT_I4) {
					CheckDlgButton(IDC_RADIO_BILL_EMN_CHARGES_NO_QUOTE, TRUE);
				}
				else {
					CheckDlgButton(IDC_RADIO_BILL_QUOTE_ONLY, TRUE);
				}
			}
			else if(IsDlgButtonChecked(IDC_RADIO_BILL_EMN_CHARGES_ONLY)) {
				CheckDlgButton(IDC_RADIO_BILL_EMN_CHARGES_ONLY, FALSE);
				if(m_varInsuredPartyID.vt == VT_I4) {
					CheckDlgButton(IDC_RADIO_BILL_EMN_CHARGES_NO_QUOTE, TRUE);
				}
				else {
					CheckDlgButton(IDC_RADIO_BILL_QUOTE_ONLY, TRUE);
				}
			}
		}

		//change the label accordingly
		if(bIsPackage)
			SetDlgItemText(IDC_QUOTE_LABEL, "Package Charges");
		else
			SetDlgItemText(IDC_QUOTE_LABEL, "Quote Charges");

		CString strWhere;
		//remember to filter out charges that are only OutsideFees
		strWhere.Format("Deleted = 0 "
			"AND (CPTCodeT.Billable Is Null OR CPTCodeT.Billable = 1) "
			"AND BillID = %li "
			"AND (OthrBillFee = Convert(money,0) OR Amount > Convert(money,0))", nQuoteID);
		m_QuoteCharges->PutWhereClause(_bstr_t(strWhere));
		m_QuoteCharges->Requery();

	}NxCatchAll("Error in CEMNQuoteSelectionDlg::OnSelChosenQuoteCombo");
}

void CEMNQuoteSelectionDlg::OnCancel() 
{
	int nRet = MsgBox(MB_YESNOCANCEL, "Do you wish to continue billing this EMN without linking to a quote?");

	if(nRet == IDNO) {	
		CDialog::OnCancel();
	}
	else if(nRet == IDYES) {
		//just close the dialog saying we are not linking to a quote
		m_nLinkedQuoteBillType = elqbtEMNOnlyNoLink;
		m_nLinkedToQuoteID = -1;
		m_bLinkedQuoteIsPackage = FALSE;
		CDialog::OnOK();
	}
	
	//otherwise, do not cancel
}

void CEMNQuoteSelectionDlg::OnRequeryFinishedQuoteCombo(short nFlags) 
{
	try {

		//if only one entry, auto-select it
		
		if(m_QuoteCombo->GetRowCount() == 1) {
			IRowSettingsPtr pRow = m_QuoteCombo->GetFirstRow();
			m_QuoteCombo->PutCurSel(pRow);
			OnSelChosenQuoteCombo(pRow);
		}

	}NxCatchAll("Error in CEMNQuoteSelectionDlg::OnRequeryFinishedQuoteCombo");	
}

void CEMNQuoteSelectionDlg::OnOK() 
{
	try {

		//grab the quote ID, the bill type, and close

		//first the bill type
		
		m_nLinkedQuoteBillType = elqbtQuoteAndEMN;

		if(IsDlgButtonChecked(IDC_RADIO_BILL_QUOTE_AND_EMN)) {

			// (j.jones 2012-01-19 09:49) - PLID 47597 - if filtering by insured party ID,
			// this option should have been impossible to select
			if(m_varInsuredPartyID.vt == VT_I4) {
				ThrowNxException("Invalid option chosen: 'Bill the Quote Charges and any EMN Charges not on the Quote' when billing a partial EMN.");
			}

			m_nLinkedQuoteBillType = elqbtQuoteAndEMN;
		}
		else if(IsDlgButtonChecked(IDC_RADIO_BILL_QUOTE_ONLY)) {
			m_nLinkedQuoteBillType = elqbtQuoteOnly;

			// (j.jones 2012-01-19 09:49) - PLID 47597 - if filtering by insured party ID,
			// this option should have been impossible to select
			if(m_varInsuredPartyID.vt == VT_I4) {
				ThrowNxException("Invalid option chosen: 'Bill the Quote Charges only' when billing a partial EMN.");
			}
		}
		else if(IsDlgButtonChecked(IDC_RADIO_BILL_EMN_CHARGES_ONLY)) {
			m_nLinkedQuoteBillType = elqbtEMNOnly;
		}
		else if(IsDlgButtonChecked(IDC_RADIO_BILL_EMN_CHARGES_NO_QUOTE)) {
			m_nLinkedQuoteBillType = elqbtEMNOnlyNoLink;
		}

		// (j.jones 2011-06-17 12:40) - PLID 42758 - if we aren't linking to a quote, we shouldn't
		// force them to select one!
		if(m_nLinkedQuoteBillType == elqbtEMNOnlyNoLink) {

			m_nLinkedToQuoteID = -1;
			m_bLinkedQuoteIsPackage = FALSE;
		}
		else {

			//link to a quote

			IRowSettingsPtr pRow = m_QuoteCombo->GetCurSel();
			if(pRow == NULL) {
				if(m_nLinkedQuoteBillType != elqbtEMNOnlyNoLink) {
					//they have to select a quote...
					AfxMessageBox("You must select a quote before continuing.");
					return;
				}
				else {
					//...unless they chose the option to not link to a quote at all
					m_nLinkedToQuoteID = -1;
					m_bLinkedQuoteIsPackage = FALSE;
				}
			}
			else {
				//set our quote ID
				m_nLinkedToQuoteID = VarLong(pRow->GetValue(COLUMN_QUOTE_ID),-1);
				// (j.jones 2010-04-09 09:19) - PLID 27671 - track that this is a package
				m_bLinkedQuoteIsPackage = VarBool(pRow->GetValue(COLUMN_QUOTE_ISPACKAGE),FALSE);
			}

			if(m_nLinkedToQuoteID != -1) {
				//check to see if this quote has expired
				_RecordsetPtr rs = CreateRecordset("SELECT UseExp, ExpDays, Date FROM BillsT WHERE ID = %li",m_nLinkedToQuoteID);
				if(!rs->eof) {
					_variant_t varTmp;

					varTmp = rs->Fields->Item["UseExp"]->Value;
					if(varTmp.vt == VT_BOOL && varTmp.boolVal) {
						//this quote expires
						varTmp = rs->Fields->Item["ExpDays"]->Value;
						if(varTmp.vt == VT_I4) {
							long ExpDays = varTmp.lVal;

							//we decided that 0 and 1 will both mean it expires today
							if(ExpDays <= 0)
								ExpDays = 1;

							COleDateTime dtExp, dtNow;
							COleDateTimeSpan dtSpan;
							dtSpan.SetDateTimeSpan(ExpDays,0,0,0);
							
							dtExp = rs->Fields->Item["Date"]->Value.date;
							dtExp += dtSpan;

							dtNow = COleDateTime::GetCurrentTime();

							//the expiration date is the quote date + days to expiration.
							if(dtExp < dtNow) {
								AfxMessageBox("This quote has expired. Please go to the Quotes tab to extend the expiration date, or delete the quote entirely.");
								m_nLinkedToQuoteID = -1;
								m_bLinkedQuoteIsPackage = FALSE;
								return;
							}
						}
					}
				}
				rs->Close();
			}
		}

		CDialog::OnOK();

	}NxCatchAll("Error in CEMNQuoteSelectionDlg::OnRequeryFinishedQuoteCombo");	
}
