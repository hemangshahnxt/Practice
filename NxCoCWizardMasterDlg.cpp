// NxCoCWizardMasterDlg.cpp : implementation file
//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
//

#include "stdafx.h"
#include "Practice.h"
#include "NxCoCWizardMasterDlg.h"
#include "NxCoCMainSheet.h"
#include "NxCoCPacketSheet.h"
#include "NxCoCTemplateSheet.h"


// CNxCoCWizardMasterDlg dialog

IMPLEMENT_DYNAMIC(CNxCoCWizardMasterDlg, CNxDialog)

CNxCoCWizardMasterDlg::CNxCoCWizardMasterDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNxCoCWizardMasterDlg::IDD, pParent)
{
	m_nActiveSheetIndex = 0;
	m_bEmrStandard = FALSE; // (z.manning 2009-04-09 11:14) - PLID 33934
}

void CNxCoCWizardMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NXCOC_IMPORT, m_btnImport);
}


BEGIN_MESSAGE_MAP(CNxCoCWizardMasterDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NXCOC_NEXT, &CNxCoCWizardMasterDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_NXCOC_BACK, &CNxCoCWizardMasterDlg::OnBnClickedBack)
	ON_BN_CLICKED(IDC_NXCOC_IMPORT, &CNxCoCWizardMasterDlg::OnBnClickedImport)
	ON_BN_CLICKED(IDCANCEL, &CNxCoCWizardMasterDlg::OnCancel)
END_MESSAGE_MAP()


// CNxCoCWizardMasterDlg message handlers

BOOL CNxCoCWizardMasterDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//NxIconButton setup
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnImport.AutoSet(NXB_OK);

		//Add all of the sheets
		CNxCoCMainSheet *pMainSheet = new CNxCoCMainSheet(this);
		m_arypWizardSheets.Add(pMainSheet);

		CNxCoCPacketSheet *pPacketSheet = new CNxCoCPacketSheet(this);
		m_arypWizardSheets.Add(pPacketSheet);
			
		CNxCoCTemplateSheet *pTemplateSheet = new CNxCoCTemplateSheet(this);
		m_arypWizardSheets.Add(pTemplateSheet);

		//Done adding sheets
		//

		//Now create all the sheets
		for(int i = 0; i < m_arypWizardSheets.GetSize(); i++)
		{
			CNxCoCWizardSheet *pdlgSheet = m_arypWizardSheets.GetAt(i);
			pdlgSheet->Create(pdlgSheet->GetDialogID(), this);
		}

		//Start with the main sheet
		SetActiveSheet(0);
	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CNxCoCWizardMasterDlg::OnBnClickedNext()
{
	CWaitCursor wc;
	try {
		//Validate the sheet we're currently on
		if(!m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->Validate()) {
			return;
		}

		//Move to the next active sheet.
		if(m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1) {
			SetActiveSheet(m_nActiveSheetIndex + 1);
		}
	}NxCatchAll("Error in OnBnClickedNext");
}

void CNxCoCWizardMasterDlg::OnBnClickedBack()
{
	CWaitCursor wc;
	try {
		//No validation needed, just jump back to the previous page
		if(m_nActiveSheetIndex >= 1) {
			SetActiveSheet(m_nActiveSheetIndex - 1);
		}

	}NxCatchAll("Error in OnBnClickedBack");
}

void CNxCoCWizardMasterDlg::OnBnClickedImport()
{
	try {
		//Validate the current sheet
		if(!m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->Validate()) {
			return;
		}

		//Confirm
		// (z.manning 2009-04-09 12:42) - PLID 33934 - Reword this text to handle EMR std importing
		if(AfxMessageBox("Are you sure you want to import the NexTech " + GetImportTypeText() + " content?", MB_YESNO|MB_ICONQUESTION) != IDYES) {
			return;
		}

		//Give them a wait dialog so they know something is going on
		CShowConnectingFeedbackDlg dlgWait;
		dlgWait.SetWindowTitle("Importing Content");
		dlgWait.SetWaitMessage("Please wait while content is being imported...");

		if(!m_ImportInfo.DoNxCoCImport(this, &dlgWait)) {
			//failure
			return;
		}

		//Cleanup our temp files!
		try {
			//Do this in a try/catch just to be safe.  The import has succeeded, so if for some reason this blows up 
			//	(permissions, someone already deleted them, whatever), we just happily go on our way like nothing happened.
			m_ImportInfo.ClearTempPath();
			
		} catch(...) {
			//Close down the wait dialog, we don't want it coming up over a message box.
			dlgWait.ShowWindow(SW_HIDE);
			AfxMessageBox("An error occurred while attempting to cleanup temporary files after the import was successful.  Your data "
				"was imported correctly, however you may have some extra files in the temporary folder on this machine.");
		}

		//Close down the wait dialog, we don't want it coming up over a message box.
		dlgWait.ShowWindow(SW_HIDE);

		//...and we're done
		AfxMessageBox("Import Successful!");
		EndDialog(IDOK);
	}NxCatchAll("Error importing NxCoC content");
}

