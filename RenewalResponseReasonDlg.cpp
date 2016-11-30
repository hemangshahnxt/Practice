// RenewalResponseReasonDlg.cpp : implementation file
// (a.wilson 2013-01-08 17:50) - PLID 54512 - new dialog to allow for predefined reasons as well as free text renewal responses.

#include "stdafx.h"
#include "Practice.h"
#include "RenewalResponseReasonDlg.h"
#include "NxAPI.h"

// CRenewalResponseReasonDlg dialog

IMPLEMENT_DYNAMIC(CRenewalResponseReasonDlg, CNxDialog)

enum RenewalReasonListColumns
{
	rrlcID = 0,
	rrlcSureScriptsCode = 1,
	rrlcCheckBox = 2,
	rrlcReason = 3,
};

CRenewalResponseReasonDlg::CRenewalResponseReasonDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRenewalResponseReasonDlg::IDD, pParent) {}

CRenewalResponseReasonDlg::~CRenewalResponseReasonDlg() {}

void CRenewalResponseReasonDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_RENEWAL_RESPONSE_REASON_EDIT, m_editDetailedReason);
	DDX_Control(pDX, IDC_RENEWAL_RESPONSE_SAVETEXT, m_chkSaveDetailedReason);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	CNxDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRenewalResponseReasonDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CRenewalResponseReasonDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CRenewalResponseReasonDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_editDetailedReason.SetLimitText(70);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_pResponseReasonList = BindNxDataList2Ctrl(IDC_RENEWAL_RESPONSE_SURESCRIPT_LIST, true);
		m_pResponseReasonCustomCombo = BindNxDataList2Ctrl(IDC_RENEWAL_RESPONSE_CUSTOM_DEFAULTS, true);
		
		//a choice will generate description text and fill in the list with the necessary choices.
		CString strDescription;
		switch (m_eResponseStatus)
		{
		case NexTech_Accessor::RenewalResponseStatus_Approved:
		case NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges:
			{
				strDescription = ("Please type in notes you wish to send to the pharmacy for the approval of this renewal.");
				
				//get the distance to move the controls.
				int nDistance;
				CRect rLabel, rList, rDialog;
				GetDlgItem(IDC_RENEWAL_RESPONSE_REASON_LIST_LABEL)->GetWindowRect(&rLabel);
				GetDlgItem(IDC_RENEWAL_RESPONSE_SURESCRIPT_LIST)->GetWindowRect(&rList);
				nDistance = (rList.bottom - rLabel.top);

				//move the controls
				MoveControlUp(nDistance, IDC_RENEWAL_DROPDOWN_LABEL);
				MoveControlUp(nDistance, IDC_RENEWAL_RESPONSE_SAVETEXT);
				MoveControlUp(nDistance, IDC_RENEWAL_RESPONSE_CUSTOM_DEFAULTS);
				MoveControlUp(nDistance, IDC_RENEWAL_RESPONSE_REASON_EDIT);
				MoveControlUp(nDistance, IDOK);
				MoveControlUp(nDistance, IDCANCEL);
				
				//move the dialog bottom up.
				this->GetWindowRect(&rDialog);
				rDialog.bottom -= nDistance;
				this->MoveWindow(rDialog);

				//hide the list controls.
				GetDlgItem(IDC_RENEWAL_RESPONSE_REASON_LIST_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_RENEWAL_RESPONSE_SURESCRIPT_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_RENEWAL_RESPONSE_REASON_LIST_LABEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_RENEWAL_RESPONSE_SURESCRIPT_LIST)->EnableWindow(FALSE);
			}
			break;
		case NexTech_Accessor::RenewalResponseStatus_Denied:
			{
				strDescription = ("Please select and/or type in the reason for denying the renewal request.");
			}
			break;
		case NexTech_Accessor::RenewalResponseStatus_DeniedNewRx:
			{
				strDescription = ("Please select and/or type in the reason for denying and rewriting the prescription.");
			}
			break;
		default: //this should never happen.
			{
				ASSERT(FALSE);
			}
			break;
		}
		SetDlgItemText(IDC_RENEWAL_RESPONSE_REASON_DESCRIPTION, strDescription);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (a.wilson 2013-01-08 17:51) - PLID 54512 - this will show the dialog based on the responsestatus the user selected.
