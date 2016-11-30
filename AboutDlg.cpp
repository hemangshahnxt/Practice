#include <stdafx.h>
#include "AboutDlg.h"
#include "VersionInfo.h"
#include "Client.h"
#include "GlobalDataUtils.h"
#include "GlobalUtils.h"
#include "practicerc.h"
#include "dependenciesdlg.h"
#include "backup.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "LicenseInfoDlg.h"
#include "NxGdiPlusUtils.h"
#include "FileUtils.h"
#include "Syslog.h"
#include <NxUILib/WindowUtils.h>
#define _WIN32_MSI 110	//USE 1.1 installer API
#include <msi.h>

using namespace ADODB;
//BVB Yes, I know we convert this to a DWORD, and then back to the exact same string
//I'd rather keep the functions the same as other places, even if the actual code isn't shared
static DWORD parseVersion(char *buffer)
{
	long dot;
	DWORD version = 0;
	char *p = buffer;

	//major
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version = atoi(p) << 24;
	p += dot + 1;

	//minor
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version += atoi(p) << 16;
	p += dot + 1;

	//build
	dot = strchr(p, '.') - p;
	if (dot == -1)
		return version;
	version += atoi(p);

	return version; 
}

DWORD CAboutDlg::GetPracticeVersion()
{
	//enumerate all version of Nextech Practice Installed
	//return the highest version
	//in almost all cases, only 1 copy will be installed
	DWORD	size = 12,
			result,
			version,
			maxVersion = 0,
			i = 0;

	char	productCode[39],
			buffer[12];

	bool	done;

	do
	{
		done = true;
		result = MsiEnumRelatedProducts("{D98C20EB-4CF3-435F-B0C0-4C2967B4607C}", 0, i, productCode);
		if (result == ERROR_SUCCESS)
		{
			done = false;
			size = 12;
			result = MsiGetProductInfo(productCode, INSTALLPROPERTY_VERSIONSTRING, buffer, &size);

			if (result == ERROR_SUCCESS)
			{
				version = parseVersion(buffer);

				if (version > maxVersion)
					maxVersion = version;
			}

			i++;
		}
	}	while (!done);

	return maxVersion;
}

CString CAboutDlg::GetFileDateAsString(const CString &strFilePathName)
{
	CFileFind ff;

	// Find the file
	if (!strFilePathName.IsEmpty() && ff.FindFile(strFilePathName, 0)) {
		// Get the file modified time
		ff.FindNextFile();
		CTime tmpTime;
		ff.GetLastWriteTime(tmpTime);
		
		// Put it on the dialog
		COleDateTime dt(tmpTime.GetYear(), tmpTime.GetMonth(), tmpTime.GetDay(), 0, 0, 0);
		return FormatDateTimeForInterface(dt, NULL, dtoDate, false);
	}
	
	// If we made it to here we failed
	return "";
}

////////////////////////////////////////////
///  CAboutDlg definition

