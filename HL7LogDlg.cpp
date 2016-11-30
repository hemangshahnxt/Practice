// HL7LogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "HL7LogDlg.h"
#include "HL7Utils.h"
#include <NxHL7Lib/HL7Logging.h>
#include "FinancialRc.h"
#include "PracticeRc.h"

using namespace ADODB;

// CHL7LogDlg dialog

IMPLEMENT_DYNAMIC(CHL7LogDlg, CNxDialog)

// (r.gonet 05/01/2014) - PLID 49432 - Constructs a new CHL7LogDlg dialog
CHL7LogDlg::CHL7LogDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7LogDlg::IDD, pParent)
{
	m_hIconScriptAttention = NULL;
	m_hIconError = NULL;
	m_hIconWarning = NULL;
	m_hIconInfo = NULL;
	m_hIconDiagnostic = NULL;
	m_hIconMessage = NULL;
}

// (r.gonet 05/01/2014) - PLID 49432 - Frees up any resources (such as icons) used by the CHL7LogDlg
CHL7LogDlg::~CHL7LogDlg()
{
	// (r.gonet 05/01/2014) - PLID 49432 - Free the icons
	if(m_hIconScriptAttention) {
		DestroyIcon(m_hIconScriptAttention);
	}
	if(m_hIconError) {
		DestroyIcon(m_hIconError);
	}
	if(m_hIconWarning) {
		DestroyIcon(m_hIconWarning);
	}
	if(m_hIconInfo) {
		DestroyIcon(m_hIconInfo);
	}
	if(m_hIconDiagnostic) {
		DestroyIcon(m_hIconDiagnostic);
	}
	if(m_hIconMessage) {
		DestroyIcon(m_hIconMessage);
	}
	m_hIconScriptAttention = NULL;
	m_hIconError = NULL;
	m_hIconWarning = NULL;
	m_hIconInfo = NULL;
	m_hIconDiagnostic = NULL;
	m_hIconMessage = NULL;
}

void CHL7LogDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HL7_LOG_HEADER_STATIC, m_nxsHeaderLabel);
	DDX_Control(pDX, IDC_HL7_GROUP_NAME_LBL, m_nxsHL7GroupLabel);
	DDX_Control(pDX, IDC_HL7_FROM_FILTER_CHECK, m_checkFrom);
	DDX_Control(pDX, IDC_HL7_LOG_BEGIN_FILTER_DATETIME, m_dtFrom);
	DDX_Control(pDX, IDC_HL7_TO_FILTER_CHECK, m_checkTo);
	DDX_Control(pDX, IDC_HL7_LOG_END_FILTER_DATETIME, m_dtTo);
	DDX_Control(pDX, IDC_SHOW_ONLY_NONCOMMITTED_CHECK, m_checkOnlyShowNonCommitted);
	DDX_Control(pDX, IDC_SHOW_ONLY_ERROR_TRANSACTIONS_CHECK, m_checkOnlyShowErrorTransactions);
	DDX_Control(pDX, IDC_LEVEL_HEADER, m_nxsLevelsLabel);
	DDX_Control(pDX, IDC_SHOW_ERRORS_CHECK, m_checkErrors);
	DDX_Control(pDX, IDC_SHOW_WARNINGS_CHECK, m_checkWarnings);
	DDX_Control(pDX, IDC_SHOW_INFO_CHECK, m_checkInformation);
	DDX_Control(pDX, IDC_SHOW_DIAGNOSTIC_CHECK, m_checkDiagnostic);
	DDX_Control(pDX, IDC_HL7_LOG_REFRESH_BTN, m_nxbRefresh);
	DDX_Control(pDX, IDOK, m_nxbClose);
}


