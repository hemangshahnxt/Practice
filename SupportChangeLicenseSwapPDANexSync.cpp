//(c.copits 2011-10-11) PLID 28219 - Use guided dialog to edit licensing options
// SupportChangeLicenseSwapPDANexSync.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportChangeLicenseSwapPDANexSync.h"


// CSupportChangeLicenseSwapPDANexSync dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseSwapPDANexSync, CNxDialog)

CSupportChangeLicenseSwapPDANexSync::CSupportChangeLicenseSwapPDANexSync(CSupportChangeLicenseWizardMasterDlg* pParent /*=NULL*/)
	: CSupportChangeLicenseWizardSheet(CSupportChangeLicenseSwapPDANexSync::IDD, pParent)
{

}

CSupportChangeLicenseSwapPDANexSync::~CSupportChangeLicenseSwapPDANexSync()
{
}

void CSupportChangeLicenseSwapPDANexSync::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSupportChangeLicenseSwapPDANexSync, CNxDialog)
END_MESSAGE_MAP()


// CSupportChangeLicenseSwapPDANexSync message handlers


BOOL CSupportChangeLicenseSwapPDANexSync::OnInitDialog() 
{
	try {

		CSupportChangeLicenseWizardSheet::OnInitDialog();

		CString strMessage;

		// Before swap
		strMessage.Format("PDA bought: %lu", m_pdlgMaster->GetBNexPDA());
		GetDlgItem(IDC_STATIC_PDA_BEFORE)->SetWindowText(strMessage);
		strMessage.Format("NexSync bought: %lu", m_pdlgMaster->GetBNexSync());
		GetDlgItem(IDC_STATIC_NEXSYNC_BEFORE)->SetWindowText(strMessage);
		// After swap
		strMessage.Format("PDA bought: %lu", m_pdlgMaster->GetBNexSync());
		GetDlgItem(IDC_STATIC_PDA_AFTER)->SetWindowText(strMessage);
		strMessage.Format("NexSync bought: %lu", m_pdlgMaster->GetBNexPDA());
		GetDlgItem(IDC_STATIC_NEXSYNC_AFTER)->SetWindowText(strMessage);

		ResetValues();


	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSupportChangeLicenseSwapPDANexSync::Load()
{
	try {
		if (m_pdlgMaster->GetLicenseChangeType() != CSupportChangeLicenseWizardMasterDlg::loSwapPDANexSync) {
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

BOOL CSupportChangeLicenseSwapPDANexSync::Validate()
{
	long nTempBoughtPDA = 0;

	// Swap bought
	nTempBoughtPDA = m_pdlgMaster->GetBNexPDA();
	m_pdlgMaster->SetBNewNexPDA(m_pdlgMaster->GetBNexSync());
	m_pdlgMaster->SetBNewNexSync(nTempBoughtPDA);

	// Assign bought changes
	m_pdlgMaster->SetBChangeNexPDA( m_pdlgMaster->GetBNewNexPDA() - m_pdlgMaster->GetBNexPDA());
	m_pdlgMaster->SetBChangeNexSync( m_pdlgMaster->GetBNewNexSync() - m_pdlgMaster->GetBNexSync());

	long nPendingDiff = 0;
	//long nExistingPending = 0;

	// Assign new pending for NexPDA
	/*
	nPendingDiff = m_pdlgMaster->GetIUNexSync() - m_pdlgMaster->GetIUNexPDA();
	m_pdlgMaster->SetPChangeNexPDA(nPendingDiff);
	nExistingPending = m_pdlgMaster->GetPNexPDA();
	nPendingDiff += nExistingPending;
	m_pdlgMaster->SetPNewNexPDA(nPendingDiff);
	m_pdlgMaster->SetBChangeNexPDA(
	*/
	nPendingDiff = m_pdlgMaster->GetBNewNexPDA() - m_pdlgMaster->GetBNexPDA();
	m_pdlgMaster->SetPNewNexPDA(nPendingDiff + m_pdlgMaster->GetPNexPDA());
	m_pdlgMaster->SetPChangeNexPDA(nPendingDiff);

	// Assign new pending for NexSync
	/*
	nPendingDiff = m_pdlgMaster->GetIUNexPDA() - m_pdlgMaster->GetIUNexSync();
	m_pdlgMaster->SetPChangeNexSync(nPendingDiff);
	nExistingPending = m_pdlgMaster->GetPNexSync();
	nPendingDiff += nExistingPending;
	m_pdlgMaster->SetPNewNexSync(nPendingDiff);
	m_pdlgMaster->SetBChangeNexSync(
	*/
	nPendingDiff = m_pdlgMaster->GetBNewNexSync() - m_pdlgMaster->GetBNexSync();
	m_pdlgMaster->SetPNewNexSync(nPendingDiff + m_pdlgMaster->GetPNexSync());
	m_pdlgMaster->SetPChangeNexSync(nPendingDiff);

	m_pdlgMaster->m_nOldLicensingChangeType = CSupportChangeLicenseWizardMasterDlg::loSwapPDANexSync;

	return TRUE;
}

void CSupportChangeLicenseSwapPDANexSync::ResetValues()
{
	// Placeholder
	// UI interface resets would go here
}

void CSupportChangeLicenseSwapPDANexSync::SetChangeFromAnotherActivity(bool bChange)
{
	m_bChangeFromAnotherActivity = bChange;
}

bool CSupportChangeLicenseSwapPDANexSync::IsActivity()
{
	return true;
}
