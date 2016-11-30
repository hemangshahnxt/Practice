#include "stdafx.h"
#include "NxStandard.h"
#include "GlobalReportUtils.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "Practice.h"
#include "MainFrm.h"
#include "NxReportJob.h"
//#include "ReportDocView.h"
#include "peplus.h"
#include "Reports.h"
#include "ChildFrm.h"
#include "ReportInfo.h"
#include "FilterEditDlg.h"
#include "Filter.h"
#include "Groups.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "SalesReceiptConfigDlg.h"
#include "GlobalFinancialUtils.h" // (j.dinatale 2011-08-09 09:07) - PLID 44938

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace ADODB;
#include "crpe.h"

#include "NxRPEMDIChildWnd.h"

//PLID 14613: new global map that tells which reports this user can access
typedef CMap <long, long, long, long> CMapLongToLong;
CMapLongToLong g_mapAvailReports;
BOOL g_bIsAvailReportsLoaded  = FALSE;


static CString l_strReportsPath;
const CString &GetReportsPath()
{
	if (l_strReportsPath.IsEmpty()) {
		l_strReportsPath = GetSharedPath() ^ "Reports";
	}

	return l_strReportsPath;
}

static CString l_strCustomReportsPath;
const CString &GetCustomReportsPath()
{

	if (l_strCustomReportsPath.IsEmpty()) {
		l_strCustomReportsPath = GetSharedPath() ^ "CustomReports";
	}
	EnsureCustomReportsPath();

	return l_strCustomReportsPath;

}

const void EnsureCustomReportsPath()
{
	if (DoesExist(l_strCustomReportsPath)) {
		return;
	}
	else {
		CreatePath(l_strCustomReportsPath);
	}
}

void AddPartToClause(OUT CString &strClause, IN const CString &strPart, IN const CString &strAddWithOp /*= "AND"*/)
{
	if (strClause.IsEmpty()) {
		strClause = strPart;
	} else if (!strPart.IsEmpty()) {
		strClause += " " + strAddWithOp + " (" + strPart + ")";
	}
}

// (j.jones 2011-05-17 13:25) - PLID 42341 - removed commented-out code

// (a.walling 2013-08-30 09:01) - PLID 57998 - Removed GetParameter which should have been internal to CReportInfo anyway

// Print the report being previewed (that is the active mdi window under MainFrame), pass the CRPEJob pointer of the corresponding job
void PrintActiveReportPreview(CNxReportJob *pMustBeJob)
{
	// Get the main frame window
	CMainFrame *pmf = GetMainFrame();
	ASSERT(pmf);
	if (pmf) {
		// Got the main frame, now ask the mainframe what window is active
		CFrameWnd *pFrame = pmf->GetActiveFrame();
		ASSERT(pFrame);
		if (pFrame) {
			// Got the active window, which must be the report window, since this callback function was called
			ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CNxRPEMDIChildWnd)));
			if (pFrame->IsKindOf(RUNTIME_CLASS(CNxRPEMDIChildWnd))) {
				// Indeed, it is the report window so cast it as such
				CNxRPEMDIChildWnd *pNxRpeMdiChild = (CNxRPEMDIChildWnd *)pFrame;
				// Confirm that the report window is THE report window (the window that we created back when the report was run in the first place)
				ASSERT(pNxRpeMdiChild->m_pJob == pMustBeJob);
				if (pNxRpeMdiChild->m_pJob == pMustBeJob) {
					// Finally we know everything's ok with our pointer and our report window, so we can call the print handler
					pNxRpeMdiChild->OnFilePrint();
				} else {
					// Failure: we got a report window, it's not THE report window (the one displaying the report the user wants to print)
					AfxThrowNxException("The report you chose to print is not the current report being viewed");
				}
			} else {
				// Failure: The active frame doesn't appear to be a report preview window, 
				// at least not one that was created using the CRPEJob::Start function
				AfxThrowNxException("The active MDI frame window (type %s) was not created correctly as a CNxRPEMDIChildWnd", pFrame->GetRuntimeClass()->m_lpszClassName);
			}
		} else {
			// Failure: There is no active window, that seems unlikely, is there something wrong with our logic?
			AfxThrowNxException("Could not find the child window for this report");
		}
	} else {
		// Failure: Couldn't get the mainframe, now that's just kooky talk.  That means the program's not really open....
		AfxThrowNxException("Could not find main window");
	}
}

// event handling for crystal reports
BOOL CALLBACK ReportEventHandle (short eventID, void *param, void *userData) 
{
	///////////////////////////////////////////////////////////////////////////////
	// Because this function is a callback, called from a dll, we need to tell MFC 
	// that we're back in the app, otherwise it will do a number of things as if 
	// WE were the dll.  One thing it uses the module state for is the window map, 
	// which means that unless we do this (tell MFC we're back in the app), any 
	// calls to FromHandle will return a pointer to a CTempWnd instead of the real 
	// pointer to the real object.  Because of that (since GetActiveFrame() uses 
	// FromHandle) our IsKindOf() call wouldn't work as we expect.
	AFX_MANAGE_STATE(AfxGetAppModuleState());
	///////////////////////////////////////////////////////////////////////////////

	CNxReportJob *RepJob = (CNxReportJob *)userData;

	switch (eventID) {
	case PEP_PRINT_BUTTON_CLICKED_EVENT:
		// Safely print the report being previewed by the active frame (exceptions thrown if any assumptions fail will be caught by crystal)
		PrintActiveReportPreview(RepJob);
		// We've handled it so tell crystal not to
		return FALSE;
		break;
	case PEP_EXPORT_BUTTON_CLICKED_EVENT:
		// record history
		// The above comment was here, but no code.  I'm not sure if we want to handle this 
		// explicitly or not.  For the time being I left as is, and let crystal do the export
		return TRUE;
		break;
	case PEP_CLOSE_PRINT_WINDOW_EVENT: case PEP_ACTIVATE_PRINT_WINDOW_EVENT: case PEP_DEACTIVATE_PRINT_WINDOW_EVENT:
	case PEP_ZOOM_LEVEL_CHANGING_EVENT: case PEP_FIRST_PAGE_BUTTON_CLICKED_EVENT: case PEP_PREVIOUS_PAGE_BUTTON_CLICKED_EVENT:
	case PEP_NEXT_PAGE_BUTTON_CLICKED_EVENT: case PEP_LAST_PAGE_BUTTON_CLICKED_EVENT: case PEP_CANCEL_BUTTON_CLICKED_EVENT:
	case PEP_CLOSE_BUTTON_CLICKED_EVENT: case PEP_SEARCH_BUTTON_CLICKED_EVENT: case PEP_GROUP_TREE_BUTTON_CLICKED_EVENT:
	case PEP_PRINT_SETUP_BUTTON_CLICKED_EVENT: case PEP_REFRESH_BUTTON_CLICKED_EVENT: case PEP_SHOW_GROUP_EVENT:
	case PEP_DRILL_ON_GROUP_EVENT: case PEP_DRILL_ON_DETAIL_EVENT: case PEP_READING_RECORDS_EVENT: case PEP_START_EVENT:
	case PEP_STOP_EVENT: case PEP_MAPPING_FIELD_EVENT: case PEP_RIGHT_CLICK_EVENT:
		// A bunch of events we don't handle right now, just let crystal handle them
		return TRUE;
		break;
	case 25: // single click on report
		RepJob->NextWindowMagnification();
		// In addition to zooming, let crystal do whatever it wants too
		return TRUE;
		break;
	case 26: //click of the scroll wheel
		// Just let crystal handle it
		return TRUE;
		break;
	default:
		// Unknown event ID, we'd better let crystal take care of it
		{
			CString str;
			str.Format ("Unmapped Crystal Event: %i", eventID);
			MessageBox (GetMainFrame()->GetSafeHwnd(), str, "Practice", MB_OK);
		}
		return TRUE;
		break;
	}

	return FALSE;
}

void RunStatement(short Style, bool bPreview, CWnd *parentWnd)
{
	// 3/10/2000
	// Style: Detailed = 1, Summary = 2
	// bPreview: Print Preview = true, Print directly = false
	// parentWnd: parent window (this from wherever the function is called)

	// get the parameter list together
	//CPtrArray *params = new CPtrArray();
	//CRParameterInfo *paramInfo;
	//CPtrArray paramList;
	//CRParameterInfo *tmpPInfo;
//	CString tmp;

	
/*  paramInfo = new CRParameterInfo();
	paramInfo->m_Name = "SttmntUseComma";
	paramInfo->m_Data = GetRemotePropertyInt("SttmntUseComma", -1, 0, "<None>");
	params->Add((void *)paramInfo);

	paramInfo = new CRParameterInfo();
	paramInfo->m_Name = "SttmntUseDocName";
	paramInfo->m_Data = GetRemotePropertyInt("SttmntUseDocName", 0, 0, "<None>");
	params->Add((void *)paramInfo);

	paramInfo = new CRParameterInfo();
	paramInfo->m_Name = "SttmntUseGuarantor";
	paramInfo->m_Data = GetRemotePropertyInt("SttmntUseGuarantor", -1, 0, "<None>");
	params->Add((void *)paramInfo);
	
///////
	tmpPInfo = new CRParameterInfo;
	tmp.Format("%d", GetRemotePropertyInt("SttmntUseGuarantor"));
	tmpPInfo->m_Data = tmp;
	tmpPInfo->m_Name = (CString)"UseGuar";
	paramList.Add((void *)tmpPInfo);

	tmpPInfo = new CRParameterInfo;
	tmp.Format("%s", GetRemotePropertyMemo("SttmntPerName",""));
	tmpPInfo->m_Data = tmp;
	tmpPInfo->m_Name = (CString)"ContactPerson";
	paramList.Add((void *)tmpPInfo);

	tmpPInfo = new CRParameterInfo;
	tmp.Format("%d", GetRemotePropertyInt("SttmntUseDocName"));
	tmpPInfo->m_Data = tmp;
	tmpPInfo->m_Name = (CString)"DocName";
	paramList.Add((void *)tmpPInfo);
	
	tmpPInfo = new CRParameterInfo;
	tmp.Format("%s", GetRemotePropertyMemo( "SttmntCallMe"));
	tmpPInfo->m_Data = tmp;
	tmpPInfo->m_Name = (CString)"StmmtTitle";
	paramList.Add((void *)tmpPInfo); 

	tmpPInfo = new CRParameterInfo;
	tmp.Format("%d", GetRemotePropertyInt("SttmntUseComma"));
	tmpPInfo->m_Data = tmp;
	tmpPInfo->m_Name = (CString)"UseComma";
	paramList.Add((void *)tmpPInfo);

	tmpPInfo = new CRParameterInfo;
	tmpPInfo->m_Data = GetRemotePropertyMemo("SttmntText",(CString)"-");
	tmpPInfo->m_Name = (CString)"CustomText";
	paramList.Add((void *)tmpPInfo);
/////
	
	ViewReport("Patient Statement", "_Statement", "", &paramList, bPreview, parentWnd);
*/	
}

IDispatch FAR * CreateCdoObject();


// Globals.

short GetPeErrorCode(short nJobHandle)
{
	return PEGetErrorCode(nJobHandle);
}

CString GetPeErrorText(short nJobHandle)
{
	HANDLE hText;
	short nLen;
	if (PEGetErrorText(nJobHandle, &hText, &nLen)) {
		char strErr[512];
		if (PEGetHandleString(hText, strErr, 512)) {
			return strErr;
		} else {
			return "Could not Obtain Error String";
		}
	} else {
		return "No Error";
	}
}

BOOL IsStatement(long nID) 
{
	if (nID == 234 || nID == 338 || nID == 169 || nID == 337 || nID == 353 || nID == 354 || nID == 355 
		|| nID == 356 || nID == 434 || nID == 435 || nID == 436 || nID == 437 || nID == 483 || nID == 484 || nID == 485 || nID == 486) {

		return true;
	}
	else {
		return false;
	}
}


void CALLBACK EndReportPrinting(void *NeedsToBeRepJob) {

	CNxReportJob *RepJob = (CNxReportJob *)NeedsToBeRepJob;

	//check to see that the reportinfo ID has been initialized

	if (RepJob->m_nReportInfoID != -255) {

		// (j.gruber 2011-10-11 15:50) - PLID 45937 - added member to nxrepJob
		if (RepJob->m_bWriteToHistoryStatus) {
			CReportInfo::WriteToHistory(RepJob->m_nReportInfoID, 0,0, RepJob->m_rsReportInfo, CReportInfo::NXR_PRINT);
		}
	}
	else {

		//it hasn't been initialized!! we can't write anything to history
		MsgBox("Error Initialing Report Object::Print History Failed");
	}

	
}




bool FindReportIDInArray(const CDWordArray &aryIDs, const long nFind)
{
	// Loop through the array
	int nCount = aryIDs.GetSize();
	for (int i = 0; i < nCount; i++) {
		// Compare the current element to the given string
		if (aryIDs.GetAt(i) == (DWORD)nFind) {
			// Found it
			return true;
		}

	}

	// We looped all the way through and didn't find it
	return false;
}

// (j.gruber 2007-02-21 11:46) - PLID 24048 - added NexEMR license check
bool IsReportNexEMROnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 547: //Audit Trail - EMR
		case 576: //EMR Summary (PP)
		case 577: //EMNs by Procedure
		case 581: //EMR Search (PP)
		// (j.gruber 2007-02-26 16:47) - PLID 24609 - EMR summary screen report
		case 584: // EMR Summary Screen Report
		// (j.jones 2008-07-02 16:16) - PLID 18534
		case 631: // EMNs With Charges
		//(e.lally 2008-08-14) PLID 30732
		case 651:	//Problem List by Patient
		case 652:	//Problem List (PP)
		case 689:  // (j.gruber 2010-01-21 14:16) - PLID 34166 //EMN Service/ICD9 Codes By EMN
		case 663: //graph EMR Data by Date
		case 693: // (j.gruber 2010-02-18 13:30) - PLID 37378 graph EMR Data by Age
			return true;
		break;
		default:
			return false;
		break;
	}
	
}

// (j.gruber 2007-02-21 11:46) - PLID 24048 - added NexEMR license check
bool IsReportNexEMRORCustomRecords(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 254: //EMR Data By Provider
		case 255: //EMR Totals By Age/Gender
		case 256: //EMR Data By Patient
		case 332: //EMR Data By Item
		case 262: //ASPS Survey		// (j.jones 2008-05-27 15:20) - PLID 27933 - licensed off the ASPS Survey to EMR or Custom Records
			return true;
		break;
		default:
			return false;
		break;
	}
	
}

// (j.gruber 2007-02-21 11:46) - PLID 24048 - added Custom Records license check
bool IsReportCustomRecordsOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 257: //EMR / Op Report (PP)
			return true;
		break;
		default:
			return false;
		break;
	}
	
}



// (j.jones 2006-11-13 16:10) - PLID 23530 - added Ebilling license check
bool IsReportEbillingOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		// (j.jones 2007-06-29 08:57) - PLID 23951 - the E-Remittance report
		// was formerly in this block, but now it has its own license

		// (j.jones 2009-10-07 11:47) - PLID 35776 - added claim submission report
		case 684:	//Claim Submission Percentages By Responsibility
			return true;
			break;
		default:
			return false;
		break;
	}

	return false;
}

// (j.jones 2007-06-29 08:58) - PLID 23951 - added E-Remittance licensing
bool IsReportERemittanceOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 537:		//Electronic Remittance
			return true;
		break;
		default:
			return false;
		break;
	}
	
}

// (a.walling 2008-02-14 13:09) - PLID 28388 - added Advanced Inventory licensing
bool IsReportAdvInventoryOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {
		// (a.walling 2008-02-15 11:30) - PLID 28946 - Protected reports
	//(e.lally 2010-09-23) PLID 40563 - The Supplier Returns is now available to everyone with inventory.
	//case 616: // Supplier Returns
	// (c.haag 2008-03-07 14:00) - PLID 29170 - This report has been deprecated
	//case 617: // Allergan Product Transfer Summary Sheet
	case 618: // Inventory Overview
	case 619: // Allocation List
	case 620: // Consignment List
	case 621: // Consignment History By Date
	// (j.jones 2014-06-24 11:10) - PLID 40655 - protected the turn rate report
	case 655: // Consignment Turn Rate by Month
	case 623: // Physical Inventory - Serialized - Tally Sheet
	case 628: // Appointments Without Allocations - TES 6/18/2008 - PLID 30395
		return true;
	break;
	default:
		return false;
	break;
	}

	return false;	
}

// (r.gonet 2015-11-12 02:46) - PLID 67466 - Some reports will only be available when the global capitation preference is on.
bool IsReportCapitationOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {
	case 762:	// Capitation Distribution 
	case 763:	// Capitation Batch Payments
		return true;
	default:
		return false;
	}
}

// (a.walling 2007-08-02 11:18) - PLID 26899 - check for CC Processing license
// (j.jones 2015-09-30 10:45) - PLID 67178 - renamed to clarify which reports are available
// under any CC processing license
bool IsReportAnyCCOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 604:		//Credit Card Customer Copy
		case 605:		//Credit Card Merchant Copy
		case 606:		//Credit Card Batch Processing
		case 669:		// (d.thompson 2009-07-07) - PLID 34798 - Credit Card Processing Reconciliation
			return true;
		break;
		default:
			return false;
		break;
	}
}

// (j.jones 2015-09-30 10:45) - PLID 67178 - defines reports that should be hidden if
// ICCP is both licensed and enabled
bool IsReportNonICCPCCOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 669:		//Credit Card Processing Reconciliation
			return true;
			break;
		default:
			return false;
			break;
	}
}

// (j.jones 2015-09-30 10:58) - PLID 67179 - Defines reports that require the ICCP license.
// ICCP does not have to be enabled for these reports to be available.
bool IsReportICCPCCOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

	case 761:		//Integrated Credit Card Payments/Refunds
		return true;
		break;
	default:
		return false;
		break;
	}
}

bool IsReportRefractiveOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {

		case 327:		//Cumulative Visual Acuity
		case 328:		//Refractive Outcomes
		case 571:		//Patient Outcomes // (m.hancock 2006-11-27 09:22) - PLID 15451 - Added Patient Outcomes report
			return true;
		break;
		default:
			return false;
		break;
	}
	
}

