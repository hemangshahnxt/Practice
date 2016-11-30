// LabFollowUpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LabFollowUpDlg.h"
#include "LabEntryDlg.h"
#include "AuditTrail.h"
#include "PatientsRc.h"
#include "dontshowdlg.h"
#include "NotesDlg.h"	// (j.dinatale 2011-01-05) - PLID 41818
#include "NxModalParentDlg.h"	// (j.dinatale 2011-01-05) - PLID 41818

// (r.galicki 2008-10-23 11:47) - PLID 27214
#include "Reports.h"
#include "GlobalReportUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLabFollowUpDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

#define ID_OPEN_NEXT_LAB	WM_USER + 42420

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

//Generates a comma seperated value string of the given array of longs...
CString GenerateCSV(CArray<long, long> &arID); 

// (j.gruber 2009-10-02 09:58) - PLID 35694 - added diagnosis
enum FollowUpListColumns {
	flLabID = 0,
	flFormNumberTextID,
	flPatientID,
	flPatientName,
	flUserDefinedID, //b.eyers 2014-01-17 - PLID 41608
	flProviderName,
	flLocationID,
	flLocationName, // this is hidden
	flInputDate,
	flBiopsyDate,
	flDateReceived,
	flAnatomyID,
	flAnatomicalLocation,
	flLabLocationID,
	flLabLocationName,
	flClinicalData,
	flResults,
	// (j.jones 2010-04-06 16:10) - PLID 37066 - split diagnosis into Initial Diagnosis and Final Diagnosis
	flInitialDiagnosis,
	flFinalDiagnosis,
	flStepOrder,
	flStep,
	flCurrentStepID,
	flDiscontinued, // (c.haag 2010-09-09 16:03) - PLID 40461
	flDiscontinuedDate, // (c.haag 2010-09-09 16:03) - PLID 40461
	flHasNotes,	// (j.dinatale 2011-01-04) - PLID 41818 - flag representing if a lab result/specimen has notes
	flNotesIcon, // (j.dinatale 2011-01-04) - PLID 41818 - the actual note icon associated with the flag (like the billing tab)
	flResultID,  // (j.dinatale 2011-01-04) - PLID 41818 - need the result id for the notes dialog
	flResultFlagPriority, //TES 8/1/2013 - PLID 57823
	flMAXCOLUMN
};

//(r.farnworth 2013-04-25) PLID 51115 - Column enums for the Procedure and Provider lists
enum FilterColumns {
	fcID = 0,
	fcName,
};

// (z.manning 2008-10-22 09:59) - PLID 31784 - The step filter now has its own enum for its columns.
enum StepFilterColumns
{
	sfcID = 0,
	sfcProcedureName,
	sfcName,
	sfcStepOrder,
	sfcProcedureID,
};


// (j.armen 2012-06-05 16:04 ) - PLID 50805 - Pass in a ConfigRT entry so that this dlg will save it's size
CLabFollowUpDlg::CLabFollowUpDlg(CWnd* pParent)
	: CNxDialog(CLabFollowUpDlg::IDD, pParent, "CLabFollowUpDlg")
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_nLastProviderID = -1;
	m_nLastStepID = -1;
	m_nLastProcedureID = -1;

	// (j.dinatale 2011-01-05) - PLID 41818
	m_hNotes = NULL;
}

// (j.dinatale 2011-01-05) - PLID 41818 
CLabFollowUpDlg::~CLabFollowUpDlg()
{
	// (j.dinatale 2011-01-05) - PLID 41818 - destroy the icon when the dialog is destroyed
	DestroyIcon((HICON)m_hNotes);
	m_hNotes = NULL;
}

void CLabFollowUpDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabFollowUpDlg)
	DDX_Control(pDX, IDOK, m_nxibOK);
	DDX_Control(pDX, IDC_REMEMBER_LAB_COLUMN_SETTINGS, m_nxbRememberColumns);
	DDX_Control(pDX, IDC_LABS_SHOWALL, m_nxibShowAll);
	DDX_Control(pDX, IDC_CHECK_SPECIMEN, m_chkSpecimenLabel);
	DDX_Control(pDX, IDC_PRINT_SPECIMEN, m_nxibPrintLabels);
	DDX_Control(pDX, IDC_CHECK_AUTO_OPEN_NEXT_LAB, m_checkAutoOpenNextLab);
	DDX_Control(pDX, IDC_CHECK_LABS_NO_RESULTS, m_chkLabNoResults);	
	DDX_Control(pDX, IDC_CHECK_SHOW_DISCONTINUED, m_checkDiscontinuedLabs);
	DDX_Control(pDX, IDC_LFU_REQUISITION, m_rdShowByReq);
	DDX_Control(pDX, IDC_LFU_RESULT, m_rdShowByResult);

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLabFollowUpDlg, CNxDialog)
	ON_BN_CLICKED(IDC_REMEMBER_LAB_COLUMN_SETTINGS, OnRememberColumnSettings)
	ON_COMMAND(ID_LABS_GOTOPATIENT, OnGotopatient)
	ON_COMMAND(ID_LABS_MARKCOMPLETE, OnLabsMarkcomplete)
	ON_COMMAND(ID_LABS_MARKSTEPCOMPLETE, OnLabsMarkstepcomplete)
	ON_COMMAND(ID_LABS_OPENLAB, OnLabsOpenlab)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_LABS_SHOWALL, OnLabsShowall)
	ON_BN_CLICKED(IDC_BTN_REFRESH, OnBtnRefresh)
	ON_BN_CLICKED(IDC_CHECK_SPECIMEN, &CLabFollowUpDlg::OnBnClickedCheckSpecimen)
	ON_BN_CLICKED(IDC_PRINT_SPECIMEN, &CLabFollowUpDlg::OnBnClickedPrintSpecimen)
	ON_WM_NCDESTROY()
	ON_BN_CLICKED(IDC_CHECK_AUTO_OPEN_NEXT_LAB, &CLabFollowUpDlg::OnCheckAutoOpenNextLab)
	ON_BN_CLICKED(IDC_CHECK_LABS_NO_RESULTS, &CLabFollowUpDlg::OnBnClickedCheckLabsNoResults)
	ON_MESSAGE(NXM_LAB_ENTRY_DLG_CLOSED, OnLabEntryDlgClosed)
	ON_BN_CLICKED(IDC_CHECK_SHOW_DISCONTINUED, &CLabFollowUpDlg::OnBnClickedCheckShowDiscontinued)
	ON_BN_CLICKED(IDC_LFU_REQUISITION, &CLabFollowUpDlg::OnBnClickedLfuRequisition)
	ON_BN_CLICKED(IDC_LFU_RESULT, &CLabFollowUpDlg::OnBnClickedLfuResult)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLabFollowUpDlg message handlers

void CLabFollowUpDlg::OnOK() 
{
	try {

		// (j.jones 2010-04-06 16:39) - PLID 38073 - save the column widths
		if(IsDlgButtonChecked(IDC_REMEMBER_LAB_COLUMN_SETTINGS)) {
			SaveColumns();
		}
		else {
			SetRemotePropertyText("DefaultLabFollowupColumnSizes", "", 0, GetCurrentUserName());
			SetRemotePropertyText("DefaultLabFollowupColumnSort", "", 0, GetCurrentUserName());
		}

		// (b.savon 2013-09-06 12:29) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
		SaveProcedureDefaults();

		// (j.jones 2015-08-27 14:30) - PLID 57837 - we now remember multiple steps per user
		SaveStepDefaults();

		// (v.maida 2016-04-14 9:14) - NX-100192 - Remember multiple providers.
		SaveProviderDefaults();

		::DestroyWindow(GetSafeHwnd()); 

	}NxCatchAll(__FUNCTION__);
}

void CLabFollowUpDlg::OnCancel()
{
	try {

		// (j.jones 2010-04-06 16:39) - PLID 38073 - save the column widths
		if(IsDlgButtonChecked(IDC_REMEMBER_LAB_COLUMN_SETTINGS)) {
			SaveColumns();
		}
		else {
			SetRemotePropertyText("DefaultLabFollowupColumnSizes", "", 0, GetCurrentUserName());
			SetRemotePropertyText("DefaultLabFollowupColumnSort", "", 0, GetCurrentUserName());
		}

		// (b.savon 2013-09-06 12:29) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
		SaveProcedureDefaults();

		// (j.jones 2015-08-27 14:30) - PLID 57837 - we now remember multiple steps per user
		SaveStepDefaults();

		// (v.maida 2016-04-14 9:14) - NX-100192 - Remember multiple providers.
		SaveProviderDefaults();

		::DestroyWindow(GetSafeHwnd()); 

	}NxCatchAll(__FUNCTION__);
}

