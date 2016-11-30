// ResolveConsSurgBookingDlg.cpp : implementation file
// (c.haag 2009-01-19 16:08) - PLID 32712 - Initial implementation
//

#include "stdafx.h"
#include "Practice.h"
#include "ResolveConsSurgBookingDlg.h"


// CResolveConsSurgBookingDlg dialog

IMPLEMENT_DYNAMIC(CResolveConsSurgBookingDlg, CNxDialog)

// (j.gruber 2009-07-09 10:20) - PLID 34303 - added category so we can change the wording appropriately
CResolveConsSurgBookingDlg::CResolveConsSurgBookingDlg(PhaseTracking::AptCategory aptCat, CWnd* pParent /*=NULL*/)
	: CNxDialog(CResolveConsSurgBookingDlg::IDD, pParent)
{
	m_aptCat = aptCat;
	m_Action = eDoNothing;
}

CResolveConsSurgBookingDlg::~CResolveConsSurgBookingDlg()
{
}

void CResolveConsSurgBookingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_APPT_INFO, m_nxstaticApptText);
	DDX_Control(pDX, IDC_RADIO_ADD, m_radAdd);
	DDX_Control(pDX, IDC_RADIO_REPLACE, m_radReplace);
	DDX_Control(pDX, IDC_RADIO_NOCHANGE, m_radNoChange);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CResolveConsSurgBookingDlg, CNxDialog)
	ON_BN_CLICKED(IDC_RADIO_ADD, OnBnClickedRadioAdd)
	ON_BN_CLICKED(IDC_RADIO_REPLACE, OnBnClickedRadioReplace)
	ON_BN_CLICKED(IDC_RADIO_NOCHANGE, OnBnClickedRadioNoChange)
END_MESSAGE_MAP()


// CResolveConsSurgBookingDlg message handlers

void CResolveConsSurgBookingDlg::OnBnClickedRadioAdd()
{
	try {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		m_Action = eAddProcedures;
	}
	NxCatchAll("Error in CResolveConsSurgBookingDlg::OnBnClickedRadioAdd");
}

void CResolveConsSurgBookingDlg::OnBnClickedRadioReplace()
{
	try {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		m_Action = eReplaceProcedures;
	}
	NxCatchAll("Error in CResolveConsSurgBookingDlg::OnBnClickedRadioReplace");
}

void CResolveConsSurgBookingDlg::OnBnClickedRadioNoChange()
{
	try {
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		m_Action = eDoNothing;
	}
	NxCatchAll("Error in CResolveConsSurgBookingDlg::OnBnClickedRadioNoChange");
}
BOOL CResolveConsSurgBookingDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_nxstaticApptText.SetWindowText(m_strApptText);

		// (j.gruber 2009-07-09 10:24) - PLID 34303 - change the text if this is a minor procedure instead of a surgery
		if (m_aptCat == PhaseTracking::AC_MINOR) {
			m_radAdd.SetWindowTextA("Add this appointment's procedures to the consult (detail procedures may be substituted by their master procedures)");
			m_radReplace.SetWindowTextA("Replace all the consult's procedures with those of this appointment (detail procedures may be substituted by their master procedures)");
		}
		
	}
	NxCatchAll("Error in CResolveConsSurgBookingDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
