// BillingModuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "BillingModuleDlg.h"
#include "BillingDlg.h"
#include "GlobalUtils.h"
#include "GlobalFinancialUtils.h"
#include "GlobalReportUtils.h"
#include "pracprops.h"
#include "QuoteNotes.h"
#include "NxStandard.h"
#include "reports.h"
#include "MainFrm.h"
#include "AuditTrail.h"
#include "ReportInfo.h"
#include "Barcode.h"
#include "BillingRc.h"
#include "NxMessageDef.h"
#include "SuperBillApplyPromptDlg.h"
#include "AppliedSuperbillsDlg.h"
#include "InsuranceReferralsDlg.h"
#include "InternationalUtils.h"
#include "InsuranceReferralsSelectDlg.h"
#include "DateTimeUtils.h"
#include "LetterWriting.h"
#include "MergeEngine.h"
#include "GlobalSchedUtils.h"
#include "FinancialDlg.h"
#include "DontShowDlg.h"
#include "Rewards.h"
#include "MultiSelectDlg.h"
#include <foreach.h>	// (j.dinatale 2012-03-20 11:45) - PLID 48893 - foreach their own!
#include "PatientView.h"
#include "InsuranceBilling.h"
#include "Billing2Dlg.h"
#include "DiagCodeInfo.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2014-02-25 13:34) - PLID 61024 - Removed ancient Access error 3048 handling and nonexistent datalist Exception event

/////////////////////////////////////////////////////////////////////////////
// CBillingModuleDlg dialog

// (a.walling 2009-02-23 10:40) - PLID 11034 - Standard default billing module sizes
// (j.armen 2014-08-11 17:52) - PLID 63330 - The bill has a larger minimum size
const long g_cnBillingModuleMinBillSizeX = 950;
const long g_cnBillingModuleMinQuoteSizeX = 776;
const long g_cnBillingModuleMinSizeY = 549;

// (j.jones 2013-06-04 09:48) - PLID 31874 - added an enum for the quote report dropdown columns
enum QuoteCustomListColumns
{
	qclcID = 0,
	qclcNumber,
	qclcFileName,
	qclcTitle,
	qclcRepNum,
};

CBillingModuleDlg::CBillingModuleDlg(CWnd* pParent,
									 UINT nIDTemplate /* = IDD_BILLING_MODULE_DLG */)
	: CNxDialog(CBillingModuleDlg::IDD, pParent),
	m_dlgBilling(*(new CBillingDlg(NULL, IDD_BILLING_DLG))), 

	m_dlgInsuranceBilling(*(new CInsuranceBilling(this))),
	m_dlgBilling2(*(new CBilling2Dlg(this))),
	m_coordChecker(NetUtils::Coordinators)
{
	//{{AFX_DATA_INIT(CBillingModuleDlg)
		m_pActiveDlg = NULL;
		m_iBillID = -1;
		// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
		m_eHasAccess = batNoAccess;
		m_bHasEverHadAccess = FALSE;
		m_boAskForNewPayment = TRUE;
		m_boShowAvailablePayments = FALSE;
		m_boIsNewBill = FALSE;
		// (j.jones 2011-08-24 08:41) - PLID 44868 - added m_bHasBeenCorrected,
		// which is only TRUE if the bill ID exists in BillCorrectionsT.OriginalBillID
		m_bHasBeenCorrected = FALSE;
		m_EntryType = BillEntryType::Bill;
// (j.gruber 2009-07-10 17:19) - PLID 34724 - take this out m_bBatched = FALSE;
		m_bManuallyUnbatched = FALSE;
		m_bInsurance = FALSE;
		m_bPromptForReferral = TRUE;
		m_nInsuranceReferralID = -1;
		m_strSelectedAuthNum = "";
		m_bIsPopupAllowed = TRUE;
		m_nPatientID = -1;
		m_bUseDefaultDate = FALSE;
		m_dtDefaultDate = COleDateTime::GetCurrentTime();
		m_nQuoteEMR = -1;
		m_eCreatedFrom = BillFromType::Other;  // (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum, initialize to other
		m_pFinancialDlg = NULL;
		// (d.singleton 2014-02-28 11:49) - PLID 61072 - when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
		m_nBillToInsPartyID = -1;
	//}}AFX_DATA_INIT
	m_nIDTemplate = nIDTemplate;
	m_pPostCloseMsgWnd = NULL;
	m_nCachedPatientNameID = -1;
	m_dwSize = 0;
	// (a.walling 2009-12-23 09:26) - PLID 7002
	m_bVisibleState = false;
	// (r.gonet 07/02/2014) - PLID 62567 - Initialize the tracked status type.
	m_eBillStatusType = EBillStatusType::None;
	// (r.farnworth 2014-12-10 14:36) - PLID 64081 - If you cancel a bill from a HL7 charge message (DFT) that was commited, Nextech considers the message as imported.
	m_nMessageQueueID = -1;
}

CBillingModuleDlg::~CBillingModuleDlg()
{	
	//DRT 5/5/2004 - PLID 12207 - We no longer want to receive barcode messages!
	// (a.walling 2007-12-20 17:39) - PLID 28252 - We registered as a bill and as a quote, also we only
	// need to register once when creating and once when destructing. This dialog is most often hidden.
	if(/*m_EntryType == 2 &&*/ GetMainFrame())
		GetMainFrame()->UnregisterForBarcodeScan(this);

	m_dlgBilling2.DestroyWindow();
	m_dlgInsuranceBilling.DestroyWindow();
	m_dlgBilling.DestroyWindow();
	delete (&m_dlgBilling);
	delete (&m_dlgInsuranceBilling);
	delete (&m_dlgBilling2);
}


void CBillingModuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBillingModuleDlg)
	if (m_nIDTemplate == IDD_BILLING_MODULE_DLG)
	{
		DDX_Control(pDX, IDC_BTN_SAVE_COPY, m_btnSaveCopy);
		DDX_Control(pDX, IDC_BILL_MERGE_BTN, m_mergeButton);
	}
	DDX_Control(pDX, IDC_BTN_EDIT_TEXT, m_editTextButton);
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDC_BILL_PREVIEW_BTN, m_printPreviewButton);
	DDX_Control(pDX, IDC_BILL_EDIT_BTN, m_editButton);
	DDX_Control(pDX, IDC_BILL_DELETE_BTN, m_deleteButton);
	DDX_Control(pDX, IDC_SAVE, m_saveButton);
	DDX_Control(pDX, IDC_EDIT_DESCRIPTION, m_editDescription);
	DDX_Control(pDX, IDC_EDIT_QUOTE_NOTES, m_editQuoteNotes);
	DDX_Control(pDX, IDC_BILL_DATE, m_date);
	DDX_Control(pDX, IDC_EDIT_ID, m_nxeditEditId);
	DDX_Control(pDX, IDC_EDIT_PATIENT_ID, m_nxeditEditPatientId);
	DDX_Control(pDX, IDC_EDIT_PATIENT_NAME, m_nxeditEditPatientName);
	DDX_Control(pDX, IDC_LABEL_BILL_DATE, m_nxstaticLabelBillDate);
	DDX_Control(pDX, IDC_LABEL_BILL_ID, m_nxstaticLabelBillId);
	DDX_Control(pDX, IDC_LABEL_QUOTE_NOTE_1, m_nxstaticLabelQuoteNote1);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CBillingModuleDlg, IDC_BILL_DATE, 2 /* Change */, OnChangeBillDate, VTS_NONE)

// (a.walling 2008-05-13 14:58) - PLID 27591 - Use notify handlers for datetimepicker events
BEGIN_MESSAGE_MAP(CBillingModuleDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBillingModuleDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_BILL_DATE, OnChangeBillDate)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BILL_DELETE_BTN, OnBillDeleteBtn)
	ON_BN_CLICKED(IDC_BILL_EDIT_BTN, OnBillEditBtn)
	ON_BN_CLICKED(IDC_BILL_PREVIEW_BTN, OnBillPreviewBtn)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BTN_EDIT_TEXT, OnBtnEditText)
	ON_WM_SIZE()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_BTN_SAVE_COPY, OnBtnSaveCopy)
	ON_BN_CLICKED(IDC_QUOTE_CUSTOM_REPORT_MAKE_DEFAULT, OnQuoteMakeDefault)
	ON_BN_CLICKED(IDC_BILL_MERGE_BTN, OnMergeToWord)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(IDC_EDIT_DESCRIPTION, &CBillingModuleDlg::OnEnKillfocusEditDescription)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBillingModuleDlg message handlers

