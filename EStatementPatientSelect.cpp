// EStatementPatientSelect.cpp : implementation file
//

// (j.dinatale 2011-03-21 14:52) - PLID 41444 - Created

#include "stdafx.h"
#include "Practice.h"
#include "GlobalReportUtils.h"
#include "EStatementPatientSelect.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CEStatementPatientSelectDlg dialog

// (a.walling 2013-08-13 10:43) - PLID 57998 - EStatementPatientSelectDlg needs to make a copy of the CReportInfo, otherwise the underlying CReportInfo and parameters can change while the dialog is still active.

IMPLEMENT_DYNAMIC(CEStatementPatientSelectDlg, CNxDialog)

// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Needed to rename the enum values because there was a conflict
// (j.dinatale 2011-03-25 16:56) - PLID 41444 - Added last bill, patient balance, age of balance
// (r.goldschmidt 2016-01-15 10:18) - PLID 67899 - Added location columns and maxageofbalance
enum PatientListColumns{
	pslcPersonID = 0,
	pslcUserDefinedID,
	pslcPatientName,
	pslcLocationID,
	pslcLocationName,
	pslcLastSent,
	pslcLastBill,
	pslcLocationBalance,
	pslcPatBalance,
	pslcAgeOfBalance,
	pslcMaxAgeOfBalance,
	pslcNumberOfStatementsSent,
	pslcComboPersonIDLocationID,
};

// (c.haag 2016-05-19 14:18) - PLID-68687 - The event sink that notifies us of the status of m_prsasyncSelectedList
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE CEStatementPatientSelectDlg::CSelectedListConnectionSink::raw_ExecuteComplete(
	long RecordsAffected,
	struct ADODB::Error *pError,
	enum ADODB::EventStatusEnum *adStatus,
	struct ADODB::_Command *pCommand,
	struct ADODB::_Recordset *pRecordset,
	struct ADODB::_Connection *pConnection)
{
	try
	{
		// Make sure we're dealing with our recordset and not another one
		_RecordsetPtr prs(pRecordset);
		if (prs != m_pdlg->m_prsasyncSelectedList)
		{
			// Ignore this event because it's not ours
		}
		else
		{
			// Throw an error if one took place
			NxAdo::Events::BaseEvent ev(pError, adStatus);
			ev.ThrowError();

			// Sometimes the report query will invoke a 'Warning: Null value is eliminated by an aggregate or other SET operation.'
			// I noticed that there is a correlation between that and getting back a closed recordset. If it's closed then we can
			// iterate to the next recordset to get the results we want
			if (prs->State == adStateClosed)
			{
				prs = prs->NextRecordset(nullptr);
			}

			// Success. Populate the datalist; it's cool because we're in the main thread
			short nColumns = (short)prs->Fields->Count;
			std::set<long> setPatIDs;
			while (!prs->eof)
			{
				IRowSettingsPtr pRow = m_pdlg->m_pSelectedList->GetNewRow();
				for (short i = 0; i < nColumns; i++)
				{
					pRow->Value[i] = prs->Fields->Item[i]->Value;
				}

				// if by patient, make sure to set age of balance same as max age of balance
				// it is safe to assume that the oldest balance date will be properly selected because the recordset is ordered by oldest balance date
				// (this ordering should also take care of the max age balance, but let's be doubly sure since it is easy to do)
				if (!m_pdlg->m_bByLocation) { 
					pRow->Value[pslcAgeOfBalance] = pRow->Value[pslcMaxAgeOfBalance];
				}

				// if list is not by location AND the patient is already in the set
				if (!m_pdlg->m_bByLocation && (setPatIDs.count(pRow->Value[pslcPersonID]) != 0)) {
					// do not add row to list
				}
				// else the list is by location OR the patient is not already in the set
				else {
					// add the row to the list; add the ID to the set
					m_pdlg->m_pSelectedList->AddRowSorted(pRow, nullptr);
					setPatIDs.insert(pRow->Value[pslcPersonID]);
				}

				prs->MoveNext();
			}

			// Safe to call this because we're in the main thread
			m_pdlg->OnDatalistPopulated();
		}
	}
	NxCatchAll_NoParent("CEStatementPatientSelectDlg::CSelectedListConnectionSink::raw_ExecuteComplete 1");

	try
	{ 
		m_pdlg->m_prsasyncSelectedList = nullptr;
		m_pdlg->m_bTempTableExists = true;
		// This doesn't cause an issue, but for the record I'm not a fan of unadvising events from within an event
		Unadvise(m_pdlg->m_pconasyncSelectedList);
		m_pdlg->m_pconasyncSelectedList = nullptr;
	}
	NxCatchAll_NoParent("CEStatementPatientSelectDlg::CSelectedListConnectionSink::raw_ExecuteComplete 2");

	return S_OK;
}

CEStatementPatientSelectDlg::CEStatementPatientSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEStatementPatientSelectDlg::IDD, pParent), m_groupChecker(NetUtils::Groups)
{
	// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Needed to keep track of if we are running a summary report and the report we ran
	m_bSummary = FALSE;
	m_bHasSavedWork = false;
	m_bByLocation = false; // (r.goldschmidt 2016-01-15 12:51) - PLID 67899
	m_asyncSelectedListListener.m_pdlg = this; // (c.haag 2016-05-19 14:18) - PLID-68687
	m_bTempTableExists = false;
}

