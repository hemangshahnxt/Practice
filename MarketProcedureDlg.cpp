
#include "stdafx.h"

// Effectiveness.cpp : implementation file
//
#include "MarketProcedureDlg.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "MsgBox.h"
#include "InternationalUtils.h"
#include "MarketUtils.h"
#include "MarketFilterPickerDlg.h"
#include "DocBar.h"
#include "MarketGraphDlg.h"
#include "MarketingRc.h"
#include "ConversionRateByDateConfigDlg.h"

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
using namespace ColumnGraph;
using namespace SmallSTDOLE2Lib;
using namespace ADODB;

enum ProcedureListColumns {
	plcID = 0,
	plcName = 1,
};

/////////////////////////////////////////////////////////////////////////////
// CMarketProcedureDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketProcedureDlg::CMarketProcedureDlg(CWnd* pParent)
	: CMarketingDlg(CMarketProcedureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketProcedureDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/procedure.htm";

	SetFilter(mfFirstContactDate, mftDate);
}

CMarketProcedureDlg::~CMarketProcedureDlg()
{
	if (m_oldCursor)
		SetCursor(m_oldCursor);
	DestroyCursor(m_cursor);
}
// (j.gruber 2009-06-24 11:27) - PLID 34714 - added procedure filter
void CMarketProcedureDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketProcedureDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_UP, m_Up);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress);
	DDX_Control(pDX, IDC_COLOR1, m_color1);
	DDX_Control(pDX, IDC_PROFIT_BY_PROC, m_ProfitByProcRad);
	DDX_Control(pDX, IDC_MONEY_BY_PROC, m_MoneyByProcRad);
	DDX_Control(pDX, IDC_INQ_TO_CONS_BY_PROC, m_InqToConsByProc);
	DDX_Control(pDX, IDC_PROS_TO_CONS_BY_PROC, m_ProsToConsByProc);
	DDX_Control(pDX, IDC_PROS_TO_SURG_BY_PROC, m_ProsToSurgByProc);
	DDX_Control(pDX, IDC_CANCEL_BY_PROC, m_CancelByProc);
	DDX_Control(pDX, IDC_INQ_BY_PROC, m_InqByProc);
	DDX_Control(pDX, IDC_NO_SHOW_BY_PROC, m_NoShowByProc);
	DDX_Control(pDX, IDC_PROCS_PERF_BY_PROC, m_ProcsPerformed);
	DDX_Control(pDX, IDC_MONEY_BY_CATEGORY, m_MoneyByCategory);
	DDX_Control(pDX, IDC_PROCS_CLOSED2, m_ProcsClosed);	
	DDX_Control(pDX, IDC_PATIENTS, m_Patients);	
	DDX_Control(pDX, IDC_SHOW_INQUIRIES, m_ShowInquiries);	
	DDX_Control(pDX, IDC_CONS_TO_SURG, m_ConsToSurg);	
	DDX_Control(pDX, IDC_MARKET_PROC_LINK, m_nxlProcedureLabel);
	DDX_Control(pDX, IDC_SHOW_ALL_COLUMNS_PROC, m_ShowAllRad);
	DDX_Control(pDX, IDC_SHOW_NUMBERS_ONLY_PROC, m_ShowNumberRad);
	DDX_Control(pDX, IDC_SHOW_PERCENTAGES_ONLY_PROC, m_ShowPercentRad);
	//}}AFX_DATA_MAP
}

// (j.gruber 2009-06-24 11:27) - PLID 34714 - added procedure filter
BEGIN_MESSAGE_MAP(CMarketProcedureDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketProcedureDlg)
	ON_BN_CLICKED(IDC_MONEY_BY_PROC, OnMoneyByProc)
	ON_BN_CLICKED(IDC_PROFIT_BY_PROC, OnProfitByProc)
	ON_BN_CLICKED(IDC_INQ_TO_CONS_BY_PROC, OnInqToConsByProc)
	ON_BN_CLICKED(IDC_PROS_TO_CONS_BY_PROC, OnProsToConsByProc)
	ON_BN_CLICKED(IDC_PROS_TO_SURG_BY_PROC, OnProsToSurgByProc)
	ON_BN_CLICKED(IDC_PATS_NO_SHOW_BY_PROC, OnPatsNoShowByProc)
	ON_BN_CLICKED(IDC_CANCEL_BY_PROC, OnCancelByProc)
	ON_BN_CLICKED(IDC_INQ_BY_PROC, OnInqByProc)
	ON_BN_CLICKED(IDC_PROCS_PERF_BY_PROC, OnProcsPerformed)
	ON_BN_CLICKED(IDC_NO_SHOW_BY_PROC, OnNoShowByProc)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_PROCS_CLOSED2, OnProcsClosed)
	ON_BN_CLICKED(IDC_PATIENTS, OnPatients)
	ON_BN_CLICKED(IDC_SHOW_INQUIRIES, OnShowInquiries)
	ON_BN_CLICKED(IDC_CONS_TO_SURG, OnConsToSurg)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_BN_CLICKED(IDC_MONEY_BY_CATEGORY, OnMoneyByCategory)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_SHOWWINDOW()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)	
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TYPES_PROC, OnConfigureApptTypes)
	ON_BN_CLICKED(IDC_SHOW_ALL_COLUMNS_PROC, OnShowAllColumns)
	ON_BN_CLICKED(IDC_SHOW_NUMBERS_ONLY_PROC, OnShowNumbersOnly)
	ON_BN_CLICKED(IDC_SHOW_PERCENTAGES_ONLY_PROC, OnShowPercentagesOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketProcedureDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketProcedureDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketProcedureDlg)
	ON_EVENT(CMarketProcedureDlg, IDC_EFF_GRAPH, 1 /* OnClickColumn */, OnOnClickColumnGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketProcedureDlg, IDC_EFF_GRAPH, 2 /* OnMouseMoveColumn */, OnOnMouseMoveColumnGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketProcedureDlg, IDC_EFF_GRAPH, 3 /* OnChangeBackButtonPos */, OnChangeBackButtonPosColumnGraph, VTS_NONE)	
	ON_EVENT(CMarketProcedureDlg, IDC_MARKET_PROC_FILTER, 18, CMarketProcedureDlg::OnRequeryFinishedProcedures, VTS_I2)
	ON_EVENT(CMarketProcedureDlg, IDC_MARKET_PROC_FILTER, 16, CMarketProcedureDlg::OnSelChosenProcedureList, VTS_DISPATCH)
	ON_EVENT(CMarketProcedureDlg, IDC_MARKET_PROC_FILTER, 1 /* SelChanging */, CMarketProcedureDlg::OnSelChangingProcFilter, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CMarketProcedureDlg::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();
	CString			parent,
					item;

	//set from date back
	COleDateTime dt = COleDateTime::GetCurrentTime();
	dt.SetDate (dt.GetYear() - 1, dt.GetMonth(), dt.GetDay());

	m_graph = GetDlgItem(IDC_EFF_GRAPH)->GetControlUnknown();
	m_graph->Background = 0xFDFDFD;

	// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
	ColumnGraph::FontPtr font;
	font = m_graph->Font;
	font->PutName("Arial Narrow");
	font->PutSize(COleCurrency(13, 0));

	m_cursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_EXPAND));
	m_Up.AutoSet(NXB_LEFT);
	m_Up.EnableWindow(FALSE);
	m_oldCursor = NULL;
	m_bRenderedOnce = false; // a.walling PLID 20695 prevent render in updateview unless rendered once before
	m_bActive = false; // will be set to true as soon as OnShowWindow is handled, and false when switching to a different tab.
	m_bGraphEmpty = true;

	m_Go.AutoSet(NXB_MARKET);
	

	// (v.maida 2015-03-10 16:24) - PLID 63215 - Refunds can't be applied to procedures, so the portion of the tooltip text that mentioned refunds has been removed.
	m_MoneyByProcRad.SetToolTip("Display the net amount of applied payments generated by procedure. Includes detailed procedures' information with that procedure's master procedure.");
	m_MoneyByCategory.SetToolTip("Display the net amount of applied payments generated by category.");
	m_Patients.SetToolTip("Display the number of patients, prospects, and inquiries who have had the procedure or are interested in the procedure.");
	m_NoShowByProc.SetToolTip("Display the total number of appointment cancellations and No Shows scheduled.");
	m_InqToConsByProc.SetToolTip("Display the percentage of all inquiries that have progressed into consultations. The consultations include both patients and prospects.");
	m_ProsToConsByProc.SetToolTip("Display the percentage of all prospects who have scheduled consultations.");
	m_ProcsClosed.SetToolTip("Display the total number of procedures performed versus the number of procedures that are scheduled and have a prepayment associated with the appointment via the Procedure Information Center.");
	m_ConsToSurg.SetToolTip("Display the total number of scheduled consultations, procedures, and the percentage of consultations that progressed into procedures.");

	m_strCategory = "RootNode";

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - added procedure filter
	//(e.lally 2009-09-28) PLID 35521 - Preference for detailed or master procedure list
	m_pProcedureList = BindNxDataList2Ctrl(IDC_MARKET_PROC_FILTER, false);
	BOOL bUseDetailedProcList = GetRemotePropertyInt("MarketingUseDetailedProcedureFilter", 0, 0, GetCurrentUserName());
	CString strProcFilterWhere = "";
	if(bUseDetailedProcList == FALSE){
		strProcFilterWhere = "MasterProcedureID IS NULL";
	}
	m_pProcedureList->WhereClause = _bstr_t(strProcFilterWhere);
	m_pProcedureList->Requery();
	

	m_MoneyByProcRad.SetCheck(1);
	OnMoneyByProc(); // a.walling PLID 20695 5/18/06 Set up dialog for this graph but don't run it yet. The function
	// that runs the graphs is now MoneyByProc.. ie without the On. The On function should only update the GUI.

	m_graph->Title = "";

	// (b.cardillo 2004-08-05 10:35) - PLID 13747 - We used to call OnMoneyByProc() here, which sort of makes sense 
	// since we just set that radio button and we want to reflect the graph that the radio button indicates, BUT from 
	// what I can tell, the only time we get the OnInitDialog is when we're being initially created by the marketing 
	// module, and that only happens when the user clicks on the Procedures tab of marketing, and that automatically 
	// calls our UpdateView after we've been created, and our UpdateView checks to see which radio button is selected 
	// and it runs that graph.  So that graph was being run TWICE when the user would first go to this tab.  That's 
	// why I took out the call to OnMoneyByProc(), but we need to test to make sure this doesn't cause a problem.
		
	return FALSE;
}

