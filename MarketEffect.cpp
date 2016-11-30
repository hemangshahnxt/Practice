#include "stdafx.h"

// Effectiveness.cpp : implementation file
//
#include "MarketEffect.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "NxStandard.h"
#include "MsgBox.h"
#include "InternationalUtils.h"
#include "MultiSelectDlg.h"
#include "DocBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_GOTO_PATIENT	42034

//JJ - I am reluctant to make this a global function that overwrites the /
//currency operator, so for the time being I am making this a function specific
//to this tab
static COleCurrency GetPercent(COleCurrency a, COleCurrency b)
{
	COleCurrency cyTemp;
	// (a.walling 2006-10-06 13:20) - PLID 3897 - Have GetPercent return 0 when denominator
	//		is zero rather than crash and burning.
	if (b.m_cur.int64 == 0) {
		cyTemp.SetCurrency(0, 0);
		ASSERT(FALSE);
	}
	else {
		//we can only divide by int64. The 10000 scales the value appropriately
		//so when we factor in the decimal points, the scale is accurate.
		//the 100 is to make it a percentage value.
		cyTemp.m_cur.int64 = 100 * (10000 * a.m_cur.int64) / b.m_cur.int64;
		//This round will keep the right of the decimal to 2 places
		RoundCurrency(cyTemp);
	}
	return cyTemp;
}

/////////////////////////////////////////////////////////////////////////////
// CEffectiveness dialog
//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CEffectiveness::CEffectiveness(CWnd* pParent)
	: CMarketingDlg(CEffectiveness::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEffectiveness)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Marketing_Module/Tabs/effectiveness.htm";
	m_pReferralSubDlg = NULL;
}

CEffectiveness::~CEffectiveness()
{
	// (a.walling 2006-06-14 14:59) - PLID 21023 Free the dialog's resources (DestroyWindow isn't always called)
	DestroyWindow();
}

// (a.walling 2008-04-10 13:01) - PLID 29497 - Edit controls changed to be CNxStatic, member variable prefixes renamed
void CEffectiveness::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEffectiveness)
	DDX_Control(pDX, IDC_GROUP_BY_BILL, m_btnGroupByBill);
	DDX_Control(pDX, IDC_GROUP_BY_PATIENT, m_btnGroupByPatient);
	DDX_Control(pDX, IDC_COLLECTED2, m_lblFeesCollected2);
	DDX_Control(pDX, IDC_COLLECTED, m_lblFeesCollected);
	DDX_Control(pDX, IDC_FEES_BILLED2, m_lblFeesBilled2);
	DDX_Control(pDX, IDC_FEES_BILLED, m_lblFeesBilled);
	DDX_Control(pDX, IDC_PATIENT_COUNT, m_lblPatientCount);
	DDX_Control(pDX, IDC_PATIENT_COST, m_lblPatientCost);
	DDX_Control(pDX, IDC_REFERRAL_COST, m_lblReferralCost);
	DDX_Control(pDX, IDC_REFERRAL_COUNT, m_lblReferralCount);
	DDX_Control(pDX, IDC_EXPENSE, m_lblExpense);
	DDX_Control(pDX, IDC_COLOR1, m_color1);
	DDX_Control(pDX, IDC_COLOR3, m_color3);
	DDX_Control(pDX, IDC_ALL_REFERRALS_CHECK, m_btnAllSources);
	DDX_Control(pDX, IDC_SEL_REFERRALS_CHECK, m_btnSelSources);
	DDX_Control(pDX, IDC_EFF_LABEL9, m_nxstaticEffLabel9);
	DDX_Control(pDX, IDC_MKT_EFFECT_REFERRAL_AREA, m_btnMktEffectReferralArea);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEffectiveness, CMarketingDlg)
	//{{AFX_MSG_MAP(CEffectiveness)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ALL_REFERRALS_CHECK, OnAllReferralsCheck)
	ON_BN_CLICKED(IDC_SEL_REFERRALS_CHECK, OnSelReferralsCheck)
	ON_BN_CLICKED(IDC_GROUP_BY_BILL, OnGroupByBill)
	ON_BN_CLICKED(IDC_GROUP_BY_PATIENT, OnGroupByPatient)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_BN_CLICKED(ID_GOTO_PATIENT, OnGoToPatient)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEffectiveness message handlers