bool IsReportNexTechOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
	
	case 321: //Contact Report
	case 596:	//DRT 6/7/2007 - PLID 25892 - Sales Forecast / Pipeline
	case 597:	//DRT 6/7/2007 - PLID 25892 - High Sales Opportunities
	case 598:	//DRT 6/7/2007 - PLID 25892 - Sales Revenue Charts
	case 599:	//DRT 6/25/2007 - PLID 25892 - Sales Quarterly Targets
	case 600:	//DRT 6/26/2007 - PLID 25892 - Sales Yearly Summary
	case 615:  // (j.gruber 2007-11-09 08:54) - PLID 28047 - Clients by EMR Specialist
	case 625: // (j.gruber 2008-04-02 17:31) - PLID 29440 - EMR Clients by Name
	case 626: // (j.gruber 2008-05-28 10:56) - PLID 30181 - Clients By EMR Status
	case 696: // (f.dinatale 2010-07-19) - PLID 39541 - HL7 Integration Information By Client -->(j.deskurakis 2013-01-24) - PLID 54973 - revised to match new tables and available data
	case 697: // (f.dinatale 2010-07-19) - PLID 39541 - HL7 Integration Information By Company -->(j.deskurakis 2013-01-24) - PLID 54973 - revised to match new tables and available data
	case 701: // (f.dinatale 2011-01-13) - PLID 41275 - NexReminder Client Usage
	case 702: // (j.armen 2011-04-11) - PLID 42146 - Sales Report
	case 708:	//(b.savon 2011-06-27) - PLID 44179 - Support Escalation Requests
	case 712: //(b.savon 2011-07-19) - PLID 44523 - Support Rating Developers on Escalation Requests
	case 724: // (j.luckokski 2012-03-28) - PLID 48809 - Allow support to see clients with remaininig training hours.
	case 727: // (j.fouts 2012-05-16 16:02) - PLID 50211 - Internal Inv Mgmt Reports
	case 728: 
	case 741: // (a.levy 2012-09-26 12:12) - PLID 52490 -Sales Productivity Report to be Internal only
	case 742: // (j.deskurakis 2012-09-28) - PLID 29105 - Paid Time Off Report - Internal
		return true;
	break;
	default:
		return false;
	break;
	}
}

bool IsReportSpaOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
		case 489: //Payment Tips by Provider
		case 491: 	//Provider Commissions (Payments)
		case 544: 	//Provider Commissions (Charges)
		case 497: 	//Gift Certificate
		case 499:	//Outstanding Gift Certificate Balances
		case 501:	//Closed Case Drawers
		case 503:	//Open Cash Drawers
		case 545:	//Gift Certificates Redeemed
		case 546:	//Gift Certificates Sold
		case 561:	//Returned Product Basic Commission Summary
		// (j.gruber 2007-06-01 14:59) - PLID 25975 - added applicable nexspa reports here
		case 589:   //Sales
		case 590:   //Coupons
		case 591:   //Suggested Sales
		case 592:   //Advanced Commissions
		case 593:   //Commission Rules
		// (a.walling 2007-09-25 17:08) - PLID 25976 - include patient reward reports as well
		case 594:	//Patient Reward History
		case 595:	//Patient Reward Points by Location and Provider
		// (j.jones 2011-04-22 10:42) - PLID 42341 - added the Reward Point Redemption List
		case 705:	//Reward Point Redemption List
			return true;
		break;

		default:
			return false;
		break;
	}



}


bool IsReportNexWebOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
		case 548:  //nexweb import info
		case 685: // (j.gruber 2009-10-29 15:51) - PLID 35772 - nexweb login percentage
		case 700: // (j.gruber 2010-10-29 12:02) - PLID 35817 - Patient Messaging
			return true;
		break;

		default:
			return false;
		break;
	}



}


bool IsReportRetentionOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
		case 535:  //Retained Patients
		case 536:  //UnRetained Patients

			return true;
		break;

		default:
			return false;
		break;
	}



}



bool IsReportMarketingOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
	case 24: //Referrals (Primary)
	case 190://Effectiveness
	case 258://New Patient Procedures
	case 272://Patients by Referring Patient
	case 273://Consults Without Surgeries
	case 330://Surgery List w/Referrals
	case 334://Total Patient Revenue
	case 349://Patients by Original Referral Source
	case 350://Patients by Original Referring Physician
	case 351://Referred Patients by Referral Source
	case 352://Referred Patients by Referring Physician
	case 357://Projected Surgery Income
	case 358://Actual Surgery Income
	case 359://Referring Patients
	case 378://Referrals (Primary and Non-Primary)
	case 402://Marketing Costs
	case 420://Referred Patients
	case 480://Conversion Rate by Referral Source
	case 538://Consult to Surgery Counts by Resource
		 //marketing print preview reports
	case 502:
	//case 504: //(e.lally 2009-09-24) PLID 35053 - Depreciated and recreated as 680
	case 505:
	case 506:
	case 507:
	case 508:
	case 509:
	case 510:
	//case 511: //(e.lally 2009-08-27) PLID 35330 - depreciated and recreated as 673
	case 512:
	case 513:
	case 514:
	case 515:
	case 516:
	case 517:
	case 518:
	//case 519: //(e.lally 2009-08-28) PLID 35331 - depreciated and recreated as 676
	case 520:
	case 521:
	case 522:
	case 523:
	case 524:
	case 525:
	//case 526://(e.lally 2009-09-08) PLID 35332 - depreciated and recreated as 677
	case 527:
	case 528:
	case 529:
	case 530:
	case 531:
	case 532:
	case 533:
	case 534:
	case 540: //conversion rate by procedure

	// (j.gruber 2007-11-01 12:06) - PLID 27814 - Marketing Review
	case 614:
	case 673://(e.lally 2009-08-27) PLID 35330
	case 676://(e.lally 2009-08-28) PLID 35331
	case 677://(e.lally 2009-09-08) PLID 35332
	case 680://(e.lally 2009-09-24) PLID 35053
	case 681://(e.lally 2009-09-25) PLID 35654
	case 687:	// (d.thompson 2009-12-08) - PLID 32190
	case 738: // (r.gonet 06/27/2012) - PLID 47647 - Weekly Advertising Analysis By Top Level Referral
		return true;
		break;
		
	default:
		return false;
		break;
	}
}

bool IsReportNexTrakOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
	//Admin Tab
	case 392: //ladders

	//Marketing Tab
	case 295: //active steps by age
	case 477: //Completed steps by age
	case 258: //New Patient Procedures
	case 293: //Patient Tracking by patient coordinator
	case 291: //patients by tracking step
	case 396: //PIC Report
	case 300: //procedures by status
	case 292: //procedures by tracking step
	case 283: //tracking steps by procedure
	
	// (j.gruber 2007-11-01 12:07) - PLID 24698
	case 610:  //tracking Conversion

	//Patients Tab
	case 512: //Patients By Procedure
	case 282: //Patients By Procedure
	case 301: //Patients with Active Procedures
	case 302: //Patients with Finished Procedures
	case 367: //Patients with Procedures On Hold

	// (j.jones 2009-09-14 11:56) - PLID 13812 - added the surgery income reports
	case 357: //Projected Surgery Income
	case 358: //Actual Surgery Income
	case 674:	// (d.thompson 2009-12-10) - PLID 35068
	case 675:	// (d.thompson 2009-12-10) - PLID 35068

		return true;
		break;
		
	default:
		return false;
		break;
	}
}

// (c.haag 2010-06-02 15:57) - PLID 37998 - Appointment Reminders (CellTrust)
bool IsReportCellTrustOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
		case 694: // (c.haag 2010-06-02 15:57) - PLID 37998 - Appointment Reminders (CellTrust)
			return true;
		default:
			return false;
	}
}
// (b.spivey, August 12, 2014) - PLID 62970 - Check license for lockbox reports. 
bool IsReportLockBoxOnly(IN const CReportInfo *pReport)
{
	switch(pReport->nID) {
		case 757:
			return true;
		default:
			return false; 
	}
}

// (j.jones 2008-07-01 12:35) - PLID 30501 - this function will check
// non-report-specific permissions needed for the given report ID, and
// it should NOT be called outside of CheckCurrentUserReportAccess without
// also calling CheckCurrentUserReportAccess
BOOL CheckIndivReportAccess(long nReportID)
{
	// (j.jones 2009-09-17 09:28) - PLID 16703 - split this report into two
	if(nReportID == 539			//Case History Cost / Profit Analysis By Provider
		|| nReportID == 678) {	//Case History Cost / Profit Analysis By Procedure

		// (j.jones 2008-07-01 12:35) - PLID 30581 - they need the bioContactsDefaultCost permission
		// to view this report
		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);
		return bCanViewPersonCosts;
	}

	return TRUE;
}

bool CheckCurrentUserReportAccess(long nReportID, BOOL bSilent, BOOL bForcePermissionCheck /*= FALSE*/) {

	// (j.jones 2008-07-01 12:33) - PLID 30581 - we need to be able to check certain reports
	// for other permissions, independently of the per-report permission
	if(!CheckIndivReportAccess(nReportID)) {
		if (bSilent) {
			return FALSE;
		}
		else {
			MsgBox("You do not have permissions to run this report.");
			return FALSE;
		}
	}

	if (g_bIsAvailReportsLoaded && !bForcePermissionCheck) {
		
		//just see if its in the map
		if (g_mapAvailReports.Lookup(nReportID, nReportID)) {
			return TRUE;
		}
		else {
			if (bSilent) {
				return FALSE;
			}
			else {
				MsgBox("You do not have permissions to run this report.");
				return FALSE;
			}
		}
	}

	if (!bForcePermissionCheck) {

		//let's try to load the report so we can stop this nonsense
		LoadAvailableReportsMap();

		//check again
		if (g_bIsAvailReportsLoaded) {
			if (g_mapAvailReports.Lookup(nReportID, nReportID)) {
				return TRUE;
			}
			else {
				if (bSilent){
					return FALSE;
				}
				else {
					MsgBox("You do not have permissions to run this report.");
					return FALSE;
				}
			}
		}
	}
	
	//do it that hard way, but log this
	//Log("Checking report permissions through data");
	//we are going to do an access first approach rather than a deny first approach
	//first check if they are an adinistrator, because if so they automatically get the access
	if ((GetCurrentUserName() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME)  ||
			//DRT 5/19/2006 - PLID 20658 - We now have the caching of administrator status, so
			//	no more need to run a query here.
			IsCurrentUserAdministrator()) {
		//they are an admin, let them have the report
		return TRUE;
	}

	//get a recordset of every group that this report is in
	_RecordsetPtr rsReportGroups = CreateRecordset("SELECT GroupID "
		" FROM ReportGroupDetailsT LEFT JOIN ReportGroupsT ON ReportGroupDetailsT.GroupID = ReportGroupsT.ID "
		" WHERE (ReportID = %li) "
		" GROUP BY GroupID", nReportID, nReportID);

	if (rsReportGroups->eof) {

		//this report isn't in a group, and if its new, they don't want it, so deny
		if (bSilent) {
			return FALSE;
		}
		else {
			MsgBox("You do not have permissions to run this report.");
			return FALSE;
		}
	}
	
	//PLID 20814, I'm not sure why this note wasn't changed, we are doing an allow first method, not deny first
	//loop through the groups and check if this user has permission,
	//if we hit one deny, then we return false, otherwise return true
	while (! rsReportGroups->eof) {

		//get the group ID
		long nGroupID = AdoFldLong(rsReportGroups, "GroupID");

		//PLID 20814 - always be silent here
		if (CheckCurrentUserPermissions(bioReportGroup, sptView, TRUE,
			nGroupID, true, true)) {

			//they have permissions, succeed
			return TRUE;
		}

		rsReportGroups->MoveNext();
	}
	rsReportGroups->Close();

	//if we got here, then they don't have permissions
	//PLID 20814, we are going to check bSilent and tell them here so they know its coming from reports
	if (!bSilent) {
		MsgBox("You do not have permissions to run this report.");
	}
	//we don't have to check bSilent here because they would've already gotten the messagebox from CheckCurrentUserPermissions
	return FALSE;
}


// Is the user allowed access this report?
// Checks to see if the report is allowed under the license, and if the user has permissions to see it
// If bSilent, then no prompts or messages will be given to the user
// Returns true if allowed, false if not
bool CheckLicenseForReport(IN const CReportInfo *pReport, bool bSilent)
{
	if (IsReportRefractiveOnly(pReport) && !IsRefractive()) {
		// Can't see this report because it's a refractive-only report and we don't have a refractive license
		if (!bSilent) {
			AfxMessageBox("This report is only available with a refractive license.  Please contact NexTech to purchase this license.");
		}
		return false;
	}

	if (IsReportNexTechOnly(pReport) && !IsNexTechInternal()) {
		// Can't see this report because it's a NexTech-only report and we don't have a NexTech license
		if (!bSilent) {
			AfxMessageBox("This report is only available with a NexTech Internal license.");
		}
		return false;
	}

	if (IsReportSpaOnly(pReport)) {
		if(!IsSpa(!bSilent)) {
			// Can't see this report because it's a NexTech-only report and we don't have a NexTech license
			if (!bSilent) {
				AfxMessageBox("This report is only available with a NexSpa license.");
			}
			return false;
		}
	}

	// (j.dinatale 2012-02-01 15:24) - PLID 45511 - this is now a non beta feature
	// (j.dinatale 2011-08-09 08:56) - PLID 44938 - need to be able to hide the report if financial corrections is not enabled
	/*if(!IsLineItemCorrectionsEnabled_Beta() && pReport->nID == 713){
		return false;
	}*/
	
	if(IsReportMarketingOnly(pReport)) {
		if(!g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent)) {
			//Can't see this report because it's Marketing-only, and we don't have a marketing license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Marketing license.");
			}
			return false;
		}
	}

	if(IsReportNexTrakOnly(pReport)) {
		if(!g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
			//Can't see this report because it's NexTrak-only, and we don't have a NexTrak license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a NexTrak license.");
			}
			return false;
		}
	}

	if(IsReportNexWebOnly(pReport)) {
		// (j.gruber 2009-12-01 09:03) - PLID 36455 - change licensing
		if(!g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcNexWebLeads, CLicense::cflrSilent)) {
			//Can't see this report because it's NexWeb-only, and we don't have a NexWeblicense.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a NexWeb license.");
			}
			return false;
		}
	}

	if(IsReportRetentionOnly(pReport)) {
		if(!g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent)) {
			//Can't see this report because it's Retention-only, and we don't have a Retention license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Retention license.");
			}
			return false;
		}
	}

	// (j.jones 2006-11-13 16:12) - PLID 23530 - added check for ebilling license
	if(IsReportEbillingOnly(pReport)) {
		if(!g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent)) {
			//Can't see this report because it's Ebilling-only, and we don't have a Ebilling license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Electronic Billing license.");
			}
			return false;
		}
	}

	// (j.jones 2007-06-29 08:58) - PLID 23951 - added E-Remittance licensing
	if(IsReportERemittanceOnly(pReport)) {
		if(!g_pLicense->CheckForLicense(CLicense::lcERemittance, CLicense::cflrSilent)) {
			//Can't see this report because it's E-Remittance-only, and we don't have a E-Remittance license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Electronic Remittance license.");
			}
			return false;
		}
	}

	// (a.walling 2007-08-02 11:21) - PLID 26899 - check for CC Processing licensing
	// (d.thompson 2010-09-02) - PLID 40371 - Applies with any processing type
	// (j.jones 2015-09-30 10:45) - PLID 67178 - renamed to clarify which reports are available
	// under any CC processing license
	// (z.manning 2016-02-04 10:53) - PLID 68097 - This needs to check the ICCP license!
	if (IsReportAnyCCOnly(pReport)) {
		if(!g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcICCP, CLicense::cflrSilent)) {
			//Can't see this report because it's CCProcessing-only, and we don't have a CCProcessing license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Credit Card Processing license.");
			}
			return false;
		}
	}

	// (j.jones 2015-09-30 10:45) - PLID 67178 - defines reports that should be hidden if
	// ICCP is both licensed and enabled
	if (IsReportNonICCPCCOnly(pReport)) {
		//returns true if they have the ICCP license and it is enabled
		if (IsICCPEnabled()) {
			if (!bSilent) {
				AfxMessageBox("This report is not available when Integrated Credit Card Processing is enabled.");
			}
			return false;
		}
	}

	// (j.jones 2015-09-30 10:58) - PLID 67179 - Defines reports that require the ICCP license.
	// ICCP does not have to be enabled for these reports to be available.
	if (IsReportICCPCCOnly(pReport)) {
		//returns true if they have the ICCP license
		if (g_pLicense == NULL || !g_pLicense->CheckForLicense(CLicense::lcICCP, CLicense::cflrSilent)) {
			if (!bSilent) {
				AfxMessageBox("This report is only available with an Integrated Credit Card Processing license.");
			}
			return false;
		}
	}

	// (a.walling 2008-02-15 10:20) - PLID 28388 - Check for AdvInventory
	if(IsReportAdvInventoryOnly(pReport)) {
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			//Can't see this report because it's AdvInventory-only, and we don't have an AdvInventory license.
			// (d.thompson 2009-01-26) - PLID 32851 - Renamed to 'Consignment and Allocation' from 'Advanced Inventory'
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Consignment and Allocation license.");
			}
			return false;
		}
	}

	// (j.gruber 2007-02-21 11:47) - PLID 24048 - added custom records check
	if(IsReportNexEMRORCustomRecords(pReport)) {
		// (a.walling 2007-11-28 13:02) - PLID 28044 - Check for expired EMR license
		// (d.thompson 2009-01-06 16:49) - PLID 32147 - Check for EMR standard (same as cust recs)
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 0) 
		{
			//Can't see this report because it's CustomRecords or NexEMR, and we don't have that license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a CustomRecords or NexEMR license.");
			}
			return false;
		}
	}

	// (j.gruber 2007-02-21 11:47) - PLID 24048 - added NexEMR check
	if(IsReportCustomRecordsOnly(pReport)) {
		// (d.thompson 2009-01-06 16:50) - PLID 32147 - Check for EMR standard (same as cust recs)
		if(!g_pLicense->CheckForLicense(CLicense::lcCustomRecords, CLicense::cflrSilent)
			&& !g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent))
		{
			//Can't see this report because it's CustomReports-only, 
			if(!bSilent) {
				AfxMessageBox("This report is only available with a Custom Records license.");
			}
			return false;
		}
	}

	// (j.gruber 2007-02-21 11:47) - PLID 24048 - added NexEMR check
	if(IsReportNexEMROnly(pReport)) {
		// (a.walling 2007-11-28 13:03) - PLID 28044 - Check for expired EMR license
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(!(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2)) {
			//Can't see this report because it's NexEMR-only, and we don't have a NexEMRlicense.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a NexEMR license.");
			}
			return false;
		}
	}

	// (c.haag 2010-06-02 15:57) - PLID 37998 - Appointment Reminders (CellTrust)
	if (IsReportCellTrustOnly(pReport)) {
		// (z.manning 2010-11-17 12:05) - PLID 40643 - Renamed to NxReminder
		if(!g_pLicense->CheckForLicenseOrExpired(CLicense::lcNxReminder, CLicense::cflrSilent)) {
			//Can't see this report because it's CellTrust-only, and we don't have a CellTrust license.
			if(!bSilent) {
				AfxMessageBox("This report is only available with a NexReminder license.");
			}
			return false;
		}
	}
	// (s.dhole 2012-06-21 12:35) - PLID 49341  -- Optical shop
	if (IsReportOpticalShopOnly(pReport)) {
		if(!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders , CLicense::cflrSilent)) {
			if(!bSilent) {
				AfxMessageBox("This report is only available with an Optical Orders license.");
			}
			return false;
		}
	}	
	// (b.spivey, August 12, 2014) - PLID 62970 - Check license for lockbox reports. 
	if (IsReportLockBoxOnly(pReport)) {
		if (!g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
			if (!bSilent) {
				AfxMessageBox("This report is only available with a Lockbox Payments license.");
			}
			return false; 
		}
	}
	

	// (s.tullis 2014-07-10 13:47) - PLID 62560- Batch Vision Payments
	// (s.tullis 2014-07-14 13:34) - PLID 62559 - Batch Medical Payments
	// Need to have a Vision Payment license to see the Vision Posting Reports
	if (IsReportVisionPostingOnly(pReport)){
		if (!g_pLicense->CheckForLicense(CLicense::lcVisionPayments, CLicense::cflrSilent)) {
			if (!bSilent) {
				AfxMessageBox("This report is only available with a Vision Payment license.");
			}
			return false;
			}
	}

	// (r.gonet 2015-11-12 02:54) - PLID 67466 - Not exactly a license check, but capitation related reports should be hidden when capitation is not enabled.
	if (IsReportCapitationOnly(pReport)) {
		if (!GetRemotePropertyInt("BatchPayments_EnableCapitation", 0, 0, "<None>", true)) {
			if (!bSilent) {
				AfxMessageBox("This report is only available when the \"Enable Capitation Payments\" preference is enabled. "
					"You may go to Tools->Preferences->Financial Module->Batch Payments->General->\"Enable Capititation Payments\" to change it.");
			}
			return false;
		}
	}

	// If we made it here, it's just a regular report, so return TRUE
	return TRUE;
	//return CheckCurrentUserPermissions(pReport->GetBuiltInObjectID(), sptView, FALSE, 0, bSilent, bSilent) ? true : false;
}



