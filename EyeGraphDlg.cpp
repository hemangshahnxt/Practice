// EyeGraphDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "EyeGraphDlg.h"
#include "EyeGraphCtrl.h"
#include "MsgBox.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEyeGraphDlg dialog


CEyeGraphDlg::CEyeGraphDlg(CWnd* pParent)
	: CNxDialog(CEyeGraphDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEyeGraphDlg)
	m_dtTo = COleDateTime::GetCurrentTime();
	m_dtFrom = COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
}


void CEyeGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEyeGraphDlg)
	DDX_Control(pDX, IDC_SEF, m_btnSEF);
	DDX_Control(pDX, IDC_DEF, m_btnDEF);
	DDX_Control(pDX, IDC_ALL_EYE_DATES, m_btnDatesAll);
	DDX_Control(pDX, IDC_EYE_DATE_RANGE, m_btnDateRange);
	DDX_Control(pDX, IDC_GRAPH, m_EyeGraph);
	DDX_DateTimeCtrl(pDX, IDC_EYE_DATE_TO, m_dtTo);
	DDX_DateTimeCtrl(pDX, IDC_EYE_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_IDEAL_PERCENT, m_nxeditIdealPercent);
	DDX_Control(pDX, IDC_TOTAL_EYES, m_nxeditTotalEyes);
	DDX_Control(pDX, IDC_IDEAL_CAPTION, m_nxstaticIdealCaption);
	DDX_Control(pDX, IDC_DATE_GROUPBOX, m_btnDateGroupbox);
	DDX_Control(pDX, IDC_UPDATE, m_btnUpdate);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEyeGraphDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEyeGraphDlg)
	ON_BN_CLICKED(IDC_SEF, OnSef)
	ON_BN_CLICKED(IDC_DEF, OnDef)
	ON_BN_CLICKED(IDC_UPDATE, OnUpdate)
	ON_BN_CLICKED(IDC_ALL_EYE_DATES, OnAllDates)
	ON_BN_CLICKED(IDC_EYE_DATE_RANGE, OnEyeDateRange)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EYE_DATE_FROM, OnDatetimechangeEyeDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EYE_DATE_TO, OnDatetimechangeEyeDateTo)
	ON_BN_CLICKED(IDC_PRINT, OnPrint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEyeGraphDlg message handlers

void CEyeGraphDlg::OnSef() 
{
	//m.hancock PLID 16756 8/30/2005 - Structure for the refractive visits changed, so need to update the query
	//m_strValueCalc = "Sphere + (Cyl/2)";
	m_strValueCalc = "EyeTestsT.Sphere + (EyeTestsT.Cyl/2)";
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);
}

void CEyeGraphDlg::OnDef() 
{
	m_strValueCalc = "(CASE WHEN Sphere < 0 THEN Sphere * -1 ELSE Sphere END) + ((CASE WHEN Cyl < 0 THEN Cyl * -1 ELSE Cyl END) / 2)";
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);
}

