// EmrTableAutofillSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrTableAutofillSettingsDlg.h"
#include "EmrItemEntryDlg.h"


// CEmrTableAutofillSettingsDlg dialog
// (z.manning 2011-03-18 17:27) - PLID 23662 - Created

IMPLEMENT_DYNAMIC(CEmrTableAutofillSettingsDlg, CNxDialog)

CEmrTableAutofillSettingsDlg::CEmrTableAutofillSettingsDlg(CEmrInfoDataElement *peide, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrTableAutofillSettingsDlg::IDD, pParent)
	, m_peide(peide)
{
}

CEmrTableAutofillSettingsDlg::~CEmrTableAutofillSettingsDlg()
{
}

void CEmrTableAutofillSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_TABLE_AUTOFILL_BACKGROUND, m_nxclrBackground);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEmrTableAutofillSettingsDlg, CNxDialog)
END_MESSAGE_MAP()


// CEmrTableAutofillSettingsDlg message handlers

BOOL CEmrTableAutofillSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_nxclrBackground.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		Load();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEmrTableAutofillSettingsDlg::OnOK()
{
	try
	{
		if(ValidateAndSave()) {
			CNxDialog::OnOK();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrTableAutofillSettingsDlg::Load()
{
	switch(m_peide->m_eAutofillType)
	{
		case etatNone:
			CheckDlgButton(IDC_AUTOFILL_NONE, BST_CHECKED);
			break;

		case etatDateAndTime:
			CheckDlgButton(IDC_AUTOFILL_DATETIME, BST_CHECKED);
			break;

		case etatDate:
			CheckDlgButton(IDC_AUTOFILL_DATE_ONLY, BST_CHECKED);
			break;

		case etatTime:
			CheckDlgButton(IDC_AUTOFILL_TIME_ONLY, BST_CHECKED);
			break;
	}
}

BOOL CEmrTableAutofillSettingsDlg::ValidateAndSave()
{
	if(IsDlgButtonChecked(IDC_AUTOFILL_NONE) == BST_CHECKED) {
		m_peide->m_eAutofillType = etatNone;
	}
	else if(IsDlgButtonChecked(IDC_AUTOFILL_DATETIME) == BST_CHECKED) {
		m_peide->m_eAutofillType = etatDateAndTime;
	}
	else if(IsDlgButtonChecked(IDC_AUTOFILL_DATE_ONLY) == BST_CHECKED) {
		m_peide->m_eAutofillType = etatDate;
	}
	else if(IsDlgButtonChecked(IDC_AUTOFILL_TIME_ONLY) == BST_CHECKED) {
		m_peide->m_eAutofillType = etatTime;
	}
	else {
		MessageBox("You must make a selection.", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}