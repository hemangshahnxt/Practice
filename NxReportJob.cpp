// NxReportJob.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NxReportJob.h"
#include "GlobalUtils.h"
#include "ReportInfo.h"
#include "crpe.h"
#include "GlobalReportutils.h"
#include "AuditTrail.h"
#include "Reports.h"
#include "LabsSetupDlg.h"
#include <GlobalLabUtils.h>
#include "LabUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CNxReportJob


CNxReportJob::CNxReportJob(short jobHandle) :CRPEJob(jobHandle) {

	m_nReportInfoID = -255;
	// (j.gruber 2011-10-11 16:15) - PLID 45937 - default to true
	m_bWriteToHistoryStatus = TRUE;
}


CNxReportJob::CNxReportJob(short jobHandle, CRPEJob *parentJob) : CRPEJob(jobHandle, parentJob) {

	m_nReportInfoID = -255;
	// (j.gruber 2011-10-11 16:15) - PLID 45937 - default to true
	m_bWriteToHistoryStatus = TRUE;

}

CNxReportJob::~CNxReportJob()
{
	// (j.jones 2005-09-08 17:08) - this is one function that will be hit
	// when a report is closed by the user
	//
	// When a report is closed, 
	//    CNxRPEMDIChildWnd::~CNxRPEMDIChildWnd() is fired,
	// which deletes the report job, causing both
	//    CNxReportJob::~CNxReportJob() and
	//    CRPEJob::~CRPEJob() to fire in succession
	// and ~CRPEJob will call CRPEngine::RemoveJob()
	//
}

