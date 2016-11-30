#pragma once

#include "CCHITReportInfo.h"

// (j.gruber 2011-11-01 10:58) - PLID 46222
//(e.lally 2012-02-24) PLID 48266 - Moved here from CCHITReportsDlg.h
class CCCHITReportsLoadData
{
public:
	CCCHITReportsLoadData()
	{
		paryReports = NULL;
		bCCHITHidden = FALSE;
		hStopThread = NULL;
		bExcludeSecondaries = FALSE;
		dtTo = g_cdtInvalid;
		dtFrom = g_cdtInvalid;	
		nPatientID = -1;
		bFilterReportListByID = false;
		dwTickcountID = 0;
	}
	HWND hwndParent; // The window handle of the CCHITReportsDlg object	
public:
	CArray<CCCHITReportInfo, CCCHITReportInfo&> *paryReports; // pointer to array of Reports
	BOOL bCCHITHidden; //Bool to show only MU measures
	HANDLE hStopThread; // Event to halt the thread
	BOOL bExcludeSecondaries; //bool to exclude secondaries
	CString strProviderList;
	CString strLocationList;
	CString strExclusionTemplateList;
	COleDateTime dtTo; //Date range end
	COleDateTime dtFrom; //Date range start	
	long nPatientID; //(e.lally 2012-02-24) PLID 48268 - Single patient ID to filter on
	bool bFilterReportListByID; //(e.lally 2012-04-05) PLID 49378 - Tells us if the aryReportsIDs is in use.
	CArray<long, long&> aryReportIDs; //(e.lally 2012-04-05) PLID 49378 - List of report internal IDs to run
	DWORD dwTickcountID; //(e.lally 2012-04-05) PLID 49378 - Used to identify if an add/finished message came from the current thread

	CWinThread* BeginCalculationThread();
};
