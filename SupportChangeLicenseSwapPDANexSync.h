//(c.copits 2011-10-11) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "SupportChangeLicenseWizardSheet.h"

// CSupportChangeLicenseSwapPDANexSync dialog

class CSupportChangeLicenseSwapPDANexSync : public CSupportChangeLicenseWizardSheet
{
	DECLARE_DYNAMIC(CSupportChangeLicenseSwapPDANexSync)

public:
	//CSupportChangeLicenseSwapPDANexSync(CWnd* pParent);   // standard constructor
	CSupportChangeLicenseSwapPDANexSync(CSupportChangeLicenseWizardMasterDlg* pParent); 
	virtual ~CSupportChangeLicenseSwapPDANexSync();

// Dialog Data
	enum { IDD = IDD_SUPPORT_CHANGE_LICENSE_SWAP_PDA_NEXSYNC };

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
