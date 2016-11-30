// PicContainerDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "patientsRc.h"
#include "PicContainerDlg.h"
#include "ProcInfoCenterDlg.h"
#include "PhaseTracking.h"
#include "SelectDlg.h"
#include "ConfigPacketsDlg.h"
#include "LetterWriting.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "EmrItemAdvImageDlg.h"
#include "NxMessageDef.h"
#include "EMRMergeConflictDlg.h"
#include "EMRMergePrecedenceDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "MultiSelectDlg.h"
#include "SelectSenderDlg.h"
#include "EMRUtils.h"
#include "EmrEditorDlg.h"
#include "EMN.h"
#include "EMRTopic.h"
#include "AuditTrail.h"
#include "EMNLoader.h"
#include "HL7Utils.h"
#include "foreach.h"
#include "EmrChildWnd.h"
#include "DontShowDlg.h"
#include "EmrMedAllergyViewerDlg.h"
#include "PatientGraphConfigDlg.h"
#include "EMRBarcodeDlg.h"
#include "FileUtils.h"
#include "CreatePatientRecall.h"
#include "EmrGraphSetupDlg.h"
#include <NxSystemUtilitiesLib/dynamic_ptr.h>
#include <NxAdvancedUILib/NxRibbonControls.h>
#include "NxWordProcessorLib\GenericWordProcessorManager.h"
#include "EmrIcons.h"
#include <NxPracticeSharedLib/PracDataEmr.h>
#include <NxPracticeSharedLib/PracData.h>
#include <NxAlgorithm.h>
#include <NxUILib/WindowUtils.h>
#include "EmrRibbonControls.h"

#include "DrugInteractionDlg.h"
#include "FirstDataBankUtils.h"
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include "EMR.h"
#include "EMNDetail.h"
#include "DecisionRuleUtils.h"

#include "CCDInterface.h"
#include <CancerCaseDocument.h>
#include "BillingModuleDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
// updated the various sheet pointers to always call Get*, eg GetEmrEditor, GetPicDlg, GetHistoryDlg, GetLabsDlg

// (a.walling 2012-05-22 09:59) - PLID 50556 - Prevent unnecessary redraws of ribbon combo boxes by always using CNxRibbonAutoComboBox instead of CMFCRibbonComboBox

// (a.walling 2013-04-08 08:57) - PLID 56131 - Added more icons, more!

/////////////////////////////////////////////////////////////////////////////
// CPicContainerDlg dialog

class CEMR_ExtraMergeFields
{
public:
	CString m_strHeaders;
	CString m_strData;
};

// (c.haag 2004-06-30 12:31) - This is the callback for merging an EMR chart with a Word template
CString CALLBACK CEMR_PIC__ExtraMergeFields(BOOL bFieldNamesInsteadOfData, const CString &strKeyFieldValue, LPVOID pParam)
{
	try {
		CEMR_ExtraMergeFields *pemf = (CEMR_ExtraMergeFields *)pParam;
		if (bFieldNamesInsteadOfData) {
			return pemf->m_strHeaders;
		} else {
			return pemf->m_strData;
		}
	} NxCatchAllCallIgnore({
		return "";
	});
}


using namespace ADODB;

// (a.walling 2011-10-20 21:22) - PLID 46076 - Facelift - EMR frame window
// (a.walling 2012-02-28 14:53) - PLID 48451 - Now deriving from CEmrPatientFrameWnd

IMPLEMENT_DYNCREATE(CPicContainerDlg, CEmrPatientFrameWnd)

CPicContainerDlg::CPicContainerDlg()
	// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
	// (j.dinatale 2012-07-12 14:21) - PLID 51481 - external pic opening
	: m_nLastTotalSecondsOpen(INT_MAX)
	, m_nLastCurrentSecondsOpen(INT_MAX)
	, m_bLoadedMergePacketCombo(false)
	, m_bLoadedMergeTemplateCombo(false)
	, m_bLoadedForPic(false)
{
	m_pPicDocTemplate.reset(new CPicDocTemplate(IDR_EMRFRAME,
		RUNTIME_CLASS(CPicDocument),
		RUNTIME_CLASS(CEmrChildWnd),
		this)
	);

	m_nColor = 0x00CFE3BB;

	m_bIsNew = false;

	m_nEmrGroupID = -2;
	m_nProcInfoID = -2;

	m_bInitialized = FALSE;
	m_bIsCommitted = FALSE;

	m_bPicDlgNeedsToLoad = FALSE;

	m_lNewEmnCategoryID = -1;

	m_nPatientID = -1;

	m_cbPatientGender = 0;

	// (j.jones 2013-03-01 09:41) - PLID 52818 - cache the e-prescribing type
	m_eRxType = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
}

CPicContainerDlg::~CPicContainerDlg()
{
}

BEGIN_MESSAGE_MAP(CPicContainerDlg, CEmrPatientFrameWnd)
	//{{AFX_MSG_MAP(CPicContainerDlg)

	////
	/// UI State

	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_PIC, &CPicContainerDlg::OnUpdateViewPic)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_HISTORY, &CPicContainerDlg::OnUpdateViewHistory)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_LABS, &CPicContainerDlg::OnUpdateViewLabs)
	// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
	//ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_GRAPHS, &CPicContainerDlg::OnUpdateViewGraphs)
	// (a.walling 2012-06-11 09:30) - PLID 50913 - Organizing the ribbon
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_EPRESCRIBING, &CPicContainerDlg::OnUpdateViewEPrescribing)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_MEDALLERGY, &CPicContainerDlg::OnUpdateViewMedAllergy)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_CAMERABARCODE, &CPicContainerDlg::OnUpdateViewBarcode)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_EYEMAGINATIONS, &CPicContainerDlg::OnUpdateViewEyemaginations)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_RECALL, &CPicContainerDlg::OnUpdateViewRecall)
	// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_DEVICE_IMPORT, &CPicContainerDlg::OnUpdateViewDeviceImport)
	// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_DRUGINTERACTIONS, &CPicContainerDlg::OnUpdateViewDrugInteractions)
	
	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
	ON_UPDATE_COMMAND_UI(ID_EMR_STATUS_TOTAL_TIME_OPEN, &CPicContainerDlg::OnUpdateStatusTotalTimeOpen)
	ON_UPDATE_COMMAND_UI(ID_EMR_STATUS_CURRENT_TIME_OPEN, &CPicContainerDlg::OnUpdateStatusCurrentTimeOpen)

	/// Merging
	// (a.walling 2012-04-30 12:47) - PLID 49832 - Merging UI
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_FROM_PACKET, &CPicContainerDlg::OnUpdateMergeFromPacket)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_FROM_PACKET_TO_WORD, &CPicContainerDlg::OnUpdateMergeFromPacket)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_FROM_PACKET_TO_PRINTER, &CPicContainerDlg::OnUpdateMergeFromPacket)

	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_FROM_TEMPLATE, &CPicContainerDlg::OnUpdateMergeFromTemplate)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_FROM_TEMPLATE_TO_WORD, &CPicContainerDlg::OnUpdateMergeFromTemplate)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_FROM_TEMPLATE_TO_PRINTER, &CPicContainerDlg::OnUpdateMergeFromTemplate)

	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_EDIT_PACKET, &CPicContainerDlg::OnUpdateEditPacket)

	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_EDIT_TEMPLATE, &CPicContainerDlg::OnUpdateEditTemplate)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_NEW_TEMPLATE, &CPicContainerDlg::OnUpdateNewTemplate)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_COPY_TEMPLATE, &CPicContainerDlg::OnUpdateNewTemplate) // same criteria as new template

	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_PACKET_COMBO, &CPicContainerDlg::OnUpdateMergePacketCombo)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_TEMPLATE_COMBO, &CPicContainerDlg::OnUpdateMergeTemplateCombo)

	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_ADVANCED_SETUP, &CPicContainerDlg::OnUpdateMergeAdvancedSetup)
	ON_UPDATE_COMMAND_UI(ID_EMR_MERGE_REFRESH, &CPicContainerDlg::OnUpdateMergeRefresh)

	ON_UPDATE_COMMAND_UI_RANGE(ID_EMR_MERGE_OPTIONS_BEGIN, ID_EMR_MERGE_OPTIONS_END, &CPicContainerDlg::OnUpdateMergeOptions)

	/// Single EMN Merge	

	// (a.walling 2012-10-01 09:15) - PLID 52119 - Handle merging from single EMN, setting default templates

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_FROM_TEMPLATE, &CPicContainerDlg::OnUpdateEmnMergeFromTemplate)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_FROM_TEMPLATE_TO_WORD, &CPicContainerDlg::OnUpdateEmnMergeFromTemplate)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_FROM_TEMPLATE_TO_PRINTER, &CPicContainerDlg::OnUpdateEmnMergeFromTemplate)

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_FROM_OTHER, &CPicContainerDlg::OnUpdateEmnMergeFromOther)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_FROM_OTHER_TO_WORD, &CPicContainerDlg::OnUpdateEmnMergeFromOther)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_FROM_OTHER_TO_PRINTER, &CPicContainerDlg::OnUpdateEmnMergeFromOther)

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_EDIT_TEMPLATE, &CPicContainerDlg::OnUpdateEmnEditTemplate)

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_TEMPLATE_COMBO, &CPicContainerDlg::OnUpdateEmnMergeTemplateCombo)

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MERGE_TEMPLATE_MAKE_DEFAULT, &CPicContainerDlg::OnUpdateEmnMergeTemplateMakeDefault)

	////
	/// UI Commands

	ON_COMMAND(ID_EMR_VIEW_PIC, &CPicContainerDlg::OnViewPic)
	ON_COMMAND(ID_EMR_VIEW_HISTORY, &CPicContainerDlg::OnViewHistory)
	ON_COMMAND(ID_EMR_VIEW_LABS, &CPicContainerDlg::OnViewLabs)
	// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
	//ON_COMMAND(ID_EMR_VIEW_GRAPHS, &CPicContainerDlg::OnViewGraphs)
	// (a.walling 2012-06-11 09:30) - PLID 50913 - Organizing the ribbon
	ON_COMMAND(ID_EMR_VIEW_EPRESCRIBING, &CPicContainerDlg::OnViewEPrescribing)
	ON_COMMAND(ID_EMR_VIEW_MEDALLERGY, &CPicContainerDlg::OnViewMedAllergy)
	ON_COMMAND(ID_EMR_VIEW_CAMERABARCODE, &CPicContainerDlg::OnViewBarcode)
	ON_COMMAND(ID_EMR_VIEW_EYEMAGINATIONS, &CPicContainerDlg::OnViewEyemaginations)
	ON_COMMAND(ID_EMR_VIEW_RECALL, &CPicContainerDlg::OnViewRecall)
	// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
	ON_COMMAND(ID_EMR_VIEW_DEVICE_IMPORT, &CPicContainerDlg::OnViewDeviceImport)
	// (j.dinatale 2012-07-13 13:48) - PLID 51481 - do some special handling on save and close
	ON_COMMAND(ID_EMR_SAVE_AND_CLOSE, &CPicContainerDlg::OnSaveAndClose)
	// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
	ON_COMMAND(ID_EMR_VIEW_DRUGINTERACTIONS, &CPicContainerDlg::OnViewDrugInteractions)
	
	/// Merging
	// (a.walling 2012-04-30 13:38) - PLID 50072 - Merging implementation
	ON_COMMAND(ID_EMR_MERGE_FROM_PACKET, &CPicContainerDlg::OnMergeFromPacket)
	ON_COMMAND(ID_EMR_MERGE_FROM_PACKET_TO_WORD, &CPicContainerDlg::OnMergeFromPacketToWord)
	ON_COMMAND(ID_EMR_MERGE_FROM_PACKET_TO_PRINTER, &CPicContainerDlg::OnMergeFromPacketToPrinter)

	ON_COMMAND(ID_EMR_MERGE_FROM_TEMPLATE, &CPicContainerDlg::OnMergeFromTemplate)
	ON_COMMAND(ID_EMR_MERGE_FROM_TEMPLATE_TO_WORD, &CPicContainerDlg::OnMergeFromTemplateToWord)
	ON_COMMAND(ID_EMR_MERGE_FROM_TEMPLATE_TO_PRINTER, &CPicContainerDlg::OnMergeFromTemplateToPrinter)

	ON_COMMAND(ID_EMR_MERGE_EDIT_PACKET, &CPicContainerDlg::OnEditPacket)

	ON_COMMAND(ID_EMR_MERGE_EDIT_TEMPLATE, &CPicContainerDlg::OnEditTemplate)
	ON_COMMAND(ID_EMR_MERGE_NEW_TEMPLATE, &CPicContainerDlg::OnNewTemplate)
	ON_COMMAND(ID_EMR_MERGE_COPY_TEMPLATE, &CPicContainerDlg::OnCopyTemplate)

	ON_COMMAND(ID_EMR_MERGE_PACKET_COMBO, &CPicContainerDlg::OnMergePacketCombo)
	ON_COMMAND(ID_EMR_MERGE_TEMPLATE_COMBO, &CPicContainerDlg::OnMergeTemplateCombo)

	ON_COMMAND(ID_EMR_MERGE_ADVANCED_SETUP, &CPicContainerDlg::OnMergeAdvancedSetup)
	ON_COMMAND(ID_EMR_MERGE_REFRESH, &CPicContainerDlg::OnMergeRefresh)

	ON_COMMAND_RANGE(ID_EMR_MERGE_OPTIONS_BEGIN, ID_EMR_MERGE_OPTIONS_END, &CPicContainerDlg::OnMergeOptions)

	/// Single EMN Merge	

	// (a.walling 2012-10-01 09:15) - PLID 52119 - Handle merging from single EMN, setting default templates

	ON_COMMAND(ID_EMR_EMN_MERGE_FROM_TEMPLATE, &CPicContainerDlg::OnEmnMergeFromTemplate)
	ON_COMMAND(ID_EMR_EMN_MERGE_FROM_TEMPLATE_TO_WORD, &CPicContainerDlg::OnEmnMergeFromTemplateToWord)
	ON_COMMAND(ID_EMR_EMN_MERGE_FROM_TEMPLATE_TO_PRINTER, &CPicContainerDlg::OnEmnMergeFromTemplateToPrinter)

	ON_COMMAND(ID_EMR_EMN_MERGE_FROM_OTHER, &CPicContainerDlg::OnEmnMergeFromOther)
	ON_COMMAND(ID_EMR_EMN_MERGE_FROM_OTHER_TO_WORD, &CPicContainerDlg::OnEmnMergeFromOtherToWord)
	ON_COMMAND(ID_EMR_EMN_MERGE_FROM_OTHER_TO_PRINTER, &CPicContainerDlg::OnEmnMergeFromOtherToPrinter)

	ON_COMMAND(ID_EMR_EMN_MERGE_EDIT_TEMPLATE, &CPicContainerDlg::OnEmnEditTemplate)

	ON_COMMAND(ID_EMR_EMN_MERGE_TEMPLATE_COMBO, &CPicContainerDlg::OnEmnMergeTemplateCombo)

	ON_COMMAND(ID_EMR_EMN_MERGE_TEMPLATE_MAKE_DEFAULT, &CPicContainerDlg::OnEmnMergeTemplateMakeDefault)

	////


	ON_MESSAGE(NXM_CLOSE_PIC, &CPicContainerDlg::OnClosePic)
	ON_WM_CTLCOLOR()
	ON_WM_ACTIVATE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_MESSAGE(NXM_EMR_SAVE_CREATE_BILL, &CPicContainerDlg::OnSaveCreateEMNBill)
	ON_MESSAGE(NXM_EMR_SAVE_CREATE_QUOTE, &CPicContainerDlg::OnSaveCreateEMNQuote)
	ON_MESSAGE(NXM_EMR_PRINT_PRESCRIPTIONS_AND_CLOSE, &CPicContainerDlg::OnSaveWritePrescriptions)
	ON_MESSAGE(NXM_EMR_PRINT_PRESCRIPTIONS, &CPicContainerDlg::OnPrintPrescriptions)
	ON_MESSAGE(NXM_NON_CLINICAL_PROCEDURES_ADDED, &CPicContainerDlg::OnNonClinicalProceduresAdded)
	ON_MESSAGE(NXM_EMR_MINIMIZE_PIC, &CPicContainerDlg::OnEmrMinimizePic)
	ON_MESSAGE(NXM_WIA_EVENT, &CPicContainerDlg::OnWIAEvent)
	ON_MESSAGE(NXM_PIC_RELOAD_PAYS, &CPicContainerDlg::OnReloadProcInfoPays)
	ON_MESSAGE(NXM_ADD_IMAGE_TO_EMR, &CPicContainerDlg::OnAddImageToEMR)
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	ON_MESSAGE(NXM_ADD_GENERIC_TABLE_TO_EMR, OnAddGenericTableToEMR)
	ON_MESSAGE(NXM_EMR_SAVE_CREATE_PARTIAL_BILLS, OnSaveCreatePartialEMNBills)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPicContainerDlg, CEmrPatientFrameWnd)
    //{{AFX_EVENTSINK_MAP(CPicContainerDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
	
/////////////////////////////////////////////////////////////////////////////
// CPicContainerDlg message handlers

#define IDT_AUTO_LAUNCH	1

BOOL CPicContainerDlg::Initialize() 
{
	try {
		// (j.jones 2006-05-23 11:09) - PLID 20764 - Load all common PicContainer properties into the
		// NxPropManager cache
		// (j.armen 2011-10-24 14:40) - PLID 46139 - GetPracPath references ConfigRT
		// (r.gonet 2016-05-19 18:21) - NX-100689 - Get the computer name from the property manager rather
		// than the license object.
		g_propManager.CachePropertiesInBulk("PicContainerDlg", propNumber, "("
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMRColorTop' OR "
/*			"Name = 'PIC_Default_EMR_Merge_Expanded' OR "*/// (j.gruber 2006-12-07 17:16) - PLID 21885 - changed to new preference field
			"Name = 'EMR_Always_Expand_Merge_Settings' OR "
			"Name = 'DefaultProcedurePacket' OR "
			"Name = 'PromptHistoryMerge' OR "
			"Name = 'PicMergeEMNSelection' OR "
			"Name = 'PICSaveDocsInHistory' OR "
			// (j.jones 2007-07-17 13:18) - PLID 26712 - added EnableWord2007Templates
			// (a.walling 2007-07-23 09:10) - PLID 26342 - that preference has been removed
			//TES 2/26/2009 - PLID 33168 - Added PIC_CreateLadderOnProcedureAdded
			"Name = 'PIC_CreateLadderOnProcedureAdded' OR "
			// (j.jones 2011-06-24 12:04) - PLID 36488 - added history tab default filter
			"Name = 'PicHistoryTabDefaultFilterByPIC' OR "
			// (b.savon 2012-02-28 15:55) - PLID 48302 - Auto Create recall
			"Name = 'AutoPatientRecallEMRLinkedDiagCode' "
			// (j.jones 2012-09-27 12:02) - PLID 52820 - I added EMRDrugInteractionChecks and noticed that some
			// settings are cached in CEmrTreeWnd, which is initialized prior to the cache these options were in.
			// So I added these other four items to this cache alongside EMRDrugInteractionChecks.
			"OR Name = 'EMRSavePref' "
			"OR Name = 'EmrAutoCollapse' "
			"OR Name = 'EMNPreviewAutoScroll' "
			"OR Name = 'EMNCompleteInitialSave' "
			"OR Name = 'EMRDrugInteractionChecks' "
			")"

			") OR ("

			// (j.jones 2007-07-17 13:18) - PLID 26712 - added workstation cache option for MergeAllEMNsInPIC
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'MergeAllEMNsInPIC' "
			")"

			")", _Q(GetCurrentUserName()), _Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		BOOL bIsNew = FALSE;
		if(m_nPicID < 0 && m_nProcInfoID < 0 && m_nEmrGroupID < 0)
			bIsNew = TRUE;

		EnsureIDs();

		// (a.walling 2012-04-30 12:47) - PLID 49832 - Merging UI - Load the merge options
		m_mergeOptions.Load();

		// (j.jones 2006-02-07 08:44) - PLID 18944 - generate list of procedures (if not new)
		if(!bIsNew)
			GenerateProcedureList();

		// (j.jones 2006-02-07 08:53) - PLID 18944 - update title bar text
		//GenerateTitleBarText();
		OnUpdateFrameTitle(FALSE);

		// (a.walling 2008-06-27 17:22) - PLID 30482 - For safety's sake, ensure there is
		// not a pre-existing EmrGroup when we go here from another tab
		if (m_nEmrGroupID == -1 && m_nPicID != -1) {
			_RecordsetPtr rsProcInfo = CreateParamRecordset("SELECT EmrGroupID FROM PicT WHERE ID = {INT}", m_nPicID);
			if (!rsProcInfo->eof) {
				m_nEmrGroupID = AdoFldLong(rsProcInfo, "EmrGroupID", -1);
			}
		}

		InitializeEmrEditor(m_nEmrGroupID, m_nPicID, m_nInitialTemplateID, m_nInitialEmnID);


		//(a.wilson 2011-11-8) PLID 46335 - eyemaginations licensing.
		if (!g_pLicense->CheckForLicense(CLicense::lcEyemaginations, CLicense::cflrSilent)) {
			m_strEyePath.Empty();
		} else {
			//(a.wilson 2011-10-14) PLID 45971 - check to see where LUMA is installed on the computer.
			if (PathFileExists(FileUtils::GetShellFolderPath(CSIDL_PROGRAM_FILES) ^ "Eyemaginations LUMA\\.install_root")) {
				m_strEyePath = (FileUtils::GetShellFolderPath(CSIDL_PROGRAM_FILES) ^ "Eyemaginations LUMA\\");
			} else { //this means it is not installed on the machine.
				m_strEyePath.Empty();
			}
		}

		InitializeEmrFrame();


		// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)

		// (c.haag 2010-08-03 09:25) - PLID 39936 - Auto-open the EMR medication allergy window if necessary
		BOOL bShowCurMedAllergyRxWindow = GetRemotePropertyInt("EmrMedAllergyViewerVisible", 0, 0, GetCurrentUserName()) ? TRUE : FALSE;
		if (bShowCurMedAllergyRxWindow) {
			// Don't show the explanation window because having a modal dialog appear here causes problems in the PIC
			// dialog initialization
			ShowMedAllergyViewer(FALSE);
		}

		m_emrPhotosPane.SetPersonID(GetPatientID());
		// (j.jones 2013-09-19 12:22) - PLID 58547 - added ability to set a pointer to the pic container
		m_emrPhotosPane.SetPicContainer(this);

	} NxCatchAllCall("Error in OnInitDialog", ShowWindow(SW_HIDE); return TRUE;);

 	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
void CPicContainerDlg::InitializeEmrEditor(long nEmrGroupID, long nPicID, long nInitialTemplateID, long nInitialEmnID)
{	
	if(nInitialTemplateID != -1) {
		//TES 11/22/2010 - PLID 41582 - Pass in our PIC ID so the EMR can save itself to it.
		GetEmrEditor()->SetEmrWithNewEmn(nEmrGroupID, nPicID, nInitialTemplateID);
	}
	else {
		//TES 11/22/2010 - PLID 41582 - Pass in our PIC ID so the EMR can save itself to it.
		GetEmrEditor()->SetEmr(nEmrGroupID, nPicID, nInitialEmnID);
	}
}

// (a.walling 2011-12-08 17:27) - PLID 46641 - Create views for Labs, History, and ProcInfoCenter
void CPicContainerDlg::InitializePic()
{
	ASSERT(m_nProcInfoID >= 0);
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	m_pPicDocTemplate->GetProcInfoCenterView()->GetProcInfoCenterDlg();
}

CProcInfoCenterDlg* CPicContainerDlg::GetPicDlg(bool bAutoCreate)
{
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	return m_pPicDocTemplate->GetProcInfoCenterView(bAutoCreate)->GetProcInfoCenterDlg();
}

// (a.walling 2011-12-08 17:27) - PLID 46641 - Create views for Labs, History, and ProcInfoCenter
void CPicContainerDlg::InitializeHistory()
{
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	CHistoryView* pHistoryView = m_pPicDocTemplate->GetHistoryView();
}

CHistoryDlg* CPicContainerDlg::GetHistoryDlg(bool bAutoCreate)
{
	return m_pPicDocTemplate->GetHistoryView(bAutoCreate)->GetHistoryDlg();
}

// (a.walling 2011-12-08 17:27) - PLID 46641 - Create views for Labs, History, and ProcInfoCenter
void CPicContainerDlg::InitializeLabs()
{
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	CPatientLabsView* pLabsView = m_pPicDocTemplate->GetPatientLabsView();
}

CPatientLabsDlg* CPicContainerDlg::GetLabsDlg(bool bAutoCreate)
{
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	return m_pPicDocTemplate->GetPatientLabsView(bAutoCreate)->GetPatientLabsDlg();
}

void CPicContainerDlg::OnDestroy() 
{
	try {
		CloseCleanup();
	} NxCatchAll(__FUNCTION__);

	// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
	try {
		// (c.haag 2010-08-03 09:19) - PLID 39336 - Save whether it was opened. We'll pull this value
		// when opening a patient EMR to see whether we need to restore it.
		BOOL bIsMedAllergyViewerVisible = FALSE;
		if (m_pMedAllergyViewerDlg && ::IsWindow(m_pMedAllergyViewerDlg->GetSafeHwnd()))
		{
			bIsMedAllergyViewerVisible = m_pMedAllergyViewerDlg->IsWindowVisible();
		}
		SetRemotePropertyInt("EmrMedAllergyViewerVisible", bIsMedAllergyViewerVisible ? 1 : 0, 0, GetCurrentUserName());

		if (m_pMedAllergyViewerDlg && ::IsWindow(m_pMedAllergyViewerDlg->GetSafeHwnd()))
		{
			m_pMedAllergyViewerDlg->DestroyWindow();
		}

		if (m_pBarcodeDlg) {
			m_pBarcodeDlg->DestroyWindow();
		}

		// (j.jones 2012-09-26 13:58) - PLID 52879 - added a drug interactions dialog that uses the EMR as its parent
		if (m_pDrugInteractionDlg && ::IsWindow(m_pDrugInteractionDlg->GetSafeHwnd())) {
			//this line is seemingly never hit, the window has already been destroyed
			m_pDrugInteractionDlg->DestroyWindow();
		}
	} 
	NxCatchAll(__FUNCTION__ " Tools");
	
	// (b.cardillo 2006-06-09 17:42) - PLID 14292 - Now if we were asked to report our closure 
	// to our caller, then do so.
	GetMainFrame()->SendMessage(NXM_PIC_CONTAINER_CLOSED, 0, (LPARAM)this);

	CEmrPatientFrameWnd::OnDestroy();
}

// (a.walling 2012-05-01 15:31) - PLID 50117 - Tab numbers don't make sense anymore, now it is bShowPIC
// (j.dinatale 2012-07-12 14:23) - PLID 51481 - keep tabs on if we opened our pic externally
// (a.walling 2013-01-17 12:08) - PLID 54666 - Removed unused params
int CPicContainerDlg::OpenPic(long nPicID, bool bShowPIC, long nEmnID /*= -1*/, long nEmnTemplateID /*= -1*/)
{
	m_nInitialTemplateID = nEmnTemplateID;
	m_nInitialEmnID = nEmnID;
	m_nPicID = nPicID;
	m_bIsNew = nPicID == -1;
	m_bLoadedForPic = bShowPIC;

	// (b.cardillo 2006-06-14 08:51) - PLID 14292 - Instead of opening modally, we now just 
	// create and show the window and let it run modelessly.
	Create();

	// (a.walling 2012-05-01 15:31) - PLID 50117 - Now just show the pic
	if (bShowPIC) {		
		// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
		CView* pView = m_pPicDocTemplate->GetProcInfoCenterView(false);

		if (pView) {
			// already exists
			// (a.walling 2013-05-02 12:02) - PLID 52629 - Post a message to ensure this gets handled *after* the other posted MDI activation messages
			m_wndClientArea.PostMessage(WM_MDIACTIVATE, (WPARAM)pView->GetParentFrame()->GetSafeHwnd());
		} else {
			PostMessage(WM_COMMAND, ID_EMR_VIEW_PIC);
		}
	}

	return 0;
}

//TES 6/17/2008 - PLID 30411 - Changed long nBillInsuredPartyID to an LPARAM, which will be interpreted differently 
// based on cpaAction.
BOOL CPicContainerDlg::SaveAndClose(ClosePicAction cpaAction /*= cpaDoNothing*/, CEMN *pEMN /*= NULL*/, LPARAM paramActionInfo /*= NULL*/)
{
	try {		
		// (a.walling 2010-07-27 16:33) - PLID 39433 - destroy empty recording
		CAudioRecordDlg* pCurrentAudioRecordDlgInstance = CAudioRecordDlg::GetCurrentInstance();
		if (pCurrentAudioRecordDlgInstance != NULL && pCurrentAudioRecordDlgInstance->GetPatientID() == GetPatientID() && pCurrentAudioRecordDlgInstance->GetPicID() == m_nPicID) {
			if (pCurrentAudioRecordDlgInstance->IsEmpty()) {
				::DestroyWindow(pCurrentAudioRecordDlgInstance->GetSafeHwnd());
			} else {
				// (a.walling 2010-08-10 16:35) - PLID 39433 - Don't allow audio to record outside of the EMR now.
				MessageBox("Please save or discard the current audio recording before exiting.");
				return FALSE;
			}
		}

		CWaitCursor pWait;

		CString strTemplateName = "";

		//if writing prescriptions, warn for problems prior to saving
		if(cpaAction == cpaWritePrescriptions) {

			if(pEMN) {

				if(!GetWPManager()->CheckWordProcessorInstalled()) {
					return FALSE;
				}

				// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
				// based on how many prescriptions are printed
				long nCountPrescriptions = pEMN->GetMedicationCount();
				BOOL bExactCountFound = FALSE;
				BOOL bOtherCountFound = FALSE;
				strTemplateName = GetDefaultPrescriptionTemplateByCount(nCountPrescriptions, bExactCountFound, bOtherCountFound);

				if(strTemplateName == "") {
					MessageBox(
						"There is no default template set up. The prescription cannot be printed.\n"
						"Please go to the Medications tab to set up your default prescription template."
						, NULL
						, MB_OK|MB_ICONINFORMATION);
					return FALSE;
				}
				//do not bother looking at bExactCountFound and bOtherCountFound,
				//because the "save and print prescriptions" button already did this
			}
			else {
				//shouldn't be possible to save with the cpaWritePrescriptions status and no EMN
				ASSERT(FALSE);
			}
		}

		BOOL bAllowSave = TRUE;

		// (b.cardillo 2005-10-12 13:54) - PLID 17178 - Only give the option to delete the 
		// empty PIC if the PicT record's IsCommitted bit field isn't set true yet.
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		if(m_bIsNew && (!GetPicDlg() || GetPicDlg()->IsEmpty()) && (!GetEmrEditor() || GetEmrEditor()->IsEmpty())
			&& !ReturnsRecordsParam("SELECT MailID FROM MailSent WHERE PicID = {INT} UNION ALL SELECT ID FROM PicT WHERE ID = {INT} AND IsCommitted = 1", m_nPicID, m_nPicID)) {
			int nReturn = MessageBox("Do you want to save this empty PIC?", NULL, MB_ICONEXCLAMATION|MB_YESNOCANCEL);
			if(nReturn == IDCANCEL) {
				return FALSE;
			}
			else if(nReturn == IDNO) {
				// Delete it
				CDWordArray arNewCDSInterventions;
				//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
				if(AttemptRemovePICRecord(m_nPicID, arNewCDSInterventions, TRUE)) {
					// Remember not to try to save/audit it now that it's been deleted
					bAllowSave = FALSE;
				}
				GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
			}
		}


		// (b.cardillo 2005-10-12 13:27) - PLID 17881 - Made it only try to save if the user 
		// didn't choose to discard this PIC.  This protects us from the risk of overwriting 
		// other records that might have been created by someone else during the split second 
		// between the above call to AttemptRemovePICRecord() and the below call to m_pEmrGroupDlg->Save(), 
		// and also prevents the nonsense audit entries from appearing in the audit trail, 
		// which was the main point of the pl item.
		if (bAllowSave) {
			// (b.cardillo 2005-10-12 12:32) - PLID 17178 - If we made it here, the PIC is being 
			// saved, either because it's not new, or because it's not empty, or because it IS new 
			// AND empty AND the user said YES to saving it.  So all we have to do is update the 
			// record to indicate that it's officially "saved".

			// (c.haag 2006-04-12 09:25) - PLID 20040 - Now we only assign this if we haven't already
			if (!GetIsCommitted()) {
				Commit();
			}

			// Save the EMR if we've got one

			// (c.haag 2006-03-28 12:13) - PLID 19890 - We now consider permissions before saving EMR's
			// (a.walling 2007-11-28 11:33) - PLID 28044 - Consider licensing and expiration too
			if (CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
				// (a.walling 2008-06-26 11:02) - PLID 30513 - No point saving if nothing has changed. This was causing lots of
				// modifications to the timestamp which was quite annoying.
				if(GetEmrEditor() != NULL && GetEmrEditor()->IsEMRUnsaved()) {
					//the Save wants to know if we are propagating new IDs,
					//which we normally wouldn't do since we are closing, but we need to
					//if we are performing any actions after saving, such as making a bill

					//TES 8/4/2006 - We want to propogate IDs if the EMR is new, so that just below here we can update the PicT
					// record with the new Emr ID.
					// (j.jones 2012-10-11 18:01) - PLID 52820 - let the Save function know we're closing
					EmrSaveStatus essSaveStatus = GetEmrEditor()->Save(TRUE, TRUE);
					// (a.walling 2008-06-26 16:22) - PLID 30513 - Use FAILED macro to check for success
					if (FAILED(essSaveStatus)) {
						MessageBox("The EMR could not be saved.");
						cpaAction = cpaDoNothing;
						pEMN = NULL;
					}

					//unless we had complete success, do not close
					if(essSaveStatus != essSuccess) {
						return FALSE;
					}
				}
				
				// (a.walling 2008-06-26 11:05) - PLID 30513 - We can modify this wihtout having to save the whole EMR
				if(GetEmrEditor()!= NULL && GetEmrEditor()->GetEmrID() != m_nEmrGroupID && m_nPicID != -1) {
					//They have a new ID, make sure our PIC record is linked to it.
					// (a.walling 2007-10-16 17:11) - PLID 27786 - Verify the EmrGroupID is not in use and is safe to overwrite
					//VerifyEmrGroupID(m_nPicID, GetEmrEditor()->GetEmrID());

					// (a.walling 2008-06-27 18:04) - PLID 30482 - Ensure we are not overwriting an EMRGroup that has already
					// been set for this PIC! If so, we just need to use that EMRGroup instead of this new one, and set our
					// new one to be deleted.
					_RecordsetPtr prs = CreateRecordset(
						"SET NOCOUNT ON "
						"DECLARE @EmrGroupID INT; "
						"SET @EmrGroupID = (SELECT EmrGroupsT.ID FROM PicT INNER JOIN EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID WHERE EmrGroupsT.Deleted = 0 AND PicT.ID = %li) "
						"IF @EmrGroupID IS NULL BEGIN "
							"UPDATE PicT SET EmrGroupID = %li WHERE ID = %li "
						"END ELSE BEGIN SELECT @EmrGroupID AS ExistingEmrGroupID END "
						"SET NOCOUNT OFF ", m_nPicID, GetEmrEditor()->GetEmrID(), m_nPicID
						);

					if (prs->GetState() == adStateOpen && (!prs->eof)) {
						// oh noes!
						long nExistingGroupID = AdoFldLong(prs, "ExistingEmrGroupID", -1);
						if (nExistingGroupID != -1) {
							CEMR* pEMR = GetEmrEditor()->GetEMR();
							if (pEMR) {
								long nOldID = pEMR->GetID();
								pEMR->OverrideID(nExistingGroupID);

								// (a.walling 2012-05-18 17:18) - PLID 50546 - No longer necessary
								/*if (GetEmrEditor() != NULL && GetEmrEditor()->GetSafeHwnd() != NULL) {
									GetEmrEditor()->PostMessage(NXM_EMREDITOR_REFRESH_INFO);
								}	*/
								
								if (nOldID != -1) {
									ExecuteParamSql(
										"UPDATE EmrGroupsT SET Deleted = 1, DeletedBy = {STRING}, DeleteDate = GetDate() WHERE ID = {INT}\r\n "
										"UPDATE EMRGroupsSlipT SET EMRGroupID = {INT} WHERE EMRGroupID = {INT}", CString(GetCurrentUserName())+CString("_Practice"), nOldID, nExistingGroupID, nOldID);
									// (c.haag 2008-07-28 09:43) - PLID 30853 - Migrate all problems linked to the EMR to the existing group ID
									// (c.haag 2009-05-11 17:32) - PLID 28494 - Update the new EMR problem linking table instead
									ExecuteParamSql(FormatString("UPDATE EMRProblemLinkT SET EMRRegardingID = {INT} WHERE EMRRegardingType = %li AND EMRRegardingID = {INT} ", eprtEmrEMR), nExistingGroupID, nOldID);
								}

								m_nEmrGroupID = GetEmrEditor()->GetEmrID();

								MessageBox("Another user has already created an EMR; your EMNs have been modified to use this EMR instead.");

								// (j.jones 2012-10-12 09:09) - PLID 52820 - pass in TRUE for closing
								EmrSaveStatus essSaveAgainStatus = GetEmrEditor()->Save(TRUE, TRUE);
								// (a.walling 2008-06-26 16:22) - PLID 30513 - Use FAILED macro to check for success
								if (FAILED(essSaveAgainStatus)) {
									MessageBox("The EMR could not be saved.");
									cpaAction = cpaDoNothing;
									pEMN = NULL;
								}

								return FALSE;
							}
						} else {
							ASSERT(FALSE);
						}
					}
				}

				if(GetEmrEditor() != NULL) {
					//TES 3/18/2011 - PLID 42762 - Warn them if any of the EMNs have invalid Glasses Order data
					CEMR *pEMR = GetEmrEditor()->GetEMR();
					for(int nEMN = 0; nEMN < pEMR->GetEMNCount(); nEMN++) {
						CEMN *pEMN = pEMR->GetEMN(nEMN);
						GlassesOrder goGlassesOrder;
						CString strIgnoredData;
						if(pEMN->GetGlassesOrderData(goGlassesOrder, strIgnoredData)) {
							if(!strIgnoredData.IsEmpty()) {
								if(IDYES == MessageBox(FormatString("WARNING: The EMN '%s' has items which are associated with Glasses Order data, but "
									"cannot be filled on an actual Glasses Order.  This usually means that the EMN has multiple items selected "
									"for the same field on the Glasses Order dialog, or that invalid data has been entered for a Prescription Number field.  "
									"Would you like to return to editing in order to correct this issue?\r\n"
									"\r\n"
									"If you do not make any changes, the following items will not be included on any Glasses Order created "
									"from this EMN:\r\n"
									"%s", pEMN->GetDescription(), strIgnoredData), NULL, MB_YESNO)) {
										return FALSE;
								}
							}
						}
						//TES 4/11/2012 - PLID 49621 - Now do the same for any invalid Contact Lens Order data
						ContactLensOrder cloContactLensOrder;
						strIgnoredData = "";
						if(pEMN->GetContactLensOrderData(cloContactLensOrder, strIgnoredData)) {
							if(!strIgnoredData.IsEmpty()) {
								if(IDYES == MessageBox(FormatString("WARNING: The EMN '%s' has items which are associated with Contact Lens Order data, but "
									"cannot be filled on an actual Contact Lens Order.  This usually means that the EMN has multiple items selected "
									"for the same field on the Contact Lens Order dialog, or that invalid data has been entered for a Prescription Number field.  "
									"Would you like to return to editing in order to correct this issue?\r\n"
									"\r\n"
									"If you do not make any changes, the following items will not be included on any Contact Lens Order created "
									"from this EMN:\r\n"
									"%s", pEMN->GetDescription(), strIgnoredData), NULL, MB_YESNO)) {
										return FALSE;
								}
							}
						}
					}
				}
			}
		}

		//TES 7/13/2009 - PLID 25154 - We're closing, see if any EMNs want to export their billing information to HL7.
		SendEmnBillsToHL7();


		//variables used for the cpaActions
		long nEMNID = -1;
		if(pEMN)
			nEMNID = pEMN->GetID();

		//Now, hide the dialog, before the subdialogs start disappearing.
		ShowWindow(SW_HIDE);

		//Now fire our actions
		if(bAllowSave) {
			switch(cpaAction) {
				case cpaCreateBill:
				case cpaCreateQuote: {

					if(pEMN) {

						//TES 6/17/2008 - PLID 30414 - There was a bunch of code here to try and deduce the EMN ID if we
						// couldn't get it from pEMN.  However, that hasn't been needed for quite a while, ever since we
						// introduced the PropagateIDs stuff.  I took it all out.
						if(nEMNID == -1) {
							//shouldn't be possible at this point
							ASSERT(FALSE);
						}

						if(cpaAction == cpaCreateBill) {
							//bill this EMR

							//confirm our BillingDlg pointer is valid

							/*CPatientView* pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
							if(!pView || !m_BillingDlg || !m_BillingDlg->GetSafeHwnd() || !IsWindow(m_BillingDlg->GetSafeHwnd())) {
								//if the patients module is closed, our billing pointer is definitely invalid,
								//so we must get it back!
								if(GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
									pView = (CPatientView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
									m_BillingDlg = pView->GetBillingDlg();
								}
							}

							//DRT 7/31/2006 - PLID 21446 - We are closing this PIC window... we don't want to pass a pointer
							//	to us, it will be invalid momentarily!
							//m_BillingDlg->pFinancialDlg = this;

							
							// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
							if (!GetMainFrame()->IsBillingModuleOpen(true)) {
								m_BillingDlg->m_pFinancialDlg = NULL;
								m_BillingDlg->m_nPatientID = pEMN->GetParentEMR()->GetPatientID();

								// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
								m_BillingDlg->OpenWithBillID(-1, 1, 1, TRUE);

								//TES 6/17/2008 - PLID 30411 - For this action, paramActionInfo is the insured party ID to bill.
								long nBillInsuredPartyID = -1;
								if(paramActionInfo) {
									nBillInsuredPartyID = (long)paramActionInfo;
								}
								m_BillingDlg->PostMessage(NXM_BILL_EMR, (long)nEMNID, nBillInsuredPartyID);
							}*/

							// (j.dinatale 2012-01-18 17:42) - PLID 47620 - we now have a controller for this!
							//TES 6/17/2008 - PLID 30411 - For this action, paramActionInfo is the insured party ID to bill.
							long nBillInsuredPartyID = -1;
							if(paramActionInfo) {
								nBillInsuredPartyID = (long)paramActionInfo;
							}

							if(GetMainFrame()){
								if(pEMN->GetParentEMR()){
									GetMainFrame()->m_EMNBillController.BillEntireEMN(nEMNID, pEMN->GetParentEMR()->GetPatientID(), nBillInsuredPartyID);
						}
						}
						}
						else if(cpaAction == cpaCreateQuote) {
							//quote this EMR
							CBillingModuleDlg dlg(this);
							//DRT 7/31/2006 - PLID 21446 - We are closing this PIC window... we don't want to pass a pointer
							//	to us, it will be invalid momentarily!
							//dlg.pFinancialDlg = this;
							m_BillingDlg->m_pFinancialDlg = NULL;
							dlg.m_nPatientID = pEMN->GetParentEMR()->GetPatientID();
							dlg.m_nQuoteEMR = nEMNID;
							// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
							// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
							dlg.OpenWithBillID(-1, BillEntryType::Quote, 1, BillFromType::EMR);
						}
					}

					}
					break;

				case cpaWritePrescriptions: {
					//TES 6/17/2008 - PLID 30411 - For this action, paramActionInfo is BOOL bDirectToPrinter
					// Also, copied this code into a separate function, so we could call it from a couple different message
					// handlers.
					PrintPrescriptions(pEMN, (BOOL)paramActionInfo, strTemplateName);
					}
					break;

				// (j.dinatale 2012-01-18 17:39) - PLID 47539 - we can now do partial billing from EMR
				case cpaDoPartialBilling:
					if(pEMN){
						if(nEMNID == -1) {
							//shouldn't be possible at this point
							ASSERT(FALSE);
						}

						if(GetMainFrame()){
							if(pEMN->GetParentEMR()){
								GetMainFrame()->m_EMNBillController.BillEMNToAssignedResps(nEMNID, pEMN->GetParentEMR()->GetPatientID());
							}
						}
					}
					break;
				case cpaDoNothing:
				default:
					break;
			}//END switch( action )

			// (b.savon 2012-02-28 15:54) - PLID 48302 - Auto create patient recall
			// (j.armen 2012-03-28 09:23) - PLID 48480 - We will only use the license if they actually create a recall, so check silently here
			// (j.jones 2016-02-12 12:58) - PLID 68073 - don't do this if the PIC has no EMR editor
			if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)
				&& GetEmrEditor() != NULL)
			{
				// (j.jones 2016-02-12 12:58) - PLID 68073 - don't do this if the PIC has no EMR linked
				long nEMRID = GetEmrEditor()->GetEmrID();

				long nAutoCreateRecall = GetRemotePropertyInt("AutoPatientRecallEMRLinkedDiagCode", 1, 0, GetCurrentUserName());
				if( nAutoCreateRecall != 0 && nEMRID != -1) {
					try {
						using namespace RecallUtils;
						CArray<RecallListMap, RecallListMap> aryRecallListMap;

						// (j.jones 2016-02-12 13:04) - PLID 68073 - get the patient ID from the PIC, or the EMR if the PIC doesn't have one
						long nPatientID = GetPatientID();
						if (nPatientID == -1) {
							//why doesn't the PIC have a patient ID?
							ASSERT(FALSE);
							nPatientID = GetEmrEditor()->GetPatientID();

							//why doesn't the EMR have one either?
							ASSERT(nPatientID == -1);
						}

						if (nPatientID != -1) {

							//these are for unused return values
							long nProviderID = -1;
							long nLocationID = -1;

							// (a.walling 2013-12-13 10:20) - PLID 60010 - Get a list of template/diag code pairs that can be auto created
							// (r.gonet 04/13/2014) - PLID 60870 - This next function is now shared elsewhere. We need to specify that
							// we only want recall template IDs returned that are the only recall templates linked to a given diagnosis code.
							// (j.jones 2016-02-18 08:41) - PLID 68390 - this now also loads the providerID and locationID
							// (j.jones 2016-02-19 11:03) - PLID 68378 - renamed and added a bool to control auto-creating recalls,
							// which we want to happen here
							CalculateRecallsForEMR(true, nEMRID, nProviderID, nLocationID, aryRecallListMap);
						}
					} NxCatchAll(__FUNCTION__" - Unable to auto-create patient recall.  Please try to manually create the patient recall.");
				}//END AutoCreateRecall
			}
		}

		CloseCleanup();

		// (z.manning 2012-10-01 14:16) - PLID 52962 - We need to actually close the window.
		PostMessage(WM_CLOSE);

		return TRUE;

	}NxCatchAll("Error in CPicContainerDlg::SaveAndClose()");
	
	return FALSE;
}

