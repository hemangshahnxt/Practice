// LockBoxPaymentImportDlg.cpp : implementation file
//

// (d.singleton 2014-07-11 11:13) - PLID 62862 - create new dialog that will import a lockbox payment file

#include "stdafx.h"
#include "FinancialRc.h"
#include "Practice.h"
#include "LockBoxPaymentImportDlg.h"
#include "afxdialogex.h"
#include "PayCatDlg.h"
#include "ANSI823Parser.h"
#include "GlobalFinancialUtils.h"
#include "SelectDlg.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "BatchPayments.h"
#include "AuditTrail.h"

#include "EditComboBox.h"


// CLockBoxPaymentImportDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;
using namespace LockBox;
using namespace WaitingOnImportDialog; 

// (b.spivey, September 22, 2014) - PLID 62924 - Holder to close the modal dialog. 
namespace WaitingOnImportDialog
{
	Holder* pHeld = nullptr;

	Holder::Holder(CWnd* pWnd) : pWnd(pWnd)
	{
		ASSERT(!pHeld);
		pHeld = this;
	}

	Holder::~Holder()
	{
		ASSERT(pHeld == this);
		pHeld = nullptr;
	}

	void Close()
	{
		if (!pHeld) return;

		::PostMessage(pHeld->pWnd->GetSafeHwnd(), WM_COMMAND, IDCANCEL, 0);
	}
}


enum PaymentCategory {
	pcID = 0,
	pcName,
};

enum PaymentDescription {
	pdName = 0,
};

enum PaymentList {
	plID = 0,
	plPatientID,
	plPatientName,
	plCheckAmount,
	plCheckNumber,
	plAccountNumber,
	plRoutingNumber,
	plPatientBalance,
	plSkip,
	plManual,
	plPaymentID,
	plLockboxPatientID,
	plPersonID,	
	plOriginalAmt,
	plPostedManual,
};

enum PaymentLocation {
	plocID = 0,
	plocName,
};

enum PaymentProvider {
	ppID = 0,
	ppName,
};

IMPLEMENT_DYNAMIC(CLockBoxPaymentImportDlg, CNxDialog)

CLockBoxPaymentImportDlg::CLockBoxPaymentImportDlg(CWnd* pParent /*=NULL*/, long nBatchID /*=-1*/)
	: CNxDialog(CLockBoxPaymentImportDlg::IDD, pParent)
{
	m_nPaymentCatID = -1;
	m_nBatchID = nBatchID;
	m_cyPostedAmount = g_ccyZero;
	m_strAnsi823FilePath = "";
}

CLockBoxPaymentImportDlg::~CLockBoxPaymentImportDlg()
{
}

void CLockBoxPaymentImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//buttons
	DDX_Control(pDX, IDC_IMPORT_PAYMENT_FILE, m_nxbImport);
	DDX_Control(pDX, IDC_PAYMENT_DESCRIPTION_EDIT, m_nxbDescriptionEdit);
	DDX_Control(pDX, IDC_PAYMENT_CATEGORY_EDIT, m_nxbCategoryEdit);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_POST_PAYMENT, m_nxbPost);
	//checkboxes
	DDX_Control(pDX, IDC_PAYMENT_DATE_ENABLE, m_nxbPaymentDate);
	DDX_Control(pDX, IDC_CHECK_NUMBER, m_nxbCheckNumber);
	DDX_Control(pDX, IDC_ACCOUNT_NUMBER, m_nxbAccountNumber);
	DDX_Control(pDX, IDC_ROUTING_NUMBER, m_nxbRoutingNumber);
	DDX_Control(pDX, IDC_MARK_AS_DEPOSITED, m_nxbMarkAsDeposited);
	//date time picker
	DDX_Control(pDX, IDC_PAYMENT_DATE, m_dtpPaymentDate);
	//edit controls
	DDX_Control(pDX, IDC_BANK_NAME, m_eBankInstitution);
	DDX_Control(pDX, IDC_DEPOSIT_AMOUNT, m_ePostedAmount); 
	DDX_Control(pDX, IDC_PAYMENT_DESCRIPTION_TEXT, m_ePaymentDescText); 
	DDX_Control(pDX, IDC_LOCKBOX_PROGRESS_TEXT, m_eProgressText);
	DDX_Control(pDX, IDC_TOTAL_AMOUNT, m_eDepositAmount);
	//progress bar
	DDX_Control(pDX, IDC_LOCKBOX_PROGRESS_BAR, m_pcProgressBar);
}


BEGIN_MESSAGE_MAP(CLockBoxPaymentImportDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, &CLockBoxPaymentImportDlg::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_IMPORT_PAYMENT_FILE, &CLockBoxPaymentImportDlg::OnBnClickedImportPaymentFile)
	ON_BN_CLICKED(IDC_POST_PAYMENT, &CLockBoxPaymentImportDlg::OnBnClickedPostPayment)
	ON_BN_CLICKED(IDC_PAYMENT_CATEGORY_EDIT, &CLockBoxPaymentImportDlg::OnBnClickedPaymentCategoryEdit)
	ON_BN_CLICKED(IDC_PAYMENT_DESCRIPTION_EDIT, &CLockBoxPaymentImportDlg::OnBnClickedPaymentDescriptionEdit)
	ON_BN_CLICKED(IDC_MARK_AS_DEPOSITED, &CLockBoxPaymentImportDlg::OnBnClickedMarkAsDeposited)
	ON_BN_CLICKED(IDC_CHECK_NUMBER, &CLockBoxPaymentImportDlg::OnBnClickedCheckNumber)
	ON_BN_CLICKED(IDC_ACCOUNT_NUMBER, &CLockBoxPaymentImportDlg::OnBnClickedAccountNumber)
	ON_BN_CLICKED(IDC_ROUTING_NUMBER, &CLockBoxPaymentImportDlg::OnBnClickedRoutingNumber)
	ON_BN_CLICKED(IDC_PAYMENT_DATE_ENABLE, &CLockBoxPaymentImportDlg::OnBnClickedPaymentDate)
END_MESSAGE_MAP()

BOOL CLockBoxPaymentImportDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {	
		g_propManager.CachePropertiesInBulk("LockboxPayments", propNumber,
			"(UserName = '<None>' OR UserName = '%s') AND Name IN ("
			"'LockboxPaymentMarkAsDeposited', "
			"'LockboxPaymentAddCheckNumber', "
			"'LockboxPaymentAddAccountNumber', "
			"'LockboxPaymentAddRoutingNumber')", GetCurrentUserName());

		m_nxbCancel.AutoSet(NXB_CANCEL);
		m_nxbImport.AutoSet(NXB_MODIFY);
		m_nxbPost.AutoSet(NXB_MODIFY);

		m_dlProviderList = BindNxDataList2Ctrl(IDC_LOCKBOX_PROVIDER_LIST, true);
		m_dlLocationList = BindNxDataList2Ctrl(IDC_LOCKBOX_LOCATION_LIST, true);
		m_dlDescriptionList = BindNxDataList2Ctrl(IDC_PAYMENT_DESCRIPTION, true);
		m_dlCategoryList = BindNxDataList2Ctrl(IDC_PAYMENT_CATEGORY, true);
		m_dlPaymentList = BindNxDataList2Ctrl(IDC_PAYMENT_LIST, false);

		m_pcProgressBar.SetPos(0);
		m_eProgressText.SetWindowText("Ready to import.");

		//recreate the <nothing selected> row
		IRowSettingsPtr pRow = m_dlCategoryList->GetNewRow();
		pRow->PutValue(pcID, -1);
		pRow->PutValue(pcName, _bstr_t("< No Category Selected >"));
		m_dlCategoryList->AddRowSorted(pRow, NULL);	

		// (d.singleton 2014-08-05 10:06) - PLID 62955 - add a checkbox to the bottom of the Lockbox payment dlg called "Mark payments as deposited"
		long nDeposited, nCheckNum, nAccountNum, nRoutingNum;
		nDeposited = GetRemotePropertyInt("LockboxPaymentMarkAsDeposited", 0, 0, "<None>", true);
		m_nxbMarkAsDeposited.SetCheck(nDeposited);
		// (d.singleton 2014-08-05 10:08) - PLID 62951 - Add the following three checkboxes to the lockbox payment dialog use these as the payment description
		nCheckNum = GetRemotePropertyInt("LockboxPaymentAddCheckNumber", 0, 0, GetCurrentUserName(), true);
		m_nxbCheckNumber.SetCheck(nCheckNum);
		nAccountNum = GetRemotePropertyInt("LockboxPaymentAddAccountNumber", 0, 0, GetCurrentUserName(), true);
		m_nxbAccountNumber.SetCheck(nAccountNum);
		nRoutingNum = GetRemotePropertyInt("LockboxPaymentAddRoutingNumber", 0, 0, GetCurrentUserName(), true);
		m_nxbRoutingNumber.SetCheck(nRoutingNum);

		//default our date to unchecked
		m_dtpPaymentDate.EnableWindow(m_nxbPaymentDate.GetCheck());

		//if we are loading from the edit lockbox payment dlg then load the payments from the provided batch id
		if (m_nBatchID > 0) {
			LoadFromBatchID();
			EnableButtons(FALSE);
		}
		else {
			// (d.singleton 2014-07-22 09:43) - PLID 62918 if this is a new deposit,  ask for file and parse
			ParseAndLoadAnsi823File();
		}

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CLockBoxPaymentImportDlg::OnDestroy()
{
	try {
		CNxDialog::OnDestroy();

		GetMainFrame()->PostMessage(NXM_LOCKBOX_PAYMENT_IMPORT_CLOSED);
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-12 12:09) - PLID 63259 - make font red if the deposit amount goes negative
HBRUSH CLockBoxPaymentImportDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_DEPOSIT_AMOUNT) {		
		CString strPostedAmount, strDepositAmount;
		COleCurrency cyPostedAmount, cyDepositAmount;
		m_ePostedAmount.GetWindowText(strPostedAmount);
		m_eDepositAmount.GetWindowText(strDepositAmount);
		cyPostedAmount.ParseCurrency(strPostedAmount);
		cyDepositAmount.ParseCurrency(strDepositAmount);
		//if our current amount and total amounts do not match, show red
		if (cyPostedAmount.GetStatus() == COleCurrency::valid &&
			cyDepositAmount.GetStatus() == COleDateTime::valid &&
			cyPostedAmount != cyDepositAmount) {
			pDC->SetTextColor(Red);
		}
		//if not negative set back to black
		else {
			pDC->SetTextColor(Black);
		}
	}
	return hbr;
}

void CLockBoxPaymentImportDlg::EnableButtons(BOOL bEnable)
{
	m_nxbImport.EnableWindow(bEnable);
	m_nxbDescriptionEdit.EnableWindow(bEnable);
	m_nxbCategoryEdit.EnableWindow(bEnable);
	GetDlgItem(IDC_LOCKBOX_PROVIDER_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_LOCKBOX_LOCATION_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_PAYMENT_DESCRIPTION)->EnableWindow(bEnable);
	GetDlgItem(IDC_PAYMENT_CATEGORY)->EnableWindow(bEnable);
}