CAboutDlg::CAboutDlg(CWnd* pParent) : CNxDialog(CAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT

	m_pIcon = NULL;
	// (d.thompson 2010-03-05) - PLID 37656
	m_pBetterCareIcon = NULL;
	// (s.dhole 2013-07-01 18:32) - PLID 57404
	m_nLastServerTimeSeconds = 0;
	m_nServerTimeOffset = 0;
	m_nTimerRefresh=WM_USER + 10000;
	m_dtLastLocalDateTime.SetStatus(COleDateTime::invalid);
	m_dtServerDate.SetStatus(COleDateTime::invalid);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_SUPPORT_EXPIRES, m_strSupportExpires);
	DDX_Control(pDX, IDC_USER_NAME_BOX, m_nxeditUserNameBox);
	DDX_Control(pDX, IDC_LOC_NAME_BOX, m_nxeditLocNameBox);
	DDX_Control(pDX, IDC_MAC_NAME_BOX, m_nxeditMacNameBox);
	DDX_Control(pDX, IDC_DOCK_PATH_LABEL, m_nxeditOckPathLabel);
	DDX_Control(pDX, IDC_LOCAL_PATH_LABEL, m_nxeditLocalPathLabel);
	DDX_Control(pDX, IDC_NXSERVER, m_nxeditNxserver);
	DDX_Control(pDX, IDC_NEXT_BACKUP_LABEL, m_nxeditNextBackupLabel);
	DDX_Control(pDX, IDC_SHARED_PATH_LABEL, m_nxeditSharedPathLabel);
	DDX_Control(pDX, IDC_SESSION_PATH_LABEL, m_nxeditSessionPathLabel);	 // (a.walling 2012-09-04 17:32) - PLID 52449 - Include session path
	DDX_Control(pDX, IDC_MAILING_ADDRESS_LABEL, m_nxeditMailingAddressLabel);
	DDX_Control(pDX, IDC_PRODUCT_VERSION_LABEL, m_nxstaticProductVersionLabel);
	DDX_Control(pDX, IDC_PRODUCT_VERSION_TEXT, m_nxstaticProductVersionText);
	DDX_Control(pDX, IDC_OS_VERSION, m_nxstaticOsVersion);
	DDX_Control(pDX, IDC_NUMBERS, m_nxstaticNumbers);
	DDX_Control(pDX, IDC_SQL_VERSION, m_nxstaticSqlVersion);
	DDX_Control(pDX, IDC_OTHER_INFO, m_nxstaticOtherInfo); // (a.walling 2010-01-27 16:21) - PLID 37108 - Box for BSD license-mandated acknowledgements
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_BN_CLICKED(IDC_EMAIL, OnEmail)
	ON_BN_CLICKED(IDC_DEPENDENCIES, OnDependencies)
	// (a.walling 2012-09-04 17:32) - PLID 52449 - Handle path clicks
	ON_STN_CLICKED(IDC_LOCAL_PATH_STATIC, OnClickLocalPath)
	ON_STN_CLICKED(IDC_SHARED_PATH_STATIC, OnClickSharedPath)
	ON_STN_CLICKED(IDC_SESSION_PATH_STATIC, OnClickSessionPath)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_BACKUP_MANAGER, OnBtnBackupManager)
	ON_BN_CLICKED(IDC_BTN_LICENSE, OnBtnLicense)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	// (a.walling 2012-09-04 17:32) - PLID 52449 - Handle path link cursor
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern CPracticeApp theApp;

