// MarketBaselineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MarketBaselineDlg.h"
#include "marketingrc.h"
#include "InternationalUtils.h"
#include "globalutils.h"
#include "globaldatautils.h"
#include "globaldrawingutils.h"
#include "MultiSelectDlg.h"
#include "docbar.h"
#include "MarketBaselineConfig.h"
#import "msdatsrc.tlb"
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "MSCHRT20.tlb"

// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace ColumnGraph;

#define MAX_INTERVALS		6
#define MAX_INDICES			7

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketBaselineDlg dialog

class CMBGraphInfo
{
public:
	CString m_strSQLBase;	// The original SQL string

	// (j.jones 2005-10-04 11:35) - PLID 17623 - these fields were no longer needed with new optimizations

	//CString m_strPKField;	// The primary key field of the original SQL string
	//CString m_strDateTable;	// The table to filter the dates on
	//CString m_strDateTablePK;	// The primary key field of the table to filter the dates on
	CString m_strDateField;		// The date field of the table to filter dates on
	COleDateTime m_dtFrom;		// From date
	COleDateTime m_dtTo;		// To date
	BOOL m_bValuesArePercent;	// Percentage?

	CMBGraphInfo(const CString& strSQLBase, /*const CString& strPKField,*/
		 /*const CString& strDateTable,*/ /*const CString& strDateTablePK,*/
		 const CString& strDateField, const COleDateTime& dtFrom,
		 const COleDateTime& dtTo, BOOL bValuesArePercent = false)
	{
		m_strSQLBase = strSQLBase;
		//m_strPKField = strPKField;
		//m_strDateTable = strDateTable;
		//m_strDateTablePK = strDateTablePK;
		m_strDateField = strDateField;
		m_dtFrom = dtFrom;
		m_dtTo = dtTo;
		m_bValuesArePercent = bValuesArePercent;
	}
};

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketBaselineDlg::CMarketBaselineDlg(CWnd* pParent)
	: CMarketingDlg(CMarketBaselineDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketBaselineDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/performance_indices.htm";
	m_nColumns = 0;
	m_fGraphMax = 0;
	m_hBitmapPrint = NULL;
}


void CMarketBaselineDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketBaselineDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_STATIC_LEGEND7, m_Legend7);
	DDX_Control(pDX, IDC_STATIC_LEGEND6, m_Legend6);
	DDX_Control(pDX, IDC_STATIC_LEGEND5, m_Legend5);
	DDX_Control(pDX, IDC_STATIC_LEGEND4, m_Legend4);
	DDX_Control(pDX, IDC_STATIC_LEGEND3, m_Legend3);
	DDX_Control(pDX, IDC_STATIC_LEGEND2, m_Legend2);
	DDX_Control(pDX, IDC_STATIC_LEGEND1, m_Legend1);
	DDX_Control(pDX, IDC_STATIC_LEGEND_LABEL, m_LegendLabel);
	DDX_Control(pDX, IDC_CHECK_TRACKINGNOSHOW, m_btnTrackingNoShow);
	DDX_Control(pDX, IDC_CHECK_INQUIRIES, m_btnInquires);
	DDX_Control(pDX, IDC_CHECK_SURGERIES, m_btnSurgeries);
	DDX_Control(pDX, IDC_CHECK_CONSULTATIONS, m_btnConsultations);
	DDX_Control(pDX, IDC_CHECK_CLOSURE_LENGTH, m_btnClosureLength);
	DDX_Control(pDX, IDC_CHECK_CLOSURE, m_btnClosure);
	DDX_Control(pDX, IDC_CHECK_AVGPROC, m_btnAvgProc);
	DDX_Control(pDX, IDC_CHECK_SHOWPREVYEAR, m_btnShowPrevYear);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDD_ACTIVITIES_SETMARKETINGBASELINEGOALS, m_btnGoals);
	DDX_Control(pDX, IDC_CHART, m_chart);
	//DDX_Control(pDX, IDC_STATIC_PATIENTS_UNLIMITED, m_nxstaticPatientsUnlimited); // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketBaselineDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketBaselineDlg)
	//ON_BN_CLICKED(IDC_STATIC_PATIENTS_UNLIMITED, OnStaticPatientsUnlimited) // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDD_ACTIVITIES_SETMARKETINGBASELINEGOALS, OnActivitiesSetmarketingbaselinegoals)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_BN_CLICKED(IDC_CHECK_INQUIRIES, OnCheckInquiries)
	ON_BN_CLICKED(IDC_CHECK_CONSULTATIONS, OnCheckConsultations)
	ON_BN_CLICKED(IDC_CHECK_CLOSURE, OnCheckClosure)
	ON_BN_CLICKED(IDC_CHECK_SURGERIES, OnCheckSurgeries)
	ON_BN_CLICKED(IDC_CHECK_CLOSURE_LENGTH, OnCheckClosureLength)
	ON_BN_CLICKED(IDC_CHECK_AVGPROC, OnCheckAvgproc)
	ON_BN_CLICKED(IDC_CHECK_TRACKINGNOSHOW, OnCheckTrackingnoshow)
	ON_BN_CLICKED(IDC_CHECK_SHOWPREVYEAR, OnCheckShowprevyear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketBaselineDlg message handlers

BOOL CMarketBaselineDlg::OnInitDialog() 
{
	try {
		CNxBaselineStatic* pLabel[MAX_INDICES] = 
		{
			&m_Legend1, &m_Legend2, &m_Legend3, &m_Legend4,
			&m_Legend5, &m_Legend6, &m_Legend7,
		};

		CMarketingDlg::OnInitDialog();
		
		m_Go.AutoSet(NXB_MARKET);
		// (c.haag 2008-05-07 10:16) - PLID 29833 - Set
		// the modify style for the baseline goals button
		// because it can lead to the modification of the
		// goal metrics in data
		m_btnGoals.AutoSet(NXB_MODIFY);

		m_bRenderedOnce = false;
		m_bActive = false;
		m_bDirtyGraph = false;
		m_bLastYear = false;
		m_nColumns = 0;

		m_btnTrackingNoShow.SetToolTip("Display a graph of the total number of No Show appointments. This includes all appointment types and purposes.");
		m_btnInquires.SetToolTip("Display a graph of the total number of new patient inquiries made to the Practice.");
		m_btnSurgeries.SetToolTip("Display a graph of the total number of surgeries performed.");
		m_btnConsultations.SetToolTip("Display a graph of the total number of patient consultations scheduled. This excludes cancelled and No Show appointments.");
		m_btnClosure.SetToolTip("Display a graph of the closure ratio. The formula is (# of consults that precede scheduled surgeries for patients w/Pre-Payments divided by the total number of patient consults). This excludes cancelled and No Show appointments. A value of 100% would appear at the top of the graph, and a value of 0% would appear at the very bottom.");
		m_btnClosureLength.SetToolTip("Display a graph of the average number of days between a consult and a surgery. This includes appointments for patients who currently have pre-payments, and excludes appointments with a No Show or cancelled status.");
		m_btnAvgProc.SetToolTip("Display a graph of the average number of surgeries and minor procedures scheduled per consult. This excludes cancelled and No Show appointments.");
		m_btnShowPrevYear.SetToolTip("Display a graph of all the selected categories for the previous year. This graph is shown in dashed lines.");

		m_chart.SetTitleText("");
		m_chart.SetAllowDithering(TRUE);
		

		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtFirst(dtNow.GetYear() - 1, dtNow.GetMonth(), dtNow.GetDay(), 0,0,0);
		//m_FromDate.SetValue((_variant_t)dtFirst);
		//m_ToDate.SetValue((_variant_t)dtNow);

		SetDlgItemText(IDC_EFF_LABEL6,"Appointment Date");

		// (c.haag 2004-04-12 15:23) - TODO: Use GetRemoteProperty to preserve settings
		m_btnConsultations.SetCheck(TRUE);
		m_btnSurgeries.SetCheck(TRUE);

		// Set up our hyperlink to Patients Unlimited
		/* (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
		//CWnd* pStatic = GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED); 
		SetWindowLong(pStatic->GetSafeHwnd(), GWL_STYLE, pStatic->GetStyle() | SS_NOTIFY | SS_ENDELLIPSIS);
		SetWindowLong(pStatic->GetSafeHwnd(), GWL_EXSTYLE, pStatic->GetExStyle() | WS_EX_TRANSPARENT);*/

		m_pFontPU = new CFont;
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		if(CreateCompatiblePointFont(m_pFontPU, 80, "Arial"))
		{
			LOGFONT logfont;
			m_pFontPU->GetLogFont( &logfont );
			delete m_pFontPU;
			logfont.lfUnderline = 1;
			m_pFontPU = new CFont;
			m_pFontPU->CreateFontIndirect( &logfont );
			//pStatic->SetFont(m_pFontPU);			
		}
		for (long i=0; i < 7; i++)
		{
			SetWindowLong(pLabel[i]->GetSafeHwnd(), GWL_EXSTYLE, pLabel[i]->GetExStyle() | WS_EX_TRANSPARENT);
			pLabel[i]->SetWindowPos(&wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
			pLabel[i]->ShowWindow(SW_HIDE); // a.walling 5/22/06 PLID 20695 Hide these until they are ready
		}

		// Set up our master baseline lable to appear over all the goal labels
		m_pLabelFont = new CFont;
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		if(CreateCompatiblePointFont(m_pLabelFont, 90, "Arial"))
		{
			LOGFONT logfont;
			m_pLabelFont->GetLogFont( &logfont );
			delete m_pLabelFont;
			logfont.lfWeight = FW_BOLD;
			m_pLabelFont = new CFont;
			m_pLabelFont->CreateFontIndirect( &logfont );
		}
		SetWindowLong(m_LegendLabel.GetSafeHwnd(), GWL_STYLE, m_LegendLabel.GetStyle() | SS_NOTIFY | SS_ENDELLIPSIS);
		SetWindowLong(m_LegendLabel, GWL_EXSTYLE, m_LegendLabel.GetExStyle() | WS_EX_TRANSPARENT);
		m_LegendLabel.SetWindowPos(&wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
		m_LegendLabel.SetFont(m_pLabelFont);

		// a.walling 5/22/06 PLID 20695 Prepare the graph here for when the user decides to refresh it.
		m_LegendLabel.ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED)->ShowWindow(SW_HIDE); // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
		m_chart.SetColumnCount(0);
		m_chart.SetRowCount(0);
		RedrawWindow();
	}
	NxCatchAll("Error initializing baseline graph");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarketBaselineDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
		SetType(BASELINEGraph);
		SetFilter(mfApptDate, mftDate);
		//(a.wilson 2011-10-5) PLID 38789 - added force refresh to prevent refreshing when coming back to module.
		if (m_bRenderedOnce || (m_bActive && bForceRefresh))
		{
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
			COleDateTime dtFrom;
			COleDateTime dtTo;
			GetParameters(dtFrom, dtTo);
			COleDateTimeSpan dtInterval = dtTo - dtFrom;
			long nIntervals = min(MAX_INTERVALS, (long)dtInterval.GetTotalDays() + 1);

			// (j.gruber 2007-09-13 16:35) - PLID 27370 - function to check that the dates will come out ok
			if (!CheckDates(dtTo, dtFrom, nIntervals)) {
				//(a.wilson 2011-10-5) PLID 38789 - moved code to function to prevent repetition
				HideAndRedraw();
				ResetGraph(true);
				return;
			}

			m_bDirtyGraph = true;
			UpdateGraphColumnCount(); // updates m_nColumns
			if (!m_bRenderedOnce) // manual refresh via the main toolbar
			{
				ClearCachedRS(); // all data will be requeried
			}
			CMarketRenderButtonStatus mrbs(this);
			m_bRenderedOnce = false;

			
			CString str;
			CWaitCursor wc;
			CRect rcGraph;
						
			
			m_nIntervals = nIntervals;
			m_astrRowLabels.RemoveAll();
			m_astrLegendLabels.RemoveAll();
			m_aclrRowLines.RemoveAll();
			m_aPenStyles.RemoveAll();
			m_astrPropKeys.RemoveAll();
			m_aPropDefaults.RemoveAll();
			// m_nColumns = GetGraphColumnCount(); this is done in UpdateGraphColumnCount now
			m_nCurColumn = 0;
			m_fGraphMax = 0;


			m_chart.GetWindowRect(&rcGraph);
			if (!m_nColumns || rcGraph.Height() <= 140 || dtFrom >= dtTo)
			{
				//(a.wilson 2011-10-5) PLID 38789 - moved code to function to prevent repetition
				HideAndRedraw();
				return;
			}
			m_LegendLabel.ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED)->ShowWindow(SW_SHOW); // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
			m_progress.SetRange(0,m_nColumns * (short)m_nIntervals);
			m_progress.SetPos(0);

			m_chart.SetColumnCount(m_nColumns);
			m_chart.SetColumnLabelCount(m_nColumns); 
			//(e.lally 2009-09-24) PLID 35526 - Use the readable color names
			if (m_btnInquires.GetCheck()) { GraphInquires(dtFrom, dtTo, RGB(164,0,164)); }
			if (m_btnConsultations.GetCheck()) { GraphConsultations(dtFrom, dtTo, GetMarketGraphColor(mgcBrightGreen)); }
			if (m_btnSurgeries.GetCheck()) { GraphSurgeries(dtFrom, dtTo, GetMarketGraphColor(mgcBrightRed)); }		
			if (m_btnClosureLength.GetCheck()) { GraphClosureLength(dtFrom, dtTo, RGB(0,164,192)); }		
			if (m_btnTrackingNoShow.GetCheck()) { GraphNoShows(dtFrom, dtTo, RGB(255,128,0)); }
			if (m_btnClosure.GetCheck()) { GraphClosureRatio(dtFrom, dtTo, RGB(32,164,32)); }
			if (m_btnAvgProc.GetCheck()) { GraphAvgProceduresPerCase(dtFrom, dtTo, RGB(192,0,32)); }
			if (m_btnShowPrevYear.GetCheck())
			{
				dtFrom.SetDate(dtFrom.GetYear() - 1, dtFrom.GetMonth(), dtFrom.GetDay());
				dtTo.SetDate(dtTo.GetYear() - 1, dtTo.GetMonth(), dtTo.GetDay());
				if (m_btnInquires.GetCheck()) { GraphInquires(dtFrom, dtTo, RGB(164,0,164), (long)MSChart20Lib::VtPenStyleDashed, true); }
				if (m_btnConsultations.GetCheck()) { GraphConsultations(dtFrom, dtTo, GetMarketGraphColor(mgcBrightGreen), (long)MSChart20Lib::VtPenStyleDashed, true); }
				if (m_btnSurgeries.GetCheck()) { GraphSurgeries(dtFrom, dtTo, GetMarketGraphColor(mgcBrightRed), (long)MSChart20Lib::VtPenStyleDashed, true); }			
				if (m_btnClosureLength.GetCheck()) { GraphClosureLength(dtFrom, dtTo, RGB(0,164,192), (long)MSChart20Lib::VtPenStyleDashed, true); }			
				if (m_btnTrackingNoShow.GetCheck()) { GraphNoShows(dtFrom, dtTo, RGB(255,128,0), (long)MSChart20Lib::VtPenStyleDashed, true); }
				if (m_btnClosure.GetCheck()) { GraphClosureRatio(dtFrom, dtTo, GetMarketGraphColor(mgcBrightBlue), (long)MSChart20Lib::VtPenStyleDashed, true); }
				if (m_btnAvgProc.GetCheck()) { GraphAvgProceduresPerCase(dtFrom, dtTo, RGB(192,0,32), (long)MSChart20Lib::VtPenStyleDashed, true); }
			}

			m_chart.SetChartData(m_saGraph.Detach());
			ScaleGraph();
			SetGraphLabels();
			SetBaselineLabels();
			ResizeLegend();

			if (!m_btnShowPrevYear.GetCheck())
			{
				dtFrom.SetDate( dtFrom.GetYear() - 1, dtFrom.GetMonth(), dtFrom.GetDay() );
				dtTo.SetDate( dtTo.GetYear() - 1, dtTo.GetMonth(), dtTo.GetDay() );
			}
			str.Format("Show Previous Year (%s-%s)",
				FormatDateTimeForInterface(dtFrom, dtoDate),
				FormatDateTimeForInterface(dtTo, dtoDate));
			m_btnShowPrevYear.SetWindowText(str);

			m_progress.SetPos(0);
			RedrawWindow();
		}
		else {
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
			//(a.wilson 2011-10-5) PLID 38789 - ensure that none of the labels show when forcerefresh is false.
			if (!bForceRefresh && m_chart.GetColumnCount() == 0) {
				//(a.wilson 2011-10-5) PLID 38789 - moved code to function to prevent repetition
				HideAndRedraw();
			}
		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
	}
	NxCatchAll("Error updating baseline graph");
}

unsigned short CMarketBaselineDlg::GetGraphColumnCount()
{
	return m_nColumns;
}

void CMarketBaselineDlg::UpdateGraphColumnCount()
{
	unsigned short nColumns = 0;
	if (m_btnInquires.GetCheck()) nColumns++;
	if (m_btnConsultations.GetCheck()) nColumns++;
	if (m_btnSurgeries.GetCheck()) nColumns++;
	if (m_btnClosure.GetCheck()) nColumns++;
	if (m_btnClosureLength.GetCheck()) nColumns++;
	if (m_btnAvgProc.GetCheck()) nColumns++;
	if (m_btnTrackingNoShow.GetCheck()) nColumns++;

	// If we include the results from the prior year, we have to double the number of columns
	if (m_btnShowPrevYear.GetCheck()) {
		nColumns = nColumns * 2;
		m_bLastYear = true;
	}
	else {
		m_bLastYear = false;
	}
	m_nColumns = nColumns;
}

void CMarketBaselineDlg::SetGraphLabels()
{
	COleDateTime dtFrom, dtTo;
	unsigned short nColumn = 0;

	GetParameters(dtFrom, dtTo);

	if (!m_btnShowPrevYear.GetCheck())
	{
		if (m_btnInquires.GetCheck())
		{
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("Inquiries");
			m_astrLegendLabels.Add("Inquiries");
			m_astrPropKeys.Add("");
			m_aPropDefaults.Add(0);
		}
		if (m_btnConsultations.GetCheck())
		{
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("Consults");
			m_astrLegendLabels.Add("Consults");
			m_astrPropKeys.Add("MarketConsultsGoal");
			m_aPropDefaults.Add(27);
		}
		if (m_btnSurgeries.GetCheck())
		{
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("Surgeries");
			m_astrLegendLabels.Add("Surgeries");
			m_astrPropKeys.Add("MarketSurgeriesGoal");
			m_aPropDefaults.Add(18);
		}
		if (m_btnClosure.GetCheck())
		{
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("Closure Rate From Consults (%)");
			m_astrLegendLabels.Add("Closure Rate From Consults (%)");
			m_astrPropKeys.Add("MarketClosureRateGoal");
			m_aPropDefaults.Add(55);
		}
		if (m_btnClosureLength.GetCheck())
		{
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("Days from Closure to Surgery");
			m_astrLegendLabels.Add("Days from Closure to Surgery");
			m_astrPropKeys.Add("MarketClosureTimeSpanGoal");
			m_aPropDefaults.Add(35);
		}
		if (m_btnAvgProc.GetCheck())
		{ 
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("Avg. Procedures Per Case");
			m_astrLegendLabels.Add("Avg. Procedures Per Case");
			m_astrPropKeys.Add("");
			m_aPropDefaults.Add(0);
		}
		if (m_btnTrackingNoShow.GetCheck())
		{
			m_chart.SetColumn(++nColumn);
			m_chart.SetColumnLabel("No Shows");
			m_astrLegendLabels.Add("No Shows");
			m_astrPropKeys.Add("");
			m_aPropDefaults.Add(0);
		}
	}
	else
	{
		long nYear = dtFrom.GetYear();
		CString str;
		for (long i=0; i < 2; i++, nYear--)
		{
			if (m_btnInquires.GetCheck())
			{
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("Inquiries");
				m_astrLegendLabels.Add("Inquiries");
				m_astrPropKeys.Add("");
				m_aPropDefaults.Add(0);
			}
			if (m_btnConsultations.GetCheck())
			{
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("Consults");
				m_astrLegendLabels.Add("Consults");
				m_astrPropKeys.Add("MarketConsultsGoal");
				m_aPropDefaults.Add(27);
			}
			if (m_btnSurgeries.GetCheck())
			{
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("Surgeries");
				m_astrLegendLabels.Add("Surgeries");
				m_astrPropKeys.Add("MarketSurgeriesGoal");
				m_aPropDefaults.Add(18);
			}
			if (m_btnClosure.GetCheck())
			{
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("Closure Rate From Consults (%)");
				m_astrLegendLabels.Add("Closure Rate From Consults (%)");
				m_astrPropKeys.Add("MarketClosureRateGoal");
				m_aPropDefaults.Add(55);
			}
			if (m_btnClosureLength.GetCheck())
			{
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("Days from Closure to Surgery");
				m_astrLegendLabels.Add("Days from Closure to Surgery");
				m_astrPropKeys.Add("MarketClosureTimeSpanGoal");
				m_aPropDefaults.Add(35);
			}
			if (m_btnAvgProc.GetCheck())
			{ 
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("Avg. Procedures Per Case");
				m_astrLegendLabels.Add("Avg. Procedures Per Case");
				m_astrPropKeys.Add("");
				m_aPropDefaults.Add(0);
			}
			if (m_btnTrackingNoShow.GetCheck())
			{
				m_chart.SetColumn(++nColumn);
				m_chart.SetColumnLabel("No Shows");
				m_astrLegendLabels.Add("No Shows");
				m_astrPropKeys.Add("");
				m_aPropDefaults.Add(0);
			}
		}
	}

	m_chart.SetColumn(1);
	for (unsigned short i=0; i < m_nRows; i++)
	{
		m_chart.SetRow(i+1);
		m_chart.SetRowLabel(m_astrRowLabels[i]);
	}	
	for (i=0; i < m_nColumns; i++)
	{
		MSChart20Lib::IVcPlotPtr pPlot = m_chart.GetPlot();		
		MSChart20Lib::IVcColorPtr pClr = pPlot->SeriesCollection->Item[i+1]->DataPoints->Item[-1]->Brush->FillColor;
		pPlot->SeriesCollection->Item[i+1]->Pen->Style = (MSChart20Lib::VtPenStyle)m_aPenStyles[i];
		pClr->Red = GetRValue(m_aclrRowLines[i]);
		pClr->Green = GetGValue(m_aclrRowLines[i]);
		pClr->Blue = GetBValue(m_aclrRowLines[i]);
	}
}

void CMarketBaselineDlg::SetBaselineLabels()
{
	CNxBaselineStatic* pLabel[MAX_INDICES] = 
	{
		&m_Legend1, &m_Legend2, &m_Legend3, &m_Legend4,
		&m_Legend5, &m_Legend6, &m_Legend7
	};
	long nColumns = m_btnShowPrevYear.GetCheck() ? m_nColumns / 2 : m_nColumns;

	for (long i=0; i < nColumns; i++)
	{
		CString strLabel;
		double nGoal = (m_astrPropKeys[i] != "MarketClosureRateGoal" && m_astrPropKeys[i] != "MarketClosureTimeSpanGoal") ? ScaleGoal(GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i])) : (double)GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]);

		pLabel[i]->SetPenColor(m_aclrRowLines[i]);
		switch (m_aPenStyles[i])
		{
		case (long)MSChart20Lib::VtPenStyleSolid:
			pLabel[i]->SetPenStyle(PS_SOLID);
			break;
		case (long)MSChart20Lib::VtPenStyleDashed:
			pLabel[i]->SetPenStyle(PS_DASH);
			break;
		}

		if (!m_astrPropKeys[i].IsEmpty())
		{
			if (nGoal < 1)
				strLabel.Format("%s (<1)", m_astrLegendLabels[i], (long)nGoal);
			else
				strLabel.Format("%s (%d)", m_astrLegendLabels[i], (long)nGoal);
		}
		else
		{
			strLabel = m_astrLegendLabels[i];
		}
		pLabel[i]->SetWindowText(strLabel);
		pLabel[i]->ShowWindow(SW_SHOW);
	}
	while (i < 7)
	{
		pLabel[i]->ShowWindow(SW_HIDE);
		i++;
	}
}