CEStatementPatientSelectDlg::~CEStatementPatientSelectDlg()
{
	// (j.dinatale 2011-03-30 16:13) - PLID 42982 - Added Try Catch
	try{
		// (j.dinatale 2011-04-05 11:50) - PLID 42982 - renamed
		// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Clear the saved work
		ClearDialog(true);
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-03-30 11:57) - PLID 42982 - need build a list of Patients that are unselected so that way we can automatically move patients
//		to the unselected list if the user decides to run the report again
// (r.goldschmidt 2016-01-15 11:05) - PLID 67899 - account for by patient versus by patient by location
void CEStatementPatientSelectDlg::BuildUnselectedListCache()
{
	// remove all IDs from our unselected cache list
	if (!m_bByLocation) { // by patient
		m_aryCachedUnselPatIDs.RemoveAll();
	}
	else { // by patient by location
		m_mapCachedUnselPatIDLocIDs.clear();
	}
	IRowSettingsPtr pRow = m_pUnselectedList->GetFirstRow();

	// for each row in the unselected list, cache the PatientID and Location ID
	while(pRow){
		long nPatientID = VarLong(pRow->GetValue(pslcPersonID), -1);
		long nLocationID = VarLong(pRow->GetValue(pslcLocationID), -1);

		if(nPatientID != -1){
			if (!m_bByLocation) { // by patient
				m_aryCachedUnselPatIDs.Add(nPatientID);
			}
			else { // by patient by location
				m_mapCachedUnselPatIDLocIDs.emplace(nPatientID, nLocationID);
			}
		}

		pRow = pRow->GetNextRow();
	}
}

// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Sets the report info object and the summary flag
// (r.goldschmidt 2016-01-15 11:05) - PLID 67899 - determine if dialog is operating by patient or by patient by location
// (c.haag 2016-05-19 14:18) - PLID-68687 - We now require the report SQL to generate the qualifying patient list
void CEStatementPatientSelectDlg::SetReport(CReportInfo *pReport, BOOL bSummary, const CComplexReportQuery& reportQuery)
{
	// (c.haag 2016-05-19 14:18) - PLID-68687 - We used to clear the dialog when assigning patient ID's. Since we don't
	// do that anymore, clear it from here
	ClearDialog();

	// set the report info and if its a summary
	m_bSummary = bSummary;
	m_report = *pReport;
	m_reportQuery = reportQuery;

	switch (m_report.nID) {
	//by patient
	case 169:
	case 234:
	case 353:
	case 354:
	case 434:
	case 436:
		m_bByLocation = false;
		break;
	//by patient by location
	case 337:
	case 338:
	case 355:
	case 356:
	//by patient by location by last sent
	case 435:
	case 437:
		m_bByLocation = true;
		break;
		//unknown
	default:
		ASSERT(FALSE); // there should not be any other type of report getting here
	}
}

// (j.dinatale 2011-04-05 11:34) - PLID 42982 - bool variable now keeps track of "saved" work
// (j.dinatale 2011-03-30 12:00) - PLID 42982 - returns if either the selected or unselected list has items in it
bool CEStatementPatientSelectDlg::HasSavedWork()
{
	return m_bHasSavedWork;
}

// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Clears saved work
// (j.dinatale 2011-04-05 11:50) - PLID 42982 - renamed
// (r.goldschmidt 2016-01-15 11:05) - PLID 67899 - account for by patient by location 
void CEStatementPatientSelectDlg::ClearDialog(bool bClearCache /* = false */)
{
	// clear both lists
	m_pSelectedList->Clear();
	m_pUnselectedList->Clear();
	
	// clear the report pointer and reset the summary variable
	m_report = CReportInfo();
	m_bSummary = FALSE;

	if(bClearCache){
		// clear our arrays
		m_aryCachedUnselPatIDs.RemoveAll();
		m_mapCachedUnselPatIDLocIDs.clear();
	}
}

void CEStatementPatientSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_MOVEPATUP, m_btnUp);
	DDX_Control(pDX, IDC_MOVEPATDOWN, m_btnDown);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MOVEALLPATUP, m_btnAllUp);
	DDX_Control(pDX, IDC_CREATEUNSELGROUP, m_chkExportGroup);
	DDX_Control(pDX, IDC_ESTATEMENT_UNSELCOUNT, m_lblUnselCount);
	DDX_Control(pDX, IDC_ESTATEMENT_SELCOUNT, m_lblSelCount);
}