BEGIN_EVENTSINK_MAP(CEffectiveness, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CEffectiveness)
	ON_EVENT(CEffectiveness, IDC_NXDATALISTCTRL, 6 /* RButtonDown */, OnRButtonDownNxdatalistctrl, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEffectiveness, IDC_LIST_BY_BILL, 6 /* RButtonDown */, OnRButtonDownListByBill, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEffectiveness, IDC_NXDATALISTCTRL, 18 /* RequeryFinished */, OnRequeryFinishedPatient, VTS_I2)
	ON_EVENT(CEffectiveness, IDC_LIST_BY_BILL, 18 /* RequeryFinished */, OnRequeryFinishedBill, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CEffectiveness::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();

	try {
		CString			parent,
						item;
		
		m_bActive = false;
		m_bNeedsRefresh = true;
		m_bDataReady = false;
		//m_strSql = "";

		BOOL bGroupByPat = GetRemotePropertyInt("MarketEffectGroupByPat", 1, 0, GetCurrentUserName(), TRUE);
		if (bGroupByPat) {
			CheckDlgButton(IDC_GROUP_BY_PATIENT, TRUE);
			CheckDlgButton(IDC_GROUP_BY_BILL, FALSE);
			GetDlgItem(IDC_NXDATALISTCTRL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_BY_BILL)->ShowWindow(SW_HIDE);
		

		}
		else {
			CheckDlgButton(IDC_GROUP_BY_BILL, TRUE);
			CheckDlgButton(IDC_GROUP_BY_PATIENT, FALSE);
			GetDlgItem(IDC_NXDATALISTCTRL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_BY_BILL)->ShowWindow(SW_SHOW);
		}

		// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
		if(GetMainFrame()->GetActiveView()) {
			GetMainFrame()->GetActiveView()->RedrawWindow();
		}

		//UpdateControls(false);

		//Set up the datalist
		try {

			//bind the datalists
			// (j.jones 2010-08-19 15:27) - PLID 39053 - we ideally want to force using the regular GetRemoteData(),
			// not the snapshot connection, because we alter the command timeout and provide temp tables,
			// (however, currently this does not work in the Bind function... it makes it snapshot anyways,
			// so we still have to forcibly change it later)
			m_list = BindNxDataListCtrl(IDC_NXDATALISTCTRL, GetRemoteData(), false);
			if (m_list == NULL)
				HandleException(NULL, "DataList is Null");
		}NxCatchAll("Could Not Bind DataList");


		//Set up the datalist
		try {

			//bind the datalists
			// (j.jones 2010-08-19 15:27) - PLID 39053 - we need to force using the regular GetRemoteData(),
			// not the snapshot connection, because we alter the command timeout and provide temp tables
			// (however, currently this does not work in the Bind function... it makes it snapshot anyways,
			// so we still have to forcibly change it later)
			m_BillList = BindNxDataListCtrl(IDC_LIST_BY_BILL, GetRemoteData(), false);
			if (m_BillList == NULL)
				HandleException(NULL, "DataList is Null");
		}NxCatchAll("Could Not Bind DataList");


		if(m_pReferralSubDlg == NULL) {
			m_pReferralSubDlg = new CReferralSubDlg(this);
			m_pReferralSubDlg->Create(IDD_REFERRAL_SUBDIALOG, this);
			m_pReferralSubDlg->BringWindowToTop();		//Required to get above the NxColor control

			//attach to the given space
			CRect rc;
			GetDlgItem(IDC_MKT_EFFECT_REFERRAL_AREA)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_pReferralSubDlg->MoveWindow(rc);

			m_pReferralSubDlg->UseBackgroundColor(0x00C8FFFF);

			//TODO:  PLID 19837 - I have converted the referral tree to use the NxDatalist2.  However the loading is still synchronous.  PLID
			//	20249 is a suggestion to make it synchronous.  However, currently it is very fast (a couple hundred ms for a 700 record list), so
			//	we are not doing so right now.  If we do so in the future, the older referral tree required the referral list be fully loaded before
			//	the loading of the graphs.  We either need to fix that assumption, or wait for the asynchronous requery to finish.  I have
			//	entered this same note in PLID 20249.
		}

		m_btnSelSources.SetCheck(FALSE);
		m_btnAllSources.SetCheck(TRUE);

		//DRT 8/12/2004 - PLID 13868 - If we call OnAllReferralsCheck(), 
		//	it will refresh the tab - but the tab is already being refreshed
		//	on entry.  I added a flag that makes it not refresh.
		EnsureReferrals(FALSE);

		//DRT 5/8/2006 - PLID 20328 - This causes NxDialog to re-assess the controls in the dialog, which will include
		//	the new referral tree datalist that is created manually.  From here on, NxDialog will handle resizing and positioning.
		GetControlPositions();

		UpdateView();
		Refresh();

	} NxCatchAll("Error in OnInitDialog()");

	return FALSE;
}


HBRUSH CEffectiveness::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	CRuntimeClass *pRuntime;
	static CBrush br(PaletteColor(COLORREF(0x00C8FFFF)));

	if (pWnd->GetDlgCtrlID() == IDC_EXPENSE ||
		pWnd->GetDlgCtrlID() == IDC_REFERRAL_COUNT ||
		pWnd->GetDlgCtrlID() == IDC_REFERRAL_COST ||
		pWnd->GetDlgCtrlID() == IDC_PATIENT_COUNT ||
		pWnd->GetDlgCtrlID() == IDC_PATIENT_COST ||
		pWnd->GetDlgCtrlID() == IDC_FEES_BILLED ||
		pWnd->GetDlgCtrlID() == IDC_FEES_BILLED2 ||
		pWnd->GetDlgCtrlID() == IDC_COLLECTED ||
		pWnd->GetDlgCtrlID() == IDC_COLLECTED2 ||
		pWnd->GetDlgCtrlID() == IDC_STATIC) 
	{	pRuntime = pWnd->GetRuntimeClass();
		if (strcmp (pRuntime->m_lpszClassName, "CNxColor"))
		{	extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00C8FFFF));
			return (HBRUSH)br;
		}
	}
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CMarketingDlg::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
void CEffectiveness::GetParameters(CString &from, CString &to, CString &prov, CString &loc, CString &strPatFilter, CString &strDateField, CString &strLocationField, long &nCategory, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable)
{
	//generate provider "IN" clause
	prov = GetMainFrame()->m_pDocToolBar->GetProviderString();

	
	strLocationField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftLocation);
	//generate location "IN" clause
	loc = GetMainFrame()->m_pDocToolBar->GetLocationString();

	from = GetMainFrame()->m_pDocToolBar->GetFromDate();
	to = GetMainFrame()->m_pDocToolBar->GetToDate();

	//generate patient coordinator "IN" clause
	strPatFilter = GetMainFrame()->m_pDocToolBar->GetPatientFilterString(pCon, strPatientTempTable);

	strDateField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftDate);

	nCategory = GetMainFrame()->m_pDocToolBar->GetCategory();
}

// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
void CEffectiveness::RefreshCosts(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patFilter, const CString &from, const CString &to, const long &nCategory)
// OUT m_cost
{
	_RecordsetPtr rs;
	CString value, sql;

	SetDlgItemText(IDC_EXPENSE, "Loading...");
	GetDlgItem(IDC_EXPENSE)->RedrawWindow();

	try {
		
		//if not "all dates"
		if (GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
		
			sql = "SELECT SUM(CostSubQ.Cost) AS Cost FROM "
			"(SELECT Amount AS Cost, ReferralSource "
			"FROM MarketingCostsT "
			"WHERE EffectiveFrom >= '[from]' AND EffectiveFrom < DATEADD(day,1,'[to]') AND EffectiveTo >= '[from]' AND EffectiveTo < DATEADD(day,1,'[to]') "
			"[referrals] "

			"UNION ALL "

			"SELECT (Amount * DATEDIFF(day, DATEADD(day,1,'[to]'), EffectiveFrom)) "
			"/ DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
			"FROM MarketingCostsT "
			"WHERE EffectiveFrom < DATEADD(day,1,'[to]') AND EffectiveFrom > '[from]' AND EffectiveTo >= DATEADD(day,1,'[to]') "
			"[referrals] "

			"UNION ALL "

			"SELECT (Amount * DATEDIFF(day, DATEADD(day,1,'[to]'), '[from]')) "
			"/ DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
			"FROM MarketingCostsT "
			"WHERE EffectiveFrom <= '[from]' AND EffectiveTo >= DATEADD(day,1,'[to]') "
			"AND (EffectiveFrom <> '[from]' OR EffectiveTo <> DATEADD(day,1,'[to]')) "
			"[referrals] "

			"UNION ALL "

			"SELECT (Amount * DATEDIFF(day, EffectiveTo, '[from]')) "
			"/ DATEDIFF (day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
			"FROM MarketingCostsT "
			"WHERE EffectiveFrom < '[from]' AND EffectiveTo > '[from]' AND EffectiveTo < DATEADD(day,1,'[to]')"
			"[referrals] "

			") AS CostSubQ ";
		}
		else {

			//all dates

			sql = "SELECT SUM(CostSubQ.Cost) AS Cost FROM "
			"(SELECT Amount AS Cost, ReferralSource "
			"FROM MarketingCostsT WHERE (1=1) "
			"[referrals] "
			") AS CostSubQ ";

		}

		CString strFrom = GetMainFrame()->m_pDocToolBar->GetFromDate();
		CString strTo = GetMainFrame()->m_pDocToolBar->GetToDate();
		
		sql.Replace("[referrals]", source);
		sql.Replace("[from]", strFrom);
		sql.Replace("[to]", strTo);
		// (j.jones 2010-08-19 15:30) - PLID 39053 - use the passed-in connection
		rs = CreateRecordset(pCon, sql);
		if (rs->eof || rs->Fields->GetItem("Cost")->Value.vt == VT_NULL)
			m_cost = COleCurrency(0,0);
		else
			m_cost = AdoFldCurrency(rs, "Cost");
		rs->Close();
	} NxCatchAll ("Could not calculate costs");

	RoundCurrency(m_cost);
	value = FormatCurrencyForInterface(m_cost);

	SetDlgItemText (IDC_EXPENSE, value);
	GetDlgItem(IDC_EXPENSE)->RedrawWindow();
}

// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
void CEffectiveness::RefreshReferrals(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patFilter, const CString &from, const CString &to, const long &nCategory)
{
	_RecordsetPtr rs;
	CString value, sql, doc1;

	SetDlgItemText(IDC_REFERRAL_COST, "Loading...");
	SetDlgItemText(IDC_REFERRAL_COUNT, "Loading...");
	GetDlgItem(IDC_REFERRAL_COST)->RedrawWindow();
	GetDlgItem(IDC_REFERRAL_COUNT)->RedrawWindow();

	if (doc != "")
		doc1.Format(" AND PatientsT.MainPhysician IN %s ",doc);

	CString newSource = source;
	newSource.Replace("ReferralSource","ReferralID");

	sql = "SELECT COUNT(ReferralID) AS Referrals "
		 "FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID ";

	sql += "WHERE (1=1) "
		+ from
		+ to
		+ doc1 
		+ loc
		+ patFilter
		+ newSource;

	try {
		// (j.jones 2010-08-19 15:30) - PLID 39053 - use the passed-in connection
		rs = CreateRecordsetStd(pCon, sql);
		if (!rs->eof)
			m_referral_count = rs->Fields->GetItem("Referrals")->Value.lVal;
		else
			m_referral_count = 0;
		rs->Close();
	}NxCatchAll("Could not calculate referrals");

	SetDlgItemInt (IDC_REFERRAL_COUNT, m_referral_count);
	if (m_referral_count) {
		COleCurrency refCost;
		
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly state the type of the denominator
		refCost = m_cost / long(m_referral_count);

		RoundCurrency(refCost);

		value = FormatCurrencyForInterface(refCost);

		SetDlgItemText (IDC_REFERRAL_COST, value);
	}
	else {
		SetDlgItemText (IDC_REFERRAL_COST, FormatCurrencyForInterface(COleCurrency(0,0)));
	}

	GetDlgItem(IDC_REFERRAL_COST)->RedrawWindow();
	GetDlgItem(IDC_REFERRAL_COUNT)->RedrawWindow();
}

// (a.walling 2006-06-06 16:31) - PLID 20931 this function uses only member variables now and creates no recordsets
void CEffectiveness::RefreshPatients() // IN m_cost, m_patient_count
{
/*
	_RecordsetPtr rs;

	CString sql;
	sql.Format("SELECT Count(PatientID) AS Patients FROM %s",GetListBoxSql(source,doc,loc,patFilter,from,to,nCategory));

	try
	{	rs = CreateRecordsetStd(sql);
		if (!rs->eof)
			m_patient_count = rs->Fields->GetItem("Patients")->Value.lVal;
		else
			m_patient_count = 0;
		rs->Close();
	}NxCatchAll("Could not calculate patients");
*/
	// (a.walling 2006-06-14 11:16) - PLID 21007 The above is just as slow as just running the Patients query. we calculate this
	// based on the datalists now.

	CString value;
	
	if (m_patient_count) {
		COleCurrency patCost;
		
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly state the type of the denominator
		patCost = m_cost / long(m_patient_count);

		RoundCurrency(patCost);

		value = FormatCurrencyForInterface(patCost);

		SetDlgItemText (IDC_PATIENT_COST, value);
		SetDlgItemInt (IDC_PATIENT_COUNT, m_patient_count);
	}
	else {
		SetDlgItemText (IDC_PATIENT_COST, FormatCurrencyForInterface(COleCurrency(0,0)));
		SetDlgItemInt (IDC_PATIENT_COUNT, 0);
	}
		
	GetDlgItem(IDC_PATIENT_COUNT)->RedrawWindow();
	GetDlgItem(IDC_PATIENT_COST)->RedrawWindow();

	m_bDataReady = true;
}

// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
void CEffectiveness::RefreshListBox(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patFilter, const CString &from, const CString &to, const long &nCategory)
// (a.walling 2006-06-06 16:50) - PLID 20931 Calls RefreshPatients
{
	if (m_bNeedsRefresh) {
		SetDlgItemText (IDC_PATIENT_COST, "Loading...");
		SetDlgItemText (IDC_PATIENT_COUNT, "Loading...");
		GetDlgItem(IDC_PATIENT_COST)->RedrawWindow();
		GetDlgItem(IDC_PATIENT_COUNT)->RedrawWindow();
	}
	
	CString sql = GetListBoxSql(source,doc,loc,patFilter,from,to,nCategory);
	//m_strSql = sql;
	
	if (IsDlgButtonChecked(IDC_GROUP_BY_PATIENT) && ( (m_list->GetRowCount() == 0) || (m_bNeedsRefresh) )) {

		// (j.jones 2010-08-19 15:30) - PLID 39053 - use the passed-in connection
		m_list->PutAdoConnection(pCon);
		m_list->FromClause = (LPCSTR)sql;
		m_list->Requery();
		//(e.lally 2005-09-30) PLID 16201 - We need to wait for the requery to finish, you can get a crash when
			//quickly clicking the year down button on the date otherwise.
		//a.walling PLID 20695 6/5/06 This happened because the date control caused OnSelChanged to fire an UpdateView to the dialog.
		//		However, since now UpdateView merely refreshes filters and not the data, that is done via a PostMessage NXM_MARKET_READY,
		//		I am not sure if this bug is still relevant or not.
		//m_list->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);

		if (m_bNeedsRefresh)
			m_BillList->Clear();
	}
	else if (IsDlgButtonChecked(IDC_GROUP_BY_BILL) && ( (m_BillList->GetRowCount() == 0) || (m_bNeedsRefresh) )) {
			
		// (j.jones 2010-08-19 15:30) - PLID 39053 - use the passed-in connection
		m_BillList->PutAdoConnection(pCon);
		m_BillList->FromClause = (LPCSTR)sql;
		m_BillList->Requery();
		//(e.lally 2005-09-30) PLID 16201 - We need to wait for the requery to finish, you can get a crash when
			//quickly clicking the year down button on the date otherwise.
		//a.walling PLID 20695 6/5/06 See above comment
		//m_BillList->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
		
		if (m_bNeedsRefresh)
			m_list->Clear();
	}

}

// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
void CEffectiveness::RefreshCharges(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patFilter, const CString &from, const CString &to, const long &nCategory)
{
	CString sql, value, doc1, doc2, category;
	_RecordsetPtr rs;

	SetDlgItemText(IDC_FEES_BILLED, "Loading...");
	GetDlgItem(IDC_FEES_BILLED)->RedrawWindow();

	if (doc != "")
	{	doc1.Format(" AND ChargesT.DoctorsProviders IN %s ", doc);
		doc2.Format(" AND PaymentsT.ProviderID IN %s ", doc);
	}

	if(nCategory != -1) {
		category.Format(" AND ServiceT.Category = %li ", nCategory);
	}

	// (d.thompson 2010-12-29) - PLID 31929 - This has apparently been calculating the Fees Billed wrong since early 2004 and noone noticed...  Reworked
	//	the query to calculate properly.
	sql = "SELECT (CASE WHEN ChargeQ.Charges Is Null THEN 0 ELSE ChargeQ.Charges END) + "
	"(CASE WHEN AdjQ.Charges Is Null THEN 0 ELSE AdjQ.Charges END) AS Charges "
	"FROM ( "
	"SELECT "
	"Sum(ChargeRespT.Amount) AS Charges "
	"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
	"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID INNER JOIN ChargesT ON ChargesT.BillID = BillsT.ID "
	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
	"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
	"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "

	"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Deleted = 0 "
	"[from] [to] [doc1] [loc] [patcoord] [referrals] [category] "
	") AS ChargeQ CROSS JOIN ("

	"SELECT Sum(AppliesT.Amount) AS Charges "
	
	"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
	"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID INNER JOIN ChargesT ON ChargesT.BillID = BillsT.ID "
	"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN AppliesT ON ChargesT.ID = AppliesT.SourceID "
	"INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
	"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "

	"WHERE LineItemT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Type = 2 "
	"[from] [to] [doc2] [loc] [patcoord] [referrals] [category] "

") AS AdjQ";

	CString newSource = source;
	newSource.Replace("ReferralSource","PatientsT.ReferralID");

	sql.Replace("[referrals]", newSource);
	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[doc1]", doc1);
	sql.Replace("[doc2]", doc2);
	sql.Replace("[loc]", loc);
	sql.Replace("[patcoord]", patFilter);
	sql.Replace("[category]", category);

	try
	{
		// (j.jones 2010-08-19 15:30) - PLID 39053 - use the passed-in connection
		rs = CreateRecordset(pCon, (LPCSTR)sql);
		if (!rs->eof)
			m_total_billed = AdoFldCurrency(rs, "Charges");
		else
			m_total_billed = COleCurrency(0,0);
		rs->Close();
	}NxCatchAll("Could not calculate total charges");

	value = FormatCurrencyForInterface(m_total_billed);
	SetDlgItemText(IDC_FEES_BILLED, value);
	if (m_cost != COleCurrency(0,0))
	{
		COleCurrency cyTemp;
		//the ROI is the (amount made - cost) / cost
		cyTemp = GetPercent(m_total_billed - m_cost, m_cost);
		value = FormatCurrencyForInterface(cyTemp,FALSE);
		SetDlgItemText (IDC_FEES_BILLED2, value + '%');
	}
	else SetDlgItemText (IDC_FEES_BILLED2, "no cost");

	GetDlgItem(IDC_FEES_BILLED)->RedrawWindow();
	GetDlgItem(IDC_FEES_BILLED2)->RedrawWindow();
}

// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
void CEffectiveness::RefreshPayments(ADODB::_ConnectionPtr pCon, const CString &source, const CString &doc, const CString &loc, const CString &patFilter, const CString &from, const CString &to, const long &nCategory)
{
	CString			sql, 
					value,
					doc1, category;
	_RecordsetPtr	rs;

	SetDlgItemText(IDC_COLLECTED, "Loading...");
	GetDlgItem(IDC_COLLECTED)->RedrawWindow();

	if(nCategory != -1)
		category.Format(" AND ChargesT.ID Is Not NULL AND ServiceT.Category = %li", nCategory);

	if (doc != "")
		doc1.Format(" AND PaymentsT.ProviderID IN %s ", doc);

	if(nCategory != -1) {
		//Filtering on category, only care about applied amount
		sql = "SELECT Sum(Round(Convert(money,AppliesT.Amount),2)) AS Payments "
			"FROM LineItemT "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
			"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
			"WHERE (LineItemT.Type = 1 OR LineItemT.Type = 3) AND LineItemT.Deleted = 0 "
			"[from] [to] [referrals] [category] [doc1] [patcoord] [loc]";
	}
	else {
		//Not filtering on category, therefore don't care about applies.
		sql = "SELECT Sum(Round(Convert(money,LineItemT.Amount),2)) AS Payments "
			"FROM LineItemT "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE (LineItemT.Type = 1 OR LineItemT.Type = 3) AND LineItemT.Deleted = 0 "
			"[from] [to] [referrals] [category] [doc1] [patcoord] [loc]";
	}

	CString newSource = source;
	newSource.Replace("ReferralSource","ReferralID");

	sql.Replace("[referrals]", newSource);
	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[doc1]", doc1);
	sql.Replace("[loc]", loc);
	sql.Replace("[patcoord]", patFilter);
	sql.Replace("[category]", category);
	
	try
	{
		// (j.jones 2010-08-19 15:30) - PLID 39053 - use the passed-in connection
		rs = CreateRecordset(pCon, (LPCSTR)sql);
		if (!rs->eof)
			m_total_collected = VarCurrency(rs->Fields->GetItem("Payments")->Value,COleCurrency(0,0));
		else m_total_collected = COleCurrency(0,0);
		rs->Close();
	}NxCatchAll("Could not calculate total payments");

	value = FormatCurrencyForInterface(m_total_collected);
	SetDlgItemText (IDC_COLLECTED, value);
	if (m_cost != COleCurrency(0,0))
	{
		COleCurrency cyTemp;
		//the ROI is the (amount made - cost) / cost
		cyTemp = GetPercent(m_total_collected - m_cost, m_cost);
		value = FormatCurrencyForInterface(cyTemp,FALSE);
		SetDlgItemText (IDC_COLLECTED2, value + '%');
	}
	else SetDlgItemText (IDC_COLLECTED2, "no cost");

	GetDlgItem(IDC_COLLECTED)->RedrawWindow();
	GetDlgItem(IDC_COLLECTED2)->RedrawWindow();
}

