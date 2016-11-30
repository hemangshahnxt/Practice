// EMNMoreInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMNMoreInfoDlg.h"
#include "GlobalFinancialUtils.h"
#include "EMRSelectServiceDlg.h"
#include "MedicationSelectDlg.h"
#include "SelectDlg.h"
#include "GlobalDrawingUtils.h"
#include "EmrTreeWnd.h"
#include "EMN.h"
#include "TaskEditDlg.h"
#include "MultiSelectDlg.h"
#include "mergeengine.h"
#include "PrescriptionEditDlg.h"
#include "DontShowDlg.h"
#include "EditMedicationListDlg.h"
//#include "PatientsRc.h"
#include "TodoUtils.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "EMRProblemEditDlg.h"
#include "EditEMNOtherProvidersDlg.h"
#include "EMRProblemChooserDlg.h"
#include "ReconcileMedicationsDlg.h"
//#include "EmrRc.h"
#include "CPTCodes.h"
#include "InvEditDlg.h"
#include "NxModalParentDlg.h"
#include "HL7Utils.h"
#include "EMRChargePromptDlg.h"
#include "ChargeSplitDlg.h"
#include "EmrFrameWnd.h"
#include <NxOccManager.h>
#include "EmrColors.h"
#include <NxUILib/WindowUtils.h>
#include "EMNMedication.h"	// (j.jones 2012-11-30 14:03) - PLID 53966 - moved the EMNMedication class to its own file
#include "FavoritePharmaciesEditDlg.h" 
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here 
#include "EmrCodingGroupManager.h"
#include "EMNDetail.h"
#include "EMR.h"
#include "MedlinePlusUtils.h"
#include "EMNProvider.h"
#include "EMNTodo.h"
#include "EMRProblemListDlg.h"
#include "ConfigEMRPreviewDlg.h"
#include "NxWordProcessorLib\GenericWordProcessorManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.





#define	ID_DELETE_PRESCRIPTION	44930
#define ID_ADD_DEFAULT_ICD9S 44931
#define ID_REMOVE_PROCEDURE	44932
#define ID_EDIT_EMN_TODO 44933
#define ID_DELETE_EMN_TODO 44934
// (j.jones 2008-07-22 13:06) - PLID 30792 - added problem options
#define ID_ADD_MED_PROBLEM 44939
#define ID_EDIT_MED_PROBLEM 44940
#define ID_LINK_MED_PROBLEMS 44943

using namespace ADODB;
using namespace NXDATALISTLib;

extern CPracticeApp theApp;

// (a.walling 2009-04-22 16:10) - PLID 33948 - i can barely read with the default colors
// (j.gruber 2009-05-20 17:42) - PLID 34291 - changed the colors
/*****IF YOU CHANGE THESE, ALSO CHANGE THEM IN MedicationsDlg.cpp as well as PrescriptionQueueDlg.cpp******/
#define EPRESCRIBE_COLOR_DISCONTINUED RGB(222, 225, 231)
#define EPRESCRIBE_COLOR RGB(178, 251, 197)


// (c.haag 2008-07-07 11:38) - PLID 30607 - Added todo list
// (c.haag 2008-07-11 15:48) - PLID 30689 - Added regarding type
enum TodoListColumns
{
	tlcID = 0,
	tlcDetailID,
	tlcDetailName,
	tlcAssignees,
	tlcAssigneeIDs,
	tlcDeadline,
	tlcNotes,
	tlcDone,
	tlcPriority,
	tlcRegardingType,
};

// (j.jones 2008-11-17 09:19) - PLID 31718 - added enum for the procedure combo
enum ProcedureComboColumns {

	pccID = 0,
	pccName,
	pccHasLadder,
};
 
/////////////////////////////////////////////////////////////////////////////
// CEMNMoreInfoDlg dialog

//DRT 1/12/2007 - PLID 24177 
//Takes a delimited EMNCharge diag list and converts it to a displayable
//	list that can be put in an interface.
// (j.jones 2009-01-02 09:23) - PLID 32601 - removed, no longer needed
/*
CString FormatWhichcodes(CString strDiagList)
{
	//The diagnosis list in the EMNCharge structure is delimited by ||.  We need to parse that out and display it
	//	comma separated for the interface.
	CString strIn = strDiagList;	//copy
	CString strOut;

	//Break apart by delimiter
	long nFind = strIn.Find("||");
	while(nFind > -1) {
		strOut += strIn.Left(nFind);
		strOut += ", ";

		strIn = strIn.Mid(nFind + 2);

		nFind = strIn.Find("||");
	}

	//When done, there's 1 left in the input
	strOut += strIn;


	//If nothing at all was in the list, show <None>
	if(strOut.IsEmpty())
		strOut = "<None>";

	return strOut;
}
*/

// (a.walling 2007-07-03 13:04) - PLID 23714 - Added table checkers
// (a.walling 2007-10-04 13:43) - PLID 23714 - Added cptcode checker
CEMNMoreInfoDlg::CEMNMoreInfoDlg(CWnd* pParent)
	: CNxDialog(CEMNMoreInfoDlg::IDD, pParent),
	m_checkProcedures(NetUtils::AptPurposeT),
	m_checkProviders(NetUtils::Providers), m_checkLocations(NetUtils::LocationsT),
	m_checkCategories(NetUtils::EMNTabCategoriesT), m_checkCharts(NetUtils::EMNTabChartsT),
	m_checkCategoryChartLink(NetUtils::EmnTabChartCategoryLinkT), 
	m_checkTechnicians(NetUtils::Coordinators)
{
	m_bReadOnly = FALSE;
	m_pAddFolderThread = NULL;
	
	//{{AFX_DATA_INIT(CEMNMoreInfoDlg)
		m_bIsTemplate = FALSE;
		m_bkgColor = 0x8000000F;
	//}}AFX_DATA_INIT

	// (a.walling 2010-03-25 20:08) - PLID 34886 - All work and uninitialized variables make Adam 0xCCCCCCCC
	m_nPatientID = -1;
	m_pEMN = NULL;

	// (a.walling 2010-03-31 13:58) - PLID 34886 - More uninitialized variables	
	m_nPendingLocationID = -1;
	m_nPendingProviderID = -1;
	m_nPendingSecondaryProviderID = -1;
	m_nPendingTechnicianID = -1;
	m_bInitialized = FALSE;

}

CEMNMoreInfoDlg::~CEMNMoreInfoDlg()
{
	if(m_pAddFolderThread) {
		//TES 10/26/2007 - PLID 26831 - This object's destructor will handle all the necessary thread cleanup.
		delete m_pAddFolderThread;
		m_pAddFolderThread = NULL;
	}
}

void CEMNMoreInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	// (a.walling 2008-04-03 13:16) - PLID 29497 - Added NxButtons
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMNMoreInfoDlg)
	DDX_Control(pDX, IDC_PRINT_PRESCRIPTIONS_TO_PRINTER, m_nxbPrintDirectlyToPrinter);
	DDX_Control(pDX, IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK, m_btnSaveInHistory);
	DDX_Control(pDX, IDC_CHECK_MERGEEMNTOPRINTER, m_btnMergeToPrinter);
	DDX_Control(pDX, IDC_BTN_UPDATE_PAT_INFO, m_btnUpdatePatInfo);
	DDX_Control(pDX, IDC_BTN_CONFIG_PREVIEW, m_btnConfigPreview);
// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history handled by the frame now -- use standard ID
	DDX_Control(pDX, ID_EMR_SHOW_EMN_AUDIT_HISTORY, m_btnShowHistory);
	DDX_Control(pDX, IDC_PATEMR_EDIT_TEMPLATES, m_btnEditTemplates);
	DDX_Control(pDX, IDC_EMR_MAKE_DEFAULT, m_btnMakeDefault);
	DDX_Control(pDX, IDC_BTN_MERGE_TO_OTHER, m_btnMergeToOther);
	DDX_Control(pDX, IDC_MERGE_TO, m_btnMergeTo);
	DDX_Control(pDX, IDC_BTN_WRITE_PRESCRIPTIONS, m_btnSavePrintPrescriptions);
	DDX_Control(pDX, IDC_BTN_ADD_MEDICATION, m_btnAddMedication);
	DDX_Control(pDX, IDC_BKG3, m_bkg3);
	DDX_Control(pDX, IDC_BKG4, m_bkg4);
	DDX_Control(pDX, IDC_BKG5, m_bkg5);
	DDX_Control(pDX, IDC_EMN_DATE, m_dtDate);
	DDX_Control(pDX, IDC_BKG8, m_bkg8);
	DDX_Control(pDX, IDC_BKG9, m_bkg9);
	DDX_Control(pDX, IDC_BKG10, m_bkg10);
	DDX_Control(pDX, IDC_SELECT_EMN_PROVIDERS, m_nxlProviderLabel);
	DDX_Control(pDX, IDC_SELECT_EMN_SECONDARY_PROVIDERS, m_nxlSecondaryProvLabel);
	DDX_Control(pDX, IDC_SELECT_EMN_TECHNICIAN, m_nxlTechnicianLabel);
	DDX_Control(pDX, ID_EMR_EMN_DESCRIPTION, m_nxeditEmnDescription);
	DDX_Control(pDX, IDC_EMN_PATIENT_NAME, m_nxeditEmnPatientName);
	DDX_Control(pDX, IDC_EMN_AGE, m_nxeditEmnAge);
	DDX_Control(pDX, IDC_EMN_GENDER, m_nxeditEmnGender);
	DDX_Control(pDX, ID_EMR_EMN_NOTES, m_nxeditEditEmrNotes);
	DDX_Control(pDX, IDC_NOTES_LABEL, m_nxstaticNotesLabel);
	DDX_Control(pDX, IDC_EMR_DEFAULT_FOR, m_nxstaticEmrDefaultFor);
	DDX_Control(pDX, IDC_RADIO_MORE_INFO_TODO_INCOMPLETE, m_radioTodoIncomplete);
	DDX_Control(pDX, IDC_RADIO_MORE_INFO_TODO_COMPLETED, m_radioTodoCompleted);
	DDX_Control(pDX, IDC_RADIO_MORE_INFO_TODO_ALL, m_radioTodoAll);
	DDX_Control(pDX, IDC_EMR_OTHER_PROVS, m_btnOtherProviders);
	DDX_Control(pDX, IDC_SPELL_CHECK_NOTES, m_btnSpellCheckNotes);
	DDX_Control(pDX, IDC_BTN_RX_QUEUE_MORE_INFO, m_btnEPrescribing);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_DATE, 2 /* Change */, OnChangeEmnDate, VTS_NONE)

// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
BEGIN_MESSAGE_MAP(CEMNMoreInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMNMoreInfoDlg)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CEMNMoreInfoDlg::OnIdleUpdateCmdUI)

	// (a.walling 2012-03-23 18:01) - PLID 50638 - Notes, Description
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_NOTES, &CEMNMoreInfoDlg::OnUpdateNotes)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_DESCRIPTION, &CEMNMoreInfoDlg::OnUpdateDescription)

	ON_BN_CLICKED(IDC_BTN_ADD_MEDICATION, OnBtnAddMedication)
	ON_BN_CLICKED(IDC_BTN_WRITE_PRESCRIPTIONS, OnBtnWritePrescriptions)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(NXM_EMN_MEDICATION_ADDED, OnEmnMedicationAdded)
	ON_MESSAGE(NXM_EMN_PROCEDURE_ADDED, OnEmrProcedureAdded)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EMN_DATE, OnChangeEmnDate)
	ON_EN_CHANGE(ID_EMR_EMN_DESCRIPTION, OnChangeEmnDescription)
	ON_EN_CHANGE(ID_EMR_EMN_NOTES, OnChangeEmnNotes)
	ON_COMMAND(ID_REMOVE_PROCEDURE, OnRemoveProcedure)
	ON_BN_CLICKED(IDC_MERGE_TO, OnMergeTo)
	ON_BN_CLICKED(IDC_EMR_MAKE_DEFAULT, OnEmrMakeDefault)
	ON_BN_CLICKED(IDC_PATEMR_EDIT_TEMPLATES, OnPatemrEditTemplates)
	ON_BN_CLICKED(IDC_BTN_MERGE_TO_OTHER, OnBtnMergeToOther)
	ON_BN_CLICKED(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK, OnEmrSaveDocsInHistoryCheck)
	ON_BN_CLICKED(IDC_CHECK_MERGEEMNTOPRINTER, OnEmrMergeToPrinterCheck)
	// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history handled by the frame now
	ON_COMMAND_EX(ID_EMR_SHOW_EMN_AUDIT_HISTORY, &CEMNMoreInfoDlg::RouteCommandToParent)
	ON_MESSAGE(NXM_ADD_FOLDER_NEXT_FOLDER, OnAddFolderNext)
	ON_MESSAGE(NXM_ADD_FOLDER_COMPLETED, OnAddFolderCompleted)
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CONFIG_PREVIEW, OnConfigPreview)
	ON_BN_CLICKED(IDC_BTN_UPDATE_PAT_INFO, OnBtnUpdatePatInfo)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_PRINT_PRESCRIPTIONS_TO_PRINTER, OnPrintPrescriptionsToPrinter)
	ON_COMMAND(ID_EDIT_EMN_TODO, OnEditEMNTodo)
	ON_COMMAND(ID_DELETE_EMN_TODO, OnDeleteEMNTodo)
	// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info
	ON_COMMAND_EX(ID_EMR_SHOW_CONFIDENTIAL_INFO, &CEMNMoreInfoDlg::RouteCommandToParent)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EMN_OTHER_PROVS, &CEMNMoreInfoDlg::OnBnClickedEmnOtherProvs)
	ON_BN_CLICKED(IDC_SPELL_CHECK_NOTES, &CEMNMoreInfoDlg::OnSpellCheckNotes)
	ON_COMMAND_EX(ID_EMR_EMN_APPOINTMENT, &CEMNMoreInfoDlg::RouteCommandToParent) // (a.walling 2013-02-13 12:07) - PLID 55143 - Emr Appointment linking - UI
	ON_BN_CLICKED(IDC_BTN_RX_QUEUE_MORE_INFO, &CEMNMoreInfoDlg::OnBnClickedBtnRxQueueMoreInfo)
	ON_BN_CLICKED(IDC_MERGE_SUMMARY_CARE_BTN, &CEMNMoreInfoDlg::OnBnClickedMergeSummaryCareBtn)
	ON_BN_CLICKED(IDC_MERGE_CLINICAL_SUMMARY_BTN, &CEMNMoreInfoDlg::OnBnClickedMergeClinicalSummaryBtn)
	ON_COMMAND_EX(ID_EMR_CLINICAL_SUMMARY, &CEMNMoreInfoDlg::RouteCommandToParent)
	ON_COMMAND_EX(ID_EMR_CLINICAL_SUMMARY_CUSTOMIZED, &CEMNMoreInfoDlg::RouteCommandToParent)
	ON_COMMAND_EX(ID_EMR_CARE_SUMMARY, &CEMNMoreInfoDlg::RouteCommandToParent)
	ON_COMMAND_EX(ID_EMR_CARE_SUMMARY_CUSTOMIZED, &CEMNMoreInfoDlg::RouteCommandToParent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMNMoreInfoDlg message handlers


BOOL CEMNMoreInfoDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {

		// (z.manning 2009-02-19 10:58) - PLID 33154 - Cached all the properties in CEMNMoreInfoDlg::OnInitDialog
		g_propManager.CachePropertiesInBulk("EMNMoreInfoDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"	'CloseEmrWhenPrintingPrescriptions', "
			"	'EmrMoreInfoShowHasLadder', "
			// (z.manning 2009-02-20 16:58) - PLID 33154 - This is cached in  EmrEditorDlg
			//"	'EMRColorBottom', "
			"	'PICSaveDocsInHistory', "
			"	'PICMergeEMNToPrinter', "
			"	'MoreInfoPrintPrescriptionsToPrinter', "
			"	'EMNUseLastApptDate', "	// (j.jones 2009-09-24 10:07) - PLID 29718
			"	'ReconcileNewRxWithCurMeds', " // (c.haag 2010-02-17 17:51) - PLID 37384
			"	'ReconcileNewRxWithCurMeds_SkipEMRTable' " // (j.jones 2013-01-11 13:07) - PLID 54462
			")"
			, _Q(GetCurrentUserName()));

		// (c.haag 2008-04-25 15:51) - PLID 29796 - NxIconify buttons
		m_btnUpdatePatInfo.AutoSet(NXB_MODIFY);
		m_btnAddMedication.AutoSet(NXB_NEW);



		//TES 6/17/2008 - PLID 30414 - Depending on our preference, the Save and Print Prescriptions button may close
		// the dialog, or it may just merge to Word.
		if(GetRemotePropertyInt("CloseEmrWhenPrintingPrescriptions", 0, 0, GetCurrentUserName(), true)) {
			m_btnSavePrintPrescriptions.AutoSet(NXB_OK);
		}
		else {
			m_btnSavePrintPrescriptions.AutoSet(NXB_MERGE);
		}

		m_btnMakeDefault.AutoSet(NXB_MODIFY);
		m_btnMergeTo.AutoSet(NXB_MERGE);
		m_btnMergeToOther.AutoSet(NXB_MERGE);
		m_btnUseEMChecklist .AutoSet(NXB_MODIFY);

		//TES 10/5/2010 - PLID 40183 - Added a spellchecker for the More Info notes.
		m_btnSpellCheckNotes.AutoSet(NXB_SPELLCHECK);


		// (z.manning, 05/22/2007) - PLID 25569 - Don't requery the category list because it's based on the selected
		// chart. Instead wait until Initialize runs at which point we'll know the chart.
		// (z.manning 2013-06-05 09:15) - PLID 56962 - We now want to requery this list right away.
		m_pCategoryCombo = BindNxDataList2Ctrl(IDC_EMN_CATEGORY, true);


		//TES 2/12/2009 - PLID 33034 - Need to requery so that the embedded dropdowns will 
		// requery, clauses are set so this won't actually do anything.
		m_MedicationList = BindNxDataListCtrl(IDC_MEDICATIONS);

		// (z.manning, 04/11/2007) - PLID 25569
		m_pChartCombo = BindNxDataList2Ctrl(IDC_EMN_CHART, false);
		// (z.manning 2013-06-04 10:55) - PLID 56962 - Set the from clause here as we now include a delimited list of
		// linked category IDs.
		m_pChartCombo->PutFromClause(
			"( \r\n"
			"	SELECT DISTINCT Chart.ID, Chart.Description, STUFF(( \r\n"
			"		SELECT ',' + CONVERT(VARCHAR(20), EmnTabCategoryID) \r\n"
			"		FROM EmnTabChartCategoryLinkT LinkInner \r\n"
			"		WHERE LinkInner.EmnTabChartID = Chart.ID \r\n"
			"		FOR XML PATH('') \r\n"
			"		), 1, 1, '') AS CategoryIDs \r\n"
			"	FROM EmnTabChartsT Chart \r\n"
			"	LEFT JOIN EmnTabChartCategoryLinkT Link ON Link.EmnTabChartID = Chart.ID \r\n"
			") ChartQ \r\n"
			);
		m_pChartCombo->Requery();
	
		// (c.haag 2008-07-07 11:07) - PLID 30607 - Todo list
		m_pTodoList = BindNxDataList2Ctrl(IDC_EMN_TODO_LIST, false);
		if (!m_bIsTemplate) {
			m_radioTodoIncomplete.SetCheck(1);
		}

		// (d.lange 2011-03-22 16:42) - PLID 42136 - Assistant/Technician combo
		m_pTechnicianCombo = BindNxDataList2Ctrl(IDC_EMN_TECH, true);

		// (r.farnworth 2014-02-14 15:15) - PLID 60745
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		// (j.gruber 2007-01-08 09:45) - PLID 23399 - add secondary provider field
		// (a.walling 2007-11-09 13:43) - PLID 28059 - VS2008 - This crashes VS2008 since we were using BindNxDataListCtrl for a datalist2
		m_pSecondaryProvCombo = BindNxDataList2Ctrl(IDC_EMN_SECONDARY, true);

		m_pProviderCombo = GetDlgItemUnknown(IDC_EMN_PROVIDER);
		// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
		m_pProviderCombo->PutAdoConnection(GetRemoteDataSnapshot());
		//TES 12/22/2006 - PLID 23398 - Filter on licensed providers only.
		CString strLicensedProviders;
		CDWordArray dwaLicensedProviders;
		g_pLicense->GetUsedEMRProviders(dwaLicensedProviders);
		for(int i = 0; i < dwaLicensedProviders.GetSize(); i++) {
			strLicensedProviders += FormatString("%i,",dwaLicensedProviders[i]);
		}
		strLicensedProviders.TrimRight(",");
		if(strLicensedProviders.IsEmpty()) strLicensedProviders = "-1";
		m_pProviderCombo->WhereClause = _bstr_t("PersonT.Archived = 0 AND PersonT.ID IN (" + strLicensedProviders + ")");
		m_pProviderCombo->Requery();

		m_pLocationCombo = GetDlgItemUnknown(IDC_EMN_LOCATION);
		// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
		m_pLocationCombo->PutAdoConnection(GetRemoteDataSnapshot());
		m_pLocationCombo->Requery();

		m_pStatusCombo = GetDlgItemUnknown(IDC_EMN_STATUS);
		// (j.jones 2011-07-05 11:05) - PLID 43603 - this now queries from data,
		// but we keep our built in statuses in their intended order, before any custom statuses
		_RecordsetPtr rsStatus = CreateRecordset(GetRemoteDataSnapshot(), "SELECT ID, Name FROM EMRStatusListT "
			"WHERE ID IN (0,1,2) ORDER BY ID "
			""
			"SELECT ID, Name FROM EMRStatusListT "
			"WHERE ID NOT IN (0,1,2) ORDER BY Name");
		while(!rsStatus->eof) {
			pRow = m_pStatusCombo->GetNewRow();
			pRow->PutValue(0, rsStatus->Fields->Item["ID"]->Value);
			pRow->PutValue(1, rsStatus->Fields->Item["Name"]->Value);
			m_pStatusCombo->AddRowAtEnd(pRow, NULL);

			rsStatus->MoveNext();
		}
		rsStatus = rsStatus->NextRecordset(NULL);
		while(!rsStatus->eof) {
			pRow = m_pStatusCombo->GetNewRow();
			pRow->PutValue(0, rsStatus->Fields->Item["ID"]->Value);
			pRow->PutValue(1, rsStatus->Fields->Item["Name"]->Value);
			m_pStatusCombo->AddRowAtEnd(pRow, NULL);

			rsStatus->MoveNext();
		}
		rsStatus->Close();

		m_pTemplateCombo = BindNxDataListCtrl(IDC_TEMPLATE_LIST,false);

		
		//TES 6/20/2006 - This is slow, because it synchronously fills the four different lists of CPT Modifiers.  Let's prepare it
		//in a thread.
		//m_BillList->Requery();

		m_pProcedureCombo = GetDlgItemUnknown(IDC_EMN_PROCEDURE_COMBO);
		// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
		m_pProcedureCombo->PutAdoConnection(GetRemoteDataSnapshot());
		m_pProcedureCombo->Requery();
		m_pProcedureList = GetDlgItemUnknown(IDC_EMN_PROCEDURE_LIST);
		// (z.manning 2009-02-19 10:21) - PLID 33154 - We now have a preference to show/hide the Has Ladder column
		NXDATALIST2Lib::IColumnSettingsPtr pcolHasLadder = m_pProcedureCombo->GetColumn(pccHasLadder);
		if(pcolHasLadder != NULL) {
			if(GetRemotePropertyInt("EmrMoreInfoShowHasLadder", 0, 0, "<None>", true) == 0) {
				pcolHasLadder->PutStoredWidth(0);
			}
		}

		//TES 3/10/2014 - PLID 61313 - Various decisions were being made here based on m_bIsTemplate, but that
		// variable hasn't actually been set yet. I moved that code to EnableControls()

		m_nxlProviderLabel.SetType(dtsHyperlink);

		m_nxlSecondaryProvLabel.SetType(dtsHyperlink);

		// (d.lange 2011-03-23 09:49) - PLID 42136 -
		m_nxlTechnicianLabel.SetType(dtsHyperlink);

		// (j.gruber 2009-05-08 09:19) - PLID 33688 - Other Provider Button
		m_btnOtherProviders.AutoSet(NXB_MODIFY);

		// (a.walling 2007-07-03 13:06) - PLID 23714 - Get messages from the mainframe
		GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());

		EnableControls();

		CheckDlgButton(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK, GetRemotePropertyInt("PICSaveDocsInHistory", TRUE, 0, GetCurrentUserName(), false)?BST_CHECKED:BST_UNCHECKED);

		// (c.haag 2009-08-10 11:12) - PLID 29160 - Preference for merging the EMN directly to a printer. Defaults to off.
		CheckDlgButton(IDC_CHECK_MERGEEMNTOPRINTER, GetRemotePropertyInt("PICMergeEMNToPrinter", FALSE, 0, GetCurrentUserName(), false)?BST_CHECKED:BST_UNCHECKED);

		//TES 6/17/2008 - PLID 30411 - Load our "Merge directly to printer" preference.
		CheckDlgButton(IDC_PRINT_PRESCRIPTIONS_TO_PRINTER, GetRemotePropertyInt("MoreInfoPrintPrescriptionsToPrinter", FALSE, 0, GetCurrentUserName(), true)?BST_CHECKED:BST_UNCHECKED);

		// (b.savon 2013-01-24 10:38) - PLID 54817
		if( g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNone ){
			m_btnEPrescribing.SetWindowText("Write Prescription");
		}
		m_btnEPrescribing.AutoSet(NXB_PILLBOTTLE);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMNMoreInfoDlg::Initialize(CEMN *pEMN)
{
	try {
		m_pEMN = pEMN;

		m_nPatientID = m_pEMN->GetParentEMR()->GetPatientID();
		m_bIsTemplate = m_pEMN->IsTemplate();
		// (b.savon 2013-09-10 12:13) - PLID 58497 - Restore the Medication section on More Info to pre-10.7 for NewCrop clients
		BOOL bNewCrop = g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop;
		BOOL bUseLegacyInterface = m_bIsTemplate || bNewCrop;

		// (b.savon 2013-01-24 10:38) - PLID 54817 - Disable if were on a template
		// If this is a template, Hide the EPrescribing button but show the legacy
		// If this is a live EMN, Hide the legacy buttons and show the EPrescribing
		m_btnEPrescribing.ShowWindow( bUseLegacyInterface ? SW_HIDE : SW_SHOW);
		m_btnEPrescribing.EnableWindow(bUseLegacyInterface ? FALSE : TRUE);

		m_btnSavePrintPrescriptions.ShowWindow(bUseLegacyInterface ? SW_SHOW : SW_HIDE);
		m_btnSavePrintPrescriptions.EnableWindow(bUseLegacyInterface ? TRUE : FALSE);
		m_btnAddMedication.ShowWindow(bUseLegacyInterface ? SW_SHOW : SW_HIDE);
		m_btnAddMedication.EnableWindow(bUseLegacyInterface ? TRUE : FALSE);
		m_nxbPrintDirectlyToPrinter.ShowWindow(bUseLegacyInterface ? SW_SHOW : SW_HIDE);
		m_nxbPrintDirectlyToPrinter.EnableWindow(bUseLegacyInterface ? TRUE : FALSE);

		//propagate the screen with the EMN data


		//medications
		// (a.walling 2007-10-01 09:13) - PLID 27568 - in other words, RefreshPrescriptionList().
		RefreshPrescriptionList();

		//Procedures.
		// (a.walling 2007-08-06 13:20) - PLID 23714 - Fill the procedure list
		RefreshProcedureList();

		//notes
		SetDlgItemText(ID_EMR_EMN_NOTES, m_pEMN->GetNotes());

		//TES 12/26/2006 - PLID 23400 - Load all the providers.
		CArray<long,long> arProviderIDs;
		m_pEMN->GetProviders(arProviderIDs);
		SetProviders(arProviderIDs);
		SetLocation(m_pEMN->GetLocationID());
		SetDate(m_pEMN->GetEMNDate());
		SetStatus(m_pEMN->GetStatus());
		// (z.manning, 04/11/2007) - PLID 25569
		SetChart(m_pEMN->GetChart());
		RefilterCategoryList();
		SetCategory(m_pEMN->GetCategory());

		// (j.jones 2007-08-24 09:22) - PLID 27054 - added visit type
		//TrySetVisitType(m_pEMN->GetVisitTypeID(), m_pEMN->GetVisitTypeName());

		//m.hancock - 3/14/2006 - 19579 - Patient demographics shouldn't change after the EMN is locked or finished.
		//The EMN handles data retrieval, so simply display what the EMN has loaded.
		// (a.walling 2007-08-06 13:20) - PLID 23714 - Fill patient demographics
		RefreshPatientDemographics();

		// (c.haag 2008-07-07 11:12) - PLID 30607 - Fill the todo list
		if (!m_bIsTemplate) {
			RefreshTodoList();
		}

		// (j.gruber 2007-01-08 09:48) - PLID 23399 - add Secondary Provider Information
		CArray<long,long> arSecondaryProviderIDs;
		m_pEMN->GetSecondaryProviders(arSecondaryProviderIDs);
		SetSecondaryProviders(arSecondaryProviderIDs);

		// (d.lange 2011-03-23 10:28) - PLID 42136 - add Assistant/Technician
		CArray<long,long> arTechnicianIDs;
		m_pEMN->GetTechnicians(arTechnicianIDs);
		SetTechnicians(arTechnicianIDs);

		// (j.gruber 2009-05-07 17:09) - PLID 33688 - add Other Provider Information
		//CArray<long,long> arOtherProviders;
		//m_pEMN->GetOtherProviders(arOtherProviders);
		
		//TES 6/2/2004: Fill in our list of templates:
		// (j.jones 2006-05-23 15:32) - PLID 20746 - unlike the PIC,
		// we will only filter on EMR templates
		ASSERT(!m_pAddFolderThread); //Initialize() should only be called once.
		GetDlgItem(IDC_TEMPLATE_LIST)->EnableWindow(FALSE);
		m_pTemplateCombo->ComboBoxText = _bstr_t("Loading...");
		m_pAddFolderThread = AddFolderToList("Forms\\EMR", m_pTemplateCombo, true, this);

		//TES 2/2/2006 - Load whether we're read only.
		// (c.haag 2006-03-28 12:21) - Consider user permissions, too
		BOOL bLocked = (m_pEMN->GetStatus() == 2);
		// (a.walling 2007-11-28 11:21) - PLID 28044 - check expiration
		BOOL bCanWriteToEMR = CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);

		// (a.walling 2008-06-04 14:11) - PLID 22049 - Check writable status
		BOOL bIsReadOnly = !(m_pEMN->IsWritable());

		SetReadOnly((bLocked || !bCanWriteToEMR || bIsReadOnly) ? TRUE : FALSE);

		// (z.manning 2013-06-05 10:00) - PLID 56962 - Flag that we're now initialized
		m_bInitialized = TRUE;
	}
	NxCatchAll("Error initializing CEMNMoreInfoDlg");
}

