// MarketRetentionGraphDlg.cpp : implementation file
//
// (c.haag 2008-04-18 14:05) - PLID 29715 - Replaced the MSChart object with 
// the NexTech pie chart window 

#include "stdafx.h"
#include "marketingRc.h"
#include "MarketRetentionGraphDlg.h"
#include "Marketutils.H"
#include "datetimeutils.h"
#include "InternationalUtils.h"
#include "MultiSelectDlg.h"
#include "MarketFilterPickerDlg.h"
#include "MarketUtils.h"
#include "Docbar.h"
#include "MarketRangeConfigDlg.h"

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMarketRetentionGraphDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

// (d.moore 2007-09-05) - PLID 14670 - I added some enums to make things a little 
//  clearer when working with NxDataLists.
enum EProcedureCols {
	epcID,
	epcName
};

enum ELadderCols {
	elcID,
	elcName
};

enum EStepCols {
	escID, 
	escName
};

// (b.spivey, October 31, 2011) - PLID 38861 - Seemed appropriate to go ahead and make some enums for this.
enum EMarketingMergeCols {
	emmcID = 0, 
	emmcName
};

enum EMarketingMergeOptions {
	emmoRetained = 0, 
	emmoUnretained, 
	emmoBoth
};


//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketRetentionGraphDlg::CMarketRetentionGraphDlg(CWnd* pParent)
	: CMarketingDlg(IDD, pParent)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/retention.htm";
	
}


void CMarketRetentionGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketRetentionGraphDlg)
	DDX_Control(pDX, IDC_EXCLUDE_APPTS, m_btnExcludeAppts);
	DDX_Control(pDX, IDC_USE_SCHEDULE, m_btnUseScheduler);
	DDX_Control(pDX, IDC_USE_TRACKING, m_btnUseTracking);
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_CREATE_MERGE_GROUP, m_btnCreateMergeGroup);
	DDX_Control(pDX, IDC_MULTI_PROC_LIST, m_nxlMultiProcLabel);
	DDX_Control(pDX, IDC_PIVOT_DATE, m_dtPivot);
	DDX_Control(pDX, IDC_RETENTION_BASED_ON, m_nxstaticRetentionBasedOn);
	DDX_Control(pDX, IDC_RETENTION_LABEL1, m_nxstaticRetentionLabel1);
	DDX_Control(pDX, IDC_RETENTION_LABEL5, m_nxstaticRetentionLabel5);
	DDX_Control(pDX, IDC_RETENTION_LABEL2, m_nxstaticRetentionLabel2);
	DDX_Control(pDX, IDC_RETENTION_LABEL3, m_nxstaticRetentionLabel3);
	DDX_Control(pDX, IDC_RETENTION_LABEL4, m_nxstaticRetentionLabel4);
	DDX_Control(pDX, IDC_STATIC_PIE_GRAPH_REGION, m_btnPieGraphRegion);
	DDX_Control(pDX, IDC_ENABLE_EXCLUDE_UNRETAINED, m_checkExcludeUnretained);
	DDX_Control(pDX, IDC_EXCLUDE_UNRETAINED_RANGE, m_nxeditExcludeUnretainedRange); 
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CMarketRetentionGraphDlg, IDC_PIVOT_DATE, 2 /* Change */, OnChangePivotDate, VTS_NONE)

BEGIN_MESSAGE_MAP(CMarketRetentionGraphDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketRetentionGraphDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PIVOT_DATE, OnChangePivotDate)
	ON_BN_CLICKED(IDC_CREATE_MERGE_GROUP, OnCreateMergeGroup)
	ON_BN_CLICKED(IDC_EXCLUDE_APPTS, OnExcludeAppts)
	ON_BN_CLICKED(IDC_USE_SCHEDULE, OnUseSchedule)
	ON_BN_CLICKED(IDC_USE_TRACKING, OnUseTracking)
	ON_BN_CLICKED(IDC_RETENTION_RANGE_SETUP, OnRetentionRangeSetup)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ENABLE_EXCLUDE_UNRETAINED, &CMarketRetentionGraphDlg::OnBnClickedEnableExcludeUnretainedRange)
	ON_EN_KILLFOCUS(IDC_EXCLUDE_UNRETAINED_RANGE, &CMarketRetentionGraphDlg::OnEnKillfocusExcludeUnretainedRange) 
	ON_EN_CHANGE(IDC_EXCLUDE_UNRETAINED_RANGE, &CMarketRetentionGraphDlg::OnEnChangeExcludeUnretainedRange)
	ON_NOTIFY(NM_KILLFOCUS, IDC_PIVOT_DATE, &CMarketRetentionGraphDlg::OnNMKillfocusPivotDate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketRetentionGraphDlg message handlers


void CMarketRetentionGraphDlg::InitalizeLists() {
		
	//requery the lists
	m_pRangeList->Requery();
	// (j.jones 2010-01-26 10:20) - PLID 34354 - removed merge range list
	//m_pMergeRangeList->Requery();
}

BOOL CMarketRetentionGraphDlg::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();

	try {
		// (c.haag 2008-04-29 15:01) - PLID 29833 - NxIconify the Create Merge Group button
		m_btnCreateMergeGroup.AutoSet(NXB_NEW);

		//default to using the scheduler
		CheckDlgButton(IDC_USE_SCHEDULE, 1);
		CheckDlgButton(IDC_USE_TRACKING, 0);
	
		//initialize the datalists
		m_pProcList = BindNxDataListCtrl(IDC_RETENTION_PROCEDURE_LIST, false);
		// (d.moore 2007-09-04) - PLID 14670 - A procedure can have multiple ladders, so a new
		//  dropdown needed to be added to allow selection of the correct ladder.
		m_pLadderList = BindNxDataList2Ctrl(IDC_RETENTION_LADDER_LIST, false);
		m_pStepList = BindNxDataListCtrl(IDC_RETENTION_STEP_LIST, false);
		m_pRangeList = BindNxDataListCtrl(IDC_RANGE_LIST, false);
		// (j.jones 2010-01-26 10:20) - PLID 34354 - removed merge range list
		//m_pMergeRangeList = BindNxDataListCtrl(IDC_MERGE_RANGE_LIST, false);
		m_pPurposeList = BindNxDataListCtrl(IDC_RETENTION_PURPOSE_LIST, true);

		// (b.spivey, November 03, 2011) - PLID 38861 - Populate the list with pre-set options. 
		m_pMergeOptionList = BindNxDataList2Ctrl(IDC_MERGE_GROUP_OPTIONS, false); 
		{
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pMergeOptionList->GetNewRow(); 			
			pNewRow->PutValue(emmcID, emmoRetained);
			pNewRow->PutValue(emmcName, "Retained Patients only"); 
			m_pMergeOptionList->AddRowAtEnd(pNewRow, NULL); 

			pNewRow = m_pMergeOptionList->GetNewRow(); 
			pNewRow->PutValue(emmcID, emmoUnretained);
			pNewRow->PutValue(emmcName, "Unretained Patients only"); 
			m_pMergeOptionList->AddRowAtEnd(pNewRow, NULL); 

			pNewRow = m_pMergeOptionList->GetNewRow(); 
			pNewRow->PutValue(emmcID, emmoBoth);
			pNewRow->PutValue(emmcName, "Both"); 
			m_pMergeOptionList->AddRowAtEnd(pNewRow, NULL); 
		}
		m_pMergeOptionList->SetSelByColumn(emmcID, emmoRetained); 

		m_nExcludeUnretainedRange = 0; 

		m_bRenderedOnce = false;
		m_bActive = false;
		m_Go.AutoSet(NXB_MARKET);

		InitalizeLists();

		// (c.haag 2008-04-18 14:05) - PLID 29715 - Initialize the pie chart window
		CRect rcPieGraph;
		GetDlgItem(IDC_STATIC_PIE_GRAPH_REGION)->GetWindowRect(&rcPieGraph);
		ScreenToClient(&rcPieGraph);
		CBrush brWhite(RGB(255,255,255));
		if (!m_wndPieGraph.CreateEx(0,
			AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW), brWhite),
			"", 
			WS_CHILD, 
			rcPieGraph.left, rcPieGraph.top, rcPieGraph.Width(), rcPieGraph.Height(), // use our calculated values
			GetSafeHwnd(),
			0))
		{
			ThrowNxException("Failed to create bar chart window!");
		}
		// Make sure the pie graph is in front of the NxColor control
		m_wndPieGraph.SetWindowPos(&wndTop, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);

		// (c.haag 2008-04-18 14:06) - PLID 29715 - Make it so NxDialog auto-repositions the
		// pie chart window
		GetControlPositions();


		//set the date to be today to start with
		m_dtPivot.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		m_brush.CreateSolidBrush(PaletteColor(0x00C8FFFF));
		
		m_pRangeList->AllowSort = FALSE;

		// (j.jones 2010-01-26 10:20) - PLID 34354 - removed merge range list
		//m_pMergeRangeList->AllowSort = FALSE;

		m_nxlMultiProcLabel.SetColor(0x00C8FFFF);
		m_nxlMultiProcLabel.SetText("");
		m_nxlMultiProcLabel.SetType(dtsHyperlink);
		RefreshTab(false);


		//PLID 21773 - initialize the date combo to all dates
		GetMainFrame()->m_pDocToolBar->SetDateOptionCombo(mdotAll);

		// (a.walling 2006-10-17 09:27) - PLID 22764 - Does not properly update visible/hidden filters initially
		RefreshTab(false);

	}NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMarketRetentionGraphDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketRetentionGraphDlg)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PROCEDURE_LIST, 16 /* SelChosen */, OnSelChosenProcedureList, VTS_I4)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RANGE_LIST, 10 /* EditingFinished */, OnEditingFinishedRangeList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_STEP_LIST, 16 /* SelChosen */, OnSelChosenRetentionStepList, VTS_I4)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PROCEDURE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRetentionProcedureList, VTS_I2)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PURPOSE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRetentionPurposeList, VTS_I2)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PURPOSE_LIST, 2 /* SelChanged */, OnSelChangedRetentionPurposeList, VTS_I4)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PURPOSE_LIST, 16 /* SelChosen */, OnSelChosenRetentionPurposeList, VTS_I4)
	//ON_EVENT(CMarketRetentionGraphDlg, IDC_MERGE_RANGE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedMergeRangeList, VTS_I2)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RANGE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRangeList, VTS_I2)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_LADDER_LIST, 16 /* SelChosen */, OnSelChosenRetentionLadderList, VTS_DISPATCH)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_LADDER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRetentionLadderList, VTS_I2)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_LADDER_LIST, 1 /* SelChanging */, OnSelChangingRetentionLadderList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PURPOSE_LIST, 1 /* SelChanging */, OnSelChangingRetentionPurposeList, VTS_PI4)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_PROCEDURE_LIST, 1 /* SelChanging */, OnSelChangingRetentionProcedureList, VTS_PI4)
	ON_EVENT(CMarketRetentionGraphDlg, IDC_RETENTION_STEP_LIST, 1 /* SelChanging */, OnSelChangingRetentionStepList, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CMarketRetentionGraphDlg, IDC_MERGE_GROUP_OPTIONS, 1, CMarketRetentionGraphDlg::SelChangingMergeGroupOptions, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CMarketRetentionGraphDlg::RefreshTab(bool bRefreshGraph /*= true*/) {
	
	CWaitCursor cursor;
	
	SetType(RETENTIONGraph);
	if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);

		//set the procedure datalist
		m_pProcList->FromClause = "ProcedureT";
		m_pProcList->WhereClause = "Recur <> 0";
		m_pProcList->Requery();

		// (d.moore 2007-09-04) - PLID 14670 - Grey out the ladder list since it doesn't apply here.
		GetDlgItem(IDC_RETENTION_LADDER_LIST)->EnableWindow(FALSE);
		m_pLadderList->PutCurSel(NULL);

		//grey out the step datalist since we don't need it
		GetDlgItem(IDC_RETENTION_STEP_LIST)->EnableWindow(FALSE);
		m_pStepList->CurSel = -1;

		GetDlgItem(IDC_RETENTION_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MULTI_PROC_LIST)->ShowWindow(SW_HIDE);

	}
	else  {
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfPatientLocation, mftLocation);
		SetFilter(mfPatientProvider, mftProvider);

		// (d.moore 2007-09-04) - PLID 14670 - Procedures can be associated with multiple ladders now, so
		//  so the WHERE query had to be modified slightly.
		//set the procedure datalist
		m_pProcList->FromClause = "ProcedureT";
		m_pProcList->WhereClause = 
			"ID IN "
			"(SELECT ProcedureLadderTemplateT.ProcedureID "
				"FROM ProcedureLadderTemplateT "
					"INNER JOIN LadderTemplatesT "
					"ON ProcedureLadderTemplateT.LadderTemplateID = LadderTemplatesT.ID "
					"INNER JOIN StepTemplatesT "
					"ON LadderTemplatesT.ID = StepTemplatesT.LadderTemplateID "
				"WHERE StepTemplatesT.Action = 20 "
					"AND StepTemplatesT.Inactive = 0)";
		m_pProcList->Requery();

		// (d.moore 2007-09-04) - PLID 14670 - Enable the lists for ladder and step selection.
		//  When the procedure list has finished loading the ladder list and step lists will
		//  be requeried from OnRequeryFinishedRetentionProcedureList.
		GetDlgItem(IDC_RETENTION_LADDER_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_RETENTION_STEP_LIST)->EnableWindow(TRUE);
		
		GetDlgItem(IDC_RETENTION_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MULTI_PROC_LIST)->ShowWindow(SW_HIDE);
	}

	m_strMultiPurposeIds = "";
	m_strMultiProcedureIDs = "";
	if (IsDlgButtonChecked(IDC_EXCLUDE_APPTS)) {
		GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(FALSE);
	}


	//no sense loading the graph if we aren't ready with the procedures
	m_pProcList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	// (a.walling 2006-10-17 09:30) - PLID 22764 - This is normally called in Refresh
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	if (bRefreshGraph)
		Refresh();
}

