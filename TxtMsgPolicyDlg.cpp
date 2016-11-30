// (r.gonet 01-11-2011) - PLID 42010 - Created
// TxtMsgPolicyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PracticeRc.h"
#include "TxtMsgPolicyDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"


// CTxtMsgPolicyDlg dialog

IMPLEMENT_DYNAMIC(CTxtMsgPolicyDlg, CNxDialog)

CTxtMsgPolicyDlg::CTxtMsgPolicyDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTxtMsgPolicyDlg::IDD, pParent)
{
	// (r.gonet 01-11-2011) - PLID 42010 - This legal text was revised on 02/10/2011
	m_strPolicyMessage = 
		"The service NexReminder Automated Appointment Reminders is regulated under the " 
		"Mobile Marketing Association (MMA). The MMA requires all text message programs to "
		"alert the message recipient to certain terms and to obtain the explicit permission from "
		"the recipient before sending any messages. Any unsolicited message is classified as spam "
		"by the MMA and might result in the service's suspension or termination. \r\n"
		"\r\n"
		"NexTech Practice requires you to ask the patient if they want to opt in to the service "
		"and to tell them the following before sending any appointment reminder text messages to them: \r\n"
		"\r\n"
		"\"You are subscribing to the NexReminder Appointment Reminders program. Message and data "
		"rates may apply. You may stop at any time by replying with STOP to the appointment reminder message.\" \r\n"
		"\r\n"
		"If you check the Text Message Privacy box in Patients General 1 the patient is opted out; if you leave it unchecked the patient is opted in. "
		"You can set the following global defaults: " 
		"\r\n" 
		"Opt All Out; Opt All In; Leave Data As Is; or Cancel. \r\n"
		"\r\n"
		"After you make the selection you should modify individual patients who do not meet the default.";
}

CTxtMsgPolicyDlg::~CTxtMsgPolicyDlg()
{
}

void CTxtMsgPolicyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TXT_MSG_OPT_ALL_IN_BTN, m_btnOptAllIn);
	DDX_Control(pDX, IDC_TXT_MSG_OPT_ALL_OUT_BTN, m_btnOptAllOut);
	DDX_Control(pDX, IDC_TXT_MSG_LEAVE_AS_IS_BTN, m_btnLeaveAsIs);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_TXT_MSG_POLICY_EDIT, m_nxeditPolicy);
	DDX_Control(pDX, IDC_TXT_MSG_COLOR, m_nxcolorPolicy);
}


BEGIN_MESSAGE_MAP(CTxtMsgPolicyDlg, CNxDialog)
	ON_BN_CLICKED(IDC_TXT_MSG_OPT_ALL_OUT_BTN, &CTxtMsgPolicyDlg::OnBnClickedTxtMsgOptAllOutBtn)
	ON_BN_CLICKED(IDC_TXT_MSG_OPT_ALL_IN_BTN, &CTxtMsgPolicyDlg::OnBnClickedTxtMsgOptAllInBtn)
	ON_BN_CLICKED(IDC_TXT_MSG_LEAVE_AS_IS_BTN, &CTxtMsgPolicyDlg::OnBnClickedTxtMsgLeaveasisBtn)
END_MESSAGE_MAP()


// CTxtMsgPolicyDlg message handlers

