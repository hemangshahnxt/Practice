// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__F2B94DB1_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
#define AFX_MAINFRM_H__F2B94DB1_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <NxUILib/NxMDIFrameWnd.h>
#include "ContactBar.h"
#include "PatientToolBar.h"
#include "PalmPilotDlg.h"
#include "AptBookAlarmListDlg.h"
#include "ToDoAlarmDlg.h"
#include "AlertDlg.h"
#include "ProductItemsDlg.h"
#include "NxPracticeSharedLib/StringSortedArray.h"
#include "PhaseTracking.h"
#include "SendMessageDlg.h"
#include "OpportunityListDlg.h"
#include <tapi3.h>
#include "CallerID.h"
#include "MessagerDlg.h"
#include "AutoLogoffDlg.h"
#include "NotificationDlg.h"
#include "peplus.h"
#include "MDIClientWnd.h"
#include "SelectImageDlg.h"

#include "ActiveEMRDlg.h"
#include "EMRLockManager.h"
#include "EMNToBeBilledDlg.h"
#include "ProcedureSectionEditDlg.h"
#include "OPOSPrinterDevice.h"
#include "EmrWritableReviewDlg.h"
#include "HL7ToBeBilledDlg.h" //(j.camacho) PLID 68001

#include "NxPropManager.h"
#include "NxToolBar.h"
#include "afxpriv.h"
#include "EMRAnalysisDlg.h"
#include "FollowUpDlg.h"
//#include "EmrUtils.h"
#include <FolderMonitorUtils.h> // (z.manning 2010-05-25 10:42) - PLID 36976 - Use <> around this include now

#import "NxManagedWrapper.tlb"
#include "NxManagedWrapperEventSink.h"
#include "Modules.h" // (a.walling 2010-11-26 13:08) - PLID 40444
#include "EMNBillController.h" // (j.dinatale 2012-01-16 16:21) - PLID 47539
#include "OMRScanDlg.h"	// (j.dinatale 2012-08-13 15:50) - PLID 51941

#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <NxUILib/NxTrayIcon.h>


extern CNxPropManager g_propManager;

// (a.walling 2008-04-18 17:17) - PLID 29731 - Toolbar text height
extern long g_cnToolbarTextHeight;

#define MemberTimer(tmr)									m_n##tmr##_Event
#define DECLARE_MEMBER_TIMER(tmr)						static const long MemberTimer(tmr); long m_n##tmr##_Id;
#define IMPLEMENT_MEMBER_TIMER(class, tmr, num)		const long class::MemberTimer(tmr) = num;
#define CONSTRUCT_MEMBER_TIMER(tmr)						m_n##tmr##_Id = NULL;
#define SetMemberTimer(tmr, elapse)						(m_n##tmr##_Id = SetTimer(MemberTimer(tmr), elapse, NULL), (m_n##tmr##_Id != NULL))
#define KillMemberTimer(tmr)								{if (m_n##tmr##_Id) KillTimer(m_n##tmr##_Id);}

// (r.gonet 12/11/2012) - PLID 54117 - Forward declaration to avoid bringing in entire NxNetworkLib
namespace NxSocketUtils { typedef HANDLE HCLIENT; }

struct HL7Response;
typedef boost::shared_ptr<HL7Response> HL7ResponsePtr;
class CChildFrame;
class CNxTabView;
class CMirrorLink;
class CUnitedLink;
class CDocBar;
class CWhatsNewHTMLDlg;
class CPicContainerDlg;
class CEMRLockManager;
class COPOSMSRDevice;
class CClosingDlg;
class COPOSCashDrawerDevice;
class COPOSMSRThread;
class CLabFollowUpDlg;
class CParseTranscriptionsDlg;
class CNewCropBrowserDlg;
class CSureScriptsCommDlg;
class CLabEntryDlg;
class CVisionWebOrderDlg;
class CEStatementPatientSelectDlg;			// (j.dinatale 2011-03-28 11:44) - PLID 42982 - Make the CEStatementsPatientSelect
class CReportInfo;
class CEmrCodingGroupArray;
class COPOSBarcodeScannerThread;	// (a.wilson 2012-1-10) PLID 47517
class COPOSBarcodeScanner;			// (a.wilson 2012-1-10) PLID 47517
class CEMNBillController;			// (j.dinatale 2012-01-16 17:31) - PLID 47539
class CRecallsNeedingAttentionDlg;	// (j.armen 2012-02-27 12:25) - PLID 48303
class CContactLensOrderForm;		// (j.dinatale 2012-03-21 13:35) - PLID 49079
class CEOBDlg;
class CEligibilityRequestDlg;
class COMRScanDlg;	// (j.dinatale 2012-08-13 15:43) - PLID 51941
class CDrugInteractionDlg;			// (j.fouts 2012-9-5 3:28) - PLID 52482 - Need this dialog to be modeless
class CMUDetailedReportDlg;	// (j.dinatale 2012-10-31 12:08) - PLID 53502 - MU detailed report
class CHL7Client; // (r.gonet 12/13/2012) - PLID 53798 - Predeclare to reduce dependencies in h file.
class CHL7Client_Practice; // (z.manning 2013-05-20 12:42) - PLID 56777
class CPrescriptionQueueDlg;	// (a.wilson 2013-01-09 14:27) - PLID 54535
class CNxModelessParentDlg;		// (a.wilson 2013-01-09 14:27) - PLID 54535
class CCCHITReportsDlg; // (r.gonet 06/12/2013) - PLID 55151 - Eliminated a header include that caused needless rebuilding.
class CEMRSearch;	// (j.jones 2013-07-15 15:16) - PLID 57477 - moved header to the cpp
class CFirstAvailList; // (z.manning 2013-11-15 15:00) - PLID 58756 - Forward declare instead of including this header
class CFirstAvailableAppt; // (r.gonet 2014-11-19) - PLID 64173 - Forward declare this instead of including the header to reduce rebuilding.
class CEMRProblemListDlg;	// (j.jones 2014-02-26 11:31) - PLID 60764 - moved header to the cpp
class CEligibilityRequestDlg;	// (j.jones 2014-02-28 10:27) - PLID 60767 - moved header to the cpp
class CHL7ExportDlg; // (r.gonet 03/18/2014) - PLID 41070 - moved header to the cpp necessitating a forward declaration
class CRoomManagerDlg;
class CLockBoxPaymentImportDlg; // (d.singleton 2014-07-11 10:02) - PLID 62862 - create new dialog that will import a lockbox payment file
class CEligibilityReviewDlg; // (r.goldschmidt 2014-10-08 16:18) - PLID 62644 - make eligibility review dialog modeless
class CEligibilityRequestDetailDlg; // (r.goldschmidt 2014-10-10 16:16) - PLID 62644 - make eligibility request detail dialog modeless
class CHL7ToBeBilledDlg; // (J.camacho) plid 68001
class CICCPDeviceManager; // (z.manning 2015-08-04 14:53) - PLID 67221


enum NewCropActionType;
enum FieldOperatorEnum;
enum LabType;
enum ESchedulerTemplateEditorType;

#define IDT_TODO_TIMER			1003
#define IDT_AUTOLOGOFF_APPEAR_TIMER		1005
#define IDT_AUTOLOGOFF_COUNTDOWN_TIMER	1006
#define IDT_WAIT_FOR_SHEET_TIMER		1007
#define IDT_CONFIGRT_CACHE_FLUSH_TIMER	1008
#define IDT_UPDATE_SCHEDULER_VIEW		1010
#define IDT_NXSERVER_RECONNECT			1011
// (a.wetta 2007-07-05 09:59) - PLID 26547 - Timer for OPOS MSR device initialization
#define IDT_OPOSMSRDEVICE_INIT_TIMER	1012
// (a.walling 2009-07-10 10:08) - PLID 33644
#define IDT_QUEUEDMESSAGE_TIMER			1013
// (a.walling 2010-01-27 13:45) - PLID 22496 - Check if the date has changed
#define IDT_CHECKDATE_TIMER				1014
// (j.jones 2010-06-01 11:24) - PLID 37976 - timer for ophthalmology device folder changes
//#define IDT_DEVICE_FOLDER_CHANGED_TIMER		1015
// (a.wilson 2012-1-12) PLID 47517 - timer to ensure the barcode scanner didnt time out.
#define IDT_OPOS_BARSCAN_INIT_TIMER		1016
//TES 3/2/2015 - PLID 64736 - We can't immediately tell if NxDebug attaches successfully, so we set this timer to check the result
#define IDT_CHECK_DEBUG_MODE_TIMER		1017
// (s.tullis 2016-01-28 17:46) - PLID 68090
#define IDT_REMINDER_PRESCRIPTION_NEEDING_ATTNETION  1018
// (s.tullis 2016-01-28 17:46) - PLID 67965
#define IDT_REMINDER_RENEWALS						 1019

#define TOOLBARHEIGHT 40 //Height of all tool bars
#define CLIP_METHOD_COPY 1
#define CLIP_METHOD_CUT 2
#define CLIP_RESERVATION "ReservationClip"
#define MAXMODULES 4

//(e.lally 2012-04-12) PLID 49566 - Generic defines for our combo box sentinels
	//e.g. <All Categories>, <No Category>, <Multiple Categories>
	//These values should never be stored in data! They are just to be used as ID placeholders in dropdowns.
#define ID_COMBO_FILTER_ALL			-1
#define ID_COMBO_FILTER_NO_KEY		-2
#define ID_COMBO_FILTER_MULTIPLE	-3

//Types of notifications, bitwise.
#define NT_DEFAULT	0x000001
#define NT_TODO		0x000002
#define NT_YAK		0x000004
#define NT_GOTOPATIENT	0x000008
#define NT_HL7		0x000010	// (j.jones 2008-04-22 09:48) - PLID 29597 - added HL7 notifications
//TES 4/9/2009 - PLID 33889 - added SureScripts notifications
// (j.jones 2016-02-03 16:49) - PLID 68118 - turned this into just Renewals Needing Attention, split out Rx Needing Attention below
#define NT_SS_RENEWALS_NEEDING_ATTENTION	0x000020
#define NT_NEW_PATIENT	0x000040 // (j.gruber 2010-01-07 14:33) - PLID 36648 - added new patient
#define NT_HL7_LAB	0x000080	// (a.vengrofski 2010-08-18 08:56) - PLID <38919> - Added for labs
// (j.jones 2011-03-15 12:24) - PLID 42738 - supported the device importer
#define NT_DEVICE_IMPORT	0x000100
//TES 11/1/2013 - PLID 59276 - Support for Clinical Decision Support interventions
#define NT_CDS_INTERVENTION	0x000200
// (j.jones 2016-02-03 16:49) - PLID 68118 - split out into Rx Needing Attention
#define NT_RX_NEEDING_ATTENTION	0x000400

//How we might end up waiting to pop up a notification.
#define WT_DONT_WAIT				-1 //We don't have to wait.
#define WT_WAIT_FOR_SHEET_ACTIVE	0 //Constantly monitoring sheet to let us popup.
#define WT_WAIT_FOR_SHEET_PASSIVE	1 //Waiting for the sheet to tell us to popup
#define WT_WAIT_FOR_SELF			2 //Waiting for our own IsPopupAllowed to be true

#define WT_WAIT_ALWAYS				3 //Never popup unless they click on the notification

// (j.fouts 2012-06-06 10:58) - PLID 49611 - Send this message when a prference has been changed
#define WM_PREFERENCE_UPDATED (WM_USER + 0x0402)

// (r.gonet 2016-05-24 15:54) - NX-100732 - Message indicating that the system name used in system level
// ConfigRT preferences has been updated (may or may not have changed though).
#define WM_SYSTEM_NAME_UPDATED (WM_USER + 0x403)