void CMarketRetentionGraphDlg::OnSelChosenProcedureList(long nRow) 
{
	try {

		// (j.gruber 2008-06-23 17:46) - PLID 27440 - fixed the case for if they don't have any applicable procedures
		if (nRow == -1) {

			//this could legitimately happen if they have no procedures that fit the criteria
			if (m_pProcList->GetRowCount() == 0 ) {
				//do nothing
				return;
			}
		}
				

		
		if (!IsDlgButtonChecked(IDC_USE_SCHEDULE)) {

			// (d.moore 2007-09-04) - PLID 14670 - Procedures may now have more than one ladder
			//  associated with them.
			long nProcID = VarLong(m_pProcList->GetValue(nRow, epcID));
			
			CString strWhere;
			strWhere.Format(
				"ProcedureLadderTemplateT.ProcedureID = %li "
				"AND LadderTemplatesT.ID IN "
					"(SELECT LadderTemplateID "
						"FROM StepTemplatesT "
						"WHERE Action=20 AND Inactive = 0)", nProcID);
			m_pLadderList->PutWhereClause((_bstr_t)strWhere);
			m_pLadderList->Requery();			
		}
		else {

			//PLID 19760 - if they are in the scheduler, check to see if they picked multi-procedures
			if (VarLong(m_pProcList->GetValue(nRow, epcID)) == -2) {
				if (OnMultiProc()) {
					// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
				}
				else {
					//we don't need to refresh
					return;
				}
			}
			else {
				//set the currently selected item anyway
				GetDlgItem(IDC_RETENTION_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_MULTI_PROC_LIST)->ShowWindow(SW_HIDE);
				m_strMultiProcedureIDs = AsString(VarLong(m_pProcList->GetValue(nRow, epcID)));
			}				

		}


		//refesh the chart
		// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
		ResetGraph(true, "", true);	//(a.wilson 2011-10-7) PLID 38789

	}NxCatchAll("Error in OnSelChosenProcedureList");	
}