BEGIN_MESSAGE_MAP(CEStatementPatientSelectDlg, CNxDialog)
	ON_BN_CLICKED(IDC_MOVEPATUP, &CEStatementPatientSelectDlg::OnBnClickedMovepatup)
	ON_BN_CLICKED(IDC_MOVEPATDOWN, &CEStatementPatientSelectDlg::OnBnClickedMovepatdown)
	ON_BN_CLICKED(IDC_MOVEALLPATUP, &CEStatementPatientSelectDlg::OnBnClickedMoveallpatup)
	ON_BN_CLICKED(IDOK, &CEStatementPatientSelectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CREATEUNSELGROUP, &CEStatementPatientSelectDlg::OnBnClickedCreateunselgroup)
	ON_BN_CLICKED(IDCANCEL, &CEStatementPatientSelectDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CEStatementPatientSelectDlg::OnInitDialog() 
{
	try{
		CNxDialog::OnInitDialog();

		GetControlPositions();

		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		// set up our buttons
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		m_btnAllUp.AutoSet(NXB_UUP);

		// (j.dinatale 2011-04-08 13:41) - PLID 41444 - need to default our counts to 0
		m_lblSelCount.SetWindowText("0");
		m_lblUnselCount.SetWindowText("0");

		// bind our lists
		m_pSelectedList = BindNxDataList2Ctrl(IDC_SELECTEDPATIENTS, false);
		m_pUnselectedList = BindNxDataList2Ctrl(IDC_UNSELECTEDPATIENTS, false);

		// (j.dinatale 2011-04-05 09:40) - PLID 41444 - cache properties on init
		g_propManager.CachePropertiesInBulk("StatementSetup", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name =  'ShowBillingNotesOnEStatement' OR "	
			"Name = 'StatementsShowEStatementsPatientSelect' OR "
			"Name = 'EStatementsPatientSelect_ExportUnselGroup' "
			")",
			_Q(GetCurrentUserName()));

		// (j.dinatale 2011-04-04 14:10) - PLID 42983 - get the value of our remote property for the group export checkbox
		m_chkExportGroup.SetCheck(GetRemotePropertyInt("EStatementsPatientSelect_ExportUnselGroup", 0, 0, "<None>") == 1);
	}NxCatchAll(__FUNCTION__);
	return FALSE;
}

// (r.goldschmidt 2016-01-14 16:45) - PLID 67899 - get the comma separated LocationID list from report filters to use in a query
CString CEStatementPatientSelectDlg::GenerateLocationIDList()
{
	CString strLocationIDs = "";
	CString strList = "";
	if (m_report.nLocation != -2) {
		strLocationIDs = (m_report.GetLocationFilter(0, 0));
		strLocationIDs.Replace("=", "(");
		int nIndex = strLocationIDs.Find("(");
		strList = strLocationIDs.Mid(nIndex);
		strList.Replace("(", "");
		strList.Replace(")", "");
		strList.Trim();
	}
	else {
		strList = "-1"; // also check for nLocation later
	}
	return strList;
}


// (j.dinatale 2011-04-08 13:45) - PLID 41444 - use this function to update the counts of the list count labels for the lists
void CEStatementPatientSelectDlg::UpdateListCounts()
{
	// we can go ahead and get strings with our count numbers
	CString strSelCount;
	strSelCount.Format("%li", m_pSelectedList->GetRowCount());

	CString strUnselCount;
	strUnselCount.Format("%li", m_pUnselectedList->GetRowCount());

	// set the labels accordingly
	m_lblSelCount.SetWindowText(strSelCount);
	m_lblUnselCount.SetWindowText(strUnselCount);
}

// (r.goldschmidt 2016-01-14 16:45) - PLID 67899 - show/hide columns based on by patient versus by patient by location reports
void CEStatementPatientSelectDlg::EnsureControls()
{
	try {

		NXDATALIST2Lib::IColumnSettingsPtr pColLocationNameSel, pColLocationBalanceSel, pColLocationNameUnsel, pColLocationBalanceUnsel;
		pColLocationNameSel = m_pSelectedList->GetColumn(pslcLocationName);
		pColLocationBalanceSel = m_pSelectedList->GetColumn(pslcLocationBalance);
		pColLocationNameUnsel = m_pUnselectedList->GetColumn(pslcLocationName);
		pColLocationBalanceUnsel = m_pUnselectedList->GetColumn(pslcLocationBalance);

		if (!m_bByLocation) { // by patient
			// hide columns, set title of dialog for by patient
			pColLocationNameSel->PutStoredWidth(0);
			pColLocationNameSel->ColumnStyle = pColLocationNameSel->ColumnStyle | NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			pColLocationBalanceSel->PutStoredWidth(0);
			pColLocationBalanceSel->ColumnStyle = pColLocationBalanceSel->ColumnStyle | NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			pColLocationNameUnsel->PutStoredWidth(0);
			pColLocationNameUnsel->ColumnStyle = pColLocationNameUnsel->ColumnStyle | NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			pColLocationBalanceUnsel->PutStoredWidth(0);
			pColLocationBalanceUnsel->ColumnStyle = pColLocationBalanceUnsel->ColumnStyle | NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			SetWindowText("E-Statements Patient Selection");
		}
		else { // by patient by location
			// show columns, set title of dialog for by location
			pColLocationNameSel->PutStoredWidth(125);
			pColLocationNameSel->ColumnStyle = pColLocationNameSel->ColumnStyle & ~NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			pColLocationBalanceSel->PutStoredWidth(75);
			pColLocationBalanceSel->ColumnStyle = pColLocationBalanceSel->ColumnStyle & ~NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			pColLocationNameUnsel->PutStoredWidth(125);
			pColLocationNameUnsel->ColumnStyle = pColLocationNameUnsel->ColumnStyle & ~NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			pColLocationBalanceUnsel->PutStoredWidth(75);
			pColLocationBalanceUnsel->ColumnStyle = pColLocationBalanceUnsel->ColumnStyle & ~NXDATALIST2Lib::EColumnStyle::csFixedWidth;
			SetWindowText("E-Statements Patient Selection, By Location");
		}

	}NxCatchAll(__FUNCTION__);
}

// Enables or disables all the controls
void CEStatementPatientSelectDlg::EnableControls(BOOL bEnable)
{
	// (c.haag 2016-05-19 14:18) - PLID-68687 - Initial implementation
	GetDlgItem(IDC_SELECTEDPATIENTS)->EnableWindow(bEnable);
	GetDlgItem(IDC_UNSELECTEDPATIENTS)->EnableWindow(bEnable);
	GetDlgItem(IDC_MOVEALLPATUP)->EnableWindow(bEnable);
	GetDlgItem(IDC_MOVEPATUP)->EnableWindow(bEnable);
	GetDlgItem(IDC_MOVEPATDOWN)->EnableWindow(bEnable);
	GetDlgItem(IDOK)->EnableWindow(bEnable);
}

// (j.dinatale 2011-03-30 11:53) - PLID 42982 - broke this out of the OnInitDialog since we need control of when the dialog reloads
void CEStatementPatientSelectDlg::ReloadPatientList()
{
	try{
		EnsureControls();
		EnableControls(FALSE); // (c.haag 2016-05-19 14:18) - PLID-68687 - Disable all the controls until the async query has run

		// (j.dinatale 2011-04-08 13:44) - PLID 41444 - otherwise we are going to be requerying, so indicate as such in the labels
		m_lblSelCount.SetWindowText("Loading...");
		m_lblUnselCount.SetWindowText("Loading...");

		// (c.haag 2016-05-19 14:18) - PLID-68687 - Calculate the name of the temp table that will hold the result set of the report query
		m_strTempReportResultsTable = FormatString("#EStatementPatientSelect%s", NewUUID(true));

		// (r.goldschmidt 2016-01-14 16:45) - PLID 67899 - set where clause based on by patient versus by patient by location reports
		// (c.haag 2016-05-19 14:18) - PLID-68687 - Calculate the WHERE clause here
		CString strWhere;
		if (!m_bByLocation) { // by patient
			strWhere = " "; // instead of here, duplicates are filtered out when the datalist is populated in raw_ExecuteComplete
		}
		else { // by patient by location
			if (!GenerateLocationIDList().IsEmpty()) {
				if (m_report.nLocation != -2) {
					strWhere.AppendFormat(" WHERE LocationID IN (%s)", GenerateLocationIDList());
				}
				else {
					strWhere.Append(" WHERE (LocationID IS NULL OR LocationID = -1)");
				}
			}
		}

		// (j.dinatale 2011-04-12 10:44) - PLID 41444 - Needed to exclude line items that represented quotes
		// (j.dinatale 2011-03-25 16:55) - PLID 41444 - Added patient balance, last bill, age of balance
		// (r.goldschmidt 2016-01-14 18:27) - PLID 67899 - rework query to give option of showing by location info
		// (c.haag 2016-05-19 14:18) - PLID-68687 - Rather than being a FROM clause for the datalist, this is now a standalone query that
		// runs the report query, stores the result set in a temp table, then runs the "datalist query" that joins on the temp table
		CString strSql;
		strSql.Format(R"(
-- Run the report query and store the result set in the temp table
SET NOCOUNT ON
%s
SELECT * INTO [%s] FROM (%s) Q
SET NOCOUNT OFF

-- Run the datalist query

SELECT SubQ.*
FROM (
	SELECT ByLocationQ.PersonID
		, ByLocationQ.PatientID
		, ByLocationQ.PatName
		, ByLocationQ.LocationID
		, ByLocationQ.Location
		, ByLocationQ.LastStatementSent
		, ByLocationQ.OldestBalance
		, ByLocationQ.Balance AS [LocationBalance]
		, TotalBalancesQ.Balance AS [TotalBalance]
		, ByLocationQ.AgeOfBalance
		, DATEDIFF(d, TotalBalancesQ.OldestBalance, GetDate()) AS [MaxAgeOfBalance]
		, ByLocationQ.NumStatements
	FROM (
		SELECT PersonID
			, PatientID
			, PatName
			, LocationID
			, Location
			, MAX(LastStatementSent) AS [LastStatementSent]
			, OldestBalance
			, Balance
			, AgeOfBalance
			, COUNT(DISTINCT dbo.AsDateNoTime(LastStatementSent)) AS [NumStatements]
		FROM (
			SELECT PatientsT.PersonID AS PersonID
				, PatientsT.UserDefinedID AS PatientID
				, (PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle) AS PatName
				, MailSentQ.DATE AS LastStatementSent
				, LocationsT.ID AS LocationID
				, LocationsT.NAME AS Location
				, COALESCE(TotalBalancesQ.Balance, 0) AS Balance
				, dbo.AsDateNoTime(TotalBalancesQ.OldestBalance) AS OldestBalance
				, DATEDIFF(d, TotalBalancesQ.OldestBalance, GetDate()) AS AgeOfBalance
			FROM PatientsT
			LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID
			LEFT JOIN (
				SELECT PersonID
					, LocationID
					, Sum(Balance) AS Balance
					, Min(OldestBalance) AS OldestBalance
				FROM (
					--BalanceQ
					SELECT LineItemT.PatientID AS PersonID
						, LineItemT.LocationID
						, MIN(ChargeRespDetailT.DATE) AS OldestBalance
						, Sum(ChargeRespDetailT.Amount - Coalesce(AppliesQ.AmtApplied, 0)) AS Balance
					FROM ChargeRespDetailT
					INNER JOIN ChargeRespT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID
					INNER JOIN ChargesT ON ChargesT.ID = ChargeRespT.ChargeID
					INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID
					INNER JOIN BillsT ON BillsT.ID = ChargesT.BillID
					LEFT JOIN (
						SELECT Sum(Amount) AS AmtApplied, DetailID
						FROM ApplyDetailsT
						GROUP BY DetailID
						) AS AppliesQ ON ChargeRespDetailT.ID = AppliesQ.DetailID
					WHERE ChargeRespT.InsuredPartyID IS NULL
						AND BillsT.Deleted = 0
						AND LineItemT.Deleted = 0
						AND BillsT.EntryType = 1
						AND LineItemT.Type = 10
						AND ChargeRespDetailT.Amount - Coalesce(AppliesQ.AmtApplied, 0) > 0
					GROUP BY LineItemT.PatientID, LineItemT.LocationID
					
					UNION ALL
					
					-- UnappliesQ 					
					SELECT LineItemT.PatientID AS PersonID
						, LineItemT.LocationID
						, MIN(CASE 
								WHEN (LineItemT.Amount - Coalesce(OutgoingAppliesQ.Total, 0) + Coalesce(IncomingAppliesQ.Total, 0) < 0)
								THEN LineItemT.DATE	ELSE NULL
								END) AS OldestTransDate
						, SUM(- (LineItemT.Amount - Coalesce(OutgoingAppliesQ.Total, 0) + Coalesce(IncomingAppliesQ.Total, 0))) AS PatientBalance
					FROM LineItemT
					INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID AND PaymentsT.PrePayment = 0
					LEFT JOIN (
						SELECT SourceID, Sum(Amount) AS Total
						FROM AppliesT
						GROUP BY SourceID
						) AS OutgoingAppliesQ ON PaymentsT.ID = OutgoingAppliesQ.SourceID
					LEFT JOIN (
						SELECT DestID, Sum(Amount) AS Total
						FROM AppliesT
						GROUP BY DestID
						) AS IncomingAppliesQ ON PaymentsT.ID = IncomingAppliesQ.DestID
					WHERE LineItemT.Amount - Coalesce(OutgoingAppliesQ.Total, 0) + Coalesce(IncomingAppliesQ.Total, 0) <> 0
						AND (PaymentsT.InsuredPartyID IS NULL OR PaymentsT.InsuredPartyID = - 1)
						AND LineItemT.Deleted = 0
					GROUP BY LineItemT.PatientID, LineItemT.LocationID
					) TotalQ
				GROUP BY PersonID, LocationID
				) AS TotalBalancesQ ON TotalBalancesQ.PersonID = PatientsT.PersonID
			LEFT JOIN (
				SELECT dbo.AsDateNoTime(MailSent.ServiceDate) AS DATE
					, MailSent.PersonID AS PersonID
				FROM MailSent
				LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID
				WHERE (
						MailSentNotesT.Note LIKE '%%Patient Statement%%Printed%%'
						OR MailSentNotesT.Note LIKE '%%Patient Statement%%Run%%'
						OR MailSentNotesT.Note LIKE '%%E-Statement%%Exported%%'
						)
				) MailSentQ ON MailSentQ.PersonID = PatientsT.PersonID
				AND MailSentQ.DATE >= TotalBalancesQ.OldestBalance
			LEFT JOIN LocationsT ON LocationsT.ID = TotalBalancesQ.LocationID 
			) SubQ
		GROUP BY PersonID, PatientID, PatName, LocationID, Location, OldestBalance, Balance, AgeOfBalance
		) ByLocationQ
	LEFT JOIN (
		SELECT PatientsT.PersonID
			, CASE 
				WHEN BalanceQ.OldestBalance IS NULL AND UnappliesQ.OldestTransDate IS NULL THEN NULL
				WHEN BalanceQ.OldestBalance IS NOT NULL AND UnappliesQ.OldestTransDate IS NULL THEN BalanceQ.OldestBalance
				WHEN BalanceQ.OldestBalance IS NULL AND UnappliesQ.OldestTransDate IS NOT NULL THEN UnappliesQ.OldestTransDate
				WHEN BalanceQ.OldestBalance < UnappliesQ.OldestTransDate THEN BalanceQ.OldestBalance ELSE UnappliesQ.OldestTransDate
				END AS OldestBalance
			, COALESCE(BalanceQ.Balance, 0) + COALESCE(UnappliesQ.PatientBalance, 0) AS Balance
		FROM PatientsT
		LEFT JOIN (
			SELECT LineItemT.PatientID AS PersonID
				, MIN(ChargeRespDetailT.DATE) AS OldestBalance
				, Sum(ChargeRespDetailT.Amount - Coalesce(AppliesQ.AmtApplied, 0)) AS Balance
			FROM ChargeRespDetailT
			INNER JOIN ChargeRespT ON ChargeRespT.ID = ChargeRespDetailT.ChargeRespID
			INNER JOIN ChargesT ON ChargesT.ID = ChargeRespT.ChargeID
			INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID
			INNER JOIN BillsT ON BillsT.ID = ChargesT.BillID
			LEFT JOIN (
				SELECT Sum(Amount) AS AmtApplied, DetailID
				FROM ApplyDetailsT
				GROUP BY DetailID
				) AS AppliesQ ON ChargeRespDetailT.ID = AppliesQ.DetailID
			WHERE ChargeRespT.InsuredPartyID IS NULL
				AND BillsT.Deleted = 0
				AND LineItemT.Deleted = 0
				AND BillsT.EntryType = 1
				AND LineItemT.Type = 10
				AND ChargeRespDetailT.Amount - Coalesce(AppliesQ.AmtApplied, 0) > 0
			GROUP BY LineItemT.PatientID
			) BalanceQ ON BalanceQ.PersonID = PatientsT.PersonID
		LEFT JOIN (
			SELECT SUM(- (LineItemT.Amount - Coalesce(OutgoingAppliesQ.Total, 0) + Coalesce(IncomingAppliesQ.Total, 0))) AS PatientBalance
				, LineItemT.PatientID AS PersonID
				, MIN(CASE 
					WHEN (LineItemT.Amount - Coalesce(OutgoingAppliesQ.Total, 0) + Coalesce(IncomingAppliesQ.Total, 0) < 0)
					THEN LineItemT.DATE
					ELSE NULL
					END) AS OldestTransDate
			FROM LineItemT
			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID AND PaymentsT.PrePayment = 0
			LEFT JOIN (
				SELECT SourceID, Sum(Amount) AS Total
				FROM AppliesT
				GROUP BY SourceID
				) AS OutgoingAppliesQ ON PaymentsT.ID = OutgoingAppliesQ.SourceID
			LEFT JOIN (
				SELECT DestID, Sum(Amount) AS Total
				FROM AppliesT
				GROUP BY DestID
				) AS IncomingAppliesQ ON PaymentsT.ID = IncomingAppliesQ.DestID
			WHERE LineItemT.Amount - Coalesce(OutgoingAppliesQ.Total, 0) + Coalesce(IncomingAppliesQ.Total, 0) <> 0
				AND (PaymentsT.InsuredPartyID IS NULL OR PaymentsT.InsuredPartyID = - 1)
				AND LineItemT.Deleted = 0
			GROUP BY LineItemT.PatientID
			) UnappliesQ ON PatientsT.PersonID = UnappliesQ.PersonID
		GROUP BY PatientsT.PersonID, BalanceQ.OldestBalance, UnappliesQ.OldestTransDate, BalanceQ.Balance, UnappliesQ.PatientBalance
		) AS TotalBalancesQ ON TotalBalancesQ.PersonID = ByLocationQ.PersonID
	) SubQ
INNER JOIN (SELECT DISTINCT PatID FROM [%s]) IDQ ON IDQ.PatID = SubQ.PersonID
%s
ORDER BY SubQ.PersonID, COALESCE(SubQ.OldestBalance, GETDATE())
)"
			, m_reportQuery.m_strCTE, m_strTempReportResultsTable, m_reportQuery.m_strSQL
			, m_strTempReportResultsTable
			, strWhere
			);

		m_bHasSavedWork = true;

		// (c.haag 2016-05-19 14:18) - PLID-68687 - Now run an asynchronous query so we don't keep the main thread waiting
		m_pconasyncSelectedList = GetRemoteDataSnapshot();
		m_prsasyncSelectedList.CreateInstance(__uuidof(Recordset));
		m_asyncSelectedListListener.Advise(m_pconasyncSelectedList);
		m_prsasyncSelectedList->CursorLocation = adUseClient;
		m_prsasyncSelectedList->Open(_bstr_t(strSql), _variant_t((IDispatch *)m_pconasyncSelectedList, true), adOpenStatic, adLockOptimistic, adCmdText | adAsyncExecute);

	}NxCatchAll(__FUNCTION__);
}

