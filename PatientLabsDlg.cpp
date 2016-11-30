// PatientLabsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientLabsDlg.h"
#include "LabEntryDlg.h"
#include "LabFollowUpDlg.h"
#include "DateTimeUtils.h"
#include "GlobalUtils.h"
#include "RenameFileDlg.h"
#include "nxtwain.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "PicContainerDlg.h"
#include "PatientsRc.h"
#include "MultiSelectDlg.h"

#include "LabsSetupDlg.h" // (r.galicki 2008-10-17 11:06) - PLID 31552 - LabType enumeration (is there a better place to put this?)

#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "LabResultsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
// m_bInPICContainer replaced with Set / GetPicContainer so as to avoid GetParent stuff


/////////////////////////////////////////////////////////////////////////////
// CPatientLabsDlg dialog

// (m.hancock 2006-06-21 09:54) - PLID 21070 - Enumeration for the Labs List columns
enum LabsListColumns {
	llRowType = 0,
	llLabID,
	llStepID,
	llStepOrder,
	llDone,
	llIcon,
	llProblemStatus, // (z.manning 2009-05-28 16:26) - PLID 34340 - Added problem status and icon columns
	llProblemIcon,
	llDescription,
	llReceivingLabName,	// (j.jones 2010-06-24 17:02) - PLID 39187 - added Receiving Lab
	llInitialDiagnosis,	// (j.jones 2010-04-05 17:20) - PLID 36736 - added Initial Diagnosis
	llFinalDiagnosis, // (j.jones 2010-04-05 17:20) - PLID 36736 - added Final Diagnosis
	llLabDate, // (z.manning 2009-08-31 16:00) - PLID 27099
	llInputDate,
	llCompletedDate,
	llCompletedBy,
	llResultID,
	llMailID,
	llPath,
	llDiscontinued, // (c.haag 2010-09-09 13:46) - PLID 40461
	llDiscontinuedDate, // (c.haag 2010-09-09 13:46) - PLID 40461
};

// (m.hancock 2006-06-23 10:35) - PLID 21070 - Enumeration for the Labs list row types
enum LabsListRowTypes {
	llrSpacerRow = 0,
	llrLabRow,
	llrStepRow,
	llrDocumentRow,
};

// (r.gonet 09/02/2014) - PLID 63539 - Timer ID for a timer that will coalesce the reloading from rapid fire table checkers.
// When we get a table checker that would ordinarily cause the labs list to reload, we instead set a timer for 100ms. If we
// get another table checker during this time that would cause a lab reload, it is ignored. At the end of the 100ms, the labs 
// list is reloaded. So if multiple table checkers barrage the labs tab dialog within a very short period of time, then only 
// one of those would actually cause a reload.
#define IDT_DELAY_RELOAD_LABS_LIST 1020

CPatientLabsDlg::CPatientLabsDlg(CWnd* pParent)
	: CNxDialog(CPatientLabsDlg::IDD, pParent)
	, m_pPicContainer(NULL)
{
	//{{AFX_DATA_INIT(CPatientLabsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	// (r.galicki 2008-10-07 16:56) - PLID 31555 - PIC flag
	m_nIgnoreTableCheckersRefCnt = 0; // (c.haag 2010-09-15 12:20) - PLID 40512
	m_bGotIgnoredTableChecker = false; // (c.haag 2010-09-15 12:20) - PLID 40512
	// (r.gonet 09/02/2014) - PLID 63539 - Initialize
	m_bCoalescingTableCheckers = false;

	// (a.walling 2010-10-14 17:05) - PLID 40978
	m_id = -1;
}

void CPatientLabsDlg::DoDataExchange(CDataExchange* pDX)
{
	// (c.haag 2010-09-09 13:46) - PLID 40461 - Added checkbox for discontinued labs
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientLabsDlg)
	DDX_Control(pDX, IDC_NEW_LAB, m_newLab);
	DDX_Control(pDX, IDC_NEW_ATTACHMENT, m_newAttachment);
	DDX_Control(pDX, IDC_LABS_NEEDING_ATTENTION, m_labsNeedingAttention);
	DDX_Control(pDX, IDC_DETACH, m_detach);
	DDX_Control(pDX, IDC_DELETE_LAB, m_deleteLab);
	DDX_Control(pDX, IDC_LABS_BKG, m_bkg);
	DDX_Control(pDX, IDC_OPEN_LABS, m_openLabsRad);
	DDX_Control(pDX, IDC_CLOSED_LABS, m_closedLabsRad);
	DDX_Control(pDX, IDC_ALL_LABS, m_allLabsRad);
	DDX_Control(pDX, IDC_CHECK_FILTER_LABS_ON_EMR, m_checkFilterOnPIC);
	DDX_Control(pDX, IDC_RESULT_GRAPH_PREVIEW, m_btnResultGraphPreview);
	DDX_Control(pDX, IDC_SHOW_ALL_RESULTS, m_btnShowAllResults);
	DDX_Control(pDX, IDC_CHECK_DISCONTINUED_LABS, m_checkDiscontinuedLabs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPatientLabsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientLabsDlg)
	ON_BN_CLICKED(IDC_NEW_LAB, OnNewLab)
	ON_BN_CLICKED(IDC_DELETE_LAB, OnDeleteLab)
	ON_BN_CLICKED(IDC_LABS_NEEDING_ATTENTION, OnLabsNeedingAttention)
	ON_BN_CLICKED(IDC_NEW_ATTACHMENT, OnNewAttachment)
	ON_BN_CLICKED(IDC_DETACH, OnDetach)
	ON_BN_CLICKED(IDC_OPEN_LABS, OnOpenLabs)
	ON_BN_CLICKED(IDC_CLOSED_LABS, OnClosedLabs)
	ON_BN_CLICKED(IDC_ALL_LABS, OnAllLabs)
	ON_COMMAND(ID_LABSPOPUP_MARKDONE, OnMarkdone)
	ON_COMMAND(ID_DETACH_DETACHFROMLAB, OnDetachFromLab)
	ON_COMMAND(ID_DETACH_DETACHFROMLABANDHISTORY, OnDetachFromLabAndHistory)
	ON_COMMAND(ID_DETACH_DETACHANDDELETEFILE, OnDetachAndDeleteFile)
	ON_COMMAND(ID_NEWATTACHMENT_ATTACHEXISTINGFILE, OnAttachExistingFile)
	ON_COMMAND(ID_NEWATTACHMENT_ATTACHEXISTINGFOLDER, OnAttachExistingFolder)
	ON_COMMAND(ID_NEWATTACHMENT_IMPORTANDATTACHEXISTINGFILE, OnImportAndAttachExistingFile)
	ON_COMMAND(ID_IMPORTFROMSCANNER_SCANASPDF, OnImportFromScanAsPDF)
	ON_COMMAND(ID_IMPORTFROMSCANNER_SCANASMULTI, OnImportFromScanAsMulti)
	ON_COMMAND(ID_IMPORTFROMSCANNER_SCANASIMAGE, OnImportFromScannerCamera)
	ON_COMMAND(ID_LABSPOPUP_PRINT, OnPrintAttachment)
	ON_COMMAND(ID_LABSPOPUP_OPEN_LAB, OnOpenLab)
	ON_COMMAND(ID_LABSTABPOPUP_DISCONTINUELAB, OnDiscontinueLab)
	ON_BN_CLICKED(IDC_CHECK_FILTER_LABS_ON_EMR, OnCheckFilterOnPIC)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RESULT_GRAPH_PREVIEW, &CPatientLabsDlg::OnBnClickedResultGraphPreview)
	ON_BN_CLICKED(IDC_SHOW_ALL_RESULTS, &CPatientLabsDlg::OnShowAllResults)
	ON_MESSAGE(NXM_LAB_ENTRY_DLG_CLOSED, OnLabEntryDlgClosed)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_MESSAGE(NXM_UPDATEVIEW, OnUpdateView)
	ON_BN_CLICKED(IDC_CHECK_DISCONTINUED_LABS, &CPatientLabsDlg::OnBnClickedCheckDiscontinuedLabs)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientLabsDlg message handlers

void CPatientLabsDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);

	//CPatientDialog::SetColor(nNewColor);
}

// (c.haag 2010-09-15 12:20) - PLID 40512 - Handles an NXM_UPDATEVIEW message. This can get
// posted to this window if a table checker was received during a datalist event.
LRESULT CPatientLabsDlg::OnUpdateView(WPARAM wParam, LPARAM lParam)
{
	try {
		UpdateView();
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

void CPatientLabsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {		
		// (a.walling 2010-10-14 17:07) - PLID 40978
		long nCurrentlyLoadedID = m_id;
		m_id = GetPatientID(); // (a.walling 2010-12-27 15:53) - PLID 40908

		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			ReloadLabList();
		}
		m_ForceRefresh = false;
	} NxCatchAll("Error in CPatientLabsDlg::UpdateView");
}

BOOL CPatientLabsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// (m.hancock 2006-06-20 10:53) - PLID 21070 - Create an interface for showing Labs associated with a patient
	try {
		// (c.haag 2010-09-09 13:46) - PLID 40461 - Bulk caching
		g_propManager.CachePropertiesInBulk("CPatientLabsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"	Name = 'ShowDiscontinuedLabsInPIC' "
			"	OR Name = 'ShowDiscontinuedLabsInLabsTab' "
			"	OR Name = 'LabsTabRadioSelection' " // (z.manning 2011-07-08 15:30) - PLID 42746
			"	OR Name = 'LabsFilterOnPIC' "		// (j.jones 2013-01-30 10:40) - PLID 45871
			")",
			_Q(GetCurrentUserName()));

		//Set the icons on the NxIconButtons
		m_newLab.AutoSet(NXB_NEW);
		m_newAttachment.AutoSet(NXB_NEW);
		m_detach.AutoSet(NXB_DELETE);
		m_deleteLab.AutoSet(NXB_DELETE);
		// (j.gruber 2009-05-07 11:22) - PLID 28556 - create result graph preview button
		m_btnResultGraphPreview.AutoSet(NXB_PRINT_PREV);

		//Initialize datalists
		// (a.walling 2007-11-09 17:09) - PLID 28059 - Bad bind; use BindNxDataList2Ctrl
		m_pLabsList = BindNxDataList2Ctrl(IDC_LABS_LIST, false);

		// (z.manning 2009-05-28 15:58) - PLID 34340 - Problem icons
		m_hIconHasProblemFlag = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16, 16, 0);
		m_hIconHadProblemFlag = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16, 16, 0);

		//Set the display to show open labs for this patient
		// (z.manning 2011-07-08 15:29) - PLID 42746 - We now have a preference for this.
		int nLabOption = GetRemotePropertyInt("LabsTabRadioSelection", lrpOpen, 0, GetCurrentUserName());
		switch (nLabOption) {
		case lrpClosed:
			CheckDlgButton(IDC_CLOSED_LABS, BST_CHECKED);
			break;

		case lrpAll:
			CheckDlgButton(IDC_ALL_LABS, BST_CHECKED);
			break;

		case lrpOpen:
		default:
			CheckDlgButton(IDC_OPEN_LABS, BST_CHECKED);
			break;
		}

		CString strDiscontinuedLabPropName;
		if (GetPicContainer()) {
			CPicContainerDlg* pDlg = GetPicContainer();
			m_bkg.SetColor(pDlg->m_nColor);
			m_checkFilterOnPIC.ShowWindow(SW_SHOW);

			// (j.jones 2013-01-30 10:40) - PLID 45871 - changed the default to unchecked, also
			// remembers the user's last setting
			BOOL bFilterLabsOnPIC = (GetRemotePropertyInt("LabsFilterOnPIC", 0, 0, GetCurrentUserName(), true) == 1);
			m_checkFilterOnPIC.SetCheck(bFilterLabsOnPIC);

			strDiscontinuedLabPropName = "ShowDiscontinuedLabsInPIC";
		} else {
			m_checkFilterOnPIC.ShowWindow(SW_HIDE);
			strDiscontinuedLabPropName = "ShowDiscontinuedLabsInLabsTab";
		}

		// (c.haag 2010-09-09 13:46) - PLID 40461 - Update the checkbox for discontinued labs
		if (GetRemotePropertyInt(strDiscontinuedLabPropName, 0, 0, GetCurrentUserName())) {
			m_checkDiscontinuedLabs.SetCheck(1);
		} else {
			m_checkDiscontinuedLabs.SetCheck(0);
		}

		//Load the lab list
		// (m.hancock 2006-06-26 10:32) - UpdateView handles reloading the Labs list

	} NxCatchAll("Error in CPatientLabsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (m.hancock 2006-06-20 17:38) - PLID 21070 - This function reloads the entire lab list, including the steps.
// (r.gonet 09/02/2014) - PLID 63538 - Refactored so it only makes one trip to the database
void CPatientLabsDlg::ReloadLabList()
{
	try {
		// (r.gonet 09/02/2014) - PLID 63539 - If we were waiting on more table checkers before reloading
		// the labs list, wait no longer. We're doing it now.
		m_bCoalescingTableCheckers = false;
		KillTimer(IDT_DELAY_RELOAD_LABS_LIST);
		m_id = GetActivePatientID();

		// (r.gonet 09/02/2014) - PLID 63538 - In order to not have to load the labs, lab steps, and lab step documents
		// in a recursive manner, we need to store the datalist rows for each record ID so we can 
		// load the labs in one recordset, the steps in a second recordset, and the documents in a third recordset
		// and have them link up into a tree.
		std::map<long, NXDATALIST2Lib::IRowSettingsPtr> mapLabIDToLabRow;
		std::map<long, NXDATALIST2Lib::IRowSettingsPtr> mapStepIDToStepRow;

		//Load the Lab records for this patient, sorted by InputDate, with newest on top.
		// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot in Patient Labs dialog
		// (r.gonet 09/02/2014) - PLID 63538 - Create a recordset of the labs, steps, and documents.
		_RecordsetPtr prsLabs = GetLabRowsRecordset();

		//Clear out the current list
		// (r.gonet 08/27/2014) - PLID 63538 - Clear the list after the data is retrieved because that leads to less flashing.
		m_pLabsList->Clear();

		// (r.gonet 09/02/2014) - PLID 63538 - Loop through each LabsT record and add it to the list
		while (!prsLabs->eof) {
			long nLabID = AdoFldLong(prsLabs->Fields, "ID");
			NXDATALIST2Lib::IRowSettingsPtr pLabRow = AddLabRow(prsLabs->Fields);
			// (r.gonet 09/02/2014) - PLID 63538 - Remember the row we added this record at
			// so we can add its steps to it later when we go through that recordset.
			mapLabIDToLabRow[nLabID] = pLabRow;

			//Move to the next LabsT record
			prsLabs->MoveNext();
		}

		// (r.gonet 09/02/2014) - PLID 63538 - Loop through each LabStepsT record and add it to the list
		// under its respective lab row.
		prsLabs = prsLabs->NextRecordset(NULL);
		while (!prsLabs->eof) {
			// (r.gonet 09/02/2014) - PLID 63538 - Find the lab row we stored before for the parent lab id.
			long nLabID = AdoFldLong(prsLabs->Fields, "LabID");
			std::map<long, NXDATALIST2Lib::IRowSettingsPtr>::iterator it;
			if ((it = mapLabIDToLabRow.find(nLabID)) != mapLabIDToLabRow.end()) {
				long nStepID = AdoFldLong(prsLabs->Fields, "StepID");
				NXDATALIST2Lib::IRowSettingsPtr pLabRow = it->second;
				NXDATALIST2Lib::IRowSettingsPtr pStepRow = AddLabStepRow(prsLabs->Fields, pLabRow);
				mapStepIDToStepRow[nStepID] = pStepRow;

				// (r.gonet 09/02/2014) - PLID 63538 - If the step is not done, then expand it.
				if (pLabRow->GetValue(llDone).vt == VT_BOOL && VarBool(pLabRow->GetValue(llDone)) == FALSE) {
					pLabRow->PutExpanded(VARIANT_TRUE);
				}
			} else {
				// (r.gonet 09/02/2014) - PLID 63538 - We should have a lab row for this step....
				ASSERT(FALSE);
			}

			//Move to the next LabsT record
			prsLabs->MoveNext();
		}

		// (r.gonet 09/02/2014) - PLID 63538 - Loop through each MailSent record attached to lab steps
		// and add it to its respective step row.
		prsLabs = prsLabs->NextRecordset(NULL);
		while (!prsLabs->eof) {
			// (r.gonet 09/02/2014) - PLID 63538 - Find the step row we stored before for the parent step id.
			long nStepID = AdoFldLong(prsLabs->Fields, "LabStepID");
			std::map<long, NXDATALIST2Lib::IRowSettingsPtr>::iterator it;
			if ((it = mapStepIDToStepRow.find(nStepID)) != mapStepIDToStepRow.end()) {
				long nDocumentID = AdoFldLong(prsLabs->Fields, "MailID");
				NXDATALIST2Lib::IRowSettingsPtr pStepRow = it->second;
				AddDocumentRow(prsLabs->Fields, pStepRow);

				// (r.gonet 09/02/2014) - PLID 63538 - Expand the LabStepsT record, which will happen if there are attachments for the step
				pStepRow->PutExpanded(VARIANT_TRUE);
			} else {
				// (r.gonet 09/02/2014) - PLID 63538 - We should have a step row for this document...
				ASSERT(FALSE);
			}

			//Move to the next MailSent record
			prsLabs->MoveNext();
		}

		prsLabs->Close();

		EnableAppropriateButtons();
		m_ForceRefresh = false;

	} NxCatchAll("Error in CPatientLabsDlg::ReloadLabList");

	
	// (c.haag 2010-09-15 12:20) - PLID 40512 - Since the list has been loaded, there's no reason
	// to preserve the fact we got a table checker while we were ignoring them
	m_bGotIgnoredTableChecker = false;
}

// (r.gonet 09/02/2014) - PLID 63538 - Returns the recordset of all the patient's labs, lab steps, and lab step documents.
_RecordsetPtr CPatientLabsDlg::GetLabRowsRecordset()
{
	// (r.gonet 09/02/2014) - PLID 63538 - Constuct the where clause for the lab records.
	CSqlFragment sqlWhereClause = GetLabsListWhereClause();
	// (r.gonet 09/02/2014) - PLID 63538 - Need some temp tables to store the labs and the steps
	// because we'll need to grab the lab associated steps and then the step associated mailsent records.
	CString strTempLabsTableName, strTempLabStepsTableName;
	strTempLabsTableName.Format("#TempLabsT_%s", NewUUID(true));
	strTempLabStepsTableName.Format("#TempLabStepsT_%s", NewUUID(true));

	CParamSqlBatch sqlBatch;
	sqlBatch.Add("DECLARE @PatientID INT SET @PatientID = {INT}; ", GetPatientID());

	// (m.hancock 2006-07-07 15:31) - Changed this so we include the name of the result and display that instead of the value (positive, negative, pending)
	// (j.gruber 2008-09-18 15:48) - PLID 31432 - take out results
	// (j.gruber 2008-10-27 16:10) - PLID 31432 - I decided to put them back in 
	// (z.manning 2008-10-30 14:54) - PLID 31864 - Added type and to be ordered
	// (z.manning 2009-08-31 16:22) - PLID 27099 - Added BiopsyDate
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/8/2009 - PLID 36470 - Restored AnatomySide
	// (j.jones 2010-04-05 17:20) - PLID 36736 - added InitialDiagnosis & FinalDiagnosis
	// (j.jones 2010-06-24 14:19) - PLID 39187 - added Receiving Lab
	// (c.haag 2010-09-09 13:46) - PLID 40461 - Added Discontinued fields
	// (c.haag 2010-12-13 12:10) - PLID 41806 - LabsT no longer has a CompletedDate field. Instead we calculate
	// the completed date based on whether the lab has results that are all completed, and the max completed date
	// of those results.
	// (r.gonet 03/07/2013) - PLID 55492 - Added LOINC code and description.
	// (z.manning 2010-07-27 12:00) - PLID 39857 - Now show multiple flags if applicable
	// (d.singleton 2012-09-19 16:37) - PLID 42596 added order by specimen so it sorts alpha
	// (r.gonet 09/02/2014) - PLID 63538 - Added ReqInputDate to the select list. Also, output into a temp table
	// and select from that.
	// (a.walling 2015-01-05 13:33) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
	sqlBatch.Add(R"(
SET NOCOUNT ON;

SELECT 
	LabsT.ID, 
	LabsT.FormNumberTextID, 
	LabsT.InitialDiagnosis, 
	LabsT.FinalDiagnosis, 
	LabsT.Specimen, 
	LabsT.InputDate, 
	LabsT.AnatomySide, 
	LabsT.LastResultCompletedDate, 
	LabsT.LastResultCompletedBy, 
	LabsT.ToBeOrdered, 
	LabsT.LOINC_Code, 
	LabsT.LOINC_Description, 
	LabsT.Type, 
	LabsT.BiopsyDate, 
	LabsT.ResultName, 
	LabsT.Discontinued, 
	LabsT.DiscontinuedDate, 
	LabsT.AnatomyID, 
	LabsT.AnatomyQualifierID, 
	LabsT.LabLocationID,
	LabAnatomyT.Description AS AnatomicLocation, 
	AnatomyQualifiersT.Name AS LocationQualifier, 
	UsersT.UserName, 
	ReceivingLocationsT.Name AS ReceivingLocationName, 
	( 
		SELECT TOP 1 Resolved FROM EmrProblemsT 
		LEFT JOIN EmrProblemStatusT ON EmrProblemsT.StatusID = EmrProblemStatusT.ID 
		LEFT JOIN EmrProblemLinkT ON EmrProblemsT.ID = EmrProblemLinkT.EmrProblemID AND EmrRegardingID = LabsT.ID 
		WHERE EmrRegardingType = {CONST_INT} AND EmrProblemsT.Deleted = 0 
		ORDER BY COALESCE(Resolved, 2) ASC 
	) AS ProblemStatus,
	LabsInputQ.InputDate AS ReqInputDate
	INTO {CONST_STRING}
FROM (
	SELECT LabsT.ID, 
	LabsT.FormNumberTextID, 
	LabsT.InitialDiagnosis, 
	dbo.GetLabFinalDiagnosisList(LabsT.ID) AS FinalDiagnosis, 
	LabsT.Specimen, 
	LabsT.InputDate, 
	LabsT.AnatomySide, 
	dbo.GetLabCompletedDate(LabsT.ID) AS LastResultCompletedDate, 
	dbo.GetLabCompletedBy(LabsT.ID) AS LastResultCompletedBy, 
	LabsT.ToBeOrdered, 
	LabsT.LOINC_Code, 
	LabsT.LOINC_Description, 
	LabsT.Type, 
	LabsT.BiopsyDate, 
		
	dbo.GetLabResultFlagString(LabsT.ID) AS ResultName, 
	LabsT.Discontinued, 
	LabsT.DiscontinuedDate, 
	LabsT.AnatomyID, 
	LabsT.AnatomyQualifierID, 
	LabsT.LabLocationID 
	FROM LabsT WHERE {SQL} 
) LabsT 
LEFT JOIN ( 
	SELECT LabsT.FormNumberTextID, MAX(LabsT.InputDate) AS InputDate FROM LabsT GROUP BY LabsT.FormNumberTextID 
) LabsInputQ ON LabsT.FormNumberTextID = LabsInputQ.FormNumberTextID 
LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID 
LEFT JOIN AnatomyQualifiersT ON AnatomyQualifiersT.ID = LabsT.AnatomyQualifierID 
LEFT JOIN UsersT ON LabsT.LastResultCompletedBy = UsersT.PersonID 
LEFT JOIN LocationsT ReceivingLocationsT ON LabsT.LabLocationID = ReceivingLocationsT.ID;

SET NOCOUNT OFF;

SELECT * 
FROM {CONST_STRING}
ORDER BY ReqInputDate DESC, FormNumberTextID, Specimen;
)",
(long)eprtLab, strTempLabsTableName, sqlWhereClause, strTempLabsTableName);

	//Find the child rows for this LabsT record from LabStepsT
	// (m.hancock 2006-07-10 09:40) - PLID 21187 - We added StepOrder and Name to LabStepsT so we need to display
	// data from those fields instead of from LabProcedureStepsT
	// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot in Patient Labs dialog
	// (r.gonet 09/02/2014) - PLID 63538 - Combined with the labs query above. Output into a temp table. Select from that.
	// Only get the steps that are associated with the labs we already selected.
	sqlBatch.Add(R"(
SET NOCOUNT ON;

SELECT LabStepsT.StepID, LabStepsT.LabID, LabStepsT.StepOrder, LabStepsT.Name, LabStepsT.StepCompletedDate, LabStepsT.StepCompletedBy, UsersT.UserName 
INTO {CONST_STING}
FROM LabStepsT LEFT JOIN UsersT ON LabStepsT.StepCompletedBy = UsersT.PersonID 
WHERE LabID IN (SELECT LabsT.ID FROM {CONST_STRING} LabsT) 

SET NOCOUNT OFF;

SELECT *
FROM {CONST_STRING}
ORDER BY StepOrder ASC;
)",
strTempLabStepsTableName, strTempLabsTableName, strTempLabStepsTableName);

	// (m.hancock 2006-06-23 10:12) - PLID 21071 - Display attached documents for each step
	//Find the documents that are attached to this step
	// (j.jones 2008-09-05 09:49) - PLID 30288 - supported MailSentNotesT
	// (r.galicki 2008-10-28 12:22) - PLID 31555 - Use GetPatientID instead of GetActivePatientID()
	// (a.walling 2013-12-12 16:51) - PLID 60001 - Snapshot in Patient Labs dialog
	// (r.gonet 09/02/2014) - PLID 63538 - Combined with the labs query above. Only get the documents associated
	// with the lab steps we already found. Also, drop the temp tables.
	sqlBatch.Add(R"(
SELECT MailSent.MailID, LabStepID, Selection, PathName, MailSentNotesT.Note, Date, Sender 
FROM MailSent 
LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID 
WHERE MailSent.PersonID = @PatientID AND MailSent.LabStepID IN (SELECT LabStepsT.StepID FROM {CONST_STRING} LabStepsT)
ORDER BY MailSent.Date

SET NOCOUNT ON;

DROP TABLE {CONST_STRING};
DROP TABLE {CONST_STRING};

SET NOCOUNT OFF;
)",
strTempLabStepsTableName,
strTempLabsTableName, strTempLabStepsTableName);

	return sqlBatch.CreateRecordset(GetRemoteDataSnapshot());
}

