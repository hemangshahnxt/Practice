// LabEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "LabEntryDlg.h"
#include "PatientLabsDlg.h"
#include "EditLabResultsDlg.h"
#include "EditComboBox.h"
#include "NxStandard.h"
#include "GlobalUtils.h"
#include "InternationalUtils.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "AuditTrail.h"
#include "TaskEditDlg.h"
#include "LabFormNumberEditorDlg.h"
#include "MultiSelectDlg.h"
#include "PatientsRc.h"
#include "RenameFileDlg.h"
#include "TodoUtils.h"
#include "MsgBox.h"
#include "SignatureDlg.h"
#include "LabEditDiagnosisDlg.h"
#include "EditLabStatusDlg.h"
#include "DontShowDlg.h"
#include "LabRequisitionDlg.h"
#include "RegUtils.h"// (a.vengrofski 2010-06-10 11:08) - PLID <38544> - Needed to be able to SendLabToHL7
#include "HL7Utils.h"// (a.vengrofski 2010-06-10 11:08) - PLID <38544> - Needed to be able to SendLabToHL7
// (r.galicki 2008-10-17 09:17) - PLID 31552 - Needed for LabType enum (at least until a better location is available)
#include "LabsSetupDlg.h"
#include "LabBarcode.h"

#include "WellnessDataUtils.h"
// (b.savon 2012-02-28 17:07) - PLID 48443 - Create recall from labdlg
#include "CreatePatientRecall.h"
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include "DecisionRuleUtils.h"
#include "LabUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_LABDESC_REMOVE		40001
#define ID_CLINICALDIAG_REMOVE	40002

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CLabEntryDlg dialog

// (z.manning 2008-10-06 10:31) - PLID 21094 - Changed the constructor to take a CWnd
CLabEntryDlg::CLabEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabEntryDlg::IDD, pParent),
	m_dlgRequisitions(*(new CLabRequisitionsTabDlg(this))),
	m_dlgResults(*(new CLabResultsTabDlg(this)))
{
	// (r.galicki 2008-10-08 12:05) - PLID 31555 - Added PIC ID to labs
	//{{AFX_DATA_INIT(CLabEntryDlg)
		m_nPatientID = -1;
		m_nInitialLabID = -1;
		m_nGender = 0;
		m_nLabProcedureID = -1;
		m_nLabProcedureGroupID = -1; // (r.gonet 03/29/2012) - PLID 45856 - The lab procedure group of the lab's lab procedure. -1 is system default.
		m_varSourceActionID.vt = VT_NULL;
		m_varSourceDetailID.vt = VT_NULL;
		m_nPIC = -1;
		m_ltType = ltInvalid;
		m_bHideResults = FALSE;
		m_varSourceDataGroupID.vt = VT_NULL;
		m_varOrderSetID.vt = VT_NULL;
		m_nLastFormNumberCount = -1;
		m_bIsNewLab = FALSE;
		m_varSourceDetailImageStampID.vt = VT_NULL;
		m_bAddNewLabImmediatelyOnLoad = FALSE;
		m_bNextLabWillOpenOnClose = FALSE;
		m_varBirthDate = g_cvarNull;
		m_hwndNotify = NULL; // (c.haag 2010-07-15 17:23) - PLID 34338

		//TES 2/1/2010 - PLID 37143 - -2 = need to look up
		m_nDefaultRequestForm = -2;
		//TES 7/27/2012 - PLID 51849 - -2 = need to look up
		m_nDefaultResultForm = -2;
		// (b.spivey - January 20, 2014) - PLID 46370 - Added InitialResultID
		m_nInitialResultID = -1; 
	//}}AFX_DATA_INIT
}


CLabEntryDlg::~CLabEntryDlg()
{
	// (a.walling 2011-06-22 11:59) - PLID 44260 - Destruction of child windows should have happened way back in OnDestroy, not here.

	//TES 11/17/2009 - PLID 36190 - Destroy the requisition tab.
	//m_dlgRequisitions.DestroyWindow();
	delete (&m_dlgRequisitions);

	//TES 11/20/2009 - PLID 36191 - Also the results tab
	//m_dlgResults.DestroyWindow();
	delete (&m_dlgResults);
}

void CLabEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabEntryDlg)
	DDX_Control(pDX, IDC_CANCEL_LAB, m_cancelBtn);
	DDX_Control(pDX, IDC_LAB_SAVE_AND_RESUME_PREVIOUS, m_saveResumePrevBtn);
	DDX_Control(pDX, IDC_LAB_SAVE_AND_ADD_NEW, m_saveAddNewBtn);
	DDX_Control(pDX, IDC_LAB_PATIENT, m_nxeditLabPatient);
	DDX_Control(pDX, IDC_LAB_GENDER, m_nxeditLabGender);
	DDX_Control(pDX, IDC_LAB_AGE, m_nxeditLabAge);
	DDX_Control(pDX, IDC_PATIENT_LABEL, m_nxstaticPatientLabel);
	DDX_Control(pDX, IDC_GENDER_LABEL, m_nxstaticGenderLabel);
	DDX_Control(pDX, IDC_AGE_LABEL, m_nxstaticAgeLabel);
	DDX_Control(pDX, IDC_LAB_PRINT, m_btnPrint);
	DDX_Control(pDX, IDC_LAB_PREVIEW, m_btnPreview);
	DDX_Control(pDX, IDC_LABEL_CHECK, m_nxbtnLabel);
	DDX_Control(pDX, IDC_REQUEST_FORM_CHECK, m_nxbtnRequestForm);
	DDX_Control(pDX, IDC_RESULT_FORM_CHECK, m_nxbtnResultsForm);
	DDX_Control(pDX, IDC_EDIT_FORM_NUMBER_FORMAT, m_EditFormNumberFormatBtn);
	DDX_Control(pDX, IDC_FORM_NUMBER, m_nxeditFormNumber);
	DDX_Control(pDX, IDC_LAB_INPUT_DATE, m_nxeditInputDate);
	DDX_Control(pDX, IDC_SEND_HL7, m_nxbtnSendHL7);
	DDX_Control(pDX, IDC_LAB_ENTRY_CREATE_TODO, m_btnCreateTodo);
	DDX_Control(pDX, IDC_NXBTN_CREATE_RECALL, m_btnCreateRecall);
	DDX_Control(pDX, IDC_EDIT_LABEL_PRINTER, m_btnEditLabelPrinterBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLabEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLabEntryDlg)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_LAB_SAVE_AND_ADD_NEW, OnSaveAndAddNew)
	ON_BN_CLICKED(IDC_LAB_SAVE_AND_RESUME_PREVIOUS, OnSaveAndResumePrevious)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_LAB_PRINT, OnPrint)
	ON_BN_CLICKED(IDC_LAB_PREVIEW, OnPreview)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EDIT_FORM_NUMBER_FORMAT, &CLabEntryDlg::OnEditFormNumberFormat)
	ON_EN_KILLFOCUS(IDC_FORM_NUMBER, &CLabEntryDlg::OnKillfocusFormNumber)
	ON_BN_CLICKED(IDC_CANCEL_LAB, &CLabEntryDlg::OnBnClickedCancelLab)
	ON_BN_CLICKED(IDC_REQUEST_FORM_CHECK, &CLabEntryDlg::OnRequestFormCheck)
	ON_BN_CLICKED(IDC_SEND_HL7, &CLabEntryDlg::OnSendHL7Check)// (a.vengrofski 2010-05-12 08:42) - PLID <38547>
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LAB_ENTRY_CREATE_TODO, &CLabEntryDlg::OnBnClickedLabEntryCreateTodo)
	ON_BN_CLICKED(IDC_NXBTN_CREATE_RECALL, &CLabEntryDlg::OnBnClickedNxbtnCreateRecall)
	ON_BN_CLICKED(IDC_RESULT_FORM_CHECK, &CLabEntryDlg::OnResultFormCheck)
	ON_BN_CLICKED(IDC_EDIT_LABEL_PRINTER, &CLabEntryDlg::OnBnClickedEditLabelPrintSettings)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabEntryDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
// (a.vengrofski 2010-05-12 08:43) - PLID <38547>
BEGIN_EVENTSINK_MAP(CLabEntryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLabEntryDlg)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CLabEntryDlg, IDC_LAB_ENTRY_TABS, 1, CLabEntryDlg::OnSelectTabLabEntryTabs, VTS_I2 VTS_I2)
	ON_EVENT(CLabEntryDlg, IDC_HL7_LIST, 16 /* SelChosen */, OnSelChosenLabLinkList, VTS_DISPATCH)// (a.vengrofski 2010-05-12 08:43) - PLID <38547>
END_EVENTSINK_MAP()

enum RequestFormsListColumns {
	rflcID = 0,
	rflcNumber = 1,
	rflcName = 2,
	rflcReportName = 3,
	rflcGenerateBarcode = 4,
};

