// SchedulingProductivityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SchedulingProductivityDlg.h"
#include "pracprops.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSchedulingProductivityDlg dialog
using namespace ADODB;

CSchedulingProductivityDlg::CSchedulingProductivityDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedulingProductivityDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSchedulingProductivityDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSchedulingProductivityDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSchedulingProductivityDlg)
	DDX_Control(pDX, IDC_PRODUCTIVITY_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_PRODUCTIVITY_TO, m_dtTo);
	DDX_Control(pDX, IDC_INQUIRIES, m_nxstaticInquiries);
	DDX_Control(pDX, IDC_NEW_PROSPECTS, m_nxstaticNewProspects);
	DDX_Control(pDX, IDC_CONSULTS, m_nxstaticConsults);
	DDX_Control(pDX, IDC_PROCEDURES_PERFORMED, m_nxstaticProceduresPerformed);
	DDX_Control(pDX, IDC_PREPAYMENTS_ENTERED, m_nxstaticPrepaymentsEntered);
	DDX_Control(pDX, IDC_PRODUCTIVITY_GROUPBOX, m_btnProductivityGroupbox);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CSchedulingProductivityDlg, IDC_PRODUCTIVITY_FROM, 2 /* Change */, OnChangeProductivityFrom, VTS_NONE)
//	ON_EVENT(CSchedulingProductivityDlg, IDC_PRODUCTIVITY_TO, 2 /* Change */, OnChangeProductivityTo, VTS_NONE)

BEGIN_MESSAGE_MAP(CSchedulingProductivityDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSchedulingProductivityDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PRODUCTIVITY_FROM, OnChangeProductivityFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_PRODUCTIVITY_TO, OnChangeProductivityTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulingProductivityDlg message handlers

BEGIN_EVENTSINK_MAP(CSchedulingProductivityDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSchedulingProductivityDlg)
	ON_EVENT(CSchedulingProductivityDlg, IDC_DATE_FILTER_OPTIONS, 16 /* SelChosen */, OnSelChosenDateFilterOptions, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSchedulingProductivityDlg::OnChangeProductivityFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//Their date option is no longer valid.
	m_pDateOptions->CurSel = -1;
	Refresh();

	*pResult = 0;
}

void CSchedulingProductivityDlg::OnChangeProductivityTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//Their date option is no longer valid.
	m_pDateOptions->CurSel = -1;
	Refresh();

	*pResult = 0;
}