void CPicContainerDlg::OnClose() 
{
	try {
		// (a.walling 2010-07-27 16:33) - PLID 39433 - Prevent if non-empty recording in progress
		CAudioRecordDlg* pCurrentAudioRecordDlgInstance = CAudioRecordDlg::GetCurrentInstance();
		if (pCurrentAudioRecordDlgInstance != NULL && pCurrentAudioRecordDlgInstance->GetPatientID() == GetPatientID() && pCurrentAudioRecordDlgInstance->GetPicID() == m_nPicID && !pCurrentAudioRecordDlgInstance->IsEmpty()) {
			MessageBox("Please save or discard the current audio recording before exiting.");
			return;
		}

		if(GetEmrEditor()) {
			// (a.walling 2009-11-23 12:32) - PLID 36404 - Check if we are printing
			{				
				// (a.walling 2015-07-09 16:33) - PLID 66504 - Allow users to ignore and continue if the print template teardown did not fire properly
				long nRet = IDTRYAGAIN;
				while (GetEmrEditor()->IsPrinting() && nRet == IDTRYAGAIN) {
					nRet = MessageBox("Please wait for the current print job to complete and be sent to the spooler before exiting. This should complete in less than a minute.", nullptr, MB_CANCELTRYCONTINUE | MB_ICONEXCLAMATION);
					if (nRet == IDCANCEL) {
						return;
					}
				}
			}

			// (a.walling 2012-05-18 17:18) - PLID 50546 - No longer necessary
			//GetEmrEditor()->OnParentClosing();
			// (c.haag 2006-04-04 09:07) - PLID 19890 - Check permissions
			// (a.walling 2007-10-10 17:13) - PLID 25454 - If it is saved, but it still needs the preview copied to documents, do that now
			// (a.walling 2007-11-28 11:34) - PLID 28044 - Also consider licensing
			if(!GetEmrEditor()->IsEMRUnsaved() && CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) ) {
				for (int i = 0; i < GetEmrEditor()->GetEMNCount(); i++) {
					CEMN* pEMNToSave = GetEmrEditor()->GetEMN(i);
					if (pEMNToSave && pEMNToSave->GetNeedToSavePreview()) {
						pEMNToSave->GenerateHTMLFile(FALSE, TRUE, TRUE); // don't send message, copy to documents, ignore unsaved
						pEMNToSave->SetNeedToSavePreview(FALSE);
					}
				}
			}

			if(GetEmrEditor()->IsEMRUnsaved() && CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) ) {
				int nResult = MessageBox("You have made changes to the EMR record, do you wish to save these changes?\n\n"
					"'Yes' will save the changes and close the Procedure Information Center.\n"
					"'No' will discard the changes and close the Procedure Information Center.\n"
					"'Cancel' will cancel closing the Procedure Information Center.", "Practice", MB_ICONEXCLAMATION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL)
					return;
				else if(nResult == IDNO) {
					if(IDYES == MessageBox("All of your EMR changes will be unrecoverable! Are you sure you wish to close without saving?",
						"Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

						//they really want to close without saving

						// (c.haag 2008-07-09 12:36) - PLID 30648 - During the course of editing the EMN, the
						// user may have spawned or unspawned todo alarms. If they're cancelling out, then they
						// need to do something about those alarms. This should be done in the scope of the editor
						// dialog.
						if (!GetEmrEditor()->FindAndResolveChangedEMRTodoAlarms()) {
							// Either the user changed their mind about saving because cancelling out of making a decision about how to deal with
							// outstanding todo alarms, or there were no known changes to make.
							return;
						}

						// (m.hancock 2006-10-06 16:05) - PLID 22302 - Audit when the user chooses to exit without saving any changes.
						// This helps in future issues where clients may be having difficulty interpreting the audit trail, or when we
						// are investigating possible issues with EMR.  We should let each EMN handle its own auditing if it is unsaved,
						// so we just need to tell the EMR that we're closing without saving.
						// (m.hancock 2006-11-28 09:33) - PLID 22302 - Changed the method of retrieving the first EMN because an error
						// could be thrown if all EMNs are deleted from the EMR.  Also added try / catch block.
						/*long nEMNCount = GetEmrEditor()->GetEMNCount();
						if(nEMNCount > 0)
							GetEmrEditor()->GetEMN(0)->GetParentEMR()->AuditCloseUnsaved();*/

						//Get a pointer to the current EMN
						CEMN *pCurrEMN = GetEmrEditor()->GetCurrentEMN();

						// (a.walling 2007-04-17 11:30) - PLID 25454 - Even though we are cancelling changes, we may have saved specific
						// topics. So we need to check for whether a topic has been saved (GetNeedToSavePreview), then generate the HTML
						// file using a special flag to ignore unsaved topics and details and save the state of the last saved detail.
						if (pCurrEMN) {
							CEMN* pSaveEMN = NULL;
							CEMR* pCurrEMR = pCurrEMN->GetParentEMR();
							if (pCurrEMR) {
								for (int i = 0; i < pCurrEMR->GetEMNCount(); i++) {
									pSaveEMN = pCurrEMR->GetEMN(i);
									if (pSaveEMN && pSaveEMN->GetNeedToSavePreview()) {
										pSaveEMN->GenerateHTMLFile(FALSE, TRUE, TRUE); // don't send message, copy to documents, ignore unsaved
										pSaveEMN->SetNeedToSavePreview(FALSE);
									}
								}
							}
						}

						if(pCurrEMN) {
							//Get the parent EMR and tell it to Audit each attempt to close an unsaved EMN.
							pCurrEMN->GetParentEMR()->AuditCloseUnsaved();
						}
						//Else, we don't have a current EMN, so we cannot audit.

						//TES 7/13/2009 - PLID 25154 - We're closing, see if any EMNs want to export their 
						// billing information to HL7.
						SendEmnBillsToHL7();

						ShowWindow(SW_HIDE);

						CloseCleanup();		

						CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
						if(pTreeWnd) {
							// (j.jones 2012-10-25 16:21) - PLID 53322 - This function will check whether a blue Allergies
							// table was on the EMN, and was saved with no allergies. If so, and PatientsT.HasNoAllergies
							// is false, the user will be prompted to fill this setting.
							pTreeWnd->CheckPromptPatientHasNoAllergies(TRUE);

							// (j.jones 2012-10-29 09:20) - PLID 53324 - This function will check whether a blue Current
							// Medications table was on the EMN, and was saved with no meds. If so, and PatientsT.HasNoMeds
							// is false, the user will be prompted to fill this setting.
							pTreeWnd->CheckPromptPatientHasNoCurrentMeds(TRUE);

							// (j.jones 2012-09-28 11:11) - PLID 52820 - Our preferences may request showing
							// drug interactions only upon closing, if anything affected interactions during this
							// session. Call the function to check this now.
							pTreeWnd->TryShowSessionDrugInteractions(FALSE, TRUE);
						}

						CEmrPatientFrameWnd::OnClose();
					}
					
					return;
				}

				//otherwise, keep going, save and close
			}
		}

		if(!SaveAndClose())
			return;

		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(pTreeWnd) {
			// (j.jones 2012-10-25 16:21) - PLID 53322 - This function will check whether a blue Allergies
			// table was on the EMN, and was saved with no allergies. If so, and PatientsT.HasNoAllergies
			// is false, the user will be prompted to fill this setting.
			pTreeWnd->CheckPromptPatientHasNoAllergies(TRUE);

			// (j.jones 2012-10-29 09:20) - PLID 53324 - This function will check whether a blue Current
			// Medications table was on the EMN, and was saved with no meds. If so, and PatientsT.HasNoMeds
			// is false, the user will be prompted to fill this setting.
			pTreeWnd->CheckPromptPatientHasNoCurrentMeds(TRUE);

			// (j.jones 2012-09-28 11:11) - PLID 52820 - Our preferences may request showing
			// drug interactions only upon closing, if anything affected interactions during this
			// session. Call the function to check this now.
			pTreeWnd->TryShowSessionDrugInteractions(FALSE, TRUE);
		}
		
		CEmrPatientFrameWnd::OnClose();

		//SaveAndClose() will have called CDialog:OnOK() so we don't have to here
	}NxCatchAll("Error in CPicContainerDlg::OnBtnClosePic()");
}

struct ProcInfoDetail
{
	long nProcedureID;
	long nMasterProcedureID;
	BOOL bChosen;
};

void CPicContainerDlg::HandleNewEmrProcedures(CArray<long, long> &arProcIDs)
{
	try {
		// (c.haag 2013-12-17) - PLID 60018 - We no longer do all the writing to data in this function. Instead
		// we defer to the tracking namespace, and then we synchronize our member values with data.
		PhaseTracking::HandleNewEmrProcedures(m_nProcInfoID, arProcIDs);

		// Check if any master procedures were added
		bool bProcedureAdded = false;
		int nSurgeryApptID = -1;
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT ProcInfoDetailsT.ProcedureID, ProcInfoT.SurgeryApptID FROM ProcInfoDetailsT "
			"INNER JOIN ProcInfoT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			"INNER JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID "
			"WHERE ProcInfoDetailsT.ProcInfoID = {INT} "
			"AND ProcedureT.MasterProcedureID IS NULL "
			"AND ProcInfoDetailsT.ProcedureID NOT IN ({INTARRAY}) "
			"ORDER BY ProcInfoDetailsT.ID"
			,m_nProcInfoID, m_arPICProcedureIDs);
		if (!prs->eof)
		{
			bProcedureAdded = true;
			nSurgeryApptID = AdoFldLong(prs->Fields, "SurgeryApptID", -1);
			while (!prs->eof)
			{
				m_arPICProcedureIDs.Add(AdoFldLong(prs->Fields, "ProcedureID"));
				prs->MoveNext();
			}
		}

		//DRT 9/6/2007 - PLID 27321 - This need only happen if we actually added a procedure.  This function looks at all the IDs
		//	in m_arPICProcedureIDs, and that array is only modified here when bProcedureAdded is true.  So if no new procedures
		//	were added, then the title bar text would not have changed.
		if(bProcedureAdded) {

			if (nSurgeryApptID == -1) {
				// (a.walling 2008-05-05 11:11) - PLID 29894 - If we do not have a surgery appt, and the procedures have now changed,
				// update and load to ensure we have a valid one.
				UpdateSurgeryAppt();				
			} 

			if(GetPicDlg()) {
				if(GetPicDlg()->m_pProcNames) {
					GetPicDlg()->m_pProcNames->Requery();
				}

				if(GetPicDlg()->IsWindowVisible()) {
					GetPicDlg()->Load(PIC_AREA_ALL);
				}
				else {
					m_bPicDlgNeedsToLoad = TRUE;
				}
			}
		
			//update the window text
			//GenerateTitleBarText();
			OnUpdateFrameTitle(FALSE);
		}

	}NxCatchAll("Error in CPicContainerDlg::HandleNewEmrProcedures()");
}

struct ExistingDetail
{
	long nProcedureID;
	long nMasterProcedureID;
	CString strProcedureName;
};

void CPicContainerDlg::HandleNewProcInfoProcedures(CArray<long, long> &arProcIDs)
{
	try {
		
		for(int i = 0; i < arProcIDs.GetSize(); i++) {
			long nNewProc = arProcIDs.GetAt(i);

			//add to our cached procedure list
			BOOL bFound = FALSE;
			for(int i=0; i<m_arPICProcedureIDs.GetSize() && !bFound; i++) {
				if(m_arPICProcedureIDs.GetAt(i) == nNewProc)
					bFound = TRUE;
			}
			if(!bFound)
				m_arPICProcedureIDs.Add(nNewProc);
		}

		//update the window text
		//GenerateTitleBarText();
		OnUpdateFrameTitle(FALSE);

	}NxCatchAll("Error in CPicContainerDlg::HandleNewProcInfoProcedures()");
}

void CPicContainerDlg::HandleDeletingProcInfoProcedures(CArray<long, long> &arProcIDs)
{
	try {
		//update our cached procedure list
		CArray<long, long> arSafeIDsToRemove;

		for(int i=0; i<arProcIDs.GetSize(); i++) {

			if(GetEmrEditor()) {
				if(!GetEmrEditor()->IsProcedureInEMR(arProcIDs.GetAt(i))) {
					arSafeIDsToRemove.Add(arProcIDs.GetAt(i));
				}
			}
		}

		//now remove from the cached procedure list
		for(i=0;i<arSafeIDsToRemove.GetSize();i++) {
			for(int j=0; j<m_arPICProcedureIDs.GetSize(); j++) {
				if(m_arPICProcedureIDs.GetAt(j) == arSafeIDsToRemove.GetAt(i)) {
					m_arPICProcedureIDs.RemoveAt(j);
					break;
				}
			}
		}

		//update the window text
		//GenerateTitleBarText();
		OnUpdateFrameTitle(FALSE);

	}NxCatchAll("Error in CPicContainerDlg::HandleDeletingProcInfoProcedures()");
}