BOOL GetReportFileName(long nReportID, CString &strFileNameToRun, long &nDefaultCustomReport)    {

	_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = %li", nReportID);
	if (rsDefault->eof) {
		return FALSE;
	}
	else {
		long nCustomReportID = AdoFldLong(rsDefault, "CustomReportID", -1);

		if (nCustomReportID > 0) {
			_RecordsetPtr rsFileName = CreateRecordset("SELECT Filename From CustomReportsT WHERE ID = %li and Number = %li", nReportID, nCustomReportID);

			if (rsFileName->eof) {
				MsgBox("Practice could not find the custom report.  Please contact NexTech for assistance.");
				strFileNameToRun = "";
				//we are returning true because we don't want to run anything and if we return false, it will run the default
				return TRUE;
			}
			else {
				CString strFileName = AdoFldString(rsFileName, "FileName");
				nDefaultCustomReport = nCustomReportID;

				strFileNameToRun = strFileName;
				return TRUE;
			}
		}
		else {
			return FALSE;
		}
	}
	
}

//DRT 10/26/2005 - PLID 18085 - This is a handy function that takes an array of
//	CRParameterInfo* objects, deallocates all the memory used, and empties the array.
//	Anyone who calls the below RunReport() function, and has created their own array
//	of parameters can use this function to quickly and easily clean up after themselves.
void ClearRPIParameterList(CPtrArray *pParamList)
{
	for(int i = 0; i < pParamList->GetSize(); i++) {
		CRParameterInfo* prpi = (CRParameterInfo*)pParamList->GetAt(i);
		if(prpi)
			delete prpi;
	}
	pParamList->RemoveAll();
}

//use this one if you have to set anything in the report info object ie: filters, etc
// (j.camacho 2014-10-21 12:34) - PLID 62716 - added new parameter to specify if you are setting dates, defaults to FALSE
BOOL RunReport(CReportInfo *pReport, CPtrArray *paramList, BOOL bPreview, CWnd *wndParent, CString strTitle /*=""*/, CPrintInfo* pInfo /*=0*/, BOOL bUseDateFilter /*=FALSE*/) {

	//JMM - Check to see if they can access this report
	if (! CheckCurrentUserReportAccess(pReport->nID, FALSE)) {
		return FALSE;
	}

	// (j.camacho 2014-10-21 12:34) - PLID 62716 - we needed this to account for the reports that are specified in the reports module tabs, but can be run elsewhere in Practice as the same ReportInfoID
	if (!bUseDateFilter)
	{
		pReport->nDateRange = -1;
	}

	CString strFileName = "";
	long nDefaultReportID = 0;
	if (GetReportFileName(pReport->nID, strFileName, nDefaultReportID)) {
		pReport->nDefaultCustomReport = nDefaultReportID;
		if (!strFileName.IsEmpty()) {
			if (strTitle.IsEmpty()) {
				return pReport->ViewReport(pReport->strPrintName, strFileName, paramList, bPreview, wndParent, pInfo);
			}
			else {
				return pReport->ViewReport(strTitle, strFileName, paramList, bPreview, wndParent, pInfo);
			}
		}
	}
	else {
		//check the filename
		return pReport->ViewReport(pReport->strPrintName, pReport->strReportFile, paramList, bPreview, wndParent, pInfo);
		
	}
	return FALSE;
}



// (j.camacho 2014-10-21 12:34) - PLID 62716 - added new parameter to specify if you are setting dates, defaults to FALSE
BOOL RunReport(CReportInfo *pReport, BOOL bPreview, CWnd *wndParent, CString strTitle /*= ""*/, CPrintInfo* pInfo /*=0*/, BOOL bUseDateFilter /*=FALSE*/) {

	//JMM - Check to see if they can access this report
	if (! CheckCurrentUserReportAccess(pReport->nID, FALSE)) {
		return FALSE;
	}

	// (j.camacho 2014-10-21 12:34) - PLID 62716 - we needed this to account for the reports that are specified in the reports module tabs, but can be run elsewhere in Practice as the same ReportInfoID
	if (!bUseDateFilter)
	{
		pReport->nDateRange = -1;
	}
	
	CString strFileName = "";
	long nDefaultReportID = 0;
	if (GetReportFileName(pReport->nID, strFileName, nDefaultReportID)) {
		pReport->nDefaultCustomReport = nDefaultReportID;
		if (!strFileName.IsEmpty()) {
			if (strTitle.IsEmpty()) {
				return pReport->ViewReport(pReport->strPrintName, strFileName, bPreview, wndParent, pInfo);
			}
			else {
				return pReport->ViewReport(strTitle, strFileName, bPreview, wndParent, pInfo);
			}
		}
	}
	else {
		//check the filename
		return pReport->ViewReport(pReport->strPrintName, pReport->strReportFile, bPreview, wndParent, pInfo);
		
	}
	return FALSE;
}



BOOL GenerateFilter(IN CReportInfo *pReport, OUT CString *pstrOutFilter /*= NULL*/, OUT long *pnItemCount /*= NULL*/)
{
	ASSERT(pReport);
	if (pReport && pReport->bUseGroupOption && pReport->bUseFilter && pReport->nFilterID > 0) {
		try {
			// Do different things depending on what the caller wants
			if (pstrOutFilter) {
				// Try to open a recordset based on either the current lookup or the selected group
				_RecordsetPtr prs;
				
				CString strFilterString, strFilterWhere, strFilterFrom;
							
				//Get the filter string
				prs = CreateRecordset("SELECT Type, Filter FROM FiltersT WHERE ID = %li", pReport->nFilterID);
				long nFilterBasedOn = AdoFldLong(prs, "Type");
				strFilterString = AdoFldString(prs, "Filter");
				//Convert the string we have to an SQL
				if(!CFilter::ConvertFilterStringToClause(pReport->nFilterID, strFilterString, nFilterBasedOn, &strFilterWhere, &strFilterFrom)) {
					MsgBox("Report could not be generated because it uses an invalid filter.");
					return FALSE;
				}
				
				//Create the recordset of PatientID's that we need
				//prs = CreateRecordset("Select PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID", strFilterFrom, strFilterWhere);

				
				//add the clause as an IN
				CString strFilter;
				switch(nFilterBasedOn) {
				case fboEMN:
					strFilter.Format(" %s.EmrID IN (SELECT EmrMasterT.ID AS EmrMasterID FROM %s WHERE %s AND EMRMasterT.Deleted = 0 GROUP BY EmrMasterT.ID)", pReport->strRecordSource, strFilterFrom, strFilterWhere);
					break;
				case fboEMR:
					strFilter.Format(" %s.EmrID IN (SELECT EMRMasterT.ID AS EmrMasterID FROM EMRMasterT WHERE EMRGroupID IN (%s WHERE %s) AND EMRMasterT.Deleted = 0 GROUP BY EMRMasterT.ID)", pReport->strRecordSource, strFilterFrom, strFilterWhere);
					break;
				case fboAppointment:
					strFilter.Format(" %s.ApptID IN (SELECT AppointmentsT.ID AS AppointmentID FROM %s WHERE %s GROUP BY AppointmentsT.ID)", pReport->strRecordSource, strFilterFrom, strFilterWhere);
					break;
				default:
					ASSERT(FALSE);
				case fboPerson:
					strFilter.Format("  %s.PatID IN (Select PersonT.ID AS PersonID FROM %s WHERE %s GROUP BY PersonT.ID)", pReport->strRecordSource, strFilterFrom, strFilterWhere); 
					break;
				}
				*pstrOutFilter = strFilter;



				/******I commented this part out because I made it so that it used as IN operator instead of building a bunch
				of OR clauses - JMM


				// Try to use the recordset to generate the patient filter
				// Loop through the list adding each patient into the filter
				CString strFilter, strTemp;
				long nCount;
				FieldPtr fld = prs->Fields->Item["PersonID"];
				while (!prs->eof) {
					strTemp.Format("{%s.PatID} = %i ", pReport->strRecordSource, AdoFldLong(fld));
					strFilter += strTemp + " OR ";
					HR(prs->MoveNext());
					nCount++;
				}
				// Better to add one last " OR " and then remove 
				// it here, than check eof twice on every iteration
				if (strFilter.Right(4) == " OR ") {
					strFilter.Delete(strFilter.GetLength() - 4, 4);
				}
				

				// Return the result
				*pstrOutFilter = strFilter;
				if (pnItemCount) {
					*pnItemCount = nCount;
				}
				************************************************************************/
				
			/*} else if (!pstrOutFilter && pnItemCount) {
				// Try to open a recordset based on either the current lookup or the selected group
				_RecordsetPtr prs;
				if (pReport->nGroup == 0) { 
					#if (PRODUCT_VERSION_LONG != 200104200)
					#error TODO: Make this pull using the current lookup query instead of all patients
					#endif
					prs = CreateRecordset("SELECT COUNT(PersonID) AS ItemCount FROM PatientsT WHERE PersonID <> -25");
				} else if (pReport->nGroup != -9999) {
					prs = CreateRecordset("SELECT COUNT(PersonID) AS ItemCount FROM GroupDetailsT WHERE GroupID = %li", pReport->nGroup);
				} else {
					prs = CreateRecordset("SELECT COUNT(PersonID) AS ItemCount FROM PatientsT WHERE PersonID <> -25");
				}

				// Get the count from the recordset
				ASSERT(prs != NULL && !prs->eof);
				if (!prs->eof) {
					*pnItemCount = AdoFldLong(prs, _T("ItemCount"));
				} else {
					return FALSE;
				}*/
			} else {
				// Both output parameters are null so do nothing
			}
			// Generated a group listing
			return TRUE;
		} NxCatchAll("GlobalReportUtils::GenerateFilter");
	}

	// Didn't generate a group listing
	return FALSE;
}


void SetCommonFileArray(CDWordArray *dwAry, long nReportID) {

	//clear the array
	dwAry->RemoveAll();

	switch (nReportID) {

		case 169: //statement 6.0
		case 234: // indiv statement 6.0
		case 434: //statement by last sent 6.0
			dwAry->Add((long)169);
			dwAry->Add((long)234);
			dwAry->Add((long)434);
		break;

		case 337:  //statement by location 6.0
		case 338:  //Indiv statement by location 6.0
		case 435:  //statement by last sent 6.0
			dwAry->Add((long)337);
			dwAry->Add((long)338);
			dwAry->Add((long)435);
		break;

		case 353:  //statement 7.0
		case 354: //Indiv statement 7.0
		case 436: //statement by last sent 7.0
			dwAry->Add((long)353);
			dwAry->Add((long)354);
			dwAry->Add((long)436);
		break;

		case 355: //statement 7.0 by loc
		case 356: //Indiv statement 7.0 By Loc
		case 437: //Indiv statement by last sent 7.0
			dwAry->Add((long)355);
			dwAry->Add((long)356);
			dwAry->Add((long)437);
		break;

		case 483: //statement by Provider 6.0
		case 484: //Indiv statement by Provider 6.0
			dwAry->Add((long)483);
			dwAry->Add((long)484);
		break;

		case 485: //statement by Provider 7.0
		case 486: //Indiv statement by provider 7.0
			dwAry->Add((long)485);
			dwAry->Add((long)486);
		break;
	}
}



//this function is called right when someone logs in and loads the ID's of the reports that they can access
void LoadAvailableReportsMap () {

	try {
	
		//first, clear the current map
		g_mapAvailReports.RemoveAll();

		//now, check if this user is an administrator
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		if ((GetCurrentUserName() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME)  ||
			(ReturnsRecordsParam("SELECT PersonID FROM UsersT WHERE PersonID = {INT} AND Administrator = 1", GetCurrentUserID()))) {

			//they are an admin, so load every report into the map
			for (long i=0; i< CReports::gcs_nKnownReportCount; i++) {
				g_mapAvailReports.SetAt(CReports::gcs_aryKnownReports[i].nID, CReports::gcs_aryKnownReports[i].nID);
				
			}
			// (c.haag 2006-04-28 17:16) - PLID 20362 - g_bIsAvailReportsLoaded has to be set to
			// true! This is a huge performance hit!
			//return;
		} else {
			//they aren't an administrator, we need to figure out which reports to add
			_RecordsetPtr rsGroups = CreateRecordset("SELECT ReportID, GroupID FROM ReportGroupDetailsT ORDER BY GroupID ASC");
			//this map is only to tell us if we already checked this group or not
			CMap<long, long, BOOL, BOOL> tmpMap;
			BOOL bValue = FALSE;
			//loop through the groups and see which ones the user has access to
			while (!rsGroups->eof) {

				long nGroupID = AdoFldLong(rsGroups, "GroupID");


				if (! tmpMap.Lookup(nGroupID, bValue)) {

					//add the group to our temp map
					tmpMap.SetAt(nGroupID, CheckCurrentUserPermissions(bioReportGroup, sptView, TRUE, nGroupID, TRUE, TRUE));
					
				}

				tmpMap.Lookup(nGroupID, bValue);
				
				if (bValue) {

					long nReportID = AdoFldLong(rsGroups, "ReportID");
					
					if (! g_mapAvailReports.Lookup(nReportID, nReportID)) {
						g_mapAvailReports.SetAt(nReportID, nReportID);
					}
					
				}

				rsGroups->MoveNext();
			}
		}

		//set the bool to true
		g_bIsAvailReportsLoaded = TRUE;
		//long nTest = g_mapAvailReports.GetCount();
	}NxCatchAllCall("Error loading available reports", g_bIsAvailReportsLoaded = FALSE;);


}


void SetReportGroup(long nNewReportID, long nRepIDforGroup) {

	try {

		//first check the preferences, because if they don't want us messing with their groups, we don't have to do any of this
		if (! GetRemotePropertyInt("Reports_DisAllow_New_Addition", 0, 0, "<None>", TRUE)) {
			//get all the groups that this report already exists in
			_RecordsetPtr rsGroups = CreateRecordset("SELECT GroupID FROM ReportGroupDetailsT WHERE ReportID = %li GROUP BY GroupID", nRepIDforGroup);

			while (! rsGroups->eof) {

				long nGroupID = AdoFldLong(rsGroups, "GroupID");

				//add this report ID to all the groups that the old report ID is in
				ExecuteSql("INSERT INTO ReportGroupDetailsT (ReportID, GroupID) VALUES (%li, %li)", nNewReportID, nGroupID);

				rsGroups->MoveNext();
			}
		}

		//now insert the new reportID into ReportGroupSetT
		ExecuteSql("INSERT INTO ReportGroupSetT (ReportID) VALUES (%li)", nNewReportID);
	
	}NxCatchAll("Error adding new report to groups");

}



