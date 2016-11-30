// OHIPReportManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OHIPReportManagerDlg.h"
#include "GlobalUtils.h"
#include "OHIPBatchEditParser.h"
#include "OHIPClaimsErrorParser.h"
#include "DateTimeUtils.h"
#include "EOBDlg.h"

// COHIPReportManagerDlg dialog

// (j.jones 2008-12-17 08:59) - PLID 31900 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ReportHistoryListColumns {

	rhlcID = 0,
	rhlcFileName,
	rhlcFilePath,
	rhlcReportTypeID,
	rhlcReportTypeName,
	rhlcReportInternalDate,
	rhlcInputDate,
	rhlcFirstImportDate,
	rhlcLastImportDate,
	rhlcIsNotProcessed,
	rhlcColor,
};

// (j.jones 2009-03-10 10:45) - PLID 33419 - added ability to auto-scan for reports
#define ID_AUTO_SCAN_REPORTS	33419

//these are stored in data and cannot be changed
enum EOHIPReportType {

	eortInvalid = -1,
	eortBatchEdit = 1,
	eortClaimsError = 2,
	eortERemit = 3,
};

COHIPReportManagerDlg::COHIPReportManagerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COHIPReportManagerDlg::IDD, pParent)
{
	m_bAutoScanForReports = FALSE;
}

void COHIPReportManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COHIPReportManagerDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_SCAN_REPORTS, m_btnScanReports);
	DDX_Control(pDX, IDC_BTN_BROWSE_REPORTS, m_btnBrowseReportFolder);
	DDX_Control(pDX, IDC_EDIT_REPORT_FOLDER, m_nxeditReportFolder);
	DDX_Control(pDX, IDC_RADIO_OHIP_RM_ALL_DATES, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_OHIP_RM_DATE_RANGE, m_radioDateRange);
	DDX_Control(pDX, IDC_OHIP_RM_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_OHIP_RM_TO_DATE, m_dtTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COHIPReportManagerDlg, CNxDialog)
	//{{AFX_MSG_MAP(COHIPReportManagerDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SCAN_REPORTS, OnBtnScanReports)
	ON_BN_CLICKED(IDC_BTN_BROWSE_REPORTS, OnBtnBrowseReports)
	ON_BN_CLICKED(IDC_RADIO_OHIP_RM_ALL_DATES, OnRadioOhipRmAllDates)
	ON_BN_CLICKED(IDC_RADIO_OHIP_RM_DATE_RANGE, OnRadioOhipRmDateRange)
	ON_NOTIFY(DTN_CLOSEUP, IDC_OHIP_RM_FROM_DATE, OnCloseupOhipRmFromDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_OHIP_RM_TO_DATE, OnCloseupOhipRmToDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_OHIP_RM_FROM_DATE, OnDatetimechangeOhipRmFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_OHIP_RM_TO_DATE, OnDatetimechangeOhipRmToDate)
	ON_MESSAGE(ID_AUTO_SCAN_REPORTS, OnAutoScanReports)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COHIPReportManagerDlg message handlers