// (r.gonet 09/02/2014) - PLID 63538 - Construct the where clause for the lab list.
CSqlFragment CPatientLabsDlg::GetLabsListWhereClause()
{
	//Determine the where clause, which is derived from showing open labs, closed labs, or all labs
	// (c.haag 2010-12-13 12:10) - PLID 41806 - LabsT no longer has a CompletedDate field. We must look to the results.
	// A lab should appear if either of the following is true:
	// 1. The lab has no results.
	// 2. The lab has any results with a NULL completed date.
	CSqlFragment sqlWhere;
	if (IsDlgButtonChecked(IDC_CLOSED_LABS)) { //Closed labs
		// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
		// (r.gonet 09/02/2014) - PLID 63538 - Replaced GetPatientID() with a parameter
		sqlWhere = CSqlFragment("LabsT.PatientID = @PatientID AND LabsT.Deleted = 0 AND (LabsT.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) AND LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL))");
	} else if (IsDlgButtonChecked(IDC_ALL_LABS)) { //All labs
		// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
		// (r.gonet 09/02/2014) - PLID 63538 - Replaced GetPatientID() with a parameter
		sqlWhere = CSqlFragment("LabsT.PatientID = @PatientID AND LabsT.Deleted = 0");
	} else { //Default or Open labs
		// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
		// (r.gonet 09/02/2014) - PLID 63538 - Replaced GetPatientID() with a parameter
		sqlWhere = CSqlFragment("LabsT.PatientID = @PatientID AND LabsT.Deleted = 0 AND (LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LabsT.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL))");
	}
	// (c.haag 2010-09-09 13:46) - PLID 40461 - Apply the discontinued lab filter. Unchecked means we hide discontinued labs.
	if (!m_checkDiscontinuedLabs.GetCheck()) {
		sqlWhere += CSqlFragment(" AND LabsT.Discontinued = 0");
	}
	//TES 8/5/2011 - PLID 44901 - Filter based on permissions
	sqlWhere += CSqlFragment(" AND {SQL} ", GetAllowedLocationClause_Param("LabsT.LocationID"));

	if (GetPicContainer() && m_checkFilterOnPIC.GetCheck()) {
		sqlWhere += CSqlFragment(" AND (PicID = {INT}) ", GetPicID());
	}

	return sqlWhere;
}

// (r.gonet 09/02/2014) - PLID 63538 - Given fields for the lab, adds a lab row and spacer row to the labs list.
NXDATALIST2Lib::IRowSettingsPtr CPatientLabsDlg::AddLabRow(ADODB::FieldsPtr pFields)
{
	//Get information about this LabsT record
	long nLabID = AdoFldLong(pFields, "ID");
	CString strFormNumberTextID = AdoFldString(pFields, "FormNumberTextID", "");
	CString strSpecimen = AdoFldString(pFields, "Specimen", "");
	_variant_t vLabCompletedDate = pFields->Item["LastResultCompletedDate"]->Value;
	bool bLabComplete = vLabCompletedDate.vt != VT_EMPTY && vLabCompletedDate.vt != VT_NULL ? true : false;
	_variant_t varLabDate = pFields->GetItem("BiopsyDate")->GetValue();
	_variant_t varDiscontinued = pFields->GetItem("Discontinued")->GetValue();
	_variant_t varDiscontinuedDate = pFields->GetItem("DiscontinuedDate")->GetValue();

	// (z.manning 2008-10-30 14:55) - PLDI 31864 - Use to be ordered instead of anatomic location
	// for non-biopsy labs
	LabType eType = (LabType)AdoFldByte(pFields, "Type");
	CString strTypeDesc;
	if (eType == ltBiopsy) {
		//Assemble the anatomic location(s)
		CString strAnatomicLocation = AdoFldString(pFields, "AnatomicLocation", "");
		//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
		AnatomySide as = (AnatomySide)AdoFldLong(pFields, "AnatomySide", 0);
		CString strQualifier = AdoFldString(pFields, "LocationQualifier", "");
		strTypeDesc = ::FormatAnatomicLocation(strAnatomicLocation, strQualifier, as);
	} else {
		strTypeDesc = AdoFldString(pFields, "ToBeOrdered", "");
	}

	// (r.gonet 03/07/2013) - PLID 55492 - Output the LOINC code and description as well.
	CString strTestCode = AdoFldString(pFields, "LOINC_Code", "");
	CString strTestDescription = AdoFldString(pFields, "LOINC_Description", "");
	CString strTestCodeAndDescription;
	strTestCodeAndDescription += strTestCode;
	if (!strTestDescription.IsEmpty()) {
		if (!strTestCodeAndDescription.IsEmpty()) {
			strTestCodeAndDescription += " - ";
		}
		strTestCodeAndDescription += strTestDescription;
	}
	if (!strTestCodeAndDescription.IsEmpty()) {
		strTestCodeAndDescription = FormatString("(%s)", strTestCodeAndDescription);
		if (!strTypeDesc.IsEmpty()) {
			strTypeDesc += " ";
		}
		strTypeDesc += strTestCodeAndDescription;
	}

	//Assemble the description for the lab
	CString strDescription = strFormNumberTextID;
	if (!strSpecimen.IsEmpty()) {
		strDescription += " - " + strSpecimen;
	}
	if (!strTypeDesc.IsEmpty()) {
		strDescription += " - " + strTypeDesc;
	}

	//Declare a null variant for later use
	_variant_t varNull;
	varNull.vt = VT_NULL;

	//Add the row for the LabsT record
	// (j.gruber 2008-09-18 15:47) - PLID 31432 = took out result field
	// (j.gruber 2008-10-27 16:25) - PLID 31432 - put it back in again, says <Multiple> if there are multiple
	NXDATALIST2Lib::IRowSettingsPtr pLabsTRow = m_pLabsList->GetNewRow();
	pLabsTRow->PutValue(llRowType, (long)llrLabRow);
	pLabsTRow->PutValue(llLabID, nLabID);
	pLabsTRow->PutValue(llDone, bLabComplete ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
	pLabsTRow->PutValue(llDescription, _bstr_t(strDescription));
	// (j.jones 2010-06-24 17:02) - PLID 39187 - added Receiving Lab
	pLabsTRow->PutValue(llReceivingLabName, pFields->Item["ReceivingLocationName"]->Value);
	// (j.jones 2010-04-05 17:21) - PLID 36736 - added initial diagnosis & final diagnosis
	pLabsTRow->PutValue(llInitialDiagnosis, pFields->Item["InitialDiagnosis"]->Value);
	pLabsTRow->PutValue(llFinalDiagnosis, pFields->Item["FinalDiagnosis"]->Value);
	// (z.manning 2009-08-31 16:02) - PLID 27099 - Added lab date
	pLabsTRow->PutValue(llLabDate, varLabDate);
	pLabsTRow->PutValue(llInputDate, pFields->Item["InputDate"]->Value);
	pLabsTRow->PutValue(llResultID, _variant_t(AdoFldString(pFields, "ResultName", "")));
	pLabsTRow->PutValue(llCompletedDate, vLabCompletedDate);
	pLabsTRow->PutValue(llCompletedBy, _bstr_t(AdoFldString(pFields, "UserName", "")));
	pLabsTRow->PutBackColor(VarBool(varDiscontinued) ? RGB(192, 192, 192) : CalculateLabColor(bLabComplete, false));
	// (z.manning 2009-05-28 16:27) - PLID 34340 - Added problem columns
	_variant_t varProblemStatus = pFields->GetItem("ProblemStatus")->GetValue();
	pLabsTRow->PutValue(llProblemStatus, varProblemStatus);
	if (varProblemStatus.vt == VT_BOOL) {
		if (VarBool(varProblemStatus)) {
			// (z.manning 2009-05-28 16:31) - PLID 34340 - Only has closed problems
			pLabsTRow->PutValue(llProblemIcon, (long)m_hIconHadProblemFlag);
		} else {
			// (z.manning 2009-05-28 16:31) - PLID 34340 - Only has closed problems
			pLabsTRow->PutValue(llProblemIcon, (long)m_hIconHasProblemFlag);
		}
	}
	// (c.haag 2010-09-09 15:33) - PLID 40461 - Discontinued date
	pLabsTRow->PutValue(llDiscontinued, varDiscontinued);
	pLabsTRow->PutValue(llDiscontinuedDate, varDiscontinuedDate);
	m_pLabsList->AddRowAtEnd(pLabsTRow, NULL);

	//Add a spacer row
	NXDATALIST2Lib::IRowSettingsPtr pSpacerRow = m_pLabsList->GetNewRow();
	pSpacerRow->PutValue(llRowType, (long)llrSpacerRow);
	pSpacerRow->PutValue(llLabID, varNull);

	if (pLabsTRow->GetNextRow() == NULL) {
		m_pLabsList->AddRowAtEnd(pSpacerRow, NULL);
	} else {
		m_pLabsList->AddRowBefore(pSpacerRow, pLabsTRow->GetNextRow());
	}

	return pLabsTRow;
}

// (r.gonet 09/02/2014) - PLID 63538 - Given fields for the lab step and the parent lab row, adds a lab step row to the labs list.
NXDATALIST2Lib::IRowSettingsPtr CPatientLabsDlg::AddLabStepRow(ADODB::FieldsPtr pFields, NXDATALIST2Lib::IRowSettingsPtr pParentLabRow)
{
	//Get information about this LabStepsT record
	long nLabID = VarLong(pParentLabRow->GetValue(llLabID));
	long nStepID = AdoFldLong(pFields, "StepID");
	_variant_t vCompletedDate = pFields->Item["StepCompletedDate"]->Value;
	_variant_t varDiscontinued = pParentLabRow->GetValue(llDiscontinued);
	bool bStepComplete = vCompletedDate.vt != VT_EMPTY && vCompletedDate.vt != VT_NULL ? true : false;
	bool bStepExpanded = false;

	//Add the row for the LabStepsT record
	NXDATALIST2Lib::IRowSettingsPtr pLabStepsTRow = m_pLabsList->GetNewRow();
	pLabStepsTRow->PutValue(llRowType, (long)llrStepRow);
	pLabStepsTRow->PutValue(llLabID, nLabID);
	pLabStepsTRow->PutValue(llStepID, nStepID);
	pLabStepsTRow->PutValue(llStepOrder, AdoFldLong(pFields, "StepOrder"));
	pLabStepsTRow->PutValue(llDone, bStepComplete ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
	pLabStepsTRow->PutValue(llDescription, _bstr_t(AdoFldString(pFields, "Name")));
	// (j.jones 2010-06-24 17:02) - PLID 39187 - added Receiving Lab
	pLabStepsTRow->PutValue(llReceivingLabName, g_cvarNull);
	// (j.jones 2010-04-05 17:21) - PLID 36736 - added initial diagnosis & final diagnosis
	pLabStepsTRow->PutValue(llInitialDiagnosis, g_cvarNull);
	pLabStepsTRow->PutValue(llFinalDiagnosis, g_cvarNull);
	pLabStepsTRow->PutValue(llCompletedDate, vCompletedDate);
	//TES 1/5/2011 - PLID 42006 - If the step is complete, and StepCompletedBy is NULL, then it must have been auto-completed
	// by NxServer.
	pLabStepsTRow->PutValue(llCompletedBy, _bstr_t(AdoFldString(pFields, "UserName", bStepComplete ? "NxServer" : "")));
	pLabStepsTRow->PutValue(llLabDate, g_cvarNull); // (z.manning 2009-08-31 16:03) - PLID 27099
	pLabStepsTRow->PutValue(llDiscontinued, varDiscontinued); // (c.haag 2010-09-09 13:46) - PLID 40461
	pLabStepsTRow->PutBackColor(VarBool(varDiscontinued) ? RGB(192, 192, 192) : CalculateLabColor(bStepComplete, true));
	m_pLabsList->AddRowAtEnd(pLabStepsTRow, pParentLabRow);

	return pLabStepsTRow;
}

// (r.gonet 09/02/2014) - PLID 63538 - Given fields for the mailsent record and the parent step row, adds a document row to the labs list.
NXDATALIST2Lib::IRowSettingsPtr CPatientLabsDlg::AddDocumentRow(ADODB::FieldsPtr pFields, NXDATALIST2Lib::IRowSettingsPtr pParentLabStepRow)
{
	//Get information about this MailSent record
	long nLabID = VarLong(pParentLabStepRow->GetValue(llLabID), -1);
	long nStepID = VarLong(pParentLabStepRow->GetValue(llStepID), -1);
	long nMailID = AdoFldLong(pFields, "MailID");
	_variant_t vAttachedDate = pFields->Item["Date"]->Value;
	_variant_t varDiscontinued = pParentLabStepRow->GetValue(llDiscontinued);

	//Add the row for the MailSent record
	NXDATALIST2Lib::IRowSettingsPtr pMailSentRow = m_pLabsList->GetNewRow();
	pMailSentRow->PutValue(llRowType, (long)llrDocumentRow);
	pMailSentRow->PutValue(llLabID, nLabID);
	pMailSentRow->PutValue(llStepID, nStepID);
	pMailSentRow->PutValue(llIcon, _bstr_t(AdoFldString(pFields, "Selection")));
	pMailSentRow->PutValue(llDescription, _bstr_t(AdoFldString(pFields, "Note")));
	// (j.jones 2010-06-24 17:02) - PLID 39187 - added Receiving Lab
	pMailSentRow->PutValue(llReceivingLabName, g_cvarNull);
	// (j.jones 2010-04-05 17:21) - PLID 36736 - added initial diagnosis & final diagnosis
	pMailSentRow->PutValue(llInitialDiagnosis, g_cvarNull);
	pMailSentRow->PutValue(llFinalDiagnosis, g_cvarNull);
	pMailSentRow->PutValue(llCompletedDate, vAttachedDate);
	pMailSentRow->PutValue(llCompletedBy, _bstr_t(AdoFldString(pFields, "Sender", "")));
	pMailSentRow->PutValue(llMailID, nMailID);
	pMailSentRow->PutValue(llPath, _bstr_t(AdoFldString(pFields, "PathName")));
	pMailSentRow->PutValue(llLabDate, g_cvarNull);
	pMailSentRow->PutValue(llDiscontinued, varDiscontinued); // (c.haag 2010-09-09 13:46) - PLID 40461
	m_pLabsList->AddRowAtEnd(pMailSentRow, pParentLabStepRow);

	return pMailSentRow;
}

struct LabProcedure
{
	long nID;
	CString strName;
	LabType ltType; // (r.galicki 2008-10-17 11:04) - PLID 31552 - procedure type
};

void CPatientLabsDlg::OnNewLab() 
{
	try {
		//DRT 7/7/2006 - PLID 21088 - Permissions
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptCreate))
			return;

		// (m.hancock 2006-06-29 13:33) - PLID 21070 - Display a menu containing all of the lab procedures available for creation.
		
		//Get the list of lab procedures
		// (r.galicki 2008-10-17 11:02) - PLID 31522 - included 'Type' in recordset
		_RecordsetPtr prsProcedures = CreateRecordset(
			"SELECT ID, Name, Type FROM LabProceduresT WHERE Inactive = 0 ORDER BY Name");

		//If there are no records in the query, then we cannot create a new lab based 
		//on a specific procedure, so tell the user then return.
		if(prsProcedures->eof) {
			AfxMessageBox("No lab procedures have been defined within the system.\nPlease go to the Administrator module and define a lab procedure.", MB_ICONEXCLAMATION);
			return;
		}
		
		//Assemble an array containing the lab procedures and IDs
		CArray<LabProcedure,LabProcedure&> arProcedures;
		while(!prsProcedures->eof) {
			LabProcedure lp;
			lp.nID = AdoFldLong(prsProcedures, "ID");
			lp.strName = AdoFldString(prsProcedures, "Name", "");
			// (r.galicki 2008-10-17 11:13) - PLID 31552 - set LabProcedure.Type
			lp.ltType = LabType(AdoFldByte(prsProcedures->GetFields(), "Type"));
			arProcedures.Add(lp);
			prsProcedures->MoveNext();
		}

		//Build the menu
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		for(int i = 0; i < arProcedures.GetSize(); i++) {
			//Our command ID will be one higher than our index, because if they click off the menu it returns 0.

			// (r.galicki 2008-10-17 11:18) - PLID 31552 - Identify type in menu options
			CString strMenu = arProcedures[i].strName + " ";
			switch(arProcedures[i].ltType) {
				case ltBiopsy:
					strMenu += "(Biopsy)";
					break;
				case ltLabWork:
					strMenu += "(Lab Work)";
					break;
				case ltDiagnostics:
					strMenu += "(Diagnostics)";
					break;
				case ltCultures:
					strMenu += "(Cultures)";
					break;
			}

			mnu.InsertMenu(nIndex++, MF_BYPOSITION, i+1, strMenu);
			nIndex++;
		}

		//Pop up the menu at the given position
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_NEW_LAB);
		int nResult = 0;
		if (pWnd) 
			pWnd->GetWindowRect(&rc);
		int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON, rc.right, rc.top, this, NULL);

		//Destroy the menu
		mnu.DestroyMenu();

		//Process the menu action
		switch(nCmdID) {
		case 0:
			//They clicked off the menu, do nothing.
			return;
			break;
		default:
			//Look up the ID they selected in our array (remember to shift the index).
			long nProcedureID = arProcedures.GetAt(nCmdID-1).nID;

			// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
			// (a.walling 2012-07-10 14:16) - PLID 46648 - Dialogs must use a parent
			GetMainFrame()->OpenLab(GetPicContainer() ? GetTopLevelFrame() : NULL,
				GetPatientID(), nProcedureID, arProcedures.GetAt(nCmdID-1).ltType, -1, -1, "",
				(GetPicContainer()) ? GetPicID() : -1,
				FALSE,
				(GetPicContainer()) ? TRUE : FALSE, // Open modal in a PIC, or modeless outside the PIC
				GetSafeHwnd());

			break;
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnNewLab");
}

