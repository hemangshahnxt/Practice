#include "stdafx.h"
#include "CCHITReportsLoadData.h"


//(e.lally 2012-02-24) PLID 48266 - Moved to a more global location
UINT CalculateMeasures(LPVOID lpData)
{
	CNxPerform nxp(__FUNCTION__);
	try {
	
		CCCHITReportsLoadData* pData = (CCCHITReportsLoadData*)lpData;

		try {	
			// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification
			CNxAdoConnection pConn = CreateThreadConnection();
			CNxAdoConnection pConnSnap = CreateThreadSnapshotConnection();

			// (j.gruber 2011-05-13 17:12) - PLID 43694 - don't run cchit if they aren't showing
			BOOL bCCHITHidden = pData->bCCHITHidden;

			// (j.gruber 2011-09-27 10:04) - PLID 45616
			BOOL bExcludeSecondaries = pData->bExcludeSecondaries;

			long nCurRecord = 0;
			BOOL bDefaultRun = FALSE;
			long nDefaultDenomValue = -1;

			//(e.lally 2012-04-05) PLID 49378 - Map the list of report IDs needing to be run for faster lookip
			CMap<long, long, BOOL, BOOL> mapReportFilterByID;
			for(int i =0; i < pData->aryReportIDs.GetSize(); i++){
				mapReportFilterByID.SetAt(pData->aryReportIDs.ElementAt(i), TRUE);
			}

			// (j.gruber 2011-11-17 10:46) - PLID 44993 - variable for ending early
			BOOL bEndingEarly = FALSE;
			for(int i = 0; i < pData->paryReports->GetSize(); i++) {

				if (WaitForSingleObject(pData->hStopThread, 0) != WAIT_TIMEOUT) {
					bEndingEarly = TRUE;
					break;
				}

				//Report data to load
				CCCHITReportInfo &riReport = pData->paryReports->ElementAt(i);

				BOOL bRunReportCalc = TRUE;
				//(e.lally 2012-04-05) PLID 49378 - See if we are filtering by ID, then lookup the ID to see if we need to run that report.
				ASSERT(riReport.m_nInternalID != -1);
				if(pData->bFilterReportListByID){
					BOOL bVal = FALSE;
					if(mapReportFilterByID.Lookup(riReport.m_nInternalID, bVal) ){
						bRunReportCalc = bVal;
					}
					else {
						bRunReportCalc = FALSE;
					}
				}


				// (j.gruber 2011-11-01 11:56) - PLID 46219  - set the connection
				ADODB::_ConnectionPtr pConnToSend = NULL;
				if (riReport.m_bCanUseSnapshot) {
					pConnToSend = pConnSnap;
				}
				else {
					pConnToSend = pConn;
				}

				//Must calculate with the given date range
				// (j.gruber 2011-05-12 11:40) - PLID 43676 - add provider			
				//(e.lally 2012-04-05) PLID 49378 - Skip if we aren't set to run this report calculation
				if (((bCCHITHidden) && riReport.GetReportType() == crtMU && bRunReportCalc) ||
					(!bCCHITHidden && bRunReportCalc) 
					) {

					if (riReport.UseDefaultDenominator()) {
						//have we already used it?
						if (!bDefaultRun) {
							//calculate everything
							// (j.gruber 2011-11-07 12:23) - PLID 45365 - added exclusion list
							//(e.lally 2012-02-24) PLID 48268 - Added nPatientID
							riReport.Calculate(pConnToSend, pData->dtFrom, pData->dtTo, pData->strProviderList, pData->strLocationList, pData->strExclusionTemplateList, bExcludeSecondaries, -1, pData->nPatientID);
							bDefaultRun = TRUE;
							nDefaultDenomValue = riReport.GetDenominator();
						}
						else {		
							// (j.gruber 2011-09-27 10:04) - PLID 45616
							// (j.gruber 2011-10-27 16:41) - PLID 46160 - send flag to not run denominator
							// (j.gruber 2011-11-01 11:56) - PLID 46219 - add connection
							// (j.gruber 2011-11-07 12:23) - PLID 45365 - added exclusion list
							//(e.lally 2012-02-24) PLID 48268 - Added nPatientID
							riReport.Calculate(pConnToSend, pData->dtFrom, pData->dtTo, pData->strProviderList, pData->strLocationList, pData->strExclusionTemplateList, bExcludeSecondaries, nDefaultDenomValue, pData->nPatientID);
						}
					}
					else {
						//calculate regularly
						// (j.gruber 2011-11-01 11:56) - PLID 46219 - add connection
						// (j.gruber 2011-11-07 12:23) - PLID 45365 - added exclusion list
						//(e.lally 2012-02-24) PLID 48268 - Added nPatientID
						riReport.Calculate(pConnToSend, pData->dtFrom, pData->dtTo, pData->strProviderList, pData->strLocationList, pData->strExclusionTemplateList, bExcludeSecondaries, -1, pData->nPatientID);
					}

					nxp.Tick("Calculated report");

					//Post our message that we have a row	
					//(e.lally 2012-04-05) PLID 49378 - Pass back the tickcount ID to identify which thread this came from
					::PostMessage(pData->hwndParent, NXM_CCHIT_REPORTS_ADD_ROW, i, pData->dwTickcountID);				
					
				}
			}

			// (j.gruber 2011-11-17 10:47) - PLID 44993 - send whether we are ending early
			//(e.lally 2012-04-05) PLID 49378 - Pass back the tickcount ID to identify which thread this came from
			::PostMessage(pData->hwndParent, NXM_CCHIT_REPORTS_PROCESSING_FINISHED, bEndingEarly, pData->dwTickcountID);
			delete pData;
			pData = NULL;
			return 0;			
		}NxCatchAllThread(__FUNCTION__);

		//we still need to delete the memory
		//we would only get here if wo got an error without releasing our memory, so let's release it
	
		if (pData) {
			delete pData;
			pData = NULL;
		}
	}NxCatchAllThread("CCHITReportDlg::CalculateMeasures Delete Thread");

	return -1;
}


//(e.lally 2012-02-24) PLID 48266 - Made this a more global function
CWinThread* CCCHITReportsLoadData::BeginCalculationThread()
{
	//Assert that the caller defined these
	ASSERT(hStopThread != NULL);
	ASSERT(paryReports!= NULL && paryReports->GetSize() > 0);

	CWinThread* pThread = AfxBeginThread(CalculateMeasures, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	pThread->m_bAutoDelete = false;
	pThread->ResumeThread();
	return pThread;
}