//TES 5/20/2008 - PLID 27905 - Pass in the EMN that they're being deleted from, so we can take that into account when
// re-generating our list of IDs.
void CPicContainerDlg::HandleDeletingEmrProcedures(CArray<long, long> &arProcIDs, CEMN *pEMN)
{
	try {
		//Find out which procedures are on the PIC, and offer to delete them
		CArray<ExistingDetail, ExistingDetail&> arExistingDetails;
		
		for(int i=0;i<arProcIDs.GetSize();i++) {

			long nProcedureID = (long)arProcIDs.GetAt(i);
			//DRT 8/28/2007 - PLID 27203 - Parameterized.
			_RecordsetPtr rsExistingDetail = CreateParamRecordset("SELECT ProcedureID, MasterProcedureID, ProcedureT.Name "
				"FROM ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
				"WHERE ProcInfoID = {INT} AND (ProcedureID = {INT} OR MasterProcedureID = {INT})",m_nProcInfoID, nProcedureID, nProcedureID);
			if(!rsExistingDetail->eof) {
				
				//the procedure exists on the PIC
				ExistingDetail ed;
				ed.nProcedureID = AdoFldLong(rsExistingDetail, "ProcedureID");
				ed.nMasterProcedureID = AdoFldLong(rsExistingDetail, "MasterProcedureID", -1);
				ed.strProcedureName = AdoFldString(rsExistingDetail, "Name", "");
				arExistingDetails.Add(ed);
			}
		}

		//if none to delete, get out of here
		if(arExistingDetails.GetSize() == 0) {
			//we can safely remove from the cached procedure list
			for(int i=0;i<arProcIDs.GetSize();i++) {
				if(GetEmrEditor()) {
					if(!GetEmrEditor()->IsProcedureInEMR(arProcIDs.GetAt(i), pEMN)) {
						for(int j=0; j<m_arPICProcedureIDs.GetSize(); j++) {
							if(m_arPICProcedureIDs.GetAt(j) == arProcIDs.GetAt(i)) {
								m_arPICProcedureIDs.RemoveAt(j);
								break;
							}
						}
					}
				}
			}
			//update the window text
			//GenerateTitleBarText();
			OnUpdateFrameTitle(FALSE);
			return;
		}

		CString strWarning = "The following procedures have been removed from the EMR, but remain on the PIC:\n\n";
		for(i=0;i<arExistingDetails.GetSize();i++) {
			strWarning += arExistingDetails.GetAt(i).strProcedureName;
			strWarning += "\n";
		}
		strWarning += "\nDo you wish to remove these from the PIC?";

		if(IDNO == MessageBox(strWarning,"Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
			return;
		}

		//okay, now delete these procedures
		
		for(i=0;i<arExistingDetails.GetSize();i++) {
			long nProcedureID = arExistingDetails.GetAt(i).nProcedureID;
			long nMasterProcedureID = arExistingDetails.GetAt(i).nMasterProcedureID;
			if(nMasterProcedureID == -1) {
				ExecuteSql("DELETE FROM ProcInfoDetailsT WHERE (ProcedureID = %li OR ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID = %li)) AND ProcInfoID = %li", nProcedureID, nProcedureID, m_nProcInfoID);
			}
			else {
				ExecuteSql("UPDATE ProcInfoDetailsT SET Chosen = 0 WHERE ProcedureID = %li AND ProcInfoID = %li", nProcedureID, m_nProcInfoID);
			}
		}

		//and now remove from the cached procedure list
		for(i=0;i<arProcIDs.GetSize();i++) {
			if(GetEmrEditor()) {
				if(!GetEmrEditor()->IsProcedureInEMR(arProcIDs.GetAt(i), pEMN)) {
					for(int j=0; j<m_arPICProcedureIDs.GetSize(); j++) {
						if(m_arPICProcedureIDs.GetAt(j) == arProcIDs.GetAt(i)) {
							m_arPICProcedureIDs.RemoveAt(j);
							break;
						}
					}
				}
			}
		}

		if(GetPicDlg()) {
			if(GetPicDlg()->m_pProcNames) {
				GetPicDlg()->m_pProcNames->Requery();
			}

			if(GetPicDlg()->IsWindowVisible()) {
				GetPicDlg()->Load(PIC_AREA_ALL);	
			}
			else {
				m_bPicDlgNeedsToLoad = TRUE;
			}
		}

		//update the window text
		//GenerateTitleBarText();
		OnUpdateFrameTitle(FALSE);

	}NxCatchAll("Error in CPicContainerDlg::HandleDeletingEmrProcedures()");
}

BOOL CPicContainerDlg::GenerateCommonMergeData(const CString& strEMNIDFilter, CList<MergeField,MergeField&> &listMergeFields)
{
	// (c.haag 2004-10-01 16:35) - Add the provider name(s) of the EMR
	{
		MergeField mf;
		mf.strHeader = "EMR_Provider";
		mf.strData = "\"";
		_RecordsetPtr prsProviders = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS FullName FROM EMRMasterT "
			"LEFT JOIN (SELECT * FROM EmrProvidersT WHERE Deleted = 0) AS EmrProvidersT ON EmrMasterT.ID = EmrProvidersT.EmrID LEFT JOIN PersonT ON EmrProvidersT.ProviderID = PersonT.ID WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.ID IN (%s) AND PersonT.ID IS NOT NULL GROUP BY Last, First, Middle ORDER BY Last, First",
			strEMNIDFilter);
		while (!prsProviders->eof)
		{
			// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
			mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsProviders, "FullName", "")) + ", ";
			prsProviders->MoveNext();
		}
		prsProviders->Close();
		if(mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength()-2);
		mf.strData += "\"";
		if(listMergeFields.IsEmpty()) {
			listMergeFields.AddHead(mf);
		}
		else {
			listMergeFields.AddTail(mf);
		}
	}

	//DRT 1/9/2007 - PLID 24160 - Add the secondary provider(s)
	{
		MergeField mf;
		mf.strHeader = "EMR_Provider_Secondary";
		mf.strData = "\"";
		_RecordsetPtr prsProviders = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS FullName FROM EMRMasterT "
			"LEFT JOIN (SELECT * FROM EmrSecondaryProvidersT WHERE Deleted = 0) AS EmrSecondaryProvidersT ON EmrMasterT.ID = EmrSecondaryProvidersT.EmrID LEFT JOIN PersonT ON EmrSecondaryProvidersT.ProviderID = PersonT.ID WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.ID IN (%s) AND PersonT.ID IS NOT NULL GROUP BY Last, First, Middle ORDER BY Last, First",
			strEMNIDFilter);
		while (!prsProviders->eof)
		{
			// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
			mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsProviders, "FullName", "")) + ", ";
			prsProviders->MoveNext();
		}
		prsProviders->Close();
		if(mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength()-2);
		mf.strData += "\"";
		if(listMergeFields.IsEmpty()) {
			listMergeFields.AddHead(mf);
		}
		else {
			listMergeFields.AddTail(mf);
		}
	}

	COleDateTime dtBad;
	dtBad.SetDate(1899, 12, 31);

	// Add the EMR date (range)
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Date";
		CString strMin, strMax;
		_RecordsetPtr prsDate = CreateRecordset("SELECT Min(Date) AS MinDate FROM EMRMasterT WHERE Deleted = 0 AND ID IN (%s)", strEMNIDFilter);
		if (!prsDate->eof) {
			COleDateTime dtMin = AdoFldDateTime(prsDate, "MinDate", dtBad);
			if(dtMin == dtBad) {
				//grab the EMR input date
				//DRT 8/28/2007 - PLID 27203 - Parameterized.
				_RecordsetPtr prsInputDate = CreateParamRecordset("SELECT InputDate FROM EMRGroupsT WHERE ID = {INT}", m_nEmrGroupID);
				if(!prsInputDate->eof) {
					dtMin = AdoFldDateTime(prsInputDate, "InputDate");
					// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
					strMin = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMin, DTF_STRIP_SECONDS, dtoDate, true));
				}
				else {
					strMin = "BAD";
				}
				prsInputDate->Close();
			}
			else {
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				strMin = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMin, DTF_STRIP_SECONDS, dtoDate, true));
			}
		}
		else {
			strMin = "BAD";
		}
		prsDate->Close();
		prsDate = CreateRecordset("SELECT Max(Date) AS MaxDate FROM EMRMasterT WHERE Deleted = 0 AND ID IN (%s)", strEMNIDFilter);
		if (!prsDate->eof) {
			COleDateTime dtMax = AdoFldDateTime(prsDate, "MaxDate", dtBad);
			if(dtMax == dtBad) {
				//grab the EMR input date
				//DRT 8/28/2007 - PLID 27203 - Parameterized.
				_RecordsetPtr prsInputDate = CreateParamRecordset("SELECT InputDate FROM EMRGroupsT WHERE ID = {INT}", m_nEmrGroupID);
				if(!prsInputDate->eof) {
					dtMax = AdoFldDateTime(prsInputDate, "InputDate");
					// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
					strMax = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMax, DTF_STRIP_SECONDS, dtoDate, true));
				}
				else {
					strMax = "BAD";
				}
				prsInputDate->Close();
			}
			else {			
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				strMax = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMax, DTF_STRIP_SECONDS, dtoDate, true));
			}
		}
		else {
			strMax = "BAD";			
		}
		prsDate->Close();

		if(strMin == "BAD" || strMax == "BAD") {
			mf.strData = "\"\"";
		}
		else {
			if(strMin != strMax) {
				mf.strData = "\"" + strMin + " - " + strMax + "\"";
			}
			else {
				mf.strData = "\"" + strMin + "\"";
			}
		}
		listMergeFields.AddTail(mf);
	}

	// Add the EMR input date (range)
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Input_Date";
		CString strMin, strMax;
		_RecordsetPtr prsDate = CreateRecordset("SELECT Min(InputDate) AS MinDate FROM EMRMasterT WHERE ID IN (%s)", strEMNIDFilter);
		if (!prsDate->eof) {
			COleDateTime dtMin = AdoFldDateTime(prsDate, "MinDate", dtBad);
			if(dtMin == dtBad) {
				//grab the EMR input date
				//DRT 8/28/2007 - PLID 27203 - Parameterized.
				_RecordsetPtr prsInputDate = CreateParamRecordset("SELECT InputDate FROM EMRGroupsT WHERE ID = {INT}", m_nEmrGroupID);
				if(!prsInputDate->eof) {
					dtMin = AdoFldDateTime(prsInputDate, "InputDate");
					// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
					strMin = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMin, DTF_STRIP_SECONDS, dtoDate, true));
				}
				else {
					strMin = "BAD";
				}
				prsInputDate->Close();
			}
			else {			
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				strMin = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMin, DTF_STRIP_SECONDS, dtoDate, true));
			}
		}
		else {
			strMin = "BAD";
		}
		prsDate->Close();
		prsDate = CreateRecordset("SELECT Max(InputDate) AS MaxDate FROM EMRMasterT WHERE ID IN (%s)", strEMNIDFilter);
		if (!prsDate->eof) {
			COleDateTime dtMax = AdoFldDateTime(prsDate, "MaxDate", dtBad);
			if(dtMax == dtBad) {
				//grab the EMR input date
				//DRT 8/28/2007 - PLID 27203 - Parameterized.
				_RecordsetPtr prsInputDate = CreateParamRecordset("SELECT InputDate FROM EMRGroupsT WHERE ID = {INT}", m_nEmrGroupID);
				if(!prsInputDate->eof) {
					dtMax = AdoFldDateTime(prsInputDate, "InputDate");
					// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
					strMax = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMax, DTF_STRIP_SECONDS, dtoDate, true));
				}
				else {
					strMax = "BAD";
				}
				prsInputDate->Close();
			}
			else {			
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				strMax = ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtMax, DTF_STRIP_SECONDS, dtoDate, true));
			}
		}
		else {
			strMax = "BAD";
		}
		prsDate->Close();

		if(strMin == "BAD" || strMax == "BAD") {
			mf.strData = "\"\"";
		}
		else {
			if(strMin != strMax) {
				mf.strData = "\"" + strMin + " - " + strMax + "\"";
			}
			else {
				mf.strData = "\"" + strMin + "\"";
			}
		}
		listMergeFields.AddTail(mf);
	}

	// (c.haag 2004-10-01 16:35) - Add the procedure(s) of this EMR
	{
		MergeField mf;
		mf.strHeader = "EMR_Procedure_List";
		mf.strData = "\"";
		_RecordsetPtr prsEMNProcedures = CreateRecordset("SELECT Name FROM ProcedureT LEFT JOIN (SELECT * FROM EMRProcedureT WHERE Deleted = 0) AS EMRProcedureT ON ProcedureT.ID = EMRProcedureT.ProcedureID WHERE EMRID IN (%s) GROUP BY Name", strEMNIDFilter);
		while (!prsEMNProcedures->eof)
		{
			// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
			mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsEMNProcedures, "Name")) + ", ";
			prsEMNProcedures->MoveNext();
		}
		prsEMNProcedures->Close();
		if(mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength()-2);
		mf.strData += "\"";
		listMergeFields.AddTail(mf);
	}

	// (j.jones 2005-03-25 12:02) - PLID 16058 - Add the Service Codes
	{
		// Add the header		
		CString strHeader, strFullPathData;
		strHeader = "EMR_ServiceCode_List";
		strFullPathData.Format("{NXRTF %s_ServiceCodes_%d.htm}", GetNxTempPath() ^ "MergeHTML_EMR", m_nEmrGroupID);

		MergeField mf;
		mf.strHeader = strHeader;
		mf.strData = strFullPathData;
		listMergeFields.AddTail(mf);

		//Create the file
		CString strFilename;
		CString strHTML;

		strFilename.Format("MergeHTML_EMR_ServiceCodes_%d.htm", m_nEmrGroupID);
		CString strFullPath = GetNxTempPath() ^ strFilename;
		CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

		// (j.jones 2012-07-24 12:56) - PLID 44349 - this merge field includes all codes
		CString strHTMLServiceCodes = GetServiceCodesOutput(strEMNIDFilter, FALSE);

		// Write the content to the file
		strHTML = "<html><head></head><body>" + strHTMLServiceCodes + "</body></html>";
		f.WriteString(strHTML);
		f.Close();

		// Add to our temp file list
		m_saTempFiles.Add(strFullPath);
	}

	// (j.jones 2012-07-24 12:47) - PLID 44349 - added billable service codes
	{
		// Add the header		
		CString strHeader, strFullPathData;
		strHeader = "EMR_BillableServiceCode_List";
		strFullPathData.Format("{NXRTF %s_BillableServiceCodes_%d.htm}", GetNxTempPath() ^ "MergeHTML_EMR", m_nEmrGroupID);

		MergeField mf;
		mf.strHeader = strHeader;
		mf.strData = strFullPathData;
		listMergeFields.AddTail(mf);

		//Create the file
		CString strFilename;
		CString strHTML;

		strFilename.Format("MergeHTML_EMR_BillableServiceCodes_%d.htm", m_nEmrGroupID);
		CString strFullPath = GetNxTempPath() ^ strFilename;
		CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

		// (j.jones 2012-07-24 12:56) - PLID 44349 - this merge field includes only billable codes
		CString strHTMLServiceCodes = GetServiceCodesOutput(strEMNIDFilter, TRUE);

		// Write the content to the file
		strHTML = "<html><head></head><body>" + strHTMLServiceCodes + "</body></html>";
		f.WriteString(strHTML);
		f.Close();

		// Add to our temp file list
		m_saTempFiles.Add(strFullPath);
	}

	// (a.walling 2006-12-10 22:09) - PLID 17298 - Add medication merge fields
	{
		// Add the header		
		CString strHeader, strFullPathData;
		strHeader = "EMR_Medication_List";
		strFullPathData.Format("{NXRTF %s_Medications_%d.htm}", GetNxTempPath() ^ "MergeHTML_EMR", m_nEmrGroupID);

		MergeField mf;
		mf.strHeader = strHeader;
		mf.strData = strFullPathData;
		listMergeFields.AddTail(mf);

		//Create the file
		CString strFilename;
		CString strHTML;

		strFilename.Format("MergeHTML_EMR_Medications_%d.htm", m_nEmrGroupID);
		CString strFullPath = GetNxTempPath() ^ strFilename;
		CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

		CString strHTMLMedications = GetMedicationsOutput(strEMNIDFilter);

		// Write the content to the file
		strHTML = "<html><head></head><body>" + strHTMLMedications + "</body></html>";
		f.WriteString(strHTML);
		f.Close();

		// Add to our temp file list
		m_saTempFiles.Add(strFullPath);
	}
	
	// (j.jones 2005-03-25 12:02) - PLID 16058 - Add the ICD-9 Codes
	{
		// Add the header		
		CString strHeader, strFullPathData;
		strHeader = "EMR_DiagCode_List";
		strFullPathData.Format("{NXRTF %s_DiagCodes_%d.htm}", GetNxTempPath() ^ "MergeHTML_EMR", m_nEmrGroupID);

		MergeField mf;
		mf.strHeader = strHeader;
		mf.strData = strFullPathData;
		listMergeFields.AddTail(mf);

		//Create the file
		CString strFilename;
		CString strHTML;

		strFilename.Format("MergeHTML_EMR_DiagCodes_%d.htm", m_nEmrGroupID);
		CString strFullPath = GetNxTempPath() ^ strFilename;
		CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

		CString strHTMLDiagCodes = GetDiagCodesOutput(strEMNIDFilter);

		// Write the content to the file
		strHTML = "<html><head></head><body>" + strHTMLDiagCodes + "</body></html>";
		f.WriteString(strHTML);
		f.Close();

		// Add to our temp file list
		m_saTempFiles.Add(strFullPath);
	}

	// Add the EMR notes
	{
		// Add the header and value
		// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
		MergeField mf;
		mf.strHeader = "EMR_Notes";
		mf.strData.Format("\"%s\"", ConvertToQuotableWordCSVString(GenerateDelimitedListFromRecordsetColumn(FormatString(
			"SELECT AdditionalNotes FROM EMRMasterT WHERE ID IN (%s) ", strEMNIDFilter), "AdditionalNotes", ", ")));
		listMergeFields.AddTail(mf);
	}

	// (c.haag 2004-10-01 16:35) - Add the collection(s) of this EMR
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Collection";
		_RecordsetPtr prsCollection = CreateRecordset("SELECT Name FROM EMRCollectionT WHERE ID IN (SELECT EMRCollectionID FROM EMRMasterT WHERE EMRMasterT.ID IN (%s)) GROUP BY Name",	strEMNIDFilter);
		if(prsCollection->eof) {
			mf.strData = "\"<Generic Collection>\"";
		}
		else {
			mf.strData = "\"";
			while (!prsCollection->eof)
			{
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsCollection, "Name","")) + ", ";
				prsCollection->MoveNext();
			}
			if(mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength()-2);
			mf.strData += "\"";
		}
		listMergeFields.AddTail(mf);
	}

	// (a.wetta 2006-04-13 08:50) - PLID 13905 - Add the EMR's surgery appointment
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Surgery_Appt";
		//DRT 8/28/2007 - PLID 27203 - Parameterized.
		_RecordsetPtr prs = CreateParamRecordset("select AptTypeT.Name, AppointmentsT.StartTime "
											"from AppointmentsT "
											"left join ProcInfoT on AppointmentsT.ID = ProcInfoT.SurgeryApptID "
											"left join AptTypeT on AppointmentsT.AptTypeID = AptTypeT.ID "
											"where ProcInfoT.ID = {INT}", m_nProcInfoID);
		if(prs->eof) {
			mf.strData = "\"\"";
		}
		else {
			CString str;
			str.Format("%s: %s", AdoFldString(prs, "Name",""), AdoFldDateTime(prs, "StartTime").Format("%x %#I:%M %p"));
			
			// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
			mf.strData = "\"" + ConvertToQuotableWordCSVString(str) + "\"";
		}
		listMergeFields.AddTail(mf);		
	}

	// (a.wetta 2006-04-13 08:50) - PLID 13905 - Add the EMR's other appointments list
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Other_Appts_List";
		//DRT 8/28/2007 - PLID 27203 - Parameterized.
		_RecordsetPtr prs = CreateParamRecordset("select AptTypeT.Name, AppointmentsT.StartTime "
											"from AppointmentsT  "
											"left join AptTypeT on AppointmentsT.AptTypeID = AptTypeT.ID "
											"WHERE PatientID = {INT} "
											"AND AppointmentsT.ID <> (SELECT  CASE WHEN SurgeryApptID Is Null THEN -1 ELSE SurgeryApptID END FROM ProcInfoT WHERE ID = {INT}) "
											"AND (AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = {INT})) OR AppointmentsT.ID IN (SELECT AppointmentID FROM ProcInfoAppointmentsT WHERE ProcInfoID = {INT})) "
											"AND AppointmentsT.Status <> 4 "
											"order by AppointmentsT.StartTime",
											GetPatientID(), m_nProcInfoID, m_nProcInfoID, m_nProcInfoID);
		if(prs->eof) {
			mf.strData = "\"\"";
		}
		else {
			mf.strData = "\"";
			CString str;
			while (!prs->eof) {
				str.Format("%s: %s", AdoFldString(prs, "Name",""), AdoFldDateTime(prs, "StartTime").Format("%x %#I:%M %p"));
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				mf.strData += ConvertToQuotableWordCSVString(str) + ", ";
				prs->MoveNext();
			}
			if(mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength()-2);
			mf.strData += "\"";
		}
		prs->Close();
		listMergeFields.AddTail(mf);		
	}

	// (j.jones 2007-08-16 09:36) - PLID 27054 - added Visit Type
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Visit_Type";
		mf.strData = "\"";
		_RecordsetPtr prsEMNVisitTypes = CreateRecordset("SELECT Name FROM EMRVisitTypesT INNER JOIN (SELECT VisitTypeID FROM EMRMasterT WHERE Deleted = 0 AND ID IN (%s)) AS EMRMasterQ ON EMRVisitTypesT.ID = EMRMasterQ.VisitTypeID GROUP BY Name", strEMNIDFilter);
		while (!prsEMNVisitTypes->eof)
		{
			// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
			mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsEMNVisitTypes, "Name")) + ", ";
			prsEMNVisitTypes->MoveNext();
		}
		prsEMNVisitTypes->Close();
		if(mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength()-2);
		mf.strData += "\"";
		listMergeFields.AddTail(mf);
	}


	// (b.eyers 2016-02-23) - PLID 68323 - added discharge status description
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Discharge_Status_Description";

		mf.strData = "\"";
		_RecordsetPtr prsDischargeStatus = CreateRecordset("SELECT Description FROM DischargeStatusT INNER JOIN "
			"(SELECT DischargeStatusID FROM EMRMasterT WHERE Deleted = 0 AND ID IN(%s)) AS EMRMasterQ ON DischargeStatusT.ID = EMRMasterQ.DischargeStatusID GROUP BY Description", strEMNIDFilter);
		while (!prsDischargeStatus->eof)
		{
			mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsDischargeStatus, "Description")) + ", ";
			prsDischargeStatus->MoveNext();
		}
		prsDischargeStatus->Close();
		if (mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength() - 2);
		mf.strData += "\"";

		listMergeFields.AddTail(mf);
	}

	// (b.eyers 2016-02-23) - PLID 68323 - added discharge status code
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Discharge_Status_Code";

		mf.strData = "\"";
		_RecordsetPtr prsDischargeStatusCode = CreateRecordset("SELECT Code FROM DischargeStatusT INNER JOIN "
			"(SELECT DischargeStatusID FROM EMRMasterT WHERE Deleted = 0 AND ID IN(%s)) AS EMRMasterQ ON DischargeStatusT.ID = EMRMasterQ.DischargeStatusID GROUP BY Code", strEMNIDFilter);
		while (!prsDischargeStatusCode->eof)
		{
			mf.strData += ConvertToQuotableWordCSVString(AdoFldString(prsDischargeStatusCode, "Code")) + ", ";
			prsDischargeStatusCode->MoveNext();
		}
		prsDischargeStatusCode->Close();
		if (mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength() - 2);
		mf.strData += "\"";

		listMergeFields.AddTail(mf);
	}


	// (b.eyers 2016-02-23) - PLID 68323 - added admission time
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Admission_Time";

		COleDateTime dtTime;
		mf.strData = "\"";
		_RecordsetPtr prsAdmissionTime = CreateRecordset("SELECT AdmissionTime FROM EMRMasterT WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.ID IN(%s) AND AdmissionTime IS NOT NULL",
			strEMNIDFilter);
		while (!prsAdmissionTime->eof)
		{
			dtTime = AdoFldDateTime(prsAdmissionTime, "AdmissionTime");
			mf.strData += ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtTime, DTF_STRIP_SECONDS, dtoTime, true)) + ", ";
			prsAdmissionTime->MoveNext();
		}
		prsAdmissionTime->Close();
		if (mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength() - 2);
		mf.strData += "\"";
		if (listMergeFields.IsEmpty()) {
			listMergeFields.AddHead(mf);
		}
		else {
			listMergeFields.AddTail(mf);
		}
	}

	// (b.eyers 2016-02-23) - PLID 68323 - added discharge time
	{
		// Add the header and value
		MergeField mf;
		mf.strHeader = "EMR_Discharge_Time";

		COleDateTime dtTime;
		mf.strData = "\"";
		_RecordsetPtr prsDischargeTime = CreateRecordset("SELECT DischargeTime FROM EMRMasterT WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.ID IN(%s) AND DischargeTime IS NOT NULL",
			strEMNIDFilter);
		while (!prsDischargeTime->eof)
		{
			dtTime = AdoFldDateTime(prsDischargeTime, "DischargeTime");
			mf.strData += ConvertToQuotableWordCSVString(FormatDateTimeForInterface(dtTime, DTF_STRIP_SECONDS, dtoTime, true)) + ", ";
			prsDischargeTime->MoveNext();
		}
		prsDischargeTime->Close();
		if (mf.strData.Right(2) == ", ") mf.strData = mf.strData.Left(mf.strData.GetLength() - 2);
		mf.strData += "\"";
		if (listMergeFields.IsEmpty()) {
			listMergeFields.AddHead(mf);
		}
		else {
			listMergeFields.AddTail(mf);
		}
	}
	return TRUE;
}

// (j.jones 2012-07-24 12:49) - PLID 44349 - added filter for only billable codes
CString CPicContainerDlg::GetServiceCodesOutput(CString strEMNIDFilter, BOOL bOnlyIncludeBillableCodes)
{
	CString strOut = "";

	//do not group by code, show all of them
	// (j.jones 2012-07-24 12:53) - PLID 44349 - optionally filter on billable CPT codes
	// (all charges that are not CPT codes are considered billable)
	_RecordsetPtr rs = CreateParamRecordset("SELECT CPTCodeT.Code, ServiceT.Name "
			"FROM EMRChargesT INNER JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID "
			"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"WHERE EMRChargesT.Deleted = 0 AND EMRID IN ({INTSTRING}) "
			"AND ({INT} = 0 OR Coalesce(CPTCodeT.Billable,1) = 1) "
			"ORDER BY CPTCodeT.Code ", strEMNIDFilter, bOnlyIncludeBillableCodes ? 1 : 0);

	while(!rs->eof) {
		CString strCode = AdoFldString(rs, "Code","");
		CString strDesc = AdoFldString(rs, "Name","");
		
		strOut += strCode;
		if(!strCode.IsEmpty())
			strOut += " - ";
		strOut += strDesc;
		strOut += "<br>";

		rs->MoveNext();
	}
	rs->Close();

	if(strOut.Right(4) == "<br>")
		strOut = strOut.Left(strOut.GetLength()-4);

	return strOut;
}

CString CPicContainerDlg::GetDiagCodesOutput(CString strEMNIDFilter)
{
	CString strOut = "";

	// (j.jones 2007-01-05 10:02) - PLID 24070 - supported OrderIndex
	//(r.wilson 3/7/2014) PLID 61269 - Updated query to pull in ICD10 Code/Desc

	_RecordsetPtr rs = CreateRecordset(
		"SELECT "
		"		DiagCodes.CodeNumber AS CodeNumberICD9, Max(DiagCodes.CodeDesc) AS DescriptionICD9, "
		"		DiagCodesICD10.CodeNumber AS CodeNumberICD10, Max(DiagCodesICD10.CodeDesc) AS DescriptionICD10 "
		"FROM  "
		"		EMRDiagCodesT LEFT JOIN DiagCodes ON EMRDiagCodesT.DiagCodeID = DiagCodes.ID  "
		"		LEFT JOIN DiagCodes AS DiagCodesICD10 ON EMRDiagCodesT.DiagCodeID_ICD10 = DiagCodesICD10.ID "
		"WHERE  "
		"	EMRDiagCodesT.Deleted = 0 AND EMRID IN (%s)"
		"Group By "
		"	 DiagCodes.CodeNumber , DiagCodesICD10.CodeNumber "
		"ORDER BY Min(OrderIndex)", strEMNIDFilter
		);

	while(!rs->eof) {
		CString strCodeICD9 = AdoFldString(rs, "CodeNumberICD9","");
		CString strDescICD9 = AdoFldString(rs, "DescriptionICD9","");
		CString strCodeICD10 = AdoFldString(rs, "CodeNumberICD10","");
		CString strDescICD10 = AdoFldString(rs, "DescriptionICD10", "");
		CString strTmp = "";

		//(r.wilson 3/7/2014) PLID 61269 -
		// 1. If an ICD-9 & ICD-10 exists, display: "ICD-10 code - ICD-10 description (ICD-9 code)"		
		if(strCodeICD9 != "" && strCodeICD10 != "")
		{
			strTmp.Format("%s - %s (%s)",strCodeICD10, strDescICD10, strCodeICD9);
			strOut += strTmp;
		}
		//(r.wilson 3/7/2014) PLID 61269 -
		// 2. If only an ICD-9 exists, display is unchanged from current style: "ICD-9 code - ICD-9 description"
		else if(strCodeICD10 == "" && strCodeICD9 != "")
		{
			strTmp.Format("%s - %s",strCodeICD9, strDescICD9);
			strOut += strTmp;
		}
		//(r.wilson 3/7/2014) PLID 61269 -
		// 3. If only an ICD-10 exists, display: "ICD-10 code - ICD-10 description"
		else if(strCodeICD10 != "" && strCodeICD9 == "")
		{
			strTmp.Format("%s - %s",strCodeICD10, strDescICD10);
			strOut += strTmp;
		}
		
		strOut += "<br>";

		rs->MoveNext();
	}
	rs->Close();

	if(strOut.Right(4) == "<br>")
		strOut = strOut.Left(strOut.GetLength()-4);

	return strOut;
}

// (a.walling 2006-12-11 13:14) - PLID 17298 - Add emn medications merge fields
CString CPicContainerDlg::GetMedicationsOutput(CString strEMNIDFilter)
{
	CString strOut = "";

	// (c.haag 2007-02-02 18:28) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
	_RecordsetPtr rs = CreateRecordset("SELECT EMRDataT.Data AS Name, PatientMedications.RefillsAllowed, "
		"PatientMedications.Quantity, PatientMedications.Unit, PatientMedications.PatientExplanation "
		"FROM EMRMedicationsT "
		"LEFT JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
		"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
		"WHERE EMRMedicationsT.Deleted = 0 "
		"AND EMRMedicationsT.EmrID IN (%s) "
		"ORDER BY EMRMedicationsT.EmrID, EMRDataT.Data", strEMNIDFilter);

	while(!rs->eof) {
		CString strPatientExplanation = AdoFldString(rs, "PatientExplanation", "");
		strPatientExplanation.Remove('\r');
		strPatientExplanation.Replace("\n", " / ");

		strOut += AdoFldString(rs, "Name", "");
		strOut += " - ";
		strOut += AdoFldString(rs, "Quantity", "") + " " + AdoFldString(rs, "Unit", "") + " (" + AdoFldString(rs, "RefillsAllowed", "") + " refills)";
		if (!strPatientExplanation.IsEmpty()) {
			strOut += " - " + strPatientExplanation;
		}
		strOut += "<br>";

		rs->MoveNext();
	}

	if(strOut.Right(4) == "<br>")
		strOut = strOut.Left(strOut.GetLength()-4);

	return strOut;
}


