// (r.gonet 2010-06-23) - PLID 40574 - Created. All comments, unless marked otherwise, are for this PLID.
// ApptReminderScheduleRuleDlg.cpp : implementation file

#include "stdafx.h"
#include "PracticeRc.h"
#include "MultiSelectDlg.h"
#include "ApptReminderScheduleRuleDlg.h"
#include "ApptReminderScheduleRule.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CApptReminderScheduleRuleDlg dialog

IMPLEMENT_DYNAMIC(CApptReminderScheduleRuleDlg, CNxDialog)

CApptReminderScheduleRuleDlg::CApptReminderScheduleRuleDlg(CApptReminderScheduleRule &arrRule, CWnd* pParent /*=NULL*/)
	: m_arrRule(arrRule), CNxDialog(CApptReminderScheduleRuleDlg::IDD, pParent)
{
	m_dtOriginalSendTime = arrRule.GetSendTime();
}

CApptReminderScheduleRuleDlg::~CApptReminderScheduleRuleDlg()
{
}

void CApptReminderScheduleRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_RULE_IGNOREEX, m_chbIgnoreExceptions);
	// (r.gonet 2010-06-23) - PLID 39282 - Appointment types and resource filtering
	DDX_Control(pDX, IDC_APPT_RMNDR_RULE_TYPE_CB, m_chbFilterOnTypes);
	DDX_Control(pDX, IDC_APPT_RMNDR_RULE_RES_CB, m_chbFilterOnResources);
	DDX_Control(pDX, IDC_APPT_RMNDR_RULE_TYPES_BTN, m_btnEditTypes);
	DDX_Control(pDX, IDC_APPT_RMNDR_RULE_RES_BTN, m_btnEditResources);
	DDX_Control(pDX, IDC_APPT_RMNDR_RULE_TYPES_EDIT, m_nxeditTypes);
	DDX_Control(pDX, IDC_APPT_RMNDR_RULE_RES_EDIT, m_nxeditResources);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_APPTREMINDER_RULE_COLOR, m_nxcolorRule);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_RULE_DAYS, m_nxeditDays);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_RULE_HOURS, m_nxeditHours);
}

BOOL CApptReminderScheduleRuleDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (r.gonet 2010-06-23) - PLID 39282
		m_btnEditTypes.AutoSet(NXB_MODIFY);
		m_btnEditResources.AutoSet(NXB_MODIFY);

	    // (r.gonet 2010-06-23) - PLID 39282 - This isn't meant to be foolproof, but this should deter entering more than what we can handle.
		m_nxeditDays.SetLimitText(3);
		m_nxeditHours.SetLimitText(4);
		((CEdit*)GetDlgItem(IDC_APPT_REMINDER_SEND_TIME_DAYS_BEFORE))->SetLimitText(3);

		m_nxtSendTime = BindNxTimeCtrl(this, IDC_APPT_REMINDER_SEND_TIME);

		// Set the background colors
		m_nxcolorRule.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		
		// Fill in the checkboxes from our rule
		LoadControls();
		// (r.gonet 2010-06-23) - PLID 39282 - Put the controls into a valid state
		UpdateControls();
		return TRUE;
	}
	NxCatchAll(__FUNCTION__);

	// Return FALSE because we set the focus, we don't want to framework to set it to something else
	return FALSE;
}

BEGIN_MESSAGE_MAP(CApptReminderScheduleRuleDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CApptReminderScheduleRuleDlg::OnBnClickedOk)
	ON_EN_KILLFOCUS(IDC_APPT_RMNDR_SCHED_RULE_HOURS, &CApptReminderScheduleRuleDlg::OnEnKillfocusApptRmndrSchedRuleHours)
	ON_BN_CLICKED(IDC_APPT_RMNDR_RULE_TYPES_BTN, &CApptReminderScheduleRuleDlg::OnBnClickedEditApptTypes)
	ON_BN_CLICKED(IDC_APPT_RMNDR_RULE_RES_BTN, &CApptReminderScheduleRuleDlg::OnBnClickedEditApptResources)
	ON_BN_CLICKED(IDC_APPT_RMNDR_RULE_TYPE_CB, &CApptReminderScheduleRuleDlg::OnBnClickedFilterOnApptTypes)
	ON_BN_CLICKED(IDC_APPT_RMNDR_RULE_RES_CB, &CApptReminderScheduleRuleDlg::OnBnClickedFilterOnApptResources)
	ON_BN_CLICKED(IDC_APPT_REMINDER_BEFORE_RADIO, &CApptReminderScheduleRuleDlg::OnBnClickedApptReminderBeforeRadio)
	ON_BN_CLICKED(IDC_APPT_REMINDER_SPECIFIC_TIME_RADIO, &CApptReminderScheduleRuleDlg::OnBnClickedApptReminderSpecificTimeRadio)