BOOL CNxReportJob::OutputToPrinter(short nCopies /* = 1 */, CPrintInfo *pInfo /* =0*/)
{

#ifdef _DEBUG
	Log("CNxReportJob::OutputToPrinter()");
#endif

	BOOL bAns;

	// (a.walling 2009-07-02 13:12) - PLID 14181 - If we were not passed in a CPrintInfo structure,
	// create a default one. But ASSERT since the callers really should be doing this, at least
	// that is what I assume.

	CPrintInfo prDefault;

	if (!pInfo) {
		ASSERT(FALSE);

		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		prDefault.m_bPreview = false;
		prDefault.m_bDirect = false;
		prDefault.m_bDocObject = false;
		if(prDefault.m_pPD != NULL) {
			delete prDefault.m_pPD;
		}
		prDefault.m_pPD = dlg;

		pInfo = &prDefault;
	}

	HGLOBAL hDevMode = NULL, hDevNames = NULL;
	if(pInfo != NULL)
	{
		// (z.manning 2010-05-20 10:28) - PLID 37876 - If they are printing the lab specimen label
		// report then default to using those print settings (we still want to prompt from here).
		if(m_nReportInfoID == 654) {
			// (z.manning 2010-11-22 12:18) - PLID 40486 - New function for this
			// Retrieve the label printer settings based on the 
			// System Property Specificity preference
			LabUtils::GetLabLabelPrintSettings(hDevNames, hDevMode);
			pInfo->m_pPD->m_pd.hDevMode = hDevMode;
			pInfo->m_pPD->m_pd.hDevNames = hDevNames;
		}
	}
	
	// Run print dialog
	if (pInfo) {
		//disable the print to file checkbox (can't figure out how to get the file name, they can
		//always export the thing if necessary).
		//also disable the print selection option (meaningless)
		pInfo->m_pPD->m_pd.Flags |= PD_HIDEPRINTTOFILE|PD_NOSELECTION;
		
		/* TODO: A complicated problem.  Ask yourself what SHOULD happen under the following conditions:
		    - The user is printing a batch of two reports AND
		    - The first report in the batch has the page orientation stored as landscape AND
		    - The second report does not store a specific orientation, it uses the default.
		 Here are the possibilities
		  1. The user gets prompted with the print settings dialog for each report.  The first time 
		     the orientation settings says "landscape" and can be changed.  The second time the 
		     orientation says whatever the current app default orientation is, and can be changed.
		  2. The user gets prompted once with print settings for the whole set of reports.  The 
		     orientation option says "landscape", but the user must be trained to know that this 
		     orientation setting will only apply to the first report.  The second report uses the 
		     current app default.
		
		 These are the only two options, unless we invent a third kind of orientation called "use default"
		 
		 Below is the work-around that allows option 2 to work
		*/
		// By convention, we have decided to make it so the user only gets the PrintDialog prompt on the 
		// first report in the batch.  Therefore we need to make it revert to the app default orientation 
		// on the second, third, etc report in the batch.
		// (a.walling 2010-10-19 12:24) - PLID 40991 - Although this is not that elegant of a solution, since
		// we are already handling the specimen label report in a special manner above, I suppose it is OK
		// to do the same here.
		// So, if we are using the specimen label report, do not mess with the orientation!
		if (pInfo->m_bDirect && m_nReportInfoID != 654) {
			// Revert to the app's page orientation
			// Get the app's device settings
			CPrintInfo PrintInfoApp;
			{
				// Get the defaults from the app
				PrintInfoApp.m_pPD->m_pd.Flags |= PD_RETURNDEFAULT;
				PrintInfoApp.m_pPD->m_pd.hDevMode = (HANDLE)0;
				PrintInfoApp.m_pPD->m_pd.hDevNames = (HANDLE)0;
				// This loads the app's settings instead of the system's defaults
				AfxGetApp()->DoPrintDialog(PrintInfoApp.m_pPD);
			}
			DEVMODE *pAppDevMode = (DEVMODE *)GlobalLock(PrintInfoApp.m_pPD->m_pd.hDevMode);
			DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
			pInfoDevMode->dmPosition = pAppDevMode->dmPosition;
			GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);
			GlobalUnlock(PrintInfoApp.m_pPD->m_pd.hDevMode);
		}

		// Set the job's print settings, setting the pInfo object's device settings as well, if necessary
		GatherPrintInfo(pInfo);

#ifdef _DEBUG
		// At this point we are guaranteed to have already set our report to the correct device 
		// settings, because there's no other way the CPrintInfo structure could be set.
		// I'm asserting this fact in this _DEBUG block
		{
			DEVMODE *pRptDevMode = NULL;
			CString strDontCare;
			GetSelectedPrinter(strDontCare, strDontCare, strDontCare, &pRptDevMode);

			//DRT 3/13/03 - Commented this out.  After much investigation, over several machines and all operating systems
			//		(98, NT, 2000, XP), Windows handles trying to print with no printers selected.  In XP it prompts you to add
			//		a printer on the spot.  In the rest, it tells you how to go about adding a new printer.  I've changed other code
			//		above and in globalutils that allows the printing to get this far w/o throwing an exception, because it was
			//		really failing to print in cases where it WOULD have printed successfully!  This is just a debug assertion, 
			//		but both Crystal previewing and Windows printing handle this case (so far as my tests have concluded) - so 
			//		no need to throw an assertion here.
			//ASSERT(pRptDevMode);
		}
#endif

		//DRT 3/13/03 - TODO - Invoking the print dialog with no printers installed prompts you twice if you'd like to install a printer.  
		//		If you step into it, the code in CPrintDialog::DoModal() is calling 	int nResult = ::PrintDlg(&m_pd);, which pops
		//		up that message twice.

		// Let the user change their settings as desired
		if (pInfo->m_bPreview || pInfo->m_bDirect ||
			(pInfo->m_bDocObject && !(pInfo->m_dwFlags & PRINTFLAG_PROMPTUSER)) ||
			pInfo->m_pPD->DoModal() == IDOK) {

			// (a.walling 2011-06-17 13:33) - PLID 34999 - Ensure we set a parent window
			SetDialogParentWindow(CWnd::GetSafeOwner(NULL, NULL));

			// Take the user's changes into account
			short nCopies = ApplyPrintInfo(pInfo);

			// Now we can print
			AuditPrinting(); // (a.walling 2009-06-01 17:02) - PLID 34240
			bAns = CRPEJob::OutputToPrinter(nCopies);
			EndReportPrinting(this);

		} else {
			// The user cancelled
			bAns = FALSE;
		}
	} else {
		ASSERT(FALSE);

		AuditPrinting(); // (a.walling 2009-06-01 17:02) - PLID 34240
		bAns = CRPEJob::OutputToPrinter(nCopies);
		EndReportPrinting(this);

		if(!bAns) {
			MsgBox("There was an error printing the document.  Please ensure your printers are correctly setup.");
		}

	}

	if(hDevMode != NULL) {
		GlobalFree(hDevMode);
		hDevMode = NULL;
	}
	if(hDevNames != NULL) {
		GlobalFree(hDevNames);
		hDevNames = NULL;
	}

	// Now we're done
	return bAns;
}

