// EmrTableInputMaskDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrTableInputMaskDlg.h"

// (z.manning 2009-01-13 14:31) - PLID 32719 - Created
// CEmrTableInputMaskDlg dialog

IMPLEMENT_DYNAMIC(CEmrTableInputMaskDlg, CNxDialog)

CEmrTableInputMaskDlg::CEmrTableInputMaskDlg(CWnd* pParent)
	: CNxDialog(CEmrTableInputMaskDlg::IDD, pParent)
{

}

CEmrTableInputMaskDlg::~CEmrTableInputMaskDlg()
{
}

void CEmrTableInputMaskDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INPUT_MASK, m_nxeditInputMask);
	DDX_Control(pDX, IDC_INPUT_MASK_TEST, m_editInputMaskTest);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_INPUT_MASK_HELP, m_nxeditHelp);
}


BEGIN_MESSAGE_MAP(CEmrTableInputMaskDlg, CNxDialog)
	ON_EN_CHANGE(IDC_INPUT_MASK, &CEmrTableInputMaskDlg::OnEnChangeInputMask)
	ON_BN_CLICKED(IDOK, &CEmrTableInputMaskDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CEmrTableInputMaskDlg message handlers

BOOL CEmrTableInputMaskDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (z.manning 2009-01-15 15:15) - PLID 32724 - Limit the text to 100 characters
		m_nxeditInputMask.SetLimitText(100);
		m_nxeditInputMask.SetWindowText(m_strInputMask);

		m_nxeditHelp.SetWindowText(
			"You can specify an input mask in EMR tables that allows you to auto-format cells with a specific "
			"number and type of characters. The special formatting characters that you can use in your input "
			"mask to represent what can be typed in these cells are...\r\n\r\n"
			"\t? - Letter\r\n"
			"\t9 - Digit (0 - 9)\r\n"
			"\ta - Letter or digit\r\n"
			"\t# - Digit, space, or a plus or minus sign\r\n"
			"\tC - Any character or space\r\n"
			"\r\n"
			"Once you have entered an input mask you can click on the field below to test what it will be like "
			"to type in the cells for which you are setting the input mask.\r\n"
			);

	}NxCatchAll("CEmrTableInputMaskDlg::OnInitDialog");

	return TRUE;
}

void CEmrTableInputMaskDlg::OnEnChangeInputMask()
{
	try
	{
		m_editInputMaskTest.SetWindowText("");

		CString strInputMask;
		m_nxeditInputMask.GetWindowText(strInputMask);
		m_editInputMaskTest.SetInputMask(strInputMask);

	}NxCatchAll("CEmrTableInputMaskDlg::OnEnChangeInputMask");
}

void CEmrTableInputMaskDlg::SetInitialInputMask(const CString strInputMask)
{
	m_strInputMask = strInputMask;
}

CString CEmrTableInputMaskDlg::GetInputMask()
{
	return m_strInputMask;
}

void CEmrTableInputMaskDlg::OnBnClickedOk()
{
	try
	{
		m_nxeditInputMask.GetWindowText(m_strInputMask);
		CNxDialog::OnOK();

	}NxCatchAll("CEmrTableInputMaskDlg::OnBnClickedOk");
}