int CRenewalResponseReasonDlg::PromptForResponseReason(NexTech_Accessor::RenewalResponseStatus responseStatus)
{
	m_eResponseStatus = responseStatus;

	return DoModal();
}

void CRenewalResponseReasonDlg::OnBnClickedOk()
{
	try {
		m_arReasonSelections.RemoveAll();
		m_editDetailedReason.GetWindowText(m_strDetailedReason);
		m_strDetailedReason = m_strDetailedReason.Trim();
		//generate an array of the reasons selected.  make sure it is no more than 10.
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pResponseReasonList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (AsBool(pRow->GetValue(rrlcCheckBox))) {
				m_arReasonSelections.Add(_bstr_t(pRow->GetValue(rrlcSureScriptsCode)));
			}
		}
		if (m_arReasonSelections.GetCount() > 10) {
			MessageBox("You can only select up to 10 reasons from the list.", "Too Many Reasons", MB_OK);
			return;
		}
		if (m_strDetailedReason.GetLength() > 70)
		{
			MessageBox("You can only type up to 70 characters for your detailed reason.", "Detailed Reason too Long", MB_OK);
			return;
		}
		// (a.wilson 2013-04-15 10:11) - PLID 54512 this is optional for approvals.
		if (m_strDetailedReason.IsEmpty() && m_arReasonSelections.GetCount() == 0 
			&& m_eResponseStatus != NexTech_Accessor::RenewalResponseStatus_Approved 
			&& m_eResponseStatus != NexTech_Accessor::RenewalResponseStatus_ApprovedWithChanges) {

			MessageBox("You must provide at least one reason for your response.", "Need a Reason", MB_OK);
			return;
		}
		//if they want to save the current text then insert it into data.
		if (m_chkSaveDetailedReason.GetCheck() == BST_CHECKED && !m_strDetailedReason.IsEmpty()) {
			ExecuteParamSql(GetRemoteData(), 
				"SELECT ID FROM RenewalResponseReasonListT WHERE [Description] = {STRING} "
				"IF @@ROWCOUNT = 0 "
				"BEGIN "
					"INSERT INTO RenewalResponseReasonListT ([Description]) VALUES ({STRING}) "
				"END ", m_strDetailedReason, m_strDetailedReason);
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}
BEGIN_EVENTSINK_MAP(CRenewalResponseReasonDlg, CNxDialog)
	ON_EVENT(CRenewalResponseReasonDlg, IDC_RENEWAL_RESPONSE_CUSTOM_DEFAULTS, 16, CRenewalResponseReasonDlg::SelChosenRenewalResponseCustomDefaults, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (a.wilson 2013-04-15 10:09) - PLID 54512 - fill the edit control when they make a selection.
void CRenewalResponseReasonDlg::SelChosenRenewalResponseCustomDefaults(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			CString strCustomText = VarString(pRow->GetValue(1), "");
			CString strEditText;
			m_editDetailedReason.GetWindowText(strEditText);

			if (!strEditText.IsEmpty()) {
				if (IDNO == MessageBox("This action will overwrite the current detailed reason entered.\r\nAre you sure you want to do this?", 
					"Overwrite Detailed Reason", MB_YESNO))
				{
					return;
				}
			}
			m_editDetailedReason.SetWindowText(strCustomText);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-04-15 10:09) - PLID 54515 - move the necessary controls for an approval state.
void CRenewalResponseReasonDlg::MoveControlUp(int nDistance, int nControlID)
{
	CRect rect;
	GetDlgItem(nControlID)->GetWindowRect(&rect);

	rect.top -= nDistance;
	rect.bottom -= nDistance;

	ScreenToClient(rect);
	GetDlgItem(nControlID)->MoveWindow(rect);
}