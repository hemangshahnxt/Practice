// PatientProcedureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientProcedureDlg.h"
#include "AddProcedureDlg.h"
#include "GlobalDataUtils.h"
#include "ProcedureInfo.h"
#include "PhaseTracking.h"
#include "PatientView.h"
#include "EventSelectDlg.h"
#include "SelectStatusDlg.h"
#include "AuditTrail.h"
#include "nxmessagedef.h"
#include "NxSecurity.h"
#include "LetterWriting.h"
#include "mergeengine.h"
#include "MultiSelectDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "EnterActiveDateDlg.h"
#include "MergePromptDlg.h"
#include "PaymentDlg.h"
#include "SelectDlg.h"
#include "MergeLadderDlg.h"
#include "SingleSelectDlg.h"
#include "MultiSelectDlg.h"
#include "Modules.h"
#include "EMN.h"
#include "DecisionRuleUtils.h"
#include "BillingModuleDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-11-05 11:58) - PLID 53588 - Resolve conflict with mshtmcid.h

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_ADD_PROCEDURE		46784
#define IDM_DELETE_PROCEDURE	46785
#define IDM_EDIT_PROCEDURE	46786
#define IDM_GO		46789
#define ID_APPLY_EVENT 46791
#define ID_CHANGE_STATUS 46792
#define ID_SHOW_INFO 46793
#define ID_DELETE	46794
#define ID_UNSKIP_STEP 46795
#define ID_SKIP_STEP 46796
#define ID_PUT_ON_HOLD 46797
#define ID_ACTIVATE	46798
#define ID_MARK_DONE 46799
#define ID_MERGE 46800

#define DARK_COLOR 0x00B0B0EE
#define LIGHT_COLOR GetLighterColor(DARK_COLOR)
#define SKIPPED_COLOR GetLighterColor(RGB(255,0,0))

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace PhaseTracking;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

/////////////////////////////////////////////////////////////////////////////
// CPatientProcedureDlg dialog


CPatientProcedureDlg::CPatientProcedureDlg(CWnd* pParent)
: CPatientDialog(CPatientProcedureDlg::IDD, pParent),
	m_tcTask(NetUtils::TodoList),
	m_tcLadders(NetUtils::LaddersT)
{
	//{{AFX_DATA_INIT(CPatientProcedureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	//PLID 21512: per Don, if we don't have anything to put here, default to the earliest thing we can which is new patient
	//TES 10/29/2007 - PLID 27861 - We have a valid default topioc now.
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/Tracking/tracking_tab_overview.htm";
	// (c.haag 2009-08-12 11:02) - PLID 35157 - We now track when the pop-up menu is visible
	m_bPopupMenuVisible = FALSE;
	m_bNeedPostPopupRefresh = FALSE;
	
	// (a.walling 2010-10-13 07:26) - PLID 40977 - Keep track of the patient ID
	m_id = -1;
}


void CPatientProcedureDlg::DoDataExchange(CDataExchange* pDX)
{
	CPatientDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientProcedureDlg)
	DDX_Control(pDX, IDC_DELETE_PROCEDURE, m_btnDeleteProc);
	DDX_Control(pDX, IDC_NEW_PROCEDURE, m_btnNewProc);
	DDX_Control(pDX, IDC_TRACKING_REMEMBER_COL_WIDTHS, m_checkRememberColWidths);
	DDX_Control(pDX, IDC_ACTIVE_LADDERS, m_radioActive);
	DDX_Control(pDX, IDC_INACTIVE_LADDERS, m_radioInactive);
	DDX_Control(pDX, IDC_ALL_LADDERS, m_radioAll);
	DDX_Control(pDX, IDC_BKG, m_bkg);
	DDX_Control(pDX, IDC_HIDE_EMR_ONLY_PICS, m_hideEMROnlyPICs); // (b.eyers 2015-06-25) - PLID 39619
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatientProcedureDlg, CPatientDialog)
	//{{AFX_MSG_MAP(CPatientProcedureDlg)
	ON_BN_CLICKED(IDC_NEW_PROCEDURE, OnNewProcedure)
	ON_BN_CLICKED(IDC_DELETE_PROCEDURE, OnDeleteProcedure)
	ON_BN_CLICKED(IDC_GOTO_PROCEDURE, OnGotoProcedure)
	ON_BN_CLICKED(IDC_SHOW_PROC_INFO, OnShowProcInfo)
	ON_COMMAND(ID_UNSKIP_STEP, OnUnskipStep)
	ON_COMMAND(ID_SKIP_STEP, OnSkipStep)
	ON_MESSAGE(NXM_POST_EDIT_BILL, OnPostEditBill)
	ON_COMMAND(ID_PUT_ON_HOLD, OnPutOnHold)
	ON_COMMAND(ID_ACTIVATE, OnActivate)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_ACTIVE_LADDERS, OnActiveLadders)
	ON_BN_CLICKED(IDC_INACTIVE_LADDERS, OnInactiveLadders)
	ON_BN_CLICKED(IDC_ALL_LADDERS, OnAllLadders)
	ON_BN_CLICKED(IDC_TRACKING_REMEMBER_COL_WIDTHS, OnTrackingRememberColWidths)
	ON_COMMAND(IDM_GO, OnGo)
	ON_COMMAND(IDM_ADD_PROCEDURE, OnAdd)
	ON_COMMAND(IDM_DELETE_PROCEDURE, OnDelete)
	ON_COMMAND(ID_APPLY_EVENT, OnApplyEvent)
	ON_COMMAND(ID_CHANGE_STATUS, OnChangeStatus)
	ON_COMMAND(ID_SHOW_INFO, OnShowProcInfo)
	ON_COMMAND(ID_DELETE, OnDeleteProcedure)
	ON_COMMAND(ID_MARK_DONE, OnMarkDone)
	ON_COMMAND(ID_MERGE, OnMergeLadder)
	ON_BN_CLICKED(IDC_HIDE_EMR_ONLY_PICS, OnHideEMROnlyPICs) // (b.eyers 2015-06-25) - PLID 39619
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPatientProcedureDlg, CPatientDialog)
    //{{AFX_EVENTSINK_MAP(CPatientProcedureDlg)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 10 /* EditingFinished */, OnEditingFinishedProcedureList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 19 /* LeftClick */, OnLeftClickProcedureListTrackable, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 9 /* EditingFinishing */, OnEditingFinishingProcedureList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 6 /* RButtonDown */, OnRButtonDownProcedureListTrackable, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 8 /* EditingStarting */, OnEditingStartingProcedureListTrackable, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 3 /* DblClickCell */, OnDblClickCellProcedureListTrackable, VTS_I4 VTS_I2)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 18 /* RequeryFinished */, OnRequeryFinishedProcedureListTrackable, VTS_I2)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 2 /* SelChanged */, OnSelChangedProcedureListTrackable, VTS_I4)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 17 /* ColumnClicking */, OnColumnClickingProcedureListTrackable, VTS_I2 VTS_PBOOL)
	ON_EVENT(CPatientProcedureDlg, IDC_PROCEDURE_LIST_TRACKABLE, 22 /* ColumnSizingFinished */, OnColumnSizingFinishedProcedureListTrackable, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CPatientProcedureDlg message handlers

BOOL CPatientProcedureDlg::OnInitDialog() 
{
	try {

		CPatientDialog::OnInitDialog();
		m_procedureList = BindNxDataListCtrl(IDC_PROCEDURE_LIST_TRACKABLE, false);
		m_bReadOnly = FALSE;

		// (j.jones 2008-09-03 12:00) - PLID 10417 - bulk cache all preferences
		g_propManager.CachePropertiesInBulk("PatientProcedureDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefaultTrackingView' OR "
			"Name = 'RememberTrackingColumns' OR "
			"Name = 'TrackingAlwaysMerge' OR "
			"Name = 'TrackingIgnorePastEvents' OR "
			"Name = 'TrackingUseNoPurpose' OR "
			"Name = 'TrackingInactivateLadderOnCancel' OR "
			"Name = 'TrackingPromptOnMultiple' OR "
			"Name = 'DefaultASCLicenseWarnDayRange' OR "
			"Name = 'TrackingCombineProcedures' OR "
			"Name = 'LadderAssignment' OR "
			"Name = 'AutoCompleteNextStep' OR "
			"Name = 'QuoteDepositTypeRequired' OR "
			"Name = 'QuoteDepositPercent' OR "
			"Name = 'TrackingPromptOnMultiple' OR "
			"Name = 'WarnWhenCreatingNexWebEmn' OR " //TES 11/13/2009 - PLID 35807
			"Name = 'TrackingProcedureNamesInToDos' OR " //(e.lally 2009-12-29) PLID 13948
			"Name = 'TrackingMultiLadderSameProcedure' OR " // (d.singleton 2012-02-06 16:55) - PLID 47885
			"Name = 'HideEMROnlyPICs' " // (b.eyers 2015-06-25) - PLID 39619
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("PatientProcedureDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'QuoteDepositFee' "
			")",
			_Q(GetCurrentUserName()));
		
		//OK, now this may seem weird, but the problem is that our user drop-down won't get filled in until we requery.
		//So, since we don't want to put any actual data, we set the clauses so they can't possibly return any rows
		//(and we set up all the data fields to not return any data), and we requery once to fill in the users.
		m_procedureList->FromClause = _bstr_t("LaddersT INNER JOIN PicT ON LaddersT.ProcInfoID = PicT.ProcInfoID");
		m_procedureList->WhereClause = _bstr_t("LaddersT.ID Is Null");
		m_procedureList->Requery();

		//Go through and set everything to not be expanded.
		// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
		_RecordsetPtr rsLadders = CreateParamRecordset("SELECT ID FROM LaddersT WHERE PersonID = {INT}", GetActivePatientID());
		while(!rsLadders->eof ) {
			m_mTopLevel.SetAt(AdoFldLong(rsLadders, "ID"), false);
			rsLadders->MoveNext();
		}

		IColumnSettingsPtr pCol;
		pCol = m_procedureList->GetColumn(CP_Name);
		pCol->PutFieldType(cftTextSingleLineLink);

		GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);

		int nDefaultView = GetRemotePropertyInt("DefaultTrackingView", 2, 0, GetCurrentUserName(), true);
		CheckDlgButton(IDC_ACTIVE_LADDERS, nDefaultView == 0);
		CheckDlgButton(IDC_INACTIVE_LADDERS, nDefaultView == 1);
		CheckDlgButton(IDC_ALL_LADDERS, nDefaultView == 2);

		// (b.eyers 2015-06-25) - PLID 39619 - should the hide emr only pics be checked off or not
		if (GetRemotePropertyInt("HideEMROnlyPICs", 0, 0, GetCurrentUserName(), true) == 1) {
			CheckDlgButton(IDC_HIDE_EMR_ONLY_PICS, TRUE);
		}
		else {
			CheckDlgButton(IDC_HIDE_EMR_ONLY_PICS, FALSE);
		}

		//DRT 4/11/2008 - PLID 29636 - NxIconify
		m_btnNewProc.AutoSet(NXB_NEW);
		m_btnDeleteProc.AutoSet(NXB_DELETE);

		SecureControls();

	}NxCatchAll("Error in CPatientProcedureDlg::OnInitDialog()");
	return TRUE;
}

// (a.walling 2010-10-12 17:40) - PLID 40977
void CPatientProcedureDlg::UpdateView(bool bForceRefresh)
{
	try {
		// (a.walling 2010-10-12 17:43) - PLID 40977 - Forces focus lost messages
		CheckFocus();
		
		long nCurrentlyLoadedID = m_id;
		// (a.walling 2010-10-13 07:26) - PLID 40977 - Keep track of the patient ID
		m_id = GetActivePatientID();

		if (m_tcLadders.Changed()) {
			m_ForceRefresh = true;
		}

		if (m_tcTask.Changed()) {
			m_ForceRefresh = true;
		}

		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			Refresh();
		}
		m_ForceRefresh = false;

	} NxCatchAll(__FUNCTION__);
}

void CPatientProcedureDlg::Refresh()
{
	CWaitCursor cuWait;

	try {
		// (c.haag 2009-08-12 11:02) - PLID 35157 - The refresh should be delayed if the pop-up menu is visible
		if (m_bPopupMenuVisible) {
			m_bNeedPostPopupRefresh = TRUE;
			return;
		}

		// (a.walling 2010-10-13 07:26) - PLID 40977 - Keep track of the patient ID
		m_id = GetActivePatientID();

		// (j.jones 2010-10-11 16:39) - PLID 35424 - clear our ladder tablechecker,
		// since we are reloading everything
		m_tcLadders.Changed();
		// (a.walling 2010-10-13 07:25) - PLID 40977 - Same here
		m_tcTask.Changed();

		//JJ - 10/31/01 - SetRedraw(FALSE) so we don't visibly see the list redraw
		m_procedureList->SetRedraw(FALSE);

		int nSel = m_procedureList->CurSel;
		//Clear out the datalist.
		m_procedureList->Clear();

		// (b.eyers 2015-06-25) - PLID 39619 - if hide is checked off, we need to hide pics with no ladders
		CSqlFragment sqlHideEMROnlyPICs;

		if (IsDlgButtonChecked(IDC_HIDE_EMR_ONLY_PICS)) {
			sqlHideEMROnlyPICs = CSqlFragment("AND LaddersT.ID IS NOT NULL ");
		}

		//Get list of top-level ladders.
		_RecordsetPtr rsLadders;
		//TES 9/29/2004 - Our base unit is PICs now, not ladders.
		// (b.cardillo 2005-10-12 14:02) - PLID 17178 - Made all three of these exclude any uncommitted PICs.
		// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized these queries, and combined the recordset from
		// InsertTopLevel into these recordsets, so we can pass in all our info into InsertTopLevel
		// (b.eyers 2015-06-25) - PLID 39619 - added the sql fragment for hiding pics with no ladders
		if(IsDlgButtonChecked(IDC_ACTIVE_LADDERS) ){
			//Either we have a ladder and it's active, or we don't have a ladder, but we do have an EMR Group and its active,
			//or we have neither one (I don't think that should be possible, but we'll leave it active if it happens).
			rsLadders = CreateParamRecordset("SELECT ProcInfoT.ID AS ProcInfoID, LaddersT.ID AS LadderID, PicT.ID AS PicID, "
				"TopLevelInfoQ.Status, TopLevelInfoQ.Notes, TopLevelInfoQ.LadderName, TopLevelInfoQ.FirstInterestDate, "
				"TopLevelInfoQ.Name, TopLevelInfoQ.UserName, TopLevelInfoQ.EmrDate, TopLevelInfoQ.HasProcedure, "
				"TopLevelInfoQ.IsActive, TopLevelInfoQ.NextStep, TopLevelInfoQ.DoneDate "
				"FROM ProcInfoT "
				"LEFT JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID "
				"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
				"LEFT JOIN (SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID "

				"LEFT JOIN ("
					"SELECT ProcInfoT.ID AS ProcInfoID, dbo.CalcProcInfoName(ProcInfoT.ID) AS LadderName, LaddersT.ID, "
					"LaddersT.PersonID, LadderStatusT.Name AS Status, LaddersT.Notes, FirstInterestDate, LaddersT.Name, "
					"UsersT.UserName, EmrGroupsT.InputDate AS EmrDate, "
					"CASE WHEN EXISTS (SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID) THEN 1 ELSE 0 END AS HasProcedure, "
					"LadderStatusT.IsActive, (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) AS NextStep, "
					"(SELECT Max(CompletedDate) AS DoneDate FROM StepsT WHERE LadderID = LaddersT.ID) AS DoneDate "
					"FROM ProcInfoT LEFT JOIN (LaddersT INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID) ON ProcInfoT.ID = LaddersT.ProcInfoID "
					"LEFT JOIN ((SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID) ON ProcInfoT.ID = PicT.ProcInfoID "
					"LEFT JOIN UsersT ON LaddersT.UserID = UsersT.PersonID "
					") AS TopLevelInfoQ ON ProcInfoT.ID = TopLevelInfoQ.ProcInfoID "					

				"WHERE ((LaddersT.ID Is Not Null AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1)) "
				"OR (LaddersT.ID Is Null AND (EmrGroupsT.Status Is Null OR EmrGroupsT.Status = 0)))"
				"AND PicT.IsCommitted = 1 "
				"AND ProcInfoT.PatientID = {INT} {SQL} ", GetActivePatientID(), sqlHideEMROnlyPICs);
		}
		else if(IsDlgButtonChecked(IDC_INACTIVE_LADDERS)) {
			rsLadders = CreateParamRecordset("SELECT ProcInfoT.ID AS ProcInfoID, LaddersT.ID AS LadderID, PicT.ID AS PicID, "
				"TopLevelInfoQ.Status, TopLevelInfoQ.Notes, TopLevelInfoQ.LadderName, TopLevelInfoQ.FirstInterestDate, "
				"TopLevelInfoQ.Name, TopLevelInfoQ.UserName, TopLevelInfoQ.EmrDate, TopLevelInfoQ.HasProcedure, "
				"TopLevelInfoQ.IsActive, TopLevelInfoQ.NextStep, TopLevelInfoQ.DoneDate "
				"FROM ProcInfoT "
				"LEFT JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID "
				"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
				"LEFT JOIN (SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID "

				"LEFT JOIN ("
					"SELECT ProcInfoT.ID AS ProcInfoID, dbo.CalcProcInfoName(ProcInfoT.ID) AS LadderName, LaddersT.ID, "
					"LaddersT.PersonID, LadderStatusT.Name AS Status, LaddersT.Notes, FirstInterestDate, LaddersT.Name, "
					"UsersT.UserName, EmrGroupsT.InputDate AS EmrDate, "
					"CASE WHEN EXISTS (SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID) THEN 1 ELSE 0 END AS HasProcedure, "
					"LadderStatusT.IsActive, (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) AS NextStep, "
					"(SELECT Max(CompletedDate) AS DoneDate FROM StepsT WHERE LadderID = LaddersT.ID) AS DoneDate "
					"FROM ProcInfoT LEFT JOIN (LaddersT INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID) ON ProcInfoT.ID = LaddersT.ProcInfoID "
					"LEFT JOIN ((SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID) ON ProcInfoT.ID = PicT.ProcInfoID "
					"LEFT JOIN UsersT ON LaddersT.UserID = UsersT.PersonID "
					") AS TopLevelInfoQ ON ProcInfoT.ID = TopLevelInfoQ.ProcInfoID "	

				"WHERE NOT ((LaddersT.ID Is Not Null AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1)) "
				"OR (LaddersT.ID Is Null AND (EmrGroupsT.Status Is Null OR EmrGroupsT.Status = 0)))"
				"AND PicT.IsCommitted = 1 "
				"AND ProcInfoT.PatientID = {INT} {SQL} ", GetActivePatientID(), sqlHideEMROnlyPICs);
		}
		else {
			rsLadders = CreateParamRecordset("SELECT ProcInfoT.ID AS ProcInfoID, LaddersT.ID AS LadderID, PicT.ID AS PicID, "
				"TopLevelInfoQ.Status, TopLevelInfoQ.Notes, TopLevelInfoQ.LadderName, TopLevelInfoQ.FirstInterestDate, "
				"TopLevelInfoQ.Name, TopLevelInfoQ.UserName, TopLevelInfoQ.EmrDate, TopLevelInfoQ.HasProcedure, "
				"TopLevelInfoQ.IsActive, TopLevelInfoQ.NextStep, TopLevelInfoQ.DoneDate " 
				"FROM ProcInfoT "
				"LEFT JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID "
				"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "

				"LEFT JOIN ("
					"SELECT ProcInfoT.ID AS ProcInfoID, dbo.CalcProcInfoName(ProcInfoT.ID) AS LadderName, LaddersT.ID, "
					"LaddersT.PersonID, LadderStatusT.Name AS Status, LaddersT.Notes, FirstInterestDate, LaddersT.Name, "
					"UsersT.UserName, EmrGroupsT.InputDate AS EmrDate, "
					"CASE WHEN EXISTS (SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID) THEN 1 ELSE 0 END AS HasProcedure, "
					"LadderStatusT.IsActive, (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) AS NextStep, "
					"(SELECT Max(CompletedDate) AS DoneDate FROM StepsT WHERE LadderID = LaddersT.ID) AS DoneDate "
					"FROM ProcInfoT LEFT JOIN (LaddersT INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID) ON ProcInfoT.ID = LaddersT.ProcInfoID "
					"LEFT JOIN ((SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID) ON ProcInfoT.ID = PicT.ProcInfoID "
					"LEFT JOIN UsersT ON LaddersT.UserID = UsersT.PersonID "
					") AS TopLevelInfoQ ON ProcInfoT.ID = TopLevelInfoQ.ProcInfoID "	

				"WHERE ProcInfoT.PatientID = {INT} "
				"AND PicT.IsCommitted = 1 {SQL} "
				"", GetActivePatientID(), sqlHideEMROnlyPICs);
		}

		int nNextRow = 0; //Start at the top.
		while(!rsLadders->eof) {

			// (j.jones 2008-09-03 17:02) - PLID 10417 - now the above recordsets have all the data we need for
			// InsertTopLevel, so let's just pass it all in

			COleDateTime dtDate;

			if(rsLadders->Fields->GetItem("FirstInterestDate")->Value.vt == VT_DATE) {
				dtDate = VarDateTime(rsLadders->Fields->GetItem("FirstInterestDate")->Value);
			}
			else {
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				dtDate = VarDateTime(rsLadders->Fields->GetItem("EmrDate")->Value, dtInvalid);
			}

			// (j.jones 2008-12-01 08:57) - PLID 32262 - changed nUserID to strUserNames
			nNextRow = InsertTopLevel(nNextRow, AdoFldLong(rsLadders, "LadderID", -1), AdoFldLong(rsLadders, "PicID", -1), AdoFldLong(rsLadders, "ProcInfoID"),
				AdoFldString(rsLadders, "LadderName", ""), (BOOL)AdoFldLong(rsLadders, "HasProcedure"), AdoFldLong(rsLadders, "NextStep", -1),
				AdoFldString(rsLadders, "Status",""), AdoFldBool(rsLadders, "IsActive",FALSE), dtDate,
				AdoFldString(rsLadders, "Notes",""), rsLadders->Fields->GetItem("DoneDate")->Value, AdoFldString(rsLadders, "UserName", ""));

			rsLadders->MoveNext();
		}

		//TES 2/23/2004: Load our column settings.
		if(m_strOriginalColSizes.IsEmpty()) {
			//First, remember the original settings
			CString strOrig, str;
			for(int i = 0; i < m_procedureList->GetColumnCount(); i++) {
				IColumnSettingsPtr pCol = m_procedureList->GetColumn(i);
				str.Format("%li,", pCol->GetStoredWidth());

				strOrig += str;
			}

			m_strOriginalColSizes = strOrig;

			if(GetRemotePropertyInt("RememberTrackingColumns", 0, 0, GetCurrentUserName(), true) == 1) {
				CheckDlgButton(IDC_TRACKING_REMEMBER_COL_WIDTHS, TRUE);
				OnTrackingRememberColWidths();
			}
			else {
				CheckDlgButton(IDC_TRACKING_REMEMBER_COL_WIDTHS, FALSE);
			}
		}
		
		m_procedureList->SetRedraw(TRUE);
		//This is so that, when you first load the screen, the first ladder on the list will be highlighted.
		if(nSel != -1) {
			m_procedureList->CurSel = nSel;
		}
		else {
			if(m_procedureList->GetRowCount() > 0) {
				m_procedureList->CurSel = 0;
			}
			else {
				m_procedureList->CurSel = -1;
			}
		}

		OnSelChangedProcedureListTrackable(m_procedureList->CurSel);

		m_ForceRefresh = false;

	}NxCatchAll("Error in CPatientProcedureDlg::Refresh()");
}