BOOL CLabFollowUpDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_bDataChanged = false;
		/*
		colorHighlight = RGB(255, 235, 235);
		colorStandard = RGB(245, 245, 205);
		*/

		// (j.jones 2010-08-12 13:52) - PLID 37875 - cache preferences
		g_propManager.CachePropertiesInBulk("CLabFollowUpDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'LabColorLabIncomplete' "
			"OR Name = 'LabColorStepIncomplete' "
			"OR Name = 'LabFollowUp_AutoOpenNextLab' "
			"OR Name = 'RememberLabFollowupColumns' "
			"OR Name = 'ShowDiscontinuedLabsInLabFollowUp' " // (c.haag 2010-09-09 16:03) - PLID 40461
			"OR Name = 'LabFollowupShowList' " // (j.gruber 2010-12-22 16:09) - PLID 38740
			"OR Name = 'LabFollowupNoRecordsWarn' " // (j.gruber 2010-12-22 16:09) - PLID 38740
			"OR Name = 'LabNotesDefaultCategory' " // (j.dinatale 2011-01-06) - PLID 41818
			"OR Name = 'LabDefaultProviderFilter' " // (b.savon 2011-10-05 11:15) - PLID 45442
			"OR Name = 'LabDefaultStepFilter' " // (b.savon 2011-10-05 11:15) - PLID 45442
			")",
			_Q(GetCurrentUserName()));

		// (b.savon 2013-09-06 11:40) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
		g_propManager.CachePropertiesInBulk("CLabFollowUpDlg_Procedures", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'LabDefaultProcedureFilter' "
			"OR Name = 'LabDefaultStepsFilter' "	// (j.jones 2015-08-27 14:30) - PLID 57837 - we now remember multiple steps per user
			"OR Name = 'LabDefaultProvidersFilter' " // (v.maida 2016-04-14 9:14) - NX-100192 - Remember multiple providers.
			")",
			_Q(GetCurrentUserName()));

		// (a.walling 2006-07-19 15:24) - Get lab colors from the property manager
		colorHighlight = COLORREF(GetRemotePropertyInt("LabColorLabIncomplete", RGB(255, 253, 170), 0, GetCurrentUserName(), true));
		colorStandard = COLORREF(GetRemotePropertyInt("LabColorStepIncomplete", RGB(246, 215, 215), 0, GetCurrentUserName(), true));
		// and make sure the alternating color in the grid won't overflow the colorref (+/- 5)
		if (GetRValue(colorStandard) > 250) {
			colorStandard = RGB(250, GetGValue(colorStandard), GetBValue(colorStandard));
		}
		if (GetGValue(colorStandard) > 250) {
			colorStandard = RGB(GetRValue(colorStandard), 250, GetBValue(colorStandard));
		}
		if (GetBValue(colorStandard) > 250) {
			colorStandard = RGB(GetRValue(colorStandard), GetGValue(colorStandard), 250);
		}
		if (GetRValue(colorStandard) < 5) {
			colorStandard = RGB(5, GetGValue(colorStandard), GetBValue(colorStandard));
		}
		if (GetGValue(colorStandard) < 5) {
			colorStandard = RGB(GetRValue(colorStandard), 5, GetBValue(colorStandard));
		}
		if (GetBValue(colorStandard) < 5) {
			colorStandard = RGB(GetRValue(colorStandard), GetGValue(colorStandard), 5);
		}

		// (c.haag 2008-04-25 14:02) - PLID 29790 - NxIconify the OK (which is really Close) button
		m_nxibOK.AutoSet(NXB_CLOSE);

		// (r.galicki 2008-10-23 10:23) - PLID 27214 - Print specimen labels button
		m_nxibPrintLabels.AutoSet(NXB_PRINT);

		m_pLabList = BindNxDataList2Ctrl(IDC_LAB_LIST, false);

		// (a.walling 2008-04-15 14:17) - PLID 25755 - Load the provider list
		m_pProv = BindNxDataList2Ctrl(IDC_LIST_PROVIDERS, true);
		m_pProc = BindNxDataList2Ctrl(IDC_LIST_PROCEDURE, true);
		m_pStep = BindNxDataList2Ctrl(IDC_LIST_STEP, false);

		IRowSettingsPtr pRow = m_pProv->GetNewRow();
		pRow->PutValue(fcID, _variant_t((long)-1));
		pRow->PutValue(fcName, _variant_t(" <All Providers>"));
		m_pProv->AddRowSorted(pRow, NULL);

		// (v.maida 2016-04-13 12:58) - NX-100191 - Need to filter on multiple providers.
		pRow = m_pProv->GetNewRow();
		pRow->PutValue(fcID, _variant_t((long)-2));
		pRow->PutValue(fcName, _variant_t(" <Multiple Providers>"));
		m_pProv->AddRowSorted(pRow, NULL);

		pRow = m_pProc->GetNewRow();
		pRow->PutValue(fcID, _variant_t((long)-1));
		pRow->PutValue(fcName, _variant_t(" <All Procedures>"));
		m_pProc->AddRowSorted(pRow, NULL);

		//(r.farnworth 2013-03-18) - PLID 51115 Need to filter on Multiple Procedures
		pRow = m_pProc->GetNewRow();
		pRow->PutValue(fcID, _variant_t((long)-2));
		pRow->PutValue(fcName, _variant_t(" <Multiple Procedures>")); 
		m_pProc->AddRowSorted(pRow, NULL); 
		
		// (z.manning 2008-10-22 09:27) - PLID 31784 - This is always enabled now
		//GetDlgItem(IDC_LIST_STEP)->EnableWindow(FALSE);

		// (j.jones 2010-04-16 16:42) - PLID 37875 - added option to auto-open the next lab,
		// which just saves your last selection per user
		m_checkAutoOpenNextLab.SetCheck(GetRemotePropertyInt("LabFollowUp_AutoOpenNextLab", 0, 0, GetCurrentUserName(), true) == 1);
		
		if(GetRemotePropertyInt("RememberLabFollowupColumns", 0, 0, GetCurrentUserName(), true) == 1) {
			CheckDlgButton(IDC_REMEMBER_LAB_COLUMN_SETTINGS, true);
			RestoreColumns();
		}
		else {
			CheckDlgButton(IDC_REMEMBER_LAB_COLUMN_SETTINGS, false);
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(flPatientName);
			// (e.frazier 2016-06-01 16:13) - PLID-38147 - use csVisible|csWidthAuto when not loading a remembered width
			pCol->PutColumnStyle(csVisible | csWidthAuto);
			pCol->PutSortPriority(0);
			pCol->PutSortAscending(TRUE);
		}

		// (c.haag 2010-09-09 13:46) - PLID 40461 - Update the checkbox for discontinued labs
		if (GetRemotePropertyInt("ShowDiscontinuedLabsInLabFollowUp", 0, 0, GetCurrentUserName())) {
			m_checkDiscontinuedLabs.SetCheck(1);
		} else {
			m_checkDiscontinuedLabs.SetCheck(0);
		}

		// (b.savon 2013-09-06 13:03) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
		PopulateProcedureDefaults();

		// (v.maida 2016-04-14 9:00) - NX-100192 - Remember multiple providers.
		PopulateProviderDefaults();

		// (j.gruber 2010-12-23 12:29) - PLID 38740
		long nShowBy = GetRemotePropertyInt("LabFollowupShowList", 0,0, GetCurrentUserName());
		if (nShowBy == 0) {
			CheckDlgButton(IDC_LFU_REQUISITION, 1);
			CheckDlgButton(IDC_LFU_RESULT, 0);
			
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pLabList->GetColumn(flFormNumberTextID);
			if (pCol) {
				pCol->ColumnTitle = "Form";
			}
		}
		else {
			CheckDlgButton(IDC_LFU_REQUISITION, 0);
			CheckDlgButton(IDC_LFU_RESULT, 1);

			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pLabList->GetColumn(flFormNumberTextID);
			if (pCol) {
				pCol->ColumnTitle = "Form: Result Name";
			}
		}

		// (j.dinatale 2011-01-05) - PLID 41818 - need the Bill Notes icon to indicate that notes are on a lab/result
		m_hNotes = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16,16, 0);

		// (z.manning 2010-01-07 14:06) - PLID 36796 - ReloadList is called in OnShowWindow
		// so no need to call it again.
		//ReloadList();
	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLabFollowUpDlg::ReloadList()
{
	try {
		//setup queries

		// (j.gruber 2010-12-22 15:36) - PLID 38740 - moved where clause generation higher
		//Filter out all deleted labs and those that are complete
		// (c.haag 2010-12-13 12:10) - PLID 41806 - LabsT no longer has a CompletedDate field. We must look to the results.
		// A lab should appear if either of the following is true:
		// 1. The lab has no results.
		// 2. The lab has any results with a NULL completed date.
		// The subquery in the next statement effectively means "the lab is not completed"
		CString strWhere = " WHERE LabsT.Deleted = 0 AND (LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LabsT.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL))";
		// (c.haag 2010-09-09 13:46) - PLID 40461 - Apply the discontinued lab filter. Unchecked means we hide discontinued labs.
		if (!m_checkDiscontinuedLabs.GetCheck()) {
			strWhere += " AND LabsT.Discontinued = 0 ";	
		}

		// (a.walling 2008-04-15 16:26) - PLID 25755 - Filter on specific lab criteria
		IRowSettingsPtr pRow = m_pProv->GetCurSel();
		long nProviderID;
		if (pRow) {
			nProviderID = VarLong(pRow->GetValue(fcID), -1);
		} else {
			nProviderID = m_nLastProviderID;
		}

		if (nProviderID == -2) {
			// (v.maida 2016-04-13 13:06) - NX-100191 - Multiple providers option was selected.
			strWhere += FormatString(" AND LabsT.ID IN (SELECT LabID FROM LabMultiProviderT WHERE ProviderID IN (%s))", ArrayAsString(m_vecProvIDs));
		} else if (nProviderID >= 0) {
			// (z.manning 2010-01-07 13:29) - PLID 36795 - Use LabsT.ID instead of LabID so that we handle
			// labs without steps.
			strWhere += FormatString(" AND LabsT.ID IN (SELECT LabID FROM LabMultiProviderT WHERE ProviderID = %li)", nProviderID);
		}

		// (a.walling 2008-07-31 09:52) - PLID 25755 - Also refresh the provider list
		if (VARIANT_FALSE == m_pProv->IsRequerying()) {
			m_pProv->Requery();

			IRowSettingsPtr pRow = m_pProv->GetNewRow();
			pRow->PutValue(fcID, _variant_t((long)-1));
			pRow->PutValue(fcName, _variant_t(" <All Providers>"));
			m_pProv->AddRowSorted(pRow, NULL);

			// (v.maida 2016-04-13 13:06) - NX-100191 - Re-add multiple providers option.
			pRow = m_pProv->GetNewRow();
			pRow->PutValue(fcID, _variant_t((long)-2));
			pRow->PutValue(fcName, _variant_t(" <Multiple Providers>"));
			m_pProv->AddRowSorted(pRow, NULL);

			m_pProv->TrySetSelByColumn_Deprecated(fcID, nProviderID);
		}

		//(r.farnworth 04/23/2013) PLID 51115 - Added the ConcatenateWhere function to reduce duplicate code.
		pRow = m_pStep->GetCurSel();
		ConcatenateWhere(pRow, (int)sfcID, m_StepIDs, "CurrentStepQ.LabProcedureStepID", strWhere);
		
		pRow = m_pProc->GetCurSel();
		ConcatenateWhere(pRow, (int)fcID, m_ProcIDs, "LabsT.LabProcedureID", strWhere);

		// (r.galicki 2008-10-22 16:45) - PLID 27214
		if(m_chkSpecimenLabel.GetCheck()) {
			strWhere += " AND LabsT.HasPrintedLabel = 0";
		}

		// (j.gruber 2010-06-08 12:32) - PLID 36844
		if (m_chkLabNoResults.GetCheck()) {
			strWhere += " AND LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE DELETED = 0) ";
		}

		//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
		strWhere += " AND " + GetAllowedLocationClause("LabsT.LocationID");


		// (z.manning 2008-10-09 12:11) - PLID 31633 - LabProcedureID is now in LabsT (instead of LabStepsT)
		// (j.gruber 2008-10-10 09:57) - PLID 31432 - changed for new result data structure
		//TES 11/16/2009 - PLID 36260 - Added AnatomyQualifiersT
		// (j.gruber 2010-12-22 15:07) - PLID 38740 - show by reqs or results
		// (j.dinatale 2011-01-05) - PLID 41818 - Added the HasNotes and LabResultID fields to both specimen and result queries
		//		NOTE: LabResultID will be NULL if in the requisitions view.
		//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
		// (r.gonet 03/07/2013) - PLID 55492 - Added LOINC code and description (universal service identifier) to the AnatLocToBeOrdered column.
		//TES 8/1/2013 - PLID 57823 - Added FlagPriority. Results without a flag have the highest priority, meaning they're sorted last. Requisitions
		// with multiple flags are given the priority of the "first" flag in their list, in ascending order.
		//b.eyers 2014-01-17 - PLID 41608 - Added UserDefinedID so it can be displayed
		CString strFrom;
		if (IsDlgButtonChecked(IDC_LFU_REQUISITION)) {
			strFrom.Format(
				" (SELECT LabsT.ID as LabID, "
				" LabsT.FormNumberTextID + CASE WHEN LabsT.Specimen IS NULL THEN '' ELSE '-' + LabsT.Specimen END as FormNumber, "
				" LabsT.PatientID, "
				" Persont.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatientName, "
				" PatientsT.UserDefinedID, "
				" dbo.GetLabProviderString(LabsT.ID) as ProviderName, "
				" LabsT.LocationID, "
				" PracLocT.Name as LocationName, "
				" LabsT.InputDate, "
				" LabsT.BiopsyDate, "
				" CASE WHEN (SELECT Count(*) FROM LabResultsT WHERE Deleted = 0 and LabResultsT.LabID = LabsT.ID) > 1 THEN '<Multiple>' ELSE (SELECT CONVERT(nVarChar, DateReceived, 101) FROM LabResultsT WHERE deleted = 0 and LabResultsT.LabID = LabsT.ID) END as DateReceived, "
				" LabAnatomyT.ID as AnatomyID, "
				" LTRIM(RTRIM(CASE WHEN Type = 1 THEN LabAnatomicLocationQ.AnatomicLocation ELSE ToBeOrdered END + ' ' + "
				" CASE WHEN (LabsT.LOINC_Code IS NOT NULL AND LabsT.LOINC_CODE <> '') AND (LabsT.LOINC_Description IS NOT NULL AND LabsT.LOINC_Description <> '') THEN '(' + LabsT.LOINC_Code + ' - ' + LabsT.LOINC_Description + ')' "
				"	  WHEN (LabsT.LOINC_Code IS NOT NULL AND LabsT.LOINC_CODE <> '') AND (LabsT.LOINC_Description IS NULL OR LabsT.LOINC_Description = '') THEN '(' + LabsT.LOINC_Code + ')' "
				"	  WHEN (LabsT.LOINC_Code IS NULL OR LabsT.LOINC_CODE = '') AND (LabsT.LOINC_Description IS NOT NULL AND LabsT.LOINC_Description <> '') THEN '(' + LabsT.LOINC_Description + ')' "
				"	  ELSE '' "
				"END)) as AnatLocToBeOrdered, "
				" LabsT.LabLocationID, "
				" LabLocT.Name as LabLocName, "
				" CONVERT(nvarchar(4000), ClinicalData) as ClinicalData, "
				" dbo.GetLabResultFlagString(LabsT.ID) as FlagName, "
				" COALESCE((SELECT Min(LabResultFlagsT.Priority) FROM LabResultFlagsT INNER JOIN LabResultsT ON LabResultFlagsT.ID = LabResultsT.FlagID "
				"	WHERE LabResultsT.LabID = LabsT.ID AND LabResultsT.Deleted = 0),2147483647) AS FlagPriority, "
				" LabsT.InitialDiagnosis, "
				" dbo.GetLabFinalDiagnosisList(LabsT.ID) as DiagnosisDesc, "
				" CurrentStepQ.StepOrder, "
				" COALESCE(CurrentStepQ.Name, 'Pending') AS CurrentStepName, "
				" CurrentStepQ.CurrentStep, "
				" LabsT.Discontinued, "
				" LabsT.DiscontinuedDate, "
				" CASE WHEN NotesQ.LabID Is NOT NULL THEN 1 ELSE 0 END AS HasNotes, "
				" NULL AS LabResultID "
				" FROM "
				"			LabsT"
				" LEFT JOIN PersonT"
				"	ON LabsT.PatientID = PersonT.ID"
				" LEFT JOIN PatientsT"
				"	ON PersonT.ID = PatientsT.PersonID"
				" LEFT JOIN LabAnatomyT"
				"	ON LabsT.AnatomyID = LabAnatomyT.ID"
				" LEFT JOIN AnatomyQualifiersT "
				"   ON LabsT.AnatomyQualifierID = AnatomyQualifiersT.ID "
				" LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID "
				" LEFT JOIN LocationsT LabLocT"
				"	ON LabsT.LabLocationID = LabLocT.ID"
				" LEFT JOIN LocationsT PracLocT"
				"	ON LabsT.LocationID = PracLocT.ID"
				" LEFT JOIN (SELECT DISTINCT LabID FROM Notes) NotesQ"
				"	ON LabsT.ID = NotesQ.LabID"
				" LEFT JOIN"
				"	(SELECT NextStepOrder AS StepOrder, LabID, StepID AS CurrentStep,"
				"    Name, LabProcedureStepID FROM"
				"		(SELECT"
				"			MIN(StepOrder) AS NextStepOrder,"
				"			LabID AS CurrentLabID"
				"		FROM LabStepsT"
				"		WHERE"
				"			StepCompletedDate IS NULL "
				"		GROUP BY LabID) NextStepOrderQ"
				"	LEFT JOIN LabStepsT"
				"		ON NextStepOrderQ.CurrentLabID = LabStepsT.LabID"
				"		WHERE NextStepOrderQ.NextStepOrder = LabStepsT.StepOrder) CurrentStepQ"
				"	ON LabsT.ID = CurrentStepQ.LabID %s) SubQ", strWhere);

		}
		else {			

			//only show non-completed results
			strWhere += " and LabResultsT.ResultCompletedBy IS NULL AND LabResultsT.Deleted = 0 ";

			// (j.armen 2013-08-20 14:34) - PLID 58136 - Use INNER JOIN where appropriate
			strFrom.Format(
				" (SELECT LabsT.ID as LabID, "
				" LabsT.FormNumberTextID + CASE WHEN LabsT.Specimen IS NULL THEN '' ELSE '-' + LabsT.Specimen END  + CASE WHEN LabResultsT.Name = '' THEN '' ELSE ':' + LabResultsT.Name END as FormNumber, "
				" LabsT.PatientID, "
				" Persont.Last + ', ' + PersonT.First + ' ' + PersonT.Middle as PatientName, "
				" PatientsT.UserDefinedID, "
				" dbo.GetLabProviderString(LabsT.ID) as ProviderName, "
				" LabsT.LocationID, "
				" PracLocT.Name as LocationName, "
				" LabsT.InputDate, "
				" LabsT.BiopsyDate, "
				" CONVERT(nVarChar, LabResultsT.DateReceived, 101) as DateReceived, "
				" LabAnatomyT.ID as AnatomyID, "
				" LTRIM(RTRIM(CASE WHEN Type = 1 THEN LabAnatomicLocationQ.AnatomicLocation ELSE ToBeOrdered END + ' ' + "
				" CASE WHEN (LabsT.LOINC_Code IS NOT NULL AND LabsT.LOINC_CODE <> '') AND (LabsT.LOINC_Description IS NOT NULL AND LabsT.LOINC_Description <> '') THEN '(' + LabsT.LOINC_Code + ' - ' + LabsT.LOINC_Description + ')' "
				"	  WHEN (LabsT.LOINC_Code IS NOT NULL AND LabsT.LOINC_CODE <> '') AND (LabsT.LOINC_Description IS NULL OR LabsT.LOINC_Description = '') THEN '(' + LabsT.LOINC_Code + ')' "
				"	  WHEN (LabsT.LOINC_Code IS NULL OR LabsT.LOINC_CODE = '') AND (LabsT.LOINC_Description IS NOT NULL AND LabsT.LOINC_Description <> '') THEN '(' + LabsT.LOINC_Description + ')' "
				"	  ELSE '' "
				"END)) as AnatLocToBeOrdered, "
				" LabsT.LabLocationID, "
				" LabLocT.Name as LabLocName, "
				" CONVERT(nvarchar(4000), ClinicalData) as ClinicalData, "
				" LabResultFlagsT.Name as FlagName, "
				" COALESCE(LabResultFlagsT.Priority,2147483647) AS FlagPriority, "
				" LabsT.InitialDiagnosis, "
				" Convert(nvarchar(4000), LabResultsT.DiagnosisDesc) as DiagnosisDesc, "
				" CurrentStepQ.StepOrder, "
				" COALESCE(CurrentStepQ.Name, 'Pending') AS CurrentStepName, "
				" CurrentStepQ.CurrentStep, "
				" LabsT.Discontinued, "
				" LabsT.DiscontinuedDate, "
				" CASE WHEN NotesQ.LabResultID Is NOT NULL THEN 1 ELSE 0 END AS HasNotes, "
				" ResultID AS LabResultID "
				"FROM LabResultsT\r\n"
				"INNER JOIN LabsT ON LabResultsT.LabID = LabsT.ID\r\n"
				"LEFT JOIN LabResultFlagsT ON LabResultsT.FlagID = LabResultFlagsT.ID\r\n"
				"LEFT JOIN PersonT ON LabsT.PatientID = PersonT.ID\r\n"
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID\r\n"
				"LEFT JOIN LabAnatomyT ON LabsT.AnatomyID = LabAnatomyT.ID\r\n"
				"LEFT JOIN AnatomyQualifiersT ON LabsT.AnatomyQualifierID = AnatomyQualifiersT.ID\r\n"
				"LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID\r\n"
				"INNER JOIN LocationsT LabLocT ON LabsT.LabLocationID = LabLocT.ID\r\n"
				"INNER JOIN LocationsT PracLocT ON LabsT.LocationID = PracLocT.ID\r\n"
				" LEFT JOIN (SELECT DISTINCT LabResultID FROM Notes) NotesQ"
				"	ON LabResultsT.ResultID = NotesQ.LabResultID"
				" LEFT JOIN"
				"	(SELECT NextStepOrder AS StepOrder, LabID, StepID AS CurrentStep,"
				"    Name, LabProcedureStepID FROM"
				"		(SELECT"
				"			MIN(StepOrder) AS NextStepOrder,"
				"			LabID AS CurrentLabID"
				"		FROM LabStepsT"
				"		WHERE"
				"			StepCompletedDate IS NULL "
				"		GROUP BY LabID) NextStepOrderQ"
				"	LEFT JOIN LabStepsT"
				"		ON NextStepOrderQ.CurrentLabID = LabStepsT.LabID"
				"		WHERE NextStepOrderQ.NextStepOrder = LabStepsT.StepOrder) CurrentStepQ"
				"	ON LabsT.ID = CurrentStepQ.LabID %s) SubQ", strWhere);
		}

		m_pLabList->FromClause = _bstr_t((LPCTSTR)strFrom);

		//Requery the datalist to load all the records
		m_pLabList->Requery();

	} NxCatchAll("Error in ReloadList()");
}