void CNxCoCWizardMasterDlg::OnCancel()
{
	try {
		//Set the event to signal destruction.  This informs other areas that we are performing cleanup, 
		//	such as stopping the download if it is in progress.
		SetEvent(m_ImportInfo.m_hevDestroying);

	} NxCatchAll("Error in OnCancel");

	try {
		//Do this in a try/catch just to be safe.  The import has been dropped, so if for some reason this blows up 
		//	(permissions, someone already deleted them, whatever), we just happily go on our way like nothing happened.
		m_ImportInfo.ClearTempPath();
		
	} catch(...) {
		AfxMessageBox("An error occurred while attempting to cleanup temporary files.  You may have some extra files in the temporary folder on this machine.");
	}

	CDialog::OnCancel();
}

void CNxCoCWizardMasterDlg::SetActiveSheet(int nSheetIndex)
{
	//Ensure our given index is valid
	if(nSheetIndex < 0 || nSheetIndex >= m_arypWizardSheets.GetSize()) {
		AfxThrowNxException("Sheet index out of range: %li", nSheetIndex);
	}

	//Hide the current sheet
	m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->ShowWindow(SW_HIDE);

	//Position the new sheet correctly, then update which one is active
	CRect rcSheet;
	GetDlgItem(IDC_NXCOC_WIZARD_FRAME)->GetWindowRect(rcSheet);
	ScreenToClient(&rcSheet);
	m_arypWizardSheets.GetAt(nSheetIndex)->SetWindowPos(NULL, rcSheet.left, rcSheet.top, rcSheet.Width(), rcSheet.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	m_nActiveSheetIndex = nSheetIndex;

	//Let that sheet know it should load
	m_arypWizardSheets.GetAt(nSheetIndex)->Load();

	//Update back/next/import/etc buttons
	RefreshButtons();
}

CNxCoCWizardMasterDlg::~CNxCoCWizardMasterDlg() 
{
	try {
		//Clear the temp path, if we ever got to it.  This should never do anything because we intend to handle
		//	all cleanup gracefully in ok/cancel.  But try here *just in case* something else was missed.
		m_ImportInfo.ClearTempPath();
	} catch(...) { 
	}

	//Memory cleanup of the sheets
	for(int i = 0; i < m_arypWizardSheets.GetSize(); i++) {
		CNxCoCWizardSheet *pSheet = m_arypWizardSheets.GetAt(i);
		pSheet->DestroyWindow();
		delete pSheet;
	}
	m_arypWizardSheets.RemoveAll();
}

//Function to control the back/next/import button state.
void CNxCoCWizardMasterDlg::RefreshButtons()
{
	//On the last sheet, we get rid of 'Next' and show 'Import'
	if(m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1) {
		GetDlgItem(IDC_NXCOC_NEXT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NXCOC_NEXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_NXCOC_IMPORT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NXCOC_IMPORT)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_NXCOC_NEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NXCOC_NEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_NXCOC_IMPORT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NXCOC_IMPORT)->EnableWindow(TRUE);
	}

	//Don't allow them to go back if we're at the start
	if(m_nActiveSheetIndex <= 0) {
		GetDlgItem(IDC_NXCOC_BACK)->EnableWindow(FALSE);
	}
	else {
		//Also disallow back if the previous sheet has the 'No Return' flag set
		if(m_arypWizardSheets.GetAt(m_nActiveSheetIndex - 1)->m_bNoReturn) {
			GetDlgItem(IDC_NXCOC_BACK)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_NXCOC_BACK)->EnableWindow(TRUE);
		}
	}
}

// (z.manning 2009-04-09 11:21) - PLID 33934 - This function will return text specific to what
// type of import we're doing i.e. "Cycle of Care" or "EMR Standard"
CString CNxCoCWizardMasterDlg::GetImportTypeText()
{
	if(m_bEmrStandard) {
		return "EMR Standard";
	}
	else {
		return "Cycle of Care";
	}
}