// (j.jones 2010-08-19 15:25) - PLID 39493 - pass a connection in to all the Refresh sub-functions
void CEffectiveness::Refresh()
{
	try {
		if (m_bNeedsRefresh)
		{
			// (a.walling 2006-06-14 11:35) - PLID 21007 The return value is nonzero if the window was previously disabled. 
			if (GetDlgItem(IDC_GROUP_BY_PATIENT)->EnableWindow(FALSE) != 0) {
				return; // this means that the datalist requery has not finished.
			}
			// (a.walling 2006-07-18 14:47) - PLID 20695 Removed the redrawwindow here.
			UpdateControls(false);

			//If any table checkers have been fired, this will refresh the list.  Save the current selection.
			{
				// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
				long nID = m_pReferralSubDlg->GetSelectedReferralID(true);
				m_pReferralSubDlg->Update(true);
				m_pReferralSubDlg->SelectReferralID(nID);
			}
		}

		CWaitCursor wait;
		
				
		/*DRT 8/12/2004 - PLID 13868 - This code is causing the Refresh to call
			OnSelChosen... which in turns calls refresh!  I moved the code to
			the EnsureReferrals function, which updates the enabled state
			of the referral box, and can keep up with this as well
		m_referralTree.Update();
		if (!m_referralTree.GetSelectedItem())
			m_referralTree.SelectItem(m_referralTree.GetFirstVisibleItem());
		*/

		CString			from,
						to,
						source,
						docFilter, provIDs, 
						locFilter, locIDs,
						strPatFilter, PatFilterIDs, 
						strDateField, strLocationField;
		long			id, nCategoryID;

		_ConnectionPtr pCon = GetRemoteData();
		// (j.jones 2010-07-19 17:49) - PLID 39053 - this temp table will not be dropped until the connection is broken
		CString strPatientTempTable;

		//setup parameters that will be used in the query
		GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, nCategoryID, pCon, strPatientTempTable);

		if (PatFilterIDs != "") //user personT location instead of scheduler location
			strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", strDateField, from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
		}
		// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
		if (m_btnSelSources.GetCheck()) {
			id = m_pReferralSubDlg->GetSelectedReferralID(true);
			if (id != -1) {
				source = "AND ReferralSource " + Descendants(id, "");
			}
		}
		
		if (UseFilter(mftProvider))
			docFilter.Format("%s", provIDs);

		if (UseFilter(mftLocation))
			locFilter.Format(" AND PersonT.Location IN %s ", locIDs);

		//set the timeout
		long nOldTimeout;
		nOldTimeout = pCon->CommandTimeout;

		if (nOldTimeout < 600) {
			pCon->CommandTimeout = 600;
		}

		if (m_bNeedsRefresh) {
			
			m_bDataReady = false;

			RefreshCosts		(pCon, source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID); // sets m_cost

			RefreshReferrals	(pCon, source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID);

	//		RefreshPatients		(source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID); // needs # of patients
			// (a.walling 2006-06-06 16:29) - PLID 20931 We calculate the cost here and it is one of the longest queries.
			//		instead we can get the # of patients from the datalist when it finished querying (the same query), and then call RefreshPatients.

			RefreshCharges		(pCon, source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID);

			RefreshPayments		(pCon, source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID);

		}

		RefreshListBox		(pCon, source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID);
		// (a.walling 2006-06-06 16:37) - PLID 20931 this is now asynchronous and calls RefreshPatients when complete

		if (m_bNeedsRefresh) {
			m_bNeedsRefresh = false;
		}

		pCon->CommandTimeout = nOldTimeout;

	}NxCatchAll("Error In Refresh()");
}

void CEffectiveness::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	//(e.lally 2009-08-28) PLID 35308 - Added try/catch
	try{
		UpdateFilters();
		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);
		if (m_bActive) { // this way it won't refresh if we are just flipping among tabs or windows

			// (j.gruber 2006-12-05 11:38) - PLID 22889 - don't update if we are in a report window
				CNxTabView *pView = GetMainFrame()->GetActiveView();
				if (pView) {
					pView->RedrawWindow();
				}
				else {

					//if we aren't in the view, we don't need to refresh
					return;
				}
			m_pReferralSubDlg->RedrawWindow();
				
			m_bNeedsRefresh = true;
			Refresh();
		}
		else {
			m_mfiFilterInfo.SetFilters(); // a.walling PLID 20928 6/5/06 set the appt.loc.prov filters in the docbar
		}
	}NxCatchAll(__FUNCTION__);
}

void CEffectiveness::UpdateFilters() {
	// PLID 20695: a.walling 6/05/06 moved this to a seperate function from UpdateView.

	//PLID 16800: this needs to be here so that the provider datalist requeries itself
	GetMainFrame()->m_pDocToolBar->SetFilter(mfDependantProvider, mftProvider);
	CDWordArray dwDateAllowedFilters, dwLocAllowedFilters, dwProvAllowedFilters;
	dwDateAllowedFilters.Add(mfFirstContactDate);
	dwLocAllowedFilters.Add(mfPatientLocation);
	dwProvAllowedFilters.Add(mfDependantProvider);
	GetMainFrame()->m_pDocToolBar->SetType(-1);
	GetMainFrame()->m_pDocToolBar->SetAllowedFilters(dwDateAllowedFilters, mftDate);
	GetMainFrame()->m_pDocToolBar->SetAllowedFilters(dwLocAllowedFilters, mftLocation);
	GetMainFrame()->m_pDocToolBar->SetAllowedFilters(dwProvAllowedFilters, mftProvider);
	GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_RESP);
}

