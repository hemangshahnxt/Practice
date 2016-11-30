// EmrEditAutoNumberSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrEditAutoNumberSettingsDlg.h"


// CEmrEditAutoNumberSettingsDlg dialog
// (z.manning 2010-08-11 13:17) - PLID 40074 - Created

IMPLEMENT_DYNAMIC(CEmrEditAutoNumberSettingsDlg, CNxDialog)

CEmrEditAutoNumberSettingsDlg::CEmrEditAutoNumberSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrEditAutoNumberSettingsDlg::IDD, pParent)
{
	m_eType = etantPerRow;
}

CEmrEditAutoNumberSettingsDlg::~CEmrEditAutoNumberSettingsDlg()
{
}

void CEmrEditAutoNumberSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_AUTO_NUMBER_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEmrEditAutoNumberSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CEmrEditAutoNumberSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CEmrEditAutoNumberSettingsDlg message handlers

BOOL CEmrEditAutoNumberSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		UpdateData(FALSE);
		((CNxEdit*)GetDlgItem(IDC_EMR_AUTO_NUMBER_PREFIX))->SetLimitText(100);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetDlgItemText(IDC_EMR_AUTO_NUMBER_PREFIX, m_strPrefix);
		switch(m_eType)
		{
			case etantPerRow:
				CheckDlgButton(IDC_EMR_AUTO_NUMBER_PER_ROW, BST_CHECKED);
				break;
			case etantPerStamp:
				CheckDlgButton(IDC_EMR_AUTO_NUMBER_PER_STAMP, BST_CHECKED);
				break;
		}

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CEmrEditAutoNumberSettingsDlg::OnBnClickedOk()
{
	try
	{
		GetDlgItemText(IDC_EMR_AUTO_NUMBER_PREFIX, m_strPrefix);

		if(IsDlgButtonChecked(IDC_EMR_AUTO_NUMBER_PER_ROW) == BST_CHECKED) {
			m_eType = etantPerRow;
		}
		else if(IsDlgButtonChecked(IDC_EMR_AUTO_NUMBER_PER_STAMP) == BST_CHECKED) {
			m_eType = etantPerStamp;
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}
