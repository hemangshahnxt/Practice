// EmrEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "EMR.h"
#include "EMRTopic.h"
#include "EmrEditorDlg.h"
#include "EmrItemAdvImageDlg.h"
#include "MultiSelectDlg.h"
#include "EmrAuditHistoryDlg.h"
#include "EMRSetupDlg.h"
#include "NxModalParentDlg.h"
#include "PicContainerDlg.h"
#include "EMRUnresolvedTodosDlg.h"
#include "internationalutils.h"
#include "DevicePlugin.h"			// (d.lange 2010-06-30 09:19) - PLID 38687 - Implementation for sending patient demographics to devices
#include "DevicePluginUtils.h"
#include "EmrRc.h"
#include "EmrMedAllergyViewerDlg.h"
#include "DontShowDlg.h"
#include "PatientGraphConfigDlg.h"
#include "FileUtils.h"	//(a.wilson 2011-10-17) PLID 45971 - needed for Eyemaginations integration.
#include "EMNMedication.h"
#include "DeviceLaunchUtils.h" // (j.gruber 2013-04-04 09:39) - PLID 56012
#include "WriteTokenInfo.h"
#include "EMN.h"
#include "GlobalFinancialUtils.h"

// (a.walling 2011-12-09 17:16) - PLID 46643 - Moving tools and etc into the CEmrFrameWnd

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor
// moved lots of common operations into CEmrEditorBase

// (a.walling 2012-07-11 09:39) - PLID 51476 - NXM_SET_SAVE_DOCS_IN_HISTORY, NXM_TREE_SEL_CHANGED is no more

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_EMR_CLOCK_TICK	10001
#define ID_EMR_CALC_OFFSET	10002
#define IDM_LINK_TO_DEVICE_IMPORT	10003		// (d.lange 2010-06-29 17:43) - PLID 39202 - Menu item for launching CDeviceImportDlg dialog

// (d.lange 2010-06-30 09:06) - PLID 38687 - Added struct for implementing 'Launch Device' menu items
struct DevicePlugin {
	CString strExePath;
	CString strOverrideName;
	CString strAdditionalPath;
	long nPlugin;
	CDevicePlugin *pdPlugin;
};

/////////////////////////////////////////////////////////////////////////////
// CEmrEditorDlg dialog


CEmrEditorDlg::CEmrEditorDlg()
{
	//{{AFX_DATA_INIT(CEmrEditorDlg)
		m_nEMRID = -1;
		m_nPicID = -1;
		m_nStartingEMNID = -1;
		m_nNewEmnTemplateID = -1;
		m_bIsLoadingExistingEMR = FALSE;
		// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
		m_nTotalSecondsOpen = 0;
		m_nCurrentSecondsOpen = 0;
		m_bCheckWarnedGlobalPeriod = FALSE;
	//}}AFX_DATA_INIT
}

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
CPicContainerDlg* const CEmrEditorDlg::GetPicContainer() const
{
	CPicContainerDlg* p = polymorphic_downcast<CPicContainerDlg*>(GetTopLevelFrame());
	ASSERT(p);
	return p;
}

BEGIN_MESSAGE_MAP(CEmrEditorDlg, CEmrEditorBase)
	//{{AFX_MSG_MAP(CEmrEditorDlg)
	ON_MESSAGE(NXM_EMN_ADDED, OnEmnAdded)
	ON_MESSAGE(NXM_EMN_CHANGED, OnEmnChanged)
	ON_MESSAGE(NXM_EMN_REMOVED, OnEmnRemoved)
	ON_WM_TIMER()
	ON_MESSAGE(NXM_NON_CLINICAL_PROCEDURES_ADDED, OnNonClinicalProceduresAdded)
	ON_MESSAGE(NXM_ADD_IMAGE_TO_EMR, OnAddImageToEMR)
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	ON_MESSAGE(NXM_ADD_GENERIC_TABLE_TO_EMR, OnAddGenericTableToEMR)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_DEVICE_INFO, &CEmrEditorDlg::OnBnClickedBtnDeviceInfo)
	ON_COMMAND(IDM_LINK_TO_DEVICE_IMPORT, OnBtnShowDeviceImport)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrEditorDlg message handlers
using namespace ADODB;

extern CPracticeApp theApp;

//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
void CEmrEditorDlg::SetEmr(long nEMRID, long nPicID, long nStartingEMNID /*= -1*/)
{
	m_nEMRID = nEMRID;
	m_nPicID = nPicID;
	m_nStartingEMNID = nStartingEMNID;
}

//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
void CEmrEditorDlg::SetEmrWithNewEmn(long nEMRID, long nPicID, long nTemplateID)
{
	m_nEMRID = nEMRID;
	m_nPicID = nPicID;
	m_nNewEmnTemplateID = nTemplateID;	
}