void CMarketProcedureDlg::OnMoneyByProc() 
{
	if (LastChecked(IDC_MONEY_BY_PROC)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}
		
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	m_Up.ShowWindow(FALSE);
	m_Up.EnableWindow(FALSE);


	if(GetMainFrame()->m_pDocToolBar->GetType() != EFFMonByProc) {
		SetType(EFFMonByProc);
		SetFilter(mfPaymentDate, mftDate);
		SetFilter(mfChargeLocation, mftLocation);
		SetFilter(mfChargeProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(0);

	ResetGraph();
}

void CMarketProcedureDlg::MoneyByProc() 
{
	
	CWaitCursor wc;
	m_graph->PrintTitle = "Revenue By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	
	m_Up.ShowWindow(FALSE);
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
//	if (IsDlgButtonChecked(IDC_INCLUDE_TAX)) {
		
		GraphByProcedure(EFFMonByProc, "Revenue", "SumMoney");
//	}
//	else {
			
//		GraphByProcedure(EFFMonByProc, "Revenue", "SumNoTax");
//	}	
}


// TODO Does this do anything??
void CMarketProcedureDlg::OnProfitByProc()
{
	if (LastChecked(IDC_PROFIT_BY_PROC)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);

	m_Up.ShowWindow(FALSE);
	m_Up.EnableWindow(FALSE);
	m_graph->PrintTitle = "Profit By Procedure";
	ResetGraph();
}

void CMarketProcedureDlg::ProfitByProc() 
{	

}



/*void CMarketProcedureDlg::OnIncludeTax() 
{
	UpdateView();
	
}*/



void CMarketProcedureDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
	try {
		//(e.lally 2009-09-28) PLID 35521 - Preference for detailed or master procedure list. Check if our where clause does not match up.
		BOOL bUseDetailedProcList = GetRemotePropertyInt("MarketingUseDetailedProcedureFilter", 0, 0, GetCurrentUserName());
		CString strProcFilterWhere = AsString(m_pProcedureList->GetWhereClause());
		bool bSkipRendering = false;
		//(e.lally 2009-12-07) PLID 35521 - If we are requerying the procedure list (previous selections are unsaved right now)
			//we need to stop the graph rendering so the user can reselect their filters.
		if(bUseDetailedProcList == FALSE && strProcFilterWhere.Find("MasterProcedureID IS NULL") == -1){
			ResetGraph(true, "", true);
			m_bRenderedOnce = false;
			bSkipRendering = true;
			strProcFilterWhere = "MasterProcedureID IS NULL";
			m_pProcedureList->WhereClause = _bstr_t(strProcFilterWhere);
			m_pProcedureList->Requery();
		}
		else if(bUseDetailedProcList != FALSE && strProcFilterWhere.Find("MasterProcedureID IS NULL") != -1){
			ResetGraph(true, "", true);
			m_bRenderedOnce = false;
			bSkipRendering = true;
			strProcFilterWhere = "";
			m_pProcedureList->WhereClause = _bstr_t(strProcFilterWhere);
			m_pProcedureList->Requery();
		}
		
		
		//(a.wilson 2011-10-5) PLID 38789 - added force refresh to prevent refreshing when coming back to module.
		if ((m_bRenderedOnce || (m_bActive && bForceRefresh)) && bSkipRendering==false) {
			CMarketRenderButtonStatus mrbs(this);

			if (m_bActive && !m_bRenderedOnce) // refresh from toolbar or switching to module
			{
				// (j.gruber 2006-12-05 11:38) - PLID 22889 - don't update if we are in a report window
				CNxTabView *pView = GetMainFrame()->GetActiveView();
				if (pView) {
					pView->RedrawWindow();
				}
				else {

					//if we aren't in the view, we don't need to refresh
					return;
				}
			}

			m_bRenderedOnce = false;
			m_bGraphEmpty = false;

			if (m_ProfitByProcRad.GetCheck()) {
				ProfitByProc();
			}
			else if (m_MoneyByProcRad.GetCheck()) {
				MoneyByProc();
			}
			else if (m_InqToConsByProc.GetCheck()) {
				InqToConsByProc();
			}
			else if (m_ProsToConsByProc.GetCheck()) {
				ProsToConsByProc();
			}
			else if (m_ProsToSurgByProc.GetCheck()) {
				ProsToSurgByProc();
			}
			else if (m_CancelByProc.GetCheck()) {
				CancelByProc();
			}
			else if (m_InqByProc.GetCheck()) {
				InqByProc();
			}
			else if (m_NoShowByProc.GetCheck()) {
				NoShowByProc();
			}
			else if (m_ProcsPerformed.GetCheck()) {
				ProcsPerformed();
			}
			else if (m_ProcsClosed.GetCheck()) {
				ProcsClosed();
			}
			else if (m_Patients.GetCheck()) {
				Patients();
			}
			else if (m_ConsToSurg.GetCheck()) {
				ConsToSurg();
			}
			/*
			else if (m_InqToConsByProc.GetCheck()) {
				GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
				OnInqToConsByProc();
			}
			else if (m_ProsToConsByProc.GetCheck()) {
				GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
				OnProsToConsByProc();
			}
			*/ // a.walling PLID 20695 5/18/06 These are already done above...

			if (m_MoneyByCategory.GetCheck()) {
				MoneyByCategory();
				if (m_strCategory != "RootNode")
				{	m_graph->Title = (LPCSTR)m_strCategory;
					m_Up.EnableWindow(TRUE);
					m_Up.ShowWindow(TRUE);
					
				}
				else
				{	m_graph->Title = "All";
					m_Up.EnableWindow(FALSE);
					m_Up.ShowWindow(FALSE);
				}
			}
		}
		else { // set filters
			if (m_ProfitByProcRad.GetCheck()) {
				OnProfitByProc();
			}
			else if (m_MoneyByProcRad.GetCheck()) {
				OnMoneyByProc();
			}
			else if (m_InqToConsByProc.GetCheck()) {
				OnInqToConsByProc();
			}
			else if (m_ProsToConsByProc.GetCheck()) {
				OnProsToConsByProc();
			}
			else if (m_ProsToSurgByProc.GetCheck()) {
				OnProsToSurgByProc();
			}
			else if (m_CancelByProc.GetCheck()) {
				OnCancelByProc();
			}
			else if (m_InqByProc.GetCheck()) {
				OnInqByProc();
			}
			else if (m_NoShowByProc.GetCheck()) {
				OnNoShowByProc();
			}
			else if (m_ProcsPerformed.GetCheck()) {
				OnProcsPerformed();
			}
			else if (m_ProcsClosed.GetCheck()) {
				OnProcsClosed();
			}
			else if (m_Patients.GetCheck()) {
				OnPatients();
			}
			else if (m_ConsToSurg.GetCheck()) {
				OnConsToSurg();
			}

			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
		
		InvalidateConsToProcRadioButtons();
		// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
		InvalidateShowInquiriesButton();
		
	}NxCatchAll("Error Updating View");
}


void CMarketProcedureDlg::OnOnMouseMoveColumnGraph(short Row, short Column) 
{
 	bool bIsRowClickable = false;
	m_graph->Row = Row;
	long nRowID = m_graph->RowID;
	for(int i = 0; i < m_arClickableRows.GetSize(); i++) if(m_arClickableRows.GetAt(i) == nRowID) bIsRowClickable = true;
	
	if (bIsRowClickable)
	{	if (!m_oldCursor)
			m_oldCursor = SetCursor(m_cursor);
		else SetCursor(m_cursor);
	}
	else if (m_oldCursor)
		SetCursor(m_oldCursor);
	
}

void CMarketProcedureDlg::GraphByProcedure(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2 /*=""*/, CString strField2 /*= ""*/, CString strDesc3 /*=""*/, CString strField3 /*=""*/) {

	try {
	
		GraphDescript desc;
		CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilter, PatFilterIDs, strCategory, strResp, strLocationField, strProvField;
		int nCategory, nResp;
		CString referrals;

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//setup parameters that will be used in the query
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
		if (UseFilter(mftProvider))  {
			
			if (HasMultiFilter(strProvField)) {
				CString strFilter1, strFilter2;
				ParseMultiFilter(strProvField, strFilter1, strFilter2);
			
				//fill these, then we will clear them if we need to 
				strProvFilter1.Format(" AND %s %s) ", strFilter1, provIDs);
				strProvFilter2.Format(" AND %s %s) ", strFilter2, provIDs);

				if (strFilter1.IsEmpty()) {
					strProvFilter1 = "";
				}
					
				if (strFilter2.IsEmpty()) {
					strProvFilter2 = "";
				}
			}
			else {
				strProvFilter1.Format(" AND %s %s) ", strProvField, provIDs);
			}
		}
		
		
		if (UseFilter(mftLocation) ) {
			if (HasMultiFilter(strLocationField) ) {
				
				CString strFilter1, strFilter2;
				ParseMultiFilter(strLocationField, strFilter1, strFilter2);
		
				//fill these, then we will clear them if we need to 
				strLocFilter1.Format(" AND %s IN %s ", strFilter1, locIDs);
				strLocFilter2.Format(" AND %s IN %s ", strFilter2, locIDs);

				if (strFilter1.IsEmpty()) {
					strLocFilter1 = "";
				}
				
				if (strFilter2.IsEmpty()) {
					strLocFilter2 = "";
				}
			}
			else {
				strLocFilter1.Format(" AND %s IN %s ", strLocationField, locIDs);
			}
		}
		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);
		if (nCategory!= -1) 
			strCategory.Format(" AND ServiceT.Category = %i ", nCategory);
		
		if (nResp == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
		}
		else if (nResp == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
		}

		CString strFrom, strTo;
		if (!strDateField.IsEmpty()) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		
		sql = GetGraphSql(mktType, 1, -1, pCon, strPatientTempTable);
		
		sql.Replace("[From]", strFrom);
		sql.Replace("[To]", strTo);
		sql.Replace("[Prov1]", strProvFilter1);
		sql.Replace("[Prov2]", strProvFilter2);
		sql.Replace("[Loc1]", strLocFilter1);
		sql.Replace("[Loc2]", strLocFilter2);
		sql.Replace("[PatFilter]", strPatFilter);
		sql.Replace("[Category]", strCategory);
		sql.Replace("[Resp]", strResp);

		//MessageBox(sql);

		//set the timeout
		CIncreaseCommandTimeout ict(600);

		//calculate the summary
		CString strTempDesc1, strTempDesc2, strTempDesc3;
		if (strDesc3.IsEmpty()) {
			if (strDesc2.IsEmpty()) {
				
				strTempDesc1.Format("%s", strDescription);
				//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
				desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue), "", GraphDescript::GD_ADD);
			}
			else {

				strTempDesc1.Format("%s", strDescription);
				strTempDesc2.Format("%s", strDesc2);
				//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
				desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue), "", GraphDescript::GD_ADD);
				desc.Add(strTempDesc2, strField2, strDesc2 == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightRed), "", GraphDescript::GD_ADD);
			}


		}
		else {
			strTempDesc1.Format("%s", strDescription);
			strTempDesc2.Format("%s", strDesc2);
			strTempDesc3.Format("%s", strDesc3);
			//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
			desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue), "", GraphDescript::GD_ADD);
			desc.Add(strTempDesc2, strField2, strDesc2 == "Revenue" ?RGB(43,200,15) : strDesc2 == "Inquiries" ?RGB(255,0,255) : RGB(255,0,0), "", GraphDescript::GD_ADD);
			desc.Add(strTempDesc3, strField3, strDesc3 == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightRed), "", GraphDescript::GD_ADD);
			
		}


		desc.m_sql = sql;
		Save(sql);
		m_strSql = sql;

		if(mktType == EFFMonByProc || mktType == EFFMonByCategory)
			m_graph->Format = _bstr_t(GetCurrencySymbol());
		else
			m_graph->Format = "%0.0f";	

		GraphProcedures(desc);

		for (int i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			if (GetType() == EFFMonByProc) {
				// (b.spivey, January 24, 2012) - PLID 47757 - CString is not internationally friendly, so we take the double and 
				//	we turn it into a currency, then round just to be safe. 
				COleCurrency cy;
				cy = AsCurrency(desc.dblTotal(i)); 
				RoundCurrency(cy);
				// (z.manning 2008-06-18 16:21) - PLID 28601 - Hide decimal places on marketing graphs.
				strText.Format("%s (%s Total)", desc.Label(i), FormatCurrencyForInterface(cy, TRUE, TRUE, FALSE));
			}
			else {
				strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
			}
			m_graph->ColumnText = (_bstr_t)strText;
			m_graph->ColumnFormat = m_graph->Format;
		}
		

		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error Graphing By Procedure");

}

