#include "stdafx.h"

// Effectiveness.cpp : implementation file
//
#include "MarketReferralSourceDlg.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "MsgBox.h"
#include "InternationalUtils.h"
#include "MarketUtils.h"
#include "MarketFilterPickerDlg.h"
#include "DocBar.h"
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

#define LOAD_SCREEN 124

//(a.wilson 2012-5-21) PLID 50378 - added inactive field.
enum ReferralListColumns {
	rlcID = 0,
	rlcName = 1,
	rlcInactive = 2,
};
/////////////////////////////////////////////////////////////////////////////
// CMarketReferralSourceDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketReferralSourceDlg::CMarketReferralSourceDlg(CWnd* pParent)
	: CMarketingDlg(CMarketReferralSourceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketReferralSourceDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/compare.htm";
	m_graph = NULL;

	SetFilter(mfFirstContactDate, mftDate);
	m_bIsLoading = FALSE;

	// (a.wilson 2012-5-22) PLID 50378 - default selection to -1.
	m_strFilteredReferralList = "";
	m_nCurrentFilteredReferralID = -1;
	m_dwFilteredRefIDList.RemoveAll();
}

CMarketReferralSourceDlg::~CMarketReferralSourceDlg()
{
	if (m_oldCursor)
		SetCursor(m_oldCursor);
	DestroyCursor(m_cursor);
}

void CMarketReferralSourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketReferralSourceDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_UP, m_Up);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress);
	DDX_Control(pDX, IDC_COLOR1, m_color1);
	DDX_Control(pDX, IDC_MONEY_BY_REFERRAL, m_MoneyByRefSourRad);
	DDX_Control(pDX, IDC_PATS_BY_REFERRAL, m_PatsByReferral);
	DDX_Control(pDX, IDC_INQ_BY_REFERRAL, m_InqsByReferral);
	DDX_Control(pDX, IDC_NO_SHOW_BY_REFERRAL3, m_NoShowsByReferral);
	DDX_Control(pDX, IDC_CONVERSION_RAD, m_conversionRad);
	DDX_Control(pDX, IDC_INQ_TO_CONS_BY_PROC, m_InqToCons);
	DDX_Control(pDX, IDC_PROS_TO_CONS_BY_PROC2, m_ProsToCons);
	DDX_Control(pDX, IDC_PROCS_CLOSED2, m_ProcsClosed);
	DDX_Control(pDX, IDC_SHOW_INQUIRIES, m_ShowInquiries);
	DDX_Control(pDX, IDC_MARKET_REF_SOURCE_LINK, m_nxlReferralLabel);
	DDX_Control(pDX, IDC_SHOW_ALL_COLUMNS_REF, m_ShowAllRad);
	DDX_Control(pDX, IDC_SHOW_NUMBERS_ONLY_REF, m_ShowNumberRad);
	DDX_Control(pDX, IDC_SHOW_PERCENTAGES_ONLY_REF, m_ShowPercentRad);
	//}}AFX_DATA_MAP
}


// (j.gruber 2009-06-23 11:47) - PLID 34227 - added label click message
BEGIN_MESSAGE_MAP(CMarketReferralSourceDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketReferralSourceDlg)
	ON_BN_CLICKED(IDC_MONEY_BY_REFERRAL, OnMoneyByRefSour)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_PATS_BY_REFERRAL, OnPatsByReferral)
	ON_BN_CLICKED(IDC_INQ_BY_REFERRAL, OnInqsByReferral)
	ON_BN_CLICKED(IDC_NO_SHOW_BY_REFERRAL3, OnNoShowsByReferral)
	ON_BN_CLICKED(IDC_CONVERSION_RAD, OnConversionRad)
	ON_BN_CLICKED(IDC_SHOW_INQUIRIES, OnShowInquiries)
	ON_BN_CLICKED(IDC_INQ_TO_CONS_BY_PROC, OnInqToCons)
	ON_BN_CLICKED(IDC_PROCS_CLOSED2, OnProcsClosed)
	ON_BN_CLICKED(IDC_PROS_TO_CONS_BY_PROC2, OnProsToCons)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_SHOWWINDOW()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)	
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TYPES_REFERRAL, OnConfigureApptTypes)
	ON_BN_CLICKED(IDC_SHOW_ALL_COLUMNS_REF, OnShowAllColumns)
	ON_BN_CLICKED(IDC_SHOW_NUMBERS_ONLY_REF, OnShowNumbersOnly)
	ON_BN_CLICKED(IDC_SHOW_PERCENTAGES_ONLY_REF, OnShowPercentagesOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketReferralSourceDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketReferralSourceDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketReferralSourceDlg)
	ON_EVENT(CMarketReferralSourceDlg, IDC_EFF_GRAPH, 1 /* OnClickColumn */, OnOnClickColumnGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketReferralSourceDlg, IDC_EFF_GRAPH, 2 /* OnMouseMoveColumn */, OnOnMouseMoveColumnGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketReferralSourceDlg, IDC_EFF_GRAPH, 3 /* OnChangeBackButtonPos */, OnChangeBackButtonPosColumnGraph, VTS_NONE)	
	ON_EVENT(CMarketReferralSourceDlg, IDC_MARKET_REF_SOURCE_FILTER, 18, CMarketReferralSourceDlg::OnRequeryFinishedReferralSources, VTS_I2)
	ON_EVENT(CMarketReferralSourceDlg, IDC_MARKET_REF_SOURCE_FILTER, 16, CMarketReferralSourceDlg::OnSelChosenReferralSourceList, VTS_DISPATCH)
	ON_EVENT(CMarketReferralSourceDlg, IDC_MARKET_REF_SOURCE_FILTER, 1 /* SelChanging */, CMarketReferralSourceDlg::OnSelChangingReferralFilter, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CMarketReferralSourceDlg::OnInitDialog() 
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

	m_nCurrentDrilledDownReferral = -1;
	m_strCategory = "RootNode";
	m_bRenderedOnce = false;
	m_bActive = false;
	m_bGraphEmpty = true;
	
	m_Go.AutoSet(NXB_MARKET);

	m_cursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_EXPAND));
	m_Up.AutoSet(NXB_LEFT);
	m_Up.EnableWindow(FALSE);
	m_oldCursor = NULL;
	
	m_MoneyByRefSourRad.SetToolTip("Display the net amount of payments and refunds generated from referral sources. If a category is selected in the filters, only applied payments and refunds will be included. Otherwise, all payments and refunds are included."); 
	m_PatsByReferral.SetToolTip("Display the number of patients, prospects, and inquiries referred to the practice. This includes a breakdown of inquiries, and patients and prospects who scheduled consults and procedures.");
	m_InqsByReferral.SetToolTip("Display the number of patient inquiries generated from various referral sources.");
	m_NoShowsByReferral.SetToolTip("Display the number of appointment cancellations and No Shows from patients and prospects referred to the practice.");
	m_InqToCons.SetToolTip("Display the percentage of all inquiries that have progressed into consultations from various referral sources. The consultations include both patients and prospects.");
	m_ProsToCons.SetToolTip("Display the percentage of all prospects referred to the Practice who have scheduled consultations.");
	m_ProcsClosed.SetToolTip("Display the total number of procedures performed versus the number of procedures that are scheduled and have a prepayment associated through the Procedure Information Center. ");
	m_conversionRad.SetToolTip("Display the percentage of all patients and prospects referred to the Practice who have scheduled consultations and procedures."); 

	//set it to the Revenue by Ref Sour tab
	//let's let the user decide when to run the graph
	//m_MoneyByRefSourRad.SetCheck(1);
	
	//This will get called by UpdateView() at the appropriate time.
	//OnMoneyByRefSour();

	//in order not to call OnMonbyREfSour here, we have to do what it does without doing the graph because
	//update view will do that part for us
	//moved back into OnMoneyByRefSour

	m_nCurrentDrilledDownReferral = -1;

	m_graph->Title = "";
	m_bIsLoading = TRUE;

	// (j.gruber 2009-06-23 12:09) - PLID 34227 - initialize the datalist
	m_pReferralSourceList = BindNxDataList2Ctrl(IDC_MARKET_REF_SOURCE_FILTER);
	
	return FALSE;
}