BOOL COHIPReportManagerDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnScanReports.AutoSet(NXB_INSPECT);

		m_ReportHistoryList = BindNxDataList2Ctrl(IDC_OHIP_REPORT_HISTORY_LIST, false);

		//set the color column with the proper COLORREF values
		CString strColor;
		//un-processed reports are red, processed reports are black
		strColor.Format("CASE WHEN LastImportDate Is Null THEN %li ELSE %li END", RGB(255,0,0), RGB(0,0,0));
		m_ReportHistoryList->GetColumn(rhlcColor)->PutFieldName(_bstr_t(strColor));

		//set the default date filter to be the past 3 months
		COleDateTime dt = COleDateTime::GetCurrentTime();
		m_dtTo.SetValue(_variant_t(dt));
		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(90, 0, 0, 0);
		dt -= dtSpan;
		m_dtFrom.SetValue(_variant_t(dt));
		m_radioDateRange.SetCheck(TRUE);

		//now load the list
		RefilterList();

		//set their default reports folder, using the shared path if there currently isn't a default
		CString strReportsPath = GetPropertyText("OHIPReportManager_ScanReportPath", GetSharedPath(), 0, true);
		m_nxeditReportFolder.SetWindowText(strReportsPath);

	}NxCatchAll("Error in COHIPReportManagerDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void COHIPReportManagerDlg::RefilterList()
{
	try {

		//get the date information
		BOOL bAllDates = m_radioAllDates.GetCheck();
		m_dtFrom.EnableWindow(!bAllDates);
		m_dtTo.EnableWindow(!bAllDates);
		if(!bAllDates) {
			CString strWhere;
			COleDateTime dtFrom = VarDateTime(m_dtFrom.GetValue());
			COleDateTime dtTo = VarDateTime(m_dtTo.GetValue());
			if(dtFrom > dtTo) {
				dtFrom = dtTo;
				m_dtFrom.SetValue(dtFrom);
				AfxMessageBox("The From date is greater than the To date. This date has been changed to equal the To date.");
			}
			//always show un-imported reports, and in the rare case that the report date is NULL, always include those too
			strWhere.Format("LastImportDate Is Null OR ReportInternalDate Is Null OR (ReportInternalDate >= '%s' AND ReportInternalDate <= '%s')",
				FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));

			m_ReportHistoryList->PutWhereClause(_bstr_t(strWhere));
		}
		else {
			m_ReportHistoryList->PutWhereClause("");
		}

		m_ReportHistoryList->Requery();	

	}NxCatchAll("Error in COHIPReportManagerDlg::RefilterList");
}

void COHIPReportManagerDlg::OnOK() 
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in COHIPReportManagerDlg::OnOK");
}

BEGIN_EVENTSINK_MAP(COHIPReportManagerDlg, CNxDialog)
	ON_EVENT(COHIPReportManagerDlg, IDC_OHIP_REPORT_HISTORY_LIST, 19, OnLeftClickOhipReportHistoryList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void COHIPReportManagerDlg::OnLeftClickOhipReportHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//right now we only handle the left click on the filename hyperlink
		if(nCol != rhlcFileName) {
			return;
		}

		CString strFileName = VarString(pRow->GetValue(rhlcFileName), "");
		CString strFilePath = VarString(pRow->GetValue(rhlcFilePath), "");
		CString strFullFilePath = strFilePath ^ strFileName;

		if(strFullFilePath.IsEmpty() || !DoesExist(strFullFilePath)) {
			AfxMessageBox("This file appears to no longer exist. It may have been moved, renamed, or deleted.");
			return;
		}

		CWaitCursor pWait;

		EOHIPReportType eortType = (EOHIPReportType)VarLong(pRow->GetValue(rhlcReportTypeID), -1);
		if(eortType == eortBatchEdit) {
			COHIPBatchEditParser dlg;
			dlg.m_strFileName = strFileName;
			dlg.m_strFilePath = strFilePath;
			dlg.ParseFile();

			//requery to update the list with the new dates, colors, and sorting
			m_ReportHistoryList->Requery();
		}
		else if(eortType == eortClaimsError) {
			COHIPClaimsErrorParser dlg;
			dlg.m_strFileName = strFileName;
			dlg.m_strFilePath = strFilePath;
			dlg.ParseFile();

			//requery to update the list with the new dates, colors, and sorting
			m_ReportHistoryList->Requery();
		}
		else if(eortType == eortERemit) {

			//batch payment tab requires the Billing license, no permissions
			//EOBs require the Remittance license (a use of the license is fired when parsing)
			if(!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)
				|| !g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrSilent)) {
					
				return;
			}

			//EOBs require no permissions currently, there's a PL item to add them one day

			//warn if it has already been imported
			_variant_t varDate = pRow->GetValue(rhlcLastImportDate);
			if(varDate.vt == VT_DATE && IDNO == MessageBox("It appears this E-Remittance file has already been processed in Practice.\n\n"
				"Are you sure you wish to process this file again?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}

			//close the report manager
			CNxDialog::OnOK();

			// (j.jones 2012-10-12 10:06) - PLID 53149 - this is now a modeless window
			if(GetMainFrame()) {
				GetMainFrame()->ShowEOBDlg(strFilePath ^ strFileName);
			}

			// (j.jones 2012-10-12 10:09) - PLID 53149 - The EOB is now modeless,
			// so it will not be closed by the time we get here. Fortunately this
			// line was pointless because it was requerying after the dialog closed!

			//requery to update the list with the new dates, colors, and sorting
			//m_ReportHistoryList->Requery();
		}

	}NxCatchAll("Error in COHIPReportManagerDlg::OnLeftClickOhipReportHistoryList");
}

