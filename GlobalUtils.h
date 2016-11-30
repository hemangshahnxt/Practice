#ifndef Practice_Global_Utilities_h
#define Practice_Global_Utilities_h

#pragma once

// (j.armen 2011-10-26 16:02) - PLID 46132 - Removed dead code
// (a.walling 2008-05-23 14:44) - PLID 30099 - Global flag for using clipchildren on nxdialogs
// (a.walling 2008-06-17 15:03) - PLID 30099 - define this to revert to drawing without clipping children
//#define NXDIALOG_NOCLIPCHILDEN

#define GetDlgItemCheck(a, b) b = ((CButton *)GetDlgItem(a))->GetCheck()
#define SetDlgItemCheck(a, b) ((CButton *)GetDlgItem(a))->SetCheck(b)
#define DEVELOPER_USERNAME "Developer"

#define BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERNAME		_T("NexTech Technical Support")
#define BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID		(-26)

#include "peplus.h"
#include "PreferenceUtils.h"
#include <CxImage/ximage.h> // (a.walling 2013-05-08 16:15) - PLID 56610 - ximage.h now in CxImage/
#include "NxAES.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
// (a.walling 2010-03-15 11:55) - PLID 37751 - NxCryptoUtils
#include "NxCryptoUtils.h"
#include "oledberr.h"
#include "SOAPUtils.h"
#include <NxNetworkLib/TableCheckerDetails.h>
#include <AxControl.h>
#include "HistoryUtils.h"

#import <NxWindowless.tlb>
//#import "libid:9511C7D0-C2F0-4BC8-841A-F980A950A831" version("1.0")

namespace NxWindowlessLib
{
	// (a.walling 2011-11-11 11:11) - PLID 46619 - AxControl - typedefs for the windowless button and label controls
	_AXCONTROL_TYPEDEF(NxFreeButton, __uuidof(NxFreeButton), _DNxFreeButton, __uuidof(_DNxFreeButton));
	_AXCONTROL_TYPEDEF(NxFreeLabel, __uuidof(NxFreeLabel), _DNxFreeLabel, __uuidof(_DNxFreeLabel));
}

#define MEGABYTES(num)  ((num) << 20)

#define AC_CAN_CLOSE	0x01
#define AC_CANNOT_CLOSE	0x02

// (a.walling 2010-05-03 09:44) - PLID 38553 - Used by shared static lib
CString GetProductVersionText();

class CMainFrame;

namespace NexTech_COM
{
	struct IICCPSessionManager;
}

typedef enum 
{
	lvlInvalid = 0,
	lvlPrimary = 1,
	lvlSecondary = 2, 
	lvlAll = 3
} InfoLevel;

typedef enum 
{
	statusNone = 0,
	statusAdded = 1,
	statusChanged = 2,
	statusDeleted = 4,
} StatusLevel;

// (r.galicki 2008-07-17 12:32) - PLID 30764 - Moved PropertyType definition to NxPropManager.h - Shared utility

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated legacy module security items
typedef enum
{
	Invalid = -1,
	PatientModuleItem,
	ChangeWarning,
	CustomLabel,
	NewPatient,
	DeletePatient,
	ChangePatientID,
	Medication,
	DeleteAppointment,
	DeleteHistory,
	Insurance,
	EditInsuranceList,
	AddInsuredParty,
	Quotes,
	BillingModuleItem,
	ViewPayment,
	NewPayment,
	EditPayment,
	NewAdjustment,
	EditAdjustment,
	NewRefund,
	EditRefund,
	ViewBill,
	NewBill,
	EditBill,
	HCFASecurityItem, // (a.walling 2014-03-19 10:05) - PLID 61346 - Unused but also creating a top-level symbol named 'HCFA'. 
	// This whole thing needs to be isolated in a namespace anyway.
	EBilling,
	SchedulerModuleItem,
	EditAppointment,
	DragAppointment,
	PasteAppointment,
	LetterWritingModuleItem,
	EditGroup,
	EditFilter,
	MarketingModuleItem,
	EditReferralTree,
	FinancialModuleItem,
	InventoryModuleItem,
	NewItem,
	DeleteItem,
	EditItem,
	PlaceOrder,
	EditOrder,
	MarkReceived,
	DeleteReceived,
	ContactModuleItem,
	NewContact,
	DeleteContact,
	ReportModuleItem,
//	AdminReportTab,
//	MarketingReportTab,
//	InventoryReportTab,
//	FinancialLowSecurity,
//	FinancialHighSecurity,
//	ContactReportTab,
//	PatientReportTab,
	AdministratorModuleItem,
	PracticeConfig,
	EmployeeConfig,
	ProviderConfig,
	ReferringPhysicianConfig,
	HCFAConfig,
	BillingConfig,
	SurgeryConfig,
	MultiFeeConfig,
	ScheduleConfig,
	InterestConfig,
	ZipCodeConfig,
	MirrorIntegration,
	ViewImage,
	ImportMirror,
	ExportMirror,
	InformIntegration,
	ImportInform,
	ExportInform,
	AuditingAdminTab,
	AuditingManagement,
	UnitedIntegration,
	UnitedImport,
	UnitedExport,
	ChangePatientName,
	SecuritySize,	
} SecurityItem;

typedef enum
{
	eChangedHFFirst = 0L,
	eChangedHFLast,
	eChangedHFID,
} EChangedHFItem;

// (f.dinatale 2010-10-11) - PLID 33753 - Enumerated type for SSN masking options.
typedef enum
{
	eSSNNoMask = 0,
	eSSNPartialMask,
	eSSNFullMask,
}ESSNMask;


struct NxZipCode {

	CString strCity;
	CString strState;
	CString strZip;
	CString strAreaCode;
};

// (c.haag 2006-12-27 15:39) - PLID 23300 - The AddFolderToList function, when
// running asynchronously, now posts message to the owner window to populate a
// datalist
struct AddFolderMessage
{
	CString strFileName;
	CString strFolderName;
	LPDISPATCH pList;
};



// (j.gruber 2012-10-16 17:58) - PLID 47289
/*****EXISTS IN DATA, DO NOT MODIFY!!!!!!!*************/
typedef enum  {
	afptNone = -1,
	afptPreOp = 1,
	afptPostOp = 2,
	afptPrePostOp = 3,
}AffiliatePhysType;

// (r.gonet 06/10/2011) - PLID 30359 - Wrapper for a boolean flag that is set on construction
//  and reset when the object is deleted or goes out of scope.
class CCriticalFlag
{
private:
	bool &m_bFlag;

public:
	CCriticalFlag(bool &bFlag)
		: m_bFlag(bFlag)
	{
		m_bFlag = true;
	}

	~CCriticalFlag()
	{
		m_bFlag = false;
	}
};

// (a.walling 2010-05-06 11:52) - PLID 38553 - Sentinel values now in NxDataUtilities SDK
/*
// (a.walling 2007-08-30 18:00) - PLID 19106
VARIANT GetVariantNull();
VARIANT GetVariantEmpty();

// Commonly used variants.  We use these values all over the place, so we 
// might as well save some memory and cpu time by making it so everyone can 
// access the same global instantiations of them.
extern const _variant_t g_cvarTrue;
extern const _variant_t g_cvarFalse;
extern const _variant_t g_cvarNull;
// (a.walling 2007-08-30 17:58) - PLID 19106
extern const _variant_t g_cvarEmpty;
*/

// (a.walling 2008-10-02 17:22) - PLID 31575 - Revert to classic border drawing if true
// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
//extern BOOL g_bClassicToolbarBorders;

// (a.walling 2007-03-27 13:03) - PLID 25367 - Must define this here for the header to work out alright.
typedef CArray<_variant_t,_variant_t&> CVariantArray;


// (j.gruber 2011-07-22 17:25) - PLID 44118
#define PATIENT_LIST_INACTIVE_COLOR  RGB(116,129,185)


// (a.walling 2008-02-15 16:14) - PLID 28946 - Anyone else tired of writing extern CLicense* g_pLicense? Me, too.
// (z.manning 2015-05-20 09:37) - PLID 65971 - Changed to CPracticeLicense
class CPracticeLicense;
extern CPracticeLicense* g_pLicense;

// (a.walling 2008-09-15 10:54) - PLID Windycane Ike!
DWORD GetGDICount();
DWORD GetUSERCount();

// (j.gruber 2009-10-05 13:14) - PLID 35607 - changed to allow for city searching
bool QueryZiporCityInfo(CString strSearchZiporCity, CString *ZiporCity, CString *State, CString *AreaCode, BOOL bSearchZip);
bool QueryZipInfo(CString strSearchZip, CString *City, CString *State, CString *AreaCode);
bool QueryCityInfo(CString strSearchCity, CString *Zip, CString *State, CString *AreaCode);
// (a.walling 2010-10-04 09:51) - PLID 40573 - Check and update bad static IDs in ZipCodesT
void CheckZipStaticIDs();
bool ContainsDigit(CString strIn);
BOOL ContainsNonDigits(LPCTSTR str);

BOOL IgnoreRightAlt();

// (a.walling 2007-10-12 14:34) - PLID 26342 - Moved this to a shared string rather than update various different places in practice
// also included macro-enabled version of 2007 documents
extern const char* szCommonlyAttached;

// (a.walling 2007-10-29 17:12) - PLID 27891 - Global encryption class
extern NxAES g_NxAES;

// (a.walling 2007-10-30 17:49) - PLID 27891 - Returns a decrypted string from a variant array of bytes
CString DecryptStringFromVariant(_variant_t var);

// (a.walling 2007-10-30 17:42) - PLID 27891 - Returns a string that can be put directly into SQL statement
// Note: if the length of the string is zero, then '' will be returned, or NULL if bUseNull
CString EncryptStringForSql(const CString& str, BOOL bUseNull = TRUE);

// (j.jones 2009-04-30 16:26) - PLID 33853 - added EncryptStringToVariant, used in param recordsets
_variant_t EncryptStringToVariant(const CString& str);

// Useful for farely consistent use of the hourglass
#define BEGIN_WAIT_CURSOR()	{ CWnd *pWnd = AfxGetApp()->m_pMainWnd; if (pWnd) pWnd->BeginWaitCursor(); }
#define END_WAIT_CURSOR()		{ CWnd *pWnd = AfxGetApp()->m_pMainWnd; if (pWnd) pWnd->EndWaitCursor(); }

//warning message
int ShowWarningMessage (const CString	&message = "",
						const CString	&title = "Warning", 
						const CString	&button1 ="Yes", 
						const CString	&button2= "No");

// (a.walling 2011-03-19 16:18) - PLID 42914 - NxCatch macros defined in NxCatch now within NxExceptionLib

////standard error handling saves two lines per try block
//// (c.haag 2007-07-19 10:57) - PLID 26744 - Added new macros for throwing and calling code in threads
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfc(message)	catch (CException *e) { HandleException(e, message, __LINE__, __FILE__); }
//#define NxCatchCom(message)	catch (_com_error& e) { HandleException(e, message, __LINE__, __FILE__); }
//#define NxCatchLow(message)	catch (...) { HandleException(NULL, message, __LINE__, __FILE__); }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Need to tell HandleException to NOT autodelete the CException* if we are throwing it, otherwise we
//// end up with access violations. Also changed to catch the _com_error by reference rather than invoke the copy constructor.
//#define NxCatchMfcThrow(message)	catch (CException *e) { HandleException(e, message, __LINE__, __FILE__, FALSE); throw; }
//#define NxCatchComThrow(message)	catch (_com_error& e) { HandleException(e, message, __LINE__, __FILE__); throw; }
//#define NxCatchLowThrow(message)	catch (...) { HandleException(NULL, message, __LINE__, __FILE__); throw; }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfcCall(message, code)	catch (CException *e) { HandleException(e, message, __LINE__, __FILE__); code; }
//#define NxCatchComCall(message, code)	catch (_com_error& e) { HandleException(e, message, __LINE__, __FILE__); code; }
//#define NxCatchLowCall(message, code)	catch (...) { HandleException(NULL, message, __LINE__, __FILE__); code; }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfcCallThrow(message, code)	catch (CException *e) { HandleException(e, message, __LINE__, __FILE__, FALSE); code; throw; }
//#define NxCatchComCallThrow(message, code)	catch (_com_error& e) { HandleException(e, message, __LINE__, __FILE__); code; throw; }
//#define NxCatchLowCallThrow(message, code)	catch (...) { HandleException(NULL, message, __LINE__, __FILE__); code; throw; }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfcCallIgnore(code)	catch (CException *e) { e->Delete(); code; }
//#define NxCatchComCallIgnore(code)	catch (_com_error&) { code; }
//#define NxCatchLowCallIgnore(code)	catch (...) { code; }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfcIgnore()	catch (CException *e) { e->Delete(); }
//#define NxCatchComIgnore()	catch (_com_error&) { }
//#define NxCatchLowIgnore()	catch (...) { }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfcThread(message) catch (CException *e) { HandleException(e, message, __LINE__, __FILE__, TRUE, "", "", TRUE); }
//#define NxCatchComThread(message) catch (_com_error& e) { HandleExceptionThread(e, message, __LINE__, __FILE__, "", ""); }
//#define NxCatchLowThread(message) catch (...) { HandleException(NULL, message, __LINE__, __FILE__, TRUE, "", "", TRUE); }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Changed to catch _com_error by reference rather than invoke the copy constructor
//#define NxCatchMfcCallThread(message,code) catch (CException *e) { HandleException(e, message, __LINE__, __FILE__, TRUE, "", "", TRUE); code;}
//#define NxCatchComCallThread(message,code) catch (_com_error& e) { HandleExceptionThread(e, message, __LINE__, __FILE__, "", ""); code; }
//#define NxCatchLowCallThread(message,code) catch (...) { HandleException(NULL, message, __LINE__, __FILE__, TRUE, "", "", TRUE); code; }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Need to tell HandleException to NOT autodelete the CException* if we are throwing it, otherwise we
//// end up with access violations. Also changed to catch the _com_error by reference rather than invoke the copy constructor.
//#define NxCatchMfcThrowThread(message) catch (CException *e) { HandleException(e, message, __LINE__, __FILE__, FALSE, "", "", TRUE); throw; }
//#define NxCatchComThrowThread(message) catch (_com_error& e) { HandleExceptionThread(e, message, __LINE__, __FILE__, "", ""); throw; }
//#define NxCatchLowThrowThread(message) catch (...) { HandleException(NULL, message, __LINE__, __FILE__, TRUE, "", "", TRUE); throw; }
//
//// (a.walling 2009-12-02 09:04) - PLID 36464 - Need to tell HandleException to NOT autodelete the CException* if we are throwing it, otherwise we
//// end up with access violations. Also changed to catch the _com_error by reference rather than invoke the copy constructor.
//#define NxCatchMfcCallThrowThread(message,code) catch (CException *e) { HandleException(e, message, __LINE__, __FILE__, FALSE, "", "", TRUE); code; throw; }
//#define NxCatchComCallThrowThread(message,code) catch (_com_error& e) { HandleExceptionThread(e, message, __LINE__, __FILE__, "", ""); code; throw; }
//#define NxCatchLowCallThrowThread(message,code) catch (...) { HandleException(NULL, message, __LINE__, __FILE__, TRUE, "", "", TRUE); code; throw; }
//
//// Catches ALL in the various ways represented above
//#define NxCatchAll(message)					NxCatchMfc(message)					NxCatchCom(message)					NxCatchLow(message)
//#define NxCatchAllThrow(message)			NxCatchMfcThrow(message)			NxCatchComThrow(message)			NxCatchLowThrow(message)
//#define NxCatchAllCall(message, code)		NxCatchMfcCall(message, code)		NxCatchComCall(message, code)		NxCatchLowCall(message, code)
//#define NxCatchAllCallThrow(message, code)	NxCatchMfcCallThrow(message, code)	NxCatchComCallThrow(message, code)	NxCatchLowCallThrow(message, code)
//#define NxCatchAllSilentCallThrow(code)		catch (...) { code; throw; }
//#define NxCatchAllCallIgnore(code)			NxCatchMfcCallIgnore(code)			NxCatchComCallIgnore(code)			NxCatchLowCallIgnore(code)
//#define NxCatchAllIgnore()					NxCatchMfcIgnore()					NxCatchComIgnore()					NxCatchLowIgnore()
//#define NxCatchAllThread(message)			NxCatchMfcThread(message)			NxCatchComThread(message)			NxCatchLowThread(message)
//#define NxCatchAllCallThread(message, code)	NxCatchMfcCallThread(message,code)	NxCatchComCallThread(message,code)	NxCatchLowCallThread(message,code)
//#define NxCatchAllThrowThread(message)		NxCatchMfcThrowThread(message)		NxCatchComThrowThread(message)		NxCatchLowThrowThread(message)
//#define NxCatchAllCallThrowThread(message, code)	NxCatchMfcCallThrowThread(message,code)	NxCatchComCallThrowThread(message,code)	NxCatchLowCallThrowThread(message,code)