//no includes, so we don't have to rebuild all as often

class CClip : public CObject
{
	DECLARE_DYNAMIC( CClip );
public:
	void Clear(){};
	CClip &operator=(CClip &refClip);
};


// (a.walling 2013-04-11 17:05) - PLID 56225 - Move DeviceImport stuff into its own class


enum VisionWebOrderType;

// (a.walling 2012-07-12 08:56) - PLID 51501 - Now derived from CNxMDIFrameWnd
class CMainFrame : public CNxMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

private:
	scoped_ptr<CMUDetailedReportDlg> m_pMUDetailedReport;	// (j.dinatale 2012-10-31 12:08) - PLID 53502 - MU detailed report
	// (j.politis 2015-04-20 13:19) - PLID 65484
	bool ImportShouldBeAllowed();

	/// <summary>
	/// Alters the state of the program based on a received extended table checker message
	/// </summary>
	/// <param name="wParam">The integer parameter of the message</param>
	/// <param name="lParam">The long parameter of the message</param>
	void ProcessExtendedTableCheckerMessage(WPARAM wParam, LPARAM lParam);

	/// <summary>
	/// Alters the state of the program based on a received table checker message
	/// </summary>
	/// <param name="wParam">The integer parameter of the message</param>
	/// <param name="lParam">The long parameter of the message</param>
	void ProcessTableCheckerMessage(WPARAM wParam, LPARAM lParam);

// Attributes
public:
	COpportunityListDlg *m_pdlgOpportunityList;		//DRT 5/24/2007 - PLID 25892
	CToDoAlarmDlg m_dlgToDoAlarm;
	
	CAlertDlg m_dlgAlert;
	class CBarcodeSetupDlg* m_pBarcodeSetupDlg;	//(a.wilson 2012-1-13) PLID 47517 - changed to a pointer.
	CMessagerDlg* m_pdlgMessager;			// (a.walling 2007-05-04 10:16) - PLID 4850 - I've changed the messager and sendyakmessage
	CSendMessageDlg* m_pdlgSendYakMessage;  // dialogs to be pointers, so they can easily be destroyed and reinitialized. Their CWnd
	CActiveEMRDlg m_pActiveEMRDlg;			// objects are created in the mainframe constructor, and when switching users in InitPracYakker
	CEMRLockManager m_pEMRLockManagerDlg;   // so they are pretty much guaranteed to be valid pointers. I did not clutter up the source by
											// commenting every change to use a pointer.
	CEMRSearch &m_pEMRSearchDlg;				// (j.jones 2013-07-15 15:15) - PLID 57477 - changed to a reference
	CEMNToBeBilledDlg m_EMNToBeBilledDlg;		// (a.walling 2010-04-26 13:18) - PLID 38364
	CHL7ToBeBilledDlg m_HL7ToBeBilledDlg;
	CEmrWritableReviewDlg m_EMRWritableReviewDlg;
	CEStatementPatientSelectDlg *m_pEStatementPatientSelectDlg;	// (j.dinatale 2011-03-28 13:00) - PLID 42982 - Need this for EStatements Patient Select to be modeless
	CCCHITReportsDlg *m_pCCHITReportsDlg; // (j.gruber 2011-11-01 09:57) - PLID 46219 - make dialog modeless
	CEligibilityReviewDlg *m_pEligibilityReviewDlg; // (r.goldschmidt 2014-10-08 16:18) - PLID 62644 - make eligibility review modeless
	CEligibilityRequestDetailDlg *m_pEligibilityRequestDetailDlg; // (r.goldschmidt 2014-10-10 16:16) - PLID 62644 - make eligibility request detail modeless
	CHL7ToBeBilledDlg *m_pHL7ToBeBilledDlg;//(j.camacho 2016-01-27) PLID 68001




	// (j.dinatale 2012-01-16 16:19) - PLID 47539 - EMN bill Controller
	CEMNBillController m_EMNBillController;

	// (j.dinatale 2012-08-13 15:43) - PLID 51941
	COMRScanDlg m_OMRProcessingDlg;

	// (r.gonet 12/11/2012) - PLID 54117 - Gets the Async CHL7Client.
	inline CHL7Client_Practice *GetHL7Client()
	{
		return m_pHL7Client;
	}

	// (d.thompson 2009-10-21) - PLID 36015 - This has been removed.  Please just create your own
	//	local CProductItemsDlg if you need to use it.
	//CProductItemsDlg& GetProductItemsDlg();
	// (a.walling 2008-07-16 17:18) - PLID 30751 - Access transcription parsing
	CParseTranscriptionsDlg* m_pParseTranscriptionsDlg;
	// (j.jones 2009-03-02 14:34) - PLID 33053 - added NewCrop browser
	CNewCropBrowserDlg* m_pNewCropBrowserDlg;
	class CNxView* m_pLastActivatedView;
	// (r.gonet 03/18/2014) - PLID 60782 - Changed to reference to a reference to avoid having to include its header file in this file to avoid rebuilds.
	CHL7ExportDlg &m_dlgHL7Export;
	CAutoLogoffDlg m_dlgAutoLogoff;
	//TES 11/30/2007 - PLID 28192 - The POSPrinterDevice is now protected.  If you want it, you need to call 
	// ClaimPOSPrinter(), which will attempt to claim it, and return a pointer to it if it succeeds.  If it does not,
	// it will report the reason for failure, and return NULL.  If this does not return NULL, you MUST call 
	// ReleasePOSPrinter() once you're done with it.  Otherwise, third-party applications that want to use the receipt 
	// printer will be unable to until Nextech closes.
	// (a.walling 2011-03-21 17:32) - PLID 42931 - Use POSPrinterAccess
	//COPOSPrinterDevice* ClaimPOSPrinter();
	//void ReleasePOSPrinter();
	//TES 11/30/2007 - PLID 28192 - If you just want to check whether there is a valid receipt printer set up, but don't
	// need to actually use that printer, then this is the function for you!
	bool CheckPOSPrinter();
	COPOSPrinterDevice* GetOPOSPrinterDevice()
	{
		return m_pPOSPrinterDevice;
	};

	// (a.walling 2011-06-22 11:59) - PLID 44260 - Acquire and hold a reference to an adobe acrobat object to prevent premature DLL unloading
	void HoldAdobeAcrobatReference();

	// (d.thompson 2014-02-04) - PLID 60638 - There are 2 types of diagnosis search configuration allowed.
	//	"Follow the preference".  This includes crosswalk, icd9 only, icd10 only, etc.  There is a system preference
	//		and every list follows that options.
	class CDiagSearchConfig* GetDiagPreferenceSearchConfig();
	//	"Dual Search".  Some areas do not follow the preference (see insurance referrals).  Instead, the provide a 
	//		selection mechanism that just blindly searches all ICD9 and all ICD10 codes.
	class CDiagSearchConfig* GetDiagDualSearchConfig();

protected:
	// (d.thompson 2014-02-04) - PLID 60638
	class CDiagSearchConfig* m_pDiagPreferenceSearchConfig;
	class CDiagSearchConfig* m_pDiagDualSearchConfig;

	// (a.walling 2011-06-22 11:59) - PLID 44260
	IUnknownPtr m_pHeldAdobeAcrobatReference;

	// (a.wilson 2013-01-09 15:40) - PLID 54535 - pointers to the queue and its parent.
	scoped_ptr<class CNxModelessParentDlg> m_pPrescriptionQueueParentDlg;

	// (a.walling 2015-01-12 10:28) - PLID 64558 - Rescheduling Queue - Dock and embed
	scoped_ptr<class CNxModelessParentDlg> m_pReschedulingQueueParentDlg;

	// (j.gruber 2007-05-09 12:56) - PLID 9802 - receipt printer
	COPOSPrinterDevice *m_pPOSPrinterDevice;
	//TES 11/30/2007 - PLID 28192 - Do reference counting, so we don't claim the receipt printer except when we need it.
	// (a.walling 2011-03-21 17:32) - PLID 42931 - Use POSPrinterAccess
	//int m_nPOSPrinterDeviceClaims;
	// (r.gonet 12/11/2012) - PLID 54117 - Async CHL7Client to manage HL7 transmissions from us to NxServer.
	// (z.manning 2013-05-20 11:17) - PLID 56777 - Renamed to CHL7Client_Practice
	CHL7Client_Practice *m_pHL7Client;
public:
	// (a.walling 2007-05-15 10:38) - PLID 9801 - cash drawer
	COPOSCashDrawerDevice* m_pPOSCashDrawerDevice;

	// (a.walling 2007-07-24 12:48) - PLID 26787 - Cache common properties when logging in
	void CacheLoginProperties();

	// (a.walling 2007-07-25 11:46) - PLID 26261 - Ensure all EMR temp files have been deleted
	void CleanupEMRTempFiles(BOOL bPend);

	static void PreloadNexPhotoLibs(); // (a.walling 2011-12-14 10:41) - PLID 40593 - Preloads NexPhoto and dependencies in a separate process
// Operations
public:
	void ShowOpportunityList();						//DRT 5/24/2007 - PLID 25892
	//TES 10/6/2005 - See if we can close right now.
	bool CheckAllowExit(BOOL bSwitchUser = FALSE); // (a.walling 2007-05-04 10:18) - PLID 4850 - Special handling when switching users
	//TES 10/6/2005 - We're closing, do whatever we need to do.
	void PreExit();
	BOOL Idle(int time);//Idle time processing
	void NxClientReport();	//internal only	
	void DeleteCurrentPatient(bool bSilent = false);
	void OnActivityTimeoutSet();
	void TryResetAutoLogOffTimer(UINT message);

	// (a.walling 2007-05-04 10:18) - PLID 4850
	void HandleUserLogin(BOOL bInitialLogin = TRUE);	// Handle all tasks associated with a user logging in
	void HandleUserLogout(BOOL bSwitchUser, CClosingDlg* pClosingDlg); // Handle all tasks when a user logs out
	BOOL TryCloseAll(CClosingDlg* pClosingDlg = NULL, BOOL bSwitchUser = FALSE); // Try closing all views and hiding toolbars, calls CheckAllowExit initially.
	// (d.thompson 2014-03-07) - PLID 60638
	void ResetDiagSearchSettings();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle); // (a.walling 2010-01-27 16:46) - PLID 37089 - Manually handle updating the frame title. We want to use the title of the active child.
	//}}AFX_VIRTUAL

