//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer

#include "stdafx.h"
#include "NxCoCImportWizardSheet.h"
#include "NxCoCWizardMasterDlg.h"

CNxCoCWizardSheet::CNxCoCWizardSheet(UINT nDialogID, CNxCoCWizardMasterDlg* pParent)
	: CNxDialog(nDialogID, pParent)
{
	//{{AFX_DATA_INIT(CNxCoCWizardSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nDialogID = nDialogID;
	m_pdlgMaster = pParent;
	m_bNoReturn = FALSE;
}

BEGIN_MESSAGE_MAP(CNxCoCWizardSheet, CNxDialog)
	//{{AFX_MSG_MAP(CNxCoCWizardSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxCoCWizardSheet message handlers

BOOL CNxCoCWizardSheet::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

	}NxCatchAll("Error in OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

UINT CNxCoCWizardSheet::GetDialogID()
{
	return m_nDialogID;
}

void CNxCoCWizardSheet::OnCancel()
{
	try {
		//Intentionally do nothing here so esc key doesn't close sheets.

	}NxCatchAll("Error in OnCancel");
}

void CNxCoCWizardSheet::OnOK()
{
	try {
		//Intentionally do nothing here so the enter key doesn't close sheets.

	}NxCatchAll("Error in OnOK");
}