// (a.walling 2011-03-19 16:18) - PLID 42914 - Exception handler callbacks from NxCatch
namespace NxCatch
{
namespace Practice
{
	int HandleMFCException(CException* e,
		LPCTSTR szMessage, DWORD line, LPCTSTR szFile, DWORD flags, HWND hwndUI);

	int HandleCOMException(_com_error& e,
		LPCTSTR szMessage, DWORD line, LPCTSTR szFile, DWORD flags, HWND hwndUI);
};
};

// (a.walling 2011-03-19 16:18) - PLID 42914 - file parameter is const now
// Universally handles the exception, creating a message box, or 
// asking a question if need be.
//TES 6/6/06 - If bCalledFromThread is TRUE, the function won't call thread-unsafe functions (like GetCurrentModuleName()).
int HandleException(CException *e, CString message = "", unsigned long line = -1, const char *file = NULL, BOOL bAutoDelete = TRUE, CString strManualLocation = "", CString strManualBookmark = "", BOOL bCalledFromThread = FALSE);
//int HandleException(_com_error &e, CString message = "", unsigned long line = -1, char *file = NULL, CString strManualLocation = "", CString strManualBookmark = "", BOOL bCalledFromThread = FALSE);
//TES 6/8/2006 - For some unknown reason, the above call to HandleException caused an INTERNAL_COMPILER_ERROR in release mode
//in financialdlg.cpp.  So I left that function implementation in globalutils.cpp, and added these function declarations, which
//just call the original implementation with TRUE and FALSE respectively passed as bCalledFromThread.  WHY this fixes the problem,
//I have no idea, and probably someday the problem will pop back up again.  But I don't even know how to go about investigating it,
//so I'm leaving this workaround for the time being.
int HandleException(_com_error &e, CString message = "", unsigned long line = -1, const char *file = NULL, CString strManualLocation = "", CString strManualBookmark = "");
int HandleExceptionThread(_com_error &e, CString message = "", unsigned long line = -1, const char *file = NULL, CString strManualLocation = "", CString strManualBookmark = "");

// This global handler should only be called from the main thread in response to 
// the NXM_DISPLAY_ERROR_MESSAGE.  Just pass in the wParam and lParam given by the 
// message and return the LRESULT returned by this function.  The function will 
// take care of the rest.  NOTE: This handler must be called EXACTLY once for each 
// NXM_DISPLAY_ERROR_MESSAGE message received by the main thread.  Do not discard 
// the message because it will result in a memory leak, and do not call it multiple 
// times because it will result in an access violation.
LRESULT Global_OnShowErrorMessage(WPARAM wParam, LPARAM lParam);

void LoadErrorInfoStrings();

// (a.walling 2010-01-13 17:13) - PLID 31253 - Moved GEI_* defines to header file
#define GEI_DOCTOR_NAME			0
#define GEI_PHONE_NUMBER		1
#define GEI_COMPUTER_NAME		2
#define GEI_DOCK_PATH			3
#define GEI_LOCAL_PATH			4
#define GEI_VERSION				5
#define GEI_PRACTICE_DATE		6
#define GEI_WINDOWS_USERNAME	7
#define GEI_NEXTECH_USERNAME	8
#define GEI_OPERATING_SYSTEM	9
//(c.copits 2009-12-22) PLID 18264 - Add license number to very beginning of subject line
#define GEI_LICENSE_NUMBER		10
//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
#define GEI_USER_EMAIL			11
#define GEI_OFFICE_EMAIL		12

//(c.copits 2009-12-22) PLID 18264 - Add license number to very beginning of subject line
//(c.copits 2010-07-14) PLID 37944 - Include Email in error submissions
#define GEI_STRING_COUNT		13

// (a.walling 2010-01-13 17:12) - PLID 31253 - Copied definition to header file
CString GetErrorInfoString(LONG nInfoType);

long CalcVersion(const CString &strVersionString);

// (a.walling 2007-08-07 17:42) - PLID 26996 - TRACE the current number of GDI objects
#ifdef _DEBUG
	void dbg_TraceGDIObjects(CString str = "");
	#define TRACE_GDI_OBJECTS(str) dbg_TraceGDIObjects(str)
#else
	#define TRACE_GDI_OBJECTS(str) ((void)0)
#endif

long GetActivePatientID();
CString GetActivePatientName();
// (j.jones 2008-10-31 13:20) - PLID 31891 - supported an optional passed-in connection
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
CString GetExistingPatientName(long PatientID);
// (r.farnworth 2016-02-03 08:18) - PLID 68116 - We need the ability to lookup Location Name
CString GetLocationName(long LocationID);
// (b.savon 2011-11-16 15:28) - PLID 45433 - Thread safe, Does Patient Exist?  For Device Import
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
BOOL DoesPatientExistByUserDefinedID(const long nUserDefinedID);

long GetActiveContactID();
CString GetActiveContactName();
CString GetExistingContactName(long PersonID);
// (c.haag 2009-05-07 15:58) - PLID 28561 - This function returns a username given a valid UserID
CString GetExistingUserName(long PersonID);
// (j.jones 2007-07-17 09:21) - PLID 26702 - added birthdate functions
COleDateTime GetActivePatientBirthDate();
COleDateTime GetExistingPatientBirthDate(long PatientID);
// (z.manning 2009-05-05 10:44) - PLID 28529 - Added user defined ID functions
long GetActivePatientUserDefinedID();

// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
long GetExistingPatientUserDefinedID(const long nPatientID);
// (a.wilson 2013-01-10 09:55) - PLID 54515 - needed for converting userdefinedid into patient internal id.
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
long GetExistingPatientIDByUserDefinedID(long nPatientUserDefinedID);


void GetResBoxFormat(LPCTSTR *pstrResBoxFmt, LPCTSTR *pstrResBoxFmtEnd, bool bShowLocName, bool bShowShowStateOnPrint, BOOL bShowPatientInfo, BOOL bShowResourceInfo, COLORREF clrDefaultTextColor, int nExtraInfoStyles[10]);
//void ReloadResBoxFormat(); //Queries the data to find the right way to format this string.
void SetResBoxFormat(const LPCTSTR c_strResBoxFmt); //Sets up the global variables.

CString GetTemplatePath(const CString &strSubFolder = "", const CString &strTemplateFileName = "");

// (a.walling 2010-10-14 14:15) - PLID 40965 - This is now an alias basically for !ReturnsRecords
bool IsRecordsetEmpty(LPCTSTR strFmt, ...);
long GetRecordCount(const CString &strSql);

CString GetCurrentModuleName();

// Takes a file name of the form basename####.ext and increments the #### by one
// (b.cardillo 2005-09-21 16:53) - If strFile doesn't end with an extension, pass false for 
// the second parameter (otherwise it might incorrectly consider part of the basename to 
// actually be the extension; e.g. if your basename contains a dot)
// Returns true for success, false for failure
bool IncrementFileName(IN OUT CString &strFile, bool bFilenameIncludesExtension);

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

void StoreZipBoxes(CWnd *pWnd, int nCity, int nState, int nZip, 
				   const CString &table, const CString &keyField, int keyVal, 
				   const CString &cityField, const CString &stateField, const CString &zipField);
void StoreZipBoxes(CWnd *pWnd, int nCity, int nState, int nZip,
				   const CString &table, const CString &keyField, const CString &keyVal,
				   const CString &cityField, const CString &stateField, const CString &zipField);
void SetZipBoxes(CWnd *pWnd, int nCity, int nState, int nZip);
bool GetZipInfo(CString zipStr, CString *City, CString *State, CString *AreaCode = NULL);
// (j.gruber 2009-10-05 13:15) - PLID 35607 - added to allow city searching
bool GetZiporCityInfo(CString strSearchZiporCity, CString *ZiporCity, CString *State, CString *AreaCode = NULL, BOOL bSearchZipCode = TRUE);
bool GetCityInfo(CString strSearchCity, CString *Zip, CString *State, CString *AreaCode = NULL);
bool ShowAreaCode();
void SetShowAreaCode(bool bShowAreaCode);

// Security related functions
//bool CheckAccess(LPCTSTR strModule, LPCTSTR strFunction = NULL);
bool DoIHaveAccess(LPCTSTR strModule, LPCTSTR strFunction = NULL);
bool EnsureCurrentUser();
bool IsSecure(LPCTSTR strModule, LPCTSTR strFunction);
bool AskPassword(CString &strPass, const CString& strWindowText = "Please Enter Password");
HANDLE GetCurrentUserHandle();
void SetCurrentUserID(long nUserID); // (b.savon 2016-01-13 11:26) - PLID 67718 - Supplemental
long GetCurrentUserID();
BOOL IsCurrentUserAdministrator();	//PLID 20693
void SetCurrentUserAdministratorStatus(BOOL bIsAdmin);	//PLID 20693
void SetCurrentUserTechnicianStatus(BOOL bIsTechnician);	// (d.lange 2011-03-29 12:29) - PLID 42987 - We now are caching if the user is a technician
BOOL IsCurrentUserTechnician();
LPCTSTR GetCurrentUserName();
LPCTSTR GetCurrentUserPassword();
// (j.jones 2008-11-19 10:16) - PLID 28578 - added GetCurrentUserPasswordVerified()
BOOL GetCurrentUserPasswordVerified();
void SetCurrentUserPassword(LPCTSTR strUserPassword);	//DRT 7/23/03 - I don't really like putting this here, but I must to actually update the users pword when they change it in program (PLID 7780)
// (j.jones 2008-11-19 10:16) - PLID 28578 - added SetCurrentUserPasswordVerified()
void SetCurrentUserPasswordVerified(BOOL bIsPasswordVerified);
long GetCurrentLocationID();
//TES 9/5/2008 - PLID 27727 - I'm taking this field out, and this function wasn't called anywhere anyway.
//long GetProviderDefLocationID(long nProviderID);
LPCTSTR GetCurrentLocationName();
LPCTSTR GetCurrentLocationLogo(); // (a.walling 2008-07-01 16:29) - PLID 30586
long GetCurrentLocationLogoWidth(); // (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
long GetInactivityTimeout();
// This function was incorrectly named but is used all over the place
#define GetCurrentLocation		GetCurrentLocationID
void SetCurrentLocationID(long nLocationID);
void SetCurrentLocationName(LPCTSTR strNewLocationName);
// (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width
void SetCurrentLocationLogo(LPCTSTR strLogoPath, long nLogoWidth); // (a.walling 2008-07-01 16:33) - PLID 30586
void SetCurrentUserName(LPCTSTR strUserName);
// (j.jones 2008-11-19 10:38) - PLID 28578 - added bIsPasswordVerified as a parameter
void SetCurrentUser(HANDLE hUserHandle, long nUserID, LPCTSTR strNewUserName, LPCTSTR strNewUserPassword, BOOL bIsPasswordVerified, long nLocationId, long nInactiveMinutes = -1, LPCTSTR strNewLocationName = "NULL");
void SetInactivityTimeout(long nMinutes);

// This function is obsolete, DO NOT USE;  It is only here for backward compatibility
int UserPermission(SecurityItem item);
class CObsoleteSecurityItemLookup{public:int operator [](long siItem) const;};
extern CObsoleteSecurityItemLookup g_userPermission;

class CPermissions;
enum EBuiltInObjectIDs;
// These are the official functions to use to get the current user's permissions
//  - Use GetCurrentUserPermissions to get a CPermissions for the current user for the given object
//  - Use CanCurrentUserViewObject to silently determine whether the current user is allowed to view the given object
//  - Use CheckCurrentUserPassword to prompt the user to enter his password; returns true if answered correctly, false if cancelled
//  - Use CheckCurrentUserPermissions to do a quick check to see if the current user has the given permissions to the given object, if they have passworded permissions they will be prompted for the password, if they don't have permissions, they will be given a message saying so.  If bSilent is non-zero they will be given no user-feedback, so anything but password-free permissions will just return false
CPermissions GetCurrentUserPermissions(EBuiltInObjectIDs eBuiltInID, BOOL bUseObjectValue = FALSE, long nObjectValue = 0);
BOOL CanCurrentUserViewObject(EBuiltInObjectIDs eBuiltInID, BOOL bUseObjectValue = FALSE, long nObjectValue = 0);
BOOL CheckCurrentUserPassword(const CString& strPrompt = "Please Enter Password");
// (z.manning 2016-04-19 11:38) - NX-100244 - Removed the strPrompt param as it served no purpose
BOOL CheckAdministratorPassword();
// (j.jones 2013-08-08 15:18) - PLID 42958 - added password prompt for a specific user ID
BOOL CheckSpecificUserPassword(long nUserID, const CString& strPrompt = "Please Enter Password");
//DRT 6/21/2007 - PLID 25892 
// (z.manning 2016-04-19 14:48) - NX-100244 - Added optional param for if the user is locked out
BOOL CompareSpecificUserPassword(long nUserID, CString strPassword, OPTIONAL OUT BOOL *pbLockedOut = NULL);
BOOL CheckCurrentUserPermissions(EBuiltInObjectIDs eBuiltInID, IN const CPermissions &permsToCheck, BOOL bUseObjectValue = FALSE, long nObjectValue = 0, BOOL bSilent = FALSE, BOOL bAssumePasswordKnown = FALSE);
// (a.walling 2010-08-02 10:54) - PLID 39182 - Unified place to fire the "You do not have permission to access this function.\n" message box.
void PermissionsFailedMessageBox(const CString& strAdditionalInfo = "", const CString& strDoWhat = "access this function", const CString& strFinally = "Please see your office manager for assistance.");
void PermissionsFailedMessageBox(CWnd* pWndParent, const CString& strAdditionalInfo = "", const CString& strDoWhat = "access this function", const CString& strFinally = "Please see your office manager for assistance.");
// (z.manning 2009-06-10 14:29) - PLID 34585 - Check to see if current user has any permissions
BOOL DoesCurrentUserHaveAnyPermissions();




