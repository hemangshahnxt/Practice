// AppointmentReminderSetupDlg.cpp : implementation file
// (c.haag 2010-03-25 11:54) - PLID 37891 - Initial implementation

#include "stdafx.h"
#include "Practice.h"
#include "AppointmentReminderSetupDlg.h"
#include "NexSyncEditSubjectLineDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "NxFolderField.h"
#include "NxOutlookUtils.h"
#include "ApptReminderSchedDlg.h"
#include "ApptReminderSchedule.h"
#include "NxReminderSOAPUtils.h"
#include "TxtMsgPolicyDlg.h"
#include "AuditTrail.h"
#include "AppointmentReminderAdvancedDlg.h"

// CAppointmentReminderSetupDlg dialog

IMPLEMENT_DYNAMIC(CAppointmentReminderSetupDlg, CNxDialog)

CAppointmentReminderSetupDlg::CAppointmentReminderSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAppointmentReminderSetupDlg::IDD, pParent)
{
	m_bNexReminderEnabled = FALSE;
}

CAppointmentReminderSetupDlg::~CAppointmentReminderSetupDlg()
{
}

void CAppointmentReminderSetupDlg::UpdateControls()
{
	// If integration is disabled, disable all the settings. Otherwise, ensure they are enabled.
	BOOL bEnable = m_radioOn.GetCheck() ? TRUE : FALSE;
	GetDlgItem(IDC_START_REMINDING_DATE)->EnableWindow(bEnable);
	// (r.gonet 09-17-2010) - PLID 40574 - We now use a rules based system for when to send a reminder
	//GetDlgItem(IDC_EDIT_AR_NOTIFICATION)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_AR_FORMAT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_AR_EDIT_FORMAT)->EnableWindow(bEnable);
	// (r.gonet 2010-06-23) - PLID 39281 - Enable the new country code textbox 
	GetDlgItem(IDC_COUNTRY_CODE_TEXTBOX)->EnableWindow(bEnable);
	// (r.gonet 2010-06-25) - PLID 39012 - Enable the appointment reminder schedule editor button
	GetDlgItem(IDC_BTN_APPT_REMINDER_SCHED)->EnableWindow(bEnable);

	// (r.gonet 2011-06-17) - PLID 43299 - Enable or disable the Mark appointment as Left Message checkbox.
	GetDlgItem(IDC_NXREMINDER_LEFTMESSAGE_CHECK)->EnableWindow(bEnable);
	// (z.manning 2010-11-17 16:13) - PLID 41495
	GetDlgItem(IDC_NXREMINDER_CONFIRM_CHECK)->EnableWindow(bEnable);
	BOOL bEnableConfirmRadioButtons;
	if(bEnable) {
		bEnableConfirmRadioButtons = (IsDlgButtonChecked(IDC_NXREMINDER_CONFIRM_CHECK) == BST_CHECKED);
	}
	else {
		bEnableConfirmRadioButtons = FALSE;
	}
	GetDlgItem(IDC_NXREMINDER_CONFIRM_ALL)->EnableWindow(bEnableConfirmRadioButtons);
	GetDlgItem(IDC_NXREMINDER_CONFIRM_LATEST)->EnableWindow(bEnableConfirmRadioButtons);
	// (r.gonet 01-11-2011) - PLID 42010 - Enable or disable the policy dialog
	GetDlgItem(IDC_NXREMINDER_TXT_MSG_POLICY_BTN)->EnableWindow(bEnable);
}