// Implementation
public:
	void OnGoBack();
	void OpenAppointment(long nResID);
	void UpdateToolBarButtons(bool requery = false);
	BOOL HandleToolbarButtonMouseMessage(int nID, BOOL bDoAction); // (a.walling 2008-07-02 13:57) - PLID 29757 - Handle a click or mousemove on the toolbar
		// returns TRUE if button is not licensed
	long CreateToolBars();
	void RestoreDocking();
	CNxTabView * GetActiveView();
	int HotKey (int key);
	void DisableHotKeys (void);
	void EnableHotKeys (void);
	void ShowTodoList();
	void ShowActiveEMR();
	void ShowLockManager(); //manages unlocked EMNs (generally over 30 days old)
	// (a.walling 2006-09-22 13:34) - PLID 19598 - show the EMR Search/review
	void ShowEMRSearch();
	// (a.walling 2006-09-22 13:34) - PLID 18215 - show the patients seen today dialog!
	void ShowSeenToday();
	// (a.walling 2008-06-26 16:02) - PLID 30531 - show the EMR Writable Review dialog
	void ShowEMRWritableReview();

	// (j.dinatale 2011-03-28 13:16) - PLID 42982 - Show the EStatements Patient Select dialog
	// (a.walling 2013-08-09 09:59) - PLID 57948 - Always reload the EStatement patient selection info
	// (c.haag 2016-05-19 14:08) - PLID-68687 - In the past the report SQL was generated, then run, and
	// all of the patient ID's in the result set were passed into this function. After that, the report
	// SQL was re-generated (identically to the original one) with a filter on the selected patients. Now
	// we just take in the original report SQL and do all the work in this function where it can be done
	// more efficiently.
	void ShowEStatementsPatientSelect(CReportInfo *pReport, BOOL bSummary, const class CComplexReportQuery& reportQuery);

	// (j.gruber 2011-11-01 10:13) - PLID 46219
	void ShowCCHITReportsDlg();

	void ShowMUDetailedReport();	// (j.dinatale 2012-10-31 12:08) - PLID 53502 - MU detailed report

	void ShowEligibilityReviewDlg();	// (r.goldschmidt 2014-10-08 16:18) - PLID 62644 - make eligibility review dialog modeless

	// (r.goldschmidt 2014-10-10 16:16) - PLID 62644 // (r.goldschmidt 2015-11-12 12:53) - PLID 65363
	//this always takes in a list of request IDs to show, and optionally a list of response IDs,
	//the response IDs are only filled if we want to filter on specific responses
	void ShowEligibilityRequestDetailDlg(long nRequestID);
	void ShowEligibilityRequestDetailDlg(std::vector<long> &aryRequestIDsUpdated, bool bSilentlyCloseExistingDialog = false);
	void ShowEligibilityRequestDetailDlg(std::vector<long> &aryRequestIDsUpdated, std::vector<long> &aryResponseIDsReturned, bool bSilentlyCloseExistingDialog = false);
	void ShowHL7ToBeBilledDlg();	//(j.camacho 2016-01-27) PLID 68001																								//(j.camacho 2016-01-27) PLID 68001

	BOOL SelectTodoListUser(long nUserID);
	void ShowAlertDialog();
	//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace
	//TES 6/9/2008 - PLID 23243 - Added more parameters to determine whether this should actually be displayed.
	void AddAlert(const char* szAlert, const CDWordArray &dwaResourceIDs, BOOL bCheckData, long nAppointmentID, EAlert alert, DWORD ip);
	
	// (j.jones 2014-08-26 11:48) - PLID 63222 - called by tablecheckers, this resets the todo timer for the given user
	void TryResetTodoTimer(long nTaskID);
	void SetToDoTimer(long nTimerLength);

	CString GetPatientSQL();
	bool m_bUpdateScheduler;
	int m_Include, m_Search;
	// (r.gonet 2014-11-19) - PLID 64173 - Convert to reference because the header is now in the cpp file.
	CFirstAvailableAppt &m_FirstAvailAppt; // modal
	CAptBookAlarmListDlg m_dlgAptBookAlarmList;
	CPalmPilotDlg m_PalmPilotDlg;
	//CPPCCfgDlg m_PocketPCDlg; // (c.haag 2005-10-20 12:36) - PLID 18013 - This is now obselete.
	void PrintHistoryReport(BOOL bPreview, CPrintInfo *pInfo);
	void LoadTitleBarText(CString strOverride = "");
	void RefreshPatientName();
	bool FlipToModule(const CString &strModuleName);
	bool EnablePatientsToolBar(bool bEnable, void *pLockedBy = 0);
	CChildFrame * GetOpenViewFrame(LPCTSTR strModule);
	CChildFrame * GetActiveViewFrame();
	CNxView * GetOpenView(LPCTSTR strModuleName);
	// (j.jones 2014-08-07 16:09) - PLID 63232 - returns true if the requested module name is the currently viewed module
	bool IsActiveView(LPCTSTR strModuleName);
	void RequeryPatientsBar();
	void ClearClip();
	void GetClip(const CString &strClipType, CClip *&pClipData, int &nClipMethod);
	void SetClip(const CString &strClipType, CClip *pClipData, int nClipMethod = CLIP_METHOD_COPY);
	void RemoveView(CNxView *oldView);
	void AddView(CNxView *newView);
	void UpdateAllViews();
	void GenerateFilters(int id = -25);
	void OnContactChange();
	void OnBarcodeEvent(UINT message, WPARAM wParam, LPARAM lParam); // CH 4/5/01
	// (a.wetta 2007-03-15 17:20) - PLID 25234 - Function to handle OPOS MSR device messages
	void OnOPOSMSRDeviceEvent(UINT message, WPARAM wParam, LPARAM lParam);
	//(a.wilson 2012-1-10) PLID 47517 - function to handle device messages from thread.
	void OnOPOSBarcodeScannerEvent(UINT message, WPARAM wParam, LPARAM lParam);
	// (j.gruber 2007-07-17 13:05) - PLID 26710 - handle pin pad messages
	void OnPinPadDeviceEvent(UINT message, WPARAM wParam, LPARAM lParam);
	CString GetPatientFilter(bool bIncludeWhere = false);
	void WritePrescription();
	// (c.haag 2010-02-18 17:23) - PLID 37384 - Lets the user apply a new prescription to the current medications list.
	// Returns TRUE if any data changed.
	// (j.jones 2010-08-23 09:17) - PLID 40178 - takes in a NewCropGUID
	// (j.jones 2011-05-02 15:24) - PLID 43350 - this now takes in a Sig for the current med
	BOOL ReconcileCurrentMedicationsWithNewPrescription(long nPatientID, long nNewRxID, CString strNewCropGUID, OLE_COLOR clrBack, CString strSig);
	// (c.haag 2010-02-18 10:03) - PLID 37424 - Lets the user apply new prescriptions imported from our NewCrop
	// integration to the current medications list. Returns TRUE if any data changed.
	// (j.jones 2013-01-08 10:19) - PLID 47302 - added parent wnd
	BOOL ReconcileCurrentMedicationsWithNewPrescriptions(long nPatientID, const CStringArray& astrNewCropRxGUIDs, OLE_COLOR clrBack, CWnd *pParentWnd = NULL);
	// (j.fouts 2013-02-06 09:41) - PLID 54472 - Open the needing attention with the option to show the messages that the user was notified for
	// (j.jones 2016-02-03 16:23) - PLID 68118 - repurposed the parameter to state when we want to see renewals needing attention,
	// otherwise we want prescriptions needing attention
	void OpenPrescriptionsRenewalsNeedingAttention(bool bShowRenewalsNeedingAttention);

	// (j.jones 2013-01-09 13:26) - PLID 54530 - moved medication reconciliation functions to ReconcileMedicationsUtils

	//Opens a PIC with the specified ID, defaulting to the non-clinical tab.
	void OpenPIC(long nPicID);

	// Tries to find an existing modeless PIC window, and then makes it the active window
	// (a.walling 2013-01-17 12:08) - PLID 54666 - Returns CPicContainerDlg*
	CPicContainerDlg* TryOpenExistingPic(long nPicID);

	CList<CPicContainerDlg *, CPicContainerDlg *>& GetPicContainers();

	// (z.manning 2011-06-15 09:37) - PLID 44120
	CFirstAvailList* GetFirstAvailList();

	// (b.savon 2013-01-10 16:45) - PLID 54567 - RxNeedingAttention on Medications Tab
	void ShowPrescriptionsNeedingAttention();

	// (z.manning 2015-03-13 15:35) - NX-100432 - Function to get the tab that should be used when clicking the EMR toolbar icon
	PatientsModule::Tab GetPrimaryEmrTab();

protected:
	// (c.haag 2010-07-15 17:23) - PLID 34338 - This function will allocate and return a new lab entry dialog
	CLabEntryDlg* AllocateLab(CWnd* pParentWnd);
	// (c.haag 2010-11-15 12:57) - PLID 41124 - This function will allocate a new VisionWeb order dialog
	CVisionWebOrderDlg* AllocateVisionWebOrderDlg(CWnd* pParentWnd);
	
	// (z.manning 2011-06-15 09:37) - PLID 44120 - This is no longer public and is now a pointer
	CFirstAvailList* m_pdlgFirstAvailList; // modeless, persistent

	// (j.dinatale 2012-03-21 13:39) - PLID 49079 - allocate a new contact lens order
	CContactLensOrderForm* AllocateContactLensOrderDlg(CWnd* pParentWnd, long nOrderID);

public:
	// (c.haag 2010-07-15 17:23) - PLID 34338 - This function will open this lab on the screen. You should use this instead of DoModal().
	// (j.jones 2010-09-01 09:40) - PLID 40094 - added a location ID to be used on new labs that are created
	// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
	int OpenLab(CWnd* pParentWnd, long nPatientID, long nProcedureID, LabType ltType, long nInitialLabID, long nOrderSetID, const CString& strToBeOrdered, long nPicID, BOOL bNextLabWillOpenOnClose, BOOL bModal, HWND hwndNotify, long nNewLabLocationID = GetCurrentLocationID(), long nResultID = -1);
	// (c.haag 2010-07-15 17:23) - PLID 34338 - This will bring a lab to the front of the screen
	// (c.haag 2010-11-15 12:57) - PLID 41124 - This is now universal
	void BringDialogToFront(CDialog* pDlg);

public:
	// (c.haag 2010-11-15 12:57) - PLID 41124 - This function will open a VisionWeb order dialog. You should use this instead of DoModal().
	//TES 5/24/2011 - PLID 43737 - Added a parameter for the type of order
	int OpenVisionWebOrderDlg(CWnd* pParentWnd, long nOrderID, VisionWebOrderType vwot = (VisionWebOrderType)1/*vwotSpectacleLens*/);
	// (c.haag 2010-11-15 12:57) - PLID 41124 - This will bring a VisionWeb order to the front of the screen
	void BringVisionWebOrderWindowToFront(CVisionWebOrderDlg* pDlg);

	// (j.dinatale 2012-03-21 13:19) - PLID 49079 - Open the contact lens order form!
	int OpenContactLensOrderForm(CWnd* pParentWnd, long nOrderID);
protected:
	// (s.dhole 07/23/2012) PLID 48693
	CEligibilityRequestDlg *m_pEligibilityRequestDlg;

public:
	// (s.dhole 07/23/2012) PLID 48693
	void ShowEligibilityRequestDlg(CWnd* pParentWnd, long nID = -1,long nFormatID = -1,long nDefaultInsuredPartyID = -1,OLE_COLOR clrBackground =0) ;
	//void IsEligibilityRequestDlgOpen(CWnd* pWnd);

	