BEGIN_EVENTSINK_MAP(CBillingModuleDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBillingModuleDlg)
	ON_EVENT(CBillingModuleDlg, IDC_TAB, 1 /* SelectTab */, OnSelectTab, VTS_I2 VTS_I2)
	ON_EVENT(CBillingModuleDlg, IDC_QUOTE_CUSTOM_REPORT_LIST, 16 /* SelChosen */, OnSelChosenCustomReportCombo, VTS_I4)
	ON_EVENT(CBillingModuleDlg, IDC_COORDINATOR_COMBO, 16 /* SelChosen */, OnSelChosenCoordinatorCombo, VTS_I4)
	ON_EVENT(CBillingModuleDlg, IDC_COORDINATOR_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCoordinatorCombo, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CBillingModuleDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CString str;

	CNxDialog::OnShowWindow(bShow, nStatus);

	// (a.walling 2009-12-22 17:05) - PLID 7002 - We are now modeless, so we don't need this anymore
	/*
	if (bShow == FALSE) {
		if(!m_bIsPopupAllowed) {
			GetMainFrame()->AllowPopup();
			m_bIsPopupAllowed = TRUE;
		}
		return;
	}

	if(m_bIsPopupAllowed) {
		GetMainFrame()->DisallowPopup();
		m_bIsPopupAllowed = FALSE;
	}
	*/

	// (a.walling 2009-12-23 09:26) - PLID 7002 - Since we may be modeless, we need to ensure the OnShowWindow logic is only executed when necessary
	bool bProcess = false;
	if (bShow && !m_bVisibleState) {
		m_bVisibleState = true;
		bProcess = true;
	}

	if (!bProcess) return;

	if (m_iBillID == -1) {
		// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
		m_eHasAccess = batFullAccess;
		m_bHasEverHadAccess = TRUE;
	}

	if (m_coordChecker.Changed()) {
		m_CoordCombo->Requery();
		NXDATALISTLib::IRowSettingsPtr pRow;
		_variant_t var;
		pRow = m_CoordCombo->GetRow(-1);
		var = (long)-1;
		pRow->PutValue(0,var);
		pRow->PutValue(1,"<No Patient Coordinator>");
		m_CoordCombo->InsertRow(pRow,0);
	}

	m_bInsurance = FALSE;
// (j.gruber 2009-07-10 17:19) - PLID 34724 - take this out and put in manually batched	
	//m_bBatched = FALSE;
	m_bManuallyUnbatched = FALSE;
	m_nInsuranceReferralID = -1;
	m_strSelectedAuthNum = "";
	m_bPromptForReferral = TRUE;

	// (j.jones 2011-04-27 15:32) - PLID 43405 - this is now a variant
	m_varSuperbillID = g_cvarNull;

	// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
	// if they only have partial access then the date & pat. coord. is disabled,
	// as is the ability to delete
	GetDlgItem(IDC_BILL_EDIT_BTN)->EnableWindow(m_eHasAccess == batNoAccess);
	GetDlgItem(IDC_BTN_SAVE_COPY)->EnableWindow(m_eHasAccess == batNoAccess);
	// (j.gruber 2011-06-17 13:14) - PLID 43943 - don't enable if its a new bill
	GetDlgItem(IDC_BILL_DELETE_BTN)->EnableWindow(m_eHasAccess == batFullAccess && m_iBillID != -1);
	GetDlgItem(IDC_BILL_DATE)->EnableWindow(m_eHasAccess == batFullAccess);
	GetDlgItem(IDC_EDIT_DESCRIPTION)->EnableWindow(m_eHasAccess != batNoAccess);
	GetDlgItem(IDC_COORDINATOR_COMBO)->EnableWindow(m_eHasAccess == batFullAccess);

	///////////////////////////////
	// Change labels for a quote 
	if (m_EntryType == 2) {
		CNxColor *tmpWnd = (CNxColor*)GetDlgItem(IDC_NXCOLORCTRL2);
		tmpWnd->SetColor(RGB(255,179,128));

		GetDlgItem(IDC_LABEL_BILL_ID)->SetWindowText("Quote ID");
		GetDlgItem(IDC_LABEL_BILL_DATE)->SetWindowText("Quote Date");

		SetWindowText("Quote Information");

		m_tab->Label[0] = "&Quote";

		/* Change button positions to make room for the
		quote description edit box */
		// (a.walling 2009-02-23 10:41) - PLID 11034 - Call SetSingleControlPos to set the position in original dialog coords, but sized appropriately
		SetSingleControlPos(IDC_BILL_DELETE_BTN, NULL, 410, 10, 47, 49, SWP_NOZORDER);
		GetDlgItem(IDC_BILL_DELETE_BTN)->SetWindowText(" &Delete    Quote");
		SetSingleControlPos(IDC_BILL_MERGE_BTN, NULL, 460, 10, 47, 49, SWP_NOZORDER);
		SetSingleControlPos(IDC_BILL_EDIT_BTN, NULL, 510, 10, 47, 49, SWP_NOZORDER);
		// (j.jones 2008-11-12 09:16) - PLID 28820 - rename the Edit button to be context-sensitive
		GetDlgItem(IDC_BILL_EDIT_BTN)->SetWindowText("&Edit\r\nQuote");
		SetSingleControlPos(IDC_BTN_EDIT_TEXT, NULL, 560, 10, 47, 49, SWP_NOZORDER | SWP_SHOWWINDOW);
		SetSingleControlPos(IDC_BILL_PREVIEW_BTN, NULL, 610, 10, 47, 49, SWP_NOZORDER);
		SetSingleControlPos(IDCANCEL, NULL, 659, 10, 47, 49, SWP_NOZORDER);
		SetSingleControlPos(IDC_SAVE, NULL, 708, 10, 47, 49, SWP_NOZORDER);
		// (a.walling 2010-01-14 16:29) - PLID 36889 - All SW_SHOW calls here have been changed to SW_SHOWNOACTIVATE to stop messing with the focus
		GetDlgItem(IDC_BTN_SAVE_COPY)->ShowWindow(SW_SHOWNOACTIVATE);
		SetSingleControlPos(IDC_BTN_SAVE_COPY, NULL, 708, 63, 47, 44, SWP_NOZORDER);
		GetDlgItem(IDC_EDIT_QUOTE_NOTES)->ShowWindow(SW_SHOWNOACTIVATE);		
		GetDlgItem(IDC_BILL_PREVIEW_BTN)->SetWindowText("&Print Prev.");
		SetDlgItemText(IDC_LABEL_QUOTE_NOTE_1,"Note (non-printing)");
		GetDlgItem(IDC_QUOTE_CUSTOM_REPORT_LIST)->ShowWindow(SW_SHOWNOACTIVATE);
		GetDlgItem(IDC_QUOTE_CUSTOM_REPORT_MAKE_DEFAULT)->ShowWindow(SW_SHOWNOACTIVATE);

		// (j.gruber 2007-02-21 14:38) - 24847 - grey out the button if its a new quote since it doesn't work
		GetDlgItem(IDC_BTN_SAVE_COPY)->EnableWindow(m_iBillID != -1);
	
	}
	///////////////////////////////
	// Change labels for the bill
	else {
		//DRT 2/12/03 - Size the buttons correctly for what needs to show on the Bill.		
		if(m_EntryType == 1 && GetRemotePropertyInt("ApplySuperbillIDs",0,0,"<None>",TRUE)==1) {	//only load if they have this enabled in preferences
			// (a.walling 2009-02-23 10:41) - PLID 11034 - Call SetSingleControlPos to set the position in original dialog coords, but sized appropriately
			SetSingleControlPos(IDC_BTN_EDIT_TEXT, NULL, 400, 28, 52, 60, SWP_NOZORDER | SWP_SHOWWINDOW);
			GetDlgItem(IDC_BTN_EDIT_TEXT)->SetWindowText("Apply Superbills");

			SetSingleControlPos(IDC_BILL_DELETE_BTN, NULL, 452, 28, 51, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_BILL_MERGE_BTN, NULL, 503, 28, 51, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_BILL_EDIT_BTN, NULL, 554, 28, 51, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_BILL_PREVIEW_BTN, NULL, 605, 28, 51, 60, SWP_NOZORDER);
			SetSingleControlPos(IDCANCEL, NULL, 656, 28, 51, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_SAVE, NULL, 707, 28, 51, 60, SWP_NOZORDER);
		}
		else {
			GetDlgItem(IDC_BTN_EDIT_TEXT)->ShowWindow(SW_HIDE);

			// (a.walling 2009-02-23 10:41) - PLID 11034 - Call SetSingleControlPos to set the position in original dialog coords, but sized appropriately
			SetSingleControlPos(IDC_BILL_DELETE_BTN, NULL, 413, 28, 53, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_BILL_MERGE_BTN, NULL, 471, 28, 53, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_BILL_EDIT_BTN, NULL, 529, 28, 53, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_BILL_PREVIEW_BTN, NULL, 587, 28, 53, 60, SWP_NOZORDER);
			SetSingleControlPos(IDCANCEL, NULL, 645, 28, 53, 60, SWP_NOZORDER);
			SetSingleControlPos(IDC_SAVE, NULL, 703, 28, 53, 60, SWP_NOZORDER);
		}

		//Hide the button and datalist for custom quotes
		GetDlgItem(IDC_QUOTE_CUSTOM_REPORT_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUOTE_CUSTOM_REPORT_MAKE_DEFAULT)->ShowWindow(SW_HIDE);
	}

	// End change labels for a quote 
	///////////////////////////////

	try {

		COleVariant var;
		
		if(m_nPatientID == -1)
			m_nPatientID = (long)GetActivePatientID();

		if(m_EntryType == 1 && GetRemotePropertyInt("ApplySuperbillIDs",0,0,"<None>",TRUE)==1 && m_boIsNewBill) {
			//non-quote + new bill + they want to apply
			CSuperBillApplyPromptDlg dlg(this);

			dlg.m_nPatientID = m_nPatientID;
			dlg.DoModal();

			// (j.jones 2011-04-27 15:32) - PLID 43405 - this is now a variant
			if(dlg.m_nFinalID == -1)
				m_varSuperbillID = g_cvarNull;
			else {
				m_varSuperbillID = (long)dlg.m_nFinalID;
			}
		}

		// (j.gruber 2007-11-20 12:53) - PLID 28061 - added IsPackage into sql to know whether it is a package		

		// (a.walling 2007-05-21 14:55) - PLID 26088 - Don't query if we already know it won't return anything.
		_RecordsetPtr rs = NULL;
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(m_iBillID != -1) {
			rs = CreateParamRecordset("SELECT BillsT.*, "
				"CASE WHEN PackagesT.QuoteID IS NULL THEN 0 ELSE 1 END AS IsPackage, "
				"Convert(bit, CASE WHEN BillCorrectionsT_OriginalBillQ.OriginalBillID Is Null THEN 0 ELSE 1 END) AS IsOriginalBill "
				"FROM BillsT "
				"LEFT JOIN PackagesT ON BillsT.ID = PackagesT.QuoteID "
				"LEFT JOIN (SELECT OriginalBillID FROM BillCorrectionsT) AS BillCorrectionsT_OriginalBillQ ON BillsT.ID = BillCorrectionsT_OriginalBillQ.OriginalBillID "
				"WHERE BillsT.PatientID = {INT} AND BillsT.Deleted = 0 "
				"AND ID = {INT}", m_nPatientID, m_iBillID);
		}

		if (rs != NULL && !rs->eof) {
			///////////////////////////////////////////
			// Existing bill/quote
			///////////////////////////////////////////
			// ID
			str.Format("%d", m_iBillID);
			GetDlgItem(IDC_EDIT_ID)->SetWindowText(str);

			// (j.jones 2011-08-24 08:41) - PLID 44868 - added m_bHasBeenCorrected,
			// which is only TRUE if the bill ID exists in BillCorrectionsT.OriginalBillID
			m_bHasBeenCorrected = VarBool(rs->Fields->Item["IsOriginalBill"]->Value, FALSE);

			// Date
			var = rs->Fields->Item["Date"]->Value;
			m_date.SetValue(var);
			// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
			m_dtCurrentDate = (m_date.GetValue());
			// Notes
			var = rs->Fields->Item["Description"]->Value;
			if (var.vt == VT_NULL)
				var.SetString(" ", VT_BSTRT);
			CString strDesc = CString(var.bstrVal);
			strDesc = strDesc.Left(255);
			m_editDescription.SetWindowText(strDesc);

			// Addition non-printing quote notes
			if (m_EntryType == 2) {
				var = rs->Fields->Item["Note"]->Value;
				if (var.vt != VT_NULL) {
					GetDlgItem(IDC_EDIT_QUOTE_NOTES)->SetWindowText(CString(var.bstrVal));
				}

				// ExtraDesc (printed quote notes)
				var = rs->Fields->Item["ExtraDesc"]->Value;
				if (var.vt != VT_NULL) {
					m_strExtraDesc = CString(var.bstrVal);
				}
			}

			// Patient Coordinator - load from existing Bill
			var = rs->Fields->Item["PatCoord"]->Value;
			// (j.jones 2011-01-25 15:29) - PLID 42156 - track this coordinator ID
			m_nCurCoordinatorID = VarLong(var, -1);
			if(m_CoordCombo->TrySetSelByColumn(0,var) == -1) {
				//might have an inactive coord
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rsCoord = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nCurCoordinatorID);
				if(!rsCoord->eof) {
					m_CoordCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCoord, "Name", "")));
				}
				else 
					m_CoordCombo->PutCurSel(-1);
			}

			m_nInsuranceReferralID = AdoFldLong(rs, "InsuranceReferralID",-1);

			//get the auth number
			CString strAuth = AdoFldString(rs, "PriorAuthNum", "");
			m_strSelectedAuthNum = strAuth;

			// (j.gruber 2007-11-20 12:54) - PLID 28061 - set the correct report type
			if (m_EntryType == 2) {
				if (AdoFldLong(rs, "IsPackage", 0) != 0) {
					SetQuoteReportDropDown(TRUE);
				}	
				else {
					SetQuoteReportDropDown(FALSE);
				}
			}


			rs->Close();
		}
		else if (m_iBillID == -1) {
			///////////////////////////////////////////
			// New bill/quote
			///////////////////////////////////////////

			GetDlgItem(IDC_EDIT_ID)->SetWindowText("");

			// Date
			COleDateTime dt;

			// (j.gruber 2007-11-20 12:56) - PLID 28061 - set the report type to regular
			if (m_EntryType == 2) {
				SetQuoteReportDropDown(FALSE);
			}

			//if we're forcing a default date, use it, else do the normal processing
			if(m_bUseDefaultDate && m_dtDefaultDate.GetStatus() != COleDateTime::invalid)
				dt = m_dtDefaultDate;
			else {
				if(m_EntryType == 1) {
					long DefaultBillDate = GetRemotePropertyInt("DefaultBillDate",1,0,"<None>",TRUE);
					if(DefaultBillDate == 2) {
						// (j.jones 2007-05-07 09:43) - PLID 25906 - pull the last bill date from GlobalUtils
						dt = GetLastBillDate();

						//checks for any active global period for the new date, that we weren't already warned about
						// (a.walling 2008-07-07 18:03) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
						CheckWarnGlobalPeriod(m_nPatientID, dt, TRUE, COleDateTime::GetCurrentTime());
					}
					else {
						dt = COleDateTime::GetCurrentTime();
					}
				}
				else {
					long DefaultQuoteDate = GetRemotePropertyInt("DefaultQuoteDate",1,0,"<None>",TRUE);
					if(DefaultQuoteDate == 2) {
						// (j.jones 2007-05-07 09:44) - PLID 25906 - pull the last quote date from GlobalUtils
						dt = GetLastQuoteDate();
					}
					else {
						dt = COleDateTime::GetCurrentTime();
					}
				}

				//if they have an applied superbill, see if that superbill is linked with an appointment
				//and that appointment's resource is linked with a provider, then use that provider
				if(m_varSuperbillID.vt == VT_I4
					&& GetRemotePropertyInt("ApplySuperbillUseAptDate",0,0,"<None>",true)==1) {
					//what to do with multi-resources? For now, take the first resource that has a Provider ID
					long nProviderID = -1;
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT AppointmentsT.Date FROM AppointmentsT "
						"INNER JOIN PrintedSuperBillsT ON AppointmentsT.ID = PrintedSuperBillsT.ReservationID "
						"WHERE PrintedSuperBillsT.SavedID = {INT}", VarLong(m_varSuperbillID));
					if(!rs->eof) {
						dt = COleDateTime(rs->Fields->Item["Date"]->Value);
					}
					rs->Close();
				}
			}

			m_date.SetValue(COleVariant(dt));

			// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
			m_dtCurrentDate = (m_date.GetValue());

			// no longer needed, we didn't open the recordset to begin with
			//rs->Close(); 

			// Patient Coordinator
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			rs = CreateParamRecordset("SELECT EmployeeID FROM PatientsT WHERE PatientsT.PersonID={INT}", m_nPatientID);
			if (!rs->eof) {
				//If the Patient coordinator in gen1 is inactive, we do not want to put it here on a new bill
				var = rs->Fields->Item["EmployeeID"]->Value;
				// (j.jones 2011-01-25 15:29) - PLID 42156 - track this coordinator ID
				m_nCurCoordinatorID = VarLong(var, -1);
				m_CoordCombo->TrySetSelByColumn(0,var);
			}
			rs->Close();
		}
		else {
			MsgBox("This bill has been deleted (possibly by another user). ; please click on the Cancel button of the Billing dialog to abort this operation.");			
		}
		if(rs->State != adStateClosed)
			rs->Close();

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		rs = CreateParamRecordset("SELECT UserDefinedID FROM PatientsT WHERE PersonID = {INT}",m_nPatientID);
		if(!rs->eof) {
			str.Format("%li",rs->Fields->Item["UserDefinedID"]->Value.lVal);
			GetDlgItem(IDC_EDIT_PATIENT_ID)->SetWindowText(str);
		}
		rs->Close();
	}
	NxCatchAll("Error in BillingModuleDlg::OnShowWindow");

	//patient name
	str = GetExistingPatientName(m_nPatientID);
	GetDlgItem(IDC_EDIT_PATIENT_NAME)->SetWindowText(str);
	// Set patient ID				

	// (a.walling 2007-05-24 08:47) - PLID 26114
	try {
		// (a.walling 2007-05-24 09:06) - PLID 26114 - Reset this value (only used for unsaved charges).
		m_cyAdjustedPoints.SetCurrency(0, 0);

		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
			m_cyTotalPoints = Rewards::GetTotalPoints(m_nPatientID);
		}
	} NxCatchAll("Error reading Reward Points data!");
	
	if (m_EntryType == 1 && m_dlgInsuranceBilling.GetSafeHwnd() == NULL) {
		m_dlgInsuranceBilling.Create(IDD_INSURANCE_BILLING, this);
		m_dlgInsuranceBilling.m_pBillingModuleWnd = (CWnd*)this;
		m_dlgInsuranceBilling.m_peditBillDescription = &m_editDescription;
		m_dlgInsuranceBilling.m_peditBillDate = &m_date;
		m_dlgInsuranceBilling.SetDlgItemText(IDC_EDIT_AUTHORIZATION_NUMBER,m_strSelectedAuthNum);
		m_dlgInsuranceBilling.SetAccidentStatus();
		m_dlgInsuranceBilling.m_eHasAccess = m_eHasAccess;
		m_dlgInsuranceBilling.m_nPatientID = m_nPatientID;
	}
	else if (m_EntryType == 1 && m_dlgInsuranceBilling.GetSafeHwnd() != NULL) {
		m_dlgInsuranceBilling.m_peditBillDescription = &m_editDescription;
		m_dlgInsuranceBilling.m_peditBillDate = &m_date;
		m_dlgInsuranceBilling.SetDlgItemText(IDC_EDIT_AUTHORIZATION_NUMBER,m_strSelectedAuthNum);
		m_dlgInsuranceBilling.SetAccidentStatus();
		m_dlgInsuranceBilling.m_eHasAccess = m_eHasAccess;
		m_dlgInsuranceBilling.m_nPatientID = m_nPatientID;
	}

	if (m_dlgBilling2.GetSafeHwnd() == NULL) {
		m_dlgBilling2.m_EntryType = m_EntryType;
		m_dlgBilling2.Create(IDD_BILLING_2_DLG, this);
		m_dlgBilling2.m_pBillingModuleWnd = (CWnd*)this;
		m_dlgBilling2.m_peditBillDescription = &m_editDescription;
		m_dlgBilling2.m_peditBillDate = &m_date;
		m_dlgBilling2.m_eHasAccess = m_eHasAccess;
		m_dlgBilling2.ReadBillData();
	}
	else if (m_dlgBilling2.GetSafeHwnd() != NULL) {
		m_dlgBilling2.m_peditBillDescription = &m_editDescription;
		m_dlgBilling2.m_peditBillDate = &m_date;
		m_dlgBilling2.m_eHasAccess = m_eHasAccess;
		m_dlgBilling2.ReadBillData();
	}

	if (m_dlgBilling.GetSafeHwnd() == NULL) {
		m_dlgBilling.m_EntryType = m_EntryType;

		m_dlgBilling.Create(IDD_BILLING_DLG, this);

		m_dlgBilling.m_peditBillDescription = &m_editDescription;
		m_dlgBilling.m_peditBillDate = &m_date;
		m_dlgBilling.m_peditQuoteNotes = (CNxEdit*)GetDlgItem(IDC_EDIT_QUOTE_NOTES);
		m_dlgBilling.m_pBillingModuleWnd = this;
		m_dlgBilling.m_nPatientID = m_nPatientID;
		// (d.singleton 2014-02-27 17:25) - PLID 61072 - when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
		m_dlgBilling.m_nBillToInsPartyID = m_nBillToInsPartyID;
	}
	else {
		m_dlgBilling.m_peditBillDescription = &m_editDescription;
		m_dlgBilling.m_peditBillDate = &m_date;
		m_dlgBilling.m_peditQuoteNotes = (CNxEdit*)GetDlgItem(IDC_EDIT_QUOTE_NOTES);
		m_dlgBilling.m_pBillingModuleWnd = this;
		m_dlgBilling.m_nPatientID = m_nPatientID;
		m_dlgBilling.m_EntryType = m_EntryType;
		// (d.singleton 2014-02-27 17:25) - PLID 61072 - when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
		m_dlgBilling.m_nBillToInsPartyID = m_nBillToInsPartyID;
		m_dlgBilling.ShowWindow(SW_SHOW);
		m_dlgBilling.SetStartingFocus();
	}

	// Bring up first dialog
	ActivateBillingDlg();

	if (m_iBillID != -1)
		GetDlgItem(IDC_EDIT_DESCRIPTION)->SetFocus();

	// (j.jones 2005-10-28 12:11) - PLID 17430 - need to post a message to bill an EMR, if a quote
	if(m_EntryType == 2 && m_nQuoteEMR != -1) {
		PostMessage(NXM_BILL_EMR, (long)m_nQuoteEMR);
	}

	//DRT 5/6/03 - If they are making a new bill, check to see if their insurance company has the
	//		option selected to use ins referral authorizations

	//JMJ 2/7/04 - Only prompt for a referral if the Primary insurance requires it
	// (j.jones 2010-05-20 09:09) - PLID 32338 - we now prompt if any active insurance requires it
	if(m_EntryType == 1 && GetBillID() == -1) {

		try {

			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			_RecordsetPtr rsAuth = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID, InsuredPartyT.RespTypeID "
				"FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} AND RespTypeID > 0 AND (UseReferrals = 1 OR "
				"EXISTS (SELECT ID FROM InsuranceReferralsT "
				"LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
				"GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
				"WHERE NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) "
				"	AND StartDate <= {STRING} AND EndDate >= {STRING} AND InsuredPartyID = InsuredPartyT.PersonID)) "
				"ORDER BY Priority", m_nPatientID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(),dtoDate), FormatDateTimeForSql(COleDateTime::GetCurrentTime(),dtoDate));

			if(!rsAuth->eof) {
				//they have an insurance that uses this stuff, yay.  We can use the already-written function
				//handle prompting them and setting up all the variables too, how handy.
				// (j.jones 2010-05-20 09:16) - PLID 32338 - ApplyInsuranceReferral would re-run the same recordset as above
				// in order to determine if it needs to prompt, and with which insured party, but since we already know that,
				// just pass in the insured party ID
				int nResult = ApplyInsuranceReferral(true, true, AdoFldLong(rsAuth, "PersonID"));

				//now all we have to do is 1)  insert all the diag codes (TODO:  or the first 4 if they had more)
				//and 2)  insert a charge on this bill for every item in their authorization
				if(nResult == IR_SUCCESS) {
					_RecordsetPtr rsAuthSetup = CreateParamRecordset("SELECT InsuredPartyT.PersonID AS InsuredPartyID, AuthNum, LocationID, ProviderID, RespTypeID FROM InsuranceReferralsT "
						"LEFT JOIN InsuredPartyT ON InsuredPartyID = PersonID "
						"WHERE InsuranceReferralsT.ID = {INT}", m_nInsuranceReferralID);

					if(!rsAuthSetup->eof) {
						//1)  Set the location
						long nLocationID = AdoFldLong(rsAuthSetup, "LocationID",-1);
						//if the location is inactive, do not use it
						// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
						if(nLocationID != -1 && ReturnsRecordsParam("SELECT ID FROM LocationsT WHERE ID = {INT} AND Active = 1",nLocationID)) {
							m_dlgBilling.m_PlaceOfServiceCombo->SetSelByColumn(0, nLocationID);
							
							long nPOS = -1;
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							_RecordsetPtr rs = CreateParamRecordset("SELECT POSID FROM POSLocationLinkT INNER JOIN InsuranceCoT ON POSLocationLinkT.HCFASetupGroupID = InsuranceCoT.HCFASetupGroupID "
								"WHERE LocationID = {INT} AND InsuranceCoT.PersonID = {INT}",nLocationID,GetInsuranceCoID(AdoFldLong(rsAuthSetup, "InsuredPartyID")));
							if(!rs->eof) {
								nPOS = AdoFldLong(rs, "POSID", -1);
							}
							rs->Close();

							if(nPOS == -1) {
								//now check to see if there is just one associated with the location
								// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
								rs = CreateParamRecordset("SELECT POSID FROM LocationsT WHERE ID = {INT}",nLocationID);
								if(!rs->eof) {
									nPOS = AdoFldLong(rs, "POSID", -1);					
								}
								rs->Close();
							}

							m_dlgBilling.m_DesignationCombo->SetSelByColumn(0, nPOS);
						}

						//2)  Set the provider
						_variant_t var = rsAuthSetup->Fields->Item["ProviderID"]->Value;
						if(var.vt == VT_I4)
							m_dlgBilling.m_DefaultProvider = VarLong(var);

						//3)  Set the insurance type
						//		Currently this is always primary, but if that changes, this code should hold up to it rather well
						var = rsAuthSetup->Fields->Item["RespTypeID"]->Value;
						if(var.vt == VT_I4) {
							long nRow = m_dlgBilling.m_listBillTo->SetSelByColumn(0, VarLong(var));
							m_dlgBilling.OnSelChosenComboBillTo(nRow);
						}
					}

					//4)  insert the diag codes

					// (j.jones 2007-07-02 14:42) - PLID 24601 - added preferences to control code selection
					// when creating a bill from a referral with multiple diag codes

					long nInsReferralMultiDiagUse = GetRemotePropertyInt("InsReferralMultiDiagUse", 1, 0, "<None>", true);
					// (d.singleton 2014-02-26 15:49) - PLID 61051 - when loading bill from a referral need to load the diag codes correctly
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rsAuthList = CreateParamRecordset("SELECT DiagID, CodeNumber, CodeDesc, ICD10 FROM InsuranceReferralDiagsT INNER JOIN DiagCodes ON DiagID = DiagCodes.ID "
						"WHERE ReferralID = {INT}", m_nInsuranceReferralID);
					//create map of our codes in case the multi select pref is enabled
					CArray<long, long> aryDiagIDs;
					CMap<long, long, DiagCodeInfoPtr, DiagCodeInfoPtr> mapReferralDiagCodes;
					while(!rsAuthList->eof) {
						BOOL bIsICD10 = AdoFldBool(rsAuthList, "ICD10", FALSE);

						DiagCodeInfoPtr pNewDiag = DiagCodeInfoPtr(new DiagCodeInfo);
						pNewDiag->nID = -1;
						if(bIsICD10) {
							pNewDiag->nDiagCode10ID = AdoFldLong(rsAuthList, "DiagID", -1);
							pNewDiag->strDiagCode10Code = AdoFldString(rsAuthList, "CodeNumber", "");
							pNewDiag->strDiagCode10Desc = AdoFldString(rsAuthList, "CodeDesc", "");
							pNewDiag->nDiagCode9ID = -1;
							pNewDiag->strDiagCode9Code = "";
							pNewDiag->strDiagCode9Desc = "";
							//add to map and array
							aryDiagIDs.Add(pNewDiag->nDiagCode10ID);
							mapReferralDiagCodes.SetAt(pNewDiag->nDiagCode10ID, pNewDiag);
						}
						else {
							pNewDiag->nDiagCode9ID = AdoFldLong(rsAuthList, "DiagID", -1);
							pNewDiag->strDiagCode9Code = AdoFldString(rsAuthList, "CodeNumber", "");
							pNewDiag->strDiagCode9Desc = AdoFldString(rsAuthList, "CodeDesc", "");
							pNewDiag->nDiagCode10ID = -1;
							pNewDiag->strDiagCode10Code = "";
							pNewDiag->strDiagCode10Desc = "";
							//add to map and array
							aryDiagIDs.Add(pNewDiag->nDiagCode9ID);
							mapReferralDiagCodes.SetAt(pNewDiag->nDiagCode9ID, pNewDiag);
						}

						m_dlgBilling.m_arypDiagCodes.Add(pNewDiag);

						rsAuthList->MoveNext();
					}
					rsAuthList->Close();

					//if there are multiple codes, and the preference says to prompt, then do so
					// (d.singleton 2014-03-26 15:19) - PLID 61051 - fixed below to work with icd10
					if(m_dlgBilling.m_arypDiagCodes.GetSize() > 1 && nInsReferralMultiDiagUse != 0) {

						// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
						CMultiSelectDlg dlg(this, "DiagCodes");
						//if the preference says to pre-select, do so
						if(nInsReferralMultiDiagUse == 1) {
							dlg.PreSelect(aryDiagIDs);
						}
						
						CString strWhere;
						strWhere.Format("ReferralID = %li AND DiagCodes.Active = 1", m_nInsuranceReferralID);

						// (d.singleton 2014-03-26 15:19) - PLID 61051 - you can now have 12 selections
						int nResult = dlg.Open("InsuranceReferralDiagsT INNER JOIN DiagCodes ON DiagID = DiagCodes.ID", 
							strWhere, "DiagCodes.ID", "DiagCodes.CodeNumber", 
							"Select Diagnosis Codes from this Referral", 0);
						
						if (nResult == IDOK) {
							aryDiagIDs.RemoveAll();
							m_dlgBilling.m_arypDiagCodes.RemoveAll();
							dlg.FillArrayWithIDs(aryDiagIDs);
								
							for(int i = 0; i < aryDiagIDs.GetSize(); i++) {
								DiagCodeInfoPtr pDiag = DiagCodeInfoPtr(new DiagCodeInfo);
								if(mapReferralDiagCodes.Lookup(aryDiagIDs.GetAt(i), pDiag)) {
									m_dlgBilling.m_arypDiagCodes.Add(pDiag);
								}
							}
						}
						else {
							//if they cancelled, add nothing
							aryDiagIDs.RemoveAll();
							m_dlgBilling.m_arypDiagCodes.RemoveAll();
						}
					}

					if(m_dlgBilling.m_arypDiagCodes.GetSize() > 0) {					
						m_dlgBilling.UpdateDiagCodeList();
						m_dlgBilling.ReflectDiagCodeArrayToInterface();
						//(s.dhole 4/2/2015 1:39 PM ) - PLID 65491  Now attache whichcode combo to datalist
						m_dlgBilling.BuildWhichCodesCombo();
					}
					

					//5)  insert the cpt codes

					// (j.jones 2007-07-02 14:42) - PLID 24601 - added preferences to control code selection
					// when creating a bill from a referral with multiple service codes

					long nInsReferralMultiCPTUse = GetRemotePropertyInt("InsReferralMultiCPTUse", 2, 0, "<None>", true);

					CArray<long, long> aryServiceIDs;
					CString strServiceIDs;
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					rsAuthList = CreateParamRecordset("SELECT ServiceID FROM InsuranceReferralCPTCodesT "
						"WHERE ReferralID = {INT}", m_nInsuranceReferralID);
					while(!rsAuthList->eof) {						
						aryServiceIDs.Add(AdoFldLong(rsAuthList, "ServiceID"));						
						rsAuthList->MoveNext();
					}
					rsAuthList->Close();

					//if there are multiple codes, and the preference says to prompt, then do so
					if(aryServiceIDs.GetSize() > 1 && nInsReferralMultiCPTUse != 0) {

						// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
						CMultiSelectDlg dlg(this, "CPTCodeT");
						//if the preference says to pre-select, do so
						if(nInsReferralMultiCPTUse == 1) {
							dlg.PreSelect(aryServiceIDs);
						}
						
						CString strWhere;
						strWhere.Format("ReferralID = %li AND ServiceT.Active = 1", m_nInsuranceReferralID);

						int nResult = dlg.Open("InsuranceReferralCPTCodesT "
							"INNER JOIN ServiceT ON InsuranceReferralCPTCodesT.ServiceID = ServiceT.ID "
							"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID ", 
							strWhere, "ServiceT.ID", "CPTCodeT.Code + ' ' + CPTCodeT.SubCode", 
							"Select Service Codes from this Referral");
						
						if (nResult == IDOK) {
							aryServiceIDs.RemoveAll();
							dlg.FillArrayWithIDs(aryServiceIDs);
						}
						else {
							//if they cancelled, add nothing
							aryServiceIDs.RemoveAll();
						}
					}

					if(aryServiceIDs.GetSize() > 0) {
						//add the remaining service codes to the bill
						for(int i=0;i<aryServiceIDs.GetSize();i++) {
							m_dlgBilling.AddNewChargeToBill((long)(aryServiceIDs.GetAt(i)));
						}
					}
				}
			}
			else {
				m_bPromptForReferral = FALSE;
			}
		} NxCatchAll("Error checking insurance authorization status.");
	}

}