void CPatientProcedureDlg::OnNewProcedure() 
{
	// (z.manning 2008-10-28 11:07) - PLID 31371 - Moved this functionality to a utility function in
	// the PhaseTracking namespace.
	PhaseTracking::PromptToAddPtnProcedures(GetActivePatientID());
}

void CPatientProcedureDlg::OnDeleteProcedure() 
{
	long curSel = m_procedureList->CurSel;

	try
	{
		//Handle based on row type.
		if(curSel == -1) {
			return;
		}
		_variant_t varType = m_procedureList->Value[curSel][CP_RowType];
		if(varType.vt == VT_NULL || varType.vt == VT_EMPTY) {
			//Not a valid row
			return;
		}

		if(VarLong(varType) == 1) {

			long nPicID = VarLong(m_procedureList->GetValue(curSel, CP_PicID), -1);

			//DRT 9/13/2006 - PLID 22515 - We have decided to not allow tracking ladders
			//	to be deleted if they are tied to an EMN from here.  You must do it from
			//	the EMR screen.  Amusingly, this makes my whole effort below to combine it 
			//	all into 1 area more difficult to change =/
			CString str;
			str.Format("SELECT * FROM PicT WHERE ID = %li AND EMRGroupID IN "
				"(SELECT ID FROM EMRGroupsT WHERE ID IN (SELECT EMRGroupID FROM EMRMasterT))", nPicID);
			if(ReturnsRecords(str)) {
				//There is *some* EMR record tied to this PIC.  Do not allow deletion.
				AfxMessageBox("You may not delete a tracking ladder that is tied to an EMR.  Please have an administrator review the EMR if it should be deleted.");
				return;
			}

			//TES 9/13/2010 - PLID 39845 - Check the new Delete permission on Tracking (note the amusing fact that this was not 
			// controlled by any permissions at all until now).
			if(!CheckCurrentUserPermissions(bioPatientTracking, sptDelete)) {
				return;
			}

			//I moved the code here to a global function because deleting a record here and
			//	deleting a record in the patients EMR results in the same activity.
			CDWordArray arNewCDSInterventions;
			//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
			if(!AttemptRemovePICRecord(nPicID, arNewCDSInterventions)) {
				//delete failed
				return;
			}
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

			// (j.jones 2010-11-17 17:06) - PLID 35424 - remove this ladder now
			long nLadderID = VarLong(m_procedureList->GetValue(curSel, CP_LadderID), -1);
			RemoveLadder(nLadderID);
		}
		else if(VarLong(varType) == 3) {
			//Event level, try to unapply item.
			if(IDYES != AfxMessageBox("Are you sure you would like to unapply this event?\nThe event will no longer be tracked.", MB_YESNO)) 
				return;
			UnapplyEvent(GetActivePatientID(), (PhaseTracking::EventType)VarLong(m_procedureList->GetValue(curSel, CP_EventType),-1), VarLong(m_procedureList->GetValue(curSel, CP_EventID),-1), VarLong(m_procedureList->Value[curSel][CP_StepID]));
			Refresh();
		}
		else {
			//We can't delete steps, and anything else is an invalid row type.
			return;
		}
	}
	NxCatchAll("Could not delete patient procedure");
}

void CPatientProcedureDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
}

void CPatientProcedureDlg::OnEditingFinishedProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try{
		// (r.galicki 2008-09-19 16:11) - PLID 27363 - Don't need to check flag here (prevented edit date only permissions from working).
		//	Should not get here if permissions do not permit.
		//if(!m_bReadOnly) {
		_variant_t varType;
		switch(nCol) {
		case CP_Done:
			//What level are we at?
			varType = m_procedureList->Value[nRow][CP_RowType];
			if (varType.vt == VT_EMPTY || varType.vt == VT_NULL) {
				return;
			}
			if(VarLong(varType) == 3) {
				//There's no checkbox at this level!
				return;
			}
			else if(VarLong(varType) == 2) {
				if(VarBool(varOldValue)) {
					//Unchecked, so unapply
					//We need to check whether this ladder is done, so that if it was, and we're making it not, we can change the status accordingly.
					long nEventType = VarLong(m_procedureList->GetValue(nRow, CP_EventType), -1);
					if(nEventType != -1) {
						UnapplyEvent(GetActivePatientID(), (PhaseTracking::EventType)nEventType, -1, VarLong(m_procedureList->GetValue(nRow, CP_StepID)));
					}
					//NOTE: if eof does happen to be true, it means that this step has nothing applied to it,
					//which means that the screen is somehow out of sync with data.  So what should we do?
					//Refresh!
					//PostMessage(NXM_REFRESH_PATPROC);
				}
				else {
					//This should be done already, but double-check.
					m_procedureList->CurSel = nRow;
					OnMarkDone();
				}
			}
			else if(VarLong(varType) == 1) {
				if(!VarBool(varOldValue)) {
					//Mark all unfinished events finished.
					BeginWaitCursor();
					// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
					_RecordsetPtr rsPendingSteps = CreateParamRecordset("SELECT ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = {INT}", VarLong(m_procedureList->Value[nRow][CP_LadderID]));
					CDWordArray dwaIDs;
					while(!rsPendingSteps->eof) {
						dwaIDs.Add(AdoFldLong(rsPendingSteps, "ID"));
						rsPendingSteps->MoveNext();
					}
					rsPendingSteps->Close();
					for(int i = 0; i < dwaIDs.GetSize(); i++) {
						PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_MarkedDone, GetActivePatientID(), COleDateTime::GetCurrentTime(), -1, false, (long)dwaIDs.GetAt(i));
					}
					EndWaitCursor();
					//PostMessage(NXM_REFRESH_PATPROC);
				}
				else {
					bCommit = false;
				}
			}

			break;

		case CP_Notes:
			varType = m_procedureList->Value[nRow][CP_RowType];
			if (varType.vt == VT_EMPTY || varType.vt == VT_NULL) {
				return;
			}
			if(VarLong(varType) == 1) {
				ExecuteSql("UPDATE LaddersT SET Notes = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), VarLong(m_procedureList->Value[nRow][CP_LadderID]));
			}
			else if(VarLong(varType) == 2) {
				ExecuteSql("UPDATE StepsT SET Note = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), VarLong(m_procedureList->Value[nRow][CP_StepID]));
			}
			break;

		case CP_Date:
			if(bCommit) {
				varType = m_procedureList->Value[nRow][CP_RowType];
				if (varType.vt == VT_EMPTY || varType.vt == VT_NULL) {
					return;
				}
				// (j.jones 2010-10-11 16:41) - PLID 35424 - shouldn't do anything if the date did not change
				if(VarLong(varType) == 1 && 
					(varOldValue.vt != varNewValue.vt ||
					(varOldValue.vt == VT_DATE && varNewValue.vt == VT_DATE && VarDateTime(varNewValue) != VarDateTime(varOldValue)))) {
					CString strOldDate = "";
					if(varOldValue.vt == VT_DATE) {
						strOldDate = FormatDateTimeForInterface(VarDateTime(varOldValue), DTF_STRIP_SECONDS, dtoDate);
					}
					long nLadderID = VarLong(m_procedureList->Value[nRow][CP_LadderID]);
					ExecuteSql("UPDATE LaddersT SET FirstInterestDate = '%s' WHERE ID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), nLadderID);
					//TES 9/19/2007 - PLID 27434 - Audit changing dates.
					AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiLadderFirstInterestDate, nLadderID, strOldDate, FormatDateTimeForInterface(VarDateTime(varNewValue), 0, dtoDate, false), aepMedium, aetChanged);
					// (c.haag 2009-02-11 08:36) - PLID 33008 - Sometimes, a user creates a ladder; but moves the first interest date 
					// to a time in the past. In a sense, they really meant to have created a ladder back on the day of the FID. The
					// significance of that is when a ladder is created, events are auto-applied to it where possible. If the
					// TrackingIgnorePastEvents preference is turned on, then past events are ignored. This creates our dilemma:
					// "If a user creates a new ladder, and moves it into the past before relevant events occured, then those
					// events are never auto-applied to the ladder."
					//
					// To solve this, we check to see if the proper preferences are turned on; and that the user moved the ladder
					// backwards in time. If so, try to synchronize the ladder with those events which are now in the future. This 
					// will not remove existing steps.
					//
					COleDateTime dtOld = VarDateTime(varOldValue);
					COleDateTime dtNew = VarDateTime(varNewValue);
					// We only care about the dates
					dtOld.SetDate(dtOld.GetYear(), dtOld.GetMonth(), dtOld.GetDay());
					dtNew.SetDate(dtNew.GetYear(), dtNew.GetMonth(), dtNew.GetDay());
					if(GetRemotePropertyInt("TrackingIgnorePastEvents",1,0,"<None>",true) &&
						GetRemotePropertyInt("AutoCompleteNextStep", 1, 0, "<None>", true) &&
						dtNew < dtOld)
					{
						// (c.haag 2013-11-26) - PLID 59822 - We no longer pass in the patient ID
						TryToCompleteAllLadderSteps(nLadderID);
					}
				}
				else if(VarLong(varType) == 2 && 
					(varOldValue.vt != varNewValue.vt ||
					(varOldValue.vt == VT_DATE && varNewValue.vt == VT_DATE && VarDateTime(varNewValue) != VarDateTime(varOldValue)))) {
					//TES 9/19/2007 - PLID 27433 - For some insane reason, this was updating the CompletedDate if the
					// step was done.  It now only updates the date that, you know, the user actually edited.
					long nLadderID = VarLong(m_procedureList->Value[nRow][CP_LadderID], -1);
					long nStepID = VarLong(m_procedureList->Value[nRow][CP_StepID], -1);
					ExecuteSql("UPDATE StepsT SET ActiveDate = '%s' WHERE ID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), nStepID);
					CString strOldDate;
					if(varOldValue.vt == VT_DATE) {
						strOldDate = FormatDateTimeForInterface(VarDateTime(varOldValue), DTF_STRIP_SECONDS, dtoDate);
					}
					//TES 9/19/2007 - PLID 27434 - Audit changing dates.
					AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiStepActiveDate, nStepID, strOldDate, FormatDateTimeForInterface(VarDateTime(varNewValue), 0, dtoDate, false), aepMedium, aetChanged);
					//TES 9/19/2007 - PLID 27433 - We  now need to do some extra stuff iff this is the active step.
					if(GetNextStepID(nLadderID) == nStepID) {
						
						//Find the top row for this ladder, and redraw it.
						bool bRowFound = false;
						for(int i = nRow; i>=0 && !bRowFound; i--) {
							if(VarLong(m_procedureList->Value[i][CP_RowType]) == 1) {
								long nLadderID = VarLong(m_procedureList->Value[i][CP_LadderID]);
								long nProcInfoID = VarLong(m_procedureList->Value[i][CP_ProcInfoID]);
								long nPicID = VarLong(m_procedureList->Value[i][CP_PicID]);
								//This is the row, let's delete and insert it.
								m_procedureList->SetRedraw(FALSE);
								//(j.anspach 06-02-2006 11:23 PLID 20902) - Was passing the nProcInfoID in place of the nPicID
								//  and this was causing errors because it switched the ID for the ladder.  I just pulled the
								//  nPicID from the procedure list and passed it into the function.
								// (j.jones 2008-09-03 17:11) - PLID 10417 - this is one of the few cases where we will call InsertTopLevel_FromData,
								// which will query the ladder information from a recordset
								InsertTopLevel_FromData(i, nLadderID, nPicID, nProcInfoID, false);
								// (a.wetta 2006-06-02 09:37) - PLID 20899 - The InsertTopLevel function inserts a row for the main parent row of of the 
								// steps in a ladder and it also inserts a blank row after it.  So, we need to remove the old parent row and the
								// blank row from the procedure list.  These will be the next two rows after the new top level row.
								m_procedureList->RemoveRow(i+1);
								m_procedureList->RemoveRow(i+1);
								m_procedureList->SetRedraw(TRUE);
								bRowFound = true;
							}
						}
						SyncTodoWithLadder(VarLong(m_procedureList->GetValue(nRow, CP_LadderID)));							
					}
				}
			}
			break;
		
		case CP_DoneDate:
			if(bCommit) {
				varType = m_procedureList->Value[nRow][CP_RowType];
				if (varType.vt == VT_EMPTY || varType.vt == VT_NULL) {
					return;
				}
				//TES 9/18/2007 - PLID 27431 - The user can now edit the CompletedDate, only on steps (the ladder's
				// CompletedDate is just calculated from the steps, so it can't be edited).
				// (j.jones 2010-10-11 16:41) - PLID 35424 - shouldn't do anything if the date did not change
				if(VarLong(varType) == 2 && 
					(varOldValue.vt != varNewValue.vt ||
					(varOldValue.vt == VT_DATE && varNewValue.vt == VT_DATE && VarDateTime(varNewValue) != VarDateTime(varOldValue)))) {
					long nLadderID = VarLong(m_procedureList->Value[nRow][CP_LadderID], -1);
					long nStepID = VarLong(m_procedureList->Value[nRow][CP_StepID], -1);
					ExecuteSql("UPDATE StepsT SET CompletedDate = '%s' WHERE ID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), nStepID);
					CString strOldDate;
					if(varOldValue.vt == VT_DATE) {
						strOldDate = FormatDateTimeForInterface(VarDateTime(varOldValue), DTF_STRIP_SECONDS, dtoDate);
					}
					//TES 9/19/2007 - PLID 27434 - Audit changing dates.
					AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiStepCompletedDate, nStepID, strOldDate, FormatDateTimeForInterface(VarDateTime(varNewValue), 0, dtoDate, false), aepMedium, aetChanged);
					//The ladder's completed date may have changed, refresh it.
					// (j.jones 2010-10-11 16:40) - PLID 35424 - changed to use a member
					// variable tablechecker so our dialog does not need to reload the screen
					m_tcLadders.Refresh(nLadderID);
					//we do not want to handle the resulting tablechecker,
					//this also means that any outstanding Ladder tablecheckers that
					//have not been processed yet will be ignored
					m_tcLadders.Changed();
				}
			}
			break;
		}

	}NxCatchAll("Error in CPatientProcedureDlg::OnEditingFinishedProcedureList");
	
}

