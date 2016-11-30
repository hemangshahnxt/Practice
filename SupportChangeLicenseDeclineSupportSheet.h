//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "SupportChangeLicenseWizardSheet.h"

// CSupportChangeLicenseDeclineSupportSheet dialog

class CSupportChangeLicenseDeclineSupportSheet : public CSupportChangeLicenseWizardSheet
{
	DECLARE_DYNAMIC(CSupportChangeLicenseDeclineSupportSheet)

public:
	//CSupportChangeLicenseDeclineSupportSheet(CWnd* pParent);   // standard constructor
	CSupportChangeLicenseDeclineSupportSheet(CSupportChangeLicenseWizardMasterDlg* pParent); 
	virtual ~CSupportChangeLicenseDeclineSupportSheet();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE_DECLINE_SUPPORT_SHEET };

	virtual void Load();
	virtual BOOL Validate();
	virtual void ResetValues();
	virtual void SetChangeFromAnotherActivity(bool bChange);
	virtual bool IsActivity();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CSpinButtonCtrl m_SpinLicenses;
	CSpinButtonCtrl m_SpinConc;
	CSpinButtonCtrl m_SpinDoctors;
	CSpinButtonCtrl m_SpinNexSync;
	CSpinButtonCtrl m_SpinPalm;
	CSpinButtonCtrl m_SpinNexPDA;
	CSpinButtonCtrl m_SpinDatabases;
	CSpinButtonCtrl m_SpinEMRProv;

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	bool m_bChangeFromAnotherActivity;
};