CString GetDefaultModuleName();
// (j.jones 2008-11-19 10:51) - PLID 28578 - added *pbIsPasswordVerified as a parameter
// (j.jones 2008-11-20 09:15) - PLID 23227 - added *pbPasswordExpireNextLogin as a parameter
//TES 4/30/2009 - PLID 28573 - Added *pnFailedLogins as a parameter
bool LoadUserInfo(IN const CString &strUsername, long nLocationID, OPTIONAL OUT CString *pstrPassword = NULL, OPTIONAL OUT BOOL *pbIsPasswordVerified = NULL, OPTIONAL OUT bool *pbAutoRecall = NULL, OPTIONAL OUT long *pnUserID = NULL, OPTIONAL OUT COleDateTime* pdtPwExpires = NULL, OPTIONAL OUT BOOL *pbPasswordExpireNextLogin = NULL, OPTIONAL OUT long *pnFailedLogins = NULL);

void DeleteNxRun20(CString DirPath);

// (a.walling 2009-10-09 16:42) - PLID 35911 - Set the main frame explicitly (to prevent several possible errors that may occur before the m_pMainWnd is reset)
void SetMainFrame(CMainFrame* pMainFrame);
CMainFrame *GetMainFrame();

// Time Stamp stuff
unsigned long IncrementTimeStamp(LPCTSTR strStamp);
unsigned long GetTimeStamp(LPCTSTR strStamp);

//Hotkey stuff
void InitHotKeys	(void);
void StartHotKeys	(void);
bool HotKeysEnabled	(void);

// (a.walling 2010-05-14 15:02) - PLID 38671 - DEBUGLOG for calling Log only in debug mode

#ifdef _DEBUG
#define DEBUGLOG(...)		Log(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

#define LOG_LINE()	Trace("%s(%li): Log Line"); Log("%s(%li): Log Line", __FILE__, __LINE__)
#define LOG_FUNCTION_START(strfunc)		LogIndent("%s - Start", (strfunc))
#define LOG_FUNCTION_END(strfunc)		LogUnindent("%s - End", (strfunc))

// LogDetail file stuff
void OpenLogFile();
void ClosePracticeLogFile();
void LogDetail(LPCTSTR strFmt, ...);

// Manual stuff
void OpenManual(const CString &strLocation, const CString &strBookmark);

COleDateTime GetNextAppt (COleDateTime dtNextAfter, long nPatID);

CString SafePath (const CString &path); //removes c:\\, etc.  May be a redudant implimentation

void UnformatText(CString &str);
BOOL UpdateInform (unsigned long patientID);
BOOL UpdateUnited (DWORD dwPracticeID);

CString ParseField(CString strLine, int nStart, int nCount);
void ImportCPTCodes();

// dialog box prompt
int Prompt (CString Caption, CString &NewName, int nMaxLength);

// icon stuff
DWORD SetControlIcon (CWnd *parent, UINT control, UINT icon);

//TES 11/25/2009 - PLID 36356 - Moved Trim() to GlobalStringUtils, so it can be shared with NxServer.
CString BuildClause(LPCTSTR strStartOp, LPCTSTR strFillerOp, ...);
UINT rtrim(TCHAR *pStr, LPCTSTR strChars);		// Quickly trims the trailing characters if they can be found in the strChars parameter
//bool DeleteQuery(const CString &strQueryName);
//bool DeleteTable(const CString &strTableName);

// Copies the actual exact file modification time from one file to another
bool CopyFileModifiedTime(LPCTSTR strSourcePathName, LPCTSTR strDestPathName);

#define GNC_PATIENT_STATUS				0x0001
#define GNC_FILTER_USE_OR				0x0002

#define GNC_ADMIN						0x0011
#define GNC_CONTACT						0x0012
#define GNC_FINANCIAL					0x0013
#define GNC_INVENTORY					0x0014
#define GNC_LETTER						0x0015
#define GNC_MARKET						0x0016
#define GNC_REPORT						0x0017

#define GNC_CPT_CODE					0x0018
#define GNC_PRODUCT						0x0019
#define GNC_PERSONNEL					0x0020
//DRT 4/30/2008 - PLID 29771
#define GNC_EMR_ITEM_BG					0x0021
// (a.walling 2009-08-13 09:39) - PLID 35136 - Move toolbar colors to global GetNxColor
#define GNC_TOOLBAR						0x0022
// (a.walling 2009-08-13 09:39) - PLID 35136 - Move MDI client background color to global GetNxColor
#define GNC_MDICLIENT					0x0023

// (a.walling 2009-08-13 09:39) - PLID 35136 - Return COLORREF
// (b.spivey, May 15, 2012) - PLID 20752 - Added nPatientID for custom colors
COLORREF GetNxColor(long nCategory, long nIndex, long nPatientID = -1);

//BVB - no one but me is allowed to write palette functions.  I am getting rid of this code

// This is like CreateHalftonePalette except if the hLoadedBmp 
// is 256 colors or less, it creates the palette of the bitmap
//BOOL CreatePaletteFromBitmap( OUT CPalette &palOut, HBITMAP hLoadedBmp);



// This draws the given bitmap on the given device context at the given spot.  
// NOTE: If cx or cy is given and they are not equal to the bitmap's actual 
// size, then the bitmap will be stretched automatically (which is slower)
BOOL DrawBitmap(CDC *pdc, HBITMAP hBitmap, long x, long y, long cx = -1, long cy = -1);

//ZM: 3/23/2006 - That function didn't always work for screenshots, so this one is used for that.
// Also, it's in GlobalUtils instead of GlobalDrawingUtils to minimize dependencies for other
// programs that use GlobalDrawingUtils since an xImage is used in this function.
CRect DrawDIBitmapInRectForPrinting(CDC *pDC, CRect rcImage, HBITMAP hImage);

bool CompareFields( COleVariant Field1, COleVariant Field2);

const CString &GetNxTempPath();
// (a.walling 2011-08-24 13:20) - PLID 45172 - Practice-specific, return FileUtils::GetLocalNxTempPath ^ strSubkey ^ strSubdirectory
CString GetLocalNxTempPath(const CString& strSubdirectory = "");

// (a.walling 2007-07-19 09:27) - PLID 26261 - Create a temp file in the NxTemp path
// (a.walling 2009-11-23 11:47) - PLID 36396 - Allow passing in flags so we can create hidden files, for example
// (a.walling 2013-01-02 11:24) - PLID 54407 - same as FileUtils::CreateTempFileInPath(GetNxTempPath(), ...)
HANDLE CreateNxTempFile(OPTIONAL IN const CString &strBaseString, OPTIONAL IN const CString &strExtensionNoDot, OPTIONAL OUT CString *pstrOutTempFilePathName = NULL, OPTIONAL BOOL bOverwriteExisting = FALSE, OPTIONAL DWORD dwFlags = FILE_ATTRIBUTE_NORMAL);

// (c.haag 2007-05-07 17:29) - This class will trace and optionally
// log on a SQL server when function calls begin and end.
class CFuncCallMonitor
{
private:
	CString m_strFuncLabel;
	BOOL m_bLogToSQL;
	BOOL m_bIncludeThreadID;
	ADODB::_ConnectionPtr m_pCon;
	long m_nStartTime;

public:
	CFuncCallMonitor(const CString& strFuncLabel, BOOL bLogToSQL = FALSE, ADODB::_ConnectionPtr pCon = NULL,
		BOOL bIncludeThreadID = FALSE);
	~CFuncCallMonitor();
};

// More helpful functions (These should be put into NxStandard)
bool GetSystemPath(OUT CString &strSystemPath);
bool GetPalmPath(OUT CString &strPalmPath);
bool GetDllFilePathName(IN const CString &strDllFileName, OUT CString &strDllFilePathName);

bool SetPalmRecordStatus(CString const strTableName, const unsigned long nID, 
	 const unsigned long nStatus);

bool SetPalmRecordID(CString const strTableName, const unsigned long nID, 
	 const long nRecID);

HKEY GetPalmKey();
bool GetPalmProfileString(const CString &strValueName, CString &strValue);

CString GetSharedPath(const CString& strDatabase);
const CString &GetSharedPath(void);
const CString &RecalcSharedPath();
void SetSharedPath(const CString &strNewSharedPath);

// Some handy COM functions
// (a.walling 2010-04-30 17:12) - PLID 38553 - Part of NxException
//inline void HR(HRESULT hr) {	if (FAILED(hr)) { _com_issue_error(hr); } }
LPUNKNOWN GetDlgItemUnknown(CWnd *pDlg, IN int nIdDlgItem);

// For using normal strings that are going to be put into quotes
// (a.walling 2010-04-30 17:13) - PLID 38553 - Part of GlobalDataUtils
//CString ConvertToQuotableString(LPCTSTR lpstrString, bool bDoubleQuote = true);
//#define _Q(str)		ConvertToQuotableString(str, false)

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString ConvertToQuotableXMLString(LPCTSTR lpstrString);

// (a.walling 2012-07-03 11:17) - PLID 51263 - Moved ConvertToControlText / ConvertFromControlText to globaldatautils
// For strings that will be inserted into an html document.

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString ConvertToHTMLEmbeddable(const CString &str);

//used to determine whether a PersonID can be deleted from PersonT
#define DP_PATIENTST       0x0001
#define DP_CONTACTST       0x0002
#define DP_PROVIDERST      0x0004
#define DP_REFPHYST        0x0008
#define DP_REFERRALSOURCET 0x0010
#define DP_USERST		   0x0020
#define DP_INSURANCECOT    0x0040
#define DP_INSUREDPARTYT   0x0080
#define DP_SUPPLIERT       0x0100
// (a.walling 2010-01-22 14:29) - PLID 37038 - New flag, which keeps the PersonT record but as archived
#define DP_ARCHIVEPATIENT  0x8000

#define DP_ALL				(DP_USERST|DP_PATIENTST|DP_CONTACTST|\
							DP_PROVIDERST|DP_REFPHYST|DP_SUPPLIERT|\
							DP_INSURANCECOT|DP_INSUREDPARTYT|DP_REFERRALSOURCET)

//functions to determin whether a PErsonID can be deleted from the PersonT table
CString GetPersonTableName(UINT nPersonType);
BOOL IsPersonInTable(long nPersonId, UINT nPersonType);
bool DeletePerson(long nPersonID, UINT nFlags);
bool DoesPersonExist(long nPersonID, UINT nFlags);

/*
//TS:  define RCS to do what MAKEINTRESOURCE does
#ifdef UNICODE
#define RCS(i) (LPWSTR)((ULONG_PTR)((WORD)(i)))
#else 
#define RCS(i) (LPSTR)((ULONG_PTR)((WORD)(i)))
#endif */
#define RCS MAKEINTRESOURCE

//TS: function to get a string from an IDS_
CString GetStringOfResource(int nResource);

// Returns the IUnknown * (NULL on failure) of the child of pParent with a ControlID of nID
// This function automatically sets the NxDataList's AdoConnection pointer to pDataConn and 
// then optionally requeries based on that connection and the NxDataList's current properties
LPUNKNOWN BindNxDataListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery);
//DRT 12/08/2005 - Added to bind Version 2 NxDataListCtrl
LPUNKNOWN BindNxDataList2Ctrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery);

//same as the NxDataList one, except it doesn't requery or take a data connection
LPUNKNOWN BindNxTimeCtrl(CWnd *pParent, UINT nID);

//Converts strIn into a VARIANT, varOut.
inline void SetVariantString(VARIANT &varOut, LPCTSTR strIn)
{ 
	_variant_t varTemp; 
	varTemp.SetString(strIn); 
	varOut = varTemp.Detach();
}

// This is a general COM function that sets the first pointer equal to the second one.  But it 
// does correct reference counting through the transition.  It's a templated class, so it's 
// automatically implemented by the compiler for whatever class needs it, as long as the class 
// has public AddRef() and Release() functions.
template<class com_type> inline void SafeSetCOMPointer(OUT com_type **ppSetMe, IN com_type *pEqualToMe)
{ 
	if (pEqualToMe) {
		pEqualToMe->AddRef();
	}
	
	if (*ppSetMe) {
		(*ppSetMe)->Release();
	} 

	*ppSetMe = pEqualToMe;
}

// This is a general COM function that empties an array of COM pointers.  Since COM pointers 
// are reference-counted, they always have to have Release() called on them before they are 
// lost, so we loop through and for any non-NULL element we call Release().  When all done we 
// just call the standard RemoveAll() on the array.
// (b.cardillo 2008-07-17 16:41) - This was originally created for use by plid 30745 but is a 
// general utility function useful for any similar need.
template<class com_pointer_type> inline void SafeEmptyArrayOfCOMPointers(IN OUT CArray<com_pointer_type, com_pointer_type> &ary)
{
	// Release all non-NULL elements.
	for (long i=0, nCount=ary.GetSize(); i < nCount; i++) {
		com_pointer_type lpCur = ary.GetAt(i);
		if (lpCur) {
			lpCur->Release();
		}
	}
	// Empty the array
	ary.RemoveAll();
}

// Tries to delete the given file, and if the attempt 
// fails and the file exists, it is queued for deletion later
// If the mainframe window is unavailable, calls MoveFileAtStartup
// Return value indicates whether the file exists upon completion
BOOL DeleteFileWhenPossible(LPCTSTR strFilePathName);

//does not attempt to delete a file until nDelayInMS has elapsed
BOOL DeleteFileWhenPossible(LPCTSTR strFilePathName, long nDelayInMS);

// (a.walling 2012-03-12 17:33) - PLID 48839 - Moved COleCurrency operators * and / for doubles into GlobalDataUtils

// Takes a double and returns a long; just like the (long) cast, except this rounds instead of truncating.
inline 
long RoundAsLong(double dbl)
{
	ASSERT(dbl > LONG_MIN && dbl < LONG_MAX);
	if (dbl >= 0) {
		return (long)(dbl + 0.5);
	} else {
		return (long)(dbl - 0.5);
	}
}

////////////////////////////////////////
//TS:  This is a semi-ugly way of calling this function, which has nothing to do with Practice per se,
//	   but needs to use the ReportInfo object to loop through all the reports.  This function, if called,
//     is called in the InitInstance for the program, and it returns false immediately after.
void CreateAllTtxFiles(long nID);
// (r.gonet 10/16/2011) - PLID 45967 - Added the ability for a report to verify per custom report.
BOOL CreateAllTtxFiles(long nID, CString strPath, long nCustomReportID = -1);

//This function takes a data list and returns a comma-delimited string of all the values in column 0 (the IDs, presumably)
void BuildIDList(NXDATALISTLib::_DNxDataListPtr m_pBatchList, CString& strIDList);
// (r.galicki 2008-11-05 12:00) - PLID 27214 - Added for Datalist2, as well as specify columns
void BuildIDList(NXDATALIST2Lib::_DNxDataListPtr m_pBatchList, CString& strIDList, short nCol = 0);