END_MESSAGE_MAP()

void CApptReminderScheduleRuleDlg::OnBnClickedOk()
{
	try {
		if(!ValidateRule()) {
			return;
		}
		Save();
	} NxCatchAll(__FUNCTION__);
	CNxDialog::OnOK();
}

// Save the controls' values to the rule object
void CApptReminderScheduleRuleDlg::Save()
{
	// Get all of the filter values from the dialog
	CString strNumHours, strNumDays;
	this->GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_HOURS)->GetWindowText(strNumHours);
	this->GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_DAYS)->GetWindowText(strNumDays);
	long nNumHours = atoi(strNumHours);
	long nNumDays = atoi(strNumDays);
	BOOL bIgnoreExceptions = (m_chbIgnoreExceptions.GetCheck() ? TRUE : FALSE);
	BOOL bFilterOnTypes = (m_chbFilterOnTypes.GetCheck() && m_arynAppointmentTypes.GetCount() > 0 ? TRUE : FALSE);
	BOOL bFilterOnResources = (m_chbFilterOnResources.GetCheck() && m_arynAppointmentResources.GetCount() > 0 ? TRUE : FALSE);

	// Now fill the rule object with these values
	COleDateTimeSpan dtsAdvanceNotice;
	dtsAdvanceNotice.SetDateTimeSpan(nNumDays, nNumHours, 0, 0);
	m_arrRule.SetAdvanceNotice(dtsAdvanceNotice);
	m_arrRule.SetIgnoreExceptions(bIgnoreExceptions);
	
	// (r.gonet 2010-06-23) - PLID 39282 - Save the appointment types and resources
	if(bFilterOnTypes) {
		m_arrRule.SetAppointmentTypes(m_arynAppointmentTypes);
		m_arrRule.SetFilterOnAllTypes(FALSE);
	} else {
		m_arrRule.RemoveAllAppointmentTypes();
		m_arrRule.SetFilterOnAllTypes(TRUE);
	}
	if(bFilterOnResources) {
		m_arrRule.SetAppointmentResources(m_arynAppointmentResources);
		m_arrRule.SetFilterOnAllResources(FALSE);
	} else {
		m_arrRule.RemoveAllAppointmentResources();
		m_arrRule.SetFilterOnAllResources(TRUE);
	}

	// (z.manning 2011-05-16 12:30) - PLID 43708 - Added more controls to support different sending methods
	CApptReminderScheduleRule::ESendType eSendType = CApptReminderScheduleRule::stHoursBeforeAppt;
	if(IsDlgButtonChecked(IDC_APPT_REMINDER_SPECIFIC_TIME_RADIO) == BST_CHECKED) {
		eSendType = CApptReminderScheduleRule::stSpecificTime;
	}
	m_arrRule.SetSendType(eSendType);
	m_arrRule.SetSendTimeDaysBefore((short)GetDlgItemInt(IDC_APPT_REMINDER_SEND_TIME_DAYS_BEFORE));
	COleDateTime dtSendTime = m_dtOriginalSendTime;
	if(m_nxtSendTime->GetStatus() == 1) {
		dtSendTime = m_nxtSendTime->GetDateTime();
		dtSendTime.SetTime(dtSendTime.GetHour(), dtSendTime.GetMinute(), dtSendTime.GetSecond());
	}
	m_arrRule.SetSendTime(dtSendTime);
}

