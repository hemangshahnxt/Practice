// NexFormsImportWizardMasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexFormsImportWizardMasterDlg.h"
#include "NexFormsImportNewProcedureSheet.h"
#include "NexFormsImportLadderSheet.h"
#include "NexFormsImportProcedureColumnsSheet.h"
#include "NexFormsImportNewFileSheet.h"
#include "NexFormsImportWordTemplatesSheet.h"
#include "NexFormsImportProcedureCompareSheet.h"
#include "ShowConnectingFeedbackDlg.h"
#include "AuditTrail.h"

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
// CNexFormsImportWizardMasterDlg dialog


CNexFormsImportWizardMasterDlg::CNexFormsImportWizardMasterDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexFormsImportWizardMasterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexFormsImportWizardMasterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nActiveSheetIndex = 0;
}


void CNexFormsImportWizardMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexFormsImportWizardMasterDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEXT, m_btnNext);
	DDX_Control(pDX, IDC_IMPORT_NEXFORMS, m_btnImportNexForms);
	DDX_Control(pDX, IDC_BACK, m_btnBack);
	DDX_Control(pDX, IDC_SHEET_PLACEHOLDER, m_btnSheetPlaceholder);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexFormsImportWizardMasterDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexFormsImportWizardMasterDlg)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_BACK, OnBack)
	ON_BN_CLICKED(IDC_IMPORT_NEXFORMS, OnImportNexforms)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWizardMasterDlg message handlers

BOOL CNexFormsImportWizardMasterDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnCancel.AutoSet(NXB_CANCEL);
		//DRT 5/16/2008 - PLID 29469 - We decided to remove the WIZ styles

		ASSERT(g_pLicense->CheckForLicense(CLicense::lcNexForms, CLicense::cflrSilent));

		// (z.manning, 10/10/2007) - PLID 27706 - Load the names of the 6 procedure cheat sheet field names 
		// and store them in a map so other areas of the importer can access them easily later.
		ADODB::_RecordsetPtr prsColumnNames = CreateParamRecordset(
			"SELECT Name FROM CustomFieldsT WHERE ID >= {INT} AND ID <= {INT} ORDER BY ID", 63, 68);
		for(int nColumnIndex = 1; !prsColumnNames->eof; prsColumnNames->MoveNext()) {
			m_mapCustomFieldNames.SetAt(FormatString("Custom%i",nColumnIndex++), AdoFldString(prsColumnNames,"Name"));
		}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin adding sheets to the NexForms import wizard

		CNexFormsImportNewFileSheet *pdlgFileSheet = new CNexFormsImportNewFileSheet(this);
		m_arypWizardSheets.Add(pdlgFileSheet);

		CNexFormsImportLadderSheet *pdlgLadderSheet = new CNexFormsImportLadderSheet(this);
		m_arypWizardSheets.Add(pdlgLadderSheet);
		
		CNexFormsImportNewProcedureSheet *pdlgNewProcSheet = new CNexFormsImportNewProcedureSheet(this);
		m_arypWizardSheets.Add(pdlgNewProcSheet);

		CNexFormsImportProcedureCompareSheet *pdlgCompareSheet = new CNexFormsImportProcedureCompareSheet(this);
		m_arypWizardSheets.Add(pdlgCompareSheet);

		CNexFormsImportProcedureColumnsSheet *pdlgColumnsSheet = new CNexFormsImportProcedureColumnsSheet(this);
		m_arypWizardSheets.Add(pdlgColumnsSheet);

		CNexFormsImportWordTemplatesSheet *pdlgWordTemplateSheet = new CNexFormsImportWordTemplatesSheet(this);
		m_arypWizardSheets.Add(pdlgWordTemplateSheet);