// Validates the data content in this dialog
BOOL CAppointmentReminderSetupDlg::ValidateData()
{
	_variant_t vStartDate = m_dtStartDate.GetValue();
	COleDateTime dtStartDate;
	if (VT_DATE != vStartDate.vt) {
		AfxMessageBox("Please enter a valid reminder notification starting date.", MB_OK | MB_ICONHAND);
		return FALSE;
	}
	else {
		dtStartDate = VarDateTime(vStartDate);
		dtStartDate.SetDate( dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay() );
		if (dtStartDate.m_dt < 0) {
			AfxMessageBox("Please enter a valid reminder notification starting date.", MB_OK | MB_ICONHAND);
			return FALSE;
		}
	}

	// (r.gonet 09-17-2010) - PLID 40574 - We now use a rules based system for when to send a reminder
	//long nNotificationAdvance = atol(m_strNotification);
	//if (nNotificationAdvance <= 0) {
	//	AfxMessageBox("You must configure notifications to be sent no less than one hour before their respective appointments.", MB_OK | MB_ICONHAND);
	//	return FALSE;
	//}
	/*else if (nNotificationAdvance > 5000) {
		AfxMessageBox("You may not send notifications more than 5,000 hours ahead of their respective appointments.", MB_OK | MB_ICONHAND);
		return FALSE;
	}*/

	// Check certain fields for blanks if the On option is selected. If the link is Off, we don't care.
	if (m_radioOn.GetCheck()) {
		if (m_strMsgFormat.IsEmpty()) {
			AfxMessageBox("Please enter a non-empty notification format.", MB_OK | MB_ICONHAND);
			return FALSE;
		}
	}

	// (z.manning 2010-05-04 15:27) - PLID 38492 - Warn if the message format is > 160 (the max length of a single SMS message)
	if(m_strMsgFormat.GetLength() > 160) {
		int nResult = MessageBox("The notification format is more than 160 characters which means that the notifications "
			"may get split up into multiple text messages. Are you sure you want to continue?", NULL, MB_YESNO|MB_ICONWARNING);
		if(nResult != IDYES) {
			return FALSE;
		}
	}

	// (r.gonet 2010-06-23) - PLID 39281 - Validate that the country code is a correct number of digits (1-3) and is within a valid range.
	if(m_strCountryCode.GetLength() <= 0 || m_strCountryCode.GetLength() > 3 || atol(m_strCountryCode) <= 0) {
		MessageBox(
			"Please enter a valid country code.\r\n"
			"A country code is a positive number with 1-3 digits that is prefixed to phone numbers.",
			NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}


	// All fields passed validation
	return TRUE;
}

// Saves the data content
void CAppointmentReminderSetupDlg::SaveData()
{
	long nAuditID = BeginAuditTransaction();
	
	// Enabled
	long oldApptReminderEnabled = GetRemotePropertyInt("ApptReminderEnabled", 0);
	long newApptReminderEnabled = m_radioOn.GetCheck() ? 1 : 0;
	SetRemotePropertyInt("ApptReminderEnabled", m_radioOn.GetCheck() ? 1 : 0);
	if(oldApptReminderEnabled != newApptReminderEnabled) {
		AuditEvent(-1, "", nAuditID, aeiNexReminderStateChanged, -1, (oldApptReminderEnabled == 0 ? "Off" : "On"), (newApptReminderEnabled == 0 ? "Off" : "On"), aepMedium);
	}

	// Reminder pivot date  (This is the first day that appointment reminders get transmitted).
	_variant_t vStartDate = m_dtStartDate.GetValue();
	COleDateTime dtStartDate = VarDateTime(vStartDate);
	dtStartDate.SetDate( dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay() );
	SetRemotePropertyDateTime("ApptReminderPivotDate", dtStartDate);

	// (r.gonet 09-17-2010 16:02) - PLID 40574 - We now use the rule system to specify when to send reminders
	// // Send notifications X hours in advance
	//SetRemotePropertyInt("ApptReminderTTN", atol(m_strNotification));
	// Notification format
	SetRemotePropertyText("ApptReminderMsgFormat", m_strMsgFormat);

	// (r.gonet 2010-06-23) - PLID 39281 - Save the country code property
	SetRemotePropertyInt("ApptReminderCountryCode", atol(m_strCountryCode));

	// (z.manning 2010-11-17 16:41) - PLID 41495
	UINT nLeftMessageEnable = IsDlgButtonChecked(IDC_NXREMINDER_LEFTMESSAGE_CHECK) == BST_CHECKED ? 1 : 0;
	SetRemotePropertyInt("ApptReminderEnableLeftMessage", nLeftMessageEnable);

	// (z.manning 2010-11-17 16:41) - PLID 41495
	UINT nConfirmEnable = IsDlgButtonChecked(IDC_NXREMINDER_CONFIRM_CHECK) == BST_CHECKED ? 1 : 0;
	SetRemotePropertyInt("ApptReminderEnableConfirm", nConfirmEnable);
	UINT nConfirmOption;
	if(IsDlgButtonChecked(IDC_NXREMINDER_CONFIRM_LATEST) == BST_CHECKED) {
		nConfirmOption = 2;
	}
	else {
		nConfirmOption = 1;
	}
	SetRemotePropertyInt("ApptReminderConfirmOption", nConfirmOption);
	// (r.gonet 01-13-2011) - PLID 42010 - Now that the client has completed the policy dialog, we know that they've seen it at least once.
	SetRemotePropertyInt("TxtMsgPolicyCompleted", (m_bPolicyDlgCompleted ? 1 : 0), 0, NULL);

	// (z.manning 2011-11-02 10:16) - PLID 46220
	SetRemotePropertyText("ApptReminderLeadingDigitToTruncate", m_AdvancedSettings.strLeadingDigitToTrim);

	// (r.gonet 2010-07-09) - PLID 39012 - Save the schedule to the database
	m_arsSchedule.Save();
	CommitAuditTransaction(nAuditID);
}

void CAppointmentReminderSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_AR_EDIT_FORMAT, m_btnEditFormat);
	DDX_Control(pDX, IDC_RADIO_ON, m_radioOn);
	DDX_Control(pDX, IDC_RADIO_OFF, m_radioOff);
	DDX_Control(pDX, IDC_START_REMINDING_DATE, m_dtStartDate);
	DDX_Text(pDX, IDC_EDIT_AR_FORMAT, m_strMsgFormat);
	DDX_Control(pDX, IDC_CELLTRUST_BACKGROUND, m_nxcolorBackground);
	DDX_Control(pDX, IDC_STATIC_ARS_GROUP1, m_grp1);
	DDX_Control(pDX, IDC_STATIC_ARS_GROUP3, m_grp3);
	DDX_Control(pDX, IDC_MESSAGE_FORMAT_LENGTH, m_nxstaticFormatLength);
	DDX_Control(pDX, IDC_EDIT_AR_FORMAT, m_editNotificationFormat);
	DDX_Text(pDX, IDC_COUNTRY_CODE_TEXTBOX, m_strCountryCode);
	DDX_Control(pDX, IDC_BTN_APPT_REMINDER_SCHED, m_btnApptReminderSchedule);
	DDX_Control(pDX, IDC_NXREMINDER_TXT_MSG_POLICY_BTN, m_btnPolicy);
}