public:
	// (a.walling 2007-08-24 17:38) - PLID 26195 - Get count of open unsaved EMRs.
	long GetUnsavedEMRCount();

	// (a.walling 2013-01-17 12:08) - PLID 54666 - Returns CPicContainerDlg*
	CPicContainerDlg* StartNewEMRRecord(long nPatientID, long nPicID, long nEmnTemplateID);
	//Edits an existing EMR, can optionally have a default EMN open, or add a new EMN based on a template.
	// (z.manning 2011-10-28 17:46) - PLID 44594 - This now returns a pointer to the PIC dialog.
	// Also added a param to open invisibly.
	CPicContainerDlg* EditEmrRecord(long nPicID, long nEmnID = -1, long nNewEmnTemplateID = -1);

	void EditEmnTemplate(long nTemplateID);
	// (a.walling 2012-03-02 11:42) - PLID 48469
	void CreateNewEmnTemplate(long nCollectionID);

	void ShowRoomManager(long nShowAppointmentID = -1, BOOL bShowMinimized = FALSE);

	// (a.walling 2014-12-22 12:09) - PLID 64369 - Show rescheduling queue
	void ShowReschedulingQueue(long nApptID = -1);

	// (j.jones 2008-07-17 08:51) - PLID 30730 - added a modeless EMR Problem List	
	void ShowEMRProblemList(long nPatientID, EMRProblemRegardingTypes eprtDefaultRegardingType = eprtInvalid, long nDefaultRegardingID = -1, CString strRegardingDesc = "");

	// (j.jones 2008-10-22 08:40) - PLID 14411 - added EMR Analysis as a modeless dialog
	void ShowEMRAnalysisDlg();

	// (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
	// (j.jones 2012-10-12 10:06) - PLID 53149 - supported a file to auto-open (for OHIP)
	void ShowEOBDlg(CString strAutoOpenFilePath = "");

	// (d.moore 2007-09-26) - PLID 23861 - The NexForms Editor is now modeless. So it makes more
	//  sense to manage it from here rather than ProcedureDlg.
	void ShowNexFormEditor(long nProcedureID = -1);

	NxSocketUtils::HCLIENT GetNxServerSocket();

	// (r.gonet 12/11/2012) - PLID 54117 - CMainFrame handles all responses
	// for async usages of CHL7Client. It reports failures to the user.
	void HandleHL7Response(CHL7Client *pClient, HL7ResponsePtr pResponse);

	// (a.wetta 2007-07-03 17:48) - PLID 26547 - Determines if the OPOS MSR device exists
	BOOL DoesOPOSMSRDeviceExist();
	// (a.walling 2007-05-15 14:40) - PLID 9801 - Return the cash drawer device window (or NULL)
	COPOSCashDrawerDevice* GetCashDrawerDevice();

	// (j.jones 2009-08-13 14:14) - PLID 35213 - added a mutex for NewCrop patient account access,
	// this applies to the NewCrop browser actions only (such as ncatAccessPatientAccount and
	// ncatProcessRenewalRequest), not any follow-up SOAP calls we may be attempting
	BOOL GetIsBrowsingNewCropPatientAccount();
	void SetIsBrowsingNewCropPatientAccount(BOOL bNewValue);

	//(a.wilson 2012-1-11) PLID 47517 - function to check if the device is alive.
	BOOL DoesOPOSBarcodeScannerExist();

	// (d.singleton 2014-07-11 10:08) - PLID 62862 - show the modeless lockbox payment import dlg
	void ShowLockboxPaymentImportDlg(long nBatchID = -1);
	
protected:
	// (a.wetta 2007-07-03 13:05) - PLID 26547 - The thread which contains the OPOS MSR device
	COPOSMSRThread *m_pOPOSMSRThread;

	// (a.wilson 2012-1-10) PLID 47517 - thread pointer and bool to tell whether initialized or not.
	COPOSBarcodeScannerThread *m_pOPOSBarcodeScannerThread;
	bool m_bOPOSBarcodeScannerInitComplete;

	// (a.wetta 2007-07-05 09:56) - PLID 26547 - Keep track of if the MSR device is ready to be used
	BOOL m_bOPOSMSRDeviceInitComplete;

	// (j.jones 2009-08-13 14:14) - PLID 35213 - added a mutex for NewCrop patient account access,
	// this applies to the NewCrop browser actions only (such as ncatAccessPatientAccount and
	// ncatProcessRenewalRequest), not any follow-up SOAP calls we may be attempting
	BOOL m_bIsBrowsingNewCropPatientAccount;

public:
	// (j.jones 2010-03-31 17:24) - PLID 37980 - This function will find an open EMR for a given patient,
	// provided that the EMR has an EMN selected that is writeable, and has an open topic.
	// It will return the PicContainer object if found.
	// Will prompt if multiple qualifying EMRs are open for the same patient, using strMultiEMNPromptText
	CPicContainerDlg* GetOpenPatientEMR_WithWriteableTopic(long nPatientID, CString strMultiEMNPromptText);

protected:
	// (b.cardillo 2006-06-14 09:08) - PLID 14292 - We now store multiple PIC container pointers
	CList<CPicContainerDlg *, CPicContainerDlg *> m_lstpPicContainerDlgs;
	// (c.haag 2010-07-15 17:23) - PLID 34338 - We now store multiple Lab entry dialog pointers
	CList<CLabEntryDlg *, CLabEntryDlg *> m_lstpLabEntryDlgs;
	// (c.haag 2010-11-15 12:57) - PLID 41124 - We now store multiple VisionWeb order dialog pointers
	CList<CVisionWebOrderDlg *, CVisionWebOrderDlg *> m_lstpVisionWebOrderDlgs;

	// (j.dinatale 2012-03-21 13:11) - PLID 49079 - keep track of our contact lens orders
	CList<CContactLensOrderForm *, CContactLensOrderForm *> m_lstpContactLensOrderForms;

	CRoomManagerDlg *m_pRoomManagerDlg;

	// (j.jones 2008-07-17 08:51) - PLID 30730 - added a modeless EMR Problem List
	CEMRProblemListDlg *m_pEMRProblemListDlg;

	// (j.jones 2008-07-17 16:39) - PLID 30730 - cleanly destroys modeless EMR Problem List
	void DestroyEMRProblemList();

	// (j.jones 2008-10-21 17:41) - PLID 14411 - added EMR Analysis as a modeless dialog
	CEMRAnalysisDlg *m_pEMRAnalysisDlg;

	// (j.jones 2008-10-21 17:58) - PLID 14411 - cleanly destroys the EMR Analysis screen
	void DestroyEMRAnalysisDlg();

	// (j.jones 2012-05-25 10:55) - PLID 44367 - added E-Remittance as modeless
	CEOBDlg *m_pEOBDlg;
	void DestroyEOBDlg();

	// (d.moore 2007-09-26) - PLID 23861 - The NexForms Editor is now modeless. So it makes more
	//  sense to manage it from here rather than ProcedureDlg.
	CProcedureSectionEditDlg m_NexFormEditorDlg;

	// (a.walling 2008-04-04 10:05) - PLID 29544 - MDIClientWnd subclass; fills with gradient
	CMDIClientWnd m_MDIClientWnd;

	// (a.walling 2008-05-08 13:42) - PLID 29963 - Option to disable gradients or not
	BOOL m_bDisplayGradients;

	// (j.jones 2008-12-23 13:32) - PLID 32545 - tracks queued barcodes read in that have not yet been posted
	CStringArray m_arystrQueuedBarcodes;

	// (j.jones 2008-12-23 13:46) - PLID 32545 - this should be called when Idle() to post messages from our queue,
	// and Idle() should be called from the WinApp's OnIdle(), or from a WM_KICKIDLE message on modal dialogs
	void PumpNextQueuedBarcode();

	CLockBoxPaymentImportDlg *m_pLockboxPaymentImportDlg;
	void DestroyLockboxPaymentImportDlg();

	// (r.goldschmidt 2014-10-15 14:11) - PLID 62644 - called after eligibility request detail is closed
	void DestroyEligibilityRequestDetailDlg();
	
public:
	// (a.walling 2008-05-08 13:44) - PLID 29963 - returns m_bDisplayGradients
	inline BOOL GetDisplayGradients() { return m_bDisplayGradients; };
	CLabFollowUpDlg* m_pLabFollowupDlg; // (a.walling 2008-04-15 10:20) - PLID 25755
	BOOL ShowLabFollowUp(BOOL bShowAlways = FALSE); // (r.galicki 2008-10-13 14:59) - PLID 31373
	static BOOL HasLabsNeedingAttention(); // (r.galicki 2008-10-15 14:45) - PLID 31373	

	// (j.armen 2012-02-27 12:25) - PLID 48303
	CRecallsNeedingAttentionDlg* m_pRecallsNeedingAttentionDlg;
	bool ShowRecallsNeedingAttention(const bool& bUsePatientID = false);
	// (z.manning 2015-11-05 12:10) - PLID 57109
	void HandleRecallChanged();

	// (j.fouts 2012-9-5 3:28) - PLID 52482 - Need this dialog to be modeless
	CDrugInteractionDlg* m_pDrugInteractionDlg;

	// (j.fouts 2013-02-28 14:24) - PLID 54429 - Returns the number of interactions now, and -1 on failure
	// (j.jones 2012-09-26 14:09) - PLID 52872 - added patient ID
	// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
	// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
	// will show the dialog even if its current interactions have not changed since the last popup.
	long ShowDrugInteractions(long nPatientID, bool bRequery = true,
								bool bForceShowEvenIfBlank = false, bool bForceShowEvenIfUnchanged = false);

	// (j.fouts 2013-02-28 14:24) - PLID 54429 - Returns the number of interactions now, and -1 on failure
	// (j.fouts 2012-11-14 11:42) - PLID 53573 - Created a ShowOnInteractions to show without calling the API, this will use the arrays
	//	of interactions that are passed
	// (j.jones 2012-11-30 11:09) - PLID 53194 - Accurately renamed bForceShow to be bForceShowEvenIfBlank,
	// which shows the dialog even if there are no interactions, and added bForceShowEvenIfUnchanged, which
	// will show the dialog even if its current interactions have not changed since the last popup.
	long ShowDrugInteractions(Nx::SafeArray<IUnknown*> saryDrugDrugInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugAllergyInteracts,
									  Nx::SafeArray<IUnknown*> saryDrugDiagnosisInteracts,
									  long nPatientID,
									  bool bForceShowEvenIfBlank = false, bool bForceShowEvenIfUnchanged = false);

	// (j.fouts 2013-09-17 16:53) - PLID 58496 - Made this accessable
	long GetDrugInteractionsFilteredCount();

protected:
	//TES 9/26/03: Let's make this protected, because it doesn't check security or anything, 
	//so it should only be called by us after we've checked security.
	CFrameWnd *OpenModule(LPCTSTR moduleName = NULL, bool newModule = false);
	// (b.savon 2012-11-30 09:19) - PLID 53773
	bool IsDrugInteractionDlgCreated();
public:
	int				m_nOpenDocCnt;
	CPracticeDoc *	m_pOpenDoc;
	// (a.walling 2008-04-16 09:44) - PLID 29673 - Added descriptions, removed dead variables, added NxToolBar overload.
	//CStatusBar		m_wndStatusBar; // This is a dead variable, too
	CNxToolBar		m_wndToolBar; // Main toolbar
	CNxToolBar		m_GUIToolBar; // Refresh, print, preview, help
	//CToolBar		m_PtntButtons; // This is a dead variable
	CPatientToolBar m_patToolBar; // Patient toolbar
	CContactBar		m_contactToolBar; // Contacts toolbar
	CDocBar	*		m_pDocToolBar; // Marketing toolbar
	//CMenuBar		m_wndMenuBar;
	CMenu *	 m_pMenu; // current menu

	// (a.walling 2008-06-26 14:11) - PLID 30531 - Shared function to simply go to a patient in the patients module
	// (a.walling 2010-08-18 16:28) - PLID 17768 - Flag to check for deleted patients
	void GotoPatient(long nPatID, bool bCheckForDeleted = false);

	bool m_bIsPaletteChangedHandlerRunning;

	// (a.walling 2008-04-14 11:12) - PLID 29642 - This function allows us to specify a derived DockBar class
	void EnableDockingEx(DWORD dwDockStyle);

	void RefreshContactsModule();
	void SetStatusButtons();
	void SetTabStates(bool bState); 
	void SetToolBars(LPCTSTR moduleName);
	void ShowStandardBar(BOOL bShow);
	BOOL IsStandardBarVisible();
	BOOL IsContactsBarVisible();
	void ShowPatientBar(BOOL bShow);
	void ShowContactBar(BOOL bShow);
	void ShowDoctorBar(BOOL bShow);