void EnsureReportGroups() {


	
	//get a list of all reports that have already been taken care of
	CMap<long, long, long, long> mapReports;

	_RecordsetPtr rsReports = CreateRecordset("SELECT ReportID FROM ReportGroupSetT");
	while (!rsReports->eof) {
		long nReportID = AdoFldLong(rsReports, "ReportID");

		mapReports.SetAt(nReportID, nReportID);

		rsReports->MoveNext();
	}

	for (int i = 0; i < CReports::gcs_nKnownReportCount; i++) {

		//only call this for reports not in the table
		long nReportID = CReports::gcs_aryKnownReports[i].nID;
		if (! mapReports.Lookup(nReportID,nReportID)) {
			// (b.cardillo 2006-04-07 10:27) - PLID 20017 - Changed it to call SetReportGroup 
			// directly pulling the nRepIDForGroup from the static object.
			//this just sticks the report in the tab its in unless was either in high financial security, low financial security, or auditing
			ASSERT(CReports::gcs_aryKnownReports[i].nID > 563 || CReports::gcs_aryKnownReports[i].nID == 0);
			ASSERT(CReports::gcs_aryKnownReports[i].nID < CReportInfo::REPORT_NEXT_INFO_ID);
			SetReportGroup(CReports::gcs_aryKnownReports[i].nID, CReports::gcs_aryKnownReports[i].nRepIDForRepGroup);
		}
	}



	//check even if they have the preference checked for developers that add reports
	//check to make sure all reports have been added into a group
	_RecordsetPtr rsMax = CreateRecordset("SELECT Max(ReportID) as MaxID, Count(ReportID) as CountofReps FROM ReportGroupSetT");

	long nMax, nCount;
	nMax = AdoFldLong(rsMax, "MaxID", 0);
	nCount = AdoFldLong(rsMax, "CountofReps", 0);

	//check that the maxID in the table is one less than the next report ID
	//and that there are at least as many ID's in the table as there are in the report array
	// this will ensure that every report is in the table, while taking into account that some reports
	// might have been taken out as some point


	/* IF YOU GOT HERE, YOU NEED TO PUT AN ENTRY FOR YOUR NEW REPORT IN ENSUREDEFAULTREPORTGROUP FUNCTION ABOVE*/
//	ASSERT(nMax == (CReportInfo::REPORT_NEXT_INFO_ID - 1) && nCount >= CReports::gcs_nKnownReportCount);
}

// (j.jones 2016-05-06 10:35) - NX-100501 - changed this to a fixed list, because querying from data is insane
CString GetPaymentsExtendedSqlString(BOOL bIncludeAdjustments, BOOL bIncludeRefunds, BOOL bIncludeGCPayments)
{
	
	CString strMethods =
		"SELECT 1 AS PayMethod, 'Cash' AS Method "
		"UNION SELECT 2 AS PayMethod, 'Check' AS Method "
		"UNION SELECT 3 AS PayMethod, 'Credit Card' AS Method ";

	if (bIncludeRefunds) {
		strMethods +=
			"UNION SELECT 7 AS PayMethod, 'Cash Refund' AS Method "
			"UNION SELECT 8 AS PayMethod, 'Check Refund' AS Method "
			"UNION SELECT 9 AS PayMethod, 'Credit Card Refund' AS Method ";
	}

	if (bIncludeAdjustments) {
		strMethods +=
			"UNION SELECT 0 AS PayMethod, 'Adjustment' AS Method ";
	}

	if (bIncludeGCPayments) {
		strMethods +=
			"UNION SELECT 4 AS PayMethod, 'Gift Certificate' AS Method ";

		if (bIncludeRefunds) {
			strMethods +=
				"UNION SELECT 10 AS PayMethod, 'Gift Certificate Refund' AS Method ";
		}
	}
	
	CString strSql;
	strSql.Format("SELECT Method FROM (%s) AS MethodQ ORDER BY PayMethod", strMethods);
	return strMethods;
}

// (j.gruber 2007-07-17 15:56) - PLID 26686 - I split this out so also so it wouldn't have to be duplicated in a bunch of places
//this is run from the right click menu, it just finds the default custom report and then passes it into the main function
void RunSalesReceipt(COleDateTime dtPayDate, long nPaymentID, long nReportID, long nPatientID, CWnd *pWnd) {

	//we have to find the default report if we are running from the right click menu
	ADODB::_RecordsetPtr rs = CreateRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = 585");
	long nNumber;
	if (rs->eof) {
		nNumber = -2;
	}
	else {
		nNumber = AdoFldLong(rs, "CustomReportID", -2);
	}

	RunSalesReceipt(dtPayDate, nNumber, nPaymentID, TRUE, nReportID, nPatientID, pWnd);

}

// (j.gruber 2007-07-17 15:56) - PLID 26686 - I split this out so also so it wouldn't have to be duplicated in a bunch of places
void RunSalesReceipt(COleDateTime dtPayDate, long nReportNum, long nPaymentID, BOOL bPreview, long nReportID, long nPatientID, CWnd *pWnd) {

	// (j.gruber 2007-07-17 15:47) - PLID 26688 - make this work with the preference
	// (a.wilson 2012-06-14 10:18) - PLID 47966 - carry over old default with change from System to Global
		if (GetRemotePropertyInt("ShowSalesReceiptDialog", 1)) {
			// (a.walling 2008-07-07 17:20) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			CSalesReceiptConfigDlg dlg(dtPayDate, nReportNum, nPaymentID, bPreview, nReportID, nPatientID, NULL);
			dlg.DoModal();
			return;

		}
		else {

			//we need to just run the report without popping up the dialog
			// (j.jones 2015-03-19 16:51) - PLID 65148 - hide original and void items
			_RecordsetPtr rsItems = CreateParamRecordset("SELECT LineItemT.ID "
				"FROM LineItemT "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"LEFT JOIN (SELECT BalancingAdjID FROM LineItemCorrectionsBalancingAdjT) AS LineItemCorrectionsBalancingAdjQ ON LineItemT.ID = LineItemCorrectionsBalancingAdjQ.BalancingAdjID "
				"WHERE LineItemT.Date >= {OLEDATETIME} AND LineItemT.Date < DateAdd(dd, 1,  {OLEDATETIME}) "
				"AND LineItemT.Deleted = 0 AND LineItemT.PatientID = {INT} "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemCorrectionsBalancingAdjQ.BalancingAdjID Is Null",
				AsDateNoTime(dtPayDate), AsDateNoTime(dtPayDate), nPatientID);
			CString strFilter;
			while (! rsItems->eof) {
				strFilter += AsString(rsItems->Fields->Item["ID"]->Value) + ",";

				rsItems->MoveNext();
			}

			CString strLineItemFilter, strPaymentFilter;

			// (j.jones 2009-11-10 17:33) - PLID 34165 - the line item filter goes into a query
			// that already has an IN clause, whereas the payment filter needs an IN clause
			strLineItemFilter = strFilter;

			//the line item filter expects to end in a comma, but it needs
			//to be removed for the payment filter
			strFilter = strFilter.Left(strFilter.GetLength() - 1);

			strPaymentFilter = " AND PaymentID IN (" + strFilter + ") ";

			long nTempReportID;
			
			//see if we are just running to the receipt printer
			if (nReportID == -3) {
				PrintSalesReceiptToReceiptPrinter(strLineItemFilter, strPaymentFilter, strFilter, nReportNum);
				return;
			}
			else if (nReportID  == -2) {
				nTempReportID = 585;
			}
			else {
				nTempReportID = nReportID;
			}

			CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nTempReportID)];
			infReport.strExtraText = strLineItemFilter;
			infReport.strExtendedSql = strPaymentFilter;

			CPtrArray params;
			CRParameterInfo *tmpParam;

			tmpParam = new CRParameterInfo;
			tmpParam->m_Name = "ReceiptCustomInfo";
			tmpParam->m_Data = GetRemotePropertyText("ReceiptCustomInfo", "", 0, "<None>", true);
			params.Add((void *)tmpParam);

			CPrintInfo prInfo;
			if (!bPreview) {

				CPrintDialog* dlg;
				dlg = new CPrintDialog(FALSE);
				prInfo.m_bPreview = false;
				prInfo.m_bDirect = false;
				prInfo.m_bDocObject = false;
				if (prInfo.m_pPD) delete prInfo.m_pPD;
				prInfo.m_pPD = dlg;

			}

			if (nReportNum > 0) {

				//its a custom report, look up the filename
				ADODB::_RecordsetPtr rs = CreateRecordset("SELECT Filename from CustomReportsT WHERE ID = 585 AND Number = %li", nReportNum);
				CString strFileName;
				if (! rs->eof) {
					strFileName = AdoFldString(rs, "FileName", "");
					infReport.nDefaultCustomReport = nReportNum;
				}
				else {
					strFileName = infReport.strReportFile;
				}
				rs->Close();

				if (bPreview) {
					infReport.ViewReport(infReport.strPrintName, strFileName, &params, TRUE, pWnd);
				}
				else {
					infReport.ViewReport(infReport.strPrintName, strFileName, &params, FALSE, pWnd, &prInfo);
				}
			}
			else {

				//it's a system report!
				if (bPreview) {
					infReport.ViewReport(infReport.strPrintName, infReport.strReportFile, &params, TRUE, pWnd);
				}
				else {
					infReport.ViewReport(infReport.strPrintName, infReport.strReportFile, &params, FALSE, pWnd, &prInfo);
				}
			}

			ClearRPIParameterList(&params);	
				
			
		}

}


// (j.gruber 2007-07-17 09:27) - PLID 26686 - made the sales receipt be able to run from different places in the program
void PrintSalesReceiptToReceiptPrinter(CString strLineItemFilter, CString strPaymentFilter, CString strIDs, long nReportNumber)  {

	
	//first, check that they have a POS printer configured
	CWnd *pMainFrame = GetMainFrame();

	if (pMainFrame) {

		//TES 12/6/2007 - PLID 28192 - We're ready to start using the POS Printer, so claim it.
		// (a.walling 2011-03-21 17:32) - PLID 42931 - RAII will save us from all these ReleasePOSPrinter calls
		// not to mention providing exception safety.
		//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();
		POSPrinterAccess pPOSPrinter;

		if (pPOSPrinter) {

		
//		if (((CMainFrame*)pMainFrame)->CheckPOSPrinterEnabled()) {

			//get the settings they want
			ADODB::_RecordsetPtr rsSettings;
			if (nReportNumber > 0) {
				rsSettings = CreateRecordset("SELECT * FROM POSReceiptsT WHERE ID = %li", nReportNumber);
			}
			else {
				//use the default
				long nDefault = GetRemotePropertyInt("POSReceiptDefaultSettings",-1, 0, GetCurrentLocationName());
				if (nDefault == -1) {
					MsgBox("Please select a Receipt Printer receipt to be your default");
					return;
				}
				rsSettings = CreateRecordset("SELECT * FROM POSReceiptsT WHERE ID = %li", nDefault);
			}

			if (rsSettings->eof) {
				ThrowNxException("Receipt Printer Settings could not be found");
				return;
			}
			
			ADODB::FieldsPtr flds = rsSettings->Fields;
			
			CString strLogoPath;
			CString strLocationFormat;
			CString strFooterMessage;
			long nLinesAfterDetails;
			long nLinesAfterHeader;
			long nShowLogo;

			nShowLogo = AdoFldBool(flds, "ShowLogo", FALSE) ? 1 :  0;
			strLogoPath = AdoFldString(flds, "LogoPath", "");
			strLocationFormat = AdoFldString(flds, "LocationFormat", "");
			nLinesAfterHeader = AdoFldLong(flds, "LinesAfterHeader",2);
			nLinesAfterDetails = AdoFldLong(flds, "LinesAfterDetails", 2);
			strFooterMessage = AdoFldString(flds, "FooterMessage", "");

			// (a.walling 2011-06-09 14:16) - PLID 43459
			long nDefaultCharCount = pPOSPrinter->GetLineWidth();

			// (j.gruber 2008-01-16 17:06) - PLID 28310 - added these for printing with different fonts
			long nFontNumber = AdoFldLong(flds, "FontNumber", 1);
			long nCharCount = AdoFldLong(flds, "CharacterCount", nDefaultCharCount);


			//first, print the header
			if (! PrintReceiptHeader(pPOSPrinter, strIDs, nShowLogo, strLogoPath, strLocationFormat, nFontNumber, nCharCount)) {
				return;
			}
		

			//now skip some lines
			for (int i = 0; i < nLinesAfterHeader; i++) {
				if (!pPOSPrinter->PrintText("\n") ) {
					return;
				}
			}

			//now print the detail
			// (a.walling 2011-08-05 12:04) - PLID 44902 - There is no reason to allocate this on the heap!
			TotalsStruct totals;

			CString strCardHoldersName;
			// (a.walling 2011-06-09 14:16) - PLID 43459 - This is not in a special font, unlike the header/footer, so make sure it uses the right char width
			if (!PrintReceiptDetail(pPOSPrinter, strLineItemFilter, strPaymentFilter, &totals, strCardHoldersName, nDefaultCharCount)) {				
				return;
			}

			//now print the summary information
			// (a.walling 2011-06-09 14:16) - PLID 43459 - This is not in a special font, unlike the header/footer, so make sure it uses the right char width
			if (!PrintReceiptPaymentSummary(pPOSPrinter, &totals, strCardHoldersName, nDefaultCharCount) ){
				return;
			}

			//now skip some more lines
			for (i = 0; i < nLinesAfterDetails; i++) {
				if (!pPOSPrinter->PrintText("\n") ) {
					return;
				}
			}

			//now see if there is a footer
			if (!PrintReceiptFooter(pPOSPrinter, strIDs, strFooterMessage, nFontNumber, nCharCount)) {
				return;
			}

			//feed some out
			for (i = 0; i < 5; i++) {
				// (a.walling 2009-12-03 17:50) - PLID 36492 - This needs to be \n, not \r
				// otherwise the footer may end up at the top of the next receipt.
				if (! pPOSPrinter->PrintText("\n") ) {
					return;
				}

			}


			//now cut
			// (a.walling 2011-04-28 10:02) - PLID 43492
			if (!pPOSPrinter->FlushAndTryCut()) {
				return;
			}

			//TES 12/6/2007 - PLID 28192 - Now release the POS Printer, since we're done with it, so any third-party
			// applications that want it can use it.
			//((CMainFrame*)pMainFrame)->ReleasePOSPrinter();
		}
	}


}

// (j.gruber 2008-01-16 17:08) - PLID 28310 - replaced function for formatted printing
/*CString CenterLine(CString strText, long nLineWidth) {

	CString strDesc;
	
	long nCenterLine = (nLineWidth)/2;
	long nCenterName = strText.GetLength()/2;
	long nDiff = nCenterLine - nCenterName;
	for (int i = 0; i < nDiff; i++) {
		strDesc += " ";
	}
	strDesc += strText;
	

	return strDesc + "\n";
}*/


// (j.gruber 2008-01-16 17:08) - PLID 28310 - added function for formatted printing
CString RemoveAllFormats(CString strText) {

	
	try {
		// (a.walling 2011-06-09 15:50) - PLID 44046 - Infinite loop here. It looks like all this is doing is just getting 
		// rid of the escape codes. Thankfully we know they are all of the format <ESC>|...X where X is some capital letter,
		// and ... are alphanumeric
		/*
		long nFindFormat = strText.Find("<ESC>");
		while (nFindFormat != -1) {

			long nFindFormatEnd = strText.Find("C", nFindFormat + 5);

			if (nFindFormatEnd != -1) {
				strText = strText.Left(nFindFormat) + strText.Right(strText.GetLength() - (nFindFormatEnd + 1));

				nFindFormat = strText.Find("<ESC>");
			}
			else {
				//must be font
				// (a.walling 2011-06-09 15:50) - PLID 44046 - Best not to think in absolutes!
				nFindFormatEnd = strText.Find("T", nFindFormat + 5);
				if (nFindFormatEnd != -1) {
					strText = strText.Left(nFindFormat) + strText.Right(strText.GetLength() - (nFindFormatEnd + 1));

					nFindFormat = strText.Find("<ESC>");
				}
				else {
					ASSERT(FALSE);
				}
			}
		}
		*/

		long nEscape;
		while (-1 != (nEscape = strText.Find("<ESC>|"))) {
			long nEscapeEnd = nEscape + 6;

			while (nEscapeEnd < strText.GetLength()) {
				char c = strText[nEscapeEnd];
				if (!isalnum((unsigned char)c)) {
					// bad code, just stop here.
					ASSERT(FALSE);
					break;
				} else if (c >= 'A' && c <= 'Z') {
					// last piece of code.
					nEscapeEnd++;
					break;
				} else {
					nEscapeEnd++;
				}
			}

			strText.Delete(nEscape, nEscapeEnd - nEscape);
		}


		strText.Replace("<START_CNTRJST>", "");
		strText.Replace("<END_CNTRJST>", "");
		strText.Replace("<START_RHTJST>", "");
		strText.Replace("<END_RHTJST>", "");
		strText.Replace("<START_LFTJST>", "");
		strText.Replace("<END_LFTJST>", "");
		strText.Replace("<START_BOLD>", "");
		strText.Replace("<END_BOLD>", "");
		strText.Replace("<START_ITL>", "");
		strText.Replace("<END_ITL>", "");
		strText.Replace("<START_UNDL>", "");
		strText.Replace("<END_UNDL>", "");
		strText.Replace("<START_DBLWD>", "");
		strText.Replace("<END_DBLWD>", "");
		strText.Replace("<START_DBLHGH>", "");
		strText.Replace("<END_DBLHGH>", "");
		strText.Replace("<START_DBLWDHGH>", "");
		strText.Replace("<END_DBLWDHGH>", "");


		// (a.walling 2011-06-09 15:50) - PLID 44046 - what the? I'll leave it alone for now.
		strText.Replace("START_CNTRJST>", "");
		strText.Replace("END_CNTRJST>", "");
		strText.Replace("START_RHTJST>", "");
		strText.Replace("END_RHTJST>", "");
		strText.Replace("START_LFTJST>", "");
		strText.Replace("END_LFTJST>", "");

		return strText;
	}NxCatchAll("Error in RemoveAllFormats");

	return "";
}

