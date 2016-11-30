// GCSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GCSearchDlg.h"
#include "InternationalUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "Barcode.h"
#include "GlobalReportUtils.h"
#include "AuditTrail.h"
#include "GiftCertificateSearchUtils.h"
#include "BillingRc.h"// (j.jones 2015-03-25 15:12) - PLID 
#include "NxAPI.h"
#include "GlobalFinancialUtils.h"
#include "GCEntryDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (j.jones 2015-04-01 11:25) - PLID 65391 - defined the auditing naming
#define AUDIT_ACTIVE	"Active"
#define AUDIT_VOIDED	"Voided"

/////////////////////////////////////////////////////////////////////////////
// CGCSearchDlg dialog

CGCSearchDlg::CGCSearchDlg(CWnd* pParent)
	: CNxDialog(CGCSearchDlg::IDD, pParent)
{
	m_nSelectedGCID = -1;
	m_bIsTransferring = false;
	m_nSelectedGCIDToTransfer = -1;
}


void CGCSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGCSearchDlg)
	DDX_Control(pDX, IDC_FILTER_HIDE_VOIDED, m_btnHideVoid);
	DDX_Control(pDX, IDC_FILTER_HIDE_EXPIRED, m_btnHideExpired);
	DDX_Control(pDX, IDC_FILTER_HIDE_BALANCES, m_btnHide0Balance);
	DDX_Control(pDX, IDC_SEARCH_EXP_DATE, m_dtExpires);
	DDX_Control(pDX, IDC_SEARCH_PURCH_DATE, m_dtPurchase);
	DDX_Control(pDX, IDC_PURCH_BY, m_nxeditPurchBy);
	DDX_Control(pDX, IDC_REC_BY, m_nxeditRecBy);
	DDX_Control(pDX, IDC_GC_AMT, m_nxeditGcAmt);
	DDX_Control(pDX, IDC_GC_TYPE, m_nxeditGcType);
	DDX_Control(pDX, IDC_GC_USED, m_nxeditGcUsed);
	DDX_Control(pDX, IDC_GC_BALANCE, m_nxeditGcBalance);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_PREVIEW_GC, m_btnPreviewGC);
	DDX_Control(pDX, IDC_VOID_GC, m_btnVoidGC);
	DDX_Control(pDX, IDC_GO_TO_PURCHASER, m_btnGoToPurchaser);
	DDX_Control(pDX, IDC_GO_TO_RECEIVER, m_btnGoToReceiver);
	DDX_Control(pDX, IDC_GC_CERTIFICATE_NUMBER, m_nxeditGCNumber);
	DDX_Control(pDX, IDC_TRANSFER_GC_BALANCE, m_btnTransferGCBalance);
	DDX_Control(pDX, IDC_GC_CANCEL_TRANSFER, m_btnCancelTransfer);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CGCSearchDlg, IDC_SEARCH_EXP_DATE, 2 /* Change */, OnChangeSearchExpDate, VTS_NONE)

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
BEGIN_MESSAGE_MAP(CGCSearchDlg, CNxDialog)
	//{{AFX_MSG_MAP(CGCSearchDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SEARCH_EXP_DATE, OnChangeSearchExpDate)
	ON_BN_CLICKED(IDC_PREVIEW_GC, OnPreviewGc)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_FILTER_HIDE_BALANCES, OnFilterHideBalances)
	ON_BN_CLICKED(IDC_FILTER_HIDE_EXPIRED, OnFilterHideExpired)
	ON_BN_CLICKED(IDC_GO_TO_PURCHASER, OnGoToPurchaser)
	ON_BN_CLICKED(IDC_GO_TO_RECEIVER, OnGoToReceiver)
	ON_BN_CLICKED(IDC_VOID_GC, OnVoidGc)
	ON_BN_CLICKED(IDC_FILTER_HIDE_VOIDED, OnFilterHideVoided)
	ON_BN_CLICKED(IDC_EDITEXP, OnEditexp)
	ON_BN_CLICKED(IDC_TRANSFER_GC_BALANCE, OnBtnTransferGCBalance)
	ON_BN_CLICKED(IDC_GC_CANCEL_TRANSFER, OnCancel)
	//}}AFX_MSG_MAP		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGCSearchDlg message handlers