// Load the values from the rule into the controls on the screen
void CApptReminderScheduleRuleDlg::LoadControls()
{
	// We need to break down the reminder send time into days and hours. 140 hours is hard to fathom, but 5 days, 20 hours is easy.
	COleDateTimeSpan dtsAdvanceNotice = m_arrRule.GetAdvanceNotice();
	CString strNumHours = FormatString("%d", dtsAdvanceNotice.GetHours());
	CString strNumDays = FormatString("%d", dtsAdvanceNotice.GetDays());
	// Whether this rule overrides all exceptions. Could be an important rule.
	BOOL bIgnoreExceptions = m_arrRule.GetIgnoreExceptions();
	
	this->SetDlgItemText(IDC_APPT_RMNDR_SCHED_RULE_HOURS, strNumHours);
	this->SetDlgItemText(IDC_APPT_RMNDR_SCHED_RULE_DAYS, strNumDays);	
	m_chbIgnoreExceptions.SetCheck(bIgnoreExceptions);
	// (z.manning 2011-05-16 12:05) - PLID 43708 - Added more fields as we now support different ways to send reminders.
	if(m_arrRule.GetSendType() == CApptReminderScheduleRule::stSpecificTime) {
		CheckDlgButton(IDC_APPT_REMINDER_SPECIFIC_TIME_RADIO, BST_CHECKED);
	}
	else {
		ASSERT(m_arrRule.GetSendType() == CApptReminderScheduleRule::stHoursBeforeAppt);
		CheckDlgButton(IDC_APPT_REMINDER_BEFORE_RADIO, BST_CHECKED);
	}
	m_nxtSendTime->SetDateTime(m_arrRule.GetSendTime());
	SetDlgItemInt(IDC_APPT_REMINDER_SEND_TIME_DAYS_BEFORE, m_arrRule.GetSendTimeDaysBefore());

	// (r.gonet 2010-06-23) - PLID 39282 - Load the appointment types and resources into the textboxes as additional filters for sending messages.
	LoadAppointmentTypes();
	LoadAppointmentResources();
}

// (r.gonet 2010-06-23) - PLID 39282 - Make the controls into a valid state
void CApptReminderScheduleRuleDlg::UpdateControls()
{
	if(m_chbFilterOnTypes.GetCheck() != 0) {
		m_btnEditTypes.EnableWindow(TRUE);
	} else {
		m_btnEditTypes.EnableWindow(FALSE);
	}

	if(m_chbFilterOnResources.GetCheck() != 0) {
		m_btnEditResources.EnableWindow(TRUE);
	} else {
		m_btnEditResources.EnableWindow(FALSE);
	}

	// (z.manning 2011-05-16 10:27) - PLID 43708 - Handle the different types of sending options.
	BOOL bSendAtSpecificTime = (IsDlgButtonChecked(IDC_APPT_REMINDER_SPECIFIC_TIME_RADIO) == BST_CHECKED);
	GetDlgItem(IDC_APPT_REMINDER_SEND_TIME)->EnableWindow(bSendAtSpecificTime);
	GetDlgItem(IDC_APPT_REMINDER_SEND_TIME_DAYS_BEFORE)->EnableWindow(bSendAtSpecificTime);
	GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_DAYS)->EnableWindow(!bSendAtSpecificTime);
	GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_HOURS)->EnableWindow(!bSendAtSpecificTime);
}

// (r.gonet 2010-06-23) - PLID 39282 - Using the type ids stored in the rule and the names from the database, fill in the appointment types filter textbox
void CApptReminderScheduleRuleDlg::LoadAppointmentTypes()
{
	CString strTypeNames;
	m_arynAppointmentTypes.RemoveAll();
	m_arrRule.GetAppointmentTypes(m_arynAppointmentTypes);
	
	if(m_arynAppointmentTypes.GetCount() > 0 && !m_arrRule.GetFilterOnAllTypes()) {
		// (r.gonet 2010-06-23) - PLID 39282 - Join all of the ids together
		CString strTypeIDs;
		for(int i = 0; i < m_arynAppointmentTypes.GetCount(); i++) {
			long nTypeID = m_arynAppointmentTypes[i];
			if(nTypeID == 0) {
				// (r.gonet 2010-06-23) - PLID 39282 - We already know that 0 is no type, so add it here
				strTypeNames += "{No Type}; ";
			}
			strTypeIDs += FormatString("%li, ", nTypeID); 
		}
		strTypeIDs = strTypeIDs.Left(strTypeIDs.GetLength() - 2);
		
		// (r.gonet 2010-06-23) - PLID 39282 - We need to get the type names for the type ids and join all of the names together
		_RecordsetPtr prs = CreateRecordset(GetRemoteData(), "SELECT Name from AptTypeT WHERE ID IN (%s) ORDER BY Name; ", strTypeIDs);
		while(!prs->eof) {
			CString strName = AdoFldString(prs, "Name");
			strTypeNames += FormatString("%s; ", strName);
			prs->MoveNext();
		}
		prs->Close();
		if(!strTypeNames.IsEmpty()) {
			strTypeNames = strTypeNames.Left(strTypeNames.GetLength() - 2);
		}
		m_nxeditTypes.SetWindowText(strTypeNames);
		m_chbFilterOnTypes.SetCheck(!strTypeNames.IsEmpty());
	}
}

