// DevicePluginSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DevicePluginSetupDlg.h"
#include "DevicePlugin.h"
#include "PracticeRc.h"
#include "FileUtils.h"

// (d.lange 2010-05-19 14:10) - PLID 38536 - Created
// CDevicePluginSetupDlg dialog

IMPLEMENT_DYNAMIC(CDevicePluginSetupDlg, CNxDialog)

CDevicePluginSetupDlg::CDevicePluginSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDevicePluginSetupDlg::IDD, pParent)
{
	m_strDeviceExportPath = "";
	m_strDeviceAddlPath = "";
	m_strDeviceExePath = "";
	m_strPluginName = "";
	m_nLaunchSetting = -1;
	// (b.savon 2012-02-07 15:11) - PLID 48019 - Handle the Topcon Synergy WebLink on the Practice side.
	// (j.gruber 2013-04-22 13:16) - PLID 56032 - not needed anymore
	//m_strPlugin = "";
}

CDevicePluginSetupDlg::~CDevicePluginSetupDlg()
{
}

void CDevicePluginSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DEVICE_EXPORT_PATH, m_nxeditImportPath);
	DDX_Control(pDX, IDC_DEVICE_ADDL_PATH, m_nxeditExportPath);
	DDX_Control(pDX, IDC_EXE_PATH, m_nxeditExePath);
	DDX_Control(pDX, IDC_EDIT_PLUGIN_NAME, m_nxeditPluginName);
}

