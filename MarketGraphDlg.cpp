#include "stdafx.h"
#include "GraphDescript.h"
// CMarketGraphDlg.cpp : implementation file
/*
Add so far

Number of Patients
Fees Billed/Collected/Pending
Marketing Costs

to add later 

Cost Per Patient
% on Marketing Costs
*/

#include "marketUtils.h"
#include "MarketGraphDlg.h"
#include <winuser.h>
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "ConversionRateByDateConfigDlg.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace ColumnGraph;
using namespace NxTab;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////
// (b.cardillo 2004-08-05 18:19) - PLID 13747 - Various utility classes used by 
// several of the marketing tabs' graphs for high-performance functionality.
/////////////////////////////////////////////////////////////////

CMapLongToGraphValuesArray::CMapLongToGraphValuesArray()
{
}

CMapLongToGraphValuesArray::~CMapLongToGraphValuesArray()
{
	// Each element in the map, is a pointer to an array of m_nArrayDepth CGraphValues objects, so deallocate them
	for (POSITION p = GetStartPosition(); p; ) {
		long n;
		CGraphValues *paryRefVals;
		GetNextAssoc(p, n, paryRefVals);
		delete []paryRefVals;
	}
	// Finally remove all the references
	RemoveAll();
}

// Various handy ways of calling ScanRecordsetIntoMap()
void CMapLongToGraphValuesArray::ScanRecordsetIntoMap(IN _RecordsetPtr rs, IN const CString &strIDFieldName, IN const GraphDescript &desc)
{
	ScanRecordsetIntoMap(rs, strIDFieldName, FALSE, 0, desc);
}

void CMapLongToGraphValuesArray::ScanRecordsetIntoMap(IN _RecordsetPtr rs, IN const CString &strIDFieldName, IN long nIDDefault, IN const GraphDescript &desc)
{
	ScanRecordsetIntoMap(rs, strIDFieldName, TRUE, nIDDefault, desc);
}

// Takes a recordset, the recordset's id field name, and a graph descriptor, and adds all the 
// records' desc.Field and desc.Field2 values into the appropriate spot in an array of desc.Size() 
// CGraphValues objects based on the strIDFieldName ID for each record.
void CMapLongToGraphValuesArray::ScanRecordsetIntoMap(IN _RecordsetPtr rs, IN const CString &strIDFieldName, IN BOOL bUseDefaultIDForNULL, IN long nIDDefault, IN const GraphDescript &desc)
{
	if (!rs->bof || !rs->eof) {
		if (!rs->eof) rs->MoveFirst();
		double dbl = 0;
		FieldsPtr pflds = rs->GetFields();
		FieldPtr fldID = pflds->GetItem((LPCTSTR)strIDFieldName);
		while (!rs->eof) {
			long nReferralID;
			if (bUseDefaultIDForNULL) {
				nReferralID = AdoFldLong(fldID, nIDDefault);
			} else {
				nReferralID = AdoFldLong(fldID);
			}
			CGraphValues *aryRefVals;
			if (!Lookup(nReferralID, aryRefVals)) {
				// Didn't already exist so add it and sinze its a byte
				aryRefVals = new CGraphValues[desc.Size()];
				//memset(aryRefVals, 0, sizeof(CGraphValues) * desc.Size());
				SetAt(nReferralID, aryRefVals);
			}
			// Now add the values into the appropriate spots in the array
			for (long i=0; i<desc.Size(); i++) {
				aryRefVals[i].m_dblFieldValue += AdoFldDouble(pflds, desc.Field(i));
				CString strField2 = desc.Field2(i);
				if (!strField2.IsEmpty()) {
					aryRefVals[i].m_dblField2Value += AdoFldDouble(pflds, strField2);
				}
			}
			rs->MoveNext();
		}
		rs->MoveFirst();
	}
}

CED_GraphCalcColumnTotals_Info::CED_GraphCalcColumnTotals_Info(_RecordsetPtr prs, const CString &strIDFieldName, const GraphDescript &gdDescript, long nDescriptIndex, double dblTotal, double dblTotal2)
	: m_pmapp(NULL), m_rs(prs), m_gdDescript(gdDescript), m_nDescriptIndex(nDescriptIndex), m_dblTotal(dblTotal), m_dblTotal2(dblTotal2) 
{
}

CED_GraphCalcColumnTotals_Info::CED_GraphCalcColumnTotals_Info(CMapLongToGraphValuesArray *pmapp, const GraphDescript &gdDescript, long nDescriptIndex, double dblTotal, double dblTotal2)
	: m_pmapp(pmapp), m_rs(NULL), m_gdDescript(gdDescript), m_nDescriptIndex(nDescriptIndex), m_dblTotal(dblTotal), m_dblTotal2(dblTotal2) 
{
}

