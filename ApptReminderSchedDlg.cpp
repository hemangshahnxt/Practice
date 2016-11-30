// ApptReminderSchedDlg.cpp : implementation file
// (r.gonet 2010-06-23) - PLID 39012 - Created. All comments, unless marked otherwise, are for this PLID.

#include "stdafx.h"
#include "PracticeRc.h"
#include "ApptReminderSchedDlg.h"
#include "ApptReminderScheduleExceptionDlg.h"
#include "ApptReminderScheduleRuleDlg.h"
#include "ApptReminderScheduleRule.h"

using namespace ADODB;

// CApptReminderSchedDlg dialog

IMPLEMENT_DYNAMIC(CApptReminderSchedDlg, CNxDialog)

// Create a new appointment schedule dialog with an initialized or uninitialized schedule
CApptReminderSchedDlg::CApptReminderSchedDlg(CApptReminderSchedule &arsSchedule, CWnd* pParent /*=NULL*/)
	: m_arsSchedule(arsSchedule), CNxDialog(CApptReminderSchedDlg::IDD, pParent)

{
}

CApptReminderSchedDlg::~CApptReminderSchedDlg()
{
}

void CApptReminderSchedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APPT_SCHED_COLORBOX, m_nxcolorRules);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_EX_COLOR, m_nxcolorExceptions);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_COLOR, m_nxcolorSendingLimits);
	DDX_Control(pDX, IDC_ADD_APPTSCHED_RULE_BTN, m_btnAddRule);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_EDIT_RULE, m_btnEditRule);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_REMOVE_RULE, m_btnRemoveRule);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_ADD_EX, m_btnAddException);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_EDIT_EX, m_btnEditException);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_REMOVE_EX, m_btnRemoveException);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_RULES_HEADER, m_nxlblRulesHeader);
	DDX_Control(pDX, IDC_APPT_RMND_SCHED_EX_HEADER, m_nxlblExceptionsHeader);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_HEADER_LABEL, m_nxlblHeader);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_HEADER, m_nxlblSendingLimitsHeader);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_OPT1_RADIO, m_radioPerAppointmentPerRule);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_OPT2_RADIO, m_radioPerDayPatientAppointmentDate);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_RESOURCE_CHECK, m_checkAndResource);
	DDX_Control(pDX, IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_LOCATION_CHECK, m_checkAndLocation);
}

BOOL CApptReminderSchedDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		m_pRules = BindNxDataList2Ctrl(this, IDC_APPT_REMINDER_SCHED_RULES_DL, GetRemoteData(), false);
		m_pExceptions = BindNxDataList2Ctrl(this, IDC_APPT_REMINDER_SCHED_EXCEPTIONS_DL, GetRemoteData(), false);

		// Set the background colors
		m_nxcolorRules.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxcolorExceptions.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Init the color of the sending limits nxcolor.
		m_nxcolorSendingLimits.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// Set the button styles
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAddRule.AutoSet(NXB_NEW);
		m_btnAddException.AutoSet(NXB_NEW);
		m_btnEditRule.AutoSet(NXB_MODIFY);
		m_btnEditException.AutoSet(NXB_MODIFY);
		m_btnRemoveRule.AutoSet(NXB_DELETE);
		m_btnRemoveException.AutoSet(NXB_DELETE);
		
		// Reload the rules and exceptions datalists on the interface from what is in the in-memory schedule
		ReloadRulesList();
		ReloadExceptionsList();

		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Initialize the radio buttons and checkboxes for the sending limit options.
		if ((m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerRule) == CApptReminderSchedule::PerRule
			&& (m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerAppointment) == CApptReminderSchedule::PerAppointment) {
			m_radioPerAppointmentPerRule.SetCheck(BST_CHECKED);
		} else if ((m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerRule) == CApptReminderSchedule::PerRule
			&& (m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerPatient) == CApptReminderSchedule::PerPatient
			&& (m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerAppointmentDate) == CApptReminderSchedule::PerAppointmentDate) {
			m_radioPerDayPatientAppointmentDate.SetCheck(BST_CHECKED);

			if ((m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerResource) == CApptReminderSchedule::PerResource) {
				m_checkAndResource.SetCheck(BST_CHECKED);
			}
			if ((m_arsSchedule.GetSendingLimitOptions() & CApptReminderSchedule::PerLocation) == CApptReminderSchedule::PerLocation) {
				m_checkAndLocation.SetCheck(BST_CHECKED);
			}
		} else {
			// (r.gonet 2015-11-16 02:33) - PLID 67587 - Leaving the possibility for a custom setting...
			// Custom settings can be handled in NexReminderClient in case you are wondering.
		}

		UpdateControls();

		return TRUE;
	}
	NxCatchAll(__FUNCTION__);

	// Return FALSE because we set the focus, we don't want to framework to set it to something else
	return FALSE;
}