void CPatientProcedureDlg::OnAdd()
{
	//(e.lally 2008-09-19) PLID 31386 - Permissions for creating a new ladder are checked in OnNewProcedure
	OnNewProcedure();
}

void CPatientProcedureDlg::OnDelete()
{
	OnDeleteProcedure();
}

void CPatientProcedureDlg::OnGo() 
{
	if (m_procedureList->CurSel == -1)
		return;

	Go(m_procedureList->CurSel);
}

void CPatientProcedureDlg::OnGotoProcedure() 
{
	OnGo();
}

void CPatientProcedureDlg::Go(long nRow)
{
	try{
		long nStepID;
		long nRowType = VarLong(m_procedureList->Value[nRow][CP_RowType], -1);
		long nPicID = VarLong(m_procedureList->GetValue(nRow, CP_PicID), -1);
		if(nRowType == -1) {
			return;
		}
		long action = -1;
		BOOL bOpenPIC = FALSE;
		//TES 7/16/2010 - PLID 39400 - Track the default scope.  Also, updated all scope references in this function to use the new enum.
		MergeTemplateScope mtsDefaultScope = (MergeTemplateScope)-1;

		if(nRowType == 1) {
			nStepID = PhaseTracking::GetNextStepID(VarLong(m_procedureList->Value[nRow][CP_LadderID]));
			if(nStepID == -1) {
				MessageBox("This ladder has been completed.");
				return;
			}
			// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
			// (j.jones 2008-11-17 17:30) - PLID 30926 - added OpenPIC
			//TES 7/16/2010 - PLID 39400 - Added DefaultScope
			_RecordsetPtr rsAction = CreateParamRecordset("SELECT Action, OpenPIC, DefaultScope FROM StepTemplatesT WHERE ID = (SELECT StepTemplateID FROM StepsT WHERE ID = {INT})", nStepID);
			action = AdoFldLong(rsAction, "Action", -1);
			bOpenPIC = AdoFldBool(rsAction, "OpenPIC", FALSE);
			mtsDefaultScope = (MergeTemplateScope)AdoFldLong(rsAction, "DefaultScope", GetRemotePropertyInt("Tracking_DefaultMergeScope",mtsPic, 0, "<None>"));

			// (j.jones 2008-11-18 08:40) - PLID 30926 - if bOpenPIC is true, open the PIC instead of launching the action
			if(bOpenPIC) {
				if(nPicID != -1) {
					GetMainFrame()->OpenPIC(nPicID);
					Refresh(); //The ladder name may have changed, for one thing.
				}
				return;
			}
		}
		else {
			nStepID = VarLong(m_procedureList->Value[nRow][CP_StepID]);
			//Check for events that are finished and NOT skipped.
			long nEventID = VarLong(m_procedureList->GetValue(nRow, CP_EventID), -1);
			long nEventType = VarLong(m_procedureList->GetValue(nRow, CP_EventType), -1);
			// (j.jones 2008-11-17 17:30) - PLID 30926 - added OpenPIC (needed now, will also be loaded again when the action is loaded)
			bOpenPIC = VarBool(m_procedureList->GetValue(nRow, CP_OpenPIC), FALSE);
			//TES 7/16/2010 - PLID 39400 - Added DefaultScope
			mtsDefaultScope = (MergeTemplateScope)VarLong(m_procedureList->GetValue(nRow, CP_DefaultScope), GetRemotePropertyInt("Tracking_DefaultMergeScope",mtsPic, 0, "<None>"));

			// (j.jones 2008-11-18 08:40) - PLID 30926 - if bOpenPIC is true, open the PIC instead of launching the action
			if(bOpenPIC) {
				if(nPicID != -1) {
					GetMainFrame()->OpenPIC(nPicID);
					Refresh(); //The ladder name may have changed, for one thing.
				}
				return;
			}

			if(nEventType != -1) {//This step has been completed.		
				if(((EventType)nEventType == ET_TemplateSent ||
					(EventType)nEventType == ET_PacketSent)
					&& GetRemotePropertyInt("TrackingAlwaysMerge", 0, 0, "<None>", true)) {
					//We don't want to link, we want to do the standard merge behavior, so just do nothing, and let the code
					//below handle this.
				}
				else {					
					// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
					if (nEventType == ET_BillCreated && GetMainFrame()->IsBillingModuleOpen(true)) {
						return;
					}
					GetMainFrame()->LinkToEvent((EventType)nEventType, nEventID);
					return;
				}
			}
			// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
			_RecordsetPtr rsAction = CreateParamRecordset("SELECT Action FROM StepTemplatesT WHERE ID = (SELECT StepTemplateID FROM StepsT WHERE ID = {INT})", nStepID);
			action = AdoFldLong(rsAction, "Action", -1);
		}
		if (action == -1)//next step done or missing
			return;

		switch(action)
		{
			case PA_Quote:
			case PA_QuoteSurgery:
			case PA_QuoteCPT:
			case PA_QuoteInventory:
				{
					//(e.lally 2008-09-19) PLID 31386 - Check for permission to create quotes
					if(!CheckCurrentUserPermissions(bioPatientQuotes,sptCreate)){
						return;
					}
					CBillingModuleDlg dlg(this);
					dlg.OpenWithBillID(-1, BillEntryType::Quote, 2);
					Refresh();
				}
				break;
			case PA_Bill:
			case PA_BillSurgery:
			case PA_BillCPT:
			case PA_BillInventory:
			case PA_BillQuote:
				{
					//(e.lally 2008-09-19) PLID 31386 - Check for permission to create bills
					if(!CheckCurrentUserPermissions(bioBill,sptCreate)){
						return;
					}					
					// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
					if (GetMainFrame()->IsBillingModuleOpen(true)) {
						return;
					}
					// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
					if(GetMainFrame()->GetActiveView()) {
						m_pBillingDlg = ((CPatientView*)(GetMainFrame()->GetActiveView()))->GetBillingDlg();
						m_pOldFinancialDlg = m_pBillingDlg->m_pFinancialDlg;
						m_pBillingDlg->m_pFinancialDlg = this;
						m_pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);
					}
					Refresh();
				}
				break;
			case PA_ScheduleAptCategory:
			case PA_ScheduleAptType:
			case PA_ActualAptCategory:
			case PA_ActualAptType:
				//(e.lally 2008-09-19) PLID 31386 - Flip to module does the permission checking. We don't need to repeat that here.
				// (z.manning 2011-09-01 15:25) - PLID 43347 - We now call ActivateTab to do this.
				g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::MonthTab);
				break;
			case PA_WriteTemplate:				
			case PA_WritePacket:
				{
					//(e.lally 2008-09-19) PLID 31386 - Check for permission to merge (history write permission).
					if(!CheckCurrentUserPermissions(bioPatientHistory, sptWrite)){
						return;
					}
/***********************************************************************************************************************
/*
/* WARNING: Whenever you modify the logic in this code, please make sure it's also modified in the code for merging
/*          from the PIC.  And yes, I know it should be the same code, there's a PL Item for that, but in the
/*          meantime, keep it synchronized.
/*
/***********************************************************************************************************************/

					CString strType;
					if(action == PA_WriteTemplate) {
						strType = "template";
					}
					else {
						strType = "packet";
					}
					CMergePromptDlg dlg(this);
					dlg.m_strDocumentType = strType;
					if(IDOK == dlg.DoModal()) {
						CDWordArray dwaActionIds;
						// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
						_RecordsetPtr rsActionIds = CreateParamRecordset("SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = (SELECT StepTemplateID FROM StepsT WHERE ID = {INT})", nStepID);
						if(rsActionIds->eof) {
							if(action == PA_WriteTemplate)
								MsgBox("There is no template specified for this step.");
							else
								MsgBox("There is no packet specified for this step.");
							return;
						}
						else {
							dwaActionIds.Add(AdoFldLong(rsActionIds, "ActionID"));
							rsActionIds->MoveNext();
							if(!rsActionIds->eof) {
								//Whoops!  We have options here.
								dwaActionIds.RemoveAll();
								// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
								CMultiSelectDlg dlg(this, "MergeTemplatesT");
								if(action == PA_WriteTemplate) {
									CString strWhere;
									strWhere.Format("ID IN (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = (SELECT StepTemplateID FROM StepsT WHERE ID = %li))", nStepID);
									if(IDOK != dlg.Open("MergeTemplatesT", strWhere, "ID", "Path", "Please select one or more templates to merge.", 1)) {
										return;
									}
								}
								else {
									CString strWhere;
									strWhere.Format("ID IN (SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = (SELECT StepTemplateID FROM StepsT WHERE ID = %li))", nStepID);
									if(IDOK != dlg.Open("PacketsT", strWhere, "ID", "Name", "Please select one or more packets to merge.", 1)) {
										return;
									}
								}
								dlg.FillArrayWithIDs(dwaActionIds);
							}
						}
						rsActionIds->Close();

						//We now have the list of ids they wish to merge.
						CString strIdList;
						for(int i=0; i < dwaActionIds.GetSize(); i++) {
							CString strId;
							strId.Format("%li", (long)dwaActionIds.GetAt(i));
							strIdList += strId + ", ";
						}
						strIdList = strIdList.Left(strIdList.GetLength()-2);

						_RecordsetPtr rsDocuments;
						if(action == PA_WritePacket) {
							if(!ReturnsRecords("SELECT ID FROM PacketsT WHERE ID IN (%s)", strIdList)) {
								MsgBox("This step references a packet which does not exist.  It may have been deleted.");
								return;
							}
							// (b.cardillo 2005-05-20 12:51) - PLID 15553 - Also return the total count of components 
							// in each packet.
							// (j.jones 2008-09-03 11:26) - PLID 10417 - this query cannot be parameterized
							rsDocuments = CreateRecordset(
								"SELECT MergeTemplatesT.Path, PacketComponentsT.Scope, PacketID, (SELECT COUNT(*) FROM PacketComponentsT A WHERE A.PacketID = PacketComponentsT.PacketID) AS ArticleCountInPacket "
								"FROM PacketComponentsT INNER JOIN MergeTemplatesT ON PacketComponentsT.MergeTemplateID = MergeTemplatesT.ID "
								"WHERE PacketID IN(%s) "
								"ORDER BY PacketID, PacketComponentsT.ComponentOrder", strIdList);
						}
						else if(action == PA_WriteTemplate) {
							// (b.cardillo 2005-05-20 12:53) - PLID 15553 - For a template by itself (not in a packet) 
							// we return the number of components as 1.  This may never be used because if you're 
							// merging a single template, that template is probably not set up to pull any packet-
							// specific fields, but it might happen, and this avoids an annoying error, while giving a 
							// logical result, if it does happen.
							// (j.jones 2008-09-03 11:26) - PLID 10417 - this query cannot be parameterized
							//TES 7/16/2010 - PLID 39400 - Use the DefaultScope we loaded up above.
							rsDocuments = CreateRecordset("SELECT MergeTemplatesT.Path, %li AS Scope, -1 AS PacketID, 1 AS ArticleCountInPacket FROM MergeTemplatesT WHERE ID IN (%s)", (long)mtsDefaultScope, strIdList);
							if(rsDocuments->eof) {
								MsgBox("This step references an invalid template.  Please go to Administrator->Tracking to correct this problem");
								return;
							}
						}

						FieldsPtr fDocs = rsDocuments->Fields;
						
						long nArticleNumber = 1;
						long nnArticleCount = 1;

						long nCurrentPacketID = -1;
						long nPreviousPacketID = -1;
						long nMergedPacketID = -1;

						bool bAtLeastOneMerged = false; //Some templates may be skipped, if they ALL are, we need to tell the user why.
						//TES 12/21/2006 - PLID 23957 - Let them see the overall progress.
						long nDocCount = 0;
						if(!rsDocuments->eof) {
							rsDocuments->MoveLast();
							rsDocuments->MoveFirst();
							nDocCount = rsDocuments->GetRecordCount();
						}
						if(nDocCount == 0) {
							MsgBox("The selected packet does not contain any templates.\nNo documents will be created.");
							return;
						}
						long nCurrentDoc = 0;
						while(!rsDocuments->eof) {
							nCurrentDoc++;
							// Get template to merge to
							CString strTemplateName = AdoFldString(fDocs, "Path");
							
							//if it starts with a drive letter or "\\" then it's an absolute path, otherwise, it's relative to the shared path.
							if(strTemplateName.GetAt(0) == '\\' && strTemplateName.GetAt(1) != '\\') {
								strTemplateName = GetSharedPath() ^ strTemplateName;
							}
					
							if(!DoesExist(strTemplateName) ) {
								MsgBox("The template '%s' could not be found.  This template will be skipped.", strTemplateName);
							}
							else {			
								/// Generate the temp table
								CString strSql;
								strSql.Format("SELECT ID FROM PersonT WHERE ID = %li", GetActivePatientID());
								CString strMergeT = CreateTempIDTable(strSql, "ID");
						
								// Merge
								CMergeEngine mi;

								// (z.manning, 03/06/2008) - PLID 29131 - Need to load the sender merge fields
								mi.LoadSenderInfo(FALSE);

								bool bMerge = true; //We may want to skip this template.
						
								mi.m_arydwProcInfoIDs.RemoveAll();
								// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
								_RecordsetPtr rsProcs = CreateParamRecordset("SELECT ID, ProcedureID FROM ProcInfoDetailsT WHERE ProcedureID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID Is Null) AND ProcInfoID = (SELECT ProcInfoID FROM LaddersT WHERE ID = {INT})", VarLong(m_procedureList->Value[nRow][CP_LadderID]));
								MergeTemplateScope mtsScope = (MergeTemplateScope)AdoFldLong(fDocs, "Scope");
								//TES 7/17/2010 - PLID 39400 - Patient-scope merging shouldn't include any procedure information.
								if(mtsScope != mtsPic && mtsScope != mtsPatient) {
									// (b.cardillo 2005-05-18 14:35) - PLID 16557 - Even if this is a multi-procedure 
									// template, we are still generating ONE document, so we want the page numbers to 
									// be considered continuous, rather than resetting to 1 on each record.
									mi.m_nFlags |= BMS_CONTINUOUS_PAGE_NUMBERING;

									if(mtsScope == mtsMasterProcedure || mtsScope == mtsProcedure || mtsScope == mtsDetailProcedure) {//1=per master procedure, 2=per procedure, 2=per detail procedure
										//add all procedures to the merge	
										while(!rsProcs->eof) {
											if(mtsScope == mtsProcedure || mtsScope == mtsDetailProcedure) {
												//Are there any detail procedures chosen?
												// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
												_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ProcInfoDetailsT.ID FROM ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID WHERE ProcInfoDetailsT.Chosen = 1 AND ProcInfoDetailsT.ProcInfoID = (SELECT ProcInfoID FROM LaddersT WHERE ID = {INT}) AND ProcedureT.MasterProcedureID = {INT}", VarLong(m_procedureList->Value[nRow][CP_LadderID]), AdoFldLong(rsProcs, "ProcedureID"));
												if(rsDetails->eof) {
													if(mtsScope == mtsProcedure) {
														//We're per procedure, so just use the master procedure.
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
												//We only want master procedures, regardless of what's chosen.
												mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
											}

											rsProcs->MoveNext();
										}
										if(mtsScope == mtsDetailProcedure && mi.m_arydwProcInfoIDs.GetSize() == 0) {
											//This is for detail procedures, but they don't have any. Skip this template.
											bMerge = false;
										}
									}
									else if(mtsScope == mtsPrescription) {//1 per medication
										//Add all medications to the merge.
										// (j.jones 2008-09-03 11:26) - PLID 10417 - parameterized this query
										_RecordsetPtr rsMeds = CreateParamRecordset("SELECT PatientMedications.ID "
											"FROM PatientMedications INNER JOIN EmrMedicationsT ON PatientMedications.ID = "
											"EmrMedicationsT.MedicationID INNER JOIN EmrMasterT ON EmrMedicationsT.EmrID = EmrMasterT.ID "
											"INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID "
											"WHERE EMRMasterT.Deleted = 0 AND EMRMedicationsT.Deleted = 0 AND PicT.ID = {INT}", VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_PicID), -1));
										while(!rsMeds->eof) {
											mi.m_arydwPrescriptionIDs.Add(AdoFldLong(rsMeds, "ID"));
											rsMeds->MoveNext();
										}
									}
								}
								else {
									//TES 7/17/2010 - PLID 39400 - Patient-scope merging shouldn't include any procedure information.
									if(mtsScope == mtsPic) {
										//just add one (arbitrary, this info shouldn't be merged but might be) procedure
										if(!rsProcs->eof)
											mi.m_arydwProcInfoIDs.Add(AdoFldLong(rsProcs, "ID"));
									}
								}
								rsProcs->Close();

								if(bMerge) {
									if (g_bMergeAllFields)
										mi.m_nFlags |= BMS_IGNORE_TEMPLATE_FIELD_LIST;

									//mi.m_nFlags |= BMS_SAVE_FILE; //save the file, do not save in history
									
									nCurrentPacketID = AdoFldLong(rsDocuments, "PacketID");
									if(nCurrentPacketID != nPreviousPacketID) {
										nMergedPacketID = NewNumber("MergedPacketsT", "ID");
										ExecuteSql("INSERT INTO MergedPacketsT (ID, PacketID) "
											"VALUES (%li, %li)", nMergedPacketID, nCurrentPacketID);
										// We've just started into the next packet, so return our article number to 1 
										// and resume counting from there.
										nArticleNumber = 1;
										nnArticleCount = AdoFldLong(rsDocuments->GetFields(), "ArticleCountInPacket");
									}
									nPreviousPacketID = nCurrentPacketID;
				
									// (b.cardillo 2005-05-20 12:46) - PLID 15553 - Include the article number and count 
									// as merge fields, and then increment the number.
									mi.m_lstExtraFields.RemoveAll();
									mi.m_lstExtraFields.Add("Packet_Article_Number", AsString(nArticleNumber));
									mi.m_lstExtraFields.Add("Packet_Article_Count", AsString(nnArticleCount));
									nArticleNumber++;

									try {
										// Do the merge
										if(dlg.m_bDirectToPrinter) {
											mi.m_nFlags = (mi.m_nFlags | BMS_MERGETO_PRINTER) & ~BMS_MERGETO_SCREEN;
										}
										mi.m_nFlags = (mi.m_nFlags | BMS_SAVE_FILE_AND_HISTORY);
										CString strExtraProgress;
										if(nDocCount > 1) {
											strExtraProgress.Format("Template %li of %li", nCurrentDoc, nDocCount);
										}
										// (z.manning 2016-06-03 8:41) - NX-100806 - Check if the merge was successful
										if (mi.MergeToWord(strTemplateName, std::vector<CString>(), strMergeT, "", nMergedPacketID, -1, false, strExtraProgress)) {
											bAtLeastOneMerged = true;
										}
										else {
											break;
										}
										
									}NxCatchAll("Error 2 Printing Procedure Packet");
								}
							}
							rsDocuments->MoveNext();

						} // END while(!rsDocuments->eof)

						if(bAtLeastOneMerged) {
							// (z.manning, 07/18/2006) - PLID 21498 - We used to do this within the loop, but then
							// we could get interrupted in the middle of merging a packet if the next step 
							// prompted the user for an activation date.
							if(nMergedPacketID != -1) {
								PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PacketSent, GetActivePatientID(), COleDateTime::GetCurrentTime(), nMergedPacketID);
							}
							Refresh();
						}
						else {
							MsgBox("All documents in this packet were specified to merge for detail procedures, but this PIC did not have any "
								"detail procedures selected (or the user canceled).  No action has been taken.");
						}
					}
				}
				break;

			case PA_Ladder:
				//(e.lally 2008-09-19) PLID 31386 - Permissions for creating a new ladder are checked in OnNewProcedure
				OnNewProcedure();
				break;

			case PA_Payment:
				try {
					if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) return;

					CPaymentDlg dlg(this);
					dlg.m_bIsPrePayment = true;
					dlg.m_boIsNewPayment = TRUE;
					dlg.m_PatientID = GetActivePatientID();
					dlg.m_iDefaultPaymentType = 0;
					dlg.m_iDefaultInsuranceCo = -1;
					int nReturn = dlg.DoModal(__FUNCTION__, __LINE__);
					if(nReturn != IDCANCEL) {
						//Make sure it's still a prepayment
						if(ReturnsRecords("SELECT LineItemT.ID FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID WHERE Type = 1 AND PrePayment = 1 AND LineItemT.ID = %li", VarLong(dlg.m_varPaymentID))) {
							ExecuteSql("INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) SELECT ProcInfoID, %li FROM LaddersT WHERE ID = %li", VarLong(dlg.m_varPaymentID), VarLong(m_procedureList->Value[nRow][CP_LadderID]));
							PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, GetActivePatientID(), COleDateTime::GetCurrentTime(),VarLong(dlg.m_varPaymentID),true,-1);
							Refresh();
						}
					}
					
				}NxCatchAll("Error in CPatientProcedureDlg::Go():PA_Payment");
				break;

			// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
			// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
			case PhaseTracking::PA_CreateEMRByCollection:
			case PhaseTracking::PA_CreateEMRByTemplate:
				{
					// (j.armen 2012-05-31 14:39) - PLID 50718 - Use the license using the helper function
					if(!g_pLicense->HasEMR(CLicense::cflrUse)) {
						MsgBox("You are not licensed to access this feature.");
						return;
					}
					//(e.lally 2008-09-19) PLID 31386 - Check for permission to create a new EMR
					if(!CheckCurrentUserPermissions(bioPatientEMR, sptCreate)){
						return;
					}
					long nProcInfoID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_ProcInfoID));
					if(nProcInfoID != -1) {
						//Decide which "mint" to use.
						//Whichever has the correct collection or template, and an intersecting procedure.

						// (j.jones 2005-11-04 15:57) - PLID 15311 - in addition to any EMN using this procedure,
						// we can also use any EMN with no procedures

						// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
						_RecordsetPtr rsActionIds = CreateParamRecordset("SELECT ActionID FROM StepCriteriaT WHERE StepTemplateID = (SELECT StepTemplateID FROM StepsT WHERE ID = {INT})", nStepID);
						CString strCollectionIDs, strTemplateIDs;
						while(!rsActionIds->eof) {
							CString strID;
							strID.Format("%li,",AdoFldLong(rsActionIds, "ActionID"));
							// (j.jones 2010-04-30 10:12) - PLID 38353 - this is either a collection ID, or template ID
							if(action == PhaseTracking::PA_CreateEMRByCollection) {
								strCollectionIDs += strID;
							}
							else if(action == PhaseTracking::PA_CreateEMRByTemplate) {
								strTemplateIDs += strID;
							}
							rsActionIds->MoveNext();
						}
						strCollectionIDs.TrimRight(",");
						strTemplateIDs.TrimRight(",");						

						// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this recordset when we have
						// no collection/template IDs, it cannot be paramaterized otherwise

						_RecordsetPtr rsValidTemplates;

						// (j.jones 2010-04-30 10:12) - PLID 38353 - we either search by collection, or by template
						if(action == PhaseTracking::PA_CreateEMRByCollection) {
							if(strCollectionIDs.IsEmpty()) {
								rsValidTemplates = CreateParamRecordset("SELECT ID FROM EmrTemplateT "
									"WHERE Deleted = 0 AND (ID IN "
									"(SELECT EmrTemplateID FROM EmrTemplateProceduresT WHERE ProcedureID IN "
									"(SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = {INT})) "
									"OR ID NOT IN (SELECT EmrTemplateID FROM EmrTemplateProceduresT))", nProcInfoID);
							}
							else {
								rsValidTemplates = CreateRecordset("SELECT ID FROM EmrTemplateT "
									"WHERE Deleted = 0 AND CollectionID IN (%s) "
									"AND (ID IN "
									"(SELECT EmrTemplateID FROM EmrTemplateProceduresT WHERE ProcedureID IN "
									"(SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = %li)) "
									"OR ID NOT IN (SELECT EmrTemplateID FROM EmrTemplateProceduresT))", strCollectionIDs, nProcInfoID);
							}
						}
						else if(action == PhaseTracking::PA_CreateEMRByTemplate) {
							if(strTemplateIDs.IsEmpty()) {
								rsValidTemplates = CreateParamRecordset("SELECT ID FROM EmrTemplateT "
									"WHERE Deleted = 0 AND (ID IN "
									"(SELECT EmrTemplateID FROM EmrTemplateProceduresT WHERE ProcedureID IN "
									"(SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = {INT})) "
									"OR ID NOT IN (SELECT EmrTemplateID FROM EmrTemplateProceduresT))", nProcInfoID);
							}
							else {
								rsValidTemplates = CreateRecordset("SELECT ID FROM EmrTemplateT "
									"WHERE Deleted = 0 AND ID IN (%s) "
									"AND (ID IN "
									"(SELECT EmrTemplateID FROM EmrTemplateProceduresT WHERE ProcedureID IN "
									"(SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = %li)) "
									"OR ID NOT IN (SELECT EmrTemplateID FROM EmrTemplateProceduresT))", strTemplateIDs, nProcInfoID);
							}
						}
						
						if(rsValidTemplates->eof) {
							MsgBox("There are no EMN templates which are valid to complete this step.");
							return;
						}
						long nTemplateID = AdoFldLong(rsValidTemplates, "ID");

						if(!rsValidTemplates->eof) {
							//OK, there are multiple.  Have them choose.

							// (j.jones 2010-04-30 10:12) - PLID 38353 - this never worked, it always showed all templates!
							CString strWhereClause = "EmrTemplateT.ID IN (";
							while(!rsValidTemplates->eof) {
								strWhereClause += AsString(AdoFldLong(rsValidTemplates, "ID"));
								strWhereClause += ",";
								rsValidTemplates->MoveNext();
							}
							rsValidTemplates->Close();

							strWhereClause.TrimRight(",");
							strWhereClause += ")";

							CSelectDlg dlg(this);
							dlg.m_strCaption = "There are multiple EMN templates which could complete this step.  Please select one to use.";
							dlg.m_strTitle = "Select EMN Template";
							dlg.m_strFromClause = "EmrTemplateT";
							dlg.m_strWhereClause = strWhereClause;
							DatalistColumn dcID, dcName;
							dcID.strField = "ID";
							dcID.strTitle = "ID";
							dcID.nWidth = 0;
							dcID.nStyle = csVisible|csFixedWidth;
							dcID.nSortPriority = -1;
							dcID.bSortAsc = TRUE;
							dlg.m_arColumns.Add(dcID);
							dcName.strField = "Name";
							dcName.strTitle = "Name";
							dcName.nWidth = -1;
							dcName.nStyle = csVisible|csWidthAuto;
							dcName.nSortPriority = 0;
							dcName.bSortAsc = TRUE;
							dcName.bWordWrap = true;
							dlg.m_arColumns.Add(dcName);
							if(IDOK == dlg.DoModal()) {
								nTemplateID = VarLong(dlg.m_arSelectedValues.GetAt(0));
							}
							else {
								return;
							}
						}

						//TES 11/4/2009 - PLID 35807 - Do they want to be warned if this is the NexWeb template?
						if(GetRemotePropertyInt("WarnWhenCreatingNexWebEmn", 1, 0, GetCurrentUserName())) {
							//TES 11/4/2009 - PLID 35807 - They do, so check whether it is.
							//(e.lally 2011-05-04) PLID 43537 - Use new NexWebDisplayT structure
							_RecordsetPtr rsNexWebTemplate = CreateParamRecordset("SELECT EmrTemplateID FROM NexWebDisplayT "
								"WHERE EmrTemplateID = {INT} AND Visible = 1 ", nTemplateID);
							if(!rsNexWebTemplate->eof) {
								// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
								if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: This template is a NexWeb template.  It is designed "
									"to be created and filled out by your patients, through your website.  "
									"Are you sure you wish to continue creating an EMN based on this template?")) {
									return;
								}
							}
						}

						//TES 4/5/2006 - Ensure that there is an EMRGroupsT record.
						long nPicID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_PicID), -1);
						// (a.walling 2008-06-27 14:36) - PLID 30549 - This has always been wrong by checking for Deleted = 0.
						// we want to check for Deleted = 1! Otherwise we are overwriting the EMR link for non-deleted EMRs!
						if(ReturnsRecordsParam("SELECT ID FROM PicT WHERE ID = {INT} AND "
							"(EmrGroupID Is Null OR EmrGroupID IN (SELECT ID FROM EMRGroupsT WHERE Deleted = 1))", nPicID))
						{
							
							CParamSqlBatch sql;

							// (j.armen 2013-05-07 16:59) - PLID 56587 - EMRGroupsT.ID is now an identity.  Let's also parameterize
							sql.Declare("DECLARE @EMRGroupsT_ID TABLE(ID INT NOT NULL)");
							sql.Add(
								"INSERT INTO EMRGroupsT (PatientID, Description, Status, InputDate) \r\n"
								"OUTPUT inserted.ID INTO @EMRGroupsT_ID \r\n"
								"SELECT {INT}, '', 0, getdate()", GetActivePatientID());

							sql.Add("UPDATE PicT SET EMRGroupID = (SELECT ID FROM @EMRGroupsT_ID) WHERE ID = {INT}", nPicID);

							sql.Execute(GetRemoteData());
						}
						//Now we have a template, open up the PIC and start that template.
						GetMainFrame()->EditEmrRecord(nPicID, -1, nTemplateID);
						Refresh(); //The ladder name may have changed, for one thing.
					}

				}
				break;
		}
	}NxCatchAll("Error in CPatientProcedureDlg::Go");
}