BOOL CPicContainerDlg::GenerateCategoryMergeData(CList<MergeField,MergeField&> &listMergeFields,
												 BOOL bEditingTemplate)
{	
	// (a.walling 2010-08-16 11:48) - PLID 37580 - Built-in Signatures category
	CString strEMRSignaturesCatFieldName;
	{
		CString strEMRSignaturesCatFieldName = ConvertToHeaderName("EMR_Category", "Signatures");
		// We still need just the category name cleaned up but without the "EMR_Category_" at the front
		CString strCat = strEMRSignaturesCatFieldName.Mid(13);
		CString strFullPathData;		
		if (!bEditingTemplate) {
			strFullPathData.Format("{NXRTF %s_%s_%d.htm}", GetNxTempPath() ^ "MergeHTML_Cat_EMR", strCat, m_nEmrGroupID);
		}

		MergeField mf;
		mf.strHeader = strEMRSignaturesCatFieldName;

		// Add to the output value text
		mf.strData = strFullPathData;

		// (a.walling 2010-06-09 18:09) - PLID 37580 - What is this, CList superstition?
		if(listMergeFields.IsEmpty()) {
			listMergeFields.AddHead(mf);
		}
		else {
			listMergeFields.AddTail(mf);
		}
	}

	_RecordsetPtr prsEMRCat = CreateRecordset("SELECT EMRCategoriesT.Name AS Name FROM EMRCategoriesT ORDER BY EMRCategoriesT.Name");
	while (!prsEMRCat->eof)
	{
		// Get the merge field name based on this category
		// (b.cardillo 2004-06-02 13:01) - For some reason this used to do the work of EMR_ConvertToHeaderName() in 
		// place here instead of just calling it.  (This replaces (c.haag 2004-05-27 08:56) PLID 12614.)
		CString strCatFieldName = ConvertToHeaderName("EMR_Category", AdoFldString(prsEMRCat, "Name"));
		// We still need just the category name cleaned up but without the "EMR_Category_" at the front
		CString strCat = strCatFieldName.Mid(13);
		CString strFullPathData;
		// (c.haag 2004-10-07 10:05) - If we're editing a template, there is no need
		// to generate an NXRTF string since the document won't exist.
		if (!bEditingTemplate) {
			strFullPathData.Format("{NXRTF %s_%s_%d.htm}", GetNxTempPath() ^ "MergeHTML_Cat_EMR", strCat, m_nEmrGroupID);
		}
		// Add to the output header text
		MergeField mf;
		mf.strHeader = strCatFieldName;

		// Add to the output value text
		mf.strData = strFullPathData;
		if(listMergeFields.IsEmpty()) {
			listMergeFields.AddHead(mf);
		}
		else {
			listMergeFields.AddTail(mf);
		}
		prsEMRCat->MoveNext();
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
//
// (c.haag 2004-10-04 10:39) - The code content was copied and modified from
// EMRDlg.cpp
//
////////////////////////////////////////////////////////////////////////////
CString CPicContainerDlg::GetParagraph(CArray<CEMNDetail*, CEMNDetail*>& aDetails, long nCategoryID, EmrCategoryFormat Format, CMergeEngine &mi, CMap<long,long, CArray<long,long>*, CArray<long,long>*>& mapEmrInfoCat)
{
	CString strOutput;
	//First, begin the list based on its format.
	switch(Format) {
	case ecfParagraph:
		//Do nothing.
		break;
	case ecfList:
		// (j.jones 2005-01-04 11:59) - not required here, or else we'd be adding an extraneous blank line
		//strOutput += "<br>";
		break;
	case ecfBulletedList:
		strOutput += "<UL>";
		break;
	case ecfNumberedList:
		strOutput += "<OL>";
		break;
	}

	//TES 8/31/2004: Re-order our list of items based on tab, then y-pos, then x-pos.
	// (j.jones 2006-02-13 12:27) - we don't use tab/topic indices anymore,
	// but the list is already generated in topic order
	CEMRTopic *pLastTopic = NULL;
	CList<int,int> listOrderedIndices; //This will be an ordered list of indexes into m_arypEMNDetails.
	int i;

	for(i = 0; i < aDetails.GetSize(); i++) {

		// (j.jones 2006-03-28 10:00) - track when we've started loading a new topic
		// (we are already looping in order by topic, so just check
		// that it is not the same parent as the last item)
		BOOL bNewTopic = FALSE;
		if(pLastTopic != aDetails[i]->m_pParentTopic)
			bNewTopic = TRUE;
		pLastTopic = aDetails[i]->m_pParentTopic;


		//
		// (c.haag 2006-06-22 09:02) - PLID 21169 - We used to use 3ItemCategories,
		// which actually pulls EMR Info category relationship information from data. What
		// we do now is use mapEmrInfoCat; which is basically a copy of the relevant values
		// of EmrInfoCategoryT loaded into memory. The usage is
		//
		// mapEmrInfoCat[ nEmrInfoID ] = CArray< category ids >
		//
		// By not querying data in this loop, we get a big performance increase
		//
		
		// (a.walling 2010-08-16 12:00) - PLID 37580 - Built-in Signatures category
		if (nCategoryID != -26) {
			CArray<long,long>* pary;
			if (!mapEmrInfoCat.Lookup(aDetails[i]->m_nEMRInfoID, pary)) {
				continue;
			} else {
				BOOL bFound = FALSE;
				for (int i=0; i < pary->GetSize() && !bFound; i++) {
					if (nCategoryID == pary->GetAt(i)) {
						bFound = TRUE;
					}
				}
				if (!bFound) {
					continue;
				}
			}
		}
		/*
		// (c.haag 2004-10-11 13:42) - PLID 14242 - Make sure the category ID matches
		// (j.jones 2005-09-14 12:42) - PLID 17524 - There are multiple categories per item
		CArray<long,long> arCategoryIDs;
		GetEmrItemCategories(aDetails[i]->m_nEMRInfoID, arCategoryIDs);
		if(arCategoryIDs.GetSize() == 0)
			continue;

		BOOL bFound = FALSE;
		for(int j=0;j<arCategoryIDs.GetSize() && !bFound;j++) {
			if ((long)(arCategoryIDs.GetAt(j)) == nCategoryID) {
				bFound = TRUE;
			}
		}

		//not in this category
		if(!bFound)
			continue;
		*/

		int nCurIndex = -1;
		bool bInserted = false;
		for(POSITION pos = listOrderedIndices.GetHeadPosition(); pos;) {
			nCurIndex = listOrderedIndices.GetNext(pos);
			//Is this item later than our current item?

			//if we have moved to a new topic...
			if(bNewTopic) {
				//add to the end
				pos = listOrderedIndices.GetTailPosition();
				listOrderedIndices.InsertAfter(pos, i);
				pos = NULL;
				bInserted = true;
			}
			else {
				//Well, they're on the same topic
				CRect rnCurIndex = aDetails.GetAt(nCurIndex)->m_rcDefaultClientArea;
				CRect ri = aDetails.GetAt(i)->m_rcDefaultClientArea;
				if(aDetails.GetAt(nCurIndex)->m_pParentTopic == aDetails.GetAt(i)->m_pParentTopic
					&& (rnCurIndex.top > ri.top || (rnCurIndex.top == ri.top && rnCurIndex.left > ri.left))) {
					//Yes.
					if(pos)
						listOrderedIndices.GetPrev(pos);
					else
						pos = listOrderedIndices.GetTailPosition();
					listOrderedIndices.InsertBefore(pos, i);
					pos = NULL;
					bInserted = true;
				}
			}
		}
		if(!bInserted) {
			//Add to the end.
			if(listOrderedIndices.IsEmpty()) 
				listOrderedIndices.AddHead(i);
			else
				listOrderedIndices.AddTail(i);
		}
	}


	//Loop through all our items, using our new ordered list.
	int nCurIndex = -1;
	for(POSITION pos = listOrderedIndices.GetHeadPosition(); pos;) {
		nCurIndex = listOrderedIndices.GetNext(pos);
		//Is this item in this category (and what's its long form?)
		long nItemID = aDetails.GetAt(nCurIndex)->m_nEMRInfoID;
		BOOL bExists = FALSE;
		CArray<long,long>* pary;

		// (c.haag 2006-06-22 13:46) - PLID 21169 - Look up the map instead of the data to see if the
		// item and category pair exists
		// (a.walling 2010-08-16 12:00) - PLID 37580 - Built-in Signatures category
		if (nCategoryID != -26) {
			if (mapEmrInfoCat.Lookup(nItemID, pary)) {
				for (int i=0; i < pary->GetSize() && !bExists; i++) {
					if (nCategoryID == pary->GetAt(i)) {
						bExists = TRUE;
					}
				}
			}
		}

		//if(ReturnsRecords("SELECT * FROM EmrInfoCategoryT WHERE EMRCategoryID = %li AND EMRInfoID = %li", nCategoryID, nItemID)) {
		// (a.walling 2010-08-16 12:00) - PLID 37580 - Built-in Signatures category
		if (bExists || (nCategoryID == -26 && aDetails.GetAt(nCurIndex)->IsSignatureDetail()) )
		{
			//Yup.  Add it to our list.

			//TES 9/2/2004: Skip the tags if there's no sentence.
			CString strSentence = GetSentence(aDetails[nCurIndex], &mi, true, true, m_saTempFiles, Format);
			if(!strSentence.IsEmpty()) {

				//Put the pre-sentence tag, based on the format.
				switch(Format) {
				case ecfParagraph:
					//Nothing
					break;
				case ecfList:
					// (j.jones 2005-01-04 11:58) - required here, if it's not the first item
					if(strOutput != "")
						strOutput += "<br>";
					break;
				case ecfBulletedList:
				case ecfNumberedList:
					strOutput += "<LI>";
					break;
				}


				//Get the actual sentence.
				strOutput += strSentence;
				
				//Put the post-sentence info, based on the format.
				switch(Format) {
				case ecfParagraph:
					strOutput += " ";
					break;
				//case ecfList:
				case ecfNumberedList:
				case ecfBulletedList:
					strOutput += "<br>";
					break;
				}
			}

			//Done with this item.
		}
	}

	// (z.manning 2008-12-10 12:26) - PLID 32392 - For paragraph formatted categories we put a space in between
	// each detail's text. That's fine, but make sure we trim the final space so we're not adding a spece where
	// it doesn't make sense e.g. when a period immediately follows the category merge field.
	if(strOutput.GetLength() > 0) {
		int nLastCharPos = strOutput.GetLength() - 1;
		if(Format == ecfParagraph && strOutput.GetAt(nLastCharPos) == ' ') {
			strOutput.Delete(nLastCharPos);
		}
	}

	//Now the post-paragraph info, based on the format.
	// (j.jones 2005-02-11 14:41) - PLID 15483 - bulleted/numbered lists require a <br>
	// after them to be more compliant with Word. Read this PL item to understand why.
	switch(Format) {
	case ecfParagraph:
	case ecfList:
		//Nothing.
		break;
	case ecfBulletedList:
		strOutput += "</UL>";
		if(strOutput != "<UL></UL>")
			strOutput += "<br>";
		break;
	case ecfNumberedList:
		strOutput += "</OL>";
		if(strOutput != "<OL></OL>")
			strOutput += "<br>";
		break;
	}

	//Done, baby!
	return strOutput;
}

////////////////////////////////////////////////////////////////////////////
//
// (c.haag 2004-10-04 10:39) - The code content was copied and modified from
// EMRDlg.cpp
//
////////////////////////////////////////////////////////////////////////////
extern CString GenerateXMLFromSemiColonDelimitedIDList(const CString &strSemiColonDelimitedIDList);

class CEMN_Header_Duplicate_Local
{
public:
	// (c.haag 2007-01-09 16:59) - PLID 22026 - This is an array of indices to CheckAndWarnOfDuplicates::aDetails
	CDWordArray m_anDetails;
};

BOOL CPicContainerDlg::CheckAndWarnOfDuplicates(CList<MergeField,MergeField&> &listMergeFields, CArray<DetailInfo, DetailInfo>& aDetails)
{
	// (c.haag 2004-10-05 09:14) - Go through the detail array looking for
	// duplicate details among multiple EMN's	
	// (c.haag 2007-01-09 16:24) - PLID 22026 - The goal of CheckAndWarnOfDuplicates is to
	// warn the user if more than one detail has the same merge name so that the user can
	// decide which detail to use in the merge. The strategy here is to go through every detail
	// of every EMN (all contained in aDetails) and populate a map which maps merge field
	// names to details. If multiple details have the same merge name, they will be associated
	// with the same value (A CEMN_Header_Duplicate_Local object). After all is said and done,
	// we will be able to give the map a merge field name, and it will give us a list of all
	// the details with the matching name (The list is CEMN_Header_Duplicate_Local::m_aDetails).
	//
	CMapStringToPtr mapDuplicates;
	for (long i=0; i < aDetails.GetSize(); i++) {
		CString strField = aDetails[i].pDetail->GetMergeFieldName(TRUE);
		CEMN_Header_Duplicate_Local* pHDL = NULL;
		if (!mapDuplicates.Lookup(strField, (LPVOID&)pHDL)) {
			pHDL = new CEMN_Header_Duplicate_Local;
			mapDuplicates[strField] = pHDL;
		}
		pHDL->m_anDetails.Add(i);
	}


	// (c.haag 2004-10-05 09:15) - Now go through our duplicate list and figure out
	// how to deal with them.
	// (c.haag 2007-01-09 16:48) - PLID 22026 - Traverse the map looking for merge field
	// names that are associated with multiple DetailInfo objects. For each map value that
	// has them, prompt the user asking for the proper detail.
	//
	POSITION posDuplicate =	mapDuplicates.GetStartPosition();
	while (NULL != posDuplicate)
	{
		long nNewDetailID = -1;
		long j;

		CString strDupFieldName;
		CEMN_Header_Duplicate_Local* pHDL = NULL;
		mapDuplicates.GetNextAssoc( posDuplicate, strDupFieldName, (void*&)pHDL );

		if (pHDL->m_anDetails.GetSize() > 1)
		{
			// We just don't know which collection the user wants to use in this
			// merge, so lets ask the user.
			CEMRMergeConflictDlg dlg(this);
			dlg.SetFieldName(strDupFieldName);
			// (a.walling 2010-08-16 14:49) - PLID 37580
			bool bIsSignature = false;
			for (j=0; j < pHDL->m_anDetails.GetSize(); j++)
			{
				DetailInfo& info = aDetails[pHDL->m_anDetails[j]];
				if (info.pDetail->IsSignatureDetail()) {
					bIsSignature = true;
				}
				dlg.AddDetail(info.pDetail->m_pParentTopic->GetParentEMN()->GetDescription(),
					info.m_strFormatted,
					info.pDetail->m_nEMRDetailID);
			}
			if (IDCANCEL == dlg.DoModal())
			{
				posDuplicate = mapDuplicates.GetStartPosition();
				while (NULL != posDuplicate) {
					mapDuplicates.GetNextAssoc( posDuplicate, strDupFieldName, (void*&)pHDL );
					delete pHDL;
				}
				return FALSE; // The user cancelled, so lets fail out.
			}
			nNewDetailID = dlg.GetSelectedDetailID();



			// (c.haag 2004-10-05 13:02) - Now we figure out which EMN to use given
			// our new collection ID
			long nChosenDetailIndex = -1;
			for (j=0; j < pHDL->m_anDetails.GetSize(); j++)
			{
				DetailInfo& info = aDetails[pHDL->m_anDetails[j]];
				if (info.pDetail->m_nEMRDetailID == nNewDetailID &&
					nChosenDetailIndex == -1) 
				{
					nChosenDetailIndex = j;
				}
				// (a.walling 2010-08-16 14:49) - PLID 37580
				else if (!bIsSignature) {
					info.m_bMarkedForDeletion = true;
				}
			}
			// Additional failsafe
			if (nChosenDetailIndex == -1) {
				MessageBox("Practice could not determine which collection to merge the document with. The merge will now terminate.");
				posDuplicate = mapDuplicates.GetStartPosition();
				while (NULL != posDuplicate) {
					mapDuplicates.GetNextAssoc( posDuplicate, strDupFieldName, (void*&)pHDL );
					delete pHDL;
				}
				return FALSE;
			}
			// Now alter the data in the merge field list
			POSITION pos = listMergeFields.GetHeadPosition();
			DetailInfo& choseninfo = aDetails[ pHDL->m_anDetails[nChosenDetailIndex] ];
			for (long k=0; k < listMergeFields.GetCount(); k++)
			{
				if (listMergeFields.GetAt(pos).strHeader == choseninfo.pDetail->GetMergeFieldName(TRUE))
				{
					// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
					listMergeFields.GetAt(pos).strData = "\"" + ConvertToQuotableWordCSVString(choseninfo.m_strFormatted) + "\"";
					break;
				}
				listMergeFields.GetNext(pos);
			}
		}  // if (pHDL->m_anDetails.GetSize() > 1)
	} // while (NULL != posDuplicate)

	// Now remove the non-duplicates from our detail array
	for (i=0; i < aDetails.GetSize(); i++)
	{
		if (aDetails[i].m_bMarkedForDeletion) {
			aDetails.RemoveAt(i);
			i--;
		}
	}

	//
	// (c.haag 2007-01-09 17:03) - PLID 22026 - Cleanup
	//
	posDuplicate = mapDuplicates.GetStartPosition();
	while (NULL != posDuplicate) {
		CString strDupFieldName;
		CEMN_Header_Duplicate_Local* pHDL = NULL;
		mapDuplicates.GetNextAssoc( posDuplicate, strDupFieldName, (void*&)pHDL );
		delete pHDL;
	}
	return TRUE;
}

// (a.walling 2010-08-16 14:45) - PLID 37580 - Need to maintain signatures
BOOL CPicContainerDlg::GenerateDetailMergeData(const CString& strEMNIDFilter, CMergeEngine &mi,
											   CArray<DetailInfo, DetailInfo>& aDetails,
											   POSITION& pLastNonAlphabetized,
											   CList<MergeField,MergeField&> &listMergeFields,
											   BOOL bEditingTemplate,
											   OUT CArray<CEMN*,CEMN*> &arEMNs)
{
	pLastNonAlphabetized = listMergeFields.GetTailPosition();
	_RecordsetPtr prs = CreateRecordset(
		"SELECT ID FROM EmrMasterT WHERE ID IN (%s)",
		strEMNIDFilter);

	// (c.haag 2004-10-01 17:39) - Build an array of details from all EMN's in our EMR
	FieldsPtr f = prs->Fields;
	CArray<long,long> arEmnIDs;
	while (!prs->eof)
	{
		arEmnIDs.Add(AdoFldLong(f,"ID"));
		prs->MoveNext();
	}
	prs->Close();
	for(int i = 0; i < arEmnIDs.GetSize(); i++) {
		//
		// (c.haag 2006-06-23 11:48) - PLID 21191 - We used to always allocate new EMN
		// objects. This is a waste of resources because we already have those objects
		// in memory. So, look to memory for the object before allocating a new one.
		//
		// So that this optimization will not change behavior, we also check if the EMN
		// was modified. If it was, we have to pull from data, because the old model always
		// pulled from data and not memory.
		//
		CEMN *pEMN = NULL;
		if(GetEmrEditor()) {
			for (int j=0; j < GetEmrEditor()->GetEMNCount() && NULL == pEMN; j++) {
				// If this EMN in memory has a matching ID, and has not been modified,
				// use that
				if (GetEmrEditor()->GetEMN(j)->GetID() == arEmnIDs[i] &&
					!GetEmrEditor()->GetEMN(j)->IsUnsaved()) {
					pEMN = GetEmrEditor()->GetEMN(j);
					pEMN->AddRef(); // Increment the reference count so the object is
									// not deleted in later merge code
				}
			}
		}
		if (NULL == pEMN) {
			// If we could not find an unmodified EMN in memory, then create one
			// for temporary use.
			pEMN = new CEMN(NULL);
			pEMN->LoadFromEmnID(arEmnIDs[i]);
		}
		arEMNs.Add(pEMN);

		long nTotalDetailCount = pEMN->GetTotalDetailCount();
		for(int j = 0; j < nTotalDetailCount; j++) {
			CEMNDetail *p = pEMN->GetDetail(j);
			CString strMergeFieldName = p->GetMergeFieldName(TRUE);
			DetailInfo di;
			di.pDetail = p;
			di.m_bMarkedForDeletion = FALSE;

			CString strFormatted;
			MergeField mf;
			mf.strHeader = p->GetMergeFieldName(TRUE);

			// (c.haag 2004-10-07 09:55) - If we are editing a template, then there is no
			// reason for us to waste processor cycles in getting the sentence
			if (bEditingTemplate) {
				di.m_strFormatted = "";
				mf.strData = "\"\"";
			} else {
				di.m_strFormatted = GetSentence(p, &mi, true, false, m_saTempFiles, ecfParagraph);
				// (j.kuziel 2011-12-08 11:04) - PLID 45613 - Changed to ConvertToQuotableWordCSVString. Prepares string for a Word merge data file.
				mf.strData = "\"" + ConvertToQuotableWordCSVString(di.m_strFormatted) + "\"";
			}
			aDetails.Add(di);

			//Find the right place to put this field.
			// (c.haag 2007-01-09 17:03) - PLID 22026 - Duplicate fields were being added to the
			// merge field list. I changed the traversal to be easier to maintain and to not cause
			// that problem any longer.
			//TES 5/14/2007 - PLID 25986 - The code here was all out of wack. For one thing, it was doing a case-sensitive
			// comparison where other places (notably CEMN::GenerateRelatedMergeData() do a case-insensitive one, which
			// was causing problems, but even beyond that it just didn't end up sorting properly.  I fixed it up.
			POSITION pos = pLastNonAlphabetized;
			POSITION posResult = NULL;
			BOOL bDuplicate = FALSE;
			//TES 5/14/2007 - PLID 25986 - As soon as we've set posResult we need to break out of the loop, because it 
			// means we've found an item that is higher than the one we're inserting.
			while (pos && !posResult && !bDuplicate)
			{
				//TES 5/14/2007 - PLID 25986 - Once we get next, pos will be the position after the one we're comparing.
				// But if the one we're comparing is higher than what we're inserting, then we need to insert before
				// this item, not the one after it.  So, remember the current position before getting next, so we can
				// insert before it if necessary.
				POSITION posPrev = pos;
				CString strHeader = listMergeFields.GetNext(pos).strHeader;
				if (NULL == posResult && strHeader.CompareNoCase(strMergeFieldName) > 0) {
					posResult = posPrev;
				} else if (strHeader == strMergeFieldName) {
					bDuplicate = TRUE;
				}
			}
			if (bDuplicate) {
				//Duplicate!				
				continue;
			}
			if(!posResult)
				listMergeFields.AddTail(mf);
			else
				listMergeFields.InsertBefore(posResult, mf);

		}
	}

	// (c.haag 2004-10-04 17:39) - PLID 14242 - Deal with duplicate details
	// in the detail array.
	if (!bEditingTemplate) {
		if (!CheckAndWarnOfDuplicates(listMergeFields, aDetails))
		{
			return FALSE; // We failed to resolve duplicate merge fields
		}
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
//
// (c.haag 2004-10-04 10:39) - The code content was copied and modified from
// EMRDlg.cpp
//
////////////////////////////////////////////////////////////////////////////
extern CString EmrCalcBuiltInInfoFilter();
BOOL CPicContainerDlg::GenerateRelatedMergeData(const CArray<CEMN*,CEMN*>& arEMNs, CList<MergeField,MergeField&> &listMergeFields, POSITION posLastNonAlphabetized)
{
	//TES 3/21/2006 - Just use the CEMN function.
	for(int i = 0; i < arEMNs.GetSize(); i++) {
		arEMNs[i]->GenerateRelatedMergeData(listMergeFields, posLastNonAlphabetized);
	}
	return TRUE;
}

BOOL CPicContainerDlg::GenerateMergeData(CMergeEngine &mi, OUT CString &strHeaders, OUT CString &strData,
										 BOOL bEditingTemplate)
{
	CString strHeadersLoc, strDataLoc;
	CList<MergeField, MergeField&> listFields;
	CArray<DetailInfo, DetailInfo> aDetailInfo;
	POSITION pLastNonAlphabetized;
	CString strEMNIDFilter;

	// (c.haag 2004-10-18 13:30) - PLID 14242 - Build a string of EMN ID's so
	// we can filter out certain EMN's if the user wants to
	for (int i=0; i < mi.m_arydwEMRIDs.GetSize(); i++)
	{
		CString str;
		str.Format("%d,", mi.m_arydwEMRIDs[i]);
		strEMNIDFilter += str;
	}
	if (strEMNIDFilter.GetLength()) {
		strEMNIDFilter = strEMNIDFilter.Left( strEMNIDFilter.GetLength() - 1 ); // Trim the right comma
	} else {
		strEMNIDFilter = "-1";
	}

	//TES 9/1/2004: We want to enforce the order: Common Fields, then Category fields, then all other fields alphabetically.

	// Add fields that will be in any and every EMR merge
	if (!bEditingTemplate) {
		if (!GenerateCommonMergeData(strEMNIDFilter, listFields)) return FALSE;
	}
	// (c.haag 2004-06-30 12:46) - PLID 14242 - Merge categories
	if (!GenerateCategoryMergeData(listFields, bEditingTemplate)) return FALSE;
	// (c.haag 2004-10-01 17:34) - PLID 14242 - Merge details
	CArray<CEMN*,CEMN*> arEMNs;
	if (!GenerateDetailMergeData(strEMNIDFilter, mi, aDetailInfo, pLastNonAlphabetized, listFields, bEditingTemplate, arEMNs)) return FALSE;
	//Copy just the details out of there.
	CArray<CEMNDetail*,CEMNDetail*> arDetails;
	for(i = 0; i < aDetailInfo.GetSize(); i++)
		arDetails.Add(aDetailInfo[i].pDetail);
	// (c.haag 2004-07-06 09:09) - PLID 14242 - Now we include all the fields that don't exist for this
	// chart, but do exist for the procedures of this chart, as empty fields. We actually
	// look to data to get these values! <gasp>
	if (!GenerateRelatedMergeData(arEMNs, listFields, pLastNonAlphabetized)) {
		//Clean up the CEMN pointers.
		// (c.haag 2006-06-23 11:39) - PLID 21191 - In the past, we used to always allocate
		// new EMN objects. Now, the EMN objects may have been allocated by us, or by the
		// EMR editor window. To be safe, we now use reference counting. The EMR editor window
		// would own one reference, and the merge would own one reference. Now that we are done
		// with this object, we release our reference to it. Because the EMR editor window still
		// has a reference, the object will not be deleted. On the other hand, if we allocated
		// this EMN object, then Release() will not only decrement the reference count, but it
		// will delete the object.
		for(i = 0; i < arEMNs.GetSize(); i++) arEMNs[i]->Release();
		return FALSE;
	}
	// (c.haag 2004-05-10 10:09) - Prepare the temporary merge files
	// for merging whole sets of categories. (Note: The destructor
	// will delete them)
	// (c.haag 2004-10-07 09:59) - ....unless we are merely editing a template
	if (!bEditingTemplate) {
		GenerateTempMergeFiles(arDetails, mi);
	}

	POSITION pos = listFields.GetHeadPosition();
	for(i=0; i < listFields.GetCount(); i++) {
		MergeField mf = listFields.GetNext(pos);
		strHeadersLoc += mf.strHeader + ",";
		strDataLoc += mf.strData + ",";
	}
	strHeaders = strHeadersLoc.Left(strHeadersLoc.GetLength()-1);
	strData = strDataLoc.Left(strDataLoc.GetLength()-1);

	//Clean up.
	// (c.haag 2006-06-23 11:39) - PLID 21191 - In the past, we used to always allocate
	// new EMN objects. Now, the EMN objects may have been allocated by us, or by the
	// EMR editor window. To be safe, we now use reference counting. The EMR editor window
	// would own one reference, and the merge would own one reference. Now that we are done
	// with this object, we release our reference to it. Because the EMR editor window still
	// has a reference, the object will not be deleted. On the other hand, if we allocated
	// this EMN object, then Release() will not only decrement the reference count, but it
	// will delete the object.
	for(i = 0; i < arEMNs.GetSize(); i++) arEMNs[i]->Release();

	return TRUE;
}

// (c.haag 2004-04-30 13:14) - Generate a single HTML file for every
// individual EMR category so they can be merged into a Word template.
void CPicContainerDlg::GenerateTempMergeFiles(CArray<CEMNDetail*, CEMNDetail*>& aDetails, CMergeEngine &mi)
{
	try {
		//TES 6/7/2004: Generate our list of info items from what's on screen.
		CString strItemFilter;
		for(int i = 0; i < aDetails.GetSize(); i++) {
			CString strPart;
			strPart.Format("%li, ", aDetails.GetAt(i)->m_nEMRInfoID);
			strItemFilter += strPart;
		}
		if(strItemFilter.IsEmpty()) return;
		//Trim last ", "
		strItemFilter = strItemFilter.Left(strItemFilter.GetLength()-2);
		_RecordsetPtr rsCategories = CreateRecordset("SELECT EmrCategoriesT.ID, EmrCategoriesT.Name AS CatName, EmrCategoriesT.Format "
			"FROM EmrCategoriesT WHERE ID IN (SELECT EMRCategoryID FROM EMRInfoCategoryT WHERE EmrInfoID IN (%s)) "
			"ORDER BY EmrCategoriesT.Name ASC", strItemFilter);

		//
		// (c.haag 2006-06-22 10:16) - PLID 21169 - GenerateTempMergeFile will call GetParagraph, which requires many
		// lookups to EmrInfoCategoryT. In order to optimize GenerateTempMergeFiles as a whole, we need to basically
		// load the relevant records of EmrInfoCategoryT into memory. What we have is a map object which maps EmrInfoID's
		// to arrays of category ID's. Basically, it's:
		//
		// mapEmrInfoCat[ nEmrInfoID ] = CArray< category ids >
		//
		CMap<long,long, CArray<long,long>*, CArray<long,long>*> mapEmrInfoCat;
		_RecordsetPtr rsEmrInfoCat = CreateRecordset("SELECT EmrInfoID, EmrCategoryID FROM EmrInfoCategoryT WHERE EmrInfoID IN (%s) ORDER BY EmrInfoID, EmrCategoryID",
			strItemFilter);
		while (!rsEmrInfoCat->eof) {
			long nEmrInfoID = AdoFldLong(rsEmrInfoCat, "EmrInfoID");
			long nEmrCategoryID = AdoFldLong(rsEmrInfoCat, "EmrCategoryID");
			CArray<long,long>* pary = NULL;
			if (!mapEmrInfoCat.Lookup(nEmrInfoID, pary)) {
				mapEmrInfoCat[nEmrInfoID] = pary = new CArray<long,long>;
			}
			pary->Add(nEmrCategoryID);
			rsEmrInfoCat->MoveNext();
		}
		rsEmrInfoCat->Close();

		while (!rsCategories->eof) {
			GenerateTempMergeFile(aDetails, AdoFldLong(rsCategories, "ID"), (EmrCategoryFormat)AdoFldLong(rsCategories, "Format"),
				AdoFldString(rsCategories, "CatName"), mi, mapEmrInfoCat);
			rsCategories->MoveNext();
		}

		// (a.walling 2010-08-16 11:57) - PLID 37580 - Built-in Signatures category
		GenerateTempMergeFile(aDetails, -26, ecfList, "Signatures", mi, mapEmrInfoCat);

		//
		// (c.haag 2006-06-22 10:18) - PLID 21169 - Clean up the map
		//
		POSITION pos = mapEmrInfoCat.GetStartPosition();
		CArray<long,long>* pValue;
		long nKey;
		while (pos) {
		  mapEmrInfoCat.GetNextAssoc(pos, nKey, pValue);
		  if (pValue) delete pValue;
		}
	}
	NxCatchAll("Error generating temporary merge chart notes");
}

// (j.jones 2007-01-02 11:13) - PLID 24051 - I removed the local definition of EMR_ConvertToHeaderName since it was
//identical code to EMRUtils' ConvertToHeaderName

void CPicContainerDlg::GenerateTempMergeFile(CArray<CEMNDetail*, CEMNDetail*>& aDetails, long nCategoryID, EmrCategoryFormat fmt, const CString& strCatName, CMergeEngine &mi, CMap<long,long, CArray<long,long>*, CArray<long,long>*>& mapEmrInfoCat)
{
	CString strFilename;
	CString strHTML;
	/*
	CString strNewName = strCatName;

	// (c.haag 2004-05-27 08:56) PLID 12614 - Treat field names consistently
	// with how we do it in EMR (See ConvertToHeaderName() in EMRDlg.cpp)
	// First replace every non-alphanumeric character with an underscore
	for (long i=0; i<strNewName.GetLength(); i++) {
		if (!isalnum(strNewName.GetAt(i))) {
			strNewName.SetAt(i, '_');
		}
	}
	// Then make every sequence of more than one underscore into a single underscore
	while (strNewName.Replace("__", "_"));
	*/

	// (j.jones 2007-01-02 10:53) - PLID 24051 - corrected truncation problems by determing exactly
	// what the merge field was truncated to (if at all)
	CString strNewName = ConvertToHeaderName("EMR_Category", strCatName);
	//remove the EMR_Category prefix
	strNewName = strNewName.Right(strNewName.GetLength() - 13);

	// Generate our temporary file
	strFilename.Format("MergeHTML_Cat_EMR_%s_%d.htm", strNewName, m_nEmrGroupID);
	CString strFullPath = GetNxTempPath() ^ strFilename;
	CStdioFile f(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareCompat);

	// Write the content to the file
	strHTML = "<html><head></head><body>" + GetParagraph(aDetails, nCategoryID, fmt, mi, mapEmrInfoCat) + "</body></html>";
	f.WriteString(strHTML);
	f.Close();

	// Add to our temp file list
	m_saTempFiles.Add(strFullPath);
}

void CPicContainerDlg::RemoveTempMergeFiles()
{
	for (long i=0; i < m_saTempFiles.GetSize(); i++)
	{
		DeleteFileWhenPossible(m_saTempFiles[i]);
	}
	m_saTempFiles.RemoveAll();
}

LRESULT CPicContainerDlg::OnClosePic(WPARAM wParam, LPARAM lParam)
{
	//We've been told to close.  Who are we to argue?
	SaveAndClose();

	//Now, did we actually close?
	if(!GetSafeHwnd() || !::IsWindow(GetSafeHwnd()) || !IsWindowVisible()) {
		//Now, see what else we have been asked to do.
		switch((ClosePicReason)wParam) {
		case cprScheduler:
			//Flip to the scheduler.
			// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
			g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MonthTab);
			break;
		
		case cprEmnTemplate:
			//Flip to the EMR tab.
			GetMainFrame()->EditEmnTemplate((long)lParam);
			break;

		default:
		case cprNone:
			//Do nothing.
			break;
		}
	}
	return 0;
}

HBRUSH CPicContainerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CEmrPatientFrameWnd::OnCtlColor(pDC, pWnd, nCtlColor);
	switch (nCtlColor)
	{	case CTLCOLOR_STATIC:
		//make anything (STATIC text) with WS_EX_TRANSPARENT
		//appear transparent without subclassing it
			if (pWnd->GetExStyle() & WS_EX_TRANSPARENT)
			{	pDC->SetBkMode (TRANSPARENT);
				return (HBRUSH)GetStockObject(NULL_BRUSH);
			}
			break;
		case CTLCOLOR_BTN:
		case CTLCOLOR_DLG:
		case CTLCOLOR_MSGBOX:
		case CTLCOLOR_EDIT:
		default:
			break;
	}
	return hbr;
}

long CPicContainerDlg::GetCurrentProcInfoID()
{
	return m_nProcInfoID;
}

long CPicContainerDlg::GetCurrentEMRGroupID()
{
	return m_nEmrGroupID;
}

LRESULT CPicContainerDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
	try {
		// (c.haag 2007-07-05 10:25) - PLID 26555 - Ensure we have a valid window handle; having
		// a valid pointer alone isn't enough
		if(GetEmrEditor() && IsWindow(GetEmrEditor()->GetSafeHwnd()))
			GetEmrEditor()->SendMessage(WM_TABLE_CHANGED, wParam, lParam);

		// (c.haag 2010-07-29 17:30) - PLID 38928 - Forward table checkers to the 
		// medication / allergy viewer. It has to update real-time.
		if (m_pMedAllergyViewerDlg && IsWindow(m_pMedAllergyViewerDlg->GetSafeHwnd())) {
			m_pMedAllergyViewerDlg->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}

		// (c.haag 2007-07-05 10:25) - PLID 26555 - Ensure we have a valid window handle; having
		// a valid pointer alone isn't enough
		// (c.haag 2010-07-20 17:29) - PLID 30894 - Propogate it to the labs tab
		if(GetLabsDlg() && IsWindow(GetLabsDlg()->GetSafeHwnd()))
			GetLabsDlg()->SendMessage(WM_TABLE_CHANGED, wParam, lParam);

		// (j.dinatale 2012-07-02 13:36) - PLID 51282 - refresh our photopane if we get the table checker
		if (wParam == NetUtils::MailSent) {

			// (j.jones 2014-08-04 14:56) - PLID 63181 - MailSent should only be sent in EX form,
			// if a regular version is sent we will not respond to it
		}

		// (j.jones 2014-08-04 16:18) - PLID 63181 - send this to our history tab
		if (GetHistoryDlg()) {
			GetHistoryDlg()->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}
				
	} NxCatchAll("Error in CPicContainerDlg::OnTableChanged");

	return 0;
}

// (j.jones 2014-08-04 14:59) - PLID 63181 - added Ex handling
LRESULT CPicContainerDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {

		case NetUtils::MailSent:

			try {

				// (j.jones 2014-08-04 15:00) - PLID 63181 - the Ex message now gives us the MailSentID,
				// the PatientID, and IsNonPhoto. IsNonPhoto is only 1 when we know it is not a photo.
				// It would be 0 for files where the photo status is unknown, or definite photos.
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPatientID), -1);

				//if not for this patient, leave now
				if (nPatientID != GetPatientID()) {
					return 0;
				}

				// (j.jones 2014-08-04 16:14) - PLID 63181 - do not try to refresh the photo pane if
				// we know this change is not for a photo
				TableCheckerDetailIndex::MailSent_PhotoStatus ePhotoStatus = (TableCheckerDetailIndex::MailSent_PhotoStatus)VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPhotoStatus), (long)TableCheckerDetailIndex::MailSent_PhotoStatus::mspsUnknown);
				if (ePhotoStatus != TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto) {
					//this function will only refresh immediately if the photos pane is visible,
					//otherwise it will not refresh until the user switches to the photos pane
					RefreshPhotoPane();
				}

			} NxCatchAll("Error in CPicContainerDlg::OnTableChangedEx:MailSent");
		}

		// (r.gonet 09/02/2014) - PLID 63221 - Forward ex table checkers to the labs dialog.
		if (GetLabsDlg() && IsWindow(GetLabsDlg()->GetSafeHwnd()))
			GetLabsDlg()->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);

		// (j.jones 2014-08-04 16:18) - PLID 63181 - send this to our history tab
		if (GetHistoryDlg()) {
			GetHistoryDlg()->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}

		//TES 8/14/2014 - PLID 63519 - Pass on to the EMR editor
		if (GetEmrEditor() && IsWindow(GetEmrEditor()->GetSafeHwnd())) {
			GetEmrEditor()->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}

	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (c.haag 2004-12-16 16:09) - PLID 14987 - We need to be able to save the EMR
// from the history tab.
BOOL CPicContainerDlg::Save()
{
	// (c.haag 2004-10-08 15:56) - PLID 14347 - Save the EMR
	if(GetEmrEditor()) {		
		// (a.walling 2008-06-26 16:22) - PLID 30513 - Use FAILED macro to check for success
		if(FAILED(GetEmrEditor()->Save(TRUE)))
			return FALSE;
	}
	return TRUE;
}

