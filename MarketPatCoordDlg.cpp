#include "stdafx.h"

// CMarketPatCoordDlg.cpp : implementation file

#include "marketUtils.h"
#include "MarketPatCoordDlg.h"
#include <winuser.h>
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "ConversionRateByDateConfigDlg.h"
#include "GlobalDrawingUtils.h"
#include "MarketFilterPickerDlg.h"
#include "DocBar.h"
#include "MarketingRc.h"

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NxTab;
using namespace ADODB;
using namespace ColumnGraph;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMarketPatCoordDlg dialog

CMarketPatCoordDlg::~CMarketPatCoordDlg()
{
}

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketPatCoordDlg::CMarketPatCoordDlg(CWnd* pParent)
	: CMarketingDlg(CMarketPatCoordDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CMarketPatCoordDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/patientcoordinator.htm";

}

void CMarketPatCoordDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketPatCoordDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_CONSULTS_BY_PAT_COORD, m_ConsByPatCoord);
	DDX_Control(pDX, IDC_CANCEL_BY_PAT_COORD, m_CancelByPatCoord);
	DDX_Control(pDX, IDC_CONTACT_WITH_PROS_BY_PAT_COORD, m_ContactWithProsByPatCoord);
	DDX_Control(pDX, IDC_CONS_TO_SURG_BY_PAT_COORD, m_ConsToSurgByPatCoord);
	DDX_Control(pDX, IDC_MONEY_BY_PAT_COORD, m_MoneyByPatCoordRad);
	DDX_Control(pDX, IDC_INQ_TO_CONS_BY_STAFF2, m_InqToConsByStaff);
	DDX_Control(pDX, IDC_PATIENTS, m_Patients);
	DDX_Control(pDX, IDC_PROS_TO_CONS_BY_PROC, m_ProsToCons);
	DDX_Control(pDX, IDC_PROCS_CLOSED3, m_ProcsClosed);
	DDX_Control(pDX, IDC_SHOW_ALL_COLUMNS_COORD, m_ShowAllRad);
	DDX_Control(pDX, IDC_SHOW_NUMBERS_ONLY_COORD, m_ShowNumberRad);
	DDX_Control(pDX, IDC_SHOW_PERCENTAGES_ONLY_COORD, m_ShowPercentRad);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMarketPatCoordDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketPatCoordDlg)
	ON_BN_CLICKED(IDC_CONSULTS_BY_PAT_COORD, OnConsByPatCoord)
	ON_BN_CLICKED(IDC_CANCEL_BY_PAT_COORD, OnCancelByPatCoord)
	ON_BN_CLICKED(IDC_CONTACT_WITH_PROS_BY_PAT_COORD, OnContactWithProsByPatCoord)
	ON_BN_CLICKED(IDC_MONEY_BY_PAT_COORD, OnMoneyByPatCoord)
	ON_BN_CLICKED(IDC_CONS_TO_SURG_BY_PAT_COORD, OnConsToSurgByPatCoord)
	ON_BN_CLICKED(IDC_INQ_TO_CONS_BY_STAFF2, OnInqToConsByStaff)
	ON_BN_CLICKED(IDC_PATIENTS, OnPatients)
	ON_BN_CLICKED(IDC_PROCS_CLOSED3, OnProcsClosed)
	ON_BN_CLICKED(IDC_PROS_TO_CONS_BY_PROC, OnProsToCons)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TYPES_COORDINATOR, OnConfigureApptTypes)
	ON_BN_CLICKED(IDC_SHOW_ALL_COLUMNS_COORD, OnShowAllColumns)
	ON_BN_CLICKED(IDC_SHOW_NUMBERS_ONLY_COORD, OnShowNumbersOnly)
	ON_BN_CLICKED(IDC_SHOW_PERCENTAGES_ONLY_COORD, OnShowPercentagesOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMarketPatCoordDlg, CMarketingDlg)
	//{{AFX_DISPATCH_MAP(CMarketPatCoordDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketPatCoordDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketPatCoordDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketPatCoordDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


