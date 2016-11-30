// CodeLinkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CodeLinkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCodeLinkDlg dialog


CCodeLinkDlg::CCodeLinkDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCodeLinkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCodeLinkDlg)
	//}}AFX_DATA_INIT
}


void CCodeLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCodeLinkDlg)
	DDX_Control(pDX, IDC_CHECK_ENABLE_CODELINK, m_btnEnableCodeLink);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_CODELINK_PATH, m_nxeditEditCodelinkPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCodeLinkDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCodeLinkDlg)
	ON_BN_CLICKED(IDC_BTN_BROWSE_CODELINK_PATH, OnBtnBrowseCodelinkPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCodeLinkDlg message handlers

BOOL CCodeLinkDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	CheckDlgButton(IDC_CHECK_ENABLE_CODELINK,GetRemotePropertyInt("EnableCodeLinkLaunching", 0, 0, "<None>", true) ? 1 : 0);

	CString strPath = GetRemotePropertyText("CodeLinkExePath","",0,"<None>",TRUE);

	//try to initialize it to the typical location, if it exists
	if(strPath == "" && DoesExist("C:\\CONTEXT\\CODELINK.EXE")) {
		strPath = "C:\\CONTEXT\\CODELINK.EXE";
		SetRemotePropertyText("CodeLinkExePath",strPath,0,"<None>");
	}
	
	SetDlgItemText(IDC_EDIT_CODELINK_PATH,strPath);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCodeLinkDlg::OnOK() 
{
	CString strPath;

	GetDlgItemText(IDC_EDIT_CODELINK_PATH,strPath);

	if(strPath != "") {

		if(strPath.Right(4).CompareNoCase(".exe") != 0 || !DoesExist(strPath)) {
			AfxMessageBox("Please enter a path pointing to a valid executable file.");
			return;
		}
		else if(strPath.Right(12).CompareNoCase("Codelink.exe") != 0) {
			if(IDNO == MessageBox("The selected program doesn't appear to be a CodeLink file (expected 'CodeLink.exe').\n"
				"Are you sure you wish to use this path?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
	}

	SetRemotePropertyText("CodeLinkExePath",strPath,0,"<None>");

	SetRemotePropertyInt("EnableCodeLinkLaunching", IsDlgButtonChecked(IDC_CHECK_ENABLE_CODELINK) ? 1: 0, 0, "<None>");
	
	CDialog::OnOK();
}

void CCodeLinkDlg::OnBtnBrowseCodelinkPath() 
{
	CString strInitPath;

	// (a.walling 2012-04-27 15:23) - PLID 46648 - Dialogs must set a parent!
	CFileDialog dlgBrowse(TRUE, "exe", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "CodeLink Executable|Codelink.exe|All Files|*.*|", this);

	GetDlgItemText(IDC_EDIT_CODELINK_PATH,strInitPath);
	if (strInitPath != "")
		GetFilePath(strInitPath);
	else strInitPath = "c:\\";

	dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
	if (dlgBrowse.DoModal() == IDOK) 
	{
		SetDlgItemText(IDC_EDIT_CODELINK_PATH,dlgBrowse.GetPathName());		
	}
}
