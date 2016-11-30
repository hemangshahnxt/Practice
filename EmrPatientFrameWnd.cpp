// EmrFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrIcons.h"
#include "EmrPatientFrameWnd.h"
#include "EmrFrameWnd.h"
#include "EmrChildWnd.h"
#include "EMRProblemEditDlg.h"
#include "EMRProblemChooserDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "foreach.h"
// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxRibbonControls.h>
#include <NxAdvancedUILib/NxRibbonCheckList.h>
#include <NxAdvancedUILib/NxRibbonDateTime.h>
#include <NxAdvancedUILib/NxRibbonListButton.h>
#include <NxAdvancedUILib/NxRibbonPanel.h>
#include <afxpriv.h>
#include <afxribbonbar.h>
#include <set>
#include "EditEMNOtherProvidersDlg.h"
#include "PatientSummaryDlg.h"
#include "EmrAuditHistoryDlg.h"
#include "EMNConfidentialInfoDlg.h"
#include <NxPracticeSharedLib/PracDataEmr.h>
#include <NxPracticeSharedLib/PracData.h>
#include <NxSystemUtilitiesLib/dynamic_ptr.h>
#include <NxAlgorithm.h>
#include "EmrRibbonControls.h"
#include "EMRTopic.h"
#include "EMR.h"
#include "NxApi.h"
#include "MergeEngine.h"
#include "CCDInterface.h"
#include "NxAPIUtils.h"
#include "EMN.h"
#include "EMNProvider.h"
#include "EMRUtils.h" // (b.savon 2014-02-25 14:16) - PLID 61029
#include "EMRProblemListDlg.h"

#pragma TODO("!!!no PLID (specifically, yet) -- Ensure cases where there is NO active EMN are handled!!!")

// (a.walling 2011-10-20 21:22) - PLID 46076 - Facelift - EMR frame window

// (a.walling 2012-02-28 14:53) - PLID 48451 - CEmrPatientFrameWnd - move patient-related logic here, keeping CEmrFrameWnd agnostic for both template and patient frames

// (a.walling 2012-02-29 06:42) - PLID 46644 - A bit of reorganization with ribbon initialization

// (a.walling 2012-05-22 09:59) - PLID 50556 - Prevent unnecessary redraws of ribbon combo boxes by always using CNxRibbonAutoComboBox instead of CMFCRibbonComboBox

// (a.walling 2012-05-22 15:15) - PLID 50582 - Use CNxRibbonButton, CNxRibbonCheckbox everywhere to ensure all bugs are consistently worked around

// (a.walling 2012-06-26 13:36) - PLID 51204 - Use CNxRibbonCategory, CNxRibbonPanel

// (a.walling 2012-10-01 09:04) - PLID 52119 - EmrRibbonControls now has CNxRibbonMergeTemplateComboBox

// (a.walling 2013-04-08 08:57) - PLID 56131 - Added more icons, more!

// (a.walling 2014-04-24 12:00) - VS2013 - update cached multi_index structs to use ->get<>()

// CEmrPatientFrameWnd

IMPLEMENT_DYNAMIC(CEmrPatientFrameWnd, CEmrFrameWnd)

namespace {
	std::vector<CEmrPatientFrameWnd*> g_allPatientFrameWnds;
}

CEmrPatientFrameWnd::CEmrPatientFrameWnd()
	: CEmrFrameWnd(false)
{
	g_allPatientFrameWnds.push_back(this);
}

CEmrPatientFrameWnd::~CEmrPatientFrameWnd()
{
	boost::remove_erase(g_allPatientFrameWnds, this);
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::UpdateContextCategories()
{
	//Photos pane
	ShowContextCategory(EmrContextCategory::PhotosPane, m_emrPhotosPane.IsVisible() ? true : false);
	// preview pane
	// "!!" converts BOOL to bool intentionally here
	ShowContextCategory(EmrContextCategory::PreviewPane, !!m_emrPreviewPane.IsVisible());
}

BEGIN_MESSAGE_MAP(CEmrPatientFrameWnd, CEmrFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()

	////
	/// UI state

	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_EMN, &CEmrPatientFrameWnd::OnUpdateNewEMN)
	ON_UPDATE_COMMAND_UI(ID_EMR_STATUS, &CEmrPatientFrameWnd::OnUpdateEMRStatus)	// (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
	ON_UPDATE_COMMAND_UI(ID_EMR_DESCRIPTION, &CEmrPatientFrameWnd::OnUpdateEMRDescription) // (a.walling 2012-05-18 17:18) - PLID 50546 - EMR Description

	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_PHOTOS_PANE, &CEmrPatientFrameWnd::OnUpdateViewPhotosPane)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_MU_PROGRESS_PANE, &CEmrPatientFrameWnd::OnUpdateViewMUProgressPane) //(e.lally 2012-02-23) PLID 48016
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_GRAPHING_PANE, &CEmrPatientFrameWnd::OnUpdateViewGraphingPane) // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_STATUS, &CEmrPatientFrameWnd::OnUpdateStatus)

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_NOTES, &CEmrPatientFrameWnd::OnUpdateNotes)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_LOCATION, &CEmrPatientFrameWnd::OnUpdateLocation)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_DATE, &CEmrPatientFrameWnd::OnUpdateDate)
	// (a.walling 2012-06-07 08:59) - PLID 50920 - Dates - Modified, Created
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_DATE_CREATED, &CEmrPatientFrameWnd::OnUpdateDateCreated)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_DATE_MODIFIED, &CEmrPatientFrameWnd::OnUpdateDateModified)

	// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_PROVIDER, &CEmrPatientFrameWnd::OnUpdateProvider)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_SECONDARY_PROVIDER, &CEmrPatientFrameWnd::OnUpdateSecondaryProvider)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_ASSISTANT, &CEmrPatientFrameWnd::OnUpdateAssistants)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_OTHER_PROVIDERS, &CEmrPatientFrameWnd::OnUpdateOtherProviders)

	// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_PROCEDURES, &CEmrPatientFrameWnd::OnUpdateProcedures)	

	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_TODO, &CEmrPatientFrameWnd::OnUpdateNewTodo)
	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_RECORDING, &CEmrPatientFrameWnd::OnUpdateNewRecording)
	// (a.walling 2012-06-11 09:34) - PLID 50925
	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_CHARGE, &CEmrPatientFrameWnd::OnUpdateNewCharge)
	ON_UPDATE_COMMAND_UI(ID_EMR_NEW_DIAG_CODE, &CEmrPatientFrameWnd::OnUpdateNewDiagCode)

	ON_UPDATE_COMMAND_UI(ID_EMR_PATIENT_SUMMARY, &CEmrPatientFrameWnd::OnUpdatePatientSummary)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_APPOINTMENT, &CEmrPatientFrameWnd::OnUpdateLinkedAppointment) // (a.walling 2013-02-13 10:41) - PLID 55143 - Emr Appointment linking - UI
	ON_UPDATE_COMMAND_UI(ID_EMR_SHOW_CONFIDENTIAL_INFO, &CEmrPatientFrameWnd::OnUpdateShowConfidentialInfo)
	ON_UPDATE_COMMAND_UI(ID_EMR_UPDATE_DEMOGRAPHICS, &CEmrPatientFrameWnd::OnUpdateUpdateDemographics)
		
	// (a.walling 2012-06-11 09:23) - PLID 50921 - Patient demographics: name, age, gender
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_PATIENT_NAME, &CEmrPatientFrameWnd::OnUpdatePatientName)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_PATIENT_AGE, &CEmrPatientFrameWnd::OnUpdatePatientAge)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_PATIENT_GENDER, &CEmrPatientFrameWnd::OnUpdatePatientGender)

	// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
	ON_UPDATE_COMMAND_UI(ID_EMR_SHOW_PROBLEMS, &CEmrPatientFrameWnd::OnUpdateShowProblems)
	ON_UPDATE_COMMAND_UI(ID_EMR_ADD_NEW_PROBLEM, &CEmrPatientFrameWnd::OnUpdateAddNewProblem)
	ON_UPDATE_COMMAND_UI(ID_EMR_LINK_PROBLEM, &CEmrPatientFrameWnd::OnUpdateLinkExistingProblem)
	ON_UPDATE_COMMAND_UI(ID_EMR_ADD_NEW_PROBLEM_TO_EMR, &CEmrPatientFrameWnd::OnUpdateAddNewProblemToEMR)
	ON_UPDATE_COMMAND_UI(ID_EMR_ADD_NEW_PROBLEM_TO_EMN, &CEmrPatientFrameWnd::OnUpdateAddNewProblemToEMN)
	ON_UPDATE_COMMAND_UI(ID_EMR_ADD_NEW_PROBLEM_TO_TOPIC, &CEmrPatientFrameWnd::OnUpdateAddNewProblemToTopic)
	ON_UPDATE_COMMAND_UI(ID_EMR_LINK_PROBLEM_TO_EMR, &CEmrPatientFrameWnd::OnUpdateLinkExistingProblemToEMR)
	ON_UPDATE_COMMAND_UI(ID_EMR_LINK_PROBLEM_TO_EMN, &CEmrPatientFrameWnd::OnUpdateLinkExistingProblemToEMN)
	ON_UPDATE_COMMAND_UI(ID_EMR_LINK_PROBLEM_TO_TOPIC, &CEmrPatientFrameWnd::OnUpdateLinkExistingProblemToTopic)

	// (b.spivey, March 06, 2012) - PLID 48581 - Update message
	ON_UPDATE_COMMAND_UI(ID_EMR_EM_CHECKLIST, &CEmrPatientFrameWnd::OnUpdateEMChecklist)
	ON_UPDATE_COMMAND_UI(ID_EMR_EM_VISITTYPE, &CEmrPatientFrameWnd::OnUpdateEMVisitType)

	ON_UPDATE_COMMAND_UI(ID_EMR_SHOW_EMR_AUDIT_HISTORY, &CEmrPatientFrameWnd::OnUpdateShowEMRAuditHistory)
	ON_UPDATE_COMMAND_UI(ID_EMR_SHOW_EMN_AUDIT_HISTORY, &CEmrPatientFrameWnd::OnUpdateShowEMNAuditHistory)

	//(e.lally 2012-02-16) PLID 48065
	ON_UPDATE_COMMAND_UI(ID_EMR_PROBLEM_LIST_BTN_MENU, &CEmrPatientFrameWnd::OnUpdateClassicBtnProblemList)

	//(e.lally 2012-04-24) PLID 49637
	ON_UPDATE_COMMAND_UI(ID_EMR_PHOTO_DISPLAY_LABELS, &CEmrPatientFrameWnd::OnUpdatePhotoDisplayLabels)
	//(e.lally 2012-04-12) PLID 49566 - Photo pane filtering
	ON_UPDATE_COMMAND_UI(ID_EMR_PHOTO_FILTER_CATEGORY, &CEmrPatientFrameWnd::OnUpdatePhotoCategoryFilter)
	ON_UPDATE_COMMAND_UI(ID_EMR_PHOTO_FILTER_PROCEDURE, &CEmrPatientFrameWnd::OnUpdatePhotoProcedureFilter)
	//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
	ON_UPDATE_COMMAND_UI(ID_EMR_PHOTO_SORT, &CEmrPatientFrameWnd::OnUpdatePhotoSort)
	ON_UPDATE_COMMAND_UI(ID_EMR_PHOTO_SORT_ASCENDING, &CEmrPatientFrameWnd::OnUpdatePhotoSortAscending)
	//(e.lally 2012-04-17) PLID 49636
	ON_UPDATE_COMMAND_UI(ID_EMR_PHOTO_GROUP, &CEmrPatientFrameWnd::OnUpdatePhotoGroup)

	// (a.walling 2013-11-15 11:16) - PLID 59517 - Care Summary, Clinical Summary in ribbon

	ON_UPDATE_COMMAND_UI(ID_EMR_CARE_SUMMARY, &CEmrPatientFrameWnd::OnUpdate_AlwaysEnable)
	ON_UPDATE_COMMAND_UI(ID_EMR_CLINICAL_SUMMARY, &CEmrPatientFrameWnd::OnUpdate_AlwaysEnable)
	ON_UPDATE_COMMAND_UI(ID_EMR_CARE_SUMMARY_CUSTOMIZED, &CEmrPatientFrameWnd::OnUpdate_AlwaysEnable)
	ON_UPDATE_COMMAND_UI(ID_EMR_CLINICAL_SUMMARY_CUSTOMIZED, &CEmrPatientFrameWnd::OnUpdate_AlwaysEnable)
	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	ON_UPDATE_COMMAND_UI(ID_EMR_ASC_DISCHARGE_STATUS, &CEmrPatientFrameWnd::OnUpdateDischargeStatus)
	ON_UPDATE_COMMAND_UI(ID_EMR_ASC_ADMISSION_TIME, &CEmrPatientFrameWnd::OnUpdateAdmissionTime)
	ON_UPDATE_COMMAND_UI(ID_EMR_ASC_DISCHARGE_TIME, &CEmrPatientFrameWnd::OnUpdateDischargeTime)

	////
	/// UI Commands

	ON_COMMAND(ID_EMR_NEW_EMN, &CEmrPatientFrameWnd::OnNewEMN)
	ON_COMMAND(ID_EMR_STATUS, &CEmrPatientFrameWnd::OnEMRStatus)	// (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
	ON_COMMAND(ID_EMR_DESCRIPTION, &CEmrPatientFrameWnd::OnEMRDescription) // (a.walling 2012-05-18 17:18) - PLID 50546 - EMR Description
	
	ON_COMMAND(ID_EMR_VIEW_PHOTOS_PANE, &CEmrPatientFrameWnd::OnViewPhotosPane)
	ON_COMMAND(ID_EMR_VIEW_MU_PROGRESS_PANE, &CEmrPatientFrameWnd::OnViewMUProgressPane) //(e.lally 2012-02-23) PLID 48016
	ON_COMMAND(ID_EMR_VIEW_GRAPHING_PANE, &CEmrPatientFrameWnd::OnViewGraphingPane)  // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

	ON_COMMAND(ID_EMR_EMN_STATUS, &CEmrPatientFrameWnd::OnStatus)

	ON_COMMAND(ID_EMR_EMN_NOTES, &CEmrPatientFrameWnd::OnNotes)
	ON_COMMAND(ID_EMR_EMN_LOCATION, &CEmrPatientFrameWnd::OnLocation)
	ON_COMMAND(ID_EMR_EMN_DATE, &CEmrPatientFrameWnd::OnDate)

	// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
	ON_COMMAND(ID_EMR_EMN_PROVIDER, &CEmrPatientFrameWnd::OnProvider)
	ON_COMMAND(ID_EMR_EMN_SECONDARY_PROVIDER, &CEmrPatientFrameWnd::OnSecondaryProvider)
	ON_COMMAND(ID_EMR_EMN_ASSISTANT, &CEmrPatientFrameWnd::OnAssistants)
	ON_COMMAND(ID_EMR_EMN_OTHER_PROVIDERS, &CEmrPatientFrameWnd::OnOtherProviders)

	// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
	ON_COMMAND(ID_EMR_EMN_PROCEDURES, &CEmrPatientFrameWnd::OnProcedure)

	ON_COMMAND(ID_EMR_NEW_TODO, &CEmrPatientFrameWnd::OnNewTodo)
	ON_COMMAND(ID_EMR_NEW_RECORDING, &CEmrPatientFrameWnd::OnNewRecording)
	// (a.walling 2012-06-11 09:34) - PLID 50925
	ON_COMMAND(ID_EMR_NEW_CHARGE, &CEmrPatientFrameWnd::OnNewCharge)
	ON_COMMAND(ID_EMR_NEW_DIAG_CODE, &CEmrPatientFrameWnd::OnNewDiagCode)

	ON_COMMAND(ID_EMR_PATIENT_SUMMARY, &CEmrPatientFrameWnd::OnPatientSummary)
	ON_COMMAND(ID_EMR_EMN_APPOINTMENT, &CEmrPatientFrameWnd::OnLinkedAppointment) // (a.walling 2013-02-13 10:41) - PLID 55143 - Emr Appointment linking - UI
	ON_COMMAND(ID_EMR_SHOW_CONFIDENTIAL_INFO, &CEmrPatientFrameWnd::OnShowConfidentialInfo)
	ON_COMMAND(ID_EMR_UPDATE_DEMOGRAPHICS, &CEmrPatientFrameWnd::OnUpdateDemographics)

	// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
	ON_COMMAND(ID_EMR_SHOW_PROBLEMS, &CEmrPatientFrameWnd::OnShowProblems)
	ON_COMMAND(ID_EMR_ADD_NEW_PROBLEM, &CEmrPatientFrameWnd::OnAddNewProblem)
	ON_COMMAND(ID_EMR_LINK_PROBLEM, &CEmrPatientFrameWnd::OnLinkExistingProblem)
	ON_COMMAND(ID_EMR_ADD_NEW_PROBLEM_TO_EMR, &CEmrPatientFrameWnd::OnAddNewProblemToEMR)
	ON_COMMAND(ID_EMR_ADD_NEW_PROBLEM_TO_EMN, &CEmrPatientFrameWnd::OnAddNewProblemToEMN)
	ON_COMMAND(ID_EMR_ADD_NEW_PROBLEM_TO_TOPIC, &CEmrPatientFrameWnd::OnAddNewProblemToTopic)
	ON_COMMAND(ID_EMR_LINK_PROBLEM_TO_EMR, &CEmrPatientFrameWnd::OnLinkExistingProblemToEMR)
	ON_COMMAND(ID_EMR_LINK_PROBLEM_TO_EMN, &CEmrPatientFrameWnd::OnLinkExistingProblemToEMN)
	ON_COMMAND(ID_EMR_LINK_PROBLEM_TO_TOPIC, &CEmrPatientFrameWnd::OnLinkExistingProblemToTopic)



	// (b.spivey, March 06, 2012) - PLID 48581 - Event handlers. 
	ON_COMMAND(ID_EMR_EM_CHECKLIST, &CEmrPatientFrameWnd::OnShowEMChecklist)
	ON_COMMAND(ID_EMR_EM_VISITTYPE, &CEmrPatientFrameWnd::OnSelectEMVisitType)

	ON_COMMAND(ID_EMR_SHOW_EMR_AUDIT_HISTORY, &CEmrPatientFrameWnd::OnShowEMRAuditHistory)
	ON_COMMAND(ID_EMR_SHOW_EMN_AUDIT_HISTORY, &CEmrPatientFrameWnd::OnShowEMNAuditHistory)

	//(e.lally 2012-04-24) PLID 49637
	ON_COMMAND(ID_EMR_PHOTO_DISPLAY_LABELS, &CEmrPatientFrameWnd::OnPhotoDisplayLabels)
	//(e.lally 2012-04-12) PLID 49566 - Photo pane filtering
	ON_COMMAND(ID_EMR_PHOTO_FILTER_CATEGORY, &CEmrPatientFrameWnd::OnPhotoCategoryFilter)
	ON_COMMAND(ID_EMR_PHOTO_FILTER_PROCEDURE, &CEmrPatientFrameWnd::OnPhotoProcedureFilter)
	//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
	ON_COMMAND(ID_EMR_PHOTO_SORT, &CEmrPatientFrameWnd::OnPhotoSort)
	ON_COMMAND(ID_EMR_PHOTO_SORT_ASCENDING, &CEmrPatientFrameWnd::OnPhotoSortAscending)
	//(e.lally 2012-04-17) PLID 49636
	ON_COMMAND(ID_EMR_PHOTO_GROUP, &CEmrPatientFrameWnd::OnPhotoGroup)

	// (a.walling 2013-11-15 11:16) - PLID 59517 - Care Summary, Clinical Summary in ribbon

	ON_COMMAND(ID_EMR_CARE_SUMMARY, &CEmrPatientFrameWnd::OnCareSummary)
	ON_COMMAND(ID_EMR_CLINICAL_SUMMARY, &CEmrPatientFrameWnd::OnClinicalSummary)
	ON_COMMAND(ID_EMR_CARE_SUMMARY_CUSTOMIZED, &CEmrPatientFrameWnd::OnCareSummaryCustomized)
	ON_COMMAND(ID_EMR_CLINICAL_SUMMARY_CUSTOMIZED, &CEmrPatientFrameWnd::OnClinicalSummaryCustomized)
		// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	ON_COMMAND(ID_EMR_ASC_DISCHARGE_STATUS, &CEmrPatientFrameWnd::OnDischargeStatus)
	ON_COMMAND(ID_EMR_ASC_ADMISSION_TIME, &CEmrPatientFrameWnd::OnAdmissionTime)
	ON_COMMAND(ID_EMR_ASC_DISCHARGE_TIME, &CEmrPatientFrameWnd::OnDischargeTime)

	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, OnChangeActiveTab)