BOOL CLabEntryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {

		// (c.haag 2008-04-25 13:57) - PLID 29790 - NxIconify buttons
		m_saveResumePrevBtn.AutoSet(NXB_OK);
		m_saveAddNewBtn.AutoSet(NXB_NEW);
		m_cancelBtn.AutoSet(NXB_CANCEL);
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPreview.AutoSet(NXB_PRINT_PREV);
		m_btnCreateTodo.AutoSet(NXB_NEW); // (z.manning 2010-05-13 09:19) - PLID 37405
		m_btnCreateRecall.AutoSet(NXB_RECALL); // (b.savon 2012-02-28 12:28) - PLID 48443
		m_btnEditLabelPrinterBtn.AutoSet(NXB_MODIFY);
		
		// (j.armen 2012-03-28 10:59) - PLID 48480 - Hide the button if we don't have recalls
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
		{
			m_btnCreateRecall.AutoSet(NXB_RECALL); // (b.savon 2012-02-28 12:28) - PLID 48443
			m_btnCreateRecall.EnableWindow(TRUE);
			m_btnCreateRecall.ShowWindow(SW_SHOWNA);

			//(a.wilson 2012-3-23) PLID 48472 - check to ensure they have permission to access recall system.
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

		// Disable Send To HL7 button if they're not licensed for HL7
		if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
		{
			m_nxbtnSendHL7.EnableWindow(TRUE);
		}
		else 
		{
			m_nxbtnSendHL7.EnableWindow(FALSE);
		}

		//TES 12/3/2009 - PLID 36193
		// (b.eyers 2015-08-06) - PLID 43031 - updated field to 200
		m_nxeditFormNumber.SetLimitText(200);

		//(a.wilson 2012-3-23) PLID 48472 - check to ensure they have permission to access recall system.
		if ((GetCurrentUserPermissions(bioRecallSystem) & (sptCreate | sptCreateWithPass))) {
			GetDlgItem(IDC_NXBTN_CREATE_RECALL)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_NXBTN_CREATE_RECALL)->EnableWindow(FALSE);
		}

		// (z.manning 2009-05-29 14:02) - PLID 29911 - Added bulk caching for LabProblemsRequired
		// preference and included some other existing ones.
		//TES 11/20/2009 - PLID 36382 - Changed the name from "EMNMoreInfoDlg" to "LabEntryDlg", so that these preferences will actually
		// get cached.
		g_propManager.CachePropertiesInBulk("LabEntryDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'LabProblemsRequired' \r\n"
			"	, 'LabChoosePrimary' \r\n"
			"	, 'LabSpawnToDo' \r\n"
			// (z.manning 2010-04-29 15:27) - PLID 38420 - Properties to remember window size/position
			"	, 'LabEntryDlgMaximied' \r\n"
			"	, 'LabEntryDlgWidth' \r\n"
			"	, 'LabEntryDlgHeight' \r\n"
			"	, 'LabEntryDlgTop' \r\n"
			"	, 'LabEntryDlgLeft' \r\n"
			"	, 'LabsRetainInitialDiagnosis' \r\n" // (z.manning 2010-05-05 09:29) - PLID 37190
			"   , 'HL7Import_FailedACKCheckDelaySeconds' \r\n" // (c.haag 2010-08-11 12:10) - PLID 39799
			"	, 'LabsRetainComments' \r\n" //TES 9/29/2010 - PLID 40644
			"	, 'LabsRetainProviders' \r\n" //TES 9/29/2010 - PLID 40644
			"	, 'Labs_UseCustomToDoText' \r\n" // (r.gonet 06/22/2011) - PLID 37414 - Will be used in a library call to TodoCreateForLab, so we'll cache it here
			"	, 'Labs_RequisitionsTabWhenNoResults' \r\n" //TES 7/8/2011 - PLID 43793
			"	, 'DefaultLabProvider' \r\n"				// (d.thompson 2012-10-29) - PLID 42843
			"	, 'DefaultLabProvider_ApptMaxDays' \r\n"	// (r.gonet 07/22/2013) - PLID 45187
			"	, 'Lab_DefaultTodoPriority' \r\n" //TES 8/6/2013 - PLID 51147
			"	, 'ShowPatientEducationLinks' \r\n" // (r.gonet 2014-01-27 15:29) - PLID 59339
			"	, 'ShowPatientEducationButtons' \r\n" // (r.gonet 2014-01-27 15:29) - PLID 59339
			"	, 'DuplicateSpawnedEmrProblemBehavior' \r\n" // (z.manning 2016-04-08) - NX-100090
			")"
			, _Q(GetCurrentUserName()));
		// (z.manning 2010-02-01 15:00) - PLID 34808 - I noticed that LabFormNumberFormat had been getting cached as
		// a number, but it's a text property.
		// (r.gonet 03/29/2012) - PLID 45856 - Removed LabFormNumberFormat		
		g_propManager.CachePropertiesInBulk("LabEntryDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'Labs_CustomToDoText' \r\n" // (r.gonet 06/22/2011) - PLID 37414 - Will be used in a library call to TodoCreateForLab, so we'll cache it here
			")"
			, _Q(GetCurrentUserName()));

		m_color = 0x00FFB9A8;
		//m_color = 0x00FFCC99; //Old blue patient's dialog color
		m_brush.CreateSolidBrush(PaletteColor(m_color));

		//TES 11/20/2009 - PLID 36190 - Initialize our tabs
		m_tab = GetDlgItemUnknown(IDC_LAB_ENTRY_TABS);
		if (m_tab == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDCANCEL);
			return 0;
		}
		else {
			m_tab->PutTabWidth(4);
			if(m_bHideResults) {
				//TES 11/20/2009 - PLID 36190 - Don't make a Results tab if we've been asked to hide results.
				m_tab->PutSize(1);
				//TES 11/25/2009 - PLID 36193 - This tab is now plural.
				m_tab->PutLabel(0, "Requisitions");
			}
			else {
				m_tab->PutSize(2);
				//TES 11/25/2009 - PLID 36193 - This tab is now plural
				m_tab->PutLabel(0, "Requisitions");
				m_tab->PutLabel(1, "Results");
			}
		}

		m_bDataChanged = false;
		m_bOpenedReport = false;

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		// (a.vengrofski 2010-05-11 15:02) - PLID <38547> - Fill the Lab Link List
		m_pLabLinkList = BindNxDataList2Ctrl(IDC_HL7_LIST,true);
		pRow = m_pLabLinkList->GetNewRow();
		pRow->PutValue(lllID, (long) -1);
		pRow->PutValue(lllName, _variant_t("{No Link}"));
		
		m_pLabLinkList->AddRowAtEnd(pRow, NULL);

		//TES 1/29/2010 - PLID 34439 - Set up our list of available Request Forms.
		//TES 2/1/2010 - PLID 37143 - Call our function to get the default.
		long nDefaultReportNum = GetDefaultRequestForm();
		m_pRequestFormsList = BindNxDataList2Ctrl(IDC_REQUEST_FORM_LIST,false);


		pRow = m_pRequestFormsList->GetNewRow();

		//TES 1/29/2010 - PLID 34439 - Add the system default report.
		pRow->PutValue(rflcID, (long)-1);
		pRow->PutValue(rflcNumber, (long)-1);
		pRow->PutValue(rflcName, _variant_t("Lab Request Form"));
		pRow->PutValue(rflcReportName, _variant_t(""));
		// (r.gonet 10/11/2011) - PLID 46437 - Standard report doesn't use barcodes.
		pRow->PutValue(rflcGenerateBarcode, g_cvarFalse);
		m_pRequestFormsList->AddRowAtEnd(pRow, NULL);
		//TES 1/29/2010 - PLID 34439 - Is the system default the actual default?
		if(nDefaultReportNum == -1) {
			pRow->PutForeColor(RGB(255,0,0));
			m_pRequestFormsList->CurSel = pRow;
		}

		//TES 1/29/2010 - PLID 34439 - Now add any custom reports.
		_RecordsetPtr rsReports = CreateRecordset("SELECT ID, Number, Title, Filename, CONVERT(BIT, GenerateBarcode) AS GenerateBarcode FROM CustomReportsT WHERE ID = 658 ");
		while (!rsReports->eof) {

			long nRepID = AdoFldLong(rsReports, "ID");
			long nRepNumber = AdoFldLong(rsReports, "Number");

			pRow = m_pRequestFormsList->GetNewRow();
			pRow->PutValue(rflcID, nRepID );
			pRow->PutValue(rflcNumber, nRepNumber);
			pRow->PutValue(rflcName, _variant_t(AdoFldString(rsReports,"Title", "")));
			pRow->PutValue(rflcReportName, _variant_t(AdoFldString(rsReports,"FileName", "")));
			// (r.gonet 10/11/2011) - PLID 46437 - Add whether to generate the barcode, since that takes a while.
			pRow->PutValue(rflcGenerateBarcode, _variant_t(AdoFldBool(rsReports,"GenerateBarcode", FALSE) ? VARIANT_TRUE : VARIANT_FALSE, VT_BOOL));

			m_pRequestFormsList->AddRowAtEnd(pRow, NULL);

			//TES 1/29/2010 - PLID 34439 - Is this the default report?
			if(nRepNumber == nDefaultReportNum) {
				pRow->PutForeColor(RGB(255,0,0));
				m_pRequestFormsList->CurSel = pRow;
			}

			rsReports->MoveNext();
		}
		rsReports->Close();

		//TES 7/27/2012 - PLID 51071 - Set up our list of available Request Forms.
		//TES 7/27/2012 - PLID 51849 - Call our function to get the default.
		nDefaultReportNum = GetDefaultResultForm();
		m_pResultFormsList = BindNxDataList2Ctrl(IDC_RESULT_FORM_LIST,false);


		pRow = m_pResultFormsList->GetNewRow();

		//TES 7/27/2012 - PLID 51071 - Add the system default report.
		pRow->PutValue(rflcID, (long)-1);
		pRow->PutValue(rflcNumber, (long)-1);
		pRow->PutValue(rflcName, _variant_t("Lab Results Form"));
		pRow->PutValue(rflcReportName, _variant_t(""));
		m_pResultFormsList->AddRowAtEnd(pRow, NULL);
		//TES 7/27/2012 - PLID 51071 - Is the system default the actual default?
		if(nDefaultReportNum == -1) {
			pRow->PutForeColor(RGB(255,0,0));
			m_pResultFormsList->CurSel = pRow;
		}

		//TES 7/27/2012 - PLID 51071 - Now add any custom reports.
		rsReports = CreateRecordset("SELECT ID, Number, Title, Filename FROM CustomReportsT WHERE ID = 567 ");
		while (!rsReports->eof) {

			long nRepID = AdoFldLong(rsReports, "ID");
			long nRepNumber = AdoFldLong(rsReports, "Number");

			pRow = m_pResultFormsList->GetNewRow();
			pRow->PutValue(rflcID, nRepID );
			pRow->PutValue(rflcNumber, nRepNumber);
			pRow->PutValue(rflcName, _variant_t(AdoFldString(rsReports,"Title", "")));
			pRow->PutValue(rflcReportName, _variant_t(AdoFldString(rsReports,"FileName", "")));
			
			m_pResultFormsList->AddRowAtEnd(pRow, NULL);

			//TES 7/27/2012 - PLID 51071 - Is this the default report?
			if(nRepNumber == nDefaultReportNum) {
				pRow->PutForeColor(RGB(255,0,0));
				m_pResultFormsList->CurSel = pRow;
			}

			rsReports->MoveNext();
		}
		rsReports->Close();

		//TES 11/17/2009 - PLID 36190 - Create the Requisition tab
		//TES 11/25/2009 - PLID 36193 - Changed to the new dialog, which holds multiple requisitions.
		m_dlgRequisitions.Create(IDD_LAB_REQUISITIONS_TAB_DLG, this);		
		CRect rcSubDlg;
		GetDlgItem(IDC_TABS_PLACEHOLDER)->GetWindowRect(&rcSubDlg);
		ScreenToClient(&rcSubDlg);
		m_dlgRequisitions.MoveWindow(&rcSubDlg);
		m_dlgRequisitions.ShowWindow(SW_SHOW);

		//TES 11/20/2009 - PLID 36191 - Also the results tab
		m_dlgResults.Create(IDD_LAB_RESULTS_TAB_DLG, this);		
		m_dlgResults.MoveWindow(&rcSubDlg);

		SetDlgItemCheck(IDC_HL7_LIST,FALSE);// (a.vengrofski 2010-05-12 15:02) - PLID <38547>
		m_nxbtnSendHL7.SetCheck(FALSE);// (a.vengrofski 2010-05-12 15:02) - PLID <38547>
		GetDlgItem(IDC_HL7_LIST)->EnableWindow(FALSE);// (a.vengrofski 2010-05-12 15:02) - PLID <38547>
		
		if(m_nInitialLabID > 0) {
			//TES 11/25/2009 - PLID 36193 - Remember that we are not new
			m_bIsNewLab = FALSE;
			LoadExisting();
			// (z.manning 2008-11-04 09:07) - PLID 31904 - For existing labs, check to print the results
			// report by default.
			m_nxbtnResultsForm.SetCheck(BST_CHECKED);
			//TES 7/27/2012 - PLID 51071 - Enable the Results Form dropdown
			OnResultFormCheck();
			//TES 2/5/2010 - PLID 34439 - Disable the dropdown, since Request Forms isn't checked.
			OnRequestFormCheck();

			//TES 7/8/2011 - PLID 43793 - If we don't have any results, check our preference to default to the requisition tab.
			bool bForceRequisitionsTab = false;
			CArray<IRowSettingsPtr,IRowSettingsPtr> arResults;
			//TES 11/6/2012 - PLID 53591 - Tell it to check all specimens (this is a change in behavior from before, but I believe it
			// to be correct).
			m_dlgResults.GetResults(m_nInitialLabID, arResults, CLabResultsTabDlg::groAllSpecimens);
			if(!arResults.GetSize()) {
				if(GetRemotePropertyInt("Labs_RequisitionsTabWhenNoResults", FALSE, 0, GetCurrentUserName())) {
					bForceRequisitionsTab = true;
				}
			}

			//TES 11/20/2009 - PLID 36190 - Select the Results tab (if we have one).
			if(m_bHideResults) {
				m_tab->CurSel = 0;
			}
			else {
				if(bForceRequisitionsTab) {
					//TES 7/8/2011 - PLID 43793 - Default to the Requisitions tab, as well as defaulting the report checkboxes.
					m_tab->CurSel = 0;
					m_nxbtnResultsForm.SetCheck(BST_UNCHECKED);
					_RecordsetPtr prsDefaults = CreateParamRecordset("SELECT SendSpecimenLabel, SendRequestForm FROM LabProceduresT WHERE ID = {INT};", m_nLabProcedureID);
					if(!prsDefaults->eof) {
						m_nxbtnLabel.SetCheck(AdoFldBool(prsDefaults, "SendSpecimenLabel"));
						CheckDlgButton(IDC_REQUEST_FORM_CHECK, AdoFldBool(prsDefaults, "SendRequestForm"));
					}
					else {
						//Just in case *something* happens and we lose the lab procedure, let's default back to 'on', which was the default
						//	behavior before PLID 36791 came along.
						m_nxbtnLabel.SetCheck(BST_CHECKED);
						m_nxbtnRequestForm.SetCheck(BST_CHECKED);
					}
					OnRequestFormCheck();
					//TES 7/27/2012 - PLID 51071 - Enable the Results Form dropdown
					OnResultFormCheck();
				}
				else {
					m_tab->CurSel = 1;
				}
			}
		}
		else {
			//TES 11/25/2009 - PLID 36193 - Remember that we are new
			m_bIsNewLab = TRUE;
			LoadNew();
			// (z.manning 2008-11-04 09:08) - PLID 31904 - For new labs, check to print label and request
			// form by default.
			// (d.thompson 2010-06-14) - PLID 36791 - These are no longer always checked, but are loaded per-procedure type.
			_RecordsetPtr prsDefaults = CreateParamRecordset("SELECT SendSpecimenLabel, SendRequestForm FROM LabProceduresT WHERE ID = {INT};", m_nLabProcedureID);
			if(!prsDefaults->eof) {
				m_nxbtnLabel.SetCheck(AdoFldBool(prsDefaults, "SendSpecimenLabel"));
				CheckDlgButton(IDC_REQUEST_FORM_CHECK, AdoFldBool(prsDefaults, "SendRequestForm"));
			}
			else {
				//Just in case *something* happens and we lose the lab procedure, let's default back to 'on', which was the default
				//	behavior before PLID 36791 came along.
				m_nxbtnLabel.SetCheck(BST_CHECKED);
				m_nxbtnRequestForm.SetCheck(BST_CHECKED);
			}
			//TES 2/5/2010 - PLID 34439 - Enable the dropdown.
			// (d.thompson 2010-06-14) - PLID 36791 - Just call the function now, 'send form' may or may not be checked
			//GetDlgItem(IDC_REQUEST_FORM_LIST)->EnableWindow(TRUE);
			OnRequestFormCheck();
			//TES 7/27/2012 - PLID 51071 - Enable the Results Form dropdown
			OnResultFormCheck();

			//TES 11/20/2009 - PLID 36190 - Select the Requisition tab
			m_tab->CurSel = 0;
		}
		//TES 11/20/2009 - PLID 36190 - Show the appropriate subdialog
		ReflectCurrentTab();

		//TES 11/17/2009 - PLID 36190 - Tell our requisition tab to do whatever it needs to do after data is loaded.
		m_dlgRequisitions.PostLoad();

		

		// (z.manning 2010-03-17 10:21) - PLID 37439 - Do we want to immediately add a new specimen?
		if(m_bAddNewLabImmediatelyOnLoad) {
			AddNew();
			m_bAddNewLabImmediatelyOnLoad = FALSE;
		}

		SetDlgItemCheck(IDC_HL7_LIST,FALSE);// (a.vengrofski 2010-05-12 15:02) - PLID <38547>

		// (z.manning 2010-04-29 14:38) - PLID 38420 - We now remember the position of this dialog
		if(GetRemotePropertyInt("LabEntryDlgMaximied", 0, 0, GetCurrentUserName()) == 1) {
			// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
			SetWindowPos(NULL, 0, 0, 994, 738, 0);
			ShowWindow(SW_MAXIMIZE);
		}
		else {
			CRect rcLabEntry, rcRemembered;
			GetWindowRect(rcLabEntry);
			int nWidth = GetRemotePropertyInt("LabEntryDlgWidth", rcLabEntry.Width(), 0, GetCurrentUserName());
			int nHeight = GetRemotePropertyInt("LabEntryDlgHeight", rcLabEntry.Height(), 0, GetCurrentUserName());
			int nTop = GetRemotePropertyInt("LabEntryDlgTop", rcLabEntry.top, 0, GetCurrentUserName());
			int nLeft = GetRemotePropertyInt("LabEntryDlgLeft", rcLabEntry.left, 0, GetCurrentUserName());
			if(nWidth > 50 && nHeight > 50) {
				rcRemembered.left = nLeft;
				rcRemembered.top = nTop;
				rcRemembered.right = rcRemembered.left + nWidth;
				rcRemembered.bottom = rcRemembered.top + nHeight;

				CRect rcDesktopRect;
				GetDesktopWindow()->GetWindowRect(rcDesktopRect);
				rcDesktopRect.DeflateRect(0, 0, 30, 50); // deflate the rect's bottom right corner to ensure the top left
					// corner of the new window rect is visible and able to move

				// either the top left or top right corner should be in our desktop rect
				CPoint ptTopLeft = rcRemembered.TopLeft();
				CPoint ptTopRight = ptTopLeft;
				ptTopRight.x += rcRemembered.Width();

				if (!(rcDesktopRect.PtInRect(ptTopLeft) || rcDesktopRect.PtInRect(ptTopRight))) {
					rcRemembered.CopyRect(rcLabEntry); // get the initial rect
					// now set the width and height
					rcRemembered.right = rcRemembered.left + nWidth;
					rcRemembered.bottom = rcRemembered.top + nHeight;
				}
				MoveWindow(rcRemembered);
			}
		}

		//TES 11/20/2009 - PLID 36191 - Also the results tab
		// (j.gruber 2010-11-30 12:35) - PLID 41606 - moved to be after the resize
		m_dlgResults.PostLoad();

		//TES 8/6/2013 - PLID 51147 - Initialize the priority to the default, and note that we haven't added any result flags to a todo yet.
		m_TodoPriority = (TodoPriority)GetRemotePropertyInt("Lab_DefaultTodoPriority", 1, 0, "<None>");
		m_bTodoHasFlag = false;

		return TRUE;
	}NxCatchAll("Error in CLabEntryDElg::OnInitDialog");
	return FALSE;

}