BOOL CTxtMsgPolicyDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//  This isn't really part of any module, so let's just use patients module color.
		m_nxcolorPolicy.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// Set the button icons
		m_btnOptAllOut.AutoSet(NXB_MODIFY);
		m_btnOptAllIn.AutoSet(NXB_MODIFY);
		m_btnLeaveAsIs.AutoSet(NXB_MODIFY);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Now set the policy text
		m_nxeditPolicy.SetWindowText(_bstr_t(m_strPolicyMessage));

		// Optionally show the leave as is button
		if(!m_bShowLeaveAsIsButton) {
			m_btnLeaveAsIs.ShowWindow(SW_HIDE);
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 01-11-2011) - PLID 42010 - Handles the user's request to opt all patients into text messages.
void CTxtMsgPolicyDlg::OnBnClickedTxtMsgOptAllInBtn()
{
	try {
		// (r.gonet 01-11-2011) - PLID 42010 - Requires a very strict warning because of the spam regulations out there.
		if(IDYES == MessageBox(
			"By opting all existing patients in, you need to have gotten their explicit confirmation "
			"that they want opted in to the NexReminder Automated Appointment Reminders program. "
			"Sending unsolicited messages is regarded as spamming by the MMA. \r\n"
			"\r\n"
			"Are you sure that all existing patients explicitly said they want to opt in?",
			"Warning and Confirmation", MB_YESNO|MB_ICONEXCLAMATION))
		{
			// SO BE IT! Now for the mass edit.
			ExecuteSql(GetRemoteData(),
				"UPDATE PersonT "
				"SET TextMessage = 0 "
				"FROM PersonT JOIN "
				"	PatientsT ON PersonT.ID = PatientsT.PersonID ");

			// Audit of course to cover us against the MMA
			long nAuditID = BeginAuditTransaction();
			AuditEvent(-1, "", nAuditID, aeiMassTxtMsgOptedIn, -1, "", "", aepHigh);
			CommitAuditTransaction(nAuditID);

			MessageBox("All existing patients have been opted in to receiving text messages.", 0, MB_OK|MB_ICONINFORMATION);
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 01-11-2011) - PLID 42010 - Handles the user's request to opt all patients out of text messages.
void CTxtMsgPolicyDlg::OnBnClickedTxtMsgOptAllOutBtn()
{
	try {
		// // (r.gonet 01-11-2011) - PLID 42010 - Requires a much less strict warning message.
		if(IDYES == MessageBox(
			"All existing patients' Text Message Privacy settings will be turned on. "
			"Are you sure you want to do this?",
			"Warning and Confirmation", MB_YESNO|MB_ICONEXCLAMATION))
		{
			// SO BE IT! Now for the mass edit.
			ExecuteSql(GetRemoteData(),
				"UPDATE PersonT "
				"SET TextMessage = 1 "
				"FROM PersonT JOIN "
				"	PatientsT ON PersonT.ID = PatientsT.PersonID ");
			// Audit of course to cover us against the MMA
			long nAuditID = BeginAuditTransaction();
			AuditEvent(-1, "", nAuditID, aeiMassTxtMsgOptedOut, -1, "", "", aepHigh);
			CommitAuditTransaction(nAuditID);

			MessageBox("All existing patients have been opted out of receiving text messages.", 0, MB_OK|MB_ICONINFORMATION);
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 01-11-2011) - PLID 42010 - Handles the user's request to leave the privacy data as is while enabling NexReminder.
void CTxtMsgPolicyDlg::OnBnClickedTxtMsgLeaveasisBtn()
{
	try {
		// (r.gonet 01-11-2011) - PLID 42010 - Requires a warning message equally as strict as opting all in.
		if(IDYES == MessageBox(
			"By leaving all text message privacy settings as is, you need to have gotten existing explicit confirmation "
			"that patients without Text Message Privacy turned on want to be opted into the NexReminder Automated Appointment Reminders program. "
			"Sending unsolicited messages is regarded as spamming by the MMA. \r\n"
			"\r\n"
			"Are you sure that these patients explicitly said they want to opt in?",
			"Warning and Confirmation", MB_YESNO|MB_ICONEXCLAMATION))
		{
			// (r.gonet 01-11-2011) - PLID 42010 - Yeah, even though we didn't change anything, we are still going to audit because they agreed that they got permission
			//  from the patients who had Text Message Privacy off before.
			// Audit of course to cover us against the MMA
			long nAuditID = BeginAuditTransaction();
			AuditEvent(-1, "", nAuditID, aeiMassTxtMsgLeftAsIs, -1, "", "", aepHigh);
			CommitAuditTransaction(nAuditID);

			MessageBox("All existing patients have had the privacy settings left as is.", 0, MB_OK|MB_ICONINFORMATION);
			CNxDialog::OnOK();
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 01-11-2011) - PLID 42010 - Toggles on and off whether the Leave All As Is button will show up. It is useless in certain circumstances.
void CTxtMsgPolicyDlg::ShowLeaveAsIsButton(BOOL bShow)
{
	m_bShowLeaveAsIsButton = bShow;
}
