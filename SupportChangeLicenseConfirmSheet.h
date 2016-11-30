//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "SupportChangeLicenseWizardSheet.h"

// CSupportChangeLicenseConfirmSheet dialog

class CSupportChangeLicenseConfirmSheet : public CSupportChangeLicenseWizardSheet
{
	DECLARE_DYNAMIC(CSupportChangeLicenseConfirmSheet)

public:
	//CSupportChangeLicenseConfirmSheet(CWnd* pParent);   // standard constructor
	CSupportChangeLicenseConfirmSheet(CSupportChangeLicenseWizardMasterDlg* pParent); 
	virtual ~CSupportChangeLicenseConfirmSheet();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE_CONFIRM_SHEET };

	virtual void Load();
	virtual bool IsActivity();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