void CPicContainerDlg::EnsureIDs()
{
	//Load ProcInfo

	using namespace NXDATALISTLib;

	m_cbPatientGender = 0;
	m_strPatientAge = "";
	m_strPrimaryInsCo = "";

	// (c.haag 2006-04-12 10:04) - PLID 20040 - Load the IsCommitted flag too
	if(m_nPicID == -1) {
		m_nProcInfoID = -1;
		m_nEmrGroupID = -1;
		SetIsCommitted(FALSE);

		// (j.jones 2010-04-01 14:59) - PLID 34915 - load the patient gender, age, and primary insurance company
		if(m_nPatientID != -1) {
			_RecordsetPtr rsPatInfo = CreateParamRecordset("SELECT PersonT.Gender, PersonT.BirthDate, InsuranceCoT.Name AS InsCoName "
				"FROM PersonT "
				"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 1) AS PrimaryInsuredPartyT ON PersonT.ID = PrimaryInsuredPartyT.PatientID "
				"LEFT JOIN InsuranceCoT ON PrimaryInsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"WHERE PersonT.ID = {INT}", m_nPatientID);
			if(!rsPatInfo->eof) {
				m_cbPatientGender = AdoFldByte(rsPatInfo, "Gender", 0);		
				_variant_t varBirthDate = rsPatInfo->Fields->GetItem("BirthDate")->Value;
				if(varBirthDate.vt == VT_DATE) {
					// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
					//  validation should only be done when bdays are entered/changed
					m_strPatientAge = ::GetPatientAgeOnDate(VarDateTime(varBirthDate), COleDateTime::GetCurrentTime(), TRUE);
				}
				else {
					m_strPatientAge = "";
				}
				m_strPrimaryInsCo = VarString(rsPatInfo->Fields->GetItem("InsCoName")->Value, "");
			}
			rsPatInfo->Close();
		}
	}
	else {
		//DRT 8/28/2007 - PLID 27203 - Parameterized.
		// (j.jones 2010-04-01 14:59) - PLID 34915 - load the patient gender, age, and primary insurance company
		_RecordsetPtr rsProcInfo = CreateParamRecordset("SELECT ProcInfoID, EmrGroupID, IsCommitted, "
			"PersonT.Gender, PersonT.BirthDate, InsuranceCoT.Name AS InsCoName "
			"FROM PicT "
			"LEFT JOIN ProcInfoT ON PicT.ProcInfoID = ProcInfoT.ID "
			"LEFT JOIN EMRGroupsT ON PicT.EmrGroupID = EMRGroupsT.ID "
			"LEFT JOIN PersonT ON EmrGroupsT.PatientID = PersonT.ID OR ProcInfoT.PatientID = PersonT.ID "
			"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 1) AS PrimaryInsuredPartyT ON PersonT.ID = PrimaryInsuredPartyT.PatientID "
			"LEFT JOIN InsuranceCoT ON PrimaryInsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE PicT.ID = {INT}", m_nPicID);
		if(rsProcInfo->eof) {
			m_nProcInfoID = -1;
			m_nEmrGroupID = -1;
			SetIsCommitted(FALSE);
		}
		else {
			m_nProcInfoID = AdoFldLong(rsProcInfo, "ProcInfoID", -1);
			m_nEmrGroupID = AdoFldLong(rsProcInfo, "EmrGroupID", -1);
			SetIsCommitted(AdoFldBool(rsProcInfo, "IsCommitted", FALSE));

			// (j.jones 2010-04-01 14:59) - PLID 34915 - added patient gender, age, and primary insurance company
			m_cbPatientGender = AdoFldByte(rsProcInfo, "Gender", 0);		
			_variant_t varBirthDate = rsProcInfo->Fields->GetItem("BirthDate")->Value;
			if(varBirthDate.vt == VT_DATE) {
				// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
				//  validation should only be done when bdays are entered/changed
				m_strPatientAge = ::GetPatientAgeOnDate(VarDateTime(varBirthDate), COleDateTime::GetCurrentTime(), TRUE);
			}
			else {
				m_strPatientAge = "";
			}
			m_strPrimaryInsCo = VarString(rsProcInfo->Fields->GetItem("InsCoName")->Value, "");
		}
		rsProcInfo->Close();
	}

	//Is our proc info id valid?
	if(m_nProcInfoID == -1) {
		
		if(m_nProcInfoID == -2 || m_nProcInfoID == -1) {
			//OK, we're going to create it.
			
			//Get our list of procedures.
			CArray<int,int> arProcIDs;
			bool bTrackableFound = false;

			if(m_nPicID != -1) {
				// (d.moore 2007-06-11 17:33) - PLID 14670 - Modified query to use the ProcedureLadderTemplateT  table to determine tracking.
				//This is the list of all procedures that are on an EMR for this PIC and are masters, or are the masters of procedures on this EMR.
				//DRT 8/28/2007 - PLID 27203 - Parameterized.
				_RecordsetPtr rsEmrProcs = CreateParamRecordset(
					"SELECT EmrProcedureT.ProcedureID, "
					"CASE WHEN TrackingQ.ProcedureID > 0 THEN 1 ELSE 0 END AS Trackable "
					"FROM (SELECT * FROM EMRProcedureT WHERE Deleted = 0) AS EmrProcedureT "
						"INNER JOIN ProcedureT ON EmrProcedureT.ProcedureID = ProcedureT.ID "
						"LEFT JOIN (SELECT DISTINCT ProcedureID FROM ProcedureLadderTemplateT)  AS TrackingQ "
						"ON EmrProcedureT.ProcedureID = TrackingQ.ProcedureID "
					"WHERE EMRID IN "
						"(SELECT EmrMasterT.ID FROM "
							"(SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EmrMasterT "
							"INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID "
							"WHERE PicT.ID = {INT}) AND ProcedureT.MasterProcedureID Is Null "
					"UNION SELECT MasterProcedureID, 0 AS Trackable "
					"FROM (SELECT * FROM EMRProcedureT WHERE Deleted = 0) AS EmrProcedureT "
						"INNER JOIN ProcedureT ON EmrProcedureT.ProcedureID = ProcedureT.ID "
					"WHERE EmrProcedureT.EMRID IN "
						"(SELECT EmrMasterT.ID FROM "
							"(SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EmrMasterT "
							"INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID "
							"WHERE PicT.ID = {INT}) "
					"AND ProcedureT.MasterProcedureID Is Not Null",
					m_nPicID, m_nPicID);				
				
				while(!rsEmrProcs->eof) {
					if(AdoFldLong(rsEmrProcs, "Trackable") == 1)
						bTrackableFound = true;
					arProcIDs.Add(AdoFldLong(rsEmrProcs, "ProcedureID"));
					rsEmrProcs->MoveNext();
				}
			}

			m_nProcInfoID = PhaseTracking::CreateProcInfo(GetPatientID(), arProcIDs, false);

			if(m_nPicID != -1) {
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				ExecuteParamSql("UPDATE ProcInfoDetailsT SET Chosen = 1 WHERE ProcInfoID = {INT} "
					"AND ProcedureID IN (SELECT ProcedureID FROM EmrProcedureT WHERE Deleted = 0 AND EmrID IN (SELECT EmrMasterT.ID FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EmrMasterT INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID WHERE PicT.ID = {INT})) "
					"AND ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID Is Not Null)",m_nProcInfoID, m_nPicID);
			}

			// (j.jones 2011-07-22 14:25) - PLID 38119 - do not do this unless they have the tracking license
			if(bTrackableFound && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
				//TES 2/26/2009 - PLID 33168 - This is now controlled by a preference
				if(GetRemotePropertyInt("PIC_CreateLadderOnProcedureAdded", 1, 0, "<None>")) {
					if(IDYES == MessageBox("This EMR has procedures associated with it that are not being tracked.  Would you like to begin tracking them now?", NULL, MB_YESNO)) {
						PhaseTracking::AddLadderToProcInfo(m_nProcInfoID, GetPatientID());
					}
				}
			}
		}
	}

	//Is our emr group id valid?
	else if(m_nEmrGroupID == -1) { //If both are -1, we must be creating a new EMR, so don't prompt, just create below.
		//See if there's one we can use.
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		if(ReturnsRecordsParam("SELECT EmrGroupsT.ID FROM EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
			"WHERE PatientID = {INT} AND PicT.ProcInfoID Is Null AND EMRGroupsT.Deleted = 0", GetPatientID())) {
			CSelectDlg dlg(this);
			dlg.m_strTitle = "Select EMR";
			dlg.m_strCaption = "This patient has EMRs which are not already associated with a Tracking record.  Please select one to associate with this Tracking record, or Cancel to create a new EMR.";
			dlg.m_strFromClause = "(SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID";
			dlg.m_strWhereClause.Format("PatientID = %li AND PicT.ProcInfoID Is Null", GetPatientID());
			DatalistColumn dcID, dcPicID, dcName;
			dcID.strField = "EmrGroupsT.ID";
			dcID.strTitle = "ID";
			dcID.nWidth = 0;
			dcID.nStyle = csVisible|csFixedWidth;
			dcID.nSortPriority = -1;
			dcID.bSortAsc = TRUE;
			dlg.m_arColumns.Add(dcID);
			dcPicID.strField = "PicT.ID";
			dcPicID.strTitle = "PicID";
			dcPicID.nWidth = 0;
			dcPicID.nStyle = csVisible|csFixedWidth;
			dcPicID.nSortPriority = -1;
			dcPicID.bSortAsc = TRUE;
			dlg.m_arColumns.Add(dcPicID);
			dcName.strField = "Description";
			dcName.strTitle = "Name";
			dcName.nWidth = -1;
			dcName.nStyle = csVisible|csWidthAuto;
			dcName.nSortPriority = 0;
			dcName.bSortAsc = TRUE;
			dlg.m_arColumns.Add(dcName);
			if(IDOK == dlg.DoModal()) {
				m_nEmrGroupID = VarLong(dlg.m_arSelectedValues.GetAt(0));
				long nNewPicID = VarLong(dlg.m_arSelectedValues.GetAt(1));
				if(m_nPicID != -1) {
					//TES 9/22/2005 - If we're uniting two PICs, we need to delete one.
					//TES 8/31/2009 - PLID 34045 - Move any attached documents and labs
					ExecuteParamSql("UPDATE MailSent SET PicID = {INT} WHERE PicID = {INT} \r\n"
						"UPDATE LabsT SET PicID = {INT} WHERE PicID = {INT} \r\n"
						"DELETE FROM PicT WHERE ID = {INT}", 
						nNewPicID, m_nPicID, nNewPicID, m_nPicID, m_nPicID);
					// (c.haag 2010-07-20 17:34) - PLID 30894 - We need to send a labs table refresh
					// to any related open PIC's that exist so their labs tabs are still accurate.
					// (r.gonet 08/25/2014) - PLID 63221 - Supply the patient ID
					CClient::RefreshLabsTable(GetPatientID(), -1);
				}
				m_nPicID = nNewPicID;
			}
		}
	}

	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	_variant_t varProcInfoID = g_cvarNull;
	if(m_nProcInfoID != -1) varProcInfoID = m_nProcInfoID;
	_variant_t varEmrGroupID = g_cvarNull;
	if(m_nEmrGroupID != -1) varEmrGroupID = m_nEmrGroupID;

	if(m_nPicID == -1) {
		// (b.cardillo 2005-10-12 13:42) - PLID 17178 - Default IsCommitted to false here 
		// because this is where the placeholder PicT record is being created to begin with; 
		// it will be set to committed as soon as appropriate (when the user dismisses the PIC 
		// window, at the very latest).
		//DRT 7/16/2007 - PLID 26693 - Combine these into 1 single statement, saves us a trip to the database
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr rsNewNum = CreateParamRecordset("SET NOCOUNT ON;\r\n"
			"INSERT INTO PicT (ProcInfoID, EmrGroupID, IsCommitted) VALUES ({VT_I4}, {VT_I4}, 0);\r\n"
			"SET NOCOUNT OFF;\r\n"
			"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS NewID;\r\n", varProcInfoID, varEmrGroupID);
		m_nPicID = AdoFldLong(rsNewNum, "NewID");
	}
	else {
		// (a.walling 2007-10-16 17:12) - PLID 27786 - Verify the EmrGroupID is not in use and is safe to overwrite
		VerifyEmrGroupID(m_nPicID, m_nEmrGroupID);
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		//(e.lally 2011-06-30) PLID 44394 - This can hold an exclusive lock while the same user is querying the PicT table in a datatlist on SQL 2000 (no snapshot isolation) causing a consistent deadlock
		//	So I changed the where clause to be more selective when doing the update, namely when something is actually changing in value.
		// (b.cardillo 2011-09-07 12:36) - PLID 45372 - Fixed to account for NULL
		ExecuteParamSql("UPDATE PicT SET ProcInfoID = {VT_I4}, EmrGroupID = {VT_I4} WHERE ID = {INT} AND (\r\n"
			" ProcInfoID IS NULL AND {VT_I4} IS NOT NULL OR \r\n"
			" ProcInfoID IS NOT NULL AND {VT_I4} IS NULL OR \r\n"
			" ProcInfoID <> {VT_I4} OR \r\n"
			" EmrGroupID IS NULL AND {VT_I4} IS NOT NULL OR \r\n"
			" EmrGroupID IS NOT NULL AND {VT_I4} IS NULL OR \r\n"
			" EmrGroupID <> {VT_I4} \r\n"
			" ) ", varProcInfoID, varEmrGroupID, m_nPicID, varProcInfoID, varProcInfoID, varProcInfoID, varEmrGroupID, varEmrGroupID, varEmrGroupID);
	}
}

long CPicContainerDlg::GetCurrentPicID()
{
	return m_nPicID;
}

int CPicContainerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEmrPatientFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	try {
		Initialize();
	} NxCatchAll("CPicContainerDlg::OnCreate");
	
	return 0;
}

void CPicContainerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CEmrPatientFrameWnd::OnSize(nType, cx, cy);
}

LRESULT CPicContainerDlg::OnSaveCreateEMNBill(WPARAM wParam, LPARAM lParam)
{
	try {
		SaveAndClose(cpaCreateBill, (CEMN*)wParam, (long)lParam);
	} NxCatchAll(__FUNCTION__);
	return 0;
}

LRESULT CPicContainerDlg::OnSaveCreateEMNQuote(WPARAM wParam, LPARAM lParam)
{
	try {
		SaveAndClose(cpaCreateQuote, (CEMN*)wParam);
	} NxCatchAll(__FUNCTION__);
	return 0;
}

LRESULT CPicContainerDlg::OnSaveWritePrescriptions(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 6/17/2008 - PLID 30411 - lParam is now meaningful.
		SaveAndClose(cpaWritePrescriptions, (CEMN*)wParam, (long)lParam);
	} NxCatchAll(__FUNCTION__);
	return 0;
}

LRESULT CPicContainerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
		// (d.lange 2010-10-26 17:08) - PLID 41088 - Need to pass this message along to EmrEditorDlg
		case NXM_DEVICE_IMPORT_STATUS: {
			if (GetEmrEditor()) {
				GetEmrEditor()->SendMessage(message, wParam, lParam);
			}

			if (GetHistoryDlg()) {
				GetHistoryDlg()->SendMessage(message, wParam, lParam);
			}
			break;
		}

		case NXM_PRE_DELETE_EMN_PROCEDURE: {
			try {
				CArray<long,long> arProcIDs;
				arProcIDs.Add((long)wParam);
				//TES 5/20/2008 - PLID 27905 - We need to pass in the EMN that these procedures are being deleted from as well.
				HandleDeletingEmrProcedures(arProcIDs, (CEMN*)lParam);
			} NxCatchAll("Error in WindowProc : PreDeleteEMNProcedure");
			break;
		}

		case NXM_EMN_PROCEDURE_ADDED: {
			try {
				EMNProcedure* pProc = (EMNProcedure*)wParam;
				CArray<long,long> arProcIDs;
				arProcIDs.Add(pProc->nID);
				HandleNewEmrProcedures(arProcIDs);
			} NxCatchAll("Error in WindowProc : EMNProcedureAdded");
			break;
		}

		case NXM_EMN_MULTI_PROCEDURES_ADDED:
		{
			try {
				//DRT 9/6/2007 - PLID 27310 - Allow multiple procedures to be added at once.  This dialog
				//	only wants the IDs.
				CArray<long,long> arProcIDs;
				CArray<EMNProcedure*,EMNProcedure*> *aryProcs = (CArray<EMNProcedure*,EMNProcedure*>*)wParam;
				//Loop through all our parameter's procedures and pull the IDs into the long array
				for(int i = 0; i < aryProcs->GetSize(); i++) {
					EMNProcedure *pProc = aryProcs->GetAt(i);
					arProcIDs.Add(pProc->nID);
				}

				//Now pass the whole batch for handling
				HandleNewEmrProcedures(arProcIDs);

			} NxCatchAll("Error in WindowProc : EMNMultiProceduresAdded");
		}
		break;
	}

	return CEmrPatientFrameWnd::WindowProc(message, wParam, lParam);
}

CString CPicContainerDlg::GenerateTitleBarText()
{
	CString strWindowText = GetPatientName();

	CString strProcedures;

	if(!m_arPICProcedureIDs.IsEmpty()) {
		/*
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID IN ({INTARRAY}) ORDER BY Name", m_arPICProcedureIDs);
		while(!rs->eof) {
			strProcedures += AdoFldString(rs, "Name","");
			strProcedures += ", ";
			rs->MoveNext();
		}
		rs->Close();
		*/
		// (a.walling 2012-06-07 09:36) - PLID 50918 - Procedures
		using namespace PracData;
		// (a.walling 2014-04-24 12:00) - VS2013 - use ->get<> instead of by
		for (const Procedure& procedure : Cached<ProcedureT>()->get<Name>()) {
			if (boost::exists(m_arPICProcedureIDs, procedure.id)) {
				strProcedures += procedure.name;
				strProcedures += ", ";
			}
		}
		//remove the last comma
		/*if(!strProcedures.IsEmpty())
			strProcedures = strProcedures.Left(strProcedures.GetLength()-2);*/
		strProcedures.TrimRight(", ");
	}

	strProcedures.TrimLeft();
	strProcedures.TrimRight();

	// this just looks unprofessional
	//if(strProcedures.IsEmpty())
	//	strProcedures = "<No Procedure Selected>";

	// (j.jones 2010-04-01 14:59) - PLID 34915 - added patient gender, age, and primary insurance company
	CString strExtraCaptionInfo;
	if(m_cbPatientGender == 1) {
		strExtraCaptionInfo.Append("Male");
	}
	else if(m_cbPatientGender == 2) {
		strExtraCaptionInfo.Append("Female");
	}
	
	if(!m_strPatientAge.IsEmpty()) {
		if(!strExtraCaptionInfo.IsEmpty()) {
			strExtraCaptionInfo.Append(", ");
		}
		strExtraCaptionInfo.Append("Age: ");
		strExtraCaptionInfo.Append(m_strPatientAge);
	}

	if(!m_strPrimaryInsCo.IsEmpty()) {
		if(!strExtraCaptionInfo.IsEmpty()) {
			strExtraCaptionInfo.Append(", ");
		}
		strExtraCaptionInfo.Append("Primary Insurance: ");
		strExtraCaptionInfo.Append(m_strPrimaryInsCo);
	}

	if(!strExtraCaptionInfo.IsEmpty()) {
		strWindowText.AppendFormat(" (%s)", strExtraCaptionInfo);
	}

	if (!strProcedures.IsEmpty()) {
		strWindowText.AppendFormat(" - %s", strProcedures);
	}

	return strWindowText;
}

void CPicContainerDlg::GenerateProcedureList()
{
	if(m_nProcInfoID != -1) {
		//which procedures are already on the ProcInfo?
		//DRT 8/28/2007 - PLID 27203 - Parameterized.
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProcedureID FROM ProcInfoDetailsT "
			"WHERE ProcInfoID = {INT} AND ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID Is Null)", m_nProcInfoID);
		while(!rs->eof) {
			long nProcedureID = AdoFldLong(rs, "ProcedureID");
			BOOL bFound = FALSE;
			for(int i=0; i<m_arPICProcedureIDs.GetSize() && !bFound; i++) {
				if(m_arPICProcedureIDs.GetAt(i) == nProcedureID)
					bFound = TRUE;
			}
			if(!bFound)
				m_arPICProcedureIDs.Add(nProcedureID);
			rs->MoveNext();
		}
		rs->Close();
	}
	if(m_nEmrGroupID != -1) {
		//which procedures are already on the EMR?
		//DRT 8/28/2007 - PLID 27203 - Parameterized.
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProcedureID FROM EMRProcedureT "
			"INNER JOIN EMRMasterT ON EMRProcedureT.EMRID = EMRMasterT.ID "
			"WHERE EMRProcedureT.Deleted = 0 AND EMRMasterT.Deleted = 0 AND EMRGroupID = {INT}", m_nEmrGroupID);
		while(!rs->eof) {
			long nProcedureID = AdoFldLong(rs, "ProcedureID");
			BOOL bFound = FALSE;
			for(int i=0; i<m_arPICProcedureIDs.GetSize() && !bFound; i++) {
				if(m_arPICProcedureIDs.GetAt(i) == nProcedureID)
					bFound = TRUE;
			}
			if(!bFound)
				m_arPICProcedureIDs.Add(nProcedureID);
			rs->MoveNext();
		}
		rs->Close();
	}
}

CString CPicContainerDlg::GetDelimitedProcedureList()
{
	CString strProcedureIDs;

	for(int i=0; i<m_arPICProcedureIDs.GetSize(); i++) {
		strProcedureIDs += AsString(m_arPICProcedureIDs.GetAt(i));
		strProcedureIDs += ", ";
	}
	//remove the last comma
	strProcedureIDs = strProcedureIDs.Left(strProcedureIDs.GetLength()-2);

	return strProcedureIDs;
}

void CPicContainerDlg::CloseCleanup()
{
	if(GetEmrEditor()) {
		GetEmrEditor()->StopTrackingEMNTimes();

		// (j.jones 2006-12-19 08:55) - PLID 22794 - check the preference to open the room manager
		if(GetRemotePropertyInt("OpenRoomMgrWhenCloseEMR", 0, 0, GetCurrentUserName(), true) == 1) {
			GetMainFrame()->PostMessage(NXM_OPEN_ROOM_MANAGER, (long)-1,
				(BOOL)GetRemotePropertyInt("OpenRoomMgrWhenCloseEMRMinimized", 0, 0, GetCurrentUserName(), true) == 1);
		}
	}
}

void CPicContainerDlg::Commit()
{
	if(GetEmrEditor() && GetEmrEditor()->GetEmrID() != m_nEmrGroupID && m_nPicID != -1) {
		//They have a new ID, make sure our PIC record is linked to it.
		//might as well commit here, saves a step
		// (a.walling 2007-10-16 17:12) - PLID 27786 - Verify the EmrGroupID is not in use and is safe to overwrite
		//VerifyEmrGroupID(m_nPicID, GetEmrEditor()->GetEmrID());
		//ExecuteSql("UPDATE PicT SET IsCommitted = 1, EmrGroupID = %li WHERE ID = %li", GetEmrEditor()->GetEmrID(), m_nPicID);

		// (a.walling 2008-06-27 18:04) - PLID 30482 - Ensure we are not overwriting an EMRGroup that has already
		// been set for this PIC! If so, we just need to use that EMRGroup instead of this new one, and set our
		// new one to be deleted.
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		//TES 11/22/2010 - PLID 41582 - The EMR should now have saved itself to the PIC already.  So, just double-check that the PIC
		// doesn't have a DIFFERENT EmrGroupID stored to it.  We could have been doing this extra level of checking all along, and probably
		// should have.
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON "
			"DECLARE @EmrGroupID INT; "
			"SET @EmrGroupID = (SELECT EmrGroupsT.ID FROM PicT INNER JOIN EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID WHERE EmrGroupsT.Deleted = 0 AND PicT.ID = {INT} AND EmrGroupsT.ID <> {INT}) "
			"IF @EmrGroupID IS NULL BEGIN "
				"UPDATE PicT SET IsCommitted = 1, EmrGroupID = {INT} WHERE ID = {INT} "
			"END ELSE BEGIN SELECT @EmrGroupID AS ExistingEmrGroupID END "
			"SET NOCOUNT OFF ", m_nPicID, GetEmrEditor()->GetEmrID(), GetEmrEditor()->GetEmrID(), m_nPicID
			);

			if (prs->GetState() == adStateOpen && (!prs->eof)) {
				// oh noes!
				long nExistingGroupID = AdoFldLong(prs, "ExistingEmrGroupID", -1);
				if (nExistingGroupID != -1) {
					CEMR* pEMR = GetEmrEditor()->GetEMR();
					if (pEMR) {
						long nOldID = pEMR->GetID();
						pEMR->OverrideID(nExistingGroupID);

						// (a.walling 2012-05-18 17:18) - PLID 50546 - No longer necessary
						/*if (GetEmrEditor() != NULL && GetEmrEditor()->GetSafeHwnd() != NULL) {
							GetEmrEditor()->PostMessage(NXM_EMREDITOR_REFRESH_INFO);
						}	*/
						
						// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
						ExecuteParamSql("UPDATE PicT SET IsCommitted = 1 WHERE ID = {INT}", m_nPicID);
						if (nOldID != -1) {
							ExecuteParamSql(
								"UPDATE EmrGroupsT SET Deleted = 1, DeletedBy = {STRING}, DeleteDate = GetDate() WHERE ID = {INT}\r\n "
								"UPDATE EMRGroupsSlipT SET EMRGroupID = {INT} WHERE EMRGroupID = {INT}", CString(GetCurrentUserName())+CString("_Practice"), nOldID, nExistingGroupID, nOldID);

							// (c.haag 2008-08-05 11:12) - PLID 30853 - Migrate all problems linked to the EMR to the existing group ID
							// (c.haag 2009-05-11 17:32) - PLID 28494 - Update the new EMR problem linking table instead
							ExecuteParamSql(FormatString("UPDATE EMRProblemLinkT SET EMRRegardingID = {INT} WHERE EMRRegardingType = %li AND EMRRegardingID = {INT} ", eprtEmrEMR), nExistingGroupID, nOldID);
						}
						
						m_nEmrGroupID = GetEmrEditor()->GetEmrID();

						MessageBox("Another user has already created an EMR; your EMNs have been modified to use this EMR instead.");

						EmrSaveStatus essSaveAgainStatus = GetEmrEditor()->Save(TRUE);
						// (a.walling 2008-06-26 16:22) - PLID 30513 - Use FAILED macro to check for success
						if (FAILED(essSaveAgainStatus)) {
							MessageBox("The EMR could not be saved.");
						}						

						return;
					}
				} else {
					ASSERT(FALSE);
				}
			}

		m_nEmrGroupID = GetEmrEditor()->GetEmrID();
	}
	else {
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		ExecuteParamSql("UPDATE PicT SET IsCommitted = 1 WHERE ID = {INT}", m_nPicID);
	}
	SetIsCommitted(TRUE);
}

// (j.jones 2007-02-06 15:23) - PLID 24493 - tell EMR when the workstation time is changed
void CPicContainerDlg::FireTimeChanged()
{
	try {

		if(GetEmrEditor())
			GetEmrEditor()->FireTimeChanged();

	} NxCatchAll("Error in CPicContainerDlg::FireTimeChanged");
}

BOOL CPicContainerDlg::AnyProcedureNotInEMN(CArray<int,int>& aProcIDs, CEMN* pEMN)
{
	//
	// (c.haag 2007-03-07 14:51) - PLID 21207 - Returns TRUE if an EMN does not have
	// a procedure that exists in aProcIDs
	//
	if (!pEMN) {
		return FALSE;
	} else {
		//DRT 7/13/2007 - PLID 26671 - GetProcedures() changed to return the full procedures, 
		//	not just the IDs, so this function needs to handle it.
		CArray<EMNProcedure, EMNProcedure> aryProcedures;
		pEMN->GetProcedures(aryProcedures);
		for (int i=0; i < aProcIDs.GetSize(); i++) {
			BOOL bFound = FALSE;
			long nSize = aryProcedures.GetSize();
			// (a.walling 2008-08-11 17:33) - PLID 31035 - This was redefining i and causing access violations / array out of bounds / garbage
			for(int j = 0; j < nSize && !bFound; j++) {
				if(aProcIDs[i] == aryProcedures.GetAt(j).nID) {
					//found it
					bFound = TRUE;
				}
			}

			if(!bFound)
				return TRUE;
		}
		return FALSE;
	}
}

BOOL CPicContainerDlg::AnyProcedureNotInEMN(CArray<int,int>& aProcIDs, long nEMNID)
{
	//
	// (c.haag 2007-03-07 14:51) - PLID 21207 - Returns TRUE if an EMN does not have
	// a procedure that exists in aProcIDs
	//
	if (GetRecordCount("SELECT EMRID FROM EmrProcedureT WHERE Deleted = 0 AND EMRID = " + AsString(nEMNID) + " AND ProcedureID IN (" + ArrayAsString(aProcIDs,false) + ")") == aProcIDs.GetSize()) {
		return FALSE;
	} else {
		return TRUE;
	}
}

LRESULT CPicContainerDlg::OnNonClinicalProceduresAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		//
		// (c.haag 2007-03-07 09:14) - PLID 21207 - This message is posted when one or
		// more procedures are added to the non-clinical tab. This is posted from the
		// CProcInfoCenterDlg class and must be passed to the editor dialog if it exists
		//
		CArray<int,int>* paProcIDs = (CArray<int,int>*)wParam;
		CArray<CString,CString>* paProcNames = (CArray<CString,CString>*)lParam;

		// Requires full EMR licensing
		if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			delete paProcIDs;
			delete paProcNames;
			return 0;
		} 

		try {
			int nProcedures = paProcIDs->GetSize();
			CString strMsg;
			int i;

			// We should never get here without procedures
			ASSERT(nProcedures > 0);

			//
			// Handling for when the editor dialog is not null and all of the EMN data
			// is in memory
			//
			if (NULL != GetEmrEditor()) {
				CArray<CEMN*,CEMN*> apEMNs;
				CArray<CEMN*,CEMN*> apEMNsToAdd;
				int nEMNs = GetEmrEditor()->GetEMNCount();

				// Build the list of EMN's which to apply procedures to
				for (i=0; i < nEMNs; i++) {
					CEMN* pEMN = GetEmrEditor()->GetEMN(i);
					if (AnyProcedureNotInEMN(*paProcIDs, pEMN) && pEMN->GetStatus() != 2) {
						apEMNs.Add(GetEmrEditor()->GetEMN(i));
					}
				}
				nEMNs = apEMNs.GetSize();

				// Handle cases where there is one EMN to add the procedure to
				if (nEMNs == 1) {
					CEMN* pEMN = apEMNs[0];
					switch (GetRemotePropertyInt("EMR_AutoAddProcedureFromPICToEMN", 1, 0, GetCurrentUserName(), true)) {
					case 0: // Automatically add it to the EMN
						apEMNsToAdd.Add(pEMN);
						break;
					case 1: // Prompt the user 
						if (nProcedures == 1) {
							strMsg.Format("Do you wish to add the %s procedure to the '%s' EMN?", paProcNames->GetAt(0), apEMNs[0]->GetDescription());
						} else {
							strMsg.Format("Do you wish to add the new procedures to the '%s' EMN?", apEMNs[0]->GetDescription());
						}
						if (IDYES == MessageBox(strMsg, "New Procedures", MB_ICONQUESTION|MB_YESNO)) {
							apEMNsToAdd.Add(pEMN);
						}		
						break;
					case 2: // Do not add it to the EMN
						break;
					default:
						ASSERT(FALSE); // (c.haag 2007-03-07 09:46) - This should never happen
						break;
					}
				} 
				// Handle cases where there is more than one EMN to add the procedures to
				else if (nEMNs > 1) {
					switch (GetRemotePropertyInt("EMR_AutoAddProcedureFromPICToMultipleEMNs", 1, 0, GetCurrentUserName(), true)) {
					case 0: // Automatically add it to every EMN
						for (i=0; i < nEMNs; i++) { apEMNsToAdd.Add(apEMNs[i]); }
						break;
					case 1: // Prompt the user as to which EMNs to add the procedure to
						{
							// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
							CMultiSelectDlg dlg(this, "EmrMasterT");
							CString strSql = "(";
							for (i=0; i < nEMNs; i++) {
								CEMN* pEMN = apEMNs[i];
								strSql += FormatString("SELECT %d AS ID, '%s' AS Description ", i, _Q(pEMN->GetDescription()));
								if (i < nEMNs - 1) {
									strSql += "UNION ";
								}
								dlg.PreSelect(i);
							}
							strSql += ") SubQ";

							if (nProcedures == 1) {
								strMsg = "Please select EMNs to apply the new procedure to";
							} else {
								strMsg = "Please select EMNs to apply the new procedures to";
							}
							
							if (IDOK == dlg.Open(strSql, "", "ID", "Description", strMsg))
							{
								CArray<long,long> anIndex;
								dlg.FillArrayWithIDs(anIndex);
								for (i=0; i < anIndex.GetSize(); i++) {
									 apEMNsToAdd.Add(apEMNs[anIndex[i]]);
								}
							}
						}
						break;
					case 2: // Do not add it to any EMN
						break;
					default:
						ASSERT(FALSE); // (c.haag 2007-03-06 17:49) - This should never happen
						break;
					}
				}
				
				// When we get here, we finally have a list of EMN's to apply the procedure to. If there
				// is anything in the list, make sure that the editor dialog exists, and then apply the
				// procedure to those EMN's.
				if (apEMNsToAdd.GetSize() > 0) {

					// Rather than adding everything here, we want to treat it the same as if the user
					// went to the more info tab and selected each procedure. So, take what we have and
					// make the editor dialog do the rest of the work
					CArray<CEMN*,CEMN*>* papEMNs = new CArray<CEMN*,CEMN*>;
					CArray<int,int>* paProcIDForEditor = new CArray<int,int>;
					papEMNs->Copy(apEMNsToAdd);
					paProcIDForEditor->Copy(*paProcIDs);
					GetEmrEditor()->PostMessage(NXM_NON_CLINICAL_PROCEDURES_ADDED, (WPARAM)paProcIDForEditor, (LPARAM)papEMNs);
				}
			}
			//
			// Handling for when the editor dialog has not been created and the EMN's are not in memory;
			// but rather just the data
			//
			else if (m_nEmrGroupID > 0) {
				CArray<long,long> anEMNIDs;
				CArray<long,long> anEMNsToAdd;
				CStringArray astrEMNNames;
				int nEMNs = 0;

				// Build the list of EMN's which to apply procedures to
				//DRT 8/28/2007 - PLID 27203 - Parameterized.
				_RecordsetPtr prs = CreateParamRecordset("SELECT ID, Description FROM EMRMasterT WHERE EMRGroupID = {INT} AND Deleted = 0 AND Status <> 2", m_nEmrGroupID);
				FieldsPtr f = prs->Fields;
				while (!prs->eof) {
					if (AnyProcedureNotInEMN(*paProcIDs, AdoFldLong(f, "ID"))) {
						anEMNIDs.Add(AdoFldLong(f, "ID"));
						astrEMNNames.Add(AdoFldString(f, "Description"));
						nEMNs++;
					}
					prs->MoveNext();				
				}
				prs->Close();

				// Handle cases where there is one EMN to add the procedure to
				if (nEMNs == 1) {
					switch (GetRemotePropertyInt("EMR_AutoAddProcedureFromPICToEMN", 1, 0, GetCurrentUserName(), true)) {
					case 0: // Automatically add it to the EMN
						anEMNsToAdd.Add(anEMNIDs[0]);
						break;
					case 1: // Prompt the user 
						if (nProcedures == 1) {
							strMsg.Format("Do you wish to add the %s procedure to the '%s' EMN?", paProcNames->GetAt(0), astrEMNNames[0]);
						} else {
							strMsg.Format("Do you wish to add the new procedures to the '%s' EMN?", astrEMNNames[0]);
						}
						if (IDYES == MessageBox(strMsg, "New Procedures", MB_ICONQUESTION|MB_YESNO)) {
							anEMNsToAdd.Add(anEMNIDs[0]);
						}		
						break;
					case 2: // Do not add it to the EMN
						break;
					default:
						ASSERT(FALSE); // (c.haag 2007-03-07 09:46) - This should never happen
						break;
					}
				} 
				// Handle cases where there is more than one EMN to add the procedures to
				else if (nEMNs > 1) {
					switch (GetRemotePropertyInt("EMR_AutoAddProcedureFromPICToMultipleEMNs", 1, 0, GetCurrentUserName(), true)) {
					case 0: // Automatically add it to every EMN
						for (i=0; i < nEMNs; i++) { anEMNsToAdd.Add(anEMNIDs[i]); }
						break;
					case 1: // Prompt the user as to which EMNs to add the procedure to
						{
							// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
							CMultiSelectDlg dlg(this, "EmrMasterT");
							if (nProcedures == 1) {
								strMsg = "Please select EMNs to apply the new procedure to";
							} else {
								strMsg = "Please select EMNs to apply the new procedures to";
							}

							dlg.PreSelect(anEMNIDs);						
							if (IDOK == dlg.Open("EmrMasterT", CString("ID IN (") + ArrayAsString(anEMNIDs,false) + ")", "ID", "Description", strMsg))
							{
								CArray<long,long> anIndex;
								dlg.FillArrayWithIDs(anIndex);
								for (i=0; i < anIndex.GetSize(); i++) {
									anEMNsToAdd.Add(anIndex[i]);								
								}
							}
						}
						break;
					case 2: // Do not add it to any EMN
						break;
					default:
						ASSERT(FALSE); // (c.haag 2007-03-06 17:49) - This should never happen
						break;
					}
				}

				// When we get here, we finally have a list of EMN's to apply the procedure to. If there
				// is anything in the list, make sure that the editor dialog exists, and then apply the
				// procedure to those EMN's.
				if (anEMNsToAdd.GetSize() > 0) {
					CWaitCursor wc;

					// Add the procedures to data
					CArray<long,long> arProcIDs;
					// (a.walling 2008-08-11 17:35) - PLID 30515 - Keep track of any failed procedure insertions
					CStringArray saErrors;
					try {
						for (i=0; i < paProcIDs->GetSize(); i++) {
							for (int j=0; j < anEMNsToAdd.GetSize(); j++) {
								// If the EMN/Procedure combination already exist, then do nothing
								// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
								if (!ReturnsRecordsParam("SELECT EMRID FROM EmrProcedureT WHERE EMRID = {INT} AND ProcedureID = {INT}", anEMNsToAdd[j], paProcIDs->GetAt(i))) {
									// (a.walling 2008-08-11 17:35) - PLID 30515 - We have to handle each EMN seperately so we can track which ones succeeded and which may have failed
									CString strSqlBatch = BeginSqlBatch();
									BOOL bSuccess = TRUE;
									try {
										// (j.armen 2013-05-14 12:39) - PLID 56680 - EMN Access Refactoring
										AddStatementToSqlBatch(strSqlBatch, 
											"IF EXISTS(\r\n"
											"	SELECT 1\r\n"
											"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
											"	WHERE EmnID = %li)\r\n"
											"BEGIN\r\n"
											"	RAISERROR('Procedure cannot be saved; the EMN is being modified by another user.', 16, 43)\r\n"
											"	ROLLBACK TRAN\r\n"
											"	RETURN\r\n"
											"END\r\n", anEMNsToAdd[j]);
										AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EmrProcedureT (EMRID, ProcedureID) VALUES (%li, %li)", anEMNsToAdd[j], paProcIDs->GetAt(i));
										ExecuteSqlBatch(strSqlBatch);
										long nAuditTransactionID = BeginAuditTransaction();
										AuditEvent(GetPatientID(), GetPatientName(), nAuditTransactionID, aeiEMNProcedureAdded, anEMNsToAdd[j], "", paProcNames->GetAt(i), aepMedium, aetCreated);
									} catch (_com_error e) {
										// (a.walling 2008-08-27 14:10) - PLID 30515 - Use common function to get SQL Error Info.
										CSQLErrorInfo eSqlError;
										if (GetSQLErrorInfo(e, eSqlError) && eSqlError.IsNxRaisedError() && eSqlError.nState == 43) {
											// write token is missing or invalid
											saErrors.Add(FormatString("The procedure '%s' could not be added to the EMN '%s'.", paProcNames->GetAt(i), VarString(GetTableField("EMRMasterT", "Description", "ID", anEMNsToAdd[j]), "")));										
										} else {
											throw e;
										}
									} catch (...) {
										throw;
									}
								}
							}
							arProcIDs.Add(paProcIDs->GetAt(i));
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
					NxCatchAllThrow("Error in CPicContainerDlg::OnNonClinicalProceduresAdded Sub");

					// Now do post-procedure addition handling
					HandleNewEmrProcedures(arProcIDs);
				}
			}
		}
		NxCatchAll("Error in CPicContainerDlg::OnNonClinicalProceduresAdded");
		delete paProcIDs;
		delete paProcNames;
	} NxCatchAll(__FUNCTION__);
	return 0;
}