void CMarketReferralSourceDlg::OnMoneyByRefSour() 
{
	if (LastChecked(IDC_MONEY_BY_REFERRAL)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051

	//TES 6/16/2004: Why are we resetting the current referral?
	/*m_nCurrentDrilledDownReferral = -1;
	m_Up.ShowWindow(FALSE);
	m_Up.EnableWindow(FALSE);
	m_graph->Title = "All";*/

	m_Up.ShowWindow(FALSE);
	m_Up.EnableWindow(FALSE);
	SetType(EFFMonByRefSour);
	SetFilter(mfPaymentDate, mftDate);
	m_graph->PrintTitle = "Revenue By Referrals";

	SetType(EFFMonByRefSour);
	SetFilter(mfPaymentDate, mftDate);
	SetFilter(mfTransLocation, mftLocation);
	SetFilter(mfTransProvider, mftProvider);
	m_graph->PrintTitle = "Revenue By Referrals";		

	EnsureFilter(mftDate);
	EnsureFilter(mftLocation);
	EnsureFilter(mftProvider);
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(0);

	ResetGraph();
}


void CMarketReferralSourceDlg::MoneyByRefSour() 
{	
	
	CWaitCursor wc;
	try {


		CString from, to, doc1, doc2, doc3, strLocFilter1, strLocFilter2, provIDs, strProvFilter1;
		CString locIDs, category, strDateField, strFrom, strTo, strPatFilter, PatFilterIDs, strResp, strResp2, strProvField;
		CString strLocationField, strCat2, strTo2, strFrom2, strFrom3, strTo3, strFrom4, strTo4;
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int nCatID=0, nRespID=0;
		CString strRefSourSql, strRefPhysSql, strRefPatSql;

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCatID, nRespID, pCon, strPatientTempTable);

		//PLID 16404 - let them choose which provider to filter on
		if (UseFilter(mftProvider) ) {
			strProvFilter1.Format(" AND %s %s) ", strProvField, provIDs);
		}

		//PLID 16175 - let them choose which location to filter on
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
		

		long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
		long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
		strRefSourSql = GetGraphSql(EFFMonByRefSour, 1, -1, pCon, strPatientTempTable);

		if (nRefPhyID > 0) {
			//this is an odd place for the replace, but we have to do it before its combined
			CString strTmp = GetGraphSql(EFFMonByRefSour, 1, -1, pCon, strPatientTempTable);
			CString str, str2;
			str.Format(" AND PatientsT.DefaultReferringPhyID IS NULL %s ", Descendants(nRefPhyID, "AND ReferralID")); 
			//str2.Format(" AND PatientsT.DefaultReferringPhyID IS NULL %s ", Descendants(nRefPhyID, "AND ReferralSource"));
			strTmp.Replace("[referrals]", str);
			strTmp.Replace("[referral2]", Descendants(nRefPhyID, "AND ReferralSource"));
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-2 AS ");
			strRefPhysSql =  strTmp + " UNION ALL " + GetGraphSql(EFFMonByRefSour, 2, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPhysSql = GetGraphSql(EFFMonByRefSour, 2, -1, pCon, strPatientTempTable);
		}


		if (nRefPatID > 0) {
			CString strTmp = GetGraphSql(EFFMonByRefSour, 1, -1, pCon, strPatientTempTable);
			//this is an odd place for the replace, but we have to do it before its combined
			CString str;/*, str2;*/
			str.Format(" AND PatientsT.ReferringPatientID IS NULL %s ", Descendants(nRefPatID, "AND ReferralID"));
			//str2.Format(" AND ReferralSource = %li OR ReferralSource IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);*/
			strTmp.Replace("[referrals]", str);
			strTmp.Replace("[referral2]", Descendants(nRefPatID, "AND ReferralSource"));
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-3 AS ");
			strRefPatSql =  strTmp + " UNION ALL " + GetGraphSql(EFFMonByRefSour, 3, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPatSql = GetGraphSql(EFFMonByRefSour, 3, -1, pCon, strPatientTempTable);
		}

		
		

	//	strDateField.Replace("LineChargesT.Date", "LineItemT.Date");
		
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
			
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

		if (nRespID == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
			strResp2.Format(" AND (RefundsT.InsuredPartyID = -1 OR RefundsT.InsuredPartyID IS NULL)");
		}
		else if (nRespID == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
			strResp2.Format(" AND RefundsT.InsuredPartyID <> -1 AND RefundsT.InsuredPartyID IS NOT NULL");
		}
		if (PatFilterIDs != "") 
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		if (nCatID != -1) {
			category.Format(" AND ServiceT.Category = %li ", nCatID);
			strCat2.Format(" AND (1 = -1) ");
		}

		//increment the to date
		COleDateTime dtTo;
		dtTo.ParseDateTime(to, VAR_DATEVALUEONLY);
		dtTo += COleDateTimeSpan(1,0,0,0);
		to = FormatDateTimeForSql(dtTo, dtoDate);


		strRefSourSql.Replace("[From]", strFrom);
		strRefSourSql.Replace("[To]", strTo);
		strRefSourSql.Replace("[From2]", strFrom2);
		strRefSourSql.Replace("[To2]", strTo2);
		strRefSourSql.Replace("[From3]", strFrom3);
		strRefSourSql.Replace("[To3]", strTo3);
		strRefSourSql.Replace("[From4]", strFrom4);
		strRefSourSql.Replace("[To4]", strTo4);
		strRefSourSql.Replace("[Prov]", strProvFilter1);
		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		strRefSourSql.Replace("[referrals]", (m_nCurrentDrilledDownReferral == -1) ? "AND ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND ReferralID"));
		strRefSourSql.Replace("[referral2]", (m_nCurrentDrilledDownReferral == -1) ? "AND ReferralSource IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND ReferralSource"));
		//strRefSourSql.Replace("[refphys]", (m_nCurrentDrilledDownReferral == -1) ? "AND RefPhysID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND RefPhysID"));
		//strRefSourSql.Replace("[refpat]", (m_nCurrentDrilledDownReferral == -1) ? "AND RefPatID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND RefPatID"));
		strRefSourSql.Replace("[Loc1]", strLocFilter1);
		strRefSourSql.Replace("[Category]", category);
		strRefSourSql.Replace("[Category2]", strCat2);
		strRefSourSql.Replace("[from]", "'" + from + "'");
		strRefSourSql.Replace("[to]", "'" + to + "'");
		strRefSourSql.Replace("[PatFilter]", strPatFilter);
		strRefSourSql.Replace("[Resp]", strResp);
		strRefSourSql.Replace("[Resp2]", strResp2);
		strRefSourSql.Replace("[Loc2]", strLocFilter2);
		strRefSourSql.Replace("[ReplacePref]", "");

		//PLID 14894: take out any referral sources of people that also have referring phys or referring pats
		CString strRefPatReplace;
		if (nRefPatID > 0) {
			//strRefPatReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);
			strRefPatReplace = Descendants(nRefPatID, "AND ReferralID NOT ");
		}
		else {
			strRefPatReplace = "";
		}
		
		CString strRefPhysReplace;
		if (nRefPhyID > 0) {
			//strRefPhysReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPhyID, nRefPhyID);
			strRefPhysReplace = Descendants(nRefPhyID, "AND ReferralID NOT ");
		}
		else {
			strRefPhysReplace = "";
		}
		strRefSourSql.Replace("[TakeOutForRefPhys]", strRefPhysReplace);
		strRefSourSql.Replace("[TakeOutForRefPat]", strRefPatReplace);


		strRefPhysSql.Replace("[From]", strFrom);
		strRefPhysSql.Replace("[To]", strTo);
		strRefPhysSql.Replace("[From2]", strFrom2);
		strRefPhysSql.Replace("[To2]", strTo2);
		strRefPhysSql.Replace("[From3]", strFrom3);
		strRefPhysSql.Replace("[To3]", strTo3);
		strRefPhysSql.Replace("[From4]", strFrom4);
		strRefPhysSql.Replace("[To4]", strTo4);
		strRefPhysSql.Replace("[Prov]", strProvFilter1);
		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		strRefPhysSql.Replace("[referrals]", (m_nCurrentDrilledDownReferral == -1) ? "AND ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND ReferralID"));
		strRefPhysSql.Replace("[referral2]", (m_nCurrentDrilledDownReferral == -1) ? "AND ReferralSource IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND ReferralSource"));
		strRefPhysSql.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID "));
		strRefPhysSql.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID"));
		strRefPhysSql.Replace("[Loc1]", strLocFilter1);
		strRefPhysSql.Replace("[Loc2]", strLocFilter2);
		strRefPhysSql.Replace("[Category]", category);
		strRefPhysSql.Replace("[Category2]", strCat2);
		strRefPhysSql.Replace("[from]", "'" + from + "'");
		strRefPhysSql.Replace("[to]", "'" + to + "'");
		strRefPhysSql.Replace("[PatFilter]", strPatFilter);
		strRefPhysSql.Replace("[Resp]", strResp);
		strRefPhysSql.Replace("[Resp2]", strResp2);

		// (j.dinatale 2012-12-18 14:50) - PLID 54186 - need to add a new filter here, to determine if the primary referral source is the referring phys
		CString strRefPhysPrimFilter;
		if(nRefPhyID > 0){
			strRefPhysPrimFilter = Descendants(nRefPhyID, "AND PatientsT.ReferralID");
		}else{
			// We need a filter here that will make the where clause ALWAYS be false, because we dont want a new category to be added for the referring phys.
			// This will prevent revenue from being reported more than once in multiple different categories
			strRefPhysPrimFilter = " AND 1=0 ";
		}
		strRefPhysSql.Replace("[RefPhysPrimSource]", strRefPhysPrimFilter);
		
		strRefPatSql.Replace("[From]", strFrom);
		strRefPatSql.Replace("[To]", strTo);
		strRefPatSql.Replace("[From2]", strFrom2);
		strRefPatSql.Replace("[To2]", strTo2);
		strRefPatSql.Replace("[From3]", strFrom3);
		strRefPatSql.Replace("[To3]", strTo3);
		strRefPatSql.Replace("[From4]", strFrom4);
		strRefPatSql.Replace("[To4]", strTo4);
		strRefPatSql.Replace("[Prov]", strProvFilter1);
		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		strRefPatSql.Replace("[referrals]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferralID"));
		strRefPatSql.Replace("[referral2]", (m_nCurrentDrilledDownReferral == -1) ? "AND ReferralSource IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND ReferralSource"));
		strRefPatSql.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID"));
		strRefPatSql.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID"));
		strRefPatSql.Replace("[Loc1]", strLocFilter1);
		strRefPatSql.Replace("[Loc2]", strLocFilter2);
		strRefPatSql.Replace("[Category]", category);
		strRefPatSql.Replace("[Category2]", strCat2);
		strRefPatSql.Replace("[from]", "'" + from + "'");
		strRefPatSql.Replace("[to]", "'" + to + "'");
		strRefPatSql.Replace("[PatFilter]", strPatFilter);
		strRefPatSql.Replace("[Resp]", strResp);
		strRefPatSql.Replace("[Resp2]", strResp2);

		// (j.dinatale 2012-12-18 14:50) - PLID 54186 - need to add a new filter here, to determine if the primary referral source is the referring patient
		CString strRefPatPrimFilter;
		if(nRefPatID > 0){
			strRefPatPrimFilter = Descendants(nRefPatID, " AND PatientsT.ReferralID ");
		}else{
			// We need a filter here that will make the where clause ALWAYS be false, because we dont want a new category to be added for the referring patient.
			// This will prevent revenue from being reported more than once in multiple different categories
			strRefPatPrimFilter = " AND 1=0 ";
		}
		strRefPatSql.Replace("[PatientPrimSource] ", strRefPatPrimFilter);
		
		//set the timeout
		CIncreaseCommandTimeout ict(600);		

		//MessageBox(strRefPhysSql);
		GraphDescript desc;
		CString strTempDesc1, strTempDesc2;
				
	/*	COleCurrency cyRefSourTotal1, cyRefSourTotal2, cyRefPhysTotal1, cyRefPhysTotal2, cyRefPatTotal1, cyRefPatTotal2;
		COleCurrency cyZero(0,0);
		double dblRefSourTotal1, dblRefSourTotal2, dblRefPhysTotal1, dblRefPhysTotal2, dblRefPatTotal1, dblRefPatTotal2;
		CString strRefSourTotal1, strRefSourTotal2, strRefPhysTotal1, strRefPhysTotal2, strRefPatTotal1, strRefPatTotal2;
		
		rsTotal = CreateRecordset("SELECT CAST (Sum(%s) AS MONEY) as Total1, CAST( Sum(%s) AS MONEY) as Total2 FROM (%s) Base ", "Payments", "Cost", strRefSourSql);			
		cyRefSourTotal1 = AdoFldCurrency(rsTotal, "Total1", cyZero);
		cyRefSourTotal2 = AdoFldCurrency(rsTotal, "Total2", cyZero);

		strRefSourTotal1 = AsString(rsTotal->Fields->Item["Total1"]->Value);
		strRefSourTotal2 = AsString(rsTotal->Fields->Item["Total2"]->Value);

		strRefSourTotal1.TrimLeft("$");
		strRefSourTotal2.TrimLeft("$");
		strRefSourTotal1.Replace(",", "");
		strRefSourTotal2.Replace(",", "");

		dblRefSourTotal1 = atof(strRefSourTotal1);
		dblRefSourTotal2 = atof(strRefSourTotal2);

		rsTotal = CreateRecordset("SELECT CAST (Sum(%s) AS MONEY) as Total1, CAST( Sum(%s) AS MONEY) as Total2 FROM (%s) Base ", "Payments", "Cost", strRefPhysSql);			
		cyRefPhysTotal1 = AdoFldCurrency(rsTotal, "Total1", cyZero);
		cyRefPhysTotal2 = AdoFldCurrency(rsTotal, "Total2", cyZero);

	

		strRefPhysTotal1 = AsString(rsTotal->Fields->Item["Total1"]->Value);
		strRefPhysTotal2 = AsString(rsTotal->Fields->Item["Total2"]->Value);

		strRefPhysTotal1.TrimLeft("$");
		strRefPhysTotal2.TrimLeft("$");
		strRefPhysTotal1.Replace(",", "");
		strRefPhysTotal2.Replace(",", "");

		dblRefPhysTotal1 = atof(strRefPhysTotal1);
		dblRefPhysTotal2 = atof(strRefPhysTotal2);

		CArray<double, double> aryRefPhys;
		aryRefPhys.Add(dblRefPhysTotal1);
		aryRefPhys.Add(dblRefPhysTotal2);

		rsTotal = CreateRecordset("SELECT CAST (Sum(%s) AS MONEY) as Total1, CAST( Sum(%s) AS MONEY) as Total2 FROM (%s) Base ", "Payments", "Cost", strRefPatSql);			
		cyRefPatTotal1 = AdoFldCurrency(rsTotal, "Total1", cyZero);
		cyRefPatTotal2 = AdoFldCurrency(rsTotal, "Total2", cyZero);
		

		strRefPatTotal1 = AsString(rsTotal->Fields->Item["Total1"]->Value);
		strRefPatTotal2 = AsString(rsTotal->Fields->Item["Total2"]->Value);

		strRefPatTotal1.TrimLeft("$");
		strRefPatTotal2.TrimLeft("$");
		strRefPatTotal1.Replace(",", "");
		strRefPatTotal2.Replace(",", "");

		dblRefPatTotal1 = atof(strRefPatTotal1);
		dblRefPatTotal2 = atof(strRefPatTotal2);


		//set it back
		g_ptrRemoteData->CommandTimeout = nOldTimeout;

		COleCurrency cyGrandTotal1, cyGrandTotal2;
		cyGrandTotal1 = cyRefSourTotal1 + cyRefPhysTotal1 + cyRefPatTotal1;
		cyGrandTotal2 = cyRefSourTotal2 + cyRefPhysTotal2 + cyRefPatTotal2;

		CArray<double, double> aryRefPat;
		aryRefPat.Add(dblRefPatTotal1);
		aryRefPat.Add(dblRefPatTotal2);*/
		

		strTempDesc1.Format("%s", "Income", "");
		strTempDesc2.Format("%s", "Expenditure", "");
			
			
		//desc.Add("Charges",		"Charges",	0xFF0000);
		desc.Add(strTempDesc1,	"Payments",	RGB(43,200,15));
		desc.Add(strTempDesc2,	"Cost",		RGB(215,0,0));

		m_graph->Format = _bstr_t(GetCurrencySymbol());

		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc);

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error in MoneyByRefSour");

	
}

//(e.lally 2009-09-18) PLID 35300 - Got rid of unused parameters, added bIncludeNoReferralEntry.
void CMarketReferralSourceDlg::GraphReferrals(CString strRefSourSql, CString strRefPhysSql, CString strRefPatSql, GraphDescript &desc, BOOL bIncludeNoReferralEntry /*= FALSE*/)
{

	//set up the recordsets to use 
	_RecordsetPtr rsRefSourMasterList, rsRefPhysMasterList, rsRefPatMasterList;
	
	m_progress.SetPos(5);

	CIncreaseCommandTimeout ict(600);

	//These are the special mappings for the referral source used to indicate it was a referring physican or referred by patient.
	long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
	long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);

	CString strRefPatName = "", strRefPhyName = "";
	//(e.lally 2009-09-18) PLID 35300 Simplified some
	{
		_RecordsetPtr rsName = CreateRecordset(" SELECT (SELECT Name FROM ReferralSourceT WHERE PersonID = %li) as RefPatName, "
			" (SELECT Name FROM ReferralSourceT WHERE PersonID = %li) as RefPhyName; \r\n ", nRefPatID, nRefPhyID);
		CString strTmpValue = AdoFldString(rsName, "RefPatName", "");
		if(!strTmpValue.IsEmpty()){
			strRefPatName = "/" + strTmpValue;
		}
		strTmpValue = AdoFldString(rsName, "RefPhyName", "");
		if(!strTmpValue.IsEmpty()){
			strRefPhyName = "/" + strTmpValue;
		}
	}
		
	//PLID 14894 - take the name out of referral sources if we are adding it into referring physcian
	//(e.lally 2009-09-18) PLID 35300 - Simplified some
	CString strWhere = "", strNotRefPatFilter, strNotRefPhyFilter;
	if (nRefPatID > 0){
		strNotRefPatFilter.Format("%s", Descendants(nRefPatID, "AND PersonID NOT "));
	}
	if (nRefPhyID > 0) {
		strNotRefPhyFilter.Format("%s", Descendants(nRefPhyID, " AND PersonID NOT "));
	}

	strWhere.Format("%s %s", strNotRefPatFilter, strNotRefPhyFilter);

	// (j.gruber 2009-06-23 11:31) - PLID 34227 - filter on only the referral sources they want to see
	//TES 7/23/2009 - PLID 34227 - We also need to apply this filter to the referring physicians and referring patients,
	// since they can be linked to referral sources.
	CString strFilter, strRefPhysFilter, strRefPatFilter;

	// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
	// so I cleaned up the code so that is much more obvious now

	//if they are drilled down, we are keeping that filter
	if (m_nCurrentDrilledDownReferral == -1) {
		if (m_dwFilteredRefIDList.GetSize() > 1) {
			strFilter.Format(" AND (PersonID IN %s) ", m_strFilteredReferralList);			
			strRefPhysFilter.Format(" AND (%li IN %s) ", nRefPhyID, m_strFilteredReferralList);
			strRefPatFilter.Format(" AND (%li IN %s) ", nRefPatID, m_strFilteredReferralList);
		}
		else if (m_nCurrentFilteredReferralID != -1) {
			strFilter.Format(" AND (PersonID = %li) ", m_nCurrentFilteredReferralID);
			strRefPhysFilter.Format(" AND (%li = %li) ", nRefPhyID, m_nCurrentFilteredReferralID);
			strRefPatFilter.Format(" AND (%li = %li) ", nRefPatID, m_nCurrentFilteredReferralID);
		}
		else {
			//TES 7/22/2009 - PLID 34227 - We still need this, so that child referral sources won't show on the top level.
			strFilter.Format(" AND (Parent = %li or PersonID = %li) ", m_nCurrentDrilledDownReferral, m_nCurrentDrilledDownReferral);
		}
	}
	else {
		strFilter.Format(" AND (Parent = %li or PersonID = %li) ", m_nCurrentDrilledDownReferral, m_nCurrentDrilledDownReferral);
	}


	//(e.lally 2009-09-18) PLID 35300 - Added No Referral optional entry
	//(e.lally 2010-03-11) PLID 37709 - Do not show the No Referral option if we are drilled down on the referring physicians or referring patients
	// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
	// so I cleaned up the code so that is much more obvious now
	CString strNoReferralEntry = bIncludeNoReferralEntry == FALSE || m_nCurrentDrilledDownReferral == -2 || m_nCurrentDrilledDownReferral == -3 ? "": FormatString("SELECT '<No Referral>' AS Name, %li AS PersonID UNION ", NOREFERRAL_SENTINEL_ID);
	rsRefSourMasterList = CreateRecordset(adOpenStatic, adLockReadOnly,  strNoReferralEntry +
		"SELECT (CASE WHEN PersonID = %li THEN 'ZZZZZZZ' ELSE ReferralSourceT.Name END) AS Name, PersonID "
		"FROM ReferralSourceT "
		"WHERE (1=1) %s %s "
		"ORDER BY Name;", m_nCurrentDrilledDownReferral, strFilter, strWhere);

	//(e.lally 2010-04-06) PLID 37102 - use _Q for the ref phys name string
	rsRefPhysMasterList = CreateRecordset(adOpenStatic, adLockReadOnly, " SELECT 'Referring Physician%s' AS Name, -2 AS PersonID WHERE -1 = %li %s " 
		" UNION SELECT '[Other]' AS Name, -2 AS PersonID WHERE -2 = %li %s "
		"UNION SELECT Last + ', ' + First + ' ' + Middle AS Name, ID AS PersonID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = "
		"PatientsT.DefaultReferringPhyID WHERE -2 = %li %s", _Q(strRefPhyName), m_nCurrentDrilledDownReferral, strRefPhysFilter, m_nCurrentDrilledDownReferral, strRefPhysFilter, m_nCurrentDrilledDownReferral, strRefPhysFilter);
	
	//(e.lally 2010-04-06) PLID 37102 - use _Q for the ref patient name string
	rsRefPatMasterList = CreateRecordset(adOpenStatic, adLockReadOnly, "SELECT 'Referring Patient%s' AS Name, -3 AS PersonID WHERE -1 = %li %s "
			" UNION SELECT '[Other]', -3 as PersonID WHERE -3 = %li %s "
			"UNION SELECT Last + ', ' + First + ' ' + Middle AS Name, ID AS PersonID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = "
			"PatientsT.ReferringPatientID WHERE -3 = %li %s"
			"ORDER BY Name;", _Q(strRefPatName), m_nCurrentDrilledDownReferral, strRefPatFilter, m_nCurrentDrilledDownReferral, strRefPatFilter, m_nCurrentDrilledDownReferral, strRefPatFilter);
	
	m_progress.SetPos(10);

	//set the row count for the graph
	//(e.lally 2009-09-18) PLID 35300 - Send in the row counts to our setup function
	long nRowCount = rsRefSourMasterList->RecordCount + rsRefPhysMasterList->RecordCount + rsRefPatMasterList->RecordCount;	

	//run the main query first
	//(e.lally 2009-09-18) PLID 35300 - Renamed recordset variable to be clearer
	_RecordsetPtr rsReferralSourceDetails = CreateRecordsetStd(strRefSourSql, adOpenStatic, adLockReadOnly);
	m_progress.SetPos(30);
	long nCurrentRow = 0, nCurrentColumn = 0;

	//Finish graph setup
	//(e.lally 2009-09-18) PLID 35300 - Created function to set up the graph with the GraphDescript details
	InitializeGraph(desc, nRowCount);
	
	//Graph all the standard referral source data
	//(e.lally 2009-09-18) PLID 35300 - Combined into one GraphBy function
	GraphByReferral(desc, rsReferralSourceDetails, rsRefSourMasterList, nCurrentRow, nCurrentColumn, bIncludeNoReferralEntry);	
	m_progress.SetPos(40);
	if (nCurrentRow != -1) {
		//Get the referring physician referral details
		_RecordsetPtr rsRefPhysDetails = CreateRecordsetStd(strRefPhysSql, adOpenStatic, adLockReadOnly);
		m_progress.SetPos(50);
		//Now graph the details for ref phy
		//(e.lally 2009-09-18) PLID 35300 - Combined into one GraphBy function
		GraphByReferral(desc, rsRefPhysDetails, rsRefPhysMasterList, nCurrentRow, nCurrentColumn, FALSE  /*, -2, aryRefPhys, aryRefPhys2*/);
		m_progress.SetPos(60);
		if (nCurrentRow != -1) {
			//Referring Patients
			_RecordsetPtr rsRefPatientDetails = CreateRecordsetStd(strRefPatSql, adOpenStatic, adLockReadOnly);
			m_progress.SetPos(70);
			//(e.lally 2009-09-18) PLID 35300 - Combined into one GraphBy function
			GraphByReferral(desc, rsRefPatientDetails, rsRefPatMasterList, nCurrentRow, nCurrentColumn, FALSE /*, -3, aryRefPat, aryRefPat2*/);
			m_progress.SetPos(80);
		}
	}

	//if this is EFFMonByRefSour then set the descriptions now
	if (GetType() == EFFMonByRefSour) {
		for (int i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			COleCurrency cy;
			// (b.spivey, January 25, 2012) - PLID 47757 - Convert directly from a double and round for international clients. 
			cy = AsCurrency(desc.dblTotal(i));
			RoundCurrency(cy); 
			// (z.manning 2008-06-18 16:21) - PLID 28601 - Hide decimal places on marketing graphs.
			strText.Format("%s (%s Total)", desc.Label(i), FormatCurrencyForInterface(cy, TRUE, TRUE, FALSE));
			m_graph->ColumnText = (_bstr_t)strText;
			m_graph->ColumnFormat = m_graph->Format;
		}
	}
	m_progress.SetPos(100);


	// (j.jones 2004-06-15 13:23) - do not sort until all three have been finished,
	// because will already specified the row count but will not have filled in all the rows
	m_graph->SortGraph();

	m_progress.SetPos(0);
}