void CMarketProcedureDlg::GraphProcedures(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsPurpose = NULL,
						rs = NULL;
		int				currentID, id = 0, i = 0, 
						j,
						max;
		double			total, total2;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		m_progress.SetPos(0);

		// (j.gruber 2009-06-24 11:49) - PLID 34714 - filter on only the procedures they want to see
		CString strFilter;
		if (m_dwProcIDList.GetSize() > 1) {
			strFilter.Format(" AND (ProcedureT.ID IN %s) ", m_strProcedureList, m_strProcedureList);			
		}
		else if (m_nCurrentProcedureID != -1) {
			strFilter.Format(" AND (ProcedureT.ID = %li) ", m_nCurrentProcedureID);
		}
		else {			
			strFilter = "";
		}

		sql.Format ("SELECT AptPurposeT.Name AS Name, "
			"AptPurposeT.ID as PurposeID FROM AptPurposeT INNER JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID "
			"WHERE ProcedureT.MasterProcedureID Is Null %s "
			"ORDER BY Name;", strFilter);

		rsPurpose = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsPurpose->RecordCount;
		m_graph->RowCount = max;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(25);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			if(desc.Label(i).Find("Percentage") != -1 || desc.Label(i).Find("Conversion Rate") != -1)
				m_graph->ColumnFormat = "%0.0f%%";
			else
				m_graph->ColumnFormat = m_graph->Format;
		}

		m_graph->XAxisDesc = "Procedure";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		// (b.cardillo 2004-08-06 09:17) - PLID 13747 - Before the loop, scan the recordset into 
		// our map of values, so that we can access it quickly inside the loop.
		CMapLongToGraphValuesArray mapPurposeGraphValues;
		mapPurposeGraphValues.ScanRecordsetIntoMap(rs, "PurposeID", -1, desc);

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		i = 0;
		while (!rsPurpose->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsPurpose->Fields->GetItem("Name")->Value);
			m_graph->RowID = AdoFldLong(rsPurpose, "PurposeID");

			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = rsPurpose->Fields->GetItem("PurposeID")->Value.lVal;
			descendants.Add(currentID);

			//Set rowList so we know if there is a descendant
			if (descendants.GetSize() > 1) {
				m_arClickableRows.Add(currentID);
				m_graph->RowDrillDown = TRUE;
			}
			else {
				m_graph->RowDrillDown = FALSE;
			}

			//for each column
			for (j = 0; j < desc.Size(); j++)
			{	m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				// (b.cardillo 2004-08-06 09:19) - PLID 13747 - We used to loop through the recordset here, 
				// adding up the values from Field and Field2 in the recordset wherever the recordset's 
				// PurposeID was currentID.  Now we do the same thing but we use our super-fast map that we 
				// loaded before the loop instead of the recordset over and over.
				if (!rs->eof) {
					// Create the object that will do the processing
					CED_GraphCalcColumnTotals_Info gbrsi(&mapPurposeGraphValues, desc, j, total, total2);
					// Call it for the root level
					gbrsi.ProcessGraphValues(currentID);
					// Procedures by definition can't have "descendents" so no need to enumerate like 
					// we do in the referrals graphs.
					// Now return the totals values to our local variables
					total = gbrsi.m_dblTotal;
					total2 = gbrsi.m_dblTotal2;
				}

				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						m_graph->Value = total;
						break;
					case GraphDescript::GD_DIV:
					//(e.lally 2009-09-16) PLID 35559 - Added support for percents
					case GraphDescript::GD_PERCENT:
						if (GetType() == CNVInqToConsByProc || GetType() == CNVProsToConsByProc) {
							//only add to the total if they both aren't zero because otherwise the totals won't be right because the procedures don't match up
							if (total != 0 && total2 != 0) {
								desc.AddToTotal(j, total);
								desc.AddToTotal2(j, total2);
							}
						}
						else {
							desc.AddToTotal(j, total);
							desc.AddToTotal2(j, total2);
						}						
						if (total2 != 0) {
							double dblResult = total / total2;
							//(e.lally 2009-09-16) PLID 35559 - percent mode expects 1/2 to be 50, not 0.5 
								//so multiple the result by 100
							if(desc.Op(j) == GraphDescript::GD_PERCENT){
								dblResult = dblResult * 100.00;
							}
							m_graph->Value = dblResult;
						}
						else m_graph->Value = 0;
						break;
					default:
						ASSERT(FALSE);//things I haven't handeled yet, or bad values
				}
			}
			rsPurpose->MoveNext();
			m_progress.SetPos(50 + ((50 * i) / max));
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}