BOOL CAboutDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-28 11:16) - PLID 29793 - NxIconized the OK button
		m_btnOK.AutoSet(NXB_CLOSE);
		
		CWaitCursor wc;

		m_bSupportExp = FALSE;
		
		GetDlgItem(IDC_PRODUCT_VERSION_LABEL)->SetFont(&theApp.m_subtitleFont);
		GetDlgItem(IDC_PRODUCT_VERSION_TEXT)->SetFont(&theApp.m_subtitleFont);	
		
		
		//set the phone numbers
		GetDlgItem(IDC_NUMBERS)->SetFont(&theApp.m_boldFont);
		// (j.gruber 2007-08-06 09:20) - PLID 26901 - added location and FL tech support number
		//DRT 5/9/2008 - PLID 29995 - Changed Outside NA phone number for support
		CString strPhoneNumbers;
		strPhoneNumbers.Format("TechSupport: East Coast, Midwest States, and Canada: (866) 654 4396\n"
							   "                      West Coast and Central States: (888) 417 8464\n"
							   "                      Outside North America: +1 (937) 438 1095 or +1 (813) 425 9270\n\n"
							   "Sales: United States and Canada: (800) 490 0821\n"
							   "           Outside North America: +1 (813) 425 9200");
		SetDlgItemText(IDC_NUMBERS, strPhoneNumbers);

		//22738 - Add mailing address
		// (d.thompson 2010-03-05) - PLID 37658 - New address (2 years ago)
		SetDlgItemText(IDC_MAILING_ADDRESS_LABEL, "5550 W Executive Dr Suite 350\r\n"
			"Tampa, FL  33609");


		// Get this exe's path
		// (j.armen 2011-10-24 15:56) - PLID 46132 - We have the practice exe path stored.
		CString strExePath = GetPracPath(PracPath::PracticeEXEPath);

		//Show main version
		//DRT 6/20/2007 - PLID 26406 - Changed to 2008.
		// (d.thompson 2009-01-08) - PLID 32656 - Changed to 2009.
		// (d.thompson 2009-04-30) - PLID 33707 - Changed to 2010.
		// (z.manning 2010-01-08 15:37) - PLID 36427 - 2011
		// (b.savon 2011-8-23) - PLID 44566 - Changed to 2012
		// (j.dinatale 2013-02-12 11:54) - PLID 55075 - 2013
		// (j.jones 2014-01-27 16:35) - PLID 60475 - 2014
		// (r.gonet 04/02/2014) - PLID 61628 - Change instances of "Practice 20NN" to "Nextech".
		CString strVer;
		DWORD dwVer = GetPracticeVersion();		
		strVer.Format("(V %i.%i) exe: %s", (dwVer & 0xFF000000) >> 24, (dwVer & 0x00FF0000) >> 16, GetFileDateAsString(strExePath));
		SetDlgItemText(IDC_PRODUCT_VERSION_TEXT, strVer);

		// Show other info
		SetDlgItemText(IDC_DOCK_PATH_LABEL, GetSqlServerName());
		SetDlgItemText(IDC_SHARED_PATH_LABEL, (GetSharedPath() ^ ""));
		// (j.armen 2011-10-24 14:49) - PLID 46132 - Show the shared practice path
		SetDlgItemText(IDC_LOCAL_PATH_LABEL, (GetPracPath(PracPath::PracticePath) ^ ""));
		// (a.walling 2012-09-04 17:32) - PLID 52449 - Include session path
		SetDlgItemText(IDC_SESSION_PATH_LABEL, (GetPracPath(PracPath::SessionPath) ^ ""));
		SetDlgItemText(IDC_NEXT_BACKUP_LABEL, GetNextBackupTime());
		SetDlgItemText(IDC_BACKUP_TO_LABEL, GetUserBackupDirectory());

		// Added by Chris 6/15
		CString strOSVer;
		GetOsVersion(strOSVer);
		SetDlgItemText(IDC_OS_VERSION, CString("OS:  ") + strOSVer);

		// BVB 5/4/01
		// (j.armen 2011-10-26 14:58) - PLID 46132 - Removed dead code and simplified
		DWORD dwIP = inet_addr(NxRegUtils::ReadString(GetRegistryBase() + "NxServerIP"));
		CString strIP;
		strIP.Format("%i.%i.%i.%i", 
			dwIP & 0xFF, 
			(dwIP & 0xFF00) >> 8, 
			(dwIP & 0xFF0000) >> 16, 
			(dwIP & 0xFF000000) >> 24);
		
		SetDlgItemText(IDC_NXSERVER, strIP);

		SetDlgItemText(IDC_USER_NAME_BOX, GetCurrentUserName());
		try
		{
			SetDlgItemText(IDC_LOC_NAME_BOX, GetCurrentLocationName());
			_RecordsetPtr rs = CreateRecordset("SELECT @@version AS Name");

			// (a.walling 2010-01-28 17:05) - PLID 37108 - Get rid of the tabs since this makes it a bit too long now
			CString strSqlVersion = AdoFldString(rs, "Name");
			strSqlVersion.Replace("\t", " ");
			SetDlgItemText(IDC_SQL_VERSION, strSqlVersion);
		}
		catch (...)//just don't set anything
		{
		}

		// (a.walling 2010-01-27 16:22) - PLID 37108 - Box for BSD license-mandated acknowledgements
		// (a.walling 2010-03-16 11:13) - PLID 37769 - OpenSSL has a BSD-like license, that requires this acknowledgement
		SetDlgItemText(IDC_OTHER_INFO, 
			"Portions of this software Copyright © 2007 TWAIN Working Group: "
			"Adobe Systems Incorporated, AnyDoc Software Inc., Eastman Kodak Company, "
			"Fujitsu Computer Products of America, JFL Peripheral Solutions Inc., "
			"Ricoh Corporation, and Xerox Corporation. All rights reserved."
			"\r\n\r\n"
			"This product includes software developed by the OpenSSL Project "
			"for use in the OpenSSL Toolkit (http://www.openssl.org/).");
		//(e.lally 2011-05-24) PLID 42819 - PDFSharp is an open-source MIT License.
		SetDlgItemText(IDC_OTHER_INFO2,
			"PDFSharp Copyright © 2005-2009 empira Software GmbH, Cologne (Germany)"
			"\r\nPortions of this software Copyright © 2007 XtractPro (Data Extraction Magazine)"			);
		// (j.gruber 2012-06-18 15:40) - PLID 50844	
		SetDlgItemText(IDC_OTHER_INFO3,												
			"Portions of this software derived from:\r\n"
			"ScrollView - jQuery plugin 0.1 Copyright © 2009 Toshimitsu Takahashi Released under the MIT license. http://www.opensource.org/licenses/mit-license.php \r\n"
			"jQuery.splitter.js - animated splitter plugin version 1.0 (2010/01/02) Dual licensed under the MIT and GPL licenses: http://www.opensource.org/licenses/mit-license.php http://www.gnu.org/licenses/gpl.html");

		char buffer[MAX_PATH];
		unsigned long size = MAX_PATH;
		if (GetComputerName(buffer, &size))
			SetDlgItemText(IDC_MAC_NAME_BOX, buffer);

		COleDateTime dtSupportExp;
		dtSupportExp = GetSupportExpirationDate();
		if(dtSupportExp.m_status == COleDateTime::invalid) {
			m_strSupportExpires.SetWindowText("");
		}
		else {
			CString strSupportExpires;
			if(dtSupportExp == COleDateTime(1899,12,30,0,0,0)) {
				strSupportExpires = "No Support Expiration Date";
			}
			else {
				strSupportExpires = "Tech Support Expires:  " + FormatDateTimeForInterface(dtSupportExp, NULL, dtoDate);
			}
			m_strSupportExpires.SetWindowText(strSupportExpires);
			if(!IsSupportPaidFor()) {
				m_bSupportExp = TRUE;
				strSupportExpires.Replace("Expires:","Expired:");
				m_strSupportExpires.SetWindowText(strSupportExpires);
			}
		}

		// Make sure the dialog fits in the window space
		{

		}

		// Set the focus to the ok button so that the edit box doesn't have focus (because it looks stupid when it does, since it's not an editable field)
		GetDlgItem(IDOK)->SetFocus();
		// (s.dhole 2013-07-01 18:32) - PLID 57404

		RefreshClockDisplay();
	}
	NxCatchAll("Error in CAboutDlg::OnInitDialog");

	// Return FALSE because we set the focus, we don't want to framework to set it to something else
	return FALSE;
}