BEGIN_EVENTSINK_MAP(CEMNMoreInfoDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMNMoreInfoDlg)
	ON_EVENT(CEMNMoreInfoDlg, IDC_MEDICATIONS, 6 /* RButtonDown */, OnRButtonDownMedications, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_MEDICATIONS, 9 /* EditingFinishing */, OnEditingFinishingMedications, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMNMoreInfoDlg, IDC_MEDICATIONS, 10 /* EditingFinished */, OnEditingFinishedMedications, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEMNMoreInfoDlg, IDC_MEDICATIONS, 8 /* EditingStarting */, OnEditingStartingMedications, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_PROVIDER, 16 /* SelChosen */, OnSelChosenEmnProvider, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_LOCATION, 16 /* SelChosen */, OnSelChosenEmnLocation, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_CHART, 16 /* SelChosen */, OnSelChosenEmnChart, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_CATEGORY, 16 /* SelChosen */, OnSelChosenEmnCategory, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_STATUS, 16 /* SelChosen */, OnSelChosenEmnStatus, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_PROCEDURE_COMBO, 16 /* SelChosen */, OnSelChosenEmnProcedureCombo, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_PROCEDURE_LIST, 6 /* RButtonDown */, OnRButtonDownEmnProcedureList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_TEMPLATE_LIST, 16 /* SelChosen */, OnSelChosenTemplateList, VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_LOCATION, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmnLocation, VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_PROVIDER, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmnProvider, VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_PROVIDER, 18 /* RequeryFinished */, OnRequeryFinishedEmnProvider, VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_SECONDARY, 18 /* RequeryFinished */, OnRequeryFinishedEmnSecondary, VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_SECONDARY, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmnSecondary, VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_SECONDARY, 16 /* SelChosen */, OnSelChosenEmnSecondary, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_CHART, 18 /* RequeryFinished */, OnRequeryFinishedEmnChart, VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_CATEGORY, 18 /* RequeryFinished */, OnRequeryFinishedEmnCategory, VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_MEDICATIONS, 4 /* LButtonDown */, OnLButtonDownMedications, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_TODO_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmnTodoList, VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_TODO_LIST, 3 /* DblClickCell */, OnDblClickCellEmnTodoList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_TODO_LIST, 6 /* RButtonDown */, OnRButtonDownEmnTodoList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_TECH, 18 /* RequeryFinished */, OnRequeryFinishedEmnTechnician, VTS_I2)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_TECH, 16 /* SelChosen */, OnSelChosenEmnTechnician, VTS_DISPATCH)
	ON_EVENT(CEMNMoreInfoDlg, IDC_EMN_TECH, 20 /* TrySetSelFinished */, OnTrySetSelFinishedEmnTechnician, VTS_I4 VTS_I4)
END_EVENTSINK_MAP()



// (a.walling 2013-05-15 15:38) - PLID 56697 - Helper object to map arbitrary values to sequential generated command IDs for tracking popup menus
namespace Nx {

template<typename ValueType = long>
struct MenuCommandMap
{
	MenuCommandMap()
		: m_nextCmdID(100) // 100 by convention
	{} 
	explicit MenuCommandMap(long initialValue) 
		: m_nextCmdID(initialValue)
	{}

	typedef std::map<long, ValueType> Map;

	long m_nextCmdID;
	Map m_map;

	Map& GetMap()
	{ return m_map; }

	/// lookups
	boost::optional<ValueType> Translate(long cmdID)
	{
		Map::iterator it = m_map.find(cmdID);
		if (it == m_map.end()) {
			return boost::none;
		} else {
			return it->second;
		}
	}

	bool Translate(long cmdID, ValueType& val)
	{
		boost::optional<ValueType> oVal = Lookup(cmdID);
		if (!oVal) {
			return false;
		} else {
			val = *oVal;
			return true;
		}
	}
	
	// returns next command ID for given value; will be unique with each invocation
	// in other words, there are no checks for pre-existing values
	long operator()(ValueType value)
	{ return m_map.insert(std::make_pair(m_nextCmdID++, value)).first->first; }
};

}


void CEMNMoreInfoDlg::OnBtnAddMedication() 
{
	try {

		// Ask the user for a medication to add
		long nMedicationID = -1;

		// (j.jones 2008-05-20 15:39) - PLID 30079 - removed the todo creation code,
		// it is now handled by the prescription editor

		{
			// If we're on an EMN (not an EMN Template) we can relate the prescription to the current 
			// patient.  Thus we have different logic (because we need to check permissions and give a 
			// different-looking prompt)
			if (!m_bIsTemplate) {
				//check to see that they have permissions - this will prompt a password
				if(!CheckCurrentUserPermissions(bioPatientMedication,sptCreate))
					return;

				//TES 5/20/2008 - PLID 30123 - We will pop out a context menu with their "Quick List" medications.
				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu mnu;
				mnu.m_hMenu = CreatePopupMenu();

				//TES 5/20/2008 - PLID 30123 - Pull the Quick List
				CArray<int,int> aryQuickListMeds;
				GetRemotePropertyArray("MedicationsQuickList", aryQuickListMeds, 0, GetCurrentUserName());

				int nCmdId = 0;
				
				//TES 5/20/2008 - PLID 30123 - We need to look up the names.  Note that this will also filter out any bad ids that
				// are in the array (maybe one got deleted), and make sure that it's ordered properly on the menu.
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				_RecordsetPtr rsDrugList = CreateParamRecordset("SELECT DrugList.ID, EmrDataT.Data "
					"FROM DrugList INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID "
					"WHERE EMRDataT.Inactive = 0 AND DrugList.ID IN ({INTARRAY}) ORDER BY Data",
					aryQuickListMeds);
				if(rsDrugList->eof) {
					//TES 5/20/2008 - PLID 30123 - They don't have any in their quick list.  Therefore, we're not going to pop out
					// the menu, but just go with the old behavior (which, as it happens, is the same as they get if they click 
					// "<More Medications...>", which we are assigning ID -2.
					nCmdId = -2;
					
					//TES 5/20/2008 - PLID 30123 - However, let them know about this new feature, otherwise nobody might ever use it.
					// But give them the option to turn it off.
					DontShowMeAgain(this, "For your convenience, you can add commonly prescribed medications to your 'Quick List.'\r\n\r\n"
						"To do this, click the '...' button on the following dialog, and check off the 'Quick List' column for any "
						"medications you would like to have easy access to when prescribing.", "MedicationQuickListSetup");
					
				}
				else {					
					// (a.walling 2013-05-15 15:38) - PLID 56697
					// since CEMNMoreInfoDlg lives within a frame now, the frame handles WM_INITMENUPOPUP and hence will attempt to update
					// the menu items via the command ID. EMR's commands are all >= 40000, and MFC and reserved IDs all higher than that
					// so we need to restrict the range. The most straightforward way to do this is with a map: enter Nx::MenuCommands
					Nx::MenuCommandMap<long> menuCmds;

					//TES 5/20/2008 - PLID 30123 - Go through and add menu items for all the drugs in the quick list.
					while(!rsDrugList->eof) {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, menuCmds(AdoFldLong(rsDrugList, "ID")), 
							AsMenuItem(AdoFldString(rsDrugList, "Data")));
						rsDrugList->MoveNext();
					}
				
					//TES 5/20/2008 - PLID 30123 - Now add the "<More Medications...>" option.
					mnu.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, menuCmds(-2), "<More Medications...>");
					//TES 5/20/2008 - PLID 30123 - And the "<Configure>" option, which takes them to the CEditMedicationsDlg.
					mnu.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
					//TES 5/20/2008 - PLID 30123 - Disable the menu option if they don't have permission.
					DWORD dwEnabled = MF_DISABLED|MF_GRAYED;
					if(GetCurrentUserPermissions(bioPatientMedication) & SPT______0_____ANDPASS) {
						dwEnabled = MF_ENABLED;
					}
					mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, menuCmds(-3), "<Configure...>");
					
					
					//TES 5/20/2008 - PLID 30123 - Now show the menu.
					CRect rc;
					CWnd *pWnd = GetDlgItem(IDC_BTN_ADD_MEDICATION);
					if (pWnd) {
						pWnd->GetWindowRect(&rc);
						nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
					} else {
						CPoint pt;
						GetCursorPos(&pt);
						nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
					}

					//(e.lally 2012-03-27) PLID 49249 - No selection will always be 0
					if(nCmdId == 0) {
						//TES 5/20/2008 - PLID 30123 - They clicked off the menu, do nothing.
						return;
					}

					// (a.walling 2013-05-15 15:38) - PLID 56697
					nCmdId = menuCmds.Translate(nCmdId).get_value_or(-2);
				}

				if(nCmdId == -2) {
					//TES 5/20/2008 - PLID 30123 - They chose "<More Medications...>", just do it the old way.

					//Open the Medication Selection Dialog
					// TODO: (b.cardillo 2004-10-06 14:21) - I notice CMedicationSelectDlg uses 
					// GetActivePatient() to decide what patient to look at which works, but only because 
					// right now you have to be on a certain patient in order to look at that patient's 
					// EMNs.  But probably someday we'll be able to look at various patients' EMNs at once 
					// without switching from patient to patient, so this really should be passing m_PatID 
					// into the CMedicationSelectDlg by a member variable or whatever.
					// (c.haag 2007-03-08 12:47) - PLID 25110 - I've addressed Bob's concern by assigning
					// the patient ID to the medication select dialog
					CMedicationSelectDlg dlg(this);
					dlg.SetPatientID(m_pEMN->GetParentEMR()->GetPatientID());
					long nResult;
					nResult = dlg.DoModal();
					if (nResult == IDOK) {
						// The user chose a medication
						nMedicationID = dlg.m_nMedicationID;
					}
					else {
						// The user canceled
						return;
					}
				}
				else if(nCmdId == -3) {
					//TES 5/20/2008 - PLID 30123 - They chose "<Configure>" so make sure they have permission.
					if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
						return;
					}

					//open the edit medication dialog

					CEditMedicationListDlg dlg(this);
					dlg.DoModal();
					return;
				}
				else {
					//TES 5/20/2008 - PLID 30123 - They chose one of the quick list medications.
					nMedicationID = nCmdId;
				}
				
				if(nMedicationID != -1) {
					//Check that the patient isn't allergic to it.
					if(!CheckAllergies(m_nPatientID, nMedicationID)) {
						return;
					}
				} 
			} else {
				// We're on a mint so just give the user the complete list of all drugs (ignore patient)
				CSelectDlg dlg(this);
				dlg.m_strTitle = "Select Medication";
				dlg.m_strCaption = "Please select the medication you wish to add:";
				// (c.haag 2007-02-02 18:16) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
				// (c.haag 2007-02-02 19:05) - PLID 24565 - We now store the inactive flag in EmrDataT
				dlg.m_strFromClause = "DrugList LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID WHERE EMRDataT.Inactive = 0";
				// Tell it to use the ID column
				int nIdColumnArrayIndex;
				{
					DatalistColumn dcID;
					dcID.strField = "DrugList.ID"; // (c.haag 2007-02-02 18:12) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
					dcID.strTitle = "ID";
					dcID.nWidth = 0;
					dcID.nStyle = csVisible|csFixedWidth;
					dcID.nSortPriority = -1;
					dcID.bSortAsc = TRUE;
					nIdColumnArrayIndex = dlg.m_arColumns.Add(dcID);
				}
				// Tell it to use the Name column
				{
					DatalistColumn dcName;
					dcName.strField = "Data"; // (c.haag 2007-02-02 16:06) - PLID 24561 - We now pull the medication name from EmrDataT
					dcName.strTitle = "Medication";
					dcName.nWidth = -1;
					dcName.nStyle = csVisible|csWidthAuto;
					dcName.nSortPriority = 0;
					dcName.bSortAsc = TRUE;
					dlg.m_arColumns.Add(dcName);
				}
				// Pop up the dialog
				if (dlg.DoModal() == IDOK) {
					// The user made a selection
					nMedicationID = VarLong(dlg.m_arSelectedValues[nIdColumnArrayIndex]);
				} else {
					// The user cancelled
					return;
				}
			}
		}

		if (nMedicationID != -1) {

			//fail if the EMN is still loading, as that should be impossible
			if(m_pEMN->IsLoading()) {
				ThrowNxException("CEMNMoreInfoDlg::OnBtnAddMedication called while the EMN is still loading!");
			}
			
			//Update the memory object.
			// (m.hancock 2006-10-27 10:13) - PLID 21730 - Save a pointer to the medication so we can access properties like drug name.
			// (j.jones 2008-05-20 11:18) - PLID 30079 - this will show the prescription editor first, which is cancelable,
			// which means it may return NULL
			SourceActionInfo saiBlank;
			EMNMedication *pMedication = m_pEMN->AddMedication(nMedicationID, TRUE, TRUE, saiBlank);

			// (a.walling 2007-10-01 09:00) - PLID 27568 - Use the object from the EMN
			//Update the interface

			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now

			// (j.jones 2012-11-14 17:00) - PLID 52819 - The medication pointer is no longer necessary,
			// the new medication now saves immediately. The ID would be -1 if the user ended up deleting it.
			//AddMedication(pEMNMedication);
			long nNewPrescriptionID = -1;
			if(pMedication) {
				nNewPrescriptionID = pMedication->nID;
				delete pMedication;
			}

			if(nNewPrescriptionID != -1) {

				CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
				if(pTreeWnd) {
					// (c.haag 2010-02-17 10:19) - PLID 37384 - Let the user apply the prescriptions to the current medications list.
					// (j.jones 2012-10-11 10:52) - PLID 52946 - don't do this on a template
					// (j.jones 2012-11-16 13:51) - PLID 53765 - we now pass in the prescription ID, not a pointer
					if(!m_bIsTemplate) {
						// (j.jones 2013-01-09 11:55) - PLID 54530 - this is now a treeWnd function
						pTreeWnd->ReconcileCurrentMedicationsWithNewPrescription(m_pEMN, nNewPrescriptionID);

						//if reconciliation changed the current meds table, it would have forced a save
					}

					// (j.jones 2012-09-28 15:45) - PLID 52922 - this function will check their preference
					// to save the EMN and warn about drug interactions				

					// (j.jones 2012-10-22 17:40) - PLID 52819 - must reload medications, if we added one
					m_pEMN->ReloadMedicationsFromData(TRUE);

					// (j.jones 2012-11-13 10:10) - PLID 52869 - changed to be a posted message
					if(!m_bIsTemplate) {
						// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
						m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
					}
				}
			}
		}
	
	}NxCatchAll("Error adding prescription.");
}

void CEMNMoreInfoDlg::OnBtnWritePrescriptions() 
{
	// (j.jones 2005-11-23 16:19) - this shouldn't be called on a template
	if (m_bIsTemplate) {
		return;
	}

	try {
		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(!pTreeWnd) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(m_MedicationList->GetRowCount() == 0) {
			MsgBox("This EMN does not have any Prescriptions to be printed.");
			return;
		}

		// Get template to merge to

		// (j.jones 2008-06-05 11:54) - PLID 29154 - now we support different default templates
		// based on how many prescriptions are printed
		long nCountPrescriptions = m_MedicationList->GetRowCount();
		BOOL bExactCountFound = FALSE;
		BOOL bOtherCountFound = FALSE;
		CString strTemplateName = GetDefaultPrescriptionTemplateByCount(nCountPrescriptions, bExactCountFound, bOtherCountFound);

		if(strTemplateName == "") {
			MsgBox(MB_OK|MB_ICONINFORMATION, 
				"There is no default template set up. The prescription cannot be printed.\n"
				"Please go to the Medications tab to set up your default prescription template.");
			return;
		}
		//if no template was found for the exact count, and there are some for other counts,
		//ask if they wish to continue or not (will use the standard default otherwise)
		else if(!bExactCountFound && bOtherCountFound) {
			CString str;
			str.Format("There is no default template configured for use with %li prescription%s, "
				"but there are templates configured for other counts of prescriptions.\n\n"
				"Would you like to continue merging using the standard prescription template?",
				nCountPrescriptions, nCountPrescriptions == 1 ? "" : "s");
			if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
				AfxMessageBox("The prescription will not be be printed.\n"
					"Please go to the Medications tab to set up your default prescription template.");
				return;
			}
		}

		// (a.walling 2007-02-08 16:55) - PLID 24674 - Get a list of medication IDs
		CDWordArray dwaMedIDs;
		long p = m_MedicationList->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{
			m_MedicationList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			// (b.savon 2014-12-03 08:49) - PLID 64142 - Display a message to the user if they try to merge a NewCrop prescription to Word from within Nextech
			if (!AsString(pRow->GetValue(mclNewCropGUID)).IsEmpty()){
				MessageBox(
					"At least 1 prescription selected was written in NewCrop.  If you wish to print the NewCrop prescription, please do so from NewCrop.  "
					"If you wish to print the non-NewCrop prescription, please do so from the patient's medications tab.", "Nextech", MB_ICONEXCLAMATION);
				return;
			}

			dwaMedIDs.Add((DWORD)VarLong(pRow->GetValue(mclMedicationID)));

			pDisp->Release();
		}

		// (a.walling 2007-02-08 16:57) - PLID 24674 - Now call CheckAllAllergies to warn the user if necessary
		// and return if they choose to cancel and review.
		if (dwaMedIDs.GetSize() > 0) {
			if (!CheckAllAllergies(m_nPatientID, &dwaMedIDs))
				return;
		}

		//TES 6/17/2008 - PLID 30411 - The button says "Save and Print Prescriptions", so this is just a totally 
		// unnecessary annoyance to the user.
		//AfxMessageBox("Your changes to this EMR will first be saved before writing prescriptions.");

		//TES 6/17/2008 - PLID 30411 - Check whether they want to send it straight to the printer or not.
		BOOL bDirectToPrinter = IsDlgButtonChecked(IDC_PRINT_PRESCRIPTIONS_TO_PRINTER);
		//TES 6/17/2008 - PLID 30414 - Check our preference to decide what we want to tell ou rparent to do.
		BOOL bCloseEmr = GetRemotePropertyInt("CloseEmrWhenPrintingPrescriptions", 0, 0, GetCurrentUserName(), true);
		if(bCloseEmr) {
			pTreeWnd->GetParent()->SendMessage(NXM_EMR_PRINT_PRESCRIPTIONS_AND_CLOSE, (WPARAM)m_pEMN, (LPARAM)bDirectToPrinter);
		}
		else {
			pTreeWnd->GetParent()->SendMessage(NXM_EMR_PRINT_PRESCRIPTIONS, (WPARAM)m_pEMN, (LPARAM)bDirectToPrinter);
		}

	}NxCatchAll("Error writing prescriptions.");
}

void CEMNMoreInfoDlg::OnRButtonDownMedications(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu pMenu;
		m_MedicationList->CurSel = nRow;
		
		if(nRow != -1) {
			pMenu.CreatePopupMenu();

			IRowSettingsPtr pRow = m_MedicationList->GetRow(nRow);
			EMNMedication *pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));

			// (j.jones 2008-07-22 12:45) - PLID 30792 - add ability to add problems, and view problem information
			if (!m_bIsTemplate) {
				// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
				// if the EMN is not writeable
				pMenu.AppendMenu(MF_STRING|MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), ID_ADD_MED_PROBLEM, "Link with New &Problem");
				// (c.haag 2009-05-28 14:56) - PLID 34312 - Option for linking with existing problems
				CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
				if (NULL != pEMR) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), ID_LINK_MED_PROBLEMS, "Link with Existing Problems");
				}
				if (pMed->HasProblems()) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION, ID_EDIT_MED_PROBLEM, "Update Problem &Information");
				}
				pMenu.AppendMenu(MF_SEPARATOR);
			}

			if((GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS)) {
				
				// (a.walling 2009-04-28 13:48) - PLID 34046 - Always allow this even for e-prescriptions, it will be handled in the actual
				// deletion code since it requires a db access to know whether this was successfully prescribed or not.
				pMenu.AppendMenu(MF_BYPOSITION, ID_DELETE_PRESCRIPTION, "Delete Prescription");
			}

			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
			pMenu.DestroyMenu();
		}
	} NxCatchAll("CEMNMoreInfoDlg::OnRButtonDownMedications");
}

