// AttendanceCustomDateRangeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AttendanceCustomDateRangeSetupDlg.h"
#include "AttendanceUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAttendanceCustomDateRangeSetupDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29139 - Created

CAttendanceCustomDateRangeSetupDlg::CAttendanceCustomDateRangeSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAttendanceCustomDateRangeSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttendanceCustomDateRangeSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAttendanceCustomDateRangeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttendanceCustomDateRangeSetupDlg)
	DDX_Control(pDX, IDC_RAD_PER_MONTH, m_btnMonthly);
	DDX_Control(pDX, IDC_RAD_WEEKLY, m_btnWeekly);
	DDX_Control(pDX, IDC_NUMBER_OF_TIMES_PER_MONTH, m_nxeditNumberOfTimesPerMonth);
	DDX_Control(pDX, IDC_NUMBER_OF_WEEKS, m_nxeditNumberOfWeeks);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAttendanceCustomDateRangeSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAttendanceCustomDateRangeSetupDlg)
	ON_BN_CLICKED(IDC_RAD_PER_MONTH, OnRadPerMonth)
	ON_BN_CLICKED(IDC_RAD_WEEKLY, OnRadWeekly)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttendanceCustomDateRangeSetupDlg message handlers

BOOL CAttendanceCustomDateRangeSetupDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		Load();

		UpdateView();
	
	}NxCatchAll("CAttendanceCustomDateRangeSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttendanceCustomDateRangeSetupDlg::OnOK() 
{
	try
	{
		if(!Validate()) {
			return;
		}

		Save();
	
		CDialog::OnOK();

	}NxCatchAll("CAttendanceCustomDateRangeSetupDlg::OnOK");
}

void CAttendanceCustomDateRangeSetupDlg::Load()
{
	ECustomDateRangeTypes nType;
	int nFrequency;
	LoadAttendanceCustomDateRangeProperty(nType, nFrequency);
	switch(nType)
	{
		case cdrtMonthly:
			CheckDlgButton(IDC_RAD_PER_MONTH, BST_CHECKED);
			SetDlgItemInt(IDC_NUMBER_OF_TIMES_PER_MONTH, nFrequency);
			SetDlgItemInt(IDC_NUMBER_OF_WEEKS, 2);
			break;

		case cdrtWeekly:
			CheckDlgButton(IDC_RAD_WEEKLY, BST_CHECKED);
			SetDlgItemInt(IDC_NUMBER_OF_WEEKS, nFrequency);
			SetDlgItemInt(IDC_NUMBER_OF_TIMES_PER_MONTH, 2);
			break;

		default:
			ASSERT(FALSE);
			break;
	}
}

void CAttendanceCustomDateRangeSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (z.manning, 11/27/2007) - Disable all non-radio button controls and then enable the appropriate one based on
	// the current selection.
	GetDlgItem(IDC_NUMBER_OF_TIMES_PER_MONTH)->EnableWindow(FALSE);
	GetDlgItem(IDC_NUMBER_OF_WEEKS)->EnableWindow(FALSE);

	if(IsDlgButtonChecked(IDC_RAD_PER_MONTH) == BST_CHECKED) {
		GetDlgItem(IDC_NUMBER_OF_TIMES_PER_MONTH)->EnableWindow(TRUE);
	}
	else if(IsDlgButtonChecked(IDC_RAD_WEEKLY) == BST_CHECKED) {
		GetDlgItem(IDC_NUMBER_OF_WEEKS)->EnableWindow(TRUE);
	}
}

BOOL CAttendanceCustomDateRangeSetupDlg::Validate()
{
	if(IsDlgButtonChecked(IDC_RAD_PER_MONTH) == BST_CHECKED)
	{
		int nMonths = GetDlgItemInt(IDC_NUMBER_OF_TIMES_PER_MONTH);
		if(nMonths <= 0 || nMonths > 15) {
			MessageBox("Please enter a month value between 1 and 15.");
			return FALSE;
		}
	}
	else if(IsDlgButtonChecked(IDC_RAD_WEEKLY) == BST_CHECKED)
	{
		int nWeeks = GetDlgItemInt(IDC_NUMBER_OF_WEEKS);
		if(nWeeks <= 0 || nWeeks > 52) {
			MessageBox("Please enter a week value between 1 and 52.");
			return FALSE;
		}
	}

	return TRUE;
}

void CAttendanceCustomDateRangeSetupDlg::Save()
{
	int nType, nNumber;
	if(IsDlgButtonChecked(IDC_RAD_PER_MONTH) == BST_CHECKED)
	{
		nType = cdrtMonthly;
		nNumber = GetDlgItemInt(IDC_NUMBER_OF_TIMES_PER_MONTH);
	}
	else if(IsDlgButtonChecked(IDC_RAD_WEEKLY) == BST_CHECKED)
	{
		nType = cdrtWeekly;
		nNumber = GetDlgItemInt(IDC_NUMBER_OF_WEEKS);
	}
	CString strProperty = FormatString("%i;%i", nType, nNumber);
	SetRemotePropertyText("AttendanceCustomDateRange", strProperty, 0, "<None>");
}

void CAttendanceCustomDateRangeSetupDlg::OnRadPerMonth() 
{
	try
	{
		UpdateView();

	}NxCatchAll("CAttendanceCustomDateRangeSetupDlg::OnRadPerMonth");
}

void CAttendanceCustomDateRangeSetupDlg::OnRadWeekly() 
{
	try
	{
		UpdateView();

	}NxCatchAll("CAttendanceCustomDateRangeSetupDlg::OnRadWeekly");
}