bool CAboutDlg::SetDlgItemDllFileDate(int nID, const CString &strFileName)
{
	CFileFind ff;
	CString strFilePathName;
	GetDllFilePathName(strFileName, strFilePathName);
	return SetDlgItemFileDate(nID, strFilePathName);
}

// (a.walling 2008-09-18 17:01) - PLID 28040
/*
bool CAboutDlg::SetDlgItemDatabaseDate(int nID, const CString &strFilePathName)
{
	return SetDlgItemVersion(nID, ::GetFileVersion(strFilePathName));
}
*/

bool CAboutDlg::SetDlgItemVersion(int nID, long nVer)
{
	CString strText;
	strText.Format("%02li-%02li-%04li.%01li", GetVerMonth(nVer), GetVerDay(nVer), GetVerYear(nVer), GetVerExtra(nVer));
	SetDlgItemText(nID, strText);
	return true;
}

bool CAboutDlg::SetDlgItemFileDate(int nID, const CString &strFilePathName)
{
	CFileFind ff;

	// Find the file
	if (!strFilePathName.IsEmpty() && ff.FindFile(strFilePathName, 0)) {
		// Get the file modified time
		ff.FindNextFile();
		CTime tmpTime;
		ff.GetLastWriteTime(tmpTime);
		
		// Put it on the dialog
		CString strText;
		strText.Format("%02li-%02li-%04li", tmpTime.GetMonth(), tmpTime.GetDay(), tmpTime.GetYear());
		SetDlgItemText(nID, strText);
		return true;
	}
	
	// If we made it to here we failed
	return false;
}



