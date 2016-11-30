#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "WhereClause.h"
#include "NxReportJob.h"
#include "crpe.h"
#include "peplus.h"
#include "MsgBox.h"
#include "ExternalForm.h"
#include "GlobalUtils.h"
#include "letterwriting.h"
#include "InternationalUtils.h"
#include "GetNewIDName.h"
#include "RegUtils.h"
#include "ReportsRc.h"
#include "AuditTrail.h"
#include "ReportsRc.h"
#include "NxTaskDialog.h"
#include "GlobalFinancialUtils.h"
#include "ReportAdo.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 11/7/2007 - PLID 27979 - VS2008 -  FindText is already a macro, and we don't use the imported FindText anyway.
#import "craxdrt.tlb" exclude("IFontDisp") rename("FindText","CRAXDRT_FindText")


using namespace ADODB;

// TODO: Use the actual SQL limits here
//DRT 8/24/01 - These are not being set until after the CReportInfo constructor (which uses them) is called
//const COleDateTime dtMin(1700, 1, 1, 0, 0, 0);
//const COleDateTime dtMax(2030, 12, 31, 0, 0, 0);
//const COleDateTime dtToday = COleDateTime::GetCurrentTime();


//EBuiltInObjectIDs GetReportBuiltInObjectFromString(const CString &strCategory);

// (c.haag 2015-02-23) - PLID 63751 - We now inherit from a class that maintains the business logic for reports used in other projects
CReportInfo::CReportInfo(long nID, LPCTSTR strPrintName, long nRepIDForRepGroup, LPCTSTR strReportName, LPCTSTR strRecordSource, LPCTSTR strCategory, LPCTSTR strReportFile, BOOL bCreateGroup, short nDetail, long nProvider, long nLocation, long nPatient, 
			short	nDateRange, short nDateFilter, BOOL bOneDate, LPCTSTR strDateCaption, BOOL bExternal, LPCTSTR strReportSpecificName, LPCTSTR strListBoxFormat, 
			LPCTSTR strListBoxWidths, LPCTSTR strListBoxSQL, LPCTSTR strFilterField, BOOL bExtended, LPCTSTR strExtraText, LPCTSTR strExtraField, LPCTSTR strExtraValue, BOOL bUseGroupOption, 
			short nExtendedStyle, LPCTSTR strExtendedSql, /*BOOL bPhaseOut /*= FALSE,*/ BOOL bEditable, long nVersion, CString strDateOptions, long nOneDateOption, long nSupportedSubfilter /*= -1*/, BOOL bAllowMultipleExtended /*= FALSE*/)
			: CSharedReportInfo(nID, strReportFile, nVersion)
{
	// If there is an assertion failure on this line, it is an indication
	// that the id used in this ADD_REPORT statement is out of range
	// It must be greater than or equal to 0 and less than 
	// REPORT_NEXT_INFO_ID; this is just an aid to help ensure the 
	// appropriate use of info ids
	//TES 6/29/2007 - PLID 26510 - Actually, 0 is not a valid ID, because the array is initialized to 0s, so an entry
	// with an ID of 0 would always get flagged by the next ASSERT as a duplicate.  And besides, having an ID of 0 seems
	// like a bad idea no matter what.
	ASSERT((nID > 0 && nID < REPORT_NEXT_INFO_ID));

	// If there is an assertion failure on this line, it is an indication 
	// that the id used in this ADD_REPORT statement is already in 
	// use by a different one
	// (d.thompson 2015-10-19 10:12) - PLID 67403 - We cannot safely iterate through the array of known reports -- we're in the process of 
	//	creating that array as we speak!
	//ASSERT(GetInfoIndex(nID) == -1);

	// (b.cardillo 2006-04-07 10:37) - Here we assert that only, AND ALL, reports with id's 
	// greater than 563 are given a companion report for report-grouping purposes.  See the 
	// EnsureReportGroups() function for more information.
	ASSERT(nRepIDForRepGroup == -1 && nID <= 563 ||
		   nRepIDForRepGroup != -1 && nID > 563);
	// We also assert that the companion report already exists at the point in time that this 
	// report is being added because report group defaults are done in the order of entries in 
	// the static array (see the for-statement in EnsureReportGroups()), so if a report 
	// referenced a companion that's later in the array, the given report wouldn't exist at the 
	// time its defaults are being set and so the defaulting would fail.
	ASSERT(nRepIDForRepGroup == -1 || GetInfoIndex(nRepIDForRepGroup) != -1);
	

	this->nID = nID;
	this->strPrintName = strPrintName;
	this->nRepIDForRepGroup = nRepIDForRepGroup;
	this->strReportName = strReportName;
	this->strRecordSource = strRecordSource;
	this->strCategory = strCategory;
	this->strReportFile = strReportFile;
	this->bCreateGroup = bCreateGroup;
	this->nDetail = nDetail;
	this->nProvider = nProvider;
	this->nLocation = nLocation;
	this->nPatient = nPatient;
	this->nDateRange = nDateRange;
	this->nDateFilter = nDateFilter;
	this->bOneDate = bOneDate;
	this->DateFrom = nDateRange ? COleDateTime::GetCurrentTime() : COleDateTime::COleDateTime(1700, 1, 1, 0, 0, 0);
	this->DateTo = nDateRange ? COleDateTime::GetCurrentTime() : COleDateTime::COleDateTime(2030, 12, 31, 0, 0, 0);
	this->strDateCaption = strDateCaption;
	this->bExternal = bExternal;
	this->strReportSpecificName = strReportSpecificName;
	this->strListBoxFormat = strListBoxFormat;
	this->strListBoxWidths = strListBoxWidths;
	this->strListBoxSQL = strListBoxSQL;
	this->strFilterField = strFilterField;
	this->bExtended = bExtended;
	this->strExtraText = strExtraText;
	this->strExtraField = strExtraField;
	this->SetExtraValue(strExtraValue);
	this->bUseGroupOption = bUseGroupOption;
	this->bUseGroup = FALSE;
	this->nGroup = -9999;
	this->nExtendedStyle = nExtendedStyle;
	this->strExtendedSql = strExtendedSql;
//	this->bPhaseOut = bPhaseOut;
	this->nExtraID = -1;
	this->nFilterID = -1;
	this->bUseFilter = FALSE;
	this->bUseGroup = FALSE;
	this->nDefaultCustomReport = -1;
	this->bEditable = bEditable;
	this->nVersion = nVersion;
	this->strDateOptions = strDateOptions;
	this->nOneDateOption = nOneDateOption;
	this->nSupportedSubfilter = nSupportedSubfilter;
//	this->m_bioBuiltInObjectID = GetReportBuiltInObjectFromString(this->strCategory);
	this->bAllowMultipleExtended = bAllowMultipleExtended;

	// (j.gruber 2008-07-14 15:11) - PLID 28976 - added AllYear flag
	this->bUseAllYears = FALSE;
	this->ClearParameterList();

	// (r.gonet 12/18/2012) - PLID 53629 - We must save the output types for statement
	//  reports at the time of the report being run so we can properly pass them along
	//  to WriteToHistory. This prevents races where the properties change while the
	//  the report is running.
	this->nStatementType = -1;
	this->nOutputType = -1;
	this->nCategoryType = -1; // (r.goldschmidt 2014-08-05 14:06) - PLID 62717
}

CReportInfo::CReportInfo()
{
	this->nID = -1;
	this->strPrintName = _T("");
	this->nRepIDForRepGroup = -1;
	this->strReportName = _T("");
	this->strRecordSource = _T("");
	this->strCategory = _T("");
	this->strReportFile = _T("");
	this->bCreateGroup = FALSE;
	this->nDetail = 0;
	this->nProvider = 0;
	this->nLocation = 0;
	this->nPatient = 0;
	this->nDateRange = 0;
	this->nDateFilter = 0;
	this->bOneDate = FALSE;
	this->DateFrom = COleDateTime::COleDateTime(1700, 1, 1, 0, 0, 0);
	this->DateTo = COleDateTime::COleDateTime(2030, 12, 31, 0, 0, 0);
	this->strDateCaption = _T("");
	this->bExternal = FALSE;
	this->strReportSpecificName = _T("");
	this->strListBoxFormat = _T("");
	this->strListBoxWidths = _T("");
	this->strListBoxSQL = _T("");
	this->strFilterField = _T("");
	this->bExtended = FALSE;
	this->strExtraText = _T("");
	this->strExtraField = _T("");
	this->SetExtraValue(_T(""));
	this->bUseGroupOption = FALSE;
	this->bUseGroup = FALSE;
	this->nGroup = -9999;
	this->nExtendedStyle = 0;
	this->strExtendedSql = _T("");
	//this->bPhaseOut = FALSE;
	this->nExtraID = -1;
	this->bUseFilter = FALSE;
	this->nFilterID = -1;
	this->nDefaultCustomReport = -1;
	this->bEditable = FALSE;
	this->nVersion = 0;
	this->strDateOptions = "";
	this->nOneDateOption = -1;
	this->nSupportedSubfilter = -1;
	//this->m_bioBuiltInObjectID = GetReportBuiltInObjectFromString(this->strCategory);
	this->bAllowMultipleExtended = TRUE;
		
	// (j.gruber 2008-07-14 15:11) - PLID 28976 - added AllYear flag
	this->bUseAllYears = FALSE;

	this->ClearParameterList();

	// (r.gonet 12/18/2012) - PLID 53629 - We must save the output types for statement
	//  reports at the time of the report being run so we can properly pass them along
	//  to WriteToHistory. This prevents races where the properties change while the
	//  the report is running.
	this->nStatementType = -1;
	this->nOutputType = -1;
	this->nCategoryType = -1; // (r.goldschmidt 2014-08-05 14:06) - PLID 62717
}

// (a.walling 2013-08-30 09:01) - PLID 57998 - Let the compiler create our copy constructor and assignment operator

CReportInfo::~CReportInfo()
{
	ClearParameterList();

#ifdef _DEBUG
	FreeP2smonDlls();
#endif
}

/*
EBuiltInObjectIDs GetReportBuiltInObjectFromString(const CString &strCategory)
{
	bioReportsFinancialHigh;
	
	if (strCategory == "PatientP") return bioReportsPatientTab;
	else if (strCategory == "OthrContactP") return bioReportsContactTab;
	else if (strCategory == "ScheduleP") return bioReportsModule;//return bioReportsSchedulerTab;
	else if (strCategory == "InventoryP") return bioReportsInventoryTab;
	else if (strCategory == "MarketP") return bioReportsMarketingTab;
	else if (strCategory == "ChargesP") return bioReportsFinancialLow;//return bioReportsChargesTab;
	else if (strCategory == "PaymentsP") return bioReportsFinancialLow;//return bioReportsPaymentsTab;
	else if (strCategory == "FinancialP") return bioReportsFinancialLow;
	else if (strCategory == "ASCP") return bioReportsASCTab;
	else if (strCategory == "AdminP") return bioReportsAdminTab;
	else if (strCategory == "OtherP") return bioReportsModule;//return bioReportsOtherTab;
	else if (strCategory == "") return bioReportsModule;//return bioReportsUncategorized;
	else {
		ASSERT(FALSE);
		return bioInvalidID;
	}
}*/

long CReportInfo::GetInfoIndex(long nID)
{
	// Search through all the fields to find
	for (long i=0; i<CReports::gcs_nKnownReportCount; i++) {
		if (CReports::gcs_aryKnownReports[i].nID == nID) {
			return i;
		}
	}
	
	// Failed
	return -1;
}

// (a.walling 2013-05-08 16:00) - PLID 50104 - Better error message UI when failing to open a report
namespace {

class ReportJobErrorDialog
	: public NxTaskDialog
	, private boost::noncopyable
{
public:
	ReportJobErrorDialog(CWnd* pParent, short nErrorCode, const CString& strErrorText, const CString& strPathName, const CString& strReportTitle)
		: NxTaskDialog(pParent)
		, m_nErrorCode(nErrorCode)
		, m_strErrorText(strErrorText)
		, m_strPathName(strPathName)
		, m_strReportTitle(strReportTitle)
	{		
		m_strDetails.Format("Error %li opening report file %s: %s", nErrorCode, strPathName, strErrorText);

		Log(m_strDetails);
	}

	CString m_strDetails;

	short m_nErrorCode;
	CString m_strErrorText;
	CString m_strPathName;
	CString m_strReportTitle;

	enum Commands
	{
		eRetry = 0x100,
		eBack,
		eBrowse,
		eContact,
		eContactNotify,
		eContactEmail,
		eContactWebsite,
	};

	virtual BOOL OnButtonClicked(int nButton) override
	{
		// return TRUE to prevent close, FALSE to close
		switch (nButton)
		{
			case eContact:
				ConfigPushNew()
					.ErrorIcon().CancelOnly()
					.MainInstructionText("Contact NexTech technical support")
					.ContentText("East Coast and Midwest states, call (866) 654-4396\nWest Coast and Central states, call (888) 417-8464")
					.AddCommand(eContactNotify, "Send error report\nRelevant error information will be automatically included and submitted with your notification")
					// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
					.AddCommand(eContactEmail, "Email technical support\nOpens your default email application to send an email to allsupport@nextech.com")
					.AddCommand(eContactWebsite, "More contact options\nOpens the NexTech website for further contact options and information")
					.AddCommand(eBack, "Back")
				;

				NavigatePage();
				return TRUE;
			case ReportJobErrorDialog::eContactNotify:
				// (a.walling 2010-06-28 18:04) - PLID 38442 - SendErrorToNexTech
				SendErrorToNexTech(m_pTaskDialogParent, "NexTech Report Job Error", "CReportInfo", m_strDetails);
				return TRUE;
			case ReportJobErrorDialog::eContactEmail:
				SendErrorEmail(m_pTaskDialogParent, m_strDetails, "NexTech Report Job Error", false);
				return TRUE;
			case ReportJobErrorDialog::eContactWebsite:
				ShellExecute(m_pTaskDialogParent->GetSafeHwnd(), NULL, "http://www.nextech.com/Contact/Support.aspx", NULL, NULL, SW_SHOW);
				return TRUE;
			///
			case eBrowse:
				{
					CString strPath = FileUtils::GetFilePath(m_strPathName);
					int error = (int)ShellExecute(m_pTaskDialogParent->GetSafeHwnd(), "explore", strPath, NULL, NULL, SW_SHOW);
					if (error <= 32) {
						MessageBox(FormatString("Failed to open the path `%s` with error code %li", strPath, error), NULL, MB_ICONERROR);
					}
				}
				return TRUE;
			///
			case eBack:
				ConfigPop();
				NavigatePage();
				return TRUE;
		}

		return NxTaskDialog::OnButtonClicked(nButton);
	}
};

}

// (a.walling 2013-05-08 16:00) - PLID 50104 - Better error message UI when failing to open a report
int HandleReportJobError(CWnd* pParent, short nErrorCode, const CString& strErrorText, const CString& strPathName, const CString& strReportTitle)
{
	if (nErrorCode == PE_ERR_CANNOTACCESSREPORT /*544*/) {
		AfxMessageBox("You currently have 20 instances of this report open.  Please close these reports before continuing.\nYou can see which reports are currently open by going to the Window menu at the top of NexTech Practice.");
		return IDCANCEL;
	}

	ReportJobErrorDialog dlg(pParent, nErrorCode, strErrorText, strPathName, strReportTitle);


	dlg.Config()
		.ErrorIcon().CancelOnly()
		.MainInstructionText(FormatString("Unable to open the %s report.", strReportTitle))
		.ContentText(FormatString(
			"The report file %s is not accessible. "
			"You may have lost connection to the server, or your network credentials may be expired or invalid. "
			"If this problem continues, please refer to your network administrator "
			"or call NexTech technical support for assistance.", strPathName)
		)
		.ExpandedInformationText(dlg.m_strDetails)
		.AddCommand(ReportJobErrorDialog::eRetry, "Retry\nAttempt to open the report again.")
		.AddCommand(ReportJobErrorDialog::eBrowse, "Browse\nTry to browse to the path in Windows Explorer.")
		.AddCommand(ReportJobErrorDialog::eContact, "Contact NexTech technical support\nWe can answer your questions and help resolve your issues.")
	;
	
	int ret = dlg.DoModal();
	switch (ret) {			
		case ReportJobErrorDialog::eRetry:
			return IDRETRY;
	}

	return IDCANCEL;
}