HBRUSH CLabEntryDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_PATIENT_LABEL:
		case IDC_AGE_LABEL:
		case IDC_GENDER_LABEL:
		case IDC_PROVIDER_LABEL:
		case IDC_LOCATION_LABEL:
		case IDC_LEFT_SIDE:
		case IDC_RIGHT_SIDE:
		case IDC_STATIC:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(m_color));
			return m_brush;
	}
	
	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (d.thompson 2010-01-29) - PLID 34335 - This button is specifically pressing the Cancel button.
void CLabEntryDlg::OnBnClickedCancelLab()
{
	try {
		//(e.lally 2011-09-20) PLID 45527 - Added separate code block so that all local variables go out of scope (and especially so the button disabler calls its destructor)
		//	prior to the CodeCleanup being called.
		{
			//	(b.spivey, July 17, 2013) - PLID 45194 
			if (NXTWAINlib::IsAcquiring())
			{
				MsgBox("Please wait for the current image acquisition to complete before closing this dialog.");
				return;
			}

			CLabEntryButtonDisabler btndisabler(this); // (z.manning 2010-11-16 15:46) - PLID 41499

			// (z.manning 2010-05-13 14:40) - PLID 37405 - If we have any pending to-dos then prompt first and
			// if they choose to continue, delete them.
			if(m_dlgRequisitions.GetTotalPendingTodoCount() > 0) {
				int nResult = MessageBox("You have unsaved to-do(s) for this lab.\r\n\r\nAre you sure you want to cancel?", "Labs", MB_YESNO|MB_ICONWARNING);
				if(nResult != IDYES) {
					return;
				}

				m_dlgRequisitions.DeleteAllPendingTodos();
			}

			// (z.manning, 07/28/2006) - PLID 21576 - If we generated a form number, we need to reset the counter
			// to what it was before we ever opened this dialog.  Note: we don't just decrement it here in case
			// other users also created new labs which may have incremented the counter by more than 1.
			if(m_nLastFormNumberCount > 0) {
				// (r.gonet 03/29/2012) - PLID 45856 - Added lab procedure group id
				SetLabFormNumberCounterIfLess(m_nLastFormNumberCount, m_nLabProcedureGroupID);
			}

			CDialog::OnCancel();
		}
		CloseCleanup(IDCANCEL); // (c.haag 2010-07-15 17:23) - PLID 34338

		//(e.lally 2011-09-20) PLID 45527 - WARNING: if m_hwndNotify is not null, the current instance 
		//	of this dialog may bedestroyed by this point. Any attempts to use anything (like member variables)
		//	associated with this instance of the dlg will crash and burn if that is the case.

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-01-29) - PLID 34335 - This is hitting escape or the X at the top right of the window.
void CLabEntryDlg::OnCancel() 
{
	try {

		// (b.spivey, July 17, 2013) - PLID 45194
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before closing this dialog.");
			return;
		}

		// (d.thompson 2010-01-29) - PLID 34335 - Warn before cancelling
		int nResult = AfxMessageBox("Do you want to save your changes to this Lab?\r\n\r\n"
			"'Yes' will save any changes and close the Lab.\r\n"
			"'No' will discard any changes and close the Lab.\r\n"
			"'Cancel' will cancel this action and leave the Lab open.", MB_YESNOCANCEL);
		if(nResult == IDYES) {
			//They want to save, not cancel. We'll use save & resume because they clicked the X, so they're
			//	clearly done with the lab itself.
			OnSaveAndResumePrevious();
		}
		else if(nResult == IDNO) {
			//They want to actually cancel, so do so.
			OnBnClickedCancelLab();
		}
		else {
			//Cancel the act of cancelling.  Do nothing.
		}
	} NxCatchAll(__FUNCTION__);
}

void CLabEntryDlg::SetPatientID(long nPatientID)
{
	m_nPatientID = nPatientID;
	//TES 11/17/2009 - PLID 36190 - Also set it on the Requisition tab
	m_dlgRequisitions.SetPatientID(nPatientID);
	//TES 11/19/2009 - PLID 36191 - And the Results tab
	m_dlgResults.SetPatientID(nPatientID);
}

void CLabEntryDlg::SetLabProcedureID(long nLabProcedureID)
{
	m_nLabProcedureID = nLabProcedureID;
	//TES 11/17/2009 - PLID 36190 - Also set it on the Requisition tab
	m_dlgRequisitions.SetLabProcedureID(nLabProcedureID);
	//TES 11/19/2009 - PLID 36191 - And the results tab
	m_dlgResults.SetLabProcedureID(nLabProcedureID);
	
	// (z.manning 2013-10-30 16:38) - PLID 59240 - This used to be in a separate function but I moved it here to
	// ensure it gets set when setting the lab procedure ID or else we may generate the wrong form number.
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(),
		"SELECT LabProcedureGroupID \r\n"
		"FROM LabProceduresT \r\n"
		"WHERE ID = {INT}; \r\n"
		, nLabProcedureID);
	if(!prs->eof) {
		m_nLabProcedureGroupID = VarLong(prs->Fields->Item["LabProcedureGroupID"]->Value, -1);
	}
	else {
		// we've already set this to be the default group in the constructor
	}
}

// (r.galicki 2008-10-20 11:54) - PLID 31552 - set lab type member
void CLabEntryDlg::SetLabProcedureType(LabType ltType) 
{
	m_ltType = ltType;
	//TES 11/17/2009 - PLID 36190 - Also set it on the Requisition tab
	m_dlgRequisitions.SetLabProcedureType(ltType);
	//TES 11/19/2009 - PLID 36191 - And the results tab
	m_dlgResults.SetLabProcedureType(ltType);
}

//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
void CLabEntryDlg::SetInitialAnatomicLocation(long nAnatomicLocationID, long nAnatomicQualifierID, AnatomySide asSide)
{
	m_dlgRequisitions.SetInitialAnatomicLocation(nAnatomicLocationID, nAnatomicQualifierID, asSide);
}

void CLabEntryDlg::LoadNew()
{
	try{

		m_varBirthDate = g_cvarNull;

		// (z.manning 2008-11-24 16:04) - PLID 31990 - Parameterized
		//TES 11/17/2009 - PLID 36190 - Removed any fields that will be loaded by the Requisition tab
		// (b.spivey, August 29, 2013) - PLID 58015 - This is in a seperate place for some reason... have to handle it here too.
		// (d.singleton 2013-10-25 14:36) - PLID 59195 - need patient id in the patient name field of the lab entry dialog
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"PersonT.Last AS PatientLast, PersonT.First AS PatientFirst, PersonT.Middle AS PatientMiddle, "
			"CASE WHEN (PrivCell = 1) THEN 'Private' ELSE PersonT.CellPhone END AS CellPhone, "
			"CASE WHEN (PrivHome = 1) THEN 'Private' ELSE PersonT.HomePhone END AS HomePhone, "
			"PersonT.BirthDate, PersonT.Gender, dbo.AsDateNoTime(GetDate()) AS CurDate, PatientsT.UserDefinedID AS UserDefinedID  "
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE PersonT.ID = {INT} "
			, m_nPatientID);
		//TES 11/25/2009 - PLID 36193 - The form number is now on the LabEntry dialog, not the Lab Requisition dialog.
		//Form number
		CString strFormNumber = GetNewFormNumber();
		
		if(!rs->eof){
			((CNxEdit*)GetDlgItem(IDC_FORM_NUMBER))->SetReadOnly(FALSE);
			SetDlgItemText (IDC_FORM_NUMBER, strFormNumber);

			SetCurrentDate(rs->Fields->Item["CurDate"]->Value);
			
			//Patient Name
			//(e.lally 2007-08-08) PLID 26978 - Labs store the patient name in the record, set our member variables
			// (d.singleton 2013-10-25 14:41) - PLID 59195 - need patient id in the patient name field of the lab entry dialog
			m_strPatientFirst = AdoFldString(rs, "PatientFirst");
			m_strPatientMiddle = AdoFldString(rs, "PatientMiddle");
			m_strPatientLast = AdoFldString(rs, "PatientLast");
			m_strUserDefinedID = AsString(AdoFldLong(rs, "UserDefinedID"));
			CString strCellPhone = AdoFldString(rs, "CellPhone", "");
			CString strHomePhone = AdoFldString(rs, "HomePhone", ""); 
			CString strPatientName;
			strPatientName.Format("%s, %s %s (%s)", m_strPatientLast, m_strPatientFirst, m_strPatientMiddle, m_strUserDefinedID);
			strPatientName.TrimRight();
			SetDlgItemText (IDC_LAB_PATIENT, strPatientName);
			// (b.spivey, August 23, 2013) - PLID 58015 - Put the cellphone and homephone in the title bar. 
			CString strWindowText = "";
			strWindowText += (strPatientName) + (strHomePhone.IsEmpty() ? "" : " - Home Phone: " + strHomePhone) +
				(strCellPhone.IsEmpty() ? "" : " - Mobile Phone: " + strCellPhone) + " - Lab Entry" ; 
			// (c.haag 2010-07-20 15:14) - PLID 34338 - Make it easier for users to see which patient a minimized lab is for
			// (though in some Windows themes it won't help much)
			SetWindowText(strWindowText);
			CString strAge="";

			

			//Gender
			if(rs->Fields->Item["Gender"]->Value.vt !=VT_NULL){
				m_nGender = AdoFldByte(rs, "Gender");
				if(m_nGender == 1)
					SetDlgItemText (IDC_LAB_GENDER, "Male");
				else if(m_nGender == 2)
					SetDlgItemText (IDC_LAB_GENDER, "Female");
			}
			
			//Age
			// (j.jones 2010-05-06 10:00) - PLID 38524 - do not display the age yet,
			// its is dependent on the biopsy date, which we haven't loaded yet
			m_varBirthDate = rs->Fields->Item["BirthDate"]->Value;
		}

		//TES 11/17/2009 - PLID 36190 - Now tell our requisition tab to load
		//TES 11/25/2009 - PLID 36193 - It needs to know whether we were launched from an EMN
		m_dlgRequisitions.LoadNew();
		//TES 11/19/2009 - PLID 36191 - And the Results tab.
		m_dlgResults.LoadNew();

		// (j.jones 2010-05-06 09:22) - PLID 38520 - supported input date
		// (c.haag 2010-09-13 11:19) - PLID 38989 - Show the input time as well
		SetDlgItemText(IDC_LAB_INPUT_DATE, FormatDateTimeForInterface(m_dlgRequisitions.GetFirstInputDate(), NULL, dtoDateTime));

		// (j.jones 2010-05-06 10:01) - PLID 38524 - moved age calculation into its own function
		CalculateAndDisplayPatientAge();

	}NxCatchAll("Error in CLabEntryDlg::LoadNew");
}