//(e.lally 2009-09-18) PLID 35300 - Moved this code into a separate function to more easily set the graph details
void CMarketReferralSourceDlg::InitializeGraph(GraphDescript &desc, long nRowCount)
{
	m_graph->ClearGraph();
	m_arClickableRows.RemoveAll();

	m_graph->RowCount = nRowCount;

	//setup graph
	m_graph->ColumnCount = desc.Size();

	for (int i = 0; i < desc.Size(); i++)
	{	m_graph->Column = i;
		m_graph->Color = desc.Color(i);
		m_graph->ColumnText = (_bstr_t)desc.Label(i);
		//(e.lally 2009-09-18) PLID 35559 - if the column is a percent, set the format automatically
		if(desc.Op(i) == GraphDescript::GD_PERCENT){
			m_graph->ColumnFormat = "%0.0f%%";
		}
		else{
			m_graph->ColumnFormat = m_graph->Format;
		}
	}
	
	m_graph->XAxisDesc = "Referral Source";
	m_graph->XAxisSortStyle = cgSortAlpha;
	m_graph->AllowXAxisSort = TRUE;
}

//(e.lally 2009-09-18) PLID 35300 - Combined into one function. Rearraged and renamed parameters for better clarity
void CMarketReferralSourceDlg::GraphByReferral(GraphDescript &desc, ADODB::_RecordsetPtr rsReferralDetails, ADODB::_RecordsetPtr rsMasterReferralList, long &nCurrentRow, long &nCurrentColumn, BOOL bIncludeNoReferralEntry /*=FALSE*/)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int				i=0, 
						j=0;
		double			total=0, total2=0;
		CString			sql;
		CWaitCursor		wait;
		
		// (b.cardillo 2004-08-06 09:17) - PLID 13747 - Before the loop, scan the recordset into 
		// our map of values, so that we can access it quickly inside the loop.
		CMapLongToGraphValuesArray mapReferralGraphValues;
		//(e.lally 2009-09-18) PLID 35300 - Added default value for nulls
		mapReferralGraphValues.ScanRecordsetIntoMap(rsReferralDetails, "ReferralID", NOREFERRAL_SENTINEL_ID, desc);

		//all the real work is done, but we still have to sort the results
		int currentID =0;
		i = nCurrentRow;
		FieldPtr fldName = rsMasterReferralList->Fields->GetItem("Name");
		FieldPtr fldPersonID = rsMasterReferralList->Fields->GetItem("PersonID");
		while (!rsMasterReferralList->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR)AdoFldString(fldName);
			currentID = AdoFldLong(fldPersonID);
			m_graph->RowID = currentID;

			//Get descendants of the ith child
			// (b.cardillo 2004-08-06 09:48) - PLID 13747 - We used to do this by filling an array, 
			// now we just get a pointer to the sub-tree so it's much faster, which is important 
			// inside a loop.  This pointer is used to determine if there are any children, and then 
			// the subtree it points to is used below for calculating the totals.
			const CTreeLong *ptlDescendents = NULL;
			if (currentID != m_nCurrentDrilledDownReferral) {//special [Other] column
				//(e.lally 2009-09-18) PLID 35300 - Added bIncludeNoReferralEntry parameter
				Descendants(currentID, &ptlDescendents, bIncludeNoReferralEntry);
			} else {
				m_graph->RowText = "[Other]";
				m_graph->RowID = m_nCurrentDrilledDownReferral;
				ptlDescendents = NULL;
			}

			//Set rowList so we know if there is a descendant
			if (ptlDescendents && ptlDescendents->GetFirstChildPosition()) {
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
				// ReferralID was currentID or any of its descendents.  Now we do the same thing but we use 
				// our super-fast map that we loaded before the loop instead of the recordset over and over.
				if (!rsReferralDetails->eof) {
					// Create the object that will do the processing
					CED_GraphCalcColumnTotals_Info gbrsi(&mapReferralGraphValues, desc, j, total, total2);
					// Call it for the root level
					gbrsi.ProcessGraphValues(currentID);
					// If we're on a drill-down-able node, then add all of those in too
					if (ptlDescendents) {
						ptlDescendents->EnumDescendents(CED_GraphCalcColumnTotals_Info::CallbackProcessGraphValues, &gbrsi, FALSE);
					}
					// Now return the totals values to our local variables
					total = gbrsi.m_dblTotal;
					total2 = gbrsi.m_dblTotal2;
	
				}
				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						m_graph->Value = total;
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						break;
					case GraphDescript::GD_DIV:
					//(e.lally 2009-09-18) PLID 35559 - Added support for percent operations, follows same logic as division
					case GraphDescript::GD_PERCENT:
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						if (total2 != 0) {
							double dblResult = total / total2;
							//(e.lally 2009-09-18) PLID 35559 - percent mode expects 1/2 to be 50, not 0.5 
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

			rsMasterReferralList->MoveNext();
		}

		// (j.jones 2004-06-15 13:23) - do not sort until all three have been finished,
		// because will already specified the row count but will not have filled in all the rows
		//m_graph->SortGraph();

		nCurrentRow =i;
		nCurrentColumn = j;
	}
	NxCatchAllCall("Could not calculate graph", nCurrentRow = nCurrentColumn = -1;);
}

// a.walling PLID 20695 UpdateView modified to not render immediately. All checkboxes/radios/options etc will not cause a render either.
void CMarketReferralSourceDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
	try {
		//(a.wilson 2011-10-5) PLID 38789 - changed to prevent the module from refreshing when coming back to it.
		if ( (!m_bIsLoading && m_bRenderedOnce) || (bForceRefresh && m_bActive) ) {
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

			if(m_graph) {
				if (m_nCurrentDrilledDownReferral > 0) {	
					CString strReferralName;
					_RecordsetPtr rsRefName = CreateRecordset("SELECT Name FROM ReferralSourceT WHERE PersonID = %li", m_nCurrentDrilledDownReferral);
					strReferralName = AdoFldString(rsRefName, "Name");
					m_graph->Title = _bstr_t(strReferralName);
					m_Up.EnableWindow();
					m_Up.ShowWindow(TRUE);
				}
				else if(m_nCurrentDrilledDownReferral == -2) {
					m_graph->Title = "Referring Physician";
					m_Up.EnableWindow(TRUE);
					m_Up.ShowWindow(TRUE);
				}
				else if(m_nCurrentDrilledDownReferral == -3) {
					m_graph->Title = "Referring Patient";
					m_Up.EnableWindow(TRUE);
					m_Up.ShowWindow(TRUE);
				}
				else {	
					m_graph->Title = "All";
					m_Up.EnableWindow(FALSE);
					m_Up.ShowWindow(FALSE);
				}
			}
			if (m_MoneyByRefSourRad.GetCheck()) {
				// (c.haag 2004-04-22 12:16) - Make sure we have a valid filter type
				if (GetType() != EFFMonByRefSour)
				{
					SetType(EFFMonByRefSour);
					SetFilter(mfPaymentDate, mftDate);
				}
				//hang for a second so the rest of the screen draws
				//removed since we do not automatically run the report now
				MoneyByRefSour();
			}
			else if (m_PatsByReferral.GetCheck()) {
				PatsByReferral();
			}
			else if (m_InqsByReferral.GetCheck()) {
				InqsByReferral();
			}
			else if (m_NoShowsByReferral.GetCheck()) {
				NoShowsByReferral();
			}
			else if (m_ProcsClosed.GetCheck()) {
				ProcsClosed();
			}
			else if (m_conversionRad.GetCheck()) {
				ConversionRad();
			}
			else if (m_InqToCons.GetCheck()) {
				InqToCons();
			}
			else if (m_ProsToCons.GetCheck()) {
				ProsToCons();
			}		
		}
		else { // setup filters
			if (m_MoneyByRefSourRad.GetCheck()) {
				OnMoneyByRefSour();
			}
			else if (m_PatsByReferral.GetCheck()) {
				OnPatsByReferral();
			}
			else if (m_InqsByReferral.GetCheck()) {
				OnInqsByReferral();
			}
			else if (m_NoShowsByReferral.GetCheck()) {
				OnNoShowsByReferral();
			}
			else if (m_ProcsClosed.GetCheck()) {
				OnProcsClosed();
			}
			else if (m_conversionRad.GetCheck()) {
				OnConversionRad();
			}
			else if (m_InqToCons.GetCheck()) {
				OnInqToCons();
			}
			else if (m_ProsToCons.GetCheck()) {
				OnProsToCons();
			}		

			m_mfiFilterInfo.SetFilters();
		}
		//ensure the back button is still positioned correctly
		OnChangeBackButtonPosColumnGraph();


		if (m_bIsLoading) {
			m_bIsLoading = FALSE;
			m_Up.EnableWindow(FALSE);
			m_Up.ShowWindow(FALSE);
			GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
			ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051
			m_MoneyByRefSourRad.SetCheck(TRUE); // set the default report then
			OnMoneyByRefSour();
		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
		
		InvalidateConsToProcRadioButtons();
		// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
		InvalidateShowInquiriesButton();
		
		//(a.wilson 2012-5-22) PLID 50378 - refresh the refrral list in case a referral has been inactivated.
		m_pReferralSourceList->Requery();
		
	}NxCatchAll("Error Updating View");
}

void CMarketReferralSourceDlg::OnUp() 
{
	// (j.jones 2012-08-07 17:28) - PLID 51058 - changed the variable used here to clarify that
	// it only reflects the referral we are drilled down to on the graph, it is not the referral
	// selected in the dropdown filter

	CWaitCursor wc;

	if (m_nCurrentDrilledDownReferral == -1)
		return;

	try
	{
		if(m_nCurrentDrilledDownReferral > 0) {
			_RecordsetPtr rs = CreateRecordset("SELECT Parent FROM ReferralSourceT "
				"WHERE ReferralSourceT.PersonID = %li", m_nCurrentDrilledDownReferral);
			
			m_nCurrentDrilledDownReferral = AdoFldLong(rs, "Parent");
		}
		else {
			m_nCurrentDrilledDownReferral = -1;
		}
	}
	NxCatchAll("Could not go back");
	
	OnGo();

	//revert to showing the default amount of records
	m_graph->ShowXRecords = 10;	
}

void CMarketReferralSourceDlg::OnOnClickColumnGraph(short Row, short Column) 
{
	m_graph->Row = Row;
	for(int i = 0; i < m_arClickableRows.GetSize(); i++) {
		if(m_arClickableRows.GetAt(i) == m_graph->RowID) {
			m_graph->Row = Row;
			// (j.jones 2012-08-07 17:28) - PLID 51058 - changed the variable used here to clarify that
			// it only reflects the referral we are drilled down to on the graph, it is not the referral
			// selected in the dropdown filter
			m_nCurrentDrilledDownReferral = m_graph->RowID;
			OnGo();
			return;
		}
	}
	
}

void CMarketReferralSourceDlg::OnOnMouseMoveColumnGraph(short Row, short Column) 
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