void CEmrEditorDlg::Initialize() 
{
	m_dtCurrentTimeOpened.SetStatus(COleDateTime::invalid);
	m_dtHistoricalTimeOpenedSpan.SetDateTimeSpan(0, 0, 0, 0);
	m_dtOffset.SetDateTimeSpan(0, 0, 0, 0);
	m_dtsPreviousTimePaused.SetDateTimeSpan(0, 0, 0, 0); // (c.haag 2010-01-14 11:37) - PLID 26036
	m_dtLastPaused_Local.SetStatus(COleDateTime::invalid); // (c.haag 2010-01-14 11:37) - PLID 26036

	try {
#pragma TODO("Don't forget eye button, etc")

		// (j.jones 2006-05-23 12:33) - PLID 20764 - Load all common EmrEditorDlg properties into the
		// NxPropManager cache
		g_propManager.CachePropertiesInBulk("EmrEditorDlg-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMRSavePref' OR "
			"Name = 'EmrAutoCollapse' OR "
			"Name = 'EMRTreeIconSize' OR "
			"Name = 'EMRSaveDocsInHistory' OR "
			"Name = 'EMRColorBottom' OR "
			// (j.jones 2007-08-30 09:29) - PLID 27243 - Office Visit incrementing is no longer
			// used in the L2 EMR, it's in Custom Records only
			//"Name = 'EnableEMROfficeVisitIncrementing' OR "
			"Name = 'EMRAutoGenDescription' OR "
			"Name = 'EMRAutoAlignToGrid' OR "
			"Name = 'OpenRoomMgrWhenOpenEMR' OR "
			"Name = 'OpenRoomMgrWhenOpenEMRMinimized' OR "
			"Name = 'OpenRoomMgrWhenCloseEMR' OR "
			"Name = 'OpenRoomMgrWhenCloseEMRMinimized' OR "
			// (a.walling 2007-05-08 16:32) - PLID 25725 - more splitter bar prefs to save
			"Name = 'CEmrTreeWndSplitterSnapToTree' OR "
			"Name = 'CEmrTreeWndSplitterMinimize' OR "
			// (a.walling 2007-06-19 17:18) - PLID 25548 - Autoscroll preference
			"Name = 'EMNPreviewAutoScroll' OR "
			// (j.jones 2007-07-06 12:10) - PLID 25457 - added EMRPreviewPaneLeftClick
			"Name = 'EMRPreviewPaneLeftClick' OR "
			// (a.walling 2007-07-16 16:52) - PLID 26640 - Cache preference for more info fields to show/hide in preview
			"Name = 'EMRPreviewMoreInfoFields' OR "
			// (a.walling 2007-12-17 13:21) - PLID 28354 - Cache preview options
			"Name = 'EMRPreview_HideDetails' OR "
			// (a.walling 2008-10-09 14:37) - PLID 31339 - Image width parameters			
			"Name = 'EMRPreview_ImageWidthThreshold' OR "
			"Name = 'EMRPreview_ImageWidthPercentage' OR "
			// (a.walling 2008-07-01 16:07) - PLID 30586 - Including location logo in preview
			"Name = 'EMRPreviewIncludeLocationLogo' OR " 
			// (a.walling 2009-08-03 10:51) - PLID 34542 - Toggle for detail dash prefix
			"Name = 'EMRPreviewPrefixDash' OR "
			// (a.walling 2010-03-24 20:19) - PLID 29293 - Option to override default font
			"Name = 'EMRPreviewOverrideDefaultNarrativeFont' OR "
			// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
			"Name = 'EMRPreviewEnableInteractiveNarrative' OR "
			//DRT 7/11/2007 - PLID 25939 - Stragglers that I happened upon 
			"Name = 'EMNTemplateActionBold' OR "
			"Name = 'LoadGeneral2ICD9ToEMR' OR "
			"Name = 'EmnDontMergeInklessImages' OR "
			// (j.jones 2007-07-17 13:32) - PLID 26712 - added more stragglers
			"Name = 'EMR_LinkSpawnedChargesDiagsByTopic' OR "
			"Name = 'EMR_Image_Use_Custom_Stamps' OR "
			"Name = 'DefaultEMRImagePenColor' OR "
			"Name = 'EmnPopupAutoDismiss' OR "			
			"Name = 'NxInkPicture_ShrinkToFitArea' OR " // (a.walling 2010-05-19 11:21) - PLID 38778 - Option to shrink to fit the display area
			"Name = 'EMR_AlignSliderValueBottom' OR "
			"Name = 'EMR_AlignSliderValueRight' OR " // (a.walling 2007-08-15 15:56) - PLID 27083 - Added the Right as well.
			"Name = 'EMNCompleteInitialSave' OR " // (a.walling 2007-08-31 15:09) - PLID 24733 - Save entire EMN first time
			"Name = 'EmnTableDropdownAutoAdvance' OR "	// (j.jones 2008-06-05 09:17) - PLID 18529
			"Name = 'EmrShowTogglePreviewButton' OR " //TES 8/19/2009 - PLID 35288
			"Name = 'CEmrTreeWndRestorePreview' OR " //TES 8/25/2009 - PLID 29348
			"Name = 'EMNUseLastApptDate' OR "	// (j.jones 2009-09-24 10:07) - PLID 29718
			"Name = 'WarnWhenCreatingNexWebEmn' OR "	//TES 11/13/2009 - PLID 35807
			"Name = 'EMRStopTimeTrackingWhenMinimized' OR "// (c.haag 2010-01-14 12:47) - PLID 26036
			"Name = 'EmrSpawnedTodoFormat' OR " // (c.haag 2010-02-02 11:34) - PLID 33925
			"Name = 'EMR_AddSmartStampOnHotSpot' OR "	// (j.jones 2010-02-18 10:10) - PLID 37423
			"Name = 'ReconcileNewRxWithCurMeds' OR " // (c.haag 2010-02-18 11:50) - PLID 37424
			"Name = 'ReconcileNewRxWithCurMeds_SkipEMRTable' OR " // (j.jones 2013-01-11 13:07) - PLID 54462
			"Name = 'EMNDefaultProviderType' OR "	// (j.jones 2010-04-06 09:07) - PLID 37158
			"Name = 'EMNDefaultProviderType_ApptFallback' OR "			// (d.thompson 2013-08-15) - PLID 57809
			"Name = 'EMNDefaultProviderType_CreatingUserFallback' OR "	// (d.thompson 2013-08-15) - PLID 57809
			"Name = 'EmrMedAllergyViewerVisible' OR " // (c.haag 2010-08-03 09:24) - PLID 39936
			// (c.haag 2010-08-03 09:45) - PLID 38928 - Next four entries are for the Emr medication/allergy/rx viewer
			"Name = 'EmrMedAllergyViewerWidth' OR "
			"Name = 'EmrMedAllergyViewerHeight' OR "
			"Name = 'EmrMedAllergyViewerTop' OR "
			"Name = 'EmrMedAllergyViewerLeft' OR "
			"Name = 'EMNChargesAllowQtyIncrement' OR " // (j.jones 2010-09-22 08:12) - PLID 24221
			"Name = 'ChargeAllowQtyIncrement' OR "
			// (c.haag 2010-12-13 10:04) - PLID 41817 - Added EmrMedAllergyViewerColumns
			"Name = 'EmrMedAllergyViewerColumns' "
			"OR Name = 'EmrTableAutofillBehavior' " // (z.manning 2011-03-22 14:57) - PLID 42954
			"OR Name = 'EmrTechnicianAutofillBehavior' "	// (d.lange 2011-03-25 16:42) - PLID 42987 - Added EmrTechnicianAutofillBehavior
			// (j.jones 2011-04-07 09:29) - PLID 43166 - added EMR_SkipLinkingDiagsToNonBillableCPTs, and a couple other missing prefs.
			"OR Name = 'EMR_SkipLinkingDiagsToNonBillableCPTs' "
			"OR Name = 'EMR_AutoLinkSpawnedDiagCodesToCharges' "
			"OR Name = 'EMR_SpawnedDiagCodes_NoLinkPrompt' "
			// (d.thompson 2011-05-10) - PLID 43123 - Bulk cache property for emr item dialog
			"OR Name = 'EMRItemColor' "
			// (r.gonet 05/27/2011) - PLID 43542 - Bulk cache enable text scaling property
			"OR Name = 'EnableEMRImageTextScaling' "
			// (j.jones 2011-08-22 13:52) - PLID 44977 - added DeviceImport_EMRButtonOnlyImport
			"OR Name = 'DeviceImport_EMRButtonOnlyImport' "
			"OR Name = 'SignatureCheckPasswordEMR' "
			"OR Name = 'EmrPostSignatureInsertStatus' " // (z.manning 2011-09-02 16:01) - PLID 32123
			"OR Name = 'Nx3DBackgroundColor' " // (j.armen 2011-09-15 14:48) - PLID 45424
			"OR Name = 'EMRPreview_UseOriginalFileSize' " //TES 1/25/2012 - PLID 47505
			"OR Name = 'EMN_HideUnbillableCPTCodes' " // (j.jones 2012-02-20 10:25) - PLID 47886
			// (j.jones 2012-03-27 14:59) - PLID 44763 - added global period warning
			"OR Name = 'CheckWarnGlobalPeriod_EMR' "
			"OR Name = 'GlobalPeriodSort' "
			"OR Name = 'EmrMUProgressTheme' " //(e.lally 2012-04-24) PLID 48016
			"OR Name = 'GlobalPeriod_OnlySurgicalCodes' "	// (j.jones 2012-07-24 10:44) - PLID 51737
			"OR Name = 'GlobalPeriod_IgnoreModifier78' "	// (j.jones 2012-07-26 14:26) - PLID 51827
			"OR Name = 'EMRLoadEMNsSortOrder' "	// (j.jones 2012-07-31 17:06) - PLID 51750
			// (j.jones 2012-08-22 08:44) - PLID 50486 - added EMNChargeRespDefault, also cached some absent prefs.
			"OR Name = 'EMNChargeRespDefault' "
			"OR Name = 'BillEMNTo' "
			"OR Name = 'RequireEMRChargeResps' "
			"OR Name = 'DisplayChargeSplitDlgAlways' "
			"OR Name = 'EMRDrugInteractionChecks' "		// (j.jones 2012-09-27 09:27) - PLID 52820
			"OR Name = 'EMRRememberPoppedUpTableColumnWidths' " // (r.gonet 02/14/2013) - PLID 40017
			"OR Name = 'HideEMNDefaultSignatureTitle' " // (b.spivey, March 06, 2013) - PLID 39697 
			"OR Name = 'DisableSmartStampTableDropdownStampFilters' "	// (j.jones 2013-03-07 12:41) - PLID 55511
			"OR Name = 'dontshow EMRCurrentMedsSyncingDisabled'	" // (j.jones 2013-03-26 13:45) - PLID 55139
			"OR Name = 'EMNDefaultSecondaryProviderType' " // (j.luckoski 2013-04-02 13:20) - PLID 55910 - Cache my new pref
			// (j.jones 2013-05-08 13:27) - PLID 44634 - added more options for linking diags/charges, also cached two that were previously missed
			"OR Name = 'EMR_SpawnedDiagCodes_AutoLinkPrompt' "
			"OR Name = 'EMR_SpawnedCharges_AutoLinkPrompt' "
			"OR Name = 'EMR_LinkSpawnedChargesDiagsByTopic_Prompt' "
			"OR Name = 'EMR_AutoLinkSpawnedChargesToDiagCodes' "
			"OR Name = 'EMR_SpawnedCharges_NoLinkPrompt' "
			// (a.wilson 2013-05-21 15:07) - PLID 56763 - cache send to hl7 property.
			"OR Name = 'SendBillingToHL7FromNewEMN' "
			// (j.jones 2013-06-05 14:40) - PLID 57049 - this wasn't cached, so I added it
			"OR Name = 'EMRPreview_MedAllergy' "
			"OR Name = 'EMNAppointmentLinking' " // (j.jones 2013-06-13 11:02) - PLID 57129
			"OR Name = 'EMNCopy_SkipSignatures' "	// (j.jones 2013-06-18 13:25) - PLID 47217
			"OR Name = 'MatchLabReqSpawning' "		// (d.thompson 2013-07-18) - PLID 40000
			"OR Name = 'RequireCPTCodeEMNLocking' " // (d.singleton 2013-07-24 16:40) - PLID 44840
			"OR Name = 'RequireDiagCodeEMNLocking' " // (d.singleton 2013-07-24 16:40) - PLID 44840
			"OR Name = 'EMRLastOtherUserSignatureID' " // (j.jones 2013-08-08 10:38) - PLID 42958
			"OR Name = 'AutoPromptForDefaultSignature' "	// (j.jones 2013-08-08 13:59) - PLID 42958 - this wasn't cached, so I added it
			"OR Name = 'EMRPreview_HideEmnTitle' " 
			"OR Name = 'EMR_WarnWhenUnmatchedDiagnosisCodes' " // (b.savon 2014-03-07 11:30) - PLID 60826 - Cache the warning
			"OR Name = 'CreateTodoWhenDiagCodeSpawnedInDiffDiagMode' " // (b.savon 2014-08-18 12:08) - PLID 62712 - Cache the pref
			"OR Name = 'DuplicateSpawnedEmrProblemBehavior' " // (z.manning 2016-04-08) - NX-100090
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2007-05-08 16:36) - PLID 25464 - CEmrTreeWndSplitterTopicRightRelative
		// changed from an int to a float, and while no other EMR preferences are floats yet,
		// we decided to start a new batch anyways so people would remember to use it
		g_propManager.CachePropertiesInBulk("EmrEditorDlg-2", propFloat,
			"(Username = '<None>' OR Username = '%s') AND ("			
			// (j.jones 2007-04-04 09:57) - PLID 25464 - stored splitter bar location
			"Name = 'CEmrTreeWndSplitterTopicRightRelative' "
			")",
			_Q(GetCurrentUserName()));

		// (b.savon 2014-08-18 12:09) - PLID 62712 - Cache the memo
		g_propManager.CachePropertiesInBulk("EmrEditorDlg-3", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'TodoUsersWhenDiagCodeSpawnedInDiffDiagMode' "
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2006-02-02 15:40) - PLID 19123 - the loading process won't begin until OnShowWindow

		// (c.haag 2006-03-28 12:37) - PLID 19890 - Disable certain controls if the user is not allowed to edit them
		// (a.walling 2007-11-28 11:18) - PLID 28044 - Also check for expired license
		//BOOL bCanWriteToEMR = CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
	
		// (j.jones 2007-02-06 15:15) - PLID 24493 - used for the clock
		CalculateLocalTimeOffset();

		// (j.jones 2012-03-27 14:59) - PLID 44763 - tracks if we have warned about a global period in this EMR session
		m_bCheckWarnedGlobalPeriod = FALSE;

#pragma TODO("eye button path checks?")

		//Load the global information.
		//Here we will load the Patient name, and the EMR Description and Status.  We will also put default values for the patient
		//age and gender on the screen; those fields, as well as the procedures, and anything else we pull from the EMN level,
		//will be set when we get the NXM_EMN_ADDED message.

		// (a.walling 2012-05-18 17:15) - PLID 50546 - No longer necessary

		//_RecordsetPtr rsEmr;
		//if(m_nEMRID == -1) {
		//	//DRT 8/28/2007 - PLID 27207 - Parameterized.
		//	rsEmr = CreateParamRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
		//		"'' AS Description, 0 AS Status "
		//		"FROM PersonT WHERE ID = {INT}", GetPatientID());
		//	if(rsEmr->eof) {
		//		//Yikes!
		//		AfxThrowNxException("No active patient found when creating an EMR");
		//	}
		//}
		//else {
		//	//DRT 8/28/2007 - PLID 27207 - Parameterized.
		//	rsEmr = CreateParamRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
		//		"EmrGroupsT.Description, EmrGroupsT.Status "
		//		"FROM EmrGroupsT INNER JOIN PersonT ON EmrGroupsT.PatientID = PersonT.ID "
		//		"WHERE EmrGroupsT.ID = {INT}", m_nEMRID);
		//	if(rsEmr->eof) {
		//		//Yikes!
		//		AfxThrowNxException("Attempted to load invalid EMR ID %li", m_nEMRID);
		//	}
		//}

		//m_strPatientName = AdoFldString(rsEmr, "Name");
		//CString strDescription = AdoFldString(rsEmr, "Description", "");

		// (j.jones 2010-04-01 15:58) - PLID 34915 - this tries to update the dialog text,
		// but dialog has no menu bar, it's part of the PIC!
		//CString strWindowCaption = "EMR for " + m_strPatientName;
		//SetWindowText(strWindowCaption);

		// (j.jones 2006-12-19 08:55) - PLID 22794 - check the preference to open the room manager
		// (a.walling 2010-04-13 13:22) - PLID 37150 - Now this occurs after the EMR is entirely loaded
		/*
		if(GetRemotePropertyInt("OpenRoomMgrWhenOpenEMR", 0, 0, GetCurrentUserName(), true) == 1) {
			GetMainFrame()->PostMessage(NXM_OPEN_ROOM_MANAGER, (long)-1,
				(BOOL)GetRemotePropertyInt("OpenRoomMgrWhenOpenEMRMinimized", 0, 0, GetCurrentUserName(), true) == 1);
		}
		*/

	} NxCatchAll(__FUNCTION__);
}