void CLabFollowUpDlg::OnRememberColumnSettings()
{
	long nSaveColumns;

	try {
		SaveColumns();

		if(IsDlgButtonChecked(IDC_REMEMBER_LAB_COLUMN_SETTINGS))
		{
			nSaveColumns = 1;
			RestoreColumns();
		}
		else
		{
			nSaveColumns = 0;
			MessageBox("Your column settings will be reset the next time Practice is opened.");
		}
		SetRemotePropertyInt("RememberLabFollowupColumns", nSaveColumns, 0, GetCurrentUserName());
	} NxCatchAll("Error in OnRememberColumnSettings()");
}

void CLabFollowUpDlg::SaveColumns()
{
	CString strColumnWidths, strColumnSort;
	try {
		// (a.walling 2006-07-11 16:33) - PLID 21073 - Store the columns in a xx,xx,xx,xx format
		for (int i = 0; i < m_pLabList->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(i);
			CString str;
			
			
			str.Format("%d", pCol->StoredWidth);
			
			if (i > 0) {
				strColumnWidths += ",";
				strColumnSort += ",";
			}

			strColumnWidths += str;

			// (j.dinatale 2011-01-10) - PLID 42062 - there is the potential that the sort priority of the Initial Diag. column can be something other than -1.
			//		That is bad because at the present time the initial diag column is of ntext type so if the user ever sorted by that column, we should exclude
			//		it from our sort priority save string. This will prevent the dialog from becoming unusable in the future and is only a temporary "fix". By setting
			//		one of the sort priorities to -1, it may break the sequence of sort orders (i.e. 0,1,3,4,5 - hence the missing 2) but the datalist will then just sort
			//		by the first visible column.
			if(i != flInitialDiagnosis){
				// (a.walling 2008-04-15 11:45) - PLID 25755 - Save sort criteria
				BOOL bAscending = (VARIANT_TRUE == pCol->GetSortAscending()) ? TRUE : FALSE;
				short nSortPriority = pCol->GetSortPriority();

				DWORD dwValue = MAKELONG(nSortPriority, bAscending);
				str.Format("%lu", dwValue);
			}else{
				DWORD dwValue = MAKELONG(-1, TRUE);
				str.Format("%lu", dwValue);
			}

			strColumnSort += str;
		}

		SetRemotePropertyText("DefaultLabFollowupColumnSizes", strColumnWidths, 0, GetCurrentUserName());
		SetRemotePropertyText("DefaultLabFollowupColumnSort", strColumnSort, 0, GetCurrentUserName());
	} NxCatchAll("Error in SaveColumns()");
}

void CLabFollowUpDlg::RestoreColumns()
{
	try {
		CArray<int, int> arWidths;
		CArray<DWORD, DWORD> arSorts;
		BOOL bRestoreSort = TRUE;

		{
			CString strColumnWidths = GetRemotePropertyText("DefaultLabFollowupColumnSizes", "", 0, GetCurrentUserName(), false);

			int tokIndex = strColumnWidths.Find(',');

			if (tokIndex == -1) {
				// (a.walling 2006-07-11 16:33) - PLID 21073 - It is empty or invalid, so rebuild
				SaveColumns();
				return;
			}

			while(tokIndex != -1) {
				CString str = strColumnWidths.Left(tokIndex);
				arWidths.Add(atoi(str));
				strColumnWidths = strColumnWidths.Right(strColumnWidths.GetLength() - (tokIndex + 1));
				tokIndex = strColumnWidths.Find(',');
			}
			arWidths.Add(atoi(strColumnWidths));

			if (arWidths.GetSize() != m_pLabList->ColumnCount) {
				ASSERT(FALSE); // (a.walling 2008-08-20 16:59) - PLID 25755 - This can be OK if the dialog columns have changed
				// (a.walling 2006-07-11 16:33) - PLID 21073 - It is inconsistent, so rebuild

				SaveColumns();
				return;
			}
		}

		// (a.walling 2008-04-15 11:42) - PLID 25755 - Save/restore sorts
		{
			CString strColumnSort = GetRemotePropertyText("DefaultLabFollowupColumnSort", "", 0, GetCurrentUserName(), false);

			int tokIndex = strColumnSort.Find(',');

			if (tokIndex == -1) {
				bRestoreSort = FALSE;
			} else {
				while(tokIndex != -1) {
					CString str = strColumnSort.Left(tokIndex);
					arSorts.Add(atoi(str));
					strColumnSort = strColumnSort.Right(strColumnSort.GetLength() - (tokIndex + 1));
					tokIndex = strColumnSort.Find(',');
				}
				arSorts.Add(atoi(strColumnSort));
			}
			
			if (arSorts.GetSize() != m_pLabList->ColumnCount) {
				ASSERT(FALSE); // (a.walling 2008-08-20 16:59) - PLID 25755 - This can be OK if the dialog columns have changed
				bRestoreSort = FALSE;
			}
		}

		for (int i = 0; i < m_pLabList->ColumnCount; i++)
		{
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(i);
			// (e.frazier 2016-06-01 16:13) - PLID-38147 - set the widths to be simply csVisible to avoid forcing a stored width
			pCol->PutColumnStyle(csVisible);
			pCol->PutStoredWidth(arWidths[i]);
			
			if (bRestoreSort) {
				short nSortPriority = LOWORD(arSorts[i]);
				BOOL bAscending = HIWORD(arSorts[i]);

				pCol->PutSortPriority(nSortPriority);
				pCol->PutSortAscending(bAscending ? VARIANT_TRUE : VARIANT_FALSE);
			}
		}

		if (!bRestoreSort) {
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(flPatientName);
			pCol->PutSortPriority(0);
			pCol->PutSortAscending(TRUE);
		}
	} NxCatchAll("Error in RestoreColumns()");
}

BEGIN_EVENTSINK_MAP(CLabFollowUpDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLabFollowUpDlg)
	ON_EVENT(CLabFollowUpDlg, IDC_LAB_LIST, 18 /* RequeryFinished */, OnRequeryFinishedLabList, VTS_I2)
	ON_EVENT(CLabFollowUpDlg, IDC_LAB_LIST, 6 /* RButtonDown */, OnRButtonDownLabList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLabFollowUpDlg, IDC_LAB_LIST, 3 /* DblClickCell */, OnDblClickCellLabList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CLabFollowUpDlg, IDC_LAB_LIST, 19 /* LeftClick */, OnLeftClickLabList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CLabFollowUpDlg, IDC_LAB_LIST, 23 /* ChangeColumnSortFinished */, OnChangeColumnSortFinishedLabList, VTS_I2 VTS_BOOL VTS_I2 VTS_BOOL)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_PROVIDERS, 16 /* SelChosen */, OnSelChosenListProviders, VTS_DISPATCH)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_STEP, 16 /* SelChosen */, OnSelChosenListStep, VTS_DISPATCH)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_PROCEDURE, 16 /* SelChosen */, OnSelChosenListProcedure, VTS_DISPATCH)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_PROVIDERS, 20 /* TrySetSelFinished */, OnTrySetSelFinishedListProviders, VTS_I4 VTS_I4)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_PROVIDERS, 1 /* SelChanging */, OnSelChangingListProviders, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_PROCEDURE, 1 /* SelChanging */, OnSelChangingListProcedure, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabFollowUpDlg, IDC_LIST_STEP, 1 /* SelChanging */, OnSelChangingListStep, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLabFollowUpDlg, IDC_LAB_LIST, 17, OnColumnClickingLabList, VTS_I2 VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


//TES 4/8/2009 - PLID 33376 - Moved this macro (which wasn't used in this file anyway) to GlobalUtils.h
/*#define FOR_ALL_ROWS(datalist)	IRowSettingsPtr pRow = datalist->GetFirstRow();\
	CArray<LPDISPATCH,LPDISPATCH> arParentRows;\
	while(pRow) */

void CLabFollowUpDlg::OnRequeryFinishedLabList(short nFlags) 
{
	ColorLabList(); // this function will highlight labs that have all steps completed but need
			// a priveleged user (or someone) to mark the lab as complete

	// (j.dinatale 2011-01-04) - PLID 41818 - need to set up the note icon for each row
	SetupNoteIcons();
}