void CMarketRetentionGraphDlg::Refresh() {
	try {
		// (c.haag 2008-04-18 14:06) - PLID 29715 - Replaced the MSChart control with the
		// NexTech pie graph window

		//set the timeout
		CIncreaseCommandTimeout ict(600);		

		if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
			SetFilter(mfFirstContactDate, mftDate);
			SetFilter(mfApptLocation, mftLocation);
			SetFilter(mfApptProvider, mftProvider);
		}
		else {
			SetFilter(mfFirstContactDate, mftDate);
			SetFilter(mfPatientLocation, mftLocation);
			SetFilter(mfPatientProvider, mftProvider);
		}

		CWaitCursor cWait;

		//get the top parameters (main Physician, date, and location
		CString strDateFrom, strDateTo, strMainPhysician, provIDs, strLocation, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		int nCategory, nResp;

		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;
		
		GetParameters(strDateFrom, strDateTo, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
		
		//check out the values
		if (UseFilter(mftProvider)) {
			if ((!IsDlgButtonChecked(IDC_USE_SCHEDULE)) && (strProvField.Find("Resource") != -1)) {
				MessageBox("The Retention Tab does not support filtering on resource when based on Tracking, reverting to Patient Provider");
				SetFilter(mfPatientProvider, mftProvider);
				strProvField = "(PatientsT.MainPhysician IN";
				
				//we requeried the list , so its now set back to -1
				strMainPhysician = "";
			}
			else {
				strMainPhysician.Format(" AND %s %s) ", strProvField, provIDs);
			}
			
			
		}
			
		if (UseFilter(mftLocation)) {
			if ((!IsDlgButtonChecked(IDC_USE_SCHEDULE)) && (strLocationField.Find("Appointment") != -1)) {
				MessageBox("The Retention Tab does not support filtering on resource when based on Tracking, reverting to Patient Location");
				SetFilter(mfPatientLocation, mftLocation);
				strLocationField = "PersonT.Location";
				
			}
			strLocation.Format(" AND %s IN %s ", strLocationField, locIDs);
			
			
		}
		
		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && UseFilter(mftDate)) {
			if ((!IsDlgButtonChecked(IDC_USE_SCHEDULE)) && (strDateField.Find("Appointment") != -1)) {
				MessageBox("The Retention Tab does not support filtering on resource when based on Tracking, reverting to First Contact Date");
				SetFilter(mfFirstContactDate, mftDate);
				strDateField = "PersonT.FirstContactDate";
			
			}
			strFrom.Format(" AND %s >= '%s' ", strDateField, strDateFrom);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, strDateTo);
			
			
		}
	
		//now for the filters on just this tab
		long nProcedureID = -1, nLadderID = -1, nStepTempForCompletionID = -1;
		COleDateTime dtPivotDate;
		CString strProcedure;

		if(m_pProcList->CurSel != -1) {
			nProcedureID = VarLong(m_pProcList->GetValue(m_pProcList->CurSel, epcID));
			if (nProcedureID == -2) {
				//we are using multiple procedures
				strProcedure = m_strMultiProcedureIDs;
			}
			else {
				strProcedure = AsString(nProcedureID);
			}
		}
		else {
			strProcedure = AsString(nProcedureID);
		}
		
		// (d.moore 2007-10-10) - PLID 14670 - If a ladder template was selected for the filter
		//  then we need to find any matches for it in the LaddersT table.
		CString strLadderTemplateIdQuery;
		if (m_pLadderList->CurSel != NULL) {
			nLadderID = VarLong(m_pLadderList->CurSel->GetValue(elcID));
			if (nLadderID > 0) {
				strLadderTemplateIdQuery.Format(
					"AND LaddersT.ID IN "
					"(SELECT LaddersT.ID "
					"FROM LaddersT "
						"INNER JOIN StepsT "
						"ON LaddersT.ID = StepsT.LadderID "
						"INNER JOIN StepTemplatesT "
						"ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"INNER JOIN LadderTemplatesT "
						"ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
						"INNER JOIN ProcInfoT "
						"ON LaddersT.ProcInfoID = ProcInfoT.ID "
						"INNER JOIN ProcInfoDetailsT "
						"ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
					"WHERE LadderTemplatesT.ID = %li "
						"AND ProcInfoDetailsT.ProcedureID = %li) ", 
					nLadderID, nProcedureID);
			}
		}
		
		if(m_pStepList->CurSel != -1) {
			nStepTempForCompletionID = VarLong(m_pStepList->GetValue(m_pStepList->CurSel, escID));
		}

		// (j.jones 2010-01-26 10:36) - PLID 34354 - the ability to exclude future appointments
		// now applies to the pie graph generation
		COleDateTime dtCurDate = COleDateTime::GetCurrentTime();
		CString strExcludeFutureApptsFilter = "";
		//take out the people that have appts
		if (IsDlgButtonChecked(IDC_EXCLUDE_APPTS)) {
			if (m_strMultiPurposeIds == "-1" || m_strMultiPurposeIds == "") {
				//they are using all purposes
				strExcludeFutureApptsFilter.Format(" AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4) ", FormatDateTimeForSql(dtCurDate, dtoDate));
			}
			else {
				strExcludeFutureApptsFilter.Format(" AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4 "
					" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE "
					" PurposeID IN (%s))) ", FormatDateTimeForSql(dtCurDate, dtoDate), m_strMultiPurposeIds);
			}
		}

		dtPivotDate = VarDateTime(m_dtPivot.GetValue());

		CString strBaseSql, strBaseTemp, strBaseInner;
		if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {

			// (b.spivey, November 04, 2011) - PLID 46267 - Added [ExcludeUnretainedRange] to take place of the unretained filter when we do str.Replace() later.
			//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
			strBaseSql.Format("SELECT Count(ID) AS TotalCount  "
				"    FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID  "
				"   FROM AppointmentsT  "
				"   LEFT JOIN PersonT ON AppointmentsT.PatientID   "
				"   = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
				"   LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				"    WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
				"   AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3)  "  //procedure, minor procedure, other procedure
				"   AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
				" 	  %s %s %s %s %s %s "
				"		[ExcludeUnretainedRange] "
				"   GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT "
				"   ON AppointmentsT.ID = ApptsQ.ApptID ", 				
				FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedure, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strExcludeFutureApptsFilter);

			//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
			strBaseInner.Format(" SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID  "
				"   FROM AppointmentsT  "
				"   LEFT JOIN PersonT ON AppointmentsT.PatientID   "
				"   = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
				"   LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				"    WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
				"   AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3)  "  //procedure, minor procedure, other procedure
				"   AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
				" 	  %s %s %s %s %s %s "
				"		[ExcludeUnretainedRange] "
				"   GROUP BY AppointmentsT.PatientID ",
				FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedure, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strExcludeFutureApptsFilter);


			//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
			strBaseTemp.Format("SELECT Count(ID) AS TotalCount  "
				"    FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID  "
				"   FROM AppointmentsT  "
				"   LEFT JOIN PersonT ON AppointmentsT.PatientID   "
				"   = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
				"   LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
				"    WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
				"   AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3)  "  //procedure, minor procedure, other procedure
				"   AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
				"   AND AppointmentsT.ID IN (%s) "
				" 	  %s %s %s %s %s %s [RangeFilter] "
				"   GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT "
				"   ON AppointmentsT.ID = ApptsQ.ApptID ", 				
				FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedure, strBaseInner, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strExcludeFutureApptsFilter);
		}
		else {
			// (d.moore 2007-10-09) - PLID 14670 - If a ladder template was selected for the filter then
			//  a query will be added to find a match for the template in the LaddersT table.
			strBaseInner.Format("SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
				"	FROM LaddersT "
				" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
				"	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
				" 	WHERE NOT EXISTS      " 
				" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID    "
				" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
				" 	AND ProcInfoID IN  "
				" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     "
				"   %s %s %s %s %s %s %s "
				"		[ExcludeUnretainedRange] "
				" 	 GROUP BY LaddersT.PersonID ",
				nProcedureID, strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strExcludeFutureApptsFilter);

			strBaseSql.Format("SELECT Count(LadderID) AS TotalCount FROM ( "
				" 	SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
				"	FROM LaddersT "
				" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
				"	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
				" 	WHERE NOT EXISTS      " 
				" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID    "
				" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
				" 	AND ProcInfoID IN  "
				" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     "
				"   %s %s %s %s %s %s %s "
				"		[ExcludeUnretainedRange] "
				" 	 GROUP BY LaddersT.PersonID "
				" ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID ",
				nProcedureID, strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strExcludeFutureApptsFilter);

			strBaseTemp.Format("SELECT Count(LadderID) AS TotalCount FROM ( "
				" 	SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
				"	FROM LaddersT "
				" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
				"	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
				" 	WHERE NOT EXISTS      " 
				" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID    "
				" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
				" 	AND ProcInfoID IN  "
				" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     "
				"   AND LaddersT.ID IN (%s) "
				"   %s %s %s %s %s %s %s [RangeFilter] "
				" 	 GROUP BY LaddersT.PersonID "
				" ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID ",
				nProcedureID, strBaseInner, strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strExcludeFutureApptsFilter);
		}


		// (b.spivey, November 03, 2011) - PLID 46267 - Moved this up. It's not going to hurt anything to get this a little sooner. 
		//find out the value for the completion date
		CString strCompletionDate;
		if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
			strCompletionDate = "AppointmentsT.StartTime";
		}
		else {
			if (nStepTempForCompletionID < 0) {
				strCompletionDate = " LaddersT.FirstInterestDate";
			}
			else {
				
				strCompletionDate.Format(" (SELECT CompletedDate FROM StepsT WHERE StepTemplateID = %li AND LadderID = LaddersT.ID) ", nStepTempForCompletionID);

			}
		}

		// (b.spivey, November 01, 2011) - PLID 46267 - Either add the formatted filter or replace it with a blank space.
		if (m_checkExcludeUnretained.GetCheck()){ 

			CString strWithinFilter;
			strWithinFilter.Format("	AND %s >= DATEADD(mm, -%li, '%s')  ", strCompletionDate, m_nExcludeUnretainedRange, FormatDateTimeForSql(dtPivotDate, dtoDate));
			strBaseSql.Replace("[ExcludeUnretainedRange]", strWithinFilter); 
			strBaseInner.Replace("[ExcludeUnretainedRange]", strWithinFilter); 
			strBaseTemp.Replace("[ExcludeUnretainedRange]", strWithinFilter); 
		}
		else
		{
			strBaseSql.Replace("[ExcludeUnretainedRange]", " "); 
			strBaseInner.Replace("[ExcludeUnretainedRange]", " ");
			strBaseTemp.Replace("[ExcludeUnretainedRange]", " "); 
		}

		//we first need to run this as is in order to get the total number of finished procedures for this procedure
		_RecordsetPtr rsProcCount = CreateRecordsetStd(strBaseSql);
		
		double nTotalProcCount = (double)AdoFldLong(rsProcCount, "TotalCount");



		// go through all the sinarios and see if what date ranges they want to use
		//loop through and generate one big SQL statement with all the queries in it and then loop through the recordsets getting the information out
		CString strRange;
		long nRangeTo, nRangeFrom, nChartCount = 0;	
		double nRangeTotal = 0, nRangeCalc = 0, nRangeSum = 0;
		CString strPivotDate = FormatDateTimeForSql(dtPivotDate);
		CString strSql;

		//we need an array to hold the checkbox names
		CStringArray  aryDesc;
		for (int i = 0; i < m_pRangeList->GetRowCount(); i++ ) {
			
			if (VarBool(m_pRangeList->GetValue(i, 1))) {

				nRangeTo = VarLong(m_pRangeList->GetValue(i, 3));
				nRangeFrom = VarLong(m_pRangeList->GetValue(i, 4));
				/*if (nRangeFrom == -1) {
					strRange.Format(" AND %s < DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);						
				}
				else if (nRangeFrom == 0) {
					strRange.Format(" AND %s > DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);
				}
				else {*/
					strRange.Format(" AND %s > DATEADD(mm, -%li, '%s') AND %s <= DATEADD(mm, -%li, '%s') ", strCompletionDate, nRangeTo, strPivotDate, strCompletionDate, nRangeFrom, strPivotDate);
				//}

				CString str;
				if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
					str = strBaseTemp;
					
					str.Replace("[RangeFilter]", strRange);

				}
				else {
					str = strBaseTemp;
					str.Replace("[RangeFilter]", strRange);
				}
				
				strSql += str + ";\r\n";

				//add the description of the checkbox to our array
				aryDesc.Add(VarString(m_pRangeList->GetValue(i, 2)));
				nChartCount++;
			}
		}
		
		_RecordsetPtr rsRange;
		//now run the big recordset
		if (!strSql.IsEmpty()) {
			// (c.haag 2006-02-14 12:44) - PLID 19051 - We cannot assume NOCOUNT is off here.
			// If we do, then if NOCOUNT is on, then the call to NextRecordset will return NULL
			//
			rsRange = CreateRecordsetStd(CString("SET NOCOUNT OFF\r\n") + strSql);
		}

		//set the timeout back
		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

			//now loop through the recordsets in rsRange and set the values accordingly

		// (c.haag 2008-05-20 11:06) - PLID 29704 - Make sure the pie graph is clear
		m_wndPieGraph.Clear();

		for (int j = 0; j < nChartCount; j++) {
			if (NULL == rsRange) {
				// (c.haag 2006-02-15 09:04) - PLID 19051 - If this is NULL, something bad happened
				AfxThrowNxException("CMarketRetentionGraphDlg::Refresh: Failed to load NextRecordset!");
			}

			nRangeTotal = AdoFldLong(rsRange, "TotalCount");	

			if (nTotalProcCount == 0) {
				nRangeCalc = 0;
			}else {
				nRangeCalc = (nRangeTotal/nTotalProcCount) * 100;
			}

			// (c.haag 2008-04-18 14:26) - PLID 29704 - Add the value to the NexTech pie graph
			nRangeSum += nRangeCalc;
			// (z.manning 2010-10-06 11:27) - PLID 24774 - Use FormatDoubleForInterface for international compliance
			m_wndPieGraph.Add((long)(nRangeCalc * 1000.0), FormatDoubleForInterface(nRangeCalc, 1) + "% at " + aryDesc.GetAt(j));

			rsRange = rsRange->NextRecordset(NULL);
		}
			
	
		if ((100 - nRangeSum) > 0) {
			// (c.haag 2008-04-18 14:26) - PLID 29704 - Add the value to the NexTech pie graph
			// (z.manning 2010-10-06 11:27) - PLID 24774 - Use FormatDoubleForInterface for international compliance
			m_wndPieGraph.Add((long)((100 - nRangeSum) * 1000.0), FormatDoubleForInterface(100 - nRangeSum, 1) + "% Retention lost");
		}

		// (d.thompson 2009-09-15) - PLID 18793 - Add a title with the procedure, it will also show up in the print
		long nCurSel = m_pProcList->GetCurSel();
		if(nCurSel != NXDATALISTLib::sriNoRow) {
			NXDATALISTLib::IRowSettingsPtr pRowSel = m_pProcList->GetRow(nCurSel);
			long nID = VarLong(pRowSel->GetValue(epcID));

			CString strLadder = VarString(pRowSel->GetValue(epcName));
			if(nID == -2) {
				//Multi select
				strLadder = m_nxlMultiProcLabel.GetText();
			}
			m_wndPieGraph.SetTitleText(strLadder);
		}

		m_wndPieGraph.Invalidate(FALSE);
			
	}NxCatchAll("Error setting Graph");

}


BOOL CMarketRetentionGraphDlg::CheckBoxes() {

	BOOL bChecked = FALSE;

	for (int i = 0; i < m_pRangeList->GetRowCount(); i++ ) {
			
		if (VarBool(m_pRangeList->GetValue(i, 1))) {
			bChecked = TRUE;
		}
	}
	return bChecked;
}

