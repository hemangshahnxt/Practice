//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
// SupportChangeLicenseDeclineSupportSheet.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportChangeLicenseDeclineSupportSheet.h"

// CSupportChangeLicenseDeclineSupportSheet dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseDeclineSupportSheet, CNxDialog)

CSupportChangeLicenseDeclineSupportSheet::CSupportChangeLicenseDeclineSupportSheet(CSupportChangeLicenseWizardMasterDlg* pParent /*=NULL*/)
	: CSupportChangeLicenseWizardSheet(CSupportChangeLicenseDeclineSupportSheet::IDD, pParent)
{

}

CSupportChangeLicenseDeclineSupportSheet::~CSupportChangeLicenseDeclineSupportSheet()
{
}

void CSupportChangeLicenseDeclineSupportSheet::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPIN_LICENSES, m_SpinLicenses);
	DDX_Control(pDX, IDC_SPIN_CONC, m_SpinConc);
	DDX_Control(pDX, IDC_SPIN_DOCTORS, m_SpinDoctors);
	DDX_Control(pDX, IDC_SPIN_NEXSYNC, m_SpinNexSync);
	DDX_Control(pDX, IDC_SPIN_PALM, m_SpinPalm);
	DDX_Control(pDX, IDC_SPIN_NEXPDA, m_SpinNexPDA);
	DDX_Control(pDX, IDC_SPIN_DATABASES, m_SpinDatabases);
	DDX_Control(pDX, IDC_SPIN_EMRPROV, m_SpinEMRProv);
}


BEGIN_MESSAGE_MAP(CSupportChangeLicenseDeclineSupportSheet, CNxDialog)
END_MESSAGE_MAP()