void CEmrEditorDlg::LoadEMRObject()
{
	CWaitCursor pWait;

	try {

		//DRT 7/27/2007 - PLID 26836 - To properly gauge which loading method is necessary for the preloader, 
		//	we need to know BEFORE we start loading which EMN is going to be "selected".  This is the EMN that
		//	will be expanded in the tree, and thus it needs "special" loading behavior (its first topic is loaded
		//	synchronously, then all the others are loaded in the background).  But for all the EMNs that are not
		//	going to be selected, they can be entirely loaded in the background from the start, if we only know
		//	ahead of time which will be selected.
		//A value of -1 means that none of the loaded values will be selected, and that they can all be loaded 
		//	in the background.  Anytime we are adding an EMN from a template ID, that template will become active.
		long nWillBeActiveEMNID = -1;
		if(m_nNewEmnTemplateID != -1) {
			//No existing saved EMN will be displayed
			nWillBeActiveEMNID = -1;
		}
		else {
			//There is no new template, we must just be loading an existing EMR, or we're creating a new EMN with a -1 template (blank).
			//	In either case we follow the preset member variable.
			if(m_nStartingEMNID != -1) {
				nWillBeActiveEMNID = m_nStartingEMNID;
			}
			else {
				//We were not given a "starting" EMN for this EMR, but we are not loading a template.  At this time it's impossible to know
				//	which EMN should be loaded, so we will let the EMR loading code choose.  It will pass back the result into this variable.
				nWillBeActiveEMNID = -2;
			}

			// (j.jones 2008-06-04 10:51) - PLID 28073 - tracks when we are loading the EMR,
			// *not* creating a new EMR from a template
			m_bIsLoadingExistingEMR = TRUE;
		}


		//patient EMR
		// (c.haag 2007-03-08 09:39) - PLID 25110 - Make sure the tree is
		// working with the right patient ID
		//TES 11/22/2010 - PLID 41582 - Pass in PIC ID so that new EMRs will save to the correct PIC
		m_wndEmrTree.SetPatientEMR(m_nEMRID, m_nPicID, GetPatientID(), nWillBeActiveEMNID);

		//At this point, nWillBeActiveEMNID is the ID that was set to load as the active EMN.  Use it in all future choices

		// (a.walling 2007-04-11 12:21) - PLID 25548 
		if(m_nStartingEMNID != -1) {
			m_wndEmrTree.SetActiveEMN(nWillBeActiveEMNID);
		}

		//add a new EMN from a template, if instructed to do so
		SourceActionInfo saiBlank; // (z.manning 2009-03-04 15:43) - PLID 33338
		if(m_nNewEmnTemplateID != -1) {
			// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
			CEMN* pEMN = m_wndEmrTree.AddEMNFromTemplateWithAppointmentPrompt(m_nNewEmnTemplateID, saiBlank);
			if(pEMN) {
				m_wndEmrTree.ShowEMN(pEMN);
			}
		}
		else if(m_nStartingEMNID != -1) {
			// (z.manning 2011-05-20 17:06) - PLID 33114 - It's possible that the EMN we're trying to show never
			// actually got loaded due to chart permissions so let's check and see if it exists first.
			if(GetEMR() != NULL && GetEMR()->GetEMNByID(nWillBeActiveEMNID) != NULL) {
				m_wndEmrTree.ShowEMN(nWillBeActiveEMNID);
			}
		}
		else {
			//DRT 2/8/2006 - PLID 19178 - We are not making a new EMN from a template, 
			//	and we are not starting with an existing EMN.  So therefore, we must 
			//	just be told to open a certain EMR.  We'll want to pick the first EMN 
			//	and start them off there.
			//The act of calling SetPatientEMR() will have loaded all of the EMN info into 
			//	m_arEmnInfo.  We can use that to just grab the first ID.  Note that if
			//	for some reason there are no EMNs for an EMR (Is that possible?), then we'll
			//	select nothing here, and the editor will look a little odd.

			//(e.lally 2006-03-06) PLID 19510 - Adding a generic EMN template needs to actually
			//add the empty EMN and More Info topic to a new EMR, otherwise you do have a case where there is an
			//EMR with no EMNs

			//Check if there are any other EMNs.
			if(m_arEmnInfo.GetSize() == 0){
				// (j.dinatale 2012-07-12 13:44) - PLID 51481 - need to look at if we intended to open the pic or not
				bool bLoadedForPic = false;
				if(GetPicContainer()){
					bLoadedForPic = GetPicContainer()->OpenedForPic();
				}

				// (a.walling 2007-11-28 12:47) - PLID 28044 - Don't add a new one if license expired
				if (!bLoadedForPic && g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
					// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
					m_wndEmrTree.AddEMNFromTemplateWithAppointmentPrompt(m_nNewEmnTemplateID, saiBlank);
					//TES 5/22/2008 - PLID 29994 - Since we didn't have any EMNs, we'll want to start by showing this new EMN
					// (which we know has an ID of -1, since it was just created).
					nWillBeActiveEMNID = -1;
				}
			}
			//Re-check the size of the array and show the first EMN.
			if(m_arEmnInfo.GetSize() > 0) {
				m_wndEmrTree.ShowEMN(nWillBeActiveEMNID);
			}
		}

		// (j.jones 2008-06-04 10:54) - PLID 28073 - don't generate the description when loading an existing EMR
		if(!m_bIsLoadingExistingEMR) {
			TryGenerateAndApplyEMRDescription();
		}

		// (j.jones 2007-02-06 15:12) - PLID 24509 - start tracking the EMR time
		m_wndEmrTree.TryStartTrackingEMRTime();

		// (j.jones 2007-01-31 13:38) - PLID 24493 - start tracking time for the clock
		m_dtCurrentTimeOpened = m_wndEmrTree.GetEMRStartEditingTime();
		if(m_dtCurrentTimeOpened.GetStatus() == COleDateTime::invalid) {
			//should be impossible
			ASSERT(FALSE);
			m_dtCurrentTimeOpened = COleDateTime::GetCurrentTime() + m_dtOffset;
		}

		//calculate the historical time open for this EMR, if one exists
		if(m_nEMRID != -1) {
			//DRT 8/28/2007 - PLID 27207 - Parameterized.
			// (j.jones 2013-02-27 15:47) - PLID 55351 - Ignore all time slips with a null end time, which means they are uncommitted.
			_RecordsetPtr rs = CreateParamRecordset("SELECT Coalesce(Sum(DATEDIFF(second, StartTime, EndTime)),0) AS TotalSeconds "
				"FROM EMRGroupsSlipT WHERE EMRGroupID = {INT} AND EndTime Is Not Null", m_nEMRID);
			if(!rs->eof) {
				//grab the total seconds
				long nTotalSeconds = AdoFldLong(rs, "TotalSeconds",0);
				m_dtHistoricalTimeOpenedSpan.SetDateTimeSpan(0,0,0,nTotalSeconds);
			}
			rs->Close();
		}

		//now update the screen and start the timer event
		// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
		m_nTotalSecondsOpen = static_cast<int>(m_dtHistoricalTimeOpenedSpan.GetTotalSeconds());
		m_nCurrentSecondsOpen = 0;
		/*
		CString strTime;
		strTime.Format("%li:%02li:%02li", m_dtHistoricalTimeOpenedSpan.GetHours(), m_dtHistoricalTimeOpenedSpan.GetMinutes(), m_dtHistoricalTimeOpenedSpan.GetSeconds());
		SetDlgItemText(IDC_EDIT_TOTAL_TIME_OPEN, strTime);
		SetDlgItemText(IDC_EDIT_CURRENT_TIME_OPEN, "0:00:00");
		*/
		SetTimer(ID_EMR_CLOCK_TICK,0,NULL);

	}NxCatchAll("Error in CEmrEditorDlg::LoadEMRObject()");

	// (j.jones 2008-06-04 10:52) - PLID 28073 - track that we are done loading
	m_bIsLoadingExistingEMR = FALSE;

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Now that everything's finished loading, and we know we're 
	// on screen, give the user a warning if this EMR is a real EMR (not a new one or a template) and it 
	// contains EMNs with any reconstructed details.
	try {
		if (m_nNewEmnTemplateID == -1) {
			CString strEMNIDs;
			for (long i=0, nCount=m_arEmnInfo.GetSize(); i<nCount; i++) {
				CString str;
				str.Format("%li,", m_arEmnInfo[i].pEMN->GetID());
				strEMNIDs += str;
			}
			if (!strEMNIDs.IsEmpty()) {
				// Drop the extra comma from the end
				strEMNIDs.Delete(strEMNIDs.GetLength() - 1, 1);
				// Now get the count of details on any of these EMNs that contain details in ReconstructedEMRDetailsT
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT COUNT(*) AS CountToWarn "
					"FROM EMRDetailsT INNER JOIN ReconstructedEMRDetailsT ON EMRDetailsT.ID = ReconstructedEMRDetailsT.EMRDetailID "
					"WHERE ReconstructedEMRDetailsT.ReviewStatus IN (-1) AND EMRDetailsT.EMRID IN ({INTSTRING})", strEMNIDs);
				long nCountToWarn = AdoFldLong(prs->GetFields()->GetItem("CountToWarn"));
				if (nCountToWarn > 0) {
					CString strWarning;
					strWarning.Format(
						"This EMR contains %li locked detail%s (colored light pink) that %s reconstructed by NexTech and %s not yet "
						"been verified by you.  The reconstruction was based on one or more of the following sources: \r\n"
						"    - Backed up data \r\n"
						"    - This patient's merged EMR Word documents \r\n"
						"    - This patient's scanned documents \r\n"
						"    - Audit trail \r\n"
						"\r\n"
						"Each reconstructed detail will be displayed with a light pink background until you review it.  Once you "
						"have verified the correctness of a detail, please right-click on it and choose 'Reconstruction Verified' to "
						"return it to its normal color or 'Reconstruction Verified Leave Highlighted' if you want that detail to have "
						"a white background for future reference.  Once you have verified all reconstructed details on this EMR, you "
						"will no longer receive this warning.  Only authorized users are permitted to mark reconstructed details as "
						"verified. \r\n"
						"\r\n"
						// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
						"If you have ANY questions please contact NexTech Technical Support at 888-417-8464 or allsupport@nextech.com.",
						nCountToWarn, nCountToWarn==1?"":"s", nCountToWarn==1?"was":"were", nCountToWarn==1?"has":"have");
					MessageBox(strWarning, "Reconstructed Data", MB_ICONINFORMATION|MB_OK);
				}
			}
		}
	} NxCatchAll("CEmrEditorDlg::LoadEMRObject:ReconstructedDataWarning");
}
			
