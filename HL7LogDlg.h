#pragma once

// (r.gonet 05/01/2014) - PLID 49432 - Created.

#include "FinancialRc.h"


// CHL7LogDlg dialog

// (r.gonet 05/01/2014) - PLID 49432 - Displays the HL7 log entries for each HL7 transaction that has taken place.
class CHL7LogDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7LogDlg)
private:
	// (r.gonet 05/01/2014) - PLID 49432 - Column enumeration for the HL7 Group combo
	enum EHL7GroupComboColumns
	{
		hgccID,
		hgccName,
	};

	// (r.gonet 05/01/2014) - PLID 49432 - Column enumeration for the HL7 Transaction List
	enum ETransactionListColumn
	{
		etlcID = 0,
		etlcHL7GroupName,
		etlcTransactionTimeStamp,
		etlcUsername,
		etlcMessageIcon,
		etlcPatientName,
		etlcPatientSex,
		etlcPatientDOB,
		etlcDescription,
		etlcAction,
		etlcMessageTime,
		etlcMessageType,
		etlcEventType,
		etlcHL7GroupID,
		etlcImportMessageID,
		etlcBackColor,
	};

	// (r.gonet 05/01/2014) - PLID 49432 - Column enumeration for the Log Entry List
	enum ELogEntryListColumn
	{
		lelcID = 0,
		lelcHL7TransactionID,
		lelcLevelIcon,
		lelcLevel,
		lelcRowNumber,
		lelcTimeStamp,
		lelcText,
		lelcFunctionName,
		lelcLineNumber,
	};

public:
	// (r.gonet 05/01/2014) - PLID 49432 - Constructs a new CHL7LogDlg dialog
	CHL7LogDlg(CWnd* pParent = NULL);   // standard constructor
	// (r.gonet 05/01/2014) - PLID 49432 - Frees up any resources (such as icons) used by the CHL7LogDlg
	virtual ~CHL7LogDlg();

// Dialog Data
	enum { IDD = IDD_HL7_LOG_DLG };

	// (r.gonet 05/01/2014) - PLID 49432 - Header label describing the purpose of this dialog
	CNxStatic m_nxsHeaderLabel;
	// (r.gonet 05/01/2014) - PLID 49432 - Label naming the HL7 Group combo box
	CNxStatic m_nxsHL7GroupLabel;
	// (r.gonet 05/01/2014) - PLID 49432 - Combo box to let the user filter on HL7 transactions of certain HL7SettingsT groups
	NXDATALIST2Lib::_DNxDataListPtr m_pHL7GroupsCombo;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox enabling the From date time picker filter
	NxButton m_checkFrom;
	// (r.gonet 05/01/2014) - PLID 49432 - Date Time Picker that lets the user filter on HL7 transactions occurring on or after the specified date.
	CDateTimePicker m_dtFrom;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox enabling the To date time picker filter
	NxButton m_checkTo;
	// (r.gonet 05/01/2014) - PLID 49432 - Date Time Picker that lets the user filter on HL7 transactions occurring before or on the specified date
	CDateTimePicker m_dtTo;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox to show only HL7 transactions for messages that are still in the queue, uncommitted.
	NxButton m_checkOnlyShowNonCommitted;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox to show only HL7 transactions with errors
	NxButton m_checkOnlyShowErrorTransactions;
	// (r.gonet 05/01/2014) - PLID 49432 - Datalist displaying the HL7 transactions currently filtered in.
	NXDATALIST2Lib::_DNxDataListPtr m_pTransactions;
	// (r.gonet 05/01/2014) - PLID 49432 - Label that describes the various levels the user can choose to see.
	CNxStatic m_nxsLevelsLabel;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox causing the error type log entries to show in the log entry list.
	NxButton m_checkErrors;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox causing the warning type log entries to show in the log entry list.
	NxButton m_checkWarnings;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox causing the information type log entries to show in the log entry list.
	NxButton m_checkInformation;
	// (r.gonet 05/01/2014) - PLID 49432 - Checkbox causing the diagnostic/debug type log entries to show in the log entry list.
	NxButton m_checkDiagnostic;
	// (r.gonet 05/01/2014) - PLID 49432 - Datalist displaying the HL7 log entries for the currently selected transaction.
	NXDATALIST2Lib::_DNxDataListPtr m_pLogEntries;
	// (r.gonet 05/01/2014) - PLID 49432 - Button that refreshes the HL7 log manually.
	CNxIconButton m_nxbRefresh;
	// (r.gonet 05/01/2014) - PLID 49432 - Button that closes the HL7 log dialog.
	CNxIconButton m_nxbClose;

	// (r.gonet 05/01/2014) - PLID 49432 - Icon for the title bar
	HICON m_hIconScriptAttention;
	// (r.gonet 05/01/2014) - PLID 49432 - Icon for error type log entries
	HICON m_hIconError;
	// (r.gonet 05/01/2014) - PLID 49432 - Icon for warning type log entries
	HICON m_hIconWarning;
	// (r.gonet 05/01/2014) - PLID 49432 - Icon for information type log entries
	HICON m_hIconInfo;
	// (r.gonet 05/01/2014) - PLID 49432 - Icon for diagnostic type log entries
	HICON m_hIconDiagnostic;
	// (r.gonet 05/01/2014) - PLID 49432 - Icon for the transaction list's preview message column.
	HICON m_hIconMessage;