void CMarketProcedureDlg::OnInqToConsByProc() 
{
	if (LastChecked(IDC_INQ_TO_CONS_BY_PROC)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}
	
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != CNVInqToConsByProc) {
		SetType(CNVInqToConsByProc);
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();

}

void CMarketProcedureDlg::InqToConsByProc() 
{
	
	CWaitCursor wc;

	m_graph->PrintTitle = "Inquiries to Consults By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	GraphDescript desc;
	CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilter, PatFilterIDs, strLocationField, strProvField;
	int nCategoryID, nResp;
	CString referrals;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

	if (UseFilter(mftProvider))  {
			
		if (HasMultiFilter(strProvField)) {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strProvField, strFilter1, strFilter2);
		
			//fill these, then we will clear them if we need to 
			strProvFilter1.Format(" AND %s %s) ", strFilter1, provIDs);
			strProvFilter2.Format(" AND %s %s) ", strFilter2, provIDs);

			if (strFilter1.IsEmpty()) {
				strProvFilter1 = "";
			}
				
			if (strFilter2.IsEmpty()) {
				strProvFilter2 = "";
			}
		}
		else {
			strProvFilter1.Format(" AND %s %s) ", strProvField, provIDs);
		}
	}

	

	if (UseFilter(mftLocation)) {
		CString strFilter1, strFilter2;
			ParseMultiFilter(strLocationField, strFilter1, strFilter2);
	
			//fill these, then we will clear them if we need to 
			strLocFilter1.Format(" AND %s IN %s ", strFilter1, locIDs);
			strLocFilter2.Format(" AND %s IN %s ", strFilter2, locIDs);

			if (strFilter1.IsEmpty()) {
				strLocFilter1 = "";
			}
			
			if (strFilter2.IsEmpty()) {
				strLocFilter2 = "";
			}
	}

	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


	sql = GetGraphSql(CNVInqToConsByProc, -1, -1, pCon, strPatientTempTable);

	CString strFrom, strTo;
	if (!strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	
	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov1]", strProvFilter1);
	sql.Replace("[Prov2]", strProvFilter2);
	sql.Replace("[Loc1]", strLocFilter1);
	sql.Replace("[Loc2]", strLocFilter2);
	sql.Replace("[PatFilter]", strPatFilter);

	desc.m_sql = sql;
	Save(sql);
	m_strSql = sql;

	/*//set the timeout
	long nOldTimeout;
	nOldTimeout = g_ptrRemoteData->CommandTimeout;

	if (nOldTimeout < 600) {
		g_ptrRemoteData->CommandTimeout = 600;
	}

	_RecordsetPtr rsTotal = CreateRecordset("SELECT CASE WHEN Sum(NumInquiries) = 0 THEN 0 ELSE 100 * (Sum(ConsCount)/Sum(NumInquiries)) END AS Total1 FROM (%s) BASE ", sql);

	
	CString strDesc;
	strDesc.Format("Percentage (%s%% Total) ", AsString(rsTotal->Fields->Item["Total1"]->Value).IsEmpty() ? "0": AsString(rsTotal->Fields->Item["Total1"]->Value));
	*/

	CString strDesc;
	strDesc.Format("Percentage");
	
	//desc.Add("Inquiries", "NumInquiries", RGB(255,0,0), "", GraphDescript::GD_ADD);
	//desc.Add("Consults", "ConsCount", RGB(0,255,0), "", GraphDescript::GD_ADD);
	//JMJ 4/8/2004 - when graphing, we need to change the Format for the percentage column,
	//right now it triggers on the word "Conversion Rate", so change that if you change this name
	//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
	desc.Add(strDesc, "ConsCount", GetMarketGraphColor(mgcBrightBlue), "NumInquiries", GraphDescript::GD_DIV);
				
	m_graph->Format = "%0.0f%%";
	GraphProcedures(desc);

	for (int i = 0; i < desc.Size(); i++)
	{	m_graph->Column = i;
		m_graph->Color = desc.Color(i);
		CString strText;
		if (desc.dblTotal2(i) == 0) {
			strText.Format("%s (%s%% Total)", desc.Label(i), "0");
		}
		else {
			double dbl = desc.dblTotal(i)/desc.dblTotal2(i);
			strText.Format("%s (%.0f%% Total)", desc.Label(i), dbl);
		}
		m_graph->ColumnText = (_bstr_t)strText;
		m_graph->ColumnFormat = m_graph->Format;
	}

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
	
}