// (j.gruber 2008-01-16 17:08) - PLID 28310 - added function for formatted printing
long GetTextLengthIgnoringFormatsWithDoubleFormatting(CString strText, BOOL &bLastDoubleWide) {

	try {
		long nLength = 0;
		if (bLastDoubleWide) {

			//the last line had a double wide in it, so let's see it this one ends it
			if (strText.Find("<END_DBLWD>") == -1 && strText.Find("<END_DBLWDHGH>") == -1) {

				//it might not be the whole line if the Start began on this line and we are checking letter by letter
				if (strText.Find("<START_DBLWD") == -1 || strText.Find("<START_DBLWDHGH>") == -1) {

					//the whole line is double wide
					CString strTmp = RemoveAllFormats(strText);
					strTmp.TrimLeft();
					strTmp.TrimRight();

					bLastDoubleWide = TRUE;
		
					return strTmp.GetLength() * 2;
				}
				else {
					//we started the double wide on this line, so don't could the letters before it started as double wide
					long nResult = strText.Find("<START_DBLWD>");
					if (nResult == -1 ){
						nResult = strText.Find("<START_DBLWDHGH>");
					}

					CString strTmp = strText.Left(nResult);
					strTmp = RemoveAllFormats(strTmp);

					nLength += strTmp.GetLength();

					//now the part after the start
					strTmp = strText.Right(strText.GetLength() - (nResult));
					strTmp = RemoveAllFormats(strTmp);
					nLength += strTmp.GetLength();

					return nLength;
				}
			}
			else {
				//it ends on this line
				long nResult = strText.Find("<END_DBLWD>");
				if (nResult == -1) {
					//it must be the other format
					nResult = strText.Find("<END_DBLWDHGH>");
				}

				CString strTmp = strText.Left(nResult);
				strTmp = RemoveAllFormats(strTmp);
				

				nLength = strTmp.GetLength() * 2;
				strText = strTmp;
				bLastDoubleWide = FALSE;
			}
		}

		//see if there are any double wide formats 
		if (strText.Find("<START_DBLWD>") != -1 || strText.Find("<START_DBLWDHGH>") != -1) {

			//we found one, set the start char
			long nResult = strText.Find("<START_DBLWD>");
			if (nResult == -1) {
				nResult = strText.Find("<START_DBLWDHGH>");
			}

			CString strTmp = strText.Left(nResult);
			strTmp = RemoveAllFormats(strTmp);
			//this is just the normal text
			nLength += strTmp.GetLength();

			//find where the end double is
			if (strText.Find("<END_DBLWD>", nResult) != -1 || strText.Find("<END_DBLWDHGH>", nResult) != -1) {

				//it ends on this line also, so find where that is
				BOOL bDblWd = TRUE;
				long nEndChar = strText.Find("<END_DBLWD>", nResult);
				if (nEndChar == -1) {
					nEndChar = strText.Find("<END_DBLWDHGH>", nResult);
					bDblWd = FALSE;
				}

				CString strTmp = strText.Mid(nResult, nEndChar - nResult);
				strTmp = RemoveAllFormats(strTmp);
				nLength += strTmp.GetLength() * 2;

				//now we have to see if there is another one
				if (strText.Find("<START_DBLWD>", nEndChar) != -1 || strText.Find("<START_DBLWDHGH>", nEndChar) != -1) {
					//here we go again
					CString strTemp;
					if (bDblWd) {
						strTemp = strText.Right(strText.GetLength() - (nEndChar + 11));
					}
					else {
						strTemp = strText.Right(strText.GetLength() - (nEndChar + 14));
					}

					return nLength += GetTextLengthIgnoringFormatsWithDoubleFormatting(strTemp, bLastDoubleWide);
				}
				else {
					
					//we just need to add the remaining stuff
					CString strtmp = strText.Right((strText.GetLength() - nEndChar));
					strtmp = RemoveAllFormats(strtmp);
					return nLength + strtmp.GetLength();
					
				}


			}
			else {
				//it doesn't end on this line, so get everything from the start to the end
				CString strTmp = strText.Right(strText.GetLength() - (nResult));
				strTmp = RemoveAllFormats(strTmp);
				nLength += strTmp.GetLength() * 2;

				bLastDoubleWide = TRUE;
			}
		}
		else {

			//its just a normal line
			strText = RemoveAllFormats(strText);
			strText.TrimLeft();
			strText.TrimRight();
			
			return strText.GetLength();
		}

		return nLength;
	}NxCatchAll("Error in GetTextLengthIgnoringFormatsWithDoubleFormatting");

	return -1;

}

// (j.gruber 2008-01-16 17:08) - PLID 28310 - added function for formatted printing
BOOL IsFormat(CString strText, CString &strFormat, long &nFormatLength) {

	try {
		//find the next >
		long nEndChar = strText.Find(">");
		if (nEndChar == -1) {
			//its not a format
			nFormatLength = -1;
			strFormat = "";
			return FALSE;
		}
		else {
			strFormat = strText.Left(nEndChar) + ">";

			// (a.walling 2011-06-09 15:50) - PLID 44046 - This did not include underlines...
			if (strFormat == "<START_BOLD>" ||
				strFormat == "<END_BOLD>" ||
				strFormat == "<START_ITL>" ||
				strFormat == "<END_ITL>" ||
				strFormat == "<START_UNDL>" ||
				strFormat == "<END_UNDL>" ||
				strFormat == "<START_DBLWD>" ||
				strFormat == "<END_DBLWD>" ||
				strFormat == "<START_DBLHGH>" ||
				strFormat == "<END_DBLHGH>" ||
				strFormat == "<START_DBLWDHGH>" ||
				strFormat == "<END_DBLWDHGH>" ||
				strFormat == "<START_CNTRJST>" ||
				strFormat == "<END_CNTRJST>" ||
				strFormat == "<START_RHTJST>" ||
				strFormat == "<END_RHTJST>" || 
				strFormat == "<START_LFTJST>" ||
				strFormat == "<END_LFTJST>") {

				nFormatLength = strFormat.GetLength();
				
				return TRUE;
			}
			else {
				nFormatLength = -1;
				strFormat = "";
				return FALSE;
			}
		}
	}NxCatchAll("Error in IsFormat");

	return FALSE;
}

// (j.gruber 2008-01-16 17:08) - PLID 28310 - added function for formatted printing
void WordWrap(CStringArray *strText, long nLineWidth)  {

	try {
	
		CStringArray strReturnArray;
		long nArraySize = strText->GetSize();

		for (int i = 0; i < nArraySize; i++) {

			CString strLine = strText->GetAt(i);
			BOOL bLastDoubleWideText = FALSE;

			if (GetTextLengthIgnoringFormatsWithDoubleFormatting(strLine, bLastDoubleWideText) > nLineWidth) {

				//we have to wrap this line
				CString strWrappedLine;

				//find the first space
				long nSpace = strLine.Find(" ");
				//there are no spaces on the line
				//wrap each letter until we get to the end
				for (int j = 0; j < strLine.GetLength(); j++) {
					if(GetTextLengthIgnoringFormatsWithDoubleFormatting(strWrappedLine, bLastDoubleWideText) < nLineWidth) {
						if (strLine.GetAt(j) == '<') {
							//it might be a formatting character
							CString strFormat;
							long nFormatLength;
							if (IsFormat(strLine.Right(strLine.GetLength() - j), strFormat, nFormatLength)) {
								strWrappedLine += strFormat;
								//subtract 1 because we were already at the <
								j += nFormatLength - 1;
							}
							else {
								//its just a normal character
								strWrappedLine += strLine.GetAt(j);
							}
						}
						else {
							//add it to the line
							strWrappedLine += strLine.GetAt(j);
						}
					}
					else {
						//go back to the last space we had
						long nSpace = strWrappedLine.ReverseFind(' ');
						if (nSpace == -1) {
							//no spaces, just use the last character
							//we have to add a line break
							strWrappedLine.TrimLeft();
							strWrappedLine.TrimRight();
							strReturnArray.Add(strWrappedLine);
							strWrappedLine = "";
							//put the index back so that we don't skip that letter
							j--;
						}
						else {

							CString strAdd = strWrappedLine.Left(nSpace);
							strAdd.TrimLeft();
							strAdd.TrimRight();
							strReturnArray.Add(strAdd);
							strWrappedLine = strWrappedLine.Right(strWrappedLine.GetLength() - nSpace);
							strWrappedLine.TrimLeft();
							j--;
						}
						
					}
				}
				
				//we are at the end, add the rest to the array
				strWrappedLine.TrimLeft();
				strWrappedLine.TrimRight();
				if (!strWrappedLine.IsEmpty()) {
					strReturnArray.Add(strWrappedLine);
				}
				
			} 
			else {
				//we can just add it to the return array
				strLine.TrimLeft();
				strLine.TrimRight();
				strReturnArray.Add(strLine);
			}
		}

		//now clear the original array
		strText->RemoveAll();

		for (int k = 0; k < strReturnArray.GetSize(); k++) {
			strText->Add(strReturnArray.GetAt(k));
		}
	}NxCatchAll("Error in WordWrap");
}


// (j.gruber 2008-01-16 17:08) - PLID 28310 - added function for formatted printing
void ReplaceFormattedText(COPOSPrinterDevice* pPOSPrinter, CString strIDs, CString IN strText, CStringArray OUT *aryStr, long nFontNumber, long nCharCount) 
{

	try {

		//print the address
		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		ADODB::_RecordsetPtr rsAddress = CreateRecordset(GetRemoteDataSnapshot(), "SELECT Name, Address1, Address2, City, State, Zip, Phone "
			" FROM LocationsT WHERE ID = (SELECT top 1 LocationID FROM LineItemT WHERE ID IN (%s))", strIDs);

		ADODB::FieldsPtr flds = rsAddress->Fields;
		CString strOutput;
		if (! rsAddress->eof) {

			strText.Replace("<Name>", AdoFldString(flds, "Name", ""));
			strText.Replace("<Address1>", AdoFldString(flds, "Address1", ""));
			strText.Replace("<Address2>", AdoFldString(flds, "Address2", ""));
			strText.Replace("<City>", AdoFldString(flds, "City", ""));
			strText.Replace("<State>",  AdoFldString(flds, "State", ""));
			strText.Replace("<Zip>", AdoFldString(flds, "Zip", ""));			
			strText.Replace("<MainPhone>", AdoFldString(flds, "Phone", ""));
		}
		else {

			//this will only happen if they are doing a test print
			// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
			rsAddress = CreateRecordset(GetRemoteDataSnapshot(), "SELECT Name, Address1, Address2, City, State, Zip, Phone "
			" FROM LocationsT WHERE ID = %li", GetCurrentLocationID());
			flds = rsAddress->Fields;
			if (! rsAddress->eof) {

				strText.Replace("<Name>", AdoFldString(flds, "Name", ""));
				strText.Replace("<Address1>", AdoFldString(flds, "Address1", ""));
				strText.Replace("<Address2>", AdoFldString(flds, "Address2", ""));
				strText.Replace("<City>", AdoFldString(flds, "City", ""));
				strText.Replace("<State>",  AdoFldString(flds, "State", ""));
				strText.Replace("<Zip>", AdoFldString(flds, "Zip", ""));			
				strText.Replace("<MainPhone>", AdoFldString(flds, "Phone", ""));
			}
		}

		
		// (j.gruber 2008-06-11 16:20) - PLID 28631 - added next appt date
		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		ADODB::_RecordsetPtr rsPatient = CreateRecordset(GetRemoteDataSnapshot(), "SELECT UserDefinedID, First, Middle, Last, "
			" (SELECT Top 1 StartTime FROM AppointmentsT WHERE StartTime > GetDate() AND Status <> 4 AND AppointmentsT.PatientID = PersonT.ID ORDER BY StartTime ASC) as NextApptDate "
			" FROM PatientsT INNER JOIN "
			" PersonT ON PatientsT.PersonID = PersonT.ID "
			" WHERE PersonT.ID IN (SELECT Top 1 PatientID FROM LineItemT WHERE ID IN (%s))", strIDs);
		if (!rsPatient->eof) {

			strText.Replace("<PatFirst>", AdoFldString(rsPatient, "First", ""));
			strText.Replace("<PatMiddle>", AdoFldString(rsPatient, "Middle", ""));
			strText.Replace("<PatLast>", AdoFldString(rsPatient, "Last", ""));
			CString strID;
			strID.Format("%li", AdoFldLong(rsPatient, "UserDefinedID", -1));
			strText.Replace("<PatID>", strID);

			COleDateTime dtNULL, dtDate;
			dtNULL.SetDate(1899,12,31);
			dtDate = AdoFldDateTime(rsPatient, "NextApptDate", dtNULL);
			CString strDate; 
			if (dtDate == dtNULL) {
				strDate = "<None>";
			}
			else {
				strDate = FormatDateTimeForInterface(dtDate, NULL, dtoDateTime, true);
			}
			strText.Replace("<PatNextApptDate>", strDate);
		}
		else {
			//this will only happen if they are doing a test print
			strText.Replace("<PatFirst>", "Test");
			strText.Replace("<PatMiddle>", "P");
			strText.Replace("<PatLast>", "Patient");
			strText.Replace("<PatID>", "12345");

			strText.Replace("<PatNextApptDate>", FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoDateTime, true));
		}


		//the formatting is a little more difficult, first break everything into lines
		CString strTemp = strText;
		long nResult = strTemp.Find("\r\n");
		while (nResult != -1) {
			aryStr->Add(strTemp.Left(nResult));
			strTemp = strTemp.Right(strTemp.GetLength() - (nResult + 2));

			nResult = strTemp.Find("\r\n");						
		}

		//now do the last line
		aryStr->Add(strTemp);

		//word wrap 
		WordWrap(aryStr, nCharCount);
		

		CStringArray straryFormatStart;
		//now go through each line, and if a format starts, but doesn't end on that line, keep it through until the next line
		for (int i = 0; i < aryStr->GetSize(); i++) {
			
			CString strLine = aryStr->GetAt(i);


			//first, add all our starts from the previous line to this line
			CString strStarts;
			for (int j = 0; j < straryFormatStart.GetSize(); j++ ) {
				strStarts += straryFormatStart.GetAt(j);
			}

			CString strTemp = aryStr->GetAt(i);
			aryStr->RemoveAt(i);
			strTemp = strStarts + strTemp;
			aryStr->InsertAt(i, strTemp);
			
				
			//now see if we have any new strings to add
			CStringArray straryCurrentLineFormatStart;

			long nResult = strLine.Find("<START_");
			while (nResult != -1) {

				//get the entire statement
				long nResult2 = strLine.Find(">", nResult);
				// (a.walling 2011-06-09 15:50) - PLID 44046 - What if nResult2 is -1? this can corrupt hte heap or crash
				if (nResult2 == -1) {
					LogDetail("WARNING: Invalid formatting line %s", strLine);
					ASSERT(FALSE);
					nResult = -1;
					break;
				} else {
					CString strEndFormat = "<END_" + strLine.Mid(nResult + 7, (nResult2-(nResult + 7))) + ">";
					CString strStartFormat = "<START_" + strLine.Mid(nResult + 7, (nResult2-(nResult + 7))) + ">";

					//now see if it ends on this line
					if (strLine.Find(strEndFormat) == -1) {
						//add it to the array
						straryCurrentLineFormatStart.Add(strStartFormat);
					}

					nResult = strLine.Find("<START_", nResult2);
				}
			}

			
			//now that we've looked if formats that started on this line ended on this line,
			//we need to look if formats that didn't start on this line end on this line
			long nCount = straryFormatStart.GetSize();
			for (int k = nCount - 1; k >= 0; k--) {
				CString strEndString = straryFormatStart.GetAt(k);
				strEndString.Replace("START", "END");
				if (strLine.Find(strEndString) != -1) {
					//we found it
					straryFormatStart.RemoveAt(k);
				}
			}

			//now add the current line's starts to the whole
			for (int l = 0; l < straryCurrentLineFormatStart.GetSize(); l++) {
				straryFormatStart.Add(straryCurrentLineFormatStart.GetAt(l));
			}
			straryCurrentLineFormatStart.RemoveAll();
			
		}

		ASSERT(straryFormatStart.GetSize() == 0);
		straryFormatStart.RemoveAll();

		
		//add the array values back to the string
		for (i = 0; i < aryStr->GetSize(); i++ ){

			strText = aryStr->GetAt(i);

			// (a.walling 2011-04-29 12:24) - PLID 43507 - The <END_x> tokens all just reset the character width...
			// We want to reset the format attribute. Since nesting never really worked beforehand (and this led
			// to styles persisting to the end of the line, which was mistakenly blamed on the printer / driver)
			// I think we are OK to say you just need to repeat format specs again after closing any if nesting them.
			
			strText.Replace("<START_BOLD>", "<ESC>|bC");
			strText.Replace("<END_BOLD>", "<ESC>|N");

			strText.Replace("<START_ITL>", "<ESC>|iC");
			strText.Replace("<END_ITL>", "<ESC>|N");

			strText.Replace("<START_UNDL>", "<ESC>|2uC");
			strText.Replace("<END_UNDL>", "<ESC>|N");

			strText.Replace("<START_DBLWD>", "<ESC>|2C");
			strText.Replace("<END_DBLWD>", "<ESC>|N");

			strText.Replace("<START_DBLHGH>", "<ESC>|3C");
			strText.Replace("<END_DBLHGH>", "<ESC>|N");

			strText.Replace("<START_DBLWDHGH>", "<ESC>|4C");
			strText.Replace("<END_DBLWDHGH>", "<ESC>|N");			

			//line chars
			if (strText.Find("<ESC>|2C") != -1 || strText.Find("<ESC>|4C") != -1) {
				strText = FormatText(pPOSPrinter, strText, TRUE, nCharCount);
			}
			else {
				strText = FormatText(pPOSPrinter, strText, FALSE, nCharCount);
			}
			
			//now replace the <ESC>
			CString strESC = char(27);
			strText.Replace("<ESC>", strESC);

			aryStr->RemoveAt(i);
			aryStr->InsertAt(i, strText);
		}

	}NxCatchAll("Error in GlobalReportUtils::ReplaceFormattedText");

}


long GetTextLengthIgnoringFormats(CString strText) {

	try {
		strText = RemoveAllFormats(strText);

		strText.TrimLeft();
		strText.TrimRight();

		return strText.GetLength();
	}NxCatchAll("Error in GetTextLengthIgnoringFormats");

	return strText.GetLength();

}


