#include "stdafx.h"

// Effectiveness.cpp : implementation file
//
#include "Marketingrc.h"
#include "MarketDateDlg.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "MsgBox.h"
#include "InternationalUtils.h"
#include "MarketUtils.h"
#include "ConversionRateByDateConfigDlg.h"
#include "MarketFilterPickerDlg.h"
#include "DocBar.h"
#include "ConfigureApptConvGroupsDlg.h"

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.gruber 2011-05-06 16:40) - PLID 43583 - conversion group list
enum ConvGroupColumns {
	cgcID = 0,
	cgcName,
};

// (a.walling 2007-11-06 11:56) - PLID 28000 - VS2008 - Need to specify namespace
using namespace NXDATALISTLib;
using namespace SmallSTDOLE2Lib;
using namespace ADODB;
using namespace ColumnGraph;

/////////////////////////////////////////////////////////////////////////////
// CMarketDateDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketDateDlg::CMarketDateDlg(CWnd* pParent)
	: CMarketingDlg(CMarketDateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketDateDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/date.htm";

	SetFilter(mfFirstContactDate, mftDate);
}

CMarketDateDlg::~CMarketDateDlg()
{
}

void CMarketDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketDateDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress);
	DDX_Control(pDX, IDC_COLOR1, m_color1);
	DDX_Control(pDX, IDC_CONVERSION_DATE_RAD, m_ConvDateRad);
	DDX_Control(pDX, IDC_SHOW_ALL_COLUMNS, m_ShowAllRad);
	DDX_Control(pDX, IDC_SHOW_NUMBERS_ONLY, m_ShowNumberRad);
	DDX_Control(pDX, IDC_SHOW_PERCENTAGES_ONLY, m_ShowPercentRad);
	DDX_Control(pDX, IDC_PROCS_CLOSED3, m_ProcsClosed);
	DDX_Control(pDX, IDC_PROCS_PERF	, m_ProcsPerf);
	DDX_Control(pDX, IDC_TOTAL_REVENUE3, m_TotalRevenue);
	DDX_Control(pDX, IDC_PATIENTS, m_Patients);
	DDX_Control(pDX, IDC_CANCELLATIONS3, m_Cancellations);
	DDX_Control(pDX, IDC_SHOW_INQUIRIES, m_ShowInquiries);
	DDX_Control(pDX, IDC_INQ_TO_CONS_BY_PROC3, m_InqToCons);
	DDX_Control(pDX, IDC_PROS_TO_CONS_BY_PROC3, m_ProsToCons);		
	DDX_Control(pDX, IDC_MARKET_DATE_MULTI_PURPOSE_LIST, m_nxlPurposeLabel);
	DDX_Control(pDX, IDC_APPT_TO_CHARGE, m_ApptsToCharge);	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketDateDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketDateDlg)
	ON_BN_CLICKED(IDC_CONVERSION_DATE_RAD, OnConversionDateRad)
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TYPES, OnConfigureApptTypes)
	ON_BN_CLICKED(IDC_SHOW_ALL_COLUMNS, OnShowAllColumns)
	ON_BN_CLICKED(IDC_SHOW_NUMBERS_ONLY, OnShowNumbersOnly)
	ON_BN_CLICKED(IDC_SHOW_PERCENTAGES_ONLY, OnShowPercentagesOnly)
	ON_BN_CLICKED(IDC_PROCS_CLOSED3, OnProcsClosed)
	ON_BN_CLICKED(IDC_PROCS_PERF, OnProcsPerformed)
	ON_BN_CLICKED(IDC_TOTAL_REVENUE3, OnTotalRevenue)
	ON_BN_CLICKED(IDC_PATIENTS, OnPatients)
	ON_BN_CLICKED(IDC_CANCELLATIONS3, OnCancellations)
	ON_BN_CLICKED(IDC_SHOW_INQUIRIES, OnShowInquiries)
	ON_BN_CLICKED(IDC_INQ_TO_CONS_BY_PROC3, OnInqToCons)
	ON_BN_CLICKED(IDC_PROS_TO_CONS_BY_PROC3, OnProsToCons)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)	
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_APPT_TO_CHARGE, &CMarketDateDlg::OnBnClickedApptToCharge)
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TO_CHARGE, &CMarketDateDlg::OnBnClickedConfigureApptToCharge)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketDateDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketDateDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketDateDlg)
	//ON_EVENT(CMarketDateDlg, IDC_FILTERED_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenCategoryList, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_DATE_OPTION_LIST, 16 /* SelChosen */, OnSelChosenDateOptionList, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_PAT_COORD_LIST, 16 /* SelChosen */, OnSelChosenPatCoordList, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_FILTERED_PROVIDER_LIST, 16 /* SelChosen */, OnSelChosenFilteredProviderList, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_FILTERED_LOCATION_LIST, 16 /* SelChosen */, OnSelChosenFilteredLocationList, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_FILTERED_FROM_DATE, 2 /* Change */, OnChangeFilteredFromDate, VTS_NONE)
	//ON_EVENT(CMarketDateDlg, IDC_FILTERED_TO_DATE, 2 /* Change */, OnChangeFilteredToDate, VTS_NONE)
	ON_EVENT(CMarketDateDlg, IDC_APPT_PURPOSE_LIST, 16 /* SelChosen */, OnSelChosenApptPurposeList, VTS_I4)
	ON_EVENT(CMarketDateDlg, IDC_YEAR_FILTER, 16 /* SelChosen */, OnSelChosenYearFilter, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_DATE_OPTION_LIST, 1 /* SelChanging */, OnSelChangingDateOptionList, VTS_PI4)
	//ON_EVENT(CMarketDateDlg, IDC_RESP_FILTER, 16 /* SelChosen */, OnSelChosenRespFilter, VTS_I4)
	//ON_EVENT(CMarketDateDlg, IDC_EFF_GRAPH, 3 /* OnChangeBackButtonPos */, OnChangeBackButtonPosColumnGraph, VTS_NONE)	
	ON_EVENT(CMarketDateDlg, IDC_APPT_PURPOSE_LIST, 18, OnRequeryFinishedApptPurposeList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CMarketDateDlg, IDC_CONVERSION_GROUP_LIST, 16, CMarketDateDlg::SelChosenConversionGroupList, VTS_DISPATCH)
	ON_EVENT(CMarketDateDlg, IDC_CONVERSION_GROUP_LIST, 18, CMarketDateDlg::RequeryFinishedConversionGroupList, VTS_I2)
END_EVENTSINK_MAP()

BOOL CMarketDateDlg::OnInitDialog() 
{
	
	try {
		CMarketingDlg::OnInitDialog();
		CString			parent,
						item;


		m_graph = GetDlgItem(IDC_EFF_GRAPH)->GetControlUnknown();
		m_graph->Background = 0xFDFDFD;

		// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
		ColumnGraph::FontPtr font;
		font = m_graph->Font;
		font->PutName("Arial Narrow");
		font->PutSize(COleCurrency(13, 0));

		
		//bind the filter data lists
		InitializeControls();

		m_bRenderedOnce = false;
		m_bActive = false; // will be set to true as soon as OnShowWindow is handled, and false when switching to a different tab.
		m_bGraphEmpty = true;

		m_Go.AutoSet(NXB_MARKET);
		
		// (j.gruber 2007-04-19 15:42) - PLID 25288 - adding label
		m_nxlPurposeLabel.SetColor(0x00C8FFFF);
		m_nxlPurposeLabel.SetText("");
		m_nxlPurposeLabel.SetType(dtsHyperlink);

		m_TotalRevenue.SetToolTip("Display the net amount of applied payments and applied refunds generated.");
		m_ConvDateRad.SetToolTip("Display the total number of consultations, procedures, and the conversion rate.  These are configured using the ellipsis button");
		m_Patients.SetToolTip("Display the number of patients and prospects who joined the practice.");
		m_Cancellations.SetToolTip("Display the number of appointment cancellations and No Shows.");
		m_InqToCons.SetToolTip("Display the percentage of all inquiries that have progressed into consultations. The consultations include both patients and prospects.");
		m_ProsToCons.SetToolTip("Display the percentage of total prospects who have scheduled consultations.");
		m_ProcsClosed.SetToolTip("Display the total number of procedures performed versus the number of procedures that are scheduled and have a prepayment associated with the appointment via the Procedure Information Center.");
		// (j.gruber 2011-05-06 16:41) - PLID 38153
		m_ApptsToCharge.SetToolTip("Display the total number of appointments, converted appointments, and the conversion rate.  These are configured using the ellipsis button.");

		m_TotalRevenue.SetCheck(1);
		OnTotalRevenue();

		
	}NxCatchAll("Error in OnInitDialog");
	return FALSE;
}


/*void CMarketDateDlg::OnIncludeTax() 
{
	UpdateView();
	
}
*/

void CMarketDateDlg::InitializeControls() {

	m_pApptPurposeList = BindNxDataListCtrl(IDC_APPT_PURPOSE_LIST, GetRemoteData(), FALSE);
	//(e.lally 2009-09-28) PLID 35521 - Preference for detailed or master procedure list
	BOOL bUseDetailedProcList = GetRemotePropertyInt("MarketingUseDetailedProcedureFilter", 0, 0, GetCurrentUserName());
	CString strProcFilterWhere = "";
	if(bUseDetailedProcList == FALSE){
		strProcFilterWhere = "MasterProcedureID IS NULL";
	}
	m_pApptPurposeList->WhereClause = _bstr_t(strProcFilterWhere);
	m_pApptPurposeList->Requery();

	m_pYearFilter = BindNxDataListCtrl(IDC_YEAR_FILTER, GetRemoteData(), FALSE);
	// (j.gruber 2011-05-05 17:47) - PLID 43583 - conversion Group Filter
	m_pConvGroupList = BindNxDataList2Ctrl(IDC_CONVERSION_GROUP_LIST);
}