LRESULT CEmrEditorDlg::OnEmnAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN *pEmn = (CEMN*)wParam;
		LocalEmn le;
		le.pEMN = pEmn;
		le.strPatientAge = pEmn->GetPatientAge();
		le.nPatientGender = pEmn->GetPatientGender();
		//DRT 7/13/2007 - PLID 26671 - Get the whole procedure structure instead of just an ID
		pEmn->GetProcedures(le.arProcedures);
		m_arEmnInfo.Add(le);

		// (j.jones 2010-04-01 15:58) - PLID 34915 - this tries to update the dialog text,
		// but dialog has no menu bar, it's part of the PIC!
		//DisplayEmnInfo();

		// (j.jones 2008-06-04 10:53) - PLID 28073 - don't regenerate if we are loading an existing EMR
		if(!m_bIsLoadingExistingEMR) {
			TryGenerateAndApplyEMRDescription();

			// (j.jones 2012-10-01 12:18) - PLID 52922 - if the new EMN contains medications, we may
			// need to save the EMN and warn about drug interactions, but not if the EMN is still loading
			// (j.jones 2012-10-01 15:44) - PLID 52869 - also trigger an interaction check if the EMN
			// contains diagnosis codes
			CEmrTreeWnd *pTree = GetEmrTreeWnd();
			if(!pEmn->IsTemplate() && pTree != NULL && !pEmn->IsLoading() && (pEmn->GetMedicationCount() > 0 || pEmn->GetDiagCodeCount() > 0)) {

				// (j.jones 2013-01-09 11:55) - PLID 54530 - open medication reconciliation
				if(pEmn->GetMedicationCount() > 0 && pEmn->IsWritable()) {

					if(SUCCEEDED(pTree->SaveEMR(esotEMN, (long)pEmn, FALSE))) {

						//this is a new EMN, so all prescriptions on this EMN would have been newly created
						CArray<long, long> aryNewPrescriptionIDs;
						for(int m=0; m<pEmn->GetMedicationCount(); m++) {
							EMNMedication *pMed = pEmn->GetMedicationPtr(m);
							aryNewPrescriptionIDs.Add(pMed->nID);
						}
						pTree->ReconcileCurrentMedicationsWithNewPrescriptions(pEmn, aryNewPrescriptionIDs);

						//if reconciliation changed the current meds table, it would have forced a save
					}
				}

				// This function will check their preference to save the EMN immediately,
				// potentially do so, and warn about drug interactions.
				// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
				pEmn->CheckSaveEMNForDrugInteractions(FALSE);
			}
		}

	} NxCatchAll("Error in OnEmnAdded");

	return 0;
}

LRESULT CEmrEditorDlg::OnEmnChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		//Construct a LocalEmn.
		CEMN *pEmn = (CEMN*)wParam;
		LocalEmn le;
		le.pEMN = pEmn;
		le.strPatientAge = pEmn->GetPatientAge();
		le.nPatientGender = pEmn->GetPatientGender();
		//DRT 7/13/2007 - PLID 26671 - Get the whole procedure structure instead of just an ID
		pEmn->GetProcedures(le.arProcedures);

		//Now, find the corresponding item in our array.
		for(int i = 0; i < m_arEmnInfo.GetSize(); i++) {
			if(m_arEmnInfo[i].pEMN == le.pEMN) {
				//Found it.  Do they 100% match?
				//First, check the procedures.
				bool bMismatchFound = false;
				for(int j = 0; j < m_arEmnInfo[i].arProcedures.GetSize() && !bMismatchFound; j++) {
					bool bFound = false;
					for(int k = 0; k < le.arProcedures.GetSize() && !bFound; k++) {
						if(m_arEmnInfo[i].arProcedures[j].nID == le.arProcedures[k].nID)
							bFound = true;
					}
					if(bFound)
						bMismatchFound = true;
				}
				if(!bMismatchFound) {
					//Do the other fields we care about match as well?
					if(m_arEmnInfo[i].strPatientAge == le.strPatientAge && m_arEmnInfo[i].nPatientGender == le.nPatientGender 
						&& m_arEmnInfo[i].arProcedures.GetSize() == le.arProcedures.GetSize()) {
						//This is a 100% match, we don't need to do anything.
						return 0;
					}			
				}

				if(m_arEmnInfo[i].arProcedures.GetSize() != le.arProcedures.GetSize())
					bMismatchFound = TRUE;

				//If we got here, they don't match, so update our array.
				m_arEmnInfo.SetAt(i, le);
				//If the procedures didn't match, update our description.
				// (j.jones 2008-06-04 10:53) - PLID 28073 - don't regenerate if we are loading an existing EMR
				if(bMismatchFound && !m_bIsLoadingExistingEMR) {
					TryGenerateAndApplyEMRDescription();
				}
				//And update the screen.
				// (j.jones 2010-04-01 15:58) - PLID 34915 - this tries to update the dialog text,
				// but dialog has no menu bar, it's part of the PIC!
				//DisplayEmnInfo();
				return 0;
			}
		}

		//Odd, we didn't find it.  Well, we'll just add it.
		m_arEmnInfo.Add(le);
		
		// (j.jones 2010-04-01 15:58) - PLID 34915 - this tries to update the dialog text,
		// but dialog has no menu bar, it's part of the PIC!
		//DisplayEmnInfo();

	} NxCatchAll("Error in OnEmnChanged");
	return 0;
}

