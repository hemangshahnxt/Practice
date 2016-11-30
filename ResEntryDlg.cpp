// ResEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ResEntryDlg.h"
#include "GlobalSchedUtils.h"
#include "CommonSchedUtils.h"
#include "NxSchedulerDlg.h"
#include "MultiResourceDlg.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "UserWarningDlg.h"
#include "NxStandard.h"
#include "SchedulerRc.h"
#include "NewPatient.h"
#include "PhaseTracking.h"
#include "PatientsRc.h"
#include "AuditTrail.h"
#include "MultiSelectDlg.h"
#include "GlobalDrawingUtils.h"
#include "GlobalDataUtils.h"
#include "ResLinkDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalFinancialUtils.h"
#include "ReasonDlg.h"
#include "VersionInfo.h"
#include "InvPatientAllocationDlg.h"
#include "InvOrderDlg.h"
#include "PatientView.h"
#include "HL7Utils.h"
#include "ResolveConsSurgBookingDlg.h"
#include "OHIPUtils.h"
#include "MultiResourcePasteDlg.h"
#include "RemoteDataCache.h"
#include "SecurityGroupsDlg.h"
#include "AlbertaHLINKUtils.h"
#include "SelectDlg.h"
#include "GlobalInsuredPartyUtils.h"
#include "CreatePatientRecall.h" // (b.savon 2012-02-28 16:51) - PLID 48441 - Create Appt Recall
#include "RecallLinkDlg.h" // (a.wilson 2012-2-29) PLID 48418

#include "ColorUtils.h"

#include "ApptChooseMoreInsuredDlg.h" // (j.gruber 2012-08-02 12:12) - PLID 51896

#include "Rewards.h" // (j.luckoski 2013-03-04 13:46) - PLID 33548
#include "AppointmentsDlg.h"
#include "BillingModuleDlg.h"
#include "FinancialDlg.h"
#include "MedicationHistoryDlg.h" // (r.gonet 09/20/2013) - PLID 58416
#include "SharedInsuranceUtils.h"
#include "ApptChooseMoreInsuredDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

extern CPracticeApp theApp;

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace SINGLEDAYLib;
using namespace NXTIMELib;
using namespace ADODB;
using namespace NXDATALISTLib;

#include "CaseHistoryDlg.h"
#include "NxMessageDef.h"
#include "ReschedulingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (j.gruber 2012-08-01 09:20) - PLID 51885
#define IDM_RES_ADD_NEW_INS_PARTY 33474
// (j.gruber 2012-08-01 09:21) - PLID 51896
#define IDM_RES_CHOOSE_ADDITIONAL 33475

#ifdef _DEBUG
	#define DEBUG_FUNCTION_PROFILING_INIT()						int dfpcth_dwTimeCount = 0; DWORD dfpcth_arydwTime[50]; CString dfpcth_arystrid[50]; DWORD dfpcth_dwLast = GetTickCount();
	#define DEBUG_FUNCTION_PROFILING_CALC_TIME_HERE(strid)		dfpcth_arystrid[dfpcth_dwTimeCount] = strid; dfpcth_arydwTime[dfpcth_dwTimeCount] = GetTickCount() - dfpcth_dwLast; dfpcth_dwLast = GetTickCount(); dfpcth_dwTimeCount++;
	#define DEBUG_FUNCTION_PROFILING_FINISH(bflat)				(dfpcth_arydwTime[dfpcth_dwTimeCount] = GetTickCount() - dfpcth_dwLast, TestFormatArrayOfDWords(dfpcth_arydwTime, dfpcth_arystrid, dfpcth_dwTimeCount+1, bflat))
	#define DEBUG_FUNCTION_PROFILING_FINISH_TRACE()				TRACE(DEBUG_FUNCTION_PROFILING_FINISH(true) + "\r\n");
#else
	#define DEBUG_FUNCTION_PROFILING_INIT()
	#define DEBUG_FUNCTION_PROFILING_CALC_TIME_HERE(strid)
	#define DEBUG_FUNCTION_PROFILING_FINISH(bflat)
	#define DEBUG_FUNCTION_PROFILING_FINISH_TRACE()
#endif

// (j.jones 2007-11-21 11:27) - PLID 28147 - enumerated hotkey values
enum HotKeys {
	hkSave = 1,
	hkSaveGoToPatient,
	hkSaveCreateInvAlloc,
	hkSaveCreateCaseHist,
	hkSaveCreateNewPat,
	hkChangePatient,
	hkApptLinking,
	hkShowMoreInfo,
	hkSaveCreateInvOrder,	// (j.jones 2008-03-18 13:47) - PLID 29309
	hkSaveCreateBill,		// (j.jones 2008-06-24 10:06) - PLID 30455
	hkSaveViewMedicationHistory, // (r.gonet 09/03/2013) - PLID 58416
};

// (a.walling 2010-05-27 15:31) - PLID 38917
enum PatListColumns {
	plID = 0,		// old value 0
	plLast,			// old value 1
	plFirst,		// old value 2
	plMiddle,		// old value 3
	//plName, // no longer used
	plUserDefinedID,	// old value 5
	plBirthDate,	// old value 6
	plSSN,			// old value 7
	plHealthCard,	// old value 8
	plSecurityGroup, // (j.gruber 2010-10-04 14:34) - PLID 40415
	plForeColor, // (j.gruber 2011-07-22 17:07) - PLID 44118
};

// (j.gruber 2013-01-07 16:55) - PLID 54414
enum ReferringPhysColumns{
	rpcID = 0,
	rpcName = 1,
};

// (j.gruber 2012-07-26 15:27) - PLID 51830 - insurance column
enum InsuranceCatColumn
{
	rccID = 0,
	rccName,
};

CString TestFormatArrayOfDWords(DWORD arydw[], CString arystrids[], int nCount, bool bFlat)
{
	CString strDelim = bFlat ? ", " : "\n";

	CString strAns;
	for (int i=1; i<nCount; i++) {
		CString str;
		str.Format("%s:%lu%s", arystrids[i-1], arydw[i], strDelim);
		strAns += str;
	}
	if (strAns.GetLength() > strDelim.GetLength()) {
		strAns.Delete(strAns.GetLength()-strDelim.GetLength(), strDelim.GetLength());
	}
	return strAns;
}

// (c.haag 2010-03-08 17:05) - PLID 37326 - Version for CDWordArrays. Requires
// unique values in the arrays.
bool CompareArrays(const CDWordArray &ar1, const CDWordArray &ar2)
{
	if(ar1.GetSize() != ar2.GetSize()) return false;
	
	for(int i = 0; i < ar1.GetSize(); i++) {
		bool bMatched = false;
		for(int j = 0; j < ar2.GetSize() && !bMatched; j++) {
			if(ar1[i] == ar2[j]) bMatched = true;
		}
		if(!bMatched) return false;
	}
	return true;
}


// (a.walling 2010-03-03 08:13) - PLID 37612 - Modified all NxCatch* macros in this source file to use the CResEntryDlg_NxCatch* version, except for the Ignores.

#define CResEntryDlg_NxCatchMfc(message)	catch (CException *e) { bool b = m_bCheckActive; m_bCheckActive = false; HandleException(e, message, __LINE__, __FILE__); m_bCheckActive = b;}
#define CResEntryDlg_NxCatchCom(message)	catch (_com_error e) { bool b = m_bCheckActive; m_bCheckActive = false; HandleException(e, message, __LINE__, __FILE__); m_bCheckActive = b;}
#define CResEntryDlg_NxCatchLow(message)	catch (...) { bool b = m_bCheckActive; m_bCheckActive = false; HandleException(NULL, message, __LINE__, __FILE__); m_bCheckActive = b;}

#define CResEntryDlg_NxCatchAll(message)	CResEntryDlg_NxCatchMfc(message)	CResEntryDlg_NxCatchCom(message)	CResEntryDlg_NxCatchLow(message)



#define CResEntryDlg_NxCatchMfcCall(message, code)	catch (CException *e) { CKeepDialogActive kda(this); HandleException(e, message, __LINE__, __FILE__); code; }
#define CResEntryDlg_NxCatchComCall(message, code)	catch (_com_error e) { CKeepDialogActive kda(this); HandleException(e, message, __LINE__, __FILE__); code; }
#define CResEntryDlg_NxCatchLowCall(message, code)	catch (...) { CKeepDialogActive kda(this); HandleException(NULL, message, __LINE__, __FILE__); code; }

#define CResEntryDlg_NxCatchAllCall(message, code)		CResEntryDlg_NxCatchMfcCall(message, code)		CResEntryDlg_NxCatchComCall(message, code)		CResEntryDlg_NxCatchLowCall(message, code)

// (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling - Throw-style of these exception macros
#define CResEntryDlg_NxCatchMfcThrow(message)	catch (CException *e) { CKeepDialogActive kda(this); HandleException(e, message, __LINE__, __FILE__, FALSE); throw; }
#define CResEntryDlg_NxCatchComThrow(message)	catch (_com_error& e) { CKeepDialogActive kda(this); HandleException(e, message, __LINE__, __FILE__); throw; }
#define CResEntryDlg_NxCatchLowThrow(message)	catch (...) { CKeepDialogActive kda(this); HandleException(NULL, message, __LINE__, __FILE__); throw; }

#define CResEntryDlg_NxCatchAllThrow(message)			CResEntryDlg_NxCatchMfcThrow(message)			CResEntryDlg_NxCatchComThrow(message)			CResEntryDlg_NxCatchLowThrow(message)

#define CResEntryDlg_ASSERT(expr)			{ bool b = m_bCheckActive; m_bCheckActive = false; ASSERT(expr); m_bCheckActive = b; }


// (a.walling 2010-06-15 07:32) - PLID 30856
//CString GetModuleModifiedDate()
//{
//	try {
//		CString strExePath;
//		GetModuleFileName(NULL, strExePath.GetBuffer(4096), 4096);
//		strExePath.ReleaseBuffer();
//		if (!strExePath.IsEmpty()) {
//			CFileFind ff;
//			if (ff.FindFile(strExePath, 0)) {
//				ff.FindNextFile();
//				CTime tmpTime;
//				ff.GetLastWriteTime(tmpTime);
//				CString strFileDate;
//				strFileDate.Format("%02li/%02li/%li %li:%02li", 
//					tmpTime.GetMonth(), tmpTime.GetDay(), tmpTime.GetYear(), 
//					tmpTime.GetHour(), tmpTime.GetMinute());
//				return strFileDate;
//			} else {
//				return "<ERR: module not found>";
//			}
//		} else {
//			return "<ERR: no module>";
//		}
//	} catch(...) { return "<ERR: exception>"; }
//}
//
//static CFile l_fLogFilePlid17432;
//static BOOL l_bLogFilePlid17432Locked = FALSE;
//
//CFile *GetLogFilePlid17432()
//{
//	// See if the file's already open
//	if (l_fLogFilePlid17432.m_hFile != CFile::hFileNull) {
//		return &l_fLogFilePlid17432;
//	}
//	
//	// We'll probably need to open the log file; it's at this path.
//	CString strFilePath = GetPracPath(true) ^ "NexTech\\plid17432.log";
//	
//	// If we already know this log file is locked, there's no need to even try
//	if (l_bLogFilePlid17432Locked) {
//		// But, if the locked file was later removed for analysis, we should resume logging in 
//		// case we want to try to log the problem happening again.  (This way the user won't 
//		// have to close and re-open Practice just to get the logging to start up again.)
//		if (DoesExist(strFilePath)) {
//			// Yep, the locked file is still there, so don't mess with it
//			return NULL;
//		} else {
//			// The locked file was removed, so proceed normally, recreating a new log file
//			l_bLogFilePlid17432Locked = FALSE;
//		}
//	}
//	
//	// Make sure the parent folder exists
//	{
//		CString strFolderPath = GetFilePath(strFilePath);
//		if (!strFolderPath.IsEmpty() && !DoesExist(strFolderPath)) {
//			CreatePath(strFolderPath);
//		}
//	}
//	
//	// Try to open the file
//	if (l_fLogFilePlid17432.Open(strFilePath, CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite|CFile::shareDenyWrite, NULL)) {
//		// We opened the file, now see if it has the 'locked' indicator.
//		char chIsLocked;
//		if (l_fLogFilePlid17432.Read(&chIsLocked, 1) == 1 && chIsLocked == '1') {
//			// File is locked, which means the problem has been reproduced and appropriately 
//			// logged so there's no need to keep adding to the log file.
//			l_fLogFilePlid17432.Close();
//			l_bLogFilePlid17432Locked = TRUE;
//			return NULL;
//		} else {
//			// We're free to truncate the file and start from scratch
//			l_fLogFilePlid17432.SeekToBegin();
//			l_fLogFilePlid17432.SetLength(0);
//			// Write the 'unlocked' indicator
//			// Followed by the version/timestamp of this executable
//			CString strInitLog;
//			strInitLog.Format("0 - version: %s; modified date: %s\r\n", PRODUCT_VERSION_TEXT, GetModuleModifiedDate());
//			l_fLogFilePlid17432.Write((LPCTSTR)strInitLog, strInitLog.GetLength());
//			// Return
//			return &l_fLogFilePlid17432;
//		}
//	} else {
//		// Couldn't open the file, try again next time around.
//		return NULL;
//	}
//}
//
//void LogPlid17432(LPCTSTR strFormat, ...)
//{
//	CFile *pLog = GetLogFilePlid17432();
//	if (pLog && strFormat) {
//		// Convert the ... to and argumented string
//		CString strString;
//		va_list argList;
//		va_start(argList, strFormat);
//		strString.FormatV(strFormat, argList);
//		va_end(argList);
//		
//		// Format the string for output
//		CString strOutput;
//		strOutput.Format("%s:%5li: %s", COleDateTime::GetCurrentTime().Format("%c"), GetTickCount(), strString);
//		
//		// Output the string
//		pLog->Write(strOutput, strOutput.GetLength());
//	}
//}
//
//void LogPlid17432LockAndClose()
//{
//	CFile *pLog = GetLogFilePlid17432();
//	if (pLog) {
//		pLog->SeekToBegin();
//		pLog->Write("1", 1);
//		pLog->Close();
//		// Also rename it so we don't have to manually delete the log file just to get logging to resume
//		{
//			CString strPath = GetPracPath(true) ^ "NexTech";
//			CString strNewName;
//			strNewName.Format("plid17432_%s.log", COleDateTime::GetCurrentTime().Format("%Y%m%d-%H%M%S"));
//			MoveFile(strPath ^ "plid17432.log", strPath ^ strNewName);
//		}
//		// And remember we just locked it so next time we try to log we'll create a new log file
//		l_bLogFilePlid17432Locked = TRUE;
//	}
//}

CArray<CDurationGroup*, CDurationGroup*> m_aDurationGroups;


CResEntryDlg::CResDLSelection::CResDLSelection()
{
	m_bValid = FALSE;
}

CResEntryDlg::CResDLSelection::~CResDLSelection()
{
}

COleVariant CResEntryDlg::CResDLSelection::GetValue()
{
	return m_v;
}

void CResEntryDlg::CResDLSelection::SetValue(const COleVariant& v)
{
	m_v = v;
	SetValid(TRUE);
}

void CResEntryDlg::CResDLSelection::SetValid(BOOL bValid)
{
	m_bValid = bValid;
}

BOOL CResEntryDlg::CResDLSelection::IsValid()
{
	return m_bValid;
}

/////////////////////////////////////////////////////////////////////////////
// CResEntryDlg dialog

// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
CResEntryDlg::CResEntryDlg(CWnd* pParent, UINT nIDTemplate, CSchedulerView *pSchedViewParent)
	: CNxDialog(nIDTemplate, pParent),
	m_apttypeChecker(NetUtils::AptTypeT, false),
	m_aptpurposeChecker(NetUtils::AptPurposeT, false),
	m_ResourceChecker(NetUtils::Resources, false),
	m_locationChecker(NetUtils::LocationsT, false),
	m_showstateChecker(NetUtils::AptShowStateT, false),
	m_refPhysChecker(NetUtils::RefPhys, false), // (j.gruber 2013-01-07 15:10) - PLID 54414
	m_Res("CResEntryDlg"),
	m_ResID(-1),
	m_schedAuditItems(*(new SchedAuditItems))
	// (a.walling 2010-11-16 14:40) - PLID 41501 - Initialize m_ResID. Previously it was uninitialized, so if the first ResEntry popup is creating an appointment, 
	// CreatePatientInfoRecordset would run with an undefined appointment ID, although it is currently just used for CheckInTime and CheckOutTime.
{
	//{{AFX_DATA_INIT(CResEntryDlg)
	m_Notes = _T("");
	//}}AFX_DATA_INIT
	m_Res.SetDispatch(NULL);
	m_DoSave = false;
	m_PromptForCaseHistory = true;
	m_CheckboxesUpdated = false;
	m_ExtraActive = false;
	m_bExtraInfo = false;
	m_ActiveBorderColor = 0;
	m_ActiveStatusColor = RGB(255,255,236); // Default pending status color
	m_nOldStamp = 0;
	m_bAutoUpdate = true;
	m_bCheckActive = true;
	m_pSchedulerView = pSchedViewParent;
	m_nKeepActiveCount = 0;

	m_bIsCurPatientSet = FALSE;
	m_nCurPatientID = -1;

	m_nTemplateItemAptTypeID = -1;

	m_bIsCurLocationSet = FALSE;
	m_nCurLocationID = -1;
	m_nInitLocationID = -1;
	m_strCurLocationName = _T("");
	m_strResourceString = _T("");
	m_dtLastLM.SetStatus(COleDateTime::invalid);
	m_dtModified.SetStatus(COleDateTime::invalid);
	//TES 11/12/2008 - PLID 11465 - Added Input Date
	m_dtInput.SetStatus(COleDateTime::invalid);

	m_Pinned = NULL;
	m_Modified = NULL;

	m_nAptInitStatus = -1;

	// (j.gruber 2013-01-08 08:41) - PLID 54414
	m_bIsCurRefPhysSet = FALSE;
	m_nCurRefPhysID = -1;

	// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
	m_bIsMixRuleOverridden = false;

	// (c.haag 2010-03-08 16:52) - PLID 37326 - Reset initial values
	m_nAptInitType = -1;
	m_adwAptInitResources.RemoveAll();
	m_dtAptInitStartTime.SetStatus(COleDateTime::invalid);
	m_dtAptInitEndTime.SetStatus(COleDateTime::invalid);


	m_bShowArrivalTime = FALSE;
	m_bIsExtraInfoDirty = TRUE;

	m_bG1CustomFieldNameChanged = FALSE;

	// (z.manning, 01/08/2007) - PLID 23199 - Need to intialize this variable!
	m_nAptTypeDuration = 0;

	//TES 4/5/2007 - PLID 25512 - Need to initialize this one, too!
	m_nAptTypeID = -1;

	//TES 12/17/2008 - PLID 32479 - Changed the way these values are stored, they're not DDX'd to combo boxes any more.
	m_bMoveUp = FALSE;
	m_bOldMoveUp = FALSE; // (c.haag 2010-09-08 10:52) - PLID 37734
	m_nConfirmed = 0;
	m_bReady = FALSE;
	m_bWantedUpdateViewWhileReserved = FALSE; // (c.haag 2010-10-11 12:05) - PLID 40108

	//TES 10/27/2010 - PLID 40868 - Initialize the Calculated value to -2, meaning we need to re-calculate it.
	m_nCalculatedTemplateItemID = -2;
	m_nInitialTemplateItemID = -1;

	// (r.gonet 05/25/2012) - PLID 49059 - Init the location template label related data
	m_nLocationTemplateColor = 0;	
}


void CResEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResEntryDlg)
	DDX_Control(pDX, IDC_EXTRA_BOLD8, m_btnBold8);
	DDX_Control(pDX, IDC_EXTRA_BOLD7, m_btnBold7);
	DDX_Control(pDX, IDC_EXTRA_BOLD6, m_btnBold6);
	DDX_Control(pDX, IDC_EXTRA_BOLD5, m_btnBold5);
	DDX_Control(pDX, IDC_EXTRA_BOLD4, m_btnBold4);
	DDX_Control(pDX, IDC_EXTRA_BOLD3, m_btnBold3);
	DDX_Control(pDX, IDC_EXTRA_BOLD2, m_btnBold2);
	DDX_Control(pDX, IDC_EXTRA_BOLD1, m_btnBold1);
	DDX_Control(pDX, IDC_PIN_RESENTRY, m_btnPin);
	DDX_Text(pDX, IDC_NOTES_BOX, m_Notes);
	DDX_Control(pDX, IDC_DATE, m_date);
	DDX_Control(pDX, IDC_EVENT_DATE_START, m_dateEventStart);
	DDX_Control(pDX, IDC_EVENT_DATE_END, m_dateEventEnd);
	DDX_Control(pDX, IDC_NOTES_BOX, m_nxeditNotesBox);
	DDX_Control(pDX, IDC_RES_PATIENT_ID_BOX, m_nxeditResPatientIdBox);
	DDX_Control(pDX, IDC_RES_PATIENT_HOME_PHONE_BOX, m_nxeditResPatientHomePhoneBox);
	DDX_Control(pDX, IDC_RES_PATIENT_WORK_PHONE_BOX, m_nxeditResPatientWorkPhoneBox);
	DDX_Control(pDX, IDC_RES_PATIENT_BIRTH_DATE_BOX, m_nxeditResPatientBirthDateBox);
	DDX_Control(pDX, IDC_RES_PATIENT_CELL_PHONE_BOX, m_nxeditResPatientCellPhoneBox);
	DDX_Control(pDX, IDC_RES_PATIENT_PREFERRED_CONTACT_BOX, m_nxeditResPatientPreferredContactBox);
	DDX_Control(pDX, IDC_RES_PTBAL_BOX, m_nxeditResPtbalBox);
	DDX_Control(pDX, IDC_RES_INSBAL_BOX, m_nxeditResInsbalBox);
	DDX_Control(pDX, IDC_STATIC_MOVEUP, m_nxstaticMoveup);
	DDX_Control(pDX, IDC_STATIC_CONFIRMED, m_nxstaticConfirmed);
	DDX_Control(pDX, IDC_STATIC_CONFIRMED3, m_nxstaticConfirmed3);
	DDX_Control(pDX, IDC_STATIC_DATE, m_nxstaticDate);
	DDX_Control(pDX, IDC_STATIC_START_DATE, m_nxstaticStartDate);
	DDX_Control(pDX, IDC_STATIC_NOT_APPLICABLE, m_nxstaticNotApplicable);
	DDX_Control(pDX, IDC_STATIC_START, m_nxstaticStart);
	DDX_Control(pDX, IDC_STATIC_END, m_nxstaticEnd);
	DDX_Control(pDX, IDC_STATIC_ARRIVAL, m_nxstaticArrival);
	DDX_Control(pDX, IDC_STATIC_RESOURCE, m_nxstaticResource);
	DDX_Control(pDX, IDC_STATIC_LISTOFPURPOSES, m_nxstaticListofpurposes);
	DDX_Control(pDX, IDC_STATIC_END_DATE, m_nxstaticEndDate);
	DDX_Control(pDX, IDC_EXTRA_INFO_OFFSET_LABEL, m_nxstaticExtraInfoOffsetLabel);
	DDX_Control(pDX, IDC_REQUESTED_RESOURCE_LABEL, m_nxstaticRequestedResourceLabel);
	DDX_Control(pDX, IDC_REFERRING_PHYS_LABEL, m_nxstaticRefPhysLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_GOTOPATIENT, m_btnGotoPatient);
	DDX_Control(pDX, IDC_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MORE_INFO_GROUPBOX, m_btnMoreInfoGroupbox);
	DDX_Control(pDX, IDC_PATIENT_ID_CHECK, m_checkPatientID);
	DDX_Control(pDX, IDC_HOME_PHONE_CHECK, m_checkHomePhone);
	DDX_Control(pDX, IDC_WORK_PHONE_CHECK, m_checkWorkPhone);
	DDX_Control(pDX, IDC_BIRTH_DATE_CHECK, m_checkBirthDate);
	DDX_Control(pDX, IDC_CELL_PHONE_CHECK, m_checkCellPhone);
	DDX_Control(pDX, IDC_PREFERRED_CONTACT_CHECK, m_checkPreferredContact);
	DDX_Control(pDX, IDC_PATBAL_CHECK, m_checkPatBal);
	DDX_Control(pDX, IDC_INSBAL_CHECK, m_checkInsBal);
	DDX_Control(pDX, IDC_GOTO_INSURANCE, m_btnGotoInsurance);
	DDX_Control(pDX, IDC_NXBTN_CREATE_RECALL_SCHED, m_btnCreateRecall);
	DDX_Control(pDX, IDC_RES_INS_MORE, m_btnMoreInsurance);	
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CResEntryDlg, IDC_DATE, 4 /* DropDown */, OnDropDownDate, VTS_NONE)
//	ON_EVENT(CResEntryDlg, IDC_DATE, 3 /* CloseUp */, OnCloseUpDate, VTS_NONE)
//	ON_EVENT(CResEntryDlg, IDC_DATE, 2 /* Change */, OnChangeDate, VTS_NONE)
//	ON_EVENT(CResEntryDlg, IDC_EVENT_DATE_START, 4 /* DropDown */, OnDropDownDateEventStart, VTS_NONE)
//	ON_EVENT(CResEntryDlg, IDC_EVENT_DATE_START, 3 /* CloseUp */, OnCloseUpDateEventStart, VTS_NONE)
//	ON_EVENT(CResEntryDlg, IDC_EVENT_DATE_END, 4 /* DropDown */, OnDropDownDateEventEnd, VTS_NONE)
//	ON_EVENT(CResEntryDlg, IDC_EVENT_DATE_END, 3 /* CloseUp */, OnCloseUpDateEventEnd, VTS_NONE)

// (a.walling 2008-05-13 10:31) - PLID 27591 - Use notify events for CDateTimePicker
BEGIN_MESSAGE_MAP(CResEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CResEntryDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE, OnChangeDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATE, OnCloseUpDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_EVENT_DATE_START, OnCloseUpDateEventStart)
	ON_NOTIFY(DTN_CLOSEUP, IDC_EVENT_DATE_END, OnCloseUpDateEventEnd)
	ON_NOTIFY(DTN_DROPDOWN, IDC_DATE, OnDropDownDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_EVENT_DATE_START, OnDropDownDateEventStart)
	ON_NOTIFY(DTN_DROPDOWN, IDC_EVENT_DATE_END, OnDropDownDateEventEnd)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_DELETE_BTN, OnDeleteRes)
	ON_WM_ACTIVATE()
	ON_EN_CHANGE(IDC_NOTES_BOX, OnChangeNotesBox)
	ON_WM_KEYUP()
	ON_BN_CLICKED(IDC_EXTRA_INFO_BTN, OnExtraInfoBtn)
	ON_BN_CLICKED(IDC_PATIENT_ID_CHECK, OnPatientIdCheck)
	ON_BN_CLICKED(IDC_HOME_PHONE_CHECK, OnHomePhoneCheck)
	ON_BN_CLICKED(IDC_WORK_PHONE_CHECK, OnWorkPhoneCheck)
	ON_BN_CLICKED(IDC_CELL_PHONE_CHECK, OnCellPhoneCheck)
	ON_BN_CLICKED(IDC_BIRTH_DATE_CHECK, OnBirthDateCheck)
	ON_BN_CLICKED(IDC_PATBAL_CHECK, OnPatBalCheck)
	ON_BN_CLICKED(IDC_INSBAL_CHECK, OnInsBalCheck)
	ON_BN_CLICKED(IDC_PREFERRED_CONTACT_CHECK, OnPreferredContactCheck)
	ON_BN_CLICKED(IDC_GOTOPATIENT, OnGotopatient)
	ON_EN_KILLFOCUS(IDC_NOTES_BOX, OnKillfocusNotesBox)
	ON_BN_CLICKED(IDC_BTN_CHANGEPATIENT, OnBtnChangepatient)
	ON_BN_CLICKED(IDC_BTN_NEWPATIENT, OnBtnNewpatient)
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_APPOINTMENTLINKING, OnBtnAptLinking)
	ON_BN_CLICKED(IDC_SAVE_CREATE_INV_ALLOCATION, OnSaveCreateInvAllocation)
	ON_BN_CLICKED(IDC_SAVE_CREATE_INV_ORDER, OnSaveCreateInvOrder)
	ON_BN_CLICKED(IDC_SAVE_CREATE_BILL, OnSaveCreateBill)
	ON_BN_CLICKED(IDC_SAVE_CREATE_CASE_HISTORY, OnSaveCreateCaseHistory)
	ON_BN_CLICKED(IDC_SAVE_VIEW_MEDICATION_HISTORY, OnSaveViewMedicationHistory)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS1, OnSelchangeExtraFields1)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS2, OnSelchangeExtraFields2)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS3, OnSelchangeExtraFields3)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS4, OnSelchangeExtraFields4)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS5, OnSelchangeExtraFields5)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS6, OnSelchangeExtraFields6)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS7, OnSelchangeExtraFields7)
	ON_CBN_SELCHANGE(IDC_EXTRA_FIELDS8, OnSelchangeExtraFields8)
	ON_BN_CLICKED(IDC_PIN_RESENTRY, OnPinResentry)
	ON_MESSAGE(WM_HOTKEY,OnHotKey)
	ON_BN_CLICKED(IDC_EXTRA_BOLD1, OnExtraBold1)
	ON_BN_CLICKED(IDC_EXTRA_BOLD2, OnExtraBold2)
	ON_BN_CLICKED(IDC_EXTRA_BOLD3, OnExtraBold3)
	ON_BN_CLICKED(IDC_EXTRA_BOLD4, OnExtraBold4)
	ON_BN_CLICKED(IDC_EXTRA_BOLD5, OnExtraBold5)
	ON_BN_CLICKED(IDC_EXTRA_BOLD6, OnExtraBold6)
	ON_BN_CLICKED(IDC_EXTRA_BOLD7, OnExtraBold7)
	ON_BN_CLICKED(IDC_EXTRA_BOLD8, OnExtraBold8)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_BN_CLICKED(IDC_GOTO_INSURANCE, OnGotoInsurance)
	ON_BN_CLICKED(ID_GOTO_INSURANCE, OnGotoInsurance)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_NXBTN_CREATE_RECALL_SCHED, &CResEntryDlg::OnBnClickedNxbtnCreateRecallSched)
	ON_BN_CLICKED(IDC_RES_INS_MORE, &CResEntryDlg::OnBnClickedResInsMore)
	ON_COMMAND(IDM_RES_ADD_NEW_INS_PARTY, OnNewInsuredParty)
	ON_COMMAND(IDM_RES_CHOOSE_ADDITIONAL, OnChooseAdditionalInsParties)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CResEntryDlg, CNxDialog)
   //{{AFX_EVENTSINK_MAP(CResEntryDlg)
	ON_EVENT(CResEntryDlg, IDC_LIST_STATUS, 16 /* SelChosen */, OnSelChosenNoShowCombo, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTTYPE_COMBO, 2 /* SelChanged */, OnSelChangedApttypeCombo, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTPURPOSE_COMBO, 2 /* SelChanged */, OnSelChangedAptPurposeCombo, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTPURPOSE_COMBO, 1 /* SelChanging */, OnSelChangingAptPurposeCombo, VTS_PI4)
	ON_EVENT(CResEntryDlg, IDC_APTLOCATION_COMBO, 1 /* SelChanging */, OnSelChangingAptLocationCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResEntryDlg, IDC_APTTYPE_COMBO, 1 /* SelChanging */, OnSelChangingApttypeCombo, VTS_PI4)
	ON_EVENT(CResEntryDlg, IDC_APTRESOURCE_COMBO, 16 /* SelChosen */, OnSelChosenAptResourceCombo, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTPURPOSE_COMBO, 16 /* SelChosen */, OnSelChosenAptPurposeCombo, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTTYPE_COMBO, 16 /* SelChosen */, OnSelChosenAptTypeCombo, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTRESOURCE_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedAptResourceCombo, VTS_I4 VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTLOCATION_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedLocationCombo, VTS_I4 VTS_I4)
	
	ON_EVENT(CResEntryDlg, IDC_APTLOCATION_COMBO, 16 /* SelChosen */, OnSelChosenAptLocationCombo, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_ARRIVAL_TIME_BOX, 1 /* KillFocus */, OnKillFocusArrivalTimeBox, VTS_NONE)
	ON_EVENT(CResEntryDlg, IDC_START_TIME_BOX, 1 /* KillFocus */, OnKillFocusStartTimeBox, VTS_NONE)
	ON_EVENT(CResEntryDlg, IDC_END_TIME_BOX, 1 /* KillFocus */, OnKillFocusEndTimeBox, VTS_NONE)
	ON_EVENT(CResEntryDlg, IDC_ARRIVAL_TIME_BOX, 2 /* Changed */, OnChangedArrivalTimeBox, VTS_NONE)
	ON_EVENT(CResEntryDlg, IDC_START_TIME_BOX, 2 /* Changed */, OnChangedStartTimeBox, VTS_NONE)
	ON_EVENT(CResEntryDlg, IDC_END_TIME_BOX, 2 /* Changed */, OnChangedEndTimeBox, VTS_NONE)
	ON_EVENT(CResEntryDlg, IDC_REQUESTED_RESOURCE, 16 /* SelChosen */, OnSelChosenRequestedResource, VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_APTPURPOSE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedAptpurposeCombo, VTS_I2)
	ON_EVENT(CResEntryDlg, IDC_APTTYPE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedAptTypeCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CResEntryDlg, IDC_MOVEUP, 16, CResEntryDlg::OnSelChosenMoveup, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_CONFIRMED, 16, CResEntryDlg::OnSelChosenConfirmed, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_READY, 16, CResEntryDlg::OnSelChosenReady, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_MOVEUP, 1, CResEntryDlg::OnSelChangingMoveup, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResEntryDlg, IDC_CONFIRMED, 1, CResEntryDlg::OnSelChangingConfirmed, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResEntryDlg, IDC_READY, 1, CResEntryDlg::OnSelChangingReady, VTS_DISPATCH VTS_PDISPATCH)
	//ON_EVENT(CResEntryDlg, IDC_PATIENT_COMBO, 1 /* SelChanging */, OnSelChangingPatientCombo, VTS_PI4)
	//ON_EVENT(CResEntryDlg, IDC_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenPatientCombo, VTS_I4)
	//ON_EVENT(CResEntryDlg, IDC_PATIENT_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedPatientCombo, VTS_I4 VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_PATIENT_COMBO_2, 1, CResEntryDlg::SelChangingPatientCombo2, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResEntryDlg, IDC_PATIENT_COMBO_2, 16, CResEntryDlg::SelChosenPatientCombo2, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_PATIENT_COMBO_2, 20, CResEntryDlg::TrySetSelFinishedPatientCombo2, VTS_I4 VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_RES_INS_CAT, 16, CResEntryDlg::SelChosenResInsCat, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_RES_INS_CAT, 1, CResEntryDlg::SelChangingResInsCat, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CResEntryDlg, IDC_REFERRING_PHYS_LIST, 18, CResEntryDlg::RequeryFinishedReferringPhysList, VTS_I2)
	ON_EVENT(CResEntryDlg, IDC_REFERRING_PHYS_LIST, 16, CResEntryDlg::SelChosenReferringPhysList, VTS_DISPATCH)
	ON_EVENT(CResEntryDlg, IDC_REFERRING_PHYS_LIST, 20, CResEntryDlg::TrySetSelFinishedReferringPhysList, VTS_I4 VTS_I4)
	ON_EVENT(CResEntryDlg, IDC_REFERRING_PHYS_LIST, 1, CResEntryDlg::SelChangingReferringPhysList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResEntryDlg message handlers

void CResEntryDlg::ZoomResNewAppointment(LPDISPATCH theRes, IN const long nDefaultResourceID, IN const COleDateTime &dtDefaultDate)
{
	//Make sure this is identical with ZoomResExistingAppointment
	if(IsWindowVisible() && (m_Pinned || m_Modified)) {
		// (c.haag 2010-03-19 11:46) - PLID 37221 - Wrap this in CReservation first
		CReservation(__FUNCTION__, theRes).AddRef();
	
		//Oh, I see, they've pinned us, and then clicked on the schedule.  Fair enough.
		//TES 3/15/2004: Now, when we call OnCancel, theRes will no longer be valid.  
		//So let's remember some things about it.
		long nDay;
		DATE dtStart, dtEnd;
		OLE_COLOR cColor;
		CSingleDay pSingleDay;
		{
			CReservation pRes(__FUNCTION__, theRes);
			nDay = pRes.GetDay();
			dtStart = pRes.GetStartTime();
			dtEnd = pRes.GetEndTime();
			cColor = pRes.GetBorderColor();
			pSingleDay.SetDispatch( pRes.GetSingleDayCtrl() ); // (c.haag 2010-03-26 15:10) - PLID 37332 -  Use SetDispatch
		}		
		// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
		// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
		HandleCancelAndCloseReservation();
		if(IsWindowVisible()) {
			//They refused to Cancel.  Let's abandon this new appointment.
			{
				CReservation pRes(__FUNCTION__, theRes);
				//This is for our AddRef above.
				pRes.Release();
				//This is to actually remove the appointment.
				pRes.DeleteRes(__FUNCTION__);
			}

			return;
		}
		//TES 3/16/04: Now, we only want to do this if the dispatch pointer has actually been destroyed.  
		CReservation pTmp(__FUNCTION__, theRes);
		if(pTmp.GetSingleDayCtrl() == NULL) 
		{
			pSingleDay.AddReservation(__FUNCTION__, (short)nDay, dtStart, dtEnd, cColor, FALSE);
			//AddReservation will eventually lead to this function being called again, so...			
			pTmp.Release(); // (c.haag 2010-03-19 12:17) - PLID 37221 - Release from CReservation
			return;
		}
		else {
			pTmp.Release(); // (c.haag 2010-03-19 12:17) - PLID 37221 - Release from CReservation
		}
		
	}

	// (j.luckoski 2012-04-26 09:28) - PLID 11597 - Set m_bIsCancelled for NewAppointments as it will always be active
	// (j.luckoski 2012-06-14 13:06) - PLID 11597 - Call IsCancelled false after OnCancel is called.
	m_bIsCancelled = false;

	// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
	m_bIsMixRuleOverridden = false;
		
	// ASSERT the reservation thinks it's new
	#ifdef _DEBUG
	{
		CReservation pRes(__FUNCTION__, theRes);
		ASSERT(pRes.GetReservationID() == -1);
	}
	#endif

	// (c.haag 2004-03-31 13:29) PLID 11690 - Determine if we need to requery the type dropdown based on the
	// selected resource
	BOOL bRequeryTypes = (m_arydwSelResourceIDs.GetSize() == 1 && m_arydwSelResourceIDs[0] == (unsigned long)nDefaultResourceID) ? FALSE : TRUE ;

	// Initialize the resource array to have just the one resource in it
	m_arydwSelResourceIDs.RemoveAll();
	m_arydwSelResourceIDs.Add(nDefaultResourceID);
	m_dlAptResource->PutCurSel(sriNoRow);
	EnsureResourceComboVisibility();
	
	// Initialize the date
	m_ActiveDate.SetDate(dtDefaultDate.GetYear(), dtDefaultDate.GetMonth(), dtDefaultDate.GetDay());

	// Requery the types if necessary. We don't requery purposes because there is no type selected right now.
	if (bRequeryTypes) RequeryAptTypes();

	// New appointments never default to being modified
	SetModifiedFlag(false);
	
	// (j.gruber 2012-07-30 12:39) - PLID 51830 - just in case, clear our map
	ClearInsurancePlacements(TRUE);	

	// (j.gruber 2013-01-08 13:48) - PLID 54414 - just in case, clear the ref phys info
	m_nCurRefPhysID  = -1;
	m_strCurRefPhysName = "";
	m_bIsCurRefPhysSet = FALSE;
	m_pRefPhysList->PutCurSel(NULL);
	
	// Now go ahead
	ZoomResInternal(theRes);
}

void CResEntryDlg::ZoomResExistingAppointment(LPDISPATCH theRes)
{
	if(IsWindowVisible() && (m_Pinned || m_Modified)) {
		//Oh, I see, they've pinned us, and then clicked on the schedule.  Fair enough.
		// (c.haag 2010-02-04 12:18) - PLID 37221 - We no longer use IReservation*
		if(CReservation(__FUNCTION__, theRes).GethWnd() == m_Res.GethWnd()) {
			//They clicked on the appointment they already had open.  Fantastic!
			SetFocus();
			return;
		}

		//Now, keep in mind that OnCancel will sometimes cause our theRes pointer to be deleted.
		//Get its day and index.
		int nDay = 0;
		int nIndex = 0;
		CSingleDay pSingleDay;
		{
			CReservation pRes(__FUNCTION__, theRes);
			CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)pRes.GethWnd());
			pSingleDay.SetDispatch((LPDISPATCH)pWndNxTextbox->GetParent()->GetControlUnknown()); // (c.haag 2010-03-26 15:11) - PLID 37332 - Use SetDispatch
			nDay = pRes.GetDay();
			bool bFound = false;
			while(!bFound) {
				// (c.haag 2010-03-26 15:30) - PLID 37332 - Use HasReservation(). Don't call GetReservation() and discard the result;
				// while done internally, we don't want to expose Reservation dispatches outside CReservations, and we also don't
				// want to needlessly create CReservation objects.
				if(pSingleDay.HasReservation(nDay, nIndex)) {
					// (c.haag 2010-03-19 11:21) - PLID 37078 - All references to GethWnd() for reservations need to be encapsulated in CReservation
					if(pSingleDay.GetReservation(__FUNCTION__, nDay, nIndex).GethWnd() == pRes.GethWnd()) bFound = true;
					else nIndex++;
				}
				else {
					ASSERT(FALSE);
					return;
				}
				
			}
		}
		
		// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
		//OK, we've got everything we need, so cancel the old appointment.
		// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
		HandleCancelAndCloseReservation();
		//Now, see if it was actually cancelled.
		if(IsWindowVisible()) {
			return;
		}
		else {
			//OK, it was cancelled.  Now, is our old pointer still valid?
			CReservation pTmp(__FUNCTION__, theRes);
			if(pTmp.GetSingleDayCtrl() == NULL) 
			{
				//Nope, so let's rezoom (we know this isn't an infinite loop because of the IsWindowVisible().
				// (c.haag 2010-03-26 15:54) - PLID 37332 - Use GetReservationRaw because ZoomResExistingAppointment
				// takes in an LPDISPATCH. We intentionally don't want to change that legacy behavior, and we don't want
				// to add a reference to the reservation object when one was never added in the legacy code.
				ZoomResExistingAppointment(pSingleDay.GetReservationRaw(nDay, nIndex));
				return;
			}
		}

	}

	// (c.haag 2004-03-18 14:56) - If the appointment was modified and
	// the user changed their mind about changing it, then don't go
	// any further.
	if (m_Modified != NULL && m_Res != NULL)
	{
		return;
	}

	// ASSERT the reservation thinks it's existing
	#ifdef _DEBUG
	{
		CReservation pRes(__FUNCTION__, theRes);
		ASSERT(pRes.GetReservationID() != -1);
	}
	#endif

	// Clear the array to have just the one resource in it; this isn't really necessary 
	// because we're editing an existing appointment which means that as soon as we run 
	// our query we'll immediately re-clear the array and fill it with the query results.
	m_arydwSelResourceIDs.RemoveAll();
	m_dlAptResource->PutCurSel(sriNoRow);
	EnsureResourceComboVisibility();

	// Track the modified date of this appointment in case someone else
	// using Practice modifies the appointment
	{
		// (j.luckoski 2012-04-26 09:32) - PLID 11597 - Set m_bIsCancelled to true if the appt is cancelled
		CReservation pRes(__FUNCTION__, theRes);
		long nID = pRes.GetReservationID();
		if(nID != NULL || nID != -1) {
			// (a.walling 2013-01-21 16:48) - PLID 54744 - Use CancelledDate in Reserveration::Data property map
			if(pRes.Data["CancelledDate"].vt == VT_DATE) {
				m_bIsCancelled = true;
			}
			else {
				m_bIsCancelled = false;
			}
		}
		else {
			m_bIsCancelled = false;
		}
		m_pSchedulerView->TrackLastModifiedDate(nID);
	}
	
	// Existing appointments never default to being modified
	SetModifiedFlag(false);

	// Now go ahead
	ZoomResInternal(theRes);
}

void CResEntryDlg::ZoomResInternal(LPDISPATCH theRes)
{
	m_nWarnedForPatientID = -1;
	// (j.luckoski 2012-06-26 12:12) - PLID 11597 - Added m_bRevertActive to store when to update a res with the old text or cancelled/restored text
	m_bRevertActive = true;
	// (j.luckoski 2012-07-02 15:57) - PLID 11597 - Addeed my cancelled prefs as they will update before every open now.
	m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
	m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-14 14:16) - PLID 11597 - Add cancel color so we can change it upon click to seem more responsive
	m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
	// Make sure all expected conditions are met before we proceed
	{
		// (j.luckoski 2012-04-26 09:33) - PLID 11597 - If cancelled, disable recall btn
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) 
		{
			m_btnCreateRecall.AutoSet(NXB_RECALL); // (b.savon 2012-02-28 16:52) - PLID 48441 - Create Recall
			m_btnCreateRecall.ShowWindow(SW_SHOWNA);

			//(a.wilson 2012-3-23) PLID 48472 - check whether the current user has permission to create recalls.
			if ((GetCurrentUserPermissions(bioRecallSystem) & (sptCreate | sptCreateWithPass)) && !m_bIsCancelled) {
				m_btnCreateRecall.EnableWindow(TRUE);
			} else {
				m_btnCreateRecall.EnableWindow(FALSE);
			}
		}
		else
		{
			m_btnCreateRecall.EnableWindow(FALSE);
			m_btnCreateRecall.ShowWindow(SW_HIDE);
		}
		// We expect to have already been created
		ASSERT(m_hWnd);
		if (!m_hWnd) return;
		
		// We expect to not already be visible
		ASSERT(!IsWindowVisible());
		if (IsWindowVisible()) return;
		
		// We expect to not already be zoomed in on some other res
		ASSERT(m_Res == NULL);
		if (m_Res) return; // This should NEVER happen, if termination of dialog is done right

		// We expect to have been given an CReservation to zoom on
		ASSERT(theRes);
		if (!theRes) return;  // Might want to have this trigger a new reservation
	}

	m_bCheckActive = false;
	m_bIsExtraInfoDirty = TRUE;

	// Get pointer to box
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	m_Res.SetDispatch(theRes);

	// Get pointer to box window
	CWnd *pResWnd = CWnd::FromHandle((HWND)m_Res.GethWnd());
	ASSERT(pResWnd);  // this ASSERTs that m_Res actually exists...
	if (!pResWnd) {
		m_bCheckActive = true;
		return;    //TRP
	}

	// Get position of that window
	CRect rcResRect;
	pResWnd->GetWindowRect(rcResRect);
	rcResRect += CPoint(3, 3);

	CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379 
	m_OldText = (LPCTSTR)(m_Res.GetText());
	m_ActiveBorderColor = m_OldBorderColor = m_Res.GetBorderColor();
	m_ActiveStatusColor = m_OldStatusColor = RGB(255,255,236); // Default pending status color

	// Effectively turn off extra event handling
	Activate(FALSE);

	// Requery any lists that have changed from the outside
	bool bTypesChanged = m_apttypeChecker.Changed();
	bool bPurposesChanged = m_aptpurposeChecker.Changed();
	bool bResourceChanged = m_ResourceChecker.Changed();
	bool bLocationChanged = m_locationChecker.Changed();
	bool bShowStateChanged = m_showstateChecker.Changed();
	// (j.gruber 2013-01-07 15:11) - PLID 54414 - refphys
	bool bRefPhysChanged = m_refPhysChecker.Changed();

	if (bTypesChanged) {
		RequeryAptTypes();
	}
	if (bPurposesChanged) {
		RequeryAptPurposes();
	}
	if (bResourceChanged) {
		RequeryResources();
	}

	// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker,
	// these properties are cached, so just update the member variables from the cache
	m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
	m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

	if(bLocationChanged) {
		RequeryLocations(FALSE);
	}
	if(bShowStateChanged) {
		RequeryShowStates();
	}

	// (j.gruber 2013-01-07 15:12) - PLID 54414 - change ref phys
	if (bRefPhysChanged) {
		RequeryReferringPhys();
	}
	// (b.cardillo 2006-07-06 18:03) - PLID 21342 - If we have been told that one of the general1 custom 
	// field names has changed, then set up the more info combos again now.
	if (m_bG1CustomFieldNameChanged) {
		RepopulateExtraInfoCombos();
	}

	// Fill dialog with correct info
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	long ResID = m_Res.GetReservationID();
	if (ResID != -1) {
		// Returns false if the ID was not found

		// Disable the patient dropdown
		GetDlgItem(IDC_PATIENT_COMBO_2)->EnableWindow(FALSE); // (a.walling 2010-12-30 11:29) - PLID 40081 - Use datalist2 to avoid row index race conditions
		// Disable the new patient button
		if (GetDlgItem(IDC_BTN_NEWPATIENT))
			GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(FALSE);
		// Enable the change patient button
		if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
			GetDlgItem(IDC_BTN_CHANGEPATIENT)->EnableWindow(TRUE);

		if (MoveToRes(ResID) == FALSE) {
			m_Res.SetDispatch(NULL);
			m_bCheckActive = true;
			GetMainFrame()->UpdateAllViews();
			return;
		}
	} else {

		// Don't let the user make an appointment if the user does not have permissions
		//TES 1/14/2010 - PLID 36762 - Check whether they have access to this patient.
		if (!GetMainFrame()->CanAccessPatient(m_nCurPatientID, false)) {
			m_Res.SetDispatch(NULL);
			m_bCheckActive = true;
			GetMainFrame()->UpdateAllViews();
			return;
		}
		if (!CheckCurrentUserPermissions(bioAppointment, SPT____C______))
		{
			m_Res.SetDispatch(NULL);
			m_bCheckActive = true;
			GetMainFrame()->UpdateAllViews();
			return;
		}
		if(!CheckCurrentUserPermissions(bioSchedIndivResources,sptWrite,TRUE,GetCurResourceID_ExpectSingle())) {
			m_Res.SetDispatch(NULL);
			m_bCheckActive = true;
			GetMainFrame()->UpdateAllViews();
			return;
		}

		// Enable the patient dropdown
		GetDlgItem(IDC_PATIENT_COMBO_2)->EnableWindow(TRUE); // (a.walling 2010-12-30 11:29) - PLID 40081 - Use datalist2 to avoid row index race conditions
		// Enable the new patient button
		if (GetDlgItem(IDC_BTN_NEWPATIENT))
			GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(TRUE);
		// Disable the change patient button
		if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
			GetDlgItem(IDC_BTN_CHANGEPATIENT)->EnableWindow(FALSE);

		MakeNewRes();
	}
	if (m_Res == NULL)
		return;

	EnsureResourceComboVisibility();
	EnsurePurposeComboVisibility();

	//////////////////////////////////////
	// (b.cardillo 2003-07-07 15:01) Moved this function call.  We used to call it right before the 
	// above if-statement, but since it seems to depend on some properties of the appointment being 
	// edited, I thought it needed to be called only after the load (MoveToRes/MakeNewRes) has 
	// finished.  I checked this with c.haag and he confirms this call belongs after the if.
	// Fill the default duration list so that when the user chooses
	// a type and purpose, the EndTime will automatically fill in
	// with a default duration.
	RefreshDefaultDurationList();

	// (z.manning, 05/30/2007) - PLID 26177 - We used to fill default times in MakeNewRes, however, we call that before
	// we've loaded the default duration info, so it didn't work. So, we now do that here, but only if it's a new
	// appointment, of course.
	if(ResID == -1) {
		FillDefaultEndTime();
		FillDefaultArrivalTime();
	}
	//////////////////////////////////////

	// Effectively turn on extra event handling
	Activate();

	//Don't let a ToDo alarm pop up over us.
	GetMainFrame()->DisallowPopup();

	// Make current settings apparent in the res box
	ApplyToResBox();

	// (b.savon 2012-06-04 16:43) - PLID 49860 - This is now handled in CenterDialog(...)
/////// Make sure the window will be completely on the screen/////////
//	CRect dskRect, curRect;														//
//	GetDesktopWindow()->GetWindowRect(dskRect);							//
//	GetWindowRect(curRect);														//
//	if ((rcResRect.left + curRect.Width()) > dskRect.right)				//
//		rcResRect.left = dskRect.right - curRect.Width();					//
//	if ((rcResRect.top + curRect.Height()) > dskRect.bottom)			//
//		rcResRect.top = dskRect.bottom - curRect.Height();				//
//	if (rcResRect.left < dskRect.left)											//
//		rcResRect.left = dskRect.left;											//
//	if (rcResRect.top < dskRect.top)											//
//		rcResRect.top = dskRect.top;		                            //
//	if (rcResRect.top > 300)
//		rcResRect.top=rcResRect.top-29;
//////////////////////////////////////////////////////////////////////

	// Set the focus to the first data entry point
	CWnd *pWnd = NULL;
	if (ResID == -1) {
		// It's a new appointment, so set the focus to the patient combo
		GetDlgItem(IDC_PATIENT_COMBO_2)->SetFocus(); // (a.walling 2010-12-30 11:30) - PLID 40081 - Use datalist2 to avoid row index race conditions
	} else {
		// It's an existing appointment, so set the focus to the notes
		GetDlgItem(IDC_NOTES_BOX)->SetFocus();
	}

	// Enable/disable the Go To Patient Button as appropriate
	EnableGoToPatient(GetCurPatientID() != -25 ? TRUE : FALSE);

	if(!IsSurgeryCenter(FALSE)) {
		//if not ASC, remove the "create case history" item
		DisableSaveCreateCaseHistory();
	}
	else {
		//if ASC, change the case history text to reflect if there is a case history attached or not
		ToggleSaveCreateCaseHistory(!DoesCaseHistoryExist(ResID));
	}

	// (j.jones 2007-11-21 10:59) - PLID 28147 - added ability to create an inventory allocation,
	// but we need to disable it if they don't have inventory or create permissions
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)
		|| !g_pLicense->HasCandAModule(CLicense::cflrSilent)
		|| !(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS)) {
		DisableSaveCreateInvAllocation();
	}
	
	// (j.jones 2008-03-18 13:48) - PLID 29309 - added ability to create an inventory order
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)
		|| !g_pLicense->HasCandAModule(CLicense::cflrSilent)
		|| !(GetCurrentUserPermissions(bioInvOrder) & SPT____C_______ANDPASS)) {
		DisableSaveCreateInvOrder();
	}

	// (r.gonet 09/20/2013) - PLID 58416 - If the office does not have the surescripts license
	// or the current user lacks the permission to read the med history, disable the med history menu item.
	if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptSureScripts
		|| !(GetCurrentUserPermissions(bioPatientMedicationHistory) & (sptRead|sptReadWithPass))) 
	{
		DisableSaveViewMedicationHistory();
	}

	// Show window

	//TES 3/8/2011 - PLID 41519 - Check if we're highlighting appointments (but only if this isn't an event).
	// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
	if(!IsEvent() && GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true)) {
		//TES 3/8/2011 - PLID 41519 - This is now the "last edited" appointment, so tell the scheduler to highlight it
		CSingleDay pSingleDay;
		CReservation pRes(__FUNCTION__, theRes);
		CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)pRes.GethWnd());
		pSingleDay.SetDispatch((LPDISPATCH)pWndNxTextbox->GetParent()->GetControlUnknown()); // (c.haag 2010-03-26 15:11) - PLID 37332 - Use SetDispatch
		pSingleDay.SetHighlightRes(theRes);
	}

	// CAH 4/17
	if (IsEvent()) {
		// Hide time boxes
		GetDlgItem(IDC_START_TIME_BOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_END_TIME_BOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ARRIVAL_TIME_BOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_START)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_END)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_ARRIVAL)->ShowWindow(SW_HIDE);

		// Hide MoveUp and Confirmed boxes
		//GetDlgItem(IDC_STATIC_MOVEUP)->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_MOVEUP_COMBO)->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_STATIC_CONFIRMED)->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_CONFIRMED_COMBO)->ShowWindow(SW_HIDE);

		// Hide one-day date box
		GetDlgItem(IDC_STATIC_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DATE)->ShowWindow(SW_HIDE);

		// Show start date and end date box
		GetDlgItem(IDC_STATIC_START_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EVENT_DATE_START)->ShowWindow(SW_SHOW);

		// CAH: Lets keep the end date window still hidden because we
		// don't yet support truly multi-day events
		GetDlgItem(IDC_STATIC_END_DATE)->ShowWindow(SW_HIDE/*SW_SHOW*/);
		GetDlgItem(IDC_EVENT_DATE_END)->ShowWindow(SW_HIDE/*SW_SHOW*/);
	}
	else {
		GetDlgItem(IDC_START_TIME_BOX)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_END_TIME_BOX)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ARRIVAL_TIME_BOX)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_START)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_END)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_ARRIVAL)->ShowWindow(SW_SHOW);

		
		//TES 12/17/2008 - PLID 32479 - Make sure to pass SW_SHOWNA to datalists, otherwise they take focus
		GetDlgItem(IDC_STATIC_MOVEUP)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MOVEUP)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_STATIC_CONFIRMED)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CONFIRMED)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_READY)->ShowWindow(SW_SHOWNA);


		GetDlgItem(IDC_STATIC_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DATE)->ShowWindow(SW_SHOW);

		// Hide start date and end date box
		GetDlgItem(IDC_STATIC_START_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EVENT_DATE_START)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_END_DATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EVENT_DATE_END)->ShowWindow(SW_HIDE);

		// (r.gonet 05/25/2012) - PLID 49059 - Update the location template name and color
		RefreshLocationTemplate();
	}
	
	// (z.manning, 4/18/2008, PLID 19088) - Hide arrival time unless they have the preference set.
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
	if(GetRemotePropertyInt("ShowArrivalTimeOnAppts", 1, 0, "<None>", true) != 0) {
		ShowArrivalTime(TRUE);
	}
	else {
		ShowArrivalTime(FALSE);
	}

	// (j.jones 2010-09-03 11:45) - PLID 40397 - load the user's last extra info setting
	m_bExtraInfo = GetRemotePropertyInt("ResEntry_RememberShowExtraInfo", 0, 0, GetCurrentUserName(), true) == 1;
	ShowExtraInfo(m_bExtraInfo);

	// See if this is a block time; and if so, change the patient
	// combo box accordingly
	// (c.haag 2006-05-08 15:47) - PLID 20482 - No more waiting indefinitely. Before this
	// punch list item was worked on, GetCurAptTypeCategory() was already made wait-safe
	//m_dlAptType->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	short nCat = GetCurAptTypeCategory();
	if (nCat == PhaseTracking::AC_BLOCK_TIME)
		OnBlockTimeCatSelected(TRUE);
	else
		OnBlockTimeCatSelected(FALSE);
	

	// (b.savon 2012-06-04 16:43) - PLID 49860 - This is now handled in CenterDialog(...)
	//SetWindowPos(&wndNoTopMost, rcResRect.left, rcResRect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	
	if (ResID != -1) {
		// Due to what appears to be a Win32 glitch, we have to scroll-caret AFTER the window is visible
		GetDlgItem(IDC_NOTES_BOX)->SendMessage(EM_SCROLLCARET, 0, 0);
	}

	// (j.jones 2007-11-21 11:29) - PLID 28147 - converted to enums
	RegisterHotKey(this->m_hWnd,hkSave,MOD_ALT,'S');
	RegisterHotKey(this->m_hWnd,hkSaveGoToPatient,MOD_ALT,'G');
	RegisterHotKey(this->m_hWnd,hkSaveCreateInvAlloc,MOD_ALT,'A');
	RegisterHotKey(this->m_hWnd,hkSaveCreateCaseHist,MOD_ALT,'H');
	RegisterHotKey(this->m_hWnd,hkSaveCreateNewPat,MOD_ALT,'N');
	RegisterHotKey(this->m_hWnd,hkChangePatient,MOD_ALT,'P');
	RegisterHotKey(this->m_hWnd,hkApptLinking,MOD_ALT,'L');
	RegisterHotKey(this->m_hWnd,hkShowMoreInfo,MOD_ALT,'I');
	RegisterHotKey(this->m_hWnd,hkSaveCreateInvOrder,MOD_ALT,'O');	// (j.jones 2008-03-18 13:49) - PLID 29309
	RegisterHotKey(this->m_hWnd,hkSaveCreateBill,MOD_ALT,'B');	// (j.jones 2008-06-24 10:06) - PLID 30455
	RegisterHotKey(this->m_hWnd,hkSaveViewMedicationHistory,MOD_ALT,'M'); // (r.gonet 12/16/2013) - PLID 58416
	m_bCheckActive = true;

	//DRT - 6/4/03 - New appointment, make sure it's allowed by ins auths
	{
		m_bCheckActive = false;
		bool bContinue = AttemptWarnForInsAuth(GetCurPatientID(), GetDate());
		m_bCheckActive = true;

		if(!bContinue) {
			//they said no they don't want to continue.  Cancel the dialog
			// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
			HandleCancelAndCloseReservation();
			return;
		}
	}
	//done

	// (c.haag 2004-03-17 09:39) PLID 11439 - Highlight the colors on the time column
	// that correspond to this appointment
	HighlightSheetTimeColumn(TRUE);
	
	if(GetRemotePropertyInt("ShowPackageInfo", 0, 0, "<None>", true)){
		CKeepDialogActive kda(this);
		ShowPackageInfo(GetCurPatientID());
	}

	// (c.haag 2003-10-06 11:05) - Warn if the patient is inactive.
	WarnIfInactivePatientSelected();

	// (c.haag 2004-03-26 09:15) - Disable the reservation so noone can move or click on it
	if (m_Res != NULL)
		m_Res.PutMouseEnabled(FALSE);

		// (j.luckoski 2012-04-26 09:33) - PLID 11597 - If cancelled, disable a few items and change window text
		if(m_bIsCancelled) {
			SetDlgItemText(IDC_DELETE_BTN, "&Restore Appointment");
			GetDlgItem(IDC_GOTOPATIENT)->EnableWindow(FALSE);
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			GetDlgItem(IDC_LIST_STATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVEUP)->EnableWindow(FALSE);
			GetDlgItem(IDC_CONFIRMED)->EnableWindow(FALSE);
			GetDlgItem(IDC_READY)->EnableWindow(FALSE);
			GetDlgItem(IDC_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_APTLOCATION_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_PATIENT_COMBO_2)->EnableWindow(FALSE);
			GetDlgItem(IDC_START_TIME_BOX)->EnableWindow(FALSE);
			GetDlgItem(IDC_END_TIME_BOX)->EnableWindow(FALSE);
			GetDlgItem(IDC_ARRIVAL_TIME_BOX)->EnableWindow(FALSE);
			GetDlgItem(IDC_APTRESOURCE_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_APTTYPE_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_REQUESTED_RESOURCE)->EnableWindow(FALSE);
			GetDlgItem(IDC_NOTES_BOX)->EnableWindow(FALSE);
			// (j.gruber 2013-01-03 14:48) - PLID 54414
			GetDlgItem(IDC_REFERRING_PHYS_LIST)->EnableWindow(FALSE);
			//GetDlgItem(IDC_EVENT_DATE_END)->EnableWindow(FALSE);
			DisableMenuForCancelled(FALSE);

			// (j.gruber 2013-05-07 10:13) - PLID 56568 - disable the insurance fields
			GetDlgItem(IDC_RES_INS_CAT)->EnableWindow(FALSE);
			GetDlgItem(IDC_RES_PRI_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_RES_SEC_LABEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_RES_INS_MORE)->EnableWindow(FALSE);

		} else {
			SetDlgItemText(IDC_DELETE_BTN, "&Cancel Appointment");
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDC_LIST_STATUS)->EnableWindow(TRUE);
			GetDlgItem(IDC_MOVEUP)->EnableWindow(TRUE);
			GetDlgItem(IDC_CONFIRMED)->EnableWindow(TRUE);
			GetDlgItem(IDC_READY)->EnableWindow(TRUE);
			GetDlgItem(IDC_DATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_APTLOCATION_COMBO)->EnableWindow(TRUE);
			//(r.wilson 10/10/2012) plid 53107- There is logic earlier that determines if the patient drop down will be enabled or not.
			//									Since this is not cancelled then just let the earlier logic do its work instead of always defaulting to enabled
			//GetDlgItem(IDC_PATIENT_COMBO_2)->EnableWindow(TRUE);
			GetDlgItem(IDC_START_TIME_BOX)->EnableWindow(TRUE);
			GetDlgItem(IDC_END_TIME_BOX)->EnableWindow(TRUE);
			GetDlgItem(IDC_ARRIVAL_TIME_BOX)->EnableWindow(TRUE);
			GetDlgItem(IDC_APTRESOURCE_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_APTTYPE_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);
			GetDlgItem(IDC_REQUESTED_RESOURCE)->EnableWindow(TRUE);
			GetDlgItem(IDC_NOTES_BOX)->EnableWindow(TRUE);
			//GetDlgItem(IDC_EVENT_DATE_END)->EnableWindow(TRUE);
			DisableMenuForCancelled(TRUE);
			// (j.gruber 2013-01-03 14:49) - PLID 54414 - referring phys list
			GetDlgItem(IDC_REFERRING_PHYS_LIST)->EnableWindow(TRUE);

			// (j.gruber 2013-05-07 10:13) - PLID 56568 - enable the insurance fields
			GetDlgItem(IDC_RES_INS_CAT)->EnableWindow(TRUE);
			GetDlgItem(IDC_RES_PRI_LABEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_RES_SEC_LABEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_RES_INS_MORE)->EnableWindow(TRUE);
		}

		// (b.savon 2012-06-04 16:44) - PLID 49860 - Once we've done all the dirty work, center the dialog
		CenterDialog();

}

// (b.savon 2012-06-04 16:44) - PLID 49860 - Center the dialog
// Based off of http://support.microsoft.com/KB/102328
void CResEntryDlg::CenterDialog()
{
	CPoint   Point;
	CRect    rectRes;
	CRect    rectParent;
	int      nWidth;
	int      nHeight;
	CWnd     *DesktopWindow = NULL;
	CWnd     *MainWindow = AfxGetApp()->m_pMainWnd;

	// Get the size of the dialog box.
	GetWindowRect(rectRes);

	// Determine if the main window exists. This can be useful when
	// the application creates the dialog box before it creates the
	// main window. If it does exist, retrieve its size to center
	// the dialog box with respect to the main window.
	if (MainWindow != NULL){
		MainWindow->GetClientRect(rectParent);
	// If the main window does not exist, center with respect to
	// the desktop window.
	}else{
		DesktopWindow = GetDesktopWindow();
		DesktopWindow->GetWindowRect(rectParent);
	}

	// Calculate the height and width for MoveWindow().
	nWidth = rectRes.Width();
	nHeight = rectRes.Height();

	// Find the center point and convert to screen coordinates.
	Point.x = rectParent.Width() / 2;
	Point.y = rectParent.Height() / 2;
	if (MainWindow){
		MainWindow->ClientToScreen(&Point);
	}
	else{
		DesktopWindow->ClientToScreen(&Point);
	}

	// Calculate the new X, Y starting point.
	Point.x -= nWidth / 2;
	Point.y -= nHeight / 2;

	// Move it...
	// (a.walling 2013-04-18 12:41) - PLID 56333 - If not using desktop composition (eg, XP or with Aero disabled) the resentry dialog may draw funky
	// ensure we pass TRUE so the windows affected are repainted.
	MoveWindow(Point.x, Point.y, nWidth, nHeight, TRUE);

	// ...Show it!
  	ShowWindow(SW_SHOW);
}//END void CResEntryDlg::CenterDialog()

void CResEntryDlg::MakeNewRes()
{
	CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
	m_DoSave = false;
// m_Res.GetStartTime() returns 0 sometimes, giving 12 am
	// Set attributes
	m_date.SetValue((COleVariant)m_ActiveDate);
	m_dateEventStart.SetValue((COleVariant)m_ActiveDate);
	m_dateEventEnd.SetValue((COleVariant)m_ActiveDate);

	// Find out if this is an event or an appointment
	CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)m_Res.GethWnd());
	CWnd* pWndSingleDay = pWndNxTextbox->GetParent();
	CNxSchedulerDlg* pWndScheduler = (CNxSchedulerDlg*)pWndSingleDay->GetParent();

	m_pSchedulerView->ResetTrackedAppt();

	if ((pWndScheduler->m_pEventCtrl != NULL) && 
		((HWND)pWndScheduler->m_pEventCtrl.GethWnd()) == pWndSingleDay->GetSafeHwnd() /* If this was made in an event control */)
	{
		m_dtStartTime.SetTime(0,0,0);
		m_dtEndTime.SetTime(0,0,0);
	}
	else
	{
		if (m_Res) {
			m_dtStartTime = COleDateTime(m_Res.GetStartTime());
			m_dtEndTime = COleDateTime(m_Res.GetEndTime());
		} else {
			m_dtStartTime.SetTime(3,0,0);
			m_dtEndTime.SetTime(3,0,0);
		}
	}
	m_dtArrivalTime = m_dtStartTime;

	m_Notes = "";
	long tmpID = -25;
	//TES 2004-1-8: We've now set all of our DDX variables, let's reflect them on screen right away, so that none of the
	//following code (such as SetDefaultEndTime) gets confused.
	UpdateData(FALSE);

	//TES 12/17/2008 - PLID 32479 - Set the values and the controls, now that these aren't combo boxes any more.
	m_bMoveUp = FALSE;
	m_bOldMoveUp = FALSE; // (c.haag 2010-09-08 10:52) - PLID 37734
	m_pMoveUpCombo->SetSelByColumn(0,(long)0);

	m_nConfirmed = 0;
	m_pConfirmedCombo->SetSelByColumn(0,(long)0);

	m_bReady = FALSE;
	m_pReadyCombo->SetSelByColumn(0,(long)0);

	if (!IsEvent()) {
		// If it's not going to be an event, use the active patient as the default
		// (c.haag 2003-07-16 17:32) - ...so long as the user prefers it.
		if (!GetRemotePropertyInt("NewApptPatientState", 0, 0, "<None>", true)) {
			//TES 1/14/2010 - PLID 36762 - This is fine, GetActivePatientID() will always return an accessible patient.
			tmpID = GetActivePatientID();
		}
	}

	// Continue with display of values
	SetCurPatient(tmpID);

	//TES 2004-01-30: Store this for use on the extra info section, we know it won't change.
	m_strInputName = GetCurrentUserName();
	m_strModifiedLogin = GetCurrentUserName();
	m_dtLastLM.SetStatus(COleDateTime::invalid);
	m_dtModified = COleDateTime::GetCurrentTime();
	//TES 11/12/2008 - PLID 11465 - Added Input Date
	m_dtInput = m_dtModified;

	_variant_t varNull;
	varNull.vt = VT_NULL;
	m_dlAptType->SetSelByColumn(0, varNull);
	//With this line of code, we state that from now on, the apt type information should be read solely
	//from the datalist.
	m_bValidAptTypeStored = false;

	// (r.farnworth 2015-08-13 11:10) - PLID 64009 - Commented out
	//if (m_dlAptPurpose == NULL) m_dlAptPurpose = BindNxDataListCtrl(this, IDC_APTPURPOSE_COMBO, GetRemoteData(), false);
	m_dlAptPurpose->Clear();
	m_dlAptPurpose->WhereClause = "";
	GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
	if (GetDlgItem(IDC_BTN_NEWPATIENT))
		GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(TRUE);

	//0 - no location by default
	//1 - set the location to the currently logged in location
	//2 - set the location to the patient's location
	//3 - set the location to the resource's location
	long nAptLoc = GetRemotePropertyInt("ApptLocationPref", 1, 0, "<None>", true);
	if(nAptLoc == 0) {
		// (z.manning 2011-07-01 12:49) - PLID 32004 - Added an option to default to no location
		SetCurLocation(-1, "");
	}
	else if(nAptLoc == 1) {
		// Set the location to the currently logged in location
		SetCurLocation(GetCurrentLocationID(), GetCurrentLocationName());
	}
	else if (nAptLoc == 2) {
		// Set the location to the patient's location
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Location, Name FROM PersonT INNER JOIN LocationsT ON PersonT.Location = LocationsT.ID WHERE PersonT.ID = {INT}",m_nCurPatientID);
		if(!rs->eof) {
			SetCurLocation(AdoFldLong(rs, "Location"), AdoFldString(rs, "Name",""));
		}
		else {
			// Set the location to the currently logged in location
			SetCurLocation(GetCurrentLocationID(), GetCurrentLocationName());
		}
		rs->Close();
	}
	else
	{
		//TES 8/3/2010 - PLID 39264 - First, see if there's a location template at this time slot, that will be our first choice.
		long nLocationID = pWndScheduler->m_ResourceAvailLocations.GetLocationID(m_Res.GetDay(), m_dtStartTime);
		if(nLocationID != -1) {
			//TES 8/3/2010 - PLID 39264 - There was one, so use it.			
			// (a.walling 2010-06-23 11:19) - PLID 39321 - Use the remote data cache to minimize sql server accesses
			SetCurLocation(nLocationID, GetRemoteDataCache().GetLocationName(nLocationID));
		}
		else {
			//TES 8/3/2010 - PLID 39264 - There wasn't, so move on to our pre-existing checks (resource location, then logged-in location)

			// (c.haag 2003-07-30 16:10) - Set the location to the resource's location
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT LocationID, Name FROM ResourceLocationConnectT LEFT JOIN LocationsT ON LocationsT.ID = LocationID WHERE ResourceID = {INT}", GetCurResourceID_ExpectSingle());
			if (!rs->eof)
			{
				SetCurLocation(AdoFldLong(rs, "LocationID"), AdoFldString(rs, "Name",""));
			}
			else {
				// (c.haag 2003-07-30 16:15) - If a resource has no location, Practice
				// defaults it to the first available managed location
				//(e.lally 2010-07-19) PLID 39628 - We are allowing resources to have no default location assigned. When that is the case,
				//we are going to use the currently logged in location like the other branches.
				
				// Set the location to the currently logged in location
				SetCurLocation(GetCurrentLocationID(), GetCurrentLocationName());
			}
			rs->Close();
		}
	}

	// CAH 5/9/03: The resource TrySetSel is already safely handled; we don't need
	// to do what we do with types, location and patient combos.
	long nResourceSelIndex = m_dlAptResource->TrySetSelByColumn(0, GetCurResourceID_ExpectSingle());
	if (nResourceSelIndex == sriNoRow) {
		// Failed to get a row, we need to tell the user that they'll have to manually select a 
		// resource (this is just like the OnTrySetSelFinishedAptResourceCombo handler)
		m_arydwSelResourceIDs.RemoveAll();
		m_dlAptResource->PutCurSel(sriNoRow);
		EnsureResourceComboVisibility();
	} else {
		// Either we set the row successfully (which was our whole goal) or we got an 
		// sriNoRowYet_WillFireEvent which means the OnTrySetSelFinishedAptResourceCombo will handle it)
		ASSERT(nResourceSelIndex == sriNoRowYet_WillFireEvent || nResourceSelIndex >= 0);
	}

	//TES 4/19/04: Now set our new "requested resource" based on their preferences.
	if(GetRemotePropertyInt("RequestedResourceDefaultNone", 1, 0, GetCurrentUserName(), true)) {
		m_dlRequestedResource->PutCurSel(sriNoRow);
	}
	else {
		m_dlRequestedResource->SetSelByColumn(0, GetCurResourceID_ExpectSingle());
	}
	
	m_adwPurposeID.RemoveAll();
	EnsurePurposeComboVisibility();

	CalculateDefaultAptTypeByTemplateItemID();

	//DRT 7/21/03 - Added a preference to allow users to auto-select the Type and Purpose(s) based
	//		on the last time the patient was in.
	AutoSelectTypePurposeByPatient();

	// (c.haag 2006-12-07 16:33) - PLID 23767 - Now try to auto-fill in purposes
	// based on existing preferences...but only if we are a template block
	if (-1 != m_Res.GetTemplateItemID() && -1 != GetCurAptTypeID()) {
		TryAutoFillPurposes();
	}

	m_dlNoShow->TrySetSelByColumn(0, (long)0);

	long nTempCurSelStatus = m_dlNoShow->CurSel;
	if(nTempCurSelStatus == -1){
		m_nAptInitStatus = -1;
	}
	else{
		m_nAptInitStatus = VarLong(m_dlNoShow->Value[nTempCurSelStatus][0]);
	}

	// (c.haag 2010-03-08 16:53) - PLID 37326 - Set initial values
	m_nAptInitType = GetCurAptTypeID();
	m_adwAptInitResources.Copy(m_arydwSelResourceIDs);
	m_dtAptInitStartTime = m_dtStartTime;
	m_dtAptInitEndTime = m_dtEndTime;

	// Apply everything
	ObtainActiveBorderColor();

	
	// (a.walling 2010-11-16 14:40) - PLID 41501 - Set the m_nResID before calling ApplyCurrentPatient; otherwise it will have the previous value, which was uninitialized to begin with
	m_NewRes = true;
	m_ResID = -1;
	m_nNewResID = -1;
	

	// (c.haag 2006-12-05 09:34) - PLID 23666 - This new reservation may be based off a template line item
	// (c.haag 2006-12-11 10:46) - PLID 23808 - Don't assign it if the template item no longer exists
	//TES 10/27/2010 - PLID 40868 - Pull the InitialTemplateItemID, then copy it to the Calculated variable.
	// (a.walling 2015-02-05 12:55) - PLID 64416 - Calculate the clicked-on templateitem before calling the Apply functions
	if (m_Res && (-1 != m_Res.GetTemplateItemID())) {
		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		if (ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM TemplateItemT WHERE ID = {INT}", m_Res.GetTemplateItemID())) {
			m_nInitialTemplateItemID = m_Res.GetTemplateItemID();
		} else {
			MsgBox("The template item has been deleted by another user, but you may still create a new appointment.");
			m_nInitialTemplateItemID = -1;
		}
	} else {
		m_nInitialTemplateItemID = -1;
	}
	//TES 2/25/2011 - PLID 40868 - If we don't have an initial template item, go ahead and still act like we need to calculate it.  That just
	// means they clicked outside of a template (on white space or on the time on the left), but it's still entirely possible that we match one.
	if(m_nInitialTemplateItemID != -1) {
		m_nCalculatedTemplateItemID = m_nInitialTemplateItemID;
	}
	else {
		m_nCalculatedTemplateItemID = -2;
	}

	// (j.jones 2014-11-14 10:30) - PLID 64116 - now takes in a boolean to state whether
	// the user has changed the patient on the appointment, true here because we're setting
	// the patient on a new appointment
	ApplyCurrentPatient(true);
	ApplyToResBox();

	// Disable and hide the cancel appointment button, since this a new appointment
	{
		CWnd *pDelBtn = GetDlgItem(IDC_DELETE_BTN);
		pDelBtn->EnableWindow(FALSE);
		pDelBtn->ShowWindow(SW_HIDE);
		EnableCancelAppointmentMenuOption(FALSE);

		// (z.manning 2008-11-24 10:06) - PLID 31130 - Enable and show the save and go to insurance
		// button on new appointments.
		GetDlgItem(IDC_GOTO_INSURANCE)->EnableWindow(TRUE);
		GetDlgItem(IDC_GOTO_INSURANCE)->ShowWindow(SW_SHOW);
	}

	COleDateTime dtStart, dtEnd;
	dtStart = m_dtStartTime;
	dtEnd = m_dtEndTime;

	// (c.haag 2009-01-19 12:01) - PLID 32712 - Copy initial purpose list
	m_adwAptInitPurposes.Copy(m_adwPurposeID);

	// If someone made an appointment in such a way that the end time is
	// 12:00am and the start time is not 12:00am, we assume the appt spilled
	// into the next day, and therefore, we have to change the end time to
	// 11:59pm
	if (dtEnd.m_dt == 0 /* 12:00am */ && dtStart.m_dt != 0 /* 12:00am */)
		dtEnd.SetTime(23,59,0);

	ASSERT(dtStart <= dtEnd); // dtStart = dtEnd for events

	m_nxtStart->SetDateTime(dtStart);	//setup start time
	m_nxtEnd->SetDateTime(dtEnd);	//setup end time
}


BOOL CResEntryDlg::MoveToRes(long ResID)
{
	m_DoSave = false;

	// Open the record
	try {

		// (d.moore 2007-05-22 10:45) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
		//TES 11/12/2008 - PLID 11465 - Added Input Date
		// (c.haag 2008-11-20 16:35) - PLID 31128 - Added InsuranceReferralSummary
		// (z.manning 2009-06-30 17:11) - PLID 34744 - I have no idea why ins ref summary was added here,
		// but it is not needed so I took it out.
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (j.gruber 2012-07-30 11:56) - PLID 51830 - Added insurance information
		// (j.gruber 2013-01-07 15:57) - PLID 54414 - added referring physician
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		// (j.jones 2014-12-03 10:58) - PLID 64274 - added IsMixRuleOverridden, always false if the appt. is cancelled
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), 
			" SET XACT_ABORT ON; "
			"SELECT PatientID, StartTime, EndTime, ArrivalTime, "
			"CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID={INT}) THEN 1 ELSE 0 END) AS MoveUp, "
			"Ready, Confirmed, ShowState, Notes, AptTypeID, AptPurposeID, LocationID, CreatedLogin, LastLM, ModifiedDate, ModifiedLogin, RequestedResourceID, "
			"dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDString, AptTypeT.Color AS AptTypeColor, AptShowStateT.Color AS AptStatusColor, AppointmentsT.RefPhysID as RefPhysID, "
			//"dbo.InsReferralSummary(AppointmentsT.PatientID, GetDate()) AS InsuranceReferralSummary, "
			"CreatedDate, "
			"Convert(bit, CASE WHEN AppointmentMixRuleOverridesQ.AppointmentID Is Not Null THEN 1 ELSE 0 END) AS IsMixRuleOverridden "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID "
			"LEFT JOIN AptShowStateT ON AptShowStateT.ID = AppointmentsT.ShowState "
			"LEFT JOIN ("
			"	SELECT AppointmentID "
			"	FROM AppointmentMixRuleOverridesT "
			"	GROUP BY AppointmentID "
			") AS AppointmentMixRuleOverridesQ ON AppointmentsT.ID = AppointmentMixRuleOverridesQ.AppointmentID AND AppointmentsT.Status <> 4"
			"WHERE AppointmentsT.ID = {INT};"
			"\r\n"
			" SELECT InsuredPartyT.PersonID, InsuranceCoT.Name as Company, RespTypeT.TypeName, Placement "
			" FROM AppointmentInsuredPartyT INNER JOIN "
			" InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID "
			" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			" WHERE AppointmentInsuredPartyT.AppointmentID = {INT}; "
			, ResID, ResID, ResID);

		if (prs->eof) {
			// Assume the find first failed because the entry was
			// deleted elsewhere in the network
			MsgBox(MB_ICONEXCLAMATION|MB_OK, RCS(IDS_APPOINTMENT_NOT_FOUND));
			return FALSE;
		} else {
			FieldsPtr flds = prs->Fields;

			// Enable and show the cancel appointment button, since this is an existing appointment
			{
				CWnd *pDelBtn = GetDlgItem(IDC_DELETE_BTN);
				pDelBtn->EnableWindow(TRUE);
				pDelBtn->ShowWindow(SW_SHOW);
				EnableCancelAppointmentMenuOption(TRUE);

				// (z.manning 2008-11-24 10:06) - PLID 31130 - Diable and hide the save and go to insurance
				// button on new appointments.
				GetDlgItem(IDC_GOTO_INSURANCE)->EnableWindow(FALSE);
				GetDlgItem(IDC_GOTO_INSURANCE)->ShowWindow(SW_HIDE);
			}

			m_NewRes = false;
			m_ResID = ResID;
			m_nNewResID = -1;
			//TES 10/27/2010 - PLID 40868 - If we have a calculated TemplateItemID, it's invalid now.
			RefreshTemplateItemID();

			// Set properties
			//TES 2004-01-30: Store this for use on the extra info section, we know it won't change.
			m_strInputName = AdoFldString(flds, "CreatedLogin","");

			// (c.haag 2004-02-26 11:30) - Same idea
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			m_dtLastLM = AdoFldDateTime(flds, "LastLM", dtInvalid);
			m_dtModified = AdoFldDateTime(flds, "ModifiedDate", dtInvalid);
			m_strModifiedLogin = AdoFldString(flds, "ModifiedLogin", "");
			//TES 11/12/2008 - PLID 11465 - Added Input Date
			m_dtInput = AdoFldDateTime(flds, "CreatedDate", dtInvalid);
			
			long nPatientID = AdoFldLong(flds, _T("PatientID"));

			// Log the patient ID
			Log("::MoveToRes (Opening ResEntryDlg) patient ID = %d appt ID = %d", nPatientID, ResID);

			// If the patient is actually there
/*	We don't have anything setup yet
			COleDateTime dt;
			dt = m_nxtStart->GetDateTime();
			m_StartTime = dt.Format("%I:%M %p");
			dt = m_nxtEnd->GetDateTime();
			m_EndTime = dt.Format("%I:%M %p");
*/			UpdateData(TRUE);

			// Set the patient combo in this screen to reflect the selected patient
			//TES 1/14/2010 - PLID 36762 - Make sure we can access this patient.
			if (!GetMainFrame()->CanAccessPatient(nPatientID, false)) {
				return FALSE;
			}
			SetCurPatient(nPatientID);

			// Get the list of resources for this appointment
			{
				//
				// (c.haag 2006-05-08 12:45) - PLID 20475 - This method does not require its own recordset
				//
				CString strResourceIDs = AdoFldString(flds, _T("ResourceIDString"));
				m_arydwSelResourceIDs.RemoveAll();
				while (strResourceIDs.GetLength()) {
					int n = strResourceIDs.Find(" ");
					if (-1 == n) {
						m_arydwSelResourceIDs.Add(atoi(strResourceIDs));
						strResourceIDs.Empty();
					} else {
						CString strID = strResourceIDs.Left(n);
						m_arydwSelResourceIDs.Add(atoi(strID));
						strResourceIDs = strResourceIDs.Right( strResourceIDs.GetLength() - n - 1);
					}
				}

				/*
				// Load the recordset
				_RecordsetPtr rsResource = CreateRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %d", ResID);
				// Clear out the existing list (there really wouldn't be any but just in case)
				ASSERT(m_arydwSelResourceIDs.GetSize() == 0);
				m_arydwSelResourceIDs.RemoveAll();
				// Loop through the recordset
				FieldPtr fldResourceID = rsResource->Fields->Item["ResourceID"];
				while (!rsResource->eof)
				{
					// Add the recordset's resource id to the array for each record
					m_arydwSelResourceIDs.Add(VarLong(fldResourceID->Value));
					// Move to the next record
					rsResource->MoveNext();
				}
				// Close the recordset ((b.cardillo 2003-07-07 13:30) - Now that this is in a code block the recordset is 
				// about to go out of scope which automatically closes it, so this isn't technically necessary anymore, but 
				// I'm leaving it here for completeness)
				rsResource->Close();*/

				// Requery the appointment type combo based on the resource selected
				RequeryAptTypes();
			}
			EnsureResourceComboVisibility();

			// Empty the purpose ID list
			m_adwPurposeID.RemoveAll();

			// Set appointment type combo
			_variant_t varAptTypeID = flds->Item[_T("AptTypeID")]->Value;
			//First, set it on the screen.
			SetCurAptType(VarLong(varAptTypeID, -1));
			//Now, set up the other controls that may be affected by the type.
			if (varAptTypeID.vt == VT_NULL) {
				//DRT 4/12/2004 - PLID 11877 - This just up and started happening out of the blue.  If you first open the program then 
				//	try to edit an appt w/ no types, you'll get an error b/c the datalist isn't initialized at all.

				// (r.farnworth 2015-08-13 11:10) - PLID 64009 - Commented out
				//if (m_dlAptPurpose == NULL) m_dlAptPurpose = BindNxDataListCtrl(this, IDC_APTPURPOSE_COMBO, GetRemoteData(), false);

				// A null type means the purpose is meaningless so empty and disable it
				m_dlAptPurpose->Clear();
				m_dlAptPurpose->WhereClause = "";
				GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);

				// Make sure we show the patient combo
				OnBlockTimeCatSelected(FALSE);
			} else {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
				_RecordsetPtr rsPurpose = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PurposeID, AptPurposeT.Name FROM AppointmentPurposeT INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID WHERE AppointmentID = {INT}", ResID);
					
				//TES 6/16/2004: Clear out the old list of purposes first, we don't want them around.
				m_schedAuditItems.aryOldPurpose.RemoveAll();
				//(e.lally 2005-06-09) PLID 16680 - When comparing the old and new purposes arrays, they need to be initialized to the same value
					//whether that is empty or the old value
				m_schedAuditItems.aryNewPurpose.RemoveAll();
				while (!rsPurpose->eof)
				{
					// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
					// replaced it with TryAddPurposeIDToAppt
					TryAddPurposeIDToAppt(AdoFldLong(rsPurpose, "PurposeID"));
					m_schedAuditItems.aryOldPurpose.Add(AdoFldString(rsPurpose, "Name"));
					m_schedAuditItems.aryNewPurpose.Add(AdoFldString(rsPurpose, "Name"));
					rsPurpose->MoveNext();
				}
				rsPurpose->Close();

				// Requery the appointment purposes
				// (c.haag 2004-03-31 13:47) PLID 11690 - We no longer need to set the filter here
				// as it is now set within RequeryAptPurposes
				/*CString strPurposeFilter;
				strPurposeFilter.Format(
					"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li)", 
					VarLong(varAptTypeID));
				m_dlAptPurpose->WhereClause = _bstr_t(strPurposeFilter);*/
				RequeryAptPurposes();

				// Display our new purpose list to the user. If the appointment has a purpose that does not
				// exist in the purpose dropdown, it will be added to the combo within RefreshPurposeCombo.
				// (c.haag 2006-05-08 16:43) - PLID 20482 - This is redundant because RefreshPurposeCombo
				// is called in OnRequeryFinished!
				//RefreshPurposeCombo();
				GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);
			}
			EnsurePurposeComboVisibility();

			// Set location combo to the location according to the data (notice we don't know the name of the location right yet)
			SetCurLocation(AdoFldLong(flds, "LocationID"), "");

			// (j.jones 2014-12-02 08:55) - PLID 64178 - added initial location ID
			m_nInitLocationID = AdoFldLong(flds, "LocationID");

			// (j.gruber 2013-01-07 15:59) - PLID 54414 - ref phys
			SetCurRefPhys(AdoFldLong(flds, "RefPhysID", -1));

			// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
			// If this is true, we need to revalidate whether the rule is still exceeded,
			// in which case the override text does not need to be shown.
			m_bIsMixRuleOverridden = AdoFldBool(flds, "IsMixRuleOverridden", FALSE) ? true : false;			
			if (m_bIsMixRuleOverridden) {
				m_bIsMixRuleOverridden = IsAppointmentMixRuleOverrideCurrent(ResID);
			}

			// Display our new resource list to the user
			RefreshResourceCombo();

			if (varAptTypeID.vt == VT_I4)
			{
				// (c.haag 2006-05-08 15:37) - PLID 20475 - We now get the colors from the appointment query
				//
				//_RecordsetPtr prsColor = CreateRecordset("SELECT Color FROM AptTypeT WHERE ID = %d", varAptTypeID.lVal);
				//if (!prsColor->eof) {
				//	FieldsPtr fldsColor = prsColor->Fields;
				//	m_ActiveBorderColor = VarLong(fldsColor->Item[_T("Color")]->Value, 0);
				//}
				m_ActiveBorderColor = AdoFldLong(flds, "AptTypeColor", 0);
			}
			m_ActiveStatusColor = m_OldStatusColor = AdoFldLong(flds, "AptStatusColor", DEFAULT_APPT_BACKGROUND_COLOR);

			m_dtArrivalTime = AdoFldDateTime(flds, _T("ArrivalTime"));
			m_dtStartTime = AdoFldDateTime(flds, _T("StartTime"));
			m_dtEndTime = AdoFldDateTime(flds, _T("EndTime"));

			if (!IsEvent())
			{
				m_date.SetValue((COleVariant)AdoFldDateTime(flds, _T("StartTime")));
				
				//TES 12/17/2008 - PLID 32479 - Changed the way we load these values now that they're loaded into
				// datalists, rather than combo boxes.
				// (c.haag 2010-09-08 10:52) - PLID 37734 - Preserve the prior move-up status of the appointment
				m_bMoveUp = m_bOldMoveUp = AdoFldBool(flds, _T("MoveUp"))?true:false;
				if(m_bMoveUp) {
					m_pMoveUpCombo->SetSelByColumn(0, (long)1);
				}
				else {
					m_pMoveUpCombo->SetSelByColumn(0, (long)0);
				}
				
				m_nConfirmed = AdoFldLong(flds, _T("Confirmed"));
				m_pConfirmedCombo->SetSelByColumn(0, m_nConfirmed);
				
				m_bReady = AdoFldBool(flds, _T("Ready"))?true:false;
				if(m_bReady) {
					m_pReadyCombo->SetSelByColumn(0, (long)1);
				}
				else {
					m_pReadyCombo->SetSelByColumn(0, (long)0);
				}
			}
			else {
				m_dateEventStart.SetValue((COleVariant)AdoFldDateTime(flds, _T("StartTime")));
				m_dateEventEnd.SetValue((COleVariant)AdoFldDateTime(flds, _T("EndTime")));
			}

			m_dlNoShow->FindByColumn(0, AdoFldLong(flds, _T("ShowState")), 0, TRUE);
			long nTempCurSelStatus = m_dlNoShow->CurSel;
			if(nTempCurSelStatus == -1){
				m_nAptInitStatus = -1;
			}
			else{
				m_nAptInitStatus = VarLong(m_dlNoShow->Value[nTempCurSelStatus][0]);
			}

			m_Notes = AdoFldString(flds, _T("Notes"));
			if (m_Notes == " ") {
				m_Notes = "";
			}

			if(flds->GetItem("RequestedResourceID")->Value.vt == VT_I4) {
				m_dlRequestedResource->SetSelByColumn(0, AdoFldLong(flds, "RequestedResourceID"));
			}
			else {
				m_dlRequestedResource->CurSel = -1;
			}

			// (c.haag 2009-01-19 12:01) - PLID 32712 - Copy initial purpose list
			m_adwAptInitPurposes.Copy(m_adwPurposeID);

			// (c.haag 2010-03-08 16:53) - PLID 37326 - Set initial values
			m_nAptInitType = GetCurAptTypeID();
			m_adwAptInitResources.Copy(m_arydwSelResourceIDs);
			m_dtAptInitStartTime = m_dtStartTime;
			m_dtAptInitEndTime = m_dtEndTime;
		}

		// (j.gruber 2012-07-30 11:59) - PLID 51830 -  Insurance Information
		prs = prs->NextRecordset(NULL);
		while (!prs->eof) {
			//load insurance information
			long nPersonID = AdoFldLong(prs->Fields, "PersonID");
			long nPlace = AdoFldLong(prs->Fields, "Placement");
			CString strName = AdoFldString(prs->Fields, "Company");
			CString strRespType = AdoFldString(prs->Fields, "TypeName");
			SetInsurance(nPlace,  nPersonID, strName, strRespType);			
			//set the original
			InsuranceInfo *pInsInfo = new InsuranceInfo();	
			pInsInfo->nInsuredPartyID = nPersonID;
			pInsInfo->strInsCoName = strName;
			pInsInfo->strRespType = strRespType;
			m_mapOrigIns.SetAt(nPlace, pInsInfo);	
	
			prs->MoveNext();
		}

		//draw the primary and secondary
		InvalidateRect(m_rcPriIns);
		InvalidateRect(m_rcSecIns);		

		COleDateTime dt;

		{
			m_nxtArrival->SetDateTime(m_dtArrivalTime); // setup arrival time
			m_nxtStart->SetDateTime(m_dtStartTime); //setup start time
			m_nxtEnd->SetDateTime(m_dtEndTime); //setup end time
		}

		
		// Do this as late as possible; the Duration field is dependent on the start and end time being set 
		// properly.
		// Reflect the patient properties

		// (j.jones 2014-11-14 10:30) - PLID 64116 - now takes in a boolean to state whether
		// the user has changed the patient on the appointment, false here because we're
		// simply loading the existing patient on the appointment
		ApplyCurrentPatient(false);


		UpdateData(FALSE);

		{
			CWnd *pNotesWnd = GetDlgItem(IDC_NOTES_BOX);
			long nNoteLen = m_Notes.GetLength();
			pNotesWnd->SendMessage(EM_SETSEL, nNoteLen, nNoteLen);
			pNotesWnd->SendMessage(EM_SCROLLCARET, 0, 0);
		}

		return TRUE;
	} CResEntryDlg_NxCatchAllCall("CResEntryDlg::MoveToRes Error 1", return FALSE);
}

// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
// action is being performed after saving the appointment
BOOL CResEntryDlg::ValidateSaveClose(AppointmentSaveAction asaAction)
{
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	try {
		try {
		m_bCheckActive = false;
		if (!UserPermission(EditAppointment)) {
			m_bCheckActive = true;
			return FALSE;
		}

		// (a.walling 2010-06-14 17:36) - PLID 23560 - Resource Sets
		// (j.jones 2014-12-02 15:55) - PLID 64178 - if a mix rule is overridden, it is passed up to the caller
		std::vector<SchedulerMixRule> overriddenMixRules;
		SelectedFFASlotPtr pSelectedFFASlot;
		pSelectedFFASlot.reset();
		if (ValidateDataWithResourceSets(true, overriddenMixRules, pSelectedFFASlot)) {

			// (j.jones 2014-12-19 14:07) - PLID 64178 - if a new appointment slot was provided, move the appt.
			// to that slot
			if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {

				m_ActiveDate = AsDateNoTime(pSelectedFFASlot->dtStart);
				m_date.SetValue((COleVariant)m_ActiveDate);

				m_nxtStart->SetDateTime(pSelectedFFASlot->dtStart);
				m_dtStartTime = pSelectedFFASlot->dtStart;

				m_nxtEnd->SetDateTime(pSelectedFFASlot->dtEnd);
				m_dtEndTime = pSelectedFFASlot->dtEnd;

				m_nxtArrival->SetDateTime(pSelectedFFASlot->dtArrival);
				m_dtArrivalTime = pSelectedFFASlot->dtArrival;

				// (r.farnworth 2016-02-02 09:27) - PLID 68116 - FFA results transmit location to new appointment.
				m_nCurLocationID = pSelectedFFASlot->nLocationID;
				
				m_arydwSelResourceIDs.RemoveAll();
				m_arydwSelResourceIDs.Append(pSelectedFFASlot->dwaResourceIDs);
				EnsureResourceComboVisibility();
			}

			m_DoSave = true;
			m_bCheckActive = true;
			PreSave();
			// (j.jones 2010-01-04 10:53) - PLID 32935 - if we're going to create an allocation OR an order,
			// we don't need to check to create an allocation during the save
			CleanUp(asaAction != asaCreateInvAllocation && asaAction != asaCreateInvOrder);
			//CNxDialog::OnOK();
			Dismiss();

			// (j.jones 2014-12-02 16:19) - PLID 64178 - if the user overrode a mix rule,
			// track that they did so
			if (overriddenMixRules.size() > 0) {
				long nAppointmentID = m_ResID == -1 ? m_nNewResID : m_ResID;
				ASSERT(nAppointmentID != -1);
				TrackAppointmentMixRuleOverride(nAppointmentID, overriddenMixRules);

				// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
				m_bIsMixRuleOverridden = true;
			}
			
			m_bCheckActive = true;
			return TRUE;
		} else {
			m_bCheckActive = true;
			return FALSE;
		}
		
		} catch (_com_error e) { 
			// (b.cardillo 2005-10-18 11:06) - PLID 17432 - The problem happened, so lock the 
			// log file so we can analyze it later.  Then re-throw the exception.
			// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
			/*try { try { CString strErr; strErr.Format("COM ERROR %li: %s (%s)", e.Error(), e.ErrorMessage(), (LPCTSTR)e.Description()); 
				LogPlid17432("(%lu) PLID 17432: CResEntryDlg::ValidateSaveClose:exception: Line %lu; %s\r\n", GetTickCount(), __LINE__, strErr); } catch(...) { };
				LogPlid17432LockAndClose(); } catch(...) { }*/
			throw;
		}
	
	}CResEntryDlg_NxCatchAll("Error in ValidateSaveClose()");
	
	m_bCheckActive = true;	
	return FALSE;
}

void CResEntryDlg::OnOK() 
{
	try 
	{
		// (c.haag 2015-11-12) - PLID 67577 - Call the business logic
		HandleSaveAndCloseReservation();
	}
	CResEntryDlg_NxCatchAll("Error in OnOK()");
}

void CResEntryDlg::HandleSaveAndCloseReservation()
{
	try
	{
		// If the user is cancelling the act of editing the appointment, we
		// need to consider the fact that the appointment may have been
		// deleted or cancelled by another user since the time this window
		// was opened.
		if (m_Res != NULL && m_Res.GetReservationID() >= 0)
		{
			long nResID = m_Res.GetReservationID();

			//if the appointment was deleted
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			if (!ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM AppointmentsT WHERE ID = {INT}", nResID)) {
				CKeepDialogActive kda(this);
				if (IDNO == MsgBox(MB_YESNO, "The appointment you were editing has been deleted by another user. Would you like to recreate it as a new appointment?"))
				{
					KillThisResObject(TRUE);
					// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
					CleanUp();
					Dismiss();
					return;
				}
				m_NewRes = true;
				m_ResID = -1;
				//TES 10/27/2010 - PLID 40868 - If we have a calculated TemplateItemID, it's invalid now.
				RefreshTemplateItemID();
			}
		}

		// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
		// action is being performed after saving the appointment
		ValidateSaveClose(asaNormal);
	}
	// (c.haag 2015-11-12) - PLID 67577 - Log that we touched on this function before passing it to the caller
	NxCatchAllSilentCallThrow(Log("Exception being thrown from %s", __FUNCTION__))
}

void CResEntryDlg::OnDeleteRes() // This should really be OnCancelRes
{
	try {
		try {
			m_bCheckActive = false;
			if (!CheckCurrentUserPermissions(bioAppointment, SPT________2__)) {
				m_bCheckActive = true;
				return;
			}
			if(!m_bIsCancelled) { // (j.luckoski 2012-05-02 16:22) - PLID 11597 - If cancelled, skip most of this.
				m_bIsCancelled = true;
				if (m_NewRes) {
					CResEntryDlg_ASSERT(FALSE);
					m_bCheckActive = true;
					return;
				}
				bool bSilent = false;
				// First check to see if the appointment is valid
				// (j.jones 2014-12-02 15:55) - PLID 64178 - this override is never used in this function, but is a required parameter
				std::vector<SchedulerMixRule> overriddenMixRules;
				SelectedFFASlotPtr pSelectedFFASlot;
				pSelectedFFASlot.reset();
				if (ValidateDataNoResourceSets(false, overriddenMixRules, pSelectedFFASlot)) {
					// It's valid, so save it before canceling it
					m_DoSave = true;
					PreSave();
					SaveRes(false);
					m_DoSave = false;
					SetModifiedFlag(false); // (b.eyers 2014-07-08) - PLID 64914 - reset modified flag because the appt was just saved
				} else {
					// It's not valid so ask the user what to do
					m_bCheckActive = false;
					CString strQuestion;
					strQuestion = CString(
						"You are trying to cancel an appointment that cannot be saved.  If you proceed, your\n") +
						"latest changes to the appointment will not be recorded in the appointment history.\n\n" +
						"Would you like to cancel the appontment anyway?";
					if (AfxMessageBox(strQuestion, MB_ICONQUESTION | MB_YESNO) == IDYES) {
						bSilent = true;
					} else {
						// The user didn't click yes
						// (b.eyers 2014-07-08) - PLID 64914 - set cancelled back to false becasue the cancelling the appt was cancelled; set revertactive to false so the appt tab doesn't revert the text back
						m_bIsCancelled = false;
						m_bRevertActive = false;
						return;
					}
				}

				// If we made it to here, we need to cancel (delete) the appointment
				BeginWaitCursor();

				// (j.jones 2014-12-02 15:55) - PLID 64178 - this override is never used in this function,
				// so assert to confirm this
				if (overriddenMixRules.size() > 0) {
					//we don't check mix rules when deleting an appointment,
					//so why is this filled?
					ASSERT(FALSE);
				}

				// (j.jones 2015-01-05 11:36) - PLID 64178 - the FFA option is never used in this function,
				// so assert to confirm this
				if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
					//we don't check mix rules when deleting an appointment,
					//so why is this filled?
					ASSERT(FALSE);
				}

				if (AppointmentCancel(m_ResID, bSilent, bSilent)) {
					// Delete the m_Res object from the screen (this means decrement our reference, but 
					// ALSO tell the IReservation object to take itself out of whatever Singleday it's in)
					// (j.luckoski 2012-06-14 14:13) - PLID 11597 - If outside the date range, kill the object and don't show it.
					// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
					if(!m_nCancelledAppt || !ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT * FROM AppointmentsT WHERE Status = 4 AND CancelledDate >= DATEADD(hh, -{INT}, StartTime) AND ID = {INT}", m_nDateRange, m_Res.GetReservationID())) {
						KillThisResObject(TRUE);
					} else {
						KillThisResObject(FALSE);
					}
				} else {
					CKeepDialogActive kda(this);			
					//MsgBox("The appointment could not be cancelled."); ZM - PLID 20283 - This msg box was an unnecessary nuissance because if we get here, it means they cancelled the appt cancel.
					m_bCheckActive = true;
					// (b.eyers 2014-07-08) - PLID 64914 - set cancelled back to false becasue the cancelling the appt was cancelled; set revertactive to false so the appt tab doesn't revert the text back
					m_bIsCancelled = false; 
					m_bRevertActive = false;
					return;
				}
				// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
				//m_Res.SetDispatch(NULL);
				m_bCheckActive = true;
				// (j.luckoski 2012-06-26 12:15) - PLID 11597 - Set it to false so we run the HandleCancelRestore function
				m_bRevertActive = false;
				EndWaitCursor();
				// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
				HandleCancelAndCloseReservation();
				//// ToDo: Figure out why you need to call EndWaitCursor twice...
				EndWaitCursor();
			} else {
				//m_Res.SetDispatch(NULL);
				BeginWaitCursor();
				CKeepDialogActive kda(this);
				// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
				if (AppointmentUncancel(this, m_ResID)) {
					// (j.luckoski 2012-06-26 12:15) - PLID 11597 - Set it to false so we run the HandleCancelRestore function
					m_bRevertActive = false;
					// (j.luckoski 2012-06-14 13:15) - PLID 11597 - Make not cancelled anymore
					m_bIsCancelled = false;
					m_bCheckActive = true;
					EndWaitCursor();
					// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
					HandleCancelAndCloseReservation();
				}
				//// ToDo: Figure out why you need to call EndWaitCursor twice...
				EndWaitCursor();
			}
		} catch (_com_error e) { 
			// (b.cardillo 2005-10-18 11:06) - PLID 17432 - The problem happened, so lock the 
			// log file so we can analyze it later.  Then re-throw the exception.
			// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
			/*try { try { CString strErr; strErr.Format("COM ERROR %li: %s (%s)", e.Error(), e.ErrorMessage(), (LPCTSTR)e.Description()); 
				LogPlid17432("(%lu) PLID 17432: CResEntryDlg::OnDeleteRes:exception: Line %lu; %s\r\n", GetTickCount(), __LINE__, strErr); } catch(...) { };
				LogPlid17432LockAndClose(); } catch(...) { }*/
			throw;
		}

	}CResEntryDlg_NxCatchAll("Error cancelling appointment.");
}

void CResEntryDlg::Dismiss()
{
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	if (!SetWindowPos(0,0,0,0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOMOVE|SWP_HIDEWINDOW)) {
		DWORD dwErr = GetLastError();
		SetLastError(0);
		ShowWindow(SW_HIDE);
		dwErr = GetLastError();
	}
	// (b.cardillo 2008-07-22 12:30) - PLID 17432 - Finally an explanation!  See plids 11410 and 15498 for more 
	// information, but suffice it to say that we'd previously discovered a way that the hiding of the window 
	// could fail but not report failure: there was enough of a delay that Windows XP thought our app had hung, 
	// and therefore replaced our window with a placeholder.  So our solution is to forcibly modify the window 
	// style to take away the WS_VISIBLE flag.  This has the affect of completing the hide of the window if the 
	// SetWindowPos(...SWP_HIDEWINDOW...) above failed, and does nothing if it succeeded.  This fits everything 
	// we know about this problem, and the solution has proven itself in at least two other cases.  Our logging 
	// indicates that the slowness we often encounter (that I believe is resulting in the XP placholder window) 
	// is in the ValidateAppt() called at the end of ValidateData() which itself is called from inside the 
	// ValidateSaveClose() function just prior to calling some cleanup followed by Dismiss(), which brings us 
	// here.  Since we don't do any message pumping in the intervening time, we end up with the potential XP 
	// placeholder which interferes with our hiding of the window.
	ModifyStyle(WS_VISIBLE, 0, 0);

	CWnd *pParent = GetParent();
	if (pParent->GetSafeHwnd()) {
		pParent->SetForegroundWindow();
	}


	//Allow ToDos to pop up again.
	GetMainFrame()->AllowPopup();


// (j.jones 2007-11-21 11:29) - PLID 28147 - converted to enums
	UnregisterHotKey(this->m_hWnd,hkSave);
	UnregisterHotKey(this->m_hWnd,hkSaveGoToPatient);
	UnregisterHotKey(this->m_hWnd,hkSaveCreateInvAlloc);
	UnregisterHotKey(this->m_hWnd,hkSaveCreateCaseHist);
	UnregisterHotKey(this->m_hWnd,hkSaveCreateNewPat);
	UnregisterHotKey(this->m_hWnd,hkChangePatient);
	UnregisterHotKey(this->m_hWnd,hkApptLinking);
	UnregisterHotKey(this->m_hWnd,hkShowMoreInfo);
	UnregisterHotKey(this->m_hWnd,hkSaveCreateInvOrder);	// (j.jones 2008-03-18 13:49) - PLID 29309
	UnregisterHotKey(this->m_hWnd,hkSaveCreateBill);		// (j.jones 2008-06-24 10:06) - PLID 30455
	UnregisterHotKey(this->m_hWnd,hkSaveViewMedicationHistory); // (r.gonet 12/16/2013) - PLID 58416

	// (b.cardillo 2003-06-27 13:08) I just moved this from the end of CResEntryDlg::OnCancel() to the end of CResEntryDlg::Dismiss()
	// because we found this weird drawing issue would happen under a few other odd ways of closing the resentry window (sometimes 
	// even when saving it, if you got a few warnings).  So now with this code being called whenever the dialog disappears, no matter 
	// what the reason, the problem should appear less often.  The nice thing is that this only happens while running the debugger, 
	// we've never seen it when just running the .exe normally, even if it's a debug-build of the .exe.  I added this to our "other 
	// important points" section below.
	// (b.cardillo 2003-06-18 16:18) TODO: Despite hours of research, I cannot figure out a bug (see description below), and the only 
	// work-around I have is to manually refresh the singleday that owns the appointment.
	// THE BUG: Following certain steps you can get the NxTextBoxes to be only partially drawn.  Here are the steps:
	//    1. Open Practice.
	//    2. Go to the scheduler module.
	//    3. Click on the "Week" tab to see the week view.
	//    4. Click on the "Day" tab to go right back to the day view.
	//    5. Click on some empty space in a singleday to start a new appointment.
	//    6. Click "Exit without Saving".
	//  When the resentry box disappears, any pre-existing NxTextBoxes on the screen that had been partially obscured by the resbox 
	// will not redraw the sections that had been obscured.  I put traces in the OnDraw of the text boxes as well as a trace 
	// immediately after the SetWindowPos in the Dismiss() function called above that hides the resentry, and the drawing seems to be 
	// happening in the same order in all cases, whether you have switched to the week view and back (steps 3-4) or not!  So there 
	// has to be some other way the obscured area of the screen is being redrawn.  I tried a playing with the SWP_NOCOPYBITS flags on
	// the SetWindowPos but no luck.  So there you go, I couldn't figure it out but this work-around seems to be effective.  The 
	// Refresh method implemented by the singleday simply invalidates itself and then calls updatewindow on itself.  So all we're 
	// doing is invalidating.  Bizarre.
	//
	// Some other important points: 
	//   1. Try as I might, I cannot reproduce this behavor in a test app.
	//   2. If you use an older Practice.exe (even with current .ocx files registered) the problem DOES NOT HAPPEN, so it must be in 
	//      the .exe.
	//   3. This problem only seems to happen when debugging, never when "executing" from a debugger or running the executable 
	//      directly, even when you're using a debug-build executable.
	//
	CNxSchedulerDlg *pSheet = NULL;
	if (m_pSchedulerView && (pSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet())) {
		if (pSheet->m_pSingleDayCtrl) {
			pSheet->m_pSingleDayCtrl.Refresh();
		}
		if (pSheet->m_pEventCtrl) {
			pSheet->m_pEventCtrl.Refresh();
		}
	}
}

void CResEntryDlg::OnCancel() 
{
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	try 
	{
		// (c.haag 2015-11-12) - PLID 67577 - Call the business logic
		HandleCancelAndCloseReservation();
	}
	CResEntryDlg_NxCatchAll("Error in OnCancel()");
}

void CResEntryDlg::HandleCancelAndCloseReservation()
{
	try
	{
		m_DoSave = false;

		// If the user is cancelling the act of editing the appointment, we
		// need to consider the fact that the appointment may have been
		// deleted or cancelled by another user since the time this window
		// was opened.
		if (m_Res != NULL && m_Res.GetReservationID() >= 0)
		{
			long nResID = m_Res.GetReservationID();

			//if the appointment was deleted
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			if (!ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM AppointmentsT WHERE ID = {INT}", nResID)) {
				CKeepDialogActive kda(this);
				MsgBox("The appointment you were editing has been deleted by another user.");
				KillThisResObject(TRUE); // (j.luckoski 2012-05-02 16:23) - PLID 11597 - Pass true
			}
			//if the appointment was cancelled
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			// (j.luckoski 2012-04-26 09:57) - PLID 11597 - If not cancelled already and it becomes cancelled, someone cancelled it.
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			else if (!m_bIsCancelled && !ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM AppointmentsT WHERE ID = {INT} AND Status <> 4", nResID)) {
				CKeepDialogActive kda(this);
				MsgBox("The appointment you were editing has been cancelled by another user.");
				// (j.luckoski 2012-04-26 09:59) - PLID 11597 - Since we have an update view that determines if it is going to be seen or not
				// hid KillThisResObject so it doesn't kill it entirely.
				// (j.luckoski 2012-06-14 14:14) - PLID 11597 - No more hiding it, now we test to see if its in the date range of a 
				// cancelled appt and if so we show it, if not then we kill it to avoid hangups.
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
				if (!m_nCancelledAppt || !ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT * FROM AppointmentsT WHERE Status = 4 AND CancelledDate >= DATEADD(hh, -{INT}, StartTime) AND ID = {INT}", m_nDateRange, m_Res.GetReservationID())) {
					KillThisResObject(TRUE);
				}
				else {
					KillThisResObject(FALSE);
				}
				m_Res.SetDispatch(NULL);
			}
		}

		// If we didn't kill the res object, we need to see if the user modified
		// the appointment, and if they did, be prompted as to whether they want
		// to abandon their changes.
		if (m_Res != NULL)
		{
			// We've tested every field except the start time and end time. They don't
			// fire events when they've changed, so we need to figure out on our own
			// if they changed.
			if (!GetModifiedFlag())
			{
				COleDateTime dtNxStart = m_nxtStart->GetDateTime();
				COleDateTime dtNxEnd = m_nxtEnd->GetDateTime();
				COleDateTime dtNxArrival = m_nxtArrival->GetDateTime();

				if (m_dtStartTime.GetHour() != dtNxStart.GetHour() ||
					m_dtStartTime.GetMinute() != dtNxStart.GetMinute() ||

					m_dtEndTime.GetHour() != dtNxEnd.GetHour() ||
					m_dtEndTime.GetMinute() != dtNxEnd.GetMinute() ||

					m_dtArrivalTime.GetHour() != dtNxArrival.GetHour() ||
					m_dtArrivalTime.GetMinute() != dtNxArrival.GetMinute())
				{
					SetModifiedFlag();
				}
			}

			// Ok, now we will see if the appointment was modified
			if (GetModifiedFlag() && IsWindowVisible())
			{
				CKeepDialogActive kda(this);
				if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to exit without saving this appointment?"))
					return;
			}
		}

		CleanUp();
		//CNxDialog::OnCancel();
		Dismiss();
	}
	// (d.thompson 2009-07-07) - PLID 34803 - Extra logging to be removed when we figure out PLID 37221
	// (c.haag 2015-11-12) - PLID 67577 - Log that we touched on this function before passing it to the caller.
	// Also Don meant to say to keep that logging until PLID 28378 is resolved, not 37221
	NxCatchAllSilentCallThrow(
		Log("Exception being thrown from %s", __FUNCTION__);
		Log("PLID 34803:  %s", CClient::PLID34803_GetPrintableCString());
	)
}

// (j.luckoski 2012-06-20 11:19) - PLID 11597 - This is the old function that now calls the new function if there are any leftover
// stragler functions.
CString CResEntryDlg::GetActiveText()
{
	return GetActiveText(false);
}

// (j.luckoski 2012-06-20 11:19) - PLID 11597 - Now includes bIsCancelled to determine if the text will include 
// CANCELLED: at the front or if it will not.
CString CResEntryDlg::GetActiveText(bool bIsCancelled)
{
	CString strAns;

	// Get the format for the res box
	LPCTSTR strBoxFmt = NULL;
	// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
	// (j.luckoski 2012-06-20 11:20) - PLID 11597 - GetResBoxFormat is split to send either the cancelled color for the text, or the normal.
	if(!bIsCancelled) {
		GetResBoxFormat(&strBoxFmt, NULL, m_pSchedulerView->m_bShowAppointmentLocations, m_pSchedulerView->m_bShowApptShowState == 0 ? false : true, m_pSchedulerView->m_bShowPatientInformation, m_pSchedulerView->m_bShowAppointmentResources,  GetRemotePropertyInt("ColorApptTextWithStatus", GetPropertyInt("ColorApptTextWithStatus", 0, 0, false), 0, GetCurrentUserName(), true) ? ObtainActiveStatusColor() : RGB(0,0,0), m_pSchedulerView->m_nExtraInfoStyles);
	} else {
		GetResBoxFormat(&strBoxFmt, NULL, m_pSchedulerView->m_bShowAppointmentLocations, m_pSchedulerView->m_bShowApptShowState == 0 ? false : true, m_pSchedulerView->m_bShowPatientInformation, m_pSchedulerView->m_bShowAppointmentResources, GetRemotePropertyInt("ColorApptTextWithStatus", GetPropertyInt("ColorApptTextWithStatus", 0, 0, false), 0, GetCurrentUserName(), true) ? m_nCancelColor : RGB(0,0,0), m_pSchedulerView->m_nExtraInfoStyles);
	}
		
	// Make sure we got something
	ASSERT(strBoxFmt);
	if (strBoxFmt) {
		CString strFmt = strBoxFmt;
		strAns.Empty();

		int len = strFmt.GetLength();
		int i, di;
		// (j.luckoski 2012-06-26 12:16) - PLID 11597 - Format the color for uncancelled apts
		i = strFmt.Find('[');
		strAns += strFmt.Left(i);
		strFmt = strFmt.Right(strFmt.GetLength() - i);
		while (strFmt.GetLength()) {
			i = strFmt.Find('[');
			if (i > -1) {
				CString strOutText;
				// (j.luckoski 2012-06-20 11:22) - PLID 11597 - If cancelled we want to see CANCELLED:
				// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
				di = AddTextItem(strFmt, strOutText, i, m_bIsCancelled, m_bIsMixRuleOverridden);
				if (strOutText.GetLength()) {
					strAns += strFmt.Left(i) + strOutText;
				}
				strFmt = strFmt.Mid(i + di);
			} else {
				strAns += strFmt;
				strFmt = "";
			}
		}
	}
	
	return strAns;
}

// (j.jones 2010-01-04 10:53) - PLID 32935 - added parameter to disable checking for required allocations
void CResEntryDlg::CleanUp(bool bCheckForAllocations /*= true*/)
{
	m_nWarnedForPatientID = -1;
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
	// Undo our special time column highlight
	HighlightSheetTimeColumn(FALSE);

	// Re-enable the reservation
	if (m_Res != NULL)
		m_Res.PutMouseEnabled(TRUE);

	// If the user changed any of the checkboxes, update them and
	// determine how the code below will know to refresh the scheduler
	if (m_CheckboxesUpdated)
	{			
		for (int i=0; i < 8; i++)
		{
			SetRemotePropertyInt("ResEntryMoreInf", m_pSchedulerView->m_bExtraInfo[i], i);
			SetRemotePropertyInt("ResEntryMoreInfoStyle",m_pSchedulerView->m_nExtraInfoStyles[i],i);
			int nField = m_pSchedulerView->m_nExtraFields[i];
			switch(nField) {
			case efCustom1: 
				nField = -1; 
				break;
			case efCustom2: 
				nField = -2; 
				break;
			case efCustom3: 
				nField = -3; 
				break;
			case efCustom4: 
				nField = -4; 
				break;
			}
			SetRemotePropertyInt("ResEntryExtraField", nField, i);
		}
		// If we are not saving the appointment, we do not reset the flag, but we
		// do hide the window so the scheduler will refresh
		if (!m_DoSave)
		{
			Dismiss();
		}
		else
		{				
			// (b.cardillo 2003-10-28 17:02) - Since under this branch of the logic we know we're going to be 
			// saving down below, we used to set m_CheckboxesUpdated to false here so that we wouldn't end up 
			// refreshing the view twice (once in SaveRes and once further down in this function where it checks 
			// if m_CheckboxesUpdated is not false).  The problem is, the SaveRes has been optimized to ONLY 
			// refresh the one appointment that has changed, not the whole view, so if we're changing the "more 
			// info" status that one appointment looks right but the rest of the view still reflects the wrong 
			// "more info" until the user manually refreshes.  So now, even though it's slightly less efficient, 
			// we're going to NOT revert this to false, which means this appointment will be refreshed twice.  
			// If doing this causes performance issues (which I don't believe it will) then maybe we should 
			// consider having the SaveRes function take another parameter of whether the resbox should be 
			// refreshed or not.  Then in this case we could set that flag and then the appointment will only 
			// be refreshed once (since m_CheckboxesUpdated is true).
		}
	}
	if (m_DoSave) {
		// TODO: The dialog has always been dismissed right here, BEFORE the SaveRes.  I couldn't figure 
		// out why, and I still don't think there's a philosophically viable reason, but the fact is that 
		// when you put the Dismiss AFTER the SaveRes, you get errors sometimes (e.g. if the schedule is 
		// filtered on a certain type and you save an appointment of a different type, you get an ugly 
		// error).  It all has something to do with the timing of when the IReservation object goes out 
		// of scope...
		Dismiss();
		// SaveRes will delete the pointer to m_Res
		// (j.jones 2010-01-04 10:55) - PLID 32935 - pass in the allocation check boolean
		SaveRes(true, bCheckForAllocations);
//		CNxSchedulerDlg *pSheet = NULL;
//		if (m_pSchedulerView && (pSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet())) {
//			if (pSheet->m_pSingleDayCtrl) {
//				pSheet->m_pSingleDayCtrl.Resolve();
//				pSheet->m_pSingleDayCtrl.Refresh();
//			}
//		}
		m_DoSave = false;
	} else if (m_NewRes) {
		// Delete the m_Res object from the screen (this means decrement our reference, but 
		// ALSO tell the IReservation object to take itself out of whatever Singleday it's in)
		// (c.haag 2006-12-05 09:44) - PLID 23666 - If m_TemplateItemID is not -1, that means this is
		// a scheduler templating block. Rather than destroy this reservation object, just restore the
		// original template item layout. After that, we call m_Res = NULL because the reservation
		// is dead to us
		//TES 10/27/2010 - PLID 40868 - Check the Initial value, not the Calculated value, we're only concerned with the block that
		// was originally clicked on.
		if (-1 != m_nInitialTemplateItemID) {
			RevertResAppearanceToTemplateItem();
			m_Res.SetDispatch(NULL);
		} else {
			KillThisResObject(TRUE); // (j.luckoski 2012-05-02 16:23) - PLID 11597 - Pass true
		}
	}

	// (j.luckoski 2012-06-26 12:22) - PLID 11597 - If revert active then do it, else handle the cancel restore.
	if (m_Res && m_bRevertActive) {
		RevertActive();
		// Delete the res box if that was either
		// created by ZoomRes or was not created
		m_Res.SetDispatch(NULL);
	} else if(m_Res) {
		HandleCancelRestore();
		m_Res.SetDispatch(NULL);
	}

	// If the checkboxes were updated while the window was visible, refresh
	// the scheduler
	if (m_CheckboxesUpdated)
	{
		m_CheckboxesUpdated = false;
		CNxSchedulerDlg *p = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();
		if (p) p->m_bNeedUpdate = true;
		//TES 3/24/2004: PLID 11593 - Let's just post the message, there may be messages pending that would be messed up 
		//if we clear the singleday.
		// (c.haag 2004-06-23 10:33) PLID 13130 - I commented it out because it caused Practice to crash
		// because, later on, we post a NXM_RESERVATION_DELETE delete message. NXM_RESERVATION_DELETE
		// would appear/ in the message queue after the update view, and that causes problems.
		//m_pSchedulerView->PostMessage(NXM_UPDATEVIEW);
		// (c.haag 2004-07-21 11:26) PLID 13130 - As Tom so dilligently pointed out, this caused
		// a problem where the more info attributes would not update on other appointments. Though
		// updating the view is not viable here, it is safe to use UpdateReservations instead.
		//
		// (z.manning, 05/10/2007) - PLID 11593 - Added a function to update only the text of the reservation
		// to so we don't have to refresh the single day which was causing problems.
		if(p != NULL) {
			p->UpdateReservationText();
		}
	}

	// Clean our duration groups
	while (m_aDurationGroups.GetSize())
	{
		CDurationGroup* p = m_aDurationGroups.GetAt(0);
		delete p;
		m_aDurationGroups.RemoveAt(0);
	}

	// (j.gruber 2012-07-30 14:51) - PLID 51830 -clean up insurance
	ClearInsurancePlacements(TRUE);	

	// Clean up any inactive patients that would appear in the active patient
	// dropdown
	if(GetRemotePropertyInt("SchedShowActivePtOnly", 0, 0, "<None>", true))
	{
		for (long i=0; i < m_adwInactivePtInActiveList.GetSize(); i++)
		{
			// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
			// (a.walling 2010-12-30 11:08) - PLID 40081 - Use datalist2 to avoid row index race conditions
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAptPatient->FindByColumn(plID, (long)m_adwInactivePtInActiveList[i], NULL, VARIANT_FALSE);
			if (pRow)
			{
				// If we have it selected, we have to unselect it.
				if (pRow->IsSameRow(m_dlAptPatient->CurSel))
				{
					m_dlAptPatient->CurSel = NULL;
					m_nCurPatientID = -25;
					// (a.walling 2008-12-04 16:19) - PLID 32345 - We reset the m_nCurPatientID, so we also need to reset this boolean.
					m_bIsCurPatientSet = FALSE;
				}
				m_dlAptPatient->RemoveRow(pRow);
			}
		}
	}
	m_adwInactivePtInActiveList.RemoveAll();

	// Reset our appointment type
	SetCurAptType(-1);

	Activate(FALSE);
}

void CResEntryDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	try {
		CNxDialog::OnActivate(nState, pWndOther, bMinimized);

		if (nState == WA_INACTIVE && IsWindowVisible() && m_bCheckActive && m_nKeepActiveCount <= 0) {
			// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
			HandleCancelAndCloseReservation();
		} else if (nState == WA_CLICKACTIVE) {
			// Not sure why this is in there 
			m_bCheckActive = true;
		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

//void CResEntryDlg::OnDropDownPatientCombo() 
//{
//	try {
//		m_bCheckActive = false;
//	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
//}

void CResEntryDlg::DoRequeryPatientList()
{
	// Requery it because that's what this function does
	//TES 1/18/2010 - PLID 36895 - Make sure the FROM clause is up to date.
	m_dlAptPatient->FromClause = _bstr_t(GetPatientFromClause());
	SetPtComboWhereClause();

	// (j.jones 2010-05-04 12:21) - PLID 32325 - added OHIP as an optional column
	CString strOHIPHealthTitle = "Health Card Num.";
	if(UseOHIP()) {
		//if enabled, get the actual name of the custom field
		long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Name FROM CustomFieldsT WHERE ID = {INT}", nHealthNumberCustomField);
		if(!rs->eof) {
			CString str = AdoFldString(rs, "Name", "");
			str.TrimLeft();
			str.TrimRight();
			if(!str.IsEmpty()) {
				strOHIPHealthTitle = str;
			}
		}
		rs->Close();
	}
	// (j.jones 2010-11-04 16:45) - PLID 39620 - supported Alberta
	else if(UseAlbertaHLINK()) {
		//if enabled, get the actual name of the custom field
		long nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Name FROM CustomFieldsT WHERE ID = {INT}", nHealthNumberCustomField);
		if(!rs->eof) {
			CString str = AdoFldString(rs, "Name", "");
			str.TrimLeft();
			str.TrimRight();
			if(!str.IsEmpty()) {
				strOHIPHealthTitle = str;
			}
		}
		rs->Close();
	}

	//show or hide the SSN/Health Card columns according to the OHIP preference
	// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
	// (a.walling 2010-12-30 11:11) - PLID 40081 - Use datalist2 to avoid row index race conditions
	NXDATALIST2Lib::IColumnSettingsPtr pSSNCol = m_dlAptPatient->GetColumn(plSSN);
	NXDATALIST2Lib::IColumnSettingsPtr pHealthCardCol = m_dlAptPatient->GetColumn(plHealthCard);
	pHealthCardCol->PutColumnTitle(_bstr_t(strOHIPHealthTitle));

	if(UseOHIP() || UseAlbertaHLINK()) {
		pHealthCardCol->PutColumnStyle(csVisible);
		pHealthCardCol->PutStoredWidth(85);
		pSSNCol->PutColumnStyle(csVisible|csFixedWidth);
		pSSNCol->PutStoredWidth(0);
	}
	else {
		pSSNCol->PutColumnStyle(csVisible);
		pSSNCol->PutStoredWidth(85);
		pHealthCardCol->PutColumnStyle(csVisible|csFixedWidth);
		pHealthCardCol->PutStoredWidth(0);
	}

	// (j.gruber 2010-10-04 14:34) - PLID 40415
	// (a.walling 2010-12-30 11:11) - PLID 40081 - Use datalist2 to avoid row index race conditions
	NXDATALIST2Lib::IColumnSettingsPtr pSecurityGroupCol = m_dlAptPatient->GetColumn(plSecurityGroup);
	//if they have the preference set for the patient toolbar, show it here too
	if (GetRemotePropertyInt("PtTB_Security_Group", -9, 0, GetCurrentUserName(), true) > 0) {
		pSecurityGroupCol->PutColumnStyle(csVisible);
		pSecurityGroupCol->PutStoredWidth(70);		
	}
	else {
		pSecurityGroupCol->PutColumnStyle(csVisible|csFixedWidth);
		pSecurityGroupCol->PutStoredWidth(0);
	}

	m_dlAptPatient->Requery();

	// Add a row to allow the user to select "no patient"
	// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
	// (a.walling 2010-12-30 11:11) - PLID 40081 - Use datalist2 to avoid row index race conditions
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAptPatient->GetNewRow();
	pRow->Value[plID] = (long)-25;
	pRow->Value[plLast] = _bstr_t(SCHEDULER_TEXT_PATIENT__NO_PATIENT);
	pRow->Value[plFirst] = _bstr_t("");
	pRow->Value[plMiddle] = _bstr_t("");
	// (a.walling 2010-05-27 15:35) - PLID 38917 - No more Name column
	//pRow->Value[4] = _bstr_t(SCHEDULER_TEXT_PATIENT__NO_PATIENT);
	// (a.walling 2008-05-08 10:49) - PLID 29786 - For consistency, fill the rest of the columns with NULL rather than EMPTY
	pRow->Value[plUserDefinedID] = g_cvarNull; 
	pRow->Value[plBirthDate] = g_cvarNull;
	pRow->Value[plSSN] = g_cvarNull;
	pRow->Value[plHealthCard] = g_cvarNull;	// (j.jones 2010-05-04 12:03) - PLID 32325 - added OHIP Health Card Num.
	pRow->Value[plSecurityGroup] = g_cvarNull; // (j.gruber 2010-10-04 14:35) - PLID 40415 - security group
	pRow->Value[plForeColor] = (long)0; // (j.gruber 2011-07-22 17:16) - PLID 44118 - show as active
	m_dlAptPatient->AddRowBefore(pRow, m_dlAptPatient->GetFirstRow()); // (a.walling 2010-12-30 11:12) - PLID 40081 - Use datalist2 to avoid row index race conditions
}

// Forces a refresh of the patient list.  If there is currently something selected, it stays selected if possible
void CResEntryDlg::RequeryPersons()
{
	// If the window is visible, we better have a "current patient"
	ASSERT(m_bIsCurPatientSet || !IsWindowVisible());

	// See if we have a current patient
	if (m_bIsCurPatientSet) {
		// For use in setting the appropriate selection after the requery
		long nCurPersonID = GetCurPatientID();

		// Actually do the requery
		DoRequeryPatientList();

		// Set the selection appropriately
		SetCurPatient(nCurPersonID);
	} else {
		// No need to keep tabs on the currently selected patient because there isn't 
		// one, just do the requery and don't bother resetting the selection
		DoRequeryPatientList();
	}
}

// Forces a refresh of the show state list.
void CResEntryDlg::RequeryShowStates()
{
	// For use in setting the appropriate selection after the requery
	_variant_t varShowStateID;
	varShowStateID.vt = VT_NULL;

	// (r.farnworth 2015-08-13 11:31) - PLID 64009 - Commented Out
	/*if (m_dlNoShow == NULL)
		m_dlNoShow = BindNxDataListCtrl(this, IDC_LIST_STATUS, GetRemoteData(), false);
	else {
		long nCurSel = m_dlNoShow->CurSel;
		if (nCurSel != -1) {
			varShowStateID = m_dlNoShow->Value[nCurSel][0];
		}
	}*/

	// (r.farnworth 2015-08-13 11:31) - PLID 64009 - Freed from its if-statement prison
	long nCurSel = m_dlNoShow->CurSel;
	if (nCurSel != -1) {
		varShowStateID = m_dlNoShow->Value[nCurSel][0];
	}

	m_dlNoShow->Requery();

	// The currently selected show state could have been deleted. Lets wait and see whether
	// we should reset this to pending or not.
	m_dlNoShow->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	// Find the show state we had selected earlier
	if (-1 == m_dlNoShow->FindByColumn(0, varShowStateID, 0, TRUE))
		m_dlNoShow->CurSel = 0; // If it's gone, default to pending
}

// Forces a refresh of the appointment type list.  If there is currently something selected, it stays selected if possible
void CResEntryDlg::RequeryAptTypes()
{
	// For use in setting the appropriate selection after the requery
	long nCurAptTypeID = -1;
	
	// Attach to the datalist if necessary

	// (r.farnworth 2015-08-13 10:44) - PLID 64009 - Commented out
	/*if (m_dlAptType == NULL) {
		m_dlAptType = BindNxDataListCtrl(this, IDC_APTTYPE_COMBO, GetRemoteData(), false);
	} else {
		nCurAptTypeID = GetCurAptTypeID();
	}*/

	// (r.farnworth 2015-08-13 11:31) - PLID 64009 - Freed from its if-statement prison
	nCurAptTypeID = GetCurAptTypeID();

	//DRT 5/17/2004 - PLID 12406 - This needs to be preferenced, otherwise it will hide types
	//	and the user may not have a clue why.
	if(GetRemotePropertyInt("HideEmptyTypes", 0, 0, "<None>", true)) {
		// (c.haag 2004-03-31 12:41) PLID 11690 - Assign the where clause for the type dropdown
		// to reflect only the available types to pull.
		CString strWhere;
		CString strResources;
		if (m_arydwSelResourceIDs.GetSize() > 0)
		{
			for (long i=0; i < m_arydwSelResourceIDs.GetSize(); i++)
			{
				CString str;
				str.Format(",%d ", m_arydwSelResourceIDs[i]);
				strResources += str;
			}
			strResources = strResources.Right(strResources.GetLength() - 1);
			strWhere.Format("Inactive = 0 AND ID IN (select apttypeid from (select resourceid, apttypeid from resourcepurposetypet group by resourceid, apttypeid) SubQ where SubQ.ResourceID in (%s) group by apttypeid having count(apttypeid) = %d)",
				strResources, m_arydwSelResourceIDs.GetSize());
			m_dlAptType->WhereClause = _bstr_t(strWhere);
		}
		else // (c.haag 2004-06-08 14:59) - PLID 12896 - We shouldn't show inactive types
			m_dlAptType->WhereClause = "Inactive = 0";
	}
	else{
		m_dlAptType->WhereClause = "Inactive = 0";
	}

	// Requery the datalist because that's what this function does
	m_dlAptType->Requery();

	// Set the selection appropriately
	SetCurAptType(nCurAptTypeID);
}

// Forces a refresh of the purposes list.  If there is currently something selected, it stays selected if possible
void CResEntryDlg::RequeryAptPurposes(_variant_t *pVarTrySetSel /*= NULL*/)
{
/* DRT 8/4/2004 - PLID 13773 - This code appears to be getting the current selection so 
	we can set it once the requery is finished.  However, as of this point in time, 
	this value is never used after this short block of code.  In the interest of pulling
	out every speed enhancement we can get, I'm removing this code until such a point
	as we find it necessary.  The datalist binding code is moved down below.

	// For use in setting the appropriate selection after the requery
	_variant_t varAptPurposeID; // default to no selection
	varAptPurposeID.vt = VT_NULL;

	// Attach to the datalist if necessary
	if (m_dlAptPurpose == NULL) {
		m_dlAptPurpose = BindNxDataListCtrl(this, IDC_APTPURPOSE_COMBO, GetRemoteData(), false);
	} else {
		if (pVarTrySetSel) {
			varAptPurposeID = *pVarTrySetSel;
		} else {
			long nCurSel = m_dlAptPurpose->CurSel;
			if (nCurSel != -1) {
				varAptPurposeID = m_dlAptPurpose->Value[nCurSel][0];
			}
		}
	}
*/
	// Attach to the datalist if necessary
	// (r.farnworth 2015-08-13 11:10) - PLID 64009 - Commented out
	//if (m_dlAptPurpose == NULL)
		//m_dlAptPurpose = BindNxDataListCtrl(this, IDC_APTPURPOSE_COMBO, GetRemoteData(), false);

	// (c.haag 2004-03-31 12:41) PLID 11690 - Assign the where clause for the purpose dropdown
	// to reflect only the available types to pull.
	CString strWhere;
	CString strResources;
	if (m_arydwSelResourceIDs.GetSize() > 0)
	{
		//DRT 8/4/2004 - PLID 13773 - Fix absurd slowness in requerying the purpose list - 
		//	I changed the query to have an "and" part per resource.  This could possibly be 
		//	a little slower if you had about 15 resources selected (I haven't tested that high),
		//	but for 1-4 resources, it's about 90% faster.

		//1)  Filter on the type selected, these are the purposes checked for this type in the 
		//	administrator module, scheduler tab.
		strWhere.Format("AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) ", GetCurAptTypeID());

		//2)  Loop through all selected resource and generate a query for all purposes allowed
		//	by this Resource / Type combination.  This is the Allowed Purposes on the
		//	resource editor (NexSpa only, but applies to all)
		for (long i=0; i < m_arydwSelResourceIDs.GetSize(); i++)
		{
			CString str;
			str.Format(" AND AptPurposeT.ID IN "
				"(SELECT AptPurposeID FROM ResourcePurposeTypeT WHERE AptTypeID = %li and ResourceID = %li GROUP BY ResourceID, AptPurposeID) ", 
				GetCurAptTypeID(), m_arydwSelResourceIDs[i]);

			strWhere += str;
		}
	}
	else
	{
		// (c.haag 2004-04-01 16:12) - This should never happen but we should have safeguards.
		strWhere.Format(
			"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li)", 
			GetCurAptTypeID());
	}

	// (c.haag 2008-12-09 12:26) - PLID 32376 - We now exclude inactive procedures
	strWhere += " AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ";

	m_dlAptPurpose->WhereClause = _bstr_t(strWhere);

	// Requery the datalist because that's what this function does
	m_dlAptPurpose->Requery();

/* DRT 8/4/2004 - PLID 13773 - There is no need to do a wait and an immediate refresh here - 
	we can let the OnRequeryFinished handle adding these fields and calling the Refresh...() 
	function.

	_variant_t varNull;
	varNull.vt = VT_NULL;
	IRowSettingsPtr pRow;

	// Add a row to allow the user to select "multiple purposes"
	//if (IsSurgeryCenter())
	//{
	m_dlAptPurpose->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	if (m_dlAptPurpose->GetRowCount() > 1)
	{
		pRow = m_dlAptPurpose->Row[-1];
		pRow->Value[0] = varNull;
		pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_APPOINTMENT_PURPOSE__MULTIPLE_PURPOSES);
		m_dlAptPurpose->InsertRow(pRow, 0);	
	}

	// Add a row to allow the user to select "no purpose"
	pRow = m_dlAptPurpose->Row[-1];
	pRow->Value[0] = (long)-1; // (c.haag 2004-04-02 09:17) - This used to be varNull but
								// we need to give it a unique sentinel value for when
								// we requery the appointment type combo.
	pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_APPOINTMENT_PURPOSE__NO_PURPOSE);
	m_dlAptPurpose->InsertRow(pRow, 0);

	// Display our new purpose list to the user
	RefreshPurposeCombo();
*/
}

void CResEntryDlg::OnRequeryFinishedAptpurposeCombo(short nFlags) 
{
	try {
		//DRT 8/4/2004 - PLID 13773 - Moved this code from the RequeryAptPurposes function - 
		//	it had no business being there.
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow;

		// Add a row to allow the user to select "multiple purposes"
		if (m_dlAptPurpose->GetRowCount() > 1)
		{
			pRow = m_dlAptPurpose->Row[-1];
			pRow->Value[apcID] = varNull;
			pRow->Value[apcName] = _bstr_t(SCHEDULER_TEXT_APPOINTMENT_PURPOSE__MULTIPLE_PURPOSES);
			// (a.walling 2008-05-08 10:49) - PLID 29786 - For consistency, ensure NULL is this column's value rather than EMPTY
			pRow->Value[apcArrivalPrepMinutes] = varNull; 
			m_dlAptPurpose->InsertRow(pRow, 0);	
		}

		// Add a row to allow the user to select "no purpose"
		pRow = m_dlAptPurpose->Row[-1];
		pRow->Value[apcID] = (long)-1; // (c.haag 2004-04-02 09:17) - This used to be varNull but
									// we need to give it a unique sentinel value for when
									// we requery the appointment type combo.
		pRow->Value[apcName] = _bstr_t(SCHEDULER_TEXT_APPOINTMENT_PURPOSE__NO_PURPOSE);
		// (a.walling 2008-05-08 10:49) - PLID 29786 - For consistency, ensure NULL is this column's value rather than EMPTY
		pRow->Value[apcArrivalPrepMinutes] = varNull; 
		m_dlAptPurpose->InsertRow(pRow, 0);

		// Display our new purpose list to the user
		RefreshPurposeCombo();
	} CResEntryDlg_NxCatchAll("Error in OnRequeryFinishedAptPurposeCombo");
}

void CResEntryDlg::OnRequeryFinishedAptTypeCombo(short nFlags)
{
	try {
		// (c.haag 2006-05-08 16:10) - PLID 20482 - Add the { No Type } row now that
		// everything else has been added
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow = m_dlAptType->Row[-1];
		pRow->Value[0] = varNull;
		pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_APPOINTMENT_TYPE__NO_TYPE);
		pRow->Value[2] = varNull;
		pRow->Value[3] = varNull;
		pRow->Value[4] = varNull;
		pRow->Value[5] = varNull;
		long nNoSelectionRow = m_dlAptType->InsertRow(pRow, 0);

		//DRT 5/17/2004 - PLID 12406 - This needs to be preferenced, otherwise it will hide types
		//	and the user may not have a clue why.  Same as above.
		// (a.walling 2010-06-11 09:48) - PLID 39005 - I don't understand the point of the code below; it just ends up losing
		// data. We shouldn't touch it if it is already saved. Types which are 'empty' (have no purposes) should be handled
		// the same as Inactive types. The only time we should ever do the code below is when dealing with a new appointment.
		if(m_NewRes && GetRemotePropertyInt("HideEmptyTypes", 0, 0, "<None>", true)) {
			// See if our type exists in the dropdown. If it does not, blank
			// out the selection.
			// (z.manning, 04/07/2008) - PLID 27835 - We should only be using m_nAptTypeID if its value is valid.
			if(m_bValidAptTypeStored && m_nAptTypeID != -1 && m_dlAptType->FindByColumn(0, m_nAptTypeID, 0, TRUE) == -1)
			{
				// Reset the type and the purpose
				m_nAptTypeID = -1;
				if (m_dlAptPurpose != NULL)
				{
					if (-1 == m_dlAptPurpose->FindByColumn(0, (long)-1, 0, TRUE))
					{
						IRowSettingsPtr pRow = m_dlAptPurpose->Row[-1];
						pRow->Value[0] = (long)-1; // (c.haag 2004-04-02 09:17) - This used to be varNull but
													// we need to give it a unique sentinel value for when
													// we requery the appointment type combo.
						pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_APPOINTMENT_PURPOSE__NO_PURPOSE);
						// (a.walling 2008-05-08 10:49) - PLID 29786 - For consistency, ensure NULL is this column's value rather than EMPTY
						pRow->Value[apcArrivalPrepMinutes] = varNull; 
						m_dlAptPurpose->CurSel = m_dlAptPurpose->InsertRow(pRow, 0);
					}
					// Preserve the modified flag because OnSelChosenAptPurposeCombo changes it
					bool bModified = GetModifiedFlag();
					// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
					ApplyToResBox();
					HandleAptPurposeComboSelection(m_dlAptPurpose->CurSel);
					SetModifiedFlag(bModified);
				}
			}
		}

		// (c.haag 2006-05-08 16:06) - PLID 20482 - We no longer wait for the appointment type
		// combo to requery. However, we need to execute all of the logic that used to require
		// the requery having been completed.
		if (m_bValidAptTypeStored) {
			SetCurAptType_PostRequery(m_nAptTypeID);
		} else if (m_dlAptType->CurSel == -1) {
			m_dlAptType->CurSel = nNoSelectionRow;
		}
	} CResEntryDlg_NxCatchAll("Error in OnRequeryFinishedAptTypeCombo");
}

void CResEntryDlg::RequeryLocations(BOOL bKeepCurLocationSelected)
{
	// First in all cases we want to make sure our variable is attached to the datalist
	if (m_dlAptLocation == NULL) {
		// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
		m_dlAptLocation = BindNxDataList2Ctrl(this, IDC_APTLOCATION_COMBO, GetRemoteData(), false);
	}

	// Now use different logic depending on the parameter
	if (bKeepCurLocationSelected) {
		// For use in setting the appropriate selection after the requery
		long nCurLocID = GetCurLocationID();
		CString strCurLocName = GetCurLocationName();

		// Requery the datalist because that's what this function does
		m_dlAptLocation->Requery();
		
		// Set the selection appropriately back to what it was before the requery
		SetCurLocation(nCurLocID, strCurLocName);
	} else {
		// Requery the datalist because that's what this function does
		m_dlAptLocation->Requery();
		
		// Set to have no selection
		ReflectCurLocationFailure();
	}
}

// This is like a pdl->Requery() except it also adds the "multiple resources" entry
void CResEntryDlg::DoRequeryResourcesDatalist()
{
	try {
		// Requery the datalist because that's what this function does
		m_dlAptResource->Requery();
		m_dlRequestedResource->Requery();

		// (c.haag 2005-06-29 12:33) - PLID 6400 - We need to retain the ID's and names
		// of inactive resources so that we can draw the resource hyperlink even with
		// names that don't appear in m_dlAptResource
		_RecordsetPtr prsInactive = CreateRecordset("SELECT ID, Item FROM ResourceT WHERE Inactive = 1");
		FieldsPtr f = prsInactive->Fields;
		m_adwInactiveResIDs.RemoveAll();
		m_astrInactiveResNames.RemoveAll();
		while (!prsInactive->eof) {
			m_adwInactiveResIDs.Add(AdoFldLong(f, "ID"));
			m_astrInactiveResNames.Add(AdoFldString(f, "Item"));
			prsInactive->MoveNext();
		}

		// Add a row to allow the user to select "multiple resources"
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow = m_dlAptResource->Row[-1];
		pRow->Value[0] = varNull;
		pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_RESOURCE__MULTIPLE_RESOURCES);
		m_dlAptResource->InsertRow(pRow, 0);

		//Add a row to allow the user to select no requested resource.
		pRow = m_dlRequestedResource->GetRow(-1);
		pRow->Value[0] = varNull;
		pRow->Value[1] = " { No Resource } ";
		m_dlRequestedResource->InsertRow(pRow, 0);

		m_dlAptResource->PutCurSel(sriNoRow);
		m_dlRequestedResource->PutCurSel(sriNoRow);
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// Note: we used to try to keep the selection intact, but WE DO NOT DO SO ANYMORE.  The idea used 
// to be that we could call this function from anywhere at any time (I guess someone thought 
// we should be able to refresh the list in-place in response to table-checker messages or 
// something...that person was sadly mistaken.  Table-checker messages are only to tell the 
// system that the data has changed and WHEN IT'S CONVENIENT, AND NOT CONFUSING FOR THE USER, we 
// can go ahead and refresh that data.) but now it's defined in a more specific way; this 
// function now simply loads the list, clearing the selection if there is one.  It should really 
// only ever be called if desired right before the dialog is made visible.
void CResEntryDlg::RequeryResources()
{
	// Attach to the datalist if necessary
	// (r.farnworth 2015-08-13 11:26) - PLID 64009 - Commented out
	/*if (m_dlAptResource == NULL) {
		m_dlAptResource = BindNxDataListCtrl(this, IDC_APTRESOURCE_COMBO, GetRemoteData(), false);
	}
	if (m_dlRequestedResource == NULL) {
		m_dlRequestedResource = BindNxDataListCtrl(this, IDC_REQUESTED_RESOURCE, GetRemoteData(), false);
	}*/
	
	// Just requery and leave the selection clear (the requery automatically clears the selection)
	DoRequeryResourcesDatalist();
}

// (j.gruber 2013-01-03 14:49) - PLID 54414 - let's requery referring physicians
void CResEntryDlg::RequeryReferringPhys()
{

	// For use in setting the appropriate selection after the requery	
	if (m_pRefPhysList == NULL) {
		m_pRefPhysList = BindNxDataList2Ctrl(this, IDC_REFERRING_PHYS_LIST, GetRemoteData(), false);
	}
	
	if (GetRemotePropertyInt("ApptShowRefPhys", 0, 0, "<None>", true) == 1) {
		m_pRefPhysList->Requery();	
	}

	//we need to add the no selection row
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRefPhysList->GetNewRow();
	if (pRow) {
		pRow->PutValue(rpcID, (long)-1);
		pRow->PutValue(rpcName, _variant_t(" { None } "));
		m_pRefPhysList->AddRowSorted(pRow, NULL);
	}
	
}

BOOL CResEntryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (j.luckoski 2012-05-08) PLID 11597 - Bulk cache properties
		// (j.luckoski 2012-05-08 17:10) - PLID 50242 - Add all properties to bulk cache
		// (r.gonet 06/05/2012) - PLID 49059 - Added ColorApptLocationComboBox
		// (j.luckoski 2012-06-14 13:23) - PLID 11597 - Added SchedCancelledApptColor
		// (j.luckoski 2012-06-20 11:23) - PLID 50242 - Removed duplicate property
		// (j.gruber 2013-01-03 16:02) - PLID 54414 - show ref phys
		// (j.luckoski 2013-03-04 13:47) - PLID 33548 - DisplayRewardsWarning preference
		// (z.manning 2013-11-18 17:28) - PLID 59597 - Added ResEntryPinned
		// (d.singleton 2014-10-13 08:34) - PLID 62634
		// (j.jones 2014-11-14 09:22) - PLID 64116 - added AutoFillApptInsurance and AutoFillApptInsurance_DefaultCategory
		g_propManager.CachePropertiesInBulk("ResEntryDlg-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ("
			"'ShowCancelledAppointment', 'CancelledDateRange', "
			"'SchedShowActivePtOnly', 'OHIP_HealthNumberCustomField', 'Alberta_PatientULICustomField', "
			"'PtTB_Security_Group',	'HideEmptyTypes', 'FormatPhoneNums', "
			"'ShowArrivalTimeOnAppts', 'ApptPromptCaseHistory', "
			"'ApptMoveUpOpenWaitingList', 'AutoActivateScheduledInactivePts', "
			"'NewPatientOpenSecurityGroups', 'AddApptDefaultDurations', 'AddPrepTimes', "
			"'WarnForInactivePtInResEntry',	'ResShowPatientWarning', 'ShowInsuranceReferralWarning', "
			"'WarnCopays', 'AutoSelectTypePurpose', 'AutoFillFollowupPurposes', "
			"'AutoFillTrackingBased', 'ShowPackageInfo', 'ApptLocationPref', "
			"'SchedHighlightLastAppt', 'ResEntry_RememberShowExtraInfo', 'NewApptPatientState', "
			"'RequestedResourceDefaultNone', 'SchedCancelledApptColor', "
			"'SchedulerUseSimpleForeColors', 'ResEntryPinned', "
			"'ColorApptLocationComboBox', 'ApptShowRefPhys', 'DisplayRewardsWarning', "
			"'NexWebPromptForLoginOnNewAppt', 'NexWebPromptForLoginOnNewApptThreshhold', "
			"'AutoFillApptInsurance', 'AutoFillApptInsurance_DefaultCategory' "
			"))", _Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("ResEntryDlg-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ('PhoneFormatString'))", _Q(GetCurrentUserName()));

		// (j.jones 2014-12-03 10:58) - PLID 64274 - added m_bIsMixRuleOverridden
		m_bIsMixRuleOverridden = false;
		// (j.luckoski 2012-04-26 10:03) - PLID 11597 - Load variables for cancelled appts
		m_bIsCancelled = false;
		m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
		m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-14 14:16) - PLID 11597 - Add cancel color so we can change it upon click to seem more responsive
		m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
		// (c.haag 2008-04-24 12:52) - PLID 29776 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnGotoPatient.AutoSet(NXB_OK);
		m_btnGotoInsurance.AutoSet(NXB_OK); // (z.manning 2008-11-24 10:03) - PLID 31130
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.armen 2012-03-28 09:48) - PLID 48480 - Check for License
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) 
		{
			m_btnCreateRecall.AutoSet(NXB_RECALL); // (b.savon 2012-02-28 16:52) - PLID 48441 - Create Recall
			m_btnCreateRecall.ShowWindow(SW_SHOWNA);

			//(a.wilson 2012-3-23) PLID 48472 - check whether the current user has permission to create recalls.
			if ((GetCurrentUserPermissions(bioRecallSystem) & (sptCreate | sptCreateWithPass))) {
				m_btnCreateRecall.EnableWindow(TRUE);
			} else {
				m_btnCreateRecall.EnableWindow(FALSE);
			}
		}
		else
		{
			m_btnCreateRecall.EnableWindow(FALSE);
			m_btnCreateRecall.ShowWindow(SW_HIDE);
		}

		m_nNewResID = -1;

		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		if(GetRemotePropertyInt("ShowArrivalTimeOnAppts", 1, 0, "<None>", true) != 0) {
			ShowArrivalTime(TRUE);
		}
		else {
			ShowArrivalTime(FALSE);
		}

		m_dlAptPatient = BindNxDataList2Ctrl(this, IDC_PATIENT_COMBO_2, GetRemoteData(), false); // (a.walling 2010-12-30 10:41) - PLID 40081 - Use datalist2 to avoid row index race conditions
		DoRequeryPatientList();

		// (j.gruber 2012-07-26 15:16) - PLID 51830 - bind the type list
		m_pInsCatList = BindNxDataList2Ctrl(IDC_RES_INS_CAT, false);
		
		m_pInsCatList->FromClause = "(SELECT 1 as ID, 'Medical' as Name "
			" UNION SELECT 2, 'Vision' "
			" UNION SELECT 3, 'Auto' "
			" UNION SELECT 4, 'Workers'' Comp.' "
			" UNION SELECT 5, 'Dental' "
			" UNION SELECT 6, 'Study' "
			" UNION SELECT 7, 'Letter of Protection'"
			" UNION SELECT 8, 'Letter of Agreement'	) Q ";
		m_pInsCatList->Requery();


		// (f.dinatale 2010-10-28) - PLID 33753 - If the user does not have permission to see the full SSN, hide it entirely.
		if(!CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) || !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
			// (a.walling 2010-12-30 11:12) - PLID 40081 - Use datalist2 to avoid row index race conditions
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_dlAptPatient->GetColumn(plSSN);
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
		}
		
		// (c.haag 2004-04-02 10:31) - These are done later on
		//RequeryAptTypes();
		//RequeryAptPurposes();

		// (r.farnworth 2015-08-13 10:46) - PLID 64009 - Error in OnReservationClick() Invalid pointer when opening an appointment
		m_dlAptType = BindNxDataListCtrl(this, IDC_APTTYPE_COMBO, GetRemoteData(), false);
		m_dlAptPurpose = BindNxDataListCtrl(this, IDC_APTPURPOSE_COMBO, GetRemoteData(), false);
		m_dlAptResource = BindNxDataListCtrl(this, IDC_APTRESOURCE_COMBO, GetRemoteData(), false);
		m_dlRequestedResource = BindNxDataListCtrl(this, IDC_REQUESTED_RESOURCE, GetRemoteData(), false);
		m_dlNoShow = BindNxDataListCtrl(this, IDC_LIST_STATUS, GetRemoteData(), false);

		// Attach to the datalist and requery at the same time
		// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
		m_dlAptLocation = BindNxDataList2Ctrl(this, IDC_APTLOCATION_COMBO, GetRemoteData(), true);

		RequeryResources();
		RequeryShowStates();

		// (j.gruber 2013-01-07 15:16) - PLID 54414
		RequeryReferringPhys();

		GetDlgItem(IDC_STATIC_LISTOFPURPOSES)->GetWindowRect(m_rcProcedures);
		GetDlgItem(IDC_STATIC_LISTOFPURPOSES)->ShowWindow(SW_HIDE);
		ScreenToClient(&m_rcProcedures);

		// (j.gruber 2012-07-26 14:22) - PLID 51830 - insurance information
		GetDlgItem(IDC_RES_PRI_LABEL)->GetWindowRect(m_rcPriIns);
		ScreenToClient(&m_rcPriIns);

		GetDlgItem(IDC_RES_SEC_LABEL)->GetWindowRect(m_rcSecIns);
		ScreenToClient(&m_rcSecIns);

		// (j.gruber 2012-08-08 10:51) - PLID 51882 - more box
		GetDlgItem(IDC_RES_INS_MORE)->GetWindowRect(m_rcInsMore);
		ScreenToClient(&m_rcInsMore);

		if(IsSpa(FALSE)) {
			GetDlgItem(IDC_REQUESTED_RESOURCE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_REQUESTED_RESOURCE_LABEL)->ShowWindow(SW_SHOW);			
		}
		else {
			GetDlgItem(IDC_REQUESTED_RESOURCE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_REQUESTED_RESOURCE_LABEL)->ShowWindow(SW_HIDE);			
		}	

		// (j.gruber 2013-01-03 14:44) - PLID 54414 - check to see if they wan to show ref phys
		if (GetRemotePropertyInt("ApptShowRefPhys", 0, 0, "<None>", true) == 1) {
			//ShowRefPhysInfo();
			//its already showing in the resources, so we don't have to do anything
		}
		else {
			HideRefPhysInfo();
		}			

		// (z.manning 2013-11-18 17:47) - PLID 59597 - We now have a setting to remember the state of the button
		if(GetRemotePropertyInt("ResEntryPinned", 0, 0, GetCurrentUserName()) == 1) {
			CheckDlgButton(IDC_PIN_RESENTRY, BST_CHECKED);
		}
		// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
		HandleResEntryPinChange();
			
		m_nxtArrival = BindNxTimeCtrl(this, IDC_ARRIVAL_TIME_BOX);
		m_nxtStart = BindNxTimeCtrl(this, IDC_START_TIME_BOX);
		m_nxtEnd = BindNxTimeCtrl(this, IDC_END_TIME_BOX);

		//TES 12/17/2008 - PLID 32479 - Set up our datalists (MoveUp, Confirmed, and Ready, all were formerly MS Combo controls)
		m_pMoveUpCombo = BindNxDataList2Ctrl(IDC_MOVEUP, false);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMoveUpCombo->GetNewRow();
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1,_bstr_t("Yes"));
		m_pMoveUpCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_pMoveUpCombo->GetNewRow();
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1,_bstr_t("No"));
		m_pMoveUpCombo->AddRowAtEnd(pRow, NULL);

		m_pConfirmedCombo = BindNxDataList2Ctrl(IDC_CONFIRMED, false);
		pRow = m_pConfirmedCombo->GetNewRow();
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1,_bstr_t("No"));
		m_pConfirmedCombo->AddRowAtEnd(pRow, NULL);pRow = m_pConfirmedCombo->GetNewRow();
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1,_bstr_t("Yes"));
		m_pConfirmedCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_pConfirmedCombo->GetNewRow();
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1,_bstr_t("LM"));
		m_pConfirmedCombo->AddRowAtEnd(pRow, NULL);

		m_pReadyCombo = BindNxDataList2Ctrl(IDC_READY, false);
		pRow = m_pReadyCombo->GetNewRow();
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1,_bstr_t("Yes"));
		m_pReadyCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_pReadyCombo->GetNewRow();
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1,_bstr_t("No"));
		m_pReadyCombo->AddRowAtEnd(pRow, NULL);
		

		// Make the font smaller if the screen is 640x480.
		if (IsScreen640x480())
		{
			extern CPracticeApp theApp;
			CWnd *tmpWnd;
			long i;

			for (i=0, tmpWnd = GetWindow(GW_CHILD); tmpWnd; i++, tmpWnd = tmpWnd->GetWindow(GW_HWNDNEXT)) 
			{
				if (tmpWnd && tmpWnd->m_hWnd)
				{
					tmpWnd->SetFont(&theApp.m_smallFont);
				}
			}
		}

		RepopulateExtraInfoCombos();

	} CResEntryDlg_NxCatchAll("CResEntryDlg::OnInitDialog");

	try {
		// Make sure all drop-downs are showing data
		m_dlAptPatient->DropDownWidth = 550;
		
		m_btnBold1.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold2.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold3.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold4.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold5.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold6.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold7.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
		m_btnBold8.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BOLD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR|LR_SHARED));
	} CResEntryDlg_NxCatchAll("CResEntryDlg::OnInitDialog-2"); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
	
// TODO: Took this out because the datalist doesn't support it	
//	m_PatientCombo.SetChangeFormatOnColumnClick("-1|-1|-1|-1|-1");

	//JJ - TACKY!
	// Make the New Patient button bold
	//extern CPracticeApp theApp;
	//GetDlgItem(IDC_BTN_NEWPATIENT)->SetFont(&theApp.m_boldFont);

	//m_NoShowCombo.ShowWindow(SW_SHOW);

	// We return FALSE because we consider ourselves to be a property page (which 
	// really means we don't want the framework to send us an initial WM_ACTIVATE 
	// message to deactivate the window (which will happen if we let it give us 
	// focus even though we're not visible))
	return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CResEntryDlg::PreTranslateMessage(MSG* pMsg)
{
	// (z.manning, 08/10/2006) - PLID 21805 - In case they changed the start time,
	// we need to process the kill focus event for it to ensure we don't get any 
	// bizarre warnings about arrival time.
	switch(pMsg->message) 
	{
		case WM_KEYDOWN:
			switch(pMsg->wParam) 
			{
				case VK_RETURN:
					OnKillFocusStartTimeBox();
					break;
			}
			break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

// (c.haag 2009-01-14 09:54) - PLID 32712 - This function checks whether this is a surgery appointment; and if so,
// looks for the most recent consult for that patient, and ensures it has the same master procedures this appt has.
// (a.wilson 2014-08-28 16:51) - PLID 63170 - modified to use full size appointment tablechecker.
void CResEntryDlg::SyncSurgeryPurposesWithConsult(IN const long nPatID, const COleDateTime& dtDate, const COleDateTime& dtStart)
{
	CKeepDialogActive kda(this); // I don't know for sure whether I actually need this, but this feature works fine. Still, I won't take any chances.
	try {
		if(nPatID == -25)
			return;

		//NOTE
		// (d.thompson 2009-08-12) - PLID 33992 - Some clients have requested to turn off the prompt.  m.clark says
		//	we absolutely will not do this.  If they are getting prompted too often, it's because they've setup their
		//	purposes wrong, and should work with tech support to fix them.


		// And its type is a Surgery category
		short nCat = GetCurAptTypeCategory();
		// (j.gruber 2009-07-09 10:13) - PLID 34303 - also do it for minor procedures
		if (nCat == PhaseTracking::AC_SURGERY || nCat == PhaseTracking::AC_MINOR) {
			
			// Look for the most recent non-No-Show non-cancelled appointment with a Consult category that
			// pre-dates this new appointment. Also get the master procedures for it.
			//
			// Additionally, get a list of master procedures for this appointment.
			//
			// Do it all in one trip to the server.
			//
			COleDateTime dtDateTime;
			dtDateTime.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
			_RecordsetPtr prs = CreateParamRecordset(
				"DECLARE @ApptID INT;\r\n"
				"/* Find the consult */\r\n"
				"SET @ApptID = COALESCE((SELECT TOP 1 AppointmentsT.ID FROM AppointmentsT LEFT JOIN AptTypeT ON AptTypeT.ID = AptTypeID "
				"WHERE PatientID = {INT} AND Status <> 4 AND ShowState <> {INT} AND Category = {INT} AND StartTime < convert(datetime, {STRING}) ORDER BY StartTime DESC),0);\r\n"
				"IF @ApptID > 0 BEGIN\r\n"
				"	/* Get consult information */\r\n"
				"	SELECT AppointmentsT.ID, PatientID, LocationID, Status, ShowState, StartTime, EndTime, \r\n"
				"	dbo.GetResourceIDString(@ApptID) AS ResourceIDs, AptTypeT.Name, dbo.GetPurposeString(@ApptID) AS Purposes, Notes \r\n"
				"	FROM AppointmentsT \r\n"
				"	LEFT JOIN AptTypeT ON AptTypeT.ID = AptTypeID\r\n"
				"	WHERE AppointmentsT.ID = @ApptID\r\n"

				"	/* Get all the procedures for the consult and their master ID's */\r\n"
				"	SELECT ProcedureT.ID, ProcedureT.Name, ProcedureT.MasterProcedureID, Master.Name AS MasterName "
				"	FROM ProcedureT\r\n"
				"	LEFT JOIN ProcedureT Master ON Master.ID = ProcedureT.MasterProcedureID "
				"	INNER JOIN AppointmentPurposeT ON AppointmentPurposeT.PurposeID = ProcedureT.ID\r\n"
				"	WHERE AppointmentPurposeT.AppointmentID = @ApptID ORDER BY ProcedureT.Name\r\n"

				"	/* Get all the procedures for the surgery and their master ID's */\r\n"
				"	SELECT ProcedureT.ID, ProcedureT.Name, ProcedureT.MasterProcedureID, Master.Name AS MasterName "
				"	FROM ProcedureT\r\n"
				"	LEFT JOIN ProcedureT Master ON Master.ID = ProcedureT.MasterProcedureID "
				"	WHERE ProcedureT.ID IN ({INTSTRING}) ORDER BY ProcedureT.Name\r\n"
				
				"	/* Get the new Appointment information for tablechecker. */\r\n"
				"	SELECT PatientID, LocationID, Status, ShowState, StartTime, EndTime, dbo.GetResourceIDString(ID) AS ResourceIDs FROM AppointmentsT WHERE ID = {INT}\r\n"
				"END\r\n", nPatID, CAppointmentsDlg::astNoShow, PhaseTracking::AC_CONSULT, FormatDateTimeForSql(dtDateTime), ArrayAsString(m_adwPurposeID), m_nNewResID);
			int i,j;
			if (prs->State == adStateClosed) {
				// Could not find a qualifying consult. Nothing to do.
				return;
			}

			// Now that we have the consult, we need to fetch basic information about it
			const long nConsultID = AdoFldLong(prs, "ID");
			long nPatientID = AdoFldLong(prs, "PatientID");
			long nLocationID = AdoFldLong(prs, "LocationID");
			long nStatus = (long)AdoFldByte(prs, "Status");
			long nShowState = AdoFldLong(prs, "ShowState");
			COleDateTime dtStart = AdoFldDateTime(prs, "StartTime");
			COleDateTime dtEnd = AdoFldDateTime(prs, "EndTime");
			CString strConsultPurposes = AdoFldString(prs, "Purposes", "");
			CString strNotes = AdoFldString(prs, "Notes", "");
			CString strResourceIDs = AdoFldString(prs, "ResourceIDs", "");
			if (strConsultPurposes.IsEmpty()) { strConsultPurposes = "<No Purposes>"; }
			if (!strNotes.IsEmpty()) { strNotes = " - " + strNotes; }
			const CString strApptText = FormatString("%s - %s - %s%s", FormatDateTimeForInterface(dtStart), AdoFldString(prs, "Name", "<No Type>"), strConsultPurposes, strNotes);
			CMap<long,long,CString,LPCTSTR> mapProcedureIDToName;
			CMap<long,long,long,long> mapProcedureIDToMasterID;

			// Now fetch the procedures for the consult. Also gather procedure names for auditing,
			// and master ID's for referencing later.
			CArray<long,long> anConsultProcs;
			prs = prs->NextRecordset(NULL);
			FillProcedureData(prs, anConsultProcs, mapProcedureIDToName, mapProcedureIDToMasterID);

			// Now fetch the procedures for the surgery. Also gather procedure names for auditing,
			// and master ID's for referencing later.
			CArray<long,long> anSurgeryProcs;
			prs = prs->NextRecordset(NULL);
			FillProcedureData(prs, anSurgeryProcs, mapProcedureIDToName, mapProcedureIDToMasterID);

			prs = prs->NextRecordset(NULL);

			// (c.haag 2009-01-21 13:30) - PLID 32712 - If the surgery has no procedures, offer to
			// copy them from the consult.
			if (0 == anSurgeryProcs.GetSize()) {
				CString strMsg;
				// (j.gruber 2009-07-09 10:16) - PLID 34303 - updated the text for minor procedures
				strMsg.Format("This %s has no procedures. Would you like to copy them from the patient's most recent consult?\r\n"
					"\r\n"
					"%s", 
					nCat==PhaseTracking::AC_SURGERY ? "surgery" : "appointment",					
					strApptText);
				if (IDYES == MsgBox(MB_YESNO | MB_ICONQUESTION, strMsg)) {
					// Change the surgery
					ExecuteSql("INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) "
						"SELECT %d, ID FROM ProcedureT WHERE ID IN (%s)",
						m_nNewResID, ArrayAsString(anConsultProcs));

					// Do auditing. The purposes don't have to be in exact alphabetical order, but we try to come reasonably close.
					long nAuditID = BeginNewAuditEvent();
					// Generate the new audit value
					CString strPurposeNew;
					for (i=0; i < anConsultProcs.GetSize(); i++) {
						CString strProcName;
						mapProcedureIDToName.Lookup(anConsultProcs[i], strProcName);
						if (strPurposeNew.IsEmpty()) {
							strPurposeNew = strProcName;
						} else {
							strPurposeNew += ", " + strProcName;
						}
					}
					AuditEvent(nPatID, GetExistingPatientName(nPatID), nAuditID, aeiApptPurpose, m_nNewResID, "", strPurposeNew, aepMedium, aetChanged);

					// Fire a table checker
					if (!prs->eof) {
						CClient::RefreshAppointmentTable(m_nNewResID, AdoFldLong(prs, "PatientID"), AdoFldDateTime(prs, "StartTime"), AdoFldDateTime(prs, "EndTime"), 
							(long)AdoFldByte(prs, "Status"), AdoFldLong(prs, "ShowState"), AdoFldLong(prs, "LocationID"), AdoFldString(prs, "ResourceIDs", ""));
					}
				}
				return;
			} // if (0 == anSurgeryProcs.GetSize()) {

			// If the array contents already match, or there are no surgery procedures, there's nothing to do.
			if (AreArrayContentsMatched(anConsultProcs,anSurgeryProcs)) {
				return;
			}
			// (c.haag 2009-01-21 11:35) - PLID 32712 - Do a second comparison. This time, go through all
			// the surgery procedures and see if their master equivalents are in the consult. If so, then
			// there's nothing to do.
			BOOL bSurgHasMasterThatConsDoesnt = FALSE;
			for (i=0; i < anSurgeryProcs.GetSize() && !bSurgHasMasterThatConsDoesnt; i++) {
				long nSurgProcID = anSurgeryProcs[i];
				long nMasterID = -1;
				// Make sure nSurgProcID is a master equivalent of itself
				if (mapProcedureIDToMasterID.Lookup(nSurgProcID, nMasterID)) {
					nSurgProcID = nMasterID;
				} else {
					// Already a master
				}
				if (!IsIDInArray(nSurgProcID, anConsultProcs)) {
					// If we get here, the surgery has a master ID, or a detail pointing to a master ID
					// that the consult does not.
					bSurgHasMasterThatConsDoesnt = TRUE;
				}
			}
			if (!bSurgHasMasterThatConsDoesnt) {
				// If we get here, then all the master procedures that are related in some way to the surgery, 
				// already exist in the consult. Nothing to do
				return;
			}

			// At this point, we have all the procedures for both the consult and this new surgery, and 
			// their master ID's. We are trying to enforce the following:
			//
			// *** We want the consult to match the surgery as closely as possible; but if detail procedure content is
			// inconsistent for the same master procedure, then assign the master procedure to the consult instead of 
			// any details. ***
			//
			// So for example, if the sx is "Abdominoplasty", the consult must have "Abdominoplasty".
			// If the sx is "Abdominoplasty - Mini", the consult must have either "Abdominoplasty - Mini"
			// or "Abdominoplasty".
			//
			//	To do this, we:
			//
			//	1. Copy anNormalizedSurgeries with content from anSurgeryProcs
			//	2. Remove any master procedures from anNormalizedSurgeries that are masters of detail procedures inside
			//		that same array.
			//	3. Use a global scheduling utility function to calculate the set of procedure ID's which best meet
			//		the starred summary criteria above. The result set ONLY CONTAINS SURGERY PROCEDURES.
			//	4. Find out whether the user wants to replace the consult procedures with the result set, or add them
			//		to the consult.
			//	5. If we're doing a replace, then we're done with anIdealProcedures. If doing an addage, we add all
			//		consult procedures who themselves, and their masters, don't exist in anIdealProcedures, to
			//		anIdealProcedures.
			//
			CArray<long,long> anNormalizedSurgeries;
			CArray<long,long> anIdealProcedures;
			anNormalizedSurgeries.Copy(anSurgeryProcs); // 1
			PruneMasterIDsOfDetailProcedures(anNormalizedSurgeries, mapProcedureIDToMasterID); // 2
			CalculateIdealConsultProcedures(anNormalizedSurgeries, anConsultProcs, mapProcedureIDToMasterID, anIdealProcedures); // 3

			if (anIdealProcedures.GetSize() > 0) {
				// If we get here, we need to change the consult. The options include either doing nothing, adding
				// procedures to the consult, or replacing the consult procedures with master surgery procedures. 
				// Here is an example truth table fragment:
				//
				// Consult			Surgery			Action			New Consult Procedures
				//========================================================================
				//Abdominoplasty	<none>			<prompt the user to add the consult procedures to the surgery>
				//Abd.				Abd.			<user is not prompted; consult already matches>
				//Abd.				Blepharoplasty	Add				Abd., Bleph.
				//Abd.				Blepharoplasty	Replace			Bleph.
				//
				//Abd. Extended		Abd.			Add				Abd., Abd. Extended (*Users should normally use a detail procedure when booking a surgery like this.)
				//Abd. Extended		Abd.			Replace			Abd. (*Users should normally use a detail procedure when booking a surgery like this.)
				//Abd.				Abd. Extended	<user is not prompted>

				//Bleph				Abd. Extended	Add				Abd., Bleph.
				//Bleph				Abd. Extended	Replace			Abd.
				//Bleph				Abd. Ext & Mini	Replace			Abd.
				//
				//Abd. Mini			Abd., Abd. Mini	Add				Abd., Abd. Mini (*The surgery shouldn't have a master anyway; but since
				//																	its a master, we have to carry it over to the consult)
				//Abd., Abd. Mini	Abd. Mini		<user is not prompted>
				//
				//Abd. Extended		Abd. Mini		Add				Abd. Extended, Abd. Mini (*Abd. Extended has the same master as Abd. Mini)
				//Abd. Extended		Abd. Mini		Replace			Abd.
				//
				CString strPurposeOld = strConsultPurposes;
				CResolveConsSurgBookingDlg dlg((PhaseTracking::AptCategory)nCat, this);
				dlg.m_strApptText = strApptText;
				if (IDOK == dlg.DoModal()) { // 4
					switch (dlg.m_Action) {
						case CResolveConsSurgBookingDlg::eDoNothing:
							// Do nothing
							return;
						case CResolveConsSurgBookingDlg::eAddProcedures:
							//
							// Adding procedures is not so straight-forward. We want to avoid taking a 
							// Consult with "Abd. Extended" and a surgery with "Abd. Mini", and making 
							// it "Abd. Extended, Abdombinoplasty". It's like reading "Well we're having
							// an extended abdominplasty...and, some other kind of abdominoplasty"...or 
							// reading "We're only having an extended abdominoplasty."
							//
							// To accomplish a result like "Abd. Extended, Abd. Mini", we do three passes
							// on the surgery array:
							//
							// 1. Add all surgery master procedures to the consult that don't already exist
							//
							// 2. For all surgery detail procedures, add corresponding master procedures
							// that do not exist in the consult AND do not own any consult detail procedure.
							//
							// 3. For all surgery detail procedures, add corresponding detail procedures
							// that do not exist in the consult AND own at least one consult detail procedure.
							//

							// Fill anIdealProcedures with the consult procedures
							anIdealProcedures.Copy(anConsultProcs);

							// Add surgery procedures to anIdealProcedures. First, add only the
							// master procedures in the actual surgery appointment.
							for (i=0; i < anSurgeryProcs.GetSize(); i++) {
								const long nSurgProcID = anSurgeryProcs[i];
								long nSurgMasterID = -1;
								mapProcedureIDToMasterID.Lookup(nSurgProcID, nSurgMasterID);
								if (-1 == nSurgMasterID) {
									if (!ExistsInArray(nSurgProcID, anIdealProcedures)) {
										anIdealProcedures.Add(nSurgProcID);
									}
								}
							} // for (i=0; i < anSurgeryProcs.GetSize(); i++) {

							// Now add master procedures that don't exist by themselves or
							// as a master of any existing detail procedure.
							for (i=0; i < anSurgeryProcs.GetSize(); i++) {
								const long nSurgProcID = anSurgeryProcs[i];
								long nSurgMasterID = -1;
								mapProcedureIDToMasterID.Lookup(nSurgProcID, nSurgMasterID);

								if (-1 == nSurgMasterID) {
									// Do nothing; nSurgProcID was already added to the consult
									// in the previous loop
								}
								else if (ExistsInArray(nSurgProcID, anIdealProcedures)) {
									// Do nothing; nSurgProcID is already in the consult
								}
								else if (ExistsInArray(nSurgMasterID, anIdealProcedures)) {
									// Do nothing. The consult already owns the master.
								}
								else {
									// See if any detail procedures in anIdealProcedures have
									// nSurgMasterID as a master. If not, then add nSurgMasterID
									// to anIdealProcedures.
									BOOL bHasCommonMaster = FALSE; 
									for (j=0; j < anIdealProcedures.GetSize() && !bHasCommonMaster; j++) {
										//TES 12/11/2009 - PLID 36564 - This was pulling from the wrong array.
										const long nConsProcID = anIdealProcedures[j];
										long nConsMasterID = -1;
										mapProcedureIDToMasterID.Lookup(nConsProcID, nConsMasterID);
										if (-1 != nConsMasterID && nConsMasterID == nSurgMasterID) {
											bHasCommonMaster = TRUE;
										}
									}
									if (!bHasCommonMaster) {
										anIdealProcedures.Add(nSurgMasterID);
									}
								}
							} // for (i=0; i < anSurgeryProcs.GetSize(); i++) {

							// Finally, add surgery procedure details where the master doesn't actually
							// exist in the array, but the master is shared by other details.
							for (i=0; i < anSurgeryProcs.GetSize(); i++) {
								const long nSurgProcID = anSurgeryProcs[i];
								long nSurgMasterID = -1;
								mapProcedureIDToMasterID.Lookup(nSurgProcID, nSurgMasterID);

								if (-1 == nSurgMasterID) {
									// Do nothing; nSurgProcID was already added to the consult
									// in the previous loop
								}
								else if (ExistsInArray(nSurgProcID, anIdealProcedures)) {
									// Do nothing; nSurgProcID is already in the consult
								}
								else if (ExistsInArray(nSurgMasterID, anIdealProcedures)) {
									// Do nothing. The consult already owns the master.
								}
								else {
									// See if any detail procedures in anIdealProcedures have
									// nSurgMasterID as a master. If so, then add nSurgProcID
									// to anIdealProcedures.
									BOOL bHasCommonMaster = FALSE; 
									for (j=0; j < anIdealProcedures.GetSize() && !bHasCommonMaster; j++) {
										//TES 12/11/2009 - PLID 36564 - This was pulling from the wrong array.
										const long nConsProcID = anIdealProcedures[j];
										long nConsMasterID = -1;
										mapProcedureIDToMasterID.Lookup(nConsProcID, nConsMasterID);
										if (-1 != nConsMasterID && nConsMasterID == nSurgMasterID) {
											bHasCommonMaster = TRUE;
										}
									}
									if (bHasCommonMaster) {
										anIdealProcedures.Add(nSurgProcID);
									}
								}
							} // for (i=0; i < anSurgeryProcs.GetSize(); i++) {
							break;
						case CResolveConsSurgBookingDlg::eReplaceProcedures:
							// Nothing to do; anIdealProcedures is already what we want it to be
							break;
					}

					// Last check to see if the two arrays match up. If they do, changing the data
					// will have no effect.
					if (AreArrayContentsMatched(anConsultProcs,anIdealProcedures)) {
						return;
					}

					// Change the consult
					ExecuteSql("DELETE FROM AppointmentPurposeT WHERE AppointmentID = %d;\r\n"
						"INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) "
						"SELECT %d, ID FROM ProcedureT WHERE ID IN (%s)",
						nConsultID, nConsultID, ArrayAsString(anIdealProcedures));

					// Do auditing. The purposes don't have to be in exact alphabetical order, but we try to come reasonably close.
					long nAuditID = BeginNewAuditEvent();
					// Generate the new audit value
					CString strPurposeNew;
					for (i=0; i < anIdealProcedures.GetSize(); i++) {
						CString strProcName;
						mapProcedureIDToName.Lookup(anIdealProcedures[i], strProcName);
						if (strPurposeNew.IsEmpty()) {
							strPurposeNew = strProcName;
						} else {
							strPurposeNew += ", " + strProcName;
						}
					}
					AuditEvent(nPatID, GetExistingPatientName(nPatID), nAuditID, aeiApptPurpose, nConsultID, strPurposeOld, strPurposeNew, aepMedium, aetChanged);

					// Fire a table checker
					CClient::RefreshAppointmentTable(nConsultID, nPatientID, dtStart, dtEnd, nStatus, nShowState, nLocationID, strResourceIDs);

				} // if (IDOK == dlg.DoModal()) { // 4
				else {
					// Do not change the consult
				}
			} // if (anIdealProcedures.GetSize() > 0) {
			else {
				// This should never happen
			}
		} // if (nCat == PhaseTracking::AC_SURGERY) {
	} CResEntryDlg_NxCatchAll("CResEntryDlg::SyncSurgeryPurposesWithConsult");
}

void CResEntryDlg::CheckCreateCaseHistory(IN const long nPatID, IN const COleDateTime &dtSurgeryDate, const long nAppointmentID)
{
	CKeepDialogActive kda(this);
	try {
		// If it's a new appointment
		if (m_NewRes) {

			if(nPatID == -25)
				return;

			// And its type is a Surgery category
			short nCat = GetCurAptTypeCategory();
			if (nCat == PhaseTracking::AC_SURGERY) {
				// Then prompt the user to create a case history
				int nAskCreateResult = MsgBox(MB_ICONQUESTION|MB_YESNO, 
					"You have just saved a new Surgery appointment.  Would you like to "
					"create a case history for this patient?");
				if (nAskCreateResult == IDYES) {
					// They want to make a case history
					// Have the user choose the surgery upon which this case history is to be based
					CString strCaseHistoryName;
					long nProviderID; //we used to call CalcDefaultCaseHistoryProvider, but is now in the ChooseSurgery
					// (c.haag 2007-03-09 11:12) - PLID 25138 - Pass in the patient ID as well
					// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
					// (j.jones 2009-08-31 15:07) - PLID 35378 - we now allow multiple preference cards to be chosen
					CArray<long, long> arynPreferenceCardIDs;
					// (j.jones 2009-08-31 17:54) - PLID 17734 - this now takes in the appointment ID
					if(CCaseHistoryDlg::ChoosePreferenceCards(arynPreferenceCardIDs, strCaseHistoryName, nProviderID, m_nCurPatientID, nAppointmentID)) {
						
						// (j.jones 2006-12-18 15:30) - PLID 23578 - just open it and allow them to change things before saving
						CCaseHistoryDlg dlg(this);
						// (j.jones 2009-08-26 09:52) - PLID 34943 - we no longer pass in location ID, a preference inside this function will calculate it
						dlg.OpenNewCase(nPatID, arynPreferenceCardIDs, strCaseHistoryName, nProviderID, dtSurgeryDate, nAppointmentID);

						/*						
						// Create the case history for this patient based on the given surgery (this also puts it in MailSent automatically)
						long nCaseHistoryID = CCaseHistoryDlg::CreateCaseHistory(nPatID, nSurgeryID, strName, nProviderID, GetCurrentLocationID(), dtSurgeryDate, nAppointmentID);

						// Now that it's been created, see if they want to edit it
						int nAskEditResult = MsgBox(MB_ICONQUESTION|MB_YESNO, 
							"The Case History has been created.  Would you like to edit it now?");
						if (nAskEditResult == IDYES) {
							// Show the new case history on screen, allowing it to be edited
							CCaseHistoryDlg dlg;
							dlg.OpenExistingCase(nCaseHistoryID);
						}
						*/
					}
				}
			}
		}
	} CResEntryDlg_NxCatchAll("CResEntryDlg::CheckCreateCaseHistory");
}

// (j.jones 2010-01-04 10:53) - PLID 32935 - added parameter to disable checking for required allocations
void CResEntryDlg::SaveRes(bool bAutoDelete /* = true */, bool bCheckForAllocations /*= true*/)
{
	// (c.haag 2010-11-29 11:56) - PLID 41524 - It's possible during saving that modal windows will appear. During
	// those times, table checker messages can appear and try to destroy the appointment. We must reserve this
	// reservation until the save is complete.
	CReserveResEntry rre(this, __FUNCTION__);	
	CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
	BOOL bNeedsRefiltering = FALSE;
	BeginWaitCursor();

	m_dtStartTime = m_nxtStart->GetDateTime();
	m_dtEndTime = m_nxtEnd->GetDateTime();
	m_dtArrivalTime = m_nxtArrival->GetDateTime();

	// (z.manning, 10/12/2006) - PLID 23021 - Make sure we don't save an end time of midnight unless it's an event.
	if( m_dtEndTime.GetHour() == 0 && m_dtEndTime.GetMinute() == 0 &&
		 !(m_dtStartTime.GetHour() == 0 && m_dtStartTime.GetMinute() == 0) )
	{
		m_dtEndTime.SetDateTime(m_dtEndTime.GetYear(), m_dtEndTime.GetMonth(), m_dtEndTime.GetDay(), 23, 59, 0);
	}

	UpdateData();

	long nPatientID, nPurposeID=0;

	// Get the proper patient ID
	/*long nPatCurSel = m_PatientCombo->CurSel;
	if (nPatCurSel != -1) {
		nPatientID = VarLong(m_PatientCombo->Value[nPatCurSel][0]);
	} else {
		nPatientID = -25;
	}*/
	nPatientID = GetCurPatientID();

	// Block times do not have patients
	if (m_dlAptType->CurSel > 0)
	{
		short nCat = GetCurAptTypeCategory();
		if (nCat == PhaseTracking::AC_BLOCK_TIME)
		{
			nPatientID = -25;
		}
	}

	// Log the patient ID
	Log("::SaveRes (Closing ResEntryDlg) patient ID = %d appt ID = %d", nPatientID, m_ResID);

	// Get the appropriate AptPurposeID
	long nAptPurposeID;
	long nAptPurposeIndex = m_dlAptPurpose->CurSel;
	if (nAptPurposeIndex != -1) {
		nAptPurposeID = VarLong(m_dlAptPurpose->Value[nAptPurposeIndex][0], -1);
	} else {
		nAptPurposeID = -1;
	}
	// Get the appropriate AptTypeID
	long nAptTypeID = GetCurAptTypeID();
	//TES 6/21/2010 - PLID 21341 - And the current LocationID
	long nLocationID = GetCurLocationID();

	// Get the rest of the values
	COleDateTime dtDate = GetDate();

	bool bSaveSuccess = false;
	BOOL bNeedUpdate = IsSchedulerDirty();
	m_nNewResID = -1;
	CNxSchedulerDlg *pSheet = NULL;
	
	// First find out if the active sheet already needs to be updated
	if (m_pSchedulerView) {
		pSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();
	}

	//TES 4/19/04: Save the "Requested" resource.
	long nRequestedResource;
	if(m_dlRequestedResource->CurSel != -1 && m_dlRequestedResource->GetValue(m_dlRequestedResource->CurSel, 0).vt == VT_I4) {
		nRequestedResource = VarLong(m_dlRequestedResource->GetValue(m_dlRequestedResource->CurSel, 0));
	}
	else {
		nRequestedResource = -1;
	}
	

	// CAH 5/28/02 - No longer necessary now that we use _Q()
	// ADDED BY CHRIS 7/1 - allows quotes in notes
	//m_Notes.Replace("\"", "\"\"");

	// (a.walling 2006-10-31 13:35) - PLID 23195 - This var was declared later on, but it will come in handy in the
	//	AppointmentCreate/Modify blocks.
	long nCurStatus = GetNoShowComboSel();
	//(e.lally 2008-06-20) PLID 29500 - Get the text name too
	CString strShowStateName = 	GetNoShowComboSelName();	

	// Now actually do the action (create new or edit current)
	if (m_NewRes) {
		// Create new

		// CreateAppointment. Note that we are declining to send a tablechecker message in this
		// function call; that's because we don't want to send it until AFTER we assign the
		// reservation ID to our reservation object. If we don't do this, it may visibly
		// duplicate the appointment, and that will make Chris and Bob distressed.
		
		
		// (m.cable 2004-05-28 15:56) - PLID 11108 - Check Location
		long nLocationID;
		CDWordArray const &arydwResources = GetCurResourceIDList();
		//TES 8/10/2010 - PLID 39264 - Pass in the Date and StartTime
		if (!GetValidAppointmentLocation(GetCurLocationID(), nLocationID, m_ResID, dtDate, m_dtStartTime, &arydwResources)) {
			bSaveSuccess = false;
		} else {
			//reflect the real location
			if(nLocationID != GetCurLocationID()) {
				SetCurLocation(nLocationID, GetCurLocationName());
			}
			// (j.jones 2010-01-04 10:55) - PLID 32935 - pass in the allocation check boolean
			// (j.gruber 2012-07-30 14:36) - PLID 51869 - send in the map
			// (j.gruber 2013-01-08 09:07) - PLID 54483 - added refphyID
			m_nNewResID = AppointmentCreate(nPatientID, GetCurResourceIDList(), nLocationID, 
				dtDate, m_dtStartTime, m_dtEndTime, m_dtArrivalTime, m_nConfirmed, m_bMoveUp, 
				GetNoShowComboSel(), m_Notes, nAptTypeID, m_adwPurposeID, m_bReady, nRequestedResource, FALSE, bCheckForAllocations, &m_mapInsPlacements, m_nCurRefPhysID);
			// (d.moore 2007-10-16) - PLID 26546 - We need to check to see if the new appointment 
			//  might satisfy a request in the waiting list.
			// (c.haag 2010-05-26 15:19) - PLID 38652 - This entails a modal loop, so don't check 
			// until after we let go of the reservation (otherwise UpdateView is called and m_Res gets killed)
			if (m_nNewResID > 0) {
				m_Res.PutReservationID(m_nNewResID);
				// (c.haag 2009-12-21 17:21) - PLID 36691 - Assign the appointment type and patient ID
				m_Res.PutAptTypeID(nAptTypeID);
				m_Res.PutPersonID(nPatientID);
				// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
				// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
				CString strResourceIDs = ArrayAsString(GetCurResourceIDList());
				//this is space-delimited
				strResourceIDs.Replace(",", " ");
				CClient::RefreshAppointmentTable(m_nNewResID, nPatientID, dtDate + m_dtStartTime, dtDate + m_dtEndTime, 1, GetNoShowComboSel(), nLocationID, strResourceIDs);
				// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
				// (r.gonet 12/03/2012) - PLID 54106 - Changed to use refactored send function.
				SendNewAppointmentHL7Message(m_nNewResID, true);
				bSaveSuccess = true;
			} else {
				// On failure delete the appointment box
				bSaveSuccess = false;
				if (bAutoDelete) {
					// Delete the m_Res object from the screen (this means decrement our reference, but 
					// ALSO tell the IReservation object to take itself out of whatever Singleday it's in)
					KillThisResObject(TRUE); // (j.luckoski 2012-05-02 16:23) - PLID 11597 - Pass true
				}
			}


			 //(a.walling 2006-10-31 13:33) - PLID 23195 - Set the reason for noshow if a new appointment is made no show
			//		(Why this would be done? who knows. but it's plausible.)
			// they marked the appt no show, lets find out why
			//(e.lally 2008-06-20) PLID 29500 - Unrelated to this item, I changed the hardcoded 3 to the new enum value for readability

			if(nCurStatus == assNoShow){
				//check if the status actually changed
				if(nCurStatus != m_nAptInitStatus){
					CReasonDlg dlg(this);
					dlg.m_bNoShow = true;
					dlg.m_strText = "Are you sure you want to mark this appointment as No Show?";
					dlg.m_nApptID = m_nNewResID;
					if(IDOK == dlg.DoModal()) {
						// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
						if(dlg.IsCustomReason()) {
							ExecuteParamSql("UPDATE AppointmentsT SET NoShowReason = {STRING} WHERE ID = {INT}", dlg.m_strReason, m_nNewResID);
						}
						else {
							long nReasonID;
							dlg.GetReason(nReasonID);
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							ExecuteParamSql("UPDATE AppointmentsT SET NoShowReasonID = {INT} WHERE ID = {INT}", nReasonID, m_nNewResID);
						}
						// (a.walling 2014-12-22 10:48) - PLID 64366 - Reschedule when marking as No Show
						if (dlg.m_bReschedule) {
							Nx::Scheduler::AddToReschedulingQueue(m_nNewResID, nPatientID);
						}
						//(e.lally 2008-06-20) PLID 29500 - We would update linked appointment show states here,
							//but the interface only allows appointment linking from existing appointments, not new ones
							//which we are here. I'm leaving the untested function call here in comments in case the interface ever changes
							//and we need to reference this.
						//UpdateLinkedAppointmentShowState(m_nNewResID, nCurStatus, strShowStateName, &dlg);
					}
					// (d.singleton 2011-12-21 10:05) - PLID 47110 check to see if the preference to require a reason is enabled,  if so save new appt as Pending
					else if(dlg.m_bReqReason)
					{
						AfxMessageBox("It is required you enter a reason before marking as no show, the appointment will save with pending status");
						ExecuteParamSql("UPDATE AppointmentsT SET ShowState = 0, NoShowDate = NULL WHERE ID = {INT}", m_nNewResID);
					}
						
				}//end if status changed
			}
			else{
				//(a.wilson 2012-2-29) PLID 48420 - need to prompt use if they have any unlinked recalls when creating an appointment.
				CheckForRecalls();
				// the appointment isn't a no show, make sure there isn't a reason filled in
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				ExecuteParamSql("UPDATE AppointmentsT SET NoShowReason = '', NoShowReasonID = NULL WHERE ID = {INT}", m_nNewResID);
			}

		}
	} // if (m_NewRes)
	else {
		// Edit current
		long nLocationID;
		//TES 8/10/2010 - PLID 39264 - Pass in the Date and StartTime
		if (!GetValidAppointmentLocation(GetCurLocationID(), nLocationID, m_ResID, dtDate, m_dtStartTime))
		{
			bSaveSuccess = false;
		}
		else
		{
			//reflect the real location
			if(nLocationID != GetCurLocationID()) {
				SetCurLocation(nLocationID, GetCurLocationName());
			}
			// (a.walling 2006-10-31 13:34) - PLID 23195 - We put the cancellation description in the todo-followup
			// via tracking functions. However, the reasons were not being set here until after the tracking functions
			// were called, so we handle it now beforehand.
			// they marked the appt no show, lets find out why
			//(e.lally 2008-06-20) PLID 29500 - Unrelated to this item, I changed the hardcoded 3 to the new enum value for readability
			if(nCurStatus == assNoShow)
			{
			//	//check if the status actually changed
				if(nCurStatus != m_nAptInitStatus)
				{
					CReasonDlg dlg(this);
					dlg.m_bNoShow = true;
					dlg.m_strText = "Are you sure you want to mark this appointment as No Show?";
					dlg.m_nApptID = m_ResID;
					if(IDOK == dlg.DoModal()) {
						if(dlg.IsCustomReason()) {
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							ExecuteParamSql("UPDATE AppointmentsT SET NoShowReason = {STRING} WHERE ID = {INT}", dlg.m_strReason, m_ResID);
						}
						else {
							long nReasonID;
							dlg.GetReason(nReasonID);
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							ExecuteParamSql("UPDATE AppointmentsT SET NoShowReasonID = {INT} WHERE ID = {INT}", nReasonID, m_ResID);
						}
						// (a.walling 2014-12-22 10:48) - PLID 64366 - Reschedule when marking as No Show
						if (dlg.m_bReschedule) {
							Nx::Scheduler::AddToReschedulingQueue(m_ResID, nPatientID);
						}
						//(e.lally 2008-06-20) PLID 29500 - Update linked appointment show states
						UpdateLinkedAppointmentShowState(m_ResID, nCurStatus, strShowStateName, &dlg);
					}
					// (d.singleton 2011-12-21 10:05) - PLID 47110 check to see if the preference to require a reason is enabled,  if so revert back to original status
					else if(dlg.m_bReqReason) {
						AfxMessageBox("It is required you enter a reason before marking as no show, the appointment will revert back to its original status");
						ExecuteParamSql("UPDATE AppointmentsT SET ShowState = {INT}, NoShowDate = NULL WHERE ID = {INT}", m_nAptInitStatus, m_ResID);
						//set the status drop down to original status
						m_dlNoShow->SetSelByColumn(0, m_nAptInitStatus);
					}

					// (z.manning 2011-04-01 15:11) - PLID 42973 - We may need to void superbills since this is an existing appt.
					int nSuperbillResult = NeedsToVoidNoShowSuperbills(CSqlFragment("AppointmentsT.ID = {INT}", m_ResID), false);
					if(nSuperbillResult == IDYES) {
						ExecuteParamSql(
							"UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = {STRING} \r\n"
							"WHERE PrintedSuperbillsT.ReservationID = {INT} AND Void = 0 \r\n"
							, GetCurrentUserName(), m_ResID);
					}
				}//end if status changed
			}
			else{
				// the appointment isn't a no show, make sure there isn't a reason filled in
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				ExecuteParamSql("UPDATE AppointmentsT SET NoShowReason = '', NoShowReasonID = NULL WHERE ID = {INT}", m_ResID);
			}


			// (z.manning, PLID16278, 06/27/05)
			// Make sure that there are no message boxes that can possible pop up between this statement
			//   and the statement: m_Res.SetDispatch(NULL);
			// (j.jones 2010-01-04 10:55) - PLID 32935 - pass in the allocation check boolean
			// (j.gruber 2012-07-30 14:36) - PLID 51869 - send in the maps
			// (j.gruber 2013-01-08 10:23) - PLID 54483 - add refphysID
			bSaveSuccess = AppointmentModify(m_ResID, nPatientID, GetCurResourceIDList(), nLocationID,
				dtDate, m_dtStartTime, m_dtEndTime, m_dtArrivalTime, m_nConfirmed, m_bMoveUp, 
				GetNoShowComboSel(), m_Notes, nAptTypeID, m_adwPurposeID, &m_schedAuditItems, (m_bReady), nRequestedResource, bCheckForAllocations, &m_mapOrigIns, &m_mapInsPlacements, m_nCurRefPhysID);
		}

		m_nNewResID = m_ResID;
	}
	
	// Reflect changes in the current view's box
	//(e.lally 2005-08-02) PLID 16910 - the logic here did not break up the save success and need update into
		//two if statements. This caused an appointment that was opened and saved to not clear out junk values
		//on the scheduler. The Is-Scheduler-Dirty() function and Scheduler-Clean() functions were not 
		//implemented when they were put in the global scheduling utils (back in 2000), 
		//but I am fixing the logic here rather than remove them in case someone decided to implement
		//that optimization in the future.
	if (bSaveSuccess) {
		if(!bNeedUpdate){
			// Check the active sheet's recordset's timestamp
			SchedulerClean();
		}

		// (c.haag 2006-12-06 08:41) - PLID 23666 - Now clear the template item ID so that
		// if we click on the newly created appointment, the scheduler will think it's an
		// appointment and not a template block
		m_Res.PutTemplateItemID(-1);
	}

	// (z.manning, PLID16278, 06/27/05)
	// Make sure that there are no message boxes that can possible pop up between this if statement
	//   and the call to AppointmentModify.
	// Let go of CReservation object
	if (bAutoDelete) {
		m_Res.SetDispatch(NULL);
	}

	// (d.moore 2007-10-16) - PLID 26546 - We need to check to see if the new appointment 
	//  might satisfy a request in the waiting list.
	// (c.haag 2010-05-26 15:19) - PLID 38652 - It is now safe to do this now that we've released the
	// reservation.
	if (m_NewRes && bSaveSuccess && m_nNewResID > 0 && !m_bMoveUp) {
		CheckAppointmentAgainstWaitingList(m_nNewResID, nPatientID, GetCurResourceIDList(), dtDate, m_dtStartTime);
	}

	//(e.lally 2005-5-31) PLID 16612 - if the status has been changed, make a log entry
//	long nCurStatus = GetNoShowComboSel(); // I needed this variable earlier, so it has moved up
	if(m_nAptInitStatus != -1 && m_nAptInitStatus != nCurStatus && m_nNewResID != -1){
		//create log
		LogShowStateTime(m_nNewResID, nCurStatus);
		//(e.lally 2008-06-20) PLID 29500 - if there are linked appointments, check to update those too.
			//We already updated no show appointments, so only update non-no shows
			//The interface only allows appointment linking if the appointment has already been saved, so we
			//can put this update inside this block with the Log Show State Time.
		if(nCurStatus != assNoShow){
			UpdateLinkedAppointmentShowState(m_nNewResID, nCurStatus, strShowStateName, NULL);
		}
	}

	if (bSaveSuccess)
	{
		// Check the patient provider; try to assign one based on
		// the appointment resource provider
		AppointmentCheckPatientProvider(nPatientID, m_nNewResID);
	}

	// (c.haag 2009-01-14 09:51) - PLID 32712 - When new surgeries are booked, try to add our purposes to the most 
	// recent consult. Do it when the user changes purposes, or the appointment is new.
	if (bSaveSuccess)
	{
		if (nPatientID != -25 && (!AreArrayContentsMatched(m_adwAptInitPurposes, m_adwPurposeID) || m_NewRes)) {
			SyncSurgeryPurposesWithConsult(nPatientID, dtDate, m_dtStartTime);
		}

		// (c.haag 2009-01-21 13:50) - PLID 32712 - Because the surgery may have changed, we need to check for creating
		// a case history now rather than earlier.
		if (m_NewRes) {
			// (b.spivey, February 03, 2012) - PLID 23874 - Rearranged if statement to check variables before calling functions. 
				if(nPatientID != -25 && m_PromptForCaseHistory && IsSurgeryCenter(false) && GetRemotePropertyInt("ApptPromptCaseHistory", 1, 0, "<None>", true)) {
					CheckCreateCaseHistory(nPatientID, dtDate, m_nNewResID);
				}
		}
	}

	// Tell the user if unsuccessful
	if (!bSaveSuccess) {
		MsgBox("The appointment could not be saved.");
		EndWaitCursor();
		return;
	}

	// If the appointment will not appear because of the
	// filters used in the scheduler, warn the user.
	{
		CNxSchedulerDlg *pSchedSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();
		if (pSchedSheet)
		{
			long lTypeID = pSchedSheet->GetPurposeSetId();
			long lPurpID = pSchedSheet->GetPurposeId();
			//TES 6/21/2010 - PLID 21341 - Also check against the Location filter.
			long lLocationID = pSchedSheet->GetLocationId();

			// (z.manning, 11/07/05, PLID 18249)
			// Revamped the code to check the scheduler view's type & purpose filters because
			// originally theyd did not handle the multiple type/purpose situation correctly.
			BOOL bTypeFiltered = FALSE;
			BOOL bPurposeFiltered = FALSE;
			BOOL bLocationFiltered = FALSE;
			if(lTypeID == -1) { // All types
				bTypeFiltered = TRUE;
			}
			else if(lTypeID == -2) { // Multiple types
				CDWordArray dwaryTypeIDs;
				pSchedSheet->GetTypeFilterIDs(&dwaryTypeIDs);
				for(int i = 0; i < dwaryTypeIDs.GetSize(); i++) {
					if(dwaryTypeIDs.GetAt(i) == (DWORD)nAptTypeID) {
						bTypeFiltered = TRUE;
						break;
					}
				}
			}
			else { // One type
				if(lTypeID == nAptTypeID) {
					bTypeFiltered = TRUE;
				}
			}

			if(lPurpID == -1) { // All purposes
				bPurposeFiltered = TRUE;
			}
			else if(lPurpID == -2) { // Multiple purposes
				CDWordArray dwaryPurposeIDs;
				pSchedSheet->GetPurposeFilterIDs(&dwaryPurposeIDs);
				// Check all of the purposes for this appt, and if one of them is one of the purposes
				// being filtered on, then we're ok
				for(int nPurposeCount = 0; nPurposeCount < m_adwPurposeID.GetSize(); nPurposeCount++) {
					for(int i = 0; i < dwaryPurposeIDs.GetSize(); i++) {
						if(dwaryPurposeIDs.GetAt(i) == m_adwPurposeID.GetAt(nPurposeCount)) {
							bPurposeFiltered = TRUE;
							break;
						}
					}
				}
			}
			else { // One purpose
				for(int nPurposeCount = 0; nPurposeCount < m_adwPurposeID.GetSize(); nPurposeCount++) {
					if((DWORD)lPurpID == m_adwPurposeID.GetAt(nPurposeCount)) {
						bPurposeFiltered = TRUE;
						break;
					}
				}
			}

			//TES 6/21/2010 - PLID 21341 - Check the location filter.
			if(lLocationID == -1) { // All locations
				bLocationFiltered = TRUE;
			}
			// (d.singleton 2012-07-02 16:49) - PLID 47473 multiple locations
			else if(lLocationID == -2) { 
				CDWordArray dwaryLocationIDs;
				pSchedSheet->GetLocationFilterIDs(&dwaryLocationIDs);
				for(int i = 0; i < dwaryLocationIDs.GetCount(); i++) {
					if((long)dwaryLocationIDs.GetAt(i) == nLocationID) {
						bLocationFiltered = TRUE;
						break;
					}
				}
			}
			else { // One location
				if(lLocationID == nLocationID) {
					bLocationFiltered = TRUE;
				}
			}

			if(!bTypeFiltered || !bPurposeFiltered || !bLocationFiltered)
			{
				if (IDYES == pSchedSheet->MessageBox(
					"The appointment was saved successfully but it is not being displayed because the Scheduler is currently filtered on a different type, purpose, or location.\r\n\r\n"
					"Would you like to clear the current Scheduler filters so that all types, purposes, and locations are visible?", NULL, 
					MB_YESNO|MB_ICONQUESTION))
				{
					pSchedSheet->m_dlTypeFilter->TrySetSelByColumn(0, (long)-1);
					pSchedSheet->m_dlPurposeFilter->TrySetSelByColumn(0, (long)-1);
					pSchedSheet->m_dlLocationFilter->TrySetSelByColumn(0, (long)-1);
					pSchedSheet->SetActivePurposeSetId(-1);
					pSchedSheet->SetActivePurposeId(-1);
					pSchedSheet->SetActiveLocationId(-1);
					bNeedsRefiltering = TRUE;
				}
			}
		}
	}

	// If the appointment will not appear because the user
	// changed the resource, warn the user
	{
		CNxSchedulerDlg *pSchedSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();

		// If this is a multi-resource sheet, make sure at least one resource is visible
		if (pSchedSheet == &m_pSchedulerView->m_MultiResourceSheet)
		{
			if (!m_pSchedulerView->AreAnyResourcesInCurView(GetCurResourceIDList()))
			{
				// Not visible, tell the user that the appointment he just saved won't be visible
				pSchedSheet->MessageBox(
					"The appointment was saved successfully but will not be displayed because it is not associated with any of the resources currently visible in the Scheduler.", 
					NULL, MB_OK|MB_ICONINFORMATION);
			} else {
				// It's one of the resources in the resource view, but that doesn't mean it's visible, because it could be scrolled off the screen
				// TODO: Figure out whether it's visible or not, and if not, tell the user that it isn't visible, and offer to scroll to the nearest visible resource
			}
		}
		// Otherwise, see if the selected resource is visible
		else {
			if (pSchedSheet->GetSafeHwnd()) {
				// See if THE active resource is one of the resources in this appointment
				if (FindElementInArray(GetCurResourceIDList(), m_pSchedulerView->GetActiveResourceID()) == feiarNotFound) {
					// Ok, well THE active resource isn't one of our resources, but maybe another one that's in the current view IS one of ours
					const CResourceEntry *pre = m_pSchedulerView->FindResourceInCurrentList(GetCurResourceIDList());
					if (pre) {
						// It was found, see if the user wants to change to that as the active resource
						if (IDYES == pSchedSheet->MessageBox(
							"The appointment was saved successfully but will not be displayed because it is not associated with the resource currently visible in the Scheduler.\r\n\r\n"
							"Would you like to switch to a resource that the appointment is associated with?", NULL, 
							MB_YESNO|MB_ICONQUESTION))
						{
							m_pSchedulerView->SetActiveResourceID(pre->m_nResourceID, TRUE);
						}
					} else {
						// It wasn't found, so we're done trying
						MsgBox("The appointment was saved successfully, but will not be displayed because it is not associated with the resource currently visible in the Scheduler.");
					}
				}
			}
		}
	}

	// (c.haag 2010-09-08 10:52) - PLID 37734 - If this is a new appointment, or an existing
	// appointment where the move-up flag changed to "Yes", then prompt the user with regards
	// to opening the waiting list
	if (m_bMoveUp && (m_NewRes || !m_bOldMoveUp)) {
		int nWaitingListAction = GetRemotePropertyInt("ApptMoveUpOpenWaitingList", 2, 0, GetCurrentUserName(), true);
		BOOL bOpenWaitingList = FALSE;
		switch (nWaitingListAction)
		{
		case 0: // Do not open the waiting list
			break;
		case 1: // Open the waiting list
			bOpenWaitingList = TRUE;
			break;
		case 2: // Prompt to open the waiting list
			if (IDYES == AfxMessageBox("Would you like to open the waiting list now?", MB_YESNO | MB_ICONQUESTION))
			{
				bOpenWaitingList = TRUE;
			}
			break;
		}
		if (bOpenWaitingList) {
			CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if (pView) {
				pView->PostMessage(WM_COMMAND, ID_SHOW_MOVEUP_LIST);
			}
		}
	}

	// Update the view if necessary
	// C.HAAG (3/26/2003) 3:44 PM - Table checkers should make this obselete
	/*if (bNeedUpdate && m_pSchedulerView) {
		CNxSchedulerDlg *p = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();
		if (p) p->m_bNeedUpdate = true;
		m_pSchedulerView->UpdateView();
	}*/

	if (bNeedsRefiltering)
	{
		CNxSchedulerDlg *p = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();
		if (p) p->m_bNeedUpdate = true;
		//TES 3/24/2004: PLID 11593 - Let's just post the message, there may be messages pending that would be messed up 
		//if we clear the singleday.
		m_pSchedulerView->PostMessage(NXM_UPDATEVIEW);
	} 
	// (c.haag 2006-12-06 09:42) - PLID 23666 - Now that we support template blocks, and the appearance of those template
	// blocks is a function of how appointments appear, we must inevitably refresh the schedule to ensure the blocks
	// show up properly
	else {
		m_pSchedulerView->PostMessage(NXM_UPDATEVIEW);
	}

	EndWaitCursor();
}

void CResEntryDlg::OnChangeNotesBox() 
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		if (m_Res) {
			// (j.luckoski 2012-06-20 11:24) - PLID 11597 - If cancelled let the PutText know to handle it as such.
				m_Res.PutText((_bstr_t)GetActiveText(m_bIsCancelled));
		}
		SetModifiedFlag();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelChosenNoShowCombo(long nRow)
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		if (nRow < 0)
		{
			m_dlNoShow->CurSel = 0;
			nRow = 0;
		}
		if (m_Res) {
			// (j.luckoski 2012-06-20 11:24) - PLID 11597 - If cancelled let the PutText know to handle it as such.
				m_Res.PutText((_bstr_t)GetActiveText(m_bIsCancelled));
		}
		m_ActiveStatusColor = VarLong(m_dlNoShow->Value[nRow][2]);
		ApplyToResBox();
		SetModifiedFlag();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

OLE_COLOR CResEntryDlg::GetActiveBorderColor()
{
	return m_ActiveBorderColor;
}

OLE_COLOR CResEntryDlg::GetActiveStatusColor()
{
	return m_ActiveStatusColor;
}

// (j.luckoski 2012-06-26 12:23) - PLID 11597 - Obtain the active status color so we can update the Handle Restore Function
OLE_COLOR CResEntryDlg::ObtainActiveStatusColor()
{
	// Get the color simply based on the appointment type
	try {
		long nStatusID = GetCurStatusID();
		if (nStatusID != -1) {
			// (c.haag 2006-05-09 11:07) - PLID 20493 - Try to get the color
			// from the dropdown first
			long nRow = m_dlNoShow->FindByColumn(0, nStatusID, 0, false);
			if (nRow != -1) {
				m_ActiveStatusColor = VarLong(m_dlNoShow->GetValue(nRow, 2), 0);
				return m_ActiveStatusColor;
			} else {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT Color FROM AptShowStateT WHERE ID = {INT}", 
					nStatusID);
				if (!prs->eof) {
					// Got the record we needed
					m_ActiveStatusColor = AdoFldLong(prs, "Color");
					return m_ActiveStatusColor;
				}
			}
		} else {
			// This appointment has no type, and therefore no border color
			m_ActiveStatusColor = 0;
			return m_ActiveStatusColor;
		}
	} CResEntryDlg_NxCatchAll("CResEntryDlg::ObtainActiveStatusColor");


	// If we made it here, we failed
	ASSERT(FALSE);
	m_ActiveStatusColor = 0;
	return m_ActiveStatusColor;
}


OLE_COLOR CResEntryDlg::ObtainActiveBorderColor()
{
	// Get the color simply based on the appointment type
	try {
		long nAptTypeID = GetCurAptTypeID();
		if (nAptTypeID != -1) {
			// (c.haag 2006-05-09 11:07) - PLID 20493 - Try to get the color
			// from the dropdown first
			long nRow = m_dlAptType->FindByColumn(0, nAptTypeID, 0, false);
			if (nRow != -1) {
				m_ActiveBorderColor = VarLong(m_dlAptType->GetValue(nRow, 5), 0);
				return m_ActiveBorderColor;
			} else {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT Color FROM AptTypeT WHERE ID = {INT}", 
					nAptTypeID);
				if (!prs->eof) {
					// Got the record we needed
					m_ActiveBorderColor = AdoFldLong(prs, "Color");
					return m_ActiveBorderColor;
				}
			}
		} else {
			// This appointment has no type, and therefore no border color
			m_ActiveBorderColor = 0;
			return m_ActiveBorderColor;
		}
	} CResEntryDlg_NxCatchAll("CResEntryDlg::ObtainActiveBorderColor");


	// If we made it here, we failed
	ASSERT(FALSE);
	m_ActiveBorderColor = 0;
	return m_ActiveBorderColor;
}

void CResEntryDlg::Activate(bool toBeActive /* = TRUE */)
{
	m_ExtraActive = toBeActive;
}

void CResEntryDlg::ApplyToResBox()
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		if (m_Res) {
			// (j.luckoski 2012-06-20 11:24) - PLID 11597 - If cancelled let the PutText know to handle it as such.
				m_Res.PutText((_bstr_t)GetActiveText(m_bIsCancelled));
			// (j.luckoski 2012-06-26 12:23) - PLID 11597 - If cancelled make the border and status color the cancelled color.				
			if(!m_bIsCancelled) {
				// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
				// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
				COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
				if (GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true)) {
					//use the status color
					// (j.jones 2014-12-04 16:10) - PLID 64119 - an open appt. will intentionally not apply the mix rule color
					clrBkg = ObtainActiveStatusColor();
				} else {
					//use the border color
					clrBkg = ObtainActiveBorderColor();
				}
				ColorReservationBackground(m_Res, clrBkg);
				m_Res.PutBorderColor(ObtainActiveBorderColor());
			}
			else {
				//ignore the color preference for cancelled appts.
				ColorReservationBackground(m_Res, m_nCancelColor);
				m_Res.PutBorderColor(m_nCancelColor);
			}
		}
	} CResEntryDlg_NxCatchAllThrow("Error in CResEntryDlg::ApplyToResBox"); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// (j.luckoski 2012-06-26 12:24) - PLID 11597 - Revert active should manage the old text with the correct colors if cancelled.
void CResEntryDlg::RevertActive()
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		if (m_Res) {
			m_Res.PutText(_bstr_t(m_OldText));
			if (m_bIsCancelled) {
				ColorReservationBackground(m_Res, m_nCancelColor);
				m_Res.PutBorderColor(m_nCancelColor);
			}
			else {
				COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
				// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
				// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
				if (GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true)) {
					// (j.jones 2014-12-04 16:10) - PLID 64119 - Use the mix rule color if one exists and
					// the appt. was pending, otherwise use the old status color.
					// Remember this is reverting any changes so all calculations must be off the initial appt. values.
					long nMixRuleColor = -1;

					if (m_nAptInitStatus == 0) {	//pending
						InsuranceInfo *pOrigInsInfo = NULL;
						m_mapOrigIns.Lookup(priIns, pOrigInsInfo);
						long nPrimaryInsuredPartyID = -1;
						if (pOrigInsInfo) {
							nPrimaryInsuredPartyID = pOrigInsInfo->nInsuredPartyID;
						}
						nMixRuleColor = GetAppointmentMixRuleColor(AsDateNoTime(m_dtAptInitStartTime), m_nInitLocationID, nPrimaryInsuredPartyID, m_nAptInitType, m_adwAptInitResources, m_adwAptInitPurposes);
					}
					
					if (nMixRuleColor != -1) {
						clrBkg = nMixRuleColor;
					}
					else {
						clrBkg = m_OldStatusColor;
					}
				}
				else {
					clrBkg = m_OldBorderColor;
				}
				ColorReservationBackground(m_Res, clrBkg);
				m_Res.PutBorderColor(m_OldBorderColor);
			}
		}
	} CResEntryDlg_NxCatchAllThrow("Error in CResEntryDlg::RevertActive"); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// (j.luckoski 2012-06-26 12:24) - PLID 11597 - Function handles the cancelled and restored reservatins to make it seems quick and responsive.
void CResEntryDlg::HandleCancelRestore()
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		if (m_Res) {

			// (j.luckoski 2012-06-14 14:18) - PLID 11597 - If not cancelled remove from text
			if(!m_bIsCancelled) {
				// Set the appropriate text for the appointment
				m_Res.PutText(_bstr_t(GetActiveText(false)));
				// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Return putallowdrag back to true
				m_Res.PutAllowDrag(TRUE);
				m_Res.PutAllowResize(TRUE);

				COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
				// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
				// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
				if (GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true)) {
					// (j.jones 2014-12-04 16:10) - PLID 64119 - Use the mix rule color if one exists and
					// the appt. is pending, otherwise use the status color.
					// This is going off of the current appointment values.
					long nMixRuleColor = -1;
					if (GetNoShowComboSel() == 0) {	//pending
						InsuranceInfo *pCurInsInfo = NULL;
						m_mapInsPlacements.Lookup(priIns, pCurInsInfo);
						long nPrimaryInsuredPartyID = -1;
						if (pCurInsInfo) {
							nPrimaryInsuredPartyID = pCurInsInfo->nInsuredPartyID;
						}
						nMixRuleColor = GetAppointmentMixRuleColor(AsDateNoTime(GetDate()), GetCurLocationID(), nPrimaryInsuredPartyID, GetCurAptTypeID(), m_arydwSelResourceIDs, m_adwPurposeID);
					}

					if (nMixRuleColor != -1) {
						clrBkg = nMixRuleColor;
					}
					else {
						clrBkg = ObtainActiveStatusColor();
					}
				} else {
					clrBkg = ObtainActiveBorderColor();
				}
				ColorReservationBackground(m_Res, clrBkg);
				m_Res.PutBorderColor(ObtainActiveBorderColor());
			} else {
				m_Res.PutText(_bstr_t(GetActiveText(true)));
				// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Prevent them from moving or resizing cancelled appts
				m_Res.PutAllowDrag(VARIANT_FALSE);
				m_Res.PutAllowResize(VARIANT_FALSE);
				ColorReservationBackground(m_Res, m_nCancelColor);
				m_Res.PutBorderColor(m_nCancelColor);
			}
			// (j.luckoski 2012-07-02 15:58) - PLID 11597 - Make sure the view updates as soon as you handle these in order to have quick response.
			Dismiss();
			m_pSchedulerView->PostMessage(NXM_UPDATEVIEW);
		}
		// (c.haag 2015-11-12) - PLID 67577 - Use the correct function name
	} CResEntryDlg_NxCatchAllThrow("Error in CResEntryDlg::HandleCancelRestore"); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CNxDialog::OnKeyUp(nChar, nRepCnt, nFlags);
}

// If you change any logic here, make sure to change it also in CSet::AddTextItem
// (j.luckoski 2012-06-20 11:28) - PLID 11597 - Add bool to show cancelled and to handle the pCancel as CANCELLED or ""
// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
int CResEntryDlg::AddTextItem(CString & strFormat, CString & strAddTo, int iPos, bool bIsCancelled, bool bIsMixRuleOverridden)
{
	if (strFormat.GetLength() <= iPos) return 0; 
	if (strFormat[iPos] != '[') return 0;

	int iLen;
	iPos = iPos + 1;
	iLen = strFormat.Mid(iPos).Find(']');
	
	CString strItemName = strFormat.Mid(iPos, iLen);
	CString strItem;
	
	// Return whatever the user requests
	// If the request is unknown, return the request
	int cnt = 0;
	if (strItemName.Left(7) == "pCancel") {
		if (bIsCancelled) {
			strItem = _T("CANCELLED:");
		} else {
			strItem = _T("");
		}
		cnt = 7;
	}
	// (j.jones 2014-12-03 10:40) - PLID 64274 - if the appt. has overridden a mix rule, show the word OVERRIDE:
	else if (strItemName.Left(9) == "pOverride") {
		if (bIsMixRuleOverridden) {
			strItem = _T("OVERRIDE:");
		}
		else {
			strItem = _T("");
		}
		cnt = 9;
	}
	else if (strItemName.Left(5) == "pName") {
		// (a.walling 2010-12-30 11:13) - PLID 40081 - Use datalist2 to avoid row index race conditions
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_dlAptPatient->CurSel;
		if (pCurSel) {
			// (a.walling 2010-05-27 15:35) - PLID 38917 - No more Name column
			strItem.Format("%s, %s %s", VarString(pCurSel->GetValue(plLast), ""), VarString(pCurSel->GetValue(plFirst), ""), VarString(pCurSel->GetValue(plMiddle), ""));
			strItem.TrimRight();
		} else {
			strItem = _T("");
		}
		cnt = 5;
	} else if (strItemName.Left(8) == "rPurpose") {
		long nAptPurposeIndex = m_dlAptPurpose->CurSel;
		if (nAptPurposeIndex != -1) {
			strItem = VarString(m_dlAptPurpose->Value[nAptPurposeIndex][1]);
		} else {
			strItem = _T("");
		}
		cnt = 8;
	} else if (strItemName.Left(6) == "rNotes") {
		GetDlgItem(IDC_NOTES_BOX)->GetWindowText(strItem);
		cnt = 6;
	} else if (strItemName.Left(16) == "rMoveUpConfirmed") {
		if (m_bMoveUp) {
			strItem = "m";
		}
		if (m_nConfirmed == 1) {
			strItem += "c";
		}
		else if (m_nConfirmed == 2) {
			strItem += "l";
		}
		if (m_bReady) {
			strItem += "r";
		}
		//strItem = (m_MoveUpVal == 0) ? "m" : CString("");
		//strItem += (m_ConfirmedVal == 0) ? "c" : CString("");
		cnt = 16;
	} 
	else if (strItemName.Left(10) == "rShowState") {
		/*switch (GetNoShowComboSel()) {
		case 0: // Pending appointment
			strItem = "p";
			break;
		case 1: // In
			strItem = "i";
			break;
		case 2: // Out
			strItem = "o";
			break;
		case 3: // No Show
			strItem = "n";
			break;
		case 4: // Received
			strItem = "x";
			break;
		}*/
		if (m_dlNoShow->CurSel != -1)
			strItem = VarString(m_dlNoShow->Value[m_dlNoShow->CurSel][1]).Left(1);
		else
			strItem.Empty();
		cnt = 10;
	}
	else if (strItemName.Left(9) == "rPurpType") {
		strItem = _T(GetCurAptTypeName());
		cnt = 9;
	} else if (strItemName.Left(13) == "rLocationName") {
		strItem = GetCurLocationName();
		cnt = 13;
	} else if (strItemName.Left(13) == "rResourceName") {
		strItem = GetResourceString();
		cnt = 13;
	} else if (strItemName.Left(9) == "ExtraInfo") {
		long tmpExtraOrd = atol(strItemName.Mid(9));
		strItem = GetExtraInfo(tmpExtraOrd);
		cnt = 10;
	} else {
		strItem = strItemName;
		cnt = strItemName.GetLength();
	}

	// If we are returning any text, concatenate bonus 
	// text (ie, everything else remaining before the ']'
	strItem.TrimLeft(); strItem.TrimRight();
	strItem.Replace("%", "%%");
	if (strItem.GetLength()) {
		strItem += strItemName.Mid(cnt);
	}

	strAddTo += strItem;

	return iLen+2;
}

void CResEntryDlg::OnExtraInfoBtn() 
{
	try {
		ShowExtraInfo(!m_bExtraInfo);
		// If we are now showing extra info, ensure that those fields have values
		if(m_bExtraInfo) {
			ApplyToExtraInfo();
		}

		// (j.jones 2010-09-03 11:45) - PLID 40397 - save the user's last extra info setting
		SetRemotePropertyInt("ResEntry_RememberShowExtraInfo", m_bExtraInfo ? 1 : 0, 0, GetCurrentUserName());

		// (b.savon 2012-06-04 16:41) - PLID 49860 - Recenter after toggling More Info.
		CenterDialog();

	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::ShowExtraInfo(bool bShow)
{
	CRect tmpRect;
	GetWindowRect(tmpRect);

	// Adjust the RIGHT side of that rect to include or exclude the "more info" section
	{
		CRect rcOffsetLabel;
		GetDlgItem(IDC_EXTRA_INFO_OFFSET_LABEL)->GetWindowRect(&rcOffsetLabel);
		if (bShow) {
			tmpRect.right = rcOffsetLabel.right;
			SetDlgItemText(IDC_EXTRA_INFO_BTN, "<< Less &Info");
			m_bExtraInfo = true;
		} else {
			tmpRect.right = rcOffsetLabel.left;
			SetDlgItemText(IDC_EXTRA_INFO_BTN, "More &Info >>");
			m_bExtraInfo = false;
		}
	}

	
	// Make the more info accessible or inaccessible as appropriate
	{
		int nCmdShow = bShow ? SW_SHOW : SW_HIDE;

		GetDlgItem(IDC_EXTRA_FIELDS1)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_PATIENT_ID_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_PATIENT_ID_BOX)->ShowWindow(nCmdShow);

		GetDlgItem(IDC_EXTRA_FIELDS2)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_HOME_PHONE_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_PATIENT_HOME_PHONE_BOX)->ShowWindow(nCmdShow);

		GetDlgItem(IDC_EXTRA_FIELDS3)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_WORK_PHONE_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_PATIENT_WORK_PHONE_BOX)->ShowWindow(nCmdShow);

		GetDlgItem(IDC_EXTRA_FIELDS4)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_BIRTH_DATE_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_PATIENT_BIRTH_DATE_BOX)->ShowWindow(nCmdShow);

		GetDlgItem(IDC_EXTRA_FIELDS5)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_CELL_PHONE_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_PATIENT_CELL_PHONE_BOX)->ShowWindow(nCmdShow);

		GetDlgItem(IDC_EXTRA_FIELDS6)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_PATBAL_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_PTBAL_BOX)->ShowWindow(nCmdShow);

		GetDlgItem(IDC_EXTRA_FIELDS7)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_INSBAL_CHECK)->ShowWindow(nCmdShow);
		GetDlgItem(IDC_RES_INSBAL_BOX)->ShowWindow(nCmdShow);
	}

	MoveWindow(tmpRect);
}

_RecordsetPtr CResEntryDlg::CreatePatientInfoRecordset()
{
	// (d.moore 2007-06-05 17:16) - PLID 13550 - Added insurance fields (Primary and Secondary).
	//DRT 9/10/2008 - PLID 31127 - Added check in and check out times
	//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
	// (c.haag 2008-11-20 16:19) - PLID 31128 - Added InsuranceReferralSummary
	// (z.manning 2009-06-30 16:59) - PLID 34744 - Only load ins ref summary if necessary
	// (j.jones 2009-09-17 10:33) - PLID 35572 - ensure we do not warn about inactive insurances
	// (a.walling 2010-07-01 16:38) - PLID 18081 - Warning categories
	// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party,
	// but we can detect if they even have copays in this query
	// (a.walling 2010-11-01 00:01) - PLID 40965 - Parameterized, and removed open dynamic etc since we simply read this!
	//return CreateRecordset(adOpenDynamic, adLockOptimistic,
	// (j.armen 2011-07-01 15:42) - PLID 44205 - Added record for confirmed by field
	// (j.luckoski 2012-05-02 16:25) - PLID 11597 - Add CancelledDate for the extra fields	
	// (j.gruber 2012-08-01 10:36) - PLID 51885 - load the patient information will need for insurances	
	// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
	// (j.luckoski 2013-03-04 13:50) - PLID 33548 - Added DisplayRewardsWarning
	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
	// (s.tullis 2014-01-24 10:54) - PLID 49748 - Add nexweb activation code to the  extra info fields
	// (s.tullis 2014-01-24 15:20) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
	return CreateParamRecordset(GetRemoteDataSnapshot(), FormatString(		
		"DECLARE @PatientID INT;"
		"DECLARE @AppointmentID INT;"
		"SET @PatientID = {INT};"
		"SET @AppointmentID = {INT};"
		"SELECT UserDefinedID, PersonT.DisplayWarning, PersonT.WarningMessage, PersonT.WarningDate, PersonT.WarningExpireDate, "
		"PersonT.WarningUseExpireDate, WarningCategoriesT.Color AS WarningColor, WarningCategoriesT.ID AS WarningID, WarningCategoriesT.Name AS WarningName, UserName, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, "
		"PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Title AS PatTitle, "
		"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Country, "
		"PersonT.PrivHome, PersonT.PrivWork, PersonT.PrivCell, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Extension, "
		"PersonT.CellPhone, PreferredContact, PersonT.BirthDate, PersonT.SocialSecurity, PersonT.Gender, PersonT.Company, "
		"PersonT.Email, PersonT.OtherPhone, PersonT.Pager, PersonT.PrivEmail, PersonT.PrivOther, PersonT.PrivPager, "
		"ReferralSourceT.Name AS RefSourceName, Custom1Q.TextParam AS Custom1, Custom2Q.TextParam AS Custom2, Custom3Q.TextParam "
		"AS Custom3, Custom4Q.TextParam AS Custom4, "
		"CASE WHEN PersonT.ID IN (SELECT PatientID FROM InsuredPartyT "
		"	INNER JOIN InsuredPartyPayGroupsT ON InsuredPartyT.PersonID = InsuredPartyPayGroupsT.InsuredPartyID "
		"	WHERE (CoPayMoney Is Not Null OR CopayPercentage Is Not Null) AND RespTypeID <> -1) THEN 1 ELSE 0 END AS HasCoPay, "
		"PrimaryInsurance.Name AS PrimaryIns, PrimaryInsurance.CoPay AS PriInsCoPay, "
		"PrimaryInsurance.CopayPercent AS PriInsCoPayPer, "
		"SecondaryInsurance.Name AS SecondaryIns, SecondaryInsurance.CoPay AS SecInsCoPay, "
		"SecondaryInsurance.CopayPercent AS SecInsCoPayPer,  "
		"PersonT.DisplayAllergyWarning, "
		"(SELECT MAX([TimeStamp]) FROM AptShowStateHistoryT WHERE AppointmentID = @AppointmentID AND ShowStateID = 1) AS CheckInTime, "
		"(SELECT MAX([TimeStamp]) FROM AptShowStateHistoryT WHERE AppointmentID = @AppointmentID AND ShowStateID = 2) AS CheckOutTime, "
		"CASE WHEN RefPersonT.ID Is Null THEN '' ELSE RefPersonT.Last + ', ' + RefPersonT.First + ' ' + RefPersonT.Middle END AS RefPhysName, "
		"RefPersonT.WorkPhone AS RefPhysWorkPhone, RefPersonT.Fax AS RefPhysFax, %s AS InsuranceReferralSummary, "
		" CONVERT(DateTime, (SELECT CancelledDate FROM AppointmentsT WHERE AppointmentsT.ID = @AppointmentID)) AS CancelledDate, " 
		" (SELECT ConfirmedBy FROM AppointmentsT WHERE AppointmentsT.ID = @AppointmentID) AS ConfirmedBy, "
		"PersonT.DisplayRewardsWarning, "
		"PatientsT.SecurityCode, "
		"PatientsT.SecurityCodeCreationDate, "
		"(SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebSecurityCodeExpire'), -1)) AS SecurityCodeExpDate "
		"FROM PersonT LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"LEFT JOIN WarningCategoriesT ON PersonT.WarningCategoryID = WarningCategoriesT.ID "
		"LEFT JOIN UsersT ON UsersT.PersonID = PersonT.WarningUserID "
		"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "		
		"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 1) AS Custom1Q ON PersonT.ID = Custom1Q.PersonID "
		"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 2) AS Custom2Q ON PersonT.ID = Custom2Q.PersonID "
		"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 3) AS Custom3Q ON PersonT.ID = Custom3Q.PersonID "
		"LEFT JOIN (SELECT PersonID, TextParam FROM CustomFieldDataT WHERE FieldID = 4) AS Custom4Q ON PersonT.ID = Custom4Q.PersonID "
		"LEFT JOIN (SELECT InsuredPartyT.PatientID, InsuranceCoT.Name, CoPay, CopayPercent "
			"FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID WHERE RespTypeID = 1) InsuredPartyT LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE InsuredPartyT.RespTypeID = 1) AS PrimaryInsurance ON PersonT.ID = PrimaryInsurance.PatientID "
		"LEFT JOIN (SELECT InsuredPartyT.PatientID, InsuranceCoT.Name, CoPay, CopayPercent "
			"FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID WHERE RespTypeID = 2)InsuredPartyT LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE InsuredPartyT.RespTypeID = 2) AS SecondaryInsurance ON PersonT.ID = SecondaryInsurance.PatientID "
		"LEFT JOIN PersonT RefPersonT ON PatientsT.DefaultReferringPhyID = RefPersonT.ID "
		
		
		"WHERE PersonT.ID = @PatientID"
		, m_pSchedulerView == NULL ? "''" : m_pSchedulerView->GetInsReferralSummarySql("@PatientID")
		)
		, GetCurPatientID()
		, m_ResID
		);
}

///* New way not using CPatientSet (8/27/1999 bob)
void CResEntryDlg::ApplyToExtraInfo(_RecordsetPtr prsPatient /* = NULL */)
{
	if (!m_bIsExtraInfoDirty && !m_CheckboxesUpdated) return;

	// This function fills in all the appropriate info in the 'extra info' boxes
	// based on what the current patient is
	long nPatientID = GetCurPatientID();

	try {
		// Try to open just the record we need from the patient set
		_RecordsetPtr prs;
		FieldsPtr flds = NULL;
		if (nPatientID != -25)
		{
			if (NULL == prsPatient) {
				prs = CreatePatientInfoRecordset();
			} else {
				prs = prsPatient;
			}

			// Proceed only if we are on a good record
			if (!prs->eof) {
				flds = prs->Fields;
			}
		}

		// (z.manning, 09/12/05, PLID 16454)
		// Only bother with these calculations if we are showing extra info, and the individual item is checked.
		// (c.haag 2006-05-08 16:55) - PLID 20475 - I moved the check to the start of the function
		//if(m_bExtraInfo) {
			// Set the fields in the res entry dialog
			long nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel();
			BOOL bCheck = m_pSchedulerView->m_bExtraInfo[0];
			if(nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PATIENT_ID_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PATIENT_ID_BOX, "");
			}
			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS2))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[1];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PATIENT_HOME_PHONE_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PATIENT_HOME_PHONE_BOX, "");
			}
			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS3))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[2];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PATIENT_WORK_PHONE_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PATIENT_WORK_PHONE_BOX, "");
			}
			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS4))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[3];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PATIENT_BIRTH_DATE_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PATIENT_BIRTH_DATE_BOX, "");
			}
			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS5))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[4];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PATIENT_CELL_PHONE_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PATIENT_CELL_PHONE_BOX, "");
			}
			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS6))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[5];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PATIENT_PREFERRED_CONTACT_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PATIENT_PREFERRED_CONTACT_BOX, "");
			}

			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS7))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[6];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_PTBAL_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_PTBAL_BOX, "");
			}
			nField = ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS8))->GetCurSel();
			bCheck = m_pSchedulerView->m_bExtraInfo[7];
			if (nField != -1 && bCheck) {
				SetDlgItemText(IDC_RES_INSBAL_BOX, GetExtraFieldText(flds, (ExtraFields)nField));
			}
			else {
				SetDlgItemText(IDC_RES_INSBAL_BOX, "");
			}
			m_bIsExtraInfoDirty = FALSE;
		//}
	} CResEntryDlg_NxCatchAll("CResEntryDlg::ApplyToExtraInfo");			
}
//*/

void CResEntryDlg::OnPatientIdCheck() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_PATIENT_ID_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[0] = tmpCheck;
		ApplyToExtraInfo();
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnHomePhoneCheck() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_HOME_PHONE_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[1] = tmpCheck;
		ApplyToExtraInfo();
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 1);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnWorkPhoneCheck() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_WORK_PHONE_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[2] = tmpCheck;
		ApplyToExtraInfo();

		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 2);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnBirthDateCheck() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_BIRTH_DATE_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[3] = tmpCheck;
		ApplyToExtraInfo();

		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 3);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnCellPhoneCheck()
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_CELL_PHONE_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[4] = tmpCheck;
		ApplyToExtraInfo();

		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 4);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnPreferredContactCheck()
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_PREFERRED_CONTACT_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[5] = tmpCheck;
		ApplyToExtraInfo();

		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 5);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnPatBalCheck()
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_PATBAL_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[6] = tmpCheck;
		ApplyToExtraInfo();

		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 6);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnInsBalCheck()
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_INSBAL_CHECK, tmpCheck);
		m_pSchedulerView->m_bExtraInfo[7] = tmpCheck;
		ApplyToExtraInfo();

		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 7);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// Get the proper patient ID
long CResEntryDlg::GetCurPatientID()
{
	if (m_bIsCurPatientSet) {
		
		// See if we have a datalist to compare our value to
		if (m_dlAptPatient != NULL) {
			// Make sure the location selection is valid
			// (a.walling 2010-12-30 11:14) - PLID 40081 - Use datalist2 to avoid row index race conditions
			NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_dlAptPatient->GetCurSel();
			if (!(m_dlAptPatient->IsRequerying() ||
				// PLID 16033 the datalist is dropped down
				m_dlAptPatient->GetDropDownState() ||
				// No row selected because it's still waiting for the requery to finish, OR...
				(m_dlAptPatient->GetIsTrySetSelPending() && pCurSel == NULL) || 
				// ...a row IS selected and it matches up with what we're about to return
				// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
				(pCurSel && VarLong(pCurSel->GetValue(plID)) == m_nCurPatientID)))
			{
				// One of the above assertions failed
				ASSERT(FALSE);
				ThrowNxException("The current selection in the patients dropdown does not match the stored value (patset = %s, patid = %li)!", 
					m_bIsCurPatientSet ? "true" : "false", m_nCurPatientID);
			}
		}

		// We now are sure our value is correct because if the datalist exists, the selection in it matches our value
		return m_nCurPatientID;
	} else {
		// It has to be set before it can be read
		ThrowNxException("GetCurPatientID was called before the current patient was set!");
	}
}

void CResEntryDlg::SetCurPatient(long nNewPatientID)
{
	// Set these immediately in case anyone asks between now and when the 
	// datalist actually gets set (which could be a while if TrySetSel 
	// returns sriNoRowYet_WillFireEvent) 
	m_nCurPatientID = nNewPatientID;
	m_bIsCurPatientSet = TRUE;

	// Try to set the selection, 
	// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
	long nSelResult = m_dlAptPatient->TrySetSelByColumn_Deprecated(plID, m_nCurPatientID);
	if (nSelResult == sriNoRowYet_WillFireEvent) {
		// The selection wasn't set yet, but the TrySetSelFinished event will be fired and we'll handle things then
		Log("Waiting on TrySetSelFinished to set the patient.");
	} else if (nSelResult == sriNoRow) {

		// Try to correct the problem. For example, we may have an inactive patient selected where the
		// user only wants to see active patients.
		if (TryCorrectingPatientSelResultFailure())
		{
			ReflectCurPatient();
			return;
		}

		// If the patients combo is not filtered, there should be no way the trysetsel would have failed.
		// This could happen if the user is logged in without using NxServer and someone adds a patient and an 
		// appointment on another machine, then this user opens that appointment without requerying the datalist.
		ReflectCurPatientNotFound();
	} else {
		// We have a valid row index that the datalist is now set to, so get the info out of that row
		//ASSERT(nSelResult == m_dlAptPatient->GetCurSel());
		ReflectCurPatient();
	}
}

void CResEntryDlg::ReflectCurPatient()
{
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_dlAptPatient->GetCurSel();
	if (!pCurSel) {
		return;
	}
	// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
	m_nCurPatientID = VarLong(pCurSel->GetValue(plID));
	m_bIsCurPatientSet = TRUE;
}

void CResEntryDlg::ReflectCurPatientNotFound()
{
	// This shouldn't happen because it can only mean that someone has added a patient and made an 
	// appointment for it, yet this workstation wasn't notified (it means the network code must 
	// have failed in some regard).  We will notify the user and try to pull the new patient from 
	// the database, and add it to the patient list, and if that fails, we'll just have to throw 
	// an exception;  I would like to throw an exception except for two facts: 1. We should be 
	// able to query the database and get this patient's info, and 2. The exception handling would 
	// cause a return value of FALSE, and ZoomRes doesn't handle this well at all.  I'm putting a 
	// TODO in there to fix that.

	//DRT 10/15/2004 - If this isn't supposed to happen, let's log if it does.
	Log("Reached ReflectCurPatientNotFound.  Current patient = %li", m_nCurPatientID);

	// Force the datalist to have no selection
	m_dlAptPatient->PutCurSel(NULL); // (a.walling 2010-12-30 11:18) - PLID 40081 - Use datalist2 to avoid row index race conditions
	// We must have a current patient already (you can't call this function unless the resentry has already loaded the cur patient id)
	ASSERT(m_bIsCurPatientSet);
	// Load the patient from the data
	// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
	//long nPatientNewSelIndex = UpdatePatient(m_nCurPatientID);
	// (a.walling 2010-12-30 11:22) - PLID 40081 - Use datalist2 to avoid row index race conditions
	NXDATALIST2Lib::IRowSettingsPtr pPatientNewSelRow = UpdatePatient(m_nCurPatientID);
	// The patient has to exist
	ASSERT(pPatientNewSelRow);
	if (pPatientNewSelRow) {
		// Good, the patient was added.  Select it.
		m_dlAptPatient->PutCurSel(pPatientNewSelRow);
	} else {
		// The location doesn't exist in data so this is a real problem
		ThrowNxException("ReflectCurPatientNotFound could not load patient.  Internal ID %li.", m_nCurPatientID);
	}
}

void CResEntryDlg::ReflectCurPatientFailure()
{
	// Clear the member variables
	m_bIsCurPatientSet = FALSE;
	m_nCurPatientID = -1;
	
	// Set the datalist to no selection
	m_dlAptPatient->PutCurSel(NULL); // (a.walling 2010-12-30 11:23) - PLID 40081 - Use datalist2 to avoid row index race conditions
}

long CResEntryDlg::GetCurAptPurposeID()
{
	// Get the appropriate AptPurposeID
	long nAptPurposeIndex = m_dlAptPurpose->CurSel;
	if (nAptPurposeIndex != -1) {
		return VarLong(m_dlAptPurpose->Value[nAptPurposeIndex][0], -1);
	} else {
		return -1;
	}
}

// Get the appropriate AptTypeID
long CResEntryDlg::GetCurAptTypeID()
{
	long nAptTypeIndex = m_dlAptType->CurSel;

	// (j.gruber 2014-06-06 13:30) - PLID 51324 - check >= 0
	if (nAptTypeIndex >= 0) {
		return VarLong(m_dlAptType->Value[nAptTypeIndex][0], -1);
	} else {
		//Do we have a valid inactive row showing?
		if(nAptTypeIndex == -1 && m_bValidAptTypeStored && m_dlAptType->GetIsComboBoxTextInUse() && 
			CString((LPCTSTR)m_dlAptType->GetComboBoxText()) == m_strAptTypeName ) {
			return m_nAptTypeID;
		}
		else {
			return -1;
		}
	}
}

// (j.luckoski 2012-06-26 12:25) - PLID 11597 - Return the statusID from the table
long CResEntryDlg::GetCurStatusID()
{
	long nStatusID = -1;
	nStatusID = m_dlNoShow->CurSel;
	if (nStatusID != -1) {
		return VarLong(m_dlNoShow->Value[nStatusID][0], -1);
	} else {
		return -1;
	}
}

// Get the appropriate type name
CString CResEntryDlg::GetCurAptTypeName()
{
	long nAptTypeIndex = m_dlAptType->CurSel;
	// (j.gruber 2014-06-06 13:30) - PLID 51324 - check >= 0 and send "" for the no type selection
	if (nAptTypeIndex >= 0) {
		CString strTypeName = VarString(m_dlAptType->Value[nAptTypeIndex][1], "");
		if (strTypeName == SCHEDULER_TEXT_APPOINTMENT_TYPE__NO_TYPE) {
			strTypeName = "";
		}	
		return strTypeName;
	} else {
		//Do we have a valid inactive row showing?
		if(nAptTypeIndex == -1 && m_bValidAptTypeStored && m_dlAptType->GetIsComboBoxTextInUse() && 
			CString((LPCTSTR)m_dlAptType->GetComboBoxText()) == m_strAptTypeName ) {
			return m_strAptTypeName;
		}
		else {
			return "";
		}
	}
}

//Get the appropriate category for our current type.
short CResEntryDlg::GetCurAptTypeCategory()
{
	long nAptTypeIndex = m_dlAptType->CurSel;
	// (j.gruber 2014-06-06 13:30) - PLID 51324 - check >= 0
	if (nAptTypeIndex >= 0) {
		return VarByte(m_dlAptType->Value[nAptTypeIndex][2], -1);
	} else {
		//Do we have a valid inactive row showing?
		if(nAptTypeIndex == -1 && m_bValidAptTypeStored && m_dlAptType->GetIsComboBoxTextInUse() && 
			CString((LPCTSTR)m_dlAptType->GetComboBoxText()) == m_strAptTypeName ) {
			return m_nAptTypeCategory;
		}
		else {
			return -1;
		}
	}
}

//Get the appropriate duration for our current type.
long CResEntryDlg::GetCurAptTypeDuration()
{
	long nAptTypeIndex = m_dlAptType->CurSel;
	// (j.gruber 2014-06-06 13:30) - PLID 51324 - check >= 0
	if (nAptTypeIndex >= 0) {
		return VarLong(m_dlAptType->Value[nAptTypeIndex][3], -1);
	} else {
		//Do we have a valid inactive row showing?
		if(nAptTypeIndex == -1 && m_bValidAptTypeStored && m_dlAptType->GetIsComboBoxTextInUse() && 
			CString((LPCTSTR)m_dlAptType->GetComboBoxText()) == m_strAptTypeName ) {
			return m_nAptTypeDuration;
		}
		else {
			return -1;
		}
	}
}

// Get the appropriate Location ID
long CResEntryDlg::GetCurLocationID()
{
	// Make sure our variable is set
	if (m_bIsCurLocationSet)
	{		
		// (z.manning 2011-07-01 13:02) - PLID 32004 - We now allow no inital selection
		// See if we have a datalist to compare our value to
		/*if (m_dlAptLocation != NULL) {
			// Make sure the location selection is valid
			long nCurSel = m_dlAptLocation->GetCurSel();
			if (!(
				// No row selected because it's an inactive location
				(nCurSel == sriNoRow && m_dlAptLocation->GetIsComboBoxTextInUse() && m_strCurLocationName.Compare(m_dlAptLocation->GetComboBoxText()) == 0) ||
				// No row selected because it's still waiting for the requery to finish
				(nCurSel == sriNoRowYet_WillFireEvent) || 
				// A row is selected and it matches up with what we're about to save
				(nCurSel >= 0 && VarLong(m_dlAptLocation->GetValue(nCurSel, 0)) == m_nCurLocationID)))
			{
				// One of the above assertions failed
				ASSERT(FALSE);
				ThrowNxException("The current selection in the locations dropdown does not match the stored value (locset = %s, locid = %li)!", 
					m_bIsCurLocationSet ? "true" : "false", m_nCurLocationID);
			}
		}*/

		// We now are sure our value is correct because if the datalist exists, the selection in it matches our value
		return m_nCurLocationID;
	}
	else {
		// It has to be set before it can be read
		ThrowNxException("GetCurLocationID was called before the current location was set!");
	}
}

CString CResEntryDlg::GetCurLocationName()
{
	if (m_bIsCurLocationSet) {
		return m_strCurLocationName;
	} else {
		ThrowNxException("GetCurLocationName was called before the current location was set!");
	}
}


CString CResEntryDlg::GetResourceString()
{
	if (m_bIsCurLocationSet) {
		m_strResourceString = GenerateResourceString();
		return m_strResourceString;
	} else {
		ThrowNxException("GetCurResourceString threw an error");
	}
}


// The strNewLocName is optional because it may not be known at the time of calling the function 
// and it's not a key value.  If it's not passed in, it will either be pulled out of the datalist 
// itself (when it is loaded) or if the datalist never has the value (like if the location is 
// inactive) then it will be explicitly looked up in the database.
// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
void CResEntryDlg::SetCurLocation(long nNewLocID, OPTIONAL const CString &strNewLocName)
{
	// Set these immediately in case anyone asks between now and when the 
	// datalist actually gets set (which could be a while if TrySetSel 
	// returns sriNoRowYet_WillFireEvent) 
	m_nCurLocationID = nNewLocID;
	m_strCurLocationName = strNewLocName;
	m_bIsCurLocationSet = TRUE;

	// (z.manning 2011-07-01 13:03) - PLID 32004 - We now allow no initial selection
	if(nNewLocID == -1) {
		m_dlAptLocation->CurSel = NULL;
		return;
	}

	// Try to set the selection, 
	long nSelResult = m_dlAptLocation->TrySetSelByColumn_Deprecated(0, m_nCurLocationID);
	if (nSelResult == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
		// The selection wasn't set yet, but the TrySetSelFinished event will be fired and we'll handle things then
	} else if (nSelResult == NXDATALIST2Lib::sriNoRow) {
		// It wasn't found, which we interpret to mean the location must be inactive (because it's not in the datalist)
		ReflectCurLocationInactive();
	} else {
		// We have a valid row index that the datalist is now set to, so get the info out of that row
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_dlAptLocation->GetCurSel();
		ASSERT(pCurSel != NULL && nSelResult == pCurSel->CalcRowNumber());
		ReflectCurLocation();
	}
}

// An exception will be thrown if there is no current selection in the datalist, so only 
// call this function when the selection has just been set to the correct row.
void CResEntryDlg::ReflectCurLocation()
{
	// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAptLocation->GetCurSel();
	ASSERT(pRow != NULL);
	m_nCurLocationID = VarLong(pRow->GetValue(0));
	m_strCurLocationName = VarString(pRow->GetValue(1));
	m_bIsCurLocationSet = TRUE;
}

// Call this function when the current location is an inactive one and therefore cannot be found in the datalist
// This will set the datalist to have no selection, and set the combo text to the name of the inactive location
void CResEntryDlg::ReflectCurLocationInactive()
{
	// Force the datalist to have no selection
	// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
	m_dlAptLocation->PutCurSel(NULL);
	// We must have a current location already (you can't call this function unless the resentry has already loaded the cur location id)
	ASSERT(m_bIsCurLocationSet);
	// Find the location name in data
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr rsLocName = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_nCurLocationID);
	// The location has to exist
	ASSERT(!rsLocName->eof);
	if (!rsLocName->eof) {
		// The location does exist, so get the name into our member string
		m_strCurLocationName = AdoFldString(rsLocName, "Name");
		// And put the name in the combo text of the location datalist
		m_dlAptLocation->PutComboBoxText((LPCTSTR)m_strCurLocationName);
	} else {
		// The location doesn't exist in data so this is a real problem
		ThrowNxException("ReflectCurLocationInactive cannot load inactive location: locid = %li", m_nCurLocationID);
	}
}

// This should only be called in utter error conditions.  It's when the code just doesn't know what the appointment's 
// location is.  Calling this makes no selection in the datalist and clears the member variables that store the cur 
// location, which means that an exception is likely to happen when the user tries to save the appointment.
void CResEntryDlg::ReflectCurLocationFailure()
{
	// Clear the member variables
	m_bIsCurLocationSet = FALSE;
	m_nCurLocationID = -1;
	m_strCurLocationName = _T("");
	
	// Set the datalist to no selection
	// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
	m_dlAptLocation->PutCurSel(NULL);
}

// (a.walling 2010-06-14 14:33) - PLID 23560 - Resource Sets
// (j.jones 2014-12-02 15:55) - PLID 64178 - if a mix rule is overridden, it is passed up to the caller
BOOL CResEntryDlg::ValidateDataWithResourceSets(BOOL bCheckRules, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT shared_ptr<SelectedFFASlot> &pSelectedFFASlot)
{
	CKeepDialogActive kda(this);

	// check for all the other stuff before bothering with the resources.
	if (!ValidateDataNoResourceSets(bCheckRules, overriddenMixRules, pSelectedFFASlot)) {
		return FALSE;
	}
	else {

		// (a.walling 2010-06-16 17:34) - PLID 23560 - Check for an actual type before handling anything else
		long nCurAptTypeID = GetCurAptTypeID();

		if (nCurAptTypeID == -1) {
			return TRUE;
		}
		
		// (a.walling 2010-06-16 17:34) - PLID 23560 - Compare type and purposes as well as resources
		//TES 10/20/2010 - PLID 41044 - We also need to check the rules if the start or end time has changed.
		COleDateTime dtCurrentDate = GetDate();
		COleDateTime dtCurrentStartTime = m_nxtStart->GetDateTime();
		dtCurrentStartTime.SetDateTime(
				dtCurrentDate.GetYear(), dtCurrentDate.GetMonth(), dtCurrentDate.GetDay(),
				dtCurrentStartTime.GetHour(), dtCurrentStartTime.GetMinute(), dtCurrentStartTime.GetSecond());
		COleDateTime dtCurrentEndTime = m_nxtEnd->GetDateTime();
		dtCurrentEndTime.SetDateTime(
				dtCurrentDate.GetYear(), dtCurrentDate.GetMonth(), dtCurrentDate.GetDay(),
				dtCurrentEndTime.GetHour(), dtCurrentEndTime.GetMinute(), dtCurrentEndTime.GetSecond());
		if (!bCheckRules || 
			(!m_NewRes && 
				(CompareArrays(m_adwAptInitResources, m_arydwSelResourceIDs) && 
				CompareArrays(m_adwAptInitPurposes, m_adwPurposeID) && 
				m_nAptInitType == nCurAptTypeID &&
				dtCurrentStartTime == m_dtAptInitStartTime &&
				dtCurrentEndTime == m_dtAptInitEndTime
			) )) {
			return TRUE;
		}

		bool bIncludeNonPurpose = false;
		CDWordArray dwaConfigPurposes;
		// First, we need to gather the resource set info
		{
			_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), 
				"SELECT ResourceSetID, AptPurposeID "
				"FROM AptResourceSetLinksT "
				"WHERE AptTypeID = {INT}", nCurAptTypeID);

			if (prs->eof) {
				return TRUE;
			}

			bool bHasNonPurpose = false;
			while (!prs->eof) {
				int nPurposeID = AdoFldLong(prs, "AptPurposeID", -1);

				if (nPurposeID != -1) {
					dwaConfigPurposes.Add((DWORD)nPurposeID);
				} else {
					bHasNonPurpose = true;
				}

				prs->MoveNext();
			}

			for (int i = dwaConfigPurposes.GetSize() - 1; i >= 0; i--) {
				bool bFound = false;
				for (int j = 0; j < m_adwPurposeID.GetSize() && !bFound; j++) {
					if (m_adwPurposeID[j] == dwaConfigPurposes[i]) {
						bFound = true;
					}
				}

				if (!bFound) {
					// get rid of it in the array
					dwaConfigPurposes.RemoveAt(i);
				}
			}
			// Now dwaConfigPurposes contains all purposes that we would be interested in getting resource set info for.
			// So, dwaConfigPurposes will not contain any items that are not also in m_adwPurposeID.
			// However, m_adwPurposeID may contain items that are not contained within dwaConfigPurposes.
			// If bHasNonPurpose, then we need to check to see if there are items in m_adwPurposeID that do not exist
			// in dwaConfigPurposes so we can ensure that the non-purpose resource sets are included.

			if (m_adwPurposeID.IsEmpty()) {
				if (!bHasNonPurpose) {
					return TRUE;
				} else {
					bIncludeNonPurpose = true;
				}
			} else {
				for (int i = 0; i < m_adwPurposeID.GetSize() && !bIncludeNonPurpose; i++) {
					bool bFound = false;
					for (int j = 0; j < dwaConfigPurposes.GetSize() && !bFound; j++) {
						if (m_adwPurposeID[i] == dwaConfigPurposes[j]) {
							bFound = true;
						}
					}

					if (!bFound) {
						bIncludeNonPurpose = true;
					}
				}
			}
		}

		_RecordsetPtr prs;

		CString strFilter;
		if (dwaConfigPurposes.IsEmpty()) {
			// no specific configuration for purposes for this type.
			if (!bIncludeNonPurpose) {
				// no config for non-purpose? then quit.
				return TRUE;
			} else {
				// only include non-purpose sets for this appointment type
				strFilter = "AptResourceSetLinksT.AptPurposeID IS NULL";
			}
		} else {
			if (bIncludeNonPurpose) {
				strFilter.Format("AptResourceSetLinksT.AptPurposeID IS NULL OR AptResourceSetLinksT.AptPurposeID IN (%s)", ArrayAsString(dwaConfigPurposes));
			} else {
				strFilter.Format("AptResourceSetLinksT.AptPurposeID IN (%s)", ArrayAsString(dwaConfigPurposes));
			}
		}

		prs = CreateParamRecordset(GetRemoteData(), 
			FormatString("SELECT ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex, ResourceSetDetailsT.ResourceID "
			"FROM AptResourceSetLinksT "
			"INNER JOIN ResourceSetT "
				"ON AptResourceSetLinksT.ResourceSetID = ResourceSetT.ID "
			"INNER JOIN ResourceSetDetailsT "
				"ON ResourceSetT.ID = ResourceSetDetailsT.ResourceSetID "
			"INNER JOIN ResourceT "
				"ON ResourceSetDetailsT.ResourceID = ResourceT.ID "
			"WHERE AptResourceSetLinksT.AptTypeID = {INT} AND ResourceT.Inactive = 0 AND (%s) "
			"GROUP BY ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex, ResourceSetDetailsT.ResourceID "
			"ORDER BY ResourceSetDetailsT.ResourceSetID, ResourceSetDetailsT.OrderIndex, ResourceSetDetailsT.ResourceID", strFilter),
			nCurAptTypeID
		);		

		if (prs->eof) {
			return TRUE;
		}
		
		CDWordArray dwaBaseResources;
		dwaBaseResources.Copy(m_arydwSelResourceIDs);

		CDWordArray dwaExistingResources;
		dwaExistingResources.Copy(m_arydwSelResourceIDs);

		ResourceSet* pFirstResourceSet = NULL;
		ResourceSet* pCurrentResourceSet = NULL;
		int nTotalResourceItems = 0;
		while (!prs->eof) {
			int nResourceSetID = AdoFldLong(prs, "ResourceSetID");
			DWORD nResourceID = (DWORD)AdoFldLong(prs, "ResourceID");		
			
			// (a.walling 2010-06-15 12:34) - PLID 23560 - Resource Sets - How to handle existing resources that are part of the set?
			// After playing around with a few approaches, the most reasonable seems to be to leave them alone.
			/*
			for (int i = dwaBaseResources.GetSize() - 1; i >= 0; i--) {
				if (dwaBaseResources.GetAt(i) == nResourceID) {
					dwaBaseResources.RemoveAt(i);
					dwaExistingResources.Add(nResourceID);
				}
			}
			*/
			if (IsIDInArray((DWORD)nResourceID, &dwaBaseResources)) {
				// this resource has already been chosen, so ignore this set.
				
				if (pCurrentResourceSet != NULL && nResourceSetID == pCurrentResourceSet->m_nResourceSetID) {
					pCurrentResourceSet->m_nResourceSetID = -1;
					pCurrentResourceSet->m_arResources.RemoveAll();
				}

				long nSkippingResourceSetID = nResourceSetID;
				while (!prs->eof) {
					nResourceSetID = AdoFldLong(prs, "ResourceSetID");
					nResourceID = (DWORD)AdoFldLong(prs, "ResourceID");	

					if (nResourceSetID != nSkippingResourceSetID) {
						// (a.walling 2010-06-16 17:33) - PLID 23560 - Only assign if the current resource set is not null
						// (b.spivey, May 18, 2012) - PLID 50468 - If we're on the first resource set and this resource set has 
						//	 not been cleared out yet, we can update the resource set ID. Otherwise we may get residual values
						//	 once we break out of this loop. 
						if (pCurrentResourceSet != NULL 
							&& (pCurrentResourceSet != pFirstResourceSet 
							|| pFirstResourceSet->m_nResourceSetID == -1)) {

							pCurrentResourceSet->m_nResourceSetID = nResourceSetID;
						}
						break;
					}

					prs->MoveNext();
				}

				if (prs->eof) {
					break;
				}
			}

			if (pFirstResourceSet == NULL) {
				pFirstResourceSet = new ResourceSet(nResourceSetID);
				pCurrentResourceSet = pFirstResourceSet;
			}

			if (nResourceSetID != pCurrentResourceSet->m_nResourceSetID) {
				pCurrentResourceSet->m_pNext = new ResourceSet(nResourceSetID);
				pCurrentResourceSet = pCurrentResourceSet->m_pNext;
			}

			pCurrentResourceSet->m_arResources.Add(nResourceID);
			nTotalResourceItems++;


			prs->MoveNext();
		}

		if (nTotalResourceItems == 0) {
			return TRUE;
		}

		BOOL bReturn = FALSE;

		bool bIgnoredWarnings = false;
		CDWordArray dwaFinalResources;

		BOOL bFit = FitResourceSets(dwaBaseResources, dwaFinalResources, pFirstResourceSet, 0, bIgnoredWarnings);
		
		// (a.walling 2010-06-17 09:46) - PLID 23560 - Keep track of our initial resources
		if (bFit && CompareArrays(dwaExistingResources, dwaFinalResources)) {
			// arrays have not changed, so just return. We have already validated with this set.
			bReturn = TRUE;
		} else {
			if (bFit && !dwaFinalResources.IsEmpty()) {
				ChooseMultipleResources(false, dwaFinalResources);

				// (a.walling 2010-06-17 11:30) - PLID 23560 - These messages were just confusing. They will get the warnings regardless when they save.
				// Instead, just inform them of the new set of resources.

				/*
				if (bIgnoredWarnings) {
					if (IDYES == MessageBox("Some warnings were encountered while automatically assigning resources to this appointment.\r\n\r\nDo you want to review the changes before continuing?", NULL, MB_YESNO|MB_ICONINFORMATION)) {
						bReturn = FALSE;
					} else {
						bReturn = ValidateDataNoResourceSets(bCheckRules);
					}
				} else {
					if (IDYES == MessageBox("Practice has automatically assigned resources to this appointment.\r\n\r\nDo you want to review the changes before continuing?", NULL, MB_YESNO|MB_ICONQUESTION)) {
						bReturn = FALSE;					
					} else {
						bReturn = ValidateDataNoResourceSets(bCheckRules);
					}
				}
				*/

				_RecordsetPtr prsResources = CreateRecordset("SELECT Item FROM ResourceT WHERE ID IN (%s) ORDER BY Item", ArrayAsString(dwaFinalResources, true));
				
				if (!prsResources->eof) {
					CString strResourceString;
					while (!prsResources->eof) {
						CString strResourceName = AdoFldString(prsResources, "Item");
						strResourceString += strResourceName;
						strResourceString += ", ";

						prsResources->MoveNext();
					}

					strResourceString.TrimRight(", ");

					MessageBox(FormatString("Practice has automatically selected assigned resources to this appointment. The appointment is now using these resources:\r\n\r\n%s", strResourceString), NULL, MB_ICONINFORMATION);
				}

				bReturn = ValidateDataNoResourceSets(bCheckRules, overriddenMixRules, pSelectedFFASlot);
			} else {
				// (a.walling 2010-06-17 11:30) - PLID 23560 - Alternative approach: just show the user what they need.

				CString strMessage;
				bool bNoConflicts = CheckNewApptResourceSetConflicts(nCurAptTypeID, m_adwPurposeID, dwaExistingResources, strMessage);

				if (!bNoConflicts) {
					CString strFinalMessage;
					strFinalMessage.Format("Practice was unable to automatically assign resources for this appointment due to warnings. This appointment must be manually saved once the following warnings are resolved:\r\n\r\n%s", strMessage);

					MessageBox(strFinalMessage, NULL, MB_ICONERROR);

					bReturn = FALSE;
				} else {
					bReturn = TRUE;
				}
			}
		}

		ResourceSet* pResourceSet = pFirstResourceSet;
		while (pResourceSet) {
			ResourceSet* pNext = pResourceSet->m_pNext;
			delete pResourceSet;
			pResourceSet = pNext;
		}

		return bReturn;
	}
}

BOOL CResEntryDlg::FitResourceSets(const CDWordArray& dwaBaseResources, CDWordArray& dwaFinalResources, ResourceSet* pResourceSet, int nRuleTolerance, bool& bIgnoredWarnings)
{
	// (a.walling 2010-06-16 18:01) - PLID 23560
	if (pResourceSet == NULL || (pResourceSet && pResourceSet->m_arResources.IsEmpty() && pResourceSet->m_pNext == NULL)) {
		dwaFinalResources.Copy(dwaBaseResources);
		return TRUE;
	}

	if (pResourceSet && pResourceSet->m_arResources.IsEmpty() && pResourceSet->m_pNext != NULL) {
		return FitResourceSets(dwaBaseResources, dwaFinalResources, pResourceSet->m_pNext, nRuleTolerance, bIgnoredWarnings);
	}

	
	for (int i = 0; i < pResourceSet->m_arResources.GetSize(); i++) {
		CDWordArray dwaCheckResources;
		dwaCheckResources.Copy(dwaBaseResources);
		dwaCheckResources.Add(pResourceSet->m_arResources.GetAt(i));

		// now we will sequentially attempt to save by appending the next resource in the set.
	
		bool ResourcesAreOK = CheckResources(dwaCheckResources, nRuleTolerance);

		if (ResourcesAreOK) {
			// well, this works! But we also need to figure out if it works for the other sets.

			if (pResourceSet->m_pNext == NULL) {
				dwaFinalResources.Copy(dwaCheckResources);
				return TRUE;
			} else if (FitResourceSets(dwaCheckResources, dwaFinalResources, pResourceSet->m_pNext, 0, bIgnoredWarnings)) {
				// we are good!
				return TRUE;
			}
			// otherwise, we must keep searching!
		}
	}

	if (nRuleTolerance == 0) {
		bool bSubIgnore = true;
		BOOL bRet = FitResourceSets(dwaBaseResources, dwaFinalResources, pResourceSet, rwHasWarnings, bSubIgnore);
		if (bRet) {			
			bIgnoredWarnings = true;
		}
		return bRet;
	}
	return FALSE;
}

bool CResEntryDlg::CheckResources(const CDWordArray& dwaResources, int nRuleTolerance)
{
	//TES 10/27/2010 - PLID 40868 - Call GetTemplateItemID(), which will calculate the value if necessary
	BOOL bRes = AppointmentValidateByRules(GetCurPatientID(), dwaResources, GetCurLocationID(), 
		GetDate(), m_nxtStart->GetDateTime(), m_nxtEnd->GetDateTime(), GetCurAptTypeID(), 
		m_adwPurposeID, m_ResID, FALSE, FALSE, GetTemplateItemID(), TRUE, nRuleTolerance);

	return bRes ? TRUE : FALSE;
}

// CAH 6/20/03 - When we cancel appointments, we don't want to check for templates,
// permissions and alarm rules. Should we decide that we need to check any one of these
// things later on, I suggest we replace the BOOL with either a enumeration or
// flags that specify what rules to check.
// (j.jones 2014-12-02 15:55) - PLID 64178 - if a mix rule is overridden, it is passed up to the caller
BOOL CResEntryDlg::ValidateDataNoResourceSets(BOOL bCheckRules, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot)
{
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging

	// (z.manning, 08/11/2006) - PLID 21898 - Need to make sure we have valid times.
	if(m_nxtStart->GetStatus() != 1) {
		MessageBox("This appointment cannot be saved because you have entered an invalid start time.");
		GetDlgItem(IDC_START_TIME_BOX)->SetFocus();
		return FALSE;
	}
	if(m_nxtEnd->GetStatus() != 1) {
		MessageBox("This appointment cannot be saved because you have entered an invalid end time.");
		GetDlgItem(IDC_END_TIME_BOX)->SetFocus();
		return FALSE;
	}
	if(m_nxtArrival->GetStatus() != 1) {
		if(GetDlgItem(IDC_ARRIVAL_TIME_BOX)->IsWindowVisible()) {
			MessageBox("This appointment cannot be saved because you have entered an invalid arrival time.");
			GetDlgItem(IDC_ARRIVAL_TIME_BOX)->SetFocus();
			return FALSE;
		}
		else {
			// If we have an invalid time and the arrival time fields is not even visible then it's a bug,
			// but since they likely don't care about the arrival time, there's no reason for this to cause
			// validation to fail.
			ASSERT(FALSE);
			m_nxtArrival->SetDateTime(m_nxtStart->GetDateTime());
		}
	}

	// (z.manning 2011-07-01 13:05) - PLID 32004 - Make sure they select a location first
	if(m_nCurLocationID == -1) {
		MessageBox("You must select a location first.", NULL, MB_ICONERROR);
		return FALSE;
	}

	APPT_INFO aptInfo;
	// (a.walling 2010-06-15 07:32) - PLID 30856
	FillApptInfo(aptInfo);

	// (c.haag 2010-03-08 16:58) - PLID 37326 - Determine whether we need to validate this appointment against template
	// rules. If various key fields match perfectly, then we do not.	
	BOOL bValidateTemplates = TRUE;
	// (j.jones 2014-11-26 17:23) - PLID 64178 - mix rules follow the same logic but also check if the location or primary insurance changed
	BOOL bValidateMixRules = TRUE;
	if (m_ResID > 0) {
		//
		// If we get here, the appointment is existing. So, we need to do some further testing. Start by pulling the time values
		// from aptInfo, as opposed to m_dtStart/End which are not set by this point.
		//
		COleDateTime dtActualStart, dtActualEnd;
		// Make sure the date, start and end times are valid. If any are not, make our "actual" times invalid. This will cause
		// the test to return a negative, and templates will be checked (I don't see how this could happen, but I won't make
		// assumptions)
		if (aptInfo.dtApptDate.GetStatus() == COleDateTime::valid &&
			aptInfo.dtStartTime.GetStatus() == COleDateTime::valid &&
			aptInfo.dtEndTime.GetStatus() == COleDateTime::valid)
		{
			dtActualStart.SetDateTime(
				aptInfo.dtApptDate.GetYear(), aptInfo.dtApptDate.GetMonth(), aptInfo.dtApptDate.GetDay(),
				aptInfo.dtStartTime.GetHour(), aptInfo.dtStartTime.GetMinute(), aptInfo.dtStartTime.GetSecond());
			dtActualEnd.SetDateTime(
				aptInfo.dtApptDate.GetYear(), aptInfo.dtApptDate.GetMonth(), aptInfo.dtApptDate.GetDay(),
				aptInfo.dtEndTime.GetHour(), aptInfo.dtEndTime.GetMinute(), aptInfo.dtEndTime.GetSecond());
		}
		else {
			// Somehow, one of the date/time fields is invalid
			dtActualStart.SetStatus(COleDateTime::invalid);
			dtActualEnd.SetStatus(COleDateTime::invalid);
		}

		if (m_nAptInitType == aptInfo.nAptTypeID && // Types must match
			CompareArrays(m_adwAptInitResources, m_arydwSelResourceIDs) && // Resources must match
			CompareArrays(m_adwAptInitPurposes, m_adwPurposeID) && // Purposes must match

			dtActualStart.GetStatus() == COleDateTime::valid && // Start time must be valid and match
			m_dtAptInitStartTime.GetStatus() == COleDateTime::valid &&
			dtActualStart == m_dtAptInitStartTime &&

			dtActualEnd.GetStatus() == COleDateTime::valid && // End time must be valid and match
			m_dtAptInitEndTime.GetStatus() == COleDateTime::valid &&
			dtActualEnd == m_dtAptInitEndTime
			)
		{
			// If we get here, no template-related appointment information changed, so there's no need to validate
			// the appointment against templates. Arguably, the templates may have changed. However, without this
			// code, the user will be unable to open and save changes to an appointment that was already saved
			// over a precision template.
			bValidateTemplates = FALSE;
			
			// (j.jones 2014-11-26 17:23) - PLID 64178 - if the above checked out ok, we still need to check to see
			// if the location changed or the primary insurance changed, for mix rule validation
			if (GetCurLocationID() == m_nInitLocationID) {
				//location didn't change, now check the primary insurance
				InsuranceInfo *pOrigInsInfo = NULL;
				InsuranceInfo *pCurInsInfo = NULL;
				m_mapOrigIns.Lookup(priIns, pOrigInsInfo);
				m_mapInsPlacements.Lookup(priIns, pCurInsInfo);
				//insurance matches if both are null, or if both are not null and the IDs are the same
				if ((pOrigInsInfo == NULL && pCurInsInfo == NULL)
					|| (pOrigInsInfo != NULL && pCurInsInfo != NULL && pOrigInsInfo->nInsuredPartyID == pCurInsInfo->nInsuredPartyID)) {

					bValidateMixRules = FALSE;
				}
			}
		}
		else {
			// If we get here, either no information changed, or at least one time is invalid. Proceed to validate
			// the templates.
		}

	} // if (m_ResID > 0) {
	else {
		// Always test template rules for new appts
	}

	// (c.haag 2010-03-08 16:36) - PLID 37326 - Updated to use new parameters
	//TES 10/27/2010 - PLID 40868 - Call GetTemplateItemID(), which will calculate the value if necessary
	// (j.jones 2014-11-26 10:43) - PLID 64179 - added pParentWnd
	// (j.jones 2014-11-26 16:42) - PLID 64178 - Added an optional overriddenMixRules returned,
	// this is only filled if the appt. exceeded a scheduler mix rule and a user overrode it. The caller needs to save this info.
	CKeepDialogActive kda(this);
	BOOL bVal = ValidateAppt(this, &aptInfo, m_pSchedulerView, bCheckRules, FALSE, IsEvent(), TRUE, TRUE, GetTemplateItemID(), bValidateTemplates, bValidateMixRules, overriddenMixRules, pSelectedFFASlot);
	//if its an event, it was setting the end time, so we still need to do that
	if (IsEvent()) {
		COleDateTime dtTemp = m_nxtEnd->GetDateTime();
		if (aptInfo.dtEndTime != dtTemp) {
			m_nxtEnd->SetDateTime(aptInfo.dtEndTime);
		}
	}
	return bVal;
}

// (a.walling 2010-06-15 07:32) - PLID 30856
void CResEntryDlg::FillApptInfo(APPT_INFO& aptInfo)
{
	aptInfo.dtApptDate = GetDate();
	aptInfo.dtStartTime = m_nxtStart->GetDateTime();
	aptInfo.dtEndTime =  m_nxtEnd->GetDateTime();
	aptInfo.dtArrivalTime = m_nxtArrival->GetDateTime();
	aptInfo.dwPurposeList = &m_adwPurposeID;
	aptInfo.dwResourceList = &m_arydwSelResourceIDs;
	aptInfo.nApptID = m_ResID;
	aptInfo.nAptInitStatus = m_nAptInitStatus;
	aptInfo.nAptTypeID = GetCurAptTypeID();
	aptInfo.nLocationID = GetCurLocationID();
	aptInfo.nPatientID = GetCurPatientID();
	aptInfo.nShowStateID = GetNoShowComboSel();
	// (j.gruber 2012-07-31 12:24) - PLID 51830 - insurance
	aptInfo.pmapInsInfo = &m_mapInsPlacements;
}

CString CResEntryDlg::GetExtraInfo(int nOrdinal)
{
	CString Ans;// = "";
	Ans.Empty();
	BOOL tmpCheck;
	switch (nOrdinal) {
	case 0:
		GetDlgItemCheck(IDC_PATIENT_ID_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PATIENT_ID_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), Ans);
		}
		break;
	case 1:
		GetDlgItemCheck(IDC_HOME_PHONE_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS2))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PATIENT_HOME_PHONE_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS2))->GetCurSel(), Ans);
		}
		break;
	case 2:
		GetDlgItemCheck(IDC_WORK_PHONE_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS3))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PATIENT_WORK_PHONE_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS3))->GetCurSel(), Ans);
		}
		break;
	case 3:
		GetDlgItemCheck(IDC_BIRTH_DATE_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS4))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PATIENT_BIRTH_DATE_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS4))->GetCurSel(), Ans);
		}
		break;
	case 4:
		GetDlgItemCheck(IDC_CELL_PHONE_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS5))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PATIENT_CELL_PHONE_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS5))->GetCurSel(), Ans);
		}
		break;
	case 5:
		GetDlgItemCheck(IDC_PREFERRED_CONTACT_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS6))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PATIENT_PREFERRED_CONTACT_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS6))->GetCurSel(), Ans);
		}
		break;
	case 6:
		GetDlgItemCheck(IDC_PATBAL_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS7))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_PTBAL_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS7))->GetCurSel(), Ans);
		}
		break;
	case 7:
		GetDlgItemCheck(IDC_INSBAL_CHECK, tmpCheck);
		if (tmpCheck && ((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS8))->GetCurSel() != -1) {
			GetDlgItemText(IDC_RES_INSBAL_BOX, Ans);
			return FormatTextForReadset((ExtraFields)((CComboBox*)GetDlgItem(IDC_EXTRA_FIELDS8))->GetCurSel(), Ans);
		}
		break;
	}

	return Ans;
}

/*
void CResEntryDlg::OnColumnClickPatientCombo(long iColumn) 
{
	((CMainFrame *)AfxGetMainWnd())->m_patToolBar.SetActivePatientID(m_PatientCombo.GetValue().lVal);
	CString TmpSQLStr;
	switch(iColumn){
	case 0:
		m_PatientCombo.SetTextboxFormat("[Last Name], [First Name]");
		break;
	case 1:
		m_PatientCombo.SetTextboxFormat("[First Name] [Last Name]");
		break;
	case 2:
		m_PatientCombo.SetTextboxFormat("[ID]");
		break;
	case 3:
		m_PatientCombo.SetTextboxFormat("[Birth Date]");
		break;
	case 4:
		m_PatientCombo.SetTextboxFormat("[SS #]");
		break;
	}
	m_PatientCombo.SetValue(COleVariant(((CMainFrame *)AfxGetMainWnd())->m_patToolBar.GetActivePatientID()));
	switch(iColumn){
		case 3:
			m_PatientCombo.SetText(m_PatientCombo.GetValueByColumn(0).pcVal);
			break;
		case 4:
			m_PatientCombo.SetText(m_PatientCombo.GetValueByColumn(0).pcVal);
			break;
		}
}
*/

void CResEntryDlg::PreSave()
{
	try {
		CMainFrame *pMain = GetMainFrame();
		if (pMain) {
			long nPatID = GetCurPatientID();
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT PersonID, Archived, CurrentStatus FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PersonID = {INT};", nPatID);
			if (!prs->eof) {
				// We found a record so mark the given patient as the active patient in this execution of practice
				pMain->m_patToolBar.m_bChangedInScheduler = true;
				pMain->m_patToolBar.m_PatientIDFromScheduler = nPatID;
				pMain->m_patToolBar.m_PatientStatusFromSchedule = AdoFldShort(prs, "CurrentStatus");
				// (j.jones 2011-07-22 13:45) - PLID 44117 - check the preference before marking as active
				if(AdoFldBool(prs, "Archived") && m_NewRes
					&& GetRemotePropertyInt("AutoActivateScheduledInactivePts", 1, 0, "<None>", true) == 1) 
				{
					// (a.walling 2012-07-30 13:55) - PLID 51868 - Don't update using a cursor; could cause cursor conflict
					ExecuteParamSql("UPDATE PersonT SET Archived = 0 WHERE ID = {INT}", nPatID);
				}
			} else {
				pMain->m_patToolBar.m_bChangedInScheduler = false;
			}
		} else {
			AfxThrowNxException("Could not obtain MainFrame object");
		}
	} CResEntryDlg_NxCatchAll("CResEntryDlg::PreSave");
}

void CResEntryDlg::SetComboValue(_DNxDataListPtr &cboCombo, const _variant_t &varValue, bool bDropDown /* = true */)
{
//
//	// Set what WE WANT the auto-drop-down status to be
//	bool bTempDropDown = (cboCombo.GetPulldownOnFind()?true:false);
//	cboCombo.SetPulldownOnFind(bDropDown);


	// Actually set the value
	cboCombo->SetSelByColumn(0, varValue);


//
//	// Return the auto-drop-down status to whatever it was before
//	cboCombo.SetPulldownOnFind(bTempDropDown);
}

void CResEntryDlg::SetComboRecordNumber(_DNxDataListPtr &cboCombo, long nPos, bool bDropDown /* = true */)
{
//
//	// Set what WE WANT the auto-drop-down status to be
//	bool bTempDropDown = (cboCombo.GetPulldownOnFind()?true:false);
//	cboCombo.SetPulldownOnFind(bDropDown);


	// Actually set the value
	cboCombo->CurSel = nPos;


//
//	// Return the auto-drop-down status to whatever it was before
//	cboCombo.SetPulldownOnFind(bTempDropDown);
}

void CResEntryDlg::OnGotopatient() 
{

	// TODO: (b.cardillo 2002-04-12 11:30 AM) There are other implementations of 
	// very similar functions in a number of other places in the code.  We need 
	// to consolidate these various implementations!

	try {
		// (z.manning 2008-11-24 11:24) - PLID 31330 - I started the process ob Bob's above todo by
		// moving all this code to a utility function.
		SaveAndGotoPatient(-1);

	} CResEntryDlg_NxCatchAll("CResEntryDlg::OnGotopatient");
}

// CAH 4/17: Returns TRUE if this is an event
BOOL CResEntryDlg::IsEvent()
{
	// Create a recordset based on the current appointment ID
	if (m_dtStartTime == m_dtEndTime && m_dtStartTime.GetHour() == 0 && m_dtStartTime.GetMinute() == 0)
		return TRUE;
	return FALSE;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnDropDownDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		IncActiveCount();

		*pResult = 0;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnDropDownDateEventStart(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		IncActiveCount();

		*pResult = 0;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnDropDownDateEventEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		IncActiveCount();

		*pResult = 0;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnCloseUpDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {

		//DRT - 6/4/03 - They've chosen a date, have it prompt them for ins auth if necessary and react
		{
			long nPatientID = GetCurPatientID();
			m_bCheckActive = false;
			bool bContinue = AttemptWarnForInsAuth(nPatientID, GetDate());
			m_bCheckActive = true;

			// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
			if(!bContinue) {
				//they said no they don't want to continue.  Cancel the dialog
				// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
				HandleCancelAndCloseReservation();
				return;
			}
		}
		//done

		DecActiveCount();
		SetFocus();
	
	}CResEntryDlg_NxCatchAll("Error in OnCloseUpDate()");

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnCloseUpDateEventStart(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		DecActiveCount();
		SetFocus();

		*pResult = 0;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnCloseUpDateEventEnd(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		DecActiveCount();
		SetFocus();

		*pResult = 0;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelChangedApttypeCombo(long nRow) 
{
	/////////////////////////////////////////////////////////////
	// (c.haag 2004-04-02 15:52) - All of the code here
	// has been moved to OnSelChosenApttypeCombo since
	// there is no need to requery the purposes and recolor
	// the reservation just by browsing through available types
	// in the dropdown
	/////////////////////////////////////////////////////////////
}

void CResEntryDlg::OnSelChangedAptPurposeCombo(long nRow) 
{
	try {
		// Just apply the new text to the reservation box
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelChangingAptPurposeCombo(long FAR* nNewSel) 
{
	try {
		if (*nNewSel == -1)
			*nNewSel = m_dlAptPurpose->CurSel;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

// This code is based on void CPatientToolBar::ChangePatient(long nID)
// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
// (a.walling 2010-12-30 11:18) - PLID 40081 - Use datalist2 to avoid row index race conditions
NXDATALIST2Lib::IRowSettingsPtr CResEntryDlg::UpdatePatient(long nID)
{
	// It's expected that no one will call this function on the -25 patient
	CResEntryDlg_ASSERT(nID != -25);
	if (nID == -25) {
		return NULL;
	}


	try {
		//TES 1/18/2010 - PLID 36895 - Pull from our GetPatientFromClause() function, and hide BirthDate and SocialSecurity as appropriate.
		// (j.jones 2010-05-04 12:03) - PLID 32325 - added OHIP Health Card Num.
		// (j.gruber 2010-10-04 14:36) - PLID 40415 - added security group
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (j.gruber 2011-07-22 17:17) - PLID 44118 - included forecolor
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), FormatString("SELECT ComboQ.PersonID, ComboQ.Last, ComboQ.First, ComboQ.Middle, "
			// (a.walling 2010-05-27 15:35) - PLID 38917 - No more Name column
			//"(ComboQ.[Last] + ', ' + ComboQ.[First] + ' ' + ComboQ.[Middle]) AS FullName, "
			"ComboQ.PatientID, CASE WHEN BlockedPatientsQ.PatientID Is Null THEN ComboQ.BirthDate ELSE NULL END AS BirthDate, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN ComboQ.SocialSecurity ELSE NULL END AS SocialSecurity, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN OHIPHealthCardNum ELSE NULL END AS OHIPHealthCardNum, "
			" ComboQ.SecGroupName, "
			" ComboQ.ForeColor "
			"FROM %s "
			"WHERE ComboQ.PersonID = {INT} %s", GetPatientFromClause(), GetRemotePropertyInt("SchedShowActivePtOnly", 0, 0, "<None>", true) ? "AND Archived = 0" : "")
			, nID);

		// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAptPatient->FindByColumn(plID, nID, NULL, VARIANT_FALSE); // (a.walling 2010-12-30 11:19) - PLID 40081 - Use datalist2 to avoid row index race conditions

		if (rs->eof)
		{	//patient was deleted
			if (pRow) {	//don't remove if not in list

				BOOL bCurrent = FALSE;

				if(IsWindowVisible()) {
					//if it is the currently selected patient, warn the user
					if(pRow->IsSameRow(m_dlAptPatient->GetCurSel())) { 

						bCurrent = TRUE;
						
						CChildFrame *p = GetMainFrame()->GetActiveViewFrame();
						if (p) {
							CNxTabView *pView = (CNxTabView *)p->GetActiveView();
							CChildFrame *pFrame = GetMainFrame()->GetActiveViewFrame();
							if (pView && pFrame->IsOfType(SCHEDULER_MODULE_NAME)) {
								if(m_NewRes) {
									CKeepDialogActive kda(this);
									MsgBox("This patient has been deleted by another user.\n"
									"The selection will now be changed to the next patient.");
								}
								else {
									//JMJ - deleting a patient now fires the delete appointment message,
									//so this message box would be redundant, but it should now never fire.
									//Just incase, we will keep the code here if the apt.delete message fails.
									MsgBox("This patient has been deleted by another user.\n"
									"The appointment you were editing has since been deleted.");
								}
							}
						}
					}
				}

				m_dlAptPatient->RemoveRow(pRow);
				
				if(bCurrent) {
					//TES 1/15/2010 - PLID 36762 - We rely on m_nCurPatientID being a patient we can access, so use GetActivePatientID(), 
					// which will always return an accessible patient.
					SetCurPatient(GetActivePatientID());
				}
				else {
					// (b.cardillo 2005-05-25 10:33) - PLID 16575 - For plid 8816 we need to set the current 
					// patient on the resentry to be a valid patient, because if the m_nCurPatientID ever 
					// doesn't match what's selected in the combo an exception will be thrown.  But for plid 
					// 16583 we don't want to just change the appointment's patient to be the system active 
					// patient, because that makes no sense.  The user would be sitting there looking at a 
					// perfectly valid appointment when all of the sudden for no good reason the current 
					// patient just changes.  Then again, if the appointment the user is looking at is FOR 
					// the patient we just removed from the list, then the selection HAS to change, and we 
					// need to keep the selection in sync with the m_nCurPatientID.  SO, the resolution is to 
					// set the current patient to be the active patient (as we did for plid 8816) BUT to ONLY 
					// do so IF the id we just removed from the list (nID) is identical to what we last 
					// thought we had selected (m_nCurPatientID).  So I've added that if-statement around the 
					// existing code.  Now, you may wonder under what circumstances nID would be identical to 
					// m_nCurPatientID while bCurrent would be false, because bCurrent is supposed to be true 
					// if nRow was the same as the current sel row prior to removing nRow.  The answer is, it 
					// can happen only when the resentry dialog isn't visible!  And that's correct behavior, 
					// because if bCurrent is true, then OnSelChosenPatientCombo() gets called, which in turn 
					// does a bunch of stuff that could prompt the user, which we certainly don't want if the 
					// resentry isn't even on the screen.  But we do sort of want to do all the same "behind-
					// the-scenes" stuff that OnSelChosenPatientCombo() does, which boils down to just the 
					// ReflectCurPatient() function.  So we would simply call that function here, but since 
					// our selection just changed (which we know it did if nID == m_nCurPatientID) we want to 
					// pick the patient it changes to (first just so it can be predicted, and second so that 
					// we know it didn't change to "nothing"), so we call SetCurPatient() which essentially 
					// just calls ReflectCurPatient() after making sure the "cur patient" is what we want it 
					// to be.  Whew!  Jobdone.
					if (nID == m_nCurPatientID) {
						SetCurPatient(GetActivePatientID());
					}
				}
			}
		}
		else
		{
			//IRowSettingsPtr pRow;
			FieldsPtr f = rs->Fields;

			bool bIsCurSel = false;
			if (pRow) {
				if (pRow == m_dlAptPatient->CurSel) {
					bIsCurSel = true;
				} else {
					m_dlAptPatient->RemoveRow(pRow);
					pRow = NULL;
				}
			}

			if (!pRow) {
				pRow = m_dlAptPatient->GetNewRow();
			}

			// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
			pRow->Value[plID] = f->Item["PersonID"]->Value;
			pRow->Value[plLast] = f->Item["Last"]->Value;
			pRow->Value[plFirst] = f->Item["First"]->Value;
			pRow->Value[plMiddle] = f->Item["Middle"]->Value;
			// (a.walling 2010-05-27 15:35) - PLID 38917 - No more Name column
			//pRow->Value[4] = f->Item["FullName"]->Value;
			pRow->Value[plUserDefinedID] = f->Item["PatientID"]->Value;
			pRow->Value[plBirthDate] = f->Item["BirthDate"]->Value;
			pRow->Value[plSSN] = f->Item["SocialSecurity"]->Value;
			// (j.jones 2010-05-04 12:03) - PLID 32325 - added OHIP Health Card Num.
			pRow->Value[plHealthCard] = f->Item["OHIPHealthCardNum"]->Value;
			// (j.gruber 2010-10-04 14:37) - PLID 40415 - security group
			pRow->Value[plSecurityGroup] = f->Item["SecGroupName"]->Value;
			// (j.gruber 2011-07-22 17:19) - PLID 44118 - Forecolor
			pRow->Value[plForeColor] = f->Item["ForeColor"]->Value;
			pRow->PutForeColor(f->Item["ForeColor"]->Value);

			// (a.walling 2013-01-21 16:48) - PLID 54743 - PatientToolBar and ResToolBar are re-sorting in response to tablecheckers even in situations that are unnecessary, which can introduce a significant delay for large lists
			if (!bIsCurSel) {
				pRow = m_dlAptPatient->AddRowSorted(pRow, NULL);
			} else {
				//m_dlAptPatient->Sort(); // remove this eventually and have the datalist sort a single row
				// (a.walling 2013-03-07 09:32) - PLID 55502 - The future is now! Use SortSingleRow
				m_dlAptPatient->SortSingleRow(pRow);
				m_dlAptPatient->EnsureRowInView(pRow);
			}

		}
		rs->Close();

		return pRow;
	} CResEntryDlg_NxCatchAll("CResEntryDlg::ChangePatient");

	return NULL;
}

void CResEntryDlg::OnKillfocusNotesBox() 
{
	try {
		//Don't let the dialog get away!
		CKeepDialogActive kda(this);

		ForceFieldLength((CNxEdit*)GetDlgItem(IDC_NOTES_BOX), 3000);
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnBtnChangepatient() 
{
	try {
		short nCategory = GetCurAptTypeCategory();
		if (nCategory != PhaseTracking::AC_BLOCK_TIME)
		{
			// Enable the patient combo and drop it down for them
			GetDlgItem(IDC_PATIENT_COMBO_2)->EnableWindow(TRUE); // (a.walling 2010-12-30 11:29) - PLID 40081 - Use datalist2 to avoid row index race conditions
			m_dlAptPatient->PutDropDownState(_variant_t(VARIANT_TRUE, VT_BOOL));

			// Enable the new patient button in case the patient they want isn't in the list
			if (GetDlgItem(IDC_BTN_NEWPATIENT))
				GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(TRUE);
			
			// Disable the change patient button
			if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
				GetDlgItem(IDC_BTN_CHANGEPATIENT)->EnableWindow(FALSE);
		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnBtnNewpatient()
{
	try {
		//CNxTabView* pView;
		long id = -1;

		//Don't let the dialog get away!
		CKeepDialogActive kda(this);

		if (!GetMainFrame()->CheckAllowCreatePatient()) {
			return;
		}

		// (z.manning, 04/16/2008) - PLID 29680 - Removed all references to obsolete 640x480 dialogs
		CNewPatient dlg(NULL, IDD_NEW_PATIENT);
		// (m.cable 6/21/2004 12:27) - le the new patient dlg know we are coming from the res entry.
		dlg.m_bFromResEntry = true;
		long nChoice = dlg.DoModal(&id);

			// (j.gruber 2010-10-04 13:26) - PLID 40413 - check to see whether we are opening the security groups dialog
		long nOpen = GetRemotePropertyInt("NewPatientOpenSecurityGroups", 0, 0, GetCurrentUserName(), true);

		//they want to open it and they didn't cancel and they have permission		
		// (j.gruber 2010-10-26 13:58) - PLID 40416 - change to assign permission, instead of write
		BOOL bHasPerms = CheckCurrentUserPermissions(bioSecurityGroup, sptDynamic0, 0, 0, TRUE, TRUE);
		if (id > -1 && nOpen == 1 && bHasPerms && nChoice != 0) {		
			
			CSecurityGroupsDlg dlg(this);
			dlg.m_nPatientID = id;
			dlg.DoModal();
		}	
		
		switch (nChoice)
		{
		case 0:		//they canceled
			break;
		case 3:	// Save and schedule
			{
			// Put the new patient in the mainframe patient list
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			GetMainFrame()->m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient);
			//TES 1/7/2010 - PLID 36761 - No need to check for failure, we wouldn't have been able to open the ResEntryDlg
			// if we couldn't access this patient.
			GetMainFrame()->m_patToolBar.TrySetActivePatientID(id);//don't need to requery if we set to active

			SetModifiedFlag();

			// Requery our list
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			UpdatePatient(id);
			//DoRequeryPatientList();

			// We used to do a WaitForRequery here, but there's no need since we now handle the TrySetSel properly
			
			// Set to the new selection
			//TES 1/14/2010 - PLID 36762 - This patient will never be blocked; it was just created, so it can't be in any security groups yet.
			SetCurPatient(id);

			// (j.gruber 2012-07-31 14:15) - PLID 51882 - insurance
			//clear out the current insurance
			ClearInsurancePlacements(FALSE);
			
			//now just invalidate
			InvalidateRect(m_rcPriIns);
			InvalidateRect(m_rcSecIns);

			ApplyToResBox();

			// (j.armen 2012-04-25 17:54) - PLID 49997 - After the patient is updated, we want to call 
			//	ApplyCurrentPatient to be sure that all of the extended ResEntryDlg properties are updated correctly
			// (j.jones 2014-11-14 10:30) - PLID 64116 - now takes in a boolean to state whether
			// the user has changed the patient on the appointment, true here because we're
			// selecting our new patient
			ApplyCurrentPatient(true);

			}
			break;

		case 2:	// Save, edit further in patients module

			// (c.haag 2005-02-08 14:11) - PLID 15298 - Dr. Lynch's office needed this feature, so I am making it a preference.

			// Put the new patient in the mainframe patient list
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			GetMainFrame()->m_patToolBar.UpdatePatient(id);
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient);
			//TES 1/7/2010 - PLID 36761 - No need to check for failure, we wouldn't have been able to open the ResEntryDlg
			// if we couldn't access this patient.
			GetMainFrame()->m_patToolBar.TrySetActivePatientID(id);//don't need to requery if we set to active

			// Assign the patient and save the appointment
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			UpdatePatient(id);
			//TES 1/14/2010 - PLID 36762 - This patient will never be blocked; it was just created, so it can't be in any security groups yet.
			SetCurPatient(id);
			ApplyToResBox();
			// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
			HandleSaveAndCloseReservation();

			if(GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
				CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
				if (pView) {
					if(pView->GetActiveTab() != 0) {
						pView->SetActiveTab(0);
					}

					// (j.jones 2009-06-03 10:25) - PLID 34461 - need to always update the patient view
					pView->UpdateView();
				}
			}

			// (m.cable 6/21/2004 12:25) - PLID 13126 - If they click on save and edit further, then they are going to lose
			// any info they entered.  The new patient dlg should have this option disabled due to setting m_bFromResEntry to true
			//ASSERT(FALSE);
			break;

		case 4:		//add another
			SetModifiedFlag();
			//first save as if they did nothing, case 1
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient);
			//and now act like they clicked the button again
			OnBtnNewpatient();
			break;
		case 1: // Save, resume previous
			SetModifiedFlag();
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here
			CClient::RefreshPatCombo(id, false, CClient::pcatActive, dlg.GetIsProspect() ? CClient::pcstProspect : CClient::pcstPatient);
			break;

		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnBtnAptLinking()
{
	try {

		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		CKeepDialogActive kda(this);
		CResLinkDlg dlg(this);

		//TES 12/18/2008 - PLID 32508 - This is not available for Scheduler Standard users.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Linking related appointments for a single patient")) {
			return;
		}

		if (!GetAppointmentLinkingAllowed())
		{
			MsgBox("You must purchase an Ambulatory Surgery Center license in order to use this feature. Please contact your NexTech sales representative for support.");
			return;
		}

		// Get the proper patient ID
		long nPatientID = GetCurPatientID();
		if (nPatientID == -25) {
			MsgBox("You must select a patient before linking appointments");
			return;
		}
		if (m_Res.GetReservationID() < 0)
		{
			MsgBox("You must save this appointment before linking it with other appointments");
			return;
		}
		dlg.Open(m_Res.GetReservationID(), nPatientID);

	}CResEntryDlg_NxCatchAll("Error opening the appointment linking dialog.");
}

void CResEntryDlg::OnBlockTimeCatSelected(BOOL bIsBlockTime)
{
	try {
		// If we're selecting a blocktime type, then hide the patient combo, and associated buttons
		if (bIsBlockTime) {
			// Hide the patient combo, show the "not applicable" label
			GetDlgItem(IDC_PATIENT_COMBO_2)->ShowWindow(SW_HIDE); // (a.walling 2010-12-30 11:29) - PLID 40081 - Use datalist2 to avoid row index race conditions
			GetDlgItem(IDC_STATIC_NOT_APPLICABLE)->ShowWindow(SW_SHOW);

			// Hide the buttons
			{
				if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
					GetDlgItem(IDC_BTN_CHANGEPATIENT)->ShowWindow(SW_HIDE);
				if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
					GetDlgItem(IDC_GOTOPATIENT)->ShowWindow(SW_HIDE);
				if (GetDlgItem(IDC_BTN_NEWPATIENT))
					GetDlgItem(IDC_BTN_NEWPATIENT)->ShowWindow(SW_HIDE);
			}
			// Because they're buttons, we need disable them so they can't be accessed with their alt-keys
			{
				GetDlgItem(IDC_GOTOPATIENT)->EnableWindow(FALSE);
				if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
					GetDlgItem(IDC_BTN_CHANGEPATIENT)->EnableWindow(FALSE);
				if (GetDlgItem(IDC_BTN_NEWPATIENT))
					GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(FALSE);
				// (z.manning 2008-11-24 09:28) - PLID 31130 - Disable the save and edit insurance button
				// if it's block time.
				GetDlgItem(IDC_GOTO_INSURANCE)->EnableWindow(FALSE);
			}
		} else {
			// Show the patient combo, hide the "not applicable" label
			GetDlgItem(IDC_PATIENT_COMBO_2)->ShowWindow(SW_SHOW); // (a.walling 2010-12-30 11:29) - PLID 40081 - Use datalist2 to avoid row index race conditions
			GetDlgItem(IDC_STATIC_NOT_APPLICABLE)->ShowWindow(SW_HIDE);

			// Show the buttons
			{
				if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
					GetDlgItem(IDC_BTN_CHANGEPATIENT)->ShowWindow(SW_SHOW);
				if (GetDlgItem(IDC_GOTOPATIENT))
					GetDlgItem(IDC_GOTOPATIENT)->ShowWindow(SW_SHOW);
				if (GetDlgItem(IDC_BTN_NEWPATIENT))
					GetDlgItem(IDC_BTN_NEWPATIENT)->ShowWindow(SW_SHOW);
				// (z.manning 2008-11-24 09:28) - PLID 31130 - Show the save and edit insurance button on
				// new appointments.
				GetDlgItem(IDC_GOTO_INSURANCE)->ShowWindow(m_NewRes ? SW_SHOW : SW_HIDE);
			}
			
			// Re-enable the now-visible buttons as appropriate
			{
				EnableGoToPatient(GetCurPatientID() != -25 ? TRUE : FALSE);

				if (GetDlgItem(IDC_PATIENT_COMBO_2)->IsWindowEnabled()) { // (a.walling 2010-12-30 11:29) - PLID 40081 - Use datalist2 to avoid row index race conditions
					if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
						GetDlgItem(IDC_BTN_CHANGEPATIENT)->EnableWindow(FALSE);
					if (GetDlgItem(IDC_BTN_NEWPATIENT))
						GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(TRUE);
				} else {
					if (GetDlgItem(IDC_BTN_CHANGEPATIENT))
						GetDlgItem(IDC_BTN_CHANGEPATIENT)->EnableWindow(TRUE);
					if (GetDlgItem(IDC_BTN_NEWPATIENT))
						GetDlgItem(IDC_BTN_NEWPATIENT)->EnableWindow(FALSE);
				}
			}
		}
	}
	CResEntryDlg_NxCatchAll("Error in OnBlockTimeCatSelected");
}

// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
void CResEntryDlg::OnSelChangingAptLocationCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (!m_dlAptLocation->GetIsComboBoxTextInUse()) {
			if (*lppNewSel == NULL)
				SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelChangingApttypeCombo(long FAR* nNewSel) 
{
	try {
		if (*nNewSel == -1)
			*nNewSel = m_dlAptType->CurSel;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelChosenAptResourceCombo(long nCurSel) 
{
	try 
	{
		// (c.haag 2015-11-12) - PLID 67577 - Call the business logic
		HandleAptResourceComboSelection(nCurSel);	
	}
	CResEntryDlg_NxCatchAll(__FUNCTION__);
}

void CResEntryDlg::HandleAptResourceComboSelection(long nCurSel)
{
	try
	{
		SetModifiedFlag();

		//boolean to set if we do not want to try filling the default end times no matter what
		bool bDontFill = false;

		if (nCurSel == sriNoRow) {
			//if the resource somehow became unselected, try to re-select the current resource
			CResEntryDlg_ASSERT(m_arydwSelResourceIDs.GetSize() == 1); // The datalist should be visible iff there's exactly one resource for this appointment.
																	   // Also check for the special '0' case, where there are no entries, which can only happen if you're making an appointment on a resource that no longer exists in the datalist
			if (m_arydwSelResourceIDs.GetSize() == 0) {
				// If there was nothing in the array, then we're stuck, we just have to leave it as no selection
				return;
			}
			long nResourceSelIndex = m_dlAptResource->TrySetSelByColumn(0, (long)m_arydwSelResourceIDs[0]);
			if (nResourceSelIndex == sriNoRow) {
				// Failed to get a row, we need to tell the user that they'll have to manually select a 
				// resource (this is just like the OnTrySetSelFinishedAptResourceCombo handler)
				m_arydwSelResourceIDs.RemoveAll();
				m_dlAptResource->PutCurSel(sriNoRow);
				EnsureResourceComboVisibility();
			}
			else {
				// Either we set the row successfully (which was our whole goal) or we got an 
				// sriNoRowYet_WillFireEvent which means the OnTrySetSelFinishedAptResourceCombo will handle it)
				CResEntryDlg_ASSERT(nResourceSelIndex == sriNoRowYet_WillFireEvent || nResourceSelIndex >= 0);
			}
		}
		else if (VarString(m_dlAptResource->Value[nCurSel][1], "") == SCHEDULER_TEXT_RESOURCE__MULTIPLE_RESOURCES)
		{
			// Init the dialog and fill with existing selections
			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "ResourceT");
			for (int i = 0; i < m_arydwSelResourceIDs.GetSize(); i++)
			{
				dlg.PreSelect(m_arydwSelResourceIDs[i]);
			}

			dlg.m_strNameColTitle = "Resource";

			// Have the user select all the resources to associate this appointment with
			HRESULT hRes;
			{
				CKeepDialogActive kda(this);
				CString strWhere = "Inactive = 0 OR ID IN (";
				for (i = 0; i < m_arydwSelResourceIDs.GetSize(); i++) {
					CString str;
					str.Format("%d,", m_arydwSelResourceIDs[i]);
					strWhere += str;
				}
				strWhere.TrimRight(',');
				strWhere += ")";
				hRes = dlg.Open("ResourceT", strWhere, "ID", "Item", "Please select the resources to associate with this appointment.", 1);
			}


			// If we said yes, update our array of resources with this information
			if (hRes == IDOK)
			{
				CDWordArray dwaResources;
				dlg.FillArrayWithIDs(dwaResources);

				//auditing
				m_schedAuditItems.aryNewResource.RemoveAll();
				for (int i = 0; i < dlg.m_aryNewName.GetSize(); i++)
					m_schedAuditItems.aryNewResource.Add(AsString(dlg.m_aryNewName[i]));
				m_schedAuditItems.aryOldResource.RemoveAll();
				for (i = 0; i < dlg.m_aryOldName.GetSize(); i++)
					m_schedAuditItems.aryOldResource.Add(AsString(dlg.m_aryOldName[i]));

				ChooseMultipleResources(true, dwaResources);
			}
			else {
				//we cancelled the selection, meaning nothing changed at all in our res, so make
				//sure we don't do any default time stuff
				bDontFill = true;
			}

			// If we're down to 1 resource, then show the combo again with the current resource selected and hide the link 
			// text, if we're multi-resource then hide the combo and show the link text.
			RefreshResourceCombo();
		}
		else
		{
			//normal resource selection

			// Selecting a single resource so empty the array and add back the id of the one selected resource
			m_arydwSelResourceIDs.RemoveAll();
			m_arydwSelResourceIDs.Add(VarLong(m_dlAptResource->Value[nCurSel][0]));
			EnsureResourceComboVisibility();

			// (c.haag 2004-04-26 13:44) PLID 12094 - If the user changes the resource, ensure
			// the purpose list is empty so that we can enforce restrictions on what purposes
			// and types we can or can't have
			// (c.haag 2008-12-26 15:31) - PLID 32376 - Filter out inactive procedures
			if (m_adwPurposeID.GetSize())
			{
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				_RecordsetPtr prsPurpose = CreateParamRecordset("select aptpurposeid from resourcepurposetypet "
					"where aptpurposeid not in (select id from proceduret where inactive = 1) "
					"and apttypeid = {INT} and resourceid = {INT}",
					GetCurAptTypeID(), VarLong(m_dlAptResource->Value[nCurSel][0]));
				CDWordArray adwNewPurposes;
				while (!prsPurpose->eof)
				{
					long nPurposeID = AdoFldLong(prsPurpose, "AptPurposeID");

					for (long i = 0; i < m_adwPurposeID.GetSize(); i++)
					{
						if (m_adwPurposeID[i] == (unsigned long)nPurposeID)
						{
							adwNewPurposes.Add(nPurposeID);
							break;
						}
					}
					prsPurpose->MoveNext();
				}
				m_adwPurposeID.RemoveAll();
				for (long i = 0; i < adwNewPurposes.GetSize(); i++)
				{
					// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
					// replaced it with TryAddPurposeIDToAppt
					TryAddPurposeIDToAppt(adwNewPurposes[i]);
				}
				EnsurePurposeComboVisibility();
			}
			if (GetCurAptTypeID() > -1)
			{
				// (a.walling 2010-10-14 15:28) - PLID 40965 - Use ReturnsRecordsParam
				if (!ReturnsRecordsParam("select top 1 apttypeid from resourcepurposetypet where apttypeid = {INT} and resourceid = {INT}",
					GetCurAptTypeID(), VarLong(m_dlAptResource->Value[nCurSel][0])))
				{
					m_dlAptType->CurSel = -1;
					m_nAptTypeID = -1;
					m_bValidAptTypeStored = false;
				}
			}

			// (c.haag 2004-03-31 14:01) PLID 11690 - Requery the type and purpose combos
			RequeryAptTypes();
			RequeryAptPurposes();

			// Fill the default duration list so that when the user chooses
			// a type and purpose, the EndTime will automatically fill in
			// with a default duration.
			RefreshDefaultDurationList();
		}

		// Assign a default end time based on the currently selected
		// type, purposes, and resources
		if (!bDontFill)
			FillDefaultEndTime();

		if (m_arydwSelResourceIDs.GetSize() > 1)
			GetDlgItem(IDC_APTTYPE_COMBO)->SetFocus();
	}
	// (c.haag 2015-11-12) - PLID 67577 - Log that we touched on this function before passing it to the caller
	NxCatchAllSilentCallThrow(
		Log("Exception being thrown from %s", __FUNCTION__);
		// Set to an invalid state
		try {
			m_arydwSelResourceIDs.RemoveAll();
			m_dlAptResource->PutCurSel(sriNoRow);
			EnsureResourceComboVisibility();
		}
		catch (...) {}
	)
}

// (a.walling 2010-06-14 14:29) - PLID 23560 - Resource Sets
void CResEntryDlg::ChooseMultipleResources(bool bFromUser, CDWordArray& dwaResources)
{
	/*
	// (a.walling 2010-06-14 14:29) - PLID 23560 - Resource Sets - This is the general auditing code when changing resources. However it is not actually used anywhere.
	// The AppointmentModify will grab the existing resources from data and create the new ones from the array passed into it.
	//auditing
	m_schedAuditItems.aryNewResource.RemoveAll();
	for(int i = 0; i < dlg.m_aryNewName.GetSize(); i++)
		m_schedAuditItems.aryNewResource.Add(AsString(dlg.m_aryNewName[i]));
	m_schedAuditItems.aryOldResource.RemoveAll();
	for(i = 0; i < dlg.m_aryOldName.GetSize(); i++)
		m_schedAuditItems.aryOldResource.Add(AsString(dlg.m_aryOldName[i]));
	*/

	ASSERT(!dwaResources.IsEmpty());

	m_arydwSelResourceIDs.Copy(dwaResources);

	EnsureResourceComboVisibility();

	// (c.haag 2004-04-26 13:44) PLID 12094 - If the user changes the resource, ensure
	// the purpose list is empty so that we can enforce restrictions on what purposes
	// and types we can or can't have.
	CString strResources = ArrayAsString(dwaResources);

	if (m_adwPurposeID.GetSize())
	{
		// (c.haag 2004-06-08 15:11) - PLID 12094, I added "having count(aptpurposeid)..." because the
		// count has to be equal to the number of resources we selected, which guarantees that every
		// resource is associated with each purpose.
		// (c.haag 2008-12-26 15:31) - PLID 32376 - Filter out inactive procedures
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized (since resources are relatively static)
		_RecordsetPtr prsPurpose = CreateParamRecordset(FormatString(
			"select aptpurposeid from resourcepurposetypet "
			"where aptpurposeid not in (select id from proceduret where inactive = 1) "
			"and apttypeid = {INT} and resourceid in (%s) group by aptpurposeid having count(aptpurposeid) = {INT}", strResources),
			GetCurAptTypeID(), m_arydwSelResourceIDs.GetSize());
		CDWordArray adwNewPurposes;
		while (!prsPurpose->eof)
		{
			long nPurposeID = AdoFldLong(prsPurpose, "AptPurposeID");
			
			for (long i=0; i < m_adwPurposeID.GetSize(); i++)
			{
				if (m_adwPurposeID[i] == (unsigned long)nPurposeID)
				{
					adwNewPurposes.Add(nPurposeID);
					break;
				}
			}
			prsPurpose->MoveNext();
		}
		m_adwPurposeID.RemoveAll();
		for (long i=0; i < adwNewPurposes.GetSize(); i++)
		{
			// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
			// replaced it with TryAddPurposeIDToAppt
			TryAddPurposeIDToAppt(adwNewPurposes[i]);
		}
		EnsurePurposeComboVisibility();
	}
	if (GetCurAptTypeID() > -1)
	{
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if (!ReturnsRecordsParam(FormatString("select top 1 apttypeid from resourcepurposetypet where apttypeid = {INT} and resourceid in (%s)", strResources), GetCurAptTypeID()))
		{
			m_dlAptType->CurSel = -1;
			m_nAptTypeID = -1;
			m_bValidAptTypeStored = false;
		}
	}

	// (c.haag 2004-03-31 14:01) PLID 11690 - Requery the type and purpose combos				
	RequeryAptTypes();	
	RequeryAptPurposes();

	//DRT 7/24/03 - Added this inside the == IDOK setup ... why would we want to do them if they cancelled?

	// Fill the default duration list so that when the user chooses
	// a type and purpose, the EndTime will automatically fill in
	// with a default duration.
	RefreshDefaultDurationList();

	if (!bFromUser) {
		RefreshResourceCombo();

		// Assign a default end time based on the currently selected
		// type, purposes, and resources
		FillDefaultEndTime();
	}
}

void CResEntryDlg::OnSelChosenAptTypeCombo(long nCurSel)
{
	try {
DEBUG_FUNCTION_PROFILING_INIT()
	// First set the color
	ObtainActiveBorderColor();
	ApplyToResBox();

	// Then apply the selection change to the purpose combo list

	//With this line of code, we state that from now on, the apt type information should be read solely
	//from the datalist (since a valid row has been selected).
	m_bValidAptTypeStored = false;
	
	long nAptTypeID = GetCurAptTypeID();

	if (nAptTypeID != -1) {
DEBUG_FUNCTION_PROFILING_CALC_TIME_HERE("a");
		// Generate the new filter based on the apt type
		m_adwPurposeID.RemoveAll();
		EnsurePurposeComboVisibility();

DEBUG_FUNCTION_PROFILING_CALC_TIME_HERE("b");
		// (c.haag 2004-03-31 13:55) PLID 11690 - This is obselete now because the filter is now
		// set in RequeryAptPurposes
/*		CString strPurposeFilter;
		strPurposeFilter.Format(
			"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li)", 
			nAptTypeID);
		m_dlAptPurpose->WhereClause = _bstr_t(strPurposeFilter);*/
		RequeryAptPurposes();

		TryAutoFillPurposes();

		EnsurePurposeComboVisibility();
		GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);

DEBUG_FUNCTION_PROFILING_CALC_TIME_HERE("c");
		///////////////////////////////////////////////////
		// If this is a block time, disable the patient
		// drop down
		if (GetCurAptTypeCategory() == PhaseTracking::AC_BLOCK_TIME)
		{
			OnBlockTimeCatSelected(TRUE);
		}
		else
		{
			OnBlockTimeCatSelected(FALSE);
			//////////////////////////////////////////////////////////////////////////////////////
			// TODO: This line of code works around a bug in the datalist where if you set another 
			// datalist to visible when this datalist is dropped down, this datalist loses focus 
			// but doesn't close up!  When the datalist bug is fixed, remove this line of code.
			GetDlgItem(IDC_APTTYPE_COMBO)->SetFocus();
			//////////////////////////////////////////////////////////////////////////////////////
		}
DEBUG_FUNCTION_PROFILING_CALC_TIME_HERE("d");
		// If this is not a block time, only enable it if it's a
		// new appointment
/*		else if (m_ResID == -1)
		{
			GetDlgItem(IDC_PATIENT_COMBO_2)->EnableWindow(TRUE);
		}*/


	} else {
		m_dlAptPurpose->Clear();
		m_dlAptPurpose->WhereClause = "";
		GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
		m_adwPurposeID.RemoveAll();
		EnsurePurposeComboVisibility();
		RefreshPurposeCombo();

		// Make sure we show the patient's combo
		OnBlockTimeCatSelected(FALSE);
		//////////////////////////////////////////////////////////////////////////////////////
		// TODO: This line of code works around a bug in the datalist where if you set another 
		// datalist to visible when this datalist is dropped down, this datalist loses focus 
		// but doesn't close up!  When the datalist bug is fixed, remove this line of code.
		GetDlgItem(IDC_APTTYPE_COMBO)->SetFocus();
		//////////////////////////////////////////////////////////////////////////////////////
	}
DEBUG_FUNCTION_PROFILING_FINISH_TRACE();

	SetModifiedFlag();

	// Assign a default end time based on the currently selected
	// type and purposes.
	FillDefaultEndTime();

	FillDefaultArrivalTime();

	}
	// (d.thompson 2009-07-07) - PLID 34803 - Extra logging to be removed when we figure out PLID 37221
	CResEntryDlg_NxCatchAllCall("Error changing appointment type.", Log("PLID 34803:  %s", CClient::PLID34803_GetPrintableCString()););
}

void CResEntryDlg::OnSelChosenAptPurposeCombo(long nCurSel)
{
	try 
	{
		// (c.haag 2015-11-12) - PLID 67577 - Call the business logic
		HandleAptPurposeComboSelection(nCurSel);
	}
	CResEntryDlg_NxCatchAll("Error changing appointment purpose.");
}

void CResEntryDlg::HandleAptPurposeComboSelection(long nCurSel)
{	
	try
	{
		CString strFrom, strWhere;
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptPurposeT");
		HRESULT hRes;
		bool bDontFill = false;

		SetModifiedFlag();

		//if (nCurSel != 1 /* 1 = Multiple purposes */)
		if (nCurSel > -1 && VarString(m_dlAptPurpose->Value[nCurSel][1], "") != SCHEDULER_TEXT_APPOINTMENT_PURPOSE__MULTIPLE_PURPOSES)
		{
			m_adwPurposeID.RemoveAll();
			m_schedAuditItems.aryNewPurpose.RemoveAll();
			if (VarLong(m_dlAptPurpose->Value[nCurSel][0], -1) > 0) {
				// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
				// replaced it with TryAddPurposeIDToAppt
				TryAddPurposeIDToAppt(VarLong(m_dlAptPurpose->Value[nCurSel][0]));
				m_schedAuditItems.aryNewPurpose.Add(VarString(m_dlAptPurpose->Value[nCurSel][1]));
			}
			EnsurePurposeComboVisibility();
		}
		else
		{
			// Fill the dialog with existing selections
			for (int i=0; i < m_adwPurposeID.GetSize(); i++)
			{
				dlg.PreSelect(m_adwPurposeID[i]);
			}

			long nAptTypeID = GetCurAptTypeID();

			dlg.m_strNameColTitle = "Purpose";

			// Have the user select all the procedures to associate this appointment with
	/*		strFrom = "AptPurposeT INNER JOIN "
				"ProcedureT ON AptPurposeT.ID = ProcedureT.ID INNER JOIN "
				"AptPurposeTypeT ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID INNER JOIN "
				"AptTypeT ON AptPurposeTypeT.AptTypeID = AptTypeT.ID";*/

	/*		strFrom = "AptPurposeT INNER JOIN "
				"AptPurposeTypeT ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID INNER JOIN "
				"AptTypeT ON AptPurposeTypeT.AptTypeID = AptTypeT.ID";*/

			strFrom = "AptPurposeT";

			CString strResources;
			if (m_arydwSelResourceIDs.GetSize() > 0)
			{
				for (long i=0; i < m_arydwSelResourceIDs.GetSize(); i++)
				{
					CString str;
					str.Format(",%d ", m_arydwSelResourceIDs[i]);
					strResources += str;
				}
				strResources = strResources.Right(strResources.GetLength() - 1);
				//TES 7/15/2004 - PLID 13466 - The purpose needs to be valid both by resource AND type.
				strWhere.Format("AptPurposeT.ID IN (select aptpurposeid from (select resourceid, aptpurposeid from resourcepurposetypet where apttypeid = %d group by resourceid, aptpurposeid) SubQ where SubQ.ResourceID in (%s) "
					" AND AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) "
					"group by aptpurposeid having count(aptpurposeid) = %d)", 
					nAptTypeID,
					strResources, 			
					nAptTypeID, m_arydwSelResourceIDs.GetSize());
			}
			else
			{
				// (c.haag 2004-04-01 16:12) - This should never happen but we should have safeguards.
				strWhere.Format(
					"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li)", 
					nAptTypeID);
			}

			// (c.haag 2008-12-29 12:27) - PLID 32376 - We now exclude inactive procedures unless they're already tied to the appointment
			strWhere += FormatString(" AND (AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) OR AptPurposeT.ID IN (%s) ) ", ArrayAsString(m_adwPurposeID));

			/*strWhere.Format("AptTypeT.ID = %d", nAptTypeID);*/

			{
				try {
					CKeepDialogActive kda(this);
					hRes = dlg.Open(strFrom, strWhere, "AptPurposeT.ID", "AptPurposeT.Name", "Please select the procedures to associate with this appointment.");
				}CResEntryDlg_NxCatchAll("Error loading multi-select purpose list");
			}

			// Update our array of procedures with this information
			if (hRes == IDOK)
			{
				dlg.FillArrayWithIDs(m_adwPurposeID);
				EnsurePurposeComboVisibility();

				//auditing
				m_schedAuditItems.aryNewPurpose.RemoveAll();
				for(int i = 0; i < dlg.m_aryNewName.GetSize(); i++)
					m_schedAuditItems.aryNewPurpose.Add(AsString(dlg.m_aryNewName[i]));
				//We filled this on loading.
				//m_schedAuditItems.aryOldPurpose.Copy(dlg.m_aryOldName);
			}
			else {
				//we don't want to fill in any default end times if we cancelled, 
				//nothing has changed!
				bDontFill = true;
			}

			// Display our new purpose list to the user
			RefreshPurposeCombo();
		}

		// Assign a default end time based on the currently selected
		// type and purposes.
		if(!bDontFill) {
			FillDefaultEndTime();
			FillDefaultArrivalTime();
		}

		if (m_adwPurposeID.GetSize() > 1)
			GetDlgItem(IDC_NOTES_BOX)->SetFocus();
	}
	// (c.haag 2015-11-12) - PLID 67577 - Log that we touched on this function before passing it to the caller
	NxCatchAllSilentCallThrow(Log("Exception being thrown from %s", __FUNCTION__))
}

void CResEntryDlg::RefreshPurposeCombo()
{
	COleVariant varNull;
	varNull.vt = VT_NULL;

	// If nothing is checked, select nothing
	if (m_adwPurposeID.GetSize() == 0)
	{
		m_dlAptPurpose->FindByColumn(0, (long)-1, 0, TRUE);
	}
	// If we only have one item checked, lets just select it
	else if (m_adwPurposeID.GetSize() == 1)
	{
		// (c.haag 2004-03-31 13:50) PLID 11690 - If the purpose does not exist in the combo, we need to add it manually.
		if (-1 == m_dlAptPurpose->FindByColumn(0, (long)m_adwPurposeID[0], 0, TRUE))
		{
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM AptPurposeT WHERE ID = {INT}", m_adwPurposeID[0]);
			if (!prs->eof)
			{
				IRowSettingsPtr pRow = m_dlAptPurpose->Row[-1];
				pRow->Value[apcID] = (long)m_adwPurposeID[0];
				pRow->Value[apcName] = prs->Fields->Item["Name"]->Value;
				// (a.walling 2008-05-08 10:35) - PLID 29786 - This used to be empty, but it should have some value
				pRow->Value[apcArrivalPrepMinutes] = _variant_t(long(0), VT_I4);
				m_dlAptPurpose->AddRow(pRow);
				if (-1 == m_dlAptPurpose->FindByColumn(0, (long)m_adwPurposeID[0], 0, TRUE))
					m_dlAptPurpose->FindByColumn(0, (long)-1, 0, TRUE); // This should never happen
			}
			else
				m_dlAptPurpose->FindByColumn(0, (long)-1, 0, TRUE); // This should probably never happen
		}
	}
	else
	{
		m_dlAptPurpose->PutCurSel(1); // We have multiple items
	}

	InvalidateRect(m_rcProcedures); // Paint the hyperlink text list
}

// (r.gonet 05/25/2012) - PLID 49059 - Sets the location template label so that it corresponds
//  with the highest priority template that this reservation is on.
void CResEntryDlg::RefreshLocationTemplate()
{
	if(!GetRemotePropertyInt("ColorApptLocationComboBox", 1, 0, GetCurrentUserName(), true)) {
		// (r.gonet 06/05/2012) - PLID 49059 - Don't color, so set the color to the defaults
		m_dlAptLocation->ComboBackColor = GetSysColor(COLOR_WINDOW);
		m_dlAptLocation->ComboForeColor = GetSysColor(COLOR_WINDOWTEXT);
		return;
	}

	CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)m_Res.GethWnd());
	CWnd* pWndSingleDay = pWndNxTextbox->GetParent();
	CNxSchedulerDlg* pWndScheduler = (CNxSchedulerDlg*)pWndSingleDay->GetParent();
	bool bSuccess = pWndScheduler->m_ResourceAvailLocations.GetLocationTemplateColor(m_Res.GetDay(), m_Res.GetStartTime(), m_nLocationTemplateColor);
	if(bSuccess) {
		// We have a template, so set the color of the location combo
		// (r.gonet 05/25/2012) - PLID 49059 - We want the location template label to look like the actual template
		long nUseSimpleForeColors = GetRemotePropertyInt("SchedulerUseSimpleForeColors", -1, 0, GetCurrentUserName(), false);
		OLE_COLOR backColor = m_nLocationTemplateColor;
		// (r.gonet 05/25/2012) - PLID 49059 - Use the same method as what SingleDay uses.
		m_dlAptLocation->ComboBackColor = backColor;
		m_dlAptLocation->ComboForeColor = ColorUtils::CalcGoodForeColor(backColor, TRUE, nUseSimpleForeColors ? TRUE : FALSE);
	} else {
		// (r.gonet 03/20/2012) - PLID 48738 - No template, so set the color to the defaults
		m_dlAptLocation->ComboBackColor = GetSysColor(COLOR_WINDOW);
		m_dlAptLocation->ComboForeColor = GetSysColor(COLOR_WINDOWTEXT);
	}
}

// This either selects the one "current" resource in the datalist, or if it's multi-resources, it clears 
// the selection.  Either way, it invalidates the region of this dialog that the datalist covers, and when 
// it is painted the datalist will either be shown or hidden, depending on whether we have one resource or 
// multiple resources (respectively).
void CResEntryDlg::RefreshResourceCombo()
{
	CRect rc;
	GetDlgItem(IDC_APTRESOURCE_COMBO)->GetWindowRect(rc);
	ScreenToClient(&rc);

	if (m_arydwSelResourceIDs.GetSize() == 1) {
		long nResourceID = GetCurResourceID_ExpectSingle();
		if (-1 == m_dlAptResource->FindByColumn(0, GetCurResourceID_ExpectSingle(), 0, TRUE)) {
			//
			// (c.haag 2005-06-29 12:53) - PLID 6400 - If we couldn't find the resource ID
			// in the list, maybe it's because it's inactive.
			//
			for (long j=0; j < m_adwInactiveResIDs.GetSize(); j++) {
				if ((long)m_adwInactiveResIDs[j] == nResourceID) {
					// 
					// (c.haag 2005-06-29 12:59) - Yep, it was inactive. Lets add the item to the
					// resource dropdown and select it
					//
					IRowSettingsPtr pRow = m_dlAptResource->GetRow(-1);
					pRow->Value[0] = nResourceID;
					pRow->Value[1] = _bstr_t(m_astrInactiveResNames[j]);
					m_dlAptResource->CurSel = m_dlAptResource->AddRow(pRow);
					break;
				}
			}
		}
	} else {
		long nCurSel = m_dlAptResource->FindByColumn(1, _bstr_t(SCHEDULER_TEXT_RESOURCE__MULTIPLE_RESOURCES), 0, VARIANT_TRUE);
		CResEntryDlg_ASSERT(nCurSel != sriNoRow);
	}

	InvalidateRect(rc); // Paint the hyperlink text list
}


void CResEntryDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	try {
		DoClickHyperlink(nFlags, point);
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling

	CNxDialog::OnLButtonDown(nFlags, point);
}

void CResEntryDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	try {
		DrawResourceHyperlinkList(&dc);
		DrawProcedureHyperlinkList(&dc);
		// (j.gruber 2012-07-30 14:36) - PLID 51830
		DrawPrimaryInsuranceText(&dc);
		DrawSecondaryInsuranceText(&dc);
	} NxCatchAllIgnore(); // We're ignoring because paint messages get processed constantly.  If we gave a message to the user, they would never be able to dismiss it.
}


void CResEntryDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	// If there's more than one resource selected and the user clicked in the resource link area, then pop up the multi-select dialog for resources
	// (j.gruber 2013-05-07 10:42) - PLID 56568 - and not cancelled
	if (IsCurResourceListMultiResource() && !(m_bIsCancelled))
	{
		CRect rc;
		GetDlgItem(IDC_APTRESOURCE_COMBO)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(point)) {
			// Open the resource checkbox list

			//DRT 11/1/02 - this has been calling the function with a 0, assuming the "Multiple resources" is at the top.  That is not always the case.
			// (b.cardillo 2003-07-07 15:12) Looks like the nID variable was meant to be called nIndex.
			long nID = m_dlAptResource->FindByColumn(1, _bstr_t(SCHEDULER_TEXT_RESOURCE__MULTIPLE_RESOURCES), 0, false);
			if (nID >= 0)
			{
				// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
				HandleAptResourceComboSelection(nID);	//fake a call to OnSelChosen giving the ID of where "Multiple Resources" is located
			}
		}
	}
	
	// If there's still more than one purpose selected and the user clicked in the procedure link area, then pop up the multi-select dialog for purposes
	// (j.gruber 2013-05-07 10:43) - PLID 56568 - and not cancelled
	if (m_adwPurposeID.GetSize() > 1 && !(m_bIsCancelled))
	{
		if (m_rcProcedures.PtInRect(point)) {
			// Open the procedure checkbox list

			//DRT 11/1/02 - this has been calling the function with a 1, assuming the "Multiple Purposes is at the top.  That is not always the cse.
			// (b.cardillo 2003-07-07 15:12) Looks like the nID variable was meant to be called nIndex.
			long nID = m_dlAptPurpose->FindByColumn(1, _bstr_t(SCHEDULER_TEXT_APPOINTMENT_PURPOSE__MULTIPLE_PURPOSES), 0, false);
			if (nID >= 0)
			{
				// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
				HandleAptPurposeComboSelection(nID);
			}
		}
	}

	// (j.gruber 2012-07-26 13:24) - PLID 51830
	if (m_rcPriIns.PtInRect(point)) {
		ChooseInsParty(priIns);
		
	}	
	if (m_rcSecIns.PtInRect(point)) {
		ChooseInsParty(secIns);		
	}

}


CString CResEntryDlg::GenerateResourceString() {

	CString strResources;

	// TODO: Optimize this later by only building a list of resources whenever
	// the user changes them
	for (long i=0; i < m_arydwSelResourceIDs.GetSize(); i++)
	{
		long lRow = m_dlAptResource->FindByColumn(0, (long)m_arydwSelResourceIDs[i], 0, FALSE);
		if (lRow != -1)
		{
			CString strThisResource = VarString(m_dlAptResource->GetValue(lRow, 1));
			if (strResources.IsEmpty())
				strResources = strThisResource;
			else
				strResources += ", " + strThisResource;
		}
		else {
			// (c.haag 2005-06-29 12:38) - PLID 6400 - If we get here, the
			// resource may be inactive. Look in our inactive resource array.
			for (long j=0; j < m_adwInactiveResIDs.GetSize(); j++) {
				if (m_adwInactiveResIDs[j] == m_arydwSelResourceIDs[i]) {
					CString strThisResource = m_astrInactiveResNames[j];
					if (strResources.IsEmpty())
						strResources = strThisResource;
					else
						strResources += ", " + strThisResource;
					break;
				}
			}
		}
	}

	return strResources;
}


void CResEntryDlg::DrawResourceHyperlinkList(CDC *pdc)
{
	CString strResources;
	CRect rc;

	if (!IsCurResourceListMultiResource())
	{
		// (c.haag 2005-02-15 10:16) - PLID 15623 - What a horrible place to show the resource dropdown!!
		// If we should not draw the hyperlink, just quit.

		//CWnd* pFocus = GetFocus();
		//GetDlgItem(IDC_APTRESOURCE_COMBO)->ShowWindow(SW_SHOW);
		// The showwindow messed with the focus; so set it back
		//if (pFocus) pFocus->SetFocus();
		return;
	}
	GetDlgItem(IDC_APTRESOURCE_COMBO)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APTRESOURCE_COMBO)->GetWindowRect(rc);
	ScreenToClient(&rc);

	strResources = GenerateResourceString();

	
	pdc->FillSolidRect(rc, GetSolidBackgroundColor());
	// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent

	// (j.gruber 2013-05-07 10:43) - PLID 56568 - gray out the hyperlink if its cancelled
	EDrawTextOnDialogStyle dtsStyle = dtsHyperlink;
	if (m_bIsCancelled) {
		dtsStyle = dtsDisabledHyperlink;
	}
	DrawTextOnDialog(this, pdc, rc, strResources, dtsStyle, false, DT_LEFT, true, false, 0);
}

void CResEntryDlg::DrawProcedureHyperlinkList(CDC *pdc)
{
	CString strProcedures;
	CRect rc = m_rcProcedures;

	if (m_adwPurposeID.GetSize() <= 1)
	{
		// (c.haag 2005-02-15 10:16) - PLID 15623 - What a horrible place to show the purpose dropdown!!
		// If we should not draw the hyperlink, just quit.

		//CWnd* pFocus = GetFocus();
		//GetDlgItem(IDC_APTPURPOSE_COMBO)->ShowWindow(SW_SHOW);
		// The showwindow messed with the focus; so set it back
		//if (pFocus) pFocus->SetFocus();
		return;
	}
	GetDlgItem(IDC_APTPURPOSE_COMBO)->ShowWindow(SW_HIDE);

	// TODO: Optimize this later by only building a list of procedures whenever
	// the user changes them
	for (long i=0; i < m_adwPurposeID.GetSize(); i++)
	{
		long lRow = m_dlAptPurpose->FindByColumn(0, (long)m_adwPurposeID[i], 0, FALSE);
		// (c.haag 2004-04-26 13:07) - PLID 11690 - The purpose may be missing if it is not an acceptable
		// resource-type-purpose configuration, so we will add those to the purpose dropdown and the hyperlink. 
		//
		// Note to developers: You should otherwise avoid accessing records in a paint function. It's bad
		// practice.
		//
		if (lRow == -1)
		{
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM AptPurposeT WHERE ID = {INT}", m_adwPurposeID[i]);
			if (!prs->eof)
			{
				IRowSettingsPtr pRow = m_dlAptPurpose->Row[-1];
				pRow->Value[apcID] = (long)m_adwPurposeID[i];
				pRow->Value[apcName] = prs->Fields->Item["Name"]->Value;
				// (a.walling 2008-05-08 10:35) - PLID 29786 - This used to be empty, but it should have some value
				pRow->Value[apcArrivalPrepMinutes] = _variant_t(long(0), VT_I4);
				m_dlAptPurpose->AddRow(pRow);
				lRow = m_dlAptPurpose->FindByColumn(0, (long)m_adwPurposeID[i], 0, TRUE);
			}
		}
		if (lRow != -1)
		{
			CString strThisProcedure = VarString(m_dlAptPurpose->GetValue(lRow, 1));
			if (strProcedures.IsEmpty())
				strProcedures = strThisProcedure;
			else
				strProcedures += ", " + strThisProcedure;
		}
	}

	pdc->FillSolidRect(rc, GetSolidBackgroundColor());

	// (j.gruber 2013-05-07 10:45) - PLID 56568 - disalbe the hyperlink if its cancelled
	EDrawTextOnDialogStyle dtsStyle = dtsHyperlink;
	if (m_bIsCancelled) {
		dtsStyle = dtsDisabledHyperlink;
	}
	// (j.jones 2008-05-01 16:38) - PLID 29874 - Set background color to transparent
	DrawTextOnDialog(this, pdc, rc, strProcedures, dtsStyle, false, DT_LEFT, true, false, 0);
}

BOOL CResEntryDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (j.gruber 2013-05-16 16:53) - PLID 56568 - don't switch if its cancelled
		if (IsCurResourceListMultiResource() && !m_bIsCancelled)
		{
			CRect rc;
			GetDlgItem(IDC_APTRESOURCE_COMBO)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (j.gruber 2013-05-16 16:53) - PLID 56568 - don't switch if its cancelled
		if (m_adwPurposeID.GetSize() > 1 && !m_bIsCancelled)
		{
			if (m_rcProcedures.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		// (j.gruber 2012-07-26 13:18) - PLID 51830 - add our labels
		// (j.gruber 2013-05-16 16:53) - PLID 56568 - don't switch if its cancelled
		CRect rcPriIns;
		GetDlgItem(IDC_RES_PRI_LABEL)->GetWindowRect(rcPriIns);
		ScreenToClient(&rcPriIns);

		if (rcPriIns.PtInRect(pt) && GetCurPatientID() != -25 && !m_bIsCancelled) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
		// (j.gruber 2013-05-16 16:53) - PLID 56568 - don't switch if its cancelled
		CRect rcSecIns;
		GetDlgItem(IDC_RES_SEC_LABEL)->GetWindowRect(rcSecIns);
		ScreenToClient(&rcSecIns);

		if (rcSecIns.PtInRect(pt) && GetCurPatientID() != -25 && !m_bIsCancelled) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CResEntryDlg::IncActiveCount()
{
	m_nKeepActiveCount++;
}

void CResEntryDlg::DecActiveCount()
{
	m_nKeepActiveCount--;
}

// Delete the m_Res object from the screen (this means decrement our reference, but 
// ALSO tell the IReservation object to take itself out of whatever Singleday it's in)
void CResEntryDlg::KillThisResObject(BOOL bDetach /* = TRUE */)
{
	CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
	// Get the active sheet
	CNxDialog *pActiveSheet = m_pSchedulerView->GetActiveSheet();
	
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging

	// Make sure the active sheet exists and is a CNxSchedulerDlg
	CResEntryDlg_ASSERT(pActiveSheet->GetSafeHwnd() /*TODO: CNxSchedulerDlg needs to support IsKindOf    && pActiveSheet->IsKindOf(RUNTIME_CLASS(CNxSchedulerDlg))*/);
	if (pActiveSheet->GetSafeHwnd() /*TODO: CNxSchedulerDlg needs to support IsKindOf    && pActiveSheet->IsKindOf(RUNTIME_CLASS(CNxSchedulerDlg))*/) {

		// We now know it's a valid CNxSchedulerDlg object, we can cast it and post our message
		CNxSchedulerDlg *pNxSchedulerDlg = (CNxSchedulerDlg *)pActiveSheet;

		// Post a message to delete the reservation; we would do it right here (m_Res->Delete()) but 
		// JUST in case there are any messages pending that are destined for the m_Res window, we 
		// should post the message so that it only gets process after all pending messages.

		// (j.luckoski 2012-05-02 16:26) - PLID 11597 - If past bDetach, then detach res, otherwise don't
		if(bDetach) {
			// (j.luckoski 2012-04-30 08:40) - PLID 11597 - Hide the detach as the page refresh takes care of removal if they
			// don't have the preference to show cancelled appts on.
			pNxSchedulerDlg->PostMessage(NXM_RESERVATION_DELETE, 0, (LPARAM)m_Res.Detach());

			// Set m_Res to NULL (which is just for code completeness, since the m_Res.Detach() above 
			// actually sets m_Res to NULL for us)
			m_Res.SetDispatch(NULL);
		}

	} else {
	//	
	// Well we couldn't get a good CNxSchedulerDlg window, so just perform the delete 
	// directly (which should be safe, because it's only NOT safe if there are messages 
	// waiting for the reservation window)
	m_Res.DeleteRes(__FUNCTION__);
		
	// And of course in this case, we need to remove OUR reference
	m_Res.SetDispatch(NULL);
	}
}

void CResEntryDlg::RefreshDefaultDurationList()
{
	//JMJ - removed 11/10/2003
	//if (!IsSurgeryCenter())
		//return;

	try
	{
		// Clear the existing list
		while (m_aDurationGroups.GetSize())
		{
			CDurationGroup* p = m_aDurationGroups.GetAt(0);
			delete p;
			m_aDurationGroups.RemoveAt(0);
		}

		CArray<long,long> arynResourceIDs;
		for(int nResourceIndex = 0; nResourceIndex < m_arydwSelResourceIDs.GetSize(); nResourceIndex++) {
			DWORD dwResourceID = m_arydwSelResourceIDs.GetAt(nResourceIndex);
			arynResourceIDs.Add(dwResourceID);
		}

		// (z.manning 2011-05-06 16:29) - PLID 43601 - Moved this code to a utility function.
		FillDefaultDurationGroups(&m_aDurationGroups, &arynResourceIDs);

	} CResEntryDlg_NxCatchAll("Error refreshing default duration list");
}

void CResEntryDlg::FillDefaultEndTime()
{
	//JMJ - removed 11/10/2003
	//if (!IsSurgeryCenter())
		//return;

	if (IsEvent()) // Don't do default durations for events!
		return;

	CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
	// (z.manning, 05/02/2007) - PLID 25882 - If we have a template ID it means we are creating the appointment
	// from a precision template so we should use its duration and not default durations.
	if(m_Res.GetTemplateItemID() != -1) {
		return;
	}

	// Preserve the data of every control because we will be calling UpdateData(FALSE)
	// at the end of this function to update the times
	UpdateData(TRUE);


	long nAptTypeID = GetCurAptTypeID();

	// CAH 5/7/2003: Start by assigning the default duration per the type itself
	long nDuration = GetCurAptTypeDuration();
	if (nDuration > 0)
	{
		COleDateTime dt = m_nxtStart->GetDateTime();
		COleDateTime dtNew = dt + COleDateTimeSpan(0,0,nDuration,0);
		if(EnsureValidEndTime(dtNew)) {
			m_nxtEnd->SetDateTime(dtNew);
		}
	}

 	// Traverse each group to find the first one where all of the
	// purposes are an exact match.
	BOOL bDurationGroupFound = FALSE;
	for (int i=0; i < m_aDurationGroups.GetSize(); i++)
	{
		CDurationGroup* pGroup = m_aDurationGroups[i];
		if(m_adwPurposeID.GetSize() != pGroup->GetPurposeCount())
			continue;
		if (pGroup->GetAptTypeID() != nAptTypeID)
			continue;
		for (int iPurpose=0; iPurpose < m_adwPurposeID.GetSize(); iPurpose++)
		{
			if (!pGroup->InGroup(m_adwPurposeID[iPurpose]))
				break;
		}
		if (iPurpose == m_adwPurposeID.GetSize())
		{
			// Yes, we have a perfect match!
			bDurationGroupFound = TRUE;

			if (pGroup->GetDuration() >= 0) // Only change with a non-negative duration (safeguard)
			{
				COleDateTime dtNew = m_nxtStart->GetDateTime() + COleDateTimeSpan(0,0,pGroup->GetDuration(),0);
				if(EnsureValidEndTime(dtNew)) {
					m_nxtEnd->SetDateTime(dtNew);
				}
			}			
			break;
		}
	}

	//(z.manning, PLID 16692, 07/11/05)
	//If no duration group is found for selected purposes, then add the individual default durations for each purpose.
	//(If there's only 1 purpose, there's no need to execute this code as default duration would have already been set.)
	if(!bDurationGroupFound && m_adwPurposeID.GetSize() > 1) {
		long nMaxDuration = 0;
		long nTotalDuration = 0;
		for(int nPurposeCount = 0; nPurposeCount < m_adwPurposeID.GetSize(); nPurposeCount++) {
			for(int nGroupCount = 0; nGroupCount < m_aDurationGroups.GetSize(); nGroupCount++) {
				if(m_aDurationGroups[nGroupCount]->GetPurposeCount() == 1
					&& m_aDurationGroups[nGroupCount]->GetAptTypeID() == nAptTypeID
					&& m_aDurationGroups[nGroupCount]->InGroup(m_adwPurposeID[nPurposeCount]) )
				{
					long nCurrentDuration = m_aDurationGroups[nGroupCount]->GetDuration();
					nTotalDuration = nTotalDuration + nCurrentDuration;
					if(nCurrentDuration > nMaxDuration) {
						nMaxDuration = nCurrentDuration;
					}
				}
			}
		}
		if(GetRemotePropertyInt("AddApptDefaultDurations", 1, 0, "<None>", TRUE) == 0) {
			// (z.manning, 12/09/05, PLID 18511)
			// Then we don't want to add durations, so let's just use the maximum single duration.
			nTotalDuration = nMaxDuration;
		}
		if(nTotalDuration > 0) {
			COleDateTime dtStart = m_nxtStart->GetDateTime();
			COleDateTime dtNew = dtStart + COleDateTimeSpan(0,0,nTotalDuration,0);
			if(EnsureValidEndTime(dtNew)) {
				m_nxtEnd->SetDateTime(dtNew);
			}
		}
	}

	UpdateData(FALSE);
}

void CResEntryDlg::FillDefaultArrivalTime()
{
	try {

		UpdateData(TRUE);
		m_nxtArrival->SetDateTime(m_nxtStart->GetDateTime());
		int nMinsToSubtract = 0;

		if(IsEvent()) {
			return;
		}

		// First, let's try to set arrival time based on the procedures prep time if it's a procedural appt.
		if( IsAptTypeCategorySurgical(GetCurAptTypeCategory()) ) {
			int nPurposeCount = m_adwPurposeID.GetSize();
			int nMaxMins = 0;
			for(int i = 0; i < nPurposeCount; i++) {
				_variant_t var = m_dlAptPurpose->Value[m_dlAptPurpose->FindByColumn(apcID, (long)m_adwPurposeID.GetAt(i), 0, VARIANT_FALSE)][apcArrivalPrepMinutes];
				// (a.walling 2008-05-08 08:53) - PLID 29786 - This field may be empty
				if (var.vt != VT_EMPTY) {
					int nCurMins = VarLong(var, 0);
					nMinsToSubtract += nCurMins;
					if(nCurMins > nMaxMins) {
						nMaxMins = nCurMins;
					}
				}
			}
			if(nMinsToSubtract != nMaxMins) {
				// (z.manning, 4/27/2006, PLID 20195) - Check the preference to see if we want the sum or a max.
				if(GetRemotePropertyInt("AddPrepTimes", 1, 0, "<None>", true) == 0) {
					nMinsToSubtract = nMaxMins;
				}
			}
		}

		// If that didn't work, let's try the type
		if(nMinsToSubtract <= 0 && m_dlAptType->GetCurSel() > 0) {
			nMinsToSubtract = VarLong(m_dlAptType->Value[m_dlAptType->CurSel][atcDefaultArrivalMins]);
		}

		COleDateTime dtMinsToSubtract;
		int nHours = nMinsToSubtract / 60;
		int nMins = nMinsToSubtract % 60;
		dtMinsToSubtract.SetTime(nHours, nMins, 0);
		if(dtMinsToSubtract.GetStatus() == COleDateTime::valid) {
			COleDateTime dtNewArrival = m_nxtStart->GetDateTime() - dtMinsToSubtract;
			if(EnsureValidArrivalTime(dtNewArrival)) {
				m_nxtArrival->SetDateTime(dtNewArrival);
			}
		}

		UpdateData(FALSE);
	
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::FillDefaultArrivalTime");
}


void CResEntryDlg::OnSaveOpenCaseHistory()
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379

		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					CKeepDialogActive kda(this);
					MessageBox("Please close all open Case Histories before continuing.");
					return;
				}
			}
		}

		if(!IsSurgeryCenter(true)) {
			CKeepDialogActive kda(this);
			MsgBox("A Case History can only be accessed if you have purchased the Surgery Center license.\n"
				"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		long nApptID = m_Res.GetReservationID();

		//save the appt.
		m_PromptForCaseHistory = false;

		// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
		// action is being performed after saving the appointment
		ValidateSaveClose(asaOpenCaseHistory);

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ID FROM CaseHistoryT WHERE AppointmentID = {INT}",nApptID);
		if(!rs->eof) {
			CCaseHistoryDlg dlg(this);
			dlg.OpenExistingCase(AdoFldLong(rs, "ID"));
		}
		else {
			AfxMessageBox("No case histories are associated with this appointment.");
		}

		m_PromptForCaseHistory = true;

	}CResEntryDlg_NxCatchAll("Error opening case history.");
}

// (r.gonet 09/20/2013) - PLID 58416 - Save the appointment and open the medication history window.
void CResEntryDlg::OnSaveViewMedicationHistory()
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
		if(!ValidateSaveClose(asaViewMedicationHistory)) {
			return;
		}

		long nPatientID = GetCurPatientID();
		if(nPatientID == -25) {
			// (r.gonet 09/20/2013) - PLID 58416 - Med History is patient specific.
			return;
		}

		// (r.gonet 09/20/2013) - PLID 58416 - Check for the surescripts erx license and the permission
		// to read med history.
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptSureScripts
			|| !CheckCurrentUserPermissions(bioPatientMedicationHistory, sptRead)) 
		{
			return;
		}

		CMedicationHistoryDlg dlg(this, nPatientID);
		dlg.DoModal();

	}CResEntryDlg_NxCatchAll("Error opening medication history.");
}

// (j.jones 2007-11-21 10:59) - PLID 28147 - added ability to create an inventory allocation
void CResEntryDlg::OnSaveCreateInvAllocation()
{
	try {

		// Get the proper patient ID
		long nPatientID = GetCurPatientID();

		if(nPatientID == -25) {
			//do not give a warning
			return;
		}

		{
			//keep active while checking license & permissions, incase they get any
			//message boxes, in any of the functions (plus the one here)
			CKeepDialogActive kda(this);

			// (a.walling 2008-03-21 12:24) - PLID 28946 - Check for adv inventory licensing
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32851 - Renamed to 'Consignment and Allocation' from 'Advanced Inventory'
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(!g_pLicense->HasCandAModule(CLicense::cflrUse) || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {
				MsgBox("An Inventory Allocation can only be created if you have purchased the Consignment and Allocation license.\n"
					"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
				return;
			}

			if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
				return;
			}
		}

		//save the appt.
		// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
		// action is being performed after saving the appointment
		ValidateSaveClose(asaCreateInvAllocation);

		//and now create the allocation

		CWaitCursor pWait;

		long nApptID = m_ResID;
		if(m_NewRes && m_nNewResID != -1) {
			nApptID = m_nNewResID;
		}

		CInvPatientAllocationDlg dlg(this);
		dlg.m_nDefaultPatientID = nPatientID;
		dlg.m_nDefaultAppointmentID = nApptID;
		dlg.m_nDefaultLocationID = GetCurLocationID();
		dlg.DoModal();

	}CResEntryDlg_NxCatchAll("Error creating inventory allocation.");
}

// (j.jones 2008-03-18 13:56) - PLID 29309 - added ability to create an inventory order
void CResEntryDlg::OnSaveCreateInvOrder()
{
	try {

		// Get the proper patient ID
		long nPatientID = GetCurPatientID();

		if(nPatientID == -25) {
			//do not give a warning
			return;
		}

		{
			//keep active while checking license & permissions, incase they get any
			//message boxes, in any of the functions (plus the one here)
			CKeepDialogActive kda(this);

			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-26) - PLID 32851 - Renamed to 'Consignment and Allocation' from 'Advanced Inventory'
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(!g_pLicense->HasCandAModule(CLicense::cflrUse) || !g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse)) {
				MsgBox("An Inventory Order can only be created from an appointment if you have purchased the Consignment and Allocation license.\n"
					"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
				return;
			}

			if(!UserPermission(InventoryModuleItem)
				|| !CheckCurrentUserPermissions(bioInvOrder, sptCreate)) {
				return;
			}
		}

		//save the appt.
		// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
		// action is being performed after saving the appointment
		ValidateSaveClose(asaCreateInvOrder);

		//and now create the order

		CWaitCursor pWait;

		long nApptID = m_ResID;
		if(m_NewRes && m_nNewResID != -1) {
			nApptID = m_nNewResID;
		}

		// (j.jones 2008-03-18 13:57) - PLID 29309 - cancel if there is already an order linked to this appt.
		// (j.jones 2008-09-24 16:37) - PLID 31493 - changed to allow multiple orders, but warn first
		_RecordsetPtr rsOrders = CreateParamRecordset("SELECT ID FROM OrderAppointmentsT "
			"WHERE AppointmentID = {INT}", nApptID);
		if(!rsOrders->eof) {
			CString strWarning = "There is already an order linked to this appointment. "
				"Are you sure you wish to create an additional order linked to this appointment?";
			if(rsOrders->GetRecordCount() > 1) {
				strWarning = "There are already multiple orders linked to this appointment. "
					"Are you sure you wish to create an additional order linked to this appointment?";
			}

			int nRes = MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO);
			if(nRes != IDYES) {
				return;
			}
		}
		rsOrders->Close();

		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if(p) {
			if(p->FlipToModule(INVENTORY_MODULE_NAME)) {

				pView = (CNxTabView *)p->GetOpenView(INVENTORY_MODULE_NAME);
				if (pView)  {
					if(pView->GetActiveTab() == InventoryModule::OrderTab) {
						pView->UpdateView();
					}
					else {
						pView->SetActiveTab(InventoryModule::OrderTab);
					}

					((CInvOrderDlg *)pView->GetActiveSheet())->CreateOrder(FALSE, nApptID, GetCurLocationID());
				}
			}
		}

	}CResEntryDlg_NxCatchAll("Error creating inventory order.");
}

// (j.jones 2008-06-24 10:05) - PLID 30455 - added ability to create a bill
void CResEntryDlg::OnSaveCreateBill()
{
	try {

		// Get the proper patient ID
		long nPatientID = GetCurPatientID();

		if(nPatientID == -25) {
			//do not give a warning
			return;
		}

		{
			//keep active while checkingpermissions, incase they get any
			//message boxes, in any of the functions (plus the one here)
			CKeepDialogActive kda(this);

			if(!CheckCurrentUserPermissions(bioBill, sptCreate)) {
				return;
			}
			
			// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
			if (GetMainFrame()->IsBillingModuleOpen(true)) {
				return;
			}
		}

		//save the appt.
		// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
		// action is being performed after saving the appointment
		if(!ValidateSaveClose(asaCreateBill)) {
			return;
		}

		CWaitCursor pWait;

		long nApptID = m_ResID;
		if(m_NewRes && m_nNewResID != -1) {
			nApptID = m_nNewResID;
		}

		//now that it is saved, confirm this appointment exist in the bill list?
		CString str;
		// (j.gruber 2010-07-21 09:28) - PLID 39739 - fix the ID
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		str.Format("%s AND Q.ID = {INT}", GetBillAppointmentQuery());
		// (z.manning 2008-12-10 15:18) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors
		if(!ReturnsRecordsParam(str, nApptID)) {
			//explain why the appt. may not be billable
			// (j.gruber 2010-07-20 15:57) - PLID 39739 - added types as billable
			AfxMessageBox("This appointment is not able to be billed. Billable appointments are those that are for a procedural type "
				"(Office Procedure, Surgery, etc.) and have a purpose that has service codes or inventory items linked to it "
				"or are those that have a non-procedural appointment type and have service codes or inventory items linked to their appointment type. "
				"The appointment also must not be marked as a no-show.");
			return;
		}

		//and now create the bill

		CMainFrame *pMainFrame = GetMainFrame();

		if(pMainFrame) {
			//switch to the patients module, switch to that patient, and open a bill

			//Set the active patient
			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			
			//TES 1/7/2010 - PLID 36761 - No need to check for failure, we wouldn't have been able to open the ResEntryDlg
			// if we couldn't access this patient.
			pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID);
			
			//Now just flip to the patient's module and set the active Patient
			if(pMainFrame->FlipToModule(PATIENT_MODULE_NAME)) {
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView) {
					pView->UpdateView();
				}

				//now open a bill
				CPatientView* pPatView = (CPatientView*)pMainFrame->GetOpenView(PATIENT_MODULE_NAME);
				// (z.manning 2008-09-04 11:08) - PLID 31252 - Switch to the billing tab.
				pView->SetActiveTab(PatientsModule::BillingTab);
				CBillingModuleDlg *pBillingDlg = pPatView->GetBillingDlg();

				pBillingDlg->m_pFinancialDlg = pPatView->GetFinancialDlg();
				pBillingDlg->m_nPatientID = GetActivePatientID();
				pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);

				pBillingDlg->PostMessage(NXM_BILL_APPOINTMENT, (long)nApptID);
			}
		}
		else {
			AfxThrowNxException("Cannot Open MainFrame");
		}

	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSaveCreateBill");
}

void CResEntryDlg::OnSaveCreateCaseHistory()
{
	try {
		CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379

		if(!IsSurgeryCenter(true)) {
			CKeepDialogActive kda(this);
			MsgBox("A Case History can only be created if you have purchased the Surgery Center license.\n"
				"Please contact NexTech Sales if you wish to add this module.",MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		if(DoesCaseHistoryExist(m_Res.GetReservationID())) {
			OnSaveOpenCaseHistory();
			return;
		}

		// Get the proper patient ID
		long nPatientID = GetCurPatientID();

		// Block times do not have patients
		if (m_dlAptType->CurSel > 0)
		{
			//DRT 11/1/02 - Changed this to make sure we have a valid type before doing the VarShort, it was failing on "No Appointment Type"
			short nCat = GetCurAptTypeCategory();
			if (nCat == PhaseTracking::AC_BLOCK_TIME)
			{
				nPatientID = -25;
			}
		}

		if(nPatientID == -25)
			//do not give a warning
			return;

		COleDateTime dtDate = GetDate();

		//see if itis a surgery type
		short nCat = GetCurAptTypeCategory();
		if (nCat == PhaseTracking::AC_SURGERY) {

			//make a case history

			//save the appt.
			m_PromptForCaseHistory = false;
			// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
			// action is being performed after saving the appointment
			ValidateSaveClose(asaCreateCaseHistory);

			CString strCaseHistoryName;
			long nProviderID; //we used to call CalcDefaultCaseHistoryProvider, but is now in the ChooseSurgery
			// (c.haag 2007-03-09 11:13) - PLID 25138 - Include the patient ID as well
			// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
			// (j.jones 2009-08-31 15:07) - PLID 35378 - we now allow multiple preference cards to be chosen
			CArray<long, long> arynPreferenceCardIDs;
			// (j.jones 2009-08-31 17:54) - PLID 17734 - this now takes in the appointment ID
			if(CCaseHistoryDlg::ChoosePreferenceCards(arynPreferenceCardIDs, strCaseHistoryName, nProviderID, m_nCurPatientID, m_ResID)) {
				
				// (j.jones 2006-12-18 15:30) - PLID 23578 - just open it and allow them to change things before saving
				CCaseHistoryDlg dlg(this);
				// (j.jones 2009-08-26 09:52) - PLID 34943 - we no longer pass in location ID, a preference inside this function will calculate it
				dlg.OpenNewCase(nPatientID, arynPreferenceCardIDs, strCaseHistoryName, nProviderID, dtDate, m_ResID);
				
				/*
				// Create the case history for this patient based on the given surgery (this also puts it in MailSent automatically)
				long nCaseHistoryID = CCaseHistoryDlg::CreateCaseHistory(nPatientID, nSurgeryID, strName, nProviderID, GetCurrentLocationID(), dtDate, m_ResID);

				// Now that it's been created, see if they want to edit it
				int nAskEditResult = AfxMessageBox(
					"The Case History has been created.  Would you like to edit it now?",
					MB_ICONQUESTION|MB_YESNO);
				if (nAskEditResult == IDYES) {
					// Show the new case history on screen, allowing it to be edited
					CCaseHistoryDlg dlg;
					dlg.OpenExistingCase(nCaseHistoryID);
				}
				*/
			}
			m_PromptForCaseHistory = true;
		}
		else {

			int nResult;

			{
				CKeepDialogActive kda(this);
				nResult = MsgBox(MB_ICONQUESTION|MB_YESNO, "You must save a surgery appointment in order to create a case history.\n"
					"Do you wish to continue saving without creating a case history?");
			}

			if (nResult == IDNO)
			{
				return;
			}
			else
			{
				//save the appt.
				// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
				HandleSaveAndCloseReservation();
			}
		}

	}CResEntryDlg_NxCatchAll("Error creating case history.");
}

// (j.jones 2007-11-21 11:27) - PLID 28147 - converted to use enums
LRESULT CResEntryDlg::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {
			case hkSave: //Save
				OnOK();
				return TRUE;
				break;		

			case hkSaveGoToPatient:	//Save & Go To Patient
				OnGotopatient();
				return TRUE;
				break;

			// (j.jones 2007-11-21 11:28) - PLID 28147 - added allocations
			case hkSaveCreateInvAlloc:	//Save & Create Inventory Allocation
				OnSaveCreateInvAllocation();
				return TRUE;
				break;

			// (j.jones 2008-03-18 13:52) - PLID 29309 - added ability to create inv. orders
			case hkSaveCreateInvOrder:	//Save & Create Inventory Order
				OnSaveCreateInvOrder();
				return TRUE;
				break;

			// (j.jones 2008-06-24 10:05) - PLID 30455 - added ability to create a bill
			case hkSaveCreateBill:	//Save & Create Bill
				OnSaveCreateBill();
				return TRUE;
				break;

			case hkSaveCreateCaseHist:	//Save & Create Case History
				if(DoesCaseHistoryExist(m_Res.GetReservationID()))
					OnSaveOpenCaseHistory();
				else
					OnSaveCreateCaseHistory();
				return TRUE;
				break;

			// (r.gonet 09/03/2013) - PLID 58416 - Added ability to view the medication history from the appointment.
			case hkSaveViewMedicationHistory: // Save & View Medication History
				OnSaveViewMedicationHistory();
				return TRUE;
				break;

			case hkSaveCreateNewPat:	//Create a New Patient
				OnBtnNewpatient();
				return TRUE;
				break;

			case hkChangePatient:	//Change the Patient
				OnBtnChangepatient();
				return TRUE;
				break;

			case hkApptLinking:	//Appointment Linking
				OnBtnAptLinking();				
				return TRUE;
				break;

			case hkShowMoreInfo:	//Show More Info
				OnExtraInfoBtn();
				return TRUE;
				break;
		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling

	return 0;
}

void CResEntryDlg::OnTrySetSelFinishedAptResourceCombo(long nRowEnum, long nFlags) 
{
	try {
		if (nFlags == dlTrySetSelFinishedSuccess) {
			// We can use VarLong because the TrySetSel should never be called on the AptResourceCombo if the value 
			// being set was "multiple resources" or "no selection".  Therefore the ID column of the selected row 
			// would have to be non-null.
			long nResourceID = VarLong(m_dlAptResource->GetValueByEnum(nRowEnum, 0));
			m_arydwSelResourceIDs.RemoveAll();
			m_arydwSelResourceIDs.Add(nResourceID);
			EnsureResourceComboVisibility();
		} else { // (nFlags == dlTrySetSelFinishedSuccess)
			// The only way this can happen is if the resource id we tried to set at the requery never appeared in 
			// the list, which we know can only mean one thing (because this list is an unfiltered list of ALL 
			// resources): that the resource has been deleted from the system.  This is super-rare, but it can 
			// indeed happen.  So give the user a nice little message and clear out the selection so they have 
			// to select a resource before committing the appointment.
			m_arydwSelResourceIDs.RemoveAll();
			m_dlAptResource->PutCurSel(sriNoRow);
			EnsureResourceComboVisibility();
		}
	} CResEntryDlg_NxCatchAllCall("CResEntryDlg::OnTrySetSelFinishedAptResourceCombo", {
		try {
			m_arydwSelResourceIDs.RemoveAll();
			m_dlAptResource->PutCurSel(sriNoRow);
			EnsureResourceComboVisibility();
		} catch (...) { };
	});
}

void CResEntryDlg::SetPtComboWhereClause()
{
	CString strWhere = "PersonID > 0";
	if(GetRemotePropertyInt("SchedShowActivePtOnly", 0, 0, "<None>", true)) 
		strWhere += " AND Archived = 0";

	strWhere += " AND CurrentStatus <> 4";

	if (m_dlAptPatient != NULL)
		m_dlAptPatient->WhereClause = (LPCTSTR)strWhere;
}

CString CResEntryDlg::GetPatientFromClause()
{
	// (z.manning 2010-10-28 11:46) - PLID 41129 - Made a global function for this
	return ::GetPatientFromClause();
}

void CResEntryDlg::OnTrySetSelFinishedLocationCombo(long nRowEnum, long nFlags)
{
	// OK, the idea here is to behave exactly as if we had just finished a call to "SetSelByColumn"
	try {
		if (nFlags == dlTrySetSelFinishedSuccess) {
			// The current selection must be something because we were given dlTrySetSelFinishedSuccess!
			if (nRowEnum) {
				// TODO: It would be nice to be able to assert that the nRowEnum row was the same row as the 
				// CurSel, but the datalist doesn't offer the ability to get the row index of an IRowSettings 
				// object yet.
				ReflectCurLocation();
			} else {
				// No selection, this is bad news; how could this be since we were given dlTrySetSelFinishedSuccess?
				CResEntryDlg_ASSERT(FALSE);
				ReflectCurLocationFailure();
			}
		} else {
			// It wasn't found, which we interpret to mean the location must be inactive (because it's not in the datalist)
			ReflectCurLocationInactive();
		}
	} CResEntryDlg_NxCatchAll("CResEntryDlg::OnTrySetSelFinishedLocationCombo");
}

// (r.gonet 06/05/2012) - PLID 49059 - Converted to DL2
void CResEntryDlg::OnSelChosenAptLocationCombo(LPDISPATCH lpCurSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpCurSel);
		// The current selection must be something because we were given dlTrySetSelFinishedSuccess!
		if (pRow) {
			// Store the new selection back into our official variables
			ReflectCurLocation();
		} else {
			// This actually does happen legally, if the current location is an inactive location and 
			// therefore is not represented by any row in the datalist, and the user drops down and 
			// just hits the enter key without changing the selection.
			// So the best we can do is assert that the combo box test is in use, which means we have an inactive location
			CResEntryDlg_ASSERT(m_dlAptLocation->GetIsComboBoxTextInUse() != VARIANT_FALSE);
		}
		SetModifiedFlag();
	} CResEntryDlg_NxCatchAll("CResEntryDlg::OnSelChosenAptLocationCombo");
}

LRESULT CResEntryDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		if (wParam == NetUtils::PatCombo) {
			try {
				//if the patients combo needs requeried
				if (lParam != -1) {
					// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
					UpdatePatient(lParam);
				}
				else {
					if (!IsWindowVisible())
					{
						//only update if the resentry exists and is NOT visible
						Log("CResEntryDlg::OnTableChanged:  Requerying all persons.");
						RequeryPersons();
					}
				}
			} CResEntryDlg_NxCatchAll("Error in OnTableChanged::PatCombo");
		}
		else if (wParam == NetUtils::AppointmentsT)
		{
			// (j.jones 2014-08-12 15:30) - PLID 63200 - this is now Ex-only, and should no longer be sent
			// as a conventional tablechecker
		}
		else if (wParam == NetUtils::DefaultDuration)
		{
			try {
				// Refresh the ResEntry appointment type combo to reflect the new
				// default durations for appointment types
				RequeryAptTypes();
			} CResEntryDlg_NxCatchAll("Error in ResEntryDlg::OnTableChanged:DefaultDuration");
		}
		else if (wParam == NetUtils::PatG1 || wParam == NetUtils::PatBal)
		{
			try {
				// (a.walling 2013-12-12 16:51) - PLID 60004 - Don't ApplyToExtraInfo unless the patient ID matches
				if (lParam >= 0 && lParam == m_nCurPatientID)
				{
					if (IsWindowVisible())
					{
						ApplyToExtraInfo();
					}
				}
			} CResEntryDlg_NxCatchAll("Error in ResEntryDlg::OnTableChanged:PatientDemographics");
		}
		else if (wParam == NetUtils::AptTypeT)
		{
			try {
				if (lParam == -1)
				{
					RequeryAptTypes();
				}
				else if (m_dlAptType != NULL)
				{
					long nAptTypeID = lParam;
					long nRow = m_dlAptType->FindByColumn(0, nAptTypeID, 0, false);
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					_RecordsetPtr prs = CreateParamRecordset("SELECT Name, Category, DefaultDuration, DefaultArrivalMins, Color FROM AptTypeT WHERE ID = {INT}",
						nAptTypeID);
					IRowSettingsPtr pRow = NULL;

					if (nRow > -1 && !prs->eof)
					{
						pRow = m_dlAptType->GetRow(nRow);
					}
					else if (!prs->eof)
					{
						pRow = m_dlAptType->GetRow(-1);
					}

					if (pRow != NULL)
					{
						pRow->Value[0] = nAptTypeID;
						pRow->Value[1] = prs->Fields->Item["Name"]->Value;
						pRow->Value[2] = prs->Fields->Item["Category"]->Value;
						pRow->Value[3] = prs->Fields->Item["DefaultDuration"]->Value;
						pRow->Value[4] = prs->Fields->Item["DefaultArrivalMins"]->Value;
						pRow->Value[5] = prs->Fields->Item["Color"]->Value;
						if (nRow == -1)
						{
							m_dlAptType->InsertRow(pRow, m_dlAptType->GetRowCount());
						}
						//TES 3/29/2004: PLID 11643 - Don't do this if we're not open, that's up to the scheduler.
						else if (nRow == m_dlAptType->CurSel && IsWindowVisible())
						{
							CClaimReservation ccr(m_Res, __FUNCTION__); // (c.haag 2010-05-26 15:04) - PLID 38379
							ObtainActiveBorderColor();
							m_Res.PutBorderColor(GetActiveBorderColor());
							// (j.jones 2014-12-04 16:10) - PLID 64119 - an open appt. will intentionally not apply the mix rule color
							ColorReservationBackground(m_Res, GetActiveBorderColor());
						}
					}
				}
			} CResEntryDlg_NxCatchAll("Error in ResEntryDlg::OnTableChanged:AptTypeT");
		}
	} CResEntryDlg_NxCatchAll("Error in ResEntryDlg::OnTableChanged");

	return 0;
}

// (j.jones 2014-08-12 15:23) - PLID 63200 - added Ex handler
LRESULT CResEntryDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {

		switch (wParam) {

		case NetUtils::AppointmentsT:
			try {
				if (IsWindowVisible())
				{
					CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
					long nApptID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);

					if (nApptID == m_Res.GetReservationID()) {
						
						//we don't currently do anything here
					}
				}
			} CResEntryDlg_NxCatchAll("Error in ResEntryDlg::OnTableChangedEx:AppointmentsT");
			break;
		case NetUtils::PatCombo:
		{
				try {
					//if the patients combo needs requeried
					CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
					long nPatientID = VarLong(pDetails->GetDetailData(CClient::pcdiPersonID), -1);
					UpdatePatient(nPatientID);
				} CResEntryDlg_NxCatchAll("Error in OnTableChangedEx::PatCombo");
			}
		}

	} CResEntryDlg_NxCatchAll(__FUNCTION__);

	return 0;
}

// Throw exceptions
BOOL CResEntryDlg::IsCurResourceListMultiResource()
{
	switch (m_arydwSelResourceIDs.GetSize()) {
	case 0:
		// No resources, so not a multi-resource list (we used to ASSERT(FALSE) here and throw an exception but it turns out it's legit)
		return FALSE;
		break;
	case 1:
		// Not a multi-resource list
		return FALSE;
		break;
	default:
		// Is a multi-resource list
		return TRUE;
		break;
	}
}

// Throw exceptions
const CDWordArray &CResEntryDlg::GetCurResourceIDList()
{
	// The list better have been set!  NOTE: It doesn't have to be MULTI-resource, just at least 1 resource.
	CResEntryDlg_ASSERT(m_arydwSelResourceIDs.GetSize() > 0);
	if (m_arydwSelResourceIDs.GetSize() > 0) {
		return m_arydwSelResourceIDs;
	} else {
		ThrowNxException("CResEntryDlg::GetCurResourceIDList: The resource list for this appointment is not set!");
	}
}

// Throw exceptions
long CResEntryDlg::GetCurResourceID_ExpectSingle()
{
	// The list better have been set!
	long nSize = m_arydwSelResourceIDs.GetSize();
	CResEntryDlg_ASSERT(nSize > 0);
	if (nSize > 0) {
		// And it better have exactly one element
		if (nSize == 1) {
			return m_arydwSelResourceIDs.GetAt(0);
		} else {
			ThrowNxException("CResEntryDlg::GetCurResourceID_ExpectSingle: The resource list for this appointment has more than one entry and was expecting only one!");
		}
	} else {
		ThrowNxException("CResEntryDlg::GetCurResourceID_ExpectSingle: The resource list for this appointment is not set!");
	}
}

void CResEntryDlg::SetCurAptType(long nTypeID)
{
	if(nTypeID == -1) {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		//m_dlAptType->SetSelByColumn(0, varNull);
		m_dlAptType->TrySetSelByColumn(0, varNull);
		m_bValidAptTypeStored = false;
	}
	else {
		// (c.haag 2006-05-08 16:01) - PLID 20482 - We no longer wait for the requery. Here's the logic:
		//
		// If a requery is in progress
		//		Store the type in memory
		//		When OnRequeryFinished is called, execute the old code
		// Else
		//		Execute the old code
		//
		//m_dlAptType->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		if (m_dlAptType->IsRequerying()) {
			m_nAptTypeID = nTypeID;
			m_dlAptType->PutComboBoxText(_bstr_t(m_strAptTypeName));
			m_bValidAptTypeStored = true;
		}
		else {
			SetCurAptType_PostRequery(nTypeID);
		}
	}
}

void CResEntryDlg::SetCurAptType_PostRequery(long nTypeID)
{
	if(nTypeID == -1) {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		m_dlAptType->SetSelByColumn(0, varNull);
		m_bValidAptTypeStored = false;
	}
	else {
		if(m_dlAptType->FindByColumn(0, nTypeID, 0, TRUE) == -1) {
			//It is, in fact, inactive, we'll have to pull all the info from data and save it.
			m_nAptTypeID = nTypeID;
			//m.hancock - 6.20.2005 - PLID 16023 - Warn and remove selected type if last apt. type for the patient is now inactive
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr rsType = CreateParamRecordset("SELECT Name, Category, DefaultDuration, Inactive FROM AptTypeT WHERE ID = {INT}", m_nAptTypeID);
			if(rsType->eof) { //m.hancock - 11/3/2005 - PLID 18204 - Check if the appointment type even exists.  If it doesn't, we can't display anything.
				//We're trying to open to a non-existent type, just don't load anything in the
				//dropdown, I guess.
				m_nAptTypeID = -1;
				m_bValidAptTypeStored = false;
				//m.hancock - 11/3/2005 - PLID 18204 - If execution continued after this case, we'd get an error.
				return;
			}

			//m.hancock - 6.20.2005 - PLID 16023 - Warn and remove selected type if last apt. type for the patient is now inactive
			FieldsPtr fType = rsType->Fields;
			//The apt. type is inactive, so display a message and select no apt.type
			//This shows if the appointment type is inactive and this is a new appointment
			if(AdoFldBool(fType, "Inactive") && m_NewRes)
			{
				AfxMessageBox("The current preferences are set to auto-select the patient's last appointment type and purpose for each new appointment.\nHowever, the last appointment type for this patient is now inactive.  The appointment type for this appointment will be reset.");
				m_nAptTypeID = -1;
				m_bValidAptTypeStored = false;
			}
			
			else {
				m_strAptTypeName = AdoFldString(fType, "Name");
				//DRT 9/26/2005 - PLID 17482 - The current selection could be anything be we reach this point (often it's 0 for no appt type), 
				//	so if we do not reset the cursel to -1, then we get a row.  GetCurAptType relies on the current selection being -1 when
				//	attempting to retrieve the information for an inactive type.
				m_dlAptType->CurSel = -1;
				m_dlAptType->PutComboBoxText(_bstr_t(m_strAptTypeName));
				m_nAptTypeCategory = AdoFldByte(fType, "Category");
				m_nAptTypeDuration = AdoFldLong(fType, "DefaultDuration");				
				m_bValidAptTypeStored = true;
			}
		}
		else {
			//The proper type is selected, so let's remember to always get the data out of the
			//datalist.
			m_bValidAptTypeStored = false;
		}
	}
}


void CResEntryDlg::OnSelchangeExtraFields1() 
{
	try {
		m_pSchedulerView->m_nExtraFields[0] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 0);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields2() 
{
	try {
		m_pSchedulerView->m_nExtraFields[1] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS2))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 1);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields3() 
{
	try {
		m_pSchedulerView->m_nExtraFields[2] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS3))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 2);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields4() 
{
	try {
		m_pSchedulerView->m_nExtraFields[3] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS4))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 3);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields5() 
{
	try {
		m_pSchedulerView->m_nExtraFields[4] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS5))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 4);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields6() 
{
	try {
		m_pSchedulerView->m_nExtraFields[5] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS6))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 5);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields7() 
{
	try {
		m_pSchedulerView->m_nExtraFields[6] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS7))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 6);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnSelchangeExtraFields8() 
{
	try {
		m_pSchedulerView->m_nExtraFields[7] = ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS8))->GetCurSel();
		
		//SetPropertyInt("ResEntryExtraField", ((CComboBox *)GetDlgItem(IDC_EXTRA_FIELDS1))->GetCurSel(), 7);
		m_CheckboxesUpdated = true;
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

CString CResEntryDlg::GetExtraFieldText(FieldsPtr fPatInfo, ExtraFields efField)
{
	CString strReturn;
	switch(efField) {
	case efID:
		if (fPatInfo == NULL) return "";
		strReturn.Format("%li", AdoFldLong(fPatInfo, "UserDefinedID"));
		return strReturn;
		break;
	case efHomePhone:
		{
			if (fPatInfo == NULL) return "";
			BOOL bPrivate = AdoFldBool(fPatInfo, "PrivHome");
			if(bPrivate) {
				return "Private";
			}
			else {
				// (j.gruber 2007-08-08 17:36) - PLID 25045 - show hidden if its hidden
				if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
					return "<Hidden>";
				}
				else {
					CString strAns;
					CString strHomePhone = AdoFldString(fPatInfo, "HomePhone");
					if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
						FormatText (strHomePhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
					}
					else {
						//DRT 10/30/2007 - PLID 27910 - Needs to show the home phone as-is if we're not formatting
						strAns = strHomePhone;
					}
					return strAns;
				}
			}
		}
		break;
	case efWorkPhone:
		{
			if (fPatInfo == NULL) return "";
			BOOL bPrivate = AdoFldBool(fPatInfo, "PrivWork");
			if(bPrivate) {
				return "Private";
			}
			else {
				// (j.gruber 2007-08-08 17:36) - PLID 25045 - show hidden if its hidden
				if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
					return "<Hidden>";
				}
				else {
				
					CString strAns;
					CString strWorkPhone = AdoFldString(fPatInfo, "WorkPhone");
					if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
						FormatText (strWorkPhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
					}
					else {
						//DRT 10/30/2007 - PLID 27910 - Needs to show the work phone as-is if we're not formatting
						strAns = strWorkPhone;
					}

					CString strExt = AdoFldString(fPatInfo, "Extension");
					if(strExt.IsEmpty()) {
						return strAns;
					}
					else {
						return strAns + " x" + strExt;
					}
				}
			}
		}
		break;
	case efBirthDate:
		{
			if (fPatInfo == NULL) return "";
			_variant_t varBDate = fPatInfo->GetItem("BirthDate")->GetValue();
			if(varBDate.vt == VT_NULL) {
				return "";
			}
			else {
				return FormatDateTimeForInterface(VarDateTime(varBDate), NULL, dtoDate);
			}
		}
		break;
	// (z.manning 2008-11-19 14:26) - PLID 12282 - Added age
	case efAge:
		{
			if (fPatInfo == NULL) return "";
			_variant_t varBDate = fPatInfo->GetItem("BirthDate")->GetValue();
			if(varBDate.vt == VT_NULL) {
				return "";
			}
			else {
				COleDateTime dtBirthDate = VarDateTime(varBDate);
				COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
				if(dtBirthDate < dtCurrent) {
					// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
					//  validation should only be done when bdays are entered/changed
					// (z.manning 2010-01-13 11:31) - PLID 22672 - Age is now a string
					return GetPatientAgeOnDate(dtBirthDate, dtCurrent, TRUE);
				}
				else {
					return "";
				}
			}
		}
		break;
	case efMobilePhone:
		{
			if (fPatInfo == NULL) return "";
			BOOL bPrivate = AdoFldBool(fPatInfo, "PrivCell", FALSE);
			if (bPrivate) {
				return "Private";
			}
			else {
				// (j.gruber 2007-08-08 17:36) - PLID 25045 - show hidden if its hidden
				if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
					return "<Hidden>";
				}
				else {
					CString strAns;
					CString strCellPhone = AdoFldString(fPatInfo, "CellPhone");
					if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
						FormatText (strCellPhone, strAns, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
					}
					else {
						//DRT 10/30/2007 - PLID 27910 - Needs to show the cell phone as-is if we're not formatting
						strAns = strCellPhone;
					}
					return strAns;
				}
			}
		}
		break;
	case efPrefContact:
		{
			if (fPatInfo == NULL) return "";
			long nPreferredContact = AdoFldLong(fPatInfo, "PreferredContact",0);
			switch(nPreferredContact) {
				case 1:
					return "H:" + GetExtraFieldText(fPatInfo, efHomePhone);
					break;
				case 2:
					return "W:" + GetExtraFieldText(fPatInfo, efWorkPhone);
					break;
				case 3:
					return "M:" + GetExtraFieldText(fPatInfo, efMobilePhone);
					break;
				case 4:
					return "P:" + GetExtraFieldText(fPatInfo, efPager);
					break;
				case 5:
					return "O:" + GetExtraFieldText(fPatInfo, efOtherPhone);
					break;
				case 6:
					return "E:" + GetExtraFieldText(fPatInfo, efEmail);
					break;
				default:
					return "";
					break;
			}
		}
		break;
	case efPatBalance:
		{
			if (fPatInfo == NULL) return "";
			COleCurrency cyTotal;
			if(!(GetCurrentUserPermissions(bioPatientBilling) & SPT__R_________ANDPASS)) {
				return "Private";
			}
			else {
				if (GetPatientTotal(GetCurPatientID(), &cyTotal))
					return FormatCurrencyForInterface(cyTotal);
				else 
					return "";
			}
		}
		break;
	case efInsBalance:
		{
			if (fPatInfo == NULL) return "";
			COleCurrency cyTotal;
			if(!(GetCurrentUserPermissions(bioPatientBilling) & SPT__R_________ANDPASS)) {
				return "Private";
			}
			else {
				if (GetPatientInsuranceTotal(GetCurPatientID(), &cyTotal))
					return FormatCurrencyForInterface(cyTotal);
				else 
					return "";
			}
		}
		break;
	case efRefSource:
		{
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "RefSourceName", "");
		}
		break;
	case efDuration:
		{
			//If they're both valid, and not both midnight.
			if(m_nxtStart->GetStatus() == 1 && m_nxtEnd->GetStatus() == 1 && 
				(COleDateTime(m_nxtStart->GetDateTime()).GetHour() != 0 || COleDateTime(m_nxtStart->GetDateTime()).GetMinute() != 0 || COleDateTime(m_nxtEnd->GetDateTime()).GetHour() != 0 || COleDateTime(m_nxtEnd->GetDateTime()).GetMinute() != 0) ) {
				CString strReturn;
				strReturn.Format("%li:%02li", (COleDateTime(m_nxtEnd->GetDateTime()) - COleDateTime(m_nxtStart->GetDateTime())).GetHours(), (COleDateTime(m_nxtEnd->GetDateTime())-COleDateTime(m_nxtStart->GetDateTime())).GetMinutes());
				return strReturn;
			}
			else {
				return "";
			}
		}
		break;
	case efEmail:
		{
			if (fPatInfo == NULL) return "";
			BOOL bPrivate = AdoFldBool(fPatInfo, "PrivEmail", FALSE);
			if (bPrivate) {
				return "Private";
			}
			else {
				// (j.gruber 2007-08-08 17:36) - PLID 25045 - show hidden if its hidden
				if (!(GetCurrentUserPermissions(bioPatientEmail) & (SPT__R________))) {
					return "<Hidden>";
				}
				else {
					return AdoFldString(fPatInfo, "Email");
				}
			}
		}
		break;
	case efPager:
		{
			if (fPatInfo == NULL) return "";
			BOOL bPrivate = AdoFldBool(fPatInfo, "PrivPager", FALSE);
			if (bPrivate) {
				return "Private";
			}
			else {
				// (j.gruber 2007-08-08 17:36) - PLID 25045 - show hidden if its hidden
				if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
					return "<Hidden>";
				}
				else {
					return AdoFldString(fPatInfo, "Pager");
				}
			}
		}
		break;
	case efOtherPhone:
		{
			if (fPatInfo == NULL) return "";
			BOOL bPrivate = AdoFldBool(fPatInfo, "PrivOther", FALSE);
			if (bPrivate) {
				return "Private";
			}
			else {
				// (j.gruber 2007-08-08 17:36) - PLID 25045 - show hidden if its hidden
				if (!(GetCurrentUserPermissions(bioPatientPhoneNumbers) & (SPT__R________))) {
					return "<Hidden>";
				}
				else {
					return AdoFldString(fPatInfo, "OtherPhone");
				}
			}
		}
		break;

	case efLastLM:
		{
			if(m_dtLastLM.GetStatus() == COleDateTime::invalid) {
				return "";
			}
			else {
				// (d.lange 2015-02-25 14:08) - PLID 64955 - Changed to include the date and time
				return FormatDateTimeForInterface(m_dtLastLM, NULL, dtoDateTime);
			}
		}
		break;

	case efCustom1:
		{
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "Custom1", "");
		}
		break;
	case efCustom2:
		{
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "Custom2", "");
		}
		break;
	case efCustom3:
		{
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "Custom3", "");
		}
		break;
	case efCustom4:
		{
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "Custom4", "");
		}
		break;

	case efInputName:
		{
			return m_strInputName;
		}
		break;

	case efModifiedDate:
		{
			if(m_dtModified.GetStatus() == COleDateTime::invalid) {
				return "";
			}
			else {
				return FormatDateTimeForInterface(m_dtModified, NULL, dtoDate);
			}
		}
		break;

	case efModifiedLogin:
		{
			return m_strModifiedLogin;
		}
		break;
	
	case efPrimaryInsurance:
		{ // (d.moore 2007-06-05 16:29) - PLID 13550 - Adding insurance fields.
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "PrimaryIns", "");
		}
		break;
	case efPrimaryCoPay:
		{ // (d.moore 2007-06-05 16:29) - PLID 13550 - Adding insurance fields.
			if (fPatInfo == NULL) return "";

			// (j.gruber 2010-08-03 15:41) - PLID 39948 - change structure
			_variant_t varCopay, varPercent;
			varCopay = fPatInfo->Item["PriInsCopay"]->Value;
			varPercent = fPatInfo->Item["PriInsCopayPer"]->Value;

			if (varCopay.vt == VT_CY) {
				return FormatCurrencyForInterface(VarCurrency(varCopay), TRUE, FALSE);
			}
			else if (varPercent.vt == VT_I4) {
				CString strPercent;
				strPercent.Format("%li%%", VarLong(varPercent));
				return strPercent;
			}
			else {
				return "";
			}
			
		}
		break;
	case efSecondaryInsurance:
		{ // (d.moore 2007-06-05 16:29) - PLID 13550 - Adding insurance fields.
			if (fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "SecondaryIns", "");
		}
		break;
	case efSecondaryCoPay:
		{ // (d.moore 2007-06-05 16:29) - PLID 13550 - Adding insurance fields.
			if (fPatInfo == NULL) return "";
			// (j.gruber 2010-08-03 15:41) - PLID 39948 - change structure
			_variant_t varCopay, varPercent;
			varCopay = fPatInfo->Item["SecInsCoPay"]->Value;
			varPercent = fPatInfo->Item["SecInsCoPayPer"]->Value;

			if (varCopay.vt == VT_CY) {
				return FormatCurrencyForInterface(VarCurrency(varCopay), TRUE, FALSE);
			}
			else if (varPercent.vt == VT_I4) {
				CString strPercent;
				strPercent.Format("%li%%", VarLong(varPercent));
				return strPercent;
			}
			else {
				return "";
			}		
		}
		break;
	case efCheckInTime:
		{	//DRT 9/10/2008 - PLID 31127
			if(fPatInfo == NULL) return "";
			_variant_t var = fPatInfo->Item["CheckInTime"]->Value;
			if(var.vt == VT_DATE) {
				COleDateTime dt = VarDateTime(var);
				return FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
			}
			else {
				return "";
			}
		}
		break;

	case efCheckOutTime:
		{	//DRT 9/10/2008 - PLID 31127
			if(fPatInfo == NULL) return "";
			_variant_t var = fPatInfo->Item["CheckOutTime"]->Value;
			if(var.vt == VT_DATE) {
				COleDateTime dt = VarDateTime(var);
				return FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
			}
			else {
				return "";
			}
		}
		break;
	case efInputDate:
		{
			//TES 11/12/2008 - PLID 11465 - Added InputDate
			if(m_dtInput.GetStatus() == COleDateTime::invalid) {
				return "";
			}
			else {
				return FormatDateTimeForInterface(m_dtInput, NULL, dtoDate);
			}
		}
		break;
	//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
	case efRefPhysName:
		{
			if(fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "RefPhysName", "");
		}
		break;
	case efRefPhysWorkPhone:
		{
			if(fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "RefPhysWorkPhone", "");
		}
		break;
	case efRefPhysFax:
		{
			if(fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "RefPhysFax", "");
		}
		break;
	// (c.haag 2008-11-20 16:04) - PLID 31128 - Insurance Referral summary
	case efInsuranceReferrals:
		{
			if(fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "InsuranceReferralSummary", "");
		}
		break;
	// (j.armen 2011-07-01 15:43) - PLID 44205 - Confirmed By field
	case efConfirmedBy:
		{
			if(fPatInfo == NULL) return "";
			return AdoFldString(fPatInfo, "ConfirmedBy", "");
		}
		break;
	case efArrivalTime: // (z.manning 2011-07-01 16:16) - PLID 23273
		{
			CString strArrivalTime;
			if(m_nxtArrival->GetStatus() == 1) {
				COleDateTime dtArrival(m_nxtArrival->GetDateTime());
				strArrivalTime = FormatDateTimeForInterface(dtArrival, DTF_STRIP_SECONDS, dtoTime);
			}
			return strArrivalTime;
		}
		break;
	case efCancelledDate: // (j.luckoski 2012-05-02 16:27) - PLID 11597 - Added in cancelledDate to know when cancelled.
		{
			if (fPatInfo == NULL) return "";
			COleDateTime dtCancelled = AdoFldDateTime(fPatInfo, "CancelledDate", g_cdtInvalid);
			if(dtCancelled.GetStatus() == COleDateTime::invalid) {
				return "";
			}
			else {
				return FormatDateTimeForInterface(dtCancelled, NULL, dtoDate);
			}
		}
		break;
	case efApptPrimaryIns: // (j.gruber 2012-09-05 11:35) - PLID 51926
		{
			InsuranceInfo *pInsInfo;
			if (m_mapInsPlacements.Lookup(1, pInsInfo)) {
				return pInsInfo->strInsCoName;
			}
			else {
				return "";
			}
		}
	break;
	case efApptSecondaryIns: // (j.gruber 2012-09-05 11:35) - PLID 51926
		{
			InsuranceInfo *pInsInfo;
			if (m_mapInsPlacements.Lookup(2, pInsInfo)) {
				return pInsInfo->strInsCoName;
			}
			else {
				return "";
			}
		}
	break;
	// (j.gruber 2012-09-05 11:35) - PLID 51926
	case efApptInsList:
		{			
			//first we need to loop through the map and get the highest placement
			long nHighPlacement = -1;
			POSITION pos = m_mapInsPlacements.GetStartPosition();
			InsuranceInfo *pInsInfo;
			long nPlacement = -1;

			while (pos != NULL) {				
				m_mapInsPlacements.GetNextAssoc( pos, nPlacement, pInsInfo);
				if (nPlacement > nHighPlacement) {
					nHighPlacement = nPlacement;
				}
			}

			CString strList;
			//now construct our string in placement order
			for (int i=1; i <=nHighPlacement; i++){
				if (m_mapInsPlacements.Lookup(i, pInsInfo)){
					strList += pInsInfo->strRespType + ": " + pInsInfo->strInsCoName + "; ";
				}
				else {
					//took out this assertion, we used it for saving, but if they chose the secondary before the primary, we are ok with that.
					//ASSERT(FALSE);
				}
			}

			//trim off the last space
			strList.TrimRight();
			return strList;
		}
	break;
	case efApptRefPhys: // (j.gruber 2013-01-08 15:19) - PLID 54497 - referring phys
		{
			if (m_bIsCurRefPhysSet) {
				return m_strCurRefPhysName;
			}
			else {
				return "";
			}

		}
	break;
	case efNxWebCode:// (s.tullis 2014-01-21 15:41) - PLID 49748 - Add nexweb activation code to the  extra info fields
		{
			
			if(fPatInfo == NULL) {
				return "";
			}
			else{
			    return AdoFldString(fPatInfo, "SecurityCode", "");
				
			}
		}

	break;

	case efNxWebCodeDate: // (s.tullis 2014-01-24 15:28) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
		{
		
			
	 if (fPatInfo != NULL){
			COleDateTime dtCreation = AdoFldDateTime(fPatInfo, "SecurityCodeCreationDate", COleDateTime());
			int nExpirationDays = AdoFldLong(fPatInfo, "SecurityCodeExpDate", -1);
			if( dtCreation != COleDateTime() ){
				if( nExpirationDays > 0 ){
					COleDateTimeSpan dsSpan = COleDateTimeSpan(nExpirationDays, 0, 0, 0);
					return FormatDateTimeForInterface(dtCreation+dsSpan, DTF_STRIP_SECONDS, dtoDate);
				}else{
					return  "<None>";
				}
			}else{
				return  "";
			}
		}else{
			return  "";
		}
		
		
		}

	break;

	default:
		//Invalid field!
		ASSERT(FALSE);
		return "";
	}
}

CString CResEntryDlg::FormatTextForReadset(ExtraFields efField, const CString &strText)
{
	//First off, if you pass in blank, you get back blank.
	if(strText == "") return strText;
	CString strReturn;
	switch(efField)
	{
	case efID:
		return "ID:" + strText;
		break;
	case efHomePhone:
		strReturn = strText;
		if (strReturn.GetLength() >= 7)  
			return "H:" + strReturn;
		else 
			return "";
		break;
	case efWorkPhone:
		strReturn = strText;
		if (strReturn.GetLength() >= 7)  
			return "W:" + strReturn;
		else 
			return "";
		break;
	case efBirthDate:
		return "BD:" + strText;
		break;	
	// (z.manning 2008-11-19 14:36) - PLID 12282 - Added age
	case efAge:
		return "Age:" + strText;
		break;	
	case efMobilePhone:
		strReturn = strText;
		if (strReturn.GetLength() >= 7)  
			return "M:" + strReturn;
		else 
			return "";
		break;

	case efPrefContact:
		if(strText.GetLength() > 0) 
			return "P:" + strText;
		else 
			return strText;
		break;

	case efPatBalance:
		return "Pat:" + strText;
	case efInsBalance:
		return "Ins:" + strText;
		break;

	case efRefSource:
		return "R:" + strText;
		break;

	case efDuration:
		return "D:" + strText;
		break;
	case efEmail:
		return "E:" + strText;
		break;
	case efPager:
		return "P:" + strText;
		break;
	case efOtherPhone:
		return "O:" + strText;
		break;
		
	case efCustom1:
		return "C1:" + strText;
		break;
	case efCustom2:
		return "C2:" + strText;
		break;
	case efCustom3:
		return "C3:" + strText;
		break;
	case efCustom4:
		return "C4:" + strText;
		break;
	case efInputName:
		return "I:" + strText;
	case efLastLM:
		return "LM:" + strText;
	case efModifiedDate:
		return "MD:" + strText;
	case efModifiedLogin:
		return "MB:" + strText;
	case efPrimaryInsurance: // (d.moore 2007-06-05 17:47) - PLID 13550
		// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
		return "Med Pri Ins:" + strText;
		break;
	case efPrimaryCoPay: // (d.moore 2007-06-05 17:47) - PLID 13550
		// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
		return "Med Pri Co:" + strText;
		break;
	case efSecondaryInsurance: // (d.moore 2007-06-05 17:47) - PLID 13550
		// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
		return "Med Sec Ins:" + strText;
		break;
	case efSecondaryCoPay: // (d.moore 2007-06-05 17:47) - PLID 13550
		// (j.gruber 2012-08-06 16:06) - PLID 51972 - changed the name
		return "Med Sec Co:" + strText;
		break;
	case efCheckInTime:	//DRT 9/10/2008 - PLID 31127
		return "In:" + strText;
		break;
	case efCheckOutTime:	//DRT 9/10/2008 - PLID 31127
		return "Out:" + strText;
		break;
	case efInputDate: //TES 11/12/2008 - PLID 11465 - Added InputDate
		return "IDate:" + strText;
		break;
	case efRefPhysName: //TES 11/12/2008 - PLID 12057 - Added Referring Physician info
		return "RP:" + strText;
		break;
	case efRefPhysWorkPhone:
		return "RPTel:" + strText;
		break;
	case efRefPhysFax:
		return "RPFax:" + strText;
		break;
	// (c.haag 2008-11-20 16:07) - PLID 31128 - Insurance referral summary
	case efInsuranceReferrals:
		return "Ins. Referrals: " + strText;
		break;
	// (j.armen 2011-07-01 15:43) - PLID 44205 - Confirmed By Field
	case efConfirmedBy:
		return "Conf. By: " + strText;
		break;
	case efArrivalTime: // (z.manning 2011-07-01 16:17) - PLID 23273
		return "Arrival Time:" + strText;
		break;
	case efCancelledDate: // (j.luckoski 2012-05-02 16:29) - PLID 11597 - Added cancelled date to extra fields
		return "Cancelled Date:" + strText;
		break;
	case efApptPrimaryIns: // (j.gruber 2012-08-06 09:18) - PLID 51926
		return "Appt Pri Ins:" + strText;
		break;
	case efApptSecondaryIns: // (j.gruber 2012-08-06 09:18) - PLID 51926
		return "Appt Sec Ins:" + strText;
		break;
	case efApptInsList: // (j.gruber 2012-08-06 09:18) - PLID 51926
		return "Appt Ins List:" + strText;
	break;
	case efApptRefPhys: // (j.gruber 2013-01-08 15:20) - PLID 54497
		return "Appt RP: " + strText;
    break;
	// (s.tullis 2014-01-21 15:41) - PLID 49748 - Add nexweb activation code to the  extra info fields
	case efNxWebCode:
		return "NexWeb Code:" + strText;
		break;
	case efNxWebCodeDate:// (s.tullis 2014-01-24 16:00) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
		return "NexWeb Code Exp. Date:" + strText;
		break;
	default:
		//Invalid field!
		ASSERT(FALSE);
		return "";
	}
}

void CResEntryDlg::OnKillFocusArrivalTimeBox()
{
	try {
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnKillFocusStartTimeBox() 
{
	try {
		ApplyToExtraInfo();
		ApplyToResBox();

		// (z.manning, 08/07/2006) - PLID 21804 - If the arrival time field is not visible, then
		// we need to update it when they change the start time.
		if(m_nxtStart->GetStatus() == 1 && !GetDlgItem(IDC_ARRIVAL_TIME_BOX)->IsWindowVisible()) {
			COleDateTime dtOriginalStartTime;
			dtOriginalStartTime.SetTime(m_dtStartTime.GetHour(), m_dtStartTime.GetMinute(), m_dtStartTime.GetSecond()); // Only want to compare times
			if(m_nxtStart->GetDateTime() == dtOriginalStartTime) {
				m_nxtArrival->SetDateTime(m_dtArrivalTime);
			}
			else {
				FillDefaultArrivalTime();
			}

			if(m_nxtArrival->GetDateTime() > m_nxtStart->GetDateTime()) {
				// This should be impossible, but is here as a failsafe.
				m_nxtArrival->SetDateTime(m_nxtStart->GetDateTime());
			}
		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnKillFocusEndTimeBox() 
{
	try {
		ApplyToExtraInfo();
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnChangedArrivalTimeBox()
{
	try {
		// (c.haag 2004-06-23 10:57) - PLID 13130 - We need to set the modified flag when
		// the contents of a time control changes; otherwise we will have an improper
		// active count, resulting in dumbness when a user clicks off the time control
		// onto the singleday.
		SetModifiedFlag();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnChangedStartTimeBox()
{
	try {
		// (c.haag 2004-06-23 10:57) - PLID 13130 - We need to set the modified flag when
		// the contents of a time control changes; otherwise we will have an improper
		// active count, resulting in dumbness when a user clicks off the time control
		// onto the singleday.
		SetModifiedFlag();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnChangedEndTimeBox()
{
	try {
		// (c.haag 2004-06-23 10:57) - PLID 13130 - We need to set the modified flag when
		// the contents of a time control changes; otherwise we will have an improper
		// active count, resulting in dumbness when a user clicks off the time control
		// onto the singleday.
		SetModifiedFlag();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

long CResEntryDlg::GetNoShowComboSel()
{
	return VarLong(m_dlNoShow->GetValue(m_dlNoShow->CurSel, 0));
}

CString CResEntryDlg::GetNoShowComboSelName()
{
	//(e.lally 2008-06-20) PLID 29500 - The current selection should always be valid, but we should
		//check anyways just in case.
	if(m_dlNoShow->CurSel != sriNoRow){
		return VarString(m_dlNoShow->GetValue(m_dlNoShow->CurSel, nscPlainName));
	}
	else{
		return "";
	}
}

BOOL CResEntryDlg::TryCorrectingPatientSelResultFailure()
{
	// The failure may have been do the patients combo excluding inactive patients.
	// Before we go any further, lets see if that is the case here.
	if(GetRemotePropertyInt("SchedShowActivePtOnly", 0, 0, "<None>", true))
	{
		// (j.gruber 2010-10-04 14:39) - PLID 40415 - security group
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (j.gruber 2011-07-22 17:20) - PLID 44118 - fore color
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), FormatString("SELECT ComboQ.PersonID, ComboQ.Last, ComboQ.First, ComboQ.Middle, "
			// (a.walling 2010-05-27 15:35) - PLID 38917 - No more Name column
			//"(ComboQ.[Last] + ', ' + ComboQ.[First] + ' ' + ComboQ.[Middle]) AS FullName, "
			"ComboQ.PatientID, CASE WHEN BlockedPatientsQ.PatientID Is Null THEN ComboQ.BirthDate ELSE NULL END AS BirthDate, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN ComboQ.SocialSecurity ELSE NULL END AS SocialSecurity, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN OHIPHealthCardNum ELSE NULL END AS OHIPHealthCardNum, "
			" ComboQ.SecGroupName, "
			" CASE WHEN ComboQ.Archived = 0 then CONVERT(int, 0) ELSE CONVERT(int, {INT}) END AS ForeColor "
			"FROM %s "
			"WHERE ComboQ.PersonID = {INT}", GetPatientFromClause()), PATIENT_LIST_INACTIVE_COLOR, m_nCurPatientID);
		if (!prs->eof)
		{
			// Yes, this patient is inactive. Add the patient to the dropdown in its
			// proper sorted position. The patient will persist in the list until we
			// hide the window.
		
			// (a.walling 2010-12-30 11:23) - PLID 40081 - Use datalist2 to avoid row index race conditions
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlAptPatient->GetNewRow();
			pRow->Value[plID] = m_nCurPatientID;

			// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
			pRow->Value[plLast] = prs->Fields->Item["Last"]->Value;
			pRow->Value[plFirst] = prs->Fields->Item["First"]->Value;
			pRow->Value[plMiddle] = prs->Fields->Item["Middle"]->Value;
			// (a.walling 2010-05-27 15:35) - PLID 38917 - No more Name column
			//pRow->Value[4] = prs->Fields->Item["FullName"]->Value;
			pRow->Value[plUserDefinedID] = prs->Fields->Item["PatientID"]->Value;
			pRow->Value[plBirthDate] = prs->Fields->Item["BirthDate"]->Value;
			pRow->Value[plSSN] = prs->Fields->Item["SocialSecurity"]->Value;
			// (j.jones 2010-05-04 10:58) - PLID 32325 - added OHIPHealthCardNum
			pRow->Value[plHealthCard] = prs->Fields->Item["OHIPHealthCardNum"]->Value;
			// (j.gruber 2010-10-04 14:39) - PLID 40415 - security group
			pRow->Value[plSecurityGroup] = prs->Fields->Item["SecGroupName"]->Value;
			// (j.gruber 2011-07-22 17:20) - PLID 44118 - fore color
			pRow->Value[plForeColor] = prs->Fields->Item["ForeColor"]->Value;
			//m_dlAptPatient->AddRowBefore(pRow, m_dlAptPatient->GetFirstRow());
			//m_dlAptPatient->Sort();
			// (a.walling 2013-03-07 09:32) - PLID 55502 - Just add sorted to begin with!
			pRow = m_dlAptPatient->AddRowSorted(pRow, NULL);

			// Now select it.
			// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
			//NXDATALIST2Lib::IRowSettingsPtr pSelRow = m_dlAptPatient->FindByColumn(plID, m_nCurPatientID, NULL, VARIANT_TRUE);
			m_dlAptPatient->CurSel = pRow;

			// Make sure we remember the patient ID so that when we hide this window,
			// it will remove inactive patients from what could be an active patient
			// list.
			m_adwInactivePtInActiveList.Add(m_nCurPatientID);
			return TRUE; // Success!
		}
	}
	return FALSE;
}

// (a.walling 2015-01-27 12:14) - PLID 64416 - Warn if patient in rescheduler queue
bool CResEntryDlg::WarnIfPatientInReschedulingQueue()
{
	long nPatientID = GetCurPatientID();

	if (nPatientID != m_nWarnedForPatientID && Nx::Scheduler::IsPatientInReschedulingQueue(nPatientID)) {
		m_bCheckActive = false;
		m_nWarnedForPatientID = nPatientID;
		auto nRet = GetMainFrame()->MessageBox("This patient has an appointment that needs to be rescheduled. Would you like to open the Appointment Rescheduling Queue now?", nullptr, MB_YESNO);
		if (IDYES == nRet) {
			// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
			HandleCancelAndCloseReservation();

			if (IsWindowVisible()) { // was not cancelled
				return false;
			}
			auto reschedulingQueueAppts = Nx::Scheduler::GetAppointmentsInReschedulingQueueForPatient(nPatientID);
			GetMainFrame()->ShowReschedulingQueue(reschedulingQueueAppts.front());
			return true;
		}
		m_bCheckActive = true;
	}
	return false;
}

void CResEntryDlg::WarnIfInactivePatientSelected()
{
	// (c.haag 2003-10-06 11:20) - Don't count the sentinel patient
	if (m_nCurPatientID == -25)
		return;

	if (GetRemotePropertyInt("WarnForInactivePtInResEntry", 0, 0, "<None>", true))
	{
		try
		{
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
			if (ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM PersonT WHERE ID = {INT} AND Archived <> 0", m_nCurPatientID))
			{
				CKeepDialogActive kda(this);
				MsgBox("Warning: You currently have an inactive patient selected.");
			}
		}
		CResEntryDlg_NxCatchAll("Error determining if patient is active");	
	}
}

// (j.jones 2014-11-14 10:30) - PLID 64116 - now takes in a boolean to state whether
// the user has changed the patient on the appointment
void CResEntryDlg::ApplyCurrentPatient(bool bPatientChanged)
{
	try {
		// (a.walling 2009-06-05 13:03) - PLID 34496 - Use a proper parent for dialogs from here
		_RecordsetPtr rsPatient;
		long nPatientID = GetCurPatientID();
	
		if(nPatientID == -25) {
			// (j.armen 2012-04-25 17:56) - PLID 49861 - Set the window text based on event/appointment
			// (j.luckoski 2012-05-10 17:03) - PLID 50321 - Add CANCELLED if cancelled to the PLID 49861's name addition.
			// (j.luckoski 2012-05-11 15:46) - PLID 50321 - Rearranged - to appear only when CANCELLED to avoid Event - and Appointemnt -
			CString strTitle = (IsEvent() ? "Event " : "Appointment ");
			strTitle += (m_bIsCancelled ? "- CANCELLED" : "" );
			SetWindowText(strTitle);

			// (j.gruber 2012-08-08 10:37) - PLID 51882 - if the patient ID is -25, gray out the more button			
			GetDlgItem(IDC_RES_INS_MORE)->EnableWindow(FALSE);
			m_pInsCatList->Enabled = FALSE;
			InvalidateRect(m_rcInsMore);
			
		}
		else {

			// (j.gruber 2012-08-08 10:37) - PLID 51882 - if the patient ID is not -25, enable the more button
			GetDlgItem(IDC_RES_INS_MORE)->EnableWindow(TRUE);
			m_pInsCatList->Enabled = TRUE;
			InvalidateRect(m_rcInsMore);
			
			//
			// (c.haag 2006-05-08 12:54) - PLID 20475 - Get patient info. By doing so here, we get
			// two speed advantages:
			//
			// 1. We only load patient fields once
			// 2. We don't have to run CoPay and Allergy queries when we don't need to.
			//
			rsPatient = CreatePatientInfoRecordset();
			/*_RecordsetPtr prs = CreateRecordset(adOpenDynamic, adLockOptimistic,
				"SELECT DisplayWarning, WarningMessage, WarningDate, WarningExpireDate, WarningUseExpireDate, UserName, "
				"Last + ', ' + First + ' ' + Middle AS FullName "
				"FROM PersonT  "
				"LEFT JOIN UsersT ON UsersT.PersonID = PersonT.WarningUserID "
				"WHERE PersonT.ID = %li", nPatientID);*/
			// Proceed only if we are on a good record
			if (!rsPatient->eof) {
				FieldsPtr flds = rsPatient->Fields;

				// (j.gruber 2012-08-01 10:46) - PLID 51885 - set the patient ins party 
				m_patientInsInfo.m_strInsFirst = AdoFldString(flds, "PatFirst", "");
				m_patientInsInfo.m_strInsMiddle = AdoFldString(flds, "PatMiddle", "");
				m_patientInsInfo.m_strInsLast = AdoFldString(flds, "PatLast", "");
				// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
				m_patientInsInfo.m_strInsTitle = AdoFldString(flds, "PatTitle", "");
				m_patientInsInfo.m_strInsAddress1 = AdoFldString(flds, "Address1", "");
				m_patientInsInfo.m_strInsAddress2 = AdoFldString(flds, "Address2", "");
				m_patientInsInfo.m_strInsCity = AdoFldString(flds, "City", "");
				m_patientInsInfo.m_strInsState = AdoFldString(flds, "State", "");
				m_patientInsInfo.m_strInsZip = AdoFldString(flds, "Zip", "");
				// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
				m_patientInsInfo.m_strInsCountry = AdoFldString(flds, "Country", "");
				m_patientInsInfo.m_strInsPhone = AdoFldString(flds, "Homephone", "");
				m_patientInsInfo.m_strInsSSN = AdoFldString(flds, "SocialSecurity", "");
				m_patientInsInfo.m_nInsGender = (long)AdoFldByte(flds, "Gender", -1);
				m_patientInsInfo.m_strInsEmployer = AdoFldString(flds, "Company", "");
				_variant_t varDate = flds->Item["BirthDate"]->Value;
				if (varDate.vt == VT_DATE) {
					m_patientInsInfo.m_dtInsBirthDate = VarDateTime(varDate);
				}

				// (j.armen 2012-04-25 17:56) - PLID 49861 - Set the window text based on event/appointment and pt name
				// (j.luckoski 2012-05-10 17:03) - PLID 50321 - Add CANCELLED if cancelled to the PLID 49861's name addition.
				SetWindowText((IsEvent() ? "Event - " : "Appointment - ") + AdoFldString(flds, "FullName", "Unspecified Patient") + (m_bIsCancelled ? " - CANCELLED" : "" ));

				// Handle warnings
				if (AdoFldBool(flds, "DisplayWarning") && GetRemotePropertyInt("ResShowPatientWarning", 1, 0, GetCurrentUserName(), false)) {

					BOOL bWarn = TRUE;

					//check to see if they are using the expiration date
					if (AdoFldBool(flds, "WarningUseExpireDate")) {

						//check to see if the warning has expired
						COleDateTime dtExpire = VarDateTime(flds->Item["WarningExpireDate"]->Value);
						COleDateTime dtToday;
						dtToday.SetDate(COleDateTime::GetCurrentTime().GetYear(), COleDateTime::GetCurrentTime().GetMonth(), COleDateTime::GetCurrentTime().GetDay());
						if (dtExpire < dtToday) {

							//don't show it
							bWarn = FALSE;
						}
					}

					if(bWarn) {
						// If this patient should display a warning, display it
						CUserWarningDlg dlgWarning(this);
						CString strTitle;
						BOOL bKeepWarning;
						m_bCheckActive = false;

						// Show the warning
						// Display it
						if (!GetPropertyInt("G2ShowWarningStats", 1, 0))
							// (j.luckoski 2012-05-11 15:46) - PLID 50321 - Place Unspecified patient as the null value but should not get here
							strTitle = AdoFldString(flds, "FullName", "Unspecified Patient") + ":";
						else
						{
							if (flds->Item["UserName"]->Value.vt == VT_NULL ||
								flds->Item["WarningDate"]->Value.vt == VT_NULL)
							{
								// (j.luckoski 2012-05-11 16:02) - PLID 50321 - Unspecified patient for null
								strTitle = AdoFldString(flds, "FullName", "Unspecified Patient") + ":";
							}
							else
							{
								// (j.luckoski 2012-05-11 15:47) - PLID 50321 - Use unspecified patient as the null value even though it should never be null
								strTitle = CString("Warning for ") + AdoFldString(flds, "FullName", "Unspecified Patient") + " entered by " + AdoFldString(flds, "UserName") + " on " + FormatDateTimeForInterface(AdoFldDateTime(flds, "WarningDate"), DTF_STRIP_SECONDS, dtoDate);
							}
						}

						// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
						bKeepWarning = dlgWarning.DoModalWarning(
							AdoFldString(flds, "WarningMessage", "Unspecified Warning"), TRUE, 
							strTitle, "Patient Warning", AdoFldLong(flds, "WarningColor", 0), AdoFldLong(flds, "WarningID", -1), AdoFldString(flds, "WarningName", ""));
						
						// If the user set it to not display take it out of the data
						if (!bKeepWarning) {							
							// (a.walling 2009-03-04 15:03) - PLID 33341 - This recordset does not support updating
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							ExecuteParamSql("UPDATE PersonT SET DisplayWarning = {INT} WHERE ID = {INT}", bKeepWarning?1:0, nPatientID);

							//flds->Item["DisplayWarning"]->Value = (bool)(bKeepWarning?true:false);							
							//rsPatient->Update();
							//auditing
							long nAuditID = BeginNewAuditEvent();
							// (a.walling 2008-06-04 15:43) - PLID 29900 - Use the correct patient name and ID
							AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientWarning, nPatientID, (bKeepWarning == 0 ? "Warn" : "Don't Warn"), (bKeepWarning == 0 ? "Don't Warn" : "Warn"), aepMedium, aetChanged);

						}
						m_bCheckActive = true;
					}
				}
			}

			// (j.jones 2014-11-14 09:48) - PLID 64116 - if this is a new appointment, and the user wants to
			// auto-fill the insurance placements, do so now, but not if we're simply loading an existing
			// appointment and the patient has not changed
			if (m_ResID == -1 || bPatientChanged) {

				// (j.jones 2014-11-14 13:30) - PLID 64169 - this is now a global function
				::TryAutoFillAppointmentInsurance(GetCurPatientID(), m_mapInsPlacements);

				//redraw the hyperlinks
				InvalidateRect(m_rcPriIns);
				InvalidateRect(m_rcSecIns);
			}

			if(GetRemotePropertyInt("ShowInsuranceReferralWarning",0,0,"<None>",TRUE) == 1) {
				//show active insurance referrals
				// (c.haag 2008-11-21 10:45) - PLID 32136 - Filter out inactive insured parties
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
				// (j.gruber 2011-10-05 11:45) - PLID 45837 - add other fields
				// (j.gruber 2011-10-05 14:24) - PLID 45838 - just not show expired
				COleDateTime dtNow = AsDateNoTime(COleDateTime::GetCurrentTime());
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
				_RecordsetPtr rsInsuranceReferrals = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuranceCoT.Name AS InsCoName, "
					"AuthNum, StartDate, EndDate, NumVisits, (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AS NumUsed, "
					" PersonProvT.ID as ProviderID, PersonProvT.Last + ', ' + PersonProvT.First + ' ' + PersonProvT.Middle AS ProviderName, "
					" LocationsT.ID as LocationID, LocationsT.Name as LocationName, InsuranceReferralsT.Comments "
					"FROM InsuranceReferralsT "
					"INNER JOIN InsuredPartyT ON InsuranceReferralsT.InsuredPartyID = InsuredPartyT.PersonID "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"LEFT JOIN ProvidersT ON InsuranceReferralsT.ProviderID = ProvidersT.PersonID "
					"LEFT JOIN PersonT PersonProvT ON ProvidersT.PersonID = PersonProvT.ID "
					"LEFT JOIN LocationsT ON InsuranceReferralsT.LocationId = LocationsT.ID "
					"LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "					
					"GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
					"WHERE NumVisits > COALESCE(NumUsed, 0) AND EndDate >= {OLEDATETIME} AND InsuredPartyT.PatientID = {INT} "
					"AND InsuredPartyT.RespTypeID <> -1 "
					"ORDER BY StartDate", dtNow, nPatientID);

				if(!rsInsuranceReferrals->eof) {

					CString strPrompt;
					strPrompt = "This patient has the following active insurance referrals:";

					while(!rsInsuranceReferrals->eof) {

						CString strInsCoName = AdoFldString(rsInsuranceReferrals, "InsCoName","");
						CString strAuthNum = AdoFldString(rsInsuranceReferrals, "AuthNum","");
						COleDateTime dtStartDate = AdoFldDateTime(rsInsuranceReferrals, "StartDate");
						COleDateTime dtEndDate = AdoFldDateTime(rsInsuranceReferrals, "EndDate");
						long NumVisits = AdoFldLong(rsInsuranceReferrals, "NumVisits",0);
						long NumUsed = AdoFldLong(rsInsuranceReferrals, "NumUsed",0);
						// (j.gruber 2011-10-05 12:22) - PLID 45837 - added providers, locations, and comments
						long nProviderID = AdoFldLong(rsInsuranceReferrals, "ProviderID", -1);
						CString strProvName;
						if (nProviderID != -1) {
							strProvName = "Provider: " + AdoFldString(rsInsuranceReferrals, "ProviderName", "") + ";";
						}
						CString strLocName;
						long nLocationID = AdoFldLong(rsInsuranceReferrals, "LocationID", -1);
						if (nLocationID != -1) {
							strLocName = "Location: " + AdoFldString(rsInsuranceReferrals, "LocationName", "") + ";";
						}

						CString strTemp;
						strTemp = AdoFldString(rsInsuranceReferrals, "Comments", "");
						CString strComments;
						if (!strTemp.IsEmpty()) {
							strComments = "Comments: " + strTemp + ";";
						}						

						CString strDates;
						if(dtStartDate == dtEndDate) {
							strDates.Format("Allowed Date: %s;",FormatDateTimeForInterface(dtStartDate,NULL,dtoDate));
						}
						else {
							strDates.Format("Allowed Dates: %s - %s;",FormatDateTimeForInterface(dtStartDate,NULL,dtoDate),FormatDateTimeForInterface(dtEndDate,NULL,dtoDate));
						}

						CString strWarning;
						strWarning.Format("\n\nInsurance: %s; Allowed Visits: %li, Remaining: %li; %s %s %s %s",strInsCoName,NumVisits,NumVisits - NumUsed,strDates, strProvName, strLocName, strComments);
						strPrompt += strWarning;

						rsInsuranceReferrals->MoveNext();
					}

					MessageBox(strPrompt, NULL, MB_ICONINFORMATION);
				}

				rsInsuranceReferrals->Close();
			}

			// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party,
			// but when loading an appt. HasCoPay would be false if this patient had no copays
			// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
			BOOL bWarnCopays = (GetRemotePropertyInt("WarnCopays", 0, 0, GetCurrentUserName(), true) == 1);
			if(bWarnCopays) {
				//now check for the CoPay Warning
				if (0 != AdoFldLong(rsPatient, "HasCoPay")) {
					// (j.jones 2009-09-17 10:33) - PLID 35572 - ensure we do not warn about inactive insurances
					// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
					_RecordsetPtr rsCoPay = CreateParamRecordset(GetRemoteDataSnapshot(), 
						" SELECT [Last] + ', ' + [First] AS FullName, InsuranceCoT.Name AS InsCoName, "
						" CoPayMoney, CopayPercentage, ServicePayGroupsT.Name AS PayGroupName "
						" FROM InsuredPartyT "
						" INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
						" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						" INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
						" INNER JOIN InsuredPartyPayGroupsT ON InsuredPartyT.PersonID = InsuredPartyPayGroupsT.InsuredPartyID "
						" INNER JOIN ServicePayGroupsT ON InsuredPartyPayGroupsT.PayGroupID = ServicePayGroupsT.ID "
						" WHERE PatientsT.PersonID = {INT} AND InsuredPartyT.RespTypeID <> -1 "
						" AND ((CoPayMoney Is Not Null AND CopayMoney <> Convert(money,0)) OR (CopayPercentage Is Not Null AND CopayPercentage <> 0)) "
						" ORDER BY RespTypeID, ServicePayGroupsT.Name", nPatientID);

					if(!rsCoPay->eof) {

						CString strName = AdoFldString(rsCoPay, "FullName","") + ":";
						CString strCoPayWarning = "This patient has the following copays:\r\n";

						while(!rsCoPay->eof) {

							CString str, strCopay;

							// (j.jones 2010-08-03 13:17) - PLID 39937 - the copay is now in two fields,
							// mutually exclusive, they cannot both be filled
							_variant_t varCoPayMoney = rsCoPay->Fields->Item["CoPayMoney"]->Value;
							_variant_t varCopayPercentage = rsCoPay->Fields->Item["CopayPercentage"]->Value;

							// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
							if(varCoPayMoney.vt == VT_CY && VarCurrency(varCoPayMoney) != COleCurrency(0,0)) {
								//flat amount
								strCopay.Format("%s",FormatCurrencyForInterface(VarCurrency(varCoPayMoney,COleCurrency(0,0))));
							}
							else if(varCopayPercentage.vt == VT_I4 && VarLong(varCopayPercentage) != 0) {
								//percentage
								strCopay.Format("%li%%",VarLong(varCopayPercentage,0));
							}
							else {
								//shady, why do we even have results?
								rsCoPay->MoveNext();
								continue;
							}

							str.Format("\n%s - %s: %s",
								AdoFldString(rsCoPay, "InsCoName",""),
								AdoFldString(rsCoPay, "PayGroupName",""),
								strCopay);
							strCoPayWarning += str;

							rsCoPay->MoveNext();
						}


						// Display it
						CNxSchedulerDlg *pSchedSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet();
						if(pSchedSheet) {
							pSchedSheet->MessageBox(strCoPayWarning, "CoPay Warning", MB_ICONINFORMATION|MB_OK);
						}
					}
					HR(rsCoPay->Close());
				}
			}

			//ok, now check for the Allergy Warning, this code is basically copied from above with changes for the recordset, etc
			// Open the recordset
			// (j.gruber 2009-06-02 15:12) - PLID 34450 - take out inactive allergies
			if (AdoFldBool(rsPatient, "DisplayAllergyWarning")) {
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				// (j.jones 2013-08-12 15:52) - PLID 57977 - Used snapshot isolation and changed this query to get the allergies itself and not call the UDF function.
				// This is a left join because if warning is enabled, but they have no allergies, we say "<None>".
				_RecordsetPtr rsAllergy = CreateParamRecordset(GetRemoteDataSnapshot(), 
					"SELECT PersonT.FullName, PatientAllergyQ.AllergyName "
					"FROM PatientsT "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"LEFT JOIN (SELECT EmrDataT.Data AS AllergyName, PatientAllergyT.PersonID "
					"	FROM PatientAllergyT "
					"	LEFT JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
					"	LEFT JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID "
					"	WHERE PatientAllergyT.Discontinued = 0) AS PatientAllergyQ ON PatientsT.PersonID = PatientAllergyQ.PersonID "
					"WHERE PatientsT.PersonID = {INT} "
					"AND PersonT.DisplayAllergyWarning = 1 "
					"ORDER BY PatientAllergyQ.AllergyName", nPatientID);
				if(!rsAllergy->eof) {
					FieldsPtr flds = rsAllergy->Fields;
					CString strAllergyPatientName = AdoFldString(flds, "FullName");
					CString strAllergyList = "";

					while(!rsAllergy->eof) {
						//if we have only one record, and there are no allergies, we show "<None>"
						CString strAllergyName = AdoFldString(flds, "AllergyName", "<None>");
						if(!strAllergyList.IsEmpty()) {
							strAllergyList += ", ";
						}
						strAllergyList += strAllergyName;
						rsAllergy->MoveNext();
					}
						
					CString strAllergyWarning = "This patient has the following allergies:  " + strAllergyList;

					// Display it
					CUserWarningDlg dlgWarning(this);
					// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
					BOOL bKeepWarning = dlgWarning.DoModalWarning(strAllergyWarning, 
						TRUE, strAllergyPatientName + ":", "Allergy Warning", NULL, -1, "");

					//if they turned off the warning, save the change
					if(bKeepWarning == 0) {
						ExecuteParamSql("UPDATE PersonT SET DisplayAllergyWarning = 0 WHERE ID = {INT}", nPatientID);
					}
				}
				rsAllergy->Close();
			}

			// (j.luckoski 2013-03-04 13:54) - PLID 33548 - Display rewards warning
			if (AdoFldBool(rsPatient, "DisplayRewardsWarning")) {
				COleCurrency cyRewards;
				cyRewards = Rewards::GetTotalPoints(nPatientID);
	
				_RecordsetPtr rsRewards = CreateParamRecordset("Select PersonT.DisplayRewardsWarning, PersonT.FullName from PersonT where ID = {INT}", nPatientID);

				if(!rsRewards->eof && cyRewards > g_ccyZero && GetRemotePropertyInt("DisplayRewardsWarning", 0, 0, GetCurrentUserName(), true) == 1 && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {

					FieldsPtr flds = rsRewards->Fields;
					CString strRewards = FormatCurrencyForInterface(cyRewards, FALSE, TRUE);

					FieldPtr fldDispWarning = flds->Item["DisplayRewardsWarning"];

					// If the patient is displaying a warning
					BOOL bDispWarning = AdoFldBool(fldDispWarning);
					if (bDispWarning) {


						CString strRewardsWarning = "This patient has available reward points:  " +  strRewards;

						// Display it
						CUserWarningDlg dlgWarning(this);
						// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
						BOOL bKeepWarning = dlgWarning.DoModalWarning(strRewardsWarning, 
							TRUE, AdoFldString(flds, "FullName") + ":", "Reward Points Warning", NULL, -1, "");


						// If the user asked to change the displaywarning status of the patient
						if((bKeepWarning == 0 && bDispWarning != 0) || (bKeepWarning != 0 && bDispWarning == 0)) {
							// Change it
							/*fldDispWarning->Value = bKeepWarning?true:false;
							rsAllergy->Update();*/
							// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
							ExecuteParamSql("UPDATE PersonT SET DisplayRewardsWarning = {INT} WHERE ID = {INT}", bKeepWarning?1:0, nPatientID);
						}
					}
				}
				HR(rsRewards->Close());
			}

		}
		
		ApplyToExtraInfo(rsPatient);
		
		// (a.walling 2015-01-27 12:14) - PLID 64416
		if (m_ResID == -1 && nPatientID > 0) {
			WarnIfPatientInReschedulingQueue();
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::ApplyCurrentPatient()");
}

void CResEntryDlg::AutoSelectTypePurposeByPatient()
{
	bool bTypeSet = false;
	if(m_nCurPatientID != -25) {	//do nothing for {No Patient Selected}
		if(GetRemotePropertyInt("AutoSelectTypePurpose", 0, 0, "<None>", true) == 1) {
			COleDateTime dtDate = GetDate();
			COleDateTime dtStart(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(),
				m_dtStartTime.GetHour(), m_dtStartTime.GetMinute(), m_dtStartTime.GetSecond());

			//lookup their previous type + purpose
			//TES 4/5/2007 - PLID 25476 - The formatting arguments were not in the correct order, I fixed it.
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			_RecordsetPtr prsAuto = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ID, AptTypeID, PurposeID FROM ("
				"SELECT AppointmentsT.ID, AptTypeID, AppointmentPurposeT.PurposeID, ModifiedDate from AppointmentsT "
							"LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentID "
							"WHERE StartTime = "
							"	(SELECT Max(StartTime) "
							"	FROM AppointmentsT "
							"WHERE StartTime < {OLEDATETIME} AND PatientID = {INT} AND AppointmentsT.Status <> 4) "
							"AND PatientID = {INT} AND Status <> 4 "
							") SubQ "
							"WHERE ModifiedDate = (SELECT Max(ModifiedDate) FROM AppointmentsT "
							"WHERE StartTime = "
							"(SELECT Max(StartTime) "
							" FROM AppointmentsT "
							"WHERE StartTime < {OLEDATETIME} AND PatientID = {INT} AND AppointmentsT.Status <> 4) "
							// (c.haag 2006-12-07 16:14) - PLID 23767 - Filter on our template type if we have one
							"AND {INT} IN (-1, AptTypeID) "
							//////////////////////////////////////////////////////////////////////////////////////
							"AND PatientID = {INT} AND Status <> 4) ", dtStart, m_nCurPatientID, m_nCurPatientID,
								dtStart, m_nCurPatientID, m_nTemplateItemAptTypeID, m_nCurPatientID);

			// Reset our type and purpose selections
			SetCurAptType(-1);
			m_adwPurposeID.RemoveAll();
			EnsurePurposeComboVisibility();
			GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);

			while(!prsAuto->eof) {
				//the recordset gives us a recordset which contains the appt id, the type id, and a purposeid.  It's going to 
				//have the id / type id the same in every row, so only need to pull those once, then just read the purposes.
				//This would be faster than opening 1 recordset to find the type and another to find the purposes
				_variant_t var;
				if(!bTypeSet) {
					var = prsAuto->Fields->Item["AptTypeID"]->Value;
					if(var.vt == VT_I4) {
						//set the type id
						//This may be inactive.
						SetCurAptType(VarLong(var));
					}
					else {
						SetCurAptType(-1);
					}
					bTypeSet = true;
					//TES 6/4/2004: Requery the appointment purposes now, so we know which ones are valid.
					RequeryAptPurposes();
				}

				//now add to the purpose list
				var = prsAuto->Fields->Item["PurposeID"]->Value;
				if(var.vt == VT_I4) {
					//Only add those purposes which are valid here.
					if(m_dlAptPurpose->FindByColumn(0, var, 0, false) != -1) {
						// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
						// replaced it with TryAddPurposeIDToAppt
						TryAddPurposeIDToAppt(VarLong(var));
					}
				}

				prsAuto->MoveNext();
			}
			if (GetCurAptTypeID() == -1)
				GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
			else
				GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);

			//now show the purposes on screen
			EnsurePurposeComboVisibility();
			RefreshPurposeCombo();
		}
	}

	// (c.haag 2006-12-07 16:12) - PLID 23767 - If this is a template block, and we haven't
	// assigned an appointment type to the appointment yet, and we do have an appointment
	// type to assign...then assign it now.
	if (!bTypeSet && -1 != m_nTemplateItemAptTypeID) {
		SetCurAptType(m_nTemplateItemAptTypeID);
		GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);
		bTypeSet = true;

		// Clear our purpose ID list and requery the purpose combo in lieu of the newly
		// selected type
		m_adwPurposeID.RemoveAll();
		RequeryAptPurposes();
	}
}

void CResEntryDlg::OnPinResentry()
{
	try
	{
		// (c.haag 2015-11-12) - PLID 67577 - Call the business logic
		HandleResEntryPinChange();
	}
	CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::HandleResEntryPinChange()
{
	try
	{
		// (a.walling 2008-06-03 10:06) - PLID 27686 - Use LR_SHARED
		long nPropValue;
		if (IsDlgButtonChecked(IDC_PIN_RESENTRY))
		{
			m_btnPin.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance,
				MAKEINTRESOURCE(IDI_PIN_DOWN),
				IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED));
			if (!m_Pinned) {
				m_Pinned = new CKeepDialogActive(this);
			}
			nPropValue = 1;
		}
		else
		{
			m_btnPin.SetIcon((HICON)LoadImage(AfxGetApp()->m_hInstance,
				MAKEINTRESOURCE(IDI_PIN_UP),
				IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_SHARED));
			if (m_Pinned) {
				delete m_Pinned;
				m_Pinned = NULL;
			}
			nPropValue = 0;
		}

		// (z.manning 2013-11-18 17:56) - PLID 59597 - We now remember the state of the pin button
		SetRemotePropertyInt("ResEntryPinned", nPropValue, 0, GetCurrentUserName());
	}
	// (c.haag 2015-11-12) - PLID 67577 - Log that we touched on this function before passing it to the caller
	NxCatchAllSilentCallThrow(Log("Exception being thrown from %s", __FUNCTION__))
}

void CResEntryDlg::OnExtraBold1() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD1, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[0] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold2() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD2, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[1] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold3() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD3, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[2] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold4() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD4, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[3] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold5() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD5, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[4] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold6() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD6, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[5] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold7() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD7, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[6] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::OnExtraBold8() 
{
	try {
		BOOL tmpCheck;
		GetDlgItemCheck(IDC_EXTRA_BOLD8, tmpCheck);
		m_pSchedulerView->m_nExtraInfoStyles[7] = tmpCheck;
		
		//SetPropertyInt("ResEntryMoreInf", tmpCheck, 0);
		m_CheckboxesUpdated = true;
		ApplyToResBox();
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::RepopulateExtraInfoCombos()
{
	try {
		long nResourceIDs[] = { IDC_EXTRA_FIELDS1, IDC_EXTRA_FIELDS2,
			IDC_EXTRA_FIELDS3, IDC_EXTRA_FIELDS4, IDC_EXTRA_FIELDS5,
			IDC_EXTRA_FIELDS6, IDC_EXTRA_FIELDS7, IDC_EXTRA_FIELDS8
		};
		long nOldSel[8];

		// Save the old dropdown selections, clear the content and fill the list
		// with names
		for (long i=0; i < 8; i++)
		{
			CComboBox* pCombo = ((CComboBox*)GetDlgItem(nResourceIDs[i]));
			nOldSel[i] = pCombo->GetCurSel();
			pCombo->ResetContent();
			pCombo->AddString("ID");
			pCombo->AddString("Home");
			pCombo->AddString("Work");
			pCombo->AddString("B-Day");
			pCombo->AddString("Mobile");
			pCombo->AddString("Pref:");
			pCombo->AddString("Pt. Bal.");
			pCombo->AddString("Ins. Bal.");
			pCombo->AddString("Referral");
			pCombo->AddString("Duration");
			pCombo->AddString("Pager");
			pCombo->AddString("Oth. Ph.");
			pCombo->AddString("Email");
			pCombo->AddString("Last LM");
			pCombo->AddString("Input By");
			pCombo->AddString("Mod. By");
			pCombo->AddString("Mod.Date");
			// (j.gruber 2012-08-06 16:12) - PLID 51972 - change the name
			pCombo->AddString("Med Pri Ins");
			pCombo->AddString("Med Pri CoPay");
			pCombo->AddString("Med Sec Ins");
			pCombo->AddString("Med Sec CoPay");
			//DRT 9/10/2008 - PLID 31127
			pCombo->AddString("Checked In");
			pCombo->AddString("Checked Out");
			//TES 11/12/2008 - PLID 11465 - Added InputDate
			pCombo->AddString("Input Date");
			//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
			pCombo->AddString("Ref Phy");
			pCombo->AddString("Ref Phy Tel");
			pCombo->AddString("Ref Phy Fax");
			// (z.manning 2008-11-19 16:40) - PLID 12282 - Age
			pCombo->AddString("Age");
			// (c.haag 2008-11-20 16:08) - PLID 31128 - Insurance Referral summary
			pCombo->AddString("Ins. Referrals");
			// (j.armen 2011-07-01 15:44) - PLID 44205 - Added confirmed by
			pCombo->AddString("Confirmed By");
			pCombo->AddString("Arrival Time"); // (z.manning 2011-07-01 17:03) - PLID 23273
			pCombo->AddString("Cancelled Date"); // (j.luckoski 2012-04-30 10:21) - PLID 11597 - Added Cancelled Date
			pCombo->AddString("Appt. Pri. Ins."); // (j.gruber 2012-08-06 11:33) - PLID 51926
			pCombo->AddString("Appt. Sec. Ins."); // (j.gruber 2012-08-06 11:33) - PLID 51926
			pCombo->AddString("Appt. Ins. List"); // (j.gruber 2012-08-06 11:33) - PLID 51926
			// (j.gruber 2013-01-08 15:21) - PLID 54497 - only add the appt's ref phys if the setting is set - changed to always
			pCombo->AddString("Appt. Ref. Phys.");	
			// (s.tullis 2014-01-21 15:45) - PLID 49748 - Add nexweb activation code to the  extra info fields
			pCombo->AddString("NexWeb Code");
			// (s.tullis 2014-01-24 16:00) - PLID 60470 - Add nexweb security code experation date to the  extra info fields
			pCombo->AddString("NexWeb Exp. Date");

			// (c.haag 2008-11-24 14:48) - PLID 31128 - Make sure the combo is wide enough to fit all the
			// existing items (including the # at the end of "Drivers License"
			pCombo->SetDroppedWidth(125);
		}

		//TES 2004-1-21: Fill the names of the gen 1 custom fields in the extra info combos.
		// (c.haag 2004-02-27 13:31) - I basically changed almost everything Tom wrote.
		// (b.cardillo 2006-07-28 12:03) - PLID 21674 - These records were being pulled without a 
		// specified sort order, and so sometimes wouldn't be 1,2,3,4 but would be random.  This 
		// is no good because our combo boxes are strictly expecting the fields to be in the order 
		// defined by the ExtraFields enum type.  So I've just added the ORDER BY clause.
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rsCustom = CreateRecordsetStd(GetRemoteDataSnapshot(), "SELECT Name FROM CustomFieldsT Where ID IN (1,2,3,4) ORDER BY ID ASC");
		FieldsPtr fCustom = rsCustom->Fields;
		while(!rsCustom->eof) {
			for (i=0; i < 8; i++)
			{
				((CComboBox*)GetDlgItem(nResourceIDs[i]))->AddString(AdoFldString(fCustom, "Name"));
			}
			rsCustom->MoveNext();
		}

		// Restore the old dropdown selections
		for (i=0; i < 8; i++)
		{
			((CComboBox*)GetDlgItem(nResourceIDs[i]))->SetCurSel(nOldSel[i]);
		}

		// (b.cardillo 2006-07-06 18:05) - PLID 21342 - Now that we've just reflected the current 
		// custom field names, we reset our "changed" \flag to FALSE.
		m_bG1CustomFieldNameChanged = FALSE;
	}
	CResEntryDlg_NxCatchAll("Error filling custom combo boxes");
}

void CResEntryDlg::SetModifiedFlag(bool bModified /* = true */)
{
	if (bModified && !m_Modified)
		m_Modified = new CKeepDialogActive(this);

	if (!bModified && m_Modified)
	{
		delete m_Modified;
		m_Modified = NULL;
	}

	//TES 10/27/2010 - PLID 40868 - Any time we get modified, clear our calculated TemplateItemID, it may no longer be valid.
	if(bModified) {
		RefreshTemplateItemID();
	}
}

bool CResEntryDlg::GetModifiedFlag()
{
	return (m_Modified != NULL);
}


// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CResEntryDlg::OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		SetModifiedFlag();

		*pResult = 0;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

bool CResEntryDlg::AllowChangeView()
{
	// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
	if(IsWindowVisible() && (m_Pinned || m_Modified)) {
		// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
		HandleCancelAndCloseReservation();
		if(IsWindowVisible()) {
			//This means we prompted the user, and they said No.
			return false;
		}
	}
	return true;
}

void CResEntryDlg::HighlightSheetTimeColumn(BOOL bHighlight)
{
	CNxSchedulerDlg *pSheet = NULL;
	if (m_pSchedulerView && (pSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet()) &&
		!IsEvent())
	{
		if (pSheet->m_pSingleDayCtrl) {
			if (bHighlight)
			{
				//this is temporary and will not be tracked in m_aryColoredTimes in NxSchedulerDlg
				pSheet->m_pSingleDayCtrl.SetTimeButtonColors(m_dtStartTime,
					m_dtEndTime, RGB(192, 192, 192));
			}
			else
			{
				// (j.jones 2014-12-03 16:05) - PLID 64276 - Don't reset the time button colors from here,
				// instead tell the scheduler view to do it. Because bReColorTimes is true, any special
				// colors like overridden slots will redraw the proper color.
				pSheet->ResetTimeColors();
			}
		}
	}
}

CResEntryDlg::~CResEntryDlg()
{
	if(m_Modified) delete m_Modified;
	if(m_Pinned) delete m_Pinned;
	delete &m_schedAuditItems;
}

void CResEntryDlg::OnSelChosenRequestedResource(long nRow) 
{
	try {
		SetModifiedFlag();
		if(nRow != -1 && m_dlRequestedResource->GetValue(nRow, 0).vt == VT_NULL) m_dlRequestedResource->CurSel = -1;
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::EnableGoToPatient(BOOL bEnabled)
{
	CMenu *pMenu = GetMenu();

	BOOL bPatEnable = GetCurPatientID() != -25 ? TRUE : FALSE;
	GetDlgItem(IDC_GOTOPATIENT)->EnableWindow(bPatEnable);
	// (z.manning 2008-11-24 09:28) - PLID 31130 - Even though the button may not be visible right here
	// we still want to enable the button as long as we have a valid patient so that the shortcut key
	// still works.
	GetDlgItem(IDC_GOTO_INSURANCE)->EnableWindow(bPatEnable);

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_GOTOPATIENT) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_GOTOPATIENT,bEnabled ? MF_ENABLED : MF_GRAYED);
					}
					//also disable the case history stuff
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_CASE_HISTORY) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_CASE_HISTORY, bEnabled ? MF_ENABLED : MF_GRAYED);
					}
					// (j.jones 2007-11-21 11:07) - PLID 28147 - and disable the allocation option
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_INV_ALLOCATION) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_INV_ALLOCATION, bEnabled ? MF_ENABLED : MF_GRAYED);
					}

					// (j.jones 2008-03-21 10:31) - PLID 29309 - and disable the ability to create an order
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_INV_ORDER) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_INV_ORDER, bEnabled ? MF_ENABLED : MF_GRAYED);
					}

					// (z.manning 2008-11-24 10:35) - PLID 31130 - Save and goto insurance
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == ID_GOTO_INSURANCE) {
						pMenu->GetSubMenu(i)->EnableMenuItem(ID_GOTO_INSURANCE, bEnabled ? MF_ENABLED : MF_GRAYED);
					}
					// (r.gonet 09/19/2013) - PLID 58416 - Disable the medication history stuff
					if(pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_VIEW_MEDICATION_HISTORY) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_VIEW_MEDICATION_HISTORY, bEnabled ? MF_ENABLED : MF_GRAYED);
					}
				}
			}
		}
	}
}

// (j.jones 2007-11-21 10:59) - PLID 28147 - added ability to create an inventory allocation,
// but this function will disable it if they don't have inventory or create permissions
void CResEntryDlg::DisableSaveCreateInvAllocation()
{
	CMenu *pMenu = GetMenu();

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_INV_ALLOCATION) {
						pMenu->GetSubMenu(i)->RemoveMenu(j,MF_BYPOSITION);
						break;
					}
				}
			}
		}
	}
}

// (j.jones 2008-03-18 13:54) - PLID 29309 - added ability to create an inventory order,
	// but this function will disable it if they don't have advanced inventory or create permissions
void CResEntryDlg::DisableSaveCreateInvOrder()
{
	CMenu *pMenu = GetMenu();

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_INV_ORDER) {
						pMenu->GetSubMenu(i)->RemoveMenu(j,MF_BYPOSITION);
						break;
					}
				}
			}
		}
	}
}

void CResEntryDlg::DisableSaveCreateCaseHistory()
{
	CMenu *pMenu = GetMenu();

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_CASE_HISTORY) {
						pMenu->GetSubMenu(i)->RemoveMenu(j,MF_BYPOSITION);
						break;
					}
				}
			}
		}
	}
}

// (r.gonet 09/20/2013) - PLID 58416 - Remove the medication history menu item from the Action menu.
void CResEntryDlg::DisableSaveViewMedicationHistory()
{
	CMenu *pMenu = GetMenu();

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_VIEW_MEDICATION_HISTORY) {
						pMenu->GetSubMenu(i)->RemoveMenu(j,MF_BYPOSITION);
						break;
					}
				}
			}
		}
	}
}

void CResEntryDlg::EnableCancelAppointmentMenuOption(BOOL bEnabled)
{
	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	CMenu *pMenu = GetMenu();
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_DELETE_BTN) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_DELETE_BTN, bEnabled ? MF_ENABLED : MF_GRAYED);
					}
				}
			}
		}
	}
}

void CResEntryDlg::ToggleSaveCreateCaseHistory(BOOL bSaveNew)
{
	//if bSaveNew = FALSE, then remove "Save & Create Case History"
	//if bSaveNew = TRUE, then remove "Save & Open Case History"

	CMenu *pMenu = GetMenu();

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {

					CString strText;
					pMenu->GetSubMenu(i)->GetMenuString(j, strText, MF_BYPOSITION);

					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_CASE_HISTORY) {

						if(bSaveNew)
							pMenu->GetSubMenu(i)->ModifyMenu(j,MF_BYPOSITION,IDC_SAVE_CREATE_CASE_HISTORY,"Save && Create Case &History\tAlt+H");
						else
							pMenu->GetSubMenu(i)->ModifyMenu(j,MF_BYPOSITION,IDC_SAVE_CREATE_CASE_HISTORY,"Save && Open Case &History\tAlt+H");
					}
				}
			}
		}
	}
}

BOOL CResEntryDlg::DoesCaseHistoryExist(long nResID)
{
	if(nResID == -1)
		return FALSE;

	// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
	return ReturnsRecordsParam("SELECT ID FROM CaseHistoryT WHERE AppointmentID = {INT}",nResID);	
}

void CResEntryDlg::EnsureResourceComboVisibility()
{
	// (c.haag 2005-02-18 13:46) - PLID 15623 - Make sure the dropdown
	// is visible if one or zero items are selected, and invisible
	// if more than one item is selected.
	if (m_arydwSelResourceIDs.GetSize() < 2) {
		GetDlgItem(IDC_APTRESOURCE_COMBO)->ShowWindow(SW_SHOW);
	} else {
		GetDlgItem(IDC_APTRESOURCE_COMBO)->ShowWindow(SW_HIDE);
	}
}

void CResEntryDlg::EnsurePurposeComboVisibility()
{
	// (c.haag 2005-02-18 13:46) - PLID 15623 - Make sure the dropdown
	// is visible if one or zero items are selected, and invisible
	// if more than one item is selected.
	if (m_adwPurposeID.GetSize() < 2) {
		GetDlgItem(IDC_APTPURPOSE_COMBO)->ShowWindow(SW_SHOW);
	} else {
		GetDlgItem(IDC_APTPURPOSE_COMBO)->ShowWindow(SW_HIDE);
	}
}

BOOL CResEntryDlg::EnsureValidEndTime(COleDateTime &dtEndTime)
{
	// If the new 'end time' is not equal to the same day as the old 'end time,'
	// we must have surpassed midnight, so lets snap it to 11:59:59.
	COleDateTime dt = m_nxtStart->GetDateTime();
	if (dtEndTime.GetDay() != dt.GetDay())
	{
		dtEndTime = dt;
		dtEndTime.SetTime(23, 59, 59);
	}

	COleDateTime dtCurrentEnd = m_nxtEnd->GetDateTime();

	// (d.lange 2013-08-09 13:55) - PLID 57845 - Compare the hour and minutes of the calculated end time and current end time
	if(dtCurrentEnd.GetHour() != dtEndTime.GetHour() || dtCurrentEnd.GetMinute() != dtEndTime.GetMinute()) {
		//something has changed
		//DRT 7/8/03 - If we're editing this, we want to warn them before we actually change anything.
		CKeepDialogActive kda(this);
		if(!m_NewRes) {
			if(MsgBox(MB_YESNO, "Practice is attempting to update default durations for this appointment.  Are you sure you wish to change your duration?") == IDNO) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL CResEntryDlg::EnsureValidArrivalTime(COleDateTime& dtArrivalTime)
{
	// Make sure we're on the same day as the start time and if we aren't,
	// let's set the time to midnight
	COleDateTime dtStart = m_nxtStart->GetDateTime();
	if(dtArrivalTime.GetDay() != dtStart.GetDay() || dtArrivalTime < 0) {
		dtArrivalTime = dtStart;
		dtArrivalTime.SetTime(0,0,0);
	}

	// ZM - 4/25/06 - On second thought, there's a decent chance that they were just warned
	// about the end time changing, so this message box could be overkill.  Plus, arrival time
	// is not that critical of a property of appointments, so I don't think it's better to just
	// leave out this warning altogether.
	/*if(m_bShowArrivalTime) {
		COleDateTime dtCurrentArrivalTime = m_nxtArrival->GetDateTime();
		if(dtCurrentArrivalTime != dtArrivalTime) {
			CKeepDialogActive kda(this);
			if(!m_NewRes) {
				if(MsgBox(MB_YESNO, "Practice is attempting to update the arrival time for this appointment.  Are you sure you wish to change this?") == IDNO) 
				{
					return FALSE;
				}
			}
		}
	}*/
	return TRUE;
}

COleDateTime CResEntryDlg::GetDate()
{
	if(IsEvent()) {
		//TES 06/30/2005 - PLID 15781 - This is a workaround for a bug in the DTPicker, where if you type in a date and then
		//immediately hit enter, it may return the previous date value. So, if it has focus, take focus away, which will 
		//cause it to commit its change to itself, then set focus back, and return the value.

		
		// (a.walling 2008-05-13 10:34) - PLID 27591 - Use GetSafeHwnd now
		if(::GetFocus() == (HWND)m_dateEventStart.GetSafeHwnd()) {
			::SetFocus(NULL);
			::SetFocus((HWND)m_dateEventStart.GetSafeHwnd());
		}
		return m_dateEventStart.GetValue();
	}
	else {
		return m_date.GetValue();
	}
}

void CResEntryDlg::ShowArrivalTime(BOOL bShow)
{
	// (z.manning, 08/10/2006) - PLID 21896 - All of these fields should be invisible on an event,
	// so none of this is necessary, plus we don't want to inadvertantly show the arrival time field.
	if(IsEvent()) {
		return;
	}

	CWnd* pwndStartTime = GetDlgItem(IDC_START_TIME_BOX);
	CWnd* pwndEndTime = GetDlgItem(IDC_END_TIME_BOX);
	CWnd* pwndEndTimeLabel = GetDlgItem(IDC_STATIC_END);
		
	// Get the coordinates of the necessary fields.
	CRect rcStartTime, rcEndTime, rcEndTimeLabel, rcArrivalTime, rcDate;
	pwndStartTime->GetWindowRect(rcStartTime);
	pwndEndTime->GetWindowRect(rcEndTime);
	pwndEndTimeLabel->GetWindowRect(rcEndTimeLabel);
	ScreenToClient(rcStartTime);
	ScreenToClient(rcEndTime);
	ScreenToClient(rcEndTimeLabel);

	if(bShow) {
		GetDlgItem(IDC_ARRIVAL_TIME_BOX)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_ARRIVAL)->ShowWindow(SW_SHOW);
		
		GetDlgItem(IDC_ARRIVAL_TIME_BOX)->GetWindowRect(rcArrivalTime);
		ScreenToClient(rcArrivalTime);

		// Move/resize the start time, end time, and end time label.
		int nWidth = rcArrivalTime.Width();
		pwndStartTime->MoveWindow(rcStartTime.left, rcStartTime.top, nWidth, rcStartTime.Height());
		pwndEndTime->MoveWindow(rcStartTime.left + nWidth + 6, rcEndTime.top, nWidth, rcEndTime.Height());
		pwndEndTimeLabel->MoveWindow(rcStartTime.left + nWidth + 6, rcEndTimeLabel.top, rcEndTimeLabel.Width(), rcEndTimeLabel.Height());
	}
	else {
		GetDlgItem(IDC_ARRIVAL_TIME_BOX)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_ARRIVAL)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_DATE)->GetWindowRect(rcDate);
		ScreenToClient(rcDate);

		// Move/resize the start time, end time, and end time label.
		int nOffset = rcDate.right - rcEndTime.right;
		if(nOffset > 0) {
			int nResize = nOffset / 2 - 3;
			pwndStartTime->MoveWindow(rcStartTime.left, rcStartTime.top, rcStartTime.Width() + nResize, rcStartTime.Height());
			//pwndEndTime->MoveWindow(rcEndTime.left + nOffset - nResize, rcEndTime.top, rcEndTime.Width() + nResize, rcEndTime.Height());
			//pwndEndTimeLabel->MoveWindow(rcEndTimeLabel.left + nOffset - nResize, rcEndTimeLabel.top, rcEndTimeLabel.Width(), rcEndTimeLabel.Height());
			// Let's just make end time the same size of the date combo right above it.
			pwndEndTime->MoveWindow(rcDate.left, rcEndTime.top, rcDate.Width(), rcEndTime.Height());
			pwndEndTimeLabel->MoveWindow(rcDate.left, rcEndTimeLabel.top, rcEndTimeLabel.Width(), rcEndTimeLabel.Height());
		}
	}
	m_bShowArrivalTime = bShow;
}

void CResEntryDlg::RevertResAppearanceToTemplateItem()
{
	//
	// (c.haag 2006-12-05 09:49) - PLID 23666 - This makes the reservation object look like a template
	// line item block. m_TemplateItemID must not be -1 for this to work.
	//
	// If you use this function elsewhere, please review that the call to KillThisResObject is still
	// warranted.
	//
	//TES 10/27/2010 - PLID 40868 - Check the Initial value, not the Calculated value, we're only concerned with the block that
	// was originally clicked on.
	ASSERT(-1 != m_nInitialTemplateItemID);
	try {
		if (NULL != m_Res) { // (c.haag 2007-01-11 14:35) - Safety check
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT Name, Color FROM TemplateItemT INNER JOIN TemplateT ON TemplateT.ID = TemplateItemT.TemplateID WHERE TemplateItemT.ID = {INT}", m_nInitialTemplateItemID);
			FieldsPtr f = prs->Fields;
			if (prs->eof) {
				// (c.haag 2006-12-05 10:10) - If there is no template, we should not be visible!
				KillThisResObject(TRUE); // (j.luckoski 2012-05-02 16:33) - PLID 11597 - Pass true
			} else {
				MakeResLookLikeTemplateItem(m_Res, AdoFldString(f, "Name"), AdoFldLong(f, "Color"));
			}
		}
	}
	CResEntryDlg_NxCatchAll("Error restoring template item appearance");
}

void CResEntryDlg::CalculateDefaultAptTypeByTemplateItemID()
{
	//
	// (c.haag 2006-12-05 13:24) - PLID 23767 - If this reservation is based
	// off a block template item, and an appointment type has not been designated,
	// then we need to try to auto-fill the type and purpose to something that
	// fits rigid template rules
	//
	// Due to speed, tech support concerns, and the constraints of time, I want
	// to implement a very simple version of this now, and improve on it over 
	// future scopes. "Simple" means ignoring purposes, ignoring the any/all 
	// conditional in the rules, and ignoring the flag to prevent a user from
	// scheduling an appointment. We basically filter on any appointment type that
	// does not violate any rules for the given line item's template, and if there
	// is one and only one type that can be made, then we assign it to
	// m_nTemplateItemAptTypeID. When assigned, the legacy logic will be affected.
	//
	const long nTemplateItemID = m_Res.GetTemplateItemID();
	m_nTemplateItemAptTypeID = -1;
	if (-1 != nTemplateItemID && -1 == GetCurAptTypeID()) {

		// (c.haag 2006-12-05 15:46) - Fail right away if there is a "Any Type" rule
		// (a.walling 2010-10-18 18:00) - PLID 40965 - Use ReturnsRecordsParam
		if (ReturnsRecordsParam("SELECT TOP 1 TemplateRuleT.ID FROM TemplateRuleT "
			"INNER JOIN TemplateT ON TemplateT.ID = TemplateRuleT.TemplateID "
			"WHERE AllAppts = 1 AND TemplateT.ID IN (SELECT TemplateID FROM TemplateItemT WHERE TemplateItemT.ID = {INT})",
			nTemplateItemID))
		{
			return;
		}

		// (c.haag 2006-12-05 15:17) - Pull all available types
		CArray<long, long> aTypes;
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(
			"DECLARE @TemplateItemID INT \r\n "
			"SET @TemplateItemID = {INT} \r\n "
			" \r\n "
			"SELECT ID FROM AptTypeT "
			//"/* Include all appointment types that are on a template rule's blacklist; that is, the list of types \r\n "
			//"that will NOT invoke a warning */ "
			"WHERE ( "
			"	NOT EXISTS( "
			"		SELECT TOP 1 TemplateRuleDetailsT.ID FROM TemplateRuleDetailsT  "
			"		INNER JOIN TemplateRuleT ON TemplateRuleT.ID = TemplateRuleID   "
			"		INNER JOIN TemplateT ON TemplateT.ID = TemplateRuleT.TemplateID  "
			"		WHERE TemplateT.ID IN (SELECT TemplateID FROM TemplateItemT WHERE TemplateItemT.ID = @TemplateItemID)  "
			"		AND ObjectType = 101  "
			"	) OR (  "
			"		AptTypeT.ID IN (  "
			"			SELECT ObjectID FROM TemplateRuleDetailsT  "
			"			INNER JOIN TemplateRuleT ON TemplateRuleT.ID = TemplateRuleID   "
			"			INNER JOIN TemplateT ON TemplateT.ID = TemplateRuleT.TemplateID  "
			"			WHERE TemplateT.ID IN (SELECT TemplateID FROM TemplateItemT WHERE TemplateItemT.ID = @TemplateItemID)  "
			"			AND ObjectType = 101  "
			"		)  "
			"	)  "
			")  "
			//"/* Exclude all appointment types that are explicitly blocking the creation of an appointment with a type */  "
			"UNION ALL SELECT ID FROM AptTypeT "
			"WHERE "
			"EXISTS( "
			"	SELECT TOP 1 TemplateRuleDetailsT.ID FROM TemplateRuleDetailsT "
			"	INNER JOIN TemplateRuleT ON TemplateRuleT.ID = TemplateRuleID "
			"	INNER JOIN TemplateT ON TemplateT.ID = TemplateRuleT.TemplateID "
			"	WHERE TemplateT.ID IN (SELECT TemplateID FROM TemplateItemT WHERE TemplateItemT.ID = @TemplateItemID) "
			"	AND ObjectType = 1 "
			") "
			"AND ID NOT IN ( "
			"	SELECT ObjectID FROM TemplateRuleDetailsT "
			"	INNER JOIN TemplateRuleT ON TemplateRuleT.ID = TemplateRuleID  "
			"	INNER JOIN TemplateT ON TemplateT.ID = TemplateRuleT.TemplateID "
			"	WHERE TemplateT.ID IN (SELECT TemplateID FROM TemplateItemT WHERE TemplateItemT.ID = @TemplateItemID) "
			"	AND ObjectType = 1 "
			") ", nTemplateItemID
			);

		if (1 == prs->GetRecordCount()) {
			// (c.haag 2006-12-05 16:10) - If we get here, it means there is one and only one
			// possible appointment type we can schedule here in our narrow result set
			m_nTemplateItemAptTypeID = AdoFldLong(prs, "ID");
		}

	} // if (-1 != nTemplateItemID && -1 == GetCurAptTypeID()) {
}

void CResEntryDlg::TryAutoFillPurposes()
{
	// (c.haag 2006-12-07 16:39) - PLID 23767 - I moved these checks into their own function
	// so we could use them in more than one place

	//TES 8/7/03: If this is a follow-up, auto-fill the purposes.
	// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
	if(GetRemotePropertyInt("AutoFillFollowupPurposes", 1, 0, "<None>", true) && GetCurAptTypeCategory() == PhaseTracking::AC_FOLLOW_UP) {
		COleDateTime dtStartTime(m_ActiveDate.GetYear(), m_ActiveDate.GetMonth(), m_ActiveDate.GetDay(), m_dtStartTime.GetHour(), m_dtStartTime.GetMinute(), m_dtStartTime.GetSecond());
		//Select all purposes that were in the last surgery, and are available for this type.
		// (c.haag 2008-12-09 12:32) - PLID 32376 - Exclude inactive procedures
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rsPurposes = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT PurposeID FROM AppointmentPurposeT WHERE PurposeID IN (SELECT AptPurposeID FROM "
			"AptPurposeTypeT WHERE AptTypeID = {INT}) AND AppointmentID = "
			"(SELECT TOP 1 ID FROM AppointmentsT WHERE ShowState <> 4 AND PatientID = {INT} AND "
			"AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) AND StartTime < {OLEDATETIME} ORDER BY StartTime DESC) "
			"AND PurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ", 
			GetCurAptTypeID(), GetCurPatientID(), dtStartTime
		);
		while(!rsPurposes->eof) {
			//Only add those purposes which are valid here.
			long nPurposeID = AdoFldLong(rsPurposes, "PurposeID");
			if(m_dlAptPurpose->FindByColumn(0, nPurposeID, 0, false) != -1) {
				// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
				// replaced it with TryAddPurposeIDToAppt
				TryAddPurposeIDToAppt(nPurposeID);
			}
			rsPurposes->MoveNext();
		}
		//Reflect our new purposes on screen.
		RefreshPurposeCombo();
	}
	else if(GetRemotePropertyInt("AutoFillTrackingBased", 0, 0, "<None>", true)) {
		//Find all procedures being tracked for an active ladder for this patient.
		//TES 12/31/2004 - Changed this query to group by procedureid.
		// (a.walling 2007-02-20 10:17) - PLID 24697 - Only look at committed PICs (and made this sql much more readable)
		// (c.haag 2008-12-09 12:21) - PLID 32376 - This will not add inactive procedures
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		_RecordsetPtr rsPurposes = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT   ProcedureID "
			"FROM     (SELECT ProcedureID "
					  "FROM   ProcedureT "
							 "INNER JOIN ProcInfoDetailsT "
							   "ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID "
							 "INNER JOIN ProcInfoT "
							   "ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID "
							 "INNER JOIN PicT "
							   "ON ProcInfoT.ID = PicT.ProcInfoID "
							 "INNER JOIN LaddersT "
							   "ON ProcInfoT.ID = LaddersT.ProcInfoID "
							 "INNER JOIN LadderStatusT "
							   "ON Ladderst.Status = LadderStatusT.ID "
					  "WHERE  ProcedureT.Inactive = 0 AND (ProcInfoDetailsT.Chosen = 1 "
							   "OR (ProcedureT.MasterProcedureID IS NULL  "
								   "AND NOT EXISTS (SELECT ID "
												   "FROM   ProcInfoDetailsT OtherDetails "
												   "WHERE  OtherDetails.ProcInfoID = ProcInfoT.ID "
														  "AND Chosen = 1 "
														  "AND ProcedureID IN (SELECT ID "
																			  "FROM   ProcedureT DetailProcs "
																			  "WHERE  DetailProcs.MasterProcedureID = ProcedureT.ID)))) "
							 "AND PicT.IsCommitted = 1 "
							 "AND LadderStatusT.IsActive = 1 "
							 "AND LaddersT.PersonID = {INT} "
							 "AND ProcInfoDetailsT.ProcedureID IN (SELECT AptPurposeID "
																  "FROM   AptPurposeTypeT "
																  "WHERE  AptTypeID = {INT})) AS SubQ "
			"GROUP BY ProcedureID", 
			GetCurPatientID(), GetCurAptTypeID());
		while(!rsPurposes->eof) {
			long nPurposeID = AdoFldLong(rsPurposes, "ProcedureID");
			if(m_dlAptPurpose->FindByColumn(0, nPurposeID, 0, false) != -1) {
				// (j.jones 2008-02-21 10:49) - PLID 29048 - removed call to m_adwPurposeID.Add() and
				// replaced it with TryAddPurposeIDToAppt
				TryAddPurposeIDToAppt(nPurposeID);
			}
			rsPurposes->MoveNext();
		}
		//Reflect our new purposes on screen.
		RefreshPurposeCombo();
	}
}

// (j.jones 2008-02-21 10:46) - PLID 29048 - added TryAddPurposeIDToAppt which will
// add the purpose to the appt. only if it doesn't already exist
BOOL CResEntryDlg::TryAddPurposeIDToAppt(long nNewPurposeID)
{
	//search and see if it is already in the m_adwPurposeID list
	for (int i=0; i<m_adwPurposeID.GetSize();i++) {
		if((long)(m_adwPurposeID[i]) == nNewPurposeID) {
			//do not add, return false to reflect this
			return FALSE;
		}
	}

	//if not found, it is a unique purpose ID, so add it to our list,
	//and our local list
	m_adwPurposeID.Add(nNewPurposeID);
	return TRUE;
}

void CResEntryDlg::OnGotoInsurance()
{
	try
	{
		SaveAndGotoPatient(PatientsModule::InsuranceTab);

	}CResEntryDlg_NxCatchAll("CResEntryDlg::OnGotoInsurance");
}

// (z.manning 2008-11-24 11:20) - PLID 31130 - Added a utility function to save and then 
// open the patients module to the given tab or the default tab if -1 is specified.
void CResEntryDlg::SaveAndGotoPatient(short nPatientTab)
{
	long nPatientID;
	nPatientID = GetCurPatientID();
	if (nPatientID != -25)
	{
		//Save the Appointment
		// (j.jones 2010-01-04 10:34) - PLID 32935 - added AppointmentSaveAction to determine what
		// action is being performed after saving the appointment
		if (ValidateSaveClose(asaNormal)) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {
				//We don't have to set the Active patient because it is already done when you switch the
				//patient datalist

				// CAH 4/24: True or not, it does not work having
				// this commented out.
				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == AfxMessageBox(
						"The appointment was saved correctly, but this patient is not in the current lookup.  If you proceed, Practice will "
						"clear the lookup before switching to the Patients module.\r\n\r\n"
						"Would you like to clear the lookup and proceed?", MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - No need to check for failure, we wouldn't have been able to open the ResEntryDlg
				// if we couldn't access this patient.
				pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID);
				
				
				//JM: I took this out because per the discussion in the OnSelChosenPatientList Function
				//it speeds it up a lot to not do it
				pMainFrame->UpdateAllViews();				

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

				// (z.manning 2008-11-24 11:27) - PLID 31130 - Is there a specific tab we want to open?
				if(nPatientTab != -1) {
					CPatientView* pPatView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
					if(pPatView != NULL) {
						pPatView->SetActiveTab(nPatientTab);
					}
				}
			}//end if MainFrame
			else {
				AfxThrowNxException("Cannot Open MainFrame");
			}//end else pMainFrame
		}
	}
}

void CResEntryDlg::OnSelChosenMoveup(LPDISPATCH lpRow)
{
	try {
		//TES 12/17/2008 - PLID 32479 - This is a datalist now, store its value to our member variable.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pMoveUpCombo->CurSel = m_pMoveUpCombo->GetFirstRow();
		}
		m_bMoveUp = FALSE;
		if(pRow && VarLong(pRow->GetValue(0)) == 1) {
			m_bMoveUp = TRUE;
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSelChosenMoveup()");
}

void CResEntryDlg::OnSelChosenConfirmed(LPDISPATCH lpRow)
{
	try {
		//TES 12/17/2008 - PLID 32479 - This is a datalist now, store its value to our member variable.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pConfirmedCombo->CurSel = m_pConfirmedCombo->GetFirstRow();
		}
		m_nConfirmed = 0;
		if(pRow) {
			m_nConfirmed = VarLong(pRow->GetValue(0));
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSelChosenConfirmed()");
}

void CResEntryDlg::OnSelChosenReady(LPDISPATCH lpRow)
{
	try {
		//TES 12/17/2008 - PLID 32479 - This is a datalist now, store its value to our member variable.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pReadyCombo->CurSel = m_pReadyCombo->GetFirstRow();
		}
		m_bReady = FALSE;
		if(pRow && VarLong(pRow->GetValue(0)) == 1) {
			m_bReady = TRUE;
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSelChosenReady()");
}

void CResEntryDlg::OnSelChangingMoveup(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
		//  Note that the function may pop up a message box, so keep the dialog active while that's up, if needed.
		CKeepDialogActive kda(this);
		if(*lppNewSel != lpOldSel && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
			//TES 12/17/2008 - PLID 32478 - They're not allowed to use this feature, so reset the selection and close
			// up the dropdown.
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			m_pMoveUpCombo->DropDownState = VARIANT_FALSE;
		}

		//TES 12/17/2008 - PLID 32479 - Don't let them choose nothing
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSelChangingMoveup()");
}

void CResEntryDlg::OnSelChangingConfirmed(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 12/17/2008 - PLID 32479 - Don't let them choose nothing
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSelChangingConfirmed()");
}

void CResEntryDlg::OnSelChangingReady(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//TES 12/17/2008 - PLID 32479 - Don't let them choose nothing
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}CResEntryDlg_NxCatchAll("Error in CResEntryDlg::OnSelChangingReady()");
}

// (c.haag 2010-08-27 11:38) - PLID 40108 - This function will "reserve" the resentry dialog. In the current
// implementation, we only do it to prevent scheduler refreshes when a reservation is/may be opening.
void CResEntryDlg::Reserve(const CString& strReservingFunction)
{
	m_astrReservingFunctions.Add(strReservingFunction);
}

// (c.haag 2010-08-27 11:38) - PLID 40108 - This function will "unreserve" the resentry dialog. In the current
// implementation, we only do it to prevent scheduler refreshes when a reservation is/may be opening.
void CResEntryDlg::Unreserve(const CString& strUnreservingFunction)
{
	// Sanity check: We have to be reserved before this can be called
	if (!IsReserved(FALSE)) {
		ThrowNxException("%s attempted to unreserve a CResEntryDlg object that is not reserved!", 
			strUnreservingFunction);
	}

	// Usually strUnreservingFunction corresponds to the most recent reservation, but there's no rule that says 
	// that function A can reserve it, then B, then A can unreserve it, then B. At a minimum, see that it's in
	// the array of reserved function names.
	int i;
	for (i=0; i < m_astrReservingFunctions.GetSize(); i++) {
		if (m_astrReservingFunctions[i] == strUnreservingFunction) {
			break;
		}
	}
	if (i == m_astrReservingFunctions.GetSize()) 
	{
		ThrowNxException("%s attempted to unreserve a CResEntryDlg object that it did not reserve!", 
			strUnreservingFunction);
	}
	else {
		m_astrReservingFunctions.RemoveAt(i);
		if (0 == m_astrReservingFunctions.GetSize()) {
			m_bWantedUpdateViewWhileReserved = FALSE;
		}
	}
}

// (c.haag 2010-08-27 11:38) - PLID 40108 - Returns TRUE if this object is "reserved" for use. In this
// implementation, it likely means that the reservation is/may be opening.
BOOL CResEntryDlg::IsReserved(BOOL bWantsToUpdateView)
{
	if (0 == m_astrReservingFunctions.GetSize()) {
		return FALSE;
	} else {
		// If the intent was to refresh the scheduler view, remember that for later so outside functions can
		// check it, and refresh the view themselves
		if (bWantsToUpdateView) 
		{
			m_bWantedUpdateViewWhileReserved = TRUE;
		}
		return TRUE;
	}
}

// (c.haag 2010-10-11 12:05) - PLID 40108 - Returns TRUE if IsReserved was called while this object
// was reserved and the caller intended to update the view
BOOL CResEntryDlg::WantedUpdateViewWhileReserved() const
{
	return m_bWantedUpdateViewWhileReserved;
}

//TES 10/27/2010 - PLID 40868 - Determines the precision template block associated with the current appointment, if any.
long CResEntryDlg::GetTemplateItemID()
{
	//TES 10/27/2010 - PLID 40868 - Do we need to calculate it?
	if(m_nCalculatedTemplateItemID == -2) {
		CNxSchedulerDlg *pSheet = NULL;
		if (m_pSchedulerView && (pSheet = (CNxSchedulerDlg *)m_pSchedulerView->GetActiveSheet())) {
			//TES 10/27/2010 - PLID 40868 - Now create a VisibleAppointment that represents this appointment, and pass it to our
			// parent, which will tell us which TemplateItemID, if any, that appointment is/should be on.
			VisibleAppointment *pThisAppt = new VisibleAppointment;
			pThisAppt->nReservationID = m_ResID;
			pThisAppt->nAptTypeID = GetCurAptTypeID();;
			pThisAppt->anResourceIDs.Copy(m_arydwSelResourceIDs);
			pThisAppt->anPurposeIDs.Copy(m_adwPurposeID);
			pThisAppt->dtStart = m_nxtStart->GetDateTime();;
			pThisAppt->dtEnd = m_nxtEnd->GetDateTime();;
			pThisAppt->nOffsetDay = (short)m_Res.GetDay();
			m_nCalculatedTemplateItemID = pSheet->CalculateTemplateItemID(pThisAppt);
			delete pThisAppt;			
		}
	}
	return m_nCalculatedTemplateItemID;
}

//TES 10/27/2010 - PLID 40868 - Resets the calculated value for the TemplateItemID
void CResEntryDlg::RefreshTemplateItemID()
{
	//TES 10/27/2010 - PLID 40868 - Just set the value to -2, then GetTemplateItemID() will calculate it when and if it's called.
	m_nCalculatedTemplateItemID = -2;
}




void CResEntryDlg::SelChangingPatientCombo2(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} CResEntryDlg_NxCatchAll(__FUNCTION__); // (a.walling 2010-03-03 08:13) - PLID 37612 - Exception handling
}

void CResEntryDlg::SelChosenPatientCombo2(LPDISPATCH lpRow)
{
	if (!m_ExtraActive) return;

	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel(lpRow);

		if (!pCurSel) {
			CResEntryDlg_ASSERT(FALSE);
			return;
		}

		// (j.gruber 2012-07-31 14:15) - PLID 51882 - get the current patient to see if it changed
		long nCurPatID = m_nCurPatientID;

		//TES 1/14/2010 - PLID 36762 - Check whether this patient is blocked.
		// (a.walling 2010-05-27 15:34) - PLID 38917 - Use enums for columns
		long nPatientID = VarLong(pCurSel->GetValue(plID));
		{
			CKeepDialogActive kda(this);
			if(!GetMainFrame()->CanAccessPatient(nPatientID, false)) {
				SetCurPatient(m_nCurPatientID);
				return;
			}
		}

		if (!m_NewRes && nPatientID != nCurPatID && nCurPatID != -1 && m_ResID != -1) {			
			// (a.walling 2013-01-17 09:26) - PLID 54651 - Check for any appointments linked to EMNs
			CString strLinkedEMNs = GetLinkedEMNDescriptionsFromAppointment(m_ResID);
			if (!strLinkedEMNs.IsEmpty()) {
				SetCurPatient(m_nCurPatientID);
				CKeepDialogActive nevergonnagiveyouup(this);
				MessageBox(FormatString("The patient may not be changed; this appointment is linked to the following EMN data:\r\n\r\n%s", strLinkedEMNs), NULL, MB_ICONERROR);
				return;
			}
		}

		SetModifiedFlag();

		// First reflect the new selection

		// Store the new selection back into our official variables
		ReflectCurPatient();	
	
		//DRT - 6/4/03 - They've chosen a patient, have it prompt them for ins auth if necessary and react
		{
			m_bCheckActive = false;
			bool bContinue = AttemptWarnForInsAuth(GetCurPatientID(), GetDate());
			m_bCheckActive = true;

			// (a.walling 2010-06-15 07:32) - PLID 30856 - Removed PLID 17432 logging
			if(!bContinue) {
				//they said no they don't want to continue.  Cancel the dialog
				// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
				HandleCancelAndCloseReservation();
				return;
			}
		}
		//done

		if(GetRemotePropertyInt("ShowPackageInfo", 0, 0, "<None>", true)){
			CKeepDialogActive kda(this);
			ShowPackageInfo(GetCurPatientID());
		}


		if(GetRemotePropertyInt("ApptLocationPref", 1, 0, "<None>", true) == 2) {
			// Set the location to the patient's location
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Location, Name FROM PersonT INNER JOIN LocationsT ON PersonT.Location = LocationsT.ID WHERE PersonT.ID = {INT}",m_nCurPatientID);
			if(!rs->eof) {
				SetCurLocation(AdoFldLong(rs, "Location"), AdoFldString(rs, "Name",""));
			}
			else {
				// Set the location to the currently logged in location
				SetCurLocation(GetCurrentLocationID(), GetCurrentLocationName());
			}
			rs->Close();
		}

		// (a.walling 2007-03-12 17:28) - PLID 25156
		// we've changed the patient, so it follows that the extra info is dirty, otherwise
		// the more info fields will not be filled!
		m_bIsExtraInfoDirty = TRUE;

		// (j.gruber 2012-07-31 14:15) - PLID 51882 - insurance
		//clear out the current insurance if the patient changed
		if (nCurPatID != nPatientID) {
			ClearInsurancePlacements(FALSE);
		}
		
		//now just invalidate
		InvalidateRect(m_rcPriIns);
		InvalidateRect(m_rcSecIns);

		// (j.jones 2014-11-14 10:30) - PLID 64116 - now takes in a boolean to state whether
		// the user has changed the patient on the appointment, true here because we just
		// picked a new patient
		ApplyCurrentPatient(true);
		ApplyToResBox();
		AutoSelectTypePurposeByPatient();

	// TODO: Took this out because the datalist doesn't support it	
	//	int nSort = atoi(m_PatientCombo.GetSortBy().Left(1));
	//	if (nSort == 3 || nSort == 4) {
	//		m_PatientCombo.SetText(m_PatientCombo.GetValueByColumn(0).pcVal);
	//	}
		
		// Tell the main window that we have a new active patient if: the main window 
		// exists, the patient toolbar combo box exists, this patient combo has a selection
		CMainFrame *pMainFrm = GetMainFrame();
		CResEntryDlg_ASSERT(pMainFrm->GetSafeHwnd());
		if (pMainFrm->GetSafeHwnd()) {
			ASSERT(pMainFrm->m_patToolBar.GetSafeHwnd());
			if (pMainFrm->m_patToolBar.GetSafeHwnd()) {
				if (pCurSel) {
					// Finally we can set the main window's active patient
					long nNewSelection = GetCurPatientID();
					//TES 1/7/2010 - PLID 36761 - No need to check for failure, we wouldn't have been able to open the ResEntryDlg
					// if we couldn't access this patient.
					pMainFrm->m_patToolBar.TrySetActivePatientID(nNewSelection);
					// RAC - 2/16
					// TODO: It seems to me that we should just be invalidating all views rather than updating 
					// them, because for example, if the current tab in the Patients module is billing, we every 
					// time we switch patients here, we have to endure any slowness in the Billing tab's update.  
					// That just doesn't seem fair.  For now, I guess I'll leave it as is

					//JJ- this updated the scheduler view too, and if there was a patient warning, it would erase
					//the current resentry if it had not been saved. I don't see why all views need to be updated, 
					//you really only need to update the patients, which I did not seem to find a function for.
					//However, when you switch back to the patients module it calls updateview() anyways, so we
					//bypass the slowdown AND the deleted appointment!

					//pMainFrm->UpdateAllViews();

				}
			}
		}

		// (c.haag 2003-10-06 11:05) - Warn if the patient is inactive.
		WarnIfInactivePatientSelected();

		// Enable/disable the Go To Patient Button as appropriate
		EnableGoToPatient(GetCurPatientID() != -25 ? TRUE : FALSE);
	// (d.thompson 2009-07-07) - PLID 34803 - Extra logging to be removed when we figure out PLID 37221
	} CResEntryDlg_NxCatchAllCall("CResEntryDlg::OnSelChosenPatientCombo", Log("PLID 34803:  %s", CClient::PLID34803_GetPrintableCString()););
}

void CResEntryDlg::TrySetSelFinishedPatientCombo2(long nRowEnum, long nFlags)
{
	// OK, the idea here is to behave exactly as if we had just finished a call to "SetSelByColumn"
	try {
		if (nFlags == NXDATALIST2Lib::dlTrySetSelFinishedSuccess) {
			// The current selection must be something because we were given dlTrySetSelFinishedSuccess!
			if (nRowEnum) {
				// TODO: It would be nice to be able to assert that the nRowEnum row was the same row as the 
				// CurSel, but the datalist doesn't offer the ability to get the row index of an IRowSettings 
				// object yet.
				ReflectCurPatient();
			} else {
				Log("TrySetSelFinishedPatientCombo was given success, but no RowEnum.");
				// No selection, this is bad news; how could this be since we were given dlTrySetSelFinishedSuccess?
				CResEntryDlg_ASSERT(FALSE);
				ReflectCurPatientFailure();
			}
		} else {
			Log("TrySetSelFinishedPatientCombo did not return success.");
			// Try to correct the problem. For example, we may have an inactive patient selected where the
			// user only wants to see active patients.
			if (TryCorrectingPatientSelResultFailure())
			{
				ReflectCurPatient();
				return;
			}

			// If the patients combo is not filtered, there should be no way the trysetsel would have failed.
			// This could happen if the user is logged in without using NxServer and someone adds a patient and an 
			// appointment on another machine, then this user opens that appointment without requerying the datalist.
			ReflectCurPatientNotFound();
		}

		ApplyToResBox();

	} CResEntryDlg_NxCatchAll("CResEntryDlg::OnTrySetSelFinishedPatientCombo");
}

// (b.savon 2012-02-28 16:51) - PLID 48441 - Create Appt Recall
void CResEntryDlg::OnBnClickedNxbtnCreateRecallSched()
{
	try{
		//Save original config
		BOOL bIsPinned = IsDlgButtonChecked(IDC_PIN_RESENTRY);

		//Set if it isn't pinned
		if( !bIsPinned ){
			CheckDlgButton(IDC_PIN_RESENTRY, BST_CHECKED);
			// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
			HandleResEntryPinChange();
		}

		// (j.armen 2012-03-28 09:55) - PLID 48480 - This case shouldn't be possible, but anyways, just in case check for the license
		//	We will actually use it when on the create recall dlg
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) 
		{
			//(a.wilson 2012-3-23) PLID 48472 - checking whether the current user has read permission.
			BOOL bCreatePerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreate));
			BOOL bCreatePermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreateWithPass));
			BOOL bContinue = FALSE;

			if (bCreatePerm) {
				bContinue = TRUE;
			} else if (bCreatePermWithPass && CheckCurrentUserPassword()) {
				bContinue = TRUE;
			}

			if (bContinue) {

				//Fill in necessary fields to begin a patient recall
				CCreatePatientRecall::PatientRecall prRecall;
				prRecall.nApptID = m_Res.GetReservationID();
				prRecall.nPatientID = m_Res.GetPersonID() == -1 ? AsLong(m_dlAptPatient->GetCurSel()->GetValue(0)) : m_Res.GetPersonID();
				
				if (prRecall.nPatientID != -1 && prRecall.nPatientID != -25) {

					// (j.jones 2016-02-22 09:43) - PLID 68348 - pass in a provider and location
					prRecall.nLocationID = m_nCurLocationID;

					//get the first resource, arbitrarily
					if (m_arydwSelResourceIDs.GetSize() > 0) {
						long nResourceID = m_arydwSelResourceIDs.GetAt(0);
						_RecordsetPtr rs = CreateParamRecordset("SELECT ProviderID FROM ResourceProviderLinkT WHERE ResourceID = {INT}", nResourceID);
						if (!rs->eof) {
							prRecall.nProviderID = AdoFldLong(rs, "ProviderID", -1);
						}
						rs->Close();
					}

					CCreatePatientRecall prDlg(prRecall, this);
					prDlg.DoModal();
				}
				else {
					MessageBox("Unable to determine patient to create recall for.  Please select a patient and try again.", "Practice", MB_ICONINFORMATION);
				}
			}
		}

		//If it wasn't pinned, then restore value, otherwise, leave pinned (as is)
		if( !bIsPinned ){
			CheckDlgButton(IDC_PIN_RESENTRY, BST_UNCHECKED);
			// (c.haag 2015-11-12) - PLID 67577 - Don't call events; call functions that handle event business logic
			HandleResEntryPinChange();
		}

	}NxCatchAll(__FUNCTION__);
}

//(a.wilson 2012-2-29) PLID 48420 - checks to see if this appointment is already linked to a recall otherwise we get a list of unlinked recalls to assign it.
void CResEntryDlg::CheckForRecalls() 
{
	// (j.armen 2012-03-28 09:57) - PLID 48480 - If we don't have the license, no reason to continue
	if(!g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
		return;
	}
	//(a.wilson 2012-3-23) PLID 48472 - check first if the user has permission to access recalls.
	if (!(GetCurrentUserPermissions(bioRecallSystem) & (sptRead | sptReadWithPass))) {
		return;
	}

	COleDateTime dtAppointmentDateTime(GetDate() + COleDateTimeSpan(0, m_dtStartTime.GetHour(), 
		m_dtStartTime.GetMinute(), m_dtStartTime.GetSecond())); 

	if (dtAppointmentDateTime.GetStatus() == COleDateTime::valid) {
		// (j.armen 2012-07-23 11:26) - PLID 51600 - No longer need to pass in appointment time
		CRecallLinkDlg dlgRecall(m_nCurPatientID, m_nNewResID, this);

		if (dlgRecall.HasRecalls()) {
			dlgRecall.DoModal();
		}
	} else {
		MsgBox("Invalid appointment datetime value.  Unable to check for recalls.");
	}
}

// (j.luckoski 2012-04-26 10:04) - PLID 11597 - Disable or enable the menu for the resentrydlg based on cancelled or not-cancelled.
void CResEntryDlg::DisableMenuForCancelled(BOOL bIsEnable)
{
	CMenu *pMenu = GetMenu();

	GetDlgItem(IDC_GOTO_INSURANCE)->EnableWindow(MF_GRAYED);
	
	UINT uDecision;
	if(bIsEnable == TRUE) {
		uDecision = MF_ENABLED;
	} else {
		uDecision = MF_GRAYED;
	}
	

	// (a.walling 2014-04-28 13:13) - PLID 61945 - VS2013 - GetMenuItemCount returns int now
	if (pMenu && IsMenu(pMenu->m_hMenu)) {
		// Find the given submenu
		for (int i = 0; i < pMenu->GetMenuItemCount(); i++) {
			CString strLabel;
			pMenu->GetMenuString(i, strLabel, MF_BYPOSITION);
			if (strLabel.Find("Ac&tion") != -1) {
				for (int j = 0; j < pMenu->GetSubMenu(i)->GetMenuItemCount(); j++) {
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_GOTOPATIENT) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_GOTOPATIENT, uDecision);
					}
					//also disable the case history stuff
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_CASE_HISTORY) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_CASE_HISTORY, uDecision);
					}
					// (j.jones 2007-11-21 11:07) - PLID 28147 - and disable the allocation option
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_INV_ALLOCATION) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_INV_ALLOCATION, uDecision);
					}

					// (j.jones 2008-03-21 10:31) - PLID 29309 - and disable the ability to create an order
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_INV_ORDER) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_INV_ORDER, uDecision);
					}

					// (z.manning 2008-11-24 10:35) - PLID 31130 - Save and goto insurance
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == ID_GOTO_INSURANCE) {
						pMenu->GetSubMenu(i)->EnableMenuItem(ID_GOTO_INSURANCE, uDecision);
					}

					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDOK) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDOK, uDecision);
					}

					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_CREATE_BILL) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_CREATE_BILL, uDecision);
					}

					// (j.luckoski 2012-05-10 14:08) - PLID 11597 - Cancel Appt in the file menu should be grayed when new reservation
					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_DELETE_BTN && uDecision == MF_GRAYED) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_DELETE_BTN, uDecision);
					}

					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_BTN_APPOINTMENTLINKING) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_BTN_APPOINTMENTLINKING, uDecision);
					}

					if (pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_BTN_CHANGEPATIENT) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_BTN_CHANGEPATIENT, uDecision);
					}

					// (r.gonet 09/19/2013) - PLID 58416 - Disable the medication history stuff
					if(pMenu->GetSubMenu(i)->GetMenuItemID(j) == IDC_SAVE_VIEW_MEDICATION_HISTORY) {
						pMenu->GetSubMenu(i)->EnableMenuItem(IDC_SAVE_VIEW_MEDICATION_HISTORY, uDecision);
					}
				}
			}
		}
	}
}
// (j.gruber 2012-07-30 14:36) - PLID 51885
void CResEntryDlg::OnBnClickedResInsMore()
{
	try {

		CKeepDialogActive(this);

		CMenu mnu;
		mnu.CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_RES_ADD_NEW_INS_PARTY, "Add New Insured Party");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_RES_CHOOSE_ADDITIONAL, "Set Additional Insured Parties");

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_RES_INS_MORE);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}	

	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::SelChosenResInsCat(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {

			CKeepDialogActive kda(this);

			long nID = VarLong(pRow->GetValue(rccID));
		
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.CategoryPlacement "
				" FROM InsuredPartyT LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID  "
				"  LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID  "
				"  WHERE InsuredPartyT.PatientID = {INT} "
				"  AND RespTypeT.CategoryType = {INT} AND RespTypeT.ID <> -1 "
				"  ORDER BY RespTypeT.Priority "
				, m_nCurPatientID, nID);
			if (rs->eof) {
				MsgBox("No Insured Parties of that category found.");
				m_pInsCatList->CurSel = NULL;
			}
			else {
				
				//check to see if we are going to be overwriting anything
				if (m_mapInsPlacements.GetSize() > 0) {
					if (IDNO == MsgBox(MB_YESNO, "This will overwrite any previous selected insured parties on the appointment.\nAre you sure you wish to continue?")) {
						m_pInsCatList->CurSel = NULL;
						return;
					}
				}

				//clear the map
				ClearInsurancePlacements(FALSE);

				long nCatPlace = 1;
				while (!rs->eof) {
					long nPersonID = AdoFldLong(rs->Fields, "PersonID");
					CString strName = AdoFldString(rs->Fields, "Name");				
					CString strRespType = AdoFldString(rs->Fields, "TypeName");
					SetInsurance(nCatPlace, nPersonID, strName, strRespType);
					
					rs->MoveNext();
					nCatPlace++;
				}

				InvalidateRect(m_rcPriIns);
				InvalidateRect(m_rcSecIns);

				// (j.gruber 2012-08-06 11:50) - PLID 51926 - update the fields
				ApplyToExtraInfo();
				ApplyToResBox();


				//set the type back
				m_pInsCatList->CurSel = NULL;
			}
		}

	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::SelChangingResInsCat(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {		
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}	
	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::ChooseInsParty(SchedulerInsurancePlacements placement)
{
	try {
		CKeepDialogActive kda(this);
		//don't do the -25 patient
		// (j.gruber 2013-05-07 10:31) - PLID 56568 - or on a cancelled appt
		if (GetCurPatientID() == -25 || m_bIsCancelled) {
			return;
		}

		CString strPlace;
		long nPreSelect = -1;
		if (placement == priIns) {
			strPlace = "Primary";
			nPreSelect = GetInsuranceID(priIns);
		}
		else if (placement == secIns){
			strPlace = "Secondary";
			nPreSelect = GetInsuranceID(secIns);
		}
		else {
			ThrowNxException("ChooseInsParty received invalid placement");		
		}			
		
		//BOOL bHasOptions = ReturnsRecordsParam("SELECT PersonID FROM InsuredPartyT WHERE PatientID = {INT} AND PersonID NOT IN ({INT}, {INT})", m_nCurPatientID, nCurPriID, nCurSecID);
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
		BOOL bHasOptions = ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT PersonID FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID <> -1 ", m_nCurPatientID);

		if (!bHasOptions) {
			//they have no options, so see if maybe they want to make a new one or do none
			//they aren't going to have anything to pick from, so ask if they want to make a new one
			if (IDYES == MsgBox(MB_YESNO, "There are no available Insured Parties for this patient, would you like to create a new Insured Party?\nChoose 'Yes' to create a new Insured Party.\nClick 'No' to select No Insured Party")) {
				
				long nInsuredPartyID;
				CString strInsName, strRespType;
				if (AddNewInsuredParty(this,m_patientInsInfo, m_nCurPatientID, GetExistingPatientName(m_nCurPatientID), nInsuredPartyID, strInsName, strRespType)) {
					SetInsurance(placement, nInsuredPartyID, strInsName, strRespType);
				}
			}
			else {
				//set it to none
				InsuranceInfo *blank;
				if (m_mapInsPlacements.Lookup(placement, blank)) {
					m_mapInsPlacements.RemoveKey(placement);
				}				
			}
			InvalidateRect(m_rcPriIns);
			InvalidateRect(m_rcSecIns);		
	
			// (j.gruber 2012-08-06 11:50) - PLID 51926 - update the fields
			ApplyToExtraInfo();
			ApplyToResBox();
		}
		else {			

			CSelectDlg dlg(this);
			{			
				CString strTypeFilter;
				CString strSelectionFilter;
				
				//if we alrady have a primary or secondary set, don't let them pick it
				//strSelectionFilter.Format(" AND InsuredPartyT.PersonID NOT IN (%li, %li)" , nCurPriID, nCurSecID);
				dlg.m_strTitle = "Select an Insured Party to be used as " + strPlace + " for this appointment";			
				dlg.m_strFromClause.Format("(SELECT InsuredPartyT.PersonID, InsuranceCoT.Name as Company, RespTypeT.TypeName FROM InsuredPartyT "
					" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					" WHERE RespTypeID <> -1 AND InsuredPartyT.PatientID = %li %s %s) Q "
					, m_nCurPatientID, strTypeFilter, strSelectionFilter);
				dlg.AddColumn("PersonID", "ID", FALSE, FALSE);
				dlg.AddColumn("Company", "Insurance Co", TRUE, FALSE);
				dlg.AddColumn("TypeName", "Responsibility", TRUE, FALSE);
				if (nPreSelect != -1) {
					dlg.m_varPreSelectedID = nPreSelect;
					dlg.m_nPreSelectColumn = 1;
				}
			}
			if(dlg.DoModal() == IDOK) {
				long nID = VarLong(dlg.m_arSelectedValues[0]);			
				CString strName = VarString(dlg.m_arSelectedValues[1], "");
				CString strRespType = VarString(dlg.m_arSelectedValues[2], "");
				SetInsurance(placement, nID, strName, strRespType);			

				// (j.gruber 2012-08-06 11:50) - PLID 51926 - update the fields
				ApplyToExtraInfo();
				ApplyToResBox();
			}
		}
			

	}CResEntryDlg_NxCatchAll(__FUNCTION__);
	
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::DrawPrimaryInsuranceText(CDC *pdc) {
	
	CRect rc;
	GetDlgItem(IDC_RES_PRI_LABEL)->GetWindowRect(rc);
	ScreenToClient(&rc);

	pdc->FillSolidRect(rc, GetSolidBackgroundColor());	
	InsuranceInfo * pInsInfo;
	CString strName;
	if (m_mapInsPlacements.Lookup(priIns, pInsInfo)) {
		strName = pInsInfo->strInsCoName;
	}
	else {
		strName = "<None>";
	}

	// (j.gruber 2012-07-31 14:18) - PLID 51882 - check if its -25
	// (j.gruber 2013-05-07 10:25) - PLID 56568 - or cancelled
	EDrawTextOnDialogStyle dtsStyle;
	if (GetCurPatientID() == -25 || m_bIsCancelled) {
		dtsStyle = dtsDisabledHyperlink;
	}
	else {
		dtsStyle = dtsHyperlink;
	}

	DrawTextOnDialog(this, pdc, rc, strName, dtsStyle, false, DT_LEFT, true, false, 0);
	

}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::DrawSecondaryInsuranceText(CDC *pdc) {
	
	CRect rc;
	GetDlgItem(IDC_RES_SEC_LABEL)->GetWindowRect(rc);
	ScreenToClient(&rc);

	pdc->FillSolidRect(rc, GetSolidBackgroundColor());	
	
	InsuranceInfo * pInsInfo;
	CString strName;
	if (m_mapInsPlacements.Lookup(secIns, pInsInfo)) {
		strName = pInsInfo->strInsCoName;
	}
	else {
		strName = "<None>";
	}

	// (j.gruber 2012-07-31 14:18) - PLID 51882 - check if its -25
	// (j.gruber 2013-05-07 10:26) - PLID 56568 - or cancelled
	EDrawTextOnDialogStyle dtsStyle;
	if (GetCurPatientID() == -25 || m_bIsCancelled) {
		dtsStyle = dtsDisabledHyperlink;
	}
	else {
		dtsStyle = dtsHyperlink;
	}



	DrawTextOnDialog(this, pdc, rc, strName, dtsStyle, false, DT_LEFT, true, false, 0);

}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::ClearInsurancePlacements(BOOL bClearOriginals) {
	
	// (j.jones 2014-11-14 13:41) - PLID 64169 - added a global function to clear an InsuranceInfo map
	ClearAppointmentInsuranceMap(m_mapInsPlacements);

	//originals too
	if (bClearOriginals) {
		ClearAppointmentInsuranceMap(m_mapOrigIns);
	}
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
long CResEntryDlg::GetInsuranceID(SchedulerInsurancePlacements insPlace)
{
	InsuranceInfo *pInsInfo;
	if (m_mapInsPlacements.Lookup(insPlace, pInsInfo)) {
		return pInsInfo->nInsuredPartyID;
	}
	
	return -1;
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
CString CResEntryDlg::GetInsuranceName(SchedulerInsurancePlacements insPlace)
{
	InsuranceInfo *pInsInfo;
	if (m_mapInsPlacements.Lookup(insPlace, pInsInfo)) {
		return pInsInfo->strInsCoName;
	}
	
	return "<None>";
}

// (j.gruber 2012-07-30 14:36) - PLID 51830
void CResEntryDlg::SetInsurance(long nPlacement, long nInsuredPartyID, CString strInsCoName, CString strRespType, bool bErrorOnExistence /*= false*/)
{
	// (j.jones 2014-11-14 11:03) - PLID 64169 - pass our member map to the global function
	::SetAppointmentInsuranceMap(m_mapInsPlacements, nPlacement, nInsuredPartyID, strInsCoName, strRespType, bErrorOnExistence);
	
	InvalidateRect(m_rcPriIns);
	InvalidateRect(m_rcSecIns);
}

// (j.gruber 2012-08-01 12:33) - PLID 51885
void CResEntryDlg::OnNewInsuredParty()
{
	try {
		CKeepDialogActive kda(this);

		long nInsuredPartyID;
		long nPatientID = GetCurPatientID();
		CString strPatientName = GetExistingPatientName(nPatientID);
		CString strInsName, strInsCategory;

		AddNewInsuredParty(this, m_patientInsInfo, m_nCurPatientID, GetExistingPatientName(m_nCurPatientID), nInsuredPartyID, strInsName, strInsCategory);
		
		
	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}


// (j.gruber 2012-07-30 14:36) - PLID 51896
void CResEntryDlg::OnChooseAdditionalInsParties()
{
	try {

		CKeepDialogActive kda(this);

		CApptChooseMoreInsuredDlg dlg(m_nCurPatientID, m_patientInsInfo, &m_mapInsPlacements);
		
		if (IDOK == dlg.DoModal()) {
			//update the screen
			InvalidateRect(m_rcPriIns);
			InvalidateRect(m_rcSecIns);

			// (j.gruber 2012-08-06 11:50) - PLID 51926 - update the fields
			ApplyToExtraInfo();
			ApplyToResBox();
		}

	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-01-07 16:10) - PLID 54414
void CResEntryDlg::RequeryFinishedReferringPhysList(short nFlags)
{
	try {	

		//now lets set our selection
		if (m_bIsCurRefPhysSet) {
			SetCurRefPhys(m_nCurRefPhysID);

			// (j.jones 2013-07-03 15:26) - PLID 57441 - moved the apply functions from
			// inside SetCurRefPhys to outside of it
			ApplyToExtraInfo();
			ApplyToResBox();
		}

	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}



// Get the appropriate ReferringPhys ID
// (j.gruber 2013-01-07 16:10) - PLID 54414
long CResEntryDlg::GetCurRefPhysID()
{
	// Make sure our variable is set
	if (m_bIsCurRefPhysSet)
	{		
		return m_nCurRefPhysID;
	}
	else {
		// It has to be set before it can be read
		ThrowNxException("GetCurRefPhysID was called before the current Ref Phys was set!");
	}
}

// An exception will be thrown if there is no current selection in the datalist, so only 
// call this function when the selection has just been set to the correct row.
// (j.gruber 2013-01-07 16:10) - PLID 54414
void CResEntryDlg::ReflectCurRefPhys()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRefPhysList->GetCurSel();
	ASSERT(pRow != NULL);
	m_nCurRefPhysID = VarLong(pRow->GetValue(rpcID));	
	if (m_nCurRefPhysID == -1) {
		m_strCurRefPhysName = "";
	}
	else {
		m_strCurRefPhysName = VarString(pRow->GetValue(rpcName));
	}
	m_bIsExtraInfoDirty = TRUE;
	m_bIsCurRefPhysSet = TRUE;
}

// Call this function when the current location is an inactive one and therefore cannot be found in the datalist
// This will set the datalist to have no selection, and set the combo text to the name of the inactive Referring Phys
// (j.gruber 2013-01-07 16:10) - PLID 54414
void CResEntryDlg::ReflectCurRefPhysInactive()
{
	// Force the datalist to have no selection	
	m_pRefPhysList->PutCurSel(NULL);
	// We must have a current Ref Phys already (you can't call this function unless the resentry has already loaded the cur ref phys id)
	ASSERT(m_bIsCurRefPhysSet);
	// Find the ref phys name in data	
	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in ResEntry
	_RecordsetPtr rsRefPhysName = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Last + ', ' + First + ' ' + Middle as Name FROM PersonT WHERE ID = {INT}", m_nCurRefPhysID);
	// The location has to exist
	ASSERT(!rsRefPhysName->eof);
	if (!rsRefPhysName->eof) {
		// The location does exist, so get the name into our member string
		CString strName = AdoFldString(rsRefPhysName, "Name");
		m_strCurRefPhysName = strName;
		// And put the name in the combo text of the location datalist
		m_pRefPhysList->PutComboBoxText((LPCTSTR)strName);
	} else {
		// The location doesn't exist in data so this is a real problem
		ThrowNxException("ReflectCurRefPhysInactive cannot load inactive physician: id= %li", m_nCurRefPhysID);
	}

	m_bIsExtraInfoDirty = TRUE;
}

// This should only be called in utter error conditions.  It's when the code just doesn't know what the appointment's 
// ref phys is.  Calling this makes no selection in the datalist and clears the member variables that store the cur 
// ref phys
// (j.gruber 2013-01-07 16:09) - PLID 54414
void CResEntryDlg::ReflectCurRefPhysFailure()
{
	// Clear the member variables
	m_bIsCurRefPhysSet = FALSE;
	m_nCurRefPhysID = -1;	
	m_strCurRefPhysName = "";
	
	// Set the datalist to no selection	
	m_pRefPhysList->PutCurSel(NULL);

	// (j.gruber 2013-01-09 09:27) - PLID 54497
	ApplyToExtraInfo();
	ApplyToResBox();
	
}


// (j.gruber 2013-01-07 16:09) - PLID 54414
void CResEntryDlg::SetCurRefPhys(long nNewRefPhysID)
{
	// Set these immediately in case anyone asks between now and when the 
	// datalist actually gets set (which could be a while if TrySetSel 
	// returns sriNoRowYet_WillFireEvent) 
	m_nCurRefPhysID = nNewRefPhysID;	
	m_bIsCurRefPhysSet = TRUE;

	// Try to set the selection, 
	long nSelResult = m_pRefPhysList->TrySetSelByColumn_Deprecated(0, m_nCurRefPhysID);
	if (nSelResult == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
		// The selection wasn't set yet, but the TrySetSelFinished event will be fired and we'll handle things then
	} else if (nSelResult == NXDATALIST2Lib::sriNoRow) {
		// It wasn't found, which we interpret to mean the location must be inactive (because it's not in the datalist)
		ReflectCurRefPhysInactive();
	} else {
		// We have a valid row index that the datalist is now set to, so get the info out of that row
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pRefPhysList->GetCurSel();
		ASSERT(pCurSel != NULL && nSelResult == pCurSel->CalcRowNumber());
		ReflectCurRefPhys();
	}	
}

// (j.gruber 2013-01-07 16:09) - PLID 54414
void CResEntryDlg::SelChosenReferringPhysList(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);		
		if (pRow) {
			// Store the new selection back into our official variables
			ReflectCurRefPhys();
		} else {
			// This actually does happen legally, if the current ref phys is an inactive ref phys and 
			// therefore is not represented by any row in the datalist, and the user drops down and 
			// just hits the enter key without changing the selection.
			// So the best we can do is assert that the combo box test is in use, which means we have an inactive ref phys
			CResEntryDlg_ASSERT(m_pRefPhysList->GetIsComboBoxTextInUse() != VARIANT_FALSE);
		}
		SetModifiedFlag();

		// (j.jones 2013-07-03 15:26) - PLID 57441 - moved the apply functions from
		// inside ReflectCurRefPhys to outside of it
		ApplyToExtraInfo();
		ApplyToResBox();

	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-01-07 16:07) - PLID 54414
void CResEntryDlg::TrySetSelFinishedReferringPhysList(long nRowEnum, long nFlags)
{
	try {
		// OK, the idea here is to behave exactly as if we had just finished a call to "SetSelByColumn"
		if (nFlags == dlTrySetSelFinishedSuccess) {
			// The current selection must be something because we were given dlTrySetSelFinishedSuccess!
			if (nRowEnum) {
				// TODO: It would be nice to be able to assert that the nRowEnum row was the same row as the 
				// CurSel, but the datalist doesn't offer the ability to get the row index of an IRowSettings 
				// object yet.
				ReflectCurRefPhys();
			} else {
				// No selection, this is bad news; how could this be since we were given dlTrySetSelFinishedSuccess?
				CResEntryDlg_ASSERT(FALSE);
				ReflectCurRefPhysFailure();
			}
		} else {
			// It wasn't found, which we interpret to mean the ref phys must be inactive (because it's not in the datalist)
			ReflectCurRefPhysInactive();
		}
	}CResEntryDlg_NxCatchAll(__FUNCTION__);

}


void CResEntryDlg::HideRefPhysInfo()
{
	CRect rcNotesBox, rcNotesLabel, rcRefPhysBox, rcRefPhysLabel;
	GetDlgItem(IDC_NOTES_BOX)->GetWindowRect(rcNotesBox);
	ScreenToClient(rcNotesBox);
	GetDlgItem(IDC_NOTES_BOX_LABEL)->GetWindowRect(rcNotesLabel);
	ScreenToClient(rcNotesLabel);
	GetDlgItem(IDC_REFERRING_PHYS_LIST)->GetWindowRect(rcRefPhysBox);
	ScreenToClient(rcRefPhysBox);
	GetDlgItem(IDC_REFERRING_PHYS_LABEL)->GetWindowRect(rcRefPhysLabel);
	ScreenToClient(rcRefPhysLabel);

	//now move the notes label to be the top and bottom of the ref phys label
	rcNotesLabel.top = rcRefPhysLabel.top;
	rcNotesLabel.bottom = rcRefPhysLabel.bottom;

	//and the notes box top should move to the ref phys box
	rcNotesBox.top = rcRefPhysBox.top;

	//move away!
	GetDlgItem(IDC_NOTES_BOX)->MoveWindow(rcNotesBox);
	GetDlgItem(IDC_NOTES_BOX_LABEL)->MoveWindow(rcNotesLabel);

	//and hide the ref phys boxes
	GetDlgItem(IDC_REFERRING_PHYS_LABEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFERRING_PHYS_LIST)->EnableWindow(FALSE);

	GetDlgItem(IDC_REFERRING_PHYS_LABEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_REFERRING_PHYS_LIST)->ShowWindow(SW_HIDE);
}

// (j.gruber 2013-02-11 14:21) - PLID 54483
void CResEntryDlg::SelChangingReferringPhysList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {		
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}	
	}CResEntryDlg_NxCatchAll(__FUNCTION__);
}