void CPatientLabsDlg::OnDeleteLab() 
{
	try {
		//DRT 7/7/2006 - PLID 21088 - Permissions
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptDelete))
			return;

		// (m.hancock 2006-06-22 17:42) - PLID 21070 - Create an interface for showing Labs associated with a patient
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_pLabsList->CurSel);

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			//Make sure we didn't select the spacer row
			if(VarLong(pRow->GetValue(llRowType)) == llrSpacerRow)
				return;

			//Make sure we selected a Lab row
			if(VarLong(pRow->GetValue(llRowType)) == llrLabRow) {

				//Get required data for deleting the lab
				long nLabID = VarLong(pRow->GetValue(llLabID));

				// (j.jones 2009-06-04 16:03) - PLID 34487 - Find out if any problems will be modified (links removed)
				// or deleted outright. Warn the user accordingly.
				// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
				_RecordsetPtr rsLinkedProblems = CreateParamRecordset("SELECT ID, "
					"CASE WHEN EMRProblemID IN ("
						"SELECT EMRProblemID FROM EMRProblemLinkT WHERE	NOT (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
					") "
					"THEN -1 ELSE EMRProblemID END AS EMRProblemIDToDelete "
					"FROM EMRProblemLinkT WHERE	EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}",
					eprtLab, nLabID,
					eprtLab, nLabID);

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

				CString strWarning = "Are you sure you wish to delete this lab?\nAll attached documents will remain in the patient's history.";

				// (j.jones 2009-06-04 16:40) - PLID 34487 - as long as we're doing something to problems,
				// just give this generic message, we don't really have to get into specifics here
				if(!strEMRProblemIDsToDelete.IsEmpty() || !strEMRProblemLinkIDsToDelete.IsEmpty()) {
					strWarning += "\n\nAll EMR problems linked to this lab will be deleted unless they are also linked to other EMR objects.";
				}

				//Prompt the user
				if(IDYES == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {

					// (z.manning 2008-10-09 17:08) - PLID 31628 - Moved the lab deleting logic to GlobalUtils
					DeletePatientLab(nLabID, strEMRProblemLinkIDsToDelete, strEMRProblemIDsToDelete);

					//Remove the lab row and its spacer row beneath it
					//Get the spacer row
					NXDATALIST2Lib::IRowSettingsPtr pSpacerRow = pRow->GetNextRow();
					if(pSpacerRow != NULL) {
						if(VarLong(pSpacerRow->GetValue(llRowType)) == llrSpacerRow)
						{
							//Remove the lab and spacer rows
							m_pLabsList->RemoveRow(pRow);
							m_pLabsList->RemoveRow(pSpacerRow);
						}	
					}

					EnableAppropriateButtons();
				}
			}
		}

	} NxCatchAll("Error in CPatientLabsDlg::OnDeleteLab");	
}

void CPatientLabsDlg::OnLabsNeedingAttention() 
{
	try {

		// (r.galicki 2008-11-04 15:42) - PLID 31373 - ShowLabFollowUp replaces previous code
		GetMainFrame()->ShowLabFollowUp(TRUE);

		/*// (m.hancock 2006-06-23 09:44) - PLID 21070 - Open the dialog for "Labs Needing Attention", which is the LabFollowUpDlg
		// #pragma TODO("(m.hancock 2006-06-23 09:43) - The Patient Labs tab needs to handle results of the Labs Needing Attention screen")
		
		// (a.walling 2006-07-12 17:14) - PLID 21073 Check HasDataChanged() to see if we should refresh
		if (!GetMainFrame()->m_pLabFollowupDlg) {
			GetMainFrame()->m_pLabFollowupDlg = new CLabFollowUpDlg;
			GetMainFrame()->m_pLabFollowupDlg->Create(IDD_LAB_FOLLOW_UP_DLG, NULL);
		}

		GetMainFrame()->m_pLabFollowupDlg->ShowWindow(SW_SHOW);
		GetMainFrame()->m_pLabFollowupDlg->BringWindowToTop();

		// (a.walling 2008-04-15 10:22) - PLID 25755 - Need tablecheckers to handle this now.
		//if (dlg.HasDataChanged())
		//	ReloadLabList();*/
	} NxCatchAll("Error in CPatientLabsDlg::OnLabsNeedingAttention");
}

void CPatientLabsDlg::OnNewAttachment() 
{
	try {

		//PLID 21088 - We need to check both bioPatientsLabs - sptWrite and bioPatientHistory - sptWrite, but both
		//	can have passwords, so we need to check for that first before just checking.
		//(e.lally 2007-03-28) PLID 24385 - We can bypass this permission logic for administrators, otherwise they
			//get prompted for their password.
		if(!IsCurrentUserAdministrator()){
			BOOL bLabs = (GetCurrentUserPermissions(bioPatientLabs) & sptWrite) > 0 ? TRUE : FALSE;
			BOOL bLabsWithPass = (GetCurrentUserPermissions(bioPatientLabs) & sptWriteWithPass) > 0 ? TRUE : FALSE;
			BOOL bHistory = (GetCurrentUserPermissions(bioPatientHistory) & sptWrite) > 0 ? TRUE : FALSE;
			BOOL bHistoryWithPass = (GetCurrentUserPermissions(bioPatientHistory) & sptWriteWithPass) > 0 ? TRUE : FALSE;

			//if we lack permission to either one, fail
			if( !(bLabs || bLabsWithPass) || !(bHistory || bHistoryWithPass) ) {
				// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
				// messageboxes with PermissionsFailedMessageBox
				PermissionsFailedMessageBox();
				return;
			}

			//if both require a password, prompt now
			if(bLabsWithPass && bHistoryWithPass) {
				if(!CheckCurrentUserPassword())
					return;
			}
			else {
				//Normal case, either 1 requires permission and other does not, or neither do.

				if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
					return;

				if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite))
					return;
			}
		}

		// (m.hancock 2006-06-26 11:34) - PLID 21071 - Allow attachment of documents from the Patient Labs tab
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			
			//Make sure we have a step row selected
			if(VarLong(pRow->GetValue(llRowType)) != llrStepRow)
				return;

			//Display the attach menu
			CMenu mnu;
			mnu.LoadMenu(IDR_LABS_POPUP);
			CMenu* pSubMenu = mnu.GetSubMenu(2);
			if(pSubMenu) {
				pSubMenu->EnableMenuItem(ID_NEWATTACHMENT_ATTACHEXISTINGFILE, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_NEWATTACHMENT_ATTACHEXISTINGFOLDER, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_NEWATTACHMENT_IMPORTANDATTACHEXISTINGFILE, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_IMPORTFROMSCANNER_SCANASPDF, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_IMPORTFROMSCANNER_SCANASMULTI, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_IMPORTFROMSCANNER_SCANASIMAGE, MF_BYCOMMAND|MF_ENABLED);

				//Pop up the menu at the given position
				CRect rc;
				CWnd *pWnd = GetDlgItem(IDC_NEW_ATTACHMENT);
				if (pWnd) {
					pWnd->GetWindowRect(&rc);
					pSubMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, rc.right, rc.top, this, NULL);
				}
			}
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnNewAttachment");
}

void CPatientLabsDlg::OnDetach() 
{
	try {
		//PLID 21088 - We need to check both bioPatientsLabs - sptWrite and bioPatientHistory - sptDelete, but both
		//	can have passwords, so we need to check for that first before just checking.

		//(e.lally 2007-03-28) PLID 24385 - We can bypass this permission logic for administrators, otherwise they
			//get prompted for their password.
		if(!IsCurrentUserAdministrator()){
			BOOL bLabs = (GetCurrentUserPermissions(bioPatientLabs) & sptWrite) > 0 ? TRUE : FALSE;
			BOOL bLabsWithPass = (GetCurrentUserPermissions(bioPatientLabs) & sptWriteWithPass) > 0 ? TRUE : FALSE;
			BOOL bHistory = (GetCurrentUserPermissions(bioPatientHistory) & sptDelete) > 0 ? TRUE : FALSE;
			BOOL bHistoryWithPass = (GetCurrentUserPermissions(bioPatientHistory) & sptDeleteWithPass) > 0 ? TRUE : FALSE;

			//if we lack permission to either one, fail
			if( !(bLabs || bLabsWithPass) || !(bHistory || bHistoryWithPass) ) {
				// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
				// messageboxes with PermissionsFailedMessageBox
				PermissionsFailedMessageBox();
				return;
			}

			//if both require a password, prompt now
			if(bLabsWithPass && bHistoryWithPass) {
				if(!CheckCurrentUserPassword())
					return;
			}
			else {
				//Normal case, either 1 requires permission and other does not, or neither do.

				if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
					return;

				if(!CheckCurrentUserPermissions(bioPatientHistory, sptDelete))
					return;
			}
		}

		// (m.hancock 2006-06-23 13:37) - PLID 21196 - Detach the selected document
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {

			//Make sure we have a document row selected
			if(VarLong(pRow->GetValue(llRowType)) != llrDocumentRow)
				return;

			//Display the detach menu
			CMenu mnu;
			mnu.LoadMenu(IDR_LABS_POPUP);
			CMenu* pSubMenu = mnu.GetSubMenu(1);
			if(pSubMenu) {
				pSubMenu->EnableMenuItem(ID_DETACH_DETACHFROMLAB, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_DETACH_DETACHFROMLABANDHISTORY, MF_BYCOMMAND|MF_ENABLED);
				pSubMenu->EnableMenuItem(ID_DETACH_DETACHANDDELETEFILE, MF_BYCOMMAND|MF_ENABLED);

				//Pop up the menu at the given position
				CRect rc;
				CWnd *pWnd = GetDlgItem(IDC_DETACH);
				if (pWnd) {
					pWnd->GetWindowRect(&rc);
					pSubMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, rc.right, rc.top, this, NULL);
				}
			}
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnDetach");
}

void CPatientLabsDlg::OnOpenLabs() 
{
	try {
		// (z.manning 2011-07-08 15:08) - PLID 42746 - Remember this selection
		SetRemotePropertyInt("LabsTabRadioSelection", lrpOpen, 0, GetCurrentUserName());

		// (m.hancock 2006-06-20 13:33) - PLID 21070 - Filter the listing to show only open labs
		ReloadLabList();
	} NxCatchAll("Error in CPatientLabsDlg::OnOpenLabs");
}

void CPatientLabsDlg::OnClosedLabs() 
{
	try {
		// (z.manning 2011-07-08 15:08) - PLID 42746 - Remember this selection
		SetRemotePropertyInt("LabsTabRadioSelection", lrpClosed, 0, GetCurrentUserName());

		// (m.hancock 2006-06-20 13:33) - PLID 21070 - Filter the listing to show only closed labs
		ReloadLabList();
	} NxCatchAll("Error in CPatientLabsDlg::OnClosedLabs");
}

void CPatientLabsDlg::OnAllLabs() 
{
	try {
		// (z.manning 2011-07-08 15:08) - PLID 42746 - Remember this selection
		SetRemotePropertyInt("LabsTabRadioSelection", lrpAll, 0, GetCurrentUserName());

		// (m.hancock 2006-06-20 13:33) - PLID 21070 - Filter the listing to show all labs
		ReloadLabList();
	} NxCatchAll("Error in CPatientLabsDlg::OnAllLabs");
}

BEGIN_EVENTSINK_MAP(CPatientLabsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPatientLabsDlg)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 10 /* EditingFinished */, OnEditingFinishedLabsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 6 /* RButtonDown */, OnRButtonDownLabsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 19 /* LeftClick */, OnLeftClickLabsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 8 /* EditingStarting */, OnEditingStartingLabsList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 3 /* DblClickCell */, OnDblClickCellLabsList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 2 /* SelChanged */, OnSelChangedLabsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPatientLabsDlg, IDC_LABS_LIST, 9 /* EditingFinishing */, OnEditingFinishingLabsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPatientLabsDlg::OnEditingFinishingLabsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);

		//DRT 7/7/2006 - PLID 21088 - Check permissions in EditingFinishING, do commits in EditingFinishED

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//ignore spacing rows
		if(VarLong(pRow->GetValue(llRowType)) == llrSpacerRow)
			return;

		switch(nCol) {
		case llDone:
			{
				if(VarLong(pRow->GetValue(llRowType)) == llrLabRow) {
					//This is a lab that we are marking finished.  Check the permissions for "mark done"
					if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic0)) {
						*pbContinue = FALSE;
						return;
					}
				}
				else if(VarLong(pRow->GetValue(llRowType)) == llrStepRow) {
					//This is a step.  Check write permission
					if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite)) {
						*pbContinue = FALSE;
						return;
					}
				}
			}
			break;
		}


	} NxCatchAll("Error in OnEditingFinishingLabsList");
}