//(e.lally 2009-09-28) PLID 35521 - Put the specialty rows in after the requery finishes
void CMarketDateDlg::OnRequeryFinishedApptPurposeList(short nFlags)
{
	try{
		//(e.lally 2009-09-28) PLID 35521 - Put the specialty rows at the top, not just sorted
		//add the multiple purposes 
		IRowSettingsPtr pRow;
		pRow = m_pApptPurposeList->GetRow(-1);
		pRow->PutValue(0, (long) -2);
		pRow->PutValue(1, _variant_t("<Multiple Purposes>"));
		m_pApptPurposeList->InsertRow(pRow, 0);

		pRow = m_pApptPurposeList->GetRow(-1);
		pRow->PutValue(0, (long) -3);
		pRow->PutValue(1, _variant_t("<All Purposes>"));
		m_pApptPurposeList->InsertRow(pRow, 0);

		EnsurePurposeSelection();

	}NxCatchAll(__FUNCTION__)
}

//(e.lally 2009-09-28) PLID 35521 - Ensured the cached procedure list is still valid and the proper controls are displayed
void CMarketDateDlg::EnsurePurposeSelection()
{
	if (!m_ConvDateRad.GetCheck()) {
		ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
		ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_HIDE);
		return;
	}
	m_strPurposeList = GetRemotePropertyText("CRGPurposeList", "", 0, "<None>", TRUE);

	// (j.gruber 2007-04-19 15:15) - PLID 25288 - adding label support
	//load the dword array
	LoadApptPurposeList(m_strPurposeList);

	if (m_dwPurpIDList.GetSize() > 1) {
		ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_SHOW);
		ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_HIDE);
		
		//load the label
		m_nxlPurposeLabel.SetText(GetProcedureNamesFromIDString(m_strPurposeList));
		m_nxlPurposeLabel.SetType(dtsHyperlink);

		//InvalidateDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST);
		m_nxlPurposeLabel.Invalidate();

		m_nCurrentPurposeID = -2;
	}
	else if (m_dwPurpIDList.GetSize() == 1) {
		
		ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
		ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_SHOW);
		
		//load the label
		SetDlgItemText(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, "");

		if(m_pApptPurposeList->SetSelByColumn(0, (long)m_dwPurpIDList.GetAt(0)) == sriNoRow){
			//Failed to set the remembered value
			ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
			ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_SHOW);
			
			//load the label
			SetDlgItemText(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, "");

			m_pApptPurposeList->SetSelByColumn(0, (long)-3);

			//InvalidateDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST);
			m_nxlPurposeLabel.Invalidate();

			m_nCurrentPurposeID = -3;
			m_strPurposeList = "";
			SetRemotePropertyText("CRGPurposeList", m_strPurposeList, 0, "<None>");

		}

		m_nCurrentPurposeID = m_dwPurpIDList.GetAt(0);

		InvalidateDlgItem(IDC_APPT_PURPOSE_LIST);
	}
	else {
		//there aren't any
		ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
		ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_SHOW);
		
		//load the label
		SetDlgItemText(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, "");

		m_pApptPurposeList->SetSelByColumn(0, (long)-3);

		//InvalidateDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST);
		m_nxlPurposeLabel.Invalidate();

		m_nCurrentPurposeID = -3;
	}

}


void CMarketDateDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
	try {
		//(e.lally 2009-09-28) PLID 35521 - Preference for detailed or master procedure list. Check if our where clause does not match up.
		BOOL bUseDetailedProcList = GetRemotePropertyInt("MarketingUseDetailedProcedureFilter", 0, 0, GetCurrentUserName());
		CString strProcFilterWhere = AsString(m_pApptPurposeList->GetWhereClause());
		bool bSkipRendering = false;
		//(e.lally 2009-12-07) PLID 35521 - If we are requerying the procedure list,
			//let's stop the graph rendering so the user can confirm their filters.
		if(bUseDetailedProcList == FALSE && strProcFilterWhere.Find("MasterProcedureID IS NULL") == -1){
			ResetGraph(true, "", true);
			m_bRenderedOnce = false;
			bSkipRendering = true;
			strProcFilterWhere = "MasterProcedureID IS NULL";
			m_pApptPurposeList->WhereClause = _bstr_t(strProcFilterWhere);
			m_pApptPurposeList->Requery();
		}
		else if(bUseDetailedProcList != FALSE && strProcFilterWhere.Find("MasterProcedureID IS NULL") != -1){
			ResetGraph(true, "", true);
			m_bRenderedOnce = false;
			bSkipRendering = true;
			strProcFilterWhere = "";
			m_pApptPurposeList->WhereClause = _bstr_t(strProcFilterWhere);
			m_pApptPurposeList->Requery();
		}
		//(a.wilson 2011-10-5) PLID 38789 - added force refresh to prevent refreshing when coming back to module.
		if ((m_bRenderedOnce || (m_bActive && bForceRefresh)) && bSkipRendering == false) {
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

			if (m_ConvDateRad.GetCheck()) {
				RefreshConversionDateGraph();
			}
			else if (m_ProcsClosed.GetCheck()) {
				ProcsClosed();
			}
			else if (m_ProcsPerf.GetCheck()) {
				OnProcsPerformed();
			}
			else if (m_TotalRevenue.GetCheck()) {
				TotalRevenue();
			}
			else if (m_Patients.GetCheck()) {
				Patients();
			}
			else if (m_Cancellations.GetCheck()) {
				Cancellations();
			}
			else if (m_InqToCons.GetCheck()) {
				InqToCons();
			}
			else if (m_ProsToCons.GetCheck()) {
				ProsToCons();
			}
			// (j.gruber 2011-05-03 15:40) - PLID 38153
			else if (m_ApptsToCharge.GetCheck()) {
				ApptsToCharges();
			}
		}
		else { // redraw filters
			if (m_ConvDateRad.GetCheck()) {
				OnConversionDateRad();
			}
			else if (m_ProcsClosed.GetCheck()) {
				OnProcsClosed();
			}
			else if (m_TotalRevenue.GetCheck()) {
				OnTotalRevenue();
			}
			else if (m_Patients.GetCheck()) {
				OnPatients();
			}
			else if (m_Cancellations.GetCheck()) {
				OnCancellations();
			}
			else if (m_InqToCons.GetCheck()) {
				OnInqToCons();
			}
			else if (m_ProsToCons.GetCheck()) {
				OnProsToCons();
			}
			// (j.gruber 2011-05-03 15:41) - PLID 38153
			else if (m_ApptsToCharge.GetCheck()) {
				OnBnClickedApptToCharge();
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

void CMarketDateDlg::OnConversionDateRad() 
{
	if (LastChecked(IDC_CONVERSION_DATE_RAD)) return;

	//show the configure button
	// (j.gruber 2007-05-09 14:57) - PLID 25288 - hide this for now
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->RedrawWindow();
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->RedrawWindow();
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->RedrawWindow();

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	m_ShowAllRad.SetCheck(TRUE);
	m_ShowNumberRad.SetCheck(FALSE);
	m_ShowPercentRad.SetCheck(FALSE);

	//GetDlgItem(IDC_SHOW_ALL_PROSPECTS)->ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_SHOW_CONS_PROSPECTS)->ShowWindow(SW_HIDE);

	//(e.lally 2009-09-28) PLID 35521 - Don't need to requery every time here.
	//m_pApptPurposeList->Requery();
	m_pYearFilter->Requery();


	if (m_pYearFilter->SetSelByColumn(0, (long)COleDateTime::GetCurrentTime().GetYear()) == -1) {
		m_pYearFilter->CurSel = (m_pYearFilter->GetRowCount() - 1);
	}	

	//(e.lally 2009-09-28) PLID 35521 - call our function for ensuring the purpose filter is set correctly from cache
	EnsurePurposeSelection();

	/*if (m_strPurposeList.IsEmpty()) {

		m_pApptPurposeList->SetSelByColumn(0, (long)-3);
	}
	else {
		long nResult = m_strPurposeList.Find(",");
		if (nResult != -1) {
			m_pApptPurposeList->SetSelByColumn(0, (long)-2);
		}
		else {
			CString strTmp = m_strPurposeList;
			strTmp.TrimLeft("(");
			strTmp.TrimRight(")");
			m_pApptPurposeList->SetSelByColumn(0, _variant_t(strTmp));
		}
	}*/


	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	
	if(GetMainFrame()->m_pDocToolBar->GetType() != CNVConstoSurgByDate) {
		SetType(CNVConstoSurgByDate);
		SetFilter(mfConsultDate, mftDate);
		SetFilter(mfNoPatApptLocation, mftLocation); 
		SetFilter(mfNoPatApptProvider, mftProvider);
	}

	ResetGraph();
}


void CMarketDateDlg::ConversionDateRad() 
{
	//make sure that we are on top
	m_graph->Title = "All";	


	RefreshConversionDateGraph();
}

// (j.gruber 2007-04-19 15:00) - load the DwordArray
void CMarketDateDlg::LoadApptPurposeList(CString strList) {

	m_dwPurpIDList.RemoveAll();

	strList.TrimLeft();
	strList.TrimRight();

	if (!strList.IsEmpty() ){

		strList.TrimLeft("(");
		strList.TrimRight(")");

		long nResult = strList.Find(",");	
		while (nResult  != -1) {

			m_dwPurpIDList.Add(atoi(strList.Left(nResult)));

			strList = strList.Right(strList.GetLength() - (nResult + 1));

			nResult = strList.Find(",");
		}

		//add the last one
		m_dwPurpIDList.Add(atoi(strList));
	}


}

void CMarketDateDlg::RefreshConversionDateGraph() {

	try {

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//set the timeout
		CIncreaseCommandTimeout ict(600);

		if(GetMainFrame()->m_pDocToolBar->GetType() != CNVConstoSurgByDate) {
			SetType(CNVConstoSurgByDate);
			SetFilter(mfConsultDate, mftDate);
			SetFilter(mfNoPatApptLocation, mftLocation);
			SetFilter(mfNoPatApptProvider, mftProvider);
			GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
		}

		m_graph->Format = "%0.0f";
		m_graph->PrintTitle = "Consult To Procedure By Date";
			
		//This is the "Conversion Rate By Date" button
		
		GraphDescript desc;
		//(e.lally 2009-09-28) PLID 35521 - call our function for ensuring the purpose filter is set correctly from cache
		EnsurePurposeSelection();

		//(e.lally 2009-09-01) PLID 35301 - Redid the tab to use the new base consult to procedure conversion base query. 
		//We get the sql query string from the utilities instead.
		CString strBaseSql = GetGraphSql(CNVConstoSurgByDate, 1, -1, pCon, strPatientTempTable);
		//(e.lally 2009-09-08) PLID 35301 - Split up the summary query because it cannot have the group by clause
		CString strSummarySql = GetGraphSql(CNVConstoSurgByDate, 2, -1, pCon, strPatientTempTable);

		//(e.lally 2009-09-11) PLID 35521 - Ideally, all this would happen in the utils for us, but since the filter is only implemented
		//locally, we have to manually do it ourselves.
		CString strProcedureFilter = GetConsToProcProcedureFilter();
		strBaseSql.Replace(PROCEDURE_FILTER_PLACEHOLDER, strProcedureFilter);
		strSummarySql.Replace(PROCEDURE_FILTER_PLACEHOLDER, strProcedureFilter);

		BOOL bShowNumbers = IsDlgButtonChecked(IDC_SHOW_NUMBERS_ONLY) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS);
		BOOL bShowPercentages = IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS);

		if(!IsConsToProcSetupValid()) {
			MessageBox("Please set up your configuration settings by clicking the ... button before running this graph");
			return;
		}

		desc.m_sql = strBaseSql;
		Save(strBaseSql);

		// (z.manning 2009-09-09 12:53) - PLID 35051 - Put the shared logic in a utility function
		AddConsToProcDataToGraphDesc(strSummarySql, &desc, bShowNumbers, bShowPercentages);
		
		GraphDate(desc);

		//set it back
		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error loading graph");
}


void CMarketDateDlg::OnConfigureApptTypes() 
{
	CConversionRateByDateConfigDlg dlg(this);
	dlg.DoModal();
//	RefreshConversionDateGraph(); a.walling 5/19/06 PLID 20695 refreshes are done manually in OnGo
	ResetGraph(false);
}

void CMarketDateDlg::OnSelChosenApptPurposeList(long nRow) 
{
	if(nRow == -1) {
		m_pApptPurposeList->CurSel = 0;
		nRow = 0;
		
	}

	long nID = VarLong(m_pApptPurposeList->GetValue(nRow, 0));

	if (nID == -2) {

		//they want multipurposes
		if (!SelectMultiPurposes()) {
			//nothing changed
			return;
		}

		
	}
	else if (nID == -3) {
		m_strPurposeList = "";
		m_nCurrentPurposeID = -3;
	}
	else {

		m_strPurposeList.Format("(%li)", nID);
		m_nCurrentPurposeID = nID;
	}
		
	SetRemotePropertyText("CRGPurposeList", m_strPurposeList, 0, "<None>");
//	RefreshConversionDateGraph();
	
	ResetGraph();
}




/*void CMarketDateDlg::CheckDataList(CMultiSelectDlg *dlg) {

	CString strChecks;

	strChecks = m_strPurposeList;

	if (strChecks.IsEmpty()) {

		//rockon, we are done, we have nothing to do!!
		return;
	}


	//get rid of the parenthesis
	strChecks.TrimLeft("(");
	strChecks.TrimRight(")");

	//loop through the list until we have all the numbers
	CString strChecksTmp = strChecks;
	long nResult;
	long nIDtoCheck;
	
	const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
	const _variant_t varFalse(VARIANT_FALSE, VT_BOOL);

	//now set the ones that need to be true, to true
	//TES 2004-1-28: I'm initializing this variable, because I don't like the warning, and plus it really is dangerous.
	nResult = strChecksTmp.Find(",");
	while ((!strChecksTmp.IsEmpty()) && (nResult != -1)) {

 		nResult = strChecksTmp.Find(",");

		if (nResult == -1) {
			//there must be only one left
			nIDtoCheck = atoi(strChecksTmp);
		}
		else {
			nIDtoCheck = atoi(strChecksTmp.Left(nResult));
		}


		//find the row in the datalist
		dlg->PreSelect(nIDtoCheck);

		//alrighty, we are done, move on
		strChecksTmp = strChecksTmp.Right(strChecksTmp.GetLength() - (nResult + 1));

	}


}*/

// (j.gruber 2007-04-19 12:45) - PLID 25288 - make it show a label, additionally, I cleaned up how this works
BOOL CMarketDateDlg::SelectMultiPurposes() {

	CString strFrom, strWhere;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ProcedureT");
	HRESULT hRes;
	bool bDontFill = false;

	long nResult = 0;
	/*CString strChecks, strChecksTmp;

	strChecks = m_strPurposeList;*/

	
		
	// Fill the dialog with existing selections
	// (j.gruber 2007-04-19 12:46) - PLID 25288 - this is a silly way to do this.
	//CheckDataList(&dlg);
	dlg.PreSelect(m_dwPurpIDList);
	
	dlg.m_strNameColTitle = "Procedure";

	//(e.lally 2009-09-28) PLID 35521 - Pull the clauses from our datalist
	strFrom = AsString(m_pApptPurposeList->GetFromClause());
	strWhere = AsString(m_pApptPurposeList->GetWhereClause());

	//(e.lally 2005-10-03) PLID 17770 - send the Multi Select a minimum of one selection
		//to be consistent with the docbar and avoid an exception with an empty list.
	//(e.lally 2009-09-28) PLID 35521 - Allow no selection and reset to All Procedures
	hRes = dlg.Open(strFrom, strWhere, "ProcedureT.ID", "ProcedureT.Name", "Please select the procedures you want to see.", 0);

	//better safe the sorry
	BOOL bReturn = TRUE;
	

	// Update our array of procedures with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwPurpIDList);
		m_strPurposeList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwPurpIDList.GetSize() > 1) {
			ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlPurposeLabel.SetText(strNames);
			m_nxlPurposeLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_SHOW);
			//InvalidateDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST);
			m_nxlPurposeLabel.Invalidate();
			m_nCurrentPurposeID = -2;
		}
		else if(m_dwPurpIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
			ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_SHOW);
			m_pApptPurposeList->SetSelByColumn(0, (long)m_dwPurpIDList.GetAt(0));
			m_nCurrentPurposeID = (long)m_dwPurpIDList.GetAt(0);
		}
		else {
			//(e.lally 2009-09-28) PLID 35521 - Reset to All Procedures
			m_strPurposeList = "";
			m_nCurrentPurposeID = -3;
			ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
			ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_SHOW);
			m_pApptPurposeList->SetSelByColumn(0, m_nCurrentPurposeID);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwPurpIDList.GetSize() > 1) {
			ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_HIDE);
			m_nxlPurposeLabel.SetText(GetProcedureNamesFromIDString(m_strPurposeList));
			m_nxlPurposeLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST);
		}
		else {
			//They selected exactly one. (even if that one was "<No Purpose>"
			ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
			ShowDlgItem(IDC_APPT_PURPOSE_LIST, SW_SHOW);
			m_pApptPurposeList->SetSelByColumn(0, m_nCurrentPurposeID);
		}
	}

	return bReturn;
	
}

void CMarketDateDlg::OnSelChosenYearFilter(long nRow) 
{
//	RefreshConversionDateGraph();	//a.walling 5/19/06 PLID 20695 these refreshes are done manually in OnGo

	ResetGraph();
}

void CMarketDateDlg::OnShowAllColumns() 
{
//	RefreshConversionDateGraph();	
	if (LastChecked(IDC_SHOW_ALL_COLUMNS)) return;
	ResetGraph();
}

void CMarketDateDlg::OnShowNumbersOnly() 
{
//	RefreshConversionDateGraph();	
	if (LastChecked(IDC_SHOW_NUMBERS_ONLY)) return;
	ResetGraph();
}

void CMarketDateDlg::OnShowPercentagesOnly() 
{
//	RefreshConversionDateGraph();	
	if (LastChecked(IDC_SHOW_PERCENTAGES_ONLY)) return;
	ResetGraph();
}

void CMarketDateDlg::OnProcsPerformed() {

	/*if(GetMainFrame()->m_pDocToolBar->GetType() != DATEPerfByProcs) {
		SetType(DATEPerfByProcs);
		SetFilter(mfApptDate, mftDate);
				
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	
	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	m_graph->Format = "%0.0f";
	m_graph->PrintTitle = "Procedures Performed vs. Closed By Date";
	GraphByDate(DATEPerfByProcs, "Performed", "ProcsPerformed");	*/
}

