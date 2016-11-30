// LicenseKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practicerc.h"
#include "LicenseKeyDlg.h"
#include "FileUtils.h"
#include "ASDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLicenseKeyDlg dialog


CLicenseKeyDlg::CLicenseKeyDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLicenseKeyDlg::IDD, pParent)
{
	m_nLicenseKey = -1;
	//{{AFX_DATA_INIT(CLicenseKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLicenseKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseKeyDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ENTER_LICENSE_KEY, m_nxeditEnterLicenseKey);
	DDX_Control(pDX, IDC_LICENSE_KEY_CAPTION, m_nxstaticLicenseKeyCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLicenseKeyDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLicenseKeyDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_LICENSE_LAUNCH_NXDOCK, &CLicenseKeyDlg::OnBnClickedLicenseLaunchNxdock)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLicenseKeyDlg message handlers

BOOL CLicenseKeyDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(ID_LICENSE_LAUNCH_NXDOCK))->AutoSet(NXB_MODIFY);
		
		SetDlgItemText(IDC_LICENSE_KEY_CAPTION, m_strMessage);
		
		if(m_nLicenseKey != -1) {
			SetDlgItemInt(IDC_ENTER_LICENSE_KEY, m_nLicenseKey);
		}

		// (a.walling 2011-04-13 12:20) - PLID 40262
		if (m_strServer.IsEmpty()) {
			CString strServer = CLicense::GetSqlServerNameForLicense(GetSubRegistryKey());

			if (strServer.IsEmpty()) {
				// no override
				strServer = GetSqlServerName();
			}

			m_strServer = strServer;
		}

		if (m_strOfficialServer.IsEmpty()) {
			m_strOfficialServer = GetOfficialServerName();
		}
		
		CString strCaption2;
		if (m_strOfficialServer.IsEmpty() || (m_strOfficialServer.CompareNoCase(m_strServer) == 0)) {
			strCaption2.Format("You are currently identifying the server name as %s. "
				"\r\n\r\nPlease ensure this is correct before continuing!", m_strServer);

			m_nxeditEnterLicenseKey.SetFocus();
		} else {
			strCaption2.Format("You are currently identifying the server name as %s. "
				"However, the official server name is %s. "
				"\r\n\r\nPlease ensure this is correct before continuing!"
				, m_strServer, m_strOfficialServer);

			GetDlgItem(ID_LICENSE_LAUNCH_NXDOCK)->SetFocus();
		}

		SetDlgItemText(IDC_LICENSE_KEY_CAPTION2, strCaption2);

	} NxCatchAllCall(__FUNCTION__, CNxDialog::OnCancel());

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLicenseKeyDlg::OnOK() 
{
	CString strKey;
	GetDlgItemText(IDC_ENTER_LICENSE_KEY, strKey);
	if(strKey.IsEmpty()) {
		MsgBox("Please enter a license key before clicking OK.");
		return;
	}

	// (j.jones 2016-01-26 08:52) - PLID 67723 - this warning is now meaningless if you aren't using
	// SQLServerNameForLicense... and even then is still fairly pointless.
	CString strSQLServerNameForLicense = CLicense::GetSqlServerNameForLicense(GetSubRegistryKey());
	
	// (a.walling 2011-04-13 12:20) - PLID 40262
	if (!m_strOfficialServer.IsEmpty() && (m_strOfficialServer.CompareNoCase(m_strServer) != 0)
		&& !strSQLServerNameForLicense.IsEmpty()) {
		
		CASDDlg dlg(this);
		dlg.SetParams(
			FormatString(
				"The current server is identified as %s which does not match the official server name of %s. "
				"You should run NxDock Client and either clear out SqlServerNameForLicense, or update it to match the server name. \r\n\r\n"
				"Existing valid licensing information could be corrupted if this is incorrect. If you are not certain, please contact NexTech Technical Support.\r\n\r\n"
				"Do you want to continue updating the licensing information?", m_strServer, m_strOfficialServer),
			"Updating licensing information",
			"WARNING! Please read and review this carefully:",
			"I understand this warning and want to continue");

		if (IDOK != dlg.DoModal()) {
			return;
		}
	}

	m_nLicenseKey = atoi(strKey);
	
	CDialog::OnOK();
}

// (a.walling 2011-04-13 12:20) - PLID 40262
void CLicenseKeyDlg::OnBnClickedLicenseLaunchNxdock()
{
	try {
		CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
		if (strPath.IsEmpty()) {
			// (j.armen 2011-10-25 10:52) - PLID 46137 - We already have the practice exe path here
			strPath = FileUtils::GetFilePath(GetPracPath(PracPath::PracticeEXEPath));
		}

		if (strPath.IsEmpty()) {
			strPath = "NxDockClientToServer.exe";
		} else {
			strPath = strPath ^ "NxDockClientToServer.exe";
		}

		SHELLEXECUTEINFO sei;
		ZeroMemory(&sei, sizeof(sei));

		sei.cbSize = sizeof(sei);

		sei.hwnd = GetSafeHwnd();
		sei.lpFile = strPath;
		sei.nShow = SW_SHOWNORMAL;

		if (ShellExecuteEx(&sei) && (int)sei.hInstApp > 32) {
			return;
		} else {
			ThrowNxException("Failed to launch NxDock Client!");
		}
	} NxCatchAll(__FUNCTION__);
}