CString FormatText(COPOSPrinterDevice* pPOSPrinter, CString strTextToFormat, BOOL bIsDoubleWide, long nLineChars) {

	try {
		if (pPOSPrinter) {

			COPOSPrinterDevice *pPrinter = pPOSPrinter;

			// (a.walling 2011-06-09 14:16) - PLID 43459 - Will never be -1
			long nLineWidth = nLineChars;
			/*long nLineWidth = pPrinter->GetLineWidth();
			if (nLineChars != -1) {
				nLineWidth = nLineChars;
			}			*/

			if (bIsDoubleWide) {
				nLineWidth = nLineWidth/2;
			}

			CString strReturn;
			//check to see if there is a Center, left, or right justify on here 
			long nFindCenterStart = strTextToFormat.Find("<START_CNTRJST>");
				
			if (nFindCenterStart != -1) {

				strTextToFormat.Replace("<START_CNTRJST>", "");
				strTextToFormat.Replace("<END_CNTRJST>", "");
				strReturn = CenterLine(strTextToFormat, nLineWidth);
			}
			else {

				//see if we are right justifying
				long nFindRight = strTextToFormat.Find("<START_RHTJST>");
				
				if (nFindRight != -1) {
					//are we also left justifying some of the line?
					long nFindLeft = strTextToFormat.Find("<START_LFTJST>");
					if (nFindLeft != -1 ) {

						//we are both left and right justifying the line

						//find the end of the left justify
						long nFindLeftEnd = strTextToFormat.Find("<END_LFTJST>");

						//CString strLeft = strTextToFormat.Mid(nFindLeft + 14, (nFindLeftEnd - (nFindLeft + 14)));
						CString strLeft = strTextToFormat.Left(nFindLeftEnd);

						//now for the right
						long nFindRightEnd = strTextToFormat.Find("<END_RHTJST>");

						//CString strRight = strTextToFormat.Mid(nFindRight + 14, (nFindRightEnd - (nFindRight + 14)));
						CString strRight = strTextToFormat.Right(strTextToFormat.GetLength() - (nFindRight + 14));

						//remove the left and right justs here
						strLeft.Replace("<START_RHTJST>", "");
						strLeft.Replace("<END_RHTJST>", "");
						strLeft.Replace("<START_LFTJST>", "");
						strLeft.Replace("<END_LFTJST>", "");
						strRight.Replace("<START_RHTJST>", "");
						strRight.Replace("<END_RHTJST>", "");
						strRight.Replace("<START_LFTJST>", "");
						strRight.Replace("<END_LFTJST>", "");

						//strRight = RemoveAllFormats(strRight);

						//now format the line
						strReturn = LeftRightJustify(strLeft, strRight, nLineWidth);
					}
					else {

						//we are just right justifying, but still could have more to left justify on the left only
						//we are assuming the right justify ends at the end of the line, because anything else wouldn't make sense
						CString strLeft = strTextToFormat.Left(nFindRight);
						
						if (strLeft.IsEmpty()) {

							strTextToFormat.Replace("<START_RHTJST>", "");
							strTextToFormat.Replace("<END_RHTJST>", "");

							//we are right Justifying the whole thing
							strReturn = LeftRightJustify("", strTextToFormat, nLineWidth);
						}
						else {
							CString strRight = strTextToFormat.Right(strTextToFormat.GetLength() - (nFindRight + 14));
							//strRight = RemoveAllFormats(strRight);

							strRight.Replace("<END_RHTJST>", "");

							strReturn = LeftRightJustify(strLeft, strRight, nLineWidth);
						}
					}
				}
				else {
					//we must just have a left justify or nothing, so do that
					strTextToFormat.Replace("<START_LFTJST>", "");
					strTextToFormat.Replace("<END_LFTJST>", "");
					strReturn = LeftJustify(strTextToFormat, nLineWidth);
				}
			}

			return strReturn;
		}

		//can't get the printer?
		return "";
	}NxCatchAll("Error in FormatText");

	return "";

}




//TES 12/6/2007 - PLID 28192 - This now takes the POSPrinter as a parameter.
BOOL PrintReceiptHeader(COPOSPrinterDevice* pPOSPrinter, CString strIDs, long nShowLogo, CString strLogoPath, CString strLocationFormat, long nFontNumber, long nCharCount) {

	CWnd *pMainFrame  = GetMainFrame();

	if (pMainFrame) {
		if (nShowLogo == 1) {

			//find the bitmap they want to use
			if (strLogoPath.IsEmpty()) {

				//throw an error because this shouldn't happen
				MsgBox("You have selected the option to print a logo at the top of the receipt, but have not specified a logo file.\nPlease set this in the Receipt Printer settings and try again.");
				return FALSE;
			}
			else {
				if (! pPOSPrinter->PrintBitmap(strLogoPath) ) {
					return FALSE;
				}
			}
		}

		// (j.gruber 2008-01-16 17:07) - PLID 28310 - added new formatted printing

		CStringArray arystrHeader;
		ReplaceFormattedText(pPOSPrinter, strIDs, strLocationFormat, &arystrHeader, nFontNumber, nCharCount);

		CString strFont;
		strFont.Format("<ESC>|%lifT", nFontNumber);
		CString strEsc = char(27);
		strFont.Replace("<ESC>"	, strEsc);
		for (int i = 0; i < arystrHeader.GetSize(); i++) {
			CString strText = strFont + arystrHeader.GetAt(i);
			strText.Replace("\n", "");
				
			// (a.walling 2011-04-27 10:08) - PLID 43459 - Reset the font when we are done
			// (a.walling 2011-06-10 16:43) - PLID 43459 - Reset to font 1; some printers seem to die otherwise
			if (i == arystrHeader.GetSize() - 1) {
				strText += "\x1b|1fT";
			}

			strText += "\n";
			
			if (!pPOSPrinter->PrintText(strText)) {
				return FALSE;
			}
		}

		return TRUE;	
	}
	return FALSE;

}

//this function formats a description appropriately, meaning it gets the length of the receipt width and calculates whether it needs to wrap the description or not
//TES 12/6/2007 - PLID 28192 - This now takes the POSPrinter as a parameter.
// (d.lange 2013-04-17 11:48) - PLID 55889 - Added flag for bolding the description
BOOL GetReceiptDescription(COPOSPrinterDevice* pPOSPrinter, CString strDate, CString &strDescription, CString strQuantity, CString strAmount, BOOL bShowParenthesis, long nCharCount, BOOL bBoldDesc) {

	long nDateLen = strDate.GetLength();
	long nDescLen = strDescription.GetLength();
	long nQuantityLen = strQuantity.GetLength();
	long nAmountLen = strAmount.GetLength();

	CString strBeginBold = "\x1b|bC";
	CString strEndBold = "\x1b|N";

	//long nWidthOfPrinter = pPOSPrinter->GetLineWidth();
	long nWidthOfPrinter = nCharCount;

	if (nWidthOfPrinter != -1) {

		long nTotalWidth;
		if (bShowParenthesis) {
			nTotalWidth = nDateLen + nDescLen + nQuantityLen + nAmountLen + 9; //add 9 for 3 spaces between items and () around quantity and an extra space
		}
		else {
			if (!strQuantity.IsEmpty()) {
				nTotalWidth = nDateLen + nDescLen + nQuantityLen + nAmountLen + 9; //add 9 for 3 spaces between items 
			}
			else {
				nTotalWidth = nDateLen + nDescLen + nQuantityLen + nAmountLen + 6; //add 6 for 2 spaces between items 
			}
		}

		if (!strAmount.IsEmpty() && strAmount.GetAt(0) != '(') {
			nTotalWidth++;
		}
		
		
		if (nTotalWidth <=  nWidthOfPrinter) {

			//we are good to go
			CString strReturn = strDescription;

			if(bBoldDesc) {
				strReturn = strBeginBold + strDescription + strEndBold;
			}

			if (!strQuantity.IsEmpty()) {
				if (bShowParenthesis) {
					strReturn += " (" + strQuantity + ")" + "   ";
				}
				else {
					strReturn += "   " + strQuantity + "   ";
				}
			}
			else {
				strReturn += "   ";
			}
			long nAmtToAdd = (nWidthOfPrinter - nTotalWidth);
			for (int i = 0; i < nAmtToAdd; i++) {
				strReturn += " ";
			}
			strReturn += strAmount + "\n";

			strDescription = strReturn;
			return TRUE;
		}

		//we have some fixing to do...we are putting the price on the first line
		long nDefaultLen = nDateLen + nAmountLen + 6; // we don't know if the quantity will fit, so we aren't counting the () here

		long nUsableAmt = nWidthOfPrinter - nDefaultLen;

		CString strTotalDescription = strDescription;
		if (!strQuantity.IsEmpty()) {
			if (bShowParenthesis) {
				strTotalDescription += " (" + strQuantity + ")";
			}
			else {
				strTotalDescription += "   " + strQuantity;
			}
			

		}
		CString strReturn;

		//see if we can description has spaces in it in order to split there
		long nResult = strTotalDescription.Find(" ");
		long nLength = 0;
		long nLine = 1;
		if (nResult == -1) {
			
			//there aren't any spaces we will just have to wrap until its done
			long nTotalLength = strTotalDescription.GetLength();
			while(nLength < nTotalLength) {

				if (nLine == 1) {
					if(bBoldDesc) {
						strReturn = strBeginBold + strTotalDescription.Left(nUsableAmt) + strEndBold;
					}else {
						strReturn = strTotalDescription.Left(nUsableAmt);
					}
					
					strReturn += "   ";

					long nAmtToAdd = (nWidthOfPrinter - strReturn.GetLength() - nAmountLen - (nDateLen + 3));
					if (!strAmount.IsEmpty() && strAmount.GetAt(0) != '(') {
						nAmtToAdd--;
					}
					for (int i = 0; i < nAmtToAdd; i++) {
						strReturn += " ";
					}

					


					strReturn += strAmount + "\n";

					
				}
				else {

					for (int i = 0; i < nDateLen; i++) {

						strReturn += " ";
					}
					//add 3 spaces for the break
					strReturn += "   ";
					strReturn += (bBoldDesc ? strBeginBold + strTotalDescription.Left(nUsableAmt) + strEndBold : strTotalDescription.Left(nUsableAmt));
					strReturn += "\n";
				
				}

				strTotalDescription = strTotalDescription.Right(strTotalDescription.GetLength() - (nUsableAmt));
				if (strTotalDescription.IsEmpty()) {
					strReturn += "\n";
				}
				nLine++;
				nLength += nUsableAmt;
			}
		}
		else {

			//there are spaces in the description, so let's wrap at the spaces
			CString strTemp;
			long nCount = 0;
			long nTotalLength = strTotalDescription.GetLength();
			while (nLength < (nTotalLength - nLine)) {

				//go the number of characters we have
				strTemp = strTotalDescription.Left(nUsableAmt);

				//see if by chance we are on a space
				if (strTotalDescription.GetAt(strTemp.GetLength() - 1) == ' ') {

					//excellent, this worked out well
					nCount = nUsableAmt;
				}
				else {

					if (strTemp.GetLength() < nUsableAmt) {
						nCount = strTemp.GetLength();
					}
					else {
						//we need to find the last space
						nCount = strTemp.ReverseFind(' ');
					}
				}

				if (nCount == -1) {
					if (strTemp.GetLength() > nUsableAmt) {
						nCount = nUsableAmt;
					}
					else {
						nCount = strTemp.GetLength();
					}
				}

				if (nLine == 1) {
					if(bBoldDesc) {
						strReturn = strBeginBold + strTotalDescription.Left(nCount) + strEndBold;
					}else {
						strReturn = strTotalDescription.Left(nCount);
					}
					
					strReturn += "   ";

					long nAmtToAdd = (nWidthOfPrinter - strReturn.GetLength() - nAmountLen - (nDateLen + 3));
					if (!strAmount.IsEmpty() && strAmount.GetAt(0) != '(') {
						nAmtToAdd--;
					}
					for (int i = 0; i < nAmtToAdd; i++) {
						strReturn += " ";
					}
					strReturn += strAmount + "\n";
					

				}
				else {

					for (int i = 0; i < nDateLen; i++) {
						strReturn += " ";
					}
					//add 3 spaces for the break
					strReturn += "   ";
					if(bBoldDesc) {
						strReturn += strBeginBold + strTotalDescription.Left(nCount) + strEndBold;
					}else {
						strReturn += strTotalDescription.Left(nCount);
					}
					strReturn += "\n";

				}

				//just take off the amount of the string that we added to strReturn
				strTotalDescription = strTotalDescription.Right(strTotalDescription.GetLength() - (nCount + 1));
				nLine++;
				nLength += nCount;
			}
		}

		strDescription = strReturn;
		return TRUE;
	}
	else {

		//I guess just print the line because we already gave an error
		// (a.walling 2011-06-09 14:16) - PLID 43459
		LogDetail("WARNING: Printing receipt description, nWidthOfPrinter invalid");
		ASSERT(FALSE);
		return FALSE;
	}
}

// (d.lange 2013-04-17 11:48) - PLID 55889 - This function does not bold the receipt description
BOOL GetReceiptDescription(COPOSPrinterDevice* pPOSPrinter, CString strDate, CString &strDescription, CString strQuantity, CString strAmount, BOOL bShowParenthesis, long nCharCount)
{
	return GetReceiptDescription(pPOSPrinter, strDate, strDescription, strQuantity, strAmount, bShowParenthesis, nCharCount, FALSE);
}
			