void CEMNMoreInfoDlg::OnEditingFinishingMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(*pbCommit == FALSE)
			return;

		// (j.jones 2012-11-21 15:37) - PLID 52819 - currently this feature is completely disabled,
		// you are now required to edit the prescription through the dialog popup only

		/*

		if(!CheckCurrentUserPermissions(bioPatientMedication,sptWrite)) {
			// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
			VariantClear(pvarNewValue);
			*pvarNewValue = _variant_t(varOldValue).Detach();
			*pbCommit = FALSE;
			return;
		}

		bool bEmnModified = false;
		switch(nCol) {
		case mclExplanation:
			//Description
			{
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();

				//update the CEMN object
				// (a.walling 2007-10-01 08:49) - PLID 27568 - Update the object via our stored pointer
				EMNMedication* pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
				if (pMed) {
					pMed->strPatientExplanation = strTemp;
					// (j.jones 2010-05-07 11:05) - PLID 36062 - calculate the EnglishDescription
					pMed->strEnglishDescription = GetLatinToEnglishConversion(pMed->strPatientExplanation);
					// (a.walling 2009-04-22 12:50) - PLID 34044 - Set as modified
					pMed->bChanged = TRUE;
					bEmnModified = true;
				} else {
					ASSERT(FALSE);
					ThrowNxException("Failed to get a valid medication object!");
				}
			}
			break;

		case mclRefills:
			{
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(strTemp.IsEmpty()) {
					//Don't commit.
					MsgBox("You cannot enter a blank amount for refills.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
				if(strTemp.GetLength() > 50) {
					MsgBox("The refills amount cannot have more than 50 characters");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}

				//update the CEMN object
				// (a.walling 2007-10-01 08:49) - PLID 27568 - Update the object via our stored pointer
				EMNMedication* pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
				if (pMed) {
					pMed->strRefillsAllowed = strTemp;
					// (a.walling 2009-04-22 12:50) - PLID 34044 - Set as modified
					pMed->bChanged = TRUE;
					bEmnModified = true;
				} else {
					ASSERT(FALSE);
					ThrowNxException("Failed to get a valid medication object!");
				}
			}
			break;

		case mclPills:
			{
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(strTemp.IsEmpty()) {
					//Don't commit.
					//TES 2/11/2009 - PLID 33034 - Renamed PillsPerBottle to Quantity
					MsgBox("You cannot enter a blank amount for Quantity.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
				if(strTemp.GetLength() > 50) {
					//TES 2/11/2009 - PLID 33034 - Renamed PillsPerBottle to Quantity
					MsgBox("The Quantity cannot have more than 50 characters");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}

				//update the CEMN object
				// (a.walling 2007-10-01 08:49) - PLID 27568 - Update the object via our stored pointer
				EMNMedication* pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
				if (pMed) {
					pMed->strQuantity = strTemp;
					// (a.walling 2009-04-22 12:50) - PLID 34044 - Set as modified
					pMed->bChanged = TRUE;
					bEmnModified = true;
				} else {
					ASSERT(FALSE);
					ThrowNxException("Failed to get a valid medication object!");
				}
			}
			break;

		//TES 2/12/2009 - PLID 33034 - Added Strength
		//TES 3/31/2009 - PLID 33750 - Removed Strength

		case mclUnit:
			//Unit
			{
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(strTemp.IsEmpty()) {
					//Don't commit.
					MsgBox("You cannot enter a blank unit description");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
				
				//update the CEMN object
				// (a.walling 2007-10-01 08:49) - PLID 27568 - Update the object via our stored pointer
				EMNMedication* pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
				if (pMed) {
					pMed->strUnit = strTemp;
					// (a.walling 2009-04-22 12:50) - PLID 34044 - Set as modified
					pMed->bChanged = TRUE;
					bEmnModified = true;
				} else {
					ASSERT(FALSE);
					ThrowNxException("Failed to get a valid medication object!");
				}
			}
			break;

		//TES 2/12/2009 - PLID 33034 - Added Dosage Form
		//TES 3/31/2009 - PLID 33750 - Removed Dosage Form
		}
		if(bEmnModified) {
			//Tell our parent, so it can update its interface.
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface
			m_pEMN->SetMoreInfoUnsaved();
		}

		*/

	} NxCatchAll("CEMNMoreInfoDlg::OnEditingFinishingMedications");
}

void CEMNMoreInfoDlg::OnEditingFinishedMedications(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	
}

// (a.walling 2009-04-22 13:41) - PLID 33948 - Disable editing EPrescribe meds except via the editor
void CEMNMoreInfoDlg::OnEditingStartingMedications(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{	
	try {
		if(nRow == -1)
			return;

		// (j.jones 2012-11-21 15:37) - PLID 52819 - currently this feature is completely disabled,
		// you are now required to edit the prescription through the dialog popup only
		*pbContinue = FALSE;
		return;

		/*
		EMNMedication* pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
		if (pMed) {
			if (pMed->nID != -1) {

				// (j.jones 2009-04-22 16:49) - PLID 33736 - make sure if its new crop, its read only
				if(!pMed->strNewCropGUID.IsEmpty()) {
					//silently disable this, they will get a message if they try to open the prescription dialog
					*pbContinue = FALSE;
				}
				else if (pMed->bEPrescribe) {
					*pbContinue = FALSE;			
					DontShowMeAgain(this, "To edit an electronically prescribed medication, please click on the medication name to open the editor dialog.",
						"EPrescribeMoreInfoEdit", "Electronic Prescriptions");
				}
			}

		} else {
			ASSERT(FALSE);
			ThrowNxException("Failed to get a valid medication object!");
		}
		*/

	} NxCatchAll("CEMNMoreInfoDlg::OnEditingStartingMedications");
}


BOOL CEMNMoreInfoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	 if(wParam == ID_DELETE_PRESCRIPTION) {
		try {
			if(m_MedicationList->GetCurSel() != -1) {
				if (CheckCurrentUserPermissions(bioPatientMedication, sptDelete)) {
					// (a.walling 2007-10-01 08:55) - PLID 27568 - Remove via the object pointer.
					//long nID = m_MedicationList->GetValue(m_MedicationList->GetCurSel(),mclID).lVal;
					//long nMedicationID = m_MedicationList->GetValue(m_MedicationList->GetCurSel(),mclMedicationID).lVal;
					EMNMedication* pMed = (EMNMedication*)VarLong(m_MedicationList->GetValue(m_MedicationList->GetCurSel(),mclObjectPtr));

					if (pMed) {
						// (c.haag 2008-07-25 16:14) - PLID 30826 - Don't let the user delete the medication
						// if it is tied to saved problems and the user doesn't have permissions
						if (!CanCurrentUserDeleteEmrProblems()) {
							if (m_pEMN->DoesMedicationHaveSavedProblems(pMed)) {
								MsgBox(MB_OK | MB_ICONERROR, "You may not delete this prescription because it is associated with one or more saved problems.");
								return CNxDialog::OnCommand(wParam, lParam);			
							}
						}

						// (j.jones 2008-07-29 14:14) - PLID 30729 - see if there are any problems at all
						BOOL bHasProblems = pMed->HasProblems();
						
						// (a.walling 2009-04-28 13:47) - PLID 34046 - Check for template
						if (pMed->nID != -1 && !m_bIsTemplate) {
							// (c.haag 2009-07-02 11:57) - PLID 34102 - Don't discriminate on the message type being mtPendingRx. Just don't allow the deletion
							// if the prescription exists in the SureScripts table.
							// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
							// (j.jones 2012-10-29 14:45) - PLID 53259 - also cannot delete if the prescription status is E-Prescribed
							if(ReturnsRecordsParam("SELECT TOP 1 SureScriptsMessagesT.ID "
								"FROM SureScriptsMessagesT "
								"WHERE PatientMedicationID = {INT} "
								"UNION SELECT TOP 1 PatientMedications.ID "
								"FROM PatientMedications "
								"WHERE PatientMedications.ID = {INT} AND PatientMedications.QueueStatus IN ({SQL})",
								pMed->nID, pMed->nID, GetERxStatusFilter())) {

								// (a.walling 2009-04-24 13:53) - PLID 34046 - Prevent deleting entirely
								// (b.savon 2013-09-23 07:31) - PLID 58486 - Changed the wording
								// (b.eyers 2016-02-05) - PLID 67980 - added dispensed in house
								MessageBox("This prescription cannot be deleted because it has been printed, voided, electronically prescribed, or dispensed in-house.", NULL, MB_ICONSTOP);

								return CNxDialog::OnCommand(wParam, lParam);
							}
						}

						CString strMsg;
						strMsg += "Are you sure you want to remove this prescription?";

						if (IDYES == MsgBox(MB_YESNO, strMsg)) {

							//remove from the CEMN object
							// (a.walling 2007-10-01 08:57) - PLID 27568 - by pointer, rather than by drugid.
							// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
							m_pEMN->RemoveMedication(pMed);

							//remove from the datalist
							m_MedicationList->RemoveRow(m_MedicationList->GetCurSel());

							// (j.jones 2008-07-22 09:26) - PLID 30792 - try to hide the problem icon column
							ShowHideProblemIconColumn_MedicationList();
						}

						// (j.jones 2008-07-29 14:15) - PLID 30729 - if we has problems, send a message
						// stating that they changed
						if(bHasProblems) {
							GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);
						}

					} else {
						ASSERT(FALSE);
						ThrowNxException("Failed to get valid medication object!");
					}
				}
			}
		} NxCatchAll("CEMNMoreInfoDlg::OnCommand:ID_DELETE_PRESCRIPTION");
	}
	
	// (c.haag 2008-07-08 17:15) - PLID 30607 - Filters for the todo list
	else if (IDC_RADIO_MORE_INFO_TODO_INCOMPLETE == wParam ||
		IDC_RADIO_MORE_INFO_TODO_COMPLETED == wParam ||
		IDC_RADIO_MORE_INFO_TODO_ALL == wParam)
	{
		RefreshTodoList();
	}
	// (j.jones 2008-07-22 13:06) - PLID 30792 - added problem options


	else if(wParam == ID_ADD_MED_PROBLEM) {

		try {

			if(m_MedicationList->GetCurSel() != -1) {
				
				IRowSettingsPtr pRow = m_MedicationList->GetRow(m_MedicationList->GetCurSel());
				EMNMedication* pMedication = (EMNMedication*)VarLong(pRow->GetValue(mclObjectPtr));

				if(pMedication == NULL) {
					ThrowNxException("Add Medication Problem - failed because no medication object was found!");
				}

				if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
				// this is independent of locking
				if(!m_pEMN->IsWritable()) {
					//prompt, but do not attempt to take access, let the user decide what to do
					AfxMessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				CEMRProblemEditDlg dlg(this);

				const BOOL bEMNIsLocked = (m_pEMN->GetStatus() == 2) ? TRUE : FALSE;
				if (bEMNIsLocked) {
					dlg.SetWriteToData(TRUE); // This will force the dialog to save the problem details to data
				}

				// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
				// (j.jones 2009-05-21 16:42) - PLID 34250 - renamed this function
				dlg.AddLinkedObjectInfo(-1, eprtEmrMedication, pMedication->m_strDrugName, "", pMedication->nID);
				dlg.SetPatientID(m_pEMN->GetParentEMR()->GetPatientID());

				if (IDOK == dlg.DoModal()) {
					if (!dlg.ProblemWasDeleted()) {

						COleDateTime dtInvalid;
						dtInvalid.SetStatus(COleDateTime::invalid);

						// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
						// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
						// (c.haag 2009-05-16 14:21) - PLID 34312 - Create a new problem and problem link
						CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
						if (NULL != pEMR) {

							// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
							long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
							dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

							// (z.manning 2009-05-27 10:14) - PLID 34297 - Added patient ID
							// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
							// (s.tullis 2015-02-23 15:44) - PLID 64723
							// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
							CEmrProblem *pProblem = pEMR->AllocateEmrProblem(dlg.GetProblemID(), pEMR->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
								dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(), dlg.GetProblemDoNotShowOnCCDA(),
								dlg.GetProblemDoNotShowOnProblemPrompt());
							CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtEmrMedication, pMedication->nID, -1);
							pNewLink->UpdatePointersWithMedication(m_pEMN, pMedication);
							pMedication->m_apEmrProblemLinks.Add(pNewLink);
							// (c.haag 2009-05-16 14:23) - PLID 34312 - Release the reference that pProblem has
							// to the problem. Only the detail and the EMR (which tracks all the problems it created)
							// should own a reference.
							pProblem->Release();
						} else {
							ThrowNxException("Could not create a new problem because there is no valid EMR");
						}

						//update the icon
						_variant_t varProblemIcon = g_cvarNull;
						if(pMedication->HasOnlyClosedProblems()) {
							HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
							varProblemIcon = (long)hProblem;
						}
						else {
							HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
							varProblemIcon = (long)hProblem;
						}
						pRow->PutValue(mclProblemIcon, varProblemIcon);

						ShowHideProblemIconColumn_MedicationList();

						if(!m_pEMN->IsLockedAndSaved()) {

							// (j.jones 2013-01-07 16:45) - PLID 52819 - this should not be needed, we should still
							// mark more info as changed, but we don't need to flag the medication as having changed
							//pMedication->bChanged = TRUE;

							// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
							m_pEMN->SetMoreInfoUnsaved();
						}
						// (c.haag 2009-06-01 13:17) - PLID 34312 - We need to flag the EMR as unsaved because it is now the "manager" of problems
						pEMR->SetUnsaved();

						// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
						GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);
					}
				}
			}

		} NxCatchAll("CEMNMoreInfoDlg::OnCommand:ID_ADD_MED_PROBLEM");
	}
	else if(wParam == ID_EDIT_MED_PROBLEM) {

		EditMedicationProblems();
	}
	else if (wParam == ID_LINK_MED_PROBLEMS) {
		// (c.haag 2009-05-28 16:11) - PLID 34312 - Link with existing problems
		LinkMedicationProblems();
	}
	return CNxDialog::OnCommand(wParam, lParam);
}

HBRUSH CEMNMoreInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}


BOOL CEMNMoreInfoDlg::AddProcedure(long nProcedureID, CString strName)
{
	//Make sure it's not already in the list
	if(m_pProcedureList->FindByColumn(0, nProcedureID, NULL, FALSE)) {
		return FALSE;
	}

	//Add it to the list.
	//TES 6/20/2006 - Since we've got the name anyway, looking it up in the combo doesn't seem to save any time, and, if the
	//combo happens to be requerying, actually slows us down.  So, I'm taking that out.
	/*NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureCombo->FindByColumn(0, nProcedureID, NULL, FALSE);
	if(pRow) {
		m_pProcedureList->AddRowAtEnd(pRow, NULL);
	}
	else {
		//It may be an inactive procedure.*/
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->GetNewRow();
		pRow->PutValue(0, nProcedureID);
		pRow->PutValue(1, _bstr_t(strName));
		m_pProcedureList->AddRowAtEnd(pRow,NULL);
	//}

	return TRUE;
}

BOOL CEMNMoreInfoDlg::LookupProcedureNameFromCombo(IN long nProcedureID, OUT CString& strProcName)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureCombo->FindByColumn(pccID, nProcedureID, NULL, FALSE);
	if(pRow) {
		strProcName = VarString(pRow->GetValue(pccName));
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void CEMNMoreInfoDlg::RemoveProcedure(long nProcedureID)
{
	try {
		//Remove from the screen.
		// (c.haag 2008-07-31 12:42) - PLID 30905 - Check if the index is -1 before trying to actually
		// remove it. Silently fail if it doesn't exist; there's nothing to do in that case.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->FindByColumn(0, nProcedureID, NULL, FALSE);
		if (NULL != pRow) {
			m_pProcedureList->RemoveRow(pRow);
		}
	} NxCatchAll("Error in RemoveProcedure");
}

// (a.walling 2007-08-06 12:07) - PLID 23714 - Refresh the procedure list
void CEMNMoreInfoDlg::RefreshProcedureList()
{
	try {
		m_pProcedureList->Clear();

		for(int i = 0; i < m_pEMN->GetProcedureCount(); i++) {
			EMNProcedure *pProc = m_pEMN->GetProcedure(i);
			AddProcedure(pProc->nID, pProc->strName);
		}
	} NxCatchAll("Error refreshing procedure list");
}


// (a.walling 2007-08-06 13:21) - PLID 23714 - Refresh patient demographics
void CEMNMoreInfoDlg::RefreshPatientDemographics()
{
	try {
		// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization - use NxSetWindowText
		CString strPatientName;
		strPatientName.Format("%s, %s %s", m_pEMN->GetPatientNameLast(), m_pEMN->GetPatientNameFirst(), m_pEMN->GetPatientNameMiddle());
		WindowUtils::NxSetWindowText(GetDlgItem(IDC_EMN_PATIENT_NAME), strPatientName);
		
		// (z.manning 2010-01-13 12:14) - PLID 22672 - Age is now a string
		WindowUtils::NxSetWindowText(GetDlgItem(IDC_EMN_AGE), m_pEMN->GetPatientAge());
		CString strPatientGender;
		if(m_pEMN->GetPatientGender() == 1) strPatientGender = "Male";
		else if(m_pEMN->GetPatientGender() == 2) strPatientGender = "Female";
		WindowUtils::NxSetWindowText(GetDlgItem(IDC_EMN_GENDER), strPatientGender);
	} NxCatchAll("Error refreshing patient demographics");
}

// (c.haag 2008-07-07 12:59) - PLID 30607 - Whenever we query for EMR todo's, we use the SQL returned here
// (c.haag 2008-07-11 14:50) - PLID 30689 - Also factor in the regarding type when calculating the note
CString CEMNMoreInfoDlg::GetTodoSelectSql()
{
	return FormatString("SELECT TaskID AS ID, RegardingID AS DetailID, "
			"dbo.GetTodoAssignToNamesString(TaskID) AS AssignToNames, "
			"dbo.GetTodoAssignToIDString(TaskID) AS AssignToIDs, "
			"Deadline, Done, Priority, RegardingType, "
			"/* This big crazy conditional is TRUE if the note appears to be properly formatted in "
			"the form EMN: ... Detail:... If it is, we can just strip that part out of the note because "
			"the information it gives us is redundant */ "
			"CASE WHEN RegardingType = %d THEN "
				"CASE WHEN CHARINDEX(char(10), Notes) > 0 "
					"AND CHARINDEX(char(10), Notes, CHARINDEX(char(10), Notes) + 1) > 0 "
					"AND Len(Notes) > 3 "
					"AND Left(Notes,4) = 'EMN:' "
					"AND SUBSTRING(Notes, CHARINDEX(char(10), Notes) + 1, 7) = 'Detail:' "
				"THEN "
					"Right(Notes, Len(Notes) - CHARINDEX(char(10), Notes, CHARINDEX(char(10), Notes) + 1)) "
				"ELSE "
					"Notes "
				"END "
			"ELSE "
				"CASE WHEN CHARINDEX(char(10), Notes) > 0 "
					"AND Len(Notes) > 3 "
					"AND Left(Notes,4) = 'EMN:' "
				"THEN "
					"Right(Notes, Len(Notes) - CHARINDEX(char(10), Notes)) "
				"ELSE "
					"Notes "
				"END "
			"END "
			"AS Notes "
			"FROM TodoList "
			,ttEMNDetail);
}

// (c.haag 2008-07-08 17:42) - PLID 30607 - Returns additional filters for any visible todo list query
CString CEMNMoreInfoDlg::GetTodoFilterSql()
{
	CString strFilter;

	// Filter on completed/incompleted todos
	if (m_radioTodoIncomplete.GetCheck()) {
		strFilter += FormatString(" AND Done IS NULL ");
	}
	else if (m_radioTodoCompleted.GetCheck()) {
		strFilter += FormatString(" AND Done IS NOT NULL ");
	}

	// (c.haag 2008-07-11 09:20) - PLID 17244 - Do not filter on permissions. Having EMR read
	// permissions are enough here.

	return strFilter;
}

// (c.haag 2008-07-07 13:14) - PLID 30607 - Returns the detail name corresponding to an EMN detail todo
CString CEMNMoreInfoDlg::GetEMNTodoDetailName(long nTaskID, long nRegardingID)
{
	CEMNDetail* pDetail = NULL;
	if (nRegardingID > 0) {
		// This todo corresponds to a detail existing in data. Keep in mind that if the user
		// deleted or changed the detail, then the todo is retroactively updated.
		pDetail = m_pEMN->GetDetailByID(nRegardingID);

	} else {
		// (c.haag 2008-07-14 13:34) - PLID 30696 - This todo corresponds to an unsaved detail. 
		// We have to traverse the created todo list to get the detail information.
		ASSERT(nTaskID > 0);
		CArray<EMNTodo*,EMNTodo*> apTodos;
		m_pEMN->GenerateCreatedTodosWhileUnsavedList(apTodos);
		const int nTodos = apTodos.GetSize();
		for (int i=0; i < apTodos.GetSize() && NULL == pDetail; i++) {
			EMNTodo* pTodo = apTodos[i];
			if (pTodo->nTodoID == nTaskID) {
				pDetail = pTodo->sai.pSourceDetail;
			}						
		}
	}

	if (NULL != pDetail) {
		// Found the detail
		return pDetail->GetLabelText();

	} else {
		// The detail was deleted but the todo still somehow exists
		ASSERT(FALSE);
		return "";
	}
}

// (c.haag 2008-07-07 11:10) - PLID 30607 - Do a full refresh of the EMN todo list. This
// is a synchronous operation.
void CEMNMoreInfoDlg::RefreshTodoList()
{
	try {
		// First, generate the SQL statement. This is a function of whether the EMN is locked.
		// If it's unlocked, the todos for this EMN are 1:1 with patient todo's whose type is
		// related to EMNs. If it's locked, we have to pull from a special table.
		//
		// (c.haag 2008-07-14 13:35) - PLID 30696 -  Additionally, we have to factor in todo's
		// saved in data, but not associated to saved details. We fetch them from the EMN's created
		// todo array.
		//

		// Don't requery if this is a template
		if (m_bIsTemplate) {
			return;
		}

		CArray<EMNTodo*,EMNTodo*> apCreatedTodos;
		CArray<long,long> anCreatedTodoIDs;
		m_pEMN->GenerateCreatedTodosWhileUnsavedList(apCreatedTodos);
		CString strFrom;

		int nCreatedTodos = apCreatedTodos.GetSize();
		for (int i=0; i < nCreatedTodos; i++) {
			anCreatedTodoIDs.Add( apCreatedTodos[i]->nTodoID );
		}

		if(m_pEMN->GetID() != -1) {

			// Handle for existing EMN's
			if (m_pEMN->GetStatus() == 2) {

				// (c.haag 2008-07-07 21:04) - PLID 30632 - Locked. Pull from the special locked table.
				// (c.haag 2008-07-11 13:17) - PLID 30689 - Also filter on EMN ID
				strFrom.Format(
					"(SELECT NULL AS ID, EMRMasterID AS EMNID, EMRDetailID AS DetailID, Assignees AS AssignToNames, dbo.GetEMRLockedTodoAssignToIDString(EMRLockedTodosT.ID) AS AssignToIDs, "
					"Deadline, Done, Notes, NULL AS Priority, RegardingType "
					"FROM EMRLockedTodosT "
					"WHERE ("
					"EMRDetailID IN (SELECT ID FROM EMRDetailsT WHERE Deleted = 0 AND EmrID = %d) "
					"OR "
					"EMRMasterID = %d "
					") "
					"%s ) SubQ",
					m_pEMN->GetID(), m_pEMN->GetID(), GetTodoFilterSql());

			} else {

				// Unlocked. Pull from the todo list
				// (c.haag 2008-07-11 13:21) - PLID 30689 - Also filter on EMN ID for EMN-specific tasks
				strFrom.Format(
					"(%s WHERE (1=1) %s AND ("
					"(RegardingType = %d AND RegardingID IN (SELECT ID FROM EmrDetailsT WHERE Deleted = 0 AND EmrID = %d)) "
					"OR (RegardingType = %d AND RegardingID = %d) "
					"OR (TaskID IN (%s))) ) SubQ",
					GetTodoSelectSql(), GetTodoFilterSql(), 
					(long)ttEMNDetail, m_pEMN->GetID(),
					(long)ttEMN, m_pEMN->GetID(),
					ArrayAsString(anCreatedTodoIDs, false)
					);
			}
		}
		else {
			// This is a new EMN. Even though todo's may exist for this EMN, they don't have
			// valid regarding ID's because the details haven't been saved yet. Just clear the
			// list
			if (0 == anCreatedTodoIDs.GetSize()) {

				// The list is empty. Just clear the list and quit
				m_pTodoList->Clear();
				return;
			}
			// (c.haag 2008-07-11 13:24) - PLID 30689 - Also filter on EMN-specific tasks
			strFrom.Format(
				"(%s WHERE RegardingType IN (%d,%d) %s AND TaskID IN (%s) ) SubQ",
				GetTodoSelectSql(), ttEMNDetail, ttEMN, GetTodoFilterSql(), ArrayAsString(anCreatedTodoIDs, false)
				);
		}

		// Set the from clause
		m_pTodoList->FromClause = _bstr_t(strFrom);

		// Now we can requery the list
		m_pTodoList->Requery();
	}
	NxCatchAll("Error in CEMNMoreInfoDlg::RefreshTodoList");
}

// (c.haag 2008-07-07 12:55) - PLID 30607 - Refreshes a single row in the todo list
void CEMNMoreInfoDlg::RefreshTodoListItem(long nTaskID)
{
	try {
		// (c.haag 2008-07-07 20:20) - 30632 - We can't refresh locked EMN's
		if (m_pEMN->GetStatus() == 2) {
			return;
		}

		// Don't update if this is a template
		if (m_bIsTemplate) {
			return;
		}

		// We must know whether this is a todo for this EMN. Create a recordset to figure it out
		_RecordsetPtr prs = NULL;
		if (nTaskID > 0) {
			// We have a specific task

			// (c.haag 2008-07-08 17:42) - PLID 30607 - Get the base filters that are used when requerying the
			// list as a whole
			// (c.haag 2008-07-11 13:24) - PLID 30689 - Also include EMN-specific todos
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			CSqlFragment sqlParamSql("{CONST_STR} WHERE RegardingType IN ({CONST},{CONST}) AND TaskID = {INT} {CONST_STR} ",
				GetTodoSelectSql(), ttEMNDetail, ttEMN, nTaskID, GetTodoFilterSql());

			// Work the filter more thoroughly
			CArray<EMNTodo*,EMNTodo*> apCreatedTodos;
			CArray<long,long> anCreatedTodoIDs;
			CString strFrom;
			m_pEMN->GenerateCreatedTodosWhileUnsavedList(apCreatedTodos);

			int nCreatedTodos = apCreatedTodos.GetSize();
			int i;

			for (i=0; i < nCreatedTodos; i++) {
				anCreatedTodoIDs.Add( apCreatedTodos[i]->nTodoID );
			}
			BOOL bRunQuery = TRUE;

			if (m_pEMN->GetID() > 0) {
				// Existing EMN -- filter on the EMN details
				// (c.haag 2008-07-11 13:26) - PLID 30689 - And filter on EMN tasks
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				sqlParamSql += CSqlFragment("AND ("
					"(RegardingType = {CONST} AND RegardingID IN (SELECT ID FROM EmrDetailsT WHERE Deleted = 0 AND EmrID = {INT})) "
					"OR (RegardingType = {CONST} AND RegardingID = {INT}) "
					"OR (TaskID IN ({INTARRAY})))",
					ttEMNDetail, m_pEMN->GetID(),
					ttEMN, m_pEMN->GetID(),
					anCreatedTodoIDs);
			} else {
				// New EMN; only filter on the todo ID. We can do that right here.
				BOOL bFound = FALSE;
				for (i=0; i < nCreatedTodos && !bFound; i++) {
					if (anCreatedTodoIDs[i] == nTaskID) {
						bFound = TRUE;
					}
				}
				// This task definitely doesn't belong in the list because we're a new EMN
				// and it's not in our non-regarding todo list
				if (!bFound) {
					bRunQuery = FALSE;
				}
			}

			if (bRunQuery) {
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				prs = CreateParamRecordset("{SQL}", sqlParamSql);
				if (prs->eof) {
					prs->Close();
					prs.Release();
				}
			}
		} else {
			// If we get here, it means the task is for all todo alarms. Refresh the entire todo list.
			RefreshTodoList();
			return;
		}

		// Find the row in the list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTodoList->FindByColumn(tlcID, nTaskID, NULL, VARIANT_FALSE);
		BOOL bNew = FALSE;

		// Ensure the row does not exist if the task no longer exists in data
		if (NULL == prs) {
			if (NULL != pRow) {
				m_pTodoList->RemoveRow(pRow);
			}
			return;
		}

		// Add the row if it doesn't exist
		if (NULL == pRow) {
			pRow = m_pTodoList->GetNewRow();
			bNew = TRUE;
		}

		// Now update the row
		pRow->Value[tlcID] = nTaskID;
		pRow->Value[tlcDetailID] = prs->Fields->Item["DetailID"]->Value;
		// (c.haag 2008-07-11 15:49) - PLID 30689 - If the regarding type is that of a detail, get the detail name. Otherwise,
		// leave blank.
		if (ttEMNDetail == (TodoType)AdoFldLong(prs, "RegardingType")) {
			pRow->Value[tlcDetailName] = _bstr_t(GetEMNTodoDetailName(nTaskID, AdoFldLong(prs, "DetailID")));
		} else {
			pRow->Value[tlcDetailName] = "";
		}
		pRow->Value[tlcAssignees] = prs->Fields->Item["AssignToNames"]->Value;
		// (z.manning 2009-09-14 09:22) - PLID 32048 - Need to set assignee IDs
		pRow->PutValue(tlcAssigneeIDs, prs->GetFields()->GetItem("AssignToIDs")->GetValue());
		pRow->Value[tlcDeadline] = prs->Fields->Item["Deadline"]->Value;
		pRow->Value[tlcNotes] = prs->Fields->Item["Notes"]->Value;
		pRow->Value[tlcDone] = prs->Fields->Item["Done"]->Value;
		pRow->Value[tlcPriority] = prs->Fields->Item["Priority"]->Value;

		// Add the row if it doesn't exist
		if (bNew) {
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pTodoList->AddRowSorted(pRow, NULL);
			pRow = pNewRow;
		}

		// Colorize the new row
		if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1 && m_pEMN->GetStatus() != 2) {
			if (NULL != pRow) {
				ColorizeEMNTodoListItem(pRow);
			}
		}
	}
	NxCatchAll("Error in CEMNMoreInfoDlg::RefreshTodoListItem");
}