END_MESSAGE_MAP()

// CEmrPatientFrameWnd Command UI handlers

// CEmrPatientFrameWnd Command handlers

// CEmrPatientFrameWnd message handlers

void CEmrPatientFrameWnd::InitializePanes()
{
	m_emrPreviewPane.CreateEx(	WS_EX_CONTROLPARENT, "Preview",	this, CRect(0, 0, 250, 100), TRUE, ID_EMR_VIEW_PREVIEW_PANE,						WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT |  CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE);
	m_emrTreePane.CreateEx(		WS_EX_CONTROLPARENT, "Topic List",			this, CRect(0, 0, 250, 100), TRUE, ID_EMR_VIEW_TREE_PANE,				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT  |  CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE); 
	m_emrPhotosPane.CreateEx(	WS_EX_CONTROLPARENT, "Photos",				this, CRect(0, 0, 275, 172), TRUE, ID_EMR_VIEW_PHOTOS_PANE,				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT |  CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE);
	//(e.lally 2012-02-16) PLID 48065 - Add button pane from our "classic" interface
	m_emrClassicButtonPane.CreateEx(WS_EX_CONTROLPARENT, "Classic Buttons",	this, CRect(0, 0, 800, 70),  TRUE, ID_EMR_VIEW_CLASSIC_BUTTON_PANE,		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI | CBRS_SIZE_FIXED, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE);
	 //(e.lally 2012-02-23) PLID 48016 - Add pane with Meaningful Use progress bar
	m_emrMUProgressPane.CreateEx(WS_EX_CONTROLPARENT, "Meaningful Use",		this, CRect(0, 0, 400, 85),  TRUE, ID_EMR_VIEW_MU_PROGRESS_PANE,		WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_TOP    | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE);
	// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane
	m_emrGraphingPane.CreateEx(WS_EX_CONTROLPARENT, "Graphing",				this, CRect(0, 0, 400, 300),  TRUE, ID_EMR_VIEW_GRAPHING_PANE,			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT  | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE);

	// (a.walling 2012-06-28 10:08) - PLID 51258 - Explicitly allow converting a pane to a tabbed document
	m_emrPreviewPane.AllowTabbedDocument();
	m_emrPhotosPane.AllowTabbedDocument();
	m_emrGraphingPane.AllowTabbedDocument();

	m_emrPreviewPane.SafeGetEmrPreviewCtrl()->SetTreeWnd(m_emrTreePane.GetEmrTreeWnd());

	m_emrPreviewPane.EnableDocking(CBRS_ALIGN_ANY);
	m_emrTreePane.EnableDocking(CBRS_ALIGN_ANY);
	m_emrPhotosPane.EnableDocking(CBRS_ALIGN_ANY);
	m_emrClassicButtonPane.EnableDocking(CBRS_ALIGN_ANY); //(e.lally 2012-02-16) PLID 48065
	m_emrMUProgressPane.EnableDocking(CBRS_ALIGN_ANY);  //(e.lally 2012-02-23) PLID 48016
	m_emrGraphingPane.EnableDocking(CBRS_ALIGN_ANY); // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// (j.jones 2014-08-04 10:21) - PLID 63144 - initialize the refresh flag
	m_bPhotosPaneNeedsRefresh = false;

	// (j.armen 2012-07-02 11:30) - PLID 51313 - If the client has EMR or expired, then dock the panes, else hide them
	if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {

		DockPane(&m_emrTreePane);
		DockPane(&m_emrClassicButtonPane); //(e.lally 2012-02-16) PLID 48065
		DockPane(&m_emrMUProgressPane); //(e.lally 2012-02-23) PLID 48016

		{
			// standard docking
			/*DockPane(&m_emrPreviewPane);
			DockPane(&m_emrPhotosPane);
			DockPane(&m_emrGraphingPane);*/

			// (a.walling 2012-07-16 09:30) - PLID 51548 - Default layout: preview pane, photos, graphing should be docked together to the right
			// (a.walling 2012-07-16 09:30) - PLID 51548 - Trying to autohide a tabbed group by default makes the docking manager explode

			//DockPane(&m_emrPreviewPane);

			//
			//CDockablePane* pTabbedBar = NULL;
			//DockPane(&m_emrPhotosPane);
			//// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane
			//m_emrGraphingPane.AttachToTabWnd(&m_emrPhotosPane, DM_SHOW, TRUE, &pTabbedBar);

			//m_emrPreviewPane.SetAutoHideMode(TRUE, CBRS_ALIGN_RIGHT, NULL, FALSE);

			//// problem here -- this causes a ghost pane to remain, and showing hte autohide creates an infinite width window, ugh. No time to figure it out now really.
			//m_emrPhotosPane.SetAutoHideMode(TRUE, CBRS_ALIGN_RIGHT, NULL, FALSE);
			//m_emrGraphingPane.SetAutoHideMode(TRUE, CBRS_ALIGN_RIGHT, NULL, FALSE);
			
			// (a.walling 2012-07-16 09:30) - PLID 51548 - Even trying to set anythign to autohide on the right leads to bugs!
			/*DockPane(&m_emrPreviewPane);
			DockPane(&m_emrPhotosPane);
			DockPane(&m_emrGraphingPane);
			m_emrPreviewPane.SetAutoHideMode(TRUE, CBRS_ALIGN_RIGHT, NULL, FALSE);
			m_emrPhotosPane.SetAutoHideMode(TRUE, CBRS_ALIGN_RIGHT, NULL, FALSE);
			m_emrGraphingPane.SetAutoHideMode(TRUE, CBRS_ALIGN_RIGHT, NULL, FALSE);*/

			// (a.walling 2012-07-16 09:30) - PLID 51548 - Best we can do for now is to just keep them all in a tabbed group
						
			CDockablePane* pTabbedBar = NULL;
			DockPane(&m_emrPreviewPane);
			// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane
			m_emrPhotosPane.AttachToTabWnd(&m_emrPreviewPane, DM_SHOW, FALSE, &pTabbedBar);
			m_emrGraphingPane.AttachToTabWnd(pTabbedBar, DM_SHOW, FALSE, &pTabbedBar);
		}

		// (a.walling 2012-07-16 09:30) - PLID 51548 - Default layout, tree should be pinned
		//m_emrTreePane.SetAutoHideMode(TRUE, CBRS_LEFT, NULL, FALSE);
	} else {
		m_emrTreePane.ShowPane(FALSE, FALSE, FALSE);
		m_emrPreviewPane.ShowPane(FALSE, FALSE, FALSE);
		m_emrPhotosPane.ShowPane(FALSE, FALSE, FALSE);
		m_emrMUProgressPane.ShowPane(FALSE, FALSE, FALSE);
		m_emrClassicButtonPane.ShowPane(FALSE, FALSE, FALSE);
		m_emrGraphingPane.ShowPane(FALSE, FALSE, FALSE);
	}

	//(e.lally 2012-03-15) PLID 48915 - Set a minimum size so the panes can always be found. 
	//	The main goal is to avoid allowing the user to shrink them down to a 0x0 pane, 
	//	because it is hard to find the sizable border to make it visible again. 
	//	But for some, I made it large enough to see at least a partial icon.
	m_emrTreePane.SetMinSize(CSize(40, 65));
	m_emrPreviewPane.SetMinSize(CSize(40, 50));
	m_emrPhotosPane.SetMinSize(CSize(75, 75));
	m_emrMUProgressPane.SetMinSize(CSize(75, 50));
	m_emrClassicButtonPane.SetMinSize(CSize(75, 55));
	m_emrGraphingPane.SetMinSize(CSize(100, 100));
}

// (a.walling 2012-03-02 16:30) - PLID 48598 - return our layout section name
// (j.armen 2012-07-02 15:18) - PLID 51313 - use "EMR" when we have an emr license, and "PicOnly" when we do not
CString CEmrPatientFrameWnd::GetLayoutSectionName()
{
	return g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2 ? "EMR" : "PicOnly";
}

void CEmrPatientFrameWnd::InitializeRibbon()
{
	// (j.armen 2012-07-02 11:32) - PLID 51313 - Hide EMR related panes
	bool bHasEMR = g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2;
	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	InitializeRibbon_MainPanel();

	InitializeRibbon_Edit();

	if(bHasEMR) {
		InitializeRibbon_EMN();
		InitializeRibbon_MoreInfo();
	}
	InitializeRibbon_Patient();

	if(bHasEMR) {
		InitializeRibbon_Preview();
		InitializeRibbon_Photos();
	}
	
	InitializeRibbon_Merge();

	InitializeRibbon_View();

	InitializeRibbon_QAT();

	InitializeRibbonTabButtons();
}