//TES 12/6/2007 - PLID 28192 - This now takes the POSPrinter as a parameter.
// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection in all recordsets in here
BOOL PrintReceiptDetail(COPOSPrinterDevice* pPOSPrinter, CString strLineItemFilter, CString strPaymentFilter, TotalsStruct *pTotals, CString &strCardHolderName, long nCharCount) {

	try {
		// (j.gruber 2009-04-01 16:35) - PLID PLID 33358 - updated discount structure
		// (j.jones 2009-11-11 12:42) - PLID 34165 - supported showing other applies
		ADODB::_RecordsetPtr rsCharges = CreateRecordset(GetRemoteDataSnapshot(), " SELECT LineItemT.ID, LineItemT.Type, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Amount, "
			" ChargesT.ItemCode, ChargesT.ItemSubCode, ChargesT.TaxRate, ChargesT.TaxRate2, COALESCE(TotalPercentOff, 0) as PercentOff, COALESCE(TotalDiscount, CONVERT(money, 0)), ChargesT.Quantity, "
			" ProvPersonT.First as ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle,  "
			" ProvPersonPrefixT.Prefix as ProvPrefix, "
			" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, "
			" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
			
			" Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChargeTotalNoDiscountsWithTax, "
			
			" Round(CONVERT(money, LineItemT.Amount * (CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChargeAmount,  "

								
			" (Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2)) - "
			" Round(Convert(money, ((((CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]* "
			" (CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*"
			" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*"
			" (CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*"
			" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)))) "
			" *CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			" - COALESCE([TotalDiscount], 0)), 2) AS DiscountAmt, "

			" Round(Convert(money, ((( "
			" /* Base Calculation */ "
			" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			" )* /* Discount 1 */ "
			" CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			" ) - /* Discount 2 */ "
			" CASE WHEN Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
			" )* /* Tax */ "
			" (ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1)  "
			" ), 2) AS TotalChargeWithDiscounts, "

			" Round(Convert(money, ((( "
			" /* Base Calculation */ "
			" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			" )* /* Discount 1 */ "
			" CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			" ) - /* Discount 2 */ "
			" CASE WHEN LineItemT.Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
			" )* /* Tax */ "
			"  (ChargesT.[TaxRate]-1)  "
			" ),2) AS Tax1Total,  "

			" Round(Convert(money, ((( "
			" /* Base Calculation */ "
			" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			" )* /* Discount 1 */ "
			" CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			" ) - /* Discount 2 */ "
			" CASE WHEN LineItemT.Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
			" )* /* Tax */ "
			"  (ChargesT.[TaxRate2]-1)  "
			" ),2) AS Tax2Total,  "

			" Round(Convert(money, ((( "
			" /* Base Calculation */ "
			" (CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*  "
			" (CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 END)* "
			" (CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)* "
			" (CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)* "
			" (CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END) "
			" )* /* Discount 1 */ "
			" CASE WHEN [TotalPercentOff] IS NULL THEN 1 ELSE (1.0 - (Convert(float,[TotalPercentOff])/100.0)) END "
			" ) - /* Discount 2 */ "
			" CASE WHEN Amount > 0 OR OthrBillFee = 0 THEN COALESCE([TotalDiscount],0) ELSE 0 END "
			" ) "
			" ), 2) AS TotalChargeWithDiscountsNoTax, "

			" Round(Convert(money,(CASE WHEN [LineItemT].[Amount] Is Null THEN 0 ELSE [LineItemT].[Amount] End)*[Quantity]*(CASE WHEN CPTMultiplier1 Is Null THEN 1 ELSE CPTMultiplier1 End)*(CASE WHEN CPTMultiplier2 Is NULL THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)),2) AS ChargeTotalNoDiscountsNoTax, "
			" 0 as CountTips, "
			" Coalesce(OtherPaymentAppliesQ.ApplyAmt, Convert(money,0)) AS OtherPaymentApplies "

			" FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			" LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			" LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
			" LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
			" LEFT JOIN PersonT ProvPersonT ON ProvidersT.PersonID = ProvPersonT.ID "
			" LEFT JOIN PrefixT ProvPersonPrefixT ON ProvPersonT.PrefixID = ProvPersonPrefixT.ID "
			" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
			" LEFT JOIN LocationsT ON LineItemT.LocationId = LocationsT.ID "
			" LEFT JOIN "
				"(SELECT AppliesT.DestID, Sum(AppliesT.Amount) AS ApplyAmt "
				"FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
				"WHERE Deleted = 0 AND SourceID NOT IN (%s -1) "
				"GROUP BY DestID "
				") OtherPaymentAppliesQ "
			" ON LineItemT.ID = OtherPaymentAppliesQ.DestID "
			" WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND BillsT.Deleted = 0 AND LineItemT.ID IN (%s -1) ",
			 strLineItemFilter, strLineItemFilter);
			
		ADODB::FieldsPtr fldsCh = rsCharges->Fields;

		CString strDetail;
		//long nLineWidth = pPOSPrinter->GetLineWidth();
		long nLineWidth = nCharCount;
		for (int h = 0; h < nLineWidth; h++ ) {
			strDetail += "_";
		}
		strDetail += "\n";

		//GetMainFrame()->m_pPOSPrinterDevice->PrintText("-----------------------------");
		
		while (!rsCharges->eof) {

			//print out the details
			CString strDate = FormatDateTimeForInterface(AdoFldDateTime(fldsCh, "Date"), NULL, dtoDate, TRUE);
			CString strDescription = AdoFldString(fldsCh, "Description", "");
			CString strChargeAmt = FormatCurrencyForInterface(AdoFldCurrency(fldsCh, "ChargeTotalNoDiscountsNoTax"), TRUE, TRUE);
			double dblQuantity = AdoFldDouble(fldsCh, "Quantity", 1);
			CString strQuantity = AsString(dblQuantity);
			COleCurrency cyDiscount = AdoFldCurrency(fldsCh, "DiscountAmt");
			//TES 11/6/2007 - PLID 27981 - VS2008 - COleCurrencys must be multiplied by longs (explicitly).
			CString strDiscountAmt = FormatCurrencyForInterface(cyDiscount * -1L, TRUE, TRUE);
			//CString strLineTotal = FormatCurrencyForInterface(AdoFldCurrency(fldsCh, "ChargeTotalNoDiscountsNoTax"), TRUE, TRUE);
			double dblTax1 = (AdoFldDouble(fldsCh, "TaxRate") - 1) * 100;
			double dblTax2 = (AdoFldDouble(fldsCh, "TaxRate2") - 1) * 100;

			strDetail += strDate + "   ";
			BOOL bSuccess = GetReceiptDescription(pPOSPrinter, strDate, strDescription, strQuantity, strChargeAmt, TRUE, nCharCount);
			if (bSuccess) {
				strDetail += strDescription;
			}
			else {
				return FALSE;						
			}
			
			if (cyDiscount > COleCurrency(0,0)) {

				CString strDateHolder;
				for (int i = 0; i < strDate.GetLength(); i++) {
					strDateHolder += " ";
				}
				strDetail += strDateHolder + "   ";
				CString strTmp = char(27);
				CString strDiscDesc = "Discount";
				// (d.lange 2013-04-17 11:48) - PLID 55889 - Added flag for bolding the description
				// Apply the bold format after formatting the detail description so the bold format doesn't get truncated
				bSuccess = GetReceiptDescription(pPOSPrinter, strDateHolder, strDiscDesc, "", strDiscountAmt, FALSE, nCharCount, TRUE);
				if (bSuccess) {
					strDetail += strDiscDesc;
				}
				else {
					return FALSE;
				}
				
			}

			pTotals->cyChargeTotal += AdoFldCurrency(fldsCh, "ChargeTotalNoDiscountsNoTax");

			// (j.gruber 2008-02-19 12:43) - PLID 28896 - need a total for charges with discounts due to rounding problem
			pTotals->cyChargeWithDiscountsTotal += AdoFldCurrency(fldsCh, "TotalChargeWithDiscounts");
			pTotals->cyDiscountTotal += AdoFldCurrency(fldsCh, "DiscountAmt");
			if (dblTax1 > pTotals->dblMaxTax1) {
				pTotals->dblMaxTax1 = dblTax1;
			}

			if (dblTax2 > pTotals->dblMaxTax2) {
				pTotals->dblMaxTax2 = dblTax2;
			}

			pTotals->cyTax1Total += AdoFldCurrency(fldsCh, "Tax1Total", COleCurrency(0,0));
			pTotals->cyTax2Total += AdoFldCurrency(fldsCh, "Tax2Total", COleCurrency(0,0));

			// (j.jones 2009-11-11 12:42) - PLID 34165 - supported showing other applies
			pTotals->cyOtherPaymentApplies += AdoFldCurrency(fldsCh, "OtherPaymentApplies", COleCurrency(0,0)) * -1L;
			
			if (!pPOSPrinter->PrintText(strDetail)) {
				return FALSE;
			}
			strDetail = "";
			rsCharges->MoveNext();
		}
		
		// (j.jones 2009-11-11 12:42) - PLID 34165 - supported showing other applies
		ADODB::_RecordsetPtr rsPayments = CreateRecordset(GetRemoteDataSnapshot(), "SELECT LineItemT.ID, LineItemT.Type, LineItemT.Description, LineItemT.Date, LineItemT.InputDate, LineItemT.InputName, LineItemT.Amount, "
			" ''As ItemCode, '' AS ItemSubCode, -1 as TaxRate, -1 as TaxRate2, -1 as PercentOff, convert(money, -1) as Discount, -1 as Quantity, "
			" PaymentsT.PrePayment, PaymentsT.PayMethod, PaymentPlansT.CheckNo, PaymentPlansT.BankNo, PaymentPlansT.CheckAcctNo, CreditCardNamesT.CardName as CCType ,  "
			" PaymentPlansT.CCNumber, PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.BankRoutingNum, "
			" PaymentsT.CashReceived, "
			" ProvPersonT.First as ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle,  "
			" ProvPersonPrefixT.Prefix as ProvPrefix, LocationsT.Name,  "
			" LocationsT.Address1, LocationsT.Address2, LocationsT.City, "
			" LocationsT.State, LocationsT.Zip, LocationsT.Phone, "
			" CoordPersonT.First as CoordFirst, CoordPersonT.Middle as CoordMiddle, CoordPersonT.Last as CoordLast, CoordPersonT.Title as CoordTitle,  "
			" NULL as BillDate, '' AS BillDescription,  "
			" PatientsT.UserDefinedID, PersonT.First, PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, "
			" PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
			" Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0), Convert(money, 0),   "
			" (SELECT Count(ID) FROM PaymentTipsT WHERE PaymentTipsT.PaymentID = LineItemT.ID) As CountTips, " 
			" Coalesce(OtherChargeAppliesQ.ApplyAmt, Convert(money,0)) AS OtherChargeApplies, "
			" Coalesce(OtherPaymentAppliesQ.ApplyAmt, Convert(money,0)) AS OtherPaymentApplies "

			" FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			" INNER JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
			" LEFT JOIN ProvidersT ON PaymentsT.ProviderID = ProvidersT.PersonID "
			" LEFT JOIN PersonT ProvPersonT ON ProvidersT.PersonID = ProvPersonT.ID "
			" LEFT JOIN PrefixT ProvPersonPrefixT ON ProvPersonT.PrefixID = ProvPersonPrefixT.ID "
			" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			" LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID "
			" LEFT JOIN LocationsT ON LineItemT.LocationId = LocationsT.ID "
			" LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
			" LEFT JOIN "
				//find other ChargesT records this payment is applied to
				"(SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS ApplyAmt, PatientID "
				"FROM AppliesT INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE Deleted = 0 AND ChargesT.ID NOT IN (%s -1) "						
				"GROUP BY SourceID, PatientID "
				") OtherChargeAppliesQ "
			" ON LineItemT.ID = OtherChargeAppliesQ.SourceID "
			" LEFT JOIN "
				"(SELECT PaymentID, Sum(ApplySubAmt) AS ApplyAmt, PatientID FROM "
					//find other PaymentsT records applied to this payment
					"(SELECT AppliesT.DestID AS PaymentID, Sum(AppliesT.Amount) AS ApplySubAmt, PatientID "
					"FROM AppliesT INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"WHERE Deleted = 0 AND PaymentsT.ID NOT IN (%s -1) "
					"GROUP BY DestID, PatientID "
					"UNION ALL "
					//find other PaymentsT records this payment is applied to
					"SELECT AppliesT.SourceID AS PaymentID, Sum(-AppliesT.Amount) AS ApplySubAmt, PatientID "
					"FROM AppliesT INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"WHERE Deleted = 0 AND PaymentsT.ID NOT IN (%s -1) "
					"GROUP BY SourceID, PatientID "
					") OtherPaymentAppliesSubQ "
					"GROUP BY PaymentID, PatientID "
				") OtherPaymentAppliesQ "
			" ON LineItemT.ID = OtherPaymentAppliesQ.PaymentID "
			" WHERE LineItemT.Type < 10 AND LineItemT.Deleted = 0 AND LineItemT.ID IN (%s -1) "
			, strLineItemFilter, strLineItemFilter, strLineItemFilter, strLineItemFilter);

		strDetail = "";
		ADODB::FieldsPtr flds = rsPayments->Fields;
		while (! rsPayments->eof) {

			CString strDate = FormatDateTimeForInterface(AdoFldDateTime(flds, "Date"), NULL, dtoDate, TRUE);
			CString strDescription = AdoFldString(flds, "Description", "");
			//TES 11/6/2007 - PLID 27981 - VS2008 - COleCurrencys must be multiplied by longs (explicitly).
			COleCurrency cyAmount = AdoFldCurrency(flds, "Amount") * -1L;
			CString strAmount = FormatCurrencyForInterface(cyAmount, TRUE, TRUE);
			long nPaymentID = AdoFldLong(flds, "ID");

			strCardHolderName = AdoFldString(flds, "CCHoldersName", "");
			
			
			long nType = AdoFldLong(flds, "Type");
			CString strDesc;
			switch (nType) {
				case 1:
					if (AdoFldBool(flds, "Prepayment")) {
						strDesc = "PREPAYMENT: " + strDescription;
					}
					else {
						strDesc = "PAYMENT: " + strDescription;
					}
				break;

				case 2:
					strDesc = "ADJUSTMENT: " + strDescription;
					pTotals->cyAdjTotal += cyAmount;
				break;

				case 3:
					strDesc = "REFUND: " + strDescription;
					pTotals->cyRefundTotal += cyAmount;
				break;
			}

			long nMethod = AdoFldLong(flds, "PayMethod");
			CString strExtraDesc;
			switch (nMethod) {
				case 1:
					//cash
					strExtraDesc = "Cash";
					pTotals->cyCashTotal += cyAmount;
				break;

				case 2:
					//check
					strExtraDesc = "Check Number: " + AdoFldString(flds, "CheckNo", "");
					pTotals->cyCheckTotal += cyAmount;
				break;

				case 3:
					//charge
					strExtraDesc = AdoFldString(flds, "CCType", "") + " - XXXXXXXXXXXX" + AdoFldString(flds, "CCNumber", "").Right(4);
					pTotals->cyCreditTotal += cyAmount;
				break;

				case 4:
					//gc
					strExtraDesc = "Gift Certificate";
					pTotals->cyGCTotal += cyAmount;
				break;
			}

			strDetail += strDate + "   "; 

			CString strAmtHolder;
			for (int i = 0; i < strAmount.GetLength(); i++) {
				strAmtHolder += " ";
			}
			BOOL bSuccess = GetReceiptDescription(pPOSPrinter, strDate, strDesc, "", strAmtHolder, FALSE, nCharCount);
			if (bSuccess) {
				strDetail += strDesc;
			}
			else {
				return FALSE;						
			}
			
			CString strDateHolder;
			for (i = 0; i < strDate.GetLength(); i++ ) {
				strDateHolder += " ";
			}
			//Print the date Holder
			strDetail += strDateHolder + "   ";
			bSuccess = GetReceiptDescription(pPOSPrinter, strDateHolder, strExtraDesc, "", strAmount, FALSE, nCharCount);
			if (bSuccess) {
				strDetail += strExtraDesc;
			}
			else {
				return FALSE;						
			}

			//now for the tips
			ADODB::_RecordsetPtr rsTips = CreateRecordset(GetRemoteDataSnapshot(), "SELECT PaymentID, Amount, PayMethod,  "
							" ProvT.First as ProvFirst, ProvT.Middle AS ProvMiddle, "
							" ProvT.Last AS ProvLast, ProvT.Title AS ProvTitle "
							" FROM "
							" PaymentTipsT LEFT JOIN ProvidersT ON  "
							" PaymentTipsT.ProvID = ProvidersT.PersonID "
							" LEFT JOIN PersonT ProvT ON ProvidersT.PersonID = ProvT.ID "
							" WHERE (PaymentID = %li) ", nPaymentID);
			
			if (!rsTips->eof) {
				strDetail += "\x1b|bCTip(s)\x1b|N:\n";
				if (!pPOSPrinter->PrintText(strDetail)) {
					return FALSE;
				}
				strDetail = "";
				
			}
			while (!rsTips->eof) {

				strDetail += strDateHolder + "   ";

				CString strProv = AdoFldString(rsTips, "ProvFirst", "") + " " + AdoFldString(rsTips, "ProvLast", "") + ", " + AdoFldString(rsTips, "ProvTitle", "");
				CString strMethod;
				long nPayMethod = AdoFldLong(rsTips, "PayMethod");
				switch (nPayMethod) {
					case 1:
						strMethod = "Cash";
					break;

					case 2:
						strMethod = "Check";
					break;

					case 3:
						strMethod = "Credit Card";
					break;
				}

				CString strTipAmt = FormatCurrencyForInterface(AdoFldCurrency(rsTips, "Amount"), TRUE, TRUE);

				bSuccess = GetReceiptDescription(pPOSPrinter, strDateHolder, strProv, strMethod, strTipAmt, FALSE, nCharCount);
				if (bSuccess) {
					strDetail += strProv;
				}

				pTotals->cyTipTotal += AdoFldCurrency(rsTips, "Amount");
				rsTips->MoveNext();
			}
			rsTips->Close();				

			//gift certificates don't fill in this field
			if (nType == 1 && (nMethod == 1 || nMethod == 2 || nMethod == 3)) {
				pTotals->cyAmountReceivedTotal += AdoFldCurrency(flds, "CashReceived", COleCurrency(0,0));
				COleCurrency cyChangeGiven = AdoFldCurrency(flds, "CashReceived", COleCurrency(0,0)) + cyAmount;
				pTotals->cyChangeGivenTotal += cyChangeGiven;
			}

			// (j.jones 2009-11-11 12:42) - PLID 34165 - supported showing other applies
			pTotals->cyOtherChargeApplies += AdoFldCurrency(flds, "OtherChargeApplies", COleCurrency(0,0));
			pTotals->cyOtherPaymentApplies += AdoFldCurrency(flds, "OtherPaymentApplies", COleCurrency(0,0)) * -1L;;

			rsPayments->MoveNext();
		}
		if (!pPOSPrinter->PrintText(strDetail)) {
			return FALSE;
		}
			
			
	return TRUE;
	}NxCatchAll("Error Printing Detail");
		return FALSE;

}


CString GetLargestString(CString str1, CString str2) {

	if (str1.GetLength() > str2.GetLength()) {
		return str1;
	}
	else {
		return str2;
	}
}