void CEffectiveness::OnAllReferralsCheck() 
{
	EnsureReferrals();
}

void CEffectiveness::EnsureReferrals(BOOL bRefresh /*= TRUE*/)
{
	//disable the referral list
	if(m_btnAllSources.GetCheck()) {
		m_pReferralSubDlg->EnableAll(FALSE);
		// (b.eyers 2015-11-04) - PLID 67536 - if we are changing back to all sources, the dialog needs to refresh the data
		//bRefresh = FALSE;
	}
	else {
		//enable the referral list
		m_pReferralSubDlg->EnableAll(TRUE);

		// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
		if (m_pReferralSubDlg->GetSelectedReferralID(true) == -1) {
			bRefresh = FALSE; // don't refresh if there is no item selected (first time going to selected items)
		}
	}

	if(bRefresh) {
		m_bNeedsRefresh = true;
		Refresh();
	}
}

void CEffectiveness::OnSelReferralsCheck() 
{
	EnsureReferrals();
}

void CEffectiveness::OnRButtonDownNxdatalistctrl(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_list->CurSel = nRow;

	if(nRow==-1)
		return;

	try {

		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();
	
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_GOTO_PATIENT, "Go To Patient");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	
	}NxCatchAll("Error in OnRButtonDownNxdatalistctrl");
}

void CEffectiveness::OnGoToPatient()
{

	NXDATALISTLib::_DNxDataListPtr pList;

	if (IsDlgButtonChecked(IDC_GROUP_BY_BILL)) {
		pList = m_BillList;
	}
	else if (IsDlgButtonChecked(IDC_GROUP_BY_PATIENT)) {
		pList = m_list;
	}

	if(pList->GetCurSel()==-1)
		return;

	try {

		long nPatientID = VarLong(pList->GetValue(pList->GetCurSel(),0));

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView)
						pView->UpdateView();
				}

			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR - MarketEffect.cpp: Cannot Open Mainframe");
			}//end else pMainFrame
		}//end if nPatientID

	}NxCatchAll("Error in OnGoToPatient");
}

CString CEffectiveness::GetListBoxSql(const CString &source, const CString &doc, const CString &loc, const CString &patFilter, const CString &from, const CString &to, const long &nCategory, bool bForceByPatient/* = false*/)
{

	CString sql, doc1, doc2, category;

	if (doc != "") {
		doc1.Format(" AND ChargesT.DoctorsProviders IN %s ", doc);
		doc2.Format(" AND PaymentsT.ProviderID IN %s ", doc);
	}

	if(nCategory != -1) {
		category.Format(" AND ServiceT.Category = %li ", nCategory);
	}

	if (IsDlgButtonChecked(IDC_GROUP_BY_PATIENT) || bForceByPatient) {
		// (a.walling 2008-05-02 16:39) - PLID 29889 - Charges can exceed the lineitem amount by being manually edited; modified to use the chargerespt if available to be in sync with the billing module
		//TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
		sql = "(SELECT ChargeAdjQ.PatientID, ChargeAdjQ.FullName, LastProcedureQ.LastProcedureDate, ChargeAdjQ.Charges, '' AS Description FROM "
				"(SELECT (CASE WHEN ChargeQ.ID Is Null THEN AdjQ.ID ELSE ChargeQ.ID END) AS PatientID, "
				"(CASE WHEN ChargeQ.ID Is Null THEN AdjQ.FullName ELSE ChargeQ.FullName END) AS FullName, "
				"(CASE WHEN ChargeQ.ID Is Null THEN 0 ELSE ChargeQ.Charges END) "
				"+ (CASE WHEN AdjQ.ID Is Null THEN 0 ELSE AdjQ.Charges END) AS Charges "

				"FROM ( "
					"SELECT PersonT.ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS FullName, "
					"Sum(CASE WHEN ChargeRespT.ID IS NULL THEN LineItemT.Amount ELSE ChargeRespT.Amount END) AS Charges "
					"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID INNER JOIN ChargesT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"LEFT JOIN ChargeRespT ON ChargeRespT.ChargeID = ChargesT.ID "

					"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Deleted = 0 "
					"[from] [to] [doc1] [loc] [patcoord] [category] [referrals] "

					"GROUP BY PersonT.ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] "
				") AS ChargeQ FULL JOIN ("

					"SELECT PersonT.ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS FullName, "
					"Sum(AppliesT.Amount) AS Charges "
					"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID INNER JOIN ChargesT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN AppliesT ON ChargesT.ID = AppliesT.SourceID "
					"INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "

					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 AND BillsT.EntryType = 1 "
					"[from] [to] [doc1] [loc] [patcoord] [category] [referrals] "

					"GROUP BY PersonT.ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] "
				") AS AdjQ ON ChargeQ.ID = AdjQ.ID "
				"WHERE ChargeQ.Charges > 0 OR AdjQ.Charges > 0) AS ChargeAdjQ "

				"LEFT JOIN (SELECT PatientID, Max(Date) AS LastProcedureDate FROM AppointmentsT "
					"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"WHERE (AptTypeT.Category = 4 "		//Surgery
						"OR AptTypeT.Category = 3) "		//Minor Procedure
						"AND AppointmentsT.ShowState <> 3 " //3 = no show
						"AND AppointmentsT.Status <> 4 "	//4 = cancelled
						"AND Date <= GetDate() "
					"GROUP BY PatientID "
				") AS LastProcedureQ ON ChargeAdjQ.PatientID = LastProcedureQ.PatientID "

			") AS ChargesAndAdjustmentsQ";
	}
	else if (IsDlgButtonChecked(IDC_GROUP_BY_BILL) && !bForceByPatient) {

		sql = " (SELECT  "
					" (CASE WHEN ChargeQ.BillID Is Null THEN AdjQ.FullName ELSE ChargeQ.FullName END) AS FullName,  "
					" (CASE WHEN ChargeQ.BillID Is Null THEN AdjQ.Date ELSE ChargeQ.Date END) AS Date,  "
					" (CASE WHEN ChargeQ.BillID Is Null THEN AdjQ.Date ELSE ChargeQ.Date END) AS LastProcedureDate,  " // not really, this is for the report
					" (CASE WHEN ChargeQ.BillID Is Null THEN AdjQ.Description ELSE ChargeQ.Description END) AS Description,  "
					" (CASE WHEN ChargeQ.BillID Is Null THEN 0 ELSE ChargeQ.Charges END)  " 
					" + (CASE WHEN AdjQ.BillID Is Null THEN 0 ELSE AdjQ.Charges END) AS Charges,  "
					" (CASE WHEN ChargeQ.BillID Is Null THEN AdjQ.PatientID ELSE ChargeQ.PatientID END) AS PatientID "
					" FROM ("
					" SELECT PersonT.ID AS PatientID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS FullName, "
					"dbo.GetBillTotal(BillsT.ID) AS Charges, BillsT.Date as Date, BillsT.Description as Description, BillsT.ID AS BillID "
					"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID INNER JOIN ChargesT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "

					"WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND LineItemT.Deleted = 0 "
					"[from] [to] [doc1] [loc] [patcoord] [category] [referrals] "

					"GROUP BY PersonT.ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle], BillsT.ID, BillsT.Description, BillsT.Date "
				") AS ChargeQ FULL JOIN ("

					"SELECT PersonT.ID AS PatientID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS FullName, "
					"Sum(AppliesT.Amount) AS Charges, BillsT.Date as Date, BillsT.Description as Description, BillsT.Id AS BillID  "
					"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"INNER JOIN BillsT ON PersonT.ID = BillsT.PatientID INNER JOIN ChargesT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID INNER JOIN AppliesT ON ChargesT.ID = AppliesT.SourceID "
					"INNER JOIN PaymentsT ON AppliesT.DestID = PaymentsT.ID "
					"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "

					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 2 AND BillsT.EntryType = 1 "
					"[from] [to] [doc1] [loc] [patcoord] [category] [referrals] "

					"GROUP BY PersonT.ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle], BillsT.ID, BillsT.Description, BillsT.Date "
				") AS AdjQ ON ChargeQ.BillID = AdjQ.BillID "
				"WHERE ChargeQ.Charges > 0 OR AdjQ.Charges > 0) AS ChargesAndAdjustmentsQ";
	}
		
	CString newSource = source;
	newSource.Replace("ReferralSource","PatientsT.ReferralID");

	sql.Replace("[referrals]", newSource);
	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[doc1]", doc1);
	sql.Replace("[doc2]", doc2);
	sql.Replace("[loc]", loc);
	sql.Replace("[patcoord]", patFilter);
	sql.Replace("[category]", category);

	return sql;
}

void CEffectiveness::OnRButtonDownListByBill(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_BillList->CurSel = nRow;

	if(nRow==-1)
		return;

	try {

		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();		
	
		pMenu->InsertMenu(-1, MF_BYPOSITION, ID_GOTO_PATIENT, "Go To Patient");

		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	
	}NxCatchAll("Error in OnRButtonDownListByBill");
	
}

void CEffectiveness::OnGroupByBill() 
{
	UpdateFilters();
	SetRemotePropertyInt("MarketEffectGroupByPat", 0, 0, GetCurrentUserName());

	GetDlgItem(IDC_NXDATALISTCTRL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LIST_BY_BILL)->ShowWindow(SW_SHOW);

	Refresh();
}

void CEffectiveness::OnGroupByPatient() 
{
	UpdateFilters();
	SetRemotePropertyInt("MarketEffectGroupByPat", 1, 0, GetCurrentUserName());

	GetDlgItem(IDC_NXDATALISTCTRL)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_LIST_BY_BILL)->ShowWindow(SW_HIDE);

	Refresh();
}

