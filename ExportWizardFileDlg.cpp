// ExportWizardFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportWizardFileDlg.h"
#include "ExportWizardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFileDlg property page

IMPLEMENT_DYNCREATE(CExportWizardFileDlg, CPropertyPage)

CExportWizardFileDlg::CExportWizardFileDlg() : CPropertyPage(CExportWizardFileDlg::IDD)
{
	//{{AFX_DATA_INIT(CExportWizardFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CExportWizardFileDlg::~CExportWizardFileDlg()
{
}

void CExportWizardFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportWizardFileDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EXPORT_FILE_NAME, m_nxeditExportFileName);
	DDX_Control(pDX, IDC_FILENAME_HELP, m_nxstaticFilenameHelp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportWizardFileDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CExportWizardFileDlg)
	ON_BN_CLICKED(IDC_EXPORT_FILE_PROMPT, OnExportFilePrompt)
	ON_BN_CLICKED(IDC_EXPORT_FILE_SPECIFIC, OnExportFileSpecific)
	ON_EN_CHANGE(IDC_EXPORT_FILE_NAME, OnChangeExportFileName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFileDlg message handlers

void CExportWizardFileDlg::OnExportFilePrompt() 
{
	GetDlgItem(IDC_EXPORT_FILE_NAME)->EnableWindow(FALSE);
	((CExportWizardDlg*)GetParent())->m_bPromptForFile = true;

	//They can always go back, and can finish if they're being prompted or have entered a filename.
	CString strFile;
	GetDlgItemText(IDC_EXPORT_FILE_NAME, strFile);
	DWORD dwFinish = (IsDlgButtonChecked(IDC_EXPORT_FILE_PROMPT) || !strFile.IsEmpty())?PSWIZB_FINISH:0;
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|dwFinish);
}

void CExportWizardFileDlg::OnExportFileSpecific() 
{
	GetDlgItem(IDC_EXPORT_FILE_NAME)->EnableWindow(TRUE);
	((CExportWizardDlg*)GetParent())->m_bPromptForFile = false;
	
	//They can always go back, and can finish if they're being prompted or have entered a filename.
	CString strFile;
	GetDlgItemText(IDC_EXPORT_FILE_NAME, strFile);
	DWORD dwFinish = (IsDlgButtonChecked(IDC_EXPORT_FILE_PROMPT) || !strFile.IsEmpty())?PSWIZB_FINISH:0;
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|dwFinish);
}

void CExportWizardFileDlg::OnChangeExportFileName() 
{
	CString strFilename;
	GetDlgItemText(IDC_EXPORT_FILE_NAME, strFilename);
	((CExportWizardDlg*)GetParent())->m_strFileName = strFilename;
	
	//They can always go back, and can finish if they're being prompted or have entered a filename.
	CString strFile;
	GetDlgItemText(IDC_EXPORT_FILE_NAME, strFile);
	DWORD dwFinish = (IsDlgButtonChecked(IDC_EXPORT_FILE_PROMPT) || !strFile.IsEmpty())?PSWIZB_FINISH:0;
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|dwFinish);
}

BOOL CExportWizardFileDlg::OnSetActive()
{
	// (z.manning 2009-12-11 14:06) - PLID 36519 - For history exports we want a directory and not a
	// filename so update the radio buttons' text appropriately.
	if(((CExportWizardDlg*)GetParent())->m_ertRecordType == ertHistory) {
		SetDlgItemText(IDC_EXPORT_FILE_PROMPT, "Prompt me for a directory");
		SetDlgItemText(IDC_EXPORT_FILE_SPECIFIC, "Export to the following directory: ");
	}
	else {
		SetDlgItemText(IDC_EXPORT_FILE_PROMPT, "Prompt me for a filename");
		SetDlgItemText(IDC_EXPORT_FILE_SPECIFIC, "Export to the following file: ");
	}

	if(((CExportWizardDlg*)GetParent())->m_bPromptForFile) {
		CheckRadioButton(IDC_EXPORT_FILE_PROMPT, IDC_EXPORT_FILE_SPECIFIC, IDC_EXPORT_FILE_PROMPT);
		OnExportFilePrompt();
	}
	else {
		CheckRadioButton(IDC_EXPORT_FILE_PROMPT, IDC_EXPORT_FILE_SPECIFIC, IDC_EXPORT_FILE_SPECIFIC);
		OnExportFileSpecific();
	}

	SetDlgItemText(IDC_EXPORT_FILE_NAME, ((CExportWizardDlg*)GetParent())->m_strFileName);

	//They can always go back, and can finish if they're being prompted or have entered a filename.
	CString strFile;
	GetDlgItemText(IDC_EXPORT_FILE_NAME, strFile);
	DWORD dwFinish = (IsDlgButtonChecked(IDC_EXPORT_FILE_PROMPT) || !strFile.IsEmpty())?PSWIZB_FINISH:0;
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|dwFinish);

	((CExportWizardDlg*)GetParent())->SetTitle("Select destination for export \"" + ((CExportWizardDlg*)GetParent())->m_strName + "\"");
	return CPropertyPage::OnSetActive();
}

BOOL CExportWizardFileDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	SetDlgItemText(IDC_FILENAME_HELP, "Note: since this export may be run by users at different workstations, it would be best to specify a path that is universal (i.e., \\\\WS1\\PracStation\\Export.txt rather than c:\\PracStation\\Export.txt).  Also, you may include a %n in the filename; if you do, then the %n will be replaced with a number to differentiate the new file from any pre-existing files.  In other words, if you specify the filename as Export%n.txt, then the first export will be saved as Export1.txt.  If you run the export again, it will be saved as Export2.txt. Otherwise, subsequent exports will overwrite previous ones.");
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