void CPatientLabsDlg::OnEditingFinishedLabsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);

		//Set the selection to the row that was clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pLabsList->CurSel = pRow;

		//Make sure we didn't select the spacer row
		if(VarLong(pRow->GetValue(llRowType)) == llrSpacerRow)
			return;

		switch(nCol) {
		
		case llDone: 
			{
				// (m.hancock 2006-06-22 09:20) - PLID 21154 - Provide a method for manipulating steps associated with labs.
				//Check if we have clicked on a Lab row
				if(VarLong(pRow->GetValue(llRowType)) == llrLabRow) {
					//If the lab is already marked complete, the user should not be able to mark it incomplete by unchecking
					//the check box on the LabsT record.
					bool bRevert = false;
					if(!VarBool(varOldValue)) {
						//Ask user for result of the lab?

						// (c.haag 2010-01-13) - PLID 41825 - Check for "mark complete" permission (also applies to marking incomplete)
						if(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic0) {
							//Ask the user if they're sure they wish to mark the entire lab complete
							if(IDYES == MessageBox("Are you sure you wish to mark this entire lab complete?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
								//Since we've chosen to mark the entire lab complete, we need to mark each uncompleted step for
								//this lab as complete.
								MarkEntireLabComplete(lpRow);
							}
							else //The user has said no, so revert to the previous value
								bRevert = true;
						}
						else {
							MessageBox("You do not have permission to mark this entire lab complete. Please see your office manager for assistance", "Practice", MB_ICONQUESTION|MB_YESNO);
							bRevert = true;
						}
					}
					else //The Lab was already complete, so check the permissions and mark the lab incomplete
					{
						// (m.hancock 2006-07-19 14:29) - PLID 21516 - Users with permission to mark a lab complete should 
						// also be able to mark the lab's top record as incomplete.
						//Check for "mark complete" permission
						if(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic0) {
							//Prompt the user
							if(IDYES == MessageBox("Are you sure you wish to mark this lab incomplete?\nThe steps will not be affected by this action.", "Practice", MB_ICONQUESTION|MB_YESNO)) {
								// (c.haag 2010-01-19) - PLID 41825 - Revert on failure
								if (!MarkLabIncomplete(lpRow)) {
									bRevert = true;
								}
							}
							else
								bRevert = true;
						}
						else {
							//The user does not have permission to mark a lab complete and thus should not be allowed to 
							//mark the lab incomplete.
							AfxMessageBox("You cannot mark this lab incomplete because you do not have permission to mark "
								"the entire lab complete.\nThe lab will remain complete until a user with "
								"permission to mark the lab complete reviews it.");
							bRevert = true;
						}

						
					}

					if(bRevert) { //Revert to the previous value
						pRow->PutValue(llDone, VarBool(varOldValue) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
						return;
					}
				}

				//The row that was clicked was just a LabStepsT record, so allow the action to be taken.
				else if(VarLong(pRow->GetValue(llRowType)) == llrStepRow) {
					//Determine if the check box is checked
					BOOL bChecked = VarBool(varNewValue) ? TRUE : FALSE;
				
					//If we uncheck a completed step for a completed lab, we need to make sure the user wishes to proceed
					//because it will open the lab again.
					if(!bChecked) {
						//Get the parent row and determine if the lab is open or closed
						NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
						if(VarBool(pParentRow->GetValue(llDone))) {

							//Ask the user if they're sure they wish to reopen the lab
							// (c.haag 2010-01-19) - PLID 41825 - We no longer allow reopening labs based on step completions, so change
							// the wording of the prompt.
							if(IDYES == MessageBox("Are you sure you wish to mark this step incomplete? This will not have any effect on the completion status of the lab itself.", "Practice", MB_ICONQUESTION|MB_YESNO)) {
								// (m.hancock 2006-07-19 14:29) - PLID 21516 - Check the permission for marking complete if the 
								// user is marking this step incomplete and reopening the lab.
								// (c.haag 2010-01-19) - PLID 41825 - We no longer allow reopening labs based on step completions, so
								// don't pass in a third parameter in ModifyStepCompletion (which would have been "true" meaning "reopen 
								// the lab" in the old days), and don't check permissions.
								//if(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic0) {
									//Modify the step
									ModifyStepCompletion(lpRow, FALSE);
								/*}
								else {
									//The user does not have permission to mark a lab complete and thus should not be allowed to 
									//mark the lab incomplete.
									AfxMessageBox("You cannot mark this lab incomplete because you do not have permission to mark "
										"the entire lab complete.\nThe step and lab will remain complete until a user with "
										"permission to mark the lab complete reviews it.");
									//Revert to the previous value
									pRow->PutValue(llDone, VarBool(varOldValue) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
								}*/

								return;
							}
							else { //Revert to the previous value
								pRow->PutValue(llDone, VarBool(varOldValue) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
								return;
							}
						}

						// (m.hancock 2006-07-24 09:11) - PLID 21574 - Prompt the user for marking a lab step incomplete
						if(IDNO == MessageBox("Are you sure you wish to mark this step incomplete?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
							//Revert to the previous value
							pRow->PutValue(llDone, VarBool(varOldValue) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
							return;
						}

						//Update the data
						ModifyStepCompletion(lpRow, FALSE);
					}
					else {	
						//Update the data
						ModifyStepCompletion(lpRow, TRUE);
					}
				}
				break;
			}
		
		case llDescription: 
			{
				// (m.hancock 2006-06-27 11:25) - PLID 21071 - If the description for an attachment is edited, we need to
				// update the MailSent record

				//Check if we have clicked on a Document row
				if(VarLong(pRow->GetValue(llRowType)) == llrDocumentRow) {

					//Get the data required for the update
					long nMailID = VarLong(pRow->GetValue(llMailID));
					CString strNewDescription = VarString(varNewValue, "");

					//Update the MailSent record
					// (j.jones 2008-09-04 13:45) - PLID 30288 - supported MailSentNotesT
					ExecuteSql("UPDATE MailSentNotesT SET Note = '%s' WHERE MailID = %li", _Q(strNewDescription), nMailID);
				}
				break;
			}


		}
	} NxCatchAll("Error in CPatientLabsDlg::OnEditingFinishedLabsList");
}

void CPatientLabsDlg::OnRButtonDownLabsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);

		//Set the selection to the row that was right-clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pLabsList->CurSel = pRow;

		//Make sure we actually have a row selected
		if(pRow != NULL) {

			EnableAppropriateButtons();

			//Make sure we didn't select the spacer row
			if(VarLong(pRow->GetValue(llRowType)) == llrSpacerRow)
				return;

			//Display a popup menu with actions for the user
			CMenu mnu;
			mnu.LoadMenu(IDR_LABS_POPUP);
			CMenu* pSubMenu = NULL; // (b.savon 2013-05-07 14:16) - PLID 53061 - For one, this pointer was never initialized
			bool bDisplayMenu = false;  //Set this to true if we should display the popup menu

			// (m.hancock 2006-06-21 14:33) - PLID 21154 - Provide a method for manipulating steps associated with labs.
			//If we have a Step row, display the menu for marking complete.
			if(VarLong(pRow->GetValue(llRowType)) == llrStepRow) {
				//We're ok to show the menu
				pSubMenu = mnu.GetSubMenu(0);
				if(pSubMenu) {
					// (m.hancock 2006-07-21 17:22) - PLID 21567 - Add right click menu option to open the lab entry dialog
					// when right clicking on a step.
					pSubMenu->EnableMenuItem(ID_LABSPOPUP_OPEN_LAB, MF_BYCOMMAND|MF_ENABLED);
					//Enable marking complete only if the step is incomplete
					if(!VarBool(pRow->GetValue(llDone)))
						pSubMenu->EnableMenuItem(ID_LABSPOPUP_MARKDONE, MF_BYCOMMAND|MF_ENABLED);
					else
						pSubMenu->EnableMenuItem(ID_LABSPOPUP_MARKDONE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					bDisplayMenu = true;  //Make sure we display the menu
				}
			}

			// (m.hancock 2006-07-21 17:47) - PLID 21154 - Add right click menu option to open the lab entry dialog
			// when right clicking on a lab row.
			if(VarLong(pRow->GetValue(llRowType)) == llrLabRow) {
				//We're ok to show the menu
				pSubMenu = mnu.GetSubMenu(0);
				if(pSubMenu) {
					// (m.hancock 2006-07-21 17:22) - PLID 21567 - Add right click menu option to open the lab entry dialog
					// when right clicking on a lab.
					pSubMenu->EnableMenuItem(ID_LABSPOPUP_OPEN_LAB, MF_BYCOMMAND|MF_ENABLED);
					//Do not enable marking complete
					pSubMenu->EnableMenuItem(ID_LABSPOPUP_MARKDONE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					bDisplayMenu = true;  //Make sure we display the menu
				}
			}

			// (m.hancock 2006-06-28 14:00) - PLID 21071 - Allow the user to print the attachment
			//If we have a Document row, then display an entry for printing the document.
			if((VarLong(pRow->GetValue(llRowType)) == llrDocumentRow)) {
				//Only display the menu if we have selected a word document
				if(VarString(pRow->GetValue(llIcon), "") == "BITMAP:MSWORD") {
					//Double check the path name to see if we really have a word document
					CString strPath = VarString(pRow->GetValue(llPath), "");
					strPath.MakeUpper();
					// (a.walling 2007-08-21 07:51) - PLID 26748 - Support Office 2007 files
					// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
					if ( (strPath.Right(4) == ".DOC") || (strPath.Right(5) == ".DOCX") || (strPath.Right(5) == ".DOCM")) {
						//We're ok to show the menu
						pSubMenu = mnu.GetSubMenu(3);
						if(pSubMenu) {
							pSubMenu->EnableMenuItem(ID_LABSPOPUP_PRINT, MF_BYCOMMAND|MF_ENABLED);
							bDisplayMenu = true;  //Make sure we display the menu
						}
					}
				}
			}

			// (c.haag 2010-09-13 11:02) - PLID 40461 - If a lab is discontinued, let the user "continue" it
			if (VarBool(pRow->GetValue(llDiscontinued))) {
				// (b.savon 2013-05-07 14:17) - PLID 53061 - And two, we could get into a state (i.e. the one where this row is a document that isn't a word doc) 
				// so that we would never assign a submenu to our pointer.
				if( pSubMenu ){
					pSubMenu->ModifyMenu(ID_LABSTABPOPUP_DISCONTINUELAB, MF_BYCOMMAND, ID_LABSTABPOPUP_DISCONTINUELAB, "Remove &Discontinued Status");
				}
			} else {
				// (b.savon 2013-05-07 14:17) - PLID 53061 - And two, we could get into a state (i.e. the one where this row is a document that isn't a word doc) 
				// so that we would never assign a submenu to our pointer.  We must first check if we indeed have a valid submenu pointer; if we do, modify it
				if( pSubMenu ){
					pSubMenu->ModifyMenu(ID_LABSTABPOPUP_DISCONTINUELAB, MF_BYCOMMAND, ID_LABSTABPOPUP_DISCONTINUELAB, "&Discontinue Lab");
				}
			}

			//Pop up the menu at the given position
			if(bDisplayMenu) {
				CPoint pt;
				GetCursorPos(&pt);
				pSubMenu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
			}
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnRButtonDownLabsList");
}

void CPatientLabsDlg::OnMarkdone() 
{
	try {

		//This is a step.  Check write permission
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
			return;

		// (m.hancock 2006-06-22 09:20) - PLID 21154 - Provide a method for manipulating steps associated with labs.
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			//Determine if the check box is checked. We need to set the opposite of the current value because this
			//is false when the user wants to mark it true.
			BOOL bChecked = VarBool(pRow->GetValue(llDone)) != FALSE ? FALSE : TRUE;
			//Set the value of the check box
			pRow->PutValue(llDone, bChecked ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
			//Update the data
			ModifyStepCompletion(LPDISPATCH(pRow), bChecked);
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnMarkdone");
}

//TES 2/10/2010 - PLID 37296 - Moved GenerateStepCompletionAuditOld() to GlobalUtils.cpp

// (c.haag 2010-01-19) - PLID 41825 - We no longer allow reopening labs based on step completions
void CPatientLabsDlg::ModifyStepCompletion(LPDISPATCH lpRow, BOOL bComplete)//, bool bReopenLab /* = false*/)
{
	try {
		// (m.hancock 2006-06-22 09:43) - PLID 21154 - Provide a method for manipulating steps associated with labs.
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//Make sure we actually have a row selected
		if(pRow != NULL) {
				
			//Get required data for marking the step complete
			long nLabID = VarLong(pRow->GetValue(llLabID));
			long nStepID = VarLong(pRow->GetValue(llStepID));
			COleDateTime dtMarkedDone = COleDateTime::GetCurrentTime();
			CString strUpdateSteps; //Stores the list of steps that will be updated
			strUpdateSteps.Format("%li", nStepID);
			CString strUpdateLabs; //Stores the list of labs that will be updated by MarkEntireLabComplete
			strUpdateLabs.Format("%li", nLabID);
			bool bUpdateMultiple = false;

			//Declare a null variant for later use
			_variant_t varNull;
			varNull.vt = VT_NULL;

			//TES 6/23/2014 - PLID 60708 - Track all the labs that get modified
			CArray<long, long> arUpdatedLabIDs;
			arUpdatedLabIDs.Add(nLabID);

			//Check if there are other steps for specimens having this same form number
			if(bComplete) {
				// (c.haag 2013-08-29) - PLID 58216 - We now invoke this from a utility function
				_RecordsetPtr prsLinkedSteps = GetCompletableLinkedSteps(nStepID, false);

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

					if(IDYES == MessageBox(strMessage, "Practice", MB_ICONQUESTION|MB_YESNO)) {
						bUpdateMultiple = true;
						//Go back to the first record in the query
						prsLinkedSteps->MoveFirst();
						//Loop through the query results and add the step IDs to the list to be updated
						while(!prsLinkedSteps->eof) {
							CString strCurStepID;
							CString strCurLabID;
							strCurStepID.Format(", %li", AdoFldLong(prsLinkedSteps, "StepID"));
							strCurLabID.Format(", %li", AdoFldLong(prsLinkedSteps, "LabID"));
							//TES 6/23/2014 - PLID 60708 - Remember that this lab was modified
							arUpdatedLabIDs.Add(AdoFldLong(prsLinkedSteps, "LabID"));
							strUpdateSteps += strCurStepID;
							strUpdateLabs += strCurLabID;
							//Move to the next record
							prsLinkedSteps->MoveNext();
						}
					}
				}
			}
			
			//Update the data

			//First, check if we need to reopen the lab
			// (c.haag 2010-01-19) - PLID 41825 - Modifying a step's completion no longer has any kind of bearing
			// on the official completion status of the lab itself. A lab is only completed if all of its results are completed.
			/*if(bReopenLab == true) {
				if(bUpdateMultiple)
					ReopenLab(NULL, strUpdateLabs);
				else
					ReopenLab(pRow->GetParentRow());
			}*/

			//Second, update the steps
			BOOL bCreateLadder = FALSE;
			long nNextStepID = -1;
			if(bComplete) { //Complete the step
				// (z.manning 2008-10-21 11:08) - PLID 31371 - Also check for the next incomplete step.
				//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
				_RecordsetPtr prsUpdate = CreateRecordset(
					"SET NOCOUNT ON "
					"UPDATE LabStepsT SET StepCompletedDate = '%s', StepCompletedBy = %li WHERE StepID IN (%s)"
					"SET NOCOUNT OFF "
					"SELECT TOP 1 LabStepsT.StepID, CreateLadder FROM LabStepsT "
					"LEFT JOIN LabProcedureStepsT ON LabStepsT.LabProcedureStepID = LabProcedureStepsT.StepID "
					"WHERE StepCompletedDate IS NULL AND LabID = %li "
					"ORDER BY LabStepsT.StepOrder ASC "
					, FormatDateTimeForSql(dtMarkedDone), GetCurrentUserID(), strUpdateSteps, nLabID);

				// (z.manning 2008-10-21 11:10) - PLID 31371 - Does the next step want us to create a tracking ladder?
				if(!prsUpdate->eof) {
					if(AdoFldBool(prsUpdate->GetFields(), "CreateLadder", FALSE)) {
						bCreateLadder = TRUE;
						nNextStepID = AdoFldLong(prsUpdate->GetFields(), "StepID");
					}
				}
			}
			else { //Mark the step not complete
				ExecuteSql("UPDATE LabStepsT SET StepCompletedDate = NULL, StepCompletedBy = NULL WHERE StepID IN (%s)",
					strUpdateSteps);
			}

			// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a LabsT-level table checker because we updated lab steps.
			// I think they can span more than one lab.
			// (r.gonet 08/29/2014) - PLID 63540 - Don't send the table checker to ourself since we are handling the UI update here.
			m_labsTableRefresher.RefreshLabsTable(GetPatientID(), -1, TRUE);

			// (z.manning 2008-10-13 16:37) - PLID 31667 - Update todos for this lab
			//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
			//TES 6/23/2014 - PLID 60708 - Update all the labs that were modified
			foreach(long nUpdatedLabID, arUpdatedLabIDs) {
				SyncTodoWithLab(nUpdatedLabID, GetPatientID());
			}

			//Auditing
			// (m.hancock 2006-07-24 09:40) - PLID 21582 - Make sure that modifying steps is audited
			{
				//Need to loop through all IDs in our list, comma delimited.  Must be at least 1.
				long nAuditID = BeginNewAuditEvent();

				CString str = strUpdateSteps;		//work with a copy
				long nComma = str.Find(",");
				while(nComma > -1) {
					CString strCur = str.Left(nComma);
					str = str.Mid(nComma+1);

					long nRecordID = atoi(strCur);
					// (m.hancock 2006-07-24 09:53) - PLID 21582 - Generate a string containing the lab's description and the step name
					CString strOld = GenerateStepCompletionAuditOld(nRecordID);
					//Generate the new value string
					CString strNew = bComplete ? "<Completed>" : "<Incomplete>";
					// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
					AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientLabStepMarkedComplete, nRecordID, strOld, strNew, aepMedium, aetChanged);
					nComma = str.Find(",");
				}

				//and there should be 1 more record afterwards
				long nRecordID = atoi(str);
				// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
				AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientLabStepMarkedComplete, nRecordID, GenerateStepCompletionAuditOld(nRecordID), bComplete ? "<Completed>" : "<Incomplete>", aepMedium, aetChanged);
			}

			//TES 3/16/2009 - PLID 32678 - Need to track whether we complete a step; if so, then we know that 
			// MarkEntireLabComplete() has been called if necessary, so we shouldn't try to call it again.
			// (c.haag 2010-01-21) - PLID 41825 - I didn't change any code here, but just FYI, we no longer change
			// a lab's completed status by changing step completions. They are now two completely separate things.
			bool bStepCompleted = false;
			if(bCreateLadder) {
				// (z.manning 2008-10-21 10:55) - PLID 31371 - The next step is set to create a ladder
				// so prompt them to do that now.
				// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
				if(PhaseTracking::PromptToAddPtnProcedures(GetPatientID())) {
					// (z.manning 2008-10-21 10:55) - PLID 31371 - They created the tracking ladder, so
					// let's mark the next step complete.
					NXDATALIST2Lib::IRowSettingsPtr pNextRow = m_pLabsList->FindByColumn(llStepID, nNextStepID, NULL, VARIANT_FALSE);
					if(pNextRow != NULL) {
						ModifyStepCompletion(pNextRow, TRUE);
						pNextRow->PutValue(llDone, g_cvarTrue);
						//TES 3/16/2009 - PLID 32678 - We have marked a step as Done.
						bStepCompleted = true;
					}
					else {
						// (z.manning 2008-10-21 11:18) - PLID 31371 - This lab was clearly visible when
						// they marked the original step complete, so not sure why we wouldn't be
						// able to find the row we're looking for. However, if we do get here, no need to
						// throw an exception as the only thing that went wrong is having the step marked
						// complete (the ladder was still created properly).
						ASSERT(FALSE);
					}
				}
			}

			//If we updated multiple records, just reload the list
			if(bUpdateMultiple)
				ReloadLabList();
			//Otherwise, only update the single row in the data list
			else {
				//Update the row in the Labs list
				pRow->PutValue(llCompletedDate, bComplete ? _variant_t(dtMarkedDone, VT_DATE) : varNull);
				pRow->PutValue(llCompletedBy, _bstr_t( bComplete ? GetCurrentUserName() : ""));
				//If we reopened a lab, we need to update the lab's row, too
				// (c.haag 2010-01-19) - PLID 41825 - We no longer allow reopening labs on account of modifying
				// lab ladder steps.
				/*if(bReopenLab == true) {

					//If we are currently viewing closed labs, we should remove this lab because it is open now
					if(IsDlgButtonChecked(IDC_CLOSED_LABS)) {
						//Remove the lab row and its spacer row beneath it
						//Get the lab row
						NXDATALIST2Lib::IRowSettingsPtr pLabRow = pRow->GetParentRow();
						//Get the spacer row
						NXDATALIST2Lib::IRowSettingsPtr pSpacerRow = pLabRow->GetNextRow();
						if(pSpacerRow != NULL) {
							if(VarLong(pSpacerRow->GetValue(llRowType)) == llrSpacerRow)
							{
								//Remove the lab and spacer rows
								m_pLabsList->RemoveRow(pLabRow);
								m_pLabsList->RemoveRow(pSpacerRow);
							}	
						}
					}

					else { //Open labs or All labs

						//Update the rows
						NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();
						pParentRow->PutValue(llCompletedDate, bComplete ? _variant_t(dtMarkedDone, VT_DATE) : varNull);
						pParentRow->PutValue(llCompletedBy, _bstr_t( bComplete ? GetCurrentUserName() : ""));
						pParentRow->PutValue(llDone, bComplete ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));
						// (c.haag 2010-09-15 12:58) - PLID 40461 - Stay grayed out if discontinued
						pParentRow->PutBackColor(VarBool(pParentRow->GetValue(llDiscontinued),FALSE) ? RGB(192, 192, 192) : CalculateLabColor(bComplete ? true : false, false));
					}
				}*/
			}

			//If the lab is complete, we need to close the lab
			//TES 3/16/2009 - PLID 32678 - We do NOT want to call MarkEntireLabComplete() if we've already marked a different
			// step complete; if we did, then the act of marking that step complete will have already caused 
			// MarkEntireLabComplete() to have been called, and calling it twice can lead to duplicate prompts or even
			// exceptions.
			// (c.haag 2010-01-21) - PLID 41825 - We no longer allow users to complete a lab by marking a step complete. They will have to check the topmost row
			// in the ladder to accomplish that, and then manually review the names of each result.
			/*if(IsLabComplete(nLabID) && !bStepCompleted) {
				//DRT 7/10/2006 - PLID 21088 - Permissions!  Write permission allows them to modify steps, but they also
				//	require "Mark Complete" permission to finish the whole thing.  This may be used because a receptionist gets feedback, 
				//	but the doctor wants to ensure that only him/herself are the ones who "finish" the lab.
				if(!(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic0)) {
					//yep, no permission to mark complete.  Let them know what's up
					AfxMessageBox("You have completed the last step in this lab, however you do not have permission to mark the entire lab complete.  The lab "
						"will remain incomplete until an administrator reviews it.");
				}
				else {
					//If we updated multiple steps, we'll need to be updating multiple labs
					if(bUpdateMultiple) {
						MarkEntireLabComplete(NULL, strUpdateLabs);
					}
					else {
						//We only updated a single lab
						MarkEntireLabComplete(pRow->GetParentRow());
					}
				}
			}*/

			// (m.hancock 2006-06-22 16:28) - PLID 21100 - Apply proper color scheme
			// (c.haag 2010-09-15 12:58) - PLID 40461 - Stay grayed out if discontinued
			pRow->PutBackColor(VarBool(pRow->GetValue(llDiscontinued),FALSE) ? RGB(192, 192, 192) : CalculateLabColor(bComplete ? true : false, true));
		}
	} NxCatchAll("Error in CPatientLabsDlg::ModifyStepCompletion");
}

//For mark complete audits, we have to give some descriptive info on the "old" value.
CString GenerateCompletionAuditOld(long nLabID)
{
	CString strOld;

	// (z.manning 2008-10-30 15:19) - PLID 31864 - Use to be ordered instead of anatomic location for
	// non-biopsy labs.
	//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
	//TES 12/8/2009 - PLID 36470 - Restored AnatomySide
	//TES 12/8/2009 - PLID 36513 - Fixed bug where a NULL Specimen would result in a blank string.
	// (z.manning 2010-04-30 15:42) - PLID 37553 - Now have a view to pull anatomic location
	_RecordsetPtr prsData = CreateRecordset(
		"SELECT COALESCE(FormNumberTextID,'') + ' - ' + COALESCE(Specimen,'') + ' - ' + \r\n"
		"	CASE WHEN LabsT.Type = %i THEN \r\n"
		"		LabAnatomicLocationQ.AnatomicLocation \r\n"
		"		ELSE LabsT.ToBeOrdered END AS Description \r\n"
		"FROM LabsT \r\n"
		"LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID \r\n"
		"WHERE LabsT.ID = %li \r\n"
		, ltBiopsy, nLabID);
	if(!prsData->eof) {
		strOld = AdoFldString(prsData, "Description", "");
	}
	else {
		//shouldnt be possible
		ASSERT(FALSE);
		strOld.Format("<Incomplete Lab %li>", nLabID);
	}

	return strOld;
}

void CPatientLabsDlg::MarkEntireLabComplete(LPDISPATCH lpLabRow, LPCTSTR strLabIDs /* = NULL*/)
{
	try {
		// (m.hancock 2006-06-22 10:46) - PLID 21154 - Provide a method for manipulating steps associated with labs.
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpLabRow);
		CString strLabIDList;

		//If a list of LabIDs was passed via strLabIDs, we need to update multiple records
		if(strLabIDs)
			strLabIDList = strLabIDs;
		//Since we're not using a list, try to get the LabID from the row
		else if(pRow != NULL) {
			strLabIDList.Format("%li", VarLong(pRow->GetValue(llLabID)));
		}
		//No list, no valid row, means we need to throw an exception
		else {
			AfxThrowNxException("CPatientLabsDlg::MarkEntireLabComplete: Could not retrieve LabID");
		}

		//Make sure we actually have a valid LabID
		if(!strLabIDList.IsEmpty()) {

			//Get required data for marking the lab complete
			COleDateTime dtMarkedDone = COleDateTime::GetCurrentTime();

			// (m.hancock 2006-07-24 11:38) - PLID 21582 - Find the steps that are to be audited
			//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
			// (c.haag 2010-01-21) - PLID 41825 - Don't change lab steps, nor todo alarms since they only care
			// about steps.
			/*
			_RecordsetPtr prsSteps = CreateRecordset("SELECT StepID FROM LabStepsT WHERE LabID IN (%s) "
				"AND StepCompletedDate IS NULL", strLabIDList);

			//Update the LabStepsT data.  Since we've chosen to mark the entire lab complete, we need to mark each
			//incomplete step for this lab as complete.
			//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
			ExecuteSql("UPDATE LabStepsT SET StepCompletedDate = '%s', StepCompletedBy = %li "
				"WHERE LabID IN (%s) AND StepCompletedDate IS NULL", 
				FormatDateTimeForSql(dtMarkedDone), GetCurrentUserID(), strLabIDList);
			
			// (z.manning 2008-10-13 16:37) - PLID 31667 - Update todos for this lab
			// (z.manning 2008-11-07 15:54) - PLID 31667 - We may have more than one*/
			CArray<long,long> arynLabIDs;
			ParseDelimitedStringToLongArray(strLabIDList, ",", arynLabIDs);
			/*for(int nLab = 0; nLab < arynLabIDs.GetSize(); nLab++) {
				//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
				SyncTodoWithLab(arynLabIDs.GetAt(nLab), GetPatientID());
			}

			// (m.hancock 2006-07-24 11:38) - PLID 21582 - Make sure that modifying steps is audited
			{
				// (m.hancock 2006-07-24 11:18) - PLID 21582 - Audit each step that was marked complete
				while(!prsSteps->eof)
				{
					long nAuditID = BeginNewAuditEvent();
					//Get the StepID
					long nStepID = AdoFldLong(prsSteps, "StepID");
					//Generate a string containing the lab's description and the step name
					CString strOld = GenerateStepCompletionAuditOld(nStepID);
					// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
					AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientLabStepMarkedComplete, nStepID, strOld, "<Completed>", aepMedium, aetChanged);
					prsSteps->MoveNext();
				}
			}
			*/

			// (z.manning 2008-10-31 10:00) - PLID 21082 - We now require a signature when completing labs
			// so we need to go through them one at a time and prompt for a signature.
			BOOL bCompletedSingleLab = TRUE;
			for(int nCompletedLabIndex = 0; nCompletedLabIndex < arynLabIDs.GetSize(); nCompletedLabIndex++) {
				long nCompletedLabID = arynLabIDs.GetAt(nCompletedLabIndex);
				bCompletedSingleLab = PromptToSignAndMarkLabComplete(this, nCompletedLabID);
			}

			if (bCompletedSingleLab) {
				// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a LabsT-level table checker because we updated lab steps.
				// (r.gonet 09/03/2014) - PLID 63540 - Send the patient ID too. Also don't send to ourself.
				m_labsTableRefresher.RefreshLabsTable(GetPatientID(), -1, TRUE);
			}
			// (r.gonet 09/03/2014) - PLID 63540 - Just reload ourself immediately.The old code that manually updated a record if it was just a single lab
			// being completed resulted in a table checker reloading the list anyway.... Also it had bugs.
			ReloadLabList();
		}
	} NxCatchAll("Error in CPatientLabsDlg::MarkEntireLabComplete");
}

// (c.haag 2010-01-19) - PLID 41825 - This function now "reopens" the lab by having the user select individual,
// completed results to mark incomplete. Returns FALSE if the user cancelled the operation. Returns TRUE if they
// did not, which means they must have marked at least one result as incomplete. We also no longer accept an
// array of labs; this only operates one lab at a time.
BOOL CPatientLabsDlg::ReopenLab(LPDISPATCH lpLabRow)
{
	try {
		// (m.hancock 2006-06-22 14:57) - PLID 21154 - Provide a method for manipulating steps associated with labs.
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpLabRow);
		long nLabID;

		//If a list of LabIDs was passed via strLabIDs, we need to update multiple records
		// (c.haag 2010-01-19) - PLID 41825 - We no longer take in a list of labs
		/*if(strLabIDs)
			strLabIDList = strLabIDs;
		//Since we're not using a list, try to get the LabID from the row*/
		if(pRow != NULL) {
			nLabID = VarLong(pRow->GetValue(llLabID));
		}
		//No list, no valid row, means we need to throw an exception
		else {
			AfxThrowNxException("CPatientLabsDlg::MarkEntireLabComplete: Could not retrieve LabID");
		}

		//Update the LabsT data.
		// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
		// (z.manning 2008-10-31 10:29) - PLID 21082 - Need to also clear out the signature.
		// (j.jones 2010-04-12 17:34) - PLID 38166 - then also clear out the text ink data
		// (c.haag 2010-12-15 17:27) - PLID 41825 - We now separate completion from signing, and we now
		// do it at the result level. So, only reset the result completed info but do not touch the signatures
		// any longer. Furthermore, we must have the user decide which results to mark incomplete. 
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "LabResultsT");
		dlg.m_bPreSelectAll = FALSE;
		if (IDCANCEL == dlg.Open("LabResultsT", FormatString("Deleted = 0 AND LabID IN (SELECT ID FROM LabsT WHERE Deleted = 0 AND ID IN (%d))", nLabID), "ResultID", "Name", "Please select one or more lab results to mark as incomplete.", 1))
		{
			return FALSE;
		}

		CString strResultIDs = dlg.GetMultiSelectIDString();

		// =========================== Do auditing here =============================
		// (z.manning 2008-10-30 15:19) - PLID 31864 - Use to be ordered instead of anatomic location for
		// non-biopsy labs.
		//TES 11/10/2009 - PLID 36128 - Replaced AnatomySide with AnatomyQualifierID
		//TES 12/8/2009 - PLID 36470 - Restored AnatomySide
		//TES 12/8/2009 - PLID 36513 - Fixed bug where a NULL Specimen would result in a blank string.
		// (z.manning 2010-04-30 15:42) - PLID 37553 - Now have a view to pull anatomic location
		// (c.haag 2010-12-15 17:27) - PLID 41825 - We now audit results, not labs.
		CAuditTransaction at;
		_RecordsetPtr prsData = CreateParamRecordset(
			"SELECT "
			"LabResultsT.ResultID, "
			"COALESCE(FormNumberTextID,'') + ' - ' + COALESCE(Specimen,'') + ' - ' + COALESCE(LabResultsT.Name,'') AS AuditName "
			"FROM LabsT \r\n"
			"INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
			"LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID \r\n"
			"WHERE LabsT.Deleted = 0 AND LabResultsT.Deleted = 0 AND LabResultsT.ResultID IN ({INTSTRING}) \r\n"
			,strResultIDs);
		if (!prsData->eof) 
		{
			// Audit result completions
			while (!prsData->eof) {
				const long nResultID = AdoFldLong(prsData, "ResultID");
				const CString strOld = AdoFldString(prsData, "AuditName", "");
				AuditEvent(GetPatientID(), GetPatientName(), at, aeiPatientLabResultMarkedComplete, nResultID, strOld, "<Incomplete>", aepMedium, aetChanged);				
				prsData->MoveNext();
			}
		}
		else {
			ThrowNxException("Attempted to reopen a lab but the lab or results are unavailable!");
		}

		// Now actually change the results
		ExecuteParamSql("UPDATE LabResultsT SET ResultCompletedDate = NULL, ResultCompletedBy = NULL \r\n"
			"WHERE ResultID IN ({INTSTRING}) AND LabID IN (SELECT ID FROM LabsT WHERE PatientID = {INT}) ", 
			strResultIDs, GetPatientID());

		// Commit the audit
		at.Commit();

		// (c.haag 2010-07-19 16:04) - PLID 30894 - Fire a LabsT table checker since we modified LabsT
		// (r.gonet 09/02/2014) - PLID 63221 - Send the patient as well
		CClient::RefreshLabsTable(GetPatientID(), nLabID);

		return TRUE;

	} NxCatchAll("Error in CPatientLabsDlg::ReopenLab");
	return FALSE;
}