//	void ShowDateBar(BOOL bShow);
	BOOL IsPatientBarVisible();
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg void OnAllClicked();
	afx_msg void OnAllSearchClicked();
	bool m_bLoggingTime;
	bool m_bIsReportRunning;
	//TES 8/24/2004: To be called once this is the main window.
	void PostCreate();

	void FireTimeChanged();

	CPtrArray m_paryOpenModules;
	void UpdateOpenModuleList(CNxView *pNewView);
	void RemoveFromOpenModuleList(CNxView *pView);
	BOOL m_bCloseNthModule;
	long m_nMaxOpenModules;
	void TryCloseNthModule();

	void OnToggleTWAINUseLegacy(); // (a.walling 2010-01-28 16:08) - PLID 37107 - Support fallback to legacy TWAIN 1.9
	void OnToggleTWAINShowUI();
	void OnToggleCallerID();
	void OnToggleShowG1Images();
	void OnToggleShowMirrorImages(); //TES 11/17/2009 - PLID 35709
	void OnToggleShowG2WarningDetails();
	void OnToggleShowPrimaryOnly();
	void OnUpdateCallerID(CCmdUI* pCmdUI);
	void OnUpdateTWAINUseLegacy(CCmdUI* pCmdUI); // (a.walling 2010-01-28 16:08) - PLID 37107 - Support fallback to legacy TWAIN 1.9
	void OnUpdateShowTWAINUI(CCmdUI* pCmdUI);
	void OnUpdateShowG1Images(CCmdUI* pCmdUI);
	void OnUpdateShowMirrorImages(CCmdUI* pCmdUI); //TES 11/17/2009 - PLID 35709
	void OnUpdateShowG2WarningDetails(CCmdUI* pCmdUI);
	void OnUpdateShowPrimaryImage(CCmdUI* pCmdUI);
	void OnUpdateBarcodeID(CCmdUI* pCmdUI);
	void OnUpdateSwiperID(CCmdUI* pCmdUI);
	void OnUpdateTestCallerID(CCmdUI* pCmdUI);
	void OnUpdatePracYakker(CCmdUI* pCmdUI);
	void OnUpdateAutoCaller(CCmdUI* pCmdUI);
	void OnUpdatePastApptsID(CCmdUI* pCmdUI);
	void OnUpdateUnitedID(CCmdUI* pCmdUI);
	void OnUpdatePocketPC(CCmdUI* pCmdUI);
	void OnUpdateNexSyncSettings(CCmdUI *pCmdUI); // (z.manning 2009-08-26 14:37) - PLID 35345
	void InitPracYakker();
	BOOL InitNxServer();
	void OnAutoCaller();
	void OnTestCallerID();
	void OnUpdatePastAppts();
	void OnUpdatePalmSettings(CCmdUI* pCmdUI);
	void OnUpdateImportEmrStandardContentMenu(CCmdUI* pCmdUI); // (z.manning 2009-04-09 11:04) - PLID 33934
	void OnPracYakkerSend();
	void OnHelpLicenseAgreement();
	// (a.pietrafesa 2015-22-06) PLID 65910 - add View Log File in Help menu
	void OnViewLogFile();
	// (j.jones 2010-01-11 14:19) - PLID 26786 - added ability to mass-update product information
	void OnUpdateInformationForMultipleProducts();
	void OnActivitiesUpdateInventoryPrices();
	// (c.haag 2007-11-29 10:41) - PLID 28006 - Let the user purchase items
	// in consignment into regular inventory
	void OnActivitiesPurchaseItemsFromConsignment();
	// (c.haag 2007-11-29 10:41) - PLID 28050 - Let the user transfer items
	// from regular inventory to consignment
	void OnActivitiesTransferItemsToConsignment();
	void OnToolsLogtimePrompt();
	void OnToolsLogtimeLogManagement();
	void OnActivitiesApplySuperbills();
	void OnActivitiesVoidSuperbills();
	void OnActivitiesEyeGraph();
	void OnExportToMedinotes();
	void OnShowPreferences();
	void OnSearchNotes();
	void OnChangePassword();
	void OnViewCurrentAlerts();
	void OnActivitiesSearchCheckCCNumbers();
	void OnFileSwitchUser(); // (a.walling 2007-05-04 12:07) - PLID 4850 - Switch users
	void OnActivitiesSearchReportDescriptions();
	void OnToolsHL7Settings();
	void OnToolsProcessHL7Messages();
	void OnModulesImportFromHL7();
	void OnImportFromNexWeb();
	//void OnImportFromLYB(); //(e.lally 2007-07-30) the call to this was taken out a while ago
	void OnScanMultipleDocs();
	void OnActivitiesMergePatient();
	void OnActivitiesInactivateMultiplePatients();
	// (j.gruber 2008-07-14 09:20) - PLID 30695 - configure report segments
	void OnActivitiesConfigureReportSegments();
	void OnActivitiesAddGiftCertificate();
	void OnActivitiesFindGiftCertificate();
	void OnActivitiesNexSpaEditCashDrawerSessions();
	void OnModulesExportToTelevox();
	void OnActivitiesSchedulingProductivity();
	void OnApptsWithoutAllocations();
	// (j.jones 2008-07-08 12:06) - PLID 24624 - added ability to run the patient summary
	void OnPatientSummary();
	// (a.walling 2008-07-16 17:19) - PLID 30751 - Parse transcriptions
	void OnParseTranscriptions();
	void OnViewInquiries();
	void OnImportEmrContent();
	void OnExportEmrContent(); // (d.singleton 5/24/2011 - 12:02:00) - PLID 43340 - have the importer use the shared path rather than install path for sales DB's
	void OpenEmrExe(LPCTSTR strExeName, BOOL bExport); // (d.singleton 5/24/2011 - 12:02:00) - PLID 43340 - have the importer use the shared path rather than install path for sales DB's
	void OnToolsImportNexFormsContent(); // (z.manning, 07/18/2007) - PLID 18359
	void OnToolsImportNxCycleOfCareContent();		//DRT 11/3/2008 - PLID 31789
	void OnToolsImportEmrStandardContent();	// (z.manning 2009-04-09 10:23) - PLID 33934
	void OnConfigureAAFPRSSurvey();
	void OnSearchAllowables();
	void OnToolsLogTimeLogOtherUser();	//	PLID 26192 6/6/2008 r.galicki
	void OnUpdateEncryptionKey(); // (a.walling 2010-03-15 14:18) - PLID 37755 - Manual encryption key update requested
	// (j.jones 2009-02-27 12:21) - PLID 33265 - added e-prescribing settings
	//TES 4/10/2009 - PLID 33889 - Broke the function up into type-specific functions
	//void OnEPrescribingSettings();
	void OnNewCropSettings();
	void OnSureScriptsSettings();
	// (b.savon 2012-11-01 11:40) - PLID 53559
	void OnRegisterNexERxClientAndPrescribers();
	void DisplayRegisterNexERx();
	// (b.savon 2013-06-27 09:46) - PLID 57344 - Post the message
	struct CPopUpMessageBoxInfo
	{
		CString strMessage;
		UINT unMessageBoxType;
	};
	void PopUpMessageBoxAsync(const CString &strMessage, const UINT messageBoxType);
	// (j.gruber 2009-05-15 10:27) - PLID 28541 - new crop renewal requests
	void OnNewCropRenewalRequests();
	void OnImportCCD(); // (a.walling 2009-05-13 15:44) - PLID 34243 - Import CCD documents
	void OnCreateCCD(); // (a.walling 2009-05-21 08:41) - PLID 34318 - Create a CCD summary document
	void CreateCCD(long nPatientID, CWnd* pParent) throw(...); // (a.walling 2009-06-05 13:15) - PLID 34176	
	//Caller ID Functions
	void ShutdownTapi();
	HRESULT InitializeTapi();
	HRESULT ListenOnThisAddress(ITAddress * pAddress);
	HRESULT ListenOnAddresses();
	HRESULT RegisterTapiEventInterface();
	BOOL AddressSupportsMediaType(ITAddress * pAddress, long lMediaType);
	void OnSysCommand(UINT nID, LPARAM lParam); // (z.manning 2009-08-25 11:15) - PLID 31944

	// (j.jones 2009-03-02 14:40) - PLID 33053 - added ability to open the NewCrop web browser from anywhere
	// (j.gruber 2009-03-30 11:30) - PLID 33736 - added nEMNID and nPatientID
	// (j.gruber 2009-03-30 12:19) - PLID 33728 - added CWnd to post closing message to
	// (j.gruber 2009-05-15 17:12) - PLID 28541 - added strXML for renewal responses
	// (j.gruber 2009-06-08 11:59) - PLID 34515 - added user role
	// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
	void OpenNewCropBrowser(CString strDefaultWindowText, NewCropActionType ncatDefaultAction, long nActionID, long nPatientPersonID, long nPatientUserDefinedID, long nEMNID, CWnd *pWndToSendClosingMsgTo, OPTIONAL IN CString strXML);

	//DRT 10/25/02 - Show Release
	void StartLogTime();
	void EndLogTime();

	void ImportPatientFile();

	void ShowPatientEMNsToBeBilled();

	//Event tracking function.
	void LinkToEvent(PhaseTracking::EventType nType, long nID);

	BOOL CMainFrame::OnNeedText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	BOOL CMainFrame::GetToolText( UINT nID, CString& strTipText );

	void AllowPopup();
	void DisallowPopup();

	//Adds the window associated with hwndMessageRecipient to the list of windows that will be notified
	//when a tablechecker message is received.
	void RequestTableCheckerMessages(HWND hwndMessageRecipient);
	void UnrequestTableCheckerMessages(HWND hwndMessageRecipient);

protected:
	// (a.walling 2009-07-10 09:31) - PLID 33644
	BOOL m_bNxServerBroadcastStagger;
	long m_nTableCheckerStaggerMin;
	long m_nTableCheckerStaggerMax; // * # of workstations

	// (j.jones 2014-08-20 09:40) - PLID 63427 - Given a tablechecker type,
	// returns true if the tablechecker should be processed immediately or
	// added to our staggered queue.
	// Will always return true if the local stagger is disabled.
	bool ShouldImmediatelySendTablechecker(NetUtils::ERefreshTable ertTableType);

	struct CPendingMessage {
		CPendingMessage(UINT message, WPARAM wp, LPARAM lp)
			: msg(message), wParam(wp), lParam(lp)
		{
		};

		UINT msg;
		WPARAM wParam;
		LPARAM lParam;
	};

	CList<CPendingMessage*, CPendingMessage*> m_listPendingTableCheckerMessages;
	
	LRESULT QueuePendingMessage(UINT message, WPARAM wParam, LPARAM lParam) throw(...);

	// (j.jones 2014-08-19 10:19) - PLID 63412 - the tablechecker queue now works off of an elapsed time,
	// and will keep processing until a set period of time has passed
	void BeginProcessingQueuedMessages();
	void ProcessNextQueuedMessage(DWORD dwInitialTickCount, long nCountProcessed);

	void ResetQueuedMessagesTimer();
	void CleanupAllQueuedMessages();
	long m_nQueuedMessageTimerID;

	// (j.jones 2014-08-19 10:21) - PLID 63412 - the batch limit also has a timer now,
	// the count is now a minmum
	long m_nQueuedMessageBatchMinLimit;
	long m_nQueuedMessageBatchTimeLimitMS;

public:

	CRPEngine *GetReportEngine();
	void ClearReportEngine();

	// (j.gruber 2009-05-11 11:43) - PLID 34202 - Editing Provider Tyeps
	void OnEditProviderTypes();

	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	// (d.lange 2010-10-26 13:44) - PLID 41088 - Returns whether the Device Importer has images waiting to be imported
	// (j.jones 2011-03-15 13:54) - PLID 42738 - this now returns a count of records
	long GetDeviceImportRecordCount();
	// (d.lange 2010-06-07 23:50) - PLID 38850 - added DevicePluginImportDlg as a modeless dialog
	void ShowDevicePluginImportDlg();
	// (d.lange 2011-03-11 11:01) - PLID 41010 - Refresh the device import dialog if the current patient filter is enabled
	void NotifyDeviceImportPatientChanged();	