// (c.haag 2008-07-07 13:53) - PLID 30607 - Removes an EMN todo list item
void CEMNMoreInfoDlg::RemoveTodoListItem(long nTaskID)
{
	// (c.haag 2008-07-07 20:20) - 30632 - We can't change locked EMN's
	if (m_pEMN->GetStatus() == 2) {
		ASSERT(false);
		return;
	}

	// We should never get here as a template, but check anyway
	if (m_bIsTemplate) {
		return;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTodoList->FindByColumn(tlcID, nTaskID, NULL, VARIANT_FALSE);
	if (NULL != pRow) {
		m_pTodoList->RemoveRow(pRow);
	}
}


// (a.walling 2007-10-01 09:09) - PLID 27568 - Use a pointer to a EMNMedication
void CEMNMoreInfoDlg::AddMedication(EMNMedication* pMed)
{
	if (pMed == NULL) {
		ASSERT(FALSE);
		ThrowNxException("CEMNMoreInfoDlg::AddMedication called with NULL pointer!");
	}

	// (a.walling 2007-10-01 09:10) - PLID 27568 - Optimization - There is no reason we need to query this data,
	// it will already exist in the EMN Medication structure!

	//insert the medication into PatientMedications
	/*
	// (c.haag 2007-02-02 18:11) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
	_RecordsetPtr rs = CreateRecordset("SELECT EMRDataT.Data AS Name, Description, DefaultRefills, DefaultPills, Unit FROM DrugList "
		"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
		"WHERE DrugList.ID = %li", nMedicationID);
	COleVariant varNull;
	varNull.vt = VT_NULL;
	FieldsPtr fields;
	fields = rs->Fields;

	//Put the variables from the recordset into local variables
	CString strName, strDescription, strUnit, strPills, strRefills;

	strName = VarString(fields->Item["Name"]->Value);
	strDescription = VarString(fields->Item["Description"]->Value);
	strRefills = VarString(fields->Item["DefaultRefills"]->Value);
	strPills = VarString(fields->Item["DefaultPills"]->Value);
	strUnit = AdoFldString(fields, "Unit");			

	rs->Close();
	*/

	// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
	_variant_t varProblemIcon = g_cvarNull;			
	if(pMed->HasProblems()) {
		if(pMed->HasOnlyClosedProblems()) {
			HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
			varProblemIcon = (long)hProblem;
		}
		else {
			HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
			varProblemIcon = (long)hProblem;
		}
	}
				
	//Add the row to the medication datalist
	// (a.walling 2007-10-01 09:10) - PLID 27568 - So let's use the EMNMedication info
	IRowSettingsPtr pRow;
	pRow = m_MedicationList->GetRow(-1);
	pRow->PutValue(mclID, (long)-1);
	pRow->PutValue(mclMedicationID, (long)pMed->nMedicationID);
	pRow->PutValue(mclProblemIcon, varProblemIcon);	
	pRow->PutValue(mclMedication, _variant_t(pMed->m_strDrugName));
	//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation
	pRow->PutValue(mclExplanation, _variant_t(pMed->strPatientExplanation));
	pRow->PutValue(mclRefills, _variant_t(pMed->strRefillsAllowed));
	//TES 2/10/2009 - PLID 33034 - Renamed PillsPerBottle to Quantity
	pRow->PutValue(mclPills, _variant_t(pMed->strQuantity));
	//TES 2/12/2009 - PLID 33034 - Added Strength
	//TES 3/31/2009 - PLID 33750 - Removed Strength
	pRow->PutValue(mclUnit, _variant_t(pMed->strUnit));
	//TES 2/12/2009 - PLID 33034 - Added Dosage Form
	//TES 3/31/2009 - PLID 33750 - Removed Dosage Form
	// (a.walling 2007-10-01 09:12) - PLID 27568 - Also add the object pointer
	pRow->PutValue(mclObjectPtr, (long)pMed);

	// (j.gruber 2009-03-30 11:43) - PLID 33736 - sync new crop
	pRow->PutValue(mclNewCropGUID, _variant_t(pMed->strNewCropGUID));
	pRow->PutValue(mclIsDiscontinued, pMed->bIsDiscontinued ? g_cvarTrue : g_cvarFalse);

	//make it blue if its newcrop
	// (a.walling 2009-04-22 13:30) - PLID 33948 - make it blue if it is EPrescribe (SureScripts)
	if(!pMed->strNewCropGUID.IsEmpty() || pMed->bEPrescribe) {
		if(pMed->bIsDiscontinued) {
			//set it to a grayish blue
			pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);			
		}
		else {
			//normal blue
			pRow->PutBackColor(EPRESCRIBE_COLOR);			
		}
	}

	m_MedicationList->InsertRow(pRow, -1);

	// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
	ShowHideProblemIconColumn_MedicationList();
}

// (a.walling 2007-09-28 17:52) - PLID 27568 - Remove the prescription
// not just the first row that has the same drug
void CEMNMoreInfoDlg::RemoveMedication(EMNMedication* pMedication)
{
	try {

		//Remove from the screen.
		// (a.walling 2007-10-01 08:40) - PLID 27568 - Search by the pointer
		long nRow = m_MedicationList->FindByColumn(mclObjectPtr, (long)pMedication, 0, FALSE);
		if (nRow != sriNoRow) {
			m_MedicationList->RemoveRow(nRow);

			// (j.jones 2008-07-22 09:26) - PLID 30792 - try to hide the problem icon column
			ShowHideProblemIconColumn_MedicationList();
		}
	} NxCatchAll("Error in RemoveMedication");
}

void CEMNMoreInfoDlg::SetReadOnly(BOOL bReadOnly)
{
	m_bReadOnly = bReadOnly;
	EnableControls();
}

LRESULT CEMNMoreInfoDlg::OnEmnMedicationAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		EMNMedication *pMedication = (EMNMedication*)wParam;

		// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
		_variant_t varProblemIcon = g_cvarNull;			
		if(pMedication->HasProblems()) {
			if(pMedication->HasOnlyClosedProblems()) {
				HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
				varProblemIcon = (long)hProblem;
			}
			else {
				HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
				varProblemIcon = (long)hProblem;
			}
		}

		IRowSettingsPtr pRow;
		pRow = m_MedicationList->GetRow(-1);
		pRow->PutValue(mclID, pMedication->nID);
		pRow->PutValue(mclMedicationID, pMedication->nMedicationID);
		pRow->PutValue(mclProblemIcon, varProblemIcon);	
		pRow->PutValue(mclMedication, _variant_t(pMedication->m_strDrugName));
		//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation
		pRow->PutValue(mclExplanation, _variant_t(pMedication->strPatientExplanation));
		pRow->PutValue(mclRefills, _variant_t(pMedication->strRefillsAllowed));
		//TES 2/10/2009 - PLID 33034 - Renamed PillsPerBottle to Quantity
		pRow->PutValue(mclPills, _variant_t(pMedication->strQuantity));
		//TES 2/12/2009 - PLID 33034 - Added Strength
		//TES 3/31/2009 - PLID 33750 - Removed Strength
		pRow->PutValue(mclUnit, _variant_t(pMedication->strUnit));
		//TES 2/12/2009 - PLID 33034 - Added Dosage Form
		//TES 3/31/2009 - PLID 33750 - Removed Dosage Form
		// (a.walling 2007-10-01 09:02) - PLID 27568 - Include the object pointer
		pRow->PutValue(mclObjectPtr, (long)pMedication);

		// (j.gruber 2009-03-30 11:43) - PLID 33736 - new columns for new crop
		pRow->PutValue(mclNewCropGUID, _variant_t(pMedication->strNewCropGUID));
		pRow->PutValue(mclIsDiscontinued, pMedication->bIsDiscontinued ? g_cvarTrue : g_cvarFalse);
		
		//make it blue if its newcrop
		// (a.walling 2009-04-22 13:30) - PLID 33948 - make it blue if it is EPrescribe (SureScripts)
		if(!pMedication->strNewCropGUID.IsEmpty() || pMedication->bEPrescribe) {
			if(pMedication->bIsDiscontinued) {
				//set it to a grayish blue
				pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);			
			}
			else {
				//normal blue
				pRow->PutBackColor(EPRESCRIBE_COLOR);			
			}
		}

		m_MedicationList->InsertRow(pRow, -1);

		// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
		ShowHideProblemIconColumn_MedicationList();

	} NxCatchAll("Error in OnEmnMedicationAdded");

	return 0;
}

//DRT 3/3/2006 - PLID 19410 - Changed wParam from the ProcedureID to a ptr to an EMNProcedure structure
LRESULT CEMNMoreInfoDlg::OnEmrProcedureAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		EMNProcedure* pProc = (EMNProcedure*)wParam;

		//Update the screen.
		if(AddProcedure(pProc->nID, pProc->strName)) {

			//if successful, then it was not a duplicate
		
			//Update our memory object.
			m_pEMN->AddProcedure(pProc->nID, pProc->strName);

			if(GetEmrTreeWnd())
				GetEmrTreeWnd()->SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProc, lParam);
		}
	} NxCatchAll("Error in OnEmrProcedureAdded");

	return 0;
}

void CEMNMoreInfoDlg::SetProviders(const CArray<long,long> &arProviderIDs)
{
	// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization - update saved values
	m_lastProviders.assign(arProviderIDs.GetData(), arProviderIDs.GetData() + arProviderIDs.GetSize());
	if(arProviderIDs.GetSize() <= 1) {
		ShowDlgItem(IDC_EMN_PROVIDER, SW_SHOW);
		ShowDlgItem(IDC_SELECT_EMN_PROVIDERS, SW_HIDE);

		if(arProviderIDs.GetSize() == 0) {
			m_pProviderCombo->CurSel = NULL;
			m_pProviderCombo->PutComboBoxText(_bstr_t(""));
			return;
		}

		//TES 6/20/2006 - FindByColumn is synchronous, let's use an asynchronous method to speed up the loading.
		/*NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderCombo->FindByColumn(0, nProviderID, NULL, TRUE);
		if(pRow) {
			//Since we set bAutoSelect to TRUE, it is now selected.  We're done.
		}
		else {
			//It must be inactive.
			CString strProvName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", nProviderID),"");
			if(!strProvName.IsEmpty()) {
				m_pProviderCombo->PutComboBoxText(_bstr_t(strProvName));
			}
		}*/

		//At this point we know there's exactly one provider.
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nSel = m_pProviderCombo->TrySetSelByColumn_Deprecated(0, arProviderIDs[0]);
		if(nSel == -1) {
			//It must be inactive.
			CString strProvName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", arProviderIDs[0]),"");
			if(!strProvName.IsEmpty()) {
				m_pProviderCombo->PutComboBoxText(_bstr_t(strProvName));
			}
		}
		else if(nSel == sriNoRowYet_WillFireEvent) {
			//Remember the ID for when we get the event.
			m_nPendingProviderID = arProviderIDs[0];
		}
	}
	else {
		//Multiple providers
		ShowDlgItem(IDC_EMN_PROVIDER, SW_HIDE);
		CString strProvList;
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr rsProvs = CreateParamRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN ProvidersT ON PersonT.Id = ProvidersT.PersonID WHERE PersonT.ID IN ({INTARRAY}) ORDER BY Last, First, Middle ASC", arProviderIDs);
		while(!rsProvs->eof) {
			strProvList += AdoFldString(rsProvs, "Name") + ", ";
			rsProvs->MoveNext();
		}
		rsProvs->Close();
		strProvList = strProvList.Left(strProvList.GetLength()-2);
		m_nxlProviderLabel.SetText(strProvList);
		m_nxlProviderLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_SELECT_EMN_PROVIDERS, SW_SHOW);
		InvalidateDlgItem(IDC_SELECT_EMN_PROVIDERS);
	}
}

void CEMNMoreInfoDlg::OnSelChosenEmnProvider(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		BOOL bSelectedMultiple = FALSE;
		if(pRow) {
			//Did they select the "{Multiple Providers}" row?
			long nProviderID = VarLong(pRow->GetValue(0));
			if(nProviderID == -1) {
				//Yup.  Let them select which providers they want, this function will update both m_pEMN and the interface 
				// appropriately.
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				SelectProviders();
				//TES 5/1/2008 - PLID 27608 - SelectProviders() sends the NXM_EMN_MORE_INFO_CHANGED message itself,
				// if necessary (i.e., not if they cancel).  So, don't send it again.
				bSelectedMultiple = TRUE;

			}
			else {
				//Nope, just the one.
				CArray<long,long> arProviderIDs;
				arProviderIDs.Add(nProviderID);
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				m_pEMN->SetProviders(arProviderIDs);
			}
		}
		else {
			//Set the array of providers to an empty list.
			CArray<long,long> arProviderIDs;
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetProviders(arProviderIDs);
		}
		//Tell our parent, so it can update its interface.
		//TES 5/1/2008 - PLID 27608 - Only send this message if they didn't select {Multiple Providers}.
		if(!bSelectedMultiple) {
			//TES 5/6/2008 - PLID 27821 - Also, update any narratives.
			m_pEMN->HandleDetailChange(NULL);
		}
	} NxCatchAll("Error in OnSelChosenEmnProvider");
}

void CEMNMoreInfoDlg::OnSelChosenEmnLocation(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			// (z.manning, 05/07/2007) - PLID 25925 - Now need to pass in location name as well.
			// (a.walling 2008-07-01 17:31) - PLID 30586 - And the logopath
			//(a.walling 2010-10-29 10:33) - PLID 31435 - Logo width in CEMNMoreInfoDlg Location dropdown
			CString strLogoPath = VarString(pRow->GetValue(2), "");
			long nLogoWidth = VarLong(pRow->GetValue(3), 100);
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetLocation(VarLong(pRow->GetValue(0)), VarString(pRow->GetValue(1),""), strLogoPath, nLogoWidth);
		}
		else {
			//Location is required!
			m_pLocationCombo->CurSel = m_pLocationCombo->GetFirstRow();
			if(m_pLocationCombo->CurSel != NULL) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				OnSelChosenEmnLocation(m_pLocationCombo->CurSel);
			}
		}
	} NxCatchAll("Error in OnSelChosenEmnLocation");
}

// (z.manning, 04/11/2007) - PLID 25569
void CEMNMoreInfoDlg::OnSelChosenEmnChart(LPDISPATCH lpRow) 
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nOldChartID = m_pEMN->GetChart();
		long nNewChartID;
		if(pRow) {
			// (z.manning, 05/07/2007) - PLID 25731 - Need to include the name as well.
			nNewChartID = VarLong(pRow->GetValue(chartID));
			// (z.manning 2011-05-20 09:37) - PLID 33114 - SetChart now checks for permission and in this case we do
			// not want that check to be silent.
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			if(!m_pEMN->SetChart(nNewChartID, VarString(pRow->GetValue(chartDescription),""), FALSE)) {
				this->SetChart(nOldChartID);
				return;
			}
		}
		else {
			nNewChartID = -1;
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetChart(nNewChartID, "");
		}

		// (z.manning, 05/22/2007) - PLID 25569 - If the chart changed, we need to reload the list of categories
		// because different charts can be associated with different categories. We then try to reselect the same category.
		// If it's not there the TrySetSel stuff will handle it.
		if(nNewChartID != nOldChartID) {
			RefilterCategoryList();
			TrySetCategorySelection(m_pEMN->GetCategory(), FALSE);
		}

	}NxCatchAll("CEMNMoreInfoDlg::OnSelChosenEmnChart");
}

// (z.manning, 04/11/2007) - PLID 25569
void CEMNMoreInfoDlg::OnSelChosenEmnCategory(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		if(pRow) {
			// (z.manning, 05/07/2007) - PLID 25731 - Need to include the name as well.
			m_pEMN->SetCategory(VarLong(pRow->GetValue(catID)), VarString(pRow->GetValue(catDescription),""));
		}
		else {
			m_pEMN->SetCategory(-1, "");
		}

	}NxCatchAll("CEMNMoreInfoDlg::OnSelChosenEmnCategory");
}

void CEMNMoreInfoDlg::SetLocation(long nLocationID)
{
	if(nLocationID == -1) {
		m_pLocationCombo->CurSel = NULL;
		m_pLocationCombo->PutComboBoxText(_bstr_t(""));
		return;
	}

	//TES 6/20/2006 - FindByColumn is synchronous, which we don't like.
	/*NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationCombo->FindByColumn(0, nLocationID, NULL, TRUE);
	if(pRow) {
		//Since we set bAutoSelect to TRUE, it is now selected.  We're done.
	}
	else {
		//It must be inactive.
		CString strLocName = VarString(GetTableField("LocationsT", "Name", "ID", nLocationID),"");
		if(!strLocName.IsEmpty()) {
			m_pLocationCombo->PutComboBoxText(_bstr_t(strLocName));
		}
	}*/

	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	long nSel = m_pLocationCombo->TrySetSelByColumn_Deprecated(0, nLocationID);
	if(nSel == -1) {
		//It must be inactive.
		CString strLocName = VarString(GetTableField("LocationsT", "Name", "ID", nLocationID),"");
		if(!strLocName.IsEmpty()) {
			m_pLocationCombo->PutComboBoxText(_bstr_t(strLocName));
		}
	}
	else if(nSel == sriNoRowYet_WillFireEvent) {
		//Remember the ID for when we get the event.
		m_nPendingLocationID = nLocationID;
	}
}

// (z.manning, 04/11/2007) - PLID 25569
void CEMNMoreInfoDlg::SetChart(long nChartID)
{
	if(nChartID == -1) {
		m_pChartCombo->CurSel = NULL;
		m_pChartCombo->PutComboBoxText(_bstr_t(""));
	}
	else {
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nSel = m_pChartCombo->TrySetSelByColumn_Deprecated(chartID, nChartID);
		if(nSel == sriNoRow) {
			ASSERT(FALSE);
			m_pChartCombo->CurSel = NULL;
			m_pChartCombo->PutComboBoxText(_bstr_t(""));
		}
		else {
			// (z.manning, 05/22/2007) - Either we succeeded or let the the OnTrySetSelFinished handler take care of it.
		}
	}
}

// (z.manning, 04/11/2007) - PLID 25569
void CEMNMoreInfoDlg::SetCategory(long nCategoryID)
{
	if(nCategoryID == -1) {
		m_pCategoryCombo->CurSel = NULL;
		m_pCategoryCombo->PutComboBoxText(_bstr_t(""));
		return;
	}

	m_pCategoryCombo->SetSelByColumn(catID, nCategoryID);
}	

void CEMNMoreInfoDlg::SetDate(COleDateTime dt, BOOL bUpdate)
{
	// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization - check for modification
	_variant_t varDt(dt, VT_DATE);
	
	if (!bUpdate && m_dtDate.GetValue() == varDt) {
		return;
	}

	m_dtDate.SetValue(varDt);

	// (a.walling 2008-12-30 17:03) - PLID 30252 - If bUpdate, we want to update the dialog and EMN as well
	if (bUpdate) {
		OnChangeEmnDate(NULL, NULL);
	}
}

void CEMNMoreInfoDlg::CalculateAge()
{
	try {

		if(!m_pEMN) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}
		
		// (j.jones 2007-08-07 11:49) - PLID 26974 - disallow this on saved EMNs
		if(m_pEMN->GetID() != -1)
			return;

		COleDateTime dtBirthDate = m_pEMN->GetParentEMR()->GetPatientBirthDate();

		if(((COleDateTime)m_dtDate.GetValue()).m_dt < dtBirthDate.m_dt) {
			AfxMessageBox("The EMN date cannot be before the patient's birthdate.");
			COleDateTime dtToUse = COleDateTime::GetCurrentTime();

			// (j.jones 2009-09-24 10:13) - PLID 29718 - added preference to default to the last appt. date
			if(GetRemotePropertyInt("EMNUseLastApptDate", 0, 0, "<None>", true) == 1) {
				COleDateTime dtLast = GetLastPatientAppointmentDate(m_pEMN->GetParentEMR()->GetPatientID());
				//make sure the appt. isn't before the patient's birthdate
				COleDateTime dtBirthDate = m_pEMN->GetParentEMR()->GetPatientBirthDate();		
				if(dtLast.GetStatus() != COleDateTime::invalid
					&& (dtBirthDate.GetStatus() == COleDateTime::invalid || dtLast >= dtBirthDate)) {
					dtToUse = dtLast;
				}
			}

			m_dtDate.SetValue((_variant_t)dtToUse);
			m_pEMN->SetEMNDate(dtToUse);
			//Can't return here, or the birthdate field is then messed up.  We still need to calc the age.
		}

		//always change the age
		if(dtBirthDate.GetStatus()!=COleDateTime::invalid) {
			// Use a temporary value so that GetPatientAge doesn't write to our stored member variable
			COleDateTime dtBirthDateTemporary = dtBirthDate;
			// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
			//  validation should only be done when bdays are entered/changed
			m_pEMN->SetPatientAge(GetPatientAgeOnDate(dtBirthDateTemporary, ((COleDateTime)m_dtDate.GetValue()), TRUE));
		}
		else {
			//invalid status, make sure it is empty
			// (z.manning 2010-01-13 10:51) - PLID 22672 - Age is now a string
			m_pEMN->SetPatientAge("");
		}

	}NxCatchAll("Error in CalculateAge");
}

void CEMNMoreInfoDlg::OnChangeEmnDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->SetEMNDate(VarDateTime(m_dtDate.GetValue()));

		//PLID 19520 - We need to re-calculate the patient's age 
		// (j.jones 2007-08-07 11:49) - PLID 26974 - disallow changing the age from here on saved EMNs
		if(m_pEMN->GetID() == -1) {
			
			CalculateAge();

			//Update the screen with the new age
			CString strPatientAge = "";
			// (z.manning 2010-01-14 09:28) - PLID 22672 - Age is now a string
			if(!m_pEMN->GetPatientAge().IsEmpty()){
				strPatientAge = m_pEMN->GetPatientAge();
			}
			//Update the dialog with the age.
			SetDlgItemText(IDC_EMN_AGE, strPatientAge);
		}

		// (a.walling 2008-12-30 17:03) - PLID 30252 - Don't set if null
		if (pResult) 
			*pResult = 0;
	} NxCatchAll("Error in OnChangeEmnDate");

}