// (j.jones 2011-06-30 08:57) - PLID 43770 - added bFromEMR, which, if true,
// means we should be later posting the NXM_BILL_EMR message, and the bill
// needs to know we're about to do that
// (d.singleton 2014-02-27 17:16) - PLID 61072 - new preference to open menu when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
// (j.armen 2014-08-06 10:06) - PLID 63161 - Use an enum for the bill entry type
// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
void CBillingModuleDlg::OpenWithBillID(long lBillID, BillEntryType lEntryType, int iSaveType, BillFromType eFromType /*= BillFromType::Other*/, long nBillToInsPartyID /*= -1*/)
{
	if(lEntryType == 1 && !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {
		MsgBox("You are not licensed to access this feature.");
		return;
	}
	if(lEntryType == 2 && !g_pLicense->CheckForLicense(CLicense::lcQuotes, CLicense::cflrUse)) {
		MsgBox("You are not licensed to access this feature.");
		return;
	}

	if (lBillID < 0)
		m_boIsNewBill = TRUE;

	m_iBillID = lBillID;
	m_EntryType = lEntryType;
	m_iSaveType = iSaveType;
	// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
	m_eCreatedFrom = eFromType;
	// (d.singleton 2014-02-27 17:20) - PLID 61072 - when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
	m_nBillToInsPartyID = nBillToInsPartyID;

	if(m_EntryType==2) {
		//this is usually true, but now quotes can have payments
		//only package quotes do, and they will set this boolean appropriately
		m_boAskForNewPayment = FALSE;
		DoModal();
	}
	else {
		ShowWindow(SW_SHOW);
		ASSERT(IsWindowVisible());
		if (IsWindowVisible()) {
			// (a.walling 2009-12-22 17:08) - PLID 7002 - We are now modeless, so we don't need this anymore
			// (j.jones 2012-10-30 16:33) - PLID 53444 - we still need to toggle hotkeys, because the bill
			// has several hotkeys that are also in use by mainframe, and we can't get rid of them now
			GetMainFrame()->DisableHotKeys();
			//GetMainFrame()->EnableWindow(FALSE);
			EnableWindow();
		} else {
			LogDetail("ERROR: Bill dialog not visible!");
		}
	}
}

void CBillingModuleDlg::OnBillDeleteBtn() 
{
	CString str;

	if(m_EntryType==2) {
		if (!CheckCurrentUserPermissions(bioPatientQuotes,sptDelete))
			return;

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam("SELECT ID FROM ProcInfoT WHERE ActiveQuoteID = {INT}",m_iBillID)) {
			//active quote on a procedure
			str = "This quote is the active quote for a tracked procedure. Are you sure you wish to permanently delete this quote?";
		}
		else if(ReturnsRecordsParam("SELECT ProcInfoID FROM ProcInfoQuotesT WHERE QuoteID = {INT}",m_iBillID)) {
			//attached to a procedure
			str = "This quote is attached to a tracked procedure. Are you sure you wish to permanently delete this quote?";
		}
		else {
			//normal quote delete
			str = "You are about to permanently delete this quote. Are you sure you wish to do this?";
		}
	}
	else {

		// (j.jones 2011-08-24 08:41) - PLID 44868 - you cannot edit a bill that is an "original"
		// bill, meaning it has since been corrected, per BillCorrectionsT
		if(m_iBillID != -1) {
			if(m_bHasBeenCorrected) {
				MessageBox("This bill has been corrected, and cannot be deleted. You must delete the corrected bill instead.", "Practice", MB_ICONINFORMATION|MB_OK);
				return;
			}
			
			if(HasOriginalOrVoidCharge()) {
				MessageBox("At least one charge on this bill has been corrected, and cannot be deleted.\n\n"
					"You may, however, delete individual charges from this bill, provided that they are not read-only.", "Practice", MB_ICONINFORMATION|MB_OK);
				return;
			}

			//TES 7/25/2014 - PLID 63048 - Don't allow them to delete chargeback charges
			if (DoesChargeHaveChargeback(CSqlFragment("ChargesT.BillID = {INT}", m_iBillID))) {
				MessageBox("This bill is associated with at least one Chargeback, and cannot be deleted. In order to delete this bill, you must first expand the bill "
					"on the Billing tab, right-click on any associated Chargebacks, and select 'Undo Chargeback.'");
				return;
			}
		}

		// (c.haag 2009-04-23 14:10) - PLID 32433 - Use CanChangeHistoricFinancial
		if (!CanChangeHistoricFinancial("Bill", m_iBillID, bioBill, sptDelete)) {
			return;
		}
		//if (!CheckCurrentUserPermissions(bioBill,sptDelete))
		//	return;

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (j.dinatale 2012-07-11 09:28) - PLID 51468 can now have multiple bills on a PIC
		if(ReturnsRecordsParam("SELECT TOP 1 1 FROM ProcInfoBillsT WHERE BillID = {INT}",m_iBillID)) {
			//attached to a procedure
			str = "This bill is attached to a tracked procedure. Are you sure you wish to permanently delete this bill?";
		}
		else {
			//normal bill delete
			str = "You are about to permanently delete this bill. Are you sure you wish to do this?";
		}
	}

	if(IDYES==MessageBox(str,"Practice",MB_YESNO|MB_ICONEXCLAMATION)) {

		// (j.jones 2008-05-27 17:06) - PLID 27982 - CheckWarnDeletingChargedProductItems will potentially
		// warn a second time, if there are existing saved product items linked to the bill
		// (j.jones 2008-06-09 17:20) - PLID 28392 - CheckWarnAlteringReturnedProducts will warn yet again
		// if there are products returned from this bill
		// (j.jones 2013-07-23 10:03) - PLID 46493 - these functions have a void flag now, always false here
		// (j.jones 2015-11-04 09:36) - PLID 67459 - flipped the return product warning and product item warning
		if(m_EntryType == 2
			|| (CheckWarnAlteringReturnedProducts(true, false, m_iBillID)
				&& CheckWarnDeletingChargedProductItems(true, false, m_iBillID))) {

			if(m_iBillID != -1) {
				DeleteBill(m_iBillID);
			}
			CloseWindow();
		}
	}
}

void CBillingModuleDlg::OnOK()
{
	//CloseWindow();
}

void CBillingModuleDlg::GetTabRect15(CRect &rect)
{
	CRect rcClient, rcTab;

	GetClientRect(rcClient);
	GetDlgItem(IDC_TAB)->GetWindowRect(rcTab);
	ScreenToClient(&rcTab);
	rect = rcClient;
	rect.top = rcTab.bottom + 1;
}

void CBillingModuleDlg::ActivateBillingDlg()
{
	CRect rectSheet;

	GetTabRect15(rectSheet);

	m_dlgBilling.MoveWindow(rectSheet);	
	m_dlgBilling.ShowWindow(SW_SHOW);
	
	if (m_dlgInsuranceBilling.GetSafeHwnd())
		m_dlgInsuranceBilling.ShowWindow(SW_HIDE);

	if (m_dlgBilling2.GetSafeHwnd())
		m_dlgBilling2.ShowWindow(SW_HIDE);

	m_pActiveDlg = &m_dlgBilling;
	m_dlgBilling.SetStartingFocus();
}

void CBillingModuleDlg::ActivateInsuranceDlg()
{
	CRect rectSheet;
	GetTabRect15(rectSheet);
	m_dlgInsuranceBilling.MoveWindow(rectSheet);
	m_dlgBilling.ShowWindow(SW_HIDE);

	if (m_dlgBilling2.GetSafeHwnd())
		m_dlgBilling2.ShowWindow(SW_HIDE);

	m_dlgInsuranceBilling.ShowWindow(SW_SHOW);

	m_pActiveDlg = &m_dlgInsuranceBilling;
}

void CBillingModuleDlg::ActivateBilling2Dlg()
{
	CRect rectSheet;
	GetTabRect15(rectSheet);
	m_dlgBilling2.MoveWindow(rectSheet);

	m_dlgBilling.ShowWindow(SW_HIDE);

	if (m_dlgInsuranceBilling.GetSafeHwnd())
		m_dlgInsuranceBilling.ShowWindow(SW_HIDE);

	m_dlgBilling2.ShowWindow(SW_SHOW);

	m_pActiveDlg = &m_dlgBilling2;
}

void CBillingModuleDlg::OnBillEditBtn() 
{
	// (j.jones 2011-08-24 08:41) - PLID 44868 - you cannot edit a bill that is an "original"
	// bill, meaning it has since been corrected, per BillCorrectionsT
	if(m_iBillID != -1 && m_EntryType == 1 && m_bHasBeenCorrected) {
		MessageBox("This bill has been corrected, and is no longer editable. If a corrected bill exists, you can edit that bill in order to make changes.", "Practice", MB_ICONINFORMATION|MB_OK);
		return;
	}

	// (j.jones 2007-04-19 11:11) - PLID 25721 - In most cases CanEdit is not needed, so first
	// silently see if they have permission, and if they do, then move ahead normally. But if
	// they don't, or they need a password, then check CanEdit prior to the permission check that
	// would stop or password-prompt the user.
	if(m_iBillID != -1 &&

		// (c.haag 2009-03-10 10:29) - PLID 32433 - This code has been superceded with CanChangeHistoricFinancial
		// (j.jones 2011-01-21 09:06) - PLID 42156 - I removed the call to CanChangeHistoricFinancial, now you can edit
		// a bill if you have regular write access (or the "CanEdit" same day access), but when you try to change
		// anything that might affect A/R, only then do we check CanChangeHistoricFinancial. Minor fields are still editable.
		// (j.jones 2011-07-20 14:33) - PLID 44648 - moved CanEdit to be called prior to CheckCurrentUserPermissions,
		// because if you can edit it there is no point in giving the permission warning
		(
			(m_EntryType == 1 && CanEdit("Bill", m_iBillID)) ||
			(m_EntryType == 1 && CheckCurrentUserPermissions(bioBill, sptWrite)) ||
			(m_EntryType == 2 && (GetCurrentUserPermissions(bioPatientQuotes) & sptWrite)) ||
			// (j.gruber 2009-06-30 09:32) - PLID 34737 - make sure we give the user some feedback when they click the button
			(m_EntryType == 2 && CheckCurrentUserPermissions(bioPatientQuotes,sptWrite))
		)
		) {

		if(m_eHasAccess == batNoAccess) {
			//swap the status

			// (j.jones 2011-01-21 09:09) - PLID 42156 - if a bill, it's editable if we have normal write
			// permission, but now check to see if it is locked from a close or an old service date
			CString strBillEditWarning;
			if(m_EntryType == 1 && !IsBillWriteable_HistoricFinancial(TRUE, &strBillEditWarning)) {

				//they can't edit some fields, so switch to partial access
				m_eHasAccess = batPartialAccess;
			}
			else {
				m_eHasAccess = batFullAccess;
			}

			//If strBillEditWarning is not empty, we want to notify the user of its results.
			//Currently this is only used to tell them that the bill is locked due to a financial close.
			//It may still be filled even if you have full access, just to warn you of fields you
			//may yet get yelled about later.
			if(!strBillEditWarning.IsEmpty()) {
				MsgBox(MB_ICONINFORMATION|MB_OK, strBillEditWarning);
			}

			m_bHasEverHadAccess = TRUE;
		}
		else {
			// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
			m_eHasAccess = batNoAccess;
		}

		// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
		// if they only have partial access then the date & pat. coord. is disabled,
		// as is the ability to delete
		GetDlgItem(IDC_BILL_DELETE_BTN)->EnableWindow(m_eHasAccess == batFullAccess && m_iBillID != -1);
		GetDlgItem(IDC_BILL_DATE)->EnableWindow(m_eHasAccess == batFullAccess);
		GetDlgItem(IDC_EDIT_DESCRIPTION)->EnableWindow(m_eHasAccess != batNoAccess);
		GetDlgItem(IDC_COORDINATOR_COMBO)->EnableWindow(m_eHasAccess == batFullAccess);

		m_dlgBilling.m_eHasAccess = m_eHasAccess;
		m_dlgBilling.ChangeAccess();
		if (m_dlgInsuranceBilling.GetSafeHwnd()) {
			m_dlgInsuranceBilling.m_eHasAccess = m_eHasAccess;
			m_dlgInsuranceBilling.ChangeAccess();
		}
		if (m_dlgBilling2.GetSafeHwnd()) {
			m_dlgBilling2.m_eHasAccess = m_eHasAccess;
			m_dlgBilling2.ChangeAccess();
		}

		if(m_eHasAccess != batNoAccess) {
			if (m_pActiveDlg == &m_dlgBilling) {
				m_dlgBilling.SetStartingFocus();
			}
		}
		else {
			GetDlgItem(IDC_EDIT_DESCRIPTION)->SetFocus();
		}
	}
}

void CBillingModuleDlg::OnBillPreviewBtn() 
{
	CString str, strtemp;
	_variant_t var;

	try {

		if(m_dlgBilling.m_pList == NULL) {
			//DRT 6/2/2006 - PLID 20879 - Possibly problematic code.  If this happens, the quote
			//	must not be setup correctly.  Fail to run.
			AfxThrowNxException("Invalid charge list control.");
			return;
		}

		if(m_dlgBilling.m_pList->GetRowCount() == 0) {
			if(m_EntryType == 2)
				AfxMessageBox("There are no charges on this quote. The preview will not be generated.");
			else
				AfxMessageBox("There are no charges on this bill. The preview will not be generated.");

			return;
		}

		if(!Save())
			return;

		// (z.manning, 07/27/2006) - PLID 21645 - We don't want them to be able to do anything while
		// the report is processing.  Is called after Save() because Save may enable some buttons.
		EnableButtons(FALSE);

		if (m_EntryType == 2) 
		{
			//give user a message saying what is happening
			if(m_bHasEverHadAccess) {
				MsgBox("This quote will be saved and closed. \n You can edit the quote further after closing the quote preview");
			}

			long nReportNumber = 0;
			// (j.gruber 2007-08-14 11:02) - PLID 27068 - addedreportId since it can be more then one now
			//defaulting to the regular quote
			long nReportID = 227;
			CString strCustomReportFileName;
			if(m_pQuoteCustomList->CurSel != -1) {
				//check to see what report they want to run
				nReportNumber = VarLong(m_pQuoteCustomList->GetValue(m_pQuoteCustomList->CurSel, qclcNumber));
				nReportID = VarLong(m_pQuoteCustomList->GetValue(m_pQuoteCustomList->CurSel, qclcID));
				strCustomReportFileName = VarString(m_pQuoteCustomList->GetValue(m_pQuoteCustomList->CurSel, qclcFileName));
			}

			if (m_dlgBilling.GetIsAnyPackage()) {
				//make sure we have a package report
				// (j.gruber 2007-11-20 12:26) - PLID 28061 - allow them to print old custom reports
				if (nReportID != 608 && nReportNumber == -1) {
					MsgBox("You may only print a package quote report when quoting a package, please choose a package quote report from the drop down.");
					m_pQuoteCustomList->DropDownState = 1;
					EnableButtons(TRUE);
					return;
				}
			}
			else {
				//make sure we have a non-package report
				if (nReportID != 227) {
					MsgBox("You may only print a non-package quote report when quoting a regular quote, please choose a non-package quote report from the drop down.");
					m_pQuoteCustomList->DropDownState = 1;
					EnableButtons(TRUE);
					return;
				}
			}

			try {

				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT ExtraDesc, EntryType FROM BillsT WHERE ID = {INT}",m_iBillID);
				//DRT 6/2/2006 - PLID 20879 - Possibly problematic code
				if(rs->eof) {
					//This really shouldn't ever happen, and if it did, it probably means the Save() above failed, but
					//	somehow returned true.  We're just going to quit previewing altogether.
					return;
				}

				//DRT 6/2/2006 - PLID 20879 - This field is not allowed to be NULL
				var = rs->Fields->Item["ExtraDesc"]->Value;

				//DRT 6/2/2006 - PLID 20879 - Possibly problematic code
				if(var.vt != VT_NULL && (VarString(var) != m_strExtraDesc)) {
					long AuditID = BeginNewAuditEvent();

					if (AdoFldByte(rs, "EntryType") == 2)	//2 == quote
						//DRT 6/2/2006 - PLID 20879 - Possibly problematic code
						AuditEvent(m_nPatientID, GetBillPatientName(), AuditID, aeiQuoteExtraDesc, m_iBillID, VarString(var), m_strExtraDesc, aepLow);
					else
						//DRT 6/2/2006 - PLID 20879 - Possibly problematic code
						AuditEvent(m_nPatientID, GetBillPatientName(),AuditID,aeiBillExtraDesc,m_iBillID, VarString(var),m_strExtraDesc,aepLow);
				}
				rs->Close();

				_variant_t varReportNumber = g_cvarNull;
				if(nReportNumber != -1) {
					varReportNumber = _variant_t(nReportNumber, VT_I4);
				}

				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				// (j.jones 2013-06-04 10:17) - PLID 31874 - we now save the last report they previewed for this quote
				ExecuteParamSql("UPDATE BillsT SET ExtraDesc = {STRING}, LastQuoteReportID = {INT}, LastQuoteReportNumber = {VT_I4} "
					"WHERE ID = {INT}", m_strExtraDesc, nReportID, varReportNumber,
					m_iBillID);

			} NxCatchAll("CBillingModuleDlg::OnBillPreviewBtn - Updating Bill");

			//DRT 6/2/2006 - PLID 20879 - Possibly problematic code
			try {

				// (j.gruber 2007-08-15 09:53) - PLID 27068 - I found and fixed this when doing the package quote,
				//it didn't work because the IDC_PACKAGE_QUOTE that it checked is on the billing dlg, not this one
				PreviewQuote(m_iBillID, m_strExtraDesc, m_dlgBilling.GetIsAnyPackage()? true:false, this, nReportID, nReportNumber, strCustomReportFileName);

			} NxCatchAllCall("CBillingModuleDlg::OnBillPreviewBtn - Error previewing quote", return;);

			//DRT 6/2/2006 - PLID 20879 - Possibly problematic code
			try {
				//JJ - this must be after the ViewReport or else PostEditBill is never fired (in release mode)
				CloseWindow();
			} NxCatchAllCall("CBillingModuleDlg::OnBillPreviewBtn - CloseWindow", return;);

			//DRT 5/11/2004 - PLID 12071 - The quote can be opened from other places (like tracking).  If we've
			//	previewed the report, the PIC dialog (or others) may still be open and blocking the report from 
			//	being on top.  If so, we need to post a message back telling the parent window to close itself.
			if(m_pPostCloseMsgWnd) {
				m_pPostCloseMsgWnd->SendMessage(NXM_QUOTE_PREVIEWING, 0, 0);
				m_pPostCloseMsgWnd = NULL;	//quote is closed, we no longer need this
			}
		}
		else
		{
			if(m_bHasEverHadAccess) {
				MsgBox("This bill will be saved and closed. \n You can edit the bill further after closing the bill preview");
			}

			PreviewBill(m_iBillID, this);

			//JJ - this must be after the ViewReport or else PostEditBill is never fired (in release mode)
			CloseWindow();
		}
	}NxCatchAll("Error in Previewing Bill");

	EnableButtons(TRUE);
}

int CBillingModuleDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CBillingModuleDlg::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{	
	return CNxDialog::Create(IDD, pParentWnd);
}

BOOL CBillingModuleDlg::Create(CWnd* pParentWnd, UINT nIDTemplate)
{
	return CNxDialog::Create(nIDTemplate, pParentWnd);
}


void CBillingModuleDlg::SetQuoteReportDropDown(BOOL bIsPackage) {

	try {
		if (m_EntryType == 2) {

			long nReportID, nOtherReportID;
			CString strReportFileName;

			CString strByUserName, strOtherByUserName;
			if (bIsPackage) {
				strByUserName = "PackageQuoteDefaultByUser";
				strOtherByUserName = "QuoteDefaultByUser";
				nReportID = 608;
				nOtherReportID = 227;
			}
			else {
				strByUserName = "QuoteDefaultByUser";
				strOtherByUserName = "PackageQuoteDefaultByUser";
				nReportID = 227;
				nOtherReportID = 608;
			}
			
						
			{
				//see if they have a UserDefault
				long nNumber = GetRemotePropertyInt(strByUserName, -2, 0, GetCurrentUserName(), FALSE);
					
				if (nNumber != -2) {

					
					CString strCombo = AsString(nReportID) + "-" + AsString(nNumber);

					m_pQuoteCustomList->SetSelByColumn(qclcRepNum, _variant_t(strCombo));
					long nRow = m_pQuoteCustomList->FindByColumn(qclcRepNum, _variant_t(strCombo), -1, FALSE);
					NXDATALISTLib::IRowSettingsPtr pRow = m_pQuoteCustomList->GetRow(nRow);
					if (pRow) {
						if (bIsPackage) {
							pRow->PutForeColor(RGB(0,0,255));
						}
						else {
							pRow->PutForeColor(RGB(255,0,0));
						}
					}
				}
				else {

					//pull the default
					_RecordsetPtr rsDefault;
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					rsDefault = CreateParamRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = {INT}", nReportID);
					
					if (rsDefault->eof) {

						//no default, set it to the system report
						CString strCombo;
						strCombo.Format("%li--1", nReportID);
						m_pQuoteCustomList->SetSelByColumn(qclcRepNum, _variant_t(strCombo));

						long nRow = m_pQuoteCustomList->FindByColumn(qclcRepNum, _variant_t(strCombo), -1, FALSE);
						NXDATALISTLib::IRowSettingsPtr pRow = m_pQuoteCustomList->GetRow(nRow);
						if (pRow) {
							if (bIsPackage) {
								pRow->PutForeColor(RGB(0,0,255));
							}
							else {
								pRow->PutForeColor(RGB(255,0,0));
							}
						}
					}
					else {

						//there is a default
						long nDefault = AdoFldLong(rsDefault, "CustomReportID", -1);			
						CString strCombo = AsString(nReportID) + "-" + AsString(nDefault);
						m_pQuoteCustomList->SetSelByColumn(qclcRepNum, _variant_t(strCombo));

						long nRow = m_pQuoteCustomList->FindByColumn(qclcRepNum, _variant_t(strCombo), -1, FALSE);
						NXDATALISTLib::IRowSettingsPtr pRow = m_pQuoteCustomList->GetRow(nRow);
						if (pRow) {
							if (bIsPackage) {
								pRow->PutForeColor(RGB(0,0,255));
							}
							else {
								pRow->PutForeColor(RGB(255,0,0));
							}
						}

					}
				}
			}
			
			//now set the default for the non-selected default

			//see if they have a PackageUserDefault
			{
				long nOtherNumber = GetRemotePropertyInt(strOtherByUserName, -2, 0, GetCurrentUserName(), FALSE);
					
				if (nOtherNumber != -2) {

					//they have a user default
					CString strCombo = AsString(nOtherReportID) + "-" + AsString(nOtherNumber);

					long nRow = m_pQuoteCustomList->FindByColumn(qclcRepNum, _variant_t(strCombo), -1, FALSE);
					NXDATALISTLib::IRowSettingsPtr pRow = m_pQuoteCustomList->GetRow(nRow);
					if (pRow) {
						if (bIsPackage) {
							pRow->PutForeColor(RGB(255,0,0));
						}
						else {
							pRow->PutForeColor(RGB(0,0,255));
						}
					}
							
				}
				else {

					//pull the default for the other quote
					_RecordsetPtr rsDefault;
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					rsDefault = CreateParamRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = {INT}", nOtherReportID);
					
					if (rsDefault->eof) {

						//no default, set it to the system report
						CString strCombo;
						strCombo.Format("%li--1", nOtherReportID);
						long nRow = m_pQuoteCustomList->FindByColumn(qclcRepNum, _variant_t(strCombo), -1, FALSE);
						NXDATALISTLib::IRowSettingsPtr pRow = m_pQuoteCustomList->GetRow(nRow);
						if (pRow) {
							if (bIsPackage) {
								pRow->PutForeColor(RGB(255,0,0));
							}
							else {
								pRow->PutForeColor(RGB(0,0,255));
							}
						}

					}
					else {

						long nDefault = AdoFldLong(rsDefault, "CustomReportID", -1);			
						CString strCombo = AsString(nOtherReportID) + "-" + AsString(nDefault);
						
						long nRow = m_pQuoteCustomList->FindByColumn(qclcRepNum, _variant_t(strCombo), -1, FALSE);
						NXDATALISTLib::IRowSettingsPtr pRow = m_pQuoteCustomList->GetRow(nRow);
						if (pRow) {
							if (bIsPackage) {
								pRow->PutForeColor(RGB(255,0,0));
							}
							else {
								pRow->PutForeColor(RGB(0,0,255));
							}
						}

					}
				}
			}

			// (j.jones 2013-06-04 10:16) - PLID 31874 - now we have set a default, and colored rows
			// appropriately, but if they have already previewed a report once for this quote, change
			// the selection to use that report if the report still exists in the dropdown
			if(m_iBillID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT LastQuoteReportID, LastQuoteReportNumber "
					"FROM BillsT WHERE ID = {INT}", m_iBillID);
				if(!rs->eof) {
					long nReportID = VarLong(rs->Fields->Item["LastQuoteReportID"]->Value, -1);
					//if there is a report ID, try to select it, but also try to use the same custom report number 
					if(nReportID != -1) {
						long nReportNumber = VarLong(rs->Fields->Item["LastQuoteReportNumber"]->Value, -1);
						//nReportNumber might be -1, and that's ok
						CString strCombo = AsString(nReportID) + "-" + AsString(nReportNumber);
						if(m_pQuoteCustomList->SetSelByColumn(qclcRepNum, _variant_t(strCombo)) == -1) {
							//not found, perhaps the custom report doesn't exist anymore?
							if(nReportNumber != -1) {
								//we had a custom report, so instead just set it to the system report
								strCombo.Format("%li--1", nReportID);
								m_pQuoteCustomList->SetSelByColumn(qclcRepNum, _variant_t(strCombo));
								//don't bother checking for failure here, we just don't change the default
							}
						}
					}

					//if we couldn't set a selection for any reason, do nothing,
					//as the preceding code would have already set a default selection for us
				}
				rs->Close();
			}
		}
		
	}NxCatchAll("Error in CBillingModuleDlg::SetQuoteReportDropDown");
}


