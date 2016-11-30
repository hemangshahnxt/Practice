// MarketInternalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MarketInternalDlg.h"
#include "DocBar.h"
#include "MarketGraphDlg.h"
#include "GraphDescript.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ColumnGraph;
using namespace SmallSTDOLE2Lib;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CMarketInternalDlg dialog

//TES 6/4/2008 - PLID 30206 - Derive ourselves from CMarketingDlg
CMarketInternalDlg::CMarketInternalDlg(CWnd* pParent)
	: CMarketingDlg(CMarketInternalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketInternalDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nCurrentCategoryID = -1;
}

CMarketInternalDlg::~CMarketInternalDlg()
{
	if (m_hOldCursor)
		SetCursor(m_hOldCursor);
	DestroyCursor(m_hCursor);
}


void CMarketInternalDlg::DoDataExchange(CDataExchange* pDX)
{
	CMarketingDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketInternalDlg)
	DDX_Control(pDX, IDC_UP, m_Up);
	DDX_Control(pDX, IDC_MARKETING_INCIDENTS_PER_CAT, m_nxbIncidentsPerCat);
	DDX_Control(pDX, IDC_INCIDENTS_PER_PERSON, m_nxbIncidentsPerPerson);
	DDX_Control(pDX, IDC_INCIDENTS_PER_CLIENT, m_nxbIncidentsPerClient);
	DDX_Control(pDX, IDC_OPEN_PER_WEEK, m_nxbOpenPerWeek);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketInternalDlg, CMarketingDlg)
	//{{AFX_MSG_MAP(CMarketInternalDlg)
	ON_BN_CLICKED(IDC_MARKETING_INCIDENTS_PER_CAT, OnMarketingIncidentsPerCat)
	ON_BN_CLICKED(IDC_INCIDENTS_PER_PERSON, OnIncidentsPerPerson)
	ON_BN_CLICKED(IDC_INCIDENTS_PER_CLIENT, OnIncidentsPerClient)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_OPEN_PER_WEEK, OnOpenPerWeek)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketInternalDlg message handlers

BOOL CMarketInternalDlg::OnInitDialog() 
{
	CMarketingDlg::OnInitDialog();
	CString			parent, item;

	//set from date back
	COleDateTime dt = COleDateTime::GetCurrentTime();
	dt.SetDate (dt.GetYear() - 1, dt.GetMonth(), dt.GetDay());

	m_graph = GetDlgItem(IDC_INTERNAL_GRAPH)->GetControlUnknown();
	m_graph->Background = 0xFDFDFD;

	// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
	ColumnGraph::FontPtr font;
	font = m_graph->Font;
	font->PutName("Arial Narrow");
	font->PutSize(COleCurrency(13, 0));

	//setup the cursor
	m_hCursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_EXPAND));
	m_Up.AutoSet(NXB_LEFT);
	m_Up.EnableWindow(FALSE);
	m_hOldCursor = NULL;
	
	m_nxbIncidentsPerCat.SetToolTip("Display the number and duration of incidents entered per category.  The duration is the time spent on these incidents."); 
	m_nxbIncidentsPerPerson.SetToolTip(
		"Display information on finished incidents only.\n"
		"  Count = Number of incidents finished by this user.\n"
		"  Duration = Total amount of time spent on finished incidents (both those finished by this user and by others).\n"
		"  Average = Amount of time spent converted to minutes and divided by the number of items finished."
		);
	m_nxbIncidentsPerClient.SetToolTip("<NYI>");
	m_nxbOpenPerWeek.SetToolTip("Displays the number of incidents open at the end of each week.  Date given is the Friday of each week.");

	//set it to the incidents per category graph by default.  UpdateView will handle the refresh.
	m_nxbIncidentsPerCat.SetCheck(1);

	//in order not to call OnIncidentsPerCat here, we have to do what it does without doing the graph because
	//update view will do that part for us
	m_Up.ShowWindow(FALSE);
	m_Up.EnableWindow(FALSE);
	m_graph->Title = "All";
	SetType(INTERNAL_IncPerCat);
	SetFilter(mfChargeDate, mftDate);
	m_graph->PrintTitle = "Incidents Per Category";
	m_arClickableRows.RemoveAll();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarketInternalDlg::OnMarketingIncidentsPerCat() 
{
	CWaitCursor wc;

	SetType(INTERNAL_IncPerCat);
	SetFilter(mfChargeDate, mftDate);

	m_graph->PrintTitle = "Incidents Per Category";

	m_arClickableRows.RemoveAll();
	GraphIncidentsPerCategory();
}

