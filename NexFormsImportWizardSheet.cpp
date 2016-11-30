// NexFormsImportWizardSheet.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexFormsImportWizardSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


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


CNexFormsImportWizardSheet::CNexFormsImportWizardSheet(UINT nDialogID, CNexFormsImportWizardMasterDlg* pParent)
	: CNxDialog(nDialogID, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportWizardSheet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nDialogID = nDialogID;
	m_pdlgMaster = pParent;
	m_bSkipSheet = FALSE;
	m_bNoReturn = FALSE;
}

BEGIN_MESSAGE_MAP(CNexFormsImportWizardSheet, CNxDialog)
	//{{AFX_MSG_MAP(CNexFormsImportWizardSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWizardSheet message handlers

BOOL CNexFormsImportWizardSheet::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

	}NxCatchAll("CNexFormsImportWizardSheet::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

UINT CNexFormsImportWizardSheet::GetDialogID()
{
	return m_nDialogID;
}

void CNexFormsImportWizardSheet::OnCancel()
{
	try
	{
		// (z.manning, 06/22/2007) - Intentionally do nothing here so esc key doesn't close sheets.

	}NxCatchAll("CNexFormsImportWizardSheet::OnCancel");
}

void CNexFormsImportWizardSheet::OnOK()
{
	try
	{
		// (z.manning, 10/11/2007) - Intentionally do nothing here so the enter key doesn't close sheets.

	}NxCatchAll("CNexFormsImportWizardSheet::OnOK");
}