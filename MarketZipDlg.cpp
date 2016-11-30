// MarketZipDlg.cpp : implementation file
//
// (c.haag 2008-04-18 12:33) - PLID 29713 - Removed MSChart dependencies
//

#include "stdafx.h"
#include "practice.h"
#include "MarketZipDlg.h"
#include "MarketUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "MultiSelectDlg.h"
#include "MarketFilterPickerDlg.h"
#include "Docbar.h"
#include "MarketingRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CMarketZipDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketZipDlg::CMarketZipDlg(CWnd* pParent)
	: CMarketingDlg(IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketZipDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/regional.htm";
	m_pReferralSubDlg = NULL;
}

CMarketZipDlg::~CMarketZipDlg()
{
	DestroyWindow();
}

void CMarketZipDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketZipDlg)
	DDX_Control(pDX, IDC_GO, m_Go);
	DDX_Control(pDX, IDC_ALL_REFERRALS_CHECK, m_btnAllSources);
	DDX_Control(pDX, IDC_SEL_REFERRALS_CHECK, m_btnSelSources);
	DDX_Control(pDX, IDC_CITY_STATE, m_btnCityState);
	DDX_Control(pDX, IDC_FIRST_THREE, m_btnFirstThree);
	DDX_Control(pDX, IDC_MKTZIP_REFERRAL_AREA, m_btnMktzipReferralArea);
	DDX_Control(pDX, IDC_STATIC_PIE_GRAPH_REGION, m_btnPieGraphRegion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketZipDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketZipDlg)
	ON_BN_CLICKED(IDC_ALL_REFERRALS_CHECK, OnAllReferralsCheck)
	ON_BN_CLICKED(IDC_SEL_REFERRALS_CHECK, OnSelReferralsCheck)
	ON_BN_CLICKED(IDC_CITY_STATE, OnCityState)
	ON_BN_CLICKED(IDC_FIRST_THREE, OnFirstThree)
	ON_BN_CLICKED(IDC_GO, OnGo)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketZipDlg message handlers

void CMarketZipDlg::Refresh()
{
	// (c.haag 2008-04-18 12:34) - PLID 29713 - Replaced MSChart code with code
	// to utilize the member NexTech pie graph window
	CWaitCursor wait;

	//If any table checkers have been fired, this will refresh the list.  Save the current selection.
	{
		// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
		long nID = m_pReferralSubDlg->GetSelectedReferralID(true);
		m_pReferralSubDlg->Update();
		m_pReferralSubDlg->SelectReferralID(nID);
	}


	if(m_btnAllSources.GetCheck())
		m_wndPieGraph.SetTitleText("All Referrals");

	else {
		//DRT 5/16/2006 - PLID 20312 - IMO this just "acts" better if you are allowed to select nothing -- that just having the effect
		//	of choosing all referrals.  I got some other opinions which also agreed.
		// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
		if(m_pReferralSubDlg->GetSelectedReferralID(true) == -1)
			m_wndPieGraph.SetTitleText("All Referrals");

		else
			m_wndPieGraph.SetTitleText(m_pReferralSubDlg->GetSelectedReferralName(true));
	}

	_RecordsetPtr	rs;
	CString			sql, value, docFilter, provIDs, from, to, locFilter, locIDs, referral, strPatFilter, PatFilterIDs, strDateField, strLocationField, strProvField;
	long id;
	int  nCategory, nResp;

	_ConnectionPtr pCon = GetRemoteData();
	CString strPatientTempTable;

	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, strProvField, nCategory, nResp, pCon, strPatientTempTable);
	// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
	id = m_pReferralSubDlg->GetSelectedReferralID(true);

	if(m_btnSelSources.GetCheck())
		referral.Format(" AND (%s)", Descendants(id, "ReferralSourceT.PersonID"));

	if (UseFilter(mftProvider)) {
		docFilter.Format(" AND %s %s) ", strProvField, provIDs);
	}

	if (UseFilter(mftLocation)) {
		locFilter.Format(" AND %s IN %s ", strLocationField, locIDs);
	}

	if (PatFilterIDs != "") {
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);
	}

	CString strBasedOn;
	if(IsDlgButtonChecked(IDC_CITY_STATE)) {
		strBasedOn = "[City] + \', \' + State";
	}
	else if(IsDlgButtonChecked(IDC_FIRST_THREE)) {
		strBasedOn = "SUBSTRING(Zip, 1, 3)";
	}

	CString strFrom, strTo;
	if (!strDateField.IsEmpty() && UseFilter(mftDate)) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	try
	{
		// (c.haag 2008-05-05 09:05) - PLID 29713 - Clear the pie graph
		m_wndPieGraph.Clear();
		
		//PLID 20960: take inquires out
		sql = "SELECT TOP 16 Count(PatientsT.PersonID) AS Referrals, " + strBasedOn + " AS Region "
			"FROM ReferralSourceT INNER JOIN PatientsT ON ReferralSourceT.PersonID = PatientsT.ReferralID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"WHERE (1=1)  AND PatientsT.CurrentStatus <> 4 [From]  [To] [Referral] [Doc] [Loc] [PatFilter] " 
			//TES 2/9/2004: This line seems to be very poorly thought out.
			//+ (!referral.IsEmpty() && !doc.IsEmpty() && !loc.IsEmpty() ? " AND " : "")
			+ " GROUP BY " + strBasedOn + 
			" ORDER BY Count(PatientsT.PersonID) DESC;";

		sql.Replace("[From]", strFrom);
		sql.Replace("[To]", strTo);
		sql.Replace("[Doc]", docFilter);
		sql.Replace("[Loc]", locFilter);
		sql.Replace("[PatFilter]", strPatFilter);
		sql.Replace("[Referral]", referral);		

		rs = CreateRecordsetStd(sql);

		// (c.haag 2008-04-18 12:35) - PLID 29713 - Traverse the recordset adding data to
		// the NexTech pie chart. Notice that the "TOP 16" in the SQL statement ensures we
		// add no more than 16 values, which is the limit of the pie graph.
		while (!rs->eof)
		{
			const long nReferrals = AdoFldLong(rs, "Referrals");
			m_wndPieGraph.Add(nReferrals, FormatString("%d from %s", nReferrals, AdoFldString(rs, "Region")));
			rs->MoveNext();
		}
		m_wndPieGraph.Invalidate(FALSE);
		rs->Close();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	} NxCatchAll ("Could not calculate chart");
}