CString CMarketRetentionGraphDlg::GetProcedureNamesFromIDs(CString strIDs) {

	// we have the information in the datalist, let's just get it from there instead of querying the data
	//it should be faster that way
	CString strReturn = "";

	//first off, see how many procedures are in the list
	long nResult = strIDs.Find(",");
	if (nResult == -1) {

		//there is only one ID, so find item name
		long nRow = m_pProcList->FindByColumn(epcID, (long)atoi(strIDs), 0, FALSE);
		if (nRow != -1) {
			return VarString(m_pProcList->GetValue(nRow, epcName));
		}
		else {
			//hmm
			ASSERT(FALSE);
			return "";
		}
	}
	else {

		//make a map out of our procedure IDs
		CMap <long, long, long, long> mapIDs;
		while(nResult != -1) {

			long nID = atoi(strIDs.Left(nResult));
			mapIDs.SetAt(nID, nID);

			//take off this string
			strIDs = strIDs.Right(strIDs.GetLength() - (nResult + 1));

			//trim the string
			strIDs.TrimRight();
			strIDs.TrimLeft();

			//now search again
			nResult = strIDs.Find(",");
		}

		strIDs.TrimRight();
		strIDs.TrimLeft();
		//now add the last one
		mapIDs.SetAt(atoi(strIDs), atoi(strIDs));


		//alrighty, now that we have our map, loop through the datalist and look to see its its an ID we want
		long p = m_pProcList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while (p) {
			// (j.gruber 2007-03-21 10:17) - PLID 25292 - this was using the m_pRangeList when it should've been using m_pProcList
			m_pProcList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			long nID = VarLong(pRow->GetValue(0));
			long nVal = mapIDs.Lookup(nID, nID);
			if (nVal > 0) {
				strReturn += VarString(pRow->GetValue(1)) + ", ";
			}
		}

		//take the last comma off
		strReturn = strReturn.Left(strReturn.GetLength() - 2);
	}

	return strReturn;
}

		
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
CString CMarketRetentionGraphDlg::GetPrintSql(BOOL bExcludeAppts, CString strPurposeIDs, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)  {

	
	try {
	
		CWaitCursor cWait;

		//get the top parameters (main Physician, date, and location
		CString strDateFrom, strDateTo, strMainPhysician, provIDs, strLocation, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		int nCategory, nResp;
		GetParameters(strDateFrom, strDateTo, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
		
		//check out the values
		if (UseFilter(mftProvider)) {
			strMainPhysician.Format(" AND %s %s) ", strProvField, provIDs);
		}
			
		if (UseFilter(mftLocation)) {
			strLocation.Format(" AND %s IN %s ", strLocationField, locIDs);
		}
		
		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, strDateFrom);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, strDateTo);
		}
	

		//now for the filters on just this tab
		long nProcedureID = -1, nLadderID = -1, nStepTempForCompletionID = -1;
		COleDateTime dtPivotDate;
		CString strProcedure = "";

		if(m_pProcList->CurSel != -1) {
			nProcedureID = VarLong(m_pProcList->GetValue(m_pProcList->CurSel, epcID));
			if (nProcedureID == -2) {
				//we are using multiple procedures
				strProcedure = m_strMultiProcedureIDs;
			}
			else {
				strProcedure = AsString(nProcedureID);
			}
		}
		else {
			strProcedure = AsString(nProcedureID);
		}

		// (d.moore 2007-10-10) - PLID 14670 - If a ladder template was selected for the filter
		//  then we need to find any matches for it in the LaddersT table.
		CString strLadderTemplateIdQuery;
		if (m_pLadderList->CurSel != NULL) {
			nLadderID = VarLong(m_pLadderList->CurSel->GetValue(elcID));
			if (nLadderID > 0) {
				strLadderTemplateIdQuery.Format(
					"AND LaddersT.ID IN "
					"(SELECT LaddersT.ID "
					"FROM LaddersT "
						"INNER JOIN StepsT "
						"ON LaddersT.ID = StepsT.LadderID "
						"INNER JOIN StepTemplatesT "
						"ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"INNER JOIN LadderTemplatesT "
						"ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
						"INNER JOIN ProcInfoT "
						"ON LaddersT.ProcInfoID = ProcInfoT.ID "
						"INNER JOIN ProcInfoDetailsT "
						"ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
					"WHERE LadderTemplatesT.ID = %li "
						"AND ProcInfoDetailsT.ProcedureID = %li) ", 
					nLadderID, nProcedureID);
			}
		}

		if(m_pStepList->CurSel != -1) {
			nStepTempForCompletionID = VarLong(m_pStepList->GetValue(m_pStepList->CurSel, escID));
		}

		dtPivotDate = VarDateTime(m_dtPivot.GetValue());

		CString strBaseSql, strBaseInner;

		//find out the value for the completion date
		CString strCompletionDate;
		if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
			strCompletionDate = "AppointmentsT.StartTime";
		}
		else {
			if (nStepTempForCompletionID < 0) {
				strCompletionDate = " LaddersT.FirstInterestDate";
			}
			else {
				
				strCompletionDate.Format(" (SELECT CompletedDate FROM StepsT WHERE StepTemplateID = %li AND LadderID = LaddersT.ID) ", nStepTempForCompletionID);

			}
		}


		// go through all the sinarios and see if what date ranges they want to use
		CString strRange;
		long nRangeTo, nRangeFrom, nChartCount = 0;	
		double nRangeTotal = 0, nRangeCalc = 0, nRangeSum = 0;
		CString strPivotDate = FormatDateTimeForSql(dtPivotDate);
		CString strTotalSql = "";
		for (int i = 0; i < m_pRangeList->GetRowCount(); i++ ) {
			
			if (VarBool(m_pRangeList->GetValue(i, 1))) {

				nRangeTo = VarLong(m_pRangeList->GetValue(i, 3));
				nRangeFrom = VarLong(m_pRangeList->GetValue(i, 4));
				/*if (nRangeFrom == -1) {
					strRange.Format(" AND %s < DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);						
				}
				else if (nRangeFrom == 0) {
					strRange.Format(" AND %s > DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);
				}
				else {*/
					strRange.Format(" AND %s > DATEADD(mm, -%li, '%s') AND %s <= DATEADD(mm, -%li, '%s') ", strCompletionDate, nRangeTo, strPivotDate, strCompletionDate, nRangeFrom, strPivotDate);
				//}

				
				// (b.spivey, November 04, 2011) - PLID 46267 - Added [ExcludeUnretainedRange] to take place of the unretained filter when we do str.Replace() later.
				if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
					//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
					strBaseInner.Format(" SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID  "
						"   FROM AppointmentsT  "
						"   LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"   = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"   LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"    WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"   AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3)  "  //procedure, minor procedure, other procedure
						"   AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						" 	  %s %s  %s  %s  %s   "
						"	  [ExcludeUnretainedRange]		"
						"   GROUP BY AppointmentsT.PatientID ",
						FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedure, strTo, strFrom, strMainPhysician, strLocation, strPatFilter);


					//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
					strBaseSql.Format("SELECT PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, '%s' As UnionType,  " 
						" AppointmentsT.Date as LastProcDate, "
						" ('%s') AS ProcedureName "
						" FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID   "
						"	FROM AppointmentsT  "
						"	LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"	= PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"	LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"	WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"	AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3) "  //procedure, minor procedure, other procedure 
						"	AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						"	AND AppointmentsT.ID IN (%s) "
						"	  %s %s  %s  %s  %s  %s  "
						"	  [ExcludeUnretainedRange]		"
						"	GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT  "
						"	ON AppointmentsT.ID = ApptsQ.ApptID  "
						"	LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"	LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"   GROUP BY PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.Date UNION ",
						VarString(m_pRangeList->GetValue(i, 2)), GetProcedureNamesFromIDs(strProcedure), FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedure, strBaseInner, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strRange);

				}
				else {

					// (d.moore 2007-10-09) - PLID 14670 - If a ladder was selected from the dropdown, 
					//  then just use that value directly instead of using the inner query.					
					strBaseInner.Format("SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
						"	FROM LaddersT "
						" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
						"	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
						" 	WHERE NOT EXISTS      " 
						" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID    "
						" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
						" 	AND ProcInfoID IN  "
						" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     "
						"   %s  %s  %s  %s  %s  %s   "
						"	  [ExcludeUnretainedRange]		"
						" 	 GROUP BY LaddersT.PersonID ",
						nProcedureID, strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter);
					
					strBaseSql.Format(" SELECT PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, '%s' As UnionType,  "
						    " LaddersT.FirstInterestDate AS LastProcDate, "
							" (SELECT Name FROM ProcedureT WHERE ID = %li) AS ProcedureName "
							" FROM  "
							" 	(SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
							" 	 FROM LaddersT "
							" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
							" 	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
							" 	WHERE NOT EXISTS       "
							" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID     "
							" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))    "
							" 	AND ProcInfoID IN "
							" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     " 
							"   AND LaddersT.ID IN (%s) "
							"   %s  %s  %s  %s  %s  %s  %s "
							"	  [ExcludeUnretainedRange]		"
							"	 GROUP BY LaddersT.PersonID   "
							"	 ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID   "
							" 	 LEFT JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID "
							" 		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
							"	 Group by PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2,   "
							"	PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, LaddersT.FirstInterestDate UNION ",							
							VarString(m_pRangeList->GetValue(i, 2)), nProcedureID, nProcedureID, strBaseInner, strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strRange);
				
				}

				strTotalSql += strBaseSql;

			}
			
		}

		//take off the last Union
		strTotalSql = strTotalSql.Left(strTotalSql.GetLength() - 6);

		CString strTmpTotal = strTotalSql;
		strTmpTotal.TrimLeft();
		strTmpTotal.TrimRight();
		if (strTmpTotal.IsEmpty() ) {
			//there are no check boxes checked
			MessageBox("Please select at least one check box before running this report");
			return "";
		}

		CString strCompleteSql;

		if (GetRemotePropertyInt("MarketRetentionOption", 1, 0, "<None>")) {
			//retained

			//PLID 19762 - check if they want to exclude futre appts
			CString strExcludeAppts = "";
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			if (bExcludeAppts) {
				if (m_strMultiPurposeIds == "-1" || m_strMultiPurposeIds == "") {
				//they are using all purposes
				strCompleteSql.Format("SELECT * FROM (%s) SubQ "
					" WHERE SubQ.PersonID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4) "
					, strTotalSql, FormatDateTimeForSql(dtDate, dtoDate));
				}
				else {

					strCompleteSql.Format("SELECT * FROM (%s) SubQ "
						" WHERE SubQ.PersonID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4 "
						" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE "
						" PurposeID IN (%s))) ", strTotalSql, FormatDateTimeForSql(dtDate, dtoDate), strPurposeIDs);
				}   
			}
			else {
				strCompleteSql.Format("SELECT * FROM (%s) SubQ ", strTotalSql);
			}
		}
		else {
			//unretained

			//PLID 19762 - Check if they want to exclude future appts
			CString strExcludeAppts = "";
			COleDateTime dtDate = COleDateTime::GetCurrentTime();
			if (bExcludeAppts) {
				if (strPurposeIDs == "-1" || strPurposeIDs == "") {
					//they are using all purposes
					strExcludeAppts.Format(" AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4) "
					, FormatDateTimeForSql(dtDate, dtoDate));
				}
				else {

					strExcludeAppts.Format(" AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4 "
						" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE "
						" PurposeID IN (%s))) ", FormatDateTimeForSql(dtDate, dtoDate), strPurposeIDs);
				}   
			}


			// (b.spivey, November 04, 2011) - PLID 46267 - Added [ExcludeUnretainedRange] to take place of the unretained filter when we do str.Replace() later.
			CString strTop;
			if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
				//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				// (j.kuziel 2014-01-21 16:09) - PLID 60419 - Replaced the WHERE NOT IN with a LEFT JOIN to speed up the query.
				strTop.Format("	SELECT PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, " 
						" AppointmentsT.Date AS LastProcDate, "
						" ('%s') AS ProcedureName "
						" FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID   "
						"	FROM AppointmentsT  "
						"	LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"	= PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"	LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"	WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"	AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3) "  //procedure, minor procedure, other procedure 
						"	AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						"	  %s %s  %s  %s  %s  "
						"	  [ExcludeUnretainedRange]	 "
						"	GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT  "
						"	ON AppointmentsT.ID = ApptsQ.ApptID  "
						"	LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"	LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"   LEFT JOIN (SELECT PersonID FROM (%s) SubQ) ExcludedPatients ON PersonT.ID = ExcludedPatients.PersonID "
						"	WHERE ExcludedPatients.PersonID IS NULL"
						"   %s   "
						"   GROUP BY PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, AppointmentsT.Date ", 				 
				GetProcedureNamesFromIDs(strProcedure), FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedure, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strTotalSql, strExcludeAppts);

			}
			else {
				strTop.Format(" SELECT PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					" LaddersT.FirstInterestDate AS LastProcDate, "
					" (SELECT Name FROM ProcedureT WHERE ID = %li) AS ProcedureName "
					"	 FROM (SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
					" 	FROM LaddersT "
					" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID     "
					" 	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
					"	WHERE NOT EXISTS      "
					"		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID     "
					"	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
					" 	AND ProcInfoID IN "
					"		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)      "
					" 	  %s  %s  %s  %s  %s  %s   "
					"	  [ExcludeUnretainedRange]	 "
					"	 GROUP BY LaddersT.PersonID "
					"	 ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID "
					"	 LEFT JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID  "
					" 	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"    WHERE PatientsT.PersonID NOT IN (SELECT PersonID FROM (%s) SubQ) "
					"    %s   "
					"	 Group by PatientsT.PersonID, UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2,   "
					"	PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, Ladderst.FirstInterestDate   ",
					nProcedureID, nProcedureID, strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter, strTotalSql, strExcludeAppts);
			}

			strCompleteSql = strTop;

		}

		// (b.spivey, November 03, 2011) - PLID 46267 - Replace with a formatted filter or a blank space. 
		if (m_checkExcludeUnretained.GetCheck()){ 
			CString strWithinFilter; 
			strWithinFilter.Format(" AND %s >= DATEADD(mm, -%li, '%s')  ", strCompletionDate, m_nExcludeUnretainedRange, strPivotDate);
			strCompleteSql.Replace("[ExcludeUnretainedRange]", strWithinFilter); 
		}
		else{
			strCompleteSql.Replace("[ExcludeUnretainedRange]", " "); 
		}


		return strCompleteSql;

	}NxCatchAll("Error setting Graph");

	return "";

}