LRESULT CEmrEditorDlg::OnEmnRemoved(WPARAM wParam, LPARAM lParam)
{
	try {
		CEMN *pEMN = (CEMN*)wParam;
		bool bRemoved = false;
		for(int i = 0; i < m_arEmnInfo.GetSize(); i++) {
			if(m_arEmnInfo[i].pEMN == pEMN) {
				bRemoved = true;
				m_arEmnInfo.RemoveAt(i);
			}
		}

		if(bRemoved) {
			// (j.jones 2010-04-01 15:58) - PLID 34915 - this tries to update the dialog text,
			// but dialog has no menu bar, it's part of the PIC!
			//DisplayEmnInfo();
		}
	} NxCatchAll("Error in OnEmnRemoved");
	return 0;
}

LRESULT CEmrEditorDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
		case NXM_EMR_SAVE_CREATE_BILL:
		case NXM_EMR_SAVE_CREATE_QUOTE: 
		case NXM_EMR_SAVE_CREATE_PARTIAL_BILLS:	// (j.dinatale 2012-01-18 17:55) - PLID 47620 - Support partial billing
			{

			//we need to save the EMR, determine the EMN ID to bill/quote, and close the PIC

			//tell our parent to close and bill/quote this EMN
			if(GetPicContainer()) {
				return GetPicContainer()->SendMessage(message, wParam, lParam);
			}
			break;
		}

		case NXM_EMR_PRINT_PRESCRIPTIONS_AND_CLOSE: {

			//tell our parent to close and print the prescriptions for this EMN
			if(GetPicContainer()) {
				//TES 6/17/2008 - PLID 30411 - lParam is now meaningful.
				return GetPicContainer()->SendMessage(NXM_EMR_PRINT_PRESCRIPTIONS_AND_CLOSE, wParam, lParam);
			}
			break;
		}

		case NXM_EMR_PRINT_PRESCRIPTIONS: {
			//TES 6/17/2008 - PLID 30414 - Tell our parent to print the prescriptions for this EMN (but don't close)
			if(GetPicContainer()) {
				return GetPicContainer()->SendMessage(NXM_EMR_PRINT_PRESCRIPTIONS, wParam, lParam);
			}
			break;
										  }

		case NXM_PRE_DELETE_EMN_PROCEDURE: {			

			// (z.manning, 10/05/2007) - PLID 27630 - Moved all the logic for updating the EMR description to
			// the NXM_POST_DELETE_EMN_PROCEDURE message because we need to wait until that procedure has
			// actually been removed or else it may still incorrectly be included in the description.

			if(GetPicContainer()) {
				return GetPicContainer()->SendMessage(NXM_PRE_DELETE_EMN_PROCEDURE, wParam, lParam);
			}
			break;
		}

		// (z.manning, 10/05/2007) - PLID 27630 - Now also have a message for after we've removed the procedure.
		case NXM_POST_DELETE_EMN_PROCEDURE: {

			//
			// (c.haag 2007-03-12 13:25) - PLID 25157 - This body of code is necessary for
			// auto-updating the EMR description when a user deletes a procedure off a patient EMN.
			// The standard auto-updating in CEmrEditorDlg::TryGenerateAndApplyEMRDescription fails
			// to do this because it does not account for deleted procedures, and EMNs only track
			// deleted procedures if those EMNs are not new and the procedure itself is not new
			//
			if(GetRemotePropertyInt("EMRAutoGenDescription", 1, 0, GetCurrentUserName(), true) != 0) {
				CEMN* pEMN = (CEMN*)lParam;
				// This should never happen for templates or locked EMN's, but we will check for
				// completeness
				if (pEMN && !pEMN->IsTemplate() && pEMN->GetStatus() != 2) {
					long nProcedureID = (long)wParam;
					CEMR* pEMR = pEMN->GetParentEMR();
					//DRT 7/13/2007 - PLID 26671 - Get all the procedure info, not just the IDs
					CArray<EMNProcedure, EMNProcedure> aryProcedures;
					BOOL bProcedureStillInEMR = FALSE;
					int i;

					// Get the procedure list for the whole EMR. This list DOES account for the fact
					// that nProcedureID was removed from one EMN.
					pEMR->GetProcedures(aryProcedures);

					// Check if the procedure ID is still in the EMR's procedure list (which can happen
					// if it exists on an EMN other than pEMN). If it is, then there is no need to change
					// the description box of this dialog. If it is not, then then we may need to update
					// the description
					for (i=0; i < aryProcedures.GetSize() && !bProcedureStillInEMR; i++) {
						if (aryProcedures[i].nID == nProcedureID) {
							bProcedureStillInEMR = TRUE;
						}
					}
					
					// If this is true, then we may need to update the description. To know for sure,
					// we must compare the current description with the calculated description
					if (!bProcedureStillInEMR) {
						try {
							// Begin by calculating what the current description is. The calculation
							// uses m_arEmnInfo, which has NOT been updated to reflect the fact that
							// the procedure was removed yet
							CString strProcedureNames = GenerateEMRDescription();

							// Now compare the calculated description with the current description
							// (a.walling 2012-05-18 17:18) - PLID 50546 - Get the description from EMR
							CString strCurDesc = pEMR->GetDescription();
							
							if (strCurDesc == strProcedureNames) {

								// We have a perfect match! Unfortunately, we have to calculate the description
								// as it will be after the procedure is removed. We can't just remove the
								// procedure name itself because it could be a substring in another procedure
								// name
								strProcedureNames.Empty();
								if (aryProcedures.GetSize() > 0) {
									// Now build the new description string
									//DRT 7/13/2007 - PLID 26671 - We have all the info at hand, no need to query
									for(i = 0; i < aryProcedures.GetSize(); i++) {
										if(!strProcedureNames.IsEmpty()) {
											strProcedureNames += ", ";
										}
										strProcedureNames += aryProcedures[i].strName;
									}
								}
								if(strProcedureNames.IsEmpty())
									strProcedureNames = "<No Procedure Determined>";

								// Assign the description string to the tree and the dialog
								// (a.walling 2012-05-18 17:18) - PLID 50546 - Set the description
								pEMR->SetDescription(strProcedureNames);
							}
						}
						NxCatchAll("Error in CEmrEditorDlg::WindowProc:NXM_PRE_DELETE_EMN_PROCEDURE");
					} // if (!bProcedureStillInEMR)

				}  // if (pEMN && !pEMN->IsTemplate() && pEMN->GetStatus() != 2)

			} // if(GetRemotePropertyInt("EMRAutoGenDescription", 1, 0, GetCurrentUserName(), true) != 0)
			break;
		}

		case NXM_EMN_PROCEDURE_ADDED: {

			if(GetPicContainer()) {
				return GetPicContainer()->SendMessage(NXM_EMN_PROCEDURE_ADDED, wParam, lParam);
			}
			break;
		}

		case NXM_EMN_MULTI_PROCEDURES_ADDED:
		{
			try {
				//DRT 9/6/2007 - PLID 27310 - Just pass it through
				if(GetPicContainer()) {
					return GetPicContainer()->SendMessage(NXM_EMN_MULTI_PROCEDURES_ADDED, wParam, lParam);
				}
			} NxCatchAll("Error in WindowProc : MultiProceduresAdded");
		}
		break;
	}

	return CEmrEditorBase::WindowProc(message, wParam, lParam);
}