void CMarketInternalDlg::GraphIncidentsPerCategory()
{
	CWaitCursor wc;

	try {
		if(GetMainFrame()->m_pDocToolBar->GetType() != INTERNAL_IncPerCat) {
			SetType(INTERNAL_IncPerCat);
			SetFilter(mfFirstContactDate, mftDate);
		}
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

		m_graph->PrintTitle = "Incidents Per Category";

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//Get the data out of the bar
		CString from, to, provIDs, locIDs, PatCoordIDs, strDateField, strLocationField, strProvField;
		int nCategoryID, nResp;
		GetParameters(from, to, provIDs, locIDs, PatCoordIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		CString strSql = GetGraphSql(INTERNAL_IncPerCat, -1, m_nCurrentCategoryID, pCon, strPatientTempTable);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", "IssueT.EnteredDate", from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", "IssueT.EnteredDate", to);
		}

		//replace the from and to dates
		strSql.Replace("[from]", strFrom);
		strSql.Replace("[to]", strTo);

		//
		//create the graph legend data (including totals)
		CString strCntTotal = "Count", strDurTotal = "Duration (Hours)";
		_RecordsetPtr prsTotals = CreateRecordset("SELECT CONVERT(int, SUM(IncidentCount)) AS TotalCount, "
			"CONVERT(float, SUM(CategoryDuration)) AS TotalDuration "
			"FROM (%s) BaseQ ", strSql);

		if(!prsTotals->eof) {
			strCntTotal.Format("Count (%li total)", AdoFldLong(prsTotals, "TotalCount", 0));
			strDurTotal.Format("Duration (%0.2f hours total)", AdoFldDouble(prsTotals, "TotalDuration", 0.0));
		}
		prsTotals->Close();

		GraphDescript desc;
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
		desc.Add(strCntTotal, "IncidentCount", GetMarketGraphColor(mgcBrightBlue));
		desc.Add(strDurTotal, "CategoryDuration", GetMarketGraphColor(mgcBrightGreen));
		m_graph->Format = "%0.2f";
		//

		//generate a recordset for our graph query
		_RecordsetPtr prsQuery = CreateRecordset(strSql);

		//Actual graph formatting
		m_graph->ClearGraph();
		m_graph->ColumnCount = desc.Size();
		for (int i = 0; i < desc.Size(); i++)
		{
			m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			m_graph->ColumnFormat = m_graph->Format;
		}

		m_graph->XAxisDesc = "Categories";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		// (b.cardillo 2004-08-06 09:17) - PLID 13747 - Before the loop, scan the recordset into 
		// our map of values, so that we can access it quickly inside the loop.
		CMapLongToGraphValuesArray mapReferralGraphValues;
		mapReferralGraphValues.ScanRecordsetIntoMap(prsQuery, "CategoryID", desc);

		//all the real work is done, but we still have to sort the results
		int nCurrentID = -1;
		i = 0;

		CString strCurrentID = "IS NULL";
		if(m_nCurrentCategoryID != -1)
			strCurrentID.Format("= %li", m_nCurrentCategoryID);

		_RecordsetPtr prsCats = CreateRecordset("SELECT ID, Description FROM IssueCategoryT WHERE ParentID %s UNION SELECT -1, '<Uncategorized>'", strCurrentID);
		if(!prsCats->eof)
			m_graph->RowCount = prsCats->GetRecordCount();
		else
			m_graph->RowCount = 0;

		FieldPtr fldName = prsCats->Fields->GetItem("Description");
		FieldPtr fldPersonID = prsCats->Fields->GetItem("ID");
		while (!prsCats->eof)
		{	//set chart names
			m_graph->Row = i++;
			CString strDesc = AdoFldString(fldName);
			m_graph->RowText = (LPCTSTR)strDesc;
			nCurrentID = AdoFldLong(fldPersonID);
			m_graph->RowID = nCurrentID;

			{
				//Get descendants of the ith child
				// (b.cardillo 2004-08-06 09:48) - PLID 13747 - We used to do this by filling an array, 
				// now we just get a pointer to the sub-tree so it's much faster, which is important 
				// inside a loop.  This pointer is used to determine if there are any children, and then 
				// the subtree it points to is used below for calculating the totals.
				const CTreeLong *ptlDescendents = NULL;
	//TODO		Descendants(currentID, &ptlDescendents);

				//Old way above, new way here?  TESTING
				if(ReturnsRecords("SELECT TOP 1 * FROM IssueT LEFT JOIN IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID "
					"WHERE ParentID = %li %s %s", nCurrentID, strFrom, strTo)) {
					//there are subcategories in this date range for this current source.
					m_graph->RowDrillDown = TRUE;
					m_arClickableRows.Add(nCurrentID);
				}
				else
					m_graph->RowDrillDown = FALSE;

			}

			//for each column
			double total = 0, total2 = 0;
			for (int j = 0; j < desc.Size(); j++)
			{	m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				// (b.cardillo 2004-08-06 09:19) - PLID 13747 - We used to loop through the recordset here, 
				// adding up the values from Field and Field2 in the recordset wherever the recordset's 
				// ReferralID was currentID or any of its descendents.  Now we do the same thing but we use 
				// our super-fast map that we loaded before the loop instead of the recordset over and over.
				if (!prsQuery->eof) {
					// Create the object that will do the processing
					CED_GraphCalcColumnTotals_Info gbrsi(&mapReferralGraphValues, desc, j, total, total2);
					// Call it for the root level
					gbrsi.ProcessGraphValues(nCurrentID);
					// If we're on a drill-down-able node, then add all of those in too
//					if (ptlDescendents) {
//TODO					ptlDescendents->EnumDescendents(CED_GraphCalcColumnTotals_Info::CallbackProcessGraphValues, &gbrsi, FALSE);
//					}
					// Now return the totals values to our local variables
					total = gbrsi.m_dblTotal;
					total2 = gbrsi.m_dblTotal2;
				}


				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						m_graph->Value = total;
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

			prsCats->MoveNext();
		}

		m_graph->SortGraph();

		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error Graphing Patients");

}

void CMarketInternalDlg::OnIncidentsPerPerson() 
{
	CWaitCursor wc;

	SetType(INTERNAL_IncPerPerson);
	SetFilter(mfChargeDate, mftDate);

	m_graph->PrintTitle = "Incidents Per Person Per Category";
	m_nCurrentCategoryID = -1;

	GraphIncidentsPerPerson();
}

void CMarketInternalDlg::GraphIncidentsPerPerson()
{
	CWaitCursor wc;

	try {
		if(GetMainFrame()->m_pDocToolBar->GetType() != INTERNAL_IncPerPerson) {
			SetType(INTERNAL_IncPerPerson);
			SetFilter(mfFirstContactDate, mftDate);
		}
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

		m_graph->PrintTitle = "Incidents Per Person Per Category";

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		//Get the data out of the bar
		CString from, to, provIDs, locIDs, PatCoordIDs, strDateField, strLocationField, strProvField;
		int nCategoryID, nResp;
		GetParameters(from, to, provIDs, locIDs, PatCoordIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		CString strSql = GetGraphSql(INTERNAL_IncPerPerson, -1, m_nCurrentCategoryID, pCon, strPatientTempTable);

		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", "IssueT.FinishedDate", from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", "IssueT.FinishedDate", to);
		}

		//replace the from and to dates
		strSql.Replace("[from]", strFrom);
		strSql.Replace("[to]", strTo);

		//
		//create the graph legend data (including totals) -- since this query is "wierd" and creates a temp table and all 
		//	that jazz, we unfortunately have to loop through all records here to calculate the legend.
		CString strCntTotal = "Count", strDurTotal = "Duration (Hours)";
		_RecordsetPtr prsTotals = CreateRecordset("%s", strSql);

		double dblTtlCnt = 0;
		double dblTtlDur = 0.0;

		while(!prsTotals->eof) {
			double dblCnt = AdoFldDouble(prsTotals, "IncidentCount", 0);
			double dblDur = AdoFldDouble(prsTotals, "TotalDuration", 0.0);

			dblTtlCnt += dblCnt;
			dblTtlDur += dblDur;

			prsTotals->MoveNext();
		}
		prsTotals->Close();

		strCntTotal.Format("Count (%0.0f total)", dblTtlCnt);
		strDurTotal.Format("Duration (%0.2f hours total)", dblTtlDur);

		GraphDescript desc;
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
		desc.Add(strCntTotal, "IncidentCount", GetMarketGraphColor(mgcBrightBlue));
		desc.Add(strDurTotal, "TotalDuration", GetMarketGraphColor(mgcBrightGreen));
		desc.Add("Average (minutes)", "AvgDuration", GetMarketGraphColor(mgcBrightRed));
		m_graph->Format = "%0.2f";

		//


		//generate a recordset for our graph query
		_RecordsetPtr prsQuery = CreateRecordsetStd(strSql);

		//Actual graph formatting
		m_graph->ClearGraph();
		m_graph->ColumnCount = desc.Size();
		for (int i = 0; i < desc.Size(); i++)
		{
			m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			m_graph->ColumnFormat = m_graph->Format;
		}

		m_graph->XAxisDesc = "Categories";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		// (b.cardillo 2004-08-06 09:17) - PLID 13747 - Before the loop, scan the recordset into 
		// our map of values, so that we can access it quickly inside the loop.
		CMapLongToGraphValuesArray mapReferralGraphValues;
		if(m_nCurrentCategoryID == -1)
			//per category
			mapReferralGraphValues.ScanRecordsetIntoMap(prsQuery, "CategoryID", desc);
		else 
			//per person
			mapReferralGraphValues.ScanRecordsetIntoMap(prsQuery, "FinishedByUserID", desc);

		//all the real work is done, but we still have to sort the results
		int nCurrentID = -1;
		i = 0;

		CString strCurrentID = "IS NULL";
		if(m_nCurrentCategoryID != -1)
			strCurrentID.Format("= %li", m_nCurrentCategoryID);

		FieldPtr fldName;
		FieldPtr fldPersonID;
		_RecordsetPtr prsCats;

		if(m_nCurrentCategoryID == -1) {
			//We are showing the category on the top level
			prsCats = CreateRecordset("SELECT ID, Description FROM IssueCategoryT WHERE ParentID IS NULL UNION SELECT 0, '<Uncategorized>'");
			if(!prsCats->eof)
				m_graph->RowCount = prsCats->GetRecordCount();
			else
				m_graph->RowCount = 0;

			fldName = prsCats->Fields->GetItem("Description");
			fldPersonID = prsCats->Fields->GetItem("ID");
		}
		else {
			//We are showing per person on the bottom level
			prsCats = CreateRecordset("SELECT PersonID, UserName FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID WHERE PersonT.Archived = 0 AND PersonT.ID > 0");
			if(!prsCats->eof)
				m_graph->RowCount = prsCats->GetRecordCount();
			else
				m_graph->RowCount = 0;

			fldName = prsCats->Fields->GetItem("UserName");
			fldPersonID = prsCats->Fields->GetItem("PersonID");
		}

		while (!prsCats->eof)
		{	//set chart names
			m_graph->Row = i++;
			CString strDesc = AdoFldString(fldName);
			m_graph->RowText = (LPCTSTR)strDesc;
			nCurrentID = AdoFldLong(fldPersonID);
			m_graph->RowID = nCurrentID;

			{
				if(m_nCurrentCategoryID == -1) {
					//Always allow drill down on the top level to see per category
					m_graph->RowDrillDown = TRUE;
					m_arClickableRows.Add(nCurrentID);
				}
				else
					//never allow drill down on the persons
					m_graph->RowDrillDown = FALSE;
			}

			//for each column
			double total = 0, total2 = 0;
			for (int j = 0; j < desc.Size(); j++)
			{	m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				// (b.cardillo 2004-08-06 09:19) - PLID 13747 - We used to loop through the recordset here, 
				// adding up the values from Field and Field2 in the recordset wherever the recordset's 
				// ReferralID was currentID or any of its descendents.  Now we do the same thing but we use 
				// our super-fast map that we loaded before the loop instead of the recordset over and over.
				if (!prsQuery->eof) {
					// Create the object that will do the processing
					CED_GraphCalcColumnTotals_Info gbrsi(&mapReferralGraphValues, desc, j, total, total2);
					// Call it for the root level
					gbrsi.ProcessGraphValues(nCurrentID);
					// If we're on a drill-down-able node, then add all of those in too
//					if (ptlDescendents) {
//TODO					ptlDescendents->EnumDescendents(CED_GraphCalcColumnTotals_Info::CallbackProcessGraphValues, &gbrsi, FALSE);
//					}
					// Now return the totals values to our local variables
					total = gbrsi.m_dblTotal;
					total2 = gbrsi.m_dblTotal2;
				}


				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						m_graph->Value = total;
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
						ASSERT(FALSE);//things I haven't handled yet, or bad values
				}
			}

			prsCats->MoveNext();
		}

		m_graph->SortGraph();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error Graphing Patients");
	
}