bool CPatientLabsDlg::IsLabComplete(long nLabID)
{
	try {
		// (m.hancock 2006-06-22 15:15) - PLID 21154 - Provide a method for manipulating steps associated with labs.
		//Check the number of remaining open steps for the provided lab number
		//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
		long nOpenSteps = -1;
		_RecordsetPtr prsNum = CreateRecordset(
			"SELECT Count(*) AS Num FROM LabStepsT "
			"WHERE LabID = %li AND StepCompletedDate IS NULL", 
			nLabID);
		if(!prsNum->eof) {
			nOpenSteps = AdoFldLong(prsNum, "Num");
		}

		//If the number of open steps is zero, then the lab is complete.
		if(nOpenSteps == 0)
			return true;

	} NxCatchAll("Error in CPatientLabsDlg::IsLabComplete");

	//If we have no open steps or the query failed, then the lab is not complete, so return false.
	return false;
}

COLORREF CPatientLabsDlg::CalculateLabColor(bool bComplete, bool bStepRow)
{
	// (m.hancock 2006-06-22 15:48) - PLID 21100 - Color code the rows for the labs list
	//Default colors:	
	//A complete lab should be Green, and an incomplete lab should be Yellow.
	//A complete step should be Green, and an incomplete step should be Red.
	COLORREF cComplete;
	COLORREF cLabIncomplete;
	COLORREF cStepIncomplete;

	try {
		// (c.haag 2006-06-27 17:32) - PLID 21185 - We now use preferences to define the colors
		cComplete = GetRemotePropertyInt("LabColorComplete", RGB(192, 255, 192), 0, GetCurrentUserName(), true);
		cLabIncomplete = GetRemotePropertyInt("LabColorLabIncomplete", RGB(255, 253, 170), 0, GetCurrentUserName(), true);
		cStepIncomplete = GetRemotePropertyInt("LabColorStepIncomplete", RGB(246, 215, 215), 0, GetCurrentUserName(), true);

		//Determine if the row is complete and return the completed color if it is
		if(bComplete)
			return cComplete; //The completed row color
		//The row is incomplete, so determine if we should return the step incomplete color or the lab incomplete color
		else if (bStepRow)
			return cStepIncomplete; //The incomplete step color

	} NxCatchAll("Error in CPatientLabsDlg::CalculateLabColor");

	//If we haven't returned at this point, the lab is incomplete.
	return cLabIncomplete; //The lab incomplete color
}


