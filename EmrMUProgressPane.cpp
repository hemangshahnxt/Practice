#include "stdafx.h"
#include "EmrMUProgressPane.h"
#include "CCHITReportsLoadData.h"
#include "EmrFrameWnd.h"
#include "PicContainerDlg.h"
#include "EmrMUPatientMeasuresDlg.h" //(e.lally 2012-02-28) PLID 48265
#include "EmrMUProgressSetupDlg.h" //(e.lally 2012-03-26) PLID 48264
#include "EMN.h"

//(e.lally 2012-02-23) PLID 48016 - Created
IMPLEMENT_DYNCREATE(CEmrMUProgressPane, CEMRProgressPane)

BEGIN_MESSAGE_MAP(CEmrMUProgressPane, CEMRProgressPane)
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()

	ON_MESSAGE(NXM_CCHIT_REPORTS_ADD_ROW, OnProcessingAddMeasureResult)
	ON_MESSAGE(NXM_CCHIT_REPORTS_PROCESSING_FINISHED, OnProcessingMeasuresFinished)

	ON_BN_CLICKED(ID_EMR_PROGRESS_PANE_STATUS_LABEL, OnShowMUPatientMeasures)
	ON_BN_CLICKED(ID_EMR_PROGRESS_PANE_SHOW_DETAILS, OnShowMUPatientMeasures)
	ON_BN_CLICKED(ID_EMR_PROGRESS_PANE_CONFIGURE, OnConfigure)

	////
	/// UI State overrides
	ON_UPDATE_COMMAND_UI(ID_EMR_PROGRESS_PANE_STATUS_LABEL, &CEmrMUProgressPane::OnUpdateStatusLabel)
	ON_UPDATE_COMMAND_UI(ID_EMR_PROGRESS_PANE_SHOW_DETAILS, &CEmrMUProgressPane::OnUpdateShowDetails) //(e.lally 2012-04-23) PLID 48016

END_MESSAGE_MAP()


void CEmrMUProgressPane::OnUpdateStatusLabel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	if(m_bIsLoading == false){
		if(m_nPatientMeassuresComplete <= m_nPatientMeasuresTotal){
			SetStatusText(FormatString("%li of %li measures fulfilled", m_nPatientMeassuresComplete, m_nPatientMeasuresTotal));
			SetProgressBar(m_nPatientMeassuresComplete, m_nPatientMeasuresTotal);
		}
		else {
			//We have an invalid completed or total measures count
			SetStatusText("");
			SetProgressBar(0, 0);
		}
	}
}

void CEmrMUProgressPane::OnUpdateShowDetails(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-23) PLID 48016 - Disable when the thread is running to avoid showing partial results
	pCmdUI->Enable(m_hStopThread == NULL ? TRUE : FALSE);
}

//(e.lally 2012-02-28) PLID 48016 - Construct the cchitReportListing member variable here too
CEmrMUProgressPane::CEmrMUProgressPane() : m_cchitReportListing((CWnd*)this)
{
	try {
		m_hStopThread = NULL;
		m_pLoadingThread = NULL;
		m_nPatientMeassuresComplete = 0;
		m_nPatientMeasuresTotal = 0;
		m_bIsLoading = true;
		m_dwCurrentTickcountID = 0;
	}NxCatchAll(__FUNCTION__);
}

CEmrMUProgressPane::~CEmrMUProgressPane()
{
	try {
		//(e.lally 2012-02-28) PLID 48016 - Ensure our thread is no longer running
		KillLoadingThread();
	}NxCatchAll(__FUNCTION__);
}



int CEmrMUProgressPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try {
		if(CEMRProgressPane::OnCreate(lpCreateStruct) == -1){
			return -1;
		}
		
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (e.lally 2012-03-14) PLID 48891 - reruns all the report queries and calculations
//	and updates the interface with the results
//(e.lally 2012-04-03) PLID 48891 - Added bForceRefresh
void CEmrMUProgressPane::RecalculateMeasures(bool bForceRefresh /* = false */)
{
	CNxPerform nxp(__FUNCTION__);
	if(m_hStopThread != NULL && bForceRefresh){
		//Kill the thread
		KillLoadingThread();
	}

	if(m_hStopThread == NULL){
		m_bIsLoading = true;
		m_dwCurrentTickcountID = GetTickCount();

		m_nPatientMeassuresComplete = 0;
		m_nPatientMeasuresTotal = 0;
		m_cchitReportListing.m_aryReports.RemoveAll();
		m_aryReportIDs.RemoveAll();
		m_cchitReportListing.LoadReportListing(MU::Stage1);

		// (e.lally 2012-04-05) PLID 49378 - Load the configuration settings
		CArray<long,long> aryProviders;
		CEMN* pEMN = GetPicContainer()->GetActiveEMN();
		if(pEMN == NULL){
			// (e.lally 2012-04-04) - The caller is assuming we have an active EMN set in memory, but we do not.
			//KNOWN ISSUE:
				//This case can happen if you delete all the EMNs from the chart, but I am leaving the assertion
				//to help notify a developer if there is an unexpected case for this.
			ASSERT(FALSE);
			return;
		}
		pEMN->GetProviders(aryProviders);

		ADODB::_RecordsetPtr rs = NULL;
		if(aryProviders.GetSize() > 0){
			rs = CreateParamRecordset(GetRemoteDataSnapshot(), 
				"SELECT GETDATE() AS CurrentTime "
				"\r\n"
				"SELECT DISTINCT MeasureNum, Selected "
				"FROM ProviderMUMeasureSelectionT "
				"WHERE ProviderID IN({INTARRAY}) AND Selected = 1 "
				"\r\n"
				"SELECT DISTINCT DateOptionNum, StartDate, ExcludeSecondaryProv "
				"FROM ProviderMUMeasureOptionT "
				"WHERE ProviderID IN({INTARRAY}) ", aryProviders, aryProviders);
		}
		else {
			//If no provider is on the EMN, assume we want to do the calculations based on all provider configurations
			rs = CreateParamRecordset(GetRemoteDataSnapshot(), 
				"SELECT GETDATE() AS CurrentTime "
				"\r\n"
				"SELECT DISTINCT MeasureNum, Selected "
				"FROM ProviderMUMeasureSelectionT "
				"WHERE Selected = 1 "
				"\r\n"
				"SELECT DISTINCT DateOptionNum, StartDate, ExcludeSecondaryProv "
				"FROM ProviderMUMeasureOptionT ");
		}
		nxp.Tick("Queried providers");

		COleDateTime dtCurrentDate = COleDateTime::GetCurrentTime();
		if(!rs->eof){
			//Get this from the server while we're at it
			dtCurrentDate = VarDateTime(rs->Fields->Item["CurrentTime"]->Value);
		}
		rs = rs->NextRecordset(NULL);

		//truncate time off the current date
		dtCurrentDate.SetDate(dtCurrentDate.GetYear(), dtCurrentDate.GetMonth(), dtCurrentDate.GetDay());


		BOOL bCCHITHidden = TRUE;
		BOOL bExcludeSecondaries = FALSE;
		CString strProvIDs = ArrayAsString(aryProviders, true);
		CString strProviderList;
		if(!strProvIDs.IsEmpty()){
			strProviderList = " ("+ strProvIDs + ") ";
		}
		CString strLocationList; //Not filtering by location at this time
		CString strExclusionTemplateList = GetRemotePropertyMemo("MU_ExcludedTemplates", 0, 0, "<None>");
		COleDateTime dtTo = dtCurrentDate; //Date range end
		COleDateTime dtFrom = dtCurrentDate; //date range start

		//Calculate x to date values
		COleDateTime dtYearToDtStart(dtCurrentDate.GetYear(), 1, 1, 0, 0, 0);
		COleDateTime dtQtrToDtStart;
		if(dtCurrentDate.GetMonth() >= 1 && dtCurrentDate.GetMonth() <= 3){
			dtQtrToDtStart = COleDateTime(dtCurrentDate.GetYear(), 1, 1, 0, 0, 0);
		}
		else if(dtCurrentDate.GetMonth() >= 4 && dtCurrentDate.GetMonth() <= 6){
			dtQtrToDtStart = COleDateTime(dtCurrentDate.GetYear(), 4, 1, 0, 0, 0);
		}
		else if(dtCurrentDate.GetMonth() >= 7 && dtCurrentDate.GetMonth() <= 9){
			dtQtrToDtStart = COleDateTime(dtCurrentDate.GetYear(), 7, 1, 0, 0, 0);
		}
		else {
			dtQtrToDtStart = COleDateTime(dtCurrentDate.GetYear(), 10, 1, 0, 0, 0);
		}

		COleDateTime dtMonthToDtStart(dtCurrentDate.GetYear(), dtCurrentDate.GetMonth(), 1, 0, 0, 0);
		
		
		while(!rs->eof){
			//Use the most inclusive set of options from all results
			long nMeasureNum = VarLong(rs->Fields->Item["MeasureNum"]->Value);
			m_aryReportIDs.Add(nMeasureNum);

			rs->MoveNext();
		}
		rs = rs->NextRecordset(NULL);

		long nDateOption = -1;
		COleDateTime dtStart = g_cdtInvalid;
		BOOL bExcludeSec = FALSE;
		BOOL bHasAllExclude = TRUE;
		BOOL bHasAtLeastOneExclude = FALSE;
		while(!rs->eof){
			//Use the most inclusive set of options from all results
			nDateOption = VarLong(rs->Fields->Item["DateOptionNum"]->Value);
			dtStart = VarDateTime(rs->Fields->Item["StartDate"]->Value, g_cdtInvalid);
			switch(nDateOption){
				case CEmrMUProgressSetupDlg::mudvCustom:
					if(dtStart.m_status == COleDateTime::valid && dtStart.m_dt >0 && dtFrom > dtStart){
						dtFrom = dtStart;
					}
					break;
				case CEmrMUProgressSetupDlg::mudvThisMonthToDate:
					if(dtFrom > dtMonthToDtStart){
						dtFrom = dtMonthToDtStart;
					}
					break;
				case CEmrMUProgressSetupDlg::mudvThisQuarterToDate:
					if(dtFrom > dtQtrToDtStart){
						dtFrom = dtQtrToDtStart;
					}
					break;
				case CEmrMUProgressSetupDlg::mudvThisYearToDate:
					if(dtFrom > dtYearToDtStart){
						dtFrom = dtYearToDtStart;
					}
					break;
			}
			
			if(!AsBool(rs->Fields->Item["ExcludeSecondaryProv"]->Value)){
				bHasAllExclude = FALSE;
			}
			else {
				bHasAtLeastOneExclude = TRUE;
			}

			rs->MoveNext();
		}
		rs->Close();

		if(bHasAtLeastOneExclude && bHasAllExclude){
			bExcludeSecondaries = TRUE;
		}


		//(e.lally 2012-02-28) PLID 48016 - call our thread using the filters above
		m_hStopThread = CreateEvent(NULL, TRUE, FALSE, NULL);
		CCCHITReportsLoadData* pData = new CCCHITReportsLoadData();		
		pData->hwndParent = GetSafeHwnd();
		pData->hStopThread = m_hStopThread;
		//Reports to do calculations for
		pData->paryReports = &m_cchitReportListing.m_aryReports;
		pData->bFilterReportListByID = true;
		pData->aryReportIDs.Copy(m_aryReportIDs);
		//Filters
		pData->bCCHITHidden = bCCHITHidden;
		pData->bExcludeSecondaries = bExcludeSecondaries;
		pData->dtFrom = dtFrom;
		pData->dtTo = dtTo;
		pData->strLocationList = strLocationList;
		pData->strProviderList = strProviderList;
		pData->strExclusionTemplateList = strExclusionTemplateList;
		pData->nPatientID = GetPicContainer()->GetPatientID();
		pData->dwTickcountID = m_dwCurrentTickcountID;
		if(pData->nPatientID == -1){
			//We didn't get a valid patient ID
			ASSERT(FALSE);
			AfxThrowNxException("Patient ID was not loaded prior to Meaningful Use progress bar initialization");
		}

		m_pLoadingThread = pData->BeginCalculationThread();
	}
}


LRESULT CEmrMUProgressPane::OnProcessingAddMeasureResult(WPARAM wParam, LPARAM lParam)
{
	try {
		//(e.lally 2012-02-28) PLID 48016 - For each result, check if it has a single count for the denominator or numerator and increment our control counters accordingly
		long nIndex = (long)wParam;		

		CCCHITReportInfo &riReport = m_cchitReportListing.m_aryReports.ElementAt(nIndex);
		//If this message came from an old thread, we can ignore it. A new thread is already running.
		if(m_dwCurrentTickcountID != (DWORD)lParam){
			return 0;
		}
		
		if(riReport.GetReportType() == crtMU){
			if(!riReport.HasBeenCalculated()){
				//This indicates the report listing has been reset already so we are about to fail.
				ASSERT(FALSE);
			}
			//(e.lally 2012-03-21) PLID 48707 - Rework calculations to account for some reports
			//	that are not patient based and need a certain percentage met to pass.
			long nNumerator = riReport.GetNumerator();
			long nDenominator = riReport.GetDenominator();
			long nPercentToPass = riReport.GetPercentToPass(); 

			//Check if the percent to pass is unset. If so, the numerator should be 0 or 1, counting just this patient
			if(nPercentToPass < 0 && nNumerator >= 1){
				m_nPatientMeassuresComplete++;
				//Assert that we didn't get results for more than one patient or use a count of records that are not a patient count. e.g. prescriptions
				ASSERT(nNumerator == 1);
			}
			//Check if the percent to pass is set, if so, check if numerator over denominator is greater than this percentage
				//This is used for measures like "prescriptions sent electronically". The current patient may have 5 Rx but only 3 were sent electronically.
				//We are only going to count this patient as fulfilling the requirement if they meet the overall percentage being tested against.
			//NOTE that the percent has to be MORE than percentToPass in order to qualify. So nPercentToPass = 40% really needs 40.1% (not sure what precision is actually used here) to be accepted
			else if(nPercentToPass >= 0 && nDenominator > 0 && ((nNumerator/(float)nDenominator)*100) > nPercentToPass){
				m_nPatientMeassuresComplete++;
			}

			if(nPercentToPass < 0 && nDenominator >= 1){
				m_nPatientMeasuresTotal++;
				//Assert that we didn't get results for more than one patient or use a count of records that are not a patient count. e.g. prescriptions
				ASSERT(nDenominator == 1);
			}
			else if(nPercentToPass > 0 && nDenominator >= 1){
				m_nPatientMeasuresTotal++;
			}
		}
		

	}NxCatchAll(__FUNCTION__);
	return 0;
}

//(e.lally 2012-02-28) PLID 48016 - Handles the fired message that the thread completed all it set out to
LRESULT CEmrMUProgressPane::OnProcessingMeasuresFinished(WPARAM wParam, LPARAM lParam)
{
	try {
		BOOL bEndedEarly = (BOOL)wParam;
		//If this message came from an old thread, we can ignore it. A new thread is already running so we don't need to try to stop the old one.
		if(m_dwCurrentTickcountID != (DWORD)lParam){
			return 0;
		}
		m_bIsLoading = false;

		//Assert that we have a valid completed vs total measures count
		ASSERT(m_nPatientMeassuresComplete <= m_nPatientMeasuresTotal);

		//delete the thread
		KillLoadingThread();
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

//(e.lally 2012-02-28) PLID 48016 - Stops the MU measure calculation thread
void CEmrMUProgressPane::KillLoadingThread() 
{
	if (m_pLoadingThread) 
	{
		// Set the event to break the loop in the thread
		SetEvent(m_hStopThread);

		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pLoadingThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so, terminate it
			m_pLoadingThread->m_bAutoDelete = TRUE;
			PostThreadMessage(m_pLoadingThread->m_nThreadID, WM_QUIT, 0, 0);			
			WaitForSingleObject(m_pLoadingThread->m_hThread, INFINITE);
		}	
		else {
			// The thread is finished, so just delete it
			delete m_pLoadingThread;
		}
	
		m_pLoadingThread = NULL;
		CloseHandle(m_hStopThread);
		m_hStopThread = NULL;
	}
}

//(e.lally 2012-02-28) PLID 48265 - Shows which measure are complete and incomplete for this patient
void CEmrMUProgressPane::OnShowMUPatientMeasures()
{
	try {
		if(m_hStopThread != NULL){
			//(e.lally 2012-04-27) PLID 48016 - Thread is still running, do not open this dlg.
			//	(Clicking the status label can get us here.)
			return;
		}
		CString strPatName = GetPicContainer()->GetPatientName();
		CString strWindowTitle = FormatString("Meaningful Use Measures for %s", strPatName);
		CEmrMUPatientMeasuresDlg dlg((CWnd*)this, strWindowTitle, &m_cchitReportListing);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);

}

//(e.lally 2012-03-26) PLID 48264 - Opens the configuration screen (per provider)
void CEmrMUProgressPane::OnConfigure()
{
	try {

		CEmrMUProgressSetupDlg dlg((CWnd*)this);
		//(e.lally 2012-04-05) PLID 48264 - Get the list of providers on this EMN. Let's default to the first one.
		CEMN* pEMN = GetPicContainer()->GetActiveEMN();
		if(pEMN != NULL){
			CArray<long,long> aryProviders;
			pEMN->GetProviders(aryProviders);
			if(aryProviders.GetCount() > 0){
				//Even if there are multiple, we'll load the first one in the array.
				dlg.SetInitialProviderID(aryProviders.GetAt(0));
			}
		}

		dlg.DoModal();
		//(e.lally 2012-04-03) PLID 48891 - Recalculate measures in case the configuration changed
		RecalculateMeasures(true);

	}NxCatchAll(__FUNCTION__);
}