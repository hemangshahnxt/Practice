// EmrEditSentenceFormatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrEditSentenceFormatDlg.h"

// (z.manning 2010-07-29 10:12) - PLID 36150 - Created
// CEmrEditSentenceFormatDlg dialog

IMPLEMENT_DYNAMIC(CEmrEditSentenceFormatDlg, CNxDialog)

// (r.gonet 04/29/2013) - PLID 44897 - Added two arguments necessary to tell whether to 1) show <Row Name> or <Column Name> and 2) which one to show.
CEmrEditSentenceFormatDlg::CEmrEditSentenceFormatDlg(CWnd* pParent, bool bRowNameFieldAvailable/*=false*/, bool bIsTableFlipped/*=false*/)
	: CNxDialog(CEmrEditSentenceFormatDlg::IDD, pParent)
	, m_strSentenceFormat(_T(""))
	, m_strSpawnedItemsSeparator(_T(""))
	, m_bRowNameFieldAvailable(bRowNameFieldAvailable)
	, m_bIsTableFlipped(bIsTableFlipped)
{

}

CEmrEditSentenceFormatDlg::~CEmrEditSentenceFormatDlg()
{
}

void CEmrEditSentenceFormatDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMR_EDIT_SENTENCE_NXCOLOR, m_nxcolor);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Text(pDX, IDC_EMR_SENTENCE_FORMAT, m_strSentenceFormat);
	DDX_Text(pDX, IDC_SPAWNED_ITEMS_SEPARATOR, m_strSpawnedItemsSeparator);
}


BEGIN_MESSAGE_MAP(CEmrEditSentenceFormatDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EDIT_SENTENCE_INSERT_FIELD, &CEmrEditSentenceFormatDlg::OnBnClickedEditSentenceInsertField)
END_MESSAGE_MAP()


// CEmrEditSentenceFormatDlg message handlers

BOOL CEmrEditSentenceFormatDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		UpdateData(FALSE);
		((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->SetLimitText(2000);
		((CNxEdit*)GetDlgItem(IDC_SPAWNED_ITEMS_SEPARATOR))->SetLimitText(20);

		m_nxcolor.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CEmrEditSentenceFormatDlg::GetSentenceFormat()
{
	if(IsWindow(GetSafeHwnd())) {
		UpdateData(TRUE);
	}
	return m_strSentenceFormat;
}

void CEmrEditSentenceFormatDlg::SetInitialSentenceFormat(const CString &strSentenceFormat)
{
	m_strSentenceFormat = strSentenceFormat;
}

// (z.manning 2011-11-07 10:37) - PLID 46309
CString CEmrEditSentenceFormatDlg::GetSpawnedItemsSeparator()
{
	if(IsWindow(GetSafeHwnd())) {
		UpdateData(TRUE);
	}
	return m_strSpawnedItemsSeparator;
}

// (z.manning 2011-11-07 10:37) - PLID 46309
void CEmrEditSentenceFormatDlg::SetInitialSpawnedItemsSeparator(const CString &strSpawnedItemsSeparator)
{
	m_strSpawnedItemsSeparator = strSpawnedItemsSeparator;
}

void CEmrEditSentenceFormatDlg::OnBnClickedEditSentenceInsertField()
{
	try
	{
		CMenu mnu;
		mnu.CreatePopupMenu();
		int nPosition = 1;
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, DATA_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, AGE_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, GENDER_UPPER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, GENDER_LOWER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, SUBJ_UPPER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, SUBJ_LOWER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, OBJ_UPPER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, OBJ_LOWER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, POSS_UPPER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, POSS_LOWER_FIELD);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, SPAWNING_FIELD);
		// (z.manning 2011-11-03 15:17) - PLID 42765 - Added field for spawned item
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, SPAWNED_ITEMS_FIELD);

		// (r.gonet 04/29/2013) - PLID 44897 - Add the options for row name or column name (depending on the orientation of the table)
		if(m_bRowNameFieldAvailable) {
			mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			nPosition++;
			if(!m_bIsTableFlipped) {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, ROW_NAME_FIELD);
			} else {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nPosition++, COLUMN_NAME_FIELD);
			}
		}

		CRect rcButton;
		GetDlgItem(IDC_EDIT_SENTENCE_INSERT_FIELD)->GetWindowRect(rcButton);
		int nSelection = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, rcButton.right, rcButton.top, this);
		switch(nSelection)
		{
			case 1:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(DATA_FIELD);
				break;
			case 2:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(AGE_FIELD);
				break;
			case 3:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(GENDER_UPPER_FIELD);
				break;
			case 4:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(GENDER_LOWER_FIELD);
				break;
			case 5:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(SUBJ_UPPER_FIELD);
				break;
			case 6:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(SUBJ_LOWER_FIELD);
				break;
			case 7:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(OBJ_UPPER_FIELD);
				break;
			case 8:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(OBJ_LOWER_FIELD);
				break;
			case 9:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(POSS_UPPER_FIELD);
				break;
			case 10:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(POSS_LOWER_FIELD);
				break;
			case 11:
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(SPAWNING_FIELD);
				break;
			case 12: // (z.manning 2011-11-03 15:18) - PLID 42765 - Spawned items
				((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(SPAWNED_ITEMS_FIELD);
				break;
			case 13: // (r.gonet 04/29/2013) - PLID 44897 - separator
				break;
			case 14: // (r.gonet 04/29/2013) - PLID 44897 - row name / column name
			{
				if(!m_bIsTableFlipped) {
					((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(ROW_NAME_FIELD);
				} else {
					((CNxEdit*)GetDlgItem(IDC_EMR_SENTENCE_FORMAT))->ReplaceSel(COLUMN_NAME_FIELD);
				}
				break;
			}
		}

		if(nSelection != 0) {
			GetDlgItem(IDC_EMR_SENTENCE_FORMAT)->SetFocus();
		}

	}NxCatchAll(__FUNCTION__);
}

void CEmrEditSentenceFormatDlg::OnOK()
{
	try
	{
		UpdateData(TRUE);
		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}