void CEyeGraphDlg::OnUpdate() 
{
	try {
		m_arPoints.RemoveAll();
		//Generate the list of data points.
		//m.hancock PLID 16756 8/30/2005 - Structure for the refractive visits changed, so need to update the query
		//_RecordsetPtr rsEyes = CreateRecordset("SELECT PreopsQ.SER AS Pre, PostOpsQ.SER AS Post, PreopsQ.Name, PreopsQ.EyeType FROM (SELECT %s AS SER, EyeProcedureID, EyeType, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name FROM EyeVisitsT INNER JOIN EyeProceduresT ON EyeVisitsT.EyeProcedureID = EyeProceduresT.ID INNER JOIN PersonT ON EyeProceduresT.PatientID = PersonT.ID WHERE Sphere Is Not Null AND Cyl Is Not Null AND VisitType = 1%s%s%s) AS PreopsQ INNER JOIN (SELECT %s AS SER, EyeProcedureID, EyeType FROM EyeVisitsT INNER JOIN EyeProceduresT On EyeVisitsT.EyeProcedureID = EyeProceduresT.ID WHERE Sphere Is Not Null AND Cyl Is Not Null %s %s%s%s) AS PostOpsQ ON PreopsQ.EyeProcedureID = PostOpsQ.EyeProcedureID AND PreopsQ.EyeType = PostOpsQ.EyeType", m_strValueCalc, m_strProcWhere, m_strDateWhere, m_strProvWhere, m_strValueCalc, m_strVisitTypeWhere, m_strProcWhere, m_strDateWhere, m_strProvWhere);
		_RecordsetPtr rsEyes = CreateRecordset("SELECT PreopsQ.SER AS Pre, PostOpsQ.SER AS Post, PreopsQ.Name, PreopsQ.EyeType FROM (SELECT %s AS SER, EyeProcedureID, EyeTestsT.EyeType, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name FROM EyeVisitsT INNER JOIN EyeProceduresT ON EyeVisitsT.EyeProcedureID = EyeProceduresT.ID INNER JOIN PersonT ON EyeProceduresT.PatientID = PersonT.ID INNER JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID WHERE EyeTestsT.Sphere Is Not Null AND EyeTestsT.Cyl Is Not Null AND VisitType = 1%s%s%s) AS PreopsQ INNER JOIN (SELECT %s AS SER, EyeProcedureID, EyeTestsT.EyeType FROM EyeVisitsT INNER JOIN EyeProceduresT On EyeVisitsT.EyeProcedureID = EyeProceduresT.ID INNER JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID WHERE EyeTestsT.Sphere Is Not Null AND EyeTestsT.Cyl Is Not Null %s %s%s%s) AS PostOpsQ ON PreopsQ.EyeProcedureID = PostOpsQ.EyeProcedureID AND PreopsQ.EyeType = PostOpsQ.EyeType", m_strValueCalc, m_strProcWhere, m_strDateWhere, m_strProvWhere, m_strValueCalc, m_strVisitTypeWhere, m_strProcWhere, m_strDateWhere, m_strProvWhere);
		FieldsPtr Fields = rsEyes->Fields;
		DataPoint Point;
		while(!rsEyes->eof) {
			Point.dLogicalX = AdoFldDouble(Fields, "Pre");
			Point.dLogicalY = AdoFldDouble(Fields, "Pre") - AdoFldDouble(Fields, "Post");
			m_arPoints.Add(Point);
			rsEyes->MoveNext();
		}
		//Set the caption
		if(IsDlgButtonChecked(IDC_DEF)) {
			m_EyeGraph.m_Type = CEyeGraphCtrl::DefocusScatter;
			m_EyeGraph.m_strCaption.Format("Defocus Equivalent Refraction (%s later)", _Q(VarString(m_pVisitTypeList->GetValue(m_pVisitTypeList->CurSel, 1))));
		}
		else {
			m_EyeGraph.m_Type = CEyeGraphCtrl::SphereScatter;
			m_EyeGraph.m_strCaption.Format("Spherical Equivalent Refraction (%s later)", _Q(VarString(m_pVisitTypeList->GetValue(m_pVisitTypeList->CurSel, 1))));
		}
		m_EyeGraph.m_pPointArray = &m_arPoints;
		m_EyeGraph.Invalidate();

		//Now, calculate the percentage in the range we want.
		SetDlgItemText(IDC_IDEAL_CAPTION, "% within 1 D"); //This may at some point have the option to be snellen lines.
		double dTotal = rsEyes->GetRecordCount();
		CString strTotalEyes;
		strTotalEyes.Format("%.f", dTotal);
		SetDlgItemText(IDC_TOTAL_EYES, strTotalEyes);
		if(dTotal > 0) {
			//m.hancock PLID 16756 8/30/2005 - Structure for the refractive visits changed, so need to update the query
			//_RecordsetPtr rsGoodEyes = CreateRecordset("SELECT %s AS SER, EyeProcedureID, EyeType FROM EyeVisitsT INNER JOIN EyeProceduresT On EyeVisitsT.EyeProcedureID = EyeProceduresT.ID WHERE Sphere Is Not Null AND Cyl Is Not Null AND (%s) >= -1 AND (%s) <= 1 %s %s%s%s", m_strValueCalc, m_strValueCalc, m_strValueCalc, m_strVisitTypeWhere, m_strProcWhere, m_strDateWhere, m_strProvWhere);
			_RecordsetPtr rsGoodEyes = CreateRecordset("SELECT %s AS SER, EyeProcedureID, EyeType FROM EyeVisitsT INNER JOIN EyeProceduresT On EyeVisitsT.EyeProcedureID = EyeProceduresT.ID INNER JOIN EyeTestsT ON EyeVisitsT.ID = EyeTestsT.VisitID WHERE EyeTestsT.Sphere Is Not Null AND EyeTestsT.Cyl Is Not Null AND (%s) >= -1 AND (%s) <= 1 %s %s%s%s", m_strValueCalc, m_strValueCalc, m_strValueCalc, m_strVisitTypeWhere, m_strProcWhere, m_strDateWhere, m_strProvWhere);
			double dTotalGood = rsGoodEyes->GetRecordCount();
			CString strIdealPercent;
			strIdealPercent.Format("%.2f%%", dTotalGood/dTotal * 100);
			SetDlgItemText(IDC_IDEAL_PERCENT, strIdealPercent);
		}
		else {
			SetDlgItemText(IDC_IDEAL_PERCENT, "0%");
		}


		GetDlgItem(IDC_UPDATE)->EnableWindow(FALSE);
	}NxCatchAll("Error in CEyeGraphDlg::OnUpdate()");
}

