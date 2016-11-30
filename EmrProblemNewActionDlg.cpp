// EmrProblemNewActionDlg.cpp : implementation file
// (c.haag 2014-07-22) - PLID 62789 - Initial implementation

#include "stdafx.h"
#include "administratorrc.h"
#include "Practice.h"
#include "EmrProblemNewActionDlg.h"
#include "afxdialogex.h"
#include "UTSSearchDlg.h"

typedef enum {
	epslcID = 0,
	epslcName = 1,
} EProblemStatusListCol;

enum SNOWMEDCodeListColumns
{
	sclcID = 0,
	sclcCode,
	sclcName,
	sclcDescription,
};

// CEmrProblemNewActionDlg dialog

IMPLEMENT_DYNAMIC(CEmrProblemNewActionDlg, CNxDialog)

CEmrProblemNewActionDlg::CEmrProblemNewActionDlg(EmrActionObject SourceType,
	EmrActionObject DestType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrProblemNewActionDlg::IDD, pParent)
	,m_SourceType(SourceType)
	,m_DestType(DestType)
{

}

CEmrProblemNewActionDlg::~CEmrProblemNewActionDlg()
{
}

void CEmrProblemNewActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_LINK1, m_radioLink1);
	DDX_Control(pDX, IDC_RADIO_LINK2, m_radioLink2);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_LINK_TO, m_staticLinkTo);
	DDX_Control(pDX, IDC_ADMIN_CHECK_NOSHOWCCDA, m_checkDoNotShowOnCCDA); // (s.tullis 2015-02-24 11:31) - PLID 64724 
	DDX_Control(pDX, IDC_ADMIN_CHECK_DO_NOT_SHOW_ON_PROMPT, m_checkDoNotShowOnProblemPrompt);
}


BEGIN_MESSAGE_MAP(CEmrProblemNewActionDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CEmrProblemNewActionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_EDIT_SNOMED_LIST, &CEmrProblemNewActionDlg::OnBnClickedBtnEditSnomedList)
END_MESSAGE_MAP()


// CEmrProblemNewActionDlg message handlers