// (j.dinatale 2011-01-04) - PLID 41818 - sets up the note icon on the list for each row
void CLabFollowUpDlg::SetupNoteIcons()
{
	try{
		// get the first row
		IRowSettingsPtr pRow = m_pLabList->GetFirstRow();

		while(pRow){
			// pull the HasNotes field from the list
			BOOL bHasNotes = VarLong(pRow->GetValue(flHasNotes), FALSE);

			// set the row icon accordingly
			SetRowNoteIcon(pRow, bHasNotes);

			// next row...
			pRow = pRow->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__);
}

void CLabFollowUpDlg::OnRButtonDownLabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL)
			return;

		m_pLabList->CurSel = pRow;

		CPoint pt;
		GetCursorPos(&pt);
		
		CMenu mnu;
		CMenu* pSubMenu;
		mnu.LoadMenu(IDR_LABS_POPUP);
		pSubMenu = mnu.GetSubMenu(4);

		// (a.walling 2006-07-12 15:33) - PLID 21073
		// If they can't write, they can't open or mark anything.
		// If they can write they can open and mark steps complete, but not labs.

		_variant_t vStepID = pRow->GetValue(flCurrentStepID);
		if ( (vStepID.vt == VT_NULL) || (vStepID.vt == VT_EMPTY) )
		{	// all steps are complete but the lab is not
			pSubMenu->EnableMenuItem(ID_LABS_MARKSTEPCOMPLETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite, FALSE, 0, TRUE)) {
			pSubMenu->EnableMenuItem(ID_LABS_OPENLAB, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			pSubMenu->EnableMenuItem(ID_LABS_MARKCOMPLETE, MF_BYCOMMAND | MF_GRAYED);
			pSubMenu->EnableMenuItem(ID_LABS_MARKSTEPCOMPLETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		} else {
			if(!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic0, FALSE, 0, TRUE)) {
				pSubMenu->EnableMenuItem(ID_LABS_MARKCOMPLETE, MF_BYCOMMAND | MF_GRAYED);
			}
		}

		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	} NxCatchAll("Error in OnRButtonDownLabList()");
}

void CLabFollowUpDlg::OnDblClickCellLabList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		//DRT 7/7/2006 - PLID 21088 - Check permissions for write
		if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
			return;

		IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL)
			return;

		m_pLabList->PutCurSel(pRow);
		// (j.jones 2010-04-16 16:32) - PLID 37875 - this code was duplicated from
		// OnLabsOpenlab when it should have just called that function, as it does now
		OnLabsOpenlab();

	} NxCatchAll("Error in OnDblClickCellLabList()");
}

void CLabFollowUpDlg::OnGotopatient() 
{
	try {
		IRowSettingsPtr pRow(m_pLabList->GetCurSel());

		long nPatientID = VarLong(pRow->GetValue(flPatientID));

		GotoPatient(nPatientID);

	}NxCatchAll("Error in OnGotoPatient()");
}

void CLabFollowUpDlg::OnLabsMarkcomplete() 
{
	try {
		if (!CheckCurrentUserPermissions(bioPatientLabs, sptDynamic0)) {
			return;
		}

		IRowSettingsPtr pRow(m_pLabList->GetCurSel());

		if (pRow == NULL)
			return;

		long nLabID = VarLong(pRow->GetValue(flLabID));
		long nPatientID = pRow->GetValue(flPatientID);

		// (z.manning 2008-10-31 10:35) - PLID 21082 - We now require a signature to mark a lab complete.
		if(!PromptToSignAndMarkLabComplete(this, nLabID)) {
			return;
		}

		// any tracking code should go here
		CString strSql = BeginSqlBatch();
		// (m.hancock 2006-07-24 11:18) - PLID 21582 - Find the steps that are to be audited
		//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
		_RecordsetPtr prsSteps = CreateRecordset("SELECT StepID FROM LabStepsT WHERE LabID = %li AND StepCompletedDate IS NULL ", nLabID);
		//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
		AddStatementToSqlBatch(strSql, "UPDATE LabStepsT SET StepCompletedDate = '%s', StepCompletedBy = %li WHERE LabID = %li AND StepCompletedDate IS NULL", FormatDateTimeForSql(COleDateTime::GetCurrentTime()), GetCurrentUserID(), nLabID);
		ExecuteSqlBatch(strSql);
		// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a LabsT-level table checker because we updated lab steps
		// (r.gonet 08/25/2014) - PLID 63221 - Send the patient ID as well
		CClient::RefreshLabsTable(nPatientID, nLabID);

		// (z.manning 2008-10-13 16:37) - PLID 31667 - Update todos for this lab
		//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
		SyncTodoWithLab(nLabID, nPatientID);

		// (m.hancock 2006-07-24 11:18) - PLID 21582 - Audit each step that was marked complete
		while(!prsSteps->eof)
		{
			long nAuditID = BeginNewAuditEvent();
			//Get the StepID
			long nStepID = AdoFldLong(prsSteps, "StepID");
			//Generate a string containing the lab's description and the step name
			CString strOld = GenerateStepCompletionAuditOld(nStepID);
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditID, aeiPatientLabStepMarkedComplete, nStepID, strOld, "<Completed>", aepMedium, aetChanged);
			prsSteps->MoveNext();
		}

		m_bDataChanged = true;

		m_pLabList->RemoveRow(pRow);
	} NxCatchAll("Error in OnLabsMarkcomplete()");
}

void CLabFollowUpDlg::OnLabsMarkstepcomplete() 
{
	//(e.lally 2008-06-02) PLID 27579 - Declare an ID to use for an audit transaction
	long nAuditTransID = -1;
	try {
		if (!CheckCurrentUserPermissions(bioPatientLabs, sptWrite)) {
			return;
		}

		bool bLinkedLabs = false;
		IRowSettingsPtr pRow(m_pLabList->GetCurSel());

		if (pRow == NULL)
			return;

		CString strSql = BeginSqlBatch();		

		long nLabID = VarLong(pRow->GetValue(flLabID));
		long nStepID = VarLong(pRow->GetValue(flCurrentStepID));
		long nPatientID = pRow->GetValue(flPatientID);

		BOOL bNeedToCreateLadder = FALSE;

		///////////////////////////////
		// Taken from PatientLabsDlg
		//Check if there are other steps for specimens having this same form number
		// (c.haag 2013-08-29) - PLID 58216 - We now invoke this from a utility function
		_RecordsetPtr prsLinkedSteps = GetCompletableLinkedSteps(nStepID, false);

		CArray<long, long> arLinkedLabs; // does not include completed labs
		CArray<long, long> arLabsToComplete;
		CArray<long, long> arStepsToAudit; // (m.hancock 2006-07-24 11:06) - PLID 21582 - Keep track of the steps we are modifying for auditing purposes
		int i;

		//If we have results from the query, ask the user if they wish to also update the other 
		//specimens with this same form number
		if(!prsLinkedSteps->eof) {

			// (m.hancock 2006-07-26 14:06) - PLID 21635 - Generate a descriptive message about which steps are linked
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
				//Go back to the first record in the query
				prsLinkedSteps->MoveFirst();
				//Loop through the query results and add the step IDs to the list to be updated
				while(!prsLinkedSteps->eof) {
					// (a.walling 2006-07-13 12:47) - PLID 21073 Modified to fit this dialog
					// and made sure that no already-complete steps are being restamped with a new time/user.
					bLinkedLabs = true;
					//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
					AddStatementToSqlBatch(strSql,
						"UPDATE LabStepsT SET StepCompletedDate = '%s', StepCompletedBy = %li WHERE StepID = %li AND StepCompletedDate IS NULL",
						FormatDateTimeForSql(COleDateTime::GetCurrentTime()), GetCurrentUserID(),
						AdoFldLong(prsLinkedSteps, "StepID") );
					arLinkedLabs.Add(AdoFldLong(prsLinkedSteps, "LabID"));
					// (m.hancock 2006-07-24 11:06) - PLID 21582 - Keep track of the steps we are modifying for auditing purposes
					arStepsToAudit.Add(AdoFldLong(prsLinkedSteps, "StepID"));
					prsLinkedSteps->MoveNext();
				}
			}
		}

		///////////////////////////////

		AddStatementToSqlBatch(strSql, "UPDATE LabStepsT SET StepCompletedDate = '%s', StepCompletedBy = %li WHERE StepID = %li", FormatDateTimeForSql(COleDateTime::GetCurrentTime()), GetCurrentUserID(), nStepID);
		arLinkedLabs.Add(nLabID); // add the current lab as well, since it is not returned
			// in the query above

		ExecuteSqlBatch(strSql);
		// (c.haag 2010-07-21 11:40) - PLID 30894 - Send a LabsT-level table checker because we updated lab steps
		// (r.gonet 08/25/2014) - PLID 63221 - Send a table checker for just the patient that changed
		CClient::RefreshLabsTable(nPatientID, -1);

		// (z.manning 2008-10-13 16:37) - PLID 31667 - Update todos for this lab
		//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
		//TES 6/23/2014 - PLID 60708 - Update all the labs that were modified
		foreach(long nUpdatedLabID, arLinkedLabs) {
			SyncTodoWithLab(nUpdatedLabID, nPatientID);
		}

		// (m.hancock 2006-07-24 11:05) - PLID 21582 - Audit step completion
		//Add the current step to the list of steps to be audited
		arStepsToAudit.Add(nStepID);
		//Audit the steps
		for (i = 0; i < arStepsToAudit.GetSize(); i++) {
			if (arStepsToAudit[i] != -1) {
				//(e.lally 2008-06-02) PLID 27579 - Check for existing audit transaction or begin a new one.
				if(nAuditTransID == -1){
					nAuditTransID = BeginAuditTransaction();
				}
				//Generate a string containing the lab's description and the step name
				CString strOld = GenerateStepCompletionAuditOld(arStepsToAudit[i]);
				AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTransID, aeiPatientLabStepMarkedComplete, arStepsToAudit[i], strOld, "<Completed>", aepMedium, aetChanged);
			}
		}
		//(e.lally 2008-06-02) PLID 27579 - We used to commit the individual audit events, now let's commit the transacted audit.
		if(nAuditTransID != -1){
			CommitAuditTransaction(nAuditTransID);
			//reset our ID so we can do another transaction
			nAuditTransID = -1;
		}

		// now we have to find out if we just completed any labs, and try to mark them as complete if possible
		// and also update the datalist with new info

		CString strLinkedLabs;
		if (bLinkedLabs)
			strLinkedLabs = GenerateCSV(arLinkedLabs);
		else
			strLinkedLabs.Format("%li", nLabID);

		CString strNextStep;
		//TES 1/5/2011 - PLID 42006 - StepCompletedBy is no longer a reliable indicator of completion.
		strNextStep.Format(
			" SELECT LabID, LabStepsT.StepID AS NextStepID, LabStepsT.Name, NextStepOrder AS StepOrder, CreateLadder FROM"
			"	(SELECT"
			"		MIN(StepOrder) AS NextStepOrder,"
			"		LabID AS CurrentLabID"
			"	FROM LabStepsT"
			"	WHERE"
			"		StepCompletedDate IS NULL AND"
			"		LabID IN (%s)"
			"	GROUP BY LabID) NextStepOrderQ"
			" LEFT JOIN LabStepsT"
			"	ON NextStepOrderQ.CurrentLabID = LabStepsT.LabID"
			" LEFT JOIN LabProcedureStepsT ON LabStepsT.LabProcedureStepID = LabProcedureStepsT.StepID "
			"	WHERE NextStepOrderQ.NextStepOrder = LabStepsT.StepOrder", strLinkedLabs);

		// update labs in the datalist
		_RecordsetPtr prs = CreateRecordset(strNextStep);		
		if (!bLinkedLabs) { // only one lab to update
			if (!prs->eof) { 
				pRow->PutValue(flCurrentStepID, prs->Fields->Item["NextStepID"]->Value);
				pRow->PutValue(flStep, prs->Fields->Item["Name"]->Value);
				pRow->PutValue(flStepOrder, prs->Fields->Item["StepOrder"]->Value);
				//clear out the linked labs list, since it will refer to itself
				arLinkedLabs.RemoveAll();

				// (z.manning 2008-10-20 15:18) - PLID 31371 - See if the next step is to create a ladder
				if(AdoFldBool(prs->GetFields(), "CreateLadder")) {
					bNeedToCreateLadder = TRUE;
				}
			}
			else { // no rows? that was the last step
				arLabsToComplete.Add(nLabID);
			}
		}
		else { // we need to update all of them
			while (!prs->eof) {
				long nLinkedLabID = AdoFldLong(prs, "LabID");

				IRowSettingsPtr pUpdateRow = m_pLabList->FindByColumn(flLabID, prs->Fields->Item["LabID"]->Value, NULL, false);

				if (pUpdateRow) {
					pUpdateRow->PutValue(flCurrentStepID, prs->Fields->Item["NextStepID"]->Value);
					pUpdateRow->PutValue(flStep, prs->Fields->Item["Name"]->Value);
					pUpdateRow->PutValue(flStepOrder, prs->Fields->Item["StepOrder"]->Value);

					for (i = 0; i < arLinkedLabs.GetSize(); i++) {
						if (arLinkedLabs[i] == nLinkedLabID) {
							// this means there are still steps remaining
							// we nullify it from this array so we can add
							// the leftovers to arLabsToComplete
							arLinkedLabs[i] = -1;
						}
					}
				}

				prs->MoveNext();
			}
		}


		// now it is time to complete the labs if possible, or mark them if not.

		// first add all updatable lab rows into our array
		for (i = 0; i < arLinkedLabs.GetSize(); i++) {
			long nLinkedLabID = arLinkedLabs.GetAt(i);
			if (nLinkedLabID != -1 && !IsIDInArray(nLinkedLabID, arLabsToComplete)) {
				arLabsToComplete.Add(arLinkedLabs[i]);
			}
		}

		if (arLabsToComplete.GetSize() > 0) {
			if(!(GetCurrentUserPermissions(bioPatientLabs) & sptDynamic0)) {
				//no permission to mark complete.
				AfxMessageBox("You have completed the last step in this lab, however you do not have permission to mark the entire lab complete.  The lab "
					"will remain incomplete until an administrator reviews it.");

				//mark them as a lab to be completed
				_variant_t vtNull;
				vtNull.vt = VT_NULL;

				for (i = 0; i < arLabsToComplete.GetSize(); i++) {
					IRowSettingsPtr pUpdateRow = m_pLabList->FindByColumn(flLabID, _variant_t(arLabsToComplete[i]), NULL, false);

					if (pUpdateRow) {
						pUpdateRow->PutValue(flCurrentStepID, vtNull);
						pUpdateRow->PutValue(flStep, "None");
						pUpdateRow->PutValue(flStepOrder, "*");
						pUpdateRow->PutBackColor(colorHighlight);
					}
				}
			}
			else {
				for (i = 0; i < arLabsToComplete.GetSize(); i++) {
					long nCurrentLabID = arLabsToComplete.GetAt(i);
					IRowSettingsPtr pUpdateRow = m_pLabList->FindByColumn(flLabID, nCurrentLabID, NULL, false);
					if (pUpdateRow) // wasn't checking to see if this was null when setting the patient ID
					{
						// (z.manning 2008-10-31 10:49) - PLID 21082 - Signature is now required when completing
						// labs
						if(PromptToSignAndMarkLabComplete(this, nCurrentLabID)) {
							m_pLabList->RemoveRow(pUpdateRow);
						}
						else if(pUpdateRow != NULL) {
							pUpdateRow->PutValue(flCurrentStepID, g_cvarNull);
							pUpdateRow->PutValue(flStep, "None");
							pUpdateRow->PutValue(flStepOrder, "*");
							pUpdateRow->PutBackColor(colorHighlight);
						}
					}
				}
			}
		}

		if(bNeedToCreateLadder) {
			// (z.manning 2008-10-21 10:55) - PLID 31371 - The next step is set to create a ladder
			// so prompt them to do that now.
			if(PhaseTracking::PromptToAddPtnProcedures(nPatientID)) {
				// (z.manning 2008-10-21 10:55) - PLID 31371 - They created the tracking ladder, so
				// let's mark the next step complete.
				m_pLabList->PutCurSel(pRow);
				OnLabsMarkstepcomplete();
			}
		}