void CPatientLabsDlg::OnLeftClickLabsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);

		//Set the selection to the row that was clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pLabsList->CurSel = pRow;

		EnableAppropriateButtons();

		//Make sure we didn't select the spacer row
		if(VarLong(pRow->GetValue(llRowType)) == llrSpacerRow)
			return;

		switch(nCol) {
		
		case llIcon: 
			{
				// (m.hancock 2006-06-23 11:16) - PLID 21071 - Open the document if they clicked on the icon
				// We only want to allow this if the current row is a document
				if(VarLong(pRow->GetValue(llRowType)) != llrDocumentRow)
					return;
				// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
				OpenDocument(VarString(pRow->GetValue(llPath)), GetPatientID());
				break;
			}
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnLeftClickLabsList");	
}


// (m.hancock 2006-06-23 15:24) - PLID 21196 - set bDetachHistory to true
// to also detach from the patient's history, and set bDelete to true to also delete the file.
void CPatientLabsDlg::DetachFile(bool bDetachHistory, bool bDelete)
{
	try {

		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;

		//Make sure we have a valid row
		if(pRow != NULL) {
			// (r.gonet 09/02/2014) - PLID 63221 - Get the ID of the lab this file is associated with.
			long nLabID = -1;
			NXDATALIST2Lib::IRowSettingsPtr pRowIter = pRow;
			while (pRowIter != NULL) {
				if (VarLong(pRowIter->GetValue(llRowType)) == llrLabRow) {
					nLabID = VarLong(pRowIter->GetValue(llLabID), -1);
					break;
				} else {
					pRowIter = pRowIter->GetParentRow();
				}
			}

			//Make sure we have a document row selected
			if(VarLong(pRow->GetValue(llRowType)) != llrDocumentRow)
				return;

			//Get the required information for the operations
			long nStepID = VarLong(pRow->GetValue(llStepID));
			long nMailID = VarLong(pRow->GetValue(llMailID));
			CString strFileName = VarString(pRow->GetValue(llPath));
			bool bFailure = false;
			CStringArray astrMessages; // Error messages

			//Determine the action we should take
			//***Detach From Lab***
			if(!bDetachHistory && !bDelete) {
				
				//Prompt the user
				if(IDNO == MessageBox("Are you absolutely sure you wish to detach this file from the lab?\nThe file will still remain in the patient's history.", "Practice", MB_ICONQUESTION|MB_YESNO))
					return;

				//Update the MailSent record so it is not attached to this lab
				ExecuteSql("UPDATE MailSent SET LabStepID = NULL WHERE MailID = %li and LabStepID = %li",
					nMailID, nStepID);

					//DRT 7/5/2006 - PLID 21084 - Auditing
					long nAuditID = BeginNewAuditEvent();
					// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient 
					AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientLabDocDetachLabOnly, nMailID, strFileName, "<Detached>", aepHigh, aetDeleted);
			}

			//***Detach From Lab And History***
			else if(bDetachHistory && !bDelete) {

				//Prompt the user
				if(IDNO == MessageBox("Are you absolutely sure you wish to detach this file from the lab and the patient's history?", "Practice", MB_ICONQUESTION|MB_YESNO))
					return;

				//Delete the MailSent record so it is not attached to this lab nor the patient's history

				// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
				// (j.gruber 2008-10-16 12:55) - PLID 31432 - labResultsT
				CString strSqlBatch = BeginSqlBatch();
				long nAuditTransactionID = -1;
				CString strWhere;
				strWhere.Format(" MailID IN (SELECT MailID FROM MailSent WHERE MailID = %li and LabStepID = %li) ", nMailID, nStepID);
				// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
				nAuditTransactionID = HandleLabResultAudit(strWhere, GetPatientName(), GetPatientID());
				// (c.haag 2009-05-06 13:26) - PLID 33789 - Also clear out the Units field
				AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID IN (SELECT MailID FROM MailSent WHERE MailID = %li and LabStepID = %li)", nMailID, nStepID);
				// (c.haag 2009-10-12 12:35) - PLID 35722 - We now use AddDeleteMailSentQueryToSqlBatch for deleting MailSent records
				AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("SELECT MailID FROM MailSent WHERE MailID = %li and LabStepID = %li", nMailID, nStepID));
				try {
					ExecuteSqlBatch(strSqlBatch);
					if (nAuditTransactionID != -1) {
						CommitAuditTransaction(nAuditTransactionID);
					}
				}NxCatchAllSilentCallThrow(
					if (nAuditTransactionID != -1) {
						RollbackAuditTransaction(nAuditTransactionID);
						nAuditTransactionID = -1;
					}
				);

				// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
				// we have to send those as well.
				// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker
				CClient::RefreshMailSentTable(GetPatientID(), FileUtils::IsImageFile(strFileName) ? TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto : TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);

				CClient::RefreshTable(NetUtils::TodoList, -1);
				// (r.gonet 09/02/2014) - PLID 63221 - Send with the patient and the lab ID
				CClient::RefreshLabsTable(GetPatientID(), nLabID); // (c.haag 2010-07-21 11:40) - PLID 30894 - Send a labs table checker because lab result data may have changed

				//DRT 7/5/2006 - PLID 21084 - Auditing
				long nAuditID = BeginNewAuditEvent();
				// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
				AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientDocDetach, nMailID, strFileName, "<Detached>", aepHigh, aetDeleted);
			}

			//***Detach And Delete File***
			else if(bDetachHistory && bDelete) {

				//Prompt the user
				if(IDNO == MessageBox("Are you absolutely sure you wish to detach and delete this file?", "Practice", MB_ICONQUESTION|MB_YESNO))
					return;

				//Determine the full filename, including path
				CString strFullFileName;
				if(!strFileName.IsEmpty()) {
					if (strFileName.Find('\\') == -1)
						// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
						strFullFileName = GetPatientDocumentPath(GetPatientID()) ^ strFileName;
					else
						strFullFileName = strFileName;
				}

				//Delete the file.  We do this first so we don't detach if there is a problem deleting the file.
				//Code para-phrased from CHistoryDlg::DetachSelectedDocuments
				if (!DeleteFile(strFullFileName))
				{
					//Set the flag to prevent detaching
					bFailure = true;

					long nLastError = GetLastError();

					if (nLastError == 2)
					{
						CString str;
						str.Format("The document '%s' was detached, but could not be deleted because it doesn't exist.", strFileName);
						astrMessages.Add(str);
						bFailure = false;
					}
					else
					{
						if (astrMessages.GetSize() < 20)
						{
							CString str;
							LPVOID lpMsgBuf;
							FormatMessage( 
								FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM | 
								FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL,
								nLastError,
								MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
								(LPTSTR) &lpMsgBuf,
								0,
								NULL 
							);
							str.Format("The document '%s' could not be deleted, and was also not detached. Reason: %s", strFileName, (LPCTSTR)lpMsgBuf);
							astrMessages.Add(str);
							LocalFree(lpMsgBuf);
						}
					}

					if (astrMessages.GetSize() > 0)
					{
						CString str = "The operation completed with the following messages:\n\n";
						for (long i=0; i < astrMessages.GetSize(); i++)
							str += astrMessages[i] + "\n";
						MsgBox(str);
					}
				}
					
				if(!bFailure) {

					//Delete the MailSent record so it is not attached to this lab nor the patient's history

					// (j.jones 2008-09-04 17:45) - PLID 30288 - supported MailSentNotesT
					CString strSqlBatch = BeginSqlBatch();
					// (j.gruber 2008-10-16 12:55) - PLID 31432 - labResultsT
					long nAuditTransactionID = -1;
					CString strWhere;
					strWhere.Format(" MailID IN (SELECT MailID FROM MailSent WHERE MailID = %li and LabStepID = %li) ", nMailID, nStepID);
					// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
					nAuditTransactionID = HandleLabResultAudit(strWhere, GetPatientName(), GetPatientID());
					// (c.haag 2009-05-06 13:26) - PLID 33789 - Also clear out the Units field
					AddStatementToSqlBatch(strSqlBatch, "UPDATE LabResultsT SET MailID = NULL, Value = '', Units = '' WHERE MailID IN (SELECT MailID FROM MailSent WHERE MailID = %li and LabStepID = %li)", nMailID, nStepID);
					// (c.haag 2009-10-12 12:35) - PLID 35722 - We now use AddDeleteMailSentQueryToSqlBatch for deleting MailSent records
					AddDeleteMailSentQueryToSqlBatch(strSqlBatch, FormatString("SELECT MailID FROM MailSent WHERE MailID = %li and LabStepID = %li", nMailID, nStepID));
					try {
						ExecuteSqlBatch(strSqlBatch);
						if (nAuditTransactionID != -1) {
							CommitAuditTransaction(nAuditTransactionID);
						}
					}NxCatchAllSilentCallThrow(
						if (nAuditTransactionID != -1) {
							RollbackAuditTransaction(nAuditTransactionID);
							nAuditTransactionID = -1;
						}
					);

					// (c.haag 2010-06-10 17:19) - PLID 39057 - Send table checkers. Because todo alarms can now link with MailSent records,
					// we have to send those as well.
					// (j.jones 2014-08-04 13:31) - PLID 63141 - this now sends an Ex tablechecker
					CClient::RefreshMailSentTable(GetPatientID(), FileUtils::IsImageFile(strFileName) ? TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto : TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);

					CClient::RefreshTable(NetUtils::TodoList, -1);
					// (r.gonet 09/02/2014) - PLID 63221 - Send with the patient and lab ID
					CClient::RefreshLabsTable(GetPatientID(), nLabID); // (c.haag 2010-07-21 11:40) - PLID 30894 - Send a labs table checker because lab result data may have changed

					//DRT 7/5/2006 - PLID 21084 - Auditing
					long nAuditID = BeginNewAuditEvent();
					// (r.galicki 2008-10-31 17:33) - PL31555 - Labs available in PIC, need to get name of correct patient
					AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientDocDetach, nMailID, strFileName, "<Detached>", aepHigh, aetDeleted);
				}
			}

			//Remove the row from the list (so we don't have to requery)
			if(!bFailure)
				m_pLabsList->RemoveRow(pRow);

			EnableAppropriateButtons();
		}
	} NxCatchAll("Error in CPatientLabsDlg::DetachFile");
}

void CPatientLabsDlg::OnDetachFromLab() 
{
	try {
		//Detach From Lab
		// (m.hancock 2006-06-23 13:37) - PLID 21196 - Detach the selected document
		DetachFile(false, false);
	} NxCatchAll("Error in CPatientLabsDlg::OnDetachFromLab");
}

void CPatientLabsDlg::OnDetachFromLabAndHistory() 
{
	try {
		//Detach From Lab And History
		// (m.hancock 2006-06-23 13:37) - PLID 21196 - Detach the selected document
		DetachFile(true, false);
	} NxCatchAll("Error in CPatientLabsDlg::OnDetachFromLabAndHistory");
}

void CPatientLabsDlg::OnDetachAndDeleteFile() 
{
	try {
		//Detach And Delete File
		// (m.hancock 2006-06-23 13:37) - PLID 21196 - Detach the selected document
		DetachFile(true, true);
	} NxCatchAll("Error in CPatientLabsDlg::OnDetachAndDeleteFile");
}

void CPatientLabsDlg::OnAttachExistingFile() 
{
	try {
		// (m.hancock 2006-06-26 11:37) - PLID 21071 - Allow attachment of documents from the Patient Labs tab
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			
			//Make sure we have a step row selected
			if(VarLong(pRow->GetValue(llRowType)) != llrStepRow)
				return;

			//Get the StepID
			long nStepID = VarLong(pRow->GetValue(llStepID));

			// Code paraphrased from CHistoryDlg::OnClickAttach().
			CString path;
			//DRT 10/28/2003 - PLID 9921 - Added a row for "Commonly Attached" which contains *.doc, *.xls, *.jpg, 
			//*.bmp, *.pcx, *.tiff, *.wav.  This is the default selected item, so that users can quickly add things
			//without changing the file type.
			//	Also changed the description of the other items to include the types they show
			//PLID 18882 - added PDF's to the commonly selected files and their own type
			// (a.walling 2007-07-19 11:25) - PLID 26748 - Added office 2007 files (docx, xlsx) also jpeg
			// (a.walling 2007-09-17 16:11) - PLID 26748 - Also need PowerPoint 2007 files.
			// (a.walling 2007-10-12 14:50) - PLID 26342 - Moved to a shared string, and also include other formats
			// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
			CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, szCommonlyAttached);
			CString strInitPath = GetPatientDocumentPath(GetPatientID());
			
			if(!DoesExist(strInitPath)) {
				// This shouldn't be possible because either GetPatientDocumentPath should return a 
				// valid path to a folder that exists, or it should throw an exception
				ASSERT(FALSE);
				AfxThrowNxException("Person document folder '%s' could not be found", path);
				return;
			}

			dlg.m_ofn.lpstrInitialDir = strInitPath;
			//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.
			char * strFile = new char[5000];
			strFile[0] = 0;
			dlg.m_ofn.nMaxFile = 5000;
			dlg.m_ofn.lpstrFile = strFile;

			if (dlg.DoModal() == IDOK) {
				POSITION p = dlg.GetStartPosition();
				while (p) {	
					
					// (m.hancock 2006-06-30 11:10) - PLID 21280 - Adding a preference for assigning default attachment category
					//TES 8/3/2011 - PLID 44814 - Split to its own function
					long nCategoryID = GetDefaultAttachmentCategory();

					// (m.hancock 2006-06-27 16:33) - PLID 21071 - Attach the file to history and this lab step, returning the MailID
					CString strPath = dlg.GetNextPathName(p);
					// (a.walling 2007-08-21 07:54) - PLID 26748 - Need to support Office 2007 files
					// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
					CString strType;
					if ( (strPath.Right(4).CompareNoCase(".doc") == 0) || (strPath.Right(5).CompareNoCase(".docx") == 0) || (strPath.Right(5).CompareNoCase(".docm") == 0))
						strType = SELECTION_WORDDOC;
					else strType = SELECTION_FILE;

					// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
					long nMailSentID = ::AttachFileToHistory(strPath, GetPatientID(), GetSafeHwnd(), nCategoryID, strType, NULL, -1, nStepID);
				}
				UpdateView();
			}
			delete [] strFile;
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnAttachExistingFile");
}

void CPatientLabsDlg::OnAttachExistingFolder() 
{
	try {
		// (m.hancock 2006-06-26 11:37) - PLID 21071 - Allow attachment of documents from the Patient Labs tab
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			
			//Make sure we have a step row selected
			if(VarLong(pRow->GetValue(llRowType)) != llrStepRow)
				return;

			//Get the StepID
			long nStepID = VarLong(pRow->GetValue(llStepID));

			//Code paraphrased from CHistoryDlg::OnClickAttachFolder
			CString path;
			// (a.walling 2008-01-29 14:33) - PLID 28716 - "Attach History Path" preference is now deprecated.
			if (Browse(m_hWnd, "", path, false)) {

				// (m.hancock 2006-06-30 11:10) - PLID 21280 - Adding a preference for assigning default attachment category
				//TES 8/3/2011 - PLID 44814 - Split to its own function
				long nCategoryID = GetDefaultAttachmentCategory();

				// (m.hancock 2006-06-27 16:31) - PLID 21071 - Attach the file to history and this lab step, returning the MailID
				// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
				long nMailSentID = ::AttachFileToHistory(path, GetPatientID(), GetSafeHwnd(), nCategoryID, SELECTION_FOLDER, NULL, -1, nStepID);
			}
		}
		UpdateView();

	} NxCatchAll("Error in CPatientLabsDlg::OnAttachExistingFolder");
}