void CEMNMoreInfoDlg::SetStatus(long nStatus)
{
	m_pStatusCombo->SetSelByColumn(0, nStatus);
}

void CEMNMoreInfoDlg::OnSelChosenEmnStatus(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			//Check with the treewnd.

			// (j.jones 2011-07-05 11:56) - PLID 43603 - this is now a class
			EMNStatus emnNewStatus;
			emnNewStatus.nID = VarLong(pRow->GetValue(0));
			emnNewStatus.strName = VarString(pRow->GetValue(1));

			// (z.manning 2009-08-11 15:22) - PLID 24277 - New permission for locking EMNs
			if(emnNewStatus.nID != 2 || CheckCurrentUserPermissions(bioPatientEMR, sptDynamic3)) {
				//this will prompt the user (if changing to locked), 
				//update the status on the EMN, and re-set this screen if they change their mind				
				GetEmrTreeWnd()->SendMessage(NXM_EMN_STATUS_CHANGING, (WPARAM)&emnNewStatus, (LPARAM)m_pEMN);
			}
			else {
				m_pStatusCombo->SetSelByColumn(0, m_pEMN->GetStatus().nID);
			}
		}
		else {
			//Status is required!
			m_pStatusCombo->SetSelByColumn(0, m_pEMN->GetStatus().nID);
		}
	} NxCatchAll("Error in OnSelChosenEmnStatus");
}

void CEMNMoreInfoDlg::OnChangeEmnDescription() 
{
	try {
		CString strDescription;
		GetDlgItemText(ID_EMR_EMN_DESCRIPTION, strDescription);
		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->SetDescription(strDescription);
	} NxCatchAll("Error in OnChangeEmnDescription");
}

void CEMNMoreInfoDlg::OnChangeEmnNotes() 
{
	try {
		CString strNotes;
		GetDlgItemText(ID_EMR_EMN_NOTES, strNotes);
		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->SetNotes(strNotes);
	} NxCatchAll("Error in OnChangeEmnNotes");
}

void CEMNMoreInfoDlg::OnSelChosenEmnProcedureCombo(LPDISPATCH lpRow) 
{
	try {
		if(!lpRow) {
			return;
		}
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nProcedureID = VarLong(pRow->GetValue(0));
		if(m_pProcedureList->FindByColumn(0, nProcedureID, NULL, FALSE)) {
			MsgBox("This procedure is already selected for this EMN.");
			// (j.jones 2009-12-21 10:50) - PLID 17521 - clear the selection
			m_pProcedureCombo->PutCurSel(NULL);
			return;
		}
		CString strName = VarString(pRow->GetValue(1), "");

		//Update the screen.
		if(AddProcedure(nProcedureID, strName)) {

			//if successful, then it was not a duplicate
		
			//Update our memory object.
			EMNProcedure* pProc = m_pEMN->AddProcedure(nProcedureID, strName);

			if(GetEmrTreeWnd())
				GetEmrTreeWnd()->SendMessage(NXM_EMN_PROCEDURE_ADDED, (WPARAM)pProc, (LPARAM)m_pEMN);
		}
		
		//Notify our parent
		GetEmrTreeWnd()->SendMessage(NXM_EMN_CHANGED, (WPARAM)m_pEMN);

		// (j.jones 2009-12-21 10:50) - PLID 17521 - clear the selection
		m_pProcedureCombo->PutCurSel(NULL);

	} NxCatchAll("Error in OnSelChosenEmnProcedureCombo");
}

void CEMNMoreInfoDlg::OnRButtonDownEmnProcedureList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu pMenu;
		m_pProcedureList->CurSel = NXDATALIST2Lib::IRowSettingsPtr(lpRow);
		
		pMenu.CreatePopupMenu();
		pMenu.AppendMenu(lpRow?MF_ENABLED:MF_DISABLED|MF_GRAYED, ID_REMOVE_PROCEDURE, "Remove Procedure");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		pMenu.DestroyMenu();
	} NxCatchAll("Error in OnRButtonDownEmnProcedureList");
}

void CEMNMoreInfoDlg::OnRemoveProcedure()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->CurSel;
		if(pRow) {
			long nProcedureID = VarLong(pRow->GetValue(0));
			//Update our memory object.
			if(GetEmrTreeWnd()) {
				GetEmrTreeWnd()->SendMessage(NXM_PRE_DELETE_EMN_PROCEDURE, (WPARAM)nProcedureID, (LPARAM)m_pEMN);
			}
			m_pEMN->RemoveProcedure(nProcedureID);
			//Update the screen.
			m_pProcedureList->RemoveRow(m_pProcedureList->FindByColumn(0,nProcedureID,NULL,FALSE));
			//Notify our parent
			if(GetEmrTreeWnd()) {
				// (z.manning, 10/05/2007) - PLID 27630 - We now also have a message for after a procedure has been removed.
				GetEmrTreeWnd()->SendMessage(NXM_POST_DELETE_EMN_PROCEDURE, (WPARAM)nProcedureID, (LPARAM)m_pEMN);
				GetEmrTreeWnd()->SendMessage(NXM_EMN_CHANGED, (WPARAM)m_pEMN);
			}
		}
	} NxCatchAll("Error in OnRemoveProcedure");
}

void CEMNMoreInfoDlg::EnableControls()
{
	//TES 3/10/2014 - PLID 61313 - I moved a bunch of code that checks m_bIsTemplate here from OnInitDialog()

	//set the background color
	// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
	m_bkgColor = EmrColors::Topic::Background(!!m_bIsTemplate);

	if (m_bIsTemplate) {
		//TES 3/10/2014 - PLID 61313 - Configure Preview is available on templates now.
		// (a.walling 2007-07-13 09:29) - PLID 26640 - Disable and hide this button in the admin module
		/*m_btnConfigPreview.EnableWindow(FALSE);
		m_btnConfigPreview.ShowWindow(SW_HIDE);*/

		//TES 3/10/2014 - PLID 61313 - Disable these buttons instead of hiding them
		// (d.thompson 2009-05-27) - PLID 29909 - Do the same with the confidential info button
		GetDlgItem(ID_EMR_SHOW_CONFIDENTIAL_INFO)->EnableWindow(FALSE);
		// (a.walling 2013-02-13 12:07) - PLID 55143 - Emr Appointment linking - UI
		GetDlgItem(ID_EMR_EMN_APPOINTMENT)->EnableWindow(FALSE);

		GetDlgItem(IDC_MERGE_SUMMARY_CARE_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_MERGE_CLINICAL_SUMMARY_BTN)->EnableWindow(FALSE);
	}

	if(m_brush.m_hObject) {
		m_brush.DeleteObject();
	}
	m_brush.CreateSolidBrush(m_bkgColor);

	m_bkg3.SetColor(m_bkgColor);
	m_bkg4.SetColor(m_bkgColor);
	m_bkg5.SetColor(m_bkgColor);
	m_bkg8.SetColor(m_bkgColor);
	m_bkg9.SetColor(m_bkgColor);
	m_bkg10.SetColor(m_bkgColor);

	m_nxlProviderLabel.SetColor(m_bkgColor);
	m_nxlSecondaryProvLabel.SetColor(m_bkgColor);
	m_nxlTechnicianLabel.SetColor(m_bkgColor);

	//Controls that aren't valid on templates.
	GetDlgItem(IDC_EMN_PROVIDER)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	if(!m_bIsTemplate && !m_bReadOnly) {
		m_nxlProviderLabel.SetType(dtsHyperlink);
		// (a.wetta 2007-02-22 13:00) - PLID 24328 - We don't need to enable the window because the set type takes care of that, and when
		// EnableWindow(FALSE) is used is causes some drawing issues.  Also, only invalidate the label and don't redraw the whole window because
		// it causes more flashing.
		//m_nxlProviderLabel.EnableWindow(TRUE);		
		m_nxlProviderLabel.Invalidate();
	}
	else {
		// (a.wetta 2007-02-22 13:00) - PLID 24328 - We don't need to un-enable the window because the set type takes care of that, and when
		// EnableWindow(FALSE) is used is causes some drawing issues.  Also, only invalidate the label and don't redraw the whole window because
		// it causes more flashing.
		//GetDlgItem(IDC_SELECT_EMN_PROVIDERS)->EnableWindow(FALSE);
		m_nxlProviderLabel.SetType(dtsDisabledHyperlink);
		m_nxlProviderLabel.Invalidate();
	}
	
	// (j.gruber 2007-01-08 12:33) - 23399 - Secondary Provider
	GetDlgItem(IDC_EMN_SECONDARY)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	if(!m_bIsTemplate && !m_bReadOnly) {
		m_nxlSecondaryProvLabel.SetType(dtsHyperlink);
		// (a.wetta 2007-02-22 13:00) - PLID 24328 - We don't need to enable the window because the set type takes care of that, and when
		// EnableWindow(FALSE) is used is causes some drawing issues.  Also, only invalidate the label and don't redraw the whole window because
		// it causes more flashing.
		//m_nxlSecondaryProvLabel.EnableWindow(TRUE);		
		m_nxlSecondaryProvLabel.Invalidate();
	}
	else {
		// (a.wetta 2007-02-22 13:00) - PLID 24328 - We don't need to un-enable the window because the set type takes care of that, and when
		// EnableWindow(FALSE) is used is causes some drawing issues.  Also, only invalidate the label and don't redraw the whole window because
		// it causes more flashing.
		//GetDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS)->EnableWindow(FALSE);
		m_nxlSecondaryProvLabel.SetType(dtsDisabledHyperlink);
		m_nxlSecondaryProvLabel.Invalidate();
	}

	// (d.lange 2011-03-23 09:51) - PLID 42136 - Assistant/Technician
	GetDlgItem(IDC_EMN_TECH)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	if(!m_bIsTemplate && !m_bReadOnly) {
		m_nxlTechnicianLabel.SetType(dtsHyperlink);
		m_nxlTechnicianLabel.Invalidate();
	}else {
		m_nxlTechnicianLabel.SetType(dtsDisabledHyperlink);
		m_nxlTechnicianLabel.Invalidate();
	}

	// (j.gruber 2009-05-07 17:10) - PLID 33688 - Other Provider Button
	GetDlgItem(IDC_EMN_OTHER_PROVS)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	
	GetDlgItem(IDC_EMN_LOCATION)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	GetDlgItem(IDC_EMN_STATUS)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	GetDlgItem(IDC_EMN_DATE)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);
	// (z.manning, 04/11/2007) - PLID 25569 - Even though you are actually allowed to change the chart and category
	// of locked EMNs, let's still disable the dropdowns in more info to be consistent with all of the other controls.
	// (z.manning, 04/12/2007) - PLID 25600 - These should be enabled on templates so we can set defaults.
	GetDlgItem(IDC_EMN_CHART)->EnableWindow(!m_bReadOnly);
	GetDlgItem(IDC_EMN_CATEGORY)->EnableWindow(!m_bReadOnly);

	// (c.haag 2008-07-07 11:09) - PLID 30607 - Added todo list
	// (c.haag 2008-07-08 10:01) - PLID 30632 - Need to factor in read only for locked EMN's, and also disable for templates
	if (m_bIsTemplate) {
		m_pTodoList->Enabled = VARIANT_FALSE;
	} else {
		m_pTodoList->ReadOnly = (!m_bReadOnly) ? VARIANT_FALSE : VARIANT_TRUE;
	}

	// (c.haag 2008-07-08 16:58) - PLID 30607 - Disable the radio buttons if this is a template
	if (m_bIsTemplate) {
		m_radioTodoIncomplete.EnableWindow(FALSE);
		m_radioTodoCompleted.EnableWindow(FALSE);
		m_radioTodoAll.EnableWindow(FALSE);
	}

	//Controls that aren't valid on templates, but ARE valid on locked EMNs.
	
	GetDlgItem(IDC_BTN_WRITE_PRESCRIPTIONS)->EnableWindow(!m_bIsTemplate);

	//TES 3/10/2014 - PLID 61313 - Disable these buttons instead of hiding them
	GetDlgItem(IDC_MERGE_TO)->EnableWindow(!m_bIsTemplate);
	GetDlgItem(IDC_BTN_MERGE_TO_OTHER)->EnableWindow(!m_bIsTemplate);
	GetDlgItem(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK)->EnableWindow(!m_bIsTemplate);

	//Other controls.
	GetDlgItem(IDC_EMN_PROCEDURE_COMBO)->EnableWindow(!m_bReadOnly);
	m_pProcedureList->PutReadOnly(m_bReadOnly);
	
	GetDlgItem(IDC_BTN_ADD_MEDICATION)->EnableWindow(!m_bReadOnly);
	m_MedicationList->PutReadOnly(m_bReadOnly);

	// (j.jones 2007-08-06 15:25) - PLID 26974 - disable the "update information" button when necessary
	GetDlgItem(IDC_BTN_UPDATE_PAT_INFO)->EnableWindow(!m_bIsTemplate && !m_bReadOnly);




	//TES 10/15/2010 - PLID 40183 - Disable the spell check button on the notes if we are read only.
	m_btnSpellCheckNotes.EnableWindow(!m_bReadOnly);
}


void CEMNMoreInfoDlg::RefreshPrescriptionList()
{
	try {
		//reload the prescription list

		m_MedicationList->Clear();

		for(int i=0;i<m_pEMN->GetMedicationCount(); i++) {
			EMNMedication* pMed = NULL;
			// (a.walling 2007-10-01 08:47) - PLID 27568 - Get the pointer
			pMed = m_pEMN->GetMedicationPtr(i);

			// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
			_variant_t varProblemIcon = g_cvarNull;			
			if(pMed->HasProblems()) {
				if(pMed->HasOnlyClosedProblems()) {
					HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
					varProblemIcon = (long)hProblem;
				}
				else {
					HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
					varProblemIcon = (long)hProblem;
				}
			}

			IRowSettingsPtr pRow = m_MedicationList->GetRow(sriGetNewRow);
			pRow->PutValue(mclID, pMed->nID);
			pRow->PutValue(mclMedicationID, pMed->nMedicationID);
			pRow->PutValue(mclProblemIcon, varProblemIcon);
			pRow->PutValue(mclMedication, _bstr_t(pMed->m_strDrugName));
			//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation
			pRow->PutValue(mclExplanation, _bstr_t(pMed->strPatientExplanation));
			pRow->PutValue(mclRefills, _bstr_t(pMed->strRefillsAllowed));
			//TES 2/10/2009 - PLID 33034 - Renamed PillsPerBottle to Quantity
			pRow->PutValue(mclPills, _bstr_t(pMed->strQuantity));
			//TES 2/12/2009 - PLID 33034 - Added Strength
			//TES 3/31/2009 - PLID 33750 - Removed Strength
			pRow->PutValue(mclUnit, _bstr_t(pMed->strUnit));
			//TES 2/12/2009 - PLID 33034 - Added Dosage Form
			//TES 3/31/2009 - PLID 33750 - Removed Dosage Form
			// (a.walling 2007-09-28 18:12) - PLID 27568 - Include the pointer
			pRow->PutValue(mclObjectPtr, long(pMed));

			// (j.gruber 2009-03-30 11:43) - PLID 33736 - new columns for new crop
			pRow->PutValue(mclNewCropGUID, _variant_t(pMed->strNewCropGUID));
			pRow->PutValue(mclIsDiscontinued, pMed->bIsDiscontinued ? g_cvarTrue : g_cvarFalse);
			
			//make it blue if its newcrop
			// (a.walling 2009-04-22 13:30) - PLID 33948 - make it blue if it is EPrescribe (SureScripts)
			if(!pMed->strNewCropGUID.IsEmpty() || pMed->bEPrescribe) {
				if(pMed->bIsDiscontinued) {
					//set it to a grayish blue
					pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);	
				}
				else {
					//normal blue
					pRow->PutBackColor(EPRESCRIBE_COLOR);
				}
			}

			m_MedicationList->AddRow(pRow);

			// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
			ShowHideProblemIconColumn_MedicationList();
		}
	} NxCatchAll("Error in RefreshPrescriptionList");
}

void CEMNMoreInfoDlg::OnMergeTo()
{
	try {
		// (a.walling 2007-06-25 16:44) - PLID 15177 - Check for Word.
		if(!GetWPManager()->CheckWordProcessorInstalled())
			return;

		//TES 6/2/2004: They pick it out of the list now.
		if(m_pTemplateCombo->CurSel == -1) {
			ASSERT(FALSE);
			return;
		}

		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(!pTreeWnd) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		// (c.haag 2007-08-27 09:34) - PLID 27185 - Ensure that the EMN's initial load is done
		m_pEMN->EnsureCompletelyLoaded();

		// (a.walling 2008-06-12 13:21) - PLID 27301 - Check for duplicate merge names before we save
		if (!m_pEMN->WarnOfDuplicateMergeNames()) {
			// Returns FALSE if the user decided not to merge the chart
			return;
		}

		//(j.jones 2006-06-27 09:37) - Make sure the EMN is saved.
		//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we've already checked IsUnsaved()
		if(m_pEMN->IsUnsaved() || m_pEMN->GetID() == -1) {
			if(IDYES != MsgBox(MB_YESNO, "Before merging, the changes you have made to the EMN must be saved.  Would you like to continue?")) {
				return;
			}
			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			if(FAILED(pTreeWnd->SaveEMR(esotEMN, (long)m_pEMN, TRUE))) {
				AfxMessageBox("The EMN was not saved. The merge will now be cancelled.");
				return;
			}
		}

		CString strTemplateFileName = GetTemplatePath() ^ VarString(m_pTemplateCombo->GetValue(m_pTemplateCombo->CurSel, 1)) ^ VarString(m_pTemplateCombo->GetValue(m_pTemplateCombo->CurSel, 0));

		// (c.haag 2009-08-10 11:17) - PLID 29160 - Added "Merge to printer" option
		m_pEMN->Merge(strTemplateFileName, IsDlgButtonChecked(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK), IsDlgButtonChecked(IDC_CHECK_MERGEEMNTOPRINTER));

		//refresh the EMN to show the Word icon
		CClient::RefreshTable(NetUtils::EMRMasterT, m_pEMN->GetParentEMR()->GetPatientID());
		
	} NxCatchAll("CEMNMoreInfoDlg::OnMergeTo()");
}

void CEMNMoreInfoDlg::OnEmrMakeDefault()
{
	try {
		long nAuditID = -1;
		if(m_pTemplateCombo->CurSel == -1) {
			//Hey!  This shouldn't be possible!
			ASSERT(FALSE);
			GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(FALSE);
			return;
		}

		CString strFile = VarString(m_pTemplateCombo->GetValue(m_pTemplateCombo->CurSel, 1)) ^ VarString(m_pTemplateCombo->GetValue(m_pTemplateCombo->CurSel, 0));

		// (a.walling 2012-10-01 08:56) - PLID 52931 - Function now implemented within CEMN
		m_pEMN->SetDefaultMergeTemplate(strFile);

		for(int i = 0; i < m_pTemplateCombo->GetRowCount(); i++) {
			if(i == m_pTemplateCombo->CurSel) {
				((NXDATALISTLib::IRowSettingsPtr)m_pTemplateCombo->GetRow(i))->PutForeColor(RGB(255,0,0));
			}
			else {
				((NXDATALISTLib::IRowSettingsPtr)m_pTemplateCombo->GetRow(i))->PutForeColor(RGB(0,0,0));
			}
		}
		GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(FALSE);
	} NxCatchAll("Error in OnEmrMakeDefault");
}

void CEMNMoreInfoDlg::OnSelChosenTemplateList(long nRow) 
{
	try {
		if(nRow == -1 || ((NXDATALISTLib::IRowSettingsPtr)m_pTemplateCombo->GetRow(nRow))->ForeColor == RGB(255,0,0)) {
			GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(TRUE);
		}

		if(nRow == -1) {
			GetDlgItem(IDC_MERGE_TO)->EnableWindow(FALSE);
		}
		else {
			//TES 3/10/2014 - PLID 61313 - Don't enable on templates
			GetDlgItem(IDC_MERGE_TO)->EnableWindow(!m_bIsTemplate);
		}

		//TES 3/10/2014 - PLID 61313 - Don't enable on templates
		GetDlgItem(IDC_BTN_MERGE_TO_OTHER)->EnableWindow(!m_bIsTemplate);
	} NxCatchAll("Error in OnSelChosenTemplateList");
}

void CEMNMoreInfoDlg::EnsureDefaultTemplate()
{
	// (a.walling 2012-10-01 08:56) - PLID 52931 - Functionality now implemented within CEMN
	const CEMN::DefaultMergeTemplate& defaultTemplate = m_pEMN->GetDefaultMergeTemplate();

	CString strCurrentDefault = defaultTemplate.templateName;
	CString strProcedureName = defaultTemplate.procedureNames;
	CString strCollectionName = defaultTemplate.collectionName;

	CString strCaption;
	if(!strCollectionName.IsEmpty() && !strProcedureName.IsEmpty()) {
		strCaption = "for " + strCollectionName + " - " + strProcedureName;
	}
	else {
		strCaption = "for " + strCollectionName + strProcedureName;
	}
	
	strCaption.Replace("&","&&");
	SetDlgItemText(IDC_EMR_DEFAULT_FOR, strCaption);
	GetDlgItem(IDC_MERGE_TO)->EnableWindow(m_pTemplateCombo->CurSel != -1);
	GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(m_pTemplateCombo->CurSel != -1);
	//Now, find the default template, select it.
	//First, reset the colors.
	bool bFound = false;
	for(int i = 0; i < m_pTemplateCombo->GetRowCount(); i++) {
		((NXDATALISTLib::IRowSettingsPtr)m_pTemplateCombo->GetRow(i))->PutForeColor(RGB(0,0,0));
	}
	if(strCurrentDefault != "") {
		CString strDefaultFile, strDefaultFolder;
		strDefaultFile = strCurrentDefault.Mid(strCurrentDefault.ReverseFind('\\')+1);
		strDefaultFolder = strCurrentDefault.Left(strCurrentDefault.ReverseFind('\\'));
		long nFirstRow = FindTextInDataList(m_pTemplateCombo, 0, strDefaultFile, 0, false);
		if(nFirstRow != -1) {
			if(VarString(m_pTemplateCombo->GetValue(nFirstRow, 1)).CompareNoCase(strDefaultFolder) == 0) {
				//This is it!
				bFound = true;
				m_pTemplateCombo->CurSel = nFirstRow;
				NXDATALISTLib::IRowSettingsPtr pRow = m_pTemplateCombo->GetRow(nFirstRow);
				pRow->PutForeColor(RGB(255,0,0));
				GetDlgItem(IDC_MERGE_TO)->EnableWindow(TRUE);
				GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(FALSE);
			}
			else {
				long nNextRow = FindTextInDataList(m_pTemplateCombo, 0, strDefaultFile, nFirstRow, false);
				while(nNextRow > nFirstRow && !bFound) {
					if(VarString(m_pTemplateCombo->GetValue(nNextRow, 1)).CompareNoCase(strDefaultFolder) == 0) {
						//This is it!
						bFound = true;
						m_pTemplateCombo->CurSel = nNextRow;
						NXDATALISTLib::IRowSettingsPtr pRow = m_pTemplateCombo->GetRow(nNextRow);
						pRow->PutForeColor(RGB(255,0,0));
						GetDlgItem(IDC_MERGE_TO)->EnableWindow(TRUE);
						GetDlgItem(IDC_EMR_MAKE_DEFAULT)->EnableWindow(FALSE);
					}
					else {
						nNextRow = FindTextInDataList(m_pTemplateCombo, 0, strDefaultFile, nNextRow, false);
					}
				}
			}
		}
	}
	if(!bFound) {
		//This will clear out the "Loading..." text if it's there.
		m_pTemplateCombo->CurSel = -1;
	}
	//TES 3/10/2014 - PLID 61313 - Don't enable on templates
	GetDlgItem(IDC_BTN_MERGE_TO_OTHER)->EnableWindow(!m_bIsTemplate);
}

	// (a.walling 2012-10-01 08:56) - PLID 52931 - GetDefaultTemplateWhereClause now in CEMN

void CEMNMoreInfoDlg::OnPatemrEditTemplates() 
{
	try {
		CString strFile = "", strInitDir = "";
		if(m_pTemplateCombo->CurSel == -1) {
			strInitDir = GetTemplatePath();
		}
		else {
			strFile = VarString(m_pTemplateCombo->GetValue(m_pTemplateCombo->CurSel, 0));
			strInitDir = GetTemplatePath() ^ VarString(m_pTemplateCombo->GetValue(m_pTemplateCombo->CurSel, 1));
		}

		m_pEMN->EditWordTemplates(GetSafeHwnd(), strFile, strInitDir);
	} NxCatchAll("Error in OnPatemrEditTemplates");
}

