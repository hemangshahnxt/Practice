//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
// SupportChangeLicenseConfirmSheet.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SupportChangeLicenseConfirmSheet.h"


// CSupportChangeLicenseConfirmSheet dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseConfirmSheet, CNxDialog)

CSupportChangeLicenseConfirmSheet::CSupportChangeLicenseConfirmSheet(CSupportChangeLicenseWizardMasterDlg* pParent /*=NULL*/)
	: CSupportChangeLicenseWizardSheet(CSupportChangeLicenseConfirmSheet::IDD, pParent)
{

}

CSupportChangeLicenseConfirmSheet::~CSupportChangeLicenseConfirmSheet()
{
}

void CSupportChangeLicenseConfirmSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSupportChangeLicenseConfirmSheet, CNxDialog)
END_MESSAGE_MAP()


// CSupportChangeLicenseConfirmSheet message handlers
void CSupportChangeLicenseConfirmSheet::Load()
{
	
	try {
		CString strTemp;

		if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loAdd) {

			if (m_pdlgMaster->GetBChangeLicenses() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeLicenses());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_LICENSE_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangeConc() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeConc());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_CONCTS_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangeDoctors() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeDoctors());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_NUM_DOCTOR_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangeNexSync() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeNexSync());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_NEXSYNC_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangePalm() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangePalm());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_PALM_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangeNexPDA() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeNexPDA());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_NEXPDA_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangeDatabases() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeDatabases());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_DATABASES_CHANGE, strTemp);

			if (m_pdlgMaster->GetBChangeEMRProv() != 0) {
				strTemp.Format("%li added.", m_pdlgMaster->GetBChangeEMRProv());
			}
			else {
				strTemp.Format("-No change-"); 
			}
			SetDlgItemText(IDC_STATIC_EMRPROV_CHANGE, strTemp);
		}

		// Swap NexPDA and NexSync
		if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loSwapPDANexSync) {

			strTemp.Format("-No change-");
			SetDlgItemText(IDC_STATIC_LICENSE_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_NUM_DOCTOR_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_PALM_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_DATABASES_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_EMRPROV_CHANGE, strTemp);

			strTemp.Format("Swapped with NexSync.");
			SetDlgItemText(IDC_STATIC_NEXPDA_CHANGE, strTemp);
			strTemp.Format("Swapped with NexPDA.");
			SetDlgItemText(IDC_STATIC_NEXSYNC_CHANGE, strTemp);
		}

		// Swap PalmPilot and NexSync
		if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loSwapPPNexSync) {

			strTemp.Format("-No change-");
			SetDlgItemText(IDC_STATIC_LICENSE_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_NUM_DOCTOR_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_NEXPDA_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_DATABASES_CHANGE, strTemp);
			SetDlgItemText(IDC_STATIC_EMRPROV_CHANGE, strTemp);

			// No change
			strTemp.Format("Swapped with NexSync.");
			SetDlgItemText(IDC_STATIC_PALM_CHANGE, strTemp);
			strTemp.Format("Swapped with Palm.");
			SetDlgItemText(IDC_STATIC_NEXSYNC_CHANGE, strTemp);
		}

		// Returns
		if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loReturn) {

			if (m_pdlgMaster->GetBChangeLicenses() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeLicenses());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_LICENSE_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangeConc() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeConc());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_CONCTS_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangeDoctors() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeDoctors());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_NUM_DOCTOR_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangeNexSync() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeNexSync());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_NEXSYNC_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangePalm() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangePalm());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_PALM_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangeNexPDA() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeNexPDA());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_NEXPDA_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangeDatabases() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeDatabases());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_DATABASES_CHANGE, strTemp);

				if (m_pdlgMaster->GetBChangeEMRProv() != 0) {
					strTemp.Format("%li returned.", m_pdlgMaster->GetBChangeEMRProv());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_EMRPROV_CHANGE, strTemp);
			}
		// Support declined
		if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loDeclineSupport) {

				if (m_pdlgMaster->GetPChangeLicenses() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeLicenses());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_LICENSE_CHANGE, strTemp);
				
				if (m_pdlgMaster->GetPChangeConc() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeConc());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_CONCTS_CHANGE, strTemp);

				if (m_pdlgMaster->GetPChangeDoctors() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeDoctors());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_NUM_DOCTOR_CHANGE, strTemp);

				if (m_pdlgMaster->GetPChangeNexSync() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeNexSync());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_NEXSYNC_CHANGE, strTemp);

				if (m_pdlgMaster->GetPChangePalm() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangePalm());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_PALM_CHANGE, strTemp);

				if (m_pdlgMaster->GetPChangeNexPDA() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeNexPDA());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_NEXPDA_CHANGE, strTemp);

				if (m_pdlgMaster->GetPChangeDatabases() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeDatabases());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_DATABASES_CHANGE, strTemp);

				if (m_pdlgMaster->GetPChangeEMRProv() != 0) {
					strTemp.Format("%li declined.", m_pdlgMaster->GetPChangeEMRProv());
				}
				else {
					strTemp.Format("-No change-"); 
				}
				SetDlgItemText(IDC_STATIC_EMRPROV_CHANGE, strTemp);
			}

	} NxCatchAll(__FUNCTION__);
	
}

/*
BOOL CSupportChangeLicenseConfirmSheet::Validate()
{

	// Swap NexPDA and NexSync
	if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loSwapPDANexSync) {

		long nNexPDA, nNexSync;
		
		nNexPDA = m_pdlgMaster->GetNexPDA();
		nNexSync = m_pdlgMaster->GetNexSync();

		m_pdlgMaster->SetNewNexPDA(nNexSync);
		m_pdlgMaster->SetNewNexSync(nNexPDA);
	}

	// Swap PalmPilot and NexSync
	if (m_pdlgMaster->GetLicenseChangeType() == CSupportChangeLicenseWizardMasterDlg::loSwapPPNexSync) {

		CString strTemp;
		long nPalm, nNexSync;

		nPalm = m_pdlgMaster->GetPalm();
		nNexSync = m_pdlgMaster->GetNexSync();

		m_pdlgMaster->SetNewPalm(nNexSync);
		m_pdlgMaster->SetNewNexSync(nPalm);
	}


	return TRUE;
}
*/

bool CSupportChangeLicenseConfirmSheet::IsActivity()
{
	return false;
}