BOOL CBillingModuleDlg::OnInitDialog() 
{
	try
	{
		__super::OnInitDialog();

		HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
		SetIcon(hIcon, TRUE);
		SetIcon(hIcon, FALSE);

		// (a.walling 2009-12-23 10:38) - PLID 7002 - Remove minimize box for quotes, which are still modal
		if(m_EntryType==2) {
			ModifyStyle(WS_MINIMIZEBOX, 0, SWP_FRAMECHANGED);
		}

		// (j.jones 2006-04-13 17:06) - PLID 20136 - Load all common billing dialog properties into the
		// NxPropManager cache
		// (a.walling 2009-02-23 10:42) - PLID 11034 - Cache the resized value for bills and quotes
		g_propManager.CachePropertiesInBulk("BillingModuleDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'BillingModuleSize_Bill' OR "
			"Name = 'BillingModuleSize_Quote' OR "
			"Name = 'CPTSearch' OR "
			"Name = 'ProductSearch' OR "
			"Name = 'SrgySearch' OR "
			"Name = 'Diag1Search' OR "
			"Name = 'Diag2Search' OR "
			"Name = 'Diag3Search' OR "
			"Name = 'Diag4Search' OR "
			"Name = 'SortBillProvList' OR "
			"Name = 'RememberBillingColumnWidths' OR "
			"Name = 'BillPrimaryIns' OR "
			"Name = 'BillPrimaryInsWhenAccepted' OR "
			"Name = 'BillLocationPref' OR "
			"Name = 'DefaultTaxSource' OR "
			"Name = 'DefaultAccidentType' OR "
			"Name = 'DefaultBillAChoice' OR "
			"Name = 'SortBills' OR "
			"Name = 'ApplySuperbillIDs' OR "
			"Name = 'DefaultBillDate' OR "
			"Name = 'DefaultQuoteDate' OR "
			"Name = 'ApplySuperbillUseAptDate' OR "
			"Name = 'QuoteDefaultByUser' OR "
			"Name = 'DefaultAssignmentToInputDate' OR "
			"Name = 'BillMarkApptOut' OR "
			"Name = 'AlwaysUseDefaultClaimBatch' OR "
			"Name = 'DefaultConditionType' OR "
			"Name = 'QuoteDefaultExpires' OR "
			"Name = 'DefaultQuoteAChoice' OR "
			"Name = 'DefaultCurrentAccidentToLastBillWhenG2Empty' OR "
			"Name = 'DefaultFirstAccidentToLastBillWhenG2Empty' OR "
			"Name = 'DefaultHospDatesToLastBill' OR "
			"Name = 'DefaultNoWorkDatesToLastBill' OR "
			"Name = 'ChargeAllowQtyIncrement' OR "
			//"Name = 'EMNBillPopupDiagCodes' OR "	// (j.jones 2009-03-25 16:22) - PLID 9729 - obsolete
			// (j.jones 2007-05-02 12:19) - PLID 25844 - cached all the copay preferences
			"Name = 'PromptForCoPay' OR "
			"Name = 'WhenUseCopay' OR "	
			"Name = 'ShiftCoPayAmount' OR "
			//"Name = 'ShiftCoPayAmountWhenCPTRequires' OR "	// (j.jones 2010-08-02 10:21) - PLID 39938 - removed, obsolete
			"Name = 'ShiftCoInsuranceAmount' OR "	// (j.jones 2010-08-04 09:01) - PLID 38613
			// (a.walling 2007-05-08 17:07) - PLID 14717 - cache suggested sales prefs
			"Name = 'SuggestedSalesPopupWidth' OR "
			"Name = 'SuggestedSalesPopupHeight' OR "
			"Name = 'SuggestedSalesPopupTop' OR "
			"Name = 'SuggestedSalesPopupLeft' OR "
			// (j.jones 2007-05-22 09:31) - PLID 25558 - added bill EMN preference
			"Name = 'BillEMNWithProvider' OR "
			// (j.jones 2007-07-02 14:42) - PLID 24601 - cache the insurance referral prompting preferences
			"Name = 'InsReferralMultiDiagUse' OR "
			"Name = 'InsReferralMultiCPTUse' OR "
			// (z.manning, 08/03/2007) - PLID 26140 - Cache preference to prompt to transfer inventory item.
			"Name = 'BillPromptTransferInventoryLocation' OR "
			// (j.jones 2007-08-10 10:19) - PLID 23769 - included the packages/quotes options
			"Name = 'IncludeAllPrePaysInPopUps' OR "
			"Name = 'DoNotBillNonPrepaidPackages' OR "
			// (j.jones 2008-01-17 11:08) - PLID 28641 - included the allocation notes preference
			"Name = 'BillCreateAllocationNotes' OR "
			// (j.jones 2008-02-11 16:14) - PLID 28847 - included DisallowBatchingPatientClaims
			"Name = 'DisallowBatchingPatientClaims' OR "
			// (j.jones 2008-02-12 11:35) - PLID 28848 - added HidePatientChargesOnClaims
			"Name = 'HidePatientChargesOnClaims' OR "
			// (j.jones 2008-11-13 16:55) - PLID 21149 - added ChangeChargeProviderPrompt (and ChangeChargeDatesPrompt, which was never cached before)
			"Name = 'ChangeChargeDatesPrompt' OR "
			// (d.thompson 2009-08-26) - PLID 33953 - Added ChangeBillDatesPrompt, which works the same as the charge dates version, but is fired on bill date changes
			"Name = 'ChangeBillDatesPrompt' OR "
			"Name = 'ChangeChargeProviderPrompt' OR "
			// (j.jones 2008-12-23 11:28) - PLID 32492 - added LastDiscountBillOverwriteOption
			"Name = 'LastDiscountBillOverwriteOption' OR "
			// (j.jones 2009-02-11 10:09) - PLID 33035 - added EbillingFormatType
			"Name = 'EbillingFormatType' OR "
			"Name = 'UseOHIP' OR "	// (j.jones 2009-03-31 15:11) - PLID // (j.jones 2009-03-31 15:11) - PLID 33747
			// (c.haag 2009-04-20 09:17) - PLID 14298
			"Name = 'BillingDlg_PromptForNewBillPayment' OR "
			//(e.lally 2009-08-18) PLID 28825 - RequireDiscountCategory
			"Name = 'RequireDiscountCategory' OR "
			// (j.jones 2009-08-21 09:46) - PLID 31131 - added preference to include the diag. desc in the collapsed diag. dropdown
			"Name = 'DiagDropdownIncludeDesc' OR "
			// (c.haag 2009-08-24 13:08) - PLID 14844 - Creating new bills from appts would set the bill date
			"Name = 'SetNewBillDateToApptDate' OR "
			//(e.lally 2009-09-10) PLID 23163 - Default referring phys behavior for insurance tab
			"Name = 'BillSetDefaultRefPhys' OR "
			"Name = 'SetNewBillChargeDatesToApptDate' OR "
			// (j.jones 2009-09-14 16:51) - PLID 35382 - added case history auto-linking preference
			"Name = 'AutoLinkCaseHistoriesToBills' OR "
			// (j.jones 2009-09-28 17:04) - PLID 35686 - added preference to warn if POS = 21 and no hosp. dates are entered
			"Name = 'CheckBillHospDatesWhenPOS21' OR "
			// (j.jones 2009-10-26 09:23) - PLID 32904 - added PackageTaxEstimation
			"Name = 'PackageTaxEstimation' OR "
			// (j.jones 2009-10-27 13:51) - PLID 35934 - added preference to adjust package balances
			"Name = 'PackageTaxAdjustments' OR "
			// (j.jones 2010-06-14 10:55) - PLID 38507 - added preference to show the HCFA Box 13 override
			"Name = 'ShowBillHCFABox13Setting' OR "
			// (j.jones 2010-08-30 09:53) - PLID 32788 - added pref. to not auto-fill the bill description
			"Name = 'BillDoNotAutoFillDescription' OR "
			// (j.jones 2010-08-30 11:02) - PLID 40196 - added pref. to not auto-fill the DiagCs column
			"Name = 'BillDoNotAutoFillDiagCs' OR "
			// (j.jones 2010-08-30 11:48) - PLID 40293 - added pref. to warn about unlinked diags
			"Name = 'BillWarnUnlinkedDiagnosis' OR "
			// (j.jones 2010-09-03 08:43) - PLID 38319 - added ability to force products to be patient resp
			"Name = 'AddInventoryAsPatResp' OR "
			// (s.dhole 2010-09-13 16:05) - PLID 38434 assign default POS for Quote
			"Name = 'DefaultQuotePlaceOfService' OR "
			// (j.jones 2010-11-08 16:51) - PLID 39620 - added Alberta pref
			"Name = 'UseAlbertaHLINK' OR "
			// (j.jones 2010-11-10 16:05) - PLID 32887 - cached BillUseAppointmentProvider and a couple stragglers I found
			"Name = 'BillUseAppointmentProvider' OR "
			"Name = 'ApplySuperbillUseProvider' OR "
			"Name = 'UseDefaultLocationProviderOnCharges' OR "
			"Name = 'BarcodeAllowQtyIncrement' OR "
			"Name = 'IsShowDiagnosisCodeInDiagCsColumn' OR " // (s.dhole 2011-05-20 12:50) - PLID 33666 set show code vailu in Billing Dig code column
			"Name = 'BillLoadG2Codes' OR "	// (j.jones 2011-06-29 17:00) - PLID 43770 - added BillLoadG2Codes
			"Name = 'AutoBillPackage' "	// (j.jones 2011-07-08 13:49) - PLID 26785 - added ability to auto-bill new packages
			// (j.jones 2011-07-08 17:34) - PLID 32067 - added DefaultChargesNoProvider and RequireProviderOnCharges
			"OR Name = 'DefaultChargesNoProvider' "
			"OR Name = 'RequireProviderOnCharges' "
			//(e.lally 2011-08-26) PLID 45210 - cache a bunch more preferences.
			"OR Name = 'dontshow BillingDlgInformational' "
			"OR Name = 'DefaultBillColumnSizes' "
			"OR Name = 'KeepQuoteDiscounts' "
			"OR Name = 'WarnBillDateInFuture' "
			"OR Name = 'RefPhysComboShowNPI' "
			"OR Name = 'RefPhysComboShowID' "
			"OR Name = 'RefPhysComboShowUPIN' "
			"OR Name = 'UBFormType' "
			"OR Name = 'CredentialWarnBills' "
			"OR Name = 'ExpLicenseWarnBills' "
			"OR Name = 'CredentialWarningOnlyTOS2' "
			"OR Name = 'WarnNoICD9OnBill' "
			"OR Name = 'WarnNoApptOnBillDate' "
			"OR Name = 'LinkCPTDiagnosisCodes' " // (r.gonet 02/20/2014) - PLID 60778 - Renamed the property to remove the reference to ICD9
			"OR Name = 'LinkSelectedCPTDiagnosisCodes' " // (r.gonet 02/20/2014) - PLID 60778 - Renamed the property to remove the reference to ICD9
			"OR Name = 'AllowDynamicCPTICDLinking' " // (j.jones 2012-12-12 14:39) - PLID 47773
			"OR Name = 'CopyDiagnosisCodes' "
			"OR Name = 'GlobalPeriodToDos' "
			"OR Name = 'RewardCalcDiscountInPoints' "
			"OR Name = 'ConvertProspectOnBill' "
			"OR Name = 'ReplaceBillDescriptionByResp' " // (j.gruber 2012-01-04 14:31) - PLID 46291
			"OR Name = 'AddNoteOnCallValues' " // (d.singleton 2012-03-30 14:34) - PLID 48151
			"OR Name = 'DefaultNoteCatID' "	// (j.jones 2012-04-12 09:45) - PLID 49609
			"OR Name = 'BillEMNWithLocation' "	// (j.jones 2012-04-30 14:56) - PLID 40121
			"OR Name = 'WarnNoNDCOnNOCCode' "	// (j.dinatale 2012-06-25 11:00) - PLID 51138
			"OR Name = 'GlobalPeriod_OnlySurgicalCodes' "	// (j.jones 2012-07-24 17:04) - PLID 51651
			"OR Name = 'GlobalPeriod_WarnOfficeVisits' "	// (j.jones 2012-07-25 14:08) - PLID 50487
			"OR Name = 'GlobalPeriodSort' "
			"OR Name = 'GlobalPeriod_IgnoreModifier78' "	// (j.jones 2012-07-26 09:53) - PLID 50489
			"OR Name = 'GlobalPeriod_WarnSurgicalCodes' "	// (j.jones 2012-12-06 11:31) - PLID 52637
			"OR Name = 'Claim_SendPatAddressWhenPOS12' " // (j.jones 2013-04-24 17:09) - PLID 56453
			"OR Name = 'EnableBillInvoiceNumbers' " // (j.jones 2013-07-10 14:50) - PLID 57148
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("BillingModuleDlgText", propText,
		"(Username = '<None>' OR Username = '%s') AND ("
		" Name = 'PatientRespBillDesc' " // (j.gruber 2012-01-04 14:31) - PLID 46291
		"OR Name = 'CodeCorrectURL' " // (d.singleton 2012-04-27 16:25) - PLID 49924
		"OR Name = 'CodeCorrectURI' "
		")",
		_Q(GetCurrentUserName()));

		{
			// (a.walling 2009-02-23 10:42) - PLID 11034 - Get the cached size, and resize. The width and height are smushed into a single DWORD.
			DWORD dwSize = MAXDWORD;

			// (j.armen 2014-08-11 17:52) - PLID 63330 - Set the minimum size of the dialog
			long minSizeX;
			if (m_EntryType == BillEntryType::Bill) {
				SetMinSize(minSizeX = g_cnBillingModuleMinBillSizeX, g_cnBillingModuleMinSizeY);
				dwSize = GetRemotePropertyInt("BillingModuleSize_Bill", MAXDWORD, 0, GetCurrentUserName(), true);
			}
			else if (m_EntryType == BillEntryType::Quote) {
				SetMinSize(minSizeX = g_cnBillingModuleMinQuoteSizeX, g_cnBillingModuleMinSizeY);
				dwSize = GetRemotePropertyInt("BillingModuleSize_Quote", MAXDWORD, 0, GetCurrentUserName(), true);
			}

			bool bDidSetSize = false;

			if (dwSize != MAXDWORD) {
				m_dwSize = dwSize;
				CRect rcScreen;		
				GetWindowRect(rcScreen);

				int cx = max(LOWORD(dwSize), minSizeX);
				int cy = max(HIWORD(dwSize), g_cnBillingModuleMinSizeY);
				int x = rcScreen.left - ( (cx - rcScreen.Width()) / 2 );
				int y = rcScreen.top - ( (cy - rcScreen.Height()) / 2 );

				CRect rcDesktop;
				GetDesktopWindow()->GetWindowRect(rcDesktop);
				// (a.walling 2009-08-25 15:00) - PLID 35339 - Don't restore ths size if it exceeds the desktop
				if (cx < rcDesktop.Width() && cy < rcDesktop.Height()) {
					SetWindowPos(NULL, x, y, cx, cy, SWP_NOZORDER);
					bDidSetSize = true;
				}
			}
		
			// (j.armen 2014-08-11 17:52) - PLID 63330 - If this is the first time opening a bill, calculate,
			//	or we could not set the window position, open to to the default
			if (!bDidSetSize && m_EntryType == BillEntryType::Bill)
			{
				// Get our desktop rect
				CRect rcDesktop;
				GetDesktopWindow()->GetWindowRect(rcDesktop);

				// Calculate deflation amount to our recommended size (1024 x 549)
				long nDeflateX = max(0, -1 * (1024 - rcDesktop.Width()) / 2);
				long nDeflateY = max(0, -1 * (g_cnBillingModuleMinSizeY - rcDesktop.Height()) / 2);
				rcDesktop.DeflateRect(nDeflateX, nDeflateY);

				// Set the window position
				SetWindowPos(NULL, rcDesktop.left, rcDesktop.top, rcDesktop.Width(), rcDesktop.Height(), SWP_NOZORDER);
			}
		}

		//Attach to the NxTab - probably should have a global function
		CWnd *pWnd = GetDlgItem(IDC_TAB);
		if (pWnd)
			m_tab = pWnd->GetControlUnknown();
		else m_tab = NULL;

		if (m_tab == NULL)
		{
			HandleException(NULL, "Could not create tab control", __LINE__, __FILE__);
			return FALSE;
		}

		m_tab->TabWidth = 3;

		if (m_EntryType == 2) {
			//if a quote
			m_tab->Size = 2;
			m_tab->Label[0] = "&Quote";
			m_tab->Label[1] = "&Additional Info.";
		}
		else {
			//if a bill
			m_tab->Size = 3;
			m_tab->Label[0] = "&Billing";
			m_tab->Label[1] = "&Insurance";
			m_tab->Label[2] = "&Additional Info.";
		}	

		m_CoordCombo = BindNxDataListCtrl(IDC_COORDINATOR_COMBO);
	
		NXDATALISTLib::IRowSettingsPtr pRow = m_CoordCombo->GetRow(-1);
		_variant_t var = (long)-1;
		pRow->PutValue(0,var);
		pRow->PutValue(1,"<No Patient Coordinator>");
		m_CoordCombo->InsertRow(pRow,0);

		COleDateTime dt = COleDateTime::GetCurrentTime();

		if(m_bUseDefaultDate && m_dtDefaultDate.GetStatus() != COleDateTime::invalid)
			dt = m_dtDefaultDate;

		m_date.SetValue(COleVariant(dt));
		m_dtCurrentDate = dt;

		m_nCurCoordinatorID = -1;

		m_editDescription.SetLimitText(255);
		m_editQuoteNotes.SetLimitText(255);

		//DRT 4/15/2008 - PLID 29663 - Remove this status until we review the dialog entirely for layout and ability to position icons
		//m_saveButton.AutoSet(NXB_OK);

		if (m_EntryType == 2) {

			//set up the Quote Custom Combo
			m_pQuoteCustomList = BindNxDataListCtrl(IDC_QUOTE_CUSTOM_REPORT_LIST);

			pRow = m_pQuoteCustomList->GetRow(-1);
			pRow->PutValue(qclcID, (long)227);
			pRow->PutValue(qclcNumber, (long)-1);
			pRow->PutValue(qclcFileName, _variant_t(""));
			pRow->PutValue(qclcTitle, _variant_t("<System Report>"));
			pRow->PutValue(qclcRepNum, _variant_t("227--1"));
			m_pQuoteCustomList->InsertRow(pRow, -1);

			pRow = m_pQuoteCustomList->GetRow(-1);
			pRow->PutValue(qclcID, (long)608);
			pRow->PutValue(qclcNumber, (long)-1);
			pRow->PutValue(qclcFileName, _variant_t(""));
			pRow->PutValue(qclcTitle, _variant_t("<System Package Quote>"));
			pRow->PutValue(qclcRepNum, _variant_t("608--1"));
			m_pQuoteCustomList->InsertRow(pRow, -1);
		}

		//DRT 5/5/2004 - PLID 12207 - We want to receive barcode messages!
		// (a.walling 2007-12-20 17:39) - PLID 28252 - We want barcode messages as a bill and as a quote
		if(/*m_EntryType == 2 &&*/ GetMainFrame())
			GetMainFrame()->RegisterForBarcodeScan(this);

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

long CBillingModuleDlg::GetBillID()
{
	return m_iBillID;
}

void CBillingModuleDlg::SetBillID(long BillID)
{
	m_iBillID = BillID;

	// (j.jones 2013-07-10 16:13) - PLID 57148 - if the bill ID is set, enable
	// the invoice number field on the additional info tab
	if (m_dlgBilling2.GetSafeHwnd() && BillID != -1) {
		m_dlgBilling2.TryEnableInvoiceNumber();
	}
}


// Add charge to modified list.
//
// ASSUME THE LIST DID NOT MOVE FROM varBoundValue!!!!
//
// (j.jones 2008-06-13 13:52) - PLID 28782 - renamed the parameter here
void CBillingModuleDlg::AddToModifiedList(long nChargeID)
{
	if (m_dlgBilling.GetSafeHwnd() != NULL) {	
		m_dlgBilling.AddToModifiedList(nChargeID);
	}
}

void CBillingModuleDlg::OnBtnEditText() 
{
	//DRT 2/12/03 - Made this button show the "Applied Superbills" dialog now on the bill
	if(m_EntryType == 1) {	//bill

		//permissions
		if (!CheckCurrentUserPermissions(bioApplySuperbills, sptWrite))
			return;

		//attempt to save the dialog first, so that everything is entered in here
		if (m_dlgBilling.m_pList->GetRowCount() == 0) {
			MsgBox("The bill must have at least one charge before a bill can be saved.");
			return;
		}

		BeginWaitCursor();

		//actually save everything on that dialog
		if (m_dlgBilling.SaveChanges() != SCR_SAVED) {
			EndWaitCursor();
			return;
		}

		//Show all the applied Superbills for this dialog
		CAppliedSuperbillsDlg dlg(this);
		dlg.m_nCurrentPatientID = m_nPatientID;
		dlg.m_nCurrentBillID = m_iBillID;
		dlg.DoModal();
	}
	else {
		//Quote
		CQuoteNotes dlg(this);

		// Set the existing ExtraDesc string
		dlg.m_strSpecificNotes = m_strExtraDesc;

		if (IDOK == dlg.DoModal()) {
			// Go here if the user wants to save changes
			m_strExtraDesc = dlg.m_strSpecificNotes;
		}
	}
}

void CBillingModuleDlg::OnCancel() 
{
	/* TRUE if this is a new bill, and it was created in the
	database to view a HCFA or report. In this case, we must
	delete the bill. */
	if (m_boIsNewBill == TRUE && m_iBillID >= 0) {

		CString strWarn = "This bill has already been saved. Clicking 'Cancel' now will delete this bill.\n"
			"Are you sure you wish to continue and delete this bill?";

		if(m_EntryType == 2)
			strWarn.Replace("bill","quote");

		if(IDNO == MessageBox(strWarn,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		DeleteBill(m_iBillID);
	}
	
	// (r.farnworth 2014-12-10 14:45) - PLID 64081 - If you cancel a bill from a HL7 charge message (DFT) that was commited, Nextech considers the message as imported.
	if (m_nMessageQueueID != -1) {
		ExecuteParamSql("UPDATE HL7MessageQueueT SET ActionID = NULL WHERE ID = {INT}", m_nMessageQueueID);
		
		// (z.manning 2016-04-19 10:41) - NX-100240 - Audit that was are marking the message pending
		AuditEvent(m_nPatientID, GetBillPatientName(), BeginNewAuditEvent(), aeiHL7MessageProcessed, m_nMessageQueueID
			, "Committed", "Pending (Bill was canceled)", aepMedium, aetChanged);
	}

	CloseWindow();

	// (j.dinatale 2012-01-20 16:00) - PLID 47690 - post a message that we got cancelled
	if(m_pFinancialDlg && m_pFinancialDlg->GetSafeHwnd() && IsWindow(m_pFinancialDlg->GetSafeHwnd())) {
		m_pFinancialDlg->PostMessage(NXM_POST_CANCEL_BILL, NULL, NULL);
	}
}

BOOL m_boSetListFocus = FALSE;

BOOL CBillingModuleDlg::PreTranslateMessage(MSG* pMsg) 
{
	BOOL IsShiftKeyDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);

	COleVariant var;
	short tab = 0;

	if (pMsg->message == WM_SYSKEYDOWN) {
		switch (pMsg->wParam) {
			// For billing screen
			// (d.singleton 2014-02-26 15:04) - PLID 60974 - since new diag controls never have focus 1 - 4 will just go to the diag search
			case '1': case '2': case '3': case '4':
				if (m_pActiveDlg == &m_dlgBilling && m_EntryType == BillEntryType::Bill)
					m_dlgBilling.SetStartingFocus();
				break;
			case 'L': if (m_pActiveDlg == &m_dlgBilling) m_dlgBilling.GetDlgItem(IDC_WHAT_TO_ADD_COMBO)->SetFocus(); break;
			case 'V': if (m_pActiveDlg == &m_dlgBilling) m_dlgBilling.GetDlgItem(IDC_COMBO_PLACEOFSERVICE)->SetFocus(); break;
			case 'N': 
				if (m_pActiveDlg == &m_dlgBilling) {
					m_dlgBilling.GetDlgItem(IDC_COMBO_DESIGNATION)->SetFocus();
					break;
				}
				else if (m_pActiveDlg == &m_dlgInsuranceBilling) {
					m_dlgInsuranceBilling.OnPrintForm();
					return TRUE;
				}
				break;			
			case 'R':  if (m_pActiveDlg == &m_dlgBilling) m_dlgBilling.SetFocusToBillToCombo(); break;
			case 'S':  if (m_pActiveDlg == &m_dlgBilling) m_dlgBilling.OnBtnEditCodes(); return TRUE;

				// For insurance billing
			case 'F': if (m_pActiveDlg == &m_dlgInsuranceBilling) m_dlgInsuranceBilling.OnOpenForm(); return TRUE;

				// For billing module
			case 'O': OnSave(); return TRUE;
			case 'A': 
				//billing 2 dlg
				SetActiveTab(m_EntryType == 1 ? 2 : 1);
				return TRUE;
				//if (m_pActiveDlg == &m_dlgBilling) m_dlgBilling.SetChargeComboFocus(); break;
			case 'C':
			case 'X': OnCancel(); return TRUE;
			case 'D': OnBillDeleteBtn(); return TRUE;
			case 'E': OnBillEditBtn(); return TRUE;
			case 'P': OnBillPreviewBtn(); return TRUE;
			case 'T':
				if (m_EntryType == 2) {
					OnBtnEditText();
				}
				return TRUE;
			case 'B':
				//billing dlg
				SetActiveTab(0);
				return TRUE;
			case 'I':
				//insurance dlg
				if (m_EntryType == 1) {
					SetActiveTab(1);
				}
				return TRUE;
			case 'Y': OnBtnSaveCopy(); return TRUE;
		}
	}


	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
		if (m_EntryType == 2 && GetFocus() == GetDlgItem(IDC_EDIT_QUOTE_NOTES) && IsShiftKeyDown == FALSE) {
			if (m_pActiveDlg == &m_dlgBilling) {
				m_dlgBilling.SetFocus();
				m_dlgBilling.GetDlgItem(IDC_WHAT_TO_ADD_COMBO)->SetFocus();
				return TRUE;
			}
		}
		else if (m_EntryType == 1 && GetFocus() == GetDlgItem(IDC_EDIT_DESCRIPTION) && IsShiftKeyDown == FALSE) {
			if (m_pActiveDlg == &m_dlgBilling) {
				m_dlgBilling.SetFocus();
				m_dlgBilling.SetStartingFocus();
				return TRUE;
			}
			else if (m_pActiveDlg == &m_dlgInsuranceBilling) {
				m_dlgInsuranceBilling.SetFocus();
				m_dlgInsuranceBilling.GetDlgItem(IDC_FORM_TYPE_COMBO)->SetFocus();
				return TRUE;
			}			
		}
		else if (GetFocus() == &m_date && IsShiftKeyDown) {
			if (m_pActiveDlg == &m_dlgBilling) {
				m_dlgBilling.SetFocus();
				m_dlgBilling.GetDlgItem(IDC_COMBO_LOCATION)->SetFocus();
				return TRUE;
			}
			else if (m_pActiveDlg == &m_dlgInsuranceBilling) {
				m_dlgInsuranceBilling.SetFocus();
				m_dlgInsuranceBilling.m_editBox19.SetFocus();
				return TRUE;
			}
		}
	}
	else if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_TAB) {

	}

	if(pMsg->message == NXM_ADD_PACKAGE) {
		m_dlgBilling.PostMessage(NXM_ADD_PACKAGE, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	if(pMsg->message == NXM_BILL_EMR) {
		m_dlgBilling.PostMessage(NXM_BILL_EMR, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	// (j.dinatale 2012-01-17 10:03) - PLID 47514 - billing partial EMRs
	if(pMsg->message == NXM_BILL_EMR_FOR_INSUREDPARTY) {
		m_dlgBilling.PostMessage(NXM_BILL_EMR_FOR_INSUREDPARTY, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	// (j.jones 2008-06-24 08:40) - PLID 30455 - added ability to bill from the schedule
	if(pMsg->message == NXM_BILL_APPOINTMENT) {
		m_dlgBilling.PostMessage(NXM_BILL_APPOINTMENT, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	// (j.dinatale 2012-01-13 17:36) - PLID 47514 - need to break out the logic for billing an EMR
	if(pMsg->message == NXM_BILL_GLASSES_ORDER) {
		m_dlgBilling.PostMessage(NXM_BILL_GLASSES_ORDER, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	if(pMsg->message == NXM_BILL_CANCEL) {
		OnCancel();
		return TRUE;
	}
	
	return CNxDialog::PreTranslateMessage(pMsg);
}

BOOL CBillingModuleDlg::DestroyWindow() 
{
	m_pActiveDlg = NULL;
	// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
	m_eHasAccess = batNoAccess;
	m_boAskForNewPayment = TRUE;
	m_boShowAvailablePayments = FALSE;
	m_boIsNewBill = FALSE;
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added m_bHasBeenCorrected,
	// which is only TRUE if the bill ID exists in BillCorrectionsT.OriginalBillID
	m_bHasBeenCorrected = FALSE;

	// (a.walling 2009-12-23 09:13) - PLID 7002 - Do not save if we are minimized
	if (!IsIconic()) {
		// (a.walling 2009-02-23 10:43) - PLID 11034 - We are being destroyed, so ensure we set the cached size appropriately.
		CRect rc;
		CRect rcDesktop;
		// (a.walling 2009-08-25 15:00) - PLID 35339 - Only save if this is a sane size
		GetWindowRect(rc);
		GetDesktopWindow()->GetWindowRect(rcDesktop);

		if (rc.Width() < rcDesktop.Width() && rc.Height() < rcDesktop.Height()) {
			DWORD dwSize = MAKELONG(rc.Width(), rc.Height());
			if (dwSize != 0 && dwSize != m_dwSize) {
				if (m_EntryType == 2) {
					try {
						SetRemotePropertyInt("BillingModuleSize_Quote", dwSize, 0, GetCurrentUserName());			
					} NxCatchAll("Error saving quote window size");
				} else {
					try {
						SetRemotePropertyInt("BillingModuleSize_Bill", dwSize, 0, GetCurrentUserName());			
					} NxCatchAll("Error saving bill window size");
				}

				m_dwSize = dwSize;
			}
		}
	}

	return CNxDialog::DestroyWindow();
}

void CBillingModuleDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	// (a.walling 2009-02-23 11:05) - PLID 11034 - Reposition the subdialogs as well
	if (GetDlgItem(IDC_TAB)->GetSafeHwnd() != NULL) {
		CRect rectSheet;

		GetTabRect15(rectSheet);

		if (m_dlgBilling.GetSafeHwnd() && m_dlgBilling.IsWindowVisible())
			m_dlgBilling.MoveWindow(rectSheet);
		if (m_dlgBilling2.GetSafeHwnd() && m_dlgBilling2.IsWindowVisible())
			m_dlgBilling2.MoveWindow(rectSheet);
		if (m_dlgInsuranceBilling.GetSafeHwnd() && m_dlgInsuranceBilling.IsWindowVisible())
			m_dlgInsuranceBilling.MoveWindow(rectSheet);
	}
}

int CBillingModuleDlg::DoModal() 
{
	int result;
	GetMainFrame()->DisableHotKeys();	
	result = CNxDialog::DoModal();
	GetMainFrame()->EnableHotKeys();
	return result;
}

BOOL CBillingModuleDlg::Save()
{	
	CString str, strDate;
	long AuditID=-1;
	_RecordsetPtr rs;
	_variant_t var;

	bool bIsNew = false;
	if(GetBillID() == -1)
		bIsNew = true;

	try {
		// (r.gonet 07/09/2014) - PLID 62834 - Check if the bill contains a mix of on hold and not on hold charges.
		// If so, we'll need to split the bill into two. Warn them first.
		bool bSplitCharges = false;
		if (m_dlgBilling) {
			long nNotOnHoldChargeCount = m_dlgBilling.GetNotOnHoldChargeCount();
			long nOnHoldChargeCount = m_dlgBilling.GetOnHoldChargeCount();
			if (nNotOnHoldChargeCount > 0 && nOnHoldChargeCount > 0) {
				// We're going to be splitting the on hold charges to a new bill
				int nDialogResult = MessageBox("One or more charges on this bill are marked on hold. These charges will be saved into a new bill and put on hold. Are you "
					"sure you wish to continue?\r\n"
					"\r\n"
					"Selecting Yes will continue saving and will create a second bill containing the on hold charges.\r\n"
					"Selecting No will remove the charge holds and abort saving.\r\n"
					"Selecting Cancel will abort saving.",
					"Split On Hold Charges to New Bill", MB_ICONQUESTION | MB_YESNOCANCEL);
				if (nDialogResult == IDYES) {
					bSplitCharges = true;
				} else if (nDialogResult == IDNO) {
					m_dlgBilling.ClearChargeHolds();
					// User will need to click OK again to save the bill.
					return FALSE;
				} else {
					return FALSE;
				}
			}
		}

		// (c.haag 2009-09-03 14:09) - PLID 34781 - Ensure the insurance tab was initialized
		// because there may be system preferences that dictate the default values set within
		// the tab.
		if (m_EntryType == 1) {
			m_dlgInsuranceBilling.EnsureInitialized();
		}

		// (r.gonet 07/02/2014) - PLID 62567 - Ensure the bill description has a prefix if it should
		SetBillDescription(GetBillDescription());

		// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
		strDate = FormatDateTimeForSql((m_date.GetValue()), dtoDate);

		//check for insurance referrals
		if(m_nInsuranceReferralID != -1) {
			long nAuthID = m_nInsuranceReferralID;
			long nPOSID = -1;
			if(m_dlgBilling.m_PlaceOfServiceCombo->CurSel != -1)
				nPOSID = VarLong(m_dlgBilling.m_PlaceOfServiceCombo->GetValue(m_dlgBilling.m_PlaceOfServiceCombo->CurSel,0),-1);
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			if(!ReturnsRecordsParam("SELECT ID FROM InsuranceReferralsT WHERE (LocationID = {INT} OR LocationID Is NULL OR LocationID IN (SELECT ID FROM LocationsT WHERE Active = 0)) AND ID = {INT}", nPOSID, nAuthID)) {
				if(MsgBox(MB_YESNO, "The chosen place of service does not exist in your chosen insurance authorization.  Are you sure you wish to select this place of service?") == IDNO) {
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT LocationID FROM InsuranceReferralsT WHERE ID = {INT}",nAuthID);
					if(!rs->eof) {
						nPOSID = AdoFldLong(rs, "LocationID",-1);
						m_dlgBilling.m_PlaceOfServiceCombo->SetSelByColumn(0,(long)nPOSID);
					}
					rs->Close();
				}
			}
		}

		//DRT 5/13/03 - This function was removed, because it makes no sense (Josh said so)
		// Warn of inconsistent insurance fees of the new charges
		// if the HCFA was opened
//		if (m_dlgInsuranceBilling.m_HasHCFABeenOpened == TRUE)
//			m_dlgBilling.WarnDifferentMultiFees();

		// Save all charge details for a bill
		if(m_dlgBilling.SaveChanges() == SCR_ABORT_SAVE)
			return FALSE;

		// This will only happen if there is a zero bill total.
		if (m_iBillID == -1) {
			CloseWindow();
			return FALSE;
		}

		// (j.jones 2011-01-21 10:32) - PLID 42156 - be mindful of the access level
		GetDlgItem(IDC_BILL_DELETE_BTN)->EnableWindow(m_eHasAccess == batFullAccess);
		// (s.tullis 2016-03-08 14:45) - PLID 68319 
		long nClaimForm = -1;
		
		// Save insurance billing information for a bill
		if (m_dlgInsuranceBilling.GetSafeHwnd()) {

			long nFormType = 1;
			if(m_dlgInsuranceBilling.m_FormTypeCombo->GetCurSel() != -1) {
				nClaimForm = VarLong(m_dlgInsuranceBilling.m_FormTypeCombo->GetValue(m_dlgInsuranceBilling.m_FormTypeCombo->GetCurSel(),0), 1);
			}

			if(m_dlgInsuranceBilling.SaveChanges() == SCR_ABORT_SAVE)
				return FALSE;
		}

		if (nClaimForm == -1 && m_dlgBilling2.GetSafeHwnd() && m_dlgInsuranceBilling.GetSafeHwnd()) {
			nClaimForm = GetFormTypeFromLocationInsuranceSetup(m_dlgInsuranceBilling.m_GuarantorID1, m_dlgBilling.m_nCurLocationID);
		}

		// Save billing 2 information
		if (m_dlgBilling2.GetSafeHwnd()) {
			if(m_dlgBilling2.SaveChanges() == SCR_ABORT_SAVE)
				return FALSE;
		}

		// (j.dinatale 2012-03-20 12:14) - PLID 48893 - need to update glassesordersT accordingly if new glasses orders were added
		// (j.dinatale 2012-04-16 11:30) - PLID 49690 - this needs to happen out here now, we cant allow the BillId to be saved to GlassesOrderT if the bill fails to save.
		foreach(long nGlassesOrderID, m_dlgBilling.m_setBilledOpticalOrderIDs) {
			ExecuteParamSql("UPDATE GlassesOrderT SET BillID = {INT} WHERE ID = {INT}", GetBillID(), nGlassesOrderID);
		}

		if(m_strSelectedAuthNum.GetLength() > 0) {
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			ExecuteParamSql("UPDATE BillsT SET PriorAuthNum = {STRING} WHERE ID = {INT}",m_strSelectedAuthNum,m_iBillID);
		}

		//check to see which batch this should be sent to.
		//if they have gone to the insurance tab, the radio button will have been set automatically
		//or they will have chosen the batch. In either case, BatchBill will have already been called
		//and m_bBatched will be TRUE.
		//if it is not batched, we need to ask them if they wish to batch it only if there are insurance
		//charges. If there are insurance charges, m_bInsurance will be TRUE.		
		// (j.gruber 2009-07-10 17:19) - PLID 34724 - take out m_bBatched and add m_bManuallyUnbatched
		if(bIsNew && m_EntryType == 1 && !m_bManuallyUnbatched && m_bInsurance) {
			//get the default batch for the primary insurance
			//TODO: we will need to eventually determine if we have only secondary charges, and batch
			//according to the secondary company
			CString claim = "HCFA";
			int batch = 0;
			if(nClaimForm == 1) {
				claim = "HCFA";
				batch = FindDefaultHCFABatch(m_dlgBilling.m_GuarantorID1);
			}
			else if(nClaimForm == 2) {
				//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
				claim = "UB";
				batch = FindDefaultUB92Batch(m_dlgBilling.m_GuarantorID1);
			}

			COleCurrency cyCharges = COleCurrency(0,0);
			GetBillTotals(GetBillID(), m_nPatientID, &cyCharges, 0,0,0,0);

			//if there is a default batch type AND we have not already batched this bill AND the bill is > $0.00
			// (j.jones 2008-02-12 12:49) - PLID 28847 - don't prompt if the claim can't be created anyways
			// (j.gruber 2009-07-06 09:41) - PLID 34724 - take out the prompt and just batch it
			// (j.jones 2010-10-21 14:57) - PLID 41051 - CanCreateInsuranceClaim changed to issue its own warnings,
			// but they are silent during this call
			if(batch != 0 && cyCharges > COleCurrency(0,0) && FindHCFABatch(m_iBillID) == 0 && CanCreateInsuranceClaim(m_iBillID, TRUE)) {
				BatchBill(m_iBillID,batch,TRUE);
			}
		}		

		/* Save the bill's description and date, and if a quote,
		also save quote notes */
	
		CString strNotes, strQuoteDesc;

		m_editDescription.GetWindowText(strNotes);
		if (strNotes.GetLength() == 0)
			strNotes = "(No description)";
		if (m_EntryType == 2) {
			GetDlgItem(IDC_EDIT_QUOTE_NOTES)->GetWindowText(strQuoteDesc);
		}
		else
			strQuoteDesc = "";

		try {
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			rs = CreateParamRecordset("SELECT Description, Date, Note, ExtraDesc, EntryType FROM BillsT WHERE ID = {INT}",m_iBillID);
			if(!rs->eof) {
				var = rs->Fields->Item["Description"]->Value;
				if(var.vt != VT_NULL && CString(var.bstrVal) != strNotes) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();

					if (AdoFldByte(rs, "EntryType") == 2)	//quote
						AuditEvent(m_nPatientID, GetBillPatientName(), AuditID, aeiQuoteDescription, m_iBillID, CString(var.bstrVal), strNotes, aepLow);
					else
						AuditEvent(m_nPatientID, GetBillPatientName(),AuditID,aeiBillDescription,m_iBillID,CString(var.bstrVal),strNotes,aepLow);
				}
				
				var = rs->Fields->Item["Note"]->Value;
				if(var.vt != VT_NULL && CString(var.bstrVal) != strQuoteDesc) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();

					if (AdoFldByte(rs, "EntryType") == 2)	//quote
						AuditEvent(m_nPatientID, GetBillPatientName(),AuditID,aeiQuoteNote,m_iBillID,CString(var.bstrVal),strQuoteDesc,aepLow);
					else
						AuditEvent(m_nPatientID, GetBillPatientName(),AuditID,aeiBillNote,m_iBillID,CString(var.bstrVal),strQuoteDesc,aepLow);
				}

				var = rs->Fields->Item["ExtraDesc"]->Value;
				if(var.vt != VT_NULL && CString(var.bstrVal) != m_strExtraDesc) {
					if(AuditID==-1)
						AuditID = BeginNewAuditEvent();

					if (AdoFldByte(rs, "EntryType") == 2)	//quote
						AuditEvent(m_nPatientID, GetBillPatientName(),AuditID,aeiQuoteExtraDesc,m_iBillID,CString(var.bstrVal),m_strExtraDesc,aepLow);
					else
						AuditEvent(m_nPatientID, GetBillPatientName(),AuditID,aeiBillExtraDesc,m_iBillID,CString(var.bstrVal),m_strExtraDesc,aepLow);
				}
			}
			rs->Close();
		}NxCatchAll("Error in updating Audit Trail.");

		if(strNotes.GetLength()>255)
			strNotes = strNotes.Left(255);
		if(strQuoteDesc.GetLength()>255)
			strQuoteDesc = strQuoteDesc.Left(255);
		if(m_strExtraDesc.GetLength()>255)
			m_strExtraDesc = m_strExtraDesc.Left(255);

		CString PatCoord = "NULL";
		if(m_CoordCombo->GetCurSel() != -1) {
			_variant_t varTmp = m_CoordCombo->GetValue(m_CoordCombo->GetCurSel(),0);
			if(varTmp.vt == VT_I4 && varTmp.lVal > 0)
				PatCoord.Format("%li",varTmp.lVal);
		}
		else if(m_CoordCombo->IsComboBoxTextInUse) {
			//something is set on the combo text, but there is no selection - this is an inactive user, so save the old one
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rsCoord = CreateParamRecordset("SELECT PatCoord FROM BillsT WHERE ID = {INT} AND PatCoord IS NOT NULL", GetBillID());
			if(!rsCoord->eof) {
				PatCoord.Format("%li", AdoFldLong(rsCoord, "PatCoord"));
			}
			else {
				//there is something selected currently, but nothing is in the data ... maybe somebody else modified the data?
				ASSERT(FALSE);	//shouldn't happen
				//we'll just update the data to nothing, since we're in a state of confusion
				PatCoord.Format("NULL");
			}
		}

		CString strInsuranceReferralID = "NULL";
		if(m_nInsuranceReferralID != -1)
			strInsuranceReferralID.Format("%li",m_nInsuranceReferralID);

		//****REMEMBER: All new fields also need to be supported in FinancialCorrection.cpp
		//and in SplitChargesIntoNewBill() in GlobalFinancialUtils.****//
		str.Format("UPDATE BillsT SET Description = '%s', [Date] = '%s', Note = '%s', ExtraDesc = '%s', PatCoord = %s, InsuranceReferralID = %s WHERE ID = %d",
			_Q(strNotes), strDate, _Q(strQuoteDesc), _Q(m_strExtraDesc), PatCoord, strInsuranceReferralID, m_iBillID);
		ExecuteSql("%s",str);  //will account for % in the description


		//DRT 7/25/03 - Do some scheduler updating depending on prefs
		if(bIsNew)
			AttemptUpdateScheduler();

		//whether we use this or not is checked when we make a new bill/quote, 
		//but we always need to update the last bill/quote dates
		// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
		if(m_EntryType == 1)
			// (j.jones 2007-05-07 09:46) - PLID 25906 - use the GlobalUtils function
			AddLastBillDate(m_iBillID, (m_date.GetValue()));
		else
			// (j.jones 2007-05-07 09:46) - PLID 25906 - use the GlobalUtils function
			AddLastQuoteDate(m_iBillID, (m_date.GetValue()));

		// (r.gonet 07/09/2014) - PLID 62834 - All other saving is done. Split the bill if needed.
		if (bSplitCharges) {
			SplitOnHoldChargesIntoNewBill();
		}
	}
	NxCatchAll("Error in BillingModuleDlg::OnClickBtnSave");

	return TRUE;
}

// (r.gonet 07/09/2014) - PLID 62834 - Creates a second bill with the on hold charges from this current bill
// and removes the on hold charges from the current bill. All dependencies are copied.
void CBillingModuleDlg::SplitOnHoldChargesIntoNewBill()
{
	// We need to get a list of all the charge ids for the charges that are marked on hold.
	// Since we don't save the charge's on hold status to the database and don't store the Ids
	// back in the BillingItem list, we have to map up the saved charges in the database to the
	// on hold billing items.

	// Get the billing items that are on hold
	std::vector<BillingItemPtr> vecOnHoldCharges;
	if (m_dlgBilling) {
		m_dlgBilling.GetOnHoldCharges(vecOnHoldCharges);
	} else {
		return;
	}

	// We can map the billing items to the saved charges using the line id
	std::map<long, BillingItemPtr> mapLineIDToBillingItem;
	for each(BillingItemPtr pItem in vecOnHoldCharges)
	{
		long nLineID = VarLong(pItem->LineID);
		mapLineIDToBillingItem[nLineID] = pItem;
	}

	// Get the saved charges for this bill
	CArray<long, long> aryChargeIDsToSplit, aryChargeIDsToNotSplit;
	_RecordsetPtr prsCharges = CreateParamRecordset(
		"SELECT ChargesT.ID, ChargesT.LineID "
		"FROM ChargesT "
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"WHERE ChargesT.BillID = {INT} "
		"AND LineItemT.Deleted = 0 "
		, m_iBillID);
	while (!prsCharges->eof) {
		long nChargeID = AdoFldLong(prsCharges->Fields, "ID");
		long nLineID = AdoFldLong(prsCharges->Fields, "LineID");
		std::map<long, BillingItemPtr>::iterator iter = mapLineIDToBillingItem.find(nLineID);
		if (iter != mapLineIDToBillingItem.end()) {
			// This is one of the on hold charges. We want to split this charge off.
			aryChargeIDsToSplit.Add(nChargeID);
		} else {
			aryChargeIDsToNotSplit.Add(nChargeID);
		}

		prsCharges->MoveNext();
	}
	prsCharges->Close();

	// Make sure that after we do the split, we don't wind up with a bill that has no charges.
	if (aryChargeIDsToSplit.GetSize() > 0 && aryChargeIDsToNotSplit.GetSize() > 0) {
		long nSplitBillID = SplitChargesIntoNewBill(m_iBillID, aryChargeIDsToSplit);
		if (nSplitBillID == -1) {
			ThrowNxException("%s : Failed to split the bill.", __FUNCTION__);
		}
		// If all of the charges on the bill are marked 'On Hold' save all of the charges in the same bill. The new bill should save with the built-in 'On Hold' status.
		if (!::SetBillOnHold(nSplitBillID, TRUE, TRUE)) {
			ThrowNxException("%s : Failed to set the bill to On Hold.", __FUNCTION__);
		}
		RefreshBillInFinancialDialog(nSplitBillID);
	}

	// Reload the current bill to clean up the list. It may be that the bill is being saved but not closing, in which case we should not see the on hold charges any longer.
	m_dlgBilling.ReloadCurrentBill();
}

// (r.gonet 07/09/2014) - PLID 62834 - Update the bill in the financial dialog.
// - nBillID: The bill to refresh in the financial dialog
// - msg: The message to send to the financial dialog. Defaults to bill edited.
void CBillingModuleDlg::RefreshBillInFinancialDialog(long nBillID, UINT msg/*= NXM_POST_EDIT_BILL*/)
{
	// Check for a valid window to post a message back to so it can show the split bill
	if (m_pFinancialDlg && m_pFinancialDlg->GetSafeHwnd() && IsWindow(m_pFinancialDlg->GetSafeHwnd())) {
		m_pFinancialDlg->PostMessage(msg, nBillID, 1);

		// If this pointer is not the billing tab, update the current view, as the billing tab may be in the background
		if (!m_pFinancialDlg->IsKindOf(RUNTIME_CLASS(CFinancialDlg))) {
			CNxTabView* pView = GetMainFrame()->GetActiveView();
			if (pView) {
				pView->UpdateView();
			}
		}
	} else {
		// Update the current view, as the billing tab may be in the background
		CNxTabView* pView = GetMainFrame()->GetActiveView();
		if (pView) {
			pView->UpdateView();
		}
	}
}

void CBillingModuleDlg::CloseWindow()
{
	// (a.walling 2009-12-23 09:28) - PLID 7002
	m_bVisibleState = false;

	//reinitialize variables
	m_pActiveDlg = NULL;
	// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
	m_eHasAccess = batNoAccess;
	m_bHasEverHadAccess = FALSE;
	m_boAskForNewPayment = TRUE;
	m_boShowAvailablePayments = FALSE;
	m_boIsNewBill = FALSE;
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added m_bHasBeenCorrected,
	// which is only TRUE if the bill ID exists in BillCorrectionsT.OriginalBillID
	m_bHasBeenCorrected = FALSE;
	m_nInsuranceReferralID = -1;
	m_eCreatedFrom = BillFromType::Other;  // (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum, set back to default which is Other
	m_strSelectedAuthNum = "";
	m_bPromptForReferral = TRUE;
	m_dtCurrentDate = COleDateTime::GetCurrentTime();
	m_dtDefaultDate = COleDateTime::GetCurrentTime();
	m_nCurCoordinatorID = -1;
	m_bUseDefaultDate = FALSE;
	m_date.SetValue(COleVariant(COleDateTime::GetCurrentTime()));
	m_editDescription.SetWindowText("");
	m_strExtraDesc = "";
	m_CoordCombo->CurSel = -1;
	m_tab->CurSel = 0;
	// (z.manning 2016-04-19 10:43) - NX-100240 - Clear the HL7 message ID
	m_nMessageQueueID = -1;

	//JJ - NEVER CALL THIS HERE!
	//CAH - OK!
	//SetActiveTab(eTabBill);

	if(m_dlgBilling) {
		m_dlgBilling.ToggleChargeBatchColumn(FALSE);
		m_dlgBilling.TrySaveListColumnWidths();
		m_dlgBilling.m_EntryType = BillEntryType::Bill;
		m_dlgBilling.m_boChangesMade = FALSE;
		m_dlgBilling.m_boInitialized = FALSE;
		m_dlgBilling.m_boSetListFocus = FALSE;
		// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
		m_dlgBilling.m_eHasAccess = batNoAccess;
		m_dlgBilling.m_nCurLocationID = -1;
		m_dlgBilling.m_nCurPlaceOfServiceID = -1;

		/*// (j.gruber 2009-03-10 16:39) - PLID 33444 - this seems to have never worked
		// (a.walling 2014-02-24 11:29) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems.

		for(int i=m_billingItems.GetSize()-1;i>=0;i--) {
			//clear the normal billing array
			delete m_billingItems.GetAt(i);
		}
		m_billingItems.RemoveAll();
		*/
		m_dlgBilling.DeleteAllFromChargedProductItemsArray();
		// (j.jones 2007-11-14 12:50) - PLID 27988 - free tracked allocations in the bill
		m_dlgBilling.FreeTrackedAllocations();
		m_dlgBilling.DeleteAllFromChargedAllocationDetailsArray();
		m_dlgBilling.m_List->Clear();
		// (j.jones 2009-03-24 14:33) - PLID 9729 - clear the more diag codes array
		m_dlgBilling.ClearDiagCodesList();
		// (j.jones 2009-03-25 10:02) - PLID 33653 - also clear the "old" array
		// (b.spivey, February 26, 2014) - PLID 60975 - Updated the function name. 
		m_dlgBilling.ClearOldDiagCodesList();
		// (j.jones 2009-08-11 17:10) - PLID 35142 - clear the case history information
		// (j.jones 2009-08-12 15:32) - PLID 35179 - these are now arrays of pointers
		m_dlgBilling.ClearCaseHistoryArray();
		m_dlgBilling.ClearOldCaseHistoryArray();
	}

	if(m_dlgInsuranceBilling) {
		m_dlgInsuranceBilling.m_boInitialized = FALSE;
		// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
		m_dlgInsuranceBilling.m_eHasAccess = batNoAccess;
		m_dlgInsuranceBilling.m_HasHCFABeenOpened = FALSE;
		m_dlgInsuranceBilling.m_nPendingReferringPhyID = -1;
		m_dlgInsuranceBilling.m_GuarantorID1 = -1;
		m_dlgInsuranceBilling.m_GuarantorID2 = -1;
	}

	if(m_dlgBilling2) {
		m_dlgBilling2.m_boInitialized = FALSE;
		// (j.jones 2011-01-21 10:10) - PLID 42156 - this is now an enum
		m_dlgBilling2.m_eHasAccess = batNoAccess;
		m_dlgBilling2.ClearData();
	}

	if(m_EntryType==2) {
		//JJ - we can't reset these because the ID is needed when the quote is called from the tracking dialog
		//if we make the Quote modeless, this will have to post a message to the tracking dialog
		//m_iBillID = -1;
		//m_EntryType=-1;
		if(!m_bIsPopupAllowed) {
			GetMainFrame()->AllowPopup();
			m_bIsPopupAllowed = FALSE;
		}

		CDialog::OnOK();

		// (j.jones 2011-07-08 13:49) - PLID 26785 - added ability to auto-bill new packages
		if(m_dlgBilling.m_bSavedNewPackage) {
			long nAutoBillPackage = GetRemotePropertyInt("AutoBillPackage", 1, 0, "<None>", true);
			//0 - do not bill, 1 - prompt, 2 - auto-bill
			if(nAutoBillPackage == 2 || (nAutoBillPackage == 1 &&
				MessageBox("Would you like to bill this package now?", "Practice", MB_ICONQUESTION|MB_YESNO) == IDYES)) {

				//switch to this patient's billing tab
				CMainFrame *pMainFrame;
				pMainFrame = GetMainFrame();
				if(pMainFrame != NULL) {
					
					if(pMainFrame->m_patToolBar.TrySetActivePatientID(m_nPatientID)) {

						//Now just flip to the patient's module and set the active Patient
						pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

						CNxTabView *pView = pMainFrame->GetActiveView();
						if(pView) {
							pView->SetActiveTab(PatientsModule::BillingTab);
							pView->UpdateView();

							((CFinancialDlg*)pView->GetActiveSheet())->PostMessage(NXM_BILL_PACKAGE, m_iBillID, 0);
						}
						else {
							AfxMessageBox("The patient's billing tab could not be loaded. The package will not be billed.");
						}
					}
					else {
						AfxMessageBox("The patient's account could not be loaded. The package will not be billed.");
					}
				}
				else {
					AfxMessageBox("The patient's account could not be loaded. The package will not be billed.");
				}
			}
		}

		m_dlgBilling.m_bSavedNewPackage = FALSE;

		return;
	}
	else {
		if(m_dlgBilling)
			m_dlgBilling.ShowWindow(SW_HIDE);
		if(m_dlgInsuranceBilling)
			m_dlgInsuranceBilling.ShowWindow(SW_HIDE);
		if(m_dlgBilling2)
			m_dlgBilling2.ShowWindow(SW_HIDE);
		// (a.walling 2009-12-22 17:08) - PLID 7002 - We are now modeless, so we don't need this anymore
		// (j.jones 2012-10-30 16:33) - PLID 53444 - we still need to toggle hotkeys, because the bill
		// has several hotkeys that are also in use by mainframe, and we can't get rid of them now
		//GetMainFrame()->EnableWindow(TRUE);
		GetMainFrame()->EnableHotKeys();
		ShowWindow(SW_HIDE);
		//TES 11/3/2005 - PLID 11410 - Here's a note from the datalist, which seems relevant.
		// (b.cardillo 2005-02-02 15:44) - PLID 15498 - Well amazingly in some cases the call to ShowWindow(SW_HIDE) actually 
		// fails, and the api offers no way to know (the return value is not an indication of success or  failure, it's just 
		// the an indication of the previous state).  So just in case it fails (which it DOES in  the case this pl item 
		// describes (see pl item notes)), we ALSO call ModifyStyle to ensure the change.  We can't call JUST ModifyStyle() 
		// (without preceding it with ShowWindow(SW_HIDE) because that area of the screen won't be validated.)
		ModifyStyle(WS_VISIBLE, 0, 0);

		if(m_iBillID!=-1) {
			//DRT 7/31/2006 - PLID 21446 - Cleaned up how we check for a valid window
			if(m_pFinancialDlg && m_pFinancialDlg->GetSafeHwnd() && IsWindow(m_pFinancialDlg->GetSafeHwnd())) {
				m_pFinancialDlg->PostMessage(NXM_POST_EDIT_BILL,m_iBillID,m_iSaveType);

				// (j.jones 2011-07-15 12:19) - PLID 43117 - if this pointer is not the billing tab,
				// update the current view, as the billing tab may be in the background
				if(!m_pFinancialDlg->IsKindOf(RUNTIME_CLASS(CFinancialDlg))) {
					CNxTabView* pView = GetMainFrame()->GetActiveView();
					if(pView) {
						pView->UpdateView();
					}
				}
			}
			else {
				// (j.jones 2011-07-15 12:19) - PLID 43117 - update the current view,
				//as the billing tab may be in the background
				CNxTabView* pView = GetMainFrame()->GetActiveView();					
				if(pView) {
					pView->UpdateView();
				}
			}
		}
	}

	// (j.jones 2011-07-08 13:56) - PLID 26785 - clear m_bSavedNewPackage only at the end of this function
	if(m_dlgBilling) {
		m_dlgBilling.m_bSavedNewPackage = FALSE;
	}

	//now reset the bill ID
	m_iBillID = -1;
	m_EntryType = (BillEntryType) -1;

	//reset the patient ID last
	m_nPatientID = -1;
}

LRESULT CBillingModuleDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	if (m_tab->CurSel == 0)
		return m_dlgBilling.SendMessage(WM_BARCODE_SCAN, wParam, lParam);
	return 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CBillingModuleDlg::OnChangeBillDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	COleDateTime dtOld(1930, 1, 1, 1, 1, 1);
	COleDateTime dtInvalid(1800, 1, 1, 1, 1, 1);

	// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
	COleDateTime dt = (m_date.GetValue());
	
	if(dt.m_status==COleDateTime::invalid || dt < dtInvalid) {
		AfxMessageBox("You have entered an invalid date. It will be changed to today's date.");
		dt = COleDateTime::GetCurrentTime();
		m_date.SetValue(_variant_t(dt));
	}

	if (dt > COleDateTime::GetCurrentTime() + COleDateTimeSpan(365*3,0,0,0) ||
		dt < dtOld) {
		CString str;
		str = "This bill is dated " + FormatDateTimeForInterface(dt,NULL,dtoDate) + ", which is prior to 1930 or beyond three years from now. Are you sure you wish to save this information?";
		if (IDNO == MessageBox(str, NULL, MB_YESNO)) {
			dt = COleDateTime::GetCurrentTime();
			m_date.SetValue(_variant_t(dt));
		}
	}

	// (j.jones 2006-04-13 09:08) - warn global periods
	if(m_EntryType == 1) {
		if(GetBillID() == -1) {
			//checks for any active global period for the new date, that we weren't already warned about
			// (a.walling 2008-07-07 18:03) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			if(!CheckWarnGlobalPeriod(m_nPatientID, dt, TRUE, COleDateTime::GetCurrentTime())) {
				dt = COleDateTime::GetCurrentTime();
				m_date.SetValue(_variant_t(dt));
				return;
			}
		}
		else {
			//checks for any active global period for the new date
			// (a.walling 2008-07-07 18:03) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			if(!CheckWarnGlobalPeriod(m_nPatientID, dt)) {
				dt = COleDateTime::GetCurrentTime();
				m_date.SetValue(_variant_t(dt));
				return;
			}
		}
	}

	if (GetBillID() != -1) {

		if (m_dtCurrentDate != dt) {

			// (j.jones 2011-01-25 14:58) - PLID 42156 - check historical permissions,
			// if they have permission to edit the bill and agree to any prompts here,
			// they then also have permission to later apply the new date to all charges
			if(m_EntryType == 1 && !CanChangeHistoricFinancial("Bill", GetBillID(), bioBill, sptWrite)) {
				m_date.SetValue(_variant_t(m_dlgBilling.m_dtOldDate));
				return;
			}

			//give them the message box
			if (!GetRemotePropertyInt("DefaultAssignmentToInputDate", 0, 0, "<None>", true) && m_EntryType == 1 && MsgBox(MB_YESNO, "Changing the date of an existing bill will change all charge responsibilities to begin aging by the new date instead of the old date. \nAre you sure you wish to do this?") == IDNO) {

				//change it back
				m_date.SetValue(_variant_t(m_dlgBilling.m_dtOldDate));
			}
			else {

				//we changed the date, so we have to save it
				m_dlgBilling.m_bUpdateDate = TRUE;

				//tell the billingdlg to save our changes for each charge on the bill
				_variant_t var;
				// (j.jones 2011-10-04 12:03) - PLID 45799 - The charge list is now a DL2.
				// In changing this code, I noticed that this only referenced the bill list,
				// not the quote list, but never checked the EntryType first. I added that
				// for posterity.
				if(m_EntryType == 1) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlgBilling.m_List->GetFirstRow();
					while(pRow) {
						long nChargeID = VarLong(pRow->GetValue(1), -2);
						if(nChargeID == -2) {
							pRow = pRow->GetNextRow();
							continue;
						}

						// (j.jones 2011-08-24 08:41) - PLID 44868 - skip original & void charges
						if(m_dlgBilling.GetSafeHwnd() != NULL) {
							if(m_EntryType == 1 && nChargeID != -2
								&& m_dlgBilling.IsOriginalOrVoidCharge(nChargeID)) {

								pRow = pRow->GetNextRow();
								continue;
							}
						}
						
						AddToModifiedList(nChargeID);

						pRow = pRow->GetNextRow();
					}
				}

				// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
				//set the current date in case they change it in the future
				m_dtCurrentDate = (m_date.GetValue());
			}
		}
	}

	if(m_EntryType == 2) {
		if(m_dlgBilling.m_quoteExpCheck.GetCheck())
			m_dlgBilling.CalcExpireDate();
	}

	// (d.thompson 2009-08-26) - PLID 33953 - When we change the bill date, if the preference is on, change all the charge dates
	//	to match.
	if(GetRemotePropertyInt("ChangeBillDatesPrompt", 0, 0, GetCurrentUserName(), true) == 1) {
		if(AfxMessageBox("You have changed the date of this bill.  Would you like to update all charge service dates to match this "
			"new date?", MB_YESNO) == IDYES)
		{
			//GetValue() returns a date & time.  We just want the date.
			COleDateTime dtSet = m_date.GetDateTime();
			COleDateTime dtFixed(dtSet.GetYear(), dtSet.GetMonth(), dtSet.GetDay(), 0, 0, 0);

			_variant_t varDate(dtFixed, VT_DATE);
			m_dlgBilling.UpdateAllChargesWithNewServiceDate(varDate);
		}
	}
}

void CBillingModuleDlg::OnSave() 
{
	try {
		if(!Save())
			return;

		CloseWindow();
	} NxCatchAll("Error in OnSave");
}


LRESULT CBillingModuleDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
		case NXM_BILLINGMODULE_MSG:
			switch (wParam) {
			case 0:
				ActivateBillingDlg();
				break;
			case 1:
				if(m_EntryType == 1) {
					//if a bill
					ActivateInsuranceDlg();			
				}
				else {
					//if a quote
					ActivateBilling2Dlg();
				}
				break;
			case 2:
				ActivateBilling2Dlg();
				break;
			}
			break;
		default:
			break;
	}	
	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CBillingModuleDlg::OnSelectTab(short newTab, short oldTab) 
{
	PostMessage(NXM_BILLINGMODULE_MSG, newTab);
}

void CBillingModuleDlg::SetActiveTab(short newTab)
{
	short oldTab = m_tab->CurSel;

	if (oldTab == newTab || (newTab < 0 && newTab > 2))
		return;

	m_tab->CurSel = newTab;
	OnSelectTab(newTab, oldTab);
}

void CBillingModuleDlg::OnSelChosenCoordinatorCombo(long nRow) 
{
	try {

		if(nRow != -1) {
			_variant_t varTmp = m_CoordCombo->GetValue(m_CoordCombo->GetCurSel(),0);
			if(varTmp.vt == VT_I4 && varTmp.lVal == -1) {
				nRow = -1;
				m_CoordCombo->CurSel = -1;
			}
		}

		_variant_t varCoordID;
		varCoordID.vt = VT_NULL;
		if(nRow != -1) {
			_variant_t varTmp = m_CoordCombo->GetValue(m_CoordCombo->GetCurSel(),0);
			if(varTmp.vt == VT_I4 && varTmp.lVal > 0)
				varCoordID = varTmp;
		}

		if(m_dlgBilling.m_EntryType == 1 && m_nCurCoordinatorID != VarLong(varCoordID, -1)) {

			// (j.jones 2011-01-25 14:58) - PLID 42156 - check historical permissions,
			// if they have permission to edit the bill and agree to any prompts here,
			// they then also have permission to later apply the new coordinator to all charges
			if(m_EntryType == 1 && GetBillID() != -1
				&& !CanChangeHistoricFinancial("Bill", GetBillID(), bioBill, sptWrite)) {

				// Patient Coordinator - load from existing Bill
				if(m_CoordCombo->TrySetSelByColumn(0, (long)m_nCurCoordinatorID) == -1) {
					//might have an inactive coord
					_RecordsetPtr rsCoord = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nCurCoordinatorID);
					if(!rsCoord->eof) {
						m_CoordCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCoord, "Name", "")));
					}
					else {
						m_CoordCombo->PutCurSel(-1);
					}
				}
				return;
			}

			//we'll always update for a quote, but conditionally for a bill

			//see if they are showing the charges' patient coordinator. If so, prompt.
			//If not, update automatically.
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			if(ReturnsRecordsParam("SELECT BillPatientCoordinator FROM ConfigBillColumnsT WHERE LocationID = {INT} AND BillPatientCoordinator = 1",GetCurrentLocationID())
				&& m_dlgBilling.m_List->GetRowCount() > 0
				&& IDNO == MessageBox("Would you like to update each charge to use the new patient coordinator?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		// (j.jones 2011-01-25 15:29) - PLID 42156 - track this coordinator ID
		m_nCurCoordinatorID = VarLong(varCoordID, -1);
		
		//update each charge
		m_dlgBilling.UpdatePatCoordIDs(varCoordID);

	}NxCatchAll("Error changing coordinator.");
}

void CBillingModuleDlg::OnSelChosenCustomReportCombo(long nRow) 
{
	if(nRow == -1)
		m_pQuoteCustomList->CurSel = 0;
}

void CBillingModuleDlg::OnBtnSaveCopy() 
{
	try {

		if (m_iBillID == -1) {
			return;
		}

		if(!CheckCurrentUserPermissions(bioPatientQuotes,sptWrite))
			return;

		if(IDNO == MessageBox("Do you want to save the changes to the current quote?\n"
			"\nChoosing 'Yes' will save this quote, save a copy of it, and continue editing the existing quote."
			"\nChoosing 'No' will save changes to a new quote, and begin editing the new quote.","Practice",MB_ICONQUESTION|MB_YESNO)) {

			//save changes to a new quote
			m_iBillID = -1;
			
			//if you are not editing the quote, switch it so you are
			if(m_eHasAccess == batNoAccess) {
				m_eHasAccess = batFullAccess;
				m_bHasEverHadAccess = TRUE;
			}

			// (j.jones 2011-01-21 10:10) - PLID 42156 - the access level is now an enum,
			// if they only have partial access then the date & pat. coord. is disabled,
			// as is the ability to delete
			GetDlgItem(IDC_BILL_EDIT_BTN)->EnableWindow(m_eHasAccess == batNoAccess);
			GetDlgItem(IDC_BILL_DELETE_BTN)->EnableWindow(m_eHasAccess == batFullAccess && m_iBillID != -1);
			GetDlgItem(IDC_BILL_DATE)->EnableWindow(m_eHasAccess == batFullAccess);
			GetDlgItem(IDC_EDIT_DESCRIPTION)->EnableWindow(m_eHasAccess != batNoAccess);
			GetDlgItem(IDC_COORDINATOR_COMBO)->EnableWindow(m_eHasAccess == batFullAccess);
			m_dlgBilling.m_eHasAccess = m_eHasAccess;
			m_dlgBilling.ChangeAccess();

			m_dlgBilling.SetAllChargesToBeUnsaved();			

			// (j.gruber 2007-02-21 10:16) - PLID 24556 - set the billing2 dlg also
			m_dlgBilling2.m_eHasAccess = m_eHasAccess;
			m_dlgBilling2.ChangeAccess();

			// (j.gruber 2007-02-21 14:52) - PLID 24847 - grey out the copy save button
			GetDlgItem(IDC_BTN_SAVE_COPY)->EnableWindow(FALSE);
		}
		else {
			//save this quote, then copy it
			if(Save())
				CopyQuote(m_iBillID);
		}
	
	}NxCatchAll("Error copying quote.");
}

void CBillingModuleDlg::OnQuoteMakeDefault() {

	try {
		//first make sure that this is a quote
		if (m_EntryType == 2) {

			//get the current selection of the quote custom list drop down
			long nCurSel = m_pQuoteCustomList->CurSel;

			//get the number we need
			long nNumber = m_pQuoteCustomList->GetValue(nCurSel, qclcNumber);

			// (j.gruber 2007-08-15 11:35) - PLID 27068 - support multiple quotes
			// (j.gruber 2007-11-20 11:53) - PLID 28144 - broke this into 2 user defaults
			long nReportID = VarLong(m_pQuoteCustomList->GetValue(nCurSel, qclcID), 227);
			
			CString strByUserName, strByUserReportIDName;
			CString strMsg;
			if (nReportID == 227) {
				strMsg = "This will set the currently selected report to be the default for your regular quote only.\n\n"
					"You must select a package report to change the default report for package quotes.\n"
					"You can still run a non-package custom report on a package by selecting it from the drop down.\n\n"					
					"Are you sure you want to continue?";
				nReportID = 227;
				strByUserName = "QuoteDefaultByUser";
				strByUserReportIDName = "QuoteDefaultByUserReportID";
			}
			else {
				strMsg = "This will set the currently selected report to be the default for your package quote only.\n\n"
					"You must select a non-package report to change the default report for non-package quote.\n\n"		
					"Are you sure you want to continue?";
				nReportID = 608;
				strByUserName = "PackageQuoteDefaultByUser";
				strByUserReportIDName = "PackageQuoteDefaultByUserReportID";
			}


			if (IDYES != MsgBox(MB_YESNO, strMsg)) {

				//they want to quit
				return;
			
			}
			

			//check to see if they want to ake this a user setting or for everyone
			if (IDYES == MsgBox(MB_YESNO, "Would you like to make this default for every user or just for your username? \nClick Yes to default it for every user, No to default it for just this username.")) {

				//they want to default it for every user
				//set the value in DefaultReportsT
				//see if it exists or not yet

				
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT CustomReportID from DefaultReportsT WHERE ID = {INT}", nReportID);

				//set the remote property to reflect this
				SetRemotePropertyInt(strByUserName, -2, 0, GetCurrentUserName());
				SetRemotePropertyInt(strByUserReportIDName, -2, 0, GetCurrentUserName());
				SetRemotePropertyText(strByUserName, "", 0, GetCurrentUserName());

				if (rs->eof) {

					//there isn't one already so we have to insert it
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					ExecuteParamSql("INSERT INTO DefaultReportsT (ID, CustomReportID) VALUES ({INT}, {INT}) ", nReportID, nNumber);
				}
				else {

					//just need to update
					// (a.walling 2007-11-29 16:46) - PLID 28155 - These variables were reversed
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					ExecuteParamSql("UPDATE DefaultReportsT SET CustomReportID = {INT} WHERE ID = {INT}", nNumber, nReportID);
				}
			}
			else {

				//they want to just default it for them
				//get the filename
				//check to see if it is the system report
				if (nNumber != -1) {
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rsFileName = CreateParamRecordset("SELECT Filename FROM CustomReportsT WHERE ID = {INT} and Number = {INT}", nReportID, nNumber);
					CString strFileName = AdoFldString(rsFileName, "FileName", "");
					if (! strFileName.IsEmpty()) {
						SetRemotePropertyText(strByUserName, strFileName, 0, GetCurrentUserName());
						//set the number too
						SetRemotePropertyInt(strByUserName, nNumber, 0, GetCurrentUserName());
						SetRemotePropertyInt(strByUserReportIDName, nReportID, 0, GetCurrentUserName());
					}
				}
				else {
					//they want to use the system default report
					SetRemotePropertyText(strByUserName, "", 0, GetCurrentUserName());
					//set the number too
					SetRemotePropertyInt(strByUserName, -1, 0, GetCurrentUserName());
					SetRemotePropertyInt(strByUserReportIDName, nReportID, 0, GetCurrentUserName());
				}

			}

			//update the colors

			NXDATALISTLib::IRowSettingsPtr pRow;
			for (int i=0; i < m_pQuoteCustomList->GetRowCount(); i++) {
				pRow = m_pQuoteCustomList->GetRow(i);
				if (pRow) {
					if (VarLong(pRow->GetValue(qclcID)) == nReportID) {
						pRow->PutForeColor(RGB(0,0,0));
					}
				}
			}

			//now set the cur sel
			pRow = m_pQuoteCustomList->GetRow(nCurSel);
			if (pRow) {
				if (nReportID == 227) {
					pRow->PutForeColor(RGB(255,0,0));
				}
				else {
					pRow->PutForeColor(RGB(0,0,255));
				}
			}


		}
	}NxCatchAll("Error in QuoteMakeDefault");

}

// (j.jones 2010-05-20 09:15) - PLID 32338 - added an optional default insured party ID,
// because when opening a bill, if this function is called, the same recordset would otherwise
// be run twice
BOOL CBillingModuleDlg::ApplyInsuranceReferral(bool bForcePrompt /*= false*/, bool bIsNewBill /* = true*/, long nDefaultInsuredPartyID /* = -1*/) {

	try {

		//check to see if we need to prompt for a pre-auth, and if so, prompt!
		long nGuar = nDefaultInsuredPartyID;

		// (j.jones 2010-05-20 09:16) - PLID 32338 - if a nDefaultInsuredPartyID was given to us,
		// we do not need to run this recordset
		if(nGuar == -1) {
			//DRT 5/7/03 - If the insurance tab has the primary, we choose guar1, otherwise we choose guar2
			//JMJ 9/2/03 - If there are existing referrals but the ins.co. doesn't prompt, it should still prompt
			// (j.jones 2010-05-20 09:10) - PLID 32338 - parameterized
			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 InsuredPartyT.PersonID, InsuredPartyT.RespTypeID "
				"FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} AND RespTypeID > 0 AND (UseReferrals = 1 OR "
				"EXISTS (SELECT ID FROM InsuranceReferralsT "
				"LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
				"GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
				"WHERE NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) "
				"	AND StartDate <= {STRING} AND EndDate >= {STRING} AND InsuredPartyID = InsuredPartyT.PersonID)) "
				"ORDER BY Priority", m_nPatientID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(),dtoDate), FormatDateTimeForSql(COleDateTime::GetCurrentTime(),dtoDate));
			if(!rs->eof) {
				nGuar = AdoFldLong(rs, "PersonID");
			}
			else {
				m_bPromptForReferral = FALSE;
				return IR_FAIL;
			}
			rs->Close();
		}

		// (j.jones 2010-05-20 09:11) - PLID 32338 - removed use of bForcePrompt here as it was worthless
		// and had no effect (it is used in some location logic later)

		CInsuranceReferralsSelectDlg dlg(this);
		// (a.walling 2008-07-07 17:34) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		dlg.m_nPatientID = m_nPatientID;
		dlg.m_InsuredPartyID = nGuar;
		dlg.m_bIsNewBill = bIsNewBill;
		{
			// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
			COleDateTime dtBillDate =  (m_date.GetValue());
			// we only want the date part of the bill date, not the time
			dlg.m_dtFilterDate.SetDate(dtBillDate.GetYear(), dtBillDate.GetMonth(), dtBillDate.GetDay());
		}
		
		if(dlg.DoModal() == IDCANCEL){
			//user canceled the bill altogether
			//CBillingModuleDlg::OnCancel();
			if(bIsNewBill){
				PostMessage(NXM_BILL_CANCEL);
				return IR_CANCEL_BILL;
			}
			else{
				//m_bPromptForReferral = FALSE;
				return IR_CANCEL_BILL;
			}
		}
		else{
			if(dlg.m_nSelectedInsuranceReferralID == -1) {
				//Don't bother checking anything against it, they just want to clear it out
				m_bPromptForReferral = FALSE;
				return IR_FAIL;
			}
			else {
				//DRT 5/6/03 - they've chosen a referral, we need to make sure every item already on the bill exists
				//		in this referral.  If it does not, warn them

				// check to see if the filter date has changed though
				// (a.walling 2008-05-13 15:28) - PLID 27591 - VarDateTime not needed any longer
				COleDateTime dtBillDate = (m_date.GetValue());
										
				if(dlg.m_bUseDateFilter && CompareDateIgnoreTime(dtBillDate, dlg.m_dtFilterDate) != 0){
					CString strMessage;
					//the filter date should be a valid date for the selected referral
					ASSERT(dlg.m_dtFilterDate <= dlg.m_dtSelectedEndDate && dlg.m_dtFilterDate >= dlg.m_dtSelectedStartDate);
					if(CompareDateIgnoreTime(dtBillDate, dlg.m_dtSelectedStartDate) < 0 || CompareDateIgnoreTime(dtBillDate, dlg.m_dtSelectedEndDate) > 0){
						//bill date is out of range of the selected referral){
						strMessage.Format("The current bill date, %s, is outside of the range for this insurance "
										"referral.\r\n\r\nWould you like to change the date of the bill to %s?", FormatDateTimeForInterface(dtBillDate, 0, dtoDate), FormatDateTimeForInterface(dlg.m_dtFilterDate, 0, dtoDate));
					}
					else{					
						strMessage.Format("Would you like to set the date of the bill to %s ?", FormatDateTimeForInterface(dlg.m_dtFilterDate, 0, dtoDate));
					}
					
					if(IDYES == AfxMessageBox(strMessage,MB_YESNO)) {
						//they want to change the bill date
						m_date.SetValue(COleVariant(dlg.m_dtFilterDate));
					}	
					else{//don't change the date
					}
				}
				else if(CompareDateIgnoreTime(dtBillDate, dlg.m_dtSelectedEndDate) > 0) {
						//the bill date is after the ending date of the referral's date range
						CString strMessage;
						strMessage.Format("The current bill date, %s, is after the date range for this insurance referral.\r\n\r\n"
									"Would you like to change the date of the bill to %s ?", FormatDateTimeForInterface(dtBillDate, 0, dtoDate), FormatDateTimeForInterface(dlg.m_dtSelectedEndDate, 0, dtoDate));
						if(IDYES == AfxMessageBox(strMessage, MB_YESNO)){
							m_date.SetValue(COleVariant(dlg.m_dtSelectedEndDate));
						}
				}
				else if(CompareDateIgnoreTime(dtBillDate, dlg.m_dtSelectedStartDate) < 0) {
						//the bill date is before the start of the referral's date range
						CString strMessage;
						strMessage.Format("The current bill date, %s, is before the date range for this insurance referral.\r\n\r\n"
									"Would you like to change the date of the bill to %s ?", FormatDateTimeForInterface(dtBillDate, 0, dtoDate), FormatDateTimeForInterface(dlg.m_dtSelectedStartDate, 0, dtoDate));
						if(IDYES == AfxMessageBox(strMessage, MB_YESNO)) {
							m_date.SetValue(COleVariant(dlg.m_dtSelectedStartDate));
						}
				}
				else{
						//the date looks to be valid, so don't prompt them to change it
				}

				//if we are entering a new bill, don't prompt for location
				if(((!bForcePrompt && bIsNewBill) || !bIsNewBill) && dlg.m_nSelectedLocationID != -1) {
					long nPOSID = -1;
					CString strBillPOS, strRefPOS;
					if(m_dlgBilling.m_PlaceOfServiceCombo->CurSel != -1) {
						nPOSID = VarLong(m_dlgBilling.m_PlaceOfServiceCombo->GetValue(m_dlgBilling.m_PlaceOfServiceCombo->CurSel,0),-1);
						strBillPOS = VarString(m_dlgBilling.m_PlaceOfServiceCombo->GetValue(m_dlgBilling.m_PlaceOfServiceCombo->CurSel,1),"");
					}

					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}",dlg.m_nSelectedLocationID);
					if(!rs->eof) {
						strRefPOS = AdoFldString(rs, "Name","");
					}
					rs->Close();

					if(dlg.m_nSelectedLocationID != nPOSID) {
						CString strMessage;
						strMessage.Format("The current place of service, '%s' is different than the place of service for this insurance referral.\r\n\r\n"
									"Would you like to change the place of service of the bill to '%s' ?", strBillPOS, strRefPOS);
						if(IDYES == AfxMessageBox(strMessage, MB_YESNO)) {
							m_dlgBilling.m_PlaceOfServiceCombo->SetSelByColumn(0, (long)dlg.m_nSelectedLocationID);
						}
					}
				}
				
				//to avoid opening multiple connections to the database, lookup all the serviceid's and save them in an array
				CArray<long, long> aryServiceIDs;
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceID FROM InsuranceReferralCPTCodesT WHERE ReferralID = {INT}", dlg.m_nSelectedInsuranceReferralID);
				while(!rs->eof) {
					aryServiceIDs.Add(AdoFldLong(rs, "ServiceID"));
					rs->MoveNext();
				}
				rs->Close();

				bool bOneFailed = false;

				//Loop through all charges
				// (j.jones 2011-10-05 13:45) - PLID 45799 - this is a DL2 now
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlgBilling.m_List->GetFirstRow();
				while(pRow) {
					//get the cpt code of this charge
					long nServiceID = -1;
					_variant_t varServiceID = pRow->GetValue(m_dlgBilling.GetServiceIDColumn());//I can't use the billingdlg.cpp #define's here, but this should match COLUMN_SERVICE_ID
					if(varServiceID.vt == VT_I4)
						nServiceID = VarLong(varServiceID, -1);	

					//check to ensure it exists in our referral
					bool bFound = false;
					for(int j = 0; j < aryServiceIDs.GetSize() && !bFound; j++) {
						if(aryServiceIDs.GetAt(j) == nServiceID)
							bFound = true;
					}

					if(!bFound) {
						//this item was not found in the list of allowed referrals, but it's in the CPT Code list
						bOneFailed = true;
					}

					pRow = pRow->GetNextRow();
				}

				if(bOneFailed) {
					if(MsgBox(MB_YESNO, "You have a Service Code on this bill which is not included in the selected referral.\n"
						"Are you sure you wish to select this authorization?") == IDNO) {
						return IR_CANCEL_BILL;
					}
				}
			}

			//if we got here either everything passed our checks or they want to keep this auth anyways
			//save the info we've got
			m_nInsuranceReferralID = dlg.m_nSelectedInsuranceReferralID;
			m_strSelectedAuthNum = dlg.m_strSelectedAuthNum;
			m_bPromptForReferral = FALSE;

			if(m_dlgInsuranceBilling)
				m_dlgInsuranceBilling.SetDlgItemText(IDC_EDIT_AUTHORIZATION_NUMBER,m_strSelectedAuthNum);
		}

		return IR_SUCCESS;

	}NxCatchAll("Error applying insurance referral.");
	
	return IR_FAIL;
}

