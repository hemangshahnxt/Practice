// ExportWizardFiltersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportWizardFiltersDlg.h"
#include "ExportWizardDlg.h"
#include "ExportUtils.h"
#include "GlobalDrawingUtils.h"
#include "Groups.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
#define TEXT_DATE_CREATED		"created" //nDateMethod == 1
#define TEXT_DATE_MODIFIED		"modified" //nDateMethod == 2

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CExportWizardFiltersDlg property page
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

IMPLEMENT_DYNCREATE(CExportWizardFiltersDlg, CPropertyPage)

CExportWizardFiltersDlg::CExportWizardFiltersDlg() : CPropertyPage(CExportWizardFiltersDlg::IDD)
{
	m_nBasedOn = -1;
	//{{AFX_DATA_INIT(CExportWizardFiltersDlg)
	m_strDateHyperlink = _T("");
	//}}AFX_DATA_INIT

	//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
	m_rcDateHyperlink.left = m_rcDateHyperlink.top = m_rcDateHyperlink.right = m_rcDateHyperlink.bottom = 0;
	m_strDateHyperlink = TEXT_DATE_CREATED;
}

CExportWizardFiltersDlg::~CExportWizardFiltersDlg()
{
}

void CExportWizardFiltersDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportWizardFiltersDlg)
	DDX_Control(pDX, IDC_SORT_FIELD_UP, m_nxbUp);
	DDX_Control(pDX, IDC_SORT_FIELD_DOWN, m_nxbDown);
	DDX_Control(pDX, IDC_REMOVE_SORT_FIELD, m_nxbRemove);
	DDX_Control(pDX, IDC_ADD_SORT_FIELD, m_nxbAdd);
	DDX_Control(pDX, IDC_RECORD_DATE_FILTER_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_RECORD_DATE_FILTER_TO, m_dtTo);
	DDX_Text(pDX, IDC_DATE_HYPERLINK, m_strDateHyperlink);
	DDX_Control(pDX, IDC_FROM_PLUS_DAYS, m_nxeditFromPlusDays);
	DDX_Control(pDX, IDC_TO_PLUS_DAYS, m_nxeditToPlusDays);
	DDX_Control(pDX, IDC_ADVANCED_SORT_FIELD, m_nxeditAdvancedSortField);
	DDX_Control(pDX, IDC_DATE_HYPERLINK, m_nxstaticDateHyperlink);
	DDX_Control(pDX, IDC_ALL_NEW_RECORDS2, m_nxstaticAllNewRecords2);
	DDX_Control(pDX, IDC_FROM_DAYS_LABEL, m_nxstaticFromDaysLabel);
	DDX_Control(pDX, IDC_TO_PLUS_LABEL, m_nxstaticToPlusLabel);
	DDX_Control(pDX, IDC_TO_DAYS_LABEL, m_nxstaticToDaysLabel);
	DDX_Control(pDX, IDC_FROM_PLUS_LABEL, m_nxstaticFromPlusLabel);
	DDX_Control(pDX, IDC_ADVANCED_SORT_LABEL, m_nxstaticAdvancedSortLabel);
	DDX_Control(pDX, IDC_FILTER_GROUPBOX, m_btnFilterGroupbox);
	DDX_Control(pDX, IDC_ORDER_GROUPBOX, m_btnOrderGroupbox);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CExportWizardFiltersDlg, IDC_RECORD_DATE_FILTER_FROM, 2 /* Change */, OnChangeRecordDateFilterFrom, VTS_NONE)
//	ON_EVENT(CExportWizardFiltersDlg, IDC_RECORD_DATE_FILTER_TO, 2 /* Change */, OnChangeRecordDateFilterTo, VTS_NONE)

BEGIN_MESSAGE_MAP(CExportWizardFiltersDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CExportWizardFiltersDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RECORD_DATE_FILTER_FROM, OnChangeRecordDateFilterFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RECORD_DATE_FILTER_TO, OnChangeRecordDateFilterTo)
	ON_BN_CLICKED(IDC_ALL_NEW_RECORDS, OnAllNewRecords)
	ON_BN_CLICKED(IDC_MANUAL_SELECT, OnManualSelect)
	ON_EN_CHANGE(IDC_FROM_PLUS_DAYS, OnChangeFromPlusDays)
	ON_EN_CHANGE(IDC_TO_PLUS_DAYS, OnChangeToPlusDays)
	ON_BN_CLICKED(IDC_MANUAL_SORT, OnManualSort)
	ON_BN_CLICKED(IDC_ADD_SORT_FIELD, OnAddSortField)
	ON_BN_CLICKED(IDC_REMOVE_SORT_FIELD, OnRemoveSortField)
	ON_BN_CLICKED(IDC_SORT_FIELD_UP, OnSortFieldUp)
	ON_BN_CLICKED(IDC_SORT_FIELD_DOWN, OnSortFieldDown)
	ON_EN_CHANGE(IDC_ADVANCED_SORT_FIELD, OnChangeAdvancedSortField)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DATE_FILTER_CHECK, &CExportWizardFiltersDlg::OnBnClickedDateFilterCheck)
	ON_BN_CLICKED(IDC_LW_FILTER_CHECK, &CExportWizardFiltersDlg::OnBnClickedLwFilterCheck)
	ON_BN_CLICKED(IDC_EXPORT_DATE_LW_FILTER, &CExportWizardFiltersDlg::OnBnClickedExportDateLwFilter)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFiltersDlg message handlers

#define ADD_STANDARD_DATE(pList,nVal,strDescrip) {pRow = pList->GetRow(-1); pRow->PutValue(0,(long)nVal); pRow->PutValue(1,_bstr_t(strDescrip)); pList->AddRow(pRow);}