// (j.jones 2008-12-17 10:09) - PLID 32488 - added OnBtnScanReports
void COHIPReportManagerDlg::OnBtnScanReports()
{
	try {

		CString strFolderPath;
		m_nxeditReportFolder.GetWindowText(strFolderPath);

		if(strFolderPath.IsEmpty() || !DoesExist(strFolderPath)) {
			AfxMessageBox("You have not selected a valid folder for the downloaded reports.");
			return;
		}

		//a path this long would be crazy, but let's cleanly handle this now because
		//we can't store paths this long in the data
		if(strFolderPath.GetLength() > 1000) {
			AfxMessageBox("Practice does not support folder paths of this length.\n"
				"You must select a report path of a shorter length to use this feature.");
			return;
		}

		// (j.jones 2009-03-10 10:42) - PLID 33419 - moved this logic to ScanForReports()
		ScanForReports();

	}NxCatchAll("Error in COHIPReportManagerDlg::OnBtnScanReports");
}

// (j.jones 2008-12-17 12:39) - PLID 32488 - added ReconcileFileNames, which will
// take in an array of filenames found in the scanned folder, compare them with
// tracked data (updating paths when necessary), and reduce the array of filenames
// down to only the filenames not currently tracked
void COHIPReportManagerDlg::ReconcileFileNames(CStringArray &arystrFileNames, CString strFolderPath)
{
	//throw exceptions to the caller

	if(arystrFileNames.GetSize() == 0) {
		//we should not have been called with an empty array
		ASSERT(FALSE);
		return;
	}

	//not critical, but shouldn't be possible
	ASSERT(!strFolderPath.IsEmpty());

	//We'll accomplish this by inserting all the filename into a temp table,
	//using UNION statements, broken up at a maximum of 250 UNIONs per insert.
	//Find the filenames that match OHIPReportHistoryT but the paths differ, and update
	//the paths. Then return a list of all the filenames that aren't in OHIPReportHistoryT,
	//those are the files we will need to parse.

	CString strSqlBatch;
	
	//create our temp table
	CString strTempT;
	strTempT.Format("#Temp%luT", GetTickCount());
	AddStatementToSqlBatch(strSqlBatch, "CREATE TABLE %s (FileName nvarchar(255))", _Q(strTempT));

	//insert our filenames into this table
	int i=0;
	CString strUnionQuery;
	for(i=0; i<arystrFileNames.GetSize(); i++) {
		CString strFileName = arystrFileNames.GetAt(i);
		if(!strUnionQuery.IsEmpty()) {
			strUnionQuery += "UNION ";
		}
		
		CString str;
		str.Format("SELECT '%s' ", _Q(strFileName));
		strUnionQuery += str;

		//insert every 250 records
		if(i > 0 && i % 250 == 0 && !strUnionQuery.IsEmpty()) {
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO %s (FileName) %s", _Q(strTempT), strUnionQuery);
			strUnionQuery = "";
		}
	}

	//insert remaining filenames
	if(!strUnionQuery.IsEmpty()) {
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO %s (FileName) %s", _Q(strTempT), strUnionQuery);
		strUnionQuery = "";
	}

	//update the path of tracked entries
	AddStatementToSqlBatch(strSqlBatch, "UPDATE OHIPReportHistoryT SET FilePath = '%s' "
		"WHERE FilePath <> '%s' AND FileName IN (SELECT FileName FROM %s)", _Q(strFolderPath), _Q(strFolderPath), _Q(strTempT));	

	//remove all filenames that are tracked
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM %s WHERE FileName IN (SELECT FileName FROM OHIPReportHistoryT)", _Q(strTempT));	

	//clear our current filename array
	arystrFileNames.RemoveAll();

	//now we can execute all of these changes, and return the remaining file names
	_RecordsetPtr rs = CreateRecordset("SET NOCOUNT ON \r\n"
		"BEGIN TRAN \r\n "
		"%s "
		"COMMIT TRAN \r\n "
		"SET NOCOUNT OFF \r\n"
		"SELECT FileName FROM %s", strSqlBatch, _Q(strTempT));

	while(!rs->eof) {

		CString strFileName = AdoFldString(rs, "FileName");

		//re-add to the array, which was cleared before the batch was run
		arystrFileNames.Add(strFileName);

		rs->MoveNext();
	}
	rs->Close();

	//drop our temp table, has to be outside the batch since the batch
	//returns records from it (this can't be parameterized)
	ExecuteSql("DROP TABLE %s", _Q(strTempT));

	//our array now contains all the filenames in this folder that
	//are not tracked in OHIPReportHistoryT, if any remain
}