BOOL CED_GraphCalcColumnTotals_Info::ProcessGraphValues(long nID)
{
	if (m_pmapp) {
		// Get the value for this id
		CGraphValues *pVals;
		if (m_pmapp->Lookup(nID, pVals)) {
			// Add the total value from the given field
			m_dblTotal += pVals[m_nDescriptIndex].m_dblFieldValue;
			// Optionally add the second total for the division operation
			//(e.lally 2009-09-16) PLID 35559 - Added support for percent operations, follows same logic as division
			if (m_gdDescript.Op(m_nDescriptIndex) == GraphDescript::GD_DIV ||
				m_gdDescript.Op(m_nDescriptIndex) == GraphDescript::GD_PERCENT) {
				m_dblTotal2 += pVals[m_nDescriptIndex].m_dblField2Value;
			}
			// Tell the enumerator to keep looping
			return TRUE;
		} else {
			// No entry for it, that just means both fields are 0, so don't add anything to the totals.
			return TRUE;
		}
	} else {
		// Filter the recordset on just the ids we're looking for
		CString strFilter;
		strFilter.Format(_T(m_strIDFieldName + " = %li"), nID);
		m_rs->PutFilter((LPCTSTR)strFilter);
		// Loop through those
		while (!m_rs->eof) {
			// Add the total value from the given field
			m_dblTotal += AdoFldDouble(m_rs, m_gdDescript.Field(m_nDescriptIndex));
			// Optionally add the second total for the division operation
			//(e.lally 2009-09-16) PLID 35559 - Added support for percent operations, follows same logic as division
			if (m_gdDescript.Op(m_nDescriptIndex) == GraphDescript::GD_DIV ||
				m_gdDescript.Op(m_nDescriptIndex) == GraphDescript::GD_PERCENT) {
				m_dblTotal2 += AdoFldDouble(m_rs, m_gdDescript.Field2(m_nDescriptIndex));
			}
			// Move to the next record
			m_rs->MoveNext();
		}
		// Remove the filter
		m_rs->PutFilter("");

		// Tell the enumerator to keep looping
		return TRUE;
	}
}