void CBillingModuleDlg::OnMergeToWord()
{
	try {

		if(m_dlgBilling.m_pList->GetRowCount() == 0) {
			if(m_EntryType == 2)
				AfxMessageBox("There are no charges on this quote. The merge will not be generated.");
			else
				AfxMessageBox("There are no charges on this bill. The merge will not be generated.");

			return;
		}

		CString strWarn = "This action will first save all changes you've made to this bill.\n"
			"Clicking 'Cancel' on the bill later will not undo your changes.\n"
			"Are you sure you wish to continue?";

		if(m_EntryType == 2)
			strWarn.Replace("bill","quote");

		if(IDNO == DontShowMeAgain(this, strWarn, "BillingDlgMergeToWord", "Practice", FALSE, TRUE)) {
			return;
		}

		long nCurrentBillID = m_iBillID;

		if(!Save())
			return;

		// (j.gruber 2007-02-21 15:03) - PLID 24847 - reenable the save copy button if necessary
		if (m_EntryType == 2) {

			if (nCurrentBillID == -1) {
				GetDlgItem(IDC_BTN_SAVE_COPY)->EnableWindow(TRUE);
			}
		}

		//make sure word exists
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		// Get template to merge to
		// (a.walling 2007-03-26 15:07) - PLID 25337 - OVERWRITEPROMPT doesn't make any sense for an OPEN style file dialog.
		// Also, add OFN_FILEMUSTEXIST so the user isn't as easily fooled into thinking that they are saving it here.
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "Microsoft Word Templates|*.dot;*.dotx;*.dotm||");
		CString initialDir = GetTemplatePath();
		dlg.m_ofn.lpstrInitialDir = initialDir;
		if (dlg.DoModal() == IDOK) {

			// If the user clicked OK do the merge
			CWaitCursor wc;
			CString strTemplateName = dlg.GetPathName();

			/// Generate the temp table
			CString strSql;
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", m_nPatientID);
			CString strMergeT = CreateTempIDTable(strSql, "ID");
			
			// Merge
			CMergeEngine mi;
			
			mi.m_arydwBillIDs.Add(m_iBillID);

			if (g_bMergeAllFields)
				mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
			
			//yes, save in history
			mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;

			// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
			mi.LoadSenderInfo(FALSE);

			// Do the merge
			mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT);
		}

	} NxCatchAll("CBillingModuleDlg::OnMergeToWord");
}