// (j.jones 2008-09-03 16:58) - PLID 10417 - removed the recordset from this function and passed in all the data it needs
// (j.jones 2008-12-01 08:57) - PLID 32262 - changed nUserID to strUserNames
int CPatientProcedureDlg::InsertTopLevel(int nStartRow, long nLadderID, long nPicID, long nProcInfoID,
										 CString strLadderName, BOOL bHasProcedure, long nNextStepID,
										 CString strStatus, BOOL bIsActive, COleDateTime dtDate,
										 CString strNotes, _variant_t varDoneDate, CString strUserNames,
										 bool bAddSubRows /*=true*/)
{
	try{
		//OK, let's construct this row, one column at a time.
		IRowSettingsPtr pRow = m_procedureList->GetRow(-1);
		
		//Color
		pRow->PutBackColor(DARK_COLOR);
//		pRow->PutBackColorSel(GetLighterColor(DARK_COLOR));
//		pRow->PutForeColorSel(0x00000000);

		_variant_t varNull;
		varNull.vt = VT_NULL;
		
		//RowType
		pRow->PutValue(CP_RowType, (long)1);

		//LadderID
		pRow->PutValue(CP_LadderID, nLadderID == -1 ? varNull : nLadderID);
		
		//PicID, 
		pRow->PutValue(CP_PicID, nPicID == -1 ? varNull : nPicID); //PicID should never be null, but it's not prevented by the data, so we should handle it.

		//ProcInfoT.ID
		pRow->PutValue(CP_ProcInfoID, nProcInfoID);

		//StepID
		//Not applicable at this level
		pRow->PutValue(CP_StepID, varNull);

		//Top-level arrows.
		if(nLadderID != -1)
			pRow->PutValue(CP_TopArrow, IsRowExpanded(nLadderID, 1) ? "BITMAP:UPARROW" : "BITMAP:DOWNARROW");
		else
			pRow->PutValue(CP_TopArrow, varNull);

		//Name (ladder name, list of procedures in ladder)
		pRow->PutValue(CP_Name, _bstr_t(strLadderName));
		if(!bHasProcedure) {
			pRow->PutCellLinkStyle(CP_Name, dlLinkStyleFalse);
		}
		
		//Done? (at this level, is the entire ladder done?
		if(nLadderID == -1) {
			pRow->PutValue(CP_Done, varNull);
		}
		else {
			if(nNextStepID == -1) {
				pRow->PutValue(CP_Done, true);
			}
			else {
				pRow->PutValue(CP_Done, false);
			}
		}

		//Description (at this level, status)
		SetLadderDescription(pRow, nLadderID, nNextStepID, strStatus, bIsActive);

		//Date (at this level, FirstInterestDate or EmrDate)
		if(dtDate.GetStatus() == COleDateTime::invalid) {
			pRow->PutValue(CP_Date, varNull);
		}
		else {
			pRow->PutValue(CP_Date, _variant_t(dtDate, VT_DATE));
		}

		//Notes 
		pRow->PutValue(CP_Notes, _bstr_t(strNotes));

		//Completed Date (if the ladder is done, the date the last step was completed).
		if(nNextStepID == -1) {
			pRow->PutValue(CP_DoneDate, varDoneDate);
		}
		else {
			pRow->PutValue(CP_DoneDate, varNull);
		}

		//UserNames
		// (j.jones 2008-12-01 08:57) - PLID 32262 - changed UserID to strUserNames,
		// though the top level only has one user
		pRow->PutValue(CP_UserNames, (LPCTSTR)strUserNames);

		//Step order (meaningless, at this level).
		pRow->PutValue(CP_StepOrder, varNull);

		//Applied event ID and type (meaningless, at this level).
		pRow->PutValue(CP_EventID, varNull);
		pRow->PutValue(CP_EventType, varNull);

		//Skippable (meaningless, at this level).
		pRow->PutValue(CP_Skippable, varNull);

		// (j.jones 2008-11-17 17:48) - PLID 30926 - added OpenPIC column
		pRow->PutValue(CP_OpenPIC, varNull);

		//TES 7/16/2010 - PLID 39400 - Added DefaultScope
		pRow->PutValue(CP_DefaultScope, varNull);

		//All right, we've got a good row, let's add it.
		m_procedureList->InsertRow(pRow, nStartRow);
		int nNextRow = nStartRow+1;

		//Do we need to add more?
		if(nLadderID != -1 && IsRowExpanded(nLadderID, 1) && bAddSubRows) {
			//Fine, let's go through and add each step.
			// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query, then combined the recordset from
			// InsertMidLevel into this recordset, so InsertMidLevel can just take in the values from here
			// (j.jones 2008-11-17 17:50) - PLID 30926 - added OpenPIC
			// (j.jones 2008-11-26 17:26) - PLID 32262 - added dbo.GetStepUsernames
			//TES 7/16/2010 - PLID 39400 - Added DefaultScope
			_RecordsetPtr rsSteps = CreateParamRecordset("SELECT StepsT.ID, "
				"'     ' + StepTemplatesT.StepName AS StepName, StepTemplatesT.Action, "
				"StepTemplatesT.Note, StepTemplatesT.Skippable, EventsT.ItemID, EventsT.Type, StepsT.ActiveDate, "
				"StepsT.CompletedDate, StepsT.Note, StepsT.StepOrder, StepTemplatesT.OpenPIC, "
				"dbo.GetStepUsernames(StepsT.ID) AS Usernames, StepTemplatesT.DefaultScope "
				"FROM StepsT "
				"LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID "
				"WHERE StepsT.LadderID = {INT} "
				"ORDER BY StepsT.StepOrder", nLadderID);
			while(!rsSteps->eof) {

				long nStepID = AdoFldLong(rsSteps, "ID", -1);
				long nEventID = AdoFldLong(rsSteps, "ItemID", -1);
				long nEventType = AdoFldLong(rsSteps, "Type", -1);
				CString strStepName = AdoFldString(rsSteps, "StepName", "");
				long nAction = AdoFldLong(rsSteps, "Action", -1);
				_variant_t varActiveDate = rsSteps->Fields->Item["ActiveDate"]->Value;
				CString strNote = AdoFldString(rsSteps, "Note", "");
				_variant_t varCompletedDate = rsSteps->Fields->Item["CompletedDate"]->Value;
				long nStepOrder = AdoFldLong(rsSteps, "StepOrder", -1);
				_variant_t varSkippable = rsSteps->Fields->Item["Skippable"]->Value;
				_variant_t varOpenPIC = rsSteps->Fields->Item["OpenPIC"]->Value;
				_variant_t varDefaultScope = rsSteps->Fields->Item["DefaultScope"]->Value;

				// (j.jones 2008-11-26 17:26) - PLID 32262 - changed nUserID to strUserNames
				CString strUserNames = AdoFldString(rsSteps, "Usernames", "");;

				nNextRow = InsertMidLevel(nNextRow, nLadderID, nPicID, nProcInfoID, nStepID,
					nEventID, nEventType, strStepName, nAction, varActiveDate, strNote,
					varCompletedDate, strUserNames, nStepOrder, varSkippable, varOpenPIC, varDefaultScope);

				rsSteps->MoveNext();
			}
		}

		//Insert blank row.
		pRow = m_procedureList->GetRow(-1);
		pRow->PutValue(CP_LadderID, varNull);  //Look, I don't know why I'm doing this, either.  Talk to Bob, he'll convince you that this is right.
		pRow->PutValue(CP_RowType, varNull);
		pRow->PutCellLinkStyle(CP_Name, dlLinkStyleFalse);
		m_procedureList->InsertRow(pRow, nNextRow);
		nNextRow++;

		//OK, we're done.
		return nNextRow;
	}NxCatchAllCall("Error in CPatientProcedureDlg::InsertTopLevel", return -1;);
}