CString CAboutDlg::GetFileVersion(CString path)
{
	// (b.savon 2015-03-13 07:53) - PLID 64638 - Forward to method moved to library
	return GetDLLFileVersion(path);
}

#define APPEND(title, nID) GetDlgItemText(nID, str); msg += "\r\n" + (CString)title + str;
void CAboutDlg::OnEmail() 
{
	CString str, msg;
	
	APPEND("Version: ", IDC_PRODUCT_VERSION_TEXT);
	APPEND("User: ", IDC_USER_NAME_BOX);
	APPEND("Location: ", IDC_LOC_NAME_BOX);
	APPEND("Machine: ", IDC_MAC_NAME_BOX);
	APPEND("Dock Path: ", IDC_DOCK_PATH_LABEL);
	APPEND("NxServer: ", IDC_NXSERVER);
	APPEND("Local Path: ", IDC_LOCAL_PATH_LABEL);
	APPEND("Shared Path: ", IDC_SHARED_PATH_LABEL);
	// (a.walling 2012-09-04 17:32) - PLID 52449 - Include session path
	APPEND("Session Path: ", IDC_SESSION_PATH_LABEL);
	APPEND("", IDC_OS_VERSION);
	APPEND("\r\nDependencies:\r\n", IDC_FILELIST);

	// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
	SendEmail(this, "allsupport@nextech.com", "About Practice", msg);
}

void CAboutDlg::OnDependencies()
{
	CDependenciesDlg dlg(this);
	dlg.DoModal();
}

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_SUPPORT_EXPIRES:
			if(m_bSupportExp)
				//pDC->SetBkColor(0x9CC294);
				pDC->SetTextColor(RGB(192,0,0));
			return hbr;
		break;

		case IDC_USER_NAME_BOX:
		case IDC_LOC_NAME_BOX:
		case IDC_MAC_NAME_BOX:
		case IDC_DOCK_PATH_LABEL:
		case IDC_LOCAL_PATH_LABEL:
		case IDC_MAILING_ADDRESS_LABEL:
		case IDC_NXSERVER:
		case IDC_NEXT_BACKUP_LABEL:
		case IDC_SHARED_PATH_LABEL:
		case IDC_SESSION_PATH_LABEL: // (a.walling 2012-09-04 17:32) - PLID 52449 - Include session path
		// (s.dhole 2013-07-01 18:32) - PLID 57404 INCLIUDE SYTEM AND SERVER CLOCK
		case IDC_SYSTEM_CLOCK:
		case IDC_SERVER_CLOCK:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;

		default:
		break;
	}

	return hbr;
}