//this function is used in contactsgeneral and newcontactsdlg
void CopyPermissions(int source, int dest);

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

//is there a call coming in right now?  
extern bool m_bIncomingCall;
extern CString m_strIncomingCallNum;

void DeleteProcInfo(long nProcInfoID);
void DeleteLadder(long nLadderID, long nPatID); // (a.walling 2008-07-07 17:59) - PLID 29900 - Required patient ID

void DeleteCustomRecord(long EMRID);
void DeleteEMN(long EMNID);

//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL AttemptRemovePICRecord(long nPicID, IN OUT CDWordArray &arNewCDSInterventions, BOOL bSilent = FALSE);
BOOL DeleteNexEMR(long nEMRGroupID, IN OUT CDWordArray &arNewCDSInterventions);


//these functions check the license file for the right version
bool IsPlastic();
bool IsReproductive();
bool IsRefractive();
bool IsNexTechInternal();
bool IsSurgeryCenter(BOOL bMakingUseOfLicense);
bool IsSpa(BOOL bMakingUseOfLicense);

bool IsSupportPaidFor();
bool GetAppointmentLinkingAllowed();
COleDateTime GetSupportExpirationDate();

//Force the text of an edit box to be no greater than the specified length,
//put this is in a killfocus handler, and either the field will be truncated to the specified length,
//or the focus will be set back to the edit box
//returns true if the focus has been changed, false if the box still has focus (the field is still too long).
// (c.haag 2009-03-10 13:05) - PLID 30019 - We need return values for SendEmail to tell us if it worked
bool ForceFieldLength(CEdit * pEditBox, int nLength = 2500);
BOOL SendEmail(CWnd *pParent, LPCTSTR strTo, LPCTSTR strSubject, LPCTSTR strText, CString strFileAttachment = ""); //Zero or one attachment
BOOL SendEmail(CWnd *pParent, LPCTSTR strTo, LPCTSTR strSubject, LPCTSTR strText, CStringArray &strFileAttachments); //Multiple attachments
// (z.manning, 01/10/2008) - PLID 28594 - Need to be able to specify multiple recipients.
BOOL SendEmail(CWnd *pParent, CStringArray &arystrTo, LPCTSTR strSubject, LPCTSTR strText, CString strFileAttachment = "");
BOOL SendEmail(CWnd *pParent, CStringArray &arystrTo, LPCTSTR strSubject, LPCTSTR strText, CStringArray &strFileAttachments);


// (a.walling 2010-08-04 09:20) - PLID 38964 - I got rid of GetErrorHelpLocation and GetErrorHelpBookmark since they were not used.

//CAH: Like CFileDialog::DoModal() but you can browse folders
// (v.maida 2016-06-02 17:53) - NX-100805 - Added option bAllowNewFolder option, which defaults to true since this function used to always provide that option.
BOOL BrowseToFolder(CString* strSelectedFolder,
				   const char* lpszTitle,
				   const HWND hwndOwner, 
				   const char* strRootFolder, 
				   const char* strStartFolder,
				   const bool bAllowNewFolder = true);

// This function will invoke a dialog that will prompt
// a user to enter their name and password to enter a
// network resource. If strNetworkPath is not a network
// resource, the function will return ERROR_BAD_NET_NAME.
// If you are already connected to the the network
// resource, the dialog will still be invoked, and the
// return value will be ERROR_SESSION_CREDENTIAL_CONFLICT
// regardless of if you have access to that resource.
// If you have not connected to the resource yet, the result
// will be ERROR_ACCESS_DENIED or NO_ERROR, or another value
// which is returned from the function WNetAddConnection2.
// If the user cancells the dialog, the return value is ERROR_CANCELLED.
//DWORD DoNetworkLogin(const CString& strNetworkPath);
// (c.haag 2006-05-03 11:17) - PLID 4612 - DoNetworkLogin has been superceded
// by a superior function
DWORD TryAccessNetworkFile(const CString& strFileName);

//ActiveUpdate functions.
CString GetFirstCdRom();
bool CopyDirectory(	LPCSTR strToPath, LPCSTR strFromPath, HWND progress = NULL);
//Look, I know this function only has one letter, and that's ridiculous, but I copied it out of the setup.
char * V (char * directory, LPCSTR file);
__int64 CheckSum(CString &filename);


//Datalist helper
OLE_COLOR GetLighterColor(OLE_COLOR colDarkColor);

// Returns TRUE if we are in 640x480
BOOL IsScreen640x480();

// Returns the number of images attached to the patient's history
int GetPatientAttachedImageCount(long nPatientID);

// Returns the attached patient document
//TES 6/1/2004: Modified this to return the THUMBNAIL version of the image.
HBITMAP LoadPatientAttachedImage(long nPatientID, long nIndex, CWnd* pImageProgressCtrl, /*OUT*/ CString& strFileName);

// (c.haag 2003-10-24 10:21) - Adds an entry to the mail sent table. There are
// a lot of places that call "Insert into MailSent" and I think there should be
// some sort of consolidation.
// (m.hancock 2006-06-27 15:37) - PLID 21071 - Added LabStepID to associate MailSent records with lab steps
// (a.walling 2008-09-03 11:39) - PLID 19638 - Added PicID to associate with PICs/EMNs
void AttachToMailSent(long nPersonID, const CString &strPathName, BOOL bAttachChecksum = FALSE, long nChecksum = 0, LPCTSTR strSelection = "BITMAP:FILE", long nCategoryID = -1, long nLabStepID = -1, long nPicID = -1);

// a.walling 4/20/2006 Retrive default category for this template if not preceded by other preferences. NULL if none found.
long GetTemplateCategory(const CString &templateName);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some helpful utilities for working with device settings (mainly for printers)
////////////////////////////////////////////////////////////////////////////////

// Copy from one DEVMODE object to another (make sure pDevModeOut points to (sizeof(*pDevModeIn) + pDevModeIn->dmDriverExtra) bytes)
void CopyDevMode(IN const DEVMODE *pDevModeIn, OUT DEVMODE *pDevModeOut);

// Allocates a copy of the given device settings (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
void AllocCopyOfDeviceSettings(IN HGLOBAL hDevNames, IN HGLOBAL hDevMode, OUT DEVMODE **ppDevMode, OUT LPTSTR *ppstrPrinter, OUT LPTSTR *ppstrDriver, OUT LPTSTR *ppstrPort);

// Allocates a copy of the given CPrintInfo object's device settings (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
void AllocCopyOfDeviceSettings(IN CPrintInfo *pInfo, OUT DEVMODE **ppDevMode, OUT LPTSTR *ppstrPrinter, OUT LPTSTR *ppstrDriver, OUT LPTSTR *ppstrPort);

// Allocates a copy of THIS APPLICATION's current device settings (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
void AllocCopyOfAppDeviceSettings(OUT DEVMODE **ppDevMode, OUT LPTSTR *ppstrPrinter, OUT LPTSTR *ppstrDriver, OUT LPTSTR *ppstrPort);

// Free device settings objects (one DEVMODE object and the strings for each of the device names (printer(device), driver(driver), and port(output))
// Pass the addresses of your variables, that way this function can clear your variables after freeing the memory that your variables point to
void FreeCopyOfDeviceSettings(IN OUT DEVMODE **ppDevMode, IN OUT LPTSTR *ppstrPrinter, IN OUT LPTSTR *ppstrDriver, IN OUT LPTSTR *ppstrPort);

// Allocates a copy of the given DEVMODE object
// Returns a handle to the globally allocated memory, make sure you store the return value, so that you can call GlobalFree on it later
HGLOBAL AllocDevModeCopy(IN const DEVMODE *pDevModeIn);

// Allocates a DEVNAMES object, filling it with the given strings (printer(device), driver(driver), and port(output))
// Returns a handle to the globally allocated memory, make sure you store the return value, so that you can call GlobalFree on it later
HGLOBAL AllocDevNamesCopy(IN LPCTSTR strPrinter, IN LPCTSTR strDriver, IN LPCTSTR strPort);

//takes in an array of field names, and a recordset source, and returns an XML markup of the information
CString CreateXML(CStringArray &aryFieldNames, ADODB::_RecordsetPtr pRS, CString strKeyFieldName = "", BOOL bFilterNulls = FALSE);

//creates a temp table from an XML string, and returns the new table name
// (j.jones 2008-10-01 17:09) - PLID 31563 - added optional pstrSqlBatch parameter to return
// SQL statements in batch form, instead of executing immediately
CString CreateTempTableFromXML(CStringArray &aryFieldNames, CStringArray &aryFieldTypes, CString strXML, CString *pstrSqlBatch = NULL);
//Appends to a table created by CreateTempTableFromXML
// (j.jones 2008-10-01 17:09) - PLID 31563 - added optional pstrSqlBatch parameter to return
// SQL statements in batch form, instead of executing immediately
void AppendToTempTableFromXML(CStringArray &aryFieldNames, CStringArray &aryFieldTypes, CString strXML, const CString &strTable, CString *pstrSqlBatch = NULL);

///////////////////
//registry settings
static CString g_strRegistryBase = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nextech\\";

CString GetRegistryBase();
CString GetRegistryBase(CString strDatabase);
void SetRegistryBase(CString strBase);
CString GetSubRegistryKey();
CString GetCurrentDatabaseName();
CString GetPurgeDatabaseName(bool &bCreatedNew, bool bCreateIfNeeded = true);
int ReplaceWholeWords(IN OUT CString &strString, IN const CString &strReplaceText, IN const CString &strWithText);
int FindWholeWord(IN const CString &strString, IN const CString &strFind, IN long nStart);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// United Imaging functions
BOOL IsUnitedEnabled();

// cuts strEndingString off the end of strString
// for example if strString is passed in as "This is the test" and strEndingString = " test" 
// then strString will be shortened to "This is the"
// if strString is empty, then nothing is done to the strString, but if strString doens't end with strEndingString, 
// then an exception is thrown
void DeleteEndingString(IN OUT CString &strString, IN const CString &strEndingString);


// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString FormatCurrencyForSql(const COleCurrency &cy);
//COleCurrency ParseCurrencyFromSql(const CString &strCy);


// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//enum DateTimeOptions;
//CString FormatDateTimeForSql(const COleDateTime &dt);
//CString FormatDateTimeForSql(const COleDateTime &dt, DateTimeOptions dto);

// compares two COleDateTimes ignoring the time portion, returns 0 if equal, positive if dtDateOne is greater,
// negative if dtDateOne is less
long CompareDateIgnoreTime(const IN COleDateTime &dtDateOne, const IN COleDateTime &dtDateTwo);

// compares two COleDateTimes both of which should not have time, returns 0 if equal, positive if dtDateOne is greater,
// negative if dtDateOne is less
long CompareDates(const IN COleDateTime &dtDateOne, const IN COleDateTime &dtDateTwo);


// You can pass a handle to a sync object (like a mutex or thread or whatever) and it will pump app messages as it waits
// Note, it also tries to have the app's current mouse cursor be the hourglass
BOOL WaitForSingleObjectAppModal(HANDLE hObject);

// Spawns an app (like notepad.exe or whatever) and waits for the app to be closed before returning
// Returns >= 32 on success, less than 32 indicates an error code returned by the ShellExecute API function
// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
int ShellExecuteAppModal(HWND hWndParent, IN LPCTSTR strFileToExecute, IN OPTIONAL const CString &strParameters = "", IN OPTIONAL LPCTSTR strWorkingPath = NULL, IN OPTIONAL LPCTSTR strOperation = NULL);

// (c.haag 2003-07-28 11:23) - Filters a datalist. The function will prompt the
// user for a set of keywords, and then filters the list where the text in nCol
// contains all the keywords. nIDCol must be the column containing a unique record
// ID for each record. Returns true if at least one record returned.
BOOL FilterDatalist(NXDATALISTLib::_DNxDataListPtr& pList, short nCol, short nIDCol);
// (j.jones 2009-03-23 16:09) - PLID 33639 - added a datalist 2 version of this function
BOOL FilterDatalist2(NXDATALIST2Lib::_DNxDataListPtr& pList, short nCol, short nIDCol);
//TES 12/10/2010 - PLID 40879 - Added a Datalist2 version that takes multiple columns.  If the search term appears in any of the columns,
// the row will be included.
BOOL FilterDatalist2(NXDATALIST2Lib::_DNxDataListPtr& pList, const CArray<short,short> &arColumns, short nIDCol);

// (c.haag 2003-07-28 16:37) - Prints a file given the path. Returns TRUE on success.
// Throws an exception on error. Right now, only word documents are supported.
BOOL PrintFile(const CString& strFilePath);

// Returns an HCURSOR that will give you the IDC_HAND cursor even on older versions of Windows that didn't have the hand as a stock cursor
HCURSOR GetLinkCursor();

// (a.walling 2010-06-09 16:03) - PLID 39087 - ShowDlgItem now in CNexTechDialog's NxUI library object

// If hWnd, its parent, its parent's parent, or any window on up the line (i.e. hWnd or any of its ancestors), is equal to hWndAncestor then this returns TRUE.
// If hWnd or hWndAncestor is NULL, or if none of hWnd's ancestors is equal to hWndAncestor, then this returns FALSE.
BOOL IsOrHasAncestor(HWND hWnd, HWND hWndAncestor);

//TES 3/17/2004: Returns the UNC path to this patient's Documents folder.
// (j.gruber 2010-04-22 13:53) - PLID 38262 - added Silent Parameter
CString GetPatientDocumentPath(long nPatientID, BOOL bSilent = FALSE);
//TES 9/18/2008 - PLID 31413 - The other GetPatientDocumentPath() overload was moved to HistoryUtils (which is shared with NxServer)

// One, both, or neither of these flags may be passed into the nFlags parameter of the 
// IsMessageInQueue function.  This is how you tell the function to compare the lParam 
// and/or wParam parameters of the given message to the values passed in.
#define IMIQ_MATCH_WPARAM	0x0001
#define IMIQ_MATCH_LPARAM	0x0002

// Uses PeekMessage to find the given message in the queue; optionally (depending on 
// what you pass in for nFlags) matches wParam and lParam to the message in the queue.
// If hWnd is NULL, this considers messages for any window that belongs to the current 
// thread. If hWnd is INVALID_HANDLE_VALUE, it only considers messages whose hWnd value 
// is NULL, as posted by the PostThreadMessage function.
// *** NOTE: If you are comparing wParam and/or lParam, be aware that it simply compares 
// the values to the FIRST nMessage message in the queue.  So, for example, if the user 
// hits the tab key and the enter key and you are searching for WM_KEYDOWN of VK_ENTER, 
// you will not get a match because the wParam of the FIRST WM_KEYDOWN will be VK_TAB.
BOOL IsMessageInQueue(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam, UINT nFlags);

//TES 9/18/2008 - PLID 31413 - Moved REPLACE_INVALID_FILENAME_CHARS to HistoryUtils

//TES 9/18/2008 - PLID 31413 - Moved EnsurePatientDocumentPath() to HistoryUtils

//Used when attaching photos to determine whether they've already been attached.
long BitmapChecksum(const LPBITMAPINFO pDib);

