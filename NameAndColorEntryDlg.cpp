// NameAndColorEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerRc.h"
#include "NameAndColorEntryDlg.h"
#include "afxdialogex.h"


// (z.manning 2014-12-04 15:38) - PLID 64217 - Created
// CNameAndColorEntryDlg dialog

IMPLEMENT_DYNAMIC(CNameAndColorEntryDlg, CNxDialog)

CNameAndColorEntryDlg::CNameAndColorEntryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNameAndColorEntryDlg::IDD, pParent)
{
	m_nColor = RGB(255, 0, 0);
	m_nTextLimit = 0;
}

CNameAndColorEntryDlg::~CNameAndColorEntryDlg()
{
}

void CNameAndColorEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLOR_CHOOSER_CTRL, m_ctrlColorPicker);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CNameAndColorEntryDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NAME_COLOR_ENTRY_CHOOSE_COLOR, &CNameAndColorEntryDlg::OnBnClickedNameColorEntryChooseColor)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()


// CNameAndColorEntryDlg message handlers

BOOL CNameAndColorEntryDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		if (!m_strWindowTitle.IsEmpty()) {
			SetWindowText(m_strWindowTitle);
		}

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CEdit *peditName = (CEdit*)GetDlgItem(IDC_NAME_COLOR_ENTRY_NAME);
		if (m_nTextLimit > 0) {
			peditName->SetLimitText(m_nTextLimit);
		}

		SetDlgItemText(IDC_NAME_COLOR_ENTRY_NAME, m_strName);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNameAndColorEntryDlg::OnBnClickedNameColorEntryChooseColor()
{
	try
	{
		m_ctrlColorPicker.SetColor(m_nColor);
		m_ctrlColorPicker.ShowColor();
		m_nColor = m_ctrlColorPicker.GetColor();
		GetDlgItem(IDC_NAME_COLOR_ENTRY_CHOOSE_COLOR)->RedrawWindow();
	}
	NxCatchAll(__FUNCTION__);
}

void CNameAndColorEntryDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl == IDC_NAME_COLOR_ENTRY_CHOOSE_COLOR) {
		DrawColorOnButton(lpDrawItemStruct, m_nColor);
	}
	else {
		CNxDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
	}
}

BOOL CNameAndColorEntryDlg::Validate()
{
	GetDlgItemText(IDC_NAME_COLOR_ENTRY_NAME, m_strName);

	// (z.manning 2014-12-05 09:32) - PLID 64217 - Don't allow blank names
	m_strName.TrimRight();
	if (m_strName.IsEmpty()) {
		MessageBox("Please enter a name.", NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	if (!m_strSqlTable.IsEmpty() && !m_strSqlColumn.IsEmpty())
	{
		// (z.manning 2014-12-05 10:07) - PLID 64217 - If we were given a SQL table/column then ensure the new
		// value is not a duplicate of that.
		if (ReturnsRecordsParam(FormatString("SELECT TOP 1 1 FROM %s T WHERE T.%s = {STRING}"
			, m_strSqlTable, m_strSqlColumn)
			, m_strName))
		{
			MessageBox(FormatString("The name '%s' is already in use. Please enter a different name.", m_strName)
				, NULL, MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	return TRUE;
}

void CNameAndColorEntryDlg::OnOK()
{
	try
	{
		if (Validate()) {
			CNxDialog::OnOK();
		}
	}
	NxCatchAll(__FUNCTION__);
}