void CMarketRetentionGraphDlg::OnChangePivotDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
	ResetGraph(true, "", true);	//(a.wilson 2011-10-7) PLID 38789

	*pResult = 0;	
}

void CMarketRetentionGraphDlg::OnEditingFinishedRangeList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (nCol == 1) {
	
		//check or uncheck the RangeList Accordingly
		// (j.jones 2010-01-26 10:20) - PLID 34354 - removed merge range list
		//m_pMergeRangeList->PutValue(nRow + 1, nCol, varNewValue);
		
		// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
		ResetGraph(true, "", true);	//(a.wilson 2011-10-7) PLID 38789
	}
	
}

void CMarketRetentionGraphDlg::OnSelChosenRetentionStepList(long nRow) 
{
	// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
	ResetGraph(true, "", true); //(a.wilson 2011-10-7) PLID 38789
	
}

void CMarketRetentionGraphDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	//(e.lally 2009-08-28) PLID 35308 - Added try/catch
	try {
		//(a.wilson 2011-10-5) PLID 38789 - added force refresh to prevent refreshing when coming back to module.
		if (m_bRenderedOnce || (m_bActive && bForceRefresh)) {
			CMarketRenderButtonStatus mrbs(this);
			if (m_bActive && !m_bRenderedOnce) // refresh from toolbar or switching to module
			{
				// (j.gruber 2006-12-05 11:38) - PLID 22889 - don't update if we are in a report window
				CNxTabView *pView = GetMainFrame()->GetActiveView();
				if (pView) {
					pView->RedrawWindow();
					//(a.wilson 2011-10-5) PLID 38789
					OnGo();
				}
				else {

					//if we aren't in the view, we don't need to refresh
					return;
				}
			}
			m_bRenderedOnce = false;
			Refresh();
		}
		else { // set filters
			if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
				SetFilter(mfFirstContactDate, mftDate);
				SetFilter(mfApptLocation, mftLocation);
				SetFilter(mfApptProvider, mftProvider);
			}
			else  {
				SetFilter(mfFirstContactDate, mftDate);
				SetFilter(mfPatientLocation, mftLocation);
				SetFilter(mfPatientProvider, mftProvider);
			}
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar

			//PLID 21773 - initialize the date combo to all dates
			GetMainFrame()->m_pDocToolBar->SetDateOptionCombo(mdotAll);

		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
	}NxCatchAll(__FUNCTION__);
}

void CMarketRetentionGraphDlg::OnRequeryFinishedRetentionProcedureList(short nFlags) 
{
		if(m_pProcList->GetRowCount() > 0) {
		
			if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
				//PLID 19760- let them choose multiple procedures
				IRowSettingsPtr pRow = m_pProcList->GetRow(-1);
				pRow->PutValue(epcID, (long) -2);
				pRow->PutValue(epcName, _variant_t("<Multiple Procedures>"));
				m_pProcList->InsertRow(pRow, 0);

				m_pProcList->PutCurSel(1);
				//OnSelChosenProcedureList(1);
			}
			else {
				// (d.moore 2007-09-05) - PLID 14670 - Load the Ladder list.
				m_pProcList->PutCurSel(0);
				long nCurSel = m_pProcList->GetCurSel();
				long nProcID = -1;
				
				if(nCurSel != -1) {
					nProcID = VarLong(m_pProcList->GetValue(nCurSel, epcID));
				}

				if (nProcID > 0) {
					CString strWhere;
					strWhere.Format(
						"ProcedureLadderTemplateT.ProcedureID = %li "
						"AND LadderTemplatesT.ID IN "
							"(SELECT LadderTemplateID "
								"FROM StepTemplatesT "
								"WHERE Action=20 AND Inactive = 0)", nProcID);
					m_pLadderList->PutWhereClause((_bstr_t)strWhere);
					m_pLadderList->Requery();
				} else {
					// There was nothing in the procedure list to select from.
					m_pLadderList->Clear();
					m_pStepList->Clear();
				}
			}
		}
	
}


// (j.gruber 2007-03-21 10:01) - PLID 25260 - added filter strings to graphs
CString CMarketRetentionGraphDlg::GetProcedureFilter() {

	CString strReturn;
	//get the currently selected item
	long nCurSel = m_pProcList->CurSel;

	CString strProcName;
	if (nCurSel != -1) {

		long nID = VarLong(m_pProcList->GetValue(nCurSel, epcID));

		if (nID == -2) {
			//multiple Procedures
			strProcName = GetProcedureNamesFromIDs(m_strMultiProcedureIDs);
		}
		else {
			strProcName = VarString(m_pProcList->GetValue(nCurSel, epcName), "");
		}

		strReturn.Format("Procedure: %s", strProcName);
	}
	

	return strReturn;

}

void CMarketRetentionGraphDlg::Print(CDC * pDC, CPrintInfo * pInfo)
{
	// (c.haag 2008-04-18 14:32) - PLID 29715 - The legacy code forced landscape mode,
	// so we will do the same
	DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
	ASSERT(pInfoDevMode);
	pInfoDevMode->dmOrientation = DMORIENT_LANDSCAPE;
	GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);

	// (c.haag 2008-04-18 14:32) - PLID 29715 - Rewritten to use the pie graph window.
	// This is much simpler without the presence of an MSChart control
	CRect rectPage = pInfo->m_rectDraw;

	// Set Margins
	rectPage.top +=rectPage.bottom/52;
	rectPage.bottom-=rectPage.bottom/52;
	rectPage.left+=300;
	rectPage.right-=300;

	// Do the drawing
	m_wndPieGraph.Draw(pDC, rectPage);	
}


CString CMarketRetentionGraphDlg::GenerateRetained(CString strProcedures, long nStepTempForCompletionID, COleDateTime dtPivotDate, CString strFilter) {

	CString strBaseSql, strBaseInner;

	//find out the value for the completion date
	CString strCompletionDate;
	if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
		strCompletionDate = "AppointmentsT.StartTime";
	}
	else {
		if (nStepTempForCompletionID < 0) {
			strCompletionDate = " LaddersT.FirstInterestDate";
		}
		else {
			
			strCompletionDate.Format(" (SELECT CompletedDate FROM StepsT WHERE StepTemplateID = %li AND LadderID = LaddersT.ID) ", nStepTempForCompletionID);
		}
	}

	// go through all the scenarios and see if what date ranges they want to use
	CString strRange;
	long nRangeTo, nRangeFrom, nChartCount = 0;	
	double nRangeTotal = 0, nRangeCalc = 0, nRangeSum = 0;
	CString strPivotDate = FormatDateTimeForSql(dtPivotDate);
	CString strTotalSql = "";

	// (b.spivey, November 02, 2011) - PLID 46267 - Append unretained filter if we're using it. 
	if (m_checkExcludeUnretained.GetCheck()){ 
		CString strWithinFilter;
		strWithinFilter.Format("AND %s >= DATEADD(mm, -%li, '%s')  ", strCompletionDate, m_nExcludeUnretainedRange, strPivotDate);
		strFilter.Append(strWithinFilter); 
	}

	// (j.jones 2010-01-26 10:31) - PLID 34354 - use the regular range
	for (int i = 0; i < m_pRangeList->GetRowCount(); i++ ) {
		
		if (VarBool(m_pRangeList->GetValue(i, 1))) {

			nRangeTo = VarLong(m_pRangeList->GetValue(i, 3));
			nRangeFrom = VarLong(m_pRangeList->GetValue(i, 4));
			/*if (nRangeFrom == -1) {
					strRange.Format(" AND %s < DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);						
			}
			else if (nRangeFrom == 0) {
				strRange.Format(" AND %s > DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);
			}
			else {*/
				strRange.Format(" AND %s > DATEADD(mm, -%li, '%s') AND %s <= DATEADD(mm, -%li, '%s') ", strCompletionDate, nRangeTo, strPivotDate, strCompletionDate, nRangeFrom, strPivotDate);
			//}

			if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {

				//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				strBaseInner.Format(" SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID  "
						"   FROM AppointmentsT  "
						"   LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"   = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"   LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"    WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"   AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3)  "  //procedure, minor procedure, other procedure
						"   AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						" 	  %s  "
						"   GROUP BY AppointmentsT.PatientID ",
						FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedures, strFilter);


				//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
					strBaseSql.Format("SELECT PersonT.ID  " 
						" FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID   "
						"	FROM AppointmentsT  "
						"	LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"	= PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"	LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"	WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"	AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3) "  //procedure, minor procedure, other procedure 
						"	AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						"	AND AppointmentsT.ID IN (%s) "
						"	  %s   %s  "
						"	GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT  "
						"	ON AppointmentsT.ID = ApptsQ.ApptID  "
						"	LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"	LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"   GROUP BY PersonT.ID UNION ",
						FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedures, strBaseInner, strFilter, strRange);
		
			}
			else {
				// (d.moore 2007-10-09) - PLID 14670 - If a ladder template was selected for the filter then
				//  a query will be added to find a match for the template in the LaddersT table.
				strBaseInner.Format("SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
						"	FROM LaddersT "
						" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
						"	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
						" 	WHERE NOT EXISTS      " 
						" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID    "
						" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
						" 	AND ProcInfoID IN  "
						" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     "
						"   %s  "
						" 	 GROUP BY LaddersT.PersonID ",
						strProcedures, strFilter );

					strBaseSql.Format(" SELECT PersonT.ID"
							" FROM  "
							" 	(SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
							" 	 FROM LaddersT "
							" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
							" 	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
							" 	WHERE NOT EXISTS       "
							" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID     "
							" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))    "
							" 	AND ProcInfoID IN "
							" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID = %li)     " 
							"   AND LaddersT.ID IN (%s) "
							"   %s %s  "
							"	 GROUP BY LaddersT.PersonID   "
							"	 ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID   "
							" 	 LEFT JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID "
							" 		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
							"	 Group by PersonT.ID UNION ",
							strProcedures, strBaseInner, strFilter, strRange);
				
			}

			strTotalSql += strBaseSql;

		}
		
	}

	//take off the last Union
	strTotalSql = strTotalSql.Left(strTotalSql.GetLength() - 6);

	//retained
	return strTotalSql;

}