void CEMNMoreInfoDlg::OnBtnMergeToOther() 
{
	try {
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
		m_pEMN->EnsureCompletelyLoaded();

		// (a.walling 2008-06-12 13:21) - PLID 27301 - Check for duplicate merge names before we save
		if (!m_pEMN->WarnOfDuplicateMergeNames()) {
			// Returns FALSE if the user decided not to merge the chart
			return;
		}

		//(j.jones 2006-06-27 09:37) - Make sure the EMN is saved.
		//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we've already checked IsUnsaved()
		if(m_pEMN->IsUnsaved() || m_pEMN->GetID() == -1) {
			if(IDYES != MsgBox(MB_YESNO, "Before merging, the changes you have made to the EMN must be saved.  Would you like to continue?")) {
				return;
			}

			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			if(FAILED(pTreeWnd->SaveEMR(esotEMN, (long)m_pEMN, TRUE))) {
				AfxMessageBox("The EMN was not saved. The merge will now be cancelled.");
				return;
			}
		}

		// Let the user browse for a template
		CString strTemplateFileName;
		{
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
		m_pEMN->Merge(strTemplateFileName, IsDlgButtonChecked(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK), IsDlgButtonChecked(IDC_CHECK_MERGEEMNTOPRINTER));

		//refresh the EMN to show the Word icon
		CClient::RefreshTable(NetUtils::EMRMasterT, m_pEMN->GetParentEMR()->GetPatientID());
		
	} NxCatchAll("CEMNMoreInfoDlg::OnBtnMergeToOther() ");
}

void CEMNMoreInfoDlg::OnEmrSaveDocsInHistoryCheck() 
{
	try {
		// Remember the setting
		BOOL bSave = IsDlgButtonChecked(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK);
		SetRemotePropertyInt("PICSaveDocsInHistory", bSave ? 1 : 0, 0, GetCurrentUserName());	
		
		// (a.walling 2012-07-11 09:39) - PLID 51476 - NXM_SET_SAVE_DOCS_IN_HISTORY, NXM_TREE_SEL_CHANGED is no more

	} NxCatchAll("Error in OnEmrSaveDocsInHistoryCheck");
}

// (c.haag 2009-08-10 11:11) - PLID 29160 - Preference for merging directly to the printer
void CEMNMoreInfoDlg::OnEmrMergeToPrinterCheck() 
{
	try {
		// Remember the setting
		SetRemotePropertyInt("PICMergeEMNToPrinter", IsDlgButtonChecked(IDC_CHECK_MERGEEMNTOPRINTER) ? 1 : 0, 0, GetCurrentUserName());	
	} 
	NxCatchAll("Error in OnEmrMergeToPrinterCheck");
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history handled by the frame now

void CEMNMoreInfoDlg::SetSaveDocsInHistory(BOOL bSave)
{
	CheckDlgButton(IDC_EMR_SAVE_DOCS_IN_HISTORY_CHECK, bSave?BST_CHECKED:BST_UNCHECKED);
}

LRESULT CEMNMoreInfoDlg::OnAddFolderNext(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2006-12-28 15:30) - PLID 23300 - This message is sent when we
	// need to add an element to a dropdown that contains a list of Word templates
	// and folders
	try {
		AddFolderMessage* pMsg = (AddFolderMessage*)wParam;
		_DNxDataListPtr dl(pMsg->pList);
		if (NULL != dl) {
			IRowSettingsPtr pRow = dl->GetRow(-1);
			pRow->Value[0] = _bstr_t(pMsg->strFileName);
			pRow->Value[1] = _bstr_t(pMsg->strFolderName);
			dl->AddRow(pRow);
		}
		// (c.haag 2008-03-31 15:57) - PLID 29429 - We're done with the input object,
		// so delete it.
		delete pMsg;
	}
	NxCatchAll("Error adding folder to list");
	return 0;
}

LRESULT CEMNMoreInfoDlg::OnAddFolderCompleted(WPARAM wParam, LPARAM lParam)
{
	try {
		GetDlgItem(IDC_TEMPLATE_LIST)->EnableWindow(TRUE);
		EnsureDefaultTemplate();
	}NxCatchAll("Error in CEMNMoreInfoDlg::OnAddFolderCompleted()");
	return 0;
}


void CEMNMoreInfoDlg::OnTrySetSelFinishedEmnLocation(long nRowEnum, long nFlags) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		ASSERT(m_nPendingLocationID != -1);
		if(nFlags == dlTrySetSelFinishedFailure) {
			//It must be inactive.
			CString strLocName = VarString(GetTableField("LocationsT", "Name", "ID", m_nPendingLocationID),"");
			if(!strLocName.IsEmpty()) {
				m_pLocationCombo->PutComboBoxText(_bstr_t(strLocName));
			}
		}
		m_nPendingLocationID = -1;
	} NxCatchAll("Error in OnTrySetSelFinishedEmnLocation");
}

void CEMNMoreInfoDlg::OnTrySetSelFinishedEmnProvider(long nRowEnum, long nFlags) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		ASSERT(m_nPendingProviderID != -1);
		if(nFlags == dlTrySetSelFinishedFailure) {
			//It must be inactive.
			CString strProvName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", m_nPendingProviderID),"");
			if(!strProvName.IsEmpty()) {
				m_pProviderCombo->PutComboBoxText(_bstr_t(strProvName));
			}
		}
		m_nPendingProviderID = -1;
	} NxCatchAll("Error in OnTrySetSelFinishedEmnProvider");
}

void CEMNMoreInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	try {

		CDialog::OnSize(nType, cx, cy);
		SetControlPositions();
		RedrawWindow();

	} NxCatchAll("CEMNMoreInfoDlg::OnSize");
}

void CEMNMoreInfoDlg::OnRequeryFinishedEmnProvider(short nFlags) 
{
	try {
		//TES 12/26/2006 - PLID 23400 - Allow them to select multiple providers.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{Multiple Providers}"));
		m_pProviderCombo->AddRowBefore(pRow, m_pProviderCombo->GetFirstRow());
	}NxCatchAll("Error in CEMNMoreInfoDlg::OnRequeryFinishedEmnProvider()");
}


void CEMNMoreInfoDlg::SelectProviders()
{
	try {
		//TES 12/26/2006 - PLID 23400 - Allow them to select multiple providers.
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
	
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		int i = 0;

		//First, pre-select whatever's already selected.
		CDWordArray dwaCurrentProviders;
		for(i = 0; i < m_pEMN->GetProviderCount(); i++) {
			dwaCurrentProviders.Add(m_pEMN->GetProvider(i)->nID);
		}
		dlg.PreSelect(dwaCurrentProviders);
		
		//Next, open up the dialog, with all licensed providers
		CString strLicensedProviders;
		CDWordArray dwaLicensedProviders;
		g_pLicense->GetUsedEMRProviders(dwaLicensedProviders);
		for(i = 0; i < dwaLicensedProviders.GetSize(); i++) {
			strLicensedProviders += FormatString("%i,",dwaLicensedProviders[i]);
		}
		strLicensedProviders.TrimRight(",");
		if(strLicensedProviders.IsEmpty()) strLicensedProviders = "-1";
		//TES 12/29/2006 - PLID 23400 - Also, any providers that are already selected should show in the list so they can uncheck them.
		CString strSelectedProviders;
		for(i = 0; i < m_pEMN->GetProviderCount(); i++) {
			strSelectedProviders += FormatString("%i,",m_pEMN->GetProvider(i)->nID);
		}
		strSelectedProviders.TrimRight(",");
		if(strSelectedProviders.IsEmpty()) strSelectedProviders = "-1";

		CArray<long,long> arSelectedProviders;
		UINT nResult = dlg.Open("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID", "PersonT.ID IN (" + strSelectedProviders + ") OR (PersonT.ID IN (" + strLicensedProviders + ") AND PersonT.Archived = 0)", "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select one or more providers to filter on:", 1);
		if(nResult == IDOK) {
			CDWordArray dwaSelectedProviders;
			dlg.FillArrayWithIDs(dwaSelectedProviders);
			for(int i = 0; i < dwaSelectedProviders.GetSize(); i++) {
				arSelectedProviders.Add((long)dwaSelectedProviders[i]);
			}
		}
		else {
			//TES 1/17/2007 - PLID 23400 - Revert to what was previously selected.
			for(int i = 0; i < dwaCurrentProviders.GetSize(); i++) {
				arSelectedProviders.Add((long)dwaCurrentProviders[i]);
			}
		}
		
		if(arSelectedProviders.GetSize() > 1) {
			ShowDlgItem(IDC_EMN_PROVIDER, SW_HIDE);
			if (nResult == IDOK) {
				m_nxlProviderLabel.SetText(dlg.GetMultiSelectString());
			}
			m_nxlProviderLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_SELECT_EMN_PROVIDERS, SW_SHOW);
			InvalidateDlgItem(IDC_SELECT_EMN_PROVIDERS);
		}
		else if(arSelectedProviders.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_EMN_PROVIDER, SW_SHOW);
			ShowDlgItem(IDC_SELECT_EMN_PROVIDERS, SW_HIDE);
			m_pProviderCombo->SetSelByColumn(0, arSelectedProviders.GetAt(0));
		}
		else {
			//They must have cancelled out.
			m_pProviderCombo->CurSel = NULL;
		}

		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->SetProviders(arSelectedProviders);
	}NxCatchAll("Error in CEMNMoreInfoDlg::SelectProviders()");
}

// (j.gruber 2007-01-08 11:02) - PLID 23399 - Secondary Providers
void CEMNMoreInfoDlg::SelectSecondaryProviders()
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
		//First, pre-select whatever's already selected.
		CDWordArray dwaCurrentSecondaryProviders;
		for(int i = 0; i < m_pEMN->GetSecondaryProviderCount(); i++) {
			dwaCurrentSecondaryProviders.Add(m_pEMN->GetSecondaryProvider(i)->nID);
		}
		dlg.PreSelect(dwaCurrentSecondaryProviders);
		
		//Next, open up the dialog, with all providers
		
		CArray<long,long> arSelectedSecondaryProviders;
		long nResult = dlg.Open("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID", "PersonT.Archived = 0", "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select one or more secondary providers:", 1);
		if(IDOK == nResult) {
			CDWordArray dwaSelectedSecondaryProviders;
			dlg.FillArrayWithIDs(dwaSelectedSecondaryProviders);
			for(int i = 0; i < dwaSelectedSecondaryProviders.GetSize(); i++) {
				arSelectedSecondaryProviders.Add((long)dwaSelectedSecondaryProviders[i]);
			}

		}
		else {
			//Revert to what was previously selected.
			for(int i = 0; i < dwaCurrentSecondaryProviders.GetSize(); i++) {
				arSelectedSecondaryProviders.Add((long)dwaCurrentSecondaryProviders[i]);
			}
		}

		if(arSelectedSecondaryProviders.GetSize() > 1) {
			ShowDlgItem(IDC_EMN_SECONDARY, SW_HIDE);
			if (IDOK == nResult) {
				m_nxlSecondaryProvLabel.SetText(dlg.GetMultiSelectString());
			}
			m_nxlSecondaryProvLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS, SW_SHOW);
			InvalidateDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS);
		}
		else if(arSelectedSecondaryProviders.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_EMN_SECONDARY, SW_SHOW);
			ShowDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS, SW_HIDE);
			m_pSecondaryProvCombo->SetSelByColumn(0, arSelectedSecondaryProviders.GetAt(0));
		}
		else {
			//They must have cancelled out.
			m_pSecondaryProvCombo->CurSel = NULL;
		}

		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->SetSecondaryProviders(arSelectedSecondaryProviders);
	}NxCatchAll("Error in CEMNMoreInfoDlg::SelectSecondaryProviders()");
}

// (d.lange 2011-03-22 17:23) - PLID 42136 - The user wants to select multiple Assistant/Technicians
void CEMNMoreInfoDlg::SelectAssistantTechnician()
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "UsersT");
		//First, pre-select whatever's already selected.
		CDWordArray dwaCurrentTechnicians;
		for(int i = 0; i < m_pEMN->GetTechnicianCount(); i++) {
			dwaCurrentTechnicians.Add(m_pEMN->GetTechnician(i)->nID);
		}
		dlg.PreSelect(dwaCurrentTechnicians);

		CArray<long,long> arSelectedTechnicians;
		long nResult = dlg.Open("UsersT LEFT JOIN PersonT ON PersonID = PersonT.ID", "Archived = 0 AND PersonID <> -26 AND UsersT.Technician = 1", "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select one or more assistant/technician:", 1);
		if(IDOK == nResult) {
			CDWordArray dwaSelectedTechnicians;
			dlg.FillArrayWithIDs(dwaSelectedTechnicians);
			for(int i = 0; i < dwaSelectedTechnicians.GetSize(); i++) {
				arSelectedTechnicians.Add((long)dwaSelectedTechnicians[i]);
			}
		}else {
			for(int i = 0; i < dwaCurrentTechnicians.GetSize(); i++) {
				arSelectedTechnicians.Add((long)dwaCurrentTechnicians[i]);
			}
		}

		if(arSelectedTechnicians.GetSize() > 1) {
			//Selected multiple
			//Hide the dropdown
			ShowDlgItem(IDC_EMN_TECH, SW_HIDE);
			if(IDOK == nResult) {
				//Give the text field the list of Assistant/Technician
				m_nxlTechnicianLabel.SetText(dlg.GetMultiSelectString());
			}
			m_nxlTechnicianLabel.SetType(dtsHyperlink);
			//Show the hyperlink
			ShowDlgItem(IDC_SELECT_EMN_TECHNICIAN, SW_SHOW);
			InvalidateDlgItem(IDC_SELECT_EMN_TECHNICIAN);
		}else if(arSelectedTechnicians.GetSize() == 1) {
			//Selected one
			ShowDlgItem(IDC_EMN_TECH, SW_SHOW);
			ShowDlgItem(IDC_SELECT_EMN_TECHNICIAN, SW_HIDE);
			m_pTechnicianCombo->SetSelByColumn(0, arSelectedTechnicians.GetAt(0));
		}else {
			//Cancelled
			m_pTechnicianCombo->CurSel = NULL;
		}

		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->SetTechnicians(arSelectedTechnicians);

	} NxCatchAll("Error in CEMNMoreInfoDlg::SelectAssistantTechnician()");
}

BOOL CEMNMoreInfoDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//TES 12/28/2006 - PLID 23400 - Show the hyperlink over our provider list, if it's showing.
		if(GetDlgItem(IDC_SELECT_EMN_PROVIDERS)->IsWindowVisible() && !m_pEMN->IsLockedAndSaved()) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			GetDlgItem(IDC_SELECT_EMN_PROVIDERS)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// (j.gruber 2007-01-08 11:07) - PLID 23399 - and the secondary label
		if(GetDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS)->IsWindowVisible() && !m_pEMN->IsLockedAndSaved()) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			GetDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		// (d.lange 2011-03-23 10:02) - PLID 42136 - Assistant/Technician
		if(GetDlgItem(IDC_SELECT_EMN_TECHNICIAN)->IsWindowVisible() && !m_pEMN->IsLockedAndSaved()) {
			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			CRect rc;
			GetDlgItem(IDC_SELECT_EMN_TECHNICIAN)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

	} NxCatchAll("Error in OnSetCursor");

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CEMNMoreInfoDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_SELECT_EMN_PROVIDERS:
			//TES 12/28/2006 - PLID 23400 - Just call our function which does everything (including exception handling);
			SelectProviders();
			break;
		// (j.gruber 2007-01-08 11:01) - PLID 23399 - handle secondary providers
		case IDC_SELECT_EMN_SECONDARY_PROVIDERS:
			SelectSecondaryProviders();
		break;		
		// (d.lange 2011-03-23 09:57) - PLID 42136 - Assistant/Technician
		case IDC_SELECT_EMN_TECHNICIAN:
			SelectAssistantTechnician();
			break;
		default:
			//There aren't any other clickable labels on this dialog!
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll("Error in OnLabelClick");
	return 0;
}


// (j.gruber 2007-01-08 11:14) - PLID 23399 - Secondary Providers
void CEMNMoreInfoDlg::OnRequeryFinishedEmnSecondary(short nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSecondaryProvCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{Multiple Providers}"));
		m_pSecondaryProvCombo->AddRowBefore(pRow, m_pSecondaryProvCombo->GetFirstRow());

		pRow = m_pSecondaryProvCombo->GetNewRow();
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t("{No Provider}"));
		m_pSecondaryProvCombo->AddRowBefore(pRow, m_pSecondaryProvCombo->GetFirstRow());
	}NxCatchAll("Error in CEMNMoreInfoDlg::OnRequeryFinishedEmnSecondary()");	
}

void CEMNMoreInfoDlg::OnRequeryFinishedEmnChart(short nFlags) 
{
	try
	{
		// (z.manning, 04/11/2007) - PLID 25569 - Add an option for no chart.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pChartCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{No Chart}"));
		m_pChartCombo->AddRowBefore(pRow, m_pChartCombo->GetFirstRow());

		if(m_bInitialized)
		{
			// (z.manning 2013-06-05 10:15) - PLID 56962 - If we're initialized then this may have happened in response
			// to a table checker. Restore the select chart and then refilter the category list.
			long nChartID = m_pEMN->GetChart().nID;
			if(nChartID != -1) {
				if(m_pChartCombo->SetSelByColumn(chartID, nChartID) == NULL) {
					// (z.manning 2013-06-05 10:16) - PLID 56962 - Chart row is gone, it must have been deleted.
					m_pEMN->SetChart(-1, "");
				}
			}

			RefilterCategoryList();
			TrySetCategorySelection(m_pEMN->GetCategory().nID, TRUE);
		}
	}
	NxCatchAll("CEMNMoreInfoDlg::OnRequeryFinishedEmnChart()");	
}

void CEMNMoreInfoDlg::OnRequeryFinishedEmnCategory(short nFlags) 
{
	try
	{
		// (z.manning, 04/11/2007) - PLID 25569 - Add an option for no category.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCategoryCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{No Category}"));
		m_pCategoryCombo->AddRowBefore(pRow, m_pCategoryCombo->GetFirstRow());

		if(m_bInitialized)
		{
			// (z.manning 2013-06-05 10:15) - PLID 56962 - If we're initialized then this may have happened in response
			// to a table checker. Refilter the category list and then attempt to restore the selected category.
			RefilterCategoryList();
			TrySetCategorySelection(m_pEMN->GetCategory().nID, TRUE);
		}
	}
	NxCatchAll("CEMNMoreInfoDlg::OnRequeryFinishedEmnCategory()");	
}

// (d.lange 2011-03-23 10:31) - PLID 42136 - Assistant/Technician
void CEMNMoreInfoDlg::OnTrySetSelFinishedEmnTechnician(long nRowEnum, long nFlags)
{
	try {
		ASSERT(m_nPendingTechnicianID != -1);
		if(nFlags == dlTrySetSelFinishedFailure) {
			//It must be inactive.
			CString strTechName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", m_nPendingTechnicianID),"");
			if(!strTechName.IsEmpty()) {
				m_pTechnicianCombo->PutComboBoxText(_bstr_t(strTechName));
			}
		}
		m_nPendingSecondaryProviderID = -1;
	} NxCatchAll("CEMNMoreInfoDlg::OnTrySetSelFinishedEmnTechnician");
}

// (d.lange 2011-03-22 16:47) - PLID 42136 - Assistant/Technician
void CEMNMoreInfoDlg::OnRequeryFinishedEmnTechnician(short nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTechnicianCombo->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{Multiple Assistants}"));
		m_pTechnicianCombo->AddRowBefore(pRow, m_pTechnicianCombo->GetFirstRow());

		pRow = m_pTechnicianCombo->GetNewRow();
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t("{No Assistant}"));
		m_pTechnicianCombo->AddRowBefore(pRow, m_pTechnicianCombo->GetFirstRow());
	} NxCatchAll("CEMNMoreInfoDlg::OnRequeryFinishedEmnTechnician()");
}

// (j.gruber 2007-01-08 11:14) - PLID 23399 - Secondary Providers
void CEMNMoreInfoDlg::OnTrySetSelFinishedEmnSecondary(long nRowEnum, long nFlags) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		ASSERT(m_nPendingSecondaryProviderID != -1);
		if(nFlags == dlTrySetSelFinishedFailure) {
			//It must be inactive.
			CString strProvName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", m_nPendingSecondaryProviderID),"");
			if(!strProvName.IsEmpty()) {
				m_pSecondaryProvCombo->PutComboBoxText(_bstr_t(strProvName));
			}
		}
		m_nPendingSecondaryProviderID = -1;
	} NxCatchAll("Error in OnTrySetSelFinishedEmnSecondary");
}

void CEMNMoreInfoDlg::OnSelChosenEmnSecondary(LPDISPATCH lpRow) 
{
	
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		BOOL bSelectedMultiple = FALSE;
		if(pRow) {
			//Did they select the "{Multiple Providers}" row?
			long nProviderID = VarLong(pRow->GetValue(0));
			if(nProviderID == -1) {
				//Yup.  Let them select which providers they want, this function will update both m_pEMN and the interface 
				// appropriately.
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				SelectSecondaryProviders();
				//TES 5/1/2008 - PLID 27608 - SelectSecondaryProviders() sends the NXM_EMN_MORE_INFO_CHANGED message itself,
				// if necessary (i.e., not if they cancel).  So, don't send it again.
				bSelectedMultiple = TRUE;
			}
			else if (nProviderID == -2) {
				//they want to take one away
				CArray<long,long> arSecondaryProviderIDs;
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				m_pEMN->SetSecondaryProviders(arSecondaryProviderIDs);
			}
			else {
				//Nope, just the one.
				CArray<long,long> arSecondaryProviderIDs;
				arSecondaryProviderIDs.Add(nProviderID);
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				m_pEMN->SetSecondaryProviders(arSecondaryProviderIDs);
			}
		}
		else {
			//Set the array of providers to an empty list.
			CArray<long,long> arSecondaryProviderIDs;
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetSecondaryProviders(arSecondaryProviderIDs);
		}
	} NxCatchAll("Error in OnSelChosenEmnSecondary");
	
}

// (d.lange 2011-03-22 17:15) - PLID 42136 - Event handler for Assistant/Technician row selection
void CEMNMoreInfoDlg::OnSelChosenEmnTechnician(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		BOOL bSelectedMultiple = FALSE;
		if(pRow) {
			long nTechnicianID = VarLong(pRow->GetValue(0));
			if(nTechnicianID == -1) {
				//Selected {Multiple Assistants}
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				SelectAssistantTechnician();
				bSelectedMultiple = FALSE;

			}else if(nTechnicianID == -2) {
				//Selected {No Assistant}
				CArray<long,long> arTechnicianIDs;
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				m_pEMN->SetTechnicians(arTechnicianIDs);
			}else {
				//Selected a single Technician
				CArray<long,long> arTechnicianIDs;
				arTechnicianIDs.Add(nTechnicianID);
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				m_pEMN->SetTechnicians(arTechnicianIDs);
			}
		}else {
			CArray<long,long> arTechnicianIDs;
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetTechnicians(arTechnicianIDs);
		}

	} NxCatchAll("Error in OnSelChosenEmnTechnician");
}

// (c.haag 2008-07-07 11:30) - PLID 30607 - Called when the todo list is done requerying
void CEMNMoreInfoDlg::OnRequeryFinishedEmnTodoList(short nFlags) 
{
	try {
		// (c.haag 2008-07-07 11:30) - Populate the detail column for each row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTodoList->GetFirstRow();
		while (NULL != pRow) {

			// Update the detail column for this row
			// (c.haag 2008-07-11 14:40) - PLID 30689 - Factor in whether it's an EMN (as opposed to EMN detail) task
			CString strDetailName;
			if (ttEMNDetail == (TodoType)VarLong(pRow->Value[tlcRegardingType])) {
				strDetailName = GetEMNTodoDetailName(VarLong(pRow->Value[tlcID], -1), VarLong(pRow->Value[tlcDetailID]));
			} else {
				strDetailName.Empty();
			}
			pRow->Value[tlcDetailName] = _bstr_t(strDetailName);

			pRow = pRow->GetNextRow();
		}

		// (c.haag 2008-07-08 17:22) - Don't forget to colorize if the dialog is not read-only (sigh)
		g_propManager.CachePropertiesInBulk("CEMNMoreInfoDlg_OnRequeryFinishedEmnTodoList", propNumber,
			"(Username = '%s') AND ("
			"Name LIKE 'ToDoColor%%' "
			")",
			_Q(GetCurrentUserName()));

		if (GetRemotePropertyInt("ToDoColorize", 1, 0, GetCurrentUserName(), true) == 1 && m_pEMN->GetStatus() != 2) {
			pRow = m_pTodoList->GetFirstRow();

			while (NULL != pRow) {
				ColorizeEMNTodoListItem(pRow);
				pRow = pRow->GetNextRow();

			} // while (NULL != pRow) {
		}

	}NxCatchAll("CEMNMoreInfoDlg::OnRequeryFinishedEmnTodoList()");	
}

// (c.haag 2008-07-08 17:34) - PLID 30607 - Colorizes a todo item
void CEMNMoreInfoDlg::ColorizeEMNTodoListItem(NXDATALIST2Lib::IRowSettingsPtr& pRow)
{
	if (VT_NULL == pRow->Value[tlcDone].vt) {
		// The todo is not finished
		BYTE nPriority = VarByte(pRow->Value[tlcPriority], 0);
		COLORREF clr;
		switch(nPriority) {
			case 1: clr = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteHigh", RGB(240, 200, 200), 0, GetCurrentUserName(), true); break;
			case 2: clr = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteMedium", RGB(240, 210, 210), 0, GetCurrentUserName(), true); break;
			case 3: default: clr = (COLORREF)GetRemotePropertyInt("ToDoColorIncompleteLow", RGB(240, 220, 220), 0, GetCurrentUserName(), true); break;
		}
		pRow->BackColor = clr;

	} else {
		// The todo is finished
		pRow->BackColor = (COLORREF)GetRemotePropertyInt("ToDoColorComplete", RGB(210, 255, 210), 0, GetCurrentUserName(), true);
	}
}