void CMarketProcedureDlg::OnProsToConsByProc() 
{
	if (LastChecked(IDC_PROS_TO_CONS_BY_PROC)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}
	
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != CNVProsToConsByProc) {
		SetType(CNVProsToConsByProc);
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketProcedureDlg::ProsToConsByProc() 
{
	
	CWaitCursor wc;

	try {
		m_graph->PrintTitle = "Prospects To Consults By Procedure";
		m_Up.ShowWindow(FALSE);
		m_strCategory = "RootNode";
		m_graph->Title = "";

		GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

		GraphDescript desc;
		CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilter, PatFilterIDs, strLocationField, strProvField;
		int nCategoryID, nResp;
		CString referrals;

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//setup parameters that will be used in the query
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		if (UseFilter(mftProvider))  {
			
			if (HasMultiFilter(strProvField)) {
				CString strFilter1, strFilter2;
				ParseMultiFilter(strProvField, strFilter1, strFilter2);
			
				//fill these, then we will clear them if we need to 
				strProvFilter1.Format(" AND %s %s) ", strFilter1, provIDs);
				strProvFilter2.Format(" AND %s %s) ", strFilter2, provIDs);

				if (strFilter1.IsEmpty()) {
					strProvFilter1 = "";
				}
					
				if (strFilter2.IsEmpty()) {
					strProvFilter2 = "";
				}
			}
			else {
				strProvFilter1.Format(" AND %s %s) ", strProvField, provIDs);
			}
		}


		if (UseFilter(mftLocation)) {
			CString strFilter1, strFilter2;
				ParseMultiFilter(strLocationField, strFilter1, strFilter2);
		
				//fill these, then we will clear them if we need to 
				strLocFilter1.Format(" AND %s IN %s ", strFilter1, locIDs);
				strLocFilter2.Format(" AND %s IN %s ", strFilter2, locIDs);

				if (strFilter1.IsEmpty()) {
					strLocFilter1 = "";
				}
				
				if (strFilter2.IsEmpty()) {
					strLocFilter2 = "";
				}
		}

		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		sql = GetGraphSql(CNVProsToConsByProc, -1, -1, pCon, strPatientTempTable);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty()) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		
		sql.Replace("[From]", strFrom);
		sql.Replace("[To]", strTo);
		sql.Replace("[Prov1]", strProvFilter1);
		sql.Replace("[Prov2]", strProvFilter2);
		sql.Replace("[Loc1]", strLocFilter1);
		sql.Replace("[Loc2]", strLocFilter2);
		sql.Replace("[PatFilter]", strPatFilter);

		desc.m_sql = sql;
		Save(sql);
		m_strSql = sql;

		CString strDesc;
		strDesc.Format("Percentage");

		//if (IsDlgButtonChecked(IDC_SHOW_ALL_PROSPECTS)) {
			//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
			desc.Add(strDesc, "Consults", GetMarketGraphColor(mgcBrightBlue), "ProspectCount", GraphDescript::GD_DIV);
		/*}
		else {
			desc.Add("Percentage", "Consults", RGB(0,0,255), "ConsProsCount", GraphDescript::GD_DIV);
		}*/

		//MessageBox(sql);

		
		m_graph->Format = "%0.0f%%";
		GraphProcedures(desc);	

		for (int i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			if (desc.dblTotal2(i) == 0) {
				strText.Format("%s (%s%% Total)", desc.Label(i), "0");
			}
			else {
				double dbl = desc.dblTotal(i)/desc.dblTotal2(i);
				strText.Format("%s (%.0f%% Total)", desc.Label(i), dbl);
			}
			m_graph->ColumnText = (_bstr_t)strText;
			m_graph->ColumnFormat = m_graph->Format;
		}

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error in OnProsToConsByProc");
}

// TODO does this do anything???
void CMarketProcedureDlg::OnProsToSurgByProc() 
{
	if (LastChecked(IDC_PROS_TO_SURG_BY_PROC)) return;
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	m_graph->PrintTitle = "Prospects To Surgery By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";

	ResetGraph();
}

void CMarketProcedureDlg::ProsToSurgByProc() 
{
	
}


// TODO Does this do anything??
void CMarketProcedureDlg::OnPatsNoShowByProc()
{
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMPatNoShowsByProc) {
		SetType(NUMPatNoShowsByProc);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

}

void CMarketProcedureDlg::PatsNoShowByProc()
{
	
	CWaitCursor wc;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";


	GraphByProcedure(NUMPatNoShowsByProc, "Patients With No Shows", "PatCount");
}

void CMarketProcedureDlg::OnConsToSurg()
{
	if (LastChecked(IDC_CONS_TO_SURG)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}

	ShowConversionRateControls(TRUE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != PROCConsultToSurg) {
		SetType(PROCConsultToSurg);
		SetFilter(mfNoPatApptLocation,mftLocation);
		SetFilter(mfNoPatApptProvider, mftProvider);
		SetFilter(mfConsultDate, mftDate);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

//(e.lally 2009-09-11) PLID 35521 - Used as the local function to be used to replace the consult procedure filter placeholder
	//in the sql graph query.
//Filters using master procedures.
//TODO: this should be replaced with a shared function.
CString CMarketProcedureDlg::GetConsToProcProcedureFilter()
{
	CString strOut;
	if(!m_strProcedureList.IsEmpty()){
		strOut.Format(" AND (ProcedureT.ID IN %s OR ProcedureT.MasterProcedureID IN %s ) ", m_strProcedureList, m_strProcedureList);
	}
	return strOut;
}

void CMarketProcedureDlg::ConsToSurg()
{
	//(e.lally 2009-08-24) PLID 35298 - Updated the procedure tab to use the new base query for the consult to procedure conversion rate.
	CWaitCursor wc;

	try {

		GraphDescript desc;

		m_graph->Format = "%0.0f";
		m_graph->PrintTitle = "Consults to Procedure By Procedure";
		m_Up.ShowWindow(FALSE);
		m_strCategory = "RootNode";
		m_graph->Title = "";

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//(e.lally 2009-08-25) PLID 35298 - Get the query for the conversion rate graph
			//The filters are already applied for us based on the docbar selections.
		CString strSql = GetGraphSql(PROCConsultToSurg, 1, -1, pCon, strPatientTempTable);
		CString strSummarySql = GetGraphSql(PROCConsultToSurg, 2, -1, pCon, strPatientTempTable);

		//(e.lally 2009-09-11) PLID 35521 - Ideally, all this would happen in the utils for us, but since the filter is only implemented
		//locally, we have to manually do it ourselves.
		CString strProcedureFilter = GetConsToProcProcedureFilter();
		strSql.Replace(PROCEDURE_FILTER_PLACEHOLDER, strProcedureFilter);
		strSummarySql.Replace(PROCEDURE_FILTER_PLACEHOLDER, strProcedureFilter);
		
		GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);

		desc.m_sql = strSql;
		Save(strSql);
		m_strSql = strSql;

		//set the timeout
		CIncreaseCommandTimeout ict(600);

		BOOL bShowNumbers = IsDlgButtonChecked(IDC_SHOW_NUMBERS_ONLY_PROC) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS_PROC);
		BOOL bShowPercentages = IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY_PROC) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS_PROC);

		if(!IsConsToProcSetupValid()) {
			MessageBox("Please set up your configuration settings by clicking the ... button before running this graph");
			return;
		}

		// (z.manning 2009-09-09 12:53) - PLID 35051 - Put the shared logic in a utility function
		AddConsToProcDataToGraphDesc(strSummarySql, &desc, bShowNumbers, bShowPercentages);
		
		GraphProcedures(desc);

		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error generating graph");
		
}

void CMarketProcedureDlg::OnCancelByProc()
{
	if (LastChecked(IDC_CANCEL_BY_PROC)) return;
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMCanByProc) {
		SetType(NUMCanByProc);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();

}

void CMarketProcedureDlg::CancelByProc()
{
	CWaitCursor wc;

	m_graph->PrintTitle = "Cancellations and No Shows By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	GraphByProcedure(NUMCanByProc, "Cancelled Appointments", "NumCancelled");
		
}

void CMarketProcedureDlg::OnInqByProc()
{
	if (LastChecked(IDC_INQ_BY_PROC)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMInqByProc) {
		SetType(NUMInqByProc);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketProcedureDlg::InqByProc()
{
	
	CWaitCursor wc;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);

	m_graph->PrintTitle = "Inquiries By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	
	//take the appt date out of the date filter
	GraphByProcedure(NUMInqByProc, "Inquires", "NumCount");
	
}

void CMarketProcedureDlg::OnNoShowByProc()
{
	if (LastChecked(IDC_NO_SHOW_BY_PROC)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}
	
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMNoShowByProc) {
		SetType(NUMNoShowByProc);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketProcedureDlg::NoShowByProc()
{	
	CWaitCursor wc;

	m_graph->PrintTitle = "Cancellations and No Shows By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	GraphByProcedure(NUMNoShowByProc, "No Shows", "NumNoShow", "Cancellations", "NumCancel");	
}


void CMarketProcedureDlg::OnProcsPerformed() 
{
	if (LastChecked(IDC_PROCS_PERF_BY_PROC)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}
	
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != DATECountByProcs) {
		SetType(DATECountByProcs);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}


void CMarketProcedureDlg::ProcsPerformed() 
{
	CWaitCursor wc;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);

	m_graph->PrintTitle = "Procedure Performed vs. Closed";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	GraphByProcedure(DATECountByProcs, "Procedures", "NumProcs");
}

void CMarketProcedureDlg::OnProcsClosed() 
{
	if (LastChecked(IDC_PROCS_CLOSED2)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}
	
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != PROCSchedVClosed) {
		SetType(PROCSchedVClosed);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketProcedureDlg::ProcsClosed() 
{
	
	CWaitCursor wc;

	m_graph->PrintTitle = "Procedures Performed vs. Closed";
	
	
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);

	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";

	GraphByProcedure(PROCSchedVClosed, "Performed", "ProcsPerformed", "Closed", "ProcsClosed");
}

void CMarketProcedureDlg::OnPatients() {
	if (LastChecked(IDC_PATIENTS)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	if (m_dwProcIDList.GetSize() > 1) {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	}

	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(TRUE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != PROCPatients) {
		SetType(PROCPatients);
		SetFilter(mfPatientLocation, mftLocation);
		SetFilter(mfPatientProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketProcedureDlg::Patients() {
	
	CWaitCursor wc;

	m_graph->PrintTitle = "Patients By Procedure";
	m_Up.ShowWindow(FALSE);
	m_strCategory = "RootNode";
	m_graph->Title = "";
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(TRUE);
	GetDlgItem(IDC_SHOW_INQUIRIES)->Invalidate ();

	if (IsDlgButtonChecked(IDC_SHOW_INQUIRIES)) {
		GraphByProcedure(PROCPatients, "Patients", "PatCount", "Inquiries", "InqCount", "Prospects", "ProsCount");	
	}
	else {
		GraphByProcedure(PROCPatients, "Patients", "PatCount", "Prospects", "ProsCount");	
	}

}

void CMarketProcedureDlg::OnShowInquiries() {
//	UpdateView(); a.walling PLID 20695 Allow user to choose when to render the graph
//  5/31/06 This checkbox doesn't radically change the graph, it just adds another column. Seems best to just set up the controls but not clear the graph.
	ResetGraph(true);
}





void CMarketProcedureDlg::GraphByCategory(MarketGraphType mktType, CString strDescription, CString strField) {

	
	GraphDescript desc;
	CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilter, PatFilterIDs, strCategory, strResp, strLocationField, strProvField;
	int id, nCategory, nResp;
	CString referrals;

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
	id = GetCurrentID();
	if (UseFilter(mftProvider))  {
			
		if (HasMultiFilter(strProvField)) {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strProvField, strFilter1, strFilter2);
		
			//fill these, then we will clear them if we need to 
			strProvFilter1.Format(" AND %s %s) ", strFilter1, provIDs);
			strProvFilter2.Format(" AND %s %s) ", strFilter2, provIDs);

			if (strFilter1.IsEmpty()) {
				strProvFilter1 = "";
			}
				
			if (strFilter2.IsEmpty()) {
				strProvFilter2 = "";
			}
		}
		else {
			strProvFilter1.Format(" AND %s %s) ", strProvField, provIDs);
		}
	}

	if (UseFilter(mftLocation)) {
		//fill these, then we will clear them if we need to 
		strLocFilter1.Format(" AND %s IN %s ", strLocationField, locIDs);
	}

	if (PatFilterIDs != "") 
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);
	if (nCategory != -1) 
		strCategory.Format(" AND ServiceT.Category = %i ", nCategory);
	if (nResp == 1)  {
		strResp.Format(" AND ChargesT.ID IN (SELECT ChargeID FROM ChargeRespT WHERE InsuredPartyID = -1 OR InsuredPartyID IS NULL) ");
	}
	else if (nResp == 2) {
		strResp.Format(" AND ChargesT.ID IN (SELECT ChargeID FROM ChargeRespT WHERE InsuredPartyID <> -1 AND InsuredPartyID IS NOT NULL) ");
	}

	id = GetCurrentID();


	CString strFrom, strTo;
	if (!strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	
	if (IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
		sql = GetGraphSql(mktType, 1, -1, pCon, strPatientTempTable);
	}
	else {
		sql = GetGraphSql(mktType, -1, -1, pCon, strPatientTempTable);
	}

	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov1]", strProvFilter1);
	sql.Replace("[Prov2]", strProvFilter2);
	sql.Replace("[Loc1]", strLocFilter1);
	sql.Replace("[PatFilter]", strPatFilter);
	sql.Replace("[Resp]", strResp);
	sql.Replace("[Category]", strCategory);
	// (b.cardillo 2004-08-06 09:22) - PLID 13747 - Made it so if the id is 0, which for categories is 
	// the sentinel value that indicates "all categories", we simply exclude NULL categories, which has 
	// the same effect as filtering on the set of all categories (which is what we used to do but it 
	// was slower, especially if you had a ton of categories).
	// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
	sql.Replace("[Cat]", CString("AND ") + (id == 0 ? "ServiceT.Category IS NOT NULL" : CategoryDescendants(id, "ServiceT.Category")));
	
	//MessageBox(sql);


	//set the timeout
	CIncreaseCommandTimeout ict(600);

	desc.m_sql = sql;
	Save(sql);
	m_strSql = sql;

	CString strTempDesc1;

	//rsTotal = CreateRecordset("SELECT CAST (CASE WHEN Sum(%s) = 0 THEN 0 ELSE Sum(%s) END AS MONEY) as Total1 FROM (%s) Base ", strField, strField, sql);
	strTempDesc1.Format("%s", strDescription);
	//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
	desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue), "", GraphDescript::GD_ADD);
		
	m_graph->Format = _bstr_t(GetCurrencySymbol());
	GraphCategory(desc);

	for (int i = 0; i < desc.Size(); i++)
	{	m_graph->Column = i;
		m_graph->Color = desc.Color(i);
		CString strText;
		if (GetType() == EFFMonByCategory) {
			COleCurrency cy;
			// (b.spivey, January 25, 2012) - PLID 47757 - Convert directly from a double and round for international clients. 
			cy = AsCurrency(desc.dblTotal(i));
			RoundCurrency(cy); 
			// (z.manning 2008-06-18 16:21) - PLID 28601 - Hide decimal places on marketing graphs.
			strText.Format("%s (%s Total)", desc.Label(i), FormatCurrencyForInterface(cy, TRUE, TRUE, FALSE));
		}
		else {
			strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
		}
		m_graph->ColumnText = (_bstr_t)strText;
		m_graph->ColumnFormat = m_graph->Format;
	}



	ict.Reset();

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}

}


