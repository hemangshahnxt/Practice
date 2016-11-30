#pragma once

#include "EmrFrameWnd.h"
#include "EmrUtils.h"
#include "EmrDocument.h"
#include "EmrDocTemplate.h"
#include "EmrTreePane.h"
#include "EmrPreviewPane.h"
#include "EmrPhotosPane.h"
#include "EmrClassicButtonPane.h" //(e.lally 2012-02-16) PLID 48065
#include "EmrMUProgressPane.h" //(e.lally 2012-02-23) PLID 48016
#include "EmrGraphingPane.h"

// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxRibbonBar.h>

class CEmrMedAllergyViewerDlg;
class CEMRBarcodeDlg;
class CDrugInteractionDlg;

// (a.walling 2012-02-28 14:53) - PLID 48451 - CEmrPatientFrameWnd - move patient-related logic here, keeping CEmrFrameWnd agnostic for both template and patient frames

// (a.walling 2012-02-29 06:42) - PLID 46644 - A bit of reorganization with ribbon initialization

// CEmrPatientFrameWnd frame

class CEmrPatientFrameWnd : public CEmrFrameWnd
{
	DECLARE_DYNAMIC(CEmrPatientFrameWnd)

public:	
	// (a.walling 2013-01-17 12:36) - PLID 54666 - Adds a new EMN from template, via EmrTreeWnd
	CEMN* AddEMNFromTemplate(long nTemplateID, SourceActionInfo &sai);
	CEMN* AddEMNFromTemplate(long nTemplateID);

protected:
	BOOL m_bPhotoDisplayLabels; //(e.lally 2012-04-24) PLID 49637
	BOOL m_bPhotoSortAscending; //(e.lally 2012-04-13) PLID 49634

	CEmrPatientFrameWnd();           // protected constructor used by dynamic creation
	virtual ~CEmrPatientFrameWnd();

	//(e.lally 2012-04-12) PLID 49566	
	virtual void UpdateContextCategories();
	//(e.lally 2012-04-30) PLID 49634
	UINT GetRememberedPhotoSortCmd();

	virtual void OnDatabaseReconnect();	// (j.armen 2012-06-11 17:11) - PLID 48808

	// (j.jones 2014-08-04 10:21) - PLID 63144 - tracks whether the photos pane
	// needs to refresh the next time it is displayed
	bool m_bPhotosPaneNeedsRefresh;
	
public:
	DECLARE_MESSAGE_MAP()
	CEmrPhotosPane			m_emrPhotosPane;
	CEmrMUProgressPane		m_emrMUProgressPane; //(e.lally 2012-02-23) PLID 48016
	CEmrGraphingPane		m_emrGraphingPane; // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

	virtual void InitializeRibbon();
	virtual void InitializeStatusBar();
	virtual void InitializePanes();

	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	void InitializeRibbon_MainPanel();
	void InitializeRibbon_View();
	void InitializeRibbon_Edit();
	void InitializeRibbon_EMN();
	void InitializeRibbon_MoreInfo();
	void InitializeRibbon_Patient();
	void InitializeRibbon_Preview();
	void InitializeRibbon_Photos();
	void InitializeRibbon_Merge();
	void InitializeRibbon_QAT();

	// (a.walling 2012-03-02 16:30) - PLID 48598 - return our layout section name
	virtual CString GetLayoutSectionName();

	////
	/// UI state