BOOL CALLBACK CED_GraphCalcColumnTotals_Info::CallbackProcessGraphValues(long nID, LPVOID pParam)
{
	CED_GraphCalcColumnTotals_Info *pInfo = (CED_GraphCalcColumnTotals_Info *)pParam;
	return pInfo->ProcessGraphValues(nID);
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////




// (c.haag 2008-05-15 13:06) - PLID 30068 - This dialog has been deprecated
/*
/////////////////////////////////////////////////////////////////////////////
// CMarketGraphDlg dialog

CMarketGraphDlg::~CMarketGraphDlg()
{
	if (m_oldCursor)
		SetCursor(m_oldCursor);
	DestroyCursor(m_cursor);
}

CMarketGraphDlg::CMarketGraphDlg(CWnd* pParent)
	: CNxDialog(CMarketGraphDlg::IDD, pParent)
{
	EnableAutomation();

	//{{AFX_DATA_INIT(CMarketGraphDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Marketing/Tabs/compare.htm";
	initialized = false;
}

void CMarketGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketGraphDlg)
	DDX_Control(pDX, IDC_UP, m_up);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_NUMBER_RAD, m_numberRad);
	DDX_Control(pDX, IDC_CONVERSION_RAD, m_conversionRad);
	DDX_Control(pDX, IDC_COSTS_RAD, m_costRad);
	DDX_Control(pDX, IDC_REVENUE_RAD, m_revenueRad);
	DDX_Control(pDX, IDC_CONVERSION_DATE_RAD, m_ConvDateRad);
	DDX_Control(pDX, IDC_SHOW_ALL_COLUMNS, m_ShowAllRad);
	DDX_Control(pDX, IDC_SHOW_NUMBERS_ONLY, m_ShowNumberRad);
	DDX_Control(pDX, IDC_SHOW_PERCENTAGES_ONLY, m_ShowPercentRad);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMarketGraphDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketGraphDlg)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_CONVERSION_RAD, OnConversionRad)
	ON_BN_CLICKED(IDC_COSTS_RAD, OnCostsRad)
	ON_BN_CLICKED(IDC_REVENUE_RAD, OnRevenueRad)
	ON_BN_CLICKED(IDC_NUMBER_RAD, OnNumberRad)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_COMPLETED_ONLY, OnCompletedOnly)
	ON_BN_CLICKED(IDC_CONVERSION_DATE_RAD, OnConversionDateRad)
	ON_BN_CLICKED(IDC_CONFIGURE_APPT_TYPES, OnConfigureApptTypes)
	ON_BN_CLICKED(IDC_SHOW_ALL_COLUMNS, OnShowAllColumns)
	ON_BN_CLICKED(IDC_SHOW_NUMBERS_ONLY, OnShowNumbersOnly)
	ON_BN_CLICKED(IDC_SHOW_PERCENTAGES_ONLY, OnShowPercentagesOnly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CMarketGraphDlg, CNxDialog)
	//{{AFX_DISPATCH_MAP(CMarketGraphDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketGraphDlg message handlers

BEGIN_EVENTSINK_MAP(CMarketGraphDlg, CNxDialog)*/
    //{{AFX_EVENTSINK_MAP(CMarketGraphDlg)
	//ON_EVENT(CMarketGraphDlg, IDC_FROM, 3 /* Change */, OnChangeFrom, VTS_NONE)
	//ON_EVENT(CMarketGraphDlg, IDC_TO, 3 /* Change */, OnChangeTo, VTS_NONE)
	//ON_EVENT(CMarketGraphDlg, IDC_FROM, 1 /* CloseUp */, OnCloseUpFrom, VTS_NONE)
	//ON_EVENT(CMarketGraphDlg, IDC_TO, 1 /* CloseUp */, OnCloseUpTo, VTS_NONE)
	//ON_EVENT(CMarketGraphDlg, IDC_GRAPH, 1 /* OnClickColumn */, OnClickColumnGraph, VTS_I2 VTS_I2)
	//ON_EVENT(CMarketGraphDlg, IDC_GRAPH, 2 /* OnMouseMoveColumn */, OnMouseMoveColumnGraph, VTS_I2 VTS_I2)
	//ON_EVENT(CMarketGraphDlg, IDC_APPT_PURPOSE_LIST, 16 /* SelChosen */, OnSelChosenApptPurposeList, VTS_I4)
	//ON_EVENT(CMarketGraphDlg, IDC_YEAR_FILTER, 16 /* SelChosen */, OnSelChosenYearFilter, VTS_I4)
/*	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMarketGraphDlg::Up() 
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

int CMarketGraphDlg::GetCurrentID()
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

void CMarketGraphDlg::Graph(GraphDescript &desc)
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
		m_progress.SetPos(0);
	}
	NxCatchAll("Could not calculate graph");
}

void CMarketGraphDlg::GetParameters(CString &from, CString &to, int &prov, int &loc, int &id)
{
	prov = GetDoc();
	loc = GetLoc();
	GetDateTextFrom(from);
	GetDateTextTo(to);
	id = GetCurrentID();
}

BOOL CMarketGraphDlg::OnInitDialog() 
{
	// (a.walling 2007-11-06 16:08) - PLID 27800 - VS2008 - More namespace craziness! Ensure this is the one we are looking for.
	ColumnGraph::FontPtr font;

	CNxDialog::OnInitDialog();

	m_conversionRad.SetColor(0x00C8FFFF);
	m_costRad.SetColor(0x00C8FFFF);
	m_numberRad.SetColor(0x00C8FFFF);
	m_revenueRad.SetColor(0x00C8FFFF);
	m_ConvDateRad.SetColor(0x00C8FFFF);
	m_ShowAllRad.SetColor(0x00FFFFFF);
	m_ShowNumberRad.SetColor(0x00FFFFFF);
	m_ShowPercentRad.SetColor(0x00FFFFFF);
	
	m_graph = GetDlgItem(IDC_GRAPH)->GetControlUnknown();
	m_cursor = LoadCursor(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDC_EXPAND));
	m_up.AutoSet(NXB_UP);
	m_referral = "RootNode";
	m_numberRad.SetCheck(true);
	m_up.EnableWindow(FALSE);
	m_oldCursor = NULL;
	font = m_graph->Font;
	font->PutName("Arial Narrow");
	font->PutSize(COleCurrency(13, 0));

	//setup the "Show Completed Appointments Only" checkbox
	CheckDlgButton(IDC_COMPLETED_ONLY, GetRemotePropertyInt("MarketingCompletedOnly", 0, 0, GetCurrentUserName(), true));

	m_graph->Background = 0xFDFDFD;

	//initialize the datalist
	m_pApptPurposeList = BindNxDataListCtrl(IDC_APPT_PURPOSE_LIST, GetRemoteData(), FALSE);
	m_pYearFilter = BindNxDataListCtrl(IDC_YEAR_FILTER, GetRemoteData(), FALSE);

	return TRUE;  
}

void CMarketGraphDlg::OnChangeFrom() 
{
	UpdateView();	
}

void CMarketGraphDlg::OnChangeTo() 
{
	UpdateView();
}

void CMarketGraphDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
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

		if (m_revenueRad.GetCheck())
			OnRevenueRad();
		else if (m_numberRad.GetCheck())
			OnNumberRad();
		else if (m_costRad.GetCheck())
			OnCostsRad();
		else if (m_conversionRad.GetCheck())
			OnConversionRad();
		else if (m_ConvDateRad.GetCheck()) {
			RefreshConversionDateGraph();
		}
	}
	NxCatchAll("Could not update Marketing Comparison");
}

void CMarketGraphDlg::OnCloseUpFrom() 
{
	OnChangeFrom();
}

void CMarketGraphDlg::OnCloseUpTo() 
{
	OnChangeTo();	
}

void CMarketGraphDlg::OnUp() 
{
	Up();
}

void CMarketGraphDlg::OnClickColumnGraph(short Row, short Column) 
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

void CMarketGraphDlg::OnMouseMoveColumnGraph(short Row, short Column) 
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

void CMarketGraphDlg::OnConversionRad() 
{
	//This is the "Conversion Rate" button

	GraphDescript desc;
	CString sql, from, to, doc, loc;
	int id, ndoc, nloc;
	CString referrals;

	//disable the Show Completed only checkbox, it doesn't apply to this view
	GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_HIDE);
	//

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);
	

	//setup parameters that will be used in the query
	GetParameters(from, to, ndoc, nloc, id);
	if (ndoc != -1)
		doc.Format(" AND PatientsT.MainPhysician = %i ", ndoc);
	if (nloc != -1) //user personT location instead of scheduler location
		loc.Format(" AND PersonT.Location = %i ", nloc);

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

	sql = "SELECT PatientsT.ReferralID, "
		"CAST (100.0 * SUM(CASE WHEN SurgeriesQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Surgeries, "
		"CAST (100.0 * SUM(CASE WHEN ConsultsQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Consults, "
		"CAST (COUNT(DISTINCT ConsultsQ.PatientID)-1 AS FLOAT) AS ConsultCount, "
		"CAST (COUNT(DISTINCT PatientsT.PersonID) AS FLOAT) AS Referrals "
		"FROM PatientsT "
		"INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
		"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
		"LEFT JOIN ( "
			"SELECT AppointmentsT.PatientID "
			"FROM AppointmentsT "
			"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE (AptTypeT.Category = 4 "	//surgery
			"OR AptTypeT.Category = 3 "		//Minor procedure
			"OR AptTypeT.Category = 6) "		//Other procedure
			"AND AppointmentsT.ShowState <> 3 " // 3 = no show
			"AND AppointmentsT.Status <> 4 "
			"GROUP BY AppointmentsT.PatientID "
		") AS SurgeriesQ ON PatientsT.PersonID = SurgeriesQ.PatientID "
		"LEFT JOIN ( "
			"SELECT AppointmentsT.PatientID "
			"FROM AppointmentsT "
			"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE (AptTypeT.Category = 1) " //consult
			"AND AppointmentsT.ShowState <> 3 " //3 = no show
			"AND AppointmentsT.STatus <> 4 " //4 = cancelled
			"GROUP BY AppointmentsT.PatientID "
		") AS ConsultsQ ON PatientsT.PersonID = ConsultsQ.PatientID "
		"WHERE PersonT.FirstContactDate >= [from] "
		"AND PersonT.FirstContactDate < [to] "
		"[prov] [ref] [loc]"
		"GROUP BY PatientsT.ReferralID ";

	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[prov]", doc);
	sql.Replace("[loc]", loc);

	referrals = "AND " + Descendants(id, "PatientsT.ReferralID");
	sql.Replace("[ref]", referrals);

	desc.m_sql = sql;
	Save(sql);
	desc.Add("Referral to Consult", "Consults", 0xFF0000, "Referrals", GraphDescript::GD_DIV);
	desc.Add("Referral to Surgery", "Surgeries", 0x0000FF, "Referrals", GraphDescript::GD_DIV);
	
	m_graph->Format = "%0.0f%%";
	Graph(desc);
}

void CMarketGraphDlg::OnCostsRad() 
{
	//this appears to be unused code
	GraphDescript desc;
	CString sql, from, to, doc1, doc2, doc3;
	int id, prov, locID;

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);
	

	GetParameters(from, to, prov, locID, id);
	if (prov != -1)
	{	doc1.Format("AND PaymentsT.ProviderID = %i",		prov); 
		doc2.Format("AND ChargesT.DoctorsProviders = %i",	prov);
		doc3.Format("AND PaymentsT.ProviderID = %i",		prov);
	}
	sql = "SELECT CAST (SUM (CostSubQ.Cost) AS FLOAT) AS Cost, ReferralSource AS ReferralID FROM "
	"(SELECT Amount AS Cost, ReferralSource "
	"FROM MarketingCostsT "
	"WHERE EffectiveFrom >= [from] AND EffectiveFrom < [to] AND EffectiveTo >= [from] AND EffectiveTo < [to] "
	"AND [referrals] "

	"UNION ALL "

	"SELECT (Amount * DATEDIFF(day, [to], EffectiveFrom)) "
	"/ DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
	"FROM MarketingCostsT "
	"WHERE EffectiveFrom < [to] AND EffectiveFrom > [from] AND EffectiveTo >= [to] "
	"AND [referrals] "

	"UNION ALL "

	"SELECT (Amount * DATEDIFF(day, [to], [from])) "
	"/ DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
	"FROM MarketingCostsT "
	"WHERE EffectiveFrom <= [from] AND EffectiveTo >= [to] "
	"AND (EffectiveFrom <> [from] OR EffectiveTo <> [to]) "
	"AND [referrals] "

	"UNION ALL "

	"SELECT (Amount * DATEDIFF(day, EffectiveTo, [from])) "
	"/ DATEDIFF (day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
	"FROM MarketingCostsT "
	"WHERE EffectiveFrom < [from] AND EffectiveTo > [from] AND EffectiveTo < [to]"
	"AND [referrals] "

	") AS CostSubQ "
	"GROUP BY ReferralSource";

	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[referrals]", Descendants(id, "ReferralSource"));

	desc.m_sql = sql;
	desc.Add("Cost", "Cost",  0x00B5FF);

	m_graph->Format = _bstr_t(GetCurrencySymbol());
	Graph(desc);	
}

void CMarketGraphDlg::OnRevenueRad() 
{
	//This is the "$ from Referrals" button

	GraphDescript desc;
	CString sql, from, to, doc1, doc2, doc3, loc, category;
	int id, prov, locID, catID;

	//disable the Show Completed only checkbox
	GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_HIDE);

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);
	

	GetParameters(from, to, prov, locID, id);
	catID = GetCategory();
	if (prov != -1)
	{	doc1.Format("AND PaymentsT.ProviderID = %i",		prov); 
		doc2.Format("AND ChargesT.DoctorsProviders = %i",	prov);
		doc3.Format("AND PaymentsT.ProviderID = %i",		prov);
	}

	if (locID != -1)
		loc.Format(" AND PersonT.Location = %i ", locID);


	//DRT 4/2/03 - Filter on category if one is chosen.  If it is chosen, we will only
	//		show the income (payments) which are applied to charges of that category.
	if(catID != -1) {
		category.Format(" AND ServiceT.Category = %li", catID);

		sql.Format("SELECT ReferralSourceT.PersonID AS ReferralID, "
		"CAST ((CASE WHEN PayQ.Payments IS NULL THEN 0 ELSE PayQ.Payments END) AS FLOAT) AS Payments, "
		"0 as Cost "
		"FROM ReferralSourceT LEFT JOIN "
		"(	SELECT Sum(CASE WHEN (ChargesT.ID IS NULL OR %li = -1) THEN LineItemT.Amount ELSE AppliesT.Amount END) AS Payments, PatientsT.ReferralID AS ID "
			"FROM LineItemT "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
			"LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID "
			"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
	//		"LEFT JOIN AppliesT AS AppliesT1 ON LineItemT.ID = AppliesT1.SourceID "
	//		"LEFT JOIN AppliesT AS AppliesT2 ON LineItemT.ID = AppliesT2.DestID "
			"WHERE LineItemT.Deleted = 0 AND (LineItemT.Type = 1 OR LineItemT.Type = 3) "		//DRT 6/5/02 - Added refunds so this graph matches the Effectiveness #'s
	//		"AND (PaymentsT.PrePayment = 0 OR AppliesT1.ID IS NULL AND AppliesT2.ID IS NULL) "
			"AND FirstContactDate >= [from] AND FirstContactDate < [to] "
			"AND [referrals] "
			"[doc3] "
			"[loc] "
			"[category] "
			"GROUP BY PatientsT.ReferralID "
		") AS PayQ ON ReferralSourceT.PersonID = PayQ.ID "

		//costs
		"UNION ALL "

		"SELECT ReferralSource AS ReferralID, 0 AS Payments, "
		"CAST (SUM (CostSubQ.Cost) AS FLOAT) AS Cost "
		"FROM "
		"(SELECT Amount AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom >= [from] AND EffectiveFrom < [to] AND EffectiveTo >= [from] AND EffectiveTo < [to] "
		"AND [referral2] "

		"UNION ALL "

		"SELECT (Amount * DATEDIFF(day, [to], EffectiveFrom)) / DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom < [to] AND EffectiveFrom > [from] AND EffectiveTo >= [to] "
		"AND [referral2] "

		"UNION ALL "

		"SELECT (Amount * DATEDIFF(day, [to], [from])) / DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom <= [from] AND EffectiveTo >= [to] "
		"AND (EffectiveFrom <> [from] OR EffectiveTo <> [to]) "
		"AND [referral2] "

		"UNION ALL "

		"SELECT (Amount * DATEDIFF(day, EffectiveTo, [from])) / DATEDIFF (day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom < [from] AND EffectiveTo > [from] AND EffectiveTo < [to]"
		"AND [referral2] "

		") AS CostSubQ "
		"GROUP BY ReferralSource", catID);
	}
	else {
		//Ignore applies, which makes our calculation easier.
		sql = "SELECT ReferralSourceT.PersonID AS ReferralID, "
		"CAST ((CASE WHEN PayQ.Payments IS NULL THEN 0 ELSE PayQ.Payments END) AS FLOAT) AS Payments, "
		"0 as Cost "
		"FROM ReferralSourceT LEFT JOIN "
		"(	SELECT Sum(LineItemT.Amount) AS Payments, PatientsT.ReferralID AS ID "
			"FROM LineItemT "
			"INNER JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE LineItemT.Deleted = 0 AND (LineItemT.Type = 1 OR LineItemT.Type = 3) "		//DRT 6/5/02 - Added refunds so this graph matches the Effectiveness #'s
	//		"AND (PaymentsT.PrePayment = 0 OR AppliesT1.ID IS NULL AND AppliesT2.ID IS NULL) "
			"AND FirstContactDate >= [from] AND FirstContactDate < [to] "
			"AND [referrals] "
			"[doc3] "
			"[loc] "
			"GROUP BY PatientsT.ReferralID "
		") AS PayQ ON ReferralSourceT.PersonID = PayQ.ID "

		//costs
		"UNION ALL "

		"SELECT ReferralSource AS ReferralID, 0 AS Payments, "
		"CAST (SUM (CostSubQ.Cost) AS FLOAT) AS Cost "
		"FROM "
		"(SELECT Amount AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom >= [from] AND EffectiveFrom < [to] AND EffectiveTo >= [from] AND EffectiveTo < [to] "
		"AND [referral2] "

		"UNION ALL "

		"SELECT (Amount * DATEDIFF(day, [to], EffectiveFrom)) / DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom < [to] AND EffectiveFrom > [from] AND EffectiveTo >= [to] "
		"AND [referral2] "

		"UNION ALL "

		"SELECT (Amount * DATEDIFF(day, [to], [from])) / DATEDIFF(day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom <= [from] AND EffectiveTo >= [to] "
		"AND (EffectiveFrom <> [from] OR EffectiveTo <> [to]) "
		"AND [referral2] "

		"UNION ALL "

		"SELECT (Amount * DATEDIFF(day, EffectiveTo, [from])) / DATEDIFF (day, EffectiveTo, EffectiveFrom) AS Cost, ReferralSource "
		"FROM MarketingCostsT "
		"WHERE EffectiveFrom < [from] AND EffectiveTo > [from] AND EffectiveTo < [to]"
		"AND [referral2] "

		") AS CostSubQ "
		"GROUP BY ReferralSource";
	}


	desc.m_sql = sql;

	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[doc1]", doc1);
	sql.Replace("[doc2]", doc2);
	sql.Replace("[doc3]", doc3);
	sql.Replace("[referrals]", Descendants(id, "PatientsT.ReferralID"));
	sql.Replace("[referral2]", Descendants(id, "ReferralSource"));
	sql.Replace("[loc]", loc);
	sql.Replace("[category]", category);

	Save(sql);

	desc.m_sql = sql;
//	desc.Add("Charges",		"Charges",	0xFF0000);
	desc.Add("Income",	"Payments",	0x00C000);
	desc.Add("Expenditure",	"Cost",		0x00B5FF);

	m_graph->Format = _bstr_t(GetCurrencySymbol());

	Graph(desc);	
}

void CMarketGraphDlg::OnNumberRad() 
{
	//this is the "Number of Referrals" button

	GraphDescript desc;
	CString sql, from, to, doc, loc;
	int id, ndoc, nloc;
	CString referrals;

	GetParameters(from, to, ndoc, nloc, id);
	if (ndoc != -1)
		doc.Format(" AND PatientsT.MainPhysician = %i ", ndoc);
	if (nloc != -1)
		loc.Format(" AND Location = %i ", nloc);

	//enable the Show Completed only checkbox
	GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_SHOW);

	//disable the ... button and the datalist
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_HIDE);
	

	//
	////////////////
	//Generate a SQL statement for the graph
	//
	////////////////
	//Select:	ReferralID
	//			Count the Patients with scheduled surgery appointments AS Conv (blue bar)
	//			Count the patients as Referrals (red bar)
	//			Count the pts with scheduled consults as ConsConv (green bar)
	////////////////
	//SubQ's:	SubQ
	//			Selects the PatientID for all appointments
	//		Filter:
	//			Category is Surgery (4) or Minor Procedure (3) or Other Procedure (6)
	//			Filters out No Show's
	//			Filters out Cancelled
	//		Groups by:
	//			PatientID
	////////////////
	//			SubConsultQ
	//			Selects the PatientID for all appointments
	//		Filter:
	//			Category is Consult
	//			Filters out No Show's
	//			Filters out Cancelled
	//		Groups by:
	//			PatientID
	////////////////
	//Filters:
	//			First Contact Date between toolbar range
	//			Provider chosen in toolbar
	//			Referral source (chosen in the graph, if any)
	//			Location chosen in toolbar
	//			Appointment Start Time <= Current Time (optional checkbox)
	//Groups By:
	//			ReferralID
	////////////////
	//

	////////////////
	//Changes/Updates/Revisions:
	//		September 23rd, 2002 - DRT - Added [IDC_COMPLETED_ONLY] checks, to allow users to only show surgeries where patients have come in yet
	//		May 2002 - DRT - Added consults scheduled bar (green color)
	//		May 2002 - DRT - Previously only Surgery category items were shown.  Now Surgery, Minor Procedure, Other Procedure are shown
	////////////////

	sql = "SELECT PatientsT.ReferralID, "
		"CAST (SUM(CASE WHEN SubQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Conv, "
		"CAST (COUNT(DISTINCT PatientsT.PersonID) AS FLOAT) AS Referrals, "
		"CAST (SUM(CASE WHEN SubConsultQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS ConsConv "
		"FROM PatientsT "
		"INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
		"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "

		"LEFT JOIN ( "
			"SELECT AppointmentsT.PatientID "
			"FROM AppointmentsT "
			"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE (AptTypeT.Category = 4 "	//surgery
			"OR AptTypeT.Category = 3 "	//minor procedure
			"OR AptTypeT.Category = 6) "	//other procedure
			"AND AppointmentsT.ShowState <> 3 "
			"AND AppointmentsT.Status <> 4 ";

			if(IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
				//they only wish to see appointments previous to today
				sql += " AND AppointmentsT.StartTime <= GetDate() ";
			}

	sql += 
			"GROUP BY AppointmentsT.PatientID "
		") AS SubQ ON PatientsT.PersonID = SubQ.PatientID "

		"LEFT JOIN ( "
		"	SELECT "
		"	AppointmentsT.PatientID "
		"	FROM "
		"	AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"	WHERE (AptTypeT.Category = 1 "	//consult 
		"		AND AppointmentsT.ShowState <> 3 ";

		if(IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
			//they only wish to see appointments previous to today
			sql += " AND AppointmentsT.StartTime <= GetDate() ";
		}		

	sql += 
		"		AND AppointmentsT.Status <> 4) "
		"GROUP BY AppointmentsT.PatientID) SubConsultQ ON PatientsT.PersonID = SubConsultQ.PatientID "

		"WHERE PersonT.FirstContactDate >= [from] "
		"AND PersonT.FirstContactDate < [to] "
		"[prov] "
		"[ref] "
		"[loc] "
		"GROUP BY PatientsT.ReferralID ";

	sql.Replace("[from]", from);
	sql.Replace("[to]", to);
	sql.Replace("[prov]", doc);
	sql.Replace("[loc]", loc);

	referrals = "AND " + Descendants(id, "PatientsT.ReferralID");
	sql.Replace("[ref]", referrals);

	desc.m_sql = sql;
	Save(sql);
	desc.Add("Referrals", "Referrals", 0x0000FF);
	if(IsDlgButtonChecked(IDC_COMPLETED_ONLY)) {
		desc.Add("Patients that completed a consult", "ConsConv", 0x00AF00);
		desc.Add("Patients that completed a procedure", "Conv", 0xFF0000);
	}
	else {
		desc.Add("Patients that scheduled a consult", "ConsConv", 0x00AF00);
		desc.Add("Patients that scheduled a procedure", "Conv", 0xFF0000);
	}

	m_graph->Format = "%0.0f";
	Graph(desc);
}

void CMarketGraphDlg::OnCompletedOnly() 
{
	//Called when the user selects the "Show Only Completed Appointments" checkbox

	//save the fact that they have this checked
	SetRemotePropertyInt("MarketingCompletedOnly", IsDlgButtonChecked(IDC_COMPLETED_ONLY) ? 1 : 0, 0, GetCurrentUserName());


	OnNumberRad();	//rebuilds the sql query
}


void CMarketGraphDlg::OnConversionDateRad() 
{
	
	//disable the Show Completed only checkbox, it doesn't apply to this view
	GetDlgItem(IDC_COMPLETED_ONLY)->ShowWindow(SW_HIDE);
	//

	//show the configure button
	GetDlgItem(IDC_APPT_PURPOSE_LIST)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CONFIGURE_APPT_TYPES)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_YEAR_FILTER)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_SHOW_ALL_COLUMNS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_NUMBERS_ONLY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_SHOW_PERCENTAGES_ONLY)->ShowWindow(SW_SHOW);
	
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

void CMarketGraphDlg::RefreshConversionDateGraph() {

	try {
	
		//This is the "Conversion Rate By Date" button

		GraphDescript desc;
		CString strSql, strFrom, strTo, strDoc, strLoc;
		int nID, nProvID, nLocID;
		
		//setup parameters that will be used in the query
		GetParameters(strFrom, strTo, nProvID, nLocID, nID);
		if (nProvID!= -1)
			strDoc.Format(" AND PatientsT.MainPhysician = %i ", nProvID);
		if (nLocID != -1) //user personT location instead of scheduler location
			strLoc.Format(" AND PersonT.Location = %i ", nLocID);

		//figure out what their values are from ConfigRT
		BOOL bSplitConsults = GetRemotePropertyInt("CRGSplitConsults", 1, 0, "<None>", TRUE);

		CString strConsIDList = GetRemotePropertyText("CRGConsultList", "", 0, "<None>", TRUE);
		CString strSurgIDList = GetRemotePropertyText("CRGSurgeryList", "", 0, "<None>", TRUE);

		CString strBaseSql, strExtraTypeSql;

		CString strYearFilter = AsString(m_pYearFilter->GetValue(m_pYearFilter->CurSel, 0));

		CString strCons1Label, strCons2Label, strTotalLabel, strSurgeryLabel;

		CString strFirstContactDateFrom, strFirstContactDateTo;

		strFirstContactDateFrom.Format(" AND PersonT.FirstContactDate >= %s ", strFrom);
		strFirstContactDateTo.Format(" AND PersonT.FirstContactDate <= %s ", strTo);

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
				" [from] [to] [prov] [loc] "
				" GROUP BY DATEPART(MM, Date) "
				" UNION ALL "
				" SELECT 0 AS ConsultCount,  " 
				" DATEPART(MM, Date) AS Month, " 
				" Count(AppointmentsT.ID) AS SurgeryCount, 0 As OtherTypeCount " 
				" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND AptTypeID IN %s  %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [from] [to] [prov] [loc] "
				" GROUP BY DATEPART(MM, Date) ) SubQuery Group By Month ) Sub ", strConsIDList, strPurpose, strYearFilter, strSurgIDList, strPurpose, strYearFilter);  

			strBaseSql.Replace("[from]", strFirstContactDateFrom);
			strBaseSql.Replace("[to]", strFirstContactDateTo);
			strBaseSql.Replace("[prov]", strDoc);
			strBaseSql.Replace("[loc]", strLoc);

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

				desc.Add(strCons1Label + strSumConsult, "ConsultCount", 0xFF0000, "ConsultCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0x0000FF, "SurgeryCount");
			}
			else if (IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY)) {
				
				desc.Add("Conversion Rate" + strYearPercent, "Cons1Percent", 0xFF00FF, "Cons1Percent");

			}
			else {

				desc.Add(strCons1Label + strSumConsult, "ConsultCount", 0xFF0000, "ConsultCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0x0000FF, "SurgeryCount");
				desc.Add("Conversion Rate" + strYearPercent, "Cons1Percent", 0xFF00FF, "Cons1Percent");

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
				" [from] [to] [prov] [loc] "
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
				" [from] [to] [prov] [loc] "
				" Group BY DATEPART(MM, Date) "
				" UNION ALL "
				" SELECT 0 as ConsultCount,  "
				" DATEPART(MM, Date) AS Month, "
				" 0 AS SurgeryCount, Count(AppointmentsT.ID) As OtherTypeCount "
				" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE AppointmentsT.Status <> 4 AND  AptTypeID IN %s %s "
				" AND DATEPART(YYYY, Date) = %s "
				" [from] [to] [prov] [loc] "
				" Group BY DATEPART(MM, Date) ) SubQuery Group By Month) Sub2", strCons1Tmp, strPurpose, strYearFilter, strSurgIDList, strPurpose, strYearFilter, strCons2Tmp, strPurpose, strYearFilter);

			strBaseSql.Replace("[from]", strFirstContactDateFrom);
			strBaseSql.Replace("[to]", strFirstContactDateTo);
			strBaseSql.Replace("[prov]", strDoc);
			strBaseSql.Replace("[loc]", strLoc);


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

				desc.Add(strCons1Label + strSumCons1, "ConsultCount", 0xFF0000, "ConsultCount");
				desc.Add(strCons2Label + strSumCons2, "OtherTypeCount", 0x00FF00, "OtherTypeCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0x0000FF, "SurgeryCount");
				desc.Add("Total Consults" + strSumTotal, "TotalConsultCount", 0x00FFFF, "TotalConsultCount");
			}
			else if (IsDlgButtonChecked(IDC_SHOW_PERCENTAGES_ONLY)) {
				
				desc.Add(strCons1Label + " Conversion Rate" + strCons1Percent, "Cons1Percent", 0xFF8000, "Cons1Percent");
				desc.Add(strCons2Label + " Conversion Rate" + strCons2Percent, "Cons2Percent", 0xC0C0C0, "Cons2Percent");
				desc.Add("Total Conversion Rate" + strTotalPercent, "TotalPercent", 0xFF00FF, "TotalPercent");
			}
			else {

				desc.Add(strCons1Label + strSumCons1, "ConsultCount", 0xFF0000, "ConsultCount");
				desc.Add(strCons2Label + strSumCons2, "OtherTypeCount", 0x00FF00, "OtherTypeCount");
				desc.Add(strSurgeryLabel + strSumSurgery, "SurgeryCount", 0x0000FF, "SurgeryCount");
				desc.Add("Total Consults" + strSumTotal, "TotalConsultCount", 0x00FFFF, "TotalConsultCount");
				desc.Add(strCons1Label + " Conversion Rate" + strCons1Percent, "Cons1Percent", 0xFF8000, "Cons1Percent");
				desc.Add(strCons2Label + " Conversion Rate" + strCons2Percent, "Cons2Percent", 0xC0C0C0, "Cons2Percent");
				desc.Add("Total Conversion Rate" + strTotalPercent, "TotalPercent", 0xFF00FF, "TotalPercent");


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
		m_progress.SetPos(0);

	}NxCatchAll("Error loading graph");


}


void CMarketGraphDlg::OnConfigureApptTypes() 
{
	CConversionRateByDateConfigDlg dlg;
	dlg.DoModal();
	RefreshConversionDateGraph();
}

void CMarketGraphDlg::OnSelChosenApptPurposeList(long nRow) 
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




void CMarketGraphDlg::CheckDataList(CMultiSelectDlg *dlg) {

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

void CMarketGraphDlg::SelectMultiPurposes() {

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

void CMarketGraphDlg::OnSelChosenYearFilter(long nRow) 
{
	RefreshConversionDateGraph();	
}

void CMarketGraphDlg::OnShowAllColumns() 
{
	RefreshConversionDateGraph();	
	
}

void CMarketGraphDlg::OnShowNumbersOnly() 
{
	RefreshConversionDateGraph();	
	
}

void CMarketGraphDlg::OnShowPercentagesOnly() 
{
	RefreshConversionDateGraph();	
	
}

void CMarketGraphDlg::Refresh() {

	//MessageBox("Hi");
}
*/