void CMarketProcedureDlg::GraphCategory(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsCategories = NULL,
						rs = NULL;
		int				id, 
						i, 
						j, 
						k,
						max;
		double			total, total2;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		m_progress.SetPos(0);

		id = GetCurrentID();
		//get referrals

		CString strOrderBy;
//		if (IsDlgButtonChecked(IDC_INCLUDE_TAX)) {
		
			strOrderBy = "SumMoney";
		//}
		//else {
			
		//	strOrderBy = "SumNoTax";
		//}
		
		sql.Format ("SELECT (CASE WHEN ID = %i THEN 'ZZZZZZZ' ELSE CategoriesT.Name END) AS Name, "
			"ID FROM CategoriesT "
			"WHERE Parent = %i OR ID = %i ORDER BY Name", id, id, id);
				
		rsCategories = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsCategories->RecordCount;
		m_graph->RowCount = max;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(25);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			m_graph->ColumnFormat = m_graph->Format;
		}

		m_graph->XAxisDesc = "Category";

		m_graph->XAxisSortStyle = cgSortAlpha;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		int currentID;
		i = 0;
		while (!rsCategories->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsCategories->Fields->GetItem("Name")->Value);
			m_graph->RowID = AdoFldLong(rsCategories, "ID");

			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = rsCategories->Fields->GetItem("ID")->Value.lVal;
			descendants.Add(currentID);
			if (currentID != id)//special [Other] column
				CategoryDescendants(descendants);
			else m_graph->RowText = "[Other]";

			//Set rowList so we know if there is a descendant
			if (descendants.GetSize() > 1) {
				m_arClickableRows.Add(currentID);
				m_graph->RowDrillDown = TRUE;
			}
			else {
				m_graph->RowDrillDown = FALSE;
			}

			//for each column
			for (j = 0; j < desc.Size(); j++)
			{	m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				if (!rs->eof)
					for (k = 0; k < descendants.GetSize(); k++) //for each record
					{	while (!rs->eof)
						{	if (descendants[k] == AdoFldLong(rs, "CategoryID"))
							{	total += AdoFldDouble(rs, desc.Field(j));
								//(e.lally 2009-09-16) PLID 35559 - Added support for percents
								if (desc.Op(j) == GraphDescript::GD_DIV || desc.Op(j) == GraphDescript::GD_PERCENT)
									total2 += AdoFldDouble(rs, desc.Field2(j));
							}							
							rs->MoveNext();
						}
						rs->MoveFirst();
					}
				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						m_graph->Value = total;
						break;
					case GraphDescript::GD_DIV:
					//(e.lally 2009-09-16) PLID 35559 - Added support for percent operations
					case GraphDescript::GD_PERCENT:
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						if (total2 != 0) {
							double dblResult = total / total2;
							//(e.lally 2009-09-16) PLID 35559 - percent mode expects 1/2 to be 50, not 0.5 
								//so multiple the result by 100
							if(desc.Op(j) == GraphDescript::GD_PERCENT){
								dblResult = dblResult * 100.00;
							}
							m_graph->Value = dblResult;
						}
						else m_graph->Value = 0;
						break;
					default:
						ASSERT(FALSE);//things I haven't handeled yet, or bad values
				}
			}
			rsCategories->MoveNext();
			m_progress.SetPos(50 + ((50 * i) / max));
		}

		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}

// TODO Does this do anything?? no one calls here
void CMarketProcedureDlg::OnMoneyByCategory() 
{
	if (LastChecked(IDC_MONEY_BY_CATEGORY)) return;

	// (j.gruber 2009-06-24 11:51) - PLID 34714 - show the procedure filter
	GetDlgItem(IDC_MARKET_PROC_FILTER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_MARKET_PROC_LINK)->ShowWindow(SW_HIDE);
	
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 16:51) - PLID 35051

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(FALSE);
	if(GetMainFrame()->m_pDocToolBar->GetType() != EFFMonByCategory) {
		SetType(EFFMonByCategory);
		SetFilter(mfPaymentDate, mftDate);
		SetFilter(mfChargeLocation, mftLocation);
		SetFilter(mfChargeProvider, mftProvider);
		m_strCategory = "RootNode";
		m_graph->Title = "All";
		m_graph->PrintTitle = "Revenue By Category";
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(0);

	ResetGraph();
}


void CMarketProcedureDlg::MoneyByCategory() 
{
	CWaitCursor wc;
	
	m_graph->Format = _bstr_t(GetCurrencySymbol());

//	if (IsDlgButtonChecked(IDC_INCLUDE_TAX)) {
		
		GraphByCategory(EFFMonByCategory, "Revenue", "SumMoney");
//	}
//	else {
			
//		GraphByCategory(EFFMonByCategory, "Revenue", "SumNoTax");
//	}


}


void CMarketProcedureDlg::OnUp() 
{

	if (m_strCategory == "RootNode")
		return;

	try
	{
		CString	sql = "SELECT R2.Name FROM CategoriesT AS R1 LEFT JOIN CategoriesT AS R2 "
			"ON R1.Parent = R2.ID "
			"WHERE R1.Name = \'" + _Q(m_strCategory) + "\';";
		
		_RecordsetPtr rs = CreateRecordsetStd(sql);
		ASSERT (rs->RecordCount == 1);

		if (rs->Fields->GetItem("Name")->Value.vt == VT_BSTR)
			m_strCategory= (LPCTSTR) _bstr_t(rs->Fields->GetItem("Name")->Value);
		else m_strCategory = "RootNode";

	}
	NxCatchAll("Could not go back");

	OnGo();

	//revert to showing the default amount of records
	m_graph->ShowXRecords = 10;	
}