void CAboutDlg::OnBtnBackupManager() 
{
	//
	// (c.haag 2006-06-02 16:54) - PLID 20851 - Launch NxServerConfig instead of opening the
	// backup dialog. The CBackupDlg object has been removed from the project.
	//
	//CBackupDlg dlg;
	//dlg.DoModal();
	//

	// (c.haag 2006-07-21 15:35) - PLID 21550 - Pass in the subkey
	CString strRegKey = GetSubRegistryKey();
	if(!strRegKey.IsEmpty()) strRegKey = "r:" + strRegKey;
	// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
	// (j.jones 2010-09-02 15:44) - PLID 40388 - fixed crash caused by passing in "this" as a parent
	// (j.armen 2011-10-24 16:00) - PLID 46132 - Use the practice.exe path for locating nxserverconfig.exe
	HINSTANCE hInst;
	if ((int)(hInst=ShellExecute((HWND)GetDesktopWindow(), NULL, FileUtils::GetFilePath(GetPracPath(PracPath::PracticeEXEPath)) ^ "nxserverconfig.exe", strRegKey, NULL, SW_SHOW)) <= 32) {
		CString strError;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, strError.GetBuffer(MAX_PATH), MAX_PATH, NULL);
		strError.ReleaseBuffer();
		MsgBox("The configuration program could not be opened. %s", strError);
	}
}

void CAboutDlg::OnBtnLicense() 
{
	CLicenseInfoDlg dlg(this);
	dlg.DoModal();
}

void CAboutDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// (a.walling 2008-05-27 12:46) - PLID 29673 - Load our icon if necessary
	if (!m_pIcon) {
		m_pIcon = NxGdi::BitmapFromICOResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME), 256, TRUE);
	}

	// (d.thompson 2010-03-05) - PLID 37656 - Load the image
	if(!m_pBetterCareIcon) {
		m_pBetterCareIcon = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_BETTER_CARE), 216);
	}

	ASSERT(m_pIcon);

	// (a.walling 2008-05-27 12:46) - PLID 29673 - Then draw it.
	if (m_pIcon) {
		Gdiplus::Graphics g(dc.GetSafeHdc());
		g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
		g.DrawImage(m_pIcon, 20, 5, 64, 64);
	}

	// (d.thompson 2010-03-05) - PLID 37656 - Draw better care logo
	if(m_pBetterCareIcon) {
		Gdiplus::Graphics g(dc.GetSafeHdc());
		g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBilinear);
		g.DrawImage(m_pBetterCareIcon, 400, 23, 216, 24);
	}
	
	// Do not call CNxDialog::OnPaint() for painting messages
}

