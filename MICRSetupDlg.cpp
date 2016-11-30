// MICRSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MICRSetupDlg.h"

// (j.jones 2007-05-09 16:55) - PLID 25550 - created

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMICRSetupDlg dialog


CMICRSetupDlg::CMICRSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMICRSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMICRSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMICRSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMICRSetupDlg)
	DDX_Control(pDX, IDC_CHECK_USE_OLD_MICR_FORM, m_btnUseLegacyForm);
	DDX_Control(pDX, IDC_EDIT_MICR_PROV_CODE_QUAL, m_nxeditEditMicrProvCodeQual);
	DDX_Control(pDX, IDC_EDIT_MICR_REF_ORDER_QUAL, m_nxeditEditMicrRefOrderQual);
	DDX_Control(pDX, IDC_EDIT_MICR_BOX44_QUAL, m_nxeditEditMicrBox44Qual);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMICRSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMICRSetupDlg)
	ON_BN_CLICKED(IDC_CHECK_USE_OLD_MICR_FORM, OnCheckUseOldMicrForm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMICRSetupDlg message handlers

void CMICRSetupDlg::OnCheckUseOldMicrForm() 
{
	try {
	
		BOOL bChecked = IsDlgButtonChecked(IDC_CHECK_USE_OLD_MICR_FORM);

		GetDlgItem(IDC_EDIT_MICR_PROV_CODE_QUAL)->EnableWindow(!bChecked);
		GetDlgItem(IDC_EDIT_MICR_REF_ORDER_QUAL)->EnableWindow(!bChecked);
		GetDlgItem(IDC_EDIT_MICR_BOX44_QUAL)->EnableWindow(!bChecked);

	}NxCatchAll("Error in CMICRSetupDlg::OnCheckUseOldMicrForm");
}

BOOL CMICRSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
	
		// (z.manning, 04/30/2008) - PLID 29864 - Set button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);	

		CheckDlgButton(IDC_CHECK_USE_OLD_MICR_FORM, GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) != 1);
		OnCheckUseOldMicrForm();

		SetDlgItemText(IDC_EDIT_MICR_PROV_CODE_QUAL, GetRemotePropertyText("MICR2007_Provider_Code_Qual", "1B", 0, "<None>", true));
		SetDlgItemText(IDC_EDIT_MICR_REF_ORDER_QUAL, GetRemotePropertyText("MICR2007_Ref_Order_Qual", "0B", 0, "<None>", true));
		SetDlgItemText(IDC_EDIT_MICR_BOX44_QUAL, GetRemotePropertyText("MICR2007_Box44_Qual", "0B", 0, "<None>", true));

	}NxCatchAll("Error in CMICRSetupDlg::OnInitDialog() ");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMICRSetupDlg::OnOK() 
{
	try {

		SetRemotePropertyInt("Use2007MICR", IsDlgButtonChecked(IDC_CHECK_USE_OLD_MICR_FORM) ? 0 : 1, 0, "<None>");

		CString str;
		
		GetDlgItemText(IDC_EDIT_MICR_PROV_CODE_QUAL, str);
		SetRemotePropertyText("MICR2007_Provider_Code_Qual", str, 0, "<None>");

		GetDlgItemText(IDC_EDIT_MICR_REF_ORDER_QUAL, str);
		SetRemotePropertyText("MICR2007_Ref_Order_Qual", str, 0, "<None>");

		GetDlgItemText(IDC_EDIT_MICR_BOX44_QUAL, str);
		SetRemotePropertyText("MICR2007_Box44_Qual", str, 0, "<None>");

		CDialog::OnOK();

	}NxCatchAll("Error in CMICRSetupDlg::OnOK() ");
}