BEGIN_MESSAGE_MAP(CApptReminderSchedDlg, CNxDialog)
	ON_BN_CLICKED(IDC_APPT_RMND_SCHED_ADD_EX, &CApptReminderSchedDlg::OnBnClickedApptReminderAddExceptionBtn)
	ON_BN_CLICKED(IDC_APPT_RMND_SCHED_EDIT_EX, &CApptReminderSchedDlg::OnBnClickedApptReminderEditExceptionBtn)
	ON_BN_CLICKED(IDC_APPT_RMND_SCHED_REMOVE_EX, &CApptReminderSchedDlg::OnBnClickedApptReminderRemoveExceptionBtn)
	ON_BN_CLICKED(IDC_ADD_APPTSCHED_RULE_BTN, &CApptReminderSchedDlg::OnBnClickedAddApptSchedRuleBtn)
	ON_BN_CLICKED(IDC_APPT_RMND_SCHED_EDIT_RULE, &CApptReminderSchedDlg::OnBnClickedApptRmndSchedEditRule)
	ON_BN_CLICKED(IDC_APPT_RMND_SCHED_REMOVE_RULE, &CApptReminderSchedDlg::OnBnClickedRemoveApptSchedRuleBtn)
	ON_BN_CLICKED(IDC_RULESHELPBTN, &CApptReminderSchedDlg::OnBnClickedRuleshelpbtn)
	ON_BN_CLICKED(IDC_EXCEPTIONSHELPBTN, &CApptReminderSchedDlg::OnBnClickedExceptionshelpbtn)
	ON_BN_CLICKED(IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_OPT1_RADIO, &CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsOpt1Radio)
	ON_BN_CLICKED(IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_OPT2_RADIO, &CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsOpt2Radio)
	ON_BN_CLICKED(IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_RESOURCE_CHECK, &CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsResourceCheck)
	ON_BN_CLICKED(IDC_APPT_RMNDR_SCHED_SENDING_LIMITS_LOCATION_CHECK, &CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsLocationCheck)
END_MESSAGE_MAP()


// CApptReminderSchedDlg message handlers