BOOL CNxReportJob::SetAllLogonInfo(CRPELogOnInfo *pLogonInfo)
{
	// Set all report table logon information
	short nTableCount = GetNTables();
	for (short iTable=0; iTable<nTableCount; iTable++) {
		if (!SetNthTableLogonInfo(iTable, pLogonInfo, TRUE)) {
			return FALSE;
		}
	}

	// Loop through all sections 
	short nCode;
	short nSubRepCount, iSubRep;
	CRPESubreportInfo sriSubRep;
	CNxReportJob *pSubRepJob;
	short nSectionCount = GetNSections();
	for (short iSection=0; iSection<nSectionCount; iSection++) {
		nCode = GetSectionCode(iSection);
		// Loop through all subreports for each section
		nSubRepCount = GetNSubreportsInSection(nCode);
		for (iSubRep=0; iSubRep<nSubRepCount; iSubRep++) {
			if (GetSubreportInfo(GetNthSubreportInSection(nCode, iSubRep), &sriSubRep)) {
				// Open the subreport
				pSubRepJob = (CNxReportJob *)OpenSubreportJob(sriSubRep.m_name);
				if (pSubRepJob) {
					// Assuming it was opened successfully, set IT'S logon info
					pSubRepJob->SetAllLogonInfo(pLogonInfo);
					// And then close it
					pSubRepJob->Close();
				} else {
					// We couldn't open the subreport so fail
					return FALSE;
				}
			} else {
				// We couldn't get the subreport info so fail
				return FALSE;
			}
		}
	}

	// If we made it to here succeed
	return TRUE;
}

/*
void CNxReportJob::Close()
{


	CRPEJob::Close();
}*/

BOOL CNxReportJob::SetDataSource(const CReportInfo *pReport, OPTIONAL OUT _Recordset **ppRecordsetOut /*= NULL*/)
{
	return SetDataSource(pReport, 0, 0, ppRecordsetOut);
}


BOOL CNxReportJob::SetDataSource(const CReportInfo *pReport, long nSubLevel, long nSubRepNum, OPTIONAL OUT _Recordset **ppRecordsetOut /*= NULL*/)
{
	try {

		// Set the report to use the recordset
		//(e.lally 2008-04-07) PLID 9989 - Unset the flag for when we are verifying reports.
		BOOL bForReportVerify = FALSE;
		m_rsReportInfo = pReport->GetRecordset(nSubLevel, nSubRepNum, bForReportVerify);
		IDispatch *pDataPtr = (IDispatch *)m_rsReportInfo; // This increments the reference count
		PETablePrivateInfo pi;

		//set the report info id in the report job
		m_nReportInfoID = pReport->nID;
		
		memset(&pi, 0, PE_SIZEOF_TABLE_PRIVATE_INFO);
		pi.StructSize = PE_SIZEOF_TABLE_PRIVATE_INFO;
		pi.nBytes = sizeof(IDispatch FAR *);
		pi.tag = 3;
		pi.dataPtr = (BYTE FAR *)(&pDataPtr);
		
		if (!PESetNthTablePrivateInfo(GetJobHandle(), 0, &pi)) {
			AfxThrowNxException("Error %li starting report job: \n\n%s", 
				GetErrorCode(), GetErrorText());
		}
	} NxCatchAllCall("CNxReportJob::SetDataSource " + FormatString("SubLevel: %li, SubRepNum: %li", nSubLevel, nSubRepNum), {
		m_rsReportInfo.Release();
		Close();
		return FALSE;
	});

	// Loop through all sections 
	short nCode;
	short nSubRepCount, iSubRep;
	CRPESubreportInfo sriSubRep;
	CNxReportJob *pSubRepJob;
	short nSectionCount = GetNSections();
	long nReportNum = 0;
	for (short iSection=0; iSection<nSectionCount; iSection++) {
		nCode = GetSectionCode(iSection);
		// Loop through all subreports for each section
		nSubRepCount = GetNSubreportsInSection(nCode);
		for (iSubRep=0; iSubRep<nSubRepCount; iSubRep++) {
			if (GetSubreportInfo(GetNthSubreportInSection(nCode, iSubRep), &sriSubRep)) {
				// Open the subreport
				pSubRepJob = (CNxReportJob *)OpenSubreportJob(sriSubRep.m_name);
				if (pSubRepJob) {
					// Assuming it was opened successfully, set IT'S data source
					pSubRepJob->SetDataSource(pReport, nSubLevel+1, nReportNum++);
					// And then close it
					pSubRepJob->Close();
				} else {
					// We couldn't open the subreport so fail
					return FALSE;
				}
			} else {
				// We couldn't get the subreport info so fail
				return FALSE;
			}
		}
	}

	// (j.gruber 2011-10-11 15:44) - PLID 45937 - Set WriteToHistoryStatus
	m_bWriteToHistoryStatus = pReport->GetWriteToHistoryStatus();

	// If we made it to here succeed
	if (ppRecordsetOut != NULL) {
		*ppRecordsetOut = m_rsReportInfo;
		m_rsReportInfo.AddRef();
	}
	return TRUE;
}