void CLabEntryDlg::LoadExisting()
{
	try{

		m_varBirthDate = g_cvarNull;

		// (j.jones 2007-07-19 15:50) - PLID 26751 - added DiagnosisDesc and ClinicalDiagnosisDesc
		//(e.lally 2007-08-08) PLID 26978 - Labs store the patient name in the record and must pull from those fields
		// (j.gruber 2008-09-18 15:40) - PLID 31332 - take out result fields
		// (z.manning 2008-10-16 10:32) - PLID 21082 - Added signature fields
		// (z.manning 2008-10-24 10:22) - PLID 31807 - Added to be ordered
		// (r.galicki 2008-10-28 15:01) - PLID 31552 - Added lab procedure type
		// (z.manning 2008-10-30 12:47) - PLID 31613 - Load the lab procedure ID
		// (z.manning 2008-11-24 13:47) - PLID 31990 - Added melanoma history and previous biopsy
		// (c.haag 2009-05-11 14:09) - PLID 28515 - Added instructions
		//TES 5/14/2009 - PLID 33792 - Added InitialDiagnosis
		//TES 6/23/2009 - PLID 34698 - Fixed bad join that caused the Input user to display as the Completed user.
		//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
		//TES 11/17/2009 - PLID 36190 - Removed any fields that are now loaded by the Requisition tab
		// (b.spivey, August 28, 2013) - PLID 58015 - consideration for private phone numbers. 
		// (d.singleton 2013-10-25 14:41) - PLID 59195 - need patient id in the patient name field of the lab entry dialog
		_RecordsetPtr rs = CreateParamRecordset("SELECT LabsT.PatientID, "
			"LabsT.PatientLast, LabsT.PatientFirst, LabsT.PatientMiddle, "
			"PersonT.BirthDate, PersonT.Gender, "
			"CASE WHEN (PrivCell = 1) THEN 'Private' ELSE PersonT.CellPhone END AS CellPhone, "
			"CASE WHEN (PrivHome = 1) THEN 'Private' ELSE PersonT.HomePhone END AS HomePhone, "
			"LabsT.Type, LabsT.FormNumberTextID, "
			"dbo.AsDateNoTime(GetDate()) AS CurDate, "
			"LabsT.LabProcedureID, PatientsT.UserDefinedID AS UserDefinedID "
			""
			"FROM LabsT INNER JOIN PersonT ON LabsT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			""
			"WHERE LabsT.ID = {INT} ", m_nInitialLabID);
		if(!rs->eof){
			CString strAge="";

			// (z.manning 2008-10-30 12:48) - PLID 31613 - Load the lab procedure ID if it hasn't been set already
			if(m_nLabProcedureID == -1) {
				SetLabProcedureID(AdoFldLong(rs->GetFields(), "LabProcedureID"));
			}

			m_nPatientID = AdoFldLong(rs, "PatientID", -1);

			// (r.galicki 2008-10-28 15:06) - PLID 31552 - procedure type
			SetLabProcedureType(LabType(AdoFldByte(rs->GetFields(),"Type", ltBiopsy)));

			SetCurrentDate(rs->Fields->Item["CurDate"]->Value);
			
			//We want all dates to default to an invalid date
			COleDateTime dtDefault = COleDateTime(0.00);
			dtDefault.SetStatus(COleDateTime::invalid);

			//Patient Name
			//(e.lally 2007-08-08) PLID 26978 - track the patient name to save with the lab record
			// (d.singleton 2013-10-25 14:41) - PLID 59195 - need patient id in the patient name field of the lab entry dialog
			m_strPatientFirst = AdoFldString(rs, "PatientFirst");
			m_strPatientMiddle = AdoFldString(rs, "PatientMiddle");
			m_strPatientLast = AdoFldString(rs, "PatientLast");
			m_strUserDefinedID = AsString(AdoFldLong(rs, "UserDefinedID"));
			CString strCellPhone = AdoFldString(rs, "CellPhone", "");
			CString strHomePhone = AdoFldString(rs, "HomePhone", "");
			CString strPatientName;
			strPatientName.Format("%s, %s %s (%s)", m_strPatientLast, m_strPatientFirst, m_strPatientMiddle, m_strUserDefinedID);
			strPatientName.TrimRight();
			SetDlgItemText (IDC_LAB_PATIENT, strPatientName);

			// (b.spivey, August 23, 2013) - PLID 58015 - Put the cellphone and homephone in the title bar. 
			CString strWindowText = "";
			strWindowText += (strPatientName) + (strHomePhone.IsEmpty() ? "" : " - Home Phone: " + strHomePhone) +
				(strCellPhone.IsEmpty() ? "" : " - Mobile Phone: " + strCellPhone) + " - Lab Entry" ; 

			// (c.haag 2010-07-20 15:14) - PLID 34338 - Make it easier for users to see which patient a minimized lab is for
			// (though in some Windows themes it won't help much)
			SetWindowText(strWindowText);

			

			//Gender
			if(rs->Fields->Item["Gender"]->Value.vt !=VT_NULL){
				m_nGender = AdoFldByte(rs, "Gender");
				if(m_nGender == 1)
					SetDlgItemText (IDC_LAB_GENDER, "Male");
				else if(m_nGender == 2)
					SetDlgItemText (IDC_LAB_GENDER, "Female");
			}

			//Age - based on biopsy date

			// (j.jones 2010-05-06 10:00) - PLID 38524 - do not display the age yet,
			// its is dependent on the biopsy date, which we haven't loaded yet
			m_varBirthDate = rs->Fields->Item["BirthDate"]->Value;

			//Form Number
			m_strSavedFormNumberTextID = AdoFldString(rs, "FormNumberTextID");
			SetDlgItemText (IDC_FORM_NUMBER, m_strSavedFormNumberTextID);
		}
		rs->Close();

		//TES 11/17/2009 - PLID 36190 - Tell the requisition tab to load
		m_dlgRequisitions.LoadExisting();
		//TES 11/19/2009 - PLID 36191 - And the results tab.
		m_dlgResults.LoadExisting();

		// (j.jones 2010-05-06 09:22) - PLID 38520 - supported input date
		// (c.haag 2010-09-13 11:19) - PLID 38989 - Show the input time as well
		SetDlgItemText(IDC_LAB_INPUT_DATE, FormatDateTimeForInterface(m_dlgRequisitions.GetFirstInputDate(), NULL, dtoDateTime));

		// (j.jones 2010-05-06 10:01) - PLID 38524 - moved age calculation into its own function
		CalculateAndDisplayPatientAge();

	}NxCatchAll("Error in CLabEntryDlg::LoadExisting");

}

//DRT 7/11/2006 - Generic save which handles figuring out
//	whether it's new or not, and let's the caller know.
BOOL CLabEntryDlg::Save()
{
	//Check if required fields have data.
	if(!IsLabValid())
		return FALSE;

	long nAuditTransID = 0;

	try{
		//TES 11/25/2009 - PLID 36193 - We no longer differentiate between new and existing, we leave that up to our subtabs.  
		// Our job here is to gather all the information together that will be shared by all labs, pass it in to our subtabs, and 
		// let them handle it from there.
		nAuditTransID = BeginAuditTransaction();

		long nNewLabID = -1;

		//TES 10/31/2013 - PLID 59251 - Track any new interventions that are triggered
		CDWordArray arNewCDSInterventions;

		//TES 11/25/2009 - PLID 36193 - Fill in a struct with the information all our Labs will need.
		SharedLabInformation sli;
		//Form Number
		GetDlgItemText(IDC_FORM_NUMBER, sli.strFormTextID);

		sli.strPatientFirst = m_strPatientFirst;
		sli.strPatientMiddle = m_strPatientMiddle;
		sli.strPatientLast = m_strPatientLast;

		// (z.manning 2008-10-06 14:48) - PLID 21094 - Save the source action data if we have it
		sli.strSourceActionID = "NULL";
		if(VarLong(m_varSourceActionID, -1) != -1) {
			sli.strSourceActionID = AsString(m_varSourceActionID);
		}
		sli.strSourceDetailID = "NULL";
		if(VarLong(m_varSourceDetailID, -1) != -1) {
			sli.strSourceDetailID = AsString(m_varSourceDetailID);
		}
		// (z.manning 2009-02-27 10:12) - PLID 33141 - SourceDataGroupID
		sli.strSourceDataGroupID = "NULL";
		if(VarLong(m_varSourceDataGroupID, -1) != -1) {
			sli.strSourceDataGroupID = AsString(m_varSourceDataGroupID);
		}
		// (z.manning 2010-02-26 16:42) - PLID 37540
		sli.strSourceDetailImageStampID = "NULL";
		if(VarLong(m_varSourceDetailImageStampID, -1) != -1) {
			sli.strSourceDetailImageStampID = AsString(m_varSourceDetailImageStampID);
		}

		// (r.galicki 2008-10-08 13:00) - PLID 31555 - Added PIC ID to labs
		sli.strPicID = "NULL";
		if(m_nPIC > 0) {
			sli.strPicID.Format("%li", m_nPIC);
		}

		// (z.manning 2009-05-08 09:46) - PLID 28554 - Order set id
		sli.strOrderSetID = "NULL";
		if(m_varOrderSetID.vt == VT_I4) {
			sli.strOrderSetID = AsString(m_varOrderSetID);
		}

		//TES 11/25/2009 - PLID 36193 - Tell our requisitions to save.
		//TES 12/1/2009 - PLID 36452 - This will output the ID of the new lab (if any) that gets created.
		//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
		m_dlgRequisitions.Save(nAuditTransID, sli, nNewLabID, arNewCDSInterventions);

		// (j.gruber 2008-09-22 13:25) - PLID <ADD PLID> 
		//we save and audit the results at the same time, so do that here
		BOOL bSpawnToDo = FALSE;
		//TES 11/20/2009 - PLID 36191 - Tell the Results tab to save
		//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
		m_dlgResults.Save(nNewLabID, nAuditTransID, bSpawnToDo, arNewCDSInterventions);

		// (z.manning 2008-10-13 16:35) - PLID 31667 - We now also have per-step todos.
		//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
		SyncTodoWithLab(nNewLabID, m_nPatientID);

		// (m.hancock 2006-07-12 13:57) - PLID 21353 - Spawn a to-do task after we have saved if the result has been set
		if(bSpawnToDo) { //Should we attempt to spawn a new to-do task?
			SpawnToDo(nNewLabID);
		}

		// (z.manning, 07/28/2006) - PLID 21576 - The form number has been saved, so we don't need to save the counter anymore.
		m_nLastFormNumberCount = -1;
		
		//DRT 7/3/2006 - PLID 21083 - Commit our audit.
		if(nAuditTransID != 0) {
			CommitAuditTransaction(nAuditTransID);
		}

		// (z.manning 2010-05-13 14:11) - PLID 37405 - We just saved so any pending to-dos are now committed
		m_dlgRequisitions.ClearAllPendingTodos();
		
		//TES 10/31/2013 - PLID 59251 - If any interventions were triggered, notify the user
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

		//we're done!
		m_bDataChanged = true;
		return TRUE;

	}NxCatchAll("Error in CLabEntryDlg::Save()");

	//If we get here, there was an exception.  Must rollback the audit
	if(nAuditTransID != 0) {
		RollbackAuditTransaction(nAuditTransID);
	}

	return FALSE;
}

void CLabEntryDlg::OnSaveAndAddNew() 
{
	try {
		//DRT 7/10/2006 - PLID 21088 - Users with "Write" permission may get into this dialog, but not
		//	be allowed to make anything new.
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptCreate))
			return;

		if (!m_dlgResults.CheckResultSave(TRUE)) {
			return;
		}

		if(!Save())
			return;

		AddNew();

	} NxCatchAll("Error in CLabEntryDlg::OnSaveAndAddNew");
}

// (z.manning 2010-03-16 15:50) - PLID 37439 - Function to add a new lab specimen to the current lab group
void CLabEntryDlg::AddNew()
{
	//TES 11/17/2009 - PLID 36190 - Tell the Requisition tab to reset everything.
	//TES 11/25/2009 - PLID 36193 - Actually, just tell it to add a new requisition.
	m_dlgRequisitions.AddNew(m_bAddNewLabImmediatelyOnLoad);
	//TES 11/19/2009 - PLID 36191 - And the results tab
	//TES 11/30/2009 - PLID 36452 - Likewise, tell it to add a new requisition, rather than clearing everything out.
	m_dlgResults.AddNew();
}

void CLabEntryDlg::OnSaveAndResumePrevious() 
{
	try {
		//(e.lally 2011-09-20) PLID 45527 - Added separate code block so that all local variables go out of scope (and especially so the button disabler calls its destructor)
		//	prior to the CodeCleanup being called.
		{

			// (b.spivey, July 17, 2013) - PLID 45194
			if (NXTWAINlib::IsAcquiring())
			{
				MsgBox("Please wait for the current image acquisition to complete before closing this dialog.");
				return;
			}

			CLabEntryButtonDisabler btndisabler(this); // (z.manning 2010-11-16 15:46) - PLID 41499

			if (!m_dlgResults.CheckResultSave(TRUE)) {
				return;
			}

			if(!Save())
				return;
			
			// (a.vengrofski 2010-06-10 16:26) - PLID <38544>
			if (!AutoExportTheLab())
			{
				return;
			}

			CDialog::OnOK();
		}
		
		CloseCleanup(IDOK); // (c.haag 2010-07-15 17:23) - PLID 34338

		//(e.lally 2011-09-20) PLID 45527 - WARNING: if m_hwndNotify is not null, the current instance 
		//	of this dialog may be destroyed by this point. Any attempts to use anything (like member variables)
		//	associated with this instance of the dlg will crash and burn if that is the case.

	} NxCatchAll("Error in CLabEntryDlg::OnSaveAndResumePrevious");
	
}

BOOL CLabEntryDlg::IsLabValid()
{
	try{
		//We MUST check for no selection (curSel == NULL) here because we assume when we get the
			//values in the save functions that it has a valid selection.

		//Form Number
		CString strFormNumber="";
		GetDlgItemText(IDC_FORM_NUMBER, strFormNumber);
		if(strFormNumber.IsEmpty()){
			MessageBox("This lab cannot be saved because you must enter a Form Number first.");
			return FALSE;
		}
		// (b.eyers 2015-08-06) - PLID 43031 - updated field to 200
		if(strFormNumber.GetLength() > 200){
			MessageBox("The Form Number entered is longer than 200 characters. "
				"Please shorten it before saving.");
			GetDlgItem(IDC_FORM_NUMBER)->SetFocus();
			return FALSE;
		}

		//(e.lally 2006-08-07) PLID 21642 - We need to check if this form number
		//already exists for another patient, but only if it has changed (or is new).
		//We will allow it to be re-used if the other records are marked as deleted.
		// (z.manning 2014-06-27 11:06) - PLID 62456 - Parameterized
		if(strFormNumber != m_strSavedFormNumberTextID)
		{
			if(ReturnsRecordsParam(
				"SELECT ID FROM LabsT \r\n"
				"WHERE FormNumberTextID = {STRING} AND PatientID <> {INT} AND Deleted = 0"
				, strFormNumber, m_nPatientID))
			{
				MessageBox("This Form Number is already in use by another patient, please "
					"enter a new Form Number before saving.");
				return FALSE;
			}
		}

		//TES 11/17/2009 - PLID 36190 - First ask our Requisition tab
		//TES 11/25/2009 - PLID 36193 - Tell it to check all labs
		if(!m_dlgRequisitions.AreAllLabsValid()) {
			return FALSE;
		}

		return TRUE;

	}NxCatchAll("Error in CLabEntryDlg::IsLabValid");
	return FALSE;
}