void CMarketInternalDlg::OnIncidentsPerClient() 
{
	CWaitCursor wc;

	SetType(INTERNAL_IncPerClient);
	SetFilter(mfChargeDate, mftDate);

	m_graph->PrintTitle = "Incidents Per Client";

	GraphIncidentsPerClient();
}

void CMarketInternalDlg::GraphIncidentsPerClient()
{
	AfxMessageBox("Not yet implemented.");
}

void CMarketInternalDlg::OnOpenPerWeek() 
{
	CWaitCursor wc;

	SetType(INTERNAL_OpenPerWeek);
	SetFilter(mfFirstContactDate, mftDate);

	m_graph->PrintTitle = "Open Per Week";

	GraphOpenPerWeek();
}

void CMarketInternalDlg::GraphOpenPerWeek()
{
	CWaitCursor wc;

	try {

		/////////
		//Options
		MarketGraphType eType = INTERNAL_OpenPerWeek;
		MarketFilter mfDefaultDateFilter = mfFirstContactDate;
		CString strGraphTitle = "Incidents Per Person Per Category";
		CString strGraphXAxis = "Date";

		//Information for mapping the recordset to calculate totals
		CString strMapIDField;
		strMapIDField = "DateID";
		//


		//Information for displaying the categories
		CString strCatQuery;
		CString strCatFieldName;
		CString strCatIDFieldName;
		//We are showing the category on the top level
		strCatQuery = "select YEAR(EnteredDate) * 10000 + MONTH(EnteredDate) * 100 + DAY(EnteredDate) AS DateID, EnteredDate "
			"FROM "
			"( "
			"SELECT convert(nvarchar, EnteredDate, 101) as EnteredDate "
			"FROM IssueT "
			"WHERE DATENAME(dw, EnteredDate) = 'Friday' "
			"[from] [to] "
			"GROUP BY convert(nvarchar, EnteredDate, 101) "
			") Q "
			"ORDER BY YEAR(EnteredDate) * 10000 + MONTH(EnteredDate) * 100 + DAY(EnteredDate)";
		strCatFieldName = "EnteredDate";
		strCatIDFieldName = "DateID";
		//

		m_graph->ShowXRecords = 20;
		//
		/////////



		if(GetMainFrame()->m_pDocToolBar->GetType() != eType) {
			SetType(eType);
			SetFilter(mfDefaultDateFilter, mftDate);
		}
		GetMainFrame()->m_pDocToolBar->SetHiddenFilters(DBF_CATEGORY|DBF_RESP);

		m_graph->PrintTitle = _bstr_t(strGraphTitle);

		//Get the data out of the bar
		CString from, to, provIDs, locIDs, PatCoordIDs, strDateField, strLocationField, strProvField;
		int nCategoryID, nResp;

		_ConnectionPtr pCon = GetRemoteData();
		CString strPatientTempTable;

		GetParameters(from, to, provIDs, locIDs, PatCoordIDs, strDateField, strLocationField, strProvField, nCategoryID, nResp, pCon, strPatientTempTable);

		CString strSql = GetGraphSql(eType, -1, m_nCurrentCategoryID, pCon, strPatientTempTable);
/**/
		CString strFrom, strTo;
		if (!strDateField.IsEmpty() && GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
			strFrom.Format(" AND %s >= '%s' ", "IssueT.EnteredDate", from);
			strTo.Format(" AND %s < DATEADD(day,1,'%s') ", "IssueT.EnteredDate", to);
		}
/**/
		//replace the from and to dates
		strSql.Replace("[from]", strFrom);
		strSql.Replace("[to]", strTo);

		//replace the same in the category query
		strCatQuery.Replace("[from]", strFrom);
		strCatQuery.Replace("[to]", strTo);


/*

I don't think we need this section at all.

		//
		//The other marketing graphs all do a "SELECT SUM(SOMETHING) FROM (<insert graph query here>) AS BaseQ" type thing, but
		//	since I'm taking advantage of some special T-SQL stuff (cursors, temp tables, etc), it's a little more robust to 
		//	calculate this by looping.  Not too much slower.
		CString strCntTotal = "Count", strDurTotal = "Duration (Hours)";
		_RecordsetPtr prsTotals = CreateRecordset("%s", strSql);

		double dblTtl = 0;

		while(!prsTotals->eof) {
			dblTtl += AdoFldDouble(prsTotals, "NumOpen", 0);

			prsTotals->MoveNext();
		}
		prsTotals->Close();
*/
		GraphDescript desc;
		//(e.lally 2009-09-24) PLID 35526 - Use the readable bargraph color names
		desc.Add("Number Open", "NumOpen", GetMarketGraphColor(mgcBrightBlue));
		m_graph->Format = "%0.2f";
		//

		//generate a recordset for our graph query
		_RecordsetPtr prsQuery = CreateRecordsetStd(strSql);

		//Actual graph formatting
		m_graph->ClearGraph();
		m_graph->ColumnCount = desc.Size();
		for (int i = 0; i < desc.Size(); i++)
		{
			m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
			m_graph->ColumnFormat = m_graph->Format;
		}

		m_graph->XAxisDesc = _bstr_t(strGraphXAxis);
		//To sort by the category, we have to set the sort column and the style
		m_graph->SortColumn = -1;
		m_graph->XAxisSortStyle = cgSortID;
		m_graph->AllowXAxisSort = TRUE;

		// (b.cardillo 2004-08-06 09:17) - PLID 13747 - Before the loop, scan the recordset into 
		// our map of values, so that we can access it quickly inside the loop.
		CMapLongToGraphValuesArray mapReferralGraphValues;
		mapReferralGraphValues.ScanRecordsetIntoMap(prsQuery, strMapIDField, desc);

		//all the real work is done, but we still have to sort the results
		int nCurrentID = -1;
		i = 0;

		FieldPtr fldName;
		FieldPtr fldPersonID;
		_RecordsetPtr prsCats;

		//////////////////////
		//Figure out all the categorization (x axis)
		prsCats = CreateRecordset(strCatQuery);
		if(!prsCats->eof)
			m_graph->RowCount = prsCats->GetRecordCount();
		else
			m_graph->RowCount = 0;

		fldName = prsCats->Fields->GetItem(_bstr_t(strCatFieldName));
		fldPersonID = prsCats->Fields->GetItem(_bstr_t(strCatIDFieldName));

		while (!prsCats->eof)
		{	//set chart names
			m_graph->Row = i++;
			CString strDesc = AdoFldString(fldName);
			m_graph->RowText = (LPCTSTR)strDesc;
			nCurrentID = AdoFldLong(fldPersonID);
			m_graph->RowID = nCurrentID;

			{/*		no need for drill down here

				if(m_nCurrentCategoryID == -1) {
					//Always allow drill down on the top level to see per category
					m_graph->RowDrillDown = TRUE;
					m_arClickableRows.Add(nCurrentID);
				}
				else
					//never allow drill down on the persons
					m_graph->RowDrillDown = FALSE;
			*/}

			//for each column
			double total = 0, total2 = 0;
			for (int j = 0; j < desc.Size(); j++)
			{	m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				// (b.cardillo 2004-08-06 09:19) - PLID 13747 - We used to loop through the recordset here, 
				// adding up the values from Field and Field2 in the recordset wherever the recordset's 
				// ReferralID was currentID or any of its descendents.  Now we do the same thing but we use 
				// our super-fast map that we loaded before the loop instead of the recordset over and over.
				if (!prsQuery->eof) {
					// Create the object that will do the processing
					CED_GraphCalcColumnTotals_Info gbrsi(&mapReferralGraphValues, desc, j, total, total2);
					// Call it for the root level
					gbrsi.ProcessGraphValues(nCurrentID);
					// If we're on a drill-down-able node, then add all of those in too
//					if (ptlDescendents) {
//TODO					ptlDescendents->EnumDescendents(CED_GraphCalcColumnTotals_Info::CallbackProcessGraphValues, &gbrsi, FALSE);
//					}
					// Now return the totals values to our local variables
					total = gbrsi.m_dblTotal;
					total2 = gbrsi.m_dblTotal2;
				}


				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						m_graph->Value = total;
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
						ASSERT(FALSE);//things I haven't handled yet, or bad values
				}
			}

			prsCats->MoveNext();
		}

		m_graph->SortGraph();

		// (j.jones 2010-07-19 15:45) - PLID 39053 - drop our temp table if we have one
		if(!strPatientTempTable.IsEmpty()) {
			ExecuteSql(pCon, "DROP TABLE %s", strPatientTempTable);
			strPatientTempTable = "";
		}

	}NxCatchAll("Error Graphing open per week");


}