/*		if (bLinkedLabs)
			ReloadList(); */
		// I used to reload the list if we needed to, but now we actually keep track of everything that changes
		// so this is unnecessary.

		m_bDataChanged = true;
		return;

	//(e.lally 2008-06-02) PLID 27579 - When an exception gets caught, 
		//check for existing audit transaction and rollback.
	} NxCatchAllCall("Error in OnLabsMarkstepcomplete()", 
		if(nAuditTransID != -1){RollbackAuditTransaction(nAuditTransID);} );
}

BOOL CLabFollowUpDlg::DestroyWindow() 
{
	return CNxDialog::DestroyWindow();
}


void CLabFollowUpDlg::OnLabsOpenlab() 
{
	try {

		if(!CheckCurrentUserPermissions(bioPatientLabs, sptWrite))
			return;

		IRowSettingsPtr pRow(m_pLabList->GetCurSel());

		if (pRow == NULL)
			return;

		//Get the labID
		long nLabID = VarLong(pRow->GetValue(flLabID));
		long nPatientID = pRow->GetValue(flPatientID);
		// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
		long nResultID = VarLong(pRow->GetValue(flResultID), -1); 
		
		//Open the lab entry dialog
		// (c.haag 2010-07-16 9:51) - PLID 34338 - New way of opening labs. Legacy code commented out.
		GetMainFrame()->OpenLab(NULL, nPatientID, -1, ltInvalid, nLabID, -1, "", -1, m_checkAutoOpenNextLab.GetCheck(), FALSE, GetSafeHwnd(), GetCurrentLocationID(), nResultID);

		/*
		// (j.jones 2010-04-19 12:45) - PLID 37875 - tell the lab that we intend to open the next lab upon closing
		dlg.m_bNextLabWillOpenOnClose = m_checkAutoOpenNextLab.GetCheck();
		dlg.SetPatientID(nPatientID);
		//TES 11/25/2009 - PLID 36193 - We set the initial lab ID, the dialog may actually have multiple labs on it
		dlg.SetInitialLabID(nLabID);

		// We only need to reload the list if they made changes
		int nRet = dlg.DoModal();
		if(dlg.HasOpenedReport()) { // they have opened a report so close our window
			OnOK();
		}
		else {
			
			CArray<long, long> aryNextLabIDs;
			// (j.jones 2010-04-16 17:19) - PLID 37875 - if the user did not cancel
			// the lab, get the next lab IDs, need to cache all of them because
			// they could open one lab screen with multiple requisitions, which could
			// then disappear if we mark all of them completed
			if(nRet != IDCANCEL && m_checkAutoOpenNextLab.GetCheck()) {
				IRowSettingsPtr pNextRow = pRow->GetNextRow();
				while(pNextRow) {
					long nNextLabID = VarLong(pNextRow->GetValue(flLabID));
					aryNextLabIDs.Add(nNextLabID);
					pNextRow = pNextRow->GetNextRow();
				}
			}

			if(dlg.HasDataChanged()) {
				m_bDataChanged = true;
				ReloadList();
			}
			
			// (j.jones 2010-04-16 17:24) - PLID 37875 - Try to open the next lab.
			// This will wait for the requery as needed.
			if(nRet != IDCANCEL && m_checkAutoOpenNextLab.GetCheck()) {

				BOOL bOpenLab = FALSE;

				//first see if our original lab exists
				IRowSettingsPtr pOldRow = m_pLabList->SetSelByColumn(flLabID, (long)nLabID);
				if(pOldRow) {
					//the lab we just edited still exists, so now try to open the next one
					IRowSettingsPtr pNextRow = pOldRow->GetNextRow();
					if(pNextRow) {
						m_pLabList->PutCurSel(pNextRow);
						bOpenLab = TRUE;
					}
				}
				else {
					//the lab no longer exists (we probably completed it), but we cached the next
					//lab IDs, does any of them still exist?
					for(int i=0; i<aryNextLabIDs.GetSize() && !bOpenLab; i++) {
						IRowSettingsPtr pNextRow = m_pLabList->SetSelByColumn(flLabID, (long)aryNextLabIDs.GetAt(i));
						if(pNextRow) {
							//the next lab we wanted still exists, open it (it's already selected)
							bOpenLab = TRUE;
						}
					}
					//if none of our cached labs exist, give up
				}
				
				if(bOpenLab) {
					//the lab we want is now the CurSel, so tell the dialog to open this lab
					PostMessage(ID_OPEN_NEXT_LAB);
				}
			}
		}*/
	} NxCatchAll("Error in OnDblClickCellLabList()");	
}

// (j.dinatale 2011-01-05) - PLID 41818 - restructed this function a little bit because we are now interested in tracking clicks in both the notes and patient name column
void CLabFollowUpDlg::OnLeftClickLabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{ 
	try
	{
		switch(nCol){
			case flPatientName:
				{
					// follow the hyperlink!
					IRowSettingsPtr pRow(lpRow);
					if (pRow == NULL)
						return;

					long nPatientID = pRow->GetValue(flPatientID);

					GotoPatient(nPatientID);
				}
				break;
			// (j.dinatale 2011-01-05) - PLID 41818 - we are now interested in the notes column
			case flNotesIcon: 
				{	
					IRowSettingsPtr pRow(lpRow);

					// get the patient ID, result ID, and Lab ID
					long nResultID = VarLong(pRow->GetValue(flResultID), -1);
					long nPatientID = pRow->GetValue(flPatientID);
					long nLabID = VarLong(pRow->GetValue(flLabID), -1);

					// construct the notes dlg
					CNotesDlg dlgLabNotes(this);

					// assign it a lab result ID, lab ID, a the patient ID, tell it that its displaying lab notes, and the color to use
					dlgLabNotes.SetPersonID(nPatientID);
					dlgLabNotes.m_nLabID = nLabID;
					dlgLabNotes.m_nLabResultID = nResultID;
					dlgLabNotes.m_bIsLabNotes = true;
					dlgLabNotes.m_clrLabNote = ((CNxColor *)GetDlgItem(IDC_NXCOLORCTRL1))->GetColor();

					//default our prepended text to be blank
					CString strPrependToNote = "";

					// (j.dinatale 2011-01-06) - PLID 41818 - in order to have the prepended text show up correctly, need to run a query so we can retrieve
					//		the form number along with the specimen name, and if needed, the result name. The reason is because the way the Form column query is set up,
					//		it returns the form name in a slightly different format which makes it unusable for prepended text.
					if(nResultID != -1){
						// if we have a result ID, construct the name using the result ID
						_RecordsetPtr rsLabInfo = CreateParamRecordset(
							"SELECT '(Form # ' + LabsT.FormNumberTextID + "
							"CASE WHEN LabsT.Specimen IS NULL THEN '' ELSE ' - ' + LabsT.Specimen END  "
							"+ ')' + CASE WHEN LabResultsT.Name = '' THEN '' ELSE ': ' + LabResultsT.Name END as FormNumber "
							"FROM LabResultsT "
							"LEFT JOIN LabsT ON LabResultsT.LabID = LabsT.ID "
							"WHERE LabResultsT.ResultID = {INT}", nResultID);

						if(!rsLabInfo->eof){
							strPrependToNote = AdoFldString(rsLabInfo, "FormNumber", "");
						}
					}else{
						// otherwise, if we have a lab ID, construct the name using the lab ID 
						if(nLabID != -1){
							_RecordsetPtr rsLabInfo = CreateParamRecordset(
							"SELECT '(Form # ' + LabsT.FormNumberTextID + "
							"CASE WHEN LabsT.Specimen IS NULL THEN '' ELSE ' - ' + LabsT.Specimen END + ')' as FormNumber "
							"FROM LabsT "
							"WHERE LabsT.ID = {INT}", nLabID);

							if(!rsLabInfo->eof){
								strPrependToNote = AdoFldString(rsLabInfo, "FormNumber", "");
							}
						}
					}

					// give it to the notes dialog
					dlgLabNotes.m_strPrependText = strPrependToNote;

					// (j.dinatale 2011-08-12 15:00) - PLID 44861 - we were passing in a user name, that shouldnt happen because this is a global preference!
					// provide a category override
					dlgLabNotes.m_nCategoryIDOverride = GetRemotePropertyInt("LabNotesDefaultCategory",-1,0,"<None>",true);

					CString strCaption = "";

					if(nResultID != -1){
						strCaption = "Lab Result Notes: " + strPrependToNote;
					}else{
						strCaption = "Lab Specimen Notes: " + strPrependToNote;
					}

					// display it as a modal dialog
					CNxModalParentDlg dlg(this, &dlgLabNotes, strCaption);
					dlg.DoModal();

					// after we went modal, we need to ensure that the icon for the current row is correct.
					// assume no notes to begin with.
					BOOL bHasNotes = FALSE;

					// if we have a result ID thats valid, check to see if there are notes for the result
					if(nResultID != -1){
						bHasNotes = ReturnsRecordsParam("SELECT 1 FROM Notes WHERE LabResultID = {INT}", nResultID);
					}else{
						// otherwise, if we dont have a valid result ID, check for lab notes if we have a valid ID
						if(nLabID != -1){
							bHasNotes = ReturnsRecordsParam("SELECT 1 FROM Notes WHERE LabID = {INT}", nLabID);
						}
					}

					// set the row icon accordingly
					SetRowNoteIcon(pRow, bHasNotes);
				}
				break;
		}

	}NxCatchAll("CLabFollowUpDlg::OnLeftClickLabList");
}

// (j.dinatale 2011-01-05) - PLID 41818 - sets up the note icon for a given row.
void CLabFollowUpDlg::SetRowNoteIcon(IRowSettingsPtr pRow, BOOL bHasNotes)
{
	if(bHasNotes){
		// if we have notes, ensure the HasNotes field is 1 and set the icon to be the red notes icon
		pRow->PutValue(flHasNotes, 1);
		pRow->PutValue(flNotesIcon, (long)m_hNotes);
	}else{
		// otherwise, we dont have notes, so ensure HasNotes is 0 and set the icon to be the white single note
		pRow->PutValue(flHasNotes, 0);
		pRow->PutValue(flNotesIcon, (LPCTSTR)"BITMAP:FILE");
	}
}

void CLabFollowUpDlg::GotoPatient(long nPatID)
{
	try {
		if (nPatID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatID)) {
					return;
				}

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView)
					pView->UpdateView();
				// (z.manning 2008-11-07 12:17) - PLID 31955 - Dismissing this dialog will cause the datalist
				// to be destroyed which can cause the program to crash when this is called from the main list's
				// left click event. Plus I think minimizing is preferable in the first place.
				//OnOK(); // exit this dialog!
				Minimize();
			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - Cannot Open Mainframe");
			}//end else pMainFrame
		}//end if nPatientID
	}NxCatchAll("Error in GotoPatient()");
}

void CLabFollowUpDlg::OnChangeColumnSortFinishedLabList(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending) 
{
	try {
		ColorLabList();
	} NxCatchAll("Error in OnChangeColumnSortFinishedLabList()");	
}

CString GenerateCSV(CArray<long, long> &arID)
{//Generates a comma seperated value string of the given array of longs...
	CString strFinal, strTemp;

	try {
		int c = 0;
		for (int i = 0; i < arID.GetSize(); i++) {
			strTemp.Format("%li", arID[i]);
			if (c > 0) {
				strFinal += ",";
			}
			strFinal += strTemp;
			c++;
		}
	} NxCatchAll("Error in GenerateCSV()");

	return strFinal;
}

bool CLabFollowUpDlg::HasDataChanged() // return true if data may have been modified
{
	return m_bDataChanged;
}