// (c.haag 2007-03-16 09:49) - PLID 21622 - This function has been depreciated now that
// we use a multi-select dialog to choose diagnoses.
/*
void CLabEntryDlg::OnSelChosenLabDescriptionOptionsList(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pSelRow(lpRow);

		if(pSelRow != NULL) {
			long nDiagIDAdded = VarLong(pSelRow->GetValue(ldolcID));

			//See if it's already been selected
			if(m_pLabDescList->FindByColumn(ldlcDiagID, (long)nDiagIDAdded, NULL, FALSE) == NULL) {
				//Not found, we can add it anew
				IRowSettingsPtr pNewRow = m_pLabDescList->GetNewRow();
				pNewRow->PutValue(ldlcLabID, (long)m_nLabID);
				pNewRow->PutValue(ldlcDiagID, (long)nDiagIDAdded);
				pNewRow->PutValue(ldlcDesc, pSelRow->GetValue(ldolcDesc));
				m_pLabDescList->AddRowAtEnd(pNewRow, NULL);

				//Now that we have added it, check our map.  Some unscrupulous user might have
				//	deleted the code then re-added it.  We can save some work by just getting
				//	it out of our deleted list.
				long nTmp = 0;
				if(m_mapDeletedLabDesc.Lookup(nDiagIDAdded, nTmp)) {
					//Found in deleted list, get rid of it
					m_mapDeletedLabDesc.RemoveKey(nDiagIDAdded);
				}
				else {
					//This is a new diagnosis
					m_mapNewLabDesc.SetAt(nDiagIDAdded, nDiagIDAdded);
				}
			}
			else {
				//The diagnosis ID was found so tell the user.
				MessageBox("This diagnosis has already been added to the list.");
			}
		}

	} NxCatchAll("Error in OnSelChosenLabDescriptionOptionsList");
}*/

void CLabEntryDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		
		// (j.jones 2007-07-19 16:34) - PLID 26751 - no right click menus are needed currently,
		// as I removed the diagnosis/clinical diag lists

	} NxCatchAll("Error in CLabEntryDlg::OnContextMenu");
}

// (z.manning 2008-11-04 09:01) - PLID 31904 - Changed this function to be able to print multiple forms at once
void CLabEntryDlg::PrintLabForms(BOOL bPreview)
{
	//(e.lally 2011-09-20) PLID 45527 - Added separate code block so that all local variables go out of scope (and especially so the button disabler calls its destructor)
	//	prior to the CodeCleanup being called.
	{
		CLabEntryButtonDisabler btndisabler(this); // (z.manning 2010-11-16 15:46) - PLID 41499

		// (z.manning 2008-11-03 17:19) - PLID 31904 - Need to decide which reports we'll be running
		BOOL bPrintLabel = m_nxbtnLabel.GetCheck() == BST_CHECKED;
		BOOL bPrintRequest = m_nxbtnRequestForm.GetCheck() == BST_CHECKED;
		BOOL bPrintResults = m_nxbtnResultsForm.GetCheck() == BST_CHECKED;
		if(!bPrintLabel && !bPrintRequest && !bPrintResults) {
			MessageBox(FormatString("You must select at least one report before %s.", bPreview ? "previewing" : "printing"));
			return;
		}

		//TES 1/29/2010 - PLID 34439 - If they're printing a request form, make sure they have one selected.
		if(bPrintRequest) {
			if(m_pRequestFormsList->CurSel == NULL) {
				MessageBox(FormatString("Please select a Request Form to %s.", bPreview ? "preview" : "print"));
				return;
			}
		}
		//TES 7/27/2012 - PLID 51071 - Likewise for Results Forms
		if(bPrintResults) {
			if(m_pResultFormsList->CurSel == NULL) {
				MessageBox(FormatString("Please select a Results Form to %s.", bPreview ? "preview" : "print"));
				return;
			}
		}

		//DRT 7/3/2006 - PLID 21204 - Get the form ID from the interface, we will preview the report
		//	displaying all labs with the same form ID.
		CString strForm;
		GetDlgItemText(IDC_FORM_NUMBER, strForm);

		if (!m_dlgResults.CheckResultSave(TRUE)) {
			return;
		}


		//Close the dialog & save
		if(!Save())
			return;

		//TES 10/7/2010 - PLID 40589 - Moved here from OnPrint()/OnPreview(), so that it wouldn't be sent until AFTER it was saved.
		// (a.vengrofski 2010-05-13 11:15) - PLID <38544> - Send the send the lab via HL7
		if (!AutoExportTheLab())
		{
			return;
		}

		//Additionally, this is the point at which specimen identifiers are created for all specimens
		//	which have this form ID and do not yet have a specimen
		// (z.manning 2010-06-02 14:14) - PLID 38976 - We now generate specimen codes when creating the 
		// labs so I got rid of the code here that did that.

		// (z.manning 2008-11-04 09:03) - PLID 31904 - Do we want to print a label for this lab?
		if(bPrintLabel)
		{
			CPrintInfo prLabelInfo;
			// (a.walling 2010-06-09 08:23) - PLID 37876 - Initialize HGLOBALS to NULL
			HGLOBAL hDevMode = NULL, hDevNames = NULL;
			if(!bPreview) {
				// (z.manning 2010-05-14 10:03) - PLID 37876 - We now have an option for separate print settings
				// specifically for lab labels.
				// (z.manning 2010-11-22 12:47) - PLID 40486 - New function for this
				// Retrieve the label printer settings based on the 
				// System Property Specificity preference
				LabUtils::GetLabLabelPrintSettings(hDevNames, hDevMode);
				CPrintDialog* dlg;
				dlg = new CPrintDialog(FALSE);
				prLabelInfo.m_bPreview = FALSE;
				prLabelInfo.m_bDocObject = FALSE;
				if(prLabelInfo.m_pPD != NULL) {
					delete prLabelInfo.m_pPD;
				}
				prLabelInfo.m_pPD = dlg;
				if(hDevMode != NULL && hDevNames != NULL) {
					// (z.manning 2010-05-14 10:04) - PLID 37876 - We found settings so don't waste time prompting
					// for a printer and just print directly to what they setup.
					prLabelInfo.m_bDirect = TRUE;
					prLabelInfo.m_pPD->m_pd.hDevMode = hDevMode;
					prLabelInfo.m_pPD->m_pd.hDevNames = hDevNames;
				}
			}

			CReportInfo  infLabel(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(654)]);
			infLabel.strExtraText = FormatString(" AND FormNumberTextID = '%s' ",  _Q(strForm));
			RunReport(&infLabel, bPreview, this, "Lab Specimen Label", &prLabelInfo);

			if(hDevMode != NULL) {
				GlobalFree(hDevMode);
				hDevMode = NULL;
			}
			if(hDevNames != NULL) {
				GlobalFree(hDevNames);
				hDevNames = NULL;
			}

			// (r.galicki 2008-11-05 17:03) - PLID 27214 - Update HasPrintedLabel field
			ExecuteSql("UPDATE LabsT SET HasPrintedLabel = 1 WHERE FormNumberTextID = '%s' AND Deleted = 0", _Q(strForm));
		}
		
		CPrintInfo prInfo;
		if (!bPreview) {
			CPrintDialog* dlg;
			dlg = new CPrintDialog(FALSE);
			prInfo.m_bPreview = false;
			prInfo.m_bDirect = false;
			prInfo.m_bDocObject = false;
			if(prInfo.m_pPD != NULL) {
				delete prInfo.m_pPD;
			}
			prInfo.m_pPD = dlg;
		}

		//Now that we've saved, run the report
		//TES 3/16/2009 - PLID 33468 - The results and request forms are now two separate reports, so check them each
		// separately.
		if(bPrintResults) {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(567)]);
			infReport.nPatient = m_nPatientID;	//If we ever want to allow 1 form for multiple patients, just remove this.
			infReport.AddExtraValue(strForm);	//We're hijacking the "extra values" parameter, which is the "use extended" list.			
			CPtrArray aryParams;
			//TES 7/27/2012 - PLID 51071 - Check which report they want to run.
			IRowSettingsPtr pRow = m_pResultFormsList->CurSel;
			long nReportNum = VarLong(pRow->GetValue(rflcNumber),-1);
			if(nReportNum == -1) {
				//TES 7/27/2012 - PLID 51071 - They want the system default.
				infReport.ViewReport(infReport.strPrintName, infReport.strReportFile, &aryParams, bPreview, this, &prInfo);
			}
			else {
				//TES 7/27/2012 - PLID 51071 - They want one of the custom ones, so we need to look up the filename.
				CString strFileName = VarString(pRow->GetValue(rflcReportName),"");
				infReport.nDefaultCustomReport = nReportNum;
				infReport.ViewReport(infReport.strPrintName, strFileName, &aryParams, bPreview, this, &prInfo);
			}
			ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
		}
		if(bPrintRequest) {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(658)]);
			infReport.nPatient = m_nPatientID;	//If we ever want to allow 1 form for multiple patients, just remove this.
			infReport.AddExtraValue(strForm);	//We're hijacking the "extra values" parameter, which is the "use extended" list.

			// (r.gonet 10/11/2011) - PLID 46437 - So am I, for barcodes with custom fields in them, which must be generated at runtime
			CString strBarcode = "", strOverflowBarcode = "";
			IRowSettingsPtr pCurRow = m_pRequestFormsList->CurSel;
				
			BOOL bGenerateBarcode = VarBool(pCurRow->GetValue(rflcGenerateBarcode), FALSE);
			if(bGenerateBarcode) {
				GenerateBarcodes(strBarcode, strOverflowBarcode);
			}
			infReport.AddExtraValue(strBarcode);
			infReport.AddExtraValue(strOverflowBarcode);

			// (r.gonet 10/19/2011) - PLID 46039 - The report demands the HL7 Settings Group ID. This feels like a hack.
			//  at some point, I think we need to add extra objects to the report instead of just extra strings.
			long nHL7GroupID = -1;
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabLinkList->CurSel;
				if(pRow) {
					nHL7GroupID = VarLong(pRow->GetValue(lllID), -1);
				}
			}
			infReport.AddExtraValue(FormatString("%li", nHL7GroupID));

			CPtrArray aryParams;
			//TES 1/29/2010 - PLID 34439 - Check which report they want to run.
			IRowSettingsPtr pRow = m_pRequestFormsList->CurSel;
			long nReportNum = VarLong(pRow->GetValue(rflcNumber),-1);
			if(nReportNum == -1) {
				//TES 1/29/2010 - PLID 34439 - They want the system default.
				infReport.ViewReport(infReport.strPrintName, infReport.strReportFile, &aryParams, bPreview, this, &prInfo);
			}
			else {
				//TES 1/29/2010 - PLID 34439 - They want one of the custom ones, so we need to look up the filename.
				CString strFileName = VarString(pRow->GetValue(rflcReportName),"");
				infReport.nDefaultCustomReport = nReportNum;
				infReport.ViewReport(infReport.strPrintName, strFileName, &aryParams, bPreview, this, &prInfo);
			}
			ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
		}

		//close up the lab entry
		CDialog::OnOK();

		//flag our caller to close
		// (z.manning 2008-11-04 09:26) - PLID 31904 - Not necessary when printing
		if(bPreview) {
			m_bOpenedReport = true;
		}

	}

	CloseCleanup(IDOK); // (c.haag 2010-07-15 17:23) - PLID 34338

	//(e.lally 2011-09-20) PLID 45527 - WARNING: if m_hwndNotify is not null, the current instance 
	//	of this dialog may be destroyed by this point. Any attempts to use anything (like member variables)
	//	associated with this instance of the dlg will crash and burn if that is the case.
}

void CLabEntryDlg::OnPrint() 
{
	try {

		//	(b.spivey, July 17, 2013) - PLID 45194
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before closing this dialog.");
			return;
		}

		//TES 10/7/2010 - PLID 40589 - Moved to PrintLabForms(), at this point the lab hasn't even been saved yet.
		/*// (a.vengrofski 2010-05-13 11:15) - PLID <38544> - Send the send the lab via HL7
		if (!AutoExportTheLab())
		{
			return;
		}*/

		PrintLabForms(FALSE);

	} NxCatchAll("CLabEntryDlg::OnPrint");
}

void CLabEntryDlg::OnPreview() 
{
	try {

		// (b.spivey, July 17, 2013) - PLID 45194
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before closing this dialog.");
			return;
		}

		// (j.jones 2010-04-19 12:45) - PLID 37875 - if the Labs Needing Attention dialog
		// intends to open another lab when this one closes, that will not occur if they
		// print preview something, so give a DontShowMeAgain warning about this
		if(m_bNextLabWillOpenOnClose) {
			if(IDNO == DontShowMeAgain(this, "The Labs Needing Attention screen will not open the next lab in the list if you print preview a lab form.\n"
				"Are you sure you wish to continue?", 
				"LabEntryDlg_PreviewLabWithAutoOpenEnabled", "Warning", FALSE, TRUE)) {
				//they do not want to preview
				return;
			}
		}

		//TES 10/7/2010 - PLID 40589 - Moved to PrintLabForms(), at this point the lab hasn't even been saved yet.
		/*// (a.vengrofski 2010-05-13 11:15) - PLID <38544> - Send the send the lab via HL7
		if (!AutoExportTheLab())
		{
			return;
		}*/

		PrintLabForms(TRUE);

	} NxCatchAll("CLabEntryDlg::OnPreview");
}

// (z.manning 2010-05-12 15:48) - PLID 37405 - Added lab ID parameter
void CLabEntryDlg::SpawnToDo(const long nLabID)
{
	try {
		// (m.hancock 2006-07-12 10:56) - PLID 21353 - When user selects a lab result, spawn a to-do task.
		// In the future we will probably add to-do spawning per step, similar to tracking, but for now, most users
		// will simply want to take action when the result or diagnosis of a lab is set.  In many cases, this will
		// likely be a tracking related action, such as calling the patient or scheduling an appointment.

		//Check preference to see if we should even create the to-do task
		bool bSpawnToDo = GetRemotePropertyInt("LabSpawnToDo",1,0,"<None>",true) == 1 ? true : false;
		if(!bSpawnToDo) //We're not supposed to be spawning the to-do task, so just return
			return;

		//Create a new todo from the task edit dialog 

		// (j.jones 2008-11-14 10:38) - PLID 31208 - we decided you should not be required to have patient permissions 
		//to create a todo alarm
		/*
		if (!CheckCurrentUserPermissions(bioPatient, sptWrite))
			return;
		*/
		
		//TES 10/10/2008 - PLID 31646 - I copied much of this function into a new global function.
		//TES 8/6/2013 - PLID 51147 - Pass in our stored priority
		long nTaskID = TodoCreateForLab(m_nPatientID, nLabID, m_arystrToDoDesc, false, m_TodoPriority);

		if(nTaskID == -1) {
			//TES 10/10/2008 - PLID 31646 - No ToDo was created.
			return;
		}

		m_arystrToDoDesc.RemoveAll();
		//TES 8/6/2013 - PLID 51147 - Reset m_TodoPriority and m_bTodoHasFlag
		m_TodoPriority = (TodoPriority)GetRemotePropertyInt("Lab_DefaultTodoPriority", 1, 0, "<None>");
		m_bTodoHasFlag = false;
		
		//TES 10/10/2008 - PLID 31646 - OK, we have a new ToDo, give them a chance to modify it.
		CTaskEditDlg dlg(this);
		//(e.lally 2010-02-25) PLID 37542 - Set the personID to this patientID
		dlg.m_nPersonID = m_nPatientID;
		dlg.m_iTaskID = nTaskID;
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlg.m_bIsNew = TRUE;

		//Open the to-do
		if (dlg.DoModal() != IDOK) {
			//TES 10/10/2008 - PLID 31646 - They cancelled, meaning they don't want a ToDo at all, so let's delete it
			// (this is the same functionality that existed prior to my changes).
			TodoDelete(nTaskID, false);
			return; //User canceled
		}

		// Added by CH 1/17: Force the next remind
		// time to be 5 minutes if a new task is
		// added so the "Don't remind me again"
		// option will not cause the new task to
		// be forgotten.
		{
			COleDateTime dt = COleDateTime::GetCurrentTime();
			dt += COleDateTimeSpan(0,0,5,0);
			SetPropertyDateTime("TodoTimer", dt);
			// (j.dinatale 2012-10-22 17:59) - PLID 52393 - set our user preference
			SetRemotePropertyInt("LastTimeOption_User", 5, 0, GetCurrentUserName());
			SetTimer(IDT_TODO_TIMER, 5*60*1000, NULL);
		}

	}NxCatchAll("Error in CLabEntryDlg::SpawnToDo");
}