// (j.jones 2008-09-03 16:54) - PLID 10417 - this function should be called as infrequently as possible,
// it will re-load the ladder from data, using a recordset, instead of passed-in values
int CPatientProcedureDlg::InsertTopLevel_FromData(int nStartRow, long nLadderID, long nPicID, long nProcInfoID, bool bAddSubRows /*=true*/)
{
	try {

		// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
		_RecordsetPtr rsLadder = CreateParamRecordset("SELECT dbo.CalcProcInfoName(ProcInfoT.ID) AS LadderName, LaddersT.ID, "
			"LaddersT.PersonID, LadderStatusT.Name AS Status, LaddersT.Notes, FirstInterestDate, LaddersT.Name, "
			"UsersT.UserName, EmrGroupsT.InputDate AS EmrDate, "
			"CASE WHEN EXISTS (SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID) THEN 1 ELSE 0 END AS HasProcedure, "
			"LadderStatusT.IsActive, (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) AS NextStep, "
			"(SELECT Max(CompletedDate) AS DoneDate FROM StepsT WHERE LadderID = LaddersT.ID) AS DoneDate "
			"FROM ProcInfoT LEFT JOIN (LaddersT INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID) ON ProcInfoT.ID = LaddersT.ProcInfoID "
			"LEFT JOIN ((SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID) ON ProcInfoT.ID = PicT.ProcInfoID "
			"LEFT JOIN UsersT ON LaddersT.UserID = UsersT.PersonID "
			"WHERE ProcInfoT.ID = {INT}", nProcInfoID);

		if(!rsLadder->eof) {

			COleDateTime dtDate;

			if(rsLadder->Fields->GetItem("FirstInterestDate")->Value.vt == VT_DATE) {
				dtDate = VarDateTime(rsLadder->Fields->GetItem("FirstInterestDate")->Value);
			}
			else {
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				dtDate = VarDateTime(rsLadder->Fields->GetItem("EmrDate")->Value, dtInvalid);
			}

			// (j.jones 2008-12-01 08:57) - PLID 32262 - changed nUserID to strUserNames
			return InsertTopLevel(nStartRow, nLadderID, nPicID, nProcInfoID, 
				AdoFldString(rsLadder, "LadderName", ""), (BOOL)AdoFldLong(rsLadder, "HasProcedure"), AdoFldLong(rsLadder, "NextStep", -1),
				AdoFldString(rsLadder, "Status",""), AdoFldBool(rsLadder, "IsActive",FALSE), dtDate,
				AdoFldString(rsLadder, "Notes",""), rsLadder->Fields->GetItem("DoneDate")->Value, AdoFldString(rsLadder, "UserName", ""),
				bAddSubRows);
		}
		rsLadder->Close();

		return -1;

	}NxCatchAllCall("Error in CPatientProcedureDlg::InsertTopLevel_FromData", return -1;);
}

// (j.jones 2008-09-04 08:59) - PLID 10471 - InsertMidLevel now no longer runs a recordset to get its data,
// and can instead take in all the information it needs.
// (j.jones 2008-11-17 17:50) - PLID 30926 - added varOpenPIC
// (j.jones 2008-11-26 16:00) - PLID 32262 - changed nUserID to strUserNames
//TES 7/16/2010 - PLID 39400 - Added varDefaultScope
int CPatientProcedureDlg::InsertMidLevel(int nStartRow, long nLadderID, long nPicID, long nProcInfoID, long nStepID, 
										 long nEventID, long nEventType, CString strStepName, long nAction,
										 _variant_t varActiveDate, CString strNote, _variant_t varCompletedDate,
										 CString strUserNames, long nStepOrder, _variant_t varSkippable, _variant_t varOpenPIC,
										 _variant_t varDefaultScope,
										 bool bAddSubRows  /*=true*/)
{
	try{
		_variant_t varNull;
		varNull.vt = VT_NULL;

		//Let's go through this one column at a time.
		IRowSettingsPtr pRow = m_procedureList->GetRow(-1);
		pRow->PutBackColor(LIGHT_COLOR);
//		pRow->PutBackColorSel(GetLighterColor(LIGHT_COLOR));
//		pRow->PutForeColorSel(0x00000000);
		
		//RowType
		pRow->PutValue(CP_RowType, (long)2);

		//LadderID
		pRow->PutValue(CP_LadderID, nLadderID);

		//PicT.ID
		pRow->PutValue(CP_PicID, nPicID == -1 ? varNull : nPicID);

		//ProcInfoT.ID
		pRow->PutValue(CP_ProcInfoID, nProcInfoID);

		//StepID
		pRow->PutValue(CP_StepID, nStepID);

		//Top-level arrows (always detail lines, at this level).
		pRow->PutValue(CP_TopArrow, "BITMAP:DETAILLINE");
		//Darken
		pRow->PutCellBackColor(CP_TopArrow, DARK_COLOR);
//		pRow->PutCellBackColorSel(CP_TopArrow, GetLighterColor(DARK_COLOR));
		
		//Name
		//Step name, indented.
		bool bEventApplied = (nEventType == -1) ? false : true;
		bool bSkipped = false;
		bool bLink = true;
		
		if(bEventApplied) {
			if(nEventType == ET_Skipped) bSkipped = true;
			if(!IsEventLinkable((EventType)nEventType)) {
				bLink = false;
			}
		}

		pRow->PutValue(CP_Name, _bstr_t(strStepName));
		// (j.jones 2008-11-18 09:31) - PLID 30926 - it is linkable if bOpenPIC is true
		BOOL bOpenPIC = FALSE;
		if(varOpenPIC.vt == VT_BOOL) {
			bOpenPIC = VarBool(varOpenPIC);
		}
		if(!bOpenPIC && !IsActionLinkable((PhaseTracking::PhaseAction)nAction)) {
			bLink = false;
		}
		pRow->PutCellLinkStyle(CP_Name, bLink?dlLinkStyleTrue:dlLinkStyleFalse);


		//Done?
		pRow->PutValue(CP_Done, bEventApplied && !bSkipped);
		if(bSkipped) {
			pRow->PutCellBackColor(CP_Done, SKIPPED_COLOR);
		}

		//Description
		//TS 10/24/02: Since we're taking out the event level, this will be a.) if the step has been completed,
		//a description of the event that completed it, and b.) if the step is not completed, a good description
		//of what needs to happen to complete the step.
		if(bEventApplied) {
			pRow->PutValue(CP_Description, _bstr_t(PhaseTracking::GetItemDescription((PhaseTracking::EventType)nEventType, nEventID)));
			if(bSkipped) {
				pRow->PutCellBackColor(CP_Description, SKIPPED_COLOR);
			}
		}
		else {
			pRow->PutValue(CP_Description, _bstr_t(PhaseTracking::GetPhaseActionDescription((PhaseAction)nAction)));   
		}

		//Date
		//The active date for this step.
		//TES 9/19/2007 - PLID 27433 - We only want to show the Active Date if this is a completed (or skipped) step,
		// or if it is the active step.
		if(varActiveDate.vt == VT_DATE) {
			if(bEventApplied || GetNextStepID(nLadderID) == nStepID) {
				if(VarDateTime(varActiveDate).GetYear() == 9999) {
					_variant_t varString;
					varString.SetString("On Hold");
					pRow->PutValue(CP_Date, varString);
				}
				else {
					pRow->PutValue(CP_Date, varActiveDate);
				}
			}
		}
		
		//Notes
		//Notes from Admin (this may or may not change at some point in the future)
		pRow->PutValue(CP_Notes, _bstr_t(strNote));
		if(bSkipped) {
			//pRow->PutCellBackColor(CP_Notes, SKIPPED_COLOR);
		}

		//DoneDate
		if(bEventApplied) {
			//We don't want to show a DoneDate if the step is skipped, because it was never done, now was it?
			if(!bSkipped) {				
				pRow->PutValue(CP_DoneDate, varCompletedDate); //If this is null, we'll leave it blank, that's fine.
			}
			else {
				pRow->PutCellBackColor(CP_DoneDate, SKIPPED_COLOR);
			}
		}
		else {
			pRow->PutValue(CP_DoneDate, varNull);
		}

		//UserName
		// (j.jones 2008-12-01 08:54) - PLID 32262 - convert this to show strUserNames
		pRow->PutValue(CP_UserNames, (LPCTSTR)strUserNames);
		if(bSkipped) {
			//pRow->PutCellBackColor(CP_UserNames, SKIPPED_COLOR);
		}

		//Step order
		pRow->PutValue(CP_StepOrder, nStepOrder);

		//Applied event ID and type.
		pRow->PutValue(CP_EventID, nEventID);
		pRow->PutValue(CP_EventType, nEventType);

		//Skippable.
		pRow->PutValue(CP_Skippable, varSkippable);

		// (j.jones 2008-11-17 17:48) - PLID 30926 - added OpenPIC column
		pRow->PutValue(CP_OpenPIC, varOpenPIC);

		//TES 7/16/2010 - PLID 39400 - Added DefaultScope
		pRow->PutValue(CP_DefaultScope, varDefaultScope);

		//All right, we've got a good row, let's add it.
		m_procedureList->InsertRow(pRow, nStartRow);
		int nNextRow = nStartRow+1;

		//OK, we're done.
		return nNextRow;
	}NxCatchAllCall("Error in CPatientProcedureDlg::InsertMidLevel", return -1);
}


/*void CPatientProcedureDlg::InsertEventLevel(int nRow, long nEventID, long nLadderID, long nStepID)
{
	try{
		//Let's go through this one column at a time.
		IRowSettingsPtr pRow = m_procedureList->GetRow(-1);
		pRow->PutBackColor(DARK_COLOR);
//		pRow->PutBackColorSel(GetLighterColor(DARK_COLOR));
//		pRow->PutForeColorSel(0x00000000);

		//RowType
		pRow->PutValue(CP_RowType, (long)3);
		//LadderID
		pRow->PutValue(CP_LadderID, nLadderID);

		//StepID
		pRow->PutValue(CP_StepID, nStepID);

		//Top-level arrows (always detail lines, at this level).
		pRow->PutValue(CP_TopArrow, "BITMAP:DETAILLINE");

		//Name
		_RecordsetPtr rsEvent = CreateRecordset("SELECT Type, Date, ItemID, Note FROM EventsT LEFT JOIN (SELECT EventID, Note FROM EventAppliesT WHERE StepID = %li) EventApply ON EventsT.ID = EventApply.EventID WHERE EventsT.ID = %li", nStepID, nEventID);
		pRow->PutValue(CP_Name, _bstr_t("        " + GetEventDescription((PhaseTracking::EventType)AdoFldLong(rsEvent, "Type"))));
		//If this is a type we don't link to, let's disable the linkage action.
		if(((PhaseTracking::EventType)AdoFldLong(rsEvent, "Type")) == PhaseTracking::ET_MarkedDone) {
			pRow->PutCellLinkStyle(CP_Name, dlLinkStyleFalse);
		}
		
		//Done?
		//Actually, we don't even want to have this at this level.
		_variant_t varTmp;
		varTmp.vt = VT_NULL;
		pRow->PutValue(CP_Done, varTmp);

		//Description
		pRow->PutValue(CP_Description, _bstr_t(GetItemDescription((PhaseTracking::EventType)AdoFldLong(rsEvent, "Type"), AdoFldLong(rsEvent, "ItemID"))));

		//Date
		pRow->PutValue(CP_Date, rsEvent->Fields->GetItem("Date")->Value);

		//Notes
		pRow->PutValue(CP_Notes, rsEvent->Fields->GetItem("Note")->Value);

		//OK, we've got a good row, let's add it.
		m_procedureList->InsertRow(pRow, nRow);

		//All done.
	}NxCatchAll("Error in CPatientProcedureDlg::InsertEventLevel");
}*/

bool CPatientProcedureDlg::IsRowExpanded(int nID, int nRowType)
{
	bool bReturnVal = false;
	if(nRowType == 1){
		m_mTopLevel.Lookup(nID, bReturnVal);
	}
	return bReturnVal;
}

void CPatientProcedureDlg::OnLeftClickProcedureListTrackable(long nRow, short nCol, long x, long y, long nFlags) 
{
	CString strArrow;
	_variant_t varArrow;
	switch(nCol) {
	case CP_TopArrow:
		//Is this currently a down arrow?
		varArrow = m_procedureList->GetValue(nRow, CP_TopArrow);
		if(varArrow.vt == VT_EMPTY || varArrow.vt == VT_NULL) return;
		strArrow = VarString(m_procedureList->GetValue(nRow, CP_TopArrow));
		if(strArrow == "BITMAP:DOWNARROW") {
			m_mTopLevel.SetAt(VarLong(m_procedureList->GetValue(nRow, CP_LadderID)), true);
			Refresh();
		}
		else if(strArrow == "BITMAP:UPARROW") {
			m_mTopLevel.SetAt(VarLong(m_procedureList->GetValue(nRow, CP_LadderID)), false);
			Refresh();
		}
		break;
	case CP_Name:
		{
			if (nRow == -1)
				return;
			_variant_t varTmp = m_procedureList->Value[nRow][CP_ProcInfoID];
			if(varTmp.vt == VT_NULL || varTmp.vt == VT_EMPTY) {
				return;
			}

			long nRowType = m_procedureList->Value[nRow][CP_RowType];
			if(nRowType == 1) {
				CProcedureInfo dlg(this);
				// (d.thompson 2009-02-17) - PLID 3843 - We want to be able to preview
				dlg.m_bAllowPreview = true;
				// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
				_RecordsetPtr rsProcedureID = CreateParamRecordset("SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = {INT}", VarLong(varTmp));
				while(!rsProcedureID->eof) {
					dlg.m_arProcIDs.Add(AdoFldLong(rsProcedureID, "ProcedureID"));
					rsProcedureID->MoveNext();
				}
				if(dlg.m_arProcIDs.GetSize())
					dlg.DoModal();
			}
			else {
				Go(nRow);
			}
		}
		break;
	// (j.jones 2008-12-01 08:53) - PLID 32262 - the User column is now a left click hyperlink, not a dropdown
	case CP_UserNames:
		{
			if(nRow == -1) {
				return;
			}

			if(!CheckCurrentUserPermissions(bioPatientTracking, sptWrite)) {
				return;
			}

			//What level are we at?
			_variant_t varType = m_procedureList->GetValue(nRow, CP_RowType);
			if (varType.vt == VT_EMPTY || varType.vt == VT_NULL) {
				return;
			}
			if(VarLong(varType) == 1) {
				//this is a ladder, get the ID
				long nLadderID = VarLong(m_procedureList->GetValue(nRow, CP_LadderID), -1);
				if(nLadderID == -1) {
					return;
				}

				long nUserID = -1, nNewUserID = -1;
				_variant_t varNewName = g_cvarNull;
				//find the existing ID
				_RecordsetPtr rs = CreateParamRecordset("SELECT UserID FROM LaddersT WHERE ID = {INT}", nLadderID);
				if(!rs->eof) {
					nUserID = AdoFldLong(rs, "UserID", -1);
				}
				rs->Close();

				CSingleSelectDlg dlg(this);
				dlg.PreSelect(nUserID);
				//TES 10/13/2008 - PLID 21093 - Pass in true to force them to select a record.
				if(IDOK == dlg.Open("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID", "PersonT.Archived = 0 AND PersonT.ID > 0", "PersonT.ID", "UsersT.UserName", 
					"Please select a user to be responsible for this ladder.", true)) {
					nNewUserID = dlg.GetSelectedID();
					varNewName = VarString(dlg.GetSelectedDisplayValue(), "");
				}
				else {
					return;
				}

				if(nNewUserID != nUserID) {
					ExecuteParamSql("UPDATE LaddersT SET UserID = {INT} WHERE ID = {INT}", nNewUserID, nLadderID);
					//TES 8/18/2009 - PLID 32912 - Make sure any ToDos get reassigned.
					SyncTodoWithLadder(VarLong(m_procedureList->GetValue(nRow, CP_LadderID)));
				}

				//update the interface
				m_procedureList->PutValue(nRow, CP_UserNames, varNewName);
			}
			else if(VarLong(varType) == 2) {
				//this is a step, get the ID
				long nStepID = VarLong(m_procedureList->GetValue(nRow, CP_StepID), -1);
				if(nStepID == -1) {
					return;
				}

				CArray<long, long> aryUserIDs, aryNewUserIDs;
				//find the existing IDs
				_RecordsetPtr rs = CreateParamRecordset("SELECT UserID FROM StepsAssignToT WHERE StepID = {INT}", nStepID);
				while(!rs->eof) {
					aryUserIDs.Add(AdoFldLong(rs, "UserID"));
					rs->MoveNext();
				}
				rs->Close();

				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "UsersT");
				dlg.PreSelect(aryUserIDs);
				if(IDOK == dlg.Open("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID", "PersonT.Archived = 0 AND PersonT.ID > 0", "PersonT.ID", "UsersT.UserName", 
					"Please select users to be responsible for this step.", 1)) {
						dlg.FillArrayWithIDs(aryNewUserIDs);
				}
				else {
					return;
				}

				//update the users for this step
				_variant_t varNewNames = g_cvarNull;
				CString strSqlBatch;
				CNxParamSqlArray aryParams;
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET NOCOUNT ON");
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM StepsAssignToT WHERE StepID = {INT}", nStepID);
				int i=0;
				for(i=0;i<aryNewUserIDs.GetSize();i++) {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO StepsAssignToT (UserID, StepID) "
						"VALUES ({INT}, {INT})", (long)aryNewUserIDs.GetAt(i), nStepID);
				}
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET NOCOUNT OFF");
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SELECT dbo.GetStepUsernames({INT}) AS Usernames", nStepID);
				
				// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
				rs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);

				if(!rs->eof) {
					varNewNames = rs->Fields->Item["Usernames"]->Value;
				}
				rs->Close();

				//TES 8/18/2009 - PLID 32912 - Make sure any ToDos get reassigned.
				SyncTodoWithLadder(VarLong(m_procedureList->GetValue(nRow, CP_LadderID)));
				
				m_procedureList->PutValue(nRow, CP_UserNames, varNewNames);
			}

		}
		break;
	}
}