// (j.gruber 2007-01-08 11:18) - PLID 23399 - Add SEcondary PRoviders
void CEMNMoreInfoDlg::SetSecondaryProviders(const CArray<long,long> &arSecondaryProviderIDs)
{
	// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization - update saved values
	m_lastSecondaryProviders.assign(arSecondaryProviderIDs.GetData(), arSecondaryProviderIDs.GetData() + arSecondaryProviderIDs.GetSize());
	if(arSecondaryProviderIDs.GetSize() <= 1) {
		ShowDlgItem(IDC_EMN_SECONDARY, SW_SHOW);
		ShowDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS, SW_HIDE);

		if(arSecondaryProviderIDs.GetSize() == 0) {
			m_pSecondaryProvCombo->CurSel = NULL;
			m_pSecondaryProvCombo->PutComboBoxText(_bstr_t(""));
			return;
		}

		//At this point we know there's exactly one provider.
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nSel = m_pSecondaryProvCombo->TrySetSelByColumn_Deprecated(0, arSecondaryProviderIDs[0]);
		if(nSel == -1) {
			//It must be inactive.
			CString strProvName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", arSecondaryProviderIDs[0]),"");
			if(!strProvName.IsEmpty()) {
				m_pSecondaryProvCombo->PutComboBoxText(_bstr_t(strProvName));
			}
		}
		else if(nSel == sriNoRowYet_WillFireEvent) {
			//Remember the ID for when we get the event.
			m_nPendingSecondaryProviderID = arSecondaryProviderIDs[0];
		}
	}
	else {
		//Multiple providers
		ShowDlgItem(IDC_EMN_SECONDARY, SW_HIDE);
		CString strProvList;
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr rsProvs = CreateParamRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN ProvidersT ON PersonT.Id = ProvidersT.PersonID WHERE PersonT.ID IN ({INTARRAY}) ORDER BY Last, First, Middle ASC", arSecondaryProviderIDs);
		while(!rsProvs->eof) {
			strProvList += AdoFldString(rsProvs, "Name") + ", ";
			rsProvs->MoveNext();
		}
		rsProvs->Close();
		strProvList = strProvList.Left(strProvList.GetLength()-2);
		m_nxlSecondaryProvLabel.SetText(strProvList);
		m_nxlSecondaryProvLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS, SW_SHOW);
		InvalidateDlgItem(IDC_SELECT_EMN_SECONDARY_PROVIDERS);
	}
}

// (d.lange 2011-03-23 10:08) - PLID 42136 - Assistant/Technician
void CEMNMoreInfoDlg::SetTechnicians(const CArray<long,long> &arTechnicianIDs)
{
	// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization - update saved values
	m_lastTechnicians.assign(arTechnicianIDs.GetData(), arTechnicianIDs.GetData() + arTechnicianIDs.GetSize());
	if(arTechnicianIDs.GetSize() <= 1) {
		ShowDlgItem(IDC_EMN_TECH, SW_SHOW);
		ShowDlgItem(IDC_SELECT_EMN_TECHNICIAN, SW_HIDE);

		if(arTechnicianIDs.GetSize() == 0) {
			m_pTechnicianCombo->CurSel = NULL;
			m_pTechnicianCombo->PutComboBoxText(_bstr_t(""));
			return;
		}

		long nSel = m_pTechnicianCombo->TrySetSelByColumn_Deprecated(0, arTechnicianIDs[0]);
		if(nSel == -1) {
			//This must be inactive
			CString strTechName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", arTechnicianIDs[0]),"");
			if(!strTechName.IsEmpty()) {
				m_pTechnicianCombo->PutComboBoxText(_bstr_t(strTechName));
			}
		}else if(nSel == sriNoRowYet_WillFireEvent) {
			m_nPendingTechnicianID = arTechnicianIDs[0];
		}

	}else {
		ShowDlgItem(IDC_EMN_TECH, SW_HIDE);
		CString strTechList;
		_RecordsetPtr rsTechs = CreateParamRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
													"WHERE PersonT.ID IN ({INTARRAY}) ORDER BY Last, First, Middle ASC", arTechnicianIDs);
		while(!rsTechs->eof) {
			strTechList += AdoFldString(rsTechs, "Name") + ", ";
			rsTechs->MoveNext();
		}
		rsTechs->Close();
		strTechList = strTechList.Left(strTechList.GetLength() - 2);
		m_nxlTechnicianLabel.SetText(strTechList);
		m_nxlTechnicianLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_SELECT_EMN_TECHNICIAN, SW_SHOW);
		InvalidateDlgItem(IDC_SELECT_EMN_TECHNICIAN);
	}
}


long CEMNMoreInfoDlg::GetChartID()
{
	if(m_pEMN != NULL && !m_pChartCombo->IsRequerying()) {
		return m_pEMN->GetChart().nID;
	}
	else {
		if(m_pChartCombo->GetCurSel() == NULL) {
			return -1;
		}
		else {
			return VarLong(m_pChartCombo->GetCurSel()->GetValue(chartID), -1);
		}
	}
}

NXDATALIST2Lib::IRowSettingsPtr CEMNMoreInfoDlg::GetChartRowByID(const long nChartID)
{
	return m_pChartCombo->FindByColumn(chartID, nChartID, NULL, VARIANT_FALSE);
}

boost::unordered_set<long> CEMNMoreInfoDlg::GetLinkedCategoryIDs(const long nChartID)
{
	boost::unordered_set<long> setCategoryIDs;
	if(nChartID != -1)
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = GetChartRowByID(nChartID);
		if(pRow != NULL)
		{
			CString strCategoryIDList = VarString(pRow->GetValue(chartDelimitedCategoryIDs), "");
			while(!strCategoryIDList.IsEmpty())
			{
				int nComma = strCategoryIDList.Find(',');
				if(nComma == -1) {
					setCategoryIDs.insert(atol(strCategoryIDList));
					strCategoryIDList.Empty();
				}
				else {
					setCategoryIDs.insert(atol(strCategoryIDList.Left(nComma)));
					strCategoryIDList.Delete(0, nComma + 1);
				}
			}
		}
	}

	return setCategoryIDs;
}

// (z.manning 2013-06-04 12:37) - PLID 56962 - Renamed this as it no longer does a requery
void CEMNMoreInfoDlg::RefilterCategoryList()
{
	long nChartID = GetChartID();
	boost::unordered_set<long> setCategoryIDs = GetLinkedCategoryIDs(nChartID);
	NXDATALIST2Lib::IRowSettingsPtr pCatRow = m_pCategoryCombo->FindAbsoluteFirstRow(VARIANT_FALSE);
	for(; pCatRow != NULL; pCatRow = m_pCategoryCombo->FindAbsoluteNextRow(pCatRow, VARIANT_FALSE))
	{
		long nCatID = VarLong(pCatRow->GetValue(catID), -1);
		// (z.manning 2013-06-04 16:55) - PLID 56962 - If we don't have a chart selected or if the category
		// is linked to this chart then make the row visible, otherwise hide it.
		if(nCatID == -1 || nChartID == -1 || setCategoryIDs.count(nCatID) > 0) {
			pCatRow->PutVisible(VARIANT_TRUE);
		}
		else {
			pCatRow->PutVisible(VARIANT_FALSE);
		}
	}
}

void CEMNMoreInfoDlg::TrySetCategorySelection(long nCategoryID, BOOL bSilent)
{
	if(nCategoryID == -1) {
		m_pCategoryCombo->PutCurSel(NULL);
		m_pCategoryCombo->PutComboBoxText(_bstr_t(""));
		return;
	}

	// (z.manning 2013-06-05 08:57) - PLID 56962 - We used to call the deprecated try set sel function here
	// but that's no longer necessary as we no longer requery the category combo when selecting a new chart.
	NXDATALIST2Lib::IRowSettingsPtr pCatRow = m_pCategoryCombo->FindByColumn(catID, nCategoryID, NULL, VARIANT_FALSE);
	if(pCatRow != NULL && pCatRow->GetVisible()) {
		m_pCategoryCombo->PutCurSel(pCatRow);
	}
	else {
		m_pEMN->SetCategory(-1, "");
		m_pCategoryCombo->PutCurSel(NULL);
		m_pCategoryCombo->PutComboBoxText(_bstr_t(""));
		if(!bSilent) {
			// (z.manning 2013-06-05 09:23) - PLID 56962 - The previously selected category is not linked to the new
			// chart so warn the user that we cleared the category.
			MessageBox("This chart/category combination is not valid. There is no longer a category selected on this EMN.", NULL, MB_ICONINFORMATION);
		}
	}
}

void CEMNMoreInfoDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	/*try {
		CNxDialog::OnShowWindow(bShow, nStatus);
		if (bShow) {
			UpdateChangedInfo();
		}
		
	} NxCatchAll("Error in CEMNMoreInfoDlg::OnShowWindow()");*/
}

// (a.walling 2007-07-09 13:04) - PLID 23714 - Update any changed information based on the table checkers
void CEMNMoreInfoDlg::UpdateChangedInfo()
{
	try
	{


		if (m_checkProcedures.Changed()) {
			m_pProcedureCombo->Requery();
			RefreshProcedureList(); // (a.walling 2007-10-03 14:11) - PLID 23714
		}

		if (m_checkProviders.Changed()) {
			CString strLicensedProviders;
			CDWordArray dwaLicensedProviders;
			g_pLicense->GetUsedEMRProviders(dwaLicensedProviders);
			for(int i = 0; i < dwaLicensedProviders.GetSize(); i++) {
				strLicensedProviders += FormatString("%i,",dwaLicensedProviders[i]);
			}
			strLicensedProviders.TrimRight(",");
			if(strLicensedProviders.IsEmpty()) strLicensedProviders = "-1";
			m_pProviderCombo->WhereClause = _bstr_t("PersonT.Archived = 0 AND PersonT.ID IN (" + strLicensedProviders + ")");
			m_pProviderCombo->Requery();

			CArray<long,long> arProviderIDs;
			m_pEMN->GetProviders(arProviderIDs);
			SetProviders(arProviderIDs);

			m_pSecondaryProvCombo->Requery();
			CArray<long,long> arSecondaryProviderIDs;
			m_pEMN->GetSecondaryProviders(arSecondaryProviderIDs);
			SetSecondaryProviders(arSecondaryProviderIDs);
		}

		// (d.lange 2011-03-24 11:54) - PLID 42136 - Let's refresh the list of Assistant/Tech
		if (m_checkTechnicians.Changed()) {
			m_pTechnicianCombo->Requery();
			CArray<long,long> arTechnicianIDs;
			m_pEMN->GetTechnicians(arTechnicianIDs);
			SetTechnicians(arTechnicianIDs);
		}

		if (m_checkLocations.Changed()) {
			m_nPendingLocationID = m_pEMN->GetLocationID();
			m_pLocationCombo->Requery();
			SetLocation(m_nPendingLocationID);
		}

		if(m_checkCharts.Changed() || m_checkCategoryChartLink.Changed()) {
			m_pChartCombo->Requery();
			m_checkCharts.m_changed = false;
			m_checkCategoryChartLink.m_changed = false;
		}

		if(m_checkCategories.Changed()) {
			m_pCategoryCombo->Requery();
			m_checkCategories.m_changed = false;
		}


		// (a.walling 2012-03-22 16:50) - PLID 49141 - Anything that changed should already notify via one of the functions above
	} NxCatchAllThrow("Error in CEMNMoreInfoDlg::UpdateChangedInfo()");
}

void CEMNMoreInfoDlg::OnDestroy() 
{
	try {
		CNxDialog::OnDestroy();
		
		// (a.walling 2007-07-03 13:07) - PLID 23714
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
	} NxCatchAll("Error in CEMNMoreInfoDlg::OnDestroy()");
}

// (a.walling 2007-07-03 13:09) - PLID 23714
LRESULT CEMNMoreInfoDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// If we are not visible, don't bother updating. It will be updated in OnShowWindow.
		// Otherwise try to update the lists.
		if (IsWindowVisible()) {
			UpdateChangedInfo();
			// (c.haag 2008-07-07 12:54) - PLID 30607 - If a todo item has changed,
			// we require a greater precision than just ::Changed() for practical
			// purposes.
			if (!m_bIsTemplate && NetUtils::TodoList == wParam) {
				RefreshTodoListItem(lParam);
			}
		}
	} NxCatchAll("Error in CEMNMoreInfoDlg::OnTableChanged");
	return 0;
}

// (a.walling 2007-07-13 09:26) - PLID 26640 - Configure fields to show or hide in the more info.
void CEMNMoreInfoDlg::OnConfigPreview() 
{
	try {		
		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(!pTreeWnd || !m_pEMN) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		// (a.walling 2008-10-14 10:22) - PLID 31678 - Configure the preview via the preview control
		if (GetEmrPreviewCtrl()) {
			GetEmrPreviewCtrl()->ConfigurePreview();
		}
		else {
			//TES 3/10/2014 - PLID 61313 - If we don't have a preview control (presumably because this is a template),
			// just pop up the dialog directly.
			CConfigEMRPreviewDlg dlg(this);
			dlg.DoModal();
		}
	} NxCatchAll("Error in OnConfigPreview");
}

// (j.jones 2007-08-06 15:20) - PLID 26974 - added ability to update the saved patient info. for this EMN
void CEMNMoreInfoDlg::OnBtnUpdatePatInfo() 
{
	try {

		if(!m_pEMN) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(m_bIsTemplate)
			return;

		if(IDNO == MessageBox("This action will update the patient information on this EMN with the latest information from General 1.\n"
			"Are you sure you wish to do this?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		// (a.walling 2012-06-11 09:27) - PLID 50922 - Update patient demographics
		m_pEMN->UpdatePatientDemographics(this);

		//now display the new data
		RefreshPatientDemographics();

	}NxCatchAll("Error in CEMNMoreInfoDlg::OnBtnUpdatePatInfo");
}

// (j.jones 2008-05-20 13:14) - PLID 30079 - added ability to left click on a medication and edit it
// in the prescription editor
void CEMNMoreInfoDlg::OnLButtonDownMedications(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {

		if(nRow == -1) {
			return;
		}

		if(nCol == mclMedication) {

			EMNMedication* pMedication = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
			if (pMedication) {

				//edit the medication in the prescription editor

				//if it is locked, check that first, because then we don't need
				//to check the permissions
				// (a.walling 2008-08-22 16:32) - PLID 23138 - Must not be able to edit this if readonly
				BOOL bReadOnly = (m_pEMN->GetStatus() == 2) || m_bReadOnly;
				if(!bReadOnly && !m_bReadOnly) {
					
					//it is not locked, so check write permissions

					//if they don't have any, it will open as read-only
					BOOL bHasWritePermissions = FALSE;
					if((GetCurrentUserPermissions(bioPatientMedication) & sptWrite) ||
						((GetCurrentUserPermissions(bioPatientMedication) & sptWriteWithPass)
						&& CheckCurrentUserPassword())) {

						//they have write permissions
						bReadOnly = FALSE;
					}
					else {
						bReadOnly = TRUE;

						AfxMessageBox("You do not have permission to edit prescriptions. The prescription will be opened for viewing only.");
					}
				}

				// (j.gruber 2009-03-30 11:45) - PLID 33736 - make sure if its new crop, its read only
				if(!pMedication->strNewCropGUID.IsEmpty()) {
					bReadOnly = TRUE;
					AfxMessageBox("This prescription was written through E-Prescribing and cannot be modified in Practice. The prescription will be opened for viewing only.");
				}

				// (j.jones 2012-10-01 10:13) - PLID 52922 - track the diagnoses
				CArray<long,long> aryOldDiagIDs;
				{
					for(int d=0; d<pMedication->aryDiagnoses.GetSize();d++) {
						aryOldDiagIDs.Add(pMedication->aryDiagnoses.GetAt(d).nID);
					}
				}

				// (j.jones 2012-10-31 11:28) - PLID 52818 - we must force a save before opening the queue
				CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
				//TES 2/12/2014 - PLID 60740 - No need to check IsMoreInfoUnsaved() if we've already checked IsUnsaved()
				if(pTreeWnd && (m_pEMN->IsUnsaved() || m_pEMN->GetID() == -1)) {
					if(FAILED(pTreeWnd->SaveEMR(esotEMN, (long)m_pEMN, FALSE))) {
						//Warn if this happened. Presumably the user has already been told why the save failed.
						CString strWarn;
						strWarn.Format("The EMN must be saved before a medication can be edited. "
							"Because the save was cancelled, the prescription for %s cannot be opened.", pMedication->m_strDrugName);
						MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_OK);
						return;
					}
				}

				// (j.jones 2012-10-31 15:15) - PLID 52819 - provide the EMN ID and template status
				CPrescriptionEditDlg dlg(this, m_pEMN->GetID(), m_bIsTemplate);
				// (j.jones 2012-10-24 17:03) - PLID 52819 - changed to just EditPrescription, we no longer update the medication pointer
				// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription,
				// though the result is not actually used here
				// (j.fouts 2013-03-12 10:19) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
				PrescriptionInfo rxInformation = LoadFullPrescription(pMedication->nID, !!m_bIsTemplate);

				// (j.jones 2013-11-25 09:55) - PLID 59772 - this does not need the drug interaction info		
				// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
				PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(false, rxInformation, NULL, bReadOnly);
				// (j.jones 2012-10-24 17:07) - PLID 52819 - do nothing, we will always reload all medications
				// after editing any prescription
				/*
				if(epdrvReturn != epdrvDeleteRx && !bReadOnly) {

					//update the row
					//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation
					m_MedicationList->PutValue(nRow, mclExplanation, (LPCTSTR)(pMedication->strPatientExplanation));
					m_MedicationList->PutValue(nRow, mclRefills, (LPCTSTR)(pMedication->strRefillsAllowed));
					//TES 2/10/2009 - PLID 33034 - Renamed PillsPerBottle to Quantity
					m_MedicationList->PutValue(nRow, mclPills, (LPCTSTR)(pMedication->strQuantity));
					//TES 2/12/2009 - PLID 33034 - Added Strength
					//TES 3/31/2009 - PLID 33750 - Removed Strength
					m_MedicationList->PutValue(nRow, mclUnit, (LPCTSTR)(pMedication->strUnit));
					//TES 2/12/2009 - PLID 33034 - Added Dosage Form
					//TES 3/31/2009 - PLID 33750 - Removed Dosage Form

					// (a.walling 2009-04-22 14:41) - PLID 33948 - they hit OK, so set this as reviewed
					if (pMedication->bEPrescribe) {
						pMedication->bEPrescribeReviewed = TRUE;
					}
					
					// (a.walling 2012-03-22 16:50) - PLID 49141 - notify the interface
					m_pEMN->SetMoreInfoUnsaved();

					// (j.jones 2012-10-01 09:27) - PLID 52922 - If the diagnosis list changed,
					// then changes to this medication could affect drug interactions.
					CEmrTreeWnd *pTree = m_pEMN->GetInterface();
					if(pTree != NULL) {

						//compare to the new diagnoses
						BOOL bDiagsChanged = FALSE;
						if(aryOldDiagIDs.GetSize() != pMedication->aryDiagnoses.GetSize()) {
							bDiagsChanged = TRUE;
						}
						else if(aryOldDiagIDs.GetSize() > 0 && aryOldDiagIDs.GetSize() == pMedication->aryDiagnoses.GetSize()) {
							//see if any changed
							for(int d=0; d<pMedication->aryDiagnoses.GetSize() && !bDiagsChanged; d++) {
								long nNewID = pMedication->aryDiagnoses.GetAt(d).nID;
								BOOL bFound = FALSE;
								for(int e=0; e<aryOldDiagIDs.GetSize() && !bFound; e++) {
									if(aryOldDiagIDs.GetAt(e) == nNewID) {
										bFound = TRUE;
									}
								}

								if(!bFound) {								
									bDiagsChanged = TRUE;
								}
							}
						}

						if(!m_bIsTemplate && bDiagsChanged && !m_pEMN->IsLoading()) {
							// This function will check their preference to save the EMN immediately,
							// potentially do so, and warn about drug interactions.
							// (j.jones 2012-11-13 10:10) - PLID 52869 - changed to be a posted message
							// (j.jones 2013-02-06 16:28) - PLID 55045 - Medication changes save immediately now,
							// so pass in TRUE for the lParam to tell this function that we already saved the changes
							// and the interactions should open if it was otherwise waiting for a save to succeed.
							m_pEMN->CheckSaveEMNForDrugInteractions(TRUE);
						}
					}
				}
				*/

				// (j.jones 2012-10-22 17:40) - PLID 52819 - Editing one prescription doesn't display the queue,
				// so we know other prescriptions on other EMNs would not have changed.
				// So just reload this EMNs medications.
				if(!m_pEMN->IsLoading()) {

					// (j.jones 2013-02-07 09:56) - PLID 55045 - ReloadMedicationsFromData will automatically
					// tell drug interactions to refresh, so we do not need to do so from here
					m_pEMN->ReloadMedicationsFromData(TRUE);
				}

			} else {
				ASSERT(FALSE);
				ThrowNxException("Failed to get a valid medication object!");
			}
		}
		// (j.jones 2008-07-22 12:02) - PLID 30792 - if they click on a problem icon, display the problems for this
		// medication in the problem list
		else if(nCol == mclProblemIcon) {

			m_MedicationList->CurSel = nRow;
			EMNMedication* pMedication = (EMNMedication*)VarLong(m_MedicationList->GetValue(nRow, mclObjectPtr));
			if(pMedication) {

				// (c.haag 2009-05-20 10:55) - PLID 34312 - Use new problem structure
				if(pMedication->m_apEmrProblemLinks.GetSize() == 0) {
					AfxMessageBox("There are no problems currently on this medication.");
					return;
				}

				EditMedicationProblems();
			}
			else {
				ASSERT(FALSE);
				ThrowNxException("Failed to get a valid medication object!");
			}
		}

	}NxCatchAll("Error in CEMNMoreInfoDlg::OnLButtonDownMedications");	
}

void CEMNMoreInfoDlg::OnPrintPrescriptionsToPrinter() 
{
	try {

		//TES 6/17/2008 - PLID 30411 - Save this preference.
		BOOL bSave = IsDlgButtonChecked(IDC_PRINT_PRESCRIPTIONS_TO_PRINTER);
		SetRemotePropertyInt("MoreInfoPrintPrescriptionsToPrinter", bSave?1:0, 0, GetCurrentUserName());

	}NxCatchAll("Error in CEMNMoreInfoDlg::OnPrintPrescriptionsToPrinter()");
}

// (c.haag 2008-07-07 14:42) - PLID 30607 - Called when the user double-clicks on the todo list
void CEMNMoreInfoDlg::OnDblClickCellEmnTodoList(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2008-07-07 17:56) - PLID 30632 - Ignore for read-only EMN's
	try {
		// Quit immediately if we happen to be a template
		if (m_bIsTemplate) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow && !m_bReadOnly) {
			m_pTodoList->CurSel = pRow;
			OnEditEMNTodo();
		}
	}
	NxCatchAll("Error in CEMNMoreInfoDlg::OnDblClickCellEmnTodoList");
}

// (c.haag 2008-07-07 14:46) - PLID 30607 - Called when a user wants to invoke the pop-up
// menu over a todo alarm row
void CEMNMoreInfoDlg::OnRButtonDownEmnTodoList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-07-07 17:56) - PLID 30632 - Ignore for read-only EMN's
	try {
		// Quit immediately if we happen to be a template
		if (m_bIsTemplate) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow && !m_bReadOnly) {
			m_pTodoList->CurSel = pRow;
			// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
			CNxMenu mPopup;
			mPopup.CreatePopupMenu();
			mPopup.InsertMenu(0, MF_BYPOSITION, ID_EDIT_EMN_TODO, "Edit");
			mPopup.InsertMenu(1, MF_BYPOSITION, ID_DELETE_EMN_TODO, "Delete");
			mPopup.SetDefaultItem(0, TRUE);

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll("Error in CEMNMoreInfoDlg::OnRButtonDownEmnTodoList");
}