void CEmrEditorDlg::TryGenerateAndApplyEMRDescription()
{
	//check the preference to see if they really want to do this
	if(GetRemotePropertyInt("EMRAutoGenDescription", 1, 0, GetCurrentUserName(), true) == 0) {
		return;
	}

	CEMR* pEMR = GetEMR();
	if (!pEMR) {
		ASSERT(FALSE);
		return;
	}

	// (a.walling 2012-05-18 17:18) - PLID 50546 - Get the description from EMR
	CString strCurDescription = pEMR->GetDescription();
	CString strNewDescription;

	strCurDescription.TrimLeft();
	strCurDescription.TrimRight();

	if(strCurDescription.IsEmpty() || strCurDescription == "<No Procedure Determined>") {
		//if there is no description, then generate from the EMN procedures
		strNewDescription = GenerateEMRDescription();
	}
	else {
		//if there is a description, then determine if it is a generated one, and if we need to re-generate it

		//clear out any generated text from our end

		//"no procedure"
		strCurDescription.Replace("<No Procedure Determined>","");

		//all procedure names
		//DRT 7/13/2007 - PLID 26671 - Reworked to better handle the fact that we have all procedure info, not
		//	just the name
		CArray<EMNProcedure, EMNProcedure> arAllProcedures;
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		int i = 0;

		for(i = 0; i < m_arEmnInfo.GetSize(); i++) {
			for(int j = 0; j < m_arEmnInfo[i].arProcedures.GetSize(); j++) {
				//Is it already in our list?
				bool bFound = false;
				for(int k = 0; k < arAllProcedures.GetSize() && !bFound; k++) {
					//Use ID to compare, we are trying to avoid duplicates
					if(arAllProcedures[k].nID == m_arEmnInfo[i].arProcedures[j].nID) bFound = true;
				}
				if(!bFound) {
					// (z.manning 2009-09-22 11:49) - PLID 35618 - To improve the overall process of generating
					// the EMR description, let's sort the local list of procedures in descending order by name.
					// That will prevent problems later when replacing each procedure nameif we have, for example,
					// both "Abdominoplasty" and "Abdominoplasty - Mini" as procedures in this EMR.
					bool bAdded = false;
					for(int nProcIndex = 0; nProcIndex < arAllProcedures.GetSize() && !bAdded; nProcIndex++) {
						EMNProcedure tempProc = arAllProcedures.GetAt(nProcIndex);
						if(m_arEmnInfo.GetAt(i).arProcedures.GetAt(j).strName > tempProc.strName) {
							// (z.manning 2009-09-22 17:28) - PLID 35631 - We trim the EMR description text,
							// so in order to prevent a possible break in the EMR description generation for
							// procedures that end in a space, let's also trim the procedure name.
							EMNProcedure proc = m_arEmnInfo[i].arProcedures[j];
							proc.strName.Trim();
							arAllProcedures.InsertAt(nProcIndex, proc);
							bAdded = true;
						}
					}
					if(!bAdded) {
						// (z.manning 2009-09-22 17:28) - PLID 35631 - We trim the EMR description text,
						// so in order to prevent a possible break in the EMR description generation for
						// procedures that end in a space, let's also trim the procedure name.
						EMNProcedure proc = m_arEmnInfo[i].arProcedures[j];
						proc.strName.Trim();
						arAllProcedures.Add(proc);
					}
				}
			}
		}

		//DRT 7/13/2007 - PLID 26671 - We don't need to query, we have the info here now!
		for(i = 0; i < arAllProcedures.GetSize(); i++) {
			strCurDescription.Replace(arAllProcedures[i].strName, "");
		}

		//punctuation
		strCurDescription.Replace(",","");
		strCurDescription.Replace(" ","");

		//if anything is left, we can't do anything, else re-generate
		if(strCurDescription.IsEmpty())
			strNewDescription = GenerateEMRDescription();
		else
			return;
	}

	//now set the description
	if(!strNewDescription.IsEmpty()) {		
		// (a.walling 2012-05-18 17:18) - PLID 50546 - Set the description
		pEMR->SetDescription(strNewDescription);
	}
}

CString CEmrEditorDlg::GenerateEMRDescription()
{
	//create a description based on the procedures on all EMNs

	//Get the list of procedures.
	//DRT 7/13/2007 - PLID 26671 - Reworked to better handle the fact that we have the whole procedure info, 
	//	not just an ID any longer.
	CArray<EMNProcedure, EMNProcedure> arAllProcedures;
	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
	int i = 0;

	for(i = 0; i < m_arEmnInfo.GetSize(); i++) {
		for(int j = 0; j < m_arEmnInfo[i].arProcedures.GetSize(); j++) {
			//Is it already in our list?
			bool bFound = false;
			for(int k = 0; k < arAllProcedures.GetSize() && !bFound; k++) {
				//Compare on ID, we are trying to avoid duplicates
				if(arAllProcedures[k].nID == m_arEmnInfo[i].arProcedures[j].nID) bFound = true;
			}
			if(!bFound) {
				// (z.manning 2009-09-22 17:28) - PLID 35631 - We trim the EMR description text,
				// so in order to prevent a possible break in the EMR description generation for
				// procedures that end in a space, let's also trim the procedure name.
				EMNProcedure proc = m_arEmnInfo[i].arProcedures[j];
				proc.strName.Trim();
				arAllProcedures.Add(proc);
			}
		}
	}

	//DRT 7/13/2007 - PLID 26671 - We no longer need to query, we have access to the procedure names
	//	in our array.
	CString strProcedureNames;
	for(i = 0; i < arAllProcedures.GetSize(); i++) {
		if(!strProcedureNames.IsEmpty()) {
			strProcedureNames += ", ";
		}
		strProcedureNames += arAllProcedures[i].strName;
	}

	if(strProcedureNames.IsEmpty())
		strProcedureNames = "<No Procedure Determined>";

	return strProcedureNames;
}

CString CEmrEditorDlg::GetProcedureName(long nProcedureID)
{
	CString strName;
	if(m_mapProcedureNames.Lookup(nProcedureID,strName)) {
		//Found it!
		return strName;
	}
	else {
		//We haven't stored this yet, so look it up in data.
		strName = VarString(GetTableField("ProcedureT","Name","ID",nProcedureID));
		m_mapProcedureNames.SetAt(nProcedureID,strName);
		return strName;
	}
}


//TES 5/20/2008 - PLID 27905 - This is sometimes called when procedures are about to be deleted; if so, pass in the EMN
// they're being deleted from and this will only return true if the procedure is on some other EMN.
BOOL CEmrEditorDlg::IsProcedureInEMR(long nProcedureID, CEMN *pExcludingEmn /*= NULL*/)
{
	return m_wndEmrTree.IsProcedureInEMR(nProcedureID, pExcludingEmn);
}

// (a.walling 2008-06-27 13:57) - PLID 30496
CEMN* CEmrEditorDlg::GetEMNByID(int nID)
{
	return m_wndEmrTree.GetEMNByID(nID);
}

long CEmrEditorDlg::GetEmrID()
{
	return m_wndEmrTree.GetEMRID();
}

// (j.jones 2007-01-30 11:21) - PLID 24353 - we commit EMN times when the dialog is closed
// (j.jones 2007-02-06 15:04) - PLID 24509 - also commits EMR time
void CEmrEditorDlg::StopTrackingEMNTimes()
{
	m_wndEmrTree.StopTrackingTimes();
}

void CEmrEditorDlg::OnTimer(UINT nIDEvent) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (j.jones 2007-01-31 13:57) - PLID 24493 - increment the clocks
		if(nIDEvent == ID_EMR_CLOCK_TICK) {

			KillTimer(ID_EMR_CLOCK_TICK);

			// (c.haag 2010-01-14 12:04) - PLID 26036 - Update our states which track
			// whether the clock is paused or not. This function internally checks whether
			// the window is minimized (or maybe other circumstances in future implementations)
			// and pauses or resumes the clock accordingly.
			//
			// The reason we don't do this in OnSysCommand, which is a message event
			// that transpires when you minimize or restore a window, is because that message
			// may not always preceed the event. (Try minimizing an EMR, then minimizing and
			// maximizing Practice). This timer is the closest thing I know of now to a reliable
			// way to "control the timer".
			//
			UpdateClockPauseState();

			if(m_dtCurrentTimeOpened.GetStatus() == COleDateTime::invalid) {
				ASSERT(FALSE);
				//shouldn't happen, but if so, restart tracking time for the clock
				m_dtCurrentTimeOpened = COleDateTime::GetCurrentTime() + m_dtOffset;
				//SetDlgItemText(IDC_EDIT_CURRENT_TIME_OPEN, "0:00:00");

				// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
				m_nCurrentSecondsOpen = 0;

				SetTimer(ID_EMR_CLOCK_TICK,0,NULL);
			}

			//calculate the time the EMR has been open currently
			COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
			COleDateTimeSpan dtSpan = dtNow - m_dtCurrentTimeOpened;
			// (c.haag 2010-01-14 11:57) - PLID 26036 - Subtract the time it was paused
			dtSpan -= GetTotalTimeClockWasPaused();

			long nMinuteDifference = (dtSpan.GetHours() * 60) + dtSpan.GetMinutes();

			if(nMinuteDifference < 0) {
				//with the offset, this shouldn't be possible unless the server time changed
				//SetDlgItemText(IDC_EDIT_CURRENT_TIME_OPEN, "0:00:00");	

				// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
				m_nCurrentSecondsOpen = 0;	
			}
			else {
				//CString strTime;
				//strTime.Format("%li:%02li:%02li", dtSpan.GetHours(), dtSpan.GetMinutes(), dtSpan.GetSeconds());
				//SetDlgItemText(IDC_EDIT_CURRENT_TIME_OPEN, strTime);

				// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
				m_nCurrentSecondsOpen = static_cast<int>(dtSpan.GetTotalSeconds());
			}

			//now calculate the time the EMR has been open historically
			COleDateTimeSpan dtTotalSpan = m_dtHistoricalTimeOpenedSpan + dtSpan;

			nMinuteDifference = (dtTotalSpan.GetHours() * 60) + dtTotalSpan.GetMinutes();

			if(nMinuteDifference < 0) {
				//with the offset, this shouldn't be possible unless the server time changed
				//SetDlgItemText(IDC_EDIT_TOTAL_TIME_OPEN, "0:00:00");		

				// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
				m_nTotalSecondsOpen = 0;
			}
			else {
				//CString strTime;
				//strTime.Format("%li:%02li:%02li", dtTotalSpan.GetHours(), dtTotalSpan.GetMinutes(), dtTotalSpan.GetSeconds());
				//SetDlgItemText(IDC_EDIT_TOTAL_TIME_OPEN, strTime);

				// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
				m_nTotalSecondsOpen = static_cast<int>(dtTotalSpan.GetTotalSeconds());
			}

			//fire again in 1 second
			SetTimer(ID_EMR_CLOCK_TICK, 1000, NULL);
		}
		else if(nIDEvent == ID_EMR_CALC_OFFSET) {

			KillTimer(ID_EMR_CALC_OFFSET);

			//CalculateLocalTimeOffset ensures our offset
			//between server time and local time,
			//and restarts the timer
			CalculateLocalTimeOffset();
		}
	} NxCatchAll("Error in OnTimer");
	
	CEmrEditorBase::OnTimer(nIDEvent);
}