void CEmrPatientFrameWnd::InitializeRibbon_MainPanel() 
{
	CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory("Main", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_SAVE, "Save", EmrIcons::Small::Save, EmrIcons::Large::Save));
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_SAVE_ALL, "Save All", EmrIcons::Small::SaveAll, EmrIcons::Large::SaveAll));
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_SAVE_AND_CLOSE, "Save and Close", EmrIcons::Small::SaveAndClose, EmrIcons::Large::SaveAndClose));		
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_NEW_EMN, "New EMN", EmrIcons::Small::EmnFolder, EmrIcons::Large::NewEmn));
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_CLOSE, "Close", EmrIcons::Small::Close, EmrIcons::Large::Close));
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_View()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("View", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Modules");

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_PIC, "Procedure Information Center"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_HISTORY, "Patient History"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_LABS, "Patient Labs"));
	}

	// (j.armen 2012-07-02 11:32) - PLID 51313 - Hide these if we don't have emr
	if (g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Panes");

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_TREE_PANE, "Topic List"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_PREVIEW_PANE, "Preview Pane"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_PHOTOS_PANE, "Photos"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_CLASSIC_BUTTON_PANE, "Classic Buttons")); //(e.lally 2012-02-16) PLID 48065
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_MU_PROGRESS_PANE, "Meaningful Use Progress")); //(e.lally 2012-02-23) PLID 48016
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_GRAPHING_PANE, "Graphing")); // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Layout");

		pPanel->Add(new CNxRibbonButton(ID_EMR_RESET_LAYOUT, "Reset Layout", EmrIcons::Small::Refresh, EmrIcons::Large::Refresh));
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_Edit()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("Edit", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Data");

		CNxRibbonButton* pSave = new CNxRibbonButton(ID_EMR_SAVE, "Save", EmrIcons::Small::Save, EmrIcons::Large::Save);
		pSave->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pSave);
		pPanel->Add(new CNxRibbonButton(ID_EMR_SAVE_ALL, "Save All", EmrIcons::Small::SaveAll, EmrIcons::Large::SaveAll));
		pPanel->Add(new CNxRibbonButton(ID_EMR_SAVE_AND_CLOSE, "Save and Close", EmrIcons::Small::SaveAndClose, EmrIcons::Large::SaveAndClose));		
		pPanel->Add(new CNxRibbonButton(ID_EMR_CLOSE, "Close", EmrIcons::Small::Close, EmrIcons::Large::Close));
	}

	// (j.armen 2012-07-02 15:19) - PLID 51313 - Hide all of these when we do not have EMR
	if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) 
	{
		{
			CNxRibbonPanel* pPanel = pCategory->AddPanel("Topics");

			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			CNxRibbonButton* pAddSubtopic = (new CNxRibbonButton(ID_EMR_NEW_SUBTOPIC, "Add Subtopic", EmrIcons::Small::AddSubtopic, EmrIcons::Large::AddSubtopic));
			pAddSubtopic->SetAlwaysLargeImage(TRUE);
			pPanel->Add(pAddSubtopic);
			pPanel->Add(new CNxRibbonButton(ID_EMR_NEW_TOPIC, "Add Topic", EmrIcons::Small::AddTopic, EmrIcons::Large::AddTopic));
			pPanel->Add(new CNxRibbonButton(ID_EMR_IMPORT_SUBTOPICS, "Import Subtopics", EmrIcons::Small::ImportSubtopic, EmrIcons::Large::ImportSubtopic));
			pPanel->Add(new CNxRibbonButton(ID_EMR_IMPORT_TOPICS, "Import Topics", EmrIcons::Small::AddTopic, EmrIcons::Large::ImportTopic));
		}

		{
			CNxRibbonPanel* pPanel = pCategory->AddPanel("Items");

			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			CNxRibbonButton* pAddItem = new CNxRibbonButton(ID_EMR_ADD_ITEM, "Add Item", EmrIcons::Small::Item, EmrIcons::Large::AddItem);
			pAddItem->SetAlwaysLargeImage(TRUE);
			pPanel->Add(pAddItem);
			pPanel->Add(new CNxRibbonButton(ID_EMR_ADD_IMAGE, "Add Image", EmrIcons::Small::ImageItem, EmrIcons::Large::None));

			// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
			{
				std::auto_ptr<CNxRibbonButton> pNew(new CNxRibbonButton(ID_EMR_ADD_SIGNATURE, "Add Signature", EmrIcons::Small::SignatureItem, EmrIcons::Large::None));

				pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_YOUR_SIGNATURE, "Add Your Signature", EmrIcons::Small::SignatureItem, EmrIcons::Large::None));
				pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_OTHER_USERS_SIGNATURE, "Add Another User's Signature", EmrIcons::Small::SignatureItem, EmrIcons::Large::None));

				pPanel->Add(pNew.release());
			}

			pPanel->Add(new CNxRibbonButton(ID_EMR_ADD_TEXT_MACRO, "Add Text Macro", EmrIcons::Small::TextItem, EmrIcons::Large::None));
		}

		// (a.walling 2013-06-12 10:37) - PLID 52524 - The EMR Description field is in a very inconvenient place and requires clients to click in a few places in order to reach it.
		{
			CNxRibbonPanel* pPanel = pCategory->AddPanel("Descriptions");

			pPanel->SetJustifyColumns(TRUE);

			CNxRibbonEditEx* pEmrDescription = new CNxRibbonEditEx(ID_EMR_DESCRIPTION, 200, "EMR");
			pEmrDescription->SetLimitText(1000);
			pPanel->Add(pEmrDescription);

			// (a.walling 2012-06-11 12:55) - PLID 50937 - Support line breaks
			CNxRibbonEditEx* pEmnDescription = new CNxRibbonEditEx(ID_EMR_EMN_DESCRIPTION, 200, "EMN");
			pEmnDescription->SetAllowLineBreaks();
			pEmnDescription->SetLimitText(255);
			pPanel->Add(pEmnDescription);
		}
		
		{
			CNxRibbonPanel* pPanel = pCategory->AddPanel("EMNs");

			CNxRibbonButton* pAddEMN = new CNxRibbonButton(ID_EMR_NEW_EMN, "Add EMN", EmrIcons::Small::EmnFolder, EmrIcons::Large::NewEmn);
			pAddEMN->SetAlwaysLargeImage(TRUE);
			pPanel->Add(pAddEMN);
		}

		{
			CNxRibbonPanel* pPanel = pCategory->AddPanel("Add New");

			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			pPanel->Add(new CNxRibbonButton(ID_EMR_NEW_TODO, "To-Do", EmrIcons::Small::Task, EmrIcons::Large::Task));			
			pPanel->Add(new CNxRibbonButton(ID_EMR_NEW_RECORDING, "Recording", EmrIcons::Small::Recording, EmrIcons::Large::Recording));
			//pPanel->Add(new CNxRibbonButton(ID_EMR_NEW_CHARGE, "Charge", EmrIcons::Small::TextUndo, EmrIcons::Large::Windows));
			//pPanel->Add(new CNxRibbonButton(ID_EMR_NEW_DIAG_CODE, "Diag Code", EmrIcons::Small::TextUndo, EmrIcons::Large::Windows));
		}

		// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
		{
			CNxRibbonPanel* pPanel = pCategory->AddPanel("Problems");

			pPanel->SetCenterColumnVert(TRUE);
			
			pPanel->Add(new CNxRibbonButton(ID_EMR_SHOW_PROBLEMS, "Show Problems", EmrIcons::Small::Problem, EmrIcons::Large::Problem));
			
			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu

			{
				std::auto_ptr<CNxRibbonButton> pNew(new CNxRibbonButton(ID_EMR_ADD_NEW_PROBLEM, "Create New", EmrIcons::Small::Add));
				pNew->SetDefaultCommand(FALSE);

				pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_NEW_PROBLEM_TO_EMR, "for this EMR", EmrIcons::Small::Emr));
				pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_NEW_PROBLEM_TO_EMN, "for this EMN", EmrIcons::Small::EmnFolder));
				pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_NEW_PROBLEM_TO_TOPIC, "for this Topic", EmrIcons::Small::Topic));

				pPanel->Add(pNew.release());
			}

			{
				std::auto_ptr<CNxRibbonButton> pExisting(new CNxRibbonButton(ID_EMR_LINK_PROBLEM, "Link Existing", EmrIcons::Small::Link));
				pExisting->SetDefaultCommand(FALSE);

				pExisting->AddSubItem(new CNxRibbonButton(ID_EMR_LINK_PROBLEM_TO_EMR, "to this EMR", EmrIcons::Small::Emr));
				pExisting->AddSubItem(new CNxRibbonButton(ID_EMR_LINK_PROBLEM_TO_EMN, "to this EMN", EmrIcons::Small::EmnFolder));
				pExisting->AddSubItem(new CNxRibbonButton(ID_EMR_LINK_PROBLEM_TO_TOPIC, "to this Topic", EmrIcons::Small::Topic));
				
				pPanel->Add(pExisting.release());
			}
		}
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_EMN()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("EMN", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Access");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		CNxRibbonButton* pWriteAccessButton = new CNxRibbonButton(ID_EMR_EMN_WRITE_ACCESS, "Writable", EmrIcons::Small::WriteAccess, EmrIcons::Large::WriteAccess);
		pWriteAccessButton->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pWriteAccessButton);
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Status");

		pPanel->SetJustifyColumns(TRUE);
		pPanel->SetCenterColumnVert(TRUE);
		
		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_EMN_STATUS, FALSE, 75, "Status"));

		// (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
		CNxRibbonAutoComboBox* pEmrStatus = new CNxRibbonAutoComboBox(ID_EMR_STATUS, FALSE, 75, "EMR Status");
		pPanel->Add(pEmrStatus);
		pEmrStatus->AddItem("Open", 0);
		pEmrStatus->AddItem("Closed", 1);

		// (a.walling 2012-11-27 10:07) - PLID 53892 - CNxRibbonSeparator - separate without an unsightly line
		pPanel->Add(new CNxRibbonSeparator);

		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_EMN_CHART, FALSE, 75, "Chart"));
		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_EMN_CATEGORY, FALSE, 75, "Category"));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Dates");

		pPanel->SetJustifyColumns(TRUE);
		
		pPanel->Add(new CNxRibbonDateTime(ID_EMR_EMN_DATE, 150, "Date"));
		pPanel->Add(new CNxRibbonEditEx(ID_EMR_EMN_DATE_CREATED, 150, "Created"));
		pPanel->Add(new CNxRibbonEditEx(ID_EMR_EMN_DATE_MODIFIED, 150, "Modified"));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Info");

		pPanel->SetJustifyColumns(TRUE);

		CNxRibbonEditEx* pEmrDescription = new CNxRibbonEditEx(ID_EMR_DESCRIPTION, 200, "EMR");
		pEmrDescription->SetLimitText(1000);
		pPanel->Add(pEmrDescription);

		// (a.walling 2012-06-11 12:55) - PLID 50937 - Support line breaks
		CNxRibbonEditEx* pEmnDescription = new CNxRibbonEditEx(ID_EMR_EMN_DESCRIPTION, 200, "EMN");
		pEmnDescription->SetAllowLineBreaks();
		pEmnDescription->SetLimitText(255);
		pPanel->Add(pEmnDescription);
		// (a.walling 2012-04-09 09:03) - PLID 49515 - Location
		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_EMN_LOCATION, FALSE, 200 - 15, "Location"));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Providers");

		pPanel->SetJustifyColumns(TRUE);
		//pPanel->EnableLaunchButton(ID_EMR_EMN_OTHER_PROVIDERS, EmrIcons::Small::Erase);

		CNxRibbonCheckList* pMain = new CNxRibbonCheckList(ID_EMR_EMN_PROVIDER, 120, "Main");
		CNxRibbonCheckList* pSecondary = new CNxRibbonCheckList(ID_EMR_EMN_SECONDARY_PROVIDER, 120, "Secondary");
		CNxRibbonCheckList* pAssistant = new CNxRibbonCheckList(ID_EMR_EMN_ASSISTANT, 120, "Assistant");
		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		CNxRibbonButton* pOther = new CNxRibbonButton(ID_EMR_EMN_OTHER_PROVIDERS, "More", EmrIcons::Small::Command, EmrIcons::Large::Command);

		pPanel->Add(pMain);
		pPanel->Add(pSecondary);
		pPanel->Add(pAssistant);
		pPanel->Add(pOther);
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_MoreInfo()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("More Info", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("View");
		pPanel->Add(new CNxRibbonButton(ID_EMR_EMN_MORE_INFO, "More Info Tab", EmrIcons::Small::MoreInfo, EmrIcons::Large::MoreInfo));
		//TES 2/13/2014 - PLID 60749 - Codes button (uses same icon)
		pPanel->Add(new CNxRibbonButton(ID_EMR_EMN_CODES, "Codes Tab", EmrIcons::Small::MoreInfo, EmrIcons::Large::MoreInfo));
	}

	{
		// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Procedures");
		pPanel->SetCenterColumnVert(TRUE);
		pPanel->Add(new CMFCRibbonLabel("Selected Procedures"));
		pPanel->Add(new CNxRibbonCheckList(ID_EMR_EMN_PROCEDURES, 200, ""));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Notes");
		pPanel->Add(new CNxRibbonMultiLineEditEx(ID_EMR_EMN_NOTES, 150, ""));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Other");

		// (a.walling 2013-02-13 10:39) - PLID 55143 - Emr Appointment linking - UI
		pPanel->Add(new CNxRibbonButton(ID_EMR_EMN_APPOINTMENT, "Linked Appointment", EmrIcons::Small::LinkedAppointment, EmrIcons::Large::LinkedAppointment));
		pPanel->Add(new CNxRibbonButton(ID_EMR_SHOW_CONFIDENTIAL_INFO, "Confidential Info", EmrIcons::Small::ConfidentialInfo, EmrIcons::Large::ConfidentialInfo));		
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Auditing");

		pPanel->SetJustifyColumns(TRUE);
		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		pPanel->Add(new CNxRibbonButton(ID_EMR_SHOW_EMN_AUDIT_HISTORY, "EMN History", EmrIcons::Small::EmnHistory, EmrIcons::Large::EmnHistory));
		pPanel->Add(new CNxRibbonButton(ID_EMR_SHOW_EMR_AUDIT_HISTORY, "EMR History", EmrIcons::Small::EmrHistory, EmrIcons::Large::EmrHistory));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("E/M Checklist");

		pPanel->SetJustifyColumns(TRUE);

		pPanel->Add(new CMFCRibbonLabel("Visit Type"));

		// (b.spivey, March 07, 2012) - PLID 48581 - Combo box for the visit types. This combo will hold the name and ID. 
		CNxRibbonAutoComboBox* pVisitList = new CNxRibbonAutoComboBox(ID_EMR_EM_VISITTYPE, FALSE, 128, "");

		// (b.spivey, March 07, 2012) - PLID 48581 - Grab them all, there is really no user defined filtering to do here. 
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Name FROM EMRVisitTypesT WHERE Inactive <> 1"); 

		// (b.spivey, March 21, 2012) - PLID 48581 - Whether or not we have records, we need to add this to the list. 
		//	 Otherwise we have no way of selecting "none." 
		pVisitList->AddItem("<No type selected>", (DWORD_PTR)-1); 

		if(!rs->eof) {
			while (!rs->eof) {
				long nID = AdoFldLong(rs, "ID");
				CString strName = AdoFldString(rs, "Name");
				pVisitList->AddItem(strName, (DWORD_PTR)nID);
				rs->MoveNext();
			}
		}

		// (b.spivey, March 07, 2012) - PLID 48581 - Add the list to this.  
		pPanel->Add(pVisitList); 
		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		pPanel->Add(new CNxRibbonButton(ID_EMR_EM_CHECKLIST, "Open Checklist", EmrIcons::Small::Checklist));
	}

	// (r.gonet 08/03/2012) - PLID 51027
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Charges");
		pPanel->Add(new CNxRibbonButton(ID_EMR_WOUND_CARE_AUTO_CODING, "Wound Care Coding", EmrIcons::Small::WoundCare, EmrIcons::Large::WoundCare));
	}

	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	CNxRibbonPanel* pPanel = pCategory->AddPanel("ASC");
	{
		pPanel->SetJustifyColumns(TRUE);
		pPanel->SetCenterColumnVert(TRUE);

		pPanel->Add(new CNxRibbonTime(ID_EMR_ASC_ADMISSION_TIME, 100, "Admission Time"));
		pPanel->Add(new CNxRibbonTime(ID_EMR_ASC_DISCHARGE_TIME, 100, "Discharge Time"));

		CNxRibbonAutoComboBox* pStatus = new CNxRibbonAutoComboBox(ID_EMR_ASC_DISCHARGE_STATUS, FALSE, 150, "Discharge Status");
		pStatus->AddItem("<No Status>", (DWORD_PTR)-1);
		pPanel->Add(pStatus);

	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_Patient()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("Patient", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Info");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		pPanel->Add(new CNxRibbonButton(ID_EMR_PATIENT_SUMMARY, "Summary", EmrIcons::Small::Summary, EmrIcons::Large::Summary));
	}


	// (a.walling 2012-06-11 09:23) - PLID 50921 - Patient demographics: name, age, gender	
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Demographics");
		pPanel->SetJustifyColumns(TRUE);

		pPanel->Add(new CNxRibbonEditEx(ID_EMR_EMN_PATIENT_NAME, 175, "Name"));

		{
			// (a.walling 2012-10-01 09:11) - PLID 52932 - Use CNxRibbonButtonsGroup
			CNxRibbonButtonsGroup* pGroup = new CNxRibbonButtonsGroup();

			pGroup->AddButton(new CNxRibbonEditEx(ID_EMR_EMN_PATIENT_GENDER, 85, "Gender"));
			pGroup->AddButton(new CNxRibbonEditEx(ID_EMR_EMN_PATIENT_AGE, 57, "Age"));

			pPanel->Add(pGroup);
		}

		pPanel->Add(new CNxRibbonButton(ID_EMR_UPDATE_DEMOGRAPHICS, "Update Demographics", EmrIcons::Small::UpdateDemographics));
	}
	
	// (j.jones 2012-07-09 10:38) - PLID 51433 - Hide E-Rx if they don't have full EMR, because E-Rx inside this
	// window is dependent on EMR being in use. Also, E-Rx requires the NewCrop license, so hide the button if it
	// is unlicensed, which is what we used to do in the old EMR.
	// (j.jones 2012-07-09 10:38) - PLID 51433 - Drug Interactions requires FirstDataBank. But Drug Interactions inside
	// EMR requires the EMR license, so hide it if they don't have full EMR (must like we do for E-Rx).
	if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) 
	{
		//always add this panel
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Medications");

		CLicense::EPrescribingType eRxType = g_pLicense->HasEPrescribing(CLicense::cflrSilent);		
		CString strERxName = "e-Rx";
		if(eRxType == CLicense::eptNone) {
			//call this "Medications" if no E-Rx license of any sort
			strERxName = "Medications";
		}
			
		pPanel->Add(new CNxRibbonButton(ID_EMR_VIEW_EPRESCRIBING, strERxName, EmrIcons::Small::eRx, EmrIcons::Large::eRx));	
	
		// (j.jones 2012-09-26 14:18) - PLID 52879 - added drug interaction, FDB license only
		// (j.fouts 2013-05-30 09:47) - PLID 56807 - Drug Interactions is now tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts) {
			pPanel->Add(new CNxRibbonButton(ID_EMR_VIEW_DRUGINTERACTIONS, "Drug Interactions", EmrIcons::Small::DrugInteractions, EmrIcons::Large::DrugInteractions));
		}
	}
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_Preview()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddContextCategory("Preview", "Preview Pane", EmrContextCategory::PreviewPane, AFX_CategoryColor_Violet, IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Printing");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		CNxRibbonButton* pPrintPreview = new CNxRibbonButton(ID_EMR_PREVIEW_PANE_PRINT_PREVIEW, "Print Preview", EmrIcons::Small::PrintPreview, EmrIcons::Large::PrintPreview);
		pPrintPreview->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pPrintPreview);
		pPanel->Add(new CNxRibbonButton(ID_EMR_PREVIEW_PANE_PRINT, "Print to Printer", EmrIcons::Small::Print, EmrIcons::Large::Print));
		pPanel->Add(new CNxRibbonButton(ID_EMR_PREVIEW_PANE_PRINT_MULTIPLE, "Print Multiple EMNs", EmrIcons::Small::Print, EmrIcons::Large::Print));
	}

	// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane options
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Options");

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_PREVIEW_PANE_AUTO_SCROLL, "Auto-Scroll"));
		pPanel->Add(new CNxRibbonButton(ID_EMR_PREVIEW_PANE_CONFIGURE, "Configure", EmrIcons::Small::Settings));
	}
}


//(e.lally 2012-04-12) PLID 49566 - Photos pane
// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_Photos() 
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddContextCategory("Photos", "Photos", EmrContextCategory::PhotosPane, AFX_CategoryColor_Violet, IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	//(e.lally 2012-04-24) PLID 49637 - Photos pane display
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Display");

		//A quick way to show/hide the text labels under the Photos
		m_bPhotoDisplayLabels = TRUE;
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_PHOTO_DISPLAY_LABELS, "Show Thumbnail Labels"));
	}

	//Photos pane filtering
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Filter");

		pPanel->SetJustifyColumns(TRUE);

		CNxRibbonAutoComboBox* pCategory = new CNxRibbonAutoComboBox(ID_EMR_PHOTO_FILTER_CATEGORY, FALSE, 120, "Category");
		CNxRibbonAutoComboBox* pProcedure = new CNxRibbonAutoComboBox(ID_EMR_PHOTO_FILTER_PROCEDURE, FALSE, 120, "Procedure");
		pPanel->Add(pCategory);
		pPanel->Add(pProcedure);

		//Add special selections for all and no category
		pCategory->AddItem("<All Categories>", (DWORD_PTR)ID_COMBO_FILTER_ALL); 
		pCategory->AddItem("<No Category>", (DWORD_PTR)ID_COMBO_FILTER_NO_KEY); 
		pCategory->SelectItem((DWORD_PTR)ID_COMBO_FILTER_ALL);

		//Add special selections for all and no procedure
		pProcedure->AddItem("<All Procedures>", (DWORD_PTR)ID_COMBO_FILTER_ALL); 
		pProcedure->AddItem("<No Procedure>", (DWORD_PTR)ID_COMBO_FILTER_NO_KEY); 
		pProcedure->SelectItem((DWORD_PTR)ID_COMBO_FILTER_ALL);
	}

	//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Sort");

		CNxRibbonAutoComboBox* pSort = new CNxRibbonAutoComboBox(ID_EMR_PHOTO_SORT, FALSE, 100, "Sort By");
		pPanel->Add(pSort);

		pSort->AddItem("Attach Date", (DWORD_PTR)ID_EMR_PHOTO_SORT_ATTACHDATE);
		pSort->AddItem("Category", (DWORD_PTR)ID_EMR_PHOTO_SORT_CATEGORY);
		pSort->AddItem("Note", (DWORD_PTR)ID_EMR_PHOTO_SORT_NOTE);
		pSort->AddItem("Procedure", (DWORD_PTR)ID_EMR_PHOTO_SORT_PROCEDURE);
		//(e.lally 2012-04-16) PLID 39543 - Added service date
		pSort->AddItem("Service Date", (DWORD_PTR)ID_EMR_PHOTO_SORT_SERVICEDATE);
		pSort->AddItem("Staff", (DWORD_PTR)ID_EMR_PHOTO_SORT_STAFF);

		//(e.lally 2012-04-25) PLID 49634 - Remember last sort order
		UINT nCmd = GetRememberedPhotoSortCmd();
		m_bPhotoSortAscending = GetRemotePropertyInt("PhotoViewerSortCriteriaOrder", 0, 0, GetCurrentUserName(), true) == 0 ? TRUE : FALSE;	//0 = asc, 1 = desc

		pSort->SelectItem((DWORD_PTR)nCmd);

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_PHOTO_SORT_ASCENDING, "Descending"));
	}

	//(e.lally 2012-04-17) PLID 49636 - Photos pane group by
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Tab Grouping");

		CNxRibbonAutoComboBox* pGroup = new CNxRibbonAutoComboBox(ID_EMR_PHOTO_GROUP, FALSE, 100, "Group By");
		pPanel->Add(pGroup);

		pGroup->AddItem("", (DWORD_PTR)ID_COMBO_FILTER_ALL); 
		pGroup->AddItem("Category", (DWORD_PTR)ID_EMR_PHOTO_GROUP_CATEGORY);
		pGroup->AddItem("Procedure", (DWORD_PTR)ID_EMR_PHOTO_GROUP_PROCEDURE);

		//(e.lally 2012-04-30) PLID 49636 - remember last grouping
		long nTabType = GetRemotePropertyInt("EmrPhotoTabType", ID_COMBO_FILTER_ALL, 0, GetCurrentUserName(), true);
		pGroup->SelectItem((DWORD_PTR)nTabType);
	}
}