CString CalcLargestInfo(TotalsStruct *pTotals) {

	CString strMaxDesc;
	CString strMaxAmt;

	CString strTempDesc;
	CString strTempAmt;

	COleCurrency cyZero(0,0);

	if (pTotals->cyChargeTotal != cyZero) {
		strTempDesc = "Total Charges:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyChargeTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyDiscountTotal != cyZero) {
		strTempDesc = "Total Discounts:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyDiscountTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}

	
	if ((pTotals->cyChargeTotal - pTotals->cyDiscountTotal) != cyZero) {
		strTempDesc = "Charge SubTotal:";
		strTempAmt = FormatCurrencyForInterface((pTotals->cyChargeTotal - pTotals->cyDiscountTotal), TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}

	
	if (pTotals->cyTax1Total != cyZero) {
		strTempDesc.Format("Tax 1 Total(%.2f%%):", pTotals->dblMaxTax1);
		strTempAmt = FormatCurrencyForInterface(pTotals->cyTax1Total, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}

	
	if (pTotals->cyTax2Total != cyZero) {
		strTempDesc.Format("Tax 2 Total(%.2f%%):", pTotals->dblMaxTax2);
		strTempAmt = FormatCurrencyForInterface(pTotals->cyTax2Total, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	COleCurrency cyChargeTotal = (pTotals->cyChargeTotal - pTotals->cyDiscountTotal) + pTotals->cyTax1Total + pTotals->cyTax2Total;
	if (cyChargeTotal != cyZero) {
		strTempDesc = "Total:";
		strTempAmt = FormatCurrencyForInterface(cyChargeTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyCashTotal != cyZero) {
		strTempDesc = "Cash Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyCashTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyCheckTotal != cyZero) {
		strTempDesc = "Check Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyCheckTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyCreditTotal != cyZero) {
		strTempDesc = "Credit Card Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyCreditTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyGCTotal != cyZero) {
		strTempDesc = "Gift Certificate Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyGCTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}

	if (pTotals->cyAdjTotal != cyZero) {
		strTempDesc = "Adjustment Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyAdjTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyRefundTotal != cyZero) {
		strTempDesc = "Refund Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyRefundTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	/*if (pTotals->cyTipTotal != cyZero) {
		strTempDesc = "Tips Total:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyTipTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}*/


	COleCurrency cyCreditTotal = pTotals->cyAdjTotal + pTotals->cyRefundTotal + pTotals->cyCashTotal +
		pTotals->cyCheckTotal + pTotals->cyCreditTotal + pTotals->cyGCTotal;
	if (cyCreditTotal != cyZero) {
		strTempDesc = "Credits Total:";	
		strTempAmt = FormatCurrencyForInterface(cyCreditTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}

	//only show the amount received if there was change given
	if (pTotals->cyChangeGivenTotal != cyZero) {
		strTempDesc = "Amount Received:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyAmountReceivedTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	if (pTotals->cyChangeGivenTotal != cyZero) {
		strTempDesc = "Change Given:";
		strTempAmt = FormatCurrencyForInterface(pTotals->cyChangeGivenTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}


	//add since the credit total is negated
	if ((cyChargeTotal + cyCreditTotal) != cyZero) {
		strTempDesc = "Balance Due:";
		strTempAmt = FormatCurrencyForInterface(cyChargeTotal - cyCreditTotal, TRUE, TRUE);

		strMaxDesc = GetLargestString(strTempDesc, strMaxDesc);
		strMaxAmt = GetLargestString(strTempAmt, strMaxAmt);
	}

	return strMaxDesc + strMaxAmt;
	

}

//TES 12/6/2007 - PLID 28192 - This now takes the POSPrinter as a parameter.
BOOL GetSummaryDescription(COPOSPrinterDevice* pPOSPrinter, CString strWhite, CString &strDesc, CString strAmount, long nCharCount) {

	long nWhiteLen = strWhite.GetLength();
	long nDescLen = strDesc.GetLength();
	long nAmountLen = strAmount.GetLength();

	//long nWidthOfPrinter = pPOSPrinter->GetLineWidth();
	long nWidthOfPrinter = nCharCount;

	if (nWidthOfPrinter != -1) {

		long nTotalWidth;
		if (!strAmount.IsEmpty() && strAmount.GetAt(0) == '(') {
			nTotalWidth = nWhiteLen + nDescLen + nAmountLen;
		}
		else {
			nTotalWidth = nWhiteLen + nDescLen + nAmountLen + 1;
		}
		
		if (nTotalWidth <=  nWidthOfPrinter) {

			//we are good to go
			CString strReturn = strDesc;
			long nAmtToAdd = (nWidthOfPrinter - nTotalWidth);
			for (int i = 0; i < nAmtToAdd; i++) {
				strReturn += " ";
			}
			strReturn += strAmount + "\n";

			strDesc = strReturn;
			return TRUE;
		}
		else {
			
			//this shouldn't happen
			// (a.walling 2011-06-09 14:16) - PLID 43459
			LogDetail("WARNING: GetSummaryDescription nTotalWidth %li > nWidthOfPrinter %li", nTotalWidth, nWidthOfPrinter);
			ASSERT(FALSE);
			return TRUE;
		}
	}
	else {
		// (a.walling 2011-06-09 14:16) - PLID 43459
		LogDetail("WARNING: GetSummaryDescription nWidthOfPrinter invalid");
		ASSERT(FALSE);
		return TRUE;
	}
	
}

//TES 12/6/2007 - PLID 28192 - This now takes the POSPrinter as a parameter.
BOOL PrintReceiptPaymentSummary(COPOSPrinterDevice* pPOSPrinter, TotalsStruct *pTotals, CString strCardHoldersName, long nCharCount)  {

	CString strOutput;
	
	//long nLineWidth = pPOSPrinter->GetLineWidth();
	long nLineWidth = nCharCount;

	//first, we have to calculate the largest line of a total and the largest line of a description and then from there figure out where they all will lie
	CString strLargestLinePossible = CalcLargestInfo(pTotals);

	//now figure out where the description will begin
	// (a.walling 2011-06-09 14:16) - PLID 43459 - Why is this an assertion? I could easily make a line longer than the width. This doesn't take off whatever the
	// magic 6 constant is below, either.
	//ASSERT(strLargestLinePossible.GetLength() < nLineWidth);

	// (a.walling 2011-06-09 14:16) - PLID 43459 - If nWhiteSpace is greater than we can handle, or some other wierd number, just use 0.
	long nWhiteSpace = (nLineWidth - (strLargestLinePossible.GetLength()) - 6);
	if (nWhiteSpace < 0 || nWhiteSpace > nLineWidth) {
		nWhiteSpace = 0;
	}

	CString strWhite;
	for (int i = 0; i < nWhiteSpace; i++) {
		strWhite += " ";
	}
	
	strOutput += strWhite;
	for (i=0; i < (nLineWidth - nWhiteSpace); i++) {
		strOutput += "_";
	}
	strOutput += "\n";

	COleCurrency cyZero(0,0);
	CString strDesc;


	if (pTotals->cyChargeTotal != cyZero) {
		strDesc = "Charge Total:";
		strOutput += strWhite;
				
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyChargeTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	if (pTotals->cyDiscountTotal != cyZero) {
		strDesc = "Discount Total:";
		strOutput += strWhite;
		//TES 11/6/2007 - PLID 27981 - VS2008 - COleCurrencys must be multiplied by longs (explicitly).
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyDiscountTotal * -1L, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}


	/*Line*/
	strOutput += strWhite;
	for (i=0; i < (nLineWidth - nWhiteSpace); i++) {
		strOutput += "_";
	}
	strOutput += "\n";

	COleCurrency cyChargeSubTotal = pTotals->cyChargeTotal - pTotals->cyDiscountTotal;
	if (cyChargeSubTotal != cyZero) {
		strDesc = "Charge Subtotal:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(cyChargeSubTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	if (pTotals->cyTax1Total != cyZero) {
		strDesc.Format("Tax 1 Total (%.2f%%):", pTotals->dblMaxTax1);
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyTax1Total, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}


	if (pTotals->cyTax2Total != cyZero) {
		strDesc.Format("Tax 2 Total (%.2f%%):", pTotals->dblMaxTax2);
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyTax2Total, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	//Line
	strOutput += strWhite;
	for (i=0; i < (nLineWidth - nWhiteSpace); i++) {
		strOutput += "_";
	}
	strOutput += "\n";

	// (j.gruber 2008-02-19 12:45) - PLID 28896 - changed due to potential rounding error
	//COleCurrency cyChargeTotal = (pTotals->cyChargeTotal - pTotals->cyDiscountTotal) + pTotals->cyTax1Total + pTotals->cyTax2Total; 
	COleCurrency cyChargeTotal = pTotals->cyChargeWithDiscountsTotal; 
	if (cyChargeTotal != cyZero) {
		strDesc = "Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(cyChargeTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	if (pTotals->cyCashTotal!= cyZero) {
		strDesc = "Cash Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyCashTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}	


	if (pTotals->cyCheckTotal!= cyZero) {
		strDesc = "Check Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc , FormatCurrencyForInterface(pTotals->cyCheckTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	if (pTotals->cyCreditTotal != cyZero) {
		strDesc = "Credit Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyCreditTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	if (pTotals->cyGCTotal != cyZero) {
		strDesc = "Gift Certificate Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyGCTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	if (pTotals->cyAdjTotal != cyZero) {
		strDesc = "Adjustment Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyAdjTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}
	
	if (pTotals->cyRefundTotal != cyZero) {
		strDesc = "Refund Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyRefundTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	//Line
	strOutput += strWhite;
	for (i=0; i < (nLineWidth - nWhiteSpace); i++) {
		strOutput += "_";
	}

	
	COleCurrency cyCreditsTotal = pTotals->cyAdjTotal + pTotals->cyRefundTotal + pTotals->cyCashTotal +
		pTotals->cyCheckTotal + pTotals->cyCreditTotal + pTotals->cyGCTotal;
	if (cyCreditsTotal != cyZero) {
		strOutput += strWhite;
		strDesc = "Credits Total:";
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface(cyCreditsTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	/*Taken out - per meikin
	if (pTotals->cyTipTotal != cyZero) {
		strDesc = "Tips Total:";
		strOutput += strWhite;
		if (GetSummaryDescription(strWhite, strDesc, FormatCurrencyForInterface(pTotals->cyTipTotal, TRUE, TRUE))) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}*/

	if (pTotals->cyChangeGivenTotal != cyZero) {
		strDesc = "Amount Received:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc , FormatCurrencyForInterface(pTotals->cyAmountReceivedTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}
	

	if (pTotals->cyChangeGivenTotal != cyZero) {
		strDesc = "Change Given:";
		strOutput += strWhite;
		if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc , FormatCurrencyForInterface(pTotals->cyChangeGivenTotal, TRUE, TRUE), nCharCount)) {
			strOutput += strDesc;
		}
		else {
			return FALSE;
		}
	}

	// (j.jones 2009-11-11 12:42) - PLID 34165 - supported showing other applies
	if (pTotals->cyOtherChargeApplies != cyZero || pTotals->cyOtherPaymentApplies != cyZero) {

		//Line
		strOutput += strWhite;
		for (i=0; i < (nLineWidth - nWhiteSpace); i++) {
			strOutput += "_";
		}
		strOutput += "\n";

		if (pTotals->cyOtherChargeApplies != cyZero) {
			strDesc = "Other Applied Charges:";
			strOutput += strWhite;
			if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc , FormatCurrencyForInterface(pTotals->cyOtherChargeApplies, TRUE, TRUE), nCharCount)) {
				strOutput += strDesc;
			}
			else {
				return FALSE;
			}
		}

		if (pTotals->cyOtherPaymentApplies != cyZero) {
			strDesc = "Other Applied Credits:";
			strOutput += strWhite;
			if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc , FormatCurrencyForInterface(pTotals->cyOtherPaymentApplies, TRUE, TRUE), nCharCount)) {
				strOutput += strDesc;
			}
			else {
				return FALSE;
			}
		}
	}

	//Line
	strOutput += strWhite;
	for (i=0; i < (nLineWidth - nWhiteSpace); i++) {
		strOutput += "_";
	}
	strOutput += "\n";

	CString strTmp = char(27);
	strDesc = "\x1b|bCBalance Due\x1b|N:";
	strOutput += strWhite;
	// (j.jones 2009-11-11 12:42) - PLID 34165 - the balance must include other applies
	if (GetSummaryDescription(pPOSPrinter, strWhite, strDesc, FormatCurrencyForInterface((cyChargeTotal + cyCreditsTotal + pTotals->cyOtherChargeApplies + pTotals->cyOtherPaymentApplies), TRUE, TRUE), nCharCount)) {
		//strOutput += char(27) + "|bC";
		strOutput += strDesc;
	}
	else {
		return FALSE;
	}


	if (!pPOSPrinter->PrintText(strOutput)) {
		return FALSE;
	}

	return TRUE;
}



// (j.gruber 2008-01-16 17:21) - PLID 28310 Changed function to support formatting
BOOL PrintReceiptFooter(COPOSPrinterDevice* pPOSPrinter, CString strIDs, CString strFooterMessage, long nFontNumber, long nCharCount)  {
	
	try {

		if (pPOSPrinter) {

			CStringArray arystrHeader;
			ReplaceFormattedText(pPOSPrinter, strIDs, strFooterMessage, &arystrHeader, nFontNumber, nCharCount);

			CString strFont;
			strFont.Format("<ESC>|%lifT", nFontNumber);
			CString strEsc = char(27);
			strFont.Replace("<ESC>"	, strEsc);
			for (int i = 0; i < arystrHeader.GetSize(); i++) {
				CString strText = strFont + arystrHeader.GetAt(i);
				strText.Replace("\n", "");

				// (a.walling 2011-04-27 10:08) - PLID 43459 - Reset the font when we are done
				// (a.walling 2011-06-10 16:43) - PLID 43459 - Do not reset; some printers seem to die.
				/*if (i == arystrHeader.GetSize() - 1) {
					strText += "\x1b|0fT";
				}*/

				strText += "\n";

				if (!pPOSPrinter->PrintText(strText)) {
					return FALSE;
				}
			}

			return TRUE;
				
		}
		return FALSE;
	}NxCatchAll("Error in PrintReceiptFooter");

	return FALSE;

}

// (s.dhole 2012-06-21 12:35) - PLID 49341 - this function will check Optical Shop reports
bool IsReportOpticalShopOnly(IN const CReportInfo *pReport)
{
	switch (pReport->nID) {
			case 704: //Glasses Order
			case 726: //Frames Sold by Manufacturer
			case 729:// Contact Lens Order
			case 730: //Optical Product Sales  By Optician
			case 731: //Optical Sales by supplier/Lab
			case 732: //Optical Orders - By Status
			case 733: //Items Sold Off The shelf
			case 734: //Contact Lens sold - By Manufacturer
			case 735: //Optical Order Cost/Profit Analysis By Provider
			case 736:// Optical Orders Capture Rate
			case 737:// Optical Prescription
				
			return true;
		break;
		default:
			return false;
		break;
	}
	
}
// (s.tullis 2014-07-14 13:34) - PLID 62559 - Batch Medical Payments
// (s.tullis 2014-07-10 13:47) - PLID 62560- Batch Vision Payments
// need to show/ hide these reports based on whether the client has vision posting license
bool IsReportVisionPostingOnly(IN const CReportInfo *pReport){

	switch (pReport->nID){
	case 755:
	case 756:
		return true;
			break;
	default:
			return false;
		break;
	}
}

// (j.jones 2015-04-27 09:13) - PLID 65388 - added shared query for calculating a gift certificates total value and balance
// (r.gonet 2015-04-29 15:17) - PLID 65657 - Reworked how refunds are factored into the overall
// numbers. We need to prevent refunds of payments by Certificate A from crediting the same Certificate A,
// otherwise we would be double counting and get incorrect numbers.
//***IF THIS QUERY CHANGES, ALSO CHANGE GiftCertificates_BaseFromClause() IN THE API***//
CString GetGiftCertificateValueQuery()
{
	return R"(
SELECT GiftCertificatesT.ID, 
IsNull(GCTotalValueQ.TotalValue, 0) + IsNull(GCRefundsQ.TotalRefunds, 0) - IsNull(GCRefundsToSelfQ.TotalRefunds, 0) AS TotalValue,
IsNull(GCTotalUsedQ.AmtUsed, 0) - IsNull(GCRefundsToSelfQ.TotalRefunds, 0) AS AmtUsed,
IsNull(GCTotalValueQ.TotalValue, 0) + IsNull(GCRefundsQ.TotalRefunds, 0) - IsNull(GCTotalUsedQ.AmtUsed, 0) AS Balance 
FROM GiftCertificatesT 

--calculate the Total Value as the Value credited from all charges,
--plus all refunds, plus all transfers in
LEFT JOIN (
    SELECT GiftCertificatesT.ID,
        IsNull(GCTotalChargedValueQ.TotalValue, 0)
            + IsNull(TransfersInQ.TotalTransfers, 0)
        AS TotalValue 
    FROM GiftCertificatesT
    -- charges
    LEFT JOIN (
	    SELECT LineItemT.GiftID, Sum(LineItemT.GCValue) AS TotalValue
	    FROM LineItemT 
        INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID
        LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID 
        LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID
	    WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10
	    AND LineItemT.GiftID Is Not Null
        AND OrigLineItemsT.OriginalLineItemID IS NULL
        AND VoidingLineItemsT.VoidingLineItemID IS NULL
	    GROUP BY LineItemT.GiftID 
    ) AS GCTotalChargedValueQ ON GiftCertificatesT.ID = GCTotalChargedValueQ.GiftID
    -- transfers in
    LEFT JOIN (
        SELECT DestGiftID, Sum(Amount) AS TotalTransfers
        FROM GiftCertificateTransfersT
        GROUP BY DestGiftID
    ) AS TransfersInQ ON GiftCertificatesT.ID = TransfersInQ.DestGiftID
) AS GCTotalValueQ ON GiftCertificatesT.ID = GCTotalValueQ.ID

-- refunds crediting the gift certificates
LEFT JOIN (
    SELECT GiftCertificatesT.ID,
        IsNull(GCAmtRefundedQ.TotalAmtRefunded, 0) AS TotalRefunds 
    FROM GiftCertificatesT
    LEFT JOIN (
        SELECT GiftID, Sum(Amount) AS TotalAmtRefunded
	    FROM (
		    SELECT LineItemT.GiftID, -1 * LineItemT.Amount AS Amount
		    FROM LineItemT 		
		    INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
            LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
            LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
            WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 3
		    AND LineItemT.GiftID IS NOT NULL
            AND OrigLineItemsT.OriginalLineItemID IS NULL
            AND VoidingLineItemsT.VoidingLineItemID IS NULL
		    UNION ALL 
		    SELECT LineItemT.GiftID, -1 * PaymentTipsT.Amount AS Amount
		    FROM LineItemT 
		    INNER JOIN PaymentTipsT ON LineItemT.ID = PaymentTipsT.PaymentID 
            LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
            LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
		    WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 3 
		    AND LineItemT.GiftID Is Not NULL 
		    AND PaymentTipsT.PayMethod = 10 
	        AND OrigLineItemsT.OriginalLineItemID IS NULL
	        AND VoidingLineItemsT.VoidingLineItemID IS NULL
	    ) AS GCAllRefundsQ
	    GROUP BY GiftID 
    ) AS GCAmtRefundedQ ON GiftCertificatesT.ID = GCAmtRefundedQ.GiftID
) AS GCRefundsQ ON GiftCertificatesT.ID = GCRefundsQ.ID

-- refunds crediting the gift certificates but from the same gift certificate (We do not want to double count them.)
LEFT JOIN (
    SELECT GiftCertificatesT.ID,
        IsNull(GCAmtRefundedToSelfQ.TotalAmtRefunded, 0) AS TotalRefunds
    FROM GiftCertificatesT
    -- refunds
    LEFT JOIN (
        SELECT GiftID, Sum(Amount) AS TotalAmtRefunded
	    FROM (
		    SELECT LineItemT.GiftID, -1 * LineItemT.Amount AS Amount
		    FROM LineItemT 		
		    INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
            LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
            LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
            WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 3
		    AND LineItemT.GiftID IS NOT NULL AND PaymentsT.RefundedFromGiftID IS NOT NULL
			AND LineItemT.GiftID = PaymentsT.RefundedFromGiftID
            AND OrigLineItemsT.OriginalLineItemID IS NULL
            AND VoidingLineItemsT.VoidingLineItemID IS NULL
			UNION ALL 
			SELECT LineItemT.GiftID, -1 * PaymentTipsT.Amount AS Amount
		    FROM LineItemT 
			INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID
		    INNER JOIN PaymentTipsT ON LineItemT.ID = PaymentTipsT.PaymentID 
            LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
            LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
		    WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 3 
		    AND LineItemT.GiftID Is Not NULL 
		    AND PaymentTipsT.PayMethod = 10 
			AND LineItemT.GiftID IS NOT NULL AND PaymentsT.RefundedFromGiftID IS NOT NULL
			AND LineItemT.GiftID = PaymentsT.RefundedFromGiftID
	        AND OrigLineItemsT.OriginalLineItemID IS NULL
	        AND VoidingLineItemsT.VoidingLineItemID IS NULL
	    ) AS GCAllRefundsToSelfQ
	    GROUP BY GiftID
    ) AS GCAmtRefundedToSelfQ ON GiftCertificatesT.ID = GCAmtRefundedToSelfQ.GiftID
) AS GCRefundsToSelfQ ON GiftCertificatesT.ID = GCRefundsToSelfQ.ID

--calculate the Amount Used as the total amount of all payments,
--plus all transfers out
LEFT JOIN (
    SELECT GiftCertificatesT.ID,
        IsNull(GCAmtSpentQ.TotalAmtSpent, 0)
            + IsNull(TransfersOutQ.TotalTransfers, 0)
        AS AmtUsed
    FROM GiftCertificatesT
    -- payments
	LEFT JOIN (
        SELECT GiftID, Sum(Amount) AS TotalAmtSpent 
	    FROM (
		    SELECT LineItemT.GiftID, LineItemT.Amount 
		    FROM LineItemT 		
            LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
            LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
            WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1
		    AND LineItemT.GiftID IS NOT NULL
            AND OrigLineItemsT.OriginalLineItemID IS NULL
            AND VoidingLineItemsT.VoidingLineItemID IS NULL
		    UNION ALL 
		    SELECT LineItemT.GiftID, PaymentTipsT.Amount 
		    FROM LineItemT 
		    INNER JOIN PaymentTipsT ON LineItemT.ID = PaymentTipsT.PaymentID 
            LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON LineItemT.ID = OrigLineItemsT.OriginalLineItemID 
            LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID
		    WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 
		    AND LineItemT.GiftID Is Not NULL 
		    AND PaymentTipsT.PayMethod = 4 
	        AND OrigLineItemsT.OriginalLineItemID IS NULL
	        AND VoidingLineItemsT.VoidingLineItemID IS NULL
	    ) AS GCAllPaysQ
	    GROUP BY GiftID 
    ) AS GCAmtSpentQ ON GiftCertificatesT.ID = GCAmtSpentQ.GiftID
    -- transfers out
    LEFT JOIN (
        SELECT SourceGiftID, Sum(Amount) AS TotalTransfers
        FROM GiftCertificateTransfersT
        GROUP BY SourceGiftID
    ) AS TransfersOutQ ON GiftCertificatesT.ID = TransfersOutQ.SourceGiftID
) AS GCTotalUsedQ ON GiftCertificatesT.ID = GCTotalUsedQ.ID
)";
}