// (r.gonet 2010-06-23) - PLID 39282 - Using the resource ids stored in the rule and the names from the database, fill in the appointment resources filter textbox
void CApptReminderScheduleRuleDlg::LoadAppointmentResources()
{
	CString strResourceNames;
	m_arynAppointmentResources.RemoveAll();
	m_arrRule.GetAppointmentResources(m_arynAppointmentResources);
	
	if(m_arynAppointmentResources.GetCount() > 0 && !m_arrRule.GetFilterOnAllResources()) {
		// (r.gonet 2010-06-23) - PLID 39282 - Join all of the ids together
		CString strResourceIDs;
		for(int i = 0; i < m_arynAppointmentResources.GetCount(); i++) {
			long nResourceID = m_arynAppointmentResources[i];
			strResourceIDs += FormatString("%li, ", nResourceID); 
		}
		strResourceIDs = strResourceIDs.Left(strResourceIDs.GetLength() - 2);
		
		// (r.gonet 2010-06-23) - PLID 39282 - We need to get the resource names for the resource ids and join all of the names together
		_RecordsetPtr prs = CreateRecordset(GetRemoteData(), "SELECT Item from ResourceT WHERE ID IN (%s) ORDER BY Item; ", strResourceIDs);
		while(!prs->eof) {
			CString strName = AdoFldString(prs, "Item");
			strResourceNames += FormatString("%s; ", strName);
			prs->MoveNext();
		}
		prs->Close();
		if(!strResourceNames.IsEmpty()) {
			strResourceNames = strResourceNames.Left(strResourceNames.GetLength() - 2);
		}
		m_nxeditResources.SetWindowText(strResourceNames);
		m_chbFilterOnResources.SetCheck(!strResourceNames.IsEmpty());
	}
}

// Check to see if the rule entered by the user contains valid values. Return TRUE iff it is valid.
BOOL CApptReminderScheduleRuleDlg::ValidateRule()
{
	CString strNumHours, strNumDays;
	this->GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_HOURS)->GetWindowText(strNumHours);
	this->GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_DAYS)->GetWindowText(strNumDays);
	long nNumHours = atoi(strNumHours);
	long nNumDays = atoi(strNumDays);

	// (z.manning 2011-05-16 12:25) - PLID 43708 - We now support different methods of sending reminders so
	// let's only validate based on what the user selected.
	if(IsDlgButtonChecked(IDC_APPT_REMINDER_SPECIFIC_TIME_RADIO) == BST_CHECKED)
	{
		BOOL bTimeValid = FALSE;
		if(m_nxtSendTime->GetStatus() == 1) {
			COleDateTime dtSendTime = m_nxtSendTime->GetDateTime();
			if(dtSendTime.GetStatus() == COleDateTime::valid) {
				bTimeValid = TRUE;
			}
		}
		if(!bTimeValid) {
			MessageBox("Please enter a valid time to send reminders.", NULL, MB_ICONERROR);
			return FALSE;
		}

		CString strSendDaysBefore;
		GetDlgItemText(IDC_APPT_REMINDER_SEND_TIME_DAYS_BEFORE, strSendDaysBefore);
		if(strSendDaysBefore.IsEmpty()) {
			MessageBox("Please enter a valid number of days.", NULL, MB_ICONERROR);
			return FALSE;
		}
	}
	else
	{
		if(nNumHours < 0) {
			MessageBox("The number of hours cannot be negative.", "Error", MB_ICONHAND);
			return FALSE;
		}

		if(nNumDays < 0) {
			MessageBox("The number of days cannot be negative.", "Error", MB_ICONHAND);
			return FALSE;
		}

		// This retains the valid range of 0 < x <= 5000
		if(24 * nNumDays + nNumHours > 5000) {
			AfxMessageBox("You may not send notifications more than 5,000 hours ahead of their respective appointments.", MB_OK | MB_ICONHAND);
			return FALSE;
		}

		// 0 days and 0 hours is questionable, as it is useless I think, I'm forbidding it here and now. This was also the case with the original implementation.
		if(nNumHours == 0 && nNumDays == 0) { 
			MessageBox("You must configure notifications to be sent no less than one hour before their respective appointments.", "Error", MB_ICONHAND);
			return FALSE;
		}
	}

	return TRUE;
}

