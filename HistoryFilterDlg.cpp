// HistoryFilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HistoryFilterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHistoryFilterDlg dialog


CHistoryFilterDlg::CHistoryFilterDlg(CWnd* pParent)
	: CNxDialog(CHistoryFilterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHistoryFilterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHistoryFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHistoryFilterDlg)
	DDX_Control(pDX, IDC_FROM_DATE_FILTER, m_dtFrom);
	DDX_Control(pDX, IDC_TO_DATE_FILTER, m_dtTo);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRINT_HEADER, m_checkPrintHeader);
	DDX_Control(pDX, IDC_ALL_DATES_FILTER, m_radioAllDatesFilter);
	DDX_Control(pDX, IDC_FILTER_DATES, m_radioFilterDates);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHistoryFilterDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHistoryFilterDlg)
	ON_BN_CLICKED(IDC_ALL_DATES_FILTER, OnAllDatesFilter)
	ON_BN_CLICKED(IDC_FILTER_DATES, OnFilterDates)
	ON_BN_CLICKED(IDC_PRINT_HEADER, OnPrintHeader)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHistoryFilterDlg message handlers

void CHistoryFilterDlg::OnOK() 
{
	//save the currently chosen location
	if(IsDlgButtonChecked(IDC_PRINT_HEADER) && m_pLocations->CurSel > -1) {
		SetRemotePropertyInt("HistoryFilterLocation", VarLong(m_pLocations->GetValue(m_pLocations->CurSel, 0),-1), 0, "<None>");
		m_strLocationName = VarString(m_pLocations->GetValue(m_pLocations->CurSel, 1), "");
	}
	else {
		//either nothing selected, or it's unchecked
		SetRemotePropertyInt("HistoryFilterLocation", -1, 0, "<None>");
		m_strLocationName = "";
	}

	//save formatted dates that can be pulled out
	if(IsDlgButtonChecked(IDC_FILTER_DATES)) {
		m_dtDateFrom = (COleDateTime)m_dtFrom.GetValue();
		m_dtDateTo = (COleDateTime)m_dtTo.GetValue();
		m_bUseDates = true;
	}
	else
		m_bUseDates = false;

	CNxDialog::OnOK();
}

void CHistoryFilterDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

BOOL CHistoryFilterDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 12:43) - PLID 29866 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pLocations = BindNxDataListCtrl(IDC_LOCATIONS_LIST);

		//if there is a location chosen in ConfigRT, check the box and set the selection to that location
		long nLoc = GetRemotePropertyInt("HistoryFilterLocation",0,0,"<None>",true);
		if(nLoc > 0) {
			CheckDlgButton(IDC_PRINT_HEADER, true);
			m_pLocations->SetSelByColumn(0, long(nLoc));
		}
		else {
			//leave the button unchecked, disable the location box
			m_pLocations->Enabled = false;
		}

		//setup our dt pickers
		m_dtFrom.SetValue(_variant_t(COleDateTime::GetCurrentTime()));
		m_dtTo.SetValue(_variant_t(COleDateTime::GetCurrentTime()));

		//enable all dates + disable the dt pickers
		CheckDlgButton(IDC_ALL_DATES_FILTER, true);
		EnsureFilters();
	}
	NxCatchAll("Error in CHistoryFilterDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHistoryFilterDlg::OnAllDatesFilter() 
{
	//filter all dates, disable the date pickers
	EnsureFilters();
}

void CHistoryFilterDlg::OnFilterDates() 
{
	//filter just certain dates, enable the date pickers
	EnsureFilters();
}

void CHistoryFilterDlg::OnPrintHeader() 
{
	if(IsDlgButtonChecked(IDC_PRINT_HEADER)) {
		//they've chosen to print a header, enable the datalist
		m_pLocations->Enabled = true;
	}
	else
		m_pLocations->Enabled = false;
}

//ensures that the date picker buttons are in the correct enabled/disabled state depending
//on the radio button selected
void CHistoryFilterDlg::EnsureFilters()
{
	if(IsDlgButtonChecked(IDC_ALL_DATES_FILTER)) {
		m_dtFrom.EnableWindow(false);
		m_dtTo.EnableWindow(false);
	}
	else {
		m_dtFrom.EnableWindow(true);
		m_dtTo.EnableWindow(true);
	}
}

BEGIN_EVENTSINK_MAP(CHistoryFilterDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHistoryFilterDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

