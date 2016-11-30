#include "stdafx.h"
#include "GlobalUtils.h"
#include "LoginDlg.h"
#include "PracProps.h"
#include "PasswordEntryDlg.h"
#include "Practice.h"
#include "MainFrm.h"
#include "NxView.h"
#include "NxStandard.h"
#include "Reports.h"
#include "NxErrorDialog.h"
#include "GetNewIDName.h"
#include "GlobalDataUtils.h"
#include "NxReportJob.h"
#include "ReportInfo.h"
#include <mapi.h>
#include "ChildFrm.h"
#include "VersionInfo.h"
#include "ExportDuplicates.h"
#include "RegUtils.h"
#include "UnitedLink.h"
#include "UnitedLinkPatient.h"
#include "ZipChooser.h"
#include "NxSecurity.h"
#include "RenameFileDlg.h"
#include "NxServer.h"
#include "TodoUtils.h"
#include "SignatureDlg.h"
#include "MultiSelectDlg.h"
#include "PatientDocumentStorageWarningDlg.h"
#include "mirrorimagebutton.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "DateTimeUtils.h"
#include "EMR.h"
#include "EMN.h"
#include "MergeEngine.h"
#include "DoesExist.h"
#include "AuditTrail.h"
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include <time.h>
#include "NxAPI.h"
#include "iconutils.h"
#include "FileUtils.h"
#include "pracprops.h"
#include "GlobalDrawingUtils.h"
#include "GlobalReportUtils.h"
#include "NxPropManager.h"
#include "ShowNxErrorFeedbackDlg.h"
#include "NxBackupUtils.h"
#include <afxmt.h>
#include "ZipcodeUtils.h"
#include "opos.h"
#include "CityZipUtils.h"
#include "GlobalStringUtils.h"
#include "LetterWriting.h"
#include "HistoryUtils.h"
#include "InternationalUtils.h"
#include "SingleSelectDlg.h"
#include "SpellExUtils.h"
#include "HL7Utils.h"
#include "CCDUtils.h"
#include "CCDInterface.h" //(e.lally 2010-02-18) PLID 37438
#include "WellnessDataUtils.h"
#include "DeleteEMNConfirmDlg.h"
#include "NxXMLUtils.h"
#include "GenericXMLBrowserDlg.h"
#include "DecisionRuleUtils.h"
#include "InternetUtils.h"
#include "OHIPUtils.h"
#include "AlbertaHLINKUtils.h"
#include "WindowlessUtils.h"
#include "CCDAViewerDlg.h"
#include <NxSystemUtilitiesLib/NxHandle.h>
#include <boost/utility/string_ref.hpp>
#include <boost/tokenizer.hpp>
#include <set>
#include "NxDataUtilitiesLib/CLRUtils.h"
#include <boost\lexical_cast.hpp>
#include <NxPracticeSharedLib\SendErrorUtils.h>
#include "NxPracticeSharedLib\ICCPDeviceManager.h"
#include "UserVerifyDlg.h"
#include "DocumentOpener.h"
#include <NxSystemUtilitiesLib\RemoteDesktopServices.h>
#include "WindowUtils.h"
#include "Practice.h"
#include "NxCompressUtils.h"
#include <NxDataUtilitiesLib\UnixTime.h>
#include <NxExceptionLib/NxDebug.h>

//(c.copits 2011-09-22) PLID 45626 - Validate Email Addresses
#import "RegEx.tlb" rename_namespace("RegEx")

#import "NexTech.COM.tlb"


// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2013-04-24 14:57) - PLID 56247 - Consolidated some thumb utility functions up here
namespace Nx
{

// keeps track of expected modified time and the format and quality of the thumb
struct ThumbInfo
{
	// default construct
	ThumbInfo()
		: ftExpectedMod()
		, nFormat(0)
		, nQuality(0)
	{}

	FILETIME		ftExpectedMod;
	long			nFormat;		/* using CxImage enum, ie CXIMAGE_FORMAT_PNG, CXIMAGE_FORMAT_JPG, etc */
	unsigned long	nQuality;	/* ignored or set to 0 when irrelevant */


	// safe bool
	typedef void (ThumbInfo::*unspecified_bool_type)() const;
	operator unspecified_bool_type() const
	{
		if (ftExpectedMod.dwHighDateTime == 0 && ftExpectedMod.dwLowDateTime == 0) {
			return NULL;
		} else {
			return &ThumbInfo::bool_helper;
		}
	}

protected:

	void bool_helper() const {}
};


// (a.walling 2013-04-24 14:57) - PLID 56247 - Determine whether ThumbInfo is up to date 
// with the modified time given and thumb settings
bool ThumbUpToDate(FILETIME ftCurrentMod, const ThumbInfo& info)
{
	try {
		if (0 != FileUtils::CompareFileModifiedTimes(info.ftExpectedMod, ftCurrentMod)) {
			return false;
		}

		// mod times are accurate, how about the compression parameters?
		unsigned long nQuality = GetRemotePropertyInt("JPEGThumbnailQuality", 75, 0, "<None>", true);
		long nFormat;

		if (GetRemotePropertyInt("LosslessThumbnail", 1, 0, "<None>", true) == 1) {
			nFormat = CXIMAGE_FORMAT_PNG;
		}
		else {
			nFormat = CXIMAGE_FORMAT_JPG;
			if (nQuality != info.nQuality) {
				return false;
			}
		}

		if (nFormat != info.nFormat) {
			return false;
		}

		return true;
	} catch(...) {
		Log(NxCatch::GetErrorMessage(__FUNCTION__));
	}

	return false;
}


// (a.walling 2013-04-24 14:57) - PLID 56247 - Load the thumb info into the given struct
bool LoadThumbInfo(const IN CString &strFile, ThumbInfo& thumbInfo)
{
	Nx::Handle hThumbInfo(
		::CreateFile(strFile + ":NxThumbInfo"
			, GENERIC_READ
			, FILE_SHARE_READ // only allow reads
			, NULL
			, OPEN_EXISTING
			, FILE_ATTRIBUTE_NORMAL
			, NULL
		)
	);

	if (!hThumbInfo) {
		return false;
	}

	DWORD dwSize = ::GetFileSize(hThumbInfo, NULL);
	if (dwSize != sizeof(ThumbInfo)) {
		// file sizes do not match
		return false;
	}

	ThumbInfo info;

	DWORD dwRead = 0;
	if (!::ReadFile(hThumbInfo, &info, sizeof(ThumbInfo), &dwRead, NULL)) {
		// failed to read file?
		return false;
	}

	if (dwRead != sizeof(ThumbInfo)) {
		return false;
	}

	thumbInfo = info;

	return true;
}

}


// (a.walling 2013-04-24 14:57) - PLID 56247 - Simple wrapper for CxImage buffers -- calls free() to Destroy
// since the data is allocated via malloc()
struct CxImageData
	: private boost::noncopyable
{
	CxImageData()
		: data(NULL)
		, length(0)
	{}

	~CxImageData()
	{
		Destroy();
	}

	void Destroy()
	{
		if (data) {
			free(data);
			data = NULL;
		}
		length = 0;
	}

	BYTE* data;
	// (a.walling 2013-05-08 16:56) - PLID 56610 - CxImage standardizing with cstdint
	int length;
};

// (a.walling 2007-02-09 16:50) - PLID 22991 - Resample the CxImage and place on black background.
void CreateThumbnail(IN OUT CxImage &xImg, long nThumbWidth, long nThumbHeight);

using namespace ADODB;
using namespace NXTIMELib;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (z.manning 2015-05-20 09:59) - PLID 65971 - Changed to CPracticeLicense
CPracticeLicense *g_pLicense = NULL; //global license properties
extern CNxPropManager g_propManager; // global property manager


// (a.walling 2009-08-11 14:46) - PLID 35180 - Not sure about this one. Might be a good idea for some datalists gone awry with huge
// queries behind them, such as those in the inventory module, but most are probably straight selects with minimal time holding locks.
// Then again, all of our datalists are read-only, so maybe this is a good idea. I'll try enabling it now and revert it later
// if necessary
// (a.walling 2009-09-02 17:33) - PLID 35180 - We won't do this just yet.
//#pragma TODO("PLID 35180 - Defaulting datalist connection to use snapshot isolation so requeries do not hold any locks in the system. Does this have any negative consequences?")
// (a.walling 2010-07-28 13:14) - PLID 39871 - REAL HOLOGRAPHIC SIMULATED EVIL LINCOLN IS BACK!!! I mean, DEFAULT_DATALIST_TO_SNAPSHOT_ISOLATION IS BACK! but now in PracProps.

CString g_strUserPermissions = 
	"SELECT PatientModule, ChangeWarning, CustomLabel, NewPatient, DeletePatient, "
	"ChangePatientID, Medication, DeleteAppointment, DeleteHistory, Insurance, "
	"EditInsuranceList, AddInsuredParty, Quotes, BillingModule, ViewPayment, NewPayment, "
	"EditPayment, NewAdjustment, EditAdjustment, NewRefund, EditRefund, ViewBill, NewBill, "
	"EditBill, HCFA, EBilling, SchedulerModule, EditAppointment, DragAppointment, "
	"PasteAppointment, LetterWritingModule, EditGroup, EditFilter, MarketingModule, EditReferralTree, "
	"FinancialModule, InventoryModule, NewItem, DeleteItem, EditItem, PlaceOrder, EditOrder, "
	"MarkReceived, DeleteReceived, ContactModule, NewContact, DeleteContact, ReportModule, "
	/*"AdminReportTab, MarketingReportTab, InventoryReportTab, FinancialLowSecurity, "*/
	/*"FinancialHighSecurity, ContactReportTab, PatientReportTab,*/" AdministratorModule, "
	"PracticeConfig, EmployeeConfig, ProviderConfig, ReferringPhysicianConfig, HCFAConfig, "
	"BillingConfig, SurgeryConfig, MultiFeeConfig, ScheduleConfig, InterestConfig, "
	"ZipCodeConfig,	MirrorIntegration, ViewImage, ImportMirror, ExportMirror, "
	"InformIntegration, ImportInform, ExportInform, AuditingAdminTab, AuditingManagement, "
	"UnitedIntegration, UnitedImport, UnitedExport, ChangePatientName "
	"FROM UserPermissionsT";

// (a.walling 2008-10-02 17:22) - PLID 31575 - Revert to classic border drawing if true
// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
//BOOL g_bClassicToolbarBorders = TRUE;

// (a.walling 2007-10-12 14:34) - PLID 26342 - Moved this to a shared string rather than update various different places in practice
// also included macro-enabled version of 2007 documents
const char* szCommonlyAttached =
			"Commonly Attached (*.doc, *.docx, *.docm, *.xls, *.xlsx, *.xlsm, *.jpg, *.jpeg, *.bmp, *.pcx, *.tiff, *.wav, *.pdf)|*.doc;*.docx;*.docm;*.xls;*.xlsx;*.xlsm;*.xlsb;*.jpg;*.jpeg;*.bmp;*.pcx;*.tiff;*.wav;*.pdf|"
			"Microsoft Word Documents (*.doc, *.docx, *.doxm)|*.doc;*.docx;*.docm|"
			"Microsoft Excel Spreadsheet (*.xls, *.xlsx, *.xlsm, *.xlsb)|*.xls;*.xlsx;*.xlsm;*.xlsb|"
			"Microsoft Power Point Presentation (*.ppt, *.pptx, *.pptm, *.ppsx, *.ppsm)|*.ppt;*.pptx;*.pptm;*.ppsx;*.ppsm|"
			"Adobe Acrobat Files (*.pdf)|*.pdf|"
			"Image (*.bmp, *.jpg, *.jpeg, *.pcx, *.tiff)|*.bmp;*.jpg;*.jpeg;*.pcx;*.tiff|"
			"Sound Recording (*.wav)|*.wav|"
			"All Files (*.*)|*.*|"
			"|";

// (a.walling 2007-10-29 17:26) - PLID 27891 - Global encryption class
NxAES g_NxAES(g_raw128bitKeyInitial, 128);

// (a.walling 2010-05-06 11:52) - PLID 38553 - Sentinel values now in NxDataUtilities SDK
/*
VARIANT GetVariantNull()
{
	VARIANT varNull;
	varNull.vt = VT_NULL;
	return varNull;
}

// (a.walling 2007-08-30 18:00) - PLID 19106
VARIANT GetVariantEmpty()
{
	VARIANT varEmpty;
	varEmpty.vt = VT_EMPTY;
	return varEmpty;
}

// Commonly used variants.  We use these values all over the place, so we 
// might as well save some memory and cpu time by making it so everyone can 
// access the same global instantiations of them.
extern const _variant_t g_cvarTrue(VARIANT_TRUE, VT_BOOL);
extern const _variant_t g_cvarFalse(VARIANT_FALSE, VT_BOOL);
extern const _variant_t g_cvarNull(GetVariantNull());
// (a.walling 2007-08-30 17:59) - PLID 19106
extern const _variant_t g_cvarEmpty(GetVariantEmpty());
*/

// (a.walling 2010-05-03 09:44) - PLID 38553 - Used by shared static lib
CString GetProductVersionText()
{
	return PRODUCT_VERSION_TEXT;
}

// (a.walling 2007-08-08 17:02) - PLID 27017 - Global incrementing variable for session-unique numbers. Rather than
// starting at 0, we'll start at the tick count

volatile long g_nUniqueSessionNum = GetTickCount();

// (a.walling 2007-08-08 17:14) - PLID 27017 - Return the incremented session unique number
DWORD GetUniqueSessionNum()
{
	//WARNING - it is possible that this number will overflow back to its beginning, if it has been called too many times.
	//However, for every situation that is actually using this function at the moment, this should not be an issue
	return InterlockedIncrement(&g_nUniqueSessionNum);
}

long GetActivePatientID()
{
	return GetMainFrame()->m_patToolBar.GetActivePatientID();
}

CString GetActivePatientName()
{
	return GetMainFrame()->m_patToolBar.GetActivePatientName();
}

// only for CCDUtils which looks for this to be implemented in the static lib
CString GetExistingPatientName(long PatientID, ADODB::_Connection* lpCon)
{
	return GetExistingPatientName(PatientID);
}

// (j.jones 2008-10-31 13:20) - PLID 31891 - supported an optional passed-in connection
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
CString GetExistingPatientName(long PatientID)
{
	if (PatientID == -1) {
		return "";
	}

	if(GetMainFrame()) {
		if(GetMainFrame()->m_patToolBar) {
			return GetMainFrame()->m_patToolBar.GetExistingPatientName(PatientID);
		}
	}

	//no mainframe? no pattoolbar? fine, query the recordset
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT [Last] + ', ' + [First] + ' ' + [Middle] AS FullName FROM PersonT WHERE ID = {INT}",PatientID);
	if(!rs->eof) {
		return AdoFldString(rs, "FullName","");
	}

	return "";
}

// (r.farnworth 2016-02-03 08:18) - PLID 68116 - We need the ability to lookup Location Name
CString GetLocationName(long LocationID)
{
	if (LocationID == -1)
	{
		return "";
	}

	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Name FROM LocationsT WHERE ID = {INT}", LocationID);
	if (!rs->eof) {
		return AdoFldString(rs, "Name", "");
	}

	return "";
}

// (j.jones 2007-07-17 09:21) - PLID 26702 - added birthdate functions
COleDateTime GetActivePatientBirthDate()
{
	return GetMainFrame()->m_patToolBar.GetActivePatientBirthDate();
}

// (j.jones 2007-07-17 09:21) - PLID 26702 - added birthdate functions
COleDateTime GetExistingPatientBirthDate(long PatientID)
{
	COleDateTime dtBirthDate;
	dtBirthDate.SetStatus(COleDateTime::invalid);

	if(GetMainFrame()) {
		if(GetMainFrame()->m_patToolBar) {
			return GetMainFrame()->m_patToolBar.GetExistingPatientBirthDate(PatientID);
		}
	}

	//no mainframe? no pattoolbar? fine, query the recordset
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation when querying GetExistingPatientName
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT BirthDate FROM PersonT WHERE ID = {INT}",PatientID);
	if(!rs->eof) {
		return AdoFldDateTime(rs, "BirthDate",dtBirthDate);
	}

	return dtBirthDate;
}

// (z.manning 2009-05-05 10:44) - PLID 28529 - Added user defined ID functions
long GetActivePatientUserDefinedID()
{
	return GetMainFrame()->m_patToolBar.GetActivePatientUserDefinedID();
}

// (z.manning 2009-05-05 10:44) - PLID 28529 - Added user defined ID functions
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
long GetExistingPatientUserDefinedID(const long nPatientID)
{
	if(GetMainFrame()) {
		if(GetMainFrame()->m_patToolBar) {
				return GetMainFrame()->m_patToolBar.GetExistingPatientUserDefinedID(nPatientID);
		}
	}

	//no mainframe? no pattoolbar? fine, query the recordset
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation when querying GetExistingPatientName
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT UserDefinedID FROM PatientsT WHERE PersonID = {INT}", nPatientID);

	if(!rs->eof) {
		return AdoFldLong(rs->GetFields(), "UserDefinedID", -1);
	}

	return -1;
}

// (a.wilson 2013-01-10 10:02) - PLID 54515 - get the patient's internal id based on their userdefinedid
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
long GetExistingPatientIDByUserDefinedID(long nPatientUserDefinedID)
{
	CMainFrame *pMainFrame = GetMainFrame();
	if(pMainFrame){
		if(pMainFrame->m_patToolBar){
			return pMainFrame->m_patToolBar.GetExistingPatientIDByUserDefinedID(nPatientUserDefinedID);
		}
	}
	//no mainframe? no pattoolbar? fine, query the recordset
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation when querying GetExistingPatientName
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PersonID FROM PatientsT WHERE UserDefinedID = {INT}", nPatientUserDefinedID);
	if (!rs->eof) {
		return AdoFldLong(rs->GetFields(), "PersonID", -1);
	}

	return -1;
}

// (b.savon 2011-11-16 15:28) - PLID 45433 - Thread safe, Does Patient Exist?  For Device Import
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
BOOL DoesPatientExistByUserDefinedID(const long nUserDefinedID)
{
	if(GetMainFrame()){
		if(GetMainFrame()->m_patToolBar){
			return GetMainFrame()->m_patToolBar.DoesPatientExistByUserDefinedID(nUserDefinedID);
		}
	}

	//no mainframe? no pattoolbar? fine, query the recordset
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation when querying GetExistingPatientName
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT UserDefinedID FROM PatientsT WHERE UserDefinedID = {INT}", nUserDefinedID);
	if(!rs->eof) {
		return AdoFldLong(rs->GetFields(), "UserDefinedID", -1) == -1 ? FALSE : TRUE;
	}

	return FALSE;
}

long GetActiveContactID()
{
	//DRT 1/12/2004 - Generally this is not an issue, but sometimes when creating
	//	ttx files for reports, you are not able to GetMainFrame().  This is a safer
	//	call anyways.
	if(GetMainFrame())
		return GetMainFrame()->m_contactToolBar.GetActiveContact();
	else
		return -1;
}

CString GetActiveContactName()
{
	return GetMainFrame()->m_contactToolBar.GetActiveContactName();
}

CString GetExistingContactName(long PersonID)
{
	return GetMainFrame()->m_contactToolBar.GetExistingContactName(PersonID);
}

// (c.haag 2009-05-07 15:58) - PLID 28561 - This function returns a username given a valid UserID
CString GetExistingUserName(long PersonID)
{
	return GetMainFrame()->m_contactToolBar.GetExistingUserName(PersonID);
}

// orig: //const LPCTSTR gc_strResBoxFmt = _T("%1b[pName   ]%0b([rMoveUpConfirmed)   ]%0b([rShowState)   ]%3b[rPurpType   ]%3b[rPurpose   ]%0b[rNotes   ]%0bID:[ExtraInfo0  ]%0bH:[ExtraInfo1   ]%0bW:[ExtraInfo2   ]%0bBD:[ExtraInfo3   ]%0b[ExtraInfo4   ]%0b[ExtraInfo5]");
//static const LPCTSTR c_strResBoxFmtDefault = _T("%1b[pName   ]%0b([rMoveUpConfirmed)   ]%0b([rShowState)   ]%3b[rPurpType   ]%3b[rPurpose   ]%2b[rLocationName   ]%0b[rNotes   ]%0bID:[ExtraInfo0  ]%0bH:[ExtraInfo1   ]%0bW:[ExtraInfo2   ]%0bBD:[ExtraInfo3   ]%0b[ExtraInfo4   ]%0b[ExtraInfo5]");
//static const LPCTSTR c_strResBoxFmtNoLocation = _T("%1b[pName   ]%0b([rMoveUpConfirmed)   ]%0b([rShowState)   ]%3b[rPurpType   ]%3b[rPurpose   ]%0b[rNotes   ]%0bID:[ExtraInfo0  ]%0bH:[ExtraInfo1   ]%0bW:[ExtraInfo2   ]%0bBD:[ExtraInfo3   ]%0b[ExtraInfo4   ]%0b[ExtraInfo5]");

static TCHAR l_strResBoxFmt[512] = _T("");
static size_t l_nResBoxFmtLen = 0;
static LPCTSTR l_strResBoxFmtEnd = l_strResBoxFmt;
static bool l_bShowLocName = false;
static bool l_bShowShowStateOnPrint = true;
static BOOL l_bShowPatientInfo = true;
static BOOL l_bShowResourceInfo = false;
static COLORREF l_clrDefaultTextColor = 0;
static int l_nExtraInfoStyles[10];
// (c.haag 2004-10-01 08:23) - PLID 10907 - We now have a "Default Text Color"
// field which defines the standard color of an entire line of text.
void GetResBoxFormat(LPCTSTR *pstrResBoxFmt, LPCTSTR *pstrResBoxFmtEnd, bool bShowLocName, bool bShowShowStateOnPrint, BOOL bShowPatientInfo, BOOL bShowResourceInfo, COLORREF clrDefaultTextColor, int nExtraInfoStyles[10]) 
{
	if(l_bShowLocName != bShowLocName ||
		l_bShowShowStateOnPrint != bShowShowStateOnPrint ||
		l_nResBoxFmtLen == 0 ||
		l_bShowPatientInfo != bShowPatientInfo ||
		l_bShowResourceInfo != bShowResourceInfo ||
		// (c.haag 2006-04-28 14:42) - PLID 20351 - Use a memory comparison, not a pointer comparison!
		memcmp(l_nExtraInfoStyles, nExtraInfoStyles, sizeof(int) * 8) ||
		l_clrDefaultTextColor != clrDefaultTextColor) 
	{
		COLORREF clrGray = GetSysColor(COLOR_GRAYTEXT);
		char szTmp[128];

		l_bShowLocName = bShowLocName;
		l_bShowShowStateOnPrint = bShowShowStateOnPrint;
		l_bShowPatientInfo = bShowPatientInfo;
		l_bShowResourceInfo = bShowResourceInfo;
		l_clrDefaultTextColor = clrDefaultTextColor;
		memcpy(l_nExtraInfoStyles, nExtraInfoStyles, sizeof(int) * 8);
		
		//All responsibility for speed optimization is in the calling function, we don't access data any more.

		// (j.luckoski 2012-06-20 10:57) - PLID 11597 - Add cancelled section after color for all getboxtext functions
		// (j.jones 2014-12-03 10:40) - PLID 64274 - added override section after cancelled
		_snprintf(l_strResBoxFmt, 512, "%%%dc%%1b[pCancel  %%0b]%%1b[pOverride  %%0b]", l_clrDefaultTextColor);

		if(l_bShowPatientInfo) {
			strcat(l_strResBoxFmt, "%1b[pName   %0b]");
		}

		strcat(l_strResBoxFmt, "([rMoveUpConfirmed)   ]");
		if(bShowShowStateOnPrint) {
			strcat(l_strResBoxFmt, "%0h([rShowState)   ]");
		}
		else {
			strcat(l_strResBoxFmt, "%1h([rShowState)   %0h]");
		}

		strcat(l_strResBoxFmt, "[rPurpType   ]%3b[rPurpose   %0b]");

		if(bShowLocName) {
			_snprintf(szTmp, 128, "%%2b[rLocationName   %%0b]");
			strcat(l_strResBoxFmt, szTmp);
			//strcat(l_strResBoxFmt, "%2b[rLocationName   %0b%0c]");
		}

		if(bShowResourceInfo) {
			strcat(l_strResBoxFmt, "[rResourceName	]");
		}

		//NOE: We originally made notes part of the Patient information intentionally, then Chris (thinking that it
		//was a bug), took them out.  After careful consideration, we have decided to leave them out, on the theory
		//that notes aren't _necessarily_ PHI, and that hiding them would be more likely to render the feature
		//unusable to an office then showing them.
		strcat(l_strResBoxFmt, "[rNotes   ]");

		int nBoldIndex = 4;
		if(l_bShowPatientInfo) {
			for(int i = 0; i < 8; i++) {
				CString strFmt;
				if(nExtraInfoStyles[i]) {
					strFmt.Format("%%1b[ExtraInfo%i   %%0b]",/*nBoldIndex,*/i);
					nBoldIndex++;
				}
				else {
					strFmt.Format("[ExtraInfo%i   ]",i);
				}
				strcat(l_strResBoxFmt, strFmt);
			}
			//strcat(l_strResBoxFmt, "[ExtraInfo0  ][ExtraInfo1   ][ExtraInfo2   ][ExtraInfo3   ][ExtraInfo4   ][ExtraInfo5   ][ExtraInfo6   ][ExtraInfo7]");
		}

		l_nResBoxFmtLen = strlen(l_strResBoxFmt);
		l_strResBoxFmtEnd = l_strResBoxFmt + l_nResBoxFmtLen;
	}
	
	// We must at least have pstrResBoxFmt
	ASSERT(pstrResBoxFmt);
	*pstrResBoxFmt = l_strResBoxFmt;
	
	// pstrResBoxFmtEnd is optional
	if (pstrResBoxFmtEnd) {
		*pstrResBoxFmtEnd = l_strResBoxFmtEnd;
	}
}

/*void ReloadResBoxFormat()
{
	//Check user settings, see if we need to use some fancy-shmancy format string.
	if(GetRemotePropertyInt("ResShowLocName", 0, 0, GetCurrentUserName(), true) == 0) {
		SetResBoxFormat(c_strResBoxFmtNoLocation);
	}
	else {
		//We haven't found any applicable user settings, just use the default.
		SetResBoxFormat(c_strResBoxFmtDefault);
	}
}*/

void SetResBoxFormat(const LPCTSTR c_strResBoxFmt)
{
	//Fill in the global variables based on c_strResBoxFmt.
	strncpy(l_strResBoxFmt, c_strResBoxFmt, 512);
	l_nResBoxFmtLen = strlen(l_strResBoxFmt);
	l_strResBoxFmtEnd = l_strResBoxFmt + l_nResBoxFmtLen;
}

////////////////////////////////////////////////////////////////////////////////////////
long g_nCurrentUserID = -1;
HANDLE g_hCurrentUserHandle = NULL;
long g_nInactivityMinutes = -1;
CString g_strCurrentUserName;
CString g_strCurrentUserPassword;
// (j.jones 2008-11-19 10:16) - PLID 28578 - added g_bIsCurrentUserPasswordVerified()
BOOL g_bIsCurrentUserPasswordVerified = FALSE;
BOOL g_bIsAdministrator = FALSE;
// (d.lange 2011-03-29 12:24) - PLID 42987 - added g_bIsCurrentUserTechnician
BOOL g_bIsCurrentUserTechnician = FALSE;

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo
void StoreZipBoxes(CWnd *pWnd, int nCity, int nState, int nZip, 
				   const CString &table, const CString &keyField, int keyVal, 
				   const CString &cityField, const CString &stateField, const CString &zipField)
{
	CString keyValStr;
	keyValStr.Format ("%i", keyVal);
	StoreZipBoxes(pWnd, nCity, nState, nZip, table, keyField, keyValStr, cityField, stateField, zipField);
}

void StoreZipBoxes(CWnd *pWnd, int nCity, int nState, int nZip, 
				   const CString &table, const CString &keyField, const CString &keyVal, 
				   const CString &cityField, const CString &stateField, const CString &zipField)
{
	CString city, state, zip, sql;

	SetZipBoxes(pWnd, nCity, nState, nZip);
	pWnd->GetDlgItemText(nCity, city);
	pWnd->GetDlgItemText(nState, state);
	pWnd->GetDlgItemText(nZip, zip);

	EnsureRemoteData();
	try
	{	ExecuteSql("UPDATE %s "
		"SET %s = '%s', %s = '%s', %s = '%s' "
		"WHERE %s = %s;",
		table, 
		cityField, _Q(city), stateField, _Q(state), zipField, _Q(zip), 
		keyField, keyVal);
	}NxCatchAll("Could not save zip info");
}

void SetZipBoxes(CWnd *pWnd, int nCity, int nState, int nZip)
{
	// (j.gruber 2009-10-07 16:20) - PLID 35825 - support looking up by city
	if (GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>") == 0) {
		CString city, state, zip, temp1, temp2;

		pWnd->GetDlgItemText(nZip,	zip);
		pWnd->GetDlgItemText(nCity, temp1);
		pWnd->GetDlgItemText(nState, temp2);
		temp1.TrimRight();
		temp2.TrimRight();
		// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
		if(!temp1.IsEmpty() || !temp2.IsEmpty()) {
			CWnd* pFocus = pWnd->GetFocus();
			if(pWnd->MessageBox("You have changed the postal code but the city or state already have data in them.  Would you like to overwrite "
				"this data with that of the new postal code?", "Practice", MB_YESNO) == IDYES)
			{
				//Just treat them as empty and the code below will fill them.
				temp1.Empty();
				temp2.Empty();
			}
			// (b.cardillo 2010-08-27 17:56) - PLID 38689 - If the user changed to a different app we won't 
			// have a focus window pointer, so we can't try to use it.
			if (pFocus->GetSafeHwnd() != NULL) {
				pFocus->SetFocus();
			}
		}
		if(temp1 == "" || temp2 == "") {
			// (s.tullis 2013-10-21 10:14) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode
			GetZipInfo(zip, &city, &state);
			if(city == "" && state == ""){					
				CString str;
				str = zip.Left(5);// Get the 5 digit zip code
				GetZipInfo(str, &city, &state);
				// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
				//digit zipcode in the locations tab of Administrator, it looks
				//up the city and state based off the 5 digit code, and then 
				//changes the zip code to 5 digits. It should not change the zip code.
			}

			if (temp1 == "")
				pWnd->SetDlgItemText(nCity, city);

			if (temp2 == "")
				pWnd->SetDlgItemText(nState, state);
		}
	}
	else {
		CString city, state, zip, temp1, temp2;

		pWnd->GetDlgItemText(nZip,	temp1);
		pWnd->GetDlgItemText(nCity, city);
		pWnd->GetDlgItemText(nState, temp2);
		temp1.TrimRight();
		temp2.TrimRight();
		// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
		if(!temp1.IsEmpty() || !temp2.IsEmpty()) {
			CWnd* pFocus = pWnd->GetFocus();
			if(pWnd->MessageBox("You have changed the city but the postal code or state already have data in them.  Would you like to overwrite "
				"this data with that of the new city?", "Practice", MB_YESNO) == IDYES)
			{
				//Just treat them as empty and the code below will fill them.
				temp1.Empty();
				temp2.Empty();
			}
			// (b.cardillo 2010-08-27 17:56) - PLID 38689 - If the user changed to a different app we won't 
			// have a focus window pointer, so we can't try to use it.
			if (pFocus->GetSafeHwnd() != NULL) {
				pFocus->SetFocus();
			}
		}
		if(temp1 == "" || temp2 == "") {			
			GetCityInfo(city, &zip, &state);

			if (temp1 == "")
				pWnd->SetDlgItemText(nZip, zip);

			if (temp2 == "")
				pWnd->SetDlgItemText(nState, state);
		}
	}
		
}


NxZipCode  g_aryZipCodes[5];
static bool g_bUseAreaCode;
static bool g_bUseAreaCodeSet = false;
long g_nZipCodesCount, g_nZipCodesCurrent;
CTableChecker g_ZipCodePrimaryChecker(NetUtils::PrimaryZipCode);


bool QueryZipInfo(CString strSearchZip, CString *City, CString *State, CString *AreaCode) {

	try {		
		// (a.walling 2010-10-04 09:51) - PLID 40573 - Check and update bad static IDs in ZipCodesT
		CheckZipStaticIDs();

		// (a.walling 2006-12-04 15:21) - PLID 23753 - Overhauled the zip code implementation. Now we use our ZipUtils namespace instead of
		//		an access database. See ZipUtils.cpp/h for more info.

		//DRT 8/30/02 - New code for the static zip code list stuff
		//open the mdb file
		/*
		_ConnectionPtr pMDB;
		try{
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
			strPath = strPath ^ "ZipCodeList.mdb";
			pMDB.CreateInstance("ADODB.Connection");
			pMDB->Provider = "Microsoft.Jet.OLEDB.4.0";
			pMDB->Open((LPCTSTR)strPath,"", "",adConnectUnspecified);
		} NxCatchAll("Error opening Zip Code List");
		*/
		
		//first we need to see if a primary is set in our database
			_RecordsetPtr rs = CreateParamRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip FROM ZipCodesT WHERE ZipCode = {STRING} AND PrimaryZip = 1 AND Deleted = 0", strSearchZip);
		

		if(!rs->eof) {
			//we've got a primary!
			//if these fields are null, we have to pull from the mdb, otherwise we take what's in practice
			//_RecordsetPtr rsMDB(__uuidof(Recordset));
			//CString sql;
			//sql.Format("SELECT ID, Zip, City, State, Area, Alias FROM ZipCodeList WHERE Zip = '%s' AND ID = %li", _Q(zipStr), AdoFldLong(rs, "StaticID", -1));

			//rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);

			// (a.walling 2006-12-04 15:22) - PLID 23753 - Return the first zip code in ziZip. < 15ms
			ZipcodeUtils::ZipcodeInfo ziZip;
			ZipcodeUtils::FindByIDAndZipcode(ziZip, AdoFldLong(rs, "StaticID", -1), strSearchZip);
			
			FieldsPtr fields;
			fields = rs->Fields;
			//mdbFields = rsMDB->Fields;
			// (j.gruber 2009-10-05 13:02) - PLID 35607 - added ability to search by city
			if(fields->Item["City"]->Value.vt == VT_BSTR)
				City->Format("%s", VarString(fields->Item["City"]->Value, ""));
			else if(ziZip.IsValid())
				City->Format("%s", ziZip.City);
			else
				City->Empty();
			
			if(fields->Item["State"]->Value.vt == VT_BSTR)
				State->Format("%s", VarString(fields->Item["State"]->Value, ""));
			else if(ziZip.IsValid())
				State->Format("%s", ziZip.State);
			else
				State->Empty();

			if(fields->Item["AreaCode"]->Value.vt == VT_BSTR)
				AreaCode->Format("%s", VarString(fields->Item["AreaCode"]->Value, ""));
			else if(ziZip.IsValid())
				AreaCode->Format("%s", AsString((long)ziZip.nArea));
			else
				AreaCode->Empty();

			rs->Close();
			//rsMDB->Close();
			//pMDB->Close();
			return true;
		}
		else {	//nothing has been set so far, see if they want to be prompted
			//We will prompt them if: a.) they have "Prompt me" checked and b.) there are 2 or more options.

			//CString strSql;
			//strSql.Format("SELECT Count(ID) AS CodeCount FROM ZipCodeList WHERE Zip = '%s'", _Q(zipStr));
			//_RecordsetPtr rsMDB(__uuidof(Recordset));;
			//rsMDB->Open(_bstr_t(strSql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);
			//long nZipCount = rsMDB->eof ? 0 : AdoFldLong(rsMDB, "CodeCount", 0);

			// (a.walling 2006-12-04 15:22) - PLID 23753 - Use our utility to return the count of the zip code. < 15ms.
			// (j.gruber 2009-10-05 13:02) - PLID 35607 - added ability to serach by City
			long nZipCount = -1;
			nZipCount = ZipcodeUtils::GetZipcodeCount(strSearchZip);
			
			if(nZipCount < 2) {
				//OK, now add in the dynamic zip codes.
				_RecordsetPtr rsDynamic = CreateParamRecordset("SELECT Count(UniqueID) AS CodeCount FROM ZipCodesT WHERE ZipCode = {STRING} AND StaticID Is Null AND Deleted = 0", strSearchZip);
				nZipCount += rsDynamic->eof ? 0 : AdoFldLong(rsDynamic, "CodeCount", 0);
			}
			
			if ( (GetRemotePropertyInt("PromptZipCodes",0,0,"<None>",TRUE) == 1) && nZipCount > 1){
				//they do!  Open up the ZipChooser dialog and let them choose the city they want to set for this zip
				CZipChooser dlg(NULL);
				dlg.m_bSearchZip = TRUE;
				dlg.m_strSearchField = strSearchZip;				
				
				if(dlg.DoModal() == IDOK) {
					//they hit ok, so send back their choices
					City->Format("%s", dlg.m_strCity);
					State->Format("%s", dlg.m_strState);
					AreaCode->Format("%s", dlg.m_strArea);
					return true;
				}
				else {
					//cancelled, nothing to see here
					return false;
				}
			}
			else {
				//aww, they don't want to use the fun ZipChooser, so just look up the one that is not aliased for this zip code in the static list
				//open a recordset to get the info
				//PLID 19309 - they might have changed the data in our system, so check if there is a record there
				try{

					//JMM PLID 19309 - I copied this from below, it was checking the MDB and then our database, when
					// it should be checking ours first to see is something changed
					_RecordsetPtr rsZip = CreateParamRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip FROM ZipCodesT WHERE ZipCode = {STRING} AND Deleted = 0", strSearchZip);
					if (! rsZip->eof) {
						//DRT 3/11/03 - Not sure why this wasn't ever put in before, but if we get here, we've found nothing in the static list
						//		We NEED to check the Practice list, to make sure there is nothing there
						//Indeed!  they did change the zipcode, so pull the info from here
						if(rsZip->Fields->Item["City"]->Value.vt == VT_BSTR)
								City->Format("%s", AdoFldString(rsZip, "City", ""));
						if(rsZip->Fields->Item["State"]->Value.vt == VT_BSTR)
							State->Format("%s", AdoFldString(rsZip, "State", ""));
						if(rsZip->Fields->Item["AreaCode"]->Value.vt == VT_BSTR)
							AreaCode->Format("%s", AdoFldString(rsZip, "AreaCode", ""));
						rsZip->Close();
						
						//success!
						return true;

					}
					else {

						//_RecordsetPtr rsMDB(__uuidof(Recordset));
						//CString sql;
						//sql.Format("SELECT ID, Zip, City, State, Area, Alias FROM ZipCodeList WHERE Alias IS NULL AND Zip = '%s'", _Q(zipStr));
				
						//rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);

						// (a.walling 2006-12-04 15:23) - PLID 23753 - Use our ZipUtils to find a non-aliased zip. < 15ms
						ZipcodeUtils::ZipcodeInfo zi;
						ZipcodeUtils::FindByZipcode(zi, strSearchZip, true);
					
						//DRT 11/19/2003 - PLID 5937 - It is possible we deleted this code in the ZipCodeList.  Check that before we do anything final.
						// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
						if(zi.IsValid() && !ReturnsRecordsParam("SELECT UniqueID, StaticID, ZipCode FROM ZipCodesT WHERE StaticID = {INT} AND Deleted = 1", zi.nID)) {
							//we've got a primary!
							//FieldsPtr fields;
							//fields = rsMDB->Fields;
							//if(fields->Item["City"]->Value.vt == VT_BSTR)
							City->Format("%s", zi.City);
							
							//if(fields->Item["State"]->Value.vt == VT_BSTR)
							State->Format("%s", zi.State);
							//if(fields->Item["Area"]->Value.vt == VT_BSTR)
							AreaCode->Format("%s", AsString((long)zi.nArea));
							//rsMDB->Close();
							//pMDB->Close();
							return true;
						}
						//no primary found, just leave the strings blank and return
						//pMDB->Close();
					}
					//failure
					return false;
				} NxCatchAll("Error accessing static Zip code list");
			}
		}

/*	Old code from Pre-static zip codes
		_RecordsetPtr tmpRS = CreateRecordset("SELECT City, State, AreaCode FROM ZipCodes WHERE ZipCode = '" + _Q(zipStr.Left(5)) + "'");
		if (!tmpRS->eof) {	
						
			City->Format("%s", AdoFldString(tmpRS, "City"));
			State->Format("%s", AdoFldString(tmpRS, "State"));
			AreaCode->Format("%s", AdoFldString(tmpRS, "AreaCode"));
	
			tmpRS->Close();
			return true;
		} else {
			tmpRS->Close();
			return false;
		}
*/
	}NxCatchAll("Error in QueryZipInfo");
	return false;
}

// (j.gruber 2009-10-08 12:59) - PLID 35607 - added for city lookup
bool QueryCityInfo(CString strSearchCity, CString *Zip, CString *State, CString *AreaCode) {

	try {

		// (a.walling 2006-12-04 15:21) - PLID 23753 - Overhauled the zip code implementation. Now we use our ZipUtils namespace instead of
		//		an access database. See ZipUtils.cpp/h for more info.

		//DRT 8/30/02 - New code for the static zip code list stuff
		//open the mdb file
		/*
		_ConnectionPtr pMDB;
		try{
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
			strPath = strPath ^ "ZipCodeList.mdb";
			pMDB.CreateInstance("ADODB.Connection");
			pMDB->Provider = "Microsoft.Jet.OLEDB.4.0";
			pMDB->Open((LPCTSTR)strPath,"", "",adConnectUnspecified);
		} NxCatchAll("Error opening Zip Code List");
		*/
		
		//first we need to see if a primary is set in our database
		_RecordsetPtr rs = CreateParamRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip FROM ZipCodesT WHERE City = {STRING} AND PrimaryZip = 1 AND Deleted = 0", strSearchCity);
		
		if(!rs->eof) {
			//we've got a primary!
			//if these fields are null, we have to pull from the mdb, otherwise we take what's in practice
			//_RecordsetPtr rsMDB(__uuidof(Recordset));
			//CString sql;
			//sql.Format("SELECT ID, Zip, City, State, Area, Alias FROM ZipCodeList WHERE Zip = '%s' AND ID = %li", _Q(zipStr), AdoFldLong(rs, "StaticID", -1));

			//rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);

			// (a.walling 2006-12-04 15:22) - PLID 23753 - Return the first zip code in ziZip. < 15ms
			CityZipUtils::CityZipInfo ciCity;
			CityZipUtils::FindByIDAndCity(ciCity, AdoFldLong(rs, "StaticID", -1), strSearchCity);
			
			FieldsPtr fields;
			fields = rs->Fields;
			//mdbFields = rsMDB->Fields;
			// (j.gruber 2009-10-05 13:02) - PLID 35607 - added ability to search by city
			if(fields->Item["ZipCode"]->Value.vt == VT_BSTR)
				Zip->Format("%s", VarString(fields->Item["Zipcode"]->Value, ""));
			else if(ciCity.IsValid())
				Zip->Format("%s", ciCity.Zip);
			else
				Zip->Empty();			

			if(fields->Item["State"]->Value.vt == VT_BSTR)
				State->Format("%s", VarString(fields->Item["State"]->Value, ""));
			else if(ciCity.IsValid())
				State->Format("%s", ciCity.State);
			else
				State->Empty();

			if(fields->Item["AreaCode"]->Value.vt == VT_BSTR)
				AreaCode->Format("%s", VarString(fields->Item["AreaCode"]->Value, ""));
			else if(ciCity.IsValid())
				AreaCode->Format("%s", AsString((long)ciCity.nArea));
			else
				AreaCode->Empty();

			rs->Close();
			//rsMDB->Close();
			//pMDB->Close();
			return true;
		}
		else {	//nothing has been set so far, see if they want to be prompted
			//We will prompt them if: a.) they have "Prompt me" checked and b.) there are 2 or more options.

			//CString strSql;
			//strSql.Format("SELECT Count(ID) AS CodeCount FROM ZipCodeList WHERE Zip = '%s'", _Q(zipStr));
			//_RecordsetPtr rsMDB(__uuidof(Recordset));;
			//rsMDB->Open(_bstr_t(strSql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);
			//long nZipCount = rsMDB->eof ? 0 : AdoFldLong(rsMDB, "CodeCount", 0);

			// (a.walling 2006-12-04 15:22) - PLID 23753 - Use our utility to return the count of the zip code. < 15ms.
			// (j.gruber 2009-10-05 13:02) - PLID 35607 - added ability to serach by City
			long nCityCount = -1;
			nCityCount = CityZipUtils::GetCityCount(strSearchCity);
			
			if(nCityCount < 2) {
				//OK, now add in the dynamic zip codes.
				_RecordsetPtr rsDynamic = CreateParamRecordset("SELECT Count(UniqueID) AS CodeCount FROM ZipCodesT WHERE City = {STRING} AND StaticID Is Null AND Deleted = 0", strSearchCity);
				nCityCount += rsDynamic->eof ? 0 : AdoFldLong(rsDynamic, "CodeCount", 0);
			}

			// (j.gruber 2009-10-05 13:05) - PLID 35607 - if they are searching by city, always pop up the box
			if( nCityCount > 1 ) {
				//they do!  Open up the ZipChooser dialog and let them choose the city they want to set for this zip
				CZipChooser dlg(NULL);
				dlg.m_bSearchZip = FALSE;
				dlg.m_strSearchField = strSearchCity;				
				
				if(dlg.DoModal() == IDOK) {
					//they hit ok, so send back their choices
					Zip->Format("%s", dlg.m_strZip);
					State->Format("%s", dlg.m_strState);
					AreaCode->Format("%s", dlg.m_strArea);
					return true;
				}
				else {
					//cancelled, nothing to see here
					return false;
				}
			}
			else {
				//aww, they don't want to use the fun ZipChooser, so just look up the one that is not aliased for this zip code in the static list
				//open a recordset to get the info
				//PLID 19309 - they might have changed the data in our system, so check if there is a record there
				try{

					//JMM PLID 19309 - I copied this from below, it was checking the MDB and then our database, when
					// it should be checking ours first to see is something changed
					_RecordsetPtr rsZip = CreateParamRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip FROM ZipCodesT WHERE City = {STRING} AND Deleted = 0", strSearchCity);
					if (! rsZip->eof) {
						//DRT 3/11/03 - Not sure why this wasn't ever put in before, but if we get here, we've found nothing in the static list
						//		We NEED to check the Practice list, to make sure there is nothing there
						//Indeed!  they did change the zipcode, so pull the info from here
						if(rsZip->Fields->Item["ZipCode"]->Value.vt == VT_BSTR)
							Zip->Format("%s", AdoFldString(rsZip, "ZipCode", ""));
					
						if(rsZip->Fields->Item["State"]->Value.vt == VT_BSTR)
							State->Format("%s", AdoFldString(rsZip, "State", ""));
						if(rsZip->Fields->Item["AreaCode"]->Value.vt == VT_BSTR)
							AreaCode->Format("%s", AdoFldString(rsZip, "AreaCode", ""));
						rsZip->Close();
						
						//success!
						return true;

					}
					else {

						//_RecordsetPtr rsMDB(__uuidof(Recordset));
						//CString sql;
						//sql.Format("SELECT ID, Zip, City, State, Area, Alias FROM ZipCodeList WHERE Alias IS NULL AND Zip = '%s'", _Q(zipStr));
				
						//rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);

						// (a.walling 2006-12-04 15:23) - PLID 23753 - Use our ZipUtils to find a non-aliased zip. < 15ms
						CityZipUtils::CityZipInfo ci;
						CityZipUtils::FindByCity(ci, strSearchCity, true);
						

						//DRT 11/19/2003 - PLID 5937 - It is possible we deleted this code in the ZipCodeList.  Check that before we do anything final.
						// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
						if(ci.IsValid() && !ReturnsRecordsParam("SELECT UniqueID, StaticID, ZipCode FROM ZipCodesT WHERE StaticID = {INT} AND Deleted = 1", ci.nID)) {
							//we've got a primary!
							//FieldsPtr fields;
							//fields = rsMDB->Fields;
							//if(fields->Item["City"]->Value.vt == VT_BSTR)
							Zip->Format("%s", ci.Zip);							

							//if(fields->Item["State"]->Value.vt == VT_BSTR)
							State->Format("%s", ci.State);
							//if(fields->Item["Area"]->Value.vt == VT_BSTR)
							AreaCode->Format("%s", AsString((long)ci.nArea));
							//rsMDB->Close();
							//pMDB->Close();
							return true;
						}
						//no primary found, just leave the strings blank and return
						//pMDB->Close();
					}
					//failure
					return false;
				} NxCatchAll("Error accessing static City code list");
			}
		}

/*	Old code from Pre-static zip codes
		_RecordsetPtr tmpRS = CreateRecordset("SELECT City, State, AreaCode FROM ZipCodes WHERE ZipCode = '" + _Q(zipStr.Left(5)) + "'");
		if (!tmpRS->eof) {	
						
			City->Format("%s", AdoFldString(tmpRS, "City"));
			State->Format("%s", AdoFldString(tmpRS, "State"));
			AreaCode->Format("%s", AdoFldString(tmpRS, "AreaCode"));
	
			tmpRS->Close();
			return true;
		} else {
			tmpRS->Close();
			return false;
		}
*/
	}NxCatchAll("Error in QueryCityInfo");
	return false;

}

// (j.gruber 2009-10-05 12:56) - PLID 35607 - changed to use city if they ahve the preference
bool QueryZiporCityInfo(CString strSearchZiporCity, CString *ZiporCity, CString *State, CString *AreaCode, BOOL bSearchZip) {

	if (bSearchZip) {
		return QueryZipInfo(strSearchZiporCity, ZiporCity, State, AreaCode);
	}
	else {
		return QueryCityInfo(strSearchZiporCity, ZiporCity, State, AreaCode);
	}
}

// (a.walling 2010-10-04 09:51) - PLID 40573 - Check and update bad static IDs in ZipCodesT
void CheckZipStaticIDs()
{
	static bool bCheckingZipStaticIDs = false;

	// prevent reentry
	if (bCheckingZipStaticIDs) {
		return;
	}

	try {
		bCheckingZipStaticIDs = true;
		if (GetRemotePropertyInt("ZipCodes_CheckedStaticIDs", FALSE, 0, "<None>", true)) {			
			bCheckingZipStaticIDs = false;
			return;
		}

		// (a.walling 2010-11-12 13:03) - no PLID - In debug mode this would raise an assertion due to including the special debug heap codes.
		// However that is intended, and if the guards were in place initially we never would have had this problem to begin with. Luckily this
		// will only run once per database. But to avoid future confusion, I am going to modify this to use the hex values converted to an int
		// so the assertion will not trip.
		_RecordsetPtr prs = CreateRecordsetStd(FormatString("SELECT * FROM ZipCodesT WHERE StaticID IN (CONVERT(INT, 0x%08x), CONVERT(INT, 0x%08x))", 0xCCCCCCCC, 0xCDCDCDCD), 
			adOpenForwardOnly, adLockOptimistic, adCmdText, adUseServer);

		while (!prs->eof) {
			_variant_t varNewStaticID = g_cvarNull;

			CString strZip = AdoFldString(prs, "ZipCode", "");
			CString strCity = AdoFldString(prs, "City", "");

			if (!strZip.IsEmpty() && !strCity.IsEmpty()) {
				// (d.singleton 2011-11-18 09:49) - PLID 18875 - need to pass a BOOL & to ListAll() so make one
				BOOL bFailure;
				ZipcodeUtils::ZipcodeSet* pZs = ZipcodeUtils::ListAll(bFailure, INVALID_HANDLE_VALUE, ZipcodeUtils::zfZip, strZip);

				if (pZs) {
					for (int i = 0; i < pZs->GetSize(); i++) {
						ZipcodeUtils::ZipcodeInfo* pInfo = pZs->GetAt(i);

						if (pInfo->nID == 0xCCCCCCCC || pInfo->nID == 0xCDCDCDCD) {
							ThrowNxException("Please ensure you are using the latest zip code database");
						}

						if (pInfo->nID != 0  && strCity.CompareNoCase(pInfo->City) == 0) {
							varNewStaticID = long(pInfo->nID);
							break;
						}
					}
					ZipcodeUtils::ClearZipcodeSet(pZs);
				}
			}

			prs->Update("StaticID", varNewStaticID);

			prs->MoveNext();
		}

		SetRemotePropertyInt("ZipCodes_CheckedStaticIDs", TRUE, 0, "<None>");
		bCheckingZipStaticIDs = false;
	} NxCatchAllCallThrow("Error checking zip code static IDs", {bCheckingZipStaticIDs = false;} );
}

void SetShowAreaCode(bool bShowAreaCode) {

	g_bUseAreaCode = bShowAreaCode;
	SetRemotePropertyInt("UseAreaCode", bShowAreaCode, 0, "<None>");

}


bool ShowAreaCode() {

	//check to see if this has already been set
	if (!g_bUseAreaCodeSet) {

		//set the zipcode global variable so that we don't have to keep checking configRT for it
		g_bUseAreaCodeSet = true;
		long nUseAreaCode;
		nUseAreaCode = GetRemotePropertyInt("UseAreaCode", 1, 0, "<None>");
		if (nUseAreaCode == 0) {
			g_bUseAreaCode = false;
		}
		else {
			g_bUseAreaCode = true;
		}
	}

	return g_bUseAreaCode;
}

bool GetZipInfo(CString zipStr, CString *City, CString *State, CString *AreaCode /*= NULL*/) {

	return GetZiporCityInfo(zipStr, City, State, AreaCode, TRUE);
}

bool GetCityInfo(CString strSearchCity, CString *Zip, CString *State, CString *AreaCode /*= NULL*/) {

	return GetZiporCityInfo(strSearchCity, Zip, State, AreaCode, FALSE);
}


// (j.gruber 2009-10-05 12:40) - PLID 35607 - changed to lookup either city or zip
bool GetZiporCityInfo(CString strSearchZiporCity, CString *ZiporCity, CString *State, CString *AreaCode /*= NULL*/, BOOL bSearchZipCode /*=TRUE*/)
{

	//DRT 1/31/03 - If we changed the primary zip code, then this array may contain an incorrect value - also, someone else on
	//			the network may have changed it.  If we got a tablechecker message about the primary changing, then just wipe out
	//			all the saved items.
	if(g_ZipCodePrimaryChecker.Changed()) {
		g_nZipCodesCurrent = 0;
		g_nZipCodesCount = 0;
		for(int x = 0; x < 5; x++) {
			g_aryZipCodes[x].strAreaCode = "";
			g_aryZipCodes[x].strCity = "";
			g_aryZipCodes[x].strState = "";
			g_aryZipCodes[x].strZip = "";
		}
	}

	//ok, lets try this wrapping backwards thing
	for (int i= g_nZipCodesCurrent; i >= 0; i--) {
		// (j.gruber 2009-10-05 12:43) - PLID 35607 - changed to search either zip or city
		if (bSearchZipCode) {
			if (g_aryZipCodes[i].strZip ==  strSearchZiporCity) {
				//fill the info
				if (ZiporCity != NULL ) {
					ZiporCity->Format("%s", g_aryZipCodes[i].strCity);
				}
				if (State != NULL) {
					State->Format("%s", g_aryZipCodes[i].strState);
				}
				if (AreaCode != NULL) {
					AreaCode->Format("%s", g_aryZipCodes[i].strAreaCode);
					if(AreaCode->IsEmpty())
						AreaCode->Format("%s",GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
				}
				return true;
			}
		}
		else {
			if (g_aryZipCodes[i].strCity ==  strSearchZiporCity) {
				//fill the info
				if (ZiporCity != NULL ) {
					ZiporCity->Format("%s", g_aryZipCodes[i].strZip);
				}
				if (State != NULL) {
					State->Format("%s", g_aryZipCodes[i].strState);
				}
				if (AreaCode != NULL) {
					AreaCode->Format("%s", g_aryZipCodes[i].strAreaCode);
					if(AreaCode->IsEmpty())
						AreaCode->Format("%s",GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
				}
				return true;
			}
		}

	}

	//if we got here, we have to start at 5 and reloop
	for (int g = g_nZipCodesCount;  g >= g_nZipCodesCurrent; g--) {
		if (bSearchZipCode) {
			if (g_aryZipCodes[g].strZip ==  strSearchZiporCity) {
				//fill the info
				if (ZiporCity != NULL) {
					ZiporCity->Format("%s", g_aryZipCodes[g].strCity);
				}
				if (State != NULL) {
					State->Format("%s",	g_aryZipCodes[g].strState);
				}
				if (AreaCode != NULL) {
					AreaCode->Format("%s", g_aryZipCodes[g].strAreaCode);
					if(AreaCode->IsEmpty())
						AreaCode->Format("%s",GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
				}
				return true;
			}
		}else {
			if (g_aryZipCodes[g].strCity ==  strSearchZiporCity) {
				//fill the info
				if (ZiporCity != NULL) {
					ZiporCity->Format("%s", g_aryZipCodes[g].strZip);
				}
				if (State != NULL) {
					State->Format("%s",	g_aryZipCodes[g].strState);
				}
				if (AreaCode != NULL) {
					AreaCode->Format("%s", g_aryZipCodes[g].strAreaCode);
					if(AreaCode->IsEmpty())
						AreaCode->Format("%s",GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
				}
				return true;
			}
		}

	}


	//if we didn't find it, we have to get it from the data
	CString strZiporCity, strState, strAreaCode;

	if (QueryZiporCityInfo(strSearchZiporCity, &strZiporCity, &strState, &strAreaCode, bSearchZipCode)) {

		//set the return variables		
		if (ZiporCity != NULL) {
			ZiporCity->Format("%s", strZiporCity);
		}

		if (State != NULL) {
			State->Format("%s", strState);
		}
		
		if (AreaCode != NULL) {
			AreaCode->Format("%s", strAreaCode);
			if(AreaCode->IsEmpty())
				AreaCode->Format("%s",GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
		}

		
		//fill the array 
		if (bSearchZipCode) {
			g_aryZipCodes[g_nZipCodesCurrent].strZip = strSearchZiporCity;
			g_aryZipCodes[g_nZipCodesCurrent].strCity = strZiporCity;
		}
		else {
			g_aryZipCodes[g_nZipCodesCurrent].strZip = strZiporCity;
			g_aryZipCodes[g_nZipCodesCurrent].strCity = strSearchZiporCity;
		}
		
		g_aryZipCodes[g_nZipCodesCurrent].strState = strState;
		g_aryZipCodes[g_nZipCodesCurrent].strAreaCode = strAreaCode;
		
		//increment the array			
		//find out what number we are on because the max is 5
		if (g_nZipCodesCount < 4 ) {
			//increment it
			g_nZipCodesCount++;
		}

		//increment the current count
		if (g_nZipCodesCurrent == 4) {
			g_nZipCodesCurrent = 0;
		}
		else {
			g_nZipCodesCurrent++;
		}

		return true;
	}
	else {
		
		//if they are only pulling the area code, then we can send the default area code
		if(ZiporCity == NULL && State == NULL && AreaCode != NULL) {
			AreaCode->Format("%s",GetRemotePropertyText("DefaultAreaCode","",0,"<None>",TRUE));
			return true;
		}
		else {
			return false;
		}
	}
	
	
}

struct
{
	EBuiltInObjectIDs obj;
	unsigned long perms; // Regular permissions (the with-pass perms will be calculated from these because all old SecurityItems supported passwords)
} g_arySecurityItemToBuiltInObject[] = {
	
	{ bioPatientsModule,		SPT_V_________ },	// PatientModule
	{ bioPatientWarning,		SPT___W_______ },	// ChangeWarning
	{ bioPatientsCustomLabel,	SPT___W_______ },	// CustomLabel
	{ bioPatient,				SPT____C______ },	// NewPatient
	{ bioPatient,				SPT_____D_____ },	// DeletePatient
	{ bioPatientID,				SPT___W_______ },	// ChangePatientID
	{ bioPatientMedication,		SPT_VRWCD_____ },	// Medication
	{ bioAppointment,			SPT_____D_____ },	// DeleteAppointment
	{ bioPatientHistory,		SPT_____D_____ },	// DeleteHistory
	{ bioPatientInsurance,		SPT_VRW_______ },	// Insurance
	{ bioInsuranceCo,			SPT__RWCD_____ },	// EditInsuranceList
	{ bioPatientInsurance,		SPT____C______ },	// AddInsuredParty
	{ bioPatientQuotes,			SPT_VRW_______ },	// Quotes
	{ bioPatientBilling,		SPT_VRW_______ },	// BillingModule
	{ bioPayment,				SPT_V_________ },	// ViewPayment
	{ bioPayment,				SPT____C______ },	// NewPayment
	{ bioPayment,				SPT___W_______ },	// EditPayment
	{ bioAdjustment,			SPT____C______ },	// NewAdjustment
	{ bioAdjustment,			SPT___W_______ },	// EditAdjustment
	{ bioRefund,				SPT____C______ },	// NewRefund
	{ bioRefund,				SPT___W_______ },	// EditRefund
	{ bioBill,					SPT_VR________ },	// ViewBill
	{ bioBill,					SPT____C______ },	// NewBill
	{ bioBill,					SPT___W_______ },	// EditBill
	{ bioClaimForms,			SPT_VRW_______ },	// HCFA
	{ bioEBilling,				SPT_VRW_______ },	// EBilling
	{ bioSchedulerModule,		SPT_V_________ },	// SchedulerModule
	{ bioAppointment,			SPT___W_______ },	// EditAppointment
	{ bioAppointment,			SPT______0____ },	// DragAppointment
	{ bioAppointment,			SPT_______1___ },	// PasteAppointment
	{ bioLetterWritingModule,	SPT_V_________ },	// LetterWritingModule
	{ bioLWGroup,				SPT___W_______ },	// EditGroup
	{ bioLWFilter,				SPT___W_______ },	// EditFilter
	{ bioMarketingModule,		SPT_V_________ },	// MarketingModule
	{ bioReferralTree,			SPT___W_______ },	// EditReferralTree
	{ bioFinancialModule,		SPT_V_________ },	// FinancialModule
	{ bioInventoryModule,		SPT_V_________ },	// InventoryModule
	{ bioInvItem,				SPT____C______ },	// NewItem
	{ bioInvItem,				SPT_____D_____ },	// DeleteItem
	{ bioInvItem,				SPT___W_______ },	// EditItem
	{ bioInvOrder,				SPT____C______ },	// PlaceOrder
	{ bioInvOrder,				SPT___W_______ },	// EditOrder
	{ bioInvOrderReceived,		SPT___W_______ },	// MarkReceived
	{ bioInvOrder,				SPT_____D_____ },	// DeleteReceived
	{ bioContactsModule,		SPT_V_________ },	// ContactModule
	{ bioContact,				SPT____C______ },	// NewContact
	{ bioContact,				SPT_____D_____ },	// DeleteContact
	{ bioReportsModule,			SPT_V_________ },	// ReportModule
	//{ bioReportsAdminTab,		SPT_V_________ },	// AdminReportTab
	//{ bioReportsMarketingTab,	SPT_V_________ },	// MarketingReportTab
	//{ bioReportsInventoryTab,	SPT_V_________ },	// InventoryReportTab
	//{ bioReportsFinancialLow,	SPT_V_________ },	// FinancialLowSecurity
	//{ bioReportsFinancialHigh,	SPT_V_________ },	// FinancialHighSecurity
	//{ bioReportsContactTab,		SPT_V_________ },	// ContactReportTab
	//{ bioReportsPatientTab,		SPT_V_________ },	// PatientReportTab
	{ bioAdministratorModule,	SPT_V_________ },	// AdministratorModule
	{ bioAdminLocations,		SPT_VRW_______ },	// PracticeConfig
	{ bioContactsUsers,			SPT_VRW_______ },	// EmployeeConfig
	{ bioContactsProviders,		SPT_VRW_______ },	// ProviderConfig
	{ bioContactsRefPhys,		SPT_VRW_______ },	// ReferringPhysicianConfig
	{ bioAdminHCFA,				SPT_VRW_______ },	// HCFAConfig
	{ bioAdminBilling,			SPT_VRW_______ },	// BillingConfig
	{ bioAdminSurgery,			SPT_VRW_______ },	// SurgeryConfig
	{ bioAdminMultiFee,			SPT_VRW_______ },	// MultiFeeConfig
	{ bioAdminScheduler,		SPT_VRW_______ },	// ScheduleConfig
	{ bioInvalidID,				SPT_VRWCD01234 },	// InterestConfig // this SecurityItem wasn't used before and so doesn't map to a Built-In Object
	{ bioAdminZipCode,			SPT_VRW_______ },	// ZipCodeConfig
	{ bioMirrorIntegration,		SPT_V_W_______ },	// MirrorIntegration
	{ bioPatientImage,			SPT_V_________ },	// ViewImage
	{ bioMirrorIntegration,		SPT______0____ },	// ImportMirror
	{ bioMirrorIntegration,		SPT_______1___ },	// ExportMirror
	{ bioInformIntegration,		SPT_V_W_______ },	// InformIntegration
	{ bioInformIntegration,		SPT______0____ },	// ImportInform
	{ bioInformIntegration,		SPT_______1___ },	// ExportInform
	{ bioAdminAuditTrail,		SPT_VR________ },	// AuditingAdminTab
	{ bioAdminAuditTrail,		SPT___W_D_____ },	// AuditingManagement
	{ bioUnitedIntegration,		SPT_V_W_______ },	// UnitedIntegration
	{ bioUnitedIntegration,		SPT______0____ },	// UnitedImport
	{ bioUnitedIntegration,		SPT_______1___ },	// UnitedExport
	{ bioPatientName,			SPT_V_W_______ },	// ChangePatientName
	{ bioPreferences,			SPT_V_________ },	// Preferences
	
	{ bioInvalidID,				SPT_VRWCD01234 }
};

void SecurityItemToBuiltInObject(IN SecurityItem siItem, OUT EBuiltInObjectIDs *pbioObject, OUT unsigned long *pnPermissions)
{
	ASSERT(siItem >= 0 && siItem < SecuritySize);

	if (siItem >= 0 && siItem < SecuritySize) {
		// Assert that the permission doesn't contain any WithPass flags
		ASSERT(((g_arySecurityItemToBuiltInObject[siItem].perms) & (SPT_VRWCD01234_ONLYWITHPASS)) == 0);
		// Assert that the permission shifted left by 1 doesn't contain any non-WithPass flags
		ASSERT(((g_arySecurityItemToBuiltInObject[siItem].perms << 1) & (SPT_VRWCD01234)) == 0);

		// Set the output parameters
		*pbioObject = g_arySecurityItemToBuiltInObject[siItem].obj;
		*pnPermissions = g_arySecurityItemToBuiltInObject[siItem].perms;
	} else {
		// Invalid siItem
		ASSERT(FALSE);
		*pbioObject = bioInvalidID;
		*pnPermissions = -1;
	}
}

int CObsoleteSecurityItemLookup::operator [](long siItem) const
{
	// Get the built-in object and required permissions mapped from the old security item enum
	EBuiltInObjectIDs eBuiltInID;
	unsigned long nPermissions;
	SecurityItemToBuiltInObject((SecurityItem)siItem, &eBuiltInID, &nPermissions);

	// Check the permissions
	CPermissions perms;
	GetPermissions(GetCurrentUserHandle(), eBuiltInID, FALSE, 0, perms);
	if ((perms.nPermissions & nPermissions) == nPermissions) {
		// They have full password-free access
		return 1;
	} else {
		unsigned long nPermissionsWithPass = (nPermissions << 1);
		if ((perms.nPermissions & nPermissionsWithPass) == nPermissionsWithPass) {
			// They have full passworded access
			return 2;
		} else {
			// They have partial or no access
			return 0;
		}
	}
}

CObsoleteSecurityItemLookup g_userPermission;

int UserPermission(SecurityItem item)
{
	// Get the built-in object and required permissions mapped from the old security item enum
	EBuiltInObjectIDs bioObject;
	unsigned long nPermissions;
	SecurityItemToBuiltInObject(item, &bioObject, &nPermissions);

	if (CheckCurrentUserPermissions(bioObject, nPermissions, FALSE, 0)) {
		return 1;
	} else {
		return 0;
	}
}

// Security stuff
/*
bool CheckAccess(LPCTSTR strModule, LPCTSTR strFunction /* = NULL *//*)
{
	bool Ans = false;
	
	// Make sure a user is currently logged in
	if (!EnsureCurrentUser()) return false;

	// Once a user is logged in check security
	if (DoIHaveAccess(strModule, strFunction)) {
		if (IsSecure(strModule, strFunction)) {
			bool bContinue = true;
			CString strPass;
			while (bContinue) {
				bContinue = AskPassword(strPass);
				if (bContinue) {
					strPass.MakeUpper();
					if ((strPass == (CString)GetCurrentUserPassword()) || (strPass == (CString)GetSecretPassword())) {
						bContinue = false;
						Ans = true;
					} else {
						MsgBox("You have not entered the correct\npassword for the logged in user.");
					}
				}
			}
			return Ans;
		} else {
			Ans = true;
		}
	}

	if (!Ans) {
		if (strFunction) {
			MsgBox(CString("I'm sorry, you do not have access to the\n") +
				CString("%s function of the %s module.\n\n") + 
				CString("Please contact your office manager\n") + 
				CString("if you need access to this function."), 
				strFunction, strModule);
		} else {
			MsgBox(CString("I'm sorry, you do not have access to %s.\n\n") +
				CString("Please contact your office manager for assistance."), 
				strModule);
		}
	}

	return Ans;
}

bool DoIHaveAccess(LPCTSTR strModule, LPCTSTR strFunction)
{
	if (strcmp(GetCurrentUserName(), DEVELOPER_USERNAME) == 0 && 
		strcmp(GetCurrentUserPassword(), GetSecretPassword()) == 0) {
		return true;
	}

	CDaoRecordset rs(&g_dbPractice);
	CString strMod(strModule), strFunc;
	CString strSQL;
///    CUserPrivileges recUserPriv(&g_dbPractice);
	bool bAns = true; // return value (default true)
	// Make sure strFunc contains either what was passed, or the default
	strFunc = (strFunction && strFunction[0]) ? strFunction : "Enter Module";
	
	// Check to see if the specified module and function are securable
	// If they are not securable, then why was this function called? 
	// So let's add it
	strSQL.Format("SELECT * FROM UserPrivilegesItemsT WHERE ModuleName = '%s' AND ItemName = '%s';", strMod, strFunc);
	rs.Open(AFX_DAO_USE_DEFAULT_TYPE, strSQL);
	if (rs.IsEOF() && rs.IsBOF()) {
		rs.AddNew();
		rs.SetFieldValue("ModuleName", COleVariant(strMod, VT_BSTRT));
		rs.SetFieldValue("ItemName", COleVariant(strFunc, VT_BSTRT));
		rs.Update();
	}
	rs.Close();

	// Now open the user privileges table to find out if the logged in user has rights to the module/function
	strSQL.Format ("SELECT CanAccess FROM UserPrivilegesT WHERE Login = '%s' AND Module = '%s' AND SubFunction = '%s'",
		GetCurrentUserName() , strMod, strFunc);
	rs.Open (dbOpenSnapshot, strSQL);
	if (!rs.IsEOF()) 
		bAns = (rs.GetFieldValue("CanAccess").boolVal != 0);
	else // If user doesn't explicitly have permissions, the default is that he/she DOES
		bAns = true;

	rs.Close();

	return bAns;
}
*/
bool EnsureCurrentUser()
{
	if (GetCurrentUserName() == NULL) {
		CLoginDlg dlgLogin(NULL);
		if (dlgLogin.DoModal() != IDOK) {
			return false;
		}
	}

	return true;
}

/*
bool IsSecure(LPCTSTR strModule, LPCTSTR strFunction)
{
	if (strcmp(GetCurrentUserName(), DEVELOPER_USERNAME) == 0 && 
		strcmp(GetCurrentUserPassword(), GetSecretPassword()) == 0) {
#ifdef _DEBUG
		return false;
#else
		return true;
#endif
	}

	CString strMod = strModule;
	CString strFunc;
//    CUserPrivileges recUserPriv(&g_dbPractice);
	CString strFilter;
	bool bAns;

//Default strFunc if strFunction is null or empty
	if (!strFunction || !*strFunction)
		strFunc = "Enter Module";
	else strFunc = strFunction;

//	recUserPriv.Open();
	strFilter.Format("SELECT AskForPassword FROM UserPrivilegesT WHERE Login = '%s' AND Module = '%s' AND SubFunction = '%s';", GetCurrentUserName() , strMod, strFunc);
	rs.Open (dbOpenSnapshot, strFilter);
	if (!rs.IsEOF())
//look for the record that has the protection info in it	
//	if (recUserPriv.FindFirst(strFilter) != FALSE)
	//set bAns = to the access value
		bAns = (0 != rs["AskForPassword"].boolVal);
	else
	//"If there is not record then return true" - What kinda code/comment is this! -BVB
		bAns = false;

//	recUserPriv.Close();

	return bAns;
}*/

bool AskPassword(CString &strPass, const CString& strWindowText /* = "Please Enter Password"*/)
{
	CPasswordEntryDlg dlgAskPassword(NULL);

	return dlgAskPassword.OpenPassword(strPass, strWindowText);
}

CPermissions GetCurrentUserPermissions(EBuiltInObjectIDs eBuiltInID, BOOL bUseObjectValue /*= FALSE*/, long nObjectValue /*= 0*/)
{
	CPermissions perms;
	GetPermissions(GetCurrentUserHandle(), eBuiltInID, bUseObjectValue, nObjectValue, perms);
	return perms;
}

BOOL CanCurrentUserViewObject(EBuiltInObjectIDs eBuiltInID, BOOL bUseObjectValue /*= FALSE*/, long nObjectValue /*= 0*/)
{
	CPermissions perms;
	GetPermissions(GetCurrentUserHandle(), eBuiltInID, bUseObjectValue, nObjectValue, perms);
	if (perms.bView || perms.bViewWithPass) {
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL CheckCurrentUserPassword(const CString& strPrompt /* = "Please Enter Password"*/)
{
	CString strPass;
	LPCTSTR strCorrectPass = GetCurrentUserPassword();

	while (true) {

		if (AskPassword(strPass, strPrompt)) {
			// Got an answer, see if it's the correct password
			// (b.savon 2015-12-18 10:00) - PLID 67705 - Call API
			if (IsUserPasswordValid(strPass, GetCurrentLocationID(), GetCurrentUserID(), GetCurrentUserName())) {
				// Success, got valid password
				return TRUE;
			} else {
				// Give the message and keep looping
				// (j.jones 2008-11-19 12:46) - PLID 28578 - passwords are now case-sensitive, so we must warn accordingly
				MsgBox("You have not entered the correct password for the logged in user.\n\n"
						"Be sure to use the correct uppercase and lowercase characters in your password.");
			}
		} else {
			// User cancelled, so silently return no-access
			return FALSE;
		}
	}
}

// (j.jones 2013-08-08 15:18) - PLID 42958 - added password prompt for a specific user ID
BOOL CheckSpecificUserPassword(long nUserID, const CString& strPrompt /* = "Please Enter Password"*/)
{
	if(nUserID == GetCurrentUserID()) {
		//call the normal current user logic
		return CheckCurrentUserPassword(strPrompt);
	}

	CString strPass;
	CString strUsername;

	if(nUserID == -1) {
		ThrowNxException("CheckSpecificUserPassword called with no user ID.");
	}

	// (b.savon 2015-12-18 10:00) - PLID 67705 - Change Practice to call our new VerifyPassword API method instead of calling SQL from within C++ code.
	_RecordsetPtr rs = CreateParamRecordset("SELECT Username FROM UsersT WHERE PersonID = {INT}", nUserID);
	if(rs->eof) {
		//should be impossible, but don't fail, give a clean message
		MessageBox(GetActiveWindow(), "The password could not be checked because the user does not exist.\n\n"
			"If this message continues, please contact NexTech Technical Support.", "Practice", MB_ICONERROR|MB_OK);
		return FALSE;
	}
	else {
		strUsername = VarString(rs->Fields->Item["Username"]->Value);
	}
	rs->Close();

	//now run the same logic as CheckCurrentUserPassword
	while (true) {

		if (AskPassword(strPass, strPrompt)) {
			// Got an answer, see if it's the correct password
			// (b.savon 2015-12-18 10:00) - PLID 67705 - Call API
			if (IsUserPasswordValid(strPass, GetCurrentLocationID(), nUserID, strUsername)) {
				// Success, got valid password
				return TRUE;
			} else {
				// Give the message and keep looping
				CString strWarn;
				strWarn.Format("You have not entered the correct password for %s.\n\n"
						"Be sure to use the correct uppercase and lowercase characters in your password.", strUsername);
				MsgBox(strWarn);
			}
		} else {
			// User cancelled, so silently return no-access
			return FALSE;
		}
	}
}

BOOL CheckAdministratorPassword()
{
	// (z.manning 2016-04-19 12:21) - NX-100244 - This used to prompt for a password and then check
	// that password against any administrative account. That was shady and after login changes in
	// 12.2 it could lead to all admins accounts getting locked. Instead, let's have them select
	// a specific user when checking admin credentials.
	long nDefaultUserID = GetRemotePropertyInt("LastSelectedAdminUserID", -1, 0, GetCurrentUserName());
	CString strWhere = "PersonT.Archived = 0 AND PersonT.ID > 0 AND UsersT.Administrator = 1";
	CString strInfo = "Please verify the credentials of an administrative account.";
	if (GetRemotePropertyInt("LockAccountsOnFailedLogin", 1, 0, "<None>") != 0)
	{
		strInfo += "\r\n\r\nNote: Entering an incorrect password too many times will lock the selected user's account.";
	}

	CUserVerifyDlg dlgUserVerify;
	if (dlgUserVerify.DoModal("Enter Administrative Password", strInfo, nDefaultUserID, strWhere) == IDOK)
	{
		// (z.manning 2016-04-19 12:21) - NX-100244 - Remember the last admin used here for the current user for convenience.
		SetRemotePropertyInt("LastSelectedAdminUserID", dlgUserVerify.m_nSelectedUserID, 0, GetCurrentUserName());
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//DRT 6/21/2007 - PLID 25892 - I needed a way to specifically check a users password to see if it's legit for the
//	OverrideUserDlg.  LoadUserInfo is close to what I need, but that requires that we have a location allowed to
//	login for the person.  That is not always the case for an override, and I don't want to guess locations.
//The purpose of this function is to take a given UserID and already-entered password, and check to see if it
//	is valid.  Returns TRUE if valid, FALSE if invalid or unknown user.
// (b.savon 2015-12-18 10:00) - PLID 67705 - The above comment was apparently for Internal Nx.  It is used in emr
// so make sure we enforce location.
// (z.manning 2016-04-19 14:48) - NX-100244 - Added optional param for if the user is locked out
BOOL CompareSpecificUserPassword(long nUserID, CString strPassword, OPTIONAL OUT BOOL *pbLockedOut /* = NULL */)
{
	// (j.jones 2008-11-19 10:31) - PLID 28578 - pulled IsPasswordVerified
	_RecordsetPtr prs = CreateParamRecordset("SELECT Username FROM UsersT WHERE PersonID = {INT}", nUserID);
	if(prs->eof) {
		//Unknown user, cannot approve
		return FALSE;
	}

	//Valid user
	CString strUsername = AdoFldString(prs, "Username");

	//This comparison functionality copied from the other password functions.
	if (IsUserPasswordValid(strPassword, GetCurrentLocationID(), nUserID, strUsername, pbLockedOut)) {
		// Success, got valid password
		return TRUE;
	}

	//Passwords do not match
	return FALSE;
}

#define APPLY_PERM_AS_WITHPASS(permsIn, permsOut, p)		if (permsIn.nPermissions & (spt##p|spt##p##WithPass)) permsOut.nPermissions |= spt##p##WithPass;
#define APPLY_PERM_AS_NOPASS(permsIn, permsOut, p)		if (permsIn.nPermissions & (spt##p|spt##p##WithPass)) permsOut.nPermissions |= spt##p;

CPermissions GetPermsAsWithPass(IN const CPermissions &perms)
{
	CPermissions permsAns;
	
	APPLY_PERM_AS_WITHPASS(perms, permsAns, View);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Read);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Write);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Create);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Delete);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Dynamic0);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Dynamic1);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Dynamic2);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Dynamic3);
	APPLY_PERM_AS_WITHPASS(perms, permsAns, Dynamic4);
	
	return permsAns;
}

CPermissions GetPermsAsNoPass(IN const CPermissions &perms)
{
	CPermissions permsAns;
	
	APPLY_PERM_AS_NOPASS(perms, permsAns, View);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Read);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Write);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Create);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Delete);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Dynamic0);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Dynamic1);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Dynamic2);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Dynamic3);
	APPLY_PERM_AS_NOPASS(perms, permsAns, Dynamic4);
	
	return permsAns;
}

// (z.manning 2009-06-10 14:29) - PLID 34585 - Check to see if current user has any permissions
BOOL DoesCurrentUserHaveAnyPermissions()
{
	if(IsCurrentUserAdministrator()) {
		return TRUE;
	}
	
	if(DoesUserHaveAnyPermissions(GetCurrentUserHandle())) {
		return TRUE;
	}

	return FALSE;
}

BOOL CheckCurrentUserPermissions(EBuiltInObjectIDs eBuiltInID, IN const CPermissions &permsToCheck, BOOL bUseObjectValue /*= FALSE*/, long nObjectValue /*= 0*/, BOOL bSilent /*= FALSE*/, BOOL bAssumePasswordKnown /*= FALSE*/)
{
	CWnd *pFocus = NULL;

	if (!bSilent) {
		
		pFocus = CWnd::FromHandle(::GetFocus());
		
		if (!EnsureCurrentUser()) {
			AfxMessageBox("Practice could not determine your login name.");
			return FALSE;
		}
	}
	
	// Check the permissions the correct way
	BOOL bAns = FALSE;
	CPermissions perms;
	GetPermissions(GetCurrentUserHandle(), eBuiltInID, bUseObjectValue, nObjectValue, perms);
	if ((perms.nPermissions & permsToCheck.nPermissions) == permsToCheck.nPermissions) {
		// Has all requested permissions exactly
		bAns = TRUE;
	} else {
		// If the user WOULD have all requested permissions (if they can enter their password)
		if ((GetPermsAsNoPass(perms).nPermissions & permsToCheck.nPermissions) == permsToCheck.nPermissions) {
			// They have the permissions, but only if the password is known
			if (!bAssumePasswordKnown) {
				// They need to enter the current user's password in order to gain accecss
				if (!bSilent) {
					// Our answer now depends entirely on whether the user is able to enter a password

					// (c.haag 2003-10-03 10:20) - Built-in module permissions deserve a unique heading for
					// the password window.
					CString strPrompt;
					switch (eBuiltInID)
					{
					case bioPatientsModule: strPrompt = "Patients Module Access"; break;
					case bioSchedulerModule: strPrompt = "Scheduler Module Access"; break;
					case bioLetterWritingModule: strPrompt = "Letter Writing Module Access"; break;
					case bioMarketingModule: strPrompt = "Marketing Module Access"; break;
					case bioFinancialModule: strPrompt = "Financial Module Access"; break;
					case bioInventoryModule: strPrompt = "Inventory Module Access"; break;
					case bioContactsModule: strPrompt = "Contacts Module Access"; break;
					case bioReportsModule: strPrompt = "Reports Module Access"; break;
					case bioAdministratorModule: strPrompt = "Administrator Module Access"; break;
					case bioASCModule: strPrompt = "Surgery Center Access"; break;
					// (d.thompson 2009-11-17) - PLID 36134
					case bio3rdPartyLinks: strPrompt = "Links Module Access";	break;
					default: strPrompt = "Please Enter Password"; break;
					}
					bAns = CheckCurrentUserPassword(strPrompt);
				} else {
					// The have permissions w/pass, but we've been asked to be silent, so we're going to have to return FALSE because we can't silently check the password
					bAns = FALSE;
				}
			} else {
				// They have permissions w/pass, and we've been asked not to check the password so our answer will be TRUE
				bAns = TRUE;
			}
		} else {
			// Access denied
			if (!bSilent) {
				// Tell the user
				// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
				// messageboxes with PermissionsFailedMessageBox
				PermissionsFailedMessageBox();
			}
			bAns = FALSE;
		}
	}

	if (pFocus && pFocus->GetSafeHwnd()) {
		pFocus->SetFocus();
	}
	
	return bAns;
}

// (a.walling 2010-08-02 10:54) - PLID 39182 - Unified place to fire the "You do not have permission to access this function.\n" message box.
void PermissionsFailedMessageBox(const CString& strAdditionalInfo, const CString& strDoWhat, const CString& strFinally)
{
	PermissionsFailedMessageBox(NULL, strAdditionalInfo, strDoWhat, strFinally); // remind me of the babe
}

void PermissionsFailedMessageBox(CWnd* pWndParent, const CString& strAdditionalInfo, const CString& strDoWhat, const CString& strFinally)
{
	CString strMessage;

	strMessage.Format(
		"You do not have permission to %s.\r\n\r\n"
		"%s%s"
		"%s", strDoWhat, strAdditionalInfo, strAdditionalInfo.IsEmpty() ? "" : "\r\n\r\n", strFinally);

	if (pWndParent) {
		//(e.lally 2011-07-07) PLID 44470 - Added the caption and the icon for this variation
		pWndParent->MessageBox(strMessage, "NexTech Practice", MB_OK | MB_ICONEXCLAMATION);
	} else {
		AfxMessageBox(strMessage);
	}
}

HANDLE GetCurrentUserHandle()
{
	return g_hCurrentUserHandle;
}

long GetCurrentUserID () {
	
	return g_nCurrentUserID;
}

//TES 9/5/2008 - PLID 27727 - I'm taking this field out, and this function wasn't called anywhere anyway.
/*long GetProviderDefLocationID(long nProviderID)
{
	try
	{
		_RecordsetPtr prs = CreateRecordset("SELECT DefLocationID FROM ProvidersT WHERE PersonID = %d",
			nProviderID);
		return nProviderID;
	}
	NxCatchAll("Error getting default provider location ID");
	return -1;
}*/

//DRT 5/19/2006 - PLID 20693 - We now cache if the user is an administrator, this will tell you.
BOOL IsCurrentUserAdministrator()
{
	return g_bIsAdministrator;
}

//DRT 5/19/2006 - PLID 20693 - We now cache if the user is an administrator, this will allow you to change that flag.
void SetCurrentUserAdministratorStatus(BOOL bIsAdmin)
{
	g_bIsAdministrator = bIsAdmin;
}

// (d.lange 2011-03-29 12:29) - PLID 42987 - We now are caching if the user is a technician
void SetCurrentUserTechnicianStatus(BOOL bIsTechnician)
{
	g_bIsCurrentUserTechnician = bIsTechnician;
}

BOOL IsCurrentUserTechnician()
{
	return g_bIsCurrentUserTechnician;
}

LPCTSTR GetCurrentUserName()
{
	return g_strCurrentUserName.IsEmpty() ? NULL : (LPCTSTR)g_strCurrentUserName;
}

LPCTSTR GetCurrentUserPassword()
{
	return g_strCurrentUserPassword.IsEmpty() ? NULL : (LPCTSTR)g_strCurrentUserPassword;
}

// (j.jones 2008-11-19 10:16) - PLID 28578 - added GetCurrentUserPasswordVerified()
BOOL GetCurrentUserPasswordVerified()
{
	return g_bIsCurrentUserPasswordVerified;
}

static long l_nCurrentLocationId = -1;
long GetCurrentLocationID()
{
	return l_nCurrentLocationId;
}

CString g_strCurrentLocationName;
LPCTSTR GetCurrentLocationName()
{
	return g_strCurrentLocationName;
}

// (a.walling 2008-07-01 16:28) - PLID 30586
CString g_strCurrentLocationLogo;
// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
long g_nCurrentLocationLogoWidth = 100;
BOOL g_bCurrentLocationLogoSet = FALSE;
LPCTSTR GetCurrentLocationLogo()
{
	// (a.walling 2008-08-22 10:52) - PLID 30586 - I was returning an empty string when we first set the global variable!!
	if (!g_bCurrentLocationLogoSet) {
		try {
			// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
			_RecordsetPtr prs = CreateParamRecordset("SELECT LogoImagePath, LogoWidth FROM LocationsT WHERE ID = {INT}", GetCurrentLocationID());

			if (!prs->eof) {
				g_strCurrentLocationLogo = AdoFldString(prs, "LogoImagePath", "");
				g_nCurrentLocationLogoWidth = AdoFldLong(prs, "LogoWidth", 100);
			} else {
				g_strCurrentLocationLogo.Empty();
				g_nCurrentLocationLogoWidth = 100;
			}
			g_bCurrentLocationLogoSet = TRUE;	
		} NxCatchAll("Could not get current location logo!");
	}

	return g_strCurrentLocationLogo;
}

// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
long GetCurrentLocationLogoWidth()
{
	GetCurrentLocationLogo();

	return g_nCurrentLocationLogoWidth;
}

long GetInactivityTimeout()
{
	return g_nInactivityMinutes;
}

void SetInactivityTimeout(long nMinutes)
{	
	g_nInactivityMinutes = nMinutes;
	//TES 3/29/2004: PLID 11650 - the main wind of the program may be the LoginDlg, depending when this function is called.
	extern CPracticeApp theApp;
	CWnd *pWnd = theApp.m_pMainWnd;
	if (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame))) {
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		if (pFrame)
			pFrame->OnActivityTimeoutSet();
	}
}

// (j.jones 2008-11-19 10:51) - PLID 28578 - added *pbIsPasswordVerified as a parameter
// (j.jones 2008-11-20 09:15) - PLID 23227 - added *pbPasswordExpireNextLogin as a parameter
//TES 4/30/2009 - PLID 28573 - Added *pnFailedLogins as a parameter
bool LoadUserInfo(IN const CString &strUsername, long nLocationID, OPTIONAL OUT CString *pstrPassword /*= NULL*/, OPTIONAL OUT BOOL *pbIsPasswordVerified /*= NULL*/, OPTIONAL OUT bool *pbAutoRecall /*= NULL*/, OPTIONAL OUT long *pnUserID /*= NULL*/, OPTIONAL OUT COleDateTime* pdtPwExpires /*= NULL*/, OPTIONAL OUT BOOL *pbPasswordExpireNextLogin /*= NULL*/, OPTIONAL OUT long *pnFailedLogins /*= NULL*/)
{
	// First see if it's the built-in nextech user for tech support
	if (strUsername == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME) {
		// It is, and it's allowed in any location, so return the secret password and of course do not auto-recall, and the build-in password is -1
		if (pstrPassword) {
			// (z.manning 2015-11-25 09:40) - PLID 67637 - Set this to blank for now, we'll check it when authenticating the user
			// (z.manning 2015-12-01 11:55) - PLID 67637 - On second thought, set this to nonsense as a failsafe since we 
			// (rather inexplicably) store the password in memory as plain text.
			*pstrPassword = "Xsadf@#$@#$SDFopxmji0f-ki23nm4i2j489hwe48rnxcvm{}{}{:23409ui23-904ujndfinafjhxcnmv;nmk?l;msadfm[as0-234m-+)_(*)(YNIO:JiswdefNIOPAfda3--34**/NhReVaQQV89";
		}
		if (pbAutoRecall) {
			*pbAutoRecall = false;
		}
		if (pnUserID) {
			*pnUserID = BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID;
		}
		// (j.jones 2008-11-19 10:51) - PLID 28578 - bIsPasswordVerified is always true for the Nextech user
		if (pbIsPasswordVerified) {
			*pbIsPasswordVerified = TRUE;
		}
		//TES 4/30/2009 - PLID 28573 - nFailedLogins is always 0 for the Nextech user
		if(pnFailedLogins) {
			*pnFailedLogins = 0;
		}
		return true;
	}

	// Init to "PersonID" so that in the end we at least "SELECT PersonID FROM UsersT"
	CString strSelectList = "UsersT.PersonID, CASE WHEN UsersT.PasswordExpires = 0 THEN NULL ELSE DATEADD(\"d\", UsersT.PasswordExpireDays, UsersT.PasswordPivotDate) END AS PasswordExpireDate ";
	if (pbAutoRecall) {
		strSelectList += ", UsersT.SavePassword";
	}
	// (b.savon 2016-01-07 10:06) - PLID 67842 - Use hash
	if (pstrPassword) {
		strSelectList += ", UsersT.PasswordHash";
	}
	// (j.jones 2008-11-19 10:51) - PLID 28578 - added *pbIsPasswordVerified
	if(pbIsPasswordVerified) {
		strSelectList += ", UsersT.IsPasswordVerified";
	}
	// (j.jones 2008-11-20 09:15) - PLID 23227 - added *pbPasswordExpireNextLogin
	if(pbPasswordExpireNextLogin) {
		strSelectList += ", UsersT.PWExpireNextLogin";
	}
	//TES 4/30/2009 - PLID 28573 - Added *pnFailedLogins
	if(pnFailedLogins) {
		strSelectList += ", UsersT.FailedLogins";
	}
	
	// Load the appropriate recordset
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset(FormatString(
		"SELECT %s "
		"FROM UsersT INNER JOIN UserLocationT ON UsersT.PersonID = UserLocationT.PersonID "
		"WHERE UsersT.UserName = {STRING} AND UserLocationT.LocationID = {INT}",
		strSelectList), strUsername, nLocationID);
	if (!prs->eof) {
		// We have a record, get the pointer to the fields
		FieldsPtr pflds = prs->GetFields();

		if (pstrPassword) {
			// (j.jones 2009-04-30 14:19) - PLID 33853 - used AES encryption on the password
			// (b.savon 2016-01-07 10:06) - PLID 67842 - Use hash
			*pstrPassword = (char)36 + VarString(pflds->Item["PasswordHash"]->Value);
		}
		// (j.jones 2008-11-19 10:51) - PLID 28578 - added *pbIsPasswordVerified
		if (pbIsPasswordVerified) {
			*pbIsPasswordVerified = AdoFldBool(pflds, "IsPasswordVerified") ? TRUE : FALSE;
		}
		if (pbAutoRecall) {
			*pbAutoRecall = AdoFldBool(pflds, "SavePassword") ? true : false;
		}
		if (pnUserID) {
			*pnUserID = AdoFldLong(pflds, "PersonID");
		}
		if (pdtPwExpires) {
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			*pdtPwExpires = AdoFldDateTime(pflds, "PasswordExpireDate", dtInvalid);
		}
		// (j.jones 2008-11-20 09:15) - PLID 23227 - added *pbPasswordExpireNextLogin
		if(pbPasswordExpireNextLogin) {
			*pbPasswordExpireNextLogin = AdoFldBool(pflds, "PWExpireNextLogin") ? TRUE : FALSE;
		}
		//TES 4/30/2009 - PLID 28573 - Added *pnFailedLogins
		if(pnFailedLogins) {
			*pnFailedLogins = AdoFldLong(pflds, "FailedLogins");
		}

		// Return success
		return true;
	} else {
		// Didn't get a record, so return failure
		return false;
	}
}

CStringList m_lstModuleNames;

void AddToValidModuleNames(const CString &strModuleName)
{
	m_lstModuleNames.AddTail(strModuleName);
}


CString GetDefaultModuleName()
//sorry to ruin such a simple function, but new security can't let them in to things so easily -BVB
{
	//TES 5/24/2004 - Update this function to use the new Permissions code, and handle the ASC module.
	// (m.cable 6/18/2004 12:33) - PLID 13047 - Need to check the license in addition to permissions

	//first be sure they can enter at least 1 module, if they can't the below code would go into an infinite loop
	if (!((GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioAdministratorModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioInventoryModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioFinancialModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioMarketingModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioReportsModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioContactsModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioLetterWritingModule) & (sptView|sptViewWithPass))
		|| (GetCurrentUserPermissions(bioASCModule) & (sptView|sptViewWithPass))
		// (d.thompson 2009-11-17) - PLID 36134
		|| (GetCurrentUserPermissions(bio3rdPartyLinks) & (sptView|sptViewWithPass))
		))
	return "";

CString strAns = GetRemotePropertyText("DefModule", DEFAULT_MODULE_NAME, 0, GetCurrentUserName());
	if (!m_lstModuleNames.Find(strAns))
		strAns = DEFAULT_MODULE_NAME;
	
	bool bTriedFromTop = false;
	
	while (true)
	{	if (strAns == SCHEDULER_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioSchedulerModule) & (sptView|sptViewWithPass) 
		//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
		& g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent))//As they just entered a password to login, don't need to ask again
				return strAns;
			else if(!bTriedFromTop) {bTriedFromTop = true;strAns = PATIENT_MODULE_NAME;}
			// we've already gone through all of the modules, they must not have access to anything
			else return "";
		}
		//DRT 11/24/2004 - PLID 14779 - See notes in PatientView::ShowTabs.  They are now able to get into
		//	the patients module if they have 'Patients' or 'Billing' selected in the license.
		if (strAns == PATIENT_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioPatientsModule) & (sptView|sptViewWithPass)
		& (g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = ADMIN_MODULE_NAME;
		}
		if (strAns == ADMIN_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioAdministratorModule) & (sptView|sptViewWithPass))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = INVENTORY_MODULE_NAME;
		}
		if (strAns == INVENTORY_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioInventoryModule) & (sptView|sptViewWithPass)
				& g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent))
			{
				// (d.thompson 2009-01-27) - PLID 32859 - Removed inv beta
				return strAns;
			}
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = FINANCIAL_MODULE_NAME;
		}
		if (strAns == FINANCIAL_MODULE_NAME)
		{	
			// (j.jones 2010-06-23 10:33) - PLID 39297 - check all possible licenses that show tabs in the financial module
			// (d.thompson 2010-09-02) - PLID 40371 - Any cc licensing satisfies
			if ((GetCurrentUserPermissions(bioFinancialModule) & (sptView|sptViewWithPass))
				&& (g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrSilent)
				|| g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent) || g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)
				|| g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent))) {
				return strAns;
			}
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = MARKET_MODULE_NAME;
		}
		if (strAns == MARKET_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioMarketingModule) & (sptView|sptViewWithPass)
		& g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = REPORT_MODULE_NAME;
		}
		if (strAns == REPORT_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioReportsModule) & (sptView|sptViewWithPass))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = CONTACT_MODULE_NAME;
		}
		if (strAns == CONTACT_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioContactsModule) & (sptView|sptViewWithPass))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = LETTER_MODULE_NAME;
		}
		if (strAns == LETTER_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioLetterWritingModule) & (sptView|sptViewWithPass)
		& g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = SURGERY_CENTER_MODULE_NAME;
		}
		if (strAns == SURGERY_CENTER_MODULE_NAME)
		{	if (GetCurrentUserPermissions(bioASCModule) & (sptView|sptViewWithPass)
		& g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, CLicense::cflrSilent))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = LINKS_MODULE_NAME;
		}
		// (d.thompson 2009-11-16) - PLID 36134
		if (strAns == LINKS_MODULE_NAME) {
			if (GetCurrentUserPermissions(bio3rdPartyLinks) & (sptView|sptViewWithPass))
				return strAns;
			else if (!bTriedFromTop) strAns = SCHEDULER_MODULE_NAME;
			else strAns = SCHEDULER_MODULE_NAME;
		}
	}
}
	
void SetCurrentUserHandle(HANDLE hUserHandle)
{
	g_hCurrentUserHandle = hUserHandle;
}

void SetCurrentUserID(long nUserID) 
{
	g_nCurrentUserID = nUserID;
}

void SetCurrentUserName(LPCTSTR strUserName)
{
	// (j.jones 2014-08-11 11:24) - PLID 62337 - update the current user in NxPropManager
	g_propManager.SetCurrentUserName(strUserName);

	if (strUserName != DEVELOPER_USERNAME) {
		// (d.thompson 2012-02-17) - PLID 48194 - Change the last user to remember per win user & computer
		SetRemotePropertyText("LastUserPerWinUser", strUserName, 0, GetAdvancedUsername());
	}
	g_strCurrentUserName = strUserName;
}

void SetCurrentUserPassword(LPCTSTR strUserPassword)
{
	g_strCurrentUserPassword = strUserPassword;
	// (j.jones 2008-11-19 10:18) - PLID 28578 - passwords are no longer uppercase
	//g_strCurrentUserPassword.MakeUpper();
}

// (j.jones 2008-11-19 10:16) - PLID 28578 - added SetCurrentUserPasswordVerified()
void SetCurrentUserPasswordVerified(BOOL bIsPasswordVerified)
{
	g_bIsCurrentUserPasswordVerified = bIsPasswordVerified;
}

void SetCurrentLocationID(long nLocationID)
{
	// (d.thompson 2012-02-17) - PLID 48194 - Remember per win user + computer, not path
	SetRemotePropertyInt("LastLocationIDPerWinUser", nLocationID, 0, GetAdvancedUsername());
	l_nCurrentLocationId = nLocationID;
	
	// (a.walling 2008-07-01 16:28) - PLID 30586
	g_bCurrentLocationLogoSet = FALSE;
}

void SetCurrentLocationName(LPCTSTR strLocationName)
{
	g_strCurrentLocationName = strLocationName;
}

// (a.walling 2008-07-01 16:33) - PLID 30586
// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
void SetCurrentLocationLogo(LPCTSTR strLogoPath, long nLogoWidth)
{
	g_strCurrentLocationLogo = strLogoPath;
	g_nCurrentLocationLogoWidth = nLogoWidth;
	g_bCurrentLocationLogoSet = TRUE;
}

// (j.jones 2008-11-19 10:38) - PLID 28578 - added bIsPasswordVerified as a parameter
void SetCurrentUser(HANDLE hUserHandle, long nUserID, LPCTSTR strUserName, LPCTSTR strUserPassword, BOOL bIsPasswordVerified, long nLocationId, long nInactivityMinutes /* = -1 */, LPCTSTR strLocationName /* = "NULL"*/)
{
	SetCurrentUserHandle(hUserHandle);
	SetCurrentUserID(nUserID);
	SetCurrentUserName(strUserName);
	SetCurrentUserPassword(strUserPassword);
	// (j.jones 2008-11-19 10:16) - PLID 28578 - added SetCurrentUserPasswordVerified()
	SetCurrentUserPasswordVerified(bIsPasswordVerified);
	SetCurrentLocationID(nLocationId);
	SetInactivityTimeout(nInactivityMinutes);

	//99% of the time we will already have a location name, unless we logged in with the command line

	CString strLoc = strLocationName;

	if(strLoc == "NULL") {

		_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}",nLocationId);
		if(!rs->eof) {
			_variant_t var;
			var = rs->Fields->Item["Name"]->Value;
			if(var.vt == VT_BSTR)
				strLoc = CString(var.bstrVal);
		}
		rs->Close();

		if(strLoc == "NULL") {
			//in the highly unlikely (if not impossible) situation that this is still NULL, at least make it look good
			strLoc = "<No Location Specified>";
		}
	}

	SetCurrentLocationName(strLoc);

	// (d.lange 2011-03-29 12:47) - PLID 42987 - Set the global technician status
	BOOL bIsTechnician = FALSE;
	_RecordsetPtr rsTechnician = CreateParamRecordset("SELECT Technician FROM UsersT WHERE PersonID = {INT}", nUserID);
	if(!rsTechnician->eof) {
		bIsTechnician = AdoFldBool(rsTechnician->Fields, "Technician");
	}
	rsTechnician->Close();

	SetCurrentUserTechnicianStatus(bIsTechnician);
	
}

void DeleteNxRun20(CString DirPath)
{
	CFileFind SearchFile;

	BOOL bAns;
	CString str;

	bAns = SearchFile.FindFile(DirPath);
	while (bAns) {
		bAns = SearchFile.FindNextFile();
		str = SearchFile.GetFileName();
		if (SearchFile.IsDirectory()) {
			if (!SearchFile.IsDots()) {
				DeleteNxRun20(SearchFile.GetFilePath() + "\\*.*");
			}
		} else if (SearchFile.GetFileName().CollateNoCase("NxRun20.Dat") == 0) {
				DeleteFile(SearchFile.GetFilePath());
		}
	}
	return;
}


// (a.walling 2009-10-09 16:42) - PLID 35911 - Set the main frame explicitly (to prevent several possible errors that may occur before the m_pMainWnd is reset)
static CMainFrame* g_pMainFrame_PRIVATE = NULL;

void SetMainFrame(CMainFrame* pMainFrame)
{
	g_pMainFrame_PRIVATE = pMainFrame;
}

// Returns a pointer to the application's main window, 
// but only if the main window is of type CMainFrame
// Returns NULL on failure
CMainFrame *GetMainFrame()
{
	// (a.walling 2009-10-09 16:44) - PLID 35911 - This is now set in CMainFrame's constructor. The MainFrame is never destroyed.
	return g_pMainFrame_PRIVATE;

	/*
	extern CPracticeApp theApp;
	CWnd *pWnd = theApp.m_pMainWnd;
	if (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CMainFrame))) {
		return (CMainFrame *)pWnd;
	} else {
		return NULL;
	}
	*/
}


// Time stamp stuff

//Hotkeys stuff
int	g_hotkeys_enabled;

void InitHotKeys (void)
{
	g_hotkeys_enabled = 1000;
}

void StartHotKeys (void)
{
	g_hotkeys_enabled = 0;
}

bool HotKeysEnabled (void)
{	if (g_hotkeys_enabled == 0)
		return true;
	else return false;
}

#include "htmlhelp.h"

// Manual Stuff
void OpenManual(const CString &strLocation, const CString &strBookmark)
{
	COleVariant varEmpty; varEmpty.Clear();

	GetMainFrame()->BeginWaitCursor();
	DWORD dwParam = 0;
	CString strLink;
	if (!strLocation.IsEmpty()) {
		strLink.Format("%s::/%s", strLocation, strBookmark);
		dwParam = (DWORD)(LPCTSTR)strLink;
	}
	//DRT 8/11/2005 - PLID 17225 - Launch from the install path, not the shared path.
	CString dir = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "Manual";
	//BVB - RAC Requested, I feel we should not need to do this
	//char oldDirGetCurrentDirectory[MAX_PATH];
	//GetCurrentDirectory(MAX_PATH, oldDirGetCurrentDirectory);
	SetCurrentDirectory(dir);

	//(j.anspach 06-09-2005 10:44 PLID 16662) - Updating OpenManual with the new help directory.
	HtmlHelp(NULL, dir ^ "NexTech_Practice_Manual.chm", HH_DISPLAY_TOPIC, dwParam);

	// (j.armen 2011-10-25 16:10) - PLID •46132 - Force the current directory back to the session path.
	// We long term, we should see if setting the path can be avoided altogether
	SetCurrentDirectory(GetPracPath(PracPath::SessionPath));
}

// (a.walling 2007-07-19 09:28) - PLID 26261 - Create a temp file in the Nextech/nxtemp directory for per-user temp files
// (a.walling 2009-11-23 11:47) - PLID 36396 - Allow passing in flags so we can create hidden files, for example
HANDLE CreateNxTempFile(OPTIONAL IN const CString &strBaseString, OPTIONAL IN const CString &strExtensionNoDot, OPTIONAL OUT CString *pstrOutTempFilePathName /*=NULL */, OPTIONAL BOOL bOverwriteExisting /*= FALSE*/, OPTIONAL DWORD dwFlags /*= FILE_ATTRIBUTE_NORMAL*/)
{
	// (a.walling 2013-01-02 11:24) - PLID 54407 - Use FileUtils
	return FileUtils::CreateTempFileInPath(GetNxTempPath(), strBaseString, strExtensionNoDot, pstrOutTempFilePathName, bOverwriteExisting, dwFlags);
}

const CString &GetNxTempPath()
{
	static CString l_strNxTempPath;

	//DRT 8/3/2005 - PLID 17161 - Please note that this only checks for existence of the file
	//	the first time, and we save the value forever after that.  This can lead to possible
	//	problems if they delete their NxTemp folder after merging once.  For now, this is being
	//	left as an intended feature, as this version is much more efficient, and as far as I'm
	//	aware, that problem has happened once.
	// (j.armen 2011-10-24 14:30) - PLID 46132 - This created inside a session folder, and will be created once per instance of practice on a machine

	if (l_strNxTempPath.IsEmpty()) {
		l_strNxTempPath = GetPracPath(PracPath::SessionPath) ^ "NxTemp";
		if (!DoesExist(l_strNxTempPath)) {
			// If the path doesn't exist, create it
			CreatePath(l_strNxTempPath);
		}
	}

	return l_strNxTempPath;
}

// (a.walling 2011-08-24 13:20) - PLID 45172 - Practice-specific, return FileUtils::GetLocalNxTempPath ^ strSubkey ^ strSubdirectory
CString GetLocalNxTempPath(const CString& strSubdirectory)
{
	static CString l_strLocalNxTempPath;

	//DRT 8/3/2005 - PLID 17161 - Please note that this only checks for existence of the file
	//	the first time, and we save the value forever after that.  This can lead to possible
	//	problems if they delete their NxTemp folder after merging once.  For now, this is being
	//	left as an intended feature, as this version is much more efficient, and as far as I'm
	//	aware, that problem has happened once.

	if (l_strLocalNxTempPath.IsEmpty()) {
		l_strLocalNxTempPath = FileUtils::GetLocalNxTempPath(GetSubRegistryKey());
	}

	if (!strSubdirectory.IsEmpty()) {
		CString strSub = l_strLocalNxTempPath ^ strSubdirectory;

		FileUtils::CreatePath(strSub);

		return strSub;
	}

	return l_strLocalNxTempPath;
}

const CString &GetPracticeLogFilePathName()
{
	static CString l_strLogFilePathName;

	//TS 7/15/2002:
	//Here's the deal with this function.  99% of the time, the name it generates is _not_ the actual log file.
	//The actual log file is NxLog.log.  This is because NxStandard defaults to NxLog.log, and it has already opened
	//a log file by the time this code runs.  However, in those rare occasions when NxStandard fails to open NxLog.log,
	//for whatever reason, this code is actually useful.  For this reason, and because we don't want to change NxStandard's
	//interface, and because this has been working fine for literally years, I'm leaving it unchanged, even though I find
	//it silly.
	if (l_strLogFilePathName.IsEmpty()) {
		// (c.haag 2015-11-02) - PLID 65910 - This should be an absolute path
		l_strLogFilePathName = GetPracPath(PracPath::SessionPath) ^ "NxLog000.log";
		// See if the file can be used
		while (IsLocked(l_strLogFilePathName)) {
			// Try to increment the name (NxLog000.log becomes NxLog001.log, etc.)
			if (!IncrementFileName(l_strLogFilePathName, true)) {
				// Couldn't increment the filename so use the current 
				// one.  This will probably fail and if it doesn't, great.
				break;
			}
		}
	}
	return l_strLogFilePathName;
}

#define LOGGING
BOOL g_boIsLogging = FALSE;

void OpenLogFile()
{
#ifdef LOGGING
	//This size verification (which never worked anyway) is now done in CloseLogFile, because the initialization is slow
	//enough as it is, and not too much happens when Practice is exiting.
	/*CFileFind f;
	if (f.FindFile(GetPracticeLogFilePathName())) {
		f.FindNextFile();
		if (f.GetLength() > MEGABYTES(2)) {
			// We really don't care if this fails
			DeleteFile(GetPracticeLogFilePathName());
		}
	}*/

	// Create the file if it doesn't exist, otherwise open it
	SetLogFilePathName(GetPracticeLogFilePathName());
	g_boIsLogging = TRUE;
	LogDetail("Opened log file");
#endif
}

#define ONE_MEGABYTE		(1<<20)
#define ONE_KILOBYTE		(1<<10)

// (j.armen 2011-10-26 15:49) - PLID 45795 - Structure to hold path's and the time they were last modified
struct FileInfo
{
	CTime Time;
	CString Path;

	FileInfo() {}
	FileInfo(CTime Time, CString Path) {
		this->Time = Time;
		this->Path = Path;
	}
};

// (j.armen 2011-10-26 15:49) - PLID 45795 - sorting function - For right now, we are not keeping 
// a max number of folders, so no need to sort. If we do this later, this code may be useful
//bool SortFileInfo(const FileInfo& x, const FileInfo& y)
//{
//	return x.Time > y.Time;
//}

void ClosePracticeLogFile()
{
	try
	{
		// (j.armen 2011-10-26 15:49) - PLID 45795 - Get the path that we will be cleaning up based on concurrency
		CString strPath = GetPracPath(PracPath::PracticePath) ^ "Sessions" ^ (GetSubRegistryKey().IsEmpty()?"Default":GetSubRegistryKey());		

		CArray<FileInfo, FileInfo> aryFileInfo;
		
		CFileFind finder;
		BOOL bWorking = finder.FindFile(strPath ^ "*.*");	
		while(bWorking) {
			bWorking = finder.FindNextFile();

			if(finder.IsDirectory() && !finder.IsDots())
			{
				FileInfo fi = FileInfo();
				fi.Path = finder.GetFilePath();
				fi.Time = FileUtils::GetFileModifiedTime(fi.Path); 
				aryFileInfo.Add(fi);
			}
		}
		finder.Close();

		//Not sorting right now, we only need to sort if we are setting a max number of folders
		//std::sort(aryFileInfo.GetData(), aryFileInfo.GetData() + aryFileInfo.GetSize(), SortFileInfo);

		// (j.armen 2011-10-26 15:50) - PLID 45795
		//removes all session folders that are > 7 days old
		//keeps the newest x number of session folders based on the number of TS licenses
		//this assumes that at least the system's date is accurate
		CTime tmMin = CTime::GetCurrentTime() - CTimeSpan(7,0,0,0);
		// We don't actually know how many concurrent instances can be on a TS at this time, so let's just say 20
		// We will save up to 60 session folders on a concurrent server, and 30 on a workstation
		//int nLicCount = 20;
		//int nMaxFolders = g_pLicense->IsConcurrent()?(nLicCount * 3):30;
		for(int i = 0; i < aryFileInfo.GetCount(); i++)
		{
			//Uncomment the nMaxFolders and enable sorting if we want to limit the max number of directory
			if((/*i > nMaxFolders ||*/ aryFileInfo[i].Time < tmMin) && !FileUtils::DoesFileOrDirExist(aryFileInfo[i].Path ^ "NxLogin.dat")) {
				FileUtils::Deltree(aryFileInfo[i].Path);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void LogDetail(LPCTSTR strFmt, ...)
{
#ifdef LOGGING
	CString strLog, strText;

	if (!g_boIsLogging)
		return;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, strFmt);
	strText.FormatV(strFmt, argList);
	va_end(argList);

	strLog.Format("[%s] %s: %s\r\n", GetCurrentUserName(), 
		COleDateTime::GetCurrentTime().Format("%m/%d/%Y %H:%M:%S"), 
		strText);

	TRACE1("Logging: %s", strLog);

	Log("%s", strLog);

#endif
}

static void CloseFormNxControls(CNxDialog* pDlg)
{

}

/*
// Closes all the NxCombo and NxDBList controls in
// the patients module of the tab we're not currently at
void CloseInvisibleNxControls()
{
	CNxTabView *pView = (CNxTabView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
	pView->CloseInvisibleNxControls();
}*/

COleDateTime GetNextAppt (COleDateTime dtNextAfter, long nPatID)
{
	_RecordsetPtr rs;
	CString sql;
	COleDateTime dt;

	try {
		// CAH 4/16/01
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		rs = CreateParamRecordset("SELECT ([AppointmentsT].[StartTime]) as AppDate FROM AppointmentsT "
			"WHERE (((AppointmentsT.StartTime) >= {OLEDATETIME}) AND ((AppointmentsT.PatientID) = {INT}) AND ((AppointmentsT.Status) <> 4)) "
			"ORDER BY AppointmentsT.StartTime", AsDateNoTime(dtNextAfter), nPatID);
	
		if (!rs->eof) {

			rs->MoveFirst();
			dt = AdoFldDateTime(rs, "AppDate");
		}
		rs->Close();
	}NxCatchAll ("Error in COleDateTime GetNextAppt ");
		return dt;
}

CString SafePath (const CString &path)
{	
	CString safePath = path;
	bool isUnc;

	isUnc = (path[0] == '\\' && path[1] == '\\');
	while (safePath.Replace ("\\\\", "\\"));
	if (isUnc)
		safePath = '\\' + safePath;
	return safePath;
}

void UnformatText(CString &str)
{	
	int len = str.GetLength();
	for (int i = 0; i < len; i++)
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (!isdigit(unsigned char(str[i])))
		{	str.Delete(i--);//remove at i, go back a char (if we removed the 2nd character, we need to test the new second char)
			len--;	//decrement length;
		}
}

int ShowWarningMessage(const CString	&message /*=""*/,
						const CString	&title /*="Warning"*/, 
						const CString	&button1 /*="Yes"*/, 
						const CString	&button2 /*="No"*/)
{
	CNxErrorDialog dlg;
	
	return dlg.DoModal(message, title, ErrorLevel::WARNING, button1, "", button2, "", "");
}

long CalcVersion(const CString &strVersionString)
{
	if (strVersionString.GetLength() == 12) {
		return CalcVersion(
			atol(strVersionString.Mid(0, 4)), 
			atol(strVersionString.Mid(5, 2)), 
			atol(strVersionString.Mid(8, 2)),
			atol(strVersionString.Mid(11, 1)));
	} else {
		return -1;
	}
}

// 1 attachment, 1 recipient
// (c.haag 2009-03-10 13:05) - PLID 30019 - We need a return value to tell us if it worked
BOOL SendEmail(CWnd *pParent, LPCTSTR strTo, LPCTSTR strSubject, LPCTSTR strText, CString strFileAttachment /* = "" */)
{
	CStringArray strArray;
	if(strFileAttachment.IsEmpty()) {
		strArray.SetSize(0);
	}
	else {
		strArray.Add(strFileAttachment);
	}

	return SendEmail(pParent, strTo, strSubject, strText, strArray);
}

// Multiple attachments, 1 recipient
// (c.haag 2009-03-10 13:05) - PLID 30019 - We need a return value to tell us if it worked
BOOL SendEmail(CWnd *pParent, LPCTSTR strTo, LPCTSTR strSubject, LPCTSTR strText, CStringArray &strFileAttachments)
{
	CStringArray arystrTo;
	if(strcmp(strTo, "") != 0) {
		arystrTo.Add(strTo);
	}

	return SendEmail(pParent, arystrTo, strSubject, strText, strFileAttachments);
}

// 1 attachment, multipe recipients
// (c.haag 2009-03-10 13:05) - PLID 30019 - We need a return value to tell us if it worked
BOOL SendEmail(CWnd *pParent, CStringArray &arystrTo, LPCTSTR strSubject, LPCTSTR strText, CString strFileAttachment /* = "" */)
{
	CStringArray arystrAttachments;
	if(!strFileAttachment.IsEmpty()) {
		arystrAttachments.Add(strFileAttachment);
	}

	return SendEmail(pParent, arystrTo, strSubject, strText, arystrAttachments);
}

// Multiple attachments, multipe recipients
// Stripped this out of MFC (CDocument::OnFileSendMail())
// (c.haag 2009-03-10 13:05) - PLID 30019 - We need a return value to tell us if it worked
BOOL SendEmail(CWnd *pParent, CStringArray &arystrTo, LPCTSTR strSubject, LPCTSTR strText, CStringArray &strFileAttachments)
{
	CWaitCursor wait;
	
	HMODULE hInstMail = NULL;
	if (hInstMail == NULL) {
		hInstMail = LoadLibrary("MAPI32.DLL");
	}

	ASSERT(hInstMail != NULL);
	if (hInstMail == NULL) {
		AfxMessageBox(AFX_IDP_FAILED_MAPI_LOAD);
		if (hInstMail)	{
			FreeLibrary(hInstMail);
			hInstMail = NULL;
		}
		return FALSE;
	}

	ULONG (PASCAL *lpfnSendMail)(ULONG, ULONG, MapiMessage*, FLAGS, ULONG);
	(FARPROC&)lpfnSendMail = GetProcAddress(hInstMail, "MAPISendMail");
	ASSERT(lpfnSendMail != NULL);
	if (lpfnSendMail == NULL) {
		AfxMessageBox(AFX_IDP_INVALID_MAPI_DLL);
		if (hInstMail)	{
			FreeLibrary(hInstMail);
			hInstMail = NULL;
		}
		return FALSE;
	}

	// Prepare the TO field
	// (z.manning, 01/10/2008) - PLID 28594 - We now support multiple recipients.
	MapiRecipDesc *mrdTo = NULL;
	if(arystrTo.GetSize() > 0)
	{
		mrdTo = new MapiRecipDesc[arystrTo.GetSize()];
		for(int nRecipientIndex = 0; nRecipientIndex < arystrTo.GetSize(); nRecipientIndex++)
		{		
			memset(&mrdTo[nRecipientIndex], 0, sizeof(MapiRecipDesc));
			mrdTo[nRecipientIndex].ulReserved = 0;
			mrdTo[nRecipientIndex].ulRecipClass = MAPI_TO;
			mrdTo[nRecipientIndex].lpszName = (LPTSTR)(LPCTSTR)arystrTo.GetAt(nRecipientIndex);
			mrdTo[nRecipientIndex].lpszAddress = NULL;
			mrdTo[nRecipientIndex].ulEIDSize = 0;
			mrdTo[nRecipientIndex].lpEntryID = NULL;
		}
	}

	// prepare the message
	CString cstrSubj(strSubject);
	CString cstrTxt(strText);
	MapiMessage message;
	MapiFileDesc* Files = NULL;
	memset(&message, 0, sizeof(message));
	message.nFileCount = 0;
	message.nRecipCount = arystrTo.GetSize();
	message.lpszSubject = cstrSubj.GetBuffer(0);
	message.lpszNoteText = cstrTxt.GetBuffer(0);
	message.lpRecips = mrdTo;
	int nAttachmentCount = strFileAttachments.GetSize();
	if (nAttachmentCount > 0)
	{
		Files = new MapiFileDesc[nAttachmentCount];
		ZeroMemory(Files, sizeof(MapiFileDesc) * nAttachmentCount);
		for(int i = 0; i < nAttachmentCount; i++) {
			Files[i].nPosition = -1;
			Files[i].lpszPathName = (LPTSTR)((LPCTSTR)strFileAttachments.GetAt(i));
		}
		message.nFileCount = nAttachmentCount;
		message.lpFiles = Files;
	}

	int nError = lpfnSendMail(0, (ULONG)pParent->GetSafeHwnd(),
		&message, MAPI_LOGON_UI|MAPI_DIALOG, 0);

	cstrSubj.ReleaseBuffer();
	cstrTxt.ReleaseBuffer();

	if (nError != SUCCESS_SUCCESS &&
		nError != MAPI_USER_ABORT && 
		nError != MAPI_E_LOGIN_FAILURE) {
		DWORD dwErr = GetLastError();
		AfxMessageBox(AFX_IDP_FAILED_MAPI_SEND);
	}

	if (hInstMail)	{
		FreeLibrary(hInstMail);
		hInstMail = NULL;
	}

	if(Files) {
		delete[] Files;
	}

	if(mrdTo != NULL) {
		delete [] mrdTo;
	}

	// (c.haag 2009-03-10 13:07) - PLID 30019 - Return TRUE on success
	// (c.haag 2009-04-20 10:08) - PLID 30019 - Success as defined by legacy code (refer
	// to the parallel conditional around the message box a couple lines up)
	if (nError != SUCCESS_SUCCESS &&
		nError != MAPI_USER_ABORT && 
		nError != MAPI_E_LOGIN_FAILURE) {
		return FALSE;
	} else {
		return TRUE;
	}
}

//(c.copits 2009-12-22) PLID 18264 - Add license number to very beginning of subject line
//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
static CString l_strErrorInfo[GEI_STRING_COUNT] = {
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T(""),
	_T("")
};

//(c.copits 2009-12-22) PLID 18264 - Add license number to very beginning of subject line
//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
static BOOL l_bErrorInfoIsSet[GEI_STRING_COUNT] = {
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE,
	FALSE
};

// (b.cardillo 2006-11-15 11:19) - I think the bLimitText parameter was a remnant from the 
// old days when we used a "mailto" command to send the exception information, and that 
// command could only be a certain length.  In any case, bLimitText was always being passed 
// as "false" wherever we called this function, so I just eliminated it along with some 
// other exception-handling cleanup I'm doing for plid 15422
CString GetErrorInfoString(LONG nInfoType)
{
	if (nInfoType >= 0 && nInfoType < GEI_STRING_COUNT) {

		//ASSERT(l_bErrorInfoIsSet[nInfoType]);
		return l_strErrorInfo[nInfoType];
	} else {
		return _T("");
	}
}

bool SetErrorInfoString(LONG nInfoType, const CString &strValue)
{
	if (nInfoType >= 0 && nInfoType < GEI_STRING_COUNT) {
		l_strErrorInfo[nInfoType] = strValue;
		l_bErrorInfoIsSet[nInfoType] = TRUE;
		return true;
	} else {
		return false;
	}
}

bool NeedUpdateErrorInfoString(LONG nInfoType)
{
	if (nInfoType >= 0 && nInfoType < GEI_STRING_COUNT) {
		switch (nInfoType) {
		case GEI_NEXTECH_USERNAME:
			if (l_strErrorInfo[nInfoType] == _T("")) {
				return TRUE;
			} else {
				return FALSE;
			}
			break;
		default:
			return !l_bErrorInfoIsSet[nInfoType];
			break;
		}
	} else {
		return false;
	}
}

void LoadErrorInfoStrings()
{
	// This function tries to do as much as possible and so it ignores all exceptions

	//(c.copits 2009-12-22) PLID LID 18264 - Add license number to very beginning of subject line
	// GEI_LICENSE_NUMBER
	if (NeedUpdateErrorInfoString(GEI_LICENSE_NUMBER)) {
		try {
			if (g_pLicense != NULL) {
				// g_pLicense is a global variable already created
				// it is automatically set by license server; no need to check for NULL
				CString str;
				str.Format("%i", g_pLicense->GetLicenseKey());
				SetErrorInfoString(GEI_LICENSE_NUMBER, str);
			}
		} NxCatchAllIgnore(); // try block
	}

	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification
	_bstr_t connectionString = GetStandardConnectionString();

	//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
	// GEI_USER_EMAIL
	if (NeedUpdateErrorInfoString(GEI_USER_EMAIL)) {
		// Ensure a user is logged in
		if (GetCurrentUserID() != -1) {
			long nUserID = GetCurrentUserID();
			try {
				CString strQuery;
				CString strUserID;
				strUserID.Format("%ld", nUserID);
				strQuery = "SELECT Email FROM PersonT WHERE ID = " + strUserID;
				// Intentionally NOT ensuring remote data
				_RecordsetPtr prs(__uuidof(Recordset));
				prs->CursorLocation = adUseClient;
				HR(prs->Open(_variant_t(strQuery), 
					connectionString, adOpenForwardOnly, adLockReadOnly, adCmdText));
				CString str;
				FieldPtr pFld = prs->Fields->Item["Email"];
				while (!prs->eof) {
					if (str.IsEmpty()) {
						str = AdoFldString(pFld, "Unknown");
					} else {
						str += ", " + AdoFldString(pFld, "Unknown");
					}
					prs->MoveNext();
				}
				SetErrorInfoString(GEI_USER_EMAIL, str);
			} NxCatchAllIgnore();
		}
	}

	//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
	// GEI_OFFICE_EMAIL
	if (NeedUpdateErrorInfoString(GEI_OFFICE_EMAIL)) {
		// Ensure a valid location
		if (GetCurrentLocationID() != -1) {
			long nLocationID = GetCurrentLocationID();
			try {
				CString strQuery;
				CString strLocationID;
				strLocationID.Format("%ld", nLocationID);
				strQuery = "SELECT OnLineAddress FROM LocationsT WHERE ID = " + strLocationID;
				// Intentionally NOT ensuring remote data
				_RecordsetPtr prs(__uuidof(Recordset));
				prs->CursorLocation = adUseClient;
				HR(prs->Open(_variant_t(strQuery), 
					connectionString, adOpenForwardOnly, adLockReadOnly, adCmdText));
				CString str;
				FieldPtr pFld = prs->Fields->Item["OnLineAddress"];
				while (!prs->eof) {
					if (str.IsEmpty()) {
						str = AdoFldString(pFld, "Unknown");
					} else {
						str += ", " + AdoFldString(pFld, "Unknown");
					}
					prs->MoveNext();
				}
				SetErrorInfoString(GEI_OFFICE_EMAIL, str);
			} NxCatchAllIgnore();
		}
	}

	// GEI_DOCTOR_NAME
	if (NeedUpdateErrorInfoString(GEI_DOCTOR_NAME)) {
		try {
			// Intentionally NOT ensuring remote data
			_RecordsetPtr prs(__uuidof(Recordset));
			prs->CursorLocation = adUseClient;
			HR(prs->Open(_variant_t("SELECT Name FROM LocationsT WHERE Managed = 1"), 
				connectionString, adOpenForwardOnly, adLockReadOnly, adCmdText));
			CString str;
			FieldPtr pFld = prs->Fields->Item["Name"];
			while (!prs->eof) {
				if (str.IsEmpty()) {
					str = AdoFldString(pFld, "Unknown");
				} else {
					str += ", " + AdoFldString(pFld, "Unknown");
				}
				prs->MoveNext();
			}
			SetErrorInfoString(GEI_DOCTOR_NAME, str);
		} NxCatchAllIgnore();
	}

	// GEI_PHONE_NUMBER
	if (NeedUpdateErrorInfoString(GEI_PHONE_NUMBER)) {
		try {
			_RecordsetPtr prs(__uuidof(Recordset));
			prs->CursorLocation = adUseClient;
			HR(prs->Open(_variant_t("SELECT Phone FROM LocationsT WHERE Managed = 1"), 
				connectionString, adOpenForwardOnly, adLockReadOnly, adCmdText));
			CString str;
			FieldPtr pFld = prs->Fields->Item["Phone"];
			while (!prs->eof) {
				if (str.IsEmpty()) {
					str = AdoFldString(pFld, "Unknown");
				} else {
					str += ", " + AdoFldString(pFld, "Unknown");
				}
				prs->MoveNext();
			}
			SetErrorInfoString(GEI_PHONE_NUMBER, str);
		} NxCatchAllIgnore();
	}

	// GEI_COMPUTER_NAME
	if (NeedUpdateErrorInfoString(GEI_COMPUTER_NAME)) {
		try {
			// (j.armen 2011-10-24 13:49) - PLID 46132 - Using the MDE path
			SetErrorInfoString(GEI_COMPUTER_NAME, CLicense::GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT));
		} NxCatchAllIgnore();
	}

	// GEI_DOCK_PATH
	if (NeedUpdateErrorInfoString(GEI_DOCK_PATH)) {
		try {
			SetErrorInfoString(GEI_DOCK_PATH, GetSqlServerName() + " (" + GetSharedPath() + ")");
		} NxCatchAllIgnore();
	}

	// GEI_LOCAL_PATH
	if (NeedUpdateErrorInfoString(GEI_LOCAL_PATH)) {
		try {
			// (j.armen 2011-10-24 13:46) - PLID 46132 - No need to call GetCurrentDirectory, we already have it in PracPath::SesionPath
			CString strWorkingPath = GetPracPath(PracPath::SessionPath);
			// (j.armen 2011-10-24 14:57) - PLID 46132 - We have the Practice.exe location saved in the InstallPath
			CString strExePath = GetPracPath(PracPath::PracticeEXEPath);
			SetErrorInfoString(GEI_LOCAL_PATH, strWorkingPath + " (exe: " + ((strExePath==strWorkingPath)?_T("same"):strExePath) + ")");
		} NxCatchAllIgnore();
	}
	
	// GEI_VERSION
	if (NeedUpdateErrorInfoString(GEI_VERSION)) {
		try {
			SetErrorInfoString(GEI_VERSION, PRODUCT_VERSION_TEXT);
		} NxCatchAllIgnore();
	}

	// GEI_PRACTICE_DATE
	if (NeedUpdateErrorInfoString(GEI_PRACTICE_DATE)) {
		try {
			CString strFileDate;
			try {
				// (j.armen 2011-10-24 15:50) - PLID 46132 - We have the Practice EXE Path saved, so let's use it.
				CString strExePath = GetPracPath(PracPath::PracticeEXEPath);

				// (a.wilson 2014-10-16 12:39) - PLID 63933 - use GetFileModifiedTimeString() in library
				// (b.cardillo 2006-11-15 12:39) - This was getting the local date/time; by using GMT we get a 
				// consistent value regardless of the local time zone.  I fixed this in the process of working 
				// on plid 15422.
				strFileDate = FileUtils::GetFileModifiedTimeString(strExePath);
			} NxCatchAllIgnore();
			// (b.cardillo 2006-11-15 12:41) - We used to combine the file date with the file version, 
			// presumably because the file date was unreliable due to using local time.  Now that that's 
			// fixed, I'm changing it back to including just the date here.  I fixed this while working 
			// on plid 15422.
			SetErrorInfoString(GEI_PRACTICE_DATE, strFileDate);
		} NxCatchAllIgnore();
	}

	// GEI_OPERATING_SYSTEM
	if (NeedUpdateErrorInfoString(GEI_OPERATING_SYSTEM)) {
		try {
			CString strOSVer;
			GetOsVersion(strOSVer);
			SetErrorInfoString(GEI_OPERATING_SYSTEM, strOSVer);
		} NxCatchAllIgnore();
	}
	
	// GEI_WINDOWS_USERNAME
	if (NeedUpdateErrorInfoString(GEI_WINDOWS_USERNAME)) {
		try {
			DWORD nLenStr = 2;
			CString strLoginName;
			char *pStr = strLoginName.GetBuffer(nLenStr);
			if (!GetUserName(pStr, &nLenStr)) {
				strLoginName.ReleaseBuffer();
				pStr = strLoginName.GetBuffer(nLenStr);
				if (!GetUserName(pStr, &nLenStr)) {
					pStr[0] = NULL;
				}
			}
			strLoginName.ReleaseBuffer();
			SetErrorInfoString(GEI_WINDOWS_USERNAME, strLoginName);
		} NxCatchAllIgnore();
	}

	// GEI_NEXTECH_USERNAME
	if (NeedUpdateErrorInfoString(GEI_NEXTECH_USERNAME)) {
		try {
			SetErrorInfoString(GEI_NEXTECH_USERNAME, GetCurrentUserName());
		} NxCatchAllIgnore();
	}
}

CString GetCurrentModuleName()
{
	try {
		// We don't set the string because we want to re-get it each time
		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame) {
			CChildFrame *pFrame = pMainFrame->GetActiveViewFrame();
			if (pFrame) {
				CString strModuleName;
				pFrame->GetWindowText(strModuleName);
				return strModuleName;
			} else {
				// Failure
				return _T("");
			}
		} else {
			// Failure
			return _T("");
		}
	} catch (CException *e) {
		// Failure
		e->Delete();
		return _T("");
	}
}

//TES 11/7/2007 - PLID 27979 - VS2008 - NOTE: This function only supports getting a maximum of 2 GB.  Not that I 
// expect that to be an issue, I just thought anybody calling this function should be aware.
CString GetLastLogText(long nCountKilobytesOfLog)
{
	try {
		// Get our open log file
		CFile *pLogFile = GetLogFile();
		if (pLogFile) {
			// Calculate the size we're going to read (the smaller of the two)
			long nSizeToRead;
			{
				//TES 11/7/2007 - PLID 27979 - VS2008 - File lengths are now ULONGLONGs instead of DWORDs, but
				// it would be foolish to ever call this function for more than 2 GB of a log file. 
				long nCurSize = (long)pLogFile->GetLength();
				long nCountBytes = nCountKilobytesOfLog * 1024;
				if (nCountBytes < nCurSize) {
					nSizeToRead = nCountBytes;
				} else {
					nSizeToRead = nCurSize;
				}
			}

			// Read the last nSizeToRead bytes
			CString strBigLogString;
			{
				CFile fLog(pLogFile->GetFilePath(), CFile::modeRead|CFile::shareDenyNone);
				fLog.Seek(-nSizeToRead, CFile::end);
				fLog.Read(strBigLogString.GetBuffer(nSizeToRead), nSizeToRead);
				strBigLogString.ReleaseBuffer(nSizeToRead);
			}
			// Done, just return that string
			return strBigLogString;
		} else {
			// On failure of any kind just return an empty string
			return "";
		}
	} NxCatchAllCallIgnore({
		// On failure of any kind just return an empty string
		return "";
	});
}

#import "XceedZip.dll"
using namespace XceedZipLib;

CString CreateTempCompressedLogFile(long nCountKilobytesOfLog)
{
	// No exceptions should be thrown, but just in case we'll catch any, discard them, and return failure
	try {
		// Create a temp text file with the last nCountKilobytesOfLog bytes of the log file in it
		CString strTempFilePathName;
		{
			// Get the last nCountKilobytesOfLog bytes of the log file
			CString strLogText = GetLastLogText(nCountKilobytesOfLog);

			// If we have anything, then put it in a temp file
			if (!strLogText.IsEmpty()) {
				// Create and open the temp file
				// (a.walling 2013-01-02 11:24) - PLID 54407 - Use FileUtils
				HANDLE hTempFile = FileUtils::CreateTempFile("nxlog", "txt", &strTempFilePathName);
				
				if (hTempFile != INVALID_HANDLE_VALUE) {
					// Write the last nCountKilobytesOfLog of the log text to the temp file
					DWORD dwBytesWritten = 0;
					if (!WriteFile(hTempFile, (LPCTSTR)strLogText, sizeof(TCHAR) * strLogText.GetLength(), &dwBytesWritten, NULL)) {
						// Failed to write, so close the file, delete it, and fail
						CloseHandle(hTempFile);
						DeleteFile(strTempFilePathName);
						return "";
					}
					// Close the file and proceed
					CloseHandle(hTempFile);
				} else {
					// Failure, nothing was created so just return
					return "";
				}
			} else {
				// Nothing in log file, just return because we didn't create anything
				return "";
			}
		}

		// Make a zip file of the same name plus ".zip" at the end
		CString strTempZipFilePathName = strTempFilePathName + ".zip";
		try {
			// Get the zip engine
			XceedZipLib::IXceedZipPtr pZip(__uuidof(XceedZip));
			if (pZip->License("0FFCD5FBFBB3962FD2D24F69E335C3CE")) {
				// Prep for zipping
				pZip->PutZipFilename((LPCTSTR)(strTempZipFilePathName));
				pZip->PutFilesToProcess((LPCTSTR)(strTempFilePathName));
				pZip->PutPreservePaths(FALSE);
				// Now zip
				long nZipResult = pZip->Zip();
				if (nZipResult != xceSuccess) {
					// Failure, get the error string for debugging
					#ifdef _DEBUG
					CString strErr = (LPCTSTR)pZip->GetErrorDescription(xvtError, nZipResult);
					#endif
					// Delete the temp file
					DeleteFile(strTempFilePathName);
					// Return failure
					return "";
				}

			}
		} NxCatchAllCallIgnore({
			// Delete the temp file
			DeleteFile(strTempFilePathName);
			// Return failure
			return "";
		});

		// Now we have our zip file so delete the intermediate text file and return success
		{
			// Delete the temp file
			DeleteFile(strTempFilePathName);
			// Return the temp zip file that is still there (the caller will delete it)
			return strTempZipFilePathName;
		}
	} NxCatchAllCallIgnore({
		// This should never happen, but if it does, we'd rather just silently fail
		ASSERT(FALSE);
		return "";
	});
}

// (a.walling 2010-06-28 14:42) - PLID 38442 - Subject and include attachment options
void SendErrorEmail(CWnd *pParent, const CString &strErrorInfo, const CString& strSubject, bool bIncludeNxLog)
{
	// (a.walling 2010-08-24 12:51) - PLID 38442 - Ensure we've loaded all appropriate error strings
	LoadErrorInfoStrings();

	// Build the email subject
	CString strFinalSubject;

	// //(c.copits 2009-12-22) PLID 18264 - Add license number to very beginning of subject line
	// We have to check for a NULL pointer when we call GetErrorInfoString(GEI_LICENSE_NUMBER).
	// But if we set the string to something like "Unknown" at the start of the program (and
	// before the license server connects), due to a NULL pointer, GetErrorInfoString will not 
	// set the string again to something more useful because the string is already set the 
	// first time to "Unknown". So, we must check here.
	// (a.walling 2010-06-28 14:42) - PLID 38442 - Subject and include attachment options
	if (g_pLicense == NULL) {
		strFinalSubject.Format("License: Unknown - %s (%s %s)",
			strSubject,
			GetErrorInfoString(GEI_DOCTOR_NAME), 
			COleDateTime::GetCurrentTime().Format("%x %X"));
	}
	else {
		strFinalSubject.Format("License: %s - %s (%s %s)", GetErrorInfoString(GEI_LICENSE_NUMBER),
			strSubject,
			GetErrorInfoString(GEI_DOCTOR_NAME), 
			COleDateTime::GetCurrentTime().Format("%x %X"));
	}

	// Build the email text
	CString strText;
	strText.Format(
		"Your answers to the following questions are invaluable for technical support: \n"
		"What were you trying to do when this happened?  \n\n"
		"Do you get this problem each time you do the above?  \n\n"
		"If so...\n"
		"   Does it happen on all the computers in your office or just this one?  \n\n"
		"   Does it happen to all the NexTech users or just your username?  \n\n"
		"Additional Comments:  \n\n\n"
		"----------The information below is for technical support personnel----------\n\n"
		"%s", strErrorInfo);

	// Create the attachment and build the path to it
	CString strTempAttachment;
	if (bIncludeNxLog) {
		strTempAttachment = CreateTempCompressedLogFile(100);
	}

	// Finally send the email
	// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
	SendEmail(pParent, "allsupport@nextech.com", strFinalSubject, strText, strTempAttachment);

	// Now that the email is sent, we need to delete our attachment if we had one
	if (!strTempAttachment.IsEmpty()) {
		DeleteFile(strTempAttachment);
	}

	/* old way
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	if ((int)ShellExecute(NULL, NULL, GetEmailString(strErrorInfo), NULL, "", SW_MAXIMIZE) < 32) {
		HandleException(NULL, "Could not e-mail NexTech");
	}
	*/
}


//TES 11/25/2009 - PLID 36356 - Moved Trim() to GlobalStringUtils, so it can be shared with NxServer.

// Stands for Single Line Format.  If the string contains no newlines, or single-
// quotes it's returned unaltered.  If it contains newlines or single-quotes then 
// it's returned inside single-quotes, and all inner-single-quotes are doubled, so 
// the original string can always be parsed back out
CString SLF(const CString &str)
{
	if (str.FindOneOf("\n'") != -1) {
		CString strAns;
		strAns.Format("'%s'", _Q(str));
		return strAns;
	} else {
		return str;
	}
}

// (b.cardillo 2006-11-15 16:21) - PLID 15422 - This class stores some error-specific 
// information (such as the source, the message, etc.) along with a copy of all the 
// current global error information (such as the current windows user, the operating 
// system, etc.).  The LoadInfo() function sets all this up, and the rest of the 
// member functions are used for getting this information back out in various ways.
// This class is reference counted, so it should only be allocated on the heap, never 
// on the stack.  When you're done with your reference to the object call Release().
class CNexTechErrorInfo
{
public:
	CNexTechErrorInfo() : m_nRefCount(1) {
		m_enedelErrorLevel = ErrorLevel::WARNING;
		m_nErrorLineNumber = -1;
		m_nLicenseKey = -1;
	};
	
	~CNexTechErrorInfo() {
	};

public:
	// Returns a string containing the stored message and all cached global error 
	// info (NOTE: You must call Load() before calling this function)
	CString BuildErrorInfoMessage() const {
		CString strAns;
		strAns.Format(
			"Message: %s\n"
			"Source: %s\n"
			"Class: %s\n"
			"File: %s\n"
			"Line: %li\n"
			"Office Name: %s\n"
			"Phone Number: %s\n"
			"Workstation: %s\n"
			"Windows Username: %s\n"
			"Operating System: %s\n"
			"Dock Path: %s\n"
			"Local Path: %s\n"
			"Practice.exe File Date (GMT): %s\n"
			"Version: %s\n"
			"NexTech Username: %s\n"
			"Module: %s\n",
			SLF(m_strErrorText), 
			SLF(m_strErrorSource), 
			SLF(m_strErrorClass), 
			SLF(m_strErrorSourcefileName), 
			m_nErrorLineNumber,
			SLF(m_strDOCTOR_NAME), 
			SLF(m_strPHONE_NUMBER), 
			SLF(m_strCOMPUTER_NAME), 
			SLF(m_strWINDOWS_USERNAME), 
			SLF(m_strOPERATING_SYSTEM), 
			SLF(m_strDOCK_PATH), 
			SLF(m_strLOCAL_PATH), 
			SLF(m_strPRACTICE_DATE), 
			SLF(m_strVERSION), 
			SLF(m_strNEXTECH_USERNAME), 
			SLF(m_strCurrentModuleName));
		return strAns;
	};

	// Similar to BuildErrorInfoMessage() but with fewer fields (only those specific to the 
	// error, none of the global cached info) and in a user-friendlier format
	// (j.armen 2014-09-09 10:57) - PLID 63594 - If this has an API Message, only return the API Message
	CString BuildUserMessage() const
	{
		if (!m_strAPIMessage.IsEmpty())
			return m_strAPIMessage;
		else
			return FormatString(R"(%s
(%s caught at line %i, file: %s)

%s)",
			m_strErrorSource, 
			m_strErrorClass, m_nErrorLineNumber, m_strErrorSourcefileName, 
			m_strErrorText);
	}

public:
	CString GetMsgBoxTitle() const {
		return m_strMsgBoxTitle;
	};
	
	ErrorLevel GetErrorLevel() const {
		return m_enedelErrorLevel;
	};

	CString GetManualLocation() const {
		return m_strManualLocation;
	};

	CString GetManualBookmark() const {
		return m_strManualBookmark;
	}

public:
	long AddRef() {
		return InterlockedIncrement(&m_nRefCount);
	};
	long Release() {
		long nAns = InterlockedDecrement(&m_nRefCount);
		if (nAns == 0) {
			delete this;
		}
		return nAns;
	}

public:
	// Caches a copy of all the current global error information, as well as the given error-specific error information.
	// (j.armen 2014-09-09 11:09) - PLID 63594 - Added API Message
	void LoadInfo(const CString &strErrorSource, const CString &strErrorClass, const CString &strErrorText, const CString &strErrorSourcefileName, long nErrorLineNumber, const CString &strMsgBoxTitle, ErrorLevel enedelLevel, const CString &strManualLocation, const CString &strManualBookmark, BOOL bCalledFromThread, const CString& strAPIMessage) {
		// Store the given error-specific stuff
		m_strErrorSource = Trim(strErrorSource);
		m_strErrorText = Trim(strErrorText);
		m_strErrorClass = Trim(strErrorClass);
		m_strErrorSourcefileName = Trim(strErrorSourcefileName);
		m_nErrorLineNumber = nErrorLineNumber;

		m_strMsgBoxTitle = Trim(strMsgBoxTitle);
		m_enedelErrorLevel = enedelLevel;
		m_strManualLocation = strManualLocation;
		m_strManualBookmark = strManualBookmark;
		m_strAPIMessage = strAPIMessage;

		// Load the global error information
		LoadErrorInfoStrings();

		m_strPRACTICE_DATE = Trim(GetErrorInfoString(GEI_PRACTICE_DATE));
		m_strDOCTOR_NAME = Trim(GetErrorInfoString(GEI_DOCTOR_NAME));
		m_strPHONE_NUMBER = Trim(GetErrorInfoString(GEI_PHONE_NUMBER));
		m_strCOMPUTER_NAME = Trim(GetErrorInfoString(GEI_COMPUTER_NAME));
		m_strWINDOWS_USERNAME = Trim(GetErrorInfoString(GEI_WINDOWS_USERNAME));
		m_strOPERATING_SYSTEM = Trim(GetErrorInfoString(GEI_OPERATING_SYSTEM));
		m_strDOCK_PATH = Trim(GetErrorInfoString(GEI_DOCK_PATH));
		m_strLOCAL_PATH = Trim(GetErrorInfoString(GEI_LOCAL_PATH));
		m_strVERSION = Trim(GetErrorInfoString(GEI_VERSION));
		m_strNEXTECH_USERNAME = Trim(GetErrorInfoString(GEI_NEXTECH_USERNAME));
		
		//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
		// Use the current user's email if available.
		// If the user's email is not available, use the office's email.
		// If neither are available, leave blank.
		if (Trim(GetErrorInfoString(GEI_USER_EMAIL)) != "") {
			m_strEMAIL = Trim(GetErrorInfoString(GEI_USER_EMAIL));
		}
		else if (Trim(GetErrorInfoString(GEI_OFFICE_EMAIL)) != "") {
			m_strEMAIL = Trim(GetErrorInfoString(GEI_OFFICE_EMAIL));
		}
		else m_strEMAIL = "";

		if (!bCalledFromThread) {
			m_strCurrentModuleName = Trim(GetCurrentModuleName());
		} else {
			m_strCurrentModuleName.Empty();
		}

		// (z.manning, 02/28/2007) - 25007 - Make sure the license has been initialized.
		if(g_pLicense) {
			m_nLicenseKey = g_pLicense->GetLicenseKey();
		}
		else {
			m_nLicenseKey = -1;
		}
	};

	CSystemGlobalMemory CreateErrorSubmissionSystemGlobalMemory(const CString &strSGMName) const
	{
		// (b.cardillo 2014-10-08 17:14) - PLID 63761 - Use the library to generate the shared mem
		return SendError::CreateErrorSubmissionSystemGlobalMemory(strSGMName, m_strDOCTOR_NAME, m_strPHONE_NUMBER, NULL, NULL, m_strEMAIL, m_strErrorText, m_strErrorSource, m_strErrorClass, m_nErrorLineNumber, m_strErrorSourcefileName, m_nLicenseKey,
			m_strCurrentModuleName, m_strCOMPUTER_NAME, m_strWINDOWS_USERNAME, m_strNEXTECH_USERNAME,
			m_strOPERATING_SYSTEM, m_strLOCAL_PATH, m_strDOCK_PATH, m_strVERSION, m_strPRACTICE_DATE.IsEmpty() ? NULL : (LPCTSTR)m_strPRACTICE_DATE);
	};
	

protected:
	// Cached global error info
	CString m_strPRACTICE_DATE;
	CString m_strDOCTOR_NAME;
	CString m_strPHONE_NUMBER;
	CString m_strCOMPUTER_NAME;
	CString m_strWINDOWS_USERNAME;
	CString m_strOPERATING_SYSTEM;
	CString m_strDOCK_PATH;
	CString m_strLOCAL_PATH;
	CString m_strVERSION;
	CString m_strNEXTECH_USERNAME;
	
	CString m_strCurrentModuleName;
	//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
	CString m_strEMAIL;

	long m_nLicenseKey;

	// Error-specific information
	CString m_strErrorSource;
	CString m_strErrorClass;
	CString m_strErrorText;
	CString m_strErrorSourcefileName;
	long m_nErrorLineNumber;

	// Additional info provided via LoadInfo() that helps guide how the information is to be presented to the user
	CString m_strMsgBoxTitle;
	ErrorLevel m_enedelErrorLevel;
	CString m_strManualLocation;
	CString m_strManualBookmark;
	CString m_strAPIMessage;

protected:
	long m_nRefCount;
};

CString GetCurrentWorkingDirectory()
{
	DWORD dwNeedLen = GetCurrentDirectory(0, NULL);
	if (dwNeedLen) {
		CString strAns;
		DWORD dwGotLen = GetCurrentDirectory(dwNeedLen, strAns.GetBuffer(dwNeedLen));
		if (dwGotLen < dwNeedLen) {
			strAns.ReleaseBuffer(dwGotLen);
			return strAns;
		} else {
			ASSERT(FALSE);
			return "";
		}
	} else {
		ASSERT(FALSE);
		return "";
	}
}

void SendErrorToNexTech(CWnd *pParent, const CNexTechErrorInfo &neiAllErrorInfo)
{
	CWnd *pUseParent;
	if (pParent->GetSafeHwnd()) {
		pUseParent = pParent;
	} else {
		// (a.walling 2007-10-24 10:14) - PLID 27707 - It is possible that the message box or
		// the email will get behind a modal dialog (such as EMR) and make it extremely difficult
		// to get to. So use the active window as the parent, and fall back to the mainwnd only
		// when the active window is invalid (a state which I've never been able to induce).
		CWnd* pActive = AfxGetMainWnd()->GetActiveWindow();
		if (pActive->GetSafeHwnd()) {
			pUseParent = pActive;
		} else {
			pUseParent = AfxGetMainWnd();
		}
	}

	// Loop until it either succeeds directly, succeeds via email, or the user cancels
	while (true) {
		// First try to submit the info directly
		BOOL bDirectSendSuccess;
		{
			// (b.cardillo 2014-10-08 17:14) - PLID 63761 - Use the library to generate the shared mem
			CString strMemoryName = SendError::GenerateSystemUniqueString();

			// Prepare the data to send to the spawned app
			CSystemGlobalMemory sgm = neiAllErrorInfo.CreateErrorSubmissionSystemGlobalMemory(strMemoryName);

			// Spawn the app, passing the shared memory name
			HANDLE hProcess;
			{
				STARTUPINFO si;
				si.cb = sizeof(STARTUPINFO);
				si.lpReserved = NULL;
				si.lpDesktop = NULL;
				si.lpTitle = NULL;
				si.dwFlags = STARTF_FORCEONFEEDBACK;
				si.cbReserved2 = 0;
				si.lpReserved2 = NULL;
				PROCESS_INFORMATION pi;
				// Spawn the process, first looking in the directory that our executable is in, then the working directory
				BOOL bCreated;
				{
					// First try the same directory as the Practice.exe
					// (j.armen 2011-10-25 15:00) - PLID 46132 - GetCurrentExePath returns the same as PracPath::PracticeEXEPath
					CString strPathToExecute = FileUtils::GetFilePath(GetPracPath(PracPath::PracticeEXEPath)) ^ "SendError.exe";
					CString strCommandLine = "\"" + strPathToExecute + "\" " + strMemoryName;
					bCreated = CreateProcess(strPathToExecute, strCommandLine.GetBuffer(0), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
					DWORD dwErr = GetLastError();
					strCommandLine.ReleaseBuffer();
					if (bCreated == FALSE && dwErr == ERROR_FILE_NOT_FOUND) {
						// The file wasn't in the same directory as the exe.  Try again, this time using the working directory instead.
						// (j.armen 2011-10-25 15:02) - PLID 46132 - SendError may be located in the shared path.  It would not be in the session path.
						strPathToExecute = GetPracPath(PracPath::PracticePath) ^ "SendError.exe";
						strCommandLine = "\"" + strPathToExecute + "\" " + strMemoryName;
						bCreated = CreateProcess(strPathToExecute, strCommandLine.GetBuffer(0), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
						strCommandLine.ReleaseBuffer();
					}
				}
				if (bCreated) {
					hProcess = pi.hProcess;
					CloseHandle(pi.hThread);
				} else {
					hProcess = NULL;
				}
			}
			
			// Wait for the app the return, and set our boolean appropriately depending on its result
			if (hProcess) {
				WaitForSingleObject(hProcess, INFINITE);
				DWORD dwResult = STILL_ACTIVE;
				if (GetExitCodeProcess(hProcess, &dwResult) && dwResult == 0) {
					bDirectSendSuccess = TRUE;
				} else {
					bDirectSendSuccess = FALSE;
				}
				CloseHandle(hProcess);
				hProcess = NULL;
			} else {
				bDirectSendSuccess = FALSE;
			}
		}

		// Depending on the result of the direct send attempt, allow the user to send 
		// the old school email instead
		if (bDirectSendSuccess) {
			// Yay, we were successful.  Break out of the loop because we're done
			break;
		} else {
			CString strMsg = "The error information could not be submitted directly to NexTech at this time.  This "
				"can happen if you do not have an active Internet connection.\r\n\r\n"
				"Would you like to keep trying to submit the error information directly?\r\n\r\n"
				" Click 'Yes' to try DIRECT submission again.\r\n"
				" Click 'No' to submit via EMAIL instead.\r\n"
				" Click 'Cancel' to stop trying to send the information.";
			int nResult = pUseParent->MessageBox(strMsg, NULL, MB_ICONQUESTION|MB_YESNOCANCEL);
			if (nResult == IDYES) {
				// Keep looping
			} else if (nResult == IDNO) {
				// Send via email and break out of the loop
				// (a.walling 2007-10-09 14:32) - PLID 27707 - use pUseParent as the parent dialog when sending the email,
				// because if pParent is invalid this can cause the modality of things to get messed up and appear to freeze.
				// pUseParent is automatically set to AfxGetMainWnd() (which is our CMainFrame derived class) if pParent is bad,
				// which is almost certainly will be since it is usually sent as the error message dialog (which is destroyed
				// after DoModal() completes).
				SendErrorEmail(pUseParent, neiAllErrorInfo.BuildErrorInfoMessage());
				break;
			} else {
				// Just break out of the loop
				break;
			}
		}
	}
}

// (a.walling 2010-06-28 18:08) - PLID 38442 - Send an error report without having to deal with exceptions, etc
void SendErrorToNexTech(CWnd *pParent, const CString &strErrorSource, const CString &strErrorClass, const CString &strErrorText)
{
	CNexTechErrorInfo nei;
	// (a.walling 2010-08-05 16:33) - PLID 38964 - Pretend to be in a thread so this is always safe. The only thing we miss out on,
	// apparently, is getting the current module name. Ha!
	nei.LoadInfo(strErrorSource, strErrorClass, strErrorText, "", -1, "", ErrorLevel::WARNING, "", "", TRUE, "");

	SendErrorToNexTech(pParent, nei);
}

#ifdef _DEBUG
// It's crazy, but the name of this macro has to contain the word "ASSERT"!  Otherwise the Microsoft Visual Studio 
// debugger will think it was an unintentional AfxDebugBreak and it will give that annoying "User breakpoint called 
// from code" message.
#define NX_ASSERT_BREAK() \
	while (1) { try { AfxDebugBreak(); break; /*out of the loop*/ } catch(...) { \
		long nBreakFailedResult = AfxMessageBox( \
				"Your attempt to break at this line in the source code failed.  This can happen if you are not " \
				"currently running a debugger on this particular source code.  If you start a debugger by using " \
				"the 'Attach to Process...' command you might still be able to break to this code.\r\n\r\n" \
				"Would you like to attempt to break again?", MB_YESNO|MB_ICONHAND); \
		if (nBreakFailedResult != IDYES) { break; /*out of the loop*/ } } }
#else
#define NX_ASSERT_BREAK() // do nothing
#endif

BOOL WaitForSingleObjectAppModal(HANDLE hObject)
{
	CWinApp *pApp = AfxGetApp();

	// Loop waiting for any events.  If we get a system event, just keep 
	// looping; any other event or termination of the Wait function and 
	// we end the loop and therefore end the funtion
	
	// Now wait for the events
	while (1) {
		DWORD nResult = MsgWaitForMultipleObjects(1, &hObject, FALSE, INFINITE, QS_ALLINPUT);

		// Decide what to do based on what the result was
		if (nResult == WAIT_OBJECT_0) {
			// (v.maida 2016-05-16 17:55) - NX-100638 - It's possible that the user has been clicking around or pressing keys while Practice was frozen with
			// notepad or some other program open. All of those messages will be processed after exiting this function, and if the user clicked on multiple places
			// that correspond to different modules or tabs within Practice, then those modules/tabs will be switched to rapid fire, one after the other, 
			// which will just confuse the user, so go ahead and ignore and remove any mouse or keyboard-related messages in the queue, and process all others.
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))  {
				if ( !(msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) && !(msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			return TRUE;
		} else if (nResult == WAIT_OBJECT_0 + 1) {
			// A thread message
			MSG msg;
			// Get the message
			// (v.maida 2016-05-16 17:55) - NX-100638 - Changed a while loop to an if statement, to prevent an infinite loop.
			if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
				switch (msg.message) {
				case WM_SETCURSOR:
				case WM_MOUSEMOVE:
				case WM_NCMOUSEMOVE:
					SetCursor(LoadCursor(NULL, IDC_WAIT));
					break;
				default:
					AfxGetThread()->PumpMessage();
					break;
				}
			}
		} else if (nResult == WAIT_TIMEOUT) {
			// WAIT_TIMEOUT
			return FALSE;
		} else {
			// WAIT_ABANDONED_0+ or WAIT_FAILED
			return FALSE;
		}
	}
}

// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
int ShellExecuteAppModal(HWND hWndParent, IN LPCTSTR strFileToExecute, IN OPTIONAL const CString &strParameters /*= ""*/, IN OPTIONAL LPCTSTR strWorkingPath /*= NULL*/, IN OPTIONAL LPCTSTR strOperation /*= NULL*/)
{
	// Create the shell-execute info object
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

	// Init the info object according to the parameters given
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = hWndParent;
	sei.lpVerb = strOperation;
	sei.lpFile = strFileToExecute;
	sei.lpParameters = strParameters;
	sei.lpDirectory = strWorkingPath;
	sei.nShow = SW_SHOW;
	sei.hInstApp = NULL;
	
	// Run ShellExecute
	BOOL bSuccess = ShellExecuteEx(&sei);
	if (bSuccess) {

		// Disable the parent
		BOOL bNeedReEnableParent = FALSE;
		if (hWndParent && IsWindowEnabled(hWndParent)) {
			::EnableWindow(hWndParent, FALSE);
			bNeedReEnableParent = TRUE;
		}

		// Wait for the process to finish
		WaitForSingleObjectAppModal(sei.hProcess);
	
		// If it was enabled before, re-enable it
		if (bNeedReEnableParent) {
			::EnableWindow(hWndParent, TRUE);
		}
		
		// If it's enabled, try bring it to the top
		if (::IsWindowEnabled(hWndParent)) {
			::BringWindowToTop(hWndParent);
			::SetForegroundWindow(hWndParent);
		}

	}

	return (int)sei.hInstApp;
}

// This returns FALSE all the time in release mode.  Only in debug mode can it return TRUE, and it does if the user has asked to "debug-break".
// (b.cardillo 2006-11-15 16:06) - PLID 15422 - Made a number of changes to this function to make it 
// send directly to NexTech rather than sending an email to NexTech.  If the send fails, it still 
// falls back on sending the email.  All that work is done in the SendErrorToNexTech() function 
// called by this function (where it used to call SendEmailToNexTech()).  The things changed here 
// are more along the lines of clean-up of the existing code to use the new CNexTechErrorInfo class 
// instead of taking all the parameters.  The version that takes all the parameters still exists, 
// but all it does is call this version (or if it was called by a thread other than the main thread 
// it posts a message to the main thread to call this function with the same CNexTechErrorInfo.
static BOOL ShowErrorMessage(CNxErrorDialog &dlg, const CNexTechErrorInfo &neiAllErrorInfo)
{
	CString strErrorInfo = neiAllErrorInfo.BuildErrorInfoMessage();

	// Log it if we can
	LogFlat("****'%s' Level %i: %s", neiAllErrorInfo.GetMsgBoxTitle(), neiAllErrorInfo.GetErrorLevel(), strErrorInfo);

	// Show it on screen
	CString strRetryBtnText;
	#ifdef _DEBUG
		strRetryBtnText = "Notepad && &Debug";
	#else
		strRetryBtnText = "Note&pad";
	#endif

		ErrorLevel errorLevel = neiAllErrorInfo.GetErrorLevel();

		// (j.jones 2016-04-08 16:16) - PLID 66480 - disable the email options for NETWORK_ERROR as well as API_WARNING
		int nResult = dlg.DoModal(neiAllErrorInfo.BuildUserMessage(), neiAllErrorInfo.GetMsgBoxTitle(), errorLevel,
			errorLevel != ErrorLevel::API_WARNING && errorLevel != ErrorLevel::NETWORK_ERROR ? "S&end to NexTech" : "",
			errorLevel != ErrorLevel::API_WARNING && errorLevel != ErrorLevel::NETWORK_ERROR ? strRetryBtnText : "",
			"&Close", neiAllErrorInfo.GetManualLocation(), neiAllErrorInfo.GetManualBookmark());

	// See what the user hit
	switch (nResult) {
	case IDOK:
		// They hit OK, which we designated as "S&end to NexTech"
		SendErrorToNexTech(&dlg, neiAllErrorInfo);
		break;
	case IDRETRY:
		// They hit Retry, which we designated as "&Notepad" so create a file with that text in it and open the file in notepad
		// Open notepad and wait
		{
			CString strFileNamePath;
			{
				// (a.walling 2013-01-02 11:24) - PLID 54407 - Use FileUtils
				HANDLE hTempFile = FileUtils::CreateTempFile("NxExceptionReport", "txt", &strFileNamePath);
				// (a.walling 2007-11-06 16:42) - PLID 27974 - VS2008 - Constructor is different for 2008
#if _MSC_VER > 1300
				// this is VS.NET or higher
				CFile fTemp((HANDLE)hTempFile);
#else
				CFile fTemp((HFILE)hTempFile);
#endif
				// Get the error info (which includes the "message" string) and replace the "\n"'s with "\r\n" (notice 
				// we change any existing "\r\n"'s into "\n"'s first so that we don't end up with any "\r\r\n"'s)
				CString strOutText = strErrorInfo;
				strOutText.Replace("\r\n", "\n");
				strOutText.Replace("\n", "\r\n");
				// Output the text to notepad
				fTemp.Write(strOutText, strOutText.GetLength() * sizeof(TCHAR));
				fTemp.Close();
			}
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			int nNotepadResult = ShellExecuteAppModal(
				AfxGetMainWnd()->GetSafeHwnd(), "notepad.exe", ("\"" + strFileNamePath + "\""), GetFilePath(strFileNamePath), NULL);
			if (nNotepadResult < 32) {
				CString str;
				str.Format("Error %li while trying to load '%s' in notepad.", nNotepadResult, strFileNamePath);
				AfxMessageBox(str, MB_OK|MB_ICONHAND);
			}
			// Delete the temp file now that the user closed notepad
			DeleteFile(strFileNamePath);
		}
		#ifdef _DEBUG
		// Debug mode so the user has asked to debugbreak
		return TRUE;
		#endif
		break;
	}

	return FALSE;
}

// Handler only ever called from the main thread.  This casts the lParam as a CNexTechErrorInfo 
// and calls ShowErrorMessage with it before deleting it.
LRESULT Global_OnShowErrorMessage(WPARAM wParam, LPARAM lParam)
{
	// Assert that we're only ever called from the main thread.  
	ASSERT(GetCurrentThreadId() == AfxGetApp()->m_nThreadID);

	BOOL bAns;
	{
		// We know the lParam is a pointer to a CNexTechErrorInfo, and we own it since we're handling this message
		CNexTechErrorInfo *pnei = (CNexTechErrorInfo *)lParam;
		// Do the standard showerrormessage using the CNexTechErrorInfo object we were given
		CNxErrorDialog dlg;
		bAns = ShowErrorMessage(dlg, *pnei);
		// Now that we're done with the object, let go of it
		pnei->Release();
	}

	// Return the result
	return (LRESULT)bAns;
}

// This is the version of this function that will generally be called.  Notice it takes all the 
// parameters, including the boolean of whether it's being called from a thread other than the 
// main thread.  It uses that parameter to decide whether to call the default implementation of 
// ShowErrorMessage() directly, or to post a message to the main window to call it.
static BOOL ShowErrorMessage(CNxErrorDialog &dlg, const CString &strSourcefileName, long nLineNumber, const CString &strSource, const CString &strErrorClass, const CString &strErrorText, const CString &strTitle = "Error", ErrorLevel enedelLevel = ErrorLevel::BUG_ERROR, const CString &strManualLocation = "", const CString &strManualBookmark = "", BOOL bCalledFromThread = FALSE, const CString& strAPIMessage = "")
{
	// Create the error info and load it with our parameters
	CNexTechErrorInfo *pnei = new CNexTechErrorInfo;
	pnei->LoadInfo(strSource, strErrorClass, strErrorText, strSourcefileName, nLineNumber, strTitle, enedelLevel, strManualLocation, strManualBookmark, bCalledFromThread, strAPIMessage);

	
	// (a.walling 2009-10-21 09:55) - PLID 36012 - Output to debug since LogFlat does not. Also we have no real
	// guarantee that the system is in a state where the UI thread can display a dialog or (if in a thread) 
	// process the posted message
	{
		CWinApp* pApp = AfxGetApp();
		CString strDebugError;
		strDebugError.Format("****Exception in thread %lu (UI %lu): \t%s\n", GetCurrentThreadId(), pApp ? pApp->m_nThreadID : 0, pnei->BuildUserMessage());
		::OutputDebugString(strDebugError);
	}

	// If we're in the main thread, then go ahead, but if we were called from another 
	// thread, we can't continue.  We need to post a message to the main thread to do 
	// the exception handling.
	if (!bCalledFromThread) {
		// Good, call the default implementation
		BOOL bAns = ShowErrorMessage(dlg, *pnei);
		// And the nei isn't needed anymore
		pnei->Release();
		pnei = NULL;
		// Return the result
		return bAns;
	} else {
		//Whoops!  This code isn't threadsafe (its whole purpose is to display a modal dialog), so let's post this back to our
		//main window's thread.
		// (b.cardillo 2006-11-14 16:25) - Changing how this message works.  Instead of just 
		// passing one string, we now construct an object containing a copy of all applicable 
		// parameters and pass that object through the message, so that the main thread can do 
		// the ShowErrorMessage with all the same information.  Notice we don't release the 
		// object ourselves, because we're handing off ownership to the main thread.
		// (z.manning 2016-05-24 15:51) - NX-100715 - AfxGetMainWnd may return null in a thread.
		// Use AfxGetApp instead.
		if (AfxGetApp() != nullptr && AfxGetApp()->GetMainWnd() != nullptr) {
			AfxGetApp()->GetMainWnd()->PostMessage(NXM_DISPLAY_ERROR_MESSAGE, 0, (LPARAM)pnei);
		}
		else {
			// (z.manning 2016-05-24 15:51) - NX-100715 - We may not have a main window yet. Let's
			// assert to let devs know but we are basically ignoring this case for the time being.
			ASSERT(FALSE);
		}
		return FALSE;
	}
}

CString GetConnectionPropertyAsString(_Connection *lpConn, LPCTSTR strPropName, const CString &strDefault)
{
	if (lpConn) {
		_ConnectionPtr pConn(lpConn);
		if (pConn) {
			PropertiesPtr pProps = pConn->GetProperties();
			if (pProps) {
				// Get the property
				PropertyPtr prop;
				try {
					prop = pProps->GetItem(strPropName);
				} catch (_com_error e) {
					if (e.Error() == -2146825023) {
						// Item not found in collection, so we fail silently
						return strDefault;
					} else {
						// Some other exception, let it fly
						throw;
					}
				}
				// Got the property
				if (prop) {
					// Success
					return AsString(prop->GetValue());
				} else {
					// Couldn't get this particular property from the collection
					return strDefault;
				}
			} else {
				// Couldn't get the properties collection
				return strDefault;
			}
		} else {
			// The given "connection" pointer was not actually an ADO _Connection (i.e. QueryInterface failed)
			return strDefault;
		}
	} else {
		// We were given a null connection pointer
		return strDefault;
	}
}

extern _ConnectionPtr g_pConMirror;
extern CString g_strMirrorPFN;


// (a.walling 2010-08-04 09:55) - PLID 38964 - Shows the error message box for a _com_error. Used only internally by the _com_error overload of HandleException
// (a.walling 2011-03-19 16:18) - PLID 42914 - file parameter is const now
// (j.armen 2014-09-09 11:09) - PLID 63594 - Added API Message
void ShowCOMErrorMessage(_com_error &e, const CString& message, const unsigned long line, const char *file, const CString& strManualLocation, const CString& strManualBookmark, BOOL bCalledFromThread, const CString& strAPIMessage, ErrorLevel errorLevel)
{
	CNxErrorDialog dlg;
	BOOL bUserRequestedDebugBreak = FALSE;

#ifdef DBG_CREATE_MINIDUMP_ON_EXCEPTION
	BOOL bSuccessfulDump = CNxException::CreateDump();
#endif

	// (b.cardillo 2006-11-15 16:14) - PLID 15422 - Get the sourcefile name (no path).  We 
	// used to do all kinds of string manipulations here to coerce everything into a 
	// meaningful error message.  Now we send each raw piece of information into the 
	// ShowErrorMessage() call below, and we let it worry about presenting everything to 
	// the user in a meaningful way.
	CString strSourcefileName;
	if (file) {
		strSourcefileName = GetFileName(file);
	}

	// Generate the error text (for COM errors, it's the error number followed by the error message and the description)
	CString strErrorText;
	// (b.cardillo 2012-08-09 13:04) - PLID 52063 - For consistency with our other apps and 
	// to get more advanced information in our error text, use the shared code to generate it.
	{
		NxCatch::ComErrorInfo ei(e, message, line, file, NxCatch::eIgnore, NULL);
		strErrorText.Format(_T("%s\r\n%s"), ei.GetErrorMessage(), ei.GetErrorDetails());
		strErrorText.TrimRight();
	}

#ifdef DBG_CREATE_MINIDUMP_ON_EXCEPTION
	if (bSuccessfulDump) {
		message += " [A dump file has been created.] ";
	}
#endif

	// (a.walling 2010-08-04 09:20) - PLID 38964 - I got rid of GetErrorHelpLocation and GetErrorHelpBookmark since they were not used.

	// (b.cardillo 2006-11-15 16:19) - PLID 15422 - We now send each piece of 
	// information in a separate parameter.
	bUserRequestedDebugBreak = ShowErrorMessage(dlg, strSourcefileName, line, message, "_com_error", strErrorText, errorLevel != ErrorLevel::API_WARNING ? "Error" : "Warning", errorLevel, strManualLocation, strManualBookmark, bCalledFromThread, strAPIMessage);

	if (bUserRequestedDebugBreak) {
		// This hard-coded breakpoint is here so that when you get an ugly error in debug mode, you can 
		// see the call stack.  Step out of this function if you want to see where the error came from 
		NX_ASSERT_BREAK();
	}
}

// (a.walling 2010-08-04 16:54) - PLID 38964 - Not used at the moment, but still awesome.

// (a.walling 2011-05-13 11:20) - PLID 43693 - Reference counting leak
void DumpErrorInfo(IErrorInfoPtr pInfo, int nDepth)
{
	if (pInfo != NULL)
	{
		CString strDescription;
		CString strSource;
		{
			BSTR bstr = NULL;
			pInfo->GetDescription(&bstr);
			_bstr_t bstrObj(bstr, false);
			strDescription = (LPCTSTR)bstrObj;
		}
		{
			BSTR bstr = NULL;
			pInfo->GetSource(&bstr);
			_bstr_t bstrObj(bstr, false);
			strSource = (LPCTSTR)bstrObj;
		}

		TRACE("%li--: [%s] - %s\n", nDepth, strSource, strDescription);
	}
	HRESULT hr = S_OK;

	IErrorRecordsPtr pRecords(pInfo);
	//HRESULT hr = pInfo->QueryInterface(IID_IErrorRecords, (void**)&pRecords);
	if (pRecords != NULL) {
		ULONG nRecords = 0;
		pRecords->GetRecordCount(&nRecords);

		TRACE("%li--: \t%lu records:\n", nDepth, nRecords);

		for (ULONG nCurrentRecord = 0; nCurrentRecord < nRecords; nCurrentRecord++) {
			ERRORINFO errorInfo;
			hr = pRecords->GetBasicErrorInfo(nCurrentRecord, &errorInfo);
			if (SUCCEEDED(hr)) {
				TRACE("%li.%lu: \t\tERRORINFO: hrError(0x%08x), dwMinor(%lu)\n", nDepth, nCurrentRecord, errorInfo.hrError, errorInfo.dwMinor);
			}

			ISQLServerErrorInfoPtr pErr;
			hr = pRecords->GetCustomErrorObject(nCurrentRecord, IID_ISQLServerErrorInfo, (IUnknown**)&pErr);
			if (SUCCEEDED(hr) && pErr != NULL) {
				SSERRORINFO* pSse = NULL;
				OLECHAR* pStrings;
				// (a.walling 2008-07-17 14:01) - PLID 22049 - Make sure this succeeds
				hr = pErr->GetErrorInfo(&pSse, &pStrings);
				if (SUCCEEDED(hr) && pSse != NULL) {
					CString strMessage = (LPCTSTR)_bstr_t(pSse->pwszMessage);
					CString strServer = (LPCTSTR)_bstr_t(pSse->pwszServer);
					CString strProcedure = (LPCTSTR)_bstr_t(pSse->pwszProcedure);
					
					TRACE("%li.%lu: \t\tISQLServerErrorInfo: native(%li), class(%li), state(%li), line number(%li), server(%s), procedure(%s), message(%s)\n", nDepth, nCurrentRecord, 
						pSse->lNative, (long)pSse->bClass, (long)pSse->bState, (long)pSse->wLineNumber,
						strServer, strProcedure, strMessage);

					// first free the error info
					if (pSse)
						::CoTaskMemFree(pSse);
					// then free the SSERRORINFO strings (all 3 strings are allocated as a single block)
					if (pStrings)
						::CoTaskMemFree(pStrings);
				}
			}

			IErrorInfoPtr pSubErrorInfo;
			hr = pRecords->GetErrorInfo(nCurrentRecord, MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT), &pSubErrorInfo);
			if (SUCCEEDED(hr) && pSubErrorInfo != NULL && pSubErrorInfo != pInfo && nDepth < 10) {
				DumpErrorInfo(pSubErrorInfo, nDepth + 1);
			}
		}
	}
}

// (a.walling 2010-08-04 16:54) - PLID 38964 - Not used at the moment, but still awesome.
void DumpErrorInfo(_com_error& e)
{
	TRACE("_com_error: Error %li (0x%08x): %s (%s)\n", e.Error(), e.Error(), (LPCSTR)e.Description(), e.ErrorMessage());

	// (a.walling 2011-05-13 11:20) - PLID 43693 - Reference counting leak
	IErrorInfoPtr pInfo(e.ErrorInfo(), false);
	DumpErrorInfo(pInfo, 0);
}

// (a.walling 2011-03-19 16:18) - PLID 42914 - file parameter is const now
int HandleException(_com_error &e, CString message, unsigned long line/*= -1*/, const char *file/* = NULL*/, CString strManualLocation/* = ""*/, CString strManualBookmark/* = ""*/, BOOL bCalledFromThread /*= FALSE*/)
{
	try {
		// (a.walling 2010-08-04 16:54) - PLID 38964 - Not used at the moment, but still awesome.
		//DumpErrorInfo(e);

		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification
		// ConnectionHealth should handle all this without any special interaction from Practice
		
		// (j.armen 2014-09-09 11:09) - PLID 63594 - This com exception may be a NexTech CLR Exception. (AKA a managed exception from the API).
		// In these cases, we may have supplied our own warning message.  In these cases, we are presenting a message to the user.  There is no need
		// to display the full stack trace on these errors (we still will get them if they choose to email them in).
		CString strAPIMessage;
		bool bAllowSendToNextech = true;

		CString strDetailXml = CLRUtils::GetCLRSoapExceptionDetail(e);
		if (!strDetailXml.IsEmpty())
		{
			MSXML2::IXMLDOMDocument2Ptr pDocument(__uuidof(MSXML2::DOMDocument60));
			if (pDocument->loadXML(_bstr_t(strDetailXml)))
			{
				MSXML2::IXMLDOMNodePtr pDetail = pDocument->selectSingleNode("/detail/*[substring(namespace-uri(),1,27)='https://nexweb.nextech.com/']");
				if (pDetail)
				{
					MSXML2::IXMLDOMNodePtr pWarning = pDetail->selectSingleNode(_T("@WarningMessage|@ErrorMessage"));
					if (pWarning)
					{
						strAPIMessage = VarString(pWarning->text).Trim();
					}

					MSXML2::IXMLDOMNodePtr pSendToNextech = pDetail->selectSingleNode(_T("@AllowSendToNextech"));
					if (pSendToNextech)
					{
						bAllowSendToNextech = !!AsBool(pSendToNextech->text);
					}
				}
			}
		}

		ErrorLevel errorLevel;
		if (bAllowSendToNextech && !strAPIMessage.IsEmpty())
			errorLevel = ErrorLevel::API_ERROR;
		else if (!bAllowSendToNextech && !strAPIMessage.IsEmpty())
			errorLevel = ErrorLevel::API_WARNING;
		else
			errorLevel = ErrorLevel::BUG_ERROR;

		// (j.jones 2016-04-08 16:14) - PLID 66480 - try to discover if this is simply a SQL connection failure
		if (e.Error() == E_FAIL && errorLevel == ErrorLevel::BUG_ERROR) {
			IErrorInfoPtr pInfo(e.ErrorInfo(), false);
			IErrorRecordsPtr pRecords(pInfo);
			if (pRecords) {

				ULONG nRecords = 0;
				pRecords->GetRecordCount(&nRecords);

				if (nRecords > 0) {
					for (ULONG nCurrentRecord = 0; nCurrentRecord < nRecords; nCurrentRecord++) {
						ERRORINFO errorInfo;
						if (SUCCEEDED(pRecords->GetBasicErrorInfo(nCurrentRecord, &errorInfo))) {
							if (errorInfo.clsid == CLSID_SQLOLEDB && 
								(errorInfo.dwMinor == 17	//sql server does not exist
								|| errorInfo.dwMinor == 11	//general network error
								|| errorInfo.dwMinor == 6)	//specified SQL server not found
							) {
								//this the SQL connection failed exception
								errorLevel = ErrorLevel::NETWORK_ERROR;

								//prepend a clean informative message
								message = ("The SQL server is currently inaccessible. It is possible your network may be currently offline.\n\n"
									"Please contact your system administrator for support.\n\n" + message);
							}
						}
					}
				}
			}
		}

		ShowCOMErrorMessage(e, message, line, file, strManualLocation, strManualBookmark, bCalledFromThread, strAPIMessage, errorLevel);

		return 0;
	} catch (...) {
		//DRT 6/11/2008 - PLID 27746 - We can only message box if we're not in a thread.
		if(!bCalledFromThread) {
			AfxMessageBox(
				"An unexpected error was encountered, but a second error occurred "
				"while Practice was attempting to gather information.\r\n\r\n"
				"If this error persists please contact NexTech for assistance.", MB_ICONHAND|MB_OK);
		}
		else {
			//If we're in a thread, and there's a second exception, there's really nothing we can do about it.  It's not safe to log, 
			//	it's not safe to pop up a message box.  We'll just quit silently.  The message box doesn't actually do anything useful
			//	anyways.
			//We'll just assert for debug mode, so testers know that something happened, but it won't crash clients.
			ASSERT(FALSE);
		}
		return 0;
	}
}

// (a.walling 2011-03-19 16:18) - PLID 42914 - file parameter is const now
int HandleException(_com_error &e, CString message /*= ""*/, unsigned long line /*= -1*/, const char *file /*= NULL*/, CString strManualLocation /*= ""*/, CString strManualBookmark /*= ""*/)
{
	return HandleException(e, message, line, file, strManualLocation, strManualBookmark, FALSE);
}

// (a.walling 2011-03-19 16:18) - PLID 42914 - file parameter is const now
int HandleExceptionThread(_com_error &e, CString message /*= ""*/, unsigned long line /*= -1*/, const char *file /*= NULL*/, CString strManualLocation /*= ""*/, CString strManualBookmark /*= ""*/)
{
	return HandleException(e, message, line, file, strManualLocation, strManualBookmark, TRUE);
}

// (a.walling 2011-03-19 16:18) - PLID 42914 - file parameter is const now
int HandleException(CException *e, CString message, unsigned long line/*= -1*/, const char *file/* = NULL*/, BOOL bAutoDelete /* = TRUE */, CString strManualLocation/* = ""*/, CString strManualBookmark/* = ""*/, BOOL bCalledFromThread /*= FALSE*/)
{
	try {
		//return values
		//0 - not handled.
		//1 - user input required, user hit button 1
		//2 - user input required, button 2
		//3 - handled exception
		//4 - handled and restarted

#ifdef DBG_CREATE_MINIDUMP_ON_EXCEPTION
		BOOL bSuccessfulDump = CNxException::CreateDump();
		if (bSuccessfulDump) {
			message += " [A dump file has been created.] ";
		}
#endif

		CNxErrorDialog dlg;

		message.TrimRight("\n");
		message.TrimRight(" ");

		// (b.cardillo 2006-11-15 16:14) - PLID 15422 - Get the sourcefile name (no path).  We 
		// used to do all kinds of string manipulations here to coerce everything into a 
		// meaningful error message.  Now we send each raw piece of information into the 
		// ShowErrorMessage() call below, and we let it worry about presenting everything to 
		// the user in a meaningful way.
		CString strSourcefileName;
		if (file) {
			strSourcefileName = GetFileName(file);
		}

		// (a.walling 2010-08-04 09:20) - PLID 38964 - I got rid of GetErrorHelpLocation and GetErrorHelpBookmark since they were not used.

		if (e)
		{	CRuntimeClass *pRuntime;
			pRuntime = e->GetRuntimeClass();
			
			if (!strcmp (pRuntime->m_lpszClassName, "COleDispatchException"))
			{	switch ((((COleDispatchException *)e)->m_scError)) 
				{	case 0x800a1436:
						{
						// (b.cardillo 2006-11-15 16:19) - PLID 15422 - We now send each piece of 
						// information in a separate parameter.
						BOOL bUserRequestedDebugBreak = 
							ShowErrorMessage(dlg,
							strSourcefileName, line, message, 
							"COleDispatchException", 
							"Could not open file, the file may not exist, or may be inaccessible due to network problems.", 
							"Dispatch Error", 
							ErrorLevel::BUG_ERROR);
						if (bUserRequestedDebugBreak) {
							// This hard-coded breakpoint is here so that when you get an ugly error in debug mode, you can 
							// see the call stack.  Step out of this function if you want to see where the error came from 
							NX_ASSERT_BREAK();
						}
						if (bAutoDelete) e->Delete();
						}
						return 0;
					default:
					{	CString code;
						code.Format ("%i", ((COleDispatchException*)e)->m_scError);
						// (b.cardillo 2006-12-13 12:09) - PLID 23852 - We need to pass bCalledFromThread be passed into this call 
						// to ShowErrorMessage() just like we do elsewhere.
						// (b.cardillo 2006-11-15 16:19) - PLID 15422 - We now send each piece of 
						// information in a separate parameter.
						BOOL bUserRequestedDebugBreak = 
							ShowErrorMessage(dlg,
							strSourcefileName, line, message, 
							"COleDispatchException",
							"(" + code + ") " + ((COleDispatchException *)e)->m_strDescription, 
							"Dispatch Error", 
							ErrorLevel::BUG_ERROR, strManualLocation, strManualBookmark, bCalledFromThread);
						if (bUserRequestedDebugBreak) {
							// This hard-coded breakpoint is here so that when you get an ugly error in debug mode, you can 
							// see the call stack.  Step out of this function if you want to see where the error came from 
							NX_ASSERT_BREAK();
						}
						if (bAutoDelete) e->Delete();
						return 0;
					}
				}
			}
			else
			{	
				char errorMessage[4096];
				// (b.cardillo 2006-12-13 12:09) - PLID 23852 - If the call to GetErrorMessage() fails, we need to fill 
				// the string with a generic message instead of leaving it as garbage.
				if (!e->GetErrorMessage(errorMessage, 4096)) {
					strcpy(errorMessage, "Unknown CException!");
				}
				// (b.cardillo 2006-12-13 12:09) - PLID 23852 - We need to pass bCalledFromThread be passed into this call 
				// to ShowErrorMessage() just like we do elsewhere.
				// (b.cardillo 2006-11-15 16:19) - PLID 15422 - We now send each piece of 
				// information in a separate parameter.
				BOOL bUserRequestedDebugBreak = 
					ShowErrorMessage (dlg, 
					strSourcefileName, line, message,
					pRuntime->m_lpszClassName,
					errorMessage,
					"Error", 
					ErrorLevel::BUG_ERROR, strManualLocation, strManualBookmark, bCalledFromThread);
				if (bUserRequestedDebugBreak) {
					// This hard-coded breakpoint is here so that when you get an ugly error in debug mode, you can 
					// see the call stack.  Step out of this function if you want to see where the error came from 
					NX_ASSERT_BREAK();
				}
				if (bAutoDelete) e->Delete();
				return 0;
			}
		}
		// (b.cardillo 2006-11-15 16:19) - PLID 15422 - We now send each piece of 
		// information in a separate parameter.
		BOOL bUserRequestedDebugBreak = ShowErrorMessage(dlg, strSourcefileName, line, message, "UNKNOWN", "NULL", "Error", ErrorLevel::BUG_ERROR, strManualLocation, strManualBookmark, bCalledFromThread);

		if (bUserRequestedDebugBreak) {
			// This hard-coded breakpoint is here so that when you get an ugly error in debug mode, you can 
			// see the call stack.  Step out of this function if you want to see where the error came from 
			NX_ASSERT_BREAK();
		}

		return 0;
	} catch (...) {
		//DRT 6/11/2008 - PLID 27746 - We can only message box if we're not in a thread.
		if(!bCalledFromThread) {
			AfxMessageBox(
				"An unexpected error was encountered, but a second error occurred "
				"while Practice was attempting to gather information.\r\n\r\n"
				"If this error persists please contact NexTech for assistance.", MB_ICONHAND|MB_OK);
		}
		else {
			//If we're in a thread, and there's a second exception, there's really nothing we can do about it.  It's not safe to log, 
			//	it's not safe to pop up a message box.  We'll just quit silently.  The message box doesn't actually do anything useful
			//	anyways.
			//We'll just assert for debug mode, so testers know that something happened, but it won't crash clients.
			ASSERT(FALSE);
		}
		return 0;
	}
}

BOOL UpdateUnited (DWORD dwPracticeID)
{
	CString strRemotePath = GetRemotePropertyText("UnitedDataPath");
	DWORD dwRemoteID, dwRetval;
	BOOL bStop;
	EGenericLinkPersonAction action;

	try
	{
		if (!IsUnitedEnabled()) {
			MsgBox("The link to United is currently disabled. Please go to Modules=>Link to United to reconnect to the United Database.");
			return TRUE; // (c.haag 2003-10-17 09:31) - In this function, TRUE = failure.
		}

		_ConnectionPtr pConRemote;
		if (NULL == (pConRemote = GetMainFrame()->GetUnitedLink()->GetRemoteData())) {
			MsgBox("The United Imaging database is invalid, or is inaccessible from your computer.\r\n\r\nThe link to United will be disabled for this session of Practice. Please go to Modules=>Link to United to reconnect with the United Imaging database.");
			GetMainFrame()->GetUnitedLink()->EnableLink(FALSE);
			return TRUE;
		}

		CUnitedLinkPatient pat(NULL, ::GetRemoteData(), pConRemote, strRemotePath);
		dwRetval = pat.Export(dwPracticeID, dwRemoteID, bStop, action);
	} NxCatchAll("Error updating United Database");

	return dwRetval;
}

BOOL UpdateInform (unsigned long patientID)
{
	CWaitCursor pWait;
	BOOL IsFullVersion = TRUE;
	CFileDialog Browse(TRUE);

	_variant_t var;

	CString informPath;

	long	informCN,
			practiceID = patientID,
			PracUserDefinedID,
			InformID = 0,
			RefID = 0,
			RefDetailID = 0;
	CString	sql,
			ssn,
			first, 
			last,
			middle;
	_RecordsetPtr rsID(__uuidof(Recordset)),rsPractice;

	bool NewPatient = true;

	_ConnectionPtr informdb(__uuidof(Connection));

	BOOL bIsValidPath = FALSE;

	while(!bIsValidPath) {
		try {
			CDoesExist de;
			informPath = GetRemotePropertyText ("InformDataPath", "", 0, "<None>");
			if(informPath != "")
			{
				de.SetPath(informPath);
				de.TryAccess();
			}

			if(informPath=="" || CDoesExist::eFailed == de.WaitForAccess("Please wait while Practice connects to your Inform database...")) {
				MsgBox("The Inform Link has not been properly set.\n"
					"On the following screen, please locate your Consent.mdb file.");
				Browse.m_ofn.lpstrInitialDir = informPath;
				Browse.m_ofn.lpstrFilter = "Inform Database Files\0*.mde;*.mdb\0All Files\0*.*\0\0";
				if (Browse.DoModal() == IDCANCEL) 
					return FALSE;
				informPath = Browse.GetPathName();
			}

			if(informdb->State != adStateClosed)
				informdb->Close();

			try {
				//open a connection for recordsets we will use
				informdb->Provider = "Microsoft.Jet.OLEDB.4.0";
				informdb->Open(_bstr_t(informPath),"","",NULL);
			}catch(_com_error e) {
				//if an error occurred, the database is invalid. Don't crash, just fix it!
				AfxMessageBox("The database you selected is not a valid Inform database.");
				bIsValidPath = FALSE;
				SetRemotePropertyText("InformDataPath", "", 0, "<None>");
				continue;
			}
			
			_RecordsetPtr rsID(__uuidof(Recordset));
			try {
				//check the two key tables to identify the database as being an inform database
				rsID->Open("SELECT Count(strVersion) AS Total FROM tblVersion", _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				rsID->Close();
				rsID->Open("SELECT Count(ID) AS Total FROM tblPatient", _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				rsID->Close();
			} catch(_com_error e) {
				//THIS is what we want to happen. If the above tables do not exist, then it's not an Inform database.
				AfxMessageBox("The database you selected is not a valid Inform database.");
				bIsValidPath = FALSE;
				SetRemotePropertyText("InformDataPath", "", 0, "<None>");
				continue;
			}

			//if we get this far, then we know it is indeed an inform database, and we can access it safely
			bIsValidPath = TRUE;

			//checking version of Inform, most everyone is full, so assume full if it cannot be determined
			CString sql,strVersion;
			sql.Format("SELECT strVersion FROM tblVersion");
			rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
			if(!rsID->eof) {
				strVersion = CString(rsID->Fields->Item["strVersion"]->Value.bstrVal);
				if(strVersion.Left(1)<"3") {
					IsFullVersion = FALSE;
				}
				else {
					IsFullVersion = TRUE;
				}
			}
			else {
				IsFullVersion = TRUE;
			}
		}NxCatchAll("Error, Could not set inform path.");
	}

	//you cannot get here until you have selected a valid inform database, so only NOW should we save the name.
	SetRemotePropertyText ("InformDataPath", informPath, 0, "<None>");

	try {

		//Make sure we have a name
		rsID = CreateRecordset("SELECT [First], [Last], [Middle], [SocialSecurity], InformID, UserDefinedID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = %li", practiceID);	
		
		var = rsID->Fields->Item["First"]->Value;
		if (var.vt == VT_BSTR)
			first = CString(var.bstrVal);
		
		var = rsID->Fields->Item["Last"]->Value;
		if (var.vt == VT_BSTR)
			last = CString(var.bstrVal);
		
		var = rsID->Fields->Item["Middle"]->Value;
		if (var.vt == VT_BSTR)
			middle = CString(var.bstrVal);

		PracUserDefinedID = AdoFldLong(rsID, "UserDefinedID");
		
		if (IsFullVersion && rsID->Fields->Item["SocialSecurity"]->Value.vt == VT_BSTR)
			ssn	= CString(rsID->Fields->Item["SocialSecurity"]->Value.bstrVal);
		//if there is an InformID on our side, this patient is not new in Inform (theoretically)
		var = rsID->Fields->Item["InformID"]->Value;
		if (var.vt != VT_NULL && var.lVal > 0) {
			NewPatient = false;
			InformID = var.lVal;
		}
		rsID->Close();
		if (first == "" || last == "") {
			AfxMessageBox("Inform requires both a first and last name, could not export patient \"" + first + " " + last + ".\"");
			return false;
		}

		ssn.Replace(" ","");
		ssn.Replace("-","");
		ssn.Replace("#","");

		//Make sure SSN is unique or blank, but only if it is a new patient in inform
		if (ssn != "" && IsFullVersion) {
			if(NewPatient) {
				sql.Format("SELECT [Last], [First], SSN FROM tblPatient WHERE SSN = '%s'", ssn);
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				if (!rsID->eof)
				{	AfxMessageBox("Patient \"" + CString(rsID->Fields->Item["First"]->Value.bstrVal) + " " + CString(rsID->Fields->Item["Last"]->Value.bstrVal) + "\" already exists in Inform with the Social Security Number " + ssn + ",\ncould not export patient \"" + first + " " + last + ".\"");
					rsID->Close();
					return false;
				}
				rsID->Close();
			}
			else {
				sql.Format("SELECT ID, [Last], [First], SSN FROM tblPatient WHERE SSN = '%s' AND ID <> %li ", ssn, InformID);
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				if (!rsID->eof)
				{	AfxMessageBox("Patient \"" + CString(rsID->Fields->Item["First"]->Value.bstrVal) + " " + CString(rsID->Fields->Item["Last"]->Value.bstrVal) + "\" already exists in Inform with the Social Security Number " + ssn + ",\ncould not export patient \"" + first + " " + last + ".\"");
					rsID->Close();
					return false;
				}
				rsID->Close();
			}
		}

		CExportDuplicates dlg(NULL);
		if(NewPatient && dlg.FindDuplicates(first,last,middle,"Practice","Inform",informdb,informPath)) {
			int choice = dlg.DoModal();
			switch(choice) {
			case(1):
				//add new
				//do nothing different
				break;
			case(2):
				//update
				ExecuteSql("UPDATE PatientsT SET InformID = %li WHERE PersonID = %li",dlg.m_varIDToUpdate.lVal,practiceID);
				NewPatient = FALSE;
				InformID = dlg.m_varIDToUpdate.lVal;
				//continue normally
				break;
			case(3):
				//skip
				return FALSE;
				break;
			default:
				//stop
				return FALSE;
				break;
			}
		}

		//check to see if this chartnumber is in Inform
		if(NewPatient) {
			sql.Format("SELECT ChartNumber FROM tblPatient WHERE ChartNumber = '%li'", PracUserDefinedID);
			rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
			//if it is in inform
			if (!rsID->eof) {
				//if the patient is new, get a new number
				rsID->Close();
				//(e.lally 2009-10-09) PLID 35899 - Ignore numeric chart numbers that are larger than Max Long (2147483647)
				sql.Format("SELECT Max(CLng(IIf(ChartNumber Is Null,0,IIf(IsNumeric(ChartNumber),IIf(CDbl(ChartNumber) <= 2147483647, ChartNumber, 0),0)))) AS MaxChartNum FROM tblPatient");
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				var = rsID->Fields->Item["MaxChartNum"]->Value;
				informCN = var.lVal + 1;
			}
			else {
				informCN = PracUserDefinedID;
			}
			rsID->Close();
		}
		else {
			sql.Format("SELECT ID, ChartNumber FROM tblPatient WHERE ID = %li",InformID);
			rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
			if(rsID->eof) {
				//the patient was deleted in inform!!! recopy
				NewPatient = true;
				rsID->Close();
				sql.Format("SELECT ChartNumber FROM tblPatient WHERE ChartNumber = '%li'", PracUserDefinedID);
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				//if it is in inform
				if (!rsID->eof) {
					//if the patient is new, get a new number
					rsID->Close();
					//(e.lally 2009-10-09) PLID 35899 - Ignore numeric chart numbers that are larger than Max Long (2147483647)
					sql.Format("SELECT Max(CLng(IIf(ChartNumber Is Null,0,IIf(IsNumeric(ChartNumber),IIf(CDbl(ChartNumber) <= 2147483647, ChartNumber, 0),0)))) AS MaxChartNum FROM tblPatient");
					rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
					var = rsID->Fields->Item["MaxChartNum"]->Value;
					informCN = var.lVal + 1;
				}
				else {
					informCN = PracUserDefinedID;
				}
			}
			else {
				informCN = atol(CString(rsID->Fields->Item["ChartNumber"]->Value.bstrVal));			
			}
			rsID->Close();
		}

		//////////////////////////////////////////////////////////////
		//REFERRAL SOURCE

		long PracRefParent = -1;
		CString Refsource;

		if(IsFullVersion) {

			//first check to see if the referral source is a detailed one
			sql.Format("SELECT ReferralSourceT.Name FROM PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID WHERE ReferralID > 0 AND ReferralSourceT.Name Is Not Null AND PatientsT.PersonID = %li",practiceID);
			rsID = CreateRecordsetStd(sql);
			if (!rsID->eof) {
				Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
				rsID->Close();
				sql.Format("SELECT ID, RefSourceID, Name FROM tblSourceDetOther WHERE Name = '%s'",_Q(Refsource));
				rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
				if(!rsID->eof) {
					RefDetailID = rsID->Fields->Item["ID"]->Value.lVal;
					RefID = rsID->Fields->Item["RefSourceID"]->Value.lVal;
				}
			}
			rsID->Close();

			//if that try was unsuccessful, try looking at parent sources
			if(RefID==0) {
				sql.Format("SELECT ReferralSourceT.Name, ReferralSourceT.Parent FROM PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID WHERE ReferralID > 0 AND ReferralSourceT.Name Is Not Null AND PatientsT.PersonID = %li",practiceID);
				rsID = CreateRecordsetStd(sql);
				if (!rsID->eof) {
					Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
					//get the parent incase we have to go to a third iteration
					PracRefParent = rsID->Fields->Item["Parent"]->Value.lVal;
					rsID->Close();
					sql.Format("SELECT ID, Label FROM tblRefSource WHERE Label = '%s'",_Q(Refsource));
					rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
					if(!rsID->eof) {
						RefID = rsID->Fields->Item["ID"]->Value.lVal;
					}		
				}
				rsID->Close();
			}

			//and if THAT try was unsuccessful it's time to be sneaky. We could give up now, but that's not the NexTech way. haHA!
			//now check to see if the parent of the selected referral source is itself a detailed or regular referral source
			if(RefID==0 && PracRefParent != -1) {
				//first check to see if the referral source is a detailed one
				sql.Format("SELECT Name FROM ReferralSourceT WHERE PersonID = %li",PracRefParent);
				rsID = CreateRecordsetStd(sql);
				if (!rsID->eof) {
					Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
					rsID->Close();
					sql.Format("SELECT ID, RefSourceID, Name FROM tblSourceDetOther WHERE Name = '%s'",Refsource);
					rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
					if(!rsID->eof) {
						RefDetailID = rsID->Fields->Item["ID"]->Value.lVal;
						RefID = rsID->Fields->Item["RefSourceID"]->Value.lVal;
					}		
				}
				rsID->Close();
				
				if(RefID==0) {
					sql.Format("SELECT Name FROM ReferralSourceT WHERE PersonID = %li",PracRefParent);
					rsID = CreateRecordsetStd(sql);
					if (!rsID->eof) {
						Refsource = CString(rsID->Fields->Item["Name"]->Value.bstrVal);
						rsID->Close();
						sql.Format("SELECT ID, Label FROM tblRefSource WHERE Label = '%s'",_Q(Refsource));
						rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
						if(!rsID->eof) {
							RefID = rsID->Fields->Item["ID"]->Value.lVal;
						}
					}
					rsID->Close();
				}				
			}

			//This gets the values
			rsPractice = CreateRecordset("SELECT Archived, (CASE WHEN PrefixT.InformID Is Null THEN 0 ELSE PrefixT.InformID END) AS SalutationID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, "
				"Extension, CellPhone, Email, (CASE WHEN(PersonT.[Gender]=1) THEN 'M' WHEN(PersonT.[Gender]=2) THEN 'F' ELSE '' END) AS MF, "
				"Fax, BirthDate, SocialSecurity, FirstContactDate, Nickname FROM PatientsT INNER JOIN (PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON PatientsT.PersonID = PersonT.ID WHERE PersonT.ID = %li", practiceID);

			//safer than doing in the sql format

			long Archived,SalutationID;
			CString First,Last,Middle,Nickname,SocialSecurity,BirthDate,FirstContactDate,Address1,Address2,
				City,State,Zip,HomePhone,WorkPhone,Email,CellPhone,Extension,Fax,NickName,Gender;
			COleDateTime birthdate,firstdate,inputdate;
			
			//this is SO much safer then doing all the rs-> calls inside CreateRecordset
			if(rsPractice->Fields->Item["Archived"]->Value.vt==VT_NULL)
				Archived = 0;
			else
				Archived = rsPractice->Fields->Item["Archived"]->Value.boolVal;
			Gender = CString(rsPractice->Fields->Item["MF"]->Value.bstrVal);
			if(rsPractice->Fields->Item["SalutationID"]->Value.vt == VT_NULL)
				SalutationID = 0;
			else
				SalutationID = rsPractice->Fields->Item["SalutationID"]->Value.lVal;
			if(rsPractice->Fields->Item["First"]->Value.vt==VT_NULL)
				First = "";
			else
				First = _Q(CString(rsPractice->Fields->Item["First"]->Value.bstrVal));
			if(rsPractice->Fields->Item["Last"]->Value.vt==VT_NULL)
				Last = "";
			else
				Last = _Q(CString(rsPractice->Fields->Item["Last"]->Value.bstrVal));
			if(rsPractice->Fields->Item["Middle"]->Value.vt==VT_NULL)
				Middle = "";
			else
				Middle = _Q(CString(rsPractice->Fields->Item["Middle"]->Value.bstrVal));
			if(rsPractice->Fields->Item["Nickname"]->Value.vt==VT_NULL)
				Nickname = "";
			else
				Nickname = _Q(CString(rsPractice->Fields->Item["Nickname"]->Value.bstrVal));
			if(rsPractice->Fields->Item["SocialSecurity"]->Value.vt==VT_NULL)
				SocialSecurity = "NULL";
			else {
				SocialSecurity = CString(rsPractice->Fields->Item["SocialSecurity"]->Value.bstrVal);
				SocialSecurity.Replace(" ","");
				SocialSecurity.Replace("-","");
				SocialSecurity.Replace("#","");
				SocialSecurity = "'" + SocialSecurity + "'";
			}
			if(SocialSecurity=="''")
				SocialSecurity="NULL";
			if(rsPractice->Fields->Item["Address1"]->Value.vt==VT_NULL)
				Address1 = "";
			else
				Address1 = _Q(CString(rsPractice->Fields->Item["Address1"]->Value.bstrVal));
			if(Address1.GetLength()>30) {
				AfxMessageBox("Inform only supports an Address 1 field that is 30 characters in length.\n"
					"You must edit the account of patient '" + Last + ", " + First + "' before exporting.");
				rsPractice->Close();
				return FALSE;
			}
			if(rsPractice->Fields->Item["Address2"]->Value.vt==VT_NULL)
				Address2 = "NULL";
			else
				Address2.Format("'%s'", _Q(AdoFldString(rsPractice, "Address2")));
			if(Address2 == "''" || Address2 == "") {
				Address2 = "NULL";
			}
			if(Address2.GetLength()>50) {
				AfxMessageBox("Inform only supports an Address 2 field that is 50 characters in length.\n"
					"You must edit the account of patient '" + Last + ", " + First + " before exporting.");
				rsPractice->Close();
				return FALSE;
			}
			if(rsPractice->Fields->Item["City"]->Value.vt==VT_NULL)
				City = "";
			else
				City = _Q(CString(rsPractice->Fields->Item["City"]->Value.bstrVal));
			if(City.GetLength()>30) {
				AfxMessageBox("Inform only supports a City field that is 30 characters in length.\n"
					"You must edit the account of patient '" + Last + ", " + First + " before exporting.");
				rsPractice->Close();
				return FALSE;
			}
			if(rsPractice->Fields->Item["State"]->Value.vt==VT_NULL)
				State = "";
			else
				State = _Q(CString(rsPractice->Fields->Item["State"]->Value.bstrVal));
			if(rsPractice->Fields->Item["Zip"]->Value.vt==VT_NULL)
				Zip = "";
			else
				Zip = _Q(CString(rsPractice->Fields->Item["Zip"]->Value.bstrVal));
			if(rsPractice->Fields->Item["BirthDate"]->Value.vt==VT_NULL)
				BirthDate = "NULL";
			else {
				birthdate = rsPractice->Fields->Item["BirthDate"]->Value.date;
				BirthDate = birthdate.Format("'%m/%d/%Y'");
			}
			if(rsPractice->Fields->Item["FirstContactDate"]->Value.vt==VT_NULL)
				FirstContactDate = "NULL";
			else {
				firstdate = rsPractice->Fields->Item["FirstContactDate"]->Value.date;
				FirstContactDate = firstdate.Format("'%m/%d/%Y'");
			}
			if(rsPractice->Fields->Item["HomePhone"]->Value.vt==VT_NULL)
				HomePhone = "";
			else
				HomePhone = CString(rsPractice->Fields->Item["HomePhone"]->Value.bstrVal);
			if(rsPractice->Fields->Item["WorkPhone"]->Value.vt==VT_NULL)
				WorkPhone = "";
			else
				WorkPhone = CString(rsPractice->Fields->Item["WorkPhone"]->Value.bstrVal);
			if(rsPractice->Fields->Item["Extension"]->Value.vt==VT_NULL)
				Extension = "";
			else
				Extension = CString(rsPractice->Fields->Item["Extension"]->Value.bstrVal);
			if(Extension.GetLength()>6) {
				AfxMessageBox("Inform only supports a phone Extension field that is 6 characters in length.\n"
				"You must edit the account of patient '" + Last + ", " + First + " before exporting.");
				rsPractice->Close();
				return FALSE;
			}
			if(rsPractice->Fields->Item["Fax"]->Value.vt==VT_NULL)
				Fax = "";
			else
				Fax = CString(rsPractice->Fields->Item["Fax"]->Value.bstrVal);
			if(rsPractice->Fields->Item["EMail"]->Value.vt==VT_NULL)
				Email = "";
			else
				Email = _Q(CString(rsPractice->Fields->Item["EMail"]->Value.bstrVal));
			if(rsPractice->Fields->Item["CellPhone"]->Value.vt==VT_NULL)
				CellPhone = "";
			else
				CellPhone = CString(rsPractice->Fields->Item["CellPhone"]->Value.bstrVal);
			rsPractice->Close();

			BOOL bIgnoreSourceDetailInfo = FALSE;

			try
			{
				informdb->Execute("SELECT TOP 1 SourceDetailID FROM tblPatient", NULL, adCmdText);
			}
			catch(_com_error)
			{
				bIgnoreSourceDetailInfo = TRUE;
			}

			if(NewPatient) {
				if(!bIgnoreSourceDetailInfo)
					sql.Format("INSERT INTO tblPatient (ChartNumber, RefSourceID, SourceDetailID, SalutationID, Inactive, [First], Middle, [Last], Address1, Address2, City, State, "
						"Zip, NightPhone, DayPhone, DayExt, CellPhone, EMail, [M/F], Fax, "
						"BirthDate, SSN, DateOfContact, Salutation) VALUES "
						"(%li,%li,%li,%li,%li,'%s','%s','%s','%s',%s,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s', %s, %s, %s,'%s')",
						informCN,RefID,RefDetailID,SalutationID,Archived,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,CellPhone,
						Email,Gender,Fax,BirthDate,SocialSecurity,FirstContactDate,Nickname);
				else
					sql.Format("INSERT INTO tblPatient (ChartNumber, RefSourceID, SalutationID, Inactive, [First], Middle, [Last], Address1, Address2, City, State, "
						"Zip, NightPhone, DayPhone, DayExt, [M/F], Fax, "
						"BirthDate, SSN, DateOfContact, Salutation) VALUES "
						"(%li,%li,%li,%li,'%s','%s','%s','%s',%s,'%s','%s','%s','%s','%s','%s','%s','%s', %s, %s, %s,'%s')",
						informCN,RefID,SalutationID,Archived,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,
						Gender,Fax,BirthDate,SocialSecurity,FirstContactDate,Nickname);
			}
			else {
				if(!bIgnoreSourceDetailInfo)
					sql.Format("UPDATE tblPatient SET RefSourceID = %li, SourceDetailID = %li, SalutationID = %li, Inactive = %li, [First] = '%s', Middle = '%s', [Last] = '%s', Address1 = '%s', Address2 = %s, City = '%s', State = '%s', "
						"Zip = '%s', NightPhone = '%s', DayPhone = '%s', DayExt = '%s', CellPhone = '%s', EMail ='%s', [M/F] = '%s', Fax = '%s', "
						"BirthDate = %s, SSN = %s, DateOfContact = %s, Salutation = '%s' WHERE ID = %li",
						RefID,RefDetailID,SalutationID,Archived,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,CellPhone,
						Email,Gender,Fax,BirthDate,SocialSecurity,FirstContactDate,Nickname,InformID);
				else
					sql.Format("UPDATE tblPatient SET RefSourceID = %li, SalutationID = %li, Inactive = %li, [First] = '%s', Middle = '%s', [Last] = '%s', Address1 = '%s', Address2 = %s, City = '%s', State = '%s', "
						"Zip = '%s', NightPhone = '%s', DayPhone = '%s', DayExt = '%s', [M/F] = '%s', Fax = '%s', "
						"BirthDate = %s, SSN = %s, DateOfContact = %s, Salutation = '%s' WHERE ID = %li",
						RefID,SalutationID,Archived,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,WorkPhone,Extension,
						Gender,Fax,BirthDate,SocialSecurity,FirstContactDate,Nickname,InformID);
			}
			informdb->Execute(_bstr_t(sql),NULL,adCmdText);
		}
		else {
			if(NewPatient) {
				CString FirstContactDate;
				COleDateTime firstdate;
				rsPractice = CreateRecordset("SELECT FirstContactDate FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE ID = %li", practiceID);
				if(rsPractice->Fields->Item["FirstContactDate"]->Value.vt==VT_NULL)
					FirstContactDate = "NULL";
				else {
					firstdate = rsPractice->Fields->Item["FirstContactDate"]->Value.date;
					FirstContactDate = firstdate.Format("%m/%d/%Y");
					FirstContactDate = "'" + FirstContactDate + "'";
				}
				rsPractice->Close();
				sql.Format("INSERT INTO tblPatient (ChartNumber, [Last], [First], Middle, DateOfContact) VALUES (%li,'%s','%s','%s', %s)",
					informCN,_Q(last),_Q(first),_Q(middle),FirstContactDate);
				informdb->Execute(_bstr_t(sql),NULL,adCmdText);
			}
			else {
				CString FirstContactDate;
				COleDateTime firstdate;
				rsPractice = CreateRecordset("SELECT FirstContactDate FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE ID = %li", practiceID);
				if(rsPractice->Fields->Item["FirstContactDate"]->Value.vt==VT_NULL)
					FirstContactDate = "NULL";
				else {
					firstdate = rsPractice->Fields->Item["FirstContactDate"]->Value.date;
					FirstContactDate = firstdate.Format("'%m/%d/%Y'");
				}
				rsPractice->Close();
				sql.Format("UPDATE tblPatient SET [Last] = '%s', [First] = '%s', Middle = '%s', DateOfContact = %s WHERE ID = %li",
					_Q(last),_Q(first),_Q(middle),FirstContactDate,InformID);
				informdb->Execute(_bstr_t(sql),NULL,adCmdText);
			}
		}
			
		if(NewPatient) {
			sql.Format("SELECT ID FROM tblPatient WHERE ChartNumber = '%li'", informCN);
			rsID->Open(_bstr_t(sql), _variant_t((IDispatch *) informdb, true), adOpenStatic, adLockReadOnly, adCmdText);
			if(!rsID->eof) {
				ExecuteSql("UPDATE PatientsT SET InformID = %li WHERE PersonID = %li;",rsID->Fields->Item["ID"]->Value.lVal, practiceID);
			}
			rsID->Close();
		}
		else {
			ExecuteSql("UPDATE PatientsT SET InformID = %li WHERE PersonID = %li;",InformID, practiceID);
		}
		return true;
	}NxCatchAll("Error in Link To Inform");
	return false;
}


int Prompt (CString Caption, CString &NewName, int nMaxLength)
{
	CGetNewIDName dlg(NULL);
	dlg.m_pNewName = &NewName;
	dlg.m_strCaption = Caption;
	dlg.m_nMaxLength = nMaxLength;
	return dlg.DoModal();
}

DWORD SetControlIcon (CWnd *parent, UINT control, UINT icon) {
	((CButton *)(parent->GetDlgItem(control)))->SetIcon(AfxGetApp()->LoadIcon(icon));
	return GetLastError();
}

inline bool IsKeyDown(long nVirtKey)
{
	if (GetAsyncKeyState(nVirtKey) & 0x80000000) {
		return true;
	} else {
		return false;
	}
}

bool IsSuperUser()
{
	// Check for special key
	if (IsKeyDown(VK_SHIFT) ||
		 IsKeyDown(VK_CONTROL) ||
		 IsKeyDown(VK_NUMLOCK) ||
		 IsKeyDown(VK_SPACE)) {
		return true;
	} else {
		return false;
	}
}

// (a.walling 2010-10-14 14:15) - PLID 40965 - This is now an alias basically for !ReturnsRecords
bool IsRecordsetEmpty(LPCTSTR strFmt, ...)
{
	CString strSql;

	// Parse string inserting arguments where appropriate
	va_list argList;
	va_start(argList, strFmt);
	strSql.FormatV(strFmt, argList);
	va_end(argList);

	try {
		return ReturnsRecords("%s", strSql) ? false : true;
	} catch (_com_error& e) {
		// (a.walling 2010-10-14 15:50) - PLID 40965 - This means there was something sent to IsRecordsetEmpty that couldn't be embedded within ReturnsRecords!
		ASSERT(FALSE); 

		// (a.walling 2010-10-14 15:50) - PLID 40965 - Now throw rather than fallback
		throw e;
	}

	/*
	// Now check to see if the formatted sql statement yields any records
	if (GetRecordCount(strSql) == 0) {
		return true;
	} else {
		return false;
	}
	*/
}

#include "WhereClause.h"
long GetRecordCount(const CString &strSql)
{
	try {
		// Remove the orderby clause if there is one
		CString strLocSql(strSql);
		long nOrderBy, nNext;
		FindClause(strSql, fcOrderBy, nOrderBy, nNext);
		if (nOrderBy >= 0) {
			strLocSql.Delete(nOrderBy, nNext-nOrderBy);
		}
		// Run the query but only retrieve the count
//		Log("::GetRecordCount() query = SELECT COUNT(*) AS RecordCount FROM (%s) GetRecordCountQ", strLocSql);
		_RecordsetPtr prs = CreateRecordset("SELECT COUNT(*) AS RecordCount FROM (%s) GetRecordCountQ", strLocSql);
		return AdoFldLong(prs, "RecordCount");
	} NxCatchAllCall("::GetRecordCount", return 0);
}

//TES 9/18/2008 - PLID 31413 - Moved IncrementFileName() to HistoryUtils

CString GetTemplatePath(const CString &strSubFolder /*= ""*/, const CString &strTemplateFileName /*= ""*/)
{
	// Todo: Someday this can be changed to be more generic.  It could pull the 
	// templates path from the registry and then append the subfolder name to it
	return GetSharedPath() ^ "Templates" ^ strSubFolder ^ strTemplateFileName;
}

// For a statement like:
// str = BuildClause("WHERE", "AND", "", "One", "Two", "", "Four", -1);
//
// str will equal "WHERE One AND Two AND Four"
//
//
// This function ignores empty strings and NULLs, and only terminates when it hits the required -1 at the end
//
// Generic way of building a clause.  Call like: "BuildClause(strStartOp, strFillerOp, str1, str2, str3, str4, ..., strN, -1);"
// The function will return "strStartOp str1 strFillerOp str2 strFillerOp str3 strFillerOp str4 strFille...")
CString BuildClause(LPCTSTR strStartOp, LPCTSTR strFillerOp, ...)
{
	CString strAns;
	LPCTSTR lpszArg = NULL;

	va_list argList;
	va_start(argList, strFillerOp);
	
	// Loop until we hit the -1
	while ((lpszArg = va_arg(argList, LPCTSTR)) != (LPCTSTR)-1) {
		// If the current argument is empty string or null, skip it, otherwise append it
		if (lpszArg && strlen(lpszArg) > 0) {
			if (strAns.GetLength() == 0) {
				// If the return string is empty, we want to start with the StartOp
				strAns = CString(strStartOp) + " " + lpszArg;
			} else {
				// If the return string is not empty, we want to append with the FilerOp
				strAns += " " + CString(strFillerOp) + " " + lpszArg;
			}
		}
	}
	
	// Done looping through the arguments
	va_end(argList);

	// Return results
	return strAns;
}

// Quickly trims the trailing characters if they can be found in the strChars parameter
UINT rtrim(TCHAR *pStr, LPCTSTR strChars)
{
	UINT nCount = 0;

	// Find the terminating NULL
	TCHAR *pPos = pStr;
	while (*pPos) pPos++;
	
	// Then loop backwards until we've not removed any characters from the end
	LPCTSTR pInChars;
	while (pPos>pStr) {
		pPos--;
		for (pInChars=strChars; *pInChars; pInChars++) {
			if (*pPos == *pInChars) {
				// We found one of the characters in the strChars to be 
				// the last character in the string so truncate it and break
				*pPos = '\0';
				nCount++;
				break;
			}
		}
		// The only way for *pInChars to be pointing to a NULL character is if 
		// no characters from the strChars were encountered, so we're done
		if (*pInChars == '\0') {
			break;
		}
	}
	
	return nCount;
}
/*
bool DeleteQuery(const CString &strQueryName)
{
	CString strObjName(strQueryName);
	strObjName.TrimLeft(); strObjName.TrimRight();
	if (!strObjName.IsEmpty()) {
		if (strObjName.Find("[", 1) == -1) strObjName.TrimLeft("["); strObjName.TrimRight("]");
		try {
			SAFE_ENSURE_PRACTICE();
			g_dbPractice.DeleteQueryDef(strObjName);
			return true;
		} NxCatch("Could not delete query " + strObjName);
		return false;
	} else {
		return true;
	}
}*/

/*bool DeleteTable(const CString &strTableName)
{
	CString strObjName(strTableName);
	strObjName.TrimLeft(); strObjName.TrimRight();
	if (!strObjName.IsEmpty()) {
		if (strObjName.Find("[", 1) == -1) strObjName.TrimLeft("["); strObjName.TrimRight("]");
		try {
			SAFE_ENSURE_PRACTICE();
			g_dbPractice.DeleteTableDef(strObjName);
			return true;
		} NxCatch("Could not delete table " + strObjName);
		return false;
	} else {
		return true;
	}
}*/

#define PATIENT_TYPE_COLOR				0xFFB9A8
#define PATIENT_PROSPECT_TYPE_COLOR		0xACD2A4
//DRT 4/18/2008 - PLID 29723 - Changed prospects from 0xACD1E6 to RGB(227, 194, 185)
#define PROSPECT_TYPE_COLOR				RGB(227, 194, 185)
//DRT 4/18/2008 - PLID 29723 - Changed admin from 0x8080FF to RGB(255, 206, 206)
//DRT 4/23/2008 - PLID 29723 - Changed admin from RGB(255, 206, 206) to RGB(255, 170, 170)
#define ADMIN_COLOR						RGB(255, 170, 170)
//DRT 4/18/2008 - PLID 29723 - Changed contacts from 0x77C8FF to RGB(254, 233, 148)
#define CONTACT_COLOR					RGB(254, 233, 148)
//DRT 4/18/2008 - PLID 29723 - Changed contacts from 0xD1B8AF to RGB(214, 232, 198)
#define FINANCIAL_COLOR					RGB(214, 232, 198)
// (a.walling 2009-08-12 16:12) - PLID 35136 - These were undefined
#define INVENTORY_COLOR					RGB(219, 219, 255) 
#define LETTER_COLOR					RGB(255, 228, 202)
#define MARKET_COLOR					RGB(255, 255, 200)
#define FILTER_USE_OR_COLOR				RGB(0xEF, 0xD4, 0xBA)
#define FILTER_USE_AND_COLOR			RGB(0xFF, 0xF4, 0xDA)
//DRT 4/21/2008 - PLID 29723 - Set Reports color to RGB(168, 216, 227)
#define REPORT_COLOR					RGB(168, 216, 227)

//DRT 4/30/2008 - PLID 29771 - Added color here for EMRItemAdv... backgrounds, since the code is duplicated
//	for regular windows and popup windows.
#define EMR_ITEM_BG_COLOR				RGB(233, 231, 254)

//these are very light colors, designed for coloring rows in surgeries
//TODO: find out the correct 0x... values for these
#define CPT_COLOR						RGB(255,230,230) //red, like the Admin. module
#define PRODUCT_COLOR					RGB(230,245,255) //blue, like the Inventory module
#define PERSONNEL_COLOR					RGB(255,240,220) //yellow-orange, like the Contacts module

// (a.walling 2009-08-13 09:39) - PLID 35136 - Move toolbar colors to global GetNxColor
#define TOOLBAR_TOP_COLOR				RGB(0xD0, 0xDF, 0xF2)
#define TOOLBAR_BOTTOM_COLOR			RGB(0x57, 0x7E, 0xB1)
#define TOOLBAR_AVG_COLOR				RGB(0xBA, 0xCE, 0xE7)
// (a.walling 2009-08-12 16:12) - PLID 35136 - Move MDI client background color to global GetNxColor
#define MDICLIENT_AVG_COLOR				RGB(227, 242, 241)

// Returns the color of the specified index within the category
// (a.walling 2009-08-13 09:43) - PLID 35136 - All should return PaletteColor version, which basically ORs with 0x02000000
// And now we return a COLORREF rather than a DWORD
// (b.spivey, May 15, 2012) - PLID 20752 - Added a new param with a default. 
COLORREF GetNxColor(long nCategory, long nIndex, long nPatientID /*= -1*/)
{
	COLORREF color = RGB(0xFF, 0xFF, 0xFF);
	switch (nCategory) {
	case GNC_PATIENT_STATUS:
		{
			// (b.spivey, May 15, 2012) - PLID 20752 - If we're updating the patient module and we have a patient, a type of 
			//	 patient, and a color that is not -1 then we can update using the custom color. Otherwise follow old code paths. 
#pragma TODO("Performance concern -- do we really want to be creating a recordset for this?")
			if (nPatientID != -1) {
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation when querying patient status for the patient dialog nxcolor (which happens far too often)
				_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT GT.Color FROM PatientsT PT "
					"LEFT JOIN GroupTypes GT ON PT.TypeOfPatient = GT.TypeIndex "
					"WHERE PT.PersonID = {INT} ", nPatientID); 
				if(!prs->eof) {
					int nColor = AdoFldLong(prs, "Color", -1); 
					if (nColor != -1) {
						color = nColor; 
						break; 
					}
				}
			}

			switch (nIndex) {
			case 1:
				color = PATIENT_TYPE_COLOR;
				break;
			case 3:
				color = PATIENT_PROSPECT_TYPE_COLOR;
				break;
			case 2: // Prospect is the default
			default:
				color = PROSPECT_TYPE_COLOR;
				break;
			}
		}
		break;
	case GNC_FILTER_USE_OR:
		// nIndex is interpreted as a boolean bUseOr 
		// (true indicates OR, false indicates AND)
		// The background color of filter-related stuff is return RGB(0xCA, 0xE4, 0xFF);
		if (nIndex) {
			// Or
			color = FILTER_USE_OR_COLOR;
		} else {
			// And
			color = FILTER_USE_AND_COLOR;
		}
		break;
	case GNC_ADMIN:
		color = ADMIN_COLOR;
		break;
	case GNC_CONTACT:
		color = CONTACT_COLOR;
		break;
	case GNC_FINANCIAL:
		color = FINANCIAL_COLOR;
		break;
	case GNC_INVENTORY:
		color = INVENTORY_COLOR;
		break;
	case GNC_LETTER:
		color = LETTER_COLOR;
		break;
	case GNC_MARKET:
		color = MARKET_COLOR;
		break;
	case GNC_REPORT:
		color = REPORT_COLOR;
		break;

	case GNC_CPT_CODE:
		color = CPT_COLOR;
		break;
	case GNC_PRODUCT:
		color = PRODUCT_COLOR;
		break;
	case GNC_PERSONNEL:
		color = PERSONNEL_COLOR;
		break;

	//DRT 4/30/2008 - PLID 29771 - Added color here for EMRItemAdv... backgrounds, since the code is duplicated
	//	for regular windows and popup windows.
	case GNC_EMR_ITEM_BG:
		color = EMR_ITEM_BG_COLOR;
		break;

	// (a.walling 2009-08-13 09:39) - PLID 35136 - Move toolbar colors to global GetNxColor
	case GNC_TOOLBAR:
		{
			switch (nIndex) {
			case 1: // top
				color = TOOLBAR_TOP_COLOR;
				break;
			case 2: // Bottom
				color = TOOLBAR_BOTTOM_COLOR;
				break;
			case 0: // Average color
			default:
				color = TOOLBAR_AVG_COLOR;
				break;
			}
		}
		break;

	// (a.walling 2009-08-13 09:39) - PLID 35136 - Move MDI client background color to global GetNxColor
	case GNC_MDICLIENT:
		color = MDICLIENT_AVG_COLOR;
		break;
	}

	return PaletteColor(color);
}

// Some useful functions for bitmap display

/*
//BVB - got rid of this
// This is like CreateHalftonePalette except if the hLoadedBmp 
// is 256 colors or less, it creates the palette of the bitmap
BOOL CreatePaletteFromBitmap( OUT CPalette &palOut, HBITMAP hLoadedBmp)
{
	// The only way this is supposed to be able to fail is if hLoadedBmp is NULL.  We all 
	// know that's not the only possible way of failing but I think we're fairly safe, and 
	// this code came from MSDN so I'll blame them if something goes wrong.
	if (hLoadedBmp == NULL) {
		return FALSE;
	}

	// Get the color depth of the DIBSection
	BITMAP bm;
	GetObject(hLoadedBmp, sizeof(BITMAP), &bm);

	// If the DIBSection is 256 color or less, it has a color table
	if ((bm.bmBitsPixel * bm.bmPlanes) <= 8) {
		HBITMAP       hOldBitmap;
		RGBQUAD       rgb[256];
		LPLOGPALETTE  pLogPal;

		// Create a memory DC and select the DIBSection into it
		CDC dcMem;
		dcMem.CreateCompatibleDC(NULL);
		hOldBitmap = (HBITMAP)dcMem.SelectObject(hLoadedBmp );

		// Get the DIBSection's color table
		UINT nCount = GetDIBColorTable( dcMem.m_hDC, 0, 256, rgb );

		// Create a palette from the color table
		pLogPal = (LOGPALETTE *)malloc( sizeof(LOGPALETTE) + (nCount*sizeof(PALETTEENTRY)) );
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = nCount;
		for (UINT i=0; i<nCount; i++) {
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		// Create the palette based on the logpalette
		palOut.CreatePalette(pLogPal);
		// Clean up
		free( pLogPal );
		// Done with the bitmap
		dcMem.SelectObject(hOldBitmap);
	} else {
		// It has no color table, so use a halftone palette
		// Get the screen device context
		HDC    hRefDC;
		hRefDC = GetDC(NULL);

		// Create a halftone palette
		HPALETTE hPalette = CreateHalftonePalette(hRefDC);
		palOut.Attach(hPalette);
		
		// Done with the screen dc
		ReleaseDC(NULL, hRefDC);
	}

	return TRUE;
}
*/

// This draws the given bitmap on the given device context at the given spot.  
// NOTE: If cx or cy is given and they are not equal to the bitmap's actual 
// size, then the bitmap will be stretched automatically (which is slower)
BOOL DrawBitmap(CDC *pdc, HBITMAP hBitmap, long x, long y, long cx /*= -1*/, long cy /*= -1*/)
{
	CDC dcMem;
	HGDIOBJ oldBitmap;
	BITMAP bm;

	// Create a memory device context to store the bitmap (and select the bitmap into it)
	dcMem.CreateCompatibleDC(pdc);
	oldBitmap = dcMem.SelectObject(hBitmap);

	// Get the bitmap info
	GetObject(hBitmap, sizeof(BITMAP), &bm);

	if (cx == -1) 
		cx = bm.bmWidth;
	if (cy == -1) 
		cy = bm.bmHeight;

	// Decide whether to blit or stretch-blit the bitmap, and then do it
	if (cx != bm.bmWidth || cy != bm.bmHeight) 
	{	pdc->SetStretchBltMode(HALFTONE);
		pdc->StretchBlt(x, y, cx, cy, &dcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	} 
	else pdc->BitBlt(x, y, cx, cy, &dcMem, 0, 0, SRCCOPY);

	// We're done with our device contexts
	dcMem.SelectObject(oldBitmap);

	return TRUE;
}

// ZM - 3/23/06 - PLID 19251 - This function is in GlobalUtils instead of GlobalDrawingUtils 
// to minimize dependencies for other programs that use GlobalDrawingUtils since an xImage 
// is used in this function.
CRect DrawDIBitmapInRectForPrinting(CDC *pDC, CRect rcImage, HBITMAP hImage)
{
	CBorrowDeviceContext bdc(pDC);

	CRect rImageActual = rcImage;
	BITMAP tmpBmp;
	GetObject(hImage, sizeof(tmpBmp), &tmpBmp);
	HBITMAP hImageActual;

	//Check whether this will work with our device.
	CxImage image;
	image.CreateFromHBITMAP(hImage);
	hImageActual = image.MakeBitmap(pDC->GetSafeHdc());
	GetObject(hImageActual, sizeof(tmpBmp), &tmpBmp);

	if ((float)(tmpBmp.bmWidth) / (float)rcImage.Width() > (float)tmpBmp.bmHeight / (float)rcImage.Height())
	{
		float fScale = (float)rcImage.Width() / (float)tmpBmp.bmWidth;
		float fSImageHeight = (float)tmpBmp.bmHeight * fScale;
		rImageActual.left = rcImage.left;
		rImageActual.right = rcImage.right;
		rImageActual.top = rcImage.top + (rcImage.Height() - (long)fSImageHeight) / 2;
		rImageActual.bottom = rcImage.bottom - (rcImage.Height() - (long)fSImageHeight) / 2;
	}
	else
	{
		float fScale = (float)rcImage.Height() / (float)tmpBmp.bmHeight;
		float fSImageWidth = (float)tmpBmp.bmWidth * fScale;
		rImageActual.top = rcImage.top;
		rImageActual.bottom = rcImage.bottom;
		rImageActual.left = rcImage.left + (rcImage.Width() - (long)fSImageWidth) / 2;
		rImageActual.right = rcImage.right - (rcImage.Width() - (long)fSImageWidth) / 2;
	}

	CRect rSource(0,0,tmpBmp.bmWidth, tmpBmp.bmHeight);
	
	//Now, we need to convert to DIB.
	BITMAPINFO* pbmi;
	pbmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = tmpBmp.bmWidth;
	bmih.biHeight = tmpBmp.bmHeight;
	bmih.biPlanes = 1;
	bmih.biBitCount = tmpBmp.bmBitsPixel;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
	pbmi->bmiHeader = bmih;

	//Call once to fill in BITMAPINFO.
	::GetDIBits(pDC->GetSafeHdc(), hImageActual, 0, tmpBmp.bmHeight, 0, pbmi, DIB_RGB_COLORS);

	//Now call to actually fill array.
	BYTE *arBits = new BYTE[pbmi->bmiHeader.biSizeImage];	
	int r = ::GetDIBits(pDC->GetSafeHdc(), hImageActual, 0, tmpBmp.bmHeight, arBits, pbmi, DIB_RGB_COLORS);

	HBITMAP hImageNew = CreateDIBitmap(pDC->GetSafeHdc(), &bmih, CBM_INIT, arBits, pbmi, DIB_RGB_COLORS);

	CDC bmpDC;
	bmpDC.CreateCompatibleDC(pDC);
	HGDIOBJ oldBitmap = bmpDC.SelectObject(hImageNew);
	pDC->SetStretchBltMode(HALFTONE);
	pDC->StretchBlt (rImageActual.left, 
				rImageActual.top, 
				rImageActual.right - rImageActual.left,
				rImageActual.bottom - rImageActual.top, 
				&bmpDC, 
				rSource.left, 
				rSource.top, 
				rSource.right - rSource.left, 
				rSource.bottom - rSource.top, 
				SRCCOPY);
	bmpDC.SelectObject(oldBitmap);
	bmpDC.DeleteDC();
	
	delete [] arBits;

	// (c.haag 2006-04-26 16:44) - PLID 20303 - Delete image information we created
	free(pbmi);
	DeleteObject(hImageActual);
	DeleteObject(hImageNew);

	return rImageActual;
}

bool CompareFields(COleVariant Field1, COleVariant Field2)
{
	double nTmp1;
	double nTmp2;
	CString strTmp1, strTmp2;

	//TS:  This function compares two variants and returns true if they are the same, false if they are not.

	//If either one is empty, then return false.
	if( Field1.vt == VT_EMPTY || Field2.vt == VT_EMPTY) return false;

	//If both fields are null, return true
	if( Field1.vt == VT_NULL && Field2.vt == VT_NULL) return true;

	//We can now assume that they are not both null (we would have returned) therefore if one of them is null, return false
	if( Field1.vt == VT_NULL || Field2.vt == VT_NULL) return false;

	if( Field1.vt == VT_BSTR && Field2.vt == VT_BSTR){
		strTmp1 = CString(Field1.bstrVal);
		strTmp2 = CString(Field2.bstrVal);
		return ( strTmp1 == strTmp2);
	}

	if( Field1.vt == VT_BSTR){
		if(Field2.vt == VT_DATE) {
			Field1.vt = VT_DATE;
			return (Field1.date == Field2.date);
		}
		if(Field2.vt == VT_CY){
			(COleCurrency) Field2;
			Field1.vt = VT_CY;
			(COleCurrency) Field1;
			return (Field1 == Field2)?true:false;
		}
		//Field2 is a number
		strTmp1 = CString(Field1.bstrVal);
		if (strTmp1.GetAt(0) != 45 && (strTmp1.GetAt(0) < 48 || strTmp1.GetAt(0) > 57) ) return false;
		else {
			nTmp1 = atoi(strTmp1);
			return (Field2.dblVal == nTmp1);
		}
	}
	
	if( Field2.vt == VT_BSTR){
		if(Field1.vt == VT_DATE) {
			Field2.vt = VT_DATE;
			return (Field1.date == Field2.date);
		}
		if(Field1.vt == VT_CY){
			(COleCurrency) Field1;
			Field2.vt = VT_CY;
			(COleCurrency) Field2;
			return (Field1 == Field1)?true:false;
		}
		//Field1 is a number
		strTmp2 = CString(Field2.bstrVal);
		if (strTmp2.GetAt(0) != 45 && (strTmp2.GetAt(0) < 48 || strTmp2.GetAt(0) > 57) ) return false;
		else {
			nTmp2 = atoi(strTmp2);
			return (Field1.dblVal == nTmp2);
		}
	}

	if( Field1.vt == VT_DATE || Field2.vt == VT_DATE) return (Field1.date == Field2.date); //if the other is not a date, then this will be false, which is fine.

	if(Field1.vt == VT_CY || Field2.vt == VT_CY) return (Field1 == Field2)?true:false;

	//Both of them are not empty, null, string, date, or currency; therefore they are both numbers.

	return (Field1.dblVal == Field2.dblVal);
}

bool CopyFileModifiedTime(LPCTSTR strSourcePathName, LPCTSTR strDestPathName)
{
	COleDateTime dt;
	if (GetFileSystemTime(strSourcePathName, dt) && SetFileSystemTime(strDestPathName, dt)) {
		return true;
	} else {
		return false;
	}
}

// (c.haag 2007-05-07 17:37) - Global nest level tracking for the
// CFuncCallMonitor function
static int g_nFuncCallMonitorNestLevel = 0;
// (c.haag 2007-05-07 17:29) - This class will trace and optionally
// log on a SQL server when function calls begin and end.
CFuncCallMonitor::CFuncCallMonitor(const CString& strFuncLabel, BOOL bLogToSQL /*= FALSE */, ADODB::_ConnectionPtr pCon /*= NULL */,
								   BOOL bIncludeThreadID /*= FALSE */)
{
	// (c.haag 2007-05-08 08:51) - You should always remove all of your instances of the CFuncCallMonitor
	// class before finishing checking in your code. Just in case someone does not, the logic is protected
	// so that it won't do anything to a client's machine.
#ifdef _DEBUG
	try {
		m_nStartTime = GetTickCount();
		m_strFuncLabel = strFuncLabel;
		m_bLogToSQL = bLogToSQL;
		m_bIncludeThreadID = bIncludeThreadID;
		m_pCon = pCon;
		if (NULL == m_pCon && bLogToSQL) {
			m_pCon = GetRemoteData();
		}
		for (int i=0; i < g_nFuncCallMonitorNestLevel; i++) {
			m_strFuncLabel = CString("   ") + m_strFuncLabel;
		}
		CString strOut = FormatString("%s BEGIN", m_strFuncLabel);
		if (m_bIncludeThreadID) { strOut += FormatString(" (tid=0x%04x)", GetCurrentThreadId()); }
		TRACE(strOut + "\n");
		if (m_bLogToSQL) {
			if (NULL != m_pCon) {
				m_pCon->Execute( _bstr_t( CString("-- ") + _Q(strOut) ), NULL, adCmdText);
			}
		}
		g_nFuncCallMonitorNestLevel++;
	}
	NxCatchAll("Error in CFuncCallMonitor");
#endif
}

CFuncCallMonitor::~CFuncCallMonitor()
{
	// (c.haag 2007-05-08 08:51) - You should always remove all of your instances of the CFuncCallMonitor
	// class before finishing checking in your code. Just in case someone does not, the logic is protected
	// so that it won't do anything to a client's machine.
#ifdef _DEBUG
	try {
		g_nFuncCallMonitorNestLevel--;
		CString strOut = FormatString("%s END (Duration=%dms)", m_strFuncLabel, GetTickCount()-m_nStartTime);
		if (m_bIncludeThreadID) { strOut += FormatString(" (tid=0x%04x)", GetCurrentThreadId()); }
		TRACE(strOut + "\n");
		if (m_bLogToSQL) {
			if (NULL != m_pCon) m_pCon->Execute( _bstr_t( CString("-- ") + _Q(strOut) ), NULL, adCmdText);
		}		
	}
	NxCatchAll("Error in ~CFuncCallMonitor");
#endif
}

bool GetSystemPath(OUT CString &strSystemPath)
{
	CString strPath;
	UINT nLen = GetSystemDirectory(strPath.GetBuffer(MAX_PATH), MAX_PATH);
	strPath.ReleaseBuffer();
	if (nLen == 0 || nLen == MAX_PATH) {
		// Either case we didn't get the correct full string
		return false;
	} else {
		strSystemPath = strPath;
		return true;
	}
}

bool GetPalmPath(OUT CString &strPalmPath)
{
	// Todo: First check the registry

	// If it wasn't found in the registry, try C:\Palm
	if (DoesExist("C:\\Palm")) {
		strPalmPath = "C:\\Palm";
		return true;
	}

	// If it wasn't found in the registry, try C:\Pilot
	if (DoesExist("C:\\Pilot")) {
		strPalmPath = "C:\\Pilot";
		return true;
	}

	// It wasn't found
	return false;
}

bool GetModuleFilePathName(IN const HMODULE hModule, OUT CString &strModuleFilePathName)
{
	CString strPathName;
	DWORD nLen = GetModuleFileName(hModule, strPathName.GetBuffer(MAX_PATH), MAX_PATH);
	strPathName.ReleaseBuffer();
	if (nLen) {
		strModuleFilePathName = strPathName;
		return true;
	} else {
		return false;
	}
}

bool GetDllFilePathName(IN const CString &strDllFileName, OUT CString &strDllFilePathName)
{
	if (!strDllFileName.IsEmpty()) {
		// Get a handle to the ocx/dll
		HMODULE hModule = GetModuleHandle(strDllFileName);
		if (hModule) {
			// The module is in memory already, just return the path
			return GetModuleFilePathName(hModule, strDllFilePathName);
		} else {
			// Not a dll or not currently loaded in memory, so try to load it 
			// just so we can see what WOULD be loaded once it's attempted

			// (z.manning, 07/06/2006) - PLID 21334 - We don't need to fully load the dll since all we want is its name.
			HMODULE hLoaded = LoadLibraryEx(strDllFileName, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hLoaded) {
				// Successfully loaded the dll or whatever, now get its path
				bool bAns = GetModuleFilePathName(hLoaded, strDllFilePathName);
				// And then unload it
				FreeLibrary(hLoaded);
				// And return our success value
				return bAns;
			} else {
				// Could not load it, it must not be a dll
				return false;
			}
		}
	} else {
		// No file name given
		return false;
	}
}



bool SetPalmRecordStatus(CString const strTableName, const unsigned long nID, 
	 const unsigned long nStatus) 
{
	CString strTable(strTableName);
	CString strSQL;
	long nIDToAdd = nID;
	long nStatusToAdd = nStatus;

	if (strTableName.IsEmpty())
		return false;

	try{
		EnsureRemoteData();
		//JJ - 3-16-01 - this tried to update PersonT where PersonT.PersonID.. etc. bad Stydie!
		if(strTableName!="PersonT") {
			strSQL.Format("UPDATE [%s] SET [%s].Status = %li WHERE [%s].PersonID = %li", strTable, strTable, nStatusToAdd, strTable, nIDToAdd);
			ExecuteSql(strSQL);
		}
		// TODO: remove this once the palm conduit works properly
		//strSQL.Format("UPDATE [PersonT] SET [PersonT].Status = %li WHERE [PersonT].ID = %li", nStatusToAdd, nIDToAdd);
		//ExecuteSql(strSQL);
	} NxCatchAll("Error in SetPalmRecordStatus");

	// if it works
	return true;

}


bool SetPalmRecordID(CString const strTableName, const unsigned long nID, 
	 const long nRecordID) 
{
	CString strTable(strTableName);
	CString strSQL;
	long nIDToAdd = nID;
	long nRecIDToAdd = nRecordID;

	if (strTableName.IsEmpty())
		return false;

	try { 
		EnsureRemoteData();
		strSQL.Format("UPDATE [%s] SET [%s].RecordID = %li WHERE [%s].PersonID = %li", strTable, strTable, nRecIDToAdd, strTable, nIDToAdd);
		ExecuteSql(strSQL);
	} NxCatchAll("Error in SetPalmRecordID");

	// if it works
	return true;

}









/*************************************************************************
*    Class:  None                                                                  
*                                                                         
*    Method: GetPalmKey()                                                                    
*                                                                        
*    Description: Gets the palm key from the registry                                          
*                                                                        
*    Parameters: None                                                                                       
*                                                                       
*    Return values: Returns the key                                                      
*                                                                        
*    Revision History:                                                                    
*    -----------------------------------------------------                                                                    
*    Last Revision: 5/19/2000 CJS                                                                    
**************************************************************************/
HKEY GetPalmKey() {

	// init variables
	HKEY hSoftKey = NULL;

	// First open the registry
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\U.S. Robotics\\Pilot Desktop\\Core"), 0, KEY_WRITE|KEY_READ,
		&hSoftKey) != ERROR_SUCCESS) {
		return hSoftKey;		
	}

	// Return the open key
	return hSoftKey;
}


/*************************************************************************
*    Class:  None                                                                  
*                                                                         
*    Method: GetPalmProfileString()                                                                    
*                                                                        
*    Description: Gets a registry key corresponding to the string you send it                                                
*                                                                        
*    Parameters: strValueName: the name of the value you want
*                     strValue:         what to put the new key into
*                                                                                                   
*                                                                        
*    Return values: returns true if good, false if bad                                         
*                                                                        
*    Revision History:                                                                    
*    -----------------------------------------------------                                                                    
*    Last Revision: 5/19/2000 CJS                                                                    
**************************************************************************/

bool GetPalmProfileString(const CString &strValueName, CString &strValue)
{
	// Get the key
	HKEY hPalm = GetPalmKey();
	long nResult = NULL;
	
	// Try to get the data from the value
	if (hPalm) {
		unsigned long nType = REG_NONE;
		unsigned long nSize = MAX_PATH;
		nResult = RegQueryValueEx(hPalm, strValueName, NULL, &nType, (LPBYTE)strValue.GetBuffer(MAX_PATH), &nSize);
		strValue.ReleaseBuffer();
		if (nResult == ERROR_MORE_DATA) {
			// If our buffer wasn't big enough, use the right sized buffer
			nResult = RegQueryValueEx(hPalm, strValueName, NULL, &nType, (LPBYTE)strValue.GetBuffer(nSize), &nSize);
			strValue.ReleaseBuffer();
		}
		RegCloseKey(hPalm);
	}

	// Return success value
	if (nResult == ERROR_SUCCESS) {
		return true;
	} else {
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
#define INVALID_PATH _T("{????? No Path Specified ?????}")
static CString l_strSharedPath;
static bool l_bSharedPathSet = false;

const CString &GetSharedPath(void)
{

	//BVB
	if (!l_bSharedPathSet) {
		//Try to get it from the registry
		l_strSharedPath = NxRegUtils::ReadString(GetRegistryBase() + "SharedPath");

		if (l_strSharedPath.IsEmpty())
		{	
			l_strSharedPath = GetPracPath(PracPath::PracticePath);		
		}

		// Got the path
		l_bSharedPathSet = true;
	}

	return l_strSharedPath;
}

CString GetSharedPath(const CString& strDatabase)
{
	CString strSharedPath;

	strSharedPath = NxRegUtils::ReadString(GetRegistryBase(strDatabase) + "SharedPath");
	if (strSharedPath.GetLength() && strSharedPath.GetAt(strSharedPath.GetLength() - 1) != '\\')
		strSharedPath += "\\";

	return strSharedPath;
}

const CString &RecalcSharedPath()
{
	l_bSharedPathSet = false;
	return GetSharedPath();
}

void SetSharedPath(const CString &strNewSharedPath)
{
	l_strSharedPath = strNewSharedPath;
	l_bSharedPathSet = true;
}
///////////////////////////////////////////////////////////////////////////////////////

LPUNKNOWN GetDlgItemUnknown(CWnd *pDlg, IN int nIdDlgItem)
{
	CWnd *pCtrl = pDlg->GetDlgItem(nIdDlgItem);
	if (pCtrl) {
		// TODO: Step into this function and do it yourself instead of calling the function
		return pCtrl->GetControlUnknown();
	}

	// If we made it to here, we failed
	return NULL;
}

// (a.walling 2010-04-30 17:13) - PLID 38553 - Part of GlobalDataUtils
//CString ConvertToQuotableString(LPCTSTR lpstrString, bool bDoubleQuote)
//{
//	CString strAns = lpstrString;
//	if (bDoubleQuote) {
//		strAns.Replace("\"", "\"\"");
//		//TES 12/22/2004 - You can't tell, but these are the left- and right-facing double quote characters.
//		strAns.Replace("”","””");
//		strAns.Replace("“","““");
//	} else {
//		strAns.Replace("'", "''");
//	}
//	return strAns;
//}


// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString ConvertToQuotableXMLString(LPCTSTR lpstrString)
//{
//	CString strAns = lpstrString;
//	strAns.Replace("&","&amp;");
//	strAns.Replace("<","&lt;");
//	strAns.Replace(">","&gt;");
//	strAns.Replace("'","&apos;");
//	strAns.Replace("\"","&quot;");
//	return strAns;
//}

// (a.walling 2012-07-03 11:17) - PLID 51263 - Moved ConvertToControlText / ConvertFromControlText to globaldatautils

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString ConvertToHTMLEmbeddable(const CString &str)
//{
//	CString strAns = str;
//	strAns.Replace("&", "&amp;");
//	strAns.Replace("<", "&lt;");
//	strAns.Replace(">", "&gt;");
//	// (a.walling 2010-01-08 09:56) - PLID 36814 - Should also escape quotes
//	// (a.walling 2010-01-08 10:00) - PLID 36814 - Hm, turns out &apos; is not officially part of the HTML standard, 
//	// although all browsers support it. Except, of course, IE6 or lower. So the XHTML website recommends &#39;
//	// see http://www.w3.org/TR/xhtml1/#C_16
//	//strAns.Replace("'","&apos;");
//	strAns.Replace("'","&#39;");
//	strAns.Replace("\"","&quot;");
//	// (a.walling 2007-07-12 12:07) - PLID 26640 - Need to replace newlines with <br> tags
//	// (a.walling 2007-10-23 12:49) - PLID 27820 - Replace \r\n with \n so we can then replace with <br>
//	// (a.walling 2007-10-24 09:39) - PLID 27820 - There may be situations we would only be getting a \n
//	// instead of a \r\n. So to be complete, I replace all \r\n with \n, then replace \n with <br/>, so
//	// as not to end up with any \r<br/> sequences.
//	strAns.Replace("\r\n", "\n");
//	strAns.Replace("\n", "<br/>");
//	return strAns;
//}

// Simply returns the name of the known 
// table (returns "" if the table is unknown)
CString GetPersonTableName(UINT nPersonType)
{
	switch (nPersonType) {
	
	// Simply return the name of the known table
	case DP_PATIENTST:			return "PatientsT";
	case DP_USERST:				return "UsersT";
	case DP_CONTACTST:			return "ContactsT";
	case DP_PROVIDERST:			return "ProvidersT";
	case DP_REFPHYST:			return "ReferringPhysT";
	case DP_SUPPLIERT:			return "SupplierT";
	case DP_INSURANCECOT:		return "InsuranceCoT";
	case DP_INSUREDPARTYT:		return "InsuredPartyT";
	case DP_REFERRALSOURCET:	return "ReferralSourceT";
	
	// The given flag is unknown so return failure
	default:					return "";
	}
}

BOOL IsPersonInTable(long nPersonID, UINT nPersonType)
{
	// Create a recordset that simply gives the count of 
	// records in the appropriate table with the specified ID
	_RecordsetPtr rs;
	CString strTableName = GetPersonTableName(nPersonType);
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	rs = CreateParamRecordset("SELECT Count(*) As NumRecords "
		"FROM {CONST_STR} WHERE PersonID = {INT};", 
		strTableName, nPersonID);
	
	// Now handle the different possibilities
	long nRecCount = AdoFldLong(rs, "NumRecords");
	rs->Close();
	if (nRecCount == 0) {
		// There are no records in the given table with this patient ID
		return FALSE;
	} else if (nRecCount == 1) {
		// There is exactly one record in the given table with this patient ID
		return TRUE;
	} else {
		// Why the heck would there be anything but 0 or 1?  This must 
		// represent something we didn't expect (i.e. an "exception")
		AfxThrowNxException(
			"Unexpected record count in table %s for person ID %li", 
			strTableName, nPersonID);
		// This code is never executed
		return TRUE;
	}
}

// THIS MACRO IS ONLY FOR USE IN THE FOLLOWING FUNCTION (DeletePerson)
// If person_type is not in nFlags, ExecuteSql will not be called
#define LOCAL_DELETE_PERSON(person_type) \
	if (nFlags & person_type) \
		ExecuteSql("DELETE FROM %s WHERE PersonID = %li", \
		GetPersonTableName(person_type), nPersonID);

// This function deletes the records for the given person 
// ID from each of the tables specified in nFlags.  It
// returns true if the person is completely gone (i.e. got 
// deleted from PersonT)
// Throws _com_error, CNxException
bool DeletePerson(long nPersonID, UINT nFlags)
{
	// Try to delete this person ID from each table (only 
	// actually try to delete if the parameter is in nFlags)
	LOCAL_DELETE_PERSON(DP_PATIENTST);
	LOCAL_DELETE_PERSON(DP_USERST);
	LOCAL_DELETE_PERSON(DP_CONTACTST);
	LOCAL_DELETE_PERSON(DP_PROVIDERST);
	LOCAL_DELETE_PERSON(DP_REFPHYST);
	LOCAL_DELETE_PERSON(DP_SUPPLIERT);
	LOCAL_DELETE_PERSON(DP_INSURANCECOT);
	LOCAL_DELETE_PERSON(DP_INSUREDPARTYT);
	LOCAL_DELETE_PERSON(DP_REFERRALSOURCET);
	
	// If the person exists in any table except ones specified 
	// by nFlags, don't try to delete the record from PersonT
	if (DoesPersonExist(nPersonID, DP_ALL & ~nFlags)) {
		// The person exists in some table so don't delete
		return false;
	} else {		
		// (a.walling 2010-01-22 14:29) - PLID 37038 - New flag, which keeps the actual PersonT record but as archived
		// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
		// (d.thompson 2012-08-14) - PLID 52165 - Reworked language table structure
		// (j.luckoski 2013-03-04 13:19) - PLID 33548 - Added DisplayRewardsWarning for prompting reward points > 0 for a patient
		if (nFlags & DP_ARCHIVEPATIENT) {
			// (b.spivey, May 21, 2013) - PLID 55937 - Parameterized in my quest to delete multiple patient races. 
			// (s.tullis 2016-05-25 15:41) - NX-100760 -  Update Location to Null
			ExecuteParamSql(
				"BEGIN TRAN "
				"SET NOCOUNT ON "
				"	"
				"UPDATE PersonT "
				"SET Archived = 1, Location = NULL, "
				"First = '', Middle = '', Last = '', Address1 = '', Address2 = '', City = '', State = '', Zip = '', "
				"Gender = 0, Suffix = '', Title = '', "
				"HomePhone = '', PrivHome = 0, WorkPhone = '', PrivWork = 0, Extension = '', "
				"CellPhone = '', OtherPhone = '', Email = '', Pager = '', Fax = '', "
				"BirthDate = NULL, SocialSecurity = '', " // keep FirstContact and InputDate, why not
				"UserID = NULL, WarningMessage = '', DisplayWarning = 0, Note = '', "
				"PalmFlag = 2, Status = 1, RecordID = -1, Company = '', CompanyID = '', "
				"EmergFirst = '', EmergLast = '', EmergHPhone = '', EmergWPhone = '', EmergRelation = '', "
				"Spouse = '', PrivCell = 0, ExcludeFromMailings = 1, DisplayAllergyWarning = 0, DisplayRewardsWarning = 0, "
				"PrefixID = NULL, WarningUseExpireDate = 0, WarningExpireDate = NULL, " // Keep ModifiedDate
				"WarningUserID = NULL, WarningDate = NULL, "
				"PrivEmail = 0, PrivOther = 0, PrivFax = 0, PrivPager = 0, "
				" TextMessage = 0, LanguageID = NULL, Country = NULL, Ethnicity = NULL "
				" WHERE PersonT.ID = {INT} "
				"	"
				"SET NOCOUNT OFF "
				"COMMIT TRAN", nPersonID);
		} else {
			// The person does not exist in any other table so delete
			// (b.spivey, June 10, 2013) - PLID 55937 - Why not? Parameterized this too. 
			ExecuteParamSql("DELETE FROM PersonT WHERE PersonT.ID = {INT}", nPersonID);
		}
		return true;
	}
}

// THIS MACRO IS ONLY FOR USE IN THE FOLLOWING FUNCTION (DoesPersonExist)
// If person_type is not in nFlags, IsPersonInTable will not even be called
#define LOCAL_CHECK_PERSON_EXISTENCE(person_type) \
	if ((nFlags & person_type) && IsPersonInTable(nPersonId, person_type)) return true;

//this function returns whether or not the personID sent to it is used in any other tables besides 
//the PersonT table
bool DoesPersonExist(long nPersonId, UINT nFlags) {


	// Each one of these macros will return false if the person doesn't exist
	LOCAL_CHECK_PERSON_EXISTENCE(DP_PATIENTST);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_USERST);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_CONTACTST);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_PROVIDERST);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_REFPHYST);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_SUPPLIERT);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_INSURANCECOT);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_INSUREDPARTYT);
	LOCAL_CHECK_PERSON_EXISTENCE(DP_REFERRALSOURCET);
	
	// If none of the above macros returned false, then the person 
	// doesn't exist in any of the tables included in nFlags so return 
	// true, the person does exist
	return false;
	
}

CString GetStringOfResource(int nResource)
{
	CString str;
	if(str.LoadString(nResource))
	return str;
	else return "";
}

LPUNKNOWN BindNxTimeCtrl(CWnd *pParent, UINT nID)
{
	// We have a valid connection so get the com control
	_DNxTimePtr pdl = GetDlgItemUnknown(pParent, nID);
	if (pdl) {
		// If we made it here we have success
		return pdl;
	}

	// If we made it here we failed
	return NULL;
}

LPUNKNOWN BindNxDataListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery)
{
	// We have a valid connection so get the com control
	LPUNKNOWN punkControl = GetDlgItemUnknown(pParent, nID);
	NXDATALISTLib::_DNxDataListPtr pdl = punkControl;
	if (pdl) {
		try {
			// If the caller wants to attach it to data
			if (pDataConn) {
				// (a.walling 2009-08-11 14:46) - PLID 35180
				// (a.walling 2010-07-28 13:23) - PLID 39781 - FYI the _com_ptr_t wrapper for _Connection will handle proper identity comparison with pDataConn
				// There are so many places that call this with GetRemoteData that it is easier to simply handle this here for now.
				if (GetRemoteData(acDoNotAffirm) == pDataConn) {
					pDataConn = GetRemoteDataSnapshot();
				}

				// We have a valid com control so init
				pdl->AdoConnection = (LPDISPATCH) pDataConn;
				if (bAutoRequery) {
					pdl->Requery();
				}
			}
			// If we made it here we have success
			return pdl;
		} catch (_com_error e) {
			// Failed!
			HandleException(e, "CNxDialog::BindNxDataListCtrl Error 1", __LINE__, __FILE__);
		}
	} else {
		// There wasn't a valid com object

		// (a.walling 2008-10-13 11:09) - PLID 31658 - This used to work in VC6 if passed a dl or dl2. However now it will
		// fail. We think we have handled all situations, but just in case, there is no point crippling the program when we
		// can safely recover. Therefore, if the QI of the datalist/2 fails, then try the other. We will still throw an
		// exception so we can identify when and where this occurs and fix it properly.
		ASSERT(FALSE);
		CString strWindow;
		if (pParent) {
			pParent->GetWindowText(strWindow);
		}
		HandleException(NULL, FormatString("CNxDialog::BindNxDataListCtrl Error 2 (%li:'%s')", nID, strWindow), __LINE__, __FILE__);
		
		NXDATALIST2Lib::_DNxDataListPtr pdl2 = punkControl;
		if (pdl2) {
			return BindNxDataList2Ctrl(pParent, nID, pDataConn, bAutoRequery);
		}
	}

	// If we made it here we failed
	return NULL;
}

LPUNKNOWN BindNxDataList2Ctrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery)
{
	// We have a valid connection so get the com control
	LPUNKNOWN punkControl = GetDlgItemUnknown(pParent, nID);
	NXDATALIST2Lib::_DNxDataListPtr pdl = punkControl;
	if (pdl) {
		try {
			// If the caller wants to attach it to data
			if (pDataConn) {
				// (a.walling 2009-08-11 14:46) - PLID 35180
				// (a.walling 2010-07-28 13:23) - PLID 39781 - FYI the _com_ptr_t wrapper for _Connection will handle proper identity comparison with pDataConn
				// There are so many places that call this with GetRemoteData that it is easier to simply handle this here for now.
				if (GetRemoteData(acDoNotAffirm) == pDataConn) {
					pDataConn = GetRemoteDataSnapshot();
				}

				// We have a valid com control so init
				pdl->AdoConnection = (LPDISPATCH) pDataConn;
				if (bAutoRequery) {
					pdl->Requery();
				}
			}
			// If we made it here we have success
			return pdl;
		} catch (_com_error e) {
			// Failed!
			HandleException(e, "CNxDialog::BindNxDataList2Ctrl Error 1", __LINE__, __FILE__);
		}
	} else {
		// There wasn't a valid com object
		
		// (a.walling 2008-10-13 11:09) - PLID 31658 - This used to work in VC6 if passed a dl or dl2. However now it will
		// fail. We think we have handled all situations, but just in case, there is no point crippling the program when we
		// can safely recover. Therefore, if the QI of the datalist/2 fails, then try the other. We will still throw an
		// exception so we can identify when and where this occurs and fix it properly.
		ASSERT(FALSE);
		CString strWindow;
		if (pParent) {
			pParent->GetWindowText(strWindow);
		}
		HandleException(NULL, FormatString("CNxDialog::BindNxDataList2Ctrl Error 2 (%li:'%s')", nID, strWindow), __LINE__, __FILE__);

		NXDATALISTLib::_DNxDataListPtr pdl1 = punkControl;
		if (pdl1) {
			return BindNxDataListCtrl(pParent, nID, pDataConn, bAutoRequery);
		}
	}

	// If we made it here we failed
	return NULL;
}


// (a.walling 2012-03-12 17:33) - PLID 48839 - Moved COleCurrency operators * and / for doubles into GlobalDataUtils

/**************************************************************************
* GetFourDigitDecimalPartAsLong
*	This retrieves the first four digits of the decimal portion of the 
*   double value, applying financial rounding, and returns those four 
*   digits as a long.
*
*   dbl: any double value (remember it will be rounding to at most four 
*	     places after the decimal
*
*/
// TODO: I HATE that this function has to exist.  We need to change our tax rates to be stored as currency!
//LONG GetFourDigitDecimalPartAsLong(double dbl)
//{
//	CString str;
//	str.Format("%.4f", dbl);
//	long nDecPos = str.Find(".");
//	ASSERT(nDecPos >= 0);
//	if (nDecPos >= 0) {
//		return atol(str.Mid(nDecPos+1));
//	} else {
//		return 0;
//	}
//}
//
////Multiplies COleCurrency values -BVB
//COleCurrency operator * (COleCurrency a, double b)
//{	//returns a * b
//
//	if (a.GetStatus() == COleCurrency::valid) {
//		// 1. multiply the left part of the double (the integer portion) by the currency value
//		// 2. multiply the right part of the double (exactly four digits of the fractional 
//		//    portion) by the currency value and divide by a thousand
//		// 3. add these two currency amounts together
//		// (a.walling 2007-11-06 16:43) - PLID 27974 - VS2008 - Need to explicitly state the type of the denominator
//		a = (a * (long)b) + ((a * GetFourDigitDecimalPartAsLong(b)) / long(10000));
//	}
//
//	return a;
//}
//
//COleCurrency operator * (double a, COleCurrency b)
//{	//returns a * b
//	return b * a;
//}
//
////*
////Divides COleCurrency values -JJ
//COleCurrency operator / (COleCurrency a, double b)
//{	//returns a / b
//
//	if (a.GetStatus() == COleCurrency::valid)
//		a.m_cur.int64 = (_int64)(double)(a.m_cur.int64 / b);
//
//	return a;
//}
//
//COleCurrency operator / (double a, COleCurrency b)
//{	//returns a / b
//
//	return b / a;
//}

//*/

// Takes a double and returns a long; just like the (long) cast, except this rounds instead of truncating.
//long RoundAsLong(double dbl)
//{
//	ASSERT(dbl > LONG_MIN && dbl < LONG_MAX);
//	if (dbl >= 0) {
//		return (long)(dbl + 0.5);
//	} else {
//		return (long)(dbl - 0.5);
//	}
//}

////////////////////////////////////////
//TS:  This is a semi-ugly way of calling this function, which has nothing to do with Practice per se,
//	   but needs to use the ReportInfo object to loop through all the reports.  This function, if called,
//     is called in the InitInstance for the program, and it returns false immediately after.
//DT:  Added a parameter for the report id so that you only have to check out 1 file instead of 2 every time.
void CreateAllTtxFiles(long nID){
	/*for(int i = 1; i <125; i++){
		CReports::gcs_aryKnownReports[i].CreateTtxFile();
	}*/
	if(nID > 0){
		// Get the report from the master list
		const CReportInfo *pRep = NULL;
		for (long i=0; i<CReports::gcs_nKnownReportCount; i++) {
			if (CReports::gcs_aryKnownReports[i].nID == nID) {
				pRep = &CReports::gcs_aryKnownReports[i];
				break;
			}
		}
		if (pRep) {
			pRep->CreateTtxFile();
		}
	}
	else{
		if(nID == -1){
			//TS:  Code to generate a list of all the currently required .rpt files.
			//TODO: This does not work with the new date options stuff.  If anyone ever needs this 
			//functionality again, feel free to fix it.
			/*CString strRptList, strNextRpt;
			strRptList = "Required .rpt files:";
			for(int i = 0; i < CReports::gcs_nKnownReportCount; i++){
				if(CReports::gcs_aryKnownReports[i].nDetail != 0){//Detailed/Summary
					if(CReports::gcs_aryKnownReports[i].nDateFilter != 0){//Service/Input
						strNextRpt.Format("%sInputDtld.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
						strNextRpt.Format("%sInputSmry.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
						strNextRpt.Format("%sServiceDtld.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
						strNextRpt.Format("%sServiceSmry.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
					}
					else{//No Service/Input
						strNextRpt.Format("%sDtld.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
						strNextRpt.Format("%sSmry.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
					}
				}
				else{//No Detailed/Summary
					if(CReports::gcs_aryKnownReports[i].nDateFilter != 0){//Service/Input
						strNextRpt.Format("%sInput.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
						strNextRpt.Format("%sService.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
					}
					else{//No Service/Input
						strNextRpt.Format("%s.rpt", CReports::gcs_aryKnownReports[i].strReportFile);
						strRptList += "\r\n" + strNextRpt;
					}
				}
			}
			Log(strRptList);*/
		}
	}		
}



//this is an overload of the above function which takes an ID of the report and a Path to save the Ttx File to
BOOL CreateAllTtxFiles(long nID, CString strFilePath, long nCustomReportID/*= -1*/) {
	
	const CReportInfo pRep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nID)]);
	// (r.gonet 10/16/2011) - PLID 45967 - Some reports, currently only custom lab requests, can verify per custom report. Sorry const!
	const_cast<CReportInfo&>(pRep).nDefaultCustomReport = nCustomReportID;
	
				
	if (pRep.CreateTtxFile(strFilePath)) {
				return TRUE;
		}
		
	
	//if we got here, we failed
	return FALSE;
}


void BuildIDList(_DNxDataListPtr m_pBatchList, CString& strIDList){
	
	CString str;
	long nCount = m_pBatchList->GetRowCount();
	for (long i=0; i<nCount; i++) {
		str.Format("%li,", VarLong(m_pBatchList->Value[i][0]));
		strIDList += str;
	}
	
	
}

// (r.galicki 2008-11-05 12:07) - PLID 27214 - BuildIDList for datalist2, added column specifier
void BuildIDList(NXDATALIST2Lib::_DNxDataListPtr m_pBatchList, CString& strIDList, short nCol){
	
	CString str;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBatchList->GetFirstRow();
	while(pRow) {
		str.Format("%li,", VarLong(pRow->GetValue(nCol)));
		strIDList += str;
		pRow = pRow->GetNextRow();
	}
	
	strIDList.TrimRight(',');
}

// Tries to delete the given file, and if the attempt 
// fails and the file exists, it is queued for deletion later
// If the mainframe window is unavailable, calls MoveFileAtStartup
// Return value indicates whether the file exists upon completion
BOOL DeleteFileWhenPossible(LPCTSTR strFilePathName)
{
	CMainFrame *pMainFrame = GetMainFrame();
	if (pMainFrame) {
		// We got a pointer to the mainframe window 
		// so call it's version of this function
		return pMainFrame->DeleteFileWhenPossible(strFilePathName);
	} else {
		// We couldn't get the mainframe window so we just MoveFileAtStartup
		MoveFileAtStartup(strFilePathName, NULL);
		return DoesExist(strFilePathName);
	}
}

//does not attempt to delete a file until nDelayInMS has elapsed
BOOL DeleteFileWhenPossible(LPCTSTR strFilePathName, long nDelayInMS)
{
	CMainFrame *pMainFrame = GetMainFrame();
	if (pMainFrame) {
		// We got a pointer to the mainframe window 
		// so call it's version of this function
		return pMainFrame->DeleteFileWhenPossible(strFilePathName, nDelayInMS);
	}
	else {
		// We couldn't get the mainframe window so we just MoveFileAtStartup
		MoveFileAtStartup(strFilePathName, NULL);
		return DoesExist(strFilePathName);
	}
}

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

void DeleteProcInfo(long nProcInfoID)
{
	// (a.walling 2008-07-07 18:00) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	_RecordsetPtr rsLadderID = CreateRecordset("SELECT ID, PersonID FROM LaddersT WHERE ProcInfoID = %li", nProcInfoID);
	long nLadderID = rsLadderID->eof ? -1 : AdoFldLong(rsLadderID, "ID");
	long nPersonID = rsLadderID->eof ? -1 : AdoFldLong(rsLadderID, "PersonID");
	rsLadderID->Close();

	if(nLadderID != -1) {
		DeleteLadder(nLadderID, nPersonID); // (a.walling 2008-07-07 18:00) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	}
	else {
		//m.hancock - 5/4/2006 - PLID 20308 - Refresh the ProcInfo record
		//TES 8/12/2014 - PLID 63224 - Removed this tablechecker (this was the only place that fired it).
		//CClient::RefreshTable(NetUtils::ProcInfoT, nProcInfoID);
	}

	//(e.lally 2007-05-17) PLID 26049 - Combine statements into one batch.
	CString strSqlBatch = BeginSqlBatch();
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nProcInfoID INT\r\n");
	AddStatementToSqlBatch(strSqlBatch, "SET @nProcInfoID = %li \r\n", nProcInfoID);

	AddStatementToSqlBatch(strSqlBatch, "UPDATE PicT SET ProcInfoID = NULL WHERE ProcInfoID = @nProcInfoID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoDetailsT WHERE ProcInfoID = @nProcInfoID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoAppointmentsT WHERE ProcInfoID = @nProcInfoID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoMedicationsT WHERE ProcInfoID = @nProcInfoID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoQuotesT WHERE ProcInfoID = @nProcInfoID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoPaymentsT WHERE ProcInfoID = @nProcInfoID");	
	// (j.dinatale 2012-07-11 09:55) - PLID 51474 - handle ProcInfoBillsT when deleting from ProcInfoT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoBillsT WHERE ProcInfoID = @nProcInfoID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProcInfoT WHERE ID = @nProcInfoID");
	ExecuteSqlBatch(strSqlBatch);
}

// (a.walling 2008-07-07 18:01) - PLID 29900 - Required patientID
void DeleteLadder(long nLadderID, long nPatID)
{
	PhaseTracking::UnapplyEvent(nPatID, PhaseTracking::ET_LadderCreated, nLadderID, -1);
	//(e.lally 2007-05-17) PLID 26049 - Combine statements into one batch.
	CString strSqlBatch = BeginSqlBatch();
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nLadderID INT\r\n");
	AddStatementToSqlBatch(strSqlBatch, "SET @nLadderID = %li \r\n", nLadderID);

	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EventAppliesT WHERE StepID IN (SELECT ID FROM StepsT WHERE LadderID = @nLadderID)");
	// (c.haag 2008-06-10 17:45) - PLID 30328 - Also need to delete from TodoAssignToT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM ToDoList WHERE RegardingType = %i AND RegardingID IN (SELECT ID FROM StepsT WHERE LadderID = @nLadderID))", ttTrackingStep);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ToDoList WHERE RegardingType = %i AND RegardingID IN (SELECT ID FROM StepsT WHERE LadderID = @nLadderID)", ttTrackingStep);
	// (j.jones 2008-11-26 14:27) - PLID 30830 - delete from StepsAssignToT
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM StepsAssignToT WHERE StepID IN (SELECT ID FROM StepsT WHERE LadderID = @nLadderID)");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM StepsT WHERE LadderID = @nLadderID");
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LaddersT WHERE ID = @nLadderID");
	ExecuteSqlBatch(strSqlBatch);
	//CClient::RefreshTable(NetUtils::LaddersT, nLadderID);
}

//DRT 3/7/2006 - PLID 19571 - Renamed this because it is only safe for deleting Custom Records.  To delete
//	a NexEMR record, please use DeleteNexEMR.
void DeleteCustomRecord(long EMRID) {
	try {
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM EMRMasterT WHERE EMRGroupID = %li",EMRID);
		while(!rs->eof) {

			long nEMNID = AdoFldLong(rs, "ID");
			DeleteEMN(nEMNID);

			rs->MoveNext();
		}
		rs->Close();

		//prepare for auditing
		rs = CreateRecordset("SELECT PatientID, Description FROM EMRGroupsT WHERE ID = %li", EMRID);
		CString strDescription;
		long nPatientID = -25;
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID");
			strDescription = AdoFldString(rs, "Description","");
		}
		rs->Close();
		if(strDescription == "")
			strDescription = "{No Description}";

		//DRT 3/7/2006 - PLID 19582 - Since this function is now for Custom Records only, we can 
		//	fix a bug that was leaving blank PicT records.
		//This execute will just toss all PIC records with no associated ProcInfo records (this seems to
		//	be true for every Custom Record).
		ExecuteSql("DELETE FROM PicT WHERE EmrGroupID = %li AND ProcInfoID IS NULL", EMRID);
		//This is a safety... there should not be anything left, but if somehow a ProcInfo did
		//	get tied to a Custom record, this will remove the custom record, but leave the tracking
		//	ladder portion.
		ExecuteSql("UPDATE PicT SET EmrGroupID = NULL WHERE EmrGroupID = %li", EMRID);

		//do the deletion
		// (j.jones 2006-04-26 09:37) - PLID 20064 - we now simply mark these records as being deleted
		//ExecuteSql("DELETE FROM EMRGroupsT WHERE ID = %li",EMRID);
		ExecuteSql("UPDATE EMRGroupsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), EMRID);
		
		// (c.haag 2008-07-28 09:43) - PLID 30853 - Delete all problems linked directly with the EMR. Note that we don't do this for EMN's and their
		// children as well; this is because they are not marked as deleted (but they are deleted by virtue of the EMR being deleted)
		// (c.haag 2009-05-12 09:12) - PLID 28494 - Use the new EMR problem linking table
		// (j.jones 2009-06-02 14:09) - PLID 34301 - detect if other links exist than the ones we are deleting,
		// and if so, delete just our links, otherwise delete the whole problem
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		_RecordsetPtr rsLinkedProblems = CreateParamRecordset("SELECT ID, "
			"CASE WHEN EMRProblemID IN ("
				"SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
			") "
			"THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete "
			"FROM EMRProblemLinkT WHERE	EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}",
			eprtEmrEMR, EMRID,
			eprtEmrEMR, EMRID);

		CString strEMRProblemLinkIDsToDelete, strEMRProblemIDsToDelete;
		while(!rsLinkedProblems->eof) {

			//every record references a link to delete
			long nProblemLinkID = AdoFldLong(rsLinkedProblems, "ID");

			if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
				strEMRProblemLinkIDsToDelete += ",";
			}
			strEMRProblemLinkIDsToDelete += AsString(nProblemLinkID);

			//the problem ID will be -1 if we're not deleting the problem the above link references,
			//will be a real ID if we do need to delete the problem
			long nProblemID = AdoFldLong(rsLinkedProblems, "EMRProblemIDToDelete", -1);
			if(nProblemID != -1) {
				if(!strEMRProblemIDsToDelete.IsEmpty()) {
					strEMRProblemIDsToDelete += ",";
				}
				//we might end up having duplicate IDs in this string, but it's just for an IN clause,
				//so it's no big deal
				strEMRProblemIDsToDelete += AsString(nProblemID);
			}

			rsLinkedProblems->MoveNext();
		}
		rsLinkedProblems->Close();

		// (j.jones 2009-06-02 14:55) - PLID 34301 - now we already know which problems & problem links to delete
		if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
			ExecuteSql("DELETE FROM EmrProblemLinkT WHERE ID IN (%s)", strEMRProblemLinkIDsToDelete);
		}
		if(!strEMRProblemIDsToDelete.IsEmpty()) {
			ExecuteSql("UPDATE EmrProblemsT SET Deleted = 1, DeletedDate = GetDate(), DeletedBy = '%s' WHERE ID IN (%s)", _Q(GetCurrentUserName()), strEMRProblemIDsToDelete);
		}

		//TES 6/3/2009 - PLID 34371 - Update patient wellness
		UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), nPatientID);
		// (c.haag 2010-09-21 11:35) - PLID 40612 - Don't create todo alarms for decisions when deleting records
		//TodoCreateForDecisionRules(GetRemoteData(), nPatientID);

		//now audit
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID),nAuditID,aeiEMRDeleted,EMRID,strDescription,"<Deleted>",aepHigh,aetDeleted);

		CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);

	}NxCatchAll("Error deleting EMR.");
}

void DeleteEMN(long EMNID) {

	_RecordsetPtr rsTaskIds = CreateRecordset("SELECT FollowupTaskID, ProcedureTaskID FROM EmrMasterT WHERE ID = %li", EMNID);
	long nFollowupTaskId = AdoFldLong(rsTaskIds, "FollowupTaskID", -1);
	long nProcedureTaskId = AdoFldLong(rsTaskIds, "ProcedureTaskID", -1);		
	_RecordsetPtr rs = CreateRecordset("SELECT PatientID, dbo.GetEmrString(EmrMasterT.ID) AS ProcName FROM EmrMasterT WHERE ID = %li", EMNID);
	long nPatientID = AdoFldLong(rs, "PatientID");
	CString strProcedureName = AdoFldString(rs, "ProcName", "");

	// (c.haag 2006-11-16 08:41) - PLID 21454 - Delete problems related to the detail
	// (j.jones 2008-07-16 09:39) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
	// (c.haag 2009-05-12 09:12) - PLID 28494 - Use the new EMR problem linking table
	// (j.jones 2009-06-02 14:09) - PLID 34301 - detect if other links exist than the ones we are deleting,
	// and if so, delete just our links, otherwise delete the whole problem
	// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
	_RecordsetPtr rsLinkedProblems = CreateParamRecordset("SELECT ID, "
		"CASE WHEN EMRProblemID IN ("
			"SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT "
				"(((EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT}) AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRID = {INT})) "
				"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRTopicsT WHERE EMRID = {INT})) "
				"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRMasterT WHERE ID = {INT})) "
				"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDiagCodesT WHERE EMRID = {INT})) "
				"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRChargesT WHERE EMRID = {INT})) "
				"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT MedicationID FROM EMRMedicationsT WHERE EMRID = {INT}))) "
		") "
		"THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete "
		"FROM EMRProblemLinkT WHERE "
			"((EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT}) AND EMRRegardingID IN (SELECT ID FROM EMRDetailsT WHERE EMRID = {INT})) "
			"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRTopicsT WHERE EMRID = {INT})) "
			"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRMasterT WHERE ID = {INT})) "
			"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRDiagCodesT WHERE EMRID = {INT})) "
			"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT ID FROM EMRChargesT WHERE EMRID = {INT})) "
			"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID IN (SELECT MedicationID FROM EMRMedicationsT WHERE EMRID = {INT})) ",
			eprtEmrItem, eprtEmrDataItem, EMNID,
			eprtEmrTopic, EMNID,
			eprtEmrEMN, EMNID,
			eprtEmrDiag, EMNID,
			eprtEmrCharge, EMNID,
			eprtEmrMedication, EMNID,
			eprtEmrItem, eprtEmrDataItem, EMNID,
			eprtEmrTopic, EMNID,
			eprtEmrEMN, EMNID,
			eprtEmrDiag, EMNID,
			eprtEmrCharge, EMNID,
			eprtEmrMedication, EMNID);

	CString strEMRProblemLinkIDsToDelete, strEMRProblemIDsToDelete;
	while(!rsLinkedProblems->eof) {

		//every record references a link to delete
		long nProblemLinkID = AdoFldLong(rsLinkedProblems, "ID");

		if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
			strEMRProblemLinkIDsToDelete += ",";
		}
		strEMRProblemLinkIDsToDelete += AsString(nProblemLinkID);

		//the problem ID will be -1 if we're not deleting the problem the above link references,
		//will be a real ID if we do need to delete the problem
		long nProblemID = AdoFldLong(rsLinkedProblems, "EMRProblemIDToDelete", -1);
		if(nProblemID != -1) {
			if(!strEMRProblemIDsToDelete.IsEmpty()) {
				strEMRProblemIDsToDelete += ",";
			}
			//we might end up having duplicate IDs in this string, but it's just for an IN clause,
			//so it's no big deal
			strEMRProblemIDsToDelete += AsString(nProblemID);
		}

		rsLinkedProblems->MoveNext();
	}
	rsLinkedProblems->Close();

	BEGIN_TRANS("DeleteEMN") {

		// (j.jones 2009-06-02 14:55) - PLID 34301 - now we already know which problems & problem links to delete
		if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
			ExecuteSql("DELETE FROM EmrProblemLinkT WHERE ID IN (%s)", strEMRProblemLinkIDsToDelete);
		}
		if(!strEMRProblemIDsToDelete.IsEmpty()) {
			ExecuteSql("UPDATE EmrProblemsT SET Deleted = 1, DeletedDate = GetDate(), DeletedBy = '%s' WHERE ID IN (%s)", _Q(GetCurrentUserName()), strEMRProblemIDsToDelete);
		}
		ExecuteSql("UPDATE EmrDetailsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE EMRID = %li", _Q(GetCurrentUserName()), EMNID);
		ExecuteSql("UPDATE EMRTopicsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE EMRID = %li", _Q(GetCurrentUserName()), EMNID);
		ExecuteSql("UPDATE EMRProcedureT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE EMRID = %li", _Q(GetCurrentUserName()), EMNID);		
		ExecuteSql("UPDATE EMRChargesT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE EMRID = %li", _Q(GetCurrentUserName()), EMNID);		
		ExecuteSql("UPDATE EMRDiagCodesT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE EMRID = %li", _Q(GetCurrentUserName()), EMNID);		
		ExecuteSql("UPDATE EmrMedicationsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE EMRID = %li", _Q(GetCurrentUserName()), EMNID);		
		//(e.lally 2006-08-10) PLID 21900 - There are two fields that can reference the ToDo alarms that we delete just afterwards. We need to make those fields NULL.
		ExecuteSql("UPDATE EMRMasterT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s', FollowUpTaskID = NULL, ProcedureTaskID = NULL WHERE ID = %li", _Q(GetCurrentUserName()), EMNID);

		PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_EMRCreated, EMNID, -1);
		// (c.haag 2008-06-10 17:45) - PLID 30328 - Also need to delete from TodoAssignToT
		TodoDelete(FormatString("TaskID = %li OR TaskID = %li", nFollowupTaskId, nProcedureTaskId));

		//TES 6/3/2009 - PLID 34371 - Update patient wellness
		UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), nPatientID);
		// (c.haag 2010-09-21 11:35) - PLID 40612 - Don't create todo alarms for decisions when deleting records
		//TodoCreateForDecisionRules(GetRemoteData(), nPatientID);

	}END_TRANS_CATCH_ALL("DeleteEMN");

	//audit the deletion
	long nAuditID = BeginNewAuditEvent();
	if(strProcedureName == "")
		strProcedureName = "{No Procedure}";
	strProcedureName = "For " + strProcedureName;
	AuditEvent(nPatientID, GetExistingPatientName(nPatientID),nAuditID,aeiEMNDeleted,EMNID,strProcedureName,"<Deleted>",aepHigh,aetDeleted);

	CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);
}

//DRT 3/6/2006 - PLID 19571 - Copied this code from CPatientProcedureDlg::OnDeleteProcedure(), since tracking records
//	and EMR records are now entirely associated, we'll use this to share the deletion.  My only changes were some wording
//	regarding the prompts.
//Any call to this function will prompt the user for failure (unless bSilent is set), the calling function does not need to provide feedback if
//	return value is FALSE.
//Note that the bSilent flag will remove the prompt for password if permission is set, it will just return FALSE.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL AttemptRemovePICRecord(long nPicID, IN OUT CDWordArray &arNewCDSInterventions, BOOL bSilent /*= FALSE*/)
{
	long nEMRGroupID = -1;
	long nProcInfoID = -1;
	CString strLadderName = "";
	long nPatientID = -1;

	//Get our IDs first
	{		
		// (a.walling 2008-06-04 15:33) - PLID 29900 - Gather the patient ID while we are here.
		_RecordsetPtr prs = CreateRecordset("SELECT ProcInfoID, EMRGroupID, dbo.CalcProcInfoName(ProcInfoID) AS LadderName, ProcInfoT.PatientID FROM PicT LEFT JOIN ProcInfoT ON PicT.ProcInfoID = ProcInfoT.ID WHERE PicT.ID = %li", nPicID);
		if(prs->eof) {
			if(!bSilent)
				AfxMessageBox("You have chosen to delete an invalid record, it may have already been deleted.  Please refresh and try again.");
			return FALSE;
		}

		nEMRGroupID = AdoFldLong(prs, "EMRGroupID", -1);
		nProcInfoID = AdoFldLong(prs, "ProcInfoID", -1);
		strLadderName = AdoFldString(prs, "LadderName", "");
		nPatientID = AdoFldLong(prs, "PatientID", -1);
		prs->Close();
	}

	//Check for locked EMNs.
	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	if(nEMRGroupID != -1 && ReturnsRecordsParam("SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND Status = 2 AND EmrGroupID = {INT}", nEMRGroupID)) {
		if(!bSilent)
			MsgBox("This PIC contains locked EMNs.  It cannot be deleted.");
		return FALSE;
	}

	// (z.manning 2009-12-15 11:29) - PLID 35810 - Don't allow deletion of non-finalized patient-created EMNs.
	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	if(nEMRGroupID != -1 && ReturnsRecordsParam("SELECT TOP 1 ID FROM EmrMasterT WHERE Deleted = 0 AND PatientCreatedStatus = {INT} AND EmrGroupID = {INT}", CEMN::pcsCreatedByPatientNotFinalized, nEMRGroupID)) {
		if(!bSilent) {
			MsgBox("This PIC contains patient-created EMNs that have not yet been finalized.  It cannot be deleted.");
		}
		return FALSE;
	}

	// (c.haag 2006-07-10 16:44) - PLID 19977 - Check for problem details
	// (c.haag 2006-12-13 09:54) - PLID 21454 - Check for deleted problems
	// (j.jones 2008-07-16 09:39) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
	// for details only, we will need to change this further to support other types
	// (c.haag 2008-07-24 12:26) - PLID 30826 - Stop users who cannot delete EMR's with problems
	// at any level from deleting them
	if(nEMRGroupID != -1 && !CanCurrentUserDeleteEmrProblems()) {
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		// (j.jones 2012-07-31 16:08) - PLID 51750 - GetEMNProblemSql now takes in an Order By clause, but the order is not important here, so just order by EMRMasterT.ID
		// (j.jones 2013-07-02 08:56) - PLID 57271 - removed sqlEMNSortOrder, it always sorts by EMR ID now
		_RecordsetPtr prs = CreateParamRecordset("{SQL}", GetEMNProblemSql(CSqlFragment("SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND EmrGroupID = {INT}", nEMRGroupID)));
		if (!prs->eof) {
			prs->Close();
			if(!bSilent)
				MsgBox(MB_OK | MB_ICONERROR, "This PIC contains EMNs with details marked as problems.  It cannot be deleted.");
			return FALSE;
		}
	}

	// (a.walling 2008-06-25 16:06) - PLID 30515 - Check for in-use EMNs
	// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	// (j.armen 2013-05-15 09:55) - PLID 56680 - EMN Access Refactoring
	if(nEMRGroupID != -1 && ReturnsRecordsParam(
		"SELECT 1 FROM EMNAccessT A WITH(UPDLOCK, HOLDLOCK)\r\n"
		"INNER JOIN EMRMasterT M ON A.EmnID = M.ID\r\n"
		"WHERE M.EmrGroupID = {INT} AND Deleted = 0",
		nEMRGroupID))
	{
		if(!bSilent)
			MsgBox("This PIC contains EMNs which are currently being modified by another user.  It cannot be deleted.");
		return FALSE;
	}

	//first ensure they have EMR and a record exists

	BOOL bHasEMNs = FALSE;
	CString strEMRDescription;

	if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {

		// (j.jones 2009-10-01 11:36) - PLID 30479 - get the EMR description for later
		_RecordsetPtr rsEMR = CreateParamRecordset("SELECT TOP 1 EMRGroupsT.Description "
			"FROM EmrMasterT "
			"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"WHERE EMRGroupsT.Deleted = 0 AND EmrMasterT.Deleted = 0 AND EMRGroupsT.ID = {INT}", nEMRGroupID);
		if(!rsEMR->eof) {
			//they have an EMN, so let's check permissions
			// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
			// it is now administrator-only
			// (a.walling 2007-11-28 13:11) - PLID 28044 - Also check for a valid current EMR license
			if(!IsCurrentUserAdministrator() || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
				AfxMessageBox("You do not have permission to delete the EMR for this record. Only an Administrator user can perform this action.");
				return FALSE;
			}

			bHasEMNs = TRUE;
			strEMRDescription = AdoFldString(rsEMR, "Description", "");
		}
		rsEMR->Close();
	}

	//Top level row, delete whole proceduure
	if(!bSilent) {
		if(IDYES != AfxMessageBox("This will delete all information about this record, including:\r\n"
			" - All ladders\r\n"
			" - All Procedure Information Center data\r\n"
			" - All EMNs\r\n\r\n"
			"This cannot be undone.\r\nAre you SURE you wish to do this?", MB_YESNO)) {
			return FALSE;
		}
	}

	// (j.jones 2009-10-01 11:36) - PLID 30479 - added a specific dialog
	// to prompt for EMR deletion confirmation, which will double-warn if
	// any EMNs exists in the EMR for this PIC (you won't get this dialog
	// if there is an empty EMR)
	// It should not be possible to have this function called with bSilent = TRUE
	// if an EMN exists, so ignore that boolean and always warn.
	if(bHasEMNs) {
		CDeleteEMNConfirmDlg dlg(ecdtEMR, strEMRDescription, NULL);
		if(dlg.DoModal() != DELETE_EMN_RETURN_DELETE) {
			//unless the return value specifically says to delete, leave now
			return FALSE;
		}
	}


	//1)  Clear out the "general" stuff first
	ExecuteSql("UPDATE MailSent SET PicID = NULL WHERE PicID = %li", nPicID);
	// (r.galicki 2008-10-08 11:43) - PLID 31555 - Constraint in LabsT
	ExecuteSql("UPDATE LabsT SET PicID = NULL WHERE PicID = %li", nPicID);
	// (c.haag 2010-07-20 17:34) - PLID 30894 - We need to send a labs table refresh
	// to remove the PIC's old labs from any open lab lists.
	// (r.gonet 09/02/2014) - PLID 63221 - Send an ex tablechecker and the patient ID
	CClient::RefreshLabsTable(nPatientID, -1);

	//Now remove the EMR itself
	if(nEMRGroupID != -1) {
		long nLicense = g_pLicense->HasEMR(CLicense::cflrSilent);
		if(nLicense == 1) {
			//Custom records
			DeleteCustomRecord(nEMRGroupID);
		}
		else {
			//Either we have NexEMR, or there's a record and no license (probably de-activated).
			//	Since we don't sell Custom Records, there'd be no reason it's deactivated, therefore
			//	it's highly likely this is a NexEMR record.
			//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
			if(!DeleteNexEMR(nEMRGroupID, arNewCDSInterventions))
				return FALSE;
		}
	}

	//And now remove the tracking portion	

	if(nProcInfoID != -1) {
		DeleteProcInfo(nProcInfoID);
	}

	ExecuteSql("DELETE FROM PicT WHERE ID = %li", nPicID);

	//audit
	long nAuditID = BeginNewAuditEvent();
	// (a.walling 2008-06-04 15:33) - PLID 29900 - Use the correct patient name and ID
	AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientProcedureDelete, nPatientID, strLadderName, "<Deleted>", aepMedium, aetDeleted);

	return TRUE;
}

//This function should NEVER be called without first ensuring that there are no locked EMNs on the given EMR.  There
//	is a failsafe however, this function will return FALSE if it finds a locked EMN while attempting to delete.  If
//	that happens, no deletion will take place.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL DeleteNexEMR(long nEMRGroupID, IN OUT CDWordArray &arNewCDSInterventions)
{
	//1)  Take out the EMR.  We'll do this by loading the EMR record, telling it to remove all EMNs, and then save it.  This
	//	is the safest way of achieving our goal, because it lets us keep just 1 single point of saving for all the EMR.
	//This will handle all the EMR related auditing as well.
	if(nEMRGroupID != -1) {

		//prepare for auditing
		_RecordsetPtr rs = CreateRecordset("SELECT PatientID, Description FROM EMRGroupsT WHERE ID = %li", nEMRGroupID);
		CString strDescription;
		long nPatientID = -25;
		if(!rs->eof) {
			nPatientID = AdoFldLong(rs, "PatientID");
			strDescription = AdoFldString(rs, "Description","");
		}
		rs->Close();
		if(strDescription == "")
			strDescription = "{No Description}";

		CWaitCursor wc;
		//Load the entire EMR, this might take a little time.
		CEMR* pEMR = new CEMR;
		//DRT 7/27/2007 - PLID 26836 - Since we're just deleting and not displaying, set the "emn to be displayed" to -1, this
		//	will make loading faster for all EMNs
		long nEMNID = -1;
		pEMR->LoadFromID(nEMRGroupID, FALSE, nEMNID);

		// (c.haag 2008-07-24 12:22) - PLID 30820 - Warn the user if any problems are going to be deleted
		if (!CheckForDeletedEmrProblems(esotEMR, (long)pEMR, TRUE)) {
			// (j.jones 2009-05-30 18:55) - PLID 34408 - cleanup our memory
			delete pEMR;
			return FALSE; // User aborted the delete
		} else {
			// User accepts deleting all the problems.
		}

		//TES 6/5/2008 - PLID 30196 - Track whether any of the deleted EMNs had system tables on them, we will need to warn
		// the user if so.
		bool bHasCurrentMeds = false;
		bool bHasAllergies = false;
		//Loop over all EMNs and remove them from the EMR.
		for(int i = pEMR->GetEMNCount() - 1; i >= 0; i--) {
			CEMN* pEMN = pEMR->GetEMN(i);
			if(pEMN->GetStatus() == 2) {
				//We may NOT delete a locked EMN.  This is a safety, anyone calling this
				//	function should have checked beforehand.
				// (j.jones 2009-05-30 18:55) - PLID 34408 - cleanup our memory
				delete pEMR;
				return FALSE;
			}
			if(!bHasCurrentMeds && pEMN->HasSystemTable(eistCurrentMedicationsTable)) {
				bHasCurrentMeds = true;
			}
			if(!bHasAllergies && pEMN->HasSystemTable(eistAllergiesTable)) {
				bHasAllergies = true;
			}
			// (a.walling 2008-06-25 16:10) - PLID 30515 - This will acquire a write token for all EMNs.
			pEMR->RemoveEMN(pEMN);
		}

		// (j.jones 2012-09-28 09:00) - PLID 52820 - SaveEMRObject requires this parameter, but currently
		// we do not check drug interactions after deleting EMRs
		BOOL bDrugInteractionsChanged = FALSE;

		//Now save the EMR object with no EMNs.  This in effect deletes all EMNs from the EMR.
		// (a.walling 2008-06-26 16:20) - PLID 30513 - use FAILED macro to test success of SaveEMRObject
		//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
		if(FAILED(SaveEMRObject(esotEMR, (long)pEMR, FALSE, bDrugInteractionsChanged, arNewCDSInterventions))) {
			//Cleanup
			delete pEMR;
			return FALSE;
		}

		//Cleanup
		delete pEMR;

		//The above clears all EMNs, but leaves us an empty EMR record.  Clear the last of the data
		ExecuteSql("UPDATE PicT SET EMRGroupID = NULL WHERE EMRGroupID = %li", nEMRGroupID);
		
		// (j.jones 2006-04-26 09:37) - PLID 20064 - we now simply mark these records as being deleted
		//ExecuteSql("DELETE FROM EMRGroupsT WHERE ID = %li", nEMRGroupID);
		ExecuteSql("UPDATE EMRGroupsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), nEMRGroupID);

		// (c.haag 2008-07-28 10:33) - PLID 30853 - Delete all problems linked directly with the EMR. Note that we don't do this for EMN's and their
		// children as well; this is because they are not marked as deleted (but they are deleted by virtue of the EMR being deleted)
		// (c.haag 2009-05-12 09:12) - PLID 28494 - Use the new EMR problem linking table
		// (j.jones 2009-06-02 14:09) - PLID 34301 - detect if other links exist than the ones we are deleting,
		// and if so, delete just our links, otherwise delete the whole problem
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		_RecordsetPtr rsLinkedProblems = CreateParamRecordset("SELECT ID, "
			"CASE WHEN EMRProblemID IN ("
				"SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
			") "
			"THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete "
			"FROM EMRProblemLinkT WHERE	EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}",
			eprtEmrEMR, nEMRGroupID,
			eprtEmrEMR, nEMRGroupID);

		CString strEMRProblemLinkIDsToDelete, strEMRProblemIDsToDelete;
		while(!rsLinkedProblems->eof) {

			//every record references a link to delete
			long nProblemLinkID = AdoFldLong(rsLinkedProblems, "ID");

			if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
				strEMRProblemLinkIDsToDelete += ",";
			}
			strEMRProblemLinkIDsToDelete += AsString(nProblemLinkID);

			//the problem ID will be -1 if we're not deleting the problem the above link references,
			//will be a real ID if we do need to delete the problem
			long nProblemID = AdoFldLong(rsLinkedProblems, "EMRProblemIDToDelete", -1);
			if(nProblemID != -1) {
				if(!strEMRProblemIDsToDelete.IsEmpty()) {
					strEMRProblemIDsToDelete += ",";
				}
				//we might end up having duplicate IDs in this string, but it's just for an IN clause,
				//so it's no big deal
				strEMRProblemIDsToDelete += AsString(nProblemID);
			}

			rsLinkedProblems->MoveNext();
		}
		rsLinkedProblems->Close();

		// (j.jones 2009-06-02 14:55) - PLID 34301 - now we already know which problems & problem links to delete
		if(!strEMRProblemLinkIDsToDelete.IsEmpty()) {
			ExecuteSql("DELETE FROM EmrProblemLinkT WHERE ID IN (%s)", strEMRProblemLinkIDsToDelete);
		}
		if(!strEMRProblemIDsToDelete.IsEmpty()) {
			ExecuteSql("UPDATE EmrProblemsT SET Deleted = 1, DeletedDate = GetDate(), DeletedBy = '%s' WHERE ID IN (%s)", _Q(GetCurrentUserName()), strEMRProblemIDsToDelete);
		}

		//TES 6/3/2009 - PLID 34371 - Update patient wellness
		UpdatePatientWellnessQualification_EMRProblems(GetRemoteData(), nPatientID);
		// (c.haag 2010-09-21 11:35) - PLID 40612 - Don't create todo alarms for decisions when deleting records
		//TodoCreateForDecisionRules(GetRemoteData(), nPatientID);

		//now audit
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID),nAuditID,aeiEMRDeleted,nEMRGroupID,strDescription,"<Deleted>",aepHigh,aetDeleted);

		if(bHasCurrentMeds || bHasAllergies) {
			//TES 6/5/2008 - PLID 30196 - Warn them to check the medications tab.  Note that I'm comfortable putting an 
			// AfxMessageBox in this function, because it's only called from AttemptRemovePicRecord(), which has its own
			// AfxMessageBoxes.
			CString strTables;
			if(bHasCurrentMeds && bHasAllergies) {
				strTables = "'Current Medications' and 'Allergies' tables";
			}
			else if(bHasCurrentMeds) {
				strTables = "'Current Medications' table";
			}
			else if(bHasAllergies) {
				strTables = "'Allergies' table";
			}

			AfxMessageBox("The deleted EMR contained the " + strTables + ".  Please review this patient's Medications tab to "
				"make sure that its corresponding information is up to date.");
		}
	}

	return TRUE;
}

bool ForceFieldLength(CEdit * pEditBox, int nLength /* = 2500*/) {
	//We can only store notes here of up to nLength characters in length.
	
	//How long is this note?
	CString strNote;
	pEditBox->GetWindowText(strNote);
	
	//If it's too long
	int nActualLength = strNote.GetLength();
	if(nActualLength > nLength){

		//Tell them the error of their ways
		CString strMessage;
		strMessage.Format("Notes in this field can be no greater than %li characters in length.  Would you like to continue?  If you say yes, the field will be truncated to %li characters", nLength, nLength);
		int nID = MessageBox(GetDesktopWindow(), strMessage, "Invalid Note Length", MB_YESNO);

		//If they don't care
		if(nID == IDYES){
			//Truncate
			strNote = strNote.Left(nLength);
			pEditBox->SetWindowText(strNote);
			return true;
			
		}
		//If they do care
		else{
			//Put the focus back
			pEditBox->SetFocus();
			
			//Set the cursor to the nLengthth character
			pEditBox->SetSel(nLength,nActualLength);
			return false;
		}
	}

	return true;
				

	//That's all  (the actual value will be saved at a later date)


}

// (a.walling 2010-08-04 09:20) - PLID 38964 - I got rid of GetErrorHelpLocation and GetErrorHelpBookmark since they were not used.

//TS: ActiveUpdate functions
// Returns the text string of the first cd rom drive found on the system
CString GetFirstCdRom()
{
	// Get the list of all drives
	TCHAR strAllDrives[MAX_PATH];
	DWORD dwLen = GetLogicalDriveStrings(MAX_PATH, strAllDrives);

	if (dwLen > 0) {
		if (dwLen > MAX_PATH) dwLen = MAX_PATH;
		// Loop through the (null-delimited null-terminated) list of drive paths
		TCHAR *pCurPos = strAllDrives;
		// While our current string doesn't begin with a null
		while (*pCurPos != NULL) {
			// Pull the current null-terminated string
			// See what kind of drive it is
			if (GetDriveType(pCurPos) == DRIVE_CDROM) {
				return pCurPos;
			}
			// Move to the next null-terminated string
			pCurPos += strlen(pCurPos) + 1;
		}
	}

	// If we made it to here we didn't find any cdrom drives
	return "";
}

//taken from RAC
// IMPORTANT NOTE: Make sure the TO path is not a subdirectory of the FROM path and vice-versa!!!
bool CopyDirectory(	LPCSTR strToPath, 
					LPCSTR strFromPath,
					HWND progress /*= NULL*/)
{
	char	strFrom[MAX_PATH], 
			strTo[MAX_PATH];

	strcpy(strTo, strToPath);
	strcpy(strFrom, strFromPath);
	strcat(strFrom, "\\*.*");

	bool bContinue;// = f.FindFile(strFrom + "\\*.*");
		
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	hFind = FindFirstFile(strFrom, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		bContinue = false;
	else bContinue = true;

	while (bContinue) 
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(FindFileData.cFileName, ".") && strcmp(FindFileData.cFileName, ".."))
			{
				//reset the paths!
				strcpy(strTo, strToPath);
				strcpy(strFrom, strFromPath);

				strcat(strTo, "\\");
				strcat(strTo, FindFileData.cFileName);
				strcat(strFrom, "\\");
				strcat(strFrom, FindFileData.cFileName);

				CreateDirectory(strTo, NULL);
				if (!CopyDirectory(strTo, strFrom, progress))
					return false;
			}
		} 
		else 
		{	// Just a regular file, so copy it
			strcpy(strTo, strToPath);
			V(strTo, FindFileData.cFileName);
			strcpy(strFrom, strFromPath);
			V(strFrom, FindFileData.cFileName);

			if (!CopyFile(strFrom, strTo, FALSE))
			{
				//try harder
				SetFileAttributes(strTo, FILE_ATTRIBUTE_NORMAL);
				if (!CopyFile(strFrom, strTo, FALSE))
					return false;
			}
			if (progress && ::IsWindow(progress))
				SendMessage(progress, PBM_STEPIT, 0, 0);
		}

		if (!FindNextFile(hFind, &FindFileData))
			bContinue = false;
	}
	return true;
}

char * V (char * directory, LPCSTR file)
{
	if (directory[strlen(directory) - 1] != '\\')
		strcat(directory, "\\");
	strcat(directory, file);
	return directory;
}

__int64 CheckSum(CString &filename)
{
	__int64 result = 0, 
		buffer;
	DWORD dwRead;
	CFile file;

	if (!file.Open(filename, CFile::modeRead | CFile::shareCompat))
		return 0;

	do 
	{	dwRead = file.Read((void *)&buffer, 8);
		result += buffer;
	}
	while (dwRead > 0);
	return result;
}


OLE_COLOR GetLighterColor(OLE_COLOR colDarkColor)
{
	return RGB((GetRValue(colDarkColor)+255)/2,(GetGValue(colDarkColor)+255)/2,(GetBValue(colDarkColor)+255)/2);
}

//check the license file for Plastic version
bool IsPlastic() {

	if(g_pLicense->GetPracticeType()==0)
		return TRUE;
	else
		return FALSE;
}

//check the license file for Reproductive version
bool IsReproductive() {

	if(g_pLicense->GetPracticeType()==1)
		return TRUE;
	else
		return FALSE;
}

//check the license file for Refractive version
bool IsRefractive() {

	if(g_pLicense->GetPracticeType()==2)
		return TRUE;
	else
		return FALSE;
}

//check the license file for NexTech Internal version
bool IsNexTechInternal() {

	if(g_pLicense->GetPracticeType()==3)
		return TRUE;
	else
		return FALSE;
}

// Check to see if we're licensed for surgery center components
bool IsSurgeryCenter(BOOL bMakingUseOfLicense)
{
	CLicense::ECheckForLicenseReason ecflr = bMakingUseOfLicense ? CLicense::cflrUse : CLicense::cflrSilent;
	if (g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, ecflr)) {
		return true;
	} else {
		return false;
	}
}

bool IsSpa(BOOL bMakingUseOfLicense)
{
	CLicense::ECheckForLicenseReason ecflr = bMakingUseOfLicense ? CLicense::cflrUse : CLicense::cflrSilent;
	if (g_pLicense->CheckForLicense(CLicense::lcNexSpa, ecflr)) {
		return true;
	} else {
		return false;
	}
}

bool IsSupportPaidFor() {

	COleDateTimeSpan dtSpan;
	dtSpan.SetDateTimeSpan(1,0,0,0);
	COleDateTime dtNew, dtNow;
	dtNew = g_pLicense->GetExpirationDate();
	dtNew += dtSpan;
	dtNow = COleDateTime::GetCurrentTime();

	if(dtNew.m_status == COleDateTime::invalid) {
		return FALSE;
	}
	
	if(dtNew.m_dt <= dtNow.m_dt)
		return FALSE;

	return TRUE;
}

bool GetAppointmentLinkingAllowed()
{
	//JMJ removed 11/10/2003
	//if (g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, CLicense::cflr) || GetRemotePropertyInt("ForceAptLinking", 0, 0, "<None>") == 1)
		return true;
	//return false;
}

COleDateTime GetSupportExpirationDate() {

	return g_pLicense->GetExpirationDate();
}

// Copy from one DEVMODE object to another (make sure pDevModeOut points to (sizeof(*pDevModeIn) + pDevModeIn->dmDriverExtra) bytes)
void CopyDevMode(IN const DEVMODE *pDevModeIn, OUT DEVMODE *pDevModeOut)
{
    memcpy(pDevModeOut, pDevModeIn, sizeof(*pDevModeIn) + pDevModeIn->dmDriverExtra);
}

// Allocates a copy of the given device settings (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
void AllocCopyOfDeviceSettings(IN HGLOBAL hDevNames, IN HGLOBAL hDevMode, OUT DEVMODE **ppDevMode, OUT LPTSTR *ppstrPrinter, OUT LPTSTR *ppstrDriver, OUT LPTSTR *ppstrPort)
{

	//DRT 3/13/03 - ensure that the values we're trying to use are not NULL!
	if(hDevNames) 
		
	// Get all the names out of the pInfo structures, then copy the strings into newly 
	// allocated memory and pass the pointers out through the function parameters
	{
		// Pointer to a base struct
		DEVNAMES* pAppDevNames = (DEVNAMES *)GlobalLock(hDevNames);
		// The struct's members are OFFSETS FROM the address of the struct
		// Add each OFFSET to the address of the struct and you get the address of the corresponding string
		// (This is called a "moveable block of memory" because you can relocated the whole thing and the offsets are still correct)
		LPTSTR strPrinter = NULL, strDriver = NULL, strPort = NULL;
		strPrinter = reinterpret_cast<LPTSTR>(pAppDevNames) + pAppDevNames->wDeviceOffset;
		strDriver = reinterpret_cast<LPTSTR>(pAppDevNames) + pAppDevNames->wDriverOffset;
		strPort = reinterpret_cast<LPTSTR>(pAppDevNames) + pAppDevNames->wOutputOffset;

		// Allocate space for the new names, and copy the strings in there
		LPTSTR strNewPrinter = new TCHAR[strlen(strPrinter) + 1];
		strcpy(strNewPrinter, strPrinter);
		
		LPTSTR strNewDriver = new TCHAR[strlen(strDriver) + 1];
		strcpy(strNewDriver, strDriver);
		
		LPTSTR strNewPort= new TCHAR[strlen(strPort) + 1];
		strcpy(strNewPort, strPort);

		// We're finished with the global memory so release the handle
		GlobalUnlock(hDevNames);
		pAppDevNames = NULL;

		// Now that they're all copied, set the output parameters to point to the new memory
		*ppstrPrinter = strNewPrinter;
		*ppstrDriver = strNewDriver;
		*ppstrPort = strNewPort;
	}



	//DRT 3/13/03 - ensure that the values we're trying to use are not NULL!
	if(hDevMode)

	// Get the app's devmode object and make a copy of it, passing the newly allocated memory out through the ppDevMode parameter
	{
		// Grab the app's devmode object
		DEVMODE* pAppDevMode = (DEVMODE *)GlobalLock(hDevMode);

		// Make a copy of the app's device mode object, this is what we will use in the end
		DEVMODE *pNewDevMode = (DEVMODE *)(new BYTE[sizeof(DEVMODE) + pAppDevMode->dmDriverExtra]);
		CopyDevMode(pAppDevMode, pNewDevMode);

		// Release the original (we still have our copy)
		GlobalUnlock(hDevMode);
		pAppDevMode = NULL;

		// Set our output variable
		*ppDevMode = pNewDevMode;
	}
}

// Allocates a copy of the given CPrintInfo object's device settings (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
void AllocCopyOfDeviceSettings(IN CPrintInfo *pInfo, OUT DEVMODE **ppDevMode, OUT LPTSTR *ppstrPrinter, OUT LPTSTR *ppstrDriver, OUT LPTSTR *ppstrPort)
{
	AllocCopyOfDeviceSettings(pInfo->m_pPD->m_pd.hDevNames, pInfo->m_pPD->m_pd.hDevMode, 
		ppDevMode, ppstrPrinter, ppstrDriver, ppstrPort);
}

// Allocates a copy of THIS APPLICATION's current device settings (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
void AllocCopyOfAppDeviceSettings(OUT DEVMODE **ppDevMode, OUT LPTSTR *ppstrPrinter, OUT LPTSTR *ppstrDriver, OUT LPTSTR *ppstrPort)
{
	// Get the app's print info object, which we will use to load all our app device settings
	CPrintInfo PrintInfo, *pInfo = &PrintInfo;
	{
		// Get the defaults from the app
		pInfo->m_pPD->m_pd.Flags |= PD_RETURNDEFAULT;
		pInfo->m_pPD->m_pd.hDevMode = (HANDLE)0;
		pInfo->m_pPD->m_pd.hDevNames = (HANDLE)0;
		// This loads the app's settings instead of the system's defaults
		AfxGetApp()->DoPrintDialog(pInfo->m_pPD);

		if(pInfo->m_pPD->m_pd.hDevNames == NULL) {
			//DRT 3/13/03 - If this returns null, we have no printers available!  Not much reason to try allocating
			//		any space for them, since there is nothing to put there.
			return;
		}
	}

	// Allocated the copies
	AllocCopyOfDeviceSettings(pInfo, ppDevMode, ppstrPrinter, ppstrDriver, ppstrPort);

	return;
}

// Free device settings objects (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
// Pass the addresses of your variables, that way this function can clear your variables after freeing the memory that your variables point to
void FreeCopyOfDeviceSettings(IN OUT DEVMODE **ppDevMode, IN OUT LPTSTR *ppstrPrinter, IN OUT LPTSTR *ppstrDriver, IN OUT LPTSTR *ppstrPort)
{
	ASSERT(ppDevMode && *ppDevMode);
	ASSERT(ppstrPrinter && *ppstrPrinter);
	ASSERT(ppstrDriver && *ppstrDriver);
	ASSERT(ppstrPort && *ppstrPort);
	
	delete *ppDevMode;
	*ppDevMode = NULL;
	
	delete [] *ppstrPrinter;
	*ppstrPrinter = NULL;
	
	delete [] *ppstrDriver;
	*ppstrDriver = NULL;

	delete [] *ppstrPort;
	*ppstrPort = NULL;
}

// Allocates a copy of the given DEVMODE object
// Returns a handle to the globally allocated memory, make sure you store the return value, so that you can call GlobalFree on it later
HGLOBAL AllocDevModeCopy(IN const DEVMODE *pDevModeIn)
{
    // Allocate a global handle for the data
    HGLOBAL hDevModeOut = GlobalAlloc(GHND, sizeof(*pDevModeIn) + pDevModeIn->dmDriverExtra);
    ASSERT(hDevModeOut);
    DEVMODE * pDevModeOut = (DEVMODE *)GlobalLock(hDevModeOut);
    ASSERT(pDevModeOut);

    // Copy the data
    CopyDevMode(pDevModeIn, pDevModeOut);

	// Unlock the global handle
    GlobalUnlock(hDevModeOut);

	// Return the global handle to the new devmode object
	return hDevModeOut;
}

// Allocates a DEVNAMES object, filling it with the given strings (printer(device), driver(driver), and port(output))
// Returns a handle to the globally allocated memory, make sure you store the return value, so that you can call GlobalFree on it later
HGLOBAL AllocDevNamesCopy(IN LPCTSTR strPrinter, IN LPCTSTR strDriver, IN LPCTSTR strPort)
{
	//TES 11/7/2007 - PLID 27979 - VS2008 - We assign these to WORDs, so declare them as WORDs
	// (d.thompson 2015-10-14 11:28) - PLID 67352 - strlen() returns a size_t, not a WORD.  Need to explicitly cast since we are truncating.
	WORD nPrinterLen = (WORD)strlen(strPrinter);
	WORD nDriverLen = (WORD)strlen(strDriver);
	long nPortLen = strlen(strPort);

	HGLOBAL hDevNamesOut = GlobalAlloc(GHND, sizeof(DEVNAMES) + nPrinterLen+1 + nDriverLen+1 + nPortLen+1);
	ASSERT(hDevNamesOut);
	
	DEVNAMES *pDevNamesOut = (DEVNAMES *)GlobalLock(hDevNamesOut);
	ASSERT(pDevNamesOut);

	// Set the offsets
	pDevNamesOut->wDeviceOffset = sizeof(DEVNAMES);
	pDevNamesOut->wDriverOffset = sizeof(DEVNAMES) + nPrinterLen+1;
	pDevNamesOut->wOutputOffset = sizeof(DEVNAMES) + nPrinterLen+1 + nDriverLen+1;
	pDevNamesOut->wDefault = 0;
	
	// Copy the strings into the memory at those offsets
	LPTSTR strBase = (LPTSTR)pDevNamesOut;
	strcpy(strBase + pDevNamesOut->wDeviceOffset, strPrinter);
	strcpy(strBase + pDevNamesOut->wDriverOffset, strDriver);
	strcpy(strBase + pDevNamesOut->wOutputOffset, strPort);

	// Unlock the global handle
	GlobalUnlock(hDevNamesOut);

	// Return the global handle to the new moveable DEVNAMES object
	return hDevNamesOut;
}

bool ContainsDigit(CString strIn) {

	bool bDigitFound = false;

	for (int i =0; i < strIn.GetLength(); i++) {

		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (isdigit(unsigned char(strIn.GetAt(i))) != 0) {
			bDigitFound = true;
		}

	}
	return bDigitFound;
}

BOOL ContainsNonDigits(LPCTSTR str)
{
	for (const TCHAR *p = str; ; p++) {
		switch (*p) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			// Digit char so keep looping
			break;
		case '\0':
			// We're done and we encountered no non-digit chars
			return FALSE;
			break;
		default:
			// Non-digit char
			return TRUE;
			break;
		}
	}
}

BOOL IsScreen640x480()
{
	int iWidth = GetSystemMetrics(SM_CXSCREEN);
	int iHeight = GetSystemMetrics(SM_CYSCREEN);
	return ((iWidth == 640) && (iHeight == 480));
}

//registry functions
CString GetRegistryBase() {
	return g_strRegistryBase;
}

CString GetRegistryBase(CString strDatabase)
{
	//
	// (c.haag 2005-12-27 10:28) - PLID 17833 - Use the database name
	// to determine the proper registry base. If it's pracdata or blank,
	// we will pull the default base. Otherwise, we will pull it as a
	// function of the database name
	//
	CString strRegBase;
	if (!strDatabase.IsEmpty() && strDatabase.CompareNoCase("pracdata")) {
		strDatabase.MakeUpper();
		strDatabase.Replace("PRACDATA_", "");
		strRegBase.Format("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\%s\\", (LPCSTR)strDatabase);
	}
	else
		strRegBase.Format("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\");
	return strRegBase;
}

//this will ALWAYS append on to HKEY_LOCAL_MACHINE\SOFTWARE\Nextech, so that all registry settings we use will go under the nextech key
//it will also always end in a backslash
void SetRegistryBase(CString strBase) {

	g_strRegistryBase = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\";
	g_strRegistryBase += strBase;

	//make sure it ends in a backslash
	if(g_strRegistryBase.Right(1) != "\\")
		g_strRegistryBase += "\\";
}

//this will return the subkey that we are currently using, returns an empty string if we're in the main nextech key
CString GetSubRegistryKey() {

	if(GetRegistryBase() == "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\")
		return "";

	CString str;
	str = GetRegistryBase();
	str.Replace("HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\", "");
	str.TrimRight("\\");

	return str;
	
}

CString GetCurrentDatabaseName()
{
	CString strDatabase;
	if(GetSubRegistryKey().IsEmpty()) {
		strDatabase = "PracData";
	}
	else {
		strDatabase.Format("PracData_%s", GetSubRegistryKey());
	}

	return strDatabase;
}

//DRT 7/20/2004 - PLID 13487 - Get the name of the database we are using to purge to.  If 
//	the bCreateIfNeeded argument is true, we will create the database (if it doesn't exist)
//	and return that name.  If that argument is false, an empty string will be returned if
//	there is no purge database.  The bCreatedNew parameter tells the calling function
//	that we created this database as new, and we need to add it to NxServer.
CString GetPurgeDatabaseName(bool &bCreatedNew, bool bCreateIfNeeded /*= true*/)
{
	bCreatedNew = false;

	try {
		//Our target name is PracData_<subkey>_Purge
		CString strTarget;
		strTarget.Format("PracData_%s_Purge", GetSubRegistryKey());
		strTarget.Replace("__", "_");	//if there is no registry key, change PracData__Purge to PracData_Purge

		//Now see if this database is attached
		//Open a connection to the 'master' database on this same SQL server

		// (a.walling 2011-09-07 18:01) - PLID 45448 - NxAdo unification
		//CNxAdoConnection masterConn = GetRemoteConnection();
		//masterConn.SetDatabase("master");

		//_ConnectionPtr pMasterConn = masterConn.GetRemoteData();

		//{
		//	// (a.walling 2010-06-02 07:49) - PLID 31316 - Standard database connection string
		//	CString strConn = GetStandardConnectionString(true, "master");

		//	pMasterConn->Open(_bstr_t(strConn), "", "", 0);

		//	if(pMasterConn == NULL || pMasterConn->GetState() == adStateClosed)
		//		//If we couldn't open the master database, that's a major problem
		//		return "";
		//}

		//Now see if our target is attached
		// (a.walling 2012-02-18 23:41) - PLID 48230 - We don't need a new connection; we can query the master tables directly
		bool bAttached = false;
		{
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT 1 FROM master..sysdatabases WHERE name = {STRING}", strTarget);
			if(!prs->eof)
				bAttached = true;
			prs->Close();
		}

		//Generate the target location on disk by looking at our current database
		CString strTargetLocMdf = "";
		CString strTargetLocLdf = "";
		{
			CString strDB;
			if(GetSubRegistryKey().IsEmpty())
				strDB = "PracData";
			else 
				strDB.Format("PracData_%s", GetSubRegistryKey());
			
			// (a.walling 2012-02-18 23:41) - PLID 48230 - We don't need a new connection; we can query the master tables directly
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT * FROM master..sysdatabases WHERE name = {STRING}", strDB);
			if(!prs->eof) {
				CString strLoc = AdoFldString(prs, "filename", "");

				//Now we should have something like, 'C:\PracStation\data\MainTesting\PracData_main.mdf'
				int nSlash = strLoc.ReverseFind('\\');
				if(nSlash > -1) {
					strTargetLocMdf = strLoc.Left(nSlash);
					strTargetLocMdf.TrimRight("\\");
					strTargetLocLdf = strTargetLocMdf;
					strTargetLocMdf += "\\" + strTarget + ".mdf";
					strTargetLocLdf += "\\" + strTarget + ".ldf";
				}
				else
					return "";	//Failure here, we cannot continue
			}
			else
				return "";	//Failure here, we cannot continue

			prs->Close();
		}

		//If the file does not exist, create a database at the target location
		if(!bAttached) {
			//Since this could be (and likely is) a database on a separate computer, we 
			//	cannot just do a simple file test to see if the file exists previous to 
			//	creating it.  Just try to do the create, and if we get an appropriate
			//	exception, we will catch it and know that the files are already there!
			//This is not a case that should really ever come up, unless the database
			//	somehow was detached and left in the same spot.
			
			// (a.walling 2012-02-18 23:41) - PLID 48230 - We don't need a new connection; we can query the master tables directly
			try {
				ExecuteSql("CREATE DATABASE %s ON "
					"(NAME = %s_DAT, "
					"Filename = '%s', "
					"SIZE = 10, "
					"Filegrowth = 20 )"
					"LOG ON "
					"(NAME = '%s_log', "
					"FILENAME = '%s', "
					"SIZE = 5, "
					"FILEGROWTH = 5MB ) ", strTarget, strTarget, strTargetLocMdf, strTarget, strTargetLocLdf);

				bAttached = true;	//Executing this command actually attaches the database as well as creating it
				bCreatedNew = true;

			} catch(_com_error& e) {
				if(e.Error() == -2147217900) {
					//This means the files are there, but not attached for some reason.  We can just carry on
					//	and let the code below handle attaching them, pretending like it was created.
				}
				else 
					throw e;
			}
		}

		if(!bAttached) {
			//now we need to attach the database
			// (a.walling 2012-02-18 23:41) - PLID 48230 - We don't need a new connection; we can query the master tables directly
			ExecuteSql("EXEC sp_attach_db %s, '%s', '%s'", strTarget, strTargetLocMdf, strTargetLocLdf);

			bAttached = true;	//it's not attached
			bCreatedNew = true;	//try it here as well, just in case
		}

		//pMasterConn->Close();

		//At this point, we have a file that exists and is attached, whether we created it or it was already there.
		//	Return the name of the target database
		return strTarget;

	} NxCatchAll("Error determining purge database.");

	//If we got any errors, we can't return anything
	return "";
}

int FindWholeWord(IN const CString &strString, IN const CString &strFind, IN long nStart) {

	if (strString.IsEmpty()) {
		//this should never happen because why would you search a blank string??
		ASSERT(FALSE);
		return 0;
	}

	long nResult;
	char chCheck;
	long nPosition = nStart;
	bool bNeedCheckBeginning = TRUE, bNeedCheckEnd = TRUE;
	bool bBeginningGood = FALSE, bEndGood = FALSE;

	long nFindLength = strFind.GetLength();
	long nStringLen = strString.GetLength();
	long nEnd;
	
	while (true) {

		//find the search string
		nResult = strString.Find(strFind, nPosition);
		if (nResult == -1 ) {
			break;
		} else {
			nEnd = nResult + nFindLength;
		}		

		// If we found it at the very beginning of the string, then we won't need to check
		if (nResult == 0) {
			bNeedCheckBeginning = FALSE;
			bBeginningGood = TRUE;
		}
		else {
			bNeedCheckBeginning = TRUE;
		}

		// If the string we found ends at the very end of the given string, then there is 
		// no character following it so there's no character to check
		ASSERT(nEnd <= nStringLen);
		if (nEnd == nStringLen) {
			bNeedCheckEnd = FALSE;
			bEndGood = TRUE;
		} else {
			bNeedCheckEnd = TRUE;
		}


		if (bNeedCheckBeginning) {

			//we aren't at the beginning, so check the spot before this one
			chCheck = strString.GetAt(nResult - 1);
			// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
			if (isalnum(unsigned char(chCheck)) || chCheck == '_') {
				//its no good, keep going
				nPosition = nResult + 1;
				continue;
			}
			else {
				bBeginningGood = TRUE;
			}

		}
		
		if (bNeedCheckEnd) {
			
			chCheck = strString.GetAt(nEnd);
			// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
			if (isalnum(unsigned char(chCheck)) || chCheck == '_') {
					//no good, keep going
					nPosition = nResult + 1;
			}
			else {
				bEndGood = TRUE;
			}
		}

		
		if (bBeginningGood && bEndGood) {

			return nResult;
		}
		
	}

	//if we got here, we didn't find the whole word
	return -1;

}

int ReplaceWholeWords(IN OUT CString &strString, IN const CString &strReplaceText, IN const CString &strWithText)
{
	if (strString.IsEmpty()) {
		//this should never happen because why would you search a blank string??
		ASSERT(FALSE);
		return 0;
	}
	else if (strString.GetLength() < strReplaceText.GetLength()) {
		//this should also never happen because you shouldn't be searching for text that is longer than what you are searching
		//ASSERT(FALSE);
		return 0;
	}


	CString strOut;
	long nResult = 0, nPosition = 0;
	long nCount = 0;
	long nReplaceLength = strReplaceText.GetLength();
	long nStringLength = strString.GetLength();

	while (nResult != -1) {

		nResult = FindWholeWord(strString, strReplaceText, nPosition);

		if (nResult == -1) {
			strOut += strString.Right(nStringLength - ((nPosition == 0) ? nPosition : nPosition + 1));
			strString = strOut;
			return nCount;
		}
		else {

			strOut += strString.Mid(nPosition, (nResult - nPosition)) +  " " + strWithText + " ";
			nCount++;
			nPosition = nResult + nReplaceLength;
		}

	}

	strOut += strString.Right(nStringLength - ((nPosition == 0) ? nPosition : nPosition + 1));
	strString = strOut;
	return nCount;	



}

// (a.walling 2007-07-25 13:33) - PLID 17467 - Fixed this function also by using the shared IsImageFile SQL function
int GetPatientAttachedImageCount(long nPatientID)
{
	try {

		// (j.jones 2008-06-13 11:00) - PLID 30388 - filtered only on photos (or unknown), and parameterized
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(MailID) AS CountPhotos "
			"FROM MailSent "
			"WHERE PersonID = {INT} "
			"AND dbo.IsImageFile(PathName) = 1 "
			"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
			"AND {SQLFRAGMENT}", nPatientID, GetAllowedCategoryClause_Param("CategoryID"));
		if(!rs->eof) {
			return AdoFldLong(rs, "CountPhotos", 0);
		}
		else {
			return 0;
		}
		rs->Close();
		
	} NxCatchAll("Error getting patient image count");
	return 0;
}

// This is declared in history.cpp. Too bad; it seems like
// a nice global function.
//TES 3/17/2004: I agree with the above comment, but rather than just lamenting the fact that the function was in the wrong file,
//I moved it.  And if you don't mind my saying so: duh!
// (j.gruber 2010-04-22 13:51) - PLID 38262 - add a silent parameter
CString GetPatientDocumentPath(long nPatientID, BOOL bSilent /*=FALSE*/)
{
	//TES 9/18/2008 - PLID 31413 - This now just calls the HistoryUtils function (which is shared with NxServer).
	return GetPatientDocumentPath(GetRemoteData(), nPatientID, bSilent);
}

//TES 9/18/2008 - PLID 31413 - The other GetPatientDocumentPath() overload was moved to HistoryUtils (which is shared with NxServer)

//TES 6/1/2004: Modified this to load the THUMBNAIL version of the image.
HBITMAP LoadPatientAttachedImage(long nPatientID, long nIndex, CWnd* pImageProgressCtrl, /*OUT*/ CString& strFileName)
{
	try {
		_RecordsetPtr prs;
		
		if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0)) {
			// (j.jones 2008-06-13 11:00) - PLID 30388 - filtered only on photos (or unknown), and parameterized
			//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
			prs = CreateParamRecordset("SELECT MailID, PathName "
				"FROM MailSent "
				"INNER JOIN PatientsT ON PatientsT.PatPrimaryHistImage = MailSent.MailID "
				"WHERE PatientsT.PersonID = {INT} "
				"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
				"AND {SQLFRAGMENT}", nPatientID, GetAllowedCategoryClause_Param("CategoryID"));
		}
		else {
			// (a.walling 2006-10-20 09:01) - PLID 22991 - Might as well include .jpeg while i am at it.
			// (a.walling 2007-07-25 13:35) - PLID 17467 - Use shared IsImageFile SQL Function
			// (j.jones 2008-06-13 11:00) - PLID 30388 - filtered only on photos (or unknown), and parameterized
			//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
			prs = CreateParamRecordset("SELECT MailID, PathName "
				"FROM MailSent "
				"WHERE PersonID = {INT} "
				"AND dbo.IsImageFile(PathName) = 1 "
				"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
				"AND {SQLFRAGMENT} "
				"ORDER BY [Date] Desc", nPatientID, GetAllowedCategoryClause_Param("CategoryID"));
			prs->Move(nIndex);
		}
		if (prs->eof)
			return NULL;

		CString strDoc;
		HBITMAP hBitmap = NULL;

		//Load the correct version of the filename.
		strFileName = GetPatientDocumentPath(nPatientID) ^ AdoFldString(prs, "PathName");
		if (!DoesExist(strFileName))
		{
			// The file failed to open. Lets try opening with just the path name
			// by itself.
			strFileName = AdoFldString(prs, "PathName");
			if (!DoesExist(strFileName))
			{
				return NULL;
			}
		}

		// (a.walling 2006-10-20 09:07) - PLID 22991 - load the image thumbnail automagically with type detection
		CxImage xImg;
		CString strFileStream = strFileName + ":NxThumb";
		// (a.walling 2006-11-01 11:31) - PLID 23310 - Mod date checking and rewriting is all handled in loadbitmapfromfile
		return LoadBitmapFromFile(strFileStream);

		/*
		bool bThumbValid = xImg.Load(strFileStream, CXIMAGE_FORMAT_UNKNOWN);
		if (bThumbValid)
		{
			return xImg.MakeBitmap();
		}
		else if(WriteThumbnailToMailSent(AdoFldLong(prs, "MailID"))) {
			if(xImg.Load(strFileStream, CXIMAGE_FORMAT_UNKNOWN)) {
				return xImg.MakeBitmap();
			}
			else { // still failed!
				return NULL;
			}
		}
		else {
			return NULL;
		}
		*/

		/*
		// (a.walling 2006-10-09 12:21) - PLID 22689 - Modified to support loading different thumbnail types.
		_variant_t varThumb = prs->Fields->GetItem("Thumbnail")->Value;
		long nThumbBitDepth = AdoFldLong(prs, "ThumbBitDepth", 0);
		long nThumbType = AdoFldLong(prs, "ThumbType", 0);
		HBITMAP hThumb;
		BOOL bDepthIncreased = FALSE;
		CBorrowDeviceContext bdc(AfxGetMainWnd()->GetDC());
		//Note the cunning exploitation of short-circuited || logic.
		if(varThumb.vt == VT_NULL || nThumbBitDepth < GetDeviceCaps(bdc.m_pDC->GetSafeHdc(), BITSPIXEL) 
			|| !LoadVariantAsBitmap(bdc.m_pDC, varThumb, hThumb, nThumbType)) {
			if(!WriteThumbnailToMailSent(AdoFldLong(prs, "MailID"))) {
				return NULL;
			}
			else {
				prs->Close();
				if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0))
					prs = CreateRecordset("SELECT MailID, PathName, Thumbnail, ThumbType FROM MailSent INNER JOIN PatientsT ON PatientsT.PatPrimaryHistImage = MailSent.MailID WHERE PatientsT.PersonID = %d", nPatientID);
				else
				{
					prs = CreateRecordset("SELECT MailID, PathName, Thumbnail, ThumbType FROM MailSent WHERE PersonID = %d AND (Right([PathName],3) = 'jpg' OR Right([PathName],3) = 'bmp') ORDER BY [Date] Desc", nPatientID);
					prs->Move(nIndex);
				}
				if(prs->eof) {
					return NULL;
				}
				varThumb = prs->Fields->GetItem("Thumbnail")->Value;
				nThumbType = AdoFldLong(prs, "ThumbType", 0);
				if(varThumb.vt == VT_NULL || !LoadVariantAsBitmap(bdc.m_pDC, varThumb, hThumb, nThumbType)) {
					return NULL;
				}
			}
		}
		//We've now either returned, or we have the thumbnail loaded.
		return hThumb;
		*/

		/*LPPICTURE gpPicture = NULL;
		LPVOID pvData = NULL;

		DWORD dwFileSize = pFile->GetLength();

		// alloc memory based on file size
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwFileSize);
		_ASSERTE(NULL != hGlobal);

		pvData = GlobalLock(hGlobal);
		_ASSERTE(NULL != pvData);

		DWORD dwBytesRead = 0;
		// read file and store in global memory
		pFile->Read(pvData, dwFileSize);
		GlobalUnlock(hGlobal);

		LPSTREAM pstm = NULL;
		// create IStream* from global memory
		HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pstm);
		_ASSERTE(SUCCEEDED(hr) && pstm);

		// Create IPicture from image file
		if (gpPicture)
			gpPicture->Release();
		hr = ::OleLoadPicture(pstm, dwFileSize, FALSE, IID_IPicture, (LPVOID *)&gpPicture);
		_ASSERTE(SUCCEEDED(hr) && gpPicture);	
		pstm->Release();


#define HIMETRIC_INCH	2540

		HBITMAP hbmpDC = NULL;
		if (gpPicture)
		{
			// get width and height of picture
			long hmWidth;
			long hmHeight;
			gpPicture->get_Width(&hmWidth);
			gpPicture->get_Height(&hmHeight);

			// convert himetric to pixels
			int nWidth	= MulDiv(hmWidth, GetDeviceCaps(GetMainFrame()->GetDC()->m_hDC, LOGPIXELSX), HIMETRIC_INCH);
			int nHeight	= MulDiv(hmHeight, GetDeviceCaps(GetMainFrame()->GetDC()->m_hDC, LOGPIXELSY), HIMETRIC_INCH);



			/////////////////////////////////////////////////////////////////
			// Draw the image
			static CDC dcMem;
			hbmpDC = CreateCompatibleBitmap(GetMainFrame()->GetDC()->m_hDC, nWidth, nHeight);
			if (dcMem.m_hDC == NULL)
				dcMem.CreateCompatibleDC(NULL);
			HBITMAP hbmpOld = (HBITMAP)SelectObject(dcMem.m_hDC, hbmpDC);


			RECT rc;
			rc.top = rc.left = 0;
			rc.right = nWidth;
			rc.bottom = nHeight;

			// display picture using IPicture::Render
			gpPicture->Render(dcMem.m_hDC, 0, 0, nWidth, nHeight, 0, hmHeight, hmWidth, -hmHeight, &rc);

			SelectObject(dcMem.m_hDC, hbmpOld);

			// Cleanup
			gpPicture->Release();
		}

		pFile->Close();
		delete pFile;
		return hbmpDC;*/

	} catch(CNxPersonDocumentException *pException) {
		// (d.thompson 2013-07-01) - PLID 13764 - Specially handle exceptions regarding the person documents
		CPatientDocumentStorageWarningDlg dlg(pException->m_strPath);
		//No longer need the exception, clean it up
		pException->Delete();

		//Inform the user
		dlg.DoModal();
	} NxCatchAll("Error loading patient image");
	return NULL;
}

void AttachToMailSent(long nPersonID, const CString &strPathName, BOOL bAttachChecksum /*= TRUE*/, long nChecksum /*= 0*/, LPCTSTR strSelection /*= SELECTION_FILE*/, long nCategoryID /*= -1*/, long nLabStepID /*= -1*/, long nPicID /*= -1*/)
{
	// Insert the record in MailSent
	// (c.haag 2003-10-24 10:07) - Let the caller deal with thrown exceptions.
	//(e.lally 2008-08-05) PLID 30950 - Don't set the new ID until we are actually ready to use it. 
	long nMailSentID = -1;

	if(!bAttachChecksum) {
		nChecksum = -1;
	}
		
	// (a.walling 2008-11-05 10:05) - PLID 31486 - Make sure we remove the path if we actually are using the document path
	CString strSimplifiedPathName;
	if (GetFilePath(strPathName).CompareNoCase(GetPatientDocumentPath(GetRemoteData(), nPersonID)) == 0) {
		strSimplifiedPathName = GetFileName(strPathName);
	} else {
		strSimplifiedPathName = strPathName;
	}

	// (a.wetta 2007-07-09 16:49) - PLID 17467 - If this file is an image, determine if it should be added as a photo
	BOOL bIsPhoto = FALSE;
	// If this is the non-person ID, then this file can't be in a photos tab anyway or the history tab for that matter
	// (a.walling 2007-07-25 15:53) - PLID 17467 - Should probably set the bool here...
	if (nPersonID != -25) {
		bIsPhoto = IsHistoryImageAPhoto(strPathName);
	}

	// (j.jones 2008-09-04 15:28) - PLID 30288 - converted to use CreateNewMailSentEntry,
	// which creates the data in one batch and sends a tablechecker
	// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
	// (d.singleton 2013-11-15 11:18) - PLID 59513 - need to insert the CCDAType when generating a CCDA
	COleDateTime dtNull;
	dtNull.SetStatus(COleDateTime::null);
	CreateNewMailSentEntry(nPersonID, strSimplifiedPathName, strSelection, strSimplifiedPathName, GetCurrentUserName(), "", GetCurrentLocationID(), dtNull, nChecksum, nCategoryID, nPicID, nLabStepID, bIsPhoto, -1, "", ctNone);

	// (a.walling 2008-09-03 11:42) - PLID 19638 - need to send an EMR tablechecker if in a PIC that has an EMR
	if (nPicID != -1) {
		CClient::RefreshTable(NetUtils::EMRMasterT, nPersonID);
	}
}

static CString strTmpPath;
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	TCHAR szDir[MAX_PATH];
	switch(uMsg){
	case BFFM_INITIALIZED:
		if (lpData){
			strcpy(szDir, strTmpPath.GetBuffer(strTmpPath.GetLength()));
			SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);
		}
		break;
	case BFFM_SELCHANGED: {
	   if (SHGetPathFromIDList((LPITEMIDLIST) lParam ,szDir)){
		  SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
	   }
	   break;
	}
	default:
	   break;
	}
	return 0;
}

// (v.maida 2016-06-02 17:53) - NX-100805 - Added option bAllowNewFolder option, which defaults to true since this function used to always provide that option.
BOOL BrowseToFolder(CString* strSelectedFolder,
				   const char* lpszTitle,
				   const HWND hwndOwner, 
				   const char* strRootFolder, 
				   const char* strStartFolder,
	               const bool bAllowNewFolder /*= true*/)
{
	char pszDisplayName[MAX_PATH];
	LPITEMIDLIST lpID;
	BROWSEINFOA bi;
	
	bi.hwndOwner = hwndOwner;
	if (strRootFolder == NULL){
		bi.pidlRoot = NULL;
	}else{
	   LPITEMIDLIST  pIdl = NULL;
	   IShellFolder* pDesktopFolder;
	   char          szPath[MAX_PATH];
	   OLECHAR       olePath[MAX_PATH];
	   ULONG         chEaten;
	   ULONG         dwAttributes;

	   strcpy(szPath, (LPCTSTR)strRootFolder);
	   if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	   {
		   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szPath, -1, olePath, MAX_PATH);
		   pDesktopFolder->ParseDisplayName(NULL, NULL, olePath, &chEaten, &pIdl, &dwAttributes);
		   pDesktopFolder->Release();
	   }
	   bi.pidlRoot = pIdl;
	}
	bi.pszDisplayName = pszDisplayName;
	bi.lpszTitle = lpszTitle;
	// (v.maida 2016-06-02 17:53) - NX-100805 - Use the bAllowNewFolder parameter to determine whether or not the caller will have the option to make new folders.
	UINT nCreateNewFolder = bAllowNewFolder ? 0 : BIF_NONEWFOLDERBUTTON;
	// (j.jones 2013-03-01 16:40) - PLID 55120 - added BIF_NEWDIALOGSTYLE to users could make new folders from this dialog
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE | nCreateNewFolder;
	bi.lpfn = BrowseCallbackProc;
	if (strStartFolder == NULL){
		bi.lParam = FALSE;
	}else{
		strTmpPath.Format("%s", strStartFolder);
		bi.lParam = TRUE;
	}
	bi.iImage = NULL;
	lpID = SHBrowseForFolderA(&bi);
	if (lpID != NULL){
		BOOL b = SHGetPathFromIDList(lpID, pszDisplayName);
		if (b == TRUE){
			strSelectedFolder->Format("%s",pszDisplayName);
			return TRUE;
		}
	}
	return FALSE;
}

//takes in an array of field names, and a recordset source, and returns an XML markup of the information
CString CreateXML(CStringArray &aryFieldNames, ADODB::_RecordsetPtr pRS, CString strKeyFieldName /*= ""*/, BOOL bFilterNulls /*= FALSE*/) {

	CString strXml = "";

	// Start at a string length of 2^17 (we'll double this if we go out of bounds)
	long nMaxLen = 1<<17;
	long nLen = 0;
	long nSubLen;
	CString str;
	TCHAR *pstr = strXml.GetBuffer(nMaxLen);
	nSubLen = sprintf(pstr, "<ROOT>");
	pstr += nSubLen;
	nLen += nSubLen;

	// Loop through the recordset
	if (!(pRS->eof && pRS->bof)) {
		//move it back to the beginning of the file
		pRS->MoveFirst();
	}

	//last key field value, store to ensure no duplicates (assumes that the key is what you are sorting on)
	CString strLastKeyData = "";

	while(!pRS->eof) {

		BOOL bSkip = FALSE;
			
		// Create a string that will represent the <RECORD <Field1Name>=<Field1Data> <Field2Name>=<Field2Data>... /> part 
		// of our XML (we use "P" instead of "RECORD" because this could be 
		// a huge string and we want it to be as efficient as possible)

		str = "<P ";
				
		for(int i=0;i<aryFieldNames.GetSize();i++) {

			CString fieldName = aryFieldNames.GetAt(i);

			_variant_t var = pRS->Fields->Item[_bstr_t(fieldName)]->Value;
			if(var.vt == VT_NULL && bFilterNulls) {
				bSkip = TRUE;
			}

			//now make a string version of this data
			CString strData = AsStringForSql(var);

			//if a keyfield was specified, check it for a duplicate
			if(fieldName == strKeyFieldName) {
				if(strLastKeyData == strData)
					bSkip = TRUE;
				else
					strLastKeyData = strData;
			}

			//check to see that the string does not contain and invalid characters and replace them if it does
			strData.Replace("&", "And");
			strData.Replace("<", " ");
			strData.Replace(">", " ");
			strData.Replace("\"", " ");
		
			//now let's create our data type
			CString strTemp;
			strTemp.Format("%s=\"%s\" ",fieldName,strData);
			str += strTemp;
		}

		str += "/>";

		if(!bSkip) {

			//if we are skipping, this data won't be appended

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
		}

		pRS->MoveNext();
	}

	// Close the XML
	strXml.ReleaseBuffer();
	strXml += "</ROOT>";

	return strXml;
}

//DRT 10/6/2005 - PLID 17549 - See notes below
CMutex g_mtxTempTable;
unsigned long g_nTempTableIndex = 0;

//creates a temp table from an XML string, and returns the new table name
// (j.jones 2008-10-01 17:09) - PLID 31563 - added optional pstrSqlBatch parameter to return
// SQL statements in batch form, instead of executing immediately
CString CreateTempTableFromXML(CStringArray &aryFieldNames, CStringArray &aryFieldTypes, CString strXML, CString *pstrSqlBatch /*= NULL*/) {

	// Create a local temporary table (local means it only exists 
	// in the context of this connection, and temporary means it 
	// will be deleted as soon as this connection closes) and fill it

	ASSERT(aryFieldNames.GetSize() == aryFieldTypes.GetSize());

	//DRT 10/6/2005 - PLID 17549 - It is possible to call this function twice in succession fast enough
	//	that GetTickCount() returns the same value!  So I'm semi-overhauling this function so that we
	//	maintain a global integer of 
	g_mtxTempTable.Lock();

	// Unique temp table name within this connection (other connections have their own names)
	CString strTempT;
	strTempT.Format("#Temp%luT", g_nTempTableIndex++);

	//Unlock the mutex
	g_mtxTempTable.Unlock();

	//begin to create our temp table, which is comprised of a rownumber and our passed in fields
	CString str, strSQL = "", strXMLInsert = "";

	for(int i=0;i<aryFieldNames.GetSize();i++) {
		CString fieldName = aryFieldNames.GetAt(i);
		CString fieldType = aryFieldTypes.GetAt(i);
		str.Format("%s %s,",fieldName,fieldType);
		strSQL += str;
		str.Format("%s,",fieldName);
		strXMLInsert += str;
	}
	strSQL.TrimRight(",");
	strXMLInsert.TrimRight(",");

	// (j.jones 2008-10-01 17:10) - PLID 31563 - batch these two statements, always
	CString strSqlBatch;
	AddStatementToSqlBatch(strSqlBatch, "CREATE TABLE %s (RowNumber INT IDENTITY, %s)", strTempT,strSQL);
	
	// Add the rows from the XML into our temp table
	AddStatementToSqlBatch(strSqlBatch,
		"DECLARE @hDoc AS INT; "  // We need a document handle
		"EXEC sp_xml_preparedocument @hDoc OUTPUT, N'%s'; " // Ask SQL to parse the XML (returns the document handle)
		"INSERT INTO %s (%s) SELECT * FROM OPENXML(@hDoc, '/ROOT/P') WITH (%s); " // Insert into the table
		"EXEC sp_xml_removedocument @hDoc;",  // Release our document handle
		strXML, strTempT, strXMLInsert, strSQL);

	// (j.jones 2008-10-01 17:12) - PLID 31563 - now, if we have the pstrSqlBatch parameter,
	// simply add our batch to that one (not using the AddStatement function, it's already formatted)
	// but if we don't, execute the batch now
	if(pstrSqlBatch) {
		(*pstrSqlBatch) += strSqlBatch;
	}
	else {
		long nRecordsAffected = 0;
		//changed nMaxRecordsAffected to 10k, since creating an XML table
		//doesn't really change "real" data
		// (a.walling 2010-10-20 09:41) - PLID 34813 - Use XACT_ABORT!
		
		// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
		NxAdo::PushMaxRecordsWarningLimit pmr(10000);
		ExecuteSqlStd(
			TransactionUtils::Wrap(strSqlBatch),
			&nRecordsAffected, adCmdText);
	}

	// Return the name of the temp table
	return strTempT;
}

//Appends to a table created by CreateTempTableFromXML
// (j.jones 2008-10-01 17:09) - PLID 31563 - added optional pstrSqlBatch parameter to return
// SQL statements in batch form, instead of executing immediately
void AppendToTempTableFromXML(CStringArray &aryFieldNames, CStringArray &aryFieldTypes, CString strXML, const CString &strTable, CString *pstrSqlBatch /*= NULL*/) 
{
	ASSERT(aryFieldNames.GetSize() == aryFieldTypes.GetSize());
	
	//begin to create our temp table, which is comprised of a rownumber and our passed in fields
	CString str, strSQL = "", strXMLInsert = "";

	for(int i=0;i<aryFieldNames.GetSize();i++) {
		CString fieldName = aryFieldNames.GetAt(i);
		CString fieldType = aryFieldTypes.GetAt(i);
		str.Format("%s %s,",fieldName,fieldType);
		strSQL += str;
		str.Format("%s,",fieldName);
		strXMLInsert += str;
	}
	strSQL.TrimRight(",");
	strXMLInsert.TrimRight(",");

	// Add the rows from the XML into our temp table
	CString strSqlExecute, strDeclare = "";
	if(pstrSqlBatch == NULL || pstrSqlBatch->Find("DECLARE @hDoc AS INT") == -1) {
		//only add @hDoc if it is not already defined
		strDeclare = "DECLARE @hDoc AS INT; "; // We need a document handle
	}
	strSqlExecute.Format(
		"%s"
		"EXEC sp_xml_preparedocument @hDoc OUTPUT, N'%s'; " // Ask SQL to parse the XML (returns the document handle)
		"INSERT INTO %s (%s) SELECT * FROM OPENXML(@hDoc, '/ROOT/P') WITH (%s); " // Insert into the table
		"EXEC sp_xml_removedocument @hDoc;",  // Release our document handle
		strDeclare, strXML, strTable, strXMLInsert, strSQL);

	// (j.jones 2008-10-01 17:12) - PLID 31563 - now, if we have the pstrSqlBatch parameter,
	// simply add this statement to our batch, but if we don't, execute the statement now
	if(pstrSqlBatch) {
		//format as %s, just incase, we don't want to parse this sql
		AddStatementToSqlBatch(*pstrSqlBatch, "%s", strSqlExecute);
	}
	else {
		long nRecordsAffected = 0;
		//changed nMaxRecordsAffected to 10k, since creating an XML table
		//doesn't really change "real" data
		// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
		NxAdo::PushMaxRecordsWarningLimit pmr(10000);
		ExecuteSqlStd(strSqlExecute, &nRecordsAffected, adCmdText);
	}
}

// CAH 5/15/03 TODO - Find a better place for this function
BOOL IsUnitedEnabled()
{
	//(e.lally 2011-08-26) PLID 44950 - Auto-create the pref so we don't have the added round-trip the first time every login.
	if (GetPropertyInt("UnitedDisable", 0, 0, true))
		return FALSE;
	if (!GetMainFrame()->GetUnitedLink()->IsLinkEnabled())
		return FALSE;
	return TRUE;
}

DWORD TryNetworkLogin(const CString& strNetworkPath)
{
	//
	// (c.haag 2006-05-03 08:27) - PLID 4612 - Did a bit of a rewrite here
	//
	char szNetworkPath[MAX_PATH];
	DWORD nSize = 512;
	DWORD nResult;
	NETRESOURCE res = {
		0, RESOURCETYPE_DISK, RESOURCEDISPLAYTYPE_GENERIC, 
			RESOURCEUSAGE_CONNECTABLE, NULL,
			szNetworkPath, "", NULL
	};
	strncpy(szNetworkPath, strNetworkPath, MAX_PATH);

	do {
		//
		// Try to connect the way we are right now. By passing in NULL for the username
		// and password, we are forcing the function to look at the user context for
		// both those pieces of information.
		//
		// In our switch statement, we break if the error is unrelated to the user profile.
		// Otherwise, we fail outright because the problem is with the network. 
		//
		switch (nResult = WNetAddConnection2(&res, NULL, NULL, 0)) {
			case NO_ERROR:							// We have an established connection, but
													// that doesn't mean we can access the file.
													// Call GetFileAttributes to see whether the
													// problem is due to a logon failure or not. If
													// it is, take the user to the interactive prompt.
													// Otherwise, fail out here.
				if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strNetworkPath)) {
					if (ERROR_LOGON_FAILURE == GetLastError()) {
						break;
					}
				}
				return nResult;

			case ERROR_BAD_NET_NAME:				// The value specified by the lpRemoteName member is not
													//     acceptable to any network resource provider, either
													//     because the resource name is invalid, or because the named
													//     resource cannot be located.
													// ERROR_BAD_NET_NAME is also returned if you use a local path, like "C:"
				return nResult;

			case ERROR_ACCESS_DENIED:				// Access to the network resource was denied.
			case ERROR_INVALID_PASSWORD:			// Invalid password.
			case ERROR_BAD_PROFILE:					// The user profile is in an incorrect format.
			case ERROR_CANNOT_OPEN_PROFILE:			// The system is unable to open the user profile to 
													// process persistent connections.

			case ERROR_LOGON_FAILURE:				// Logon failure: unknown user name or bad password.
				// Do nothing here; we will fall down to the next paragraph of code
				// and let the user enter their login information
				break;

			case ERROR_ALREADY_ASSIGNED:			// The local device specified by the lpLocalName member
													//      is already connected to network resource.
			case ERROR_BAD_DEV_TYPE:				// This is not a network resource
			case ERROR_BAD_DEVICE:					// The value specified by lpLocalName is invalid.
			case ERROR_BAD_PROVIDER:				// The value specified by the lpProvider member does not match any
													//     provider.
			case ERROR_BUSY:						// The router or provider is busy, possibly initializing. The caller
													//		should retry
			case ERROR_DEVICE_ALREADY_REMEMBERED:	// An entry for the device specified by lpLocalName is already in the
													//      user profile.
			case ERROR_EXTENDED_ERROR:				// A network-specific error occurred. Call the WNetGetLastError function
													//      to obtain a description of the error.
			case ERROR_NO_NET_OR_BAD_PATH:			// The operation cannot be performed because a network component is not
													//      started or because a specified name cannot be used.
			case ERROR_NO_NETWORK:					// The network is unavailable.
			case ERROR_SESSION_CREDENTIAL_CONFLICT:	// Multiple connections to a server or shared resource
													// by the same user, using more than one user name, are
													// not allowed. Disconnect all previous connections to
													// the server or shared resource and try again.
			default:
				MsgBox(CString("Could not access '") + strNetworkPath + "'. Reason given: " + FormatError(nResult) +
					"\n\nIf the problem persists, please contact your network administrator.");
				return nResult;
		}
		//
		// Something went wrong. Lets try connecting again, and this time, allow the user
		// to specify a login. First, we must first close any existing connection. 
		// Otherwise, we will get a ERROR_SESSION_CREDENTIAL_CONFLICT return value. We
		// should only get here as a result of a ERROR_LOGON_FAILURE error, so forcefully
		// closing the connection should not damage anything because no resources should
		// be accessible in this connection.
		//
		WNetCancelConnection2(strNetworkPath, 0, TRUE);
		//
		// Now repeat the access attempt interactively. If the password is wrong, WNetAddConnection2
		// internally pops up the interactive dialog again. We still check for ERROR_INVALID_PASSWORD
		// anyway to keep our bases covered.
		//
		switch ((nResult = WNetAddConnection2(&res, NULL, NULL, CONNECT_INTERACTIVE | CONNECT_PROMPT)))
		{
			case NO_ERROR:							// We're connected, but that doesn't mean we can access
													// the file. TryAccessNetworkFile will take things from
													// here and repeat the access request from scratch,
													// hopefully with success.
				return nResult;

			default:
				MsgBox(CString("Could not access '") + strNetworkPath + "'. Reason given: " + FormatError(nResult) +
					"\n\nIf the problem persists, please contact your network administrator.");
				break;
		}
	}
	while (nResult == ERROR_ACCESS_DENIED || nResult == ERROR_INVALID_PASSWORD ||
		nResult == ERROR_BAD_PROFILE || nResult == ERROR_CANNOT_OPEN_PROFILE);
	return nResult;
}

DWORD TryAccessNetworkFile(const CString& strFileName)
{
	//
	// (c.haag 2006-05-03 10:39) - PLID 4612 - Here is the logic of this algorithm:
	// 
	// Try to access the file. If we can't access it, first check if the file doesn't exist.
	// If we can't determine that, assume the problem is that the file exists over the network
	// and is not accessible. So, try to establish a connection to the remote computer to access
	// the file. If that succeeds, try to access the file again, and repeat. Otherwise, fail.
	//
	CString strFilePath = GetFilePath(strFileName);
	BOOL bSuccess = FALSE;

	while (!bSuccess) {
		DWORD dwErrCode;
		//
		// First and foremost, just try to access the file normally
		//
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strFileName)) {
			//
			// If we fail to access the file, examine the cause of the problem. If it's
			// anything other than a logon failure, then fail here.
			//
			if (ERROR_LOGON_FAILURE != (dwErrCode = GetLastError())) {
				return dwErrCode;
			}

			//
			// Now try to access the path using Windows networking functions.
			// If it's a local path, TryNetworkLogin will return ERROR_BAD_NET_NAME.
			//
			switch (dwErrCode = TryNetworkLogin(strFilePath)) {
				case NO_ERROR:
					//
					// If we get here, it means we were able to access the network path,
					// so we should try again
					//
					break;

				default:
					//
					// Anything else is probably a problem we have no control over
					//
					return dwErrCode;
			}

		} else {
			//
			// If we get here, it means we were able to access the file
			//
			bSuccess = TRUE;
		}
	}
	return NO_ERROR;
}

// (j.gruber 2007-08-22 16:39) - 
//NOTE: As observed in PLID 24782 this can return Invalid Currency
//but the only known cause is if you change your regional settings and do
//not restart practice before using it.

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString FormatCurrencyForSql(const COleCurrency &cy)
//{
//	//When saving to Sql, ALWAYS format the American way.
//	return cy.Format(0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));
//}
//
//COleCurrency ParseCurrencyFromSql(const CString &strCy)
//{
//	//Sql always converts money to American dollars.
//	COleCurrency cy;
//	cy.ParseCurrency(strCy,0,MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT));
//	return cy;
//}


// cuts strEndingString off the end of strString
// for example if strString is passed in as "This is the test" and strEndingString = " test" 
// then strString will be shortened to "This is the"
// if strString is empty, then nothing is done to the strString, but if strString doens't end with strEndingString, 
// then an exception is thrown
void DeleteEndingString(IN OUT CString &strString, IN const CString &strEndingString)
{
	//this function assumes that either strString is empty or the ending characters of strString are strEndingString
	ASSERT(strString.IsEmpty() || (strString.Right(strEndingString.GetLength()) == strEndingString));
		
	if (!strString.IsEmpty()) {
		long nLengthToTrim = strEndingString.GetLength();
		
		if (strString.Right(nLengthToTrim) == strEndingString) {
			strString.Delete(strString.GetLength() - nLengthToTrim, nLengthToTrim);
		} else {
			ThrowNxException("The string '%s' you are trying to trim is shorter then %li", strString, nLengthToTrim);
		}
	}
	else{
		// do nothing if strString is empty
	}
}


// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString FormatDateTimeForSql(const COleDateTime &dt)
//{
//	return FormatDateTimeForSql(dt, dtoNaturalDatetime);
//}
//
//CString FormatDateTimeForSql(const COleDateTime &dt, DateTimeOptions dto /*= dtoNaturalDatetime*/)
//{
//	CString strFormat;
//	double dblTime = dt.m_dt - ((double)(long)dt.m_dt);
//	//If we've chosen date or datetime, or if we've chosen natural and the date isn't zero
//	if(dto == dtoDate || dto == dtoDateTime || (dto == dtoNaturalDatetime && (long)dt.m_dt != 0) ) {
//		strFormat = "%Y-%m-%d ";
//	}
//	//If we've chosen time or datetime, or if we've chosen natural and the time isn't zero.
//	if(dto == dtoTime || dto == dtoDateTime || (dto == dtoNaturalDatetime && (dblTime < -0.00001 || dblTime > .00001)) ) {
//		strFormat += "%H:%M:%S:000";
//	}
//	//Trim any trailing spaces.
//	strFormat.TrimRight();
//	if(strFormat == "") {
//		//The only way this could happen is if they've chosen natural and the date and time are both zero.
//		//In this special case, we format as a DateTime.
//		strFormat = "%Y-%m-%d %H:%M:%S:000";
//	}
//	
//	return dt.Format(strFormat);
//}

long CompareDateIgnoreTime(const IN COleDateTime &dtDateOne, const IN COleDateTime &dtDateTwo)
{
	int nYearOne = dtDateOne.GetYear();
	int nYearTwo = dtDateTwo.GetYear();
	
	if(nYearOne > nYearTwo){
		return 1;
	}
	else if(nYearOne < nYearTwo){
		return -1;
	}
	else{
		int nMonthOne = dtDateOne.GetMonth();
		int nMonthTwo = dtDateTwo.GetMonth();
		if(nMonthOne > nMonthTwo){
			return 1;
		}
		else if(nMonthOne < nMonthTwo){
			return -1;
		}
		else{
			int nDayOne = dtDateOne.GetDay();
			int nDayTwo = dtDateTwo.GetDay();
			if(nDayOne > nDayTwo){
				return 1;
			}
			else if(nDayOne < nDayTwo){
				return -1;
			}
			else{
				return 0;
			}
		}
	}
}

long CompareDates(const IN COleDateTime &dtDateOne, const IN COleDateTime &dtDateTwo)
{
	ASSERT(dtDateOne.GetHour() == 0 && dtDateOne.GetMinute() == 0 && dtDateOne.GetSecond() == 0);
	ASSERT(dtDateTwo.GetHour() == 0 && dtDateTwo.GetMinute() == 0 && dtDateTwo.GetSecond() == 0);
	return CompareDateIgnoreTime(dtDateOne, dtDateTwo);
}

// (c.haag 2003-07-28 11:23) - Filters a datalist. The function will prompt the
// user for a set of keywords, and then filters the list where the text in nCol
// contains all the keywords. nIDCol must be the column containing a unique record
// ID for each record. Returns true if at least one record returned.
BOOL FilterDatalist(_DNxDataListPtr& pList, short nCol, short nIDCol)
{
	// (c.haag 2003-07-28 09:52) - Get the search string
	IColumnSettingsPtr pCol = pList->GetColumn(nCol);
	CString strField = (LPCTSTR)pCol->FieldName;

	// m.hancock 2005-06-15 - In some cases, the field name is defined as "dbFieldName AS AliasedField".  The alias
	// causes the query to filter a data list to crash.  If the full field name is used rather than the alias, the
	// query will succeed.
	if(strField.Find(" AS ") >= 0)
		strField = strField.Left(strField.Find(" AS "));

	CString strSearch, strWhere, strOldWhere = (LPCTSTR)pList->WhereClause;
	long nOldRow = pList->CurSel;
	COleVariant vOldID;	
	if (IDCANCEL == InputBox(NULL, "Enter a list of required keywords", strSearch, ""))
		return FALSE;
	if (!strSearch.GetLength())
		return FALSE;
	
	//m.hancock 2005-07-14 - If a search for a string containing a % is requested, the sql query strips off the %
	// from the query.  In order to preserve the search term, the % must be placed within brackets.  This also goes
	// for other characters such as [ _ .  Note: the order in which these lines execute matters because they could
	// potentially replace the characters in the query.
	strSearch.Replace("[", "[[]");
	strSearch.Replace("%", "[%]");
	strSearch.Replace("_", "[_]");

	// (c.haag 2003-07-28 11:09) - Preserve the current selection
	if (nOldRow > -1)
		vOldID = pList->GetValue(nOldRow, nIDCol);
	
	// (c.haag 2003-07-28 09:52) - Calculate the WHERE clause
	// (d.thompson 2009-03-03) - Fixed while doing PLID 33103
	CString strOldWhereClause = (LPCTSTR)pList->WhereClause;
	if(strOldWhereClause.IsEmpty()) {
		strWhere = "";
	}
	else {
		strWhere.Format("(%s)", strOldWhereClause);
	}
	
	// (j.jones 2009-04-08 09:13) - PLID 33905 - fixed a memory leak
	char* szSearch = new char[strSearch.GetLength()+1];
	strcpy(szSearch, strSearch);
	char *token;
	token = strtok(szSearch, " ");	
	while (token != NULL)
	{
		if (strWhere.GetLength())
			strWhere += " AND ";
		strWhere += "(" + strField + " LIKE '%%" + _Q(token) + "%%')";
		token = strtok(NULL, " ");
	}
	delete szSearch;

	// (c.haag 2003-07-28 10:06) - Assign the WHERE clause
	pList->WhereClause = (LPCTSTR)strWhere;
	pList->Requery();

	// (c.haag 2003-07-28 11:09) - Restore the current value
	pList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	// (c.haag 2003-07-28 11:12) - If no results are returned, restore the current filter
	if (pList->GetRowCount() == 0)
	{
		MsgBox("Your filter returned no records. The filter will now be removed");
		pList->WhereClause = (LPCTSTR)strOldWhere;
		pList->Requery();
		pList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		pList->CurSel = nOldRow;
		return FALSE;
	}
	else if (nOldRow > -1 && pList->IsComboBox)
	{
		if (-1 == pList->FindByColumn(nIDCol, vOldID, 0, TRUE))
			pList->CurSel = 0;
	}

	// (c.haag 2003-07-28 10:06) - Set the background in a tinted red so the user knows
	// a filter is in place, then requery the list. We will leave the details of unfiltering
	// to the caller.
	for (short i=0; i < pList->ColumnCount; i++)
		pList->GetColumn(i)->BackColor = RGB(255,210,210);
	return TRUE;
}

// (j.jones 2009-03-23 16:09) - PLID 33639 - added a datalist 2 version of this function
// ID for each record. Returns true if at least one record returned.
BOOL FilterDatalist2(NXDATALIST2Lib::_DNxDataListPtr& pList, short nCol, short nIDCol)
{
	// (c.haag 2003-07-28 09:52) - Get the search string
	NXDATALIST2Lib::IColumnSettingsPtr pCol = pList->GetColumn(nCol);
	CString strField = (LPCTSTR)pCol->FieldName;

	// m.hancock 2005-06-15 - In some cases, the field name is defined as "dbFieldName AS AliasedField".  The alias
	// causes the query to filter a data list to crash.  If the full field name is used rather than the alias, the
	// query will succeed.
	if(strField.Find(" AS ") >= 0) {
		strField = strField.Left(strField.Find(" AS "));
	}

	CString strSearch, strWhere, strOldWhere = (LPCTSTR)pList->WhereClause;
	NXDATALIST2Lib::IRowSettingsPtr pOldRow = pList->GetCurSel();
	COleVariant vOldID;	
	if (IDCANCEL == InputBox(NULL, "Enter a list of required keywords", strSearch, ""))
		return FALSE;

	if (!strSearch.GetLength())
		return FALSE;

	CWaitCursor wc;
	
	//m.hancock 2005-07-14 - If a search for a string containing a % is requested, the sql query strips off the %
	// from the query.  In order to preserve the search term, the % must be placed within brackets.  This also goes
	// for other characters such as [ _ .  Note: the order in which these lines execute matters because they could
	// potentially replace the characters in the query.
	strSearch.Replace("[", "[[]");
	strSearch.Replace("%", "[%]");
	strSearch.Replace("_", "[_]");

	// (c.haag 2003-07-28 11:09) - Preserve the current selection
	if(pOldRow) {
		vOldID = pOldRow->GetValue(nIDCol);
	}
	
	// (c.haag 2003-07-28 09:52) - Calculate the WHERE clause
	// (d.thompson 2009-03-03) - Fixed while doing PLID 33103
	CString strOldWhereClause = (LPCTSTR)pList->WhereClause;
	if(strOldWhereClause.IsEmpty()) {
		strWhere = "";
	}
	else {
		strWhere.Format("(%s)", strOldWhereClause);
	}

	char* szSearch = new char[strSearch.GetLength()+1];
	strcpy(szSearch, strSearch);
	char *token;
	token = strtok(szSearch, " ");	
	while (token != NULL)
	{
		if (strWhere.GetLength())
			strWhere += " AND ";
		strWhere += "(" + strField + " LIKE '%%" + _Q(token) + "%%')";
		token = strtok(NULL, " ");
	}
	delete szSearch;

	// (c.haag 2003-07-28 10:06) - Assign the WHERE clause
	pList->WhereClause = (LPCTSTR)strWhere;
	pList->Requery();

	// (c.haag 2003-07-28 11:09) - Restore the current value
	pList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	// (c.haag 2003-07-28 11:12) - If no results are returned, restore the current filter
	if (pList->GetRowCount() == 0)
	{
		MsgBox("Your filter returned no records. The filter will now be removed");
		CWaitCursor wc;
		pList->WhereClause = (LPCTSTR)strOldWhere;
		pList->Requery();
		pList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		// (z.manning 2009-05-20 14:14) - PLID 33774 - Cleaned up the preserve selection code so that
		// we don't get an error in the event that we did not find the previously selected row after
		// all our requerying.
		if(NULL == pList->FindByColumn(nIDCol, vOldID, pList->GetFirstRow(), VARIANT_FALSE)) {
			pList->CurSel = pList->GetFirstRow();
		}
		return FALSE;
	}
	else if (pOldRow != NULL && pList->IsComboBox)
	{
		if(NULL == pList->FindByColumn(nIDCol, vOldID, pList->GetFirstRow(), TRUE)) {
			pList->CurSel = pList->GetFirstRow();
		}
	}

	// (c.haag 2003-07-28 10:06) - Set the background in a tinted red so the user knows
	// a filter is in place, then requery the list. We will leave the details of unfiltering
	// to the caller.
	for (short i=0; i < pList->ColumnCount; i++) {
		pList->GetColumn(i)->BackColor = RGB(255,210,210);
	}
	return TRUE;
}

//TES 12/10/2010 - PLID 40879 - Added a Datalist2 version that takes multiple columns.  If the search term appears in any of the columns,
// the row will be included.
BOOL FilterDatalist2(NXDATALIST2Lib::_DNxDataListPtr& pList, const CArray<short,short> &arColumns, short nIDCol)
{
	CString strSearch, strOldWhere = (LPCTSTR)pList->WhereClause;
	NXDATALIST2Lib::IRowSettingsPtr pOldRow = pList->GetCurSel();
	COleVariant vOldID;	
	if (IDCANCEL == InputBox(NULL, "Enter a list of required keywords", strSearch, ""))
		return FALSE;

	if (!strSearch.GetLength())
		return FALSE;

	CWaitCursor wc;
	
	//m.hancock 2005-07-14 - If a search for a string containing a % is requested, the sql query strips off the %
	// from the query.  In order to preserve the search term, the % must be placed within brackets.  This also goes
	// for other characters such as [ _ .  Note: the order in which these lines execute matters because they could
	// potentially replace the characters in the query.
	strSearch.Replace("[", "[[]");
	strSearch.Replace("%", "[%]");
	strSearch.Replace("_", "[_]");
	
	CString strExtraWhere;
	for(int i = 0; i < arColumns.GetSize(); i++) {
		short nCol = arColumns[i];
		CString strThisWhere;
		// (c.haag 2003-07-28 09:52) - Get the search string
		NXDATALIST2Lib::IColumnSettingsPtr pCol = pList->GetColumn(nCol);
		CString strField = (LPCTSTR)pCol->FieldName;

		// m.hancock 2005-06-15 - In some cases, the field name is defined as "dbFieldName AS AliasedField".  The alias
		// causes the query to filter a data list to crash.  If the full field name is used rather than the alias, the
		// query will succeed.
		if(strField.Find(" AS ") >= 0) {
			strField = strField.Left(strField.Find(" AS "));
		}

		char* szSearch = new char[strSearch.GetLength()+1];
		strcpy(szSearch, strSearch);
		char *token;
		token = strtok(szSearch, " ");	
		while (token != NULL)
		{
			if (strThisWhere.GetLength())
				strThisWhere += " AND ";
			strThisWhere += "(" + strField + " LIKE '%%" + _Q(token) + "%%')";
			token = strtok(NULL, " ");
		}
		delete szSearch;
	// (s.dhole 2011-06-17 13:37) - PLID 44049 Since we do not have add-on condition  for where clause, Skip this OR
		if (strThisWhere.GetLength()){
			if(strExtraWhere.GetLength()) {
				strExtraWhere += " OR ";
			}
			strExtraWhere += "(" + strThisWhere + ")";
		}
	}

	// (c.haag 2003-07-28 11:09) - Preserve the current selection
	if(pOldRow) {
		vOldID = pOldRow->GetValue(nIDCol);
	}
	
	// (c.haag 2003-07-28 09:52) - Calculate the WHERE clause
	// (d.thompson 2009-03-03) - Fixed while doing PLID 33103
	CString strWhere;
	CString strOldWhereClause = (LPCTSTR)pList->WhereClause;
	if(strOldWhereClause.IsEmpty()) {
		strWhere = "";
	}
	else {
		strWhere.Format("(%s)", strOldWhereClause);
	}
	// (s.dhole 2011-06-17 13:37) - PLID 44049 Since we do not have add-on condition  for where clause, Skip this AND
	if (strExtraWhere.GetLength()){
		if(strWhere.GetLength()) {
			strWhere += " AND ";
		}
		strWhere += "(" + strExtraWhere + ")";
	}
	// (c.haag 2003-07-28 10:06) - Assign the WHERE clause
	pList->WhereClause = (LPCTSTR)strWhere;
	pList->Requery();

	// (c.haag 2003-07-28 11:09) - Restore the current value
	pList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	// (c.haag 2003-07-28 11:12) - If no results are returned, restore the current filter
	if (pList->GetRowCount() == 0)
	{
		MsgBox("Your filter returned no records. The filter will now be removed");
		CWaitCursor wc;
		pList->WhereClause = (LPCTSTR)strOldWhere;
		pList->Requery();
		pList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		// (z.manning 2009-05-20 14:14) - PLID 33774 - Cleaned up the preserve selection code so that
		// we don't get an error in the event that we did not find the previously selected row after
		// all our requerying.
		if(NULL == pList->FindByColumn(nIDCol, vOldID, pList->GetFirstRow(), VARIANT_FALSE)) {
			pList->CurSel = pList->GetFirstRow();
		}
		return FALSE;
	}
	else if (pOldRow != NULL && pList->IsComboBox)
	{
		if(NULL == pList->FindByColumn(nIDCol, vOldID, pList->GetFirstRow(), TRUE)) {
			pList->CurSel = pList->GetFirstRow();
		}
	}

	// (c.haag 2003-07-28 10:06) - Set the background in a tinted red so the user knows
	// a filter is in place, then requery the list. We will leave the details of unfiltering
	// to the caller.
	for (short i=0; i < pList->ColumnCount; i++) {
		pList->GetColumn(i)->BackColor = RGB(255,210,210);
	}
	return TRUE;
}

BOOL PrintFile(const CString& strFilePath)
{
	// This has always assumed the input file is a Word file, and will continue to assume so
	std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
	if (nullptr == pApp)
	{
		ThrowNxException("Failed to get a word processor instance");
	}
	else
	{
		pApp->EnsureValid();
		return pApp->PrintFile(strFilePath);
	}
}

HCURSOR GetLinkCursor()
{
	return ::LoadCursor(NULL, IDC_HAND);
}

// (a.walling 2010-06-09 16:03) - PLID 39087 - ShowDlgItem now in CNexTechDialog's NxUI library object

CString ParseField(CString strLine, int nStart, int nCount)
{
	if(strLine.GetLength() < nStart + nCount) {	//line is not this long, so we need to read from start to the end
		if(strLine.GetLength() < nStart) //start is past the end, just quite
			return "";
		else
			nCount = strLine.GetLength() - nStart;
	}

	CString strReturn = strLine.Mid(nStart, nCount);

	strReturn.TrimLeft();
	strReturn.TrimRight();
	return strReturn;
}

void ImportCPTCodes() {

	try {
		if(!CheckCurrentUserPermissions(bioServiceCodes, sptCreate)) return;

		//import CPT Codes from the AMA list, the three formats are:

		///////////////////////////////////////////////////////////////////////
		//CPT LONG DESCRIPTION
		//
		//All records are 80 characters in length, fixed-field
		//format.  When long CPT code descriptions exceed
		//72 characters, the CPT code is repeated and the
		//sequence number (2-digit counter) is increased
		//by one (eg, 01, 02, 03, etc). Blank space will
		//fill records through position 80 if record
		//descriptions are less than 72 characters in
		//length. Record content includes:
		//
		//CPT Code - position 1 through 5 (alpha/numeric)
		//Sequence Number - position 6 though 7 (alpha/numeric)
		//Blank space - position 8
		//CPT Long Description - position 9 through 80 (alpha/numeric)
		//
		//Example:
		//
		//1234501 I am a description of a code.
		//9924101 I am also a description of another code.
		//
		///////////////////////////////////////////////////////////////////////
		//
		//CPT SHORT DESCRIPTION
		//
		//All records are 80 characters in length, fixed-field
		//format. Short CPT code descriptions do not exceed
		//28 characters in length. Blank space will
		//fill records through position 80. Record content includes:
		//
		//CPT Code - position 1 through 5 (alpha/numeric)
		//Blank space - position 6
		//CPT Short Description - position 7 up to 34 (alpha/numeric)
		//Blank space through position 80
		//
		//Example:
		//
		//12345 I am a description of a code.
		//99241 I am also a description of another code.
		//
		///////////////////////////////////////////////////////////////////////
		//CPT MEDIUM DESCRIPTION
		//
		//All records are 106 characters in length. Medium CPT
		//code descriptions do not exceed 100 characters in length.
		//Blank space will fill records through position 106 if record
		//descriptions are less than 100 characters in length. The 
		//truncation convention list developed for the medium descriptions
		//is provided for your information. Record content includes:
		//
		//CPT Code - position 1 through 5 (alpha/numeric)
		//Blank space - position 6
		//CPT Medium Description - position 7 through 106 (alpha/numeric)
		//
		//Example:
		//
		//12345 I am a description of a code.
		//99241 I am also a description of another code.
		//
		///////////////////////////////////////////////////////////////////////

		AfxMessageBox("In the following window, please select your code file to import.");
		CFileDialog BrowseFiles(TRUE,NULL,NULL,OFN_FILEMUSTEXIST);
		if (BrowseFiles.DoModal() == IDCANCEL) return;
		CString strFileName = BrowseFiles.GetPathName();

		CFile InputFile, OutputFile;

		//open the file for reading
		if(!InputFile.Open(strFileName,CFile::modeRead | CFile::shareCompat)) {
			AfxMessageBox("The input file could not be found or opened. Please double-check your path and filename.");
			return;
		}

		CWaitCursor pWait;

		//quickly run through and try to identify that this is a valid file
		CArchive arTest(&InputFile, CArchive::load);
		CString strTest;
		arTest.ReadString(strTest);
		BOOL bFileHasData = FALSE;
		BOOL bValid = TRUE;
		BOOL bLong = FALSE;
		while(arTest.ReadString(strTest) && bValid) {

			if(!bFileHasData) {
				//before we set bValid to true, test once to see if it is long format
				if(ParseField(strTest,5,1) != "") {
					//it would appear we're in the long format
					bLong = TRUE;
				}
			}

			//there is at least data in this file, so we're on the right track
			bFileHasData = TRUE;			

			if(bLong && ParseField(strTest,7,1) != "")
				bValid = FALSE;

			if(!bLong && ParseField(strTest,5,1) != "")
				bValid = FALSE;
		}

		if(!bValid) {
			AfxMessageBox("This does not appear to be a valid code file. Please double-check the file you selected.");
			return;
		}
		
		//reopen the file for reading again
		arTest.Close();
		InputFile.Close();
		InputFile.Open(strFileName,CFile::modeRead | CFile::shareCompat);

		BOOL bOutputFileUsed = FALSE;
		BOOL bAdditions = FALSE;

		CString strOutputFile = GetNxTempPath() ^ "OverwrittenCPTCodes.txt";
		if(!OutputFile.Open(strOutputFile,CFile::modeCreate|CFile::modeWrite | CFile::shareCompat)) {
			AfxMessageBox("The output file could not be created.");
			return;
		}

		CString strIn;	//input string
		CArchive arIn(&InputFile, CArchive::load);

		arIn.ReadString(strIn); //skip the first line, it's a copyright notice (or our "overwritten codes" header);

		CString OutputString;
		OutputString = "The following codes were overwritten by your code import. You may re-import this file to restore these values.\r\n";
		OutputFile.Write(OutputString,OutputString.GetLength());

		CString strCPTCode;
		CString strDescription;

		long nLastCPTID = -1;	//used if we're in the long format and on the second line of a CPT Code's description

		while(arIn.ReadString(strIn)) {

			strCPTCode = ParseField(strIn,0,5);	//the CPT Code
			
			//test to see if we're in the long format
			if(ParseField(strIn,5,1) != "") {
				//we are indeed in the long format
				
				//get the description
				strDescription = ParseField(strIn,8,72);	//long description

				CString strIncrement = ParseField(strIn,5,2);
				if(strIncrement != "" && strIncrement != "01") {
					//we're adding on to the last code, so update now and continue
					
					if(strCPTCode != "" && strDescription != "") {
						strDescription = " " + strDescription;
						ExecuteSql("UPDATE ServiceT SET Name = Name + '%s' WHERE ID = %li",_Q(strDescription),nLastCPTID);
						continue;
					}
				}
			}
			else {
				//we're not in the long format

				//get the description
				strDescription = ParseField(strIn,6,100);	//short/medium description
			}
			
			//now check if we need to update a code, or add new
			_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ServiceT.ID, Name FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID WHERE CPTCodeT.Code = '%s' ORDER BY Code, SubCode", _Q(strCPTCode));
			if(!rs->eof) {
				//update

				CString strOldDesc = AdoFldString(rs, "Name","");
				if(strOldDesc != strDescription) {
					//store this information in the output file
					OutputString.Format("%s %s\r\n",strCPTCode,strOldDesc);
					OutputFile.Write(OutputString,OutputString.GetLength());
					bOutputFileUsed = TRUE;
				}

				nLastCPTID = AdoFldLong(rs, "ID",-1);
				//all we do is change the description, really
				ExecuteSql("UPDATE ServiceT SET Name = '%s' WHERE ID = %li",_Q(strDescription),nLastCPTID);
			}
			else {
				//add new
				nLastCPTID = NewNumber("ServiceT","ID");
				ExecuteSql("INSERT INTO ServiceT (ID, Name, Price, Taxable1, Taxable2) VALUES (%li, '%s', Convert(money, '0'), 0, 0);", nLastCPTID, _Q(strDescription));
				ExecuteSql("INSERT INTO CPTCodeT (ID, Code, SubCode, TypeOfService) VALUES (%li, '%s', '', '');", nLastCPTID, _Q(strCPTCode));
				// (j.gruber 2012-12-04 11:38) - PLID 48566 - serviceLocationInfoT
				ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT {INT}, ID FROM LocationsT WHERE Managed = 1 "
						, nLastCPTID);	

				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, "", nAuditID, aeiCPTCreate, -1, "", strCPTCode, aepMedium, aetCreated);

				bAdditions = TRUE;
			}
			rs->Close();
		}

		//close the files
		InputFile.Close();
		OutputFile.Close();

		if(bOutputFileUsed) {
			AfxMessageBox("All codes imported successfully!\n\n"
				"Some codes were overwritten. Please save the following file as a backup.\n"
				"(It can be re-imported to restore the changed codes.)");
			//open the output file in notepad
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			ShellExecute(GetDesktopWindow(), NULL, "notepad.exe", ("'" + strOutputFile + "'"), NULL, SW_SHOW);
		}
		else {
			if(!bAdditions)
				AfxMessageBox("No codes were added or modified. Your code list is up to date.");
			else
				AfxMessageBox("All codes imported successfully!");
		}

		CClient::RefreshTable(NetUtils::CPTCodeT);

	}NxCatchAll("Error importing codes.");
}

BOOL IsValidChar(TCHAR sch)
{
	// Make sure it's unsigned
	TBYTE ch = sch;
	if (ch >= 32 && ch < 127 || ch >= 128 && ch < 255) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// If the character is valid, this just returns the character
// If the character is invalid, this returns a valid character (the returned character is not 
//  random; if called multiple times passing the same invalid character as the parameter, the 
//  same valid character will always be returned)
TCHAR EnsureValidChar(TCHAR ch)
{
	if (IsValidChar(ch)) {
		// It's valid, just return it
		return ch;
	} else {
		// It's not valid, so map it into the set of valid characters
		const int nNumberOfValidChars = (127-32) + (255-128) /*95 + 127 = 222 (the number of printable characters)*/;
		int nIndexIntoValidCharList = ch % nNumberOfValidChars;
		if (nIndexIntoValidCharList < (127-32) /*95*/) {
			// It's in the first range so map it into the valid characters in that range by subtracting the index from the end of the range
			return (TCHAR)(126 - nIndexIntoValidCharList);
		} else {
			// It's in the second range (it must be because it's not less than 95, and we modded against 222 so it must be less than 222)
			ASSERT(nIndexIntoValidCharList >= 95 && nIndexIntoValidCharList < 222);
			// So map it into the second range of valid characters by subtracting 95 (to make it 0-based) and subtracting that (now 0-based) index from the end of the range
			return (TCHAR)(254 - (nIndexIntoValidCharList - 95));
		}
	}
}

// If hWnd, its parent, its parent's parent, or any window on up the line (i.e. hWnd or any of its ancestors), is equal to hWndAncestor then this returns TRUE.
// If hWnd or hWndAncestor is NULL, or if none of hWnd's ancestors is equal to hWndAncestor, then this returns FALSE.
BOOL IsOrHasAncestor(HWND hWnd, HWND hWndAncestor)
{
	if (hWnd == NULL || hWndAncestor == NULL) {
		return FALSE;
	} else if (hWnd == hWndAncestor) {
		return TRUE;
	} else {
		return IsOrHasAncestor(::GetParent(hWnd), hWndAncestor);
	}
}

// Uses PeekMessage to find the given message in the queue; optionally (depending on 
// what you pass in for nFlags) matches wParam and lParam to the message in the queue.
// If hWnd is NULL, this considers messages for any window that belongs to the current 
// thread. If hWnd is INVALID_HANDLE_VALUE, it only considers messages whose hWnd value 
// is NULL, as posted by the PostThreadMessage function.
// *** NOTE: If you are comparing wParam and/or lParam, be aware that it simply compares 
// the values to the FIRST nMessage message in the queue.  So, for example, if the user 
// hits the tab key and the enter key and you are searching for WM_KEYDOWN of VK_ENTER, 
// you will not get a match because the wParam of the FIRST WM_KEYDOWN will be VK_TAB.
BOOL IsMessageInQueue(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam, UINT nFlags)
{
	// Find the first of the given message in the queue
	MSG msg;
	if (PeekMessage(&msg, hWnd, nMessage, nMessage, PM_NOREMOVE)) {
		// If we're being asked to compare the wParam, then fail if they don't match
		if (nFlags & IMIQ_MATCH_WPARAM) {
			if (msg.wParam != wParam) {
				return FALSE;
			}
		}
		// If we're being asked to compare the lParam, then fail if they don't match
		if (nFlags & IMIQ_MATCH_LPARAM) {
			if (msg.lParam != lParam) {
				return FALSE;
			}
		}
		// Found the message, return TRUE
		return TRUE;
	} else {
		// The message wasn't in the queue
		return FALSE;
	}
}


//TES 9/18/2008 - PLID 31413 - Moved EnsurePatientDocumentPath() to HistoryUtils

// (c.haag 2006-07-19 11:13) - PLID 21505 - Returns the number of colors in the dib: 0, 2, 16, or 256
WORD DibNumColors (VOID FAR *pv)
{
	int Bits = 0;
	LPBITMAPINFOHEADER lpbi = NULL;
	LPBITMAPCOREHEADER lpbc = NULL;

	ASSERT(pv);

	lpbi = ((LPBITMAPINFOHEADER)pv);
	lpbc = ((LPBITMAPCOREHEADER)pv);

	//    With the BITMAPINFO format headers, the size of the palette
	//    is in biClrUsed, whereas in the BITMAPCORE - style headers, it
	//    is dependent on the bits per pixel ( = 2 raised to the power of
	//    bits/pixel).
	if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
	{
		if (lpbi->biClrUsed != 0)
		{
			return (WORD)lpbi->biClrUsed;
		}

		Bits = lpbi->biBitCount;
	}
	else
	{ 
		Bits = lpbc->bcBitCount;
	}

	switch (Bits)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			// A 24 bitcount DIB has no color table
			return 0;
	}
}

//Used by the history tabs when attaching photos, to identify ones that have already been attached.
// (c.haag 2006-07-19 11:12) - PLID 21505 - Fixed incorrect bytes per line calculation
#define  WIDTHBYTES(i)    ((i+31)/32*4)
long BitmapChecksum(const LPBITMAPINFO pDib)
{
	long nChecksum = 0;
	unsigned char * ptr = (unsigned char *) pDib;
	long nImageSize = pDib->bmiHeader.biSizeImage;
	if (nImageSize == 0) {
		nImageSize = WIDTHBYTES ((DWORD)pDib->bmiHeader.biWidth * pDib->bmiHeader.biBitCount) * pDib->bmiHeader.biHeight;
	}
	DWORD dwColorTableSize = (DWORD)(DibNumColors (pDib) * sizeof(RGBQUAD));
	ptr += sizeof(BITMAPINFOHEADER);
	ptr += dwColorTableSize;
	//ptr now points to the beginning of the actual bits.
	int nBytesPerLine = (nImageSize / pDib->bmiHeader.biHeight);
	for(int i = 0; i < nBytesPerLine * pDib->bmiHeader.biHeight; i++) {
		nChecksum += ptr[i];
	}
	return nChecksum;
}

// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
void WINAPI CALLBACK OnNxTwainPreCompress(const LPBITMAPINFO pDib, /* The image that was scanned */
		BOOL& bAttach,
		BOOL& bAttachChecksum, long &nChecksum, ADODB::_Connection* lpCon)
{
	//Our assumption is if they show the interface, then they know what they're doing.
	// (a.walling 2010-01-28 14:36) - PLID 28806 - We can access the static setting from NxTwain here
	extern bool g_bShowUI;
	if(!g_bShowUI) {
		nChecksum = BitmapChecksum(pDib);
		//See if this image has been attached already, by generating a checksum, and checking for its existence.
		// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam(lpCon, "SELECT MailID FROM MailSent WHERE Checksum = {INT}", nChecksum)) {
			bAttach = FALSE;
		}
		else {
			bAttach = TRUE;
			bAttachChecksum = TRUE;
		}
	}
}

// (a.walling 2010-03-15 12:02) - PLID 37751 - GetByteArrayAsHexString moved to GlobalDataUtils




// (a.walling 2013-04-24 14:57) - PLID 56247 - Loads an existing thumb from the file if it is up to date (and if it exists obviously)
HBITMAP LoadExistingThumbFromFile(const IN CString &strFile)
{
	try {
		FILETIME ftModified = {0};

		// (r.gonet 09/12/2012) - PLID 52597 - Check if the file exists. Before, if it didn't and the path
		//  was a UNC path, then CreateFile could cause memory corruption (for a currently undetermined reason)
		//  when it tries to create a file stream to it. DoesExist does not alter any behavior. It does add
		//  a little overhead by making a second file lookup over the network though. The check for existence
		//  prevents the memory corruption.	
		{
			WIN32_FILE_ATTRIBUTE_DATA attributes = {0};
			if (!::GetFileAttributesEx(strFile, GetFileExInfoStandard, &attributes)) {
				return NULL;
			}
			ftModified = attributes.ftLastWriteTime;
		}

		Nx::ThumbInfo thumbInfo;
		if (!LoadThumbInfo(strFile, thumbInfo)) {
			return NULL;
		}

		if (!ThumbUpToDate(ftModified, thumbInfo)) {
			return NULL;
		}

		Nx::Handle hThumbFile(
			::CreateFile(strFile + ":NxThumb"
				, GENERIC_READ
				, FILE_SHARE_READ // only allow concurrent reads
				, NULL
				, OPEN_EXISTING
				, FILE_ATTRIBUTE_NORMAL
				, NULL
			)
		);

		if (!hThumbFile) {
			return NULL;
		}

		LARGE_INTEGER fileSize = {0};
		if (!::GetFileSizeEx(hThumbFile, &fileSize)) {
			return NULL;
		}

		// sanity check, 10mb max
		if (fileSize.QuadPart <= 0 || fileSize.QuadPart > 0xA00000) {
			return NULL;
		}

		CxImageData thumbData;
		thumbData.length = (long)fileSize.QuadPart;
		thumbData.data = (BYTE*)malloc(thumbData.length);

		DWORD dwRead = 0;
		if (!::ReadFile(hThumbFile, thumbData.data, thumbData.length, &dwRead, NULL)) {
			return NULL;
		}
		if (dwRead != thumbData.length) {
			return NULL;
		}

		CxImage image;
		if (!image.Decode(thumbData.data, (DWORD)thumbData.length, thumbInfo.nFormat)) {
			return NULL;
		}

		return image.MakeBitmap();
	} catch (...) {	
		Log(NxCatch::GetErrorMessage(__FUNCTION__));
		return NULL;
	}
}


// (a.walling 2013-04-24 14:57) - PLID 56247 - Attempts to encode and write the thumb and info streams
bool TryUpdateThumb(CxImage& image, const Nx::ThumbInfo& thumbInfo, const CString& strThumbInfoFile, const CString& strThumbFile)
{
	try {
		if (thumbInfo.nFormat == CXIMAGE_FORMAT_JPG) {
			image.SetJpegQuality((BYTE)thumbInfo.nQuality);
		}

		// encode to target format
		CxImageData encoded;
		if (!image.Encode(encoded.data, encoded.length, thumbInfo.nFormat)) {
			return false;
		}

		// (d.thompson 2015-10-14 11:40) - PLID 67352 - Misc type conversion issues in vs2015, explicitly declare our types.
		static const FILETIME g_cftIgnore = {(DWORD)-1, (DWORD)-1};

		Nx::Handle hThumbInfo(
			::CreateFile(strThumbInfoFile
				, GENERIC_WRITE
				, 0 // no sharing
				, NULL
				, CREATE_NEW // always create new (we deleted above)
				, FILE_ATTRIBUTE_NORMAL
				, NULL
			)
		);
		if (!hThumbInfo) {
			return false;
		}

		// prevent changes to file modified time through this handle
		if (!::SetFileTime(hThumbInfo, NULL, NULL, &g_cftIgnore)) {
			ASSERT(FALSE);
		}

		Nx::Handle hThumbStream(
			::CreateFile(strThumbFile
				, GENERIC_WRITE
				, 0 // no sharing
				, NULL
				, CREATE_NEW // always create new (we deleted above)
				, FILE_ATTRIBUTE_NORMAL
				, NULL
			)
		);
		if (!hThumbStream) {
			return false;
		}

		// prevent changes to file modified time through this handle
		if (!::SetFileTime(hThumbStream, NULL, NULL, &g_cftIgnore)) {
			ASSERT(FALSE);
		}

		// write the thumb
		DWORD thumbWritten = 0;
		::WriteFile(hThumbStream, encoded.data, encoded.length, &thumbWritten, NULL);

		// write the thumbinfo
		DWORD infoWritten = 0;
		::WriteFile(hThumbInfo, &thumbInfo, sizeof(thumbInfo), &infoWritten, NULL);
		
		hThumbStream.Close();
		hThumbInfo.Close();

		if (thumbWritten != encoded.length) {
			return false;
		}
		if (infoWritten != sizeof(thumbInfo)) {
			return false;
		}

		return true;
	} catch(...) {
		Log(NxCatch::GetErrorMessage(__FUNCTION__));
	}
	return false;
}
// (a.walling 2013-04-24 14:57) - PLID 56247 - Loads the full image and creates thumbnail. Will always return
// that thumbnail if it managed to create it; attempts to update the thumbnail on the filesystem which may fail.
HBITMAP CreateAndUpdateThumbFromFile(const IN CString& strFile)
{
	HBITMAP hThumb = NULL;
	try {
		// prepare our thumb info		
		Nx::ThumbInfo thumbInfo;
		if (GetRemotePropertyInt("LosslessThumbnail", 1, 0, "<None>", true) == 1) {
			thumbInfo.nFormat = CXIMAGE_FORMAT_PNG;
		} else {
			thumbInfo.nFormat = CXIMAGE_FORMAT_JPG;
			thumbInfo.nQuality = GetRemotePropertyInt("JPEGThumbnailQuality", 75, 0, "<None>", true);
		}

		CString strThumbInfoFile = strFile + ":NxThumbInfo";
		CString strThumbFile = strFile + ":NxThumb";

		// destroy existing streams, if any
		// counter-intuitively, calling SetEndOfFile on a stream will update the modification time of the main file
		// it is safer and easier to delete this outright; this also has the benefit of being able to CREATE_NEW
		// when recreating the files, so if another process is attempting to do so, it will gracefully exit.
		::DeleteFile(strThumbInfoFile);
		::DeleteFile(strThumbFile);

		// load the full image read only
		Nx::Handle hFile(
			::CreateFile(strFile
				, GENERIC_READ | FILE_WRITE_ATTRIBUTES
				, FILE_SHARE_READ // only allow concurrent reads
				, NULL
				, OPEN_EXISTING
				, FILE_ATTRIBUTE_NORMAL
				, NULL
			)
		);

		if (!hFile) {
			return NULL;
		}

		// get modified date
		if (!::GetFileTime(hFile, NULL, NULL, &thumbInfo.ftExpectedMod)) {
			return NULL;
		}

		// and the file size
		LARGE_INTEGER fileSize = {0};
		if (!::GetFileSizeEx(hFile, &fileSize)) {
			return NULL;
		}

		// sanity check, 100mb max
		if (fileSize.QuadPart <= 0 || fileSize.QuadPart > 0x6000000) {
			return NULL;
		}

		// now load the full image
		CxImage image;
		{
			// read into local buffer
			CxImageData imageData;
			imageData.length = (long)fileSize.QuadPart;
			imageData.data = (BYTE*)malloc(imageData.length);

			DWORD dwRead = 0;
			if (!::ReadFile(hFile, imageData.data, imageData.length, &dwRead, NULL)) {
				return NULL;
			}
			if (dwRead != imageData.length) {
				return NULL;
			}

			long nFormatID = GuessCxFormatFromExtension(strFile);

			if (!image.Decode(imageData.data, (DWORD)imageData.length, nFormatID)) {
				if (nFormatID == CXIMAGE_FORMAT_UNKNOWN) {
					return NULL;
				}
				// try another time with CXIMAGE_FORMAT_UNKNOWN
				
				if (!image.Decode(imageData.data, (DWORD)imageData.length, CXIMAGE_FORMAT_UNKNOWN)) {
					return NULL;
				}
			}
		}

		// create the thumbnail
		CreateThumbnail(image, 100, 100);

		// now we have our thumb!
		hThumb = image.MakeBitmap();

		// try to update the streams; if this fails we will still return the hThumb from above
		if (!TryUpdateThumb(image, thumbInfo, strThumbInfoFile, strThumbFile)) {
			Log("Failed to update thumb for %s", strFile);
		}

		// although the file stream operations should not have modified the file's modified time
		// due to the special ignore value set for the streams, just in case we will try to reset
		// the modified time anyway. Since the file is open exlusively to us, no writing, we can
		// be confident no one has modified it since now anyway.

		if (!::SetFileTime(hFile, NULL, NULL, &thumbInfo.ftExpectedMod)) {
			Log("Failed to reset file modified time for %s", strFile);
		}

		return hThumb;
	} catch (...) {	
		Log(NxCatch::GetErrorMessage(__FUNCTION__));
		return hThumb;
	}
}

// (a.walling 2013-04-24 14:57) - PLID 56247 - Loads the thumbnail from a file. Attempts to write/update if necessary.
HBITMAP LoadThumbFromFile(const IN CString &strFile)
{
	try {
		// Get an existing, up-to-date thumbnail
		HBITMAP hThumb = LoadExistingThumbFromFile(strFile);
		if (!hThumb) {
			// Not available, so create a thumbnail, and attempt to write it to the filesystem as well
			hThumb = CreateAndUpdateThumbFromFile(strFile);
		}
		return hThumb;
	} catch (...) {	
		Log(NxCatch::GetErrorMessage(__FUNCTION__));
		return NULL;
	}
}

//This returns the username for the license agreement prompt ConfigRT setting in the format of:
//win user=user
CString GetLicenseAgreementUsername()
{
	// (r.gonet 2016-05-19 18:13) - NX-100689 - The license agreement prompt will occur once per
	// windows user. Ron Kozlin, our CFO, has approved this. It used to prompt once per windows username,
	// machine name, and session path. This change was made for Azure RemoteApp because the particular TS VM
	// Practice is running on is not important and is not enumerable. It was extended to all clients though
	// because apparently it was a common complaint that the license agreement prompted too often.

	CString strWindowsUsername = GetWindowsUsername();
	// Truncate the username so it will fit in the ConfigRT.Username field. It would be absurdly large if this actually mattered.
	return FormatString("win user=%s", strWindowsUsername.Left(111));
}

//This returns an advanced username in the format of:
//COMPUTER|working path|win user=user
CString GetAdvancedUsername()
{
	return g_propManager.GetAdvancedUsername();
}

void LaunchCodeLink(HWND hwnd)
{
	CString strPath = GetRemotePropertyText("CodeLinkExePath","",0,"<None>",TRUE);

	if(strPath != "" && DoesExist(strPath)) {
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		ShellExecute(hwnd, NULL, strPath, NULL, "", SW_SHOWNORMAL);
	}
	else {
		AfxMessageBox("The stored path to the CodeLink file is either invalid or missing.\n"
			"Please configure the correct path under \"Modules->Link to CodeLink...\"");
	}
}

//DRT 5/13/2004 - See notes in CImportAMACodesDlg::OnInitDialog()
/*DRT 4/19/2007 - PLID 25598 - This function no longer exists.
CString GetCurrentAMACodeVersion()
{
	//This is the original description of the problem.  The next set of comments is the 
	//	solution that was decided upon.
	//The program must keep track of what version they are currently a part of.  There
	//	are currently 3 schools of thought on how/where to track this:  1)  In the license.  
	//	Using the license enables us to easily change their current version by sending a new 
	//	file to the office.  We can also have it default to the current version through code.
	//	2)  In the database.  We can setup a table that tracks a history as we update them
	//	to different versions of the AMA code list.  This downside is that a manual upgrade
	//	requires that we run a query on the data.  3)  Keep a separate file (similar to the 
	//	license file) that will keep track of their history.  We can then possibly put this
	//	in the install (but never to overwrite if one already exists), which will handle 
	//	any default behavior.

	//DRT 5/18/2004 - I have decided to go with the data option, for several reasons.  #1, it's
	//	the easiest to implement and easiest to read version we have.  It also lets us keep a 
	//	very nice version history for each client.  The extra file idea is basically the same 
	//	implementation, just not in the data.  I am avoiding that solution because it would
	//	just be an extra file to keep track of in support and updating the installation.
	//	#2, We are soon going to be switching the licensing system to be contained in the database
	//	itself, as well as developing a method of giving the user a code over the phone that
	//	will automatically upgrade certain things (w/s #'s, modules, etc).  We can easily adapt that 
	//	to the same system.
*/
/*
	try {

		//read from AMAVersionHistoryT - the highest version is the one we will use.  All data should
		//be in the format of "YYYY.V"
		_RecordsetPtr prs = CreateRecordset("SELECT Name, Date FROM AMAVersionHistoryT");
		if(prs->eof) {
			//no version written, we'll be on the current one - but write it to data first
			ExecuteSql("INSERT INTO AMAVersionHistoryT (Name, Date) values ('%s', GetDate())", AMA_CODES_VERSION);
			return AMA_CODES_VERSION;
		}

		CString strMax;
		while(!prs->eof) {
			CString str = AdoFldString(prs, "Name", "");
			if(str > strMax)
				strMax = str;

			prs->MoveNext();
		}

		//highest version available
		return strMax;
	} NxCatchAll("Error loading the AMA Code version.");

	//no idea what to do with this if we got an error - this will prevent
	//anything from using it
	return "";
*/

/*	DRT 4/10/2007 - PLID 25556 - We no longer fill in this manner.  I made a mod to remove the AMAVersionHistoryT table, 
		so this code is no longer of value.

	//DRT 11/5/2004 - After some rethought, we're just going to display the latest ama codes, it's really the only
	//	way we can track it - in the old thought process, if they never opened the ama import, they never would have 
	//	been flagged for a version, and therefore could later get the next year's codes and we would not know it.
	//We are still going to keep a history, just in case.
	if(!ReturnsRecords("SELECT * FROM AMAVersionHistoryT WHERE Name = '%s'", AMA_CODES_VERSION))
		ExecuteSql("INSERT INTO AMAVersionHistoryT (Name, Date) values ('%s', GetDate())", AMA_CODES_VERSION);
*/
/*
	return AMA_CODES_VERSION;
}
*/

//TES 9/18/2008 - PLID 31413 - Moved MakeValidFolderName() to HistoryUtils

// (a.walling 2010-05-17 12:45) - PLID 34056 - Returns success/failure. Must be called prior to actually changing the data. bAllowContinue can be set to false to not allow them to continue without moving the folder.
bool EnsureCorrectHistoryFolder(CWnd* pWnd, EChangedHFItem item, const CString& strNewValue, long nPersonID, bool bAllowContinue)
{
	CString strError;

	// (c.haag 2004-05-20 15:01) - Exceptions are intentionally not caught here
	// because the calling process may need to roll back a complete set of
	// instructions if an intermediate step fails.
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset("SELECT [Last], [First], CASE WHEN PatientsT.UserDefinedID IS NULL THEN PersonT.ID ELSE PatientsT.UserDefinedID END AS UserDefinedID "
		"FROM PersonT LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = {INT}",nPersonID);
	if (prs->eof)
		return true; // Do nothing if we found no record

	CString strLast = AdoFldString(prs, "Last", "");
	CString strFirst = AdoFldString(prs, "First", "");
	CString strUserDefinedID = FormatString("%li", AdoFldLong(prs, "UserDefinedID"));

	// Get the new folder path
	CString strOldFolderName = MakeValidFolderName(strLast + "-" + strFirst + "-" + strUserDefinedID);
	CString strExistingFolderPath = GetSharedPath() ^ "Documents" ^ strOldFolderName;

	switch (item)
	{
	case eChangedHFID:
		strUserDefinedID = strNewValue;
		break;
	case eChangedHFFirst:
		strFirst = strNewValue;
		break;
	case eChangedHFLast:
		strLast = strNewValue;
		break;
	default:
		ThrowNxException("EnsureCorrectHistoryFolder: Invalid item specified");
		break;
	}
	// Get the new folder path
	CString strNewFolderName = MakeValidFolderName(strLast + "-" + strFirst + "-" + strUserDefinedID);
	CString strNewFolderPath = GetSharedPath() ^ "Documents" ^ strNewFolderName;

	if (strNewFolderPath.CompareNoCase(strExistingFolderPath) == 0)
		return true; // They match -- no need to do anything

	while (true) {
		strError.Empty();

		// Find out if the old folder exists.
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strExistingFolderPath))
		{
			long nLastError = GetLastError();
			switch (nLastError)
			{
			case 2: // The system cannot find the file specified
				// It doesn't exist, so we have nothing to change
				return true;
			default:
				{
					CString strSysError;
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, nLastError, 0, strSysError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
					strSysError.ReleaseBuffer();
					strError.Format("The document folder '%s' is inaccessible from this computer.\r\n\r\nError: %s",
						strExistingFolderPath, strSysError);
				}
			}
		}

		if (strError.IsEmpty()) {
			// Rename the folder
			if (!MoveFile(strExistingFolderPath, strNewFolderPath))
			{
				CString strSysError;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strSysError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strSysError.ReleaseBuffer();
				strError.Format("Could not update the document folder name from '%s' to '%s'.\r\n\r\nAnother user may currently be using a document within this folder.\r\n\r\nError: %s",
					 strExistingFolderPath, strNewFolderPath, strSysError);
			}
			else {
				// (c.haag 2010-10-25 10:57) - PLID 33601 - It's possible that the patient has subdirectories in their default
				// folder with attached documents in them. They are attached with the full path included. Find those records
				// and replace the first occurrence of the last-first-id combination with the new values.
				_RecordsetPtr prsSubDirs = CreateRecordset("SELECT MailID, PathName FROM MailSent "
					"WHERE PersonID = %d AND (PathName LIKE '%%Documents\\%s%%' OR PathName LIKE '%%Documents/%s%%')"
					,nPersonID, _Q(strOldFolderName), _Q(strOldFolderName));
				if (!prsSubDirs->eof)
				{
					while (!prsSubDirs->eof)
					{
						const long nMailID = AdoFldLong(prsSubDirs, "MailID");
						const CString strPathInData = AdoFldString(prsSubDirs, "PathName");
						int i = strPathInData.Find("Documents\\" + strOldFolderName);
						if (-1 == i) {
							i = strPathInData.Find("Documents/" + strOldFolderName);
						}

						if (-1 == i) {
							// This should never happen; our search was quite clear on this! The user is already
							// told to review the subdirectory structure anyway, so do nothing here.
						}
						else {
							// If we get here, we found what we were looking for. Replace just the first
							// occurrence.
							CString strNewPathInData = strPathInData.Left(i + 10) + strNewFolderName + 
								strPathInData.Mid(i + 10 + strOldFolderName.GetLength());

							if (-1 != strNewPathInData.Find(strOldFolderName)) {
								// If for some wacky reason the old path STILL exists (e.g. Documents\Doe-Jane-123\Documents\Doe-Jane-123),
								// the user is still warned to review the subdirectory structure anyway.
							}

							// Run it
							ExecuteParamSql("UPDATE MailSent SET PathName = {STRING} WHERE MailID = {INT}"
								,strNewPathInData, nMailID);
							// Now also update MailSent notes that are identical to the old path in data. If the note is anything else
							// then the user changed it, and we don't want to overwrite user changes.
							ExecuteParamSql("UPDATE MailSentNotesT SET Note = {STRING} WHERE Note = {STRING} AND MailID = {INT}"
								,strNewPathInData, strPathInData, nMailID);
						}
						prsSubDirs->MoveNext();
					} // while (!prsSubDirs->eof)

					// Always throw up a warning message if any records that point to full paths were found.
					pWnd->MessageBox(FormatString(
						"The document folder for %s, %s has been renamed; but this person may have attachments located in subfolders or "
						"outside their documents folder entirely.\r\n\r\n"
						"Please go to the History tab and confirm that all attached files are still accessible for this person."
						,strLast, strFirst), NULL, MB_OK | MB_ICONINFORMATION);

				} // if (!prsSubDirs->eof)

			}

		} // if (strError.IsEmpty()) {

		if (!strError.IsEmpty()) {
			int nResult = 0;
			if (bAllowContinue) {
				nResult = pWnd->MessageBox(FormatString("%s\r\n\r\nYou may continue to proceed without changing the folder name, but the folder name will not be changed. This requires administrator access. Otherwise, please try again or cancel.", strError), NULL, MB_CANCELTRYCONTINUE | MB_ICONSTOP);
			} else {
				nResult = pWnd->MessageBox(FormatString("%s\r\n\r\nPlease try again or cancel.", strError), NULL, MB_RETRYCANCEL | MB_ICONSTOP);
			}

			switch (nResult) {
				case IDCANCEL:
				case IDABORT:
					return false;
				case IDCONTINUE:
				case IDIGNORE:
					if (!IsCurrentUserAdministrator()) {
						if (CheckAdministratorPassword()) {
							return true;
						}
					} else {
						return true;
					}
			}
		}
	}

	return strError.IsEmpty() ? true : false;
}

void EnsureCorrectHistoryFolder(const CString& strOldUserDefinedID, const CString& strOldFirst,
								const CString& strOldLast, long nPersonID)
{
	// (c.haag 2004-05-20 15:01) - Exceptions are intentionally not caught here
	// because the calling process may need to roll back a complete set of
	// instructions if an intermediate step fails.
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset("SELECT [Last], [First], UserDefinedID FROM PatientsT INNER JOIN PersonT ON PersonT.ID = PatientsT.PersonID WHERE PersonID = {INT}",
		nPersonID);
	if (prs->eof)
		return; // Do nothing if we found no record

	CString strLast = AdoFldString(prs, "Last", "");
	CString strFirst = AdoFldString(prs, "First", "");
	CString strUserDefinedID;
	CString strOldFolderPath, strNewFolderPath;
	strUserDefinedID.Format("%d", AdoFldLong(prs, "UserDefinedID"));

	// Get the new folder path
	strNewFolderPath = GetSharedPath() ^ "Documents" ^ MakeValidFolderName(strLast + "-" + strFirst + "-" + strUserDefinedID);

	// Get the old folder path
	strOldFolderPath = GetSharedPath() ^ "Documents" ^ MakeValidFolderName(strLast + "-" + strFirst + "-" + strUserDefinedID);

	if (strOldFolderPath == strNewFolderPath)
		return; // They match -- no need to do anything

	// Find out if the old folder exists.
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strOldFolderPath))
	{
		long nLastError = GetLastError();
		switch (nLastError)
		{
		case 2: // The system cannot find the file specified
			// It doesn't exist, so we have nothing to change
			return;
		default:
			{
				CString strError;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, nLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strError.ReleaseBuffer();
				MsgBox("The patient document folder '%s' is inaccessible from this computer. Please ensure the folder name is consistent with the patient's ID, first, and last name.\n\nError: " + strError);
			}
			return;
		}
	}

	// Rename the folder
	if (!MoveFile(strOldFolderPath, strNewFolderPath))
	{
		CString strError;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
		strError.ReleaseBuffer();
		MsgBox("Could not update the patient document folder name from '%s' to '%s'. Please ensure the folder name is consistent with the patient's ID, first, and last name.\n\nError: %s",
			 strOldFolderPath, strNewFolderPath, strError);
	}
}

CPoint CalcContextMenuPos(CWnd *pWnd, CPoint pos)
{
	if (pos.x == -1) {
		// We weren't given a good point, so try to get it from the applicable window, and 
		// failing that just go ahead and use the cursor position on screen.
		CPoint point(pos);
		if (pWnd->GetSafeHwnd()) {
			point.x = point.y = 0;
			pWnd->ClientToScreen(&point);
		} else {
			GetCursorPos(&point);
		}
		return point;
	} else {
		// The given pos is good, just return it
		return pos;
	}
}

// Given a control, usually a checkbox or radio button, this calculates the size the control 
// should be set to in order to fit its contents.
// (a.walling 2011-11-11 11:11) - PLID 46622 - Pass the buffer sizes
void CalcControlIdealDimensions(CDC* pdc, CWnd *pControl, IN OUT CSize &sz, BOOL bAllowMultiLine /*= FALSE*/, const CSize& szBuffer /*= GetControlBufferSize()*/)
{	
	// (a.walling 2010-07-02 14:23) - PLID 39498 - use CNexTechIconButton instead -- CNxIconButton is now only a wrapper for AutoSet
	if(pControl->IsKindOf(RUNTIME_CLASS(CNexTechIconButton))) {
		((CNexTechIconButton*)pControl)->CalcIdealDimensions(sz);
		return;
	}

	if (sz.cx != LONG_MAX) {
		sz.cx -= szBuffer.cx;
	}

	if (sz.cy != LONG_MAX) {
		sz.cy -= szBuffer.cy;
	}
	
	CDC& dc = *pdc;

	// (a.walling 2011-11-11 11:11) - PLID 46628 - Use INxWindowlessLayout for windowless controls
	LPUNKNOWN pUnk = pControl->GetControlUnknown();
	if (pUnk) {
		NxWindowlessLib::INxWindowlessLayoutPtr pWindowlessLayout(pUnk);

		if (pWindowlessLayout) {
			pWindowlessLayout->GetIdealSize(reinterpret_cast<long>(dc.m_hDC), &sz.cx, &sz.cy, bAllowMultiLine ? VARIANT_TRUE : VARIANT_FALSE);
			sz.cx += szBuffer.cx;
			sz.cy += szBuffer.cy;
			return;
		}
	}
	
	CFont *pFont = GetControlFont(pControl);

	CFont *pOldFont = NULL;
	if(pFont) {
		pOldFont = dc.SelectObject(pFont);
	}
	
	// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - use GetControlText

	CString strText;
	GetControlText(pControl, strText);

	CRect rc(CPoint(0, 0), sz);

	//DRT 2/9/2006 - PLID 19175 - Apparently if your text is "", instead of calculating a size of 0, the
	//	DrawText function actually does nothing (your lpRect parameter is left unchanged).  In that case, 
	//	we're going to set the rectangle to 0, 0 width before calling DrawText.  I'm unsure if this is
	//	a by-design thing or if it's an MFC bug (i can find no documentation)... so we'll still call the function
	//	in case it's changed in the future.
	//The bug happening for this PLID is because we pass in a sz.cx of MAX_LONG, and then we add space onto it
	//	to cover the radio button part, wrapping over into negatives.  I thought about fixing that (it's probably
	//	the true problem), but adding a bunch of checks there seems slower for a not-noticeable gain in functionality.
	//	This text really can fit in a 0x0 rectangle, so that is accurate.
	if(strText.IsEmpty())
		rc.bottom = rc.top = rc.left = rc.right = 0;

	dc.DrawText(strText, rc, DT_LEFT|(bAllowMultiLine ? DT_WORDBREAK : DT_SINGLELINE)|DT_EDITCONTROL|DT_CALCRECT);
	if(pOldFont) dc.SelectObject(pOldFont);
	
	sz = rc.Size();
	sz.cx += szBuffer.cx;
	sz.cy += szBuffer.cy;

	long nMinHeight = GetSystemMetrics(SM_CYMENUCHECK) + 2;
	if (sz.cy < nMinHeight) {
		sz.cy = nMinHeight;
	}

	if(pControl->IsKindOf(RUNTIME_CLASS(CButton))) {
		sz.cx += GetSystemMetrics(SM_CXMENUCHECK) + 2;
	}
}

// Sets pszTotal to contain the amount of width (cx) and height (cy) taken up by the non-client 
// area of the given window.  Sort of the total window area minus the client's part of the area.
// Put another way, this
// sets pszTotal->cx equal to the sum of the left border width and the right border width, and 
// sets pszTotal->cy equal to the sum of the top border height and the bottom border height.
void CalcWindowBorderSize(IN CWnd *pWnd, OUT CSize *pszTotal)
{
	CRect rc(0, 0, 1000, 1000);
	pWnd->CalcWindowRect(&rc, CWnd::adjustOutside);
	pszTotal->cx = rc.Width() - 1000;
	pszTotal->cy = rc.Height() - 1000;
}

// Given a window and an expected total window rect, this alters the rect to be what 
// the client rect would be (the window without borders, titles, scrollbars, etc.).
void CalcClientFromWindow(IN CWnd *pWnd, IN OUT LPRECT prc)
{
	CRect rcFakeClient(0, 0, 1000, 1000);
	pWnd->CalcWindowRect(&rcFakeClient, CWnd::adjustOutside);
	prc->left += (0 - rcFakeClient.left);
	prc->top += (0 - rcFakeClient.top);
	prc->right += (1000 - rcFakeClient.right);
	prc->bottom += (1000 - rcFakeClient.bottom);
}

// Given a window and an expected total window size, this alters the size to be what 
// the client size would be (the window without borders, titles, scrollbars, etc.).
void CalcClientFromWindow(IN CWnd *pWnd, IN OUT CSize *psz)
{
	CRect rcFakeClient(0, 0, 1000, 1000);
	pWnd->CalcWindowRect(&rcFakeClient, CWnd::adjustOutside);
	psz->cx -= (rcFakeClient.Width() - 1000);
	psz->cy -= (rcFakeClient.Height() - 1000);

	// (a.walling 2011-05-31 12:40) - PLID 43843 - The psz that is passed in is not necessarily large enough for the borders! In that case,
	// make sure we never return any negative value.
	if (psz->cx < 0) {
		psz->cx = 0;
	}
	if (psz->cy < 0) {
		psz->cy = 0;
	}
}

void AddFolderToList_Flat(const CString &strFolderName, HWND hWndNotify, _DNxDataListPtr pList, HANDLE hevQuit, bool bPrePendSharedPath = true)
{
	CFileFind finder;
	CString strFind = "";
	if(bPrePendSharedPath){
		strFind = GetSharedPath() ^ "Templates" ^ strFolderName;
	}
	else{
		strFind = strFolderName;
	}

	IRowSettingsPtr pRow;
	if (finder.FindFile(strFind ^ "*.*"))
	{
		while (finder.FindNextFile())
		{
			if(finder.IsDirectory() && !finder.IsDots()) {
				//Recurse
				AddFolderToList_Flat(CString(strFolderName ^ finder.GetFileName()), hWndNotify, pList, hevQuit);
			}
			// (a.walling 2007-06-14 16:09) - PLID 26342 - Support .dotx files
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			else if( ( (finder.GetFileName().Right(4).CompareNoCase(".dot") == 0) || (finder.GetFileName().Right(5).CompareNoCase(".dotx") == 0) 
				|| (finder.GetFileName().Right(5).CompareNoCase(".dotm") == 0))
						&& !finder.IsHidden() && !finder.IsSystem() ) {
			// (a.walling 2006-07-20 09:58) - PLID 21525 Skip over hidden and system files

				// (c.haag 2006-12-27 15:45) - PLID 23300 - If we have a notify window, we must
				// pass a message to the main window and have it add the row. Having a notification
				// window means that this function is running in a worker thread, and we cannot
				// do direct datalist accesses.
				if (NULL != hWndNotify) {
					if (IsWindow(hWndNotify)) {
						// (c.haag 2008-03-31 15:54) - PLID 29429 - Use PostMessage instead of SendMessage because
						// the main thread may not be pumping messages; but instead, be waiting for us to quit.
						AddFolderMessage* pMsg = new AddFolderMessage;
						pMsg->pList = (LPDISPATCH)pList;
						pMsg->strFileName = finder.GetFileName();
						pMsg->strFolderName = strFolderName;
						if (!::PostMessage(hWndNotify, NXM_ADD_FOLDER_NEXT_FOLDER, (WPARAM)pMsg, 0)) {
							// If it fails, then the message wasn't posted, so delete the message data.
							delete pMsg;
						}
					}
				} else {
					pRow = pList->GetRow(-1);
					pRow->Value[0] = _bstr_t(finder.GetFileName());
					pRow->Value[1] = _bstr_t(strFolderName);
					pList->AddRow(pRow);
				}
			}
			//TES 10/26/2007 - PLID 24831 - If we were given a Quit event, check whether it's set, and if it is, 
			// immediately return.
			if(hevQuit) {
				if(WAIT_OBJECT_0 == WaitForSingleObject(hevQuit,0)) {
					return;
				}
			}
		}
		//do once more
		if(finder.IsDirectory() && !finder.IsDots()) {
			//Recurse
			AddFolderToList_Flat(CString(strFolderName ^ finder.GetFileName()),hWndNotify,pList, hevQuit);
		}
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		else if( ( (finder.GetFileName().Right(4).CompareNoCase(".dot") == 0) || (finder.GetFileName().Right(5).CompareNoCase(".dotx") == 0) 
				|| (finder.GetFileName().Right(5).CompareNoCase(".dotm") == 0))
					&& !finder.IsHidden() && !finder.IsSystem() ) {
			pRow = pList->GetRow(-1);
			pRow->Value[0] = _bstr_t(finder.GetFileName());
			pRow->Value[1] = _bstr_t(strFolderName);
			pList->AddRow(pRow);
		}
	} 
}

struct AddFolderToListInfo
{
	CString strFolderName;
	_DNxDataListPtr pList;
	bool bPrependSharedPath;
	HWND hwndNotify;
	HANDLE hevQuit;
};

UINT AddFolderToListThread(LPVOID p)
{
	AddFolderToListInfo *pAftli = (AddFolderToListInfo*)p;
	AddFolderToList_Flat(pAftli->strFolderName, pAftli->hwndNotify, pAftli->pList, pAftli->hevQuit, pAftli->bPrependSharedPath);
	if(::IsWindow(pAftli->hwndNotify)) {
		::PostMessage(pAftli->hwndNotify, NXM_ADD_FOLDER_COMPLETED, (WPARAM)(LPDISPATCH)pAftli->pList, NULL);
	}
	//TES 2/20/2007 - PLID 24831 - Let go of the list.
	//TES 8/3/2007 - PLID 24831 - ...making sure to decrement the reference count.
	pAftli->pList.Release();
	delete pAftli;
	return 0;
}

//Recurses through the given folder, adds each file to the list, assumes two BSTR columns, the first for the filename,
//the second for the folder.
CAddFolderToListThread* AddFolderToList(const CString &strFolderName, _DNxDataListPtr pList, bool bPrePendSharedPath /*= true*/, CWnd *pNotifyWnd /*= NULL*/)
{
	if(pNotifyWnd) {
		//Launch a thread.
		AddFolderToListInfo *pAftli = new AddFolderToListInfo;
		pAftli->strFolderName = strFolderName;
		//TES 2/20/2007 - PLID 24831 - This was merely setting pAftli->pList = pList, which meant that if the window was
		// closed (and therefore pList was destroyed) before the AddFolderToListThread finished executing, Practice crashed
		// with an access violation.
		pAftli->pList.Attach(pList.Detach());
		pAftli->bPrependSharedPath = bPrePendSharedPath;
		pAftli->hwndNotify = pNotifyWnd->GetSafeHwnd();
		//TES 10/26/2007 - PLID 24831 - Set an event that we can signal to let the thread know that it needs to abort
		// its loading before we destroy the list.
		HANDLE hevQuit = CreateEvent(NULL, TRUE, FALSE, NULL);
		pAftli->hevQuit = hevQuit;
		CWinThread *pCreatedThread = AfxBeginThread(AddFolderToListThread, pAftli, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		pCreatedThread->m_bAutoDelete = FALSE;
		pCreatedThread->ResumeThread();
		//It is up to our calling function to clean up this thread.
		//TES 10/26/2007 - PLID 24831 - To make it easier on them, we'll encapsulate everything in a CAddFolderToListThread
		// object, they just have to delete it and the destructor will do the rest.
		CAddFolderToListThread *pThread = new CAddFolderToListThread(pCreatedThread, hevQuit);
		return pThread;
	}
	else {
		//Just load the folder synchronously.
		AddFolderToList_Flat(strFolderName, NULL, pList, NULL, bPrePendSharedPath);
		//No thread was created.
		return NULL;
	}
	return NULL;
	
}

CAddFolderToListThread::CAddFolderToListThread(CWinThread *pThread, HANDLE hevQuit)
{
	m_hevQuit = hevQuit;
	m_pThread = pThread;
}

CAddFolderToListThread::~CAddFolderToListThread()
{
	// See if the thread is still active
	DWORD dwExitCode = 0;
	::GetExitCodeThread(m_pThread->m_hThread, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE) {
		//TES 10/26/2007 - PLID 24831 - Set our event telling our thread to terminate itself.
		SetEvent(m_hevQuit);
		//TES 10/26/2007 - PLID 24831 - We need to keep pumping messages, because our thread has a ::SendMessage() 
		// call to the main thread, so if we don't pump messages we'll deadlock.  So, keep pumping messages until
		// the thread terminates, or our wait times out.
		// (c.haag 2008-03-31 15:55) - PLID 29429 - We now use ::PostMessage instead of ::SendMessage, so there's
		// no longer a need to use MsgWaitForMultipleObjects. We'll just do a simple wait instead.
		/*DWORD dwResult = WAIT_OBJECT_0 + 1;
		DWORD dwTickCount = GetTickCount();
		while(dwResult == WAIT_OBJECT_0 + 1) {
			
			dwResult = MsgWaitForMultipleObjects(1, &m_pThread->m_hThread, FALSE, 5000, QS_SENDMESSAGE);
			if(dwResult == WAIT_OBJECT_0 + 1) {
				PeekAndPump();
				if(GetTickCount() > dwTickCount + 5000) {
					//It shouldn't take 5 seconds for our thread to check the quit event.
					dwResult = WAIT_TIMEOUT;
				}
			}
		}*/
		DWORD dwResult = WaitForSingleObject(m_pThread->m_hThread, 5000);
		if(dwResult == WAIT_TIMEOUT) {
			//TES 10/26/2007 - PLID 24831 - This should never happen, but just in case it does, let's terminate the thread.
			ASSERT(FALSE);
			TerminateThread(m_pThread->m_hThread, 0);
		}
		else {
			//We must have gotten our thread.
			ASSERT(dwResult == WAIT_OBJECT_0);
		}
	}
	//TES 10/26/2007 - PLID 24831 - Now, let's clean up our memory.
	delete m_pThread;
	CloseHandle(m_hevQuit);
}

//finds text in a datalist, case-insensitive
int FindTextInDataList(NXDATALISTLib::_DNxDataListPtr pList, int iColumn, CString strText, int iStartRow /* = 0 */, bool bAutoSelect /* = false */)
{
	for(int i=iStartRow;i<pList->GetRowCount();i++) {
		CString strRowString = VarString(pList->GetValue(i,iColumn));
		if(strRowString.CompareNoCase(strText) == 0) {
			if(bAutoSelect)
				pList->CurSel = i;
			return i;
		}
	}

	return -1;
}

// (r.gonet 07/09/2012) - PLID 50708 - Removed VariantCompare utility functions to the GlobalDataUtils.cpp
//  Took into account a.walling's modification for the VT_UI1|VT_ARRAY variants.

// Removes an entry from the given submenu of the given root menu (so for example, this can remove the 
// ID_ACTIVATIES_EYEGRAPH entry from the "Activities" submenu of the patient's root menu by calling 
// RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Activities", ID_ACTIVATIES_EYEGRAPH, TRUE))
void RemoveEntryFromSubMenu(CMenu *pRootMenu, LPCTSTR strSubMenuLabel, UINT nIDEntry, BOOL bRemovePrecedingSeparatorIfExists)
{
	if (pRootMenu && IsMenu(pRootMenu->m_hMenu)) {
		// Find the given submenu
		// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
		for (int i = 0; i < pRootMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pRootMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find(strSubMenuLabel) != -1) {
				// Now, find the given entry (we have to loop through and find it, because
				// we want to remove the preceding separator.
				for (int j = 0; j < pRootMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pRootMenu->GetSubMenu(i)->GetMenuItemID(j) == nIDEntry) {
						// Remove this one for sure.
						pRootMenu->GetSubMenu(i)->RemoveMenu(j, MF_BYPOSITION);
						// What about the immediately preceding one?
						if (bRemovePrecedingSeparatorIfExists) {
							MENUITEMINFO mui;
							mui.cbSize = sizeof(MENUITEMINFO);
							mui.fMask = MIIM_FTYPE;
							pRootMenu->GetSubMenu(i)->GetMenuItemInfo(j-1, &mui, TRUE);
							if (mui.fType & MFT_SEPARATOR) {
								pRootMenu->GetSubMenu(i)->RemoveMenu(j-1, MF_BYPOSITION);
							}
						}
						break;
					}
				}
			}
		}
	}
}

CString GenerateDelimitedListFromLongArray(IN const CDWordArray &arydw, IN const CString &strDelimiter /*= ";"*/)
{
	long nCount = arydw.GetSize();
	switch (nCount) {
	case 0:
		// Empty
		return "";
		break;
	case 1:
		// Just one entry
		return AsString((long)arydw.GetAt(0));
		break;
	default:
		// 2 or more
		{
			CString strAns;
			for (long i=0; i<nCount; i++) {
				strAns += AsString((long)arydw.GetAt(i)) + strDelimiter;
			}
			long nDelimLen = strDelimiter.GetLength();
			if (!strAns.IsEmpty() && nDelimLen > 0) {
				ASSERT(strAns.Right(nDelimLen) == strDelimiter);
				strAns.Delete(strAns.GetLength() - nDelimLen, nDelimLen);
			}
			return strAns;
		}
		break;
	}
}

CString GenerateDelimitedListFromStringArray(IN const CStringArray &arystr, IN const CString &strDelimiter /*= ";"*/)
{
	long nCount = arystr.GetSize();
	switch (nCount) {
	case 0:
		// Empty
		return "";
		break;
	case 1:
		// Just one entry
		return arystr.GetAt(0);
		break;
	default:
		// 2 or more
		{
			CString strAns;
			for (long i=0; i<nCount; i++) {
				strAns += arystr.GetAt(i) + strDelimiter;
			}
			long nDelimLen = strDelimiter.GetLength();
			if (!strAns.IsEmpty() && nDelimLen > 0) {
				ASSERT(strAns.Right(nDelimLen) == strDelimiter);
				strAns.Delete(strAns.GetLength() - nDelimLen, nDelimLen);
			}
			return strAns;
		}
		break;
	}
}

CString GenerateDelimitedListFromDatalistColumn(IN NXDATALISTLib::_DNxDataList *lpdl, IN const short nColIndex, IN const CString &strDelimiter /*= ";"*/)
{
	_DNxDataListPtr pdl(lpdl);
	
	CString strAns;
	LONG p = pdl->GetFirstRowEnum();
	while (p) {
		LPDISPATCH lpDisp = NULL;
		pdl->GetNextRowEnum(&p, &lpDisp);
		if (lpDisp) {
			IRowSettingsPtr pRow(lpDisp);
			lpDisp->Release();
			CString str = AsString(pRow->GetValue(nColIndex));
			str.TrimRight();
			str.TrimLeft();
			if (!str.IsEmpty()) {
				strAns += str + strDelimiter;
			}
		}
	}
	if (!strDelimiter.IsEmpty() && strAns.Right(strDelimiter.GetLength()) == strDelimiter) {
		strAns.Delete(strAns.GetLength() - strDelimiter.GetLength(), strDelimiter.GetLength());
	}
	return strAns;
}

// (b.cardillo 2009-07-22 12:26) - Originally created this more generic form of GenerateDelimitedListFromRecordsetColumn() 
// which takes a recordset, and prefix/suffix instead of just delimiter, for use in PLID 34844
CString GenerateDelimitedListFromRecordsetColumn(IN _Recordset *lprs, IN const _variant_t &varField, IN const CString &strListEntryPrefix /*= ""*/, IN const CString &strListEntrySuffix /*= ";"*/)
{
	_RecordsetPtr prs = lprs;
	if (!prs->eof) {
		// Generate the list
		CString strAns;
		FieldPtr fld = prs->GetFields()->GetItem(varField);
		while (!prs->eof) {
			strAns += strListEntryPrefix + AdoFldString(fld) + strListEntrySuffix;
			prs->MoveNext();
		}
		// We know the recordset wasn't empty so we don't have to check that strAns is not empty 
		// before deleting the suffix from the end.
		if (!strListEntrySuffix.IsEmpty()) {
			strAns.Delete(strAns.GetLength() - strListEntrySuffix.GetLength(), strListEntrySuffix.GetLength());
		}
		return strAns;
	} else {
		return "";
	}
}

CString GenerateDelimitedListFromRecordsetColumn(IN const CString &strSql, IN const _variant_t &varField, IN const CString &strDelimiter /*= ";"*/)
{
	CString strAns;
	_RecordsetPtr prs = CreateRecordsetStd(strSql);
	if (!prs->eof) {
		FieldPtr fld = prs->GetFields()->GetItem(varField);
		while (!prs->eof) {
			CString str = AsString(fld->GetValue());
			str.TrimRight();
			str.TrimLeft();
			if (!str.IsEmpty()) {
				strAns += str + strDelimiter;
			}
			prs->MoveNext();
		}
	}
	if (!strDelimiter.IsEmpty() && strAns.Right(strDelimiter.GetLength()) == strDelimiter) {
		strAns.Delete(strAns.GetLength() - strDelimiter.GetLength(), strDelimiter.GetLength());
	}
	return strAns;
}

//DRT 6/20/2006
//Loops through pArray looking for dwIDToFind.  Returns FALSE if it completes with no find
//	or if the pArray is NULL.
// (c.haag 2009-01-19 12:15) - PLID 32712 - Made pArray const
BOOL IsIDInArray(DWORD dwIDToFind, const CDWordArray *pArray)
{
	//Invalid search
	if(pArray == NULL)
		return FALSE;

	long nSize = pArray->GetSize();
	for(int i = 0; i < nSize; i++) {
		if(dwIDToFind == pArray->GetAt(i)) {
			//found it
			return TRUE;
		}
	}

	//did not find it in the array
	return FALSE;
}

BOOL IsIDInArray(long nIDToFind, const CArray<long,long>& ary)
{
	//
	// (c.haag 2007-01-25 13:03) - PLID 22420 - New overload for IsIDInArray. We pass
	// in an array reference so it's almost impossible to have an invalid array when 
	// this function is called.
	//
	const int nSize = ary.GetSize();
	for (int i = 0; i < nSize; i++) {
		if(nIDToFind == ary[i]) {
			// Found it
			return TRUE;
		}
	}
	// Did not find it
	return FALSE;
}

//DRT 6/6/2008 - This works just like the IDInArray as above, except it searches for strings in CStringArray's
//	I made this for PLID 30306, but it's generic for anyone.  Note that at this point I made it case-insensitive, 
//	but it would be pretty easy to add a flag if we need to.
bool IsStringInStringArray(CString strText, CStringArray *pary)
{
	if(pary == NULL)
		return false;

	long nSize = pary->GetSize();
	for(int i = 0; i < nSize; i++) {
		if(strText.CompareNoCase(pary->GetAt(i)) == 0) {
			//Found it!
			return true;
		}
	}

	//Not found
	return false;
}

// (z.manning 2011-12-07 16:19) - PLID 46906
BOOL IsDateTimeInDateTimeArray(const COleDateTime dtToFind, CArray<COleDateTime> *parydt)
{
	if (parydt == NULL) {
		return FALSE;
	}

	for (int nDateIndex = 0; nDateIndex < parydt->GetCount(); nDateIndex++)
	{
		if (parydt->GetAt(nDateIndex) == dtToFind) {
			return TRUE;
		}
	}

	return FALSE;
}

//DRT 1/23/2008 - PLID 28603 - This is a generic function to take a delimited string of numbers and convert them
//	to an array.  There are numerous places we do this in code already, but oddly noone ever made a 
//	handy function for it.  Use this now!
//This should work for both strings that end in the delimiter and those that have 1 field after the delimiter
//	Function does no input verification -- you give it non-numeric data, you'll get a bunch of 0's back.
void ParseDelimitedStringToDWordArray(const IN CString strToParse, const IN CString strDelim, OUT CDWordArray &aryToFill)
{
	//copy the string to start
	CString strInput = strToParse;
	while (!strInput.IsEmpty())	{
		long nDelim = strInput.Find(strDelim);

		if (nDelim == -1) {
			//Not found.  If we're still inside the loop, the string must not be empty, so
			//	there must be 1 more field after the end.
			aryToFill.Add(atoi(strInput));

			strInput.Empty();
		}
		else {
			aryToFill.Add(atoi(strInput.Left(nDelim)));

			//Truncate the input to remove what we pulled out
			strInput = strInput.Mid(nDelim + 1);
		}
	}
}

// (c.haag 2008-06-03 09:46) - PLID 30221 - Parses a delimited string to a long array
void ParseDelimitedStringToLongArray(const IN CString strToParse, const IN CString strDelim, OUT CArray<long, long> &aryToFill)
{
	//copy the string to start
	CString strInput = strToParse;
	while (!strInput.IsEmpty())	{
		long nDelim = strInput.Find(strDelim);

		if (nDelim == -1) {
			//Not found.  If we're still inside the loop, the string must not be empty, so
			//	there must be 1 more field after the end.
			aryToFill.Add(atoi(strInput));

			strInput.Empty();
		}
		else {
			aryToFill.Add(atoi(strInput.Left(nDelim)));

			//Truncate the input to remove what we pulled out
			strInput = strInput.Mid(nDelim + 1);
		}
	}
}
// (j.luckoski 2012-11-12 14:54) - PLID 53704 - PArses a delimited string into a string array.
void ParseDelimitedStringToStringArray(const IN CString strToParse, const IN CString strDelim, OUT CStringArray &aryToFill)
{
	//copy the string to start
	CString strInput = strToParse;
	while (!strInput.IsEmpty())	{
		long nDelim = strInput.Find(strDelim);

		if (nDelim == -1) {
			//Not found.  If we're still inside the loop, the string must not be empty, so
			//	there must be 1 more field after the end.
			aryToFill.Add(strInput);

			strInput.Empty();
		}
		else {
			aryToFill.Add(strInput.Left(nDelim));

			//Truncate the input to remove what we pulled out
			strInput = strInput.Mid(nDelim + 1);
		}
	}
}

// trim from start
inline std::string ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
inline std::string rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
inline std::string trim(std::string &s) {
	return ltrim(rtrim(s));
}

// (j.armen 2014-10-08 14:22) - PLID 63229 - Templated function allows for parsing different types
template<typename T>
std::vector<T> ParseDelimitedString(const CString& strToPars, const CString& strDelim, bool bTrim /* = true */)
{
	std::vector<T> ary;

	std::string strInput = strToPars;

	while (!strInput.empty())
	{
		long nDelim = strInput.find(strDelim);

		if (nDelim == -1)
		{
			//Not found.  If we're still inside the loop, the string must not be empty, so
			//	there must be 1 more field after the end.
			if (bTrim)
				ary.push_back(boost::lexical_cast<T>(trim(strInput)));
			else
				ary.push_back(boost::lexical_cast<T>(strInput));

			strInput.clear();
		}
		else
		{
			if (bTrim)
				ary.push_back(boost::lexical_cast<T>(trim(strInput.substr(0, nDelim))));
			else
				ary.push_back(boost::lexical_cast<T>(strInput.substr(0, nDelim)));

			//Truncate the input to remove what we pulled out
			strInput = strInput.substr(nDelim + 1);
		}
	}

	return ary;
}

// (j.armen 2014-10-08 14:22) - PLID 63229 - Valid Templates
template std::vector<std::string> ParseDelimitedString<std::string>(const CString& strToParse, const CString& strDelim, bool bTrim);
template std::vector<long> ParseDelimitedString<long>(const CString& strToParse, const CString& strDelim, bool bTrim);

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

// (j.gruber 2010-01-26 16:18) - PLID 23693 - created this if we have a -1 category for an image
//this is only called from below if its an image file and a -1 categoryID
//so, this only needs to check our new preference since we already checked the other one and it came back as -1
long GetImageCategoryID() {	

	return GetRemotePropertyInt("PatientHistoryDefaultImageCategory", -1, 0, GetCurrentUserName());

}

// (j.dinatale 2011-09-19 10:37) - PLID 45548 - gets the file's icon
CString GetFileIcon(CString &strFilePath)
{
	// (m.hancock 2006-10-16 09:07) - PLID 23053 - Determine the icon to be used (word or wav file) based on the file's extension.
	CString strIcon = SELECTION_FILE;
	CString strFileName = GetFileName(strFilePath);

	// (a.walling 2013-11-19 13:36) - PLID 59624 - Dragging and dropping an xml / ccda / etc file into the history tab will throw an exception
	// need to use the file path, not the file name, for the IsCCD/IsCancerCase etc calls.
	if(IsWaveFile(strFileName)) {
		strIcon = SELECTION_AUDIO; //Audio icon
	}else if (IsWordFile(strFileName)) {
		strIcon = SELECTION_WORDDOC; //Word icon
	}else if (NxXMLUtils::IsCCDFile(strFilePath)) { // (j.jones 2010-06-30 10:15) - PLID 38031 - added clean validation for IsCCDFile()
		strIcon = SELECTION_CCD;	
	}else if (NxXMLUtils::IsCCDAFile(strFilePath)) { 
		strIcon = SELECTION_CCDA; // (j.gruber 2013-11-08 11:10) - PLID 59375 - CCDA support
	}
	else if(NxXMLUtils::IsCancerCaseFile(strFilePath)) { //TES 11/14/2013 - PLID 57415 - Support for Cancer Case submissions
		strIcon = SELECTION_CANCERCASE;
	}

	return strIcon;
}

// (j.dinatale 2011-09-19 10:38) - PLID 45548 - attempts to copy the file over to the path given
// (b.savon 2012-05-09 12:30) - PLID 50260 - Propogate the silent flag
bool TryToCopyToHistory(const CString &strSourcePath, const CString &strDestPath, long nPatientID, HWND hwndMessageParent, long nCategoryID,
						CString &strIcon, long nPicID, LPCTSTR strNote, COleDateTime dtServiceDate, OUT CString *pstrNewFileName, 
						OUT long *pnMailSentID, IN OUT DWORD &dwLastError, bool bAttemptSilent /* = false */)
{
	bool bSucceeded = false;

	// (j.dinatale 2011-09-20 15:55) - PLID 45548 - we should really be getting the filename from the destination path
	// (j.jones 2010-12-29 15:56) - PLID 41943 - if the final filename is requested,
	// initialize it now with the filename we are attaching, if we rename it later
	// it will be reset to the new name
	if(pstrNewFileName) {
		(*pstrNewFileName) = GetFileName(strDestPath);
	}

	if (CopyFile(strSourcePath, strDestPath, TRUE)) {
		// (m.hancock 2006-10-16 09:28) - PLID 23053 - Replaced string for icon to be used (previously SELECTION_FILE).
		// (a.walling 2009-06-03 14:17) - PLID 34176
		// (b.savon 2012-05-09 12:30) - PLID 50260 - Propogate the silent flag
		long nMailID = AttachFileToHistory(strDestPath, nPatientID, hwndMessageParent, 
			nCategoryID, strIcon, NULL, nPicID, -1, strNote, dtServiceDate, bAttemptSilent);

		// (d.lange 2011-05-25 14:56) - PLID 43253 - assign our OUT parameter with the MailSent ID created
		if(pnMailSentID)
			*pnMailSentID = nMailID;

		// (j.gruber 2013-11-08 11:11) - PLID 59375 - CCDA too
		if (strIcon == SELECTION_CCD
			|| strIcon == SELECTION_CCDA) {
			try {
				// (j.jones 2014-08-04 13:33) - PLID 63157 - this now needs a patient ID
				CCD::UpdateCCDInformation(strDestPath, nMailID, nPatientID);
			} NxCatchAllThread("Error updating CCD/CCDA Descriptions");
		}

		bSucceeded = true;
	}

	dwLastError = GetLastError();
	return bSucceeded;
}

//b.spivey (November 19th, 2013) PLID 59336 - Validate the file and ensure it. Return this destination
CString ValidateImportPath(FileFolder Folder, const CString &strRootDir,  long nPatientID)
{
	// (j.dinatale 2011-09-19 10:28) - PLID 45548 - for whatever reason we werent always ensuring the right directory. Thats because the old code was
	//		reconstructing the path later in the function and not being consistent with how it was done
	CString strDestPath = GetPatientDocumentPath(GetRemoteData(), nPatientID);
	//Start by making sure the "root" folder is valid.
	if(Folder.strFolderName != "") {
		if(strRootDir == "")
			strDestPath = strDestPath ^ GetFileName(Folder.strFolderName);
		else
			strDestPath = strDestPath ^ strRootDir ^ GetFileName(Folder.strFolderName);
	}else {
		if(strRootDir != "")
			strDestPath = strDestPath ^ strRootDir;
	}

	FileUtils::EnsureDirectory(strDestPath);

	return strDestPath;
}

// (j.dinatale 2011-09-20 11:58) - PLID 45514 - added a silent flag
// (j.dinatale 2011-09-19 14:16) - PLID 45548 - refactored
// (j.gruber 2010-01-27 08:59) - PLID 23693 - added bool to see to check for ImageCategory
// (j.jones 2010-11-02 09:19) - PLID 41188 - added strNote, to override the note for the attached file
// (j.jones 2010-12-29 15:41) - PLID 41943 - added optional OUT parameter for the new file name, in the case that the name changed
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate (optional)
// (d.lange 2011-05-25 14:56) - PLID 43253 - added OUT parameter for returning the mailsent ID
BOOL ImportAndAttachFolder(FileFolder &Folder, long nPatientID, HWND hwndMessageParent, long nCategoryID /*= -1*/, const CString &strRootDir /*= ""*/, long nPicID /*= -1*/, BOOL bCheckForImageCategory /*= FALSE*/,
						   LPCTSTR strNote /*= ""*/, OUT CString *pstrNewFileName /*= NULL*/, OPTIONAL IN COleDateTime dtServiceDate /*= g_cdtInvalid*/, OUT long *pnMailSentID /*= NULL*/,
						   bool bAttemptSilent /*= false*/)
{
	
	CString strDestPath = "";
	if (nPatientID != -1) 
	{
		strDestPath = ValidateImportPath(Folder, strRootDir, nPatientID);
	}

	// (j.jones 2010-12-29 16:00) - PLID 41943 - if we were given pstrNewFileName as a parameter,
	// but attached a folder, ASSERT, because that parameter is not supported for multiple file attachments
	if(Folder.arSubFolders.GetSize() > 0 || Folder.saFileNames.GetSize() > 1) {
		ASSERT(pstrNewFileName == NULL);
	}

	//Now, any files in this folder.
	bool bCancelled = false;	
	for(int i = 0; i < Folder.saFileNames.GetSize() && !bCancelled; i++) {
		long nCurrentPatientID = nPatientID;

		// (j.dinatale 2011-09-19 10:28) - PLID 45548 - get the icon for the file with the new utility function
		CString strIcon;
		DWORD dwLastError = ERROR_SUCCESS;
		{
			CString strFile = Folder.saFileNames.GetAt(i);
			strIcon = GetFileIcon(strFile);
		}

		if(strIcon == SELECTION_CCD
			|| strIcon == SELECTION_CCDA // (j.gruber 2013-11-08 11:13) - PLID 59375 - or the ccda
		   ){
			// (a.walling 2009-06-03 14:17) - PLID 34176
			try {
				// (b.spivey, October 30, 2015) PLID 67423 - I'm fixing this under this item but there was never a reason
				//	 to throw this prompt if we know who we're importing to., 
				if (nCurrentPatientID <= 0) {
					nCurrentPatientID = CCD::ReviewAndMatchPatient(Folder.saFileNames.GetAt(i), nCurrentPatientID, CWnd::FromHandle(hwndMessageParent));
				}
				//reset the dest path with our new patientID
				strDestPath = ValidateImportPath(Folder, strRootDir, nCurrentPatientID);
				if (nCurrentPatientID == -1) 
					return FALSE;
			} NxCatchAllCall("Error importing CCDA/CCD/CCR document", {
				strIcon = SELECTION_FILE; //Default icon
			});
		}

		// (j.gruber 2010-01-26 16:16) - PLID 23693 - if we have -1 and have an image, chek the preference
		if (bCheckForImageCategory && IsImageFile(GetFileName(Folder.saFileNames.GetAt(i))) && nCategoryID == -1) {
			nCategoryID = GetImageCategoryID();
		}

		CString strSourcePath;
		if(Folder.strFolderName == "") 
			strSourcePath = Folder.saFileNames.GetAt(i);
		else
			strSourcePath = Folder.strFolderName ^ Folder.saFileNames.GetAt(i);

		// (j.dinatale 2011-09-19 10:29) - PLID 45548 - use the path that we ensured above
		CString strDestFilePath = strDestPath ^ GetFileName(Folder.saFileNames.GetAt(i));

		// (j.dinatale 2011-09-19 10:29) - PLID 45514 - if we want to attempt this silently, we get a unique file name using the new fileutils function.
		//		if for some reason it fails, it is best to let the user choose a file name.
		if(bAttemptSilent){
			// find a good filename in the format of filename(n).ext
			strDestFilePath = FileUtils::GetUniqueFileName(strDestFilePath);
		}

		// (b.savon 2012-05-09 11:07) - PLID 50260 - Propogate the silent flag
		if(!TryToCopyToHistory(strSourcePath, strDestFilePath, nCurrentPatientID, hwndMessageParent, nCategoryID, strIcon, nPicID, 
			strNote, dtServiceDate, pstrNewFileName, pnMailSentID, dwLastError, bAttemptSilent )){
				if(dwLastError == ERROR_FILE_EXISTS){
					CRenameFileDlg dlgRename(strSourcePath, strDestPath, CWnd::FromHandle(hwndMessageParent));

					try {						
						if(dlgRename.DoModal() == IDOK) {
							// (j.dinatale 2011-09-16 15:00) - PLID 45548 - if the old icon was a CCD icon, then the file is a CCD, so change the icon back to that
							//		(since strIcon was redefined playing a guessing game on this one)
							// (m.hancock 2006-10-16 09:36) - PLID 23053 - Determine the icon to be used (word or wav file) based on the file's extension.
							//we don't need to get a new icon, all we did was rename the file
							/*CString strNewIcon = GetFileIcon(dlgRename.m_strDstFullPath);
							if(strIcon == SELECTION_CCD){ 
								strNewIcon = SELECTION_CCD;
							}

							// (j.gruber 2013-11-08 11:14) - PLID 59375 - support ccda
							if (strIcon == SELECTION_CCDA)
							{
								strNewIcon = SELECTION_CCDA;
							}*/
							

							if(!TryToCopyToHistory(strSourcePath, dlgRename.m_strDstFullPath, nCurrentPatientID, hwndMessageParent, nCategoryID, strIcon, nPicID, 
								strNote, dtServiceDate, pstrNewFileName, pnMailSentID, dwLastError)){
									CString strError;
									FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
									// (j.jones 2011-01-11 15:06) - PLID 41960 - if you cancelled and you only have one file,
									// there should be no prompt here, just a warning
									if(Folder.saFileNames.GetSize() > 1) {
										if(IDYES != MsgBox(MB_YESNO, "The file '%s' could not be imported into the patient's documents folder. Windows returned the following error message:\r\n\r\n"
											"%s\r\n"
											"Would you like to continue?", Folder.saFileNames.GetAt(i), strError))
											bCancelled = true;
									}
									else {
										MsgBox("The file '%s' could not be imported into the patient's documents folder. Windows returned the following error message:\r\n\r\n"
											"%s", Folder.saFileNames.GetAt(i), strError);
										bCancelled = true;
									}
							}
						}else{
							// (j.jones 2011-01-11 15:06) - PLID 41960 - if you cancelled and you only have one file,
							// there should be no prompt here
							if(Folder.saFileNames.GetSize() > 1) {
								if(IDYES != MsgBox(MB_YESNO, "Would you like to continue with the import?")) {
									bCancelled = true;
								}
							} else {
								bCancelled = true;
							}
						}
					}NxCatchAll("GlobalUtils::ImportAndAttachFolder:RenamingFile");
				}
		}
	}


	if(bCancelled) return FALSE;

	//Now, any subfolders.
	for(i = 0; i < Folder.arSubFolders.GetSize(); i++) {
		CString strRoot;
		if(strRootDir == "") {
			strRoot = GetFileName(Folder.strFolderName);
		} else {
			strRoot = strRootDir ^ GetFileName(Folder.strFolderName);
		}

		// (j.jones 2010-12-29 16:00) - PLID 41943 - do not propagate pstrNewFileName, it is not supported on folder attachments
		if(!ImportAndAttachFolder(Folder.arSubFolders.GetAt(i), nPatientID, hwndMessageParent, nCategoryID, strRoot, nPicID, bCheckForImageCategory)) return FALSE;
	}

	return TRUE;
}

// (j.dinatale 2011-09-20 12:05) - PLID 45514 - added an attempt silent flag
// (j.gruber 2010-01-27 09:00) - PLID 23693 - added bool to check image catgory
// (j.jones 2010-10-29 12:02) - PLID 41187 - added nPicID
// (j.jones 2010-11-02 09:19) - PLID 41188 - added strNote, to override the note for the attached file
// (j.jones 2010-12-29 15:41) - PLID 41943 - added optional OUT parameter for the new file name, in the case that the name changed
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate (optional)
// (d.lange 2011-05-25 14:56) - PLID 43253 - added OUT parameter for returning the mailsent ID
BOOL ImportAndAttachFileToHistory(const CString &strFileName, long nPatientID, HWND hwndMessageParent,
								  long nCategoryID /*= -1*/, long nPicID /*= -1*/, BOOL bCheckImageCategory /*=FALSE*/,
								  LPCTSTR strNote /*= ""*/, OUT CString *pstrNewFileName /*= NULL*/,
								  OPTIONAL IN COleDateTime dtServiceDate /*= g_cdtInvalid*/, OUT long *pnMailSentID /*= NULL*/, bool bAttemptSilent /* = false */)
{
	FileFolder ff;
	ff.saFileNames.Add(strFileName);
	return ImportAndAttachFolder(ff, nPatientID, hwndMessageParent, nCategoryID, "", nPicID, bCheckImageCategory,
		strNote, pstrNewFileName, dtServiceDate, pnMailSentID, bAttemptSilent);
}

//TES 9/18/2008 - PLID 31413 - Moved EnsureDirectory() to FileUtils

//DRT 11/12/2003 - Changed to return the MailID of the item attached.  Since it previously returned a bool, we'll return 0 if the attachment failed.
//	I also updated to the code to actually be possible to fail (this was not possible previously).  No code that I can find ever looks at the
//	return value of this function.
//TES 10/29/2004 - Made this a global function; it seemed appropriate to me.  Any updating of the view will be handled by
//the tablechecker anyhow.
// (m.hancock 2006-06-27 16:04) - PLID 21071 - Added nLabStepID field for associating MailSent records with lab step records
// (j.jones 2010-11-02 09:19) - PLID 41188 - added strNote, to override the note for the attached file
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate (optional)
// (b.savon 2012-05-09 11:09) - PLID 50260 - Propogate the silent flag
long AttachFileToHistory(const CString &path, long nPatientID, HWND hwndMessageParent, long nCategoryID /*= -1*/, LPCTSTR strSelection /*= SELECTION_FILE*/, LPCTSTR strPDAFilename /*= NULL*/,
						 long nPicID /*= -1*/, long nLabStepID /*= -1*/,
						 LPCTSTR strNote /*= ""*/, OPTIONAL IN COleDateTime dtServiceDate /*= g_cdtInvalid*/, bool bAttemptSilent /* = false */)
{
	long nMailID = 0;

	try {
		CString strPathName;
		if (GetFilePath(path).CompareNoCase(GetPatientDocumentPath(GetRemoteData(), nPatientID)) == 0) {
			strPathName = GetFileName(path);
		} else {
			strPathName = path;
		}

		// (a.wetta 2007-07-09 16:49) - PLID 17467 - If this file is an image, determine if it should be added as a photo
		// (a.walling 2007-07-25 15:54) - PLID 17467 - Don't prompt if not a normal person
		// (b.savon 2012-05-09 12:31) - PLID 50260 - Propogate the silent flag
		BOOL bIsPhoto = FALSE;
		if (nPatientID != -25) {
			bIsPhoto = IsHistoryImageAPhoto(strPathName, bAttemptSilent);
		}

		// (j.jones 2010-11-02 09:22) - PLID 41188 - strNote is now
		// an optional parameter, use the path name if empty
		CString strNoteToUse = strNote;
		if(strNoteToUse.IsEmpty()) {
			strNoteToUse = strPathName;
		}
		
		// (j.jones 2008-09-04 15:29) - PLID 30288 - converted to use CreateNewMailSentEntry,
		// which creates the data in one batch and sends a tablechecker
		// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
		// (j.jones 2011-03-10 16:26) - PLID 42329 - pass in dtServiceDate, but if it is invalid, set it to null
		// (d.singleton 2013-11-15 11:18) - PLID 59513 - need to insert the CCDAType when generating a CCDA
		if(dtServiceDate.GetStatus() == COleDateTime::invalid) {
			dtServiceDate.SetStatus(COleDateTime::null);
		}
		nMailID = CreateNewMailSentEntry(nPatientID, strNoteToUse, strSelection, strPathName, GetCurrentUserName(), "", GetCurrentLocationID(), dtServiceDate, -1, nCategoryID, nPicID, nLabStepID, bIsPhoto, -1, "", ctNone);

		// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker if attaching to a PIC
		if(nPicID > 0 && nPatientID > 0) {
			//update this patient's EMR tab
			CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);
		}

	} NxCatchAllCall("Error in attaching file.", nMailID = 0;);

	// (a.walling 2008-01-29 14:31) - PLID 28716 - This has never worked correctly. Look carefully and you'll see why:
	// the result of ReverseFind should not be subtracted from the Length. However this is being removed entirely.
	// SetPropertyText("Attach History Path", path.Left(path.GetLength() - path.ReverseFind('\\')));
	
	return nMailID;
}

// (a.walling 2010-02-01 12:30) - PLID 28806 - Added bSilent and pbPrompt values to determine whether the user intends to be prompted
// Will return FALSE if bSilent and prompt is required.
BOOL IsHistoryImageAPhoto(const CString& strPathName, bool bSilent /*= false*/, bool* pbPrompt /*= NULL*/)
{
	// (a.wetta 2007-07-09 16:49) - PLID 17467 - If this file is an image, determine if it should be added as a photo
	BOOL bPhotoPrompt = FALSE;
	BOOL bIsPhoto = FALSE;
	if (pbPrompt) {
		*pbPrompt = false;
	}

	if (IsFileType(strPathName, ".jpg")  ||
		IsFileType(strPathName, ".jpeg") ||
		IsFileType(strPathName, ".jpe")  ||
		IsFileType(strPathName, ".jfif") ) {
		long nAttachJpg = GetRemotePropertyInt("HistoryAttachJpgAsPhoto", 0, 0, "<None>", true);
		if(nAttachJpg == 0)
			bIsPhoto = TRUE;
		else if(nAttachJpg == 2)
			bPhotoPrompt = TRUE;
	}
	else if (IsFileType(strPathName, ".bmp")) {
		long nAttachBmp = GetRemotePropertyInt("HistoryAttachBmpAsPhoto", 0, 0, "<None>", true);
		if(nAttachBmp == 0)
			bIsPhoto = TRUE;
		else if(nAttachBmp == 2)
			bPhotoPrompt = TRUE;
	}
	// (a.walling 2007-07-25 13:52) - PLID 17467 - Removed JPEG as a seperate pref
	else if (IsFileType(strPathName, ".tif") ||
			 IsFileType(strPathName, ".tiff") ) {
		long nAttachTif = GetRemotePropertyInt("HistoryAttachTifAsPhoto", 0, 0, "<None>", true);
		if(nAttachTif == 0)
			bIsPhoto = TRUE;
		else if(nAttachTif == 2)
			bPhotoPrompt = TRUE;
	}
	else if (IsFileType(strPathName, ".gif")) {
		long nAttachGif = GetRemotePropertyInt("HistoryAttachGifAsPhoto", 0, 0, "<None>", true);
		if(nAttachGif == 0)
			bIsPhoto = TRUE;
		else if(nAttachGif == 2)
			bPhotoPrompt = TRUE;
	}
	else if (IsFileType(strPathName, ".png")) { // (a.walling 2009-12-30 15:09) - PLID 36734 - include PNG files
		long nAttachPng = GetRemotePropertyInt("HistoryAttachPngAsPhoto", 0, 0, "<None>", true);
		if(nAttachPng == 0)
			bIsPhoto = TRUE;
		else if(nAttachPng == 2)
			bPhotoPrompt = TRUE;
	}

	// (a.walling 2010-02-01 12:32) - PLID 28806 - Don't prompt if bSilent
	if (!bSilent && bPhotoPrompt) {
		CString strPrompt;
		strPrompt.Format("Would you like to attach the image file '%s' as a photo?  This means that the image will appear in the "
					"'Photos' category of the History tab.", strPathName);
		if (IDYES == AfxMessageBox(strPrompt, MB_YESNO|MB_ICONQUESTION))
			bIsPhoto = TRUE;
	} else if (bPhotoPrompt) {
		// (a.walling 2010-02-01 12:32) - PLID 28806 - Need to prompt, but we are silent. So we return FALSE and set *pbPrompt.
		bIsPhoto = FALSE;
		if (pbPrompt) {
			*pbPrompt = true;
		}
	}

	return bIsPhoto;
}

BOOL IsWaveFile(const CString& strFullPath)
{
	if (strFullPath.GetLength() >= 4 && !strFullPath.Right(4).CompareNoCase(".wav"))
		return TRUE;
	return FALSE;
}

BOOL IsWordFile(const CString& strFullPath)
{
	if (strFullPath.GetLength() >= 4 && !strFullPath.Right(4).CompareNoCase(".doc"))
		return TRUE;
	// (a.walling 2007-07-19 10:15) - PLID 26748 - Support new word document files
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	else if (strFullPath.GetLength() >= 5 && !strFullPath.Right(5).CompareNoCase(".docx"))
		return TRUE;
	else if (strFullPath.GetLength() >= 5 && !strFullPath.Right(5).CompareNoCase(".docm"))
		return TRUE;
	return FALSE;
}

// (d.singleton 2013-02-15 15:57) - PLID 54859 copied over to FileUtils so nxserver can use
// (a.wetta 2007-07-09 17:09) - PLID 17467 - Determines if the document is an image file
// CHANGES TO THIS SHOULD ALSO CHANGE IN dbo.IsImageFile
BOOL IsImageFile(const CString& strFullPath)
{
	return FileUtils::IsImageFile(strFullPath);
}

namespace {
	shared_ptr<void> OpenFileForValidation(const CString& strFileName)
	{
		shared_ptr<void> hFile;

		DWORD dwError = 0;

		{
			HANDLE h = ::CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
			if (h && h != INVALID_HANDLE_VALUE) {
				hFile.reset(h, &::CloseHandle);
			} else {
				dwError = ::GetLastError();
			}
		}

		return hFile;
	}

	// (a.walling 2012-12-04 16:24) - PLID 54041 - A JPEG file will end with 0xFF 0xD9 (0xD9FF as a little-endian WORD)
	bool ValidateCompleteJPEG(shared_ptr<void> hFile)
	{
		if (!hFile) return false;

		DWORD dwError = 0;

		DWORD pos = ::SetFilePointer(hFile.get(), -2, NULL, FILE_END);
		if (INVALID_SET_FILE_POINTER == pos) {
			dwError = ::GetLastError();
			return false;
		}

		static const WORD jpegEof = 0xD9FF;
		WORD checkEof = 0;

		DWORD dwRead = 0;
		BOOL rc = ::ReadFile(hFile.get(), &checkEof, 2, &dwRead, NULL);

		if (!rc || dwRead != 2) {
			dwError = ::GetLastError();
			return false;
		}

		if (checkEof != jpegEof) {
			return false;
		}

		return true;
	}

	// (a.walling 2012-12-04 16:24) - PLID 54041 - A PDF file will end with %%EOF
	bool ValidateCompletePDF(shared_ptr<void> hFile)
	{
		if (!hFile) return false;

		static const long bufferSize = 64;

		DWORD dwError = 0;

		DWORD pos = ::SetFilePointer(hFile.get(), -bufferSize, NULL, FILE_END);
		if (INVALID_SET_FILE_POINTER == pos) {
			dwError = ::GetLastError();
			return false;
		}

		char checkEof[bufferSize + 1];
		memset(checkEof, 0, sizeof(checkEof));

		DWORD dwRead = 0;
		BOOL rc = ::ReadFile(hFile.get(), &checkEof, bufferSize, &dwRead, NULL);

		if (!rc || dwRead != bufferSize) {
			dwError = ::GetLastError();
			return false;
		}

		for (char* pos = checkEof; pos < checkEof + bufferSize; ++pos) {
			if (!*pos) {
				*pos = ' ';
			}
		}

		return NULL != strstr(checkEof, "%%EOF");
	}
}

// (a.walling 2012-12-04 16:24) - PLID 54041 - Tries to determine if this is a complete file (eg, not truncated). Returns true if it can't figure it out.
bool ValidateCompleteFile(const CString& strFileName)
{
	CiString strExt = FileUtils::GetFileExtension(strFileName);

	shared_ptr<void> hFile;
	bool isValid = true;
	if (strExt == "jpeg" || strExt == "jpg" || strExt == "jfif") {
		hFile = OpenFileForValidation(strFileName);
		isValid = ValidateCompleteJPEG(hFile);
	} else if (strExt == "pdf" || strExt == "fdf") {
		hFile = OpenFileForValidation(strFileName);
		isValid = ValidateCompletePDF(hFile);
	} else {
		return true;
	}

	if (isValid) {
		return true;
	}

	if (!hFile.get()) {
		return false;
	}

	__int64 createTime = 0;
	__int64 writeTime = 0;
	__int64 curTime = 0;
	if (!::GetFileTime(hFile.get(), (FILETIME*)&createTime, NULL, (FILETIME*)&writeTime)) {
		return false;
	}

	// (a.walling 2014-01-22 15:55) - PLID 60271 - If the file is more than 10 minutes old, let's just assume it is complete.
	__int64 latestTime = max(createTime, writeTime);
	::GetSystemTimeAsFileTime((FILETIME*)&curTime);

	if (curTime > latestTime) {
		__int64 diff = curTime - latestTime;

		if (diff > (10LL * 60LL * 10000000LL)) {
			return true;
		}
	}

	return false;
}

// (a.wetta 2007-07-09 17:09) - PLID 17467 - Determines if the given file is of the given file type
BOOL IsFileType(const CString& strFullPath, const CString& strFileType)
{
	// (a.walling 2007-07-19 11:55) - PLID 17467 - Fixed to work with any extension

	long nExtLength = strFileType.GetLength();

	if ((strFullPath.GetLength() >= nExtLength) && (strFullPath.Right(nExtLength).CompareNoCase(strFileType) == 0))
		return TRUE;
	return FALSE;
}

FileFolder LoadFolder(CString strFolder)
{
	FileFolder fOut;
	fOut.strFolderName = strFolder;
	CFileFind ff;
	if(!ff.FindFile(strFolder ^ "*.*")) return fOut;
	BOOL bFound = ff.FindNextFile();
	while(bFound) {
		if(ff.IsDirectory() && !ff.IsDots()) {
			fOut.arSubFolders.Add(LoadFolder(strFolder ^ ff.GetFileName()));
		}
		else if(!ff.IsDots()) {
			fOut.saFileNames.Add(ff.GetFileName());
		}
		bFound = ff.FindNextFile();
	}
	//Add the bonus file CFileFind thoughtfully provides.
	if(ff.IsDirectory() && !ff.IsDots()) {
		fOut.arSubFolders.Add(LoadFolder(strFolder ^ ff.GetFileName()));
	}
	else if(!ff.IsDots()) {
		fOut.saFileNames.Add(ff.GetFileName());
	}
	
	return fOut;
}

void GetFileNameList(FileFolder fFiles, CString &strMessage, const CString &strRoot, int &nFilesAdded, int nMaxFiles)
{
	//First, add the folder name.
	if(fFiles.strFolderName != "") {
		if(strRoot == "")
			strMessage += GetFileName(fFiles.strFolderName) + "\r\n";
		else 
			strMessage += strRoot ^ GetFileName(fFiles.strFolderName) + "\r\n";
		nFilesAdded++;
	}
	//Next, any files in this folder.
	for(int i = 0; i < fFiles.saFileNames.GetSize() && nFilesAdded < nMaxFiles; i++) {
		nFilesAdded++;
		if(strRoot == "")
			if(fFiles.strFolderName == "")
				strMessage += GetFileName(fFiles.saFileNames.GetAt(i)) + "\r\n";
			else
				strMessage += GetFileName(fFiles.strFolderName) ^ GetFileName(fFiles.saFileNames.GetAt(i)) + "\r\n";
		else
			strMessage += strRoot ^ GetFileName(fFiles.strFolderName) ^ GetFileName(fFiles.saFileNames.GetAt(i)) + "\r\n";
	}
	//Finally, any subfolders.
	for(i = 0; i < fFiles.arSubFolders.GetSize() && nFilesAdded < nMaxFiles; i++) {
		if(strRoot == "")
			GetFileNameList(fFiles.arSubFolders.GetAt(i), strMessage, GetFileName(fFiles.strFolderName), nFilesAdded, nMaxFiles);
		else
			GetFileNameList(fFiles.arSubFolders.GetAt(i), strMessage, strRoot ^ GetFileName(fFiles.strFolderName), nFilesAdded, nMaxFiles);
	}
	if(i < fFiles.arSubFolders.GetSize()) {
		//We hit the wall.
		strMessage += "<More Files>";
	}
}

void WriteFolderToDisk(FileFolder fFiles, const CString &strDestDir)
{
	for(int i = 0; i < fFiles.saFileNames.GetSize(); i++) {
		CString strFileName = fFiles.strFolderName.IsEmpty() ? fFiles.saFileNames.GetAt(i) : fFiles.strFolderName ^ fFiles.saFileNames.GetAt(i);
		if(!DoesExist(strDestDir ^ GetFileName(strFileName)) ||
			IDYES == MsgBox(MB_YESNO, "A file named %s already exists at the specified location.  Would you like to overwrite it?", fFiles.saFileNames.GetAt(i))) {
			CString strDest = strDestDir ^ GetFileName(strFileName);
			BOOL bRet = CopyFile(strFileName, strDest, TRUE);
			if(!bRet) {
				DWORD dwErr = GetLastError();
			}
		}
	}
	for(i = 0; i < fFiles.arSubFolders.GetSize(); i++) {
		WriteFolderToDisk(fFiles.arSubFolders.GetAt(i), strDestDir ^ fFiles.arSubFolders.GetAt(i).strFolderName);
	}
}

CString GetLatinToEnglishConversion(CString strLatinDescription) {

	try {

		//transform any latin notation to english version
		
		// (j.fouts 2013-04-04 09:08) - PLID 53396 - Consolidating this down so everything uses the Convert function
		CString strConversion = "";
		_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.ConvertLatinToEnglish({STRING}) AS EnglishString", strLatinDescription);
		if(!rs->eof) {
			strConversion = AdoFldString(rs, "EnglishString");
		}

		return strConversion;

	}NxCatchAll(__FUNCTION__);

	return "";
}

// (j.jones 2010-05-07 10:35) - PLID 36062 - we removed this ability, we should never be changing existing medications
/*
void UpdateMedicationsWithLatinToEnglishConversion() {

	try {

		//update existing prescriptions with the current latin->english conversion

		//TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation
		_RecordsetPtr rs = CreateRecordset("SELECT ID, PatientExplanation FROM PatientMedications");
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID",-1);
			CString strPatientExplanation = AdoFldString(rs, "PatientExplanation","");

			strPatientExplanation = GetLatinToEnglishConversion(strPatientExplanation);

			ExecuteSql("UPDATE PatientMedications SET EnglishDescription = '%s' WHERE ID = %li",_Q(strPatientExplanation),nID);

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error updating medication descriptions.");
}
*/

//TES 2/10/2009 - PLID 33002 - This turns out to be dead code.
/*void UpdateBlankMedicationsWithLatinToEnglishConversion() {

	try {

		//update existing prescriptions with the current latin->english conversion

		//TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation
		_RecordsetPtr rs = CreateRecordset("SELECT ID, PatientExplanation FROM PatientMedications WHERE Description <> '' AND EnglishDescription = ''");
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID",-1);
			CString strPatientExplanation = AdoFldString(rs, "PatientExplanation","");

			strPatientExplanation = GetLatinToEnglishConversion(strPatientExplanation);

			ExecuteSql("UPDATE PatientMedications SET EnglishDescription = '%s' WHERE ID = %li",_Q(strPatientExplanation),nID);

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error updating medication descriptions.");
}*/

long CalcSysIconTitleFontHeight()
{
    // Create a scratch device context
    CDC dc;
	//DRT 10/9/2007 - PLID 27704 - This was leaking GDI resources.  This is the old code:
	//	dc.Attach(::CreateCompatibleDC(::GetDC(NULL)));
	//The problem is that the innermost GetDC(NULL) creates a new DC that must be released.  The 2ndary call to 
	//	CreateCompatibleDC returns a *2nd* device context.  We then Attach() that into 'dc', which means the 
	//	'dc' object is now responsible for cleaning it up.  But the original call to GetDC still has a hanging 
	//	device context.  We can release the original DC as soon as we create the copy, it's no longer needed.
	HDC hdcSource = ::GetDC(NULL);
	HDC hdcCompatible = ::CreateCompatibleDC(hdcSource);

	//hdcSource is no longer necessary, we've made a compatible copy of it.
	::ReleaseDC(NULL, hdcSource);

	//After this, hdcCompatible is controlled by 'dc', and we do not need to manually release hdcCompatible.
	dc.Attach(hdcCompatible);
 
    // Select the correct system font into that dc
    CFont font, *pFontOld;
    {
        LOGFONT lfInfo;
        if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), (PVOID)&lfInfo, NULL)) {
            font.CreateFontIndirect(&lfInfo);
        } else {
            //TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
			CreateCompatiblePointFont(&font, 90, "MS Sans Serif", &dc);
        }
        pFontOld = dc.SelectObject(&font);
    }
 
    // Get the height of that font on this device context
    long nAns;
    {
        TEXTMETRIC tmTextMetric;
        dc.GetOutputTextMetrics(&tmTextMetric);
        nAns = tmTextMetric.tmHeight;
    }
 
    // Free everything
    dc.SelectObject(pFontOld);
    font.DeleteObject();
 
    // Return the anser
    return nAns;
}

// Given a string, calculate the ideal width of the text when outputted using the system font
long CalcSysIconTitleFontWidth(CString str)
{
	// Create a scratch device context
    CDC dc;
	//DRT 10/9/2007 - PLID 27704 - This was leaking GDI resources.  This is the old code:
	//	dc.Attach(::CreateCompatibleDC(::GetDC(NULL)));
	//The problem is that the innermost GetDC(NULL) creates a new DC that must be released.  The 2ndary call to 
	//	CreateCompatibleDC returns a *2nd* device context.  We then Attach() that into 'dc', which means the 
	//	'dc' object is now responsible for cleaning it up.  But the original call to GetDC still has a hanging 
	//	device context.  We can release the original DC as soon as we create the copy, it's no longer needed.
	HDC hdcSource = ::GetDC(NULL);
	HDC hdcCompatible = ::CreateCompatibleDC(hdcSource);

	//hdcSource is no longer necessary, we've made a compatible copy of it.
	::ReleaseDC(NULL, hdcSource);

	//After this, hdcCompatible is controlled by 'dc', and we do not need to manually release hdcCompatible.
	dc.Attach(hdcCompatible);
 
    // Select the correct system font into that dc
    CFont font, *pFontOld;
    {
        LOGFONT lfInfo;
        if (SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), (PVOID)&lfInfo, NULL)) {
            font.CreateFontIndirect(&lfInfo);
        } else {
            //TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
			CreateCompatiblePointFont(&font, 90, "MS Sans Serif", &dc);
        }
        pFontOld = dc.SelectObject(&font);
    }
 
    // Get the height of that font on this device context
    long nAns;
    {
		CRect rc(0,0,0,0);
        dc.DrawText(str,&rc,DT_LEFT|DT_CALCRECT|DT_SINGLELINE);
        nAns = rc.Width();
    }
 
    // Free everything
    dc.SelectObject(pFontOld);
    font.DeleteObject();
 
    // Return the anser
    return nAns;
}

bool LoadImageFile(CString &strFile, HBITMAP &hImage, long nPersonID)
{
	CString strFileName;
	if(nPersonID == -1) {
		strFileName = strFile;
	}
	else {
		strFileName = GetPatientDocumentPath(GetRemoteData(), nPersonID) ^ strFile;
		//TES 2/9/2005 - Sometimes absolute paths are attached to patient's accounts.
		if(!DoesExist(strFileName)) {
			strFileName = strFile;
		}
	}

	// (a.walling 2008-10-13 12:06) - PLID 31669 - Use NxGdiPlus' LoadImageFile
	return NxGdi::LoadImageFile(strFileName, hImage);
}

bool IsWindowModal(HWND hwnd)
{
  HWND hwndOwner = GetNextWindow(hwnd, GW_OWNER);
  if (hwndOwner == NULL)
     return false;
  return !(IsWindowEnabled(hwndOwner));
}

void UpdateASCLicenseToDos()
{
	try {

		if(!IsSurgeryCenter(FALSE))
			return;

		long nDefUserID = GetRemotePropertyInt("DefaultASCLicenseWarnUser",-1,0,"<None>",true);
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		if(nDefUserID == -1 || !ReturnsRecordsParam("SELECT PersonID FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID WHERE PersonID = {INT} AND Archived = 0",nDefUserID)) {
			//not a valid user
			return;
		}

		long nDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

		//category ID to be used later
		long nCatID = -1;

		//loop through all expired or soon-to-expire licenses, and create To-Dos for those licenses
		// (j.jones 2009-09-10 10:29) - PLID 28872 - only load up the certifications for active people
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonCertificationsT.ID, PersonCertificationsT.PersonID, "
			"PersonCertificationsT.Name, PersonCertificationsT.ExpDate, TodoListQ.EnteredBy "
			"FROM PersonCertificationsT "
			"INNER JOIN PersonT ON PersonCertificationsT.PersonID = PersonT.ID "
			"LEFT JOIN (SELECT TodoList.EnteredBy, TodoList.RegardingID FROM TodoList WHERE TodoList.RegardingType = {INT}) AS TodoListQ "
			"ON PersonCertificationsT.ID = TodoListQ.RegardingID "
			"WHERE PersonT.Archived = 0 "
			"AND PersonCertificationsT.ExpDate <= DateAdd(day,{INT},CONVERT(datetime,CONVERT(nvarchar, GetDate(), 101)))", ttLicenseCertification, nDayRange);

		//Clear previous todo items for licenses
		long nRecordsAffected = 0;
		// (c.haag 2008-06-10 17:45) - PLID 30328 - Also need to delete from TodoAssignToT
		ExecuteParamSql("DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE RegardingType = {INT})", ttLicenseCertification);
		ExecuteSql(&nRecordsAffected, adCmdText, "DELETE FROM TodoList WHERE RegardingType = %i", ttLicenseCertification);

		BOOL bRefreshAllTodos = FALSE;

		if (nRecordsAffected != 0) {
			//we deleted todo alarms so we have to update all of them
			//but we will do so later
			bRefreshAllTodos = TRUE;
		}

		while(!rs->eof) {

			long nLicenseID = AdoFldLong(rs, "ID");
			long nPersonID = AdoFldLong(rs, "PersonID");
			CString strLicenseName = AdoFldString(rs, "Name", "");
			COleDateTime dtExpDate = AdoFldDateTime(rs, "ExpDate");
			//nEnteredBy should default to -1, which represents the current user
			long nEnteredBy = -1;
			//if the todo was already in the list, we should use the former EnteredBy value
			if (!AdoFldNull(rs, "EnteredBy")) {
				nEnteredBy = AdoFldLong(rs, "EnteredBy");
			}

			//load the nCatID only once
			if(nCatID == -1) {
				_RecordsetPtr rs = CreateRecordset("SELECT ID FROM NoteCatsF WHERE Description = 'Licensing'");
				if(!rs->eof) {
					nCatID = rs->Fields->Item["ID"]->Value.lVal;
				}
				else {
					//TES 8/1/2011 - PLID 44716 - Moved to GlobalUtils function
					nCatID = CreateNoteCategory("Licensing");
				}
				rs->Close();
			}
			
			// (c.haag 2008-06-09 11:07) - PLID 30321 - Use TodoCreate instead of an insert query
			//if(nDayRange > 0)
			//	nDayRange = -nDayRange;
			long nTaskID = TodoCreate(dtExpDate - COleDateTimeSpan(abs(nDayRange),0,0,0), dtExpDate, nDefUserID, 
				strLicenseName, "", nLicenseID, ttLicenseCertification, nPersonID, -1, ttpLow, nCatID, (DATE)0, nEnteredBy);

			//if we aren't already planning to refresh all Todos, then
			//refresh only each Todo that is created
			// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
			if(!bRefreshAllTodos) {
				CClient::RefreshTodoTable(nTaskID, nPersonID, nDefUserID, TableCheckerDetailIndex::tddisAdded);
			}

			rs->MoveNext();
		}
		rs->Close();

		//if we needed to refresh all Todos, do it now
		if(bRefreshAllTodos) {
			CClient::RefreshTable(NetUtils::TodoList, -1);
		}

	}NxCatchAll("Error updating licensing To-Do alarms.");
}

// (a.walling 2010-11-08 11:53) - PLID 40965 - Parameterize
_variant_t GetTableField(const CString &strTable, const CString &strField, const CString &strIDField, long nID)
{
	_RecordsetPtr rs = CreateParamRecordset("SELECT {CONST_STR} FROM {CONST_STR} WHERE {CONST_STR} = {INT}", strField, strTable, strIDField, nID);
	if(rs->eof) {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		return varNull;
	}
	else {
		return rs->Fields->GetItem(0L)->GetValue();
	}
}

ECredentialWarning CheckServiceCodeCredential(long nProviderID, long nServiceCodeID)
{
	ECredentialWarning eCurWarning = ePassedAll;

	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(!ReturnsRecordsParam("SELECT CPTCodeID FROM CredentialedCPTCodesT WHERE ProviderID = {INT} AND CPTCodeID = {INT}",nProviderID,nServiceCodeID)) {
		//not credentialed for this code

		eCurWarning = eFailedCredential;

		//see if the provider is credentialed for a procedure using this code, and if not, return eFailedCredential
		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam("SELECT ProcedureID FROM CredentialedProceduresT WHERE ProviderID = {INT} AND ProcedureID IN (SELECT ProcedureID FROM ServiceT WHERE ID = {INT})",nProviderID,nServiceCodeID)) {
			//this means they ARE credentialed for this code by way of being credentialed for the procedure,
			//so the code did not fail after all
			eCurWarning = ePassedAll;
		}

		//return now if it has failed
		if(eCurWarning != ePassedAll)
			return eCurWarning;
	}

	//obviously we are returning ePassedAll otherwise, but for future code
	//is smarter if we just return eCurWarning only if it did not pass
	if(eCurWarning != ePassedAll)
		return eCurWarning;

	return ePassedAll;
}

ECredentialWarning CheckProcedureCredential(long nProviderID, long nProcedureID)
{
	ECredentialWarning eCurWarning = ePassedAll;

	if(nProviderID == -1 || nProcedureID == -1)
		//invalid data, so leave now
		return ePassedAll;

	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	if(!ReturnsRecordsParam("SELECT ProcedureID FROM CredentialedProceduresT WHERE ProviderID = {INT} AND ProcedureID = {INT}",nProviderID,nProcedureID)) {		
		//not credentialed for this procedure
		return eFailedCredential;
	}

	//obviously we are returning ePassedAll otherwise, but for future code
	//is smarter if we just return eCurWarning only if it did not pass
	if(eCurWarning != ePassedAll)
		return eCurWarning;

	return ePassedAll;
}

ECredentialWarning CheckPersonCertifications(long nPersonID)
{
	//check the status of licenses for the given person

	if(nPersonID == -1)
		//invalid data, so leave now
		return ePassedAll;

	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	if(ReturnsRecordsParam("SELECT ID FROM PersonCertificationsT WHERE PersonID = {INT}",nPersonID)) {
		//they have some sort of license, so perform our checks

		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam("SELECT ID FROM PersonCertificationsT WHERE PersonID = {INT} AND "
			"ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nPersonID)) {

			//they have at least one license that expired on or before yesterday
			return eFailedLicenseExpired;
		}

		//check if a license will expire within the given day range
		long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam("SELECT ID FROM PersonCertificationsT WHERE PersonID = {INT} AND "
			"ExpDate < DateAdd(day,{INT},Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nPersonID, nLicenseWarnDayRange)) {

			//they have at least one license that will expire within the set date range
			return eFailedLicenseExpiringSoon;
		}
	}

	return ePassedAll;
}

BOOL g_bIgnoreRightAlt = FALSE;
BOOL g_bIsRightAltStatusSet = FALSE;

BOOL IgnoreRightAlt() {

	if(!g_bIsRightAltStatusSet) {
		g_bIgnoreRightAlt = GetRemotePropertyInt("IgnoreRightAlt", 0, 0, "<None>", true);
		g_bIsRightAltStatusSet = TRUE;
	}

	return g_bIgnoreRightAlt;
}

CString FormatNumericString(const CString &strInput, const CString &strFormat)
{
	//First, just get the numbers.
	CString strInputNums;
	for(int i = 0; i < strInput.GetLength(); i++) {
		if(strInput.GetAt(i) >= '0' && strInput.GetAt(i) <= '9') strInputNums += strInput.GetAt(i);
	}

	//Now, how many numbers do we need.
	int nNumCount = 0;
	for(i = 0; i < strFormat.GetLength(); i++) {
		if(strFormat.GetAt(i) == '#' || strFormat.GetAt(i) == 'n') nNumCount++;
	}

	//Get the rightmost set of numbers.
	// (z.manning 2016-02-17 10:19) - PLID 40375 - Not sure who thought it was a good idea to arbitrarily
	// delete data, but let's not. If we have more numbers than we can handle, just return the unformatted string.
	//strInputNums = strInputNums.Right(nNumCount);
	if (strInputNums.GetLength() > nNumCount) {
		return strInput;
	}

	//Now, go through and replace.
	int nNumIndex = 0;
	CString strOut;
	for(i = 0; i < strFormat.GetLength(); i++) {
		if(strFormat.GetAt(i) == '#') {
			if(nNumIndex < strInputNums.GetLength()) {
				strOut += strInputNums.GetAt(nNumIndex);
			}
			else {
				strOut += '#';
			}
			nNumIndex++;
		}
		else if(strFormat.GetAt(i) == 'n') {
			if(nNumIndex < strInputNums.GetLength()) {
				strOut += strInputNums.GetAt(nNumIndex);
			}
			else {
				strOut += ' ';
			}
			nNumIndex++;
		}
		else {
			strOut += strFormat.GetAt(i);
		}
	}

	// (z.manning 2016-02-16 15:28) - PLID 40375 - Most places that called this function were doing this
	// and those that wern't should have been. So let's save callers the trouble and do this here so we
	// don't get random #s saved to data for no good reason.
	strOut.Replace(strFormat, "");
	strOut.Replace("#", "");

	return strOut;
}

CString FormatPhone(const CString &strPhone, const CString &strFormat)
{
	return FormatNumericString(strPhone, strFormat);
}

CString FormatSSN(const CString &strSSN, OPTIONAL const CString &strFormat /* = "###-##-####" */)
{
	return FormatNumericString(strSSN, strFormat);
}


void PreviewQuote(long nQuoteID, CString strExtraDesc, bool bIsPackage, CWnd *pParent, long nReportID, long nReportNumber /*= -2*/, CString strReportFileName /*= ""*/)
{
	long nCustomReportNumber = nReportNumber;
	CString strCustomReportFileName = strReportFileName;
	if(nReportNumber == -2) {
		// (j.gruber 2007-03-26 14:34) - PLID 25343 - fix the user custom quote - this should be here
		//check to see if there is a user default
		// (j.gruber 2007-08-14 11:10) - PLID 27068 - added report number since there are multiple now
		// (j.gruber 2007-11-20 12:21) - PLID 28144 - separated out regular and custom user defaults
		CString strFileName = "";
		CString strByUserName;
		CString strByUserReportID;
		long nTempReportID;
		if (bIsPackage) {
			strByUserName = "PackageQuoteDefaultByUser";
			strByUserReportID = "PackageQuoteDefaultByUserReportID";
			nTempReportID = 608;
		}
		else {
			strByUserName = "QuoteDefaultByUser";
			strByUserReportID = "QuoteDefaultByUserReportID";
			nTempReportID = 227;
		}

		
		strFileName = GetRemotePropertyText(strByUserName, "", 0, GetCurrentUserName(), FALSE);
		long nNumber = GetRemotePropertyInt(strByUserName, -2, 0, GetCurrentUserName(), FALSE);
		long nID = GetRemotePropertyInt(strByUserReportID, nTempReportID, 0, GetCurrentUserName(), FALSE);
		if ((! strFileName.IsEmpty()) && nNumber != -2) {

			nCustomReportNumber = nNumber;
			strCustomReportFileName = strFileName;
			nReportID = nID;
		}
		else {

			//check to see if there is a default report
			// (j.gruber 2007-03-26 14:53) - PLID 25343 - I took this out because it was unneccesary and confusing
			//CString strCustomReportFilename;
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rsDefault = CreateParamRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = {INT}", nReportID);
			if(!rsDefault->eof) {
				nCustomReportNumber = AdoFldLong(rsDefault, "CustomReportID", -1);
				if(nCustomReportNumber != -1) {
					//get the filename
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr rsFileName = CreateParamRecordset("SELECT Filename FROm CustomReportsT WHERE ID = {INT} and Number = {INT}", nReportID, nCustomReportNumber);
					if (rsFileName->eof) {
						//houston, we have a problem
						MessageBox(pParent->GetSafeHwnd(), "Error generating report, please select the default report again", NULL, MB_OK);
						return;
					}
					else {
						strCustomReportFileName = AdoFldString(rsFileName, "Filename");
					}
				}
			}
		}
	}
	//crystal report
	CPtrArray params;
	CRParameterInfo *tmpParam;
	
	//sets up params to quote
	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuotePatCoord";
	if (GetRemotePropertyInt("QuotePatCoord", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else if (GetRemotePropertyInt("QuotePatCoord", 2, 0, "<None>") == 1) {
		tmpParam->m_Data = "1";
	}
	else if (GetRemotePropertyInt("QuotePatCoord", 2, 0, "<None>") == 2){
		tmpParam->m_Data = "2";
	}
	params.Add((void *)tmpParam);


	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuotePaidToOthers";
	if (GetRemotePropertyInt("QuotePaidOthers", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else if (GetRemotePropertyInt("QuotePaidOthers", 2, 0, "<None>") == 1) {
		tmpParam->m_Data = "1";
	}
	else if (GetRemotePropertyInt("QuotePaidOthers", 2, 0, "<None>") == 2){
		tmpParam->m_Data = "2";
	}
	params.Add((void *)tmpParam);


	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteDiscount";
	if (GetRemotePropertyInt("QuoteDiscount", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else if (GetRemotePropertyInt("QuoteDiscount", 2, 0, "<None>") == 1) {
		tmpParam->m_Data = "1";
	}
	else if (GetRemotePropertyInt("QuoteDiscount", 2, 0, "<None>") == 2){
		tmpParam->m_Data = "2";
	}
	params.Add((void *)tmpParam);


	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuotePercent";
	if (GetRemotePropertyInt("QuotePercent", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else if (GetRemotePropertyInt("QuotePercent", 2, 0, "<None>") == 1) {
		tmpParam->m_Data = "1";
	}
	else if (GetRemotePropertyInt("QuotePercent", 2, 0, "<None>") == 2){
		tmpParam->m_Data = "2";
	}
	params.Add((void *)tmpParam);
	
	
	
	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteQuantity";
	if (GetRemotePropertyInt("QuoteQuantity", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else if (GetRemotePropertyInt("QuoteQuantity", 2, 0, "<None>") == 1) {
		tmpParam->m_Data = "1";
	}
	else if (GetRemotePropertyInt("QuoteQuantity", 2, 0, "<None>") == 2){
		tmpParam->m_Data = "2";
	}
	params.Add((void *)tmpParam);


	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteUsePatSig";
	if (GetRemotePropertyInt("QuoteUsePatSig", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else { 
		tmpParam->m_Data = "1";
	}
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteUseWitSig";
	if (GetRemotePropertyInt("QuoteUseWitSig", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else {
		tmpParam->m_Data = "1";
	}
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteUseEmail";
	if (GetRemotePropertyInt("QuoteUseEmail", 2, 0, "<None>") == 0) {
		tmpParam->m_Data = "0";
	}
	else {
		tmpParam->m_Data = "1";
	}
	params.Add((void *)tmpParam);
	
	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteExtraText";
	tmpParam->m_Data = strExtraDesc;
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteShowDescription";
	tmpParam->m_Data = GetRemotePropertyInt("QuoteShowDescription", 1, 0, "<None>", true) ? "true" : "false";
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteIsPackage";
	tmpParam->m_Data = bIsPackage ? "true" : "false";
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteSeparateTotals";
	tmpParam->m_Data = GetRemotePropertyInt("QuoteSeparateTotals", 0, 0, "<None>", true) ? "true" : "false";	//DRT 2/13/2004 - PLID 10995 - This should be default to 0 like is in the quote admin!
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteUseOtherPhone";
	tmpParam->m_Data = GetRemotePropertyInt("QuoteUseOtherPhone", 0, 0, "<None>", true) ? "true" : "false";		//DRT 2/13/2004 - PLID 10995 - This should be default to 0 like is in the quote admin!
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteOtherPracPhone";
	tmpParam->m_Data = GetRemotePropertyText("QuoteOtherPracPhone", "", 0, "<None>", true);
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteUseQuoteDate";
	tmpParam->m_Data = GetRemotePropertyInt("QuoteUseQuoteDate", 1, 0, "<None>", true) ? "true" : "false";
	params.Add((void *)tmpParam);

	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteShowProcDescriptions";
	tmpParam->m_Data = GetRemotePropertyInt("QuoteShowProcDescriptions", 1, 0, "<None>", true) ? "true" : "false";		//JMM 06/30/2005 - Added procedure descriptions to the quote
	params.Add((void *)tmpParam);

	// (j.gruber 2009-03-19 17:44) - PLID 33349 - quote setting for how to list discounts
	tmpParam = new CRParameterInfo;
	tmpParam->m_Name = "QuoteShowDiscountsByCategory";
	tmpParam->m_Data = GetRemotePropertyInt("QuoteShowDiscountsByCategory", 0, 0, "<None>", true) ? "true" : "false";		
	params.Add((void *)tmpParam);



	CReportInfo riQuote = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)];
	riQuote.nExtraID = nQuoteID;
	
	// (j.gruber 2007-03-26 14:34) - PLID 25343 - fix the user custom quote - took out an if
	if (nCustomReportNumber == -1) {

		//they want to run the system report
		//There are no summary/detail options or anything to worry about for the quote.

		if (nReportID == 227) {
			if (GetRemotePropertyInt("QuoteUseAlternate", 0,0, "<None>") == 0 ) {
				riQuote.ViewReport("Quote Report", "Quote", &params, true, pParent);
			}
			else {
				riQuote.ViewReport("Quote Report", "QuoteAlternate", &params, true, pParent);
			}
		}
		else {
			ASSERT(nReportID == 608);
			riQuote.ViewReport("Package Quote", "PackageQuote", &params, true, pParent);
		}

	}
	else {

		//check to see if it is a custom report
		if (nCustomReportNumber > 0) {

			//tell the reportinfo object that we're using a custom report.
			riQuote.nDefaultCustomReport = nCustomReportNumber;

			CString strTitle;
			if (nReportID == 227) {
				strTitle = "Quote Report";
			}
			else {
				strTitle = "Package Quote";
			}

			//run the report
			riQuote.ViewReport(strTitle, strCustomReportFileName, &params, true, pParent);
		}
		else {
			//They have a customreportid of 0, which shouldn't happen.  Let's assert, 
			//but then let's go ahead and run the default.
			if (nReportID == 227) {
				if (GetRemotePropertyInt("QuoteUseAlternate", 0,0, "<None>") == 0 ) {
					riQuote.ViewReport("Quote Report", "Quote", &params, true, pParent);
				}
				else {
					riQuote.ViewReport("Quote Report", "QuoteAlternate", &params, true, pParent);
				}
			}
			else {
				riQuote.ViewReport("Package Quote", "PackageQuote", &params, true, pParent);
			}

		}
		
	}

	for (long i=0; i < params.GetSize(); i++)
		if (params[i]) delete ((CRParameterInfo*)params[i]);
	params.RemoveAll();
}

void PreviewBill(long nBillID, CWnd *pParentWnd)
{
	CReportInfo riBill = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(226)];
	riBill.nExtraID = nBillID;
	if(IsNexTechInternal())
		riBill.ViewReport("Bill Report", "BillingDialogInternal", true, pParentWnd);	//if we're internal, use this report, it's got some extra jazz
	else {
		//check to see if there is a default report
		_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 226");
		if (rsDefault->eof) {
			riBill.ViewReport("Bill Report", "BillingDialog", true, pParentWnd);
		}
		else {
			//there must be a default custom report so find the filename of it
			long nDefault = AdoFldLong(rsDefault, "CustomReportID", -1);
			if(nDefault == -1) {
				//Their "default" report is the system one.  If this report had any options, we would fill them in here
				//from DefaultReportsT.
				riBill.ViewReport("Bill Report", "BillingDialog", true, pParentWnd);
			}
			else {

				//get the filename
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr rsFileName = CreateParamRecordset("SELECT Filename FROM CustomReportsT WHERE ID = {INT} and Number = {INT}", 226, nDefault);
				if (rsFileName->eof) {
					//houston, we have a problem
					MessageBox(pParentWnd->GetSafeHwnd(), "Error generating report, please select the default report again", NULL, MB_OK);
					return;
				}
				else {
					riBill.nDefaultCustomReport = nDefault;
					CString strFileName = AdoFldString(rsFileName, "Filename");
					riBill.ViewReport("Bill Report", strFileName, true, pParentWnd);
				}
			}
		}
	}
}

BOOL RunTroubleshooter()
{
	// Run the troubleshooter
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);		
	CString strCmd;
	strCmd.Format("%s -ver %s", GetCommandLine(), NXSERVER_VERSION);
	PROCESS_INFORMATION pi;
	DWORD dwExitCode;
	if (CreateProcess("troubleshoot.exe", (char*)(LPCTSTR)strCmd, NULL, NULL, false, 0, NULL, GetSharedPath(), &si, &pi))
	{
		{
			CWaitCursor wc;
			WaitForSingleObject(pi.hProcess, INFINITE);
		}

		// Get the exit code and see if the problem was fixed.
		// A return code of zero means it was fixed.
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		if (dwExitCode)
		{
			// Failure -- troubleshooter could not fix the problem
			return FALSE;
		} else {
			// Troubleshooter proclaimed success, but what if the troubleshooter 
			// is wrong?  We don't want the possibility of getting the user stuck 
			// in an interface loop.  We need to give him the option of escape.
			if (AfxMessageBox(
				"The troubleshooter reports that your connection information is "
				"correct.  Click OK to try to connect again.", 
				MB_OKCANCEL|MB_ICONINFORMATION) != IDOK) {
				return FALSE;
			}
		}
	}
	else {
		// Failure -- could not spawn troubleshooter
		return FALSE;
	}
	return TRUE;
}

BOOL OpenDocument(CString filename, long nPatientID)
{
	//this is the conventional way of opening an attached file
	if (filename.Find('\\') == -1) {
		// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
		filename = GetPatientDocumentPath(GetRemoteData(), nPatientID) ^ filename;
	}

	return OpenDocument(filename);
}

// (z.manning 2016-02-11 09:49) - PLID 68223 - Overload to simple open a file without the patient logic
BOOL OpenDocument(CString filename)
{
	// If we are trying to open a Word document and are using virtual channels to perform Microsoft Word actions, then
	// perform the open using the virtual channel Word library
	CiString strExt = FileUtils::GetFileExtension(filename);
	if ("doc" == strExt || "docx" == strExt)
	{
		if (WordProcessorType::VTSCMSWord == GetWordProcessorType())
		{
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp)
			{
				// Could not get a word processor instance and an exception was not thrown. Lets throw one ourselves
				// since this is an unexpected failure and we don't want to leave the user guessing what's going on
				ThrowNxException("Failed to get a virtual channel word processor instance");
			}
			else
			{
				pApp->EnsureValid();
				return pApp->OpenDocument(filename);
			}
		}
		else
		{
			// Not using virtual channels for Word. Open with the shell.
		}
	}
	else
	{
		// Not a Word document. Open with the shell.
	}

	// (z.manning, 04/27/2007) - PLID 9793 - Use ShellExecuteEx instead of ShellExecute to open the document.
	SHELLEXECUTEINFO ExecInfo;
	memset(&ExecInfo, 0, sizeof(SHELLEXECUTEINFO));
	ExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ExecInfo.fMask = SEE_MASK_FLAG_NO_UI;
	ExecInfo.hwnd = GetDesktopWindow();
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	ExecInfo.lpVerb = NULL;
	ExecInfo.lpFile = filename;
	// (a.walling 2009-11-20 08:10) - PLID 36373 - The problem here is that the CString returned from GetFilePath is destroyed immediately after
	// this line. Therefore the LPCTSTR returned is immediately invalidated and therefore points to invalid memory. The proper solution is to
	// hold the value in a local CString so it won't be freed until it goes out of scope, and therefore remains valid until the ShellExecute call
	// has entirely completed. One odd thing about this issue is that sometimes it causes no problems, but other time it ends up messing up 
	// memory such that the actual access violation is triggered elsewhere in Practice.
	//ExecInfo.lpDirectory = GetFilePath(filename);
	CString strDirectory = GetFilePath(filename);
	ExecInfo.lpDirectory = strDirectory;
	// (a.walling 2009-11-16 13:56) - PLID 36306 - This should be SW_SHOWDEFAULT and we should let the application decide how to display to the user
	//ExecInfo.nShow = SW_MAXIMIZE;
	ExecInfo.nShow = SW_SHOWDEFAULT;
	ShellExecuteEx(&ExecInfo);

	if ((int)ExecInfo.hInstApp < 32)
	{
		if ((int)ExecInfo.hInstApp != ERROR_FILE_NOT_FOUND && (int)ExecInfo.hInstApp != ERROR_PATH_NOT_FOUND)
		{
			// (c.haag 2003-08-05 12:29) - Try again (the prompt won't appear if it's not a UNC path)
			//if (ERROR_CANCELLED == DoNetworkLogin(GetFilePath(filename)))
			if (NO_ERROR == TryAccessNetworkFile(filename)) {
				ShellExecuteEx(&ExecInfo);
				if ((int)ExecInfo.hInstApp >= 32) {
					return TRUE;
				}
			}
			else {
				return FALSE;
			}
		}

		// CAH 3/26/02: ///////////////////////////////////////////////////////////
		//
		// Failed to open the document. Lets so some hard core searching.
		//
		///////////////////////////////////////////////////////////////////////////
		CFileFind ff;
		CString strFile = filename.Right(filename.GetLength() - filename.ReverseFind('\\') - 1);
		CString strRootSearchPath = GetSharedPath() ^ "Documents";
		CString strFoundInPath;
		BOOL bSuccess = FALSE;

		// Is it a multipatient document?
		if (-1 != strFile.Find("MultiPatDoc"))
		{
			// Yes it is, try to open it
			// (z.manning, 04/27/2007) - PLID 9793 - Use ShellExecuteEx instead of ShellExecute to open the document.
			CString strMultiDocPath = strRootSearchPath ^ "---25" ^ strFile;
			ExecInfo.lpFile = strMultiDocPath;
			ShellExecuteEx(&ExecInfo);
			if ((int)ExecInfo.hInstApp >= 32) {
				return TRUE; // Yes it opened, lets get out of here.
			}
		}

		//TES 10/8/2004 - I'm taking out this feature.  It's too dangerous; at Gangnes' office it was subsituting random 
		//patient's images for images that had been moved.
		/*// It's not multipatient, OR it's multipatient but it failed. So, lets
		// make sure we know where the file is.
		if (FindFileRecursively(strRootSearchPath, strFile, strFoundInPath))
		{
		// Yeah, we found it. Lets move the file to the right place. If it's a
		// multipatient document, move it to the ---25 folder.
		if (-1 != strFile.Find("MultiPatDoc"))
		{
		if (MoveFile(strFoundInPath ^ strFile, strRootSearchPath ^ "---25" ^ strFile))
		{
		// The move worked, lets try to open the document in it's new place.
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		bSuccess = ((int)ShellExecute(GetDesktopWindow(), NULL, strRootSearchPath ^ "---25" ^ strFile, NULL, GetFilePath(filename), SW_MAXIMIZE) < 32) ? FALSE : TRUE;
		}
		else
		{
		// The move failed; but we DO know where the document is. Lets ask the user if
		// he/she wants to open the document anyway.
		if (IDYES == MessageBox("Practice found the document outside the multi-patient documents folder, but was unable to move it there. Do you want to open the document from where it is?", NULL, MB_YESNO))
		{
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		bSuccess = ((int)ShellExecute(GetDesktopWindow(), NULL, strFoundInPath ^ strFile, NULL, GetFilePath(filename), SW_MAXIMIZE) < 32) ? FALSE : TRUE;
		}
		}
		}
		// Nope, it's a single patient document; move it to the patient's folder.
		else
		{
		if (MoveFile(strFoundInPath ^ strFile, filename))
		{
		// The move worked, lets try to open the document in it's new place.
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		bSuccess = ((int)ShellExecute(GetDesktopWindow(), NULL, filename, NULL, GetFilePath(filename), SW_MAXIMIZE) < 32) ? FALSE : TRUE;
		}
		else
		{
		// The move failed; but we DO know where the document is. Lets ask the user if
		// he/she wants to open the document anyway.
		if (IDYES == MessageBox("Practice found the document outside the person's folder, but was unable to move it there. Do you want to open the document from where it is?", NULL, MB_YESNO))
		{
		// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
		bSuccess = ((int)ShellExecute(GetDesktopWindow(), NULL, strFoundInPath ^ strFile, NULL, GetFilePath(filename), SW_MAXIMIZE) < 32) ? FALSE : TRUE;
		}
		}
		}
		}*/
		return bSuccess;
	}
	else {
		return TRUE;
	}
}

BOOL CheckAllergies(long nPatientID, long nMedicationID, IN BOOL bSilent /*= FALSE*/, OPTIONAL OUT CString *pstrAllergies /*= NULL*/, OPTIONAL OUT int *pnAllergyCount /*= NULL*/)
{
	// (c.haag 2007-04-03 14:47) - PLID 25482 - We now get the allergy name from EmrDataT
	// since AllergyT.Name has been depreciated
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr rsAllergies = CreateParamRecordset("SELECT [Data] AS Name FROM PatientAllergyT "
		"INNER JOIN DrugAllergyT ON PatientAllergyT.AllergyID = DrugAllergyT.AllergyID "
		"INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
		"INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID "
		"WHERE PatientAllergyT.PersonID = {INT} AND DrugAllergyT.DrugID = {INT} "
		"GROUP BY [Data]", nPatientID, nMedicationID);
	if(!rsAllergies->eof) {
		//OK, they are allergic.  Get the allergies and allergy count, if we need them.
		CString strAllergies = "";
		int nAllergyCount = 0;			
		if(!bSilent || pstrAllergies || pnAllergyCount) {
			while(!rsAllergies->eof) {
				nAllergyCount++;
				strAllergies += AdoFldString(rsAllergies, "Name") + "\r\n";
				rsAllergies->MoveNext();
			}
			//Trim the comma and space.
			strAllergies = strAllergies.Left(strAllergies.GetLength()-2);
			if(pstrAllergies) {
				*pstrAllergies = strAllergies;
			}
			if(pnAllergyCount) {
				*pnAllergyCount = nAllergyCount;
			}
		}
		if(!bSilent) {
			CString strMessage;
			if(nAllergyCount > 1) {
				strMessage.Format("WARNING: This patient has the following allergies, which interact with this medication:\r\n%s\r\nAre you SURE you wish to continue?", strAllergies);
			}
			else {
				strMessage.Format("WARNING: This patient has an allergy (%s) which interacts with this medication!  Are you SURE you wish to continue?", strAllergies);
			}
			if(IDYES != MsgBox(MB_YESNO, "%s", strMessage)) {
				return FALSE;
			}
			else {
				return TRUE;
			}
		}
		else {
			return FALSE;
		}
	}
	else {
		return TRUE;
	}
}


// (a.walling 2007-02-08 14:40) - PLID 24674 - Check whether the patient is allergic to the given list of medication
// (Send Drug IDs, not Patient Medication IDs.) Displays a warning box and returns TRUE if they choose to continue.
BOOL CheckAllAllergies(IN long nPatientID, OPTIONAL IN CDWordArray *pdwaDrugIDs /* = NULL */)
{
	if ( (!pdwaDrugIDs) || (pdwaDrugIDs->GetSize() == 0)) {
		ASSERT(FALSE);
		return TRUE; // for some reason we were passed in a blank list of drug ids. At least return true
		// so the merge continues unabated.
	}

	CString strField = "DrugAllergyT.DrugID";

	CString strDrugIDList;
	for (int i = 0; i < pdwaDrugIDs->GetSize(); i++) {
		strDrugIDList += FormatString("%li, ", pdwaDrugIDs->GetAt(i));
	}
	strDrugIDList.TrimRight(", ");

	// (c.haag 2007-04-03 14:47) - PLID 25482 - We now get the allergy name from EmrDataT
	// since AllergyT.Name has been depreciated
	_RecordsetPtr prs = CreateRecordset("SELECT DISTINCT EDAllergy.Data AS Name, AllergyT.ID, EDDrugList.Data as DrugName, DrugID "
		"FROM PatientMedications "
		"INNER JOIN DrugAllergyT ON PatientMedications.MedicationID = DrugAllergyT.DrugID "
		"INNER JOIN PatientAllergyT ON PatientAllergyT.AllergyID = DrugAllergyT.AllergyID AND PatientAllergyT.PersonID = PatientMedications.PatientID "
		"LEFT JOIN AllergyT ON DrugAllergyT.AllergyID = AllergyT.ID "
		"LEFT JOIN EmrDataT EDAllergy ON EDAllergy.ID = AllergyT.EmrDataID "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"LEFT JOIN EMRDataT EDDrugList ON DrugList.EMRDataID = EDDrugList.ID "
		"WHERE PatientMedications.PatientID = %li "
		"AND %s IN (%s) "
		"ORDER BY EDAllergy.Data", nPatientID, strField, strDrugIDList);

	long nAllergyID = -1, nCurAllergyID = -1;
	CString strAllergy;
	CStringArray saDrugList, saWarningList;
	while (!prs->eof) {
		nAllergyID = AdoFldLong(prs, "ID");

		if (nAllergyID != nCurAllergyID) {
			if (saDrugList.GetSize() > 0) {

				CString strMessage, strDrugList;
				for (int i = 0; i < saDrugList.GetSize(); i++) {
					strDrugList += saDrugList.GetAt(i);
					strDrugList += ", ";
				}
				strDrugList.TrimRight(", ");

				strMessage.Format("The Patient's allergy (%s) conflicts with the medication%s(%s)", strAllergy, saDrugList.GetSize() > 1 ? "s " : " ", strDrugList);
				saWarningList.Add(strMessage);
			}
			saDrugList.RemoveAll();
			nCurAllergyID = nAllergyID;
			strAllergy = AdoFldString(prs, "Name");
		}
		
		saDrugList.Add(AdoFldString(prs, "DrugName"));
		prs->MoveNext();
	}
	
	// write the final message
	if (saDrugList.GetSize() > 0) {
		CString strMessage, strDrugList;
		for (int i = 0; i < saDrugList.GetSize(); i++) {
			strDrugList += saDrugList.GetAt(i);
			strDrugList += ", ";
		}
		strDrugList.TrimRight(", ");

		strMessage.Format("The Patient's allergy (%s) conflicts with the medication%s(%s)", strAllergy, saDrugList.GetSize() > 1 ? "s " : " ", strDrugList);
		saWarningList.Add(strMessage);

		CString strFinalMessage = "WARNING!\r\n\r\n";
		for (i = 0; i < saWarningList.GetSize(); i++) {
			strFinalMessage += saWarningList.GetAt(i) + "\r\n";
		}

		strFinalMessage += "\r\nAllergic reactions to medication can be extremely dangerous. Please ensure that the patient's medications and allergies"
			" have been reviewed before continuing. Would you like to continue printing these prescriptions?";

		
		return IDYES == AfxGetMainWnd()->MessageBox(strFinalMessage, "Practice", MB_YESNO | MB_ICONWARNING);
	}

	return TRUE; // this means they must have had no interactions
}


int ShowPreferencesDlg(PreferenceItem piStartTab /*= piPatientsModule*/)
{
	CNxSuppressBkpProgress sbp; // (c.haag 2006-07-24 16:24) - PLID 21588 - Make sure the backup progress window is not visible
	//TES 6/23/2009 - PLID 34155 - Pass in whether this user is allowed to edit Global preferences
	// (j.luckoski 2012-08-15 10:21) - PLID 23345 - Send currentlocation name
	return ShowPreferencesDlg(&g_propManager, GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piStartTab, GetCurrentUserPermissions(bioPreferences)&sptDynamic0, GetCurrentLocationName());
}

int ShowPreferencesDlg(LPDISPATCH pCon, CString strUserName, CString strRegistryBase, PreferenceItem piStartTab /*= piPatientsModule*/)
{
	CNxSuppressBkpProgress sbp; // (c.haag 2006-07-24 16:24) - PLID 21588 - Make sure the backup progress window is not visible
	//TES 6/23/2009 - PLID 34155 - Pass in whether this user is allowed to edit Global preferences
	// (j.luckoski 2012-08-15 10:21) - PLID 23345 - Send current location name
	return ShowPreferencesDlg(&g_propManager, pCon, strUserName, strRegistryBase, piStartTab, GetCurrentUserPermissions(bioPreferences)&sptDynamic0, GetCurrentLocationName());
}

// (a.walling 2010-04-30 17:13) - PLID 38553 - Part of GlobalDataUtils

//// (a.walling 2007-03-27 12:42) - PLID 25367 - Create a delimited string out of a variant array
//// must be variants of an integer type!! Otherwise will return ""/-1 after catching error.
//CString ArrayAsString(IN const CVariantArray &arVariants, bool bCanReturnEmpty /* = false*/)
//{
//	try {
//		if(!arVariants.GetSize()) return bCanReturnEmpty?"":"-1";
//
//		CString strIDs;
//		for(int i = 0; i < arVariants.GetSize(); i++) {
//			strIDs += FormatString("%li,", AsLong(arVariants[i])); // this will issue a com error
//		}
//		strIDs.TrimRight(",");
//		return strIDs;
//	} NxCatchAll("Bad variable passed to ArrayAsString(CVariantArray) -- must be castable to long!");
//
//	return bCanReturnEmpty?"":"-1";
//}
//
//CString ArrayAsString(IN const CArray<long,long> &arLongs, bool bCanReturnEmpty /*= false*/)
//{
//	if(!arLongs.GetSize()) return bCanReturnEmpty?"":"-1";
//
//	CString strLongs;
//	for(int i = 0; i < arLongs.GetSize(); i++) {
//		strLongs += FormatString("%li,", arLongs[i]);
//	}
//	strLongs.TrimRight(",");
//	return strLongs;
//}
//
//CString ArrayAsString(IN const CArray<int,int> &arInts, bool bCanReturnEmpty /*= false*/)
//{
//	// (c.haag 2007-03-07 15:17) - PLID 21207 - Added overload for ints
//	if(!arInts.GetSize()) return bCanReturnEmpty?"":"-1";
//
//	CString strInts;
//	for(int i = 0; i < arInts.GetSize(); i++) {
//		strInts += FormatString("%i,", arInts[i]);
//	}
//	strInts.TrimRight(",");
//	return strInts;
//}
//
//CString ArrayAsString(IN const CDWordArray &dwa, bool bCanReturnEmpty /*= false*/)
//{
//	if(!dwa.GetSize()) return bCanReturnEmpty?"":"-1";
//
//	CString strDWords;
//	for(int i = 0; i < dwa.GetSize(); i++) {
//		strDWords += FormatString("%i,", dwa[i]);
//	}
//	strDWords.TrimRight(",");
//	return strDWords;
//}

//DRT 12/15/2005 - Copied from old patientemrdlg.cpp.  Converts a string to be ready
//	for being put in as a menu item.
CString AsMenuItem(const CString &str)
{
	CString strNew = str;
	strNew.Replace("&","&&");
	return strNew;
}

//This takes the given string, and returns a string that escapes the delimiting character with a character of your choice..
//So for example, if the delimiting character was a ;, and the escape character was \, this function would replace 
//";" with "\;", and "\" with "\\"
CString FormatForDelimitedField(const CString &strIn, char chDelimiter, char chEscape)
{
	CString strOut = strIn;
	CString strEscape(chEscape);
	CString strDelimiter(chDelimiter);
	strOut.Replace(strEscape, strEscape+strEscape);
	strOut.Replace(strDelimiter, strEscape+strDelimiter);
	return strOut;
}

//This takes a string that was formatted with FormatForDelimitedField, and returns the original string.
CString ReadFromDelimitedField(const CString &strIn, char chDelimiter, char chEscape)
{
	CString strOut = strIn;
	CString strEscape(chEscape);
	CString strDelimiter(chDelimiter);
	strOut.Replace(strEscape+strDelimiter, strDelimiter);
	strOut.Replace(strEscape+strEscape, strEscape);
	return strOut;
}

//Returns the position, in the given string, of the first non-escaped delimiter.
// (c.haag 2007-01-30 13:03) - PLID 24485 - Added an optional string start parameter
int FindDelimiter(const CString &strIn, char chDelimiter, char chEscape, int nStart /*= 0 */)
{	
	bool bEscaped = false;
	const int nLen = strIn.GetLength();
	for(int i = nStart; i < nLen; i++) {
		//TES 2/2/2007 - PLID 24537 - Fixed it to reset the bEscaped variable in the case where the character is chDelimiter
		// and bEscaped is true, otherwise the case of chEscape|chDelimiter|chDelimiter wasn't handled properly.
		if(strIn.GetAt(i) == chDelimiter) {
			if(!bEscaped) return i;
		}
		if(strIn.GetAt(i) == chEscape) {
			//If we were already escaped, we're not now.
			bEscaped = !bEscaped;
		}
		else {
			bEscaped = false;
		}
	}
	return -1;
}

// get the default template category as defined in the PIC/History advanced section
// we only work with the filename, no paths
// a.walling (4/26/06) modified to remove paths if they are passed
long GetTemplateCategory(const CString &templateName) {
	long lCatID;

	CString templateFile = GetFileName(templateName); // works with path or not
	
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 NoteCatID FROM EMRWordTemplateCategoryT WHERE TemplateName = {STRING} ORDER BY ID Desc", templateName);
	if(!rs->eof){
		lCatID = AdoFldLong(rs, "NoteCatID");
	}
	else {
		lCatID = -1;
	}
	rs->Close();
	return lCatID;
}

void ShowAndHideWindows(long nCountToShow, long nCountToHide, ...)
{
	CArray<HWND,HWND> aryhwndHide, aryhwndShow;
	va_list args;
	va_start(args, nCountToHide);
	{
		for (long i=0; i<nCountToShow; i++) {
			aryhwndShow.Add(va_arg(args, HWND));
		}
	}
	{
		for (long i=0; i<nCountToHide; i++) {
			aryhwndHide.Add(va_arg(args, HWND));
		}
	}
	va_end(args);
	
	// We have our lists, so try to do it all at once
	HDWP hdwp = BeginDeferWindowPos(aryhwndHide.GetSize() + aryhwndShow.GetSize());
	// Hide all
	{
		for (long i = 0, nCount = aryhwndHide.GetSize(); hdwp && i<nCount; i++) {
			hdwp = DeferWindowPos(hdwp, aryhwndHide.GetAt(i), NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_HIDEWINDOW);
		}
	}
	// Show all
	{
		for (long i = 0, nCount = aryhwndShow.GetSize(); hdwp && i<nCount; i++) {
			hdwp = DeferWindowPos(hdwp, aryhwndShow.GetAt(i), NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_SHOWWINDOW);
		}
	}
	
	// Go for it all out once now
	if (hdwp && EndDeferWindowPos(hdwp)) {
		// Success!
	} else {
		// The defer failed, so do it all in series like normal
		// Hide all
		{
			for (long i = 0, nCount = aryhwndHide.GetSize(); hdwp && i<nCount; i++) {
				SetWindowPos(aryhwndHide.GetAt(i), NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_HIDEWINDOW);
			}
		}
		// Show all
		{
			for (long i = 0, nCount = aryhwndShow.GetSize(); hdwp && i<nCount; i++) {
				SetWindowPos(aryhwndShow.GetAt(i), NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_SHOWWINDOW);
			}
		}
	}
}

CStringArray l_aryCustomFieldNames;
BOOL GetCustomFieldName(long nCustomFieldIndex, OUT CString &strAns)
{
	if (l_aryCustomFieldNames.GetSize() == 0) {
		// Get ALL the custom field names at one time, because the chances are high that whoever 
		// is asking for the custom field name is going to be asking for at least a few others 
		// very shortly.  By pulling them all at once we save tons of round-trip time overall.
		_RecordsetPtr prs = CreateRecordsetStd("SELECT ID, Name FROM CustomFieldsT");
		FieldPtr fldID = prs->GetFields()->GetItem("ID");
		FieldPtr fldName = prs->GetFields()->GetItem("Name");
		while (!prs->eof) {
			l_aryCustomFieldNames.SetAtGrow(AdoFldLong(fldID), AdoFldString(fldName));
			prs->MoveNext();
		}
	}
	// Now we can just look it up in the cache
	if (l_aryCustomFieldNames.GetSize() > nCustomFieldIndex) {
		strAns = l_aryCustomFieldNames.GetAt(nCustomFieldIndex);
		if (!strAns.IsEmpty()) {
			return TRUE;
		} else {
			// Hmm, for some reason our caching didn't support this custom field, we have to fail
			// (d.thompson 2009-01-26) - PLID 32861 - This assert can fire if you are missing
			//	data in CustomFieldsT.  This is a legal condition.  You can remove this comment once
			//	we fix this PLID.
			// (b.eyers 2015-10-01) - PLID 32861 - Removing the assert since nothing breaks, per j.jones
			//ASSERT(FALSE);
			return FALSE;
		}
	} else {
		// Hmm, for some reason our caching didn't support this custom field, we have to fail
		// (b.eyers 2015-10-01) - PLID 32861 - Removing the assert since nothing breaks, per j.jones
		//ASSERT(FALSE);
		return FALSE;
	}
}

// Simpler way of calling the standard implementation of GetCustomFieldName() above, except 
// instead of reporting failure when it can't get the field name, it returns "Custom Field %li"
CString GetCustomFieldName(long nCustomFieldIndex)
{
	CString strFieldName;
	if (!GetCustomFieldName(nCustomFieldIndex, strFieldName)) {
		strFieldName.Format("Custom Field %li", nCustomFieldIndex);
	}
	return strFieldName;
}

void SetCustomFieldNameCachedValue(long nCustomFieldIndex, const CString &strNewName)
{
	// Only set the array entry if the array has already been cached.  If not, then setting it 
	// would fail anyway, plus we want to load from data whenever anyone calls GetCustomFieldName.
	if (l_aryCustomFieldNames.GetSize() != 0) {
		l_aryCustomFieldNames.SetAt(nCustomFieldIndex, strNewName);
	}
}

// (j.jones 2013-03-29 11:35) - PLID 54281 - reworked this to just return the properties for product version (actual version #),
// the level (RTM, SP1, etc.), and edition (Express, Developer, etc.)
void GetSqlServerEdition(OUT CString &strVersion, OUT CString &strLevel, OUT CString &strEdition)
{
	// (j.jones 2013-03-29 11:35) - PLID 54281 - reworked this to get the product version (actual version #), the level (RTM, SP1, etc.), and edition (Express, Developer, etc.)
	_RecordsetPtr prsSqlEdition = CreateRecordset("SELECT SERVERPROPERTY('productversion') AS Version, SERVERPROPERTY('productlevel') AS Level, SERVERPROPERTY('edition') AS Edition");
	strVersion = AdoFldString(prsSqlEdition, "Version", "");
	strLevel = AdoFldString(prsSqlEdition, "Level", "");
	strEdition = AdoFldString(prsSqlEdition, "Edition", "");
}

// (j.jones 2013-03-29 11:50) - PLID 54281 - Returns the maximum size of a database, in MB, for
// the current version of SQL. For example, 4GB will return a long of 4096.
// If there is no limit, we return -1.
long GetMaximumDatabaseSize()
{
	CString strVersion, strLevel, strEdition;
	GetSqlServerEdition(strVersion, strLevel, strEdition);

	long nVersionMajor = -1, nVersionMinor = -1;
	int nDot = strVersion.Find(".");
	if(nDot != -1) {
		nVersionMajor = atoi(strVersion.Left(nDot));
		CString strTempVersion = strVersion.Right(strVersion.GetLength() - nDot - 1);
		nDot = strTempVersion.Find(".");
		if(nDot != -1) {
			nVersionMinor = atoi(strTempVersion.Left(nDot));
		}
	}

	if(strEdition.Find("Desktop Engine") != -1) {
		//the limit for all desktop engine versions (MSDE) is 2GB
		return 2048;
	}
	else if(strEdition.Find("Express Edition") != -1) {

		//2008 (10.0) and under have a limit of 4GB.
		//2008 R2 (10.50) and 2012 (11.0) have a limit of 10GB.
		//If a higher version comes out past 2012, we assume the limit is 10GB.

		if(nVersionMajor < 10
			|| (nVersionMajor == 10 && nVersionMinor < 50)) {
			//the limit for Express versions lower than 2008 is 4GB
			//the limit for 2008 Express versions earlier than R2 is 4GB
			return 4096;
		}
		else if(nVersionMajor > 10
			|| (nVersionMajor == 10 && nVersionMinor >= 50)) {
			//the limit for 2008 Express R2 is 10GB
			//the limit for anything higher than 2008 R2 is 10GB
			return 10240;
		}
	}
	else if(strEdition.Find("Standard") != -1 || strEdition.Find("Workgroup") != -1
		|| strEdition.Find("Developer") != -1 || strEdition.Find("Enterprise") != -1) {
		//All standard, workgroup, developer, and enterprise editions have no limit.
		//(Technically inaccurate, some old versions limit to 1TB, others are >500TB,
		//but for our purposes we treat these like unlimited.)
		return -1;
	}

	//If we get here, we really don't know what the maximum size is,
	//so return -1 as we cannot stop them if we don't know the limit.
	//But ideally we should have a specific case for this.
	ASSERT(FALSE);
	return -1;
}

// (b.savon 2011-10-05 15:25) - PLID 44428 - Change return type to long
long GetCurrentDatabaseSize()
{
	// (z.manning, 06/15/2006) - Although the sysfiles table does not exist as a table in 
	// SQL 2005, it does exist as a view for backwards compatibility.
	_RecordsetPtr prsSize = CreateRecordset("SELECT Size FROM sysfiles WHERE Status & 0x40 <> 0x40"); // If status has 0x40, then it's a log file.
	ASSERT(prsSize->GetRecordCount() == 1);

	// (b.savon 2011-10-05 15:25) - PLID 44428 - Change to MB
	// The size from sysfiles is in 8 KB pages, so convert it to MB.
	return ( (AdoFldLong(prsSize, "Size", 0) * 8) / 1024 );
}

BOOL IsLabAttachment(CString strMailIDs)
{
	// (m.hancock 2006-06-30 14:49) - PLID 21307 - Returns true if the passed MailID records in MailSent are 
	// associated with a lab.  The passed MailIDs should be comma separated.  This returns FALSE if they do not
	// have a license for Labs.
	// (j.gruber 2008-10-16 12:27) - PLID 31432 - added lab Results
	if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
		return(ReturnsRecords("SELECT MailID, LabStepID FROM MailSent WHERE LabStepID IS NOT NULL AND MailID IN (%s)"
			" UNION "
			" SELECT ResultID, -1 FROM LabResultsT WHERE LabResultsT.Deleted = 0 AND MailID IN (%s)"		
			, strMailIDs, strMailIDs));
	}
	else
		return FALSE;
}

//Recursive function.  Given a point in the tree, will tell you how deep the tree goes
//	below this row.  The current row does not count.  For example:
// +  Source
//    - Child 1
//    - Child 2
//    + Child 3
//      - Child A
//	    - Child B
//will return a depth of 2. 
long CountDepthBelowRow(NXDATALIST2Lib::IRowSettingsPtr pSourceRow)
{
	long nDepth = 0;
	if(pSourceRow != NULL && pSourceRow->GetFirstChildRow() != NULL) {
		//there is at least 1 row below
		nDepth++;	//increment for the 1 known

		//Now loop through all children, getting their depth, and add the highest
		long nMaxDepth = 0;
		NXDATALIST2Lib::IRowSettingsPtr pRow = pSourceRow->GetFirstChildRow();
		while(pRow != NULL) {
			long nCurDepth = CountDepthBelowRow(pRow);
			if(nCurDepth > nMaxDepth)
				nMaxDepth = nCurDepth;
			pRow = pRow->GetNextRow();
		}

		//We've finished, so add the max depth found to our current
		nDepth += nMaxDepth;
	}

	return nDepth;
}

//Returns the depth of this row from the top of the tree.  For example:
// + Topmost
//	 - Child 1
//	 - Child 2
//	   - Child A
//		 - Source
//	   - Child B
//The source row would be a depth of 3.  The topmost row would be at a depth of 1.
long CountDepthOfRowFromTop(NXDATALIST2Lib::IRowSettingsPtr pSourceRow)
{
	//Just simply traverse upwards on the parents.
	NXDATALIST2Lib::IRowSettingsPtr pRow = pSourceRow;
	long nDepth = 1;

	while(pRow != NULL && pRow->GetParentRow() != NULL) {
		nDepth++;
		pRow = pRow->GetParentRow();
	}

	return nDepth;
}

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
int InputBoxNonEmpty(CWnd* pParent, const CString &strPrompt, CString &strValue, int nMaxLength /*= -1*/)
{
	bool bContinue = true;
	UINT nResult = IDCANCEL;
	CString strOriginal = strValue, str;
	while (bContinue) {
		str = strOriginal;
		if(nMaxLength == -1) {
			nResult = InputBox(pParent, strPrompt, str, "");
		}
		else {
			nResult = InputBoxLimited(pParent, strPrompt, str, "", nMaxLength,false,false,NULL);
		}
		str.TrimLeft(); str.TrimRight();
		if (nResult == IDOK && str.IsEmpty()) {
			MsgBox("You must either cancel or enter a value.", MB_ICONINFORMATION|MB_OK);
			bContinue = true;
		} else {
			bContinue = false;
			strValue = str;
		}
	}
	return nResult;
}

// (a.walling 2006-10-05 13:05) - PLID 22869 - EncodeURL function returns a safe encoded URL
//		specifically, characters are replaced by their escaped equivalents.
CString EncodeURL(IN const CString &str)
{
	// (a.walling 2010-10-21 09:14) - PLID 41047 - Escape special characters in a URL (moved to InternetUtils)
	return InternetUtils::EncodeURL(str);
}

//if bDepthIncreased is true, you should regenerate the bitmap at the current color depth, if possible.
bool LoadVariantAsBitmap(CDC *pDC, _variant_t varBits, HBITMAP &hImage)
{
	CBorrowDeviceContext bdc(pDC);
	BYTE *arBits2 = NULL;
	COleSafeArray sa2;
	sa2.Attach(varBits);
	sa2.AccessData((VOID**)&arBits2);

	/*
	// (a.walling 2006-10-09 14:30) - PLID 22689 - Load depending on compression
	if (nThumbType == eThumbBMP)
		hImage = GetBitmapFromByteArray(bdc.m_pDC, arBits2, sa2.GetOneDimSize());
	else if (nThumbType == eThumbJPG)
		hImage = GetBitmapFromJPGByteArray(bdc.m_pDC, arBits2, sa2.GetOneDimSize());
	else if (nThumbType == eThumbPNG)
		hImage = GetBitmapFromPNGByteArray(bdc.m_pDC, arBits2, sa2.GetOneDimSize());
	else
		ASSERT(FALSE);
	*/

	// (a.walling 2006-10-20 10:21) - PLID 22991 - Fix this function in case it comes in handy in the future
	hImage = GetBitmapFromByteArray(bdc.m_pDC, arBits2, sa2.GetOneDimSize());

	sa2.UnaccessData();
	sa2.Clear();
	
	if(hImage) return true;
	else return false;
	return false;
}

// (a.walling 2006-10-09 11:51) - PLID 22689 - Load variant array of jpeg bytes into a bitmap
HBITMAP GetBitmapFromJPGByteArray(CDC *pDC, BYTE *arBytes, DWORD dwSize)
{
	CxImage xImg;

	if (xImg.Decode(arBytes, dwSize, CXIMAGE_FORMAT_JPG)) {
		// success! now make an hbitmap
		return xImg.MakeBitmap(pDC->m_hDC);
	}
	
	return NULL; // we failed!
}

// (a.walling 2006-10-09 11:51) - PLID 22689 - Load variant array of PNG bytes into a bitmap
HBITMAP GetBitmapFromPNGByteArray(CDC *pDC, BYTE *arBytes, DWORD dwSize)
{
	CxImage xImg;

	if (xImg.Decode(arBytes, dwSize, CXIMAGE_FORMAT_PNG)) {
		// success! now make an hbitmap
		return xImg.MakeBitmap(pDC->m_hDC);
	}
	
	return NULL; // we failed!
}

// (a.walling 2006-10-20 10:06) - PLID 22991 - Load any kind of image file and return an hbitmap
// (a.walling 2006-11-01 11:12) - PLID 23310 - Automatically check the modified date and reload if necessary (if loading a thumb)
// (a.walling 2006-11-21 14:48) - PLID 22922 - Reload the thumb to immediately reflect quality of encoding.
// (a.walling 2007-02-12 10:11) - PLID 22991 - Guess the type of file based on extension, or ThumbInfo, to improve performance
HBITMAP LoadBitmapFromFile(IN CString strFile)
{
	// (a.walling 2013-04-24 14:57) - PLID 56247 - A lot of duplicated code here. This is simple. If we are requesting a :NxThumb
	// then call LoadThumbFromFile. Otherwise just load via LoadCxImage.
	bool bLoadingThumb = ( strFile.Find(":NxThumb") >= 0 );
	if (bLoadingThumb) {
		CString strRootFile = strFile;
		strRootFile.TrimRight(":NxThumb");
		// (j.jones 2013-05-16 09:01) - PLID 56704 - pass strRootFile into this function
		// so it correctly creates a thumbnail on the base file name
		return LoadThumbFromFile(strRootFile);
	}

	CxImage image;
	if (!LoadCxImage(image, strFile)) {
		return NULL;
	}

	return image.MakeBitmap();
}


// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//BOOL IsNumeric(const CString &str)
//{
//	for(int i=0; i < str.GetLength(); i++) {
//		if(str.GetAt(i) < '0' || str.GetAt(i) > '9') {
//			return FALSE;
//		}
//	}
//
//	return TRUE;
//}
//
//void StripNonNumericChars(CString &str)
//{
//	for(int i = 0; i < str.GetLength(); i++) {
//		if(!(str.GetAt(i) >= '0' && str.GetAt(i) <= '9')) {
//			str.Delete(i);
//			i--;
//		}
//	}
//}
//
//void StripNonAlphanumericChars(CString &str)
//{
//	for(int i = 0; i < str.GetLength(); i++) 
//	{
//		char ch = str.GetAt(i);
//		if( !((ch >= '0' && ch <= '9')
//		  	|| (ch >= 'a' && ch <= 'z') 
//			|| (ch >= 'A' && ch <= 'Z')
//			|| ch == ' ') )
//		{
//			str.Delete(i);
//			i--;
//		}
//	}
//}

// (z.manning 2011-09-08 10:48) - PLID 45335 - Moved LooseCompareDouble to GlobalDataUtils

BOOL AreArrayContentsMatched(const CArray<long,long>& a1, const CArray<long,long>& a2)
{
	int i;

	//
	// (c.haag 2007-01-25 13:01) - PLID 24420 - Returns true if the contents of two arrays
	// match perfectly
	//
	if (a1.GetSize() != a2.GetSize()) {
		return FALSE;
	}

	//
	// Make sure everything in a1 also exists in a2
	//
	for (i=0; i < a1.GetSize(); i++) {
		if (!IsIDInArray(a1[i], a2)) {
			return FALSE;
		}
	}

	// Make sure everything in a2 also exists in a1
	for (i=0; i < a2.GetSize(); i++) {
		if (!IsIDInArray(a2[i], a1)) {
			return FALSE;
		}
	}

	// Success
	return TRUE;
}

BOOL AreArrayContentsMatched(const CDWordArray& a1, const CDWordArray& a2)
{
	int i;

	//
	// (c.haag 2007-01-25 13:01) - PLID 24420 - Returns true if the contents of two arrays
	// match perfectly
	// (c.haag 2009-01-19 11:14) - PLID 32712 - Overload for CDWordArrays
	//
	if (a1.GetSize() != a2.GetSize()) {
		return FALSE;
	}

	//
	// Make sure everything in a1 also exists in a2
	//
	for (i=0; i < a1.GetSize(); i++) {
		if (!IsIDInArray(a1[i], &a2)) {
			return FALSE;
		}
	}

	// Make sure everything in a2 also exists in a1
	for (i=0; i < a2.GetSize(); i++) {
		if (!IsIDInArray(a2[i], &a1)) {
			return FALSE;
		}
	}

	// Success
	return TRUE;
}

// (a.walling 2007-02-09 16:50) - PLID 22991 - Resample the CxImage and place on black background.
void CreateThumbnail(IN OUT CxImage &xImg, long nThumbWidth, long nThumbHeight)
{
	try {
		CBorrowDeviceContext bdc(AfxGetMainWnd());
		HBITMAP hFullImage = xImg.MakeBitmap();

		CDC dcMem;
		dcMem.CreateCompatibleDC(bdc.m_pDC);
		BITMAPINFO *pBmiTemp;
		//Allocate enough space for the biggest possible palette.
		pBmiTemp = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
		BITMAPINFOHEADER bih;
		bih.biSize          = sizeof(BITMAPINFOHEADER);
		bih.biWidth         = nThumbWidth;
		bih.biHeight        = nThumbHeight;
		bih.biPlanes        = 1;                // Must always be 1 according to docs
		bih.biBitCount      = 32;
		bih.biCompression   = BI_RGB;
		bih.biSizeImage     = nThumbWidth * nThumbHeight * 4; // 4 bytes per pixel
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed       = 0;
		bih.biClrImportant  = 0;
		pBmiTemp->bmiHeader = bih;

		LPVOID pBits;
		HBITMAP hThumb = CreateDIBSection(dcMem.GetSafeHdc(), pBmiTemp, DIB_RGB_COLORS, &pBits, NULL, 0);
		HBITMAP hOld = (HBITMAP)SelectObject(dcMem.GetSafeHdc(), hThumb);
		DrawDIBitmapInRect(&dcMem, CRect(0,0,nThumbWidth,nThumbHeight), hFullImage);
		SelectObject(dcMem.GetSafeHdc(), hOld);

		dcMem.Detach();

		dcMem.DeleteDC();
		free(pBmiTemp);
		DeleteObject(hFullImage);

		xImg.CreateFromHBITMAP(hThumb);
		DeleteObject(hThumb);
	}NxCatchAll("Error in CreateThumbnail()");
}

// (a.walling 2007-04-12 12:12) - PLID 25605 - Take our bmp and save to a temp file, returning the full path, and taking a desired filename too.
CString SaveBitmapToTempFile(HBITMAP hbmp, CString strFileName /* = ""*/)
{
	CString strFullPath;

	// (a.walling 2007-07-19 09:54) - PLID 26261 - Use the NxTemp path
	HANDLE hFile = CreateNxTempFile(strFileName, "jpg", &strFullPath, TRUE); // try to overwrite the existing file instead of creating a new numbered one.

	if (hFile == INVALID_HANDLE_VALUE) {
		ASSERT(FALSE);
		return "";
	} else {
		CloseHandle(hFile);

		// (a.walling 2013-06-18 11:16) - PLID 57198 - Use CxImage / libjpeg-turbo for saving jpeg files (eg SaveBitmapToTempFile via the preview pane / GetDataOutput / etc)
		CxImage xImg;
		if (!xImg.CreateFromHBITMAP(hbmp)) {
			ASSERT(FALSE);
			LogDetail("Failed to load image from memory");
			return "";
		}
		xImg.SetJpegQuality(80);
		if (!xImg.Save(strFullPath, CXIMAGE_FORMAT_JPG)) {
			ASSERT(FALSE);
			LogDetail("Failed to save image to %s", strFullPath);
			return "";
		}

		return strFullPath;
	}

	ASSERT(FALSE);
	return "";
}

int ComparePaths(CString strPath1, CString strPath2)
{
	// (c.haag 2007-02-22 11:48) - PLID 24701 - This function returns zero if thie
	// paths point to the same place, or non-zero if otherwise.

	// Strip the trailing backslashes
	if (strPath1.GetLength() > 0 && strPath1[ strPath1.GetLength() - 1 ] == '\\') {
		strPath1 = strPath1.Left( strPath1.GetLength() - 1 );
	}
	if (strPath2.GetLength() > 0 && strPath2[ strPath2.GetLength() - 1 ] == '\\') {
		strPath2 = strPath2.Left( strPath2.GetLength() - 1 );
	}

	// Now do the comparison
	return strPath1.CompareNoCase(strPath2);
}

// (a.walling 2007-02-27 09:26) - PLID 24451 - Simply takes a date time and sets the seconds part to zero
COleDateTime StripSeconds(IN const COleDateTime &dt)
{
	COleDateTime dtOut;

	dtOut.SetDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), dt.GetHour(), dt.GetMinute(), 0);
	dtOut.SetStatus(dt.GetStatus());

	return dtOut;
}

// (j.jones 2009-09-01 09:26) - PLID 17734 - optionally takes in an appointment ID
long CalcDefaultCaseHistoryProvider(long nPatID, long nAppointmentID /*= -1*/)
{
	//DRT 11/11/2003 - This is not a member of the CHistoryDlg, but case histories cannot be made for contacts, so we can 
	//		leave this to use the patient id.
	// (c.haag 2007-03-09 11:06) - PLID 25138 - We are now given the patient ID
	//long nPatID = GetActivePatientID();

	// (j.jones 2009-09-01 09:28) - PLID 17734 - if we are given an appointment ID, try to find the resource provider
	// (just use the first provider we find, incase there are multiple resources)
	if(nAppointmentID != -1) {
		_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ProviderID FROM ResourceProviderLinkT "
			"INNER JOIN AppointmentResourceT ON ResourceProviderLinkT.ResourceID = AppointmentResourceT.ResourceID "
			"INNER JOIN PersonT ON ResourceProviderLinkT.ProviderID = PersonT.ID "
			"WHERE ProviderID Is Not Null AND PersonT.Archived = 0 AND AppointmentResourceT.AppointmentID = {INT} "
			"AND PersonT.Archived = 0", nAppointmentID);
		if (!prs->eof) {
			return AdoFldLong(prs->GetFields(), "ProviderID");
		}
		prs->Close();
	}
	
	//if we didn't find a resource provider, now search by patient ID
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT "
		"CASE WHEN MainPhysician IS NOT NULL THEN MainPhysician ELSE (SELECT MIN(PersonID) FROM ProvidersT) END AS DefProvID "
		"FROM PatientsT WHERE PersonID = {INT}", nPatID);
	if (!prs->eof) {
		return AdoFldLong(prs->GetFields(), "DefProvID");
	} else {
		AfxThrowNxException("CalcDefaultCaseHistoryProvider: No patient records match the current patient ID %li", nPatID);
		return -1;
	}
}

//TES 3/13/2007 - PLID 24993 - Returns whether they're configured to use the UB92 or UB04.  Will always return
// one of those two values; if a bad value is found in ConfigRT this will reset it to eUB92 and return eUB92.
UBFormType GetUBFormType()
{
	int nFormType = GetRemotePropertyInt("UBFormType", eUB92, 0, "<None>", true);
	if(nFormType == eUB04 || nFormType == eUB92) {
		//It's valid
		return (UBFormType)nFormType;
	}
	else {
		//It's invalid!
		ASSERT(FALSE);
		SetRemotePropertyInt("UBFormType", eUB92, 0, "<None>");
		return eUB92;
	}
}

//DRT 4/10/2007 - PLID 25564 - This function used to exist in several different dialogs.  We now need it to be global for 
//	ftputils to work correctly, and those rest might as well use 1 central function.
BOOL PeekAndPump()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (msg.message == WM_QUIT) return FALSE;
		else AfxGetApp()->PumpMessage();
	}
	return FALSE;
}

// (j.jones 2007-05-04 08:54) - PLID 23280 - removed g_dtLastPaymentDate
// in lieu of a stack that tracks the payments we've edited, in order
// of last edited ASC

// (j.jones 2007-05-04 09:07) - PLID 23280 - used to get the last payment date
COleDateTime GetLastPaymentDate()
{
	try {

		//return the last date
		if(g_pLastPaymentInfoList.GetSize() > 0)
			return ((LastPaymentDate*)g_pLastPaymentInfoList.GetAt(g_pLastPaymentInfoList.GetSize()-1))->dtDate;

	}NxCatchAll("Error in GetLastPaymentDate");

	//return the current date if there is no last date
	return COleDateTime::GetCurrentTime();
}

// (j.jones 2007-05-04 09:07) - PLID 23280 - used to add a new payment date
void AddLastPaymentDate(long PaymentID, COleDateTime dtDate)
{
	try {

		LastPaymentDate *pNew = new LastPaymentDate;
		pNew->nID = PaymentID;
		pNew->dtDate = dtDate;
		g_pLastPaymentInfoList.Add(pNew);

	}NxCatchAll("Error in AddLastPaymentDate");
}

// (j.jones 2007-05-04 09:08) - PLID 23280 - used to remove a payment date
void DeleteLastPaymentDate(long PaymentID)
{
	try {

		for(int i = g_pLastPaymentInfoList.GetSize()-1; i>=0; i--) {
			LastPaymentDate *pLPD = ((LastPaymentDate*)g_pLastPaymentInfoList.GetAt(i));
			if(pLPD->nID == PaymentID) {
				delete pLPD;
				g_pLastPaymentInfoList.RemoveAt(i);			
			}
			//do not return, we may have this ID in here twice if we edited the payment
		}

	}NxCatchAll("Error in DeleteLastPaymentDate");
}

// (j.jones 2007-05-04 09:09) - PLID 23280 - used to clear out the array
void EmptyLastPaymentDates()
{
	try {

		for(int i = g_pLastPaymentInfoList.GetSize()-1; i>=0; i--) {
			delete ((LastPaymentDate*)g_pLastPaymentInfoList.GetAt(i));
		}

		g_pLastPaymentInfoList.RemoveAll();

	}NxCatchAll("Error in EmptyLastPaymentDates");
}

// (j.jones 2007-05-07 08:50) - PLID 25906 - removed g_dtLastBillDate and g_dtLastQuoteDate
// in lieu of stacks that track the bills and quotes we've edited, in order
// of last edited ascending

// (j.jones 2007-05-07 08:51) - PLID 25906 - used to get the last bill date
COleDateTime GetLastBillDate()
{
	try {

		//return the last date
		if(g_pLastBillInfoList.GetSize() > 0)
			return ((LastBillDate*)g_pLastBillInfoList.GetAt(g_pLastBillInfoList.GetSize()-1))->dtDate;

	}NxCatchAll("Error in GetLastBillDate");

	//return the current date if there is no last date
	return COleDateTime::GetCurrentTime();
}

// (j.jones 2007-05-07 08:51) - PLID 25906 - used to add a new bill date
void AddLastBillDate(long BillID, COleDateTime dtDate)
{
	try {

		LastBillDate *pNew = new LastBillDate;
		pNew->nID = BillID;
		pNew->dtDate = dtDate;
		g_pLastBillInfoList.Add(pNew);

	}NxCatchAll("Error in AddLastBillDate");
}

// (j.jones 2007-05-07 08:51) - PLID 25906 - used to remove a bill date
void DeleteLastBillDate(long BillID)
{
	try {

		for(int i = g_pLastBillInfoList.GetSize()-1; i>=0; i--) {
			LastBillDate *pLPD = ((LastBillDate*)g_pLastBillInfoList.GetAt(i));
			if(pLPD->nID == BillID) {
				delete pLPD;
				g_pLastBillInfoList.RemoveAt(i);			
			}
			//do not return, we may have this ID in here twice if we edited the bill
		}

	}NxCatchAll("Error in DeleteLastBillDate");
}

// (j.jones 2007-05-07 08:51) - PLID 25906 - used to clear out the array
void EmptyLastBillDates()
{
	try {

		for(int i = g_pLastBillInfoList.GetSize()-1; i>=0; i--) {
			delete ((LastBillDate*)g_pLastBillInfoList.GetAt(i));
		}

		g_pLastBillInfoList.RemoveAll();

	}NxCatchAll("Error in EmptyLastBillDates");
}

// (j.jones 2007-05-07 08:53) - PLID 25906 - used to get the last quote date
COleDateTime GetLastQuoteDate()
{
	try {

		//return the last date
		if(g_pLastQuoteInfoList.GetSize() > 0)
			return ((LastBillDate*)g_pLastQuoteInfoList.GetAt(g_pLastQuoteInfoList.GetSize()-1))->dtDate;

	}NxCatchAll("Error in GetLastQuoteDate");

	//return the current date if there is no last date
	return COleDateTime::GetCurrentTime();
}

// (j.jones 2007-05-07 08:53) - PLID 25906 - used to add a new quote date
void AddLastQuoteDate(long QuoteID, COleDateTime dtDate)
{
	try {

		LastBillDate *pNew = new LastBillDate;
		pNew->nID = QuoteID;
		pNew->dtDate = dtDate;
		g_pLastQuoteInfoList.Add(pNew);

	}NxCatchAll("Error in AddLastQuoteDate");
}

// (j.jones 2007-05-07 08:53) - PLID 25906 - used to remove a quote date
void DeleteLastQuoteDate(long QuoteID)
{
	try {

		for(int i = g_pLastQuoteInfoList.GetSize()-1; i>=0; i--) {
			LastBillDate *pLPD = ((LastBillDate*)g_pLastQuoteInfoList.GetAt(i));
			if(pLPD->nID == QuoteID) {
				delete pLPD;
				g_pLastQuoteInfoList.RemoveAt(i);			
			}
			//do not return, we may have this ID in here twice if we edited the quote
		}

	}NxCatchAll("Error in DeleteLastQuoteDate");
}

// (j.jones 2007-05-07 08:53) - PLID 25906 - used to clear out the array
void EmptyLastQuoteDates()
{
	try {

		for(int i = g_pLastQuoteInfoList.GetSize()-1; i>=0; i--) {
			delete ((LastBillDate*)g_pLastQuoteInfoList.GetAt(i));
		}

		g_pLastQuoteInfoList.RemoveAll();

	}NxCatchAll("Error in EmptyLastQuoteDates");
}

// (a.walling 2007-05-10 13:14) - PLID 25971 - Set the thread name. Use the inline
// SetThreadName function instead of calling dbg_SetThreadName directly
// inline void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
// such as:
// SetThreadName(-1, "Practice");
// SetThreadName(myWinThread.m_nThreadID, "EMRLoader");
#ifdef _DEBUG
void dbg_SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
	ASSERT(strlen(szThreadName) <= 9); // only supports 9 chars, sorry.

	THREADNAME_INFO info;
	{
		info.dwType = 0x1000;
		info.szName = szThreadName;
		info.dwThreadID = dwThreadID; // -1 is the main thread
		info.dwFlags = 0;
	}
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION) {}
}
#endif

CString GetContactImagePath()
{
	// (z.manning, 06/07/2007) - PLID 26255 - We use a new path for provider images so that users don't have
	// to look through all the EMR images to find what they're looking for.
	return GetSharedPath() ^ "Images\\Provider";
}

// (d.moore 2007-07-19) - PLID 26696 - Returns the default path where location images are stored.
CString GetLocationImagePath()
{
	return GetSharedPath() ^ "Images\\Location";
}

// (a.walling 2007-08-07 17:42) - PLID 26996 - TRACE the current number of GDI objects
#ifdef _DEBUG
void dbg_TraceGUIObjects(CString str /*=""*/)
{
	TRACE("Current GUI Resources count: %lu GDI objects\t%lu USER objects\t%s\n", GetGDICount(), GetUSERCount(), str);
}
#endif

// (a.walling 2007-09-28 09:14) - PLID 27556 - functions to return ResultCode and OpenResult constants in English.
// descriptions derived from IBM's OPOS manual at http://www-900.ibm.com/cn/support/library/pos/download/opos-apg140.pdf
CString OPOS::GetMessage(long nCode) {
	switch(nCode) {
		// ResultCode
		case OPOS_SUCCESS:
			return "Successful operation.";
			break;
		case OPOS_E_CLOSED:
			return "Attempt was made to access a closed device.";
			break;
		case OPOS_E_CLAIMED:
			return "Attempt was made to access a device that is claimed by another process. The other process must release the device before this access may be made.";
			break;
		case OPOS_E_NOTCLAIMED:
			return "Attempt was made to access an exclusive-use device that must be claimed before the method or property set action can be used.";
			break;
		case OPOS_E_NOSERVICE:
			return "The Control cannot communicate with the Service Object. Most likely, a setup or configuration error must be corrected.";
			break;
		case OPOS_E_DISABLED:
			return "Cannot perform operation while device is disabled.";
			break;
		case OPOS_E_ILLEGAL:
			return "Attempt was made to perform an illegal or unsupported operation with the device, or an invalid parameter value was used.";
			break;
		case OPOS_E_NOHARDWARE:
			return "The device is not connected to the system or is not powered on.";
			break;
		case OPOS_E_OFFLINE:
			return "The device is off-line.";
			break;
		case OPOS_E_NOEXIST:
			return "The file or device name does not exist.";
			break;
		case OPOS_E_EXISTS:
			return "The file or device name already exists.";
			break;
		case OPOS_E_FAILURE:
			return "The device cannot perform the requested procedure.";
			break;
		case OPOS_E_TIMEOUT:
			return "The Service Object timed out waiting for a response from the device, or the Control timed out waiting for a response from the Service Object.";
			break;
		case OPOS_E_BUSY:
			return "The current Service Object state is busy, and does not allow this request.";
			break;
		case OPOS_E_EXTENDED:
			return "A class-specific error condition occurred."; // more info in ResultCodeExtended
			break;
		case OPOS_E_DEPRECATED:
			// supposedly this should never happen with newer drivers.
			return "The service object has returned a deprecated response.";
			break;
		// OpenResult
		case OPOS_OR_ALREADYOPEN:
			return "Control Object already open.";
			break;
		case OPOS_OR_REGBADNAME:
			return "The registry does not contain a key for the specified device name.";
			break;
		case OPOS_OR_REGPROGID:
			return "Could not read the device name key's default value, or could not convert this Prog ID to a valid Class ID.";
			break;
		case OPOS_OR_CREATE:
			return "Could not create a service object instance, or could not get its IDispatch interface.";
			break;
		case OPOS_OR_BADIF:
			return "The service object does not support one or more of the method required by its release.";
			break;
		case OPOS_OR_FAILEDOPEN:
			return "The service object returned a failure status from its open call, but doesn't have a more specific failure code.";
			break;
		case OPOS_OR_BADVERSION:
			return "The service object major version number is not 1.";
			break;
		// The following can be returned by the service object if it
		// returns a failure status from its open call.
		case OPOS_ORS_NOPORT:
			return "Port access required at open, but configured port is invalid or inaccessible.";
			break;
		case OPOS_ORS_NOTSUPPORTED:
			return "Service Object does not support the specified device.";
			break;
		case OPOS_ORS_CONFIG:
			return "Configuration information error.";
			break;
		default:
			{
				CString str;
				if (nCode >= OPOS_ORS_SPECIFIC) { // Errors greater than this value are SO-specific.
					str.Format("A Service Object-specific code (%li) was returned", nCode);
				} else {
					ASSERT(FALSE);
					str.Format("Unknown code (%li)", nCode);
				}
				return str;
				break;
			}
	}
}

// (a.walling 2007-10-30 17:42) - PLID 27891 - Returns a string that can be put directly into SQL statement
// Note: if the length of the string is zero, then '' will be returned, or NULL if bUseNull
CString EncryptStringForSql(const CString& str, BOOL bUseNull /* = TRUE */)
{
	// (z.manning 2013-08-15 17:13) - PLID 55813 - Moved this logic to NxAES lib
	return g_NxAES.EncryptStringForSql(str, bUseNull);
}

// (j.jones 2009-04-30 16:26) - PLID 33853 - added EncryptStringToVariant, used in param recordsets
_variant_t EncryptStringToVariant(const CString& str)
{
	// (z.manning 2013-08-15 17:13) - PLID 55813 - Moved this logic to NxAES lib
	return g_NxAES.EncryptStringToVariant(str);
}

// (a.walling 2007-10-30 17:49) - PLID 27891 - Returns a decrypted string from a variant array of bytes
CString DecryptStringFromVariant(_variant_t var)
{
	// (z.manning 2013-08-15 17:13) - PLID 55813 - Moved this logic to NxAES lib
	return g_NxAES.DecryptStringFromVariant(var);
}

// (z.manning, 04/22/2008) - PLID 29745 - Deleted the obsolete TransparentButton and TransparentStatic classes
// (NxButton and CNxStatic should be used instead)

//TES 3/25/2008 - PLID 24157 - Moved out of schedulerview
BOOL IsWindowDescendent(const HWND hwndAncestor, const HWND hwndDescendent)
{
	HWND hwndParent = ::GetParent(hwndDescendent);
	while (hwndParent) {
		if (hwndAncestor == hwndParent) {
			return TRUE;
		} else {
			hwndParent = ::GetParent(hwndParent);
		}
	}
	return FALSE;
}

bool GenericTransparentCtlColor(HBRUSH& hbr, CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{	
	// (a.walling 2010-06-23 18:38) - PLID 39336 - Give the child a chance to handle the reflected message!	
	LRESULT lResult = 0;
	if (pWnd->SendChildNotifyLastMsg(&lResult)) {
		hbr = (HBRUSH)lResult;     // eat it
		return true;
	}

	switch (nCtlColor)
	{	case CTLCOLOR_STATIC:
		case CTLCOLOR_BTN:
		//make anything (STATIC text) with WS_EX_TRANSPARENT
		//appear transparent without subclassing it
		if (pWnd->GetExStyle() & WS_EX_TRANSPARENT)
		{	pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
			return true;
		}
		break;
	}

	return false;
}

// (a.walling 2008-04-21 15:29) - PLID 29711 - Retrieve the text color of the tabs
// (j.jones 2016-04-18 17:02) - NX-100214 - this is now GetDefaultNxTabTextColor in NxUILib
//COLORREF GetTabViewTextColor()

// (j.jones 2008-05-16 09:14) - PLID 29732 - printing prescriptions is now a global function
// (c.haag 2009-08-18 13:11) - PLID 15479 - Added bMergeToPrinter
// (r.farnworth 2013-01-21) PLID 54667 - Added bMergetoFax
// (j.fouts 2013-03-05 08:36) - PLID 55427 - Removed PatientID paramater
BOOL MergePrescriptionsToWord(CArray<long, long> &aryPrescriptionIDs, CString strTemplateName, CString strFullTemplatePath, CString &strPath,
							  BOOL bMergeToPrinter /* = FALSE */, BOOL bMergetoFax /* = FALSE */)
{
	try {

		//Note: the EMR prescription merge does not use this function at this time

		CWaitCursor pWait;

		if(aryPrescriptionIDs.GetSize() == 0) {
			AfxMessageBox("There are no prescriptions selected. No prescription will be merged.");
			return FALSE;
		}

		if(strTemplateName.IsEmpty() || strFullTemplatePath.IsEmpty()) {
			AfxMessageBox("There is no template selected. The prescription cannot be merged.");
			return FALSE;
		}

		//make sure word exists
		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return FALSE;
		}

		// (a.walling 4/24/06 PL 18343) Get category for the template
		_variant_t varCategoryID = g_cvarNull;
		long nCatID = GetTemplateCategory(GetFileName(strTemplateName));
		if (nCatID != -1) {
			//TES 8/2/2011 - PLID 44814 - Make sure the user has permission for this category
			if(CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, nCatID, TRUE)) {
				varCategoryID = (long)nCatID;
			}
		}
		
		CString strPrescriptions;
		foreach(long nPrescriptionIDs, aryPrescriptionIDs)
		{
			strPrescriptions += FormatString("%li, ", nPrescriptionIDs);
		}
		strPrescriptions.Trim(", ");

		/// Generate the temp table
		// (j.fouts 2013-03-05 08:36) - PLID 55427 - Generate the temp table from the prescriptions rather than the patientIDs
		CString strSql;
		strSql.Format("SELECT DISTINCT PatientID AS ID FROM PatientMedications WHERE ID IN (%s)", strPrescriptions);
		CString strMergeT = CreateTempIDTable(strSql, "ID");
		
		// Merge
		CMergeEngine mi;

		// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
		if(!mi.LoadSenderInfo(TRUE)) {
			return FALSE;
		}
		
		//add all prescriptions to the merge
		int i = 0;
		for(i=0;i<aryPrescriptionIDs.GetSize();i++) {
			mi.m_arydwPrescriptionIDs.Add((DWORD)(aryPrescriptionIDs.GetAt(i)));
		}	

		if (g_bMergeAllFields) {
			mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;
		}

		//(r.farnworth 2013-1-18) PLID 54667 - Added bMergetoFax
		//(r.farnworth 2013-04-02) PLID 56028 - Added BMS_HIDE_WORDUI
		if(bMergetoFax){
			mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
			mi.m_nFlags |= BMS_HIDE_WORDUI;
		} else {
			mi.m_nFlags |= BMS_SAVE_FILE_NO_HISTORY; //save the file, do not save in history
		}
		

		// (c.haag 2009-08-18 13:11) - PLID 15479 - Added the option to merge directly to the printer
		if (bMergeToPrinter) {
			mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
		}

		// Do the merge
		// (a.wilson 2013-04-18 14:11) - PLID 56142 - check if the merge failed (missing merge field).
		if (!mi.MergeToWord(strFullTemplatePath, std::vector<CString>(), strMergeT)) {
			return FALSE;
		}
		//(r.farnworth 2013-1-18) PLID 54667 - We need to return the file path for faxing prescriptions
		strPath = mi.m_strSavedAs;


		//Update patient history here, because multiple merges per patient ID
		//will screw up the merge engine's method of doing it. But hey,
		//we get to make the description a lot better as a result!

		// (c.haag 2007-02-02 17:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		// (j.fouts 2013-03-05 08:36) - PLID 55427 - Reworked this to run just one query and generate the description for multiple patients
		_RecordsetPtr prs = CreateParamRecordset(
		"SELECT OuterQ.PersonID, LEFT('Prescription printed for ' + (SELECT STUFF(( "
		"SELECT '  /  ' + EMRDataT.Data + ', Quantity: ' + Quantity + ' ' + Unit + ' Refills: ' + RefillsAllowed "
		"FROM PatientMedications "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
		"WHERE PatientMedications.ID IN ({INTARRAY}) AND PatientID = OuterQ.PersonID "
		"FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1, 5, '')), 4000) AS Description "
		"FROM PatientsT OuterQ "
		"WHERE PersonID IN (SELECT PatientID FROM PatientMedications WHERE ID IN ({INTARRAY}))",aryPrescriptionIDs,aryPrescriptionIDs);

		COleDateTime dtNull;
		dtNull.SetStatus(COleDateTime::null);
		while(!prs->eof)
		{
			//Each record contains the description and PersonID for the new mail entry
			CString strDescription = AdoFldString(prs, "Description", "");
			long nPatientID = AdoFldLong(prs, "PersonID", -1);
			
			// (j.jones 2008-09-04 15:46) - PLID 30288 - converted to use CreateNewMailSentEntry,
			// which creates the data in one batch and sends a tablechecker
			// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
			// (d.singleton 2013-11-15 11:18) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			CreateNewMailSentEntry(nPatientID, strDescription, SELECTION_WORDDOC, mi.m_strSavedAs, GetCurrentUserName(), "", GetCurrentLocationID(), dtNull, -1, VarLong(varCategoryID, -1), -1, -1, FALSE, -1, "", ctNone);

			prs->MoveNext();
		}
		prs->Close();		

		return TRUE;

	}NxCatchAll("Error merging prescriptions to Word.");

	return FALSE;
}

//(r.farnworth 2013-1-18) PLID 54667 Overloaded to return value of the new file as StrPath
// (j.fouts 2013-03-05 08:36) - PLID 55427 - Removed PatientID paramater
BOOL MergePrescriptionsToWord(CArray<long, long> &aryPrescriptionIDs, CString strTemplateName, CString strFullTemplatePath, 
						      BOOL bMergeToPrinter /* = FALSE */, BOOL bMergetoFax /* = FALSE */)
{
	CString blankTMP = "   ";
	// (j.fouts 2013-03-05 08:36) - PLID 55427 - Removed PatientID paramater
	return MergePrescriptionsToWord(aryPrescriptionIDs, strTemplateName, strFullTemplatePath, blankTMP, bMergeToPrinter, bMergetoFax);
}

// (a.walling 2008-05-19 15:22) - PLID 27810 - This function will display a messagebox if the NPI is invalid
BOOL CheckNPI(const CString& strNum, CWnd* pParent, BOOL bAllowCancel, CString strExtra)
{
	// this preference is not bulk cached anywhere
	// (j.jones 2011-04-07 11:33) - PLID 37253 - ensured we ignored this for OHIP and Alberta as well
	if(GetRemotePropertyInt("AutoValidateNPI", TRUE, 0, "<None>", true) == FALSE
		|| UseOHIP() || UseAlbertaHLINK())
		return TRUE;

	BOOL bIsValid = IsValidNPI(strNum);

	if (!bIsValid) {
		if (pParent == NULL) {
			ASSERT(FALSE); // should always pass a parent!
			pParent = GetMainFrame();
		}

		CString strMessage;
		strMessage.Format("The NPI '%s' %sis invalid!\r\n\r\n", strNum, strExtra);
		if (strNum.GetLength() != 10) {
			strMessage += "National Provider Identifiers must be ten digits long.";
		} else {
			long nBad = 0;

			for (int i = 0; i < strNum.GetLength(); i++) {
				// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
				if (!isdigit(unsigned char(strNum[i]))) {
					nBad++;
				}
			}

			if (nBad > 0) {
				CString str;
				str.Format("There are %li invalid characters. Identifiers must contain only numeric digits and no spaces.", nBad);
				strMessage += str;
			} else {
				strMessage += "Please check your records to ensure the correct National Provider Identifier was entered. The CMS website at https://nppes.cms.hhs.gov/ also provides an NPI Registry that may be used to verify your information.";
			}
		}

		if (!bAllowCancel) {
			strMessage += "\r\n\r\nDespite failing validation, the number will still be saved.";
		} else {
			strMessage += "\r\n\r\nPlease cancel and correct the number, or choose OK to continue.";
		}

		int nResult = pParent->MessageBox(strMessage, "Practice - NPI Validation", (bAllowCancel ? MB_OKCANCEL : MB_OK) | MB_ICONEXCLAMATION);

		if (bAllowCancel) {
			return nResult == IDOK;
		}
	}

	return bIsValid;
}

// (a.walling 2008-05-19 15:22) - PLID 27810 - Returns whether the NPI is valid or not
BOOL IsValidNPI(const CString& strNum)
{
	if (strNum.GetLength() == 0)
		return TRUE;
	else if (strNum.GetLength() != 10)
		return FALSE;

	CString strNPI = CString("80840") + strNum;

	return CheckLuhn(strNPI);
}

// (a.walling 2008-05-19 15:17) - PLID 27810 - Returns whether the string has a valid Luhn checksum
BOOL CheckLuhn(const CString& strNum)
{
	// a blank string is technically valid
	long nSum = 0;
	long nNum = 0;
	BOOL bAlt = FALSE;

	for (int i = strNum.GetLength() - 1; i >= 0; i--) {
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (!isdigit(unsigned char(strNum[i]))) // a non-numeric character is an immediate failure
			return FALSE;

		nNum = strNum[i] - 48; // get the actual numeric value of the ASCII character

		if(bAlt) { // double and cast out nines every other digit
			nNum *= 2;

			// add the values of the digits if > 9 (cast out nines)
			if(nNum > 9)
				nNum -= 9;
		}

		nSum += nNum;
		bAlt = !bAlt;
	}

	return nSum % 10 == 0;
}

// (j.jones 2013-04-10 15:41) - PLID 56191 - Takes a number that needs a Luhn checksum added, and returns the check digit.
long CalculateLuhn(const CString& strNumToCalculate)
{
	long nSum = 0;
	long nNum = 0;
	BOOL bAlt = FALSE;

	//begin by adding a check digit of zero to the end
	CString strNum = strNumToCalculate + "0";

	for (int i = strNum.GetLength() - 1; i >= 0; i--) {
		// (a.walling 2008-10-03 17:29) - PLID 31589 - ASSERTion if you try to pass a signed char to any ::is* functions.
		if (!isdigit(unsigned char(strNum[i]))) // a non-numeric character is an immediate failure
			return FALSE;

		nNum = strNum[i] - 48; // get the actual numeric value of the ASCII character

		if(bAlt) { // double and cast out nines every other digit
			nNum *= 2;

			// add the values of the digits if > 9 (cast out nines)
			if(nNum > 9)
				nNum -= 9;
		}

		nSum += nNum;
		bAlt = !bAlt;
	}

	if(nSum % 10 == 0) {
		//the check digit really is zero!
		return 0;
	}
	else {
		//the check digit is 10 minus our sum % 10
		return 10 - (nSum % 10);
	}
}

// (j.jones 2008-06-05 11:25) - PLID 29154 - this function will take in a count of prescriptions, and
// returns the template to be used, also will fill in booleans to let the caller know whether
// a template was found for the exact count (if false, means we sent back the generic default),
// and whether templates for other counts were found (not filled if bExactCountFound = TRUE)
CString GetDefaultPrescriptionTemplateByCount(long nCountPrescriptions, BOOL &bExactCountFound, BOOL &bOtherCountFound)
{
	try {

		//see if they have a default for the given count
		CString strDefaultTemplateForCount = GetRemotePropertyText("DefaultPrescriptionFilenameByCount", "", (int)nCountPrescriptions, "<None>", false);
		if(strDefaultTemplateForCount.IsEmpty()) {

			bExactCountFound = FALSE;

			//grab the global default template, which is per workstation
			CString strDefaultTemplate = GetPropertyText("DefaultPrescriptionFilename", "", 0, false);
			
			//find out if any other DefaultPrescriptionFilenameByCount records exist
			// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam (although not really necessary, this could take advantage of cached queries when that is enabled)
			if(ReturnsRecordsParam("SELECT Name FROM ConfigRT WHERE Name = 'DefaultPrescriptionFilenameByCount' AND TextParam <> ''")) {
				//they have at least one default-by-count template stored, but not one for this count
				bOtherCountFound = TRUE;
			}
			
			//return strDefaultTemplate, even if it is empty
			return strDefaultTemplate;
		}
		else {

			bExactCountFound = TRUE;
			bOtherCountFound = FALSE; //meaningless now, but fill it anyways

			return strDefaultTemplateForCount;
		}

	}NxCatchAll("Error in GetDefaultPrescriptionTemplateByCount");

	return "";
}

// (a.walling 2008-06-09 16:58) - PLID 22049 - Helper functions for GetMessagePos.. MFC-Style!
void GetMessagePos(CPoint& pt)
{
	DWORD n = ::GetMessagePos();
	// (a.walling 2009-04-01 14:10) - PLID 33796 - using LOWORD/HIWORD will mess up the sign for multiple monitors
	pt.x = GET_X_LPARAM(n);
	pt.y = GET_Y_LPARAM(n);
}

void GetMessagePos(CPoint* ppt)
{
	DWORD n = ::GetMessagePos();
	// (a.walling 2009-04-01 14:10) - PLID 33796 - using LOWORD/HIWORD will mess up the sign for multiple monitors
	ppt->x = GET_X_LPARAM(n);
	ppt->y = GET_Y_LPARAM(n);
}

// (a.walling 2008-07-25 10:26) - PLID 30836 - Are we Vista+?
static DWORD g_nWinVerMajor = -1;
BOOL IsVistaOrGreater()
{
	if (g_nWinVerMajor == -1) {
		OSVERSIONINFO osver;
		
		ZeroMemory(&osver, sizeof(OSVERSIONINFO));
		osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		//d.thompson 10/14/2015 - PLID 67348 - GetVersionEx is now deprecated.  
		//	Future PLID 67347
#pragma warning (push)
#pragma warning(disable: 4996)
		::GetVersionEx(&osver);
#pragma warning(pop)
		g_nWinVerMajor = osver.dwMajorVersion;
	}

	return g_nWinVerMajor >= 6;
}

// (z.manning 2008-07-23 17:12) - PLID 30804 - Moved this here from CNxSchedulerDlg
void GetIDsFromCommaDelimitedString(CDWordArray* dwaryIDs, CString strIDList)
{
	dwaryIDs->RemoveAll();
	CString strTempIDList = strIDList;
	strTempIDList.TrimRight();

	if(strTempIDList.IsEmpty()) {
		return;
	}

	// (z.manning 2008-07-23 17:38) - PLID 30804 - This function expects a comma at the end, so
	// ensure that is the case.
	if(strTempIDList.Right(1) != ',') {
		strTempIDList += ',';
	}

	while(strTempIDList.GetLength() > 0)
	{
		// Get the next ID in the list, then delete it and the comma that follows from the temp string
		long nComma = strTempIDList.Find(",");
		dwaryIDs->Add(atoi(strTempIDList.Left(nComma)));
		if(strTempIDList.GetLength() >= nComma + 1) {
			strTempIDList.Delete(0, nComma + 1);
		}
		else {
			// This shouldn't ever happen, but just in case.
			strTempIDList.Delete(0, strTempIDList.GetLength());
		}
	}
}

// (j.jones 2013-10-11 14:53) - PLID 58965 - Same concept as GetIDsFromCommaDelimitedString,
// but the resulting value is a CStringArray. Naturally this can only be used on strings
// that are absolutely certain to not have commas in them.
void GetStringsFromCommaDelimitedString(CStringArray* aryStrings, CString strStringList)
{
	aryStrings->RemoveAll();
	CString strTempStringList = strStringList;
	strTempStringList.TrimRight();

	if(strTempStringList.IsEmpty()) {
		return;
	}

	//this function expects a comma at the end, so ensure that is the case.
	if(strTempStringList.Right(1) != ',') {
		strTempStringList += ',';
	}

	while(strTempStringList.GetLength() > 0)
	{
		// Get the next ID in the list, then delete it and the comma that follows from the temp string
		long nComma = strTempStringList.Find(",");
		aryStrings->Add(strTempStringList.Left(nComma));
		if(strTempStringList.GetLength() >= nComma + 1) {
			strTempStringList.Delete(0, nComma + 1);
		}
		else {
			// This shouldn't ever happen, but just in case.
			strTempStringList.Delete(0, strTempStringList.GetLength());
		}
	}
}

// (a.walling 2008-08-27 12:52) - PLID 30855 - returns TRUE if the _com_error is a SQL server error with extended info,
// and fills the CSQLErrorInfo structure with useful info.
BOOL GetSQLErrorInfo(_com_error& e, CSQLErrorInfo& eSqlError)
{
	if (e.Error() != DB_E_ERRORSINCOMMAND) {
		return FALSE;	
	}

	// (a.walling 2011-05-13 11:20) - PLID 43693 - Reference counting leak
	IErrorInfoPtr pInfo(e.ErrorInfo(), false);

	if (!pInfo) {
		return FALSE;
	}

	IErrorRecordsPtr pRecords;
	HRESULT hr = pInfo->QueryInterface(IID_IErrorRecords, (void**)&pRecords);
	if (SUCCEEDED(hr) && pRecords != NULL) {
		unsigned long nRecords = 0;
		pRecords->GetRecordCount(&nRecords);

		if (nRecords > 0) {
			ISQLServerErrorInfoPtr pErr;
			hr = pRecords->GetCustomErrorObject(0, IID_ISQLServerErrorInfo, (IUnknown**)&pErr);

			if (SUCCEEDED(hr) && pErr != NULL) {
				SSERRORINFO* pSse = NULL;
				OLECHAR* pStrings;
				// (a.walling 2008-07-17 14:01) - PLID 22049 - Make sure this succeeds
				hr = pErr->GetErrorInfo(&pSse, &pStrings);
				if (SUCCEEDED(hr) && pSse != NULL) {
					//CString strMessage = (LPCTSTR)_bstr_t(pSse->pwszMessage);
					//CString strServer = (LPCTSTR)_bstr_t(pSse->pwszServer);
					//CString strProcedure = (LPCTSTR)_bstr_t(pSse->pwszProcedure);

					// this is the msg_id. Since we use adhoc messages, and not defined ones,
					// this will always be 50000.
					eSqlError.nNative = pSse->lNative;

					// the 'class' is what we would call the severity in RAISERROR, and is 16 for all of our calls.
					eSqlError.nClass = pSse->bClass;

					// the state helps us differentiate between several different errors that we may raise.
					eSqlError.nState = pSse->bState;

					// WORD wLineNumber = pSse->wLineNumber;

					// (a.walling 2008-05-30 12:41) - PLID 22049
					// see Raymond's blog entry here http://blogs.msdn.com/oldnewthing/archive/2004/07/05/173226.aspx
					// where he points out that CoTaskMemFree is exactly the same as CoGetMalloc(MEMCTX_TASK) + IMalloc::Free.
					// Larry Osterman also has something to say on this topic:
					// http://blogs.msdn.com/larryosterman/archive/2007/02/07/the-sad-story-of-cogetmalloc.aspx

					// first free the error info
					if (pSse)
						::CoTaskMemFree(pSse);
					// then free the SSERRORINFO strings (all 3 strings are allocated as a single block)
					if (pStrings)
						::CoTaskMemFree(pStrings);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

// (c.haag 2010-01-28 10:18) - PLID 37086 - Created version of CreateNewMailSentEntry that does not take in a service date
long CreateNewMailSentEntry(long nPersonID, CString strNote, CString strSelection, CString strPathName, CString strSender, CString strSubject, long nLocationID)
{
	// This is identical to the CreateNewMailSentEntry that Josh wrote, except it doesn't take in as many parameters
	try {
		long nID = CreateNewMailSentEntry(GetRemoteData(), nPersonID, strNote, strSelection, strPathName, strSender, strSubject, nLocationID);
		if(nID != -1) {
			// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker, we do not know the photo status
			CClient::RefreshMailSentTable(nPersonID, nID, IsImageFile(strPathName) ? TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto : TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);
		}
		return nID;
	}NxCatchAll("Error in CreateNewMailSentEntry");

	return -1;
}

// (c.haag 2010-01-28 09:07) - PLID 37086 - Refactored CreateNewMailSentEntry to no longer support optional parameters
// (d.singleton 2013-11-15 10:00) - PLID 59513 - need to insert the CCDAType in mailsent when generating a CCDA
long CreateNewMailSentEntry(long nPersonID, CString strNote, CString strSelection, CString strPathName, CString strSender, CString strSubject, long nLocationID, COleDateTime dtServiceDate, CCDAType ctType)
{
	return CreateNewMailSentEntry(nPersonID, strNote, strSelection, strPathName, strSender, strSubject, nLocationID, dtServiceDate,
		-1, -1, -1, -1, FALSE, -1, "", ctType);
}

// (j.jones 2008-09-04 14:07) - PLID 30288 - CreateNewMailSentEntry will create a new entry
// in MailSent and MailSentNotesT in one batch, and send a tablechecker with the new ID, as
// well as return that new ID
// (r.farnworth 2016-03-10 11:36) - PLID 68401 - Added nOnlineVisitID
long CreateNewMailSentEntry(long nPersonID, CString strNote, CString strSelection, CString strPathName, CString strSender, CString strSubject, long nLocationID, COleDateTime dtServiceDate,
							long nChecksum, long nCategoryID, long nPicID, long nLabStepID, BOOL bIsPhoto, long nInternalRefID, CString strInternalTblName, CCDAType ctType, int nOnlineVisitID /*= -1*/)
{
	try {

		//TES 9/18/2008 - PLID 31413 - Call the HistoryUtils function that is shared with NxServer.
		long nID = CreateNewMailSentEntry(GetRemoteData(), nPersonID, strNote, strSelection, strPathName, strSender, strSubject, nLocationID, dtServiceDate,
			nChecksum, nCategoryID, nPicID, nLabStepID, bIsPhoto, nInternalRefID, strInternalTblName, (long)ctType, nOnlineVisitID);

		if(nID != -1) {
			//TES 9/18/2008 - PLID 31413 - Send a tablechedker
			// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker
			CClient::RefreshMailSentTable(nPersonID, nID, bIsPhoto ? TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto : TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);
		}

		return nID;

	}NxCatchAll("Error in CreateNewMailSentEntry");

	return -1;
}

// (j.jones 2008-09-11 12:18) - PLID 31335 - finds all active insured parties with
// an inactive date that is today's date or earlier, and inactivates them
void UpdateInactiveInsuredParties()
{
	long nAuditTransactionID = -1;

	try {
		
		//This function is called upon login, provided that the LastInactiveInsuredPartyProcess
		//setting does not return today's date.

		//Find all insured parties that are not inactive, but have inactive dates prior to or on today's date.

		_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID AS InsuredPartyID, "
			"InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespTypeName, PersonT.ID AS PatientID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName "
			"FROM InsuredPartyT "
			"LEFT JOIN PersonT ON InsuredPartyT.PatientID = PersonT.ID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE RespTypeID <> -1 AND ExpireDate Is Not Null "
			"AND dbo.AsDateNoTime(ExpireDate) <= dbo.AsDateNoTime(GetDate())");

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		CParamSqlBatch batch;

		CArray<long,long> arynPatientIDs;
		while(!rs->eof) {

			long nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID");
			CString strInsCoName = AdoFldString(rs, "InsCoName", "");
			CString strRespTypeName = AdoFldString(rs, "RespTypeName", "");
			CString strPatientName = AdoFldString(rs, "PatientName", "");

			//Update these insured parties to be inactive, keeping the balances in inactive responsibility.
			//(Update the status only, as we already have a date.)
			batch.Add("UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PersonID = {INT}", nInsuredPartyID);

			// (z.manning 2009-01-09 09:53) - PLID 32663 - Keep track of the patient IDs.
			long nPatientID = AdoFldLong(rs->GetFields(), "PatientID");
			if(!IsIDInArray(nPatientID, arynPatientIDs)) {
				arynPatientIDs.Add(nPatientID);
			}

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			//Audit the inactivation in a manner different from the regular inactivation,
			//so we can tell later when the system did it, versus when a human did it.
			CString strOld;
			strOld.Format("%s (%s)", strRespTypeName, strInsCoName);
			AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInsPartyInactivation_ByPractice, nInsuredPartyID, strOld, "Inactivated (Expired)", aepMedium, aetChanged);

			rs->MoveNext();
		}
		rs->Close();

		if(!batch.IsEmpty()) {
			batch.Execute(GetRemoteData());
		}

		// (z.manning 2009-01-09 09:54) - PLID 32663 - Update any changed patients in HL7
		try {
			//TES 11/12/2015 - PLID 67500 - Use the function to updated them all at once
			UpdateMultipleExistingPatientsInHL7(arynPatientIDs);
		}NxCatchAll("::UpdateInactiveInsuredParties - HL7");

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}
		
		//store the current date/time as the last time we ran this process
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		SetRemotePropertyDateTime("LastInactiveInsuredPartyProcess", dtNow, 0, "<None>");

		//if anything changed, update the view
		if(!batch.IsEmpty()) {
			if(GetMainFrame()) {
				GetMainFrame()->UpdateAllViews();
			}
		}

	}NxCatchAllCall("Error in UpdateInactiveInsuredParties",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

// (a.walling 2008-09-15 10:54) - PLID Windycane Ike!
DWORD GetGDICount()
{
	return GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
}

// (a.walling 2008-09-15 10:54) - PLID Windycane Ike!
DWORD GetUSERCount()
{
	return GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);
}

// (j.gruber 2008-10-27 10:33) - PLID 31432 - this function is used when detaching a history document that is attached to a lab in order 
//to audit the value fields since that changes when the document is no longer attached to a lab
// (a.walling 2010-01-21 17:00) - PLID 37023 - send in a patient ID
long HandleLabResultAudit(CString strWhere, CString strPersonName, long nPatientID) {

	long nAuditTransactionID = -1;

	_RecordsetPtr rs = CreateRecordset("SELECT ResultID, Value, Units FROM LabResultsT WHERE DELETED = 0 AND %s", strWhere);
	while (!rs->eof) {

		long nResultID = AdoFldLong(rs, "ResultID");
		CString strOldValue = AdoFldString(rs, "Value", "");

		if (!strOldValue.IsEmpty()) {

			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
	
				AuditEvent(nPatientID, strPersonName, nAuditTransactionID, aeiLabResultValue, nResultID, strOldValue, "", 2, aetChanged);
			}
		}

		// (c.haag 2009-05-06 13:18) - PLID 33789 - Repeat this logic for untis
		strOldValue = AdoFldString(rs, "Units"); // Non-nullable
		if (!strOldValue.IsEmpty()) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
	
				AuditEvent(nPatientID, strPersonName, nAuditTransactionID, aeiLabResultUnits, nResultID, strOldValue, "", 2, aetChanged);
			}
		}

		rs->MoveNext();
	}

	return nAuditTransactionID;
}

// (z.manning 2008-10-09 15:37) - PLID 31628 - Utility function to delete a lab
// (j.gruber 2008-10-10 11:12) - PLID 31432 - update the lab results also
// (Moved here from PatientLabsDlg.cpp)
// (j.jones 2009-06-04 16:36) - PLID 34487 - need to pass in lists of problem links to delete and problems to delete
void DeletePatientLab(IN const long nLabID, IN const CString strEMRProblemLinkIDsToDelete, IN const CString strEMRProblemIDsToDelete)
{
	CString strDeleteEMRProblemLinkIDs = strEMRProblemLinkIDsToDelete;
	CString strDeleteEMRProblemIDs = strEMRProblemIDsToDelete;

	BOOL bDeletedProblems = FALSE;

	if(strDeleteEMRProblemLinkIDs.IsEmpty()) {
		strDeleteEMRProblemLinkIDs = "-1";
	}
	else {
		bDeletedProblems = TRUE;
	}
	if(strDeleteEMRProblemIDs.IsEmpty()) {
		strDeleteEMRProblemIDs = "-1";
	}
	else {
		bDeletedProblems = TRUE;
	}

	//"Delete" the Lab by updated its deleted flag
	// (z.manning 2008-10-13 11:32) - PLID 31667 - Also delete any todos associated with this lab
	// (z.manning 2008-10-30 11:06) - PLID 31864 - We now pull to be ordered for non-biopsy labs for
	// the audit text.
	// (j.jones 2009-06-04 16:17) - PLID 34487 - handle deleting problems
	// (z.manning 2010-05-12 16:18) - PLID 37405 - Also handle ttLab regarding to-dos
	_RecordsetPtr prsDelete = CreateRecordset(
		"SET NOCOUNT ON \r\n"
		"UPDATE LabsT SET Deleted = 1, DeletedDate = GetDate(), DeletedBy = %li \r\n"
		"WHERE ID = %li \r\n"
		"UPDATE LabResultsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = %li \r\n"
		"WHERE LabID = %li \r\n "
		"\r\n"
		"DECLARE @DeletedTodosT TABLE (TaskID int NOT NULL PRIMARY KEY, AssignToNames nvarchar(1500), Deadline datetime, Category nvarchar(50), Notes nvarchar(2000)) \r\n"
		"INSERT INTO @DeletedTodosT (TaskID, AssignToNames, Deadline, Category, Notes) \r\n"
		"SELECT TaskID, dbo.GetTodoAssignToNamesString(TodoList.TaskID), Deadline, Description, Notes \r\n"
		"FROM ToDoList \r\n"
		"LEFT JOIN NoteCatsF ON TodoList.CategoryID = NoteCatsF.ID \r\n"
		"WHERE (RegardingType = %li AND RegardingID IN (SELECT StepID FROM LabStepsT WHERE LabID = %li)) \r\n"
		"	OR (RegardingType = %li AND RegardingID = %li) \r\n"
		"DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM @DeletedTodosT) \r\n"
		"DELETE FROM ToDoList WHERE TaskID IN (SELECT TaskID FROM @DeletedTodosT) \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT PatientID, COALESCE(FormNumberTextID, '') + '-' + COALESCE(Specimen, '') + ' - ' + CASE WHEN LabsT.Type = %li THEN COALESCE(LabAnatomyT.Description, '') ELSE ToBeOrdered END AS Description \r\n"
		"FROM LabsT \r\n"
		"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID \r\n"
		"WHERE LabsT.ID = %li \r\n"
		// (z.manning 2008-10-14 10:46) - PLID 31667 - Select the info needed for auditing deleted to-dos
		"SELECT TaskID, AssignToNames, Deadline, Category, Notes \r\n"
		"FROM @DeletedTodosT \r\n"
		// (j.jones 2009-06-04 16:18) - PLID 34487 - handle problems, which we do not audit if we're deleting their linked object
		"DELETE FROM EmrProblemLinkT WHERE ID IN (%s)\r\n"
		"UPDATE EmrProblemsT SET Deleted = 1, DeletedDate = GetDate(), DeletedBy = '%s' WHERE ID IN (%s)\r\n"
		, GetCurrentUserID(), nLabID, GetCurrentUserID(), nLabID, ttLabStep, nLabID, ttLab, nLabID, ltBiopsy, nLabID,
		strDeleteEMRProblemLinkIDs, _Q(GetCurrentUserName()), strDeleteEMRProblemIDs);

	//DRT 7/5/2006 - PLID 21084 - Auditing
	long nAuditID = BeginAuditTransaction();
	try
	{
		long nPatientID = AdoFldLong(prsDelete, "PatientID");
		CString strPatientName = GetExistingPatientName(nPatientID);
		AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientLabDeleted
			, nLabID, AdoFldString(prsDelete, "Description", ""), "<Deleted>", aepHigh, aetDeleted);

		// (z.manning 2008-10-14 10:23) - PLID 31667 - Audit any deleted to-dos
		prsDelete = prsDelete->NextRecordset(NULL);
		// (z.manning 2008-10-14 10:31) - PLID 31667 - If we deleted more than one to-do send a global table checker.
		if(!prsDelete->eof && prsDelete->GetRecordCount() > 1) {
			CClient::RefreshTable(NetUtils::TodoList);
		}
		for(; !prsDelete->eof; prsDelete->MoveNext()) {
			FieldsPtr f = prsDelete->Fields;
			long nTaskID = AdoFldLong(f, "TaskID");
			CString strAssignTo = AdoFldString(f, "AssignToNames", "");
			CString strCategory = AdoFldString(f, "Category", "");
			CString strNotes = AdoFldString(f, "Notes", "");
			// CR and LF's don't show up in the audit trail elegantly; replace them with spaces or else the note will look ugly
			strNotes.Replace(10, ' ');
			strNotes.Replace(13, ' ');
			COleDateTime dtDeadline = AdoFldDateTime(f, "Deadline");
			CString strOld = FormatString("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", 
				strAssignTo, FormatDateTimeForInterface(dtDeadline, 0, dtoDate), strCategory, strNotes);
			AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientTodoTaskDelete, nTaskID, strOld, "<Deleted>", aepMedium, aetDeleted);

			if(prsDelete->GetRecordCount() == 1) {
				// (z.manning 2008-10-14 10:30) - PLID 31667 - This is our only deleted to-do so
				// send a table checker with its ID.
				// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
				CClient::RefreshTodoTable(nTaskID, nPatientID ,-1 , TableCheckerDetailIndex::tddisDeleted);
			}
		}

		CommitAuditTransaction(nAuditID);

		// (c.haag 2010-07-19 16:04) - PLID 30894 - Fire a LabsT table checker
		// (r.gonet 09/02/2014) - PLID 63221 - Send an ex tablechecker and the patient ID
		CClient::RefreshLabsTable(nPatientID, nLabID);

		// (j.jones 2009-06-12 12:11) - PLID 34487 - send a tablechecker if problems were affected
		if(bDeletedProblems) {
			CClient::RefreshTable(NetUtils::EMRProblemsT, nPatientID);
		}

	}NxCatchAllCall("::DeletePatientLab (Audit)", RollbackAuditTransaction(nAuditID));
}

//TES 1/4/2011 - PLID 37877 - Added a parameter for nPatientID, to save a data access.
//TES 1/14/2015 - PLID 55674 - Added pListChangedTodos. If it is not null, the function will append to the list with information about Todos that need tablecheckers, 
// rather than sending the tablecheckers itself
void SyncTodoWithLab(IN const long nLabID, IN const long nPatientID, std::list<ChangedTodo> *pListChangedTodos /*= NULL*/)
{
	//TES 1/4/2011 - PLID 37877 - Moved most of this code to GlobalTodoUtils, but we're still responsible for auditing and sending
	// table checkers based on what it returns.
	CArray<CommonAuditData,CommonAuditData&> arAuditData;
	CArray<long,long> arUpdatedTodoIDs;
	CArray<long, long> arUpdatedTodosAssignedUsers;
	SyncTodoWithLab(GetRemoteData(), nLabID, GetCurrentUserID(), GetCurrentUserName(), arAuditData, arUpdatedTodoIDs, arUpdatedTodosAssignedUsers);
	//TES 1/4/2011 - PLID 37877 - Audit
	if(arAuditData.GetSize()) {
		long nAuditID = BeginAuditTransaction();
		for(int i = 0; i < arAuditData.GetSize(); i++) {
			CommonAuditData cad = arAuditData[i];
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, cad.eEvent, cad.nRecordID, cad.strOld, cad.strNew, aepMedium, cad.eType);
		}
		CommitAuditTransaction(nAuditID);
	}
	//TES 1/4/2011 - PLID 37877 - Send table checkers
	for(int i = 0; i < arUpdatedTodoIDs.GetSize(); i++) {

		
		// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
		//TES 1/14/2015 - PLID 55674 - If we have a list, just add to it
		if (pListChangedTodos) {
			ChangedTodo ct;
			ct.nTaskID = arUpdatedTodoIDs[i];
			ct.nPatientID = nPatientID;
			ct.nAssignedTo = arUpdatedTodosAssignedUsers[i];
			ct.tsStatus = TableCheckerDetailIndex::tddisAdded;
			pListChangedTodos->push_back(ct);
		}
		else {
			CClient::RefreshTodoTable(arUpdatedTodoIDs[i], nPatientID, arUpdatedTodosAssignedUsers[i], TableCheckerDetailIndex::tddisAdded);
		}
		
	}
}


// (z.manning 2008-10-29 11:27) - PLID 31667 - Takes a task ID and makes sure that if there's an associated
// lab step that it stays in sync.
void SyncLabWithTodo(const long nTaskID)
{
	if(!g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
		return;
	}

	long nAuditID = BeginAuditTransaction();
	try
	{
		// (z.manning 2008-10-29 11:55) - PLID 31667 - Do we have a lab step associted with the given todo?
		// Currently we only support marking lab steps done when marking to-dos done, so only pull the lab
		// step ID if the lab step is incomplete and the to-do is complete.
		//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
		_RecordsetPtr prsStepID = CreateParamRecordset(
			"SELECT RegardingID \r\n"
			"FROM TodoList \r\n"
			"INNER JOIN LabStepsT ON TodoList.RegardingID = LabStepsT.StepID AND RegardingType = {INT} \r\n"
			"WHERE TodoList.TaskID = {INT} AND TodoList.Done IS NOT NULL AND \r\n"
			"	StepCompletedDate IS NULL \r\n"
			, ttLabStep, nTaskID);
		if(!prsStepID->eof)
		{
			long nLabStepID = AdoFldLong(prsStepID->GetFields(), "RegardingID");
			// (z.manning 2008-10-29 11:55) - PLID 31667 - Ok, we have a lab step so mark it complete and audit as much
			//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
			_RecordsetPtr prsStepComplete = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"DECLARE @nLabID int \r\n"
				"DECLARE @nPatientID int \r\n"
				"SET @nLabID = (SELECT LabID FROM LabStepsT WHERE StepID = {INT}) \r\n"
				"SET @nPatientID = (SELECT PatientID FROM LabsT WHERE ID = @nLabID) \r\n"
				"UPDATE LabStepsT SET StepCompletedDate = GetDate(), StepCompletedBy = {INT} \r\n"
				"WHERE StepID = {INT} \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT COUNT(*) AS IncompleteStepCount, @nLabID AS LabID, @nPatientID AS PatientID \r\n"
				"FROM LabStepsT \r\n"
				"WHERE LabID = @nLabID AND StepCompletedDate IS NULL \r\n"
				, nLabStepID, GetCurrentUserID(), nLabStepID);
			long nPatientID = AdoFldLong(prsStepComplete->GetFields(), "PatientID");
			long nLabID = AdoFldLong(prsStepComplete->GetFields(), "LabID");
			CString strOld = GenerateStepCompletionAuditOld(nLabStepID);
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientLabStepMarkedComplete, nLabStepID, strOld, "<Completed>", aepMedium, aetChanged);
			// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a LabsT-level table checker because we updated lab steps
			// (r.gonet 09/02/2014) - PLID 63221 - Send an ex tablechecker and the patient ID
			CClient::RefreshLabsTable(nPatientID, nLabID);
			if(AdoFldLong(prsStepComplete->GetFields(), "IncompleteStepCount") == 0) {
				// (z.manning 2008-10-29 11:55) - PLID 31667 - There are no more incomplete steps in this lab
				// so mark the entire lab complete.
				// (z.manning 2008-10-31 10:31) - PLID 21082 - Now have a global function to do this since
				// signature is required
				// (b.cardillo 2013-09-12 08:42) - PLID 58537 - Only try to mark the lab complete if the user has permission
				if (CheckCurrentUserPermissions(bioPatientLabs, sptDynamic0)) {
					PromptToSignAndMarkLabComplete(NULL, nLabID);
				} else {
					AfxMessageBox(
						  _T("You have completed the last step for a lab, but the lab could not be marked complete because you do not have the necessary permission. The lab will remain incomplete until an administrator or other user with permission completes it.")
						, MB_OK|MB_ICONEXCLAMATION
						);
				}
			}
			else {
				// (z.manning 2008-10-29 12:05) - PLID 31667 - We marked a step complete so we may need to
				// create a to-do for the next active step.
				//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
				SyncTodoWithLab(nLabID, nPatientID);
			}
		}

		CommitAuditTransaction(nAuditID);

	}NxCatchAllCall("::SyncLabWithTodo", RollbackAuditTransaction(nAuditID));
}

// (z.manning 2008-10-30 17:08) - PLID 21082 - Prompts a user for a signature and if they enter one
// will complete the given lab.
// (r.gonet 09/02/2014) - PLID 63221 - Added the option for the function to not send a table checker.
BOOL PromptToSignAndMarkLabComplete(IN CWnd *pwndParent, IN const long nLabID, BOOL bSendTableChecker/*=TRUE*/)
{
	HWND hwndParent = pwndParent == NULL ? NULL : pwndParent->GetSafeHwnd();

	// (z.manning 2008-10-31 10:16) - PLID 21082 - Load details about the lab for dialog text and auditing.
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/8/2009 - PLID 36470 - AnatomySide is back.
	// (c.haag 2010-12-17) - PLID 41825 - If we get to this function, this lab should have at least one incomplete
	// result. If that condition fails, then we fail. We also used to not check whether the lab was deleted; we 
	// should not be allowing people to sign off on deleted labs.
	_RecordsetPtr prsLab = CreateParamRecordset(
		"SELECT LabsT.PatientID, LabsT.Deleted, LabsT.ToBeOrdered, LabsT.Type, LabsT.AnatomySide, AnatomyQualifiersT.Name AS LocationQualifier, LabsT.FormNumberTextID \r\n"
		"	, LabsT.Specimen, LabAnatomyT.Description AS AnatomicLocation \r\n"
		"	, PersonT.First, PersonT.Last \r\n"
		"	, (SELECT COUNT(*) FROM LabResultsT WHERE LabID = {INT} AND Deleted = 0) AS ResultCount \r\n"
		"	, (SELECT COUNT(*) FROM LabResultsT WHERE LabID = {INT} AND Deleted = 0 AND ResultCompletedDate IS NULL) AS IncompleteResultCount \r\n"
		"	, (SELECT COUNT(*) FROM LabResultsT WHERE LabID = {INT} AND Deleted = 0 AND ResultSignedDate IS NULL) AS UnsignedResultCount \r\n"
		"FROM LabsT \r\n"
		"LEFT JOIN LabAnatomyT ON LabsT.AnatomyID = LabAnatomyT.ID \r\n"
		"LEFT JOIN AnatomyQualifiersT ON LabsT.AnatomyQualifierID = AnatomyQualifiersT.ID \r\n"
		"LEFT JOIN PersonT ON LabsT.PatientID = PersonT.ID \r\n"
		"WHERE LabsT.ID = {INT} \r\n"
		, nLabID, nLabID, nLabID, nLabID);

	const long nPatientID = AdoFldLong(prsLab->GetFields(), "PatientID");

	// (z.manning 2008-10-31 10:16) - PLID 21082 - Construct the lab description
	CString strSpecimen = AdoFldString(prsLab->GetFields(), "Specimen", "");
	LabType eType = (LabType)AdoFldByte(prsLab->GetFields(), "Type");
	CString strTypeDesc;
	if(eType == ltBiopsy) {
		//TES 12/3/2008 - PLID 32302 - AnatomicLocation is permitted to be NULL now.
		strTypeDesc = AdoFldString(prsLab->GetFields(), "AnatomicLocation","");
		AnatomySide eSide = (AnatomySide)AdoFldLong(prsLab->GetFields(), "AnatomySide", -1);
		CString strQual = AdoFldString(prsLab->GetFields(), "LocationQualifier", "");
		// (z.manning 2010-04-30 17:36) - PLID 37553 - We now have a function to format this
		strTypeDesc = ::FormatAnatomicLocation(strTypeDesc, strQual, eSide);
	}
	else {
		strTypeDesc = AdoFldString(prsLab->GetFields(), "ToBeOrdered", "");
	}	
	CString strDescription = AdoFldString(prsLab->GetFields(), "FormNumberTextID", "");
	if(!strSpecimen.IsEmpty()) {
		strDescription += " - " + strSpecimen;
	}
	if(!strTypeDesc.IsEmpty()) {
		strDescription += " - " + strTypeDesc;
	}

	// (c.haag 2010-12-17 11:07) - PLID 41825 - We used to just have the user sign off on the lab here. These days we
	// have the user sign off on individual results. Here are the changes we're applying here:
	//
	// Old and busted
	//		User signs off on lab, marks it completed (technically they sign and complete the requisition as far as our data is concerned)
	//
	// New hotness
	//		First make sure we actually have something to sign.
	//		Second, have the user check off every result they're completing.
	//		Third, see if any results are not unsigned. If so, make the user sign those now.
	//		Finally, mark all incomplete results as complete

	// (c.haag 2010-12-17 11:07) - PLID 41825 - First make sure we have any results to sign
	if (AdoFldBool(prsLab->Fields, "Deleted")) {
		CString strMsg;
		strMsg.Format("Lab '%s' for patient '%s %s' is not eligible for being marked complete (or all of the steps marked complete) because it has been deleted."
				, strDescription, AdoFldString(prsLab->GetFields(), "First", ""), AdoFldString(prsLab->GetFields(), "Last", ""));
		AfxMessageBox(strMsg, MB_OK | MB_ICONHAND);
		return FALSE;
	}
	else if (0 == AdoFldLong(prsLab->Fields, "ResultCount")) {
		CString strMsg;
		strMsg.Format("Lab '%s' for patient '%s %s' is not eligible for being marked complete (or all of the steps marked complete) because it has no results."
				, strDescription, AdoFldString(prsLab->GetFields(), "First", ""), AdoFldString(prsLab->GetFields(), "Last", ""));
		AfxMessageBox(strMsg, MB_OK | MB_ICONHAND);
		return FALSE;
	}
	else if (0 == AdoFldLong(prsLab->Fields, "IncompleteResultCount")) {
		CString strMsg;
		strMsg.Format("Lab '%s' for patient '%s %s' is not eligible for being marked complete (or all of the steps marked complete) because it has no incomplete results."
				, strDescription, AdoFldString(prsLab->GetFields(), "First", ""), AdoFldString(prsLab->GetFields(), "Last", ""));
		AfxMessageBox(strMsg, MB_OK | MB_ICONHAND);
		return FALSE;
	}

	// (c.haag 2010-12-17 11:07) - PLID 41825 -  Second, have the user choose which results to complete if there is more than one. If
	// there is only one, skip it. To be consistent with the labs dialog, we pre-select everything. However, the user MUST leave all items
	// checked before they can dismiss the window.
	const long nIncompleteResultCount = AdoFldLong(prsLab->Fields, "IncompleteResultCount");
	if (nIncompleteResultCount > 1) 
	{
		CString strMsg = FormatString("Lab '%s' for patient '%s %s' is being marked complete (or all of the steps are complete). Please review the list of results, and click OK to continue."
				, strDescription, AdoFldString(prsLab->GetFields(), "First", ""), AdoFldString(prsLab->GetFields(), "Last", ""));
		BOOL bContinue = TRUE;

		while (bContinue) 
		{
			// (s.tullis 2015-03-24 17:54) - PLID 65338 - Do not show deleted lab results in the select list
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlgResults(pwndParent, "LabResults"); // We have to put this in the loop because the multi-select dialog was not designed to open more than once
			dlgResults.m_bPreSelectAll = TRUE;
			if (IDOK != dlgResults.Open("LabResultsT", FormatString("LabID = %d AND ResultCompletedBy IS NULL AND Deleted = 0 ", nLabID), "ResultID", "Name", strMsg))
			{
				return FALSE;
			}
			else {
				// Make sure that every selection is checked. We don't support partial completions.
				CString strSelections = dlgResults.GetMultiSelectIDString();
				CArray<long,long> arResults;
				ParseDelimitedStringToLongArray(strSelections, " ", arResults);
				if (arResults.GetSize() != nIncompleteResultCount) {
					AfxMessageBox("All results must be selected before continuing.\n\nIf you do not wish to mark all results completed at this time, please press Cancel on the selection window and open the lab for editing to mark individual results as completed.", MB_OK | MB_ICONHAND);	
				}
				else {
					// All results are selected, we can terminate the loop
					bContinue = FALSE;
				}
			}
		}
	}

	// (c.haag 2010-12-17 11:07) - PLID 41825 - Third, see if any results are not unsigned. If so, make the user sign those now.
	const long nUnsignedResultCount = AdoFldLong(prsLab->Fields, "UnsignedResultCount");
	if (nUnsignedResultCount > 0) 
	{
		// (b.cardillo 2013-09-11 21:40) - PLID 57922 - Require signing permission
		// We're going to prompt the user to sign, but first verify permission to sign
		if (!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic3)) {
			MessageBox(hwndParent
				, _T("This lab could not be marked complete because you must sign a lab in order to mark it complete and you do not have permission to sign labs. The lab will remain incomplete until an administrator or other user with permission completes it.")
				, _T("Sign Lab")
				, MB_OK|MB_ICONEXCLAMATION
				);
			return FALSE;
		}
		// (z.manning 2008-10-31 10:17) - PLID 21082 - Prompt them to sign the lab before we mark it complete
		CSignatureDlg dlgSign(pwndParent);
		dlgSign.SetText(FormatString("Lab '%s' for patient '%s %s' is being marked complete (or all of the steps are complete). Please sign in order to mark the lab complete."
			, strDescription, AdoFldString(prsLab->GetFields(), "First", ""), AdoFldString(prsLab->GetFields(), "Last", "")));
		dlgSign.m_bRequireSignature = TRUE;
		// (z.manning 2008-12-09 09:07) - PLID 32260 - Added a preference for checking for password when loading signature.
		dlgSign.m_bCheckPasswordOnLoad = (GetRemotePropertyInt("SignatureCheckPasswordLab", 1, 0, GetCurrentUserName()) == 1);

		while(dlgSign.DoModal() != IDOK) {
			CString strWarn = "You must sign a lab in order to mark it complete. Would you like to attempt signing the lab again?";
			if(IDYES != MessageBox(hwndParent, strWarn, "Sign Lab", MB_YESNO|MB_ICONQUESTION)) {
				// (z.manning 2008-10-31 10:17) - PLID 21082 - The didn't sign, so we can't mark the lab complete.
				return FALSE;
			}
		}

		CString strSignatureFileName = dlgSign.GetSignatureFileName();
		_variant_t varSignatureInkData = dlgSign.GetSignatureInkData();
		// (j.jones 2010-04-12 17:14) - PLID 38166 - added a date/timestamp
		_variant_t varSignatureTextData = dlgSign.GetSignatureTextData();
		if(varSignatureInkData.vt == VT_EMPTY) {
			varSignatureInkData.vt = VT_NULL;
		}
		if(varSignatureTextData.vt == VT_EMPTY) {
			varSignatureTextData.vt = VT_NULL;
		}

		if(varSignatureTextData.vt != VT_EMPTY && varSignatureTextData.vt != VT_NULL) {			

			// (j.jones 2010-04-13 08:48) - PLID 38166 - the loaded text is the words "Date/Time",
			// we need to replace it with the actual date/time
			CNxInkPictureText nipt;
			nipt.LoadFromVariant(varSignatureTextData);

			COleDateTime dtSign = COleDateTime::GetCurrentTime();
			_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS CurDateTime");
			if(!rs->eof){
				dtSign = AdoFldDateTime(rs, "CurDateTime");
			}
			rs->Close();

			CString strDate;
			if(dlgSign.GetSignatureDateOnly()) {
				strDate = FormatDateTimeForInterface(dtSign, NULL, dtoDate);					
			}
			else {
				strDate = FormatDateTimeForInterface(dtSign, DTF_STRIP_SECONDS, dtoDateTime);					
			}

			//technically, there should only be one SIGNATURE_DATE_STAMP_ID in the array,
			//and currently we don't support more than one stamp in the signature
			//anyways, but just incase, replace all instances of the SIGNATURE_DATE_STAMP_ID with
			//the current date/time
			for(int i=0; i<nipt.GetStringCount(); i++) {
				if(nipt.GetStampIDByIndex(i) == SIGNATURE_DATE_STAMP_ID) {
					nipt.SetStringByIndex(i, strDate);
				}
			}
			varSignatureTextData = nipt.GetAsVariant();
		}

		CString strSigTextData;
		if(varSignatureTextData.vt == VT_NULL || varSignatureTextData.vt == VT_EMPTY) {
			strSigTextData = "NULL";
		}
		else {
			strSigTextData = CreateByteStringFromSafeArrayVariant(varSignatureTextData);
		}

		// (z.manning 2008-10-31 10:17) - PLID 21082 - We have a signature so mark the lab complete.
		// Note: Param queries do not support image type yet so this is sort of a hybrid param query.
		// (j.jones 2010-04-12 17:34) - PLID 38166 - added SignatureTextData, which I confirmed
		// does not work with {VARBINARY}
		// (c.haag 2010-12-10 13:22) - PLID 41825 - We now mark results as signed. Careful to only
		// do it to unsigned results that are not deleted.
		_RecordsetPtr prs = CreateParamRecordset("SELECT ResultID FROM LabResultsT WHERE ResultSignedBy IS NULL AND Deleted = 0 AND LabID = {INT}", nLabID);
		CArray<long,long> anLabResultIDs;
		while (!prs->eof) {
			anLabResultIDs.Add(AdoFldLong(prs->Fields, "ResultID"));
			prs->MoveNext();
		}
		prs->Close();
		ExecuteParamSql(FormatString(
			"DECLARE @Now DATETIME \r\n"
			"SET @Now = GetDate() \r\n"
			"UPDATE LabResultsT SET \r\n"
			"	  ResultSignedDate = @Now \r\n"
			"	, ResultSignedBy = {INT} \r\n"
			"	, ResultSignatureImageFile = {STRING} \r\n"
			"	, ResultSignatureInkData = {VARBINARY} \r\n"
			"	, ResultSignatureTextData = %s \r\n"
			"WHERE ResultSignedBy IS NULL AND Deleted = 0 AND LabID = {INT} \r\n"
			,
			strSigTextData),
			GetCurrentUserID(), strSignatureFileName, varSignatureInkData,
			nLabID); 

		// Auditing
		CAuditTransaction audit;
		for (int i=0; i < anLabResultIDs.GetSize(); i++) {
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), audit, aeiPatientLabResultSigned, anLabResultIDs[i], strDescription, "<Signed>", aepMedium, aetChanged);
		}
		audit.Commit(); // Rollbacks are automatically handled from within the class
	}

	// (c.haag 2010-12-17 11:07) - PLID 41825 - Finally, mark all incomplete results as complete
	{
		_RecordsetPtr prs = CreateParamRecordset("SELECT ResultID FROM LabResultsT WHERE ResultCompletedBy IS NULL AND Deleted = 0 AND LabID = {INT}", nLabID);
		CArray<long,long> anLabResultIDs;
		while (!prs->eof) {
			anLabResultIDs.Add(AdoFldLong(prs->Fields, "ResultID"));
			prs->MoveNext();
		}
		prs->Close();
		ExecuteParamSql(
			"DECLARE @Now DATETIME \r\n"
			"SET @Now = GetDate() \r\n"
			"UPDATE LabResultsT SET \r\n"
			"	ResultCompletedDate = @Now \r\n"
			" , ResultCompletedBy = {INT} \r\n"
			"WHERE ResultCompletedBy IS NULL AND Deleted = 0 AND LabID = {INT} \r\n",
			GetCurrentUserID(), nLabID);

		// (b.spivey, April 22, 2013) - PLID 55943 - Get a list of lab steps 
		ADODB::_RecordsetPtr prsStepsCompleteBySigning = CreateParamRecordset(
			"SELECT LST.StepID, LST.StepCompletedBy, LST.StepCompletedDate, LPST.CreateLadder "
			"FROM LabStepsT LST "
			"INNER JOIN LabProcedureStepsT LPST ON LST.LabProcedureStepID = LPST.StepID "
			"LEFT JOIN ( "
			"	SELECT MIN(CASE WHEN LabResultsT.ResultSignedDate IS NOT NULL THEN 1 ELSE 0 END) AS AllSigned, LabID "
			"	FROM LabResultsT "
			"	WHERE Deleted = 0 "
			"	GROUP BY LabID "
			") LabResultsT ON LST.LabID = LabResultsT.LabID "
			"WHERE LST.LabID = {INT} AND LPST.CompletedBySigning = {BIT} "
			"	AND (StepCompletedBy IS NULL OR StepCompletedBy = '') "
			"	AND (StepCompletedDate IS NULL OR StepCompletedDate = '') "
			"	AND LabresultsT.AllSigned = 1 " 
			"ORDER BY LST.StepOrder ", nLabID, TRUE);

		// (b.spivey, April 22, 2013) - PLID 55943 - Get a list of all lab steps that complete by signing all lab steps
		//	 if all lab steps are signed. 
		while(!prsStepsCompleteBySigning->eof) {
			long nStepID = AdoFldLong(prsStepsCompleteBySigning->Fields, "StepID", -1);
			GlobalModifyStepCompletion(nLabID, nStepID, true, true); 
			prsStepsCompleteBySigning->MoveNext(); 
		}

		// (b.spivey, April 05, 2013) - PLID 44387 - See if we have any steps left to complete. 
		//		If so, ask if they want to complete them, then sync todos. 
		if(ReturnsRecordsParam("SELECT StepID FROM LabStepsT WHERE StepCompletedDate IS NULL AND LabID = {INT} ", nLabID) &&
			AfxMessageBox("Do you want to mark all open lab steps as completed?", MB_YESNO|MB_ICONQUESTION) == IDYES) {


			// Complete lab steps. 
			// (b.spivey, April 23, 2013) - PLID 44387 - Complete steps.
			_RecordsetPtr prsUncompleteSteps = CreateParamRecordset(
				"BEGIN TRAN "
				"SET NOCOUNT ON " 
				"SELECT StepID "
				"FROM LabStepsT "
				"WHERE StepCompletedDate IS NULL AND StepCompletedBy IS NULL "
				"	AND LabID = {INT} "
				"	"
				"UPDATE LabStepsT "
				"SET StepCompletedDate = GETDATE(), StepCompletedBy = {INT} "
				"WHERE StepCompletedDate IS NULL AND StepCompletedBy IS NULL "
				"	AND LabID = {INT} "
				"SET NOCOUNT OFF "
				"COMMIT TRAN "
				, nLabID
				, GetCurrentUserID()
				, nLabID); 
			

			//We updated something!
			// (b.spivey, April 23, 2013) - PLID 44387 - Audit. 
			if(!prsUncompleteSteps->eof) {
				
				CAuditTransaction auditTran;
				CString strPatientName = GetExistingPatientName(nPatientID); 
				//audit. 
				while(!prsUncompleteSteps->eof) {
					
					long nRecordID = AdoFldLong(prsUncompleteSteps->Fields, "StepID", -1); 
					CString strOld = GenerateStepCompletionAuditOld(nRecordID);
					CString strNew = "<Completed>"; 
					AuditEvent(nPatientID, strPatientName, auditTran, aeiPatientLabStepMarkedComplete, nRecordID, strOld, strNew, aepMedium, aetChanged);
					prsUncompleteSteps->MoveNext(); 
				}

				auditTran.Commit(); 

			}

		}

		// (b.spivey, April 22, 2013) - PLID 55943 - Sync todos for completed by signing steps. 
		SyncTodoWithLab(nLabID, nPatientID); 

		// Auditing
		CAuditTransaction audit;
		for (int i=0; i < anLabResultIDs.GetSize(); i++) {
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), audit, aeiPatientLabResultMarkedComplete, anLabResultIDs[i], strDescription, "<Signed>", aepMedium, aetChanged);
		}
		audit.Commit(); // Rollbacks are automatically handled from within the class
	}

	// (r.gonet 09/02/2014) - PLID 63221 - Added the option for the function to not send a table checker.
	if (bSendTableChecker) {
		// (c.haag 2010-07-19 16:04) - PLID 30894 - Fire a LabsT table checker since we modified LabsT
		// (r.gonet 09/02/2014) - PLID 63221 - Send an ex tablechecker and the patient ID
		CClient::RefreshLabsTable(nPatientID, nLabID);
	} else {
		// Don't send a table checker
	}

	return TRUE;
}

// (j.jones 2008-11-19 09:33) - PLID 28578 - this function will compare two passwords, case-sensitive,
// but if bIsPasswordVerified is FALSE then we have to do some advanced case-insensitive work
// (b.savon 2015-12-16 10:47) - PLID 67705 - Restructured this to use locationID and username; and call the api
// (z.manning 2016-04-19 14:48) - NX-100244 - Added optional param for if the user is locked out
BOOL IsUserPasswordValid(CString strPassword, long nLocationID , long nUserID, const CString& strUsername, OPTIONAL OUT BOOL *pbLockedOut /* = NULL */)
{
	//throw exceptions to the caller

	// the built-in tech support user is always case sensitive
	if (nUserID == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID)
	{
		// (z.manning 2015-11-25 10:56) - PLID 67637 - We now have a completely different way of checking
		// the support password. For increased security, we check it using the license activation server
		// (so that we don't have any knowledge whatsoever of what the password is on the client).
		return g_pLicense->CheckSupportSecretPassword(strPassword);
	}
	else {
		// (b.savon 2015-12-16 10:41) - PLID 67705 - Change Practice to call our new VerifyPassword API method instead of calling SQL from within C++ code.
		// Create User member of argument
		NexTech_Accessor::_UserPtr pUser(__uuidof(NexTech_Accessor::User));
			CString strUserID;
			strUserID.Format("%li", nUserID);
			CString strLocationID;
			strLocationID.Format("%li", nLocationID);
			pUser->PutID(AsBstr(strUserID));
			pUser->Putusername(AsBstr(strUsername));
			pUser->PutlocationID(AsBstr(strLocationID));

		// Verify the password
		NexTech_Accessor::_ValidateLoginCredentialsResultPtr pResult = GetAPI()->VerifyUserPassword(GetAPISubkey(), AsBstr(strPassword), pUser);

		// (z.manning 2016-04-19 14:51) - NX-100244 - Set the locked out flag if it was passed in
		if (pbLockedOut != nullptr) {
			*pbLockedOut = pResult->LockedOut ? TRUE : FALSE;
		}

		if (pResult == NULL) {
			return FALSE;
		}
		else {
			if (pResult->GetValid() == VARIANT_FALSE) {
				return FALSE;
			}
			else {
				if (nUserID == GetCurrentUserID()) {
					SetCurrentUserPassword(strPassword);
					SetCurrentUserPasswordVerified(TRUE);
				}
				return TRUE;
			}
		}
	}
	
}

// (j.jones 2009-05-20 09:26) - PLID 33801 - update the UserPasswordHistoryT for the given user
void UpdateUserPasswordHistory(long nUserID, CString strOldPassword, CString strNewPassword)
{
	//throw exceptions to the caller, this can be called within a transaction

	if(strOldPassword == strNewPassword) {
		//do NOT update if the password didn't actually change
		return;
	}

	CString strSqlBatch;
	CNxParamSqlArray aryParams;
	
	_variant_t varOldPass = EncryptStringToVariant(strOldPassword);
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO UserPasswordHistoryT (UserID, Password) "
		"VALUES ({INT}, {VARBINARY})", nUserID, varOldPass);

	//Keep only the last 24 entries for this user. Now, the 24 number counts the current password
	//stored in UsersT (the one we just changed to), so technically only 23 entries need to remain
	//in the history table, but I am keeping 24 just for padding incase we change our algorithm in
	//the future to check on the last X passwords prior to the current password.
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM UserPasswordHistoryT "
		"WHERE UserID = {INT} AND ID NOT IN "
		"	(SELECT TOP 24 ID FROM UserPasswordHistoryT "
		"	WHERE UserID = {INT} "
		"	ORDER BY DateLastUsed DESC "
		"	)", nUserID, nUserID);

	// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
	ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
}

// (z.manning 2008-12-16 13:17) - PLID 27682 - Global function to spell check an edit control
void SpellCheckEditControl(CWnd *pwndParent, CEdit *pEdit)
{
	// (z.manning 2008-12-16 13:19) - PLID 27682 - Moved this code here from CEmrItemAdvTextDlg::OnSpellCheck()

	ASSERT(pEdit != NULL && IsWindow(pEdit->GetSafeHwnd()));
	CWaitCursor wc;

	// Ask that owner for its spell checking mechanism
	WSPELLLib::_DWSpellPtr pwsSpellChecker = CreateSpellExObject(pwndParent);
	if (pwsSpellChecker) {
		// Tell the spell checker to look at our text box
		pwsSpellChecker->PutTextControlHWnd((long)pEdit->GetSafeHwnd());
		pwsSpellChecker->ClearTempDictionary();
		// Do the spell check and respond to the result
		short nAns = pwsSpellChecker->Start();
		if (nAns == 0) {
			// Success
			pwndParent->MessageBox("Spell check completed successfully.", NULL, MB_OK|MB_ICONINFORMATION);
		}
		else if (nAns == -17) {
			// User canceled
		}
		else {
			// An error
			CString strErr;
			strErr.Format("ERROR %hi was reported while trying to check spelling!", nAns);
			pwndParent->MessageBox(strErr, NULL, MB_OK|MB_ICONEXCLAMATION);
		}
		// Put focus back inside the text box where it belongs
		pEdit->SetFocus();
	}
	else {
		pwndParent->MessageBox("The spell check could not begin because the necessary components were not loaded.  Please verify that Practice is installed correctly and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
	}
}

// (c.haag 2009-01-19 11:14) - PLID 32712 - Returns TRUE if an element exists in a long array.
// This function already existed in CEMRTableDropdownEditorDlg; I just made it global.
BOOL ExistsInArray(long n, const CArray<long,long>& a)
{
	const int nSize = a.GetSize();
	for (int i=0; i < nSize; i++) {
		if (a[i] == n) {
			return TRUE;
		}
	}
	return FALSE;
}

//(e.lally 2009-01-28) PLID 32814 - Global function to check for email address validity.
BOOL IsValidEmail(const CString& strEmailAddress)
{
	//Check for some validity - use a simplistic approach for now.

	//check for an '@' sign, past the first character
	int nAtIndex = strEmailAddress.Find("@", 1);
	//Check for a '.', past the '@' sign
	int nDotIndex = strEmailAddress.Find(".", nAtIndex >=0 ? nAtIndex:0);

	if(nAtIndex < 0 || nDotIndex <= nAtIndex){
		return FALSE;
	}
	return TRUE;

}

// (z.manning 2009-05-05 15:27) - PLID 28529 - This function takes a datalist2 pointer and a column.
// It will then cycle through every row in the datalist and make any rows where the value in 
// nBoolCol is true invisible.
void HideDatalistRowsBasedOnBooleanColumn(IN NXDATALIST2Lib::_DNxDataListPtr pdl, const short nBoolCol)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pdl->GetFirstRow();
	for(; pRow != NULL; pRow = pRow->GetNextRow())
	{
		_variant_t var = pRow->GetValue(nBoolCol);
		if(var.vt == VT_BOOL) {
			if(VarBool(var)) {
				pRow->PutVisible(VARIANT_FALSE);
			}
		}
	}
}

//TES 5/15/2009 - PLID 28559 - Moved here from Auditing.cpp
//used when generating the user-defined filter

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString AsClauseString(const _variant_t &varValue)
//{
//	CString strText; 
//	
//	// Get the value as text
//	switch (varValue.vt) {
//	case VT_BSTR:
//		return "LIKE '" + _Q(CString((LPCTSTR)_bstr_t(varValue))) + "'";
//		break;
//	case VT_BOOL:
//		strText.Format("= %s", varValue.bVal ? "1" : "0");
//		break;
//	case VT_I2:
//		strText.Format("= %hi", varValue.iVal);
//		break;
//	case VT_I4:
//		strText.Format("= %li", varValue.lVal);
//		break;
//	case VT_R4:
//		strText.Format("= %g", varValue.fltVal);
//		break;
//	case VT_R8:
//		strText.Format("= %lg", varValue.dblVal);
//		break;
//	case VT_DATE:
//		return "= '" + FormatDateTimeForSql(COleDateTime(varValue.date)) + "'";
//	case VT_CY:
//		return "= " + FormatCurrencyForSql(varValue.cyVal);
//	case VT_NULL:
//	case VT_EMPTY:
//		return "IS NULL";
//	default:
//		ASSERT(FALSE);
//		_com_issue_error(DISP_E_BADVARTYPE);
//		break;
//	}
//
//	return strText;
//}

//TES 5/26/2009 - PLID 34302 - Returns a version of the passed in text that can be put straight into a LIKE clause
CString FormatForLikeClause(const CString &strIn)
{
	CString strOut;
	for(int i = 0; i < strIn.GetLength(); i++) {
		if(strIn[i] == '\'') {
			strOut += "''";
		}
		else if(strIn[i] == '[') {
			strOut += "[[]";
		}
		else if(strIn[i] == '_') {
			strOut += "[_]";
		}
		else if(strIn[i] == '%') {
			strOut += "[%]";
		}
		else {
			strOut += strIn[i];
		}
	}
	return strOut;
}

//TES 5/26/2009 - PLID 34302 - De-escapes the passed in text, which is assumed to be properly escaped for a LIKE clause
CString UnformatFromLikeClause(const CString &strIn)
{
	CString strOut;
	bool bEscaped = false;
	bool bPrevQuote = false;
	for(int i = 0; i < strIn.GetLength(); i++) {
		if(strIn[i] == '\'') {
			if(bPrevQuote) {
				bPrevQuote = false;
			}
			else {
				bPrevQuote = true;
				strOut += "'";
			}
		}
		else if(strIn[i] == '[') {
			if(bEscaped) {
				strOut += "[";
			}
			else {
				bEscaped = true;
			}
		}
		else if(strIn[i] == ']') {
			if(bEscaped) {
				bEscaped = false;
			}
			else {
				strOut += "]";
			}
		}
		else {
			strOut += strIn[i];
		}
	}
	return strOut;
}

// (j.jones 2009-09-24 10:13) - PLID 29718 - added global function to get a patient's
// last appointment date prior to today (will return an invalid date if no appt. exists)
COleDateTime GetLastPatientAppointmentDate(long nPatientID)
{
	//throw exceptions to the caller

	COleDateTime dt;
	dt.SetStatus(COleDateTime::invalid);

	if(nPatientID == -1) {
		return dt;
	}

	//find the last non-cancelled appointment, that is not in the future
	_RecordsetPtr rs = CreateParamRecordset("SELECT Max(AppointmentsT.Date) AS LastAppt "
		"FROM AppointmentsT "
		"WHERE AppointmentsT.PatientID = {INT} "
		"AND dbo.AsDateNoTime(AppointmentsT.Date) < GetDate() AND AppointmentsT.Status <> 4", nPatientID);
	if(!rs->eof) {

		_variant_t var = rs->Fields->Item["LastAppt"]->Value;
		if(var.vt == VT_DATE) {
			dt = VarDateTime(var);
			dt.SetDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
		}
	}
	rs->Close();

	return dt;
}

// (c.haag 2009-10-12 12:25) - PLID 35722 - Creates a SQL batch statement for deleting MailSent records based on the input filter
// (c.haag 2010-06-02 15:46) - PLID 38731 - We now delete related todo alarms
//(e.lally 2011-10-31) PLID 41195 - Disconnect the mailSent record from any FaxLogT entries
void AddDeleteMailSentQueryToSqlBatch(IN OUT CString &strSqlBatch, const CString& strMailSentInClause)
{
	AddStatementToSqlBatch(strSqlBatch, "UPDATE FaxLogT Set MailID = NULL WHERE MailID IN (%s)", strMailSentInClause);
	// (b.savon 2015-06-03 10:53) - PLID 65698 - Clear the reference in PersonT for the PortalImageID before deleting the MailSent record -- See FK_PersonT_PortalImageID
	AddStatementToSqlBatch(strSqlBatch, "UPDATE PersonT SET PortalImageID = NULL WHERE PortalImageID IN (%s)", strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSentNotesT WHERE MailID IN (%s)", strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSentProcedureT WHERE MailSentID IN (%s)", strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSentPictureTagT WHERE MailSentID IN (%s)", strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSentServiceCptT WHERE MailSentID IN (%s)", strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE RegardingType = %d AND RegardingID IN (%s))", ttMailSent, strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM TodoList WHERE RegardingType = %d AND RegardingID IN (%s)", ttMailSent, strMailSentInClause);
	// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
	// (c.haag 2010-08-26 14:45) - PLID 39473 - We're not deleting the notes anymore; just disconnecting them
	//AddStatementToSqlBatch(strSqlBatch, "DELETE FROM NoteDataT WHERE ID IN (SELECT NoteID FROM NoteInfoT WHERE MailID IN (%s))", strMailSentInClause); // (c.haag 2010-07-01 16:43) - PLID 39473
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM NoteInfoT WHERE MailID IN (%s)", strMailSentInClause);
	// (r.farnworth 2013-11-26 14:11) - PLID 59520 - Need to check for NexWebCCDAAccess before deleting MailSent items.
	AddStatementToSqlBatch(strSqlBatch, "DELETE E FROM NexWebCcdaAccessHistoryT E INNER JOIN MailSent W ON E.MailSentMailID = W.MailID WHERE W.MailID IN (%s)", strMailSentInClause);
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM DirectAttachmentT WHERE MailSentID in (%s)",strMailSentInClause); // (j.camacho 2013-12-12 16:40) - PLID 59514 - Clean up directattachmentsT before removing items from mailsent
	AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MailSent WHERE MailID IN (%s)", strMailSentInClause);
}

// (c.haag 2009-10-12 12:42) - PLID 35722 - This function deletes MailSent records. To be consistency with the legacy
// queries in the calling functions, we do not use a batch, and do not use transactions or auditing. This is a raw deletion.
// (c.haag 2010-06-02 15:46) - PLID 38731 - We now delete related todo alarms
void DeleteMailSentRecords(const CString& strMailSentInClause)
{
	// (a.walling 2010-08-02 12:11) - PLID 39867 - This is ridiculous. 'Consistency' be damned. I can't fathom any reason we should not use a batch.
	CString strSqlBatch = BeginSqlBatch();
	AddDeleteMailSentQueryToSqlBatch(strSqlBatch, strMailSentInClause);

	//(e.lally 2012-02-07) PLID 46754 - Actually execute the sql batch we created.
	ExecuteSqlBatch(strSqlBatch);
}

// (a.walling 2009-11-23 11:54) - PLID 36396 - Return a new GUID string (with dashes)
// Now in NxSystemUtilitiesLib
//CString GetNewUniqueID()
//{
//	UUID uuid;
//	UuidCreate(&uuid);
//	RPC_CSTR uuidCstr;
//	UuidToString(&uuid, &uuidCstr);
//
//	CString strUuid = uuidCstr;
//	RpcStringFree(&uuidCstr);
//
//	return strUuid;
//}

// (a.walling 2009-12-17 08:32) - PLID 36624 - Get ISO8601 timestamp
CString GetISO8601Timestamp()
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	return GetISO8601Timestamp(st);
}

CString GetISO8601Timestamp(SYSTEMTIME& st)
{
	CString strTimestamp;
	strTimestamp.Format("%04hu-%02hu-%02huT%02hu:%02hu:%02hu.%03huZ",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds);

	return strTimestamp;
}

// (b.cardillo 2010-01-07 12:48) - PLID 35780 - Added generic function to handle complete flow (including UI) for 
// prompting the user to save a file and saving it

// Handles the complete flow (including UI) for prompting the user to save any kind of file, and saving the file
BOOL PromptSaveFile(OUT CString &strSavedToFile, CWnd *pParentWnd, LPCTSTR strDefaultFileTitle, LPCTSTR strFilter, LPCTSTR strDefaultExtension, LPFUNCGETOUTPUTBYTES pfnGetOutputBytes, LPVOID pParam, BOOL bMessageBoxOnSuccess, BOOL bMessageBoxOpenSavedFileIfCsvOrTxt)
{
	CString strFilename = strDefaultFileTitle;

	CFileDialog fd(FALSE, strDefaultExtension, strFilename, OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_ENABLESIZING, strFilter, pParentWnd);
	if (fd.DoModal() == IDOK) {
		// Open the output file
		strFilename = fd.GetPathName();
		CFile f(strFilename, CFile::modeWrite|CFile::modeCreate|CFile::shareDenyWrite);

		CWaitCursor pWait;
		
		// Get the export text
		LARGE_INTEGER liPosition;
		liPosition.QuadPart = 0;
		while (true) {
			LPBYTE pData = NULL;
			UINT nSize = 0;
			pfnGetOutputBytes(pParam, liPosition, &pData, &nSize);
			if (pData != NULL) {
				if (nSize > 0) {
					// Write the text to the file
					try {
						f.Write(pData, nSize);
						delete[] pData;
					} NxCatchAllSilentCallThrow(
						try {
							delete[] pData;
						} catch (...) { }
					);
				} else {
					delete[] pData;
					break;
				}
			} else {
				break;
			}
		}

		//close the file
		f.Close();

		if (bMessageBoxOnSuccess) {
			//the file dialog ought to have always appended .csv, but just incase
			//they did not save a .csv or .txt, do not prompt to open the file
			//unless it has one of these two extensions
			if (bMessageBoxOpenSavedFileIfCsvOrTxt && strFilename.GetLength() > 4 && (strFilename.Right(4).CompareNoCase(".csv") == 0 || strFilename.Right(4).CompareNoCase(".txt") == 0)) {
				int nDoOpenSavedFile = pParentWnd->MessageBox("The file has been successfully saved. Would you like to open the file now?", "Practice", MB_ICONQUESTION|MB_YESNO);
				if (nDoOpenSavedFile == IDYES) {
					// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
					if ((int)ShellExecute(pParentWnd->GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32) {
						pParentWnd->MessageBox("Could not open the file. You will need to try to open it manually.", "Practice", MB_ICONEXCLAMATION|MB_OK);
					}
				}
			} else {
				pParentWnd->MessageBox("The file has been successfully saved.", "Practice", MB_ICONINFORMATION|MB_OK);
			}
		}

		// Done, return success
		strSavedToFile = strFilename;
		return TRUE;
	} else {
		return FALSE;
	}
}

// (b.cardillo 2010-01-07 12:48) - PLID 35780 - Implemented typical use of above generic function to save csv

// Implementation of the LPFUNCGETOUTPUTBYTES callback function to retrieve the CSV text from a datalist or datalist2 and return it all in one block
void CALLBACK GetOutputBytes_DLor2_ISODates(IN LPVOID pParam, IN OUT LARGE_INTEGER &liPosition, OUT LPBYTE *ppOutput, OUT UINT *pnOutputLen)
{
	if (liPosition.QuadPart == 0) {
		CString strExportText;
		{
			_variant_t varEmpty;
			NXDATALIST2Lib::_DNxDataListPtr pDL2 = (LPUNKNOWN)pParam;
			if (pDL2) {
				strExportText = (LPCTSTR)pDL2->CalcExportText(NXDATALIST2Lib::efCD_ForceISODates, VARIANT_FALSE, varEmpty, varEmpty);
			} else {
				NXDATALISTLib::_DNxDataListPtr pDL = (LPUNKNOWN)pParam;
				strExportText = (LPCTSTR)pDL->CalcExportText(NXDATALISTLib::efCD_ForceISODates, VARIANT_FALSE, varEmpty, varEmpty);
			}
		}
		*pnOutputLen = strExportText.GetLength() * sizeof(TCHAR);
		*ppOutput = new BYTE[*pnOutputLen];
		memcpy(*ppOutput, (LPCTSTR)strExportText, *pnOutputLen);
		liPosition.QuadPart += *pnOutputLen;
	} else {
		*ppOutput = NULL;
		*pnOutputLen = 0;
	}
}

// Wrapper for the typical use of PromptSaveFile(), in which you want to save the contents of a datalist or datalist2 to a csv file
CString PromptSaveFile_CsvFromDLor2(LPUNKNOWN pUnkDataListOr2, CWnd *pParentWnd, LPCTSTR strDefaultFileTitle, BOOL bAutoPrompt)
{
	CString strSavedFile;
	if (PromptSaveFile(strSavedFile, pParentWnd, strDefaultFileTitle, "CSV Files (*.csv;*.txt)|*.csv;*.txt|All Files (*.*)|*.*||", "csv", GetOutputBytes_DLor2_ISODates, pUnkDataListOr2, bAutoPrompt, bAutoPrompt)) {
		return strSavedFile;
	} else {
		return "";
	}
}

//TES 2/10/2010 - PLID 37296 - Moved here from PatientLabsDlg.cpp
// (m.hancock 2006-07-24 11:47) - PLID 21582 - Make sure that modifying steps is audited
//For audits on modifying steps, we have give a description containing the lab's description and the step's name
CString GenerateStepCompletionAuditOld(long nStepID)
{
	CString strOld;

	// (z.manning 2008-10-30 15:08) - PLID 31864 - Use to be ordered instead of anatomic location for
	// non-biopsy labs.
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/8/2009 - PLID 36470 - Restored AnatomySide
	// (z.manning 2010-04-30 15:33) - PLID 37553 - We now have a view to pull anatomic location
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr prsData = CreateParamRecordset(FormatString(
		"SELECT FormNumberTextID + ' - ' + CASE WHEN LabsT.Specimen IS NULL THEN '' ELSE LabsT.Specimen END + ' - ' + \r\n"
		"	CASE WHEN LabsT.Type = %i THEN \r\n"
		"		LabAnatomicLocationQ.AnatomicLocation \r\n"
		"		ELSE LabsT.ToBeOrdered END + \r\n"
		"	' - ' + LabStepsT.Name AS Description \r\n"
		"FROM LabsT \r\n"
		"LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID \r\n"
		"LEFT JOIN LabStepsT ON LabsT.ID = LabStepsT.LabID \r\n"
		"WHERE LabsT.ID = LabStepsT.LabID AND LabStepsT.StepID = {INT} \r\n"
		, ltBiopsy), nStepID);
	if(!prsData->eof) {
		strOld = AdoFldString(prsData, "Description", "");
	}
	else {
		//shouldnt be possible
		ASSERT(FALSE);
		strOld.Format("<Step Modified %li>", nStepID);
	}

	return strOld;
}

//TES 4/5/2010 - PLID 38040 - Moved from NxDialog
//TES 4/6/2010 - PLID 38040 - A copy of this function exists in NxServer's general.cpp, in the highly unlikely event that it ever changes,
// make sure it's updated there as well.

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString FixCapitalization(CString strCapitalized)
//{
//	CString strInit = "", strFixed = "";
//	TCHAR chTmp = 0;
//	_variant_t varReturn;
//	bool bNextUpper = true;
//	strInit = strCapitalized;
//	
//	for( int i = 0; i < strInit.GetLength(); i++){
//		chTmp = strInit.GetAt(i);
//		// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - Explicitly cast as char
//		if(bNextUpper) //This will be true the first time
//			strFixed += (char)toupper(strInit.GetAt(i));
//		else 
//			strFixed += (char)::tolower(strInit.GetAt(i));
//
//		if( chTmp == ' ')
//			bNextUpper = true;
//		else 
//			bNextUpper = false;
//	}
//
//	strFixed.TrimLeft();
//	strFixed.TrimRight();
//	return strFixed;
//}

// (j.jones 2010-04-23 09:42) - PLID 11596 - moved from RSIMMSLink to be a global function
CString GetThisComputerName()
{
	CString strComputerName;
	DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;
	BOOL bGotName = GetComputerName(strComputerName.GetBuffer(dwLen), &dwLen);
	if (!bGotName && GetLastError() == ERROR_BUFFER_OVERFLOW) {
		strComputerName.ReleaseBuffer(0);
		dwLen++;
		bGotName = GetComputerName(strComputerName.GetBuffer(dwLen), &dwLen);
	}
	if (bGotName) {
		strComputerName.ReleaseBuffer(dwLen);
	} else {
		strComputerName.ReleaseBuffer(0);
		strComputerName = "<unknown computer>";
	}
	return strComputerName;
}

// (j.jones 2010-04-23 09:44) - PLID 11596 - added to cleanly support the ltUserComputer preference type
CString GetCurrentUserComputerName()
{
	// (r.gonet 2016-05-19 18:13) - NX-100689 - Get the system name from the property 
	//manager rather than the license object.
	CString strComputerName = g_propManager.GetSystemName();
	if (strComputerName.IsEmpty()) {
		strComputerName = "<unknown computer>";
	}
	return GetCurrentUserName() + ("-" + strComputerName);
}

// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
// (j.jones 2010-06-30 11:42) - PLID 38031 - moved from the CCDInterface namespace, renamed to be generic
// (d.singleton 2014-06-04 10:40) - PLID 61927 takes mailsent id
void ViewXMLDocument(const CString& strXMLFilePath, CWnd* pParent, long nMailSentID/*=-1*/)
{
	// (j.jones 2010-06-30 10:58) - PLID 38031 - LoadXMLDocument has been moved to the NxXMLUtils namespace
	// (d.singleton 2014-06-04 10:40) - PLID 61927 takes mailsent id
	// (c.haag 2015-05-04) - NX-100442 - Moved all the code into DocumentOpener
	CDocumentOpener::OpenXMLDocument(strXMLFilePath, nMailSentID, pParent);
}

// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
// (j.jones 2010-06-30 11:42) - PLID 38031 - moved from the CCDInterface namespace, renamed to be generic
// (b.spivey, August 12, 2013) - PLID 57964 - CCDA viewin'
// (d.singleton 2014-06-04 10:40) - PLID 61927 takes mailsent id
void ViewXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, CWnd* pParent, long nMailSentID/*=-1*/)
{
	// sanity check		
	// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
	// (j.jones 2010-06-30 10:57) - PLID 38031 - GetXMLDocumentType is now
	// part of NxXMLUtils, not part of the CCD namespace
	// (c.haag 2015-05-04) - NX-100442 - Moved all the code into DocumentOpener
	CDocumentOpener::OpenXMLDocument(pDocument, nMailSentID, pParent);
}

// (j.jones 2010-06-30 11:58) - PLID 38031 - moved from the CCD namespace
CString GetGenericXMLXSLResourcePath()
{
	// (a.walling 2013-11-18 11:37) - PLID 59557 - XSL resources to named resources
	return "nxres://0/XSL/NxDefaultXml.xsl";
}

//TES 9/11/2012 - PLID 52518 - Added, used for viewing the output from the PQRI Reporting dialog
CString GetPQRSXSLResourcePath()
{
	// (a.walling 2013-11-18 11:37) - PLID 59557 - XSL resources to named resources
	return "nxres://0/XSL/NxPQRS.xsl";
}

// (j.jones 2013-10-11 10:03) - PLID 58806 - added QRDA XSL
CString GetQRDAXSLResourcePath()
{
	//this currently uses the same XSL as the CCDA
	return CCD::GetCCDAXSLResourcePath();
}

// (f.dinatale 2010-10-11) - PLID 33753 - Added a function to format SSN strings
// (f.dinatale 2010-10-20) - PLID 33753 - Made it so that the function will return the CString instead
// of using modifying a reference to the string. Also made it so that the number of allowable characters
// can be passed in.
CString FormatSSNText(const CString & strSSNin, ESSNMask eMask, const CString& strFormat, int nAllowedCharacters /*=4*/)
{
	// If the SSN is empty, return the format.
	// (f.dinatale 2011-01-18) - PLID 33753 - Modified so that now it returns an empty string.  Also added trimming to get rid of spaces.
	CString strIn = strSSNin;
	strIn.TrimLeft();
	strIn.TrimRight();

	if(strIn == "") {
		return "";
	} else {
		switch (eMask)
		{
		case eSSNFullMask:
			// If the user does not have permission to see the SSN, return "<Hidden>"
			return "<Hidden>";
			break;
		case eSSNNoMask:
			{
				CString strFmtSSN;

				// If the user has permission to see the full SSN, return it properly formatted.
				FormatText(strIn, strFmtSSN, strFormat);
				return strFmtSSN;
			}
			break;
		case eSSNPartialMask:
			{
				// Default the result to the Format string.
				CString strResult = strFormat;

				// Get the position of the last characters of both the Format and SSN strings.
				int iInput = strIn.GetLength() - 1;
				int iOutput = strResult.GetLength() - 1;

				// Walk through the string backwards and fill in the allowed number of characters.
				while (iInput >= 0 && iOutput >= 0 && nAllowedCharacters > 0) {
					// If the character isn't a digit, skip it.
					if (!isdigit((unsigned char)strIn[iInput])) {
						iInput--;
					} else if (strResult[iOutput] == '#') {
						strResult.SetAt(iOutput, strIn[iInput]);
						iOutput--;
						iInput--;
						nAllowedCharacters--;
					} else {
						iOutput--;
					}
				}

				// Return the modified string.
				return strResult;
			}
			break;
		default:
			// If for whatever reason an unexpected enum is passed in, throw an exception.
			ThrowNxException("Invalid social security mask setting");
			return strFormat;
		}
	}
}

// (f.dinatale 2010-10-11) - PLID 33753 - Overloaded the FormatSSN function so that it can now work right on form objects.
// (f.dinatale 2010-10-20) - PLID 33753 - Reverted the function back to its old form, but encapsulated it in this function
// so that it can be used for any editbox that contains a SSN.  The function is meant to be used when a user is editting
// the editbox that has the SSN, therefore it assumes that they have permission to view the full SSN.  Also made the format
// a parameter in the case that we decide to take the SSN masking idea to a more international level.
// (f.dinatale 2011-01-18) - PLID 33753 - Made it so that the format is no longer subbed in for an empty string.
void FormatSSN(CEdit* pEdit, const CString& strFormat)
{
	CString str;
	pEdit->GetWindowTextA(str);
	str.TrimRight();
	if (str != "") {
		FormatItemText((CWnd*) pEdit, str, strFormat);
	}
}

// (z.manning 2010-10-28 11:40) - PLID 41129 - Created a global function to get the basic patient list from clause
// (previoulsy this code was essentially duplicated in CResEntryDlg and PatientToolBar).
CString GetPatientFromClause()
{
	//TES 1/18/2010 - PLID 36895 - Need to know which patients are blocked, so that we can hide their demographic info.
	CArray<IdName,IdName> arBlockedGroups;
	GetMainFrame()->GetBlockedGroups(arBlockedGroups);
	CString strGroupIDs;
	for(int i = 0; i < arBlockedGroups.GetSize(); i++) {
		strGroupIDs += AsString(arBlockedGroups[i].nID) + ",";
	}
	CString strExtraWhere;
	if(strGroupIDs.IsEmpty()) {
		strGroupIDs = "-1";
	}
	else {
		//TES 1/18/2010 - PLID 36895 - If any groups are blocked, we need to NOT hide patients that are in those groups, but which they
		// have been granted "emergency access" to.
		strGroupIDs.TrimRight(",");
		CArray<long,long> arEmergencyPatients;
		GetMainFrame()->GetEmergencyAccessPatients(arEmergencyPatients);
		CString strPatIDs;
		for(int i = 0; i < arEmergencyPatients.GetSize(); i++) {
			strPatIDs += AsString(arEmergencyPatients[i]) + ",";
		}
		if(!strPatIDs.IsEmpty()) {
			strPatIDs.TrimRight(",");
			strExtraWhere = " AND PatientID NOT IN (" + strPatIDs + ") ";
		}
	}

	// (j.jones 2010-05-04 11:04) - PLID 32325 - added the OHIP Health Card Column, only filled if OHIP preference is enabled
	CString strOHIPField = "''";
	CString strOHIPJoin = "";
	if(UseOHIP()) {
		strOHIPField = "OHIPHealthCardQ.TextParam";
		long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
		strOHIPJoin.Format("LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WITH (NOLOCK) WHERE FieldID = %li"
			") AS OHIPHealthCardQ ON PatientsT.PersonID = OHIPHealthCardQ.PersonID ", nHealthNumberCustomField);
	}
	// (j.jones 2010-11-04 16:25) - PLID 39620 - supported this for Alberta as well
	else if(UseAlbertaHLINK()) {		
		strOHIPField = "AlbertaHealthCardQ.TextParam";
		long nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
		strOHIPJoin.Format("LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WITH (NOLOCK) WHERE FieldID = %li"
			") AS AlbertaHealthCardQ ON PatientsT.PersonID = AlbertaHealthCardQ.PersonID ", nHealthNumberCustomField);
	}

	// (j.gruber 2010-10-04 14:31) - PLID 40415 - add security groups to drop down
	CString strGroupsFrom, strGroupsSelect;
	long nGroups = GetRemotePropertyInt("PtTB_Security_Group", -9, 0, GetCurrentUserName(), TRUE);
	if (nGroups > 0) {
		strGroupsSelect = " CASE WHEN CountQ.CountID IS NULL OR CountQ.CountID = 0 THEN '' ELSE CASE WHEN CountQ.CountID = 1 THEN SecurityGroupsT.Name ELSE dbo.GetSecurityGroupList(CountQ.PatientID) END END ";
		strGroupsFrom.Format("LEFT JOIN "
			" 		(SELECT PatientID, Min(SecurityGroupID) as GroupID, Count(*) as CountID FROM SecurityGroupDetailsT WITH (NOLOCK) GROUP BY PatientID) CountQ "
			" 		ON PatientsT.PersonID = CountQ.PatientID  "
			" 		LEFT JOIN SecurityGroupsT ON CountQ.GroupID = SecurityGroupsT.ID ");	
	}
	else {
		strGroupsSelect = " '' ";
	}


	CString strSql;
	// (j.gruber 2011-07-22 16:11) - PLID 44118 - added color
	strSql.Format("(SELECT PatientsT.PersonID, PatientsT.UserDefinedID AS PatientID, PersonT.[Last], PersonT.[First], PersonT.[Middle], "
		"PersonT.SocialSecurity, PersonT.Company, PersonT.BirthDate, PatientsT.CurrentStatus, PersonT.Archived, %s AS OHIPHealthCardNum, %s as SecGroupName, "
		" CASE WHEN PersonT.Archived = 0 THEN CONVERT(int, 0) ELSE CONVERT(int, %li) END as ForeColor "
		"FROM PatientsT WITH (NOLOCK) INNER JOIN PersonT WITH (NOLOCK) ON PatientsT.PersonID = PersonT.ID "
		"%s %s "
		"WHERE PatientsT.PersonID > 0 AND PatientsT.CurrentStatus <> 4) AS ComboQ "		
		"LEFT JOIN (SELECT SecurityGroupDetailsT.PatientID FROM SecurityGroupDetailsT WITH (NOLOCK) "
		"	WHERE SecurityGroupID IN (%s) %s "
		"	GROUP BY SecurityGroupDetailsT.PatientID "
		"	) AS BlockedPatientsQ ON ComboQ.PersonID = BlockedPatientsQ.PatientID", 
		strOHIPField, strGroupsSelect, PATIENT_LIST_INACTIVE_COLOR, strOHIPJoin, strGroupsFrom, strGroupIDs, strExtraWhere);

	return _T(strSql);
}


// (a.walling 2011-03-19 16:18) - PLID 42914 - Exception handler callbacks from NxCatch
namespace NxCatch
{
namespace Practice
{
	int HandleMFCException(CException* e,
		LPCTSTR szMessage, DWORD line, LPCTSTR szFile, DWORD flags, HWND hwndUI)
	{
		if (!flags) {
			return 0;
		}

		return HandleException(e, szMessage, line, szFile, 
				(flags & NxCatch::ePreserve) ? FALSE : TRUE, 
				"", "", 
				NxCatch::IsAsync(flags));
	};

	int HandleCOMException(_com_error& e,
		LPCTSTR szMessage, DWORD line, LPCTSTR szFile, DWORD flags, HWND hwndUI)
	{
		if (!flags) {
			return 0;
		}

		return HandleException(e, szMessage, line, szFile, 
			"", "", 
			NxCatch::IsAsync(flags));
	};
};
};

//TES 8/1/2011 - PLID 44716 - Moved into its own function
//TES 10/13/2011 - PLID 44716 - This function can create a transaction, so if it's being called from someplace that's already inside a 
// transaction, it needs to not do that.
// (j.jones 2012-04-17 17:05) - PLID 13109 - this now returns the new sort order, if needed
long CreateNoteCategory(const CString &strCategory, bool bInTransaction /*= false*/, OUT long *pnNewSortOrder /*= NULL*/)
{	
	CSqlTransaction sqlTran;
	//TES 10/13/2011 - PLID 44716 - Don't start a transaction if we're already in one.
	if(!bInTransaction) {
		sqlTran.Begin();
	}
	// (j.jones 2012-04-17 16:59) - PLID 13109 - we have to assign a new sort order here, so I went ahead
	// and generated the new ID as well in this ad-hoc procedure
	long nNewID = -1;
	_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON "
		"DECLARE @newID INT, @sortOrder INT "
		"SET @newID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM NoteCatsF) "
		"SET @sortOrder = (SELECT COALESCE(MAX(SortOrder), 0) + 1 FROM NoteCatsF) "
		""
		"INSERT INTO NoteCatsF (ID, Description, SortOrder) VALUES (@newID, {STRING}, @sortOrder) "
		"SET NOCOUNT OFF "
		""
		"SELECT @newID AS NewID, @sortOrder AS NewSortOrder", strCategory);
	if(rs->eof) {
		if(!bInTransaction) {
			sqlTran.Rollback();
		}
		ThrowNxException("CreateNoteCategory failed to generate a new ID!");
	}
	else {
		nNewID = VarLong(rs->Fields->Item["NewID"]->Value);
		// (j.jones 2012-04-17 17:08) - PLID 13109 - fill pnNewSortOrder only if requested
		if(pnNewSortOrder) {
			*pnNewSortOrder = VarLong(rs->Fields->Item["NewSortOrder"]->Value);
		}
	}
	rs->Close();

	//TES 8/1/2011 - PLID 44716 - Like EMR Charts, these permissions will default to everyone (that has permission to view History at all)
	// (z.manning 2013-10-18 10:00) - PLID 59082 - Use the global define for description
	long nNewSecurityObjectID = AddUserDefinedPermissionToData(nNewID, sptView, strCategory, PERMISSION_DESC_NOTE_CATEGORY, bioHistoryCategories, sptView);
	//TES 10/13/2011 - PLID 44716 - Don't commit the transaction if we were already in one.
	if(!bInTransaction) {
		sqlTran.Commit();
	}
	//(e.lally 2011-07-08) PLID 44495 - Audit the creation
	AuditEvent(-1, "", BeginNewAuditEvent(), aeiHistoryCategoryCreated, nNewID, "", strCategory, aepMedium, aetCreated);
	AddUserDefinedPermissionToMemory(nNewSecurityObjectID, nNewID, sptView, strCategory, PERMISSION_DESC_NOTE_CATEGORY, bioHistoryCategories, sptView);
	
	return nNewID;
}

//TES 8/2/2011 - PLID 44814 - Similar to PollEmrChartPermissions, gets the IDs of all history categories the current user is permissioned
// to view.  Returns TRUE if they can view all categories
BOOL PollHistoryCategoryPermissions(OUT CArray<long,long> &arynPermissionedCategoryIDs)
{
	extern CArray<CBuiltInObject*,CBuiltInObject*> gc_aryUserDefinedObjects;;

	BOOL bAllCategoriesLicensed = TRUE;
	for(int nBuiltInObjectIndex = 0; nBuiltInObjectIndex < gc_aryUserDefinedObjects.GetSize(); nBuiltInObjectIndex++)
	{
		CBuiltInObject *pBuiltInObject = gc_aryUserDefinedObjects.GetAt(nBuiltInObjectIndex);
		if(pBuiltInObject->m_eParentID == bioHistoryCategories) {			
			if(CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, pBuiltInObject->m_nObjectValue, TRUE)) {
				arynPermissionedCategoryIDs.Add(pBuiltInObject->m_nObjectValue);
			}
			else {
				bAllCategoriesLicensed = FALSE;
			}
		}
	}

	return bAllCategoriesLicensed;
}

//TES 8/2/2011 - PLID 44814 - Returns a boolean SQL statement specifying which categories the current user is permissioned to see.
// So, either "(<field> Is Null OR <field> IN (ID1, ID2))", or "1=1" if they can view all categories
CString GetAllowedCategoryClause(const CString &strField)
{
	if(IsCurrentUserAdministrator()) {
		return "1=1";
	}

	CArray<long,long> arPermissionedCategoryIDs;
	if(PollHistoryCategoryPermissions(arPermissionedCategoryIDs)) {
		//TES 8/2/2011 - PLID 44814 - They can view all categories
		return "1=1";
	}
	else {
		CString strClause;
		strClause.Format("(%s Is Null OR %s < 0 OR %s IN (%s))", strField, strField, strField, ArrayAsString(arPermissionedCategoryIDs, false));
		return strClause;
	}
}

//TES 8/3/2011 - PLID 44814 - Added a parameterized version
CSqlFragment GetAllowedCategoryClause_Param(const CString &strField)
{
	if(IsCurrentUserAdministrator()) {
		return "1=1";
	}

	CArray<long,long> arPermissionedCategoryIDs;
	if(PollHistoryCategoryPermissions(arPermissionedCategoryIDs)) {
		//TES 8/2/2011 - PLID 44814 - They can view all categories
		return "1=1";
	}
	else {
		if(arPermissionedCategoryIDs.GetSize()) {
			CSqlFragment sql(FormatString("(%s Is Null OR %s < 0 OR %s IN ({INTARRAY}))", strField, strField, strField), arPermissionedCategoryIDs);
			return sql;
		}
		else {
			CSqlFragment sql(FormatString("(%s Is Null OR %s < 0)", strField, strField));
			return sql;
		}
	}
}


//TES 8/5/2011 - PLID 44901 - Similar to PollEmrChartPermissions, gets the IDs of all locations (for labs) the current user is permissioned
// to view.  Returns TRUE if they can view all locations
BOOL PollLabLocationPermissions(OUT CArray<long,long> &arynPermissionedLocationIDs)
{
	extern CArray<CBuiltInObject*,CBuiltInObject*> gc_aryUserDefinedObjects;;

	BOOL bAllLocationsLicensed = TRUE;
	for(int nBuiltInObjectIndex = 0; nBuiltInObjectIndex < gc_aryUserDefinedObjects.GetSize(); nBuiltInObjectIndex++)
	{
		CBuiltInObject *pBuiltInObject = gc_aryUserDefinedObjects.GetAt(nBuiltInObjectIndex);
		if(pBuiltInObject->m_eParentID == bioLabLocations) {
			if(CheckCurrentUserPermissions(bioLabLocations, sptView, TRUE, pBuiltInObject->m_nObjectValue, TRUE)) {
				arynPermissionedLocationIDs.Add(pBuiltInObject->m_nObjectValue);
			}
			else {
				bAllLocationsLicensed = FALSE;
			}
		}
	}

	return bAllLocationsLicensed;
}

//TES 8/5/2011 - PLID 44901 - Returns a boolean SQL statement specifying which locations (for labs) the current user is permissioned to see.
// So, either "<field> NOT IN (ID1, ID2)" or "" if they can view all labs
CString GetAllowedLocationClause(const CString &strField)
{
	if(IsCurrentUserAdministrator()) {
		return "1=1";
	}

	CArray<long,long> arPermissionedLocationIDs;
	if(PollLabLocationPermissions(arPermissionedLocationIDs)) {
		//TES 8/5/2011 - PLID 44901 - They can view all locations
		return "1=1";
	}
	else {
		CString strClause;
		strClause.Format("(%s Is Null OR %s < 0 OR %s IN (%s))", strField, strField, strField, ArrayAsString(arPermissionedLocationIDs, false));
		return strClause;
	}
}

//TES 8/5/2011 - PLID 44901 - Added a parameterized version
CSqlFragment GetAllowedLocationClause_Param(const CString &strField)
{
	if(IsCurrentUserAdministrator()) {
		return "1=1";
	}

	CArray<long,long> arPermissionedLocationIDs;
	if(PollLabLocationPermissions(arPermissionedLocationIDs)) {
		//TES 8/5/2011 - PLID 44901 - They can view all locations
		return "1=1";
	}
	else {
		if(arPermissionedLocationIDs.GetSize()) {
			CSqlFragment sql(FormatString("(%s Is Null OR %s < 0 OR %s IN ({INTARRAY}))", strField, strField, strField), arPermissionedLocationIDs);
			return sql;
		}
		else {
			CSqlFragment sql(FormatString("(%s Is Null OR %s < 0)", strField, strField));
			return sql;
		}
	}
}

//(c.copits 2011-09-29) PLID 45626 - Validate Email Addresses
// Returns true if valid email address and false otherwise.
bool IsValidEmailAddress(CString strEmail)
{
	// The most practical way to verify email addresses is to use regular expressions.
	// The regex is from http://www.markussipila.info/pub/emailvalidator.php
	// This should work for the majority of email addresses.
	//(a.costa 2016-07-11) PLID-63343 - Modified the regex from the link above to allow longer domain labels and better hyphen correctness

	try {

		RegEx::IRegExp2Ptr rxEmailAddress(__uuidof(RegEx::RegExp));					
		rxEmailAddress->PutIgnoreCase(VARIANT_TRUE);
		rxEmailAddress->PutGlobal(VARIANT_FALSE);
		rxEmailAddress->PutPattern("^[a-z0-9_\\+]([\\.-]?[a-z0-9_\\+]+){0,63}@[a-z0-9]([\\.-]?[a-z0-9]+){0,63}\\.([a-z]{2,63})$");

		if (VARIANT_TRUE == rxEmailAddress->Test((LPCTSTR)strEmail)) {
			return true;
		}
		else {
			return false;
		}
	} NxCatchAll(__FUNCTION__);

	return true;			// Avoid compiler warning that not all code paths reutrn value
}

// (a.walling 2012-04-11 15:39) - PLID 49594 - Replace ://PRACTICE.EXE with ://PracticeMain.exe etc if we are not actually running as practice.exe
CString FixupPracticeResModuleName(const CString& str)
{
	CiString strModuleName = FileUtils::GetFileName(_pgmptr);
	if (strModuleName == "Practice.exe") {
		return str;
	}

	strModuleName.Insert(0, "://");

	CiString istr = str;
	int replaced = istr.Replace("://practice.exe", strModuleName);
	ASSERT(replaced > 0);

	return istr;
}

// (z.manning 2012-08-31 11:06) - PLID 52413
NexTech_COM::IPatientDashboardGeneratorPtr g_pDashboardGenerator;
NexTech_COM::IPatientDashboardGenerator* GetPatientDashboardGenerator()
{
	if(g_pDashboardGenerator == NULL) {
		g_pDashboardGenerator.CreateInstance(_T("NexTech_COM.PatientDashboardGenerator"));
	}
	if(g_pDashboardGenerator == NULL) {
		AfxThrowNxException("g_pDashboardGenerator could not be initialized");
	}

	return g_pDashboardGenerator;
}

// (j.gruber 2012-10-17 13:13) - PLID 47289
CString GetAffiliateTypeDescription(AffiliatePhysType afpt)
{
	switch (afpt) {
		case afptPreOp:
			return "PreOp";
		break;
		case afptPostOp:
			return "PostOp";
		break;
		case afptPrePostOp:
			return "PreOp & PostOp";
		break;
	}
	return "";
}

// (j.fouts 2012-12-14 10:18) - PLID 54183
// This is used to sort string using alphabetic order for sequences of aplha chars,
// and numerically for sequences of number. This is particularly handing when sorting
// names of things like: page1, page2, page10. And a alphabetic sort would give page1, page10, page2.
// This will return true if strLeft should come before strRight in a sort function
BOOL CompareAlphaNumeric(CString& strLeft, CString& strRight)
{
	long nLeftIndex = 0;
	long nRightIndex = 0;
	long nLeftLength = strLeft.GetLength();
	long nRightLength = strRight.GetLength();

	//We will need to keep a buffer for each to hold our current sequence of alphas or numerics
	char* strLeftBuffer = new char[nLeftLength+1];
	char* strRightBuffer = new char[nLeftLength+1];
	//Clear the buffers to start
	strLeftBuffer[0] = '\0';
	strRightBuffer[0] = '\0';
	long nLeftBufferIndex = 0;
	long nRightBufferIndex = 0;
	
	//This will track if the buffer is currently filled with numbers or characters
	bool isAlphaSequence = false;

	//Continue until one string ends
	while(nLeftIndex < nLeftLength && nRightIndex < nRightLength)
	{
		//Get our next char in the string
		char charLeft = strLeft.GetAt(nLeftIndex);
		char charRight = strRight.GetAt(nRightIndex);

		//Check if they are digits
		if(isdigit(charLeft))
		{
			if(isdigit(charRight))
			{
				//Both are digits, so if our buffer is currently alphas then compare the buffer, clear it, and start a numeric buffer
				if(isAlphaSequence)
				{
					//Switch to a numeric buffer
					isAlphaSequence = false;

					//String compare the alpha sequence and return if they are not equal
					int nCompare = strcmp(strLeftBuffer, strRightBuffer);
					if(nCompare)
					{
						free(strLeftBuffer);
						free(strRightBuffer);
						return nCompare < 0? FALSE : TRUE;
					}

					//The buffers were equal so clear them
					strLeftBuffer[0] = '\0';
					strRightBuffer[0] = '\0';
					nLeftBufferIndex = 0;
					nRightBufferIndex = 0;
				}
			
				//Add the digits to our buffer
				strLeftBuffer[nLeftBufferIndex++] = charLeft;
				strRightBuffer[nRightBufferIndex++] = charRight;
				strLeftBuffer[nLeftBufferIndex] = '\0';
				strRightBuffer[nLeftBufferIndex] = '\0';
				nLeftIndex++;
				nRightIndex++;
			}
			else
			{
				//Left was a digit right was not
				if(isAlphaSequence)
				{
					//If we had an alpha sequence then that means the right had a longer alpha squence
					//So sorting alphabetically left should come before right
					free(strLeftBuffer);
					free(strRightBuffer);
					return TRUE;
				} 
				else
				{
					//If we had an numeric sequence then that means the left had a longer number
					//Therefore a larger number and so left should come after right
					free(strLeftBuffer);
					free(strRightBuffer);
					return FALSE;
				}
			}
		}
		else
		{
			if(isdigit(charRight))
			{
				//Right was a digit left was not
				if(isAlphaSequence)
				{
					//If we had an alpha sequence then that means the left had a longer alpha squence
					//So sorting alphabetically left should come after right
					free(strLeftBuffer);
					free(strRightBuffer);
					return FALSE;
				} 
				else
				{
					//If we had an numeric sequence then that means the right had a longer number
					//Therefore a larger number and so left should come before right
					free(strLeftBuffer);
					free(strRightBuffer);
					return TRUE;
				}
			}
			else
			{
				//Both are alpha characters, so if our buffer is currently numbers then compare the buffer, clear it, and start an alpha buffer
				if(!isAlphaSequence)
				{
					//This is no longer a digit sequence, both are alphas
					isAlphaSequence = true;

					//Compare the two numbers
					long nCompare =  atol(strLeftBuffer) - atol(strRightBuffer);
					if(nCompare)
					{
						//If right is larger than left nCompare will be negitive, therefore return true in
						//this case because we want the left to come before the right
						free(strLeftBuffer);
						free(strRightBuffer);
						return nCompare < 0 ? TRUE : FALSE;
					}

					//Clear out the buffers
					strLeftBuffer[0] = '\0';
					strRightBuffer[0] = '\0';
					nLeftBufferIndex = 0;
					nRightBufferIndex = 0;
				}

				//Add the alphas to our buffer
				strLeftBuffer[nLeftBufferIndex++] = charLeft;
				strRightBuffer[nRightBufferIndex++] = charRight;
				strLeftBuffer[nLeftBufferIndex] = '\0';
				strRightBuffer[nLeftBufferIndex] = '\0';

				nLeftIndex++;
				nRightIndex++;
			}
		}
	}

	//Compare the final buffer
	if(isAlphaSequence)
	{
		//String compare the alpha sequence and return the sort if they are not equal
		int nCompare = strcmp(strLeftBuffer, strRightBuffer);
		if(nCompare)
		{
			free(strLeftBuffer);
			free(strRightBuffer);
			return nCompare < 0 ? FALSE : TRUE;
		}
	}
	else
	{
		long nCompare =  atol(strLeftBuffer) - atol(strRightBuffer);
		if(nCompare)
		{
			free(strLeftBuffer);
			free(strRightBuffer);
			return nCompare < 0 ? TRUE : FALSE;
		}
	}

	//Up until the end of one string we are the exactly same
	if(nLeftLength < nRightLength)
	{
		//If the right is larger then we want left first
		free(strLeftBuffer);
		free(strRightBuffer);
		return TRUE;
	} 
	else if(nLeftLength > nRightLength)
	{
		//If the left is larger then we want left second
		free(strLeftBuffer);
		free(strRightBuffer);
		return FALSE;
	}

	free(strLeftBuffer);
	free(strRightBuffer);
	return TRUE;
}

//(r.farnworth 2013-01-25) PLID 54667 - Moved from PrescriptionEditDlg
bool CheckPrescriptionTemplates(CString &strTemplateName, CString &strTemplatePath, HWND parent)
{
	// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
	// based on how many prescriptions are printed
	BOOL bExactCountFound = FALSE;
	BOOL bOtherCountFound = FALSE;
	strTemplateName = GetDefaultPrescriptionTemplateByCount(1, bExactCountFound, bOtherCountFound);

	if(strTemplateName.IsEmpty()) {
		MsgBox(MB_OK|MB_ICONINFORMATION, 
			"There is no default template set up. The prescription cannot be printed.\n"
			"Please go to the Medications tab to set up your default prescription template.");
		return false;
	}
	//if no template was found for the exact count, and there are some for other counts,
	//ask if they wish to continue or not (will use the standard default otherwise)
	else if(!bExactCountFound && bOtherCountFound) {
		if(IDNO == MessageBox(parent, "There is no default template configured for use with one prescription, "
			"but there are templates configured for other counts of prescriptions.\n\n"
			"Would you like to continue merging using the standard prescription template?",
			"Practice", MB_ICONQUESTION|MB_YESNO)) {

			AfxMessageBox("The prescription will not be be printed.\n"
				"Please go to the Medications tab to set up your default prescription template.");
			return false;
		}
	}

	strTemplatePath = GetTemplatePath("Forms", strTemplateName);
	// (j.fouts 2013-04-23 10:53) - PLID 52907 - Removed warning here
	return true;
}


// (b.spivey, April 22, 2013) - PLID 55943 - Modify steps and call a single step mod function within a loop to 
//	 complete steps down the ladder if requirements are met. 
// (c.haag 2013-09-05) - PLID 58216 - bSavingPatientLab indicates whether the user just saved lab results (potentially signing them)
bool GlobalModifyStepCompletion(long nLabID, long nStepID, bool bComplete, bool bSavingPatientLabResults) 
{
	//Grab the steps. 
	_RecordsetPtr prs = CreateParamRecordset(
		"	SELECT TOP 1 LST.StepID, LST.StepCompletedBy, LST.StepCompletedDate, LPST.CreateLadder, LT.PatientID  "
			"	FROM LabStepsT LST "
			"	INNER JOIN LabProcedureStepsT LPST ON LST.LabProcedureStepID = LPST.StepID "
			"	INNER JOIN LabsT LT ON LST.LabID = LT.ID "
			"	WHERE LST.LabID = {INT} "
			"	 AND (LST.StepCompletedBy = '' OR LST.StepCompletedBy IS NULL) "
			"	 AND (LST.StepCompletedDate = '' OR LST.StepCompletedDate IS NULL) "
			"	ORDER BY LST.StepOrder "
			"	"
			"	"
			"	SELECT LST.StepID, LST.StepCompletedBy, LST.StepCompletedDate, LPST.CreateLadder, LT.PatientID  "
			"	FROM LabStepsT LST "
			"	INNER JOIN LabProcedureStepsT LPST ON LST.LabProcedureStepID = LPST.StepID "
			"	INNER JOIN LabsT LT ON LST.LabID = LT.ID "
			"	WHERE LST.LabID = {INT} "
			"	ORDER BY LST.StepOrder ", nLabID, nLabID);

	//No steps to grab.
	if (prs->eof) {
		return false;
	}

	long nActiveStepID = AdoFldLong(prs->Fields, "StepID", -1);

	prs = prs->NextRecordset(NULL);

	//No steps to grab. 
	if(prs->eof) {
		return false; 
	}

	long nCurrentStepID = -1;
	CString strPatientName = ""; 
	long nPatientID = -1; 
	bool bFoundCurrentStep = false;

	//Loop. 
	while (!prs->eof) {

		nCurrentStepID = AdoFldLong(prs->Fields, "StepID", -1);
		nPatientID = AdoFldLong(prs->Fields, "PatientID", -1); 
		bool bCreateLadder = !!AdoFldBool(prs->Fields, "CreateLadder", false);

		//Check if we're not on the current step and haven't found the current step. 
		if(nStepID != nCurrentStepID && !bFoundCurrentStep) {
			//This is our "skip" case. 
			prs->MoveNext(); 
			continue; 
		}
		//Check if we're on the current step and haven't found the current step. 
		else if (nStepID == nCurrentStepID && !bFoundCurrentStep) {
			//This is our "single step" case-- this makes sure we will complete the step we were told to complete as if 
			//	 only one step was able to be completed. 
			bFoundCurrentStep = true; 
			GlobalModifySingleStepCompletion(nCurrentStepID, nLabID, bComplete, nPatientID, bSavingPatientLabResults);
			if (nActiveStepID != nStepID) break; 
			if (!bComplete) break; 
			prs->MoveNext(); 
			continue; 
		}
		//If our step is supposed to create a ladder we ask if we're to create a ladder. If so, complete the step. 
		if(bCreateLadder) {
			if(PhaseTracking::PromptToAddPtnProcedures(nPatientID)) { 
				GlobalModifySingleStepCompletion(nCurrentStepID, nLabID, bComplete, nPatientID, bSavingPatientLabResults);
				prs->MoveNext(); 
				continue; 
			}
		}

		//For us to reach this point, we had to have been on a step that could not be completed given the parameters. Break out. 
		break; 
	} 
	return true; 
}

// (c.haag 2013-08-29) - PLID 58216 - Given a lab and a lab step, this will return the collection of related lab steps in other
// specimens that could finish if the given lab step has finished.
_RecordsetPtr GetCompletableLinkedSteps(long nStepID, bool bIgnoreStepsCompletedBySigning, ADODB::_ConnectionPtr lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) {
		pCon = lpCon;
	}
	else {
		pCon = GetRemoteData();
	}

	// (m.hancock 2006-07-07 11:16) - Added LabID to selection to assemble a list of LabIDs for use with MarkEntireLabComplete
	// (m.hancock 2006-07-07 16:04) - Added exclusion of closed labs so we do not attempt to modify labs that have already been closed
	// (m.hancock 2006-07-13 16:53) - Changed this so it only looks for linked steps if you're completing a step
	// (m.hancock 2006-07-26 13:01) - Changed query so that it only looks at the current patient's records.
	//   Also added query results for step name and lab description so a more descriptive message about linked steps is shown.
	// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
	// (z.manning 2008-10-30 15:15) - PLID 31864 - Use to be ordered instead of anatomic location for
	// non-biopsy labs
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/8/2009 - PLID 36470 - Restored AnatomySide
	// (z.manning 2010-04-30 16:00) - PLID 37553 - Use a view to pull anatomic location
	// (c.haag 2010-12-15 17:27) - PLID 41825 - New logic for finding incomplete labs
	//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
	// (c.haag 2013-08-29) - PLID 58216 - Ignore steps that would be completed by signing results. Those
	// should not be completed if a related step is completed because it misleads users into thinking something happened that
	// really did not happen.
	return CreateParamRecordset(pCon,
		FormatString( // FormatStrings inside CreateRecordsets are awesome!
			"SELECT LinkedLabSteps.StepID, LinkedLabSteps.LabID, LinkedLabSteps.Name AS StepName, "
			"(LinkedLabs.FormNumberTextID + '-' + "
			"	CASE WHEN LinkedLabs.Specimen IS NULL THEN '' ELSE LinkedLabs.Specimen END + ' - ' + "
			"	CASE WHEN LinkedLabs.Type = {INT} THEN "
			"		LinkedLabAnatomicLocationQ.AnatomicLocation \r\n"
			"	ELSE LinkedLabs.ToBeOrdered END "
			"	) AS LabDescription "
			"FROM LabStepsT "
			"INNER JOIN LabsT ON LabStepsT.LabID = LabsT.ID "
			"INNER JOIN LabsT LinkedLabs ON "
			"	LinkedLabs.PatientID = LabsT.PatientID "
			"	AND LinkedLabs.ID <> LabsT.ID "
			"	AND LinkedLabs.Deleted = CONVERT(BIT,0) "
			"	AND LinkedLabs.FormNumberTextID = LabsT.FormNumberTextID "
			"	AND ( LinkedLabs.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LinkedLabs.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL) ) "
			"INNER JOIN LabStepsT LinkedLabSteps ON "
			"	LinkedLabSteps.LabID = LinkedLabs.ID "
			"	AND LinkedLabSteps.LabProcedureStepID = LabStepsT.LabProcedureStepID "
			"	AND LinkedLabSteps.StepCompletedDate IS NULL "
			"LEFT JOIN LabAnatomicLocationQ LinkedLabAnatomicLocationQ ON LinkedLabAnatomicLocationQ.LabID = LinkedLabs.ID "
			"LEFT JOIN LabProcedureStepsT LinkedLabProcedureSteps ON LinkedLabProcedureSteps.StepID = LinkedLabSteps.LabProcedureStepID "
			"WHERE LabStepsT.StepID = {INT} "
			"%s "
			,bIgnoreStepsCompletedBySigning ? "AND (LinkedLabProcedureSteps.StepID IS NULL OR LinkedLabProcedureSteps.CompletedBySigning = 0)" : "")
		,ltBiopsy, nStepID);
}

// (b.spivey, April 22, 2013) - PLID 55943 - Modify single steps. A lot of this code came from labs, so I left the code comments in there. 
// (c.haag 2013-09-05) - PLID 58216 - bSavingPatientLab indicates whether the user just saved lab results (potentially signing them)
bool GlobalModifySingleStepCompletion(long nStepID, long nLabID, bool bComplete, long nPatientID, bool bSavingPatientLab)
{
	// (m.hancock 2006-06-22 09:43) - PLID 21154 - Provide a method for manipulating steps associated with labs.
	//Get the currently selected row

	//Make sure we actually have a row selected
	if(nStepID > 0) {
		// (b.spivey, April 22, 2013) - PLID 55943 - Special handling, there is a good chance 
		//	 that our starting step may be either marked complete and already completed, or marked incomplete 
		//	 and not complete already
		if(!bComplete && 
			ReturnsRecordsParam("SELECT TOP 1* FROM LabStepsT "
				"WHERE StepCompletedDate IS NULL "
				"	AND StepCompletedBy IS NULL "
				"	AND StepID = {INT} ", nStepID)) {
					return false; 

		}
		else if (bComplete && 
			ReturnsRecordsParam("SELECT TOP 1* FROM LabStepsT "
				"WHERE (StepCompletedDate IS NOT NULL "
				"	OR StepCompletedBy IS NOT NULL) "
				"	AND StepID = {INT} ", nStepID)) {
					return false; 

		}

		//Get required data for marking the step complete
		COleDateTime dtMarkedDone = COleDateTime::GetCurrentTime();
		CArray<long, long> aryUpdateSteps; //Stores the list of steps that will be updated
		aryUpdateSteps.Add(nStepID);
		CArray<long, long> aryUpdateLabs;  //Stores the list of labs that will be updated by MarkEntireLabComplete
		aryUpdateLabs.Add(nLabID);

		CString strPatientName = GetExistingPatientName(nPatientID); 
		

		//Declare a null variant for later use
		_variant_t varNull;
		varNull.vt = VT_NULL;

		//Check if there are other steps for specimens having this same form number
		// (c.haag 2013-08-29) - PLID 58216 - We now invoke this from a utility function
		if(bComplete) {
		
			//Parameterized. 
			_RecordsetPtr prsLinkedSteps = GetCompletableLinkedSteps(nStepID, bSavingPatientLab);

			//If we have results from the query, ask the user if they wish to also update the other 
			//specimens with this same form number
			if(!prsLinkedSteps->eof) {
				
				// (m.hancock 2006-07-26 13:21) - PLID 21154 - Generate a descriptive message about which steps are linked
				CString strMessage = "There are other specimens for this patient with the same form number.\n"
					"Would you like to also update the following corresponding incomplete steps for this form number?\n";
				while(!prsLinkedSteps->eof) {
					//Get the Lab description and step name for the steps that would be updated
					CString strStepName = AdoFldString(prsLinkedSteps, "StepName", "");
					CString strLabDescription = AdoFldString(prsLinkedSteps, "LabDescription", "");
					//Add to the message
					strMessage += "\n- " + strLabDescription + " : " + strStepName;						
					//Move to the next record
					prsLinkedSteps->MoveNext();
				}

				if(IDYES == AfxMessageBox(strMessage, MB_ICONQUESTION|MB_YESNO)) {
					//Go back to the first record in the query
					prsLinkedSteps->MoveFirst();
					//Loop through the query results and add the step IDs to the list to be updated
					while(!prsLinkedSteps->eof) {

						aryUpdateSteps.Add(AdoFldLong(prsLinkedSteps, "StepID"));
						aryUpdateLabs.Add(AdoFldLong(prsLinkedSteps, "LabID"));
						//Move to the next record
						prsLinkedSteps->MoveNext();
					}
				}
			}
		}
		
		
		if(bComplete) { //Complete the step
			// (z.manning 2008-10-21 11:08) - PLID 31371 - Also check for the next incomplete step.
			//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
			
			//Parameterized. 
			ExecuteParamSql(
				"SET NOCOUNT ON "
				"UPDATE LabStepsT SET StepCompletedDate = {STRING}, StepCompletedBy = {INT} WHERE StepID IN ({INTARRAY})"
				"SET NOCOUNT OFF "
				, FormatDateTimeForSql(dtMarkedDone), GetCurrentUserID(), aryUpdateSteps, nLabID);
		}
		else { //Mark the step not complete
			ExecuteParamSql("UPDATE LabStepsT SET StepCompletedDate = NULL, StepCompletedBy = NULL WHERE StepID IN ({INTARRAY})",
				aryUpdateSteps);
		}

		// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a LabsT-level table checker because we updated lab steps.
		// I think they can span more than one lab.
		// (r.gonet 08/25/2014) - PLID 63221 - Send a table checker with the patient ID so it doesn't cause refreshes on
		// other patients.
		CClient::RefreshLabsTable(nPatientID, -1);

		// (z.manning 2008-10-13 16:37) - PLID 31667 - Update todos for this lab
		//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
		//TES 6/23/2014 - PLID 60708 - Update all the labs that were modified
		foreach(long nUpdatedLabID, aryUpdateLabs) {
			SyncTodoWithLab(nUpdatedLabID, nPatientID);
		}

		//Auditing
		// (m.hancock 2006-07-24 09:40) - PLID 21582 - Make sure that modifying steps is audited
		{
			//Need to loop through all IDs in our list, comma delimited.  Must be at least 1.
			long nAuditID = BeginNewAuditEvent();

			for(int i = 0; i < aryUpdateSteps.GetCount(); i++) {
				
				long nRecordID = aryUpdateSteps.GetAt(i); 
				// (m.hancock 2006-07-24 09:53) - PLID 21582 - Generate a string containing the lab's description and the step name
				CString strOld = GenerateStepCompletionAuditOld(nRecordID);
				//Generate the new value string
				CString strNew = bComplete ? "<Completed>" : "<Incomplete>";
				// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiPatientLabStepMarkedComplete, nRecordID, strOld, strNew, aepMedium, aetChanged);
			}
		}
	}

	return true; 
}


//(r.wilson 4/8/2013) pl 56117 - Function to remove zeros from the front and the back of a number
//								 ex: passing the string 0002305.0200  would return 2305.02 from the function
CString RemoveInsignificantZeros(CString strText)
{
	//(r.wilson 4/8/2013) pl 56117 - Hold onto a backup of the original string
	CString strTextBackup;
	strTextBackup = strText;

	//(r.wilson 4/8/2013) pl 56117 - Trim Just in case 
	strText.Trim();

	//(r.wilson 4/8/2013) pl 56117 - Attempt to pull out a valid number from string
	char* str_pBadString;
	double dExtractedValue = strtod(strText, &str_pBadString);

	if(str_pBadString[0] != '\0' || dExtractedValue < 0)
	{
		return strTextBackup;
	}
	
	//(r.wilson 4/8/2013) pl 56117 - For our substring at the end of the function we will start cutting characters to use at this index
	int nSubStringLowerBoundIndex = 0;
	//(r.wilson 4/8/2013) pl 56117 - .. and go forward [nSubStringLength] places in order to define the end of the substring we will cut out and return
	int nSubStringLength = strText.GetLength();


	//Variable to hold the final returned string
	CString strOutputText;

	if(strText.Find('.') > -1)
	{
		//(r.wilson 4/8/2013) pl 56117 - Iterate the string backwards and decrement the UpperBound Index until we encounter a non zero character
		for(int i = strText.GetLength() - 1; i > 0; i--)
		{
			if(i > -1 && strText.GetAt(i) == '0')
			{
				nSubStringLength--;
			}
			else
			{
				//(r.wilson 4/8/2013) pl 56117 - Not 0 then stop searching in this direction
				break;
			}		
		}
	}


	//(r.wilson 4/8/2013) pl 56117 - Iterate through the string from the beginning and decrement the LowerBound Index until we encounter a non zero character
	for(int i = 0 ; i < strText.GetLength(); i++)
	{
		if(strText.GetAt(i) == '0')
		{
			if( (i+1) < strText.GetLength() && strText.GetAt(i+1) != '.')
			{
				//(r.wilson 4/8/2013) pl 56117 - For the Mid(...) function the starting point moves up an index
				nSubStringLowerBoundIndex++;									
				//(r.wilson 4/8/2013) pl 56117 - .. and the size of the string we will grab will now be smaller as well
				nSubStringLength--;
			}
		}
		else
		{
			//(r.wilson 4/8/2013) pl 56117 - Not 0 then stop searching in this direction
			break;
		}
	}

	//(r.wilson 4/8/2013) pl 56117 - Grab the final substring starting at [nSubStringLowerBoundIndex] and goign forward [nSubStringLength] characters
	strOutputText = strText.Mid(nSubStringLowerBoundIndex,nSubStringLength);

	//(r.wilson 4/8/2013) pl 56117 - If the last character is a . (period) then cut it off
	if(strOutputText.GetLength() > 1 && strOutputText.GetAt(strOutputText.GetLength()-1) == '.')
	{
		strOutputText = CString(strOutputText).Mid(0,strOutputText.GetLength() - 1);
	}	

	return strOutputText;
	
}

//(r.wilson 4/8/2013) pl 56117 - function takes in a control id, pulls the text from it, trims the spaces on both ends, then updates the control's text with the trimmed version
void TrimSpaces(CDialog *pDlg, int nControlId )
{
	if(pDlg == NULL)
	{
		return;
	}
	CString strText;	
	if(pDlg->GetDlgItemText(nControlId, strText) != NULL )
	{
		strText.Trim();
		pDlg->SetDlgItemText(nControlId, strText);
	}
}

//(r.wilson 3/6/2013) pl 55478 - 
CString FormatDecimalText(CString &strOriginalText, int nMaxDigitsAllowed)
{
	//(r.wilson 3/6/2013) pl 55478 - If there is not One and only One decimal in the textbox then the formatting is none of this functions business
	//							   - Also we should do nothing  if there are any letters in the textbox
	if(GetNumberOfDecimalsInString(strOriginalText) != 1 || DoesStringContainLetters(strOriginalText) == TRUE)
	{
		return strOriginalText;
	}

	CString strText = strOriginalText;
	strText.Trim();
	
	CString strToken, strLeftSideOfDecimal, strRightSideOfDecimal, strFinalOutput;
	int nCurrentPosition = 0;

	//(r.wilson 3/6/2013) pl 55478 - If the first character is a decimal character and the length of the string is bigger then one then add a 0 to the front 
	if(strText.GetAt(0) == '.' && strText.GetLength() > 1){		
		strFinalOutput = "0" + strText;		
	}
	//(r.wilson 3/6/2013) pl 55478 - If the user simply puts in a decimal character alone then we will replace it with a blank
	else if(strText.GetAt(0) == '.' && strText.GetLength() == 1){
		strFinalOutput = "";
	}
	//(r.wilson 3/6/2013) pl 55478 - If a decimal is the last character in the string then we can cut it off
	else if(strText.GetAt(strText.GetLength() - 1) == '.')
	{
		strFinalOutput = strText.TrimRight('.');
	}
	else
	{		
		//(r.wilson 3/6/2013) pl 55478 - Else means nothing needs to be changed
		strFinalOutput = strOriginalText;
	}		
	
	//If the length of the string is longer than 11 then return the value that was passed in back
	if(strFinalOutput.GetLength() > 11)
	{
		return strOriginalText;
	}

	// If all goes well then return the new string 
	return strFinalOutput;

}

//(r.wilson 3/6/2013) pl 55478 -
BOOL DoesStringContainLetters(CString &strText)
{
	for(int i = 0 ; i < strText.GetLength(); i++)
	{		
		if(isalpha(strText.GetAt(i)))
			return TRUE;
	}

	return FALSE;
}

//(r.wilson 3/6/2013) pl 55478 - Find the number of periods in a given string
int GetNumberOfDecimalsInString(CString &strNumber)
{
	int nDecimalCounter = 0;
	for(int i = 0; i < strNumber.GetLength(); i++){
		if(strNumber.GetAt(i) == '.'){
			nDecimalCounter++;
		}		
	}

	return nDecimalCounter;
}

// (a.walling 2013-06-18 11:44) - PLID 57204 - Extracts comma-separated strings and outputs them sorted ascending with no duplicates
CString SortAndRemoveDuplicatesFromCSV(const CString& csvStr)
{
	typedef boost::tokenizer<boost::escaped_list_separator<char>, boost::string_ref::const_iterator> string_ref_csv_tokenizer;

	boost::string_ref basestr(csvStr, csvStr.GetLength());
	string_ref_csv_tokenizer tok(basestr);	
	std::set<std::string> elements(tok.begin(), tok.end());

	CString str;
	str.Preallocate(csvStr.GetLength());
	for each (const std::string& element in elements) {
		str.Append(element.c_str(), element.length());
		str.Append(", ");
	}
	str.TrimRight(", ");

	return str;
}

// (a.walling 2013-06-20 13:49) - PLID 57204 - Extracts delimited substrings from the string and outputs them sorted ascending with no duplicates
CString SortAndRemoveDuplicates(const CString& delimStr, const char* delimiters)
{
	typedef boost::tokenizer<boost::char_separator<char>, boost::string_ref::const_iterator> string_ref_delimited_tokenizer;

	boost::string_ref basestr(delimStr, delimStr.GetLength());

	boost::char_separator<char> sep(delimiters);
	string_ref_delimited_tokenizer tok(basestr, sep);
	std::set<std::string> elements(tok.begin(), tok.end());

	CString str;
	str.Preallocate(delimStr.GetLength());
	for each (const std::string& element in elements) {
		str.Append(element.c_str(), element.length());
		str.Append(", ");
	}
	str.TrimRight(", ");

	return str;
}

//Creates a new diagnosis code to the DiagCodes table.  Returns the ID of the code that is created.
//	This is created to clean up several legacy places that all re-wrote the same SQL query, and move them all to the
//	API at the same time.
//	For legacy consistency, we trim the code to 50 chars and description to 255, no matter what is passed in.
//	All codes created via this method will be assumed to be non-ICD10.  At present you can only get an ICD-10 code by using
//	the search functionality or future import.
// (j.jones 2014-03-04 09:18) - PLID 61136 - added PCS bit
// (d.thompson 2014-03-06) - PLID 60716 - Note that the API now sends table checkers for you.
// (j.armen 2014-03-25 14:21) - PLID 61517 - Add flag for ICD10 codes
long CreateNewAdminDiagnosisCode(CString strCode, CString strDescription, bool IsAMA /*= false*/, bool bPCS /*= false*/, bool bICD10 /*= false*/)
{
	//Legacy:  Several places trimmed this to 255 regardless of it's length.  That's the current DB max
	strDescription = strDescription.Left(255);
	//Same for 50 char code
	strCode = strCode.Left(50);

	//Create our code object to pass in
	Nx::SafeArray<IUnknown *> aryCommits;
	NexTech_Accessor::_DiagnosisCodeCommitPtr pCode(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
	pCode->Code = _bstr_t(strCode);
	pCode->description = _bstr_t(strDescription);
	pCode->ICD10 = bICD10;
	//IsAMA flag
	NexTech_Accessor::_NullableBoolPtr pIsAMA(__uuidof(NexTech_Accessor::NullableBool));
	pIsAMA->SetBool(IsAMA ? VARIANT_TRUE : VARIANT_FALSE);
	pCode->IsAMA = pIsAMA;
	// (j.jones 2014-03-04 09:18) - PLID 61136 - added PCS bit
	pCode->PCS = bPCS;
	aryCommits.Add(pCode);

	//Now create the code via the API
	NexTech_Accessor::_DiagnosisCodesPtr pCodesCreated = GetAPI()->CreateDiagnosisCodes(GetAPISubkey(), GetAPILoginToken(), aryCommits);

	//Safety check:  We should have gotten some codes back, we sent some in to be created.  Tell the user to try again.
	if(pCodesCreated->Codes == NULL) {
		ThrowNxException("CreateNewAdminDiagnosisCode:  Did not get any results for code creation.  Please attempt to add your code again.");
	}

	Nx::SafeArray<IUnknown *> aryResults = pCodesCreated->Codes;

	//Safety check:  Ensure we got the same # of results as we passed in requests.  Not sure why this would happen, but the user
	//	can search again to try again.  We're not sure what was saved.
	if(aryCommits.GetCount() != aryResults.GetCount()) {
		ThrowNxException("CreateNewAdminDiagnosisCode:  Did not get results for all diagnosis commits.  Please attempt to add your code again.");
	}

	//There's always 1 result, return the ID back to the caller
	NexTech_Accessor::_DiagnosisCodePtr pResult = aryResults.GetAt(0);
	return atoi(VarString(pResult->ID));
}

// (b.savon 2015-03-19 08:33) - PLID 65248 - Add a utility to perform a database backup
void PerformManualDatabaseBackup(HWND hwndParent)
{
	//Registry Settings for backup path
	CString strSubkey = GetSubRegistryKey();
	CString strRegistryBase = GetRegistryBase();

	CString strSharedPath = NxRegUtils::ReadString((strRegistryBase + "\\SharedPath"));
	if (strSharedPath.Right(1) != "\\")
		strSharedPath += "\\";

	CString strBackupPath = strSharedPath + "Data\\NxBackup\\Manual Backups"; //Default backup path + \Manual Backups

	//Create database to backup file name
	CString strTimestamp = GetRemoteServerTime().Format("%Y-%m-%d_%H-%M-%S");
	CString strDatabaseToBackup = GetCurrentDatabaseName() + "_" + strTimestamp + ".zip?0";

	//Construct packet
	CString strPacket;
	strPacket.Format("%s|%s|%s|%s|%s|%s|", strBackupPath, strDatabaseToBackup, "", "", "", "");

	DWORD dwPacketSize = (sizeof(_PACKET_BACKUP_MANUAL_LOCATIONS) + strPacket.GetLength() + 1 /*null terminator*/);

	//Create the packet pointer to a byte array of the new packet type
	_PACKET_BACKUP_MANUAL_LOCATIONS* pPkt = (_PACKET_BACKUP_MANUAL_LOCATIONS*) new BYTE[dwPacketSize];

	// Copy in the packet string plus a null terminator
	memcpy(pPkt->szData, strPacket, strPacket.GetLength() + 1);
	//Set the flags
	pPkt->dwFlags = BACKUP_FLAG_SERVER | 0 /* Force Overwrite */ | 0 /* Workstation Only */ | BACKUP_FLAG_MANUAL_LOCATIONS;
	//Send using our existing connection
	NxBackupUtils::BackupToManualLocations(hwndParent, GetMainFrame()->GetNxServerSocket(), pPkt);
	//Return to here after our backups are done
	//Cleanup our memory allocation
	delete[]((BYTE*)pPkt);
	NxSocketUtils::Send(GetMainFrame()->GetNxServerSocket(), PACKET_TYPE_BKP_ERROR_TEXT, NULL, 0); // Get the error messages
}

// (a.walling 2010-07-29 09:58) - PLID 39871 - Moved GetLicenseAgreementAcceptedName to CLicense
// (z.manning 2015-05-19 16:33) - PLID 65971 - Moved this to global utils
CString GetLicenseAgreementAcceptedPropertyName()
{
	CString strPropName;
	strPropName.Format("LicenseAgreementAccepted_%02i.%02i.%04i.%04i", PRODUCT_VERSION_VAL);
	return strPropName;
}

// (j.jones 2015-03-23 13:45) - PLID 65281 - moved from DiagSearchConfig::SetMinDropdownWidth
//estimates the current dropdown with, forces it to be larger if it's too small
void SetMinDatalistDropdownWidth(NXDATALIST2Lib::_DNxDataListPtr pDataList, long nMinWidth)
{
	if (nMinWidth <= 0) {
		//do nothing
		return;
	}

	long nCurDropDownWidth = pDataList->GetDropDownWidth();
	if (nCurDropDownWidth == -1) {
		//try to get the pixel size of the control
		IOleObjectPtr pole = pDataList;
		if (pole) {
			SIZEL size;
			HRESULT hr = pole->GetExtent(DVASPECT_CONTENT, &size);
			if (hr == S_OK) {
				// We have himetric units, we need screen pixels, so use the screen device context to convert
				HDC dc = ::GetDC(NULL);
				// himetric units are 0.01'ths of a millimeter. there are 25.4 millimeters in an inch, so there are 2540 
				// himetric units per inch. there are LOGPIXELS pixels per inch so there are 2540/LOGPIXELS himetric units 
				// per pixel. therefore, if we have x himetric units, we have (x*LOGPIXELS)/2540 pixels.
				size.cx = MulDiv(size.cx, GetDeviceCaps(dc, LOGPIXELSX), 2540);
				size.cy = MulDiv(size.cy, GetDeviceCaps(dc, LOGPIXELSY), 2540);
				::ReleaseDC(NULL, dc);

				nCurDropDownWidth = size.cx;
			}
		}
	}

	if (nCurDropDownWidth < nMinWidth) {
		pDataList->PutDropDownWidth(nMinWidth);
	}
}

// (z.manning 2015-08-10 14:13) - PLID 67221
NexTech_COM::IICCPSessionManager* GetICCPDevice()
{
	if (GetMainFrame() != NULL && GetMainFrame()->m_pICCPDeviceManager != NULL) {
		return GetMainFrame()->m_pICCPDeviceManager->GetDevice();
	}
	else {
		return NULL;
	}
}

// (v.maida 2016-02-01 09:06) - PLID 68033 - Checks if there are any users that have the default FFA location preference set to nLocationID.
BOOL UsersHaveLocationAsDefaultFFALocationPref(long nLocationID)
{
	return ReturnsRecordsParam("SELECT Username FROM ConfigRT WHERE Name = 'FFA_DefaultLocationID' AND IntParam = {INT}", nLocationID);
}

// (v.maida 2016-02-01 09:06) - PLID 68033 - Sets the default FFA location preference back to its default value for all users who have nLocationID
// as their current preference.
void RevertDefaultFFALocationPrefForLocation(long nLocationID)
{
	ExecuteParamSql(
	"UPDATE ConfigRT "
	"SET IntParam = -100 "
	"WHERE Name = 'FFA_DefaultLocationID' "
	"AND UserName IN (SELECT Username FROM ConfigRT WHERE Name = 'FFA_DefaultLocationID' AND IntParam = {INT})", nLocationID);
}

// (v.maida 2016-02-01 09:06) - PLID 68035 - Checks if there are any users that have the default FFA resource preference set to a resource in vecResourceIDs.
BOOL UsersHaveResourcesAsDefaultFFAResourcePref(std::vector<long> vecResourceIDs)
{
	return ReturnsRecordsParam("SELECT Username FROM ConfigRT WHERE Name = 'FFA_DefaultResourceID' AND IntParam IN ({INTVECTOR})", vecResourceIDs);
}

// (v.maida 2016-02-01 09:06) - PLID 68035 - Sets the default FFA resource preference back to its default value for all users who have a resource in 
// vecResourceIDs as their current preference.
void RevertDefaultFFAResourcePrefForResources(std::vector<long> vecResourceIDs)
{
	ExecuteParamSql(
		"UPDATE ConfigRT "
		"SET IntParam = -201 "
		"WHERE Name = 'FFA_DefaultResourceID' "
		"AND UserName IN (SELECT Username FROM ConfigRT WHERE Name = 'FFA_DefaultResourceID' AND IntParam IN ({INTVECTOR}))", vecResourceIDs);
}

// (z.manning 2016-02-17 12:39) - PLID 68223 - Returns the preference for which word processor to use
WordProcessorType GetWordProcessorType()
{
	// (z.manning 2016-03-03 10:24) - NX-100500 - Added the preference
	long nType = GetRemotePropertyInt("WordProcessorType", (long)WordProcessorType::MSWord, 0);
	switch (nType)
	{
	case WordProcessorType::VTSCMSWord:
		return WordProcessorType::VTSCMSWord;
		break;

	case WordProcessorType::MSWord:
	default:
		return WordProcessorType::MSWord;
		break;
	}
}


// (j.jones 2016-03-08 13:50) - PLID 68478 - the ConfigRT value of LastLicenseUpdateAttempt
// is now encrypted to prevent tampering, this function will get the real datetime, might be g_cdtSqlZero
COleDateTime GetLastLicenseUpdateAttempt()
{
	//this is encrypted to prevent tampering
	_variant_t varLastAttempt = GetRemotePropertyImage("LastLicenseUpdateAttemptImage", 0, "<None>", true);
	if (varLastAttempt.vt != VT_NULL && varLastAttempt.vt != VT_EMPTY) {
		CString strLastAttempt = DecryptStringFromVariant(varLastAttempt);
		COleDateTime dt;
		//we forced writing the date as US M/D/Y, need to force the parse to return as US style as well
		if (dt.ParseDateTime(strLastAttempt, 0, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)) && dt.GetStatus() == COleDateTime::valid) {
			//this is a valid date, return it
			return dt;
		}
	}

	//we have no valid date, so return an old date to indicate no recent attempt has been made
	return g_cdtSqlZero;
}

// (j.jones 2016-03-08 13:50) - PLID 68478 - the ConfigRT value of LastLicenseUpdateAttempt
// is now encrypted to prevent tampering, this function will update the encrypted date to
//be the current time
void UpdateLastLicenseUpdateAttemptToCurrentTime()
{
	COleDateTime dtCurrentTime = COleDateTime::GetCurrentTime();
	//force writing as US M/D/Y, later the reader will force parsing as US M/D/Y
	CString strCurrentTime = dtCurrentTime.Format("%m/%d/%Y %H:%M:%S");
	_variant_t varEncryptedTime = EncryptStringToVariant(strCurrentTime);
	SetRemotePropertyImage("LastLicenseUpdateAttemptImage", varEncryptedTime, 0, "<None>");
}

// (v.maida 2016-05-19 16:54) - NX-100684 - If they're using Azure RemoteApp, then they should be saving to their shared path, otherwise they can
// save to their practice path.
CString GetEnvironmentDirectory()
{
	if (g_pLicense->GetAzureRemoteApp()) {
		return GetSharedPath() ^ "NexTech" ^ g_propManager.GetSystemName();
	}
	else {
		return GetPracPath(PracPath::PracticePath);
	}
}

// (b.eyers 2016-04-29 14:59) - NX-100350 - copied from ascutils
struct FindExportWindowToActivate
{
	FindExportWindowToActivate(const CString &strSubkey, const CString &strTitle = "", const CString &strExeName = "")
		: strSubkey(strSubkey),
		strTitle(strTitle),
		strExeName(strExeName)
	{
	}

	HWND m_hFoundWindow = NULL;
	CString strSubkey = "";
	CString strTitle = "";
	CString strExeName = "";

	BOOL operator()(HWND hwnd)
	{
		char szWindowText[255] = { 0 };
		::GetWindowText(hwnd, szWindowText, 255);

		CString strExpectedTitle = GetExporterWindowTitle(strTitle, strSubkey);

		if (!strExpectedTitle.Compare(szWindowText))
		{
			// We found the window, now lets make sure the process filename also matches
			DWORD dwProcessID = 0;
			DWORD dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
			if (dwProcessID > 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, TRUE, dwProcessID);
				if (NULL != hProcess)
				{
					HINSTANCE hInstLib = LoadLibraryA("PSAPI.DLL");
					if (NULL != hInstLib)
					{
						DWORD(WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE,
							LPTSTR, DWORD) = (DWORD(WINAPI *)(HANDLE, HMODULE,
								LPTSTR, DWORD)) GetProcAddress(hInstLib,
									"GetModuleBaseNameA");
						if (NULL != lpfGetModuleBaseName)
						{
							char szFileName[MAX_PATH];
							if (!lpfGetModuleBaseName(hProcess, NULL,
								szFileName, sizeof(szFileName)))
							{
								szFileName[0] = 0;
							}
							if (!CString(szFileName).CompareNoCase(strExeName))
							{
								m_hFoundWindow = hwnd;
							}
						}
						FreeLibrary(hInstLib);
					} // if (NULL != hInstLib)
					CloseHandle(hProcess);
				} // if (NULL != hProcess)
			} // if (dwProcessID > 0)
		}

		return (NULL == m_hFoundWindow);
	}

	CString GetExporterWindowTitle(const CString &strBaseTitle, const CString &strSubkey)
	{
		CString strWindowTitle = strBaseTitle;
		if (!strSubkey.IsEmpty())
		{
			strWindowTitle += FormatString(" (%s)", strSubkey);
		}

		return strWindowTitle;
	}
};


// (b.eyers 2016-04-29 14:59) - NX-100350 - open the pqrs app
void OpenPQRSExporter(const CString &strSubkey)
{
	CString strWindowTitle, strExeFullPath;
	strWindowTitle = "PQRS \\ eCQM Reporting";
	strExeFullPath = "PQRSExporter.exe";

	try {
		
		FindExportWindowToActivate f(strSubkey, strWindowTitle, strExeFullPath);
		WindowUtils::EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()), &f);
		if (NULL != f.m_hFoundWindow)
		{
			// We found it; try to make sure it's visible
			if (::IsIconic(f.m_hFoundWindow))
			{
				::ShowWindowAsync(f.m_hFoundWindow, SW_RESTORE);
			}
			::SetForegroundWindow(f.m_hFoundWindow);
		}
		else
		{
			// (s.tullis 2016-05-20 15:47) - NX-100351  
			CString strPQRSUpdateURL;
			COleDateTime dtServerAppDate;
			// (s.tullis 2016-06-03 12:32) - NX-100798 
			//Init to the Shared Path, But pass in a the file path variable and UpdatePQRSApp to tell if we need to open at another path
			CString strPQRSExePath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "PQRSExporter" ^ "PQRSExporter.exe";
			GetPQRSAppNeedsUpdateURL(strPQRSUpdateURL,dtServerAppDate);
			if (!strPQRSUpdateURL.IsEmpty())
			{
				FILETIME ftPQRSFileTime = UnixTime::ToFileTime(UnixTime::FromUtcDateTime(dtServerAppDate));
				UpdatePQRSApp(strPQRSUpdateURL, strPQRSExePath, ftPQRSFileTime);
			}

			//might need a check here unless we always want to pass a login to new apps being open?
			NexTech_Accessor::_LoginResultPtr m_pLoginApp;
			NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
			in_addr ipAddress = GetLocalIPAddress();
			CString strDeviceInfo = CLicense::GetSystemName() + '.' + GetPracPath(g_pLicense->IsConcurrent() ? PracPath::SessionPath : PracPath::PracticePath);
			m_pLoginApp = pApi->LogInUser_ByUserID(_bstr_t(strSubkey), _bstr_t(g_nCurrentUserID), _bstr_t(g_strCurrentUserPassword)
				, _bstr_t(GetCurrentLocationID()), NexTech_Accessor::DeviceType_Practice, _bstr_t(strDeviceInfo), short(ipAddress.S_un.S_un_b.s_b1)
				, short(ipAddress.S_un.S_un_b.s_b2), short(ipAddress.S_un.S_un_b.s_b3), short(ipAddress.S_un.S_un_b.s_b4)
				, NULL, g_cbstrNull);
			CString strLoginToken = (LPCTSTR)m_pLoginApp->LoginToken;

			// We didn't find it; lets launch it.
			CString strParam = "";
			if (!strSubkey.IsEmpty()) {
				strParam = FormatString("l:%s r:%s", strLoginToken, strSubkey);
			}
			else {
				strParam = FormatString("l:%s", strLoginToken);
			}
			// Just do a standard shell execution. I tried ShellExecuteModal but didn't like Practice being frozen by an hour glass nor
			// the exporter as an always-topmost window.
			SHELLEXECUTEINFO sei;
			memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
			// Init the info object according to the parameters given.
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.hwnd = (HWND)GetDesktopWindow();
			sei.lpFile = (LPCTSTR)strPQRSExePath;
			sei.lpParameters = (LPCTSTR)strParam;
			sei.nShow = SW_SHOW;
			sei.hInstApp = NULL;
			if (!ShellExecuteEx(&sei))
			{
				CString strError;
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
				strError.ReleaseBuffer();
				::MessageBox(NULL, FormatString("Failed to launch %s\n\n%s", strPQRSExePath, strError), NULL, MB_ICONHAND);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2016-05-20 15:47) - NX-100351 
// Gets the PQRS App modified date and Version and 
// Sends a Soap call to the Nextech Updates service
// Nextech Updates processes the request and will return a URL to a file if one exists
// else null string and the PQRS app is up to date so we don't need do anything later
void GetPQRSAppNeedsUpdateURL(CString &strURL, COleDateTime &dtAppDate)
{
	try {

		COleDateTime dtModifiedTime(1970, 1, 1, 12, 0, 0);

		//we want to compare the local exe's time against the zip on nextech updates
		//we set the exe later to the creation date of the latest zip on the server so we will always get an update if needed or get nothing if not needed
		CString strExeFilePath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath")  ^ "PQRSExporter" ^ "PQRSExporter.exe";

		//the exe file might not exist if we have never downloaded it before
		if (FileUtils::DoesFileOrDirExist(strExeFilePath)) {
			//get the file modified time
			FILETIME ftFileModified = FileUtils::GetFileModifiedTime(strExeFilePath);
			dtModifiedTime = COleDateTime(ftFileModified);
		}

		//this is the oldest file version this version of Practice supports,
		//it is not necessarily the current version of Practice
		CString strXml = GetXMLElementValuePair("fileversion", "12.4");
		strXml += GetXMLElementValuePair("filedate", dtModifiedTime.Format());

		MSXML2::IXMLDOMNodePtr xmlResponse = CallSoapFunction("https://services.nextech.com/Client/Update/NextechUpdate.asmx",
			"https://services.nextech.com/Client/Update", "CheckPQRSNeedsUpdate", strXml);

		if (xmlResponse)
		{
#ifdef DEBUG
			//for debugging
			CString strResult = (LPCTSTR)xmlResponse->text;

			//if no newer version was found, pNodeURL will be null,
			//pNodeDate will be the date of the exe currently on the server,
			//and strResult will only have this date in its content
#endif
			MSXML2::IXMLDOMDocument2Ptr xmlDocument = xmlResponse->ownerDocument;
			if (xmlDocument)
			{
				xmlDocument->setProperty("SelectionNamespaces", "xmlns:ns='https://services.nextech.com/Client/Update'");
				
				MSXML2::IXMLDOMNodePtr pNodeURL = xmlResponse->selectSingleNode("//ns:CheckPQRSNeedsUpdateResponse/CheckPQRSNeedsUpdateResult/URL/text()");
				MSXML2::IXMLDOMNodePtr pNodeDate = xmlResponse->selectSingleNode("//ns:CheckPQRSNeedsUpdateResponse/CheckPQRSNeedsUpdateResult/CreateDate/text()");
				if (pNodeURL && pNodeDate)
				{
					strURL = "https://services.nextech.com/Client/Update" + CString((LPCTSTR)pNodeURL->text);
					ParseDateTime_ISO8601(CString((LPCTSTR)pNodeDate->text), dtAppDate);
				}
			}
		}

	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-06-03 9:21) - NX-100798 
// Azure Remote app clients run practice with permissions to only read and execute the files in the install path that are installed by NXCD. 
// We need to be able to still run the new PQRS app if we don't have the permissions to modify / delete the old file in the install path.
// If we fail to modify the file in the install path then we will just unzip the new downloaded exe to the session path and run it from there.
// (s.tullis 2016-05-20 15:47) - NX-100351 
// Download the Zip containing the new EXE
// Unzip and copy to the install path.. then delete the old and temp files
void UpdatePQRSApp(IN CString strPQRSAppURL, OUT CString &strPQRSAppFilePathToRunExecute, IN const FILETIME &ftPQRSFileTime)
{
	try {

		//get our installed PQRS path
		CString strPQRSPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath") ^ "PQRSExporter";
		
		//get a temp file name to download the zip file
		CString strTempZipFileName = FileUtils::GetTempFileName("PQRSApp");

		//download the newest zip file
		InternetUtils::DownloadFile(strPQRSAppURL, strTempZipFileName);

		DWORD dwValid = NxCompressUtils::ValidateZipFile(strTempZipFileName, false);
		if (dwValid != 0) {
			ThrowNxException("PQRS Exporter Zip File Invalid.");
		}

		//copy the zip file to the PQRS path 
		CString strLocalDownloadedZipFile = strPQRSPath ^ "PQRSApp.zip";
	
		//get the official exe name
		CString strExeName = strPQRSPath ^ "PQRSExporter.exe";

		//generate a temp name to rename the existing exe to
		CString strTempExeName = strPQRSPath ^ FormatString("%li_DELETEME.exe", (long)GetTickCount());
	
		if (FileUtils::DoesFileOrDirExist(strExeName)) {
			//move the old exe to the temp name to be deleted
			if (!::MoveFile(strExeName, strTempExeName)) {
				// (s.tullis 2016-06-03 9:21) - NX-100798 - We failed to move / rename the old exe
				// this means we do have sufficient permissions to change files in the install path
				// we will now just unzip the new PQRS app to the SessionPath and run it from there.
				strPQRSPath = GetPracPath(PracPath::SessionPath);
				strExeName = strPQRSPath ^ "PQRSExporter.exe";
				strLocalDownloadedZipFile = strPQRSPath ^ "PQRSApp.zip";
				//delete the old exe file if one is already there
				if (FileUtils::DoesFileOrDirExist(strExeName)) {
					DeleteFile(strExeName);
				}
			}
			else {
				//Mark the old file for deletion only if we succeeded to rename and move it
				FileUtils::CAutoRemoveTempFile tempExe(strTempExeName);
			}
		}
		// Mark our temp files for deletion
		FileUtils::CAutoRemoveTempFile tempZipFile2(strLocalDownloadedZipFile);
		FileUtils::CAutoRemoveTempFile tempZipFile1(strTempZipFileName);

		//move the temp zip file to our install path
		BOOL bSuccess = ::MoveFile(strTempZipFileName, strLocalDownloadedZipFile);
		if (!bSuccess) {
			ThrowNxException("Unable to move PQRS Exporter Zip file from %s to %s.", strTempZipFileName, strLocalDownloadedZipFile);
		}

		//now unzip the zip file to our official PQRS path
		NxCompressUtils::NxUncompressFileToFile(strLocalDownloadedZipFile, strPQRSPath);

		//force the exe time to be the time from the website
		FileUtils::SetFileModifiedTime(strExeName, ftPQRSFileTime);

		//Set the Location for the New Exe that will executed later
		strPQRSAppFilePathToRunExecute = strExeName;

	}NxCatchAll(__FUNCTION__)
}

/// <summary>
/// Returns the current call stack, formatted with newlines,
/// to include in an exception report.
/// Defaults to the last 20 frames, skipping this function call and
/// Nx::Debug::CallStack itself.
/// </summary>
/// <returns>A series of hex representations of the call stack. You can identify the function names
/// by pasting them into \nx-internal\Development\CallStackSymbolizer\ while pointed to the Practice.pdb
/// file associated with the exe this was thrown from.</returns>
CString GetCallStack()
{
	//default to 20 frames and one additional frame to skip,
	//which skips this overloaded function
	return GetCallStack(20, 1);
}

/// <summary>
/// Returns the current call stack, formatted with newlines,
/// to include in an exception report.
/// Allows configuring how many frames to return, and increasing
/// the count of frames to skip in addition to the default skips
/// of this function call and Nx::Debug::CallStack itself.
/// </summary>
/// <param name="dwMaxFramesToReturn">Specifies how many frames should be returned. Standard is 20.</param>
/// <param name="dwStackFramesToSkip">Specifies how many of the most recent frames should be skipped
/// in addition to the default 2 of this function and Nx::Debug::CallStack.</param>
/// <returns>A series of hex representations of the call stack. You can identify the function names
/// by pasting them into \nx-internal\Development\CallStackSymbolizer\ while pointed to the Practice.pdb
/// file associated with the exe this was thrown from.</returns>
CString GetCallStack(DWORD dwMaxFramesToReturn, DWORD dwStackFramesToSkip)
{
	Nx::Debug::CallStack cs;

	//always increase the frames to skip by 2 to account for
	//this function call and Nx::Debug::CallStack
	dwStackFramesToSkip += 2;
	
	// Note: This is always going to be 0x400000.
	HANDLE hApplication = GetModuleHandle(NULL);
	CString strCallStack = FormatString("Call stack:\nApp base: %p\n", hApplication);

	DWORD dwCountToAdd = min(dwMaxFramesToReturn, (cs.count < dwStackFramesToSkip) ? 0 : (cs.count - dwStackFramesToSkip));
	strCallStack += "";
	for (DWORD i = dwStackFramesToSkip; i < dwCountToAdd + dwStackFramesToSkip; i++)
	{
		strCallStack += FormatString("%p ", cs.frames[i]);
	}

	return strCallStack;
}