BOOL CEyeGraphDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnUpdate.AutoSet(NXB_MODIFY);
	m_btnClose.AutoSet(NXB_CLOSE);

	m_pProcList = BindNxDataListCtrl(IDC_PROC_LIST);
	IRowSettingsPtr pRow = m_pProcList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("<All procedures>"));
	m_pProcList->InsertRow(pRow, 0);

	m_pProcList->CurSel = 0;

	m_pVisitTypeList = BindNxDataListCtrl(IDC_VISIT_TYPE_LIST);
	
	m_pProvList = BindNxDataListCtrl(IDC_PROV_LIST);
	pRow = m_pProvList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _bstr_t("<All providers>"));
	m_pProvList->InsertRow(pRow, 0);

	m_pProvList->CurSel = 0;
	
	CheckDlgButton(IDC_SEF, BST_CHECKED);
	OnSef();

	CheckDlgButton(IDC_ALL_EYE_DATES, BST_CHECKED);
	OnAllDates();
	//Don't update until visit type list is finished requerying.
	//OnUpdate();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CEyeGraphDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEyeGraphDlg)
	ON_EVENT(CEyeGraphDlg, IDC_PROC_LIST, 16 /* SelChosen */, OnSelChosenProcList, VTS_I4)
	ON_EVENT(CEyeGraphDlg, IDC_VISIT_TYPE_LIST, 16 /* SelChosen */, OnSelChosenVisitTypeList, VTS_I4)
	ON_EVENT(CEyeGraphDlg, IDC_PROV_LIST, 16 /* SelChosen */, OnSelChosenProvList, VTS_I4)
	ON_EVENT(CEyeGraphDlg, IDC_VISIT_TYPE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedVisitTypeList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEyeGraphDlg::OnSelChosenProcList(long nRow) 
{
	//First of all, they are now update-able.
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

	//Now, generate the where clause as necessary.
	if(VarLong(m_pProcList->GetValue(m_pProcList->CurSel, 0)) == -1) {
		m_strProcWhere = "";
	}
	else {
		m_strProcWhere.Format(" AND EyeProceduresT.ProcedureID = %li", VarLong(m_pProcList->GetValue(m_pProcList->CurSel, 0)));
	}
}

void CEyeGraphDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CEyeGraphDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CEyeGraphDlg::OnSelChosenVisitTypeList(long nRow) 
{
	//OK, first of all, we're update-able.
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

	//Now, generate our part of the where clause.
	m_strVisitTypeWhere.Format("AND VisitType = %li", VarLong(m_pVisitTypeList->GetValue(m_pVisitTypeList->CurSel, 0)));

}

void CEyeGraphDlg::OnAllDates() 
{
	//First of all, figure out whether this is already checked.
	if(GetDlgItem(IDC_EYE_DATE_FROM)->IsWindowEnabled()) {
		//OK, this is not already checked.
		//First, we're update-able.
		GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

		//Second, disable the date fields.
		GetDlgItem(IDC_EYE_DATE_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_EYE_DATE_TO)->EnableWindow(FALSE);

		//Finally generate the where clause (pretty simple, in this case).
		m_strDateWhere = "";
	}

}

void CEyeGraphDlg::OnEyeDateRange() 
{
	//First of all, figure out whether this is already checked.
	if(!GetDlgItem(IDC_EYE_DATE_FROM)->IsWindowEnabled()) {
		//OK, this is nbot already checked.
		//First, we're update-able.
		GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

		//Second, enable the date fields.
		GetDlgItem(IDC_EYE_DATE_FROM)->EnableWindow(TRUE);
		GetDlgItem(IDC_EYE_DATE_TO)->EnableWindow(TRUE);

		//Finally, generate the where clause.
		UpdateData();
		COleDateTimeSpan dtsOneDay;
		dtsOneDay.SetDateTimeSpan(1, 0, 0, 0);
		m_strDateWhere.Format(" AND ProcedureDate >= '%s' AND ProcedureDate < '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo+dtsOneDay, dtoDate));
	}
}

void CEyeGraphDlg::OnDatetimechangeEyeDateFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//First, we're update-able.
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

	//Now, generate the where clause.
	UpdateData();
	COleDateTimeSpan dtsOneDay;
	dtsOneDay.SetDateTimeSpan(1, 0, 0, 0);
	m_strDateWhere.Format(" AND ProcedureDate >= '%s' AND ProcedureDate < '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo+dtsOneDay, dtoDate));

	*pResult = 0;
}

void CEyeGraphDlg::OnDatetimechangeEyeDateTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//First, we're update-able.
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

	//Now, generate the where clause.
	UpdateData();
	COleDateTimeSpan dtsOneDay;
	dtsOneDay.SetDateTimeSpan(1, 0, 0, 0);
	m_strDateWhere.Format(" AND ProcedureDate >= '%s' AND ProcedureDate < '%s'", FormatDateTimeForSql(m_dtFrom, dtoDate), FormatDateTimeForSql(m_dtTo+dtsOneDay, dtoDate));

	*pResult = 0;
}

void CEyeGraphDlg::OnSelChosenProvList(long nRow) 
{
	//First, we're update-able.
	GetDlgItem(IDC_UPDATE)->EnableWindow(TRUE);

	//Now, generate the where clause.
	int nProvID = VarLong(m_pProvList->GetValue(m_pProvList->CurSel, 0));
	if(nProvID == -1) {
		m_strProvWhere = "";
	}
	else {
		m_strProvWhere.Format(" AND EyeProceduresT.ProviderID = %li", nProvID);
	}
}

void CEyeGraphDlg::OnPrint() 
{
	MsgBox("There is no printer installed.");
}

void CEyeGraphDlg::OnRequeryFinishedVisitTypeList(short nFlags) 
{
	if(m_pVisitTypeList->GetRowCount() == 0) {
		MessageBox("You have not set up any visit types.  This dialog cannot be displayed.");
		CDialog::OnOK();
		return;
	}

	m_pVisitTypeList->CurSel = 0;
	OnSelChosenVisitTypeList(0);
	OnUpdate();

}