int CMarketProcedureDlg::GetCurrentID()
{
	if (m_strCategory== "RootNode")
		return 0;

	CString sql;

	sql = "SELECT ID FROM CategoriesT WHERE Name = \'" + _Q(m_strCategory) + "\';";
	
	_RecordsetPtr rs = CreateRecordsetStd(sql);
	ASSERT (rs->RecordCount == 1);
	return rs->Fields->GetItem("ID")->Value.lVal;
}


void CMarketProcedureDlg::OnChangeBackButtonPosColumnGraph() {
	
	CRect rect;
	m_Up.GetWindowRect(rect);
	ScreenToClient(rect);

	CRect graphrect;
	GetDlgItem(IDC_EFF_GRAPH)->GetWindowRect(graphrect);
	ScreenToClient(graphrect);

 	rect.top = graphrect.bottom - m_graph->BackButtonYDiff;
	rect.bottom = rect.top + m_graph->BackButtonHeight;
	m_Up.MoveWindow(rect);
}

void CMarketProcedureDlg::OnOnClickColumnGraph(short Row, short Column) 
{
	m_graph->Row = Row;
	for(int i = 0; i < m_arClickableRows.GetSize(); i++) {
		if(m_arClickableRows.GetAt(i) == m_graph->RowID) {
			m_strCategory = (LPCTSTR)m_graph->RowText;
			OnGo();
			return;
		}
	}	
}



CString CMarketProcedureDlg::GetCurrentGraphSql() {

	return m_strSql;

}


void CMarketProcedureDlg::Print(CDC *pDC, CPrintInfo *pInfo){
	
	if (!m_bGraphEmpty) {

		// (j.gruber 2007-03-19 15:07) - PLID 25260 - show filter string on graphs 
		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame) {
			CDocBar *pDoc = pMainFrame->m_pDocToolBar;
			if (pDoc) {
				if (!(pDoc->m_dwHidden & DBF_LOCATION)) {
					m_graph->LocationFilter = _bstr_t(pDoc->GetLocationFilterString());
				}
				else {
					m_graph->LocationFilter = "";
				}

				if (!(pDoc->m_dwHidden & DBF_PROVIDER)) {
					m_graph->ProviderFilter = _bstr_t(pDoc->GetProviderFilterString());
				}
				else {
					m_graph->ProviderFilter = "";
				}
				if (!(pDoc->m_dwHidden & DBF_FILTER)) {
					m_graph->PatientFilter = _bstr_t(pDoc->GetPatFilterFilterString());
				}
				else {
					m_graph->PatientFilter = "";
				}
				if (!(pDoc->m_dwHidden & DBF_DATES)) {
					m_graph->DateFilter = _bstr_t(pDoc->GetDateFilterString());
				}
				else {
					m_graph->DateFilter = "";
				}
				if (!(pDoc->m_dwHidden & DBF_CATEGORY)) {
					m_graph->CategoryFilter = _bstr_t(pDoc->GetCategoryFilterString());
				}
				else {
					m_graph->CategoryFilter  = "";
				}
				if (!(pDoc->m_dwHidden & DBF_RESP)) {
					m_graph->RevenueSourceFilter = _bstr_t(pDoc->GetRespFilterString());
				}
				else {
					m_graph->RevenueSourceFilter  = "";
				}

				// (j.gruber 2009-06-26 11:01) - PLID 34719 - added the procedure graph
				m_graph->ExtraFilter = _bstr_t(GetProcedureFilterDescription());			

			}
		}

		m_graph->Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
	}

}


// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
CString CMarketProcedureDlg::GetCurrentGraphFilters(ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable) {

	GraphDescript desc;
	CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strPatFilter, PatFilterIDs, strDateField, strCategory, strResp, strLocationField, strProvField;
	int id, nCategory, nResp;
	CString referrals;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
	
	
	if (UseFilter(mftProvider))  {
		
		if (HasMultiFilter(strProvField)) {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strProvField, strFilter1, strFilter2);
		
			//fill these, then we will clear them if we need to 
			strProvFilter1.Format(" AND %s %s) ", strFilter1, provIDs);
			strProvFilter2.Format(" AND %s %s) ", strFilter2, provIDs);

			if (strFilter1.IsEmpty()) {
				strProvFilter1 = "";
			}
				
			if (strFilter2.IsEmpty()) {
				strProvFilter2 = "";
			}
		}
		else {
			strProvFilter1.Format(" AND %s %s) ", strProvField, provIDs);
		}
	}

	if (UseFilter(mftLocation) ) {
		if (HasMultiFilter(strLocationField) ) {
			
			CString strFilter1, strFilter2;
			ParseMultiFilter(strLocationField, strFilter1, strFilter2);
		
			//fill these, then we will clear them if we need to 
			strLocFilter1.Format(" AND %s IN %s ", strFilter1, locIDs);
			strLocFilter2.Format(" AND %s IN %s ", strFilter2, locIDs);

			if (strFilter1.IsEmpty()) {
				strLocFilter1 = "";
			}
			
			if (strFilter2.IsEmpty()) {
				strLocFilter2 = "";
			}
		}
		else {
			strLocFilter1.Format(" AND %s IN %s ", strLocationField, locIDs);
		}
	}
	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);
	
	if (((MarketGraphType)GetType() == EFFMonByProc || (MarketGraphType)GetType() == EFFMonByCategory)) {

		if (nCategory!= -1) 
			strCategory.Format(" AND ServiceT.Category = %i ", nCategory);
		
		if (nResp == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
		}
		else if (nResp == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
		}
	}

	CString strFrom, strTo;
	if (!strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}
		
	// (j.gruber 2009-06-26 12:28) - PLID 34719 - added procedure filter
	CString strProcFilter;
	if (((MarketGraphType)GetType() != EFFMonByCategory) && !m_strProcedureList.IsEmpty()) {
			strProcFilter.Format(" AND ProcedureT.ID IN %s", m_strProcedureList);
	}

	CString strFilter = " [From] [To] [Prov1] [Loc1] [PatFilter] [Resp]  [Category]  [Cat] [Loc2] [Prov2] [Proc] ";

	strFilter.Replace("[From]", strFrom);
	strFilter.Replace("[To]", strTo);
	strFilter.Replace("[Prov1]", strProvFilter1);
	strFilter.Replace("[Prov2]", strProvFilter2);
	strFilter.Replace("[Loc1]", strLocFilter1);
	strFilter.Replace("[Loc2]", strLocFilter2);
	strFilter.Replace("[PatFilter]", strPatFilter);
	strFilter.Replace("[Category]", strCategory);
	strFilter.Replace("[Resp]", strResp);
	strFilter.Replace("[Cat]", "");
	strFilter.Replace("[Proc]", strProcFilter);



	

	if (GetType() == EFFMonByCategory) {
		id = GetCurrentID();

		// (b.cardillo 2004-08-06 09:22) - PLID 13747 - Made it so if the id is 0, which for categories is 
		// the sentinel value that indicates "all categories", we simply exclude NULL categories, which has 
		// the same effect as filtering on the set of all categories (which is what we used to do but it 
		// was slower, especially if you had a ton of categories).
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
		sql.Replace("[Cat]", CString("AND ") + (id == 0 ? "ServiceT.Category IS NOT NULL" : CategoryDescendants(id, "ServiceT.Category")));
	}

	return strFilter;
}

void CMarketProcedureDlg::OnGo() 
{
	m_bRenderedOnce = true;
	UpdateView();	
}

void CMarketProcedureDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	if (!bShow) {
		LastChecked(-1);
	}

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

void CMarketProcedureDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
// a.walling PLID 20695 5/25/06 set the graph to a blank state and update the status of any controls
//			set bClear to false if you just want to reset/enable the render button
// (c.haag 2007-03-15 16:53) - PLID 24253 - Added support for forced refreshes (the parent view uses them)
{
	if (!m_bActive && !bForceReset) { // switching from another tab or module.
		return;
	}

	if (bClear) {
		// memory is freed in the control
		m_graph->ColumnCount = 0;
		m_graph->RowCount = 0;
		m_arClickableRows.RemoveAll(); // these will be rebuilt when the graph is refreshed.

		m_bGraphEmpty = true;

		if (strTitle)
			m_graph->Title = (LPCSTR)strTitle; // change the title to invalidate the graph and force a refresh
		else
			m_graph->Title = "";
	}

	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOW);
	InvalidateConsToProcRadioButtons();
	// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
	InvalidateShowInquiriesButton();
}

bool CMarketProcedureDlg::LastChecked(int nID) 
{
	if ( (m_nLastChecked == nID) && m_bActive )
		return true;
	
	m_nLastChecked = nID;
	return false;
}

LRESULT CMarketProcedureDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	GetDlgItem(IDC_GO)->EnableWindow(true);	

	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}

// (j.gruber 2009-06-24 11:15) - PLID 34714 - make graphs filterable by Procedure
LRESULT CMarketProcedureDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MARKET_PROC_LINK:
			if (SelectMultiProcedures()) {				
				ResetGraph();
			}
			break;
		
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CMarketProcedureDlg::OnLabelClick");
	return 0;
}

