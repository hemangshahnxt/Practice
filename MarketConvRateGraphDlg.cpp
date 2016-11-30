#include "stdafx.h"

// CMarketConvRateGraphDlg.cpp : implementation file
/*
Add so far

Number of Patients
Fees Billed/Collected/Pending
Marketing Costs

to add later 

Cost Per Patient
% on Marketing Costs
*/

//
//
//(e.lally 2009-09-11) Dead code
//
//

//#include "marketUtils.h"
#include "MarketConvRateGraphDlg.h"
/*
#include <winuser.h>
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "ConversionRateByDateConfigDlg.h"
#include "GlobalDrawingUtils.h"
*/
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
//using namespace ColumnGraph;
//using namespace NxTab;
//using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMarketConvRateGraphDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

CMarketConvRateGraphDlg::~CMarketConvRateGraphDlg()
{
	//if (m_oldCursor)
	//	SetCursor(m_oldCursor);
	//DestroyCursor(m_cursor);
}

CMarketConvRateGraphDlg::CMarketConvRateGraphDlg(CWnd* pParent)
	: CNxDialog(CMarketConvRateGraphDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CMarketConvRateGraphDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	//m_strManualLocation = "NexTech_Practice_Manual.chm";
	//m_strManualBookmark = "Marketing/Tabs/compare.htm";
	//initialized = false;
}
/*
void CMarketConvRateGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketConvRateGraphDlg)
	DDX_Control(pDX, IDC_UP, m_up);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_CONVERSION_RAD, m_conversionRad);
	DDX_Control(pDX, IDC_CONVERSION_DATE_RAD, m_ConvDateRad);
	DDX_Control(pDX, IDC_SHOW_ALL_COLUMNS, m_ShowAllRad);
	DDX_Control(pDX, IDC_SHOW_NUMBERS_ONLY, m_ShowNumberRad);
	DDX_Control(pDX, IDC_SHOW_PERCENTAGES_ONLY, m_ShowPercentRad);
	DDX_Control(pDX, IDC_INQ_TO_CONS_BY_PROC, m_InqToConsByProc);
	DDX_Control(pDX, IDC_PROS_TO_CONS_BY_PROC, m_ProsToConsByProc);
	DDX_Control(pDX, IDC_PROS_TO_SURG_BY_PROC, m_ProsToSurgByProc);
	DDX_Control(pDX, IDC_CONS_TO_SURG_BY_PAT_COORD, m_ConsToSurgByPatCoord);
	DDX_Control(pDX, IDC_SHOW_ALL_PROSPECTS, m_ShowAllProspects);
	DDX_Control(pDX, IDC_SHOW_CONS_PROSPECTS, m_ShowProsConsOnly);
	DDX_Control(pDX, IDC_FILTERED_TO_DATE, m_ToDate);
	DDX_Control(pDX, IDC_FILTERED_FROM_DATE, m_FromDate);
	DDX_Control(pDX, IDC_INQ_TO_CONS_BY_STAFF2, m_InqToConsByStaff);
	DDX_Control(pDX, IDC_STATIC1, m_nxstatic1);
	DDX_Control(pDX, IDC_STATIC2, m_nxstatic2);
	DDX_Control(pDX, IDC_STATIC3, m_nxstatic3);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CMarketConvRateGraphDlg, IDC_FROM, 3 /* Change * /, OnChangeFrom, VTS_NONE)
//	ON_EVENT(CMarketConvRateGraphDlg, IDC_TO, 3 /* Change * /, OnChangeTo, VTS_NONE)
//	ON_EVENT(CMarketConvRateGraphDlg, IDC_FROM, 1 /* CloseUp * /, OnCloseUpFrom, VTS_NONE)
//	ON_EVENT(CMarketConvRateGraphDlg, IDC_TO, 1 /* CloseUp * /, OnCloseUpTo, VTS_NONE)

BEGIN_MESSAGE_MAP(CMarketConvRateGraphDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketConvRateGraphDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FROM, OnChangeFrom)
	ON_NOTIFY(DTN_CLOSEUP, IDC_FROM, OnCloseUpFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TO, OnChangeTo)
	ON_NOTIFY(DTN_CLOSEUP, IDC_TO, OnCloseUpTo)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_CONVERSION_RAD, OnConversionRad)
	ON_BN_CLICKED(IDC_CONVERSION_DATE_RAD, OnConversionDateRad)
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TYPES, OnConfigureApptTypes)
	ON_BN_CLICKED(IDC_SHOW_ALL_COLUMNS, OnShowAllColumns)
	ON_BN_CLICKED(IDC_SHOW_NUMBERS_ONLY, OnShowNumbersOnly)
	ON_BN_CLICKED(IDC_SHOW_PERCENTAGES_ONLY, OnShowPercentagesOnly)
	ON_BN_CLICKED(IDC_CONS_TO_SURG_BY_PAT_COORD, OnConsToSurgByPatCoord)
	ON_BN_CLICKED(IDC_INQ_TO_CONS_BY_PROC, OnInqToConsByProc)
	ON_BN_CLICKED(IDC_PROS_TO_CONS_BY_PROC, OnProsToConsByProc)
	ON_BN_CLICKED(IDC_PROS_TO_SURG_BY_PROC, OnProsToSurgByProc)
	ON_BN_CLICKED(IDC_SHOW_CONS_PROSPECTS, OnShowConsProspects)
	ON_BN_CLICKED(IDC_SHOW_ALL_PROSPECTS, OnShowAllProspects)
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_INQ_TO_CONS_BY_STAFF2, OnInqToConsByStaff)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMarketConvRateGraphDlg, CNxDialog)
	//{{AFX_DISPATCH_MAP(CMarketConvRateGraphDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketConvRateGraphDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketConvRateGraphDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMarketConvRateGraphDlg)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_GRAPH, 1 /* OnClickColumn * /, OnClickColumnGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_GRAPH, 2 /* OnMouseMoveColumn * /, OnMouseMoveColumnGraph, VTS_I2 VTS_I2)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_APPT_PURPOSE_LIST, 16 /* SelChosen * /, OnSelChosenApptPurposeList, VTS_I4)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_YEAR_FILTER, 16 /* SelChosen * /, OnSelChosenYearFilter, VTS_I4)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_DATE_OPTION_LIST, 16 /* SelChosen * /, OnSelChosenDateOptionList, VTS_I4)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_FILTERED_FROM_DATE, 2 /* Change * /, OnChangeFilteredFromDate, VTS_NONE)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_FILTERED_LOCATION_LIST, 16 /* SelChosen * /, OnSelChosenFilteredLocationList, VTS_I4)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_FILTERED_PROVIDER_LIST, 16 /* SelChosen * /, OnSelChosenFilteredProviderList, VTS_I4)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_FILTERED_TO_DATE, 2 /* Change * /, OnChangeFilteredToDate, VTS_NONE)
	ON_EVENT(CMarketConvRateGraphDlg, IDC_PAT_COORD_LIST, 16 /* SelChosen * /, OnSelChosenPatCoordList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMarketConvRateGraphDlg::Up() 
{
	if (m_referral == "RootNode")
		return;

	try
	{
		_RecordsetPtr rs(__uuidof(Recordset));
		CString	sql = "SELECT R2.Name FROM ReferralSourceT AS R1 LEFT JOIN ReferralSourceT AS R2 "
			"ON R1.Parent = R2.PersonID "
			"WHERE R1.Name = \'" + _Q(m_referral) + "\';";

		rs->Open(
			(_bstr_t)sql, 
			_variant_t((IDispatch *) g_ptrRemoteData, true), 
			adOpenStatic,
			adLockOptimistic,
			adCmdText);
		ASSERT (rs->RecordCount == 1);

		if (rs->Fields->GetItem("Name")->Value.vt == VT_BSTR)
			m_referral = (LPCTSTR) _bstr_t(rs->Fields->GetItem("Name")->Value);
		else m_referral = "RootNode";
	}
	NxCatchAll("Could not go back");

	UpdateView();

	//revert to showing the default amount of records
	m_graph->ShowXRecords = 10;
}

int CMarketConvRateGraphDlg::GetCurrentID()
{
	if (m_referral == "RootNode")
		return - 1;

	_RecordsetPtr rs(__uuidof(Recordset));
	CString sql;

	sql = "SELECT PersonID FROM ReferralSourceT WHERE Name = \'" + _Q(m_referral) + "\';";
	rs->Open(
		(_bstr_t)sql, 
		_variant_t((IDispatch *) g_ptrRemoteData, true), 
		adOpenStatic,
		adLockOptimistic,
		adCmdText);
	ASSERT (rs->RecordCount == 1);
	return rs->Fields->GetItem("PersonID")->Value.lVal;
}

void CMarketConvRateGraphDlg::Graph(GraphDescript &desc)
{
	try
	{
		//ensure connection is active
		EnsureRemoteData();

		//declare variables
		_RecordsetPtr	rsReferrals = NULL,
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
		
		sql.Format ("SELECT (CASE WHEN PersonID = %i THEN 'ZZZZZZZ' ELSE ReferralSourceT.Name END) AS Name, "
			"PersonID FROM ReferralSourceT "
			"WHERE Parent = %i OR PersonID = %i ORDER BY Name;", id, id, id);

		rsReferrals = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsReferrals->RecordCount;
		m_graph->RowCount = max;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(25);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
		}

		m_graph->XAxisDesc = "Referral Source";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		int currentID;
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
			if (currentID != id)//special [Other] column
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
							{	total += AdoFldDouble(rs, desc.Field(j));
								if (desc.Op(j) == GraphDescript::GD_DIV)
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
						break;
					case GraphDescript::GD_DIV:
						if (total2 != 0)
							m_graph->Value = total / total2;
						else m_graph->Value = 0;
						break;
					default:
						ASSERT(FALSE);//things I haven't handeled yet, or bad values
				}
			}

			rsReferrals->MoveNext();
			m_progress.SetPos(50 + 50 * i / max);
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}


BOOL CMarketConvRateGraphDlg::OnInitDialog() 
{
	// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
	ColumnGraph::FontPtr font;

	CNxDialog::OnInitDialog();

	m_conversionRad.SetColor(0x00C8FFFF);
	m_ConvDateRad.SetColor(0x00C8FFFF);
	m_ShowAllRad.SetColor(0x00FFFFFF);
	m_ShowNumberRad.SetColor(0x00FFFFFF);
	m_ShowPercentRad.SetColor(0x00FFFFFF);
	m_InqToConsByProc.SetColor(0x00C8FFFF);
	m_ProsToConsByProc.SetColor(0x00C8FFFF);
	m_ProsToSurgByProc.SetColor(0x00C8FFFF);
	m_InqToConsByStaff.SetColor(0x00C8FFFF);
	m_ConsToSurgByPatCoord.SetColor(0x00C8FFFF);
	m_ShowAllProspects.SetColor(0x00FFFFFF);
	m_ShowProsConsOnly.SetColor(0x00FFFFFF);
	
	
	m_graph = GetDlgItem(IDC_GRAPH)->GetControlUnknown();
	m_cursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_EXPAND));
	m_up.AutoSet(NXB_UP);
	m_conversionRad.SetCheck(TRUE);
	m_referral = "RootNode";
	m_up.EnableWindow(FALSE);
	m_oldCursor = NULL;
	font = m_graph->Font;
	font->PutName("Arial Narrow");
	font->PutSize(COleCurrency(13, 0));

	m_graph->Background = 0xFDFDFD;

	//initialize the datalist
	m_pApptPurposeList = BindNxDataListCtrl(IDC_APPT_PURPOSE_LIST, GetRemoteData(), FALSE);
	m_pYearFilter = BindNxDataListCtrl(IDC_YEAR_FILTER, GetRemoteData(), FALSE);

	
	m_pLocationsList = BindNxDataListCtrl(IDC_FILTERED_LOCATION_LIST, GetRemoteData(), TRUE);
	m_pProvidersList = BindNxDataListCtrl(IDC_FILTERED_PROVIDER_LIST, GetRemoteData(), TRUE);
	m_pDateOptionsList = BindNxDataListCtrl(IDC_DATE_OPTION_LIST, GetRemoteData(), FALSE);
	m_pPatCoordList = BindNxDataListCtrl(IDC_PAT_COORD_LIST, GetRemoteData(), TRUE);

	//add the rows to the date options
	IRowSettingsPtr pRow = m_pDateOptionsList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _variant_t("<All Dates>"));
	m_pDateOptionsList->InsertRow(pRow, 0);

	pRow = m_pDateOptionsList->GetRow(-1);
	pRow->PutValue(0, (long)1);
	pRow->PutValue(1, _variant_t("Appt. Date"));
	m_pDateOptionsList->AddRow(pRow);

	pRow = m_pDateOptionsList->GetRow(-1);
	pRow->PutValue(0, (long)2);
	pRow->PutValue(1, _variant_t("First Contact Date"));
	m_pDateOptionsList->AddRow(pRow);

	pRow = m_pLocationsList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _variant_t("<All Locations>"));
	m_pLocationsList->InsertRow(pRow, 0);

	pRow = m_pProvidersList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _variant_t("<All Providers>"));
	m_pProvidersList->InsertRow(pRow, 0);

	pRow = m_pPatCoordList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _variant_t("<All Coordinators>"));
	m_pPatCoordList->InsertRow(pRow, 0);
	

	m_pProvidersList->CurSel = 0;
	m_pDateOptionsList->CurSel = 0;
	
	//grey out the date boxes
	GetDlgItem(IDC_FILTERED_FROM_DATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTERED_TO_DATE)->EnableWindow(FALSE);

	m_pLocationsList->CurSel = 0;
	m_pPatCoordList->CurSel = 0;

	COleDateTime dtNow = COleDateTime::GetCurrentTime();
	dtNow.SetDate(dtNow.GetYear() - 1, dtNow.GetMonth(), dtNow.GetDay());

	m_FromDate.SetValue((_variant_t)dtNow);
	m_ToDate.SetValue((_variant_t)COleDateTime::GetCurrentTime());


	m_brush.CreateSolidBrush(PaletteColor(0x00C8FFFF));

	return TRUE;  
}

void CMarketConvRateGraphDlg::OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateView();	

	*pResult = 0;
}

void CMarketConvRateGraphDlg::OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateView();	

	*pResult = 0;
}

void CMarketConvRateGraphDlg::UpdateView()
{
	try
	{	if (m_referral != "RootNode")
		{	m_graph->Title = (LPCSTR)m_referral;
			m_up.EnableWindow();
		}
		else
		{	m_graph->Title = "All";
			m_up.EnableWindow(FALSE);
		}

		//if (m_revenueRad.GetCheck())
		//	OnRevenueRad();
		//else if (m_numberRad.GetCheck())
		//	OnNumberRad();
		//else if (m_costRad.GetCheck())
		//	OnCostsRad();
		if (m_conversionRad.GetCheck())
			OnConversionRad();
		else if (m_ConvDateRad.GetCheck()) {
			RefreshConversionDateGraph();
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
		else if (m_ProsToSurgByProc.GetCheck()) {
			OnProsToSurgByProc();
		}
		else if (m_InqToConsByStaff.GetCheck()) {
			OnInqToConsByStaff();
		}
		else if (m_ConsToSurgByPatCoord.GetCheck()) {
			OnConsToSurgByPatCoord();
		}

	}
	NxCatchAll("Could not update Marketing Comparison");
}

void CMarketConvRateGraphDlg::OnCloseUpFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	OnChangeFrom(pNMHDR, pResult);	

	*pResult = 0;
}

void CMarketConvRateGraphDlg::OnCloseUpTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnChangeTo(pNMHDR, pResult);		

	*pResult = 0;
}

void CMarketConvRateGraphDlg::OnUp() 
{
	Up();
}

void CMarketConvRateGraphDlg::OnClickColumnGraph(short Row, short Column) 
{
	m_graph->Row = Row;
	for(int i = 0; i < m_arClickableRows.GetSize(); i++) {
		if(m_arClickableRows.GetAt(i) == m_graph->RowID) {
			m_referral = (LPCTSTR)m_graph->RowText;
			UpdateView();
			return;
		}
	}
}

void CMarketConvRateGraphDlg::OnMouseMoveColumnGraph(short Row, short Column) 
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

void CMarketConvRateGraphDlg::OnConversionRad() 
{
	//This is the "Conversion Rate" button

	GraphDescript desc;
	CString sql, from, to, doc, loc, strDateField, strPatCoord;
	int id, ndoc, nloc, nPatCoord;
	CString referrals;

	
	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_CONS_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PAT_COORD_LIST)->EnableWindow(TRUE);
	

	//setup parameters that will be used in the query
	GetParameters(from, to, ndoc, nloc, id, nPatCoord, strDateField);
	if (ndoc != -1)
		doc.Format(" AND PatientsT.MainPhysician = %i ", ndoc);
	if (nloc != -1) //user personT location instead of scheduler location
		loc.Format(" AND PersonT.Location = %i ", nloc);
	if (nPatCoord!= -1) //user personT location instead of scheduler location
		strPatCoord.Format(" AND PatientsT.EmployeeID = %i ", nPatCoord);


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

	sql = GetGraphSql(CNVConsToSurgByRefSour, -1, -1);
	
	CString strFrom, strTo;
	if (! strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s <= '%s' ", strDateField, to);
	}


	
	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov]", doc);
	sql.Replace("[Loc]", loc);
	sql.Replace("[PatCoord]", strPatCoord);


	referrals = "AND " + Descendants(id, "PatientsT.ReferralID");
	sql.Replace("[Ref]", referrals);

	desc.m_sql = sql;
	Save(sql);
	desc.Add("Referral to Consult", "Consults", 0x00FF00, "Referrals", GraphDescript::GD_DIV);
	desc.Add("Referral to Surgery", "Surgeries", 0xFF0000, "Referrals", GraphDescript::GD_DIV);
	
	m_graph->Format = "%0.0f%%";
	Graph(desc);
}



void CMarketConvRateGraphDlg::OnConversionDateRad() 
{
	
		//

	//show the configure button
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_SHOW_ALL_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_CONS_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PAT_COORD_LIST)->EnableWindow(TRUE);
	
	m_pApptPurposeList->Requery();
	m_pYearFilter->Requery();

	if (m_pYearFilter->SetSelByColumn(0, (long)COleDateTime::GetCurrentTime().GetYear()) == -1) {

		m_pYearFilter->CurSel = (m_pYearFilter->GetRowCount() - 1);
	}

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

	m_strPurposeList = GetRemotePropertyText("CRGPurposeList", "", 0, "<None>", TRUE);

	if (m_strPurposeList.IsEmpty()) {

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
	}

	CheckDlgButton(IDC_SHOW_ALL_COLUMNS, TRUE);

	//make sure that we are on top
	m_graph->Title = "All";
	m_up.EnableWindow(FALSE);
		

	RefreshConversionDateGraph();


}

void CMarketConvRateGraphDlg::RefreshConversionDateGraph() {

	try {
	
		//This is the "Conversion Rate By Date" button

		GraphDescript desc;
		CString strSql, from, to, strDoc, strLoc, strDateField, strPatCoord;
		int nID, nProvID, nLocID, nPatCoord;
		
		//setup parameters that will be used in the query
		GetParameters(from, to, nProvID, nLocID, nID, nPatCoord, strDateField);
		if (nProvID!= -1)
			strDoc.Format(" AND PatientsT.MainPhysician = %i ", nProvID);
		if (nLocID != -1) //user personT location instead of scheduler location
			strLoc.Format(" AND PersonT.Location = %i ", nLocID);
		if (nPatCoord!= -1) //user personT location instead of scheduler location
		strPatCoord.Format(" AND PatientsT.EmployeeID = %i ", nPatCoord);


		//figure out what their values are from ConfigRT
		BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);

		CString strConsIDList = GetRemotePropertyText("CRGConsultList", "", 0, "<None>", TRUE);
		CString strSurgIDList = GetRemotePropertyText("CRGSurgeryList", "", 0, "<None>", TRUE);

		CString strBaseSql, strExtraTypeSql;

		CString strYearFilter = AsString(m_pYearFilter->GetValue(m_pYearFilter->CurSel, 0));

		CString strCons1Label, strCons2Label, strTotalLabel, strSurgeryLabel;

		m_strPurposeList = GetRemotePropertyText("CRGPurposeList", "", 0, "<None>", TRUE);

		CString strPurpose;
		if (m_strPurposeList.IsEmpty()) {

			strPurpose = "";
		}
		else {
			strPurpose.Format("AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID IN %s)", m_strPurposeList);
		}

		
		if (!bSplitConsults) {

			//make sure the id list is there so that we don't get any errors
			if (strConsIDList.IsEmpty() || strSurgIDList.IsEmpty()) {

				MessageBox("Please set up your configuration settings by clicking the ... button before running this graph");
				return;
			}


			strBaseSql.Format("SELECT ConsultCount, Month, SurgeryCount, OtherTypeCount, "
				" CASE WHEN (ConsultCount = 0) THEN 0 ELSE CONVERT(int, Round((CONVERT(float, SurgeryCount)/Convert(float, ConsultCount)) * 100, 0)) END AS Cons1Percent "
				"  From ( "
				" SELECT SUM(ConsultCount) as ConsultCount, Month, Sum(SurgeryCount) as SurgeryCount, SUM(OtherTypeCount) as OtherTypeCount "
				" FROM (	"
				" SELECT Count(AppointmentsT.ID) as ConsultCount,  "
				" DATEPART(MM, Date) AS Month, "
				" 0 AS SurgeryCount, 0 as OtherTypeCount "
				" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND AptTypeID IN %s %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [From] [To] [Prov] [Loc] [PatCoord] "
				" GROUP BY DATEPART(MM, Date) "
				" UNION ALL "
				" SELECT 0 AS ConsultCount,  " 
				" DATEPART(MM, Date) AS Month, " 
				" Count(AppointmentsT.ID) AS SurgeryCount, 0 As OtherTypeCount " 
				" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND AptTypeID IN %s  %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [From] [To] [Prov] [Loc] [PatCoord] "
				" GROUP BY DATEPART(MM, Date) ) SubQuery Group By Month ) Sub ", strConsIDList, strPurpose, strYearFilter, strSurgIDList, strPurpose, strYearFilter);  

			CString strFrom, strTo;
			if (! strDateField.IsEmpty()) {
				strFrom.Format(" AND %s >= '%s' ", strDateField, from);
				strTo.Format(" AND %s <= '%s' ", strDateField, to);
			}


			
			strBaseSql.Replace("[From]", strFrom);
			strBaseSql.Replace("[To]", strTo);
			strBaseSql.Replace("[Prov]", strDoc);
			strBaseSql.Replace("[Loc]", strLoc);
			strBaseSql.Replace("[PatCoord]", strPatCoord);


			desc.m_sql = strBaseSql;
			Save(strBaseSql);

			strCons1Label =  GetRemotePropertyText("CRGConsultLabels", "Consults", 0, "<None>", TRUE);
			strSurgeryLabel = GetRemotePropertyText("CRGSurgeryLabel", "Surgeries", 0, "<None>", TRUE);	

			//run the query to get the sum for the year
			//we only have to run one 
			CString strTmp;
			strTmp.Format(" SELECT (SUM(Cons1Percent)/(12) ) AS YEARPERCENT, SUM(Consultcount) as SumConsult, Sum(SurgeryCount) as SumSurgery FROM ( %s ) BASE ", strBaseSql);
			_RecordsetPtr rsYearPercent = CreateRecordsetStd(strTmp);

			CString strYearPercent, strSumConsult, strSumSurgery;
			strYearPercent.Format(" (%li%% Conversion Rate for %s)", AdoFldLong(rsYearPercent, "YearPercent", 0), strYearFilter);
			strSumConsult.Format(" (%li Total %s For %s)", AdoFldLong(rsYearPercent, "SumConsult", 0), strCons1Label, strYearFilter);
			strSumSurgery.Format(" (%li Total %s For %s)", AdoFldLong(rsYearPercent, "SumSurgery", 0), strSurgeryLabel, strYearFilter);


			if (IsDlgButtonChecked(IDC_SHOW_NUMBERS_ONLY)) {

				desc.Add(strCons1Label + strSumConsult, "ConsultCount", 0x0000FF, "ConsultCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0xFF0000, "SurgeryCount");
			}
			else if (IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY)) {
				
				desc.Add("Conversion Rate" + strYearPercent, "Cons1Percent", 0x0000FF, "Cons1Percent");

			}
			else {

				desc.Add(strCons1Label + strSumConsult, "ConsultCount", 0x00FF00, "ConsultCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0xFF0000, "SurgeryCount");
				desc.Add("Conversion Rate" + strYearPercent, "Cons1Percent", 0x0000FF, "Cons1Percent");

			}
			

			

			//now we have to graph these for each month...hmmm this should be interesting...
	//		m_graph.RowCount = 12;


		
		}
		else {

			//there are two sets of consult appts, so we need to add on a query, I suppose
			CString strCons1Tmp, strCons2Tmp;

			long nResult = strConsIDList.Find("---");
			strCons1Tmp = strConsIDList.Left(nResult);

			strCons2Tmp = strConsIDList.Right(strConsIDList.GetLength() - (nResult + 3));

			//make sure the id list is there so that we don't get any errors
			if (strCons1Tmp.IsEmpty() || strCons2Tmp.IsEmpty() || strSurgIDList.IsEmpty()) {

				MessageBox("Please set up your configuration settings by clicking the ... button before running this graph");
				return;
			}


			strBaseSql.Format(" SELECT ConsultCount, Month, SurgeryCount, OtherTypeCount, TotalConsultCount, "
				" CASE WHEN ConsultCount = 0 THEN 0 ELSE CONVERT(int, Round((CONVERT(float, SurgeryCount)/Convert(float, ConsultCount)) * 100, 0)) END AS Cons1Percent,  "
				" CASE WHEN OtherTypeCount = 0 THEN 0 ELSE CONVERT(int, Round((CONVERT(float, SurgeryCount)/CONVERT(float, OtherTypeCount)) * 100, 0)) END AS Cons2Percent,  "
				" CASE WHEN TotalConsultCount = 0 THEN 0 ELSE CONVERT(int, Round((CONVERT(float, SurgeryCount)/CONVERT(float, TotalConsultCount)) * 100, 0)) END AS TotalPercent "
				" FROM ( "
				" SELECT Sum(ConsultCount) AS ConsultCount, Month, Sum(SurgeryCount) AS SurgeryCount, Sum(OtherTypeCount) as OtherTypeCount, Sum(ConsultCount + OtherTypeCount) As TotalConsultCount "
				" FROM ( "
				"SELECT Count(AppointmentsT.ID) as ConsultCount,  "
				" DATEPART(MM, Date) AS Month, "
				" 0 AS SurgeryCount, 0 as OtherTypeCount "
				" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND  AptTypeID IN %s %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [From] [To] [Prov] [Loc] [PatCoord]"
				" Group BY DATEPART(MM, Date) "
				" UNION ALL "
				" SELECT 0 AS ConsultCount,  " 
				" DATEPART(MM, Date) AS Month, " 
				" Count(AppointmentsT.ID) AS SurgeryCount, 0 As OtherTypeCount " 
				" FROM AppointmentsT "
				" LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND  AptTypeID IN %s %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [From] [To] [Prov] [Loc] [PatCoord] "
				" Group BY DATEPART(MM, Date) "
				" UNION ALL "
				" SELECT 0 as ConsultCount,  "
				" DATEPART(MM, Date) AS Month, "
				" 0 AS SurgeryCount, Count(AppointmentsT.ID) As OtherTypeCount "
				" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND  AptTypeID IN %s %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [From] [To] [Prov] [Loc] [PatCoord] "
				" Group BY DATEPART(MM, Date) ) SubQuery Group By Month) Sub2", strCons1Tmp, strPurpose, strYearFilter, strSurgIDList, strPurpose, strYearFilter, strCons2Tmp, strPurpose, strYearFilter);

			CString strFrom, strTo;
			if (! strDateField.IsEmpty()) {
				strFrom.Format(" AND %s >= '%s' ", strDateField, from);
				strTo.Format(" AND %s <= '%s' ", strDateField, to);
			}


			
			strBaseSql.Replace("[From]", strFrom);
			strBaseSql.Replace("[To]", strTo);
			strBaseSql.Replace("[Prov]", strDoc);
			strBaseSql.Replace("[Loc]", strLoc);
			strBaseSql.Replace("[PatCoord]", strPatCoord);


			CString strLabelTmp = GetRemotePropertyText("CRGConsultLabels", "Consults", 0, "<None>", TRUE);

			nResult = strLabelTmp.Find("---");

			strCons1Label = strLabelTmp.Left(nResult);
			strCons2Label = strLabelTmp.Right(strLabelTmp.GetLength() - (nResult + 3));

			strTotalLabel = "Total Consults";

			strSurgeryLabel = GetRemotePropertyText("CRGSurgeryLabel", "Surgeries", 0, "<None>", TRUE);	

			desc.m_sql = strBaseSql;
			Save(strBaseSql);


			//run the query to get the sum for the year
			//we only have to run one 
			CString strTmp;
			strTmp.Format(" SELECT (SUM(Cons1Percent)/(12) ) AS Cons1PERCENT, SUM(Consultcount) as SumCons1, Sum(SurgeryCount) as SumSurgery,  "
				" (SUM(Cons2Percent)/(12)) AS Cons2Percent, (SUM(TotalPercent)/(12)) AS TotalPercent, Sum(OtherTypeCount) AS SumCons2, Sum(TotalConsultCount) AS SumTotal " 
				" FROM ( %s ) BASE ", strBaseSql);
			_RecordsetPtr rsYearPercent = CreateRecordsetStd(strTmp);

			CString strCons1Percent, strCons2Percent, strTotalPercent, strSumCons1, strSumCons2, strSumSurgery, strSumTotal;
			strCons1Percent.Format(" (%li%% Conversion Rate for %s)", AdoFldLong(rsYearPercent, "Cons1Percent", 0), strYearFilter);
			strCons2Percent.Format(" (%li%% Conversion Rate for %s)", AdoFldLong(rsYearPercent, "Cons2Percent", 0), strYearFilter);
			strTotalPercent.Format(" (%li%% Conversion Rate for %s)", AdoFldLong(rsYearPercent, "TotalPercent", 0), strYearFilter);
			strSumCons1.Format(" (%li Total %s For %s)", AdoFldLong(rsYearPercent, "SumCons1", 0), strCons1Label, strYearFilter);
			strSumCons2.Format(" (%li Total %s For %s)", AdoFldLong(rsYearPercent, "SumCons2", 0), strCons1Label, strYearFilter);
			strSumTotal.Format(" (%li Total %s For %s)", AdoFldLong(rsYearPercent, "SumTotal", 0), strCons1Label, strYearFilter);
			strSumSurgery.Format(" (%li Total %s For %s)", AdoFldLong(rsYearPercent, "SumSurgery", 0), strSurgeryLabel, strYearFilter);
						



			if (IsDlgButtonChecked(IDC_SHOW_NUMBERS_ONLY)) {

				desc.Add(strCons1Label + strSumCons1, "ConsultCount", 0x00FF00, "ConsultCount");
				desc.Add(strCons2Label + strSumCons2, "OtherTypeCount", 0x00FFFF, "OtherTypeCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0xFF0000, "SurgeryCount");
				desc.Add("Total Consults" + strSumTotal, "TotalConsultCount", 0xFF00FF, "TotalConsultCount");
			}
			else if (IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY)) {
				
				desc.Add(strCons1Label + " Conversion Rate" + strCons1Percent, "Cons1Percent", 0x0000FF, "Cons1Percent");
				desc.Add(strCons2Label + " Conversion Rate" + strCons2Percent, "Cons2Percent", 0x00FFFF, "Cons2Percent");
				desc.Add("Total Conversion Rate" + strTotalPercent, "TotalPercent", 0xFF00FF, "TotalPercent");
			}
			else {

				desc.Add(strCons1Label + strSumCons1, "ConsultCount", 0x00FF00, "ConsultCount");
				desc.Add(strCons2Label + strSumCons2, "OtherTypeCount", 0x00FFFF, "OtherTypeCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0xFF0000, "SurgeryCount");
				desc.Add("Total Consults" + strSumTotal, "TotalConsultCount", 0xFF00FF, "TotalConsultCount");
				desc.Add(strCons1Label + " Conversion Rate" + strCons1Percent, "Cons1Percent", 0x0000FF, "Cons1Percent");
				desc.Add(strCons2Label + " Conversion Rate" + strCons2Percent, "Cons2Percent", 0x00FFFF, "Cons2Percent");
				desc.Add("Total Conversion Rate" + strTotalPercent, "TotalPercent", 0x0000FF, "TotalPercent");


			}
			
			//desc.Add(strCons1Label + " Conversion Rate", "ConsultCount", 0xFF8000, "SurgeryCount", GraphDescript::GD_DIV);
			//desc.Add(strCons2Label + " Conversion Rate", "OtherTypeCount", 0x000040, "SurgeryCount", GraphDescript::GD_DIV);
			//desc.Add("Total Conversion Rate", "TotalConsultCount", 0xFF00FF, "SurgeryCount", GraphDescript::GD_DIV);


		}

		//MessageBox(strBaseSql);

		
		m_graph->Format = "%0.0f";
		

		//now, we just have to loop through the 12 months and then set each column
		_RecordsetPtr		rs = NULL;
			int				i, 
							j, 
							k;
			double			total, total2;
			CString			sql;
			CArray<int,int>	descendants;
			CWaitCursor		wait;
			long nCurrentID;

		
		long nMonthCount = 12;

		m_graph->RowCount = nMonthCount;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(25);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
		}

		//JMJ 4/2/2004 - The graph auto-reverts the sort to the first column
		//whenever the column descriptions change, so normally we don't need
		//to identify if we're loading a new graph and revert the sort, etc.
		//But in this case, the default sort is the X-Axis, so we have to tell
		//the graph to change the sort, but ONLY if we're loading the "By Month"
		//graph for the first time!

		BSTR strDesc = m_graph->XAxisDesc;
		CString strTest = CString(strDesc);
		if(strTest != "Month") {
			//sort by the x-axis because we are just now switching to this graph type
			m_graph->SortColumn = -1;
			m_graph->XAxisDesc = "Month";
		}
		m_graph->XAxisSortStyle = cgSortID;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, strBaseSql);

		if (rs->eof) {
			m_progress.SetPos(100);
			m_progress.SetPos(0);
			return;
		}

		m_progress.SetPos(50);
		
		//all the real work is done, but we still have to sort the results
		i = 0;
		for (int h = 0; h < nMonthCount; h++) {
		
			//set chart names
			m_graph->Row = i++;
			m_graph->RowText = (LPCTSTR) _bstr_t(GetMonth(h + 1));
			m_graph->RowID = h+1;

			//we have no descendants
			descendants.RemoveAll();

			nCurrentID = rs->Fields->GetItem("Month")->Value.lVal;
			descendants.Add(h + 1);

			
			//for each column
			for (j = 0; j < desc.Size(); j++) {
			
				m_graph->Column = j;
				//Reset the total
				total = 0;
				total2 = 0;
				if (!rs->eof)
					for (k = 0; k < descendants.GetSize(); k++) //for each record
					{	while (!rs->eof)
						{	if (descendants[k] == AdoFldLong(rs, "Month"))
							{	total += AdoFldLong(rs, desc.Field(j));
								if (desc.Op(j) == GraphDescript::GD_DIV)
									total2 += AdoFldLong(rs, desc.Field2(j));
							}							
							rs->MoveNext();
						}
						rs->MoveFirst();
					}
				switch (desc.Op(j))
				{
					case GraphDescript::GD_ADD:
						m_graph->Value = total;
						break;
					case GraphDescript::GD_DIV:
						if (total2 != 0)
							m_graph->Value = total / total2;
						else m_graph->Value = 0;
						break;
					default:
						ASSERT(FALSE);//things I haven't handeled yet, or bad values
				}
			}
			m_progress.SetPos(50 + 50 * i / nMonthCount);
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);

	}NxCatchAll("Error loading graph");


}


void CMarketConvRateGraphDlg::OnConfigureApptTypes() 
{
	CConversionRateByDateConfigDlg dlg;
	dlg.DoModal();
	RefreshConversionDateGraph();
}

void CMarketConvRateGraphDlg::OnSelChosenApptPurposeList(long nRow) 
{

	long nID = VarLong(m_pApptPurposeList->GetValue(nRow, 0));

	if (nID == -2) {

		//they want multipurposes
		SelectMultiPurposes();
	}
	else if (nID == -3) {
		m_strPurposeList = "";
	}
	else {

		m_strPurposeList.Format("(%li)", nID);
	}
		
	SetRemotePropertyText("CRGPurposeList", m_strPurposeList, 0, "<None>");
	RefreshConversionDateGraph();
	
	
}




void CMarketConvRateGraphDlg::CheckDataList(CMultiSelectDlg *dlg) {

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


}

void CMarketConvRateGraphDlg::SelectMultiPurposes() {

	CString strFrom, strWhere;
	CMultiSelectDlg dlg;
	HRESULT hRes;
	bool bDontFill = false;

	long nResult = 0;
	CString strChecks, strChecksTmp;

	strChecks = m_strPurposeList;

	
		
	// Fill the dialog with existing selections
	CheckDataList(&dlg);
	
	dlg.m_strNameColTitle = "Purpose";

		
	strFrom = "AptPurposeT ";

	hRes = dlg.Open(strFrom, strWhere, "AptPurposeT.ID", "AptPurposeT.Name", "Please select the Purposes you want to see.");
	

	// Update our array of procedures with this information
	if (hRes == IDOK)
	{
		m_strPurposeList = "(" + dlg.GetMultiSelectIDString(",") + ")";
	}
	
}

void CMarketConvRateGraphDlg::OnSelChosenYearFilter(long nRow) 
{
	RefreshConversionDateGraph();	
}

void CMarketConvRateGraphDlg::OnShowAllColumns() 
{
	RefreshConversionDateGraph();	
	
}

void CMarketConvRateGraphDlg::OnShowNumbersOnly() 
{
	RefreshConversionDateGraph();	
	
}

void CMarketConvRateGraphDlg::OnShowPercentagesOnly() 
{
	RefreshConversionDateGraph();	
	
}

void CMarketConvRateGraphDlg::Refresh() {

	//MessageBox("Hi");
}

void CMarketConvRateGraphDlg::OnConsToSurgByPatCoord() 
{
	GraphDescript desc;
	CString sql, from, to, doc, loc, strDateField, strPatCoord;
	int id, ndoc, nloc, nPatCoord;
	CString referrals;

	//

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_CONS_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PAT_COORD_LIST)->EnableWindow(TRUE);
	

	//setup parameters that will be used in the query
	GetParameters(from, to, ndoc, nloc, id, nPatCoord, strDateField);
	if (ndoc != -1)
		doc.Format(" AND PatientsT.MainPhysician = %i ", ndoc);
	if (nloc != -1) //user personT location instead of scheduler location
		loc.Format(" AND PersonT.Location = %i ", nloc);
	if (nPatCoord!= -1) //user personT location instead of scheduler location
		strPatCoord.Format(" AND PatientsT.EmployeeID = %i ", nPatCoord);


	sql = GetGraphSql(CNVConsToSurgByPatCoord, -1, -1);

	CString strFrom, strTo;
	if (! strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s <= '%s' ", strDateField, to);
	}


	
	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov]", doc);
	sql.Replace("[Loc]", loc);
	sql.Replace("[PatCoord]", strPatCoord);


	//referrals = "AND " + Descendants(id, "PatientsT.EmployeeID");
	//sql.Replace("[ref]", referrals);

	desc.m_sql = sql;
	Save(sql);
	desc.Add("Number of Consults", "Consults", 0x00FF00, "", GraphDescript::GD_ADD);
	desc.Add("Number of Surgeries", "Surgeries", 0xFF0000, "", GraphDescript::GD_ADD);
	desc.Add("Consult to Surgery Percentage", "ConvRate", 0x0000FF, "", GraphDescript::GD_ADD);
	
	m_graph->Format = "%0.0f";
	GraphPatientCoordinators(desc);


	
	
}

void CMarketConvRateGraphDlg::OnInqToConsByProc() 
{
	GraphDescript desc;
	CString sql, from, to, doc, loc, strPatCoord = "", strDateField;
	int id, ndoc, nloc, nPatCoord;
	CString referrals;

		//

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_PROSPECTS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_CONS_PROSPECTS)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_PAT_COORD_LIST)->EnableWindow(FALSE);
	

	//setup parameters that will be used in the query
	GetParameters(from, to, ndoc, nloc, id, nPatCoord, strDateField);
	if (ndoc != -1)
		doc.Format(" AND PatientsT.MainPhysician = %i ", ndoc);
	if (nloc != -1) //user personT location instead of scheduler location
		loc.Format(" AND PersonT.Location = %i ", nloc);
	if (nPatCoord!= -1) //user personT location instead of scheduler location
		strPatCoord.Format(" AND PatientsT.EmployeeID = %i ", nPatCoord);


	sql = GetGraphSql(CNVInqToConsByProc, -1, -1);

	CString strFrom, strTo;
	if (! strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s <= '%s' ", strDateField, to);
	}

	
	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov]", doc);
	sql.Replace("[Loc]", loc);
	sql.Replace("[PatCoord]", strPatCoord);

	
	desc.m_sql = sql;
	Save(sql);
	
	desc.Add("Number of Inquiries", "NumInquiries", 0xFF0000, "", GraphDescript::GD_ADD);
	desc.Add("Number of Consults", "ConsCount", 0x00FF00, "", GraphDescript::GD_ADD);
	desc.Add("Inquiry to Consults Conversion Rate", "ConvRate", 0x0000FF, "", GraphDescript::GD_ADD);
				
	m_graph->Format = "%0.0f";
	GraphProcedures(desc);
	
}

void CMarketConvRateGraphDlg::OnInqToConsByStaff() 
{
	//MessageBox("Hi");
	
}

void CMarketConvRateGraphDlg::OnProsToConsByProc() 
{
	GraphDescript desc;
	CString sql, from, to, doc, loc, strDateField, strPatCoord;
	int id, ndoc, nloc, nPatCoord;
	CString referrals;

	//

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_PROSPECTS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_CONS_PROSPECTS)->ShowWindow(SW_SHOW);
	
	GetDlgItem(IDC_PAT_COORD_LIST)->EnableWindow(FALSE);

	//setup parameters that will be used in the query
	GetParameters(from, to, ndoc, nloc, id, nPatCoord, strDateField);
	if (ndoc != -1)
		doc.Format(" AND PatientsT.MainPhysician = %i ", ndoc);
	if (nloc != -1) //user personT location instead of scheduler location
		loc.Format(" AND PersonT.Location = %i ", nloc);
	if (nPatCoord!= -1) //user personT location instead of scheduler location
		strPatCoord.Format(" AND PatientsT.EmployeeID = %i ", nPatCoord);

	sql = GetGraphSql(CNVProsToConsByProc, -1, -1);

	CString strFrom, strTo;
	if (! strDateField.IsEmpty()) {
		strFrom.Format(" AND %s >= '%s' ", strDateField, from);
		strTo.Format(" AND %s <= '%s' ", strDateField, to);
	}

	
	sql.Replace("[From]", strFrom);
	sql.Replace("[To]", strTo);
	sql.Replace("[Prov]", doc);
	sql.Replace("[Loc]", loc);
	sql.Replace("[PatCoord]", strPatCoord);

	desc.m_sql = sql;
	Save(sql);
	
	if (IsDlgButtonChecked(IDC_SHOW_ALL_PROSPECTS)) {
		desc.Add("Prospects to Consults Percentage", "Consults", 0x0000FF, "ProspectCount", GraphDescript::GD_DIV);
	}
	else {
		desc.Add("Prospects to Consults Percentage", "Consults", 0x0000FF, "ConsProsCount", GraphDescript::GD_DIV);
	}

	
	m_graph->Format = "%0.0f%%";
	GraphProcedures(desc);

	
	
}

void CMarketConvRateGraphDlg::OnProsToSurgByProc() 
{
	// TODO: Add your control notification handler code here
	
}


void CMarketConvRateGraphDlg::GraphPatientCoordinators(GraphDescript &desc)
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
			"WHERE PatientCoordinator <> 0 ORDER BY Name;");

		rsPatCoord = CreateRecordset(adOpenStatic, adLockReadOnly, sql);

		max = rsPatCoord->RecordCount;
		m_graph->RowCount = max;
		m_arClickableRows.RemoveAll();
		m_progress.SetPos(25);

		//setup graph
		m_graph->ColumnCount = desc.Size();

		for (i = 0; i < desc.Size(); i++)
		{	m_graph->Column = i;
			m_graph->Color = desc.Color(i);
			m_graph->ColumnText = (_bstr_t)desc.Label(i);
		}

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
			if (currentID != id)//special [Other] column
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
						{	if (descendants[k] == AdoFldLong(rs, "EmployeeID"))
							{	total += AdoFldDouble(rs, desc.Field(j));
								if (desc.Op(j) == GraphDescript::GD_DIV)
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
						break;
					case GraphDescript::GD_DIV:
						if (total2 != 0)
							m_graph->Value = total / total2;
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


void CMarketConvRateGraphDlg::GraphProcedures(GraphDescript &desc)
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
						k,
						max;
		double			total, total2;
		CString			sql;
		CArray<int,int>	descendants;
		CWaitCursor		wait;

		m_progress.SetPos(0);

		sql.Format ("SELECT AptPurposeT.Name AS Name, "
			"ID as PurposeID FROM AptPurposeT ORDER BY Name;");

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
		}

		m_graph->XAxisDesc = "Procedure";
		m_graph->XAxisSortStyle = cgSortAlpha;
		m_graph->AllowXAxisSort = TRUE;

		rs = CreateRecordset(adOpenStatic, adLockReadOnly, desc.m_sql);

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
			if (currentID != id)//special [Other] column
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
						{	if (descendants[k] == AdoFldLong(rs, "PurposeID", -1))
							{	total += AdoFldDouble(rs, desc.Field(j));
								if (desc.Op(j) == GraphDescript::GD_DIV)
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
						break;
					case GraphDescript::GD_DIV:
						if (total2 != 0)
							m_graph->Value = total / total2;
						else m_graph->Value = 0;
						break;
					default:
						ASSERT(FALSE);//things I haven't handeled yet, or bad values
				}
			}

			rsPurpose->MoveNext();
			m_progress.SetPos(50 + 50 * i / max);
		}

		//will sort by whatever its current sort column is
		m_graph->SortGraph();

		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}


void CMarketConvRateGraphDlg::OnShowConsProspects() 
{
	OnProsToConsByProc();
	
}

void CMarketConvRateGraphDlg::OnShowAllProspects() 
{
	OnProsToConsByProc();
	
}

void CMarketConvRateGraphDlg::GetParameters(CString &from, CString &to, int &prov, int &loc, int &id, int &nPatCoord, CString &strDateField)
{
	prov = VarLong(m_pProvidersList->GetValue(m_pProvidersList->CurSel, 0));
	loc = VarLong(m_pLocationsList->GetValue(m_pLocationsList->CurSel, 0));
	from = FormatDateTimeForSql(VarDateTime(m_FromDate.GetValue()), dtoDate);
	to = FormatDateTimeForSql(VarDateTime(m_ToDate.GetValue()), dtoDate);
	id = GetCurrentID();
	nPatCoord = VarLong(m_pPatCoordList->GetValue(m_pPatCoordList->CurSel, 0));

	long nDateType = VarLong(m_pDateOptionsList->GetValue(m_pDateOptionsList->CurSel, 0));
	if (nDateType == 1) {
		//First Appt Date
		strDateField = "AppointmentsT.Date";
	}
	else if (nDateType == 2) {
		strDateField = "PersonT.FirstContactDate";
	}
	else {
		strDateField = "";
	}

}

void CMarketConvRateGraphDlg::OnSelChosenDateOptionList(long nRow) 
{
	if (VarLong(m_pDateOptionsList->GetValue(nRow, 0)) == -1) {
		GetDlgItem(IDC_FILTERED_FROM_DATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_FILTERED_TO_DATE)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_FILTERED_FROM_DATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_FILTERED_TO_DATE)->EnableWindow(TRUE);
	}
		
	UpdateView();
	
}

void CMarketConvRateGraphDlg::OnChangeFilteredFromDate() 
{
	UpdateView();
	
}

void CMarketConvRateGraphDlg::OnSelChosenFilteredLocationList(long nRow) 
{
	UpdateView();
	
}

void CMarketConvRateGraphDlg::OnSelChosenFilteredProviderList(long nRow) 
{
	UpdateView();
	
}

void CMarketConvRateGraphDlg::OnChangeFilteredToDate() 
{
	UpdateView();
	
}

void CMarketConvRateGraphDlg::OnSelChosenPatCoordList(long nRow) 
{
	UpdateView();
	
}

HBRUSH CMarketConvRateGraphDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-05-22 12:44) - PLID 29497 - Use NxDialog base class
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())  {
		case IDC_STATIC1:
		case IDC_STATIC2:
		case IDC_STATIC3:
	
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00C8FFFF));
			return m_brush;
		break;
		default:
		break;
	}

	return hbr;
}

*/
