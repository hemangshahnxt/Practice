// SchedulerMixWarningDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerMixWarningDlg.h"
#include "CommonSchedUtils.h"

// (j.jones 2014-11-26 09:31) - PLID 64179 - created

// CSchedulerMixWarningDlg dialog

IMPLEMENT_DYNAMIC(CSchedulerMixWarningDlg, CNxDialog)

CSchedulerMixWarningDlg::CSchedulerMixWarningDlg(CWnd* pParent /*= NULL*/)
	: CNxDialog(CSchedulerMixWarningDlg::IDD, pParent)
{	
}

CSchedulerMixWarningDlg::~CSchedulerMixWarningDlg()
{
}

void CSchedulerMixWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_OVERRIDE, m_btnOverride);
	DDX_Control(pDX, IDC_BTN_FFA, m_btnFFA);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_WARNING_LABEL, m_nxstaticWarningLabel);
}

int CSchedulerMixWarningDlg::DoModal(const std::vector<SchedulerMixRule> &exceededMixRules)
{
	try {

		m_exceededMixRules.clear();
		m_exceededMixRules.insert(m_exceededMixRules.end(), exceededMixRules.begin(), exceededMixRules.end());

		//make sure we at least received one rule, and confirm the first rule is filled out
		if (m_exceededMixRules.size() > 0 && m_exceededMixRules[0].nID == -1 || m_exceededMixRules[0].strName.IsEmpty()) {
			//should be impossible
			ASSERT(FALSE);
			ThrowNxException("No scheduling mix rule was provided.");
		}

		return CNxDialog::DoModal();

	}NxCatchAll(__FUNCTION__);

	return (int)smwrvCancelled;
}

BOOL CSchedulerMixWarningDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOverride.AutoSet(NXB_OK);
		m_btnFFA.SetIcon(IDR_PNG_FFA, 0, FALSE, TRUE);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//check the user's override permission
		bool bCanOverride = false;
		if (GetCurrentUserPermissions(bioSchedMixRuleOverride) & (sptWrite | sptWriteWithPass)) {
			bCanOverride = true;
		}

		CString strRuleNames;
		//Build a comma delimited list of rule names, but if it's going
		//to be huge, cut it off with a (more) entry. Having multiple
		//applicable rules is not going to be a common occurrence.
		for each (SchedulerMixRule rule in m_exceededMixRules)
		{
			if (!strRuleNames.IsEmpty()) {
				strRuleNames += ", ";
			}
			if (strRuleNames.GetLength() > 255) {
				strRuleNames += "(more)";
				break;
			}
			strRuleNames += rule.strName;

			if (rule.nID == -1) {
				//this is a total failure
				ASSERT(FALSE);
				ThrowNxException("SchedulerMixWarningDlg called with an invalid rule ID!");
			}
		}
								
		CString strWarningText;
		//give a context sensitive warning based on their permission
		strWarningText.Format("This appointment matches the criteria for the scheduling mix rule%s %s, "
			"and the maximum number of appointments for %s has been met for this day.\n\n",
			m_exceededMixRules.size() > 1 ? "s" : "",
			strRuleNames,
			m_exceededMixRules.size() > 1 ? "these rules" : "this rule");

		if (bCanOverride) {
			strWarningText += FormatString("You may override %s, or search for the next available appointment.", m_exceededMixRules.size() > 1 ? "these rules" : "this rule");
		}
		else {
			strWarningText += FormatString("You do not have permission to override %s. Please see your practice administrator, or you may search for the next available appointment.", m_exceededMixRules.size() > 1 ? "these rules" : "this rule");
		}

		m_nxstaticWarningLabel.SetWindowText(strWarningText);

		//if the user cannot override rules, hide the Override button,
		//rename cancel to OK and change its icon, and rearrange the buttons
		if (!bCanOverride) {
			//though it says OK, it will still cancel, and do nothing
			m_btnCancel.AutoSet(NXB_OK);
			m_btnCancel.SetWindowText("OK");

			m_btnOverride.ShowWindow(SW_HIDE);
			CRect rcCancel, rcFFA;			
			//we're swapping Cancel with where Override was
			m_btnOverride.GetWindowRect(rcCancel);
			m_btnFFA.GetWindowRect(rcFFA);
			ScreenToClient(rcCancel);
			ScreenToClient(rcFFA);

			//since we will only have two buttons, try to center them
			long nBtnHeight = rcCancel.Height();

			//Now we only have two buttons, Cancel & FFA, so move them down
			//to give a more centered look to them.
			//Move Cancel down by 1/2 its height.
			rcCancel.OffsetRect(0, (nBtnHeight / 2));
			//Move FFA down by 3/4 its height.
			rcFFA.OffsetRect(0, ((nBtnHeight / 4) * 3));

			//move the FFA & Cancel buttons, using the new positions
			//using SetWindowPos will also correct the tab order
			m_btnCancel.SetWindowPos(GetDlgItem(IDC_WARNING_LABEL), rcCancel.left, rcCancel.top, rcCancel.Width(), rcCancel.Height(), 0);
			m_btnFFA.SetWindowPos(GetDlgItem(IDCANCEL), rcFFA.left, rcFFA.top, rcFFA.Width(), rcFFA.Height(), 0);
		}
		
		//the cancel button always shows, and should be where focus begins
		m_btnCancel.SetFocus();

	}NxCatchAll(__FUNCTION__);

	return FALSE;	//we're forcing the focus above
}

BEGIN_MESSAGE_MAP(CSchedulerMixWarningDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_OVERRIDE, OnBtnOverride)
	ON_BN_CLICKED(IDC_BTN_FFA, OnBtnFFA)
	ON_BN_CLICKED(IDCANCEL, OnBtnCancel)
END_MESSAGE_MAP()

// CSchedulerMixWarningDlg message handlers

void CSchedulerMixWarningDlg::OnBtnOverride()
{
	try {

		//check their permission, they may need to enter a password
		if (!CheckCurrentUserPermissions(bioSchedMixRuleOverride, sptWrite)) {
			return;
		}

		EndDialog((int)smwrvOverride);

	}NxCatchAll(__FUNCTION__);
}


void CSchedulerMixWarningDlg::OnBtnFFA()
{
	try {

		EndDialog((int)smwrvFFA);

	}NxCatchAll(__FUNCTION__);
}


void CSchedulerMixWarningDlg::OnBtnCancel()
{
	try {

		EndDialog((int)smwrvCancelled);

	}NxCatchAll(__FUNCTION__);
}