CString CMarketRetentionGraphDlg::GenerateUnRetained(CString strProcedures, long nStepTempForCompletionID, COleDateTime dtPivotDate, CString strFilter) {

	CString strBaseSql, strBaseInner;

	//find out the value for the completion date
	CString strCompletionDate;
	if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
		strCompletionDate = "AppointmentsT.StartTime";
	}
	else {
		if (nStepTempForCompletionID < 0) {
			strCompletionDate = " LaddersT.FirstInterestDate";
		}
		else {
			
			strCompletionDate.Format(" (SELECT CompletedDate FROM StepsT WHERE StepTemplateID = %li AND LadderID = LaddersT.ID) ", nStepTempForCompletionID);
		}
	}

	// go through all the sinarios and see if what date ranges they want to use
	CString strRange;
	long nRangeTo, nRangeFrom, nChartCount = 0;	
	double nRangeTotal = 0, nRangeCalc = 0, nRangeSum = 0;
	CString strPivotDate = FormatDateTimeForSql(dtPivotDate);
	CString strTotalSql = "";

	// (b.spivey, November 02, 2011) - PLID 46267 - Append to the existing filter if we're filtering out unretained patients. 
	if (m_checkExcludeUnretained.GetCheck()){ 
		CString strWithinFilter;
		strWithinFilter.Format("	AND %s >= DATEADD(mm, -%li, '%s')  ", strCompletionDate, m_nExcludeUnretainedRange, strPivotDate); 
		strFilter.Append(strWithinFilter); 
	}

	for (int i = 0; i < m_pRangeList->GetRowCount(); i++ ) {
		
		if (VarBool(m_pRangeList->GetValue(i, 1))) {

			nRangeTo = VarLong(m_pRangeList->GetValue(i, 3));
			nRangeFrom = VarLong(m_pRangeList->GetValue(i, 4));
			/*if (nRangeFrom == -1) {
					strRange.Format(" AND %s < DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);						
			}
			else if (nRangeFrom == 0) {
				strRange.Format(" AND %s > DATEADD(mm, -%li, '%s')", strCompletionDate, nRangeTo, strPivotDate);
			}
			else {*/
				strRange.Format(" AND %s > DATEADD(mm, -%li, '%s') AND %s <= DATEADD(mm, -%li, '%s') ", strCompletionDate, nRangeTo, strPivotDate, strCompletionDate, nRangeFrom, strPivotDate);
			//}
			//run the recordset
			if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
				//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				strBaseInner.Format(" SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID  "
						"   FROM AppointmentsT  "
						"   LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"   = PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"   LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"    WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"   AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3)  "  //procedure, minor procedure, other procedure
						"   AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						" 	  %s   "
						"   GROUP BY AppointmentsT.PatientID ",
						FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedures, strFilter);


				//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
					strBaseSql.Format("SELECT PersonT.ID  " 
						" FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID   "
						"	FROM AppointmentsT  "
						"	LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"	= PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"	LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"	WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"	AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3) "  //procedure, minor procedure, other procedure 
						"	AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						"	AND AppointmentsT.ID IN (%s) "
						"	  %s %s  "
						"	GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT  "
						"	ON AppointmentsT.ID = ApptsQ.ApptID  "
						"	LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"	LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"   GROUP BY PersonT.ID UNION ",
						FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedures, strBaseInner, strFilter, strRange);
		
			}
			else {
				strBaseInner.Format("SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
						"	FROM LaddersT "
						" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
						"	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
						" 	WHERE NOT EXISTS      " 
						" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID    "
						" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
						" 	AND ProcInfoID IN  "
						" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID IN (%s))     "
						"   %s    "
						" 	 GROUP BY LaddersT.PersonID ",
						strProcedures, strFilter );

				strBaseSql.Format(" SELECT PersonT.ID"
						" FROM  "
						" 	(SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
						" 	 FROM LaddersT "
						" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID      "
						" 	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
						" 	WHERE NOT EXISTS       "
						" 		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID     "
						" 	  	 NOT IN (SELECT StepID FROM EventAppliesT))    "
						" 	AND ProcInfoID IN "
						" 		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID IN (%s))     " 
						"   AND LaddersT.ID IN (%s) "
						"   %s %s  "
						"	 GROUP BY LaddersT.PersonID   "
						"	 ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID   "
						" 	 LEFT JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID "
						" 		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"	 Group by PersonT.ID UNION ",
						strProcedures, strBaseInner, strFilter, strRange);
		}

			strTotalSql += strBaseSql;

		}
		
	}

	//take off the last Union
	strTotalSql = strTotalSql.Left(strTotalSql.GetLength() - 6);

	//unretained
	CString strTop;
	if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
			strTop.Format("	SELECT PersonT.ID  " 
						" FROM (SELECT  CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, AppointmentsT.StartTime, 121) + ' ' + CONVERT(nvarchar, AppointmentsT.ID)), 25, 80)) AS ApptID   "
						"	FROM AppointmentsT  "
						"	LEFT JOIN PersonT ON AppointmentsT.PatientID   "
						"	= PersonT.ID LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID   "
						"	LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID   "
						"	WHERE AppointmentsT.StartTime < '%s' AND AppointmentsT.Status <> 4  "
						"	AND AppointmentsT.ShowState <> 3 AND (AptTypeT.Category = 4 OR AptTypeT.Category = 3) "  //procedure, minor procedure, other procedure 
						"	AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN (%s)) "
						"	  %s "
						"	GROUP BY AppointmentsT.PatientID) ApptsQ INNER JOIN AppointmentsT  "
						"	ON AppointmentsT.ID = ApptsQ.ApptID  "
						"	LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"	LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						"   WHERE PersonT.ID NOT IN (SELECT ID FROM (%s) SubQ) "
						"   GROUP BY PersonT.ID  ", 				 
				FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), strProcedures, strFilter, strTotalSql);

	}
	else {
		strTop.Format(" SELECT PersonT.ID "
					"	 FROM (SELECT CONVERT(int, SUBSTRING(MAX(CONVERT(nvarchar, LaddersT.FirstInterestDate, 121) + ' ' + CONVERT(nvarchar, LaddersT.ID)), 25, 80)) AS LadderID "
					" 	FROM LaddersT "
					" 	LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID     "
					" 	INNER  JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID      "
					"	WHERE NOT EXISTS      "
					"		(SELECT ID FROM StepsT WHERE LadderID = LaddersT.ID AND StepsT.ID     "
					"	  	 NOT IN (SELECT StepID FROM EventAppliesT))   "
					" 	AND ProcInfoID IN "
					"		(SELECT ProcInfoID FROM ProcInfoDetailsT WHERE ProcedureID IN (%s))      "
					" 	  %s    "
					"	 GROUP BY LaddersT.PersonID "
					"	 ) LaddersQ INNER JOIN LaddersT ON  LaddersT.ID = LaddersQ.LadderID "
					"	 LEFT JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID  "
					" 	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
					"    WHERE PatientsT.PersonID NOT IN (SELECT ID FROM (%s) SubQ) "
					"	 Group by PersonT.ID ",
					strProcedures, strFilter, strTotalSql);
	}

	return strTop;
}





void CMarketRetentionGraphDlg::OnCreateMergeGroup() 
{
	try {

		//first we have to get the name of the group
		CString strGroup;
		long nResult = InputBoxLimited(this, "Enter a group name:", strGroup,"",50,false,false,NULL);
		
		if (nResult != IDOK) {
			return;
		}
			
		//check to make sure that it is non an empty string
		if (strGroup.IsEmpty()) {
			MsgBox("Please enter a valid group name");
			return;
		}

		//make sure that it doesn't already exist
		if (ReturnsRecords("SELECT ID FROM GroupsT WHERE Name = '%s'", _Q(strGroup))) {
			MsgBox("The group name already exists");
			return;
		}

		//now run the query
	
		CWaitCursor cWait;

		//get the top parameters (main Physician, date, and location
		CString strDateFrom, strDateTo, strMainPhysician, provIDs, strLocation, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		int nCategory, nResp;
		
		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;
		
		GetParameters(strDateFrom, strDateTo, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
		
		//check out the values
		if (UseFilter(mftProvider))
			strMainPhysician.Format(" AND %s %s) ", strProvField, provIDs);
		if (UseFilter(mftLocation)) //user personT location instead of scheduler location
			strLocation.Format(" AND %s IN %s ", strLocationField, locIDs);
		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, strDateFrom);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, strDateTo);
		}

		//now for the filters on just this tab
		long nProcedureID = -1, nLadderID = -1, nStepTempForCompletionID = -1;
		COleDateTime dtPivotDate;
		CString strProcedures = "";

		if(m_pProcList->CurSel != -1) {
			nProcedureID = VarLong(m_pProcList->GetValue(m_pProcList->CurSel, epcID));
			if (nProcedureID == -2) {
				//we are using multiple procedures
				strProcedures = m_strMultiProcedureIDs;
			}
			else {
				strProcedures = AsString(nProcedureID);
			}
		}
		else {
			strProcedures = AsString(nProcedureID);
		}

		// (d.moore 2007-10-10) - PLID 14670 - If a ladder template was selected for the filter
		//  then we need to find any matches for it in the LaddersT table.
		CString strLadderTemplateIdQuery;
		if (m_pLadderList->CurSel != NULL) {
			nLadderID = VarLong(m_pLadderList->CurSel->GetValue(elcID));
			if (nLadderID > 0) {
				strLadderTemplateIdQuery.Format(
					"AND LaddersT.ID IN "
					"(SELECT LaddersT.ID "
					"FROM LaddersT "
						"INNER JOIN StepsT "
						"ON LaddersT.ID = StepsT.LadderID "
						"INNER JOIN StepTemplatesT "
						"ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"INNER JOIN LadderTemplatesT "
						"ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
						"INNER JOIN ProcInfoT "
						"ON LaddersT.ProcInfoID = ProcInfoT.ID "
						"INNER JOIN ProcInfoDetailsT "
						"ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
					"WHERE LadderTemplatesT.ID = %li "
					"AND ProcInfoDetailsT.ProcedureID = %li) ", 
					nLadderID, nProcedureID);
			}
		}

		if(m_pStepList->CurSel != -1) {
			nStepTempForCompletionID = VarLong(m_pStepList->GetValue(m_pStepList->CurSel, escID));
		}

		dtPivotDate = VarDateTime(m_dtPivot.GetValue());

		CString strFilter;
		strFilter.Format(" %s  %s  %s  %s  %s  %s ", strLadderTemplateIdQuery, strTo, strFrom, strMainPhysician, strLocation, strPatFilter);

		CString strSql;
		// (b.spivey, November 04, 2011) - PLID 38861 - Depending on the selection of the row at the time we press the button, 
		//		we create a merge group for retained, unretained, or both. 
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMergeOptionList->GetCurSel();
		int nMergeOptionID = VarLong(pRow->GetValue(emmcID), emmoBoth);
		switch(nMergeOptionID)
		{
			case emmoRetained:
			{
				strSql = GenerateRetained(strProcedures, nStepTempForCompletionID, dtPivotDate, strFilter);
				break;
			}
			case emmoUnretained:
			{
				strSql = GenerateUnRetained(strProcedures, nStepTempForCompletionID, dtPivotDate, strFilter);
				break;
			}
			case emmoBoth:
			default:
			{
				strSql = GenerateRetained(strProcedures, nStepTempForCompletionID, dtPivotDate, strFilter);

				// (j.jones 2010-01-26 10:30) - PLID 34354 - added checkbox to exclude
				// unretained patients from the merge group		
				CString strUnretainedSql = GenerateUnRetained(strProcedures, nStepTempForCompletionID, dtPivotDate, strFilter);
				if(strSql.IsEmpty()) {
					strSql = strUnretainedSql;
				}
				else {
					strSql += "UNION " + strUnretainedSql;
				}
			}
		}

		if(strSql.IsEmpty()) {
			MessageBox("Please select at least one item in the list under 'Create Merge Group'.");
			return;
		}

		CString strComplete;
		COleDateTime dtDate = COleDateTime::GetCurrentTime();
		//take out the people that have appts
		if (IsDlgButtonChecked(IDC_EXCLUDE_APPTS)) {
			if (m_strMultiPurposeIds == "-1" || m_strMultiPurposeIds == "") {
				//they are using all purposes
				strComplete.Format("SELECT ID FROM PersonT "
					" WHERE PersonT.ID IN (%s)   "
					" AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4) "
					, strSql, FormatDateTimeForSql(dtDate, dtoDate));
			}
			else {

				strComplete.Format("SELECT ID FROM PersonT "
					" WHERE PersonT.ID IN (%s)   "
					" AND PersonT.ID NOT IN (SELECT PatientID FROM AppointmentsT WHERE StartTime > '%s' AND Status <> 4 "
					" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE "
					" PurposeID IN (%s))) ", strSql, FormatDateTimeForSql(dtDate, dtoDate),m_strMultiPurposeIds);
			}   
		}
		else {
			strComplete = strSql;
		}