BEGIN_MESSAGE_MAP(CHL7LogDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SHOW_ERRORS_CHECK, &CHL7LogDlg::OnBnClickedShowErrorsCheck)
	ON_BN_CLICKED(IDC_SHOW_WARNINGS_CHECK, &CHL7LogDlg::OnBnClickedShowWarningsCheck)
	ON_BN_CLICKED(IDC_SHOW_INFO_CHECK, &CHL7LogDlg::OnBnClickedShowInfoCheck)
	ON_BN_CLICKED(IDC_SHOW_DIAGNOSTIC_CHECK, &CHL7LogDlg::OnBnClickedShowDiagnosticCheck)
	ON_BN_CLICKED(IDC_HL7_FROM_FILTER_CHECK, &CHL7LogDlg::OnBnClickedHl7FromFilterCheck)
	ON_BN_CLICKED(IDC_HL7_TO_FILTER_CHECK, &CHL7LogDlg::OnBnClickedHl7ToFilterCheck)
	ON_BN_CLICKED(IDC_SHOW_ONLY_NONCOMMITTED_CHECK, &CHL7LogDlg::OnBnClickedShowOnlyNoncommittedCheck)
	ON_BN_CLICKED(IDC_HL7_LOG_REFRESH_BTN, &CHL7LogDlg::OnBnClickedHl7LogRefreshBtn)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_HL7_LOG_BEGIN_FILTER_DATETIME, &CHL7LogDlg::OnDtnDatetimechangeHl7LogBeginFilterDatetime)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_HL7_LOG_END_FILTER_DATETIME, &CHL7LogDlg::OnDtnDatetimechangeHl7LogEndFilterDatetime)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SHOW_ONLY_ERROR_TRANSACTIONS_CHECK, &CHL7LogDlg::OnBnClickedShowOnlyErrorTransactionsCheck)
END_MESSAGE_MAP()


// CHL7LogDlg message handlers