void CEmrEditorDlg::CalculateLocalTimeOffset()
{
	try {

		//calculate the difference between the local time and the server time,
		//and store the offset in m_dtOffset

		KillTimer(ID_EMR_CALC_OFFSET);

		_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS Now");
		if(!rs->eof) {
			COleDateTime dtServerNow = AdoFldDateTime(rs, "Now");
			COleDateTime dtLocalNow = COleDateTime::GetCurrentTime();

			m_dtOffset = dtServerNow - dtLocalNow;
			
			//now we can add m_dtOffset to COleDateTime::GetCurrentTime() to get a closer
			//approximation of what the server time actually is, without having to ask
			//the server
		}
		rs->Close();

		// (j.jones 2007-09-17 17:35) - PLID 27396 - send the server offset to the EMR
		m_wndEmrTree.UpdateServerTimeOffset(m_dtOffset);

		//check again in 60 minutes
		SetTimer(ID_EMR_CALC_OFFSET, 3600000, NULL);

	}NxCatchAll("Error in CEmrEditorDlg::CalculateLocalTimeOffset");
}

void CEmrEditorDlg::FireTimeChanged()
{
	//if the local time changed, reset our offset
	
	KillTimer(ID_EMR_CALC_OFFSET);
	//compare our server-to-local time offset again, hourly
	CalculateLocalTimeOffset();

	//the clock is always ticking, so the next tick will update the clocks accordingly
}

LRESULT CEmrEditorDlg::OnNonClinicalProceduresAdded(WPARAM wParam, LPARAM lParam)
{
	//
	// (c.haag 2007-03-07 09:14) - PLID 21207 - This message is posted when one or
	// more procedures are added to the non-clinical tab. Here we check to see whether
	// to add them to the clinical tab.
	//
	CArray<int,int>* panProcIDs = (CArray<int,int>*)wParam;
	CArray<CEMN*,CEMN*>* papEMNs = (CArray<CEMN*,CEMN*>*)lParam;
	try {
		int nEMNs = papEMNs->GetSize();
		int nProcedures = panProcIDs->GetSize();

		// (a.walling 2008-08-25 12:21) - PLID 30515 - Keep track of any concurrency errors
		CStringArray saErrors;

		for (int i=0; i < nEMNs; i++) {
			BOOL bEMNChanged = FALSE;
			CEMN* pEMN = papEMNs->GetAt(i);
			for (int j=0; j < nProcedures; j++) {
				long nProcedureID = panProcIDs->GetAt(j);

				// Add the procedure to the EMN if it does not already exist
				//DRT 7/13/2007 - PLID 26671 - We are using the whole procedure objects now instead of just IDs.
				CArray<EMNProcedure, EMNProcedure> aryProcedures;
				pEMN->GetProcedures(aryProcedures);
				const int nEMNProcedures = aryProcedures.GetSize();
				BOOL bFound = FALSE;
				for (int k=0; k < nEMNProcedures && !bFound; k++) {
					if (aryProcedures[k].nID == nProcedureID) {
						bFound = TRUE;
					}
				}
				if (!bFound) {
					// (a.walling 2008-08-11 17:22) - PLID 30515 - Try to acquire write access, if possible. If not, no big deal, they will be 
					// warned when they try to save/exit, and can re-acquire write access then.
					if (!pEMN->IsWritable()) {
						CWriteTokenInfo wtInfo;
						if (!pEMN->RequestWriteToken(wtInfo))
						{
							// (j.armen 2013-05-14 11:09) - PLID 56680 - Write Token Info keeps track of external status
							CString strMessage = FormatString("'%s': ", pEMN->GetDescription());

							if (wtInfo.bIsOldRevision)
								strMessage += "The loaded EMN has been modified by another user since it was initially opened.";
							else if (wtInfo.bIsDeleted)
								strMessage += "The loaded EMN has been deleted by another user.";	
							else if (wtInfo.bIsExternal)
								strMessage += FormatString("The EMN is currently held for editing by the user '%s' %s at %s (using an external device, identified as: %s).", wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday), FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime), wtInfo.strDeviceInfo);
							else
								strMessage += FormatString("The EMN is currently held for editing by the user '%s' %s at %s (using workstation %s).", wtInfo.strHeldByUserName, FormatDateTimeForInterface(wtInfo.dtHeld, NULL, dtoDateWithToday), FormatDateTimeForInterface(wtInfo.dtHeld, 0, dtoTime), wtInfo.strDeviceInfo);

							saErrors.Add(strMessage);
						}
					}

					if (pEMN->IsWritable()) {
						EMNProcedure* pProc = pEMN->AddProcedure(nProcedureID, VarString(GetTableField("ProcedureT", "Name", "ID", nProcedureID),""));
						m_wndEmrTree.SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProc, (LPARAM)pEMN);
						bEMNChanged = TRUE;
					}
				}
			}
			// Now make sure the description of the EMN is updated in the same manner as when a user
			// chooses a procedure from the more info section
			if (bEMNChanged) {
				m_wndEmrTree.SendMessage(NXM_EMN_CHANGED, (WPARAM)pEMN);
			}
		}

		// (a.walling 2008-08-11 17:36) - PLID 30515 - Warn the user if any EMNs could not be updated.
		if (saErrors.GetSize() > 0) {
			CString strErrors = "The following errors occurred due to conflicts of write access: \r\n\r\n";
			for (int f = 0; f < saErrors.GetSize(); f++) {
				strErrors += saErrors[f] + "\r\n";
			}
			strErrors += "\r\nThese procedures must manually be added to the EMN(s).";

			MessageBox(strErrors, NULL, MB_ICONEXCLAMATION);
		}	
	}
	NxCatchAll("Error in CEmrEditorDlg::OnNonClinicalProceduresAdded");
	delete panProcIDs;
	delete papEMNs;
	return 0;
}

long CEmrEditorDlg::GetPatientID() const
{
	// (c.haag 2007-03-07 16:30) - PLID 25110 - Returns the patient ID
	// (j.jones 2016-02-12 13:02) - PLID 68073 - improved the error message
	if (!GetPicContainer()) ThrowNxException("CEmrEditorDlg::GetPatientID tried to get the patient ID from invalid PIC dialog!");
	return GetPicContainer()->GetPatientID();
}

CString CEmrEditorDlg::GetPatientName() const
{
	// (c.haag 2007-03-07 16:30) - PLID 25110 - Returns the name of the active patient
	// (j.jones 2016-02-12 13:02) - PLID 68073 - improved the error message
	if (!GetPicContainer()) ThrowNxException("CEmrEditorDlg::GetPatientName tried to get the patient name from invalid PIC dialog!");
	return GetPicContainer()->GetPatientName();
}