BOOL CMarketProcedureDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_dwProcIDList.GetSize() > 1 && (!IsDlgButtonChecked(IDC_MONEY_BY_CATEGORY))) 
		{
			CRect rc;
			GetDlgItem(IDC_MARKET_PROC_FILTER)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll("Error in CMarketProcedureDlg::OnSetCursor");
	
	return CMarketingDlg::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CMarketProcedureDlg::SelectMultiProcedures() {

	CString strFrom, strWhere;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProcedureT");
	HRESULT hRes;
	bool bDontFill = false;

	long nResult = 0;
	dlg.PreSelect(m_dwProcIDList);
	
	dlg.m_strNameColTitle = "Procedure";

	//(e.lally 2009-09-28) PLID 35521 - Pull the clauses from the datalist
	strFrom = AsString(m_pProcedureList->GetFromClause());
	strWhere = AsString(m_pProcedureList->GetWhereClause());
	//(e.lally 2009-09-28) PLID 35521 - Allow no selection, which resets to all procedures
	hRes = dlg.Open(strFrom, strWhere, "ProcedureT.ID", "ProcedureT.Name", "Please select the procedures you want to see.", 0);

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of procedures with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwProcIDList);
		m_strProcedureList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwProcIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MARKET_PROC_FILTER, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlProcedureLabel.SetText(strNames);
			m_nxlProcedureLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MARKET_PROC_LINK, SW_SHOW);			
			m_nxlProcedureLabel.Invalidate();
			m_nCurrentProcedureID = -2;			
		}
		else if(m_dwProcIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MARKET_PROC_LINK, SW_HIDE);
			ShowDlgItem(IDC_MARKET_PROC_FILTER, SW_SHOW);
			m_pProcedureList->SetSelByColumn(0, (long)m_dwProcIDList.GetAt(0));
			m_nCurrentProcedureID = (long)m_dwProcIDList.GetAt(0);			

			//clear the array
			m_dwProcIDList.RemoveAll();
		}
		else {
			//(e.lally 2009-09-28) PLID 35521 - Reset to All procedures
			m_strProcedureList = "";
			m_nCurrentProcedureID = -1;
			ShowDlgItem(IDC_MARKET_PROC_LINK, SW_HIDE);
			ShowDlgItem(IDC_MARKET_PROC_FILTER, SW_SHOW);
			m_pProcedureList->SetSelByColumn(0, m_nCurrentProcedureID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwProcIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MARKET_PROC_FILTER, SW_HIDE);
			m_nxlProcedureLabel.SetText(GetProcedureNamesFromIDString(m_strProcedureList));
			m_nxlProcedureLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MARKET_PROC_LINK, SW_SHOW);
			InvalidateDlgItem(IDC_MARKET_PROC_LINK);
		}
		else {
			//They selected exactly one. (even if that one was "<No Purpose>"
			ShowDlgItem(IDC_MARKET_PROC_LINK, SW_HIDE);
			ShowDlgItem(IDC_MARKET_PROC_FILTER, SW_SHOW);
			m_pProcedureList->SetSelByColumn(0, m_nCurrentProcedureID);
		}
	}

	return bReturn;
	
}


CString CMarketProcedureDlg::GetProcedureNamesFromIDString(CString strIDs) {

	// we have the information in the datalist, let's just get it from there instead of querying the data
	//it should be faster that way
	CString strReturn = "";

	//get the parentheses off
	strIDs.TrimRight(')');
	strIDs.TrimLeft('(');

	//first off, see how many procedures are in the list
	long nResult = strIDs.Find(",");
	if (nResult == -1) {

		//there is only one ID, so find item name
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->FindByColumn(0, (long)atoi(strIDs), 0, FALSE);
		if (pRow) {
			return VarString(pRow->GetValue(plcName));
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
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pProcedureList->GetFirstRow();		
		
		while (pRow) {
			
			long nID = VarLong(pRow->GetValue(plcID));
			long nVal = mapIDs.Lookup(nID, nID);
			if (nVal > 0) {
				strReturn += VarString(pRow->GetValue(plcName)) + ", ";
			}
			pRow = pRow->GetNextRow();
		}

		//take the last comma off
		strReturn = strReturn.Left(strReturn.GetLength() - 2);
	}

	return strReturn;
}

void CMarketProcedureDlg::OnRequeryFinishedProcedures(short nFlags)
{
	try {
		//(e.lally 2009-09-28) PLID 35521 - Put the specialty rows at the top, not just sorted
		//add the multiple and the all
		NXDATALIST2Lib::IRowSettingsPtr pRowMulti, pRowAll;
		pRowMulti = m_pProcedureList->GetNewRow();
		pRowMulti->PutValue(plcID, (long)-2);
		pRowMulti->PutValue(plcName, _variant_t("<Multiple Procedures>"));
		m_pProcedureList->AddRowBefore(pRowMulti, m_pProcedureList->GetFirstRow());

		pRowAll = m_pProcedureList->GetNewRow();
		pRowAll->PutValue(plcID, (long)-1);
		pRowAll->PutValue(plcName, _variant_t("<All Procedures>"));
		m_pProcedureList->AddRowBefore(pRowAll, pRowMulti);

		//set the filter
		m_pProcedureList->CurSel = pRowAll;
		m_nCurrentProcedureID = -1;
		m_dwProcIDList.RemoveAll();
		m_strProcedureList = "";

		//(e.lally 2009-12-07) PLID 35521 - Since the previous selections are not saved, reset the procedure label and dropdown
		m_nxlProcedureLabel.SetText("");
		ShowDlgItem(IDC_MARKET_PROC_LINK, SW_HIDE);
		ShowDlgItem(IDC_MARKET_PROC_FILTER, SW_SHOW);

		
	}NxCatchAll("Error in CMarketProcedureDlg::OnRequeryFinishedReferralSources");
}

void CMarketProcedureDlg::OnSelChosenProcedureList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL) {	
			m_pProcedureList->SetSelByColumn(plcID, -1);
			m_strProcedureList = "";
			m_nCurrentProcedureID = -1;
			return;		
		}

		long nID = VarLong(pRow->GetValue(plcID));

		if (nID == -2) {

			//they want multipurposes
			if (!SelectMultiProcedures()) {
				//nothing changed
				return;
			}

			
		}
		else if (nID == -1) {
			m_strProcedureList = "";
			m_nCurrentProcedureID = -1;
		}
		else {

			m_strProcedureList.Format("(%li)", nID);
			m_nCurrentProcedureID = nID;
		}	
		
		ResetGraph();
	}NxCatchAll("Error in CMarketProcedureDlg::OnSelChosenReferralSourceList");
}


void CMarketProcedureDlg::OnSelChangingProcFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CMarketProcedureDlg::OnSelChangingProcFilter");	
}

// (j.gruber 2009-06-26 12:56) - PLID 34719 - added procedure filter function
CString CMarketProcedureDlg::GetProcedureFilterDescription() {

	try {

		if ((MarketGraphType)GetType() != EFFMonByCategory) {
			if (!m_strProcedureList.IsEmpty()) {
				return "Procedure(s): " + GetProcedureNamesFromIDString(m_strProcedureList);
			}
			else {
				return "All Procedures";
			}
		}
		else {
			return "";
		}
	}NxCatchAll("Error in CMarketProcedureDlg::GetProcedureFilterDescription()");

	return "";
}

// (z.manning 2009-08-31 17:16) - PLID 35051
void CMarketProcedureDlg::OnConfigureApptTypes() 
{
	try
	{
		CConversionRateByDateConfigDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			ResetGraph(false);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketProcedureDlg::ShowConversionRateControls(BOOL bShow)
{
	UINT nShowCmd = bShow ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES_PROC)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_ALL_COLUMNS_PROC)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY_PROC)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY_PROC)->ShowWindow(nShowCmd);

	if(bShow) {
		m_ShowAllRad.SetCheck(TRUE);
		m_ShowNumberRad.SetCheck(FALSE);
		m_ShowPercentRad.SetCheck(FALSE);
	}
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketProcedureDlg::OnShowAllColumns() 
{
	if (LastChecked(IDC_SHOW_ALL_COLUMNS_PROC)) return;
	ResetGraph();
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketProcedureDlg::OnShowNumbersOnly() 
{
	if (LastChecked(IDC_SHOW_NUMBERS_ONLY_PROC)) return;
	ResetGraph();
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketProcedureDlg::OnShowPercentagesOnly() 
{
	if (LastChecked(IDC_SHOW_PERCENTAGES_ONLY_PROC)) return;
	ResetGraph();
}

// (z.manning 2009-09-09 15:28) - PLID 35051 - Used to work around weird drawing issues with
// these radio buttons that are in the column graph area.
void CMarketProcedureDlg::InvalidateConsToProcRadioButtons()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_ALL_COLUMNS_PROC);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_NUMBERS_ONLY_PROC);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY_PROC);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}

// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
void CMarketProcedureDlg::InvalidateShowInquiriesButton()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_INQUIRIES);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}