void CMarketReferralSourceDlg::OnPatsByReferral() {
	if (LastChecked(IDC_PATS_BY_REFERRAL)) return;
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051
	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_INQUIRIES)->Invalidate();
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMPatByRefSour) {
		SetType(NUMPatByRefSour);
		//DRT 5/8/2008 - PLID 29966 - We added referral date, and if we're in internal, 
		//	we want it defaulted.
		if(IsNexTechInternal()) {
			SetFilter(mfReferralDate, mftDate);
		}
		else {
			SetFilter(mfFirstContactDate, mftDate);
		}
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::PatsByReferral()
{
	CWaitCursor wc;

	try {
		GraphDescript desc;
		CString from, to, provIDs, strPersonProv, strApptProv, strPersonLoc, strApptLoc, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int nCategoryID=0, nResp=0;

		m_graph->PrintTitle = "Patients By Referrals";

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;
			
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		if (UseFilter(mftProvider))  {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strProvField, strFilter1, strFilter2);
			
			//fill these, then we will clear them if we need to 
			strPersonProv.Format(" AND %s %s) ", strFilter1, provIDs);
			strApptProv.Format(" AND %s %s) ", strFilter2, provIDs);

			if (strFilter1.IsEmpty()) {
				strPersonProv = "";
			}
					
			if (strFilter2.IsEmpty()) {
				strApptProv = "";
			}
		}
		
		if (UseFilter(mftLocation))  {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strLocationField, strFilter1, strFilter2);
		
			//fill these, then we will clear them if we need to 
			strPersonLoc.Format(" AND %s IN %s ", strFilter1, locIDs);
			strApptLoc.Format(" AND %s IN %s ", strFilter2, locIDs);

			if (strFilter1.IsEmpty()) {
				strPersonLoc = "";
			}
				
			if (strFilter2.IsEmpty()) {
				strApptLoc = "";
			}
		}
		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);




		/*long nFlag;
		if (IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
			nFlag = 1;
		}
		else {
			nFlag = -1;
		}*/

		//DRT 6/24/2008 - While doing PLID 29966, I decided to write out some comments how this all 
		//	works, since noone did so when implementing it.  There is a preference to tie ref phys
		//	to a referral, and the same with ref pat.  If this preference link is enabled, our queries change
		//	a bit.  These are the 3 queries that can be obtained from GetGraphSql().  The numbers are the
		//	flag to use to get the right query.
		//
		//	1)  Select all Patients, joined to the ReferralSource tables.  Subqueries exist for having
		//		procedures and consults.  Inquiries are UNION'd to the end.  The selected ID is the
		//		referral source ID.
		//	2)  Select all Patients, joined to the RefPhys tables.  The selected ID is the ref phys ID.
		//	3)  Select all Patients, joined to the RefPat tables.  The selected ID is the ref patient ID.
		//
		//	So we decide whether we're using the preference link or not.  If we are not, then we just select
		//	all referring physician records.  If we are, then we select all referral source records, EXCLUDING
		//	the "linked" referral source.  We then UNION that with all Referring Physicians.  So instead of
		//	getting 2 records (1 for the referral and 1 for the phys), it will be just 1 record, filtered
		//	for the referring physician data.
		//	Referring patients work the same way.
		//	If we are using either preference, when GraphReferrals() is called, it will strip out all records
		//	from the ReferralSource portion for the preference-selected referral and its children, the record
		//	from the ReferringPhysician query will be used & renamed to include the referral name.

		CString strRefSourSql, strRefPhysSql, strRefPatSql;

		long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
		long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
		strRefSourSql = GetGraphSql(NUMPatByRefSour, 1, -1, pCon, strPatientTempTable);
		
		if (nRefPhyID > 0) {
			//this is an odd place for the replace, but we have to do it before its combined
			CString strTmp = GetGraphSql(NUMPatByRefSour, 1, -1, pCon, strPatientTempTable);
			strTmp.Replace("[ref]", Descendants(nRefPhyID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", " AND PatientsT.DefaultReferringPhyID IS NULL ");
			strTmp.Replace("[RefPatPref]", "");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-2 AS ");
			strRefPhysSql =  strTmp + " UNION ALL " + GetGraphSql(NUMPatByRefSour, 2, -1, pCon, strPatientTempTable);
			//DRT 7/9/2008 - PLID 29966 - We've added a referral filter in the query for referring phys, 
			//	so we want to filter it.  This will give us a potential MultiReferralsT.Date field which
			//	can be used if the user is filtering on "Referral Date".
			strRefPhysSql.Replace("[RefPhysReferralDateFilter]", Descendants(nRefPhyID, "AND ReferralID"));
		}
		else {
			strRefPhysSql = GetGraphSql(NUMPatByRefSour, 2, -1, pCon, strPatientTempTable);
			strRefPhysSql.Replace("[RefPhysPref]", "");
			//DRT 7/9/2008 - PLID 29966 - Same field as above, but if the preference is turned off, we
			//	force this to cause the subquery to fail.  This makes us fall back to using FirstContactDate.
			strRefPhysSql.Replace("[RefPhysReferralDateFilter]", "AND (0=1)");
		}


		if (nRefPatID > 0) {
			CString strTmp = GetGraphSql(NUMPatByRefSour, 1, -1, pCon, strPatientTempTable);
			//this is an odd place for the replace, but we have to do it before its combined
			//str2.Format(" AND ReferralSource = %li OR ReferralSource IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);*/
			strTmp.Replace("[ref]", Descendants(nRefPatID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", "");
			strTmp.Replace("[RefPatPref]", " AND PatientsT.ReferringPatientID IS NULL ");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-3 AS ");
			strRefPatSql =  strTmp + " UNION ALL " + GetGraphSql(NUMPatByRefSour, 3, -1, pCon, strPatientTempTable);
			//DRT 7/9/2008 - PLID 29966 - We've added a referral filter in the query for referring phys, 
			//	so we want to filter it.  This will give us a potential MultiReferralsT.Date field which
			//	can be used if the user is filtering on "Referral Date".
			strRefPatSql.Replace("[RefPatReferralDateFilter]", Descendants(nRefPatID, "AND ReferralID"));
		}
		else {
			strRefPatSql = GetGraphSql(NUMPatByRefSour, 3, -1, pCon, strPatientTempTable);
			strRefPatSql.Replace("[RefPatPref]", "");
			//DRT 7/9/2008 - PLID 29966 - Same field as above, but if the preference is turned off, we
			//	force this to cause the subquery to fail.  This makes us fall back to using FirstContactDate.
			strRefPatSql.Replace("[RefPatReferralDateFilter]", "AND (0=1)");
		}


		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		CString referrals = CString("AND ") + ((m_nCurrentDrilledDownReferral == -1) ? "ReferralsQ.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "ReferralsQ.ReferralID"));

		//PLID 14894: take out any referral sources of people that also have referring phys or referring pats
		CString strRefPatReplace;
		if (nRefPatID > 0) {
			//strRefPatReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);
			strRefPatReplace = Descendants(nRefPatID, "AND ReferralID NOT ");
		}
		else {
			strRefPatReplace = "";
		}
		
		CString strRefPhysReplace;
		if (nRefPhyID > 0) {
			//strRefPhysReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPhyID, nRefPhyID);
			strRefPhysReplace = Descendants(nRefPhyID, "AND ReferralID NOT ");
		}
		else {
			strRefPhysReplace = "";
		}
		strRefSourSql.Replace("[TakeOutForRefPhys]", strRefPhysReplace);
		strRefSourSql.Replace("[TakeOutForRefPat]", strRefPatReplace);

		strRefSourSql.Replace("[from]", strFrom);
		strRefSourSql.Replace("[to]", strTo);
		strRefSourSql.Replace("[Prov]", strPersonProv);
		strRefSourSql.Replace("[ApptProv]", strApptProv);
		strRefSourSql.Replace("[loc]", strPersonLoc);
		strRefSourSql.Replace("[ApptLoc]", strApptLoc);
		strRefSourSql.Replace("[PatFilter]", strPatFilter);
		strRefSourSql.Replace("[ref]", referrals);
		strRefSourSql.Replace("[ReplacePref]", "");
		strRefSourSql.Replace("[RefPatPref]", "");
		strRefSourSql.Replace("[RefPhysPref]", "");

		strRefPhysSql.Replace("[from]", strFrom);
		strRefPhysSql.Replace("[to]", strTo);
		strRefPhysSql.Replace("[Prov]", strPersonProv);
		strRefPhysSql.Replace("[ApptProv]", strApptProv);
		strRefPhysSql.Replace("[loc]", strPersonLoc);
		strRefPhysSql.Replace("[ApptLoc]", strApptLoc);
		strRefPhysSql.Replace("[PatFilter]", strPatFilter);
		strRefPhysSql.Replace("[ref]", referrals);

		strRefPatSql.Replace("[from]", strFrom);
		strRefPatSql.Replace("[to]", strTo);
		strRefPatSql.Replace("[Prov]", strPersonProv);
		strRefPatSql.Replace("[ApptProv]", strApptProv);
		strRefPatSql.Replace("[loc]", strPersonLoc);
		strRefPatSql.Replace("[ApptLoc]", strApptLoc);
		strRefPatSql.Replace("[PatFilter]", strPatFilter);
		strRefPatSql.Replace("[ref]", referrals);
			
		CString strTempDesc1, strTempDesc2, strTempDesc3, strTempDesc4;
		
		strTempDesc1.Format("%s", "Referrals");
		strTempDesc2.Format("%s", "Scheduled Consults");
		strTempDesc3.Format("%s", "Scheduled Procedures");
		strTempDesc4.Format("%s", "Inquiries");

			
			
		/*if(IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
			strTempDesc2.Format("%s (%s Total) ", "Completed Consults", AsString(rsTotal->Fields->Item["Total2"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total2"]->Value));
			strTempDesc3.Format("%s (%s Total) ", "Completed Procedures", AsString(rsTotal->Fields->Item["Total3"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total3"]->Value));
		}
		else {
			strTempDesc2.Format("%s (%s Total) ", "Scheduled Consults", AsString(rsTotal->Fields->Item["Total2"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total2"]->Value));
			strTempDesc3.Format("%s (%s Total) ", "Scheduled Procedures", AsString(rsTotal->Fields->Item["Total3"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total3"]->Value));
		}*/
		//strTempDesc4.Format("%s (%s Total) ", "Inquiries", AsString(rsTotal->Fields->Item["Total4"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total4"]->Value));

		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		desc.Add(strTempDesc1, "Referrals", GetMarketGraphColor(mgcBrightBlue));
		desc.Add(strTempDesc2, "ConsConv", GetMarketGraphColor(mgcBrightGreen));
		desc.Add(strTempDesc3, "Conv", GetMarketGraphColor(mgcBrightRed));
			
		if (IsDlgButtonChecked(IDC_SHOW_INQUIRIES)) {

			desc.Add(strTempDesc4, "NumInquiries", RGB(255,0,255));
		}

		//MessageBox(strRefPatSql);
		m_graph->Format = "%0.0f";
		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc);

		for (int i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
			m_graph->ColumnText = (_bstr_t)strText;
			m_graph->ColumnFormat = m_graph->Format;
		}

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error Graphing Patients");


}

void CMarketReferralSourceDlg::OnInqsByReferral() {
	if (LastChecked(IDC_INQ_BY_REFERRAL)) return;

	if(GetMainFrame()->m_pDocToolBar->GetType() != REFInqByReferral) {
		SetType(REFInqByReferral);
		SetFilter(mfFirstContactDate, mftDate);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::InqsByReferral() {
	
	CWaitCursor wc;
	
	GraphDescript desc;
	CString sql, from, to, docFilter, provIDs, locFilter, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
	//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
	int nCategoryID=0, nResp=0;
	CString referrals;

	m_graph->PrintTitle = "Inquiry To Consult By Referrals";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

	if (UseFilter(mftProvider))
		docFilter.Format(" AND PatientsT.MainPhysician IN %s ", provIDs);
	if (locIDs != "")
		locFilter.Format(" AND Location IN %s ", locIDs);
	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


	long nFlag;
	if (IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
		nFlag = 1;
	}
	else {
		nFlag = -1;
	}
	
	sql = GetGraphSql(REFInqByReferral, nFlag, -1, pCon, strPatientTempTable);

	CString strFrom, strTo;
	if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	sql.Replace("[from]", strFrom);
	sql.Replace("[to]", strTo);
	sql.Replace("[prov]", docFilter);
	sql.Replace("[loc]", locFilter);
	sql.Replace("[PatFilter]", strPatFilter);
	// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
	// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
	// the same effect as filtering on the set of all referrals (which is what we used to do but it 
	// was slower, especially if you had a ton of referrals).
	// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
	// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
	// so I cleaned up the code so that is much more obvious now
	referrals = CString("AND ") + ((m_nCurrentDrilledDownReferral == -1) ? "ReferralsQ.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "ReferralsQ.ReferralID"));
	sql.Replace("[ref]", referrals);

	
	desc.m_sql = sql;
	Save(sql);
	m_strSql = sql;
	desc.Add("Inquiries", "NumInquiries", RGB(255,0,255));

	//MessageBox(sql);
	
	m_graph->Format = "%0.0f";
	//GraphByReferral(desc);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}

}

void CMarketReferralSourceDlg::OnNoShowsByReferral() {
	if (LastChecked(IDC_NO_SHOW_BY_REFERRAL3)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051

	if(GetMainFrame()->m_pDocToolBar->GetType() != REFNoShowByReferral) {
		SetType(REFNoShowByReferral);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::NoShowsByReferral() {
	
	CWaitCursor wc;

	try {


		

		GraphDescript desc;
		CString sql, from, to, docFilter, provIDs, locFilter, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int nCategoryID=0, nResp=0;

		m_graph->PrintTitle = "Cancellations and No Shows By Referrals";

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		if (UseFilter(mftProvider)) {
			docFilter.Format(" AND %s %s) ", strProvField, provIDs);
		}
		
		if (locIDs != "") {
			locFilter.Format(" AND %s IN %s ", strLocationField, locIDs);
		}

		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


		/*long nFlag;
		if (IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
			nFlag = 1;
		}
		else {
			nFlag = -1;
		}*/
		CString strRefSourSql, strRefPhysSql, strRefPatSql;
		
		long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
		long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
		strRefSourSql = GetGraphSql(REFNoShowByReferral, 1, -1, pCon, strPatientTempTable);
		
		if (nRefPhyID > 0) {
			//this is an odd place for the replace, but we have to do it before its combined
			CString strTmp = GetGraphSql(REFNoShowByReferral, 1, -1, pCon, strPatientTempTable);
			strTmp.Replace("[ref]", Descendants(nRefPhyID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", " AND PatientsT.DefaultReferringPhyID IS NULL ");
			strTmp.Replace("[RefPatPref]", "");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-2 AS ");
			strRefPhysSql =  strTmp + " UNION ALL " + GetGraphSql(REFNoShowByReferral, 2, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPhysSql = GetGraphSql(REFNoShowByReferral, 2, -1, pCon, strPatientTempTable);
			strRefPhysSql.Replace("[RefPhysPref]", "");
		}


		if (nRefPatID > 0) {
			CString strTmp = GetGraphSql(REFNoShowByReferral, 1, -1, pCon, strPatientTempTable);
			//this is an odd place for the replace, but we have to do it before its combined
			//str2.Format(" AND ReferralSource = %li OR ReferralSource IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);*/
			strTmp.Replace("[ref]", Descendants(nRefPatID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", "");
			strTmp.Replace("[RefPatPref]", " AND PatientsT.ReferringPatientID IS NULL ");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-3 AS ");
			strRefPatSql =  strTmp + " UNION ALL " + GetGraphSql(REFNoShowByReferral, 3, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPatSql = GetGraphSql(REFNoShowByReferral, 3, -1, pCon, strPatientTempTable);
			strRefPatSql.Replace("[RefPatPref]", "");
		}

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		CString referrals = CString("AND ") + ((m_nCurrentDrilledDownReferral == -1) ? "ReferralsQ.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "ReferralsQ.ReferralID"));


		//PLID 14894: take out any referral sources of people that also have referring phys or referring pats
		CString strRefPatReplace;
		if (nRefPatID > 0) {
			//strRefPatReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);
			strRefPatReplace = Descendants(nRefPatID, "AND ReferralID NOT ");
		}
		else {
			strRefPatReplace = "";
		}
		
		CString strRefPhysReplace;
		if (nRefPhyID > 0) {
			//strRefPhysReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPhyID, nRefPhyID);
			strRefPhysReplace = Descendants(nRefPhyID, "AND ReferralID NOT ");
		}
		else {
			strRefPhysReplace = "";
		}
		strRefSourSql.Replace("[TakeOutForRefPhys]", strRefPhysReplace);
		strRefSourSql.Replace("[TakeOutForRefPat]", strRefPatReplace);

		strRefSourSql.Replace("[From]", strFrom);
		strRefSourSql.Replace("[To]", strTo);
		strRefSourSql.Replace("[Prov]", docFilter);
		strRefSourSql.Replace("[Loc]", locFilter);
		strRefSourSql.Replace("[PatFilter]", strPatFilter);
		strRefSourSql.Replace("[ref]", referrals);
		strRefSourSql.Replace("[ReplacePref]", "");
		strRefSourSql.Replace("[RefPatPref]", "");
		strRefSourSql.Replace("[RefPhysPref]", "");


		strRefPhysSql.Replace("[From]", strFrom);
		strRefPhysSql.Replace("[To]", strTo);
		strRefPhysSql.Replace("[Prov]", docFilter);
		strRefPhysSql.Replace("[Loc]", locFilter);
		strRefPhysSql.Replace("[PatFilter]", strPatFilter);
		strRefPhysSql.Replace("[ref]", referrals);

		strRefPatSql.Replace("[From]", strFrom);
		strRefPatSql.Replace("[To]", strTo);
		strRefPatSql.Replace("[Prov]", docFilter);
		strRefPatSql.Replace("[Loc]", locFilter);
		strRefPatSql.Replace("[PatFilter]", strPatFilter);
		strRefPatSql.Replace("[ref]", referrals);
		//MessageBox(strRefPatSql);


		//MessageBox(strRefPhysSql);


		CString strTempDesc1, strTempDesc2;
		strTempDesc1.Format("%s", "No Shows");
		strTempDesc2.Format("%s", "Cancellations");
			
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		desc.Add(strTempDesc1, "NumNoShow", GetMarketGraphColor(mgcBrightBlue));
		desc.Add(strTempDesc2, "NumCancel", GetMarketGraphColor(mgcBrightRed));

		//MessageBox(sql);	
		
		m_graph->Format = "%0.0f";
		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc);

		for (int i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
			m_graph->ColumnText = (_bstr_t)strText;
			m_graph->ColumnFormat = m_graph->Format;
		}

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error generating graph");
}

void CMarketReferralSourceDlg::OnConversionRad() {
	if (LastChecked(IDC_CONVERSION_RAD)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
	ShowConversionRateControls(TRUE); // (z.manning 2009-08-31 17:00) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != CNVConsToSurgByRefSour) {
		SetType(CNVConsToSurgByRefSour);
		//(e.lally 2009-09-24) PLID 35300 - make Consult date the default
		SetFilter(mfConsultDate, mftDate);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::ConversionRad() 
{
	CWaitCursor wc;

	//This is the "Conversion Rate" button
	m_graph->PrintTitle = "Consult To Procedure By Referrals";

	GraphDescript desc;
	CString strRefSourSql, strRefPhysSql, strRefPatSql;

	long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
	long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);

	BOOL bShowNumbers = IsDlgButtonChecked(IDC_SHOW_NUMBERS_ONLY_REF) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS_REF);
	BOOL bShowPercentages = IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY_REF) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS_REF);

	if(!IsConsToProcSetupValid()) {
		MessageBox("Please set up your configuration settings by clicking the ... button before running this graph");
		return;
	}

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	//(e.lally 2009-09-18) PLID 35300 - completely different graphing for the consult date filter
	MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);
	if  (mfDateFilter == mfConsultDate || mfDateFilter == mfFirstContactDate) {
		m_graph->PrintTitle = "Consult To Procedure By Referrals";
		//Get the query for the summary
		CString strSummarySql = GetGraphSql(CNVConsToSurgByRefSour, 4, -1, pCon, strPatientTempTable);
		//Get the queries for regular referral source, referring physician, and referring patient
		strRefSourSql = GetGraphSql(CNVConsToSurgByRefSour, 4, 1, pCon, strPatientTempTable);
		strRefPhysSql = GetGraphSql(CNVConsToSurgByRefSour, 4, 2, pCon, strPatientTempTable);
		strRefPatSql = GetGraphSql(CNVConsToSurgByRefSour, 4, 3, pCon, strPatientTempTable);

		//(e.lally 2009-09-28) PLID 35594 - Gather the referral filters. Only up to one should be non blank.
			//This filter is only locally defined, which is why we have to apply it here instead of the marketing utils.
		CString strPrimaryReferralFilter = GetConsToProcPrimaryReferralFilter();
		CString strMultiReferralFilter = GetConsToProcMultiReferralFilter();

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the summary query
		strSummarySql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strSummarySql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the referral source query
		strRefSourSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strRefSourSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the referring physicians
		strRefPhysSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strRefPhysSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the referring patients
		strRefPatSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strRefPatSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		_RecordsetPtr rsTotal = CreateRecordset("SELECT Sum(TotalConsultCount) as Total1, \r\n"
			"Sum(TotalConvertedConsultCount) as Total2, \r\n"
			"CASE WHEN SUM(TotalConsultCount) = 0 THEN 0 ELSE 100 * (SUM(TotalConvertedConsultCount) / SUM(TotalConsultCount)) END as Total3 \r\n"
			"FROM (\r\n"
			"%s \r\n) Base ", strSummarySql);

		// (z.manning 2009-09-30 10:41) - Put the shared logic in a utility function
		AddConsToProcDataToGraphDesc(strSummarySql, &desc, bShowNumbers, bShowPercentages, TRUE);

		m_graph->Format = "%0.0f";
		BOOL bIncludeNoReferral = TRUE;
		//(e.lally 2009-09-28) PLID 35594 - If we are filtering on a specific referral, we can't include "no referral"
		if(!m_strFilteredReferralList.IsEmpty()){
			bIncludeNoReferral = FALSE;
		}
		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc, bIncludeNoReferral);
	}
	else if  (mfDateFilter == mfReferralDate) {
		m_graph->PrintTitle = "Referral To Consult To Procedure By Referrals";
		//(e.lally 2009-09-24) PLID 35593 - Show the referral to consult and referral to procedure data.
		//Get the query for the summary
		CString strSummarySql = GetGraphSql(CNVConsToSurgByRefSour, 5, -1, pCon, strPatientTempTable);
		//Get the queries for regular referral source, referring physician, and referring patient
		strRefSourSql = GetGraphSql(CNVConsToSurgByRefSour, 5, 1, pCon, strPatientTempTable);
		strRefPhysSql = GetGraphSql(CNVConsToSurgByRefSour, 5, 2, pCon, strPatientTempTable);
		strRefPatSql = GetGraphSql(CNVConsToSurgByRefSour, 5, 3, pCon, strPatientTempTable);

		//(e.lally 2009-09-28) PLID 35594 - Gather the referral filters. Only up to one should be non blank
			//This filter is only locally defined, which is why we have to apply it here instead of the marketing utils.
		CString strPrimaryReferralFilter = GetConsToProcPrimaryReferralFilter();
		CString strMultiReferralFilter = GetConsToProcMultiReferralFilter();

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the summary query
		strSummarySql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strSummarySql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the referral source query
		strRefSourSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strRefSourSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the referring physicians
		strRefPhysSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strRefPhysSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		//(e.lally 2009-09-28) PLID 35594 - Apply it to the referring patients
		strRefPatSql.Replace(PRIMARY_REFERRAL_FILTER_PLACEHOLDER, strPrimaryReferralFilter);
		strRefPatSql.Replace(MULTI_REFERRAL_FILTER_PLACEHOLDER, strMultiReferralFilter);

		_RecordsetPtr rsTotal = CreateRecordset("SELECT Sum(TotalReferralCount) AS Total1, \r\n"
			"Sum(TotalConsultCount) as Total2, \r\n"
			"Sum(TotalConvertedConsultCount) as Total3, \r\n"
			"CASE WHEN SUM(TotalReferralCount) = 0 THEN 0 ELSE 100 * (SUM(TotalConsultCount) / SUM(TotalReferralCount)) END as Total4, \r\n"
			"CASE WHEN SUM(TotalReferralCount) = 0 THEN 0 ELSE 100 * (SUM(TotalConvertedConsultCount) / SUM(TotalReferralCount)) END as Total5 \r\n"
			"FROM (\r\n"
			"%s \r\n) Base ", strSummarySql);

		CString strTempDesc1, strTempDesc2, strTempDesc3, strTempDesc4, strTempDesc5;
		strTempDesc1.Format("%s (%s Total) ", "Referrals", AsString(rsTotal->Fields->Item["Total1"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total1"]->Value));
		strTempDesc2.Format("%s (%s Total) ", "Consults", AsString(rsTotal->Fields->Item["Total2"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total2"]->Value));
		strTempDesc3.Format("%s (%s Total) ", "Procedures", AsString(rsTotal->Fields->Item["Total3"]->Value).IsEmpty() ? "0" : AsString(rsTotal->Fields->Item["Total3"]->Value));
		strTempDesc4.Format("%s (%.0f%% Total) ", "Referral To Consult", AsString(rsTotal->Fields->Item["Total4"]->Value).IsEmpty() ? 0 : AdoFldDouble(rsTotal, "Total4"));
		strTempDesc5.Format("%s (%.0f%% Total) ", "Referral To Procedure", AsString(rsTotal->Fields->Item["Total5"]->Value).IsEmpty() ? 0 : AdoFldDouble(rsTotal, "Total5"));

		//Make the descriptions reflect the changes to the base query
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		//
		//(e.lally 2009-09-25) PLID 35593 - ****No need to separate out the consults here****
		//
		// (z.manning 2009-09-30 10:54) - PLID 35051 - Handle the show number/percent radio buttons
		if(bShowNumbers) {
			desc.Add(strTempDesc1, "TotalReferralCount", GetMarketGraphColor(mgcBrightBlue), "TotalReferralCount");
			desc.Add(strTempDesc2, "TotalConsultCount", GetMarketGraphColor(mgcDarkGreen), "TotalConsultCount");
			desc.Add(strTempDesc3, "TotalConvertedConsultCount", GetMarketGraphColor(mgcBrightRed), "TotalConvertedConsultCount");
		}
		if(bShowPercentages) {
			//Use the new percent operation because we can't add up the child percents
				//and the division one forces us to send in an inflated field1 amount which won't work either.
			desc.Add(strTempDesc4, "TotalConsultCount", GetMarketGraphColor(mgcBrightPurple), "TotalReferralCount", GraphDescript::GD_PERCENT);
			desc.Add(strTempDesc5, "TotalConvertedConsultCount", GetMarketGraphColor(mgcBrightOrange), "TotalReferralCount", GraphDescript::GD_PERCENT);
		}

		m_graph->Format = "%0.0f";
		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc, FALSE);
	}

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}


void CMarketReferralSourceDlg::Graph(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		//(e.lally 2009-09-18) - This code is used only by Inquiry to Consult, but doesn't really need to be
		EnsureRemoteData();

		//declare variables
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		_RecordsetPtr	rsReferrals = NULL,
						rs = NULL;
		int				i=0, 
						j=0, 
						k=0,
						max=0;
		double			total=0, total2=0;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		//m_progress.SetPos(0);

		//get referrals

		//up the timeout
		CIncreaseCommandTimeout ict(600);

		// (j.gruber 2009-06-23 11:31) - PLID 34227 - filter on only the referral sources they want to see
		CString strFilter;

		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now

		//if they are drilled down, we are keeping that filter
		if (m_nCurrentDrilledDownReferral == -1) {
			if (m_dwFilteredRefIDList.GetSize() > 1) {
				strFilter.Format(" AND (PersonID IN %s) ", m_strFilteredReferralList);			
			}
			else if (m_nCurrentFilteredReferralID != -1) {
				strFilter.Format(" AND (PersonID = %li) ", m_nCurrentFilteredReferralID);
			}
			else {
				//TES 7/22/2009 - PLID 34227 - We still need this, so that child referral sources won't show on the top level.
				strFilter.Format(" AND (Parent = %li or PersonID = %li) ", m_nCurrentDrilledDownReferral, m_nCurrentDrilledDownReferral);
			}
		}
		else {
			strFilter.Format(" AND (Parent = %li or PersonID = %li) ", m_nCurrentDrilledDownReferral, m_nCurrentDrilledDownReferral);
		}
		
		sql.Format("SELECT (CASE WHEN PersonID = %li THEN 'ZZZZZZZ' ELSE ReferralSourceT.Name END) AS Name, "
			"PersonID FROM ReferralSourceT "
			"WHERE (1=1) %s "
			"ORDER BY Name;", m_nCurrentDrilledDownReferral, strFilter);

		rsReferrals = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsReferrals->RecordCount;
		m_graph->RowCount = max;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(30);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			//(e.lally 2009-09-18) PLID 35559 - Added support for percent operations, automatically set the column format.
			if(desc.Label(i).Find("Percentage") != -1 || desc.Label(i) == "Conversion Rate" 
				|| desc.Op(i) == GraphDescript::GD_PERCENT){
				m_graph->ColumnFormat = "%0.0f%%";
			}
			else
				m_graph->ColumnFormat = m_graph->Format;
			
		}

		m_graph->XAxisDesc = "Referral Source";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);
		//set the timeout back
		ict.Reset();

		//m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int currentID=0;
		i = 0;
		while (!rsReferrals->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsReferrals->Fields->GetItem("Name")->Value);
			m_graph->RowID = AdoFldLong(rsReferrals, "PersonID");

			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = rsReferrals->Fields->GetItem("PersonID")->Value.lVal;
			descendants.Add(currentID);
			if (currentID != m_nCurrentDrilledDownReferral) //special [Other] column
				Descendants(descendants);
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
						{	if (descendants[k] == AdoFldLong(rs, "ReferralID"))
							{	
								total += AdoFldDouble(rs, desc.Field(j));
								//(e.lally 2009-09-18) PLID 35559 - Added support for percent operations, follows same logic as division
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
					//(e.lally 2009-09-18) PLID 35559 - Added support for percent operations
					case GraphDescript::GD_PERCENT:
						desc.AddToTotal(j, total);
						desc.AddToTotal2(j, total2);
						if (total2 != 0){
							double dblResult = total / total2;
							//(e.lally 2009-09-18) PLID 35559 - percent mode expects 1/2 to be 50, not 0.5 
								//so we need to multiply the resuly by 100
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

			rsReferrals->MoveNext();
			m_progress.SetPos(50 + ((50 * i) / max));
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}

void CMarketReferralSourceDlg::OnInqToCons() 
{
	if (LastChecked(IDC_INQ_TO_CONS_BY_PROC)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != REFInqtoCons) {
		SetType(REFInqtoCons);
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::InqToCons() 
{
	
	CWaitCursor wc;

	//This is the "Conversion Rate" button

	m_graph->PrintTitle = "Inquiry To Consult By Referrals";

	
	m_progress.SetPos(5);

	GraphDescript desc;
	CString sql, from, to, provIDs, strPersonProvFilter, strApptProvFilter, strPersonLocFilter, strApptLocFilter, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
	//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
	int nCategoryID=0, nResp=0;

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);
	
	if (UseFilter(mftProvider))  {
		CString strFilter1, strFilter2;
		ParseMultiFilter(strProvField, strFilter1, strFilter2);
		
		//fill these, then we will clear them if we need to 
		strPersonProvFilter.Format(" AND %s %s) ", strFilter1, provIDs);
		strApptProvFilter.Format(" AND %s %s) ", strFilter2, provIDs);

		if (strFilter1.IsEmpty()) {
			strPersonProvFilter = "";
		}
				
		if (strFilter2.IsEmpty()) {
			strApptProvFilter = "";
		}
	}
	if (UseFilter(mftLocation)) {
		CString strFilter1, strFilter2;
		ParseMultiFilter(strLocationField, strFilter1, strFilter2);
		
		//fill these, then we will clear them if we need to 
		strPersonLocFilter.Format(" AND %s IN %s ", strFilter1, locIDs);
		strApptLocFilter.Format(" AND %s IN %s ", strFilter2, locIDs);

		if (strFilter1.IsEmpty()) {
			strPersonLocFilter = "";
		}
				
		if (strFilter2.IsEmpty()) {
			strApptLocFilter = "";
		}
	}	
	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


	//
	/////////////
	//Generate the sql query that will be used to build the graph
	//
	/////////////
	//Select:	ReferralID
	//			Number of patients * 100.0 as Conv (purple bar)
	/////////////
	//SubQ's:	SubQ
	//			Selects PatientID
	//		Filter:
	//			Category is Surgery or Minor Procedure or Other Procedure
	//			Filters out No Show's
	//			Filters out Cancelled
	//		Groups by:
	//			PatientID
	/////////////
	//Filter:	First Contact Date chosen in toolbar
	//			Provider (chosen in toolbar)
	//			Referral Source (if chosen, in the graph)
	//			Location (chosen in toolbar)
	/////////////
	//

	sql = GetGraphSql(REFInqtoCons, -1, -1, pCon, strPatientTempTable);
	
	
	CString strFrom, strTo;
	if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}


	
	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov]", strPersonProvFilter);
	sql.Replace("[ApptProv]", strApptProvFilter);
	sql.Replace("[Loc]", strPersonLocFilter);
	sql.Replace("[ApptLoc]", strApptLocFilter);
	sql.Replace("[PatFilter]", strPatFilter);

	m_progress.SetPos(10);
	// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
	// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
	// the same effect as filtering on the set of all referrals (which is what we used to do but it 
	// was slower, especially if you had a ton of referrals).
	// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
	// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
	// so I cleaned up the code so that is much more obvious now
	CString referrals = CString("AND ") + ((m_nCurrentDrilledDownReferral == -1) ? "PatientsT.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "PatientsT.ReferralID"));
	sql.Replace("[Ref]", referrals);

	desc.m_sql = sql;
	
	m_progress.SetPos(15);
	/*rsTotal = CreateRecordset("SELECT Sum(ProsWithCons) AS SumNum, Sum(TotalPros) AS SumDenom From (%s) BASE", sql);
	dblRefSourTotal1 = AdoFldDouble(rsTotal, "SumNum", 0);
	dblRefSourDenom = AdoFldDouble(rsTotal, "SumDenom", 0);

	double dblPercent1;
*/
	m_progress.SetPos(20);
	
/*	if (dblRefSourDenom== 0) {
		dblPercent1 = 0;
	}
	else {
		dblPercent1 = dblRefSourTotal1/dblRefSourDenom;				
	}

	
	CString strTempDesc1;
	strTempDesc1.Format("%s (%.02f%% Total) ", "Percentage", dblPercent1);

	desc.Add(strTempDesc1, "ProsWithCons", RGB(0,0,255), "TotalPros", GraphDescript::GD_DIV);
	*/
	
	CString strTempDesc1;
	strTempDesc1.Format("%s", "Percentage");

	desc.Add(strTempDesc1, "ProsWithCons", RGB(0,0,255), "TotalPros", GraphDescript::GD_DIV);

	m_graph->Format = "%0.0f%%";
	Graph(desc);

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

void CMarketReferralSourceDlg::OnProcsClosed() 
{
	if (LastChecked(IDC_PROCS_CLOSED2)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != REFSchedVClosed) {
		SetType(REFSchedVClosed);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::ProcsClosed() 
{
	
	CWaitCursor wc;

	try {

		m_graph->PrintTitle = "Procedures Performed vs. Closed By Referrals";
		

		GraphDescript desc;
		CString from, to, docFilter, provIDs, locFilter, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int nCategoryID=0, nResp=0;
		
		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//setup parameters that will be used in the query
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		if (UseFilter(mftProvider)){
			docFilter.Format(" AND %s %s )", strProvField, provIDs);
		}
		if (locIDs != "") //user personT location instead of scheduler location
			locFilter.Format(" AND %s IN %s ", strLocationField, locIDs);
		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


		//
		/////////////
		//Generate the sql query that will be used to build the graph
		//
		/////////////
		//Select:	ReferralID
		//			Number of patients * 100.0 as Conv (purple bar)
		/////////////
		//SubQ's:	SubQ
		//			Selects PatientID
		//		Filter:
		//			Category is Surgery or Minor Procedure or Other Procedure
		//			Filters out No Show's
		//			Filters out Cancelled
		//		Groups by:
		//			PatientID
		/////////////
		//Filter:	First Contact Date chosen in toolbar
		//			Provider (chosen in toolbar)
		//			Referral Source (if chosen, in the graph)
		//			Location (chosen in toolbar)
		/////////////
		//

		CString strRefSourSql, strRefPhysSql, strRefPatSql;

		long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
		long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
		strRefSourSql = GetGraphSql(REFSchedVClosed, 1, -1, pCon, strPatientTempTable);
		
		if (nRefPhyID > 0) {
			//this is an odd place for the replace, but we have to do it before its combined
			CString strTmp = GetGraphSql(REFSchedVClosed, 1, -1, pCon, strPatientTempTable);
			strTmp.Replace("[ref]", Descendants(nRefPhyID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", " AND PatientsT.DefaultReferringPhyID IS NULL ");
			strTmp.Replace("[RefPatPref]", "");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-2 AS ");
			strRefPhysSql =  strTmp + " UNION ALL " + GetGraphSql(REFSchedVClosed, 2, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPhysSql = GetGraphSql(REFSchedVClosed, 2, -1, pCon, strPatientTempTable);
			strRefPhysSql.Replace("[RefPhysPref]", "");
		}


		if (nRefPatID > 0) {
			CString strTmp = GetGraphSql(REFSchedVClosed, 1, -1, pCon, strPatientTempTable);
			//this is an odd place for the replace, but we have to do it before its combined
			//str2.Format(" AND ReferralSource = %li OR ReferralSource IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);*/
			strTmp.Replace("[ref]", Descendants(nRefPatID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", "");
			strTmp.Replace("[RefPatPref]", " AND PatientsT.ReferringPatientID IS NULL ");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-3 AS ");
			strRefPatSql =  strTmp + " UNION ALL " + GetGraphSql(REFSchedVClosed, 3, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPatSql = GetGraphSql(REFSchedVClosed, 3, -1, pCon, strPatientTempTable);
			strRefPatSql.Replace("[RefPatPref]", "");
		}

		
		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		CString referrals = CString("AND ") + ((m_nCurrentDrilledDownReferral == -1) ? "ReferralsQ.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "ReferralsQ.ReferralID"));

		//PLID 14894: take out any referral sources of people that also have referring phys or referring pats
		CString strRefPatReplace;
		if (nRefPatID > 0) {
			//strRefPatReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);
			strRefPatReplace = Descendants(nRefPatID, "AND ReferralID NOT ");
		}
		else {
			strRefPatReplace = "";
		}
		
		CString strRefPhysReplace;
		if (nRefPhyID > 0) {
			//strRefPhysReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPhyID, nRefPhyID);
			strRefPhysReplace = Descendants(nRefPhyID, "AND ReferralID NOT ");
		}
		else {
			strRefPhysReplace = "";
		}
		strRefSourSql.Replace("[TakeOutForRefPhys]", strRefPhysReplace);
		strRefSourSql.Replace("[TakeOutForRefPat]", strRefPatReplace);
		
		strRefSourSql.Replace("[From]", strFrom);
		strRefSourSql.Replace("[To]", strTo);
		strRefSourSql.Replace("[Prov]", docFilter);
		strRefSourSql.Replace("[Loc]", locFilter);
		strRefSourSql.Replace("[PatFilter]", strPatFilter);
		strRefSourSql.Replace("[ref]", referrals);
		strRefSourSql.Replace("[ReplacePref]", "");
		strRefSourSql.Replace("[RefPatPref]", "");
		strRefSourSql.Replace("[RefPhysPref]", "");


		strRefPhysSql.Replace("[From]", strFrom);
		strRefPhysSql.Replace("[To]", strTo);
		strRefPhysSql.Replace("[Prov]", docFilter);
		strRefPhysSql.Replace("[Loc]", locFilter);
		strRefPhysSql.Replace("[PatFilter]", strPatFilter);
		strRefPhysSql.Replace("[ref]", referrals);

		strRefPatSql.Replace("[From]", strFrom);
		strRefPatSql.Replace("[To]", strTo);
		strRefPatSql.Replace("[Prov]", docFilter);
		strRefPatSql.Replace("[Loc]", locFilter);
		strRefPatSql.Replace("[PatFilter]", strPatFilter);
		strRefPatSql.Replace("[ref]", referrals);
		
		CString strTempDesc1, strTempDesc2;
		strTempDesc1.Format("%s", "Performed");
		strTempDesc2.Format("%s", "Closed");
			
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		desc.Add(strTempDesc1, "ProcsPerformed", GetMarketGraphColor(mgcBrightBlue));
		desc.Add(strTempDesc2, "ProcsClosed", GetMarketGraphColor(mgcBrightRed));
		
		m_graph->Format = "%0.0f";
		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc);

		for (int i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
			m_graph->ColumnText = (_bstr_t)strText;
			m_graph->ColumnFormat = m_graph->Format;
		}

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error Generating Graph");
}

void CMarketReferralSourceDlg::OnProsToCons() 
{
	if (LastChecked(IDC_PROS_TO_CONS_BY_PROC2)) return;

	GetDlgItem(IDC_SHOW_INQUIRIES)->ShowWindow(SW_HIDE);
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:00) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != REFProsToCons) {
		SetType(REFProsToCons);
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketReferralSourceDlg::ProsToCons() 
{
	
	CWaitCursor wc;

	try {
		//This is the "Conversion Rate" button

		m_graph->PrintTitle = "Prospect to Consults By Referrals";
		
		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		GraphDescript desc;
		CString from, to, strPersonProv, strApptProv, provIDs, strPersonLoc, strApptLoc, locIDs, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
		//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
		int nCategoryID=0, nResp=0;

		//setup parameters that will be used in the query
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);
		
		if (UseFilter(mftProvider))  {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strProvField, strFilter1, strFilter2);
			
			//fill these, then we will clear them if we need to 
			strPersonProv.Format(" AND %s %s) ", strFilter1, provIDs);
			strApptProv.Format(" AND %s %s) ", strFilter2, provIDs);

			if (strFilter1.IsEmpty()) {
				strPersonProv = "";
			}
					
			if (strFilter2.IsEmpty()) {
				strApptProv = "";
			}
		}
		if (UseFilter(mftLocation))  {
			CString strFilter1, strFilter2;
			ParseMultiFilter(strLocationField, strFilter1, strFilter2);
			
			//fill these, then we will clear them if we need to 
			strPersonLoc.Format(" AND %s IN %s ", strFilter1, locIDs);
			strApptLoc.Format(" AND %s IN %s ", strFilter2, locIDs);

			if (strFilter1.IsEmpty()) {
				strPersonLoc = "";
			}
					
			if (strFilter2.IsEmpty()) {
				strApptLoc = "";
			}
		}



		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		CString strRefSourSql, strRefPhysSql, strRefPatSql;

		
		long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
		long nRefPhyID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
		strRefSourSql = GetGraphSql(REFProsToCons, 1, -1, pCon, strPatientTempTable);
		
		if (nRefPhyID > 0) {
			//this is an odd place for the replace, but we have to do it before its combined
			CString strTmp = GetGraphSql(REFProsToCons, 1, -1, pCon, strPatientTempTable);
			strTmp.Replace("[ref]", Descendants(nRefPhyID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", " AND PatientsT.DefaultReferringPhyID IS NULL ");
			strTmp.Replace("[RefPatPref]", "");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-2 AS ");
			strRefPhysSql =  strTmp + " UNION ALL " + GetGraphSql(REFProsToCons, 2, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPhysSql = GetGraphSql(REFProsToCons, 2, -1, pCon, strPatientTempTable);
			strRefPhysSql.Replace("[RefPhysPref]", "");
		}


		if (nRefPatID > 0) {
			CString strTmp = GetGraphSql(REFProsToCons, 1, -1, pCon, strPatientTempTable);
			//this is an odd place for the replace, but we have to do it before its combined
			//str2.Format(" AND ReferralSource = %li OR ReferralSource IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);*/
			strTmp.Replace("[ref]", Descendants(nRefPatID, "AND ReferralID"));
			strTmp.Replace("[RefPhysPref]", "");
			strTmp.Replace("[RefPatPref]", " AND PatientsT.ReferringPatientID IS NULL ");
			strTmp.Replace("[TakeOutForRefPhys]", "");
			strTmp.Replace("[TakeOutForRefPat]", "");
			strTmp.Replace("[ReplacePref]", "-3 AS ");
			strRefPatSql =  strTmp + " UNION ALL " + GetGraphSql(REFProsToCons, 3, -1, pCon, strPatientTempTable);
		}
		else {
			strRefPatSql = GetGraphSql(REFProsToCons, 3, -1, pCon, strPatientTempTable);
			strRefPatSql.Replace("[RefPatPref]", "");
		}
		
		
		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
		// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
		// the same effect as filtering on the set of all referrals (which is what we used to do but it 
		// was slower, especially if you had a ton of referrals).
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now
		CString referrals = CString("AND ") + ((m_nCurrentDrilledDownReferral == -1) ? "ReferralsQ.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "ReferralsQ.ReferralID"));

		//PLID 14894: take out any referral sources of people that also have referring phys or referring pats
		CString strRefPatReplace;
		if (nRefPatID > 0) {
			//strRefPatReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPatID, nRefPatID);
			strRefPatReplace = Descendants(nRefPatID, "AND ReferralID NOT ");
		}
		else {
			strRefPatReplace = "";
		}
		
		CString strRefPhysReplace;
		if (nRefPhyID > 0) {
			//strRefPhysReplace.Format(" AND PatientsT.ReferralID <> %li AND PatientsT.ReferralID NOT IN (SELECT PersonID FROM ReferralSourceT WHERE ParentID = %li) ", nRefPhyID, nRefPhyID);
			strRefPhysReplace = Descendants(nRefPhyID, "AND ReferralID NOT ");
		}
		else {
			strRefPhysReplace = "";
		}
		strRefSourSql.Replace("[TakeOutForRefPhys]", strRefPhysReplace);
		strRefSourSql.Replace("[TakeOutForRefPat]", strRefPatReplace);
		
		strRefSourSql.Replace("[From]", strFrom);
		strRefSourSql.Replace("[To]", strTo);
		strRefSourSql.Replace("[Prov]", strPersonProv);
		strRefSourSql.Replace("[ApptProv]", strApptProv);
		strRefSourSql.Replace("[Loc]", strPersonLoc);
		strRefSourSql.Replace("[ApptLoc]", strApptLoc);
		strRefSourSql.Replace("[PatFilter]", strPatFilter);
		strRefSourSql.Replace("[ref]", referrals);
		strRefSourSql.Replace("[ReplacePref]", "");
		strRefSourSql.Replace("[RefPatPref]", "");
		strRefSourSql.Replace("[RefPhysPref]", "");

		strRefPhysSql.Replace("[From]", strFrom);
		strRefPhysSql.Replace("[To]", strTo);
		strRefPhysSql.Replace("[Prov]", strPersonProv);
		strRefPhysSql.Replace("[ApptProv]", strApptProv);
		strRefPhysSql.Replace("[Loc]", strPersonLoc);
		strRefPhysSql.Replace("[ApptLoc]", strApptLoc);
		strRefPhysSql.Replace("[PatFilter]", strPatFilter);
		strRefPhysSql.Replace("[ref]", referrals);

		strRefPatSql.Replace("[From]", strFrom);
		strRefPatSql.Replace("[To]", strTo);
		strRefPatSql.Replace("[Prov]", strPersonProv);
		strRefPatSql.Replace("[ApptProv]", strApptProv);
		strRefPatSql.Replace("[Loc]", strPersonLoc);
		strRefPatSql.Replace("[ApptLoc]", strApptLoc);
		strRefPatSql.Replace("[PatFilter]", strPatFilter);
		strRefPatSql.Replace("[ref]", referrals);

		CString strTempDesc1;
		strTempDesc1.Format("%s", "Percentage");

		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph colors
		desc.Add(strTempDesc1, "ProsWithCons", GetMarketGraphColor(mgcBrightBlue), "TotalPros", GraphDescript::GD_DIV);
		
		m_graph->Format = "%0.0f%%";
		GraphReferrals(strRefSourSql, strRefPhysSql, strRefPatSql, desc);

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

	}NxCatchAll("Error in Prospects To Consults");
}


void CMarketReferralSourceDlg::OnChangeBackButtonPosColumnGraph() {
	
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


CString CMarketReferralSourceDlg::GetCurrentGraphSql() {

	return m_strSql;

}

// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
// (j.jones 2012-06-19 10:16) - PLID 48481 - added strFilter5
void CMarketReferralSourceDlg::GetCurrentGraphFilters(CString &strFilter1, CString &strFilter2, CString &strFilter3, CString &strFilter4, CString &strFilter5,
													  ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable) {

	long nType = GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>");


	CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, category, strCat2, strDateField, strFrom, strTo, strFrom2, strTo2, strPatFilter, PatFilterIDs, strResp, strResp2, strLocationField, strProvField;
	CString strTo3, strFrom3, strTo4, strFrom4;
	//(e.lally 2010-03-11) PLID 37709 - Ensure variables are initialized
	int nCatID=0, nRespID=0;

	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCatID, nRespID, pCon, strPatientTempTable);

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
			
			CString strTempFilter1, strTempFilter2;
			ParseMultiFilter(strLocationField, strTempFilter1, strTempFilter2);
	
			//fill these, then we will clear them if we need to 
			strLocFilter1.Format(" AND %s IN %s ", strTempFilter1, locIDs);
			strLocFilter2.Format(" AND %s IN %s ", strTempFilter2, locIDs);

			if (strTempFilter1.IsEmpty()) {
				strLocFilter1 = "";
			}
			
			if (strTempFilter2.IsEmpty()) {
				strLocFilter2 = "";
			}
		}
		else {
			strLocFilter1.Format(" AND %s IN %s ", strLocationField, locIDs);
		}
	}

	//DRT 4/2/03 - Filter on category if one is chosen.  If it is chosen, we will only
	//		show the income (payments) which are applied to charges of that category.
	if(nCatID != -1) { 
		category.Format(" AND ServiceT.Category = %li", nCatID);
		strCat2.Format(" AND (1 = -1) ");
	}


	if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
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

	if (nRespID == 1)  {
		strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
		strResp2.Format(" AND (RefundsT.InsuredPartyID = -1 OR RefundsT.InsuredPartyID IS NULL)");
	}
	else if (nRespID == 2) {
		strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
		strResp2.Format(" AND RefundsT.InsuredPartyID <> -1 AND RefundsT.InsuredPartyID IS NOT NULL");
	}

	if (PatFilterIDs != "") 
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);


	if (GetType() == EFFMonByRefSour) {

		strFilter1 = " [Resp] [PatFilter] [Category] [From] [To] [Source] ";
		strFilter2 = " [Prov1] [Loc1] ";
		strFilter3 = " [Resp] [PatFilter] [Category2] [From4] [To4] [Source] ";
		strFilter4 = " [Resp2] [PatFilter] [Category2] [From2] [To2] [Source] [From3] [To3] ";
		// (j.jones 2012-06-19 10:42) - PLID 48481 - added referral2 / loc2 filter
		strFilter5 = " [referral2] [Loc2]";
	}
	else { 		

		//PLID 16850 - the only graph that uses category or source is the revenue graph, so I took them out of here
		strFilter1 = " [Source] [PatFilter] [From] [To] [Loc1] [Prov1] ";
		strFilter2 = " [Loc2] [Prov2] ";
	}

	//set the type
	switch (nType) {
		case 1: //referral source
			strFilter1.Replace("Source", "referrals");
			strFilter2.Replace("Source", "referrals");
			strFilter3.Replace("Source", "referrals");
			strFilter4.Replace("Source", "referrals");
		break;

		case 2: //referring phys
			strFilter1.Replace("Source", "RefPhys");
			strFilter2.Replace("Source", "RefPhys");
			strFilter3.Replace("Source", "RefPhys");
			strFilter4.Replace("Source", "RefPhys");
		break;

		case 3:  //referring pat
			strFilter1.Replace("Source", "RefPat");
			strFilter2.Replace("Source", "RefPat");
			strFilter3.Replace("Source", "RefPat");
			strFilter4.Replace("Source", "RefPat");
		break;
	}


	/*//set any special calls to the referrals we need	
	if (GetType() == CNVConsToSurgByRefSour || GetType() == NUMPatByRefSour) {
			referrals = "AND " + ((m_nCurrentDrilledDownReferral == -1) ? "PatientsT.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "PatientsT.ReferralID"));
			strFilter.Replace("[RefSour]", referrals);
		}
	else {
		referrals = "AND " + ((m_nCurrentDrilledDownReferral == -1) ? "MultiReferralsT.ReferralID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "MultiReferralsT.ReferralID"));
		strFilter.Replace("[RefSour]", referrals);
	}*/

	CString strRefSour;
	if (GetType() == REFNoShowByReferral || GetType() == REFSchedVClosed ||
		GetType() == REFProsToCons || GetType() == REFInqtoCons) {
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly cast to CString
		// (j.gruber 2009-06-26 15:30) - PLID 34718 - added filter for reports
		//TES 7/22/2009 - PLID 34718 - We need to check whether we're at the top level first, then we'll know
		// whether to apply our filters.
		// (j.jones 2012-08-07 17:40) - PLID 51058 - the generic id field was the current drilled down referral,
		// so I cleaned up the code so that is much more obvious now

		//check to see if we are at the top
		if (m_nCurrentDrilledDownReferral == -1) {
			if (!m_strFilteredReferralList.IsEmpty()) {			
				//we need to add our filters
				//TES 7/23/2009 - PLID 34718 - Need to include all the descendants as well.
				strRefSour.Format(" AND %s", Descendants(m_dwFilteredRefIDList, "MultiReferralsT.ReferralID"));
			}
			else {
				//they aren't using our filter, so just do is null
				strRefSour = " AND MultiReferralsT.ReferralID IS NOT NULL";
			}
		}
		else {
			//they are drilled down on something, so just use the ID
			strRefSour = CString("AND ") + Descendants(m_nCurrentDrilledDownReferral, "MultiReferralsT.ReferralID");
		}		
		
		strFilter1.Replace("[referrals]", strRefSour);
		strFilter2.Replace("[referrals]", strRefSour);
		strFilter3.Replace("[referrals]", strRefSour);
		strFilter4.Replace("[referrals]", strRefSour);
	}

	// (j.gruber 2009-06-26 15:35) - PLID 34718 - set the refsour string
	CString strRefSour2;
	//TES 7/22/2009 - PLID 34718 - We need to check whether we're at the top level first, then we'll know
	// whether to apply our filters.
	//check to see if we are at the top
	if (m_nCurrentDrilledDownReferral == -1) {
		if (!m_strFilteredReferralList.IsEmpty()) {
			//we need to add our filters
			//TES 7/23/2009 - PLID 34718 - We  need to include all the descendants as well
			strRefSour.Format(" AND %s", Descendants(m_dwFilteredRefIDList, "PatientsT.ReferralID"));
			strRefSour2.Format(" AND %s", Descendants(m_dwFilteredRefIDList, "ReferralSource"));
		}
		else {
			//they aren't using our filter, so just do is null
			strRefSour = " AND PatientsT.ReferralID IS NOT NULL";
			strRefSour2 = " AND ReferralSource IS NOT NULL";
		}
	}
	else {
		//they are drilled down on something, so just use the ID
		strRefSour = CString("AND ") + Descendants(m_nCurrentDrilledDownReferral, "PatientsT.ReferralID");
		strRefSour2 = CString("AND ") + Descendants(m_nCurrentDrilledDownReferral, "ReferralSource");
	}


	// (b.cardillo 2004-08-06 09:30) - PLID 13747 - Made it so if the id is -1, which for referrals is 
	// the sentinel value that indicates "all referrals", we simply exclude NULL referrals, which has 
	// the same effect as filtering on the set of all referrals (which is what we used to do but it 
	// was slower, especially if you had a ton of referrals).
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
	//DRT 5/22/2008 - PLID 29966 - We have to specify the table name!
	// (j.gruber 2009-06-26 15:38) - PLID 34718 - use the filter if it exists
	strFilter1.Replace("[referrals]", strRefSour);
	strFilter1.Replace("[referral2]", strRefSour2);
	strFilter1.Replace("[Loc1]", strLocFilter1);
	strFilter1.Replace("[Category]", category);
	strFilter1.Replace("[Category2]", strCat2);
	strFilter1.Replace("[from]", "'" + from + "'");
	strFilter1.Replace("[to]", "'" + to + "'");
	strFilter1.Replace("[PatFilter]", strPatFilter);
	strFilter1.Replace("[Resp]", strResp);
	strFilter1.Replace("[Resp2]", strResp2);
	strFilter1.Replace("[Loc2]", strLocFilter2);
	strFilter1.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID"));
	strFilter1.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID "));	


	strFilter2.Replace("[From]", strFrom);
	strFilter2.Replace("[To]", strTo);
	strFilter2.Replace("[From2]", strFrom2);
	strFilter2.Replace("[To2]", strTo2);
	strFilter2.Replace("[From3]", strFrom3);
	strFilter2.Replace("[To3]", strTo3);	
	strFilter2.Replace("[From4]", strFrom4);
	strFilter2.Replace("[To4]", strTo4);
	strFilter2.Replace("[Prov1]", strProvFilter1);
	strFilter2.Replace("[Prov2]", strProvFilter2);
	//DRT 5/22/2008 - PLID 29966 - We have to specify the table name!
	// (j.gruber 2009-06-26 15:38) - PLID 34718 - use the filter if it exists
	strFilter2.Replace("[referrals]", strRefSour);
	strFilter2.Replace("[referral2]", strRefSour2);
	strFilter2.Replace("[Loc1]", strLocFilter1);
	strFilter2.Replace("[Category]", category);
	strFilter2.Replace("[Category2]", strCat2);
	strFilter2.Replace("[from]", "'" + from + "'");
	strFilter2.Replace("[to]", "'" + to + "'");
	strFilter2.Replace("[PatFilter]", strPatFilter);
	strFilter2.Replace("[Resp]", strResp);
	strFilter2.Replace("[Resp2]", strResp2);
	strFilter2.Replace("[Loc2]", strLocFilter2);
	strFilter2.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID"));
	strFilter2.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID "));	

	strFilter3.Replace("[From]", strFrom);
	strFilter3.Replace("[To]", strTo);
	strFilter3.Replace("[From2]", strFrom2);
	strFilter3.Replace("[To2]", strTo2);
	strFilter3.Replace("[From3]", strFrom3);
	strFilter3.Replace("[To3]", strTo3);	
	strFilter3.Replace("[From4]", strFrom4);
	strFilter3.Replace("[To4]", strTo4);
	strFilter3.Replace("[Prov1]", strProvFilter1);
	strFilter3.Replace("[Prov2]", strProvFilter2);
	//DRT 5/22/2008 - PLID 29966 - We have to specify the table name!
	// (j.gruber 2009-06-26 15:38) - PLID 34718 - use the filter if it exists
	strFilter3.Replace("[referrals]", strRefSour);
	strFilter3.Replace("[referral2]", strRefSour2);
	strFilter3.Replace("[Loc1]", strLocFilter1);
	strFilter3.Replace("[Category]", category);
	strFilter3.Replace("[Category2]", strCat2);
	strFilter3.Replace("[from]", "'" + from + "'");
	strFilter3.Replace("[to]", "'" + to + "'");
	strFilter3.Replace("[PatFilter]", strPatFilter);
	strFilter3.Replace("[Resp]", strResp);
	strFilter3.Replace("[Resp2]", strResp2);
	strFilter3.Replace("[Loc2]", strLocFilter2);
	strFilter3.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID"));
	strFilter3.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID "));	


	strFilter4.Replace("[From]", strFrom);
	strFilter4.Replace("[To]", strTo);
	strFilter4.Replace("[From2]", strFrom2);
	strFilter4.Replace("[To2]", strTo2);
	strFilter4.Replace("[From3]", strFrom3);
	strFilter4.Replace("[To3]", strTo3);	
	strFilter4.Replace("[From4]", strFrom4);
	strFilter4.Replace("[To4]", strTo4);
	strFilter4.Replace("[Prov1]", strProvFilter1);
	strFilter4.Replace("[Prov2]", strProvFilter2);
	//DRT 5/22/2008 - PLID 29966 - We have to specify the table name!
	// (j.gruber 2009-06-26 15:38) - PLID 34718 - use the filter if it exists
	strFilter4.Replace("[referrals]", strRefSour);
	strFilter4.Replace("[referral2]", strRefSour2);
	strFilter4.Replace("[Loc1]", strLocFilter1);
	strFilter4.Replace("[Category]", category);
	strFilter4.Replace("[Category2]", strCat2);
	strFilter4.Replace("[from]", "'" + from + "'");
	strFilter4.Replace("[to]", "'" + to + "'");
	strFilter4.Replace("[PatFilter]", strPatFilter);
	strFilter4.Replace("[Resp]", strResp);
	strFilter4.Replace("[Resp2]", strResp2);
	strFilter4.Replace("[Loc2]", strLocFilter2);
	strFilter4.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID"));
	strFilter4.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID "));	

	// (j.jones 2012-06-19 10:42) - PLID 48481 - added filter5
	strFilter5.Replace("[From]", strFrom);
	strFilter5.Replace("[To]", strTo);
	strFilter5.Replace("[From2]", strFrom2);
	strFilter5.Replace("[To2]", strTo2);
	strFilter5.Replace("[From3]", strFrom3);
	strFilter5.Replace("[To3]", strTo3);	
	strFilter5.Replace("[From4]", strFrom4);
	strFilter5.Replace("[To4]", strTo4);
	strFilter5.Replace("[Prov1]", strProvFilter1);
	strFilter5.Replace("[Prov2]", strProvFilter2);
	strFilter5.Replace("[referrals]", strRefSour);
	strFilter5.Replace("[referral2]", strRefSour2);
	strFilter5.Replace("[Loc1]", strLocFilter1);
	strFilter5.Replace("[Category]", category);
	strFilter5.Replace("[Category2]", strCat2);
	strFilter5.Replace("[from]", "'" + from + "'");
	strFilter5.Replace("[to]", "'" + to + "'");
	strFilter5.Replace("[PatFilter]", strPatFilter);
	strFilter5.Replace("[Resp]", strResp);
	strFilter5.Replace("[Resp2]", strResp2);
	strFilter5.Replace("[Loc2]", strLocFilter2);
	strFilter5.Replace("[RefPhys]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.DefaultReferringPhyID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.DefaultReferringPhyID"));
	strFilter5.Replace("[RefPat]", (m_nCurrentDrilledDownReferral == -1) ? "AND PatientsT.ReferringPatientID IS NOT NULL" : Descendants(m_nCurrentDrilledDownReferral, "AND PatientsT.ReferringPatientID "));	
}


void CMarketReferralSourceDlg::Print(CDC *pDC, CPrintInfo *pInfo)
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

				m_graph->ExtraFilter = _bstr_t(GetReferralFilterDescription());
			}
		}
		m_graph->Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
	}

}

void CMarketReferralSourceDlg::OnShowInquiries() {
	
	//UpdateView();
	ResetGraph();
} 

void CMarketReferralSourceDlg::OnTimer(UINT nIDEvent) 
{
	switch (nIDEvent) {
		case LOAD_SCREEN:
			KillTimer(LOAD_SCREEN);
			//load the revenue by money because that is the only time we call this time
			MoneyByRefSour();
		break;
	}	
	CMarketingDlg::OnTimer(nIDEvent);
}

// a.walling PLID 20695 This is the only way to render the graph now!
void CMarketReferralSourceDlg::OnGo() 
{
	m_bRenderedOnce = true;
	UpdateView();
}

void CMarketReferralSourceDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	if (!bShow) {
		LastChecked(-1);
	}

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

void CMarketReferralSourceDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
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

		if (strTitle)
			m_graph->Title = (LPCSTR)strTitle; // change the title to invalidate the graph and force a refresh
		else
			m_graph->Title = "";

		m_bGraphEmpty = true;
	}

	// (j.gruber 2009-06-23 15:32) - PLID 34227 - reset the graph to the top
	m_nCurrentDrilledDownReferral = -1;

	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOW);
	InvalidateConsToProcRadioButtons();
	// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
	InvalidateShowInquiriesButton();
}

bool CMarketReferralSourceDlg::LastChecked(int nID) 
{
	if ( (m_nLastChecked == nID) && m_bActive )
		return true;
	
	m_nLastChecked = nID;
	return false;
}

LRESULT CMarketReferralSourceDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent

	GetDlgItem(IDC_GO)->EnableWindow(true);
	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOWNA);

	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}

// (j.gruber 2009-06-24 10:49) - PLID 34227 - make graphs filterable by referral source
LRESULT CMarketReferralSourceDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MARKET_REF_SOURCE_LINK:
			if (SelectMultiReferrals()) {
				//SetRemotePropertyText("CRGPurposeList", m_strPurposeList, 0, "<None>");
				ResetGraph();
			}
			break;
		
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CMarketReferralSourceDlg::OnLabelClick");
	return 0;
}

BOOL CMarketReferralSourceDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_dwFilteredRefIDList.GetSize() > 1) 
		{
			CRect rc;
			GetDlgItem(IDC_MARKET_REF_SOURCE_FILTER)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll("Error in CMarketReferralSourceDlg::OnSetCursor");
	
	return CMarketingDlg::OnSetCursor(pWnd, nHitTest, message);
}


void CMarketReferralSourceDlg::ShowDescendantWarning() {

	BOOL bFound = FALSE;

	for (int i = 0; i < m_dwFilteredRefIDList.GetSize(); i++) {

		long nReferralID = m_dwFilteredRefIDList.GetAt(i);

		//get all the descendants for this referral
		CString strDescendants = Descendants(nReferralID, "");

		//now run though our list again and make sure that none of them are descendants
		for (int j = 0; j < m_dwFilteredRefIDList.GetSize(); j++) {
			long nReferralIDToCheck = m_dwFilteredRefIDList.GetAt(j);

			if (nReferralIDToCheck != nReferralID) {
				if (strDescendants.Find(AsString(nReferralIDToCheck)) != -1) {
					bFound = TRUE;
				}
			}
		}
	}

	if (bFound) {
		//spit out the warning
		MsgBox("You have selected at least one referral source that is a sub-node of another referral source you selected.\n This will cause duplication in the summary numbers.\n Please change your selections.");
	}

}

BOOL CMarketReferralSourceDlg::SelectMultiReferrals() {

	// (j.jones 2012-08-07 17:25) - PLID 51058 - renamed the variable used here
	// to clarify that they reflect the selection from the dropdown filter,
	// not the graph drilldown data

	CString strFrom, strWhere;
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ReferralSourceT");
	HRESULT hRes;
	bool bDontFill = false;

	long nResult = 0;
	dlg.PreSelect(m_dwFilteredRefIDList);
	
	dlg.m_strNameColTitle = "Referral Source";

	strFrom = "ReferralSourceT ";

	hRes = dlg.Open(strFrom, strWhere, "ReferralSourceT.PersonID", "ReferralSourceT.Name", "Please select the Referral Sources you want to see.", 1);

	//better safe the sorry
	BOOL bReturn = TRUE;
	
	// Update our array of procedures with this information
	if (hRes == IDOK)
	{
		dlg.FillArrayWithIDs(m_dwFilteredRefIDList);
		m_strFilteredReferralList = "(" + dlg.GetMultiSelectIDString(",") + ")";
		bReturn = TRUE;

		if(m_dwFilteredRefIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MARKET_REF_SOURCE_FILTER, SW_HIDE);
			CString strNames = dlg.GetMultiSelectString();
			m_nxlReferralLabel.SetText(strNames);
			m_nxlReferralLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MARKET_REF_SOURCE_LINK, SW_SHOW);			
			m_nxlReferralLabel.Invalidate();
			m_nCurrentFilteredReferralID = -2;

			ShowDescendantWarning();
		}
		else if(m_dwFilteredRefIDList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MARKET_REF_SOURCE_LINK, SW_HIDE);
			ShowDlgItem(IDC_MARKET_REF_SOURCE_FILTER, SW_SHOW);
			m_pReferralSourceList->SetSelByColumn(0, (long)m_dwFilteredRefIDList.GetAt(0));
			m_nCurrentFilteredReferralID = (long)m_dwFilteredRefIDList.GetAt(0);			

			// (j.jones 2012-08-07 17:59) - PLID 51058 - keep this list up to date
			m_dwFilteredRefIDList.RemoveAll();
			m_dwFilteredRefIDList.Add(m_nCurrentFilteredReferralID);
		}
		else {
			//They didn't select any.  But we told multiselect dlg they had to pick at least one!
			ASSERT(FALSE);
		}
	}
	else {
		bReturn = FALSE;
		//Check if they have "multiple" selected
		if(m_dwFilteredRefIDList.GetSize() > 1) {
			ShowDlgItem(IDC_MARKET_REF_SOURCE_FILTER, SW_HIDE);
			m_nxlReferralLabel.SetText(GetReferralNamesFromIDString(m_strFilteredReferralList));
			m_nxlReferralLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MARKET_REF_SOURCE_LINK, SW_SHOW);
			InvalidateDlgItem(IDC_MARKET_REF_SOURCE_LINK);
		}
		else {
			//They selected exactly one. (even if that one was "<No Purpose>"
			ShowDlgItem(IDC_MARKET_REF_SOURCE_LINK, SW_HIDE);
			ShowDlgItem(IDC_MARKET_REF_SOURCE_FILTER, SW_SHOW);
			m_pReferralSourceList->SetSelByColumn(0, m_nCurrentFilteredReferralID);
		}
	}

	return bReturn;
	
}


CString CMarketReferralSourceDlg::GetReferralNamesFromIDString(CString strIDs) {

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
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReferralSourceList->FindByColumn(0, (long)atoi(strIDs), 0, FALSE);
		if (pRow) {
			return VarString(pRow->GetValue(rlcName));
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
		pRow = m_pReferralSourceList->GetFirstRow();		
		
		while (pRow) {
			
			long nID = VarLong(pRow->GetValue(rlcID));
			long nVal = mapIDs.Lookup(nID, nID);
			if (nVal > 0) {
				strReturn += VarString(pRow->GetValue(rlcName)) + ", ";
			}
			pRow = pRow->GetNextRow();
		}

		//take the last comma off
		strReturn = strReturn.Left(strReturn.GetLength() - 2);
	}

	return strReturn;
}

void CMarketReferralSourceDlg::OnRequeryFinishedReferralSources(short nFlags)
{
	try {
		// (a.wilson 2012-5-21) PLID 50378 - change all inactive rows to gray
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReferralSourceList->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {
			pRow->PutForeColor((VarBool(pRow->GetValue(rlcInactive)) ? RGB(143, 148, 152) : RGB(0, 0, 0)));
			pRow->PutForeColorSel((VarBool(pRow->GetValue(rlcInactive)) ? RGB(143, 148, 152) : RGB(0, 0, 0)));
		}

		//add the multiple and the all
		//(e.lally 2009-09-28) PLID 35594 - Put at the top, not just sorted.
		NXDATALIST2Lib::IRowSettingsPtr pRowMulti, pRowAll;
		pRowMulti = m_pReferralSourceList->GetNewRow();
		pRowMulti->PutValue(rlcID, (long)-2);
		pRowMulti->PutValue(rlcName, _variant_t("<Multiple Referral Sources>"));
		m_pReferralSourceList->AddRowBefore(pRowMulti, m_pReferralSourceList->GetFirstRow());

		pRowAll = m_pReferralSourceList->GetNewRow();
		pRowAll->PutValue(rlcID, (long)-1);
		pRowAll->PutValue(rlcName, _variant_t("<All Referral Sources>"));
		m_pReferralSourceList->AddRowBefore(pRowAll, pRowMulti);

		//set the filter
		m_pReferralSourceList->CurSel = m_pReferralSourceList->FindByColumn(rlcID, m_nCurrentFilteredReferralID, NULL, VARIANT_FALSE);
		
		// (j.jones 2012-08-07 17:55) - PLID 51058 - keep our values up to date to reflect if the selected referral still exists
		if(m_pReferralSourceList->CurSel == NULL) {
			m_strFilteredReferralList = "";
			m_nCurrentFilteredReferralID = -1;
			m_dwFilteredRefIDList.RemoveAll();
		}
		else if(m_nCurrentFilteredReferralID != -2 && m_nCurrentFilteredReferralID != -1) {
			m_strFilteredReferralList.Format("(%li)", m_nCurrentFilteredReferralID);
			m_dwFilteredRefIDList.RemoveAll();
			m_dwFilteredRefIDList.Add(m_nCurrentFilteredReferralID);
		}

	}NxCatchAll("Error in CMarketReferralSourceDlg::OnRequeryFinishedReferralSources");
}

void CMarketReferralSourceDlg::OnSelChosenReferralSourceList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow == NULL) {	
			m_pReferralSourceList->SetSelByColumn(rlcID, -1);
			m_strFilteredReferralList = "";
			m_nCurrentFilteredReferralID = -1;
			m_dwFilteredRefIDList.RemoveAll();
			return;
		}

		long nID = VarLong(pRow->GetValue(rlcID));

		if (nID == -2) {

			//they want multipurposes
			if (!SelectMultiReferrals()) {
				//nothing changed
				return;
			}

			
		}
		else if (nID == -1) {
			m_strFilteredReferralList = "";
			m_nCurrentFilteredReferralID = -1;
			//TES 7/23/2009 - PLID 34227 - Need to keep m_dwFilteredRefIDList up to date
			m_dwFilteredRefIDList.RemoveAll();
		}
		else {

			m_strFilteredReferralList.Format("(%li)", nID);
			m_nCurrentFilteredReferralID = nID;
			//TES 7/23/2009 - PLID 34227 - Need to keep m_dwFilteredRefIDList up to date
			m_dwFilteredRefIDList.RemoveAll();
			m_dwFilteredRefIDList.Add(nID);
		}	
		
		ResetGraph();
	}NxCatchAll("Error in CMarketReferralSourceDlg::OnSelChosenReferralSourceList");
}

void CMarketReferralSourceDlg::OnSelChangingReferralFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("CMarketReferralSourceDlg::OnSelChangingReferralFilter");	
}

// (j.gruber 2009-06-26 12:56) - PLID 34718 - added referral filter function
CString CMarketReferralSourceDlg::GetReferralFilterDescription() {

	try {

		if (m_nCurrentDrilledDownReferral != -1) {
			return "";
		}
		else {
			if (!m_strFilteredReferralList.IsEmpty()) {
				return "Referral Source(s): " + GetReferralNamesFromIDString(m_strFilteredReferralList);
			}
			else {
				return "All Referral Sources";
			}
		}
		
	}NxCatchAll("Error in CMarketReferralSourceDlg::GetReferralFilterDescription()");

	return "";
}

//(e.lally 2009-09-28) PLID 35594 - Used as the local function to be used to replace the patient's 
	//primary referal source filter placeholder in the sql graph query.
CString CMarketReferralSourceDlg::GetConsToProcPrimaryReferralFilter()
{
	CString strOut;
	if(!m_strFilteredReferralList.IsEmpty()){
		if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
			ASSERT(FALSE);
			ThrowNxException("CMarketReferralSourceDlg::GetConsToProcPrimaryReferralFilter : Invalid MainFrame pointer.");
		}
		if(GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate) != mfReferralDate){
			strOut.Format(" AND (PatientsT.ReferralID IN %s ) ", m_strFilteredReferralList);
		}
	}
	return strOut;

}

//(e.lally 2009-09-28) PLID 35594 - Used as the local function to be used to replace the patient's 
	//multi-referal source filter placeholder in the sql graph query.
CString CMarketReferralSourceDlg::GetConsToProcMultiReferralFilter()
{
	CString strOut;
	if(!m_strFilteredReferralList.IsEmpty()){
		if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
			ASSERT(FALSE);
			ThrowNxException("CMarketReferralSourceDlg::GetConsToProcMultiReferralFilter : Invalid MainFrame pointer.");
		}
		if(GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate) == mfReferralDate){
			strOut.Format(" AND (MultiReferralsT.ReferralID IN %s ) ", m_strFilteredReferralList);
		}
	}
	return strOut;
}

// (z.manning 2009-08-31 17:16) - PLID 35051
void CMarketReferralSourceDlg::OnConfigureApptTypes() 
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
void CMarketReferralSourceDlg::ShowConversionRateControls(BOOL bShow)
{
	UINT nShowCmd = bShow ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES_REFERRAL)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_ALL_COLUMNS_REF)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY_REF)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY_REF)->ShowWindow(nShowCmd);

	if(bShow) {
		m_ShowAllRad.SetCheck(TRUE);
		m_ShowNumberRad.SetCheck(FALSE);
		m_ShowPercentRad.SetCheck(FALSE);
	}
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketReferralSourceDlg::OnShowAllColumns() 
{
	if (LastChecked(IDC_SHOW_ALL_COLUMNS_REF)) return;
	ResetGraph();
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketReferralSourceDlg::OnShowNumbersOnly() 
{
	if (LastChecked(IDC_SHOW_NUMBERS_ONLY_REF)) return;
	ResetGraph();
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketReferralSourceDlg::OnShowPercentagesOnly() 
{
	if (LastChecked(IDC_SHOW_PERCENTAGES_ONLY_REF)) return;
	ResetGraph();
}

// (z.manning 2009-09-09 15:28) - PLID 35051 - Used to work around weird drawing issues with
// these radio buttons that are in the column graph area.
void CMarketReferralSourceDlg::InvalidateConsToProcRadioButtons()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_ALL_COLUMNS_REF);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_NUMBERS_ONLY_REF);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY_REF);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}

// (j.jones 2012-09-24 17:42) - PLID 29451 - added invalidate for the inquiries checkbox
void CMarketReferralSourceDlg::InvalidateShowInquiriesButton()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_INQUIRIES);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}