bool CLabEntryDlg::HasDataChanged()
{
	return m_bDataChanged;
}

bool CLabEntryDlg::HasOpenedReport()
{
	return m_bOpenedReport;
}


//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CLabEntryDlg::OnOK()
{
	//Eat the message
}

// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
void CLabEntryDlg::SetDefaultLocationID(long nLocationID)
{
	//this only exists on the requisitions tab
	m_dlgRequisitions.SetDefaultLocationID(nLocationID);
}

// (r.gonet 07/22/2013) - PLID 57683 - Sets the providers from the source EMN.
void CLabEntryDlg::SetEMNProviders(CDWordArray &dwProviderIDs)
{
	m_dlgRequisitions.SetEMNProviders(dwProviderIDs);
}

// (z.manning 2008-10-06 14:44) - PLID 21094
void CLabEntryDlg::SetSourceActionID(const long nSourceActionID)
{
	m_varSourceActionID = nSourceActionID;
}

// (z.manning 2008-10-06 14:44) - PLID 21094
void CLabEntryDlg::SetSourceDetailID(const long nSourceDetailID)
{
	m_varSourceDetailID = nSourceDetailID;
}

// (z.manning 2009-02-27 10:09) - PLID 33141
void CLabEntryDlg::SetSourceDataGroupID(const long nSourceDataGroupID)
{
	if(nSourceDataGroupID == -1) {
		m_varSourceDataGroupID.vt = VT_NULL;
	}
	else {
		m_varSourceDataGroupID = nSourceDataGroupID;
	}
}

// (z.manning 2010-02-26 16:54) - PLID 37540
void CLabEntryDlg::SetSourceDetailImageStampID(const long nSourceDetailImageStampID)
{
	if(nSourceDetailImageStampID == -1) {
		m_varSourceDetailImageStampID.vt = VT_NULL;
	}
	else {
		m_varSourceDetailImageStampID = nSourceDetailImageStampID;
	}
}

// (z.manning 2009-05-08 09:39) - PLID 28554 - Sets the order set ID if this lab is part of an order set.
void CLabEntryDlg::SetOrderSetID(const long nOrderSetID)
{
	m_varOrderSetID = nOrderSetID;
}

void CLabEntryDlg::SetDefaultToBeOrdered(const CString &strToBeOrdered)
{
	//TES 11/17/2009 - PLID 36190 - Just tell our requisition tab
	m_dlgRequisitions.SetDefaultToBeOrdered(strToBeOrdered);
}

//TES 11/25/2009 - PLID 36191 - All problem handling moved to CLabRequisitionDlg

void CLabEntryDlg::OnSelectTabLabEntryTabs(short newTab, short oldTab)
{
	try {
		//TES 11/20/2009 - PLID 36190 - Call our worker function.
		ReflectCurrentTab();
	}NxCatchAll("Error in CLabEntryDlg::OnSelectTabLabEntryTabs()");
}


void CLabEntryDlg::ReflectCurrentTab()
{
	//TES 11/20/2009 - PLID 36190 - Show the appropriate subdialog, hide the others.
	if(m_tab->CurSel == 0) {
		m_dlgRequisitions.ShowWindow(SW_SHOWNA);
		m_dlgResults.ShowWindow(SW_HIDE);
	}
	else {
		m_dlgRequisitions.ShowWindow(SW_HIDE);
		m_dlgResults.ShowWindow(SW_SHOWNA);
	}
}

//TES 11/20/2009 - PLID 36191 - Called by CLabResultsDlg when a result has changed, requiring a ToDo Alarm to be created
//TES 8/6/2013 - PLID 51147 - Pass in a TodoPriority as well, and whether that priority was loaded from an abnormal flag
void CLabEntryDlg::AddToDoDescription(const CString &strToDoDesc, TodoPriority priority, bool bIsFlag)
{
	//TES 8/6/2013 - PLID 51147 - If the passed-in priority came from a flag, and our stored priority either didn't, or did but is lower
	// than the passed-in flag, then update the stored priority to the passed-in priority
	if(bIsFlag) {
		if(!m_bTodoHasFlag) {
			m_bTodoHasFlag = true;
			m_TodoPriority = priority;
		}
		else {
			if(priority < m_TodoPriority) {
				m_TodoPriority = priority;
			}
		}
	}
	
	m_arystrToDoDesc.Add(strToDoDesc);
}

void CLabEntryDlg::SetCurrentDate(_variant_t varCurDate)
{
	m_varCurDate = varCurDate;
	//TES 11/20/2009 - PLID 36190 - Our Requisition tab also needs to know this date
	m_dlgRequisitions.SetCurrentDate(varCurDate);
	//TES 11/20/2009 - PLID 36191 - Also the results tab
	m_dlgResults.SetCurrentDate(varCurDate);
}
void CLabEntryDlg::OnEditFormNumberFormat()
{
	try {
		// (r.gonet 03/29/2012) - PLID 45856 - Edit our group's lab form number format.
		CLabFormNumberEditorDlg dlg(m_nLabProcedureGroupID, this);
		if(dlg.DoModal() == IDOK) {
			if(m_bIsNewLab) {
				// (z.manningj, 07/24/2006) - PLID 21576 - If this is a new form, let's update the form
				// number to reflect the new format. (We decrement the FormNumberCount to reuse the
				// same count that we just did since it's not been saved yet.)
				// (z.manning 2010-02-02 09:50) - PLID 34808 - Don't do this if they manually changed the
				// increment value.
				if(!dlg.m_bIncrementValueChanged) {
					if(m_nLastFormNumberCount > 0) {
						// (r.gonet 03/29/2012) - PLID 45856 - Added lab procedure group id
						SetLabFormNumberCounterIfLess(m_nLastFormNumberCount, m_nLabProcedureGroupID);
					}
				}
				CString strFormNumber = GetNewFormNumber();
				SetDlgItemText (IDC_FORM_NUMBER, strFormNumber);

				//TES 11/30/2009 - PLID 36193, 36452 - Tell our subtabs about the new form number
				HandleNewFormNumber();
			}
		}
	}NxCatchAll("Error in CLabEntryDlg::OnEditFormNumberFormat()");
}

CString CLabEntryDlg::GetNewFormNumber() 
{
	// (z.manning, 07/25/2006) - PLID 21576 - We now generate lab form numbers based on a global counter
	// and a format which they can configure.
	try {
		// (r.gonet 03/29/2012) - PLID 45856 - Get this now from the LabProcedureGroupsT table.
		CString strFormat = GetLabFormNumberFormat(m_nLabProcedureGroupID);

		// If we don't have a place to put the lab form number counter, then there's no point in
		// even trying to generate a unique form number;
		if(strFormat.Find("%n") == -1) {
			return "";
		}

		// (r.gonet 03/29/2012) - PLID 45856 - Construct the form number.
		CString strNewFormNumber = GetNewLabFormNumber(strFormat, m_nLastFormNumberCount, m_nLabProcedureGroupID);
		// Even though we may never save this lab, let's increment the form number
		// counter right away to ensure that 2 people creating labs at the
		// same time won't get the same form number.
		// (r.gonet 03/29/2012) - PLID 45856 - do this for the lab procedure group.
		IncrementLabFormNumberCounter(1, m_nLabProcedureGroupID);

		// We don't not want to automatically generate a form number that has already been used,
		// so let's check that.  We'll then arbitrarily try 3 more times to generate a new form number
		// and if that doesn't work, we'll just give up.
		int nAttempt = 1;
		BOOL bAlreadyUsed;
		do
		{
			bAlreadyUsed = ReturnsRecords("SELECT ID FROM LabsT WHERE FormNumberTextID = '%s' ", _Q(strNewFormNumber));
			if(bAlreadyUsed) {
				// Ok, it exists, we can't use this number.  We already incremented the counter, so let's try
				// the next possible form number.
				// (r.gonet 03/29/2012) - PLID 45856 - Make sure we are doing these for the lab procedure group.
				strNewFormNumber = GetNewLabFormNumber(strFormat, m_nLastFormNumberCount, m_nLabProcedureGroupID);
				IncrementLabFormNumberCounter(1, m_nLabProcedureGroupID);
			}
			nAttempt++;

		} while(bAlreadyUsed && nAttempt <= 3);

		// If we couldn't find an unused form number, then we're not going to generate one.
		if(bAlreadyUsed) {
			return "";
		}

		return strNewFormNumber;

	} NxCatchAll("Error in CLabEntryDlg::GetNewFormNumber");

	//some kind of failure
	return "";
}

//TES 11/25/2009 - PLID 36193 - Access the current form number
CString CLabEntryDlg::GetFormNumber()
{
	CString strForm;
	GetDlgItemText(IDC_FORM_NUMBER, strForm);
	return strForm;
}

//TES 11/25/2009 - PLID 36193 - Access the saved form number (used for auditing)
CString CLabEntryDlg::GetSavedFormNumber()
{
	return m_strSavedFormNumberTextID;
}

// (j.jones 2010-10-07 13:37) - PLID 40743 - allowed updating the saved form number
void CLabEntryDlg::SetSavedFormNumber(CString strFormNumberTextID)
{
	m_strSavedFormNumberTextID = strFormNumberTextID;
}

// (b.spivey - January 20, 2014) - PLID 46370 - Added mutator for InitialResultID
void CLabEntryDlg::SetInitialResultID(long nResultID)
{
	m_nInitialResultID = nResultID;
}
// (b.spivey - January 20, 2014) - PLID 46370 - Added accessor for InitialResultID
long CLabEntryDlg::GetInitialResultID()
{
	return m_nInitialResultID;
}