void CMarketDateDlg::OnProcsClosed() {
	if (LastChecked(IDC_PROCS_CLOSED3)) return;

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
	ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	if(GetMainFrame()->m_pDocToolBar->GetType() != DATECloseByProc) {
		SetType(DATECloseByProc);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	ResetGraph();
}

void CMarketDateDlg::ProcsClosed() {
	
	m_graph->Format = "%0.0f";
	m_graph->PrintTitle = "Procedures Performed vs. Closed By Date";
	GraphByDate(DATECloseByProc, "Performed", "ProcsPerformed", "Closed", "ProcsClosed");	
}

void CMarketDateDlg::OnTotalRevenue() {
	if (LastChecked(IDC_TOTAL_REVENUE3)) return;
	
	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
	ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	if(GetMainFrame()->m_pDocToolBar->GetType() != DATERevByDate) {
		SetType(DATERevByDate);
		SetFilter(mfPaymentDate, mftDate);
		SetFilter(mfTransLocation, mftLocation);
		SetFilter(mfTransProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(0);

	SetType(DATERevByDate);
	ResetGraph();
}

void CMarketDateDlg::TotalRevenue() {

	//default date filter to payment date


	m_graph->Format = _bstr_t(GetCurrencySymbol());
	m_graph->PrintTitle = "Revenue By Date";
	GraphByDate(DATERevByDate, "Revenue", "TotalRevenue");	
}

void CMarketDateDlg::OnPatients() {
	if (LastChecked(IDC_PATIENTS)) return;

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
	ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_INQUIRIES)->Invalidate();

	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	if(GetMainFrame()->m_pDocToolBar->GetType() != DATEPatients) {
		SetType(DATEPatients);
		SetFilter(mfPatientLocation, mftLocation);
		SetFilter(mfPatientProvider, mftProvider);
	}
	ResetGraph();
}

void CMarketDateDlg::Patients() {
	m_graph->Format = "%0.0f";
	m_graph->PrintTitle = "Patients By Date";
	 
	if (IsDlgButtonChecked(IDC_SHOW_INQUIRIES)) {
		GraphByDate(DATEPatients, "Patients", "PatCount", "Inquiries", "InqCount", "Prospects", "ProsCount");	
	}
	else {
		GraphByDate(DATEPatients, "Patients", "PatCount", "Prospects", "ProsCount");	
	}
}

void CMarketDateDlg::OnShowInquiries() {
//	UpdateView();
	ResetGraph(true);
}


void CMarketDateDlg::OnCancellations() {
	if (LastChecked(IDC_CANCELLATIONS3)) return;
	
	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
	ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	if(GetMainFrame()->m_pDocToolBar->GetType() != DATENoShowCancel) {	
		SetType(DATENoShowCancel);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	ResetGraph();
}

void CMarketDateDlg::Cancellations() {
	m_graph->Format = "%0.0f";
	m_graph->PrintTitle = "Cancellations And No Shows By Date";

	GraphByDate(DATENoShowCancel, "No Shows", "NumNoShows", "Cancellations", "NumCons");	
}

void CMarketDateDlg::OnInqToCons() {
	if (LastChecked(IDC_INQ_TO_CONS_BY_PROC3)) return;

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
	ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	if(GetMainFrame()->m_pDocToolBar->GetType() != DATEInqToCons) {
		SetType(DATEInqToCons);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	ResetGraph();
}

void CMarketDateDlg::InqToCons() {
	m_graph->Format = "%0.0f%%";

	m_graph->PrintTitle = "Inquiry To Consult By Date";

	GraphByDate(DATEInqToCons, "Percentage", "ProsPercent");	
}

void CMarketDateDlg::OnProsToCons() {
	if (LastChecked(IDC_PROS_TO_CONS_BY_PROC3)) return;

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
	ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

	// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
	GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_HIDE);
	// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
	GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_HIDE);

	if(GetMainFrame()->m_pDocToolBar->GetType() != DATEProsToCons) {
		SetType(DATEProsToCons);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
		
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
	ResetGraph();
}

void CMarketDateDlg::ProsToCons() {
	m_graph->Format = "%0.0f%%";

	m_graph->PrintTitle = "Prospects To Consults By Date";

	GraphByDate(DATEProsToCons, "Percentage", "ProsPercent");	
}



void CMarketDateDlg::GraphByDate(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2/*=""*/, CString strField2/*=""*/, CString strDesc3 /*=""*/, CString strField3 /*= ""*/) {

	try {
	
		GraphDescript desc;
		CString strLocationField, sql, from, to, strCat2, strProvFilter1, strProvFilter2, strResp2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strFrom2, strTo2, strPatFilterFilter, PatFilterIDs, strCat, strProvField, strResp = "";
		int nCategoryID, nResp;
		CString referrals;

		//disable the Show Completed only checkbox, it doesn't apply to this view
		//GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_SHOW);

		//show the appt date filter
		//AddApptDate();

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
			strPatFilterFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		CString strFrom, strTo, strFrom3 = "", strTo3 = "", strFrom4, strTo4;
		if (!strDateField.IsEmpty()) {


			if (mktType == DATERevByDate) {
				strFrom2.Format(" AND LineRefundsT.Date >= '%s' ", from);
				strTo2.Format(" AND LineRefundsT.Date < DATEADD(day,1,'%s') ", to);

				if (GetMainFrame()->m_pDocToolBar->GetFilterField(mftDate) == "LineChargesT.Date") {
					strFrom3.Format(" AND (1 = -1) ");
					strTo3.Format(" AND (1 = -1)");

					strFrom4 = " AND (1 = -1) ";
					strTo4 = " AND (1 = -1) ";
				}
				else {
					strFrom3 = "";
					strTo3 = "";

					strFrom4.Format(" AND %s >= '%s' ", strDateField, from);
					strTo4.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
				}
			}

			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		if (nCategoryID != -1) {
			strCat.Format(" AND ServiceT.Category = %i ", nCategoryID);
			strCat2.Format(" AND (1 = -1) ");
		}
		
		if (nResp == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
			strResp2.Format(" AND (RefundsT.InsuredPartyID = -1 OR RefundsT.InsuredPartyID IS NULL)");
		}
		else if (nResp == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
			strResp2.Format(" AND RefundsT.InsuredPartyID <> -1 AND RefundsT.InsuredPartyID IS NOT NULL");
		}

		sql = GetGraphSql(mktType, 1, -1, pCon, strPatientTempTable);
		
		m_progress.SetPos(10);

		sql.Replace("[From]", strFrom);
		sql.Replace("[To]", strTo);
		sql.Replace("[Prov1]", strProvFilter1);
		sql.Replace("[Prov2]", strProvFilter2);
		sql.Replace("[Loc1]", strLocFilter1);
		sql.Replace("[Loc2]", strLocFilter2);
		sql.Replace("[PatFilter]", strPatFilterFilter);
		sql.Replace("[Category]", strCat);
		sql.Replace("[Category2]", strCat2);
		sql.Replace("[Resp]", strResp);
		sql.Replace("[Resp2]", strResp2);
		sql.Replace("[From2]", strFrom2);
		sql.Replace("[To2]", strTo2);
		sql.Replace("[From3]", strFrom3);
		sql.Replace("[To3]", strTo3);
		sql.Replace("[From4]", strFrom4);
		sql.Replace("[To4]", strTo4);

		// (j.gruber 2011-05-06 08:41) - PLID 43583 - Add an additional function for graph specific filters
		ReplaceGraphSpecificFilters(mktType, sql);

		desc.m_sql = sql;
		Save(sql);

		//set the timeout
		CIncreaseCommandTimeout ict(600);

		//calculate the summary
		CString strTempDesc1, strTempDesc2, strTempDesc3;
		if (strDesc3.IsEmpty()) {
			if (strDesc2.IsEmpty()) {
				
				if (strDescription == "Percentage") {

					//calculate it correctly
					//rsTotal = CreateRecordset("SELECT CASE WHEN Sum(ProsWithCons) = 0 THEN 0 ELSE Sum(ProsWithCons) END AS SumNum, CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE Sum(TotalPros) END AS SumDenom From (%s) BASE", sql);

					/*double dblNum, dblDenom, dblSum;
					
					dblNum = AdoFldDouble(rsTotal, "SumNum", 0);
					dblDenom = AdoFldDouble(rsTotal, "SumDenom", 0);

					if (dblDenom == 0) {
						dblSum = 0;
					}
					else {
						dblSum = (dblNum/dblDenom) * 100.0;				
					}*/
					
					strTempDesc1.Format("%s", strDescription);
					//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
					desc.Add(strTempDesc1, "ProsWithCons", strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue), "TotalPros", GraphDescript::GD_DIV);

				}
				else {
					
					strTempDesc1.Format("%s", strDescription);
					//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
					desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue));
				}
				
				
			}
			else {

				strTempDesc1.Format("%s", strDescription);
				strTempDesc2.Format("%s", strDesc2);
				//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
				desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue));
				desc.Add(strTempDesc2, strField2, strDesc2 == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightRed));
			}


		}
		else {
			
			strTempDesc1.Format("%s", strDescription);
			strTempDesc2.Format("%s", strDesc2);
			strTempDesc3.Format("%s", strDesc3);
			//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
			desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue));
			desc.Add(strTempDesc2, strField2, strDesc2 == "Revenue" ?RGB(43,200,15) : strDesc2 == "Inquiries" ?RGB(255,0,255) : GetMarketGraphColor(mgcBrightRed));
			// (j.gruber 2011-05-06 16:45) - PLID 38153
			if (strDesc3 == "Conversion Rate") {
				desc.Add(strTempDesc3, strField2, strDesc3 == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightPurple), strField, GraphDescript::GD_PERCENT);
			}
			else {
				desc.Add(strTempDesc3, strField3, strDesc3 == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightRed));
			}
			
		}


		//MessageBox(sql);
			
		GraphDate(desc);

		for (int i = 0; i < desc.Size(); i++) {
			m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			if (GetType() == DATERevByDate) {
				COleCurrency cy;
				// (b.spivey, January 25, 2012) - PLID 47757 - Convert directly from a double and round for international clients. 
				cy = AsCurrency(desc.dblTotal(i));
				RoundCurrency(cy); 
				// (z.manning 2008-06-18 16:21) - PLID 28601 - Hide decimal places on marketing graphs.
				strText.Format("%s (%s Total)", desc.Label(i), FormatCurrencyForInterface(cy, TRUE, TRUE, FALSE));
			//(e.lally 2009-09-16) PLID 35559 - Added support for percents
			}else if (desc.Op(i) == GraphDescript::GD_DIV || desc.Op(i) == GraphDescript::GD_PERCENT) {
				if (desc.dblTotal2(i) == 0) {
					strText.Format("%s (%s%% Total)", desc.Label(i), "0");
				}
				else {
					// (j.gruber 2011-05-05 17:31) - PLID 43586 - this needs to multiply by 100 here also for the percent
					double dbl;
					if(desc.Op(i) == GraphDescript::GD_PERCENT){
						dbl = (desc.dblTotal(i) * 100)/desc.dblTotal2(i);
					}
					else {
						dbl = desc.dblTotal(i)/desc.dblTotal2(i);
					}
					strText.Format("%s (%.0f%% Total)", desc.Label(i), dbl);
				}
			}
			else {
				strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
			}
			m_graph->ColumnText = (_bstr_t)strText;
			// (j.gruber 2011-05-05 17:32) - PLID 43586 - this is already set in the graphDate function, so don't overwrite it here
			//m_graph->ColumnFormat = m_graph->Format;
		}
	


		//set it back 
		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error loading graph.");
}