public:
	CMirrorLink *GetMirrorLink();
	CUnitedLink *GetUnitedLink();

protected:
	CMirrorLink *m_pMirrorDlg;
	CUnitedLink *m_pUnitedDlg;

	CNotificationDlg *m_pNotificationDlg;
	//TES 1/4/2007 - PLID 24119 - Make sure m_pNotificationDlg has been created before we try to access it.
	void EnsureNotificationDlg();

	CRPEngine* m_pReportEngine;
	// (a.walling 2009-10-14 13:20) - PLID 35941 - The report engine destructor can result in the mainframe's ClearReportEngine being called again!
	bool m_bReportEngineClosing;

	// (c.haag 2003-08-01 17:41) - This is here because we want to be able to pump
	// barcode messages to this window.
	CProductItemsDlg m_dlgProductItems;

	void ShowControlBar(CControlBar* pBar, BOOL bShow, BOOL bDelay);
	WORD m_enableViews;//enabled buttons on the toolbar
	WORD m_initialEnabledViews;//views that can be enabled (ie security)
	WORD m_licensedViews;// (a.walling 2008-07-02 13:57) - PLID 29757 - licensed buttons on the toolbar
	void Lookup(int ID);
	void LookupSimple(long nFilterId, FieldOperatorEnum nOperator, const CString &strInitValue);

	BOOL DoesUserHaveTasks(long nUserID);

	bool m_bMovingDocBar;		//to allow the marketing toolbar time to move when you use the arrows

	long m_nCountPopupDisallows; //When this reaches 0, popups are allowed.
	BOOL IsPopupAllowed();
	long m_nWaitingType; //Are we currently waiting to pop something up?
	// Will do whatever is appropriate to notify the user in the specified way..
	// nNotificationType: NULL = default implementation (CAlertDlg), NT_TODO = TodoAlarm, NT_YAK = Yakker.
	// strMessage: notification message; ignored for NT_TODO  and NT_YAK
public:
	// (a.walling 2010-10-11 17:08) - PLID 40731 - Handle an ID and alert type (only for NT_DEFAULT)
	void NotifyUser(int nNotificationType, const CString &strMessage, bool bImmediatePopup = true, long nAssociatedID = -1, EAlert alertType = GenericAlert);
	//Will stop notifiying the user about the given type.
	void UnNotifyUser(int nNotificationType);
protected:

	// Sets whatever icon we've got to blinking, and sets any appropriate member variables.
	void AddToNotification(int nNotificationType);
	// (a.vengrofski 2010-08-18 09:31) - PLID <38919> - Use this for HL7 messages.
	void AddToNotification(int nNotificationType, const CString &strMessage);

	//List of windows who have requested that they be sent tablechecker messages.
	CArray<HWND, HWND> m_arCheckerWnds;
// Automatic file deletion
protected:
	// We're using the sorted no-case array because we want to 
	// ignore case and ignore duplicates.  we don't really care 
	// about sorting but we don't mind it and it makes the insert 
	// more efficient so kudos to whoever wrote it
	CStringSortedArrayNoCase m_arystrDeleteFiles;
	DECLARE_MEMBER_TIMER(IDT_DELETE_QUEUED_FILES_TIMER);
public:
	void DeleteCommonContactInfo();
	void DeleteOtherContact();
	void DeleteSupplier();
	// (c.haag 2008-06-23 13:07) - PLID 30471 - Added nAssignEmrTodoActionsTo, and removed optional parameters
	void DeleteUser(BOOL bDeleteMessages, long nAssignStepsTo, long nAssignTodosTo, long nAssignEmrTodoActionsTo);
	void DeleteProvider();
	void DeleteRefPhys();
	BOOL DeleteFileWhenPossible(LPCTSTR strFilePathName);
	//does not attempt to delete a file until nDelayInMS has elapsed
	BOOL DeleteFileWhenPossible(LPCTSTR strFilePathName, long nDelayInMS);
	void DeleteQueuedFiles(BOOL bForceDeQueue = FALSE);
public:
	enum DefaultContactType{
		dctProvider = 1,
		dctRefPhys	= 2,
		dctSupplier	= 3,
		dctOther	= 4,
		dctEmployer	= 5
	};
	long AddContact(DefaultContactType dctType, BOOL bSaveEditEnable = TRUE);


public:
	CWhatsNewHTMLDlg *m_pWhatsNewDlg;
	void ShowWhatsNew(BOOL bForceWhatsNew = FALSE);// (a.vengrofski 2009-11-02 14:29) - PLID <36104> - Added a passed bool to force the dlg to show.
	void ResetInactivityTimer();

protected:
	int m_nHighlightSortedColumn; // 0 means FALSE (do NOT highlight), 1 means TRUE (DO highlight), 2 means unknown
	BOOL m_bUseEnhancedNxColor; // (a.walling 2008-04-03 15:00) - PLID 29497
	BOOL m_bToolbarText; // (a.walling 2008-04-21 13:37) - PLID 29731 - Whether we are displaying toolbar text or not.

	// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
	//BOOL m_bToolbarBorders; // (a.walling 2008-10-07 12:14) - PLID 31575 - Toolbar borders

public:
	BOOL m_bIsWaitingForProcess;
	BOOL m_bAlreadyCheckedModulePermissions;
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	int ShellExecuteModal(IN LPCTSTR strFile, IN LPCTSTR strParameters = "", IN OPTIONAL LPCTSTR strWorkingPath = NULL, IN OPTIONAL LPCTSTR strOperation = NULL);

public:
	CNotificationDlg *GetNotificationDlg();

	// (j.jones 2011-03-15 15:33) - PLID 42738 - returns TRUE if the requested
	// notification type exists in our notification dialog already
	BOOL IsNotificationTypeInQueue(int nNotificationType);

	// (a.walling 2009-12-22 17:17) - PLID 7002 - Is a billing window already active?
	bool IsBillingModuleOpen(bool bShowMessage, CWnd** ppReturnBillingModule = NULL);

public:
	long m_nPatientIDToGoToOnClickNotification;
	void OnReceivedCallerIdNumber(LPCTSTR strCallerIdNumber);
	// (j.gruber 2010-01-07 15:04) - PLID 36648 - add referral source number	
	long m_nReferralIDForNewPatient;
	// (j.gruber 2010-01-11 13:42) - PLID 36140 - boolean for if newpatient is already open
	bool m_bNewPatientOpen;

public:
	BOOL CheckAllowCreatePatient();

	BOOL RegisterForBarcodeScan(CWnd* pWnd);
	BOOL UnregisterForBarcodeScan(CWnd* pWnd);

	// (j.jones 2009-02-10 12:12) - PLID 32870 - added HasQueuedBarcodes, which will simply tell is if
	// m_arystrQueuedBarcodes has any values in it
	BOOL HasQueuedBarcodes();

	// You must call UnregisterTwainDlgWaiting() once for each call made to 
	// RegisterTwainDlgWaiting() for the same CWnd*.  Messages will be sent 
	// to each CWnd* only once per twain import, regardless of how many times you 
	// register the pointer.
	// (a.walling 2008-07-24 17:50) - PLID 30836 - Made this more generic by supporting
	// any CWnd rather than a specific class
	void RegisterTwainDlgWaiting(CWnd* pWnd);
	void UnregisterTwainDlgWaiting(CWnd* pWnd);

	// (a.walling 2009-06-12 13:23) - PLID 34176 - Generic modeless window tracker
	void RegisterModelessWindow(HWND hwnd);
	void UnregisterModelessWindow(HWND hwnd);

	// (z.manning 2008-07-16 15:29) - PLID 30490 - We keep track of the IDs of any HL7 settings
	// group that exports to the schedule. This way we only need to load these once ever (plus
	// any time we get a table checker message that the settings have changed) rather than
	// loading them every single time an appointment is created, modified, or cancelled (which
	// is what patient exports do).
	CArray<long,long> m_arynScheduleHL7GroupIDs;
	void LoadScheduleHL7GroupIDs();
	void EnsureInScheduleHL7GroupIDs(const long nID);
	void EnsureNotInScheduleHL7GroupIDs(const long nID);

	// (a.walling 2008-09-10 13:20) - PLID 31334 - Last active PIC window
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	/*
	void SetLastActivePIC(CPicContainerDlg* pPic) {
		m_pLastActivePIC = pPic;
	};
	*/

	// (a.walling 2008-09-10 17:39) - PLID 31334 - Ensure this window is NOT the last active one
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	/*
	void EnsureNotActivePIC(CPicContainerDlg* pPic) {
		if (m_pLastActivePIC == pPic) {
			m_pLastActivePIC = NULL;
		}
	};
	*/

	// (j.jones 2008-10-30 16:52) - PLID 31869 - track the thread ID for the Problem List's PopulateDetailValuesThread,
	// and the Problem deletion dialog's PopulateDeletedProblemValuesThread
	long m_nProblemListDlg_PopulateDetailValues_CurThreadID;
	long m_nProblemDeleteDlg_PopulateDeletedProblemValues_CurThreadID;
	
	// (a.walling 2008-09-18 10:24) - PLID 26781 - Generic function to broadcast message to PIC windows
	void BroadcastPostMessageToPICs(UINT msg, WPARAM wParam, LPARAM lParam, CWnd* pExclude = NULL);

	// (z.manning 2009-05-19 12:16) - PLID 28512 - Call this to handle any additional functionality needed
	// when changing the active patient.
	void HandleActivePatientChange();

	//TES 1/6/2010 - PLID 36761 - Put the handling for OnFilterSearchClicked in its own function so it can be called by
	// CPatientToolbar.
	void HandleFilterSearchClicked();

// Generated message map functions
protected:
	void AdminCPTEdit();
	long GetOpenModuleCount();
	void * m_pLockPatToolBar;
	CString m_strClipType;
	int m_nClipMethod;
	CClip *m_pClipData;
	CPtrArray m_pView;
	void DockControlBarLeftOf(CToolBar* Bar, CToolBar* LeftOf);
	void DockControlBarBelow(CControlBar* Bar, CControlBar* BelowMe);
	long SaveDeleteRecord();
	void TransferTodos();
	void UnselectDeletedPatientFromToolbar();
	
	// (a.walling 2010-01-27 13:17) - PLID 22496 - Return the formatted title bar text
	CString GenerateTitleBarText(CString strOverride = "");
	// (a.walling 2010-01-27 13:17) - PLID 22496 - Reset the title bar text timer which checks for date changes
	// return true if the date changed since last reset
	bool ResetTitleBarTextTimer();

	CArray<CWnd*, CWnd*> m_aryBarcodeWindows;
#ifdef _DEBUG
	// (a.walling 2008-02-21 09:15) - PLID 29043 - Keep a list of window text as well for debugging
	CStringArray m_saBarcodeWindowText;
#endif

	bool m_bForciblyClosing;

	// (a.walling 2008-07-24 17:49) - PLID 30836 - Made this more generic rather than a specific kind of dialog
	CMap<class CWnd*, class CWnd*, LONG, LONG> m_mapTwainDlgsWaiting;

	// (a.walling 2009-06-12 13:27) - PLID 34176 - Generic modeless window handling
	CMap<HWND, HWND, long, long&> m_mapModelessWindows;

	// (a.walling 2008-09-10 13:20) - PLID 31334 - Last active PIC window
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	//CPicContainerDlg* m_pLastActivePIC;

	// (a.walling 2008-09-09 17:23) - PLID 31334 - WIA event sink
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	//NxWIA::CWIAEventSink* m_pWIAEventSink;
	// (a.walling 2008-10-28 13:51) - PLID 31334 - Disabled for now
	// (a.walling 2014-03-12 12:31) - PLID 61334 - reduce stdafx dependencies and #imports
	//WIA::IDeviceManagerPtr m_pWIADeviceManager;
	// (a.walling 2008-09-10 14:45) - PLID 31334 - Registered WIA events
	//CStringArray m_arRegisteredWIAEvents;

	void SaveLastOpenModule(); // (a.walling 2007-05-14 17:13) - PLID 4850 - Remember which module was open just before closing the program