// (d.singleton 2014-08-11 16:23) - PLID 63316 - need to load and populate the lockbox payment import dlg using an existing batch id 
void CLockBoxPaymentImportDlg::LoadFromBatchID()
{
	try {
		_RecordsetPtr prs = CreateParamRecordset(
			//Get top level data that applies to all payments from the lockbox file		
			//then get the data for all the payments
			R"(SELECT Description, BankName, Amount, PaymentDate, LocationID, ProviderID FROM LockboxBatchT WHERE ID = {INT}; 
			
			SELECT LockboxPaymentT.ID, LockboxPaymentT.Description, LockboxPaymentT.Amount, LockboxPaymentT.RoutingNumber, LockboxPaymentT.AccountNumber, 
			LockboxPaymentT.CheckNumber, LockboxPaymentT.Manual, LockboxPaymentT.PatientID AS LockboxPatientID, PatientsT.UserdefinedID, 
			PaymentsQ.PatientID AS PersonID, PaymentsQ.PaymentID
			FROM LockboxPaymentT 			
			LEFT JOIN (SELECT LockboxPaymentMapT.PaymentID, LockboxPaymentMapT.LockboxPaymentID, LineItemT.PatientID FROM LockboxPaymentMapT 
				INNER JOIN LineItemT ON LockboxPaymentMapT.PaymentID = LineItemT.ID AND LineItemT.Deleted = 0) PaymentsQ 
			ON LockboxPaymentT.ID = PaymentsQ.LockboxPaymentID
			LEFT JOIN PatientsT ON PaymentsQ.PatientID = PatientsT.PersonID
			WHERE LockboxBatchID = {INT})", m_nBatchID, m_nBatchID);

		if (prs->eof) {
			CString strError;
			strError.Format("Failed to load info for Lockbox payment ID %li", m_nBatchID);
			AfxMessageBox(strError);
			return;
		}
		else {
			//load data into controls
			m_eBankInstitution.SetWindowText(AdoFldString(prs, "BankName", ""));

			COleCurrency cyDepositAmount =  AdoFldCurrency(prs, "Amount");
			//set posted amount to total,  we will calculate as we go
			//m_ePostedAmount.SetWindowText(FormatCurrencyAsString(m_cyPostedAmount, true));
			m_eDepositAmount.SetWindowText(FormatCurrencyAsString(cyDepositAmount, true));

			m_dtpPaymentDate.SetTime(AdoFldDateTime(prs, "PaymentDate", g_cdtNull));
			m_ePaymentDescText.SetWindowText(AdoFldString(prs, "Description", ""));

			long nLocationID = AdoFldLong(prs, "LocationID", -1);
			m_dlLocationList->SetSelByColumn(plocID, nLocationID);

			long nProviderID = AdoFldLong(prs, "ProviderID", -1);
			m_dlProviderList->SetSelByColumn(plID, nProviderID);
		}

		//get the next recordset and load that into our payment list
		prs = prs->NextRecordset(NULL);
		while (!prs->eof) {
			IRowSettingsPtr pRow = m_dlPaymentList->GetNewRow();
			pRow->PutValue(plID, AdoFldLong(prs, "ID"));

			//get the patient
			//if we have a LineItemT.PatientID then we already have a matched up patient
			//so make sure to load the patient name and balance.
			PatientInfo patient;			
			patient.strLockboxPatientID = AdoFldString(prs, "LockboxPatientID");
			patient.nPersonID = AdoFldLong(prs, "PersonID", -1);
			patient.nUserdefinedID = AdoFldLong(prs, "UserDefinedID", -1);
			if (patient.nPersonID == -1) {
				//this payment doesnt have a linked practice patient,  use lockboxid to find one
				FindPatientInPractice(patient);
			}
			if (patient.nPersonID != -1) {
				//we have our patient now so load the data
				pRow->PutValue(plPersonID, patient.nPersonID);
				pRow->PutValue(plPatientID, patient.nUserdefinedID);
				//get patient name if empty
				if (patient.strPatientName.IsEmpty()) { patient.strPatientName = GetExistingPatientName(patient.nPersonID); }
				pRow->PutValue(plPatientName, _bstr_t(patient.strPatientName));
				COleCurrency cBalance = FindPatientBalance(patient.nPersonID);
				pRow->PutValue(plPatientBalance, COleVariant(cBalance));
			}
			else {				
				//still couldnt find a patient,  so load no patient
				pRow->PutValue(plPersonID, g_cvarNull);
				pRow->PutValue(plPatientID, g_cvarNull);
				pRow->PutValue(plPatientName, _bstr_t("< Select a Patient >"));
				pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleTrue);
				pRow->PutValue(plPatientBalance, g_cvarNull);
			}							
			pRow->PutValue(plLockboxPatientID, _bstr_t(patient.strLockboxPatientID));
			pRow->PutValue(plSkip, g_cvarFalse);
			//add check/payment data,  add to our posted amount
			COleCurrency cyAmount = AdoFldCurrency(prs, "Amount", COleCurrency(0, 0));
			m_cyPostedAmount += cyAmount;
			pRow->PutValue(plCheckAmount, COleVariant(cyAmount));
			pRow->PutValue(plOriginalAmt, COleVariant(cyAmount));
			pRow->PutValue(plCheckNumber, _bstr_t(AdoFldString(prs, "CheckNumber", "")));
			pRow->PutValue(plAccountNumber, _bstr_t(AdoFldString(prs, "AccountNumber", "")));
			pRow->PutValue(plRoutingNumber, _bstr_t(AdoFldString(prs, "RoutingNumber", "")));
			//manual status
			bool bManual = !!AdoFldBool(prs, "Manual", FALSE);
			pRow->PutValue(plManual, _variant_t(bManual));
			pRow->PutValue(plPostedManual, _variant_t(bManual));
			if (bManual) {
				//disable this row and subtract from out dep amt
				pRow->PutBackColor(Grey);
				m_ePostedAmount.SetWindowText(FormatCurrencyAsString(m_cyPostedAmount, true));
			}			
			//linked payment id's
			long nPaymentID = AdoFldLong(prs, "PaymentID", -1);
			if (nPaymentID == -1) {
				pRow->PutValue(plPaymentID, g_cvarNull);
			}
			else {
				pRow->PutValue(plPaymentID, nPaymentID);
				pRow->PutBackColor(Grey);
				//this payment cannot be deposited again,  so sub from our deposit total
				m_ePostedAmount.SetWindowText(FormatCurrencyAsString(m_cyPostedAmount, true));
			}
			m_dlPaymentList->AddRowAtEnd(pRow, NULL);
			
			prs->MoveNext();
		}
		//update our posted amount
		m_ePostedAmount.SetWindowText(FormatCurrencyAsString(m_cyPostedAmount, true));

		if (m_dlPaymentList->GetRowCount() > 0) {
			m_eProgressText.SetWindowText("Ready to post.");
		}		

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-07-22 09:43) - PLID 62918 - promt the user to select an import file as soon as the lockbox dialog opens,  instead of having the extra button to click
void CLockBoxPaymentImportDlg::ParseAndLoadAnsi823File()
{
	try {
		//warn if there are already records imported to the datalist
		if (m_dlPaymentList->GetRowCount() > 0) {
			if (IDYES != MessageBox("There are already records ready for posting. Are you sure you wish to clear the list and import a new file?", 
				"Nextech Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
				return;
			}
			else {
				m_dlPaymentList->Clear();
			}
		}

		CANSI823Parser parser(this, &m_pcProgressBar, &m_eProgressText);

		//parse the file
		if (!parser.ParseFile()) {
			m_pcProgressBar.SetPos(0);
			m_eProgressText.SetWindowText("");
			return;
		}
		m_strAnsi823FilePath = parser.m_strAnsi823FileName;
		//load the deposit data
		m_eBankInstitution.SetWindowText(parser.m_ldiDeposit.strBankName);
		
		// (d.lange 2015-07-16 11:19) - PLID 66129 - Change the Deposit Amount to reflect the total deposit amount 
		// from the parsed lockbox file
		m_eDepositAmount.SetWindowText(FormatCurrencyAsString(parser.m_ldiDeposit.cAmount, true));
		m_dtpPaymentDate.SetTime(parser.m_ldiDeposit.dtDepositDate);

		// Represents the total payment amount contained in the lockbox file
		COleCurrency cyTotalPaymentAmount;

		//load the payment data
		for (LockboxPaymentInfoPtr& pPayment : parser.m_ldiDeposit.aryLockboxPayments) {
			IRowSettingsPtr pRow = m_dlPaymentList->GetNewRow();
			pRow->PutValue(plID, g_cvarNull);
			//lets find that patient
			PatientInfo patient;
			patient.strLockboxPatientID = pPayment->strPatientID;
			FindPatientInPractice(patient);
			if (patient.nPersonID > 0) {
				pRow->PutValue(plPersonID, patient.nPersonID);
				pRow->PutValue(plPatientID, patient.nUserdefinedID);
				pRow->PutValue(plLockboxPatientID, _bstr_t(patient.strLockboxPatientID));
				pRow->PutValue(plPatientName, _bstr_t(patient.strPatientName));
				//get the patient balance
				COleCurrency cBalance = FindPatientBalance(patient.nPersonID);
				pRow->PutValue(plPatientBalance, COleVariant(cBalance));
			}
			else {
				pRow->PutValue(plPersonID, g_cvarNull);
				pRow->PutValue(plPatientID, g_cvarNull);
				pRow->PutValue(plLockboxPatientID, _bstr_t(patient.strLockboxPatientID));
				pRow->PutValue(plPatientName, _bstr_t("< Select a Patient >"));
				pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleTrue);
				pRow->PutValue(plPatientBalance, COleVariant(COleCurrency(0, 0)));
			}

			// Add the total payment amount for each lockbox payment
			cyTotalPaymentAmount += pPayment->cAmount;

			//add the check/payment data
			pRow->PutValue(plCheckAmount, COleVariant(pPayment->cAmount));
			pRow->PutValue(plOriginalAmt, COleVariant(pPayment->cAmount));
			pRow->PutValue(plCheckNumber, _bstr_t(pPayment->strCheckNumber));
			pRow->PutValue(plAccountNumber, _bstr_t(pPayment->strAccountNumber));
			pRow->PutValue(plRoutingNumber, _bstr_t(pPayment->strRoutingNumber));
			pRow->PutValue(plSkip, g_cvarFalse);
			pRow->PutValue(plPaymentID, g_cvarNull);
			pRow->PutValue(plManual, g_cvarFalse);
			pRow->PutValue(plPostedManual, g_cvarFalse);
			m_dlPaymentList->AddRowAtEnd(pRow, NULL);
		}

		// (d.lange 2015-07-16 11:20) - PLID 66129 - Update the Posted Amount to reflect the total payment 
		// amount from the Payment Amount column
		m_cyPostedAmount = cyTotalPaymentAmount;;
		m_ePostedAmount.SetWindowText(FormatCurrencyAsString(m_cyPostedAmount, true));

		//reset our progress and text
		m_pcProgressBar.SetPos(0);
		m_eProgressText.SetWindowText("Import Complete.");
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-07-23 15:57) - PLID 62943 - when parsing the ansi 823 file and no patient is found for that payment,  leave the patient column blank.  add context menu to manually add one
void CLockBoxPaymentImportDlg::FindPatientInPractice(LockBox::PatientInfo& patient)
{
	if (patient.strLockboxPatientID.GetLength() > 0) {
		//make sure our id is numeric
		// (b.spivey January 20, 2015) - PLID 64560 - Properly convert to base 10. 
		long nUserdefinedID = strtol(patient.strLockboxPatientID, NULL, 10);
		if (nUserdefinedID == 0) {
			//conversion failed
			return;
		}
		_RecordsetPtr prs = CreateParamRecordset("SELECT ID, UserdefinedID FROM PersonT "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE UserdefinedID = {INT}", nUserdefinedID);

		if (prs->eof) {
			return;
		}
		
		patient.nUserdefinedID = AdoFldLong(prs, "UserdefinedID", -1);
		patient.nPersonID = AdoFldLong(prs, "ID", -1);
		patient.strPatientName = GetExistingPatientName(patient.nPersonID);
	}
	else if(patient.nPersonID > 0) {
		//if we dont have a lockbox patient id and have a nextech personid use that to get userdefined and patient name.
		patient.nUserdefinedID = GetExistingPatientUserDefinedID(patient.nPersonID);
		patient.strPatientName = GetExistingPatientName(patient.nPersonID);
		return;
	}	
}

//get the patient balance
COleCurrency CLockBoxPaymentImportDlg::FindPatientBalance(const long& nPersonID)
{
	COleCurrency cBalance;
	GetPatientTotal(nPersonID, &cBalance);

	return cBalance;
}

// (d.singleton 2014-08-12 08:59) - PLID 62943 - if no patient is found for that payment,  leave the patient column blank.  add context menu to manually add one
void CLockBoxPaymentImportDlg::ChoosePatient(IRowSettingsPtr& pRow)
{
	//pop up a select dlg with patient info
	CSelectDlg dlg(this);
	dlg.m_strTitle = "Select a patient";
	dlg.m_strCaption = "Select a patient to associate with this payment.";
	dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
	// Exclude the -25 built-in patient, and inquiries
	dlg.m_strWhereClause = " PersonT.ID >= 0 AND CurrentStatus <> 4";

	dlg.AddColumn("ID", "ID", FALSE, FALSE);
	dlg.AddColumn("UserDefinedID", "PatientID", TRUE, FALSE, TRUE);
	dlg.AddColumn("Last", "Last", TRUE, FALSE, TRUE);
	dlg.AddColumn("First", "First", TRUE, FALSE, TRUE);
	dlg.AddColumn("Middle", "Middle", TRUE, FALSE, TRUE);	
	dlg.AddColumn("BirthDate", "Birth Date", TRUE, FALSE, TRUE);
	dlg.AddColumn("(CASE WHEN Gender = 2 THEN 'F' WHEN Gender = 1 THEN 'M' END)", "Gender", TRUE, FALSE, TRUE);
	dlg.AddColumn("Address1", "Address1", TRUE, FALSE, TRUE);
	dlg.AddColumn("Address2", "Address2", TRUE, FALSE, TRUE);
	dlg.AddColumn("City", "City", TRUE, FALSE, TRUE);
	dlg.AddColumn("State", "State", TRUE, FALSE, TRUE);
	dlg.AddColumn("Zip", "Zip", TRUE, FALSE, TRUE);

	//get the patient they selected and update our row
	if (dlg.DoModal() == IDOK) {
		PatientInfo patient;
		patient.nPersonID = VarLong(dlg.m_arSelectedValues[0], -1);
		patient.nUserdefinedID = VarLong(dlg.m_arSelectedValues[1], -1);
		COleCurrency cyBalance;
		if (patient.nPersonID != -1) {			
			FindPatientInPractice(patient);
			cyBalance = FindPatientBalance(patient.nPersonID);
			pRow->PutValue(plPersonID, patient.nPersonID);
			pRow->PutValue(plPatientID, patient.nUserdefinedID);
			pRow->PutValue(plPatientName, _bstr_t(patient.strPatientName));
			pRow->PutValue(plPatientBalance, COleVariant(cyBalance));
			pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleFalse);
		}
	}
}

// (d.singleton 2014-08-12 08:53) - PLID 62893 - get all the applies for this payment and store in memory
void CLockBoxPaymentImportDlg::GetAppliesForPatientPayment(PaymentInfo& payInfo)
{
	try {
		//make a copy of the payment amount
		COleCurrency cyPayAmount = payInfo.cyPaymentAmount;
		BOOL bProviderFound = FALSE;		
		//get a recordset of this patients charges with only pat resp order by oldest first
		// (z.manning 2015-06-22 16:21) - PLID 66381 - I removed the join to ChargeRespDetailT as it would result
		// in duplicate records if a charge had more than one record there. Just order by line item date instead.
		_RecordsetPtr prs = CreateParamRecordset(R"(
SELECT ChargeID, ChargesT.DoctorsProviders AS ChargeProviderID, PatientsT.MainPhysician AS ProviderID,
	ChargeRespT.Amount - COALESCE(AppliesQ.SumOfApplies, 0) as RespBalance
FROM ChargeRespT
INNER JOIN ChargesT ON ChargeRespT.ChargeID = ChargesT.ID
INNER JOIN LineItemT ON ChargeRespT.ChargeID = LineItemT.ID
INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID
LEFT JOIN(SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID
LEFT JOIN(SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID
LEFT JOIN(SELECT Sum(Amount) as SumOfApplies, RespID FROM AppliesT GROUP BY RespID) AppliesQ ON ChargeRespT.ID = AppliesQ.RespID
WHERE InsuredPartyID IS NULL AND PatientID = {INT} AND LineItemT.Deleted = 0
	/*only bills, no quotes*/
	AND LineItemT.Type = 10
	/*no voids or corrected items*/
	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null
	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null
	/*only charges with balances*/
	AND ChargeRespT.Amount - COALESCE(AppliesQ.SumOfApplies, 0) > 0
ORDER BY LineItemT.Date
)"
			, payInfo.nPersonID);
		while (!prs->eof) {			
			long nChargeID = AdoFldLong(prs, "ChargeID");
			// (d.singleton 2014-08-12 08:49) - PLID 62921 - When posting a Lockbox payment use the provider from the drop down menu.
			// But only if the payment is not applied to a charge and the patient does not have a gen 1 provider
			if (!bProviderFound) {
				long nProviderID = AdoFldLong(prs, "ChargeProviderID", -1);
				if (nProviderID == -1) {
					//ok no provider on charge,  use gen 1, but if the next record has a provider use that one
					nProviderID = AdoFldLong(prs, "ProviderID", -1);
					if (nProviderID > 0) {						
						payInfo.nProviderID = nProviderID;
					}
				}
				else {
					//use the charge provider and stop looking for next records
					payInfo.nProviderID = nProviderID;
					bProviderFound = TRUE;
				}
			}
			//get our pat resp amount
			ApplyInfo applyData;
			COleCurrency cyRespAmount = AdoFldCurrency(prs, "RespBalance");
			if (cyRespAmount > COleCurrency(0, 0)) {
				//ok we need to apply to this charge until charge or payment is zero'd out
				if (cyRespAmount >= cyPayAmount) {
					applyData.cyApplyAmount = cyPayAmount;
					applyData.nChargeID = nChargeID;
					//add the apply to our array
					payInfo.aryApplies.push_back(applyData);
					//at this point our lockbox payment is zero so break out of function
					break;
				}
				else if (cyRespAmount < cyPayAmount) {
					//resp is less than the whole payment so zero out the charge
					applyData.cyApplyAmount = cyRespAmount;
					applyData.nChargeID = nChargeID;
					//add the apply to our array
					payInfo.aryApplies.push_back(applyData);
					//subtract the applied amount from the pay amount
					cyPayAmount -= cyRespAmount;
				}
			}			
			prs->MoveNext();
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-12 08:57) - PLID 63333 - need ability to split a lockbox payment into multiple payments
void CLockBoxPaymentImportDlg::SplitPayment(IRowSettingsPtr& pRow)
{
	try {
		IRowSettingsPtr pSplitRow = m_dlPaymentList->GetNewRow();
		pSplitRow->PutValue(plID, g_cvarNull);
		pSplitRow->PutValue(plPatientID, g_cvarNull);
		//add no patient value,  make link
		pSplitRow->PutValue(plPatientName, _bstr_t("< Select a Patient >"));
		pSplitRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleTrue);

		pSplitRow->PutValue(plCheckAmount, COleVariant(COleCurrency(0, 0)));
		pSplitRow->PutValue(plCheckNumber, pRow->GetValue(plCheckNumber));
		pSplitRow->PutValue(plAccountNumber, pRow->GetValue(plAccountNumber));
		pSplitRow->PutValue(plRoutingNumber, pRow->GetValue(plRoutingNumber));
		pSplitRow->PutValue(plPatientBalance, g_cvarNull);
		pSplitRow->PutValue(plSkip, g_cvarFalse);
		pSplitRow->PutValue(plPaymentID, g_cvarNull);
		pSplitRow->PutValue(plLockboxPatientID, g_cvarNull);
		pSplitRow->PutValue(plPersonID, g_cvarNull);
		pSplitRow->PutValue(plManual, g_cvarFalse);
		pSplitRow->PutValue(plPostedManual, g_cvarFalse);
		pSplitRow->PutValue(plOriginalAmt, COleVariant(COleCurrency(0, 0)));
		//add row after the parent row
		m_dlPaymentList->AddRowBefore(pSplitRow, pRow->GetNextRow());
		//set focus to pay amount of new row
		m_dlPaymentList->StartEditing(pSplitRow, plCheckAmount);

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-11 14:23) - PLID 63257 - Need the ability to post manually on a line item for lockbox payment
void CLockBoxPaymentImportDlg::PostManually(IRowSettingsPtr& pRow)
{	
	CString strPostedAmount;
	COleCurrency cyPostedAmount, cyPaymentAmount;
	m_ePostedAmount.GetWindowText(strPostedAmount);
	cyPaymentAmount = VarCurrency(pRow->GetValue(plCheckAmount), COleCurrency(0, 0));
	cyPostedAmount.ParseCurrency(strPostedAmount);
	BOOL bManual = VarBool(pRow->GetValue(plManual), FALSE);
	if (bManual) {
		//reset color on row
		pRow->PutBackColor(White);
		if (VarString(pRow->GetValue(plPatientName), "") == "< Select a Patient >") {
			pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleTrue);
		}
		pRow->PutValue(plManual, g_cvarFalse);
	}
	else {
		//first warn them
		AfxMessageBox("You have selected to manually post this line item.  It will not be posted to any patient account. You must manually post this payment.");
				
		//make our row read only
		pRow->PutBackColor(Grey);
		//take off the link
		pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleFalse);
		//uncheck skip just in case
		pRow->PutValue(plSkip, g_cvarFalse);
		pRow->PutValue(plManual, g_cvarTrue);
	}
}

// CLockBoxPaymentImportDlg message handlers

void CLockBoxPaymentImportDlg::OnBnClickedCancel()
{
	try {
		DestroyWindow();
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-07-15 16:35) - PLID 62896 - parse the data from an ANSI 823 file and show the data in a datalist
void CLockBoxPaymentImportDlg::OnBnClickedImportPaymentFile()
{
	try {		
		ParseAndLoadAnsi823File();
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-07-23 12:44) - PLID 62893 - be able to post all the payments from the payment list of the Lockbox import dlg and apply to pat resp
void CLockBoxPaymentImportDlg::OnBnClickedPostPayment()
{
	try {
		CWaitCursor cursor;

		IRowSettingsPtr pRow;
		CString strAccountNum, strCheckNum, strRoutingNum, strBankName, strBatchDescription, strBatchAmount, strLockboxPatientID;
		strAccountNum = strCheckNum = strRoutingNum = strBankName = strBatchDescription = strBatchAmount = strLockboxPatientID =  "";
		COleCurrency cPaymentAmount;
		COleDateTime dtPaymentDate, dtBatchDate;
		long nLocationID, nPaymentID, nProviderID, nUserdefinedID, nPaymentGroupID;
		nLocationID = nPaymentID = nProviderID = nUserdefinedID = nPaymentGroupID = -1;
		
		//get the deposit data from the dlg
		m_eDepositAmount.GetWindowText(strBatchAmount);
		m_eBankInstitution.GetWindowText(strBankName);
		m_ePaymentDescText.GetWindowText(strBatchDescription);
		m_dtpPaymentDate.GetTime(dtBatchDate);
		//validate the dialog data
		pRow = m_dlProviderList->GetCurSel();
		if (!pRow) {
			AfxMessageBox("Please select a provider before posting payments");
			return;
		}
		nProviderID = VarLong(pRow->GetValue(plID), -1);
		if (!dtBatchDate.GetStatus() == COleDateTime::valid) {
			AfxMessageBox("Please select a valid date before posting payments");
			return;
		}
		// (d.singleton 2014-09-25 13:49) - PLID 62922 - Have a location drop down menu on the lockbox payment dlg,  use this as the location of all the posted lockbox payments.
		pRow = m_dlLocationList->GetCurSel();
		if (!pRow) {
			AfxMessageBox("Please select a location before posting payments.");
			return;
		}
		nLocationID = VarLong(pRow->GetValue(plocID), -1);							

		pRow = m_dlCategoryList->GetCurSel();
		if (pRow) {
			nPaymentGroupID = VarLong(pRow->GetValue(pcID), 0);
		}
		else {
			nPaymentGroupID = 0;
		}
		//do we need to mark all payments as deposited?
		BOOL bIsDeposited = m_nxbMarkAsDeposited.GetCheck();

		//create the batch as needed
		if (m_nBatchID == -1) {
			CParamSqlBatch sqlBatch;
			sqlBatch.Declare("SET NOCOUNT ON");
			sqlBatch.Declare("BEGIN TRAN");
			sqlBatch.Declare("DECLARE @BatchID INT");

			//create the lockbox batch if it doesnt exist yet, store the ID
			sqlBatch.Add("INSERT INTO LockboxBatchT(Description, BankName, Amount, PaymentDate, LocationID, ProviderID) "
				"VALUES({STRING}, {STRING}, CONVERT(MONEY, {STRING}), {STRING}, {INT}, {INT})",
				strBatchDescription,
				strBankName,
				strBatchAmount,
				FormatDateTimeForSql(dtBatchDate, dtoDate),
				nLocationID,
				nProviderID);
			sqlBatch.Declare("SET @BatchID = SCOPE_IDENTITY()");
			sqlBatch.Declare("COMMIT TRAN");
			sqlBatch.Declare("SET NOCOUNT OFF");
			sqlBatch.Declare("SELECT @BatchID AS BatchID");
			_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());
			m_nBatchID = AdoFldLong(prs, "BatchID");
		}

		//import the rows from payment list as payment
		pRow = m_dlPaymentList->GetFirstRow();
		long nRowCount = m_dlPaymentList->GetRowCount();
		m_pcProgressBar.SetPos(0);
		m_pcProgressBar.SetRange(0, (short)nRowCount); 

		// (b.spivey, August 11, 2014) - PLID 62924 - Create the error file. 
		CFile	ErrorsFile;	
		CString pathname = GetPracPath(PracPath::SessionPath) ^ "lockboxerrors.txt";
		ErrorsFile.Open(pathname, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive);
			

		for (int i = 0; i < nRowCount; i++) {
			//we do not update commited payments or if they have skipped checked
			BOOL bSkip = VarBool(pRow->GetValue(plSkip));
			nPaymentID = VarLong(pRow->GetValue(plPaymentID), -1);
			BOOL bManual = VarBool(pRow->GetValue(plManual), FALSE); 
			if (bSkip && nPaymentID == -1 && !bManual) {
				// (b.spivey, August 11, 2014) - PLID 62924 - Create an entry for skipping. 
				CString str;
				str.Format(
					"The following payment was marked as skipped. You will need to manually post this payment to the patient balance. \r\n\r\n"
					"Patient ID:		%s \r\n"
					"Payment Amount:		%s \r\n"
					"Check Number:		%s \r\n"
					"Account Number:		%s \r\n"
					"Routing Number:		%s \r\n\r\n"
					"****************************************************************************************************************\r\n\r\n",
					VarString(pRow->GetValue(plLockboxPatientID), ""),
					FormatCurrencyForInterface(VarCurrency(pRow->GetValue(plCheckAmount), COleCurrency(0, 0))),
					VarString(pRow->GetValue(plCheckNumber), ""),
					VarString(pRow->GetValue(plAccountNumber), ""), 
					VarString(pRow->GetValue(plRoutingNumber), "")
					);
				ErrorsFile.Write(str, str.GetLength());
			}
			//first make the LockboxPaymentT record
			PaymentInfo payInfo;
			payInfo.cyPaymentAmount = VarCurrency(pRow->GetValue(plCheckAmount), COleCurrency(0, 0));
			payInfo.nPersonID = VarLong(pRow->GetValue(plPersonID), -1);
			payInfo.nProviderID = VarLong(m_dlProviderList->GetCurSel()->GetValue(ppID), -1);
			m_dtpPaymentDate.GetTime(payInfo.dtPaymentDate);
			payInfo.strRoutingNum = VarString(pRow->GetValue(plRoutingNumber), "");
			payInfo.strAccountNum = VarString(pRow->GetValue(plAccountNumber), "");
			payInfo.strCheckNum = VarString(pRow->GetValue(plCheckNumber), "");
			payInfo.bManual = VarBool(pRow->GetValue(plManual), FALSE);
			payInfo.nPaymentID = VarLong(pRow->GetValue(plPaymentID), -1);
			strLockboxPatientID = VarString(pRow->GetValue(plLockboxPatientID), "");
			nUserdefinedID = VarLong(pRow->GetValue(plPatientID), -1);

			CParamSqlBatch sqlPaymentBatch;
			sqlPaymentBatch.Declare("SET NOCOUNT ON");
			sqlPaymentBatch.Declare("BEGIN TRAN");
			sqlPaymentBatch.Declare("DECLARE @LockboxPaymentID INT");
			sqlPaymentBatch.Declare("DECLARE @LineItemID INT");
			sqlPaymentBatch.Declare("DECLARE @MarkDeposited INT");
			long nLockboxPaymentID = VarLong(pRow->GetValue(plID), -1);
			//create lockbox payment record if one does not already exist
			if (nLockboxPaymentID == -1) {
				sqlPaymentBatch.Add("INSERT INTO LockboxPaymentT(LockboxBatchID, Description, Amount, RoutingNumber, AccountNumber, CheckNumber, PatientID, Manual) "
					"VALUES({INT}, {STRING}, CONVERT(MONEY, {STRING}), {STRING}, {STRING}, {STRING}, {STRING}, {BIT})",
					m_nBatchID,
					strBatchDescription,
					FormatCurrencyForSql(payInfo.cyPaymentAmount),
					payInfo.strRoutingNum, payInfo.strAccountNum,
					payInfo.strCheckNum,
					strLockboxPatientID,
					payInfo.bManual);
				sqlPaymentBatch.Declare("SET @LockboxPaymentID = SCOPE_IDENTITY()");
			}
			else {
				if (payInfo.nPaymentID == -1 || !VarBool(pRow->GetValue(plPostedManual), FALSE)) {
					sqlPaymentBatch.Add("UPDATE LockboxPaymentT SET Amount = CONVERT(MONEY, {STRING}), PatientID = {STRING}, Manual = {BIT} "
						"WHERE ID = {INT} ",
						FormatCurrencyForSql(payInfo.cyPaymentAmount),
						strLockboxPatientID,
						payInfo.bManual,
						nLockboxPaymentID);
					//add this cause we use it later on
					sqlPaymentBatch.Add("SET @LockboxPaymentID = {INT}", nLockboxPaymentID);
				}
			}

			//if we have an actual patient we can create a payment and apply it to charges
			//do not do anything if we are skipping this payment
			if (payInfo.nPersonID > 0 && !bSkip && !payInfo.bManual && payInfo.nPaymentID == -1) {	
				// (d.singleton 2014-08-12 08:50) - PLID 62925 - When posting the payment, if the total patient responsibility is less
				// than the amount being applied prompt the user with a warning.
				COleCurrency cyPatientTotal;
				GetPatientTotal(payInfo.nPersonID, &cyPatientTotal);
				if (payInfo.cyPaymentAmount > cyPatientTotal) {
					CString strWarning;
					strWarning.Format("This patient, %s(%li) has %s of patient responsibility on their account, and the payment is "
						"for %s. Do you wish to continue posting this payment?", GetExistingPatientName(payInfo.nPersonID), nUserdefinedID, FormatCurrencyAsString(cyPatientTotal, true),
						FormatCurrencyAsString(payInfo.cyPaymentAmount, true));

					if (MessageBox(strWarning, "Nextech Practice", MB_YESNO | MB_ICONWARNING) != IDYES) {
						pRow = m_dlPaymentList->FindAbsoluteNextRow(pRow, g_cvarTrue);
						continue;
					}
				}
				//get the applies data,  this will also give us the correct provider ID	
				GetAppliesForPatientPayment(payInfo);
				
				//lets make the LineItemT and Payments record
				sqlPaymentBatch.Add("INSERT INTO LineItemT(PatientID, Type, Amount, Description, Date, InputDate, InputName, Deleted, LocationID) "
					"VALUES({INT}, 1, CONVERT(MONEY, {STRING}), {STRING}, {STRING}, GetDate(), {STRING}, 0, {INT})", 
					payInfo.nPersonID, 
					FormatCurrencyForSql(payInfo.cyPaymentAmount), 
					strBatchDescription, 
					FormatDateTimeForSql(payInfo.dtPaymentDate, dtoDateTime), 
					GetCurrentUserName(), 
					nLocationID);
				sqlPaymentBatch.Declare("SET @LineItemID = SCOPE_IDENTITY()");
				
				sqlPaymentBatch.Add("SET @MarkDeposited = {INT}", m_nxbMarkAsDeposited.GetCheck());
				//need to get unique id
				sqlPaymentBatch.Declare("DECLARE @nPaymentUniqueID INT\r\n"
					"INSERT INTO PaymentUniqueT DEFAULT VALUES\r\n"
					"SET @nPaymentUniqueID = SCOPE_IDENTITY()\r\n");

				sqlPaymentBatch.Add("INSERT INTO PaymentsT(ID, ProviderID, PaymentGroupID, InsuredPartyID, Deposited, DepositDate, PrePayment, "
					"PayMethod, CashReceived, PaymentUniqueID, SentToQB, DepositInputDate) "
					"VALUES(@LineItemID, {INT}, {INT}, -1, {INT}, CASE WHEN @MarkDeposited = 1 THEN {STRING} ELSE NULL END, 0, "
					"2, CONVERT(MONEY, { STRING }), @nPaymentUniqueID, 0, CASE WHEN @MarkDeposited = 1 THEN GETDATE() ELSE NULL END)", 
					payInfo.nProviderID, 
					nPaymentGroupID,
					bIsDeposited ? 1 : 0, 
					FormatDateTimeForSql(payInfo.dtPaymentDate, dtoDateTime),
					FormatCurrencyForSql(payInfo.cyPaymentAmount));
				//need to save the check number accountnumber etc.
				sqlPaymentBatch.Add("INSERT INTO PaymentPlansT(ID, CheckNo, CheckAcctNo, BankRoutingNum) "
					"VALUES(@LineItemID, {STRING}, {STRING}, {STRING})",
					(m_nxbCheckNumber.GetCheck() ? payInfo.strCheckNum : ""),
					(m_nxbAccountNumber.GetCheck() ? payInfo.strAccountNum : ""),
					(m_nxbRoutingNumber.GetCheck() ? payInfo.strRoutingNum : ""));
				//map the LineItem ID and the lockbox ID
				sqlPaymentBatch.Add("INSERT INTO LockboxPaymentMapT(LockboxPaymentID, PaymentID) "
					"VALUES(@LockboxPaymentID, @LineItemID)");

				//need to get the payment id for applies
				sqlPaymentBatch.Declare("COMMIT TRAN");
				sqlPaymentBatch.Declare("SET NOCOUNT OFF");
				sqlPaymentBatch.Declare("SELECT @LineItemID AS PaymentID");
				_RecordsetPtr prs = sqlPaymentBatch.CreateRecordset(GetRemoteData());
				payInfo.nPaymentID = AdoFldLong(prs, "PaymentID");

				//audit payment creation
				long nAuditID = BeginNewAuditEvent();
				CString strAuditDesc;
				strAuditDesc.Format("%s Payment", FormatCurrencyForInterface(payInfo.cyPaymentAmount, TRUE, TRUE));
				AuditEvent(payInfo.nPersonID, GetExistingPatientName(payInfo.nPersonID), nAuditID, aeiPaymentCreated, payInfo.nPaymentID, "", (strAuditDesc + " (Created by Lockbox posting)"), aepHigh, aetCreated);

				COleCurrency cyTotalApplies(0, 0);
				//now need do the applies
				for (ApplyInfo applyData : payInfo.aryApplies) {
					ApplyPayToBill(payInfo.nPaymentID, payInfo.nPersonID, applyData.cyApplyAmount, "Charge",  applyData.nChargeID, -1, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE);

					cyTotalApplies.operator+=(applyData.cyApplyAmount);
				}			

				// (b.spivey, August 11, 2014) - PLID 62924 - Create an entry for no payments applied. 
				if (cyTotalApplies.operator==(COleCurrency(0,0))) {
					CString str;
					str.Format(
						"The following payment posted to the patient's account, but was left unapplied. You will need to apply this payment to a charge manually. \r\n\r\n"
						"Patient ID:		%li \r\n"
						"Payment Amount:		%s \r\n"
						"Check Number:		%s \r\n"
						"Account Number:		%s \r\n"
						"Routing Number:		%s \r\n\r\n"
						"****************************************************************************************************************\r\n\r\n",
						VarLong(pRow->GetValue(plPatientID), -1),
						FormatCurrencyForInterface(VarCurrency(pRow->GetValue(plCheckAmount), COleCurrency(0, 0))),
						VarString(pRow->GetValue(plCheckNumber), ""),
						VarString(pRow->GetValue(plAccountNumber), ""),
						VarString(pRow->GetValue(plRoutingNumber), "")
						);
					ErrorsFile.Write(str, str.GetLength());
				}
				// (b.spivey, August 11, 2014) - PLID 62924 - Create an entry for payment portion left unapplied. 
				else if (payInfo.cyPaymentAmount > cyTotalApplies) {
					CString str;
					str.Format(
						"A portion of the following payment was posted to a patient balance, but a portion was left unapplied due to no remaining "
						"patient balances. Please review this patient's account to ensure that this payment was properly posted. \r\n\r\n"
						"Patient ID:		%li \r\n"
						"Payment Amount:		%s \r\n"
						"Check Number:		%s \r\n"
						"Account Number:		%s \r\n"
						"Routing Number:		%s \r\n\r\n"
						"****************************************************************************************************************\r\n\r\n",
						VarLong(pRow->GetValue(plPatientID), -1),
						FormatCurrencyForInterface(VarCurrency(pRow->GetValue(plCheckAmount), COleCurrency(0, 0))),
						VarString(pRow->GetValue(plCheckNumber), ""),
						VarString(pRow->GetValue(plAccountNumber), ""),
						VarString(pRow->GetValue(plRoutingNumber), "")
						);
					ErrorsFile.Write(str, str.GetLength());
				}
			}
			else {

				// (b.spivey, August 11, 2014) - PLID 62924 - Create an entry for payment did not post at all. 
				if (!bSkip && !payInfo.bManual && !(payInfo.nPaymentID > 0)) {
					CString str;
					str.Format(
						"The following payment did not post to a patient account. You will need to post this payment manually. \r\n\r\n"
						"Patient ID:		%s \r\n"
						"Payment Amount:		%s \r\n"
						"Check Number:		%s \r\n"
						"Account Number:		%s \r\n"
						"Routing Number:		%s \r\n\r\n"
						"****************************************************************************************************************\r\n\r\n",
						VarString(pRow->GetValue(plLockboxPatientID), ""),
						FormatCurrencyForInterface(VarCurrency(pRow->GetValue(plCheckAmount), COleCurrency(0, 0))),
						VarString(pRow->GetValue(plCheckNumber), ""),
						VarString(pRow->GetValue(plAccountNumber), ""),
						VarString(pRow->GetValue(plRoutingNumber), "")
						);
					ErrorsFile.Write(str, str.GetLength());
				}

				//if no patient exists just create the lockboxpayment record that they can edit in the future			
				sqlPaymentBatch.Declare("COMMIT TRAN");
				sqlPaymentBatch.Execute(GetRemoteData());
			}
			pRow = m_dlPaymentList->FindAbsoluteNextRow(pRow, g_cvarTrue);
			m_pcProgressBar.StepIt();
		}
		m_pcProgressBar.SetPos(0);
		m_eProgressText.SetWindowText("Posting Complete.");

		//rename the file so the user can tell its been posted.
		if (!m_strAnsi823FilePath.IsEmpty()) {
			CString strNewFileName;
			COleDateTime dtServer = GetRemoteServerTime();
			strNewFileName.Format("POSTED_%s_%s.txt", dtServer.Format("%m%d%Y"), dtServer.Format("%H%M%S"));

			if (m_strAnsi823FilePath.Find("POSTED_") != -1) {
				strNewFileName = (m_strAnsi823FilePath.Left(m_strAnsi823FilePath.Find("POSTED"))) + strNewFileName;
			}
			else {
				strNewFileName = (m_strAnsi823FilePath.Left(m_strAnsi823FilePath.GetLength() - 4)) + strNewFileName;
			}
			CFile::Rename(m_strAnsi823FilePath, strNewFileName);
			//clear out the file name so we dont try and rename again for no reason
			m_strAnsi823FilePath = "";
		}

		// (b.spivey, August 11, 2014) - PLID 62924 - Close and show the client what we've got. 
		long nFileLength = (long)ErrorsFile.GetLength();
		
		ErrorsFile.Close(); 

		if (nFileLength > 0) {
			int nResult = (int)ShellExecute((HWND)this, NULL, "notepad.exe", ("'" + pathname + "'"), NULL, SW_SHOW);
		}

		// (b.spivey, August 11, 2014) - PLID 63597 - We should prompt them and ask if they want to see a report of this lockbox depsoit.
		//  if no, prep the dialog for another import. 
		if (IDYES == AfxMessageBox("Would you like to preview a report for payments applied from this lockbox deposit?", MB_YESNO | MB_ICONWARNING))
		{
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(757)]);
			infReport.bExternal = TRUE;
			infReport.strExternalFilter = FormatString("LBBatch.ID = %li", m_nBatchID);

			infReport.nDateRange = -1; //All dates
			infReport.nDateFilter = 1;

			CRParameterInfo *paramInfo;
			CPtrArray paParams;

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = FormatDateTimeForInterface(dtBatchDate, DTF_STRIP_SECONDS);
			paramInfo->m_Name = "DateFrom";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = FormatDateTimeForInterface(dtBatchDate, DTF_STRIP_SECONDS);
			paramInfo->m_Name = "DateTo";
			paParams.Add((void *)paramInfo);

			RunReport(&infReport, &paParams, TRUE, this, "Lockbox Payment Report");
			ClearRPIParameterList(&paParams);

			// (b.spivey, September 22, 2014) - PLID 62924 - 
			WaitingOnImportDialog::Close(); 
			DestroyWindow();
		}
		else {
			m_eBankInstitution.SetWindowText("");
			m_ePostedAmount.SetWindowText("");
			m_eDepositAmount.SetWindowText("");
			m_dtpPaymentDate.SetTime(COleDateTime::GetCurrentTime());

			m_dlPaymentList->Clear();
			m_dlProviderList->CurSel = NULL;
			m_dlLocationList->CurSel = NULL;
			m_dlDescriptionList->CurSel = NULL;
			m_dlCategoryList->CurSel = NULL;

			m_ePaymentDescText.SetWindowText("");
			m_eProgressText.SetWindowText("Ready to import.");

			//reset our values in case they want to import another file
			m_nBatchID = -1;
			m_nPaymentCatID = -1;
			m_strAnsi823FilePath = "";
			m_cyPostedAmount = COleCurrency(0, 0);

			EnableButtons(TRUE); 
		}

	}NxCatchAll(__FUNCTION__);
}

void CLockBoxPaymentImportDlg::OnBnClickedPaymentCategoryEdit()
{
	try {
		//save the cur sel row
		IRowSettingsPtr pRow = m_dlCategoryList->GetCurSel();
		if (pRow) {
			m_nPaymentCatID = VarLong(pRow->GetValue(pcID));
		}
		else {
			m_nPaymentCatID = -1;
		}
		//open the edit dlg
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			//we need to requery the list
			m_dlCategoryList->Requery();
			m_dlCategoryList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}
		//recreate the <nothing selected> row
		pRow = m_dlCategoryList->GetNewRow();
		pRow->PutValue(pcID, -1);
		pRow->PutValue(pcName, _bstr_t("< No Category Selected >"));
		m_dlCategoryList->AddRowSorted(pRow, NULL);
		//set selection to the original
		m_dlCategoryList->SetSelByColumn(pcID, m_nPaymentCatID);
	}NxCatchAll(__FUNCTION__);
}