BOOL CEffectiveness::DestroyWindow() 
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

LRESULT CEffectiveness::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case NXM_REFERRAL_ONSELCHANGED:
		//The user has changed their selection
		EnsureReferrals();
		return 0;
		break;

	case NXM_REFERRAL_ONRENAMEFINISHED:
		//The user renamed the selection
		UpdateFilters();
		break;


	}

	return CMarketingDlg::WindowProc(message, wParam, lParam);
}

void CEffectiveness::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	m_bActive = bShow; // this is called after UpdateView

	CMarketingDlg::OnShowWindow(bShow, nStatus);
}

// a.walling PLID 20695 6/05/06 DocBar Filters changed, so refresh
LRESULT CEffectiveness::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	m_mfiFilterInfo.Add(MarketFilter(wParam), MarketFilterType(lParam)); // a.walling PLID 20928 add the appt/prov/loc filters
	
	m_bNeedsRefresh = true;
	Refresh();

	return 0;
}

void CEffectiveness::OnRequeryFinishedPatient(short nFlags) 
{
	//GetDlgItem(IDC_NXDATALISTCTRL)->EnableWindow(true);
	
	m_patient_count = m_list->GetRowCount();
	RefreshPatients();
	UpdateControls(true);
}

void CEffectiveness::OnRequeryFinishedBill(short nFlags) 
{
	//GetDlgItem(IDC_LIST_BY_BILL)->EnableWindow(true);

	SetDlgItemText (IDC_PATIENT_COST, "Calculating...");
	SetDlgItemText (IDC_PATIENT_COUNT, "Calculating...");
	GetDlgItem(IDC_PATIENT_COST)->RedrawWindow();
	GetDlgItem(IDC_PATIENT_COUNT)->RedrawWindow();

	
	// (a.walling 2006-06-14 13:48) - PLID 21007
	// We create a map so we can group the patient IDs without bothering SQL Server. We use a map since we aren't guaranteed
	// any order of values in the datalist. CMap is limited to a single key, so only unique keys will occupy the hashtable.
	// In the end, the count of PatientIDs is just the count of values in the map.
	try {
		CMap<long, long, long, long> mapPatientIDs;
		long ptr = m_BillList->GetFirstRowEnum();
		LPDISPATCH pDisp = NULL;

		while (ptr) {
			m_BillList->GetNextRowEnum(&ptr, &pDisp);

			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			mapPatientIDs.SetAt(VarLong(pRow->Value[0]), 0);

			pDisp->Release();
		}

		
		m_patient_count = mapPatientIDs.GetCount();
		RefreshPatients();
		UpdateControls(true);
	} NxCatchAll("Error traversing datalist");
}