void CMarketDateDlg::GraphDate(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsDate = NULL,
						rs = NULL;
		int				currentID = 0, id = 0, i, 
						j, 
						k,
						max;
		double			total, total2;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		m_progress.SetPos(20);

		sql.Format ("SELECT SubQ.MonthYear, Left(SubQ.MonthYear, Len(MonthYear) - 4) AS MonthNum, Right(SubQ.MonthYear, 4) AS YearNum, DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name FROM (%s) SubQ", desc.m_sql);

		//MessageBox(sql);

		rsDate = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		//max = rsDate->RecordCount;
		max = CalcDateRowCount(rsDate);

		m_graph->RowCount = max;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(25);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			if(desc.Label(i).Find("Conversion Rate") != -1)
				m_graph->ColumnFormat = "%0.0f%%";
			else
				m_graph->ColumnFormat = m_graph->Format;
		}

		//JMJ 4/2/2004 - The graph auto-reverts the sort to the first column
		//whenever the column descriptions change, so normally we don't need
		//to identify if we're loading a new graph and revert the sort, etc.
		//But in this case, the default sort is the X-Axis, so we have to tell
		//the graph to change the sort, but ONLY if we're loading the "By Month"
		//graph for the first time!

		// JMM 4/17/03  This doesn't work quite right because it only sets the default to be month when  you first go into the module
		// I fixed it to set it every time but the problem is now if you change the sort and then go back it switches back to month, 
		// I think thats the lesser of 2 eils for the moment.


		//DRT 5/10/2006 - PLID 20544 - This is a _bstr_t, NOT a BSTR!  The conversion here was causing problems on the heap.  I had it fixed, 
		//	but we actually don't even use this value, so I just removed it.
		//_bstr_t strDesc = m_graph->XAxisDesc;

//		if() {
			//sort by the x-axis and all records, because we are just now switching to this graph type
			m_graph->SortColumn = -1;
			m_graph->XAxisDesc = "Month";
//		}
		m_graph->XAxisSortStyle = cgSortIDMonthYear;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);

		//JMJ 4/27/2004 - keep the min/max months, and later in this function we will go through
		//and make sure every month in between is represented

		//keep each segment - easier/faster than parsing all the time
		long minMonth = 99;
		long minYear = 9999;
		long maxMonth = 0;
		long maxYear = 0;
		long minMonthYear = 999999;
		long maxMonthYear = 0;
		
		//all the real work is done, but we still have to sort the results
		i = 0;
		while (!rsDate->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsDate->Fields->GetItem("Name")->Value);
			long nMonthYear = atol(AdoFldString(rsDate, "MonthYear"));
			m_graph->RowID = nMonthYear;

			long nMonth = atol(AdoFldString(rsDate, "MonthNum"));
			long nYear = atol(AdoFldString(rsDate, "YearNum"));

			//now compare against the min and max
			if((nMonth < minMonth && nYear <= minYear) || nYear < minYear) {
				minMonth = nMonth;
				minYear = nYear;
				minMonthYear = nMonthYear;
			}
			if((nMonth > maxMonth && nYear >= maxYear) || nYear > maxYear) {
				maxMonth = nMonth;
				maxYear = nYear;
				maxMonthYear = nMonthYear;
			}

			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = nMonthYear;
			descendants.Add(currentID);
			/*if (currentID != id)//special [Other] column
				Descendants(descendants);
			else m_graph->RowText = "[Other]";*/

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
						{	if (descendants[k] == atoi(AdoFldString(rs, "MonthYear")))
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
						m_graph->Value = total;
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						break;
					case GraphDescript::GD_DIV:
					//(e.lally 2009-09-16) PLID 35559 - Added support for percents
					case GraphDescript::GD_PERCENT:
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
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
			rsDate->MoveNext();
			m_progress.SetPos(50 + ((50 * i) / max));
		}

		//JMJ 4/27/2004 - now add in any months we missed
		for(int x=minYear; x<=maxYear; x++) {
			for(int y=1; y<=12; y++) {
				//make sure we are in range
				if(minYear == x && minMonth > y)
					continue;

				if(maxYear == x && maxMonth < y)
					continue;

				int nNewMonthYear = (y*10000) + x;

				//now check to see if this month/year combo exists
				BOOL bFound = FALSE;
				for(int z=0;z<m_graph->RowCount && !bFound;z++) {
					m_graph->Row = z;					
					long nMonthYearToCompare = m_graph->RowID;

					if(nNewMonthYear == nMonthYearToCompare)
						bFound = TRUE;
				}

				//it doesn't exist, so add it
				if(!bFound) {
					m_graph->Row = i++;
					CString strMonthYearName, strMonth, strYear;
					strYear.Format("%li",x);
					strMonth = GetMonth(y);
					strMonthYearName.Format("%s - %s",strMonth,strYear);
					m_graph->RowText = (LPCTSTR) _bstr_t(strMonthYearName);
					m_graph->RowID = nNewMonthYear;
					m_graph->RowDrillDown = FALSE;

					//for each column
					for (int z = 0; z < desc.Size(); z++) {
						m_graph->Column = z;
						m_graph->Value = 0;
					}
				}
			}
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}

long CMarketDateDlg::CalcDateRowCount(_RecordsetPtr rsDate)
{
	if (rsDate->eof) {
		return 0;
	}
	long max = rsDate->RecordCount;

	//calculate how many extra blank dates we will be adding

	long minMonth = 99;
	long minYear = 9999;
	long maxMonth = 0;
	long maxYear = 0;
	long minMonthYear = 999999;
	long maxMonthYear = 0;

	CDWordArray aryIDs;

	rsDate->MoveFirst();
	while (!rsDate->eof) {
		long nMonthYear = atol(AdoFldString(rsDate, "MonthYear"));
		long nMonth = atol(AdoFldString(rsDate, "MonthNum"));
		long nYear = atol(AdoFldString(rsDate, "YearNum"));

		//now compare against the min and max
		if((nMonth < minMonth && nYear <= minYear) || nYear < minYear) {
			minMonth = nMonth;
			minYear = nYear;
			minMonthYear = nMonthYear;
		}
		if((nMonth > maxMonth && nYear >= maxYear) || nYear > maxYear) {
			maxMonth = nMonth;
			maxYear = nYear;
			maxMonthYear = nMonthYear;
		}

		aryIDs.Add(nMonthYear);

		rsDate->MoveNext();
	}

	for(int i=minYear; i<=maxYear; i++) {
		for(int j=1; j<=12; j++) {
			//make sure we are in range
			if(minYear == i && minMonth > j)
				continue;

			if(maxYear == i && maxMonth < j)
				continue;

			int nNewMonthYear = (j*10000) + i;

			//now check to see if this month/year combo exists
			BOOL bFound = FALSE;
			for(int k=0;k<aryIDs.GetSize() && !bFound;k++) {				
				long nMonthYearToCompare = (long)aryIDs.GetAt(k);

				if(nNewMonthYear == nMonthYearToCompare)
					bFound = TRUE;
			}

			//it doesn't exist, so add it
			if(!bFound) {
				max++;
			}
		}
	}

	rsDate->MoveFirst();

	return max;
}


// (j.gruber 2007-03-21 10:01) - PLID 25260 - added filter strings to graphs
CString CMarketDateDlg::GetProcedureNamesFromIDString(CString strIDs) {

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
		long nRow = m_pApptPurposeList->FindByColumn(0, (long)atoi(strIDs), 0, FALSE);
		if (nRow != -1) {
			return VarString(m_pApptPurposeList->GetValue(nRow, 1));
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
		long p = m_pApptPurposeList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while (p) {
			m_pApptPurposeList->GetNextRowEnum(&p, &lpDisp);
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

// (j.gruber 2007-03-21 10:01) - PLID 25260 - added filter strings to graphs
CString	CMarketDateDlg::GetPurposeFilterString() {
	//(e.lally 2009-09-11) PLID 35521 - This wasn't working for multiple procedures, so I updated it to be like the procedure tab.
	if (!m_strPurposeList.IsEmpty()) {
		return "Purposes: " + GetProcedureNamesFromIDString(m_strPurposeList);
	}
	else {
		return "All Purposes";
	}
}

//(e.lally 2009-09-11) PLID 35521 - Used as the local function to be used to replace the consult procedure filter placeholder
	//in the sql graph query.
//Filters using detailed appointment purposes.
//TODO: this should be replaced with a shared function.
CString CMarketDateDlg::GetConsToProcProcedureFilter()
{
	CString strOut;
	if(!m_strPurposeList.IsEmpty()){
		strOut.Format(" AND ProcedureT.ID IN %s ", m_strPurposeList);
	}
	return strOut;
}

// (j.gruber 2011-05-06 16:46) - PLID 43584
CString CMarketDateDlg::GetConvGroupFilter()
{
	CString strFilter;
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pConvGroupList->CurSel;
	if (pRow) {
		long nID = VarLong(pRow->GetValue(cgcID));
		if (nID != -1) {
			strFilter.Format(" AND ApptServiceConvGroupsT.ID = %li", nID);						
		}		
	}
	
	return strFilter;
}

// (j.gruber 2011-05-06 16:46) - PLID 43584
CString CMarketDateDlg::GetConvGroupFilterString()
{
	CString strFilter = "All Conversion Groups";
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pConvGroupList->CurSel;
	if (pRow) {
		long nID = VarLong(pRow->GetValue(cgcID));
		CString strName = VarString(pRow->GetValue(cgcName));
		if (nID != -1) {
			strFilter = "Conversion Group: " + strName;
		}		
	}
	
	return strFilter;
}

void CMarketDateDlg::Print(CDC *pDC, CPrintInfo *pInfo)
{
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

				//if this is the conversion rate graph then do the purposes also!
				if (pDoc->GetType() == CNVConstoSurgByDate) {
					//we can put it in the category filter since we don't use that for this graph,
					//if we ever make this filter global, then we should put a variable for it in the 
					//column graph
					m_graph->CategoryFilter = _bstr_t(GetPurposeFilterString());
				}

				// (j.gruber 2011-05-09 11:56) - PLID 43583
				if (pDoc->GetType() == DATEApptToCharge) {
					m_graph->CategoryFilter = _bstr_t(GetConvGroupFilterString());
				}

			}
		}

		m_graph->Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
	}
}

// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
void CMarketDateDlg::GetCurrentGraphFilters(CString &strFilter1,  CString &strFilter2, CString &strFilter3, CString &strFilter4, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable) {

	CString sql, from, to, strFrom2, strTo2, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilterFilter, PatFilterIDs, strCat, strResp = "", strLocationField, strProvField, strResp2, strCat2;
	int nCategoryID, nResp;
	CString referrals;

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
		strPatFilterFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

	CString strFrom, strTo, strFrom3, strTo3, strFrom4, strTo4;
	if (!strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	if ((MarketGraphType)GetType() == DATERevByDate) {
		if (nCategoryID != -1) {
			strCat.Format(" AND ServiceT.Category = %i ", nCategoryID);
			strCat2.Format(" AND (1 = -1) ");
		}
		
		if (nResp == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
			strResp2.Format(" AND (RefundsT.InsuredPartyID = -1 OR RefundsT.InsuredPartyID IS NULL)");
		}
		else if (nResp == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
			strResp2.Format(" AND RefundsT.InsuredPartyID <> -1 AND RefundsT.InsuredPartyID IS NOT NULL");
		}

		//PLID 17620: this should only happens if they are filtering on date
		if (! strDateField.IsEmpty()) {
			strFrom2.Format(" AND LineRefundsT.Date >= '%s' ", from);
			strTo2.Format(" AND LineRefundsT.Date < DATEADD(day,1,'%s') ", to);

			if (GetMainFrame()->m_pDocToolBar->GetFilterField(mftDate) == "LineChargesT.Date") {
				strFrom3.Format(" AND (1 = -1) ");
				strTo3.Format(" AND (1 = -1)");

				strFrom4 = " AND (1 = -1) ";
				strTo4 = " AND (1 = -1) ";
			}
			else {
				strFrom3 = "";
				strTo3 = "";

				strFrom4.Format(" AND %s >= '%s' ", strDateField, from);
				strTo4.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
			}
		}
		
		strFilter1 = " [Resp] [PatFilter] [Category] [From] [To] ";
		strFilter2 = " [Prov1] [Loc1]  ";
		strFilter3 = " [Resp2] [PatFilter] [Category2] [From2] [To2] [From3] [To3] ";
		strFilter4 = " [Resp] [PatFilter] [Category2] [From4] [To4] ";
	}
	else {
		strFilter1 = " [From] [To] [Prov1] [Loc1] [PatFilter] [Category] [Resp] ";
		strFilter2 = " [Loc2]  [Prov2] ";
	}

	strFilter1.Replace("[From]", strFrom);
	strFilter1.Replace("[To]", strTo);
	strFilter1.Replace("[From2]", strFrom2);
	strFilter1.Replace("[To2]", strTo2);
	strFilter1.Replace("[From3]", strFrom3);
	strFilter1.Replace("[To3]", strTo3);
	strFilter1.Replace("[From4]", strFrom4);
	strFilter1.Replace("[To4]", strTo4);
	strFilter1.Replace("[Prov1]", strProvFilter1);
	strFilter1.Replace("[Prov2]", strProvFilter2);
	strFilter1.Replace("[Loc1]", strLocFilter1);
	strFilter1.Replace("[Loc2]", strLocFilter2);
	strFilter1.Replace("[PatFilter]", strPatFilterFilter);
	strFilter1.Replace("[Category]", strCat);
	strFilter1.Replace("[Category2]", strCat2);
	strFilter1.Replace("[Resp]", strResp);
	strFilter1.Replace("[Resp2]", strResp2);


	strFilter2.Replace("[From]", strFrom);
	strFilter2.Replace("[To]", strTo);
	strFilter2.Replace("[From2]", strFrom2);
	strFilter2.Replace("[To2]", strTo2);
	strFilter2.Replace("[Prov1]", strProvFilter1);
	strFilter2.Replace("[Prov2]", strProvFilter2);
	strFilter2.Replace("[Loc1]", strLocFilter1);
	strFilter2.Replace("[Loc2]", strLocFilter2);
	strFilter2.Replace("[PatFilter]", strPatFilterFilter);
	strFilter2.Replace("[Category]", strCat);
	strFilter2.Replace("[Category2]", strCat2);
	strFilter2.Replace("[Resp]", strResp);
	strFilter2.Replace("[Resp2]", strResp2);
	strFilter2.Replace("[From3]", strFrom3);
	strFilter2.Replace("[To3]", strTo3);
	strFilter2.Replace("[From4]", strFrom4);
	strFilter2.Replace("[To4]", strTo4);

	strFilter3.Replace("[From]", strFrom);
	strFilter3.Replace("[To]", strTo);
	strFilter3.Replace("[From2]", strFrom2);
	strFilter3.Replace("[To2]", strTo2);
	strFilter3.Replace("[Prov1]", strProvFilter1);
	strFilter3.Replace("[Prov2]", strProvFilter2);
	strFilter3.Replace("[Loc1]", strLocFilter1);
	strFilter3.Replace("[Loc2]", strLocFilter2);
	strFilter3.Replace("[PatFilter]", strPatFilterFilter);
	strFilter3.Replace("[Category]", strCat);
	strFilter3.Replace("[Category2]", strCat2);
	strFilter3.Replace("[Resp]", strResp);
	strFilter3.Replace("[Resp2]", strResp2);
	strFilter3.Replace("[From3]", strFrom3);
	strFilter3.Replace("[To3]", strTo3);
	strFilter3.Replace("[From4]", strFrom4);
	strFilter3.Replace("[To4]", strTo4);

	strFilter4.Replace("[From]", strFrom);
	strFilter4.Replace("[To]", strTo);
	strFilter4.Replace("[From2]", strFrom2);
	strFilter4.Replace("[To2]", strTo2);
	strFilter4.Replace("[Prov1]", strProvFilter1);
	strFilter4.Replace("[Prov2]", strProvFilter2);
	strFilter4.Replace("[Loc1]", strLocFilter1);
	strFilter4.Replace("[Loc2]", strLocFilter2);
	strFilter4.Replace("[PatFilter]", strPatFilterFilter);
	strFilter4.Replace("[Category]", strCat);
	strFilter4.Replace("[Category2]", strCat2);
	strFilter4.Replace("[Resp]", strResp);
	strFilter4.Replace("[Resp2]", strResp2);
	strFilter4.Replace("[From3]", strFrom3);
	strFilter4.Replace("[To3]", strTo3);
	strFilter4.Replace("[From4]", strFrom4);
	strFilter4.Replace("[To4]", strTo4);

}

//This is a special function to get the By Date sql for printing because we need the sql with all the filters and everything and there is
//no other way that I can think of at the moment to do it
// (z.manning 2009-09-08 14:29) - PLID 35051 - Dead code
/*
CString CMarketDateDlg::GetConversionPrintSql() {

	CString strSql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1,strLocFilter2, locIDs, strDateField, strPatFilterFilter, PatFilterIDs, strLocationField, strProvField;
		int nCategoryID, nResp;
		
		//setup parameters that will be used in the query
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp);
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
			strPatFilterFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


		//figure out what their values are from ConfigRT
		BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);

		CString strConsIDList = GetRemotePropertyText("CRGConsultList", "", 0, "<None>", TRUE);
		CString strSurgIDList = GetRemotePropertyText("CRGSurgeryList", "", 0, "<None>", TRUE);

		CString strBaseSql, strExtraTypeSql;

		CString strYearFilter = AsString(m_pYearFilter->GetValue(m_pYearFilter->CurSel, 0));

		CString strCons1Label, strCons2Label, strTotalLabel, strSurgeryLabel;


		CString strPurpose, strPurposeOuter;
		m_strPurposeList = GetRemotePropertyText("CRGPurposeList", "", 0, "<None>", TRUE);
			
		if (m_strPurposeList.IsEmpty()) {

			strPurpose = "";
			strPurposeOuter = "";
		}
		else {
			strPurpose.Format("AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN %s)", m_strPurposeList);
			strPurposeOuter.Format("AND ApptsOuterQ.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN %s)", m_strPurposeList);
		}
		


		if (!bSplitConsults) {


			strBaseSql.Format("	SELECT DateName(Month, CAST(Left(ConsultsQ.MonthYear, Len(ConsultsQ.MonthYear) - 4)+'/1/'+ Right(ConsultsQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(ConsultsQ.MonthYear, 4)  AS Name, "
				" ConsultsQ.MonthYear, ConsultsQ.UserDefinedID, ConsultsQ.First,  ConsultsQ.Middle, ConsultsQ.Last, ConsultsQ.Address1, ConsultsQ.Address2, "
				" ConsultsQ.City, ConsultsQ.State, ConsultsQ.Zip, ConsultsQ.HomePhone, "
				" ConsultsQ.WorkPhone, ConsultsQ.CellPhone, ConsultsQ.UnionType, CASE WHEN SurgsWithConsQ.PersonID IS NOT NULL THEN 1 else 0 END As HadSurgery "
				"  FROM   "
				"  ( "
				"   SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, PatientsT.PersonID, "
				"  	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone,  "
				"  	1 as UnionType  "
				" 	 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				" 	 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  	 [From] [To] [Prov1] [Loc1] [PatFilter]  "
				"  GROUP BY AppointmentsT.ID, PatientsT.PersonID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))),  "
				" 	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone) "
				" ConsultsQ "
				" LEFT JOIN "
				"  (SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, PatientsT.PersonID, "
				"  	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				" 	 FROM AppointmentsT ApptsOuterQ LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID   "
				"  	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  	 LEFT JOIN AppointmentPurposeT AptPurposeOuterQ ON ApptsOuterQ.ID = AptPurposeOuterQ.AppointmentID  "
				"	 INNER JOIN ProcedureT ON AptPurposeOuterQ.PurposeID = ProcedureT.ID "
				"  	 WHERE ApptsOuterQ.Status <> 4 AND ApptsOuterQ.ShowState <> 3 AND PersonT.ID IN (  "
				"  	 	 SELECT PatientsT.PersonID  "
				"  		 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  		 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
				"  		 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND  "
				"  		 AppointmentsT.Date <= ApptsOuterQ.Date  "
				"  		 AND (AppointmentPurposeT.PurposeID = AptPurposeOuterQ.PurposeID OR	  "
				"		 AppointmentPurposeT.PurposeID = ProcedureT.MasterProcedureID ) "
				"  		 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  		 [From] [To] [Prov1] [Loc1] [PatFilter]   "
				"  	)  "
				"     AND AptTypeID IN %s  %s    "
				" GROUP BY ApptsOuterQ.ID, PatientsT.PersonID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))), UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				" )SurgsWithConsQ ON ConsultsQ.PersonID = SurgsWithConsQ.PersonID ",
				strConsIDList, strPurpose, strConsIDList, strPurpose, strSurgIDList, strPurposeOuter);



				/*
				"	 SELECT MonthYear, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip,"
				"    HomePhone, WorkPhone, CellPhone, 3 AS UnionType "
				"	 FROM "
				" 	(SELECT LTRIM(STR(DATEPART(MM, ConsultsQ.Date))) + LTRIM(STR(DATEPART(YYYY, ConsultsQ.Date))) AS MonthYear, "
				"    ConsultsQ.UserDefinedID, ConsultsQ.First, ConsultsQ.Middle, ConsultsQ.Last, ConsultsQ.Address1, ConsultsQ.Address2, "
				"	 ConsultsQ.City, ConsultsQ.State, ConsultsQ.Zip, ConsultsQ.HomePhone, ConsultsQ.WorkPhone, ConsultsQ.CellPhone "
				" 	 FROM  "
				" 	 ( "
				"  	 	SELECT ApptsOuterQ.ID, ApptsOuterQ.PatientID, AptPurposeOuterQ.PurposeID, ApptsOuterQ.Date "
				" 	 	 FROM AppointmentsT ApptsOuterQ LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID   "
				"  		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				" 	 	 LEFT JOIN AppointmentPurposeT AptPurposeOuterQ ON ApptsOuterQ.ID = AptPurposeOuterQ.AppointmentID  "
				" 	 	 WHERE ApptsOuterQ.Status <> 4 AND ApptsOuterQ.ShowState <> 3  "
				" 		 AND AptTypeID IN %s  %s   "
				" 	 ) SurgeriesQ "
				" 	 INNER JOIN ( "
				" 		 SELECT PatientsT.PersonID AS PatientID, AppointmentsT.Date, AppointmentPurposeT.PurposeID,  "
				"		 UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				"  		 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  		 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
				"  		 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3  "
				"  		 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  		 [From] [To] [Prov1] [Loc1] [PatFilter]   "
				"  	) ConsultsQ ON SurgeriesQ.PatientID = ConsultsQ.PatientID "
				" 	AND SurgeriesQ.PurposeID = ConsultsQ.PurposeID "
				" 	WHERE ConsultsQ.Date <= SurgeriesQ.Date) InnerQ "
				"  	 GROUP BY MonthYear, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				"   ) SubQ  ",
				strConsIDList, strPurpose, strSurgIDList, strPurposeOuter, strConsIDList, strPurpose);
			*/
/*
			CString strFrom, strTo;
			if (!strDateField.IsEmpty()) {
				strFrom.Format(" AND %s >= '%s' ", strDateField, from);
				strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
			}


			
			strBaseSql.Replace("[From]", strFrom);
			strBaseSql.Replace("[To]", strTo);
			strBaseSql.Replace("[Prov1]", strProvFilter1);
			strBaseSql.Replace("[Prov2]", strProvFilter2);
			strBaseSql.Replace("[Loc1]", strLocFilter1);
			strBaseSql.Replace("[PatFilter]", strPatFilterFilter);
			strBaseSql.Replace("[Loc2]", strLocFilter2);
		}
		else {

			CString strCons1Tmp, strCons2Tmp;

			long nResult = strConsIDList.Find("---");
			strCons1Tmp = strConsIDList.Left(nResult);

			strCons2Tmp = strConsIDList.Right(strConsIDList.GetLength() - (nResult + 3));

			
			strBaseSql.Format("SELECT * FROM ( "
				" SELECT DateName(Month, CAST(Left(ConsultsQ.MonthYear, Len(ConsultsQ.MonthYear) - 4)+'/1/'+ Right(ConsultsQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(ConsultsQ.MonthYear, 4)  AS Name, "
				" ConsultsQ.MonthYear, ConsultsQ.UserDefinedID, ConsultsQ.First,  ConsultsQ.Middle, ConsultsQ.Last, ConsultsQ.Address1, ConsultsQ.Address2, "
				" ConsultsQ.City, ConsultsQ.State, ConsultsQ.Zip, ConsultsQ.HomePhone, "
				" ConsultsQ.WorkPhone, ConsultsQ.CellPhone, ConsultsQ.UnionType, CASE WHEN SurgsWithConsQ.PersonID IS NOT NULL THEN 1 else 0 END As HadSurgery "
				"  FROM   "
				"  ( "
				"   SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, PatientsT.PersonID, "
				"  	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone,  "
				"  	1 as UnionType  "
				" 	 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				" 	 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  	 [From] [To] [Prov1] [Loc1] [PatFilter]  "
				"  GROUP BY AppointmentsT.ID, PatientsT.PersonID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))),  "
				" 	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone) "
				" ConsultsQ "
				" LEFT JOIN "
				"  (SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, PatientsT.PersonID, "
				"  	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				" 	 FROM AppointmentsT ApptsOuterQ LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID   "
				"  	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  	 LEFT JOIN AppointmentPurposeT AptPurposeOuterQ ON ApptsOuterQ.ID = AptPurposeOuterQ.AppointmentID  "
				"	 LEFT JOIN ProcedureT ON AptPurposeOuterQ.PurposeID = ProcedureT.ID "
				"  	 WHERE ApptsOuterQ.Status <> 4 AND ApptsOuterQ.ShowState <> 3 AND PersonT.ID IN (  "
				"  	 	 SELECT PatientsT.PersonID  "
				"  		 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  		 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
				"  		 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND  "
				"  		 AppointmentsT.Date <= ApptsOuterQ.Date  "
				"  		 AND (AppointmentPurposeT.PurposeID = AptPurposeOuterQ.PurposeID	  "
				"		 OR AppointmentPurposeT.PurposeID = ProcedureT.MasterProcedureID ) "
				"  		 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  		 [From] [To] [Prov1] [Loc1] [PatFilter]   "
				"  	)  "
				"     AND AptTypeID IN %s  %s    "
				" GROUP BY ApptsOuterQ.ID, PatientsT.PersonID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))), UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				" )SurgsWithConsQ ON ConsultsQ.PersonID = SurgsWithConsQ.PersonID "
				" UNION ALL "
					" SELECT DateName(Month, CAST(Left(Consults2Q.MonthYear, Len(Consults2Q.MonthYear) - 4)+'/1/'+ Right(Consults2Q.MonthYear, 4) AS DATETIME)) + ' - ' + Right(Consults2Q.MonthYear, 4)  AS Name, "
				" Consults2Q.MonthYear, Consults2Q.UserDefinedID, Consults2Q.First,  Consults2Q.Middle, Consults2Q.Last, Consults2Q.Address1, Consults2Q.Address2, "
				" Consults2Q.City, Consults2Q.State, Consults2Q.Zip, Consults2Q.HomePhone, "
				" Consults2Q.WorkPhone, Consults2Q.CellPhone, Consults2Q.UnionType, CASE WHEN SurgsWithCons2Q.PersonID IS NOT NULL THEN 1 else 0 END As HadSurgery "
				"  FROM   "
				"  ( "
				"   SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, PatientsT.PersonID, "
				"  	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone,  "
				"  	2 as UnionType  "
				" 	 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				" 	 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  	 [From] [To] [Prov1] [Loc1] [PatFilter]  "
				"  GROUP BY AppointmentsT.ID, PatientsT.PersonID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))),  "
				" 	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone) "
				" Consults2Q "
				" LEFT JOIN "
				"  (SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, PatientsT.PersonID, "
				"  	UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				" 	 FROM AppointmentsT ApptsOuterQ LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID   "
				"  	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  	 LEFT JOIN AppointmentPurposeT AptPurposeOuterQ ON ApptsOuterQ.ID = AptPurposeOuterQ.AppointmentID  "
				"	 LEFT JOIN ProcedureT ON AptPurposeOuterQ.PurposeID = ProcedureT.ID "
				"  	 WHERE ApptsOuterQ.Status <> 4 AND ApptsOuterQ.ShowState <> 3 AND PersonT.ID IN (  "
				"  	 	 SELECT PatientsT.PersonID  "
				"  		 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID   "
				"  		 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
				"  		 LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID  "
				"  		 WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND  "
				"  		 AppointmentsT.Date <= ApptsOuterQ.Date  "
				"  		 AND (AppointmentPurposeT.PurposeID = AptPurposeOuterQ.PurposeID	  "
				"		  OR AppointmentPurposeT.PurposeID = ProcedureT.MasterProcedureID) "
				"  		 [Loc2] [Prov2] AND AptTypeID IN %s %s   "
				"  		 [From] [To] [Prov1] [Loc1] [PatFilter]   "
				"  	)  "
				"     AND AptTypeID IN %s  %s    "
				" GROUP BY ApptsOuterQ.ID, PatientsT.PersonID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))), UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone "
				" )SurgsWithCons2Q ON Consults2Q.PersonID = SurgsWithCons2Q.PersonID "
				" ) Query ",				
				strCons1Tmp, strPurpose, strCons1Tmp, strPurpose, strSurgIDList, strPurposeOuter,
				strCons2Tmp, strPurpose, strCons2Tmp, strPurpose, strSurgIDList, strPurposeOuter);
			
			
			
			/*strBaseSql.Format(" SELECT DateName(Month, CAST(Left(SubQ.MonthYear, Len(MonthYear) - 4)+'/1/'+ Right(SubQ.MonthYear, 4) AS DATETIME)) + ' - ' + Right(SubQ.MonthYear, 4)  AS Name, * "
				" FROM ( "
				" 	SELECT  LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, "
				"    UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, "
				"    1 UnionType  "
				" 	 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID  "
				"	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				"	 WHERE AppointmentsT.Status <> 4 [Loc2] [Prov2] AND  AptTypeID IN %s %s  "
				"	 [From] [To] [Prov1] [Loc1] [PatFilter] "
				"	 Group BY AppointmentsT.ID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))), "
				"        UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone  "
				"   UNION ALL  "
				"	 SELECT LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, "
				"         UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, "
				" 	2 AS UnionType "
				"	 FROM AppointmentsT  "
				"	 LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID  "
				"	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				"	 WHERE AppointmentsT.Status <> 4 [Loc2] [Prov2] AND  AptTypeID IN %s %s  "
				"	 [From] [To] [Prov1] [Loc1] [PatFilter]  "
				"	 Group BY AppointmentsT.ID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))), "
				"        UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone  "
				"	 UNION ALL  "
				"	 SELECT LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))) AS MonthYear, "
				"    UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone, "
				"	3  as UnionType "
				" 	 FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID  "
				" 	 LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
				"	 WHERE AppointmentsT.Status <> 4 [Loc2] [Prov2] AND  AptTypeID IN %s %s  "
				"	 [From] [To] [Prov1] [Loc1] [PatFilter]  "
				"	 Group BY AppointmentsT.ID, LTRIM(STR(DATEPART(MM, Date))) + LTRIM(STR(DATEPART(YYYY, Date))), "
				"    UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, HomePhone, WorkPhone, CellPhone ) SubQ", strCons1Tmp, strPurpose, strCons2Tmp, strPurpose, strSurgIDList, strPurpose);
			*/
/*
			CString strFrom, strTo;
			if (!strDateField.IsEmpty()) {
				strFrom.Format(" AND %s >= '%s' ", strDateField, from);
				strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
			}


			
			strBaseSql.Replace("[From]", strFrom);
			strBaseSql.Replace("[To]", strTo);
			strBaseSql.Replace("[Prov1]", strProvFilter1);
			strBaseSql.Replace("[Prov2]", strProvFilter2);
			strBaseSql.Replace("[Loc1]", strLocFilter1);
			strBaseSql.Replace("[Loc2]", strLocFilter2);
			strBaseSql.Replace("[PatFilter]", strPatFilterFilter);
		}

		return strBaseSql;

}*/

void CMarketDateDlg::OnGo() 
{
	if (!m_bRenderedOnce) {
		m_graph->Title = "";
	}
	m_bRenderedOnce = true;
	UpdateView();
}

void CMarketDateDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	if (!bShow) {
		LastChecked(-1);
	}

	CMarketingDlg::OnShowWindow(bShow, nStatus);	
}

void CMarketDateDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
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

bool CMarketDateDlg::LastChecked(int nID) 
{
	if ( (m_nLastChecked == nID) && m_bActive )
		return true;
	
	m_nLastChecked = nID;
	return false;
}

LRESULT CMarketDateDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	GetDlgItem(IDC_GO)->EnableWindow(true);	

	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}

void CMarketDateDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CMarketingDlg::OnMouseMove(nFlags, point);
}

// (j.gruber 2007-04-19 15:15) - PLID 25288 - adding label
BOOL CMarketDateDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (m_dwPurpIDList.GetSize() > 1) 
	{
		CRect rc;
		GetDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	
	return CMarketingDlg::OnSetCursor(pWnd, nHitTest, message);
}


// (j.gruber 2007-04-19 15:15) - PLID 25288
LRESULT CMarketDateDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
	case IDC_MARKET_DATE_MULTI_PURPOSE_LIST:
		if (SelectMultiPurposes()) {
			SetRemotePropertyText("CRGPurposeList", m_strPurposeList, 0, "<None>");
			ResetGraph();
		}
		break;
	
	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}

// (z.manning 2009-09-09 15:28) - PLID 35051 - Used to work around weird drawing issues with
// these radio buttons that are in the column graph area.
void CMarketDateDlg::InvalidateConsToProcRadioButtons()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_ALL_COLUMNS);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_NUMBERS_ONLY);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}

// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
void CMarketDateDlg::InvalidateShowInquiriesButton()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_INQUIRIES);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}

// (j.gruber 2011-05-06 16:47) - PLID 38153
void CMarketDateDlg::OnBnClickedApptToCharge()
{
	try {
		if (LastChecked(IDC_APPT_TO_CHARGE)) return;

		GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
		// (j.gruber 2007-05-09 14:58) - PLID 25288 - hide this since we aren't on the conversion tab
		ShowDlgItem(IDC_MARKET_DATE_MULTI_PURPOSE_LIST, SW_HIDE);
		GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

		GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);

		// (j.gruber 2011-05-03 16:06) - PLID 38153 - show the appt to charge config button
		GetDlgItem(IDC_CONFIGURE_APPT_TO_CHARGE)->ShowWindow(SW_SHOW);
		// (j.gruber 2011-05-06 09:25) - PLID 43583 - conversion group filter
		GetDlgItem(IDC_CONVERSION_GROUP_LIST)->ShowWindow(SW_SHOW);

		if(GetMainFrame()->m_pDocToolBar->GetType() != DATEApptToCharge) {
			SetType(DATEApptToCharge);
			SetFilter(mfApptDate, mftDate);
			SetFilter(mfApptLocation, mftLocation);
			SetFilter(mfApptProvider, mftProvider);
		}
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
		ResetGraph();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-06 16:48) - PLID 38153
void CMarketDateDlg::ApptsToCharges() {

	try {
		m_graph->Format = "%0.0f";
		m_graph->PrintTitle = "Appointments To Charges";
		GraphByDate(DATEApptToCharge, "Appointments", "NumAppts", "Converted Appointments", "NumCharges", "Conversion Rate", "ConvRate");
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-06 16:49) - PLID 43550
void CMarketDateDlg::OnBnClickedConfigureApptToCharge()
{
	try {
		CConfigureApptConvGroupsDlg dlg(this);
		dlg.DoModal();
		ResetGraph();

		// (j.gruber 2011-05-09 12:10) - PLID 43583//requery the conversion groups
		m_pConvGroupList->Requery();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-06 16:49) - PLID 43583
void CMarketDateDlg::SelChosenConversionGroupList(LPDISPATCH lpRow)
{
	try {
		ResetGraph();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-06 16:49) - PLID 43583
void CMarketDateDlg::RequeryFinishedConversionGroupList(short nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pConvGroupList->GetNewRow();
		pRow->PutValue(cgcID, (long) -1);
		pRow->PutValue(cgcName, _variant_t("<All Conversion Groups>"));
		m_pConvGroupList->AddRowBefore(pRow, m_pConvGroupList->GetFirstRow());

		m_pConvGroupList->CurSel = pRow;

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-05-06 16:49) - PLID 43583 - created for conversion group filter
void CMarketDateDlg::ReplaceGraphSpecificFilters(MarketGraphType mktType, CString &strSql) 
{
	try {

		switch (mktType) {

			// (j.gruber 2011-05-06 16:50) - PLID 43583
			case DATEApptToCharge:
				{
					//get the group filter
					NXDATALIST2Lib::IRowSettingsPtr pRow;
					pRow = m_pConvGroupList->CurSel;
					if (pRow) {
						long nID = VarLong(pRow->GetValue(cgcID));
						if (nID != -1) {
							CString strFilter;
							strFilter.Format(" AND ApptServiceConvGroupsT.ID = %li", nID);
							strSql.Replace("[GroupFilter]", strFilter);
							strFilter.Format(" AND ApptServiceConvTypesT.GroupID = %li", nID);
							strSql.Replace("[GroupFilterTypes]", strFilter);
							
						}
						else {
							strSql.Replace("[GroupFilter]", "");
							strSql.Replace("[GroupFilterTypes]", "");
						}
					}
					else {
						strSql.Replace("[GroupFilter]", "");
						strSql.Replace("[GroupFilterTypes]", "");
					}

				}
			break;
				
			default:
			break;		
		}
	}NxCatchAll(__FUNCTION__);
}