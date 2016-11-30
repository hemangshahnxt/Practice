//(c.copits 2011-10-13) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "SupportChangeLicenseWizardSheet.h"

// CSupportChangeLicenseSwapPalmNexSync dialog

class CSupportChangeLicenseSwapPalmNexSync : public CSupportChangeLicenseWizardSheet
{
	DECLARE_DYNAMIC(CSupportChangeLicenseSwapPalmNexSync)

public:
	//CSupportChangeLicenseSwapPalmNexSync(CWnd* pParent);   // standard constructor
	CSupportChangeLicenseSwapPalmNexSync(CSupportChangeLicenseWizardMasterDlg* pParent); 
	virtual ~CSupportChangeLicenseSwapPalmNexSync();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE_SWAP_PALM_NXSYNC };

	virtual void Load();
	virtual BOOL Validate();
	virtual void ResetValues();
	virtual void SetChangeFromAnotherActivity(bool bChange);
	virtual bool IsActivity();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	bool m_bChangeFromAnotherActivity;

	DECLARE_MESSAGE_MAP()
};
