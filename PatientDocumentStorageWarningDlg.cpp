// PatientDocumentStorageWarningDlg.cpp : implementation file
// (d.thompson 2013-07-02) - PLID 13764 - This dialog is a warning dialog for the PersonDocumentStorage warning
//	exception.  This exception happens when you try to access the patient's document folder but are unable to
//	create it (generally due to permissions or such).
//

#include "stdafx.h"
#include "Practice.h"
#include "PatientDocumentStorageWarningDlg.h"


// CPatientDocumentStorageWarningDlg dialog

IMPLEMENT_DYNAMIC(CPatientDocumentStorageWarningDlg, CNxDialog)

CPatientDocumentStorageWarningDlg::CPatientDocumentStorageWarningDlg(CString strPath, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientDocumentStorageWarningDlg::IDD, pParent), 
	m_strPath(strPath)
{

}

CPatientDocumentStorageWarningDlg::~CPatientDocumentStorageWarningDlg()
{
}

void CPatientDocumentStorageWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WARNING_LINK, m_nxlLink);
}


BEGIN_MESSAGE_MAP(CPatientDocumentStorageWarningDlg, CNxDialog)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()


// CPatientDocumentStorageWarningDlg message handlers

BOOL CPatientDocumentStorageWarningDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		SetDlgItemText(IDC_WARNING1, "NexTech could not create the person document storage folder on your server.  "
			"This most commonly happens because the server is inaccessible, you do not have permissions to the shared path, "
			"or because your workstation is improperly configured.\r\n"
			"Please click the link below to attempt to open the path.  If you are given a username/password prompt or if the "
			"path is unavailable, please contact your IT department to correct your permissions.\r\n"
			"If you have additional questions, please contact NexTech Technical Support.");

		m_nxlLink.SetType(dtsHyperlink);
		m_nxlLink.SetText(m_strPath);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

LRESULT CPatientDocumentStorageWarningDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		switch((UINT)wParam) {
		case IDC_WARNING_LINK:
			OnWarningLink();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CPatientDocumentStorageWarningDlg::OnWarningLink()
{
	// (d.thompson 2013-07-03) - PLID 13764 - A bit of a conundrum here.  If you lack access to \\server\shared\ entirely, 
	//	then shell execute will refuse to open a window to anything inside of it.  It will instead do nothing and return
	//	an error code.  If the server doesn't exist (bad dock settings), the same will happen.  So we need to try a bunch 
	//	of different options here.

	//First, try the path directly.  Maybe just this one record is screwy.
	HINSTANCE hRet = ShellExecute(NULL, "open", m_strPath, "", "", SW_SHOW);
	if((int)hRet <= 32) {
		
		//We failed.  Our ideal scenario is for Windows to pop up a message saying there's something wrong.  So let's try to 
		//	launch the shared path itself instead of this specific patient.  Most likely that's where permissions are applied, 
		//	so it's more possible to pop up.
		if((int)hRet == SE_ERR_ACCESSDENIED) {
			hRet = ShellExecute(NULL, "open", GetSharedPath(), "", "", SW_SHOW);
			if((int)hRet > 32) {
				//Success!
				return;
			}
		}

		if((int)hRet == SE_ERR_ACCESSDENIED) {
			//Still denied.  We'll just have to alert them.  You can reach this situation by putting the shared path inside a higher level
			//	share and denying permission there.  For example:  share \\server\apps\ and make your shared path \\server\apps\PracStation
			AfxMessageBox("NexTech was unable to open the path because access was denied.  Please contact your IT department to resolve your permissions.");
		}
		else if((int)hRet == SE_ERR_PNF) {
			//The path wasn't found.  You've got bad dock settings.  You can reach this situation by setting your shared path
			//	to machine name that doesn't exist.
			AfxMessageBox("NexTech was unable to open the path because the path does not exist.  This is most likely because the server is off or your dock settings are incorrect.");
		}
		else {
			//Unknown error.  Just give out the code.
			AfxMessageBox(FormatString("NexTech was unable to open the path for an unknown reason.  Error code %li.", (int)hRet));
		}
	}
}

BOOL CPatientDocumentStorageWarningDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		GetDlgItem(IDC_WARNING_LINK)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	} catch(...) {
		//no error handling for OnSetCursor
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}