// Returns TRUE if the report was output
// (a.walling 2013-08-30 09:01) - PLID 57998 - Handle map of params as well
BOOL CReportInfo::ViewReport(const CString &strTitle, const CString &strFile, const std::map<CString, CString>& paramList, BOOL bPreview, CWnd *pParentWnd, CPrintInfo* pInfo /* = 0 */) const 
{
	// uses MDI interface instead of dialog

	CRPEParameterFieldInfo tmpParamInfo;
	CNxReportJob *RepJob;
	CRPEngine *Engine = CRPEngine::GetEngine();
	CRPEJobInfo JobInfo;
	int nParams, i;

	tmpParamInfo.m_StructSize = sizeof(CRPEParameterFieldInfo);

	// We should definitely have an Engine already
	if (!Engine) Engine = GetMainFrame()->GetReportEngine();

	ASSERT(Engine);
	if (Engine && Engine->Open()) {
		try {
			// set up report
			
			//check to see if it is a custom report
			CString strPathName;
			if (nDefaultCustomReport > 0) {
				strPathName = GetCustomReportsPath() ^ strFile;
				//make sure that the report exists

				// (a.walling 2013-05-08 16:00) - PLID 50104 - Better error message UI when failing to open a report, with retry!
				for (;;) {
					if (DoesExist(strPathName)) {
						break;
					}
					if (IDRETRY != HandleReportJobError(pParentWnd, 2, "Custom report file not found", strPathName, strTitle)) {
						return FALSE;
					}
				}

				//check to see that the version is the correct version
				_RecordsetPtr rsVersion = CreateRecordset("SELECT Version FROM CustomReportsT WHERE ID = %li AND Number = %li", this->nID, this->nDefaultCustomReport);
				if (! rsVersion->eof) {
					long nVersion = AdoFldLong(rsVersion, "Version");
					if (nVersion != this->nVersion) {

						//the versions don't match
						if (IDYES == MsgBox(MB_YESNO | MB_ICONQUESTION, "The version of this report does not match the version of the query.  You must verify this report before it can be run; if you do not verify, the report will not be run.  Would you like to update and verify the report automatically?")) {
							// (a.walling 2007-06-19 13:52) - PLID 19405 - Check and auto verify the report
							BOOL bVerified = VerifyCustomReport();

							if (!bVerified) {
								MsgBox("The report could not be automatically verified; you must verify this report before it can be run.  Please open the report editor and click the verify button.");
								return FALSE;
							}

							// otherwise, we can continue!
						} else {
							// user chose not to verify automatically
							return FALSE;
						}
					}
				}
				else {
					// data issue, no friendly messagebox here. Though might be better as an exception. Don't recall ever running into it though.
					MsgBox("Practice could not find the custom report");
					return FALSE;
				}
				
			}
			else {
				strPathName = GetReportsPath() ^ strFile + ".rpt";
			}
			// (a.walling 2013-05-08 16:00) - PLID 50104 - Better error message UI when failing to open a report, with retry!
			do {
				RepJob = (CNxReportJob *)Engine->OpenJob(strPathName);
			} while (!RepJob && IDRETRY == HandleReportJobError(pParentWnd, Engine->GetErrorCode(), Engine->GetErrorText(), strPathName, strTitle));

			if (!RepJob) {
				return FALSE;
			}
			// set up the Job
			RepJob->EnableProgressDialog(TRUE);
			RepJob->ShowPrintControls(TRUE);

		} NxCatchAllCall("ViewReport Error 2", {
			if (RepJob) {
				RepJob->Close();
			}
			return FALSE;
		});

		
		_RecordsetPtr prs;
		// This is a VITAL function that generates the ADO recordset for each 
		// level of this report (i.e. the report and all subreports) and tells 
		// the RepJob to use those recordsets (GetRecordset is called, which
		// of course calls GetFilter, etc)
		RepJob->SetDataSource(this, &prs);

		// Write to the patient's history tab as appropriate for each report, passing in the preview parameter since we are previewing here
		//TODO: Implement this to determine what the user preference is for this and then only run it when the options is run
		// (j.gruber 2011-10-13 09:50) - PLID 45937 - check the rep job's write to history
		if (RepJob->m_bWriteToHistoryStatus) {
			// (r.gonet 12/18/2012) - PLID 53629 - The statement output types might have changed
			//  while the report was running. Fortunately, we have them saved away from when the
			// report started.
			// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
			WriteToHistory(this->nID, 0, 0, prs, NXR_RUN, NULL, this->nStatementType, this->nOutputType, this->nCategoryType);
		}
		
		// Set all parameters in the RepJob
		nParams = RepJob->GetNParameterFields();
		for (i = 0; i<nParams; i++) { // go through all the parameters requested in the report
			RepJob->GetNthParameterField(i, &tmpParamInfo);
			// Handle any special known parameters
			if (strstr(tmpParamInfo.m_Name, "CurrentUserName")) {
				// The Parameter Dialog shan't appear because of THIS parameter
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				strcpy(tmpParamInfo.m_CurrentValue, CString(GetCurrentUserName()).Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			} 
			else if (strstr(tmpParamInfo.m_Name, "TimeFormat")) {
				// The Parameter Dialog shan't appear because of THIS parameter
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strTimeFormat;
				NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, strTimeFormat.GetBuffer(81), 81, true);
				strTimeFormat.ReleaseBuffer();
				strcpy(tmpParamInfo.m_CurrentValue, strTimeFormat.Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			}
			else if(strstr(tmpParamInfo.m_Name, "HasNexSpa")) {
				//DRT 8/26/2004 - PLID 13974 - Added extra parameter to tell if they have NexSpa module
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strSpa = "0";
				if(IsSpa(FALSE))
					strSpa = "1";
				strcpy(tmpParamInfo.m_CurrentValue, CString(strSpa).Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			}
			else if(strstr(tmpParamInfo.m_Name, "HasEMR")) {
				//TES 9/2/2004 - PLID 13784 - Tell the report whether they have L2 (and hence are entitled to use the letters
				//E, M, and R.
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strData;
				strData.Format("%li", g_pLicense->HasEMR(CLicense::cflrSilent));
				strcpy(tmpParamInfo.m_CurrentValue, strData.Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			}
			// (j.jones 2011-05-17 13:17) - PLID 42341 - supported a currency symbol parameter
			else if (strstr(tmpParamInfo.m_Name, "CurrencySymbol")) {
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strCurrencySymbol = GetCurrencySymbol();
				strcpy(tmpParamInfo.m_CurrentValue, strCurrencySymbol);
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);
			}
			else {
				// Handle any parameters in the pParamList
				// (a.walling 2013-08-30 09:01) - PLID 57998 - Use param map
				std::map<CString, CString>::const_iterator it = paramList.find(tmpParamInfo.m_Name);
				if (it != paramList.end()) {
					// The Parameter Dialog shan't appear because of THIS parameter
					tmpParamInfo.m_CurrentValueSet = 1;
					tmpParamInfo.m_DefaultValueSet = 1;
					strcpy(tmpParamInfo.m_CurrentValue, it->second.Left(PEP_PF_VALUE_LEN));
					// Set parameter Value
					RepJob->SetNthParameterField(i, &tmpParamInfo);
				}
			}
		}

		// set up printing
		//First, tell our parent window to not allow Practice to be closed.
		if(GetMainFrame()) {
			GetMainFrame()->m_bIsReportRunning = true;
		}

		if (bPreview) {
			// Output the report to the screen
			CRect rcParent;
			pParentWnd->GetClientRect(rcParent);
			RepJob->OutputToWindow(strTitle, rcParent.left, rcParent.top, 
				rcParent.Width(), rcParent.Height(),
				WS_VISIBLE|WS_MAXIMIZE, GetMainFrame());
		} else {
			// Output the report to the printer
			if (!RepJob->OutputToPrinter(1, pInfo)) {
				//DRT 4/23/03 - Turn off m_bIsReportRunning!
				GetMainFrame()->m_bIsReportRunning = false;
				// (b.cardillo 2011-08-24 12:46) - PLID 45146 - Fixed memory and file handle leak.
				RepJob->Close();
				RepJob = NULL;
				// The user cancelled
				return FALSE;
			}
		}

		CRPEEnableEventInfo enableEventInfo(true, true, true, true, true, true, true, true);
		RepJob->EnableEvent(&enableEventInfo);
		RepJob->SetEventCallback(ReportEventHandle, (void *)RepJob);

		// Start the actual job
		BOOL nResult = RepJob->Start();

		//Now, at the first possible moment, tell our parent window to allow Practice to be closed.
		if(GetMainFrame()) {
			GetMainFrame()->m_bIsReportRunning = false;
		}

		if (nResult) {
			// Success
			if (nResult == 2) {
				// The RepJob is now owned by the mdi preview window that Start()
				RepJob = NULL;
			} else {
				// The RepJob can safely be deleted because since we printed it (not 
				// previewed it) no one else even knows it exists
				RepJob->Close();
				RepJob = NULL;
			}
			return TRUE;
		} else {
			// Failure
			pParentWnd->MessageBox("Error Starting Report Job: \n" + RepJob->GetErrorText());
			RepJob->Close();
			return FALSE;
		}
	} else {
		// No crystal Engine or the Engine isn't open
		return FALSE;
	}
}

// (a.walling 2013-08-30 09:01) - PLID 57998 - Adapt the CPtrArray to the param map
BOOL CReportInfo::ViewReport(const CString &strTitle, const CString &strFile, CPtrArray* pParamList, BOOL bPreview, CWnd *pParentWnd, CPrintInfo* pInfo) const
{
	std::map<CString, CString> params;
	if (pParamList) {
		for (int i = 0; i < pParamList->GetSize(); ++i) {
			CRParameterInfo* pParam = (CRParameterInfo*)pParamList->GetAt(i);
			if (!pParam) continue;

			if (params.count(pParam->m_Name)) {
				ASSERT(FALSE);
				continue;
			}

			params[pParam->m_Name] = pParam->m_Data;
		}
	}

	return ViewReport(strTitle, strFile, params, bPreview, pParentWnd, pInfo);
}

// Returns TRUE if the report was output
BOOL CReportInfo::ViewReport(const CString &strTitle, const CString &strFile, BOOL bPreview, CWnd *pParentWnd, CPrintInfo* pInfo /* = 0 */) const
{
	// uses MDI interface instead of dialog

	CRPEParameterFieldInfo tmpParamInfo;
	CNxReportJob *RepJob;
	CRPEngine *Engine = GetMainFrame()->GetReportEngine(); //CRPEngine::GetEngine();

	long nPrintJobs = Engine->GetNPrintJobs();
	BOOL bCanClose = Engine->CanClose();

	CRPEJobInfo JobInfo;
	int nParams, i;

	tmpParamInfo.m_StructSize = sizeof(CRPEParameterFieldInfo);

	// We should definitely have an Engine already
	if (!Engine) Engine = GetMainFrame()->GetReportEngine();

	ASSERT(Engine);
	if (Engine && Engine->Open()) {
		try {
			// set up report
			//check to see if it is a custom report
			CString strPathName;
			if (nDefaultCustomReport > 0) {
				strPathName = GetCustomReportsPath() ^ strFile;
				//make sure that the report exists

				// (a.walling 2013-05-08 16:00) - PLID 50104 - Better error message UI when failing to open a report, with retry!
				for (;;) {
					if (DoesExist(strPathName)) {
						break;
					}
					if (IDRETRY != HandleReportJobError(pParentWnd, 2, "Custom report file not found", strPathName, strTitle)) {
						return FALSE;
					}
				}

				//check to see that the version is the correct version
				_RecordsetPtr rsVersion = CreateRecordset("SELECT Version FROM CustomReportsT WHERE ID = %li AND Number = %li", this->nID, this->nDefaultCustomReport);
				if (! rsVersion->eof) {
					long nCurVersion = AdoFldLong(rsVersion, "Version");
					if (nCurVersion != this->nVersion) {

						//the versions don't match
						if (IDYES == MsgBox(MB_YESNO | MB_ICONQUESTION, "The version of this report does not match the version of the query.  You must verify this report before it can be run; if you do not verify, the report will not be run.  Would you like to update and verify the report automatically?")) {
							// (a.walling 2007-06-19 13:52) - PLID 19405 - Check and auto verify the report
							BOOL bVerified = VerifyCustomReport();

							if (!bVerified) {
								MsgBox("The report could not be automatically verified; you must verify this report before it can be run.  Please open the report editor and click the verify button.");
								return FALSE;
							}

							// otherwise, we can continue!
						} else {
							// user chose not to verify automatically	
							return FALSE;
						}
					}
				}
				else {
					// data issue, no friendly messagebox here. Though might be better as an exception. Don't recall ever running into it though.
					MsgBox("Practice could not find the custom report");
					return FALSE;
				}
			}
			else {
				strPathName = GetReportsPath() ^ strFile + ".rpt";
			}
			// (a.walling 2013-05-08 16:00) - PLID 50104 - Better error message UI when failing to open a report, with retry!
			do {
				RepJob = (CNxReportJob *)Engine->OpenJob(strPathName);
			} while (!RepJob && IDRETRY == HandleReportJobError(pParentWnd, Engine->GetErrorCode(), Engine->GetErrorText(), strPathName, strTitle));

			if (!RepJob) {
				return FALSE;
			}
			// set up the Job
			RepJob->EnableProgressDialog(TRUE);
			RepJob->ShowPrintControls(TRUE);

		} NxCatchAllCall("ViewReport Error 2", {
			if (RepJob) {
				RepJob->Close();
			}
			return FALSE;
		});

		
		_RecordsetPtr prs;
		
		// This is a VITAL function that generates the ADO recordset for each 
		// level of this report (i.e. the report and all subreports) and tells 
		// the RepJob to use those recordsets (GetRecordset is called, which
		// of course calls GetFilter, etc)
		RepJob->SetDataSource(this, &prs);

		// Write to the patient's history tab as appropriate for each report, passing in the preview parameter since we are previewing here
		//TODO: Implement this to determine what the user preference is for this and then only run it when the options is run
		// (j.gruber 2011-10-13 09:51) - PLID 45937
		if (RepJob->m_bWriteToHistoryStatus) {
			// (r.gonet 12/18/2012) - PLID 53629 - The statement output types might have changed
			//  while the report was running. Fortunately, we have them saved away from when the
			// report started.
			// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
			WriteToHistory(this->nID, 0, 0, prs, NXR_RUN, NULL, this->nStatementType, this->nOutputType, this->nCategoryType);
		}

		
	
		ASSERT(prs != NULL);

		// Set all parameters in the RepJob
		nParams = RepJob->GetNParameterFields();
		for (i = 0; i<nParams; i++) { // go through all the parameters requested in the report
			RepJob->GetNthParameterField(i, &tmpParamInfo);
			// Handle any special known parameters
			if (strstr(tmpParamInfo.m_Name, "CurrentUserName")) {
				// The Parameter Dialog shan't appear because of THIS parameter
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				strcpy(tmpParamInfo.m_CurrentValue, CString(GetCurrentUserName()).Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			} 
			else if (strstr(tmpParamInfo.m_Name, "TimeFormat")) {
				// The Parameter Dialog shan't appear because of THIS parameter
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strTimeFormat;
				NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, strTimeFormat.GetBuffer(81), 81, true);
				strTimeFormat.ReleaseBuffer();
				strcpy(tmpParamInfo.m_CurrentValue, strTimeFormat.Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			} 
			else if(strstr(tmpParamInfo.m_Name, "HasNexSpa")) {
				//DRT 8/26/2004 - PLID 13974 - Added extra parameter to tell if they have NexSpa module
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strSpa = "0";
				if(IsSpa(FALSE))
					strSpa = "1";
				strcpy(tmpParamInfo.m_CurrentValue, CString(strSpa).Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			}
			else if(strstr(tmpParamInfo.m_Name, "HasEMR")) {
				//TES 9/2/2004 - PLID 13784 - Tell the report whether they have L2 (and hence are entitled to use the letters
				//E, M, and R.
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strData;
				strData.Format("%li", g_pLicense->HasEMR(CLicense::cflrSilent));
				strcpy(tmpParamInfo.m_CurrentValue, strData.Left(PEP_PF_VALUE_LEN));
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);//Set parameter Value
			}
			// (j.jones 2011-05-17 13:17) - PLID 42341 - supported a currency symbol parameter
			else if (strstr(tmpParamInfo.m_Name, "CurrencySymbol")) {
				tmpParamInfo.m_CurrentValueSet = 1;
				tmpParamInfo.m_DefaultValueSet = 1;
				CString strCurrencySymbol = GetCurrencySymbol();
				strcpy(tmpParamInfo.m_CurrentValue, strCurrencySymbol);
				// Set parameter Value
				RepJob->SetNthParameterField(i, &tmpParamInfo);
			}
			else {
				// Handle any parameters in the pParamList
				// (a.walling 2013-08-30 09:01) - PLID 57998 - Use param map
				std::map<CString, CString>::const_iterator it = m_params.find(tmpParamInfo.m_Name);
				if (it != m_params.end()) {
					// The Parameter Dialog shan't appear because of THIS parameter
					tmpParamInfo.m_CurrentValueSet = 1;
					tmpParamInfo.m_DefaultValueSet = 1;
					strcpy(tmpParamInfo.m_CurrentValue, it->second.Left(PEP_PF_VALUE_LEN));
					// Set parameter Value
					RepJob->SetNthParameterField(i, &tmpParamInfo);
				}
			}
		}

		// set up printing
		//First, tell our parent window to not allow Practice to be closed.
		if(GetMainFrame()) {
			GetMainFrame()->m_bIsReportRunning = true;
		}	

		if (bPreview) {
			// Output the report to the screen
			CRect rcParent;
			pParentWnd->GetClientRect(rcParent);
			RepJob->OutputToWindow(strTitle, rcParent.left, rcParent.top, 
				rcParent.Width(), rcParent.Height(),
				WS_VISIBLE|WS_MAXIMIZE, GetMainFrame());
		} else {
			// Output the report to the printer
			if (!RepJob->OutputToPrinter(1, pInfo)) {
				GetMainFrame()->m_bIsReportRunning = false;
				// (b.cardillo 2011-08-24 12:46) - PLID 45146 - Fixed memory and file handle leak.
				RepJob->Close();
				RepJob = NULL;
				// The user cancelled
				return FALSE;
			}
		}

		CRPEEnableEventInfo enableEventInfo(true, true, true, true, true, true, true, true);
		RepJob->EnableEvent(&enableEventInfo);
		RepJob->SetEventCallback(ReportEventHandle, (void *)RepJob);


		// Start the actual job
		BOOL nResult = RepJob->Start();

		//Now, at the first possible moment, tell our parent window to allow Practice to be closed.
		if(GetMainFrame()) {
			GetMainFrame()->m_bIsReportRunning = false;
		}
		if (nResult) {
			// Success
			if (nResult == 2) {
				// The RepJob is now owned by the mdi preview window that Start()
				RepJob = NULL;
			} else {
				// The RepJob can safely be deleted because since we printed it (not 
				// previewed it) no one else even knows it exists
				RepJob->Close();
				RepJob = NULL;
			}
			return TRUE;
		} else {
			// Failure
			pParentWnd->MessageBox("Error Starting Report Job: \n" + RepJob->GetErrorText());
			RepJob->Close();
			return FALSE;
		}
	} else {
		// No crystal Engine or the Engine isn't open
		return FALSE;
	}
}


void CReportInfo::ConvertCrystalToSql(IN const CString &strSql, IN OUT CString &strFilter) const
{
	// TODO: Instead of calling Replace, change the GetWhere function because what if there's a bracket or pound sign in quotes or something
	// Make simple replacements
	strFilter.Replace("{", "");
	strFilter.Replace("}", "");

	//DRT 7/31/03 - We are now using a temp table for the external form, so we need to not replace all # signs.
	//		If the # sign is preceded by a [ (like [#TempMerge032322]), then we will NOT replace it in the query.
	//strFilter.Replace("#", "'");
	long nPound = strFilter.Find("#", 0);
	while(nPound > -1) {
		if(strFilter.GetAt(nPound-1) != '[')
			strFilter.SetAt(nPound, '\'');
		nPound = strFilter.Find("#", nPound+1);
	}

	// Replace field aliases with their actual definitions
	CString strFieldDef;
	if (GetFieldByAlias(strSql, "PatID", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "PatID", strFieldDef);
	}
	if (GetFieldByAlias(strSql, "ProvID", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "ProvID", strFieldDef);
	}
	if (GetFieldByAlias(strSql, "LocID", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "LocID", strFieldDef);
	}
	CString strDateField = GetDateField(nDateFilter);
	if (strDateField != "" && GetFieldByAlias(strSql, strDateField, strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + strDateField, strFieldDef);
	}
	if (GetFieldByAlias(strSql, "TMonth", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "TMonth", strFieldDef);
	}
	if (GetFieldByAlias(strSql, "TYear", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "TYear", strFieldDef);
	}
	/*if (GetFieldByAlias(strSql, "IDate", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "IDate", strFieldDef);
	}
	if (GetFieldByAlias(strSql, "Date", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "Date", strFieldDef);
	}*/
	if (GetFieldByAlias(strSql, "MonthDay", strFieldDef)) { //Used in Patients by Birth Date
		ReplaceWholeWords(strFilter, strRecordSource + "." + "MonthDay", strFieldDef);
	}
	// (j.gruber 2008-07-22 10:40) - PLID 28976 - added TMonthDay and IMonthDay for Practice Analysis reports
	if (GetFieldByAlias(strSql, "IMonthDay", strFieldDef)) { //Used in Practice Analysis reports
		ReplaceWholeWords(strFilter, strRecordSource + "." + "IMonthDay", strFieldDef);
	}
	if (GetFieldByAlias(strSql, "TMonthDay", strFieldDef)) { //Used in Practice Analysis reports
		ReplaceWholeWords(strFilter, strRecordSource + "." + "TMonthDay", strFieldDef);
	}
	if (!strFilterField.IsEmpty() && GetFieldByAlias(strSql, strFilterField, strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + strFilterField, strFieldDef);
	}
	if (!GetExtraField().IsEmpty() && GetFieldByAlias(strSql, GetExtraField(), strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + GetExtraField(), strFieldDef);
	}
	if(GetFieldByAlias(strSql, "EmrID", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "EmrID", strFieldDef);
	}
	if(GetFieldByAlias(strSql, "ApptID", strFieldDef)) {
		ReplaceWholeWords(strFilter, strRecordSource + "." + "ApptID", strFieldDef);
	}

	// If the report's alias still exists in the strFilter at this 
	// point, then we know we forgot a field alias above or someone 
	// isn't aliasing one of the filter-fields properly in this report.
	// By "aliasing" I mean the report's query needs to say "SELECT x 
	// AS FilterByField...", as opposed to just saying "SELECT x FROM..."  
	// For the automated filtering to work properly, all filter fields 
	// must be aliased and their aliases must be included somehow above

	//If you are using a subquery, you must select each item from the subquery individually, you may not use
	//SELECT * FROM a subquery if you are using any of these filters
	// If you get this assertion, tell Bob (sorry, Future-Bob) -- Past-Bob
	
	// (b.cardillo 2007-02-23 12:19) - PLID 24110 - Past-Bob mistakenly set this assert to fail if it 
	// found the record source string AT THE BEGINNING of the filter string, when really it should fail 
	// if it finds it ANYWHERE in the filter string.  What a bozo that Past-Bob was.  I've fixed it, 
	// which means it will now fire an assertion for a lot more cases, including some reports that we 
	// THOUGHT were done correctly.  Those reports will have to be fixed because they don't meet the 
	// standards required for more general filtering.  Such reports may work right now for the filters 
	// they're currently set up for, but expanding to other filtering in the future will cause them 
	// problems.  So once again I have to apologize to Future-Bob for the inevitable annoyance this 
	// comment will cause him.  -- Present-Bob
	ASSERT(ReverseFindNoCase(strFilter, strRecordSource) == -1);
}

// Open the recordset based on GetDefaultSql() combined with strFilter
//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag when we are verifying reports.
_RecordsetPtr CReportInfo::OpenDefaultRecordset(long nSubLevel, long nSubRepNum, BOOL bForReportVerify) const
{
	// Get the default sql for this report
	CString strSql;

	//why do we base this on an arbitrary number like this?
	//since we have all the parts of a report, then why not group it into the different tabs?

	if (strCategory == "PatientP") {
		strSql = GetSqlPatients(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "OthrContactP") {
		strSql = GetSqlContacts(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "ScheduleP") {
		strSql = GetSqlScheduler(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "InventoryP") {
		strSql = GetSqlInventory(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "MarketP") {
		strSql = GetSqlMarketing(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "ChargesP") {
		strSql = GetSqlCharges(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "PaymentsP") {
		strSql = GetSqlPayments(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "OpticalP") { // (s.dhole 2012-04-19 17:11) - PLID 49341  added report category
		strSql = GetSqlOptical(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "FinancialP") {
			//DRT 4/30/03 - This function has now exceeded the internal heap limit (like it used to).  Since
			//		the AR reports take up the most space of all, we'll move them (marked by their IDs) to 
			//		their own function.
			//		Note that since this is contained inside "FinancialP" section, you must have the AR report
			//		in the financial tab to use this special function.
			// (z.manning 2009-03-25 12:10) - PLID 19120 - Added AR by Financial Class (659)
			// (j.gruber 2011-10-10 08:56) - PLID 45745- added AR by Category
			if(nID == 151 || nID == 170 || nID == 177 || nID == 238 || nID == 385 || nID == 388 || nID == 390 || nID == 391 || nID == 659 || nID == 715) {
				strSql = GetSqlAR(nSubLevel, nSubRepNum);
			}
			else {
				strSql = GetSqlFinancial(nSubLevel, nSubRepNum);
			}
	}
	else if (strCategory == "ASCP") {
		strSql = GetSqlASC(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "AdminP") {
		strSql = GetSqlAdministration(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "OtherP") {
		strSql = GetSqlOther(nSubLevel, nSubRepNum);
	}
	// (j.gruber 2008-07-14 16:33) - PLID 28976 - add Practice Analysis tab
	else if (strCategory == "PracAnalP") {
		strSql = GetSqlPracticeAnalysis(nSubLevel, nSubRepNum);
	}
	else if (strCategory == "") {
		strSql = GetSqlPrintPreview(nSubLevel, nSubRepNum);
	}

	if (strSql == _T("")) {
		// We have no sql statement to open!
		return NULL;
	} else {
		// We have an sql statement.  Append our filter string 
		// to it and open it, returning the opened recordset.
		// Append the given filter
		// (f.dinatale 2011-03-02) - PLID 41275 - Fixed the conditional to exclude the WHERE clause from the NexReminder Client Usage report.
		//	(b.savon 2011-06-27) - PLID 44179 - Disregard filters of Escalation feature because they are already accounted for in query
		//(b.savon 2011-07-19) - PLID 44523 - Disregard filters of Escalation feature because they are already accounted for in query
		//(j.camacho 2014-10-31) - PLID 62716 - This should have been checked long ago. We don't need filters to verify our reports fields.
		if (!bForReportVerify)
		{
			CString strLocFilter = GetFilter(nSubLevel, nSubRepNum);
			strLocFilter.TrimLeft();
			strLocFilter.TrimRight();
			if (!strLocFilter.IsEmpty() && nID != 701 && nID != 708 && nID != 712) {
				ConvertCrystalToSql(strSql, strLocFilter);
				AddFilter(strSql, strLocFilter, TRUE);
			}
		}
		//(e.lally 2008-04-07) PLID 9989 - check if we are verifying reports and add a filter to return no records
			//for a big performance boost.
		//	(b.savon 2011-06-27) - PLID 44179 - Disregard filters of Escalation feature because they are already accounted for in query
		//(b.savon 2011-07-19) - PLID 44523 - Disregard filters of Escalation feature because they are already accounted for in query
		if(bForReportVerify != FALSE && nID != 701 && nID != 708 && nID != 712){
			CString strNoRecordFilter = " 1=0 ";
			//return no actual records for report verifications

			AddFilter(strSql, strNoRecordFilter, TRUE);
		}

		// (c.haag 2007-01-11 17:06) - PLID 7048 - Allow final changes to the resultant SQL. This
		// has to be done for the AAFPRS report to work properly because special filters are needed
		// within inner queries. Rather than potentially breaking existing functionality, this seems
		// like a fair solution to me for the .1% of reports we write that have very special needs.
		//
		// This should only be used when absolutely necessary
		//
		FinalizeReportSqlQuery(nSubLevel, nSubRepNum, strSql);

#ifdef _DEBUG
#ifndef ENABLE_VERIFY_ALL_REPORTS
		//DRT 2/12/2004 - PLID 2806 - If the query has a PatID field, but does not allow filter
		//	on patients or "create merge group", we should warn the programmer.  It needs to be
		//	able to be hidden b/c there are some reports that aren't going to have them enabled
		//	for some reason or another.
		{
			CString strTest;
			if(GetFieldByAlias(strSql, "PatID", strTest) && !strCategory.IsEmpty()) {
				CString strOut;
				//there is a pat ID.  See if they have these fields enabled
				if(nPatient == 0)
					strOut += " - Your patient filter is not enabled.\r\n";
				if(!bCreateGroup)
					strOut += " - Your 'Create Merge Group' option is not enabled.\r\n";

				if(!strOut.IsEmpty()) {
					strOut = "You have a PatID field in your report, but you do not have the following enabled.  It is recommended that all "
						"reports have these enabled:\r\n" + strOut + "Would you like to continue to be reminded about this?\r\n"
						" - If you say Yes, this warning will continue to pop up.  Please use this to remind yourself that you should enable those fields.\r\n"
						" - If you say No, a registry setting will be filled on this machine and you will no longer get this warning.  Use this option if there is a "
						"GOOD reason for these options not to be enabled.";
					CString strName;
					strName.Format("HKEY_LOCAL_MACHINE\\Software\\NexTech\\ReportDebugWarning\\%s", strPrintName);

					//if this key already exists, we don't want to prompt them.  Otherwise, ask if they want to keep reminding
					if(NxRegUtils::ReadLong(strName, 0) != 1) {
						if(AfxMessageBox(strOut, MB_YESNO) == IDNO) {
							//they do not want to remember
							NxRegUtils::WriteLong(strName, 1, true);
						}
					}
				}
			}
		}	//end debug warning

		// (z.manning 2008-11-26 15:58) - PLID 32125 - We insert images into a table variable for
		// the lab report (567) and CMsgBox doesn't seem to like to display the SQL to do so, so 
		// don't bother for that report.
		//TES 3/12/2009 - PLID 33468 - Split this into two reports, so check for both.
		// (j.gruber 2010-03-09 13:16) - PLID 37674 - also the EMN Report
		// (r.goldschmidt 2014-11-13 18:05) - PLID 63982 - preference to block inserting images 
		//   into table variable for lab results and lab requests reports (567 and 658) (and I wanted to see the queries)
		BOOL nBlockedImageInsert = GetRemotePropertyInt("BlockLabReportsFromLoadingSignatureImages", 0, 0, "<None>");
		if((nBlockedImageInsert || (nID != 567 && nID != 658)) && nID != 689) {
			CMsgBox dlg(NULL);
			dlg.msg = strSql;
			if (strSql.Find("\n") != -1)
			{
				// Its multi-line formatted. Don't mess it up.
				dlg.m_bWordWrap = false;
			}
			else
			{
				// Its a jumbled Access-generated mess or we didn't put any line formatting. 
				// Don't show it on a single line.
				dlg.m_bWordWrap = true;
			}
			dlg.DoModal();
		}
#endif
#endif
		// Open the recordset
		// (a.walling 2009-08-11 12:59) - PLID 35178 - Use the snapshot connection, which will use snapshot isolation (if it supports it)
		//(r.wilson 9/17/2013) PLID 58651 - Check to see if this report should be run using server side cursors
		ADODB::CursorLocationEnum eCursorLocation = adUseClient;

		_ConnectionPtr pCon;
		if(IsServerSide(nID))
		{
			eCursorLocation = adUseServer;
			pCon = GetRemoteDataReportSnapshot(); // (c.haag 2015-01-22) - PLID 64646 - Use a connection specific for server-side reports
		}
		else
		{
			eCursorLocation = adUseClient;
			pCon = GetRemoteDataSnapshot();
		}
		
		// was previously using adOpenDynamic, which is silly, since we just want our own copy of the data.
		// adOpenForwardOnly works great for the server-side reports. For client-side, it is equivalent to adOpenStatic.
		// However there are some places that still end up calling things like MoveFirst to re-run the recordset.
		// For safety's sake, we are going to run this with a real cursor anyway, but a static one, since we don't
		// care about changes made by other people -- we just want a static copy of the recordset.
		return CreateRecordsetStd(pCon, strSql, adOpenStatic, adLockReadOnly, ADODB::adCmdText, eCursorLocation);
	}
}

BOOL CReportInfo::CreateGroup() const
{
	// Create the group based on the given report

	//First, give them a shot at the external filter.
	try {
		CString strTemp, strSortBy;
		if (bExternal) {
			//TES 12/18/2008 - PLID 32514 - They can't use the external filter if they're
			// on the Scheduler Standard version.
			if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
				CExternalForm Form(NULL);

				Form.m_RepID.Format("%i",nID);

				Form.m_Caption		= strPrintName;
				Form.m_ColFormat	= strListBoxFormat;
				Form.m_ColWidths	= strListBoxWidths;
				strTemp				= strListBoxSQL;
				Form.m_BoundCol		= atoi(strTemp.Left(1));
				strSortBy			= strTemp.Left(2);
				Form.m_SortBy		= strSortBy.Right(1);
				Form.m_SQL			= strTemp.Right(strTemp.GetLength()-2);
				Form.m_FilterField  = strRecordSource + "." + strFilterField;
				Form.m_Filter		= (CString*)(&strExternalFilter);
				
				Form.m_RepID.Format("%li",nID);

				if (Form.DoModal() == IDCANCEL) return FALSE;
			}
			
		}
		switch(nID) {
		case 295: //Active Steps By Age
			//TES 9/30/2005 - The patients are actually listed in the first subreport.
			return CreateDefaultGroup(GetFilter(1,0), 1, 0);
			break;

		default:
			return CreateDefaultGroup(GetBaseFilter(), 0, 0);
			break;
		}

	}NxCatchAll("Error 1 in CReportInfo::CreateGroup()");
	return FALSE;
}

BOOL CReportInfo::CreateDefaultGroup(LPCTSTR strFilter, long nSubLevel, long nSubRepNum) const
{
	//Get a name from the user.
	CString strGroupName;
	CGetNewIDName dlgGetNew(NULL);
	dlgGetNew.m_pNewName = &strGroupName;
	dlgGetNew.m_strCaption = "Enter a Name for the New Group";
	dlgGetNew.m_nMaxLength = 50;

	if (dlgGetNew.DoModal() != IDOK) {
		return FALSE;
	}
	//does this group already exist?
	_RecordsetPtr rs;
	rs = CreateParamRecordset("SELECT Name FROM GroupsT WHERE Name = {STRING}", strGroupName);
	if(!rs->eof)
	{
		rs->Close();
		AfxMessageBox("Group name already exists.  Please try again with a different name.");
		return FALSE;
	}
	rs->Close();
	BOOL bReturn = TRUE;

	CString strTempTableName;

	ADODB::_ConnectionPtr pCon;

	//BEGIN_TRANS("CReportInfo::CreateDefaultGroup") {

	// (a.walling 2009-08-11 13:10) - PLID 35178 - We want to use the snapshot-capable connection, but that does not
	// have the BEGIN_TRANS etc macros, so just use the (IMO safer) CSqlTransaction class and convert the calls to use
	// the correct connection.
	// (j.jones 2015-11-05 16:10) - PLID 66881 - we now avoid the transaction entirely by making a temp table instead
	try {		
		pCon = CReportInfo::IsServerSide(nID, true) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot();
		
		// (j.jones 2015-11-05 16:10) - PLID 66881 - bump up the timeout indefinitely
		CIncreaseCommandTimeout cict(pCon, 0);
		
		// (j.jones 2015-11-05 16:11) - PLID 66881 - create a temp table for our patients
		strTempTableName = FormatString("#ReportPersonT_%lu", GetTickCount());
		ExecuteSqlStd(pCon, FormatString("CREATE TABLE %s (PersonID INT);", strTempTableName));

		// Get the default sql for this report
		// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object.
		// For any GetSql* function that returns a string, the complex query will treat it as simple SQL without a CTE.
		CComplexReportQuery preQuery;

		if (strCategory == "PatientP") {
			preQuery = GetSqlPatients(nSubLevel, nSubRepNum);
		}
			else if (strCategory == "OthrContactP") {
			preQuery = GetSqlContacts(nSubLevel, nSubRepNum);
		}
			else if (strCategory == "ScheduleP") {
			preQuery = GetSqlScheduler(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "InventoryP") {
			preQuery = GetSqlInventory(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "MarketP") {
			preQuery = GetSqlMarketing(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "ChargesP") {
			preQuery = GetSqlCharges(nSubLevel, nSubRepNum);
		}
		// (s.dhole 2012-04-19 16:39) - PLID 49341
		else if (strCategory == "OpticalP") {
			preQuery = GetSqlOptical(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "PaymentsP") {

			preQuery = GetSqlPayments(nSubLevel, nSubRepNum);

			// (j.jones 2008-05-01 17:30) - PLID 29877 - Batch Payments needs to filter out NULL patient IDs
			// (c.haag 2016-03-21) - PLID 68251 - Modify the SQL of the pre-query
			if(nID == 322) { //Batch Payments
				preQuery.m_strSQL += " WHERE BatchPaymentsSubQ.PatID Is Not Null ";
			}
		}
		else if (strCategory == "FinancialP") {
			//DRT 4/30/03 - This function has now exceeded the internal heap limit (like it used to).  Since
			//		the AR reports take up the most space of all, we'll move them (marked by their IDs) to 
			//		their own function.
			//		Note that since this is contained inside "FinancialP" section, you must have the AR report
			//		in the financial tab to use this special function.
			// (z.manning 2009-03-25 12:10) - PLID 19120 - Added AR by Financial Class (659)
			// (j.gruber 2011-10-10 08:57) - PLID 45745 - added AR by service category
			if(nID == 151 || nID == 170 || nID == 177 || nID == 238 || nID == 385 || nID == 388 || nID == 390 || nID == 391 || nID == 659 || nID == 715) {
				preQuery = GetSqlAR(nSubLevel, nSubRepNum);
			}
			else {
				preQuery = GetSqlFinancial(nSubLevel, nSubRepNum);
			}
		}
		else if (strCategory == "ASCP") {
			preQuery = GetSqlASC(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "AdminP") {
			preQuery = GetSqlAdministration(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "OtherP") {
			preQuery = GetSqlOther(nSubLevel, nSubRepNum);
		}
		// (j.gruber 2008-07-14 16:34) - PLID 28976 - Practice Analysis
		else if (strCategory == "PracAnalP") {
			preQuery = GetSqlPracticeAnalysis(nSubLevel, nSubRepNum);
		}
		else if (strCategory == "") {
			preQuery = GetSqlPrintPreview(nSubLevel, nSubRepNum);
		}
		if (!preQuery.IsValid()) {
			//DRT 1/23/2004 - PLID 10750 - Special cases!  Some reports are handled in wierd ways (see ReportInfoCallback.cpp, GetRecordset()), 
			//	so we need to handle those just in case.
			
			switch(nID) {
			case 169:  //Patient Statement
			case 337:
			case 234:
			case 338:
			case 353:
			case 354:
			case 355:
			case 356:
			case 434:
			case 435:
			case 436:
			case 437:
			case 483:
			case 484:
			case 485:
			case 486:

				// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object.
				preQuery = GetSqlStatement(pCon, nSubLevel, nSubRepNum);
				break;
			}
		}

		if(!preQuery.IsValid()) {
			// We have no sql statement to open!
			bReturn = FALSE; //Don't return now, we're in the middle of a transaction.
		}
		else {
			// We have an sql statement.  Append our filter string 
			// to it and use it to generate the new group listing
			// Append the given filter
			//PLID 19451 - the statement already has its filters built in by now
			if (!IsStatement(nID)) {
				CString strLocFilter(strFilter);
				strLocFilter.TrimLeft();
				strLocFilter.TrimRight();
				if (!strLocFilter.IsEmpty() && nID != 701) {
					ConvertCrystalToSql(preQuery.m_strSQL, strLocFilter);
					AddFilter(preQuery.m_strSQL, strLocFilter, TRUE);
				}
			}

			//Special filters for particular reports
			switch (nID) {
				//DRT 7/22/2004 - PLID 13573 - For special reports, we need to filter out
				//	the inquiries, they are not allowed to be a member of LW groups
			case 24:
			case 378:
			{
				CString str;
				str.Format("PatientsT.CurrentStatus <> 4");
				AddFilter(preQuery.m_strSQL, str, TRUE);
			}
			break;

			//DRT 10/3/2005:  PLID 17759, 17760 - These reports add a record of NULL PersonID for
			//	the initial amount for the cash drawer.  We will just drop that record
			case 501:
			case 503:
			{
				CString str;
				str.Format("PatID IS NOT NULL");
				AddFilter(preQuery.m_strSQL, str, TRUE);
			}
			}

			// (j.gruber 2009-12-18 10:36) - PLID 36109 - special handling again!
			// rearrange where the INSERT goes for the referrals report since it has a SET NOCOUNT
			// (j.jones 2015-11-05 16:11) - PLID 66881 - this now uses a temp table
			CString strInsert;
			strInsert.Format("INSERT INTO %s(PersonID) "
				"SELECT PatID AS PersonID "
				"FROM (", strTempTableName);
			
			CComplexReportQuery groupQuery;
			CString strNoCountSQL;
			switch (nID) {

			case 24:
				strNoCountSQL = "SET NOCOUNT OFF";
				// (c.haag 2016-04-01) - PLID 68251 - Use a complex query rather than a SQL string for creating
				// groups from the primary referral report.
				groupQuery = CComplexReportQuery(preQuery);
				groupQuery.m_strSQL.Replace("SET NOCOUNT OFF", strInsert);
				break;

			default:
				// (c.haag 2016-04-01) - PLID 68251 - Use a complex query rather than a SQL string
				groupQuery = CComplexReportQuery(preQuery.m_strCTE, FormatString("%s %s", strInsert, preQuery.m_strSQL));
				break;
			}

			// Add the patients into the group (only once per patient)
#ifdef _DEBUG
			// (c.haag 2016-04-01) - PLID 68251 - Format as runnable SQL
			CString str;
			str.Format("%s %s %s) RunTimeReportQ GROUP BY PatID", strNoCountSQL, groupQuery.m_strCTE, groupQuery.m_strSQL);
			CMsgBox dlg(NULL);
			dlg.msg = str;
			dlg.DoModal();
#endif

			NxAdo::PushMaxRecordsWarningLimit pmr(-1);

			// (a.walling 2009-08-11 13:15) - PLID 35178 - Use the snapshot-capable connection
			// (c.haag 2016-04-01) - PLID 68251 - Format as runnable SQL
			ExecuteSql(pCon, "%s %s %s) RunTimeReportQ GROUP BY PatID", strNoCountSQL, groupQuery.m_strCTE, groupQuery.m_strSQL);

			// since paramsql runs in sp_executesql / stored procedure scope, will auto revert to snapshot isolation when scope exits
			// (j.jones 2015-11-05 16:11) - PLID 66881 - this now uses a temp table
			ExecuteParamSql(pCon, FormatString(R"(
SET TRANSACTION ISOLATION LEVEL READ COMMITTED
BEGIN TRANSACTION
DECLARE @nGroupID INT;
SET @nGroupID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM GroupsT);

INSERT INTO GroupsT(ID, Name) VALUES (@nGroupID, {STRING});

INSERT INTO GroupDetailsT(GroupID, PersonID)
SELECT DISTINCT @nGroupID AS GroupID, PersonID 
FROM %s
WHERE PersonID IS NOT NULL;

COMMIT TRANSACTION
)", strTempTableName),
	strGroupName);
		}
	} NxCatchAll("CReportInfo::CreateDefaultGroup");

	// (j.jones 2015-11-05 16:14) - PLID 66881 - drop our temp table outside of the above try/catch block
	try {
		if (!strTempTableName.IsEmpty()) {
			ExecuteSqlStd(pCon, FormatString("DROP TABLE %s;", strTempTableName));
			strTempTableName = "";
			pCon = NULL;
		}
	} NxCatchAll("CReportInfo::CreateDefaultGroup:DropTempTable");

	return bReturn;
}

// Macro specifically used in the context of the CReportInfo::GetDescription function
#define HANDLE_REPORT_DESCRIPTION(report_id)		case report_id: return GetStringOfResource(IDS_REPORT_DESC_##report_id);

// Use this function to generate the report description for each reportinfo
// (s.dhole 2010-07-20 08:42) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
//Added 698
CString CReportInfo::GetDescription() const
{
	switch (nID) {
		HANDLE_REPORT_DESCRIPTION(1); HANDLE_REPORT_DESCRIPTION(2); HANDLE_REPORT_DESCRIPTION(3); HANDLE_REPORT_DESCRIPTION(4);
		HANDLE_REPORT_DESCRIPTION(10); HANDLE_REPORT_DESCRIPTION(11); HANDLE_REPORT_DESCRIPTION(13); HANDLE_REPORT_DESCRIPTION(14);
		HANDLE_REPORT_DESCRIPTION(15); HANDLE_REPORT_DESCRIPTION(16); HANDLE_REPORT_DESCRIPTION(17); HANDLE_REPORT_DESCRIPTION(18);
		HANDLE_REPORT_DESCRIPTION(19); HANDLE_REPORT_DESCRIPTION(23); HANDLE_REPORT_DESCRIPTION(24); HANDLE_REPORT_DESCRIPTION(25);
		HANDLE_REPORT_DESCRIPTION(42); HANDLE_REPORT_DESCRIPTION(49); HANDLE_REPORT_DESCRIPTION(50); HANDLE_REPORT_DESCRIPTION(57);
		HANDLE_REPORT_DESCRIPTION(58); HANDLE_REPORT_DESCRIPTION(59); HANDLE_REPORT_DESCRIPTION(60); HANDLE_REPORT_DESCRIPTION(61);
		HANDLE_REPORT_DESCRIPTION(63); HANDLE_REPORT_DESCRIPTION(74); HANDLE_REPORT_DESCRIPTION(75); HANDLE_REPORT_DESCRIPTION(76);
		HANDLE_REPORT_DESCRIPTION(78); HANDLE_REPORT_DESCRIPTION(81); HANDLE_REPORT_DESCRIPTION(82); HANDLE_REPORT_DESCRIPTION(86);
		HANDLE_REPORT_DESCRIPTION(87); HANDLE_REPORT_DESCRIPTION(88); HANDLE_REPORT_DESCRIPTION(90); HANDLE_REPORT_DESCRIPTION(93);
		HANDLE_REPORT_DESCRIPTION(94); HANDLE_REPORT_DESCRIPTION(99); HANDLE_REPORT_DESCRIPTION(106); HANDLE_REPORT_DESCRIPTION(107);
		HANDLE_REPORT_DESCRIPTION(108); HANDLE_REPORT_DESCRIPTION(110); HANDLE_REPORT_DESCRIPTION(118); HANDLE_REPORT_DESCRIPTION(120);
		HANDLE_REPORT_DESCRIPTION(121); HANDLE_REPORT_DESCRIPTION(122); HANDLE_REPORT_DESCRIPTION(123); HANDLE_REPORT_DESCRIPTION(124);
		HANDLE_REPORT_DESCRIPTION(125); HANDLE_REPORT_DESCRIPTION(126); HANDLE_REPORT_DESCRIPTION(130); HANDLE_REPORT_DESCRIPTION(131);
		HANDLE_REPORT_DESCRIPTION(134); HANDLE_REPORT_DESCRIPTION(135); HANDLE_REPORT_DESCRIPTION(136); HANDLE_REPORT_DESCRIPTION(137);
		HANDLE_REPORT_DESCRIPTION(138); HANDLE_REPORT_DESCRIPTION(139); HANDLE_REPORT_DESCRIPTION(140); HANDLE_REPORT_DESCRIPTION(141);
		HANDLE_REPORT_DESCRIPTION(142); HANDLE_REPORT_DESCRIPTION(143); HANDLE_REPORT_DESCRIPTION(144); HANDLE_REPORT_DESCRIPTION(145);
		HANDLE_REPORT_DESCRIPTION(146); HANDLE_REPORT_DESCRIPTION(147); HANDLE_REPORT_DESCRIPTION(148); HANDLE_REPORT_DESCRIPTION(149);
		HANDLE_REPORT_DESCRIPTION(151); HANDLE_REPORT_DESCRIPTION(152); HANDLE_REPORT_DESCRIPTION(153); HANDLE_REPORT_DESCRIPTION(154);
		HANDLE_REPORT_DESCRIPTION(155); HANDLE_REPORT_DESCRIPTION(156); HANDLE_REPORT_DESCRIPTION(157); HANDLE_REPORT_DESCRIPTION(158);
		HANDLE_REPORT_DESCRIPTION(159); HANDLE_REPORT_DESCRIPTION(160); HANDLE_REPORT_DESCRIPTION(161); HANDLE_REPORT_DESCRIPTION(162);
		HANDLE_REPORT_DESCRIPTION(163); HANDLE_REPORT_DESCRIPTION(164); HANDLE_REPORT_DESCRIPTION(165); HANDLE_REPORT_DESCRIPTION(166);
		HANDLE_REPORT_DESCRIPTION(168); HANDLE_REPORT_DESCRIPTION(169); HANDLE_REPORT_DESCRIPTION(170); HANDLE_REPORT_DESCRIPTION(171);
		HANDLE_REPORT_DESCRIPTION(172); HANDLE_REPORT_DESCRIPTION(173); HANDLE_REPORT_DESCRIPTION(174); HANDLE_REPORT_DESCRIPTION(175);
		HANDLE_REPORT_DESCRIPTION(176); HANDLE_REPORT_DESCRIPTION(177); HANDLE_REPORT_DESCRIPTION(179); HANDLE_REPORT_DESCRIPTION(180);
		HANDLE_REPORT_DESCRIPTION(183); HANDLE_REPORT_DESCRIPTION(184); HANDLE_REPORT_DESCRIPTION(185); HANDLE_REPORT_DESCRIPTION(186);
		HANDLE_REPORT_DESCRIPTION(187); HANDLE_REPORT_DESCRIPTION(188); HANDLE_REPORT_DESCRIPTION(190); HANDLE_REPORT_DESCRIPTION(192);
		HANDLE_REPORT_DESCRIPTION(193); HANDLE_REPORT_DESCRIPTION(194); HANDLE_REPORT_DESCRIPTION(195); HANDLE_REPORT_DESCRIPTION(197);
		HANDLE_REPORT_DESCRIPTION(199); HANDLE_REPORT_DESCRIPTION(200); HANDLE_REPORT_DESCRIPTION(201); HANDLE_REPORT_DESCRIPTION(202);
		HANDLE_REPORT_DESCRIPTION(203); HANDLE_REPORT_DESCRIPTION(204); HANDLE_REPORT_DESCRIPTION(205); HANDLE_REPORT_DESCRIPTION(206);
		HANDLE_REPORT_DESCRIPTION(207); HANDLE_REPORT_DESCRIPTION(208); HANDLE_REPORT_DESCRIPTION(209); HANDLE_REPORT_DESCRIPTION(210);
		HANDLE_REPORT_DESCRIPTION(211); HANDLE_REPORT_DESCRIPTION(212); HANDLE_REPORT_DESCRIPTION(213); HANDLE_REPORT_DESCRIPTION(214);
		HANDLE_REPORT_DESCRIPTION(215); HANDLE_REPORT_DESCRIPTION(216); HANDLE_REPORT_DESCRIPTION(217); HANDLE_REPORT_DESCRIPTION(218);
		HANDLE_REPORT_DESCRIPTION(219); HANDLE_REPORT_DESCRIPTION(220); HANDLE_REPORT_DESCRIPTION(221); HANDLE_REPORT_DESCRIPTION(222); 
		HANDLE_REPORT_DESCRIPTION(223); HANDLE_REPORT_DESCRIPTION(224);	HANDLE_REPORT_DESCRIPTION(225); 
		HANDLE_REPORT_DESCRIPTION(226); HANDLE_REPORT_DESCRIPTION(227); HANDLE_REPORT_DESCRIPTION(228); HANDLE_REPORT_DESCRIPTION(229);
		HANDLE_REPORT_DESCRIPTION(230); HANDLE_REPORT_DESCRIPTION(231);	HANDLE_REPORT_DESCRIPTION(233);	HANDLE_REPORT_DESCRIPTION(234); 
		HANDLE_REPORT_DESCRIPTION(235);	HANDLE_REPORT_DESCRIPTION(236); HANDLE_REPORT_DESCRIPTION(237); HANDLE_REPORT_DESCRIPTION(238); 
		HANDLE_REPORT_DESCRIPTION(240); HANDLE_REPORT_DESCRIPTION(241); HANDLE_REPORT_DESCRIPTION(242); HANDLE_REPORT_DESCRIPTION(243);
		HANDLE_REPORT_DESCRIPTION(244); HANDLE_REPORT_DESCRIPTION(245); HANDLE_REPORT_DESCRIPTION(246);
		HANDLE_REPORT_DESCRIPTION(247); HANDLE_REPORT_DESCRIPTION(248); HANDLE_REPORT_DESCRIPTION(249); HANDLE_REPORT_DESCRIPTION(250);
		HANDLE_REPORT_DESCRIPTION(251); HANDLE_REPORT_DESCRIPTION(252); HANDLE_REPORT_DESCRIPTION(253); HANDLE_REPORT_DESCRIPTION(254);
		HANDLE_REPORT_DESCRIPTION(255); HANDLE_REPORT_DESCRIPTION(256); HANDLE_REPORT_DESCRIPTION(257);  HANDLE_REPORT_DESCRIPTION(258); HANDLE_REPORT_DESCRIPTION(259);
		HANDLE_REPORT_DESCRIPTION(260); HANDLE_REPORT_DESCRIPTION(261); HANDLE_REPORT_DESCRIPTION(263); HANDLE_REPORT_DESCRIPTION(264);
		HANDLE_REPORT_DESCRIPTION(265); HANDLE_REPORT_DESCRIPTION(266); HANDLE_REPORT_DESCRIPTION(267); HANDLE_REPORT_DESCRIPTION(268);
		HANDLE_REPORT_DESCRIPTION(269); HANDLE_REPORT_DESCRIPTION(270); HANDLE_REPORT_DESCRIPTION(271); HANDLE_REPORT_DESCRIPTION(272);
		HANDLE_REPORT_DESCRIPTION(273); HANDLE_REPORT_DESCRIPTION(274); HANDLE_REPORT_DESCRIPTION(275); HANDLE_REPORT_DESCRIPTION(276);
		HANDLE_REPORT_DESCRIPTION(277); HANDLE_REPORT_DESCRIPTION(278); HANDLE_REPORT_DESCRIPTION(279); HANDLE_REPORT_DESCRIPTION(280);
		HANDLE_REPORT_DESCRIPTION(281);	HANDLE_REPORT_DESCRIPTION(282);	HANDLE_REPORT_DESCRIPTION(283);	HANDLE_REPORT_DESCRIPTION(284);
		HANDLE_REPORT_DESCRIPTION(285); HANDLE_REPORT_DESCRIPTION(286); HANDLE_REPORT_DESCRIPTION(287);	HANDLE_REPORT_DESCRIPTION(288);
		HANDLE_REPORT_DESCRIPTION(289);	HANDLE_REPORT_DESCRIPTION(290); HANDLE_REPORT_DESCRIPTION(291);	HANDLE_REPORT_DESCRIPTION(292);
		HANDLE_REPORT_DESCRIPTION(293); HANDLE_REPORT_DESCRIPTION(295); HANDLE_REPORT_DESCRIPTION(296);	HANDLE_REPORT_DESCRIPTION(297);
		HANDLE_REPORT_DESCRIPTION(298); HANDLE_REPORT_DESCRIPTION(299); HANDLE_REPORT_DESCRIPTION(300); HANDLE_REPORT_DESCRIPTION(301); HANDLE_REPORT_DESCRIPTION(302);
		HANDLE_REPORT_DESCRIPTION(303); HANDLE_REPORT_DESCRIPTION(304); HANDLE_REPORT_DESCRIPTION(305); HANDLE_REPORT_DESCRIPTION(306);
		HANDLE_REPORT_DESCRIPTION(307); HANDLE_REPORT_DESCRIPTION(308); HANDLE_REPORT_DESCRIPTION(309); HANDLE_REPORT_DESCRIPTION(310);
		HANDLE_REPORT_DESCRIPTION(311); HANDLE_REPORT_DESCRIPTION(312); HANDLE_REPORT_DESCRIPTION(313); HANDLE_REPORT_DESCRIPTION(314);
		HANDLE_REPORT_DESCRIPTION(315); HANDLE_REPORT_DESCRIPTION(316); HANDLE_REPORT_DESCRIPTION(317); HANDLE_REPORT_DESCRIPTION(318);
		HANDLE_REPORT_DESCRIPTION(319); HANDLE_REPORT_DESCRIPTION(320); HANDLE_REPORT_DESCRIPTION(321); HANDLE_REPORT_DESCRIPTION(322);
		HANDLE_REPORT_DESCRIPTION(323); HANDLE_REPORT_DESCRIPTION(324); HANDLE_REPORT_DESCRIPTION(325); HANDLE_REPORT_DESCRIPTION(326);
		HANDLE_REPORT_DESCRIPTION(327); HANDLE_REPORT_DESCRIPTION(328); HANDLE_REPORT_DESCRIPTION(329);	HANDLE_REPORT_DESCRIPTION(330);
		HANDLE_REPORT_DESCRIPTION(331);	HANDLE_REPORT_DESCRIPTION(332);	HANDLE_REPORT_DESCRIPTION(333);	HANDLE_REPORT_DESCRIPTION(334);
		HANDLE_REPORT_DESCRIPTION(335);	HANDLE_REPORT_DESCRIPTION(336);	HANDLE_REPORT_DESCRIPTION(337);	HANDLE_REPORT_DESCRIPTION(338); HANDLE_REPORT_DESCRIPTION(339);
		HANDLE_REPORT_DESCRIPTION(262); HANDLE_REPORT_DESCRIPTION(340); HANDLE_REPORT_DESCRIPTION(341); HANDLE_REPORT_DESCRIPTION(342); 
		HANDLE_REPORT_DESCRIPTION(343); HANDLE_REPORT_DESCRIPTION(344); HANDLE_REPORT_DESCRIPTION(345); HANDLE_REPORT_DESCRIPTION(346);
		HANDLE_REPORT_DESCRIPTION(347); HANDLE_REPORT_DESCRIPTION(348); HANDLE_REPORT_DESCRIPTION(349);
		HANDLE_REPORT_DESCRIPTION(350);	HANDLE_REPORT_DESCRIPTION(351);	HANDLE_REPORT_DESCRIPTION(352); HANDLE_REPORT_DESCRIPTION(353);
		HANDLE_REPORT_DESCRIPTION(354); HANDLE_REPORT_DESCRIPTION(355); HANDLE_REPORT_DESCRIPTION(356); HANDLE_REPORT_DESCRIPTION(357);
		HANDLE_REPORT_DESCRIPTION(358); HANDLE_REPORT_DESCRIPTION(359); HANDLE_REPORT_DESCRIPTION(360); HANDLE_REPORT_DESCRIPTION(361);
		HANDLE_REPORT_DESCRIPTION(362); HANDLE_REPORT_DESCRIPTION(363); HANDLE_REPORT_DESCRIPTION(364); HANDLE_REPORT_DESCRIPTION(365);
		HANDLE_REPORT_DESCRIPTION(366); HANDLE_REPORT_DESCRIPTION(367); HANDLE_REPORT_DESCRIPTION(368); HANDLE_REPORT_DESCRIPTION(369);
		HANDLE_REPORT_DESCRIPTION(370); HANDLE_REPORT_DESCRIPTION(371); HANDLE_REPORT_DESCRIPTION(372); HANDLE_REPORT_DESCRIPTION(373); 
		HANDLE_REPORT_DESCRIPTION(374); HANDLE_REPORT_DESCRIPTION(375); HANDLE_REPORT_DESCRIPTION(376); HANDLE_REPORT_DESCRIPTION(377); 
		HANDLE_REPORT_DESCRIPTION(378); HANDLE_REPORT_DESCRIPTION(379); HANDLE_REPORT_DESCRIPTION(380); HANDLE_REPORT_DESCRIPTION(381); 
		HANDLE_REPORT_DESCRIPTION(382); HANDLE_REPORT_DESCRIPTION(383); HANDLE_REPORT_DESCRIPTION(384); HANDLE_REPORT_DESCRIPTION(385); 
		HANDLE_REPORT_DESCRIPTION(386); HANDLE_REPORT_DESCRIPTION(387); HANDLE_REPORT_DESCRIPTION(388); HANDLE_REPORT_DESCRIPTION(389); 
		HANDLE_REPORT_DESCRIPTION(390); HANDLE_REPORT_DESCRIPTION(391); HANDLE_REPORT_DESCRIPTION(392); HANDLE_REPORT_DESCRIPTION(393); 
		HANDLE_REPORT_DESCRIPTION(394); HANDLE_REPORT_DESCRIPTION(395); HANDLE_REPORT_DESCRIPTION(396); HANDLE_REPORT_DESCRIPTION(397);
		HANDLE_REPORT_DESCRIPTION(398); HANDLE_REPORT_DESCRIPTION(399); HANDLE_REPORT_DESCRIPTION(400); HANDLE_REPORT_DESCRIPTION(401);
		HANDLE_REPORT_DESCRIPTION(402); HANDLE_REPORT_DESCRIPTION(403); HANDLE_REPORT_DESCRIPTION(404); HANDLE_REPORT_DESCRIPTION(405);
		HANDLE_REPORT_DESCRIPTION(406); HANDLE_REPORT_DESCRIPTION(407); HANDLE_REPORT_DESCRIPTION(408); HANDLE_REPORT_DESCRIPTION(409);
		HANDLE_REPORT_DESCRIPTION(410); HANDLE_REPORT_DESCRIPTION(411); HANDLE_REPORT_DESCRIPTION(412); HANDLE_REPORT_DESCRIPTION(413);
		HANDLE_REPORT_DESCRIPTION(414); HANDLE_REPORT_DESCRIPTION(415); HANDLE_REPORT_DESCRIPTION(416); HANDLE_REPORT_DESCRIPTION(417);
		HANDLE_REPORT_DESCRIPTION(418); HANDLE_REPORT_DESCRIPTION(419); HANDLE_REPORT_DESCRIPTION(420); HANDLE_REPORT_DESCRIPTION(421);
		HANDLE_REPORT_DESCRIPTION(422); HANDLE_REPORT_DESCRIPTION(423); HANDLE_REPORT_DESCRIPTION(424); HANDLE_REPORT_DESCRIPTION(425);
		HANDLE_REPORT_DESCRIPTION(426); HANDLE_REPORT_DESCRIPTION(427); HANDLE_REPORT_DESCRIPTION(428); HANDLE_REPORT_DESCRIPTION(429);
		HANDLE_REPORT_DESCRIPTION(430); HANDLE_REPORT_DESCRIPTION(431); HANDLE_REPORT_DESCRIPTION(432); HANDLE_REPORT_DESCRIPTION(433);
		HANDLE_REPORT_DESCRIPTION(434); HANDLE_REPORT_DESCRIPTION(435); HANDLE_REPORT_DESCRIPTION(436); HANDLE_REPORT_DESCRIPTION(437);
		HANDLE_REPORT_DESCRIPTION(438); HANDLE_REPORT_DESCRIPTION(439); HANDLE_REPORT_DESCRIPTION(440); HANDLE_REPORT_DESCRIPTION(441);
		HANDLE_REPORT_DESCRIPTION(442); HANDLE_REPORT_DESCRIPTION(443); HANDLE_REPORT_DESCRIPTION(444); HANDLE_REPORT_DESCRIPTION(445);
		HANDLE_REPORT_DESCRIPTION(446); HANDLE_REPORT_DESCRIPTION(447); HANDLE_REPORT_DESCRIPTION(448); HANDLE_REPORT_DESCRIPTION(449);
		HANDLE_REPORT_DESCRIPTION(450); HANDLE_REPORT_DESCRIPTION(451); HANDLE_REPORT_DESCRIPTION(452); HANDLE_REPORT_DESCRIPTION(453);
		HANDLE_REPORT_DESCRIPTION(454); HANDLE_REPORT_DESCRIPTION(455);
		HANDLE_REPORT_DESCRIPTION(456); HANDLE_REPORT_DESCRIPTION(457); HANDLE_REPORT_DESCRIPTION(458); HANDLE_REPORT_DESCRIPTION(459);
		HANDLE_REPORT_DESCRIPTION(460); HANDLE_REPORT_DESCRIPTION(461); HANDLE_REPORT_DESCRIPTION(462); HANDLE_REPORT_DESCRIPTION(463);
		HANDLE_REPORT_DESCRIPTION(464); HANDLE_REPORT_DESCRIPTION(465); HANDLE_REPORT_DESCRIPTION(466); HANDLE_REPORT_DESCRIPTION(467);
		HANDLE_REPORT_DESCRIPTION(468); HANDLE_REPORT_DESCRIPTION(469); HANDLE_REPORT_DESCRIPTION(470); HANDLE_REPORT_DESCRIPTION(471);
		HANDLE_REPORT_DESCRIPTION(472); HANDLE_REPORT_DESCRIPTION(473); HANDLE_REPORT_DESCRIPTION(474); HANDLE_REPORT_DESCRIPTION(475);
		HANDLE_REPORT_DESCRIPTION(476); HANDLE_REPORT_DESCRIPTION(477); HANDLE_REPORT_DESCRIPTION(478); HANDLE_REPORT_DESCRIPTION(479);
		HANDLE_REPORT_DESCRIPTION(480); HANDLE_REPORT_DESCRIPTION(481); HANDLE_REPORT_DESCRIPTION(482); HANDLE_REPORT_DESCRIPTION(483);
		HANDLE_REPORT_DESCRIPTION(484); HANDLE_REPORT_DESCRIPTION(485); HANDLE_REPORT_DESCRIPTION(486); HANDLE_REPORT_DESCRIPTION(487);
		HANDLE_REPORT_DESCRIPTION(488); HANDLE_REPORT_DESCRIPTION(489); HANDLE_REPORT_DESCRIPTION(490); HANDLE_REPORT_DESCRIPTION(491);
		HANDLE_REPORT_DESCRIPTION(492); HANDLE_REPORT_DESCRIPTION(493); HANDLE_REPORT_DESCRIPTION(494); HANDLE_REPORT_DESCRIPTION(495);
		HANDLE_REPORT_DESCRIPTION(496); HANDLE_REPORT_DESCRIPTION(497); HANDLE_REPORT_DESCRIPTION(499); HANDLE_REPORT_DESCRIPTION(500);
		//(e.lally 2009-09-24) PLID 35053 - Report 504 has been depreciated
		HANDLE_REPORT_DESCRIPTION(501); HANDLE_REPORT_DESCRIPTION(502); HANDLE_REPORT_DESCRIPTION(503); /*HANDLE_REPORT_DESCRIPTION(504);*/
		HANDLE_REPORT_DESCRIPTION(505); HANDLE_REPORT_DESCRIPTION(506); HANDLE_REPORT_DESCRIPTION(507); HANDLE_REPORT_DESCRIPTION(508);
		//(e.lally 2009-08-28) PLID 35330 - Report 511 has been depreciated
		HANDLE_REPORT_DESCRIPTION(509); HANDLE_REPORT_DESCRIPTION(510); /*HANDLE_REPORT_DESCRIPTION(511);*/ HANDLE_REPORT_DESCRIPTION(512);
		HANDLE_REPORT_DESCRIPTION(513); HANDLE_REPORT_DESCRIPTION(514); HANDLE_REPORT_DESCRIPTION(515); HANDLE_REPORT_DESCRIPTION(516);
		//(e.lally 2009-08-28) PLID 35331 - Report 519 has been depreciated
		HANDLE_REPORT_DESCRIPTION(517); HANDLE_REPORT_DESCRIPTION(518); /*HANDLE_REPORT_DESCRIPTION(519);*/ HANDLE_REPORT_DESCRIPTION(520);
		HANDLE_REPORT_DESCRIPTION(521); HANDLE_REPORT_DESCRIPTION(522); HANDLE_REPORT_DESCRIPTION(523); HANDLE_REPORT_DESCRIPTION(524);
		//(e.lally 2009-09-08) PLID 35332 - Report 526 has been depreciated
		HANDLE_REPORT_DESCRIPTION(525); /*HANDLE_REPORT_DESCRIPTION(526);*/ HANDLE_REPORT_DESCRIPTION(527); HANDLE_REPORT_DESCRIPTION(528);
		HANDLE_REPORT_DESCRIPTION(529); HANDLE_REPORT_DESCRIPTION(530); HANDLE_REPORT_DESCRIPTION(531); HANDLE_REPORT_DESCRIPTION(532);
		HANDLE_REPORT_DESCRIPTION(533); HANDLE_REPORT_DESCRIPTION(534); HANDLE_REPORT_DESCRIPTION(535); HANDLE_REPORT_DESCRIPTION(536);
		HANDLE_REPORT_DESCRIPTION(537); HANDLE_REPORT_DESCRIPTION(538); HANDLE_REPORT_DESCRIPTION(539); HANDLE_REPORT_DESCRIPTION(540);
		HANDLE_REPORT_DESCRIPTION(541); HANDLE_REPORT_DESCRIPTION(542); HANDLE_REPORT_DESCRIPTION(543); HANDLE_REPORT_DESCRIPTION(544);
		HANDLE_REPORT_DESCRIPTION(545); HANDLE_REPORT_DESCRIPTION(546); HANDLE_REPORT_DESCRIPTION(547); HANDLE_REPORT_DESCRIPTION(548);
		HANDLE_REPORT_DESCRIPTION(549);	HANDLE_REPORT_DESCRIPTION(550); HANDLE_REPORT_DESCRIPTION(551); HANDLE_REPORT_DESCRIPTION(552); 
		HANDLE_REPORT_DESCRIPTION(553); HANDLE_REPORT_DESCRIPTION(554);	HANDLE_REPORT_DESCRIPTION(555); HANDLE_REPORT_DESCRIPTION(556); 
		HANDLE_REPORT_DESCRIPTION(557); HANDLE_REPORT_DESCRIPTION(558); HANDLE_REPORT_DESCRIPTION(559);	HANDLE_REPORT_DESCRIPTION(560);
		HANDLE_REPORT_DESCRIPTION(561);	HANDLE_REPORT_DESCRIPTION(562); HANDLE_REPORT_DESCRIPTION(563); HANDLE_REPORT_DESCRIPTION(564);
		HANDLE_REPORT_DESCRIPTION(565); HANDLE_REPORT_DESCRIPTION(566);	HANDLE_REPORT_DESCRIPTION(567); HANDLE_REPORT_DESCRIPTION(568);
		HANDLE_REPORT_DESCRIPTION(569); HANDLE_REPORT_DESCRIPTION(570);	HANDLE_REPORT_DESCRIPTION(571); HANDLE_REPORT_DESCRIPTION(572); 
		HANDLE_REPORT_DESCRIPTION(573); HANDLE_REPORT_DESCRIPTION(574); HANDLE_REPORT_DESCRIPTION(575); HANDLE_REPORT_DESCRIPTION(576); 
		HANDLE_REPORT_DESCRIPTION(577); HANDLE_REPORT_DESCRIPTION(578);	HANDLE_REPORT_DESCRIPTION(579); HANDLE_REPORT_DESCRIPTION(580);
		HANDLE_REPORT_DESCRIPTION(581); HANDLE_REPORT_DESCRIPTION(582); HANDLE_REPORT_DESCRIPTION(583); HANDLE_REPORT_DESCRIPTION(584);
		HANDLE_REPORT_DESCRIPTION(585); HANDLE_REPORT_DESCRIPTION(586); HANDLE_REPORT_DESCRIPTION(587); 
		HANDLE_REPORT_DESCRIPTION(588);	HANDLE_REPORT_DESCRIPTION(589); HANDLE_REPORT_DESCRIPTION(590); HANDLE_REPORT_DESCRIPTION(591); 
		HANDLE_REPORT_DESCRIPTION(592);	HANDLE_REPORT_DESCRIPTION(593); HANDLE_REPORT_DESCRIPTION(594); HANDLE_REPORT_DESCRIPTION(595);
		HANDLE_REPORT_DESCRIPTION(596);	HANDLE_REPORT_DESCRIPTION(597);	HANDLE_REPORT_DESCRIPTION(598);	HANDLE_REPORT_DESCRIPTION(599);
		HANDLE_REPORT_DESCRIPTION(600); HANDLE_REPORT_DESCRIPTION(601); HANDLE_REPORT_DESCRIPTION(602); HANDLE_REPORT_DESCRIPTION(603);
		HANDLE_REPORT_DESCRIPTION(604); HANDLE_REPORT_DESCRIPTION(605); HANDLE_REPORT_DESCRIPTION(606); HANDLE_REPORT_DESCRIPTION(607);
		HANDLE_REPORT_DESCRIPTION(608); HANDLE_REPORT_DESCRIPTION(609); HANDLE_REPORT_DESCRIPTION(610); HANDLE_REPORT_DESCRIPTION(611); 
		HANDLE_REPORT_DESCRIPTION(612); HANDLE_REPORT_DESCRIPTION(613); HANDLE_REPORT_DESCRIPTION(614); HANDLE_REPORT_DESCRIPTION(615);
		HANDLE_REPORT_DESCRIPTION(616);	/*HANDLE_REPORT_DESCRIPTION(617);*/	HANDLE_REPORT_DESCRIPTION(618); HANDLE_REPORT_DESCRIPTION(619); 
		HANDLE_REPORT_DESCRIPTION(620);	HANDLE_REPORT_DESCRIPTION(621);	HANDLE_REPORT_DESCRIPTION(622); HANDLE_REPORT_DESCRIPTION(623);	
		// (c.haag 2008-03-07 14:01) - PLID 29170 - Report 617 has been deprecated
		HANDLE_REPORT_DESCRIPTION(624);	HANDLE_REPORT_DESCRIPTION(625);HANDLE_REPORT_DESCRIPTION(626);HANDLE_REPORT_DESCRIPTION(627);	
		HANDLE_REPORT_DESCRIPTION(628);HANDLE_REPORT_DESCRIPTION(629);HANDLE_REPORT_DESCRIPTION(630);HANDLE_REPORT_DESCRIPTION(631);
		HANDLE_REPORT_DESCRIPTION(632);HANDLE_REPORT_DESCRIPTION(633);HANDLE_REPORT_DESCRIPTION(634);HANDLE_REPORT_DESCRIPTION(635);
		HANDLE_REPORT_DESCRIPTION(636);HANDLE_REPORT_DESCRIPTION(637);HANDLE_REPORT_DESCRIPTION(638);HANDLE_REPORT_DESCRIPTION(639);
		HANDLE_REPORT_DESCRIPTION(640);HANDLE_REPORT_DESCRIPTION(641);HANDLE_REPORT_DESCRIPTION(642);HANDLE_REPORT_DESCRIPTION(643);
		HANDLE_REPORT_DESCRIPTION(644);HANDLE_REPORT_DESCRIPTION(645);HANDLE_REPORT_DESCRIPTION(646);HANDLE_REPORT_DESCRIPTION(647);
		HANDLE_REPORT_DESCRIPTION(648);HANDLE_REPORT_DESCRIPTION(649);HANDLE_REPORT_DESCRIPTION(650);HANDLE_REPORT_DESCRIPTION(651);
		HANDLE_REPORT_DESCRIPTION(652);HANDLE_REPORT_DESCRIPTION(653);HANDLE_REPORT_DESCRIPTION(654);HANDLE_REPORT_DESCRIPTION(655);
		HANDLE_REPORT_DESCRIPTION(656);HANDLE_REPORT_DESCRIPTION(657);HANDLE_REPORT_DESCRIPTION(658);HANDLE_REPORT_DESCRIPTION(659);
		HANDLE_REPORT_DESCRIPTION(660);HANDLE_REPORT_DESCRIPTION(661);HANDLE_REPORT_DESCRIPTION(662);HANDLE_REPORT_DESCRIPTION(663);
		HANDLE_REPORT_DESCRIPTION(664);HANDLE_REPORT_DESCRIPTION(665);HANDLE_REPORT_DESCRIPTION(666);HANDLE_REPORT_DESCRIPTION(667);
		HANDLE_REPORT_DESCRIPTION(668);HANDLE_REPORT_DESCRIPTION(669);HANDLE_REPORT_DESCRIPTION(670);HANDLE_REPORT_DESCRIPTION(671);
		HANDLE_REPORT_DESCRIPTION(672);HANDLE_REPORT_DESCRIPTION(673);HANDLE_REPORT_DESCRIPTION(674);HANDLE_REPORT_DESCRIPTION(675);
		HANDLE_REPORT_DESCRIPTION(676);HANDLE_REPORT_DESCRIPTION(677);HANDLE_REPORT_DESCRIPTION(678);HANDLE_REPORT_DESCRIPTION(679);
		HANDLE_REPORT_DESCRIPTION(680);HANDLE_REPORT_DESCRIPTION(681);HANDLE_REPORT_DESCRIPTION(682);HANDLE_REPORT_DESCRIPTION(683);
		HANDLE_REPORT_DESCRIPTION(684);HANDLE_REPORT_DESCRIPTION(685);HANDLE_REPORT_DESCRIPTION(686);HANDLE_REPORT_DESCRIPTION(687);
		HANDLE_REPORT_DESCRIPTION(689);
		//HANDLE_REPORT_DESCRIPTION(688);HANDLE_REPORT_DESCRIPTION(691);
		// (j.gruber 2010-01-18 11:03) - PLID 36929 - took out reportID 690
		/*HANDLE_REPORT_DESCRIPTION(690);*/HANDLE_REPORT_DESCRIPTION(692);
		HANDLE_REPORT_DESCRIPTION(693);HANDLE_REPORT_DESCRIPTION(694);HANDLE_REPORT_DESCRIPTION(695);
		HANDLE_REPORT_DESCRIPTION(696);HANDLE_REPORT_DESCRIPTION(697);
		HANDLE_REPORT_DESCRIPTION(698);HANDLE_REPORT_DESCRIPTION(699);
		HANDLE_REPORT_DESCRIPTION(700);HANDLE_REPORT_DESCRIPTION(702);HANDLE_REPORT_DESCRIPTION(703);
		HANDLE_REPORT_DESCRIPTION(704);HANDLE_REPORT_DESCRIPTION(705);
		HANDLE_REPORT_DESCRIPTION(706);HANDLE_REPORT_DESCRIPTION(707);HANDLE_REPORT_DESCRIPTION(708);HANDLE_REPORT_DESCRIPTION(709);
		HANDLE_REPORT_DESCRIPTION(710);HANDLE_REPORT_DESCRIPTION(711);HANDLE_REPORT_DESCRIPTION(712);HANDLE_REPORT_DESCRIPTION(713);
		HANDLE_REPORT_DESCRIPTION(714);HANDLE_REPORT_DESCRIPTION(715);HANDLE_REPORT_DESCRIPTION(716);HANDLE_REPORT_DESCRIPTION(717); 
		HANDLE_REPORT_DESCRIPTION(718);HANDLE_REPORT_DESCRIPTION(719);HANDLE_REPORT_DESCRIPTION(720);HANDLE_REPORT_DESCRIPTION(721);
		HANDLE_REPORT_DESCRIPTION(722);HANDLE_REPORT_DESCRIPTION(723);HANDLE_REPORT_DESCRIPTION(724);HANDLE_REPORT_DESCRIPTION(725);
		HANDLE_REPORT_DESCRIPTION(726);HANDLE_REPORT_DESCRIPTION(727);HANDLE_REPORT_DESCRIPTION(728);HANDLE_REPORT_DESCRIPTION(729);
		HANDLE_REPORT_DESCRIPTION(730);HANDLE_REPORT_DESCRIPTION(731);HANDLE_REPORT_DESCRIPTION(732);HANDLE_REPORT_DESCRIPTION(735);
		HANDLE_REPORT_DESCRIPTION(733);HANDLE_REPORT_DESCRIPTION(734);HANDLE_REPORT_DESCRIPTION(736);HANDLE_REPORT_DESCRIPTION(738);
		HANDLE_REPORT_DESCRIPTION(739);HANDLE_REPORT_DESCRIPTION(740);HANDLE_REPORT_DESCRIPTION(741);HANDLE_REPORT_DESCRIPTION(742);
		HANDLE_REPORT_DESCRIPTION(743);HANDLE_REPORT_DESCRIPTION(744);HANDLE_REPORT_DESCRIPTION(745);HANDLE_REPORT_DESCRIPTION(746);
		HANDLE_REPORT_DESCRIPTION(747);HANDLE_REPORT_DESCRIPTION(748);HANDLE_REPORT_DESCRIPTION(749);HANDLE_REPORT_DESCRIPTION(750);
		HANDLE_REPORT_DESCRIPTION(751);HANDLE_REPORT_DESCRIPTION(752);HANDLE_REPORT_DESCRIPTION(753);HANDLE_REPORT_DESCRIPTION(754);
		HANDLE_REPORT_DESCRIPTION(755); HANDLE_REPORT_DESCRIPTION(756); HANDLE_REPORT_DESCRIPTION(757); HANDLE_REPORT_DESCRIPTION(758);
		HANDLE_REPORT_DESCRIPTION(759); HANDLE_REPORT_DESCRIPTION(760); HANDLE_REPORT_DESCRIPTION(761); HANDLE_REPORT_DESCRIPTION(762); HANDLE_REPORT_DESCRIPTION(763);
	default:
		return _T("");
		break;
	}
}

CString CReportInfo::DefGetProviderFilter(long nSubLevel, long nSubRepNum) const
{
	CString strAns;
	if (nProvider > 0) {
		strAns.Format( "{%s.ProvID} = %d ", strRecordSource, nProvider);
		return strAns;
	} else if(nProvider == -2) {
		strAns.Format( "({%s.ProvID} Is Null OR {%s.ProvID} = -1)", strRecordSource, strRecordSource);
		return strAns;
	}
	else if(nProvider == -3) {
		strAns.Format("{%s.ProvID} In (", strRecordSource);
		CString strPart;
		for(int i=0; i < m_dwProviders.GetSize(); i++) {
			strPart.Format("%li, ", (long)m_dwProviders.GetAt(i));
			strAns += strPart;
		}
		strAns = strAns.Left(strAns.GetLength()-2) + ")";
		return strAns;
	}
	else {
		return "";
	}
}

CString CReportInfo::DefGetLocationFilter(long nSubLevel, long nSubRepNum) const
{
	//(e.lally 2008-09-05) PLID 6780 - Added the ability to filter on multiple locations
	CString strAns;
	if (nLocation > 0) {
		strAns.Format( "{%s.LocID} = %d ", strRecordSource, nLocation);
		return strAns;
	} else if(nLocation == -2) {
		strAns.Format( "({%s.LocID} Is Null OR {%s.LocID} = -1)", strRecordSource, strRecordSource);
		return strAns;
	}
	else if(nLocation == -3) {
		strAns.Format("{%s.LocID} In (", strRecordSource);
		CString strPart;
		for(int i=0; i < m_dwLocations.GetSize(); i++) {
			strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
			strAns += strPart;
		}
		strAns = strAns.Left(strAns.GetLength()-2) + ")";
		return strAns;
	}else {
		return "";
	}
}

CString CReportInfo::DefGetPatientFilter(long nSubLevel, long nSubRepNum) const
{
	if (nPatient > 0) {
		CString strAns;
		strAns.Format(_T("{%s.PatID} = %d"), strRecordSource, nPatient);
		return strAns;
	} else {
		return _T("");
	}
}

CString CReportInfo::DefGetDateFilterField(long nSubLevel, long nSubRepNum) const
{
	if(nDateFilter < 1) {
		return "";
	}
	else return GetDateField(nDateFilter);
}

CString CReportInfo::DefGetDateFilter(long nSubLevel, long nSubRepNum) const
{
	//DRT - bOneDate fields always have a date, and some of them are assuming
	//		an nDateRange of -1 is a-ok.  Non-bOneDate reports are automatically
	//		"fixed" to set nDateRange of -1 to 1.  So if it's a bOneDate, automatically
	//		apply the date filter to it.
	if (bOneDate || nDateRange > 0) {
		// Get some values used in the daterange filter
		CString strAns;
		COleDateTime dtToPlusOne = DateTo + COleDateTimeSpan(1, 0, 0, 0);
		CString strDateType = GetDateFilterField(nSubLevel, nSubRepNum);

		if (bOneDate) {
			// Decide what comparison operator to use
			if (nOneDateOption == 0) {
				// "date <= DateTo" is really "date < DateTo+1"
				strAns.Format("({%s.%s} < #%d-%02d-%02d#)", strRecordSource, strDateType, 
	 				dtToPlusOne.GetYear(), dtToPlusOne.GetMonth(), dtToPlusOne.GetDay());
				return strAns;
			} else {
				// "date = DateTo" is really "date >= DateTo && date < DateTo+1"
				strAns.Format("({%s.%s} >= #%d-%02d-%02d# AND {%s.%s} < #%d-%02d-%02d#)", strRecordSource, 
					strDateType, DateTo.GetYear(), DateTo.GetMonth(), DateTo.GetDay(), strRecordSource, 
					strDateType, dtToPlusOne.GetYear(), dtToPlusOne.GetMonth(), dtToPlusOne.GetDay());
				return strAns;
			}
		} else {
			// "date >= DateFrom && date <= DateTo" is really "date >= DateFrom && date < DateTo+1"
			strAns.Format("({%s.%s} >= #%d-%02d-%02d# AND {%s.%s} < #%d-%02d-%02d#)", strRecordSource, 
				strDateType, DateFrom.GetYear(), DateFrom.GetMonth(), DateFrom.GetDay(), strRecordSource, 
				strDateType, dtToPlusOne.GetYear(), dtToPlusOne.GetMonth(), dtToPlusOne.GetDay());
			return strAns;
		}
	} else {
		return _T("");
	}
}

CString CReportInfo::DefGetExtraFilter(long nSubLevel, long nSubRepNum) const
{
	if (bExtended && saExtraValues.GetSize()) {
		CString strAns = "(";
		for(int i = 0; i < saExtraValues.GetSize(); i++) {
			CString strPart;
			// See if strExtraValue contains a known operator
			if (saExtraValues[i].Find("=") != -1 || 
				 saExtraValues[i].Find(">") != -1 || 
				 saExtraValues[i].Find("<") != -1) {
				// We found an operator so generate the filter based on strExtraValue
				strPart.Format("{%s.%s} %s OR ", strRecordSource, GetExtraField(), saExtraValues[i]);
				strAns += strPart;
			} else {
				// We did not find an operator so assume "equal"
				strPart.Format("{%s.%s} = %s OR ", strRecordSource, GetExtraField(), saExtraValues[i]);
				strAns += strPart;
			}
		}
		ASSERT(strAns.Right(4) == " OR ");
		strAns = strAns.Left(strAns.GetLength()-4) + ")";
		return strAns;
	} else {
		// No extra filter
		return _T("");
	}
}

CString CReportInfo::DefGetExternalFilter(long nSubLevel, long nSubRepNum) const
{
	// Default implementation simply returns 
	// strExternalFilter if bExternal is set
	if (bExternal) {
		return strExternalFilter;
	} else {
		return _T("");
	}
}

CString CReportInfo::DefGetGroupFilter(long nSubLevel, long nSubRepNum) const
{
	// Default implementation simply returns strGroupFilter (it may be empty)
	return strGroupFilter;
}


CString CReportInfo::DefGetFilter(long nSubLevel, long nSubRepNum) const
{
	// Default implementation simply returns strGroupFilter (it may be empty)
	return strFilterString;
}

/*EBuiltInObjectIDs CReportInfo::DefGetBuiltInObjectID() const
{
	return m_bioBuiltInObjectID;
}*/


CString CReportInfo::DefGetExtraField() const
{
	// Default implementation simply returns strExtraField (it may be empty)
	return strExtraField;
}

//PLID 19451 - I needed to put the AddPartToClause fucntion in globalreportutils instead of here

CString CReportInfo::GetFilter(long nSubLevel, long nSubRepNum) const
{
	CString strFilter;

	AddPartToClause(strFilter, GetProviderFilter(nSubLevel, nSubRepNum));
	AddPartToClause(strFilter, GetLocationFilter(nSubLevel, nSubRepNum));
	AddPartToClause(strFilter, GetPatientFilter(nSubLevel, nSubRepNum));
	AddPartToClause(strFilter, GetDateFilter(nSubLevel, nSubRepNum));
	AddPartToClause(strFilter, GetExtraFilter(nSubLevel, nSubRepNum));
	// (z.manning 2009-07-18 13:23) - PLID 22054 - The external filter for the time sched productivity
	// by scheduler template report is handled manually.
	if(nID != 670) {
		AddPartToClause(strFilter, GetExternalFilter(nSubLevel, nSubRepNum));
	}
	AddPartToClause(strFilter, GetGroupFilter(nSubLevel, nSubRepNum));
	AddPartToClause(strFilter, GetFilterFilter(nSubLevel, nSubRepNum));

	return strFilter;
}

CString CReportInfo::GetBaseFilter() const
{
	return GetFilter(0, 0);
}

// (j.gruber 2010-12-01 10:31) - PLID 41540 - added a map of excluded patientIDs
// (r.gonet 12/18/2012) - PLID 53629 - Pass along the statement output types, normally ConfigRT values.
// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
const void CReportInfo::WriteToHistory(long nReportInfoID, long nSubLevel, long nSubRepNum, _RecordsetPtr pRS, NxOutputType nxOutType, CMap<long, long, long, long> *pmapExcludedPatientIDs /*= NULL*/, long nStatementType, long nOutputType, long nCategoryType)
{

	switch (nReportInfoID) {

		case 169:
		case 234: 
		case 353:
		case 354:
		case 434:
		case 436:
		
				
			//statements
			// (r.gonet 12/18/2012) - PLID 53629 - Pass along the statement output types, normally ConfigRT values.
			// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
			StatementWriteToHistory(pRS, 0, nxOutType, pmapExcludedPatientIDs, nStatementType, nOutputType, nCategoryType);
		break;
		case 337:
		case 338:
		case 355:
		case 356:
		// (j.gruber 2008-06-09 10:24) - PLID 29399 - these are by last sent
		
		case 435:
		case 437:
			//statements by location
			// (r.gonet 12/18/2012) - PLID 53629 - Pass along the statement output types, normally ConfigRT values.
			// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
			StatementWriteToHistory(pRS, 1, nxOutType, NULL, nStatementType, nOutputType, nCategoryType);
		break;
		case 483:
		case 484:
		case 485:
		case 486:
			//statements by location
			// (r.gonet 12/18/2012) - PLID 53629 - Pass along the statement output types, normally ConfigRT values.
			// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
			StatementWriteToHistory(pRS, 2, nxOutType, NULL, nStatementType, nOutputType, nCategoryType);
		break;


		case 365:  {//Billing Followup
			CString strNote = "Tracer Form Printed";
			TracerLetterWriteToHistory(pRS, nxOutType, strNote);
			//DefaultWriteToHistory(pRS, nxOutType, strNote);
				   }
	    break;
		
		//(e.lally 2010-09-10) PLID 40488 - Write to mail sent history when Wellness information is Printed
		case 665: //Wellness Guidelines
			WellnessWriteToHistory(pRS, nxOutType, "Wellness Educational Guidelines Printed");
		break;
		case 666: //Wellness References
			WellnessWriteToHistory(pRS, nxOutType, "Wellness Educational References Printed");
		break;

		// (j.gruber 2011-10-11 14:32) - PLID 45916 - Special Handling for the Affiliate Physician Report
		case 714:
			{
				////we already know they only filtered on 2 since otherwise we wouldn't be here
				if (nxOutType == NXR_PRINT) {
					if (IDYES == MsgBox(MB_YESNO, "You have filtered on only the Ready to Pay Status, would you like to change the status on all of these bills to paid?")) {
						UpdateAffiliateStatusToPaid(pRS);
					}
				}
			}
		break;


		default:
			//there really is no default
		break;
	}
				

}

typedef CMap<long, long, CString, CString> CMapLongToCString;

// (j.jones 2014-08-04 11:57) - PLID 63150 - added a modular function for saving to history
const void CReportInfo::WriteTempTableToHistory(CString strTempTableName, long nCategoryID)
{
	// (r.goldschmidt 2014-08-05 16:24) - PLID 62717 - create category id
	_variant_t varCategoryID = g_cvarNull;
	if (nCategoryID > 0) {
		varCategoryID = (long)nCategoryID;
	}

	// (j.jones 2008-09-04 17:15) - PLID 30288 - supported MailSentNotesT
	// (c.haag 2010-01-27 12:10) - PLID 36271 - Use GetDate() for the service date; not COleDateTime::GetCurrentTime
	// (j.armen 2014-01-30 10:38) - PLID 55225 - Idenitate MailSent
	// (j.jones 2014-08-04 11:44) - PLID 63150 - parameterized
	CParamSqlBatch sqlBatch;
	sqlBatch.Add("SET NOCOUNT ON");
	sqlBatch.Declare("DECLARE @nMailSentID int");
	sqlBatch.Add("SET @nMailSentID = (SELECT COALESCE(MAX(MailID), 0) + 1 FROM MailSent WITH(UPDLOCK, HOLDLOCK))");
	sqlBatch.Declare("DECLARE @nMailBatchID int");
	sqlBatch.Add("SET @nMailBatchID = (SELECT COALESCE(MAX(MailBatchID), 0) + 1 FROM MailSent WITH(UPDLOCK, HOLDLOCK))");
	sqlBatch.Add("SET IDENTITY_INSERT MailSent ON");
	sqlBatch.Add("INSERT INTO MailSent (MailID, PersonID, Selection, PathName, Subject, Sender, Date, Location, MailBatchId, ServiceDate, CategoryID) "
		" SELECT (@nMailSentID + RowNumber), ID, '', '', '', {STRING}, getDate(), {INT}, @nMailBatchID, getDate(), {VT_I4}  FROM {CONST_STRING}",
		GetCurrentUserName(), GetCurrentLocationID(), varCategoryID, strTempTableName);
	sqlBatch.Add("SET IDENTITY_INSERT MailSent OFF");
	sqlBatch.Add("INSERT INTO MailSentNotesT (MailID, Note) "
		" SELECT (@nMailSentID + RowNumber), Note FROM {CONST_STRING}", strTempTableName);
	sqlBatch.Add("SET NOCOUNT OFF");

	// (j.jones 2014-08-04 11:41) - PLID 63150 - get the list of records we just created
	// (b.savon 2015-01-15 13:06) - PLID 64601 - Made this a join so the query doesn't timeout on large recordsets
	sqlBatch.Add("SELECT PersonID, MailID FROM MailSent INNER JOIN {CONST_STRING} AS TempQ ON MailSent.MailID = (TempQ.RowNumber + @nMailSentID)", strTempTableName);

	// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
	NxAdo::PushPerformanceWarningLimit ppw(-1);
	_RecordsetPtr rs = sqlBatch.CreateRecordset(GetRemoteData());
	while (!rs->eof) {
		// (j.jones 2014-08-04 11:37) - PLID 63150 - this now sends an Ex tablechecker, we know IsPhoto is always false here
		CClient::RefreshMailSentTable(AdoFldLong(rs, "PersonID"), AdoFldLong(rs, "MailID"));
		rs->MoveNext();
	}
	rs->Close();
}

// (j.gruber 2010-12-01 10:32) - PLID 41540 - added map of excluded patient IDs
// (r.gonet 12/18/2012) - PLID 53629 - Pass along the statement output types, normally ConfigRT values.
// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass category type also
// (r.goldschmidt 2014-08-04 09:52) - PLID 62851 - Add the patient’s balance to the note
const void CReportInfo::StatementWriteToHistory(_RecordsetPtr pRS, long nType, NxOutputType nxOutType, CMap<long, long, long, long> *mapExcludedPatientIDs /*= NULL*/, long nStatementType, long nOutputType, long nCategoryType)
{
	try {

		//determine what the output type is and what we want to save to history on
		// (r.gonet 12/18/2012) - PLID 53629 - Try to get the output types from when the report was run
		// rather than at the present moment. They may have changed in between. If -1, then they are not set,
		// then just use the properties.
		long nStyle = nStatementType != -1 ? nStatementType : GetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>");
		//PLID 18403 - give them the preference for how to save it
		long nOutputTypePref = nOutputType != -1 ? nOutputType : GetRemotePropertyInt("SttmntSendToHistory", 0, 0, "<None>", true);
		// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Get category type also
		long nCategoryTypePref = nCategoryType != -1 ? nCategoryType : GetRemotePropertyInt("SttmntSendToHistoryCategory", 0, 0, "<None>", true);

		//we aren't changing always running it on the preview for e-statement
		if (nStyle == 3) {
			if (nxOutType != NXR_RUN) {
				return;
			}
		}
		else {

			//check to see what they want
			if (nOutputTypePref == 2) {

				//they never want it
				return;
			}

			if (nOutputTypePref == 0) {

				//only on print
				if (nxOutType != NXR_PRINT) {
					return;
				}
			}
			else if (nOutputTypePref == 1) {

				//only on preview
				if (nxOutType != NXR_RUN) {
					return;
				}
			}
		}

		/*if (nxOutType == NXR_RUN) {

			//we only do it on the run if they are doing ebilling format
			if (nStyle != 3) {
				return;
			}
		}
		else {
			
			//For now, the statement is only writing to history when it prints
			if (nxOutType != NXR_PRINT) {

				return;
			}
		}*/

		/*This is code that allows the report to export so that we will be able to save it and view it from the history
		The problem with this code is that the name field in the destination options struct is not getting taken correctly from the 
		Crystal code which is causing the export dialog to pop up and prompt for the name of the file.   The crystal version of getting the same info is
		the GetExportOptions function.   This creates the file on the hard drive if it isn't there yet.  It also seems to return a handle 
		to the file somehow.   It also seems to put the same series of bytes in memory after the path of the report it is exporting to
		These are 0D F0 AD BA.  We tried to stick these in after our stuff but it didn't work.   However we changed the name of the file that 
		their funcion made in memory (ie: we got the export dialog, selected a file, then in the debugger we looked at the memory where it was stored and changed the
		file it was pointing to.)  and this worked in creating the new file.   So, more investigation is needed.
		-JMM 11/05/2003*/


/*		struct UXDDiskOptions {
			WORD structsize;
			char FAR fileName[0];
		};

		UXDDiskOptions *dskOpts = (UXDDiskOptions *)(new BYTE[5000]);//UXDDiskOptions;

		dskOpts->structsize = 5000;//sizeof(dskOpts);
		if (!DoesExist(GetReportsPath() ^ "HistoricReports")) {
			CreatePath(GetReportsPath() ^ "HistoricReports");
		}
		
		CString strName = GetReportsPath() ^ "HistoricReports" ^ infReport.strReportFile + ".rpt";
		strcpy(dskOpts->fileName, "C:\\Pracstation\\Reports\\HistoricReports\\TestReport.rpt");
		(*next) = handle to the above file;
		//dskOpts->fileName = (_bstr_t)strName;
		CRPEExportOptions expOpts("u2fcr", 0, 0, "u2ddisk", 0, dskOpts);
		expOpts.m_nFormatOptionsBytes = 0;
		expOpts.m_nDestinationOptionsBytes = 70;
		RepJob->GetExportOptions(&expOpts);
		RepJob->ExportTo(&expOpts);
*/

		// (r.goldschmidt 2014-09-24 17:15) - PLID 62851 - struct to hold map data: Note, Patient Balance, and Responsible Party ID
		struct MapData {
			CString strNote;
			COleCurrency cyBalance;
			long nRespID;
		};

		switch (nType) {

			case 0:
				{
				//statements by patients

				// (r.goldschmidt 2014-09-24 17:34) - PLID 62851 - changed to MapData map to calculate balance and limit calculation to one resp party
				CMap<long, long, MapData, MapData> mapPatients;

				long nPersonID, nLineItemType, nRespPartyID;
				COleCurrency cyPatientCharge, cyPatientPayment;

				//we have to move the recordset back to the beginning because they can potentially run this twice and 
				// the second time around it would already be at the end if we don't move it to the beginning right here
				//**we have to check for both bof and eof to find out whether the recordset is empty because its always going to
				//be eof since we already looped through it*//

				if (!(pRS->eof && pRS->bof)) {
					//move it back to the beginning of the file
					pRS->MoveFirst();
				}

				//loop through the recordset and add to the map
				while (! pRS->eof) {
					nPersonID = AdoFldLong(pRS, "PatID");
					nLineItemType = AdoFldLong(pRS, "Type");
					nRespPartyID = AdoFldLong(pRS, "RespID", -1);
					
					// set Patient Charge
					cyPatientCharge = COleCurrency(0, 0);
					if (nLineItemType == 10) {
						cyPatientCharge = AdoFldCurrency(pRS, "Total") - AdoFldCurrency(pRS, "Insurance");
					}

					// set Patient Payment
					cyPatientPayment = COleCurrency(0, 0);
					if (nLineItemType == 1 || nLineItemType == 2 || nLineItemType == 3){
						if (AdoFldCurrency(pRS, "Insurance") == COleCurrency(0, 0)){
							cyPatientPayment = AdoFldCurrency(pRS, "Total");
						}
					}
					
					
					// (j.gruber 2010-12-01 10:34) - PLID 41540 - exclude excluded patients
					long nBlank;
					if (mapExcludedPatientIDs == NULL || mapExcludedPatientIDs->Lookup(nPersonID, nBlank) == 0) {

						//check to see if the patient is already in the map
						CString strNote;
						if (nStyle == 3) {
							strNote.Format("E-Statement Exported");
						}
						else {
							if (nxOutType == NXR_RUN) {
								strNote.Format("Patient Statement Run");
							}
							else {
								strNote.Format("Patient Statement Printed");
							}
						}
						
						long nResult;
						MapData mdCurrent;
						nResult = mapPatients.Lookup(nPersonID, mdCurrent);

						if (nResult == 0) {
							// it isn't in the list yet, so add it
							mdCurrent.strNote = strNote;
							mdCurrent.cyBalance = cyPatientCharge - cyPatientPayment;
							mdCurrent.nRespID = nRespPartyID;
							mapPatients.SetAt(nPersonID, mdCurrent);	
						}
						else {
							// it is in the list, so adjust currency value
							// (r.goldschmidt 2014-09-24 17:23) - PLID 62851 - ONLY IF the responsible party is a match
							if (mdCurrent.nRespID == nRespPartyID){
								mdCurrent.cyBalance = mdCurrent.cyBalance + cyPatientCharge - cyPatientPayment;
								mapPatients.SetAt(nPersonID, mdCurrent);
					}	
						}	
					}	

					pRS->MoveNext();
						
					
				}

				//PLID 62851 - adjust note to be added to include the patient balance
				//  Build a new map used to write to history
				CMapLongToCString mapPatientsTemp;
				POSITION pos = mapPatients.GetStartPosition();
				long nKey;
				MapData mdCurrent;
				CString strNote;
				while (pos != NULL){
					mapPatients.GetNextAssoc(pos, nKey, mdCurrent);
					strNote = mdCurrent.strNote + ". Reported Patient Balance: " + FormatCurrencyForInterface(mdCurrent.cyBalance);
					mapPatientsTemp.SetAt(nKey, strNote);
				}

				//ok, now we have the recordset in a map, now we just have to insert the mail ID's and insert the records
				//TODO: figure out how to do this
				
				//check to see if they can support XML
				// (a.walling 2010-08-27 17:18) - PLID 39965 - We require at least SQL 2000 now.
				{
					
					//create a temp table out of the XML
					CString strTempTableName;
					strTempTableName = CreateWriteToHistoryTempTable_80(&mapPatientsTemp);

					//Now do the Insert
					// (j.jones 2014-08-04 11:59) - PLID 63150 - saving to history is now a modular function
					// (r.goldschmidt 2014-08-05 16:24) - PLID 62717 - create category id
					WriteTempTableToHistory(strTempTableName, nCategoryTypePref);
					}
					

				}
			break;

	

			case 1: {  //by location

				
				// (j.gruber 2008-06-09 10:02) - PLID 29399 - changed to a CString map so we could add one entry for each patient/location combination
				// (r.goldschmidt 2014-09-24 17:34) - PLID 62851 - changed to MapData map to calculate balance and limit calculation to one resp party
				CMap<CString, LPCTSTR, MapData, MapData> mapPatients;

				
				long nPersonID, nLocationID, nLineItemType, nRespPartyID;
				CString strLocation;
				COleCurrency cyPatientCharge, cyPatientPayment;

				//we have to move the recordset back to the beginning because they can potentially run this twice and 
				// the second time around it would already be at the end if we don't move it to the beginning right here
				//**we have to check for both bof and eof to find out whether the recordset is empty because its always going to
				//be eof since we already looped through it*//

				if (!(pRS->eof && pRS->bof)) {
					//move it back to the beginning of the file
					pRS->MoveFirst();
				}
				
							
				//loop through the recordset and add to the map
				while (! pRS->eof) {
					
					nPersonID = AdoFldLong(pRS, "PatID");
					strLocation = AdoFldString(pRS, "PracName", "{No Location}");
					nLocationID = AdoFldLong(pRS, "LocId", -1);
					nLineItemType = AdoFldLong(pRS, "Type");
					nRespPartyID = AdoFldLong(pRS, "RespID", -1);

					// set Patient Charge
					cyPatientCharge = COleCurrency(0, 0);
					if (nLineItemType == 10) {
						cyPatientCharge = AdoFldCurrency(pRS, "Total") - AdoFldCurrency(pRS, "Insurance");
					}

					// set Patient Payment
					cyPatientPayment = COleCurrency(0, 0);
					if (nLineItemType == 1 || nLineItemType == 2 || nLineItemType == 3){
						if (AdoFldCurrency(pRS, "Insurance") == COleCurrency(0, 0)){
							cyPatientPayment = AdoFldCurrency(pRS, "Total");
						}
					}

					CString strPatLocID;
					strPatLocID.Format("%li - %li", nPersonID, nLocationID);

					//check to see if the patient is already in the map
					CString strNote;
					long nResult;
					MapData mdCurrent;
					nResult = mapPatients.Lookup(strPatLocID, mdCurrent);

					if (nResult == 0) {
						// it isn't in the list yet, so add it
						CString str;
						// (r.goldschmidt 2016-01-11 12:41) - PLID 67839 - e-statements may now be run by location
						if (nStyle == 3) {
							str.Format("E-Statement By Location Exported For %s", strLocation);
						}
						else {
							if (nxOutType == NXR_RUN) {
								str.Format("Patient Statement By Location Run For %s", strLocation);
							}
							else {
								str.Format("Patient Statement By Location Printed For %s", strLocation);
							}
						}
						mdCurrent.strNote = str;
						mdCurrent.cyBalance = cyPatientCharge - cyPatientPayment;
						mdCurrent.nRespID = nRespPartyID;
						mapPatients.SetAt(strPatLocID, mdCurrent);
					}
						else {
						// it is in the list, so adjust currency value
						// (r.goldschmidt 2014-09-24 17:23) - PLID 62851 - ONLY IF the responsible party is a match
						if (mdCurrent.nRespID == nRespPartyID){
							mdCurrent.cyBalance = mdCurrent.cyBalance + cyPatientCharge - cyPatientPayment;
							mapPatients.SetAt(strPatLocID, mdCurrent);
						}
						}
						
						pRS->MoveNext();
					
				}
						
				//PLID 62851 - adjust note to be added to include the patient balance
				//  Build a new map used to write to history
				CMap<CString, LPCTSTR, CString, CString> mapPatientsTemp;
				POSITION pos = mapPatients.GetStartPosition();
				CString strKey;
				MapData mdCurrent;
				CString strNote;
				while (pos != NULL){
					mapPatients.GetNextAssoc(pos, strKey, mdCurrent);
					strNote = mdCurrent.strNote + ". Reported Patient Balance: " + FormatCurrencyForInterface(mdCurrent.cyBalance);
					mapPatientsTemp.SetAt(strKey, strNote);
				}

				//ok, now we have the recordset in a map, now we just have to insert the mail ID's and insert the records
				
				//check to see if they can support XML
				// (a.walling 2010-08-27 17:18) - PLID 39965 - We require at least SQL 2000 now.
				{
					
					//create a temp table out of the XML
					CString strTempTableName;
					strTempTableName = CreateWriteToHistoryTempTable_80WithStringKey(&mapPatientsTemp);

					// (j.jones 2014-08-04 11:59) - PLID 63150 - saving to history is now a modular function
					// (r.goldschmidt 2014-08-05 16:24) - PLID 62717 - create category id
					WriteTempTableToHistory(strTempTableName, nCategoryTypePref);
					}
					

				}
			break;

			case 2: {  //by provider

				// (j.gruber 2008-06-09 11:22) - PLID 29558 - make the statement by provider print to history for each doctor
				// (r.goldschmidt 2014-09-24 17:34) - PLID 62851 - changed to MapData map to calculate balance and limit calculation to one resp party
				CMap<CString, LPCTSTR, MapData, MapData> mapPatients;

				
				long nPersonID, nProviderID, nLineItemType, nRespPartyID;
				CString strProvider;
				COleCurrency cyPatientCharge, cyPatientPayment;

				//we have to move the recordset back to the beginning because they can potentially run this twice and 
				// the second time around it would already be at the end if we don't move it to the beginning right here
				//**we have to check for both bof and eof to find out whether the recordset is empty because its always going to
				//be eof since we already looped through it*//
				if (!(pRS->eof && pRS->bof)) {
					//move it back to the beginning of the file
					pRS->MoveFirst();
				}
				
				//loop through the recordset and add to the map
				while (! pRS->eof) {
					
					nPersonID = AdoFldLong(pRS, "PatID");
					strProvider = AdoFldString(pRS, "DocName", "{No Provider}");
					nProviderID = AdoFldLong(pRS, "ProvID", -1);
					nLineItemType = AdoFldLong(pRS, "Type");
					nRespPartyID = AdoFldLong(pRS, "RespID", -1);

					// set Patient Charge
					cyPatientCharge = COleCurrency(0, 0);
					if (nLineItemType == 10) {
						cyPatientCharge = AdoFldCurrency(pRS, "Total") - AdoFldCurrency(pRS, "Insurance");
					}

					// set Patient Payment
					cyPatientPayment = COleCurrency(0, 0);
					if (nLineItemType == 1 || nLineItemType == 2 || nLineItemType == 3){
						if (AdoFldCurrency(pRS, "Insurance") == COleCurrency(0, 0)){
							cyPatientPayment = AdoFldCurrency(pRS, "Total");
						}
					}

					CString strPatProvID;
					strPatProvID.Format("%li - %li", nPersonID, nProviderID);

					//check to see if the patient is already in the map
					long nResult;
					MapData mdCurrent;
					nResult = mapPatients.Lookup(strPatProvID, mdCurrent);

					if (nResult == 0) {
						// it isn't in the list yet, so add it
						CString str;
						strProvider.TrimRight();
						if (nxOutType == NXR_RUN) {
							str.Format("Patient Statement By Provider Run for %s", strProvider);
						}
						else {
							str.Format("Patient Statement By Provider Printed for %s", strProvider);
						}
						mdCurrent.strNote = str;
						mdCurrent.cyBalance = cyPatientCharge - cyPatientPayment;
						mdCurrent.nRespID = nRespPartyID;
						mapPatients.SetAt(strPatProvID, mdCurrent);
					}
						else {
						// it is in the list, so adjust currency value
						// (r.goldschmidt 2014-09-24 17:23) - PLID 62851 - ONLY IF the responsible party is a match
						if (mdCurrent.nRespID == nRespPartyID){
							mdCurrent.cyBalance = mdCurrent.cyBalance + cyPatientCharge - cyPatientPayment;
							mapPatients.SetAt(strPatProvID, mdCurrent);
						}
						}
						
						pRS->MoveNext();
					
				}
						
				//PLID 62851 - adjust note to be added to include the patient balance
				//  Build a new map used to write to history
				CMap<CString, LPCTSTR, CString, CString> mapPatientsTemp;
				POSITION pos = mapPatients.GetStartPosition();
				CString strKey;
				MapData mdCurrent;
				CString strNote;
				while (pos != NULL){
					mapPatients.GetNextAssoc(pos, strKey, mdCurrent);
					strNote = mdCurrent.strNote + ". Reported Patient Balance: " + FormatCurrencyForInterface(mdCurrent.cyBalance);
					mapPatientsTemp.SetAt(strKey, strNote);
				}

				//ok, now we have the recordset in a map, now we just have to insert the mail ID's and insert the records
				
				//check to see if they can support XML
				// (a.walling 2010-08-27 17:18) - PLID 39965 - We require at least SQL 2000 now.
				{
					
					//create a temp table out of the XML
					CString strTempTableName;
					strTempTableName = CreateWriteToHistoryTempTable_80WithStringKey(&mapPatientsTemp);

					// (j.jones 2014-08-04 11:59) - PLID 63150 - saving to history is now a modular function
					// (r.goldschmidt 2014-08-05 16:24) - PLID 62717 - create category id
					WriteTempTableToHistory(strTempTableName, nCategoryTypePref);
					}

					
				}
			break;
		}


	}NxCatchAll("Error adding to patient History");

		

}

const void CReportInfo::TracerLetterWriteToHistory(_RecordsetPtr pRS, NxOutputType nxOutType, CString strNote)  {

	try {

		//determine what the output type is and what we want to save to history on
		//TODO::IMPLEMENT this, should be something like the following
		/*NxOutputType  nxOutWanted = GetRemotePropertyInt("SttmntWriteHistoryOutputType");
			
			if (nxoutWanted == NXR_PREVIEW) {
			...
			else if
			///
			}
		*/

		//For now, we are only writing to history when it prints
		if (nxOutType != NXR_PRINT) {

			return;
		}


		CMapLongToCString mapPatients;
		CMapLongToCString mapTracerForms;

		long nPersonID;

		//prepare the temp table for the Claim History update
		CStringArray aryFieldNames;
		CStringArray aryFieldTypes;
		aryFieldNames.Add("BillID");
		aryFieldTypes.Add("INT");
		aryFieldNames.Add("InsuredPartyID");
		aryFieldTypes.Add("INT");

		CString strXML = CreateXML(aryFieldNames,pRS,"BillID",TRUE);
		CString strClaimHistoryTempTableName = CreateTempTableFromXML(aryFieldNames,aryFieldTypes,strXML);

		//we have to move the recordset back to the beginning because they can potentially run this twice and 
		// the second time around it would already be at the end if we don't move it to the beginning right here
		//**we have to check for both bof and eof to find out whether the recordset is empty because its always going to
		//be eof since we already looped through it*//

		if (!(pRS->eof && pRS->bof)) {
			//move it back to the beginning of the file
			pRS->MoveFirst();
		}
										
		//loop through the recordset and add to the map
		while (! pRS->eof) {
			nPersonID = AdoFldLong(pRS, "PatID");

			//check to see if the patient is already in the map
			long nResult;
			nResult = mapPatients.Lookup(nPersonID, strNote);

			if (nResult == 0) {
				// it isn't in the list yet, so add it
				mapPatients.SetAt(nPersonID, strNote);
				
			}			

			pRS->MoveNext();
			
		}
		//ok, now we have the recordset in a map, now we just have to insert the mail ID's and insert the records
		//TODO: figure out how to do this
		
		//check to see if they can support XML
		// (a.walling 2010-08-27 17:18) - PLID 39965 - We require at least SQL 2000 now.
		{
			
			//create a temp table out of the XML
			CString strTempTableName;
			strTempTableName = CreateWriteToHistoryTempTable_80(&mapPatients);
			
			//Now do the Insert
			
			// (j.jones 2014-08-04 11:59) - PLID 63150 - saving to history is now a modular function
			WriteTempTableToHistory(strTempTableName, -1);

			//Send Type -1 - Tracer Letter
			
			//(r.wilson 10/2/2012) plid 53082 - Replace hardcoded SendType with enumerated value
			// (j.jones 2005-04-06 13:35) - for tracer letters, show all charges, not just batched ones
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges,
			// but we still do not filter on Batched status
			// (j.jones 2013-01-23 08:55) - PLID 54734 - the ID column is now an identity
			_RecordsetPtr rsClaimHistory = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"DECLARE @nFirstClaimHistoryID INT \r\n"
				"SET @nFirstClaimHistoryID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ClaimHistoryT WITH(UPDLOCK, HOLDLOCK)) \r\n"
				"INSERT INTO ClaimHistoryT (BillID, InsuredPartyID, SendType, Date, UserName) "
				"	SELECT BillID, InsuredPartyID, {INT}, GetDate(), {STRING} FROM {SQL} \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nFirstClaimHistoryID AS FirstClaimHistoryID",
				(long)ClaimSendType::TracerLetter, GetCurrentUserName(), CSqlFragment(strClaimHistoryTempTableName));
			long nFirstClaimHistoryID = -1;
			if(rsClaimHistory->eof) {
				ThrowNxException("Failed to create new claim history records!");
			}
			else {
				nFirstClaimHistoryID = VarLong(rsClaimHistory->Fields->Item["FirstClaimHistoryID"]->Value);
			}
			rsClaimHistory->Close();

			//create details for each ClaimHistoryT record we just inserted
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, BillID FROM ClaimHistoryT WHERE ID >= {INT}", nFirstClaimHistoryID);
			while(!rs->eof) {
				
				ExecuteParamSql("INSERT INTO ClaimHistoryDetailsT (ClaimHistoryID, ChargeID) "
					"SELECT {INT}, ChargesT.ID FROM ChargesT "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE Deleted = 0 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"AND BillID = {INT}", AdoFldLong(rs, "ID"),AdoFldLong(rs, "BillID"));
				rs->MoveNext();
			}
			rs->Close();
		}

						
	}NxCatchAll("Error adding to patient History");

		

}

// (j.gruber 2011-10-13 10:06) - PLID 45916 - update the bills
const void CReportInfo::UpdateAffiliateStatusToPaid(_RecordsetPtr pRS)
{
	long nAuditTransID = -1;

	try {

		CMapLongToCString mapBillID;

		long nBillID;		

		//we have to move the recordset back to the beginning because they can potentially run this twice and 
		// the second time around it would already be at the end if we don't move it to the beginning right here
		//**we have to check for both bof and eof to find out whether the recordset is empty because its always going to
		//be eof since we already looped through it*//

		if (!(pRS->eof && pRS->bof)) {
			//move it back to the beginning of the file
			pRS->MoveFirst();
		}

		//loop through the recordset and add to the map
		

		while (! pRS->eof) {
			nBillID = AdoFldLong(pRS, "BillID");
						
			CString strBlank;			
				
			long nResult;
			nResult = mapBillID.Lookup(nBillID, strBlank);

			if (nResult == 0) {
				// it isn't in the list yet, so add it
				mapBillID.SetAt(nBillID, "");										
			}			

			pRS->MoveNext();
							
		}

		//check the size of the map
		if (mapBillID.GetSize() == 0) {
			//output a message box
			MsgBox("The recordset is empty.  No bills will be updated.");
			return;	
		}
		{
			
			//create a temp table out of the XML
			CString strTempTableName;
			strTempTableName = CreateWriteToHistoryTempTable_80(&mapBillID);
			
			//Now do the Insert			
			CString strSqlBatch = BeginSqlBatch();
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nStatusID int");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @strOldStatusName nVarChar(50); ");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @strNewStatusName nVarChar(50); ");
			AddStatementToSqlBatch(strSqlBatch, " SET NOCOUNT ON; ");
			AddStatementToSqlBatch(strSqlBatch, "SET @nStatusID = (SELECT BillAffiliateStatusT.ID FROM BillAffiliateStatusT WHERE Type = 3)");
			AddStatementToSqlBatch(strSqlBatch, "SET @strOldStatusName = (SELECT BillAffiliateStatusT.Name FROM BillAffiliateStatusT WHERE Type = 2)");
			AddStatementToSqlBatch(strSqlBatch, "SET @strNewStatusName = (SELECT BillAffiliateStatusT.Name FROM BillAffiliateStatusT WHERE Type = 3)");
			AddStatementToSqlBatch(strSqlBatch, "UPDATE BillsT SET AffiliateStatusID = @nStatusID "
				" FROM BillsT INNER JOIN BillAffiliateStatusT ON BillsT.AffiliateStatusID = BillAffiliateStatusT.ID "
				" INNER JOIN %s ON BillsT.ID = %s.ID "
				" WHERE BillAffiliateStatusT.Type = 2; ",
			strTempTableName, strTempTableName);
			AddStatementToSqlBatch(strSqlBatch, " INSERT INTO BillAffiliateStatusHistoryT (BillID, StatusID) "
				" SELECT ID, @nStatusID FROM %s; ",
				strTempTableName);			
			AddStatementToSqlBatch(strSqlBatch, " SET NOCOUNT OFF; ");
			AddStatementToSqlBatch(strSqlBatch, "SELECT BillsT.ID as BillID, BillsT.PatientID, @strOldStatusName as OldStatusName, @strNewStatusName as NewStatusName "
				" FROM BillsT INNER JOIN %s ON BillsT.ID = %s.ID ", strTempTableName, strTempTableName);
			
			CNxParamSqlArray ary;
			_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, ary);

			//now loop through and audit
			while (!rs->eof) {
			
				long nBillID = AdoFldLong(rs, "BillID");
				long nPatientID = AdoFldLong(rs, "PatientID");
				CString strOldName = AdoFldString(rs, "OldStatusName");
				CString strNewName = AdoFldString(rs, "NewStatusName");
				
				if (nAuditTransID == -1) {
					nAuditTransID = BeginAuditTransaction();
				}			
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiBillAffiliatePhysStatus, nBillID, strOldName, strNewName, aepMedium, aetChanged);
			
				rs->MoveNext();
			}
			if (nAuditTransID != -1) {
				CommitAuditTransaction(nAuditTransID);
			}
		}
	}NxCatchAllSilentCallThrow(
		if (nAuditTransID != -1) {
			RollbackAuditTransaction(nAuditTransID);
		}
	);
}

//(e.lally 2010-09-10) PLID 40488 - Write entry in mailSent history when Wellness information is Printed
const void CReportInfo::WellnessWriteToHistory(_RecordsetPtr pRS, NxOutputType nxOutType, CString strNote) 
{
	try {
		//We are only writing to history when it prints
		if (nxOutType != NXR_PRINT) {
			return;
		}
		CMapLongToCString mapPatients;
		CMapLongToCString mapTracerForms;

		long nPersonID = -1;

		//we have to move the recordset back to the beginning because they can potentially run this twice and 
		// the second time around it would already be at the end if we don't move it to the beginning right here
		//**we have to check for both bof and eof to find out whether the recordset is empty because its always going to
		//be eof since we already looped through it*//

		if (!(pRS->eof && pRS->bof)) {
			//move it back to the beginning of the file
			pRS->MoveFirst();
		}
										
		//loop through the recordset and add to the map
		while (! pRS->eof) {
			nPersonID = AdoFldLong(pRS, "PatID");

			//check to see if the patient is already in the map
			long nResult;
			nResult = mapPatients.Lookup(nPersonID, strNote);

			if (nResult == 0) {
				// it isn't in the list yet, so add it
				mapPatients.SetAt(nPersonID, strNote);
			}			
			pRS->MoveNext();	
		}
		//ok, now we have the recordset in a map, now we just have to insert the mail ID's and insert the records
		//TODO: figure out how to do this
		{
			//create a temp table
			CString strTempTableName;
			strTempTableName = CreateWriteToHistoryTempTable_80(&mapPatients);
			
			//Now do the Insert
			// (j.jones 2014-08-04 11:59) - PLID 63150 - saving to history is now a modular function
			WriteTempTableToHistory(strTempTableName, -1);
			}
	}NxCatchAll(__FUNCTION__);
}


// XML Implementation (not supported by sql 7 (default install))
//
// Creates a temporary table (will be deleted automatically) and fills it 
// with the IDs from the given Map,
// consecutive duplicates will be ignored if desired
// 
// Example output:
//      ID     |   RowNumber
//    ---------+------------
//    8327     |       1
//    504	   |       2
//    6490	   |       3
//    7976	   |       4
//    24080	   |       5
//

const CString CReportInfo::CreateWriteToHistoryTempTable_80(CMap<long, long, CString, CString> *map)
{
	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it
	
	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#Temp%luT", GetTickCount());
	ExecuteSql("CREATE TABLE %s (ID int, RowNumber INT IDENTITY, NOTE NVARCHAR(255))", strTempT);

	// Start at a string length of 2^17 (we'll double this if we go out of bounds)
	long nMaxLen = 1<<17;
	long nLen = 0;
	long nSubLen;
	CString str, strXml;
	TCHAR *pstr = strXml.GetBuffer(nMaxLen);
	// We're building a simple XML string here ('<ROOT><RECORD ID="123" /><RECORD ID="456" /></ROOT>')
	nSubLen = sprintf(pstr, "<ROOT>");
	pstr += nSubLen;
	nLen += nSubLen;
	// Loop through the Map
	LONG nLastIDVal = -1;
	long i = 0;
	long nKey;
	CString pItem;
	POSITION pos = map->GetStartPosition();
	while (pos != NULL) {
		// Get the next item in the map
		map->GetNextAssoc(pos, nKey, pItem);
		LONG nIDVal = nKey;
		if ( i==0 || (nIDVal != nLastIDVal)) { // IF (don't skip duplicates OR this is our first iteration OR this one is different from the last one)
			// Remember this one for the next iteration
			nLastIDVal = nIDVal;
			
			// Create a string that will represent the <RECORD ID="123" NOTE="allala" /> part 
			// of our XML (we use "P" instead of "RECORD" because this could be 
			// a huge string and we want it to be as efficient as possible)
			
			//check to see that the string does not contain and invalid characters and replace them if it does
			pItem.Replace("&", "And");
			pItem.Replace("<", " ");
			pItem.Replace(">", " ");
			pItem.Replace("\"", " ");
			
			str.Format("<P ID=\"%li\" NOTE=\"%s\" />", nIDVal, pItem);
						
			// Here's the fun part: if this substring would make our XML larger 
			// than the currently allocated space, we want to double the currently 
			// allocated space until it would fit
			while (nLen+str.GetLength() >= nMaxLen-1) {
				strXml.ReleaseBuffer();
				nMaxLen = nMaxLen * 2;
				pstr = strXml.GetBuffer(nMaxLen) + nLen;
				ASSERT(pstr[0] == '\0');
			}

			// Write the new substring to the end of the big xml string
			nSubLen = sprintf(pstr, str);
			pstr += nSubLen;
			nLen += nSubLen;
			
			// Increment our count
			i++;
		}
	}
	// Close the XML
	strXml.ReleaseBuffer();
	strXml += "</ROOT>";
	
	// Add the rows from the XML into our temp table
	ExecuteSql(
		"DECLARE @hDoc AS INT; "  // We need a document handle
		"EXEC sp_xml_preparedocument @hDoc OUTPUT, N'%s'; " // Ask SQL to parse the XML (returns the document handle)
		"INSERT INTO %s (ID, NOTE) SELECT * FROM OPENXML(@hDoc, '/ROOT/P') WITH (ID INT, NOTE NVARCHAR(255)); " // Insert into the table
		"EXEC sp_xml_removedocument @hDoc;",  // Release our document handle
		_Q(strXml), strTempT);


	// Return the name of the temp table
	return strTempT;
}


// (j.gruber 2008-06-09 10:06) - PLID 29399 - added this version that takes a string key so that you can have more then one entry per patient
const CString CReportInfo::CreateWriteToHistoryTempTable_80WithStringKey(CMap<CString, LPCTSTR, CString, CString> *map)
{
	//the string Key MUST be in the format 
	// PatientID - OtherKey


	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it
	
	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#Temp%luT", GetTickCount());
	ExecuteSql("CREATE TABLE %s (ID int, RowNumber INT IDENTITY, NOTE NVARCHAR(255))", strTempT);

	// Start at a string length of 2^17 (we'll double this if we go out of bounds)
	long nMaxLen = 1<<17;
	long nLen = 0;
	long nSubLen;
	CString str, strXml;
	TCHAR *pstr = strXml.GetBuffer(nMaxLen);
	// We're building a simple XML string here ('<ROOT><RECORD ID="123" /><RECORD ID="456" /></ROOT>')
	nSubLen = sprintf(pstr, "<ROOT>");
	pstr += nSubLen;
	nLen += nSubLen;
	// Loop through the Map
	
	CString strLastKey;
	long i = 0;
	CString strKey;
	CString pItem;
	POSITION pos = map->GetStartPosition();
	while (pos != NULL) {
		// Get the next item in the map
		map->GetNextAssoc(pos, strKey, pItem);
		LONG nIDVal = atoi(strKey.Left(strKey.Find(" ")));
		if ( i==0 || (strKey.CompareNoCase(strLastKey) != 0)) { // IF (don't skip duplicates OR this is our first iteration OR this one is different from the last one)
			// Remember this one for the next iteration
			strLastKey = strKey;
			
			// Create a string that will represent the <RECORD ID="123" NOTE="allala" /> part 
			// of our XML (we use "P" instead of "RECORD" because this could be 
			// a huge string and we want it to be as efficient as possible)
			
			//check to see that the string does not contain and invalid characters and replace them if it does
			pItem.Replace("&", "And");
			pItem.Replace("<", " ");
			pItem.Replace(">", " ");
			pItem.Replace("\"", " ");
			
			str.Format("<P ID=\"%li\" NOTE=\"%s\" />", nIDVal, pItem);
						
			// Here's the fun part: if this substring would make our XML larger 
			// than the currently allocated space, we want to double the currently 
			// allocated space until it would fit
			while (nLen+str.GetLength() >= nMaxLen-1) {
				strXml.ReleaseBuffer();
				nMaxLen = nMaxLen * 2;
				pstr = strXml.GetBuffer(nMaxLen) + nLen;
				ASSERT(pstr[0] == '\0');
			}

			// Write the new substring to the end of the big xml string
			nSubLen = sprintf(pstr, str);
			pstr += nSubLen;
			nLen += nSubLen;
			
			// Increment our count
			i++;
		}
	}
	// Close the XML
	strXml.ReleaseBuffer();
	strXml += "</ROOT>";
	
	// Add the rows from the XML into our temp table
	ExecuteSql(
		"DECLARE @hDoc AS INT; "  // We need a document handle
		"EXEC sp_xml_preparedocument @hDoc OUTPUT, N'%s'; " // Ask SQL to parse the XML (returns the document handle)
		"INSERT INTO %s (ID, NOTE) SELECT * FROM OPENXML(@hDoc, '/ROOT/P') WITH (ID INT, NOTE NVARCHAR(255)); " // Insert into the table
		"EXEC sp_xml_removedocument @hDoc;",  // Release our document handle
		_Q(strXml), strTempT);


	// Return the name of the temp table
	return strTempT;
}






CString CReportInfo::GetQuoteEmpName() const {

	CString strReturn;

	long nEmpNameFormat = GetRemotePropertyInt("QuotePatCoordFormat");

	switch (nEmpNameFormat) {

		case 0: //first

			strReturn = "PersonEmp.First ";
		break;

		case 1:  //first last

			strReturn = " PersonEmp.First + ' ' + PersonEmp.Last ";
		break;

		case 2: //first middle last
			strReturn = " PersonEmp.First + ' ' + PersonEmp.Middle + ' ' + PersonEmp.Last ";
		break;

		case 3: //Last, first
			strReturn = " PersonEmp.Last + ', ' + PersonEmp.First ";
		break;

		case 4: //Last , First Middle
			strReturn = " PersonEmp.Last + ', ' + PersonEmp.First + ' ' + PersonEmp.Middle ";
		break;

		default :

			strReturn = " PersonEmp.Last + ', ' + PersonEmp.First + ' ' + PersonEmp.Middle ";
		break;
	}

	return strReturn;


}



//not just for debugging any longer -JMM 04/04/03
//#ifdef _DEBUG
/////////////////////////////////////////
//TS:  Functions for loading p2smon.dll

static CreateDefFnType CreateFieldDefFile;

bool EnsureP2smonDlls()
{
	
	if (!l_bP2smonDllsEnsured) {
		l_bP2smonDllsLoaded = false;
		l_hP2smonDll = ::LoadLibrary("p2smon.dll");
		if (l_hP2smonDll) {
			CreateFieldDefFile = (CreateDefFnType)GetProcAddress(l_hP2smonDll,"CreateFieldDefFile");
			if (CreateFieldDefFile) {
				l_bP2smonDllsLoaded = true;
			}
		}
		l_bP2smonDllsEnsured = true;

		// If our attempts to load above failed, we still must let go of the library
		if (!l_bP2smonDllsLoaded) {
			FreeP2smonDlls();
		}
	}
	
	return l_bP2smonDllsLoaded;
}

void FreeP2smonDlls()
{
	if (l_hP2smonDll) {
		::FreeLibrary(l_hP2smonDll);
		l_hP2smonDll = NULL;
	}
	if (CreateFieldDefFile) {
		CreateFieldDefFile = NULL;
	}
	l_bP2smonDllsEnsured = false;
	l_bP2smonDllsLoaded = false;
}


// (j.gruber 2007-02-22 16:50) - PLID 24832 - support deleting subreport ttx files also
void CReportInfo::DeleteTtxFiles() const {


	//we need to access the report engine so that we can access the subreports

	try {
		//DRT 2/24/2004 - This was crashing whenever you tried to make ttx files.
		CRPEngine *Engine = CRPEngine::GetEngine();
		if (!Engine) {
			if(GetMainFrame())
				Engine = GetMainFrame()->GetReportEngine();
			else
				Engine = new CRPEngine;
		}

		ASSERT(Engine);
		CNxReportJob *RepJob;
		if (Engine && Engine->Open()) {
			
			CString strTtxFileName;
			if (IsStatement(nID)) {
				strTtxFileName = GetCustomReportsPath() ^ GetStatementFileName(nID) + ".ttx";
			}
			else {
				strTtxFileName = GetCustomReportsPath() ^  strReportFile + ".ttx";
			}


			//get the report so we can loop through it
			CString strPathName = GetReportsPath() ^ strReportFile;
			strPathName += GetDateSuffix(nDateFilter);
			strPathName.TrimRight();
			if(!IsStatement(nID)){
				if(nDetail == 2) strPathName += "Smry";
				else if(nDetail == 1) strPathName += "Dtld";
			}
			strPathName += ".rpt";
			
			RepJob = (CNxReportJob *)Engine->OpenJob(strPathName);

			
			//delete the main file
			DeleteFile(strTtxFileName);

			//Now, loop through each section and subreport, deleting their .ttx files.
			long nSubReportCnt = 0;
			if(RepJob) {
				CString strTtxName;
				for(int i = 0; i < RepJob->GetNSections(); i++) {
					for(int j = 0; j < RepJob->GetNSubreportsInSection(RepJob->GetSectionCode(i)); j++) {
						strTtxName.Format("%s%li.ttx", strReportFile, nSubReportCnt);
						nSubReportCnt++;	//increment the count of sub reports
						
						DeleteFile(strTtxName);
					}
				}
			}
		}
	}NxCatchAll("Error in DeleteAllTtxFiles()");
}



bool CReportInfo::CreateTtxFile() const{
/*****************************************************************************************************************
	//(e.lally 2008-04-07) PLID 9989 - If we ever get rid of the functionality of the few report files that use
	//a static ttx file generation from source code, we need to search for PLID 9989 and implement the checks for
	//report verification so we can return an empty recordset to speed up the verify process.
******************************************************************************************************************/
	if(EnsureP2smonDlls()){
		_RecordsetPtr rs;
		//Loop through all subreports.
		
		try {
			//DRT 2/24/2004 - This was crashing whenever you tried to make ttx files.
			CRPEngine *Engine = CRPEngine::GetEngine();
			if (!Engine) {
				if(GetMainFrame())
					Engine = GetMainFrame()->GetReportEngine();
				else
					Engine = new CRPEngine;
			}

			ASSERT(Engine);
			CNxReportJob *RepJob;
			if (Engine && Engine->Open()) {
				CString strPathName = GetReportsPath() ^ strReportFile;
				strPathName += GetDateSuffix(nDateFilter);
				strPathName.TrimRight();
				if(!IsStatement(nID)){
					if(nDetail == 2) strPathName += "Smry";
					else if(nDetail == 1) strPathName += "Dtld";
				}
				strPathName += ".rpt";
				RepJob = (CNxReportJob *)Engine->OpenJob(strPathName);

				//First, create the main .ttx file.
				//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag that we are verifying reports.
				BOOL bIsForReportVerify = TRUE;
				rs = GetRecordset(0,0, bIsForReportVerify);
				IUnknown * lprsDisp = (IUnknown *) rs;
				if(!CreateFieldDefFile(&lprsDisp, strReportFile + ".ttx", true) ){
					MessageBox(GetActiveWindow(), "Could not find the query for this report file.\n"
						"Verify it is in the right source file and the right GetSql... function of that file.", strReportFile, MB_OK);
					return false;
				}
				//Now, loop through each section and subreport, generating their .ttx files.
				//DRT 2/24/2004 - PLID 10509 - This code has never worked.  It loops through each section of the report, 
				//	and then calls GetRecordset() on each section.  However, it was using the index of the inside
				//	loop.  This would work if all subreports were inside the same section, which they frequently are
				//	not.  So I changed it to have a sub report count, which it increments as it generates each 
				//	subreport.
				long nSubReportCnt = 0;
				if(RepJob) {
					CString strTtxName;
					for(int i = 0; i < RepJob->GetNSections(); i++) {
						for(int j = 0; j < RepJob->GetNSubreportsInSection(RepJob->GetSectionCode(i)); j++) {
							strTtxName.Format("%s%li.ttx", strReportFile, nSubReportCnt);
							//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag that we are verifying reports.
							rs = GetRecordset(1, nSubReportCnt, bIsForReportVerify);
							nSubReportCnt++;	//increment the count of sub reports
							lprsDisp = (IUnknown *) rs;
							if(!CreateFieldDefFile(&lprsDisp, strTtxName, true) ){
								MessageBox(GetActiveWindow(), strReportFile, "FALSE", MB_OK);
								return false;
							}
						}
					}
				}
			}
		}NxCatchAll("Error in CreateAllTtxFiles()");
		
		
		return true;
	}
	else return false;
}
//#endif //#ifdef _DEBUG

//overload of the CreateTTxFile function which takes a path to store the Ttx file in
BOOL CReportInfo::DefCreateTtxFile(CString strPath) const{
/*****************************************************************************************************************
	//(e.lally 2008-04-07) PLID 9989 - If we ever get rid of the functionality of the few report files that use
	//a static ttx file generation from source code, we need to search for PLID 9989 and implement the checks for
	//report verification so we can return an empty recordset to speed up the verify process.
******************************************************************************************************************/
	if(EnsureP2smonDlls()){
		_RecordsetPtr rs;
		//Loop through all subreports.
		
		try {
			CRPEngine *Engine = CRPEngine::GetEngine();
			if (!Engine) {
				if(GetMainFrame()) {
					Engine = GetMainFrame()->GetReportEngine();
				}
				else {
					 Engine = new CRPEngine;
				}
			}
			ASSERT(Engine);
			CNxReportJob *RepJob;
			if (Engine && Engine->Open()) {
				// set up report
				CString strPathName = GetReportsPath() ^ strReportFile;

				strPathName += GetDateSuffix(nDateFilter);
				strPathName.TrimRight();
				
				if (nDetail == 2) strPathName += "Smry";
				else if (nDetail == 1) strPathName += "Dtld";

				//append the .rpt
				strPathName += ".rpt";

				RepJob = (CNxReportJob *)Engine->OpenJob(strPathName);

				//First, create the main .ttx file.
				CString strTtxFile;
				strTtxFile = strPath ^ strReportFile;
				//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag that we are verifying reports.
				BOOL bIsForReportVerify = TRUE;
				rs = GetRecordset(0,0,bIsForReportVerify);
				IUnknown * lprsDisp = (IUnknown *) rs;
				if(!CreateFieldDefFile(&lprsDisp, strTtxFile + ".ttx", true) ){
					MessageBox(GetActiveWindow(), strTtxFile + ".ttx", "FALSE", MB_OK);
					return false;
				}
				//Now, loop through each section and subreport, generating their .ttx files.
				//DRT 2/24/2004 - PLID 10509 - This code has never worked.  It loops through each section of the report, 
				//	and then calls GetRecordset() on each section.  However, it was using the index of the inside
				//	loop.  This would work if all subreports were inside the same section, which they frequently are
				//	not.  So I changed it to have a sub report count, which it increments as it generates each 
				//	subreport.
				long nSubReportCnt = 0;
				if(RepJob) {
					CString strTtxName;
					for(int i = 0; i < RepJob->GetNSections(); i++) {
						for(int j = 0; j < RepJob->GetNSubreportsInSection(RepJob->GetSectionCode(i)); j++) {
							strTtxName.Format("%s%li.ttx", strTtxFile, nSubReportCnt);
							//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag that we are verifying reports.
							rs = GetRecordset(1, nSubReportCnt, bIsForReportVerify);
							nSubReportCnt++;	//increment the count of sub reports
							lprsDisp = (IUnknown *) rs;
							if(!CreateFieldDefFile(&lprsDisp, strTtxName, true) ){
								MessageBox(GetActiveWindow(), strTtxName, "FALSE", MB_OK);
								return false;
							}
						}
					}
				}
			}
		}NxCatchAll("Error in CreateAllTtxFiles()");
		
		
		return true;
	}
	else return false;
}


BOOL CReportInfo::CreateTtxFile(CReportInfo *pRep, CString strPath) const{
/*****************************************************************************************************************
	//(e.lally 2008-04-07) PLID 9989 - If we ever get rid of the functionality of the few report files that use
	//a static ttx file generation from source code, we need to search for PLID 9989 and implement the checks for
	//report verification so we can return an empty recordset to speed up the verify process.
******************************************************************************************************************/
	if(EnsureP2smonDlls()){
		_RecordsetPtr rs;
		//Loop through all subreports.
		
		try {
			CRPEngine *Engine = CRPEngine::GetEngine();
			if (!Engine) {
				if(GetMainFrame()) {
					Engine = GetMainFrame()->GetReportEngine();
				}
				else {
					 Engine = new CRPEngine;
				}
			}
			ASSERT(Engine);
			CNxReportJob *RepJob;
			if (Engine && Engine->Open()) {
				// set up report
				CString strPathName = GetReportsPath() ^ pRep->strReportFile + ".rpt";
				RepJob = (CNxReportJob *)Engine->OpenJob(strPathName);

				//First, create the main .ttx file.
				CString strTtxFile;
				strTtxFile = strPath ^ pRep->strReportFile;
				//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag that we are verifying reports.
				BOOL bIsForReportVerify = TRUE;
				rs = pRep->GetRecordset(0,0, bIsForReportVerify);
				IUnknown * lprsDisp = (IUnknown *) rs;
				if(!CreateFieldDefFile(&lprsDisp, strTtxFile + ".ttx", true) ){
					MessageBox(GetActiveWindow(), strTtxFile + ".ttx", "FALSE", MB_OK);
					return false;
				}
				//Now, loop through each section and subreport, generating their .ttx files.
				//DRT 2/24/2004 - PLID 10509 - This code has never worked.  It loops through each section of the report, 
				//	and then calls GetRecordset() on each section.  However, it was using the index of the inside
				//	loop.  This would work if all subreports were inside the same section, which they frequently are
				//	not.  So I changed it to have a sub report count, which it increments as it generates each 
				//	subreport.
				long nSubReportCnt = 0;
				if(RepJob) {
					CString strTtxName;
					for(int i = 0; i < RepJob->GetNSections(); i++) {
						for(int j = 0; j < RepJob->GetNSubreportsInSection(RepJob->GetSectionCode(i)); j++) {
							strTtxName.Format("%s%li.ttx", strTtxFile, nSubReportCnt);
							//(e.lally 2008-04-07) PLID 9989 - Add parameter to flag that we are verifying reports.
							rs = pRep->GetRecordset(1, nSubReportCnt, bIsForReportVerify);
							nSubReportCnt++;	//increment the count of sub reports
							lprsDisp = (IUnknown *) rs;
							if(!CreateFieldDefFile(&lprsDisp, strTtxName, true) ){
								MessageBox(GetActiveWindow(), strTtxName, "FALSE", MB_OK);
								return false;
							}
						}
					}
				}
			}
		}NxCatchAll("Error in CreateAllTtxFiles()");
		
		
		return true;
	}
	else return false;
}


CString CReportInfo::DefGetDateSuffix(long nOption) const
{
	long nIndex = 0;
	while(nIndex < strDateOptions.GetLength() ) {
		//Load a record into the datalist.
		//First, the id.
		long nSemicolon = strDateOptions.Find(";", nIndex);
		long nID = (long)atoi(strDateOptions.Mid(nIndex, nSemicolon-nIndex));
		nIndex = nSemicolon+1;

		//Now, the display name
		nSemicolon = strDateOptions.Find(";", nIndex);
		nIndex = nSemicolon+1;

		//Now, the field name
		nSemicolon = strDateOptions.Find(";", nIndex);
		nIndex = nSemicolon+1;

		//Finally the report suffix
		nSemicolon = strDateOptions.Find(";", nIndex);
		if(nID == nOption)
			return strDateOptions.Mid(nIndex, nSemicolon-nIndex);
		nIndex = nSemicolon+1;
	}
	//Crap, let's just return a blank string and go home.
	return "";
}

CString CReportInfo::DefGetDateField(long nOption) const
{
	long nIndex = 0;
	while(nIndex < strDateOptions.GetLength() ) {
		//Load a record into the datalist.
		//First, the id.
		long nSemicolon = strDateOptions.Find(";", nIndex);
		long nID = (long)atoi(strDateOptions.Mid(nIndex, nSemicolon-nIndex));
		nIndex = nSemicolon+1;

		//Now, the display name
		nSemicolon = strDateOptions.Find(";", nIndex);
		nIndex = nSemicolon+1;

		//Now, the field name
		nSemicolon = strDateOptions.Find(";", nIndex);
		if(nID == nOption)
			return strDateOptions.Mid(nIndex, nSemicolon-nIndex);
		nIndex = nSemicolon+1;

		//Finally the report suffix
		nSemicolon = strDateOptions.Find(";", nIndex);
		nIndex = nSemicolon+1;
	}
	//Crap, let's just return a blank string and go home.
	return "";
}

CString CReportInfo::DefGetDateName(long nOption) const
{
	long nIndex = 0;
	while(nIndex < strDateOptions.GetLength() ) {
		//Load a record into the datalist.
		//First, the id.
		long nSemicolon = strDateOptions.Find(";", nIndex);
		long nID = (long)atoi(strDateOptions.Mid(nIndex, nSemicolon-nIndex));
		nIndex = nSemicolon+1;

		//Now, the display name
		nSemicolon = strDateOptions.Find(";", nIndex);
		if(nID == nOption)
			return strDateOptions.Mid(nIndex, nSemicolon-nIndex);
		nIndex = nSemicolon+1;

		//Now, the field name
		nSemicolon = strDateOptions.Find(";", nIndex);
		nIndex = nSemicolon+1;

		//Finally the report suffix
		nSemicolon = strDateOptions.Find(";", nIndex);
		nIndex = nSemicolon+1;
	}
	//Crap, let's just return a blank string and go home.
	return "";
}

// (c.haag 2015-02-23) - PLID 63751 - Gets the allowed locations for a field.
CString CReportInfo::GetAllowedLocationClause(const CString &strField) const
{
	return ::GetAllowedLocationClause(strField);
}

// (c.haag 2015-02-23) - PLID 63751 - Gets the allowed locations for a field.
CSqlFragment CReportInfo::GetAllowedLocationClause_Param(const CString &strField) const
{
	return ::GetAllowedLocationClause_Param(strField);
}

//DRT 10/26/2005 - PLID 18085 - This function will take a CRParameterInfo pointer and add
//	it to the array for this report.  The CReportInfo object is in charge of memory 
//	deallocation of all parameters (when deconstructed).  Calling functions should never
//	deallocate the parameters themselves.
void CReportInfo::AddToParameterList(CRParameterInfo*& prpi)
{
	if (m_params.count(prpi->m_Name)) {
		ASSERT(FALSE);
	} else {
		m_params[prpi->m_Name] = prpi->m_Data;
	}
	delete prpi;
	prpi = NULL;
}

//DRT 10/26/2005 - PLID 18085 - This function will handle clearing the list of CRParameterInfo
//	objects and ensuring that we correctly deallocate the memory for them all.  We are in 
//	charge of that, no calling function should be doing it.
void CReportInfo::ClearParameterList()
{
	//clear the array
	m_params.clear();
}

// (a.walling 2007-06-19 13:50) - PLID 19405 - Verify the custom report without having to load an interface
BOOL CReportInfo::VerifyCustomReport() const
{
	BOOL bTTX = FALSE;

	try {
		CWaitCursor cws;
		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);

		_RecordsetPtr rsReportInfo = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = %li and Number = %li", nID, nDefaultCustomReport);

		if (rsReportInfo->eof) {
			ASSERT(FALSE);
			MsgBox("Practice could not find the custom report");
			return FALSE;
		}

		CString strFileName = AdoFldString(rsReportInfo, "FileName");					
		CString strFullFileName = GetCustomReportsPath() ^ strFileName;

		// (a.walling 2011-07-28 14:22) - PLID 44787 - Newer crystal runtimes set themselves as the CurVer but provide a non-backwards compatible interface.
		// So, request the 8.5 version, and fallback to version independent if that fails.
		CRAXDRT::IApplicationPtr  pApplication;
		pApplication.CreateInstance("CrystalDesignRuntime.Application.8.5");
		if (!pApplication) {
			pApplication.CreateInstance("CrystalDesignRuntime.Application");
		}

		if (pApplication != NULL) { // moving these scopes will ensure that report is freed before application
									//(which did not seem to cause a problem in the first place, but just to be safe).
			CRAXDRT::IReportPtr  pReport;

			pReport = pApplication->OpenReport((LPCTSTR)strFullFileName);

			if (pReport == NULL) {
				MsgBox("Unable to load report! You may try opening the report editor and clicking the verify button.");
				return FALSE;
			}
			
			SetCurrentDirectory(GetCustomReportsPath());
			long nCustomReportID = -1;
			if(nID == 658) {
				nCustomReportID = nDefaultCustomReport;
			}
			if (CreateAllTtxFiles(nID, GetCustomReportsPath(), nCustomReportID)) {
				bTTX = TRUE;
				//check to see that the ttx file name, is what we think it is
				CString strReportServerName = (LPCTSTR)pReport->Database->Tables->Item[1]->GetLogOnServerName();
				CString strTtxFileName;

				if (IsStatement(nID)) {
					strTtxFileName = GetStatementFileName(nID) + ".ttx";
				}
				else {
					strTtxFileName = strReportFile + ".ttx";
				}

				if (strReportServerName.CompareNoCase(strTtxFileName) == 0) {
					// let's verify
					HRESULT hr;

					hr = pReport->Database->Verify();
					
					// we are done with these
					DeleteTtxFiles();

					if (hr == 0) {
						if (SaveCustomReport(pReport, strFullFileName)) {
							//first we need to rename the file back to it's real filename, without the .tmp. We do this by destroying the references first.
							pReport.Release();
							pApplication.Release();

							//now rename the file
							CString strTempFileName = strFullFileName + "tmp";
							CString strTempOriginalFileName = strFullFileName + "t";
							if (MoveFile(strFullFileName, strTempOriginalFileName)) {
								//renamed the original report
								if (MoveFile(strTempFileName, strFullFileName)) {
									//renamed the new report!

									try {

										//if we are here then everything worked and we need to update the version in the CustomReports Table
										ExecuteSql("UPDATE CustomReportsT SET Version = %li WHERE ID = %li AND Number = %li", nVersion, nID, nDefaultCustomReport);
										
										//we also have to update all the records in CustomReportsT that use this report
										_RecordsetPtr rsCustom = CreateRecordset("SELECT ID, Number FROM CustomReportsT WHERE FileName = '%s'", _Q(strFileName));
										
										CString strSqlBatch = BeginSqlBatch();
										long n = 0;
										while (! rsCustom->eof) {
											long nID, nNumber;
											nID = AdoFldLong(rsCustom, "ID");
											nNumber = AdoFldLong(rsCustom, "Number");

											AddStatementToSqlBatch(strSqlBatch, "UPDATE CustomReportsT SET Version = %li WHERE ID = %li AND Number = %li", nVersion, nID, nNumber);

											rsCustom->MoveNext();
											n++;
										}

										if (n > 0)
											ExecuteSqlBatch(strSqlBatch);

										// remove the temp file
										CFile::Remove(strTempOriginalFileName);
									} NxCatchAllThrow("Error updating internal report information");
								} else {
									// error renaming new file

									// we could not rename the new report, so try to rename back the original report
									try {
										MoveFile(strTempOriginalFileName, strFullFileName);
									} NxCatchAll("Error restoring original report name; please contact Technical Support!");

									// now throw our error
									ThrowNxException("Error 101 renaming report: %s", FormatLastError());
								}
							} else {
								// error renaming original file

								// now throw our error
								ThrowNxException("Error 100 renaming report: %s", FormatLastError());
							}
						} else {
							// error saving

							// now throw our error
							ThrowNxException("Error saving report: %s", FormatLastError());
						}
					} else {
						// error verifying

						// now throw our error
						ThrowNxException("Error verifying report");
					}
				} else {
					// the names do not match up, so we cannot verify automatically.
					ThrowNxException("Unable to automatically verify the report.");
				}
				
			} else {
				// could not create ttx files!
				ThrowNxException("Could not create report information: %s", FormatLastError());
			}

			// we are good to go!
			return TRUE;
		} 

		// somehow we could not create the editor instance, so return false
		return FALSE; // caller should display a message box
	} NxCatchAll("Error automatically verifying report!");

	// if we get here, we must have had an exception.
	// (d.thompson said this should be in another try/catch, but it already is within the function.)
	try {
		if (bTTX)
			DeleteTtxFiles();
	} NxCatchAll("Could not clear ttx files (second attempt)");

	return FALSE; // caller should display a message box
}

// (a.walling 2007-09-11 11:17) - PLID 19405 - Save the report to a temp file (must be renamed/moved)
BOOL CReportInfo::SaveCustomReport(LPDISPATCH lpReportDisp, const CString &strFileName) const
{
	try {
		CRAXDRT::IReportPtr pReport(lpReportDisp);

		if (pReport) {
			// we cannot save the current file since it is already in use, so we have to save to a temp file
			// and then swap them around (unfortunately)
			CString strTempFileName = strFileName + "tmp";
			HRESULT hr = pReport->Save((LPCTSTR)strTempFileName);

			if(SUCCEEDED(hr)) {
				return TRUE;
			} else {
				return FALSE;
			}
		} else {
			return FALSE;
		}
	}NxCatchAll("Error Saving Report");

	return FALSE;
}

// (d.thompson 2010-03-16) - PLID 37721 - Audit that the report is run.  This can be safely used for any auditing report, 
//	as we know they all follow the "all dates" or "date range" setup equally.  Pass in the audit event to use for the 
//	right section.
void CReportInfo::AuditingReportExecuted(AuditEventItems aeiWhichReport) const
{
	CString strDate;
	if(nDateRange > 0) {
		strDate.Format("From %s to %s", FormatDateTimeForInterface(DateFrom, NULL, dtoDate), 
			FormatDateTimeForInterface(DateTo, NULL, dtoDate));
	}
	else {
		strDate = "For all dates";
	}
	AuditEvent(-1, "", BeginNewAuditEvent(), aeiWhichReport, -1, "Report Run", strDate, aepMedium, aetOpened);
}


// (j.gruber 2011-10-11 15:45) - PLID 45937 - Write to History Status
/* The WriteTohistory function handles most of this, but this function is here
in case you need to check a filter or value while the report is running.
This sets a bool in the nxReportJob based on what is returned
*/
BOOL CReportInfo::GetWriteToHistoryStatus() const
{
	switch (nID) {

		case 714:			
			if (saExtraValues.GetSize() == 1 && saExtraValues.GetAt(0) == "2") {
				return TRUE;
			}
			else {
				return FALSE;
			}
		break;

		default:
			//return TRUE here because all other cases are handled by WriteToHistory
			return TRUE;
		break;
	}
}

// only checks once per session, via the "MARS Connection" OLEDB property
static bool IsReportSnapshotUsingMARS()
{
	static bool checked = false;
	static bool isReportSnapshotUsingMARS = false;

	if (!checked) {
		checked = true;
		auto adoCon = GetRemoteConnectionReportSnapshot();

		bool wasOpen = adoCon.IsConnected();
		auto pCon = adoCon.GetRemoteData();

		isReportSnapshotUsingMARS = !!NxAdo::Properties(pCon).AsLong("MARS Connection");

		if (!wasOpen) {
			adoCon.EnsureNotRemoteData();
		}
	}

	return isReportSnapshotUsingMARS;
}

/// <summary>
/// Checks for MARS support and ConfigRT settings and per-report overrides to determine whether to use a server-side cursor
/// </summary>
BOOL CReportInfo::IsServerSide(long nReportID, bool bCalledFromTempTable/*=false*/)
{
	// if cursorOption is -1, then it means it is not set and to use the default.
	long cursorOption = GetRemotePropertyInt("ServerSideCursorReport", -1, nReportID, "<None>", false);
	
	switch (nReportID)
	{
		//AllfinancialQ exclusions
		case 137:
		case 153:
		case 154:
		case 155:
		case 297:
		case 325:
		case 468:
		case 580:
		case 603:
		case 719:
		{
			if (IsReportSnapshotUsingMARS()) {
				// MARS is supported!
				// Server side recordsets will perform better for everyone.

				if (-1 == cursorOption) {
					// not set, use default.
					return TRUE;
				} else if (0 == cursorOption) {
					return FALSE;
				}
				else {
					return TRUE;
				}
			}
			else {
				// Without MARS, financial reports are broken and need to load client side in their entirety.
				return FALSE;
			}
			break;
		}
		// Payments Under Allowed Amount may use a temp table optimization. It must be excluded if the temp table
		// is being used.
		case 366:
		{
			// if explicitly client-side or unset, use client-side cursor
			if (-1 == cursorOption || 0 == cursorOption) {
				return FALSE;
			}

			if (IsReportSnapshotUsingMARS()) {
				// MARS is supported! So we can use server-side regardless
				return TRUE;
			}
			else if (bCalledFromTempTable) {
				// if called from temp table, we cannot use server-side cursors without MARS.
				return FALSE;
			}
			else {
				// not called from temp table, so we are OK
				return TRUE;
			}
			break;
		}
		default:
		{
			// if explicitly client-side or unset, use client-side cursor
			if (-1 == cursorOption || 0 == cursorOption) {
				return FALSE;
			}

			if (IsReportSnapshotUsingMARS()) {
				// MARS is supported!
				return TRUE;
			}
			else {
				// (s.tullis 2016-05-18 17:41) - NX-100491
				// If you got here that means you are using ALL_FINANCIAL_Q, ALL_FINANCIAL_Q_FILTERED, GetAllFinancialQ_Insurance is your report query
				// You're report ID needs to be included in the the exclusions above.. clients who have the legacy setting on above will not be able to run your report
				ASSERT(!bCalledFromTempTable);
				return TRUE;
			}
			break;
		}
	}
}