//Used when attaching from scanner/camera to check whether the file has already been imported.
// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
void WINAPI CALLBACK OnNxTwainPreCompress(const LPBITMAPINFO pDib, /* The image that was scanned */	BOOL& bAttach, BOOL& bAttachChecksum, long &nChecksum, ADODB::_Connection* lpCon);

// (r.gonet 2016-05-24 14:21) - NX-100623 - This returns the username for the license agreement prompt ConfigRT setting in the format of:
//win user=user
CString GetLicenseAgreementUsername();
//This returns an advanced username in the format of:
//COMPUTER|working path|win user=user
CString GetAdvancedUsername();

void LaunchCodeLink(HWND hwnd);
//DRT 4/19/2007 - PLID 25598 - Removed
//CString GetCurrentAMACodeVersion();

//TES 9/18/2008 - PLID 31413 - Moved MakeValidFolderName() to HistoryUtils

// (a.walling 2010-05-17 12:45) - PLID 34056 - Returns success/failure. Must be called prior to actually changing the data. bAllowContinue can be set to false to not allow them to continue without moving the folder.
bool EnsureCorrectHistoryFolder(CWnd* pWnd, EChangedHFItem item, const CString& strNewValue, long nPersonID, bool bAllowContinue);
// (a.walling 2010-05-17 12:45) - PLID 34056 - Unchanged (used only in UnitedLink)
void EnsureCorrectHistoryFolder(const CString& strUserDefinedID, const CString& strFirst,
								const CString& strLast, long nPersonID);


// Handy way of getting a decent spot to pop up a context menu, given the parameters passed 
// by Windows's WM_CONTEXTMENU message.
CPoint CalcContextMenuPos(CWnd *pWnd, CPoint pos);

//effectively calculates the height of a single-line datalist row
long CalcSysIconTitleFontHeight();
//similarly, this will calculate the width of text using the same font
long CalcSysIconTitleFontWidth(CString str);

// (a.walling 2011-11-11 11:11) - PLID 46622
inline long GetControlBufferWidth() 
{
	return 2;
}

// (a.walling 2011-11-11 11:11) - PLID 46622
inline CSize GetControlBufferSize() 
{
	return CSize(2, 2);
}

// Given a control, usually a checkbox or radio button, this calculates the size the control 
// should be set to in order to fit its contents.
// (a.walling 2011-11-11 11:11) - PLID 46622 - Pass the buffer sizes
void CalcControlIdealDimensions(CDC* pdc, CWnd *pControl, IN OUT CSize &sz, BOOL bAllowMultiLine = FALSE, const CSize& szBuffer = GetControlBufferSize());

// Sets pszTotal to contain the amount of width (cx) and height (cy) taken up by the non-client 
// area of the given window.  Sort of the total window area minus the client's part of the area.
// Put another way, this
// sets pszTotal->cx equal to the sum of the left border width and the right border width, and 
// sets pszTotal->cy equal to the sum of the top border height and the bottom border height.
void CalcWindowBorderSize(IN CWnd *pWnd, OUT CSize *pszTotal);

// Given a window and an expected total window rect, this alters the rect to be what 
// the client rect would be (the window without borders, titles, scrollbars, etc.).
void CalcClientFromWindow(IN CWnd *pWnd, IN OUT CSize *psz);
// Given a window and an expected total window size, this alters the size to be what 
// the client size would be (the window without borders, titles, scrollbars, etc.).
void CalcClientFromWindow(IN CWnd *pWnd, IN OUT LPRECT prc);

//TES 10/26/2007 - PLID 24831 - Object returned by AddFolderToList, if called asynchronously.  Its destructor ensures
// that the thread terminates, thus ensuring that there will be no more accesses to the passed-in datalist or window
// after this object is destroyed.
struct AddFolderToListInfo;
class CAddFolderToListThread {
public:
	CAddFolderToListThread(CWinThread *pThread, HANDLE hevQuit);
	~CAddFolderToListThread();
protected:
	CWinThread *m_pThread;
	HANDLE m_hevQuit;
};

//Recurses through the given folder, adds each file to the list, assumes two BSTR columns, the first for the filename,
//the second for the folder.
//TES 6/16/2006 - Pass in a pNotifyWnd, and this function will load asynchronously, and post a NXM_ADD_FOLDER_COMPLETED message
//to that window once it's done; otherwise it will load synchronously.  If you pass in a pNotifyWnd, you must also read out
//the returned CWinThread*, because it is your responsibility to clean up that thread.
//TES 10/26/2007 - PLID 24831 - This has been all wrong from the beginning.  This function now will return a 
// CAddFolderToListThread object, it is the caller's responsibility (if it passes in a pNotifyWnd), to destroy that 
// object BEFORE destroying the window or datalist that was passed in.
CAddFolderToListThread* AddFolderToList(const CString &strFolderName, NXDATALISTLib::_DNxDataListPtr pList, bool bPrePendSharedPath = true, CWnd *pNotifyWnd = NULL);

//finds text in a datalist, case-insensitive
int FindTextInDataList(NXDATALISTLib::_DNxDataListPtr pList, int iColumn, CString strText, int iStartRow = 0, bool bAutoSelect = false);

// (r.gonet 07/09/2012) - PLID 50708 - Removed VariantCompare utility functions to the GlobalDataUtils.cpp
//  Took into account a.walling's modification for the VT_UI1|VT_ARRAY variants.

void RemoveEntryFromSubMenu(CMenu *pRootMenu, LPCTSTR strSubMenuLabel, UINT nIDEntry, BOOL bRemovePrecedingSeparatorIfExists);

CString GenerateDelimitedListFromLongArray(IN const CDWordArray &arydw, IN const CString &strDelimiter = ";");
CString GenerateDelimitedListFromStringArray(IN const CStringArray &arystr, IN const CString &strDelimiter = ";");
CString GenerateDelimitedListFromDatalistColumn(IN NXDATALISTLib::_DNxDataList *lpdl, IN const short nColIndex, IN const CString &strDelimiter = ";");
// (b.cardillo 2009-07-22 12:26) - Originally created this more generic form of GenerateDelimitedListFromRecordsetColumn() 
// which takes a recordset, and prefix/suffix instead of just delimiter, for use in PLID 34844
CString GenerateDelimitedListFromRecordsetColumn(IN ADODB::_Recordset *lprs, IN const _variant_t &varField, IN const CString &strListEntryPrefix = "", IN const CString &strListEntrySuffix = ";");
CString GenerateDelimitedListFromRecordsetColumn(IN const CString &strSql, IN const _variant_t &varField, IN const CString &strDelimiter = ";");

// (c.haag 2009-01-19 12:15) - PLID 32712 - Made pArray const
BOOL IsIDInArray(DWORD dwIDToFind, const CDWordArray *pArray);
// (c.haag 2007-01-25 13:03) - PLID 22420 - New overload for IsIDInArray. We pass
// in an array reference so it's almost impossible to have an invalid array when 
// this function is called.
BOOL IsIDInArray(long nIDToFind, const CArray<long,long>& ary);
//DRT 6/6/2008 - Created for use in PLID 30306
bool IsStringInStringArray(CString strText, CStringArray *pary);
BOOL IsDateTimeInDateTimeArray(const COleDateTime dtToFind, CArray<COleDateTime> *parydt); // (z.manning 2011-12-07 16:19) - PLID 46906

//DRT 1/23/2008 - PLID 28603
void ParseDelimitedStringToDWordArray(const IN CString strToParse, const IN CString strDelim, OUT CDWordArray &aryToFill);
// (c.haag 2008-06-03 09:46) - PLID 30221 - Parses a delimited string to a long array
void ParseDelimitedStringToLongArray(const IN CString strToParse, const IN CString strDelim, OUT CArray<long,long> &aryToFill);
// (j.luckoski 2012-11-12 15:10) - PLID 53704 - Parses a delimited string into a string array
void ParseDelimitedStringToStringArray(const IN CString strToParse, const IN CString strDelim, OUT CStringArray &aryToFill);

// (j.armen 2014-10-08 14:22) - PLID 63229 - Templated function allows you to parse many different types (std::string, long, etc...)
template<typename T>
std::vector<T> ParseDelimitedString(const CString& strToParse, const CString& strDelim, bool bTrim = true);

//TES 5/7/2008 - PLID 29685 - Moved some CString optimization functions into GlobalStringUtils, which can be shared outside of
// Practice.

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - lots of stuff moved to NxAdo

struct FileFolder
{
	CString strFolderName;
	CStringArray saFileNames;
	CArray<FileFolder,FileFolder&> arSubFolders;

	FileFolder(FileFolder &ffSource) {
		strFolderName = ffSource.strFolderName;
		// (a.walling 2007-11-05 17:44) - PLID 27977 - for() loops
		int i = 0;
		for(i = 0; i < ffSource.saFileNames.GetSize(); i++) {
			saFileNames.Add(ffSource.saFileNames.GetAt(i));
		}
		for(i = 0; i < ffSource.arSubFolders.GetSize(); i++) {
			arSubFolders.Add(ffSource.arSubFolders.GetAt(i));
		}
	}
	void operator =(FileFolder &ffSource) {
		strFolderName = ffSource.strFolderName;
		// (a.walling 2007-11-05 17:44) - PLID 27977 - for() loops
		int i = 0;
		for(i = 0; i < ffSource.saFileNames.GetSize(); i++) {
			saFileNames.Add(ffSource.saFileNames.GetAt(i));
		}
		for(i = 0; i < ffSource.arSubFolders.GetSize(); i++) {
			arSubFolders.Add(ffSource.arSubFolders.GetAt(i));
		}
	}
	FileFolder() {}
};

// (j.dinatale 2011-09-19 10:38) - PLID 45548 - attempts to copy the file over to the path given
// (b.savon 2012-05-09 11:31) - PLID 50260 - Propogate the silent flag
bool TryToCopyToHistory(const CString &strSourcePath, const CString &strDestPath, long nPatientID, HWND hwndMessageParent, long nCategoryID,
						CString &strIcon, long nPicID, LPCTSTR strNote, COleDateTime dtServiceDate, OUT CString *pstrNewFileName, 
						OUT long *pnMailSentID, IN OUT DWORD &dwLastError, bool bAttemptSilent = false);

// (j.dinatale 2011-09-19 10:37) - PLID 45548 - gets the file's icon
CString GetFileIcon(CString &strFilePath);

//Recursive function, imports and attaches the given "folder" (a memory structure, not an actual folder).
//Returns success or failure.
// (j.dinatale 2011-09-19 14:16) - PLID 45548 - refactored
// (j.gruber 2010-01-27 08:58) - PLID 23693 - added bool for if in patients module
// (j.jones 2010-11-02 09:19) - PLID 41188 - added strNote, to override the note for the attached file
// (j.jones 2010-12-29 15:41) - PLID 41943 - added optional OUT parameter for the new file name, in the case that the name changed
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate (optional)
// (d.lange 2011-05-25 14:56) - PLID 43253 - added OUT parameter for returning the mailsent ID
// (b.savon 2012-05-09 12:32) - PLID 50260 - Propogate the silent flag
BOOL ImportAndAttachFolder(FileFolder &Folder, long nPatientID, HWND hwndMessageParent, long nCategoryID = -1,
						   const CString &strRootDir = "", long nPicID = -1, BOOL bCheckForImageCategory = FALSE,
						   LPCTSTR strNote = "", OUT CString *pstrNewFileName = NULL,
						   OPTIONAL IN COleDateTime dtServiceDate = g_cdtInvalid, OUT long *pnMailSentID = NULL,
						   bool bAttemptSilent = false);

//Writes the given "folder" to disk at the specified location, prompts for overwrites.
void WriteFolderToDisk(FileFolder Folder, const CString &strDestDir);

//TES 9/18/2008 - PLID 31413 - Moved EnsureDirectory() to FileUtils

// (d.singleton 2013-11-15 10:00) - PLID 59513 - need to insert the CCDAType when generating a CCDA
typedef enum {
	ctNone = 0,
	ctSummaryOfCare,
	ctClinicalSummary,
} CCDAType;

// (j.dinatale 2011-09-20 12:06) - PLID 45514 - added attempt silent flag
// (j.gruber 2010-01-27 09:01) - PLID 23693- added bool to check for image category check
// (j.jones 2010-10-29 12:02) - PLID 41187 - added nPicID
// (j.jones 2010-11-02 09:19) - PLID 41188 - added strNote, to override the note for the attached file
// (j.jones 2010-12-29 15:41) - PLID 41943 - added optional OUT parameter for the new file name, in the case that the name changed
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate (optional)
// (d.lange 2011-05-25 14:56) - PLID 43253 - added OUT parameter for returning the mailsent ID
BOOL ImportAndAttachFileToHistory(const CString &strFileName, long nPatientID, HWND hwndMessageParent,
								  long nCategoryID = -1, long nPicID = -1, BOOL bCheckImageCategory = FALSE,
								  LPCTSTR strNote = "", OUT CString *pstrNewFileName = NULL,
								  OPTIONAL IN COleDateTime dtServiceDate = g_cdtInvalid, OUT long *pnMailSentID = NULL, bool bAttemptSilent = false);

// (j.jones 2010-11-02 09:19) - PLID 41188 - added strNote, to override the note for the attached file
// (j.jones 2011-03-10 16:26) - PLID 42329 - added dtServiceDate (optional)
// (b.savon 2012-05-09 11:14) - PLID 50260 - Propogate the silent flag
long AttachFileToHistory(const CString &path, long nPatientID, HWND hwndMessageParent, long nCategoryID = -1, LPCTSTR strSelection = SELECTION_FILE, LPCTSTR strPDAFilename  = NULL,
						 long nPicID = -1, long nLabStepID = -1, LPCTSTR strNote = "",
						 OPTIONAL IN COleDateTime dtServiceDate = g_cdtInvalid, bool bAttemptSilent = false);

BOOL IsWaveFile(const CString &strFullPath);
// (m.hancock 2006-10-16 09:14) - PLID 23053 - Determines if the passed file is a word document.
BOOL IsWordFile(const CString &strFullPath);
// (a.wetta 2007-07-09 17:09) - PLID 17467 - Determines if the document is an image file
BOOL IsImageFile(const CString &strFullPath);
// (a.wetta 2007-07-09 17:09) - PLID 17467 - Determines if the given file is of the given file type
BOOL IsFileType(const CString& strFullPath, const CString& strFileType);
// (a.wetta 2007-07-10 08:44) - PLID 17467 - Determines if the given file should be added to the history tab as a photo
// (a.walling 2010-02-01 12:30) - PLID 28806 - Added bSilent and pbPrompt values to determine whether the user intends to be prompted
// Will return FALSE if bSilent and prompt is required.
BOOL IsHistoryImageAPhoto(const CString& strPathName, bool bSilent = false, bool* pbPrompt = NULL);

// (a.walling 2012-12-04 16:24) - PLID 54041 - Tries to determine if this is a complete file (eg, not truncated). Returns true if it can't figure it out.
bool ValidateCompleteFile(const CString& strFileName);

FileFolder LoadFolder(CString strFolder);

void GetFileNameList(FileFolder fFiles, CString &strMessage, const CString &strRoot, int &nFilesAdded, int nMaxFiles);