BOOL CEmrProblemNewActionDlg::OnInitDialog()
{
	try {
		__super::OnInitDialog();

		// Bind datalists
		m_dlStatusCombo = BindNxDataList2Ctrl(IDC_COMBO_ACTION_PROBLEM_STATUS, false);
		m_dlSNOMEDList = BindNxDataList2Ctrl(this, IDC_LIST_PROBLEM_SNOMED, GetRemoteData(), true);
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlSNOMEDList->GetNewRow();
		pNewRow->PutValue(sclcID, g_cvarNull);
		pNewRow->PutValue(sclcCode, " <None>");
		pNewRow->PutValue(sclcName, _bstr_t(""));
		pNewRow->PutValue(sclcDescription, _bstr_t(""));
		m_dlSNOMEDList->AddRowSorted(pNewRow, NULL);
		m_dlSNOMEDList->CurSel = pNewRow;

		// Set text limitations
		((CEdit*)GetDlgItem(IDC_EDIT_PROBLEM_ACTION_DESCRIPTION))->SetLimitText(2000);

		// Iconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Set up the "Associate this new problem with" radios
		if (eaoEmrDataItem == m_SourceType || eaoEmrItem == m_SourceType)
		{
			// (z.manning 2011-11-15 09:56) - PLID 46231 - I simplified this code block now that we're able to 
			// associate spawned EMR problems with spawned topics.
			m_radioLink1.SetWindowText(GetEmrActionObjectName(m_SourceType, TRUE));
			m_radioLink2.SetWindowText(GetEmrActionObjectName(m_DestType, TRUE));
			m_radioLink1.SetCheck(1);
		}
		else {
			// This must be a hotspot action. We only support associating the problem with the destination item
			m_staticLinkTo.SetWindowText(FormatString("The new problem will be associated with the %s.",
				GetEmrActionObjectName(m_DestType, TRUE)));
			GetDlgItem(IDC_RADIO_LINK1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_LINK2)->ShowWindow(SW_HIDE);
		}

		// Fill the problem list
		m_dlStatusCombo->WhereClause = _bstr_t("Inactive = 0 AND ID <> 2");
		m_dlStatusCombo->Requery();
		m_dlStatusCombo->TrySetSelByColumn_Deprecated(epslcID, 1L); // 1 = Open problem

		// (s.tullis 2015-02-24 11:31) - PLID 64724 - Default check off
		m_checkDoNotShowOnCCDA.SetCheck(BST_UNCHECKED);
		// (r.gonet 2015-03-17 10:33) - PLID 65013 - Initialize the DoNotShowOnProblemPrompt checkbox to unchecked.
		m_checkDoNotShowOnProblemPrompt.SetCheck(BST_UNCHECKED);

		// Set the focus to the problem description so the user can just get
		// started adding problems
		GetDlgItem(IDC_EDIT_PROBLEM_ACTION_DESCRIPTION)->SetFocus();
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrProblemNewActionDlg::OnBnClickedOk()
{
	try
	{
		CString strDescription;
		GetDlgItemText(IDC_EDIT_PROBLEM_ACTION_DESCRIPTION, strDescription);

		m_selStatus = m_dlStatusCombo->CurSel->GetValue(epslcID);
		m_selDescription = _bstr_t(strDescription);
		if (eaoEmrDataItem == m_SourceType || eaoEmrItem == m_SourceType)
		{
			m_selAssocWith = (long)(m_radioLink1.GetCheck() ? m_SourceType : m_DestType);
		}
		else
		{
			// This must be a hotspot action. We only support associating the problem with the destination item
		}
		m_selSNOMEDCode = m_dlSNOMEDList->CurSel->GetValue(sclcID);

		// (s.tullis 2015-02-24 11:31) - PLID 64724 
		m_selDoNotShowOnCCDA = _variant_t(m_checkDoNotShowOnCCDA.GetCheck());
		// (r.gonet 2015-03-17 10:33) - PLID 65013 - Set the selected DoNotShowOnProblemPrompt value from the user's selection.
		m_selDoNotShowOnProblemPrompt = m_checkDoNotShowOnProblemPrompt.GetCheck() == BST_CHECKED ? g_cvarTrue : g_cvarFalse;
		__super::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CEmrProblemNewActionDlg, CNxDialog)
	ON_EVENT(CEmrProblemNewActionDlg, IDC_COMBO_ACTION_PROBLEM_STATUS, 1, CEmrProblemNewActionDlg::SelChangingComboActionProblemStatus, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEmrProblemNewActionDlg, IDC_LIST_PROBLEM_SNOMED, 1, CEmrProblemNewActionDlg::SelChangingListProblemSnomed, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()


void CEmrProblemNewActionDlg::SelChangingComboActionProblemStatus(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}


void CEmrProblemNewActionDlg::SelChangingListProblemSnomed(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}


void CEmrProblemNewActionDlg::OnBnClickedBtnEditSnomedList()
{
	try
	{
		CUTSSearchDlg dlg;
		if (IDOK == dlg.DoModal())
		{
			_variant_t vOldSel = m_dlSNOMEDList->GetCurSel()->GetValue(sclcID);

			// Requery the list
			m_dlSNOMEDList->Requery();
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlSNOMEDList->GetNewRow();
			pNewRow->PutValue(sclcID, g_cvarNull);
			pNewRow->PutValue(sclcCode, " <None>");
			pNewRow->PutValue(sclcName, _bstr_t(""));
			pNewRow->PutValue(sclcDescription, _bstr_t(""));
			m_dlSNOMEDList->AddRowSorted(pNewRow, NULL);

			// Restore the old selection
			m_dlSNOMEDList->SetSelByColumn(sclcID, vOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}
