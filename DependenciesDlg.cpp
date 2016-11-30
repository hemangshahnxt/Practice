// DependenciesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "aboutdlg.h"
#include "DependenciesDlg.h"
#include "FileUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDependenciesDlg dialog


CDependenciesDlg::CDependenciesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDependenciesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDependenciesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDependenciesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDependenciesDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_FILELIST, m_nxeditFilelist);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDependenciesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CDependenciesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern CPracticeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDependenciesDlg message handlers


void CDependenciesDlg::DisplayFileDetails(const CString &file, const CString &path)
{
	// Decide whether to use the given path or the dll's actual path
	CString strFilePathName;
	// Try to get the file path as if it's a module that's loaded into 
	// memory (a .dll, .exe, .ocx, or maybe some other kind of module)
	if (!GetDllFilePathName(file, strFilePathName)) {
		// Not a loaded module so use the given path
		strFilePathName = path ^ file;
	}

	// Get the file details
	CString ver = CAboutDlg::GetFileVersion(strFilePathName);
	if (ver.IsEmpty()) ver = "Not Found";
	CString date = CAboutDlg::GetFileDateAsString(strFilePathName);
	if (date.IsEmpty()) date = "Not Found";
	
	
	// Show the file details
	
	
	// Output filename
	m_output += file;

	// delimit	
	if (file.GetLength() < 8) m_output += "\t";
	if (file.GetLength() < 16) m_output += "\t";
	if (file.GetLength() < 24) m_output += "\t";
	m_output += " ";
	
	// Output version
	m_output += ver;
	
	// delimit
	//if (ver.GetLength() < 8) m_output += "\t";
	if (ver.GetLength() < 16) m_output += "\t";
	
	// Output date
	m_output += date;

	m_output += "\r\n";
}

void CDependenciesDlg::DisplayFileVersion()
{
	CString sysDir, pracDir, palmDir;

	GetSystemPath(sysDir);
	GetPalmPath(palmDir);

	// (j.armen 2011-10-24 16:03) - PLID 46132 - Use the local shared practice path
	pracDir = GetPracPath(PracPath::PracticePath);

	// Get this exe's path
	// (j.armen 2011-10-24 16:02) - PLID 46132 - We already have the path, so let's use it.
	CString strExePath = FileUtils::GetFilePath(GetPracPath(PracPath::PracticeEXEPath));


	DisplayFileDetails("Asycfilt.dll",		sysDir);
	DisplayFileDetails("Comcat.dll",		sysDir);
	DisplayFileDetails("Comdlg32.ocx",		sysDir);
	DisplayFileDetails("Mfc42.dll",			sysDir);
	DisplayFileDetails("Mschrt20.ocx",		sysDir);
	DisplayFileDetails("Mscomct2.ocx",		sysDir);
	DisplayFileDetails("Mscomctl.ocx",		sysDir);
	DisplayFileDetails("Msflxgrd.ocx",		sysDir);
	DisplayFileDetails("Msvcirt.dll",		sysDir);
	DisplayFileDetails("Msvcrt.dll",		sysDir);
	DisplayFileDetails("Oleaut32.dll",		sysDir);
	DisplayFileDetails("Olepro32.dll",		sysDir);

	DisplayFileDetails("Crpe32.dll",		sysDir);
	DisplayFileDetails("Fm20.dll",			sysDir);
	DisplayFileDetails("Fm20enu.dll",		sysDir);
	DisplayFileDetails("mscomm32.ocx",		sysDir);
	DisplayFileDetails("pdqcomm.ocx",		sysDir);
	DisplayFileDetails("Tab32x20.ocx",		sysDir);
	DisplayFileDetails("Xceedzip.dll",		sysDir);
	DisplayFileDetails("T2klib.dll",		pracDir);

	DisplayFileDetails("Datcn20.dll",		palmDir);
	DisplayFileDetails("Addcn30.dll",		palmDir);

	DisplayFileDetails("NxDataList.ocx",	pracDir);
	DisplayFileDetails("NxDataList2.ocx",	pracDir);
	DisplayFileDetails("NxStandard.dll",	pracDir);
	DisplayFileDetails("Preferences.dll",	pracDir);
	DisplayFileDetails("NxCanfieldLink.dll",pracDir);
	DisplayFileDetails("NxDockClientToServer.exe",	pracDir);
	DisplayFileDetails("NxQuery.exe",		pracDir);
	DisplayFileDetails("NxServer.exe",		pracDir);
	DisplayFileDetails("NxTray.exe",		pracDir);

	DisplayFileDetails("Nx3D.ocx",			pracDir); // (z.manning 2011-10-24 16:09) - PLID 44649
	DisplayFileDetails("NxColor.ocx",		pracDir);
	DisplayFileDetails("NxColumnGraph.ocx",	pracDir);
	DisplayFileDetails("NxResList.ocx",		pracDir);
	DisplayFileDetails("NxTab.ocx",			pracDir);
	DisplayFileDetails("NxTextbox.ocx",		pracDir);
	DisplayFileDetails("Singleday.ocx",		pracDir);

	m_output += "\r\n";

	DisplayFileDetails("Practice.exe",		strExePath);

	DWORD dwVer = CAboutDlg::GetPracticeVersion();
	CString strInstallVersion;
	strInstallVersion.Format("\r\n\r\n    Install Version:  %i.%i.%i\r\n", (dwVer & 0xFF000000) >> 24, (dwVer & 0x00FF0000) >> 16, (dwVer & 0x0000FFFF));
	m_output += strInstallVersion;

	SetDlgItemText(IDC_FILELIST, m_output);
/*	// Get and show the dates of files in the system directory
	CString strWinSysDir;
	GetSystemPath(strWinSysDir);
	SetDlgItemdllFileDate(IDC_MFC42_DATE, "Mfc42.dll");
	SetDlgItemdllFileDate(IDC_MSVCRT_DATE, "Msvcrt.dll");
	SetDlgItemdllFileDate(IDC_MSVCRT40_DATE, "Msvcrt40.dll");

	// Get and show the dates of files in the palm directory
	CString strpalmDir;
	GetPalmPath(strpalmDir);
	SetDlgItemFileDate(IDC_DATCN20_DATE, strpalmDir ^ "DatCn20.dll");

	// Get and show the dates of the actual file executable
	CString strexePath;
	GetModuleFileName(NULL, strexePath.GetBuffer(MAX_PATH), MAX_PATH);
	strexePath.ReleaseBuffer();
	SetDlgItemText(IDC_CUSTOM_DATE1, PRODUCT_VERSION_TEXT);
	SetDlgItemFileDate(IDC_PRACTICE_DATE, strexePath);

	// Get and show the dates of files in the practice directory
	SetDlgItemdllFileDate(IDC_COLUMN_GRAPH_DATE_BOX, "NxColumnGraph.ocx");
	SetDlgItemdllFileDate(IDC_RES_LIST_DATE, "NxResList.ocx");
	SetDlgItemdllFileDate(IDC_SINGLEDAY_DATE_BOX, "SingleDay.ocx");
	SetDlgItemdllFileDate(IDC_DATALIST_DATE_BOX, "NxataList.ocx");*/

}


BOOL CDependenciesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);
	
	GetDlgItem(IDC_FILELIST)->SetFont(&theApp.m_fixedFont);
	DisplayFileVersion();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
