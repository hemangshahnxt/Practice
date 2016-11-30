// LabConfirmDeleteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LabConfirmDeleteDlg.h"


// CLabConfirmDeleteDlg dialog
// (z.manning 2008-10-09 14:46) - PLID 31628 - Created
IMPLEMENT_DYNAMIC(CLabConfirmDeleteDlg, CNxDialog)

CLabConfirmDeleteDlg::CLabConfirmDeleteDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabConfirmDeleteDlg::IDD, pParent)
{

}

CLabConfirmDeleteDlg::~CLabConfirmDeleteDlg()
{
}

void CLabConfirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAB_DELETE_MESSAGE_TOP, m_nxstaticMessageTop);
	DDX_Control(pDX, IDC_LAB_DELETE_MESSAGE_BOTTOM, m_nxstaticMessageBottom);
	DDX_Control(pDX, IDCANCEL, m_btnKeepLab);
	DDX_Control(pDX, IDOK, m_btnDeleteLab);
	DDX_Control(pDX, IDC_LAB_DETAILS, m_nxeditLabDetails);
}


BEGIN_MESSAGE_MAP(CLabConfirmDeleteDlg, CNxDialog)
END_MESSAGE_MAP()


// CLabConfirmDeleteDlg message handlers

BOOL CLabConfirmDeleteDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnDeleteLab.AutoSet(NXB_DELETE);

		CString strMessageTop = "You are attempting to delete the following lab";
		if(!m_strPatientName.IsEmpty()) {
			strMessageTop += " for patient " + m_strPatientName;
		}
		strMessageTop += "...";
		m_nxstaticMessageTop.SetWindowText(strMessageTop);

		CString strMessageBottom = "Are you sure you want to delete this lab?  "
			"(The lab will be deleted immediately and this change cannot be undone.)";
		m_nxstaticMessageBottom.SetWindowText(strMessageBottom);

		m_nxeditLabDetails.SetWindowText(m_strLabInfo);

	}NxCatchAll("CLabConfirmDeleteDlg::OnInitDialog");

	return TRUE;
}

void CLabConfirmDeleteDlg::SetLabInfo(IN const CString strLabInfo)
{
	m_strLabInfo = strLabInfo;
}

void CLabConfirmDeleteDlg::SetPatientName(IN const CString strPatientName)
{
	m_strPatientName = strPatientName;
}