// (c.haag 2008-07-07 14:48) - PLID 30607 - Called to modify a todo alarm
void CEMNMoreInfoDlg::OnEditEMNTodo()
{
	try {
		// (c.haag 2008-07-07 20:29) - 30632 - We can't change read-only EMN's
		if (m_bReadOnly) {
			ASSERT(FALSE);
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTodoList->GetCurSel();
		// (c.haag 2008-07-08 12:24) - PLID 30641 - The dialog now checks itself for write permissions
		if (NULL != pRow && !m_bReadOnly) {
			CTaskEditDlg dlg(this);			
			dlg.m_nPersonID = m_nPatientID; // (a.walling 2008-07-07 17:50) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
			dlg.m_iTaskID = VarLong(pRow->GetValue(tlcID));
			// (c.haag 2008-07-11 10:00) - PLID 30550 - Now that the dialog can flag whether it's opened
			// from an EMN, we need to set that flag so EMR-less todo permissions will not be checked
			dlg.m_bInvokedFromEMN = TRUE;
			//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
			dlg.m_bIsNew = FALSE;

			if(IDOK == dlg.DoModal()) {

				// (c.haag 2008-08-18 16:48) - PLID 30607 - If the item was deleted, we need to remove it from the EMN
				if (dlg.m_bWasDeleted) {
					CArray<EMNTodo*,EMNTodo*> apTodos;
					BOOL bFound = FALSE;
					m_pEMN->GenerateCreatedTodosWhileUnsavedList(apTodos);
					for (int i=0; i < apTodos.GetSize() && !bFound; i++) {
						if (apTodos[i]->nTodoID == dlg.m_iTaskID) {
							bFound = TRUE;
						}
					}
					if (bFound) {
						CArray<long,long> an; // This won't be touched in the function
						_variant_t v; // This won't be touched in the function
						SourceActionInfo sai;
						m_pEMN->AddDeletedTodoWhileUnsaved(dlg.m_iTaskID, NULL, v, NULL,
							v,v,v,v, v,v,v,v, v,v,v,v, an,v);
					}

					// (c.haag 2008-08-19 17:51) - PLID 30607 - Ensure the EMN and more info are unsaved
					//TES 2/26/2014 - PLID 60972 - Call SetMoreInfoUnsaved(), not SetUnsaved()
					m_pEMN->SetMoreInfoUnsaved();

					// (c.haag 2008-08-19 12:19) - PLID 30696 - Notify the tree window
					GetEmrTreeWnd()->PostMessage(NXM_EMN_TODO_DELETED, (WPARAM)m_pEMN);
				}
				// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
				RefreshTodoListItem(dlg.m_iTaskID);
				if (dlg.m_anAssignTo.GetSize() == 1){
					CClient::RefreshTodoTable( dlg.m_iTaskID,dlg.m_nPersonID, dlg.m_anAssignTo[0], TableCheckerDetailIndex::tddisDeleted);
				}
				else{
					CClient::RefreshTodoTable(dlg.m_iTaskID, dlg.m_nPersonID,-1, TableCheckerDetailIndex::tddisDeleted);
				}
			}
		}
	}
	NxCatchAll("Error in CEMNMoreInfoDlg::OnEditEMNTodo");
}

// (c.haag 2008-07-07 14:48) - PLID 30607 - Called to delete a todo alarm
void CEMNMoreInfoDlg::OnDeleteEMNTodo()
{
	try {
		// (c.haag 2008-07-07 20:29) - 30632 - We can't change read-only EMN's
		if (m_bReadOnly) {
			ASSERT(FALSE);
			return;
		}

		// (c.haag 2008-07-11 09:20) - PLID 17244 - Do not check todo permissions. Having EMR write
		// permissions is enough.

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTodoList->GetCurSel();
		if (NULL != pRow && !m_bReadOnly) {

			CString strAssigneeIDs = VarString(pRow->GetValue(tlcAssigneeIDs), "");
			CArray<long,long> arynAssignTo;
			ParseDelimitedStringToLongArray(strAssigneeIDs, " ", arynAssignTo);
			if (!CheckAssignToPermissions(arynAssignTo, sptDelete)) {
				return;
			}

			// Have the user confirm they want to delete the task
			if (IDNO == MsgBox(MB_ICONQUESTION | MB_YESNO, "Are you sure you want to delete this task?")) {
				return;
			}

			// (c.haag 2008-07-14 12:39) - PLID 30607 - Also needs auditing
			long nTaskID = VarLong(pRow->GetValue(tlcID));
			// (z.manning 2010-02-25 11:41) - PLID 37532 - SourceDetailImageStampID
			_RecordsetPtr prs = CreateParamRecordset("SELECT TodoList.TaskID, RegardingID, RegardingType, Remind, Done, EnteredBy, PersonID, Priority, Task, LocationID, CategoryID, dbo.GetTodoAssignToIDString(TodoList.TaskID) AS AssignToIDs, dbo.GetTodoAssignToNamesString(TodoList.TaskID) AS AssignToNames, Deadline, (SELECT Description FROM NoteCatsF WHERE ID = TodoList.CategoryID) AS Category, Notes, SourceActionID, SourceDataGroupID, SourceDetailImageStampID FROM TodoList LEFT JOIN EmrTodosT ON EmrTodosT.TaskID = TodoList.TaskID WHERE TodoList.TaskID = {INT}", nTaskID);
			if (!prs->eof) {
				FieldsPtr f = prs->Fields;
				CString strAssignTo = AdoFldString(f, "AssignToNames");
				CString strCategory = AdoFldString(f, "Category", "");
				CString strNotes = AdoFldString(f, "Notes");
				// CR and LF's don't show up in the audit trail elegantly; replace them with spaces or else the note will look ugly
				strNotes.Replace(10, ' ');
				strNotes.Replace(13, ' ');
				COleDateTime dtDeadline = AdoFldDateTime(f, "Deadline");
				CString strOld = FormatString("Assigned To: %s, Deadline: %s, Category: %s, Note: %s", 
					strAssignTo, FormatDateTimeForInterface(dtDeadline, 0, dtoDate), strCategory, strNotes);

				// Audit the deletion
				long nAuditID = -1;
				try {
					nAuditID = BeginAuditTransaction();
					AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), nAuditID, aeiPatientTodoTaskDelete, nTaskID, strOld, "<Deleted>", aepMedium, aetDeleted);

					// (c.haag 2008-07-14 12:39) - PLID 30696 - "Remember" that the todo was deleted so we can
					// recover it if the user cancels out of the EMN
					CArray<long,long> anAssignToIDs;
					CString strAssignToIDs = AdoFldString(f, "AssignToIDs");
					TodoType RegardingType = (TodoType)AdoFldLong(f, "RegardingType");
					long nRegardingID = AdoFldLong(f, "RegardingID");
					ParseDelimitedStringToLongArray(strAssignToIDs, " ", anAssignToIDs);

					m_pEMN->AddDeletedTodoWhileUnsaved(nTaskID, 
						(ttEMNDetail == RegardingType) ? m_pEMN->GetDetailByID(nRegardingID) : NULL,
						AdoFldLong(f, "SourceActionID", -1),
						//TES 3/17/2010 - PLID 37530 - Pass in -1 for SourceStampID and SourceStampIndex
						&TableRow(AdoFldLong(f, "SourceDataGroupID", -1), AdoFldLong(f, "SourceDetailImageStampID", -1), -1, -1), // (z.manning 2009-02-26 15:53) - PLID 33141
						f->Item["Remind"]->Value,
						f->Item["Done"]->Value,
						f->Item["Deadline"]->Value,
						f->Item["EnteredBy"]->Value,
						f->Item["PersonID"]->Value,
						f->Item["Notes"]->Value,
						f->Item["Priority"]->Value,
						f->Item["Task"]->Value,
						f->Item["LocationID"]->Value,
						f->Item["CategoryID"]->Value,
						f->Item["RegardingID"]->Value,
						f->Item["RegardingType"]->Value,
						anAssignToIDs,
						_bstr_t(strAssignTo)
						);
					

					// Delete the task
					TodoDelete(nTaskID, TRUE);

					// Commit the audit transaction
					CommitAuditTransaction(nAuditID);

					// (c.haag 2008-08-19 17:51) - PLID 30607 - Ensure the EMN and more info are unsaved
					//TES 2/26/2014 - PLID 60972 - Call SetMoreInfoUnsaved(), not SetUnsaved()
					m_pEMN->SetMoreInfoUnsaved();

					// (c.haag 2008-07-14 16:36) - PLID 30696 - Notify the tree window
					GetEmrTreeWnd()->PostMessage(NXM_EMN_TODO_DELETED, (WPARAM)m_pEMN);

					// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
					if (anAssignToIDs.GetSize() == 1){
						CClient::RefreshTodoTable(nTaskID, VarLong(f->Item["PersonID"]->Value, -1), anAssignToIDs[0], TableCheckerDetailIndex::tddisDeleted);
					}
					else{
						CClient::RefreshTodoTable(nTaskID, VarLong(f->Item["PersonID"]->Value, -1), -1, TableCheckerDetailIndex::tddisDeleted);
					}

					prs->Close();
					RemoveTodoListItem(nTaskID);
				} 
				NxCatchAllSilentCallThrow(if(nAuditID==-1){RollbackAuditTransaction(nAuditID);})
			} // if (!prs->eof) { 			
		} // if (NULL != pRow && !m_bReadOnly) {
	}
	NxCatchAll("Error in CEMNMoreInfoDlg::OnDeleteEMNTodo");
}



// (j.jones 2008-07-22 09:05) - PLID 30792 - this function will show or hide
// the problem icon column in the medication list, based on whether any meds have the icon
void CEMNMoreInfoDlg::ShowHideProblemIconColumn_MedicationList()
{
	try {

		//see if any row has an icon, and if so, show the column, otherwise hide it

		BOOL bHasIcon = FALSE;
		for(int i=0; i<m_MedicationList->GetRowCount() && !bHasIcon; i++) {
			long nIcon = VarLong(m_MedicationList->GetValue(i, mclProblemIcon), 0);
			if(nIcon != 0) {
				bHasIcon = TRUE;
			}
		}

		NXDATALISTLib::IColumnSettingsPtr pCol = m_MedicationList->GetColumn(mclProblemIcon);

		if(bHasIcon) {
			//show the column
			pCol->PutStoredWidth(20);
		}
		else {
			//hide the column
			pCol->PutStoredWidth(0);
		}

	}NxCatchAll("Error in CEMNMoreInfoDlg::ShowHideProblemIconColumn_MedicationList");
}



// (j.jones 2008-07-25 09:00) - PLID 30792 - placed problem editing into their own functions
void CEMNMoreInfoDlg::EditMedicationProblems()
{
	try {

		if(m_MedicationList->GetCurSel() != -1) {

			IRowSettingsPtr pRow = m_MedicationList->GetRow(m_MedicationList->GetCurSel());
			EMNMedication* pMedication = (EMNMedication*)VarLong(pRow->GetValue(mclObjectPtr));

			if(pMedication == NULL) {
				ThrowNxException("View Medication Problems - failed because no medication object was found!");
			}

			// (c.haag 2009-05-26 10:25) - PLID 34312 - Use new problem structure
			if(pMedication->m_apEmrProblemLinks.GetSize() == 0) {
				AfxMessageBox("This medication has no problems.");
				return;
			}

			//close the current problem list, if there is one
			CMainFrame *pFrame = GetMainFrame();
			if(pFrame) {
				pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
			}

			//now open filtered on this medication
			CEMRProblemListDlg dlg(this);
			dlg.SetDefaultFilter(m_pEMN->GetParentEMR()->GetPatientID(), eprtEmrMedication, pMedication->nID, pMedication->m_strDrugName);
			// (c.haag 2009-05-26 10:25) - PLID 34312 - Use new problem structure
			dlg.LoadFromProblemList(GetEmrTreeWnd(), &pMedication->m_apEmrProblemLinks);
			dlg.DoModal();

			//try to update the icon
			_variant_t varProblemIcon = g_cvarNull;
			BOOL bChanged = FALSE;

			if(pMedication->HasProblems()) {
				if(pMedication->HasOnlyClosedProblems()) {
					HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
					varProblemIcon = (long)hProblem;
				}
				else {
					HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
					varProblemIcon = (long)hProblem;
				}
			}
			else {
				bChanged = TRUE;
			}
			pRow->PutValue(mclProblemIcon, varProblemIcon);

			ShowHideProblemIconColumn_MedicationList();

			//also see if any problem changed, if so, mark the medication as changed
			if(bChanged || pMedication->HasChangedProblems()) {

				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);

				if(!m_pEMN->IsLockedAndSaved()) {

					// (j.jones 2013-01-07 16:45) - PLID 52819 - this should not be needed, we should still
					// mark more info as changed, but we don't need to flag the medication as having changed
					//pMedication->bChanged = TRUE;

					// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
					m_pEMN->SetMoreInfoUnsaved();
				}
			}
		}

	} NxCatchAll("Error in CEMNMoreInfoDlg::EditMedicationProblems");
}

// (c.haag 2009-05-28 15:47) - PLID 34312 - Link the selected medication with existing problems
void CEMNMoreInfoDlg::LinkMedicationProblems()
{
	try {
		if(m_MedicationList->GetCurSel() != -1) 
		{
			IRowSettingsPtr pRow = m_MedicationList->GetRow(m_MedicationList->GetCurSel());
			EMNMedication* pMedication = (EMNMedication*)VarLong(pRow->GetValue(mclObjectPtr));
			CArray<CEmrProblemLink*,CEmrProblemLink*> aNewProblemLinks;
			int i;

			if(pMedication == NULL) {
				ThrowNxException("View Medication Problems - failed because no medication object was found!");
			}

			for (i=0; i < pMedication->m_apEmrProblemLinks.GetSize(); i++) {
				pMedication->m_apEmrProblemLinks[i]->UpdatePointersWithMedication(m_pEMN, pMedication);
			}
			if (LinkProblems(pMedication->m_apEmrProblemLinks, eprtEmrMedication, pMedication->nID, aNewProblemLinks)) {
				for (i=0; i < aNewProblemLinks.GetSize(); i++) {
					aNewProblemLinks[i]->UpdatePointersWithMedication(m_pEMN, pMedication);
					pMedication->m_apEmrProblemLinks.Add(aNewProblemLinks[i]);
				}

				//try to update the icon
				_variant_t varProblemIcon = g_cvarNull;
				BOOL bChanged = FALSE;

				if(pMedication->HasProblems()) {
					if(pMedication->HasOnlyClosedProblems()) {
						HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
						varProblemIcon = (long)hProblem;
					}
					else {
						HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
						varProblemIcon = (long)hProblem;
					}
				}
				else {
					bChanged = TRUE;
				}
				pRow->PutValue(mclProblemIcon, varProblemIcon);

				ShowHideProblemIconColumn_MedicationList();

				//also see if any problem changed, if so, mark the medication as changed
				if(bChanged || pMedication->HasChangedProblems()) {

					// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
					GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);

					if(!m_pEMN->IsLockedAndSaved()) {

						// (j.jones 2013-01-07 16:45) - PLID 52819 - this should not be needed, we should still
						// mark more info as changed, but we don't need to flag the medication as having changed
						//pMedication->bChanged = TRUE;

						// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
						m_pEMN->SetMoreInfoUnsaved();
					}
				}
			}
		}

	} NxCatchAll("Error in CEMNMoreInfoDlg::LinkMedicationProblems");
}

void CEMNMoreInfoDlg::HandleProblemChange(CEmrProblem *pChangedProblem)
{
	//TES 10/30/2008 - PLID 31269 - A problem has changed, so refresh all our problem icons.
	
	//TES 10/30/2008 - PLID 31269 - First, diag codes.
	int i = 0;

	




	//TES 10/30/2008 - PLID 31269 - Finally, medications.
	for(i = 0; i < m_MedicationList->GetRowCount(); i++) {
		IRowSettingsPtr pRow = m_MedicationList->GetRow(i);
		_variant_t varProblemIcon = g_cvarNull;
		EMNMedication *pMed = (EMNMedication*)VarLong(pRow->GetValue(mclObjectPtr));
		if(pMed->HasProblems()) {
			if(pMed->HasOnlyClosedProblems()) {
				HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
				varProblemIcon = (long)hProblem;
			}
			else {
				HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
				varProblemIcon = (long)hProblem;
			}
		}
		pRow->PutValue(mclProblemIcon, varProblemIcon);
	}

	ShowHideProblemIconColumn_MedicationList();

}
// (j.gruber 2009-05-11 17:59) - PLID 33688 - added other providers
void CEMNMoreInfoDlg::OnBnClickedEmnOtherProvs()
{

	try {

		//create a temporary array
		CArray<EMNProvider*, EMNProvider*> arOtherProvs;
		m_pEMN->GetOtherProviders(arOtherProvs);
		CEditEMNOtherProvidersDlg dlg(&arOtherProvs, this);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {
			//copy the arOtherProvs into the main list
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetOtherProviders(arOtherProvs);	
			
		}

		//clear the temporary array
		for (int i = arOtherProvs.GetSize() - 1; i >= 0 ; i--) {

			EMNProvider * pProv = arOtherProvs.GetAt(i);
			arOtherProvs.RemoveAt(i);

			delete pProv;
		}
	}NxCatchAll("Error in CEMNMoreInfoDlg::OnBnClickedEmnOtherProvs()");

}

// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info now in top level frame

// (c.haag 2009-05-28 15:22) - PLID 34312 - This function is called when the user wants to link something
// in the more info section with one or more existing problems. Returns TRUE if at least one problem was
// added to the output array
BOOL CEMNMoreInfoDlg::LinkProblems(CArray<CEmrProblemLink*,CEmrProblemLink*>& aryEMRObjectProblemLinks, 
								  EMRProblemRegardingTypes eprtType, long nEMRRegardingID,
								  OUT CArray<CEmrProblemLink*,CEmrProblemLink*>& aNewProblemLinks)
{
	CArray<CEmrProblem*,CEmrProblem*> aryAllProblems;
	CArray<CEmrProblem*,CEmrProblem*> aryEMRObjectProblems;
	CArray<CEmrProblem*,CEmrProblem*> arySelectionsInMemory;
	CArray<long,long> arynSelectionsInData;
	int i;

	CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
	if (NULL == m_pEMN || NULL == pEMR) {
		ThrowNxException("Called CEMNMoreInfoDlg::LinkProblems without a valid EMN or EMR");
	}

	// We require problem create permissions to create new links
	if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
		return FALSE;
	}

	// Pull ALL problems, including deleted ones, because the chooser dialog will run a query
	// that must be filtered by both deleted and non-deleted problems in memory
	pEMR->GetAllProblems(aryAllProblems, TRUE);
	for (i=0; i < aryEMRObjectProblemLinks.GetSize(); i++) {
		EnsureProblemInArray(aryEMRObjectProblems, aryEMRObjectProblemLinks[i]->GetProblem());
	}

	CEMRProblemChooserDlg dlg(aryAllProblems, aryEMRObjectProblems, pEMR->GetPatientID(), this);
	if (dlg.HasProblems()) {
		if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
			// User chose at least one problem
			
			// First, go through the problems that already exist in memory (and owned
			// by this EMR), and create problem links for them.
			for (i=0; i < arySelectionsInMemory.GetSize(); i++) {
				CEmrProblemLink* pNewLink = new CEmrProblemLink(arySelectionsInMemory[i], -1, eprtType, nEMRRegardingID, -1);
				aNewProblemLinks.Add(pNewLink);
			}
			// Next, go through the problems that exist in data and not in memory, create
			// problem objects for them, and then links for those. Here's the neat part:
			// If the problem is already in memory, but linked with nothing; then the EMR
			// is smart enough to just give you the problem already in memory when calling
			// the function to allocate it.
			// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
			// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			if (arynSelectionsInData.GetSize() > 0) {
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
				_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
					"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, PatientID, EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, "
					"EmrProblemsT.DoNotShowOnProblemPrompt "
					"FROM EMRProblemsT "
					"WHERE Deleted = 0 AND ID IN ({INTARRAY})", arynSelectionsInData);
				while (!prs->eof) {
					CEmrProblem* pNewProblem = pEMR->AllocateEmrProblem(prs->Fields);
					CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtType, nEMRRegardingID, -1);
					aNewProblemLinks.Add(pNewLink);
					pNewProblem->Release();
					prs->MoveNext();
				}
			}

			// (c.haag 2009-06-01 12:56) - PLID 34312 - Because we added or modified a problem or problem link, we must flag
			// the EMR as modified.
			pEMR->SetUnsaved();

		}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
		else {
			// User changed their mind
		}
	} else {
		// Dialog has no visible problems
		AfxMessageBox("There are no available problems to choose from.");
	}

	return (aNewProblemLinks.GetSize() > 0) ? TRUE : FALSE;
}


void CEMNMoreInfoDlg::OnSpellCheckNotes()
{
	try {
		//TES 10/5/2010 - PLID 40183 - Added a spellchecker for the More Info notes.
		SpellCheckEditControl(this, &m_nxeditEditEmrNotes);

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-28 14:31) - PLID 61007  causing issue on tab order. 
//I am rmoving call to Parent PreTranslateMessage (pParent->PreTranslateMessage(pMsg))


// (j.jones 2012-02-17 10:33) - PLID 47886 - Added ability to hide unbillable service codes,


// (a.walling 2012-03-23 09:13) - PLID 49154 - Update the interface while idle
LRESULT CEMNMoreInfoDlg::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2012-03-23 15:27) - PLID 49154 - Update all our controls
		if (CWnd* pTop = GetTopLevelFrame()) {
			if (CNxOleControlContainer* pNxCtrlCont = polymorphic_downcast<CNxOleControlContainer*>(GetControlContainer())) {
				pNxCtrlCont->UpdateDialogControls(pTop, FALSE);
				return TRUE;
			}
		}

		ASSERT(FALSE);
	} NxCatchAllIgnore();

	return 0;
}

// (a.walling 2012-03-23 15:27) - PLID 49154 - Routes commands up to the top-level frame
BOOL CEMNMoreInfoDlg::RouteCommandToParent(UINT nID)
{
	if (CWnd* pTop = GetTopLevelFrame()) {
		pTop->PostMessage(WM_COMMAND, nID);
		return TRUE;
	}

	ASSERT(FALSE);
	return FALSE;
}

// (a.walling 2012-03-23 18:01) - PLID 50638 - Notes
void CEMNMoreInfoDlg::OnUpdateNotes(CCmdUI* pCmdUI)
{
	if (CEdit* pEdit = dynamic_cast<CEdit*>(pCmdUI->m_pOther)) {
		bool bReadOnly = !!(pEdit->GetStyle() & ES_READONLY);
		if (bReadOnly != !!m_bReadOnly) {
			pEdit->SetReadOnly(m_bReadOnly);
		}
	}
	pCmdUI->SetText(m_pEMN->GetNotes());
	pCmdUI->ContinueRouting();
}

// (a.walling 2012-03-23 18:01) - PLID 50638 - Description
void CEMNMoreInfoDlg::OnUpdateDescription(CCmdUI* pCmdUI)
{
	if (CEdit* pEdit = dynamic_cast<CEdit*>(pCmdUI->m_pOther)) {
		bool bReadOnly = !!(pEdit->GetStyle() & ES_READONLY);
		if (bReadOnly != !!m_bReadOnly) {
			pEdit->SetReadOnly(m_bReadOnly);
		}
	}
	pCmdUI->SetText(m_pEMN->GetDescription());
	pCmdUI->ContinueRouting();
}

// (a.walling 2012-07-06 17:37) - PLID 49154 - More info synchronization
void CEMNMoreInfoDlg::OnMoreInfoChanged()
{
	// first off do an idle update
	AfxGetApp()->OnIdle(-1);
	
	SetDate(m_pEMN->GetEMNDate());
	SetLocation(m_pEMN->GetLocationID());
	SetStatus(m_pEMN->GetStatus());

	{
		long nCurChartID = -1;
		
		if (IRowSettingsPtr pChartRow = m_pChartCombo->CurSel) {
			nCurChartID = VarLong(pChartRow->GetValue(chartID));
		}

		// (z.manning, 04/11/2007) - PLID 25569
		SetChart(m_pEMN->GetChart());

		if (nCurChartID != m_pEMN->GetChart()) {
			RefilterCategoryList();
		}
		
		SetCategory(m_pEMN->GetCategory());
	}

	// (j.jones 2007-08-24 09:22) - PLID 27054 - added visit type
	//TrySetVisitType(m_pEMN->GetVisitTypeID(), m_pEMN->GetVisitTypeName());

	//m.hancock - 3/14/2006 - 19579 - Patient demographics shouldn't change after the EMN is locked or finished.
	//The EMN handles data retrieval, so simply display what the EMN has loaded.
	// (a.walling 2007-08-06 13:20) - PLID 23714 - Fill patient demographics
	RefreshPatientDemographics();

	//TES 12/26/2006 - PLID 23400 - Load all the providers.
	std::vector<long> newProvs;
	{
		CArray<long,long> arProviderIDs;
		m_pEMN->GetProviders(arProviderIDs);
		newProvs.assign(arProviderIDs.GetData(), arProviderIDs.GetData() + arProviderIDs.GetSize());

		if (m_lastProviders != newProvs) {
			SetProviders(arProviderIDs);
		}
	}

	// (j.gruber 2007-01-08 09:48) - PLID 23399 - add Secondary Provider Information
	{
		CArray<long,long> arSecondaryProviderIDs;
		m_pEMN->GetSecondaryProviders(arSecondaryProviderIDs);		
		newProvs.assign(arSecondaryProviderIDs.GetData(), arSecondaryProviderIDs.GetData() + arSecondaryProviderIDs.GetSize());

		if (m_lastSecondaryProviders != newProvs) {
			SetSecondaryProviders(arSecondaryProviderIDs);
		}
	}

	// (d.lange 2011-03-23 10:28) - PLID 42136 - add Assistant/Technician
	{
		CArray<long,long> arTechnicianIDs;
		m_pEMN->GetTechnicians(arTechnicianIDs);		
		newProvs.assign(arTechnicianIDs.GetData(), arTechnicianIDs.GetData() + arTechnicianIDs.GetSize());
		
		if (m_lastTechnicians != newProvs) {
			SetTechnicians(arTechnicianIDs);
		}
	}
}

// (b.savon 2013-01-24 10:53) - PLID 54817 - Remove the buttons in the prescription section of more info in EMR and replace it with a queue button
void CEMNMoreInfoDlg::OnBnClickedBtnRxQueueMoreInfo()
{
	try{
		CEmrTreeWnd* pTree = GetEmrTreeWnd();
		
		if(!pTree) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}
			
		pTree->ShowEPrescribing();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-02-25 15:50) - PLID 60913 - UPDATE - Add two buttons in More Info topic for "Merge Summary of Care" and "Merge Clinical Summary"
void CEMNMoreInfoDlg::OnBnClickedMergeSummaryCareBtn()
{
	try{

		// (a.walling 2014-05-12 16:05) - PLID 62115 - Create a drop down in the EMN More Info for Summary of Care and Clinical Summary 
		// to allow for either a customized version.  The drop down should display "Customized"

		CNxMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_BYCOMMAND, ID_EMR_CARE_SUMMARY, "Generate");
		menu.AppendMenu(MF_BYCOMMAND, ID_EMR_CARE_SUMMARY_CUSTOMIZED, "Customized...");

		menu.SetDefaultItem(ID_EMR_CARE_SUMMARY, FALSE);

		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-02-25 15:50) - PLID 60913 - UPDATE - Add two buttons in More Info topic for "Merge Summary of Care" and "Merge Clinical Summary"
void CEMNMoreInfoDlg::OnBnClickedMergeClinicalSummaryBtn()
{
	try {

		// (a.walling 2014-05-12 16:05) - PLID 62115 - Create a drop down in the EMN More Info for Summary of Care and Clinical Summary 
		// to allow for either a customized version.  The drop down should display "Customized"

		CNxMenu menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_BYCOMMAND, ID_EMR_CLINICAL_SUMMARY, "Generate");
		menu.AppendMenu(MF_BYCOMMAND, ID_EMR_CLINICAL_SUMMARY_CUSTOMIZED, "Customized...");

		menu.SetDefaultItem(ID_EMR_CARE_SUMMARY, FALSE);

		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
	} NxCatchAll(__FUNCTION__);
}