public:
// (s.tullis 2016-01-28 17:46) - PLID 68090
// (s.tullis 2016-01-28 17:46) - PLID 67965
	void RefreshNexERxNeedingAttention(BOOL bLogin = FALSE);
	BOOL CanShowAttentionRenewalMessages(CString strProp, CString strPropValue, UINT_PTR uTimerID , BOOL &bTimerActive ,BOOL bLogin = FALSE);
	BOOL CanShowRxAttentionNotifications(BOOL bLogin = FALSE);
	BOOL CanShowRenewalNotifications(BOOL bLogin = FALSE);
	void ResetRxAttentionTimer();
	void ResetRenewalTimer();
	BOOL GetCanShowRxAttentionMessages();
	void SetCanShowRxAttentionMessages(BOOL bValue);
	BOOL GetCanShowRenewalMessages();
	void SetCanShowRenewalMessages(BOOL bValue);
protected:
	// (j.fouts 2013-01-25 17:29) - PLID 54472 - Added NexERx
	//Maps a provider ID to their last notified renewal time
	std::map<long,COleDateTime> m_mapProvIDToRenewalTime;
	//Maps a provider ID to their last notified prescription time
	std::map<long,COleDateTime> m_mapProvIDToPrescriptionTime;
	//Has the NexERx Needing Attention Notification been loaded
	bool m_bNexERxNeedingAttentionLoaded;
	//The message that we should display for a NexERx notification
	CString m_strSureScriptsNotificationMessage;

	//TES 4/21/2009 - PLID 33376 - Our one and only SureScriptsCommDlg.
	CSureScriptsCommDlg* m_pSureScriptsCommDlg;
	// (s.tullis 2016-01-28 17:46) - PLID 68090
	BOOL m_bTimerGoingErxNeedingAttentionMessages;
	BOOL m_bCanShowAttentionMessages;
	// (s.tullis 2016-01-28 17:46) - PLID 67965
	BOOL m_bTimerGoingRecieveRenewalMessages;
	BOOL m_bCanShowRenewalMessages;

public:
	//TES 1/11/2010 - PLID 36761 - Does the current user have permission to access the given patient?
	bool CanAccessPatient(long nPatientID, bool bSilent);

	//TES 1/11/2010 - PLID 36761 - Which Security Groups does the current user not have access to?
	void GetBlockedGroups(OUT CArray<IdName,IdName> &arBlockedGroups);

	//TES 1/15/2010 - PLID 36762 - Which patients has the user been granted emergency access to?
	void GetEmergencyAccessPatients(OUT CArray<long,long> &arPatientIDs);

protected:
	bool m_bBlockedGroupsValid;
	CArray<IdName, IdName> m_arBlockedGroups;
	// (j.jones 2010-01-14 14:14) - PLID 35775 - If the user gains emergency access to a patient,
	// they have access for the remainder of their Practice session. Store those IDs here.
	CArray<long, long> m_arEmergencyAccessedPatientIDs;

public:
	
	// (j.jones 2010-02-10 11:33) - PLID 37224 - functions for loading and accessing EMR image stamps
	void LoadEMRImageStamps(BOOL bForceReload = FALSE);
	void ClearCachedEMRImageStamps();
	// (j.jones 2010-02-10 11:33) - PLID 37224 - EditEMRImageStamps will return TRUE
	// if anything was changed
	// (a.walling 2012-03-12 10:06) - PLID 46648 - Dialogs must set a parent!
	BOOL EditEMRImageStamps(CWnd* pParent);
	// (j.jones 2010-02-16 15:01) - PLID 37224 - be able to return one stamp pointer by ID,
	// returns NULL if no stamp exists with that ID
	EMRImageStamp* GetEMRImageStampByID(long nStampID);
	// (z.manning 2010-03-01 17:14) - PLID 37571 - Function to get actions by stamp ID
	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
	void GetEmrActionsByStampID(const long nStampID, BOOL bIncludeDeleted, OUT MFCArray<EmrAction> &aryActions);

	// (j.jones 2010-02-10 11:33) - PLID 37224 - cache EMR image stamps globally
	BOOL m_bEMRImageStampsLoaded;
	CArray<EMRImageStamp*, EMRImageStamp*> m_aryEMRImageStamps;
	// (j.jones 2013-06-21 14:42) - PLID 57269 - added a map to look up stamps by ID
	CMap<long, long, EMRImageStamp*, EMRImageStamp*> m_mapEMRImageStamps;

protected:
	// (z.manning 2011-07-05 12:39) - PLID 44421 - We load and cache EMR coding groups in a similar manner to image stamps
	// where we load them once and then cache them. If this pointer is not null that means they've been loaded.
	CEmrCodingGroupArray *m_parypEmrCodingGroupManager;
public:
	CEmrCodingGroupArray* GetEmrCodingGroupManager();
	void ClearCachedEmrCodingGroups();

protected:
	// (c.haag 2010-06-08 17:55) - PLID 38898 - The modeless NexPhoto Import form
	NxManagedWrapperLib::INxPhotoImportFormPtr m_pNxPhotoImportForm;

	// (c.haag 2010-06-22 13:37) - PLID 39295 - We now listen for NexPhoto import form events
	CNxManagedWrapperEventSink m_NxPhotoEventSink;

	//(c.copits 2011-10-25) PLID 45709 - Deleting a supplier that is used in the Glasses Catalog Setup will generate a FK constraint error.
	BOOL CheckGlassesSupplierDesigns();
	BOOL CheckGlassesSupplierMaterials();
	BOOL CheckGlassesSupplierTreatments();
	BOOL CheckGlassesSupplierFrames();

	// (b.spivey, May 02, 2013) - PLID 56542 - Track the status in this variable. 
	BOOL m_bIsTopazConnected; 

public:

	// (b.spivey, May 02, 2013) - PLID 56542 - Accessor/Mutator
	BOOL GetIsTopazConnected();
	void SetIsTopazConnected(BOOL bIsConnected);	

	// (c.haag 2010-06-08 17:55) - PLID 38898 - Shows or hides the NexPhoto modeless import form
	void ShowNexPhotoImportForm(BOOL bShow);

	// (c.haag 2010-06-28 15:11) - PLID 39392 - Returns true if the NexPhoto Import window is open.
	BOOL IsNexPhotoImportFormOpen() const;
	
protected:
	// (z.manning 2014-12-01 16:37) - PLID 64205
	void OpenTemplateEditor(ESchedulerTemplateEditorType eType, COleDateTime dtDefaultDate = g_cdtInvalid);

public:
	//(e.lally 2010-07-15) PLID 39626 - Opens the templated editor for location templates (in the scheduler)
	// (j.jones 2011-07-15 14:45) - PLID 39838 - added default date
	void EditLocationTemplating(COleDateTime dtDefaultDate = g_cdtInvalid);

	// (z.manning 2014-12-01 13:53) - PLID 64205
	void OpenSchedulerTemplateCollectionEditor();

	// (d.lange 2011-03-09 16:53) - PLID 41010 - Returns an array of open EMR patient IDs
	void GetOpenEMRPatientIDs(OUT CArray<long,long> &aryOpenEMRPatientIDs);

	// (r.wilson 5/2/2012) PLID 43741 - Holds glasses order statuses in memory
	CMap<CString,LPCSTR,long,long> m_mapGlassesOrderStatus;	

	//TES 10/31/2013 - PLID 59251 - Notify the user that the interventions identified in the array (DecisionRuleInterventionT.IDs) have been triggered
	void DisplayNewInterventions(const CDWordArray &arInterventionIDs);
	//TES 11/1/2013 - PLID 59276 - Displays all pending interventions, and then clears out the list
	void OpenInterventionsList();

	//(b.spivey, December 13th, 2013) - PLID 59022 - Cleaner to put this in a function.
	void UpdateDirectMessageAddressData(long nID, long nStatus);

	// (z.manning 2015-08-04 14:53) - PLID 67221 - Credit card device manager object
	CICCPDeviceManager *m_pICCPDeviceManager;
	void EnsureICCPDeviceManager();
	// (z.manning 2015-09-04 09:08) - PLID 67236 - Added overload
	void EnsureICCPDeviceManager(BOOL bIgnorePreference);
	// (z.manning 2015-09-30 17:03) - PLID 67255
	void RefreshICCPDeviceManager();

	// (v.maida 2016-02-19 15:43) - PLID 68385 - Added m_bPracticeMinimized.
	BOOL m_bPracticeMinimized;

protected:
	//TES 11/1/2013 - PLID 59276 - Tracks CDS Interventions that have been triggered but not displayed
	CDWordArray m_arPendingInterventionIDs;

protected:
	// (c.haag 2016-05-31 11:51) - NX-100789 - Associates NXB_TYPE's with application resource ID's
	void InitializeNexTechIconButtonIcons();