BEGIN_EVENTSINK_MAP(CMarketInternalDlg, CMarketingDlg)
    //{{AFX_EVENTSINK_MAP(CMarketInternalDlg)
	ON_EVENT(CMarketInternalDlg, IDC_INTERNAL_GRAPH, 1 /* OnClickColumn */, OnOnClickColumnInternalGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketInternalDlg, IDC_INTERNAL_GRAPH, 2 /* OnMouseMoveColumn */, OnOnMouseMoveColumnInternalGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketInternalDlg, IDC_INTERNAL_GRAPH, 3 /* OnChangeBackButtonPos */, OnOnChangeBackButtonPosInternalGraph, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMarketInternalDlg::OnOnClickColumnInternalGraph(short Row, short Column) 
{
	m_graph->Row = Row;
	for(int i = 0; i < m_arClickableRows.GetSize(); i++) {
		if(m_arClickableRows.GetAt(i) == m_graph->RowID) {
			m_graph->Row = Row;
			m_nCurrentCategoryID = m_graph->RowID;

			//Set the title so we know what category we are in
			CString strText = VarString(_bstr_t(m_graph->RowText), "");
			m_graph->Title = _bstr_t(strText);

			UpdateView();
			return;
		}
	}
}

void CMarketInternalDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {		
		if (m_nxbIncidentsPerCat.GetCheck()) {
			GraphIncidentsPerCategory();
		}
		else if (m_nxbIncidentsPerPerson.GetCheck()) {
			GraphIncidentsPerPerson();
		}
		else if (m_nxbIncidentsPerClient.GetCheck()) {
			GraphIncidentsPerClient();
		}
		else if(m_nxbOpenPerWeek.GetCheck()) {
			GraphOpenPerWeek();
		}

		m_Up.EnableWindow(TRUE);
		m_Up.ShowWindow(SW_SHOW);

		//ensure the graph's back button is still positioned correctly
		OnOnChangeBackButtonPosInternalGraph();

		//(e.lally 2009-08-28) PLID 35308 - For safety, ensure all the filters are valid
		EnsureFilter(mftDate);
		EnsureFilter(mftLocation);
		EnsureFilter(mftProvider);

	}NxCatchAll("Error in UpdateView");
}

