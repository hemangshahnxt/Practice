//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
#pragma once

#include "SupportChangeLicenseWizardMasterDlg.h"

class CSupportChangeLicenseWizardSheet : public CNxDialog
{
// Construction
public:

	CSupportChangeLicenseWizardSheet(UINT nDialogID, CSupportChangeLicenseWizardMasterDlg* pParent);   // standard constructor
	~CSupportChangeLicenseWizardSheet(void);

	virtual UINT GetDialogID();
	virtual void Load() {}
	virtual BOOL Validate() { return TRUE; }
	virtual void SetChangeFromAnotherActivity(bool bChange) {}
	virtual bool IsActivity() { return true; }

	BOOL m_bSkipSheet;
	BOOL m_bNoReturn;

// Implementation
protected:

	UINT m_nDialogID;

	CSupportChangeLicenseWizardMasterDlg *m_pdlgMaster;

	virtual BOOL OnInitDialog();

	virtual void OnCancel();
	virtual void OnOK();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};