// can be used to obtain the comma separated list of IDs on the selected list
CString CEStatementPatientSelectDlg::GenerateSelectedPersonIDList()
{
	try{
		CString strWhere = "";
		IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();

		// for each row in the list, we want to append the personID value
		while(pRow){
			CString strToAdd;
			long nPersonID = VarLong(pRow->GetValue(pslcPersonID));

			strToAdd.Format("%d,", nPersonID);
			strWhere += strToAdd;

			pRow = pRow->GetNextRow();
		}

		// remove the trailing comma
		if(strWhere.GetLength() > 0){
			strWhere = strWhere.Left(strWhere.GetLength() - 1);
		}

		return strWhere;
	}NxCatchAll(__FUNCTION__);

	return "";
}

// (r.goldschmidt 2016-01-18 09:06) - PLID 67899 - generate filter for selected records when on per patient per location
CString CEStatementPatientSelectDlg::GenerateSelectedPersonIDLocationIDList()
{
	try {
		CString strWhere = "";
		IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();

		// for each row in the list, we want to get the personID and locationID value
		while (pRow) {
			CString strToAdd;
			long nPersonID = VarLong(pRow->GetValue(pslcPersonID));
			long nLocationID = VarLong(pRow->GetValue(pslcLocationID), -1);

			strToAdd.Format(" (PatID = %li AND ", nPersonID);
			if (nLocationID == -1) {
				strToAdd.AppendFormat("(LocID IS NULL OR LocID = -1)) OR");
			}
			else {
				strToAdd.AppendFormat("LocID = %li) OR", nLocationID);
			}
			strWhere += strToAdd;

			pRow = pRow->GetNextRow();
		}

		// remove the trailing OR
		if (strWhere.GetLength() > 0) {
			strWhere = strWhere.Left(strWhere.GetLength() - 2);
		}

		return strWhere;
	}NxCatchAll(__FUNCTION__);

	return "";
}

// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Dialog can no longer be modal
//int CEStatementPatientSelectDlg::DoModal()
//{
//	int nResult;
//	GetMainFrame()->DisableHotKeys();	
//	nResult = CNxDialog::DoModal();
//	GetMainFrame()->EnableHotKeys();
//	return nResult;
//}

// moves patient "up" to the selected list
void CEStatementPatientSelectDlg::OnBnClickedMovepatup()
{
	try{
		// take the current selection from the unselected list and move it to the selected list
		m_pSelectedList->TakeCurrentRowAddSorted(m_pUnselectedList, NULL);

		// (j.dinatale 2011-04-12 09:52) - PLID 41444
		UpdateListCounts();
	}NxCatchAll(__FUNCTION__);
}

// moves patient "down" to the unselected list
void CEStatementPatientSelectDlg::OnBnClickedMovepatdown()
{
	try{
		// take the current selection from the selected list and move it to unselected
		m_pUnselectedList->TakeCurrentRowAddSorted(m_pSelectedList, NULL);

		// (j.dinatale 2011-04-12 09:52) - PLID 41444
		UpdateListCounts();
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEStatementPatientSelectDlg, CNxDialog)
	ON_EVENT(CEStatementPatientSelectDlg, IDC_SELECTEDPATIENTS, 3, CEStatementPatientSelectDlg::DblClickCellSelectedpatients, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEStatementPatientSelectDlg, IDC_UNSELECTEDPATIENTS, 3, CEStatementPatientSelectDlg::DblClickCellUnselectedpatients, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEStatementPatientSelectDlg, IDC_SELECTEDPATIENTS, 19, CEStatementPatientSelectDlg::LeftClickPatient, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEStatementPatientSelectDlg, IDC_UNSELECTEDPATIENTS, 19, CEStatementPatientSelectDlg::LeftClickPatient, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// moves a patient "down" to the unselected list
void CEStatementPatientSelectDlg::DblClickCellSelectedpatients(LPDISPATCH lpRow, short nColIndex)
{
	try{
		IRowSettingsPtr pRow(lpRow);

		if(pRow){
			// if we have a row, take it from the selected list
			m_pUnselectedList->TakeRowAddSorted(pRow);
		}

		// (j.dinatale 2011-04-08 13:44) - PLID 41444 - after we move the row, update the counts
		UpdateListCounts();
	}NxCatchAll(__FUNCTION__);
}

// moves a patient "up" to the selected list
void CEStatementPatientSelectDlg::DblClickCellUnselectedpatients(LPDISPATCH lpRow, short nColIndex)
{
	try{
		IRowSettingsPtr pRow(lpRow);

		if(pRow){
			// if we have a row, take it from the unselected list
			m_pSelectedList->TakeRowAddSorted(pRow);
		}

		// (j.dinatale 2011-04-08 13:44) - PLID 41444 - after we move the row, update the counts
		UpdateListCounts();
	}NxCatchAll(__FUNCTION__);
}

// moves all patients "up" to the selected list
void CEStatementPatientSelectDlg::OnBnClickedMoveallpatup()
{
	try{
		// just take all rows from the unselected list
		m_pSelectedList->TakeAllRows(m_pUnselectedList);

		// (j.dinatale 2011-04-08 13:44) - PLID 41444 - after we move the row, update the counts
		UpdateListCounts();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Since we are modeless, we need to do the saving in here instead
void CEStatementPatientSelectDlg::OnBnClickedOk()
{
	try{
		if(m_pSelectedList->GetRowCount() == 0){
			MessageBox("There are no patients selected for an E-Statements export. Please select some patients to run an export.");
			return;
		}

		_RecordsetPtr rs;

		// (j.dinatale 2011-03-30 12:04) - PLID 42982 - no need to reset the filters, the RunEStatements function passes us a reportinfo pointer
		//		with a report that has all the necessary filters
		//if(!AppendFilters(pReport)){
		//return;
		//}

		// determine where to save the file
		// (c.haag 2016-05-19 14:18) - PLID-68687 - Seems silly to do this after running a big query; if the user cancels then it was for nothing
		// Plus the flow of "do a time consuming thing" "prompt user" "do a time consuming thing" is dumb
		CString strExportPath;
		if (!DetermineSavePath(strExportPath)) {
			return;
		}

		// (c.haag 2016-05-19 14:18) - PLID-68687 - The results of the report query are in m_strTempReportResultsTable, so all we need to do
		// is query on that with a patient ID filter
		{
			CWaitCursor cwait;

			// If nobody is in the exclusion list, we don't need a filter
			rs = CreateRecordset(GetRemoteDataSnapshot(), R"(
SELECT * FROM [%s] Q %s ORDER BY Last ASC, PatientID ASC, ID Asc, Type DESC
)"
				, m_strTempReportResultsTable
				, (m_pUnselectedList->GetRowCount() == 0) ? "" : FormatString("WHERE PatID IN (%s)", GenerateSelectedPersonIDList())
				);
		}

		CFile OutFile(strExportPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);

		{
			CWaitCursor cwait;
			// process the e-statements
			if(!ProcessEStatements(rs, OutFile, &m_report, m_bSummary, false)){
				return;
			}
		}

		// (j.dinatale 2011-04-04 12:57) - PLID 42983 - if the checkbox is checked, then attempt to create a group
		if(m_chkExportGroup.GetCheck()){
			// if for any reason we failed at creating a group, return... we need the user to export again.
			if(!CreateGroup()){
				return;
			}
		}

		// (j.dinatale 2011-04-05 11:09) - PLID 42983 - shouldnt clear anything from the dialog, just mark the dialog as having no "saved" work
		m_bHasSavedWork = false;

		// (c.haag 2016-05-19 14:18) - PLID-68687 - Cleanup
		CleanUpAsyncQueryObjectsAndHideWindow();

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-03-30 12:25) - PLID 42982 - Will switch to the patient module and select the patient that was clicked
void CEStatementPatientSelectDlg::LeftClickPatient(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		// if we clicked the patient name column
		if(nCol == pslcPatientName){
			IRowSettingsPtr pRow(lpRow);

			if(pRow){
				// attempt to get the patient ID
				long nPatientID = VarLong(pRow->GetValue(pslcPersonID), -1);

				if(nPatientID != -1){
					// minimize the E-Statements selection window and flip to the patient that was clicked
					{
						WINDOWPLACEMENT wp;
						wp.length = sizeof(WINDOWPLACEMENT);
						if (GetWindowPlacement(&wp)) {
							if (!IsIconic()) {
								wp.showCmd = SW_MINIMIZE;
								SetWindowPlacement(&wp);
							}
						}
					}

					if(GetMainFrame()) {
						GetMainFrame()->GotoPatient(nPatientID);
					}

					g_Modules[Modules::Patients]->ActivateTab(PatientsModule::BillingTab);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-03-30 12:25) - PLID 42982 - after we requery, need to move any patients from the selected list to our unselected list
//		since we are caching them
// (r.goldschmidt 2016-01-15 11:05) - PLID 67899 - populate unselected list based on appropriate container for by patient versus by patient by location
// (c.haag 2016-05-19 14:18) - PLID-68687 - Called when the asynchronous query finished running and the datalist is populated
void CEStatementPatientSelectDlg::OnDatalistPopulated()
{
	// (j.dinatale 2011-03-30 16:13) - PLID 42982 - Added Try Catch
	try{
		if (!m_bByLocation) { // by patient
			int nCount = m_aryCachedUnselPatIDs.GetCount();

			// for each unselected PatientID in our cache, move it to the unselected list
			for (int i = 0; i < nCount; i++) {
				IRowSettingsPtr pRow = m_pSelectedList->FindByColumn(pslcPersonID, m_aryCachedUnselPatIDs.GetAt(i), NULL, VARIANT_FALSE);

				if (pRow) {
					m_pUnselectedList->TakeRowAddSorted(pRow);
				}
			}
		}
		else { // by patient by location
			for (auto it = m_mapCachedUnselPatIDLocIDs.cbegin(); it != m_mapCachedUnselPatIDLocIDs.cend(); ++it) {
				CString strValue;
				strValue.Format("%li, %li", it->first, it->second);
				IRowSettingsPtr pRow = m_pSelectedList->FindByColumn(pslcComboPersonIDLocationID, _variant_t(strValue), NULL, VARIANT_FALSE);
				if (pRow) {
					m_pUnselectedList->TakeRowAddSorted(pRow);
				}
			}

		}

		// (j.dinatale 2011-04-08 13:44) - PLID 41444 - after the requery is done, we update our counts
		UpdateListCounts();
		// (c.haag 2016-05-19 14:18) - PLID-68687 - Enable the controls again
		EnableControls(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-04-04 09:14) - PLID 42983
bool CEStatementPatientSelectDlg::CreateGroup()
{
	try{
		// if the row count is 0 for the unselected list, then there is no filter to make, return true
		if(m_pUnselectedList->GetRowCount() == 0)
			return true;

		// initialize variables
		bool bSuccess = false;
		CString strGroupName = "Untitled Group";

		// while the user doesnt provide any good input and hasnt hit cancel
		while(true){
			// prompt the user
			if(Prompt("Enter the name for the new patient group:", strGroupName, 50)== IDOK){
				strGroupName.Trim();

				// validate the name given is not an empty string or a system group and check that it doesnt exist already
				if(strGroupName == "") {
					AfxMessageBox("Please enter a non-blank name for this group.");
					continue;
				}

				if(strGroupName == "{Current Patient}" || strGroupName == "{Current Filter}" || strGroupName == "New Group...") {
					AfxMessageBox("You cannot make a group with the same name as a system group.\n"
						"Please enter a different name.");
					continue;
				}

				{
					_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM GroupsT WHERE Name = {STRING}",strGroupName);
					if(!rs->eof) {
						AfxMessageBox("That group name already exists, please choose a new name.");
						continue;
					}
				}

				CWaitCursor wc;
				long nGroupID = -1;

				CSqlTransaction trans("CEStatementsPatientSelect");
				trans.Begin();

				// (j.armen 2012-10-18 16:52) - PLID 53269 - Handle the case where nothing exists in GroupsT
				// insert the new group into GroupsT
				_RecordsetPtr rs = CreateParamRecordset(
					"SET NOCOUNT ON "
					"DECLARE @ID INT "
					"SET @ID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM GroupsT) "
					"SET NOCOUNT OFF "
					"INSERT INTO GroupsT (ID, Name) "
					"OUTPUT inserted.ID "
					"VALUES(@ID, {STRING})",
					strGroupName);

				// fetch the group ID we just inserted
				if(!rs->eof){
					nGroupID = AdoFldLong(rs, "ID");
				}else{
					// this is not possible, but we throw an exception because this is an error case
					ThrowNxException("Could not obtain GroupID for inserted Group.");
				}

				// for each personID in the unselected list, go ahead and insert it into GroupDetailsT
				// (r.goldschmidt 2016-01-19 10:55) - PLID 67899 - make sure the patient only gets inserted once
				IRowSettingsPtr pRow = m_pUnselectedList->GetFirstRow();
				std::set<long> setPersonIDs;
				while(pRow){
					long nPersonID = VarLong(pRow->GetValue(pslcPersonID), -1);

					if(nPersonID != -1){
						setPersonIDs.insert(nPersonID);
					}

					pRow = pRow->GetNextRow();
				}

				for (auto it = setPersonIDs.cbegin(); it != setPersonIDs.cend(); ++it) {
					ExecuteParamSql("INSERT INTO GroupDetailsT (GroupID, PersonID) VALUES ({INT}, {INT})", nGroupID, *it);
				}

				// commit our transaction
				trans.Commit();

				// fire the table checker to cause the letter writing module to refresh
				m_groupChecker.Refresh();

				// the user did their job correctly, break
				bSuccess = true;
				break;
			}else{
				// user hit cancel, we werent successful so break
				bSuccess = false;
				break;
			}
		}

		// return if we succeeded or failed
		return bSuccess;
	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.dinatale 2011-04-04 14:03) - PLID 42983 - Used to save the value of the export group checkbox
void CEStatementPatientSelectDlg::OnBnClickedCreateunselgroup()
{
	SetRemotePropertyInt("EStatementsPatientSelect_ExportUnselGroup", m_chkExportGroup.GetCheck() ? 1 : 0, 0, "<None>");
}

// Must be called to dismiss the dialog
void CEStatementPatientSelectDlg::CleanUpAsyncQueryObjectsAndHideWindow()
{
	// (c.haag 2016-05-19 14:18) - PLID-68687 - Cancel any queries in progress
	try
	{
		if (nullptr != m_prsasyncSelectedList)
		{
			m_asyncSelectedListListener.Unadvise(m_pconasyncSelectedList);
			m_prsasyncSelectedList->Cancel();
		}
	}
	NxCatchAll("CEStatementPatientSelectDlg::OnBnClickedCancel 1");
	m_prsasyncSelectedList = nullptr; // Ensure this is always null even if an exception was caught

	try
	{
		if (m_bTempTableExists)
		{
			ExecuteSql(GetRemoteDataSnapshot(), R"(DROP TABLE [%s])", m_strTempReportResultsTable);
		}
	}
	NxCatchAll("CEStatementPatientSelectDlg::OnBnClickedCancel 2");
	m_bTempTableExists = false; // Even if an exception was caught there's nothing else we can do with the temp table
	m_pconasyncSelectedList = nullptr;

	// just hide the window, we dont want to get rid of it
	ShowWindow(SW_HIDE);
}

// (j.dinatale 2011-04-08 16:27) - PLID 42983
void CEStatementPatientSelectDlg::OnBnClickedCancel()
{
	CleanUpAsyncQueryObjectsAndHideWindow();
}