BOOL CBillingModuleDlg::AttemptUpdateScheduler()
{
	//DRT 7/25/03 - I added a new preference that attempts to tie the schedule in with the billing.  If the current patient has
	//		an appointment that is marked as 'In' (for today's date), then there are 3 options - Never do anything, Always mark
	//		the appt as 'Out', Prompt before marking the appt as 'Out'.  This should *only* work on today's appts, the idea being
	//		that the bill is made as the patient is leaving the office, meaning they are to be marked 'Out' anyways.

	//1)  Check the preference
	// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Always
	long nMarkOut = GetRemotePropertyInt("BillMarkApptOut", 1, 0, "<None>", true);

	if(m_EntryType == 1 && nMarkOut > 0) {	//0 = never

		//2)  See if they have an appointment currently marked in.  If there are more than 1 appts currently marked in for today, 
		//		all appointments will be marked as out.
		// (j.jones 2008-06-03 15:01) - PLID 27154 - ensured both the appt date and GetDate() are converted the same way so they would be equivalent
		_RecordsetPtr prsRes = CreateParamRecordset("SELECT AppointmentsT.ID, Date, StartTime, AptTypeT.Name AS TypeName "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE "
			"AppointmentsT.Status <> 4 AND (AppointmentsT.ShowState = 1 OR AppointmentsT.ShowState = 4) "
			"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
			"AND AppointmentsT.PatientID = {INT}", m_nPatientID);

		if(!prsRes->eof) {
			//there are some appointments today.
			bool bContinue = true;
			if(nMarkOut == 2) {
				//we need to prompt them, so make up a description from the recordset
				CString strDesc;
				while(!prsRes->eof) {
					CString str;
					_variant_t var, var2;
					var = prsRes->Fields->Item["TypeName"]->Value;
					var2 = prsRes->Fields->Item["StartTime"]->Value;
					str.Format("Type:  %s   at  %s.\r\n", 
						var.vt == VT_BSTR ? VarString(var) : "<No Type>", 
						FormatDateTimeForInterface(VarDateTime(var2), dtoTime));

					strDesc += str;

					prsRes->MoveNext();
				}

				//now prompt them
				CString strMsg;
				strMsg.Format("You are saving a bill for a patient who is currently marked 'In' for the following appointments:\r\n"
					"%s\r\n"
					"Would you like to mark these appointments as 'Out'?", strDesc);

				if(MsgBox(MB_YESNO, strMsg) == IDNO) {
					bContinue = false;
				}
			}

			//If bContinue is true, then either we prompted them and they said 'Yes', or they were set to 'Always' update, 
			//or bContinue is false, meaning they said 'No' to the update.
			if(bContinue) {
				//we need to update all of these items to 'Out'
				prsRes->MoveFirst();
				while(!prsRes->eof) {
					//mark the appt as 'Out'
					AppointmentMarkOut(AdoFldLong(prsRes, "ID"));
					prsRes->MoveNext();
				}

				return TRUE;
			}
			else {
				//they didn't want to update
			}
		}
	}

	return FALSE;
}