// (a.walling 2012-04-30 12:47) - PLID 49832 - Merging
// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_Merge()
{
	// (a.walling 2012-10-01 09:15) - PLID 52119 - Merge category reorganized significantly, adding Single EMN merge

	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("Merge", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);
	
	// (j.armen 2012-07-02 15:19) - PLID 51313 - Hide when we do not have EMR
	if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Single EMN");

		pPanel->SetCenterColumnVert();

		// (a.walling 2012-09-11 11:16) - PLID 52581 - Specifically for the templates, this splits the filename from the path
		// and displays the path right-aligned in a second column. The width can now be smaller too.

		pPanel->Add(new CMFCRibbonLabel("Choose a Template"));
		pPanel->Add(new CNxRibbonMergeTemplateComboBox(ID_EMR_EMN_MERGE_TEMPLATE_COMBO, FALSE, 180));

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		
		{
			// (a.walling 2012-10-01 09:11) - PLID 52932 - Use CNxRibbonButtonsGroup
			// (a.walling 2012-09-11 11:16) - PLID 52581 - Also group the template edit/copy/new buttons together
			std::auto_ptr<CNxRibbonButtonsGroup> pGroup(new CNxRibbonButtonsGroup); 

			pGroup->AddButton(new CNxRibbonButton(ID_EMR_EMN_MERGE_EDIT_TEMPLATE, "Edit Templates", EmrIcons::Small::Command, EmrIcons::None));
			pGroup->AddButton(new CNxRibbonButton(ID_EMR_EMN_MERGE_TEMPLATE_MAKE_DEFAULT, "Make Default", EmrIcons::Small::Check, EmrIcons::None));

			pPanel->Add(pGroup.release());
		}
		
		{
			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			std::auto_ptr<CNxRibbonButton> pMerge(new CNxRibbonButton(ID_EMR_EMN_MERGE_FROM_TEMPLATE, "Merge", EmrIcons::Small::Merge, EmrIcons::Large::Merge));
			pMerge->SetAlwaysLargeImage(TRUE);

			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_EMN_MERGE_FROM_TEMPLATE_TO_WORD, "To Word", EmrIcons::Small::Command, EmrIcons::None));
			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_EMN_MERGE_FROM_TEMPLATE_TO_PRINTER, "To Printer", EmrIcons::Small::Command, EmrIcons::None));

			pPanel->Add(pMerge.release());
		}

		{
			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			std::auto_ptr<CNxRibbonButton> pMerge(new CNxRibbonButton(ID_EMR_EMN_MERGE_FROM_OTHER, "Merge Other", EmrIcons::Small::Merge, EmrIcons::Large::Merge));

			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_EMN_MERGE_FROM_OTHER_TO_WORD, "To Word", EmrIcons::Small::Command, EmrIcons::None));
			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_EMN_MERGE_FROM_OTHER_TO_PRINTER, "To Printer", EmrIcons::Small::Command, EmrIcons::None));

			pPanel->Add(pMerge.release());
		}
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("EMR Packet");

		pPanel->SetJustifyColumns();

		pPanel->Add(new CMFCRibbonLabel("Choose a Packet"));
		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_MERGE_PACKET_COMBO, FALSE));

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		pPanel->Add(new CNxRibbonButton(ID_EMR_MERGE_EDIT_PACKET, "Edit Packets", EmrIcons::Small::Command, EmrIcons::None));	
		
		{
			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			std::auto_ptr<CNxRibbonButton> pMerge(new CNxRibbonButton(ID_EMR_MERGE_FROM_PACKET, "Merge", EmrIcons::Small::Merge, EmrIcons::Large::Merge));
			pMerge->SetAlwaysLargeImage(TRUE);

			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_MERGE_FROM_PACKET_TO_WORD, "To Word", EmrIcons::Small::Command, EmrIcons::None));
			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_MERGE_FROM_PACKET_TO_PRINTER, "To Printer", EmrIcons::Small::Command, EmrIcons::None));

			pPanel->Add(pMerge.release());
		}

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_MERGE_OPTIONS_REVERSE_ORDER, "Reverse Order"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_MERGE_OPTIONS_SEPARATE_HISTORY, "Single History"));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Entire EMR");

		pPanel->SetJustifyColumns();

		// (a.walling 2012-09-11 11:16) - PLID 52581 - Specifically for the templates, this splits the filename from the path
		// and displays the path right-aligned in a second column. The width can now be smaller too.

		pPanel->Add(new CMFCRibbonLabel("Choose a Template"));
		pPanel->Add(new CNxRibbonMergeTemplateComboBox(ID_EMR_MERGE_TEMPLATE_COMBO, FALSE, 140));

		{
			// (a.walling 2012-10-01 09:11) - PLID 52932 - Use CNxRibbonButtonsGroup
			// (a.walling 2012-09-11 11:16) - PLID 52581 - Also group the template edit/copy/new buttons together
			std::auto_ptr<CNxRibbonButtonsGroup> pTemplateGroup(new CNxRibbonButtonsGroup); 

			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			pTemplateGroup->AddButton(new CNxRibbonButton(ID_EMR_MERGE_EDIT_TEMPLATE, "Edit", EmrIcons::Small::Command, EmrIcons::None));
			pTemplateGroup->AddButton(new CNxRibbonButton(ID_EMR_MERGE_COPY_TEMPLATE, "Copy", EmrIcons::Small::Command, EmrIcons::None));
			pTemplateGroup->AddButton(new CNxRibbonButton(ID_EMR_MERGE_NEW_TEMPLATE, "New", EmrIcons::Small::Command, EmrIcons::None));

			pPanel->Add(pTemplateGroup.release());
		}
		
		{
			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
			std::auto_ptr<CNxRibbonButton> pMerge(new CNxRibbonButton(ID_EMR_MERGE_FROM_TEMPLATE, "Merge", EmrIcons::Small::Merge, EmrIcons::Large::Merge));
			pMerge->SetAlwaysLargeImage(TRUE);

			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_MERGE_FROM_TEMPLATE_TO_WORD, "To Word", EmrIcons::Small::Command, EmrIcons::None));
			pMerge->AddSubItem(new CNxRibbonButton(ID_EMR_MERGE_FROM_TEMPLATE_TO_PRINTER, "To Printer", EmrIcons::Small::Command, EmrIcons::None));

			pPanel->Add(pMerge.release());
		}
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Options");
		
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_MERGE_OPTIONS_SAVE_TO_HISTORY, "Save to History"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_MERGE_OPTIONS_TO_PRINTER, "Direct to Printer"));

		// (j.armen 2012-07-02 15:20) - PLID 51313 - Hide when we do not ave EMR
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			pPanel->Add(new CNxRibbonCheckBox(ID_EMR_MERGE_OPTIONS_ALL_EMNS, "Use all EMNs"));
		}
	}

	// (a.walling 2013-11-15 11:16) - PLID 59517 - Care Summary, Clinical Summary in ribbon
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Summaries");

		pPanel->SetJustifyColumns();
		
		{
			std::auto_ptr<CNxRibbonButton> pSum(new CNxRibbonButton(ID_EMR_CARE_SUMMARY, "Summary of Care", EmrIcons::Small::Command, EmrIcons::Large::Command));
			//pSum->SetAlwaysLargeImage(TRUE);

			// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
			pSum->AddSubItem(new CNxRibbonButton(ID_EMR_CARE_SUMMARY_CUSTOMIZED, "Customized...", EmrIcons::Small::Command, EmrIcons::None));
			
			pPanel->Add(pSum.release());
		}

		{
			std::auto_ptr<CNxRibbonButton> pSum(new CNxRibbonButton(ID_EMR_CLINICAL_SUMMARY, "Clinical Summary", EmrIcons::Small::Command, EmrIcons::Large::Command));
			//pSum->SetAlwaysLargeImage(TRUE);

			// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
			pSum->AddSubItem(new CNxRibbonButton(ID_EMR_CLINICAL_SUMMARY_CUSTOMIZED, "Customized...", EmrIcons::Small::Command, EmrIcons::None));

			pPanel->Add(pSum.release());
		}
	}

	// (j.armen 2012-07-02 15:19) - PLID 51313 - Hide when we do not have EMR
	if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Actions");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		// we always refresh the template list upon dropdown
		//pPanel->Add(new CNxRibbonButton(ID_EMR_MERGE_REFRESH, "Refresh Templates", EmrIcons::Small::Refresh, EmrIcons::Refresh));			
		pPanel->Add(new CNxRibbonButton(ID_EMR_MERGE_ADVANCED_SETUP, "Advanced Options", EmrIcons::Small::Settings, EmrIcons::None));		
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrPatientFrameWnd::InitializeRibbon_QAT()
{
	// Add quick access toolbar commands:
	CList<UINT, UINT> lstQATCmds;

	lstQATCmds.AddTail(ID_EMR_SAVE);
	lstQATCmds.AddTail(ID_EMR_NEW_EMN);

	m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);
}

void CEmrPatientFrameWnd::InitializeStatusBar()
{
	m_wndStatusBar.AddElement(new CNxRibbonCheckBox(ID_EMR_EDIT_MODE, "Edit Mode"), "Edit Mode");

	CMFCRibbonStatusBarPane* pPane;
	
	pPane = new CMFCRibbonStatusBarPane(ID_EMR_STATUS_TOTAL_TIME_OPEN, "", TRUE, NULL, "Total: WW:WW:WW");
	m_wndStatusBar.AddExtendedElement(pPane, "Total Time");
	pPane->SetDescription("Time open total");


	pPane = new CMFCRibbonStatusBarPane(ID_EMR_STATUS_CURRENT_TIME_OPEN, "", TRUE, NULL, "Current: WW:WW:WW");
	m_wndStatusBar.AddExtendedElement(pPane, "Current Time");
	pPane->SetDescription("Time open this session");

	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	m_wndStatusBar.AddExtendedElement(CreateNavigationButtonsGroup().release(), "Topic Navigation");
}

void CEmrPatientFrameWnd::OnUpdateNewEMN(CCmdUI* pCmdUI)
{
	// (j.armen 2012-07-02 15:20) - PLID 49831 - Only enable when we have permission to do so
	if((g_pLicense->HasEMR(CLicense::cflrSilent) == 2)
		&& CheckCurrentUserPermissions(bioPatientEMR, sptCreate | sptWrite, FALSE, 0, TRUE, TRUE)) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
void CEmrPatientFrameWnd::OnUpdateEMRStatus(CCmdUI* pCmdUI)
{	
	BOOL bCanWriteToEMR = CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);

	CEMR* pEMR = GetActiveEMR();

	pCmdUI->Enable((pEMR && bCanWriteToEMR) ? TRUE : FALSE);

	long nStatus = -1;
	if (pEMR) {
		nStatus = pEMR->GetStatus();
	}

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;		
	
	if (-1 == nStatus) {
		pList->SelectItem(-1);
	} else {
		pList->SelectItem((DWORD_PTR)nStatus);
	}
}

// (a.walling 2012-05-18 17:18) - PLID 50546 - EMR Description
void CEmrPatientFrameWnd::OnUpdateEMRDescription(CCmdUI* pCmdUI)
{
	CEMR* pEMR = GetActiveEMR();
	if (!pEMR) {
		pCmdUI->Enable(FALSE);
		return;
	}
	
	pCmdUI->Enable(TRUE);

	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetEditText(pEMR->GetDescription());
		}
	}
}

void CEmrPatientFrameWnd::OnUpdateViewPhotosPane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_emrPhotosPane.IsVisible() ? TRUE : FALSE);
}

//(e.lally 2012-02-23) PLID 48016 - Handles updating the UI (ribbon checkbox) based on our state.
void CEmrPatientFrameWnd::OnUpdateViewMUProgressPane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_emrMUProgressPane.IsVisible() ? TRUE : FALSE);
}

// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane
void CEmrPatientFrameWnd::OnUpdateViewGraphingPane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_emrGraphingPane.IsVisible() ? TRUE : FALSE);
}

void CEmrPatientFrameWnd::OnViewPhotosPane()
{
	try {
		CDockablePane& pane(m_emrPhotosPane);

		if (pane.IsVisible()) {
			pane.ShowPane(FALSE, FALSE, FALSE);
		} else {
			pane.ShowPane(TRUE, FALSE, TRUE);
		}
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

 //(e.lally 2012-02-23) PLID 48016 - Shows or hides the MU progress pane
void CEmrPatientFrameWnd::OnViewMUProgressPane()
{
	try {
		CDockablePane& pane(m_emrMUProgressPane);

		if (pane.IsVisible()) {
			pane.ShowPane(FALSE, FALSE, FALSE);
		} else {
			pane.ShowPane(TRUE, FALSE, TRUE);
			// (e.lally 2012-03-14) PLID 48891 - Recalculate the results if this was not in use previously
			m_emrMUProgressPane.RecalculateMeasures();
		}
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane
void CEmrPatientFrameWnd::OnViewGraphingPane()
{
	try {
		CDockablePane& pane(m_emrGraphingPane);

		if (pane.IsVisible()) {
			pane.ShowPane(FALSE, FALSE, FALSE);
		} else {
			pane.ShowPane(TRUE, FALSE, TRUE);
		}
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status
void CEmrPatientFrameWnd::OnUpdateStatus(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;		
	
	EMNStatus curStatus = pEMN->GetStatus();

	if (-1 == curStatus.nID) {
		pList->SelectItem(-1);
	} else if (!pList->SelectItem((DWORD_PTR)curStatus.nID)) {
		pList->SelectItem(pList->AddItem(curStatus.strName, (DWORD_PTR)curStatus.nID));
	}
}

void CEmrPatientFrameWnd::OnUpdateNotes(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 10:57) - PLID 52629 - Disable entirely if no EMN
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
		// (j.armen 2012-11-14 09:46) - PLID 53687 - Set the NxEdit to be read only
		if (dynamic_ptr<CNxEdit> pNxEdit = pCmdUI->m_pOther) {
			pNxEdit->SetReadOnly(GetActiveEditableEMN() ? FALSE : TRUE);
		}
	}

	// (a.walling 2012-03-23 18:01) - PLID 49196 - Notes - ensure we set the EditText rather than the label if this is a ribbon item
	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		// (a.walling 2012-05-22 14:46) - PLID 50571 - Always use the best type to handle non-virtual overrides
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetReadOnly(GetActiveEditableEMN() ? FALSE : TRUE);
			if (pEMN) {
				pEdit->SetEditText(pEMN->GetNotes());
			}
		}
	}
}

// (a.walling 2012-04-09 09:03) - PLID 49515 - Location
void CEmrPatientFrameWnd::OnUpdateLocation(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;		
	
	// (a.walling 2012-04-09 09:03) - PLID 49515 - Location - Return const ref
	const EMNLocation& emnLocation = pEMN->GetLocation();

	if (-1 == emnLocation.nID) {
		pList->SelectItem(-1);
	} else if (!pList->SelectItem((DWORD_PTR)emnLocation.nID)) {
		pList->SelectItem(pList->AddItem(emnLocation.strName, (DWORD_PTR)emnLocation.nID));
	}
}

// (a.walling 2012-05-17 17:46) - PLID 50495 - EMR Date
void CEmrPatientFrameWnd::OnUpdateDate(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI;
	if (!pRibbonCmdUI) return;

	dynamic_ptr<CNxRibbonDateTime> pDt = pRibbonCmdUI->m_pUpdated;
	if (!pDt) return;

	if (pDt->GetDateTime() != pEMN->GetEMNDate()) {
		pDt->SetDateTime(pEMN->GetEMNDate());
	}
}

// (a.walling 2012-06-07 08:59) - PLID 50920 - Dates - Modified, Created
void CEmrPatientFrameWnd::OnUpdateDateCreated(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 10:57) - PLID 52629 - Disable entirely if no EMN
	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			if (CEMN* pEMN = GetActiveEMN()) {
				pCmdUI->Enable(TRUE);
				pEdit->SetReadOnly();
				COleDateTimeSpan dts = pEMN->GetEMNInputDate() - COleDateTime::GetCurrentTime();
				CString strRelativeSpan = DescribeRelativeDateTimeSpan(dts);
				pEdit->SetEditText(strRelativeSpan);
				pEdit->SetToolTipText("Created " + strRelativeSpan);
				pEdit->SetDescription("on " + FormatDateTimeForInterface(pEMN->GetEMNInputDate()));

				return;
			} else {
				pEdit->SetEditText("");
				pEdit->SetToolTipText("");
				pEdit->SetDescription("");
			}
		}
	}

	pCmdUI->Enable(FALSE);
}