// (r.gonet 05/01/2014) - PLID 49432
BOOL CHL7LogDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxbRefresh.AutoSet(NXB_REFRESH);
		m_nxbClose.AutoSet(NXB_CLOSE);

		// (r.gonet 05/01/2014) - PLID 49432 - Set the title bar icon
		m_hIconScriptAttention = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_SCRIPT_ATTENTION), IMAGE_ICON, 16, 16, 0);
		SetIcon(m_hIconScriptAttention, FALSE);

		// (r.gonet 05/01/2014) - PLID 49432 - Load the other icons we'll be using.
		m_hIconError = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CANCEL), IMAGE_ICON, 16, 16, 0);
		m_hIconWarning = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_WARNING_ICO), IMAGE_ICON, 16, 16, 0);
		m_hIconInfo = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INFO_ICON), IMAGE_ICON, 16, 16, 0);
		m_hIconDiagnostic = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_STETHOSCOPE), IMAGE_ICON, 16, 16, 0);
		m_hIconMessage = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_FILE), IMAGE_ICON, 16, 16, 0);

		// (r.gonet 05/01/2014) - PLID 49432 - Set the datetime filter to filter on the whole of today.
		COleDateTime dtNow(COleDateTime::GetCurrentTime());
		m_checkFrom.SetCheck(BST_CHECKED);
		m_dtFrom.EnableWindow(TRUE);
		m_checkTo.SetCheck(BST_CHECKED);
		m_dtTo.EnableWindow(TRUE);
		m_dtFrom.SetValue(COleDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0));
		m_dtTo.SetValue(COleDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0));
		// (r.gonet 05/01/2014) - PLID 49432 - Show only the transactions associated with messages
		// that are still uncommitted (these that have transactions have errors)
		m_checkOnlyShowNonCommitted.SetCheck(TRUE);
		m_checkOnlyShowErrorTransactions.SetCheck(TRUE);
		// (r.gonet 05/01/2014) - PLID 49432 - Show errors, warnings, and diagnostic by default
		m_checkErrors.SetCheck(BST_CHECKED);
		m_checkWarnings.SetCheck(BST_CHECKED);
		m_checkInformation.SetCheck(BST_CHECKED);
		if(GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			// (r.gonet 05/01/2014) - PLID 49432 - If they are Nextech, then show more technical details.
			m_checkDiagnostic.SetCheck(BST_CHECKED);
		}

		m_pHL7GroupsCombo = BindNxDataList2Ctrl(IDC_HL7_GROUP_COMBO);
		// (r.gonet 05/01/2014) - PLID 49432 - Need to wait on the requery because we'll use it right after.
		m_pHL7GroupsCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		// (r.gonet 05/01/2014) - PLID 49432 - Set the default group to be all groups
		NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pHL7GroupsCombo->FindByColumn(hgccID, (long)-2, NULL, VARIANT_TRUE);
		m_pHL7GroupsCombo->CurSel = pGroupRow;

		// (r.gonet 05/01/2014) - PLID 49432 - Set the instructional header text.
		m_nxsHeaderLabel.SetWindowText(
			"This screen displays the diagnostic messages that are internally logged whenever an HL7 lab result import transaction occurs. The top list shows each import transaction and the bottom list shows the errors, warnings, and other information related to that particular transaction. Logging verbosity can be configured in Configure HL7 Settings->Advanced Settings for each HL7 Settings Group.");

		m_pTransactions = BindNxDataList2Ctrl(IDC_HL7_TRANSACTION_LIST, false);
		// (r.gonet 05/01/2014) - PLID 49432 - HL7 Transaction query. Only support ORU^R01 and only imports for now.
		m_pTransactions->PutFromClause(_bstr_t(FormatString(
			"( "
				"SELECT "
					"HL7TransactionT.ID, "
					"HL7MessageQueueQ.GroupName As HL7GroupName, "
					"HL7TransactionT.TimeStamp AS TransactionTimeStamp, "
					"HL7TransactionT.Username, "
					"HL7MessageQueueQ.PatientName, "
					"(CASE WHEN HL7MessageQueueQ.PatientGender = 1 THEN 'Male' WHEN HL7MessageQueueQ.PatientGender = 2 THEN 'Female' END) AS PatientSex, "
					"HL7MessageQueueQ.PatientDOB, "
					"HL7MessageQueueQ.Description, "
					"COALESCE( "
						"CASE WHEN HL7MessageQueueQ.Action IS NOT NULL THEN HL7MessageQueueQ.Action "
							  "ELSE HL7MessageQueueQ.MessageType + '^' + HL7MessageQueueQ.EventType "
						 "END, 'Unknown Message Type') AS Action, "
					"HL7MessageQueueQ.InputDate AS MessageTime, "
					"HL7MessageQueueQ.GroupID AS HL7GroupID, "
					"HL7MessageQueueQ.ID AS ImportMessageID, "
					"CONVERT(BIT, COALESCE(ErrorsQ.HL7TransactionID, 0)) AS HasErrors, "
					"(CASE WHEN ErrorsQ.HL7TransactionID IS NOT NULL THEN %lu ELSE %lu END) AS BackColor, "
					"HL7MessageQueueQ.MessageType, "
					"HL7MessageQueueQ.EventType "
				"FROM HL7TransactionT "
				"LEFT JOIN "
				"( "
					"SELECT HL7LogT.HL7TransactionID "
					"FROM HL7LogT "
					"WHERE HL7LogT.Level = 1 "
					"GROUP BY HL7LogT.HL7TransactionID "
				") ErrorsQ ON HL7TransactionT.ID = ErrorsQ.HL7TransactionID "
				"LEFT JOIN "
				"( "
					"SELECT "
						"HL7MessageQueueT.ID, "
						"HL7MessageQueueT.GroupID, "
						"HL7SettingsT.Name AS GroupName, "
						"HL7MessageQueueT.MessageType, "
						"HL7MessageQueueT.EventType, "
						"HL7MessageQueueT.PatientName, "
						"HL7MessageQueueT.PatientGender, "
						"HL7MessageQueueT.PatientDOB, "
						"HL7MessageQueueT.Description, "
						"CASE WHEN MessageType = 'ORU' THEN "
							  "CASE WHEN EventType = 'R01' THEN 'Lab Result' "
								   "ELSE NULL "
							  "END "
							  "ELSE NULL "
						"END AS Action, "
						"HL7MessageQueueT.InputDate "
					"FROM HL7MessageQueueT "
					"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID "
					// Where we have the message , only show lab result messages, for now at least.
					"WHERE MessageType = 'ORU' AND EventType = 'R01' "
					// Don't show ORU^R01 messages that are actually patient history messages.
					"AND HL7SettingsT.ID NOT IN (SELECT HL7GenericSettingsT.HL7GroupID FROM HL7GenericSettingsT WHERE HL7GenericSettingsT.Name = 'UsePatientHistoryImportMSH_3' AND HL7GenericSettingsT.BitParam = 1) "
				") HL7MessageQueueQ ON HL7TransactionT.ImportMessageID = HL7MessageQueueQ.ID "
			") SubQ"
			, RGB(255, 185, 185), RGB(255, 255, 255))));
		NXDATALIST2Lib::IColumnSettingsPtr pMessageIconCol = m_pTransactions->GetColumn(etlcMessageIcon);
		if (pMessageIconCol != NULL && m_hIconMessage != NULL) {
			// (r.gonet 05/01/2014) - PLID 49432 - Add the message preview icon
			CString strHICON;
			strHICON.Format("%li", m_hIconMessage);
			pMessageIconCol->FieldName = (LPCTSTR)strHICON;
		}

		// (r.gonet 05/01/2014) - PLID 49432 - Put the from clause in
		// for the log entries list but don't load it up yet since it is
		// supposed to be dependent on the transaction list.
		m_pLogEntries = BindNxDataList2Ctrl(IDC_HL7_LOG_ENTRY_LIST, false);
		m_pLogEntries->PutFromClause(_bstr_t(
			"( "
				"SELECT "
					"HL7LogT.ID, "
					"HL7LogT.HL7TransactionID, "
					"CONVERT(VARCHAR, HL7LogT.TimeStamp, 14) AS TimeStamp, "
					"HL7LogT.Level, "
					"HL7LogT.Text, "
					"HL7LogT.FunctionName, "
					"HL7LogT.LineNumber "
				"FROM HL7LogT "
			") SubQ "));

		// (r.gonet 05/01/2014) - PLID 49432 - Hide the tech support only checkbox
		// and datalist columns
		ShowHideDiagnosticControls();

		// (r.gonet 05/01/2014) - PLID 49432 - Load up the transaction and log entry lists.
		FilterTransactionList();
		FilterLogEntryList();

		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pTransactions->GetFirstRow();
		if (pFirstRow) {
			m_pTransactions->CurSel = pFirstRow;
			m_pTransactions->EnsureRowInView(pFirstRow);
			FilterLogEntryList();
		}
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the errors checkbox
void CHL7LogDlg::OnBnClickedShowErrorsCheck()
{
	try {
		FilterLogEntryList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the warnings checkbox
void CHL7LogDlg::OnBnClickedShowWarningsCheck()
{
	try {
		FilterLogEntryList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the info checkbox
void CHL7LogDlg::OnBnClickedShowInfoCheck()
{
	try {
		FilterLogEntryList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the diagnostic checkbox
void CHL7LogDlg::OnBnClickedShowDiagnosticCheck()
{
	try {
		FilterLogEntryList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Hides from non-Nextech Technical Support Users the ability to show
// diagnostic/debug log entries and a couple more detailed columns in the log entry list.
void CHL7LogDlg::ShowHideDiagnosticControls()
{
	NXDATALIST2Lib::IColumnSettingsPtr pFunctionNameColumn = m_pLogEntries->GetColumn(lelcFunctionName);
	NXDATALIST2Lib::IColumnSettingsPtr pLineNumberColumn = m_pLogEntries->GetColumn(lelcLineNumber);

	// // (r.gonet 05/01/2014) - PLID 49432 - Show/Hide the Function Name/Line Number columns since 
	// that is useless and too technical to a standard user, but valuable for us.
	if(GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
		m_checkDiagnostic.ShowWindow(SW_SHOW);
		pFunctionNameColumn->StoredWidth = 120;
		pFunctionNameColumn->ColumnStyle = NXDATALIST2Lib::csVisible;
		pLineNumberColumn->StoredWidth = 50;
		pLineNumberColumn->ColumnStyle = NXDATALIST2Lib::csVisible;
	} else {
		m_checkDiagnostic.ShowWindow(SW_HIDE);
		pFunctionNameColumn->StoredWidth = -1;
		pFunctionNameColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
		pLineNumberColumn->StoredWidth = -1;
		pLineNumberColumn->ColumnStyle = NXDATALIST2Lib::csFixedWidth;
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on the group combo
CString CHL7LogDlg::CreateGroupFilter()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pHL7GroupsCombo->CurSel;
	if(pRow) {
		long nGroupID = VarLong(pRow->GetValue(hgccID), -2);
		if(nGroupID > 0) {
			return CSqlFragment(
				"(ImportMessageID IS NOT NULL AND ImportMessageID IN (SELECT ID FROM HL7MessageQueueT WHERE GroupID = {INT})) ",
				nGroupID).Flatten();
		} else {
			// -2 being all groups
			return "";
		}
	} else {
		// Somehow they managed to select a null row.
		ASSERT(FALSE);
		return "";
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on the date of the transaction
CString CHL7LogDlg::CreateTimeStampFilter()
{
	CString strFilter;
	if(m_checkFrom.GetCheck()) {
		COleDateTime dtFrom = VarDateTime(m_dtFrom.GetValue());
		// Hidden time of midnight
		dtFrom = COleDateTime(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay(), 0, 0, 0);
		if(dtFrom.GetStatus() == COleDateTime::valid) {
			strFilter += CSqlFragment("TransactionTimeStamp >= {OLEDATETIME}", dtFrom).Flatten(); 
		}
	}
	if(m_checkTo.GetCheck()) {
		COleDateTime dtTo = VarDateTime(m_dtTo.GetValue());
		// Adjust to be midnight of the next day
		dtTo = COleDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);
		dtTo += COleDateTimeSpan(1, 0, 0, 0);
		if(dtTo.GetStatus() == COleDateTime::valid) {
			if(!strFilter.IsEmpty()) strFilter += " AND ";
			strFilter += CSqlFragment("TransactionTimeStamp < {OLEDATETIME}", dtTo).Flatten(); 
		}
	}
	return strFilter;
}

// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on whether the HL7 message is committed or not.
CString CHL7LogDlg::CreateOnlyShowNonCommittedFilter()
{
	if(m_checkOnlyShowNonCommitted.GetCheck()) {
		CSqlFragment frag("(ImportMessageID IS NOT NULL AND ImportMessageID IN (SELECT ID FROM HL7MessageQueueT WHERE ActionID IS NULL))");
		return frag.Flatten();
	} else {
		return "";
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on whether the HL7 message is committed or not.
CString CHL7LogDlg::CreateOnlyShowErrorTransactionsFilter()
{
	if (m_checkOnlyShowErrorTransactions.GetCheck()) {
		CSqlFragment frag("HasErrors = 1");
		return frag.Flatten();
	} else {
		return "";
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL where clause for the log entry list that causes it to be filtered on the currently selected transaction
CString CHL7LogDlg::CreateTransactionFilter()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTransactions->CurSel;
	if(pRow) {
		CSqlFragment frag("(HL7TransactionID = {INT})", VarLong(pRow->GetValue(0)));
		return frag.Flatten();
	} else {
		// If no transaction is selected, make sure to have no log entry rows selected.
		return "1 <> 1";
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the log entry list that causes it to be filtered on the severity levels.
CString CHL7LogDlg::CreateLevelFilter()
{
	CArray<long, long> aryLevels;
	if(m_checkErrors.GetCheck()) {
		aryLevels.Add(elelError);
	}
	if(m_checkWarnings.GetCheck()) {
		aryLevels.Add(elelWarning);
	}
	if(m_checkInformation.GetCheck()) {
		aryLevels.Add(elelInfo);
	}
	if(m_checkDiagnostic.GetCheck()) {
		aryLevels.Add(elelDebug);
	}

	CSqlFragment frag("Level IN ({INTARRAY})", aryLevels);
	return frag.Flatten();
}

// (r.gonet 05/01/2014) - PLID 49432 - Filters the transaction list based on the various filter controls.
void CHL7LogDlg::FilterTransactionList()
{
	CString strFilter;
	// (r.gonet 05/01/2014) - PLID 49432 - Gather all the control filters as SQL conditions
	CArray<CString, CString> arFilters;
	arFilters.Add(CreateGroupFilter());
	arFilters.Add(CreateTimeStampFilter());
	arFilters.Add(CreateOnlyShowNonCommittedFilter());
	arFilters.Add(CreateOnlyShowErrorTransactionsFilter());

	// (r.gonet 05/01/2014) - PLID 49432 - Put together the various filters into a where clause
	for(int i = 0; i < arFilters.GetSize(); i++) {
		CString strTemp = arFilters.GetAt(i);
		if(!strFilter.IsEmpty() && strTemp != "") {
			strFilter += " AND ";
		}
		strFilter += strTemp;
	}

	// (r.gonet 05/01/2014) - PLID 49432 - Store to find the currently selected transaction again
	long nCurrentTransactionID = -1;
	NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pTransactions->CurSel;
	if(pCurrentRow) {
		nCurrentTransactionID = VarLong(pCurrentRow->GetValue(etlcID));
	}
	m_pTransactions->PutWhereClause(_bstr_t(strFilter));
	m_pTransactions->Requery();
	m_pTransactions->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	if(nCurrentTransactionID != -1) {
		// (r.gonet 05/01/2014) - PLID 49432 - Find the previously selected row
		pCurrentRow = m_pTransactions->FindByColumn(etlcID, _variant_t(nCurrentTransactionID, VT_I4), m_pTransactions->FindAbsoluteFirstRow(VARIANT_FALSE), VARIANT_TRUE);
		if(!pCurrentRow) {
			// (r.gonet 05/01/2014) - PLID 49432 - The transaction is no longer being shown, so clear the log entries list.
			m_pLogEntries->Clear();
		}
	} else {
		// (r.gonet 05/01/2014) - PLID 49432 - No transaction row was selected, clear the log entries list.
		m_pLogEntries->Clear();
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Filters the log entry list based on the level checkboxes.
void CHL7LogDlg::FilterLogEntryList()
{
	CString strFilter;
	// (r.gonet 05/01/2014) - PLID 49432 - Gather the filters from the controls
	CArray<CString, CString> arFilters;
	arFilters.Add(CreateTransactionFilter());
	arFilters.Add(CreateLevelFilter());

	// (r.gonet 05/01/2014) - PLID 49432 - Now put them together into a where clause
	for(int i = 0; i < arFilters.GetSize(); i++) {
		CString strTemp = arFilters.GetAt(i);
		if(!strFilter.IsEmpty() && strTemp != "") {
			strFilter += " AND ";
		}
		strFilter += strTemp;
	}

	// (r.gonet 05/01/2014) - PLID 49432 - Requery the log entry list with this new filter
	m_pLogEntries->PutWhereClause(_bstr_t(strFilter));
	m_pLogEntries->Requery();
	m_pLogEntries->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
}

// (r.gonet 05/01/2014) - PLID 49432 - Previews the HL7 message associated with a given transaction.
void CHL7LogDlg::ShowHL7Message(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(!pRow) {
		// No row to show? Odd.
		return;
	}

	// (r.gonet 05/01/2014) - PLID 49432 - Get the message ID
	long nMessageID = VarLong(pRow->GetValue(etlcImportMessageID), -1);
	if(nMessageID == -1) {
		// (r.gonet 05/01/2014) - PLID 49432 - Might have been deleted or might be that it was never added to the message queue.
		MessageBox("Cannot view the HL7 message. No HL7 message is stored for this transaction.", "Error", MB_OK|MB_ICONERROR);
		return;
	}

	// (r.gonet 05/01/2014) - PLID 49432 - Grab the message text
	_RecordsetPtr prs; 
	prs = CreateParamRecordset(
		"SELECT Message "
		"FROM HL7MessageQueueT "
		"WHERE ID = {INT}; ",
		nMessageID);

	if(prs->eof) {
		// (r.gonet 05/01/2014) - PLID 49432 - A message ID without a message?
		ThrowNxException("%s : Could not retrieve HL7 message from HL7 diagnostic log (Entry ID = %li).", 
			__FUNCTION__, VarLong(pRow->GetValue(etlcID)));
	} else {
		prs->Close();
		// (r.goldschmidt 2015-11-02 15:53) - PLID 66437 - Update everywhere in Practice that displays HL7 messages to handle purged messages
		ViewHL7ImportMessage(nMessageID);
	}
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the from datetimepicker
void CHL7LogDlg::OnBnClickedHl7FromFilterCheck()
{
	try {
		m_dtFrom.EnableWindow(m_checkFrom.GetCheck() == BST_CHECKED);
		FilterTransactionList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the to datetimepicker
void CHL7LogDlg::OnBnClickedHl7ToFilterCheck()
{
	try {
		m_dtTo.EnableWindow(m_checkTo.GetCheck() == BST_CHECKED);
		FilterTransactionList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the Only show non-committed checkbox
void CHL7LogDlg::OnBnClickedShowOnlyNoncommittedCheck()
{
	try {
		FilterTransactionList();
	} NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CHL7LogDlg, CNxDialog)
	ON_EVENT(CHL7LogDlg, IDC_HL7_GROUP_COMBO, 1 /* SelChanging */, OnSelChangingHl7GroupCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CHL7LogDlg, IDC_HL7_GROUP_COMBO, 16 /* SelChosen */, OnSelChosenHl7GroupCombo, VTS_DISPATCH)
	ON_EVENT(CHL7LogDlg, IDC_HL7_TRANSACTION_LIST, 19 /* LeftClick */, OnLeftClickHl7TransactionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7LogDlg, IDC_HL7_TRANSACTION_LIST, 2 /* SelChanged */, OnSelChangedHl7TransactionList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CHL7LogDlg, IDC_HL7_LOG_ENTRY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedHl7LogEntryList, VTS_I2)
END_EVENTSINK_MAP()

// (r.gonet 05/01/2014) - PLID 49432 - Don't let the user select a null row
void CHL7LogDlg::OnSelChangingHl7GroupCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - When the user chooses a group, filter on it.
void CHL7LogDlg::OnSelChosenHl7GroupCombo(LPDISPATCH lpRow)
{
	try {
		FilterTransactionList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for the refresh button
void CHL7LogDlg::OnBnClickedHl7LogRefreshBtn()
{
	try {
		FilterTransactionList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Handler for when the user clicks a cell
void CHL7LogDlg::OnLeftClickHl7TransactionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		switch(nCol) {
			case etlcMessageIcon:
				// Preview the transaction's HL7 message in a message box
				ShowHL7Message(pRow);
				break;
			default:
				break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - When the user selects a from datetime, filter on it
void CHL7LogDlg::OnDtnDatetimechangeHl7LogBeginFilterDatetime(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		FilterTransactionList();
		*pResult = 0;
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - When the user selects a to datetime, filter on it
void CHL7LogDlg::OnDtnDatetimechangeHl7LogEndFilterDatetime(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		FilterTransactionList();
		*pResult = 0;
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - Let the dialog be maximized
void CHL7LogDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		CNxDialog::OnSize(nType, cx, cy);
		SetControlPositions();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - When the user selects a transaction row, load the transaction's log entries
void CHL7LogDlg::OnSelChangedHl7TransactionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		FilterLogEntryList();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 49432 - When the log entry list finishes loading, populate the icon column.
void CHL7LogDlg::OnRequeryFinishedHl7LogEntryList(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLogEntries->GetFirstRow();
		while(pRow) {
			// Populate the icon column
			ELogEntryLevel elelLevel = (ELogEntryLevel)VarLong(pRow->GetValue(lelcLevel), elelNone);
			if(elelLevel == elelError) {
				pRow->PutValue(lelcLevelIcon, (long)m_hIconError);
			} else if(elelLevel == elelWarning) {
				pRow->PutValue(lelcLevelIcon, (long)m_hIconWarning);
			} else if(elelLevel == elelInfo) {
				pRow->PutValue(lelcLevelIcon, (long)m_hIconInfo);
			} else if(elelLevel == elelDebug) {
				pRow->PutValue(lelcLevelIcon, (long)m_hIconDiagnostic);
			}
			pRow = pRow->GetNextRow();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/02/2014) - PLID 49432 - Show only the HL7 transactions that have errors
void CHL7LogDlg::OnBnClickedShowOnlyErrorTransactionsCheck()
{
	try {
		FilterTransactionList();
	} NxCatchAll(__FUNCTION__);
}