// CSupportChangeLicenseDeclineSupportSheet message handlers
void CSupportChangeLicenseDeclineSupportSheet::Load()
{
	try {
		if (m_pdlgMaster->GetLicenseChangeType() != CSupportChangeLicenseWizardMasterDlg::loDeclineSupport) {
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

BOOL CSupportChangeLicenseDeclineSupportSheet::OnInitDialog()
{
	try {

		CSupportChangeLicenseWizardSheet::OnInitDialog();

		m_SpinLicenses.SetBuddy(GetDlgItem(IDC_LICENSES));
		m_SpinConc.SetBuddy(GetDlgItem(IDC_CONC));
		m_SpinDoctors.SetBuddy(GetDlgItem(IDC_NUMBER_DOCTORS));
		m_SpinNexSync.SetBuddy(GetDlgItem(IDC_NEXSYNC));
		m_SpinPalm.SetBuddy(GetDlgItem(IDC_PALM));
		m_SpinNexPDA.SetBuddy(GetDlgItem(IDC_NEXPDA));
		m_SpinDatabases.SetBuddy(GetDlgItem(IDC_DATABASES));
		m_SpinEMRProv.SetBuddy(GetDlgItem(IDC_EMRPROV));


		// Set the maximum changeable pending to "# bought - current pending"
		/*
		m_SpinLicenses.SetRange(0, (short)m_pdlgMaster->GetBLicenses()+(short)m_pdlgMaster->GetPLicenses()); 
		m_SpinConc.SetRange(0, (short)m_pdlgMaster->GetBConc()+(short)m_pdlgMaster->GetPConc()); 
		m_SpinDoctors.SetRange(0, (short)m_pdlgMaster->GetBDoctors()+(short)m_pdlgMaster->GetPDoctors());
		m_SpinNexSync.SetRange(0, (short)m_pdlgMaster->GetBNexSync()+(short)m_pdlgMaster->GetPNexSync());
		m_SpinPalm.SetRange(0, (short)m_pdlgMaster->GetBPalm()+(short)m_pdlgMaster->GetPPalm());
		m_SpinNexPDA.SetRange(0, (short)m_pdlgMaster->GetBNexPDA()+(short)m_pdlgMaster->GetPNexPDA());
		m_SpinDatabases.SetRange(0, (short)m_pdlgMaster->GetBDatabases()+(short)m_pdlgMaster->GetPDatabases());
		m_SpinEMRProv.SetRange(0, (short)m_pdlgMaster->GetBEMRProv()+(short)m_pdlgMaster->GetPEMRProv());
		*/
		// Set the maximum changeable pending to the number of bought
		m_SpinLicenses.SetRange(0, (short)m_pdlgMaster->GetBLicenses()); 
		m_SpinConc.SetRange(0, (short)m_pdlgMaster->GetBConc()); 
		m_SpinDoctors.SetRange(0, (short)m_pdlgMaster->GetBDoctors());
		m_SpinNexSync.SetRange(0, (short)m_pdlgMaster->GetBNexSync());
		m_SpinPalm.SetRange(0, (short)m_pdlgMaster->GetBPalm());
		m_SpinNexPDA.SetRange(0, (short)m_pdlgMaster->GetBNexPDA());
		m_SpinDatabases.SetRange(0, (short)m_pdlgMaster->GetBDatabases());
		m_SpinEMRProv.SetRange(0, (short)m_pdlgMaster->GetBEMRProv());

		ResetValues();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSupportChangeLicenseDeclineSupportSheet::Validate()
{
	
	try {

		// Check for valid values
		//if ((m_SpinLicenses.GetPos() > (short)m_pdlgMaster->GetBLicenses()+(short)m_pdlgMaster->GetPLicenses()) || (m_SpinLicenses.GetPos() < 0)) {
		if ((m_SpinLicenses.GetPos() > (short)m_pdlgMaster->GetBLicenses()) || (m_SpinLicenses.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for licenses is between 0 and %li", (short)m_pdlgMaster->GetBLicenses()+(short)m_pdlgMaster->GetPLicenses());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinConc.GetPos() > (short)m_pdlgMaster->GetBConc()) || (m_SpinConc.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for Conc. TS is between 0 and %li", (short)m_pdlgMaster->GetBConc()+(short)m_pdlgMaster->GetPConc());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinDoctors.GetPos() > (short)m_pdlgMaster->GetBDoctors()) || (m_SpinDoctors.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for doctors is between 0 and %li", (short)m_pdlgMaster->GetBDoctors()+(short)m_pdlgMaster->GetPDoctors());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinNexSync.GetPos() > (short)m_pdlgMaster->GetBNexSync()) || (m_SpinNexSync.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for NexSync is between 0 and %li", (short)m_pdlgMaster->GetBNexSync()+(short)m_pdlgMaster->GetPNexSync());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinPalm.GetPos() > (short)m_pdlgMaster->GetBPalm()) || (m_SpinPalm.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for Palm Pilots is between 0 and %li", (short)m_pdlgMaster->GetBPalm()+(short)m_pdlgMaster->GetPPalm());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinNexPDA.GetPos() > (short)m_pdlgMaster->GetBNexPDA()) || (m_SpinNexPDA.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for NexPDA is between 0 and %li", (short)m_pdlgMaster->GetBNexPDA()+(short)m_pdlgMaster->GetPNexPDA());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinDatabases.GetPos() > (short)m_pdlgMaster->GetBDatabases()) || (m_SpinDatabases.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for databases is between 0 and %li", (short)m_pdlgMaster->GetBDatabases()+(short)m_pdlgMaster->GetPDatabases());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinEMRProv.GetPos() > (short)m_pdlgMaster->GetBEMRProv()) || (m_SpinEMRProv.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for EMR Providers is between 0 and %li", (short)m_pdlgMaster->GetBEMRProv()+(short)m_pdlgMaster->GetPEMRProv());
			MessageBox(strMsg);
			return FALSE;
		}

		// Decrease pending if they don't want support any more
		m_pdlgMaster->SetPNewLicenses(-1 * m_SpinLicenses.GetPos() + m_pdlgMaster->GetPLicenses());
		m_pdlgMaster->SetPChangeLicenses(-1 * m_SpinLicenses.GetPos());

		m_pdlgMaster->SetPNewConc(-1 * m_SpinConc.GetPos() + m_pdlgMaster->GetPConc());
		m_pdlgMaster->SetPChangeConc(-1 * m_SpinConc.GetPos());

		m_pdlgMaster->SetPNewDoctors(-1 * m_SpinDoctors.GetPos() + m_pdlgMaster->GetPDoctors());
		m_pdlgMaster->SetPChangeDoctors(-1 * m_SpinDoctors.GetPos());

		m_pdlgMaster->SetPNewNexSync(-1 * m_SpinNexSync.GetPos() + m_pdlgMaster->GetPNexSync());
		m_pdlgMaster->SetPChangeNexSync(-1 * m_SpinNexSync.GetPos());

		m_pdlgMaster->SetPNewPalm(-1 * m_SpinPalm.GetPos() + m_pdlgMaster->GetPPalm());
		m_pdlgMaster->SetPChangePalm(-1 * m_SpinPalm.GetPos());

		m_pdlgMaster->SetPNewNexPDA(-1 * m_SpinNexPDA.GetPos() + m_pdlgMaster->GetPNexPDA());
		m_pdlgMaster->SetPChangeNexPDA(-1 * m_SpinNexPDA.GetPos());

		m_pdlgMaster->SetPNewDatabases(-1 * m_SpinDatabases.GetPos() + m_pdlgMaster->GetPDatabases());
		m_pdlgMaster->SetPChangeDatabases(-1 * m_SpinDatabases.GetPos());

		m_pdlgMaster->SetPNewEMRProv(-1 * m_SpinEMRProv.GetPos() + m_pdlgMaster->GetPEMRProv());
		m_pdlgMaster->SetPChangeEMRProv(-1 * m_SpinEMRProv.GetPos());

		m_pdlgMaster->m_nOldLicensingChangeType = CSupportChangeLicenseWizardMasterDlg::loDeclineSupport;

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CSupportChangeLicenseDeclineSupportSheet::ResetValues()
{
	try {
		m_SpinLicenses.SetPos(0);
		m_SpinConc.SetPos(0);
		m_SpinDoctors.SetPos(0);	
		m_SpinNexSync.SetPos(0);
		m_SpinPalm.SetPos(0);
		m_SpinNexPDA.SetPos(0);
		m_SpinDatabases.SetPos(0);
		m_SpinEMRProv.SetPos(0);

	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseDeclineSupportSheet::SetChangeFromAnotherActivity(bool bChange)
{
	m_bChangeFromAnotherActivity = bChange;
}

bool CSupportChangeLicenseDeclineSupportSheet::IsActivity()
{
	return true;
}