void CPicContainerDlg::SetPatientID(long nPatID)
{
	// (c.haag 2007-03-07 16:00) - PLID 25110 - Sets the patient ID
	m_nPatientID = nPatID;
}

long CPicContainerDlg::GetPatientID() const
{
	// (c.haag 2007-03-07 15:59) - PLID 25110 - Returns the patient ID
	if (-1 == m_nPatientID) {
		ASSERT(FALSE); // This should never happen
		ThrowNxException("The patient ID has not been assigned to the Procedure Information Center!");
	}
	return m_nPatientID;
}

CString CPicContainerDlg::GetPatientName() const
{
	// (c.haag 2007-03-07 15:59) - PLID 25110 - Returns the name of the active patient.
	// Don't keep it in the PIC because the user could change it while the PIC is open
	// (c.haag 2007-03-08 12:58) - I forgot we had this function. This is better than using
	// a query.
	return GetExistingPatientName(GetPatientID());
}

// (a.walling 2007-08-27 09:19) - PLID 26195 - Return whether the EMR is unsaved or not.
BOOL CPicContainerDlg::IsEMRUnsaved()
{
	if (GetEmrEditor()) {
		return GetEmrEditor()->IsEMRUnsaved();
	} else {
		return FALSE;
	}
}

// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
// and returns TRUE if any Image detail on this EMR references it
BOOL CPicContainerDlg::IsImageFileInUseOnEMR(const CString strFileName)
{
	if (GetEmrEditor()) {
		return GetEmrEditor()->IsImageFileInUseOnEMR(strFileName);
	} else {
		return FALSE;
	}
}

// (a.walling 2007-10-16 17:28) - PLID 27786 - Throws an exception if the EmrGroupID of this PicT record is not null,
// not a deleted group, and is not the desired group
BOOL CPicContainerDlg::VerifyEmrGroupID(long nPicID, long nDesiredGroupID)
{
	// in order to stay as consistent as possible with current behaviour, I am not going to
	// actually PREVENT anything from being overwritten here. I am just logging it and warning
	// the user. They must call us. If I were to prevent overwriting, I run the risk of causing
	// other issues deeper in logic. Since no data is lost, and we log all relevant information
	// here, _and_ because it is easy to find the problem even just by looking at the data, I
	// do not consider that to be much of an issue. A warning should be enough.
	try {
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT EmrGroupID FROM PicT INNER JOIN EmrGroupsT ON EmrGroupID = EmrGroupsT.ID "
			"WHERE PicT.ID = {INT} AND EmrGroupsT.Deleted = 0", nPicID);
		// if we have no records returned, great! That means the EmrGroupID is either NULL, does not
		// exist in EmrGroupsT (should never happen), or the EmrGroup is Deleted. So we are OK to overwrite.
		if (prs->eof) {
			return TRUE;
		} else {
			long nExistingGroupID = AdoFldLong(prs, "EmrGroupID", -1);
			if (nExistingGroupID == -1) { // should never happen, but just for completeness
				return TRUE;
			} else if (nExistingGroupID == nDesiredGroupID) { // already set to the desired group id
				return TRUE;
			} else {
				// bad! Throw the exception, return FALSE after the exception handler.
				LogDetail("WARNING: Attempted to set PIC %li with existing EmrGroup %li to use EmrGroup %li", nPicID, nExistingGroupID, nDesiredGroupID);
				// this is mostly to convince the user to call us if this occurs. We are within our own try/catch block so as not to mess up anything.
				// (a.walling 2007-10-17 17:12) - PLID 27786 - According to Christina, this has been slightly reworded.
				ThrowNxException("A consistency issue may exist; please contact Nextech Support as soon as possible. No data has been lost.");
			}
		}
	// (a.walling 2008-06-27 16:15) - PLID 30482 - Throw if this fails.
	} NxCatchAllThrow_NoParent("Error saving EMR group!"); // (a.walling 2014-05-05 13:32) - PLID 61945

	return FALSE;
}

// (a.walling 2008-05-05 11:11) - PLID 29894 - Function sets and returns the new surgery appt id for this PIC
long CPicContainerDlg::UpdateSurgeryAppt()
{
	long nNewSurgApptID = -1;

// update the surgery appt if necessary
	//TES 8/24/2009 - PLID 35322 - Include "Minor Procedure" appointments (Category 3).
	_RecordsetPtr prsSurgeryAppt = CreateParamRecordset(
			"SELECT COALESCE(MIN(AppointmentsT.ID), -1) AS ApptID "
			"FROM    AppointmentsT "
					"LEFT JOIN ProcInfoT "
					"ON      AppointmentsT.ID = ProcInfoT.SurgeryApptID "
			"WHERE   AptTypeID               IN "
					"(SELECT ID "
					"FROM    AptTypeT "
					"WHERE   Category IN (3,4) "
					") "
				"AND Status                 <> 4 "
				"AND ShowState              <> 3 "
				"AND AppointmentsT.PatientID = {INT} "
				"AND AppointmentsT.ID       IN "
					"(SELECT AppointmentID "
					"FROM    AppointmentPurposeT "
					"WHERE   PurposeID IN "
							"(SELECT ProcedureID "
							"FROM    ProcInfoDetailsT "
							"WHERE   ProcInfoID = {INT} "
							") "
						"AND ProcInfoT.ID IS NULL "
					") ", GetPatientID(), m_nProcInfoID);

	if (!prsSurgeryAppt->eof) {
		nNewSurgApptID = AdoFldLong(prsSurgeryAppt, "ApptID", -1);
		if (nNewSurgApptID != -1) {
			ExecuteParamSql("UPDATE ProcInfoT SET SurgeryApptID = {INT} WHERE ID = {INT}", nNewSurgApptID, m_nProcInfoID);
			//TES 11/25/2008 - PLID 32066 - Check whether the surgeon should be changed based on the new appointment.
			PhaseTracking::UpdateProcInfoSurgeon(m_nProcInfoID, nNewSurgApptID);
		}
	}

	return nNewSurgApptID;
}

LRESULT CPicContainerDlg::OnPrintPrescriptions(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 6/20/2008 - PLID 30414 - Save first (copied from SaveAndClose()).

		// (c.haag 2006-04-12 09:25) - PLID 20040 - Now we only assign this if we haven't already
		if (!GetIsCommitted()) {
			Commit();
		}

		// Save the EMR if we've got one
		if(GetEmrEditor()) {
			//the Save wants to know if we are propagating new IDs,
			//which we normally wouldn't do since we are closing, but we need to
			//if we are performing any actions after saving, such as making a bill

			// (c.haag 2006-03-28 12:13) - PLID 19890 - We now consider permissions before saving EMR's
			// (a.walling 2007-11-28 11:33) - PLID 28044 - Consider licensing and expiration too
			if (CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
				//TES 8/4/2006 - We want to propogate IDs if the EMR is new, so that just below here we can update the PicT
				// record with the new Emr ID.
				EmrSaveStatus essSaveStatus = GetEmrEditor()->Save(TRUE);
				// (a.walling 2008-06-26 16:22) - PLID 30513 - Use FAILED macro to check for success
				if (FAILED(essSaveStatus)) {
					MessageBox("The EMR could not be saved.");
					return 0;
				}
				else {
					if(GetEmrEditor()->GetEmrID() != m_nEmrGroupID && m_nPicID != -1) {
						//They have a new ID, make sure our PIC record is linked to it.
						// (a.walling 2007-10-16 17:11) - PLID 27786 - Verify the EmrGroupID is not in use and is safe to overwrite
						VerifyEmrGroupID(m_nPicID, GetEmrEditor()->GetEmrID());
						ExecuteSql("UPDATE PicT SET EmrGroupID = %li WHERE ID = %li", GetEmrEditor()->GetEmrID(), m_nPicID);
					}
				}

				//unless we had complete success, do not close
				if(essSaveStatus != essSuccess) {
					return 0;
				}
			}
		}

		//TES 6/17/2008 - PLID 30414 - Just call our function.
		PrintPrescriptions((CEMN*)wParam, (BOOL)lParam);
	}NxCatchAll("Error in CPicContainerDlg::OnPrintPrescriptions()");
	return 0;
}

void CPicContainerDlg::PrintPrescriptions(CEMN *pEMN, BOOL bDirectlyToPrinter, const CString &strTemplateName /*= ""*/)
{
	//TES 6/17/2008 - PLID 30414 - Copied this code out of SaveAndClose(), it's now called from a couple different message
	// handlers.

	//TODO: should we really save? can we merge to Word without saving?
	//could be a bad idea to let them merge unsaved prescriptions

	if(pEMN) {

		CWaitCursor pWait;

		long nPatientID = pEMN->GetParentEMR()->GetPatientID();
		
		/// Generate the temp table
		CString strSql;
		strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", nPatientID);
		CString strMergeT = CreateTempIDTable(strSql, "ID");
		
		// Merge
		CMergeEngine mi;

		// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
		mi.LoadSenderInfo(FALSE);

		//calculate the EMR ID
		long nEMRID = -1;
		if(pEMN->GetParentEMR()) {
			nEMRID = pEMN->GetParentEMR()->GetID();
		}

		if(nEMRID == -1)
			ASSERT(FALSE);

		long nEMNID = pEMN->GetID();
		if(nEMNID == -1) {
			//TES 6/17/2008 - PLID 30414 - There was a bunch of code here to try and deduce the EMN ID if we
			// couldn't get it from pEMN.  However, that hasn't been needed for quite a while, ever since we
			// introduced the PropagateIDs stuff.  I took it all out.
			ASSERT(FALSE);
		}

		if(!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
		// based on how many prescriptions are printed
		CString strTemplate;
		if(strTemplateName.IsEmpty()) {
			//TES 6/17/2008 - PLID 30414 - We weren't given a template, so we need to calculate it.
			long nCountPrescriptions = pEMN->GetMedicationCount();
			BOOL bExactCountFound = FALSE;
			BOOL bOtherCountFound = FALSE;
			strTemplate = GetDefaultPrescriptionTemplateByCount(nCountPrescriptions, bExactCountFound, bOtherCountFound);

			if(strTemplate == "") {
				MessageBox( 
					"There is no default template set up. The prescription cannot be printed.\n"
					"Please go to the Medications tab to set up your default prescription template."
					, NULL
					, MB_OK|MB_ICONINFORMATION);
				return;
			}
		}
		else {
			strTemplate = strTemplateName;
		}
		
		// (c.haag 2004-11-11 13:29) - PLID 14074 - Assign the EMR group ID so we can tie
		// this document in with the history tab for this EMR group
		long nPicID = VarLong(GetTableField("PicT", "ID", "EmrGroupID", nEMRID));
		mi.m_nPicID = nPicID;

		CString strPrescriptionIDs;
		CString strPatHistoryDescription = "Prescription printed for ";

		//add all prescriptions to the merge
		// (c.haag 2007-02-02 18:28) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		_RecordsetPtr rs = CreateRecordset("SELECT PatientMedications.*, EMRDataT.Data AS Name FROM PatientMedications "
			"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE PatientMedications.ID IN (SELECT MedicationID FROM EMRMedicationsT WHERE Deleted = 0 AND EMRID = %li)", nEMNID);
		while(!rs->eof) {
			mi.m_arydwPrescriptionIDs.Add(AdoFldLong(rs, "ID",-1));
			
			CString MedicationName = CString(rs->Fields->Item["Name"]->Value.bstrVal);
			CString strRefills = CString(rs->Fields->Item["RefillsAllowed"]->Value.bstrVal);
			CString strQuantity = CString(rs->Fields->Item["Quantity"]->Value.bstrVal);
			CString strUnit = AdoFldString(rs, "Unit");
			CString strExtra;
			strExtra.Format("%s, Quantity: %s %s, Refills: %s  /  ",MedicationName, strQuantity, strUnit, strRefills);
			strPatHistoryDescription += strExtra;

			if(strPatHistoryDescription.Right(5) == "  /  ")
				strPatHistoryDescription = strPatHistoryDescription.Left(strPatHistoryDescription.GetLength()-5);

			rs->MoveNext();
		}
		rs->Close();

		// (c.haag 2004-05-10 12:25) - Add procedures to the merge
		if (pEMN->GetProcedureCount() > 0)
		{
			CString strProcIDs;
			for(int i = 0; i < pEMN->GetProcedureCount(); i++) {
				CString strID;
				strID.Format("%li, ", pEMN->GetProcedure(i)->nID);
				strProcIDs += strID;
			}
			strProcIDs = strProcIDs.Left(strProcIDs.GetLength()-2);
			_RecordsetPtr prsProcInfoDetails = CreateRecordset("SELECT ID FROM ProcInfoDetailsT WHERE ProcedureID IN (%s) AND ProcInfoID IN (SELECT ID FROM ProcInfoT WHERE PatientID = %d)", 
				strProcIDs, nPatientID);
			while (!prsProcInfoDetails->eof)
			{	
				mi.m_arydwProcInfoIDs.Add(AdoFldLong(prsProcInfoDetails, "ID"));
				prsProcInfoDetails->MoveNext();
			}
		}
		mi.m_arydwEMRIDs.Add(nEMNID);

		if (g_bMergeAllFields)
			mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

		if(bDirectlyToPrinter) {
			mi.m_nFlags |= BMS_MERGETO_PRINTER;
		}

		mi.m_nFlags |= BMS_SAVE_FILE_NO_HISTORY; //save the file, do not save in history

		// Do the merge
		// (z.manning 2016-06-03 8:41) - NX-100806 - Check the return value
		if (mi.MergeToWord(GetTemplatePath("Forms", strTemplate), AsVector(m_saTempFiles), strMergeT))
		{
			//Update patient history here, because multiple merges per patient ID
			//will screw up the merge engine's method of doing it. But hey,
			//we get to make the description a lot better as a result!

			// (j.jones 2008-09-04 17:12) - PLID 30288 - converted to use CreateNewMailSentEntry,
			// which creates the data in one batch and sends a tablechecker
			// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
			// (d.singleton 2013-11-15 11:23) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);
			CreateNewMailSentEntry(nPatientID, strPatHistoryDescription, SELECTION_WORDDOC, mi.m_strSavedAs, GetCurrentUserName(), "", GetCurrentLocationID(), dtNull, -1, -1, nPicID, -1, FALSE, -1, "", ctNone);

			// (j.jones 2007-09-13 08:58) - PLID 27371 - need to send an EMR tablechecker
			//refresh the EMN to update the Word icon
			CClient::RefreshTable(NetUtils::EMRMasterT, nPatientID);
		}
	}
}

//(e.lally 2008-07-29) PLID 30732 - Minimize ourself if we aren't already
LRESULT CPicContainerDlg::OnEmrMinimizePic(WPARAM wParam, LPARAM lParam)
{
	try {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (this->GetWindowPlacement(&wp)) {
			//Check if we are not minimized
			if (!this->IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				this->SetWindowPlacement(&wp);
				//(e.lally 2008-07-30) For some reason, the set window placement likes to bring non-practice apps to the foreground.
				//I am only aware of resetting the mainframe as the foreground to workaround that. It only worked after the minimize too.
				GetMainFrame()->SetForegroundWindow();
			}
		}
	}NxCatchAll("Error in CPicContainerDlg::OnEmrMinimizePic()");
	return 0;
}

// (a.walling 2008-09-10 13:24) - PLID 31334 - Set the active PIC in the mainframe
// (a.walling 2009-06-22 10:45) - PLID 34635 - This is useless code which may end up causing problems.
/*
void CPicContainerDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	//TRACE("PicContainer Activate (%li, 0x%08x, %li)\n", nState, pWndOther->GetSafeHwnd(), bMinimized);

	try {
		if (GetMainFrame()) {
			if (nState != WA_INACTIVE) {
				GetMainFrame()->SetLastActivePIC(this);
			} else {
				GetMainFrame()->EnsureNotActivePIC(this);
			}		
		}
	} NxCatchAllCallIgnore({LogDetail("Error in CPicContainerDlg::OnActivate");});

	CEmrPatientFrameWnd::OnActivate(nState, pWndOther, bMinimized);
}
*/

LRESULT CPicContainerDlg::OnWIAEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		if (GetHistoryDlg()->GetSafeHwnd()) {
			return GetHistoryDlg()->SendMessage(NXM_WIA_EVENT, wParam, lParam);
		}
	} NxCatchAll("CPicContainerDlg::OnWIAEvent");

	return (LRESULT)-1;
}

//TES 7/13/2009 - PLID 25154 - This function goes through any EMNs on the dialog, and checks to make sure that, if
// they have the "Send to HL7" box checked, they get sent to HL7.  This function should be called just before
// closing the PIC, but after any saving has taken place.
//(a.wilson 2013-06-13) PLID 57165 - check for changes to charges in case they forgot to check the send to hl7 in more info.
void CPicContainerDlg::SendEmnBillsToHL7()
{
	//TES 7/10/2009 - PLID 25154 - We need to go through and find any EMNs that want to be sent to HL7, and send them.
	//  Code copied from NewPatient.cpp
	if(g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent) && GetEmrEditor() != NULL) {
		CArray<long, long> arEmnIDsToSend;
		CArray<long, long> arEmnIDsWithChangedCharges;
		for(int i = 0; i < GetEmrEditor()->GetEMNCount(); i++) {
			CEMN *p = GetEmrEditor()->GetEMN(i);
			if(!p->IsTemplate() && p->GetChargeCount() > 0) {
				long nID = GetEmrEditor()->GetEMN(i)->GetID();
				//TES 7/13/2009 - PLID 25154 - If the ID is -1, that means that the EMN hasn't been saved, and since
				// we know that any saving code has been called, that means it never will be.
				if (nID > 0) {
					//check if the user marked the emn to be sent.
					if (p->GetSendBillToHL7()) {
						arEmnIDsToSend.Add(nID);
					//check if any charges were changed.
					} else if (p->GetChargesChanged()) {
						arEmnIDsWithChangedCharges.Add(nID);
					}
				}
			}
		}
		//(a.wilson 213-06-13) PLID 57165 - if any charges were changed but the check to send to hl7 was not marked:
		//check data to see if emn was ever sent before if so then prompt user if they would like to send again with changes.
		if (arEmnIDsWithChangedCharges.GetSize()) {
			CString strEMNDescriptions;
			CArray<long, long> arEmnIDsToResend;
			_RecordsetPtr prs = CreateParamRecordset("SELECT EMNID FROM EMNBillsSentToHL7T WHERE EMNID IN ({INTARRAY})", arEmnIDsWithChangedCharges);

			for (; !prs->eof; prs->MoveNext()) {
				long nEMNID = AdoFldLong(prs, "EMNID");
				arEmnIDsToResend.Add(nEMNID);
				CEMN *p = GetEmrEditor()->GetEMNByID(nEMNID);
				strEMNDescriptions += FormatString("\r\n- %s (%s)", p->GetDescription(), FormatDateTimeForInterface(p->GetEMNDate(), 0, dtoDate, false));
			}
			prs->Close();

			// (r.gonet 04/16/2014) - PLID 40220 - Updated the text from More Info topic to Codes topic
			if (arEmnIDsToResend.GetSize() && IDYES == MessageBox(FormatString(
				"You have made changes to at least one charge on the following EMNs: "
				"%s\r\n\r\n"
				"These EMNs have previously sent their billing information to HL7. "
				"Would you like to resend their billing information to HL7 with any other EMNs set to send? \r\n\r\n"
				"If you don't want to do this now, you can always come back and check the 'send billing info. to HL7' box on the EMN <Codes> topic to resend. " 
				, strEMNDescriptions), "Resend EMNs to HL7", MB_ICONQUESTION | MB_YESNO)) {

				arEmnIDsToSend.Append(arEmnIDsToResend);
			}
		}
		if(arEmnIDsToSend.GetSize()) {
			//TES 7/10/2009 - PLID 25154 - OK, some want to be sent, now let's see if we have any HL7 Settings
			// that allow them to be sent.
			CArray<HL7SettingsGroup,HL7SettingsGroup&> arDefaultGroups;
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
			CArray<long,long> arDefaultGroupIDs;
			GetHL7SettingsGroupsBySetting("ExportEmnBills", TRUE, arDefaultGroupIDs);
			for(int i = 0; i < arDefaultGroupIDs.GetSize(); i++) {
				long nGroupID = arDefaultGroupIDs[i];
				HL7SettingsGroup hsg;
				hsg.nID = nGroupID;
				hsg.nExportType = GetHL7SettingInt(nGroupID, "ExportType");
				hsg.bExpectAck = GetHL7SettingBit(nGroupID, "ExpectAck");
				hsg.strName = GetHL7GroupName(nGroupID);
				arDefaultGroups.Add(hsg);
			}
			if(arDefaultGroups.GetSize()) {
				for(int i = 0; i < arDefaultGroups.GetSize(); i++) {
					HL7SettingsGroup hsg = arDefaultGroups[i];
					BOOL bExpectAck = (hsg.nExportType == 1 && hsg.bExpectAck);
					for(int j = 0; j < arEmnIDsToSend.GetSize(); j++) {
						//TES 7/10/2009 - PLID 25154 - We made it, send this EMN to this HL7 group.
						// (r.gonet 12/03/2012) - PLID 54109 - Changed to use refactored send function.
						// (r.gonet 12/11/2012) - PLID 54109 - Have CMainFrame handle any failures.
						GetMainFrame()->GetHL7Client()->SendNewEmnBillHL7Message(arEmnIDsToSend[j], hsg.nID, true);
					}
				}
			}
		}
	}
}

// (j.jones 2009-08-25 11:37) - PLID 31459 - supported allowing an external refresh of the Proc Info payments
LRESULT CPicContainerDlg::OnReloadProcInfoPays(WPARAM wParam, LPARAM lParam)
{
	try {

		//the wParam is the ProcInfoID, so if this PIC doesn't contain
		//the desired ProcInfoID, do nothing
		long nProcInfoID = (long)wParam;
		if(nProcInfoID == GetCurrentProcInfoID()) {
			if(GetPicDlg()) {
				GetPicDlg()->Load(PIC_AREA_PAY);
			}
		}

	}NxCatchAll("Error in CPicContainerDlg::OnReloadProcInfoPays");

	return 0;
}

// (j.jones 2010-03-31 17:01) - PLID 37980 - added ability to tell the EMR to add a given image
LRESULT CPicContainerDlg::OnAddImageToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		if(GetEmrEditor() == NULL) {
			//No EMR exists. Silently exit, but log that this occurred.
			
			BSTR bstrTextData = (BSTR)wParam;
			CString strImageFile(bstrTextData);
			SysFreeString(bstrTextData);

			Log("CPicContainerDlg::OnAddImageToEMR - Could not add image file '%s' to an EMR because the active PIC has no EMR.", strImageFile);
			return 0;
		}
		else {
			GetEmrEditor()->PostMessage(NXM_ADD_IMAGE_TO_EMR, wParam, lParam);
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2010-03-31 17:33) - PLID 37980 - returns TRUE if GetEmrEditor() has an EMN opened to a topic that is writeable
BOOL CPicContainerDlg::HasWriteableEMRTopicOpen()
{
	if(GetEmrEditor()) {
		return GetEmrEditor()->HasWriteableEMRTopicOpen();
	}
	else {
		return FALSE;
	}
}

// (j.jones 2010-04-01 11:58) - PLID 37980 - gets the pointer to the active EMN, if there is one
CEMN* CPicContainerDlg::GetActiveEMN()
{
	if(GetEmrEditor()) {
		return GetEmrEditor()->GetActiveEMN();
	}
	else {
		return NULL;
	}
}

// (z.manning 2011-10-28 17:49) - PLID 44594
CEMR* CPicContainerDlg::GetEmr()
{
	if(GetEmrEditor()) {
		return GetEmrEditor()->GetEMR();
	}
	else {
		return NULL;
	}
}

// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
LRESULT CPicContainerDlg::OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam)
{
	try {

		if(GetEmrEditor() == NULL) {
			//No EMR exists. Silently exit, but log that this occurred.
			Log("CPicContainerDlg::OnAddGenericTableToEMR - Could not add a generic table to an EMR because the active PIC has no EMR.");
			return 0;
		}
		else {

			// Flip to the Clinical tab
			// (d.lange 2011-04-12 17:16) - PLID 42754 - Using SendMessage instead of PostMessage
			GetEmrEditor()->SendMessage(NXM_ADD_GENERIC_TABLE_TO_EMR, wParam, lParam);
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

////
/// UI State

void CPicContainerDlg::OnUpdateViewPic(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	if (m_pPicDocTemplate->GetProcInfoCenterView(false)) {
		pCmdUI->SetCheck(TRUE);
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

void CPicContainerDlg::OnUpdateViewHistory(CCmdUI* pCmdUI)
{
	// (j.armen 2012-07-02 15:23) - PLID 49831 - Check for Permission
	pCmdUI->Enable(CheckCurrentUserPermissions(bioPatientHistory, sptRead, FALSE, 0, TRUE, TRUE));
	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	if (m_pPicDocTemplate->GetHistoryView(false)) {
		pCmdUI->SetCheck(TRUE);
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

void CPicContainerDlg::OnUpdateViewLabs(CCmdUI* pCmdUI)
{
	// (j.armen 2012-07-02 15:23) - PLID 49831 - Check for Permission
	// (j.armen 2012-07-02 15:23) - PLID 51313 - Check for License
	if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)
		&& CheckCurrentUserPermissions(bioPatientLabs, sptRead, FALSE, 0, TRUE, TRUE)) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}

	// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
	if (m_pPicDocTemplate->GetPatientLabsView(false)) {
		pCmdUI->SetCheck(TRUE);
	} else {
		pCmdUI->SetCheck(FALSE);
	}
}

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
void CPicContainerDlg::OnUpdateStatusTotalTimeOpen(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);

	int nTotalSecondsOpen = GetEmrEditor()->GetTotalSecondsOpen();

	if (nTotalSecondsOpen != m_nLastTotalSecondsOpen) {
		m_nLastTotalSecondsOpen = nTotalSecondsOpen;

		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(0, 0, 0, nTotalSecondsOpen);

		CString strTime;
		strTime.Format("Total: %li:%02li:%02li", dtSpan.GetHours(), dtSpan.GetMinutes(), dtSpan.GetSeconds());

		pCmdUI->SetText(strTime);
	}
}

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditor is for patients
void CPicContainerDlg::OnUpdateStatusCurrentTimeOpen(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);

	int nCurrentSecondsOpen = GetEmrEditor()->GetCurrentSecondsOpen();

	if (nCurrentSecondsOpen != m_nLastCurrentSecondsOpen) {
		m_nLastCurrentSecondsOpen = nCurrentSecondsOpen;

		COleDateTimeSpan dtSpan;
		dtSpan.SetDateTimeSpan(0, 0, 0, nCurrentSecondsOpen);

		CString strTime;
		strTime.Format("Current: %li:%02li:%02li", dtSpan.GetHours(), dtSpan.GetMinutes(), dtSpan.GetSeconds());

		pCmdUI->SetText(strTime);
	}
}

// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
//void CPicContainerDlg::OnUpdateViewGraphs(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable(TRUE);
//}

// (a.walling 2012-06-11 09:30) - PLID 50913 - Organizing the ribbon
void CPicContainerDlg::OnUpdateViewEPrescribing(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);

	// (j.jones 2013-03-01 09:41) - PLID 52818 - update the ePrescribing tooltip
	//if the license is neither NewCrop nor SureScripts, change the tooltip to say "Medications",
	//otherwise just call it ePrescribing
	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		if (dynamic_ptr<CNxRibbonButton> pButton = pRibbonCmdUI->m_pUpdated) {
			if(m_eRxType == CLicense::eptNone) {
				pButton->SetDescription("Create and review prescriptions, current medications, and allergies.");
				pButton->SetToolTipText("Medications");
			}
			else {
				pButton->SetDescription("Create and review prescriptions, current medications, and allergies electronically.");
				pButton->SetToolTipText("ePrescribing");
			}
		}
	}
}