BOOL CMarketPatCoordDlg::OnInitDialog() 
{
	// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
	ColumnGraph::FontPtr font;

	CMarketingDlg::OnInitDialog();

	m_bRenderedOnce = false;
	m_bActive = false;
	m_bGraphEmpty = true;
	
	// (v.maida 2015-03-10 16:47) - PLID 63216 - Tooltip text for the Coordinator tab's Revenue radio button incorrectly mentioned refunds being displayed.
	m_MoneyByPatCoordRad.SetToolTip("Display the net amount of applied payments generated per coordinator.");
	m_ConsByPatCoord.SetToolTip("Display the total number of consultations, procedures, and the conversion rate.");
	m_Patients.SetToolTip("Display the total number of patients and prospects assigned to each coordinator.");
	m_CancelByPatCoord.SetToolTip("Display the number of appointment cancellations and No Shows per coordinator. The coordinator is associated via the patient who has the appointment. ");
	m_InqToConsByStaff.SetToolTip("Display the percentage of patients and prospects who progressed from inquiries and scheduled consultations by the staff that input the inquiry. ");
	m_ProsToCons.SetToolTip("Display the percentage of prospects who have scheduled consultations.");
	m_ProcsClosed.SetToolTip("Display the total number of procedures performed versus the number of procedures that are scheduled where the patient has a prepayment associated with the appointment via the Procedure Information Center.");
	m_ConsToSurgByPatCoord.SetToolTip("Display the total number of scheduled consultations, procedures, and the percentage of consultations that progressed into procedures.");

	m_graph = GetDlgItem(IDC_GRAPH)->GetControlUnknown();
	m_Go.AutoSet(NXB_MARKET);


	font = m_graph->Font;
	font->PutName("Arial Narrow");
	font->PutSize(COleCurrency(13, 0));

	//setup the "Show Completed Appointments Only" checkbox
//	CheckDlgButton(IDC_COMPLETED_ONLY, GetRemotePropertyInt("MarketingCompletedOnly", 0, 0, GetCurrentUserName(), true));

	m_graph->Background = 0xFDFDFD;

	m_MoneyByPatCoordRad.SetCheck(1);
	OnMoneyByPatCoord();

	return TRUE;  
}


void CMarketPatCoordDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try
	{	
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

			if (m_ConsByPatCoord.GetCheck()) {
				ConsByPatCoord();
			}
			else if (m_CancelByPatCoord.GetCheck()) {
				CancelByPatCoord();
			}
			/*else if (m_NoShowByPatCoord.GetCheck()) {
				NoShowByPatCoord();
			}
			else if (m_ContactWithProsByPatCoord.GetCheck()) {
				OnContactWithProsByPatCoord();
			}*/
			else if (m_ConsToSurgByPatCoord.GetCheck()) {
				ConsToSurgByPatCoord();
			}
			else if (m_MoneyByPatCoordRad.GetCheck()) {
				MoneyByPatCoord();
			}
			else if (m_InqToConsByStaff.GetCheck()) {
				InqToConsByStaff();
			}
			else if (m_Patients.GetCheck()) {
				Patients();
			}
			else if (m_ProsToCons.GetCheck()) {
				ProsToCons();
			}
			else if (m_ProcsClosed.GetCheck()) {
				ProcsClosed();
			}
		}
		else { // set filters
			if (m_ConsByPatCoord.GetCheck()) {
				OnConsByPatCoord();
			}
			else if (m_CancelByPatCoord.GetCheck()) {
				OnCancelByPatCoord();
			}
			else if (m_ConsToSurgByPatCoord.GetCheck()) {
				OnConsToSurgByPatCoord();
			}
			else if (m_MoneyByPatCoordRad.GetCheck()) {
				OnMoneyByPatCoord();
			}
			else if (m_InqToConsByStaff.GetCheck()) {
				OnInqToConsByStaff();
			}
			else if (m_Patients.GetCheck()) {
				OnPatients();
			}
			else if (m_ProsToCons.GetCheck()) {
				OnProsToCons();
			}
			else if (m_ProcsClosed.GetCheck()) {
				OnProcsClosed();
			}
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
		
		InvalidateConsToProcRadioButtons();
	}
	NxCatchAll("Could not update Marketing Comparison");
}