CString GetLatinToEnglishConversion(CString strLatinDescription);
// (j.jones 2010-05-07 10:35) - PLID 36062 - we removed this ability, we should never be changing existing medications
//void UpdateMedicationsWithLatinToEnglishConversion();
//TES 2/10/2009 - PLID 33002 - This turns out to be dead code.
//void UpdateBlankMedicationsWithLatinToEnglishConversion();

//Loads the given filename into hImage, returns success or failure (hImage is undefined on failure).
bool LoadImageFile(CString &strFile, HBITMAP &hImage, long nPersonID);

// Returns true if the window is a modal dialog
bool IsWindowModal(HWND hwnd);

//creates to-do reminders for persons with soon-to-expire ASC licensing
void UpdateASCLicenseToDos();

typedef enum {
	ePassedAll = 0L,	//these are deliberately in order of importance, least to most
	eFailedLicenseExpiringSoon,
	eFailedLicenseExpired,
	eFailedCredential,
} ECredentialWarning;

ECredentialWarning CheckServiceCodeCredential(long nProviderID, long nServiceCodeID);
ECredentialWarning CheckProcedureCredential(long nProviderID, long nProcedureID);
ECredentialWarning CheckPersonCertifications(long nPersonID);

_variant_t GetTableField(const CString &strTable, const CString &strField, const CString &strIDField, long nID);

//TES 5/4/2005 - Just like FormatItemText, except if there are more numbers than needed, it trims from the left.
CString FormatNumericText(const CString &strIn, const CString &strFormat);
//EML 9/20/2007 - Calls FormatNumbericString using no default to force users to pick U.S. or International
CString FormatPhone(const CString &strPhone, const CString &strFormat);
//EML 9/20/2007 - Calls FormatNumericString just like formatPhone. Sends in a default format as an optional parameter.
CString FormatSSN(const CString &strSSN, OPTIONAL const CString &strFormat = "###-##-####");

//TES 6/22/2005 - This previews the given quote, it's here because it's called both from the billing module dialog
//and the quotes tab.
// (j.gruber 2007-08-14 11:04) - PLID 27068 - added ReportID since we have more then one now
void PreviewQuote(long nQuoteID, CString strExtraDesc, bool bIsPackage, CWnd *pParent, long nReportID, long nReportNumber = -2, CString strReportFileName = "");
void PreviewBill(long nBillID, CWnd *pParentWnd);

//TES 10/15/2008 - PLID 31646 - Moved the TodoType enum to GlobalTodoUtils.h

// (c.haag 2005-06-28 12:08) - Run the troubleshooter exe
BOOL RunTroubleshooter();

// (j.jones 2005-07-27 15:16) - migrated from the history tab
BOOL OpenDocument(CString filename, long nPatientID);
// (z.manning 2016-02-11 09:49) - PLID 68223 - Overload to simple open a file without the patient logic
BOOL OpenDocument(CString filename);

//TES 9/20/2005 - Check if the patient is allergic to the given medication.  If they are, and bSilent is FALSE, then it will
//prompt them whether they want to continue, and return TRUE if they do and FALSE if they don't.  If bSilent is TRUE,
//then pstrAllergies and nAllergyCount will be filled in, and it will return FALSE.
//If they are not allergic, then bSilent, pstrAllergies, and pnAllergyCount are all ignored, and it returns TRUE.
BOOL CheckAllergies(long nPatientID, long nMedicationID, IN BOOL bSilent = FALSE, OPTIONAL OUT CString *pstrAllergies = NULL, OPTIONAL OUT int *pnAllergyCount = NULL);

// (a.walling 2007-02-08 14:40) - PLID 24674 - Check whether the patient is allergic to the given list of medication
// (Send Drug IDs, not Patient Medication IDs.) Displays a warning box and returns TRUE if they choose to continue.
BOOL CheckAllAllergies(IN long nPatientID, OPTIONAL IN CDWordArray *pdwaDrugIDs = NULL);

// (a.walling 2010-06-28 14:42) - PLID 38442 - Subject and include attachment options
void SendErrorEmail(CWnd *pParent, const CString &strErrorInfo, const CString& strSubject = "Exception Report", bool bIncludeNxLog = true);

// (a.walling 2010-06-28 18:08) - PLID 38442 - Send an error report without having to deal with exceptions, etc
void SendErrorToNexTech(CWnd *pParent, const CString &strErrorSource, const CString &strErrorClass, const CString &strErrorText);

// (c.haag 2005-10-31 15:30) - PLID 16595 - These functions make it easier to invoke the preferences dialog in that
// hide the overhead of needing to specify the property manager object
int ShowPreferencesDlg(PreferenceItem piStartTab = piPatientsModule);
int ShowPreferencesDlg(LPDISPATCH pCon, CString strUserName, CString strRegistryBase, PreferenceItem piStartTab = piPatientsModule);

//TES 12/9/2005 - Takes an array of longs and returns a comma-separated list suitable for an IN statement.
//If bCanReturnEmpty is FALSE, then if the array is empty it will return "-1"
// (a.walling 2010-04-30 17:13) - PLID 38553 - Part of GlobalDataUtils
//CString ArrayAsString(IN const CArray<long,long> &arLongs, bool bCanReturnEmpty = false);
//CString ArrayAsString(IN const CDWordArray &dwa, bool bCanReturnEmpty = false);
//// (c.haag 2007-03-07 15:17) - PLID 21207 - Added overload for ints
//CString ArrayAsString(IN const CArray<int,int> &arInts, bool bCanReturnEmpty = false);
//// (a.walling 2007-03-27 12:57) - PLID 25367 - Added overload for VariantArrays.
//CString ArrayAsString(IN const CVariantArray &arVariants, bool bCanReturnEmpty = false);

CString AsMenuItem(const CString &str);

CString GetConnectionPropertyAsString(ADODB::_Connection *lpConn, LPCTSTR strPropName, const CString &strDefault);

//This takes the given string, and returns a string that escapes the delimiting character with a character of your choice..
//So for example, if the delimiting character was a ;, and the escape character was \, this function would replace 
//";" with "\;", and "\" with "\\"
CString FormatForDelimitedField(const CString &strIn, char chDelimiter, char chEscape);

//This takes a string that was formatted with FormatForDelimitedField, and returns the original string.
CString ReadFromDelimitedField(const CString &strIn, char chDelimiter, char chEscape);

//Returns the position, in the given string, of the first non-escaped delimiter.
// (c.haag 2007-01-30 13:03) - PLID 24485 - Added an optional string start parameter
int FindDelimiter(const CString &strIn, char chDelimiter, char chEscape, int nStart = 0);

// Takes an unlimited set of HWNDs and shows/hides them.  It attempts to do it all at once with 
// DeferWindowPos().  If this fails it uses the brute force way, SetWindowPos().
// First give the number you will be showing, then give the number you will be hiding, and then 
// give the HWNDs (the first nCountToShow of which will be shown, and the next nCountToHide of 
// which will be hidden).
void ShowAndHideWindows(long nCountToShow, long nCountToHide, ...);




// Get the name of any custom field in the software.  This caches all the field names for quick access.
BOOL GetCustomFieldName(long nCustomFieldIndex, OUT CString &strAns);

// Same as above, except this won't report failure.  Instead it will return a generic name for the field.
CString GetCustomFieldName(long nCustomFieldIndex);

// Lets you change the cached value for any particular field name (in case you know the field 
// name has changed and you don't want to have to clear the whole cache just to reload from data).
void SetCustomFieldNameCachedValue(long nCustomFieldIndex, const CString &strNewName);


// (z.manning, 06/14/2006) - Returns the edition of the sql server for the current db.
// (j.jones 2013-03-29 11:35) - PLID 54281 - reworked this to just return the properties for product version (actual version #),
// the level (RTM, SP1, etc.), and edition (Express, Developer, etc.)
void GetSqlServerEdition(OUT CString &strVersion, OUT CString &strLevel, OUT CString &strEdition);

// (j.jones 2013-03-29 11:50) - PLID 54281 - Returns the maximum size of a database, in MB, for
// the current version of SQL. For example, 4GB will return a long of 4096.
// If there is no limit, we return -1.
long GetMaximumDatabaseSize();

// (z.manning, 06/15/2006) - Returns the size of the current database in bytes. (__int64)
// (b.savon 2011-10-05 15:25) - PLID 44428 - Change to MB (long)
long GetCurrentDatabaseSize();

// (m.hancock 2006-06-30 14:49) - PLID 21307 - Returns true if the passed MailID records in MailSent are 
// associated with a lab.  The passed MailIDs should be comma separated.
BOOL IsLabAttachment(CString strMailIDs);

long CountDepthBelowRow(NXDATALIST2Lib::IRowSettingsPtr pSourceRow);
long CountDepthOfRowFromTop(NXDATALIST2Lib::IRowSettingsPtr pSourceRow);

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
int InputBoxNonEmpty(CWnd* pParent, const CString &strPrompt, CString &strValue, int nMaxLength = -1);

// (a.walling 2006-10-05 13:05) - PLID 22869 - EncodeURL function returns a safe encoded URL
//		specifically, characters are replaced by their escaped equivalents.
CString EncodeURL(IN const CString &str);

//TES 5/24/2004: bDepthIncreased means that this was saved at a lower color depth than is currently displayed, so the caller
//should consider reloading the image if possible.
bool LoadVariantAsBitmap(CDC *pDC, _variant_t varBits, HBITMAP &hImage);

// (a.walling 2006-10-09 11:49) - PLID 22689 - Load JPEG/PNG thumbnails
HBITMAP GetBitmapFromJPGByteArray(CDC *pDC, BYTE *arBytes, DWORD dwSize);
HBITMAP GetBitmapFromPNGByteArray(CDC *pDC, BYTE *arBytes, DWORD dwSize);

// (a.walling 2013-04-24 14:57) - PLID 56247 - Several thumb functions need not be exposed here; internal to globalutils.cpp

// (a.walling 2006-10-20 10:06) - PLID 22991 - Load any kind of image file and return an hbitmap
HBITMAP LoadBitmapFromFile(IN CString strFile);
// (a.walling 2013-04-24 14:57) - PLID 56247 - Return the thumbnail handle from this file, transparently writing a new one and rewriting if necessary.
HBITMAP LoadThumbFromFile(const IN CString &strFile);

// (a.walling 2007-04-12 11:57) - PLID 25605 - Save the bitmap to a temp file using the name specified, and return the full path
CString SaveBitmapToTempFile(HBITMAP hbmp, CString strFileName = "");

// (z.manning, 01/02/2007) - PLID 24056 - Moved here from License.cpp.

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//BOOL IsNumeric(const CString &str);

// (z.manning, 01/03/2007) - PLID 24056 - Created this funtion (which also exists as members of the
// Ebilling and CPTCode classes for no apparent reason.

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//void StripNonNumericChars(CString &str);

//void StripNonAlphanumericChars(CString &str);

// (z.manning 2011-09-08 10:48) - PLID 45335 - Moved LooseCompareDouble to GlobalDataUtils

// (c.haag 2007-01-25 13:01) - PLID 24420 - Returns true if the contents of two arrays
// match perfectly
BOOL AreArrayContentsMatched(const CArray<long,long>& a1, const CArray<long,long>& a2);
// (c.haag 2009-01-19 11:14) - PLID 32712 - Overload for CDWordArrays
BOOL AreArrayContentsMatched(const CDWordArray& a1, const CDWordArray& a2);

// (c.haag 2007-02-22 11:16) - PLID 24701 - Returns zero if the paths point to the
// same place, or non-zero if otherwise. It internally uses CString::CompareNoCase.
// This does not attempt any kind of unmapping (i.e. from k:\ to \\computer\folder)
int ComparePaths(CString strPath1, CString strPath2);

// (a.walling 2007-02-27 09:26) - PLID 24451 - Simply takes a date time and sets the seconds part to zero
COleDateTime StripSeconds(IN const COleDateTime &dt);

// (c.haag 2007-03-09 12:03) - PLID 25138 - We now require a patient ID
// (j.jones 2009-09-01 09:26) - PLID 17734 - optionally takes in an appointment ID
long CalcDefaultCaseHistoryProvider(long nPatID, long nAppointmentID = -1);

//TES 3/13/2007 - PLID 24993 - Returns whether they're configured to use the UB92 or UB04.  Will always return
// one of those two values; if a bad value is found in ConfigRT this will reset it to eUB92 and return eUB92.
//TES 3/27/2007 - PLID 24993 - Turns out that globalauditutils.cpp, which calls this function, is shared in NxOutlookAddIn.
// This define is here to enable globalauditutils to know whether it can use this function or not.
#define GET_UB_FORM_TYPE_AVAILABLE
enum UBFormType
{
	eUB92 = 0,
	eUB04 = 1,
};
UBFormType GetUBFormType();

//DRT 4/10/2007 - PLID 25564 - This function used to exist in several different dialogs.  We now need it to be global for 
//	ftputils to work correctly, and those rest might as well use 1 central function.
BOOL PeekAndPump();

// (a.walling 2007-08-08 17:13) - PLID 27017 - Return a number unique to this session
DWORD GetUniqueSessionNum();

// (j.jones 2007-05-04 08:54) - PLID 23280 - removed g_dtLastPaymentDate
// in lieu of a stack that tracks the payments we've edited, in order
// of last edited ascending
struct LastPaymentDate {
	long nID;
	COleDateTime dtDate;
};

static CArray<LastPaymentDate*,LastPaymentDate*> g_pLastPaymentInfoList;

// (j.jones 2007-05-04 09:07) - PLID 23280 - used to get the last payment date
COleDateTime GetLastPaymentDate();
// (j.jones 2007-05-04 09:07) - PLID 23280 - used to add a new payment date
void AddLastPaymentDate(long PaymentID, COleDateTime dtDate);
// (j.jones 2007-05-04 09:08) - PLID 23280 - used to remove a payment date
void DeleteLastPaymentDate(long PaymentID);
// (j.jones 2007-05-04 09:09) - PLID 23280 - used to clear out the array
void EmptyLastPaymentDates();

// (j.jones 2007-05-07 08:50) - PLID 25906 - removed g_dtLastBillDate and g_dtLastQuoteDate
// in lieu of stacks that track the bills and quotes we've edited, in order
// of last edited ascending

//LastBillDate is used for bills and quotes, in separate lists
struct LastBillDate {
	long nID;
	COleDateTime dtDate;
};

static CArray<LastBillDate*,LastBillDate*> g_pLastBillInfoList;
static CArray<LastBillDate*,LastBillDate*> g_pLastQuoteInfoList;

// (j.jones 2007-05-07 08:51) - PLID 25906 - used to get the last bill date
COleDateTime GetLastBillDate();
// (j.jones 2007-05-07 08:51) - PLID 25906 - used to add a new bill date
void AddLastBillDate(long BillID, COleDateTime dtDate);
// (j.jones 2007-05-07 08:51) - PLID 25906 - used to remove a bill date
void DeleteLastBillDate(long BillID);
// (j.jones 2007-05-07 08:51) - PLID 25906 - used to clear out the array
void EmptyLastBillDates();

