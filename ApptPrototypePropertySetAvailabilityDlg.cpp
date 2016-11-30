// ApptPrototypePropertySetAvailabilityDlg.cpp : implementation file
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
//

#include "stdafx.h"
#include "Practice.h"
#include "ApptPrototypePropertySetAvailabilityDlg.h"
#include "ApptPrototypeUtils.h"


// CApptPrototypePropertySetAvailabilityDlg dialog

IMPLEMENT_DYNAMIC(CApptPrototypePropertySetAvailabilityDlg, CNxDialog)

CApptPrototypePropertySetAvailabilityDlg::CApptPrototypePropertySetAvailabilityDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptPrototypePropertySetAvailabilityDlg::IDD, pParent)
{
	m_nAvailDays = 0;
	m_nAvailTimes = 0;
}

CApptPrototypePropertySetAvailabilityDlg::~CApptPrototypePropertySetAvailabilityDlg()
{
}

void CApptPrototypePropertySetAvailabilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CApptPrototypePropertySetAvailabilityDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CApptPrototypePropertySetAvailabilityDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CApptPrototypePropertySetAvailabilityDlg message handlers

BOOL CApptPrototypePropertySetAvailabilityDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (b.cardillo 2011-02-28 16:51) - Oops, forgot the button styles on this dialog for PLID 40419
		// Set button styles
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CheckDlgButton(IDC_DAYS_SUNDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Sunday) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DAYS_MONDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Monday) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DAYS_TUESDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Tuesday) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DAYS_WEDNESDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Wednesday) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DAYS_THURSDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Thursday) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DAYS_FRIDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Friday) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_DAYS_SATURDAY_CHECK, (m_nAvailDays & EApptPrototypeAllowDays::Saturday) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_TIMES_EARLYMORNING_CHECK, (m_nAvailTimes & EApptPrototypeAllowTimes::EarlyMorning) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_TIMES_LATEMORNING_CHECK, (m_nAvailTimes & EApptPrototypeAllowTimes::LateMorning) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_TIMES_EARLYAFTERNOON_CHECK, (m_nAvailTimes & EApptPrototypeAllowTimes::EarlyAfternoon) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_TIMES_LATEAFTERNOON_CHECK, (m_nAvailTimes & EApptPrototypeAllowTimes::LateAfternoon) ? BST_CHECKED : BST_UNCHECKED);

	} NxCatchAllCall(__FUNCTION__, { EndDialog(IDCANCEL); return FALSE; });

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CApptPrototypePropertySetAvailabilityDlg::OnBnClickedOk()
{
	try {
		m_nAvailDays = 0
			| (IsDlgButtonChecked(IDC_DAYS_SUNDAY_CHECK) ? EApptPrototypeAllowDays::Sunday : 0) 
			| (IsDlgButtonChecked(IDC_DAYS_MONDAY_CHECK) ? EApptPrototypeAllowDays::Monday : 0) 
			| (IsDlgButtonChecked(IDC_DAYS_TUESDAY_CHECK) ? EApptPrototypeAllowDays::Tuesday : 0) 
			| (IsDlgButtonChecked(IDC_DAYS_WEDNESDAY_CHECK) ? EApptPrototypeAllowDays::Wednesday : 0) 
			| (IsDlgButtonChecked(IDC_DAYS_THURSDAY_CHECK) ? EApptPrototypeAllowDays::Thursday : 0) 
			| (IsDlgButtonChecked(IDC_DAYS_FRIDAY_CHECK) ? EApptPrototypeAllowDays::Friday : 0) 
			| (IsDlgButtonChecked(IDC_DAYS_SATURDAY_CHECK) ? EApptPrototypeAllowDays::Saturday : 0) 
			;
		m_nAvailTimes = 0
			| (IsDlgButtonChecked(IDC_TIMES_EARLYMORNING_CHECK) ? EApptPrototypeAllowTimes::EarlyMorning : 0) 
			| (IsDlgButtonChecked(IDC_TIMES_LATEMORNING_CHECK) ? EApptPrototypeAllowTimes::LateMorning : 0) 
			| (IsDlgButtonChecked(IDC_TIMES_EARLYAFTERNOON_CHECK) ? EApptPrototypeAllowTimes::EarlyAfternoon : 0) 
			| (IsDlgButtonChecked(IDC_TIMES_LATEAFTERNOON_CHECK) ? EApptPrototypeAllowTimes::LateAfternoon : 0) 
			;

		OnOK();

	} NxCatchAll(__FUNCTION__);
}
