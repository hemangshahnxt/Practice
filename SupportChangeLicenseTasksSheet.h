//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "SupportChangeLicenseWizardSheet.h"

// CSupportChangeLicenseTasksSheet dialog

class CSupportChangeLicenseTasksSheet : public CSupportChangeLicenseWizardSheet
{
	DECLARE_DYNAMIC(CSupportChangeLicenseTasksSheet)

public:
	//CSupportChangeLicenseTasksSheetDlg(CWnd* pParent);   // standard constructor
	CSupportChangeLicenseTasksSheet(CSupportChangeLicenseWizardMasterDlg* pParent); 
	virtual ~CSupportChangeLicenseTasksSheet();

	NxButton	m_btnAddLicenses;
	NxButton	m_btnSubLicenses;
	NxButton	m_btnSwapPDANexSync;
	NxButton	m_btnSwapPalmNexSync;

	virtual BOOL Validate();
	virtual bool IsActivity();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE_TASKS_SHEET };
	virtual void Load();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