void CLabFollowUpDlg::ColorLabList() {
	//Loop through all rows.  There are no children rows here, so we just
	//	loop over all of them.
	long nColorAdj = 5;

	try {
		IRowSettingsPtr pRow = m_pLabList->GetFirstRow();
		while(pRow) {
			pRow->PutCellLinkStyle(flPatientName, dlLinkStyleTrue);

			// (c.haag 2010-09-09 13:46) - PLID 40461 - If the lab is discontinued, show it in gray
			_variant_t var = pRow->GetValue(flDiscontinued);
			if (VarBool(var)) {
				pRow->PutBackColor(RGB(192,192,192));
			}
			else {
				var = pRow->GetValue(flCurrentStepID);
				if ( (var.vt == VT_NULL) || (var.vt == VT_EMPTY) ) {
					pRow->PutBackColor(colorHighlight);
				} else {
					pRow->PutBackColor(RGB(GetRValue(colorStandard) + nColorAdj, GetGValue(colorStandard) + nColorAdj, GetBValue(colorStandard)  + nColorAdj));
				}
			}

			//move to the next row
			pRow = pRow->GetNextRow();
			nColorAdj = -nColorAdj;
		}
	} NxCatchAll("Error in ColorLabList()");
}

void CLabFollowUpDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	// (a.walling 2008-04-15 12:16) - PLID 25755 - Handle modeless
	CNxDialog::OnShowWindow(bShow, nStatus);

	try {
		if (!bShow) {
			if(IsDlgButtonChecked(IDC_REMEMBER_LAB_COLUMN_SETTINGS)) {
				SaveColumns();
			}
			else {
				SetRemotePropertyText("DefaultLabFollowupColumnSizes", "", 0, GetCurrentUserName());
				SetRemotePropertyText("DefaultLabFollowupColumnSort", "", 0, GetCurrentUserName());
			}
		} else {
			// (a.walling 2008-04-15 12:17) - PLID 25755 - Reload the list.
			ReloadList();
		}
	} NxCatchAll("Error in CLabFollowUpDlg::OnShowWindow");
}

void CLabFollowUpDlg::OnSelChosenListProviders(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			long nID = VarLong(pRow->GetValue(fcID), -1);
			// (v.maida 2016-04-13 13:21) - NX-100191 - Added multiple providers option (-2).
			if (nID == -2)
			{
				CMultiSelectDlg dlg(this, "ProvidersT");
				dlg.PreSelect(m_varProvIDs);
				HRESULT hRes = dlg.Open("PersonT INNER JOIN ProvidersT ON ProvidersT.PersonID = PersonT.ID", 
					"(PersonT.Archived = 0 OR PersonT.ID IN "
					     "(SELECT ProviderID FROM LabMultiProviderT INNER JOIN LabsT ON LabID = LabsT.ID WHERE LabsT.Deleted = 0 AND (LabsT.ID NOT IN "
					         "(SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LabsT.ID IN "
					            "(SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL))))", 
					"PersonT.ID", "PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle", "Select Multiple Providers", 1);

				if (hRes == IDOK)
				{
					m_vecProvIDs.clear();
					m_varProvIDs.RemoveAll();
					dlg.FillArrayWithIDs(m_varProvIDs);

					if (m_varProvIDs.GetSize() == 1)
					{
						m_pProv->SetSelByColumn(0, m_varProvIDs[0]);
					}
					for (int i = 0; i < m_varProvIDs.GetSize(); i++)
					{
						m_vecProvIDs.push_back(AsLong(m_varProvIDs[i]));
					}
				}
				else {
					// Cancel was clicked, unless their last selection was the multiple providers option, switch back to their last selection.
					if (m_nLastProviderID != -2)
						m_pProv->SetSelByColumn(fcID, m_nLastProviderID);
				}
			}
			else {
				// (v.maida 2016-04-13 13:21) - NX-100191 - Clear the multiselect options if an individual provider was chosen
				m_varProvIDs.RemoveAll();
			}
		} else {
			// A null row was selected, just set the selection back to <All Providers>
			m_pProv->SetSelByColumn(fcID, (long)-1);
		}
		m_nLastProviderID = VarLong(m_pProv->GetCurSel()->GetValue(fcID), -1);

		ReloadList();
	} NxCatchAll("Error in OnSelChosenListProviders");
}

void CLabFollowUpDlg::OnSelChosenListStep(LPDISPATCH lpRow) 
{
	try {
		//(r.farnworth 2013-03-19) PLID 51115 - MultiSelect Options for Steps 
		IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			long nID = VarLong(pRow->GetValue(fcID), -1);

			// -2 indicates the MultiSelect option was chosen
			if (nID == -2) {
				CMultiSelectDlg dlg(this, "LabProcedureStepsT");

				dlg.PreSelect(m_varStepIDs);

				// New columns needed to keep the dialog nice and organized.
				CStringArray Names;
				Names.Add("ProcedureName");
				Names.Add("StepOrder");
			
				CStringArray Fields;
				Fields.Add("ProcedureName");
				Fields.Add("StepOrder");

				// Sort the new columns so that they are grouped by Procedure and sorted by the StepOrder
				dlg.m_astrOrderColumns.Add("ProcedureName");
				dlg.m_abSortAscending.Add(TRUE);
				dlg.m_astrOrderColumns.Add("StepOrder");
				dlg.m_abSortAscending.Add(TRUE);

				// Subquery is necessary to keep the new Name column from being labeled "LabProcedureStepsT.Name"
				if(dlg.Open("LabProcedureStepsT INNER JOIN (select Name AS 'ProcedureName', ID from LabProceduresT Where Inactive = 0) A ON (LabProcedureID = A.ID)", 
					m_StepFilter, "StepID", "LabProcedureStepsT.Name", "Select Multiple Steps", 1, -1, &Names, &Fields) == IDOK)
				{
					m_StepIDs.RemoveAll();
					m_varStepIDs.RemoveAll();
					dlg.FillArrayWithIDs(m_varStepIDs);

					if(m_varStepIDs.GetSize() == 1)
					{
						//(r.farnworth 2013-03-21) - PLID 51115 If only one object is selected, change the focus to that object's row
						m_pStep->SetSelByColumn(0, m_varStepIDs[0]);
					}
					for(int i = 0; i < m_varStepIDs.GetSize(); i++)
					{
						m_StepIDs.Add(AsString(m_varStepIDs[i]));
					}		
				} else {
					//(r.farnworth 2013-03-25) PLID 51115 - On Cancel - return to the last item you had selected unless it was Multiple Steps
					if(m_nLastStepID != -2)
						m_pStep->SetSelByColumn(0, m_nLastStepID);
					return; // We don't want to requery and it isn't necessary to track the Step ID
				}
			} else {
				//(r.farnworth 2013-04-24) PLID 51115 - Clear the multiselect options if an individual step was chosen
				m_varStepIDs.RemoveAll();
			}
		}

		// (j.jones 2015-08-27 14:28) - PLID 57837 - remember this per user

		m_nLastStepID = VarLong(pRow->GetValue(fcID)); //(r.farnworth 2013-03-25) PLID 51115 - Need to keep track of last Step ID
		ReloadList();
	} NxCatchAll("Error in OnSelChosenListStep");
}

void CLabFollowUpDlg::OnSelChosenListProcedure(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		bool bRequery = HandleSelChosenListProcedure(lpRow);

		if(m_pProc->GetCurSel() != NULL)
			m_nLastProcedureID = VarLong(m_pProc->GetCurSel()->GetValue(fcID));
		else 
			m_nLastProcedureID = -1; //(r.farnworth 2013-04-25) PLID 51115 - This isn't possible but let's leave it as a failsafe for future changes

		// (b.savon 2013-09-06 16:41) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
		if( m_nLastProcedureID != -2 ){
			m_varProcIDs.RemoveAll();
		}

		if (bRequery == true){ //(r.farnworth 2013-03-25) PLID 51115 - If cancel was selected from the multiselect dlg, do not requery
			ReloadList();
		}
	} NxCatchAll("Error in CLabFollowUpDlg::OnSelChosenListProcedure");
}

// (z.manning 2010-01-07 14:16) - PLID 36796 - Moved much of the logic of OnSelChosenListProcedure here
// (b.savon 2013-09-06 16:57) - PLID 58426 - Added bUseMultiSelectDlg
bool CLabFollowUpDlg::HandleSelChosenListProcedure(LPDISPATCH lpRow, BOOL bUseMultiSelectDlg /*= TRUE*/)
{
	IRowSettingsPtr pRow(lpRow);
	bool bRequery = true; // There is only one case under which we would want to return false so we'll initalize it with true.

	if (pRow) {
		long nID = VarLong(pRow->GetValue(fcID), -1);

		// (z.manning 2008-10-22 09:27) - PLID 31784 - This is always enabled now
		//GetDlgItem(IDC_LIST_STEP)->EnableWindow(nID == -1 ? FALSE : TRUE);

		IColumnSettingsPtr pColProcName = m_pStep->GetColumn(sfcProcedureName);
		CString strWhere = "LabProcedureStepsT.Inactive = 0";
		if(nID == -1) {
			// (z.manning 2008-10-22 10:23) - PLID 31784 - Since we now always have the step filter
			// enabled, show the procedure name column if we're filtering on all procedures.
			pColProcName->PutStoredWidth(120);
		}
		else if(nID == -2){
			if( bUseMultiSelectDlg ){
				//(r.farnworth 2013-03-19) PLID 51115 - MultiSelect option for Procedures
				CMultiSelectDlg dlg(this, "LabProceduresT");
				dlg.PreSelect(m_varProcIDs);
				HRESULT hRes = dlg.Open("LabProceduresT", "Inactive = 0", "ID", "Name", "Select Multiple Procedures", 1);

				if (hRes == IDOK)
				{
					pColProcName->PutStoredWidth(120);
					m_ProcIDs.RemoveAll();
					m_varProcIDs.RemoveAll();
					dlg.FillArrayWithIDs(m_varProcIDs);

					if (m_varProcIDs.GetSize() > 0)
					{
						GetProcedureWhereClause(strWhere);
					}
				} else {
					//(r.farnworth 2013-03-25) PLID 51115 - On Cancel - return to the last item chosen and don't requery
					if(m_nLastProcedureID != -2)
						m_pProc->SetSelByColumn(0, m_nLastProcedureID);

					bRequery = false;
					strWhere = m_StepFilter; //(r.farnworth 2013-03-25)  Use the last successful Where clause to filter the Steps dropdown
				}
			}else{ //We're loading the defaults
				GetProcedureWhereClause(strWhere);
			}
		}
		else {
			//(r.farnworth 2013-04-24) PLID 51115 - Clear the multiselect options if an individual procedure was chosen
			m_varProcIDs.RemoveAll();
			strWhere += FormatString(" AND LabProcedureID = %li", nID);
			// (z.manning 2008-10-22 10:23) - PLDI 31784 - We're filter on a procedure, so let's hide the redundant procedure name column.
			pColProcName->PutStoredWidth(0);
		}
	
		m_pStep->PutWhereClause(_bstr_t(strWhere));
		m_StepFilter = strWhere; //(r.farnworth 2013-03-19) PLID 51115 - We will use this as the where clause for the multiselect dlg for steps
		if(bRequery == false)
		{	
			return bRequery;
		}
	
		m_pStep->Requery();
		
		IRowSettingsPtr pNewRow = m_pStep->GetNewRow();
		pNewRow->PutValue(sfcID, _variant_t((long)-1));
		pNewRow->PutValue(sfcProcedureName, "");
		pNewRow->PutValue(sfcName, _variant_t(" <All Steps>"));
		pNewRow->PutValue(sfcStepOrder, g_cvarNull);
		pNewRow->PutValue(sfcProcedureID, g_cvarNull);
		m_pStep->AddRowSorted(pNewRow, NULL);

		//(r.farnworth 2013-03-18) - PLID 51115 Need to filter on Multiple Steps
		IRowSettingsPtr pNewRow2 = m_pStep->GetNewRow();
		pNewRow2 = m_pStep->GetNewRow();
		pNewRow2->PutValue(sfcID, _variant_t((long)-2));
		pNewRow2->PutValue(sfcProcedureName, "");
		pNewRow2->PutValue(sfcName, _variant_t(" <Multiple Steps>"));  
		pNewRow2->PutValue(sfcStepOrder, g_cvarNull);
		pNewRow2->PutValue(sfcProcedureID, g_cvarNull);
		m_pStep->AddRowSorted(pNewRow2, NULL);

		m_varStepIDs.RemoveAll();

		// (b.savon 2011-10-05 11:27) - PLID 45442 - Load Remembered 
		// (j.jones 2015-08-28 08:37) - PLID 57837 - moved to its own function
		PopulateStepDefaults();
	}
	return bRequery;
}

void CLabFollowUpDlg::OnLabsShowall() 
{
	try {
		m_pStep->FindByColumn(sfcID, _variant_t(long(-1)), NULL, VARIANT_TRUE);
		m_pProc->FindByColumn(sfcID, _variant_t(long(-1)), NULL, VARIANT_TRUE);
		m_pProv->FindByColumn(sfcID, _variant_t(long(-1)), NULL, VARIANT_TRUE);

		// (z.manning 2008-10-22 09:27) - PLID 31784 - This is always enabled now
		//GetDlgItem(IDC_LIST_STEP)->EnableWindow(FALSE);
		OnSelChosenListProcedure(m_pStep->GetCurSel());

		ReloadList();
	} NxCatchAll("Error in CLabFollowUpDlg::OnLabsShowall");
}

void CLabFollowUpDlg::OnBtnRefresh() 
{
	try {
		ReloadList();
	} NxCatchAll("Error in CLabFollowUpDlg::OnBtnRefresh()");	
}

void CLabFollowUpDlg::OnTrySetSelFinishedListProviders(long nRowEnum, long nFlags) 
{
	try {
		if (nFlags == dlTrySetSelFinishedFailure) {
			m_pProv->SetSelByColumn(fcID, (long)-1);
		}	
	} NxCatchAll("Error in OnTrySetSelFinishedListProviders");
}

void CLabFollowUpDlg::OnSelChangingListProviders(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("Error in OnSelChangingListProviders");
}

void CLabFollowUpDlg::OnSelChangingListProcedure(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("Error in OnSelChangingListProcedure");
}

