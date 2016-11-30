// InvInternalSignEquipmentDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvInternalSignEquipmentDlg.h"


// CInvInternalSignEquipmentDlg dialog

IMPLEMENT_DYNAMIC(CInvInternalSignEquipmentDlg, CNxDialog)

CInvInternalSignEquipmentDlg::CInvInternalSignEquipmentDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvInternalSignEquipmentDlg::IDD, pParent)
{
	m_bShowSkip = false;
	m_nUserID = -1;
	m_bSkipedSign = true;
}

CInvInternalSignEquipmentDlg::~CInvInternalSignEquipmentDlg()
{
}

void CInvInternalSignEquipmentDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SKIP_SIGN, m_SkipSign);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CInvInternalSignEquipmentDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvInternalSignEquipmentDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvInternalSignEquipmentDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SKIP_SIGN, &CInvInternalSignEquipmentDlg::OnBnClickedSkipSign)
END_MESSAGE_MAP()

int CInvInternalSignEquipmentDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	
	try
	{
		//Set Icons on Buttons
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.fouts 2012-05-17 17:41) - PLID 50300 - Load the username of the person checking this out
		if(m_bShowSkip)
		{
			m_SkipSign.ShowWindow(SW_SHOW);
			m_SkipSign.EnableWindow(TRUE);
		}

		ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT PersonID FROM UsersT WHERE UserName = {String}", m_strCheckoutUsername);

		if(!prs->eof)
		{
			m_nUserID = AdoFldLong(prs, "PersonID");
			GetDlgItem(IDC_USERNAME_SIGN)->SetWindowText(m_strCheckoutUsername);
		}
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}


// CInvInternalSignEquipmentDlg message handlers

void CInvInternalSignEquipmentDlg::OnBnClickedOk()
{
	try
	{
		// (j.fouts 2012-05-17 17:40) - PLID 50300 - Check that passwords match before saving
		if(m_SkipSign.GetCheck())
		{
			CNxDialog::OnOK();
		} else {

			CString strEnteredPass;
			GetDlgItem(IDC_PASSWORD_SIGN)->GetWindowText(strEnteredPass);

			if(CompareSpecificUserPassword(m_nUserID, strEnteredPass))
			{
				m_bSkipedSign = false;
				CNxDialog::OnOK();
			}
			else
			{
				MessageBox("The entered password is incorrect. Please try again.");
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CInvInternalSignEquipmentDlg::OnBnClickedCancel()
{
	CNxDialog::OnCancel();
}

void CInvInternalSignEquipmentDlg::OnBnClickedSkipSign()
{
	if(m_SkipSign.GetCheck())
	{
		GetDlgItem(IDC_PASSWORD_SIGN)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_PASSWORD_SIGN)->EnableWindow(TRUE);
	}
}
