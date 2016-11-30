#if !defined(AFX_NEXFORMSIMPORTWIZARDSHEET_H__69176802_1E3A_42B0_B9CE_F720C40E34C5__INCLUDED_)
#define AFX_NEXFORMSIMPORTWIZARDSHEET_H__69176802_1E3A_42B0_B9CE_F720C40E34C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportWizardSheet.h : header file
//

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWizardSheet dialog

#include "NexFormsImportWizardMasterDlg.h"

class CNexFormsImportWizardSheet : public CNxDialog
{
// Construction
public:
	CNexFormsImportWizardSheet(UINT nDialogID, CNexFormsImportWizardMasterDlg* pParent);   // standard constructor

	virtual UINT GetDialogID();
	virtual void Load() {}
	virtual BOOL Validate() { return TRUE; }

	// (z.manning, 07/13/2007) - If this is true, this sheet will be skipped over when switching between sheets.
	BOOL m_bSkipSheet;

	// (z.manning, 07/13/2007) - If this is true, then once you leave this sheet, you can never go back.
	BOOL m_bNoReturn;

// Implementation
protected:
	UINT m_nDialogID;

	CNexFormsImportWizardMasterDlg *m_pdlgMaster;

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportWizardSheet)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTWIZARDSHEET_H__69176802_1E3A_42B0_B9CE_F720C40E34C5__INCLUDED_)