BOOL CDevicePluginSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Set controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Set the max length for the Plugin Name editbox
		m_nxeditPluginName.SetLimitText(255);
		m_nxeditImportPath.SetLimitText(400);
		m_nxeditExportPath.SetLimitText(400);
		m_nxeditExePath.SetLimitText(400);

		//Set the static label to the plugin name
		SetDlgItemText(IDC_DEVICE_PLUGIN_NAME, m_strPluginName);
		SetDlgItemText(IDC_DEVICE_EXPORT_PATH, m_strDeviceExportPath);
		SetDlgItemText(IDC_EXE_PATH, m_strDeviceExePath);
		SetDlgItemText(IDC_DEVICE_ADDL_PATH, m_strDeviceAddlPath);
		SetDlgItemText(IDC_EDIT_PLUGIN_NAME, m_strPluginName);

		// (b.savon 2012-02-07 15:11) - PLID 48019 - Handle the Topcon Synergy WebLink on the Practice side.
		// (j.gruber 2013-04-22 13:14) - PLID 56032 - this isn't needed anymore
		/*m_strPlugin = m_strPluginName;
		if( m_strPlugin.CompareNoCase("Topcon Synergy WebLink") == 0 ){
			GetDlgItem(IDC_STATIC_EXPORT)->SetWindowTextA("Synergy Export Path:");
			GetDlgItem(IDC_STATIC_EXE)->SetWindowTextA("Synergy WebLink URL:");
			GetDlgItem(IDC_BTN_BROWSE_EXE)->ShowWindow(SW_HIDE);
		}*/

		//Now let's only enable certain NxEdits based on the LaunchDevice() functionality
		if(m_nLaunchSetting != -1) {
			switch(m_nLaunchSetting) {
				case 0:		//the plugin does not support the LaunchDevice() functionality
					GetDlgItem(IDC_DEVICE_EXPORT_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXPORT)->EnableWindow(TRUE);

					GetDlgItem(IDC_DEVICE_ADDL_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_BROWSE_IMPORT)->EnableWindow(FALSE);
					SetDlgItemText(IDC_DEVICE_ADDL_PATH, "< Not Supported >");
				
					GetDlgItem(IDC_EXE_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_BROWSE_EXE)->EnableWindow(FALSE);
					SetDlgItemText(IDC_EXE_PATH, "< Not Supported >");
					break;
				case 1:		//the plugin supports LaunchDevice(), and needs no parameters
					GetDlgItem(IDC_DEVICE_EXPORT_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXPORT)->EnableWindow(TRUE);

					GetDlgItem(IDC_DEVICE_ADDL_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_IMPORT)->EnableWindow(TRUE);
				
					GetDlgItem(IDC_EXE_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_BROWSE_EXE)->EnableWindow(FALSE);
					SetDlgItemText(IDC_EXE_PATH, "< Not Supported >");
					break;
				case 2:		//the plugin requires a path to the exe of the device program
					GetDlgItem(IDC_DEVICE_EXPORT_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXPORT)->EnableWindow(TRUE);

					GetDlgItem(IDC_DEVICE_ADDL_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_BROWSE_IMPORT)->EnableWindow(FALSE);
					SetDlgItemText(IDC_DEVICE_ADDL_PATH, "< Not Supported >");
				
					GetDlgItem(IDC_EXE_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXE)->EnableWindow(TRUE);
					break;
				case 3:		//the plugin requires a path to which to export content
					GetDlgItem(IDC_DEVICE_EXPORT_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXPORT)->EnableWindow(TRUE);

					GetDlgItem(IDC_DEVICE_ADDL_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_IMPORT)->EnableWindow(TRUE);
				
					GetDlgItem(IDC_EXE_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_BROWSE_EXE)->EnableWindow(FALSE);
					SetDlgItemText(IDC_EXE_PATH, "< Not Supported >");
					break;
				case 4:		//the plugin requires both a path to the exe and a path to export content
					GetDlgItem(IDC_DEVICE_EXPORT_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXPORT)->EnableWindow(TRUE);

					GetDlgItem(IDC_DEVICE_ADDL_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_IMPORT)->EnableWindow(TRUE);
				
					GetDlgItem(IDC_EXE_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXE)->EnableWindow(TRUE);
					break;
				case 5:
					// (j.gruber 2013-04-01 14:20) - PLID 56032
					//the plugin requires a URL to export content to, and no browse button
					GetDlgItem(IDC_DEVICE_EXPORT_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXPORT)->EnableWindow(TRUE);

					GetDlgItem(IDC_DEVICE_ADDL_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_BTN_BROWSE_IMPORT)->EnableWindow(FALSE);
					SetDlgItemText(IDC_DEVICE_ADDL_PATH, "< Not Supported >");
				
				
					GetDlgItem(IDC_EXE_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_BTN_BROWSE_EXE)->ShowWindow(SW_HIDE);
					

					GetDlgItem(IDC_STATIC_EXPORT)->SetWindowTextA("Export Path:");
					GetDlgItem(IDC_STATIC_EXE)->SetWindowTextA("URL:");
				break;
				default:
					break;
			}
		}

	} NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CDevicePluginSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_BROWSE_IMPORT, &CDevicePluginSetupDlg::OnBnClickedBtnBrowseAddl)
	ON_BN_CLICKED(IDC_BTN_BROWSE_EXPORT, &CDevicePluginSetupDlg::OnBnClickedBtnBrowseExport)
	ON_BN_CLICKED(IDC_BTN_BROWSE_EXE, &CDevicePluginSetupDlg::OnBnClickedBtnBrowseExe)
	ON_EN_CHANGE(IDC_EDIT_PLUGIN_NAME, &CDevicePluginSetupDlg::OnEnChangeEditPluginName)
	ON_EN_CHANGE(IDC_DEVICE_EXPORT_PATH, &CDevicePluginSetupDlg::OnEnChangeDeviceExportPath)
	ON_EN_CHANGE(IDC_EXE_PATH, &CDevicePluginSetupDlg::OnEnChangeExePath)
	ON_EN_CHANGE(IDC_DEVICE_ADDL_PATH, &CDevicePluginSetupDlg::OnEnChangeDeviceAddlPath)
	ON_BN_CLICKED(IDOK, &CDevicePluginSetupDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CDevicePluginSetupDlg message handlers

void CDevicePluginSetupDlg::OnBnClickedBtnBrowseAddl()
{
	try {
		CString strDeviceAddlPath = "";
		//Browse to the Additional path
		if (BrowseToFolder(&strDeviceAddlPath, "Select Folder for the Device Additional Path", GetSafeHwnd(), NULL, NULL)) {
			//Set the editbox with the returned value
			SetDlgItemText(IDC_DEVICE_ADDL_PATH, strDeviceAddlPath);
			m_strDeviceAddlPath = strDeviceAddlPath;
		}

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnBnClickedBtnBrowseExport()
{
	try {
		CString strDeviceExportPath = "";
		//Browse to the Export path
		if (BrowseToFolder(&strDeviceExportPath, "Select Folder for the Device Export Path", GetSafeHwnd(), NULL, NULL)) {
			//Set the editbox with the returned value
			SetDlgItemText(IDC_DEVICE_EXPORT_PATH, strDeviceExportPath);
			m_strDeviceExportPath = strDeviceExportPath;
		}

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnBnClickedBtnBrowseExe()
{
	try {
		CString strDeviceExePath = "";
		//Browse to the Executable path
		CFileDialog BrowseFiles(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, "All Files (*.*)|*.*|");
		if(BrowseFiles.DoModal() == IDCANCEL) return;
		m_strDeviceExePath = BrowseFiles.GetPathName();
		SetDlgItemText(IDC_EXE_PATH, m_strDeviceExePath);

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnEnChangeEditPluginName()
{
	try {
		CString strPluginName;
		GetDlgItemText(IDC_EDIT_PLUGIN_NAME, strPluginName);
		m_strPluginName = strPluginName;

	} NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnEnChangeDeviceExportPath()
{
	try{
		if(GetDlgItem(IDC_DEVICE_EXPORT_PATH)->IsWindowEnabled()) {
			CString strDeviceExportPath;
			GetDlgItemText(IDC_DEVICE_EXPORT_PATH, strDeviceExportPath);
			m_strDeviceExportPath = strDeviceExportPath;
		}
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnEnChangeExePath()
{
	try{
		if(GetDlgItem(IDC_EXE_PATH)->IsWindowEnabled()) {
			CString strDeviceExePath;
			GetDlgItemText(IDC_EXE_PATH, strDeviceExePath);
			m_strDeviceExePath = strDeviceExePath;
		}
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnEnChangeDeviceAddlPath()
{
	try{
		if(GetDlgItem(IDC_DEVICE_ADDL_PATH)->IsWindowEnabled()) {
			CString strDeviceAddlPath;
			GetDlgItemText(IDC_DEVICE_ADDL_PATH, strDeviceAddlPath);
			m_strDeviceAddlPath = strDeviceAddlPath;
		}
	}NxCatchAll(__FUNCTION__);
}

void CDevicePluginSetupDlg::OnBnClickedOk()
{
	try {
		// (b.savon 2011-9-13) - PLID 45455 - The Device Plugin Setup dialog does not invalidate a blank plugin name.
		//Validate the Plugin Name
		if(m_strPluginName.Trim().IsEmpty()) {
			AfxMessageBox("Please enter a valid Device Plugin Name.");
			m_nxeditPluginName.SetFocus();
			return;
		}

		// (b.savon 2011-9-15) - PLID 43987 - Trim paths
		m_strDeviceExportPath = m_strDeviceExportPath.Trim();
		m_strDeviceExePath = m_strDeviceExePath.Trim();
		m_strDeviceAddlPath = m_strDeviceAddlPath.Trim();

		//Validate the Export Path
		if(!m_strDeviceExportPath.IsEmpty()) {
			if(!FileUtils::DoesFileOrDirExist(m_strDeviceExportPath) && GetDlgItem(IDC_DEVICE_EXPORT_PATH)->IsWindowEnabled()) {
				AfxMessageBox("The Device Export Path is invalid!");
				return;
			} // (b.savon 2011-9-15) - PLID 43987 - Invalidate blank export path 
		} else if( m_strDeviceExportPath.IsEmpty() && GetDlgItem(IDC_DEVICE_EXPORT_PATH)->IsWindowEnabled() ) {
			MessageBox("Please define an export path.", "Invalid Export Path", MB_ICONWARNING);
			return;
		}

		//Validate the Executable Path
		if(!m_strDeviceExePath.IsEmpty()) {
			// (b.savon 2012-02-07 15:34) - PLID 48019 - Make sure we don't check if the Exe Path exists if we are
			// setting up the Topcon Synergy WebLink plugin.
			// (j.gruber 2013-04-01 14:20) - PLID  56032 - make sure its not a link
			if(m_nLaunchSetting != 5 && !FileUtils::DoesFileOrDirExist(m_strDeviceExePath) && GetDlgItem(IDC_EXE_PATH)->IsWindowEnabled()) {
				AfxMessageBox("The Device Executable Path is invalid!");
				return;
			} // (b.savon 2011-9-15) - PLID 43987 - Invalidate blank export path 
		} else if( m_strDeviceExePath.IsEmpty() && GetDlgItem(IDC_EXE_PATH)->IsWindowEnabled() ) {
			// (j.gruber 2013-04-04 16:05) - PLID 56032 - give the appropriate message
			CString strTitle;
			strTitle = m_nLaunchSetting == 5 ? "Invalid URL" : "Invalid Executable Path";
			MessageBox(FormatString("Please define %s.", m_nLaunchSetting == 5 ? "a URL" : "an executable path"), strTitle, MB_ICONWARNING);
			return;
		}

		//Validate the Additional Path
		if(!m_strDeviceAddlPath.IsEmpty()) {
			if(!FileUtils::DoesFileOrDirExist(m_strDeviceAddlPath) && GetDlgItem(IDC_DEVICE_ADDL_PATH)->IsWindowEnabled()) {
				AfxMessageBox("The Device Additional Path is invalid!");
				return;
			} // (b.savon 2011-9-15) - PLID 43987 - Invalidate blank export path 
		} else if( m_strDeviceAddlPath.IsEmpty() && GetDlgItem(IDC_DEVICE_ADDL_PATH)->IsWindowEnabled() ) {
			MessageBox("Please define an additional path.", "Invalid Additional Path", MB_ICONWARNING);
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}