void CMarketInternalDlg::OnOnMouseMoveColumnInternalGraph(short Row, short Column) 
{
 	bool bIsRowClickable = false;
	m_graph->Row = Row;
	long nRowID = m_graph->RowID;
	for(int i = 0; i < m_arClickableRows.GetSize(); i++) if(m_arClickableRows.GetAt(i) == nRowID) bIsRowClickable = true;

	if (bIsRowClickable)
	{	if (!m_hOldCursor)
			m_hOldCursor = SetCursor(m_hCursor);
		else SetCursor(m_hCursor);
	}
	else if (m_hOldCursor)
		SetCursor(m_hOldCursor);
}

void CMarketInternalDlg::OnOnChangeBackButtonPosInternalGraph() 
{
/*	CRect rect;
	m_Up.GetWindowRect(rect);
	ScreenToClient(rect);

	CRect graphrect;
	GetDlgItem(IDC_INTERNAL_GRAPH)->GetWindowRect(graphrect);
	ScreenToClient(graphrect);

 	rect.top = graphrect.bottom - m_graph->BackButtonYDiff;
	rect.bottom = rect.top + m_graph->BackButtonHeight;
	m_Up.MoveWindow(rect);
*/
 }

void CMarketInternalDlg::OnUp() 
{
	CWaitCursor wc;

	if (m_nCurrentCategoryID <= 0)
		return;

	try
	{
		_RecordsetPtr prs = CreateRecordset("SELECT IssueCategoryT.ParentID, ICT.Description FROM IssueCategoryT LEFT JOIN IssueCategoryT AS ICT ON IssueCategoryT.ParentID = ICT.ID "
			"WHERE IssueCategoryT.ID = %li", m_nCurrentCategoryID);

		if(!prs->eof) {
			m_nCurrentCategoryID = AdoFldLong(prs, "ParentID", -1);
			CString strName = AdoFldString(prs, "Description", "All");
			m_graph->Title = _bstr_t(strName);
		}
		else {
			m_nCurrentCategoryID = -1;
			//Set the title so we know what category we are in
			m_graph->Title = "All";

		}
	}
	NxCatchAll("Error in OnUp");

	UpdateView();

	//revert to showing the default amount of records
	m_graph->ShowXRecords = 10;	
}
