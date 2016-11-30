// SalesReceiptConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "billingRc.h"
#include "SalesReceiptConfigDlg.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "DatetimeUtils.h"
#include "Internationalutils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSalesReceiptConfigDlg dialog
// (j.gruber 2007-03-15 13:36) - PLID 25020 - class created
// (j.gruber 2007-03-29 14:43) - PLID 9802 - receipt printer 
// (a.walling 2008-07-07 17:19) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
CSalesReceiptConfigDlg::CSalesReceiptConfigDlg(COleDateTime dtPay, long nNumber, long nPaymentID, BOOL bPreview, long nReportID, long nPatientID, CWnd* pParent)
	: CNxDialog(CSalesReceiptConfigDlg::IDD, pParent)
{
	m_dtPay = dtPay;
	m_nNumber = nNumber;
	m_bPreview = bPreview;
	m_nPayID = nPaymentID;
	m_nReportID = nReportID;
	m_nPatientID = nPatientID;

	//{{AFX_DATA_INIT(CSalesReceiptConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

// (j.gruber 2007-03-15 14:47) - PLID 25223 - account for being run from the right click menu
// (j.gruber 2007-03-29 14:43) - PLID 9802 - receipt printer 
// (a.walling 2008-07-07 17:19) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
CSalesReceiptConfigDlg::CSalesReceiptConfigDlg(COleDateTime dtPay, long nPaymentID, BOOL bPreview, long nReportID, long nPatientID, CWnd* pParent)
	: CNxDialog(CSalesReceiptConfigDlg::IDD, pParent)
{
	m_dtPay = dtPay;
	
	m_bPreview = bPreview;
	m_nPayID = nPaymentID;
	m_nReportID = nReportID;
	m_nPatientID = nPatientID;

	//we have to find the default report if we are running from the right click menu
	ADODB::_RecordsetPtr rs = CreateRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = 585");
	if (rs->eof) {
		m_nNumber = -2;
	}
	else {
		m_nNumber = AdoFldLong(rs, "CustomReportID");
	}

	//{{AFX_DATA_INIT(CSalesReceiptConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSalesReceiptConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSalesReceiptConfigDlg)
	DDX_Control(pDX, IDC_SALES_RECEIPT_PRINT, m_btnSalesReceiptPrint);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CSalesReceiptConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSalesReceiptConfigDlg)
	ON_BN_CLICKED(IDC_SALES_RECEIPT_PRINT, OnSalesReceiptPrint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSalesReceiptConfigDlg message handlers

BOOL CSalesReceiptConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-02 09:01) - PLID 29879 - NxIconify the buttons
		m_btnCancel.AutoSet(NXB_CLOSE);
		if (m_bPreview) {
			m_btnSalesReceiptPrint.AutoSet(NXB_PRINT_PREV);
			SetDlgItemText(IDC_SALES_RECEIPT_PRINT, "Preview");

		}
		else {
			m_btnSalesReceiptPrint.AutoSet(NXB_PRINT);
			SetDlgItemText(IDC_SALES_RECEIPT_PRINT, "Print");
		}		

		//Initialize the datalists
		m_pCharges = BindNxDataList2Ctrl(IDC_SALES_RECEIPT_CHARGES, GetRemoteData(), FALSE);
		m_pPayments = BindNxDataList2Ctrl(IDC_SALES_RECEIPT_PAYMENTS, GetRemoteData(), FALSE);

		// (j.jones 2015-03-19 16:51) - PLID 65148 - hide original and void items
		CString strChargesFrom = "LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID ";
		CString strPaymentsFrom = "LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID ";
		
		// (a.walling 2008-07-07 17:20) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		CString strWhere;
		strWhere.Format(" LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND LineItemT.PatientID = %li "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null", m_nPatientID);
		m_pCharges->FromClause = _bstr_t(strChargesFrom);
		m_pCharges->WhereClause = _bstr_t(strWhere);

		strWhere.Format(" LineItemT.Deleted = 0 AND LineItemT.Type < 10 AND LineItemT.PatientID = %li "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null", m_nPatientID);
		m_pPayments->FromClause = _bstr_t(strPaymentsFrom);
		m_pPayments->WhereClause = _bstr_t(strWhere);

		m_pCharges->Requery();
		m_pPayments->Requery();
		
		if (m_bPreview) {
			SetDlgItemText(IDC_SALES_RECEIPT_PRINT, "Preview");
		}
		else {
			SetDlgItemText(IDC_SALES_RECEIPT_PRINT, "Print");
		}
	}
	NxCatchAll("Error in CSalesReceiptConfigDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CSalesReceiptConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSalesReceiptConfigDlg)
	ON_EVENT(CSalesReceiptConfigDlg, IDC_SALES_RECEIPT_PAYMENTS, 18 /* RequeryFinished */, OnRequeryFinishedSalesReceiptPayments, VTS_I2)
	ON_EVENT(CSalesReceiptConfigDlg, IDC_SALES_RECEIPT_CHARGES, 18 /* RequeryFinished */, OnRequeryFinishedSalesReceiptCharges, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSalesReceiptConfigDlg::OnRequeryFinishedSalesReceiptPayments(short nFlags) 
{
	//check the correct boxes
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPayments->GetFirstRow();
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	while(pRow){		
		
		COleDateTime dt =  VarDateTime(pRow->GetValue(2));

		if (dt.GetYear() == m_dtPay.GetYear() && dt.GetMonth() == m_dtPay.GetMonth() && 
			dt.GetDay() == m_dtPay.GetDay()) {
			pRow->PutValue(1, varTrue);
		}
		else {
			pRow->PutValue(1, varFalse);
		}
		pRow = pRow->GetNextRow();
	}	
	
}

void CSalesReceiptConfigDlg::OnRequeryFinishedSalesReceiptCharges(short nFlags) 
{
	//check the correct boxes
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCharges->GetFirstRow();
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	while(pRow){	
			
		COleDateTime dt =  VarDateTime(pRow->GetValue(2));

		if (dt.GetYear() == m_dtPay.GetYear() && dt.GetMonth() == m_dtPay.GetMonth() && 
			dt.GetDay() == m_dtPay.GetDay()) {
			pRow->PutValue(1, varTrue);
		}
		else {
			pRow->PutValue(1, varFalse);
		}
		pRow = pRow->GetNextRow();
	}	
	
}


CString CSalesReceiptConfigDlg::GenerateIDList() {


	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCharges->GetFirstRow();
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	CString strIDs;

	while(pRow){	
		
		if (VarBool(pRow->GetValue(1))) {
			strIDs += AsString(pRow->GetValue(0)) + ", ";
		}
		
		pRow = pRow->GetNextRow();
	}	

	pRow = m_pPayments->GetFirstRow();
	while(pRow){	
		
		if (VarBool(pRow->GetValue(1))) {
			strIDs += AsString(pRow->GetValue(0)) + ", ";
		}
		
		pRow = pRow->GetNextRow();
	}	

	//take off the last ,
	strIDs.TrimRight();
	strIDs.TrimLeft();
	strIDs = strIDs.Left(strIDs.GetLength() - 1);

	return strIDs;


}


void CSalesReceiptConfigDlg::OnSalesReceiptPrint() 
{

	//generate the IDs for the query
	CString strIDs = GenerateIDList();

	if (strIDs.IsEmpty() ) {

		MsgBox("Please select at least one charge or payment to print");
		return;
	}

	long nReportID;
	if (m_nReportID == -3) {
		// (j.gruber 2007-05-08 09:57) - PLID 9802 - taking out the 587 report since we are doing it all POS
		//if (GetRemotePropertyInt("PrintPOSStyle", 0, 0, "<None>", true)) {
			CString strLineItemFilter, strPaymentFilter;

			// (j.jones 2009-11-10 17:33) - PLID 34165 - the line item filter goes into a query
			// that already has an IN clause (and expects a trailing comma), whereas the payment
			// filter needs an IN clause
			strLineItemFilter = strIDs + ",";
			strPaymentFilter = " AND PaymentID IN (" + strIDs + ") ";
			PrintSalesReceiptToReceiptPrinter(strLineItemFilter, strPaymentFilter, strIDs, m_nNumber);
			EndDialog(m_bPreview);
			return;
		//}
		/*else {
			nReportID = 587;
		}*/
	}
	else if (m_nReportID == -2) {
		nReportID = 585;
	}
	else {
		nReportID = m_nReportID;
	}
	// (j.gruber 2007-03-29 14:48) - PLID 9802 - need to change some things around to accomodate the receipt printer format
	CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)];

	// (j.jones 2009-11-10 17:33) - PLID 34165 - the line item filter (strExtraText) goes into a query
	// that already has an IN clause (and expects a trailing comma), whereas the payment filter (strExtendedSql)
	// needs an IN clause
	infReport.strExtraText = strIDs + ",";
	infReport.strExtendedSql = " AND PaymentID IN (" + strIDs + ") ";

	CPtrArray params;
	CRParameterInfo *tmpParam;

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "ReceiptCustomInfo";
	tmpParam->m_Data = GetRemotePropertyText("ReceiptCustomInfo", "", 0, "<None>", true);
	params.Add((void *)tmpParam);

	//see if they are printing or previewing
	CPrintInfo prInfo;
	if (!m_bPreview) {

		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if (prInfo.m_pPD) delete prInfo.m_pPD;
		prInfo.m_pPD = dlg;
	}


	//see if there was a number
	if (m_nNumber > 0) {

		//its a custom report, look up the filename
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Filename from CustomReportsT WHERE ID = 585 AND Number = %li", m_nNumber);
		CString strFileName;
		if (! rs->eof) {
			strFileName = AdoFldString(rs, "FileName", "");
			infReport.nDefaultCustomReport = m_nNumber;
		}
		else {
			strFileName = infReport.strReportFile;
		}
		rs->Close();

		if (m_bPreview) {
			infReport.ViewReport(infReport.strPrintName, strFileName, &params, TRUE, this);
		}
		else {
			infReport.ViewReport(infReport.strPrintName, strFileName, &params, FALSE, this, &prInfo);
		}



	}
	else {

		//it's a system report!
		if (m_bPreview) {
			infReport.ViewReport(infReport.strPrintName, infReport.strReportFile, &params, TRUE, this);
		}
		else {
			infReport.ViewReport(infReport.strPrintName, infReport.strReportFile, &params, FALSE, this, &prInfo);
		}
	}

	ClearRPIParameterList(&params);	
	EndDialog(m_bPreview);		

}

void CSalesReceiptConfigDlg::OnCancel() 
{
		
	CDialog::OnCancel();
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK and OnCancel handlers to keep behavior the same as pre-NxDialog changes
void CSalesReceiptConfigDlg::OnOK()
{
	//Eat the message
}

// (j.gruber 2007-07-16 10:38) - PLID 26686 - all sales receipt printer functionality moved to GlobalReportUitls

