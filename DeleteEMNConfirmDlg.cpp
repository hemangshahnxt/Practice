// DeleteEMNConfirmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeleteEMNConfirmDlg.h"

// (j.jones 2009-10-01 11:22) - PLID 30479 - created

// CDeleteEMNConfirmDlg dialog

CDeleteEMNConfirmDlg::CDeleteEMNConfirmDlg(EMNConfirmDeletionType eType, CString strDescription, CWnd* pParent /*=NULL*/)
	: CNxDialog(CDeleteEMNConfirmDlg::IDD, pParent)
{
	m_eType = eType;
	m_strDescription = strDescription;
}

void CDeleteEMNConfirmDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_KEEP_EMN, m_btnKeepEMN);
	DDX_Control(pDX, IDC_BTN_DELETE_EMN, m_btnDeleteEMN);
	DDX_Control(pDX, IDC_DELETE_EMN_TOP_LABEL, m_nxstaticTopText);
	DDX_Control(pDX, IDC_DELETE_EMN_DESC, m_nxstaticDescription);
	DDX_Control(pDX, IDC_DELETE_EMN_BOTTOM_LABEL, m_nxstaticBottomText);
}


BEGIN_MESSAGE_MAP(CDeleteEMNConfirmDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_KEEP_EMN, &CDeleteEMNConfirmDlg::OnBtnKeepEmn)
	ON_BN_CLICKED(IDC_BTN_DELETE_EMN, &CDeleteEMNConfirmDlg::OnBtnDeleteEmn)
END_MESSAGE_MAP()

// CDeleteEMNConfirmDlg message handlers

BOOL CDeleteEMNConfirmDlg::OnInitDialog()
{
	try {
				
		CNxDialog::OnInitDialog();

		m_btnKeepEMN.AutoSet(NXB_OK);
		m_btnDeleteEMN.AutoSet(NXB_DELETE);

		CString strTopText = "You are about to delete the following EMN, which is an UNRECOVERABLE operation.";
		CString strDescriptionText = "EMN to delete: ";
		CString strBottomText = "Are you absolutely sure you wish to permanently delete this EMN?";
		CString strKeepBtnText = "Keep This EMN";
		CString strDeleteBtnText = "Delete This EMN";

		if(m_eType == ecdtEMR) {
			strTopText.Replace("EMN", "EMR");
			strDescriptionText.Replace("EMN", "EMR");
			strKeepBtnText.Replace("EMN", "EMR");
			strDeleteBtnText.Replace("EMN", "EMR");

			strBottomText = "Are you absolutely sure you wish to permanently delete this EMR and all EMNs inside it?";
		}
		else if(m_eType == ecdtCustomRecord) {
			strTopText.Replace("EMN", "Custom Record");
			strDescriptionText.Replace("EMN", "Custom Record");
			strKeepBtnText.Replace("EMN", "Custom Record");
			strDeleteBtnText.Replace("EMN", "Custom Record");
			strBottomText.Replace("EMN", "Custom Record");
		}

		strDescriptionText += m_strDescription;

		m_btnKeepEMN.SetWindowText(strKeepBtnText);
		m_btnDeleteEMN.SetWindowText(strDeleteBtnText);
		m_nxstaticTopText.SetWindowText(strTopText);
		m_nxstaticDescription.SetWindowText(strDescriptionText);
		m_nxstaticBottomText.SetWindowText(strBottomText);
		
	}NxCatchAll("Error in CDeleteEMNConfirmDlg::OnInitDialog")
	
	return FALSE;
}

void CDeleteEMNConfirmDlg::OnBtnKeepEmn()
{
	try {

		//close the dialog with a specific message to keep the record
		EndDialog(DELETE_EMN_RETURN_KEEP);

	}NxCatchAll("Error in CDeleteEMNConfirmDlg::OnBtnKeepEmn");
}

void CDeleteEMNConfirmDlg::OnBtnDeleteEmn()
{
	try {

		//prompt one more time
		CString strWarn = "Are you absolutely sure you wish to remove this EMN?  This action is not recoverable!";
		if(m_eType == ecdtEMR) {
			strWarn.Replace("EMN", "EMR");
		}
		else if(m_eType == ecdtCustomRecord) {
			strWarn.Replace("EMN", "Custom Record");
		}

		if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			//if they changed their minds, keep the record
			EndDialog(DELETE_EMN_RETURN_KEEP);
			return;
		}

		//close the dialog with a specific message to delete the record
		EndDialog(DELETE_EMN_RETURN_DELETE);

	}NxCatchAll("Error in CDeleteEMNConfirmDlg::OnBtnDeleteEmn");
}