// (j.jones 2008-12-17 14:52) - PLID 32488 - used to parse data from the OHIP files
CString COHIPReportManagerDlg::ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim /*= FALSE*/)
{
	//we could just have used CString::Mid() but if the line isn't the proper length,
	//we should handle it more gracefully, just returning ""

	//the specs state the position to start at using 1-based counting,
	//but CString uses 0-based counting, so subtract 1 from nStart
	nStart--;

	long nLineLength = strLine.GetLength();

	if(nStart > nLineLength) {
		ASSERT(FALSE);
		return "";
	}

	if(nStart + nLength > nLineLength) {
		ASSERT(FALSE);
		CString str = strLine.Right(nLineLength - nStart);
		if(!bDoNotTrim) {
			str.TrimLeft();
			str.TrimRight();
		}
	}

	CString str = strLine.Mid(nStart, nLength);

	if(!bDoNotTrim) {
		str.TrimLeft();
		str.TrimRight();
	}

	return str;
}

void COHIPReportManagerDlg::OnBtnBrowseReports()
{
	try {

		//first prompt to browse to a given directory, and default to their last setting for this machine
		//make the setting initially default to the server's shared path
		CString strInitPath;
		m_nxeditReportFolder.GetWindowText(strInitPath);

		//use the shared path if their stored path no longer exists
		CString strSharedPath = GetSharedPath();
		if(!DoesExist(strInitPath)) {
			strInitPath = strSharedPath;		
		}

		CString strFolderPath;
		if(!BrowseToFolder(&strFolderPath, "Select Reports Folder", GetSafeHwnd(), NULL, strInitPath)) {
			return;
		}

		CWaitCursor pWait;

		if(strFolderPath.IsEmpty() || !DoesExist(strFolderPath)) {
			AfxMessageBox("You have not selected a valid folder.");
			return;
		}

		//warn if it is not in the shared path
		if(!strSharedPath.IsEmpty() && strFolderPath.Left(strSharedPath.GetLength()) != strSharedPath) {
			if(IDNO == MessageBox("The path you chose to scan for reports does not appear to be in the Practice Server computer's Shared Path.\n"
				"It is recommended that all reports are stored on the server so users on other workstations can also find these reports.\n\n"
				"Are you sure you wish to scan a different path?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		//a path this long would be crazy, but let's cleanly handle this now because
		//we can't store paths this long in the data
		if(strFolderPath.GetLength() > 1000) {
			AfxMessageBox("Practice does not support folder paths of this length.\n"
				"You must select a report path of a shorter length to use this feature.");
			return;
		}

		m_nxeditReportFolder.SetWindowText(strFolderPath);

		//set this as the default path
		SetPropertyText("OHIPReportManager_ScanReportPath", strFolderPath, 0);

	}NxCatchAll("Error in COHIPReportManagerDlg::OnBtnBrowseReports");
}

void COHIPReportManagerDlg::OnRadioOhipRmAllDates()
{
	try {
		
		RefilterList();

	}NxCatchAll("Error in COHIPReportManagerDlg::OnRadioOhipRmDateRange");
}

void COHIPReportManagerDlg::OnRadioOhipRmDateRange()
{
	try {

		OnRadioOhipRmAllDates();

	}NxCatchAll("Error in COHIPReportManagerDlg::OnRadioOhipRmDateRange");
}

void COHIPReportManagerDlg::OnCloseupOhipRmFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		RefilterList();

		*pResult = 0;

	}NxCatchAll("Error in COHIPReportManagerDlg::OnCloseupOhipRmFromDate");
}

void COHIPReportManagerDlg::OnCloseupOhipRmToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		RefilterList();

		*pResult = 0;

	}NxCatchAll("Error in COHIPReportManagerDlg::OnCloseupOhipRmToDate");
}