// End adding sheets to the NexForms import wizard
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		for(int i = 0; i < m_arypWizardSheets.GetSize(); i++)
		{
			CNexFormsImportWizardSheet *pdlgSheet = m_arypWizardSheets.GetAt(i);
			pdlgSheet->Create(pdlgSheet->GetDialogID(), this);
		}

		SetActiveSheet(0);

	}NxCatchAll("CNexFormsImportWizardMasterDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexFormsImportWizardMasterDlg::OnNext() 
{
	CWaitCursor wc;
	try
	{
		// (z.manning, 06/26/2007) - Make sure our current sheet is valid before going to the next one.
		if(!m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->Validate()) {
			// (z.manning, 06/26/2007) - And warning messages should be a part of that sheet's validate function;
			return;
		}

		if(m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1)
		{
			SetActiveSheet(m_nActiveSheetIndex + 1);

			// (z.manning, 07/10/2007) - Ok, we've loaded the new sheet. However, maybe we wanna skip it.
			// If so, let's go to the next one.
			while(m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->m_bSkipSheet && m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1) {
				SetActiveSheet(m_nActiveSheetIndex + 1);
			}
		}
		else
		{
			ASSERT(FALSE);
		}

	}NxCatchAll("CNexFormsImportWizardMasterDlg::OnNext");
}

void CNexFormsImportWizardMasterDlg::OnBack() 
{
	CWaitCursor wc;
	try
	{
		if(m_nActiveSheetIndex >= 1)
		{
			SetActiveSheet(m_nActiveSheetIndex - 1);

			// (z.manning, 07/10/2007) - Ok, we've loaded the new sheet. However, maybe we wanna skip it.
			// If so, let's go to the previous one.
			while(m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->m_bSkipSheet && m_nActiveSheetIndex >= 1) {
				SetActiveSheet(m_nActiveSheetIndex - 1);
			}
		}
		else
		{
			ASSERT(FALSE);
		}

	}NxCatchAll("CNexFormsImportWizardMasterDlg::OnBack");
	
}

void CNexFormsImportWizardMasterDlg::SetActiveSheet(int nSheetIndex)
{
	if(nSheetIndex < 0 || nSheetIndex >= m_arypWizardSheets.GetSize()) {
		AfxThrowNxException("Sheet index out of range: %li", nSheetIndex);
	}

	m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->ShowWindow(SW_HIDE);

	CRect rcSheet;
	GetDlgItem(IDC_SHEET_PLACEHOLDER)->GetClientRect(rcSheet);
	m_arypWizardSheets.GetAt(nSheetIndex)->SetWindowPos(NULL, rcSheet.left, rcSheet.top, rcSheet.Width(), rcSheet.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
	m_nActiveSheetIndex = nSheetIndex;
	m_arypWizardSheets.GetAt(nSheetIndex)->Load();

	RefreshButtons();
}

CNexFormsImportWizardMasterDlg::~CNexFormsImportWizardMasterDlg() 
{
	try
	{
		// (z.manning, 07/05/2007) - Delete any temp word template files we may have created.
		for(int i = 0; i < m_ImportInfo.m_arypWordTemplates.GetSize(); i++)
		{
			if(!m_ImportInfo.m_arypWordTemplates.GetAt(i)->strTempFileFullPath.IsEmpty())
			{
				DeleteFileWhenPossible(m_ImportInfo.m_arypWordTemplates.GetAt(i)->strTempFileFullPath);
			}
		}

		for(i = 0; i < m_arypWizardSheets.GetSize(); i++)
		{
			if(m_arypWizardSheets.GetAt(i) != NULL)
			{
				m_arypWizardSheets.GetAt(i)->DestroyWindow();
				delete m_arypWizardSheets.GetAt(i);
			}
		}
		m_arypWizardSheets.RemoveAll();

	}NxCatchAll("~CNexFormsImportWizardMasterDlg");
}

void CNexFormsImportWizardMasterDlg::RefreshButtons()
{
	// (z.manning, 06/28/2007) - If we're on the last sheet, hide the next button and show the import button.
	if(m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1)
	{
		GetDlgItem(IDC_NEXT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IMPORT_NEXFORMS)->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem(IDC_NEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IMPORT_NEXFORMS)->ShowWindow(SW_SHOW);
	}

	if(m_nActiveSheetIndex <= 0)
	{
		GetDlgItem(IDC_BACK)->EnableWindow(FALSE);
	}
	else
	{
		// (z.manning, 07/13/2007) - If the previous sheet doesn't want us to return, disable the back button.
		if(m_arypWizardSheets.GetAt(m_nActiveSheetIndex - 1)->m_bNoReturn) {
			GetDlgItem(IDC_BACK)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_BACK)->EnableWindow(TRUE);
		}
	}
}

void CNexFormsImportWizardMasterDlg::OnImportNexforms() 
{
	CWaitCursor wc;
	try
	{
		// (z.manning, 06/26/2007) - Make sure our current sheet is valid before importing
		if(!m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->Validate()) {
			return;
		}

		// (z.manning, 10/16/2007) - Confirm that they definitely want to run the import.
		if(IDYES != MessageBox("Are you sure you want to import NexForms content?", "Import NexForms?", MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		{
			// (z.manning, 07/19/2007) - PLID 26729 - Almost all of the time it takes to import
			// NexForms is the one giant query that we run for the procedure/ladder import. Because
			// of that, showing a progress bar doesn't make much sense since we don't know how
			// long the query is going to take. Instead, let's just show a please wait screen.
			CShowConnectingFeedbackDlg dlgWait;
			dlgWait.SetWindowTitle("Importing NexForms Content");
			dlgWait.SetWaitMessage("Please wait while NexForms content is being imported...");

			// (z.manning, 07/19/2007) - Let's do the packet and template import first because it's far
			// more likely to fail since it deals with creating/copying files and what not since we want
			// to try to avoid partial imports if possible.
			// (z.manning, 07/19/2007) - PLID 26746 - Added paramaters to template import that expedite
			// showing message boxes and hiding/showing the feedback dialog.
			if(!m_ImportInfo.ImportPacketsAndTemplates(this, &dlgWait)) {
				return;
			}
			// (c.haag 2009-02-03 15:09) - PLID 32647 - We now pass in the feedback dialog
			m_ImportInfo.ImportProceduresAndLadders(&dlgWait);
			// (z.manning, 07/30/2007) - PLID 26869 - Audit.
			if(m_ImportInfo.m_nAuditTransactionID != -1) {
				CommitAuditTransaction(m_ImportInfo.m_nAuditTransactionID);
				m_ImportInfo.m_nAuditTransactionID = -1;
			}
		}

		// (z.manning, 10/11/2007) - PLID 27719 - See if we want to update the font for any existing procedures
		// we are going to be overwriting.
		if(m_ImportInfo.m_bUpdateExistingProceduresFont)
		{
			// (z.manning, 10/11/2007) - Find all procedure IDs for procedures that we're importing that
			// will overwrite an existing procedure.
			CArray<long,long> arynProcedureIDs;
			for(int i = 0; i < m_ImportInfo.m_arypProcedures.GetSize(); i++) {
				NexFormsProcedure *procedure = m_ImportInfo.m_arypProcedures.GetAt(i);
				if(procedure->bImport && procedure->nExistingProcedureID > 0) {
					arynProcedureIDs.Add(procedure->nExistingProcedureID);
				}
			}

			if(arynProcedureIDs.GetSize() > 0) {
				// (z.manning, 10/11/2007) - PLID 27719 - Note: this function has its own progress bar.
				UpdateFontForExistingProcedures(arynProcedureIDs);
			}
		}

		MessageBox("Import Successful!", "Done");

		EndDialog(IDOK);

	}NxCatchAllCall("CNexFormsImportWizardMasterDlg::OnImportNexforms", if(m_ImportInfo.m_nAuditTransactionID != -1) { RollbackAuditTransaction(m_ImportInfo.m_nAuditTransactionID); } );
}

void CNexFormsImportWizardMasterDlg::OnDestroy()
{
	try
	{
		SetEvent(m_ImportInfo.m_hevDestroying);
		// (z.manning, 07/16/2007) - Clean up the post procedure import thread.
		if(m_ImportInfo.m_pPostProcedureThread != NULL)
		{
			DWORD dwExitCode;
			::GetExitCodeThread(m_ImportInfo.m_pPostProcedureThread->m_hThread, &dwExitCode);
			if(dwExitCode == STILL_ACTIVE) {
				// (z.manning, 07/16/2007) - The thread is still running. Tell it to quit and wait up
				// to 5 seconds for it to stop running.
				if(WaitForSingleObject(m_ImportInfo.m_pPostProcedureThread->m_hThread, 5000) == WAIT_TIMEOUT) {
					// (z.manning, 07/16/2007) - This should not ever happen, but just in case, let's kill the thread.
					ASSERT(FALSE);
					TerminateThread(m_ImportInfo.m_pPostProcedureThread->m_hThread, 0);
				}
			}
			delete m_ImportInfo.m_pPostProcedureThread;
			m_ImportInfo.m_pPostProcedureThread = NULL;
		}

	}NxCatchAll("CNexFormsImportWizardMasterDlg::OnDestroy");

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

void CNexFormsImportWizardMasterDlg::OnCancel()
{
	try
	{
		// (z.manning, 10/17/2007) - When downloading the content file, the only way to cancel the download
		// is by clicking this dialog's cancel button, so if they do so, set the destorying event so that the
		// download (amongst other things) knows to stop.
		SetEvent(m_ImportInfo.m_hevDestroying);
		CDialog::OnCancel();

	}NxCatchAll("CNexFormsImportWizardMasterDlg::OnCancel");
}