// If the hours are >= 24, make them days instead
void CApptReminderScheduleRuleDlg::OnEnKillfocusApptRmndrSchedRuleHours()
{
	try {
		CString strNumHours, strNumDays;
		this->GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_HOURS)->GetWindowText(strNumHours);
		this->GetDlgItem(IDC_APPT_RMNDR_SCHED_RULE_DAYS)->GetWindowText(strNumDays);

		long nNumHours = atoi(strNumHours);
		long nNumDays = atoi(strNumDays);
		long nDaysFromHours = nNumHours / 24;
		if(nDaysFromHours > 0) {
			nNumHours = nNumHours % 24;
			nNumDays = nNumDays + nDaysFromHours;
			strNumHours.Format("%d", nNumHours);
			strNumDays.Format("%d", nNumDays);
			this->SetDlgItemText(IDC_APPT_RMNDR_SCHED_RULE_HOURS, _bstr_t(strNumHours));
			this->SetDlgItemText(IDC_APPT_RMNDR_SCHED_RULE_DAYS, _bstr_t(strNumDays));
		}

	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2010-06-23) - PLID 39282 - Let the user select from a list of appointment types to filter on
void CApptReminderScheduleRuleDlg::OnBnClickedEditApptTypes()
{
	try {
		// (r.gonet 2010-06-23) - PLID 39282 - Create a multi select dialog with all of the appointment types in it
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptTypeT");
		dlg.PreSelect(m_arynAppointmentTypes);

		// (r.gonet 2010-06-23) - PLID 39282 - Add in the no type as -1
		CString strFrom =
			"( \r\n"
			"	SELECT 0 AS ID, ' { No Type }' AS Name \r\n"
			"	UNION \r\n"
			"	SELECT ID, Name FROM AptTypeT WHERE Inactive = 0 \r\n"
			") AptTypesQ \r\n";
		int nResult = dlg.Open(strFrom, "", "ID", "Name", "Select the appointment types you would like to apply this rule to. "
			"An appointment that has a type selected here would be matched by this filter.", 1);
		if(nResult == IDOK)
		{
			// (r.gonet 2010-06-23) - PLID 39282 - User selected something, so update our filter.
			m_arynAppointmentTypes.RemoveAll();
			dlg.FillArrayWithIDs(m_arynAppointmentTypes);
			CString strSelections = dlg.GetMultiSelectString("; ");
			m_nxeditTypes.SetWindowText(strSelections);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2010-06-23) - PLID 39282 - Let the user select from a list of resources to filter on
void CApptReminderScheduleRuleDlg::OnBnClickedEditApptResources()
{
	try {		
		// (r.gonet 2010-06-23) - PLID 39282 - Create a multi select dialog with all of the appointment resources in it
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ResourceT");
		dlg.PreSelect(m_arynAppointmentResources);

		CString strFrom =
			"( \r\n"
			"	SELECT ID, Item FROM ResourceT WHERE Inactive = 0 \r\n"
			") AptResourcesQ \r\n";
		int nResult = dlg.Open(strFrom, "", "ID", "Item", "Select the appointment resources you would like to apply this rule to. "
			"An appointment would have to have at least one of these resources to be matched by this filter.", 1);
		if(nResult == IDOK)
		{
			// (r.gonet 2010-06-23) - PLID 39282 - User selected something, so update our filter.
			m_arynAppointmentResources.RemoveAll();
			dlg.FillArrayWithIDs(m_arynAppointmentResources);
			CString strSelections = dlg.GetMultiSelectString("; ");
			m_nxeditResources.SetWindowText(strSelections);
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2010-06-23) - PLID 39282 - Enable or disable the edit appointment types button
void CApptReminderScheduleRuleDlg::OnBnClickedFilterOnApptTypes()
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2010-06-23) - PLID 39282 - Enable or disable the edit appointment resources button
void CApptReminderScheduleRuleDlg::OnBnClickedFilterOnApptResources()
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-05-16 10:21) - PLID 43708
void CApptReminderScheduleRuleDlg::OnBnClickedApptReminderBeforeRadio()
{
	try
	{
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-05-16 10:21) - PLID 43708
void CApptReminderScheduleRuleDlg::OnBnClickedApptReminderSpecificTimeRadio()
{
	try
	{
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);
}