void COHIPReportManagerDlg::OnDatetimechangeOhipRmFromDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		
		RefilterList();
		
		*pResult = 0;

	}NxCatchAll("Error in COHIPReportManagerDlg::OnDatetimechangeOhipRmFromDate");
}

void COHIPReportManagerDlg::OnDatetimechangeOhipRmToDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {

		LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		
		RefilterList();

		*pResult = 0;

	}NxCatchAll("Error in COHIPReportManagerDlg::OnDatetimechangeOhipRmToDate");
}

// (j.jones 2009-03-10 10:40) - PLID 33419 - ScanForReports is called by
//OnBtnScanReports and if m_bAutoScanForReports is true, it should have its
//path validated before being called
void COHIPReportManagerDlg::ScanForReports()
{
	try {

		CString strFolderPath;
		m_nxeditReportFolder.GetWindowText(strFolderPath);

		if(strFolderPath.IsEmpty() || !DoesExist(strFolderPath)) {
			//this should have been checked prior to calling this function!
			ASSERT(FALSE);
			return;
		}

		//this code was moved from OnBtnScanReports
		CWaitCursor pWait;

		//look at every file in this folder (this is not a recursive search)
		CFileFind ff;
		BOOL bContinue = ff.FindFile(strFolderPath ^ _T("*.*"));
		CStringArray arystrFileNames;
		while (bContinue) {
			bContinue = ff.FindNextFile();
			if(ff.IsDirectory() || ff.IsDots()) {
				 continue;
			}
			
			//get the file name
			CString strFileName = ff.GetFileName();

			if(strFileName.GetLength() > 255) {
				//should be impossible in windows, and OHIP
				//only sends back 8.3 named files, so silently skip it
				continue;
			}

			//track this file name in an array
			arystrFileNames.Add(strFileName);
		}
		
		if(arystrFileNames.GetSize() == 0) {
			//no files found
			AfxMessageBox("No files were found in the selected folder.");
			return;
		}

		//Now the tricky part - we need to know which files we already parsed, update their paths
		//if the path changed, and then for the files we haven't parsed, parse those.
		
		//ReconcileFileNames will handle all of this for us. When it is finished, arystrFileNames
		//will only contain those files not currently tracked.
		ReconcileFileNames(arystrFileNames, strFolderPath);

		if(arystrFileNames.GetSize() == 0) {
			//always requery, we may have updated folder names
			m_ReportHistoryList->Requery();

			//no remaining files, so we don't need to parse anything
			AfxMessageBox("No new reports were found in the selected folder.");
			return;
		}

		//Now we need to run through each file, read in the first line, and see if it appears to be
		//in one of the three OHIP formats. We pretty much only need to look at the first two
		//characters, and make our decision off of that. Of course any developer can fake this and
		//trick it, but it works for our purposes here.
		//HB - Batch Edit report
		//HX - Claims Error report
		//HR - E-Remittance report

		BOOL bOneFailed = FALSE;

		CString strSqlBatch;

		CStringArray arystrBatchEditFileNames;
		CStringArray arystrClaimsErrorFileNames;
		CStringArray arystrERemitFileNames;

		int i=0;
		for(i=0; i<arystrFileNames.GetSize(); i++) {

			CString strFileName = arystrFileNames.GetAt(i);
			CString strFilePath = strFolderPath ^ strFileName;
			if(!DoesExist(strFilePath)) {
				//why wouldn't it exist now?
				ASSERT(FALSE);
				bOneFailed = TRUE;
				continue;
			}

			CFile fInputFile;
			if(!fInputFile.Open(strFilePath, CFile::modeRead | CFile::shareCompat)) {
				bOneFailed = TRUE;
				continue;
			}

			CArchive arIn(&fInputFile, CArchive::load);

			CString strLine;
			if(arIn.ReadString(strLine) && strLine.GetLength() >= 3) {

				CString strIdentifier = strLine.Left(3);
				CString strRecordID = strIdentifier.Right(1);

				long nRecordID = atoi(strRecordID);

				EOHIPReportType eortType = eortInvalid;

				//the first record of any OHIP file - all three kinds - has to
				//have a record ID of 1, so we know it can't be valid if the 3rd
				//character is not a 1
				if(nRecordID == 1) {
					if(strIdentifier.Left(2) == "HB") {
						//this is a batch edit report
						eortType = eortBatchEdit;
					}
					else if(strIdentifier.Left(2) == "HX") {
						//this is a claims error report
						eortType = eortClaimsError;
					}
					else if(strIdentifier.Left(2) == "HR") {
						//this is an E-Remit report
						eortType = eortERemit;
					}
				}

				if(eortType != eortInvalid) {
					//this is a valid file, so let's track it in OHIPReportHistoryT

					//try to grab the report date from the file
					CString strOHIPFileDate = "";					
					if(eortType == eortBatchEdit) {
						//see COHIPBatchEditParser::ReportRecord for the specs for the Batch Create Date line
						strOHIPFileDate = ParseElement(strLine, 18, 8);
					}
					else if(eortType == eortClaimsError) {
						//see COHIPClaimsErrorParser::HX1_Header for the specs for the Claim Process Date line
						strOHIPFileDate = ParseElement(strLine, 39, 8);
					}
					else if(eortType == eortERemit) {
						//see COHIPERemitParser::FileHeader_1 for the specs for the Payment Date line
						strOHIPFileDate = ParseElement(strLine, 22, 8);
					}

					CString strReportInternalDate = "";
					//parse the date if we have one
					if(strOHIPFileDate.GetLength() == 8) {
						COleDateTime dt;
						dt.SetDate(atoi(strOHIPFileDate.Left(4)), atoi(strOHIPFileDate.Mid(4, 2)), atoi(strOHIPFileDate.Right(2)));
						if(dt.GetStatus() != COleDateTime::invalid) {
							strReportInternalDate.Format("'%s'", FormatDateTimeForSql(dt));
						}
					}

					// (j.jones 2008-12-17 15:09) - PLID 32488 - I cannot find any of the files I have received from clients,
					// of any of the three report types, that do not have dates in them. So if we found a file that matched
					// our meager two-character search string and does not have a date, assume it's not a valid file.
					// If we later find reports that legitimately have no dates, just comment out this if statement.
					if(strReportInternalDate.IsEmpty()) {
						eortType = eortInvalid;
					}

					if(eortType != eortInvalid) {

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO OHIPReportHistoryT (FileName, FilePath, ReportType, ReportInternalDate) "
							"VALUES ('%s', '%s', %li, %s)", _Q(strFileName), _Q(strFolderPath), (long)eortType, strReportInternalDate);

						//track in each array which filename is of which type
						if(eortType == eortBatchEdit) {
							arystrBatchEditFileNames.Add(strFileName);
						}
						else if(eortType == eortClaimsError) {
							arystrClaimsErrorFileNames.Add(strFileName);
						}
						else if(eortType == eortERemit) {
							arystrERemitFileNames.Add(strFileName);
						}
					}
				}
			}

			arIn.Close();
			fInputFile.Close();
		}

		//if any failures occurred, warn about those first, before giving a success message
		if(bOneFailed) {
			AfxMessageBox("At least one file in the folder failed to open. Try scanning the report folder again.\n"
				"If you receive this message regularly, please contact NexTech Support for assistance.");
		}

		if(strSqlBatch.IsEmpty()) {
			//nothing in the folder is a new OHIP report
			CString strMessage;
			strMessage = "No new reports were found in the selected folder.";

			//even though we didn't add reports, requery anyways, as we may have updated folder names
			m_ReportHistoryList->Requery();

			AfxMessageBox(strMessage);
			return;
		}

		ExecuteSqlBatch(strSqlBatch);

		//requery before we display messages, so that the interface behind the message makes sense
		m_ReportHistoryList->Requery();

		long nRemittanceReports = arystrERemitFileNames.GetSize();
		long nNonRemittanceReports = arystrBatchEditFileNames.GetSize() + arystrClaimsErrorFileNames.GetSize();
		long nTotalNewReportsFound = nRemittanceReports + nNonRemittanceReports;
		
		//report success
		CString strMessage;
		if(nTotalNewReportsFound == 1) {
			strMessage = "One new report was found and added to the report list.";
		}
		else {
			strMessage.Format("%li new reports were found and added to the report list.", nTotalNewReportsFound); 
		}

		if(nNonRemittanceReports == 1) {
			strMessage += "\n\nWould you like to view this new report now?";
		}
		else if(nNonRemittanceReports > 1) {
			strMessage += "\n\nWould you like to view these reports now?";
		}

		if(nNonRemittanceReports > 0 && nRemittanceReports > 0) {
			//remits aren't auto-opened, so let's remind them we won't be doing that
			strMessage += "\n(E-Remittance reports will need to be run manually.)";
		}

		if(nNonRemittanceReports == 0) {
			AfxMessageBox(strMessage);
		}
		else if(MessageBox(strMessage, "Practice", MB_ICONQUESTION|MB_YESNO) == IDYES) {

			//parse each batch edit report
			for(i=0; i<arystrBatchEditFileNames.GetSize(); i++) {
				COHIPBatchEditParser dlg;
				dlg.m_strFileName = arystrBatchEditFileNames.GetAt(i);
				dlg.m_strFilePath = strFolderPath;
				dlg.ParseFile();
			}

			//parse each claims error report
			for(i=0; i<arystrClaimsErrorFileNames.GetSize(); i++) {
				COHIPClaimsErrorParser dlg;
				dlg.m_strFileName = arystrClaimsErrorFileNames.GetAt(i);
				dlg.m_strFilePath = strFolderPath;
				dlg.ParseFile();
			}

			//requery again, after the files have been parsed, to reflect which ones have been processed
			m_ReportHistoryList->Requery();
		}		

	}NxCatchAll("Error in COHIPReportManagerDlg::ScanForReports");
}

