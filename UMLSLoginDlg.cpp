// UMLSLoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "UMLSLoginDlg.h"
#include "PracticeRC.h"
#include "NxUTS.h"

// (j.camacho 2013-10-07 14:52) - PLID 58678 - UMLS Setup dialog

// CUMLSLoginDlg dialog

IMPLEMENT_DYNAMIC(CUMLSLoginDlg, CNxDialog)

CUMLSLoginDlg::CUMLSLoginDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUMLSLoginDlg::IDD, pParent)
{

}

CUMLSLoginDlg::~CUMLSLoginDlg()
{
}

void CUMLSLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDOK, m_btnOK);
	DDX_Control(pDX,IDCANCEL, m_btnCancel);
	DDX_Control(pDX,IDC_UMLSLOGIN, m_nxeditLoginID);
	DDX_Control(pDX,IDC_UMLSPASSWORD, m_nxeditPassword);
	DDX_Control(pDX,IDC_NXCOLOR_UMLSSETUP, m_nxcolorBackground);
	DDX_Control(pDX,IDC_MAKE_ACCOUNT,m_nxlabelMakeAccount);

}

// (j.camacho 2013-10-24 16:57) - PLID 58678 - label message capture for making new account hyperlink
BEGIN_MESSAGE_MAP(CUMLSLoginDlg, CNxDialog)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
END_MESSAGE_MAP()


// CUMLSLoginDlg message handlers

BOOL CUMLSLoginDlg::OnInitDialog()
{
	try
	{
		
		CNxDialog::OnInitDialog();
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_nxcolorBackground.SetColor(GetNxColor(GNC_ADMIN, 0));

		m_nxeditLoginID.SetLimitText(20); // max character limit on umls site for usernames
		m_nxeditPassword.SetLimitText(30); //TODO: Should i limit the password field if there is no limit on UMLS site?
		
	
		m_nxlabelMakeAccount.SetColor(GetNxColor(GNC_INVENTORY, 0));
		m_nxlabelMakeAccount.SetType(dtsHyperlink);
		m_nxlabelMakeAccount.SetSingleLine(true);
		m_nxlabelMakeAccount.SetText("Do you need to make an account?");

		LoadFields(); 

 		GetDlgItemText(IDC_UMLSLOGIN,m_LoginID) ;
		GetDlgItemText(IDC_UMLSPASSWORD,m_Password) ;
		
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CUMLSLoginDlg::OnOK()
{
	try{
	
		if(SaveFields())
		{
			CNxDialog::OnOK();
		}
		//if there is a failure it will not save and instead prompt user to what is wrong.

	}NxCatchAll(__FUNCTION__);
}

void CUMLSLoginDlg::LoadFields()
{
	SetDlgItemText(IDC_UMLSLOGIN, GetRemotePropertyText("UMLSLogin"));
	SetDlgItemText(IDC_UMLSPASSWORD, GetRemotePropertyText("UMLSPassword"));
}

BOOL CUMLSLoginDlg::SaveFields()
{
	CString strUMLSLogin, strUMLSPassword;
	GetDlgItemText(IDC_UMLSLOGIN, strUMLSLogin);
	GetDlgItemText(IDC_UMLSPASSWORD, strUMLSPassword);

	// (j.jones 2015-03-30 15:19) - PLID 61540 - validate the login by trying to get a ticket now,
	// if the login is invalid this will cleanly fail and not warn, so we need to warn
	{
		using namespace Nx::UTS;
		if (!Security::ValidateLogin(strUMLSLogin, strUMLSPassword)) {
			MessageBox("Failed to login to UMLS.\n\n"
				"Please confirm your UMLS username and password are correct.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}
	}
	
	if(strUMLSLogin !=m_LoginID || strUMLSPassword !=m_Password)
	{
		//successful saving
		if(strUMLSPassword != "" && strUMLSLogin != "")
		{
			//save info
			SetRemotePropertyText("UMLSLogin", strUMLSLogin);
			SetRemotePropertyText("UMLSPassword", strUMLSPassword);
			return TRUE;
		}
		else 
		{
			//validation fail
			if(strUMLSPassword == "" && strUMLSLogin == "")
			{
				//Both are empty. SAVE THIS AND Return positive.
				//user has not entered or has deleted their login info.
				SetRemotePropertyText("UMLSLogin", strUMLSLogin);
				SetRemotePropertyText("UMLSPassword", strUMLSPassword);
				return TRUE;
			}
			else if(strUMLSPassword == "")
			{
				//password is empty
				AfxMessageBox("The password field is empty. Please enter a valid password.");
				return FALSE;
			}
			else
			{
				//login is empty
				AfxMessageBox("The login field is empty. Please enter a valid login.");
				return FALSE;
			}
			
			//failed saving
			return FALSE;
		}
	}
	else{
		//username and password didn't change but still a successful run
		return TRUE;
	}


}
BEGIN_EVENTSINK_MAP(CUMLSLoginDlg, CNxDialog)
END_EVENTSINK_MAP()




// (j.camacho 2013-10-24 16:55) - PLID 58678 - Opens link to umls site.
void CUMLSLoginDlg::OpenMakeNewAccount()
{
	
	try{
		CString strMakeAccount = GetRemotePropertyText("UMLSWebsite","https://uts.nlm.nih.gov/license.html");
		ShellExecute(NULL, "open", strMakeAccount, NULL,NULL, SW_SHOWNORMAL);
		
	}NxCatchAll(__FUNCTION__);

}



// (j.camacho 2013-10-24 16:55) - PLID 58678 - sets up make account Hyperlink
LRESULT CUMLSLoginDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MAKE_ACCOUNT:
			OpenMakeNewAccount();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.camacho 2013-10-24 16:55) - PLID 58678
BOOL CUMLSLoginDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	CRect rcAdd;
	GetDlgItem(IDC_MAKE_ACCOUNT)->GetWindowRect(rcAdd);
	ScreenToClient(&rcAdd);

	if (rcAdd.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}