protected:
	// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on the group combo
	CString CreateGroupFilter();
	// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on the date of the transaction
	CString CreateTimeStampFilter();
	// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on whether the HL7 message is committed or not.
	CString CreateOnlyShowNonCommittedFilter();
	// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the transaction list that causes it to be filtered on transactions that had errors
	CString CreateOnlyShowErrorTransactionsFilter();
	// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL where clause for the log entry list that causes it to be filtered on the currently selected transaction
	CString CreateTransactionFilter();
	// (r.gonet 05/01/2014) - PLID 49432 - Returns a SQL condition for the log entry list that causes it to be filtered on the severity levels.
	CString CreateLevelFilter();
	// (r.gonet 05/01/2014) - PLID 49432 - Filters the transaction list based on the various filter controls.
	void FilterTransactionList();
	// (r.gonet 05/01/2014) - PLID 49432 - Filters the log entry list based on the level checkboxes.
	void FilterLogEntryList();
	// (r.gonet 05/01/2014) - PLID 49432 - Previews the HL7 message associated with a given transaction.
	void ShowHL7Message(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (r.gonet 05/01/2014) - PLID 49432 - Hides from non-Nextech Technical Support Users the ability to show
	// diagnostic/debug log entries and a couple more detailed columns in the log entry list.
	void ShowHideDiagnosticControls();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// (r.gonet 05/01/2014) - PLID 49432
	virtual BOOL OnInitDialog();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the errors checkbox
	afx_msg void OnBnClickedShowErrorsCheck();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the warnings checkbox
	afx_msg void OnBnClickedShowWarningsCheck();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the info checkbox
	afx_msg void OnBnClickedShowInfoCheck();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the diagnostic checkbox
	afx_msg void OnBnClickedShowDiagnosticCheck();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the from datetimepicker
	afx_msg void OnBnClickedHl7FromFilterCheck();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the to datetimepicker
	afx_msg void OnBnClickedHl7ToFilterCheck();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the Only show non-committed checkbox
	afx_msg void OnBnClickedShowOnlyNoncommittedCheck();
	DECLARE_EVENTSINK_MAP()
	// (r.gonet 05/01/2014) - PLID 49432 - Don't let the user select a null row
	afx_msg void OnSelChangingHl7GroupCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (r.gonet 05/01/2014) - PLID 49432 - When the user chooses a group, filter on it.
	afx_msg void OnSelChosenHl7GroupCombo(LPDISPATCH lpRow);
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for the refresh button
	afx_msg void OnBnClickedHl7LogRefreshBtn();
	// (r.gonet 05/01/2014) - PLID 49432 - Handler for when the user clicks a cell
	afx_msg void OnLeftClickHl7TransactionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (r.gonet 05/01/2014) - PLID 49432 - When the user selects a from datetime, filter on it
	afx_msg void OnDtnDatetimechangeHl7LogBeginFilterDatetime(NMHDR *pNMHDR, LRESULT *pResult);
	// (r.gonet 05/01/2014) - PLID 49432 - When the user selects a to datetime, filter on it
	afx_msg void OnDtnDatetimechangeHl7LogEndFilterDatetime(NMHDR *pNMHDR, LRESULT *pResult);
	// (r.gonet 05/01/2014) - PLID 49432 - Let the dialog be maximized
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (r.gonet 05/01/2014) - PLID 49432 - When the user selects a transaction row, load the transaction's log entries
	afx_msg void OnSelChangedHl7TransactionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	// (r.gonet 05/01/2014) - PLID 49432 - When the log entry list finishes loading, populate the icon column.
	afx_msg void OnRequeryFinishedHl7LogEntryList(short nFlags);
	afx_msg void OnBnClickedShowOnlyErrorTransactionsCheck();
};