// This function sets the report to use the correct print settings (printer, page orientation, etc.) according to the following rules (3 is an exception)
//   1. If the pInfo DOES NOT have device settings already, fill the pInfo with a copy of the app's device settings
//   2. Now that pInfo is guaranteed to have device settings, use a copy of pInfo's device settings for the rpt file (3 is an exception)
//   3. If the RPT file was designed with the "Use Default" option UNCHECKED, use the following fiedls from rpt file's device settings:
//      - page orientation
//
// TODO: We might want to copy other settings besides page positioning (orientation) info; see todo within this GatherPrintInfo function
void CNxReportJob::GatherPrintInfo(IN OUT CPrintInfo *pInfo)
{
	BOOL bNewInfoDevMode = FALSE, bNewDevNames = FALSE;
	
	// 1. If the pInfo DOES NOT have device settings already, fill the pInfo with a copy of the app's device settings
	if (pInfo->m_pPD->m_pd.hDevMode == NULL || pInfo->m_pPD->m_pd.hDevNames == NULL) {
		// Get a copy of the app's device settings, we will use these to set the report's device settings after making our adjustments
		DEVMODE *pFinDevMode = NULL;
		LPTSTR strPrinter = NULL, strDriver = NULL, strPort = NULL;
		
		AllocCopyOfAppDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);

		// dev MODE
		if (pInfo->m_pPD->m_pd.hDevMode == NULL) {
			// The devmode object isn't set, so set it to a copy of the app's
			//DRT 3/13/03 - but only if the app has one!
			if(pFinDevMode) {
				pInfo->m_pPD->m_pd.hDevMode = AllocDevModeCopy(pFinDevMode);
				bNewInfoDevMode = TRUE;
			}
		}
		// dev NAMES
		if (pInfo->m_pPD->m_pd.hDevNames == NULL) {
			// The devnames object isn't set, so set it to a copy of the app's
			//DRT 3/13/03 - but only if the app has one!
			if(strPrinter != NULL && strDriver != NULL && strPort != NULL) {
				pInfo->m_pPD->m_pd.hDevNames = AllocDevNamesCopy(strPrinter, strDriver, strPort);
				bNewDevNames = TRUE;
			}
		}

		//DRT 3/13/03 - If either of our booleans is still false, we've got some problems setting up our printing.  Quit trying
		//		to copy things, it's all going to fail.  By this point we've already tried to load the applications printer
		//		settings, and tried to just get the default system settings.  If these are false, both of those have failed.
		//		And, equally important, nothing has been allocated.
		if(!bNewInfoDevMode || !bNewDevNames) {
			return;
		}

		// We're done with our copies because we've made a second set of copies and stored them in the pInfo object
		FreeCopyOfDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);
	}

	// Now the pInfo is set, leave it as is, but grab A COPY of its settings, for use in the report job
	DEVMODE *pFinDevMode = NULL;
	LPTSTR strPrinter = NULL, strDriver = NULL, strPort = NULL;
	AllocCopyOfDeviceSettings(pInfo, &pFinDevMode, &strPrinter, &strDriver, &strPort);
	// This just allocates memory and fills it, it should always set our pointers to the addresses of the copied memory
	ASSERT(pFinDevMode && strPrinter && strDriver && strPort);

	// Load the report's current devmode object; if it has a devmode object, we're going to grab the page orientation out of it
	{
		// If the report has a defined page orientation, use it; otherwise leave as is
		DEVMODE *pRptDevMode;
		CString strDontCare; // Notice we don't care about the device names because we know we're going to use the app's device names
		GetSelectedPrinter(strDontCare, strDontCare, strDontCare, &pRptDevMode);
		if (pRptDevMode) {
			// The report has device settings, use the its page orientation
			// TODO: We might want to copy other settings besides page positioning (orientation) info
			pFinDevMode->dmPosition = pRptDevMode->dmPosition;

			// If we were responsible for filling the pInfo devmode object's in the first place, then ALSO set it's dmPosition
			if (bNewInfoDevMode) {
				DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
				ASSERT(pInfoDevMode);
				pInfoDevMode->dmPosition = pRptDevMode->dmPosition;
				GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);
			}
		}
	}

	// Tell the report to use this new devmode object (basically just a copy of the app's), and an exact copy of the app's device names
	SelectPrinter(strDriver, strPrinter, strPort, pFinDevMode);

	// We're done with our copies because we've subitted them to the job (the job made its own copy)
	FreeCopyOfDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);
}