	// (a.walling 2012-03-20 09:24) - PLID 48990 - Ensure the ribbon item has its menu fully populated with all data
	virtual void SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement);

	// (a.walling 2012-04-06 13:19) - PLID 48990 - Ensure the most common lists are cached
	virtual void EnsureCachedData();

	// all of these use NxModel's PracData cache now

	// (a.walling 2012-03-20 09:22) - PLID 48381 - Synchronize provider items with the given ribbon element
	void SynchronizeDelayedProviderSubitems(CMFCRibbonBaseElement* pElement);
	// (a.walling 2012-03-20 09:22) - PLID 48381 - Synchronize assistant items with the given ribbon element
	void SynchronizeDelayedAssistantSubitems(CMFCRibbonBaseElement* pElement);
	// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status
	void SynchronizeDelayedStatusSubitems(CMFCRibbonBaseElement* pElement);
	// (a.walling 2012-04-09 09:03) - PLID 49515 - Location
	void SynchronizeDelayedLocationSubitems(CMFCRibbonBaseElement* pElement);
	//(e.lally 2012-04-12) PLID 49566 - NoteCatsF category for photos 
	void SynchronizeDelayedPhotoCategorySubitems(CMFCRibbonBaseElement* pElement);
	//(e.lally 2012-04-12) PLID 49566 - Procedures associated with this patient
	void SynchronizeDelayedPatientProcedureSubitems(CMFCRibbonBaseElement* pElement);	
	// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
	void SynchronizeDelayedProcedureSubitems(CMFCRibbonBaseElement* pElement);
	// (b.eyers 2016-02-22) - PLID 68321 - discharge status
	void SynchronizeDelayedDischargeStatusSubitems(CMFCRibbonBaseElement* pElement);

	afx_msg void OnUpdateNewEMN(CCmdUI* pCmdUI);

	// (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
	afx_msg void OnUpdateEMRStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEMRDescription(CCmdUI* pCmdUI); // (a.walling 2012-05-18 17:18) - PLID 50546 - EMR Description

	afx_msg void OnUpdateViewPhotosPane(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMUProgressPane(CCmdUI* pCmdUI); //(e.lally 2012-02-23) PLID 48016
	afx_msg void OnUpdateViewGraphingPane(CCmdUI* pCmdUI); // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

	afx_msg void OnUpdateStatus(CCmdUI* pCmdUI);

	afx_msg void OnUpdateNotes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLocation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDate(CCmdUI* pCmdUI); // (a.walling 2012-05-17 17:46) - PLID 50495 - EMR Date
	afx_msg void OnUpdateDateCreated(CCmdUI* pCmdUI); // (a.walling 2012-06-07 08:59) - PLID 50920 - Dates - Modified, Created
	afx_msg void OnUpdateDateModified(CCmdUI* pCmdUI); // (a.walling 2012-06-07 08:59) - PLID 50920 - Dates - Modified, Created

	// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
	afx_msg void OnUpdateProvider(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSecondaryProvider(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAssistants(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOtherProviders(CCmdUI* pCmdUI);
	
	// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
	afx_msg void OnUpdateProcedures(CCmdUI* pCmdUI);

	afx_msg void OnUpdateNewTodo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewRecording(CCmdUI* pCmdUI);
	// (a.walling 2012-06-11 09:34) - PLID 50925
	afx_msg void OnUpdateNewCharge(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewDiagCode(CCmdUI* pCmdUI);

	afx_msg void OnUpdatePatientSummary(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLinkedAppointment(CCmdUI* pCmdUI); // (a.walling 2013-02-13 10:39) - PLID 55143 - Emr Appointment linking - UI
	afx_msg void OnUpdateShowConfidentialInfo(CCmdUI* pCmdUI);	
	// (a.walling 2012-06-11 09:27) - PLID 50922 - Update patient demographics
	afx_msg void OnUpdateUpdateDemographics(CCmdUI* pCmdUI);

	// (a.walling 2012-06-11 09:23) - PLID 50921 - Patient demographics: name, age, gender
	afx_msg void OnUpdatePatientName(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePatientAge(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePatientGender(CCmdUI* pCmdUI);

	// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
	afx_msg void OnUpdateShowProblems(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAddNewProblem(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLinkExistingProblem(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAddNewProblemToEMR(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAddNewProblemToEMN(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAddNewProblemToTopic(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLinkExistingProblemToEMR(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLinkExistingProblemToEMN(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLinkExistingProblemToTopic(CCmdUI* pCmdUI);

	// (b.spivey, March 06, 2012) - PLID 48581 - Make sure to update the controls
	afx_msg void OnUpdateEMChecklist(CCmdUI* pCmdUI); 
	afx_msg void OnUpdateEMVisitType(CCmdUI* pCmdUI); 

	afx_msg void OnUpdateShowEMRAuditHistory(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowEMNAuditHistory(CCmdUI* pCmdUI);

	 //(e.lally 2012-02-16) PLID 48065 - Classic Button pane UI state
	afx_msg void OnUpdateClassicBtnProblemList(CCmdUI* pCmdUI);

	//Photos pane
	//(e.lally 2012-04-24) PLID 49637
	afx_msg void OnUpdatePhotoDisplayLabels(CCmdUI* pCmdUI);
	//(e.lally 2012-04-12) PLID 49566
	afx_msg void OnUpdatePhotoCategoryFilter(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePhotoProcedureFilter(CCmdUI* pCmdUI);
	//(e.lally 2012-04-12) PLID 49634
	afx_msg void OnUpdatePhotoSort(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePhotoSortAscending(CCmdUI* pCmdUI);
	//(e.lally 2012-04-17) PLID 49636
	afx_msg void OnUpdatePhotoGroup(CCmdUI* pCmdUI);
	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	afx_msg void OnUpdateDischargeStatus(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAdmissionTime(CCmdUI* pCmdUI);
	afx_msg void OnUpdateDischargeTime(CCmdUI* pCmdUI);

	////
	/// UI Commands

	afx_msg void OnNewEMN();
	afx_msg void OnEMRStatus(); // (a.walling 2012-11-27 10:09) - PLID 53891 - EMR Status combo
	afx_msg void OnEMRDescription(); // (a.walling 2012-05-18 17:18) - PLID 50546 - EMR Description

	afx_msg void OnViewPhotosPane();
	afx_msg void OnViewMUProgressPane(); //(e.lally 2012-02-23) PLID 48016
	afx_msg void OnViewGraphingPane(); // (a.walling 2012-04-12 14:42) - PLID 49657 - EMR Graphing pane

	afx_msg void OnStatus();

	afx_msg void OnNotes();
	afx_msg void OnLocation();
	afx_msg void OnDate(); // (a.walling 2012-05-17 17:46) - PLID 50495 - EMR Date

	// (a.walling 2012-02-24 07:49) - PLID 48381 - Providers
	afx_msg void OnProvider();
	afx_msg void OnSecondaryProvider();
	afx_msg void OnAssistants();
	afx_msg void OnOtherProviders();

	// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
	afx_msg void OnProcedure();

	afx_msg void OnNewTodo();
	afx_msg void OnNewRecording();
	afx_msg void OnNewCharge();
	afx_msg void OnNewDiagCode();

	afx_msg void OnPatientSummary();
	afx_msg void OnLinkedAppointment(); // (a.walling 2013-02-13 10:39) - PLID 55143 - Emr Appointment linking - UI
	afx_msg void OnShowConfidentialInfo();	
	// (a.walling 2012-06-11 09:27) - PLID 50922 - Update patient demographics
	afx_msg void OnUpdateDemographics();

	// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
	afx_msg void OnShowProblems();
	afx_msg void OnAddNewProblem();
	afx_msg void OnLinkExistingProblem();
	afx_msg void OnAddNewProblemToEMR();
	afx_msg void OnAddNewProblemToEMN();
	afx_msg void OnAddNewProblemToTopic();
	afx_msg void OnLinkExistingProblemToEMR();
	afx_msg void OnLinkExistingProblemToEMN();
	afx_msg void OnLinkExistingProblemToTopic();
	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	afx_msg void OnDischargeStatus();
	afx_msg void OnAdmissionTime();
	afx_msg void OnDischargeTime();

protected:
	// (b.spivey, March 06, 2012) - PLID 48581 - Handlers for the controls. 
	afx_msg void OnShowEMChecklist();
	afx_msg void OnSelectEMVisitType();

	afx_msg void OnShowEMRAuditHistory();
	afx_msg void OnShowEMNAuditHistory();

	//Photos pane
	//(e.lally 2012-04-24) PLID 49637
	afx_msg void OnPhotoDisplayLabels();
	//(e.lally 2012-04-12) PLID 49566
	afx_msg void OnPhotoCategoryFilter();
	afx_msg void OnPhotoProcedureFilter();
	//(e.lally 2012-04-13) PLID 49634
	afx_msg void OnPhotoSort();
	afx_msg void OnPhotoSortAscending();
	//(e.lally 2012-04-17) PLID 49636
	afx_msg void OnPhotoGroup();

	// (a.walling 2013-11-15 11:16) - PLID 59517 - Care Summary, Clinical Summary in ribbon

	afx_msg void OnCareSummary();
	afx_msg void OnClinicalSummary();
	// (a.walling 2014-05-12 09:24) - PLID 61787 - Customized CCDA summaries
	afx_msg void OnCareSummaryCustomized();
	afx_msg void OnClinicalSummaryCustomized();

	// (j.jones 2014-08-04 10:36) - PLID 63144 - track when the tab changes
	afx_msg LRESULT OnChangeActiveTab(WPARAM wp, LPARAM lp);

public:
	// (e.lally 2012-03-14) PLID 48891
	void OnEmnLoadSaveComplete();
	// (e.lally 2012-04-03) PLID 48891
	// (a.walling 2012-07-03 10:56) - PLID 51284 - Handling OnActiveEMNChanged event from CEmrFrameWnd
	// (j.dinatale 2012-09-18 16:52) - PLID 52702 - need to pass along to and from pointers
	virtual void OnActiveEMNChanged(CEMN *pFrom, CEMN *pTo);

	// (j.dinatale 2012-09-18 12:45) - PLID 52702 - need to handle OnActiveTopicChanged so we can support save as you go
	virtual void OnActiveTopicChanged(CEMRTopic *pFrom, CEMRTopic *pTo);
	
	static void HandleDatabaseReconnect();	// (j.armen 2012-06-11 17:11) - PLID 48808
};