BOOL CMarketZipDlg::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();

	// (c.haag 2008-04-18 12:38) - PLID 29713 - Removed MSChart objects and added
	// initialization for the NexTech marketing pie graph window
	try {
		m_bActive = false;

		if(m_pReferralSubDlg == NULL) {
			m_pReferralSubDlg = new CReferralSubDlg(this);
			m_pReferralSubDlg->Create(IDD_REFERRAL_SUBDIALOG, this);
			m_pReferralSubDlg->BringWindowToTop();

			CRect rc;
			GetDlgItem(IDC_MKTZIP_REFERRAL_AREA)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_pReferralSubDlg->MoveWindow(rc);

			m_pReferralSubDlg->UseBackgroundColor(0x00C8FFFF);

			//TODO:  PLID 19837 - I have converted the referral tree to use the NxDatalist2.  However the loading is still synchronous.  PLID
			//	20249 is a suggestion to make it synchronous.  However, currently it is very fast (a couple hundred ms for a 700 record list), so
			//	we are not doing so right now.  If we do so in the future, the older referral tree required the referral list be fully loaded before
			//	the loading of the graphs.  We either need to fix that assumption, or wait for the asynchronous requery to finish.  I have
			//	entered this same note in PLID 20249.
		}
	//	m_tree.OnInitialUpdate(true);	//DRT 8/12/2004 - Load tree synchronously, the code in this graph assumes it is loaded.

		m_Go.AutoSet(NXB_MARKET);

		//set the selection to a selected referral source
		CheckDlgButton(IDC_ALL_REFERRALS_CHECK, TRUE);
		CheckDlgButton(IDC_SEL_REFERRALS_CHECK, FALSE);
		m_pReferralSubDlg->EnableAll(FALSE);

		CheckDlgButton(IDC_CITY_STATE, TRUE);
		CheckDlgButton(IDC_FIRST_THREE, FALSE);

		// (c.haag 2008-04-18 12:38) - PLID 29713 - Initialize the pie graph window
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
			ThrowNxException("Failed to create pie graph window!");
		}
		// Make sure the pie graph is in front of the NxColor control
		m_wndPieGraph.SetWindowPos(&wndTop, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);

		//DRT 4/27/2006 - PLID 20312 - This causes NxDialog to re-assess the controls in the dialog, which will include
		//	the new referral tree datalist that is created manually.  From here on, NxDialog will handle resizing and positioning.
		GetControlPositions();

		//Refresh();
		//a.walling 5/22/06 PLID 20695 User will refresh manually
	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;
}