void CSchedulingProductivityDlg::Refresh()
{
	try {
		CString strDateFrom = FormatDateTimeForSql(COleDateTime(m_dtFrom.GetValue()));
		CString strDateTo = FormatDateTimeForSql(COleDateTime(m_dtTo.GetValue()) + COleDateTimeSpan(1,0,0,0));
		_RecordsetPtr rsInquiries = CreateRecordset("SELECT Count(PersonID) AS Inquiries "
			"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"WHERE PatientsT.CurrentStatus = 4 AND PersonT.InputDate >= '%s' AND PersonT.InputDate < '%s'", strDateFrom, strDateTo);
		CString str;
		str.Format("%li", AdoFldLong(rsInquiries, "Inquiries"));
		SetDlgItemText(IDC_INQUIRIES, str);
		rsInquiries->Close();
		
		_RecordsetPtr rsNewProspects = CreateRecordset("SELECT Count(PersonT.ID) AS NewProspects "
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE PersonT.InputDate >= '%s' AND PersonT.InputDate < '%s' "
			"AND (PatientsT.CurrentStatus IN (2,3) OR EXISTS (SELECT ID FROM PatientStatusHistoryT WHERE PersonID = PersonT.ID "
			"AND OldStatus IN (2,3)))", strDateFrom, strDateTo);
		str.Format("%li", AdoFldLong(rsNewProspects, "NewProspects"));
		rsNewProspects->Close();
		SetDlgItemText(IDC_NEW_PROSPECTS, str);

		_RecordsetPtr rsConsults = CreateRecordset("SELECT Count(AppointmentsT.ID) AS Consults "
			"FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE AptTypeT.Category = 1 AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date < '%s' "
			"AND AppointmentsT.ShowState IN (1,2,4) AND AppointmentsT.Status <> 4", strDateFrom, strDateTo);
		str.Format("%li", AdoFldLong(rsConsults, "Consults"));
		rsConsults->Close();
		SetDlgItemText(IDC_CONSULTS, str);

		_RecordsetPtr rsProcedures = CreateRecordset("SELECT Count(AppointmentsT.ID) AS Procedures "
			"FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE AptTypeT.Category IN (3,4,6) AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date < '%s' "
			"AND AppointmentsT.ShowState IN (1,2,4) AND AppointmentsT.Status <> 4", strDateFrom, strDateTo);
		str.Format("%li", AdoFldLong(rsProcedures, "Procedures"));
		rsProcedures->Close();
		SetDlgItemText(IDC_PROCEDURES_PERFORMED, str);

		_RecordsetPtr rsPrepays = CreateRecordset("SELECT CASE WHEN Sum(Amount) Is Null THEN convert(money,0) ELSE Sum(Amount) END AS Amount "
			"FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"WHERE LineItemT.Deleted = 0 AND PaymentsT.PrePayment = 1 AND LineItemT.InputDate >= '%s' AND LineItemT.InputDate < '%s'",
			strDateFrom, strDateTo);
		SetDlgItemText(IDC_PREPAYMENTS_ENTERED, FormatCurrencyForInterface(AdoFldCurrency(rsPrepays, "Amount"), TRUE, TRUE));
		rsPrepays->Close();
	}NxCatchAll("Error in CSchedulingProductivityDlg::Refresh()");
}

BOOL CSchedulingProductivityDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);
	
	//Default to yesterday.
	COleDateTime dtYesterday = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
	dtYesterday.SetDateTime(dtYesterday.GetYear(), dtYesterday.GetMonth(), dtYesterday.GetDay(), 0, 0, 0);
	m_dtFrom.SetValue(_variant_t(dtYesterday));
	m_dtTo.SetValue(_variant_t(dtYesterday));

	m_pDateOptions = BindNxDataListCtrl(this, IDC_DATE_FILTER_OPTIONS, NULL, false);
	IRowSettingsPtr pRow = m_pDateOptions->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Yesterday"));
	m_pDateOptions->AddRow(pRow);
	pRow = m_pDateOptions->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Today"));
	m_pDateOptions->AddRow(pRow);
	pRow = m_pDateOptions->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Month To Date"));
	m_pDateOptions->AddRow(pRow);
	pRow = m_pDateOptions->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Year To Date"));
	m_pDateOptions->AddRow(pRow);
	pRow = m_pDateOptions->GetRow(-1);
	pRow->PutValue(0, _bstr_t("Last Year To Date"));
	m_pDateOptions->AddRow(pRow);
	
	m_pDateOptions->CurSel = 0;
	
	Refresh();	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSchedulingProductivityDlg::OnSelChosenDateFilterOptions(long nRow) 
{
	if(nRow == -1) {
		return;
	}

	COleDateTime dtFrom, dtTo;
	switch(nRow) {
	case 0: //Yesterday
		dtFrom = COleDateTime::GetCurrentTime() - COleDateTimeSpan(1,0,0,0);
		dtTo = dtFrom;
		break;
	case 1: //Today
		dtFrom = COleDateTime::GetCurrentTime();
		dtTo = dtFrom;
		break;
	case 2: //Month to Date.
		dtTo = COleDateTime::GetCurrentTime();
		dtFrom = dtTo;
		dtFrom.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), 1);
		break;
	case 3: //Year to Date.
		dtTo = COleDateTime::GetCurrentTime();
		dtFrom = dtTo;
		dtFrom.SetDate(dtFrom.GetYear(), 1, 1);
		break;
	case 4: //Last Year to Date.
		dtTo = COleDateTime::GetCurrentTime();
		dtTo.SetDate(dtTo.GetYear()-1, dtTo.GetMonth(),dtTo.GetDay());
		dtFrom = dtTo;
		dtFrom.SetDate(dtFrom.GetYear(), 1, 1);
		break;
	default: //???
		ASSERT(FALSE);
		return;
		break;
	}
	dtFrom.SetDateTime(dtFrom.GetYear(), dtFrom.GetMonth(),dtFrom.GetDay(), 0, 0, 0);
	dtTo.SetDateTime(dtTo.GetYear(), dtTo.GetMonth(), dtTo.GetDay(), 0, 0, 0);

	m_dtFrom.SetValue(_variant_t(dtFrom));
	m_dtTo.SetValue(_variant_t(dtTo));

	Refresh();
}