// (j.jones 2009-03-10 10:47) - PLID 33419 - added OnAutoScanReports
LRESULT COHIPReportManagerDlg::OnAutoScanReports(WPARAM wParam, LPARAM lParam)
{
	try {

		CString strFolderPath;
		m_nxeditReportFolder.GetWindowText(strFolderPath);

		//process this message silently, if the folder path is invalid, do not try to scan

		if(strFolderPath.IsEmpty() || !DoesExist(strFolderPath)) {
			return 0;
		}

		//a path this long would be crazy, but let's cleanly handle this now because
		//we can't store paths this long in the data
		if(strFolderPath.GetLength() > 1000) {
			return 0;
		}

		ScanForReports();

	}NxCatchAll("Error in COHIPReportManagerDlg::OnAutoScanReports");

	return 0;
}

// (j.jones 2009-03-10 11:20) - PLID 33419 - added OnShowWindow
void COHIPReportManagerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {

		CNxDialog::OnShowWindow(bShow, nStatus);

		if(bShow) {

			// (j.jones 2009-03-10 10:48) - PLID 33419 - if m_bAutoScanForReports is true, post
			// the message to do so once the dialog is fully loaded
			if(m_bAutoScanForReports) {
				PostMessage(ID_AUTO_SCAN_REPORTS);
			}
		}

	}NxCatchAll("Error in COHIPReportManagerDlg::OnShowWindow");
}