#ifdef _DEBUG 
		MessageBox(strComplete);
#endif
		
		//set the timeout
		CIncreaseCommandTimeout ict(600);


		long nGroupID = NewNumber("GroupsT", "ID");

		BEGIN_TRANS("Market_Retention_Group") {
		ExecuteSql("INSERT INTO GroupsT (ID, Name) VALUES (%li, '%s')", nGroupID, _Q(strGroup));
		ExecuteSql("INSERT INTO GroupDetailsT (GroupID, PersonID) "
			" SELECT %li, ID FROM (%s) SubQ Group By SubQ.ID ", nGroupID, strComplete);

		CClient::RefreshTable(NetUtils::Groups, nGroupID);
		MessageBox("Group Created Successfully");
				
		}END_TRANS_CATCH_ALL("Market_Retention_Group");

		//set it back
		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}
	
	}NxCatchAll("Error Creating Group");
}

void CMarketRetentionGraphDlg::OnExcludeAppts() 
{
	try {

		// (j.jones 2010-01-26 11:01) - PLID 34354 - this now applies to the graph,
		// so we need to reset it
		ResetGraph(true, "", true); //(a.wilson 2011-10-7) PLID 38789

		if (IsDlgButtonChecked(IDC_EXCLUDE_APPTS)) {
			GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);	
}

void CMarketRetentionGraphDlg::OnRequeryFinishedRetentionPurposeList(short nFlags) 
{
	IRowSettingsPtr pRow;
	pRow = m_pPurposeList->GetRow(-1);
	pRow->PutValue(0, (long) -2);
	pRow->PutValue(1, _variant_t("{Multiple Purposes}"));
	m_pPurposeList->InsertRow(pRow, 0);
	
	pRow = m_pPurposeList->GetRow(-1);
	pRow->PutValue(0, (long) -1);
	pRow->PutValue(1, _variant_t("{All Purposes}"));
	m_pPurposeList->InsertRow(pRow, 0);
	
}

void CMarketRetentionGraphDlg::OnSelChangedRetentionPurposeList(long nNewSel) 
{
	ResetGraph(true, "", true); //(a.wilson 2011-10-7) PLID 38789
}

	


void CMarketRetentionGraphDlg::OnSelChosenRetentionPurposeList(long nRow) 
{

	if(m_pPurposeList->CurSel == -1) {
		m_strMultiPurposeIds = "-1";
		return;
	}
	
	//check to see if it is -2
	if (VarLong(m_pPurposeList->GetValue(nRow, 0)) == -2) {
	
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptPurposeT");

		//preselect existing stuff
		CString str = m_strMultiPurposeIds;
		long nComma = str.Find(",");
		while(nComma > 0) {
			dlg.PreSelect(atoi(str.Left(nComma)));
			str = str.Right(str.GetLength() - (nComma + 1));

			nComma = str.Find(",");
		}

		if(dlg.Open("AptPurposeT", "", "ID", "Name", "Select purposes") == IDCANCEL) {
			m_pPurposeList->SetSelByColumn(0, (long)0);
			return;
		}
		else {
			CString strOut = dlg.GetMultiSelectIDString();
			if(strOut.IsEmpty()) {
				MsgBox("You cannot filter on no purposes.  The filter will be reset to { All Purposes }");
				m_pPurposeList->SetSelByColumn(0, (long)-1);
				long nNewId = -1;	//just change it here, and let us continue on
			}

			m_strMultiPurposeIds = strOut;
			m_strMultiPurposeIds.Replace(" ", ",");
			
		}
	}
	else if (VarLong(m_pPurposeList->GetValue(nRow, 0)) == -1) {
		m_strMultiPurposeIds = "-1";
	}
	else {
		m_strMultiPurposeIds = AsString(m_pPurposeList->GetValue(nRow, 0));
	}	
	ResetGraph(true, "", true); //(a.wilson 2011-10-7) PLID 38789
}

void CMarketRetentionGraphDlg::OnUseSchedule() 
{
	RefreshTab(false); // a.walling 5/22/06 PLID 20695 Refreshes are now manual
	ResetGraph(true, "", true);	//(a.wilson 2011-10-7) PLID 38789
		
}

void CMarketRetentionGraphDlg::OnUseTracking() 
{
	RefreshTab(false); // a.walling 5/22/06 PLID 20695 Refreshes are now manual
	ResetGraph(true, "", true);	//(a.wilson 2011-10-7) PLID 38789
		
}

CString CMarketRetentionGraphDlg::GetUnretainedTitle() {

	//loop through the list and generate the title

	long nLastChecked = -1;
	BOOL bUseGreaterThan = TRUE;
	CString strUnit = "Months";
	for (int i=0; i < m_pRangeList->GetRowCount(); i++) {

		if (VarBool(m_pRangeList->GetValue(i, 1))) {

			if ((nLastChecked - i) != -1) {
				bUseGreaterThan = FALSE;
			}
			nLastChecked = i;
		}
	}

	//generate the greater than title
	long nLastCheckedValue = VarLong(m_pRangeList->GetValue(nLastChecked, 3));

	if (nLastChecked >= 8) {
		//convert the number into years
		nLastCheckedValue = nLastCheckedValue / 12;
		strUnit = "Years";
	}
		
	return "Unretained Patients Greater Than " + AsString(nLastCheckedValue) + " " + strUnit;
				
}

CString CMarketRetentionGraphDlg::GetUseTracking() {

	if (IsDlgButtonChecked(IDC_USE_SCHEDULE)) {
		return "false";
	}
	else {
		return "true";
	}
}

void CMarketRetentionGraphDlg::OnRetentionRangeSetup() 
{
	//pop up the range dialog
	CMarketRangeConfigDlg dlg(this);
	long nResult = dlg.DoModal();

	if (nResult == 1) {
		//refresh the range lists, checking the first two
		InitalizeLists();
	}

	ResetGraph(true, "", true); //(a.wilson 2011-10-7) PLID 38789
}

/*// (j.jones 2010-01-26 10:20) - PLID 34354 - removed merge range list
void CMarketRetentionGraphDlg::OnRequeryFinishedMergeRangeList(short nFlags) 
{
	
	IRowSettingsPtr pRow = m_pMergeRangeList->GetRow(-1);
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	//add the unretained row
	pRow = m_pMergeRangeList->GetRow(-1);
	pRow->PutValue(0, (long) -1);
	pRow->PutValue(1, varFalse);
	pRow->PutValue(2, _variant_t("UnRetained Patients"));
	pRow->PutValue(3, (long) -1);
	pRow->PutValue(4, (long) -1);
	pRow->PutValue(5, (long) 0);
	m_pMergeRangeList->InsertRow(pRow, 0);

	m_pMergeRangeList->Sort();
	
	//set the first two check boxes in each list to be checked
	// and the rest to be false
	long nCount = 0;
	long p = m_pMergeRangeList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while (p) {
		m_pMergeRangeList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
		if (nCount >=1 && nCount < 3) {
			pRow->PutValue(1, varTrue);
		}
		else {
			pRow->PutValue(1, varFalse);
		}
		nCount++;
		
	}
	
}
*/

void CMarketRetentionGraphDlg::OnRequeryFinishedRangeList(short nFlags) 
{

	IRowSettingsPtr pRow;
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	m_pRangeList->Sort();

	//set the first two check boxes in each list to be checked
	// and the rest to be false
	long nCount = 0;
	long p = m_pRangeList->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while (p) {
		m_pRangeList->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
		if (nCount < 2) {
			pRow->PutValue(1, varTrue);
		}
		else {
			pRow->PutValue(1, varFalse);
		}
		nCount++;
		
	}

	// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
	
}

BOOL CMarketRetentionGraphDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (GetDlgItem(IDC_MULTI_PROC_LIST)->IsWindowVisible()) {
	
		CRect rc;
		GetDlgItem(IDC_MULTI_PROC_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	
	return CMarketingDlg::OnSetCursor(pWnd, nHitTest, message);
}


LRESULT CMarketRetentionGraphDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
	case IDC_MULTI_PROC_LIST:
		if (OnMultiProc()) {
			// Refresh(); a.walling 5/22/06 PLID 20695 Refreshes are now manual
			ResetGraph(true, "", true); //(a.wilson 2011-10-7) PLID 38789
		}
		break;
	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}


BOOL CMarketRetentionGraphDlg::OnMultiProc() {

	//open the multiprocedure dialog
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProcedureT");

	//preselect existing stuff
	CString str = m_strMultiProcedureIDs;
	long nComma = str.Find(",");
	while(nComma > 0) {
		dlg.PreSelect(atoi(str.Left(nComma)));
		str = str.Right(str.GetLength() - (nComma + 1));

		nComma = str.Find(",");
	}

	//add the last one
	dlg.PreSelect(atoi(str));

	if(dlg.Open("ProcedureT", "Recur <> 0", "ID", "Name", "Select Procedures", 1) == IDCANCEL) {
		if (m_strMultiProcedureIDs.Find(",") == -1) {
			m_pProcList->SetSelByColumn(epcID, (long)atoi(m_strMultiProcedureIDs));
		}
		return FALSE;
	}
	else {
		CString strOut = dlg.GetMultiSelectIDString();
		m_strMultiProcedureIDs = strOut;
		m_strMultiProcedureIDs.Replace(" ", ",");

		//see if there is only 1 value
		if (m_strMultiProcedureIDs.Find(",") == -1) {
			GetDlgItem(IDC_RETENTION_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MULTI_PROC_LIST)->ShowWindow(SW_HIDE);
			m_pProcList->SetSelByColumn(epcID, (long)atoi(m_strMultiProcedureIDs));
		}
		else {
			//there are more than one
			GetDlgItem(IDC_RETENTION_PROCEDURE_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MULTI_PROC_LIST)->ShowWindow(SW_SHOW);
			m_nxlMultiProcLabel.SetText(dlg.GetMultiSelectString());
			m_nxlMultiProcLabel.SetType(dtsHyperlink);
			//redraw
			Invalidate();
		}
		return TRUE;
		
	}


}

void CMarketRetentionGraphDlg::OnGo() 
{
	if (!m_bRenderedOnce) {
		m_wndPieGraph.SetTitleText("");
	}
	m_bRenderedOnce = true;
	UpdateView();
	// (c.haag 2008-04-18 12:42) - PLID 29715 - Show the pie graph window
	m_wndPieGraph.ShowWindow(SW_SHOW);
}

void CMarketRetentionGraphDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	if (bShow) {
		// (a.walling 2006-10-17 09:39) - PLID 22764 - Ensure filters are set correctly if we are showing (not hiding)
		RefreshTab(false);
	}

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

void CMarketRetentionGraphDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
// a.walling PLID 20695 5/25/06 set the graph to a blank state and update the status of any controls
//			set bClear to false if you just want to reset/enable the render button
// (c.haag 2007-03-15 16:53) - PLID 24253 - Added support for forced refreshes (the parent view uses them)
{
	//(a.wilson 2011-10-7) PLID 38789
	if ((m_bActive && !bForceReset)) { // switching from another tab or module.
		return;
	}

	if (bClear) {
		m_wndPieGraph.Clear();
		m_wndPieGraph.SetTitleText(strTitle);
		// No need to invalidate because we're about to hide it
	}

	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOW);
	m_wndPieGraph.ShowWindow(SW_HIDE); // (c.haag 2008-04-18 14:37) - PLID 29715 - Hide the pie graph window
}