void CPatientProcedureDlg::OnEditingFinishingProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol) {
	case CP_Done:
		if(VarLong(m_procedureList->GetValue(nRow, CP_RowType), -1) == 1) {
			if(VarBool(m_procedureList->GetValue(nRow, CP_Done)) ) {
				*pbContinue = false;
				break; // (r.galicki 2008-09-22 11:15) - PLID 27363
			}
		}
		 // (r.galicki 2008-09-22 10:49) - PLID 27363 - check write permissions upon editing
		if (!CheckCurrentUserPermissions(bioPatientTracking, sptWrite)) {
			*pbContinue = false;
		}
		break;
	case CP_Notes:
		//These are editable now.
		/*if(VarLong(m_procedureList->GetValue(nRow, CP_RowType), -1) == 2) {
			*pbCommit = false;
		}*/
		break;
	case CP_Date:
	//TES 9/19/2007 - PLID 27431 - The Completed Date is now editable.
	case CP_DoneDate:
		// (r.galicki 2008-09-22 15:58) - PLID 27363 - No longer need this check.  Permissions are checked in OnEditingStarting.
		//TES 9/19/2007 - PLID 27434 - Check that they have permission.
		//if(CheckCurrentUserPermissions(bioPatientTracking, sptDynamic0)) {
			COleDateTime dtNewDate;
			dtNewDate.ParseDateTime(strUserEntered);
			if(dtNewDate.GetStatus() == COleDateTime::invalid || dtNewDate < COleDateTime(1753,1,1,0,0,0)) {
				*pbCommit = false;
			}
		/*}
		//else {
		//	*pbCommit = false;
		}*/
		break;
	}

}


LRESULT CPatientProcedureDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case NXM_REFRESH_PATPROC:
		Refresh();
		break;

	// (c.haag 2009-08-12 10:41) - PLID 35157 - Listen for when we enter the popup menu
	case WM_ENTERMENULOOP:
		m_bPopupMenuVisible = TRUE;
		break;

	// (c.haag 2009-08-12 10:41) - PLID 35157 - Listen for when we exit the popup menu.
	// When we're out, also refresh the tab if necessary.
	case WM_EXITMENULOOP:
		m_bPopupMenuVisible = FALSE;
		if (m_bNeedPostPopupRefresh) {
			m_bNeedPostPopupRefresh = FALSE;
			PostMessage(NXM_REFRESH_PATPROC);
		}
		break;
	}
	return CPatientDialog::WindowProc(message, wParam, lParam);
}

void CPatientProcedureDlg::OnRButtonDownProcedureListTrackable(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (r.galicki 2008-09-02 15:01) - PLID 27363 - Disable right click menu if read-only
	if(!m_bReadOnly) {
		try {

			m_procedureList->CurSel = nRow;

			//This only works if: 1.) This is an actual row, 2.) It's not a blank row, 3.) It's a mid-level row, and 4.) It isn't done.
			if(nRow != -1) { //1.)
				if(VarLong(m_procedureList->GetValue(nRow, CP_LadderID), -1) != -1) { //2.)
					if(VarLong(m_procedureList->GetValue(nRow, CP_RowType), -1) == 2) { //3.)
						if(!VarBool(m_procedureList->GetValue(nRow, CP_Done))) { //4.)
							m_procedureList->CurSel = nRow;
							OnSelChangedProcedureListTrackable(m_procedureList->CurSel);
							CMenu mPopup;
							mPopup.CreatePopupMenu();
							mPopup.InsertMenu(0, MF_BYPOSITION, ID_SHOW_INFO, "Procedure Information Center");
							mPopup.InsertMenu(1, MF_BYPOSITION, ID_APPLY_EVENT, "Apply existing event");
							mPopup.InsertMenu(2, MF_BYPOSITION, ID_MARK_DONE, "Mark step as Completed");
							//Is this row skipped?
							CString strSql;
							strSql.Format("SELECT ID FROM EventAppliesT WHERE StepID = %li", VarLong(m_procedureList->GetValue(nRow, CP_StepID)));
							bool bEventApplied = false;
							if(ReturnsRecords(strSql)) {
								bEventApplied = true;
							}
							bool bSkipped = false;
							if(bEventApplied) {
								long nType = VarLong(m_procedureList->GetValue(nRow, CP_EventType), -1);
								if(nType == PhaseTracking::ET_Skipped) {
									bSkipped = true;
								}
							}
							if(bSkipped) {
								mPopup.InsertMenu(3, MF_BYPOSITION, ID_UNSKIP_STEP, "Mark step as Active");
							}
							else {
								//CAN this step be skipped?
								// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
								_RecordsetPtr rsSkippable = CreateParamRecordset("SELECT Skippable FROM StepTemplatesT WHERE ID = (SELECT StepTemplateID FROM StepsT WHERE ID = {INT})", VarLong(m_procedureList->GetValue(nRow, CP_StepID)));
								if(AdoFldBool(rsSkippable, "Skippable", 0) != 0) {
									mPopup.InsertMenu(3, MF_BYPOSITION, ID_SKIP_STEP, "Mark step as Skipped");
								}
								else {
									mPopup.InsertMenu(3, MF_BYPOSITION|MF_GRAYED, ID_SKIP_STEP, "Mark step as Skipped");
								}
							}

							CPoint pt;
							GetCursorPos(&pt);
							mPopup.SetDefaultItem(ID_SHOW_INFO);
							mPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
						}
						else {
							CMenu mPopup;
							mPopup.CreatePopupMenu();
							mPopup.InsertMenu(0, MF_BYPOSITION, ID_SHOW_INFO, "Procedure Information Center");
							mPopup.SetDefaultItem(ID_SHOW_INFO);
							CPoint pt;
							GetCursorPos(&pt);
							mPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
						}
					}
					else if(VarLong(m_procedureList->GetValue(nRow, CP_RowType), -1) == 1) {//Top level
						m_procedureList->CurSel = nRow;
						OnSelChangedProcedureListTrackable(m_procedureList->CurSel);
						CMenu mPopup;
						mPopup.CreatePopupMenu();
						mPopup.InsertMenu(0, MF_BYPOSITION, ID_SHOW_INFO, "Procedure Information Center");
						mPopup.InsertMenu(1, MF_BYPOSITION, ID_CHANGE_STATUS, "Change status");
						mPopup.InsertMenu(2, MF_BYPOSITION, ID_DELETE, "Delete");
						//If this ladder isn't done or on hold.
						long nStepID = GetNextStepID(VarLong(m_procedureList->GetValue(nRow, CP_LadderID)));
						if(nStepID != -1) {
							if(!ReturnsRecords("SELECT ID FROM StepsT WHERE ActiveDate > getdate() AND ID = %li", nStepID)) {
								mPopup.InsertMenu(3, MF_BYPOSITION, ID_PUT_ON_HOLD, "Put On Hold...");
							}
							else {
								mPopup.InsertMenu(3, MF_BYPOSITION, ID_ACTIVATE, "Activate");
							}
						}
						mPopup.InsertMenu(4, MF_BYPOSITION, ID_MERGE, "Merge Ladder");
						CPoint pt;
						GetCursorPos(&pt);
						mPopup.SetDefaultItem(ID_SHOW_INFO);
						mPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
					}
				}
			}
		}NxCatchAll("Error in CPatientProcedureDlg::OnRButtonDown()");
	}
}

void CPatientProcedureDlg::OnApplyEvent()
{
	try {
		CEventSelectDlg dlg(this);
		long nStepID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_StepID));
		// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
		_RecordsetPtr rsStep = CreateParamRecordset("SELECT StepsT.StepTemplateID, StepTemplatesT.Action FROM StepsT INNER JOIN "
			"StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE StepsT.ID = {INT}", nStepID);
		dlg.m_nStepTemplateID = AdoFldLong(rsStep, "StepTemplateID");
		dlg.m_nAction = (PhaseAction)AdoFldLong(rsStep, "Action");
		rsStep->Close();
		dlg.m_nPatientID = GetActivePatientID();
		dlg.m_nLadderID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID));

		if(dlg.DoModal() == IDOK) {

			//Now we need to insert it into the data.
			CreateAndApplyEvent(GetEventType(dlg.m_nAction), GetActivePatientID(), dlg.m_dtDate, dlg.m_nItemID, true, nStepID);
			
			//PostMessage(NXM_REFRESH_PATPROC);
		}
	}NxCatchAll("Error in CPatientProcedureDlg::OnApplyEvent");
}

void CPatientProcedureDlg::OnEditingStartingProcedureListTrackable(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	//First, if we're on a non-laddered row, none of these fields are editable.
	if(m_procedureList->GetValue(nRow, CP_LadderID).vt != VT_I4) {
		*pbContinue = false;
		return;
	}

	long nRowType;

	// (r.galicki 2008-09-22 16:04) - PLID 27363 - After change in how permissions are checked, flag no longer needed.
	//												Removed m_bReadOnly check as well.
	// (r.galicki 2008-09-02 15:02) - PLID 27363 - Since Edit Date permissions are handled seperately,
	// need a new flag to determine if editing is allowed.  Prevent editing other columns if in read-only mode
	//BOOL bEditDates = FALSE;

	_variant_t varType;

	switch(nCol) {
	case CP_Notes:
		nRowType = VarLong(m_procedureList->Value[nRow][CP_RowType], -1);
		//"Mid"-level rows are editable now.
		if(/*nRowType == 2 || */nRowType == -1) {
			*pbContinue = false;
		}
		// (r.galicki 2008-09-22 10:49) - PLID 27363 - check write permissions upon editing
		else if (!CheckCurrentUserPermissions(bioPatientTracking, sptWrite)) {
			*pbContinue = false;
		}
		break;
	case CP_Date:
		nRowType = VarLong(m_procedureList->Value[nRow][CP_RowType], -1);
		if(nRowType == -1) {
			*pbContinue = false;
		}
		// (r.galicki 2008-09-22 15:38) - PLID 27363 - Alert user if permissions do not exist.
		else if(CheckCurrentUserPermissions(bioPatientTracking, sptDynamic0)) {
		//TES 9/19/2007 - PLID 27434 - Make sure they have permission to edit dates.
		//else if(GetCurrentUserPermissions(bioPatientTracking) & (sptDynamic0|sptDynamic0WithPass)) {
			//bEditDates = TRUE;
			if(nRowType == 2) {			
				//we can't edit if we're not either done or active.
				//TES 9/19/2007 - PLID 27433 - We can also edit if we're skipped.
				if(VarLong(m_procedureList->Value[nRow][CP_EventType]) == -1) {
					long nLadderID = VarLong(m_procedureList->Value[nRow][CP_LadderID], -1);
					long nStepID = VarLong(m_procedureList->Value[nRow][CP_StepID], -1);
					if(GetNextStepID(nLadderID) != nStepID) {
						*pbContinue = false;
					}
				}
			}
		}
		else {
			*pbContinue = false;
		}
		break;

	case CP_DoneDate:
		nRowType = VarLong(m_procedureList->Value[nRow][CP_RowType], -1);
		//TES 9/19/2007 - PLID 27431 - The Completed Date is now editable, only for steps.
		if(nRowType != 2) {
			*pbContinue = false;
		}
		// (r.galicki 2008-09-22 15:38) - PLID 27363 - Alert user if permissions do not exist.
		else if(CheckCurrentUserPermissions(bioPatientTracking, sptDynamic0)) {
		//TES 9/19/2007 - PLID 27434 - Make sure they have permission to edit dates.
		//else if(GetCurrentUserPermissions(bioPatientTracking) & (sptDynamic0|sptDynamic0WithPass)) {
			//bEditDates = TRUE;
			//TES 9/19/2007 - PLID 27431 - We can't edit if we're not done (skipped doesn't count).
			long nType = VarLong(m_procedureList->Value[nRow][CP_EventType]);
			if(nType == ET_Skipped || nType == -1) {
				*pbContinue = false;
			}
		}
		else {
			*pbContinue = false;
		}
		break;
	}
}

void CPatientProcedureDlg::OnChangeStatus() {
	CSelectStatusDlg dlg(this);
	dlg.DoModal();
	if(dlg.m_nStatusID != -1) {
		//Audit change.
		long nLadderID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID));
		// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
		_RecordsetPtr rsStatus = CreateParamRecordset("SELECT Name FROM LadderStatusT WHERE ID = (SELECT Status FROM LaddersT WHERE ID = {INT})", nLadderID);
		CString strOldStatus;
		if(!rsStatus->eof) {
			strOldStatus = AdoFldString(rsStatus, "Name", "");
		}
		// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
		rsStatus = CreateParamRecordset("SELECT Name FROM LadderStatusT WHERE ID = {INT}", dlg.m_nStatusID);
		CString strNewStatus;
		if(!rsStatus->eof) {
			strNewStatus = AdoFldString(rsStatus, "Name", "");
		}
		// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
		ExecuteParamSql("UPDATE LaddersT SET Status = {INT} WHERE ID = {INT}", dlg.m_nStatusID, nLadderID);
		
		if(strOldStatus != strNewStatus) 
			AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiLadderStatus, nLadderID, strOldStatus, strNewStatus, aepMedium, aetChanged);
		SyncTodoWithLadder(VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID)));
		// (j.jones 2010-10-11 16:44) - PLID 35424 - refresh the tablechecker and handle
		// the change ourselves instead of being dependent on the tablechecker receipt
		m_tcLadders.Refresh(nLadderID);
		//we do not want to handle the resulting tablechecker,
		//this also means that any outstanding Ladder tablecheckers that
		//have not been processed yet will be ignored
		m_tcLadders.Changed();
		RefreshLadder(nLadderID);
	}
}

void CPatientProcedureDlg::SecureControls()
{
	// (r.galicki 2008-09-02 11:02) - PLID 27363 - In order to allow more complete viewing of the tracking list, a 
	// flag is set rather than just disabling the entire control. Various checks are made in event calls of this dialog.
	// (r.galicki 2008-09-19 15:35) - PLID 27363 - Change to how permissions are checked.  Unless write permissions exists, make readonly
	
	CPermissions permsTracking = GetCurrentUserPermissions(bioPatientTracking);
	// Return if we have either type of write access
	if(permsTracking & sptWrite) { 
		return;
	}
	if(permsTracking & sptWriteWithPass) { 
		return;
	}
	// Otherwise...make the controls read-only
	m_bReadOnly = TRUE;
	//GetDlgItem(IDC_PROCEDURE_LIST_TRACKABLE)->EnableWindow(FALSE);
	GetDlgItem(IDC_NEW_PROCEDURE)->EnableWindow(FALSE);
	GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);
	GetDlgItem(IDC_DELETE_PROCEDURE)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHOW_PROC_INFO)->EnableWindow(FALSE);

	/*
	CPermissions permsTracking = GetCurrentUserPermissions(bioPatientTracking);
	// Return if we have write access
	if(permsTracking & sptWrite) { 
		return;
	}

	// Also return if we have write access with password
	if(permsTracking & sptReadWithPass) {
		// (z.manning, 08/03/2006) - PLID 21641 - They have read with password permissions, so they
		// just entered their password.  No reason to prompt them again.
		if(permsTracking & sptWriteWithPass) {
			return;
		}
	}
	else { 	// Otherwise, they have read without a password, so we need to fully check the write permission.

		// (r.galicki 2008-09-02 12:17) - PLID 27363 - If a user has read-only permissions (no write permissions),
		// making a call to CheckCurrentUserPermissions causes an errant message box to display.  Checking for the permission
		// before the call prevents this message box, but will prompt for password if necessary.
		if(permsTracking & sptWriteWithPass) {
			if(CheckCurrentUserPermissions(bioPatientTracking, sptWrite)) {
				return;
			}

		}
	}*/
}

void CPatientProcedureDlg::OnShowProcInfo() 
{
	try {
		if(m_procedureList->CurSel != -1) {
			if(m_procedureList->GetValue(m_procedureList->CurSel, CP_RowType).vt == VT_NULL) {
				return;		//blank line
			}

			long nPicID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_PicID));
			if(nPicID != -1) {
				GetMainFrame()->OpenPIC(nPicID);
				Refresh(); //The ladder name may have changed, for one thing.
			}
		}
	}NxCatchAll("Error in CPatientProcedureDlg::OnShowProcInfo()");
}

void CPatientProcedureDlg::OnDblClickCellProcedureListTrackable(long nRowIndex, short nColIndex) 
{
	// (r.galicki 2008-09-02 15:10) - PLID 27363 - Disable double-click if read-only
	if(!m_bReadOnly && nColIndex != CP_TopArrow && nColIndex != CP_Done)
		OnShowProcInfo();
}

void CPatientProcedureDlg::OnRequeryFinishedProcedureListTrackable(short nFlags) 
{
	//OK, let's never do that again!;
	m_procedureList->FromClause = _bstr_t("");
	m_procedureList->WhereClause = _bstr_t("");

}

void CPatientProcedureDlg::OnUnskipStep()
{
	try {
		//Nothing fancy, let's just delete any "Skipped" type applies to this step.
		int nCurSel = m_procedureList->CurSel;
		if(nCurSel != -1) {
			_variant_t varType = m_procedureList->GetValue(nCurSel, CP_RowType);
			if(varType.vt == VT_I4) {
				if(VarLong(varType) == 2) {
					//OK, great, this is a step.
					PhaseTracking::UnapplyEvent(GetActivePatientID(), PhaseTracking::ET_Skipped, -1, VarLong(m_procedureList->GetValue(nCurSel, CP_StepID)));
					//ExecuteSql("DELETE FROM EventAppliesT WHERE StepID = %li AND EventID IN (SELECT ID FROM EventsT WHERE Type = %li)", VarLong(m_procedureList->GetValue(nCurSel, CP_StepID)), PhaseTracking::ET_Skipped);
				}
			}
		}
		//Refresh();
	}NxCatchAll("Error in OnUnskipStep()");
}

void CPatientProcedureDlg::OnSkipStep()
{
	try {
		//Nothing fancy, let's just add a "Skipped" entry.
		int nCurSel = m_procedureList->CurSel;
		if(nCurSel != -1) {
			_variant_t varType = m_procedureList->GetValue(nCurSel, CP_RowType);
			if(varType.vt == VT_I4) {
				if(VarLong(varType) == 2) {
					//OK, great, this is a step. We'll assume it's active and skippable, otherwise we shouldn't have gotten here.
					long nLadderID = VarLong(m_procedureList->GetValue(nCurSel, CP_LadderID));
					long nStepID = VarLong(m_procedureList->GetValue(nCurSel, CP_StepID));
					PhaseTracking::CompleteStep(PhaseTracking::ET_Skipped, GetActivePatientID(), COleDateTime::GetCurrentTime(), -1, nStepID, COleDateTime::GetCurrentTime(), nLadderID);
					long nStepOrder = VarLong(m_procedureList->GetValue(nCurSel, CP_StepOrder));
					PhaseTracking::SetActiveDate(nLadderID, nStepID, nStepOrder, true);
					
					//Also, we have to notify the todo list.
					PhaseTracking::SyncTodoWithLadder(nLadderID);
				}
			}
		}
		//Refresh();
	}NxCatchAll("Error in OnSkipStep()");
}