// (j.jones 2007-05-07 08:53) - PLID 25906 - used to get the last quote date
COleDateTime GetLastQuoteDate();
// (j.jones 2007-05-07 08:53) - PLID 25906 - used to add a new quote date
void AddLastQuoteDate(long QuoteID, COleDateTime dtDate);
// (j.jones 2007-05-07 08:53) - PLID 25906 - used to remove a quote date
void DeleteLastQuoteDate(long QuoteID);
// (j.jones 2007-05-07 08:53) - PLID 25906 - used to clear out the array
void EmptyLastQuoteDates();

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

// (a.walling 2007-05-10 13:14) - PLID 25971 - Really getting annoying working with threads without naming
// some of them, so I put in this. Code copied from http://blogs.msdn.com/stevejs/archive/2005/12/19/505815.aspx
// it also exists on http://msdn2.microsoft.com/en-us/library/xcb2z8hs(vs.71).aspx but it has a bug.
// I modified it a bit to only use it when debugging.

#ifdef _DEBUG
	void dbg_SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);
	#define SetThreadName(ThreadID, ThreadName) dbg_SetThreadName(ThreadID, ThreadName)
#else
	#define SetThreadName(ThreadID, ThreadName) ((void)0)
#endif

// (z.manning, 06/12/2007) - PLID 26255 - Returns the default path where provider images are stored.
CString GetContactImagePath();
// (d.moore 2007-07-19) - PLID 26696 - Returns the default path where location images are stored.
CString GetLocationImagePath();

// (a.walling 2007-09-28 09:14) - PLID 27556 - functions to return ResultCode and OpenResult constants in English.
namespace OPOS {
	CString GetMessage(long nCode);
}

// (a.walling 2007-08-31 12:43) - PLID 27265 - Bob's superior modified version of my AreCOMObjectsEqual function.
// b.cardillo changed the input params from IDispatch* to a template, which allows smart pointers of any type now.
// Compares the identity of two COM objects (are these the SAME objects?)
template<class com_ptr_type1, class com_ptr_type2> BOOL AreCOMObjectsEqual(IN com_ptr_type1 pObject1, IN com_ptr_type2 pObject2)
{ 
    if (pObject1 == NULL && pObject2 == NULL) {
        return true;
    } else if (pObject1 == NULL || pObject2 == NULL) {
        return false;
    } else {
        IUnknown * pUnk1 = NULL;
        HRESULT hr = pObject1->QueryInterface(IID_IUnknown, (void**)&pUnk1);
        if (hr == S_OK) {
            IUnknown * pUnk2 = NULL;
            hr = pObject2->QueryInterface(IID_IUnknown, (void**)&pUnk2);
            if (hr == S_OK) {
                if (pUnk1 == pUnk2) {
                    pUnk1->Release();
                    pUnk2->Release();
                    return TRUE;
                } else {
                    pUnk1->Release();
                    pUnk2->Release();
                    return FALSE;
                }
            } else {
                pUnk1->Release();
                _com_issue_error(hr);
                throw;
            }
        } else {
            _com_issue_error(hr);
            throw;
        }
    }
}

// (z.manning, 04/22/2008) - PLID 29745 - Deleted the obsolete TransparentButton and TransparentStatic classes
// (NxButton and CNxStatic should be used instead)

bool GenericTransparentCtlColor(HBRUSH& hbr, CDC* pDC, CWnd* pWnd, UINT nCtlColor);

// (a.walling 2008-04-01 13:17) - PLID 29497 - Generic OnCtlColor handler for transparency
// (a.walling 2008-04-01 13:17) - PLID 29497 - Handle NxDialogs too
#define HANDLE_GENERIC_TRANSPARENT_CTL_COLOR() \
	{ HBRUSH hbr = NULL; \
	if (!GenericTransparentCtlColor(hbr, pDC, pWnd, nCtlColor)) { \
		if (IsKindOf(RUNTIME_CLASS(CNxDialog))) \
			return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor); \
		else \
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor); \
	} else return hbr; }

//TES 3/25/2008 - PLID 24157 - Moved out of schedulerview
BOOL IsWindowDescendent(const HWND hwndAncestor, const HWND hwndDescendent);

// (a.walling 2008-04-21 15:29) - PLID 29711 - Retrieve the text color of the tabs
// (j.jones 2016-04-18 17:02) - NX-100214 - this is now GetDefaultNxTabTextColor in NxUILib
//COLORREF GetTabViewTextColor();

//(r.farnworth 2013-01-25) PLID 54667 - Overloaded function to take in a strPath variable
// (j.fouts 2013-03-04 16:27) - PLID 55427 - Removed nPatientID as we can just use the Prescription for that
BOOL MergePrescriptionsToWord(CArray<long, long> &aryPrescriptionIDs, CString strTemplateName, CString strFullTemplatePath, CString &strPath,
							  BOOL bMergeToPrinter  = FALSE, BOOL bMergetoFax  = FALSE);
// (j.jones 2008-05-16 09:14) - PLID 29732 - printing prescriptions is now a global function
// (c.haag 2009-08-18 13:11) - PLID 15479 - Added bMergeToPrinter
//(r.farnworth 2013-01-25) PLID 54667 - Added bMergetoFax
// (j.fouts 2013-03-04 16:27) - PLID 55427 - Removed nPatientID as we can just use the Prescription for that
BOOL MergePrescriptionsToWord(CArray<long, long> &aryPrescriptionIDs, CString strTemplateName, CString strFullTemplatePath, BOOL bMergeToPrinter = FALSE, BOOL bMergetoFax = FALSE);

// (a.walling 2008-05-19 15:22) - PLID 27810 - This function will display a messagebox if the NPI is invalid
BOOL CheckNPI(const CString& strNum, CWnd* pParent, BOOL bAllowCancel = FALSE, CString strExtra = "");
// (a.walling 2008-05-19 15:22) - PLID 27810 - Returns whether the NPI is valid or not
BOOL IsValidNPI(const CString& strNum);
// (a.walling 2008-05-19 15:17) - PLID 27810 - Returns whether the string has a valid Luhn checksum
BOOL CheckLuhn(const CString& strNum);
// (j.jones 2013-04-10 15:41) - PLID 56191 - Takes a number that needs a Luhn checksum added, and returns the check digit.
long CalculateLuhn(const CString& strNumToCalculate);

// (j.jones 2008-06-05 11:25) - PLID 29154 - this function will take in a count of prescriptions, and
// returns the template to be used, also will fill in booleans to let the caller know whether
// a template was found for the exact count (if false, means we sent back the generic default),
// and whether templates for other counts were found (not filled if bExactCountFound = TRUE)
CString GetDefaultPrescriptionTemplateByCount(long nCountPrescriptions, BOOL &bExactCountFound, BOOL &bOtherCountFound);

// (a.walling 2008-06-09 16:58) - PLID 22049 - Helper functions for GetMessagePos.. MFC-Style!
void GetMessagePos(CPoint& pt);
void GetMessagePos(CPoint* ppt);

// (a.walling 2008-07-25 10:26) - PLID 30836 - Are we Vista+?
BOOL IsVistaOrGreater();

// (z.manning 2008-07-23 17:12) - PLID 30804 - Moved this here from CNxSchedulerDlg
void GetIDsFromCommaDelimitedString(CDWordArray* dwaryIDs, CString strIDList);

// (j.jones 2013-10-11 14:53) - PLID 58965 - Same concept as GetIDsFromCommaDelimitedString,
// but the resulting value is a CStringArray. Naturally this can only be used on strings
// that are absolutely certain to not have commas in them.
void GetStringsFromCommaDelimitedString(CStringArray* aryStrings, CString strStringList);

// (a.walling 2008-08-27 12:53) - PLID 30855 - class for extended SQL error info
struct CSQLErrorInfo
{
	CSQLErrorInfo() :
		nNative(0), nClass(0), nState(0) {
			// construction
	};

	LONG nNative;
	BYTE nClass;
	BYTE nState;

	BOOL IsNxRaisedError(BYTE nExpectedClass = 16) {
		return nExpectedClass == 16 && nNative == 50000;
	};
};

// (a.walling 2008-08-27 12:52) - PLID 30855 - returns TRUE if the _com_error is a SQL server error with extended info,
// and fills the CSQLErrorInfo structure with useful info.
BOOL GetSQLErrorInfo(_com_error& e, CSQLErrorInfo& eSqlError);

// (a.walling 2013-11-12 09:06) - PLID 59416 - AsDateNoTime, AsTimeNoDate now in DateTimeUtils.h

// (j.jones 2008-09-04 14:07) - PLID 30288 - CreateNewMailSentEntry will create a new entry
// in MailSent and MailSentNotesT in one batch, and send a tablechecker with the new ID, as
// well as return that new ID
// (c.haag 2010-01-28 10:18) - PLID 37086 - Created version of CreateNewMailSentEntry that does not take in a service date
long CreateNewMailSentEntry(long nPersonID, CString strNote, CString strSelection, CString strPathName, CString strSender, CString strSubject, long nLocationID);
// (c.haag 2010-01-28 09:07) - PLID 37086 - Refactored CreateNewMailSentEntry to no longer support optional parameters
long CreateNewMailSentEntry(long nPersonID, CString strNote, CString strSelection, CString strPathName, CString strSender, CString strSubject, long nLocationID, COleDateTime dtServiceDate, CCDAType ctType);
// (d.singleton 2013-11-15 10:00) - PLID 59513 - need to insert the CCDAType in mailsent when generating a CCDA
// (r.farnworth 2016-03-10 11:36) - PLID 68401 - Added nOnlineVisitID
long CreateNewMailSentEntry(long nPersonID, CString strNote, CString strSelection, CString strPathName, CString strSender, CString strSubject, long nLocationID, COleDateTime dtServiceDate,
							long nChecksum, long nCategoryID, long nPicID, long nLabStepID, BOOL bIsPhoto, long nInternalRefID, CString strInternalTblName, CCDAType ctType, int nOnlineVisitID = -1);

// (j.jones 2008-09-11 12:18) - PLID 31335 - finds all active insured parties with
// an inactive date that is today's date or earlier, and inactivates them
void UpdateInactiveInsuredParties();

// (j.gruber 2008-10-27 10:33) - PLID 31432 - this function is used when detaching a history document that is attached to a lab in order 
//to audit the value fields since that changes when the document is no longer attached to a lab
// (a.walling 2010-01-21 17:00) - PLID 37023 - Send in a patient ID
long HandleLabResultAudit(CString strWhere, CString strPersonName, long nPatientID);

// (z.manning 2008-10-09 15:37) - PLID 31628 - Utility function to delete a lab
// (j.jones 2009-06-04 16:36) - PLID 34487 - need to pass in lists of problem links to delete and problems to delete
void DeletePatientLab(IN const long nLabID, IN const CString strEMRProblemLinkIDsToDelete, IN const CString strEMRProblemIDsToDelete);

// (z.manning 2008-10-13 12:42) - PLID 31667
//TES 1/4/2011 - PLID 37877 - Added a parameter for nPatientID, to save a data access.
//TES 1/14/2015 - PLID 55674 - Added pListChangedTodos. If it is not null, the function will append to the list with information about Todos that need tablecheckers, 
// rather than sending the tablecheckers itself
struct ChangedTodo {
	long nTaskID;
	long nPatientID;
	long nAssignedTo;
	TableCheckerDetailIndex::Todo_Status tsStatus;
};
void SyncTodoWithLab(IN const long nLabID, IN const long nPatientID, std::list<ChangedTodo> *pListChangedTodos = NULL);
void SyncLabWithTodo(IN const long nTaskID);

// (z.manning 2008-10-30 17:08) - PLID 21082 - Prompts a user for a signature and if they enter one
// will complete the given lab.
// (r.gonet 09/02/2014) - PLID 63221 - Added the option for the function to not send a table checker.
BOOL PromptToSignAndMarkLabComplete(IN CWnd *pwndParent, IN const long nLabID, BOOL bSendTableChecker = TRUE);

// (j.jones 2008-11-19 09:33) - PLID 28578 - this function will compare two passwords, case-sensitive,
// but if bIsPasswordVerified is FALSE then we have to do some advanced case-insensitive work
// (b.savon 2015-12-16 10:47) - PLID 67705 - Restructured this to use locationID and username; and call the api
// (z.manning 2016-04-19 14:48) - NX-100244 - Added optional param for if the user is locked out
BOOL IsUserPasswordValid(CString strPassword, long nLocationID, long nUserID, const CString& strUsername, OPTIONAL OUT BOOL *pbLockedOut = NULL);

// (j.jones 2009-05-20 09:26) - PLID 33801 - update the UserPasswordHistoryT for the given user
void UpdateUserPasswordHistory(long nUserID, CString strOldPassword, CString strNewPassword);

// (z.manning 2008-12-16 13:17) - PLID 27682 - Global function to spell check an edit control
void SpellCheckEditControl(CWnd *pwndParent, CEdit *pEdit);

// (c.haag 2009-01-19 11:14) - PLID 32712 - Returns TRUE if an element exists in a long array.
// This function already existed in CEMRTableDropdownEditorDlg; I just made it global.
BOOL ExistsInArray(long n, const CArray<long,long>& a);

//(e.lally 2009-01-28) PLID 32814 - Algorithm for checking if an email address is valid.
//Currently check for '@' and trailing '.'
BOOL IsValidEmail(const CString& strEmailAddress);

//TES 4/8/2009 - PLID 33376 - Moved these macros here from EmrTreeWnd.cpp
#define FOR_ALL_ROWS_WITH_PARENT(datalist, parentrow)	NXDATALIST2Lib::IRowSettingsPtr pRow = parentrow->GetFirstChildRow();\
	CArray<LPDISPATCH,LPDISPATCH> arParentRows;\
	while(pRow) 

#define FOR_ALL_ROWS(datalist)	NXDATALIST2Lib::IRowSettingsPtr pRow = datalist->GetFirstRow();\
	CArray<LPDISPATCH,LPDISPATCH> arParentRows;\
	while(pRow) 

#define GET_NEXT_ROW(datalist)	if(pRow->GetFirstChildRow()) {\
		arParentRows.Add(pRow);\
		pRow = pRow->GetFirstChildRow();\
	}\
	else {\
		pRow = pRow->GetNextRow();\
		while(pRow == NULL && arParentRows.GetSize()) {\
			pRow = arParentRows.GetAt(arParentRows.GetSize()-1);\
			arParentRows.RemoveAt(arParentRows.GetSize()-1);\
			pRow = pRow->GetNextRow();\
		}\
	}\

// (z.manning 2009-05-05 15:27) - PLID 28529 - This function takes a datalist2 pointer and a column.
// It will then cycle through every row in the datalist and make any rows where the value in 
// nBoolCol is true invisible.
void HideDatalistRowsBasedOnBooleanColumn(IN NXDATALIST2Lib::_DNxDataListPtr pdl, const short nBoolCol);

//TES 5/15/2009 - PLID 28559 - Moved here from Auditing.cpp
// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString AsClauseString(const _variant_t &varValue);

//TES 5/26/2009 - PLID 34302 - Returns a version of the passed in text that can be put straight into a LIKE clause
CString FormatForLikeClause(const CString &strIn);
//TES 5/26/2009 - PLID 34302 - De-escapes the passed in text, which is assumed to be properly escaped for a LIKE clause
CString UnformatFromLikeClause(const CString &strIn);