void CBillingModuleDlg::OnTrySetSelFinishedCoordinatorCombo(long nRowEnum, long nFlags) 
{
	if(nFlags == NXDATALISTLib::dlTrySetSelFinishedFailure) {
		//maybe it's inactive?
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rsCoord = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = (SELECT PatCoord FROM BillsT WHERE ID = {INT})", GetBillID());
		if(!rsCoord->eof) {
			m_CoordCombo->PutComboBoxText(_bstr_t(AdoFldString(rsCoord, "Name", "")));
		}
		else 
			m_CoordCombo->PutCurSel(-1);
	}
}


// (a.walling 2010-11-01 12:38) - PLID 40965 - Returns the recordset now
ADODB::_RecordsetPtr CBillingModuleDlg::GetChargeRecordset()
{
	//DRT 4/7/2006 - PLID 11734 - Removed ProcCode, added ItemType field calc.
	// (j.gruber 2007-03-22 12:00) - PLID 24870 - discount categories
	// (j.gruber 2007-04-03 17:25) - PLID 9796 - coupons
	// (j.jones 2008-05-28 11:32) - PLID 28782 - added NDCCode
	// (j.jones 2008-06-04 15:00) - PLID 30256 - added EMRChargeID, but it's always NULL here
	// (j.jones 2008-06-04 15:00) - PLID 30256 - added AppointmentID
	// (j.gruber 2009-03-11 15:32) - PLID 33360 - take out discount fields
	// (j.jones 2009-08-13 12:27) - PLID 35206 - added other drug fields
	// (j.gruber 2009-10-19 16:46) - PLID 35947 - added allowable
	// (j.jones 2009-12-23 09:41) - PLID 32587 - added OriginalPackageQtyRemaining
	// (j.jones 2010-04-08 11:59) - PLID 15224 - added ChargesT.IsEmergency
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	// (j.jones 2010-11-09 10:06) - PLID 31392 - added ClaimProviderID
	// (s.dhole 2011-05-16 16:24) - PLID  33666 dbo.GetDiagListFromChargeWhichCodes(LineItemT.ID) AS WhichCodesExt
	//TES 7/1/2011 - PLID 44192 - Added GlassesOrderServiceID
	// (j.jones 2011-08-24 09:51) - PLID 44873 - added IsVoidingCharge
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added IsOriginalCharge
	// (j.jones 2011-10-25 16:14) - PLID 46088 - added Calls column, Alberta only
	// (j.jones 2012-01-17 15:11) - PLID 47537 - added EMRChargeID for Bills only
	// (d.singleton 2012-05-08 15:21) - PLID 48152 added taxonomy code for alberta skill column
	// (d.singleton 2014-03-03 17:26) - PLID 61154 - need to work with new data structure
	// (j.gruber 2014-03-10 09:20) - PLID 60942 - take out whichCodes field
	// (j.jones 2014-04-23 11:03) - PLID 61836 - added referring, ordering, supervising providers
	// (r.gonet 07/08/2014) - PLID 62569 - Added whitespace to make more readable since it was easy to add columns at the wrong level in the query.
	//   Added VoidingLineItemID, OriginalLineItemID, and IsNewChargeFromCorrection
	// (r.gonet 08/06/2014) - PLID 63098 - Had added a column for LabCharge. Also fixed one formatting issue. Ended up not needing the new column but kept the formatting fix.
	// (s.tullis 2015-03-24 09:28) - PLID 64975 -  Added CPT Category Count.. used to dermine whether or not we can show/edit the bill dialog category column
	// Category combo will be editable by the user 
	// (r.gonet 2015-03-27 19:08) - PLID 65279 - Get the LineItemT.GCValue as well.
	// (d.lange 2015-11-18 10:47) - PLID 67128 - Load ChargesT.AllowableInsuredPartyID for calculating the allowable
	return CreateCachedParamRecordset(
		"SELECT [PatientChargesQ].ID, "
		"	ChargesT.BillID AS SrgyTrackID, "
		"	ChargesT.ServiceLocationID AS [Service Location], [PatientChargesQ].Description AS ItemDesc,  "
		"	ChargesT.DoctorsProviders, ChargesT.ClaimProviderID, ChargesT.PatCoordID, ChargesT.Batched, [PatientChargesQ].Date AS TDate,  "
		"	[PatientChargesQ].InputDate AS IDate, ChargesT.ServiceDateTo, ChargesT.ServiceID, ChargesT.ItemCode,  "
		"	ChargesT.ServiceType AS [Service Type], ChargesT.ItemSubCode, ChargesT.Category, ChargesT.SubCategory,  "
		"	ChargesT.TaxRate, ChargesT.TaxRate2, "
		"	ChargesT.CPTModifier, ChargesT.CPTModifier2, ChargesT.CPTModifier3, ChargesT.CPTModifier4,  "
		"	CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4,  "
		"	ChargesT.Quantity, ChargesT.PackageQtyRemaining, ChargesT.OriginalPackageQtyRemaining, "
		"	[PatientChargesQ].Amount AS PracBillFee, ChargesT.OthrBillFee, ChargesT.LineID,  "
		"	CASE WHEN Ins1Charges.ChargeID Is Null THEN NULL ELSE Ins1Charges.Ins1Resp END AS Ins1Resp,  "
		"	CASE WHEN Ins2Charges.ChargeID Is Null THEN NULL ELSE Ins2Charges.Ins2Resp END AS Ins2Resp,  "
		"	CASE WHEN Ins3Charges.ChargeID Is Null THEN NULL ELSE Ins3Charges.Ins3Resp END AS Ins3Resp,  "
		"	PatientChargesQ.LocationID, ChargesT.PatCoordID, PatientChargesQ.GiftID, PackageChargeRefID,  "
		"	CASE WHEN CPTCodeT.ID IS NOT NULL THEN 1  "
		"     WHEN ProductT.ID IS NOT NULL THEN 2 "
		"     WHEN GCTypesT.ServiceID IS NOT NULL THEN 3 "
		"	  WHEN AdministrativeFeesT.ID IS NOT NULL THEN 4 END AS ItemType, "
		/*"	CASE WHEN ChargesT.DiscountCategoryID IS NULL then 0 else 1 END AS HasDiscountCategory,  "
		"	ChargesT.DiscountCategoryID, ChargesT.CustomDiscountDesc, ChargesT.CouponID, */
		"	ChargesT.NDCCode, "
		"	ChargesT.DrugUnitPrice, ChargesT.DrugUnitTypeQual, ChargesT.DrugUnitQuantity, ChargesT.PrescriptionNumber, "
		"	ChargesT.IsEmergency, "
		"	(CASE WHEN PatientChargesQ.Type = 10 THEN ChargesT.EMRChargeID ELSE NULL END) AS EMRChargeID, "
		"	ChargesT.AppointmentID, ChargesT.Allowable, ChargesT.GlassesOrderServiceID, "
		"	LineItemCorrections_NewChargesQ.VoidingLineItemID, LineItemCorrections_NewChargesQ.OriginalLineItemID,"
		"	Convert(bit, CASE WHEN LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null THEN 0 ELSE 1 END) AS IsVoidingCharge, "
		"	Convert(bit, CASE WHEN LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null THEN 0 ELSE 1 END) AS IsOriginalCharge, "
		"	Convert(bit, CASE WHEN LineItemCorrections_NewChargesQ.NewLineItemID Is Null THEN 0 ELSE 1 END) AS IsNewChargeFromCorrection, "
		"	ChargesT.Calls, ChargesT.SkillCode, ChargesT.ReferringProviderID, ChargesT.OrderingProviderID, ChargesT.SupervisingProviderID, "
		"   COALESCE(CptCategoryCountQ.CPTCategoryCount, 0 ) as CategoryCount, PatientChargesQ.GCValue, "
		"	AllowableInsuredQ.InsuranceCoID AS AllowableInsuranceCoID "
		""
		"FROM "
		"( "
		"	( "
		"		SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, ChargesT.ClaimProviderID, "
		"			ChargesT.PatCoordID, ChargesT.Batched, ChargesT.NDCCode, "
		"			ChargesT.DrugUnitPrice, ChargesT.DrugUnitTypeQual, ChargesT.DrugUnitQuantity, ChargesT.PrescriptionNumber, "
		"			ChargesT.IsEmergency, "
		"			ChargesT.AppointmentID, ChargesT.Allowable, ChargesT.GlassesOrderServiceID "
		"		FROM LineItemT  "
		"		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
		"		WHERE LineItemT.PatientID = {INT} AND LineItemT.Deleted=0 AND LineItemT.Type>=10 "
		"	) AS PatientChargesQ  "
		"	INNER JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
		")  "
		"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID  "
		"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
		"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
		"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
		"LEFT JOIN GCTypesT ON ServiceT.ID = GCTypesT.ServiceID "
		"LEFT JOIN AdministrativeFeesT ON ServiceT.ID = AdministrativeFeesT.ID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT NewLineItemID, VoidingLineItemID, OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_NewChargesQ ON PatientChargesQ.ID = LineItemCorrections_NewChargesQ.NewLineItemID "
		"LEFT JOIN "
		"( "
		"	SELECT Sum(Amount) AS Ins1Resp, ChargeID, RespTypeID FROM ChargeRespT  "
		"	INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID  "
		"	GROUP BY ChargeID, RespTypeID HAVING InsuredPartyT.RespTypeID = 1 "
		") AS Ins1Charges ON PatientChargesQ.ID = Ins1Charges.ChargeID  "
		"LEFT JOIN "
		"( "
		"	SELECT Sum(Amount) AS Ins2Resp, ChargeID, RespTypeID FROM ChargeRespT  "
		"	INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID  "
		"	GROUP BY ChargeID, RespTypeID HAVING InsuredPartyT.RespTypeID = 2 "
		") AS Ins2Charges ON PatientChargesQ.ID = Ins2Charges.ChargeID  "
		"LEFT JOIN "
		"( "
		"	SELECT Sum(Amount) AS Ins3Resp, ChargeID, RespTypeID FROM ChargeRespT  "
		"	INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID  "
		"	GROUP BY ChargeID, RespTypeID HAVING InsuredPartyT.RespTypeID = 3 "
		") AS Ins3Charges ON PatientChargesQ.ID = Ins3Charges.ChargeID  "
		"LEFT JOIN "
		"( Select ServiceID, COUNT( DISTINCT ServiceMultiCategoryT.CategoryID ) as CPTCategoryCount "
		" FROM ServiceMultiCategoryT "
		" Group BY ServiceID " 
		" ) CptCategoryCountQ On  CptCategoryCountQ.ServiceID = ServiceT.ID "
		"LEFT JOIN ( "
		"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID "
		"	FROM InsuredPartyT "
		"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID "
		"WHERE ChargesT.BillID = {INT} ORDER BY LineID ASC", m_nPatientID, m_iBillID);
}