BEGIN_MESSAGE_MAP(CAppointmentReminderSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_RADIO_ON, &CAppointmentReminderSetupDlg::OnBnClickedRadioOn)
	ON_BN_CLICKED(IDC_RADIO_OFF, &CAppointmentReminderSetupDlg::OnBnClickedRadioOff)
	ON_BN_CLICKED(IDOK, &CAppointmentReminderSetupDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_AR_EDIT_FORMAT, &CAppointmentReminderSetupDlg::OnBnClickedBtnArEditFormat)
	ON_EN_CHANGE(IDC_EDIT_AR_FORMAT, &CAppointmentReminderSetupDlg::OnEnChangeEditArFormat)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_APPT_REMINDER_SCHED, &CAppointmentReminderSetupDlg::OnBnClickedBtnApptReminderSched)
	ON_BN_CLICKED(IDC_NXREMINDER_CONFIRM_CHECK, &CAppointmentReminderSetupDlg::OnBnClickedNxreminderConfirmCheck)
	ON_BN_CLICKED(IDC_NXREMINDER_TXT_MSG_POLICY_BTN, &CAppointmentReminderSetupDlg::OnBnClickedNxreminderTxtMsgPolicyBtn)
	ON_BN_CLICKED(IDC_NXREMINDER_ADVANCED, &CAppointmentReminderSetupDlg::OnBnClickedNxreminderAdvanced)
END_MESSAGE_MAP()


// CAppointmentReminderSetupDlg message handlers