void CApptReminderSchedDlg::OnBnClickedApptReminderAddExceptionBtn()
{
	try {
		// Create a new exception object and pass it to the exception editor
		CApptReminderException *pEx = new CApptReminderException();
		CApptReminderScheduleExceptionDlg dlg(*pEx, this);
		if(dlg.DoModal() == IDOK) {
			// Add the new exception to the schedule
			m_arsSchedule.AddException(pEx);

			// Add the new exception to the exception list
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExceptions->GetNewRow();
			pRow->PutValue(elID, pEx->GetID());
			pRow->PutValue(elDescription, _bstr_t(pEx->GetDescription()));
			m_pExceptions->AddRowAtEnd(pRow, NULL);
			UpdateControls();
		} else {
			// Free the exception since the user cancelled
			delete pEx;
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedDlg::OnBnClickedApptReminderEditExceptionBtn()
{
	try {
		// Get the currently selected row and find the exception object associated with it
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExceptions->CurSel;
		if(!pRow) {
			return;
		}
		long nID = VarLong(pRow->GetValue(elID));
		CApptReminderException *pEx = m_arsSchedule.GetExceptionById(nID);

		// Open up an exception editor with this exception object
		CApptReminderScheduleExceptionDlg dlg(*pEx, this);
		if(dlg.DoModal() == IDOK) {
			pRow->PutValue(elDescription, _bstr_t(pEx->GetDescription()));
		}
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedDlg::ReloadRulesList()
{
	// Clear the rules out and reload them from the schedule
	m_pRules->Clear();

	// Iterate through the rules in the schedule
	CApptReminderScheduleRule *pRule = NULL;
	long nIndex = m_arsSchedule.GetRuleStartPosition();
	while(m_arsSchedule.GetNextRule(pRule, nIndex)) {
		if(pRule) {
			// Add the rule to the rules datalist
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRules->GetNewRow();
			pRow->PutValue(rlID, pRule->GetID());
			pRow->PutValue(rlDescription, _bstr_t(pRule->GetDescription()));
			pRow->PutValue(rlIgnoreExceptions, (pRule->GetIgnoreExceptions() ? VARIANT_TRUE : VARIANT_FALSE));
			m_pRules->AddRowAtEnd(pRow, NULL);
		}
	}
}

void CApptReminderSchedDlg::ReloadExceptionsList()
{
	// Clear the exceptions out and reload them from the schedule
	m_pExceptions->Clear();

	// Iterate through the exceptions in the schedule
	CApptReminderException *pException = NULL;
	long nIndex = m_arsSchedule.GetExceptionStartPosition();
	while(m_arsSchedule.GetNextException(pException, nIndex)) {
		if(pException) {
			// Add the exception to the exceptions datalist
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExceptions->GetNewRow();
			pRow->PutValue(elID, pException->GetID());
			pRow->PutValue(elDescription, _bstr_t(pException->GetDescription()));
			m_pExceptions->AddRowAtEnd(pRow, NULL);
		}
	}
}

// Remove the currently selected exception
void CApptReminderSchedDlg::OnBnClickedApptReminderRemoveExceptionBtn()
{
	try {
		// Get the currently selected row and find the exception object associated with it
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExceptions->CurSel;
		if(!pRow) {
			return;
		}
		long nID = VarLong(pRow->GetValue(elID));
		// Remove the exception row from the datalist
		m_pExceptions->RemoveRow(pRow);
		// Now remove it from the schedule itself
		m_arsSchedule.RemoveException(nID);
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

// Create a new rule
void CApptReminderSchedDlg::OnBnClickedAddApptSchedRuleBtn()
{
	try {
		// Bring up the rule dialog, get the rule from that, and then add it to the rule list.
		CApptReminderScheduleRule *pRule = new CApptReminderScheduleRule();
		CApptReminderScheduleRuleDlg dlg(*pRule, this);
		if(dlg.DoModal() == IDOK) {
			// Add the new rule to the schedule
			m_arsSchedule.AddRule(pRule);

			// Add the new rule to the rule list
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRules->GetNewRow();
			pRow->PutValue(rlID, pRule->GetID());
			pRow->PutValue(rlDescription, _bstr_t(pRule->GetDescription()));
			pRow->PutValue(rlIgnoreExceptions, (pRule->GetIgnoreExceptions() ? VARIANT_TRUE : VARIANT_FALSE));
			m_pRules->AddRowAtEnd(pRow, NULL);
			UpdateControls();
		} else {
			// Free the rule since the user cancelled
			delete pRule;
		}
	} NxCatchAll(__FUNCTION__);
}

// Edit the currently selected rule using the rule editor dialog
void CApptReminderSchedDlg::OnBnClickedApptRmndSchedEditRule()
{
	try {
		// Get the currently selected row and find the exception object associated with it
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRules->CurSel;
		if(!pRow) {
			return;
		}
		long nID = VarLong(pRow->GetValue(rlID));
		CApptReminderScheduleRule *pRule = m_arsSchedule.GetRuleById(nID);

		// Open up a rule editor with this rule object
		CApptReminderScheduleRuleDlg dlg(*pRule, this);
		if(dlg.DoModal() == IDOK) {
			// Add the rule to the datalist
			pRow->PutValue(rlDescription, _bstr_t(pRule->GetDescription()));
			pRow->PutValue(rlIgnoreExceptions, (pRule->GetIgnoreExceptions() ? VARIANT_TRUE : VARIANT_FALSE));
			UpdateControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// Remove the currently selected rule
void CApptReminderSchedDlg::OnBnClickedRemoveApptSchedRuleBtn()
{
	try {
		// Get the currently selected row and find the rule object associated with it
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRules->CurSel;
		if(!pRow) {
			return;
		}

		long nID = VarLong(pRow->GetValue(rlID));
		// Remove the rule from the datalist
		m_pRules->RemoveRow(pRow);
		// Remove the rule 
		m_arsSchedule.RemoveRule(nID);
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CApptReminderSchedDlg, CNxDialog)
ON_EVENT(CApptReminderSchedDlg, IDC_APPT_REMINDER_SCHED_RULES_DL, 10, CApptReminderSchedDlg::EditingFinishedApptReminderSchedRulesDl, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CApptReminderSchedDlg, IDC_APPT_REMINDER_SCHED_RULES_DL, 3, CApptReminderSchedDlg::DblClickCellApptReminderSchedRulesDl, VTS_DISPATCH VTS_I2)
ON_EVENT(CApptReminderSchedDlg, IDC_APPT_REMINDER_SCHED_EXCEPTIONS_DL, 3, CApptReminderSchedDlg::DblClickCellApptReminderSchedExceptionsDl, VTS_DISPATCH VTS_I2)
ON_EVENT(CApptReminderSchedDlg, IDC_APPT_REMINDER_SCHED_RULES_DL, 2, CApptReminderSchedDlg::SelChangedApptReminderSchedRulesDl, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CApptReminderSchedDlg, IDC_APPT_REMINDER_SCHED_EXCEPTIONS_DL, 2, CApptReminderSchedDlg::SelChangedApptReminderSchedExceptionsDl, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

// Toggle ignoring exceptions for the selected rule
void CApptReminderSchedDlg::EditingFinishedApptReminderSchedRulesDl(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			return;
		}

		// We only want the ignore exceptions column
		if(nCol != rlIgnoreExceptions) {
			return;
		}

		long nID = VarLong(pRow->GetValue(rlID));
		CApptReminderScheduleRule *pRule = m_arsSchedule.GetRuleById(nID);
		if(bCommit) {
			pRule->SetIgnoreExceptions(VarBool(varNewValue));
			UpdateControls();
		}
	} NxCatchAll(__FUNCTION__);
}

// Open up the rule editor by double clicking on it
void CApptReminderSchedDlg::DblClickCellApptReminderSchedRulesDl(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			long nID = VarLong(pRow->GetValue(rlID));
			CApptReminderScheduleRule *pRule = m_arsSchedule.GetRuleById(nID);

			// Open up a rule editor with this rule object
			CApptReminderScheduleRuleDlg dlg(*pRule, this);
			if(dlg.DoModal() == IDOK) {
				pRow->PutValue(rlDescription, _bstr_t(pRule->GetDescription()));
				pRow->PutValue(rlIgnoreExceptions, (pRule->GetIgnoreExceptions() ? VARIANT_TRUE : VARIANT_FALSE));
				UpdateControls();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// Open up the exception editor by double clicking on it
void CApptReminderSchedDlg::DblClickCellApptReminderSchedExceptionsDl(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			long nID = VarLong(pRow->GetValue(elID));
			CApptReminderException *pEx = m_arsSchedule.GetExceptionById(nID);

			// Open up an exception editor with this exception object
			CApptReminderScheduleExceptionDlg dlg(*pEx, this);
			if(dlg.DoModal() == IDOK) {
				pRow->PutValue(elDescription, _bstr_t(pEx->GetDescription()));
				UpdateControls();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedDlg::SelChangedApptReminderSchedRulesDl(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedDlg::SelChangedApptReminderSchedExceptionsDl(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedDlg::UpdateControls()
{
	// Update the rule related buttons
	BOOL bRuleRowValid = FALSE;
	if(m_pRules->CurSel) {
		bRuleRowValid = TRUE;
	}

	m_btnEditRule.EnableWindow(bRuleRowValid);
	m_btnRemoveRule.EnableWindow(bRuleRowValid);

	// Update the exception related buttons
	BOOL bExRowValid = FALSE;
	if(m_pExceptions->CurSel) {
		bExRowValid = TRUE;
	}

	m_btnEditException.EnableWindow(bExRowValid);
	m_btnRemoveException.EnableWindow(bExRowValid);

	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Enable/disable the checkboxes under the second option.
	if (m_radioPerDayPatientAppointmentDate.GetCheck() == BST_CHECKED) {
		m_checkAndResource.EnableWindow(TRUE);
		m_checkAndLocation.EnableWindow(TRUE);
	} else {
		m_checkAndResource.EnableWindow(FALSE);
		m_checkAndLocation.EnableWindow(FALSE);
	}
}
void CApptReminderSchedDlg::OnBnClickedRuleshelpbtn()
{
	try {
		MessageBox("Rules dictate when and for what kind of appointments the reminder messages will be sent. "
			"If you have multiple rules, then a separate reminder message will be sent for each of these.\r\n\r\n"
			"If you ever want an appointment reminder message to go out even when an exception says it cannot, you may "
			"choose to Override Exceptions for the rule. ", "Rules Help", MB_ICONINFORMATION | MB_OK);
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedDlg::OnBnClickedExceptionshelpbtn()
{
	try {
		MessageBox("Exceptions specify when not to send appointment reminder messages. You can specify certain days, times, and dates. "
			"If you have multiple exceptions, then all of these will apply.\r\n\r\n"
			"Exceptions always take precedence over any existing rules, except those that are marked to Override Exceptions.", "Exceptions Help", MB_ICONINFORMATION | MB_OK);
	} NxCatchAll(__FUNCTION__);
}


// (r.gonet 2015-11-16 02:33) - PLID 67587 - Handle when the user clicks the Per Appointment and Rule radio button.
void CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsOpt1Radio()
{
	try {
		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Save the sending limit options to the in-memory object.
		m_arsSchedule.SetSendingLimitOptions(CApptReminderSchedule::PerRule | CApptReminderSchedule::PerAppointment);
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-11-16 02:33) - PLID 67587 - Handle when the user clicks the Per Day, Patient, and Appointment Date radio button.
void CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsOpt2Radio()
{
	try {
		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Save the sending limit options to the in-memory object.
		long nNewOptions = CApptReminderSchedule::PerRule | CApptReminderSchedule::PerPatient | CApptReminderSchedule::PerAppointmentDate;
		if (m_checkAndResource.GetCheck() == BST_CHECKED) {
			nNewOptions |= CApptReminderSchedule::PerResource;
		}
		if (m_checkAndLocation.GetCheck() == BST_CHECKED) {
			nNewOptions |= CApptReminderSchedule::PerLocation;
		}
		m_arsSchedule.SetSendingLimitOptions(nNewOptions);
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-11-16 02:33) - PLID 67587 - Handle when the user clicks the And Resource checkbox
void CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsResourceCheck()
{
	try {
		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Save the sending limit options to the in-memory object.
		long nOldOptions = m_arsSchedule.GetSendingLimitOptions();
		long nNewOptions = nOldOptions;
		if (m_checkAndResource.GetCheck() == BST_CHECKED) {
			nNewOptions |= CApptReminderSchedule::PerResource;
		} else {
			nNewOptions &= ~CApptReminderSchedule::PerResource;
		}
		m_arsSchedule.SetSendingLimitOptions(nNewOptions);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2015-11-16 02:33) - PLID 67587 - Handle when the user clicks the And Location checkbox
void CApptReminderSchedDlg::OnBnClickedApptRmndrSchedSendingLimitsLocationCheck()
{
	try {
		// (r.gonet 2015-11-16 02:33) - PLID 67587 - Save the sending limit options to the in-memory object.
		long nOldOptions = m_arsSchedule.GetSendingLimitOptions();
		long nNewOptions = nOldOptions;
		if (m_checkAndLocation.GetCheck() == BST_CHECKED) {
			nNewOptions |= CApptReminderSchedule::PerLocation;
		} else {
			nNewOptions &= ~CApptReminderSchedule::PerLocation;
		}
		m_arsSchedule.SetSendingLimitOptions(nNewOptions);
	} NxCatchAll(__FUNCTION__);
}