void CPatientLabsDlg::OnImportAndAttachExistingFile() 
{
	try {
		// (m.hancock 2006-06-26 11:37) - PLID 21071 - Allow attachment of documents from the Patient Labs tab

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			
			//Make sure we have a step row selected
			if(VarLong(pRow->GetValue(llRowType)) != llrStepRow)
				return;

			//Get the StepID
			long nStepID = VarLong(pRow->GetValue(llStepID));

			//Code paraphrased from CHistoryDlg::OnClickImportAndAttach
			CString path;
			//JMJ 12/4/2003 - Added a row for "Commonly Attached" which contains *.doc, *.xls, *.jpg, 
			//*.bmp, *.pcx, *.tiff, *.wav.  This is the default selected item, so that users can quickly add things
			//without changing the file type.
			//	Also changed the description of the other items to include the types they show
			//PLID 18882 - added pdf's to commonly attached and on their own line
			// (a.walling 2007-07-19 11:27) - PLID 26748 - Add Office 2007 files (docx, xlsx) also jpeg
			// (a.walling 2007-09-17 16:11) - PLID 26748 - Also need PowerPoint 2007 files.
			// (a.walling 2007-10-12 14:50) - PLID 26342 - Moved to a shared string, and also include other formats
			CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT, szCommonlyAttached);
			// (a.walling 2008-01-29 14:33) - PLID 28716 - "Attach History Path" preference is now deprecated.
			dlg.m_ofn.lpstrInitialDir = NULL;
			//TES 5/26/2004: We have to supply our own buffer, otherwise it will only allow 255 characters.
			char * strFile = new char[5000];
			strFile[0] = 0;
			dlg.m_ofn.nMaxFile = 5000;
			dlg.m_ofn.lpstrFile = strFile;
			if (dlg.DoModal() == IDOK)
			{
				POSITION p = dlg.GetStartPosition();
				while (p) {
					CString strSourcePath = dlg.GetNextPathName(p);
					// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
					CString strDstPath = GetPatientDocumentPath(GetPatientID()) ^ GetFileName(strSourcePath);
 					if (CopyFile(strSourcePath, strDstPath, TRUE)) {

						// (m.hancock 2006-06-30 11:10) - PLID 21280 - Adding a preference for assigning default attachment category
						//TES 8/3/2011 - PLID 44814 - Split to its own function
						long nCategoryID = GetDefaultAttachmentCategory();

						// (a.walling 2007-08-21 07:54) - PLID 26748 - Need to support Office 2007 files
						// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
						CString strType;
						if ( (strDstPath.Right(4).CompareNoCase(".doc") == 0) || (strDstPath.Right(5).CompareNoCase(".docx") == 0) || (strDstPath.Right(5).CompareNoCase(".docm") == 0))
							strType = SELECTION_WORDDOC;
						else strType = SELECTION_FILE;

						// (m.hancock 2006-06-27 16:31) - PLID 21071 - Attach the file to history and this lab step, returning the MailID
						// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
						long nMailSentID = ::AttachFileToHistory(strDstPath, GetPatientID(), GetSafeHwnd(), nCategoryID, strType, NULL, -1, nStepID);
					}
					else {
						//If the file already exists here, prompt for rename/cancel
						DWORD dwLastError = GetLastError();
						if(dwLastError == ERROR_FILE_EXISTS) {
							// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
							strDstPath = GetPatientDocumentPath(GetPatientID());
							CRenameFileDlg dlgRename(strSourcePath, strDstPath, this);
							try {
								if(dlgRename.DoModal() == IDOK) {
									strDstPath = dlgRename.m_strDstFullPath;
									if(CopyFile(strSourcePath, strDstPath, TRUE)) {

										// (m.hancock 2006-06-30 11:10) - PLID 21280 - Adding a preference for assigning default attachment category
										//TES 8/3/2011 - PLID 44814 - Split to its own function
										long nCategoryID = GetDefaultAttachmentCategory();

										// (a.walling 2007-08-21 07:54) - PLID 26748 - Need to support Office 2007 files
										// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
										CString strType;
										if ( (strDstPath.Right(4).CompareNoCase(".doc") == 0) || (strDstPath.Right(5).CompareNoCase(".docx") == 0) || (strDstPath.Right(5).CompareNoCase(".docm") == 0) )
											strType = SELECTION_WORDDOC;
										else strType = SELECTION_FILE;

										// (m.hancock 2006-06-27 16:31) - PLID 21071 - Attach the file to history and this lab step, returning the MailID
										// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
										long nMailSentID = ::AttachFileToHistory(strDstPath, GetPatientID(), GetSafeHwnd(), nCategoryID, strType, NULL, -1, nStepID);
									}
								}
								else {
									if(p) {
										//There are more files to import, see if they want to keep going.
										if(IDYES != MsgBox(MB_YESNO, "Would you like to contine with the import?")) {
											break;
										}
									}
								}
								dwLastError = GetLastError();
							}NxCatchAll("Error in CPatientLabsDlg::OnImportAndAttachExistingFile:RenamingFile");
						}
						//there's an error other than the file already existing
						if(dwLastError != ERROR_SUCCESS) {
							CString strError;
							FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
							if(IDYES != MsgBox(MB_YESNO, "The file '%s' could not be imported into the patient's documents folder. Windows returned the following error message:\r\n\r\n"
								"%s\r\n"
								"Would you like to continue?", GetFileName(strSourcePath), strError))
								break;
						}
					}
				}
			}
			delete [] strFile;
		}
		UpdateView();

	} NxCatchAll("Error in CPatientLabsDlg::OnImportAndAttachExistingFile");
}

// (c.haag 2010-05-05 17:36) - PLID 36096 - Utility function for acquiring documents from a TWAIN device
void CPatientLabsDlg::DoTWAINAcquisition(NXTWAINlib::EScanTargetFormat eScanTargetFormat)
{
	// (m.hancock 2006-06-26 11:37) - PLID 21071 - Allow attachment of documents from the Patient Labs tab

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
	//Make sure we actually have a row selected
	if(pRow != NULL) {
		
		//Make sure we have a step row selected
		if(VarLong(pRow->GetValue(llRowType)) != llrStepRow)
			return;

		//Get the StepID
		long nStepID = VarLong(pRow->GetValue(llStepID));

		//Code paraphrased from CHistoryDlg::OnAcquire()
		if (NXTWAINlib::IsAcquiring())
		{
			MsgBox("Please wait for the current image acquisition to complete before starting a new one.");
			return;
		}

		// (m.hancock 2006-06-30 11:10) - PLID 21280 - Adding a preference for assigning default attachment category
		//TES 8/3/2011 - PLID 44814 - Split to its own function
		long nCategoryID = GetDefaultAttachmentCategory();

		// (m.hancock 2006-06-27 16:34) - PLID 21071 - Acquire and attach the file to history and this lab step
		// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
		long nPatientID = GetPatientID();
		NXTWAINlib::Acquire(GetPatientID(), GetPatientDocumentPath(nPatientID), NULL, OnNxTwainPreCompress, NULL, "", nCategoryID, nStepID, -1, eScanTargetFormat);
	}
}

// (c.haag 2010-05-05 17:36) - PLID 36096 - Scan as PDF
void CPatientLabsDlg::OnImportFromScanAsPDF()
{
	try {
		DoTWAINAcquisition(NXTWAINlib::eScanToPDF);
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-05-05 17:36) - PLID 36096 - Scan as Multi-Page PDF
void CPatientLabsDlg::OnImportFromScanAsMulti()
{
	try {
		DoTWAINAcquisition(NXTWAINlib::eScanToMultiPDF);
	}
	NxCatchAll(__FUNCTION__);
}

void CPatientLabsDlg::OnImportFromScannerCamera() 
{
	try {
		// (c.haag 2010-05-05 17:36) - PLID 36096 - Moved acquisition into its own function
		DoTWAINAcquisition(NXTWAINlib::eScanToImage);
	} 
	NxCatchAll("Error in CPatientLabsDlg::OnImportFromScannerCamera");
}

void CPatientLabsDlg::OnEditingStartingLabsList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);

		// (m.hancock 2006-06-26 15:33) - PLID 21071 - Allow attachment of documents from the Patient Labs tab

		//We can only allow editing of the done and description fields (only for the document rows)
		//Set the selection to the row that was clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//Allow editing if we're on a document row and the column is llDescription
		if((nCol == llDescription) && (VarLong(pRow->GetValue(llRowType)) == llrDocumentRow)) {
			*pbContinue = true;
			return;
		}

		//Allow editing if we're in the Done column
		if(nCol == llDone) {
			*pbContinue = true;
			return;
		}

		//Prevent all other cases of editing
		else {
			*pbContinue = false;
			return;
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnEditingStartingLabsList");
}

// (m.hancock 2006-06-27 10:11) - PLID 21070 - Returns the string value for the passed number.
// 0 is ""{No Selection}, 1 is "Left", 2 is "Right"
//TES 11/10/2009 - PLID 36128 - This is no longer an enum, but a configurable table, so the description should be loaded along with the ID.
//TES 12/8/2009 - PLID 36470 - AnatomySide is back!
CString CPatientLabsDlg::GetAnatomySideString(long nSide)
{
	CString strSideName = "";
	try {
		switch(nSide) {
		case asLeft:
			strSideName = "Left"; //Left
			break;
		case asRight:
			strSideName = "Right"; //Right
			break;
		case asNone:
		default:
			strSideName = ""; //(No selection)
			break;
		}
	} NxCatchAll("Error in CPatientLabsDlg::GetAnatomySideString");
	return strSideName;
}

// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
void WINAPI CALLBACK CPatientLabsDlg::OnNxTwainPreCompress(const LPBITMAPINFO pDib, /* The image that was scanned */
		BOOL& bAttach,
		BOOL& bAttachChecksum, long &nChecksum, ADODB::_Connection* lpCon)
{
	try {
		nChecksum = BitmapChecksum(pDib);
		//See if this image has been attached already, by generating a checksum, and checking for its existence.
		// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam(lpCon, "SELECT MailID FROM MailSent WHERE Checksum = {INT}", nChecksum)) {
			bAttach = FALSE;
		}
		else {
			bAttach = TRUE;
			bAttachChecksum = TRUE;
		}
	} NxCatchAll_NoParent("Error in CPatientLabsDlg::OnNxTwainPreCompress"); // (a.walling 2014-05-05 13:32) - PLID 61945
}

void CPatientLabsDlg::EnableAppropriateButtons()
{
	try {
		// (m.hancock 2006-06-28 11:28) - PLID 21205 - call this funtion to enable or disable the appropriate buttons on screen
		//First, disable buttons that can be disabled
		GetDlgItem(IDC_DELETE_LAB)->EnableWindow(FALSE);
		GetDlgItem(IDC_NEW_ATTACHMENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_DETACH)->EnableWindow(FALSE);

		//Second, determine what type of row is selected (if any)
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			long nRowType = VarLong(pRow->GetValue(llRowType), -1);
			//Third, enable buttons based on the row that is currently selected
			switch(nRowType) {

				//Lab
				case llrLabRow: 
				{
					GetDlgItem(IDC_DELETE_LAB)->EnableWindow(TRUE);
					break;
				}

				//Step
				case llrStepRow: 
				{
					GetDlgItem(IDC_NEW_ATTACHMENT)->EnableWindow(TRUE);
					break;
				}

				//Document
				case llrDocumentRow: 
				{
					GetDlgItem(IDC_DETACH)->EnableWindow(TRUE);
					break;
				}

				//Spacer / Default
				case llrSpacerRow: 
				default:
				{
					//Do not enable any of the disabled buttons
					break;
				}
			}
		}
	} NxCatchAll("Error in CPatientLabsDlg::EnableAppropriateButtons");
}

void CPatientLabsDlg::OnPrintAttachment() 
{
	try {
		// (m.hancock 2006-06-28 14:00) - PLID 21071 - Allow the user to print the attachment
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		//Make sure we actually have a row selected
		if(pRow != NULL) {
			//Make sure we have a document row selected
			if(VarLong(pRow->GetValue(llRowType)) == llrDocumentRow) {

				//Get the path for the document
				CString strPath = VarString(pRow->GetValue(llPath), "");

				//Determine the full filename, including path
				CString strFullFileName;
				if(!strPath.IsEmpty()) {
					if (strPath.Find('\\') == -1)
						// (r.galicki 2008-10-28 12:26) - PLID 31555 - Use GetPatientID() instead of GetActivePatientID()
						strFullFileName = GetPatientDocumentPath(GetPatientID()) ^ strPath;
					else
						strFullFileName = strPath;
				}

				try {
					//Check if the file exists at the location
					if(DoesExist(strFullFileName)) {
						//The file exists, so print it
						PrintFile(strFullFileName);
					}
				} NxCatchAll("Error printing file");
			}
		}
	} NxCatchAll("Error in CPatientLabsDlg::OnPrintAttachment");
}

void CPatientLabsDlg::OnDblClickCellLabsList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);

		//Set the selection to the row that was clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pLabsList->CurSel = pRow;

		//EnableAppropriateButtons();

		//Make sure we didn't select the spacer row
		if(VarLong(pRow->GetValue(llRowType)) == llrSpacerRow)
			return;

		switch(nColIndex) {
		
		case llDescription: 
			{
				// (m.hancock 2006-06-28 14:57) - PLID 21070 - We need to open the Lab entry dialog if we double clicked a lab
				// We only want to allow this if the current row is a lab
				if(VarLong(pRow->GetValue(llRowType)) == llrLabRow) {

					//Keep the row for the lab expanded
					pRow->PutExpanded(VARIANT_TRUE);

					//DRT 7/7/2006 - PLID 21088 - Check permissions for write
					if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
						return;

					//Get the labID
					long nLabID = VarLong(pRow->GetValue(llLabID));
					
					//Retrieve the LabProcedureID for this lab
					long nLabProcedureID = -1;
					// (z.manning 2008-10-09 12:11) - PLID 31633 - LabProcedureID is now in LabsT (instead of LabStepsT)
					_RecordsetPtr prsLabProcedure = CreateRecordset(
					"SELECT LabProcedureID FROM LabsT WHERE ID = %li", nLabID);
					if(!prsLabProcedure->eof) {
						nLabProcedureID = AdoFldLong(prsLabProcedure, "LabProcedureID");
					}

					// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
					// (a.walling 2012-07-10 14:16) - PLID 46648 - Dialogs must use a parent
					GetMainFrame()->OpenLab(GetPicContainer() ? GetTopLevelFrame() : NULL, GetPatientID(), (nLabProcedureID > -1) ? nLabProcedureID : -1, ltInvalid, nLabID, -1, "", -1,
						FALSE,
						(GetPicContainer()) ? TRUE : FALSE, // Open modal in a PIC, or modeless outside the PIC
						GetSafeHwnd());
				}
				break;
			}
		}

	} NxCatchAll("Error in CPatientLabsDlg::OnDblClickCellLabsList");
}

void CPatientLabsDlg::OnSelChangedLabsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		// (c.haag 2010-09-15 12:20) - PLID 40512 - Ignore any table checker messages when we're here
		CPatientLabsIgnoreTableChecker plitc(this);
		// (m.hancock 2006-06-28 15:50) - PLID 21205 - Enable appropriate buttons based on the current row selections
		EnableAppropriateButtons();
	} NxCatchAll("Error in CPatientLabsDlg::OnSelChangedLabsList");
}

// (c.haag 2010-01-19) - PLID 41825 - Return FALSE if the operation was cancelled or failed
BOOL CPatientLabsDlg::MarkLabIncomplete(LPDISPATCH lpLabRow)
{
	try {
		// (m.hancock 2006-07-19 14:49) - PLID 21516 - Users with permission to mark a lab complete should 
		// also be able to mark the lab's top record as incomplete.

		//Open the lab
		if (!ReopenLab(lpLabRow)) {
			return FALSE;
		}

		//Update the display
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpLabRow);		
		//If we are currently viewing closed labs, we should remove this lab because it is open now
		if(IsDlgButtonChecked(IDC_CLOSED_LABS)) {
			//Remove the lab row and its spacer row beneath it
			//Get the spacer row
			NXDATALIST2Lib::IRowSettingsPtr pSpacerRow = pRow->GetNextRow();
			if(pSpacerRow != NULL) {
				if(VarLong(pSpacerRow->GetValue(llRowType)) == llrSpacerRow)
				{
					//Remove the lab and spacer rows
					m_pLabsList->RemoveRow(pRow);
					m_pLabsList->RemoveRow(pSpacerRow);
				}	
			}
		}
		else { //Open labs or All labs

			//Declare a null variant
			_variant_t varNull;
			varNull.vt = VT_NULL;

			//Update the rows
			pRow->PutValue(llCompletedDate, varNull);
			pRow->PutValue(llCompletedBy, _bstr_t(""));
			//pRow->PutValue(llDone, _variant_t(VARIANT_FALSE, VT_BOOL));
			// (c.haag 2010-09-15 12:58) - PLID 40461 - Stay grayed out if discontinued
			pRow->PutBackColor(VarBool(pRow->GetValue(llDiscontinued),FALSE) ? RGB(192, 192, 192) : CalculateLabColor(false, false));
			return TRUE; // (c.haag 2010-01-19) - PLID 41825
		}
	} NxCatchAll("Error in CPatientLabsDlg::MarkLabIncomplete");
	return FALSE; // (c.haag 2010-01-19) - PLID 41825
}

void CPatientLabsDlg::OnOpenLab() 
{
	try {
		// (m.hancock 2006-07-21 17:08) - PLID 21567 - Add right click menu option to open the lab entry dialog
		// when right clicking on a step.

		//Get the row that was clicked
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		
		//Make sure we have a valid row
		if(pRow != NULL) {

			//Make sure we selected a lab or step row 
			long nRowType = VarLong(pRow->GetValue(llRowType));
			if((nRowType == llrLabRow) || (nRowType == llrStepRow)) {

				//Keep the row for the lab expanded
				pRow->PutExpanded(VARIANT_TRUE);

				//DRT 7/7/2006 - PLID 21088 - Check permissions for write
				if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
					return;

				//Get the labID
				long nLabID = VarLong(pRow->GetValue(llLabID));
				
				//Retrieve the LabProcedureID for this lab
				long nLabProcedureID = -1;
				// (z.manning 2008-10-09 12:11) - PLID 31633 - LabProcedureID is now in LabsT (instead of LabStepsT)
				_RecordsetPtr prsLabProcedure = CreateRecordset(
				"SELECT LabProcedureID FROM LabsT WHERE ID = %li", nLabID);
				if(!prsLabProcedure->eof) {
					nLabProcedureID = AdoFldLong(prsLabProcedure, "LabProcedureID");
				}

				LabType ltType = ltBiopsy;
				_RecordsetPtr prsLabType = CreateRecordset(
				"SELECT Type FROM LabProceduresT WHERE ID = %li", nLabProcedureID);
				if(!prsLabType->eof) {
					LabType ltType = LabType(AdoFldByte(prsLabType->GetFields(), "Type"));
				}

				//Open the lab entry dialog
				// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
				// (a.walling 2012-07-10 14:16) - PLID 46648 - Dialogs must use a parent
				GetMainFrame()->OpenLab(GetPicContainer() ? GetTopLevelFrame() : NULL, GetPatientID(), (nLabProcedureID > -1) ? nLabProcedureID : -1, ltInvalid, nLabID, -1, "", -1,
					FALSE,
					(GetPicContainer()) ? TRUE : FALSE, // Open modal in a PIC, or modeless outside the PIC
					GetSafeHwnd());
			}
		}

	} NxCatchAll("Error in CPatientLabsDlg::OnOpenLab");
}