// (a.walling 2012-06-07 08:59) - PLID 50920 - Dates - Modified, Created
void CEmrPatientFrameWnd::OnUpdateDateModified(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 10:57) - PLID 52629 - Disable entirely if no EMN
	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetReadOnly();
			if (CEMN* pEMN = GetActiveEMN()) {
				pCmdUI->Enable(TRUE);
				pEdit->SetReadOnly();
				COleDateTimeSpan dts = pEMN->GetEMNModifiedDate() - COleDateTime::GetCurrentTime();
				CString strRelativeSpan = DescribeRelativeDateTimeSpan(dts);
				pEdit->SetEditText(strRelativeSpan);
				pEdit->SetToolTipText("Last modified " + strRelativeSpan);
				pEdit->SetDescription("on " + FormatDateTimeForInterface(pEMN->GetEMNModifiedDate()));

				return;
			} else {
				pEdit->SetEditText("");
				pEdit->SetToolTipText("");
				pEdit->SetDescription("");
			}
		}
	}

	pCmdUI->Enable(FALSE);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnUpdateProvider(CCmdUI* pCmdUI)
{
	CEMN* pEMN = GetActiveEMN();
	//(e.lally 2012-05-07) PLID 48951 - Check the full Can Be Edited
	pCmdUI->Enable((pEMN && pEMN->CanBeEdited()) ? TRUE : FALSE);

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonCheckList* pChecklist = dynamic_cast<CNxRibbonCheckList*>(pRibbonCmdUI->m_pUpdated);
	if (!pChecklist) return;	

	std::set<DWORD> providers;

	if (pEMN) {
		int count = pEMN->GetProviderCount();
		for (int i = 0; i < count; ++i) {
			EMNProvider* pProvider = pEMN->GetProvider(i);
			
			if (!pChecklist->IsDroppedDown()) {
				using namespace PracData;
				const Provider& provider = Cached<ProvidersT>()->has<ID>(pProvider->nID);
				pChecklist->AddItem(provider.name.FormalFull(), (DWORD_PTR)provider.id);
			}

			providers.insert((DWORD)pProvider->nID);
		}
	}

	pChecklist->SetItemsChecked(providers);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnUpdateSecondaryProvider(CCmdUI* pCmdUI)
{
	CEMN* pEMN = GetActiveEMN();
	//(e.lally 2012-05-07) PLID 48951 - Check the full Can Be Edited
	pCmdUI->Enable((pEMN && pEMN->CanBeEdited()) ? TRUE : FALSE);


	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonCheckList* pChecklist = dynamic_cast<CNxRibbonCheckList*>(pRibbonCmdUI->m_pUpdated);
	if (!pChecklist) return;	

	std::set<DWORD> providers;

	if (pEMN) {
		int count = pEMN->GetSecondaryProviderCount();
		for (int i = 0; i < count; ++i) {
			EMNProvider* pProvider = pEMN->GetSecondaryProvider(i);
			
			if (!pChecklist->IsDroppedDown()) {
				using namespace PracData;
				const Provider& provider = Cached<ProvidersT>()->has<ID>(pProvider->nID);			
				pChecklist->AddItem(provider.name.FormalFull(), (DWORD_PTR)provider.id);
			}

			providers.insert((DWORD)pProvider->nID);
		}
	}

	pChecklist->SetItemsChecked(providers);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnUpdateAssistants(CCmdUI* pCmdUI)
{
	CEMN* pEMN = GetActiveEMN();
	//(e.lally 2012-05-07) PLID 48951 - Check the full Can Be Edited
	pCmdUI->Enable((pEMN && pEMN->CanBeEdited()) ? TRUE : FALSE);


	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonCheckList* pChecklist = dynamic_cast<CNxRibbonCheckList*>(pRibbonCmdUI->m_pUpdated);
	if (!pChecklist) return;	

	std::set<DWORD> technicians;

	if (pEMN) {
		int count = pEMN->GetTechnicianCount();
		for (int i = 0; i < count; ++i) {
			EMNProvider* pProvider = pEMN->GetTechnician(i);
			
			if (!pChecklist->IsDroppedDown()) {
				using namespace PracData;
				const User& user = Cached<UsersT>()->has<ID>(pProvider->nID);			
				pChecklist->AddItem(FormatString("%s - %s", user.userName, user.name.FormalFull()), (DWORD_PTR)user.id);
			}

			technicians.insert((DWORD)pProvider->nID);
		}
	}

	pChecklist->SetItemsChecked(technicians);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnUpdateOtherProviders(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
void CEmrPatientFrameWnd::OnUpdateProcedures(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
	
	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonCheckList* pChecklist = dynamic_cast<CNxRibbonCheckList*>(pRibbonCmdUI->m_pUpdated);
	if (!pChecklist) return;	

	std::set<DWORD> procedures;

	if (CEMN* pEMN = GetActiveEMN()) {
		int count = pEMN->GetProcedureCount();
		for (int i = 0; i < count; ++i) {
			EMNProcedure* pProcedure = pEMN->GetProcedure(i);
			
			if (!pChecklist->IsDroppedDown()) {
				using namespace PracData;
				const Procedure& procedure = Cached<ProcedureT>()->has<ID>(pProcedure->nID);
				pChecklist->AddItem(procedure.name, (DWORD_PTR)procedure.id);
			}

			procedures.insert((DWORD)pProcedure->nID);
		}
	}

	pChecklist->SetItemsChecked(procedures);
}

void CEmrPatientFrameWnd::OnUpdateNewTodo(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-04) PLID 48065 - Legacy behavior has this available when read-only too
	// (j.armen 2012-07-02 16:02) - PLID 49831 - Check for permission
	pCmdUI->Enable(CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && GetActiveEMN());
}

void CEmrPatientFrameWnd::OnUpdateNewRecording(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-04) PLID 48065 - Legacy behavior has this available when read-only too
	// (j.armen 2012-07-02 16:02) - PLID 49831 - Check for permission
	pCmdUI->Enable(CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && GetActiveEMN());
}

// (a.walling 2012-06-11 09:34) - PLID 50925
void CEmrPatientFrameWnd::OnUpdateNewCharge(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

// (a.walling 2012-06-11 09:34) - PLID 50925
void CEmrPatientFrameWnd::OnUpdateNewDiagCode(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

void CEmrPatientFrameWnd::OnUpdatePatientSummary(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (a.walling 2013-02-13 10:41) - PLID 55143 - Emr Appointment linking - UI
void CEmrPatientFrameWnd::OnUpdateLinkedAppointment(CCmdUI* pCmdUI)
{
	if (GetActiveEditableEMN()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info
void CEmrPatientFrameWnd::OnUpdateShowConfidentialInfo(CCmdUI* pCmdUI)
{
	if (GetActiveEMN() && CanAccess_ConfidentialInfo()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CEmrPatientFrameWnd::OnUpdateUpdateDemographics(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

// (a.walling 2012-06-11 09:23) - PLID 50921 - Patient demographics: name, age, gender
void CEmrPatientFrameWnd::OnUpdatePatientName(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 10:57) - PLID 52629 - Disable entirely if no EMN
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
	}

	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetReadOnly();
			if (pEMN) {
				pCmdUI->Enable(TRUE);
				pEdit->SetEditText(pEMN->GetPatientName());
			} else {
				pEdit->SetEditText("");
			}
		}
	}
}

// (a.walling 2012-06-11 09:23) - PLID 50921 - Patient demographics: name, age, gender
void CEmrPatientFrameWnd::OnUpdatePatientAge(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 10:57) - PLID 52629 - Disable entirely if no EMN
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
	}

	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetReadOnly();
			if (pEMN) {
				pCmdUI->Enable(TRUE);
				pEdit->SetEditText(pEMN->GetPatientAge());
			} else {
				pEdit->SetEditText("");
			}
		}
	}
}

// (a.walling 2012-06-11 09:23) - PLID 50921 - Patient demographics: name, age, gender
void CEmrPatientFrameWnd::OnUpdatePatientGender(CCmdUI* pCmdUI)
{
	// (a.walling 2013-05-02 10:57) - PLID 52629 - Disable entirely if no EMN
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
	}

	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetReadOnly();
			BYTE cPatientGender = 0;
			if (pEMN) {
				pCmdUI->Enable(TRUE);
				pEdit->SetEditText(pEMN->GetPatientGenderName());
			} else {
				pEdit->SetEditText("");
			}
		}
	}
}

void CEmrPatientFrameWnd::OnUpdateShowProblems(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEMN() ? TRUE : FALSE);
}