// (a.walling 2007-11-07 09:26) - PLID 27998 - VS2008 - These should be WPARAM/LPARAM
LRESULT CPatientProcedureDlg::OnPostEditBill(WPARAM iBillID, LPARAM iSaveType)
{
	try {
		//(r.wilson 8/5/2013) PLID 51531 - Need to make sure that m_pBillingDlg is not NULL before attempting to access its member variables. This will fix the problem
		//								   that was because of null pointers.
		if(m_pBillingDlg != NULL){
			//Now, clean up our tracks, so that billing will never know we "borrowed" its dialog.
			m_pBillingDlg->m_pFinancialDlg = m_pOldFinancialDlg;
		}
		m_pOldFinancialDlg = NULL;
			

		m_pBillingDlg = NULL; //Don't worry about cleanup; that's up to patient view.

		Refresh();
	}NxCatchAll("Error in CProcInfoCenterDlg::OnPostEditBill()");
	return 0;
}

void CPatientProcedureDlg::OnSelChangedProcedureListTrackable(long nNewSel) 
{
	try {
		//Enable/Disable the "Go" button.
		if(nNewSel == -1) {
			GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);
			GetDlgItem(IDC_DELETE_PROCEDURE)->EnableWindow(FALSE);
		}
		else {
			_variant_t varType = m_procedureList->GetValue(nNewSel, CP_RowType);
			if(varType.vt == VT_I4) {
				if(VarLong(varType) == 1) {
					if(m_procedureList->GetValue(nNewSel, CP_LadderID).vt == VT_I4) {
						//This is a ladder.  Does the next step have a linkable action?
						// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
						// (j.jones 2008-11-17 17:41) - PLID 30926 - added OpenPIC
						_RecordsetPtr rsAction = CreateParamRecordset("SELECT Action, OpenPIC FROM StepTemplatesT INNER JOIN StepsT ON StepTemplatesT.ID = StepsT.StepTemplateID WHERE StepsT.ID = {INT}", GetNextStepID(VarLong(m_procedureList->GetValue(nNewSel, CP_LadderID))));
						// (r.galicki 2008-09-02 15:13) - PLID 27363 - Use read-only flag instead of checking for disabled control.
						if(!rsAction->eof && !m_bReadOnly) { // GetDlgItem(IDC_PROCEDURE_LIST_TRACKABLE)->IsWindowEnabled()) { // (z.manning, 08/03/2006) - PLID 21641 - Don't want to re-enable this if they don't have write permissions.
							// (j.jones 2008-11-17 17:41) - PLID 30926 - enable the button if OpenPIC is true, or the existing IsActionLinkable behavior
							GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(AdoFldBool(rsAction, "OpenPIC", FALSE) || IsActionLinkable((PhaseTracking::PhaseAction)AdoFldLong(rsAction, "Action")));
						}
						else {
							GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);
						}
					}
					else {
						//m.hancock - 4/9/2006 - PLID 20048 - Bad variable type when clicking Go for a row in tracking tab that does not have a ladder
						//This is not a ladder, so disable the GO button.
						GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);
					}
					// (z.manning, 08/03/2006) - PLID 21641 - Don't want to re-enable this if they don't
					// have write permissions. (There isn't a tracking delete permission yet.)

					// (r.galicki 2008-09-02 15:13) - PLID 27363 - Use read-only flag instead of checking for disabled control.
					if(!m_bReadOnly) { //GetDlgItem(IDC_PROCEDURE_LIST_TRACKABLE)->IsWindowEnabled()) {
						GetDlgItem(IDC_DELETE_PROCEDURE)->EnableWindow(TRUE);
					}
				}
				else if(VarLong(varType) == 2) {
					//This is a step.  Is it completed?
					long nEventType = VarLong(m_procedureList->GetValue(nNewSel, CP_EventType), -1);
					BOOL bOpenPIC = VarBool(m_procedureList->GetValue(nNewSel, CP_OpenPIC), FALSE);
					// (r.galicki 2008-09-02 15:13) - PLID 27363 - Use read-only flag instead of checking for disabled control.
					if(!m_bReadOnly) { //GetDlgItem(IDC_PROCEDURE_LIST_TRACKABLE)->IsWindowEnabled()) { // (z.manning, 08/03/2006) - PLID 21641 - Don't want to re-enable this if they don't have write permissions.
						if(nEventType == -1) {
							//No.  Is its action linkable?

							// (j.jones 2008-11-17 17:41) - PLID 30926 - enable the button if OpenPIC is true, or the existing IsActionLinkable behavior
							if(bOpenPIC) {
								GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(TRUE);
							}
							else {
								// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query							
								_RecordsetPtr rsAction = CreateParamRecordset("SELECT Action FROM StepTemplatesT INNER JOIN StepsT ON StepTemplatesT.ID = StepsT.StepTemplateID WHERE StepsT.ID = {INT}", VarLong(m_procedureList->GetValue(nNewSel, CP_StepID)));							
								GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(IsActionLinkable((PhaseTracking::PhaseAction)AdoFldLong(rsAction, "Action")));
							}
						}
						else {
							//Yes.  Was it completed by a linkable step?
							// (j.jones 2008-11-17 17:44) - PLID 30926 - check the local OpenPIC setting
							if(bOpenPIC) {
								GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(TRUE);
							}
							else {
								GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(IsEventLinkable((PhaseTracking::EventType)nEventType));
							}
						}
					}
					GetDlgItem(IDC_DELETE_PROCEDURE)->EnableWindow(FALSE);
				}
				else {
					GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);
					GetDlgItem(IDC_DELETE_PROCEDURE)->EnableWindow(FALSE);
				}
			}
			else {
				GetDlgItem(IDC_GOTO_PROCEDURE)->EnableWindow(FALSE);
				GetDlgItem(IDC_DELETE_PROCEDURE)->EnableWindow(FALSE);
			}
		}
	}NxCatchAll("Error in CPatientProceduredlg::OnSelChangedProcedureList()");
}

void CPatientProcedureDlg::OnPutOnHold()
{
	try {
		if(m_procedureList->CurSel != -1 && m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID).vt == VT_I4) {
			long nLadderID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID));
			long nStepID = GetNextStepID(nLadderID);
			if(nStepID != -1) {
				CEnterActiveDateDlg dlg(this);
				dlg.m_strPrompt = "When should the ladder become active again?";
				COleDateTime dtTomorrow = COleDateTime::GetCurrentTime() + COleDateTimeSpan(1,0,0,0);
				dtTomorrow.SetDateTime(dtTomorrow.GetYear(), dtTomorrow.GetMonth(), dtTomorrow.GetDay(), 0, 0, 0);
				dlg.m_dtDate = dtTomorrow;
				dlg.m_bAllowCancel = true;
				dlg.m_bAllowPastDate = false;	//DRT 10/28/2003 - Don't allow previous things to save, it just immediately activates them.
				if(IDOK == dlg.DoModal()) {
					ExecuteSql("UPDATE StepsT SET ActiveDate = '%s' WHERE ID = %li", _Q(FormatDateTimeForSql(dlg.m_dtDate)), nStepID);
					SyncTodoWithLadder(nLadderID);
					// (j.jones 2010-10-11 16:44) - PLID 35424 - refresh the tablechecker and handle
					// the change ourselves instead of being dependent on the tablechecker receipt
					m_tcLadders.Refresh(nLadderID);
					//we do not want to handle the resulting tablechecker,
					//this also means that any outstanding Ladder tablecheckers that
					//have not been processed yet will be ignored
					m_tcLadders.Changed();
					RefreshLadder(nLadderID);
				}
			}
		}
	}NxCatchAll("Error in CPatientProcedureDlg::OnPutOnHold()");
}

void CPatientProcedureDlg::OnActivate()
{
	try {
		if(m_procedureList->CurSel != -1 && m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID).vt == VT_I4) {
			long nLadderID = VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_LadderID));
			long nStepID = GetNextStepID(nLadderID);
			if(nStepID != -1) {
				ExecuteSql("UPDATE StepsT SET ActiveDate = '%s' WHERE ID = %li", _Q(FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate)), nStepID);
				SyncTodoWithLadder(nLadderID);
				//TES 8/12/2014 - PLID 63224 - Removed this tablechecker
				//CClient::RefreshTable(NetUtils::StepsT, nStepID);
				//Refresh();
			}
			RefreshLadder(nLadderID);
		}
	}NxCatchAll("Error in CPatientProcedureDlg::OnActivate()");
}

LRESULT CPatientProcedureDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) 
{
	try {
		//
		// (c.haag 2009-08-12 10:43) - PLID 35157 - If you add new types here, make sure you check
		// the m_bPopupMenuVisible flag!
		//

		switch (wParam) {
		case NetUtils::LaddersT:
			if (m_bPopupMenuVisible) {
				m_bNeedPostPopupRefresh = TRUE; // (c.haag 2009-08-12 11:06) - PLID 35157
			}
			else {
				try {
					// (j.jones 2010-10-11 16:42) - PLID 35424 - respond to the tablechecker
					// only if our tracked m_tcLadders value has changed (PeekChanged does
					// not reset the Changed flag, which is what we want in this scenario)
					if (m_tcLadders.PeekChanged()) {
						long nLadderID = (long)lParam;
						//Is this ladder one of ours?
						// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
						_RecordsetPtr rsPatientID = CreateParamRecordset("SELECT PersonID FROM LaddersT WHERE ID = {INT}", nLadderID);
						if (rsPatientID->eof) {
							//It's been deleted. Get it out of our list.
							RemoveLadder(nLadderID);
						}
						else {
							long nPatientID = AdoFldLong(rsPatientID, "PersonID");
							if (nPatientID == GetActivePatientID()) {
								//This is one of ours, refresh it.
								RefreshLadder(nLadderID);
							}
						}
					}
				} NxCatchAll("Error in CPatientProcedureDlg::OnTableChanged:LaddersT");
			}
			break;
		}

		//TES 8/12/2014 - PLID 63224 - I removed StepsT and ProcInfoT handling, we should not be responding to tablecheckers automatically here

	} NxCatchAll("Error in CPatientProcedureDlg::OnTableChanged()");

	return 0;
}

void CPatientProcedureDlg::OnActiveLadders() 
{
	Refresh();
	SetRemotePropertyInt("DefaultTrackingView", 0, 0, GetCurrentUserName());
}

void CPatientProcedureDlg::OnInactiveLadders() 
{
	Refresh();
	SetRemotePropertyInt("DefaultTrackingView", 1, 0, GetCurrentUserName());
}

void CPatientProcedureDlg::OnAllLadders() 
{
	Refresh();
	SetRemotePropertyInt("DefaultTrackingView", 2, 0, GetCurrentUserName());
}

void CPatientProcedureDlg::OnColumnClickingProcedureListTrackable(short nCol, BOOL FAR* bAllowSort) 
{
	switch(nCol) {
	case CP_TopArrow:
		{
			POSITION pos = m_mTopLevel.GetStartPosition();
			bool bExpandedFound = false;
			while(pos != NULL) {
				long nIndex;
				bool bExpanded;
				m_mTopLevel.GetNextAssoc(pos, nIndex, bExpanded);
				if(bExpanded) bExpandedFound = true;
			}
			//Now, loop through and set everything.
			pos = m_mTopLevel.GetStartPosition();
			while(pos != NULL) {
				long nIndex;
				bool bExpanded;
				m_mTopLevel.GetNextAssoc(pos, nIndex, bExpanded);
				m_mTopLevel.SetAt(nIndex, !bExpandedFound);
			}
			Refresh();
		}
		*bAllowSort = FALSE;
		break;

	default:
		*bAllowSort = FALSE;
		break;
	}
}

void CPatientProcedureDlg::OnTrackingRememberColWidths() 
{
	//save the setting
	long nRemember = 0;	//default off
	if(IsDlgButtonChecked(IDC_TRACKING_REMEMBER_COL_WIDTHS))
		nRemember = 1;
	else
		nRemember = 0;
	SetRemotePropertyInt("RememberTrackingColumns", nRemember, 0, GetCurrentUserName());

	//size the datalist appropriately
	if(!IsDlgButtonChecked(IDC_TRACKING_REMEMBER_COL_WIDTHS)) {
		ResetColumnSizes();
	}
	else {
		m_procedureList->SetRedraw(FALSE);
		//Make sure all the styles are fixed-width type styles.
		for (short i=0; i < m_procedureList->ColumnCount; i++)
		{
			long nStyle = m_procedureList->GetColumn(i)->ColumnStyle;
			nStyle &= ~(csWidthPercent | csWidthAuto);
			m_procedureList->GetColumn(i)->ColumnStyle = nStyle;
		}
		SetColumnSizes();
		m_procedureList->SetRedraw(TRUE);
	}
}

void CPatientProcedureDlg::OnColumnSizingFinishedProcedureListTrackable(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth) 
{
	//TES 2/23/2004: Plagiarized from AppointmentsDlg
	//DRT 6/6/03 - Saves the sizes of all columns if bCommitted is set and the checkbox is set

	//uncommitted
	if(!bCommitted)
		return;

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_TRACKING_REMEMBER_COL_WIDTHS))
		return;

	
	
	//save width of each column
	IColumnSettingsPtr pCol;
	CString str, strList;

	for(int i = 0; i < m_procedureList->GetColumnCount(); i++) {
		pCol = m_procedureList->GetColumn(i);
		if(pCol)
			str.Format("%li,", pCol->GetStoredWidth());

		strList += str;
	}

	//write it to ConfigRT
	SetRemotePropertyText("DefaultTrackingColumnSizes", strList, 0, GetCurrentUserName());

	SetColumnSizes();
}

void CPatientProcedureDlg::ResetColumnSizes()
{
	//TES 2/23/04 - Plagiarized from AppointmentsDlg.
	//DRT - 6/6/03 - This function takes the saved m_strOriginalColSizes and
	//		resets all columns to those sizes.

	if(m_strOriginalColSizes.IsEmpty()) {
		//not sure why we wouldn't have any, but better to leave them as is
		//than to set to empty
		return;
	}

	CString strCols = m_strOriginalColSizes;
	int nWidth = 0, i = 0;

	//parse the columns out and set them
	int nComma = strCols.Find(",");
	while(nComma > 0) {
		nWidth = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_procedureList->GetColumn(i);
		if(pCol) {
			pCol->PutStoredWidth(nWidth);
		}

		i++;
		nComma = strCols.Find(",");
	}
}

void CPatientProcedureDlg::SetColumnSizes()
{
	//TES 2/23/04: Plagiarized from AppointmentsDlg
	//DRT - 6/6/03 - This function takes the saved column sizes out of ConfigRT
	//		IF the box is checked.

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_TRACKING_REMEMBER_COL_WIDTHS)) {
		return;
	}

	CString strCols = GetRemotePropertyText("DefaultTrackingColumnSizes", "", 0, GetCurrentUserName(), false);
	if(strCols.IsEmpty()) strCols = m_strOriginalColSizes;

	if(!strCols.IsEmpty()) {
		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		int nComma = strCols.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_procedureList->GetColumn(i);
			if(pCol != NULL && i >= 4)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}
}

void CPatientProcedureDlg::OnMarkDone()
{
	try {
		//We're going to try to complete any "skipped" steps first.  That way, if the step we're marking
		//done will be the active step, it will know about it and behave appropriately.
		
		//Loop through the previous rows until we find a ladder row, a completed row, or a non-skippable row.
		bool bCanSkip = true;
		bool bKeepSearching = true;
		for(long nRow = m_procedureList->CurSel; bKeepSearching && nRow >= 0; nRow--) {
			if(VarLong(m_procedureList->GetValue(nRow, CP_RowType)) == 1) {
				//ladder row.
				bKeepSearching = false;
			}
			else if(VarBool(m_procedureList->GetValue(nRow, CP_Done))) {
				bKeepSearching = false;
			}
			else if(!VarBool(m_procedureList->GetValue(nRow, CP_Skippable))) {
				bCanSkip = false;
				bKeepSearching = false;
			}
		}
		ASSERT(!bKeepSearching); //If nothing else, the top row should be a ladder-type row, which should break us out of the loop.
		if(bCanSkip) {
			//First, take care of the last --.
			nRow++;
			//This puts us on the row that was a ladder row or done, so
			nRow++;
			//Now we're on the first skippable step, start skipping.
			for(int nSkipRow = nRow; nSkipRow < m_procedureList->CurSel; nSkipRow++) {
				long nSkippedStepID = VarLong(m_procedureList->GetValue(nSkipRow, CP_StepID));
				CompleteStep(ET_Skipped, GetActivePatientID(), COleDateTime::GetCurrentTime(), -1, nSkippedStepID, COleDateTime::GetCurrentTime(), VarLong(m_procedureList->GetValue(nSkipRow, CP_LadderID)), -1);
			}
		}

		//Now that the skipping is taken care of, mark this event done, with generic event.
		PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_MarkedDone, GetActivePatientID(), COleDateTime::GetCurrentTime(), -1, true, VarLong(m_procedureList->GetValue(m_procedureList->CurSel, CP_StepID)));
		
		//PostMessage(NXM_REFRESH_PATPROC);
	}NxCatchAll("Error in CPatientProcedureDlg::OnMarkDone()");
}