void CAboutDlg::OnDestroy() 
{
	try {
		// (s.dhole 2013-07-01 18:32) - PLID 57404 stop clock
		KillTimer(m_nTimerRefresh);
		// (a.walling 2008-05-27 12:46) - PLID 29673 - Free resources
		delete m_pIcon;

		// (d.thompson 2010-03-05) - PLID 37656 - Cleanup!
		if(m_pBetterCareIcon) {
			m_pBetterCareIcon = NULL;
			delete m_pBetterCareIcon;
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

namespace {
void LaunchPathOrDie(const CString& strPath)
{
	int ret = (int)::ShellExecute(NULL, NULL, strPath, NULL, NULL, SW_SHOWNORMAL);
	if (ret > 32) return;

	ThrowNxException("Error %li launching path `%s`", ret, strPath);
}
}

// (a.walling 2012-09-04 17:32) - PLID 52449 - Handle path clicks
void CAboutDlg::OnClickLocalPath()
{
	try {
		LaunchPathOrDie(GetPracPath(PracPath::PracticePath));
	} NxCatchAll(__FUNCTION__);
}

void CAboutDlg::OnClickSharedPath()
{
	try {
		LaunchPathOrDie(GetSharedPath());
	} NxCatchAll(__FUNCTION__);
}

void CAboutDlg::OnClickSessionPath()
{
	try {
		LaunchPathOrDie(GetPracPath(PracPath::SessionPath));
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-09-04 17:32) - PLID 52449 - Handle path link cursor
BOOL CAboutDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (HWND hwnd = pWnd->GetSafeHwnd()) {
		if (
			   hwnd == GetDlgItem(IDC_LOCAL_PATH_STATIC)->GetSafeHwnd()
			|| hwnd == GetDlgItem(IDC_SHARED_PATH_STATIC)->GetSafeHwnd()
			|| hwnd == GetDlgItem(IDC_SESSION_PATH_STATIC)->GetSafeHwnd()
		)
		{
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return __super::OnSetCursor(pWnd, nHitTest, message);
}

// (s.dhole 2013-07-01 18:32) - PLID 57404 Add millisecond time offset
// millisecond
SYSTEMTIME CAboutDlg::AddTimeOffset(SYSTEMTIME s,double timeOffset)
{
	
	FILETIME ft;
	ULARGE_INTEGER tmp;
	SystemTimeToFileTime(&s, &ft);
	
	tmp.LowPart = ft.dwLowDateTime;
	tmp.HighPart = ft.dwHighDateTime;

   const double c_dMilliSecondsInterval =  1.e-4;
   const double c_dNumberOfMilliSecondsInterval  = 
                    timeOffset / c_dMilliSecondsInterval;
	tmp.QuadPart += (__int64)c_dNumberOfMilliSecondsInterval;
	ft.dwLowDateTime = tmp.LowPart;
	ft.dwHighDateTime = tmp.HighPart;
	FileTimeToSystemTime(&ft, &s);
	return s;
}

// (s.dhole 2013-07-01 18:32) - PLID 57404
void CAboutDlg::OnTimer(UINT nIDEvent) 
{
	try{
	KillTimer(m_nTimerRefresh);
	RefreshClockDisplay();
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-07-01 18:32) - PLID 57404 convert utc time to local time with milliseconds
SYSTEMTIME CAboutDlg::GetLocalTime(SYSTEMTIME s)
{
	FILETIME ftIn;
	FILETIME ft;
	SystemTimeToFileTime(&s, &ftIn);
	FileTimeToLocalFileTime(&ftIn ,&ft);
	FileTimeToSystemTime(&ft, &s);
	return s;
}

// (s.dhole 2013-07-31 16:14) - PLID  57404 Refresh clock display
SYSTEMTIME CAboutDlg::GetLocalMachineTime()
{
	SYSTEMTIME st;
	// Get system utc time with milliseconds
    GetSystemTime(&st);
	// Get system local time with milliseconds
	return GetLocalTime(st);
}


// (s.dhole 2013-07-01 18:32) - PLID 57404 Check sql server on local host
BOOL CAboutDlg::IsSQLServerLocalHost()
{	BOOL bResult = FALSE;
CString strLocalHost = SyslogClient::GetHostName();
	if (!strLocalHost.IsEmpty()) {
		ADODB::_RecordsetPtr prs = CreateRecordsetStd("SELECT SERVERPROPERTY('MachineName') as hostname");
		if (!prs->eof) {
			if (strLocalHost.CollateNoCase(AdoFldString (prs,"hostname",""))==0)  
			{
				bResult= TRUE;
			}
		}
	}
	return bResult;
}


// (s.dhole 2013-07-01 18:32) - PLID 57404 Refresh clock display
void CAboutDlg::RefreshClockDisplay()
{
// if sql server is local host then we will skip time check on Sql server
	
	//Step 1  Check Server Date
	// Get Server date
	// We execute sql check before call local time to eliminate sql delay
	if(m_dtServerDate.GetStatus()== COleDateTime::invalid){
		ADODB::_RecordsetPtr prs = CreateRecordsetStd("Select GETDATE() AS ServerDate");
		if (!prs->eof) {
			m_dtServerDate = AdoFldDateTime  (prs,"ServerDate" ); 
		}
	}

	COleDateTime dtLocalDateTime,dtServerDateTime;
	
// Step 2 Check local matchine time
	SYSTEMTIME stLocalMachine=GetLocalMachineTime();
	SYSTEMTIME stServer ;
	dtLocalDateTime.SetDateTime(stLocalMachine.wYear,stLocalMachine.wMonth,stLocalMachine.wDay ,stLocalMachine.wHour,stLocalMachine.wMinute,stLocalMachine.wSecond );
	
	BOOL bDatabaseTimeSync=FALSE; // Flag to ceck DB time again
	//refresh only if there is increment in second
	if (m_dtLastLocalDateTime.GetStatus()!= COleDateTime::invalid){
		//check last sync time diffrence , it should be less then second
		COleDateTimeSpan spanElapsed = dtLocalDateTime -m_dtLastLocalDateTime;
		double nTotalDiffrence =  spanElapsed.GetTotalSeconds();
		
		// if diffrence is more than second.. then machein time has been changed now we need to compare again with server
		//refresh only if there is increment in second
		if (nTotalDiffrence>1 || nTotalDiffrence<-1){
			bDatabaseTimeSync = TRUE;
			// we need to check db time since there is change in local clock
		}
	}
	m_dtLastLocalDateTime = dtLocalDateTime ;


	//Check DB only if following condition is true
	//1) if increament value is zero or more than 1800000 (30 min) after last db time check
	//2) if time diffrence between last local time and current local time is more than 1 second 
	stServer =GetLocalMachineTime();
	if ((m_nLastServerTimeSeconds<=0) || (m_nLastServerTimeSeconds>1800000) || bDatabaseTimeSync ){
		m_nLastServerTimeSeconds =0;
		CString strDateSQL; 
		//check time diffrence at Sql server
		// always user server date, local day may be out of sync
		SYSTEMTIME stEnd ; // To check offset, due to round trip on sql server
		// Get system utc time along with milliseconds
		
		strDateSQL=FormatString("SELECT CAST(DATEDIFF(MS, cast ('%li-%02li-%02li %02li:%02li:%02li:%03li' as datetime)  ,GETDATE()) AS bigint)  as diffInMs",m_dtServerDate.GetYear(),m_dtServerDate.GetMonth() ,m_dtServerDate.GetDay(), stServer.wHour,stServer.wMinute,stServer.wSecond, stServer.wMilliseconds  ) ;
		
		ADODB::_RecordsetPtr prs = CreateRecordsetStd(strDateSQL);
		if (!prs->eof) {
			m_nServerTimeOffset = AdoFldBigInt (prs,"diffInMs"); 
		}
		
		// Get system utc time with milliseconds
		GetSystemTime(&stEnd);// After Sql call
		// calculate offset due to sql server round trip 
		int nDiff =(stEnd.wMilliseconds  -  stLocalMachine.wMilliseconds)/2;
		m_nServerTimeOffset = m_nServerTimeOffset - nDiff;

	}
	// Increament refresh time counter
	m_nLastServerTimeSeconds += 100;
	// now calculate server time
	
	double dbDiff = static_cast<double>(m_nServerTimeOffset);  
	stServer = AddTimeOffset(stServer,dbDiff);//1000.0 =1 sec
	dtServerDateTime.SetDateTime(stServer.wYear,stServer.wMonth,stServer.wDay ,stServer.wHour,stServer.wMinute,stServer.wSecond );

	
	WindowUtils::NxSetWindowText(GetDlgItem(IDC_SERVER_CLOCK), FormatDateTimeForInterface(dtServerDateTime,0,dtoTime));
	// Set last server sec value
	WindowUtils::NxSetWindowText(GetDlgItem(IDC_SYSTEM_CLOCK), FormatDateTimeForInterface(dtLocalDateTime,0,dtoTime));
	//refresh every 100 milliseconds, We should have server and local time diffrence not more than 1 Sec 
	SetTimer(m_nTimerRefresh, 100, NULL);
}