//TES 11/25/2009 - PLID 36193 - Instead of setting a lab ID, we now set an initial lab ID, because this dialog may have multiple labs on it.
void CLabEntryDlg::SetInitialLabID(long nLabID)
{
	m_nInitialLabID = nLabID;
	m_dlgRequisitions.SetInitialLabID(nLabID);
	//TES 11/30/2009 - PLID 36452 - Same for the Results tab
	m_dlgResults.SetInitialLabID(nLabID);
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - Returns the initial lab ID
long CLabEntryDlg::GetInitialLabID() const
{
	return m_nInitialLabID;
}

//TES 11/25/2009 - PLID 36193 - Called by the requisition dialog when it saves a new EMN-originated lab
void CLabEntryDlg::AddNewEmnLab(EMNLab lab)
{
	m_aryNewLabs.Add(lab);
}

void CLabEntryDlg::HandleSpecimenChange(long nLabID, const CString &strNewSpecimen)
{
	//TES 11/30/2009 - PLID 36452 - Our results tab will need to update its labels.
	m_dlgResults.HandleSpecimenChange(nLabID, strNewSpecimen);
}

void CLabEntryDlg::SetCurrentLab(long nLabID)
{
	//TES 11/30/2009 - PLID 36193, 36452 - Keep the Results and Requisitions tabs in sync, as far as showing the same lab.
	m_dlgRequisitions.SetCurrentLab(nLabID);
	m_dlgResults.SetCurrentLab(nLabID);
}

void CLabEntryDlg::OnKillfocusFormNumber()
{
	try {
		//TES 11/30/2009 - PLID 36193, 36452 - Our subtabs will need to reflect our new form number
		HandleNewFormNumber();
	} NxCatchAll("Error in CLabEntryDlg::OnKillfocusFormNumber()");
}

void CLabEntryDlg::HandleNewFormNumber()
{
	CString strFormNumber;
	GetDlgItemText(IDC_FORM_NUMBER, strFormNumber);
	//TES 11/30/2009 - PLID 36193 - Tell the Requisitions tab to update its labels.
	m_dlgRequisitions.HandleFormNumberChange(strFormNumber);
	//TES 11/30/2009 - PLID 36452 - And the results tab
	m_dlgResults.HandleFormNumberChange(strFormNumber);
}

// (r.gonet 10/11/2011) - PLID 46437 - Generate the barcodes for this lab
void CLabEntryDlg::GenerateBarcodes(OUT CString &strBarcode, OUT CString &strOverflowBarcode)
{
	long nHL7GroupID = -1;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabLinkList->CurSel;
	if(pRow) {
		nHL7GroupID = VarLong(pRow->GetValue(lllID), -1);
	}

	// (r.gonet 12/07/2011) - PLID 46434 - This now has to be dynamically allocated since CLabBarcode is abstract.
	//  If we ever have multiple formats, then we are going to need some way of selecting the correct barcode type.
	CLabBarcode *plabBarcode = new CLabCorpLabBarcode();
	plabBarcode->Load();
	plabBarcode->FillBarcode(m_nPatientID, GetFormNumber(), nHL7GroupID);
	CArray<CString, CString> aryBarcodes;
	plabBarcode->Encode(aryBarcodes);
	strBarcode = aryBarcodes.GetSize() > 0 ? aryBarcodes[0] : "";
	strOverflowBarcode = aryBarcodes.GetSize() > 1 ? aryBarcodes[1] : "";
	// (r.gonet 12/07/2011) - PLID 46434 - Don't forget to delete the barcode.
	delete plabBarcode;
}

//TES 2/1/2010 - PLID 37143 - Called by the Requisition when the Receiving Lab changes, selects the given report in the dropdown.  
	// -1 = System report, -2 = overall default (i.e., what they set as Default in the EditReportPickerDlg).
void CLabEntryDlg::SetRequestForm(long nCustomReportNumber)
{
	long nNewDefault;
	if(nCustomReportNumber == -2) {
		//TES 2/1/2010 - PLID 37143 - Get the overall default.
		nNewDefault = GetDefaultRequestForm();
	}
	else {
		nNewDefault = nCustomReportNumber;
	}
	
	//TES 2/1/2010 - PLID 37143 - Now, set the selection to match.  Note that we do NOT mess with the coloring here, the red record will
	// always be the overall report, I think this way will resolve more confusion than it creates.
	m_pRequestFormsList->SetSelByColumn(rflcNumber, nNewDefault);

}

long CLabEntryDlg::GetDefaultRequestForm()
{
	//TES 2/1/2010 - PLID 37143 - Do we need to look it up?
	if(m_nDefaultRequestForm == -2) {
		//TES 2/1/2010 - PLID 37143 - Yup, start with -1 (system default).
		m_nDefaultRequestForm = -1;
		_RecordsetPtr rsDefaultReport = CreateParamRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = 658");
		if(!rsDefaultReport->eof) {
			//TES 2/1/2010 - PLID 37143 - Found one, use it.
			m_nDefaultRequestForm = AdoFldLong(rsDefaultReport, "CustomReportID", -1);
		}
	}
	//TES 2/1/2010 - PLID 37143 - This variable is now up-to-date, return it.
	return m_nDefaultRequestForm;
}

//TES 7/27/2012 - PLID 51849 - Called by the Requisition when the Receiving Lab changes, selects the given report in the dropdown.  
	// -1 = System report, -2 = overall default (i.e., what they set as Default in the EditReportPickerDlg).
void CLabEntryDlg::SetResultsForm(long nCustomReportNumber)
{
	long nNewDefault;
	if(nCustomReportNumber == -2) {
		//TES 7/27/2012 - PLID 51849 - Get the overall default.
		nNewDefault = GetDefaultResultForm();
	}
	else {
		nNewDefault = nCustomReportNumber;
	}
	
	//TES 7/27/2012 - PLID 51849 - Now, set the selection to match.  Note that we do NOT mess with the coloring here, the red record will
	// always be the overall report, I think this way will resolve more confusion than it creates.
	m_pResultFormsList->SetSelByColumn(rflcNumber, nNewDefault);
}

long CLabEntryDlg::GetDefaultResultForm()
{
	//TES 7/27/2012 - PLID 51849 - Do we need to look it up?
	if(m_nDefaultResultForm == -2) {
		//TES 7/27/2012 - PLID 51849 - Yup, start with -1 (system default).
		m_nDefaultResultForm = -1;
		_RecordsetPtr rsDefaultReport = CreateParamRecordset("SELECT CustomReportID FROM DefaultReportsT WHERE ID = 567");
		if(!rsDefaultReport->eof) {
			//TES 7/27/2012 - PLID 51849 - Found one, use it.
			m_nDefaultResultForm = AdoFldLong(rsDefaultReport, "CustomReportID", -1);
		}
	}
	//TES 7/27/2012 - PLID 51849 - This variable is now up-to-date, return it.
	return m_nDefaultResultForm;
}

void CLabEntryDlg::OnRequestFormCheck()
{
	try {
		//TES 2/5/2010 - PLID 34439 - Enable the dropdown based on the checkbox.
		GetDlgItem(IDC_REQUEST_FORM_LIST)->EnableWindow(IsDlgButtonChecked(IDC_REQUEST_FORM_CHECK));
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-04-29 12:39) - PLID 38420
void CLabEntryDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);

		// (z.manning 2010-04-29 14:11) - PLID 38420 - Make sure to also reposition the child dialogs
		CRect rcSubDlg;
		if(IsWindow(GetDlgItem(IDC_TABS_PLACEHOLDER)->GetSafeHwnd())) {
			GetDlgItem(IDC_TABS_PLACEHOLDER)->GetWindowRect(&rcSubDlg);
			ScreenToClient(&rcSubDlg);
			m_dlgResults.MoveWindow(&rcSubDlg);
			m_dlgRequisitions.MoveWindow(&rcSubDlg);
		}
	}NxCatchAll(__FUNCTION__);
}

void CLabEntryDlg::OnDestroy()
{
	try
	{
		// (a.walling 2011-06-22 11:59) - PLID 44260 - Don't actually destroy the window until the end
		//CNxDialog::OnDestroy();

		if(!IsIconic())
		{
			CRect rcLabEntry;
			GetWindowRect(rcLabEntry);
			if(rcLabEntry.Height() > 50 && rcLabEntry.Width() > 50)
			{
				// (z.manning 2010-04-29 15:45) - PLID 38420 - As long as this dialog isn't minimized and it's of
				// a somewhat reasonable size, then remember its position and if it's maximized.
				if(IsZoomed()) {
					SetRemotePropertyInt("LabEntryDlgMaximied", 1, 0, GetCurrentUserName());
				}
				else {
					SetRemotePropertyInt("LabEntryDlgMaximied", 0, 0, GetCurrentUserName());
					SetRemotePropertyInt("LabEntryDlgWidth", rcLabEntry.Width(), 0, GetCurrentUserName());
					SetRemotePropertyInt("LabEntryDlgHeight", rcLabEntry.Height(), 0, GetCurrentUserName());
					SetRemotePropertyInt("LabEntryDlgTop", rcLabEntry.top, 0, GetCurrentUserName());
					SetRemotePropertyInt("LabEntryDlgLeft", rcLabEntry.left, 0, GetCurrentUserName());
				}
			}
		}

		// (a.walling 2011-06-22 11:59) - PLID 44260 - Ensure our child dialogs are destroyed, though that should happen anyway
		
		//TES 11/17/2009 - PLID 36190 - Destroy the requisition tab.
		m_dlgRequisitions.DestroyWindow();
		//TES 11/20/2009 - PLID 36191 - Also the results tab
		m_dlgResults.DestroyWindow();

	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

// (j.jones 2010-05-06 11:19) - PLID 38524 - moved age calculation into its own function
void CLabEntryDlg::CalculateAndDisplayPatientAge()
{
	try {

		m_strAge = "";

		//calculate the birthdate, by first biopsy date
		if(m_varBirthDate.vt != VT_NULL){
			COleDateTime dtBirthDate;
			dtBirthDate = VarDateTime(m_varBirthDate);
			// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
			//  validation should only be done when bdays are entered/changed
			m_strAge = GetPatientAgeOnDate(dtBirthDate, m_dlgRequisitions.GetFirstBiopsyDate(), TRUE);
		}
		// (z.manning 2010-01-13 11:22) - PLID 22672 - Age is now a string
		SetDlgItemText(IDC_LAB_AGE, m_strAge);

	}NxCatchAll(__FUNCTION__);
}

// (a.vengrofski 2010-05-11 15:06) - PLID <38547>
void CLabEntryDlg::OnSendHL7Check()
{
	try {
		//TES 2/5/2010 - PLID 34439 - Enable the dropdown based on the checkbox.
		GetDlgItem(IDC_HL7_LIST)->EnableWindow(IsDlgButtonChecked(IDC_SEND_HL7));
	}NxCatchAll(__FUNCTION__);
}

// (a.vengrofski 2010-05-11 15:34) - PLID <38547>
void CLabEntryDlg::OnSelChosenLabLinkList(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL)
		{
			m_pLabLinkList->SetSelByColumn(lllID, (long)-1);//If they select the NULL row
															//force the selection to { No Link }
		}
	}NxCatchAll("Error in CLabEntryDlg::OnSelChosenLabLinkList");
}

