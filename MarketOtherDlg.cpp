#include "stdafx.h"

// Effectiveness.cpp : implementation file
//
#include "MarketOtherDlg.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "MsgBox.h"
#include "InternationalUtils.h"
#include "MarketUtils.h"
#include "MarketFilterPickerDlg.h"
#include "DocBar.h"

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
using namespace SmallSTDOLE2Lib;
using namespace ColumnGraph;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CMarketOtherDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketOtherDlg::CMarketOtherDlg(CWnd* pParent)
	: CMarketingDlg(CMarketOtherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketOtherDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/reason.htm";

}

CMarketOtherDlg::~CMarketOtherDlg()
{

}

void CMarketOtherDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketOtherDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress);
	DDX_Control(pDX, IDC_COLOR1, m_color1);
	DDX_Control(pDX, IDC_COLOR3, m_color3);
	DDX_Control(pDX, IDC_CANCEL_BY_REASON, m_CancelByReason);
	DDX_Control(pDX, IDC_NO_SHOW_BY_REASON, m_NoShowByReason);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketOtherDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketOtherDlg)
	ON_BN_CLICKED(IDC_NO_SHOW_BY_REASON, OnNoShowByReason)
	ON_BN_CLICKED(IDC_CANCEL_BY_REASON, OnCancelByReason)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_SETCURSOR()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketOtherDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketOtherDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketOtherDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CMarketOtherDlg::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();
	CString			parent,
					item;

	m_graph = GetDlgItem(IDC_EFF_GRAPH)->GetControlUnknown();
	m_graph->Background = 0xFDFDFD;

	m_bRenderedOnce = false;
	m_bActive = false; // will be set to true as soon as OnShowWindow is handled, and false when switching to a different tab.
	m_bGraphEmpty = true;

	m_Go.AutoSet(NXB_MARKET);
	

	// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
	ColumnGraph::FontPtr font;
	font = m_graph->Font;
	font->PutName("Arial Narrow");
	font->PutSize(COleCurrency(13, 0));

	//bind the filter data lists

	m_CancelByReason.SetToolTip("Display the total number of Cancelled appointments categorized by reason.");
	m_NoShowByReason.SetToolTip("Display the total number of No Show appointments categorized by reason.");

	m_CancelByReason.SetCheck(1);
	OnCancelByReason();
	
	return FALSE;
}


void CMarketOtherDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
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
				}			
				else {

					//if we aren't in the view, we don't need to refresh
					return;
				}
			}
			m_bRenderedOnce = false;
			m_bGraphEmpty = false;
			
			if (m_CancelByReason.GetCheck()) {
				CancelByReason();
			}
			else if (m_NoShowByReason.GetCheck()) {
				NoShowByReason();
			}
		}
		else { // set filters
			if (m_CancelByReason.GetCheck()) {
				OnCancelByReason();
			}
			else if (m_NoShowByReason.GetCheck()) {
				OnNoShowByReason();
			}
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
	}NxCatchAll("Error Updating View");
}