// (c.haag 2008-07-09 12:41) - PLID 30648 - This is called when cancelling out of an
// unsaved EMR. We need to see if any todo alarms were spawned or unspawned for unsaved
// topics. If so, the user needs to be made aware of them, and resolve them by either
// getting rid of them, or leaving them alone.
// (c.haag 2008-07-10 16:42) - Returns TRUE if the user accepted any changes, or there
// were no known changes to make. Returns FALSE if they cancelled out and changed their mind.
BOOL CEmrEditorDlg::FindAndResolveChangedEMRTodoAlarms()
{
	CEMR* pEMR = GetEMR();
	if (NULL != pEMR) {
		CEMRUnresolvedTodosDlg dlg(this);
		// (c.haag 2008-07-14 12:15) - PLID 30696 - We now include both spawning and manually
		// added/removed todo alarms
		pEMR->GenerateCreatedTodosWhileUnsavedList(dlg.m_apCreatedEMNTodos);
		pEMR->GenerateDeletedTodosWhileUnsavedList(dlg.m_apDeletedEMNTodos);
		if (dlg.m_apCreatedEMNTodos.GetSize() > 0 ||
			dlg.m_apDeletedEMNTodos.GetSize() > 0)
		{
			if (IDCANCEL == dlg.DoModal()) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "pause" the clock timer that tracks how long
// someone has had the chart open
void CEmrEditorDlg::PauseClock()
{
	if(GetRemotePropertyInt("EMRStopTimeTrackingWhenMinimized", 0, 0, "<None>", true) == 1) 
	{
		if (COleDateTime::valid != m_dtLastPaused_Local.GetStatus()) {
			// If we get here, the clock is ticking and were not paused. So, we need to pause now.
			m_dtLastPaused_Local = COleDateTime::GetCurrentTime();
			// Now tell the EMR we're pausing because it's responsible for writing to the time slip table
			CEMR* pEMR = GetEMR();
			if (pEMR) {
				pEMR->PauseClock(m_dtLastPaused_Local + m_dtOffset); // Pass this on to the EMR in SERVER time
			}
		}
		else {
			// This function was called even though we were paused. Ignore it.
		}
	}
}

// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "resume" the clock timer that tracks how long
// someone has had the chart open
void CEmrEditorDlg::ResumeClock()
{
	if(GetRemotePropertyInt("EMRStopTimeTrackingWhenMinimized", 0, 0, "<None>", true) == 1) 
	{
		if (COleDateTime::valid == m_dtLastPaused_Local.GetStatus()) {
			// If we get here, the clock is paused, and we need to start it again.
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			COleDateTimeSpan dts = dtNow - m_dtLastPaused_Local;
			m_dtsPreviousTimePaused += dts;
			m_dtLastPaused_Local.SetStatus(COleDateTime::invalid);
			// Now tell the EMR we're resuming because it's responsible for writing to the time slip table
			CEMR* pEMR = GetEMR();
			if (pEMR) {
				pEMR->ResumeClock(dtNow + m_dtOffset); // Pass this on to the EMR in SERVER time
			}
		}
		else {
			// This function was called even though we aren't paused. Ignore it.
		}
	}
}

// (c.haag 2010-01-14 11:35) - PLID 26036 - Returns the total amount of time paused since the
// EMR was opened
COleDateTimeSpan CEmrEditorDlg::GetTotalTimeClockWasPaused() const
{
	COleDateTimeSpan dts = m_dtsPreviousTimePaused;
	if (COleDateTime::valid == m_dtLastPaused_Local.GetStatus()) {
		dts += (COleDateTime::GetCurrentTime() - m_dtLastPaused_Local);
	}
	return dts;
}

// (c.haag 2010-01-14 12:04) - PLID 26036 - Update our states which track
// whether the clock is paused or not
void CEmrEditorDlg::UpdateClockPauseState()
{
	if(GetRemotePropertyInt("EMRStopTimeTrackingWhenMinimized", 0, 0, "<None>", true) == 1) 
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetTopLevelParent() && GetTopLevelParent()->GetWindowPlacement(&wp)) {
			if (wp.showCmd == SW_MINIMIZE || wp.showCmd == SW_SHOWMINIMIZED) {
				PauseClock();
			} else {
				ResumeClock();
			}
		} else {
			// Nothing we can do here
		}
	}
}

// (j.jones 2010-03-31 17:01) - PLID 37980 - added ability to tell the EMR to add a given image
LRESULT CEmrEditorDlg::OnAddImageToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		//CanBeEdited() checks locked status, write token, and permissions
		if(IsWindow(m_wndEmrTree.GetSafeHwnd())
			&& GetActiveEMN() != NULL && GetActiveEMN()->CanBeEdited()
			&& m_wndEmrTree.HasWriteableEMRTopicOpen()) {

			//we have a writeable topic open, so continue
			m_wndEmrTree.PostMessage(NXM_ADD_IMAGE_TO_EMR, wParam, lParam);
		}
		else {
			//We can't add the image to this EMN for some reason.
			//Silently exit, but log this.
			
			BSTR bstrTextData = (BSTR)wParam;
			CString strImageFile(bstrTextData);
			SysFreeString(bstrTextData);

			Log("CEmrEditorDlg::OnAddImageToEMR - Could not add image file '%s' to the EMR because the active EMR has no active, writeable topic.", strImageFile);
			return 0;
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
LRESULT CEmrEditorDlg::OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		//CanBeEdited() checks locked status, write token, and permissions
		if(IsWindow(m_wndEmrTree.GetSafeHwnd())
			&& GetActiveEMN() != NULL && GetActiveEMN()->CanBeEdited()
			&& m_wndEmrTree.HasWriteableEMRTopicOpen()) {

			//we have a writeable topic open, so continue
			// (d.lange 2011-04-12 17:16) - PLID 42754 - Using SendMessage instead of PostMessage
			m_wndEmrTree.SendMessage(NXM_ADD_GENERIC_TABLE_TO_EMR, wParam, lParam);
		}
		else {
			//We can't add the table to this EMN for some reason.
			//Silently exit, but log this.
			Log("CEmrEditorDlg::OnAddGenericTableToEMR - Could not add a generic table to the EMR because the active EMR has no active, writeable topic.");
			return 0;
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (d.lange 2010-06-29 17:59) - PLID 39202 - Event for showing menu for device info
void CEmrEditorDlg::OnBnClickedBtnDeviceInfo()
{
	try {

		// (j.jones 2011-08-22 13:51) - PLID 44977 - if the preference says to only open the import,
		// then go directly to the import screen, do not pass go, do not collect $200
		if(GetRemotePropertyInt("DeviceImport_EMRButtonOnlyImport", 0, 0, GetCurrentUserName(), true) == 1) {
			OnBtnShowDeviceImport();
			return;
		}

		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		//CMenu mnu;
		mnu.CreatePopupMenu();
		long nIndex = 0;

		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_LINK_TO_DEVICE_IMPORT, "Import from &Device");

		// (d.lange 2010-06-30 09:09) - PLID 38687 - Setup a menu item for each device plugin that is enabled and supports 
		// the 'LaunchDevice' functionality, this is determined by their 'GetLaunchDeviceSettings' function
		// (j.gruber 2013-04-04 09:28) - PLID 56012 - concolidate
		CArray<DeviceLaunchUtils::DevicePlugin*, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;
		DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, &mnu, nIndex, TRUE, MF_BYPOSITION);

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_DEVICE_INFO);
		long nResult = 0;

		CPoint pt;
		GetCursorPos(&pt);
		nResult = mnu.TrackPopupMenu(TPM_RIGHTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		if(nResult == IDM_LINK_TO_DEVICE_IMPORT) {
			//Show the Device Import dialog
			OnBtnShowDeviceImport();			
		}else if(nResult > 0) {
			// (d.lange 2010-06-30 09:14) - PLID 38687 - the returned menu item is the memory location for the plugin
			// (j.gruber 2013-04-04 09:30) - PLID 56012 - consolidate
			// (j.gruber 2013-04-08 15:40) - PLID 56096 - this shouldn't be GetActivePatientID, it should be the patientID of the EMN we are in
			DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nResult, GetPatientID());
		}
		DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);
		
	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-06-29 17:59) - PLID 39202 - Event for showing the CDeviceImportDlg dialog
void CEmrEditorDlg::OnBtnShowDeviceImport()
{
	try {
		//Let's show the Device Import dialog
		GetMainFrame()->ShowDevicePluginImportDlg();

	} NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-08-02 09:27) - PLID 38928 - Called when EMR content is saved
void CEmrEditorDlg::Commit()
{
	// Now commit at the PIC level
	if(GetPicContainer()) {
		GetPicContainer()->Commit();
	}
}

// (j.jones 2012-03-27 14:59) - PLID 44763 - added global period warning
void CEmrEditorDlg::CheckWarnGlobalPeriod_EMR(COleDateTime dtToCheck)
{
	try {

		//if we've already checked/warned once for this patient, don't warn again
		if(m_bCheckWarnedGlobalPeriod) {
			return;
		}

		//check their preference, they may not want to be warned at all
		if(GetRemotePropertyInt("CheckWarnGlobalPeriod_EMR", 0, 0, "<None>", true) == 0) {
			return;
		}

		// (j.jones 2012-07-24 10:44) - PLID 51737 - added a preference to only track global periods for
		// surgical codes only, if it is disabled when we would look at all codes
		long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

		// (j.jones 2012-07-26 15:14) - PLID 51827 - added another preference to NOT track global periods
		// if the charge uses modifier 78
		long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

		CString order = "ASC";
		if(GetRemotePropertyInt("GlobalPeriodSort", 0, 0, "<None>", true) == 1) {
			order = "DESC";
		}

		//find all global periods active on the dtToCheck date, which is likely the EMN date
		// (j.jones 2012-07-24 10:44) - PLID 51737 - optionally filter on surgical codes only
		// (j.jones 2012-07-26 15:14) - PLID 51827 - optionally exclude modifier 78
		_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT CPTCodeT.Code, ServiceT.Name, LineItemT.Date, CPTCodeT.GlobalPeriod, "
			"DATEADD(day,GlobalPeriod,LineItemT.Date) AS ExpDate "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 AND "
			"LineItemT.Date <= {STRING} AND DATEADD(day,GlobalPeriod,LineItemT.Date) > {STRING} "
			"AND LineItemT.PatientID = {INT} "
			"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
			"AND ({INT} <> 1 OR ServicePayGroupsT.Category = {CONST}) "
			"AND ({INT} <> 1 OR (Coalesce(ChargesT.CPTModifier, '') <> '78' AND Coalesce(ChargesT.CPTModifier2, '') <> '78' AND Coalesce(ChargesT.CPTModifier3, '') <> '78' AND Coalesce(ChargesT.CPTModifier4, '') <> '78')) "
			"ORDER BY DATEADD(day,GlobalPeriod,LineItemT.Date) %s", order),
			FormatDateTimeForSql(dtToCheck, dtoDate), FormatDateTimeForSql(dtToCheck, dtoDate),
			GetPatientID(), nSurgicalCodesOnly, PayGroupCategory::SurgicalCode, nIgnoreModifier78);

		if(!rs->eof) {

			CString str = "";
			CString strMsg = "This patient is still under the global period for:";

			while(!rs->eof) {

				CString strCPTCode = AdoFldString(rs, "Code","");
				CString strCPTDesc = AdoFldString(rs, "Name","");
				long nGlobalPeriod = AdoFldLong(rs, "GlobalPeriod");
				COleDateTime dtServiceDate = AdoFldDateTime(rs, "Date");
				COleDateTime dtExpDate = AdoFldDateTime(rs, "ExpDate");

				CString strServiceDate = FormatDateTimeForInterface(dtServiceDate);
				CString strExpDate = FormatDateTimeForInterface(dtExpDate);

				str.Format("\n\nService Code: (%s) %s, Service Date: %s, Global Period: %li days, Expires: %s",
					strCPTCode, strCPTDesc, strServiceDate, nGlobalPeriod, strExpDate);

				strMsg += str;
				
				rs->MoveNext();
			}

			//they will be prompted with this information, but will not have the option to cancel
			MessageBox(strMsg, "Practice", MB_ICONQUESTION|MB_OK);
		}
		rs->Close();

		//now set the boolean to true, even if we didn't warn, because
		//we aren't going to search for global periods a second time
		//during this EMR session
		m_bCheckWarnedGlobalPeriod = TRUE;

	}NxCatchAll(__FUNCTION__);
}