BOOL CBillingModuleDlg::CheckHasInsuranceCharges()
{
	return m_dlgBilling.CheckHasInsuranceCharges();
}

// (z.manning 2010-08-16 10:26) - PLID 40120 - Added this to CBillingModuleDlg so the insurance tab could access it
long CBillingModuleDlg::GetCurrentBillToInsuredPartyID()
{
	return m_dlgBilling.GetCurrentBillToInsuredPartyID();
}

void CBillingModuleDlg::EnableButtons(BOOL bEnable)
{	
	GetDlgItem(IDC_BILL_PREVIEW_BTN)->EnableWindow(bEnable);
	GetDlgItem(IDCANCEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_SAVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BILL_EDIT_BTN)->EnableWindow(bEnable);
	GetDlgItem(IDC_BILL_MERGE_BTN)->EnableWindow(bEnable);
	// (j.jones 2011-01-21 10:32) - PLID 42156 - be mindful of the access level
	GetDlgItem(IDC_BILL_DELETE_BTN)->EnableWindow(bEnable && m_eHasAccess == batFullAccess && m_iBillID != -1);
	GetDlgItem(IDC_BTN_EDIT_TEXT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_SAVE_COPY)->EnableWindow(bEnable);
}

// (a.walling 2008-05-05 12:53) - PLID 29897 - Patient name
CString CBillingModuleDlg::GetBillPatientName(long nPatientID /*= -1*/)
{
	if (nPatientID == -1)
		nPatientID = m_nPatientID;

	ASSERT(m_nPatientID != -1);

	if (m_nCachedPatientNameID != m_nPatientID) {
		m_strCachedPatientName = GetExistingPatientName(m_nPatientID);
		m_nCachedPatientNameID = m_nPatientID;
	}

	return m_strCachedPatientName;
}

void CBillingModuleDlg::AddNewProductToBillByServiceID(long nServiceID, double dblQtyDefault /*= 1.0*/)
{
	//TES 7/1/2008 - PLID 26143 - Tell the CBillingDlg to add it.
	m_dlgBilling.AddNewProductToBillByServiceID(nServiceID, dblQtyDefault);
}

void CBillingModuleDlg::ApplyPaymentIDs(const CDWordArray &dwaPaymentIDs)
{
	//TES 7/1/2008 - PLID 26143 - Tell the CBillingDlg to add it.
	m_dlgBilling.ApplyPaymentIDs(dwaPaymentIDs);
}

void CBillingModuleDlg::SetDefaultBillA(int nBillA)
{
	//TES 7/3/2008 - PLID 26143 - Tell the CBillingDlg what Bill A row to select.
	m_dlgBilling.SetDefaultBillA(nBillA);
}

// (c.haag 2009-08-24 13:12) - PLID 14844 - Sets the bill date. Should only be called when the
// dialog is initializing as it changes both the date control and m_dtCurrentDate.
void CBillingModuleDlg::SetDefaultDate(const COleDateTime& dt)
{
	m_date.SetValue(COleVariant(dt));
	m_dtCurrentDate = dt;
}

// (d.thompson 2009-09-02) - PLID 34694 - This function could be improved.  For right now, it tells the caller
//	if the bill is at all editable, based on the status of the historic financial preference.  However, it would
//	be nice to have a more general function that tells you if you the bill is editable for any reason.  I have no
//	objection to a future user renaming this function and making it do so.
// (j.jones 2011-01-21 09:14) - PLID 42156 - I renamed this function for clarity, and added a string for the
// billing dlg Edit button to use
BOOL CBillingModuleDlg::IsBillWriteable_HistoricFinancial(BOOL bSilent, CString *pstrBillEditWarning /*= NULL*/)
{
	//If we're on a new bill, then we know we pass these rules
	if(m_iBillID == -1) {
		return TRUE;
	}

	// (j.jones 2010-02-04 17:25) - PLID 37212 - This function inexplicably passed in the bill deletion permission,
	// so anytime this function was called, it would return FALSE if you could not delete a bill, regardless of
	// whether that is relevant. I've changed the permission that is sent in here.
	if(!CanChangeHistoricFinancial("Bill", m_iBillID, bioBill, sptWrite, bSilent, pstrBillEditWarning)) {
		return FALSE;
	}

	//All reasons passed
	return TRUE;
}

// (j.jones 2011-08-24 08:41) - PLID 44868 - returns true if any charge
// is an "original" or "void" charge, and therefore read only
BOOL CBillingModuleDlg::HasOriginalOrVoidCharge()
{
	return m_dlgBilling.HasOriginalOrVoidCharge();
}

// (a.walling 2007-05-24 08:46) - PLID 26114
// (j.jones 2013-07-10 15:48) - PLID 57148 - moved these definitions to the cpp where they belonged
COleCurrency CBillingModuleDlg::GetTotalRewardPoints() {
	return m_cyTotalPoints;
}

COleCurrency CBillingModuleDlg::GetAdjustedRewardPoints() {
	return m_cyTotalPoints - m_cyAdjustedPoints;
}

void CBillingModuleDlg::AddAdjustedRewardPoints(COleCurrency &cy) {
	m_cyAdjustedPoints += cy;
	m_dlgBilling2.UpdateRewardPointsDisplay();
}

// (j.jones 2013-07-11 08:53) - PLID 57148 - Retrieves the first charge provider sorted by LineID.
// If Charge 1 has no provider, and Charge 2 has a provider, we'll return the provider ID on Charge 2.
long CBillingModuleDlg::GetFirstChargeProviderID()
{
	if(m_dlgBilling.GetSafeHwnd()) {
		return m_dlgBilling.GetFirstChargeProviderID();
	}

	//it should be impossible for the billing tab to not exist,
	//but if it doesn't, then we definitely do not have charges to look at
	return -1;
}

// (r.gonet 07/02/2014) - PLID 62567 - Sets the bill description edit box value.
// The intention is to hide the prepending of the status prefix. The opposite of this function
// is GetBillDescription()
void CBillingModuleDlg::SetBillDescription(CString &strBillDescription)
{
	// Behind the scenes, prepend the status prefix if necessary.
	CString strNewBillDescription;
	if (m_eBillStatusType == EBillStatusType::OnHold) {
		strNewBillDescription += m_strOnHoldPrefix;
	}
	strNewBillDescription += strBillDescription;

	m_editDescription.SetWindowText(strNewBillDescription);
}

// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, with the status prefix.
CString CBillingModuleDlg::GetBillDescriptionWithPrefix()
{
	CString strBillDescription;
	m_editDescription.GetWindowText(strBillDescription);
	return strBillDescription;
}

// (r.gonet 07/02/2014) - PLID 62567 - Gets the bill description edit box value, without the status prefix.
// The intention is to hide the prepending of the status prefix. The opposite of this function
// is SetBillDescription(str)
CString CBillingModuleDlg::GetBillDescription()
{
	CString strBillDescription = GetBillDescriptionWithPrefix();
	// Behind the scenes, remove the prefix if it exists
	if (m_eBillStatusType == EBillStatusType::OnHold && strBillDescription.Find(m_strOnHoldPrefix) == 0) {
		return strBillDescription.Mid(m_strOnHoldPrefix.GetLength());
	} else {
		return strBillDescription;
	}
}

// (r.gonet 07/02/2014) - PLID 62567 - Just make sure we still have a prefix status if we should.
void CBillingModuleDlg::OnEnKillfocusEditDescription()
{
	try {
		SetBillDescription(GetBillDescription());
	} NxCatchAll(__FUNCTION__);
}

// (s.tullis 2016-02-24 16:37)- 68319 - Update Claim form selection according to the setup
void CBillingModuleDlg::UpdateClaimFormSelection()
{
	try {
		long nInsuranceID=-1;
		long nLocationID=-1;

		if (m_dlgInsuranceBilling && m_dlgBilling)
		{
			nInsuranceID = m_dlgInsuranceBilling.GetPrimaryInsuredPartyID();
			nLocationID = m_dlgBilling.m_nCurLocationID;
		}

		int nFormType = GetFormTypeFromLocationInsuranceSetup(nInsuranceID,nLocationID);

		if (nFormType != -1 && m_dlgInsuranceBilling)
		{
			m_dlgInsuranceBilling.m_FormTypeCombo->SetSelByColumn(0, nFormType);
		}

	}NxCatchAll(__FUNCTION__)
}