void CLabFollowUpDlg::OnSelChangingListStep(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("Error in OnSelChangingListStep");
}

// (r.galicki 2008-10-23 11:29) - PLID 27214
void CLabFollowUpDlg::OnBnClickedCheckSpecimen()
{
	try {
		ReloadList();
	} NxCatchAll("Error in CLabFollowUpDlg::OnBnClickedCheckSpecimen");
}

// (r.galicki 2008-10-23 11:29) - PLID 27214
void CLabFollowUpDlg::OnBnClickedPrintSpecimen()
{
	try {
		//Build the menu
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
	
		enum LabelMenu {
			lmPreview = -1,
			lmPrint = -2,
		};

		mnu.InsertMenu(-1, MF_BYPOSITION, lmPreview, "Preview Specimen Label");
		mnu.InsertMenu(-1, MF_BYPOSITION, lmPrint, "Print Specimen Label");

		//Pop up the menu at the given position
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_PRINT_SPECIMEN);
		int nResult = 0;
		if (pWnd) 
			pWnd->GetWindowRect(&rc);
		int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_RIGHTBUTTON, rc.right, rc.top, this, NULL);

		//Destroy the menu
		mnu.DestroyMenu();

		CString strIDList;
		//prepare report
		CReportInfo  infLabel(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(654)]);
		BuildIDList(m_pLabList, strIDList);

		// (r.galicki 2008-11-11 09:27) - PLID 27214 - Prompt and return if now labs are in the list
		if(strIDList.IsEmpty()) {
			MessageBox("There are no specimen labels to be printed.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		
		infLabel.strExtraText = FormatString(" AND LabsT.ID IN (%s)", strIDList);
		
		// (a.walling 2009-07-02 10:45) - PLID 14181 - Send in a print info structure
		CPrintInfo prInfo;
		
		//Process the menu action
		switch(nCmdID) {
			case lmPreview:
				RunReport(&infLabel, TRUE, this, "Lab Specimen Label", &prInfo);
				Minimize();
				break;
			case lmPrint: 
				{
					CPrintDialog* dlg;
					dlg = new CPrintDialog(FALSE);
					prInfo.m_bPreview = false;
					prInfo.m_bDirect = false;
					prInfo.m_bDocObject = false;
					if(prInfo.m_pPD != NULL) {
						delete prInfo.m_pPD;
					}
					prInfo.m_pPD = dlg;
					RunReport(&infLabel, FALSE, this, "Lab Specimen Label", &prInfo);
				}
				break;
			default:
				return;
		}

		//update HasPrintedLabel
		ExecuteSql("UPDATE LabsT SET HasPrintedLabel = 1 WHERE LabsT.ID IN (%s)", strIDList);

	}NxCatchAll("Error in CLabFollowUpDlg::OnBnClickedPrintSpecimen");
}

void CLabFollowUpDlg::PostNcDestroy() {
	CNxDialog::PostNcDestroy();

	GetMainFrame()->m_pLabFollowupDlg = NULL;

	delete this;
}

// (r.galicki 2008-11-06 11:55) - PLID 27214 - Minimize (when previewing labels) - copied from CPicContainerDlg::OnEmrMinimizePic
void CLabFollowUpDlg::Minimize() {
	try {
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(&wp)) {
			//Check if we are not minimized
			if (!IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				SetWindowPlacement(&wp);
				GetMainFrame()->SetForegroundWindow();
			}
		}
	}NxCatchAll("Error in CLabFollowUpDlg::OnEmrMinimizePic()");
}