protected:

	// (a.walling 2008-10-28 13:05) - PLID 31334 - Added OnWIASourceLaunch, when launched from WIA auto-start
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPatientsModule();
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos); // (a.walling 2013-02-05 12:24) - PLID 55019 - Mainfrm needs to coordinate with owned CNxModelessDialog windows to prevent owner activation.
	// (a.walling 2014-06-09 09:37) - PLID 57388 - Placeholders - useful for debugging without rebuilding everything
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg LRESULT OnActivateTopLevel(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSchedulerModule();
	afx_msg void OnUpdateView();
	afx_msg void OnViewToolbarsPatientsBar();
	afx_msg void OnUpdateViewToolbarsPatientsBar(CCmdUI* pCmdUI);
	afx_msg void OnViewToolbarsStandardBar();
	afx_msg void OnUpdateViewToolbarsStandardBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewToolbarsContactsBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHighlightSortedColumn(CCmdUI* pCmdUI);
	afx_msg void OnViewHighlightSortedColumn();
	afx_msg void OnChoosePatient();
	afx_msg void OnGotoFirstPatient();
	afx_msg void OnGotoLastPatient();
	afx_msg void OnGotoPreviousPatient();
	afx_msg void OnGotoNextPatient();
	afx_msg void OnPatientBreadcrumbs(); // (a.walling 2010-05-21 10:12) - PLID 17768 - Popup the patient breadcrumbs menu
	afx_msg void OnUpdateChoosePatient(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGotoFirstPatient(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGotoLastPatient(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGotoPreviousPatient(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGotoNextPatient(CCmdUI* pCmdUI);
	afx_msg void OnNewPatientBtn(long nReferralID = -1);
	afx_msg void OnPalmPilot();
	afx_msg void OnNexSyncSettings(); // (z.manning 2009-08-26 14:39) - PLID 35345
	afx_msg void OnAppointmentReminderSettings();
	afx_msg void OnNexPDALinkSettings();
	afx_msg void OnFirstAvailableAppt();
	afx_msg void OnPatientDelete(bool bSilent = false);
	afx_msg void OnActiveClicked();
	afx_msg void OnMainPhysSearchClicked();
	afx_msg void OnReferringPhysSearchClicked();
	afx_msg void OnEmployeesSearchClicked();
	afx_msg void OnOtherContactsSearchClicked();
	afx_msg void OnSupplierSearchClicked();
	afx_msg void OnActiveContacts();
	afx_msg void OnAllContacts();
	afx_msg void OnAllContactsSearchClicked();
	afx_msg void OnPatientSearchClicked();
	afx_msg void OnProspectSearchClicked();
	afx_msg void OnFilterSearchClicked();
	afx_msg void OnUpdatePatientDelete(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSchedulerModule(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePatientModule(CCmdUI* pCmdUI);
	afx_msg void OnAdministratorModule();
	afx_msg void OnContactsModule();
	afx_msg void OnFinancialModule();
	afx_msg void OnInventoryModule();
	afx_msg void OnLetterWritingModule();
	afx_msg void OnMarketingModule();
	afx_msg void OnReportsModule();
	afx_msg void OnSurgeryCenterModule();
	afx_msg void OnLinksModule();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnTemplatingBtn(); // (z.manning 2014-12-01 13:51) - PLID 64205
	afx_msg void OnTemplateCollectionsBtn();
	afx_msg void OnResourceAvailTemplating();
	afx_msg void OnAptBookingAlarmsBtn();
	afx_msg void OnRoomManagerBtn();
	afx_msg void OnReschedulingQueue();
	afx_msg void OnUpdateAdministratorModule(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInventoryModule(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLetterWritingModule(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMarketingModule(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReportsModule(CCmdUI* pCmdUI);
	afx_msg void OnOpenTodoList();
	afx_msg void OnPaperBatch();
	afx_msg void OnDownload();
	afx_msg void OnFilter();
	afx_msg void OnViewToolbarsGui();
	afx_msg void OnUpdateViewToolbarsGui(CCmdUI* pCmdUI);
	afx_msg void OnManualBtn();
	afx_msg void OnManualToolbarBtn();
	afx_msg void OnPDFManual();
	afx_msg void OnSuperbillall();
	afx_msg void OnSuperbill();
	afx_msg void OnEnvelope();
	afx_msg void OnPrescription();
	afx_msg void OnBatchMerge();
	afx_msg void OnEducationalMaterials();
	// (j.jones 2008-07-08 12:06) - PLID 24624 - added ability to run the patient summary
	afx_msg void OnUpdatePatientSummary(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSuperbill(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEnvelope(CCmdUI* pCmdUI);	
	afx_msg void OnUpdateEducationalMaterials(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrescription(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAppointmentReminderSettings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTrackSerializedItems(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGenerateBarcodes(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePatientSerializedItems(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNextContactBtn(CCmdUI* pCmdUI);
	afx_msg void OnNextContactBtn();
	afx_msg void OnPrevContactBtn();
	afx_msg void OnUpdatePrevContactBtn(CCmdUI* pCmdUI);
	afx_msg void OnApplyCareCredit();
	afx_msg void OnLinkToMirror();
	afx_msg void OnLinkToUnited();
	afx_msg void OnLinkToQuickbooks();
	afx_msg void OnLinkToCodeLink();
	afx_msg void OnLogFile();
	afx_msg void OnConfigStatement();
	afx_msg void OnConfigReportGroups();
	afx_msg void OnEditReports();
	afx_msg void OnTrackSerializedItems();
	afx_msg void OnGenerateBarcodes();
	afx_msg void OnPatientSerializedItems();
	afx_msg void OnUpdatePrefixes();	
	afx_msg void OnAdminCPTEdit();
	afx_msg void OnUpdateBoolLogFile(CCmdUI* pCmdUI);
	afx_msg void OnNewContact();
	afx_msg void OnContactDelete();
	afx_msg void OnInform();
	// (j.gruber 2007-07-03 09:36) - PLID 26535 - Credit Card Processing Setup
	afx_msg void OnCreditCardProcessingSetup();
	// (j.gruber 2007-07-09 17:09) - PLID 26584 - Credit Card Batch Processing
	afx_msg void OnCreditCardBatchProcessing();
	// (d.thompson 2009-04-14 17:09) - PLID 33973
	afx_msg void OnReviewCreditCardTransactions();
	// (j.gruber 2007-08-16 13:06) - PLID 27091 - Tracking Conversions Report Configure dialog
	afx_msg void OnTrackingConversionsConfigDlg();
	afx_msg void OnCareCreditSetupDlg();
	afx_msg void OnUMLSLoginDlg();// (j.camacho 2013-10-03 16:59) - PLID 58678
	//TES 8/24/2004: These buttons no longer exist.
	/*afx_msg void OnNextDocBtn();
	afx_msg void OnUpdateNextDocBtn(CCmdUI* pCmdUI);
	afx_msg void OnPrevDocBtn();
	afx_msg void OnUpdatePrevDocBtn(CCmdUI* pCmdUI);*/
	afx_msg void OnViewResettoolbars();
	afx_msg void OnUpdateViewResettoolbars(CCmdUI* pCmdUI);
	afx_msg void OnSubmitToChaseHealthAdvance();
	afx_msg void OnHelp();          // F1 (uses current context)
	afx_msg void OnHelpIndex();     // ID_HELP_INDEX
	afx_msg void OnHelpFinder();    // ID_HELP_FINDER, ID_DEFAULT_HELP
	afx_msg void OnHelpUsing();     // ID_HELP_USING
	afx_msg void OnBatchPayments();
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnNotificationClicked(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMessageAllowPopup(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIDPASetup();
	afx_msg void OnNYWCSetup();	
	// (j.jones 2007-05-09 17:47) - PLID 25550 - added MICR Setup
	afx_msg void OnMICRSetup();
	// (k.messina 2010-07-15 16:15) - PLID 39685 - added NY Medicaid Setup
	afx_msg void OnNYMedicaidSetup();
	afx_msg void OnSSRSSetup();
	afx_msg void OnCalc();
	afx_msg void OnCalcFinanceCharges();
	// (j.jones 2013-08-01 13:19) - PLID 53317 - added ability to undo finance charges
	afx_msg void OnUndoFinanceCharges();
	afx_msg void OnConnectToTechSupport();
	afx_msg void OnViewWebinars();
	// (a.walling 2007-05-17 16:11) - PLID 26058 - cash drawer config
	afx_msg void OnCashDrawerSettings();
	// (j.gruber 2007-05-07 15:14) - PLID 25772 - receipt printer configurations
	afx_msg void OnReceiptPrinterSettings();
	// (j.gruber 2007-05-08 12:28) - PLID 25931 - receipt settings dialog
	afx_msg void OnConfigureReceipts();
	afx_msg LRESULT OnMessageOpenMoveupDialog(WPARAM wParam, LPARAM lParam);
	afx_msg void OnChangeLocation();
	afx_msg LRESULT OnExplicitDestroyView(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOutlookSyncFinished(WPARAM wParam, LPARAM lParam);
	afx_msg void PostToBillID();
	afx_msg LRESULT OnPromptMoveup(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewPayment(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTWAINSourceLaunch(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWIASourceLaunch(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEMRGroupClosing(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintEMNReport(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNewEmr();
	afx_msg void OnDropDown(NMHDR * pNotifyStruct, LRESULT * lResult);
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	afx_msg void OnUpdateLicense();
	afx_msg void OnDeactivateWorkstations();
	afx_msg LRESULT OnForciblyClose(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRecheckLicense(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreviewPrinted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPreviewClosed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmrItemEntryDlgItemSaved(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisplayErrorMessage(WPARAM wParam, LPARAM lParam);
	// (b.savon 2013-06-27 10:02) - PLID 57344
	afx_msg LRESULT OnPopUpMessageBox(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitiateOPOSMSRDevice(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitiateOPOSPrinterDevice(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitiateOPOSCashDrawerDevice(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPopupApptsWoAllocations(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitiateOPOSBarcodeScanner(WPARAM wParam, LPARAM lParam); //(a.wilson 2012-1-10) PLID 47517
	// (a.walling 2008-06-26 13:13) - PLID 30531 - Handler for editing a template or EMN
	afx_msg LRESULT OnMsgEditEMROrTemplate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnReprintSuperbills();
	afx_msg void OnCCDASettings(); // (j.gruber 2013-10-08 13:16) - PLID 59062

	// (j.jones 2009-12-28 08:59) - PLID 32150 - added ability to delete unused service codes
	afx_msg void OnDeleteUnusedServiceCodes();
	// (j.jones 2012-03-27 08:59) - PLID 45752 - added ability to delete unused diagnosis codes
	afx_msg void OnDeleteUnusedDiagCodes();
	// (j.jones 2012-03-27 09:20) - PLID 47448 - added ability to delete unused modifiers
	afx_msg void OnDeleteUnusedModifiers();
	// (j.gruber 2010-01-11 15:51) - PLID 36647 - added ability to configure referral source phone numbers
	afx_msg void OnConfigureReferralSourcePhoneNumbers();
	// (j.gruber 2010-03-09 09:51) - PLID 37660
	afx_msg void OnConfigureEmnChargesDiagCodesReport();
	// (j.gruber 2010-02-23 16:10) - PLID 37509
	afx_msg void OnConfigureClinicalSupportDecisionRules();
	// (j.gruber 2010-04-14 09:34) - PLID 37948
	afx_msg void OnConfigurePermissionGroups();
	// (j.gruber 2010-05-21 11:06) - PLID 38817
	afx_msg void OnConfigureBold();
	afx_msg void OnPatientEducationSetup();
	afx_msg void OnConfigureWoundCareCoding();
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	//afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	// (a.walling 2008-09-10 13:31) - PLID 31334
	// (a.walling 2009-06-22 10:48) - PLID 34635 - Dead/useless code
	//afx_msg LRESULT OnWIAEvent(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-12-22 10:45) - PLID 41911 - added financial close ability
	afx_msg void OnPerformFinancialClose();
	// (j.jones 2011-03-21 15:33) - PLID 42917 - added abilities to open shared folders on the server
	afx_msg void OnViewOldEOBFiles();
	afx_msg void OnViewOldEOBWarningsFiles();
	// (j.jones 2011-03-28 14:24) - PLID 43012 - added ability to mass-configure
	// the billable status of service codes
	afx_msg void OnEditNonbillableCPTCodes();
	afx_msg void OnMenuClickUnlinkHL7ThirdPartyID();
	// (d.singleton 2012-04-02 17:04) - PLID 49336
	afx_msg void OnMenuClickCodeCorrectSetup();
	// (j.luckoski 2012-04-10 17:18) - PLID 49491 - Open specialty configuration dialog.
	afx_msg void OnMenuClickSpecialtyDlg();
	// (j.dinatale 2012-08-13 15:36) - PLID 51941 - OMR Processing
	afx_msg void OnOMRProcessing();
	// (b.spivey, August 30, 2012) - PLID 51928 - event handler for menu option
	afx_msg	void OnRemarkOMRFormEditting();
	// (j.fouts 2012-12-31 15:11) - PLID 53156 - Added a a way to open Prescriptions Needing Attention
	afx_msg void OnPrescriptionsNeedingAttention();
	// (j.jones 2012-09-25 12:01) - PLID 37269 - added OnSize
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (d.singleton 2013-5-1 11:55) - PLID 56520 need settings for topaz sig pad
	afx_msg void OnTopazSigPadSettings(); 
	// (b.spivey, May 03, 2013) - PLID 56542 - event to prompt user if topaz sig pad is connected. 
	afx_msg LRESULT OnInitiateTopazSigPadDevice(WPARAM wParam, LPARAM lParam);
	//TES 1/13/2014 - PLID 59871 - Bring up the current patient's CDS interventions
	afx_msg void OnViewCdsInterventions();
	// (j.jones 2014-08-19 10:34) - PLID 63412 - event to process the next queued tablechecker
	afx_msg LRESULT OnPostNextQueuedTablechecker(WPARAM wParam, LPARAM lParam);
	//(s.dhole 12/4/2014 10:57 AM ) - PLID 64337
	afx_msg void OnMenuClickedMassAssignSecurityCodes();
	//TES 3/2/2015 - PLID 64736
	afx_msg void OnEnableDebugMode();
	// (d.lange 2015-07-22 14:54) - PLID 67188
	afx_msg void OnICCPSettings();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__F2B94DB1_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)