void CLockBoxPaymentImportDlg::OnBnClickedPaymentDescriptionEdit()
{
	try {
		CEditComboBox(this, 2, m_dlDescriptionList, "Edit Combo Box").DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CLockBoxPaymentImportDlg::OnBnClickedPaymentDate()
{
	try {
		m_dtpPaymentDate.EnableWindow(m_nxbPaymentDate.GetCheck());
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CLockBoxPaymentImportDlg, CNxDialog)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_DESCRIPTION, 16, CLockBoxPaymentImportDlg::SelChosenPaymentDescription, VTS_DISPATCH)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_CATEGORY, 16, CLockBoxPaymentImportDlg::SelChosenPaymentCategory, VTS_DISPATCH)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_DESCRIPTION, 1, CLockBoxPaymentImportDlg::SelChangingPaymentDescription, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_CATEGORY, 1, CLockBoxPaymentImportDlg::SelChangingPaymentCategory, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_LOCKBOX_PROVIDER_LIST, 1, CLockBoxPaymentImportDlg::SelChangingLockboxProviderList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_LOCKBOX_LOCATION_LIST, 1, CLockBoxPaymentImportDlg::SelChangingLockboxLocationList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_LIST, 6, CLockBoxPaymentImportDlg::RButtonDownPaymentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_LIST, 19, CLockBoxPaymentImportDlg::LeftClickPaymentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_LIST, 10, CLockBoxPaymentImportDlg::EditingFinishedPaymentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLockBoxPaymentImportDlg, IDC_PAYMENT_LIST, 8, CLockBoxPaymentImportDlg::EditingStartingPaymentList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

void CLockBoxPaymentImportDlg::SelChosenPaymentDescription(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			CString strDescription = VarString(pRow->GetValue(pdName), "");
			m_dlDescriptionList->PutComboBoxText("");
			m_ePaymentDescText.SetWindowText(strDescription);
		}
	}NxCatchAll(__FUNCTION__);
}