void CMarketBaselineDlg::GetParameters(COleDateTime& dtFrom, COleDateTime& dtTo)
{
	CString strFrom = GetMainFrame()->m_pDocToolBar->GetFromDate();
	CString strTo = GetMainFrame()->m_pDocToolBar->GetToDate();	
	if (GetMainFrame()->m_pDocToolBar->IsFilteringAllDates()) // All dates
	{
		try {
			// (r.gonet 10-20-2010 16:17) - PLID 39721 - Select a good default From Date if there are no appointments that match this filter. We use a bit of a trick to get the date part of yesterday.
			_RecordsetPtr prs = CreateRecordset("select coalesce(min(starttime), DATEADD(dd, -1, DATEDIFF(dd, 0, GETDATE()))) as FromDate from appointmentst where apttypeid in (select id from apttypet where Category = 1 or Category = 3 or Category = 4) AND status <> 4 AND showstate <> 3");
			if (prs->eof)
				return;
			dtFrom = AdoFldDateTime(prs, "FromDate");
			dtFrom.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
			dtTo = COleDateTime::GetCurrentTime();
			GetMainFrame()->m_pDocToolBar->SetFromDate(dtFrom);
			GetMainFrame()->m_pDocToolBar->SetToDate(dtTo);
			return;
		} NxCatchAll("Error calculating baseline filter");
	}
	dtFrom.ParseDateTime(strFrom);
	dtTo.ParseDateTime(strTo);
}

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
void CMarketBaselineDlg::GetParameters(CString &prov, CString &loc, CString &strPatFilter, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{
	prov = GetMainFrame()->m_pDocToolBar->GetProviderString();
	loc = GetMainFrame()->m_pDocToolBar->GetLocationString();
	strPatFilter = GetMainFrame()->m_pDocToolBar->GetPatientFilterString(pCon, strPatientTempTable);
}

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
void CMarketBaselineDlg::FilterAppointmentSQL(CString& strSQL, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{
	CString str, provIDs, locIDs, PatFilterIDs;
	GetParameters(provIDs, locIDs, PatFilterIDs, pCon, strPatientTempTable);

	if (locIDs != "")
	{
		str.Format(" AND ApptsQ.LocationID IN %s", locIDs);
		strSQL += str;
	}
	if (UseFilter(mftProvider))
	{
		str.Format(" AND ApptsQ.PatientID IN (SELECT PersonID FROM PatientsT WHERE MainPhysician IN %s AND PersonID > -1)", provIDs);
		strSQL += str;
	}
	if (PatFilterIDs != "")
	{
		str.Format(" AND ApptsQ.PatientID IN %s ", PatFilterIDs);
		strSQL += str;
	}
}

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
void CMarketBaselineDlg::FilterInquiresSQL(CString& strSQL, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{
	CString str, provIDs, locIDs, PatFilterIDs;
	GetParameters(provIDs, locIDs, PatFilterIDs, pCon, strPatientTempTable);

	if (locIDs != "")
	{
		str.Format(" AND Location IN %s ", locIDs);
		strSQL += str;
	}
	if (UseFilter(mftProvider))
	{
		str.Format(" AND MainPhysician IN %s ", provIDs);
		strSQL += str;
	}
	if (PatFilterIDs != "")
	{
		str.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);
		strSQL += str;
	}
}

void CMarketBaselineDlg::GraphInquires(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{
	CString strSQL = "select count(personid) AS IDCount from patientst inner join persont on persont.id = patientst.personid WHERE persont.id > -1 AND CurrentStatus = 4";
	
	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterInquiresSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"PersonID", "PersonT", "ID",*/ "FirstContactDate", dtFrom, dtTo);	
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eInquires, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::GraphConsultations(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{
	CString strSQL = "select count(id) AS IDCount from appointmentst AS ApptsQ where patientid > -1 AND apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterAppointmentSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"ID", "AppointmentsT", "ID",*/ "StartTime", dtFrom, dtTo);	
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eConsultations, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::GraphSurgeries(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{
	CString strSQL = "select count(id) AS IDCount from appointmentst AS ApptsQ where patientid > -1 AND starttime < GetDate() and apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND status <> 4 AND showstate <> 3";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterAppointmentSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"ID", "AppointmentsT", "ID",*/ "StartTime", dtFrom, dtTo);	
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eSurgeries, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::GraphClosureRatio(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{
	CString strSQL =
		// Take the number of consults that progress into surgeries and divide
		// them by the number of total consults
		//(e.lally 2011-07-15) PLID 41026 - If there are no consult appts, we can't divide by zero and we'll zero out the numerator.
		"select convert(real, count(ApptsQ.id)) * 100 "
		//Check to see if our denominator is zero, if so we need to zero out our numerator too.
		" * (select CASE WHEN count(id) = 0 THEN 0 ELSE 1 END from appointmentst where apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND patientid > -1) "
		" / convert(real, (select CASE WHEN count(id) = 0 THEN 1 ELSE count(id) END from appointmentst where apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND patientid > -1)) as IDCount FROM AppointmentsT AS ApptsQ "
		// Join our consults with surgery appts on patient id where
		// the surgery is ahead of the consult...
		"LEFT JOIN (SELECT ID, StartTime, PatientID FROM AppointmentsT where apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND status <> 4 AND showstate <> 3 AND patientid > -1) SurgQ on "
		"(SurgQ.PatientID = ApptsQ.PatientID AND SurgQ.StartTime > ApptsQ.StartTime) "
		// Filter our appointment list on consults only (remember the surgeries are in SurgQ)
		"where apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 "
		// Filter on patients who have unapplied prepayments
		"AND ApptsQ.patientid in (select patientid from lineitemt inner join paymentst on paymentst.id = lineitemt.id where lineitemt.patientid = ApptsQ.patientid and prepayment = 1 and lineitemt.id not in (select sourceid from appliest)) "
		// Filter on consults that happened in the past
		"AND ApptsQ.StartTime < GetDate()  "
		// Filter on consults that actually have related surgeries
		"AND SurgQ.ID IS NOT NULL "
		// Filter on consults where there is no similar consult between that and the surgery
		// So, if you have the following:
		// ID = 100  StartTime = 4/1/04  Type = Consult  Purpose = Bleph
		// ID = 101  StartTime = 4/1/04  Type = Consult  Purpose = Bleph
		// ID = 102  StartTime = 4/1/04  Type = Surgery  Purpose = Bleph
		// Then the results will ignore ID 100 entirely
		"AND (SELECT Count(A2.ID) FROM (SELECT ID, StartTime FROM AppointmentsT WHERE PatientID = ApptsQ.PatientID and apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND StartTime > ApptsQ.StartTime AND StartTime < SurgQ.StartTime AND ID <> ApptsQ.ID) A2 WHERE A2.ID IN (SELECT AppointmentPurposeT.AppointmentID FROM AppointmentPurposeT INNER JOIN AppointmentPurposeT AS A3 ON AppointmentPurposeT.PurposeID = A3.PurposeID WHERE AppointmentPurposeT.AppointmentID = A2.ID AND ApptsQ.ID = A3.AppointmentID)) = 0";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterAppointmentSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"ApptsQ.ID", "AppointmentsT", "AppointmentsT.ID",*/ "ApptsQ.StartTime", dtFrom, dtTo, TRUE);
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eClosure, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::GraphClosureLength(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{
	CString strSQL = 
		// We want the difference, in days, between the consults and surgeries we're selecting
		"select avg(datediff(d, ApptsQ.starttime, SurgQ.starttime)) as IDCount FROM AppointmentsT AS ApptsQ "
		// Join our consults with surgery appts on patient id where
		// the surgery is ahead of the consult...
		"LEFT JOIN (SELECT ID, StartTime, PatientID FROM AppointmentsT where apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND status <> 4 AND showstate <> 3 AND patientid > -1) SurgQ on "
		"(SurgQ.PatientID = ApptsQ.PatientID AND SurgQ.StartTime > ApptsQ.StartTime) "
		// Filter our appointment list on consults only (remember the surgeries are in SurgQ)
		"where apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND ApptsQ.patientid > -1 "
		// Filter on patients who have unapplied prepayments
		"AND ApptsQ.patientid in (select patientid from lineitemt inner join paymentst on paymentst.id = lineitemt.id where lineitemt.patientid = ApptsQ.patientid and prepayment = 1 and lineitemt.id not in (select sourceid from appliest)) "
		// Filter on consults that happened in the past
		"AND ApptsQ.StartTime < GetDate()  "
		// Filter on consults that actually have related surgeries
		"AND SurgQ.ID IS NOT NULL "
		// Filter on consults where there is no similar consult between that and the surgery
		// So, if you have the following:
		// ID = 100  StartTime = 4/1/04  Type = Consult  Purpose = Bleph
		// ID = 101  StartTime = 4/1/04  Type = Consult  Purpose = Bleph
		// ID = 102  StartTime = 4/1/04  Type = Surgery  Purpose = Bleph
		// Then the results will ignore ID 100 entirely
		//"AND (SELECT Count(A2.ID) FROM AppointmentsT AS A2 where apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND A2.StartTime > ApptsQ.StartTime AND A2.StartTime < SurgQ.StartTime AND A2.ID <> ApptsQ.ID AND A2.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE AppointmentID = A2.ID AND PurposeID IN (SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = ApptsQ.ID))) = 0";
		"AND (SELECT Count(A2.ID) FROM (SELECT ID, StartTime FROM AppointmentsT WHERE PatientID = ApptsQ.PatientID and apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND StartTime > ApptsQ.StartTime AND StartTime < SurgQ.StartTime AND ID <> ApptsQ.ID) A2 WHERE A2.ID IN (SELECT AppointmentPurposeT.AppointmentID FROM AppointmentPurposeT INNER JOIN AppointmentPurposeT AS A3 ON AppointmentPurposeT.PurposeID = A3.PurposeID WHERE AppointmentPurposeT.AppointmentID = A2.ID AND ApptsQ.ID = A3.AppointmentID)) = 0";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterAppointmentSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"ApptsQ.ID", "AppointmentsT", "AppointmentsT.ID",*/ "ApptsQ.StartTime", dtFrom, dtTo);
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eClosureLength, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::GraphAvgProceduresPerCase(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{	
	CString strSQL =
		// We want the average number of surgeries per consult that have them.
		"SELECT Avg(SurgCount) AS IDCount FROM (SELECT Count(SurgeryID) AS SurgCount, StartTime, ConsultID, LocationID, PatientID FROM (select ApptsInnerQ.id as ConsultID, ApptsInnerQ.starttime, ApptsInnerQ.locationid, ApptsInnerQ.patientid, SurgQ.id AS SurgeryID FROM AppointmentsT AS ApptsInnerQ "
		// Join our consults with surgery appts on patient id where
		// the surgery is ahead of the consult...
		"LEFT JOIN (SELECT ID, StartTime, PatientID FROM AppointmentsT where PatientID > -1 AND apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND status <> 4 AND showstate <> 3) SurgQ on (SurgQ.PatientID = ApptsInnerQ.PatientID AND SurgQ.StartTime > ApptsInnerQ.StartTime) "
		// Filter our appointment list on consults only (remember the surgeries are in SurgQ)
		"where apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND ApptsInnerQ.patientid > -1 "
		// Filter on patients who have unapplied prepayments
		"AND ApptsInnerQ.patientid in (select patientid from lineitemt inner join paymentst on paymentst.id = lineitemt.id where lineitemt.patientid = ApptsInnerQ.patientid and prepayment = 1 and lineitemt.id not in (select sourceid from appliest)) "
		// Filter on consults that happened in the past
		"AND ApptsInnerQ.StartTime < GetDate() "
		// Filter on consults that actually have related surgeries
		"AND SurgQ.ID IS NOT NULL "
		// Filter on consults where there is no similar consult between that and the surgery
		// So, if you have the following:
		// ID = 100  StartTime = 4/1/04  Type = Consult  Purpose = Bleph
		// ID = 101  StartTime = 4/1/04  Type = Consult  Purpose = Bleph
		// ID = 102  StartTime = 4/1/04  Type = Surgery  Purpose = Bleph
		// Then the results will ignore ID 100 entirely
		"AND (SELECT Count(A2.ID) FROM (SELECT ID, StartTime FROM AppointmentsT WHERE PatientID = apptsinnerq.PatientID and apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND StartTime > apptsinnerq.StartTime AND StartTime < SurgQ.StartTime AND ID <> apptsinnerq.ID) A2 WHERE A2.ID IN (SELECT AppointmentPurposeT.AppointmentID FROM AppointmentPurposeT INNER JOIN AppointmentPurposeT AS A3 ON AppointmentPurposeT.PurposeID = A3.PurposeID WHERE AppointmentPurposeT.AppointmentID = A2.ID AND apptsinnerq.ID = A3.AppointmentID)) = 0 "
		// Closure (1=1 is required because AND's are appended to the query)
		") SubQ group by ConsultID, StartTime, LocationID, PatientID) ApptsQ WHERE 1=1 ";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterAppointmentSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"ConsultID", "AppointmentsT", "AppointmentsT.ID",*/ "ApptsQ.StartTime", dtFrom, dtTo);	
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eAvgProc, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::GraphNoShows(const COleDateTime& dtFrom, const COleDateTime& dtTo, COLORREF clr, long nPenStyle /* = 1 */, bool bLastYear /* = false*/)
{
	CString strSQL = "select count(id) AS IDCount from appointmentst AS ApptsQ where status <> 4 and showstate = 3 and patientid > -1";

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	FilterAppointmentSQL(strSQL, pCon, strPatientTempTable); // Filter on the dropdowns at the top
	CMBGraphInfo info(strSQL, /*"ApptsQ.ID", "AppointmentsT", "AppointmentsT.ID",*/ "ApptsQ.StartTime", dtFrom, dtTo);	
	m_aclrRowLines.Add(clr);
	m_aPenStyles.Add(nPenStyle);
	Graph(&info, eTrackingNoShow, bLastYear);

	// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
	if(!strPatientTempTable.IsEmpty()) {
		ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
		strPatientTempTable = "";
	}
}

void CMarketBaselineDlg::Graph(CMBGraphInfo* pInfo, eChartLines eLine, bool bLastYear /* = false*/)
{
	// (c.haag 2004-04-02 17:31) - We have our baseline SQL statement. This will pull
	// the complete count for all items considering all user-selected filters. Now we
	// need to integrate our interval into the SQL statement in such a way that we get
	// one record for every tick along the X-Axis graph.
	CString strSQLFinal;
	COleDateTime dtOriginalFrom;
	COleDateTime dtFrom = pInfo->m_dtFrom;
	COleDateTime dtTo = pInfo->m_dtTo;
	COleDateTimeSpan dtInterval = dtTo - dtFrom;

	// (c.haag 2004-04-26 11:58) - dtInterval gives us the number of days that span the graph.
	// For example, 4/1/04 through 4/4/04 actually demands 4 days: 4/1, 4/2, 4/3 and 4/4. Now
	// we want to figure out the real "from" date
	double dtDayInterval = dtInterval.GetTotalDays() / (double)(m_nIntervals-1);
	dtOriginalFrom = dtFrom - COleDateTimeSpan((long)dtDayInterval, 0,0,0);
	dtFrom = dtOriginalFrom;

	// (c.haag 2004-04-05 09:55) - Increment our known column count. This is how we track
	// the number of graphs we've done so far, and how many we will have done at the end.
	m_nCurColumn++; 

	for (long i=0; i < m_nIntervals; i++)
	{
		// Calculate the intermediate date to plot
		COleDateTime dt = dtFrom;
		long nDays = max(1,(long)(((double)(i+1)) * dtDayInterval));
		dt = dtOriginalFrom + COleDateTimeSpan(nDays, 0,0,0);

		// Now append the interval filter to the extended filter and add it to the final SQL statement

		// (j.jones 2005-10-04 11:33) - PLID 17623 - old, inefficient way
		/*
		CString strAdd = " AND " + pInfo->m_strPKField + " IN (SELECT " + pInfo->m_strDateTablePK + " FROM " + pInfo->m_strDateTable + " WHERE " +
			pInfo->m_strDateField + " > '" + FormatDateTimeForSql(dtFrom, dtoDate) + "' AND " +
			pInfo->m_strDateField + " <= '" + FormatDateTimeForSql(dt, dtoDate) + "')";
		*/

		// (j.jones 2005-10-04 11:33) - PLID 17623 - new, much more efficient way

		CString strAdd = " AND " + pInfo->m_strDateField + " > '" + FormatDateTimeForSql(dtFrom, dtoDate) + "' AND " +
			pInfo->m_strDateField + " <= '" + FormatDateTimeForSql(dt, dtoDate) + "'";
		strSQLFinal += pInfo->m_strSQLBase + strAdd; 

		if (m_saGraph.vt == VT_EMPTY)
		{
			m_astrRowLabels.Add(FormatDateTimeForInterface(dt, dtoDate));
		}
		// If we're not done yet, we need to add the prefix for the next line
		if (i < m_nIntervals - 1)
		{
			strSQLFinal += " UNION ALL ";
		}
		dtFrom = dt;
	}

	
	//set the timeout
	CIncreaseCommandTimeout ict(600);

	// Now open our recordset. Think of each record as X, and the data in each record as Y
	// as we plot.

	// a.walling PLID 20695 6/2/06 Cache the recordsets for each line rather than requery them each time
	if (bLastYear) {
		if (pLastYearCachedRS[eLine]) {
			pLastYearCachedRS[eLine]->MoveFirst();
		}
		else {
			pLastYearCachedRS[eLine] = CreateRecordset(adOpenStatic, adLockReadOnly, strSQLFinal);
			pLastYearCachedRS[eLine]->PutRefActiveConnection(NULL);
		}
	}
	else {
		if (pCachedRS[eLine]) {
			pCachedRS[eLine]->MoveFirst();
		}
		else {
			pCachedRS[eLine] = CreateRecordset(adOpenStatic, adLockReadOnly, strSQLFinal);
			pCachedRS[eLine]->PutRefActiveConnection(NULL);
		}
	}
	_RecordsetPtr prs = (bLastYear) ? pLastYearCachedRS[eLine]->Clone(adLockReadOnly) : pCachedRS[eLine]->Clone(adLockReadOnly);

	//_RecordsetPtr prs = CreateRecordset(strSQLFinal);

	//set the timeout back
	ict.Reset();

	// Check to see if we created our safe array object yet. We can't create it until we know how
	// many records we will be graphing for; otherwise we'd do it at the beginning.
	if (m_saGraph.vt == VT_EMPTY)
	{
		SAFEARRAYBOUND sab[2];
		m_nRows = (unsigned short)prs->GetRecordCount();
		sab[0].cElements = prs->GetRecordCount();
		sab[1].cElements = m_nColumns;
		sab[0].lLbound = sab[1].lLbound = 1;
		m_saGraph.Create(VT_BSTR, 2, sab);
	}
	long index[2] = {1,0}; // A 2D graph needs a 2D array as index array
	long nRow = 1;
	index[1] = m_nCurColumn;

	while (!prs->eof)
	{
		CString strValue;
		float fValue;
		if (prs->Fields->Item["IDCount"]->Value.vt == VT_R4)
			fValue = AdoFldFloat(prs, "IDCount", 0);
		else
			fValue = (float)AdoFldLong(prs, "IDCount", 0);

		if (pInfo->m_bValuesArePercent)
		{
			if(m_fGraphMax == 0.0f)
				m_fGraphMax = 100.0f;

			fValue = fValue * m_fGraphMax / 100.0f;
		}

		strValue.Format("%f", fValue);
		index[0] = nRow;
		m_saGraph.PutElement(index, strValue.AllocSysString());
		m_fGraphMax = max(m_fGraphMax, fValue);
		nRow++;
		prs->MoveNext();
		m_progress.SetPos(m_progress.GetPos() + 1);
	}	
}

BEGIN_EVENTSINK_MAP(CMarketBaselineDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketBaselineDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/* (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
void CMarketBaselineDlg::OnStaticPatientsUnlimited() 
{
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	ShellExecute(GetSafeHwnd(), NULL, "explorer.exe", "http://pumc.com/article.htm", NULL, SW_SHOW);
}*/

HBRUSH CMarketBaselineDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CMarketingDlg::OnCtlColor(pDC, pWnd, nCtlColor);
	/* (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetExStyle() & WS_EX_TRANSPARENT &&
		pWnd->GetSafeHwnd() == GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED)->GetSafeHwnd())
		pDC->SetTextColor(RGB(0,0,255));*/
	return hbr;
}

void CMarketBaselineDlg::OnDestroy() 
{
	CMarketingDlg::OnDestroy();
	
	ClearCachedRS();

	if (m_pFontPU) delete m_pFontPU;
	if (m_pLabelFont) delete m_pLabelFont;
}

BOOL CMarketBaselineDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	/* (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}*/
	return CMarketingDlg::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2009-02-04 09:45) - PLID 31956 - Don't override this; just use the IncludeChildPosition virtual function instead
/*int CMarketBaselineDlg::GetControlPositions()
{
	CRect tmpRect;
	long orgLeft, orgTop;
	CWnd *tmpWnd;
	long i;
	char strClassName[150];

	// Get window extents - do not use GetWindowRect, as we only want the client area
	GetClientRect(tmpRect);
	ClientToScreen(tmpRect);
	orgLeft		= tmpRect.left;
	orgTop		= tmpRect.top;

	//store the window coordinates for this window in the first slot
	originalWidth = tmpRect.right - orgLeft;
	originalHeight = tmpRect.bottom - orgTop;

	// 0=left, 1=top, 2=right, 3=bottom
	for (i=0, tmpWnd = GetWindow(GW_CHILD); tmpWnd; i++, tmpWnd = tmpWnd->GetWindow(GW_HWNDNEXT)) 
	{
		if (tmpWnd && tmpWnd->m_hWnd)
		{
			if (tmpWnd->GetSafeHwnd() == m_Legend1.GetSafeHwnd() ||
				tmpWnd->GetSafeHwnd() == m_Legend2.GetSafeHwnd() ||
				tmpWnd->GetSafeHwnd() == m_Legend3.GetSafeHwnd() ||
				tmpWnd->GetSafeHwnd() == m_Legend4.GetSafeHwnd() ||
				tmpWnd->GetSafeHwnd() == m_Legend5.GetSafeHwnd() ||
				tmpWnd->GetSafeHwnd() == m_Legend6.GetSafeHwnd())
			{
				i--;
				continue;
			}

			// Get window dimensions
			tmpWnd->GetWindowRect(tmpRect);

			// Generic settings
			// (a.walling 2008-04-17 12:13) - PLID 29697 - NxDialog was changed to use a CList of CControlInfo structures
			// rather than the old array approach
			CControlInfo c;
			c.nLeft = tmpRect.left - orgLeft;
			c.nTop = tmpRect.top - orgTop;
			c.nRight = tmpRect.right - orgLeft;
			c.nBottom = tmpRect.bottom - orgTop;
			c.hwnd = tmpWnd->m_hWnd;

			// Quick fix for standard combo box, which resizes wrong
			GetClassName(tmpWnd->GetSafeHwnd(), strClassName, 9);
			if (strcmp(strClassName, "ComboBox") == 0) 
			{
				c.nBottom = 10000;//this is the bottom of the list - ok if too big, but not if too small
			}
			
			m_listControls.AddHead(c);
		}
	}
	return 1;
}*/

// (a.walling 2009-02-04 09:47) - PLID 31956 - Tell GetControlPositions to skip certain child windows
BOOL CMarketBaselineDlg::IncludeChildPosition(HWND hwnd)
{
	if (hwnd == m_Legend1.GetSafeHwnd() ||
		hwnd == m_Legend2.GetSafeHwnd() ||
		hwnd == m_Legend3.GetSafeHwnd() ||
		hwnd == m_Legend4.GetSafeHwnd() ||
		hwnd == m_Legend5.GetSafeHwnd() ||
		hwnd == m_Legend6.GetSafeHwnd())
	{
		return FALSE;
	}

	return TRUE;
}

int CMarketBaselineDlg::SetControlPositions()
{
	int nRes = CMarketingDlg::SetControlPositions();

	// (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	////////////////////////////////////////////////////////////////
	// Set positions of baseline controls
	/*CWnd* pWnd = GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED);
	if (pWnd)
	{
		CDC* pDC = pWnd->GetDC();
		CRect rc;
		CString strText;
		pWnd->GetWindowRect(&rc);
		ScreenToClient(&rc);
		GetDlgItemText(IDC_STATIC_PATIENTS_UNLIMITED, strText);
		int nWidth = 0, nCharWidth;
		for (long i=0; i < strText.GetLength(); i++)
		{
			// You may wonder why I don't use DrawTextEx. The answer is that
			// DrawTextEx gives even less accurate values than this method
			// does!
			if (strText.GetAt(i) != ' ')
			{
				pDC->GetCharWidth(strText.GetAt(i), strText.GetAt(i), &nCharWidth);
				nWidth += nCharWidth + 1;
			}
		}
		rc.right = rc.left + nWidth;
		pWnd->ReleaseDC(pDC);
		pWnd->MoveWindow(&rc, FALSE);
		pWnd->Invalidate();
	}*/
	ResizeLegend();
	return nRes;
}

BOOL CMarketBaselineDlg::IntersectsOtherLabel(long nLast, const CRect& rc, CRect& rcIntersection)
{
	CNxBaselineStatic* pLabel[MAX_INDICES] = 
	{
		&m_Legend1, &m_Legend2, &m_Legend3, &m_Legend4,
		&m_Legend5, &m_Legend6, &m_Legend7
	};
	for (long i=0; i < nLast; i++)
	{
		CRect rcLabel;
		pLabel[i]->GetWindowRect(&rcLabel);
		ScreenToClient(&rcLabel);
		if ((rc.top < rcLabel.top && rc.bottom > rcLabel.top) ||
			(rc.top < rcLabel.bottom && rc.bottom > rcLabel.top) ||
			(rc.top == rcLabel.top))
		{
			rcIntersection = rcLabel;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CMarketBaselineDlg::IntersectsOtherPrintLabel(CRect* pRectList, long nLast, const CRect& rc, CRect& rcIntersection)
{
	for (long i=0; i < nLast; i++)
	{
		CRect& rcLabel = pRectList[i];
		if ((rc.top < rcLabel.top && rc.bottom > rcLabel.top) ||
			(rc.top < rcLabel.bottom && rc.bottom > rcLabel.top) ||
			(rc.top == rcLabel.top))
		{
			rcIntersection = rcLabel;
			return TRUE;
		}
	}
	return FALSE;
}

void CMarketBaselineDlg::ResizeLegend()
{
	CRect rc, rcColor, rcChart;
	CNxBaselineStatic* pLabel[MAX_INDICES] = 
	{
		&m_Legend1, &m_Legend2, &m_Legend3, &m_Legend4,
		&m_Legend5, &m_Legend6, &m_Legend7
	};
	COleDateTime dtFrom;
	COleDateTime dtTo;
	GetParameters(dtFrom, dtTo);
	long nColumns = m_bLastYear ? m_nColumns / 2 : m_nColumns;

	if (!nColumns || dtFrom >= dtTo)
		return;

	MSChart20Lib::IVcPlotPtr pPlot = m_chart.GetPlot();
	MSChart20Lib::IVcValueScalePtr pValueScale = pPlot->GetAxis(MSChart20Lib::VtChAxisIdY)->ValueScale;
	MSChart20Lib::IVcRectPtr pChartRect = pPlot->LocationRect;
	CRect rcInterior((long)(pChartRect->Min->GetX() / 15.0f), (long)(pChartRect->Min->GetY() / 15.0f),
		(long)(pChartRect->Max->GetX() / 15.0f), (long)(pChartRect->Max->GetY() / 15.0f));
	double dMin = 0;
	double dMax = (double)m_fGraphMax;	
	m_chart.GetWindowRect(&rcChart);
	for (long i=0; i < 7; i++) pLabel[i]->SetClipBounds(&rcChart);
	ScreenToClient(&rcChart);
	GetDlgItem(IDC_BACKGROUND)->GetWindowRect(&rcColor);
	ScreenToClient(&rcColor);

	// Set the "Baseline Goals" label position
	m_LegendLabel.SetWindowPos(NULL, rcChart.right - 15, rcChart.top + 5, 0,0, SWP_NOZORDER | SWP_NOSIZE);

	// Determine the rectangle that contains the bounds of the Y-axis.
	// This is kinda tricky, and mostly ad-hoc because the MSChart control
	// doesn't expose a lot of functions that I need to do this right.
	long nTop = rcChart.top + rcInterior.top;
	long nBottom = rcChart.bottom - 75;
	long nLeft = rcChart.right - 15;
	if (rcChart.Height() <= 142)
	{
		nTop += 42;
		nBottom += 42;
	}
	else if (rcChart.Width() <= 500)
	{
		nTop -= 42;
		nBottom -= 42;
	}

	//
	// (c.haag 2005-11-25 14:58) - This code has existed for a while, but has never been well commented.
	// Rendering the graph labels and lines is a two step process for each label. First, you need to 
	// calculate the actual true Y value of the label; that is, where exactly on the vertical scale of
	// the graph the label belongs. Once that is determined, you need to see if it overlaps any other labels;
	// and if it does, push it below those labels.
	//

	// Determine the initial position of each performance legend
	long nMaxBottom = 0;
	for (i=0; i < nColumns; i++)
	{
		CString strLabel;
		double nGoal;

		//
		// Step 1: Determine the true Y value of the label
		//

		// If we are doing a ratio, make it a function of the maximum value of the axis
		if (m_astrPropKeys[i] == "MarketClosureRateGoal")
		{
			nGoal = (double)GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]);
			nGoal = nGoal * (dMax - dMin) / 100.0f;
		}
		else if (m_astrPropKeys[i] == "MarketClosureTimeSpanGoal")
		{
			nGoal = GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]);
		}
		else
			nGoal = ScaleGoal(GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]));

		rc.left = nLeft;
		rc.right = rcColor.right;

		if (nGoal > dMax - dMin)
		{
			rc.top = nTop;
		}
		else
		{
			if (m_astrPropKeys[i] != "MarketClosureRateGoal")
				nGoal = (long)nGoal;
			if (dMax - dMin != 0)
				rc.top = nBottom - (long)((double)(nBottom - nTop) * (nGoal / (dMax - dMin))) ;
			else
				rc.top = nBottom;
		}
		rc.bottom = rc.top + 15;

		// (c.haag 2005-11-25 13:19) - PLID 18452 - Now store the true Y value
		// of the label. The key is to do this before we stack the labels on each other.
		// Before today, we did this after stacking the labels on each other, resulting
		// in each line being horizontal.
		//
		CPoint ptLine(0, (rc.top + rc.bottom) / 2);
		ClientToScreen(&ptLine);
		pLabel[i]->MoveWindow(rc);
		pLabel[i]->ScreenToClient(&ptLine);
		pLabel[i]->SetTrueYPos(ptLine.y);

		//
		// Step 2: Move the label so that it doesn't overlap any other label
		//

		// At this point, rc is the rectangle with the correct placement of the label. The
		// problem is that labels can overlap each other, so we want to push all the labels that
		// overlap downwards. Before we do, we want to retain where the line should appear.
		CRect rcIntersection;
		while (IntersectsOtherLabel(i, rc, rcIntersection))
		{
			int nOffset = rcIntersection.bottom - rc.top;
			rc.top += nOffset;
			rc.bottom += nOffset;
			pLabel[i]->SetTrueYPos( pLabel[i]->GetTrueYPos() - nOffset );
		}
		nMaxBottom = max(nMaxBottom, rc.bottom);
		pLabel[i]->MoveWindow(rc);
	}

	//
	// Finally, if the bottom is below the graph, shift all the labels up
	//
	if (nMaxBottom > rcChart.bottom)
	{
		long nMove = nMaxBottom - rcChart.bottom;
		for (long i=0; i < nColumns; i++)
		{
			pLabel[i]->GetWindowRect(&rc);
			ScreenToClient(&rc);
			rc.top -= nMove;
			rc.bottom -= nMove;
			pLabel[i]->MoveWindow(rc);
			pLabel[i]->SetTrueYPos( pLabel[i]->GetTrueYPos() + nMove );
		}
	}
}

void CMarketBaselineDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rc, rcChart;
	CPen pen(PS_SOLID, 1, RGB(255,255,255));
	GetDlgItem(IDC_BACKGROUND)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_chart.GetWindowRect(&rcChart);
	ScreenToClient(&rcChart);
	rcChart.left = rcChart.right;
	rcChart.right = rc.right;

	CPen* pOldPen = dc.SelectObject(&pen);
	dc.Rectangle(rcChart);
	dc.SelectObject(pOldPen);
	pen.DeleteObject();
}

void CMarketBaselineDlg::ScaleGraph()
{
	MSChart20Lib::IVcPlotPtr pPlot = m_chart.GetPlot();
	MSChart20Lib::IVcAxisPtr pAxis = pPlot->GetAxis(MSChart20Lib::VtChAxisIdY);
	MSChart20Lib::IVcAxisScalePtr pAxisScale = pAxis->GetAxisScale();
	MSChart20Lib::IVcValueScalePtr pValueScale = pAxis->ValueScale;

	long nMax = (long)m_fGraphMax;
	while (nMax % 20) nMax++;

	pValueScale->Minimum = 0;
	pValueScale->Maximum = (double)nMax;
}

double CMarketBaselineDlg::ScaleGoal(long nGoal)
{
	COleDateTime dtFrom;
	COleDateTime dtTo;
	GetParameters(dtFrom, dtTo);
	COleDateTimeSpan dtInterval = dtTo - dtFrom;
	double nDaysPerInterval = dtInterval.GetTotalDays() / (double)m_nIntervals;
	return (double)nGoal * nDaysPerInterval / 30.0;
}

void CMarketBaselineDlg::PrePrint()
{
/*	// (c.haag 2004-05-03 17:39) - Trick the chart into always drawing big.
	CRect rcChart;
	m_chart.GetWindowRect(&rcChart);
	ScreenToClient(&rcChart);
	SetRedraw(FALSE); // Don't show the user that we resize the graph
	m_chart.MoveWindow(0,0,800,800);
	m_chart.EditCopy();
	m_hBitmapPrint = NULL;

	// make sure the item in clipboard is a bitmap
	if(IsClipboardFormatAvailable(CF_BITMAP))
	{
		if(OpenClipboard())
		{
			m_hBitmapPrint = (HBITMAP)::GetClipboardData(CF_BITMAP);
			CloseClipboard();
		}
	}
	m_chart.MoveWindow(rcChart.left, rcChart.top, rcChart.Width(), rcChart.Height());
	SetRedraw(TRUE);*/
	CMarketingDlg::PrePrint();
}

void CMarketBaselineDlg::Print(CDC * pDC, CPrintInfo * pInfo)
{
	CRect rectPage = pInfo->m_rectDraw;
	CFont fntHeader;
	CFont* pOldFont;
	CPen* pOldPen;
	CString str;
	COleDateTime dtFrom;
	COleDateTime dtTo;
	LOGFONT lf;
	double dMin = 0;
	double dMax = 0;
	//
	// (c.haag 2005-11-30 13:44) - PLID 18472 - We now calculate the header height based
	// on the title and subtitle text heights.
	//
	long nHeaderHeight;// = 400;
	unsigned short nColumns = GetGraphColumnCount();
	unsigned short i;
	long x;
	long y;

	if (m_bLastYear) nColumns /= 2;
	if (/*!m_hBitmapPrint ||*/0 == nColumns) return;

	///////////////////////////////////////////////////////////
	// Set Margins
	rectPage.top+=rectPage.bottom/48;
	rectPage.bottom-=rectPage.bottom/48;
	rectPage.left+=200;
	rectPage.right-=200;

	///////////////////////////////////////////////////////////
	// Set the header height to twice the font height
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&fntHeader, 200, "Arial", pDC);
	fntHeader.GetLogFont(&lf);
	nHeaderHeight = -lf.lfHeight * 3;
	if (m_bLastYear) {
		CFont fntSubHeader;
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(&fntSubHeader, 100, "Arial", pDC);
		fntSubHeader.GetLogFont(&lf);
		nHeaderHeight += -lf.lfHeight * 3;
		fntSubHeader.DeleteObject();
	}

	///////////////////////////////////////////////////////////
	// Print the header
	fntHeader.GetLogFont(&lf);
	CRect rcHeader(rectPage.left, rectPage.top + (-lf.lfHeight/2), 
		rectPage.right, rectPage.top + nHeaderHeight);
	pOldFont = pDC->SelectObject(&fntHeader);
	GetParameters(dtFrom, dtTo);
	str.Format("Performance Indices from %s to %s",
		FormatDateTimeForInterface(dtFrom, dtoDate),
		FormatDateTimeForInterface(dtTo, dtoDate));
	pDC->DrawText(str, &rcHeader, DT_NOCLIP | DT_CENTER);
	pDC->SelectObject(pOldFont);
	fntHeader.DeleteObject();
	if (m_bLastYear)
	{
		CFont fntSubHeader;
		LOGFONT lfSubHeader;
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(&fntSubHeader, 100, "Arial", pDC);
		fntSubHeader.GetLogFont(&lfSubHeader);
		CRect rcSubHeader(rectPage.left, rectPage.top + (-lf.lfHeight) + (-lfSubHeader.lfHeight * 2),
			rectPage.right, rectPage.top + nHeaderHeight);
		pOldFont = pDC->SelectObject(&fntSubHeader);
		pDC->DrawText("(Dashed lines denote values for previous year)", &rcSubHeader, DT_NOCLIP | DT_CENTER);
		pDC->SelectObject(pOldFont);
		fntSubHeader.DeleteObject();
	}


	// (j.gruber 2007-03-20 11:32) - PLID 25260 - add filters
	//we are going to use a smaller font for this 
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&fntHeader, 100, "Arial", pDC);
	pOldFont = pDC->SelectObject(&fntHeader);

	//set our variables
	CString strLocFilter, strProvFilter, strDateFilter, strPatFilter, strReferralFilter;

	CMainFrame *pMain = GetMainFrame();
	if (pMain) {
		CDocBar *pDoc = pMain->m_pDocToolBar;
		if (pDoc) {

			strLocFilter = pDoc->GetLocationFilterString();
			strProvFilter = pDoc->GetProviderFilterString();
			strPatFilter = pDoc->GetPatFilterFilterString();
			strDateFilter = pDoc->GetDateFilterString();
	
			//get the rect
			//dates first according to Meikin
			CRect rcFilterString;
			rcFilterString.top = nHeaderHeight;
			rcFilterString.left = rectPage.left;
			rcFilterString.right = rectPage.right;
			rcFilterString.bottom = rcFilterString.top + pDC->GetTextExtent(strDateFilter).cy;
			
			pDC->DrawText(strDateFilter, &rcFilterString, DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);
			CRect rcTemp = rcFilterString;
			pDC->DrawText(strDateFilter, &rcTemp, DT_CALCRECT|DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);

			rcFilterString.top = rcTemp.bottom + 1;
			rcFilterString.bottom = rcFilterString.top + pDC->GetTextExtent(strLocFilter).cy;

			
			pDC->DrawText(strLocFilter, &rcFilterString, DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);
			rcTemp = rcFilterString;
			pDC->DrawText(strLocFilter, &rcTemp, DT_CALCRECT|DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);

			rcFilterString.top = rcTemp.bottom + 1;
			rcFilterString.bottom = rcFilterString.top + pDC->GetTextExtent(strProvFilter).cy;

			pDC->DrawText(strProvFilter, &rcFilterString, DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);
			rcTemp = rcFilterString;
			pDC->DrawText(strProvFilter, &rcTemp, DT_CALCRECT|DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);
			
			rcFilterString.top = rcTemp.bottom + 1;
			rcFilterString.bottom = rcFilterString.top + pDC->GetTextExtent(strPatFilter).cy;

			pDC->DrawText(strPatFilter, &rcFilterString, DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);
			rcTemp = rcFilterString;
			pDC->DrawText(strPatFilter, &rcTemp, DT_CALCRECT|DT_LEFT|DT_NOCLIP|DT_TOP|DT_WORDBREAK);

	
			nHeaderHeight = rcTemp.bottom;
		}
	}

	pDC->SelectObject(pOldFont);
	fntHeader.DeleteObject();

	///////////////////////////////////////////////////////////
	// Print the graph
	CPen penThinBlack(PS_SOLID, 1, RGB(0,0,0));
	CPen penThinGray(PS_SOLID, 1, RGB(192,192,192));
	CFont fntAxis;
	double nGraphTop = m_fGraphMax;
	double nGraphInterval;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&fntAxis, 110, "Arial", pDC);
	pOldFont = pDC->SelectObject(&fntAxis);

	// (c.haag 2005-11-25 14:04) - PLID 16976 - Calculate the axis label width and height
	CRect rcCalcWidth;
	fntAxis.GetLogFont(&lf);
	pDC->DrawText("000000", &rcCalcWidth, DT_NOCLIP | DT_SINGLELINE | DT_CALCRECT);
	const long nAxisLabelHeight = -lf.lfHeight;
	const long nAxisLabelWidth = rcCalcWidth.Width();

	// Calculate the graph interval and round up the top of the graph
	if (nGraphTop < 1) { nGraphTop = 1; nGraphInterval = 0.1; }
	else if (nGraphTop < 10) { nGraphTop = 10; nGraphInterval = 1; }
	else if (nGraphTop < 100) nGraphInterval = 10;
	else if (nGraphTop < 1000) nGraphInterval = 100;
	else if (nGraphTop < 10000) nGraphInterval = 1000;
	else if (nGraphTop < 100000) nGraphInterval = 10000;
	else if (nGraphTop < 1000000) nGraphInterval = 100000;
	if (nGraphInterval > 1) nGraphTop += nGraphInterval - (((long)nGraphTop) % (long)nGraphInterval);
	
	// Print at 70% size within left/right margin 
	CSize printSize;
	printSize.cx=(int)(rectPage.right*.70);
	printSize.cy=(int)(printSize.cx*.55);
	x = rectPage.left + nAxisLabelWidth;
	y = rectPage.top + nHeaderHeight;

	// Draw the borders
	pOldPen = pDC->SelectObject(&penThinBlack);
	pDC->MoveTo(x, y);
	pDC->LineTo(x, y + printSize.cy);
	pDC->MoveTo(x + printSize.cx, y);
	pDC->LineTo(x + printSize.cx, y + printSize.cy);

	// Draw the bottom tick mark
	pDC->MoveTo(x - 10, y + printSize.cy);
	pDC->LineTo(x + 1, y + printSize.cy);

	// Draw the tick marks, gray lines and Y labels
	pDC->SetTextColor(RGB(0,0,0));
	//for (i=(unsigned short)y; i < (unsigned short)(y + printSize.cy); i += 200)
	for (double m=nGraphInterval; m <= nGraphTop; m += nGraphInterval)
	{
		long nTickY = y + printSize.cy - (long)((m * (double)printSize.cy) / nGraphTop);
		CRect rcText(x - nAxisLabelWidth, nTickY - nAxisLabelHeight / 2, 
			x, nTickY + nAxisLabelHeight / 2);
		CString str;
		if (m >= 1)	str.Format("%d", (long)m);
		else str.Format("%.1f", m);
		pDC->DrawText(str, &rcText, DT_NOCLIP | DT_LEFT);

		pDC->SelectObject(&penThinBlack);
		pDC->MoveTo(x - 10, nTickY);
		pDC->LineTo(x + 10, nTickY);
		pDC->MoveTo(x + printSize.cx, nTickY);
		pDC->LineTo(x + printSize.cx + 5, nTickY);		

		pDC->SelectObject(&penThinGray);
		pDC->MoveTo(x + 10, nTickY);
		pDC->LineTo(x + printSize.cx, nTickY);

		dMax = max(dMax, m);
	}

	// Draw the row labels
	for (i = 0; i < m_chart.GetRowCount(); i++)
	{
		long xCenter = x + ((i+1) * printSize.cx) / (m_chart.GetRowCount()+1);
		long yCenter = y + printSize.cy + nAxisLabelHeight;
		CRect rcText(xCenter - nAxisLabelWidth / 2, yCenter, xCenter + nAxisLabelWidth / 2, yCenter + nAxisLabelHeight);
		m_chart.SetRow(i+1);
		pDC->DrawText(m_chart.GetRowLabel(), &rcText, DT_NOCLIP | DT_CENTER);
	}

	// Plot the chart
	for (i = 0; i < m_chart.GetColumnCount(); i++)
	{
		CPen pen;
		if (m_bLastYear && i >= nColumns)
			pen.CreatePen(PS_DASH, 1, m_aclrRowLines[i]);
		else
			pen.CreatePen(PS_SOLID, 8, m_aclrRowLines[i]);

		pOldPen = pDC->SelectObject(&pen);

		m_chart.SetColumn(i+1);
		for (unsigned short j=0; j < m_chart.GetRowCount(); j++)
		{
			m_chart.SetRow(j+1);
			double d = atof(m_chart.GetData());
			long xCenter = x + ((j+1) * printSize.cx) / (m_chart.GetRowCount()+1);
			long yCenter = y + printSize.cy - (long)((d * (double)printSize.cy) / nGraphTop);

			if (j == 0)	pDC->MoveTo(xCenter,yCenter);
			else pDC->LineTo(xCenter,yCenter);
		}
		pDC->SelectObject(pOldPen);
		pen.DeleteObject();
	}

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);
	penThinBlack.DeleteObject();
	penThinGray.DeleteObject();
	fntHeader.DeleteObject();

	///////////////////////////////////////////////////////////
	// Print the lines and the legend
	CRect rcLabels[MAX_INDICES];
	long nYValue[MAX_INDICES];
	long nTop = nHeaderHeight + rectPage.top;
	long nBottom = nTop + printSize.cy;
	long nLeft = rectPage.left + nAxisLabelWidth + printSize.cx;

	// (c.haag 2005-11-25 14:04) - PLID 16976 - Calculate the text height
	CFont font;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&font, 90, "Arial", pDC);
	font.GetLogFont(&lf);
	const long nTextHeight = (-lf.lfHeight * 6) / 4; // Make the font size at 150% so there's more space between labels

	//
	// (c.haag 2005-11-25 14:58) - This code has existed for a while, but has never been well commented.
	// Rendering the graph labels and lines is a two step process for each label. First, you need to 
	// calculate the actual true Y value of the label; that is, where exactly on the vertical scale of
	// the graph the label belongs. Once that is determined, you need to see if it overlaps any other labels;
	// and if it does, push it below those labels.
	//

	for (i=0; i < nColumns; i++)
	{
		double nGoal;

		//
		// Step 1: Determine the true Y value of the label
		//

		// If we are doing a ratio, make it a function of the maximum value of the axis
		if (m_astrPropKeys[i] == "MarketClosureRateGoal")
		{
			nGoal = (double)GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]);
			nGoal = nGoal * (dMax - dMin) / 100.0f;
		}
		else if (m_astrPropKeys[i] == "MarketClosureTimeSpanGoal")
		{
			nGoal = GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]);
		}
		else
			nGoal = ScaleGoal(GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]));

		if (m_astrPropKeys[i] != "MarketClosureRateGoal")
			nGoal = (long)nGoal;
		if (dMax - dMin != 0)
			nYValue[i] = nBottom - (long)((double)(nBottom - nTop) * (nGoal / (dMax - dMin))) ;
		else
			nYValue[i] = nBottom;

		// Calculate the text placement
		CRect rcText(nLeft + 130, nYValue[i] - nTextHeight / 2, x + 60, nYValue[i] + nTextHeight / 2);
		rcLabels[i] = rcText;

		//
		// Step 2: Move the label so that it doesn't overlap any other label
		//

		// Make sure the text won't overlap
		CRect rcIntersection;
		while (IntersectsOtherPrintLabel(rcLabels, i, rcLabels[i], rcIntersection))
		{
			int nOffset = rcIntersection.bottom - rcLabels[i].top;
			rcLabels[i].top += nOffset;
			rcLabels[i].bottom += nOffset;
		}
	}

	//
	// Now we actually draw the labels and lines to the right of the graph
	//

	for (i=0; i < nColumns; i++)
	{
		CPen pen(PS_SOLID, 2, m_aclrRowLines[i]);
		CString strLabel;
		pOldFont = pDC->SelectObject(&font);
		pOldPen = pDC->SelectObject(&pen);
		double nGoal;

		// Calculate the label
		nGoal = (m_astrPropKeys[i] != "MarketClosureRateGoal" && m_astrPropKeys[i] != "MarketClosureTimeSpanGoal") ? ScaleGoal(GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i])) : (double)GetRemotePropertyInt(m_astrPropKeys[i], m_aPropDefaults[i]);

		if (!m_astrPropKeys[i].IsEmpty())
		{
			if (nGoal > 0 && nGoal < 1)
				strLabel.Format("%s (<1)", m_astrLegendLabels[i], (long)nGoal);
			else
				strLabel.Format("%s (%d)", m_astrLegendLabels[i], (long)nGoal);
		}
		else
		{
			strLabel = m_astrLegendLabels[i];
		}

		// Draw the line
		pDC->MoveTo(nLeft, nYValue[i]);
		pDC->LineTo(nLeft + 40, nYValue[i]);
		pDC->LineTo(nLeft + 80, (rcLabels[i].top + rcLabels[i].bottom) / 2);
		pDC->LineTo(nLeft + 120, (rcLabels[i].top + rcLabels[i].bottom) / 2);

		// Draw the text
		pDC->DrawText(strLabel, &rcLabels[i], DT_NOCLIP | DT_LEFT);

		// Cleanup
		pDC->SelectObject(pOldPen);
		pen.DeleteObject();
		pDC->SelectObject(pOldFont);
	}

	font.DeleteObject();
}

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
CString CMarketBaselineDlg::GetReportSQL(ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{
	COleDateTime dtFrom;
	COleDateTime dtTo;
	CString strSql;

	GetParameters(dtFrom, dtTo);

	if (m_btnConsultations.GetCheck())
	{
		CString str;
		str.Format("SELECT 'Patients with Consultations' AS Name, Last + ', ' + First + ' ' + Middle AS PatientName, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip from appointmentst AS ApptsQ "
			"LEFT JOIN persont on persont.id = ApptsQ.patientid left join patientst on patientst.personid = ApptsQ.patientid "
			"where patientid > -1 and apttypeid in (select id from apttypet where Category = 1) AND ApptsQ.status <> 4 AND showstate <> 3 "
			"AND StartTime > '%s' AND StartTime <= '%s' ", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		FilterAppointmentSQL(str, pCon, strPatientTempTable);
		str += " GROUP BY Last + ', ' + First + ' ' + Middle, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip";
		if (strSql.IsEmpty()) strSql = str;
		else
		{
			str = "UNION ALL SELECT" + str.Right( str.GetLength() - 6 );
			strSql += CString(" ") + str;
		}
	}
	if (m_btnSurgeries.GetCheck())
	{
		CString str;
		str.Format("SELECT 'Patients with Surgeries' AS Name, Last + ', ' + First + ' ' + Middle AS PatientName, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip from appointmentst AS ApptsQ "
			"LEFT JOIN persont on persont.id = ApptsQ.patientid left join patientst on patientst.personid = ApptsQ.patientid "
			"where patientid > -1 and apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND ApptsQ.status <> 4 AND showstate <> 3 "
			"AND StartTime > '%s' AND StartTime <= '%s' AND StartTime < GetDate() ", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		FilterAppointmentSQL(str, pCon, strPatientTempTable);
		str += " GROUP BY Last + ', ' + First + ' ' + Middle, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip";
		if (strSql.IsEmpty()) strSql = str;
		else
		{
			str = "UNION ALL SELECT" + str.Right( str.GetLength() - 6 );
			strSql += CString(" ") + str;
		}
	}
	if (m_btnClosure.GetCheck() || m_btnClosureLength.GetCheck())
	{
		CString str;
		str.Format("SELECT 'Patients with Closure (consults that progress into surgeries) and PrePayments' AS Name, Last + ', ' + First + ' ' + Middle AS PatientName, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip FROM AppointmentsT AS ApptsQ "
			"LEFT JOIN (SELECT ID, StartTime, PatientID FROM AppointmentsT where apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND status <> 4 AND showstate <> 3 AND patientid > -1) SurgQ on "
			"(SurgQ.PatientID = ApptsQ.PatientID AND SurgQ.StartTime > ApptsQ.StartTime) "
			"LEFT JOIN PersonT ON PersonT.ID = ApptsQ.PatientID "
			"LEFT JOIN PatientsT ON PatientsT.PersonID = ApptsQ.PatientID "
			"where apttypeid in (select id from apttypet where Category = 1) AND ApptsQ.status <> 4 AND showstate <> 3 "
			"AND ApptsQ.patientid in (select patientid from lineitemt inner join paymentst on paymentst.id = lineitemt.id where lineitemt.patientid = ApptsQ.patientid and prepayment = 1 and lineitemt.id not in (select sourceid from appliest)) "
			"AND ApptsQ.StartTime < GetDate() "
			"AND ApptsQ.StartTime > '%s' AND ApptsQ.StartTime <= '%s' "
			"AND SurgQ.ID IS NOT NULL "
			"AND (SELECT Count(A2.ID) FROM (SELECT ID, StartTime FROM AppointmentsT WHERE PatientID = ApptsQ.PatientID and apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND StartTime > ApptsQ.StartTime AND StartTime < SurgQ.StartTime AND ID <> ApptsQ.ID) A2 WHERE A2.ID IN (SELECT AppointmentPurposeT.AppointmentID FROM AppointmentPurposeT INNER JOIN AppointmentPurposeT AS A3 ON AppointmentPurposeT.PurposeID = A3.PurposeID WHERE AppointmentPurposeT.AppointmentID = A2.ID AND ApptsQ.ID = A3.AppointmentID)) = 0 ",
			FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		FilterAppointmentSQL(str, pCon, strPatientTempTable);
		str += " GROUP BY Last + ', ' + First + ' ' + Middle, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip";
		if (strSql.IsEmpty()) strSql = str;
		else
		{
			str = "UNION ALL SELECT" + str.Right( str.GetLength() - 6 );
			strSql += CString(" ") + str;
		}
	}
	if (m_btnAvgProc.GetCheck())
	{
		CString str;
		str.Format("SELECT 'Patients who have Scheduled Consults and Surgeries' AS Name, Last + ', ' + First + ' ' + Middle AS PatientName, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip FROM (SELECT StartTime, ConsultID, LocationID, PatientID FROM (select ApptsInnerQ.id as ConsultID, ApptsInnerQ.starttime, ApptsInnerQ.locationid, ApptsInnerQ.patientid, SurgQ.id AS SurgeryID FROM AppointmentsT AS ApptsInnerQ "
			"LEFT JOIN (SELECT ID, StartTime, PatientID FROM AppointmentsT where PatientID > -1 AND apttypeid in (select id from apttypet where Category = 3 OR Category = 4) AND status <> 4 AND showstate <> 3) SurgQ on (SurgQ.PatientID = ApptsInnerQ.PatientID AND SurgQ.StartTime > ApptsInnerQ.StartTime) "
			"where apttypeid in (select id from apttypet where Category = 1) AND ApptsInnerQ.status <> 4 AND showstate <> 3 AND ApptsInnerQ.patientid > -1 "
			"AND ApptsInnerQ.patientid in (select patientid from lineitemt inner join paymentst on paymentst.id = lineitemt.id where lineitemt.patientid = ApptsInnerQ.patientid and prepayment = 1 and lineitemt.id not in (select sourceid from appliest)) "
			"AND ApptsInnerQ.StartTime < GetDate() "
			"AND ApptsInnerQ.StartTime > '%s' AND ApptsInnerQ.StartTime <= '%s' "
			"AND SurgQ.ID IS NOT NULL "
			"AND (SELECT Count(A2.ID) FROM (SELECT ID, StartTime FROM AppointmentsT WHERE PatientID = apptsinnerq.PatientID and apttypeid in (select id from apttypet where Category = 1) AND status <> 4 AND showstate <> 3 AND StartTime > apptsinnerq.StartTime AND StartTime < SurgQ.StartTime AND ID <> apptsinnerq.ID) A2 WHERE A2.ID IN (SELECT AppointmentPurposeT.AppointmentID FROM AppointmentPurposeT INNER JOIN AppointmentPurposeT AS A3 ON AppointmentPurposeT.PurposeID = A3.PurposeID WHERE AppointmentPurposeT.AppointmentID = A2.ID AND apptsinnerq.ID = A3.AppointmentID)) = 0 "
			") SubQ group by ConsultID, StartTime, LocationID, PatientID) ApptsQ "
			"LEFT JOIN PersonT ON PersonT.ID = ApptsQ.PatientID "
			"LEFT JOIN PatientsT ON PatientsT.PersonID = ApptsQ.PatientID "
			"WHERE 1=1 ",
			FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		FilterAppointmentSQL(str, pCon, strPatientTempTable);
		str += " GROUP BY Last + ', ' + First + ' ' + Middle, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip";
		if (strSql.IsEmpty()) strSql = str;
		else
		{
			str = "UNION ALL SELECT" + str.Right( str.GetLength() - 6 );
			strSql += CString(" ") + str;
		}
	}
	if (m_btnTrackingNoShow.GetCheck())
	{
		CString str;
		str.Format("SELECT 'No Show Patients' AS Name, Last + ', ' + First + ' ' + Middle AS PatientName, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip from appointmentst AS ApptsQ "
			"LEFT JOIN PersonT ON PersonT.ID = ApptsQ.PatientID "
			"LEFT JOIN PatientsT ON PatientsT.PersonID = ApptsQ.PatientID "
			"WHERE ApptsQ.status <> 4 and showstate = 3 and patientid > -1 "
			"AND StartTime > '%s' AND StartTime <= '%s' ", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		FilterAppointmentSQL(str, pCon, strPatientTempTable);
		str += " GROUP BY Last + ', ' + First + ' ' + Middle, UserDefinedID, HomePhone, WorkPhone, CellPhone, Address1, Address2, City, State, Zip";
		if (strSql.IsEmpty()) strSql = str;
		else
		{
			str = "UNION ALL SELECT" + str.Right( str.GetLength() - 6 );
			strSql += CString(" ") + str;
		}
	}
	if (m_btnInquires.GetCheck() && !m_btnTrackingNoShow.GetCheck() && !m_btnAvgProc.GetCheck() &&
		!m_btnClosure.GetCheck() && !m_btnClosureLength.GetCheck() && !m_btnSurgeries.GetCheck() && 
		!m_btnConsultations.GetCheck()) {
		//the only thing checked is inquiries, and we don't have a report for that
		MessageBox("There is no text report for this option, you will be shown the graphical version.");
		return "";
	}

	return strSql + " ORDER BY Name, Last + ', ' + First + ' ' + Middle";
}
void CMarketBaselineDlg::OnActivitiesSetmarketingbaselinegoals() 
{
	CMarketBaselineConfig dlg(this);
	if (IDOK == dlg.DoModal())
		OnGo();
}

void CMarketBaselineDlg::OnGo() 
{
	m_bRenderedOnce = true;
	m_chart.SetTitleText("");
	UpdateView();
}

void CMarketBaselineDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
// a.walling PLID 20695 5/25/06 set the graph to a blank state and update the status of any controls
//			set bClear to false if you just want to reset/enable the render button
// (c.haag 2007-03-15 16:53) - PLID 24253 - Added support for forced refreshes (the parent view uses them)
{
	if (!m_bActive && !bForceReset) { // switching from another tab or module.
		return;
	}

	if (bClear && m_bDirtyGraph) {
		// memory is freed in the control
		m_chart.SetColumnCount(0);
		m_chart.SetRowCount(0);

		if (strTitle)
			m_chart.SetTitleText((LPCSTR)strTitle); // change the title to invalidate the graph and force a refresh
		else
			m_chart.SetTitleText("");

		RedrawWindow();
		m_bDirtyGraph = false;
	}

	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOW);
}


void CMarketBaselineDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	CMarketingDlg::OnShowWindow(bShow, nStatus);
	
}

void CMarketBaselineDlg::OnCheckInquiries() 
{
	ResetGraph(true);
}

void CMarketBaselineDlg::OnCheckConsultations() 
{
	ResetGraph(true);	
}

void CMarketBaselineDlg::OnCheckClosure() 
{
	ResetGraph(true);	
}

void CMarketBaselineDlg::OnCheckSurgeries() 
{
	ResetGraph(true);	
}

void CMarketBaselineDlg::OnCheckClosureLength() 
{
	ResetGraph(true);	
}

void CMarketBaselineDlg::OnCheckAvgproc() 
{
	ResetGraph(true);	
}

void CMarketBaselineDlg::OnCheckTrackingnoshow() 
{
	ResetGraph(true);	
}

void CMarketBaselineDlg::OnCheckShowprevyear() 
{
	ResetGraph(true);	
}

LRESULT CMarketBaselineDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	ClearCachedRS(); // the filters may have changed so all our cached data is inaccurate
	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters
	
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	GetDlgItem(IDC_GO)->EnableWindow(true);
	
	return 0;
}

// a.walling PLID 20695 6/02/06 marketing overhaul, caching the recordsets to prevent redundant access
void CMarketBaselineDlg::ClearCachedRS()
{
	for (int i = 0; i < CHARTLINES; i++) {
		if (pCachedRS[i]) {
			pCachedRS[i]->Close(); // close the RS and free the ref count by setting to null
			pCachedRS[i] = NULL;
		}
		if (pLastYearCachedRS[i]) {
			pLastYearCachedRS[i]->Close();
			pLastYearCachedRS[i] = NULL;
		}
	}
}

// (j.gruber 2007-09-13 16:35) - PLID 27370 - function to check that the dates will come out ok
BOOL CMarketBaselineDlg::CheckDates(COleDateTime dtTo, COleDateTime dtFrom, long nIntervals) {

	COleDateTime dtOriginalFrom;
	COleDateTimeSpan dtInterval = dtTo - dtFrom;

	//(e.lally 2009-08-28) PLID 35387 - Check some more special cases
	if(dtInterval.GetTotalDays() < 1){
		if(dtInterval.GetTotalDays() <0){
			MsgBox("The date range must have a start date that is before the end date.");
			return FALSE;
		}
		MsgBox("The date range must be at least one day apart.");
		return FALSE;
	}
	if(nIntervals <= 1){
		//(e.lally 2009-08-28) PLID 35387 - This should not be possible, but we need to prevent the divide by zero error
		//	The logic that follows also really fails with a negative interval - 1.
		ASSERT(FALSE);
		return FALSE;
	}

	// (c.haag 2004-04-26 11:58) - dtInterval gives us the number of days that span the graph.
	// For example, 4/1/04 through 4/4/04 actually demands 4 days: 4/1, 4/2, 4/3 and 4/4. Now
	// we want to figure out the real "from" date
	double dtDayInterval = dtInterval.GetTotalDays() / (double)(nIntervals-1);
	dtOriginalFrom = dtFrom - COleDateTimeSpan((long)dtDayInterval, 0,0,0);
	dtFrom = dtOriginalFrom;

	// (j.gruber 2007-09-13 14:23) - PLID 27370 - check to make sure we aren't using invalid dates
	COleDateTime dtMin;
	dtMin.SetDate(1753,12,31);

	for (long i=0; i < nIntervals; i++)
	{
		// Calculate the intermediate date to plot
		COleDateTime dt = dtFrom;
		long nDays = max(1,(long)(((double)(i+1)) * dtDayInterval));
		dt = dtOriginalFrom + COleDateTimeSpan(nDays, 0,0,0);

		if (dtMin >= dtFrom) {
			MsgBox("The date intervals calculated from the specified date range include invalid dates (before the year 1753).\nThis could be caused by using a very large span between the to date and from date.\n Please correct this and run the graph again.");
			return FALSE;
		}
	}

	return TRUE;
}
//(a.wilson 2011-10-5) PLID 38789 - placed common functionality into a function to prevent repetition of code.
void CMarketBaselineDlg::HideAndRedraw()
{
	CNxBaselineStatic* pLabel[MAX_INDICES] = 
	{
		&m_Legend1, &m_Legend2, &m_Legend3, &m_Legend4,
		&m_Legend5, &m_Legend6, &m_Legend7,
	};
	for (long i=0; i < 7; i++)
	{
		pLabel[i]->ShowWindow(SW_HIDE);
	}
	m_LegendLabel.ShowWindow(SW_HIDE);
	//GetDlgItem(IDC_STATIC_PATIENTS_UNLIMITED)->ShowWindow(SW_HIDE); // (v.maida 2014-08-04 04:18) - PLID 58633 - The pumc.com link needs to be removed from the performances indicies tab. 
	m_chart.SetColumnCount(0);
	m_chart.SetRowCount(0);
	RedrawWindow();
}