// (c.haag 2010-09-09 13:46) - PLID 40461 - We now support discontinuing labs
void CPatientLabsDlg::OnDiscontinueLab()
{
	try {
		// Make sure we have a valid row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->CurSel;
		if (pRow != NULL) 
		{
			long nLabID = VarLong(pRow->GetValue(llLabID));
			if (nLabID > -1) 
			{
				CString strPrompt;
				CString strSql;
				if (VarBool(pRow->GetValue(llDiscontinued))) 
				{
					strPrompt = "Are you SURE you wish to remove the discontinued status of this lab?";
					strSql = "UPDATE LabsT SET Discontinued = 0, DiscontinuedDate = NULL WHERE ID = {INT}";
				}
				else 
				{
					strPrompt = "Are you SURE you wish to discontinue this lab?";
					strSql = "UPDATE LabsT SET Discontinued = 1, DiscontinuedDate = GETDATE() WHERE ID = {INT}";
				}
				if (IDYES == AfxMessageBox(strPrompt, MB_YESNO | MB_ICONQUESTION))
				{	
					// Discontinue the lab
					ExecuteParamSql(strSql, nLabID);

					// (v.maida 2014-08-13 12:30) - PLID 43964 - Determine whether the lab was discontinued or reactivated, and then audit the discontinuation or reactivation.
					pRow = pRow->GetParentRow() == NULL ? pRow : pRow->GetParentRow(); // if we're already on the parent row, then use that, otherwise change the current row to the parent row, so that its description can be used.
					if (VarBool(pRow->GetValue(llDiscontinued)) == FALSE) { //  make sure that the lab is actually being discontinued, instead of being changed from discontinued back to active.
						long nAuditID = BeginNewAuditEvent();
						CString strOldStatus = FormatString("Lab %s was active.", VarString(pRow->GetValue(llDescription)));
						CString strNewStatus = FormatString("Lab %s has been discontinued.", VarString(pRow->GetValue(llDescription)));
						AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientLabDiscontinued, nLabID, strOldStatus, strNewStatus, aepMedium, aetChanged);
					}
					else {
						// the lab was previously discontinued, so now it's being reactivated.
						long nAuditID = BeginNewAuditEvent();
						CString strOldStatus = FormatString("Lab %s was discontinued.", VarString(pRow->GetValue(llDescription)));
						CString strNewStatus = FormatString("Lab %s has been reactivated.", VarString(pRow->GetValue(llDescription)));
						AuditEvent(GetPatientID(), GetPatientName(), nAuditID, aeiPatientLabReactivated, nLabID, strOldStatus, strNewStatus, aepMedium, aetChanged);
					}

					// Send a table checker. This will also refresh us locally.
					// (r.gonet 09/02/2014) - PLID 63221 - Send the patient too
					CClient::RefreshLabsTable(GetPatientID(), nLabID);
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (r.galicki 2008-10-07 11:32) - PLID 31555 - Adding "Labs" tab in PIC needed this to auto-resize
void CPatientLabsDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);
		SetControlPositions();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);

	}NxCatchAll("CPatientLabsDlg::OnSize");
}

// (r.galicki 2008-10-08 13:20) - PLID 31555 - returns PIC id
int CPatientLabsDlg::GetPicID() {
	if(GetPicContainer()) {
		return GetPicContainer()->GetCurrentPicID();
	}
	else {
		return -1;
	}
}

void CPatientLabsDlg::OnCheckFilterOnPIC() {

	try {

		// (j.jones 2013-01-30 10:40) - PLID 45871 - remembers this setting, per user
		SetRemotePropertyInt("LabsFilterOnPIC", m_checkFilterOnPIC.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		UpdateView();

	}NxCatchAll(__FUNCTION__);
}

// (r.galicki 2008-10-28 11:59) - PLID 31555 - Patient ID differs if lab dialog launched in PIC (modeless)
long CPatientLabsDlg::GetPatientID() {
	long nPatientID = -1;
	if(GetPicContainer()) {
		nPatientID = GetPicContainer()->GetPatientID();
	}
	else { //get active patient
		nPatientID = GetActivePatientID();
	}

	return nPatientID;
}

// (r.galicki 2008-11-03 10:07) - PLID 31555 - Patient Name auditing helper
CString CPatientLabsDlg::GetPatientName() {
	return GetExistingPatientName(GetPatientID());
}

// (j.gruber 2009-05-07 11:22) - PLID 28556 - created result graph preview button
void CPatientLabsDlg::OnBnClickedResultGraphPreview()
{
	try {

		// (z.manning 2009-06-11 16:55) - PLID 34604 - Added ability to filter on which lab results
		// to graph.
		_RecordsetPtr prsResultNames = CreateParamRecordset(
			"SELECT DISTINCT Name \r\n"
			"FROM LabsT LEFT JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID \r\n"
			"WHERE LabsT.PatientID = {INT} AND LabsT.Deleted = 0 AND LabResultsT.Deleted = 0 \r\n"
			, GetPatientID());

		if(prsResultNames->eof) {
			// (z.manning 2009-06-11 16:55) - PLID 34604 - We have nothing to graph, so return.
			MessageBox("There are no lab results for this patient.");
			return;
		}

		// (z.manning 2009-06-11 16:55) - PLID 34604 - Populate a list for the multi select dialog
		CString strDelimitedList;
		CMap<long,long,CString,LPCTSTR> mapResultIndexToName;
		for(long i = 0; !prsResultNames->eof; prsResultNames->MoveNext(), i++) {
			CString strResultName = AdoFldString(prsResultNames->GetFields(), "Name", "");
			// (z.manning 2009-07-15 18:45) - PLID 34604 - We need the real names of the lab results
			// so since we need to replace the semicolons let's store the real name in a map.
			mapResultIndexToName.SetAt(i, strResultName);
			strResultName.Replace(';', '_');
			strDelimitedList += AsString(i) + '-' + strResultName + ';' + strResultName + ';';
		}

		// (z.manning 2009-06-11 16:55) - PLID 34604 - Open the multi select dialog and allow them to
		// chose which results to graph.
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlgMultiSelect(this, "LabResultsT");
		CVariantArray aryvarIDsToSkip;
		int nResult = dlgMultiSelect.OpenWithDelimitedComboSource(_bstr_t(strDelimitedList), aryvarIDsToSkip,
			"Please select the lab results you would like to graph.", 1);
		if(nResult != IDOK) {
			return;
		}

		// (z.manning 2009-06-11 16:55) - PLID 34604 - They selected at least one result to graph, so
		// construct a where clause segment to filter on these result names.
		CVariantArray aryvarSelected;
		dlgMultiSelect.FillArrayWithIDs(aryvarSelected);
		CString strResultNameFilter = " AND (LabResultsT.Name IN (";
		for(int nNameIndex = 0; nNameIndex < aryvarSelected.GetSize(); nNameIndex++) {
			CString strName = VarString(aryvarSelected.GetAt(nNameIndex), "");
			// (z.manning 2009-07-15 18:44) - PLID 34604 - The first part of the name has the
			// index of the map entry with the actual name.
			int nHyphen = strName.Find('-');
			if(nHyphen != -1) {
				long nMapIndex = AsLong(_bstr_t(strName.Left(nHyphen)));
				strName.Delete(0, nHyphen + 1);
				if(!mapResultIndexToName.Lookup(nMapIndex, strName)) {
					// (z.manning 2009-07-15 18:45) - PLID 34604 - We should have found this. See above
					// code that populates map.
					ASSERT(FALSE);
				}
			}
			else {
				// (z.manning 2009-07-15 18:45) - PLID 34604 - Shouldn't be possible as we manually add
				// a hyphen up above.
				ASSERT(FALSE);
			}
			strResultNameFilter += "'" + _Q(strName) + "',";
		}
		strResultNameFilter.TrimRight(',');
		strResultNameFilter += ")) ";

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(661)]);
		
		infReport.nPatient = GetPatientID();
		infReport.strExtraText = strResultNameFilter;
		
		RunReport(&infReport, true, this, "Graphical Lab Results Per Patient");
	}NxCatchAll("Error in CPatientLabsDlg::OnBnClickedResultGraphPreview()");
}

void CPatientLabsDlg::OnShowAllResults()
{
	try {
		//TES 5/14/2009 - PLID 28559 - Pop up the dialog, give it our current patient to filter on.
		CLabResultsDlg dlg(this);
		dlg.m_nPatientID = GetPatientID();
		dlg.m_strPatientName = GetPatientName();
		dlg.DoModal();

	}NxCatchAll("Error in CPatientLabsDlg::OnShowAllResults()");
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This message is received when a lab entry dialog is closed
// and the lab was opened from this specific dialog.
LRESULT CPatientLabsDlg::OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam)
{
	try {
		CLabEntryDlg *pDlg = (CLabEntryDlg *)lParam;
		if (pDlg) {
			ASSERT_VALID((CObject*)pDlg);

			// No need to refresh the labs list; the table checker will handle that

			// (c.haag 2010-07-15 17:23) - PLID 34338 - Copied Zack's code here. Labs opened from the PIC
			// are modal, and we need to minimize the PIC if the user elects to run a report.
			// (z.manning 2008-11-04 10:04) - PLID 31904 - If we opened a report we need to minimize
			// the PIC if we're in the PIC.
			if(GetPicContainer() && pDlg->HasOpenedReport()) {
				GetPicContainer()->SendMessage(NXM_EMR_MINIMIZE_PIC);
			}
		}
	}
	NxCatchAll(__FUNCTION__)
	return 0;
}

// (c.haag 2010-07-19 13:40) - PLID 30894 - Handle table checkers
LRESULT CPatientLabsDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// (r.gonet 09/02/2014) - PLID 63221 - Removed handling of LabsT table checkers.

	}
	NxCatchAll(__FUNCTION__)
	return 0;
}

// (r.gonet 09/02/2014) - PLID 63221 - We now handle lab ex table checkers and mail sent ex table checkers
LRESULT CPatientLabsDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {
		switch (wParam) {

		case NetUtils::MailSent:
		{
			CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
			long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPatientID), -1);
			long nMailSentID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiMailSentID), -1);
				
			if (nPatientID == -1 || nPatientID == GetPatientID()) {
				// (r.gonet 09/02/2014) - PLID 63221 - For the labs list to need reloaded, the mail sent ID must be in one of the existing
				// document type rows. 
				std::vector<long> vecLabIDs;
				long nCurrentLabID = -1;
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLabsList->FindAbsoluteFirstRow(VARIANT_FALSE);
				while (pRow) {
					if (VarLong(pRow->GetValue(llRowType)) == llrLabRow) {
						nCurrentLabID = VarLong(pRow->GetValue(llLabID), -1);
					} else if (VarLong(pRow->GetValue(llRowType)) == llrDocumentRow) {
						long nCurrentMailSentID = VarLong(pRow->GetValue(llMailID), -1);
						if (nCurrentLabID != -1 && (nMailSentID == -1 || nMailSentID == nCurrentMailSentID)) {
							vecLabIDs.push_back(nCurrentLabID);
						} else {
							// (r.gonet 09/02/2014) - PLID 63221 - Bad lab? Document without a lab row?
						}
					} else {
						// (r.gonet 09/02/2014) - PLID 63221 - Either a lab step row or a spacer row. Neither of which we actually care about.
					}

					pRow = m_pLabsList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
				}
				if (vecLabIDs.size() > 0) {
					// (r.gonet 09/02/2014) - PLID 63221 - So at least one of the lab rows has a document
					// that is being updated (or all mailsent records are "changed"), we may be able to reload the labs list now.
					// (c.haag 2010-09-15 12:20) - PLID 40512 - Don't refresh if we're ignoring table checkers
					if (m_nIgnoreTableCheckersRefCnt > 0) {
						m_bGotIgnoredTableChecker = true;
						return 0;
					}

					// (r.gonet 09/02/2014) - PLID 63539 - We can reload the labs list immediately, but frequently we get multiple 
					// table checkers for labs in quick succession. In order to avoid reloading for each one of those, instead set 
					// a timer to wait a little bit for potentially multiple table checker messages to arrive and then reload the labs
					// list just once, avoiding redundant trips to the database. We wait exactly 100 ms, which is an instant to humans.
					if (!m_bCoalescingTableCheckers) {
						m_bCoalescingTableCheckers = true;
						SetTimer(IDT_DELAY_RELOAD_LABS_LIST, 100, NULL);
					} else {
						// We are waiting to reload.
					}
				} else {
					// (r.gonet 09/02/2014) - PLID 63221 - Nothing to load. We're not displaying it. It is either Completed/Not Completed or Discontinued and we're not displaying those labs, 
					// or we've already removed the row. Ignore it.
				}
			}
		}
			break;

		case NetUtils::LabsT:
		{
			// (r.gonet 08/25/2014) - PLID 63221 - Is this for our patient? If so, reload the list.
			CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
			// (r.gonet 09/03/2014) - PLID 63540 - Grab who sent the tablechecker message and if we should ignore it if it came frome this dialog class instance.
			CString strSenderID = VarString(pDetails->GetDetailData((short)TableCheckerDetailIndex::Labs_DetailIndex::SenderID), "");
			BOOL bIgnoreSelf = VarLong(pDetails->GetDetailData((short)TableCheckerDetailIndex::Labs_DetailIndex::IgnoreSelf), _variant_t(0L, VT_I4)) != 0 ? TRUE : FALSE;
			long nPatientID = VarLong(pDetails->GetDetailData((short)TableCheckerDetailIndex::Labs_DetailIndex::PatientID), -1);
			long nLabID = VarLong(pDetails->GetDetailData((short)TableCheckerDetailIndex::Labs_DetailIndex::LabID), -1);

			if (strSenderID == m_labsTableRefresher.GetSenderID() && bIgnoreSelf) {
				// (r.gonet 09/03/2014) - PLID 63540 - Ignore messages that this dialog instance sent if we are instucted to.
				return 0;
			}

			if (-1 == nPatientID || nPatientID == GetPatientID()) {
				// (c.haag 2010-09-15 12:20) - PLID 40512 - Don't refresh if we're ignoring table checkers
				if (m_nIgnoreTableCheckersRefCnt > 0) {
					m_bGotIgnoredTableChecker = true;
					return 0;
				}

				// (r.gonet 09/02/2014) - PLID 63539 - We can reload the labs list immediately, but frequently we get multiple 
				// table checkers for labs in quick succession. In order to avoid reloading for each one of those, instead set 
				// a timer to wait a little bit for potentially multiple table checker messages to arrive and then reload the labs
				// list just once, avoiding redundant trips to the database. We wait exactly 100 ms, which is an instant to humans.
				if (!m_bCoalescingTableCheckers) {
					m_bCoalescingTableCheckers = true;
					SetTimer(IDT_DELAY_RELOAD_LABS_LIST, 100, NULL);
				} else {
					// We are waiting to reload.
				}
			} else {
				// (r.gonet 09/02/2014) - PLID 63221 - Not our patient. Who cares?
			}
		}
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (c.haag 2010-09-09 13:46) - PLID 40461 - Click handler for the discontinued labs filter checkbox
void CPatientLabsDlg::OnBnClickedCheckDiscontinuedLabs()
{
	try {
		CString strDiscontinuedLabPropName;
		if(GetPicContainer()) {
			strDiscontinuedLabPropName = "ShowDiscontinuedLabsInPIC";
		} else {
			strDiscontinuedLabPropName = "ShowDiscontinuedLabsInLabsTab";
		}
		SetRemotePropertyInt(strDiscontinuedLabPropName, (m_checkDiscontinuedLabs.GetCheck() ? 1 : 0), 0, GetCurrentUserName());

		// Reload the list
		ReloadLabList();
	}
	NxCatchAll(__FUNCTION__)
}

//TES 8/2/2011 - PLID 44814 - Moved code to get the default category for lab attachments into this function, so it can also
// check the permissions for the category
long CPatientLabsDlg::GetDefaultAttachmentCategory()
{
	long nCategoryID = GetRemotePropertyInt("LabAttachmentsDefaultCategory",-1,0,"<None>",true);
	if(nCategoryID != -1 && !CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, nCategoryID, TRUE)) {
		nCategoryID = -1;
	}
	return nCategoryID;
}

// (r.gonet 09/02/2014) - PLID 63539 - Added handler for timers.
void CPatientLabsDlg::OnTimer(UINT_PTR nIDEvent)
{
	try {
		switch (nIDEvent) {
		case IDT_DELAY_RELOAD_LABS_LIST:
		{
			// (r.gonet 09/02/2014) - PLID 63539 - We waited 100ms to get more tablecheckers before reloading
			// the labs list. Now we must reload it before the user takes notice of the delay.
			KillTimer(IDT_DELAY_RELOAD_LABS_LIST);
			m_bCoalescingTableCheckers = false;
			ReloadLabList();
		}
			break;
		}
	}NxCatchAll(__FUNCTION__);

	CNxDialog::OnTimer(nIDEvent);
}

// (r.gonet 09/03/2014) - PLID 63540 - Construct the CTableRefresher
CPatientLabsDlg::CTableRefresher::CTableRefresher()
{
	m_strSenderID = NewUUID(true);
}

// (r.gonet 09/03/2014) - PLID 63540 - Refreshes the labs table with the ability to tell recipients to ignore the message
// if it was sent from a particular sender ID (used to ignore tablecheckers sent to the dialog owning this refresher).
void CPatientLabsDlg::CTableRefresher::RefreshLabsTable(long nPersonID, long nLabID, BOOL bIgnoreSelf/* = FALSE*/)
{
	CClient::RefreshLabsTable(m_strSenderID, bIgnoreSelf, nPersonID, nLabID);
}

// (r.gonet 09/03/2014) - PLID 63540 - Returns the sender ID, which identifies this table refresher object.
CString CPatientLabsDlg::CTableRefresher::GetSenderID()
{
	return m_strSenderID;
}