// Sets the job's printoptions based on the given pInfo object
// Returns the number of copies to be printed (based on the pInfo's devmode object)
short CNxReportJob::ApplyPrintInfo(IN OUT CPrintInfo *pInfo)
{
	short nCopies = 1;
	BOOL bCollate = FALSE;

	ASSERT(pInfo->m_pPD->m_pd.hDevMode && pInfo->m_pPD->m_pd.hDevNames);

	// Set the report to use a copy of the pInfo device settings, exactly as they are
	{
		DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
		DEVNAMES *pInfoDevNames = (DEVNAMES *)GlobalLock(pInfo->m_pPD->m_pd.hDevNames);

		SelectPrinter(
			((LPTSTR)pInfoDevNames) + pInfoDevNames->wDriverOffset, 
			((LPTSTR)pInfoDevNames) + pInfoDevNames->wDeviceOffset, 
			((LPTSTR)pInfoDevNames) + pInfoDevNames->wOutputOffset, 
			pInfoDevMode);

		nCopies = pInfoDevMode->dmCopies;
		bCollate = (pInfoDevMode->dmCollate == DMCOLLATE_TRUE) ? TRUE : FALSE;

		GlobalUnlock(pInfo->m_pPD->m_pd.hDevNames);
		GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);
	}

	//Get the number of copies and collation out of DEVMODE, even though the MSDN _explicitly_ states that if we disable USEDEVMODECOPIES,
	//we will be able to get those values out of the PRINTDLG struct directly, but that is a flat out lie!!! @%#&!!

	//Set up a CRPEPrintOptions structure to get the number of copies, etc.
	CRPEPrintOptions Options;
	
	//Init struct size
	Options.m_StructSize = sizeof(CRPEPrintOptions);

	// Set collation
	if (bCollate) {
		Options.m_collation = PEP_COLLATED;
	} else {
		Options.m_collation = PEP_UNCOLLATED;
	}

	//We won't be printing to a file
	Options.m_outputFileName[0] = '\0';

	if(pInfo->m_pPD->m_pd.Flags & PD_PAGENUMS){ //Pages is selected, set the range
		 Options.m_startPageN = pInfo->m_pPD->m_pd.nFromPage;
		 Options.m_stopPageN = pInfo->m_pPD->m_pd.nToPage;
	}
	else{ //All pages is selected, use the maximum range
		Options.m_startPageN = pInfo->m_pPD->m_pd.nMinPage;
		// (c.haag 2013-05-29) - PLID 56903 - This should be m_stopPageN, not m_startPageN.
		Options.m_stopPageN = pInfo->m_pPD->m_pd.nMaxPage;
	}

	//Actually set the options using the structure we just made
	SetPrintOptions(&Options);

	return nCopies;
}


void CNxReportJob::AuditPrinting()
{
	// (a.walling 2009-06-01 17:02) - PLID 34240

	CReportInfo report(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(m_nReportInfoID)]);
	
	CString strAuditString = report.strPrintName;
	if (strAuditString.IsEmpty()) {
		strAuditString = report.strReportName;
	}

	long Item = aeiReportPrinted;

	// (a.walling 2009-06-01 17:16) - PLID 34240 - If we want to audit a specific report or group of reports as their own item,
	// then do so here. (leaving this in raises C4065 warning)
	/*
	switch (m_nReportInfoID) {
		default:
			Item = aeiReportPrinted;
	}
	*/

	long nAuditID = BeginNewAuditEvent();
	AuditEvent(-1, "", nAuditID, Item, m_nReportInfoID, "", strAuditString, aepLow, aetOpened);
}