// (z.manning 2010-05-12 16:37) - PLID 37405
void CLabEntryDlg::OnBnClickedLabEntryCreateTodo()
{
	try
	{
		long nLabID = -1;
		CLabRequisitionDlg *pdlgReq = GetActiveLabRequisitionDlg();
		if(pdlgReq != NULL) {
			nLabID = pdlgReq->GetLabID();
		}

		// (z.manning 2010-05-13 12:15) - PLID 37405 - For now at least, default the to-do text to blank
		// and they can type in whatever they want.
		CStringArray arystrToDoDesc;
		//TES 8/6/2013 - PLID 51147 - Pass in the default priority, since this is not related to a result
		long nTaskID = TodoCreateForLab(m_nPatientID, nLabID, arystrToDoDesc, false, (TodoPriority)GetRemotePropertyInt("Lab_DefaultTodoPriority", 1, 0, "<None>"));
		CTaskEditDlg dlgTodo(this);
		dlgTodo.m_nPersonID = m_nPatientID;
		dlgTodo.m_iTaskID = nTaskID;
		//(c.copits 2010-12-06) PLID 40794 - Permissions for individual todo alarm fields
		dlgTodo.m_bIsNew = TRUE;

		// (z.manning 2010-05-13 12:17) - PLID 37405 - If they cancelled out of the to-do dialog then
		// delete the already fully created task.
		if(dlgTodo.DoModal() == IDOK) {
			if(pdlgReq != NULL) {
				// (z.manning 2010-05-13 14:28) - PLID 37405 - We need to keep track of this because...
				// A. If this is a new lab we may need to update the regarding ID later
				// B. If they cancel this dialog we want to delete any pending to-dos
				pdlgReq->AddPendingTodoID(nTaskID);
			}
		}
		else {
			TodoDelete(nTaskID, FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-13 11:35) - PLID 37405 - Returns the currently active lab req dialog
CLabRequisitionDlg* CLabEntryDlg::GetActiveLabRequisitionDlg()
{
	CLabRequisitionDlg *pdlgReq = NULL;
	// (z.manning 2010-05-13 12:11) - PLID 37405 - First check and see if we're on the results tab and
	// if so return the active requisition based on what's selected there. If that turns out to be nothing
	// then we'll just use the active requisition tab.
	if(m_tab->GetCurSel() == 1) {
		pdlgReq = m_dlgResults.GetActiveLabRequisitionDlg();
	}

	if(pdlgReq == NULL) {
		pdlgReq = m_dlgRequisitions.GetActiveLabRequisitionDlg();
	}

	return pdlgReq;
}

// (z.manning 2010-05-13 12:03) - PLID 37405
CLabRequisitionDlg* CLabEntryDlg::GetLabRequisitionDlgByID(const long nLabID)
{
	return m_dlgRequisitions.GetLabRequisitionDlgByID(nLabID);
}

// (a.vengrofski 2010-05-12 15:51) - PLID <37143> - Borrowed from above
//TES 2/1/2010 - PLID 37143 - Called by the Requisition when the Receiving Lab changes, selects the given lab in the dropdown.  
void CLabEntryDlg::SetHL7Link(long nLabID)
{
	// Don't bother setting an HL7 link, if they're not licensed for HL7
	if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
	{
		if (nLabID == -1) {// -1 = {No Link}
			m_pLabLinkList->SetSelByColumn(lllID, (long)-1);
			m_nxbtnSendHL7.SetCheck(FALSE);
			GetDlgItem(IDC_HL7_LIST)->EnableWindow(IsDlgButtonChecked(IDC_SEND_HL7));
		}
		else {
			//TES 6/24/2011 - PLID 44261 - New method for accessing HL7 settings
			CArray<long, long> arHL7GroupIDs;
			GetHL7SettingsGroupsBySetting("DefaultLabID", nLabID, arHL7GroupIDs);
			NXDATALIST2Lib::IRowSettingsPtr pRow = NULL;
			if (arHL7GroupIDs.GetSize() > 0) {
				pRow = m_pLabLinkList->SetSelByColumn(lllID, arHL7GroupIDs[0]);
			}
			if (!pRow) {
				m_pLabLinkList->SetSelByColumn(lllID, (long)-1);
				m_nxbtnSendHL7.SetCheck(FALSE);
				GetDlgItem(IDC_HL7_LIST)->EnableWindow(IsDlgButtonChecked(IDC_SEND_HL7));
			}
			else {
				//TES 6/24/2011 - PLID 44261 - New method for accessing HL7 settings
				BOOL bAutoExport = GetHL7SettingBit(arHL7GroupIDs[0], "AutoExportLabs");
				// (r.gonet 06/11/2013) - PLID 56389 - Try to Send via HL7 if the user has added any new labs.
				if (bAutoExport && m_dlgRequisitions.GetNewLabsCount() > 0) {
					m_nxbtnSendHL7.SetCheck(BST_CHECKED);
				}
				else {
					m_nxbtnSendHL7.SetCheck(BST_UNCHECKED);
				}
				GetDlgItem(IDC_HL7_LIST)->EnableWindow(IsDlgButtonChecked(IDC_SEND_HL7));
			}
		}
	}
}

// (a.vengrofski 2010-05-11 15:34) - PLID <38547>
void CLabEntryDlg::SelChangingHl7List(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CLabEntryDlg::SelChangingHl7List");
}

// (a.vengrofski 2010-05-13 11:15) - PLID <38544> - Send the send the lab via HL7
BOOL CLabEntryDlg::AutoExportTheLab(){
	if (IsDlgButtonChecked(IDC_SEND_HL7)) {//Make sure to only send if they want to
		CString strFormNumber, strSpecimen, strClinicalData, strInstructions, strInitialDiagnosis, strToBeOrdered;
		GetDlgItemText(IDC_FORM_NUMBER, strFormNumber);

		NXDATALIST2Lib::IRowSettingsPtr pLabLinkList = m_pLabLinkList->GetCurSel();
		if (!pLabLinkList) //There is no way this could happen, but hey better safe than sorry.
		{
			MessageBox("You must select a group to send this request to.", "Unselected Lab Link", MB_ICONEXCLAMATION);
			return FALSE;//There is nothing we can do here.
		}else if (VarLong(pLabLinkList->GetValue(lllID), (long)-1) == (long)-1) //no group to send to
		{
			MessageBox("You must select a group to send this request to.", "Unselected Lab Link", MB_ICONEXCLAMATION);
			return FALSE;
		}

		if (!m_bIsNewLab)
		{
			// (c.haag 2010-08-11 12:10) - PLID 39799 - A lab is not actually "Sent" if it has a sent date but we
			// never got an ACK back from the server in a reasonable time.
			//TES 6/22/2011 - PLID 44261 - The ExportType and ExpectACK settings are stored differently now.  Let's pull all the groups that
			// have both settings, and use that array in the query
			CArray<long,long> arHL7GroupsExportTcp;
			GetHL7SettingsGroupsBySetting("ExportType", (long)1, arHL7GroupsExportTcp);
			CArray<long,long> arHL7GroupsExpectACK;
			GetHL7SettingsGroupsBySetting("ExpectACK", TRUE, arHL7GroupsExpectACK);
			CArray<long,long> arHL7GroupsRequireACK;
			for(int i = 0; i < arHL7GroupsExportTcp.GetSize(); i++) {
				bool bMatched = false;
				for(int j = 0; j < arHL7GroupsExpectACK.GetSize() && !bMatched; j++) {
					if(arHL7GroupsExpectACK[j] == arHL7GroupsExportTcp[i]) {
						arHL7GroupsRequireACK.Add(arHL7GroupsExportTcp[i]);
						bMatched = true;
					}
				}
			}
		
			long nFailedACKCheckDelay = GetRemotePropertyInt("HL7Import_FailedACKCheckDelaySeconds", 300);
			// (r.gonet 02/26/2013) - PLID 47534 - Filter out dismissed messages.
			_RecordsetPtr pRs = CreateParamRecordset(
				"SELECT (CASE WHEN HL7MessageLogT.SentDate IS NULL THEN 0 ELSE 1 END) AS BeenSent, "
				"CONVERT(BIT, CASE WHEN (SentDate IS NOT NULL AND DATEDIFF(second, SentDate, GetDate()) > {INT} AND AcknowledgeDate IS NULL AND GroupID IN ({INTARRAY})) THEN 1 ELSE 0 END) AS AckMissing "
				"FROM HL7MessageLogT "
				"WHERE MessageType = {INT} "
				"AND Dismissed = 0 "
				"AND PracticeRecordID IN "
				"(SELECT ID "
				"FROM LabsT "
				"WHERE PatientID = {INT} "
				"AND FormNumberTextID = {STRING}) ",
				nFailedACKCheckDelay, arHL7GroupsRequireACK,
				hemtNewLab, m_nPatientID, strFormNumber); 
			BOOL bBeenSent = FALSE;
			CString strMessage = "";
			if (!pRs->eof)
			{
				int nRsCount = pRs->RecordCount;
				while (!pRs->eof)
				{
					BOOL bAckMissing = AdoFldBool(pRs, "AckMissing") == 0 ? FALSE : TRUE;
					if (!bAckMissing) {
						bBeenSent |= AdoFldLong(pRs, "BeenSent") == 0 ? FALSE : TRUE;//Find out if this lab has been sent at all. A request 					
					} else {
						// (c.haag 2010-08-11 12:10) - PLID 39799 - If we never got an ACK from the server but we were 
						// expecting one, then this hasn't been "sent" as we know it.
					}
					pRs->MoveNext();												   			   //can be batched more than once with being sent.
				}
				if (nRsCount >= 1)//If the request has been sent or batched
				{
					if (bBeenSent)
					{
						strMessage = "This lab request has been sent already.\n\nAre you ABSOLUTELY sure that it should be sent again?";
						if (IDNO == MessageBox(strMessage, "Previously Sent Lab", MB_ICONEXCLAMATION | MB_YESNO))
						{
							return TRUE;
						}
					}
					else
					{
						strMessage = "This lab request is already in a batch waiting to be sent.\n\nAre you ABSOLUTELY sure that it should be added again?";
						if (IDNO == MessageBox(strMessage, "Previously Batched Lab", MB_ICONEXCLAMATION | MB_YESNO))
						{
							return TRUE;
						}
					}
				}
			}
		}
		//TES 6/24/2011 - PLID 44261 - New method for accessing HL7 settings
		long nGroupID = VarLong(pLabLinkList->GetValue(lllID), -1);
		BOOL bAutoExport = GetHL7SettingBit(nGroupID, "AutoExportLabs");
		//get the socket
		// (a.vengrofski 2010-06-10 09:43) - PLID <38544> - Borrowed the below code for the hSocket.
		// (c.haag 2005-12-02 16:07) - PLID 18505 - We now designate the appropriate database
		/*CString strDefaultDatabase = (LPCTSTR)GetRemoteData()->GetDefaultDatabase();
		NxSocketUtils::HCLIENT hSocket = NxSocketUtils::Connect(GetSafeHwnd(), NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"), STANDARD_NXLICENSESERVER_PORT);
		NxSocketUtils::Send(hSocket, PACKET_TYPE_DATABASE_NAME, (const void*)(LPCTSTR)strDefaultDatabase, strDefaultDatabase.GetLength()+1);*/
		//TES 7/15/2010 - PLID 39666 - Instead of doing that, let's just use MainFrame's socket.
		// (c.haag 2010-08-11 13:48) - PLID 40076 - Tom fixed this in his item; but the real problem was it was connecting to the license server port.

		//get the groupID
		long nHL7GroupID = VarLong(pLabLinkList->GetValue(lllID), -1);

		// (r.gonet 12/03/2012) - PLID 54110 - We now pass in the first LabsT.ID rather than the form number text id to the send to HL7 function.
		long nFirstLabID = -1;
		_RecordsetPtr prsLabID = CreateParamRecordset(
			"SELECT TOP 1 LabsT.ID "
			"FROM LabsT "
			"WHERE LabsT.FormNumberTextID = {STRING} AND LabsT.PatientID = {INT} AND LabsT.Deleted = 0 "
			"ORDER BY LabsT.ID ASC ",
			strFormNumber, m_nPatientID);
		if(prsLabID->eof) {
			// (r.gonet 12/03/2012) - PLID 54110 - Odd, it was deleted while we were in it. But we don't lock the lab, so this could happen.
			MessageBox("Practice could not send this lab to HL7. It may have been deleted by another user.", "Error", MB_ICONERROR|MB_OK);
			return TRUE;
		} else {
			nFirstLabID = AdoFldLong(prsLabID, "ID");
		}

		//This will create the HL7 message and attempt to send it.
		// (r.gonet 12/03/2012) - PLID 54110 - Use the refactored send function.
		// (r.gonet 12/11/2012) - PLID 54110 - Have CMainFrame handle any failures.
		GetMainFrame()->GetHL7Client()->SendNewLabHL7Message(nFirstLabID, nHL7GroupID, true);
	}
	return TRUE;
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This function will open this lab on the screen. You should use this instead of DoModal().
// (j.jones 2010-09-01 09:40) - PLID 40094 - added a location ID to be used on new labs that are created
// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
int CLabEntryDlg::OpenLab(long nPatientID, long nProcedureID, LabType ltType, long nInitialLabID, long nOrderSetID, const CString& strToBeOrdered, long nPicID, BOOL bNextLabWillOpenOnClose, BOOL bModal, HWND hwndNotify, long nNewLabLocationID /*= GetCurrentLocationID()*/, long nResultID /*= -1*/)
{
	SetPatientID(nPatientID);
	SetLabProcedureID(nProcedureID);
	if (-1 != nOrderSetID) {
		SetOrderSetID(nOrderSetID);
	}
	SetLabProcedureType(ltType);
	SetInitialLabID(nInitialLabID);
	m_nPIC = nPicID;
	m_hwndNotify = hwndNotify;
	m_bNextLabWillOpenOnClose = bNextLabWillOpenOnClose;

	// (j.jones 2010-09-01 09:43) - PLID 40094 - set the default location ID for new labs
	if(nNewLabLocationID == -1) {
		nNewLabLocationID = GetCurrentLocationID();
	}
	SetDefaultLocationID(nNewLabLocationID);
	// (b.spivey - January 20, 2014) - PLID 46370 - Set the intial result ID
	SetInitialResultID(nResultID); 

	if (bModal) {
		// If we're using this modally, return right after the modal loop finishes
		return DoModal();
	} else {
		// If we're modeless, then create the window and just return IDOK
		Create(IDD, m_pParentWnd); // (a.walling 2012-07-10 14:29) - PLID 46648 - Explicitly use the m_pParentWnd
		ModifyStyle(0, WS_MINIMIZEBOX);
		ShowWindow(SW_SHOW);
		return IDOK;
	}
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This must be called every time a lab window is closed
void CLabEntryDlg::CloseCleanup(int result)
{
	if (NULL != m_hwndNotify) {
		// First, notify our immediate parent that we're closing.
		if (IsWindow(m_hwndNotify)) {
			// This must be done immediately whether or not we're modal because we can't allow the
			// main window to delete this object before this message gets processed.
			::SendMessage(m_hwndNotify, NXM_LAB_ENTRY_DLG_CLOSED, result, (LPARAM)this);
		}
		// Now let the main window know. It is responsible for deleting this object.
		if(m_bNextLabWillOpenOnClose && !this->m_bIsModal){
			//(e.lally 2011-09-20) PLID 45527 - In the case of modeless dialogs expecting to immediately 
			//	open another lab upon close, I changed this to a SendMessage so that the dlg is destroyed right away.
			//	The notification to the hwndNotify above may be posting a message to reopen this same Lab entry dlg
			//	for a different specimen on the same lab so we need to ensure it gets destroyed BEFORE it gets reopened. 
			//	Since this can adversely affect the other uses of this dialog (like modal instances), we want to 
			//	limit the use of the immediate destruction.
			::SendMessage(GetMainFrame()->GetSafeHwnd(), NXM_LAB_ENTRY_DLG_CLOSED, result, (LPARAM)this);
		}
		else {
			//Continue to post the message and the dialog will be destroyed in due time
			::PostMessage(GetMainFrame()->GetSafeHwnd(), NXM_LAB_ENTRY_DLG_CLOSED, result, (LPARAM)this);
		}
	}
	else {
		// if m_hwndNotify is null, it means the function that invoked this dialog is completely responsible
		// for allocating, deleting, and doing any necessary Practice-wide notifications for this lab.
	}
}

// (z.manning 2010-11-16 15:36) - PLID 41499 - Will enable or disable all the buttons that can close this dialog.
void CLabEntryDlg::EnableClosingButtons(BOOL bEnable)
{
	m_cancelBtn.EnableWindow(bEnable);
	m_saveResumePrevBtn.EnableWindow(bEnable);
	m_btnPrint.EnableWindow(bEnable);
	m_btnPreview.EnableWindow(bEnable);
	m_saveAddNewBtn.EnableWindow(bEnable);
	m_btnCreateTodo.EnableWindow(bEnable);
	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if(pSysMenu != NULL)
	{
		UINT nEnable = bEnable ? MF_ENABLED : MF_DISABLED;
		pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND|nEnable);
	}
}

// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if there is at least one result and all the results are completed
// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
// because it may come in handy someday and may even help developers better understand the lab code.
/*BOOL CLabEntryDlg::IsLabCompleted(long nLabID, OUT long& nCompletedBy, OUT COleDateTime& dtCompletedDate)
{
	return m_dlgResults.IsLabCompleted(nLabID, nCompletedBy, dtCompletedDate);
}*/

// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if this lab has any unsigned results
// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
// because it may come in handy someday and may even help developers better understand the lab code.
/*BOOL CLabEntryDlg::LabHasUnsignedResults(long nLabID)
{
	return m_dlgResults.LabHasUnsignedResults(nLabID);
	}*/

// (b.savon 2012-02-28 17:08) - PLID 48443 - Create recall from labdlg
void CLabEntryDlg::OnBnClickedNxbtnCreateRecall()
{
	try{
		// (j.armen 2012-03-28 11:01) - PLID 48480 - We'll use the license later
		if(!g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
			return;
		}
		//(a.wilson 2012-3-23) PLID 48472 - checking whether the current user has read permission.
		BOOL bCreatePerm = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreate));
		BOOL bCreatePermWithPass = (GetCurrentUserPermissions(bioRecallSystem) & (sptCreateWithPass));

		if (!bCreatePerm && !bCreatePermWithPass) {
			PermissionsFailedMessageBox();
			return;
		} else if (!bCreatePerm && bCreatePermWithPass) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}
	
		//Fill in necessary fields to begin a patient recall
		CCreatePatientRecall::PatientRecall prRecall;
		prRecall.nLabID = m_dlgResults.GetInitialLabID();
		prRecall.nPatientID = m_dlgResults.GetPatientID(); 

		// (j.jones 2016-02-22 09:43) - PLID 68348 - pass in a provider and location
		prRecall.nLocationID = m_dlgRequisitions.GetFirstLocationID();
		prRecall.nProviderID = m_dlgRequisitions.GetFirstLabProviderID();

		CCreatePatientRecall prDlg(prRecall, this);
		prDlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

void CLabEntryDlg::OnResultFormCheck()
{
	try {
		//TES 7/27/2012 - PLID 51071 - Enable the dropdown based on the checkbox
		GetDlgItem(IDC_RESULT_FORM_LIST)->EnableWindow(IsDlgButtonChecked(IDC_RESULT_FORM_CHECK));
	}NxCatchAll(__FUNCTION__);
}

void CLabEntryDlg::OnBnClickedEditLabelPrintSettings()
{
	try {
		// Loads the lab label print settings and displays the print dialog
		LabUtils::ShowLabLabelPrintSettings();
	} NxCatchAll(__FUNCTION__);
}

CString CLabEntryDlg::GetAnatomicLocationByLabID(long nLabID) 
{
	CLabRequisitionDlg* pDlg = m_dlgRequisitions.GetLabRequisitionDlgByID(nLabID);
	CString str = "";
	// (b.spivey, September 05, 2013) - PLID 46295 - can be null, so only try to do this if it exists. 
	if (pDlg) {
		str = pDlg->GetAnatomicLocationString();
	}
	return str; 
}