BOOL CAppointmentReminderSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// (r.gonet HERE)

		//  This isn't really part of any module, so let's just use patients module color.
		m_nxcolorBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// Set the button icons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnEditFormat.AutoSet(NXB_MODIFY);
		m_btnApptReminderSchedule.AutoSet(NXB_MODIFY);

		// Load all properties in bulk
		g_propManager.CachePropertiesInBulk("AppointmentReminder", propNumber,
			"(Username = '<None>') AND ("
			"Name = 'ApptReminderEnabled' OR "
			"Name = 'ApptReminderEnableLeftMessage' OR "
			"Name = 'ApptReminderEnableConfirm' OR "
			"Name = 'ApptReminderConfirmOption' OR "
			// (r.gonet 09-17-2010) - PLID 40574 - We now use a rules based system for when to send a reminder
			// "Name = 'ApptReminderTTN' OR "
			// (r.gonet 2010-06-23) - PLID 39281 - Cache the country code as well
			"Name = 'ApptReminderCountryCode' OR "
			// (r.gonet 01-11-2011) - PLID 42010 - Make sure that the policy has been seen at least once 
			"Name = 'TxtMsgPolicyCompleted' OR "
			// (r.gonet 2015-11-16 02:33) - PLID 67587
			"Name = 'ApptReminderSendingLimitOptions' "
			")");
		g_propManager.CachePropertiesInBulk("AppointmentReminder", propText,
			"(Username = '<None>') AND ("
			"Name = 'ApptReminderMsgFormat' "
			"	OR Name = 'ApptReminderLeadingDigitToTruncate' "
			")");
		g_propManager.CachePropertiesInBulk("AppointmentReminder", propDateTime,
			"(Username = '<None>') AND ("
			"Name = 'ApptReminderPivotDate'"
			")");

		///////// Fill in the fields /////////

		// Enabled
		BOOL bEnabled = GetRemotePropertyInt("ApptReminderEnabled", 0, 0, NULL, false);
		m_radioOn.SetCheck(bEnabled ? 1 : 0);
		m_radioOff.SetCheck(bEnabled ? 0 : 1);
		// (r.gonet 01-13-2011) - PLID 42010 - Keep track of the enabled state of NexReminder
		m_bNexReminderEnabled = bEnabled;

		// (r.gonet 01-13-2011) - PLID 42010 - Make sure the policy has been seen at least once
		BOOL m_bPolicyDlgCompleted = GetRemotePropertyInt("TxtMsgPolicyCompleted", 0, 0, NULL, true);
		if(m_bNexReminderEnabled && !m_bPolicyDlgCompleted) {
			// I don't know how this happened but they got by without seeing our policy on text messages
			CTxtMsgPolicyDlg dlg(this);
			dlg.ShowLeaveAsIsButton(TRUE);
			if(IDCANCEL == dlg.DoModal()) {
				m_radioOff.SetCheck(1);
				m_radioOn.SetCheck(0);
				m_bNexReminderEnabled = FALSE;
			} else {
				m_bPolicyDlgCompleted = TRUE;
			}
		}

		// Reminder pivot date (This is the first day that appointment reminders get transmitted).
		// The default value is tomorrow.
		COleDateTime dtTomorrow = COleDateTime::GetCurrentTime();
		dtTomorrow.SetDate( dtTomorrow.GetYear(), dtTomorrow.GetMonth(), dtTomorrow.GetDay() );
		dtTomorrow += COleDateTimeSpan(1,0,0,0);
		COleDateTime dtPivot = GetRemotePropertyDateTime("ApptReminderPivotDate", &dtTomorrow, 0, NULL, false);
		m_dtStartDate.SetValue(dtPivot);

		// (r.gonet 09-17-2010) - PLID 40574 - We now use a rules based system for when to send a reminder
		// // Send notifications X hours in advance
		//((CEdit*)GetDlgItem(IDC_EDIT_AR_NOTIFICATION))->SetLimitText(4);
		//m_strNotification.Format("%d", GetRemotePropertyInt("ApptReminderTTN", 24, 0, NULL, false));

		// Notification format
		// (z.manning 2010-11-23 14:19) - PLID 41560 - Reworded the default message to let users know they
		// can confirm appointment by replying with the confirm call to action keyword.
		CString strDefaultMessageFormat = FormatString("We are looking forward to seeing you on [Date] at [StartTime]. "
			"Please reply with [Confirm] as the text to confirm your appointment. Thanks!");
		m_strMsgFormat = GetRemotePropertyText("ApptReminderMsgFormat", strDefaultMessageFormat, 0, NULL, false);

		// (r.gonet 2010-06-23) - PLID 39281 - Fill the country code textbox with either what is saved in the database or a default of 1 for USA.
		((CEdit*)GetDlgItem(IDC_COUNTRY_CODE_TEXTBOX))->SetLimitText(3);
		long nCountryCode = GetRemotePropertyInt("ApptReminderCountryCode", 1, 0, NULL, false);
		m_strCountryCode.Format("%li", nCountryCode);

		// (z.manning 2010-11-17 16:34) - PLID 41495
		if(GetRemotePropertyInt("ApptReminderEnableLeftMessage", 1, 0, "<None>") == 1) {
			CheckDlgButton(IDC_NXREMINDER_LEFTMESSAGE_CHECK, BST_CHECKED);
		}
		// (z.manning 2010-11-17 16:34) - PLID 41495
		if(GetRemotePropertyInt("ApptReminderEnableConfirm", 1, 0, "<None>") == 1) {
			CheckDlgButton(IDC_NXREMINDER_CONFIRM_CHECK, BST_CHECKED);
		}
		long nConfirmOption = GetRemotePropertyInt("ApptReminderConfirmOption", 1, 0, "<None>");
		switch(nConfirmOption)
		{
			default:
				ASSERT(FALSE);
			case 1:
				CheckDlgButton(IDC_NXREMINDER_CONFIRM_ALL, BST_CHECKED);
				break;

			case 2:
				CheckDlgButton(IDC_NXREMINDER_CONFIRM_LATEST, BST_CHECKED);
				break;
		}

		// (z.manning 2011-11-02 10:09) - PLID 46220
		m_AdvancedSettings.strLeadingDigitToTrim = GetRemotePropertyText("ApptReminderLeadingDigitToTruncate", "", 0);

		// (z.manning 2010-11-19 17:16) - PLID 41560 - Try and load the account nickname from the reminder server, but
		// silently ignore any exceptions in case they don't have internet access or something else is wrong. This is
		// only here for their reference so if it's missing it won't cause any problems.
		// (z.manning 2010-11-30 15:06) - PLID 41560 - Scratch that, this is no longer relevant to the user.
		/*CNxRetrieveNxReminderClient nxremserver(AsString((long)g_pLicense->GetLicenseKey()));
		CString strAccount;
		try {
			strAccount = nxremserver.GetNicknameFromServer();
		}NxCatchAllIgnore();
		SetDlgItemText(IDC_NXREMINDER_ACCOUNT_NAME, strAccount);*/

		// (r.gonet 2010-06-23) - PLID 39012 - Show the schedule button.
		GetDlgItem(IDC_BTN_APPT_REMINDER_SCHED)->ShowWindow(SW_SHOW);

		///////// Update the controls and ensure that they are read-only or otherwise /////////
		UpdateData(FALSE);
		UpdateControls();

		UpdateMessageFormatLength();

		// Initialize the schedule
		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Combined loading into one function.
		m_arsSchedule.Load();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAppointmentReminderSetupDlg::OnBnClickedRadioOn()
{
	try {
		// (r.gonet 01-11-2011) - PLID 42010 - We need to display to them the text message policy before they turn this on
		if(!m_bNexReminderEnabled) {
			CTxtMsgPolicyDlg dlg(this);
			if(dlg.DoModal() == IDCANCEL) {
				m_radioOn.SetCheck(FALSE);
				m_radioOff.SetCheck(TRUE);			
			} else {
				m_bNexReminderEnabled = TRUE;
				m_bPolicyDlgCompleted = TRUE;
			}
		}
		// Update the controls (ensure that they are read-only or otherwise)
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}

void CAppointmentReminderSetupDlg::OnBnClickedRadioOff()
{
	try {
		m_bNexReminderEnabled = FALSE;
		// Update the controls (ensure that they are read-only or otherwise)
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}

static void PopulateAppointmentReminderFields(LPDISPATCH lpFieldsList)
{
	NXDATALISTLib::_DNxDataListPtr pdlFields(lpFieldsList);
	CNxFolderFieldArray astrFields;

	// (z.manning 2010-11-23 12:35) - PLID 41560 - Make the confirm call to action keyword an available field
	astrFields.Add(CNxFolderField("[Confirm]", "Confirmation Reply Keyword"));

	// Patient fields
	astrFields.Add(CNxFolderField("[First]", "First Name"));
	astrFields.Add(CNxFolderField("[Last]", "Last Name"));
	astrFields.Add(CNxFolderField("[Middle]", "Middle Name"));
	astrFields.Add(CNxFolderField("[MiddleInitial]", "Middle Initial"));

	astrFields.Add(CNxFolderField("[Date]", "Appointment Date"));
	// (z.manning 2013-04-19 09:30) - PLID 54425 - Added arrival time
	astrFields.Add(CNxFolderField("[ArrivalTime]", "Appointment Arrival Time"));
	astrFields.Add(CNxFolderField("[StartTime]", "Appointment Start Time"));
	astrFields.Add(CNxFolderField("[EndTime]", "Appointment End Time"));
	astrFields.Add(CNxFolderField("[Duration]", "Appointment Duration"));
	astrFields.Add(CNxFolderField("[Type]", "Appointment Type"));
	astrFields.Add(CNxFolderField("[Purposes]", "Appointment Purposes"));
	astrFields.Add(CNxFolderField("[Resources]", "Appointment Resources"));
	astrFields.Add(CNxFolderField("[Notes]", "Appointment Notes"));

	astrFields.Add(CNxFolderField("[Location]", "Appointment Location"));
	astrFields.Add(CNxFolderField("[Address1]", "Location Address 1"));
	astrFields.Add(CNxFolderField("[Address2]", "Location Address 2"));
	astrFields.Add(CNxFolderField("[City]", "Location City"));
	astrFields.Add(CNxFolderField("[State]", "Location State"));
	astrFields.Add(CNxFolderField("[Zip]", "Location Zip"));
	astrFields.Add(CNxFolderField("[Website]", "Website"));
	astrFields.Add(CNxFolderField("[Phone]", "Main Phone"));
	astrFields.Add(CNxFolderField("[Phone2]", "Alternate Phone"));
	astrFields.Add(CNxFolderField("[TollFreePhone]", "Toll-Free Phone"));
	astrFields.Add(CNxFolderField("[OnLineAddress]", "E-Mail"));

	for (long i=0; i < astrFields.GetSize(); i++)
	{
		NXDATALISTLib::IRowSettingsPtr pRow = pdlFields->GetRow(-1);
		pRow->Value[NxOutlookUtils::flcCheckbox] = (long)0;
		pRow->Value[NxOutlookUtils::flcField] = (LPCTSTR)astrFields[i].GetField();
		pRow->Value[NxOutlookUtils::flcAlias] = (LPCTSTR)astrFields[i].GetAlias();
		pdlFields->AddRow(pRow);
	}
	pdlFields->Sort();
}

void TryInsertOptionalLiteral(const CString& strPrevFieldValue, const CString& strFieldValue, const CString& strLiteral, CString& strText);
static void GenerateSubjectLinePreview(CEdit *peditSubject, CEdit *peditSubjectPreview)
{
	//
	// This code updates the preview text contained in the 
	// picture of the Outlook appointment
	//
	CString strScript;
	CString strSubject;
	CString strItem;
	BOOL bInBrackets = FALSE;
	BOOL bInDoubleBrackets = FALSE;
	peditSubject->GetWindowText(strScript);
	long n = 0;

	// Variables used for double-bracket operations
	CString strPrevFieldValue;
	CString strDoubleBracketedItem;
	while (n < strScript.GetLength())
	{
		if (strScript.GetAt(n) == '[') {
			if (bInBrackets)
			{
				strDoubleBracketedItem.Empty();
				bInDoubleBrackets = TRUE;
			}
			else
			{
				strItem.Empty();
				bInBrackets = TRUE;
			}
		}
		else if (strScript.GetAt(n) == ']') {
			if (bInDoubleBrackets)
			{
				bInDoubleBrackets = FALSE;
			}
			else if (bInBrackets)
			{
				if (strItem.GetLength())
				{
					CString strFieldValue;

					if (strItem == "Last")
						strFieldValue = "Doe";
					else if (strItem == "First")
						strFieldValue = "Jane";
					else if (strItem == "Middle")
						strFieldValue = "Quindor";
					else if (strItem == "MiddleInitial")
						strFieldValue = "Q";

					else if (strItem == "Date")
						strFieldValue = "3/5/2010";
					// (z.manning 2013-04-19 09:33) - PLID 54425 - Added arrival time
					else if (strItem == "ArrivalTime") {
						COleDateTime dt(2013,3,5, 10,30,0);
						strFieldValue = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
					}
					else if (strItem == "StartTime") {
						COleDateTime dt(2013,3,5, 11,00,0);
						strFieldValue = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
					}
					else if (strItem == "EndTime") {
						COleDateTime dt(2013,3,5, 12,30,0);
						strFieldValue = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
					}
					else if (strItem == "Duration")
						strFieldValue = "1:30";
					else if (strItem == "Type")
						strFieldValue = "Surgery";
					else if (strItem == "Purposes")
						strFieldValue = "Breast Augmentation";
					else if (strItem == "Resources")
						strFieldValue = "Dr. Smith, Dr. Penn";
					else if (strItem == "Notes")
						strFieldValue = "Treat pt with care";

					else if (strItem == "Location")
						strFieldValue = "Pawtuckett Plastic Surgeons";
					else if (strItem == "Address1")
						strFieldValue = "1850 Bluegrass Avenue";
					else if (strItem == "Address2")
						strFieldValue = "Ste. 210";
					else if (strItem == "City")
						strFieldValue = "Louisville";
					else if (strItem == "State")
						strFieldValue = "KY";
					else if (strItem == "Zip")
						strFieldValue = "40215";
					else if (strItem == "Website")
						strFieldValue = "yournewimage.internet.com";
					else if (strItem == "Phone")
						strFieldValue = "(555) 555-3494";
					else if (strItem == "Phone2")
						strFieldValue = "(555) 555-4983";
					else if (strItem == "TollFreePhone")
						strFieldValue = "(800) 555-2381";
					else if (strItem == "OnLineAddress")
						strFieldValue = "jdoe@internet.com";
					// (z.manning 2014-08-11 11:17) - PLID 62169 - Handle the confirm keyword in the preview
					else if (strItem == "Confirm")
						strFieldValue = "CONFIRMED";

					TryInsertOptionalLiteral(strPrevFieldValue, strFieldValue, strDoubleBracketedItem, strSubject);
					strDoubleBracketedItem.Empty();

					strSubject += strFieldValue;
					strPrevFieldValue = strFieldValue;
				}
				bInBrackets = FALSE;
			}				
		}
		else {
			if (bInDoubleBrackets)
				strDoubleBracketedItem.Insert(strDoubleBracketedItem.GetLength(), strScript.GetAt(n));
			else if (bInBrackets)
				strItem.Insert(strItem.GetLength(), strScript.GetAt(n));
			else
			{
				TryInsertOptionalLiteral(strPrevFieldValue, "", strDoubleBracketedItem, strSubject);
				strDoubleBracketedItem.Empty();
				strPrevFieldValue.Empty();
				strSubject.Insert(strSubject.GetLength(), strScript.GetAt(n));
			}
		}
		n++;
	}
	TryInsertOptionalLiteral(strPrevFieldValue, "", strDoubleBracketedItem, strSubject);
	peditSubjectPreview->SetWindowText(strSubject);
}

static void HandleSubjectLineEditChange(CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList)
{
	//
	// Update the fields checklist
	//
	NXDATALISTLib::_DNxDataListPtr pdlFields(lpFieldsList);
	CString strSubject;
	peditSubject->GetWindowText(strSubject);

	for (long i=0; i < pdlFields->GetRowCount(); i++)
	{
		CString strField(pdlFields->GetValue(i, NxOutlookUtils::flcField).bstrVal);
		COleVariant vCheckbox = pdlFields->GetValue(i, NxOutlookUtils::flcCheckbox);

		if (-1 == strSubject.Find(strField, 0))
		{
			if (vCheckbox.boolVal)
			{
				vCheckbox.boolVal = FALSE;
				pdlFields->PutValue(i, NxOutlookUtils::flcCheckbox, vCheckbox);
			}
		}
		else
		{
			if (!vCheckbox.boolVal)
			{
				vCheckbox.boolVal = TRUE;
				pdlFields->PutValue(i, NxOutlookUtils::flcCheckbox, vCheckbox);
			}
		}
	}

	//
	// Update the preview text contained in the appointment example box
	//
	GenerateSubjectLinePreview(peditSubject, peditSubjectPreview);
}

static void HandleSubjectLineFieldsListEditingFinished(CDialog *pdlg, CEdit *peditSubject, CEdit *peditSubjectPreview, LPDISPATCH lpFieldsList, long nRow, _variant_t varNewValue, IN OUT CString &strSubject)
{
	NXDATALISTLib::_DNxDataListPtr pdlFields(lpFieldsList);
	if (varNewValue.boolVal)
	{
		pdlg->UpdateData();
		strSubject += CString(" ") + CString(pdlFields->GetValue(nRow, NxOutlookUtils::flcField).bstrVal);
	}
	else
	{
		strSubject.Replace( CString(pdlFields->GetValue(nRow, NxOutlookUtils::flcField).bstrVal), "" );
		strSubject.TrimRight();
	}
	// (z.manning 2009-10-20 09:18) - PLID 35997 - The subject line cannot be more than 255 characters.
	if(strSubject.GetLength() > 255) {
		strSubject = strSubject.Left(255);
	}

	pdlg->UpdateData(FALSE);

	//
	// Update the preview text contained in the picture of the
	// Outlook appointment
	//
	GenerateSubjectLinePreview(peditSubject, peditSubjectPreview);
}

void CAppointmentReminderSetupDlg::OnBnClickedBtnArEditFormat()
{
	try {
		// Use the CNexSyncEditSubjectLineDlg for formatting. It already does all the work for us
		// (granted it was designed for NexSync); I changed it to be just generic enough to support
		// this dialog, too.
		CNexSyncEditSubjectLineDlg dlg(this);
		dlg.SetDefaultWindowText("Reminder Notification Format");
		dlg.SetDefaultDescription("Please select the fields that you would like to appear in the text message to be sent to the patient's phone.");
		dlg.SetDefaultSampleText("Message Preview");
		dlg.SetDefaultSubjectText("Message Format");
		dlg.SetSubjectLineListCallbackFunc(PopulateAppointmentReminderFields);
		dlg.SetHandleSubjectLineEditChangeCallbackFunc(HandleSubjectLineEditChange);
		dlg.SetHandleSubjectLineFieldsListEditingFinishedCallbackFunc(HandleSubjectLineFieldsListEditingFinished);

		UpdateData();
		dlg.SetSubject(m_strMsgFormat);
		if(dlg.DoModal() == IDOK) {
			m_strMsgFormat = dlg.GetSubject();
			UpdateData(FALSE);
			UpdateMessageFormatLength();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CAppointmentReminderSetupDlg::OnBnClickedOk()
{
	try {
		// Have our member fields reflect the form content
		UpdateData();

		// Validation
		if (!ValidateData()) {
			return;
		}

		// Save and close
		SaveData();
		CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-04 14:50) - PLID 38492
void CAppointmentReminderSetupDlg::UpdateMessageFormatLength()
{
	CString strMessageFormat;
	GetDlgItemText(IDC_EDIT_AR_FORMAT, strMessageFormat);
	m_nxstaticFormatLength.SetWindowText(FormatString("%d / 160", strMessageFormat.GetLength()));
}

// (z.manning 2010-05-04 14:57) - PLID 38492
void CAppointmentReminderSetupDlg::OnEnChangeEditArFormat()
{
	try
	{
		UpdateMessageFormatLength();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-04 15:12) - PLID 38492
HBRUSH CAppointmentReminderSetupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	try
	{
		HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

		if(pWnd == &m_nxstaticFormatLength) {
			CString strMessageFormat;
			GetDlgItemText(IDC_EDIT_AR_FORMAT, strMessageFormat);
			if(strMessageFormat.GetLength() > 160) {
				pDC->SetTextColor(RGB(255,0,0));
			}
		}

		return hbr;

	}NxCatchAll(__FUNCTION__);
	return NULL;
}

// (r.gonet 2010-06-25) - PLID 39012 - Open the appointment reminder schedule editor dialog to edit our
//  loaded schedule.
void CAppointmentReminderSetupDlg::OnBnClickedBtnApptReminderSched()
{
	try {
		// (r.gonet 2010-06-25) - PLID 39012 - Pass in the loaded schedule by reference so that the editor can work on it.
		CApptReminderSchedDlg dlg(m_arsSchedule, this);
		dlg.DoModal();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-11-17 16:08) - PLID 41495
void CAppointmentReminderSetupDlg::OnBnClickedNxreminderConfirmCheck()
{
	try
	{
		UpdateControls();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 01-11-2011) - PLID 42010 - Let the user view the policy at any time
void CAppointmentReminderSetupDlg::OnBnClickedNxreminderTxtMsgPolicyBtn()
{
	try {
		if(m_radioOn.GetCheck()) {
			CTxtMsgPolicyDlg dlg(this);
			dlg.ShowLeaveAsIsButton(FALSE);
			dlg.DoModal();	
		}
	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-11-02 09:49) - PLID 46220
void CAppointmentReminderSetupDlg::OnBnClickedNxreminderAdvanced()
{
	try
	{
		CAppointmentReminderAdvancedDlg dlg(&m_AdvancedSettings, this);
		dlg.DoModal();
	}
	NxCatchAll(__FUNCTION__);
}