void CMarketPatCoordDlg::OnConsByPatCoord()
{	
	if (LastChecked(IDC_CONSULTS_BY_PAT_COORD)) return;

	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMConsByPatCoord) {
		SetType(NUMConsByPatCoord);
	}
	m_graph->PrintTitle = "Consults By Coordinator";
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}


void CMarketPatCoordDlg::ConsByPatCoord()
{
	
	GraphByPatientCoordinator(NUMConsByPatCoord, "Consults", "NumCons");
		
}


void CMarketPatCoordDlg::OnCancelByPatCoord()
{
	if (LastChecked(IDC_CANCEL_BY_PAT_COORD)) return;
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != NUMCanByPatCoord) {
		SetType(NUMCanByPatCoord);
		SetFilter(mfApptDate, mftDate);
		SetFilter(mfApptLocation,mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}

	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();

}

void CMarketPatCoordDlg::CancelByPatCoord() 
{
	m_graph->PrintTitle = "Cancellations and No Shows By Coordinator";	
	
	GraphByPatientCoordinator(NUMCanByPatCoord, "No Shows", "NumNoShows", "Cancellations", "NumCons" );
}


void CMarketPatCoordDlg::OnProsToCons()
{
	if (LastChecked(IDC_PROS_TO_CONS_BY_PROC)) return;
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != COORDProsToCons) {
		SetType(COORDProsToCons);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketPatCoordDlg::ProsToCons() 
{
	m_graph->PrintTitle = "Prospects To Consults By Coordinator";
	
	GraphByPatientCoordinator(COORDProsToCons, "Percentage", "ProsPercent");
}


void CMarketPatCoordDlg::OnProcsClosed()
{
	if (LastChecked(IDC_PROCS_CLOSED3)) return;
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != COORDSchedVClosed) {
		SetType(COORDSchedVClosed);
		SetFilter(mfApptDate, mftDate);	
		SetFilter(mfApptLocation, mftLocation);
		SetFilter(mfApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketPatCoordDlg::ProcsClosed() 
{
	m_graph->PrintTitle = "Procedures Performed vs. Closed By Coordinator";

	GraphByPatientCoordinator(COORDSchedVClosed, "Performed", "ProcsPerformed", "Closed", "ProcsClosed");	
}


void CMarketPatCoordDlg::OnPatients()
{
	if (LastChecked(IDC_PATIENTS)) return;
	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != COORDPatients) {
		SetType(COORDPatients);
		SetFilter(mfPatientLocation, mftLocation);
		SetFilter(mfPatientProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketPatCoordDlg::Patients() 
{
	m_graph->PrintTitle = "Patients By Coordinator";
		
	GraphByPatientCoordinator(COORDPatients, "Patients", "PatCount", "Prospects", "ProsCount");
}

void CMarketPatCoordDlg::OnShowInquiries() 
{
//	UpdateView(); a.walling 5/18/06 PLID 20695 we refresh manually in OnGo.	

}



void CMarketPatCoordDlg::GraphPatientCoordinators(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsPatCoord = NULL,
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


		sql.Format ("SELECT (PersonT.First + ' ' + PersonT.Last) AS Name, "
			"ID FROM PersonT INNER JOIN UsersT On PersonT.Id = UsersT.PersonID "
			"WHERE PatientCoordinator <> 0 ORDER BY Name;");

		//(e.lally 2009-08-24) PLID 35314 - The consult to surgery graph can now support "No Coordinator"
		MarketGraphType mtGraph = (MarketGraphType)GetType();
		switch(mtGraph){
			case CNVConsToSurgByPatCoord:
				sql = "SELECT (PersonT.First + ' ' + PersonT.Last) AS Name, "
					"ID, 2 AS SortOrder "
					"FROM PersonT INNER JOIN UsersT On PersonT.Id = UsersT.PersonID "
					"WHERE PatientCoordinator <> 0 "
					"UNION SELECT '<No Coordinator>' AS Name, -1 AS ID, 1 AS SortOrder "
					"ORDER BY SortOrder, Name;";
				break;
			default:
				break;
		}
		

		rsPatCoord = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsPatCoord->RecordCount;
		m_graph->RowCount = max;
		m_progress.SetPos(30);

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

		//JMJ - although I disabled the x-axis sort for this graph,
		//I left these two lines commented in for posterity
		m_graph->XAxisDesc = "Patient Coordinator";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = FALSE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		i = 0;
		while (!rsPatCoord->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsPatCoord->Fields->GetItem("Name")->Value);
			m_graph->RowID = AdoFldLong(rsPatCoord, "ID");

			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = rsPatCoord->Fields->GetItem("ID")->Value.lVal;
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
						{	//(e.lally 2009-08-24) PLID 35314 - Add a default double value for No Coordinator
							if (descendants[k] == AdoFldLong(rs, "EmployeeID", -1))
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
			rsPatCoord->MoveNext();
			m_progress.SetPos(50 + ((50 * i) / max));
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}


void CMarketPatCoordDlg::OnContactWithProsByPatCoord() 
{
	
}
/*
void CMarketPatCoordDlg::OnCompletedOnly() 
{
	//Called when the user selects the "Show Only Completed Appointments" checkbox

	//save the fact that they have this checked
	SetRemotePropertyInt("MarketingCompletedOnly", IsDlgButtonChecked(IDC_COMPLETED_ONLY) ? 1 : 0, 0, GetCurrentUserName());


	UpdateView();	//rebuilds the sql query
}*/



void CMarketPatCoordDlg::GraphByPatientCoordinator(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2/* = ""*/, CString strField2 /*= ""*/) {
	try {
		GraphDescript desc;
		CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilter, PatFilterIDs, strCategory, strResp, strLocationField, strProvField;
		int  nCategory, nResp;
		CString referrals;

		//disable the Show Completed only checkbox, it doesn't apply to this view
		//GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_SHOW);

		//show the appt date filter
		//AddApptDate();
		
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

		CString strFrom, strTo;
		if (!strDateField.IsEmpty()) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}

		if (nCategory!= -1) 
			strCategory.Format(" AND ServiceT.Category = %i ", nCategory);
		
		if (nResp == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
		}
		else if (nResp == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
		}
		
		//if (IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
		//	sql = GetGraphSql(mktType, 1, -1);
		//}
		//else {
			sql = GetGraphSql(mktType, -1, -1, pCon, strPatientTempTable);
		//}

		sql.Replace("[From]", strFrom);
		sql.Replace("[To]", strTo);
		sql.Replace("[Prov1]", strProvFilter1);
		sql.Replace("[Prov2]", strProvFilter2);
		sql.Replace("[Loc1]", strLocFilter1);
		sql.Replace("[Loc2]", strLocFilter2);
		sql.Replace("[PatFilter]", strPatFilter);
		sql.Replace("[Resp]", strResp);
		sql.Replace("[Category]", strCategory);

		desc.m_sql = sql;
		Save(sql);
		//calculate the summary
		_RecordsetPtr rsTotal;
		CString strTempDesc1, strTempDesc2;

		//set the timeout
		CIncreaseCommandTimeout ict(600);

		m_progress.SetPos(10);

		if (strDesc2.IsEmpty()) {
			
			if (strDescription == "Percentage") {

				//calculate it correctly
				//rsTotal = CreateRecordset("SELECT CASE WHEN Sum(ProsWithCons) = 0 THEN 0 ELSE Sum(ProsWithCons) END  AS SumNum, CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE Sum(TotalPros) END AS SumDenom From (%s) BASE WHERE EmployeeID <> -1 AND EmployeeID IS NOT NULL ", sql);
			
				strTempDesc1.Format("%s", strDescription);
				//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
				desc.Add(strTempDesc1, "ProsWithCons", GetMarketGraphColor(mgcBrightBlue), "TotalPros", GraphDescript::GD_DIV);

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



	//	MessageBox(sql);

		if(mktType == EFFMonByPatCoord)
			m_graph->Format = _bstr_t(GetCurrencySymbol());
		else if(mktType == COORDProsToCons)
			m_graph->Format = "%0.0f%%";
		else
			m_graph->Format = "%0.0f";
		
		GraphPatientCoordinators(desc);

		//show the totals
		//it can't total percentages right now, just add
		for (int i = 0; i < desc.Size(); i++) {
			m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			CString strText;
			if (GetType() == EFFMonByPatCoord) {
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
					double dbl = desc.dblTotal(i)/desc.dblTotal2(i);
					strText.Format("%s (%.0f%% Total)", desc.Label(i), dbl);
				}
			}
			else {
				strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
			}
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

	}NxCatchAll("Error Graphing Patient Coordinators");
}


void CMarketPatCoordDlg::OnConsToSurgByPatCoord()
{
	if (LastChecked(IDC_CONS_TO_SURG_BY_PAT_COORD)) return;

	ShowConversionRateControls(TRUE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != CNVConsToSurgByPatCoord) {
		SetType(CNVConsToSurgByPatCoord);
		SetFilter(mfNoPatApptLocation, mftLocation);
		SetFilter(mfNoPatApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketPatCoordDlg::ConsToSurgByPatCoord() 
{
	
	try {
		//(e.lally 2009-08-24) PLID 35299 - Updated the coordinator tab to use the new base query for the consult to procedure conversion rate.
	
		// (j.gruber 2007-10-31 16:18) - PLID 27941 - fixed a typo
		m_graph->PrintTitle = "Consults To Procedure By Coordinator";

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//(e.lally 2009-08-25) PLID 35299 - Get the SQL query string - the filters are already applied for us based on the docbar selections.
		CString strSql = GetGraphSql(CNVConsToSurgByPatCoord, -1, -1, pCon, strPatientTempTable);
		
		GraphDescript desc;

		//set the timeout
		CIncreaseCommandTimeout ict(600);

		desc.m_sql = strSql;
		Save(strSql);

		BOOL bShowNumbers = IsDlgButtonChecked(IDC_SHOW_NUMBERS_ONLY_COORD) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS_COORD);
		BOOL bShowPercentages = IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY_COORD) || IsDlgButtonChecked(IDC_SHOW_ALL_COLUMNS_COORD);

		if(!IsConsToProcSetupValid()) {
			MessageBox("Please set up your configuration settings by clicking the ... button before running this graph");
			return;
		}

		// (z.manning 2009-09-09 12:53) - PLID 35051 - Put the shared logic in a utility function
		AddConsToProcDataToGraphDesc(strSql, &desc, bShowNumbers, bShowPercentages);
		
		m_graph->Format = "%0.0f";
		GraphPatientCoordinators(desc);	

		ict.Reset();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error generating graph");
}


void CMarketPatCoordDlg::OnMoneyByPatCoord()
{
	if (LastChecked(IDC_MONEY_BY_PAT_COORD)) return;

	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != EFFMonByPatCoord) {
		SetType(EFFMonByPatCoord);
		SetFilter(mfPaymentDate, mftDate);
		SetFilter(mfChargeLocation, mftLocation);
		SetFilter(mfChargeProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(0);

	ResetGraph();
}

void CMarketPatCoordDlg::MoneyByPatCoord() 
{
	m_graph->PrintTitle = "Revenue By Coordinator";

	//if (IsDlgButtonChecked(IDC_INCLUDE_TAX)) {
		
		GraphByPatientCoordinator(EFFMonByPatCoord, "Revenue", "SumMoney");
	//}
	//else {
			
	//	GraphByPatientCoordinator(EFFMonByPatCoord, "Revenue", "SumNoTax");
	//}	
}


void CMarketPatCoordDlg::OnInqToConsByStaff()
{
	if (LastChecked(IDC_INQ_TO_CONS_BY_STAFF2)) return;

	ShowConversionRateControls(FALSE); // (z.manning 2009-08-31 17:07) - PLID 35051
	if(GetMainFrame()->m_pDocToolBar->GetType() != CNVInqToConsByStaff) {
		SetType(CNVInqToConsByStaff);
		SetFilter(mfPatNoApptLocation, mftLocation);
		SetFilter(mfPatNoApptProvider, mftProvider);
	}
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

	ResetGraph();
}

void CMarketPatCoordDlg::InqToConsByStaff() 
{
	m_graph->PrintTitle = "Inquiries By Staff";
	
	GraphByStaff(CNVInqToConsByStaff, "Percentage", "ProsPercent");
}


void CMarketPatCoordDlg::GraphByStaff(MarketGraphType mktType, CString strDescription, CString strField, CString strDesc2/* = ""*/, CString strField2 /*= ""*/) {

	
	GraphDescript desc;
	CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strDateField, strPatFilter, PatFilterIDs, strLocationField, strProvField;
	int nCategory, nResp;
	CString referrals;

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);

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
	sql.Replace("[Loc2]", strLocFilter2);
	sql.Replace("[PatFilter]", strPatFilter);
	//TEMPORARY SOLUTION
	sql.Replace("[Resp]", "");
	sql.Replace("[Category]", "");

	desc.m_sql = sql;
	Save(sql);
	//calculate the summary
	_RecordsetPtr rsTotal;
	CString strTempDesc1, strTempDesc2;

	//set the timeout
	CIncreaseCommandTimeout ict(600);

	if (strDesc2.IsEmpty()) {
		
		if (strDescription == "Percentage") {

				//calculate it correctly
			/*	rsTotal = CreateRecordset("SELECT CASE WHEN Sum(ProsWithCons) = 0 THEN 0 ELSE Sum(ProsWithCons) END  AS SumNum, CASE WHEN Sum(TotalPros) = 0 THEN 0 ELSE Sum(TotalPros) END AS SumDenom From (%s) BASE", sql);

				double dblNum, dblDenom, dblSum;
				
				dblNum = AdoFldDouble(rsTotal, "SumNum", 0);
				dblDenom = AdoFldDouble(rsTotal, "SumDenom", 0);

				if (dblDenom == 0) {
					dblSum = 0;
				}
				else {
					dblSum = (dblNum/dblDenom) * 100.0;				
				}
				*/
				strTempDesc1.Format("%s", strDescription);
				//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
				desc.Add(strTempDesc1, "ProsWithCons", GetMarketGraphColor(mgcBrightBlue), "TotalPros", GraphDescript::GD_DIV);


		}
		else {
			strTempDesc1.Format("%s", strDescription);
			//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
			desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue));
		}
			
		
	}
	else {

		//rsTotal = CreateRecordset("SELECT CASE WHEN Sum(%s) = 0 THEN 0 ELSE Sum(%s) END as Total1, CASE WHEN Sum(%s) = 0 THEN 0 ELSE Sum(%s) END as Total2 FROM (%s) Base ", strField, strField, strField2, strField2, sql);			
		strTempDesc1.Format("%s", strDescription);
		strTempDesc2.Format("%s", strDesc2);
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
		desc.Add(strTempDesc1, strField, strDescription == "Revenue" ?RGB(43,200,15) : GetMarketGraphColor(mgcBrightBlue));
		desc.Add(strTempDesc2, strField2, strDesc2 == "Revenue" ?RGB(43,200,15) : RGB(255,0,0));
	}

	//MessageBox(sql);

	if(mktType == EFFMonByPatCoord)
		m_graph->Format = _bstr_t(GetCurrencySymbol());
	else if(mktType == COORDProsToCons) 
		m_graph->Format = "%0.0f%%";
	else
		m_graph->Format = "%0.0f%%";
	
	GraphStaff(desc);

	for (int i = 0; i < desc.Size(); i++)
	{	m_graph->Column = i;
		m_graph->Color = desc.Color(i);
		CString strText;
		if (GetType() == EFFMonByPatCoord) {
			CString strCur;
			COleCurrency cy;
			strCur.Format("%.2f", desc.dblTotal(i));
			cy.ParseCurrency(strCur);
			// (z.manning 2008-06-18 16:21) - PLID 28601 - Hide decimal places on marketing graphs.
			strText.Format("%s (%s Total)", desc.Label(i), FormatCurrencyForInterface(cy, TRUE, TRUE, FALSE));
		}
		//(e.lally 2009-09-16) PLID 35559 - Added support for percents
		else if (desc.Op(i) == GraphDescript::GD_DIV || desc.Op(i) == GraphDescript::GD_PERCENT) {
			if (desc.dblTotal2(i) == 0) {
				strText.Format("%s (%s%% Total)", desc.Label(i), "0");
			}
			else {
				double dbl = desc.dblTotal(i)/desc.dblTotal2(i);
				strText.Format("%s (%.0f%% Total)", desc.Label(i), dbl);
			}
		}
		else {
			strText.Format("%s (%s Total)", desc.Label(i), AsString(_variant_t(desc.dblTotal(i))));
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

void CMarketPatCoordDlg::GraphStaff(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsPatCoord = NULL,
						rs = NULL;
		int				currentID = 0, id = 0, i, 
						j, 
						k,
						max;
		double			total, total2;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		m_progress.SetPos(0);

		sql.Format ("SELECT (PersonT.First + ' ' + PersonT.Last) AS Name, "
			"ID FROM PersonT INNER JOIN UsersT On PersonT.Id = UsersT.PersonID "
			"ORDER BY Name;");

		rsPatCoord = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsPatCoord->RecordCount;
		m_graph->RowCount = max;
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

		//JMJ - although I disabled the x-axis sort for this graph,
		//I left these two lines commented in for posterity
		m_graph->XAxisDesc = "Patient Coordinator";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = FALSE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		i = 0;
		while (!rsPatCoord->eof)
		{	//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(rsPatCoord->Fields->GetItem("Name")->Value);
			m_graph->RowID = AdoFldLong(rsPatCoord, "ID");

			//Get descendants of the ith child
			descendants.RemoveAll();
			currentID = rsPatCoord->Fields->GetItem("ID")->Value.lVal;
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
						{	if (descendants[k] == AdoFldLong(rs, "UserID"))
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
			rsPatCoord->MoveNext();
			m_progress.SetPos(50 + 50 * i / max);
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}



void CMarketPatCoordDlg::Print(CDC *pDC, CPrintInfo *pInfo){
	
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

// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
void CMarketPatCoordDlg::GetCurrentGraphFilters(CString &strFilter1, CString &strFilter2, ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable) {

	GraphDescript desc;
	CString sql, from, to, strProvFilter1, strProvFilter2, provIDs, strLocFilter1, strLocFilter2, locIDs, strPatFilter, PatFilterIDs, strDateField, strCat, strResp, strLocationField, strProvField;
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
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

	CString strFrom, strTo;
	if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	if ((MarketGraphType)GetType() == EFFMonByPatCoord) {
		if (nCategoryID != -1) 
			strCat.Format(" AND ServiceT.Category = %i ", nCategoryID);
		
		if (nResp == 1)  {
			strResp.Format(" AND (PaymentsT.InsuredPartyID = -1 OR PaymentsT.InsuredPartyID IS NULL)");
		}
		else if (nResp == 2) {
			strResp.Format(" AND PaymentsT.InsuredPartyID <> -1 AND PaymentsT.InsuredPartyID IS NOT NULL");
		}
	}
	else {
		strCat = "";
		strResp = "";
	}


	strFilter1 = " [From] [To] [Prov1] [Loc1] [PatFilter] [Cat] [Resp] ";
		
	strFilter1.Replace("[From]", strFrom);
	strFilter1.Replace("[To]", strTo);
	strFilter1.Replace("[Prov1]", strProvFilter1);
	strFilter1.Replace("[Prov2]", strProvFilter2);
	strFilter1.Replace("[Loc1]", strLocFilter1);
	strFilter1.Replace("[PatFilter]", strPatFilter);
	strFilter1.Replace("[Cat]", strCat);
	strFilter1.Replace("[Resp]", strResp);

	strFilter2 = " [Loc2] [Prov2]";

	strFilter2.Replace("[Loc2]", strLocFilter2);
	strFilter2.Replace("[Prov2]", strProvFilter2);
}

void CMarketPatCoordDlg::OnGo() 
{
	m_bRenderedOnce = true;
	m_graph->Title = ""; // no report here sets a title. remove this and set them in the On functions if you like.
	UpdateView();
}

void CMarketPatCoordDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
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
	InvalidateConsToProcRadioButtons();
}

bool CMarketPatCoordDlg::LastChecked(int nID) 
{
	if ( (m_nLastChecked == nID) && m_bActive )
		return true;
	
	m_nLastChecked = nID;
	return false;
}

LRESULT CMarketPatCoordDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	GetDlgItem(IDC_GO)->EnableWindow(true);	

	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}

void CMarketPatCoordDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	if (!bShow) {
		LastChecked(-1);
	}

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

// (z.manning 2009-08-31 17:16) - PLID 35051
void CMarketPatCoordDlg::OnConfigureApptTypes() 
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
void CMarketPatCoordDlg::ShowConversionRateControls(BOOL bShow)
{
	UINT nShowCmd = bShow ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES_COORDINATOR)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_ALL_COLUMNS_COORD)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY_COORD)->ShowWindow(nShowCmd);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY_COORD)->ShowWindow(nShowCmd);

	if(bShow) {
		m_ShowAllRad.SetCheck(TRUE);
		m_ShowNumberRad.SetCheck(FALSE);
		m_ShowPercentRad.SetCheck(FALSE);
	}
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketPatCoordDlg::OnShowAllColumns() 
{
	if (LastChecked(IDC_SHOW_ALL_COLUMNS_COORD)) return;
	ResetGraph();
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketPatCoordDlg::OnShowNumbersOnly() 
{
	if (LastChecked(IDC_SHOW_NUMBERS_ONLY_COORD)) return;
	ResetGraph();
}

// (z.manning 2009-09-03 16:58) - PLID 35051
void CMarketPatCoordDlg::OnShowPercentagesOnly() 
{
	if (LastChecked(IDC_SHOW_PERCENTAGES_ONLY_COORD)) return;
	ResetGraph();
}

// (z.manning 2009-09-09 15:28) - PLID 35051 - Used to work around weird drawing issues with
// these radio buttons that are in the column graph area.
void CMarketPatCoordDlg::InvalidateConsToProcRadioButtons()
{
	CWnd *pwnd = GetDlgItem(IDC_SHOW_ALL_COLUMNS_COORD);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_NUMBERS_ONLY_COORD);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
	pwnd = GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY_COORD);
	if(pwnd->IsWindowVisible()) {
		pwnd->Invalidate();
		pwnd->ShowWindow(SW_SHOW);
	}
}