void CLockBoxPaymentImportDlg::SelChosenPaymentCategory(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_nPaymentCatID = VarLong(pRow->GetValue(pcID), -1);
		}
	}NxCatchAll(__FUNCTION__);
}


void CLockBoxPaymentImportDlg::SelChangingPaymentDescription(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}


void CLockBoxPaymentImportDlg::SelChangingPaymentCategory(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}


void CLockBoxPaymentImportDlg::SelChangingLockboxProviderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}


void CLockBoxPaymentImportDlg::SelChangingLockboxLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CLockBoxPaymentImportDlg::RButtonDownPaymentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			//make this the selected row
			m_dlPaymentList->PutCurSel(pRow);

			CMenu menu;
			menu.CreatePopupMenu();
			//if its marked as manual do not allow spliting or changing patient
			//value of the check box
			BOOL bManual = VarBool(pRow->GetValue(plManual), FALSE);
			//value of the database
			BOOL bPostedManual = VarBool(pRow->GetValue(plPostedManual), FALSE);
			BOOL bPosted = (VarLong(pRow->GetValue(plPaymentID), -1) != -1 ? TRUE : FALSE);
			if (nCol == plPatientName) {
				menu.InsertMenu(0, MF_BYPOSITION | (bManual || bPosted ? MF_DISABLED : MF_ENABLED), -1, "&Select a different patient");
			}
			menu.InsertMenu(1, MF_BYPOSITION | (bManual || bPosted ? MF_DISABLED : MF_ENABLED), -2, "Split &Payment");
			//add the manual posting option, only allow on new batches			
			menu.InsertMenu(2, MF_BYPOSITION | (bPostedManual || bPosted ? MF_DISABLED : MF_ENABLED) | (bManual ? MF_CHECKED : MF_UNCHECKED), -3, "Post &Manually");
			
			CPoint pt;
			GetCursorPos(&pt);

			int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_TOPALIGN, pt.x, pt.y, this, NULL);
			switch (nResult) {
			case -1:
				// (d.singleton 2014-08-12 08:59) - PLID 62943 - if no patient is found for that payment,  leave the patient column blank.  add context menu to manually add one
				ChoosePatient(pRow);
				break;
			case -2:
				// (d.singleton 2014-08-12 08:57) - PLID 63333 - need ability to split a lockbox payment into multiple payments
				SplitPayment(pRow);
				break;
			case -3:
				// (d.singleton 2014-08-11 14:23) - PLID 63257 - Need the ability to post manually on a line item for lockbox payment
				PostManually(pRow);
				break;
			}			
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-12 08:59) - PLID 62943 - if no patient is found for that payment,  leave the patient column blank.  add context menu to manually add one
void CLockBoxPaymentImportDlg::LeftClickPaymentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (nCol == plPatientName) {
				CString strName = VarString(pRow->GetValue(plPatientName), "");
				BOOL bManual = VarBool(pRow->GetValue(plManual), g_cvarFalse);
				// cannot change patient once its marked manual
				if (strName == "< Select a Patient >" && !bManual) {
					ChoosePatient(pRow);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-05 10:06) - PLID 62955 - add a checkbox to the bottom of the Lockbox payment dlg called "Mark payments as deposited"
void CLockBoxPaymentImportDlg::OnBnClickedMarkAsDeposited()
{
	try {
		long nDeposited = m_nxbMarkAsDeposited.GetCheck();
		SetRemotePropertyInt("LockboxPaymentMarkAsDeposited", nDeposited, 0, "<None>");
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-05 10:06) - PLID 62951 - Add the following checkboxes to the lockbox payment dialog use these as the payment description
void CLockBoxPaymentImportDlg::OnBnClickedCheckNumber()
{
	try {
		long nCheckNumber = m_nxbCheckNumber.GetCheck();
		SetRemotePropertyInt("LockboxPaymentAddCheckNumber", nCheckNumber, 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-05 10:06) - PLID 62951 - Add the following checkboxes to the lockbox payment dialog use these as the payment description
void CLockBoxPaymentImportDlg::OnBnClickedAccountNumber()
{
	try {
		long nAccountNumber = m_nxbAccountNumber.GetCheck();
		SetRemotePropertyInt("LockboxPaymentAddAccountNumber", nAccountNumber, 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-05 10:06) - PLID 62951 - Add the following checkboxes to the lockbox payment dialog use these as the payment description
void CLockBoxPaymentImportDlg::OnBnClickedRoutingNumber()
{
	try {
		long nRoutingNumber = m_nxbRoutingNumber.GetCheck();
		SetRemotePropertyInt("LockboxPaymentAddRoutingNumber", nRoutingNumber, 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-12 09:01) - PLID 63258 - if a user manually changes the amount of a lockbox payment line item,  make the font red
void CLockBoxPaymentImportDlg::EditingFinishedPaymentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow && bCommit) {
			if (nCol == plCheckAmount) {
				//if they actually changed an amount we need to make font red,  also update the deposit amount
				COleCurrency cyOld, cyNew, cyChange;
				cyOld = VarCurrency(varOldValue, COleCurrency(0, 0));
				cyNew = VarCurrency(varNewValue, COleCurrency(0, 0));
				if (cyOld != cyNew) {
					if (cyNew == VarCurrency(pRow->GetValue(plOriginalAmt), COleCurrency(0, 0))) {
						pRow->PutForeColor(Black);
					}
					else {
						pRow->PutForeColor(Red);
					}

					// (d.singleton 2014-08-08 15:48) - PLID 63259 - keep track of the deposit amount.  needs to change when a user edits a payment line item.  make font red if the amount goes negative
					//this could create a negative COleCurrency so keep that in mind when debugging				
					cyChange = cyOld - cyNew;
					m_cyPostedAmount -= cyChange;
					m_ePostedAmount.SetWindowText(FormatCurrencyAsString(m_cyPostedAmount, true));
				}
			}
			if (nCol == plManual) {
				if (VarBool(pRow->GetValue(plManual), g_cvarFalse)) {
					AfxMessageBox("You have selected to manually post this line item.  It will not be posted to any patient account. You must manually post this payment.");

					//make our row read only
					pRow->PutBackColor(Grey);
					//if its marked as skipped un check that
					pRow->PutValue(plSkip, g_cvarFalse);
					pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleFalse);
				}
				else {
					pRow->PutBackColor(White);
					if (VarString(pRow->GetValue(plPatientName), "") == "< Select a Patient >") {
						pRow->PutCellLinkStyle(plPatientName, NXDATALIST2Lib::dlLinkStyleTrue);
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2014-08-11 14:53) - PLID 63257 - cannot edit row if its marked as manual, or if it has been posted (has a payment id)
// (d.singleton 2014-08-12 08:52) - PLID 62962 - When selecting Edit Payment, open the Lockbox Payment Posting dialog, but disable any payments that have already been
void CLockBoxPaymentImportDlg::EditingStartingPaymentList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			//current checkbox value
			BOOL bManual = VarBool(pRow->GetValue(plManual), FALSE);
			//saved database value
			BOOL bPostedManual = VarBool(pRow->GetValue(plPostedManual), FALSE);
			long nPaymentID = VarLong(pRow->GetValue(plPaymentID), -1);
			if (nCol == plCheckAmount) {								
				if (bManual || nPaymentID != -1) {
					*pbContinue = FALSE;
				}
			}
			//we need to check if it was previously saved as manual,  then it shouldnt be able to change,  also check payment id
			if (nCol == plManual) {
				if (bPostedManual || nPaymentID != -1) {
					*pbContinue = FALSE;
				}
			}
			if (nCol == plSkip) {
				if (bManual || nPaymentID != -1) {
					*pbContinue = FALSE;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}