// (j.jones 2009-09-24 10:13) - PLID 29718 - added global function to get a patient's
// last appointment date prior to today (will return an invalid date if no appt. exists)
COleDateTime GetLastPatientAppointmentDate(long nPatientID);

// (c.haag 2009-10-12 12:25) - PLID 35722 - Creates a SQL batch statement for deleting MailSent records based on the input filter
void AddDeleteMailSentQueryToSqlBatch(IN OUT CString &strSqlBatch, const CString& strMailSentInClause);

// (c.haag 2009-10-12 12:42) - PLID 35722 - This function deletes MailSent records. To be consistency with the legacy
// queries in the calling functions, we do not use a batch, and do not use transactions or auditing. This is a raw deletion.
void DeleteMailSentRecords(const CString& strMailSentInClause);

// (a.walling 2009-11-23 11:54) - PLID 36396 - Return a new GUID string (with dashes)
// Now in NxSystemUtilitiesLib
//CString GetNewUniqueID();

// (a.walling 2009-12-17 08:32) - PLID 36624 - Get ISO8601 timestamp
CString GetISO8601Timestamp();
CString GetISO8601Timestamp(SYSTEMTIME& st);

// (b.cardillo 2010-01-07 12:48) - PLID 35780 - Added generic function to handle complete flow (including UI) for 
// prompting the user to save a file and saving it

// Callback function for use by PromptSaveFile() to iteratively get the bytes to write to the file.
// This function's responsibilities are:
//  1. allocate and fill the data, return the data and its size in the last two parameters
//  2. increment the second parameter (the in/out large integer position parameter) to move forward 
//     the appropriate amount in the iteration
// The caller of this callback function will do nothing to increment the position. It simply calls 
// this function repeatedly, allowing the function to update its own position parameter, until the 
// output returned is empty (either null byte pointer, or zero size).
typedef void (CALLBACK *LPFUNCGETOUTPUTBYTES)(LPVOID, LARGE_INTEGER &, LPBYTE *, UINT *);

// Handles the complete flow (including UI) for prompting the user to save any kind of file, and saving the file
BOOL PromptSaveFile(OUT CString &strSavedToFile, CWnd *pParentWnd, LPCTSTR strDefaultFileTitle, LPCTSTR strFilter, LPCTSTR strDefaultExtension, LPFUNCGETOUTPUTBYTES pfnGetOutputBytes, LPVOID pParam, BOOL bMessageBoxOnSuccess, BOOL bMessageBoxOpenSavedFileIfCsvOrTxt);


// (b.cardillo 2010-01-07 12:48) - PLID 35780 - Implemented typical use of above generic function to save csv

// Implementation of the LPFUNCGETOUTPUTBYTES callback function to retrieve the CSV text from a datalist or datalist2 and return it all in one block
void CALLBACK GetOutputBytes_DLor2_ISODates(IN LPVOID pParam, IN OUT LARGE_INTEGER &liPosition, OUT LPBYTE *ppOutput, OUT UINT *pnOutputLen);

// Wrapper for the typical use of PromptSaveFile(), in which you want to save the contents of a datalist or datalist2 to a csv file
CString PromptSaveFile_CsvFromDLor2(LPUNKNOWN pUnkDataListOr2, CWnd *pParentWnd, LPCTSTR strDefaultFileTitle, BOOL bPromptOpenIfCsvOrTxt);

//TES 1/11/2010 - PLID 36761 - Moved here from PatientGroupsConfigDlg.h, seems like a handy struct
struct IdName {
	long nID;
	CString strName;
};

// (j.gruber 2010-01-26 16:32) - PLID 23693
long GetImageCategoryID();

//TES 4/7/2010 - PLID 38040 - Moved AnatomySide and ResultValue enums from here to GlobalLabUtils.h

// (m.hancock 2006-07-24 11:47) - PLID 21582 - Make sure that modifying steps is audited
// For audits on modifying steps, we have give a description containing the lab's description and the step's name
CString GenerateStepCompletionAuditOld(long nStepID);

//TES 4/5/2010 - PLID 38040 - Moved from NxDialog
//TES 4/6/2010 - PLID 38040 - A copy of this function exists in NxServer's general.h, in the highly unlikely event that it ever changes,
// make sure it's updated there as well.

// (a.walling 2010-05-03 10:18) - PLID 38553 - Now in NxDataUtilities SDK
//CString FixCapitalization(CString strCapitalized);

// (j.jones 2010-04-23 09:42) - PLID 11596 - moved from RSIMMSLink to be a global function
CString GetThisComputerName();

// (j.jones 2010-04-23 09:44) - PLID 11596 - added to cleanly support the ltUserComputer preference type
CString GetCurrentUserComputerName();

// (a.walling 2010-01-06 12:06) - PLID 36809 - Handle CCR documents
// (j.jones 2010-06-30 11:42) - PLID 38031 - moved from the CCDInterface namespace, renamed to be generic
// (d.singleton 2014-05-30 13:38) - PLID 61927 - need mailsent id in case its a ccda
void ViewXMLDocument(const CString& strXMLFilePath, CWnd* pParent, long nMailSentID = -1);
void ViewXMLDocument(MSXML2::IXMLDOMDocument2Ptr pDocument, CWnd* pParent, long nMailSentID = -1);

// (j.jones 2010-06-30 11:58) - PLID 38031 - moved from the CCD namespace
CString GetGenericXMLXSLResourcePath();

//TES 9/11/2012 - PLID 52518 - Added, used for viewing the output from the PQRI Reporting dialog
CString GetPQRSXSLResourcePath();

// (j.jones 2013-10-11 10:03) - PLID 58806 - added QRDA XSL
CString GetQRDAXSLResourcePath();

// (f.dinatale 2010-10-11) - PLID 33753 - Added a function to format SSN strings
CString FormatSSNText(const CString & strSSNin, ESSNMask eMask, const CString& strFormat, int nAllowedCharacters = 4);
// (f.dinatale 2010-10-11) - PLID 33753 - Overloaded the FormatSSN function so that it can now work right on form objects.
void FormatSSN(CEdit* pWnd, const CString& strFormat);

// (z.manning 2010-10-28 11:40) - PLID 41129 - Created a global function to get the basic patient list from clause
// (previoulsy this code was essentially duplicated in CResEntryDlg and PatientToolBar).
CString GetPatientFromClause();

//TES 8/1/2011 - PLID 44716 - Moved into its own function
//TES 10/13/2011 - PLID 44716 - This function can create a transaction, so if it's being called from someplace that's already inside a 
// transaction, it needs to not do that.
// (j.jones 2012-04-17 17:05) - PLID 13109 - this now returns the new sort order, if needed
long CreateNoteCategory(const CString &strCategory, bool bInTransaction = false, OUT long *pnNewSortOrder = NULL);

//TES 8/2/2011 - PLID 44814 - Similar to PollEmrChartPermissions, gets the IDs of all history categories the current user is permissioned
// to view.  Returns TRUE if they can view all categories
BOOL PollHistoryCategoryPermissions(OUT CArray<long,long> &arynPermissionedCategoryIDs);

//TES 8/2/2011 - PLID 44814 - Returns a boolean SQL statement specifying which categories the current user is permissioned to see.
// So, either "<field> NOT IN (ID1, ID2)" or "" if they can view all categories
CString GetAllowedCategoryClause(const CString &strField);

//TES 8/3/2011 - PLID 44814 - Added a parameterized version
class CSqlFragment;
CSqlFragment GetAllowedCategoryClause_Param(const CString &strField);


//TES 8/5/2011 - PLID 44901 - Similar to PollEmrChartPermissions, gets the IDs of all locations (for labs) the current user is permissioned
// to view.  Returns TRUE if they can view all locations
BOOL PollLabLocationPermissions(OUT CArray<long,long> &arynPermissionedLocationIDs);

//TES 8/5/2011 - PLID 44901 - Returns a boolean SQL statement specifying which locations (for labs) the current user is permissioned to see.
// So, either "<field> NOT IN (ID1, ID2)" or "" if they can view all labs
CString GetAllowedLocationClause(const CString &strField);

//TES 8/5/2011 - PLID 44901 - Added a parameterized version
CSqlFragment GetAllowedLocationClause_Param(const CString &strField);

//(c.copits 2011-09-29) PLID 45626 - Validate Email Addresses
// Returns true if valid email address and false otherwise.
bool IsValidEmailAddress(CString strEmail);

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - moved NxAPI globals to Practice unit

// (a.walling 2012-04-11 15:39) - PLID 49594 - Replace ://PRACTICE.EXE with ://PracticeMain.exe etc if we are not actually running as practice.exe
CString FixupPracticeResModuleName(const CString& str);

// (c.haag 2013-08-29) - PLID 58216 - Given a lab and a lab step, this will return the collection of related lab steps in other
// specimens that could finish if the given lab step has finished.
ADODB::_RecordsetPtr GetCompletableLinkedSteps(long nStepID, bool bIgnoreStepsCompletedBySigning, ADODB::_ConnectionPtr lpCon = NULL);

// (b.spivey, April 12, 2013) - PLID 55943 - Function to recursively prompt for a ladder and complete lab steps. 
// (c.haag 2013-09-05) - PLID 58216 - bSavingPatientLabResults indicates whether the user just saved lab results (potentially signing them)
bool GlobalModifyStepCompletion(long nLabID, long nStepID, bool bComplete, bool bSavingPatientLabResults);
bool GlobalModifySingleStepCompletion(long nStepID, long nLabID, bool bComplete, long nPatientID, bool bSavingPatientLabResults);

// (z.manning 2012-08-31 10:58) - PLID 52413
namespace NexTech_COM {
	struct IPatientDashboardGenerator;
};
NexTech_COM::IPatientDashboardGenerator* GetPatientDashboardGenerator();

// (j.gruber 2012-10-17 13:13) - PLID 47289
CString GetAffiliateTypeDescription(AffiliatePhysType afpt);

// (j.fouts 2012-12-14 10:18) - PLID 54183
// This is used to sort string using alphabetic order for sequences of aplha chars,
// and numerically for sequences of number. This is particularly handing when sorting
// names of things like: page1, page2, page10. And a alphabetic sort would give page1, page10, page2.
// This will return true if strLeft should come before strRight in a sort function
BOOL CompareAlphaNumeric(CString& strLeft, CString& strRight);
//(r.farnworth 2013-01-25) PLID 54667 - Moved from PrescriptionEditDlg. Validates the user's selected templates when attempting to merge.
bool CheckPrescriptionTemplates(CString &strTemplateName, CString &strTemplatePath, HWND parent);
//(r.wilson 4/8/2013) pl 56117
CString RemoveInsignificantZeros(CString strText);
//(r.wilson 3/6/2013) pl 55478
int GetNumberOfDecimalsInString(CString &strNumber);
//(r.wilson 3/6/2013) pl 55478
CString FormatDecimalText(CString &strOriginalText, int nMaxDigitsAllowed);
//(r.wilson 3/6/2013) pl 55478
BOOL DoesStringContainLetters(CString &strText);
//(r.wilson 4/8/2013) pl 56117
void TrimSpaces(CDialog *pDlg, int nControlId );

// (a.walling 2013-06-18 11:44) - PLID 57204 - Extracts comma-separated strings and outputs them sorted ascending with no duplicates
CString SortAndRemoveDuplicatesFromCSV(const CString& csvStr);
// (a.walling 2013-06-20 13:49) - PLID 57204 - Extracts delimited substrings from the string and outputs them sorted ascending with no duplicates
CString SortAndRemoveDuplicates(const CString& delimStr, const char* delimiters = " ,\r\n\t");
// (d.thompson 2014-02-14) - PLID 60716 - Create a diagnosis code, send back the ID
// (j.jones 2014-03-04 09:18) - PLID 61136 - added PCS bit
// (j.armen 2014-03-25 14:21) - PLID 61517 - Add flag for ICD10 codes
long CreateNewAdminDiagnosisCode(CString strCode, CString strDescription, bool IsAMA = false, bool bPCS = false, bool bICD10 = false);

// (b.savon 2015-03-19 08:33) - PLID 65248 - Add a utility to perform a database backup
void PerformManualDatabaseBackup(HWND hwndParent);

// (a.walling 2010-07-29 09:58) - PLID 39871 - Moved GetLicenseAgreementAcceptedName to CLicense
// (z.manning 2015-05-19 16:33) - PLID 65971 - Moved this to global utils
CString GetLicenseAgreementAcceptedPropertyName();

// (j.jones 2015-03-23 13:45) - PLID 65281 - moved from DiagSearchConfig::SetMinDropdownWidth
//estimates the current dropdown with, forces it to be larger if it's too small
void SetMinDatalistDropdownWidth(NXDATALIST2Lib::_DNxDataListPtr pDataList, long nMinWidth);

// (z.manning 2015-08-10 13:50) - PLID 67221
NexTech_COM::IICCPSessionManager* GetICCPDevice();

// (v.maida 2016-02-01 09:06) - PLID 68033 
BOOL UsersHaveLocationAsDefaultFFALocationPref(long nLocationID);
void RevertDefaultFFALocationPrefForLocation(long nLocationID);
// (v.maida 2016-02-01 09:06) - PLID 68035
BOOL UsersHaveResourcesAsDefaultFFAResourcePref(std::vector<long> vecResourceIDs);
void RevertDefaultFFAResourcePrefForResources(std::vector<long> vecResourceIDs);

enum class WordProcessorType;
// (z.manning 2016-02-17 12:39) - PLID 68223 - Returns the preference for which word processor to use
WordProcessorType GetWordProcessorType();

// (j.jones 2016-03-08 13:50) - PLID 68478 - the ConfigRT value of LastLicenseUpdateAttempt
// is now encrypted to prevent tampering, this function will get the real datetime, might be g_cdtSqlZero
COleDateTime GetLastLicenseUpdateAttempt();
// (j.jones 2016-03-08 13:50) - PLID 68478 - the ConfigRT value of LastLicenseUpdateAttempt
// is now encrypted to prevent tampering, this function will update the encrypted date to
//be the current time
void UpdateLastLicenseUpdateAttemptToCurrentTime();
// (v.maida 2016-05-19 16:54) - NX-100684 - If they're using Azure RemoteApp, then they should be saving to their shared path, otherwise they can
// save to their practice path.
CString GetEnvironmentDirectory();

// (b.eyers 2016-04-29 14:59) - NX-100350 - pqrs exporter
void OpenPQRSExporter(const CString &strSubkey);

void GetPQRSAppNeedsUpdateURL(CString &strURL, COleDateTime &dtAppDate);
void UpdatePQRSApp( IN CString strPQRSAppURL,OUT CString &strPQRSAppFilePathToRunExecute, IN const FILETIME &ftPQRSFileTime);

/// <summary>
/// Returns the current call stack, formatted with newlines,
/// to include in an exception report.
/// Defaults to the last 20 frames, skipping this function call and
/// Nx::Debug::CallStack itself.
/// </summary>
/// <returns>A series of hex representations of the call stack. You can identify the function names
/// by pasting them into \nx-internal\Development\CallStackSymbolizer\ while pointed to the Practice.pdb
/// file associated with the exe this was thrown from.</returns>
CString GetCallStack();

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
CString GetCallStack(DWORD dwMaxFramesToReturn, DWORD dwStackFramesToSkip);

#endif