LRESULT CMarketRetentionGraphDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}

void CMarketRetentionGraphDlg::OnRequeryFinishedRetentionLadderList(short nFlags) 
{
	// (d.moore 2007-09-05) - PLID 14670 - When the list first loads attempt to set
	//  the first row to selected and then load the Step list based on that selection.
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLadderList->GetFirstRow();
		long nLadderID = -1;
		if (pRow != NULL) {
			m_pLadderList->PutCurSel(pRow);
			nLadderID = VarLong(pRow->GetValue(elcID), -1);
		}

		if (nLadderID <= 0) {
			// There were no ladders to choose from, or the selected row had
			//  an invalid ID value.
			m_pStepList->Clear();
			//add the <Ladder Begin Date> to the Step list
			IRowSettingsPtr pNewRow = m_pStepList->GetRow(-1);
			pNewRow->PutValue(escID, (long)-2);
			pNewRow->PutValue(escName, "<Ladder Begin Date>");
			m_pStepList->InsertRow(pNewRow, 0);
			m_pStepList->SetSelByColumn(escID, (long)-2);
			return;
		}
		
		// Now load the Step list.
		CString strWhere;
		strWhere.Format("LadderTemplateID = %li And Inactive = 0", nLadderID);
		m_pStepList->WhereClause = (LPCTSTR)strWhere;
		m_pStepList->Requery();

		//add the <Ladder Begin Date> to the Step list
		IRowSettingsPtr pNewRow = m_pStepList->GetRow(-1);
		pNewRow->PutValue(escID, (long)-2);
		pNewRow->PutValue(escName, "<Ladder Begin Date>");
		m_pStepList->InsertRow(pNewRow, 0);
		m_pStepList->SetSelByColumn(escID, (long)-2);

	} NxCatchAll("CMarketRetentionGraphDlg::OnRequeryFinishedRetentionLadderList");
}

void CMarketRetentionGraphDlg::OnSelChosenRetentionLadderList(LPDISPATCH lpRow) 
{
	// (d.moore 2007-09-04) - PLID 14670 - When a selection is made from the ladder list
	//  we need to fill the list of steps to go along with the ladder.
	try {
		if (lpRow == NULL) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//get the LadderTemplateID
		long nLadderTemplateID = VarLong(pRow->GetValue(0));

		//requery the step list based on this
		CString strWhere;
		strWhere.Format("LadderTemplateID = %li And Inactive = 0", nLadderTemplateID);

		m_pStepList->WhereClause = (LPCTSTR)strWhere;
		m_pStepList->Requery();

		//add the <Ladder Begin Date> to the Step list
		IRowSettingsPtr pNewRow = m_pStepList->GetRow(-1);
		pNewRow->PutValue(escID, (long)-2);
		pNewRow->PutValue(escName, "<Ladder Begin Date>");
		m_pStepList->InsertRow(pNewRow, 0);
		m_pStepList->SetSelByColumn(escID, (long)-2);

		//refesh the chart
		ResetGraph(true, "", true);	//(a.wilson 2011-10-7) PLID 38789
	} NxCatchAll("Error IN: CMarketRetentionGraphDlg::OnSelChosenRetentionLadderList");
}


// (j.gruber 2008-05-28 12:39) - PLID 27440  - fixed the drop down lists so that you can't unselect them
void CMarketRetentionGraphDlg::OnSelChangingRetentionLadderList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CMarketRetentionGraphDlg::OnSelChangingRetentionLadderList");
	
}

void CMarketRetentionGraphDlg::OnSelChangingRetentionPurposeList(long FAR* nNewSel) 
{
	try {
		if (*nNewSel == sriNoRow) {
			*nNewSel = m_pPurposeList->GetCurSel();
		}
	}NxCatchAll("Error in CMarketRetentionGraphDlg::OnSelChangingRetentionPurposeList");
	
}

void CMarketRetentionGraphDlg::OnSelChangingRetentionProcedureList(long FAR* nNewSel) 
{
	try {
		if (*nNewSel == sriNoRow) {
			*nNewSel = m_pProcList->GetCurSel();
		}
	}NxCatchAll("Error in CMarketRetentionGraphDlg::OnSelChangingRetentionProcedureList");
	
}

void CMarketRetentionGraphDlg::OnSelChangingRetentionStepList(long FAR* nNewSel) 
{
	try {
		if (*nNewSel == sriNoRow) {
			*nNewSel = m_pStepList->GetCurSel();
		}
	}NxCatchAll("Error in CMarketRetentionGraphDlg::OnSelChangingRetentionStepList");
	
}


void CMarketRetentionGraphDlg::OnBnClickedEnableExcludeUnretainedRange()
{
	// (b.spivey, November 04, 2011) - PLID 46267 -If we enable it, go ahead and move focus.
	try{
		ResetGraph(true, "", true);	
		if(m_checkExcludeUnretained.GetCheck()){
			m_nxeditExcludeUnretainedRange.EnableWindow(TRUE); 
			m_nxeditExcludeUnretainedRange.SetFocus(); 
		}
		else{
			m_nxeditExcludeUnretainedRange.EnableWindow(FALSE); 
		}
	}NxCatchAll(__FUNCTION__);
}

void CMarketRetentionGraphDlg::OnEnKillfocusExcludeUnretainedRange()
{
	// (b.spivey, November 04, 2011) - PLID 46267 -If we have no length, it's empty. Set to zero for safety. 
	try{
		if(m_nxeditExcludeUnretainedRange.GetWindowTextLength() <= 0) {
			ResetGraph(true, "", true);	
			m_nxeditExcludeUnretainedRange.SetWindowText("0"); 
			m_nExcludeUnretainedRange = 0; 
		}
		else{
			// (b.spivey, November 09, 2011) - PLID 46267 - If we're before 1753, set focus back to the edit control. 
			if (!CheckValidExcludeDateRange()){
				MsgBox(MB_ICONWARNING | MB_OK, "Please enter an amount of months that does not result in a range before January 1st, 1753."); 
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CMarketRetentionGraphDlg::OnEnChangeExcludeUnretainedRange()
{
	// (b.spivey, November 04, 2011) - PLID 46267 - If we change it, go ahead and force a refresh (if not already refreshed!)
	try{
		if(m_wndPieGraph.IsWindowVisible()){
			ResetGraph(true, "", true); 
		}
	}NxCatchAll(__FUNCTION__);
}

void CMarketRetentionGraphDlg::SelChangingMergeGroupOptions(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	// (b.spivey, November 03, 2011) - PLID 38861 - Implemented to prevent nulls. 
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, November 10, 2011) - PLID 46267 - Safest way to ensure we don't get an invalid range is to check every time we break focus. 
void CMarketRetentionGraphDlg::OnNMKillfocusPivotDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	// (b.spivey, November 10, 2011) - PLID 46267 - If we're using exclude unretained, then we need to check for a range 
	//		that goes before 1753. If true, set focus back to the edit box. 
	if(m_checkExcludeUnretained.GetCheck()){
		if (!CheckValidExcludeDateRange()){
			MsgBox(MB_ICONWARNING | MB_OK, "Please enter an amount of months that does not result in a range before January 1st, 1753.");  
		}
	}
}

// (b.spivey, November 10, 2011) - PLID 46267 - Added this function for maintainability. 
bool CMarketRetentionGraphDlg::CheckValidExcludeDateRange()
{
	COleDateTime dtPivotDate = m_dtPivot.GetValue();
	CString strExcludeUnretained;
	m_nxeditExcludeUnretainedRange.GetWindowText(strExcludeUnretained);
	int nExcludeUnretainedRange = atoi(strExcludeUnretained); 
		
	//If we have remaining months we'll need them later. 
	int nExcludeMonths = nExcludeUnretainedRange % 12; 
	//Explicitly cast, or you might end up with a case where the compiler thinks 2010 is less than 1753
	int nExcludeYears = (int)(nExcludeUnretainedRange / 12.0);
	//If we have more months than the pivot date, that's an extra year. 
	if(nExcludeMonths > dtPivotDate.GetMonth())
	{
		nExcludeYears++; 
	}
	//The number of years back we're going to go. 
	int nTargetYear = dtPivotDate.GetYear() - nExcludeYears; 
	int nMinimalYear = g_cdtSqlMin.GetYear(); //Min year is 1753 
	if (nTargetYear < nMinimalYear){
		// (b.spivey, November 10, 2011) - PLID 46267 - An invalid number couldn't have gotten in there before, so reset it if 
		//   the current and previous are not equal. 
		if (m_nExcludeUnretainedRange != nExcludeUnretainedRange) {
			CString strUnretainedRange;
			strUnretainedRange.Format("%li", m_nExcludeUnretainedRange); 
			m_nxeditExcludeUnretainedRange.SetWindowText(strUnretainedRange);
		}
		//Else, set it to zero. We can't really trust anything else. 
		else {
			m_nExcludeUnretainedRange = 0; 
			m_nxeditExcludeUnretainedRange.SetWindowText("0");
		}
		return false; //not valid
	}
	else{
		//If valid, go ahead and update m_nExcludeUnretainedRange. 
		m_nExcludeUnretainedRange = nExcludeUnretainedRange;
		return true; //valid
	}
}