void CPicContainerDlg::OnUpdateViewMedAllergy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateViewBarcode(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
void CPicContainerDlg::OnUpdateViewDrugInteractions(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateViewEyemaginations(CCmdUI* pCmdUI)
{
	if (m_strEyePath.IsEmpty()) {
		pCmdUI->Enable(FALSE);
	} else {
		pCmdUI->Enable(TRUE);
	}
}

// (b.savon 2012-02-22 10:37) - PLID 48307 - Added Update event handler for Recall
void CPicContainerDlg::OnUpdateViewRecall(CCmdUI* pCmdUI)
{
	// (j.armen 2012-07-02 15:24) - PLID 49831 - Check permission
	// (b.savon 2012-10-30 15:32) - PLID 53477 - Fix this permission bug.  This will enable the button.
	pCmdUI->Enable((GetCurrentUserPermissions(bioRecallSystem) & (sptCreate|sptCreateWithPass)));
}

// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
void CPicContainerDlg::OnUpdateViewDeviceImport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

/// Merging
// (a.walling 2012-04-30 12:47) - PLID 49832 - Merging UI

void CPicContainerDlg::OnUpdateMergeFromPacket(CCmdUI* pCmdUI)
{
	// MergeFromPacket
	CNxRibbonAutoComboBox* pCombo = GetRibbonPacketCombo();
	if (pCombo && -1 != pCombo->GetCurSel()) {
		pCmdUI->Enable(TRUE);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CPicContainerDlg::OnUpdateMergeFromTemplate(CCmdUI* pCmdUI)
{
	// MergeFromTemplate
	CNxRibbonAutoComboBox* pCombo = GetRibbonTemplateCombo();
	if (pCombo && -1 != pCombo->GetCurSel()) {
		pCmdUI->Enable(TRUE);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CPicContainerDlg::OnUpdateEditPacket(CCmdUI* pCmdUI)
{
	// EditPacket
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateEditTemplate(CCmdUI* pCmdUI)
{
	// EditTemplate
	CNxRibbonAutoComboBox* pCombo = GetRibbonTemplateCombo();
	if (pCombo && -1 != pCombo->GetCurSel()) {
		pCmdUI->Enable(TRUE);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CPicContainerDlg::OnUpdateNewTemplate(CCmdUI* pCmdUI)
{
	// NewTemplate
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateMergePacketCombo(CCmdUI* pCmdUI)
{
	// MergePacketCombo
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateMergeTemplateCombo(CCmdUI* pCmdUI)
{
	// MergeTemplateCombo
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateMergeAdvancedSetup(CCmdUI* pCmdUI)
{
	// MergeAdvancedSetup
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateMergeRefresh(CCmdUI* pCmdUI)
{
	// MergeRefresh
	pCmdUI->Enable(TRUE);
}

void CPicContainerDlg::OnUpdateMergeOptions(CCmdUI* pCmdUI)
{
	// MergeOptions

	switch (pCmdUI->m_nID) {
		case ID_EMR_MERGE_OPTIONS_REVERSE_ORDER:
			pCmdUI->SetCheck(m_mergeOptions.reverseOrder ? TRUE : FALSE);
			break;
		case ID_EMR_MERGE_OPTIONS_SEPARATE_HISTORY:
			pCmdUI->SetCheck(m_mergeOptions.separateHistory ? TRUE : FALSE);
			break;
		case ID_EMR_MERGE_OPTIONS_SAVE_TO_HISTORY:
			pCmdUI->SetCheck(m_mergeOptions.saveToHistory ? TRUE : FALSE);
			break;
		case ID_EMR_MERGE_OPTIONS_TO_PRINTER:
			pCmdUI->SetCheck(m_mergeOptions.toPrinter ? TRUE : FALSE);
			break;
		case ID_EMR_MERGE_OPTIONS_ALL_EMNS:
			pCmdUI->SetCheck(m_mergeOptions.allEMNs ? TRUE : FALSE);
			break;
		default: 
			pCmdUI->Enable(FALSE);
			return;
	}
	
	pCmdUI->Enable(TRUE);
}

/// Single EMN Merge

// (a.walling 2012-10-01 09:15) - PLID 52119 - Handle merging from single EMN, setting default templates

void CPicContainerDlg::OnUpdateEmnMergeFromTemplate(CCmdUI* pCmdUI)
{
	// EmnMergeFromTemplate
	if (GetActiveEMN()) {
		CNxRibbonAutoComboBox* pCombo = GetRibbonEmnTemplateCombo();
		if (pCombo && -1 != pCombo->GetCurSel()) {
			pCmdUI->Enable(TRUE);
			return;
		}
	}

	pCmdUI->Enable(FALSE);
}

void CPicContainerDlg::OnUpdateEmnMergeFromOther(CCmdUI* pCmdUI)
{
	// EmnMergeFromTemplate
	if (GetActiveEMN()) {
		pCmdUI->Enable(TRUE);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CPicContainerDlg::OnUpdateEmnEditTemplate(CCmdUI* pCmdUI)
{
	// EmnEditTemplate
	CNxRibbonAutoComboBox* pCombo = GetRibbonEmnTemplateCombo();
	if (pCombo && -1 != pCombo->GetCurSel()) {
		pCmdUI->Enable(TRUE);
		return;
	}

	pCmdUI->Enable(FALSE);
}

void CPicContainerDlg::OnUpdateEmnMergeTemplateCombo(CCmdUI* pCmdUI)
{
	// EmnMergeTemplateCombo
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
		return;
	}

	pCmdUI->Enable(TRUE);

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonMergeTemplateComboBox* pList = dynamic_cast<CNxRibbonMergeTemplateComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;

	Emr::UIState& ui = m_uiState[pEMN];

	if (!ui.selectedEmnMergeTemplate) {
		// never been set
		CEMN::DefaultMergeTemplate defaultTemplate;
		try {
			defaultTemplate = pEMN->GetDefaultMergeTemplate();
		} NxCatchAllIgnore();

		ui.defaultEmnMergeTemplate = 
			FileUtils::ExtractSubPath(
				defaultTemplate.templateName
				, GetSharedPath() / "Templates"
			);
		ui.selectedEmnMergeTemplate = ui.defaultEmnMergeTemplate;
	}

	if (ui.selectedEmnMergeTemplate) {
		if (ui.selectedEmnMergeTemplate->IsEmpty()) {
			pList->SelectItem(-1);
		} else if (!pList->SelectItem(*ui.selectedEmnMergeTemplate)) {
			pList->SelectItem(pList->AddItem(*ui.selectedEmnMergeTemplate, (DWORD_PTR)0));
		}
	}

	if(ui.defaultEmnMergeTemplate) {
		pList->SetHighlightName(*ui.defaultEmnMergeTemplate);
	} else {
		pList->SetHighlightName(CString());
	}
}

void CPicContainerDlg::OnUpdateEmnMergeTemplateMakeDefault(CCmdUI* pCmdUI)
{
	// EmnMergeTemplateMakeDefault
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) {
		pCmdUI->Enable(FALSE);
		return;
	}

	{
		CNxRibbonAutoComboBox* pCombo = GetRibbonEmnTemplateCombo();
		if (!pCombo || -1 == pCombo->GetCurSel()) {
			pCmdUI->Enable(FALSE);
			return;
		}
	}

	pCmdUI->Enable(TRUE);

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonButton* pButton = dynamic_cast<CNxRibbonButton*>(pRibbonCmdUI->m_pUpdated);
	if (!pButton) return;

	CEMN::DefaultMergeTemplate defaultTemplate;
	try {
		defaultTemplate = pEMN->GetDefaultMergeTemplate();
	} NxCatchAllIgnore();

	CString strCaption;
	if(!defaultTemplate.collectionName.IsEmpty() && !defaultTemplate.procedureNames.IsEmpty()) {
		strCaption = "for " + defaultTemplate.collectionName + " - " + defaultTemplate.procedureNames;
	}
	else {
		strCaption = "for " + defaultTemplate.collectionName + defaultTemplate.procedureNames;
	}

	pButton->SetToolTipText(strCaption);
}

CNxRibbonAutoComboBox* CPicContainerDlg::GetRibbonPacketCombo()
{
	return polymorphic_downcast<CNxRibbonAutoComboBox*>(m_wndRibbonBar.FindByID(ID_EMR_MERGE_PACKET_COMBO, FALSE));
}

CNxRibbonAutoComboBox* CPicContainerDlg::GetRibbonTemplateCombo()
{
	return polymorphic_downcast<CNxRibbonAutoComboBox*>(m_wndRibbonBar.FindByID(ID_EMR_MERGE_TEMPLATE_COMBO, FALSE));
}

CNxRibbonAutoComboBox* CPicContainerDlg::GetRibbonEmnTemplateCombo()
{
	return polymorphic_downcast<CNxRibbonAutoComboBox*>(m_wndRibbonBar.FindByID(ID_EMR_EMN_MERGE_TEMPLATE_COMBO, FALSE));
}

void CPicContainerDlg::MergeOptions::Load()
{
	// seems to be the only one from properties...
	saveToHistory = !!GetRemotePropertyInt("PICSaveDocsInHistory", TRUE, 0, GetCurrentUserName(), false);
	allEMNs = !!GetPropertyInt("MergeAllEMNsInPIC", TRUE);
	
	reverseOrder = false;
	separateHistory = false;
	toPrinter = false;
}

void CPicContainerDlg::MergeOptions::Save()
{
	SetRemotePropertyInt("PICSaveDocsInHistory", saveToHistory ? TRUE : FALSE, 0, GetCurrentUserName());
	SetPropertyInt("MergeAllEMNsInPIC", allEMNs ? TRUE : FALSE);
}

static void FillTemplateList(CNxRibbonAutoComboBox* pCombo, const CString& strFolderName, const CString& strRootFolderName)
{	
	CFileFind finder;

	if (finder.FindFile(strFolderName ^ "*.*"))
	{
		while (finder.FindNextFile())
		{
			if(finder.IsDirectory() && !finder.IsDots()) {
				//Recurse
				FillTemplateList(pCombo, strFolderName ^ finder.GetFileName(), strRootFolderName);
			}
			// (a.walling 2007-06-14 16:09) - PLID 26342 - Support .dotx files
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			else if( ( (finder.GetFileName().Right(4).CompareNoCase(".dot") == 0) || (finder.GetFileName().Right(5).CompareNoCase(".dotx") == 0) 
				|| (finder.GetFileName().Right(5).CompareNoCase(".dotm") == 0))
						&& !finder.IsHidden() && !finder.IsSystem() ) {
				// (a.walling 2006-07-20 09:58) - PLID 21525 Skip over hidden and system files
				// (a.walling 2012-09-11 11:16) - PLID 52581 - flip the endian-ness of the path
				pCombo->AddItem(FileUtils::ExtractSubPath(finder.GetFilePath(), strRootFolderName), (DWORD_PTR)0);
			}
		}
		//do once more
		if(finder.IsDirectory() && !finder.IsDots()) {
			//Recurse
			FillTemplateList(pCombo, strFolderName ^ finder.GetFileName(), strRootFolderName);
		}
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		else if( ( (finder.GetFileName().Right(4).CompareNoCase(".dot") == 0) || (finder.GetFileName().Right(5).CompareNoCase(".dotx") == 0) 
				|| (finder.GetFileName().Right(5).CompareNoCase(".dotm") == 0))
					&& !finder.IsHidden() && !finder.IsSystem() ) {

			// (a.walling 2012-09-11 11:16) - PLID 52581 - flip the endian-ness of the path
			pCombo->AddItem(FileUtils::ExtractSubPath(finder.GetFilePath(), strRootFolderName), (DWORD_PTR)0);
		}
	} 
}

// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
void CPicContainerDlg::SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement)
{
	switch (pElement->GetID()) {
		case ID_EMR_MERGE_PACKET_COMBO:
			//if (!m_bLoadedMergePacketCombo) {
			if (dynamic_ptr<CNxRibbonAutoComboBox> pCombo = pElement)
			{
				try {
					ADODB::_RecordsetPtr prs = CreateRecordsetStd("SELECT ID, Name FROM PacketsT WHERE Deleted = 0 ORDER BY Name");

					DWORD_PTR nExisting = 0;
					if (-1 != pCombo->GetCurSel()) {
						nExisting = pCombo->GetItemData(pCombo->GetCurSel());
					}

					pCombo->RemoveAllItems();

					while (!prs->eof) {
						pCombo->AddItem(AdoFldString(prs, "Name", ""), (DWORD_PTR)AdoFldLong(prs, "ID"));
						prs->MoveNext();
					}

					if (nExisting) {
						pCombo->SelectItem(nExisting);
					}

					m_bLoadedMergePacketCombo = true;
				} NxCatchAll(__FUNCTION__ " - Updating packet list");
			}
			return;
		case ID_EMR_MERGE_TEMPLATE_COMBO:
			//if (m_bLoadedMergeTemplateCombo) {
			if (dynamic_ptr<CNxRibbonAutoComboBox> pCombo = pElement) {
				try {
					CString strExisting;
					if (-1 != pCombo->GetCurSel()) {
						strExisting = pCombo->GetItem(pCombo->GetCurSel());
					}

					pCombo->RemoveAllItems();

					// (a.walling 2012-08-14 17:31) - PLID 52120 - Include letters, add Templates to the root path
					FillTemplateList(pCombo, GetSharedPath() / "Templates\\Forms", GetSharedPath() / "Templates");
					FillTemplateList(pCombo, GetSharedPath() / "Templates\\Letters", GetSharedPath() / "Templates");

					if (!strExisting.IsEmpty()) {
						pCombo->SelectItem(strExisting);
					}

					m_bLoadedMergeTemplateCombo = true;
				} NxCatchAll(__FUNCTION__ " - Updating template list");
			}
			return;
		case ID_EMR_EMN_MERGE_TEMPLATE_COMBO:
			// (a.walling 2012-10-01 09:15) - PLID 52119 - Load up templates for this emn
			//if (m_bLoadedMergeTemplateCombo) {
			if (dynamic_ptr<CNxRibbonMergeTemplateComboBox> pCombo = pElement) {
				try {
					CString strExisting;
					if (-1 != pCombo->GetCurSel()) {
						strExisting = pCombo->GetItem(pCombo->GetCurSel());
					}

					pCombo->RemoveAllItems();

					// (a.walling 2012-08-14 17:31) - PLID 52120 - Include letters, add Templates to the root path
					FillTemplateList(pCombo, GetSharedPath() / "Templates\\Forms\\EMR", GetSharedPath() / "Templates");

					if (!strExisting.IsEmpty()) {
						pCombo->SelectItem(strExisting);
					}

					m_bLoadedMergeTemplateCombo = true;
				} NxCatchAll(__FUNCTION__ " - Updating template list");
			}
			return;
	}

	__super::SynchronizeDelayedRibbonSubitems(pElement);
}


////
/// UI Commands

void CPicContainerDlg::OnViewPic()
{
	try {
		// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
		CView* pView = m_pPicDocTemplate->GetProcInfoCenterView(false);

		if (pView) {
			pView->GetParentFrame()->SendMessage(WM_CLOSE);
		} else {
			InitializePic();
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnViewHistory()
{
	try {
		// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
		CView* pView = m_pPicDocTemplate->GetHistoryView(false);

		if (pView) {
			pView->GetParentFrame()->SendMessage(WM_CLOSE);
		} else {
			InitializeHistory();
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnViewLabs()
{
	try {
		// (a.walling 2012-11-09 08:45) - PLID 53671 - Views are now created / accessed via the doctemplate
		CView* pView = m_pPicDocTemplate->GetPatientLabsView(false);

		if (pView) {
			pView->GetParentFrame()->SendMessage(WM_CLOSE);
		} else {
			InitializeLabs();
		}
	} NxCatchAll(__FUNCTION__);
}


/// Merging
// (a.walling 2012-04-30 13:38) - PLID 50072 - Merging implementation

void CPicContainerDlg::OnMergeFromPacket()
{	
	try {
		// MergeFromPacket
		if (m_mergeOptions.toPrinter) {
			OnMergeFromPacketToPrinter();
		} else {
			OnMergeFromPacketToWord();
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnMergeFromPacketToWord()
{	
	try {
		// MergeFromPacketToWord
		MergeOptions mergeOptionsOriginal = m_mergeOptions;

		m_mergeOptions.toPrinter = false;
		try {
			DoMergeFromPacket();
		} NxCatchAll(__FUNCTION__);

		m_mergeOptions = mergeOptionsOriginal;
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnMergeFromPacketToPrinter()
{	
	try {
		// MergeFromPacketToPrinter
		MergeOptions mergeOptionsOriginal = m_mergeOptions;

		m_mergeOptions.toPrinter = true;
		try {
			DoMergeFromPacket();
		} NxCatchAll(__FUNCTION__);

		m_mergeOptions = mergeOptionsOriginal;
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnMergeFromTemplate()
{	
	try {
		// MergeFromTemplate
		if (m_mergeOptions.toPrinter) {
			OnMergeFromTemplateToPrinter();
		} else {
			OnMergeFromTemplateToWord();
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnMergeFromTemplateToWord()
{	
	try {
		// MergeFromTemplateToPrinter
		MergeOptions mergeOptionsOriginal = m_mergeOptions;

		m_mergeOptions.toPrinter = false;
		try {
			DoMergeFromTemplate();
		} NxCatchAll(__FUNCTION__);

		m_mergeOptions = mergeOptionsOriginal;
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnMergeFromTemplateToPrinter()
{	
	try {
		// MergeFromTemplateToPrinter
		MergeOptions mergeOptionsOriginal = m_mergeOptions;

		m_mergeOptions.toPrinter = true;
		try {
			DoMergeFromTemplate();
		} NxCatchAll(__FUNCTION__);

		m_mergeOptions = mergeOptionsOriginal;
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEditPacket()
{	
	try {
		// EditPacket

		CConfigPacketsDlg dlg(this);
		dlg.m_bShowProcSpecific = 1;
		dlg.DoModal();

		// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
		SynchronizeDelayedRibbonSubitems(GetRibbonPacketCombo());
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEditTemplate()
{	
	try {
		// EditTemplate
		
		// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo

		//DRT 3/17/03 - Check for permission first!
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		CNxRibbonAutoComboBox* pCombo = GetRibbonTemplateCombo();
		if (!pCombo || -1 == pCombo->GetCurSel()) {		
			MessageBox("Please select a template to edit.");
			return;
		}

		CWaitCursor wait;

		// (a.walling 2012-08-14 17:31) - PLID 52120 - add Templates to the root path
		// (a.walling 2012-09-11 11:16) - PLID 52581 - flip the endian-ness of the path
		CString strTemplateName = GetSharedPath() / "Templates" / pCombo->GetItem(pCombo->GetCurSel());

		CString strMergeInfoFilePath;

		try {
			// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
			std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
			if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown
			pApp->EnsureValid();

			// Create an empty MergeInfo.nxt
			long nFlags = BMS_HIDE_ALL_DATA	| BMS_DEFAULT | /*BMS_HIDE_PRACTICE_INFO |*/
						/*BMS_HIDE_PERSON_INFO | BMS_HIDE_DATE_INFO | BMS_HIDE_PRESCRIPTION_INFO |*/
						/*BMS_HIDE_CUSTOM_INFO | BMS_HIDE_INSURANCE_INFO |*/ BMS_HIDE_BILL_INFO /*|*/
						/*BMS_HIDE_PROCEDURE_INFO | BMS_HIDE_DOCTOR_INFO*/;

			// Generate the extra fields
			CMergeEngine infDummy;
			CEMR_ExtraMergeFields emf;
			CString strHeadersLoc;
			CString strDataLoc;

			// (c.haag 2004-10-01 15:49) - PLID 14242 - Add EMN IDs to the merge
			_RecordsetPtr prsEMNs = CreateRecordset("SELECT ID, Description FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = %d", m_nEmrGroupID);
			while (!prsEMNs->eof)
			{
				infDummy.m_arydwEMRIDs.Add(AdoFldLong(prsEMNs, "ID"));
				prsEMNs->MoveNext();
			}
			prsEMNs->Close();

			// (c.haag 2004-10-05 17:07) - PLID 14242 - Fill the merge engine object with information
			// (z.manning 2010-01-13 17:57) - PLID 36864 - Removed an unnecessary parameter
			if (!GenerateMergeData(infDummy, strHeadersLoc, strDataLoc, TRUE))
				return;
			emf.m_strHeaders = strHeadersLoc;
			emf.m_strData = strDataLoc;

			strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, CEMR_PIC__ExtraMergeFields, &emf);

			if (!strMergeInfoFilePath.IsEmpty()) {

				// Open the template
				// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
				// (c.haag 2016-04-22 10:40) - NX-100275 - OpenTemplate no longer returns a document. We never did anything with it except throw an exception if it were null
				// anyway, and now OpenTemplate does that for us
				pApp->OpenTemplate(strTemplateName, strMergeInfoFilePath);

				// We can't delete the merge info text file right now because it is in use, but 
				// it's a temp file so mark it to be deleted after the next reboot
				DeleteFileWhenPossible(strMergeInfoFilePath);
				strMergeInfoFilePath.Empty();
			} else {
				AfxThrowNxException("Could not create blank merge info");
			}
			// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
		}NxCatchAll("CPicContainerDlg::OnEditTemplate 1");

		if (!strMergeInfoFilePath.IsEmpty()) {
			// This means the file wasn't used and/or it wasn't 
			// marked for deletion at startup, so delete it now
			DeleteFile(strMergeInfoFilePath);
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnNewTemplate()
{	
	try {
		// NewTemplate
		
		// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
		
		//DRT 3/17/03 - Check for permission first!
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		// User just wants a new blank template
		DoCreateNewTemplate("");
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnCopyTemplate()
{
	try {
		// NewTemplate
		
		// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
		
		//DRT 3/17/03 - Check for permission first!
		if (!CheckCurrentUserPermissions(bioLWEditTemplate, sptView))
			return;

		if (!GetWPManager()->CheckWordProcessorInstalled()) {
			return;
		}

		CString strBaseTemplate;
		// Get the path to the template on which to base this new template
		// User wants to browse
		if (!BrowseForTemplate(GetSafeHwnd(), strBaseTemplate)) {
			// The user canceled the browse
			return;
		}

		DoCreateNewTemplate(strBaseTemplate);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::DoCreateNewTemplate(const CString& strBaseTemplate)
{
	//Path for the MergeInfo file (will be set below)
	CString strMergeInfoFilePath;

	try { 
		CWaitCursor wc;
		// (z.manning 2016-02-12 13:57) - PLID 68230 - Use the base word processor app class
		std::shared_ptr<CGenericWordProcessorApp> pApp = GetWPManager()->GetAppInstance();
		if (nullptr == pApp) return; // (c.haag 2016-06-01 11:48) - NX-100320 - If it's null then it's not supported and an exception was not thrown

		// Create an empty MergeInfo.nxt
		long nFlags = BMS_HIDE_ALL_DATA	| BMS_DEFAULT | BMS_HIDE_BILL_INFO;
							
		// Generate the extra fields
		CMergeEngine infDummy;
		CEMR_ExtraMergeFields emf;
		CString strHeadersLoc, strDataLoc;

		// (c.haag 2004-10-01 15:49) - PLID 14242 - Add EMN IDs to the merge
		_RecordsetPtr prsEMNs = CreateRecordset("SELECT ID, Description FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = %d", m_nEmrGroupID);
		while (!prsEMNs->eof)
		{
			infDummy.m_arydwEMRIDs.Add(AdoFldLong(prsEMNs, "ID"));
			prsEMNs->MoveNext();
		}
		prsEMNs->Close();

		// (c.haag 2004-10-05 17:07) - PLID 14242 - Fill the merge engine object with information
		// (z.manning 2010-01-13 17:57) - PLID 36864 - Removed an unnecessary parameter
		if (!GenerateMergeData(infDummy, strHeadersLoc, strDataLoc, TRUE))
			return;
		emf.m_strHeaders = strHeadersLoc;
		emf.m_strData = strDataLoc;

		strMergeInfoFilePath = CMergeEngine::CreateBlankMergeInfo(nFlags, CEMR_PIC__ExtraMergeFields, &emf);
				
		if (!strMergeInfoFilePath.IsEmpty()) {

			// Open the template
			// (c.haag 2016-01-20) - PLID 68172 - Use the CWordDocument object to perform the business logic
			// (z.manning 2016-02-12 13:49) - PLID 68239 - Use the base document class
			// (z.manning 2016-02-18 09:18) - PLID 68366 - This now returns a word processor document object
			// (c.haag 2016-04-22 10:51) - NX-100275 - The merge info assignment is now done in CreateTemplate
			pApp->CreateTemplate(strBaseTemplate, strMergeInfoFilePath);

			// We can't delete the merge info text file right now because it is in use, but 
			// it's a temp file so mark it to be deleted after the next reboot
			DeleteFileWhenPossible(strMergeInfoFilePath);
			strMergeInfoFilePath.Empty();
		} else {
			MessageBox("DoCreateNewTemplate Error 50\n\nCould not create template");
		}
		// (c.haag 2016-02-23) - PLID 68416 - We no longer catch Word-specific exceptions here. Those are now managed deep within the WordProcessor application object
	} NxCatchAll("DoCreateNewTemplate Error 200");

	if (!strMergeInfoFilePath.IsEmpty()) {
		// This means the file wasn't used and/or it wasn't 
		// marked for deletion at startup, so delete it now
		DeleteFile(strMergeInfoFilePath);
	}
}

// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
void CPicContainerDlg::OnMergePacketCombo()
{	
	// MergePacketCombo
}

// (a.walling 2012-04-30 14:33) - PLID 50076 - Packet and template combo
void CPicContainerDlg::OnMergeTemplateCombo()
{	
	// MergeTemplateCombo
}

void CPicContainerDlg::OnMergeAdvancedSetup()
{	
	try {
		// MergeAdvancedSetup
		CEMRMergePrecedenceDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnMergeRefresh()
{	
	// MergeRefresh
}

void CPicContainerDlg::OnMergeOptions(UINT nID)
{	
	try {
		// MergeOptions

		switch (nID) {
			case ID_EMR_MERGE_OPTIONS_REVERSE_ORDER:
				m_mergeOptions.reverseOrder = !m_mergeOptions.reverseOrder;
				break;
			case ID_EMR_MERGE_OPTIONS_SEPARATE_HISTORY:
				m_mergeOptions.separateHistory = !m_mergeOptions.separateHistory;
				break;
			case ID_EMR_MERGE_OPTIONS_SAVE_TO_HISTORY:
				m_mergeOptions.saveToHistory = !m_mergeOptions.saveToHistory;			
				m_mergeOptions.Save();
				break;
			case ID_EMR_MERGE_OPTIONS_TO_PRINTER:
				m_mergeOptions.toPrinter = !m_mergeOptions.toPrinter;
				break;
			case ID_EMR_MERGE_OPTIONS_ALL_EMNS:
				m_mergeOptions.allEMNs = !m_mergeOptions.allEMNs;
				break;
			default: return;
		}
	} NxCatchAll(__FUNCTION__);
}


/// Single EMN Merge

// (a.walling 2012-10-01 09:15) - PLID 52119 - Handle merging from single EMN, setting default templates

void CPicContainerDlg::OnEmnMergeFromTemplate()
{	
	try {
		// EmnMergeFromTemplate
		if (m_mergeOptions.toPrinter) {
			OnEmnMergeFromTemplateToPrinter();
		} else {
			OnEmnMergeFromTemplateToWord();
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnMergeFromTemplateToWord()
{	
	try {
		// EmnMergeFromTemplateToPrinter
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		Emr::UIState& ui = m_uiState[pEMN];

		CString strTemplate;
		if (ui.selectedEmnMergeTemplate) {
			strTemplate = GetSharedPath() / "Templates" / *ui.selectedEmnMergeTemplate;
		}

		DoEmnMergeFromTemplate(strTemplate, m_mergeOptions.saveToHistory, false);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnMergeFromTemplateToPrinter()
{	
	try {
		// EmnMergeFromTemplateToPrinter
		CEMN* pEMN = GetActiveEMN();
		if (!pEMN) return;

		Emr::UIState& ui = m_uiState[pEMN];

		CString strTemplate;
		if (ui.selectedEmnMergeTemplate) {
			strTemplate = GetSharedPath() / "Templates" / *ui.selectedEmnMergeTemplate;
		}

		DoEmnMergeFromTemplate(strTemplate, m_mergeOptions.saveToHistory, true);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnMergeFromOther()
{	
	try {
		// EmnMergeFromOther
		if (m_mergeOptions.toPrinter) {
			OnEmnMergeFromOtherToPrinter();
		} else {
			OnEmnMergeFromOtherToWord();
		}
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnMergeFromOtherToWord()
{	
	try {
		// EmnMergeFromOtherToPrinter
		DoEmnMergeFromTemplate("", m_mergeOptions.saveToHistory, false);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnMergeFromOtherToPrinter()
{	
	try {
		// EmnMergeFromOtherToPrinter
		DoEmnMergeFromTemplate("", m_mergeOptions.saveToHistory, true);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::DoEmnMergeFromTemplate(CString strTemplateFileName, bool bSaveInHistory, bool bDirectToPrinter)
{
	try {
		CEMN* pEMN = GetActiveEMN();

		if (!pEMN) return;

		// (a.walling 2007-06-25 16:44) - PLID 15177 - Check for Word.
		if(!GetWPManager()->CheckWordProcessorInstalled())
			return;

		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(!pTreeWnd) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		// (c.haag 2007-08-27 09:34) - PLID 27185 - Ensure that the EMN's initial load is done
		pEMN->EnsureCompletelyLoaded();

		// (a.walling 2008-06-12 13:21) - PLID 27301 - Check for duplicate merge names before we save
		if (!pEMN->WarnOfDuplicateMergeNames()) {
			// Returns FALSE if the user decided not to merge the chart
			return;
		}

		//(j.jones 2006-06-27 09:37) - Make sure the EMN is saved.
		//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we've already checked IsMoreInfoUnsaved()
		if(pEMN->IsUnsaved() || pEMN->GetID() == -1) {
			if(IDYES != MsgBox(MB_YESNO, "Before merging, the changes you have made to the EMN must be saved.  Would you like to continue?")) {
				return;
			}

			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			if(FAILED(pTreeWnd->SaveEMR(esotEMN, (long)pEMN, TRUE))) {
				AfxMessageBox("The EMN was not saved. The merge will now be cancelled.");
				return;
			}
		}

		if (strTemplateFileName.IsEmpty()) {
			// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
			// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
			CString strFilter;
			// Always support Word 2007 templates
			strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

			CFileDialog dlg(TRUE, "dot", NULL, OFN_FILEMUSTEXIST|OFN_SHAREAWARE|OFN_EXPLORER|OFN_ENABLESIZING|OFN_HIDEREADONLY, strFilter, this);
			CString strInitPath = GetTemplatePath(); // We need to store this because the next line is a pointer to it
			dlg.m_ofn.lpstrInitialDir = strInitPath;
			dlg.m_ofn.lpstrTitle = "Select a template for this merge";
			if (dlg.DoModal() == IDOK) {
				// User clicked ok so get the path to the file he selected and proceed
				strTemplateFileName = dlg.GetPathName();
			} else {
				// User canceled
				return;
			}
		}

		// (c.haag 2009-08-10 11:17) - PLID 29160 - Added "Merge to printer" option
		pEMN->Merge(strTemplateFileName, bSaveInHistory, bDirectToPrinter);

		//refresh the EMN to show the Word icon
		CClient::RefreshTable(NetUtils::EMRMasterT, pEMN->GetParentEMR()->GetPatientID());
		
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnEditTemplate()
{	
	try {
		// EmnEditTemplate

		CEMN* pEMN = GetActiveEMN();
		CNxRibbonAutoComboBox* pList = GetRibbonEmnTemplateCombo();

		if (!pEMN || !pList) return;

		int nCurSel = pList->GetCurSel();
		
		CString strFile, strInitDir;
		if (-1 != nCurSel) {
			CString strTemplate = pList->GetItem(nCurSel);

			if (strTemplate.IsEmpty()) {
				strInitDir = GetTemplatePath();
			} else {
				strTemplate = GetSharedPath() / "Templates" / strTemplate;

				strInitDir = FileUtils::GetFilePath(strTemplate);
				strFile = FileUtils::GetFileName(strTemplate);
			}
		} else {
			strInitDir = GetTemplatePath();
		}

		pEMN->EditWordTemplates(GetSafeHwnd(), strFile, strInitDir);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnEmnMergeTemplateCombo()
{	
	// EmnMergeTemplateCombo	
	
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_EMN_MERGE_TEMPLATE_COMBO);
	if (!pList) return;

	Emr::UIState& ui = m_uiState[pEMN];

	long nCurSel = pList->GetCurSel();

	if (-1 != nCurSel) {
		ui.selectedEmnMergeTemplate = pList->GetItem(nCurSel);
	} else {
		ui.selectedEmnMergeTemplate = CString();
	}
}

void CPicContainerDlg::OnEmnMergeTemplateMakeDefault()
{	
	// EmnMergeTemplateMakeDefault
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	CNxRibbonAutoComboBox* pCombo = GetRibbonEmnTemplateCombo();
	if (!pCombo) return;

	int nCurSel = pCombo->GetCurSel();
	if (-1 == nCurSel) return;

	CString strItem = pCombo->GetItem(nCurSel);
	CString strTemplate = strItem;

	m_uiState[pEMN].defaultEmnMergeTemplate = strItem;
	ASSERT(m_uiState[pEMN].defaultEmnMergeTemplate == m_uiState[pEMN].selectedEmnMergeTemplate);
	
	pEMN->SetDefaultMergeTemplate(strTemplate);
}


// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
// (d.thompson 2011-03-22) - PLID 42928 - Button to access the "graph emr" dialog
// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
//void CPicContainerDlg::OnViewGraphs()
//{
//	try {
//		CPatientGraphConfigDlg dlg(this);
//		if(dlg.DoModal() != IDCANCEL) {
//			//They previewed.  We should minimize this EMR so the report is not behind it.
//			if(GetTopLevelParent()) {
//				GetTopLevelParent()->ShowWindow(SW_MINIMIZE);
//			}
//		}
//	} 
//	NxCatchAll(__FUNCTION__);
//}

// (a.walling 2012-06-11 09:30) - PLID 50913 - Organizing the ribbon
void CPicContainerDlg::OnViewEPrescribing()
{
	try {
		GetEmrTreeWnd()->OnEPrescribing();
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnViewMedAllergy()
{
	try {
		BOOL bShowExplanation = (NULL == m_pMedAllergyViewerDlg) ? TRUE : FALSE;
		ShowMedAllergyViewer(bShowExplanation);
	} NxCatchAll(__FUNCTION__);
}

void CPicContainerDlg::OnViewBarcode()
{
	try {
		// (j.dinatale 2011-09-20 10:26) - PLID 44702 - need to save the EMR before we show the barcode, since if its a new emr, we dont have an ID
		if(GetEmrEditor()->IsEMRUnsaved()){
			GetEmrEditor()->Save();
		}

		if (!m_pBarcodeDlg) {
			m_pBarcodeDlg.reset(new CEMRBarcodeDlg(this));
		}

		m_pBarcodeDlg->SetInfo(GetEmrEditor()->GetPatientID(), GetEmrEditor()->GetEMR()->GetID());
		m_pBarcodeDlg->ShowWindow(SW_SHOW);
	} NxCatchAll(__FUNCTION__);
} 

// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
void CPicContainerDlg::OnViewDrugInteractions()
{
	try {

		// (j.jones 2009-03-24 14:44) - PLID 33652 - supported the NewCrop license
		// (j.fouts 2013-05-30 09:47) - PLID 56807 - Drug Interactions is now tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptSureScripts) {
			return;
		}

		//forcibly save the EMR before showing interactions, since the EMN
		//may have new content that would affect the interactions
		if(GetEmrEditor()->IsEMRUnsaved()){
			GetEmrEditor()->Save();
		}

		//sending true will show the dialog even if it has no interactions in it
		ViewDrugInteractions(true, true);

	} NxCatchAll(__FUNCTION__);
} 

// (j.jones 2012-09-26 14:04) - PLID 52879 - added drug interactions
// (j.jones 2012-11-30 11:09) - PLID 53194 - added bShowEvenIfUnchanged, which
// will show the dialog even if its current interactions have not changed since the last popup
void CPicContainerDlg::ViewDrugInteractions(bool bShowEvenWhenEmpty /*= false*/, bool bShowEvenIfUnchanged /*= false*/)
{
	try {

		//this function will silently do nothing if FDB is not licensed,
		//if a Use license is needed, it's the responsibility of the caller to do so

		// (j.fouts 2013-05-30 09:47) - PLID 56807 - Drug Interactions is now tied to NexERx
		if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts) 
		{
			if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
				//Also check that the database exists
				if(!FirstDataBank::EnsureDatabase(this, true))
				{
					//We decided not to have an additional warning here about drug interactions not
					//being check because if FDB is not built yet then there should really be no way
					//that they have FDB drugs/allergies in their database yet.
					return;
				}
			}

			if (!m_pDrugInteractionDlg) {
				// (b.savon 2012-11-30 10:30) - PLID 53773 - Pass in the parent enum
				m_pDrugInteractionDlg.reset(new CDrugInteractionDlg(this, eipEMR));
			}

			if (!IsWindow(m_pDrugInteractionDlg->GetSafeHwnd()))
			{
				m_pDrugInteractionDlg->Create(IDD_DRUG_INTERACTION_DLG, this);
			}

			m_pDrugInteractionDlg->ShowOnInteraction(m_nPatientID, true, bShowEvenWhenEmpty, bShowEvenIfUnchanged);
		}

	}NxCatchAll(__FUNCTION__);
}

//(a.wilson 2011-10-17) PLID 45971 - when clicked, this will launch the LUMA Software.
void CPicContainerDlg::OnViewEyemaginations()
{
	try {
		//(a.wilson 2011-12-1) PLID 46335 - first we need to check whether eyemaginations is installed 
		//which will also decide whether they have a license or not from OnInitDialog().
		if (!m_strEyePath.IsEmpty()) {
			//next we need to check to see if they have any number of uses of the license.
			if (g_pLicense->CheckForLicense(CLicense::lcEyemaginations, CLicense::cflrUse)) {

				BOOL bXmlGenerated = GenerateLumaXml();
				CString strParameter = FileUtils::GetLocalAppDataFolderPath(true) ^ "\\Nextech\\Eyemaginations\\LumaRequest.xml";
				CString strDirectory = m_strEyePath ^ "manager\\";

				if (PathFileExists(strParameter) && bXmlGenerated == TRUE) {
					strParameter = "--emr \"" + strParameter + "\"";
				} else {
					strParameter.Empty();
				}
				// Create the shell-execute info object
				SHELLEXECUTEINFO sei;
				memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.hwnd =(HWND)GetDesktopWindow();
				sei.lpFile = "LumaManager.exe";
				sei.lpDirectory = strDirectory;
				sei.lpParameters = strParameter;
				sei.nShow = SW_SHOW;
				sei.hInstApp = NULL;

				// Run ShellExecute
				ShellExecuteEx(&sei);
			}
		} else {
			//this will happen if they have a license but eyemaginations is not installed on the workstation.
			MessageBox("Eyemaginations LUMA is not installed on this computer.", "Nextech Practice", MB_OK);
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-02-22 10:38) - PLID 48307 - Added Event Handler for toolbar button
void CPicContainerDlg::OnViewRecall()
{
	try {
		//(a.wilson 2012-3-23) PLID 48472 - checking whether the current user has create permission.
		BOOL bCreatePerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreate));
		BOOL bCreatePermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreateWithPass));

		if (!bCreatePerm && !bCreatePermWithPass) {
			return;
		} else if (!bCreatePerm && bCreatePermWithPass) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		// (b.savon 2012-02-28 16:56) - PLID 48442 - Create recall from EMR
		//Save first incase they entered a DiagCode in more info without saving.
		GetEmrEditor()->Save();

		//Fill in necessary fields to begin a patient recall
		CCreatePatientRecall::PatientRecall prRecall;
		prRecall.nEmrGroupID = GetEmrEditor()->GetEmrID();
		prRecall.nPatientID = GetEmrEditor()->GetPatientID(); 

		// (j.jones 2016-02-22 09:47) - PLID 68348 - because the EMR is saved,
		// existing EMR calculation logic will later pull in the default provider
		// and location for the recall

		CCreatePatientRecall prDlg(prRecall, this);
		prDlg.DoModal();
		
	} NxCatchAll(__FUNCTION__);
} 

// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
void CPicContainerDlg::OnViewDeviceImport()
{
	try {
		GetEmrEditor()->PostMessage(WM_COMMAND, MAKEWPARAM(IDC_BTN_DEVICE_INFO, BN_CLICKED), 0);
	} NxCatchAll(__FUNCTION__);
}

//(a.wilson 2011-10-17) PLID 45971 - Generates the xml file to enter the media list screen.
BOOL CPicContainerDlg::GenerateLumaXml()
{
	using namespace FileUtils;
	try {
		//if it already exists then do ahead and use it otherwise recreate it.
		if (DoesFileOrDirExist(GetLocalAppDataFolderPath(true) ^ "\\Nextech\\Eyemaginations\\LumaRequest.xml")) {
			return TRUE;
		}

		//ensure the path exists, if not create it.
		if (!EnsureDirectory(GetLocalAppDataFolderPath(true) ^ "\\Nextech\\Eyemaginations")) {
			return FALSE;
		}

		//generate the ID for the request.
		CString strRequestID = "";
		for (int i = 0; i < 15; i++) {
			long nDigit = (rand() % 36);
			CString str = "";
			if (nDigit > 9) { //if higher than one digit then make it an alphabet letter.
				char chr = (char)(nDigit + 55); //10 - 35 => 65 - 90, which is A - Z in ASCII
				str = chr;
			} else {
				str.Format("%li", nDigit);
			}
			strRequestID +=  str;
		}
		//create the xml file
		CFile xmlFile(GetLocalAppDataFolderPath(true) ^ "\\Nextech\\Eyemaginations\\LumaRequest.xml", 
			CFile::modeCreate | CFile::modeReadWrite | CFile::shareCompat);
		CString strXml = "<emr> \r\n"
			"\t<request> \r\n"
			"\t\t<command name=\"load\" type=\"all\" keyword=\"\" id=\"" + strRequestID + "\"> \r\n"
			"\t\t\t<params> \r\n"
			"\t\t\t\t<param>assetID</param> \r\n"
			"\t\t\t</params> \r\n"
			"\t\t</command> \r\n"
			"\t</request> \r\n"
			"</emr>";
		//write it to the path and close the file
		xmlFile.Write(strXml, strXml.GetLength());
		xmlFile.Close();

	} NxCatchAllCall(__FUNCTION__, { return FALSE; });

	return TRUE;
}

// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
// (c.haag 2010-08-03 17:14) - PLID 38928 - Display the medication / allergy viewer
void CPicContainerDlg::ShowMedAllergyViewer(BOOL bShowExplanation)
{
	if (!m_pMedAllergyViewerDlg) {
		m_pMedAllergyViewerDlg.reset(new CEmrMedAllergyViewerDlg(this));
		m_pMedAllergyViewerDlg->SetPatientID(GetEmrEditor()->GetPatientID());
		m_pMedAllergyViewerDlg->SetPatientName(GetEmrEditor()->GetPatientName());
	}

	if (!IsWindow(m_pMedAllergyViewerDlg->GetSafeHwnd()))
	{
		m_pMedAllergyViewerDlg->Create(IDD_EMR_MED_ALLERGY_VIEWER_DLG, this);
		m_pMedAllergyViewerDlg->RestoreSize();
	}

	if (!m_pMedAllergyViewerDlg->IsWindowVisible()) 
	{
		// Requery the viewer
		m_pMedAllergyViewerDlg->Requery();
	}

	// Bring the window to the foreground
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	m_pMedAllergyViewerDlg->ShowWindow(SW_SHOW);
	if (m_pMedAllergyViewerDlg->GetWindowPlacement(&wp)) {
		if (m_pMedAllergyViewerDlg->IsIconic()) {
			if (wp.flags & WPF_RESTORETOMAXIMIZED) {
				wp.showCmd = SW_MAXIMIZE;
			} else {
				wp.showCmd = SW_RESTORE;
			}
			m_pMedAllergyViewerDlg->SetWindowPlacement(&wp);
		}
	}

	// Bring the window to the foreground so the user is sure to see it
	m_pMedAllergyViewerDlg->SetForegroundWindow();

	if (bShowExplanation) {
		// Explain the purpose of this window to the user. This happens once per unique EMR.
		DontShowMeAgain(m_pMedAllergyViewerDlg.get(), 
			"For your convenience, the 'Saved Current Medication / Allergy / Rx List' displays the current medications, allergies and prescriptions corresponding to this EMR's patient.\n\n"
			"This feature only reflects values that are saved in data. If you change patient information from this EMR, this display will not be updated with your changes until you save the EMR.\n\n"
			, "Practice");
	}
}

void CPicContainerDlg::InitializeRibbonTabButtons()
{
	// (j.armen 2012-07-02 15:24) - PLID 51313 - We only use the ribbon tab if we have EMR
	if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) != 2)
		return;

	// (a.walling 2011-12-09 17:16) - PLID 46643 - Tool buttons and UI components (time, etc)
	// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
	// (a.walling 2014-08-27 08:34) - PLID 63488 - Ensure the buttons clean up their icons; this fixes a GDI leak
	
	// (a.walling 2012-06-11 09:30) - PLID 50913 - Organizing the ribbon
	m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_EPRESCRIBING, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::eRx), FALSE, NULL, TRUE));
	// (j.jones 2012-09-26 14:18) - PLID 52879 - added drug interaction
	// (j.fouts 2013-05-30 10:21) - PLID 56807 - Drug Interactions is now tied to NexERx
	if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts)
	{
		m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_DRUGINTERACTIONS, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::DrugInteraction), FALSE, NULL, TRUE));
	}
	m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_MEDALLERGY, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::Caduceus), FALSE, NULL, TRUE));

	// (b.savon 2012-02-22 09:51) - PLID 48307 - Added Recall button
	//(a.wilson 2012-3-23) PLID 48472 - check if current user can create recalls and remove if they can't.
	// (j.armen 2012-03-28 09:34) - PLID 48480 - Check for license
	if (g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
		m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_RECALL, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::Recall), FALSE, NULL, TRUE));
	}
	// (a.wilson 2012-4-30) PLID 49479 - removing the graph button because it is no longer necessary.
	//m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_GRAPHS, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::Graphs)));
	m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_CAMERABARCODE, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::Camera), FALSE, NULL, TRUE));

	if (!m_strEyePath.IsEmpty()) {
		m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_EYEMAGINATIONS, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::EyeMaginations), FALSE, NULL, TRUE));
	}

	// (a.walling 2012-04-02 16:51) - PLID 49360 - Device Import handling
	if(g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
		m_wndRibbonBar.AddToTabs(new CNxRibbonButton(ID_EMR_VIEW_DEVICE_IMPORT, "", m_ribbonToolImages.ExtractIcon(EmrIcons::Tools::DeviceImport), FALSE, NULL, TRUE));
	}

	CEmrPatientFrameWnd::InitializeRibbonTabButtons();
}

// (j.dinatale 2012-01-18 17:31) - PLID 47539 - can now do partial billing
LRESULT CPicContainerDlg::OnSaveCreatePartialEMNBills(WPARAM wParam, LPARAM lParam)
{
	try {
		SaveAndClose(cpaDoPartialBilling, (CEMN*)wParam, (long)lParam);
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (z.manning 2010-01-13 18:04) - PLID 36864 - Added bPacket parameter
BOOL CPicContainerDlg::BuildMergeEngineObjects(CEMR_ExtraMergeFields& emf, CMergeEngine& mi, BOOL bPacket)
{
	// (b.cardillo 2005-06-29 08:57) - PLID 16577 - Check the preference to see if we should 
	// save the document in the patient's history or not.
	if (m_mergeOptions.saveToHistory) {
		// (b.cardillo 2004-06-21 09:48) - PLID 12909 - Make sure it saves itself into this patient's history tab.
		mi.m_nFlags |= BMS_SAVE_FILE_AND_HISTORY;
	}

	// (c.haag 2004-10-01 15:42) - PLID 14242 - Figure out which category (if any) this collection connects to.
	// This is a function fo template name.
	// (z.manning 2010-01-13 18:05) - PLID 36864 - We should not be pulling category info when merging a packet
	// as the selected template is completely arbitrary in that case.
	if(!bPacket) {

		CString strTemplateName;

		CNxRibbonAutoComboBox* pCombo = GetRibbonTemplateCombo();

		if (!pCombo || -1 == pCombo->GetCurSel()) {
		} else {
			// (a.walling 2012-08-14 17:31) - PLID 52120 - add Templates to the root path
			// (a.walling 2012-09-11 11:16) - PLID 52581 - flip the endian-ness of the path
			strTemplateName = GetSharedPath() / "Templates" / pCombo->GetItem(pCombo->GetCurSel());
		}

		_RecordsetPtr prsCat = CreateParamRecordset("SELECT NoteCatID FROM EMRWordTemplateCategoryT WHERE TemplateName = {STRING}",
			strTemplateName);
		if (!prsCat->eof) {
			mi.m_nCategoryID = AdoFldLong(prsCat->GetFields(), "NoteCatID");
		}
		prsCat->Close();
	}

	// (c.haag 2004-10-01 15:49) - PLID 14242 - Add EMN IDs to the merge
	//DRT 8/28/2007 - PLID 27203 - Parameterized.
	_RecordsetPtr prsEMNs = CreateParamRecordset("SELECT ID, Description FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = {INT}", m_nEmrGroupID);
	while (!prsEMNs->eof)
	{
		mi.m_arydwEMRIDs.Add(AdoFldLong(prsEMNs, "ID"));
		prsEMNs->MoveNext();
	}
	prsEMNs->Close();
	if (!m_mergeOptions.allEMNs && mi.m_arydwEMRIDs.GetSize() > 1) {
		if (!PickEMNMergeList(mi.m_arydwEMRIDs))
			return FALSE; // The user changed their mind
	}

	// Generate the extra fields (passing in the mergeengine object so any temp files needed can be deleted AFTER the merge)
	// (z.manning 2010-01-13 17:57) - PLID 36864 - Removed an unnecessary parameter
	if (!GenerateMergeData(mi, emf.m_strHeaders, emf.m_strData, FALSE)) {
		return FALSE; // We failed to generate merge data
	}

	// Pass the emf into the merge engine so it knows where to get the extra merge fields
	mi.m_pfnCallbackExtraFields = CEMR_PIC__ExtraMergeFields;
	mi.m_pCallbackParam = &emf;

	// (z.manning 2008-06-18 14:02) - PLID 30426 - There used to be code here where we would load the 
	// procedure IDs for all EMNs on this PIC and then pass those procedure IDs into the MergeInfo's
	// as m_arydwProcInfoIDs. Needless to say that made no sense and could result in merging the incorrect
	// procedures in certain situations.

	// (c.haag 2004-10-01 15:50) - Assign the rest of the flags for the merge
	// based on user merge preferences
	if(m_mergeOptions.toPrinter) {
		mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
	}
	// (b.cardillo 2005-06-29 08:57) - PLID 16577 - Check the preference to see if we should 
	// save the document in the patient's history or not.
	if (m_mergeOptions.saveToHistory) {
		mi.m_nFlags = (mi.m_nFlags | BMS_SAVE_FILE_AND_HISTORY);
	}

	return TRUE;
}

bool CPicContainerDlg::PickEMNMergeList(IN OUT CDWordArray& adwEMNIDs)
{
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "EMRMasterT");

	if(GetRemotePropertyInt("PicMergeEMNSelection", 1, 0, GetCurrentUserName(), true) == 1) {
		for(int i=0; i < adwEMNIDs.GetSize(); i++)
			dlg.PreSelect(adwEMNIDs[i]);
	}

	CString strFrom;
	strFrom.Format("Deleted = 0 AND EMRGroupID = %d", m_nEmrGroupID);
	if (IDOK != dlg.Open("EMRMasterT", strFrom, "ID", "Description", "Please select which EMNs to merge"))
		return false;

	dlg.FillArrayWithIDs(adwEMNIDs);
	return true;
}

void CPicContainerDlg::DoMergeFromTemplate()
{	
	// (c.haag 2004-10-01 15:42) - PLID 14242 - The following code was copied from EMRDlg.cpp
	// and then reconstructed to handle EMR's instead of EMN's
	try {
		// (a.walling 2007-06-25 16:44) - PLID 15177 - Check for Word.
		if(!GetWPManager()->CheckWordProcessorInstalled())
			return;

		//TES 2/8/2006 - Make sure the EMR is saved.
		if(GetEmrEditor() && GetEmrEditor()->IsEMRUnsaved()) {
			if(IDYES != MessageBox("Before merging, the changes you have made to the EMR must be saved.  Would you like to continue?", NULL, MB_YESNO)) {
				return;
			}
			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			if(FAILED(GetEmrEditor()->Save(TRUE))) {
				MessageBox("The EMN was not saved. The merge will now be cancelled.");
				return;
			}
		}

		// Create the merge engine object
		CMergeEngine mi;
		CWaitCursor wc;
		CEMR_ExtraMergeFields emf;

		// (c.haag 2004-11-11 09:46) - PLID 14074 - Add ProcInfo and EMRGroup ID's
		mi.m_nPicID = m_nPicID;
		
		// (c.haag 2004-10-21 13:33) - PLID 14242 - Warn them if there are no EMN's
		// (f.gelderloos 2013-05-29 18:07 - PLID 56705 - Check for licensing before displaying warning
		if (GetEmrEditor() && GetEmrEditor()->IsEmpty() && g_pLicense->CheckForLicense(CLicense::lcNexEMR, CLicense::cflrSilent)) {
			if (IDNO == MessageBox("You currently have no EMN's in this EMR. Do you wish to merge a template anyway?", NULL, MB_YESNO))
				return;
		}

		CNxRibbonAutoComboBox* pCombo = GetRibbonTemplateCombo();

		if (!pCombo || -1 == pCombo->GetCurSel()) {
			return;
		}

		// Prepare the table with the patient ids we're going to merge
		CString strMergeT;
		{
			CString strSql;
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", GetPatientID());
			strMergeT = CreateTempIDTable(strSql, "ID");
		}

		// (c.haag 2004-10-05 17:07) - Fill the merge engine object with information
		if (!BuildMergeEngineObjects(emf, mi, FALSE))
			return;

		// Get template to merge to
		// (a.walling 2012-08-14 17:31) - PLID 52120 - add Templates to the root path
		// (a.walling 2012-09-11 11:16) - PLID 52581 - flip the endian-ness of the path
		CString strTemplateName = GetSharedPath() / "Templates" / pCombo->GetItem(pCombo->GetCurSel());
		if(!DoesExist(strTemplateName) ) {
			MessageBox(FormatString("The template '%s' could not be found.", strTemplateName));
			return;
		}

		//DRT 8/28/2007 - PLID 27203 - Parameterized.
		_RecordsetPtr rsProcs = CreateParamRecordset("SELECT ID, ProcedureID FROM (SELECT ProcInfoDetailsT.ID, ProcedureID "
			"FROM ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			"WHERE ProcInfoID = {INT} AND ProcedureT.MasterProcedureID IS Null  "
			"AND ProcedureID NOT IN "
			"(SELECT MasterProcedureID FROM ProcedureT INNER JOIN ProcInfoDetailsT SubT ON ProcedureT.ID = SubT.ProcedureID "
			"WHERE SubT.ProcInfoID = {INT} AND SubT.Chosen = 1) "
			"UNION SELECT ID, ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = {INT} AND Chosen = 1) SubQ", 
			m_nProcInfoID, m_nProcInfoID, m_nProcInfoID);
		if(!rsProcs->eof) {
			rsProcs->MoveLast();
			rsProcs->MoveFirst();
		}
		if(rsProcs->GetRecordCount() > 1) {
			if(IDYES == MessageBox("Would you like to generate a record for each separate procedure?", NULL, MB_YESNO)) {
				// (b.cardillo 2005-05-23 13:04) - PLID 15553 - The document is being created for one 
				// patient, yet there may be more than one record (one for each separate procedure), 
				// and so we want the page numbers to be flattened, so it doesn't reset to 1 on each 
				// new record (procedure) but rather just counts pages from 1 to the total number of 
				// pages for all procedures.
				mi.m_nFlags |= BMS_CONTINUOUS_PAGE_NUMBERING;

				//add all procedures to the merge
				while(!rsProcs->eof) {
					//Are there any detail procedures chosen?
					//DRT 8/28/2007 - PLID 27203 - Parameterized.
					_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ProcInfoDetailsT.ID FROM ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID WHERE ProcInfoDetailsT.Chosen = 1 AND ProcInfoDetailsT.ProcInfoID = {INT} AND ProcedureT.MasterProcedureID = {INT}", m_nProcInfoID, AdoFldLong(rsProcs, "ProcedureID"));
					if(rsDetails->eof) {
						mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
					}
					else {
						while(!rsDetails->eof) {
							mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsDetails, "ID"));
							rsDetails->MoveNext();
						}
					}
					rsProcs->MoveNext();
				}		
			}
			else {
				//just add one (arbitrary, this info shouldn't be merged but might be) procedure
				if(!rsProcs->eof) 
					mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
			}
		}
		else if(rsProcs->GetRecordCount() == 1) {
			//There's only one, add it.
			mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
		}

		//get the sender
		// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
		if(!mi.LoadSenderInfo(TRUE)) {
			return;
		}

		// Do the merge
		BOOL bMergeSuccess = mi.MergeToWord(strTemplateName, AsVector(m_saTempFiles), strMergeT);

		RemoveTempMergeFiles();

		// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
		if (bMergeSuccess) {
			//refresh the EMN to show the Word icon
			CClient::RefreshTable(NetUtils::EMRMasterT, GetPatientID());
		}

	} NxCatchAll("Error merging EMR template");
}


void CPicContainerDlg::DoMergeFromPacket()
{
	try {
		// (a.walling 2007-06-25 16:44) - PLID 15177 - Check for Word.
		if(!GetWPManager()->CheckWordProcessorInstalled())
			return;

		//TES 2/8/2006 - Make sure the EMR is saved.
		if(GetEmrEditor() && GetEmrEditor()->IsEMRUnsaved()) {
			if(IDYES != MessageBox("Before merging, the changes you have made to the EMR must be saved.  Would you like to continue?", NULL, MB_YESNO)) {
				return;
			}
			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			if(FAILED(GetEmrEditor()->Save(TRUE))) {
				MessageBox("The EMN was not saved. The merge will now be cancelled.");
				return;
			}
		}

		CMergeEngine mi;
		CWaitCursor wc;
		CEMR_ExtraMergeFields emf;

		// (c.haag 2004-11-11 09:46) - PLID 14074 - Add ProcInfo and EMRGroup ID's
		mi.m_nPicID = m_nPicID;
		
		// (c.haag 2004-10-21 13:33) - PLID 14242 - Warn them if there are no EMN's
		if (GetEmrEditor() && GetEmrEditor()->IsEmpty()) {
			if (IDNO == MessageBox("You currently have no EMN's in this EMR. Do you wish to generate a packet anyway?", NULL, MB_YESNO))
				return;
		}

		CNxRibbonAutoComboBox* pCombo = GetRibbonPacketCombo();

		if (!pCombo || -1 == pCombo->GetCurSel()) {
			return;
		}

		long nPacketID = pCombo->GetItemData(pCombo->GetCurSel());
		
		// (a.walling) 5/1/06 PLID 18343 
		long nPacketCategoryID = CMergeEngine::GetPacketCategory(nPacketID);
		bool bSeperateDocs = m_mergeOptions.separateHistory;
		
		_RecordsetPtr rsDocuments = CreateRecordset(
			// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here.
			// If we do, then if NOCOUNT is on, then the call to NextRecordset will return NULL
			//
			"SET NOCOUNT OFF\r\n"
			"SELECT COUNT(*) AS TotalArticleCount FROM PacketComponentsT WHERE PacketID = %li "
			"SELECT MergeTemplatesT.Path, PacketComponentsT.Scope FROM PacketComponentsT INNER JOIN MergeTemplatesT ON PacketComponentsT.MergeTemplateID = MergeTemplatesT.ID "
			"WHERE PacketID = %li "
			"ORDER BY PacketComponentsT.ComponentOrder %s", nPacketID, nPacketID, m_mergeOptions.reverseOrder ? "DESC" : "");
		// (b.cardillo 2005-05-16 12:21) - PLID 15553 - First get the total template 
		// count, then move on to the next query and proceed as normal.
		const long nArticleCount = AdoFldLong(rsDocuments->GetFields(), "TotalArticleCount");
		rsDocuments = rsDocuments->NextRecordset(NULL);

		FieldsPtr fDocs = rsDocuments->Fields;
		
		if(rsDocuments->eof) {
			MessageBox("The selected packet does not contain any templates.\nNo documents will be created.");
			return;
		}

		// (b.cardillo 2005-06-29 10:31) - PLID 16577 - Don't store the merged packet entry if the 
		// option to save in the history is turned off
		long nMergedPacketID;
		if (m_mergeOptions.saveToHistory) {
			nMergedPacketID = NewNumber("MergedPacketsT", "ID");
			ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID, SeparateDocuments) VALUES (%li, %li, %li)", nMergedPacketID, nPacketID,
				m_mergeOptions.separateHistory ? 1 : 0);
		} else {
			nMergedPacketID = -1;
		}

		// Prepare the table with the patient ids we're going to merge
		CString strMergeT;
		{
			CString strSql;
			strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", GetPatientID());
			strMergeT = CreateTempIDTable(strSql, "ID");
		}

		// (c.haag 2004-10-05 17:07) - Fill the merge engine object with information
		if (!BuildMergeEngineObjects(emf, mi, TRUE))
			return;

		long nArticleNumber = 1;
		//TES 8/1/2006 - Some templates may be skipped (if no detail procedures are present), and if no templates end up being 
		// merged, we need to tell the user why.
		bool bAtLeastOneMerged = false; 

		//TES 12/21/2006 - PLID 23957 - Track the total progress.
		long nDocCount = 0;
		if(!rsDocuments->eof) {
			rsDocuments->MoveLast();
			rsDocuments->MoveFirst();
			nDocCount = rsDocuments->GetRecordCount();
		}
		long nCurrentDoc = 0;
		while(!rsDocuments->eof) {
			nCurrentDoc++;
			CString strTemplateName = AdoFldString(fDocs, "Path");
			if(!DoesExist(strTemplateName) ) {
				//Try the shared path.
				CString strNameWithPath = GetSharedPath() ^ strTemplateName;
				if(!DoesExist(strNameWithPath)) {
					MessageBox(FormatString("The template '%s' could not be found.", strTemplateName));
					return;
				}
				else {
					strTemplateName = strNameWithPath;
				}
			}
			mi.m_arydwProcInfoIDs.RemoveAll();
			bool bMerge = true; //We may decide to skip this template.
			long nScope = AdoFldLong(fDocs, "Scope");
			if(nScope) {
				// (b.cardillo 2005-05-18 14:35) - PLID 15553 - Even if this is a multi-procedure 
				// template, we are still generating ONE document, so we want the page numbers to 
				// be considered continuous, rather than resetting to 1 on each record.
				mi.m_nFlags |= BMS_CONTINUOUS_PAGE_NUMBERING;

				if(nScope == 1 || nScope == 2 || nScope == 4) {//1=per master procedure, 2=per procedure, 4=per detail procedure.
					//add all procedures to the merge
					_RecordsetPtr rsProcs = CreateRecordset("SELECT ProcInfoDetailsT.ID, ProcedureT.ID AS ProcedureID FROM ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
						"WHERE ProcInfoDetailsT.ProcInfoID = %li AND ProcedureT.MasterProcedureID Is Null", m_nProcInfoID);
					while(!rsProcs->eof) {
						if(nScope == 2 || nScope == 4) {
							//Are there any detail procedures chosen?
							_RecordsetPtr rsDetails = CreateRecordset("SELECT ProcInfoDetailsT.ID "
								"FROM ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
								"WHERE ProcInfoDetailsT.Chosen = 1 AND ProcInfoDetailsT.ProcInfoID = %li "
								"AND ProcedureT.MasterProcedureID = %li", m_nProcInfoID, AdoFldLong(rsProcs, "ProcedureID"));
							if(rsDetails->eof) {
								if(nScope == 2) {
									//We're per procedure, so just use the master.
									mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
								}
							}
							else {
								while(!rsDetails->eof) {
									mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsDetails, "ID"));
									rsDetails->MoveNext();
								}
							}
						}
						else {
							//We just want master procedures, regardless of what's chosen.
							mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
						}

						rsProcs->MoveNext();
					}
				}
				else if(nScope == 3) {//1 per medication
					//Add all medications to the merge.
					_RecordsetPtr rsMeds = CreateRecordset("SELECT PatientMedications.ID "
						"FROM PatientMedications INNER JOIN EmrMedicationsT ON PatientMedications.ID = EmrMedicationsT.MedicationID "
						"INNER JOIN EmrMasterT ON EmrMedicationsT.EmrID = EmrMasterT.ID "
						"INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID "
						"WHERE PicT.ID = %li AND EMRMasterT.Deleted = 0 AND EMRMedicationsT.Deleted = 0", m_nPicID);
					while(!rsMeds->eof) {
						mi.m_arydwPrescriptionIDs.Add(AdoFldLong(rsMeds, "ID"));
						rsMeds->MoveNext();
					}
				}

				if(nScope == 4 && mi.m_arydwProcInfoIDs.GetSize() == 0) {
					//This was per detail procedure, but there were no details selected, so skip this template.
					bMerge = false;
				}
			}
			else {
				//just add one (arbitrary, this info shouldn't be merged but might be) procedure
				_RecordsetPtr rsProcs = CreateRecordset("SELECT ProcInfoDetailsT.ID FROM ProcInfoDetailsT WHERE ProcInfoID = %li", m_nProcInfoID);
				if(!rsProcs->eof) 
					mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
			}

			if(bMerge) {
				//get the sender
				// (z.manning, 03/05/2008) - PLID 29131 - Now have a function to load sender information.
				if(!mi.LoadSenderInfo(TRUE)) {
					return;
				}
				
				// (b.cardillo 2005-05-16 13:51) - PLID 15553 - Include the article number and count 
				// as merge fields, and then increment the count.
				mi.m_lstExtraFields.RemoveAll();
				mi.m_lstExtraFields.Add("Packet_Article_Number", AsString(nArticleNumber));
				mi.m_lstExtraFields.Add("Packet_Article_Count", AsString(nArticleCount));
				nArticleNumber++;

				// Do the merge
				// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
				if (!mi.MergeToWord(strTemplateName, AsVector(m_saTempFiles), strMergeT, "", nMergedPacketID, nPacketCategoryID, bSeperateDocs, FormatString("Template %li of %li", nCurrentDoc, nDocCount))) {
					break;
				}

				//We're good to go ahead and merge.
				bAtLeastOneMerged = true;
			}
			rsDocuments->MoveNext();
		}

		if(bAtLeastOneMerged) {
			PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PacketSent, GetPatientID(), COleDateTime::GetCurrentTime(), nMergedPacketID);

			//refresh the EMN to show the Word icon
			CClient::RefreshTable(NetUtils::EMRMasterT, GetPatientID());
		}
		else {
			MessageBox("All documents in this packet were specified to merge for detail procedures, but this PIC did not have any "
				"detail procedures selected.  No action has been taken.");
		}

		RemoveTempMergeFiles();

	} NxCatchAll("Error merging EMR packet");
}

// (j.dinatale 2012-06-29 12:49) - PLID 51282 - need to refresh certain panes when a file is attached to history
void CPicContainerDlg::RefreshPhotoPane()
{
	if (m_emrPhotosPane.GetSafeHwnd()) {
		
		// (j.jones 2014-08-04 10:01) - PLID 63144 - do not refresh if the photos pane is not visible
		if (m_emrPhotosPane.IsWindowVisible()) {
			// (j.jones 2012-07-12 17:48) - PLID 49636 - OnPhotoGroup will update the group, if one is
			// in use (grouping by tabs of categories, procedures, or none), and reload the images
			OnPhotoGroup();
		}
		else {
			// (j.jones 2014-08-04 10:01) - PLID 63144 - tell the pane to refresh the next time
			// it becomes visible
			m_bPhotosPaneNeedsRefresh = true;
		}
	}
}

// (j.dinatale 2012-07-12 14:32) - PLID 51481 - keep track of whether we wanted to show Proc Info Center
bool CPicContainerDlg::OpenedForPic()
{
	return m_bLoadedForPic;
}

// (j.dinatale 2012-07-13 12:59) - PLID 51481 - check and see if we opened this thing for the PIC
void CPicContainerDlg::OnSaveAndClose()
{
	try{
		if(OpenedForPic() && !GetEmrTreeWnd()->GetEMR()->HasValidInfo()){
			PostMessage(WM_CLOSE);
			return;
		}

		__super::OnSaveAndClose();
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-11-09 08:45) - PLID 53670 - Create a view from path name
CMDIChildWndEx* CPicContainerDlg::CreateDocumentWindow(LPCTSTR lpcszDocName, CObject* pObj)
{
	CView* pView = NULL;
	if (!strcmp(lpcszDocName, "History.View.Emr.Nx")) {
		if (!WindowUtils::IsCommandEnabled(ID_EMR_VIEW_HISTORY, this)) {
			return NULL;
		}
		pView = m_pPicDocTemplate->GetHistoryView(true);
	} else if (!strcmp(lpcszDocName, "Labs.View.Emr.Nx")) {
		if (!WindowUtils::IsCommandEnabled(ID_EMR_VIEW_LABS, this)) {
			return NULL;
		}
		pView = m_pPicDocTemplate->GetPatientLabsView(true);
	} else if (!strcmp(lpcszDocName, "ProcInfoCenter.View.Emr.Nx")) {
		if (!WindowUtils::IsCommandEnabled(ID_EMR_VIEW_PIC, this)) {
			return NULL;
		}
		pView = m_pPicDocTemplate->GetProcInfoCenterView(true);
	}

	// (a.walling 2013-02-13 09:30) - PLID 52629 - Dynamic view layout - call base class
	if (pView) {
		return dynamic_cast<CMDIChildWndEx*>(pView->GetParentFrame());
	} else {
		return __super::CreateDocumentWindow(lpcszDocName, pObj);
	}
}

// (j.jones 2014-08-04 10:12) - PLID 63144 - added OnShowWindow
void CPicContainerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {

		CEmrPatientFrameWnd::OnShowWindow(bShow, nStatus);

		// (j.jones 2014-08-04 10:21) - PLID 63144 - if m_bPhotosPaneNeedsRefresh
		// is true, reload the photos now
		if (bShow && m_bPhotosPaneNeedsRefresh) {
			OnPhotoGroup();
		}
	} NxCatchAll(__FUNCTION__);
}