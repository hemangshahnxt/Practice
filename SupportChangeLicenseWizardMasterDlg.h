//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "PatientsRc.h"

#define	MAX_LICENSES	1000		// Should be enough

// CChangeLicenseWizardMasterDlg dialog

class CSupportChangeLicenseWizardSheet;

class CSupportChangeLicenseWizardMasterDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSupportChangeLicenseWizardMasterDlg)

public:
	CSupportChangeLicenseWizardMasterDlg(CWnd* pParent);   // standard constructor
	virtual ~CSupportChangeLicenseWizardMasterDlg();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE_WIZARD_MASTER_DLG };

	enum LicensingOptions {
		loNone,					// Nothing
		loAdd,					// Add-on support
		loDeclineSupport,		// Decline support
		loReturn,				// Return support
		loSwapPDANexSync,
		loSwapPPNexSync,
		loEnd
	};

	void SetLicenseChangeType(int nType);
	int GetLicenseChangeType();

	void SetBLicenses(long nLicenses);
	void SetBConc(long nConc);
	void SetBDoctors(long nDoctors);
	void SetBNexSync(long nNexSync);
	void SetBPalm(long nPalm);
	void SetBNexPDA(long nNexPDA);
	void SetBDatabases(long nDatabases);
	void SetBEMRProv(long nEMRProv);

	long GetBLicenses();
	long GetBConc();
	long GetBDoctors();
	long GetBNexSync();
	long GetBPalm();
	long GetBNexPDA();
	long GetBDatabases();
	long GetBEMRProv();

	long GetBNewLicenses();
	long GetBNewConc();
	long GetBNewDoctors();
	long GetBNewNexSync();
	long GetBNewPalm();
	long GetBNewNexPDA();
	long GetBNewDatabases();
	long GetBNewEMRProv();

	void SetBNewLicenses(long nNewLicenses);
	void SetBNewConc(long nNewConc);
	void SetBNewDoctors(long nNewDoctors);
	void SetBNewNexSync(long nNewNexSync);
	void SetBNewPalm(long nNewPalm);
	void SetBNewNexPDA(long nNewNexPDA);
	void SetBNewDatabases(long nNewDatabases);
	void SetBNewEMRProv(long nNewEMRProv);

	long GetBChangeLicenses();
	long GetBChangeConc();
	long GetBChangeDoctors();
	long GetBChangeNexSync();
	long GetBChangePalm();
	long GetBChangeNexPDA();
	long GetBChangeDatabases();
	long GetBChangeEMRProv();

	void SetBChangeLicenses(long nLicenses);
	void SetBChangeConc(long nConc);
	void SetBChangeDoctors(long nDoctors);
	void SetBChangeNexSync(long nNexSync);
	void SetBChangePalm(long nPalm);
	void SetBChangeNexPDA(long nNexPDA);
	void SetBChangeDatabases(long nDatabases);
	void SetBChangeEMRProv(long nEMRProv);

	void SetPLicenses(long nPending);
	void SetPConc(long nConc);
	void SetPDoctors(long nDoctors);
	void SetPNexSync(long nNexSync);
	void SetPPalm(long nPalm);
	void SetPNexPDA(long nNexPDA);
	void SetPDatabases(long nDatabases);
	void SetPEMRProv(long nEMRProv);

	long GetPLicenses();
	long GetPConc();
	long GetPDoctors();
	long GetPNexSync();
	long GetPPalm();
	long GetPNexPDA();
	long GetPDatabases();
	long GetPEMRProv();

	void SetPNewLicenses(long nLicenses);
	void SetPNewConc(long nConc);
	void SetPNewDoctors(long nDoctors);
	void SetPNewNexSync(long nNexSync);
	void SetPNewPalm(long nPalm);
	void SetPNewNexPDA(long nNexPDA);
	void SetPNewDatabases(long nDatabases);
	void SetPNewEMRProv(long nEMRProv);

	long GetPNewLicenses();
	long GetPNewConc();
	long GetPNewDoctors();
	long GetPNewNexSync();
	long GetPNewPalm();
	long GetPNewNexPDA();
	long GetPNewDatabases();
	long GetPNewEMRProv();

	void SetPChangeLicenses(long nLicenses);
	void SetPChangeConc(long nConc);
	void SetPChangeDoctors(long nDoctors);
	void SetPChangeNexSync(long nNexSync);
	void SetPChangePalm(long nPalm);
	void SetPChangeNexPDA(long nNexPDA);
	void SetPChangeDatabases(long nDatabases);
	void SetPChangeEMRProv(long nEMRProv);

	long GetPChangeLicenses();
	long GetPChangeConc();
	long GetPChangeDoctors();
	long GetPChangeNexSync();
	long GetPChangePalm();
	long GetPChangeNexPDA();
	long GetPChangeDatabases();
	long GetPChangeEMRProv();

	void SetIULicenses(long nLicenses);
	void SetIUConc(long nConc);
	void SetIUDoctors(long nDoctors);
	void SetIUNexSync(long nNexSync);
	void SetIUPalm(long nPalm);
	void SetIUNexPDA(long nNexPDA);
	void SetIUDatabases(long nDatabases);
	void SetIUEMRProv(long nEMRProv);

	long GetIUNewLicenses();
	long GetIUNewConc();
	long GetIUNewDoctors();
	long GetIUNewNexSync();
	long GetIUNewPalm();
	long GetIUNewNexPDA();
	long GetIUNewDatabases();
	long GetIUNewEMRProv();

	void SetIUNewLicenses(long nNewLicenses);
	void SetIUNewConc(long nConc);
	void SetIUNewDoctors(long nNewDoctors);
	void SetIUNewNexSync(long nNewNexSync);
	void SetIUNewPalm(long nNewPalm);
	void SetIUNewNexPDA(long nNewNexPDA);
	void SetIUNewDatabases(long nNewDatabases);
	void SetIUNewEMRProv(long nNewEMRProv);

	long GetIULicenses();
	long GetIUConc();
	long GetIUDoctors();
	long GetIUNexSync();
	long GetIUPalm();
	long GetIUNexPDA();
	long GetIUDatabases();
	long GetIUEMRProv();

	long GetIUChangeLicenses();
	long GetIUChangeConc();
	long GetIUChangeDoctors();
	long GetIUChangeNexSync();
	long GetIUChangePalm();
	long GetIUChangeNexPDA();
	long GetIUChangeDatabases();
	long GetIUChangeEMRProv();

	void SetIUChangeLicenses(long nLicenses);
	void SetIUChangeConc(long nConc);
	void SetIUChangeDoctors(long nDoctors);
	void SetIUChangeNexSync(long nNexSync);
	void SetIUChangePalm(long nPalm);
	void SetIUChangeNexPDA(long nNexPDA);
	void SetIUChangeDatabases(long nDatabases);
	void SetIUChangeEMRProv(long nEMRProv);

	BOOL AnyChanges();
	void ResetLicenseValues();
	int m_nOldLicensingChangeType;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	CArray<CSupportChangeLicenseWizardSheet*, CSupportChangeLicenseWizardSheet*> m_arypWizardSheets;
	long m_nActiveSheetIndex;

	void SetActiveSheet(int nSheetIndex);

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();

	void RefreshButtons();

	// Task
	int m_nLicensingChangeType;

	// Existing Bought
	long m_nBLicenses;
	long m_nBConc;
	long m_nBDoctors;
	long m_nBNexSync;
	long m_nBPalm;
	long m_nBNexPDA;
	long m_nBDatabases;
	long m_nBEMRProv;

	// New Bought
	long m_nBNewLicenses;
	long m_nBNewConc;
	long m_nBNewDoctors;
	long m_nBNewNexSync;
	long m_nBNewPalm;
	long m_nBNewNexPDA;
	long m_nBNewDatabases;
	long m_nBNewEMRProv;

	// Change Bought
	long m_nBChangeLicenses;
	long m_nBChangeConc;
	long m_nBChangeDoctors;
	long m_nBChangeNexSync;
	long m_nBChangePalm;
	long m_nBChangeNexPDA;
	long m_nBChangeDatabases;
	long m_nBChangeEMRProv;

	// Existing Pending
	long m_nPLicenses;
	long m_nPConc;
	long m_nPDoctors;
	long m_nPNexSync;
	long m_nPPalm;
	long m_nPNexPDA;
	long m_nPDatabases;
	long m_nPEMRProv;

	// Change Pending
	long m_nPChangeLicenses;
	long m_nPChangeConc;
	long m_nPChangeDoctors;
	long m_nPChangeNexSync;
	long m_nPChangePalm;
	long m_nPChangeNexPDA;
	long m_nPChangeDatabases;
	long m_nPChangeEMRProv;

	// New Pending
	long m_nPNewLicenses;
	long m_nPNewConc;
	long m_nPNewDoctors;
	long m_nPNewNexSync;
	long m_nPNewPalm;
	long m_nPNewNexPDA;
	long m_nPNewDatabases;
	long m_nPNewEMRProv;

	// Existing in-use
	long m_nIULicenses;
	long m_nIUConc;
	long m_nIUDoctors;
	long m_nIUNexSync;
	long m_nIUPalm;
	long m_nIUNexPDA;
	long m_nIUDatabases;
	long m_nIUEMRProv;
	
	// New in-use
	long m_nIUNewLicenses;
	long m_nIUNewConc;
	long m_nIUNewDoctors;
	long m_nIUNewNexSync;
	long m_nIUNewPalm;
	long m_nIUNewNexPDA;
	long m_nIUNewDatabases;
	long m_nIUNewEMRProv;

	// Change in-use
	long m_nIUChangeLicenses;
	long m_nIUChangeConc;
	long m_nIUChangeDoctors;
	long m_nIUChangeNexSync;
	long m_nIUChangePalm;
	long m_nIUChangeNexPDA;
	long m_nIUChangeDatabases;
	long m_nIUChangeEMRProv;

public:
	afx_msg void OnNext();
	afx_msg void OnBack();
	afx_msg void OnFinish();
};