// (j.jones 2010-04-16 16:42) - PLID 37875 - added option to auto-open the next lab
void CLabFollowUpDlg::OnCheckAutoOpenNextLab()
{
	try {

		//save this selection, per user
		SetRemotePropertyInt("LabFollowUp_AutoOpenNextLab", m_checkAutoOpenNextLab.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-04-16 16:42) - PLID 37875 - added option to auto-open the next lab
LRESULT CLabFollowUpDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	try {

		if(message == ID_OPEN_NEXT_LAB) {
			OnLabsOpenlab();
			return 0;
		}				

	} NxCatchAll(__FUNCTION__);

	return CNxDialog::WindowProc(message, wParam, lParam);
}

// (j.gruber 2010-06-08 12:35) - PLID 36844
void CLabFollowUpDlg::OnBnClickedCheckLabsNoResults()
{
	try {

		// (j.gruber 2010-12-23 11:23) - PLID 38740 - warn if they have by reuslt selected
		if (IsDlgButtonChecked(IDC_CHECK_LABS_NO_RESULTS) && IsDlgButtonChecked(IDC_LFU_RESULT)) {
			DontShowMeAgain(this, "You have the 'Show by Results' option selected.  Showing the list by results with this checkbox checked will yield zero results.", "LabFollowupNoRecordsWarn");
		}

		ReloadList();
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-07-15 17:23) - PLID 34338 - This message is received when a lab entry dialog is closed
// and the lab was opened from this specific dialog.
LRESULT CLabFollowUpDlg::OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam)
{
	try {
		int nRet = wParam;
		CLabEntryDlg *pDlg = (CLabEntryDlg *)lParam;
		if (pDlg) {
			CLabEntryDlg& dlg = *pDlg;

			long nLabID = dlg.GetInitialLabID();
			// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
			long nResultID = dlg.GetInitialResultID(); 

			if(dlg.HasOpenedReport()) { // they have opened a report so close our window
				// (c.haag 2010-07-15 17:23) - PLID 34338 - We don't need to do this; the main frame 
				// will dismiss all windows that obstruct print previews
				//OnOK();
			}
			else {
				CArray<long, long> aryNextLabIDs;
				// (b.spivey - January 20, 2014) - PLID 46370 - Added ResultID
				std::vector<std::pair<long, long>> vecLabPairs; 

				//If we're showing by result, lets double check we have a result ID. 
				bool bSearchByResult = !!m_rdShowByResult.GetCheck(); 
				FollowUpListColumns fulcSearchColumn = flLabID;
				long nSearchValue = nLabID;

				if (bSearchByResult && nResultID > 0) {
					fulcSearchColumn = flResultID;
					nSearchValue = nResultID; 
					bSearchByResult = false; 
				}

				//Don't use LabID to search by. We may want to search by ResultID.
				IRowSettingsPtr pRow = m_pLabList->FindByColumn(fulcSearchColumn, nSearchValue, NULL, VARIANT_FALSE);

				if (NULL != pRow) {
					// (j.jones 2010-04-16 17:19) - PLID 37875 - if the user did not cancel
					// the lab, get the next lab IDs, need to cache all of them because
					// they could open one lab screen with multiple requisitions, which could
					// then disappear if we mark all of them completed
					if(nRet != IDCANCEL && m_checkAutoOpenNextLab.GetCheck()) {
						IRowSettingsPtr pNextRow = pRow->GetNextRow();
						long nLastLabID = nLabID;
						while(pNextRow) {
							long nNextLabID = VarLong(pNextRow->GetValue(flLabID));
							long nNextResultID = VarLong(pNextRow->GetValue(flResultID), -1); 
							// (j.gruber 2011-01-14 15:54) - PLID 38740 - call the next labID, not the next result
							// (c.haag 2013-02-04) - PLID 54955 - Removed the "m_rdShowByResult.GetCheck()" call;
							// it is unnecessary since every row, result or specimen, will have a LabID.
							if (nLastLabID != nNextLabID) {
								//Put this pairing into the vector. If we're searching by lab ID it's just one extra value, 
								//	but if we're searching by result ID then we'll know we're on the right row later on. 
								std::pair<long, long> pair(nNextLabID, nNextResultID);
								vecLabPairs.push_back(pair); 						
							}
							nLastLabID = nNextLabID;
							pNextRow = pNextRow->GetNextRow();
						}
					}

					if(dlg.HasDataChanged()) {
						m_bDataChanged = true;
						ReloadList();
					}
					
					// (j.jones 2010-04-16 17:24) - PLID 37875 - Try to open the next lab.
					// This will wait for the requery as needed.
					if(nRet != IDCANCEL && m_checkAutoOpenNextLab.GetCheck()) {

						BOOL bOpenLab = FALSE;

						//first see if our original lab exists
						//Unless we're searching for result row, then look for that instead. If we find it, the rest 
						//	 of this code should work without issue.
						IRowSettingsPtr pOldRow = m_pLabList->SetSelByColumn(fulcSearchColumn, nSearchValue);
						if(pOldRow) {
							//the lab we just edited still exists, so now try to open the next one
							// (j.gruber 2011-01-14 16:15) - PLID 38740 - go to the next lab
							long nLastLabID = nLabID;
							IRowSettingsPtr pNextRow = pOldRow->GetNextRow();							
							// (c.haag 2013-02-04) - PLID 54955 - Removed the "m_rdShowByResult.GetCheck()" call;
							// it is unnecessary since every row, result or specimen, will have a LabID.
							//loop until we get to a new labID
							BOOL bContinue = TRUE;
							while (pNextRow && bContinue) {
								long nNextLabID = VarLong(pNextRow->GetValue(flLabID));
								if (nLastLabID == nNextLabID) {
									pNextRow = pNextRow->GetNextRow();
								}
								else {
									bContinue = FALSE;
								}
							}
								
							if(pNextRow) {
								m_pLabList->PutCurSel(pNextRow);
								bOpenLab = TRUE;
							}
						}
						else {
							//the lab no longer exists (we probably completed it), but we cached the next
							//lab IDs, does any of them still exist?
							// (b.spivey - January 20, 2014) - PLID 46370 - If we didn't find our old result row, we have 
							//	 to try and find the next one in this list. Searching by result is safest as it allows us to 
							//	 retain our location in the list. 
							for(int i = 0; i < (long)vecLabPairs.size() && !bOpenLab; i++) {
								
								//Get the pairing.
								std::pair<long, long> pairLabIDs = vecLabPairs.at(i); 
								//We have a bool for this, but it's safer just to see what the searchcolumn value is, because 
								//	if through some calamity it gets set to something other than what it started as, this code 
								//	may bomb or even open the wrong lab.
								nSearchValue = (fulcSearchColumn == flResultID ? pairLabIDs.second : pairLabIDs.first); 
								
								// (b.spivey - January 20, 2014) - PLID 46370 - The search value should be unique, and therefore not cause the PLID46370 bug.
								IRowSettingsPtr pNextRow = m_pLabList->SetSelByColumn(fulcSearchColumn, nSearchValue);
								if(pNextRow) {
									//the next lab we wanted still exists, open it (it's already selected)
									// (c.haag 2013-02-04) - PLID 54955 - We also need to make pNextRow the current selection
									// so that the call to opening the lab knows we actually want that row.
									m_pLabList->PutCurSel(pNextRow);
									bOpenLab = TRUE;
								}
							}
							//if none of our cached labs exist, give up
						}
						
						if(bOpenLab) {
							//the lab we want is now the CurSel, so tell the dialog to open this lab
							PostMessage(ID_OPEN_NEXT_LAB);
						}
					}
				}
				else {
					// (c.haag 2010-07-15 17:23) - PLID 34338 - The lab was removed from the list since having been closed
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__)
	return 0;
}

// (c.haag 2010-09-09 13:46) - PLID 40461 - Click handler for the discontinued labs filter checkbox
void CLabFollowUpDlg::OnBnClickedCheckShowDiscontinued()
{
	try {
		SetRemotePropertyInt("ShowDiscontinuedLabsInLabFollowUp", (m_checkDiscontinuedLabs.GetCheck() ? 1 : 0), 0, GetCurrentUserName());
		// Reload the list
		ReloadList();
	}
	NxCatchAll(__FUNCTION__)
}

// (j.gruber 2010-12-23 12:29) - PLID 38740 - added a show by reqs
void CLabFollowUpDlg::OnBnClickedLfuRequisition()
{
	try {		

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pLabList->GetColumn(flFormNumberTextID);
		if (pCol) {
			pCol->ColumnTitle = "Form";
		}
		
		SetRemotePropertyInt("LabFollowupShowList", 0,0, GetCurrentUserName());
		
		ReloadList();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-12-23 12:29) - PLID 38740 - added a show by results
void CLabFollowUpDlg::OnBnClickedLfuResult()
{
	try {

		//see if they have the show labs without results selected and if so, tell them this will show no results
		if (IsDlgButtonChecked(IDC_CHECK_LABS_NO_RESULTS)) {
			DontShowMeAgain(this, "You have the 'Show only labs without Results' checkbox checked.  Showing the list by results with this checkbox checked will yield zero results.", "LabFollowupNoRecordsWarn");
		}

		//change the first column name
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pLabList->GetColumn(flFormNumberTextID);
		if (pCol) {
			pCol->ColumnTitle = "Form: Result Name";
		}
		
		SetRemotePropertyInt("LabFollowupShowList", 1,0, GetCurrentUserName());
		
		ReloadList();
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2013-04-23) PLID 51115 - Function to concat where clause for Steps and Procedures
void CLabFollowUpDlg::ConcatenateWhere(IRowSettingsPtr pRow, int ColumnID, CStringArray &ListIDs, CString ColumnName, CString &strWhere)
{
	if (pRow) {
		long nID = VarLong(pRow->GetValue(ColumnID), -1);

		//(r.farnworth 2013-03-19) PLID 51115 - MultiSelect Options for Steps
		if (nID == -2) {
			if(ListIDs.GetSize() > 0) 
			{
				strWhere += FormatString(" AND (%s = %s", ColumnName, ListIDs[0]);

				if(ListIDs.GetSize() > 1) //Need to use OR statements if more than 1 option was chosen
				{ 
					for(int i = 1; i < ListIDs.GetSize(); i++)
						strWhere += FormatString(" OR %s = %s", ColumnName, ListIDs[i]);
				}
				strWhere += ")";
			}
		} else if (nID != -1) {
			strWhere += FormatString(" AND %s = %li", ColumnName, nID);
		}
	}
}

//TES 8/1/2013 - PLID 57823 - Used in our custom sorting
typedef struct {
	short nColumnID;
	short nSortOrder;
} SortColumn;
//TES 8/1/2013 - PLID 57823 -  Used in our custom sorting.
static int CompareSortColumns(const void *pDataA, const void *pDataB)
{
	const SortColumn *pa = (const SortColumn*)pDataA;
	const SortColumn *pb = (const SortColumn*)pDataB;

	if (pa->nSortOrder < pb->nSortOrder) {
		return -1;
	}
	else if (pa->nSortOrder > pb->nSortOrder) {
		return 1;
	}
	else {
		return 0;
	}
}

void CLabFollowUpDlg::OnColumnClickingLabList(short nCol, BOOL* bAllowSort)
{
	try {

		//TES 8/1/2013 - PLID 57823 - Basically what we need to do here is, insert the FlagPriority column immediately before the FlagName column in the sort
		// order, then sort the list ourselves.  This code is modelled after CPatientToolBar::ApplySortLastFirstAtOnce()

		CArray<SortColumn,SortColumn&> aSortColumns;

		// Build an array of column indices and sort priorities for items that have a sort priority
		short i, nColumns = m_pLabList->GetColumnCount();
		SortColumn scPrimary;
		scPrimary.nSortOrder = -1;
		for (i=0; i < nColumns; i++) {
		
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(i);
			SortColumn sc;
			sc.nColumnID = i;
			sc.nSortOrder = pCol->GetSortPriority();

			//TES 8/1/2013 - PLID 57823 - Add this column to the sort array if it's anything but the flag priority
			if (i != flResultFlagPriority) {
				if (sc.nSortOrder > -1) {
					aSortColumns.Add(sc);
				}
			}

			// Find the column with the lowest non-negative sort order. This is the primary sort column.
			if (-1 == scPrimary.nSortOrder || (sc.nSortOrder >= 0 && sc.nSortOrder < scPrimary.nSortOrder)) {
				scPrimary = sc;
			}
		}

		//TES 8/1/2013 - PLID 57823 - Now check on if they clicked on the primary column. For our purposes, the flag name and priority columns are the same
		BOOL bClickedOnPrimary = FALSE;
		if(scPrimary.nColumnID == nCol || (scPrimary.nColumnID == flResultFlagPriority && nCol == flResults)) {
			bClickedOnPrimary = TRUE;
		}

		// At this point, the array now looks something like:
		//
		// ColID = 5  SortID = 2
		// ColID = 6  SortID = 4
		// ColID = 7  SortID = 3
		//
		// Now we sort the array from highest to lowest priority (0 = top priority, > 0 = lower)
		//
		if (aSortColumns.GetSize() > 0) {
			qsort(aSortColumns.GetData(), aSortColumns.GetSize(), sizeof(SortColumn), CompareSortColumns);
		}

		//TES 8/1/2013 - PLID 57823 - Now remove the clicked-on column, and re-insert it at the top.
		bool bRemoved = false;
		for(i = 0; i < aSortColumns.GetSize() && !bRemoved; i++) {
			if(aSortColumns[i].nColumnID == nCol) {
				aSortColumns.RemoveAt(i);
				bRemoved = true;
			}
		}
		SortColumn scClicked;
		scClicked.nColumnID = nCol;
		scClicked.nSortOrder = 0;
		aSortColumns.InsertAt(0, scClicked);

		// At this point, the array now looks something like:
		//
		// ColID = 5  SortID = 2
		// ColID = 7  SortID = 3
		// ColID = 6  SortID = 4
		//
		//TES 8/1/2013 - PLID 57823 - Add in the FlagPriority immediately before the FlagName column

		bool bInserted = false;
		for (i=0; i < aSortColumns.GetSize() && !bInserted; i++) {
			if(aSortColumns[i].nColumnID == flResults) {
				SortColumn sc;
				sc.nColumnID = flResultFlagPriority;
				aSortColumns.InsertAt(i, sc);
				bInserted = true;
			}
		}

		//*************************************************************//
		// Update sort priorities
		//*************************************************************//
		// At this point, the array now looks something like:
		//
		// ColID = 3  SortID = 1
		// ColID = 4  SortID = <undefined>
		// ColID = 5  SortID = 2
		// ColID = 7  SortID = 3
		// ColID = 6  SortID = 4
		//
		// Now reset all sort priorities in the list
		for (i=0; i < nColumns; i++) {
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(i);
			pCol->PutSortPriority(-1);
		}
		// Now assign priorities based on the array contents. Remember, the array is
		// sorted by datalist column sort priority, and 0 is the highest priority.
		for (i=0; i < aSortColumns.GetSize(); i++) {
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(aSortColumns[i].nColumnID);
			pCol->PutSortPriority(i);
		}

		//*************************************************************//
		// Update sort order
		//*************************************************************//
		// See if the user clicked on the primary sort column.
		if (bClickedOnPrimary) {
			//  If so, we have to reverse the sort order for all columns.
			for (i=0; i < nColumns; i++) {
				IColumnSettingsPtr pCol = m_pLabList->GetColumn(i);
				pCol->PutSortAscending( !pCol->GetSortAscending() );
			}						
		}
		else {
			// If not, we have to force the sort for the clicked column to be ascending
			IColumnSettingsPtr pCol = m_pLabList->GetColumn(nCol);
			const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
			pCol->PutSortAscending(varTrue);
		}

		//TES 8/1/2013 - PLID 57823 - Ensure that the FlagPriority column has the same sort as the FlagName column
		IColumnSettingsPtr pColPriority = m_pLabList->GetColumn(flResultFlagPriority);
		IColumnSettingsPtr pColName = m_pLabList->GetColumn(flResults);
		pColPriority->PutSortAscending( pColName->GetSortAscending() );

		m_pLabList->Sort();

		//TES 8/1/2013 - PLID 57823 - Since we just sorted, tell the datalist not to
		*bAllowSort = FALSE;
	}NxCatchAll(__FUNCTION__);

}

// (b.savon 2013-09-06 12:36) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
void CLabFollowUpDlg::PopulateProcedureDefaults()
{
	CString strProcedureDefault = GetRemotePropertyText("LabDefaultProcedureFilter", "-1", 0, GetCurrentUserName());	
	CArray<long, long> aryIDs;
	StringAsArray(strProcedureDefault, aryIDs);
	for(int i = 0; i < aryIDs.GetCount(); i++){
		m_varProcIDs.Add(_variant_t(aryIDs.GetAt(i)));
	}
	aryIDs.RemoveAll();
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = NULL;
	if( m_varProcIDs.GetCount() == 1 ){
		pRow = m_pProc->FindByColumn(fcID, AsLong(m_varProcIDs.GetAt(0)), NULL, VARIANT_TRUE);
	}else if( m_varProcIDs.GetCount() > 1 ){
		pRow = m_pProc->FindByColumn(fcID, (long)-2, NULL, VARIANT_TRUE);
	}

	if( pRow == NULL ){
		pRow = m_pProc->FindByColumn(fcID, (long)-1, NULL, VARIANT_TRUE);
	}

	m_nLastProcedureID = VarLong(pRow->GetValue(fcID)); //(r.farnworth 2013-03-25) PLID 51115 - Need to keep track of last Procedure ID

	HandleSelChosenListProcedure(pRow, FALSE);	
}

// (b.savon 2013-09-06 12:29) - PLID 58426 - Split this out into its own function
void CLabFollowUpDlg::GetProcedureWhereClause(CString &strWhere)
{
	// The first appendage to the where clause has to use AND while every other appendage after that must use OR
	m_ProcIDs.Add(AsString(m_varProcIDs[0]));
	strWhere += FormatString(" AND (LabProcedureID = %s", m_ProcIDs[0]);

	if(m_varProcIDs.GetSize() > 1)
	{
		for(int i = 1; i < m_varProcIDs.GetSize(); i++)
		{
			m_ProcIDs.Add(AsString(m_varProcIDs[i]));
			strWhere += FormatString(" OR LabProcedureID = %s", m_ProcIDs[i]);
		}

	} else {
		//(r.farnworth 2013-03-21) - PLID 51115 If only one object is selected, change the focus to that object's row
		m_pProc->SetSelByColumn(0, m_varProcIDs[0]);
	}
	strWhere += ") ";
}

// (b.savon 2013-09-06 12:29) - PLID 58426 - Save the procedure defaults
void CLabFollowUpDlg::SaveProcedureDefaults()
{

	CString strProcedureIDs = ArrayAsString(m_varProcIDs, true);
	if( strProcedureIDs.IsEmpty() ){
		strProcedureIDs.Format("%li", m_nLastProcedureID);
	}
	SetRemotePropertyText("LabDefaultProcedureFilter", strProcedureIDs, 0, GetCurrentUserName());
}

// (j.jones 2015-08-27 14:30) - PLID 57837 - we now remember multiple steps per user
void CLabFollowUpDlg::PopulateStepDefaults()
{
	m_varStepIDs.RemoveAll();
	m_StepIDs.RemoveAll();

	//see if they had a previously remembered single step
	long nDefaultStepID = GetRemotePropertyInt("LabDefaultStepFilter", -1, 0, GetCurrentUserName(), false);

	CString strDefaultStepID = "-1";
	if (nDefaultStepID != -1) {
		strDefaultStepID = AsString(nDefaultStepID);
	}

	CString strStepDefault = GetRemotePropertyText("LabDefaultStepsFilter", strDefaultStepID, 0, GetCurrentUserName());
	CArray<long, long> aryIDs;
	StringAsArray(strStepDefault, aryIDs);
	for (int i = 0; i < aryIDs.GetCount(); i++){
		long nStepID = aryIDs.GetAt(i);
		// (j.jones 2015-08-28 08:38) - PLID 57837 - don't add it to the arrays if the step doesn't exist anymore
		if (m_pStep->FindByColumn(sfcID, nStepID, NULL, VARIANT_FALSE) != NULL) {
			m_varStepIDs.Add(_variant_t(nStepID));
			m_StepIDs.Add(AsString(nStepID));
		}
	}
	aryIDs.RemoveAll();

	NXDATALIST2Lib::IRowSettingsPtr pRow = NULL;
	if (m_varStepIDs.GetCount() == 1){
		pRow = m_pStep->FindByColumn(sfcID, AsLong(m_varStepIDs.GetAt(0)), NULL, VARIANT_TRUE);
	}
	else if (m_varStepIDs.GetCount() > 1){
		pRow = m_pStep->FindByColumn(sfcID, (long)-2, NULL, VARIANT_TRUE);
	}

	if (pRow == NULL){
		pRow = m_pStep->FindByColumn(sfcID, (long)-1, NULL, VARIANT_TRUE);
	}

	m_nLastStepID = VarLong(pRow->GetValue(sfcID));
}

// (j.jones 2015-08-27 14:30) - PLID 57837 - we now remember multiple steps per user
void CLabFollowUpDlg::SaveStepDefaults()
{
	CString strStepIDs = ArrayAsString(m_varStepIDs, true);
	if (strStepIDs.IsEmpty()){
		strStepIDs.Format("%li", m_nLastStepID);
	}
	SetRemotePropertyText("LabDefaultStepsFilter", strStepIDs, 0, GetCurrentUserName());
}

// (v.maida 2016-04-14 9:00) - NX-100192 - Remember multiple providers.
void CLabFollowUpDlg::PopulateProviderDefaults()
{
	m_varProvIDs.RemoveAll();
	m_vecProvIDs.clear();

	//see if they had a previously remembered single provider
	long nDefaultProvID = GetRemotePropertyInt("LabDefaultProviderFilter", -1, 0, GetCurrentUserName(), false);

	CString strDefaultProvID = "-1";
	if (nDefaultProvID != -1) {
		strDefaultProvID = AsString(nDefaultProvID);
	}

	CString strProvDefault = GetRemotePropertyText("LabDefaultProvidersFilter", strDefaultProvID, 0, GetCurrentUserName());
	CArray<long, long> aryIDs;
	StringAsArray(strProvDefault, aryIDs);
	for (int i = 0; i < aryIDs.GetCount(); i++) {
		long nProvID = aryIDs.GetAt(i);
		// Don't add it to the array/vector if the provider doesn't exist anymore.
		if (m_pProv->FindByColumn(fcID, nProvID, NULL, VARIANT_FALSE) != NULL) {
			m_varProvIDs.Add(_variant_t(nProvID));
			m_vecProvIDs.push_back(nProvID);
		}
	}
	aryIDs.RemoveAll();

	NXDATALIST2Lib::IRowSettingsPtr pRow = NULL;
	if (m_varProvIDs.GetCount() == 1) {
		pRow = m_pProv->FindByColumn(fcID, AsLong(m_varProvIDs.GetAt(0)), NULL, VARIANT_TRUE);
	}
	else if (m_varProvIDs.GetCount() > 1) {
		pRow = m_pProv->FindByColumn(fcID, (long)-2, NULL, VARIANT_TRUE);
	}

	if (pRow == NULL) {
		pRow = m_pProv->FindByColumn(fcID, (long)-1, NULL, VARIANT_TRUE);
	}

	m_nLastProviderID = VarLong(pRow->GetValue(fcID));
}

// (v.maida 2016-04-14 9:00) - NX-100192 - Remember multiple providers.
void CLabFollowUpDlg::SaveProviderDefaults()
{
	CString strProvIDs = ArrayAsString(m_varProvIDs, true);
	if (strProvIDs.IsEmpty()) {
		strProvIDs.Format("%li", m_nLastProviderID);
	}
	SetRemotePropertyText("LabDefaultProvidersFilter", strProvIDs, 0, GetCurrentUserName());
}