void CMarketZipDlg::SetColor(OLE_COLOR nNewColor)
{
}

void CMarketZipDlg::OnAllReferralsCheck() 
{
	//disable the referral list
	m_pReferralSubDlg->EnableAll(FALSE);
	ResetGraph(true);
	//Refresh(); a.walling 5/22/06 PLID 20695 all refreshes are now manual
}

void CMarketZipDlg::OnSelReferralsCheck() 
{
	//enable the referral list
	m_pReferralSubDlg->EnableAll(TRUE);
	ResetGraph(true);
	//Refresh();
}

void CMarketZipDlg::OnCityState() 
{
	//Refresh();
	ResetGraph(true);
}

void CMarketZipDlg::OnFirstThree() 
{
	//Refresh();
	ResetGraph(true);
}

void CMarketZipDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	//(e.lally 2009-08-28) PLID 35308 - Added try/catch
	try{
		SetType(ZIPGraph);
		SetFilter(mfFirstContactDate, mftDate);
		SetFilter(mfPatientLocation, mftLocation);
		SetFilter(mfPatientProvider, mftProvider);
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);
		//(a.wilson 2011-10-5) PLID 38789 - added force refresh to prevent refreshing when coming back to module.
		if (m_bActive && bForceRefresh) { // manual refresh in toolbar
			// (j.gruber 2006-12-05 11:38) - PLID 22889 - don't update if we are in a report window
			CNxTabView *pView = GetMainFrame()->GetActiveView();
			if (pView) {
				pView->RedrawWindow();
				//(a.wilson 2011-10-5) PLID 38789 - we want to refresh if the toolbar refresh button was pressed.
				OnGo();
			}
					
		}
		else {
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
		}
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CMarketZipDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketZipDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMarketZipDlg::Print(CDC * pDC, CPrintInfo * pInfo)
{
	// (c.haag 2008-04-18 14:30) - PLID 29713 - The legacy code forced landscape mode,
	// so we will do the same
	DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
	ASSERT(pInfoDevMode);
	pInfoDevMode->dmOrientation = DMORIENT_LANDSCAPE;
	GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);

	// (c.haag 2008-04-18 12:33) - PLID 29713 - Rewritten to use the pie graph window.
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

BOOL CMarketZipDlg::DestroyWindow() 
{
	try {
		if(m_pReferralSubDlg != NULL) {
			m_pReferralSubDlg->DestroyWindow();
			delete m_pReferralSubDlg;
			m_pReferralSubDlg = NULL;
		}


	} NxCatchAll("Error in DestroyWindow()");

	return CMarketingDlg::DestroyWindow();
}

LRESULT CMarketZipDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case NXM_REFERRAL_ONSELCHANGED:
		//The user has changed their selection
		
		ResetGraph(true);
		break;

	case NXM_REFERRAL_ONRENAMEFINISHED:
		//The user renamed the selection

		ResetGraph(true);
		break;
	}

	return CMarketingDlg::WindowProc(message, wParam, lParam);
}

void CMarketZipDlg::OnGo() 
{
	CMarketRenderButtonStatus mrbs(this);
	Refresh();
	// (c.haag 2008-04-18 12:42) - PLID 29713 - Show the pie graph window
	m_wndPieGraph.ShowWindow(SW_SHOW);
}

void CMarketZipDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

LRESULT CMarketZipDlg::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	//ResetGraph(true); // (c.haag 2007-03-15 16:59) - PLID 24253 - Already called by the parent
	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters

	return 0;
}

void CMarketZipDlg::ResetGraph(OPTIONAL bool bClear /*= true */, OPTIONAL CString strTitle/* = ""*/, OPTIONAL bool bForceReset /*= false */)
// a.walling PLID 20695 5/25/06 set the graph to a blank state and update the status of any controls
//			set bClear to false if you just want to reset/enable the render button
// (c.haag 2007-03-15 16:53) - PLID 24253 - Added support for forced refreshes (the parent view uses them)
{
	if (!m_bActive && !bForceReset) { // switching from another tab or module.
		return;
	}

	// (c.haag 2008-04-18 12:42) - PLID 29713 - Clear the pie graph window
	if (bClear) {
		m_wndPieGraph.Clear();
		m_wndPieGraph.SetTitleText(strTitle);
		// No need to invalidate because we're about to hide it
	}

	GetDlgItem(IDC_GO)->ShowWindow(SW_SHOW);
	m_wndPieGraph.ShowWindow(SW_HIDE); // (c.haag 2008-04-18 12:43) - PLID 29713 - Hide the pie graph window
}
