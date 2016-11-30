//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
// SupportChangeLicenseTasksSheet.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportChangeLicenseTasksSheet.h"

// CSupportChangeLicenseTasksSheet dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseTasksSheet, CNxDialog)

CSupportChangeLicenseTasksSheet::CSupportChangeLicenseTasksSheet(CSupportChangeLicenseWizardMasterDlg* pParent /*=NULL*/)
	: CSupportChangeLicenseWizardSheet(CSupportChangeLicenseTasksSheet::IDD, pParent)
{

}

CSupportChangeLicenseTasksSheet::~CSupportChangeLicenseTasksSheet()
{
}

void CSupportChangeLicenseTasksSheet::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_LICENSE, m_btnAddLicenses);
	DDX_Control(pDX, IDC_REMOVE_LICENSE, m_btnSubLicenses);
	DDX_Control(pDX, IDC_SWAP_NEXPDA_NEXSYNC, m_btnSwapPDANexSync);
	DDX_Control(pDX, IDC_SWAP_PALM_NEXSYNC, m_btnSwapPalmNexSync);

}


BEGIN_MESSAGE_MAP(CSupportChangeLicenseTasksSheet, CNxDialog)
END_MESSAGE_MAP()

void CSupportChangeLicenseTasksSheet::Load()
{
	/*
	m_pdlgMaster->SetNewLicenses(m_pdlgMaster->GetLicenses());
	m_pdlgMaster->SetNewDoctors(m_pdlgMaster->GetDoctors());
	m_pdlgMaster->SetNexSync(m_pdlgMaster->GetNexSync());
	m_pdlgMaster->SetNewPalm(m_pdlgMaster->GetPalm());
	m_pdlgMaster->SetNewNexPDA(m_pdlgMaster->GetNexPDA());
	m_pdlgMaster->SetNewDatabases(m_pdlgMaster->GetDatabases());
	m_pdlgMaster->SetNewEMRProv(m_pdlgMaster->GetEMRProv());
	*/
}

// CSupportChangeLicenseTasks message handlers
BOOL CSupportChangeLicenseTasksSheet::Validate()
{
	try {
		if(IsDlgButtonChecked(IDC_ADD_LICENSE)) {
			m_pdlgMaster->SetLicenseChangeType(CSupportChangeLicenseWizardMasterDlg::loAdd);
		}
		else if (IsDlgButtonChecked(IDC_RETURN_LICENSE)) {
			m_pdlgMaster->SetLicenseChangeType(CSupportChangeLicenseWizardMasterDlg::loReturn);
		}
		else if (IsDlgButtonChecked(IDC_REMOVE_LICENSE)) {
			m_pdlgMaster->SetLicenseChangeType(CSupportChangeLicenseWizardMasterDlg::loDeclineSupport);
		}
		else if (IsDlgButtonChecked(IDC_SWAP_NEXPDA_NEXSYNC)) {
			m_pdlgMaster->SetLicenseChangeType(CSupportChangeLicenseWizardMasterDlg::loSwapPDANexSync);
		}
		else if (IsDlgButtonChecked(IDC_SWAP_PALM_NEXSYNC)) {
			m_pdlgMaster->SetLicenseChangeType(CSupportChangeLicenseWizardMasterDlg::loSwapPPNexSync);
		}
		else {
			MessageBox("Please select a task.");
			return FALSE;
		}
	} NxCatchAll(__FUNCTION__);
	return TRUE;
}

bool CSupportChangeLicenseTasksSheet::IsActivity()
{
	return true;
}