//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options
#include "stdafx.h"
#include "SupportChangeLicenseWizardSheet.h"

CSupportChangeLicenseWizardSheet::CSupportChangeLicenseWizardSheet(UINT nDialogID, CSupportChangeLicenseWizardMasterDlg* pParent)
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

CSupportChangeLicenseWizardSheet::~CSupportChangeLicenseWizardSheet(void)
{
}

BOOL CSupportChangeLicenseWizardSheet::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CSupportChangeLicenseWizardSheet, CNxDialog)
	//{{AFX_MSG_MAP(CChangeLicenseWizardSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

UINT CSupportChangeLicenseWizardSheet::GetDialogID()
{
	return m_nDialogID;
}

void CSupportChangeLicenseWizardSheet::OnCancel()
{
	try	{

		// Intentionally do nothing here so the enter key doesn't close sheets.

	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseWizardSheet::OnOK()
{
	try {

		// Intentionally do nothing here so the enter key doesn't close sheets.

	} NxCatchAll(__FUNCTION__);
}