BOOL CGCSearchDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 17:12) - PLID 29876 - NxIconify buttons
		// (j.jones 2015-04-24 10:42) - PLID 65710 - if transferring, this is an OK button
		m_btnOK.AutoSet(m_bIsTransferring ? NXB_OK : NXB_CLOSE);
		m_btnPreviewGC.AutoSet(NXB_PRINT_PREV);
		m_btnVoidGC.AutoSet(NXB_MODIFY);
		// (j.jones 2015-04-24 10:15) - PLID 65710 - added ability to transfer balances
		m_btnTransferGCBalance.AutoSet(NXB_MODIFY);
		// (j.jones 2015-04-24 10:40) - PLID 65710 - added a cancel button for transferring into new GCs
		m_btnCancelTransfer.AutoSet(NXB_CANCEL);

		// (j.jones 2015-04-24 10:43) - PLID 65710 - if transferring, we have only OK/Cancel buttons
		if (m_bIsTransferring) {
			m_btnOK.SetWindowText("OK");

			//hide the preview/void/transfer buttons
			m_btnPreviewGC.ShowWindow(SW_HIDE);
			m_btnVoidGC.ShowWindow(SW_HIDE);
			m_btnTransferGCBalance.ShowWindow(SW_HIDE);

			// (r.gonet 2015-06-01 10:10) - PLID 65710 - If transferring, then don't let them select a voided gift certificate.
			CheckDlgButton(IDC_FILTER_HIDE_VOIDED, BST_CHECKED);
			GetDlgItem(IDC_FILTER_HIDE_VOIDED)->EnableWindow(FALSE);

			//move the OK button to be where the void button is
			CRect rcVoid;
			GetDlgItem(IDC_VOID_GC)->GetWindowRect(rcVoid);
			ScreenToClient(rcVoid);
			GetDlgItem(IDOK)->MoveWindow(rcVoid);

			//change the window title
			SetWindowText("Select Gift Certificate For Balance Transfer");

			//and lastly offset this window so it doesn't overlap the old one completely
			CRect rc;
			GetWindowRect(rc);
			rc.OffsetRect(40, 20);
			MoveWindow(rc);
		}
		else {
			//hide the cancel button
			m_btnCancelTransfer.ShowWindow(SW_HIDE);
		}

		// (j.jones 2015-04-24 10:17) - PLID 65710 - disable buttons that they do not have permission for
		if (!GetCurrentUserPermissions(bioVoidGiftCertificates, sptDynamic0)) {
			m_btnVoidGC.EnableWindow(FALSE);
		}
		if (!GetCurrentUserPermissions(bioTransferGiftCertificateBalances, sptWrite)) {
			m_btnTransferGCBalance.EnableWindow(FALSE);
		}

		//PLID 11999 - pull out the remembered filter settings so they get included in the filter requery
		CheckDlgButton(IDC_FILTER_HIDE_BALANCES, GetRemotePropertyInt("GCHideBalance", 0, 0, GetCurrentUserName(), true));
		CheckDlgButton(IDC_FILTER_HIDE_EXPIRED, GetRemotePropertyInt("GCHideExpired", 0, 0, GetCurrentUserName(), true));
		CheckDlgButton(IDC_FILTER_HIDE_VOIDED, 1); // I can't see why this would ever be unchecked even as a pref

		// (j.jones 2015-03-25 15:02) - PLID 65390 - converted the dropdown to a search list
		m_SearchList = GiftCertificateSearchUtils::BindGiftCertificateGeneralSearchListCtrl(this, IDC_GC_SEARCH_LIST, GetRemoteData(),
			IsDlgButtonChecked(IDC_FILTER_HIDE_BALANCES) ? true : false, IsDlgButtonChecked(IDC_FILTER_HIDE_EXPIRED) ? true : false, IsDlgButtonChecked(IDC_FILTER_HIDE_VOIDED) ? true : false);

		// (j.jones 2015-04-01 10:11) - PLID 65391 - converted to a DL2
		m_HistoryList = BindNxDataList2Ctrl(IDC_GC_HISTORY_LIST, false);

		//set up the history list
		{
			// (j.dinatale 2011-12-09 13:54) - PLID 46971 - need to take into account the quantity of the charge, because voiding line items have a negative quantity,
			//		this will allow us to show which amounts are negative and which are positive when dealing with items that are financially corrected
			//find all charges that incremented this balance
			//TES 3/19/2015 - PLID 65072 - Include any GC tips in the balance
			// (j.jones 2015-04-01 10:12) - PLID 65391 - Moved this query into the from clause.
			// This also now includes much more data, like void & corrects and refunds, and now has an
			// "activity" column to explain what the action was that affected the balance.
			// (r.gonet 2015-05-05 09:53) - PLID 66304 - Gift certificate tips can now be refunded. Include those too.
			CString strHistoryFromClause;
			strHistoryFromClause.Format("("
				//initial purchase or recharge
				"SELECT LineItemT.ID AS LineItemID, LineItemT.GiftID, "
				"	CASE WHEN NewLineItemsT.ID Is Not Null THEN 'Gift Certificate ' + ChargesQ.GCType + ' Corrected' "
				"		WHEN VoidingLineItemsT.ID Is Not Null THEN 'Gift Certificate ' + ChargesQ.GCType + ' Voided' "
				"		ELSE 'Gift Certificate ' + ChargesQ.GCType + 'd' "
				"	END AS Activity, "
				"	LineItemT.Date, "
				"	LineItemT.GCValue AS Amount, "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName "
				"FROM LineItemT "
				"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
				"INNER JOIN ("
				"	SELECT ChargesT.ID, "
				"	CASE WHEN IsNull(GCTypesT.IsRecharge,0) = 1 THEN 'Recharge' ELSE 'Purchase' END AS GCType "
				"	FROM ChargesT "
				"	LEFT JOIN GCTypesT ON ChargesT.ServiceID = GCTypesT.ServiceID "
				") AS ChargesQ ON LineItemT.ID = ChargesQ.ID "				
				"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
				"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
				"LEFT JOIN LineItemCorrectionsT NewLineItemsT ON LineItemT.ID = NewLineItemsT.NewLineItemID "
				"WHERE LineItemT.GiftID Is Not Null AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				//payments or refunds
				"UNION ALL "
				"SELECT LineItemT.ID AS LineItemID, LineItemT.GiftID, "
				"	CASE WHEN LineItemT.Type = 3 "	//refund
				"		THEN CASE WHEN NewLineItemsT.ID Is Not Null THEN 'Gift Certificate Refund Corrected' "
				"		WHEN VoidingLineItemsT.ID Is Not Null THEN 'Gift Certificate Refund Voided' "
				"		ELSE 'Refund Made to Gift Certificate' END "
				"	ELSE CASE WHEN NewLineItemsT.ID Is Not Null THEN 'Gift Certificate Payment Corrected' "
				"		WHEN VoidingLineItemsT.ID Is Not Null THEN 'Gift Certificate Payment Voided' "
				"		ELSE 'Payment Made from Gift Certificate' END "
				"	END AS Activity, "
				"	LineItemT.Date, "
				"	(LineItemT.Amount * -1) - COALESCE(GiftTipsQ.TotalGiftTips, 0) AS Amount, "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName "
				"FROM LineItemT "
				"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
				"LEFT JOIN ("
				"	SELECT PaymentID, Sum(Amount) AS TotalGiftTips "
				"	FROM PaymentTipsT "
				"	WHERE PayMethod IN (4,10) "
				"	GROUP BY PaymentID "
				") AS GiftTipsQ ON LineItemT.ID = GiftTipsQ.PaymentID "
				"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID "
				"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID "
				"LEFT JOIN LineItemCorrectionsT NewLineItemsT ON LineItemT.ID = NewLineItemsT.NewLineItemID "
				"WHERE LineItemT.GiftID Is Not Null AND LineItemT.Deleted = 0 AND LineItemT.Type IN (1,3) "				
				//list all transfers out, the GiftID they went to, and that GC's purchaser
				"UNION ALL "
				"SELECT NULL AS LineItemID, GiftCertificateTransfersT.SourceGiftID AS GiftID, "
				"	'Gift Certificate Balance Transferred to GC# ' + GiftCertificatesT.GiftID AS Activity, "
				"	GiftCertificateTransfersT.InputDate AS Date, "
				"	(-1 * GiftCertificateTransfersT.Amount) AS Amount, "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName "
				"FROM GiftCertificatesT "
				"INNER JOIN GiftCertificateTransfersT ON GiftCertificatesT.ID = GiftCertificateTransfersT.DestGiftID "
				"LEFT JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
				//list all transfers in, the GiftID they came from, and that GC's purchaser
				"UNION ALL "
				"SELECT NULL AS LineItemID, GiftCertificateTransfersT.DestGiftID AS GiftID, "
				"	'Gift Certificate Balance Transferred from GC# ' + GiftCertificatesT.GiftID AS Activity, "
				"	GiftCertificateTransfersT.InputDate AS Date, "
				"	GiftCertificateTransfersT.Amount, "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName "
				"FROM GiftCertificatesT "
				"INNER JOIN GiftCertificateTransfersT ON GiftCertificatesT.ID = GiftCertificateTransfersT.SourceGiftID "
				"LEFT JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
				//find out when this was voided
				//since voiding does not actually change the balance, show NULL for the amount
				"UNION ALL "
				"SELECT NULL AS LineItemID, GiftCertificatesT.ID AS GiftID, "
				"	CASE WHEN AuditDetailsQ.OldVal LIKE '%s%%' THEN 'Gift Certificate Voided' "
				"		WHEN AuditDetailsQ.OldVal LIKE '%s%%' THEN 'Gift Certificate Unvoided' "
				"		ELSE '<Unknown Status Change>' END AS Activity, "
				"	AuditT.ChangedDate AS Date, "
				"	NULL AS Amount, "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName "
				"FROM GiftCertificatesT "
				"INNER JOIN (SELECT AuditID, RecordID, Convert(nvarchar(4000), OldValue) AS OldVal "
				"	FROM AuditDetailsT WHERE ItemID = %li) AS AuditDetailsQ ON GiftCertificatesT.ID = AuditDetailsQ.RecordID "
				"INNER JOIN AuditT ON AuditDetailsQ.AuditID = AuditT.ID "
				"LEFT JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
			") AS HistoryQ", _Q(AUDIT_ACTIVE), _Q(AUDIT_VOIDED), aeiGiftCStatus);
			m_HistoryList->PutFromClause(_bstr_t(strHistoryFromClause));
		}

		// check permissions (ability to write to a bill) silently
		m_bEditable = CheckCurrentUserPermissions(bioBill, sptWrite, FALSE, 0, TRUE, FALSE);

		m_dtExpires.EnableWindow(FALSE);
		m_bExpChanged = false;

		if (!m_bEditable) {
			// (z.manning 2011-11-14 16:15) - PLID 35376 - Voiding GCs now has its own permission
			//GetDlgItem(IDC_VOID_GC)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDITEXP)->EnableWindow(FALSE);
		}
		
		//set up the gift list
		UpdateSearchListFilters();
		
		SetDlgItemText(IDC_VOID_GC, "&Void Certificate");
		lastGCVoid = 0;

		//setup the dates
		m_dtExpires.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dtPurchase.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		//register for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this))
				MsgBox("Error registering for barcode scans.  You may not be able to scan.");
		}
	}
	NxCatchAll("Error in CGCSearchDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGCSearchDlg::OnOK() 
{
	try {

		CheckAndSaveExpiration();

		// (j.jones 2015-04-24 10:42) - PLID 65710 - if transferring, they need
		// to have selected a GC
		if (m_bIsTransferring) {
			if (m_nSelectedGCID == -1) {
				AfxMessageBox("You must select a Gift Certificate for the balance transfer.");
				return;
			}

			// (r.gonet 2015-06-01 10:10) - PLID 65710 - If transferring, then don't let them select a voided gift certificate. Safety net; we already disabled
			// the option to show gift certificates in the search results.
			if (ReturnsRecordsParam("SELECT NULL FROM GiftCertificatesT WHERE ID = {INT} AND Voided = 1", m_nSelectedGCID)) {
				AfxMessageBox("The gift certificate you selected has been voided. Please choose a different gift certificate.");
				return;
			}
			
			m_nSelectedGCIDToTransfer = m_nSelectedGCID;
			GetDlgItemText(IDC_GC_CERTIFICATE_NUMBER, m_strSelectedGCNumberToTransfer);
		}

		//unregister for barcode messages
		if (GetMainFrame()) {
			if (!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CGCSearchDlg::OnCancel() 
{
	try {

		//unregister for barcode messages
		if (GetMainFrame()) {
			if (!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CGCSearchDlg, CNxDialog)
	ON_EVENT(CGCSearchDlg, IDC_GC_SEARCH_LIST, 16, OnSelChosenGcSearchList, VTS_DISPATCH)
END_EVENTSINK_MAP()

//look up history for the current m_nSelectedGCID
void CGCSearchDlg::PopulateHistory()
{
	// (j.jones 2015-03-31 09:40) - PLID 65390 - this function now goes off the m_nSelectedGCID
	try {

		if (m_nSelectedGCID == -1) {
			//no selection, no history
			m_HistoryList->Clear();
			return;
		}

		// (j.jones 2015-04-01 10:33) - PLID 65391 - the from clause is now filled in OnInitDialog,
		// all we need to do here is requery
		CString strWhereClause;
		strWhereClause.Format("GiftID = %li", m_nSelectedGCID);
		m_HistoryList->PutWhereClause(_bstr_t(strWhereClause));
		m_HistoryList->Requery();

	} NxCatchAll("Error populating history list.");
}

void CGCSearchDlg::OnPreviewGc() 
{
	try {

		if (m_nSelectedGCID == -1) {
			AfxMessageBox("You must select a Gift Certificate before previewing.");
			return;
		}
		
		CheckAndSaveExpiration();

		//preview the GC report
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(497)]);
		//set the filter for the ID to the current GC
		infReport.nExtraID = m_nSelectedGCID;

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, true, (CWnd *)this, "Gift Certificate");
		
		OnOK();

	} NxCatchAll("Error viewing gift certificate report.");
}

LRESULT CGCSearchDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		CheckAndSaveExpiration();

		// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
		CString strNumber = (LPCTSTR)_bstr_t((BSTR)lParam);

		// (j.jones 2005-11-10 11:01) - PLID 17710 - GiftCertificateIDs are now strings, albeit numeric only

		//we want to select the barcode they scanned (if we can find it)
		
		// (j.jones 2015-03-31 10:41) - PLID 65488 - search the API for this GC
		long nFoundGCID = -1;
		if (!strNumber.IsEmpty()) {
			NexTech_Accessor::_FindGiftCertificateGeneralListResultPtr pResult =
				GetAPI()->GetGiftCertificateInfo_GeneralList(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strNumber),
					IsDlgButtonChecked(IDC_FILTER_HIDE_BALANCES) ? VARIANT_TRUE : VARIANT_FALSE,
					IsDlgButtonChecked(IDC_FILTER_HIDE_EXPIRED) ? VARIANT_TRUE : VARIANT_FALSE,
					IsDlgButtonChecked(IDC_FILTER_HIDE_VOIDED) ? VARIANT_TRUE : VARIANT_FALSE);

			if (pResult != NULL && pResult->Result != NULL) {

				m_dtExpires.EnableWindow(FALSE);
				m_strNewExp = ""; m_strAuditOldVal = ""; m_strAuditNewVal = "";
				m_bExpChanged = false;

				m_nSelectedGCID = (long)atoi(VarString(pResult->Result->ID));

				SetDlgItemText(IDC_GC_CERTIFICATE_NUMBER, (LPCTSTR)pResult->Result->GiftID);
				SetDlgItemText(IDC_GC_TYPE, (LPCTSTR)pResult->Result->TypeName);
				SetDlgItemText(IDC_PURCH_BY, (LPCTSTR)pResult->Result->PurchasedBy);
				SetDlgItemText(IDC_REC_BY, (LPCTSTR)pResult->Result->ReceivedBy);
				SetDlgItemText(IDC_GC_AMT, FormatCurrencyForInterface(AsCurrency(pResult->Result->TotalValue)));
				SetDlgItemText(IDC_GC_USED, FormatCurrencyForInterface(AsCurrency(pResult->Result->AmtUsed)));
				SetDlgItemText(IDC_GC_BALANCE, FormatCurrencyForInterface(AsCurrency(pResult->Result->Balance)));
				COleDateTime dtPurchasedDate = pResult->Result->PurchaseDate;
								
				//convert to a bit
				int iVoided = VarByte(pResult->Result->Voided, 0);
				if (iVoided != 0) {
					if (lastGCVoid == 0) {
						lastGCVoid = 1;
						SetDlgItemText(IDC_VOID_GC, "Un&void");
					}
				}
				else {
					if (lastGCVoid == 1) {
						lastGCVoid = 0;
						SetDlgItemText(IDC_VOID_GC, "&Void Certificate");
					}
				}

				//Exp. Date can be null
				_variant_t varExpDate = pResult->Result->ExpDate->GetValueOrDefault(g_cvarNull);
				if (varExpDate.vt == VT_NULL) {
					COleDateTime dtStdExp = dtPurchasedDate;
					dtStdExp.SetDateTime(dtStdExp.GetYear() + 1, dtStdExp.GetMonth(), dtStdExp.GetDay(),
						dtStdExp.GetHour(), dtStdExp.GetMinute(), dtStdExp.GetSecond());
					m_dtExpires.SetValue(_variant_t(dtStdExp));
				}
				else {
					m_dtExpires.SetValue(varExpDate);
				}
				m_dtPurchase.SetValue(_variant_t(dtPurchasedDate));

				//setup the history datalist
				PopulateHistory();

				return 0;
			}
		}

		//if we're still here, nothing was found
		MessageBox("Gift Certificate not found!", "Practice", MB_ICONINFORMATION | MB_OK);
		return 0;

	}NxCatchAll(__FUNCTION__);

	return 0;
}

void CGCSearchDlg::OnFilterHideBalances() 
{
	try {

		CheckAndSaveExpiration();
		UpdateSearchListFilters();

		//PLID 11999 - remember this setting (per user)
		int nBalance = 0;
		if(IsDlgButtonChecked(IDC_FILTER_HIDE_BALANCES))
			nBalance = 1;

		SetRemotePropertyInt("GCHideBalance", nBalance, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

void CGCSearchDlg::OnFilterHideExpired() 
{
	try {

		CheckAndSaveExpiration();
		UpdateSearchListFilters();

		//PLID 11999 - remember this setting (per user)
		int nExp = 0;
		if(IsDlgButtonChecked(IDC_FILTER_HIDE_EXPIRED))
			nExp = 1;

		SetRemotePropertyInt("GCHideExpired", nExp, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

void CGCSearchDlg::OnFilterHideVoided() 
{
	try {

		CheckAndSaveExpiration();
		UpdateSearchListFilters();
	
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-31 09:42) - PLID 65390 - renamed to indicate this
// only updates the search properties, nothing is actually reloaded
void CGCSearchDlg::UpdateSearchListFilters()
{
	try {
		
		// (j.jones 2015-03-25 15:10) - PLID 65390 - change our search logic to respect the filters
		GiftCertificateSearchUtils::UpdateGeneralResultList(m_SearchList, GetRemoteData(),
			IsDlgButtonChecked(IDC_FILTER_HIDE_BALANCES) ? true : false, IsDlgButtonChecked(IDC_FILTER_HIDE_EXPIRED) ? true : false, IsDlgButtonChecked(IDC_FILTER_HIDE_VOIDED) ? true : false);

	} NxCatchAll(__FUNCTION__);
}

void CGCSearchDlg::OnGoToPurchaser() 
{
	try {

		if (m_nSelectedGCID == -1) {
			return;
		}

		CheckAndSaveExpiration();

		//now load the current patient
		_RecordsetPtr prs = CreateParamRecordset("SELECT PurchasedBy FROM GiftCertificatesT WHERE ID = {INT}", m_nSelectedGCID);
		long nID = AdoFldLong(prs, "PurchasedBy", -1);

		if(nID == -1)
			return;

		SetCurrentPatient(nID);

		//handles destruction and closing dialog
		OnOK();

	} NxCatchAll("Error loading receiver");
}

void CGCSearchDlg::OnGoToReceiver() 
{
	try {

		if (m_nSelectedGCID == -1) {
			return;
		}

		CheckAndSaveExpiration();

		//now load the current patient
		_RecordsetPtr prs = CreateParamRecordset("SELECT ReceivedBy FROM GiftCertificatesT WHERE ID = {INT}", m_nSelectedGCID);
		long nID = AdoFldLong(prs, "ReceivedBy", -1);

		if(nID == -1)
			return;

		SetCurrentPatient(nID);

		//handles destruction and closing dialog
		OnOK();

	} NxCatchAll("Error loading receiver");
}

void CGCSearchDlg::SetCurrentPatient(long nID)
{
	try {
		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if (nID != GetActivePatientID()) {
			if(!p->m_patToolBar.DoesPatientExistInList(nID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(!p->m_patToolBar.TrySetActivePatientID(nID)) {
				return;
			}
		}	

		if(p->FlipToModule(PATIENT_MODULE_NAME)) {

			pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
			if (pView) 
			{	if(pView->GetActiveTab()!=0)
					pView->SetActiveTab(0);
			}

			p->UpdateAllViews();
		}
	} NxCatchAll("Error loading patient.");
}

void CGCSearchDlg::OnVoidGc() 
{	
	try {

		// (z.manning 2011-09-16 15:44) - PLID 35376 - New permison for void gift certificates
		// (r.gonet 2015-03-20 16:47) - PLID 65270 - Renamed the permission from "Gift Certificates" to "Void Gift Certificates"
		if (!CheckCurrentUserPermissions(bioVoidGiftCertificates, sptDynamic0)) {
			return;
		}

		CheckAndSaveExpiration();

		int nVoid = 1; // set to void

		if (m_nSelectedGCID == -1) {
			AfxMessageBox("You must select a Gift Certificate before voiding.");
			return; // do nothing if no GC selected
		}

		bool bVoided = false;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Voided FROM GiftCertificatesT WHERE ID = {INT}", m_nSelectedGCID);
		if (!rs->eof) {
			bVoided = VarBool(rs->Fields->Item["Voided"]->Value) ? true : false;
		}
		rs->Close();

		if (bVoided) {
			nVoid = 0; // already void, set to deVoid
			if(IDCANCEL == MessageBox("The current gift certificate will be re-enabled.","Practice",MB_ICONEXCLAMATION|MB_OKCANCEL)) {
				return; // cancel the deVoid operation
			}
		}
		else if(IDCANCEL == MessageBox("The current gift certificate will be voided.","Practice",MB_ICONEXCLAMATION|MB_OKCANCEL)) {
			return; // cancel the Void operation
		}

		// get current gift ID
		CString strGiftID = m_nxeditGCNumber.GetText();

		// flag as void or not
		ExecuteParamSql("UPDATE GiftCertificatesT SET Voided = {INT} WHERE ID = {INT}", nVoid, m_nSelectedGCID);

		// add voided action to audit trail
		CString oldVal, newVal;
		if (nVoid == 1) {
			newVal.Format("%s (%s)", AUDIT_VOIDED, strGiftID);
			oldVal = AUDIT_ACTIVE;
		}
		else {
			newVal.Format("%s (%s)", AUDIT_ACTIVE, strGiftID);
			oldVal = AUDIT_VOIDED;
		}

		long nAuditID = BeginNewAuditEvent();
		// (a.walling 2010-01-21 15:55) - PLID 37023
		AuditEvent(VarLong(GetTableField("GiftCertificatesT", "PurchasedBy", "ID", m_nSelectedGCID)), m_nxeditPurchBy.GetText(), nAuditID, aeiGiftCStatus, m_nSelectedGCID, oldVal, newVal, aepHigh, aetChanged);

		//update the interface
		if (nVoid == 1) {
			if (lastGCVoid == 0) {
				lastGCVoid = 1;
				SetDlgItemText(IDC_VOID_GC, "Un&void");
			}
		}
		else {
			if (lastGCVoid == 1) {
				lastGCVoid = 0;
				SetDlgItemText(IDC_VOID_GC, "&Void Certificate");
			}
		}

		// (j.jones 2015-04-01 11:29) - PLID 65391 - reload the history to reflect this change
		PopulateHistory();

	} NxCatchAll("Error during void operation.");

}


// a.walling confirm they would like to change the date, then save and audit.
void CGCSearchDlg::OnChangeSearchExpDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {

		*pResult = 0;

		if (!m_bEditable || m_nSelectedGCID == -1) {
			return;
		}

		m_bExpChanged = true;

		CString strOldExp;

		CString strGiftID = m_nxeditGCNumber.GetText();

		_variant_t dtNewExp = m_dtExpires.GetValue();

		if (dtNewExp.vt != VT_NULL) {
			m_strNewExp.Format("'%s'", FormatDateTimeForSql(dtNewExp));
			m_strAuditNewVal.Format("Changed to %s (%s)", m_strNewExp, strGiftID);
		}
		else {
			m_strNewExp = "NULL";
			m_strAuditNewVal.Format("No expiration (%s)", strGiftID);
		}

		_variant_t varOldDT = g_cvarNull;
		_RecordsetPtr rs = CreateParamRecordset("SELECT ExpDate FROM GiftCertificatesT WHERE ID = {INT}", m_nSelectedGCID);
		if (!rs->eof) {
			varOldDT = rs->Fields->Item["ExpDate"]->Value;
		}
		rs->Close();
		if (varOldDT.vt == VT_DATE) {
			strOldExp = FormatDateTimeForSql(VarDateTime(varOldDT), dtoDate);
			m_strAuditOldVal.Format("Expires on '%s' (%s)", strOldExp, strGiftID);
		}
		else {
			// strOldExp = "Never";
			m_strAuditOldVal.Format("No expiration (%s)", strGiftID);
		}

	}NxCatchAll(__FUNCTION__);
}

void CGCSearchDlg::CheckAndSaveExpiration()
{
	try {

		if (m_bExpChanged) {
			if (IDYES != MsgBox(MB_YESNO, "The expiration date has changed. Would you like to save?") )
			{
				m_strNewExp = ""; m_strAuditOldVal = ""; m_strAuditNewVal = "";
			}
			else
			{
				if (m_nSelectedGCID == -1) {
					ThrowNxException("No gift certificate is selected!");
					return;
				}

				ExecuteParamSql("UPDATE GiftCertificatesT SET ExpDate = {CONST_STR} WHERE ID = {INT}", m_strNewExp, m_nSelectedGCID);

				long nAuditID = BeginNewAuditEvent();
				// (a.walling 2010-01-21 15:55) - PLID 37023
				AuditEvent(VarLong(GetTableField("GiftCertificatesT", "PurchasedBy", "ID", m_nSelectedGCID)), m_nxeditPurchBy.GetText(), nAuditID, aeiGiftCExtended, m_nSelectedGCID, m_strAuditOldVal, m_strAuditNewVal, aepHigh, aetChanged);
			}
		}

		// reset this even if they decide not to change the date.
		m_bExpChanged = false;

	} NxCatchAll("Error while extending gift certificate expiration.");
}

void CGCSearchDlg::OnEditexp() 
{
	try {

		if (m_nSelectedGCID == -1) {
			return;
		}

		m_dtExpires.EnableWindow(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-25 15:02) - PLID 65390 - converted the dropdown to a search list
void CGCSearchDlg::OnSelChosenGcSearchList(LPDISPATCH lpRow)
{
	try {

		// (j.jones 2015-03-31 09:34) - PLID 65390 - this code is modified somewhat
		// from the old dropdown logic, but largely similar

		//always save any changes to the exp. date
		CheckAndSaveExpiration();

		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		//make sure they didn't select the "no results" row
		long nID = VarLong(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscID), GC_NO_RESULTS_ROW);
		if (nID == GC_NO_RESULTS_ROW) {
			return;
		}

		m_nSelectedGCID = nID;

		// (a.walling 2006-07-03 11:43) - PLID 20113 Reset the vars and disable the expiration control.
		m_dtExpires.EnableWindow(FALSE);
		m_strNewExp = ""; m_strAuditOldVal = ""; m_strAuditNewVal = "";
		m_bExpChanged = false;

		// (j.jones 2015-03-31 09:20) - PLID 65390 - added GC # field
		SetDlgItemText(IDC_GC_CERTIFICATE_NUMBER, VarString(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscGiftID), ""));
		SetDlgItemText(IDC_GC_TYPE, VarString(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscTypeName), ""));
		SetDlgItemText(IDC_PURCH_BY, VarString(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscPurchasedBy), ""));
		SetDlgItemText(IDC_REC_BY, VarString(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscReceivedBy), ""));
		SetDlgItemText(IDC_GC_AMT, FormatCurrencyForInterface(VarCurrency(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscTotalValue), COleCurrency(0, 0))));
		SetDlgItemText(IDC_GC_USED, FormatCurrencyForInterface(VarCurrency(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscAmtUsed), COleCurrency(0, 0))));
		SetDlgItemText(IDC_GC_BALANCE, FormatCurrencyForInterface(VarCurrency(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscBalance), COleCurrency(0, 0))));

		if (VarBool(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscVoided), FALSE)) {
			if (lastGCVoid == 0) {
				lastGCVoid = 1;
				SetDlgItemText(IDC_VOID_GC, "Un&void");
			}
		}
		else {
			if (lastGCVoid == 1) {
				lastGCVoid = 0;
				SetDlgItemText(IDC_VOID_GC, "&Void Certificate");
			}
		}

		_variant_t vtExpiration = pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscExpireDate);
		if (vtExpiration.vt == VT_NULL) {
			COleDateTime dtStdExp = VarDateTime(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscPurchasedDate));
			dtStdExp.SetDateTime(dtStdExp.GetYear() + 1, dtStdExp.GetMonth(), dtStdExp.GetDay(),
				dtStdExp.GetHour(), dtStdExp.GetMinute(), dtStdExp.GetSecond());
			m_dtExpires.SetValue(_variant_t(dtStdExp));
		}
		else {
			m_dtExpires.SetValue(vtExpiration);
		}
		m_dtPurchase.SetValue(_variant_t(pRow->GetValue(GiftCertificateSearchUtils::GCGeneralSearchColumns::gcgscPurchasedDate)));

		//setup the history datalist
		PopulateHistory();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-04-24 10:15) - PLID 65710 - added ability to transfer balances
void CGCSearchDlg::OnBtnTransferGCBalance()
{
	try {

		//check permissions
		if (!CheckCurrentUserPermissions(bioTransferGiftCertificateBalances, sptWrite)) {
			return;
		}

		if (m_nSelectedGCID == -1) {
			AfxMessageBox("You must select a Gift Certificate before transferring a balance.");
			return;
		}

		//get the currently displayed balance - this is the amount they intend to transfer
		CString strBalance;
		GetDlgItemText(IDC_GC_BALANCE, strBalance);
		COleCurrency cyBalance = COleCurrency(0, 0);
		if (!cyBalance.ParseCurrency(strBalance) || cyBalance == COleCurrency(0, 0)) {
			AfxMessageBox("The current Gift Certificate does not have a balance.");
			return;
		}

		CString strCertificateID;
		GetDlgItemText(IDC_GC_CERTIFICATE_NUMBER, strCertificateID);

		enum {
			eExistingGC = 1,
			eNewGC,
		};

		CMenu menu;
		menu.m_hMenu = CreatePopupMenu();
		menu.InsertMenu(0, MF_BYPOSITION, eExistingGC, "Transfer Balance To &Existing Gift Certificate...");
		menu.InsertMenu(1, MF_BYPOSITION, eNewGC, "Transfer Balance To &New Gift Certificate...");

		CRect rc;
		GetDlgItem(IDC_TRANSFER_GC_BALANCE)->GetWindowRect(&rc);

		// (r.gonet 2015-05-11 10:46) - PLID 65392 - Track whether the transfer succeeded.
		bool bTransferred = false;
		long nResult = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		menu.DestroyMenu();
		switch (nResult) {
		case eExistingGC:
			bTransferred = TransferBalanceToExistingGC(m_nSelectedGCID, strCertificateID, cyBalance);
			break;
		case eNewGC:
			bTransferred = TransferBalanceToNewGC(m_nSelectedGCID, strCertificateID, cyBalance);
			break;
		default:
			break;
		}

		// (r.gonet 2015-05-11 10:46) - PLID 65392 - If we transferred to another gift certificate, ask to void the original.
		if (bTransferred) {
			long nPurchasedByPersonID = -1;
			CString strPurchasedByFullName;
			bool bVoided = false;
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT GiftCertificatesT.GiftID, GiftCertificatesT.PurchasedBy, PersonT.FullName, GiftCertificatesT.Voided "
				"FROM GiftCertificatesT "
				"INNER JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
				"WHERE GiftCertificatesT.ID = {INT}", m_nSelectedGCID);
			if (!rs->eof) {
				long nPurchasedByPersonID = AdoFldLong(rs->Fields, "PurchasedBy");
				CString strPurchasedByFullName = AdoFldString(rs->Fields, "FullName");
				bVoided = AdoFldBool(rs->Fields, "Voided") ? true : false;
			}
			rs->Close();

			// (r.gonet 2015-05-11 10:46) - PLID 65392 - Void the original if they desire.
			if (!bVoided && 
				GetCurrentUserPermissions(bioVoidGiftCertificates, sptDynamic0) &&
				IDYES == MessageBox("Would you like to void the original gift certificate?", "Void Original", MB_YESNO | MB_ICONQUESTION)) {
				// flag as void
				ExecuteParamSql("UPDATE GiftCertificatesT SET Voided = 1 WHERE ID = {INT}", m_nSelectedGCID);

				//update the interface
				if (lastGCVoid == 0) {
					lastGCVoid = 1;
					SetDlgItemText(IDC_VOID_GC, "Un&void");
				}

				_RecordsetPtr prsSourceGiftInfo = CreateParamRecordset(
					"SELECT GiftCertificatesT.GiftID, GiftCertificatesT.PurchasedBy, PersonT.FullName "
					"FROM GiftCertificatesT "
					"INNER JOIN PersonT ON GiftCertificatesT.PurchasedBy = PersonT.ID "
					"WHERE GiftCertificatesT.ID = {INT}",
					m_nSelectedGCID);
				if (!prsSourceGiftInfo->eof) {

					// add voided action to audit trail
					CString newVal = FormatString("%s (%s)", AUDIT_VOIDED, strCertificateID);
					CString oldVal = AUDIT_ACTIVE;

					long nAuditID = BeginNewAuditEvent();
					AuditEvent(nPurchasedByPersonID, strPurchasedByFullName, nAuditID, aeiGiftCStatus, m_nSelectedGCID, oldVal, newVal, aepHigh, aetChanged);
				}

				PopulateHistory();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-04-24 10:15) - PLID 65714 - added ability to transfer balances
// (r.gonet 2015-05-11 10:46) - PLID 65392 - Now returns whether or not the transfer succeeded.
bool CGCSearchDlg::TransferBalanceToNewGC(long nSourceGiftID, CString strSourceGiftCertNumber, COleCurrency cyAmtToTransfer)
{
	try {

		//check permissions for transferring
		if (!CheckCurrentUserPermissions(bioTransferGiftCertificateBalances, sptWrite)) {
			return false;
		}

		//check permissions for new bills - required for GC creation
		if (!CheckCurrentUserPermissions(bioBill, sptCreate)) {
			return false;
		}

		// (j.jones 2015-04-24 10:33) - PLID 65714 - supported creating a new GC to transfer into

		//warn and have them the confirm the transfer prior to creating a new GC
		CString strMsg;
		strMsg.Format("You are about to transfer %s from the balance of Gift Certificate %s to a new Gift Certificate.\n\n"
			"Are you sure you wish to do this?", FormatCurrencyForInterface(cyAmtToTransfer), strSourceGiftCertNumber);
		if (IDNO == MessageBox(strMsg, "Practice", MB_ICONQUESTION | MB_YESNO)) {
			return false;
		}

		//query the old GCs purchased by patient, provider, location, and received patient (if any)
		long nProviderID = -1, nLocationID = -1;
		long nPurchasedPatientID = -1;
		long nReceivedPatientID = -1;
		_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.DoctorsProviders, GiftCertificatesT.LocationID, "
			"GiftCertificatesT.PurchasedBy, GiftCertificatesT.ReceivedBy "
			"FROM GiftCertificatesT "
			"LEFT JOIN (SELECT Min(ID) AS FirstID, GiftID "
			"	FROM LineItemT "
			"	WHERE Deleted = 0 AND Type = 10 "
			"	GROUP BY GiftID) AS FirstChargeQ ON GiftCertificatesT.ID = FirstChargeQ.GiftID "
			"LEFT JOIN ChargesT ON FirstChargeQ.FirstID = ChargesT.ID "
			"WHERE GiftCertificatesT.ID = {INT}", nSourceGiftID);
		if (!rs->eof) {
			nProviderID = VarLong(rs->Fields->Item["DoctorsProviders"]->Value, -1);
			nLocationID = VarLong(rs->Fields->Item["LocationID"]->Value, -1);
			nPurchasedPatientID = VarLong(rs->Fields->Item["PurchasedBy"]->Value, -1);
			nReceivedPatientID = VarLong(rs->Fields->Item["ReceivedBy"]->Value, -1);
		}
		rs->Close();


		//create a new gift certificate

		long nDestGiftID = -1;
		CString strDestGiftCertNumber;

		//Open the gift certificate entry dialog displaying the Transfer Amount as the Value and the Price as 0.00. 
		//It's really going to make a $0.00 value GC, the transfer amount is just for display purposes.
		bool bGCSuccess = CreateNewGiftCertificate(this, GCCreationStyle::eTransferToNewGC, nDestGiftID, strDestGiftCertNumber,
			"", -1, g_ccyZero, cyAmtToTransfer, g_ccyZero, nProviderID, nLocationID, COleDateTime::GetCurrentTime(), false, g_cdtNull,
			nPurchasedPatientID, nReceivedPatientID != -1, nReceivedPatientID);
		if (bGCSuccess) {
			//now transfer - this function will give a success message when the transfer completes
			TransferBalanceToGC(nSourceGiftID, strSourceGiftCertNumber, nDestGiftID, strDestGiftCertNumber, cyAmtToTransfer);
			return true;
		}
		else {
			return false;
		}

	}NxCatchAll(__FUNCTION__);
	return false;
}

// (j.jones 2015-04-24 10:15) - PLID 65710 - added ability to transfer balances
// (r.gonet 2015-05-11 10:46) - PLID 65392 - Now returns whether or not the transfer succeeded.
bool CGCSearchDlg::TransferBalanceToExistingGC(long nSourceGiftID, CString strSourceGiftCertNumber, COleCurrency cyAmtToTransfer)
{
	try {

		//check permissions
		if (!CheckCurrentUserPermissions(bioTransferGiftCertificateBalances, sptWrite)) {
			return false;
		}

		//pop open a new search with an OK/Cancel button
		while (true) {

			CGCSearchDlg dlg(this);
			dlg.m_bIsTransferring = true;
			if (dlg.DoModal() == IDCANCEL) {
				return false;
			}

			long nDestGiftID = dlg.m_nSelectedGCIDToTransfer;
			if (nDestGiftID == -1) {
				//the code shouldn't have allowed this
				ASSERT(FALSE);
				AfxMessageBox("You must select a Gift Certificate for the balance transfer.");

				//reopen the dialog
				continue;
			}

			if (nSourceGiftID == nDestGiftID) {
				AfxMessageBox("You cannot transfer a Gift Certificate into itself.");
				//reopen the dialog
				continue;
			}

			CString strDestGiftCertNumber = dlg.m_strSelectedGCNumberToTransfer;

			//now warn and have them the confirm the transfer
			CString strMsg;
			strMsg.Format("You are about to transfer %s from the balance of Gift Certificate %s to Gift Certificate %s.\n\n"
				"Are you sure you wish to do this?", FormatCurrencyForInterface(cyAmtToTransfer), strSourceGiftCertNumber, strDestGiftCertNumber);
			if (IDNO == MessageBox(strMsg, "Practice", MB_ICONQUESTION | MB_YESNO)) {
				//reopen the dialog
				continue;
			}

			//if they get here, they picked a GC, so do so now, then leave this function

			//this function will give a success message when the transfer completes
			TransferBalanceToGC(nSourceGiftID, strSourceGiftCertNumber, nDestGiftID, strDestGiftCertNumber, cyAmtToTransfer);
			return true;
		}

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2015-04-24 10:21) - PLID 65711 - shared function to transfer balances
void CGCSearchDlg::TransferBalanceToGC(long nSourceGiftID, CString strSourceGiftCertNumber,
	long nDestGiftID, CString strDestGiftCertNumber, COleCurrency cyAmtToTransfer)
{
	//throw exceptions to the caller

	CWaitCursor pWait;

	TransferGiftCertificateAmount(nSourceGiftID, nDestGiftID, cyAmtToTransfer);

	//reload this GC to reflect the balance and history
	ReloadCurrentGiftCertificateBalance();

	CString strMsg;
	strMsg.Format("%s has been successfully transferred from Gift Certificate %s to Gift Certificate %s.",
		FormatCurrencyForInterface(cyAmtToTransfer), strSourceGiftCertNumber, strDestGiftCertNumber);
	MessageBox(strMsg, "Practice", MB_ICONINFORMATION | MB_OK);
}

// (j.jones 2015-04-24 11:16) - PLID 65710 - added function to reload the current GC
void CGCSearchDlg::ReloadCurrentGiftCertificateBalance()
{
	try {

		CString strCertificateID;
		GetDlgItemText(IDC_GC_CERTIFICATE_NUMBER, strCertificateID);

		if (m_nSelectedGCID == -1 || strCertificateID.IsEmpty()) {
			//why did any code call this function with no GC selected?
			ASSERT(FALSE);
			return;
		}

		//disable all filters on this lookup, it may have a zero balance now
		NexTech_Accessor::_FindGiftCertificateGeneralListResultPtr pResult =
			GetAPI()->GetGiftCertificateInfo_GeneralList(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strCertificateID),
				VARIANT_FALSE, VARIANT_FALSE, VARIANT_FALSE);

		if (pResult != NULL && pResult->Result != NULL) {
			//we only need to reload the currencies
			SetDlgItemText(IDC_GC_AMT, FormatCurrencyForInterface(AsCurrency(pResult->Result->TotalValue)));
			SetDlgItemText(IDC_GC_USED, FormatCurrencyForInterface(AsCurrency(pResult->Result->AmtUsed)));
			SetDlgItemText(IDC_GC_BALANCE, FormatCurrencyForInterface(AsCurrency(pResult->Result->Balance)));
		}
		else {
			//should not be possible
			ASSERT(FALSE);
			ThrowNxException("Failed to reload the balance of certificate ID %s", strCertificateID);
		}

		PopulateHistory();

	}NxCatchAll(__FUNCTION__);
}