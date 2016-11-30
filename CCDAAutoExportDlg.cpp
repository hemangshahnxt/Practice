// CCDAAutoExportDlg.cpp : implementation file
//

// (a.walling 2015-10-28 09:49) - PLID 67425 - Configuration for CCDA export locations

#include "stdafx.h"
#include "Practice.h"
#include "AdministratorRc.h"
#include "CCDAAutoExportDlg.h"
#include "NxAPI.h"
#include "NxAPIUtils.h"

// CCCDAAutoExportDlg dialog

IMPLEMENT_DYNAMIC(CCCDAAutoExportDlg, CNxDialog)

CCCDAAutoExportDlg::CCCDAAutoExportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_CCDA_AUTO_EXPORT_DLG, pParent)
{

}

CCCDAAutoExportDlg::~CCCDAAutoExportDlg()
{
}

void CCCDAAutoExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCCDAAutoExportDlg, CNxDialog)
	ON_BN_CLICKED(IDC_VERIFY, &CCCDAAutoExportDlg::OnBnClickedVerify)
	ON_BN_CLICKED(IDC_ADD, &CCCDAAutoExportDlg::OnBnClickedAdd)
END_MESSAGE_MAP()


// CCCDAAutoExportDlg message handlers


BOOL CCCDAAutoExportDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		AutoSet(IDOK, NXB_OK);
		AutoSet(IDC_ADD, NXB_NEW);

		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NEWPATH);
		Edit_SetCueBannerTextFocused(pEdit->GetSafeHwnd(), L"Enter a new path to add...", FALSE);
		::SHAutoComplete(pEdit->GetSafeHwnd(), SHACF_FILESYS_DIRS);

		// (v.maida 2016-06-14 10:35) - NX-100833 - Only allow the NexTech Technical Support user to configure automatic CCDA exports when on RemoteApp.
		if (g_pLicense->GetAzureRemoteApp() && GetCurrentUserID() != BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			EnableControls(FALSE);
		}
		else {
			EnableControls(TRUE);
		}

		Reload();

	} NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CCCDAAutoExportDlg::Reload()
{
	try {
		CString list = GetRemotePropertyMemo("CCDAAutoExportPaths", "", 0, "<None>", true);
		SetDlgItemText(IDC_EDIT_PATHS, list);
	} NxCatchAll(__FUNCTION__);
}

void CCCDAAutoExportDlg::Save()
{
	try {
		CString list;
		GetDlgItemText(IDC_EDIT_PATHS, list);

		list.Trim("\r\n");

		SetRemotePropertyMemo("CCDAAutoExportPaths", list, 0, "<None>");
	} NxCatchAll(__FUNCTION__);
}

void CCCDAAutoExportDlg::OnOK()
{
	try {

		if (GetFocus() == GetDlgItem(IDC_EDIT_NEWPATH)) {
			CString str;
			GetDlgItemText(IDC_EDIT_NEWPATH, str);

			if (!str.IsEmpty()) {
				SendMessage(WM_COMMAND, IDC_ADD);
				return;
			}
		}

		Save();

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2015-10-28 09:49) - PLID 67488 - Verify CCDA export paths in the API
void CCCDAAutoExportDlg::OnBnClickedVerify()
{
	try {
		CString list;
		GetDlgItemText(IDC_EDIT_PATHS, list);

		CString str = (const char*)GetAPI()->VerifyCCDAExportPaths((const char*)list);

		if (!str.IsEmpty()) {
			str += "\r\n\r\nPaths are relative to the server, and will be accessed using the account under which the NxSoapServerCS runs. If using any network shares, please ensure the NxSoapServerCS account has access to them.";
			MessageBox(str, "Verification of export paths", MB_ICONEXCLAMATION);
		}
		else {
			MessageBox("All paths verified successfully!", "Verification of export paths");
		}
	} NxCatchAll(__FUNCTION__);
}

void CCCDAAutoExportDlg::OnBnClickedAdd()
{
	try {
		CString str;
		GetDlgItemText(IDC_EDIT_NEWPATH, str);

		if (str.IsEmpty()) {
			return;
		}

		CString list;
		GetDlgItemText(IDC_EDIT_PATHS, list);
		
		if (!list.IsEmpty()) {
			list += "\r\n";
		}

		list += str;

		while (list.Replace("\r\n\r\n", "\r\n")) {}

		SetDlgItemText(IDC_EDIT_PATHS, list);
		SetDlgItemText(IDC_EDIT_NEWPATH, "");
	} NxCatchAll(__FUNCTION__);
}

// (v.maida 2016-06-14 10:35) - NX-100833 - Created a function for enabling/disabling all controls.
void CCCDAAutoExportDlg::EnableControls(BOOL bEnable)
{
	// if FALSE then we're disabling buttons and enabling the read only status of edit boxes, and vice versa.
	( (CEdit *)GetDlgItem(IDC_EDIT_NEWPATH))->SetReadOnly(!bEnable);
	( (CEdit *)GetDlgItem(IDC_EDIT_PATHS))->SetReadOnly(!bEnable);
	GetDlgItem(IDC_VERIFY)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADD)->EnableWindow(bEnable);
	GetDlgItem(IDOK)->EnableWindow(bEnable);
}

// since these will be relative to the server, a browse button is just inviting misuse.

//void CCCDAAutoExportDlg::OnBnClickedBrowse()
//{
//	try {
//		CString strResult;
//		if (afxShellManager->BrowseForFolder(strResult, this, "", NULL, BIF_USENEWUI | BIF_SHAREABLE)) {
//			SetDlgItemText(IDC_EDIT_NEWPATH, strResult);
//		}
//	} NxCatchAll(__FUNCTION__);
//}
