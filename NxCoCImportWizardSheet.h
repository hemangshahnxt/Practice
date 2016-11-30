//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer

#pragma once

#include "NxCoCWizardMasterDlg.h"

class CNxCoCWizardSheet : public CNxDialog
{
// Construction
public:
	CNxCoCWizardSheet(UINT nDialogID, CNxCoCWizardMasterDlg* pParent);   // standard constructor

	virtual UINT GetDialogID();
	virtual void Load() {}
	virtual BOOL Validate() { return TRUE; }

	//If this is true for a sheet, you will never be able to return once you move off it.
	BOOL m_bNoReturn;

// Implementation
protected:
	UINT m_nDialogID;
	CNxCoCWizardMasterDlg *m_pdlgMaster;

	// Generated message map functions
	//{{AFX_MSG(CNxCoCWizardSheet)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
