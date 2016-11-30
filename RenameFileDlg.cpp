// RenameFileDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "RenameFileDlg.h"
#include "FileUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RenameFileDlg dialog


CRenameFileDlg::CRenameFileDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRenameFileDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(RenameFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CRenameFileDlg::CRenameFileDlg(CString strSourceFilename, CString strDstPath, CWnd* pParent /*=NULL*/)
	: CNxDialog(CRenameFileDlg::IDD, pParent)
{
	m_strOldFilename = strSourceFilename;
	// (j.gruber 2009-12-28 10:59) - PLID 36708 - fix error with file being already open (which can happen from labs)
	CFile fileOldFile(m_strOldFilename, CFile::modeRead|CFile::shareDenyNone);
	m_strOldFilename = fileOldFile.GetFileName();
	fileOldFile.Close();
	m_strDstFullPath = strDstPath;
	m_strExtension = m_strOldFilename.Right(m_strOldFilename.GetLength() - m_strOldFilename.ReverseFind('.'));
}


void CRenameFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RenameFileDlg)
		DDX_Control(pDX, IDC_NEW_FILENAME, m_edtNewFilename);
	DDX_Control(pDX, IDC_RENAME_FILE_LABEL, m_nxstaticRenameFileLabel);
	DDX_Control(pDX, IDC_RENAME_LABEL1, m_nxstaticRenameLabel1);
	DDX_Control(pDX, IDC_FILE_EXTENSION, m_nxstaticFileExtension);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRenameFileDlg, CNxDialog)
	//{{AFX_MSG_MAP(RenameFileDlg)
		ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CRenameFileDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 12:46) - PLID 29866 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetTimer(WM_TEMP_TIMER, 10, NULL);

		// (j.gruber 2009-12-21 10:04) - PLID 36482 - fixed typo - 'already' duplicated
		CString strDialogText = "The file '" + m_strOldFilename + "' already exists in this patient's documents file.  Would you like to rename the new file?";
		SetDlgItemText(IDC_RENAME_LABEL1, strDialogText);
		CString strFileNoExt = m_strOldFilename.Left(m_strOldFilename.ReverseFind('.'));
		SetDlgItemText(IDC_NEW_FILENAME, strFileNoExt);
		SetDlgItemText(IDC_FILE_EXTENSION, m_strExtension);
	}
	NxCatchAll("Error in CRenameFileDlg::OnInitDialog");

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// RenameFileDlg message handlers

void CRenameFileDlg::OnTimer(UINT nIDEvent)
{
	KillTimer(WM_TEMP_TIMER);

	m_edtNewFilename.SetFocus();
	m_edtNewFilename.SetSel(0, -1);
}

void CRenameFileDlg::OnOK()
{
	CString strFilename;
	m_edtNewFilename.GetWindowText(strFilename);
	strFilename = strFilename + m_strExtension;
	if(FileUtils::DoesFileOrDirExist(m_strDstFullPath^strFilename)) {
		AfxMessageBox("The filename '" + strFilename + "' already exists in this patient's documents directory.  Please enter a unique filename or cancel the import.");
		m_edtNewFilename.SetFocus();
	}
	else {
		//Append the filename to the path, then close the dialog.
		m_strDstFullPath = m_strDstFullPath ^ strFilename;
		CNxDialog::OnOK();
	}
}