BOOL CExportWizardFiltersDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_pDateFilters = BindNxDataListCtrl(this, IDC_RECORD_DATE_FILTER_OPTIONS, NULL, false);
	m_pLwFilters = BindNxDataListCtrl(this, IDC_RECORD_LW_FILTER_OPTIONS, GetRemoteData(), false);
	m_pStandardDatesFrom = BindNxDataListCtrl(this, IDC_STANDARD_DATES_FROM, NULL, false);
	m_pStandardDatesTo = BindNxDataListCtrl(this, IDC_STANDARD_DATES_TO, NULL, false);
	m_pSortFieldsAvail = BindNxDataListCtrl(this, IDC_SORT_FIELDS_AVAIL, NULL, false);
	m_pSortFieldsSelect = BindNxDataListCtrl(this, IDC_SORT_FIELDS_SELECT, NULL, false);

	m_nxbAdd.AutoSet(NXB_RIGHT);
	m_nxbRemove.AutoSet(NXB_LEFT);
	m_nxbUp.AutoSet(NXB_UP);
	m_nxbDown.AutoSet(NXB_DOWN);

	IRowSettingsPtr pRow;
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoToday, "Today");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoYesterday, "Yesterday");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoTomorrow, "Tomorrow");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoFirstOfWeek, "First day of this week");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoFirstOfMonth, "First day of this month");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoFirstOfYear, "First day of this year");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoLastOfWeek, "Last day of this week");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoLastOfMonth, "Last day of this month");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoLastOfYear, "Last day of this year");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoFirstOfLastMonth, "First day of the previous month");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoOneMonthAgo, "This day last month");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoLastOfLastMonth, "Last day of the previous month");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoFirstOfLastYear, "First day of the previous year");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoOneYearAgo, "This day last year");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoLastOfLastYear, "Last day of the previous year");
	ADD_STANDARD_DATE(m_pStandardDatesFrom, dfoCustom, "Specific date:");

	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoToday, "Today");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoYesterday, "Yesterday");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoTomorrow, "Tomorrow");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoFirstOfWeek, "First day of this week");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoFirstOfMonth, "First day of this month");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoFirstOfYear, "First day of this year");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoLastOfWeek, "Last day of this week");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoLastOfMonth, "Last day of this month");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoLastOfYear, "Last day of this year");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoFirstOfLastMonth, "First day of the previous month");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoOneMonthAgo, "This day last month");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoLastOfLastMonth, "Last day of the previous month");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoFirstOfLastYear, "First day of the previous year");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoOneYearAgo, "This day last year");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoLastOfLastYear, "Last day of the previous year");
	ADD_STANDARD_DATE(m_pStandardDatesTo, dfoCustom, "Specific date:");

	//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
	// Calculate hyperlink rectangles
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_DATE_HYPERLINK);
	if (pWnd->GetSafeHwnd()) {
		// Get the position of the hotlinks
		pWnd->GetWindowRect(m_rcDateHyperlink);
		ScreenToClient(&m_rcDateHyperlink);

		if(((CExportWizardDlg*)GetParent())->m_ertRecordType == ertPatients ||
	       ((CExportWizardDlg*)GetParent())->m_ertRecordType == ertAppointments)
		{
			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);

			// If we previously saved with modified dates, need to change the hyperlink text
			if(((CExportWizardDlg*)GetParent())->m_efoFilterOption == efoAllNewModified)
				ChangeDateHyperlink();
		}
		else
		{
			// We're dealing with Charges or Payments, so we need to always show the "created" text
			m_strDateHyperlink = TEXT_DATE_CREATED;
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CExportWizardFiltersDlg, CPropertyPage)
    //{{AFX_EVENTSINK_MAP(CExportWizardFiltersDlg)
	ON_EVENT(CExportWizardFiltersDlg, IDC_RECORD_LW_FILTER_OPTIONS, 16 /* SelChosen */, OnSelChosenRecordLwFilterOptions, VTS_I4)
	ON_EVENT(CExportWizardFiltersDlg, IDC_RECORD_DATE_FILTER_OPTIONS, 16 /* SelChosen */, OnSelChosenRecordDateFilterOptions, VTS_I4)
	ON_EVENT(CExportWizardFiltersDlg, IDC_STANDARD_DATES_FROM, 16 /* SelChosen */, OnSelChosenStandardDatesFrom, VTS_I4)
	ON_EVENT(CExportWizardFiltersDlg, IDC_STANDARD_DATES_TO, 16 /* SelChosen */, OnSelChosenStandardDatesTo, VTS_I4)
	ON_EVENT(CExportWizardFiltersDlg, IDC_SORT_FIELDS_AVAIL, 2 /* SelChanged */, OnSelChangedSortFieldsAvail, VTS_I4)
	ON_EVENT(CExportWizardFiltersDlg, IDC_SORT_FIELDS_SELECT, 2 /* SelChanged */, OnSelChangedSortFieldsSelect, VTS_I4)
	ON_EVENT(CExportWizardFiltersDlg, IDC_SORT_FIELDS_AVAIL, 3 /* DblClickCell */, OnDblClickCellSortFieldsAvail, VTS_I4 VTS_I2)
	ON_EVENT(CExportWizardFiltersDlg, IDC_SORT_FIELDS_SELECT, 3 /* DblClickCell */, OnDblClickCellSortFieldsSelect, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CExportWizardFiltersDlg::OnAllNewRecords() 
{
	EnableWindows();

	if(((CExportWizardDlg*)GetParent())->m_ertRecordType == ertPatients ||
	   ((CExportWizardDlg*)GetParent())->m_ertRecordType == ertAppointments)
	{
		//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
		//UpdateData(TRUE); //This code was overwriting the set text and was causing the hyperlink NOT to toggle
		if(m_strDateHyperlink == TEXT_DATE_MODIFIED) //we're looking at the modified date
			((CExportWizardDlg*)GetParent())->m_efoFilterOption = efoAllNewModified;
		else
			((CExportWizardDlg*)GetParent())->m_efoFilterOption = efoAllNew;
		
		// Calculate hyperlink rectangles
		CWnd *pWnd;
		pWnd = GetDlgItem(IDC_DATE_HYPERLINK);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the hotlinks
			pWnd->GetWindowRect(m_rcDateHyperlink);
			ScreenToClient(&m_rcDateHyperlink);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
	}
	else {
		HideHyperlink();
		((CExportWizardDlg*)GetParent())->m_efoFilterOption = efoAllNew;
	}
}

void CExportWizardFiltersDlg::OnManualSelect() 
{
	EnableWindows();

	((CExportWizardDlg*)GetParent())->m_efoFilterOption = efoManual;

	//m.hancock PLID 17422 9/8/2005
	HideHyperlink();
}

void CExportWizardFiltersDlg::OnSelChosenRecordLwFilterOptions(long nRow) 
{
	try {
		if(nRow == -1) {
			m_pLwFilters->CurSel = 0;
			OnSelChosenRecordLwFilterOptions(0);
		}
		else {
			((CExportWizardDlg*)GetParent())->m_nLwFilterID = VarLong(m_pLwFilters->GetValue(nRow,0));
		}
	}NxCatchAll("Error in CExportWizardFiltersDlg::OnSelChosenRecordLwFilterOptions()");
}

void CExportWizardFiltersDlg::OnSelChosenRecordDateFilterOptions(long nRow) 
{
	if(nRow == -1) {
		m_pDateFilters->CurSel = 0;
		OnSelChosenRecordDateFilterOptions(0);
	}
	else {
		((CExportWizardDlg*)GetParent())->m_fdDateFilter = (FilterableDate)VarLong(m_pDateFilters->GetValue(nRow,0));
	}
}

void CExportWizardFiltersDlg::OnChangeRecordDateFilterFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	((CExportWizardDlg*)GetParent())->m_dtFilterFrom = m_dtFrom.GetValue();

	*pResult = 0;
}