void CMarketOtherDlg::OnNoShowByReason() 
{
	if (LastChecked(IDC_NO_SHOW_BY_REASON)) return;

	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMNoShowByReason) {
		SetType(NUMNoShowByReason);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketOtherDlg::NoShowByReason() 
{
	m_graph->PrintTitle = "No Shows By Reason";
	
	GraphByReason(NUMNoShowByReason, "No Shows", "NumNoShow", 2);
		
}

void CMarketOtherDlg::GraphByReason(MarketGraphType mktType, CString strDescription, CString strField, long nType) {

	
	GraphDescript desc;
	CString sql, from, to, docFilter, provIDs, locFilter, locIDs, strDateField, strPatFilter, PatFilterIDs, strLocationField, strProvField;
	int nResp, nCategoryID;
	CString referrals;

	//enable the Show Completed only checkbox
	//GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_SHOW);

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);
	if (UseFilter(mftProvider)) {
		docFilter.Format(" AND %s %s) ", strProvField, provIDs);
	}
	if (locIDs != "") //user personT location instead of scheduler location
		locFilter.Format(" AND %s IN %s ", strLocationField, locIDs);
	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

	CString strFrom, strTo;
	if (! strDateField.IsEmpty()) {
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
	sql.Replace("[Prov1]", docFilter);
	sql.Replace("[Loc1]", locFilter);
	sql.Replace("[PatFilter]", strPatFilter);

	desc.m_sql = sql;
	Save(sql);
		//calculate the summary
	CString strTempDesc1, strTempDesc2;

	//set the timeout
	CIncreaseCommandTimeout ict(600);

	m_progress.SetPos(10);

	// PLID 17057 - take out query for speed improvement
	//rsTotal = CreateRecordset("SELECT Sum(%s) as Total1 FROM (%s) Base ", strField, sql);
	strTempDesc1.Format("%s", strDescription);
	//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
	desc.Add(strTempDesc1, strField, strDescription == "Cancellations" ?GetMarketGraphColor(mgcBrightRed) : GetMarketGraphColor(mgcBrightBlue));
		
	m_graph->Format = "%0.0f";

	if (nType == 1) {
		//its cancels
		GraphReasons(desc, "AptCancelReasonT");
	}
	else if (nType == 2) {

		//its no shows
		GraphReasons(desc, "AptNoShowReasonT");
	}


	//show the totals
	for (int i = 0; i < desc.Size(); i++)
	{	m_graph->Column = i;
		m_graph->Color = desc.Color(i);
		CString strText;
		strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
		m_graph->ColumnText = (_bstr_t)strText;
		m_graph->ColumnFormat = m_graph->Format;
	}
	

	//set it back
	ict.Reset();

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketOtherDlg::GraphReasons(GraphDescript &desc, CString strTableName)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsReason = NULL,
						rs = NULL;
		int				currentID, id = 0, i = 0, 
						j, 
						k,
						max;
		double			total, total2;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		m_progress.SetPos(20);

		sql.Format (" SELECT * FROM (SELECT %s.Description AS Name, "
			" ID as ReasonID FROM %s  UNION  SELECT 'Other', -1) SubQ "
			" ORDER BY Name ", strTableName, strTableName);

		rsReason = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsReason->RecordCount;
		m_graph->RowCount = max;
//		m_arClickableRows.RemoveAll();
		m_progress.SetPos(30);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			m_graph->ColumnFormat = m_graph->Format;
		}

		m_graph->XAxisDesc = "Reason";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		i = 0;
		while (!rsReason->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsReason->Fields->GetItem("Name")->Value);
			m_graph->RowID = AdoFldLong(rsReason, "ReasonID");
			
			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = rsReason->Fields->GetItem("ReasonID")->Value.lVal;
			descendants.Add(currentID);


			//for each column
			for (j = 0; j < desc.Size(); j++)
			{	m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				if (!rs->eof)
					for (k = 0; k < descendants.GetSize(); k++) //for each record
					{	while (!rs->eof)
						{	if (descendants[k] == AdoFldLong(rs, "ReasonID", -1))
							{	total += AdoFldDouble(rs, desc.Field(j));
								//(e.lally 2009-09-16) PLID 35559 - Added support for percent operations
								if (desc.Op(j) == GraphDescript::GD_DIV || desc.Op(j) == GraphDescript::GD_PERCENT) {
									total2 += AdoFldDouble(rs, desc.Field2(j));
								}
							}							
							rs->MoveNext();
						}
						rs->MoveFirst();
					}
				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						m_graph->Value = total;
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						break;
					case GraphDescript::GD_DIV:
					//(e.lally 2009-09-16) PLID 35559 - Added support for percents
					case GraphDescript::GD_PERCENT:
						if (total2 != 0){
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
			rsReason->MoveNext();
			m_progress.SetPos(50 + ((50 * i) / max));
		}

		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}

void CMarketOtherDlg::OnCancelByReason()
{
	if (LastChecked(IDC_CANCEL_BY_REASON)) return;
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMCanByReason) {
		SetType(NUMCanByReason);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketOtherDlg::CancelByReason()
{
	m_graph->PrintTitle = "Cancellations By Reason";
	
	GraphByReason(NUMCanByReason, "Cancellations", "NumCancel", 1);
}


// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
CString CMarketOtherDlg::GetCurrentGraphFilters(ADODB::_ConnectionPtr pCon, CString &strPatientTempTable) {

	CString sql, from, to, strDocFilter, provIDs, strLocFilter, locIDs, strDateField, strPatFilterFilter, PatFilterIDs, strCat, strResp = "", strLocationField, strProvField;
	int nCategoryID, nResp;
	CString referrals;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);
	if (UseFilter(mftProvider)) {
		strDocFilter.Format(" AND %s %s) ", strProvField, provIDs);
	}
	//(e.lally 2010-09-02) PLID 40363 - Use the location field from the filters
	if (locIDs != "")
		strLocFilter.Format(" AND %s IN %s ", strLocationField, locIDs);
	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilterFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

	CString strFrom, strTo;
	if (!strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	CString strFilter = " [From] [To] [Prov1] [Loc] [PatFilter] ";

	strFilter.Replace("[From]", strFrom);
	strFilter.Replace("[To]", strTo);
	strFilter.Replace("[Prov1]", strDocFilter);
	strFilter.Replace("[Loc]", strLocFilter);
	strFilter.Replace("[PatFilter]", strPatFilterFilter);

	return strFilter;
}

void CMarketOtherDlg::Print(CDC *pDC, CPrintInfo *pInfo){
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
			}
		}


		m_graph->Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
	}

}

void CMarketOtherDlg::OnGo() 
{
	if (!m_bRenderedOnce) {
		m_graph->Title = "";
	}
	m_bRenderedOnce = true;
	UpdateView();
	
}

void CMarketOtherDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	if (!bShow) {
		LastChecked(-1);
	}

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

void CMarketOtherDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
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
		m_bGraphEmpty = true;

		if (strTitle)
			m_graph->Title = (LPCSTR)strTitle; // change the title to invalidate the graph and force a refresh
		else
			m_graph->Title = "";
	}


	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOW);
}

bool CMarketOtherDlg::LastChecked(int nID) 
{
	if ( (m_nLastChecked == nID) && m_bActive )
		return true;
	
	m_nLastChecked = nID;
	return false;
}

LRESULT CMarketOtherDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	GetDlgItem(IDC_GO)->EnableWindow(true);	

	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}