void CEmrPatientFrameWnd::OnUpdateAddNewProblem(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateAddNewProblemToEMR(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateAddNewProblemToEMN(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateAddNewProblemToTopic(CCmdUI* pCmdUI)
{
	// (a.walling 2012-07-12 09:32) - PLID 46078 - Active topic comes from the tree
	pCmdUI->Enable(GetActiveEditableEMN() && GetEmrTreeWnd()->GetActiveTopic() ? TRUE : FALSE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateLinkExistingProblem(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateLinkExistingProblemToEMR(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateLinkExistingProblemToEMN(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnUpdateLinkExistingProblemToTopic(CCmdUI* pCmdUI)
{
	// (a.walling 2012-07-12 09:32) - PLID 46078 - Active topic comes from the tree
	pCmdUI->Enable(GetActiveEditableEMN() && GetEmrTreeWnd()->GetActiveTopic() ? TRUE : FALSE);
}

// (b.spivey, March 07, 2012) - PLID 48581 - EM Checklist panel
void CEmrPatientFrameWnd::OnUpdateEMChecklist(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Check that the active EMN is also Editable
	// (j.jones 2013-04-22 16:32) - PLID 56372 - we now permit opening the checklist for a read-only EMN,
	// so that they can review decisions that were already made
	pCmdUI->Enable(GetActiveEMN() ? TRUE : FALSE);
}

void CEmrPatientFrameWnd::OnUpdateEMVisitType(CCmdUI* pCmdUI)
{
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
		return;
	}

	//(e.lally 2012-05-07) PLID 48951 - Check that the active EMN is also Editable
	pCmdUI->Enable(GetActiveEditableEMN() == NULL ? FALSE : TRUE);

	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if(dynamic_ptr<CNxRibbonAutoComboBox> pVisitList = pRibbonCmdUI->m_pUpdated) {
			if (-1 == pEMN->GetVisitTypeID()) {
				pVisitList->SelectItem(-1);
			} else {
				pVisitList->SelectItem((DWORD_PTR)pEMN->GetVisitTypeID());
			}
		}
	}
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
void CEmrPatientFrameWnd::OnUpdateShowEMRAuditHistory(CCmdUI* pCmdUI)
{
	if (-1 != GetActiveEMR()->GetSafeID() && CanAccess_EMRAuditHistory()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
void CEmrPatientFrameWnd::OnUpdateShowEMNAuditHistory(CCmdUI* pCmdUI)
{
	if (-1 != GetActiveEMN()->GetSafeID() && CanAccess_EMRAuditHistory()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::OnUpdateDischargeStatus(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;

	const EMNDischargeStatus& emnDischargeStatus = pEMN->GetDischargeStatus();

	if (-1 == emnDischargeStatus.nID) {
		pList->SelectItem(-1);
	}
	else if (!pList->SelectItem((DWORD_PTR)emnDischargeStatus.nID)) {
		pList->SelectItem(pList->AddItem(FormatString("%s - %s", emnDischargeStatus.strCode, emnDischargeStatus.strDesc), (DWORD_PTR)emnDischargeStatus.nID));
	}
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::OnUpdateAdmissionTime(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI;
	if (!pRibbonCmdUI) return;

	dynamic_ptr<CNxRibbonTime> pDt = pRibbonCmdUI->m_pUpdated;
	if (!pDt) return;

	if (pDt->GetTime() != pEMN->GetAdmissionTime()) {
		pDt->SetTime(pEMN->GetAdmissionTime());
	}
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::OnUpdateDischargeTime(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI;
	if (!pRibbonCmdUI) return;

	dynamic_ptr<CNxRibbonTime> pDt = pRibbonCmdUI->m_pUpdated;
	if (!pDt) return;

	if (pDt->GetTime() != pEMN->GetDischargeTime()) {
	pDt->SetTime(pEMN->GetDischargeTime());
	}
}

///

void CEmrPatientFrameWnd::OnNewEMN()
{
	try {
		GetEmrTreeWnd()->OnNewEmn();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
void CEmrPatientFrameWnd::OnEMRStatus()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMR* pEMR = GetActiveEMR();
		if (!pEMR) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_STATUS)) {
			long nCurSel = pList->GetCurSel();

			if (-1 == nCurSel) {
				return;
			}

			long nNewStatus = pList->GetItemData(nCurSel);

			if (nNewStatus == pEMR->GetStatus()) {
				return;
			}

			pEMR->SetStatus(nNewStatus);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrPatientFrameWnd::OnEMRDescription() // (a.walling 2012-05-18 17:18) - PLID 50546 - EMR Description
{
	try {
		if (CEMR* pEMR = GetActiveEMR()) {
			// (a.walling 2012-05-22 14:46) - PLID 50571 - Always use the best type to handle non-virtual overrides
			if (dynamic_ptr<CNxRibbonEditEx> pEdit = m_wndRibbonBar.FindByID(ID_EMR_DESCRIPTION)) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				pEMR->SetDescription(pEdit->GetEditText());
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-18 08:16) - PLID 49496 - Set the status
void CEmrPatientFrameWnd::OnStatus()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_EMN_STATUS)) {
			long nCurSel = pList->GetCurSel();

			if (-1 == nCurSel) {
				return;
			}

			EMNStatus newStatus;
			newStatus.nID = pList->GetItemData(nCurSel);

			if (newStatus.nID == pEMN->GetStatus().nID) {
				return;
			}

			newStatus.strName = pList->GetItem(nCurSel);

			// (z.manning 2009-08-11 15:22) - PLID 24277 - New permission for locking EMNs
			if(newStatus.nID != 2 || CheckCurrentUserPermissions(bioPatientEMR, sptDynamic3)) {
				//this will prompt the user (if changing to locked), 
				//update the status on the EMN, and re-set this screen if they change their mind				
				GetEmrTreeWnd()->SendMessage(NXM_EMN_STATUS_CHANGING, (WPARAM)&newStatus, (LPARAM)pEMN);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrPatientFrameWnd::OnNotes()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		if (CEMN* pEMN = GetActiveEditableEMN()) {
			// (a.walling 2012-05-22 14:46) - PLID 50571 - Always use the best type to handle non-virtual overrides
			if (dynamic_ptr<CNxRibbonEditEx> pEdit = m_wndRibbonBar.FindByID(ID_EMR_EMN_NOTES)) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				pEMN->SetNotes(pEdit->GetEditText());
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-09 09:03) - PLID 49515 - Location
void CEmrPatientFrameWnd::OnLocation()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_EMN_LOCATION)) {
			long nCurSel = pList->GetCurSel();
			if (-1 != nCurSel) {
				long nLocationID = (long)pList->GetItemData(nCurSel);
				using namespace PracData;
				const Location& loc = Cached<LocationsT>()->has<ID>(nLocationID);
				pEMN->SetLocation(loc.id, loc.name, loc.logoImagePath, loc.logoWidth);
			}
		}	
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-05-17 17:46) - PLID 50495 - EMR Date
void CEmrPatientFrameWnd::OnDate()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		dynamic_ptr<CNxRibbonDateTime> pDt = m_wndRibbonBar.FindByID(ID_EMR_EMN_DATE);
		if (!pDt) return;

		if (pDt->GetDateTime() != pEMN->GetEMNDate()) {
			pEMN->UpdateEMNDate(pDt->GetDateTime(), this);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnProvider()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();

		if (!pEMN) return;

		if (CNxRibbonCheckList* pCheckList = dynamic_cast<CNxRibbonCheckList*>(m_wndRibbonBar.FindByID(ID_EMR_EMN_PROVIDER))) {
			CArray<long,long> arProviderIDs;
			pCheckList->GetAllChecked(arProviderIDs);
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			pEMN->SetProviders(arProviderIDs);
			pEMN->HandleDetailChange(NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnSecondaryProvider()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();

		if (!pEMN) return;

		if (CNxRibbonCheckList* pCheckList = dynamic_cast<CNxRibbonCheckList*>(m_wndRibbonBar.FindByID(ID_EMR_EMN_SECONDARY_PROVIDER))) {
			CArray<long,long> arProviderIDs;
			pCheckList->GetAllChecked(arProviderIDs);
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			pEMN->SetSecondaryProviders(arProviderIDs);
			pEMN->HandleDetailChange(NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnAssistants()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();

		if (!pEMN) return;

		if (CNxRibbonCheckList* pCheckList = dynamic_cast<CNxRibbonCheckList*>(m_wndRibbonBar.FindByID(ID_EMR_EMN_ASSISTANT))) {
			CArray<long,long> arTechnicianIDs;
			pCheckList->GetAllChecked(arTechnicianIDs);
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			pEMN->SetTechnicians(arTechnicianIDs);
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2009-05-11 17:59) - PLID 33688 - added other providers
// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
void CEmrPatientFrameWnd::OnOtherProviders()
{
	try {
		try {
			//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
			CEMN* pEMN = GetActiveEditableEMN();

			if (!pEMN) {
				return;
			}

			//create a temporary array
			CArray<EMNProvider*, EMNProvider*> arOtherProvs;
			pEMN->GetOtherProviders(arOtherProvs);
			CEditEMNOtherProvidersDlg dlg(&arOtherProvs, this);
			long nResult = dlg.DoModal();

			if (nResult == IDOK) {
				//copy the arOtherProvs into the main list
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				pEMN->SetOtherProviders(arOtherProvs);
			}

			foreach(EMNProvider* pProv, arOtherProvs) {
				delete pProv;
			}
			arOtherProvs.RemoveAll();
		}NxCatchAll(__FUNCTION__);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
void CEmrPatientFrameWnd::OnProcedure()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();

		if (!pEMN) return;

		if (CNxRibbonCheckList* pCheckList = dynamic_cast<CNxRibbonCheckList*>(m_wndRibbonBar.FindByID(ID_EMR_EMN_PROCEDURES))) {
			CArray<long,long> addedIDs;
			CArray<long,long> removedIDs;
			
			{
				CArray<long,long> arCurProcedureIDs;
				CArray<long,long> arNewProcedureIDs;

				pEMN->GetProcedureIDs(&arCurProcedureIDs);
				pCheckList->GetAllChecked(arNewProcedureIDs);

				foreach (long id, arNewProcedureIDs) {
					if (!boost::exists(arCurProcedureIDs, id)) {
						addedIDs.Add(id);
					}
				}
				foreach (long id, arCurProcedureIDs) {
					if (!boost::exists(arNewProcedureIDs, id)) {
						removedIDs.Add(id);
					}
				}
			}

			foreach (long id, removedIDs) {
				if(GetEmrTreeWnd()) {
					GetEmrTreeWnd()->SendMessage(NXM_PRE_DELETE_EMN_PROCEDURE, (WPARAM)id, (LPARAM)pEMN);
				}
				pEMN->RemoveProcedure(id);
				//Notify our parent
				if(GetEmrTreeWnd()) {
					// (z.manning, 10/05/2007) - PLID 27630 - We now also have a message for after a procedure has been removed.
					GetEmrTreeWnd()->SendMessage(NXM_POST_DELETE_EMN_PROCEDURE, (WPARAM)id, (LPARAM)pEMN);
					GetEmrTreeWnd()->SendMessage(NXM_EMN_CHANGED, (WPARAM)pEMN);
				}
			}
			
			foreach (long id, addedIDs) {
				using namespace PracData;
				EMNProcedure* pProc = pEMN->AddProcedure(id, Cached<ProcedureT>()->has<ID>(id).name);
				if(GetEmrTreeWnd()) {
					GetEmrTreeWnd()->SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProc, (LPARAM)pEMN);
				}
				//Notify our parent
				if(GetEmrTreeWnd()) {
					GetEmrTreeWnd()->SendMessage(NXM_EMN_CHANGED, (WPARAM)pEMN);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrPatientFrameWnd::OnNewTodo()
{
	try {
		//(e.lally 2012-02-16) PLID 48065 - Just call into the EmrTreeWnd
		GetEmrTreeWnd()->OnCreateToDo();
	} NxCatchAll(__FUNCTION__);
}

void CEmrPatientFrameWnd::OnNewRecording()
{
	try {
		//(e.lally 2012-02-16) PLID 48065 - Just call into the EmrTreeWnd
		GetEmrTreeWnd()->OnRecordAudio();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 09:34) - PLID 50925
void CEmrPatientFrameWnd::OnNewCharge()
{
	try {
	#ifdef _DEBUG
		MessageBox(__FUNCTION__ " -- not implemented");
	#endif
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 09:34) - PLID 50925
void CEmrPatientFrameWnd::OnNewDiagCode()
{
	try {
	#ifdef _DEBUG
		MessageBox(__FUNCTION__ " -- not implemented");
	#endif
	} NxCatchAll(__FUNCTION__);
}

void CEmrPatientFrameWnd::OnPatientSummary()
{
	try {
		if (CEMN* pEMN = GetActiveEMN()) {
			CPatientSummaryDlg dlg(this);
			dlg.m_nPatientID = GetActiveEMN()->GetParentEMR()->GetPatientID();
			dlg.m_strPatientName = GetExistingPatientName(dlg.m_nPatientID);
			//(e.lally 2012-05-01) PLID 49635 - Use IDYES to see if a quick link was clicked.
			//	If so, minimize the PIC.
			if(IDYES == dlg.DoModal()){
				SendMessage(NXM_EMR_MINIMIZE_PIC);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-02-13 10:41) - PLID 55143 - Emr Appointment linking - UI
void CEmrPatientFrameWnd::OnLinkedAppointment()
{
	try {
		CEMN* pEMN = GetActiveEditableEMN();
		if (!pEMN) return;

		if (boost::optional<long> newAppt = PromptForEmnAppointment(this, pEMN->GetParentEMR()->GetPatientID(), pEMN->GetEMNDate(), pEMN->GetAppointment().nID)) {
			// (a.walling 2013-02-14 13:53) - PLID 55143 - Confirm when removing an appointment link
			if (*newAppt == -1 && pEMN->GetAppointment().nID != -1) {
				if (IDYES != MessageBox("Are you sure you want to unlink this EMN from an appointment?", NULL, MB_YESNO | MB_ICONQUESTION)) {
					return;
				}
			}
			if (*newAppt != pEMN->GetAppointment().nID) {
				pEMN->SetAppointment(EMNAppointment(*newAppt));
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info
void CEmrPatientFrameWnd::OnShowConfidentialInfo()
{
	try {
		if (!CheckAccess_ConfidentialInfo()) {
			return;
		}

		CEMNConfidentialInfoDlg dlg(this);
		dlg.m_strConfidentialInfo = GetActiveEMN()->GetConfidentialInfo();
		//We want the content to be readable if we don't have access to write, but
		//	they can't change it.
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		dlg.m_bReadOnly = GetActiveEditableEMN() ? false : true;

		if(dlg.DoModal() == IDOK) {
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			GetActiveEMN()->SetConfidentialInfo(dlg.m_strConfidentialInfo);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 09:27) - PLID 50922 - Update patient demographics
void CEmrPatientFrameWnd::OnUpdateDemographics()
{
	try {
		if (!GetActiveEMN()) return;

		if(IDNO == MessageBox("This action will update the patient information on this EMN with the latest information from General 1.\n"
			"Are you sure you wish to do this?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		GetActiveEMN()->UpdatePatientDemographics(this);
	} NxCatchAll(__FUNCTION__);
}

#pragma TODO("e.lally - these problem list functions probably don't need to be public once we figure out the command message routing")
//(e.lally 2012-02-27) PLID 48016 - Moved code here from EmrTreeWnd
void CEmrPatientFrameWnd::OnShowProblems()
{
	try {
		if(GetEmrTreeWnd() == NULL || GetEmrTreeWnd()->GetEMR() == NULL){
			return;
		}
		CEMR* pEMR = GetEmrTreeWnd()->GetEMR();

		
		//now open the problem list for all problems on this EMR
		// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
		CArray<CEmrProblemLink*, CEmrProblemLink*> apEmrProblemLinks;
		pEMR->GetAllProblemLinks(apEmrProblemLinks);

		if(apEmrProblemLinks.GetSize() == 0) {
			MessageBox("There are no problems currently on this EMR.");
			return;
		}

		//close the current problem list, if there is one
		CMainFrame *pFrame = GetMainFrame();
		if(pFrame) {
			pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
		}

		CEMRProblemListDlg dlg(this);
		dlg.SetDefaultFilter(pEMR->GetPatientID(), eprtEmrEMR, pEMR->GetID(), pEMR->GetDescription());
		// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
		// (a.walling 2012-06-22 14:01) - PLID 51150 - Use tree wnd as message wnd
		dlg.LoadFromProblemList(GetEmrTreeWnd(), &apEmrProblemLinks);
		dlg.DoModal();

		//see if any problem changed, if so, mark the EMR as changed
		if(pEMR->HasChangedProblems()) {
			pEMR->SetUnsaved();
		}

		// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon at all times
		// here, because anything could have changed
		// (a.walling 2012-06-08 14:48) - PLID 48016 - Post the message to the treewnd, not ourselves
		GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnAddNewProblem()
{
	ASSERT(FALSE);
	PostMessage(WM_COMMAND, ID_EMR_ADD_NEW_PROBLEM);
}

//(e.lally 2012-02-27) PLID 48016 - Moved code here from EmrTreeWnd
// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnAddNewProblemToEMR()
{
	try {
		if(GetEmrTreeWnd() == NULL || GetEmrTreeWnd()->GetEMR() == NULL){
			return;
		}
		CEMR* pEMR = GetEmrTreeWnd()->GetEMR();

		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		//there are no EMR-level write access protections, so always allow adding new problems
		CEMRProblemEditDlg dlg(this);

		// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
		// (j.jones 2009-05-22 13:50) - PLID 34250 - renamed this function
		dlg.AddLinkedObjectInfo(-1, eprtEmrEMR, pEMR->GetDescription(), "", pEMR->GetID());
		dlg.SetPatientID(pEMR->GetPatientID());

		if (IDOK == dlg.DoModal()) {
			if (!dlg.ProblemWasDeleted()) {

				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);

				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
				dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
				// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
				// (b.spivey, October 22, 2013) - PLID 58677 - refactored to work in EMR. 
				// (s.tullis 2015-02-23 15:44) - PLID 64723 
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
				CEmrProblem *pProblem = pEMR->AllocateEmrProblem(dlg.GetProblemID(), pEMR->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
					dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(), dlg.GetProblemDoNotShowOnCCDA(), 
					dlg.GetProblemDoNotShowOnProblemPrompt());
				CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtEmrEMR, pEMR->GetID(), -1);
				pNewLink->UpdatePointersWithEMR(pEMR);
				pEMR->m_apEmrProblemLinks.Add(pNewLink);
				// Release our local reference to the problem
				pProblem->Release();

				pEMR->SetUnsaved();

				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				// (a.walling 2012-06-08 14:48) - PLID 48016 - Post the message to the treewnd, not ourselves
				GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnAddNewProblemToEMN()
{
	try {
		CEMN* pEMN = GetActiveEMN();

		if(pEMN == NULL) {
			return;
		}

		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
		// this is independent of locking
		if(!pEMN->IsWritable()) {
			//prompt, but do not attempt to take access, let the user decide what to do
			MessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
			return;
		}

		CEMRProblemEditDlg dlg(this);

		const BOOL bEMNIsLocked = (pEMN->GetStatus() == 2) ? TRUE : FALSE;
		if (bEMNIsLocked) {
			dlg.SetWriteToData(TRUE); // This will force the dialog to save the problem details to data
		}

		// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
		// (j.jones 2009-05-22 13:50) - PLID 34250 - renamed this function
		// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
		dlg.AddLinkedObjectInfo(-1, eprtEmrEMN, pEMN->GetDescription(), "", pEMN->GetID());
		dlg.SetPatientID(pEMN->GetParentEMR()->GetPatientID());

		if (IDOK == dlg.DoModal()) {
			if (!dlg.ProblemWasDeleted()) {

				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);

				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
				dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
				// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
				// (b.spivey, October 22, 2013) - PLID 58677 - refactored to work in EMR. 
				// (s.tullis 2015-02-23 15:44) - PLID 64723 
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
				CEmrProblem *pProblem = GetActiveEMR()->AllocateEmrProblem(dlg.GetProblemID(), GetActiveEMR()->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
					dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(), dlg.GetProblemDoNotShowOnCCDA(),
					dlg.GetProblemDoNotShowOnProblemPrompt());
				CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtEmrEMN, pEMN->GetID(), -1);
				pNewLink->UpdatePointersWithEMN(pEMN);
				pEMN->m_apEmrProblemLinks.Add(pNewLink);
				// Release our local reference to the problem
				pProblem->Release();

				if(!pEMN->IsLockedAndSaved()) {
					pEMN->SetUnsaved();
				}
				// (c.haag 2009-06-01 13:17) - PLID 34312 - We need to flag the EMR as unsaved because it is now the "manager" of problems
				GetActiveEMR()->SetUnsaved();

				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);
			}
		}			
	}
	NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnAddNewProblemToTopic()
{
	try {
		// (a.walling 2012-07-12 09:32) - PLID 46078 - Active topic comes from the tree
		CEMRTopic* pTopic = GetEmrTreeWnd()->GetActiveTopic();

		if (!pTopic) return;

		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
		// this is independent of locking
		if(!pTopic->GetParentEMN()->IsWritable()) {
			//prompt, but do not attempt to take access, let the user decide what to do
			MessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
			return;
		}

		CEMRProblemEditDlg dlg(this);

		const BOOL bEMNIsLocked = (pTopic->GetParentEMN()->GetStatus() == 2) ? TRUE : FALSE;
		if (bEMNIsLocked) {
			dlg.SetWriteToData(TRUE); // This will force the dialog to save the problem details to data
		}

		// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
		// (j.jones 2009-05-22 13:50) - PLID 34250 - renamed this function
		dlg.AddLinkedObjectInfo(-1, eprtEmrTopic, pTopic->GetName(), "", pTopic->GetID());
		dlg.SetPatientID(pTopic->GetParentEMN()->GetParentEMR()->GetPatientID());

		if (IDOK == dlg.DoModal()) {
			if (!dlg.ProblemWasDeleted()) {

				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);

				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
				dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				// (c.haag 2009-05-26 11:47) - PLID 34312 - Use the new EMR problem linking structure
				// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
				// (b.spivey, October 22, 2013) - PLID 58677 - refactored to work in EMR. 
				// (s.tullis 2015-02-23 15:44) - PLID 64723 
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
				CEmrProblem *pProblem = GetActiveEMR()->AllocateEmrProblem(dlg.GetProblemID(), GetActiveEMR()->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
					dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(),dlg.GetProblemDoNotShowOnCCDA(),
					dlg.GetProblemDoNotShowOnProblemPrompt());
				CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtEmrTopic, pTopic->GetID(), -1);
				pNewLink->UpdatePointersWithTopic(pTopic);
				pTopic->m_apEmrProblemLinks.Add(pNewLink);
				// Release our local reference to the problem
				pProblem->Release();

				if(!pTopic->GetParentEMN()->IsLockedAndSaved()) {
					pTopic->SetUnsaved();
				}
				// (c.haag 2009-06-01 13:17) - PLID 34312 - We need to flag the EMR as unsaved because it is now the "manager" of problems
				GetActiveEMR()->SetUnsaved();


				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnLinkExistingProblem()
{
	ASSERT(FALSE);
	PostMessage(WM_COMMAND, ID_EMR_LINK_PROBLEM_TO_EMR);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
//(e.lally 2012-02-27) PLID 48016 - Moved code here from EmrTreeWnd
void CEmrPatientFrameWnd::OnLinkExistingProblemToEMR()
{
	try {
		if(GetEmrTreeWnd() == NULL || GetEmrTreeWnd()->GetEMR() == NULL){
			return;
		}
		CEMR* pEMR = GetEmrTreeWnd()->GetEMR();
		
		CArray<CEmrProblem*,CEmrProblem*> aryAllProblems;
		CArray<CEmrProblem*,CEmrProblem*> aryEMRProblems;
		CArray<CEmrProblem*,CEmrProblem*> arySelectionsInMemory;
		CArray<long,long> arynSelectionsInData;
		int i;

		// We require problem create permissions to create new links
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		// Pull ALL problems, including deleted ones, because the chooser dialog will run a query
		// that must be filtered by both deleted and non-deleted problems in memory
		pEMR->GetAllProblems(aryAllProblems, TRUE);
		for (i=0; i < pEMR->m_apEmrProblemLinks.GetSize(); i++) {
			EnsureProblemInArray(aryEMRProblems, pEMR->m_apEmrProblemLinks[i]->GetProblem());
		}
		CEMRProblemChooserDlg dlg(aryAllProblems, aryEMRProblems, pEMR->GetPatientID(), this);
		if (dlg.HasProblems()) {
			if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
				// User chose at least one problem
				
				// First, go through the problems that already exist in memory (and owned
				// by this EMR), and create problem links for them.
				for (i=0; i < arySelectionsInMemory.GetSize(); i++) {
					CEmrProblemLink* pNewLink = new CEmrProblemLink(arySelectionsInMemory[i], -1, eprtEmrEMR, pEMR->GetID(), -1);
					pNewLink->UpdatePointersWithEMR(pEMR);
					pEMR->m_apEmrProblemLinks.Add(pNewLink);
				}
				// Next, go through the problems that exist in data and not in memory, create
				// problem objects for them, and then links for those. Here's the neat part:
				// If the problem is already in memory, but linked with nothing; then the EMR
				// is smart enough to just give you the problem already in memory when calling
				// the function to allocate it.
				// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
				// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID 
				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				// (s.tullis 2015-02-23 15:44) - PLID 64723 
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
				if (arynSelectionsInData.GetSize() > 0) {
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
						"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EMRProblemsT.PatientID, "
						"EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
						"FROM EMRProblemsT "
						"WHERE Deleted = 0 AND ID IN ({INTARRAY})", arynSelectionsInData);
					while (!prs->eof) {
						CEmrProblem* pNewProblem = pEMR->AllocateEmrProblem(prs->Fields);
						CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtEmrEMR, pEMR->GetID(), -1);
						pNewLink->UpdatePointersWithEMR(pEMR);
						pEMR->m_apEmrProblemLinks.Add(pNewLink);
						pNewProblem->Release();
						prs->MoveNext();
					}
				}
				pEMR->SetUnsaved();

				// Now update the interface
				// (a.walling 2012-06-08 14:48) - PLID 48016 - Post the message to the treewnd, not ourselves
				GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);

			}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
			else {
				// User changed their mind
			}
		} else {
			// Dialog has no visible problems
			MessageBox("There are no available problems to choose from.");
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnLinkExistingProblemToEMN()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if(pEMN == NULL) {
			return;
		}

		// We require problem create permissions to create new links
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		CArray<CEmrProblem*,CEmrProblem*> aryAllProblems;
		CArray<CEmrProblem*,CEmrProblem*> aryEMNProblems;
		CArray<CEmrProblem*,CEmrProblem*> arySelectionsInMemory;
		CArray<long,long> arynSelectionsInData;
		int i;

		// Pull ALL problems, including deleted ones, because the chooser dialog will run a query
		// that must be filtered by both deleted and non-deleted problems in memory
		GetActiveEMR()->GetAllProblems(aryAllProblems, TRUE);
		for (i=0; i < pEMN->m_apEmrProblemLinks.GetSize(); i++) {
			EnsureProblemInArray(aryEMNProblems, pEMN->m_apEmrProblemLinks[i]->GetProblem());
		}
		CEMRProblemChooserDlg dlg(aryAllProblems, aryEMNProblems, GetActiveEMR()->GetPatientID(), this);
		if (dlg.HasProblems()) {
			if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
				// User chose at least one problem
				
				// First, go through the problems that already exist in memory (and owned
				// by this EMR), and create problem links for them.
				for (i=0; i < arySelectionsInMemory.GetSize(); i++) {
					CEmrProblemLink* pNewLink = new CEmrProblemLink(arySelectionsInMemory[i], -1, eprtEmrEMN, pEMN->GetID(), -1);
					pNewLink->UpdatePointersWithEMN(pEMN);
					pEMN->m_apEmrProblemLinks.Add(pNewLink);
				}
				// Next, go through the problems that exist in data and not in memory, create
				// problem objects for them, and then links for those. Here's the neat part:
				// If the problem is already in memory, but linked with nothing; then the EMR
				// is smart enough to just give you the problem already in memory when calling
				// the function to allocate it.
				if (arynSelectionsInData.GetSize() > 0) {
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID
					// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
					// (s.tullis 2015-02-23 15:44) - PLID 64723
					// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
					ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
						"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EMRProblemsT.PatientID, "
						"EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
						"FROM EMRProblemsT "
						"WHERE Deleted = 0 AND ID IN ({INTARRAY})", arynSelectionsInData);
					while (!prs->eof) {
						CEmrProblem* pNewProblem = GetActiveEMR()->AllocateEmrProblem(prs->Fields);
						CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtEmrEMN, pEMN->GetID(), -1);
						pNewLink->UpdatePointersWithEMN(pEMN);
						pEMN->m_apEmrProblemLinks.Add(pNewLink);
						pNewProblem->Release();
						prs->MoveNext();
					}
				}
				pEMN->SetUnsaved();
				// (c.haag 2009-05-30 14:25) - PLID 34249 - The EMR is the memory manager
				// of EMR problems in memory; and therefore must be flagged unsaved.
				GetActiveEMR()->SetUnsaved();

				// (j.jones 2008-07-24 08:35) - PLID 30729 - refresh the EMR problem icon,
				// because anything could have changed
				GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);

			}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
			else {
				// User changed their mind
			}
		} else {
			// Dialog has no visible problems
			MessageBox("There are no available problems to choose from.");
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
void CEmrPatientFrameWnd::OnLinkExistingProblemToTopic()
{
	try {
		// (a.walling 2012-07-12 09:32) - PLID 46078 - Active topic comes from the tree
		CEMRTopic* pTopic = GetEmrTreeWnd()->GetActiveTopic();

		if(pTopic == NULL) {
			return;
		}

		// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
		// this is independent of locking
		if(!pTopic->GetParentEMN()->IsWritable()) {
			//prompt, but do not attempt to take access, let the user decide what to do
			MessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
			return;
		}

		// We require problem create permissions to create new links
		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		CArray<CEmrProblem*,CEmrProblem*> aryAllProblems;
		CArray<CEmrProblem*,CEmrProblem*> aryTopicProblems;
		CArray<CEmrProblem*,CEmrProblem*> arySelectionsInMemory;
		CArray<long,long> arynSelectionsInData;
		int i;

		// Pull ALL problems, including deleted ones, because the chooser dialog will run a query
		// that must be filtered by both deleted and non-deleted problems in memory
		GetActiveEMR()->GetAllProblems(aryAllProblems, TRUE);
		for (i=0; i < pTopic->m_apEmrProblemLinks.GetSize(); i++) {
			EnsureProblemInArray(aryTopicProblems, pTopic->m_apEmrProblemLinks[i]->GetProblem());
		}
		CEMRProblemChooserDlg dlg(aryAllProblems, aryTopicProblems, GetActiveEMR()->GetPatientID(), this);
		if (dlg.HasProblems()) {
			if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
				// User chose at least one problem
				
				// First, go through the problems that already exist in memory (and owned
				// by this EMR), and create problem links for them.
				for (i=0; i < arySelectionsInMemory.GetSize(); i++) {
					CEmrProblemLink* pNewLink = new CEmrProblemLink(arySelectionsInMemory[i], -1, eprtEmrTopic, pTopic->GetID(), -1);
					pNewLink->UpdatePointersWithTopic(pTopic);
					pTopic->m_apEmrProblemLinks.Add(pNewLink);
				}
				// Next, go through the problems that exist in data and not in memory, create
				// problem objects for them, and then links for those. Here's the neat part:
				// If the problem is already in memory, but linked with nothing; then the EMR
				// is smart enough to just give you the problem already in memory when calling
				// the function to allocate it.
				if (arynSelectionsInData.GetSize() > 0) {
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID
					// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
					// (s.tullis 2015-02-23 15:44) - PLID 64723
					// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
					ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
						"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EMRProblemsT.PatientID,  "
						"EMRProblemsT.CodeID , EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
						"FROM EMRProblemsT "
						"WHERE Deleted = 0 AND ID IN ({INTARRAY})", arynSelectionsInData);
					while (!prs->eof) {
						CEmrProblem* pNewProblem = GetActiveEMR()->AllocateEmrProblem(prs->Fields);
						CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtEmrTopic, pTopic->GetID(), -1);
						pNewLink->UpdatePointersWithTopic(pTopic);
						pTopic->m_apEmrProblemLinks.Add(pNewLink);
						pNewProblem->Release();
						prs->MoveNext();
					}
				}
				pTopic->SetUnsaved();
				// (c.haag 2009-06-01 12:58) - PLID 34249 - Because we added or modified an EMR problem or problem link, we must
				// flag the EMR as unsaved
				GetActiveEMR()->SetUnsaved();

				// (j.jones 2008-07-24 08:35) - PLID 30729 - refresh the EMR problem icon,
				// because anything could have changed
				GetEmrTreeWnd()->PostMessage(NXM_EMR_PROBLEM_CHANGED);

			}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
			else {
				// User changed their mind
			}
		} else {
			// Dialog has no visible problems
			MessageBox("There are no available problems to choose from.");
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, March 07, 2012) - PLID 48581 - EM Checklist panel, checklist button.
void CEmrPatientFrameWnd::OnShowEMChecklist()
{
	try {

		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(pTreeWnd == NULL || pTreeWnd->GetEMR() == NULL) {
			return;
		}

		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		// (j.jones 2013-04-22 16:32) - PLID 56372 - we now permit opening the checklist for a read-only EMN,
		// so that they can review decisions that were already made
		CEMN* pEMN = GetActiveEMN();

		// (b.spivey, March 07, 2012) - PLID 48581 - If the EMN is null we need to eject ourselves. 
		if(pEMN == NULL) {
			return; 
		}

		// (j.jones 2013-02-27 10:18) - PLID 55343 - moved E/M Checklist code to the treewnd
		pTreeWnd->OpenEMChecklist(pEMN);

	}NxCatchAll(__FUNCTION__); 
}

// (b.spivey, March 07, 2012) - PLID 48581 - EM Checklist panel, visit type combo
void CEmrPatientFrameWnd::OnSelectEMVisitType()
{	
	try {

		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();
		//(e.lally 2012-05-07) PLID 48951 - Check for null as a safety
		if(pEMN == NULL){
			return;
		}

		if(CNxRibbonAutoComboBox* pVisitList = dynamic_cast<CNxRibbonAutoComboBox*>(m_wndRibbonBar.FindByID(ID_EMR_EM_VISITTYPE))) {
			long nCurSel = pVisitList->GetCurSel(); 
			long nVisitID = pVisitList->GetItemData(nCurSel); 
			CString strVisitName = pVisitList->GetItem(nCurSel); 
			// (b.spivey, March 07, 2012) - PLID 48581 - if we reselect the same thing that really doesn't count as a change. 
			if(pEMN->GetVisitTypeID() != nVisitID) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				pEMN->SetVisitType(nVisitID, strVisitName); 
			}
		}

	} NxCatchAll(__FUNCTION__); 
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
void CEmrPatientFrameWnd::OnShowEMRAuditHistory()
{
	try {
		if (!CheckAccess_EMRAuditHistory()) {
			return;
		}

		CEMRAuditHistoryDlg dlg(this);
		dlg.m_nEMRID = GetActiveEMR()->GetSafeID();
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
void CEmrPatientFrameWnd::OnShowEMNAuditHistory()
{
	try {
		if (!CheckAccess_EMRAuditHistory()) {
			return;
		}

		CEMRAuditHistoryDlg dlg(this);
		dlg.m_nEMNID = GetActiveEMN()->GetSafeID();
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

//Classic button pane UI state
//(e.lally 2012-02-16) PLID 48065

void CEmrPatientFrameWnd::OnUpdateClassicBtnProblemList(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);

	//(e.lally 2012-04-11) PLID 49446 - Update the Problem button icon on idle
	//	We assume no future developers will add database accesses etc. to the functions used.
	if(m_emrClassicButtonPane.IsVisible()){
		m_emrClassicButtonPane.UpdateEMRProblemButtonIcon();
	}

}

// (e.lally 2012-03-14) PLID 48891
void CEmrPatientFrameWnd::OnEmnLoadSaveComplete()
{
	try {
		//Update the MU progress bar upon saving, when visible
		if(m_emrMUProgressPane.IsVisible()){
			m_emrMUProgressPane.RecalculateMeasures(true);
		}
	}NxCatchAll(__FUNCTION__);
}

// (e.lally 2012-04-03) PLID 48891
// (a.walling 2012-07-03 10:56) - PLID 51284 - Handling OnActiveEMNChanged event from CEmrFrameWnd
void CEmrPatientFrameWnd::OnActiveEMNChanged(CEMN *pFrom, CEMN *pTo)
{
	try {
		CEmrFrameWnd::OnActiveEMNChanged(pFrom, pTo);

		//Update the MU progress bar upon changing EMNs, when visible
		if(m_emrMUProgressPane.IsVisible()){
			m_emrMUProgressPane.RecalculateMeasures(true);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-09-18 12:45) - PLID 52702 - need to handle OnActiveTopicChanged so we can support save as you go, also need to and from pointers
void CEmrPatientFrameWnd::OnActiveTopicChanged(CEMRTopic *pFrom, CEMRTopic *pTo)
{
	try{
		CEmrFrameWnd::OnActiveTopicChanged(pFrom, pTo);

		if(pFrom && m_nEMRSavePref == 1){
			if(!pFrom->IsLoaded()){
				return;
			}

			// (a.walling 2013-05-23 11:53) - PLID 56828 - Do not save if the EMN is not writable
			CEMN *pFromEMN = pFrom->GetParentEMN();
			if(pFromEMN && !pFromEMN->IsTemplate() && !pFromEMN->IsLockedAndSaved() && pFromEMN->IsUnsaved() && pFromEMN->IsWritable()){
				GetEmrTreeWnd()->SaveEMR(esotEMN, (long)pFromEMN, FALSE);
			}
		}
	}NxCatchAll(__FUNCTION__);
}


//(e.lally 2012-04-24) PLID 49637
void CEmrPatientFrameWnd::OnUpdatePhotoDisplayLabels(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_bPhotoDisplayLabels ? BST_CHECKED : BST_UNCHECKED);
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::OnUpdatePhotoCategoryFilter(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-17) PLID 49636 - Disable of the Group By combo is set to Category
	BOOL bEnable = TRUE;
	if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_GROUP)) {
		long nCurSel = pList->GetCurSel();
		long nGroupID = ID_COMBO_FILTER_ALL;
		if (-1 != nCurSel) {
			nGroupID = pList->GetItemData(nCurSel);
		}

		if(nGroupID == ID_EMR_PHOTO_GROUP_CATEGORY){
			bEnable = FALSE;
		}
	}
	pCmdUI->Enable(bEnable);
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::OnUpdatePhotoProcedureFilter(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-17) PLID 49636 - Disable if the Group By combo is set to Procedure
	BOOL bEnable = TRUE;
	if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_GROUP)) {
		long nCurSel = pList->GetCurSel();
		long nGroupID = ID_COMBO_FILTER_ALL;
		if (-1 != nCurSel) {
			nGroupID = pList->GetItemData(nCurSel);
		}

		if(nGroupID == ID_EMR_PHOTO_GROUP_PROCEDURE){
			bEnable = FALSE;
		}
	}
	pCmdUI->Enable(bEnable);
}

//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
void CEmrPatientFrameWnd::OnUpdatePhotoSort(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

//(e.lally 2012-04-17) PLID 49636
void CEmrPatientFrameWnd::OnUpdatePhotoGroup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
void CEmrPatientFrameWnd::OnUpdatePhotoSortAscending(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	//Check is for Descending, so use the opposite
	pCmdUI->SetCheck(m_bPhotoSortAscending ? BST_UNCHECKED : BST_CHECKED);
}

//(e.lally 2012-04-24) PLID 49637
void CEmrPatientFrameWnd::OnPhotoDisplayLabels()
{
	try {
		m_bPhotoDisplayLabels = !m_bPhotoDisplayLabels;
		m_emrPhotosPane.ShowTextLabels(m_bPhotoDisplayLabels?true:false);
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::OnPhotoCategoryFilter()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_FILTER_CATEGORY)) {
			EMNCategory newCategory;
			long nCurSel = pList->GetCurSel();
			long nCategoryID = ID_COMBO_FILTER_ALL;
			if (-1 != nCurSel) {
				nCategoryID = pList->GetItemData(nCurSel);
			}

			m_emrPhotosPane.SetCategoryFilter(nCategoryID);
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::OnPhotoProcedureFilter()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_FILTER_PROCEDURE)) {
			long nCurSel = pList->GetCurSel();
			long nID = ID_COMBO_FILTER_ALL;
			if (-1 != nCurSel) {
				nID = pList->GetItemData(nCurSel);
			}

			m_emrPhotosPane.SetProcedureFilter(nID);
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-30) PLID 49634 - Gets EMR command ID for the last saved sort on the photos
UINT CEmrPatientFrameWnd::GetRememberedPhotoSortCmd()
{
	CPhotoViewerCtrl::EDisplaySortCriteria edscCriterion = (CPhotoViewerCtrl::EDisplaySortCriteria) 
		GetRemotePropertyInt("PhotoViewerSortCriteria", (long)CPhotoViewerCtrl::dscProcedureName, 0, GetCurrentUserName(), true);
	UINT nCmd = ID_EMR_PHOTO_SORT_ATTACHDATE;
	switch(edscCriterion){
		case CPhotoViewerCtrl::dscCategory:
			nCmd = ID_EMR_PHOTO_SORT_CATEGORY;
			break;
		case CPhotoViewerCtrl::dscAttachDate:
			nCmd = ID_EMR_PHOTO_SORT_ATTACHDATE;
			break;
		case CPhotoViewerCtrl::dscProcedureName:
			nCmd = ID_EMR_PHOTO_SORT_PROCEDURE;
			break;
		case CPhotoViewerCtrl::dscStaff:
			nCmd = ID_EMR_PHOTO_SORT_STAFF;
			break;
		case CPhotoViewerCtrl::dscNote:
			nCmd = ID_EMR_PHOTO_SORT_NOTE;
			break;
		case CPhotoViewerCtrl::dscServiceDate:
			nCmd = ID_EMR_PHOTO_SORT_SERVICEDATE;
			break;
	}
	return nCmd;
}

//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
void CEmrPatientFrameWnd::OnPhotoSort()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_SORT, FALSE)) {
			long nCurSel = pList->GetCurSel();
			long nCmd = ID_EMR_PHOTO_SORT_ATTACHDATE;
			if (-1 != nCurSel) {
				nCmd = pList->GetItemData(nCurSel);
			}

			m_emrPhotosPane.SetPrimarySort(nCmd, m_bPhotoSortAscending);
		}
		else {
			//(e.lally 2012-04-30) PLID 49634 - For some reason the ribbon is not ready for us yet,
			//	we'll have to get the remembered sort cmd ourselves
			UINT nCmd = GetRememberedPhotoSortCmd();
			m_emrPhotosPane.SetPrimarySort(nCmd, m_bPhotoSortAscending);
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-13) PLID 49634 - Photos pane sorting
void CEmrPatientFrameWnd::OnPhotoSortAscending()
{
	try {
		m_bPhotoSortAscending = !m_bPhotoSortAscending;
		//(e.lally 2012-04-25) PLID 49634 - Remember last sort order
		SetRemotePropertyInt("PhotoViewerSortCriteriaOrder", m_bPhotoSortAscending == FALSE ? 1 : 0, 0, GetCurrentUserName()); //0 = asc, 1 = desc

		OnPhotoSort();
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-04-17) PLID 49636
void CEmrPatientFrameWnd::OnPhotoGroup()
{
	try {

		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;
		if(!pEMN->GetParentEMR()) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_GROUP, FALSE)) {

			// (j.jones 2014-08-04 10:21) - PLID 63144 - clear the refresh flag
			m_bPhotosPaneNeedsRefresh = false;

			long nCurSel = pList->GetCurSel();
			long nGroupID = ID_COMBO_FILTER_ALL;
			if (-1 != nCurSel) {
				nGroupID = pList->GetItemData(nCurSel);
			}

			CString strSql;
			if(nGroupID == ID_COMBO_FILTER_ALL){
				m_emrPhotosPane.RemoveAllSubTabs();
				return;
			}
			else if(nGroupID == ID_EMR_PHOTO_GROUP_CATEGORY){
				strSql = "SELECT DISTINCT NoteCatsF.Description AS TabName, NoteCatsF.ID AS ID "
					"FROM Mailsent "
					"INNER JOIN NoteCatsF ON MailSent.CategoryID = NoteCatsF.ID "
					"WHERE dbo.IsImageFile(PathName) = 1 "
					"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
					"AND MailSent.PersonID = {INT} "
					"ORDER BY NoteCatsF.Description ASC ";
				if (dynamic_ptr<CNxRibbonAutoComboBox> pCategoryFilter = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_FILTER_CATEGORY)) {
					pCategoryFilter->SelectItem((DWORD_PTR)ID_COMBO_FILTER_ALL);
				}

			}
			else if(nGroupID == ID_EMR_PHOTO_GROUP_PROCEDURE){
				strSql = "SELECT DISTINCT ProcedureT.Name AS TabName, ProcedureT.ID AS ID "
				"FROM Mailsent "
				"INNER JOIN MailSentProcedureT ON MailSent.MailID = MailSentProcedureT.MailSentID "
				"INNER JOIN ProcedureT ON MailSentProcedureT.ProcedureID = ProcedureT.ID "
				"WHERE dbo.IsImageFile(PathName) = 1 "
				"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
				"AND MailSent.PersonID = {INT} "
				"ORDER BY ProcedureT.Name ASC ";

				if (dynamic_ptr<CNxRibbonAutoComboBox> pProcFilter = m_wndRibbonBar.FindByID(ID_EMR_PHOTO_FILTER_PROCEDURE)) {
					pProcFilter->SelectItem((DWORD_PTR)ID_COMBO_FILTER_ALL);
				}
			}
			long nPatientID = pEMN->GetParentEMR()->GetPatientID();
			ADODB::_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), strSql, nPatientID);
			//Build an ordered list of tab names
			CArray<CEmrPhotosPane::SubTabValue> arySubTabs;
			while(!rs->eof){
				CEmrPhotosPane::SubTabValue photoSubTab;
				photoSubTab.strTabName = VarString(rs->Fields->Item["TabName"]->Value, "");
				photoSubTab.nValueID = VarLong(rs->Fields->Item["ID"]->Value);
				arySubTabs.Add(photoSubTab);
				rs->MoveNext();
			}
			m_emrPhotosPane.SetGroupByType(nGroupID);
			m_emrPhotosPane.CreateSubTabs(arySubTabs);
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::OnDischargeStatus()
{
	try {
		CEMN* pEMN = GetActiveEditableEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_ASC_DISCHARGE_STATUS)) {
			long nCurSel = pList->GetCurSel();
			long nStatusID = -1;
			if (-1 != nCurSel) {
				nStatusID = (long)pList->GetItemData(nCurSel);
			}
			if (nStatusID != -1 && pEMN->GetDischargeStatus() != nStatusID) {
				using namespace PracData;
				const EmrDischargeStatus& status = Cached<EmrDischargeStatusT>()->has<ID>(nStatusID);
				pEMN->SetDischargeStatus(status.id, status.code, status.description);
			}
			else if (nStatusID != -1 && pEMN->GetDischargeStatus() == nStatusID)
				return;
			else
				pEMN->SetDischargeStatus(-1, "", "");
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::OnAdmissionTime()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		dynamic_ptr<CNxRibbonTime> pDt = m_wndRibbonBar.FindByID(ID_EMR_ASC_ADMISSION_TIME);
		if (!pDt) return;

		if (pDt->GetTime() != pEMN->GetAdmissionTime()) {
			pEMN->SetAdmissionTime(pDt->GetTime());
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::OnDischargeTime()
{
	try {
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		dynamic_ptr<CNxRibbonTime> pDt = m_wndRibbonBar.FindByID(ID_EMR_ASC_DISCHARGE_TIME);
		if (!pDt) return;

		if (pDt->GetTime() != pEMN->GetDischargeTime()) {
			pEMN->SetDischargeTime(pDt->GetTime());
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-20 09:22) - PLID 48381 - Synchronize provider items with the given ribbon element
// (a.walling 2012-04-06 12:00) - PLID 48381 - Use NxModel's PracData cache
void CEmrPatientFrameWnd::SynchronizeDelayedProviderSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonCheckList* pList = dynamic_cast<CNxRibbonCheckList*>(pElement);

	if (!pList) return;

	std::set<int> licensedProviders;
	if (ID_EMR_EMN_PROVIDER == pList->GetID()) {
		CDWordArray dwaLicensedProviders;
		g_pLicense->GetUsedEMRProviders(dwaLicensedProviders);

		foreach(DWORD d, dwaLicensedProviders) {
			licensedProviders.insert((int)d);
		}
	}

	using namespace PracData;
	for (const Provider& provider : Cached<ProvidersT>()->get<Name>()) {
		if (provider.archived) {
			continue;
		}
		if (ID_EMR_EMN_PROVIDER == pList->GetID() && !licensedProviders.count(provider.id)) {
			continue;
		}

		pList->AddItem(provider.name.FormalFull(), (DWORD_PTR)provider.id);
	}
}

// (a.walling 2012-03-20 09:22) - PLID 48381 - Synchronize assistant items with the given ribbon element
// (a.walling 2012-04-06 12:00) - PLID 48381 - Use NxModel's PracData cache
void CEmrPatientFrameWnd::SynchronizeDelayedAssistantSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonCheckList* pList = dynamic_cast<CNxRibbonCheckList*>(pElement);

	if (!pList) return;

	using namespace PracData;
	for (const User& user : Cached<UsersT>()->get<UserName>()) {
		if (user.archived || !user.technician || user.id < 0) {
			continue;
		}

		pList->AddItem(FormatString("%s - %s", user.userName, user.name.FormalFull()), (DWORD_PTR)user.id);
	}
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::SynchronizeDelayedPhotoCategorySubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	long nFilteredID = ID_COMBO_FILTER_ALL;
	CArray<long> aryCategoryFilter;
	m_emrPhotosPane.GetCategoryFilter(aryCategoryFilter);
	if(aryCategoryFilter.GetSize() > 0){
		nFilteredID = aryCategoryFilter.GetAt(0);
	}

	pList->RemoveAllItems();

	//(e.lally 2012-04-12) PLID 49566 - Re-add our special selections
	pList->AddItem("<All Categories>", (DWORD_PTR)ID_COMBO_FILTER_ALL);
	pList->AddItem("<No Category>", (DWORD_PTR)ID_COMBO_FILTER_NO_KEY);

	using namespace PracData;
	//(e.lally 2012-04-12) PLID 49566 - Use a special cached list of note categories to fill the combo.
	for (const NoteCategory& category : Cached<NoteCatsF>()->get<Name>()) {
		pList->AddItem(category.description, (DWORD_PTR)category.id);
	}

	pList->SelectItem((DWORD_PTR)nFilteredID);
}

// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status
// (a.walling 2012-04-06 12:00) - PLID 49496 - Use NxModel's PracData cache
void CEmrPatientFrameWnd::SynchronizeDelayedStatusSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	long nCurStatusID = pEMN->GetStatus();
	
	pList->RemoveAllItems();

	using namespace PracData;
	for (const EmrStatus& status : Cached<EmrStatusListT>()->get<ID>()) {
		pList->AddItem(status.name, (DWORD_PTR)status.id);
	}

	if (-1 != nCurStatusID) {
		pList->SelectItem((DWORD_PTR)nCurStatusID);
	}
}

// (a.walling 2012-04-09 09:03) - PLID 49515 - Location
void CEmrPatientFrameWnd::SynchronizeDelayedLocationSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	// (a.walling 2012-04-09 09:03) - PLID 49515 - Location - Return const ref
	long nCurLocationID = pEMN->GetLocation().nID;
	
	pList->RemoveAllItems();

	using namespace PracData;
	for (const Location& location : Cached<LocationsT>()->get<ID>()) {
		if (!location.IsNormal() || !location.IsManaged()) {
			continue;
		}
		if (!location.IsActive() && location.id != nCurLocationID) {
			continue;
		}
		pList->AddItem(location.name, (DWORD_PTR)location.id);
	}
	// (a.walling 2012-06-12 08:35) - PLID 49515 - Now, include non-managed locations
	for (const Location& location : Cached<LocationsT>()->get<ID>()) {
		if (!location.IsNormal() || location.IsManaged()) {
			continue;
		}
		if (!location.IsActive() && location.id != nCurLocationID) {
			continue;
		}
		pList->AddItem(location.name, (DWORD_PTR)location.id);
	}

	if (-1 != nCurLocationID) {
		pList->SelectItem((DWORD_PTR)nCurLocationID);
	}
}

//(e.lally 2012-04-12) PLID 49566
void CEmrPatientFrameWnd::SynchronizeDelayedPatientProcedureSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	if(!pEMN->GetParentEMR()) return;

	long nPatientID = pEMN->GetParentEMR()->GetPatientID();

	long nFilteredID = ID_COMBO_FILTER_ALL;
	CArray<long> aryProcedureFilter;
	m_emrPhotosPane.GetProcedureFilter(aryProcedureFilter);
	if(aryProcedureFilter.GetSize() > 0){
		nFilteredID = aryProcedureFilter.GetAt(0);
	}

	pList->RemoveAllItems();

	//(e.lally 2012-04-12) PLID 49566 - Re-add our special selection options
	pList->AddItem("<All Procedures>", (DWORD_PTR)ID_COMBO_FILTER_ALL);
	pList->AddItem("<No Procedure>", (DWORD_PTR)ID_COMBO_FILTER_NO_KEY);

	//(e.lally 2012-04-12) PLID 49566 - Pull the list of procedures from data relevant to this patient. This code was copied from the code in the history dlg.
	ADODB::_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), 
		"SELECT ID, Name FROM ProcedureT "
		"INNER JOIN "
		"(SELECT ProcedureID "
			"FROM MailSent "
			"INNER JOIN MailSentProcedureT ON MailSent.MailID = MailSentProcedureT.MailSentID "
			"WHERE PersonID = {INT} "
			"GROUP BY ProcedureID "
		" UNION SELECT ProcedureID FROM ProcInfoDetailsT INNER JOIN ProcInfoT ON ProcInfoDetailsT.ProcInfoID = "
		"              ProcInfoT.ID WHERE ProcInfoT.PatientID = {INT} GROUP BY ProcedureID "
		" UNION SELECT ProcedureID FROM (SELECT * FROM EMRMasterT WHERE Deleted = 0) AS EmrMasterT "
		"INNER JOIN (SELECT * FROM EmrProcedureT WHERE Deleted = 0) AS EmrProcedureT ON EmrMasterT.ID = EmrProcedureT.EMRID WHERE PatientID = {INT} GROUP BY ProcedureID) SubQ "
		"ON ProcedureT.ID = SubQ.ProcedureID WHERE ProcedureT.Inactive = 0 "
		"ORDER BY ProcedureT.Name ",
		nPatientID, nPatientID, nPatientID);
	while(!rs->eof){
		pList->AddItem(AdoFldString(rs->Fields, "Name", ""), (DWORD_PTR)AdoFldLong(rs->Fields, "ID"));
		rs->MoveNext();
	}
	rs->Close();

	pList->SelectItem((DWORD_PTR)nFilteredID);
}

// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
void CEmrPatientFrameWnd::SynchronizeDelayedProcedureSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonCheckList* pList = dynamic_cast<CNxRibbonCheckList*>(pElement);

	if (!pList) return;

	using namespace PracData;
	for (const Procedure& procedure : Cached<ProcedureT>()->get<Name>()) {
		// (r.farnworth 2015-03-06 11:25) - PLID 53415 - Inactivate procdures should not show up in the selected procedure drop down
		if (procedure.inactive == false) {
			pList->AddItem(procedure.name, (DWORD_PTR)procedure.id);
		}
	}
}

// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
void CEmrPatientFrameWnd::SynchronizeDelayedDischargeStatusSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	long nCurStatusID = pEMN->GetDischargeStatus().nID;

	pList->RemoveAllItems();

	pList->AddItem("<No Status>", (DWORD_PTR)-1);

	using namespace PracData;
	for (const EmrDischargeStatus& status : Cached<EmrDischargeStatusT>()->get<Code>()) {
		pList->AddItem(FormatString("%s - %s", status.code, status.description), (DWORD_PTR)status.id);
	}

	pList->SelectItem((DWORD_PTR)nCurStatusID);
}

// (a.walling 2012-03-20 09:24) - PLID 48990 - Ensure the ribbon item has its menu fully populated with all data
void CEmrPatientFrameWnd::SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement)
{
	if (!pElement) return;
	
	typedef void (CEmrPatientFrameWnd::*SyncFn)(CMFCRibbonBaseElement*);

	struct SyncEntry {
		UINT nID;
		SyncFn fn;
	};

	// (a.walling 2012-04-06 13:19) - PLID 48990 - Changed this a bit to have a list of entries rather than a switch statement which can quickly get out of control
	static SyncEntry syncEntries[] = {
		{ID_EMR_EMN_PROVIDER,				&CEmrPatientFrameWnd::SynchronizeDelayedProviderSubitems},
		{ID_EMR_EMN_SECONDARY_PROVIDER,		&CEmrPatientFrameWnd::SynchronizeDelayedProviderSubitems},
		{ID_EMR_EMN_ASSISTANT,				&CEmrPatientFrameWnd::SynchronizeDelayedAssistantSubitems},
		{ID_EMR_EMN_STATUS,					&CEmrPatientFrameWnd::SynchronizeDelayedStatusSubitems},			// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status
		{ID_EMR_EMN_LOCATION,				&CEmrPatientFrameWnd::SynchronizeDelayedLocationSubitems},			// (a.walling 2012-04-09 09:03) - PLID 49515 - Location
		{ID_EMR_PHOTO_FILTER_CATEGORY,		&CEmrPatientFrameWnd::SynchronizeDelayedPhotoCategorySubitems},		//(e.lally 2012-04-12) PLID 49566
		{ID_EMR_PHOTO_FILTER_PROCEDURE,		&CEmrPatientFrameWnd::SynchronizeDelayedPatientProcedureSubitems},	//(e.lally 2012-04-12) PLID 49566
		{ID_EMR_EMN_PROCEDURES,				&CEmrPatientFrameWnd::SynchronizeDelayedProcedureSubitems},			// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
		{ID_EMR_ASC_DISCHARGE_STATUS,		&CEmrPatientFrameWnd::SynchronizeDelayedDischargeStatusSubitems}	// (b.eyers 2016-02-22) - PLID 68321 - discharge status
	};

	try {

		UINT nID = pElement->GetID();

		foreach(SyncEntry& syncEntry, syncEntries) {
			if (nID == syncEntry.nID) {
				EnsureCachedData();
				(this->*(syncEntry.fn))(pElement);
				return;
			}
		}

		// (a.walling 2012-05-25 17:39) - PLID 50664 - Forward to view
		CEmrFrameWnd::SynchronizeDelayedRibbonSubitems(pElement);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-06 13:19) - PLID 48990 - Ensure the most common lists are cached
void CEmrPatientFrameWnd::EnsureCachedData()
{
	// note this will not bother loading a tables if it is already loaded
	using namespace PracData;
	CacheLoader()
		.Load<LocationsT>()
		.Load<ProvidersT>()
		.Load<UsersT>()
		.Load<EmrStatusListT>()
		.Load<EmnTabChartsT>()
		.Load<EmnTabCategoriesT>()
		.Load<EmnTabChartCategoryLinkT>()
		.Load<ProcedureT>()
		.Load<EmrDischargeStatusT>()
		.Run();
}

// (j.armen 2012-06-11 17:09) - PLID 48808 - Handle the database reconnecting 
//	by releasing and attempting to obtain a new write token
void CEmrPatientFrameWnd::OnDatabaseReconnect()
{
	foreach(CEMN* pEMN, GetEmrTreeWnd()->GetEMR()->GetAllEMNs()) {
		// (a.walling 2013-10-01 10:11) - PLID 58827 - Skip new EMNs
		if (!pEMN || (pEMN->GetID() == -1) || !pEMN->IsWritable()) {
			continue;
		}

		pEMN->ReleaseWriteToken();
		GetEmrTreeWnd()->PostMessage(NXM_TRY_ACQUIRE_WRITE_ACCESS, (WPARAM)pEMN->GetID());
	}
}

// (j.armen 2012-06-11 17:10) - PLID 48808 - Iterate through all frames
void CEmrPatientFrameWnd::HandleDatabaseReconnect()
{
	foreach(CEmrPatientFrameWnd* pFrame, g_allPatientFrameWnds)
		pFrame->OnDatabaseReconnect();
}

// (a.walling 2013-01-17 12:36) - PLID 54666 - Adds a new EMN from template, via EmrTreeWnd
CEMN* CEmrPatientFrameWnd::AddEMNFromTemplate(long nTemplateID, SourceActionInfo &sai)
{
	CEMN* pEMN = NULL;
	CEmrTreeWnd* pTree = GetEmrTreeWnd();

	if (pTree) {
		// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking - auto-assign appointment or prompt to choose
		pEMN = pTree->AddEMNFromTemplateWithAppointmentPrompt(nTemplateID, sai);
	}

	return pEMN;
}

CEMN* CEmrPatientFrameWnd::AddEMNFromTemplate(long nTemplateID)
{
	SourceActionInfo sai;
	return AddEMNFromTemplate(nTemplateID, sai);
}

// (d.singleton 2013-11-15 13:37) - PLID 59527 - need to be able to create clinical summary and care summary from the emr ribbon.
void CEmrPatientFrameWnd::OnCareSummary() 
{
	try {

		CEmrTreeWnd* pTree = GetEmrTreeWnd();
		if(!pTree) {
			MessageBox("Failed to generate Summary of Care.", "NexTech Practice", MB_OK|MB_ICONWARNING);
			return;
		}

		// (b.savon 2014-02-25 14:18) - PLID 61029 - Forward request to EmrTreeWnd
		pTree->GenerateCareSummary();

	} NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-11-15 13:37) - PLID 59527 - need to be able to create clinical summary and care summary from the emr ribbon.
void CEmrPatientFrameWnd::OnClinicalSummary() 
{
	try {

		CEmrTreeWnd* pTree = GetEmrTreeWnd();
		if(!pTree) {
			MessageBox("Failed to generate Clinical Summary.", "NexTech Practice", MB_OK|MB_ICONWARNING);
			return;
		}

		// (b.savon 2014-02-25 14:18) - PLID 61029 - Forward request to EmrTreeWnd
		pTree->GenerateClinicalSummary();

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
void CEmrPatientFrameWnd::OnCareSummaryCustomized()
{
	try {

		CEmrTreeWnd* pTree = GetEmrTreeWnd();
		if (!pTree) {
			MessageBox("Failed to generate Summary of Care.", "NexTech Practice", MB_OK | MB_ICONWARNING);
			return;
		}

		// (b.savon 2014-02-25 14:18) - PLID 61029 - Forward request to EmrTreeWnd

		pTree->GenerateCareSummary(true);

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
void CEmrPatientFrameWnd::OnClinicalSummaryCustomized()
{
	try {

		CEmrTreeWnd* pTree = GetEmrTreeWnd();
		if (!pTree) {
			MessageBox("Failed to generate Clinical Summary.", "NexTech Practice", MB_OK | MB_ICONWARNING);
			return;
		}

		// (b.savon 2014-02-25 14:18) - PLID 61029 - Forward request to EmrTreeWnd

		pTree->GenerateClinicalSummary(true);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-08-04 10:36) - PLID 63144 - track when the tab changes
LRESULT CEmrPatientFrameWnd::OnChangeActiveTab(WPARAM wp, LPARAM /*lp*/)
{
	try {
		
		// (j.jones 2014-08-04 10:42) - PLID 63144 - if the photos tab
		// is now visible, and it needs to refresh, do so now
		if (m_emrPhotosPane.GetSafeHwnd()
			&& m_emrPhotosPane.IsWindowVisible()
			&& m_bPhotosPaneNeedsRefresh) {

			OnPhotoGroup();
		}

	}NxCatchAll(__FUNCTION__);
	return 0;
}