void CExportWizardFiltersDlg::OnChangeRecordDateFilterTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	((CExportWizardDlg*)GetParent())->m_dtFilterTo = m_dtTo.GetValue();

	*pResult = 0;
}

BOOL CExportWizardFiltersDlg::OnSetActive()
{
	//Fill in our datalists based on the parent's type.
	m_pDateFilters->Clear();
	switch(((CExportWizardDlg*)GetParent())->m_ertRecordType) {
	case ertPatients:
		{
			m_pLwFilters->WhereClause = _bstr_t("Type = 1");
			m_pLwFilters->Requery();

			IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdFirstContactDate);
			pRow->PutValue(1, _bstr_t("First Contact Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdHasAppointmentDate);
			pRow->PutValue(1, _bstr_t("Has Appointment With Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdNextAppointmentDate);
			pRow->PutValue(1, _bstr_t("Next Appointment Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdLastAppointmentDate);
			pRow->PutValue(1, _bstr_t("Last Appointment Date"));
			m_pDateFilters->AddRow(pRow);
		}
		break;
	
	case ertAppointments:
		{
			m_pLwFilters->WhereClause = _bstr_t("Type = 1 OR Type = 3");
			m_pLwFilters->Requery();
			
			IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdAppointmentDate);
			pRow->PutValue(1, _bstr_t("Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdAppointmentInputDate);
			pRow->PutValue(1, _bstr_t("Input Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdFirstContactDate);
			pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
			m_pDateFilters->AddRow(pRow);

		}
		break;

	case ertCharges:
		{
			m_pLwFilters->WhereClause = _bstr_t("Type = 1");
			m_pLwFilters->Requery();

			IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdServiceDate);
			pRow->PutValue(1, _bstr_t("Service Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdInputDate);
			pRow->PutValue(1, _bstr_t("Input Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdBillDate);
			pRow->PutValue(1, _bstr_t("Bill Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdFirstContactDate);
			pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdHasAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient Has Appointment With Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdNextAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient's Next Appointment Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdLastAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient's Last Appointment Date"));
			m_pDateFilters->AddRow(pRow);
		}
		break;

	case ertPayments:
		{
			m_pLwFilters->WhereClause = _bstr_t("Type = 1 OR Type = 6");
			m_pLwFilters->Requery();

			IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdServiceDate);
			pRow->PutValue(1, _bstr_t("Service Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdInputDate);
			pRow->PutValue(1, _bstr_t("Input Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdFirstContactDate);
			pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdHasAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient Has Appointment With Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdNextAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient's Next Appointment Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdLastAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient's Last Appointment Date"));
			m_pDateFilters->AddRow(pRow);
		}
		break;
	case ertEMNs:
		{
			CString strFilterWhere;
			strFilterWhere.Format("Type IN (%i,%i,%i)", fboPerson,fboEMN,fboEMR);
			m_pLwFilters->WhereClause = _bstr_t(strFilterWhere);
			m_pLwFilters->Requery();

			IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdServiceDate);
			pRow->PutValue(1, _bstr_t("Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdInputDate);
			pRow->PutValue(1, _bstr_t("Input Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdFirstContactDate);
			pRow->PutValue(1, _bstr_t("Patient's First Contact Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdHasAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient Has Appointment With Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdNextAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient's Next Appointment Date"));
			m_pDateFilters->AddRow(pRow);
			pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdLastAppointmentDate);
			pRow->PutValue(1, _bstr_t("Patient's Last Appointment Date"));
			m_pDateFilters->AddRow(pRow);
		}
		break;

	// (z.manning 2009-12-10 14:31) - PLID 36519 - Added history export
	case ertHistory:
		{
			m_pLwFilters->WhereClause = _bstr_t("Type = 1");
			m_pLwFilters->Requery();

			IRowSettingsPtr pRow = m_pDateFilters->GetRow(-1);
			pRow->PutValue(0, (long)fdAttachDate);
			pRow->PutValue(1, _bstr_t("Attachment Date"));
			m_pDateFilters->AddRow(pRow);
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}

	//Now, set all our controls.
	if(((CExportWizardDlg*)GetParent())->m_efoFilterOption == efoManual) {
		CheckRadioButton(IDC_ALL_NEW_RECORDS, IDC_MANUAL_SELECT, IDC_MANUAL_SELECT);
	}
	// (z.manning 2009-12-14 09:42) - PLID 36576 - Date and LW filters are now one option
	else if(((CExportWizardDlg*)GetParent())->m_efoFilterOption == efoDateOrLwFilter) {
		CheckRadioButton(IDC_ALL_NEW_RECORDS, IDC_MANUAL_SELECT, IDC_EXPORT_DATE_LW_FILTER);
		long nFilterFlags = ((CExportWizardDlg*)GetParent())->m_nFilterFlags;
		if(nFilterFlags & effDate) {
			CheckDlgButton(IDC_DATE_FILTER_CHECK, BST_CHECKED);
		}
		else {
			CheckDlgButton(IDC_DATE_FILTER_CHECK, BST_UNCHECKED);
		}

		if(nFilterFlags & effLetterWriting) {
			CheckDlgButton(IDC_LW_FILTER_CHECK, BST_CHECKED);
		}
		else {
			CheckDlgButton(IDC_LW_FILTER_CHECK, BST_UNCHECKED);
		}
	}
	else {
		CheckRadioButton(IDC_ALL_NEW_RECORDS, IDC_MANUAL_SELECT, IDC_ALL_NEW_RECORDS);
	}

	if(-1 == m_pDateFilters->FindByColumn(0, (long)((CExportWizardDlg*)GetParent())->m_fdDateFilter, 0, VARIANT_TRUE)) {
		m_pDateFilters->CurSel = 0;
		OnSelChosenRecordDateFilterOptions(0);
	}
	if(-1 == m_pStandardDatesFrom->FindByColumn(0, (long)((CExportWizardDlg*)GetParent())->m_dfoFromDateType, 0, VARIANT_TRUE)) {
		m_pStandardDatesFrom->CurSel = 0;
		OnSelChosenStandardDatesFrom(0);
	}
	if(-1 == m_pStandardDatesTo->FindByColumn(0, (long)((CExportWizardDlg*)GetParent())->m_dfoToDateType, 0, VARIANT_TRUE)) {
		m_pStandardDatesTo->CurSel = 0;
		OnSelChosenStandardDatesTo(0);
	}

	SetDlgItemInt(IDC_FROM_PLUS_DAYS, ((CExportWizardDlg*)GetParent())->m_nFromDateOffset);
	SetDlgItemInt(IDC_TO_PLUS_DAYS, ((CExportWizardDlg*)GetParent())->m_nToDateOffset);

	m_dtFrom.SetValue(_variant_t(((CExportWizardDlg*)GetParent())->m_dtFilterFrom));
	m_dtTo.SetValue(_variant_t(((CExportWizardDlg*)GetParent())->m_dtFilterTo));

	m_pLwFilters->WaitForRequery(dlPatienceLevelWaitIndefinitely);
	
	//(c.copits 2011-06-30) PLID 44256 - If you don't have any LW filters, the Links->Export throws exceptions.
	if (m_pLwFilters->GetRowCount() > 0) {

		UINT nChecked = IsDlgButtonChecked(IDC_LW_FILTER_CHECK);
		GetDlgItem(IDC_LW_FILTER_CHECK)->EnableWindow(TRUE);
		GetDlgItem(IDC_RECORD_LW_FILTER_OPTIONS)->EnableWindow(TRUE);
		m_pLwFilters->PutEnabled(TRUE);
		CheckDlgButton(IDC_LW_FILTER_CHECK, nChecked);

		if(-1 == m_pLwFilters->FindByColumn(0, (long)((CExportWizardDlg*)GetParent())->m_nLwFilterID, 0, VARIANT_TRUE)) {
			m_pLwFilters->CurSel = 0;
			OnSelChosenRecordLwFilterOptions(0);
		}
	}
	else {
		GetDlgItem(IDC_LW_FILTER_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_RECORD_LW_FILTER_OPTIONS)->EnableWindow(FALSE);
		m_pLwFilters->PutEnabled(FALSE);
	}

	
	//Now, the sort area.
	//First, fill in our available field list.
	m_pSortFieldsAvail->Clear();
	//Now, go through all our fields and add them to the available list.
	DWORD dwRecordType = 0;
	switch(((CExportWizardDlg*)GetParent())->m_ertRecordType) {
	case ertPatients:
		dwRecordType = EFS_PATIENTS;
		break;
	case ertAppointments:
		dwRecordType = EFS_APPOINTMENTS;
		break;
	case ertCharges:
		dwRecordType = EFS_CHARGES;
		break;
	case ertPayments:
		dwRecordType = EFS_PAYMENTS;
		break;
	case ertEMNs:
		dwRecordType = EFS_EMNS;
		break;
	// (z.manning 2009-12-11 09:58) - PLID 36519 - History exports don't allow sorting so no need
	// to handle that here.
	}
	
	// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
	int i = 0;

	for(i = 0; i < g_nExportFieldCount; i++) {
		if(g_arExportFields[i].dwSupportedRecordTypes & dwRecordType) {
			//Don't add the placeholder field.
			CExportField ef = g_arExportFields[i];
			if(ef.eftType != eftPlaceholder && ef.eftType != eftGenericNtext && ef.eftType != eftEmnItem) { //we can't order by ntext types, and we probably could order by EMN Items somehow, but it would be way too hard to bother with, at least for now.
				IRowSettingsPtr pRow = m_pSortFieldsAvail->GetRow(-1);
				pRow->PutValue(0, (long)ef.nID);
				pRow->PutValue(1, _bstr_t(ef.GetDisplayName()));
				pRow->PutValue(2, (long)ef.eftType);
				m_pSortFieldsAvail->AddRow(pRow);
			}
		}
	}

	//Now, go through any fields that should be in the selected list, and move them over.
	m_pSortFieldsSelect->Clear();
	if(m_nBasedOn != -1 && m_nBasedOn != ((CExportWizardDlg*)GetParent())->m_ertRecordType) {
		//The based on has changed since last time we were on this tab, so the array of sort fields is no longer valid.
		((CExportWizardDlg*)GetParent())->m_arSortFields.RemoveAll();
	}
	m_nBasedOn = ((CExportWizardDlg*)GetParent())->m_ertRecordType;

	for(i = 0; i < ((CExportWizardDlg*)GetParent())->m_arSortFields.GetSize(); i++) {
		SortField sf = ((CExportWizardDlg*)GetParent())->m_arSortFields.GetAt(i);
		//Find this field in the available list.
		long nAvailRow = m_pSortFieldsAvail->FindByColumn(0,sf.nID,0,FALSE);
		if(nAvailRow == -1) {
			//That shouldn't be possible!
			ASSERT(FALSE);
		}
		else {
			//Add it to the select list.
			IRowSettingsPtr pRow = m_pSortFieldsSelect->GetRow(-1);
			IRowSettingsPtr pSourceRow = m_pSortFieldsAvail->GetRow(nAvailRow);
			pRow->PutValue(0,pSourceRow->GetValue(0));
			pRow->PutValue(1,pSourceRow->GetValue(1));
			pRow->PutValue(2,pSourceRow->GetValue(2));
			pRow->PutValue(3,_bstr_t(sf.strExtraInfo));
			pRow->PutValue(4,sf.bSortDescending);
			m_pSortFieldsSelect->AddRow(pRow);
			long nType = VarLong(pSourceRow->GetValue(2));
			if(nType != eftAdvanced) {
				m_pSortFieldsAvail->RemoveRow(nAvailRow);
			}
		}
	}

	CheckDlgButton(IDC_MANUAL_SORT, ((CExportWizardDlg*)GetParent())->m_bManualSort?BST_CHECKED:BST_UNCHECKED);

	EnableWindows();

	//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
	//Hide the hyperlink if we're dealing with payments and charges
	switch(((CExportWizardDlg*)GetParent())->m_ertRecordType) {
		case ertPatients:
		case ertAppointments:
			break;
		case ertCharges:
		case ertPayments:
		case ertEMNs:
		case ertHistory: // (z.manning 2009-12-11 09:59) - PLID 36519
		default:
			HideHyperlink();
			break;
	}

	//They can always go next or back.
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
	((CExportWizardDlg*)GetParent())->SetTitle("Select records for export \"" + ((CExportWizardDlg*)GetParent())->m_strName + "\"");


	return CPropertyPage::OnSetActive();
}

void CExportWizardFiltersDlg::OnSelChosenStandardDatesFrom(long nRow) 
{
	if(nRow == -1) {
		m_pStandardDatesFrom->CurSel = 0;
		OnSelChosenStandardDatesFrom(0);
	}
	else {
		((CExportWizardDlg*)GetParent())->m_dfoFromDateType = (DateFilterOption)VarLong(m_pStandardDatesFrom->GetValue(nRow,0));
	}

	EnableWindows();
}

void CExportWizardFiltersDlg::OnSelChosenStandardDatesTo(long nRow) 
{
	if(nRow == -1) {
		m_pStandardDatesTo->CurSel = 0;
		OnSelChosenStandardDatesTo(0);
	}
	else {
		((CExportWizardDlg*)GetParent())->m_dfoToDateType = (DateFilterOption)VarLong(m_pStandardDatesTo->GetValue(nRow,0));
	}

	EnableWindows();
}

void CExportWizardFiltersDlg::OnChangeFromPlusDays() 
{
	int nOffset = GetDlgItemInt(IDC_FROM_PLUS_DAYS);
	if(nOffset > 1000) {
		AfxMessageBox("You cannot have a date offset of more than 1000 days.");
		nOffset = 0;
		SetDlgItemInt(IDC_FROM_PLUS_DAYS, nOffset);
	}

	((CExportWizardDlg*)GetParent())->m_nFromDateOffset = nOffset;
	
}

void CExportWizardFiltersDlg::OnChangeToPlusDays() 
{
	int nOffset = GetDlgItemInt(IDC_TO_PLUS_DAYS);
	if(nOffset > 1000) {
		AfxMessageBox("You cannot have a date offset of more than 1000 days.");
		nOffset = 0;
		SetDlgItemInt(IDC_TO_PLUS_DAYS, nOffset);
	}

	((CExportWizardDlg*)GetParent())->m_nToDateOffset = nOffset;
}

void CExportWizardFiltersDlg::EnableWindows()
{
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);

	//Enable what should be enabled.
	if(IsDlgButtonChecked(IDC_ALL_NEW_RECORDS) || IsDlgButtonChecked(IDC_MANUAL_SELECT)) {
		GetDlgItem(IDC_RECORD_DATE_FILTER_OPTIONS)->EnableWindow(FALSE);
		GetDlgItem(IDC_STANDARD_DATES_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_FROM_PLUS_DAYS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RECORD_DATE_FILTER_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_STANDARD_DATES_TO)->EnableWindow(FALSE);
		GetDlgItem(IDC_TO_PLUS_DAYS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RECORD_DATE_FILTER_TO)->EnableWindow(FALSE);

		GetDlgItem(IDC_RECORD_LW_FILTER_OPTIONS)->EnableWindow(FALSE);

		GetDlgItem(IDC_DATE_FILTER_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_LW_FILTER_CHECK)->EnableWindow(FALSE);
	}
	else if(IsDlgButtonChecked(IDC_EXPORT_DATE_LW_FILTER)) {
		GetDlgItem(IDC_DATE_FILTER_CHECK)->EnableWindow(TRUE);
		//(c.copits 2011-06-30) PLID 44256 - If you don't have any LW filters, the Links->Export throws exceptions.
		if (m_pLwFilters->GetRowCount() > 0) {
			GetDlgItem(IDC_LW_FILTER_CHECK)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_LW_FILTER_CHECK)->EnableWindow(FALSE);
		}

		if(IsDlgButtonChecked(IDC_DATE_FILTER_CHECK)) {
			GetDlgItem(IDC_RECORD_DATE_FILTER_OPTIONS)->EnableWindow(TRUE);
			GetDlgItem(IDC_STANDARD_DATES_FROM)->EnableWindow(TRUE);
			GetDlgItem(IDC_FROM_PLUS_DAYS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECORD_DATE_FILTER_FROM)->EnableWindow(TRUE);
			GetDlgItem(IDC_STANDARD_DATES_TO)->EnableWindow(TRUE);
			GetDlgItem(IDC_TO_PLUS_DAYS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECORD_DATE_FILTER_TO)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_RECORD_DATE_FILTER_OPTIONS)->EnableWindow(FALSE);
			GetDlgItem(IDC_STANDARD_DATES_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_FROM_PLUS_DAYS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECORD_DATE_FILTER_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_STANDARD_DATES_TO)->EnableWindow(FALSE);
			GetDlgItem(IDC_TO_PLUS_DAYS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECORD_DATE_FILTER_TO)->EnableWindow(FALSE);
		}
		
		//(c.copits 2011-06-30) PLID 44256 - If you don't have any LW filters, the Links->Export throws exceptions.
		if(IsDlgButtonChecked(IDC_LW_FILTER_CHECK)) {
			GetDlgItem(IDC_RECORD_LW_FILTER_OPTIONS)->EnableWindow(TRUE);
			m_pLwFilters->PutEnabled(TRUE);
		}
		else {
			GetDlgItem(IDC_RECORD_LW_FILTER_OPTIONS)->EnableWindow(FALSE);
			m_pLwFilters->PutEnabled(FALSE);
		}

		// (z.manning 2009-12-14 12:20) - PLID 36576 - Don't let them go next unless they have
		// at least one of the filter options checked.
		if(!IsDlgButtonChecked(IDC_DATE_FILTER_CHECK) && !IsDlgButtonChecked(IDC_LW_FILTER_CHECK)) {
			((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK);
		}

		//m.hancock PLID 17422 9/8/2005
		HideHyperlink();
	}

	// (z.manning 2009-12-11 09:50) - PLID 36519 - Hide sort options for history export
	UINT nSortShowCmd = SW_SHOW;
	if(((CExportWizardDlg*)GetParent())->m_ertRecordType == ertHistory) {
		nSortShowCmd = SW_HIDE;
	}
	GetDlgItem(IDC_MANUAL_SELECT)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_ORDER_GROUPBOX)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_MANUAL_SORT)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_SORT_FIELDS_AVAIL)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_SORT_FIELDS_SELECT)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_ADVANCED_SORT_FIELD)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_ADVANCED_SORT_LABEL)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_ADD_SORT_FIELD)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_REMOVE_SORT_FIELD)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_SORT_FIELD_UP)->ShowWindow(nSortShowCmd);
	GetDlgItem(IDC_SORT_FIELD_DOWN)->ShowWindow(nSortShowCmd);

	if(IsDlgButtonChecked(IDC_MANUAL_SORT)) {
		GetDlgItem(IDC_SORT_FIELDS_AVAIL)->EnableWindow(FALSE);
		GetDlgItem(IDC_SORT_FIELDS_SELECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVANCED_SORT_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVANCED_SORT_LABEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADD_SORT_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_SORT_FIELD)->EnableWindow(FALSE);
		GetDlgItem(IDC_SORT_FIELD_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_SORT_FIELD_DOWN)->EnableWindow(FALSE);

		//m.hancock PLID 17422 9/8/2005
		HideHyperlink();
	}
	else {
		GetDlgItem(IDC_SORT_FIELDS_AVAIL)->EnableWindow(TRUE);
		GetDlgItem(IDC_SORT_FIELDS_SELECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVANCED_SORT_FIELD)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVANCED_SORT_LABEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADD_SORT_FIELD)->EnableWindow(m_pSortFieldsAvail->CurSel != -1);
		GetDlgItem(IDC_REMOVE_SORT_FIELD)->EnableWindow(m_pSortFieldsSelect->CurSel != -1);
		GetDlgItem(IDC_SORT_FIELD_UP)->EnableWindow(m_pSortFieldsSelect->CurSel > 0);
		GetDlgItem(IDC_SORT_FIELD_DOWN)->EnableWindow(m_pSortFieldsSelect->CurSel != -1 && m_pSortFieldsSelect->CurSel < m_pSortFieldsSelect->GetRowCount()-1);
	}
	
	

	//Show what should be shown.
	if(m_pStandardDatesFrom->CurSel != -1 && VarLong(m_pStandardDatesFrom->GetValue(m_pStandardDatesFrom->CurSel,0)) == dfoCustom) {
		GetDlgItem(IDC_FROM_PLUS_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FROM_PLUS_DAYS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FROM_DAYS_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RECORD_DATE_FILTER_FROM)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_FROM_PLUS_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_FROM_PLUS_DAYS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_FROM_DAYS_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_RECORD_DATE_FILTER_FROM)->ShowWindow(SW_HIDE);
	}
	if(m_pStandardDatesTo->CurSel != -1 && VarLong(m_pStandardDatesTo->GetValue(m_pStandardDatesTo->CurSel,0)) == dfoCustom) {
		GetDlgItem(IDC_TO_PLUS_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TO_PLUS_DAYS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TO_DAYS_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RECORD_DATE_FILTER_TO)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_TO_PLUS_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TO_PLUS_DAYS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TO_DAYS_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_RECORD_DATE_FILTER_TO)->ShowWindow(SW_HIDE);
	}
	if(IsDlgButtonChecked(IDC_MANUAL_SELECT)) {
		GetDlgItem(IDC_MANUAL_SORT)->ShowWindow(SW_SHOW);
		//m.hancock PLID 17422 9/8/2005
		HideHyperlink();
	}
	else {
		if(IsDlgButtonChecked(IDC_MANUAL_SORT)) {
			CheckDlgButton(IDC_MANUAL_SORT, BST_UNCHECKED);
			OnManualSort();
		}
		GetDlgItem(IDC_MANUAL_SORT)->ShowWindow(SW_HIDE);
	}

	if(m_pSortFieldsSelect->CurSel != -1 && VarLong(m_pSortFieldsSelect->GetValue(m_pSortFieldsSelect->CurSel,2)) == eftAdvanced) {
		GetDlgItem(IDC_ADVANCED_SORT_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ADVANCED_SORT_FIELD)->ShowWindow(SW_SHOW);
	}
	else {
		GetDlgItem(IDC_ADVANCED_SORT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADVANCED_SORT_FIELD)->ShowWindow(SW_HIDE);
	}
}

void CExportWizardFiltersDlg::OnManualSort() 
{
	EnableWindows();
}

void CExportWizardFiltersDlg::OnAddSortField() 
{
	if(m_pSortFieldsAvail->CurSel == -1) return;

	IRowSettingsPtr pRowSource = m_pSortFieldsAvail->GetRow(m_pSortFieldsAvail->CurSel);
	IRowSettingsPtr pRow = m_pSortFieldsSelect->GetRow(-1);
	pRow->PutValue(0,pRowSource->GetValue(0));
	pRow->PutValue(1,pRowSource->GetValue(1));
	pRow->PutValue(2,pRowSource->GetValue(2));
	pRow->PutValue(3,_bstr_t(""));
	pRow->PutValue(4,false);
	m_pSortFieldsSelect->AddRow(pRow);
	if(VarLong(pRowSource->GetValue(2)) != eftAdvanced) {
		m_pSortFieldsAvail->RemoveRow(m_pSortFieldsAvail->CurSel);
	}

	EnableWindows();
}

void CExportWizardFiltersDlg::OnRemoveSortField() 
{
	if(m_pSortFieldsSelect->CurSel == -1) return;

	IRowSettingsPtr pRowSource = m_pSortFieldsSelect->GetRow(m_pSortFieldsSelect->CurSel);
	if(VarLong(pRowSource->GetValue(2)) != eftAdvanced) {
		IRowSettingsPtr pRow = m_pSortFieldsAvail->GetRow(-1);
		pRow->PutValue(0,pRowSource->GetValue(0));
		pRow->PutValue(1,pRowSource->GetValue(1));
		pRow->PutValue(2,pRowSource->GetValue(2));
		m_pSortFieldsAvail->AddRow(pRow);
	}
	m_pSortFieldsSelect->RemoveRow(m_pSortFieldsSelect->CurSel);

	EnableWindows();
}

void CExportWizardFiltersDlg::OnSortFieldUp() 
{
	if(m_pSortFieldsSelect->CurSel == -1 || m_pSortFieldsSelect->CurSel == 0) return;

	long nInitialCurSel = m_pSortFieldsSelect->CurSel;
	m_pSortFieldsSelect->TakeRowInsert(m_pSortFieldsSelect->GetRow(m_pSortFieldsSelect->CurSel),m_pSortFieldsSelect->CurSel-1);
	m_pSortFieldsSelect->CurSel = nInitialCurSel-1;
	
	EnableWindows();
}

void CExportWizardFiltersDlg::OnSortFieldDown() 
{
	if(m_pSortFieldsSelect->CurSel == -1 || m_pSortFieldsSelect->CurSel == m_pSortFieldsSelect->GetRowCount()-1) return;

	long nInitialCurSel = m_pSortFieldsSelect->CurSel;
	m_pSortFieldsSelect->TakeRowInsert(m_pSortFieldsSelect->GetRow(m_pSortFieldsSelect->CurSel),m_pSortFieldsSelect->CurSel+2);
	m_pSortFieldsSelect->CurSel = nInitialCurSel+1;

	EnableWindows();
}

void CExportWizardFiltersDlg::OnSelChangedSortFieldsAvail(long nNewSel) 
{
	EnableWindows();	
}

void CExportWizardFiltersDlg::OnSelChangedSortFieldsSelect(long nNewSel) 
{
	EnableWindows();

	if(nNewSel != -1 && VarLong(m_pSortFieldsSelect->GetValue(nNewSel,2)) == eftAdvanced) {
		SetDlgItemText(IDC_ADVANCED_SORT_FIELD,VarString(m_pSortFieldsSelect->GetValue(nNewSel,3),""));
	}
}

void CExportWizardFiltersDlg::OnChangeAdvancedSortField() 
{
	CString strField;
	GetDlgItemText(IDC_ADVANCED_SORT_FIELD,strField);
	m_pSortFieldsSelect->PutValue(m_pSortFieldsSelect->CurSel,3,_bstr_t(strField));
}

void CExportWizardFiltersDlg::OnDblClickCellSortFieldsAvail(long nRowIndex, short nColIndex) 
{
	OnAddSortField();
}

void CExportWizardFiltersDlg::OnDblClickCellSortFieldsSelect(long nRowIndex, short nColIndex) 
{
	OnRemoveSortField();
}

BOOL CExportWizardFiltersDlg::OnKillActive()
{
	//The filters will have already updated things, we just need to update the sort info.
	((CExportWizardDlg*)GetParent())->m_bManualSort = IsDlgButtonChecked(IDC_MANUAL_SORT)?true:false;
	((CExportWizardDlg*)GetParent())->m_arSortFields.RemoveAll();

	//If there are any advanced sort fields, we should check that they're valid (we can't check them individually, because
	//in some cases, like when there are two identical fields, they may each succeed individually, and only fail when used
	//together).
	bool bNeedValidate = false;
	CString strOrder = "ORDER BY ";
	// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
	int i = 0;

	for(i = 0; i < m_pSortFieldsSelect->GetRowCount(); i++) {
		SortField sf;
		sf.nID = VarLong(m_pSortFieldsSelect->GetValue(i,0));
		sf.bSortDescending = VarBool(m_pSortFieldsSelect->GetValue(i,4))?true:false;
		sf.strExtraInfo = VarString(m_pSortFieldsSelect->GetValue(i,3),"");
		
		CString str;
		CString strField;
		//Check whether it's an invalid sort field.
		CExportField ef = GetFieldByID(sf.nID);
		if(ef.eftType == eftAdvanced) {
			bNeedValidate = true;
			strField = sf.strExtraInfo;
		}
		else {
			strField = ef.GetField(); // (a.walling 2011-03-11 15:24) - PLID 42786 - GetField may format the strField further for SSNs etc
		}
		str.Format("%s %s, ", strField, sf.bSortDescending?"DESC":"ASC");
		strOrder += str;
			
		((CExportWizardDlg*)GetParent())->m_arSortFields.Add(sf);
	}
	if(bNeedValidate) {
		strOrder = strOrder.Left(strOrder.GetLength() - 2);
		try {
			CString strSelect = "SELECT TOP 1 ";
			for(i = 0; i < ((CExportWizardDlg*)GetParent())->m_arExportFields.GetSize(); i++) {
				CExportField ef = GetFieldByID(((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).nID);
				CString str, strField;
				if(ef.eftType == eftAdvanced) {
					strField = GetNthField(((CExportWizardDlg*)GetParent())->m_arExportFields.GetAt(i).strFormat,3,"");
				}
				else {
					strField = ef.GetField(); // (a.walling 2011-03-11 15:24) - PLID 42786 - GetField may format the strField further for SSNs etc
				}
				str.Format("%s AS Field%i, ", strField, i);
				strSelect += str;
			}
			strSelect = strSelect.Left(strSelect.GetLength()-2);
			CreateRecordset("%s %s %s", strSelect, GetExportFromClause(((CExportWizardDlg*)GetParent())->m_ertRecordType), strOrder);
		}
		catch(_com_error e) {
			MessageBox("The syntax entered for one or more {Advanced} sort fields results in an invalid query.  Please correct this problem before continuing.");
			return FALSE;
		}
	}

	return TRUE;
}

//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
void CExportWizardFiltersDlg::ChangeDateHyperlink()
{
	//UpdateData(TRUE); //This code was overwriting the set text and was causing the hyperlink NOT to toggle
	//Set the proper hyperlink text
	if(m_strDateHyperlink == TEXT_DATE_CREATED) {
		m_strDateHyperlink = TEXT_DATE_MODIFIED;
	} else {
		m_strDateHyperlink = TEXT_DATE_CREATED;
	}
	InvalidateRect(m_rcDateHyperlink);

	OnAllNewRecords();
}

void CExportWizardFiltersDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
	DrawDateHyperlink(&dc);
	
	// Do not call CPropertyPage::OnPaint() for painting messages
}

//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
void CExportWizardFiltersDlg::DrawDateHyperlink(CDC *pdc)
{
	if(IsDlgButtonChecked(IDC_ALL_NEW_RECORDS))
	{
		// Draw the hyperlink

		//TODO: if we change the background of this screen to not be the system gray color,
		//we need to make sure this hyperlink uses the new background color
		DrawTextOnDialog(this, pdc, m_rcDateHyperlink, m_strDateHyperlink, dtsHyperlink, true, DT_CENTER, true, false, GetSysColor(COLOR_BTNFACE));
	}
}

//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
void CExportWizardFiltersDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);

	CPropertyPage::OnLButtonDown(nFlags, point);
}

//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
void CExportWizardFiltersDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	if( (((CExportWizardDlg*)GetParent())->m_ertRecordType == ertPatients || 
		((CExportWizardDlg*)GetParent())->m_ertRecordType == ertAppointments) &&
		IsDlgButtonChecked(IDC_ALL_NEW_RECORDS))
	{
		if (m_rcDateHyperlink.PtInRect(point))
			ChangeDateHyperlink();
	}
}

//m.hancock PLID 17422 9/2/2005 - Add a hyperlink to designate exports based on created date or modified date
BOOL CExportWizardFiltersDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	
	if( (((CExportWizardDlg*)GetParent())->m_ertRecordType == ertPatients || 
		((CExportWizardDlg*)GetParent())->m_ertRecordType == ertAppointments) &&
		IsDlgButtonChecked(IDC_ALL_NEW_RECORDS))
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		if (m_rcDateHyperlink.PtInRect(pt)) 
		{
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return CPropertyPage::OnSetCursor(pWnd, nHitTest, message);
}

//m.hancock PLID 17422 9/8/2005 - Need to prevent the hyperlink from showing and show the text instead
void CExportWizardFiltersDlg::HideHyperlink()
{
	//set the text to be displayed
	SetDlgItemText(IDC_DATE_HYPERLINK, m_strDateHyperlink);

	InvalidateRect(m_rcDateHyperlink);
	GetDlgItem(IDC_DATE_HYPERLINK)->ShowWindow(SW_SHOW);
}

// (z.manning 2009-12-14 11:43) - PLID 36576
void CExportWizardFiltersDlg::OnBnClickedDateFilterCheck()
{
	try
	{
		EnableWindows();

		if(IsDlgButtonChecked(IDC_DATE_FILTER_CHECK)) {
			((CExportWizardDlg*)GetParent())->m_nFilterFlags |= effDate;
		}
		else {
			((CExportWizardDlg*)GetParent())->m_nFilterFlags &= ~effDate;
		}

		//m.hancock PLID 17422 9/8/2005
		HideHyperlink();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-12-14 11:43) - PLID 36576
void CExportWizardFiltersDlg::OnBnClickedLwFilterCheck()
{
	try
	{
		EnableWindows();

		if(IsDlgButtonChecked(IDC_LW_FILTER_CHECK)) {
			((CExportWizardDlg*)GetParent())->m_nFilterFlags |= effLetterWriting;
		}
		else {
			((CExportWizardDlg*)GetParent())->m_nFilterFlags &= ~effLetterWriting;
		}

		//m.hancock PLID 17422 9/8/2005
		HideHyperlink();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-12-14 11:43) - PLID 36576
void CExportWizardFiltersDlg::OnBnClickedExportDateLwFilter()
{
	try
	{
		EnableWindows();

		((CExportWizardDlg*)GetParent())->m_efoFilterOption = efoDateOrLwFilter;

		//m.hancock PLID 17422 9/8/2005
		HideHyperlink();

	}NxCatchAll(__FUNCTION__);
}
