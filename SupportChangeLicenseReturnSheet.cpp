//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
// SupportChangeLicenseReturnSheet.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportChangeLicenseReturnSheet.h"

// CSupportChangeLicenseReturnSheet dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseReturnSheet, CNxDialog)

CSupportChangeLicenseReturnSheet::CSupportChangeLicenseReturnSheet(CSupportChangeLicenseWizardMasterDlg* pParent /*=NULL*/)
	: CSupportChangeLicenseWizardSheet(CSupportChangeLicenseReturnSheet::IDD, pParent)
{

}

CSupportChangeLicenseReturnSheet::~CSupportChangeLicenseReturnSheet()
{
}

void CSupportChangeLicenseReturnSheet::DoDataExchange(CDataExchange* pDX)
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


BEGIN_MESSAGE_MAP(CSupportChangeLicenseReturnSheet, CNxDialog)
END_MESSAGE_MAP()

BOOL CSupportChangeLicenseReturnSheet::OnInitDialog() 
{
	try {

		CSupportChangeLicenseWizardSheet::OnInitDialog();

		m_SpinLicenses.SetBuddy(GetDlgItem(IDC_LICENSES));
		m_SpinLicenses.SetRange(0, (short)m_pdlgMaster->GetBLicenses()); 

		m_SpinConc.SetBuddy(GetDlgItem(IDC_CONC));
		m_SpinConc.SetRange(0, (short)m_pdlgMaster->GetBConc()); 

		m_SpinDoctors.SetBuddy(GetDlgItem(IDC_NUMBER_DOCTORS));
		m_SpinDoctors.SetRange(0, (short)m_pdlgMaster->GetBDoctors());

		m_SpinNexSync.SetBuddy(GetDlgItem(IDC_NEXSYNC));
		m_SpinNexSync.SetRange(0, (short)m_pdlgMaster->GetBNexSync());

		m_SpinPalm.SetBuddy(GetDlgItem(IDC_PALM));
		m_SpinPalm.SetRange(0, (short)m_pdlgMaster->GetBPalm());

		m_SpinNexPDA.SetBuddy(GetDlgItem(IDC_NEXPDA));
		m_SpinNexPDA.SetRange(0, (short)m_pdlgMaster->GetBNexPDA());
			
		m_SpinDatabases.SetBuddy(GetDlgItem(IDC_DATABASES));
		m_SpinDatabases.SetRange(0, (short)m_pdlgMaster->GetBDatabases());

		m_SpinEMRProv.SetBuddy(GetDlgItem(IDC_EMRPROV));
		m_SpinEMRProv.SetRange(0, (short)m_pdlgMaster->GetBEMRProv());

		ResetValues();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// CSupportChangeLicenseReturnSheet message handlers
void CSupportChangeLicenseReturnSheet::Load()
{
	try {
		if (m_pdlgMaster->GetLicenseChangeType() != CSupportChangeLicenseWizardMasterDlg::loReturn) {
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

BOOL CSupportChangeLicenseReturnSheet::Validate()
{
	try {
		// Check for valid values
		if ((m_SpinLicenses.GetPos() > (short)m_pdlgMaster->GetBLicenses()) || (m_SpinLicenses.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for licenses is between 0 and %li", (short)m_pdlgMaster->GetBLicenses());
			MessageBox(strMsg);
			return FALSE;
		}

		long n = m_SpinConc.GetPos();

		if ((m_SpinConc.GetPos() > (short)m_pdlgMaster->GetBConc()) || (m_SpinConc.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for Conc. TS is between 0 and %li", (short)m_pdlgMaster->GetBConc());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinDoctors.GetPos() > (short)m_pdlgMaster->GetBDoctors()) || (m_SpinDoctors.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for doctors is between 0 and %li", (short)m_pdlgMaster->GetBDoctors());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinNexSync.GetPos() > (short)m_pdlgMaster->GetBNexSync()) || (m_SpinNexSync.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for NexSync is between 0 and %li", (short)m_pdlgMaster->GetBNexSync());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinPalm.GetPos() > (short)m_pdlgMaster->GetBPalm()) || (m_SpinPalm.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for Palm Pilots is between 0 and %li", (short)m_pdlgMaster->GetBPalm());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinNexPDA.GetPos() > (short)m_pdlgMaster->GetBNexPDA()) || (m_SpinNexPDA.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for NexPDA is between 0 and %li", (short)m_pdlgMaster->GetBNexPDA());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinDatabases.GetPos() > (short)m_pdlgMaster->GetBDatabases()) || (m_SpinDatabases.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for databases is between 0 and %li", (short)m_pdlgMaster->GetBDatabases());
			MessageBox(strMsg);
			return FALSE;
		}

		if ((m_SpinEMRProv.GetPos() > (short)m_pdlgMaster->GetBEMRProv()) || (m_SpinEMRProv.GetPos() < 0)) {
			CString strMsg;
			strMsg.Format("Please ensure that the value for EMR Providers is between 0 and %li", (short)m_pdlgMaster->GetBEMRProv());
			MessageBox(strMsg);
			return FALSE;
		}

		// Update bought licenses
		// New Bought = "existing bought - # removed"
		// New Pending = "exising pending - # removed"

		m_pdlgMaster->SetBNewLicenses(m_pdlgMaster->GetBLicenses() - m_SpinLicenses.GetPos());
		m_pdlgMaster->SetPNewLicenses(m_pdlgMaster->GetPLicenses() - m_SpinLicenses.GetPos());
		m_pdlgMaster->SetBChangeLicenses(m_SpinLicenses.GetPos());

		m_pdlgMaster->SetBNewConc(m_pdlgMaster->GetBConc() - m_SpinConc.GetPos());
		m_pdlgMaster->SetPNewConc(m_pdlgMaster->GetPConc() - m_SpinConc.GetPos());
		m_pdlgMaster->SetBChangeConc(m_SpinConc.GetPos());

		m_pdlgMaster->SetBNewDoctors(m_pdlgMaster->GetBDoctors() - m_SpinDoctors.GetPos());
		m_pdlgMaster->SetPNewDoctors(m_pdlgMaster->GetPDoctors() - m_SpinDoctors.GetPos());
		m_pdlgMaster->SetBChangeDoctors(m_SpinDoctors.GetPos());

		m_pdlgMaster->SetBNewNexSync(m_pdlgMaster->GetBNexSync() - m_SpinNexSync.GetPos());
		m_pdlgMaster->SetPNewNexSync(m_pdlgMaster->GetPNexSync() - m_SpinNexSync.GetPos());
		m_pdlgMaster->SetBChangeNexSync(m_SpinNexSync.GetPos());

		m_pdlgMaster->SetBNewPalm(m_pdlgMaster->GetBPalm() - m_SpinPalm.GetPos());
		m_pdlgMaster->SetPNewPalm(m_pdlgMaster->GetPPalm() - m_SpinPalm.GetPos());
		m_pdlgMaster->SetBChangePalm(m_SpinPalm.GetPos());

		m_pdlgMaster->SetBNewNexPDA(m_pdlgMaster->GetBNexPDA() - m_SpinNexPDA.GetPos());
		m_pdlgMaster->SetPNewNexPDA(m_pdlgMaster->GetPNexPDA() - m_SpinNexPDA.GetPos());
		m_pdlgMaster->SetBChangeNexPDA(m_SpinNexPDA.GetPos());

		m_pdlgMaster->SetBNewDatabases(m_pdlgMaster->GetBDatabases() - m_SpinDatabases.GetPos());
		m_pdlgMaster->SetPNewDatabases(m_pdlgMaster->GetPDatabases() - m_SpinDatabases.GetPos());
		m_pdlgMaster->SetBChangeDatabases(m_SpinDatabases.GetPos());

		m_pdlgMaster->SetBNewEMRProv(m_pdlgMaster->GetBEMRProv() - m_SpinEMRProv.GetPos());
		m_pdlgMaster->SetPNewEMRProv(m_pdlgMaster->GetPEMRProv() - m_SpinEMRProv.GetPos());
		m_pdlgMaster->SetBChangeEMRProv(m_SpinEMRProv.GetPos());

		m_pdlgMaster->m_nOldLicensingChangeType = CSupportChangeLicenseWizardMasterDlg::loReturn;

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CSupportChangeLicenseReturnSheet::ResetValues()
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

void CSupportChangeLicenseReturnSheet::SetChangeFromAnotherActivity(bool bChange)
{
	m_bChangeFromAnotherActivity = bChange;
}

bool CSupportChangeLicenseReturnSheet::IsActivity()
{
	return true;
}