void CPatientProcedureDlg::OnMergeLadder()  {


	//first get the ID of the ladder they want to merge
	long nRow = m_procedureList->CurSel;

	if (nRow == -1) {
		return;
	}

	long nLadderID = VarLong(m_procedureList->GetValue(nRow, CP_LadderID));

	//now that we have the ladder ID, figure out how many other ladders this patient has
	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
	_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(*) as CountLadders FROM LaddersT WHERE PersonID = {INT} AND ID <> {INT}", GetActivePatientID(), nLadderID);
	if (! rsCount->eof) {

		long nCount = AdoFldLong(rsCount, "CountLadders", 0);

		if (nCount <= 0) {

			//there is nothing to merge into
			MsgBox("There are no other ladders for this patient to merge this ladder into");
			return;
		}
	}

	//now figure out which laddertemplateID  we are using
	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
	_RecordsetPtr rsLadderTemplateID = CreateParamRecordset("SELECT StepTemplatesT.LadderTemplateID "
		" FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
		" WHERE StepsT.LadderID = {INT} "
		" GROUP BY StepTemplatesT.LadderTemplateID ", nLadderID);
	
	if (rsLadderTemplateID->eof)  {

		//there is nothing we can do here, there are no steps?
		ASSERT(FALSE);
		ThrowNxException("Cannot find LadderTemplateID for this ladder");
		return;
	}

	long nLadderTemplateID = AdoFldLong(rsLadderTemplateID, "LadderTemplateID", -1);
	ASSERT(nLadderTemplateID != -1);

	//now we have to make sure that this patient has other ladders that have the same laddertemplateID,
	//because otherwise, we can't merge the ladders together
	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
	_RecordsetPtr rsTemplateCount =  CreateParamRecordset("SELECT Count(*) As CountTemplates"
		" FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
		" WHERE StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE PersonID = {INT} AND ID <> {INT}) "
		" AND StepTemplatesT.LadderTemplateID = {INT} ", GetActivePatientID(), nLadderID, nLadderTemplateID);

	if (rsTemplateCount->eof) {
		//this shouldn't happen ever
		ASSERT(FALSE);
		return;
	}

	long nLadderTemplateCount = AdoFldLong(rsTemplateCount, "CountTemplates", 0);

	if (nLadderTemplateCount == 0) {

		//there are no ladders to merge
		MsgBox("There are no available Ladders to merge this Ladder into.  A Ladder can only be merged into another Ladder if "
			" both ladders have the same Ladder Template selected in the Procedure Tab of the Administrator Module");
		return;
	}

	// (j.jones 2006-09-18 14:26) - PLID 22567 - ensure the ladder they are merging from does not have a locked EMN
	long nProcInfoIDToBeMerged = -1;
	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
	_RecordsetPtr rsProcInfo = CreateParamRecordset("SELECT ProcInfoID FROM LaddersT WHERE ID = {INT}", nLadderID);
	if (! rsProcInfo->eof) {
		nProcInfoIDToBeMerged = AdoFldLong(rsProcInfo, "ProcInfoID");
	}
	else {
		MsgBox("Could not find the correct ProcInfoID to be merged, cannot continue");
		return;
	}
	rsProcInfo->Close();

	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
	_RecordsetPtr rsGroupID = CreateParamRecordset("SELECT EMRGroupID FROM PicT WHERE ProcInfoID = {INT} AND EMRGroupID IS NOT NULL", nProcInfoIDToBeMerged);
	if(!rsGroupID->eof) {
		long nGroupIDToBeMerged = AdoFldLong(rsGroupID, "EMRGroupID");

		// (j.jones 2006-09-18 11:43) - PLID 22532 - you are NOT allowed to merge FROM a ladder that has a locked EMN

		//check that this isn't a locked EMR
		// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
		_RecordsetPtr rsLocked = CreateParamRecordset("SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND Status = 2 AND EmrGroupID = {INT}", nGroupIDToBeMerged);
		if (! rsLocked->eof) {
			MsgBox("This ladder is connected to an EMR with a locked EMN, and cannot be merged into another ladder.\n"
				"You may, however, merge another ladder without any locked EMNs into this ladder.");
			return;
		}
		rsLocked->Close();

		// (z.manning 2009-12-15 12:07) - PLID 35810 - Also need to check for non-finalized patient-created EMNs
		//check that this isn't a locked EMR
		_RecordsetPtr rsPatCreated = CreateParamRecordset("SELECT TOP 1 ID FROM EmrMasterT WHERE Deleted = 0 AND PatientCreatedStatus = {INT} AND EmrGroupID = {INT}", CEMN::pcsCreatedByPatientNotFinalized, nGroupIDToBeMerged);
		if (!rsPatCreated->eof) {
			MsgBox("This ladder is connected to an EMR with a patient-created EMN that is not yet finalized, and cannot be merged into another ladder.");
			return;
		}
		rsPatCreated->Close();
	}
	rsGroupID->Close();

	//alrighty, if we got here, we are good to go with the MergeLadderDlg, so let's see it
	CMergeLadderDlg dlg(nLadderID, nLadderTemplateID, GetActivePatientID(), this);

	long nResult = dlg.DoModal();

	if (nResult) {
		Refresh();
	}

}

void CPatientProcedureDlg::RemoveLadder(long nLadderID)
{
	bool bFound = false;
	for(long nRow = 0; nRow < m_procedureList->GetRowCount(); nRow++) {
		if(bFound) {
			//We're removing.  Should we still be removing?
			if(VarLong(m_procedureList->GetValue(nRow, CP_LadderID),-1) != nLadderID) bFound = false;
			//In either case, we want to remove this row, since if it's not part of the ladder it will be the blank row
			//after the ladder.
			m_procedureList->RemoveRow(nRow);
			nRow--;			
		}
		else {
			//Is this the first row of ourladder?
			if(VarLong(m_procedureList->GetValue(nRow, CP_LadderID),-1) == nLadderID) {
				//Yep!
				bFound = true;
				m_procedureList->RemoveRow(nRow);
				nRow--;
			}
		}
	}
}

void CPatientProcedureDlg::RefreshLadder(long nLadderID)
{
	//Find this row, remove it, and re-add it if necessary.
	long nRow = m_procedureList->FindByColumn(CP_LadderID, nLadderID, 0, VARIANT_FALSE);
	if(nRow == -1) {
		//It wasn't there, we'll add it at the end.
		nRow = m_procedureList->GetRowCount();
	}
	m_procedureList->SetRedraw(FALSE);
	RemoveLadder(nLadderID);

	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query, and combined the recordset from
	// InsertTopLevel into this recordset, so we can pass in all our info into InsertTopLevel
	_RecordsetPtr rsLadderInfo = CreateParamRecordset("SELECT ProcInfoT.ID AS ProcInfoID, PicT.ID AS PicID, "
		"TopLevelInfoQ.Status, TopLevelInfoQ.Notes, TopLevelInfoQ.LadderName, TopLevelInfoQ.FirstInterestDate, "
		"TopLevelInfoQ.Name, TopLevelInfoQ.UserName, TopLevelInfoQ.EmrDate, TopLevelInfoQ.HasProcedure, "
		"TopLevelInfoQ.NextStep, TopLevelInfoQ.DoneDate, "
		"CASE WHEN ((LaddersT.ID Is Not Null AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1)) "
		"OR (LaddersT.ID Is Null AND (EmrGroupsT.Status Is Null OR EmrGroupsT.Status = 0))) THEN 1 ELSE 0 END AS IsActive "
		"FROM LaddersT LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID LEFT JOIN PicT ON ProcInfoT.ID = "
		"PicT.ProcInfoID LEFT JOIN (SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID "

		"LEFT JOIN ("
			"SELECT ProcInfoT.ID AS ProcInfoID, dbo.CalcProcInfoName(ProcInfoT.ID) AS LadderName, LaddersT.ID, "
			"LaddersT.PersonID, LadderStatusT.Name AS Status, LaddersT.Notes, FirstInterestDate, LaddersT.Name, "
			"UsersT.UserName, EmrGroupsT.InputDate AS EmrDate, "
			"CASE WHEN EXISTS (SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID) THEN 1 ELSE 0 END AS HasProcedure, "
			"LadderStatusT.IsActive, (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) AS NextStep, "
			"(SELECT Max(CompletedDate) AS DoneDate FROM StepsT WHERE LadderID = LaddersT.ID) AS DoneDate "
			"FROM ProcInfoT LEFT JOIN (LaddersT INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID) ON ProcInfoT.ID = LaddersT.ProcInfoID "
			"LEFT JOIN ((SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID) ON ProcInfoT.ID = PicT.ProcInfoID "
			"LEFT JOIN UsersT ON LaddersT.UserID = UsersT.PersonID "
			") AS TopLevelInfoQ ON ProcInfoT.ID = TopLevelInfoQ.ProcInfoID "	

		"WHERE LaddersT.ID = {INT}", nLadderID);
	long nProcInfoID = AdoFldLong(rsLadderInfo, "ProcInfoID", -1);
	long nPicID = AdoFldLong(rsLadderInfo, "PicID", -1);
	BOOL bIsActive = AdoFldLong(rsLadderInfo, "IsActive") == 1 ? TRUE : FALSE;

	if( (IsDlgButtonChecked(IDC_ACTIVE_LADDERS) && !bIsActive) ||
		(IsDlgButtonChecked(IDC_INACTIVE_LADDERS) && bIsActive) ) {
		//This ladder shouldn't be showing, and we've already removed it.  Peace out.
		m_procedureList->SetRedraw(TRUE);
		return;
	}

	// (j.jones 2008-09-03 17:14) - PLID 10417 - now the above recordset has all the data we need for
	// InsertTopLevel, so let's just pass it all in

	COleDateTime dtDate;

	if(rsLadderInfo->Fields->GetItem("FirstInterestDate")->Value.vt == VT_DATE) {
		dtDate = VarDateTime(rsLadderInfo->Fields->GetItem("FirstInterestDate")->Value);
	}
	else {
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		dtDate = VarDateTime(rsLadderInfo->Fields->GetItem("EmrDate")->Value, dtInvalid);
	}

	// (j.jones 2008-12-01 08:57) - PLID 32262 - changed nUserID to strUserNames
	InsertTopLevel(nRow, nLadderID, nPicID, nProcInfoID,
		AdoFldString(rsLadderInfo, "LadderName", ""), (BOOL)AdoFldLong(rsLadderInfo, "HasProcedure"), AdoFldLong(rsLadderInfo, "NextStep", -1),
		AdoFldString(rsLadderInfo, "Status",""), bIsActive, dtDate,
		AdoFldString(rsLadderInfo, "Notes",""), rsLadderInfo->Fields->GetItem("DoneDate")->Value, AdoFldString(rsLadderInfo, "UserName", ""));

	m_procedureList->SetRedraw(TRUE);
}

void CPatientProcedureDlg::SetLadderDescription(IRowSettingsPtr pRow, long nLadderID, long nNextStepID, CString strStatus, BOOL bIsStatusActive)
{
	//OK, but this isn't so simple any more.  If this is not an "inactive" status, and the currently pending step's active date is greater than today, then we want to say that we're on hold.
	//Is this an "inactive" status.
	if(nLadderID == -1) {
		pRow->PutValue(CP_Description, _bstr_t(""));
	}
	else {
		if(!bIsStatusActive) {
			pRow->PutValue(CP_Description, _bstr_t(strStatus));
		}
		else {
			if(nNextStepID != -1) {
				// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query
				_RecordsetPtr rsNextStep = CreateParamRecordset("SELECT StepsT.ActiveDate, StepTemplatesT.StepName "
					"FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
					"WHERE StepsT.ID = {INT}", nNextStepID);
				COleDateTime dtActive = AdoFldDateTime(rsNextStep, "ActiveDate", COleDateTime::GetCurrentTime());
				if(dtActive > COleDateTime::GetCurrentTime()) {
					//We're on hold.
					CString strStatus;
					if(dtActive.GetYear() == 9999) {
						strStatus = "On Hold indefinitely";
					}
					else {
						strStatus.Format("On Hold until %s", FormatDateTimeForInterface(dtActive));
					}
					pRow->PutValue(CP_Description, _bstr_t(strStatus));
				}
				else {
					if(strStatus == "Active") {
						//Get the name of the active step.
						CString strStatus = "Active - " + AdoFldString(rsNextStep, "StepName");
						pRow->PutValue(CP_Description, _bstr_t(strStatus));
					}
					else {
						pRow->PutValue(CP_Description, _bstr_t(strStatus));
				}
				}
			}
			else {							  
				pRow->PutValue(CP_Description, _bstr_t(strStatus));
			}
		}
	}
}

void CPatientProcedureDlg::RemoveProcInfo(long nProcInfoID)
{
	//m.hancock - 5/3/2006 - PLID 20308 - This should behave the same as RemoveLadder, but should handle ProcInfo records.

	bool bFound = false;
	for(long nRow = 0; nRow < m_procedureList->GetRowCount(); nRow++) {
		if(bFound) {
			//We're removing.  Should we still be removing?
			if(m_procedureList->GetValue(nRow, CP_ProcInfoID).vt != VT_EMPTY) {
				if(VarLong(m_procedureList->GetValue(nRow, CP_ProcInfoID), -1) != nProcInfoID) bFound = false;
				//In either case, we want to remove this row, since if it's not part of the ladder it will be the blank row
				//after the ladder.
				else {
					m_procedureList->RemoveRow(nRow);
					nRow--;
				}
			}
			else {
				//This is the blank row after the ladder we chose to delete.
				m_procedureList->RemoveRow(nRow);
				nRow--;
			}
		}
		else {
			//Is this the first row of our ladder?
			if(m_procedureList->GetValue(nRow, CP_ProcInfoID).vt != VT_EMPTY) {
				if(VarLong(m_procedureList->GetValue(nRow, CP_ProcInfoID), -1) == nProcInfoID) {
					//Yep!
					bFound = true;
					m_procedureList->RemoveRow(nRow);
					nRow--;
				}
			}
		}
	}
}


void CPatientProcedureDlg::RefreshProcInfo(long nProcInfoID)
{
	//m.hancock - 5/3/2006 - PLID 20308 - This should behave the same as RefreshLadder, but should handle ProcInfo records.

	//Find this row, remove it, and re-add it if necessary.
	long nRow = m_procedureList->FindByColumn(CP_ProcInfoID, nProcInfoID, 0, VARIANT_FALSE);
	if(nRow == -1) {
		//It wasn't there, we'll add it at the end.
		nRow = m_procedureList->GetRowCount();
	}
	m_procedureList->SetRedraw(FALSE);
	RemoveProcInfo(nProcInfoID);

	//m.hancock - 5/3/2006 - PLID 20308 - Changed the query to retrieve the ladder ID instead of the ProcInfoID
	// (j.jones 2008-09-03 11:33) - PLID 10417 - parameterized this query, and combined the recordset from
	// InsertTopLevel into this recordset, so we can pass in all our info into InsertTopLevel
	_RecordsetPtr rsLadderInfo = CreateParamRecordset("SELECT LaddersT.ID AS LadderID, PicT.ID AS PicID, "
		"TopLevelInfoQ.Status, TopLevelInfoQ.Notes, TopLevelInfoQ.LadderName, TopLevelInfoQ.FirstInterestDate, "
		"TopLevelInfoQ.Name, TopLevelInfoQ.UserName, TopLevelInfoQ.EmrDate, TopLevelInfoQ.HasProcedure, "
		"TopLevelInfoQ.NextStep, TopLevelInfoQ.DoneDate, "
		"CASE WHEN ((LaddersT.ID Is Not Null AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1)) "
		"OR (LaddersT.ID Is Null AND (EmrGroupsT.Status Is Null OR EmrGroupsT.Status = 0))) THEN 1 ELSE 0 END AS IsActive "
		"FROM LaddersT LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID LEFT JOIN PicT ON ProcInfoT.ID = "
		"PicT.ProcInfoID LEFT JOIN (SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID "

		"LEFT JOIN ("
			"SELECT ProcInfoT.ID AS ProcInfoID, dbo.CalcProcInfoName(ProcInfoT.ID) AS LadderName, LaddersT.ID, "
			"LaddersT.PersonID, LadderStatusT.Name AS Status, LaddersT.Notes, FirstInterestDate, LaddersT.Name, "
			"UsersT.UserName, EmrGroupsT.InputDate AS EmrDate, "
			"CASE WHEN EXISTS (SELECT ID FROM ProcInfoDetailsT WHERE ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID) THEN 1 ELSE 0 END AS HasProcedure, "
			"LadderStatusT.IsActive, (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) AS NextStep, "
			"(SELECT Max(CompletedDate) AS DoneDate FROM StepsT WHERE LadderID = LaddersT.ID) AS DoneDate "
			"FROM ProcInfoT LEFT JOIN (LaddersT INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID) ON ProcInfoT.ID = LaddersT.ProcInfoID "
			"LEFT JOIN ((SELECT * FROM EmrGroupsT WHERE Deleted = 0) AS EmrGroupsT INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID) ON ProcInfoT.ID = PicT.ProcInfoID "
			"LEFT JOIN UsersT ON LaddersT.UserID = UsersT.PersonID "
			") AS TopLevelInfoQ ON ProcInfoT.ID = TopLevelInfoQ.ProcInfoID "

		"WHERE ProcInfoT.ID = {INT}", nProcInfoID);
	long nLadderID = AdoFldLong(rsLadderInfo, "LadderID", -1);
	long nPicID = AdoFldLong(rsLadderInfo, "PicID", -1);
	BOOL bIsActive = AdoFldLong(rsLadderInfo, "IsActive") == 1 ? TRUE : FALSE;

	if( (IsDlgButtonChecked(IDC_ACTIVE_LADDERS) && !bIsActive) ||
		(IsDlgButtonChecked(IDC_INACTIVE_LADDERS) && bIsActive) ) {
		//This ladder shouldn't be showing, and we've already removed it.  Peace out.
		m_procedureList->SetRedraw(TRUE);
		return;
	}
	
	// (j.jones 2008-09-03 17:14) - PLID 10417 - now the above recordset has all the data we need for
	// InsertTopLevel, so let's just pass it all in

	COleDateTime dtDate;

	if(rsLadderInfo->Fields->GetItem("FirstInterestDate")->Value.vt == VT_DATE) {
		dtDate = VarDateTime(rsLadderInfo->Fields->GetItem("FirstInterestDate")->Value);
	}
	else {
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		dtDate = VarDateTime(rsLadderInfo->Fields->GetItem("EmrDate")->Value, dtInvalid);
	}

	// (j.jones 2008-12-01 08:57) - PLID 32262 - changed nUserID to strUserNames
	InsertTopLevel(nRow, nLadderID, nPicID, nProcInfoID,
		AdoFldString(rsLadderInfo, "LadderName", ""), (BOOL)AdoFldLong(rsLadderInfo, "HasProcedure"), AdoFldLong(rsLadderInfo, "NextStep", -1),
		AdoFldString(rsLadderInfo, "Status",""), bIsActive, dtDate,
		AdoFldString(rsLadderInfo, "Notes",""), rsLadderInfo->Fields->GetItem("DoneDate")->Value, AdoFldString(rsLadderInfo, "UserName", ""));
	
	m_procedureList->SetRedraw(TRUE);
}

// (b.eyers 2015-06-25) - PLID 39619 - remember per user if this hide checkbox is checked or not
void CPatientProcedureDlg::OnHideEMROnlyPICs()
{
	long nRemember = 0;
	if (IsDlgButtonChecked(IDC_HIDE_EMR_ONLY_PICS))
		nRemember = 1;
	else
		nRemember = 0;
	SetRemotePropertyInt("HideEMROnlyPICs", nRemember, 0, GetCurrentUserName());

	Refresh();
	
}
