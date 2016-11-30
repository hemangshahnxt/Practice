//(c.copits 2011-10-13) PLID 28219 - Use guided dialog to edit licensing options
// SupportChangeLicenseSwapPalmNexSync.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportChangeLicenseSwapPalmNexSync.h"


// CSupportChangeLicenseSwapPalmNexSync dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseSwapPalmNexSync, CNxDialog)

CSupportChangeLicenseSwapPalmNexSync::CSupportChangeLicenseSwapPalmNexSync(CSupportChangeLicenseWizardMasterDlg* pParent /*=NULL*/)
	: CSupportChangeLicenseWizardSheet(CSupportChangeLicenseSwapPalmNexSync::IDD, pParent)
{

}

CSupportChangeLicenseSwapPalmNexSync::~CSupportChangeLicenseSwapPalmNexSync()
{
}

void CSupportChangeLicenseSwapPalmNexSync::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSupportChangeLicenseSwapPalmNexSync, CNxDialog)
END_MESSAGE_MAP()


// CSupportChangeLicenseSwapPalmNexSync message handlers


BOOL CSupportChangeLicenseSwapPalmNexSync::OnInitDialog() 
{
	try {

		CSupportChangeLicenseWizardSheet::OnInitDialog();

		CString strMessage;

		// Before swap
		strMessage.Format("Palm bought: %lu", m_pdlgMaster->GetBPalm());
		GetDlgItem(IDC_STATIC_PALM_BEFORE)->SetWindowText(strMessage);
		strMessage.Format("NexSync bought: %lu", m_pdlgMaster->GetBNexSync());
		GetDlgItem(IDC_STATIC_NEXSYNC_BEFORE2)->SetWindowText(strMessage);
		// After swap
		strMessage.Format("Palm bought: %lu", m_pdlgMaster->GetBNexSync());
		GetDlgItem(IDC_STATIC_PALM_AFTER)->SetWindowText(strMessage);
		strMessage.Format("NexSync bought: %lu", m_pdlgMaster->GetBPalm());
		GetDlgItem(IDC_STATIC_NEXSYNC_AFTER2)->SetWindowText(strMessage);

		ResetValues();


	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSupportChangeLicenseSwapPalmNexSync::Load()
{
	try {
		if (m_pdlgMaster->GetLicenseChangeType() != CSupportChangeLicenseWizardMasterDlg::loSwapPPNexSync) {
			m_bSkipSheet = TRUE;
		}
		else {
			m_bSkipSheet = FALSE;
		}
		if (m_bChangeFromAnotherActivity) {
			ResetValues();
			m_pdlgMaster->ResetLicenseValues();
		}

	} NxCatchAll(__FUNCTION__);

}

BOOL CSupportChangeLicenseSwapPalmNexSync::Validate()
{
	long nTempBoughtPalm = 0;

	// Swap bought
	nTempBoughtPalm = m_pdlgMaster->GetBPalm();
	m_pdlgMaster->SetBNewPalm(m_pdlgMaster->GetBNexSync());
	m_pdlgMaster->SetBNewNexSync(nTempBoughtPalm);

	// Assign bought changes
	m_pdlgMaster->SetBChangePalm( m_pdlgMaster->GetBNewPalm() - m_pdlgMaster->GetBPalm());
	m_pdlgMaster->SetBChangeNexSync( m_pdlgMaster->GetBNewNexSync() - m_pdlgMaster->GetBNexSync());


	long nPendingDiff = 0;

	// Assign new pending for Palm
	nPendingDiff = m_pdlgMaster->GetBNewPalm() - m_pdlgMaster->GetBPalm();
	m_pdlgMaster->SetPNewPalm(nPendingDiff + m_pdlgMaster->GetPPalm());
	m_pdlgMaster->SetPChangePalm(nPendingDiff);

	// Assign new pending for NexSync
	nPendingDiff = m_pdlgMaster->GetBNewNexSync() - m_pdlgMaster->GetBNexSync();
	m_pdlgMaster->SetPNewNexSync(nPendingDiff + m_pdlgMaster->GetPNexSync());
	m_pdlgMaster->SetPChangeNexSync(nPendingDiff);

	m_pdlgMaster->m_nOldLicensingChangeType = CSupportChangeLicenseWizardMasterDlg::loSwapPPNexSync;

	return TRUE;
}

void CSupportChangeLicenseSwapPalmNexSync::ResetValues()
{
	// Placeholder
}

void CSupportChangeLicenseSwapPalmNexSync::SetChangeFromAnotherActivity(bool bChange)
{
	m_bChangeFromAnotherActivity = bChange;
}

bool CSupportChangeLicenseSwapPalmNexSync::IsActivity()
{
	return true;
}