void CEffectiveness::UpdateControls(IN bool bEnable)  //<- Enables or disables the dialog's controls
{
	GetDlgItem(IDC_LIST_BY_BILL)->EnableWindow(bEnable);
	GetDlgItem(IDC_NXDATALISTCTRL)->EnableWindow(bEnable);
	GetDlgItem(IDC_GROUP_BY_PATIENT)->EnableWindow(bEnable);
	GetDlgItem(IDC_GROUP_BY_BILL)->EnableWindow(bEnable);
	GetDlgItem(IDC_ALL_REFERRALS_CHECK)->EnableWindow(bEnable);
	GetDlgItem(IDC_SEL_REFERRALS_CHECK)->EnableWindow(bEnable);

	if (IsDlgButtonChecked(IDC_GROUP_BY_BILL))
		GetDlgItem(IDC_LIST_BY_BILL)->RedrawWindow();
	if (IsDlgButtonChecked(IDC_GROUP_BY_PATIENT))
		GetDlgItem(IDC_NXDATALISTCTRL)->RedrawWindow();
	GetDlgItem(IDC_GROUP_BY_PATIENT)->RedrawWindow();
	GetDlgItem(IDC_GROUP_BY_BILL)->RedrawWindow();
	GetDlgItem(IDC_ALL_REFERRALS_CHECK)->RedrawWindow();
	GetDlgItem(IDC_SEL_REFERRALS_CHECK)->RedrawWindow();

	if (!bEnable) {
		if (m_pReferralSubDlg) {
			m_pReferralSubDlg->EnableAll(FALSE);
			m_pReferralSubDlg->RedrawWindow();
		}
		SetDlgItemText (IDC_EXPENSE, "");
		SetDlgItemText (IDC_REFERRAL_COST, "");
		SetDlgItemText (IDC_REFERRAL_COUNT, "");
		SetDlgItemText (IDC_PATIENT_COUNT, "");
		SetDlgItemText (IDC_PATIENT_COST, "");
		SetDlgItemText (IDC_FEES_BILLED, "");
		SetDlgItemText (IDC_FEES_BILLED2, "");
		SetDlgItemText (IDC_COLLECTED, "");
		SetDlgItemText (IDC_COLLECTED2, "");
	}
	else {
		if(m_btnSelSources.GetCheck()) {
			if (m_pReferralSubDlg)
				m_pReferralSubDlg->EnableAll(TRUE);
		}
	}
}



bool CEffectiveness::IsRequerying() // (a.walling 2006-08-08 17:39) - PLID 3897 - Is the tab finished loading?
{
	return !m_bDataReady;
}

// (j.jones 2010-08-23 12:47) - PLID 39053 - require a connection pointer
CString CEffectiveness::GetReportSQL(ADODB::_ConnectionPtr pCon) // generate the report sql
{
	// (j.jones 2010-08-23 12:46) - PLID 39053 - we have to re-generate the sql here

	CString			from,
					to,
					source,
					docFilter, provIDs, 
					locFilter, locIDs,
					strPatFilter, PatFilterIDs, 
					strDateField, strLocationField;
	long			id, nCategoryID;

	CString strPatientTempTable;

	//setup parameters that will be used in the query
	GetParameters(from, to, provIDs, locIDs, PatFilterIDs, strDateField, strLocationField, nCategoryID, pCon, strPatientTempTable);

	if (PatFilterIDs != "") //user personT location instead of scheduler location
		strPatFilter.Format(" AND PatientsT.PersonID IN %s ", PatFilterIDs);

	CString strFrom, strTo;
	if (!strDateField.IsEmpty() && UseFilter(mftDate)) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s < DATEADD(day,1,'%s') ", strDateField, to);
	}

	if (m_btnSelSources.GetCheck()) {
		id = m_pReferralSubDlg->GetSelectedReferralID();
		if (id != -1)
			source = "AND ReferralSource " + Descendants(id, "");
	}

	
	if (UseFilter(mftProvider))
		docFilter.Format("%s", provIDs);

	if (UseFilter(mftLocation))
		locFilter.Format(" AND PersonT.Location IN %s ", locIDs);

	return GetListBoxSql(source, docFilter, locFilter, strPatFilter, strFrom, strTo, nCategoryID);
}

CString CEffectiveness::GetGroupBy()	// this will be bill or patient
{
	CString strGroup;

	if (IsDlgButtonChecked(IDC_GROUP_BY_BILL))
		strGroup = "Bill";
	else
		strGroup = "Patient";

	return strGroup;
}

CString CEffectiveness::GetReferralSource()	// this returns the referral source selected, or "All" if none
{
	CString strRefSource;
	// (a.wilson 2012-5-14) PLID 50378 - update to handle inactives.
	strRefSource = m_pReferralSubDlg->GetSelectedReferralName(true);

	if (strRefSource == "") 
			strRefSource = "All";

	return "Referral Source: " + strRefSource;
}


void CEffectiveness::GetResults(CString &strExpense, CString &strReferrals, CString &strRefCost, CString &strPatients, CString &strPatCost, CString &strFeesBilled, CString &strFeesBilledPercent, CString &strFeesCollected, CString &strFeesCollectedPercent)
	// fills the params with appropriate values
{
	COleCurrency curTemp;
	COleCurrency curZero;
	curZero.SetCurrency(0, 0);

	strExpense = FormatCurrencyForInterface(m_cost);
	strReferrals.Format("%li", long(m_referral_count));

	if ( (m_cost == curZero) || (m_referral_count == 0) ) {
		curTemp.SetCurrency(0, 0);
	}
	else {
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly state the type of the denominator
		curTemp = m_cost / long(m_referral_count);
		RoundCurrency(curTemp);	
	}
	strRefCost = FormatCurrencyForInterface(curTemp);

	strPatients.Format("%li", long(m_patient_count));

	if ( (m_cost == curZero) || (m_patient_count == 0) )
	{
		curTemp.SetCurrency(0, 0);
	}
	else {
		// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - Explicitly state the type of the denominator
		curTemp = m_cost / long(m_patient_count);
		RoundCurrency(curTemp);
	}
	strPatCost = FormatCurrencyForInterface(curTemp);

	// (a.walling 2006-10-16 15:54) - PLID 3897 - Check for a zero denominator before calling GetPercent
	strFeesBilled = FormatCurrencyForInterface(m_total_billed);
	//(e.lally 2009-09-08) PLID 31096 - Fixed the comparison operator
	if (m_cost != curZero) {
		strFeesBilledPercent = FormatCurrencyForInterface(GetPercent(m_total_billed - m_cost, m_cost), false) + '%';
		strFeesCollectedPercent = FormatCurrencyForInterface(GetPercent(m_total_collected - m_cost, m_cost), false) + '%';
	}
	else {
		strFeesBilledPercent = FormatCurrencyForInterface(curZero, false) + '%';
		strFeesCollectedPercent = FormatCurrencyForInterface(curZero, false) + '%';
	}
	strFeesCollected = FormatCurrencyForInterface(m_total_collected);
	
	return;
}