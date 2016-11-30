// CommissionRuleSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "CommissionRuleSetupDlg.h"
#include "datetimeutils.h"
#include "AuditTrail.h"
#include "internationalutils.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.wetta 2007-03-27 10:34) - PLID 25327 - Dialog created

/////////////////////////////////////////////////////////////////////////////
// CCommissionRuleSetupDlg dialog


CCommissionRuleSetupDlg::CCommissionRuleSetupDlg(CWnd* pParent)
	: CNxDialog(CCommissionRuleSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommissionRuleSetupDlg)
	//}}AFX_DATA_INIT

	m_bChanged = false;
	m_bUpdating = false;
	m_backgroundColor = GetNxColor(GNC_ADMIN, 0);
	m_nNewID = -1;
	m_bUseTieredRules = FALSE;
}


void CCommissionRuleSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	// (a.wetta 2007-03-29 14:57) - PLID 25407 - Make sure the buttons match in the Retail tab (they must be CNxIconButtons)
	//{{AFX_DATA_MAP(CCommissionRuleSetupDlg)
	DDX_Control(pDX, IDC_SHOW_OUTDATED, m_btnShowOutdated);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_REMOVE_RULE, m_btnRemoveRule);
	DDX_Control(pDX, IDC_ADD_RULE, m_btnAddRule);
	DDX_Control(pDX, IDC_START_BASED_DATE, m_ctrlStartBasedDate);
	DDX_Control(pDX, IDC_START_APPLY_DATE, m_ctrlStartApplyDate);
	DDX_Control(pDX, IDC_END_BASED_DATE, m_ctrlEndBasedDate);
	DDX_Control(pDX, IDC_END_APPLY_DATE, m_ctrlEndApplyDate);
	DDX_Control(pDX, IDC_RULE_NAME, m_editRuleName);
	DDX_Control(pDX, IDC_COMM_COLOR2, m_bkgColor2);
	DDX_Control(pDX, IDC_COMM_COLOR5, m_bkgColor5);
	DDX_Control(pDX, IDC_MONEY_THRESHOLD, m_nxeditMoneyThreshold);
	DDX_Control(pDX, IDC_COMMISSION_PERCENTAGE, m_nxeditCommissionPercentage);
	DDX_Control(pDX, IDC_AMOUNT_SOLD_LABEL, m_nxstaticAmountSoldLabel);
	DDX_Control(pDX, IDC_ADV_NOTE_LABEL, m_nxstaticAdvNoteLabel);
	DDX_Control(pDX, IDC_APPLY_COMMISSION_LABEL, m_nxstaticApplyCommissionLabel);
	DDX_Control(pDX, IDC_RADIO_THIS_TIER_ONLY, m_radioThisTierOnly);
	DDX_Control(pDX, IDC_RADIO_ALL_LOWER_TIERS, m_radioAllLowerTiers);
	DDX_Control(pDX, IDC_BASED_ON_DATE_RANGE_LABEL, m_nxstaticBasedOnDateRangeLabel);
	DDX_Control(pDX, IDC_BASED_ON_DATE_START_LABEL, m_nxstaticBasedOnDateStartLabel);
	DDX_Control(pDX, IDC_BASED_ON_DATE_TO_LABEL, m_nxstaticBasedOnDateToLabel);
	DDX_Control(pDX, IDC_BASED_ON_DATE_END_LABEL, m_nxstaticBasedOnDateEndLabel);
	DDX_Control(pDX, IDC_APPLY_DATE_RANGE_LABEL, m_nxstaticApplyDateLabel);
	DDX_Control(pDX, IDC_COMMISSION_LABEL, m_nxlabelCommissionLabel); // (b.eyers 2015-05-14) - PLID 65978
	DDX_Control(pDX, IDC_COMMISSION_AMOUNT, m_nxeditCommissionAmount); // (b.eyers 2015-05-14) - PLID 65978
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CCommissionRuleSetupDlg, IDC_END_APPLY_DATE, 2 /* Change */, OnChangeEndApplyDate, VTS_NONE)
//	ON_EVENT(CCommissionRuleSetupDlg, IDC_END_BASED_DATE, 2 /* Change */, OnChangeEndBasedDate, VTS_NONE)
//	ON_EVENT(CCommissionRuleSetupDlg, IDC_START_APPLY_DATE, 2 /* Change */, OnChangeStartApplyDate, VTS_NONE)
//	ON_EVENT(CCommissionRuleSetupDlg, IDC_START_BASED_DATE, 2 /* Change */, OnChangeStartBasedDate, VTS_NONE)

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CCommissionRuleSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCommissionRuleSetupDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_END_APPLY_DATE, OnChangeEndApplyDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_END_BASED_DATE, OnChangeEndBasedDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_START_APPLY_DATE, OnChangeStartApplyDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_START_BASED_DATE, OnChangeStartBasedDate)
	ON_BN_CLICKED(IDC_ADD_RULE, OnAddRule)
	ON_BN_CLICKED(IDC_REMOVE_RULE, OnRemoveRule)
	ON_EN_CHANGE(IDC_RULE_NAME, OnChangeRuleName)
	ON_EN_CHANGE(IDC_MONEY_THRESHOLD, OnChangeMoneyThreshold)// (a.vengrofski 2009-12-22 10:45) - PLID <36364> - Added to save the money, from a KillFocus hicup.
	ON_EN_CHANGE(IDC_COMMISSION_PERCENTAGE, OnChangeCommissionPercentage)// (a.vengrofski 2009-12-22 10:50) - PLID <36364> - Added to save the commission rate, from a KillFocus hicup.
	ON_BN_CLICKED(IDC_SHOW_OUTDATED, OnShowOutdated)
	ON_EN_KILLFOCUS(IDC_RULE_NAME, OnKillfocusRuleName)
	ON_EN_KILLFOCUS(IDC_MONEY_THRESHOLD, OnKillfocusMoneyThreshold)
	ON_EN_KILLFOCUS(IDC_COMMISSION_PERCENTAGE, OnKillfocusCommissionPercentage)
	ON_BN_CLICKED(IDC_RADIO_THIS_TIER_ONLY, OnRadioThisTierOnly)
	ON_BN_CLICKED(IDC_RADIO_ALL_LOWER_TIERS, OnRadioAllLowerTiers)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick) // (b.eyers 2015-05-14) - PLID 65978
	ON_EN_KILLFOCUS(IDC_COMMISSION_AMOUNT, OnKillfocusCommissionAmount) // (b.eyers 2015-05-14) - PLID 65978
	ON_EN_CHANGE(IDC_COMMISSION_AMOUNT, OnChangeCommissionAmount) // (b.eyers 2015-05-14) - PLID 65978
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommissionRuleSetupDlg message handlers

BOOL CCommissionRuleSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pRuleList = BindNxDataList2Ctrl(IDC_COMMISSION_RULE_LIST, false);

		// (j.jones 2009-11-18 14:48) - PLID 29046 - m_bUseTieredRules determines
		// whether we show rules where IsTieredCommission = 1, or not
		CString strWhere = "EndDate >= dbo.AsDateNoTime(GetDate())";
		if(m_bUseTieredRules) {
			SetWindowText("Tiered Commission Rule Setup");
			strWhere += " AND IsTieredCommission = 1";
			m_nxstaticAmountSoldLabel.SetWindowText("If the provider sold the following dollar amount:");
			m_nxstaticApplyDateLabel.SetWindowText("If the report date range ends within:");

			//hide the based-on date range
			GetDlgItem(IDC_BASED_ON_DATE_RANGE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BASED_ON_DATE_START_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BASED_ON_DATE_TO_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BASED_ON_DATE_END_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_START_BASED_DATE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_END_BASED_DATE)->ShowWindow(SW_HIDE);

			//move the multi-rule note label higher up and re-word it
			CRect rcNote, rcDate;
			GetDlgItem(IDC_ADV_NOTE_LABEL)->GetWindowRect(rcNote);
			GetDlgItem(IDC_BASED_ON_DATE_RANGE_LABEL)->GetWindowRect(rcDate);
			ScreenToClient(rcNote);
			ScreenToClient(rcDate);
			GetDlgItem(IDC_ADV_NOTE_LABEL)->MoveWindow(rcDate.left,rcDate.bottom - rcNote.Height(), rcNote.Width(), rcNote.Height());
			m_nxstaticAdvNoteLabel.SetWindowText("Tiered rules will calculate commissions based on the total amount earned "
				"within the date range in which the report is run. "
				"Rules will be used if they are valid on the end date of the report date range.");
		}
		else {
			SetWindowText("Basic Commission Rule Setup");
			strWhere += " AND IsTieredCommission = 0";

			//hide the tiered radio options
			GetDlgItem(IDC_APPLY_COMMISSION_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_THIS_TIER_ONLY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_ALL_LOWER_TIERS)->ShowWindow(SW_HIDE);
		}

		m_pRuleList->PutWhereClause(_bstr_t(strWhere));
		m_pRuleList->Requery();
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddRule.AutoSet(NXB_NEW);
		m_btnRemoveRule.AutoSet(NXB_DELETE);

		// (a.wetta 2007-05-17 16:38) - PLID 25394 - Check to see if they can create new rules
		GetDlgItem(IDC_ADD_RULE)->EnableWindow(CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE));

		// Nothing is selected, so gray out the edit boxes
		EnableEditBoxes(false);

		// (b.eyers 2015-05-14) - PLID 65978 - Set the label text
		m_nxlabelCommissionLabel.SetType(dtsHyperlink);
		m_nxlabelCommissionLabel.SetText("Give the provider the following % commission:");
		m_bPercent = TRUE; //default is %
		GetDlgItem(IDC_COMMISSION_AMOUNT)->ShowWindow(SW_HIDE);
		//TES 7/20/2015 - PLID 66600 - Hide the label for $ commissions
		GetDlgItem(IDC_DOLLAR_TIER_COMMISSION_LABEL)->ShowWindow(SW_HIDE);

		// (a.wetta 2007-03-30 11:04) - PLID 24872 - Set the background color
		m_bkgColor2.SetColor(m_backgroundColor);
		m_bkgColor5.SetColor(m_backgroundColor);
	}
	NxCatchAll("Error in CCommissionRuleSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CCommissionRuleSetupDlg::OnCancel() 
{
	try {
		if (m_mapChangedRules.GetCount() > 0 || m_mapAddRules.GetCount() > 0 || m_aryDeleteRules.GetSize() > 0 || m_bChanged) {
			if (IDNO == MessageBox("You have made changes to the rules.  Are you sure you want to close without saving?", NULL, MB_YESNO|MB_ICONWARNING))
				return;
		}
	} NxCatchAll("Error in CCommissionRuleSetupDlg::OnCancel");

	CDialog::OnCancel();
}

void CCommissionRuleSetupDlg::OnOK() 
{
	try {
		// Make sure that the kill focus message is called for any edit boxes so that no changes are lost
		
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
		CheckFocus();

		if (m_mapChangedRules.GetCount() > 0 || m_mapAddRules.GetCount() > 0 || m_aryDeleteRules.GetSize() > 0 || m_bChanged) {
			if (!Save()) {
				// The data could not be validated
				return;
			}
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnOK");
	
	CDialog::OnOK();
}

BOOL CCommissionRuleSetupDlg::Save()
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE))
			return FALSE;

		// Make sure that we get any of the changes currently on the screen
		if (m_bChanged) {
			if (!UpdateRowFromScreen(m_pRuleList->GetCurSel())) {
				// The data could not be validated
				return FALSE;
			}
		}

		// Start the sql batch to update the data
		CString strSql = BeginSqlBatch();
		// (d.thompson 2009-12-18) - PLID 36309 - AddStatementToSqlBatch() handles string formatting, so
		//	we can't give it an already formatted string or % signs will break things.  I removed all uses
		//	of the below variable.
		//CString strSqlStatement = "";
		CommissionRuleInfo criInfo;
		long nMapID;
		long nNewID = NewNumber("CommissionRulesT", "ID");

		POSITION posMap = m_mapAddRules.GetStartPosition();
		while (posMap) {
			m_mapAddRules.GetNextAssoc(posMap, nMapID, criInfo);
			long nNewRuleID = nNewID++;

			// (b.eyers 2015-05-14) - PLID 65978 - is this $ or %

			if (criInfo.cyCommissionAmount.GetStatus() != COleCurrency::invalid) {
				//there is $, save this
				//TES 7/20/2015 - PLID 66600 - % commissions always set OverwritePriorRules to 1
				AddStatementToSqlBatch(strSql, "INSERT INTO CommissionRulesT (ID, Name, StartDate, EndDate, "
					"BasedOnStartDate, BasedOnEndDate, Amount, MoneyThreshold, "
					"IsTieredCommission, OverwritePriorRules) "
					"VALUES (%li, '%s', '%s',  '%s', '%s', '%s', '%s', %s, %li, %li)",
					nNewRuleID,
					_Q(criInfo.strName),
					FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate),
					FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnStart, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnEnd, dtoDate),
					FormatCurrencyForSql(criInfo.cyCommissionAmount),
					FormatCurrencyForSql(criInfo.cyBasedOnMoneyThreshold),
					m_bUseTieredRules ? 1 : 0,
					1);

			}
			else {
				//save %
				// Add the new rules
				// (j.jones 2009-11-18 14:53) - PLID 29046 - supported tiered commissions
				// the 'based on' date is not used in tiered rules, it will save the regular date as well
				AddStatementToSqlBatch(strSql, "INSERT INTO CommissionRulesT (ID, Name, StartDate, EndDate, "
					"BasedOnStartDate, BasedOnEndDate, Percentage, MoneyThreshold, "
					"IsTieredCommission, OverwritePriorRules) "
					"VALUES (%li, '%s', '%s',  '%s', '%s', '%s', %g, %s, %li, %li)",
					nNewRuleID,
					_Q(criInfo.strName),
					FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate),
					FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnStart, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnEnd, dtoDate),
					criInfo.dblCommissionPercentage,
					FormatCurrencyForSql(criInfo.cyBasedOnMoneyThreshold),
					m_bUseTieredRules ? 1 : 0,
					criInfo.bOverwritePriorRules ? 1 : 0);
			}

			// Audit everything
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			// (j.jones 2009-11-19 11:01) - PLID 29046 - need to audit differently "basic" commissions vs. "tiered"
			AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRuleCreated : aeiBasicCommissionRuleCreated, nNewRuleID, "", criInfo.strName, aepMedium, aetCreated);
			AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesStartDate : aeiBasicCommissionRulesStartDate, nNewRuleID, "", FormatDateTimeForInterface(criInfo.dtApplyStart, NULL, dtoDate), aepMedium, aetChanged);
			AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesEndDate : aeiBasicCommissionRulesEndDate, nNewRuleID, "", FormatDateTimeForInterface(criInfo.dtApplyEnd, NULL, dtoDate), aepMedium, aetChanged);
			// (j.jones 2009-11-19 17:14) - PLID 29046 - the 'based on' date is not currently used in tiered rules,
			//but the audit exists if we ever start using it
			if(!m_bUseTieredRules) {
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesBasedOnStartDate : aeiBasicCommissionRulesBasedOnStartDate, nNewRuleID, "", FormatDateTimeForInterface(criInfo.dtBasedOnStart, NULL, dtoDate), aepMedium, aetChanged);
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesBasedOnEndDate : aeiBasicCommissionRulesBasedOnEndDate, nNewRuleID, "", FormatDateTimeForInterface(criInfo.dtBasedOnEnd, NULL, dtoDate), aepMedium, aetChanged);
			}
			//AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesPercentage : aeiBasicCommissionRulesPercentage, nNewRuleID, "", FormatString("%g%%", criInfo.dblCommissionPercentage), aepMedium, aetChanged);
			AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesMoneyThreshold : aeiBasicCommissionRulesMoneyThreshold, nNewRuleID, "", FormatCurrencyForInterface(criInfo.cyBasedOnMoneyThreshold), aepMedium, aetChanged);	
			if(m_bUseTieredRules) {
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, aeiTieredCommissionRulesOverwritePrior, nNewRuleID, "", criInfo.bOverwritePriorRules ? "This tier and lower" : "This tier only", aepMedium, aetChanged);	
			}

			// (b.eyers 2015-05-14) - PLID 65978 & 65979
			if (criInfo.cyCommissionAmount.GetStatus() != COleCurrency::invalid) {
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesAmount : aeiBasicCommissionRulesAmount, nNewRuleID, "", FormatCurrencyForInterface(criInfo.cyCommissionAmount), aepMedium, aetChanged);
			}
			else {
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesPercentage : aeiBasicCommissionRulesPercentage, nNewRuleID, "", FormatString("%g%%", criInfo.dblCommissionPercentage), aepMedium, aetChanged);
			}
		}

		posMap = m_mapChangedRules.GetStartPosition();
		while (posMap) {
			m_mapChangedRules.GetNextAssoc(posMap, nMapID, criInfo);

			// (b.eyers 2015-05-14) - PLID 65978 - update $
			if (criInfo.cyCommissionAmount.GetStatus() != COleCurrency::invalid) {
				AddStatementToSqlBatch(strSql, "UPDATE CommissionRulesT SET Name = '%s', "
					"StartDate = '%s', "
					"EndDate = '%s', "
					"BasedOnStartDate = '%s', "
					"BasedOnEndDate = '%s', "
					"Percentage = NULL, "
					"Amount = '%s', "
					"MoneyThreshold = %s, "
					"IsTieredCommission = %li, "
					"OverwritePriorRules = %li "
					"WHERE ID = %li",
					_Q(criInfo.strName),
					FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate),
					FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnStart, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnEnd, dtoDate),
					FormatCurrencyForSql(criInfo.cyCommissionAmount),
					FormatCurrencyForSql(criInfo.cyBasedOnMoneyThreshold),
					m_bUseTieredRules ? 1 : 0,
					1,
					nMapID);
			}
			else { // (b.eyers 2015-05-14) - PLID 65978 - save amount as null
				// Update the changed rules
				// (j.jones 2009-11-18 14:53) - PLID 29046 - supported tiered commissions
				// the 'based on' date is not used in tiered rules, it will save the regular date as well
				AddStatementToSqlBatch(strSql, "UPDATE CommissionRulesT SET Name = '%s', "
					"StartDate = '%s', "
					"EndDate = '%s', "
					"BasedOnStartDate = '%s', "
					"BasedOnEndDate = '%s', "
					"Percentage = %g, "
					"Amount = NULL, "
					"MoneyThreshold = %s, "
					"IsTieredCommission = %li, "
					"OverwritePriorRules = %li "
					"WHERE ID = %li",
					_Q(criInfo.strName),
					FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate),
					FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyStart, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnStart, dtoDate),
					m_bUseTieredRules ? FormatDateTimeForSql(criInfo.dtApplyEnd, dtoDate) : FormatDateTimeForSql(criInfo.dtBasedOnEnd, dtoDate),
					criInfo.dblCommissionPercentage,
					FormatCurrencyForSql(criInfo.cyBasedOnMoneyThreshold),
					m_bUseTieredRules ? 1 : 0,
					criInfo.bOverwritePriorRules ? 1 : 0,
					nMapID);
			}

			// Audit everything
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			// (j.jones 2009-11-19 11:01) - PLID 29046 - need to audit differently "basic" commissions vs. "tiered"
			if (criInfo.strOldName != criInfo.strName)
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesName : aeiBasicCommissionRulesName, nMapID, criInfo.strOldName, criInfo.strName, aepMedium, aetChanged);
			if (criInfo.dtOldApplyStart != criInfo.dtApplyStart)
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesStartDate : aeiBasicCommissionRulesStartDate, nMapID, FormatDateTimeForInterface(criInfo.dtOldApplyStart, NULL, dtoDate), FormatDateTimeForInterface(criInfo.dtApplyStart, NULL, dtoDate), aepMedium, aetChanged);
			if (criInfo.dtOldApplyEnd != criInfo.dtApplyEnd)
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesEndDate : aeiBasicCommissionRulesEndDate, nMapID, FormatDateTimeForInterface(criInfo.dtOldApplyEnd, NULL, dtoDate), FormatDateTimeForInterface(criInfo.dtApplyEnd, NULL, dtoDate), aepMedium, aetChanged);
			// (j.jones 2009-11-19 17:14) - PLID 29046 - the 'based on' date is not currently used in tiered rules,
			//but the audit exists if we ever start using it
			if(!m_bUseTieredRules) {
				if (criInfo.dtOldBasedOnStart != criInfo.dtBasedOnStart) {
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesBasedOnStartDate : aeiBasicCommissionRulesBasedOnStartDate, nMapID, FormatDateTimeForInterface(criInfo.dtOldBasedOnStart, NULL, dtoDate), FormatDateTimeForInterface(criInfo.dtBasedOnStart, NULL, dtoDate), aepMedium, aetChanged);
				}
				if (criInfo.dtOldBasedOnEnd != criInfo.dtBasedOnEnd) {
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesBasedOnEndDate : aeiBasicCommissionRulesBasedOnEndDate, nMapID, FormatDateTimeForInterface(criInfo.dtOldBasedOnEnd, NULL, dtoDate), FormatDateTimeForInterface(criInfo.dtBasedOnEnd, NULL, dtoDate), aepMedium, aetChanged);
				}
			}
			//if (criInfo.dblOldCommissionPercentage != criInfo.dblCommissionPercentage)
			//	AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesPercentage : aeiBasicCommissionRulesPercentage, nMapID, FormatString("%g%%", criInfo.dblOldCommissionPercentage), FormatString("%g%%", criInfo.dblCommissionPercentage), aepMedium, aetChanged);
			if (criInfo.cyOldBasedOnMoneyThreshold != criInfo.cyBasedOnMoneyThreshold)
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesMoneyThreshold : aeiBasicCommissionRulesMoneyThreshold, nMapID, FormatCurrencyForInterface(criInfo.cyOldBasedOnMoneyThreshold), FormatCurrencyForInterface(criInfo.cyBasedOnMoneyThreshold), aepMedium, aetChanged);	
			if(m_bUseTieredRules && criInfo.bOldOverwritePriorRules != criInfo.bOverwritePriorRules) {
				AuditEvent(-1, criInfo.strName, nAuditTransactionID, aeiTieredCommissionRulesOverwritePrior, nMapID, criInfo.bOldOverwritePriorRules ? "This tier and lower" : "This tier only", criInfo.bOverwritePriorRules ? "This tier and lower" : "This tier only", aepMedium, aetChanged);	
			}

			// (b.eyers 2015-05-14) - PLID 65978 & 65979
			if (criInfo.cyCommissionAmount.GetStatus() != COleCurrency::invalid) {
				//amount changed, did it really? 
				if (criInfo.cyCommissionAmount != criInfo.cyOldCommissionAmount && criInfo.cyOldCommissionAmount != COleCurrency(0,0))
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesAmount : aeiBasicCommissionRulesAmount, nMapID, FormatCurrencyForInterface(criInfo.cyOldCommissionAmount), FormatCurrencyForInterface(criInfo.cyCommissionAmount), aepMedium, aetChanged);
				else {
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesPercentage : aeiBasicCommissionRulesPercentage, nMapID, FormatString("%g%%", criInfo.dblOldCommissionPercentage), "", aepMedium, aetChanged);
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesAmount : aeiBasicCommissionRulesAmount, nMapID, "", FormatCurrencyForInterface(criInfo.cyCommissionAmount), aepMedium, aetChanged);
				}
			}
			else {
				if (criInfo.dblOldCommissionPercentage != criInfo.dblCommissionPercentage && criInfo.dblOldCommissionPercentage != 0)
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesPercentage : aeiBasicCommissionRulesPercentage, nMapID, FormatString("%g%%", criInfo.dblOldCommissionPercentage), FormatString("%g%%", criInfo.dblCommissionPercentage), aepMedium, aetChanged);
				else { // (b.eyers 2015-05-14) - PLID 65978 - audit it if changed from $ to %
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesAmount : aeiBasicCommissionRulesAmount, nMapID, FormatCurrencyForInterface(criInfo.cyOldCommissionAmount), "", aepMedium, aetChanged);
					AuditEvent(-1, criInfo.strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRulesPercentage : aeiBasicCommissionRulesPercentage, nMapID, "", FormatString("%g%%", criInfo.dblCommissionPercentage), aepMedium, aetChanged);
				}
			}


		}

		for (int i = 0; i < m_aryDeleteRules.GetSize(); i++) {
			// Remove the deleted rules
			AddStatementToSqlBatch(strSql, "DELETE FROM CommissionRulesLinkT WHERE RuleID = %li", m_aryDeleteRules.GetAt(i).nID);
			// (j.jones 2009-11-18 14:57) - PLID 29046 - delete from TieredCommissionRulesLinkT
			AddStatementToSqlBatch(strSql, "DELETE FROM TieredCommissionRulesLinkT WHERE RuleID = %li", m_aryDeleteRules.GetAt(i).nID);
			AddStatementToSqlBatch(strSql, "DELETE FROM CommissionRulesT WHERE ID = %li", m_aryDeleteRules.GetAt(i).nID);

			// Audit the delete
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			// (j.jones 2009-11-19 11:01) - PLID 29046 - need to audit differently "basic" commissions vs. "tiered"
			AuditEvent(-1, m_aryDeleteRules.GetAt(i).strName, nAuditTransactionID, m_bUseTieredRules ? aeiTieredCommissionRuleDeleted : aeiBasicCommissionRuleDeleted, m_aryDeleteRules.GetAt(i).nID, m_aryDeleteRules.GetAt(i).strName, "", aepMedium, aetCreated);
		}

		ExecuteSqlBatch(strSql);

		// Commit auditing
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		m_mapAddRules.RemoveAll();
		m_mapChangedRules.RemoveAll();
		m_aryDeleteRules.RemoveAll();	

		return TRUE;
	
	}NxCatchAllCall("Error in CCommissionRuleSetupDlg::Save", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););

	return FALSE;
}

void CCommissionRuleSetupDlg::OnAddRule() 
{
	try {
		// Get the currently displayed rule
		NXDATALIST2Lib::IRowSettingsPtr pOldSel = m_pRuleList->GetCurSel();

		// Create the new row
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pRuleList->GetNewRow();
		
		long nNewID = --m_nNewID;
		CString strNewName = "<New Commission Rule>";

			// Add the new row to the datalist
		pNewRow->PutValue(crfID, _variant_t(nNewID));
		pNewRow->PutValue(crfName, _variant_t(strNewName));
		long nTime = (long)COleDateTime::GetCurrentTime();
		_variant_t varCurrentDate = COleDateTime((double)nTime);
		varCurrentDate.vt = VT_DATE;
		pNewRow->PutValue(crfApplyStart, varCurrentDate);
		pNewRow->PutValue(crfApplyEnd, varCurrentDate);
		pNewRow->PutValue(crfBasedOnStart, varCurrentDate);
		pNewRow->PutValue(crfBasedOnEnd, varCurrentDate);
		pNewRow->PutValue(crfCommissionPercentage, _variant_t((double)0));
		pNewRow->PutValue(crfMoneyThreshold, _variant_t(COleCurrency(0,0)));
		// (j.jones 2009-11-19 10:20) - PLID 29046 - support OverwritePriorRules
		pNewRow->PutValue(crfOverwritePriorRules, m_bUseTieredRules ? g_cvarFalse : g_cvarTrue);
		// (b.eyers 2015-05-14) - PLID 65978
		pNewRow->PutValue(crfCommissionAmount, _variant_t(COleCurrency(0, 0)));
		m_pRuleList->AddRowSorted(pNewRow, NULL);

		// Make sure the edit boxes are enabled
		EnableEditBoxes(true);
		// Select the row in the data list, refresh the info on the screen, and put the focus in the name box
		m_pRuleList->SetSelByColumn(crfID, _variant_t(nNewID));
		GetDlgItem(IDC_RULE_NAME)->SetFocus();

		// Handle the row change
		SelChangedCommissionRuleList(pOldSel, pNewRow);

		// Add the new rule to the add array
		m_mapAddRules[nNewID] = GetCommissionRuleInfoFromRow(pNewRow);

		m_editRuleName.SetSel(0, -1);
		
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnAddRule");
}

void CCommissionRuleSetupDlg::OnRemoveRule() 
{
	try {
		if (IDNO == MessageBox("If this rule is applied to any service items, the link will be removed.  Are you sure you want to delete this rule?", NULL, MB_YESNO|MB_ICONQUESTION))
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleList->GetCurSel();

		// Delete the row from the datalist
		m_pRuleList->RemoveRow(pRow);

		// Handle the row change
		SelChangedCommissionRuleList(pRow, m_pRuleList->GetCurSel());

		// Update the changes lists
		long lRuleID = VarLong(pRow->GetValue(crfID));
		CommissionRuleInfo criInfo;
		if (m_mapAddRules.Lookup(lRuleID, criInfo)) {
			// This rule is in the new list, just take it out
			m_mapAddRules.RemoveKey(lRuleID);
		}
		else {
			m_mapChangedRules.RemoveKey(lRuleID);
			CommissionRuleInfo criDeleteRule;
			criDeleteRule.nID = lRuleID;
			criDeleteRule.strName = VarString(pRow->GetValue(crfOldName));
			m_aryDeleteRules.Add(criDeleteRule);
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnRemoveRule");
}

// (a.wetta 2007-03-26 12:57) - PLID 25327 - Takes the information from criRuleInfo and fills in the fields on the dialog for the rule
void CCommissionRuleSetupDlg::UpdateCommissionInfoToScreen(CommissionRuleInfo criRuleInfo)
{
	try {
		// Take the info from criRuleInfo and display it on the screen
		SetDlgItemText(IDC_RULE_NAME, criRuleInfo.strName);

		// (a.walling 2008-05-14 12:47) - PLID 27591 - variants no longer necessary
		if (criRuleInfo.dtBasedOnStart == 0)
			m_ctrlStartBasedDate.SetValue((COleDateTime::GetCurrentTime()));
		else
			m_ctrlStartBasedDate.SetValue((criRuleInfo.dtBasedOnStart));

		if (criRuleInfo.dtApplyStart == 0)
			m_ctrlStartApplyDate.SetValue((COleDateTime::GetCurrentTime()));
		else
			m_ctrlStartApplyDate.SetValue((criRuleInfo.dtApplyStart));

		if (criRuleInfo.dtBasedOnEnd == 0)
			m_ctrlEndBasedDate.SetValue((COleDateTime::GetCurrentTime()));
		else
			m_ctrlEndBasedDate.SetValue((criRuleInfo.dtBasedOnEnd));

		if (criRuleInfo.dtApplyEnd == 0)
			m_ctrlEndApplyDate.SetValue((COleDateTime::GetCurrentTime()));
		else
			m_ctrlEndApplyDate.SetValue((criRuleInfo.dtApplyEnd));

		SetDlgItemText(IDC_MONEY_THRESHOLD, FormatCurrencyForInterface(criRuleInfo.cyBasedOnMoneyThreshold, TRUE, TRUE));
		CString strCommissionPercentage;
		strCommissionPercentage.Format("%g%%", criRuleInfo.dblCommissionPercentage); 
		// (b.eyers 2015-05-14) - PLID 65978
		COleCurrency cyCommissionAmount = criRuleInfo.cyCommissionAmount;
		SetDlgItemText(IDC_COMMISSION_PERCENTAGE, strCommissionPercentage);
		SetDlgItemText(IDC_COMMISSION_AMOUNT, FormatCurrencyForInterface(criRuleInfo.cyCommissionAmount, TRUE, TRUE));
		if (cyCommissionAmount > COleCurrency(0,0)) {
			//set to $ active
			m_nxlabelCommissionLabel.SetText("Give the provider the following $ commission:");
			m_bPercent = FALSE;
			//TES 7/20/2015 - PLID 66600 - Show the label for $ commissions (if we're showing tiers), hide the radio buttons
			GetDlgItem(IDC_COMMISSION_PERCENTAGE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COMMISSION_AMOUNT)->ShowWindow(SW_SHOW);
			if (m_bUseTieredRules) {
				GetDlgItem(IDC_DOLLAR_TIER_COMMISSION_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_APPLY_COMMISSION_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_RADIO_THIS_TIER_ONLY)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_RADIO_ALL_LOWER_TIERS)->ShowWindow(SW_HIDE);
			}
		}
		else {
			//set to % active
			m_nxlabelCommissionLabel.SetText("Give the provider the following % commission:");
			m_bPercent = TRUE;
			//TES 7/20/2015 - PLID 6600 - Hide the label for $ commissions (if we're showing tiers), show the radio buttons
			GetDlgItem(IDC_COMMISSION_PERCENTAGE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMMISSION_AMOUNT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DOLLAR_TIER_COMMISSION_LABEL)->ShowWindow(SW_HIDE);
			if (m_bUseTieredRules) {
				GetDlgItem(IDC_APPLY_COMMISSION_LABEL)->ShowWindow(SW_SHOWNA);
				GetDlgItem(IDC_RADIO_THIS_TIER_ONLY)->ShowWindow(SW_SHOWNA);
				GetDlgItem(IDC_RADIO_ALL_LOWER_TIERS)->ShowWindow(SW_SHOWNA);
			}
		}

		// (j.jones 2009-11-19 10:20) - PLID 29046 - load OverwritePriorRules
		m_radioThisTierOnly.SetCheck(!criRuleInfo.bOverwritePriorRules);
		m_radioAllLowerTiers.SetCheck(criRuleInfo.bOverwritePriorRules);

		m_criCurrentInfo = criRuleInfo;

		// By setting the items it thinks that something has changed, when really we have done the changing
		m_bChanged = false;

	}NxCatchAll("Error in CCommissionRuleSetupDlg::UpdateCommissionInfoToScreen");
}

BEGIN_EVENTSINK_MAP(CCommissionRuleSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCommissionRuleSetupDlg)
	ON_EVENT(CCommissionRuleSetupDlg, IDC_COMMISSION_RULE_LIST, 2 /* SelChanged */, OnSelChangedCommissionRuleList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CCommissionRuleSetupDlg, IDC_COMMISSION_RULE_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCommissionRuleList, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCommissionRuleSetupDlg::OnSelChangedCommissionRuleList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOldSelRow(lpOldSel);
		NXDATALIST2Lib::IRowSettingsPtr pNewSelRow(lpNewSel);
		SelChangedCommissionRuleList(pOldSelRow, pNewSelRow);

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnSelChangedCommissionRuleList");
}

void CCommissionRuleSetupDlg::SelChangedCommissionRuleList(NXDATALIST2Lib::IRowSettingsPtr pOldSel, NXDATALIST2Lib::IRowSettingsPtr pNewSel)
{
	try {
		// Save the current info on the screen for the previously selected row
		if (pOldSel) {
			if (!UpdateRowFromScreen(pOldSel)) {
				// The data could not be validated, so set the selection back to the old row
				m_pRuleList->SetSelByColumn(crfID, pOldSel->GetValue(crfID));
				return;
			}
		}

		// Get the rule info for the newly selected row and update it to the screen
		if (pNewSel) {
			// Make sure the edit boxes are enabled
			EnableEditBoxes(true);
			// Load info to screen
			UpdateCommissionInfoToScreen(GetCommissionRuleInfoFromRow(pNewSel));
		}
		else {
			// There is nothing to display, disable the edit boxes
			EnableEditBoxes(false);
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::SelChangedCommissionRuleList");
}

// (a.wetta 2007-03-26 12:58) - PLID 25327 - Gets the information from the given row in the rules datalist
CommissionRuleInfo CCommissionRuleSetupDlg::GetCommissionRuleInfoFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	CommissionRuleInfo criRuleInfo;
	try {
		// Make sure that the row exists
		if (pRow) {
			// Get the info from the row
			criRuleInfo.strName = VarString(pRow->GetValue(crfName));
			criRuleInfo.dtApplyStart = VarDateTime(pRow->GetValue(crfApplyStart));
			criRuleInfo.dtApplyEnd = VarDateTime(pRow->GetValue(crfApplyEnd));
			criRuleInfo.dtBasedOnStart = VarDateTime(pRow->GetValue(crfBasedOnStart));
			criRuleInfo.dtBasedOnEnd = VarDateTime(pRow->GetValue(crfBasedOnEnd));
			criRuleInfo.cyBasedOnMoneyThreshold = VarCurrency(pRow->GetValue(crfMoneyThreshold));
			criRuleInfo.dblCommissionPercentage = VarDouble(pRow->GetValue(crfCommissionPercentage), NULL); //65978
			// (j.jones 2009-11-19 10:20) - PLID 29046 - load OverwritePriorRules
			criRuleInfo.bOverwritePriorRules = VarBool(pRow->GetValue(crfOverwritePriorRules));
			// (b.eyers 2015-05-14) - PLID 65978
			criRuleInfo.cyCommissionAmount = VarCurrency(pRow->GetValue(crfCommissionAmount), COleCurrency(0,0));
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::GetCommissionRuleInfoFromRow");

	return criRuleInfo;
}

// (a.wetta 2007-03-26 12:58) - PLID 25327 - Gets the rule information from the screen
CommissionRuleInfo CCommissionRuleSetupDlg::GetCommissionRuleInfoFromScreen()
{
	CommissionRuleInfo criRuleInfo;

	try {
		// Get the info from the row
		GetDlgItemText(IDC_RULE_NAME, criRuleInfo.strName);
		CString strApplyStart, strApplyEnd, strBasedOnStart, strBasedOnEnd, strBasedOnMoneyThreshold, strCommissionPercentage, strCommissionAmount; // (b.eyers 2015-05-14) - PLID 65978
		criRuleInfo.dtApplyStart = m_ctrlStartApplyDate.GetValue();
		criRuleInfo.dtApplyEnd = m_ctrlEndApplyDate.GetValue();
		criRuleInfo.dtBasedOnStart = m_ctrlStartBasedDate.GetValue();
		criRuleInfo.dtBasedOnEnd = m_ctrlEndBasedDate.GetValue();
		GetDlgItemText(IDC_DOLLAR_THRESSHOLD, strBasedOnMoneyThreshold);
		criRuleInfo.cyBasedOnMoneyThreshold.ParseCurrency(strBasedOnMoneyThreshold);

		if (m_bPercent) { // (b.eyers 2015-05-14) - PLID 65978 - save which ever is showing
			GetDlgItemText(IDC_COMMISSION_PERCENTAGE, strCommissionPercentage);
			criRuleInfo.dblCommissionPercentage = atof(strCommissionPercentage);
			_variant_t varNull;
			varNull.vt = VT_NULL;
			criRuleInfo.cyCommissionAmount = varNull;
		}
		else {
			GetDlgItemText(IDC_COMMISSION_AMOUNT, strCommissionAmount);
			criRuleInfo.cyCommissionAmount.ParseCurrency(strCommissionAmount);
			criRuleInfo.dblCommissionPercentage = NULL;
		}
		// (j.jones 2009-11-19 10:20) - PLID 29046 - set OverwritePriorRules
		criRuleInfo.bOverwritePriorRules = m_radioAllLowerTiers.GetCheck();

	}NxCatchAll("Error in CCommissionRuleSetupDlg::GetCommissionRuleInfoFromScreen");

	return criRuleInfo;
}

// (a.wetta 2007-03-26 12:55) - PLID 25327 - This function takes the rule information from the screen and updates the corresponding
// row in the rules datalist and saves the changes to the data.  FALSE is returned if there is something wrong with the data on 
// the screen, for example the end date is before the start date
/*BOOL CCommissionRuleSetupDlg::UpdateDataAndRowFromScreen(NXDATALIST2Lib::IRowSettingsPtr pUpdateRow)
{
	try {
		if (m_bChanged) {
			// Something has changed, so let's update the datalist and the data
			long lID = VarLong(pUpdateRow->GetValue(crfID));

			CommissionRuleInfo criRuleInfo = GetCommissionRuleInfoFromScreen();

			if (!ValidateData(criRuleInfo))
				return FALSE;

			UpdateRowWithCommissionInfo(criRuleInfo, pUpdateRow);

			UpdateDataFromCommissionRuleInfo(lID, criRuleInfo);

			m_bChanged = false;
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::UpdateDataAndRowFromScreen");
	return TRUE;
}*/

BOOL CCommissionRuleSetupDlg::UpdateRowFromScreen(NXDATALIST2Lib::IRowSettingsPtr pUpdateRow)
{
	try {
		if (m_bChanged) {
			// Something has changed, so let's update the datalist
			long lID = VarLong(pUpdateRow->GetValue(crfID));

			CommissionRuleInfo criRuleInfo = GetCommissionRuleInfoFromScreen();

			if (!ValidateData(criRuleInfo))
				return FALSE;

			UpdateRowWithCommissionInfo(criRuleInfo, pUpdateRow);

			m_bChanged = false;

			if (lID > 0) {
				// Get the old info from the row
				criRuleInfo.strOldName = VarString(pUpdateRow->GetValue(crfOldName));
				criRuleInfo.dtOldApplyStart = VarDateTime(pUpdateRow->GetValue(crfOldApplyStart));
				criRuleInfo.dtOldApplyEnd = VarDateTime(pUpdateRow->GetValue(crfOldApplyEnd));
				criRuleInfo.dtOldBasedOnStart = VarDateTime(pUpdateRow->GetValue(crfOldBasedOnStart));
				criRuleInfo.dtOldBasedOnEnd = VarDateTime(pUpdateRow->GetValue(crfOldBasedOnEnd));
				criRuleInfo.cyOldBasedOnMoneyThreshold = VarCurrency(pUpdateRow->GetValue(crfOldMoneyThreshold));
				criRuleInfo.dblOldCommissionPercentage = VarDouble(pUpdateRow->GetValue(crfOldCommissionPercentage), NULL); // (b.eyers 2015-05-14) - PLID 65978
				// (j.jones 2009-11-19 10:20) - PLID 29046 - support OverwritePriorRules
				criRuleInfo.bOldOverwritePriorRules = VarBool(pUpdateRow->GetValue(crfOldOverwritePriorRules));
				// (b.eyers 2015-05-14) - PLID 65978
				criRuleInfo.cyOldCommissionAmount = VarCurrency(pUpdateRow->GetValue(crfOldCommissionAmount), COleCurrency(0,0));

				m_mapChangedRules[lID] = criRuleInfo;
			}
			else {
				// Negative IDs signify that this is a new rule that does not exist in the data yet
				m_mapAddRules[lID] = criRuleInfo;
			}
		}
	
	}NxCatchAll("Error in CCommissionRuleSetupDlg::UpdateRowFromScreen");
	return TRUE;
}

BOOL CCommissionRuleSetupDlg::ValidateData(CommissionRuleInfo criRuleInfo)
{
	try {
		if (criRuleInfo.dtApplyEnd < criRuleInfo.dtApplyStart) {
			MessageBox("The apply end date is before the apply start date. Please fix this before continuing.", NULL, MB_OK|MB_ICONWARNING);
			return FALSE;
		}

		// (j.jones 2009-11-19 17:14) - PLID 29046 - the 'based on' date is not used in tiered rules
		if(!m_bUseTieredRules) {
			if (criRuleInfo.dtBasedOnEnd < criRuleInfo.dtBasedOnStart) {
				MessageBox("The based on end date is before the based on start date. Please fix this before continuing.", NULL, MB_OK|MB_ICONWARNING);
				return FALSE;
			}
		}

		if (criRuleInfo.cyBasedOnMoneyThreshold < COleCurrency(0,0)) {
			MessageBox("The money amount cannot be less than zero. Please fix this before continuing.", NULL, MB_OK|MB_ICONWARNING);
			return FALSE;
		}

		if (m_bPercent) { // (b.eyers 2015-05-14) - PLID 65978
			if (criRuleInfo.dblCommissionPercentage < 0) {
				MessageBox("The commission percentage cannot be less than zero. Please fix this before continuing.", NULL, MB_OK | MB_ICONWARNING);
				return FALSE;
			}
		}
		else {
			if (criRuleInfo.cyCommissionAmount < COleCurrency(0, 0)) {
				MessageBox("The commission amount cannot be less than zero. Please fix this before continuing.", NULL, MB_OK | MB_ICONWARNING);
				return FALSE;
			}
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::ValidateData");
	// Everything is fine if it made it this far
	return TRUE;
}

// (a.wetta 2007-03-26 12:56) - PLID 25327 - This function updates the data on the rules datalist for the specified row with the
// data in criRuleInfo.  The information for the rule is stored on the datalist to prevent having to run multiple queries
// when updating the screen.
void CCommissionRuleSetupDlg::UpdateRowWithCommissionInfo(CommissionRuleInfo criRuleInfo, NXDATALIST2Lib::IRowSettingsPtr pUpdateRow)
{
	try {
		pUpdateRow->PutValue(crfName, _variant_t(criRuleInfo.strName));
		_variant_t varDate = criRuleInfo.dtApplyStart;
		varDate.vt = VT_DATE;
		pUpdateRow->PutValue(crfApplyStart, varDate);
		varDate = criRuleInfo.dtApplyEnd;
		pUpdateRow->PutValue(crfApplyEnd, varDate);
		varDate =  criRuleInfo.dtBasedOnStart;
		pUpdateRow->PutValue(crfBasedOnStart, varDate);
		varDate = criRuleInfo.dtBasedOnEnd;
		pUpdateRow->PutValue(crfBasedOnEnd, varDate);
		pUpdateRow->PutValue(crfCommissionPercentage, _variant_t((double)criRuleInfo.dblCommissionPercentage));
		pUpdateRow->PutValue(crfMoneyThreshold, _variant_t(criRuleInfo.cyBasedOnMoneyThreshold));
		// (j.jones 2009-11-19 10:20) - PLID 29046 - support OverwritePriorRules
		pUpdateRow->PutValue(crfOverwritePriorRules, criRuleInfo.bOverwritePriorRules ? g_cvarTrue : g_cvarFalse);
		pUpdateRow->PutValue(crfCommissionAmount, _variant_t(criRuleInfo.cyCommissionAmount)); // (b.eyers 2015-05-14) - PLID 65978

	}NxCatchAll("Error in CCommissionRuleSetupDlg::UpdateRowWithCommissionInfo");
}

// (a.wetta 2007-03-26 14:01) - PLID 25327 - This function updates the data using an sql statement made from criRuleInfo
/*void CCommissionRuleSetupDlg::UpdateDataFromCommissionRuleInfo(long lRuleID, CommissionRuleInfo criRuleInfo)
{
	try {
		CString strSQL, strTest;
		strSQL.Format("UPDATE CommissionRulesT SET Name = '%s', "
												"StartDate = '%s', "
												"EndDate = '%s', "
												"BasedOnStartDate = '%s', "
												"BasedOnEndDate = '%s', "
												"Percentage = %g, "
												"DollarThreshhold = %g "
												"WHERE ID = %li", 
												_Q(criRuleInfo.strName), 
												FormatDateTimeForSql(criRuleInfo.dtApplyStart),
												FormatDateTimeForSql(criRuleInfo.dtApplyEnd),
												FormatDateTimeForSql(criRuleInfo.dtBasedOnStart),
												FormatDateTimeForSql(criRuleInfo.dtBasedOnEnd),
												criRuleInfo.dblCommissionPercentage,
												criRuleInfo.dblBasedOnDollarThreshhold,
												lRuleID);
		ExecuteSqlStd(strSQL);

	}NxCatchAll("Error in CCommissionRuleSetupDlg::UpdateDataFromCommissionRuleInfo");
}*/

void CCommissionRuleSetupDlg::OnChangeRuleName() 
{
	try {
		// If we are intentionally making changes to the name edit box, we don't want to update the selection
		if (!m_bUpdating) {
			// Update the name in the data list
			CString strName;
			GetDlgItemText(IDC_RULE_NAME, strName);

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleList->GetCurSel();
			pRow->PutValue(crfName, _variant_t(strName));
			m_bChanged = true; // (a.vengrofski 2009-12-22 09:55) - PLID <36362> - When the user changed the rule name, it is changed; mark for save.
		}	

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeRuleName");
}

void CCommissionRuleSetupDlg::EnableEditBoxes(bool bEnable)
{
	try {
		// (a.wetta 2007-05-17 16:33) - PLID 25394 - Check their permissions to determine what they should be able to change
		BOOL bWritePermission = CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE);

		GetDlgItem(IDC_RULE_NAME)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_START_BASED_DATE)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_END_BASED_DATE)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_MONEY_THRESHOLD)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_COMMISSION_PERCENTAGE)->EnableWindow(bWritePermission && bEnable); 
		GetDlgItem(IDC_COMMISSION_AMOUNT)->EnableWindow(bWritePermission && bEnable); // (b.eyers 2015-05-14) - PLID 65978
		GetDlgItem(IDC_START_APPLY_DATE)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_END_APPLY_DATE)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_REMOVE_RULE)->EnableWindow(bWritePermission && bEnable);

		// (j.jones 2009-11-19 10:18) - PLID 29046 - supported tiered commissions
		GetDlgItem(IDC_RADIO_THIS_TIER_ONLY)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_RADIO_ALL_LOWER_TIERS)->EnableWindow(bWritePermission && bEnable);

		if (!bEnable) {
			// If we're disabling the screen let's also clear it
			m_bUpdating = true;
			UpdateCommissionInfoToScreen(CommissionRuleInfo());
			m_bUpdating = false;
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::EnableEditBoxes");
}
// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CCommissionRuleSetupDlg::OnChangeStartApplyDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		*pResult = 0;

		if (VarDateTime(m_ctrlStartApplyDate.GetValue()) == m_criCurrentInfo.dtApplyStart)
			// No change was made
			return;

		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		CString str;
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			// (a.walling 2008-05-14 12:48) - PLID 27591 - variants no longer necessary
			if (m_criCurrentInfo.dtApplyStart == 0)
				m_ctrlStartApplyDate.SetValue((COleDateTime::GetCurrentTime()));
			else
				m_ctrlStartApplyDate.SetValue((m_criCurrentInfo.dtApplyStart));
		}
		else {
			m_criCurrentInfo.dtApplyStart = m_ctrlStartApplyDate.GetValue();

			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeStartApplyDate");	
}
// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CCommissionRuleSetupDlg::OnChangeEndBasedDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		*pResult = 0;

		if (VarDateTime(m_ctrlEndBasedDate.GetValue()) == m_criCurrentInfo.dtBasedOnEnd)
			// No change was made
			return;

		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		CString str;
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			// (a.walling 2008-05-14 12:48) - PLID 27591 - variants no longer necessary
			if (m_criCurrentInfo.dtBasedOnEnd == 0)
				m_ctrlEndBasedDate.SetValue((COleDateTime::GetCurrentTime()));
			else
				m_ctrlEndBasedDate.SetValue((m_criCurrentInfo.dtBasedOnEnd));
		}
		else {
			m_criCurrentInfo.dtBasedOnEnd = m_ctrlEndBasedDate.GetValue();

			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeEndBasedDate");		
}
// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CCommissionRuleSetupDlg::OnChangeEndApplyDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		*pResult = 0;

		if (VarDateTime(m_ctrlEndApplyDate.GetValue()) == m_criCurrentInfo.dtApplyEnd)
			// No change was made
			return;

		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		CString str;
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			// (a.walling 2008-05-14 12:48) - PLID 27591 - variants no longer necessary
			if (m_criCurrentInfo.dtApplyEnd == 0)
				m_ctrlEndApplyDate.SetValue((COleDateTime::GetCurrentTime()));
			else
				m_ctrlEndApplyDate.SetValue((m_criCurrentInfo.dtApplyEnd));
		}
		else {
			m_criCurrentInfo.dtApplyEnd = m_ctrlEndApplyDate.GetValue();

			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeEndApplyDate");		
}
// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CCommissionRuleSetupDlg::OnChangeStartBasedDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		*pResult = 0;

		if (VarDateTime(m_ctrlStartBasedDate.GetValue()) == m_criCurrentInfo.dtBasedOnStart)
			// No change was made
			return;

		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		CString str;
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			// (a.walling 2008-05-14 12:48) - PLID 27591 - variants no longer necessary
			if (m_criCurrentInfo.dtBasedOnStart == 0)
				m_ctrlStartBasedDate.SetValue((COleDateTime::GetCurrentTime()));
			else
				m_ctrlStartBasedDate.SetValue((m_criCurrentInfo.dtBasedOnStart));
		}
		else {
			m_criCurrentInfo.dtBasedOnStart = m_ctrlStartBasedDate.GetValue();

			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeStartBasedDate");	
}

void CCommissionRuleSetupDlg::OnShowOutdated() 
{
	try {
		if (m_mapChangedRules.GetCount() > 0 || m_mapAddRules.GetCount() > 0 || m_aryDeleteRules.GetSize() > 0 || m_bChanged) {
			long nResult = MessageBox("The rule list will refresh but you have made changes to the rules.  "
				"Would you like to save your changes before the list refreshes?  If not, all of your changes will be lost.", NULL, MB_YESNOCANCEL|MB_ICONWARNING);
			if (nResult == IDYES) {	
				if (!Save()) {
					// The save wasn't successful, put the check back to how it was
					CheckDlgButton(IDC_SHOW_OUTDATED, !IsDlgButtonChecked(IDC_SHOW_OUTDATED));
					return;
				}
			}
			else if (nResult == IDCANCEL) {
				// Put the check back to how it was
				CheckDlgButton(IDC_SHOW_OUTDATED, !IsDlgButtonChecked(IDC_SHOW_OUTDATED));
				return;
			}
			else {
				m_bChanged = false;
				m_mapAddRules.RemoveAll();
				m_mapChangedRules.RemoveAll();
				m_aryDeleteRules.RemoveAll();
			}
		}

		// Get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleList->GetCurSel();
		long nID = -1;
		if (pRow) {
			nID = VarLong(pRow->GetValue(crfID));
		}

		// (j.jones 2009-11-18 14:48) - PLID 29046 - m_bUseTieredRules determines
		// whether we show rules where IsTieredCommission = 1, or not
		CString strWhere = "";
		if(m_bUseTieredRules) {
			strWhere = "IsTieredCommission = 1";
		}
		else {
			strWhere = "IsTieredCommission = 0";
		}

		// Update the rules list
		if(!IsDlgButtonChecked(IDC_SHOW_OUTDATED)) {
			strWhere += " AND EndDate >= dbo.AsDateNoTime(GetDate())";
		}

		m_pRuleList->PutWhereClause(_bstr_t(strWhere));
		m_pRuleList->Requery();

		// Try to reselect the previously selected row
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nResult = m_pRuleList->TrySetSelByColumn_Deprecated(crfID, _variant_t(nID));
		if (nResult >= 0) {
			SelChangedCommissionRuleList(NULL, m_pRuleList->GetCurSel());
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnShowOutdated");
	
}

void CCommissionRuleSetupDlg::OnTrySetSelFinishedCommissionRuleList(long nRowEnum, long nFlags) 
{
	try {
		// Handle the row change to whatever it's on now
		SelChangedCommissionRuleList(NULL, m_pRuleList->GetCurSel());

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnTrySetSelFinishedCommissionRuleList");	
}

void CCommissionRuleSetupDlg::OnKillfocusRuleName() 
{
	try {
		CString str;
		GetDlgItemText(IDC_RULE_NAME, str);
		if (str == m_criCurrentInfo.strName)
			// No changes were made
			return;

		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the name
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRuleList->GetCurSel();
			pRow->PutValue(crfName, _variant_t(m_criCurrentInfo.strName));
			SetDlgItemText(IDC_RULE_NAME, m_criCurrentInfo.strName);
		}
		else {
			GetDlgItemText(IDC_RULE_NAME, m_criCurrentInfo.strName);

			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnKillfocusRuleName");	
}

void CCommissionRuleSetupDlg::OnKillfocusMoneyThreshold() 
{
	try {
		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			SetDlgItemText(IDC_MONEY_THRESHOLD, FormatCurrencyForInterface(m_criCurrentInfo.cyBasedOnMoneyThreshold, TRUE, TRUE));
		}
		else {
			CString strMoneyThreshold;
			GetDlgItemText(IDC_MONEY_THRESHOLD, strMoneyThreshold);
			COleCurrency cyThreshold = ParseCurrencyFromInterface(strMoneyThreshold);
			if (strMoneyThreshold.IsEmpty()) {
				SetDlgItemText(IDC_MONEY_THRESHOLD, FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
				m_criCurrentInfo.cyBasedOnMoneyThreshold = COleCurrency(0,0);
				return;
			}
			//make sure that it is a valid currency
			else if (cyThreshold.GetStatus() == COleCurrency::invalid) {
				MessageBox("Please enter a valid money amount.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_MONEY_THRESHOLD, FormatCurrencyForInterface(m_criCurrentInfo.cyBasedOnMoneyThreshold, TRUE, TRUE));
				GetDlgItem(IDC_MONEY_THRESHOLD)->SetFocus();
				return;
			}
			else if (m_criCurrentInfo.cyBasedOnMoneyThreshold == cyThreshold) {
				// No changes were made
				return;
			}
			// Make sure the money amount is not negative
			else if (cyThreshold < COleCurrency(0,0)) {
				MessageBox("The money amount cannot be less than zero. Please fix this before continuing.", NULL, MB_OK|MB_ICONWARNING);
				SetDlgItemText(IDC_MONEY_THRESHOLD, FormatCurrencyForInterface(m_criCurrentInfo.cyBasedOnMoneyThreshold, TRUE, TRUE));
				GetDlgItem(IDC_MONEY_THRESHOLD)->SetFocus();
				return;
			}

			SetDlgItemText(IDC_MONEY_THRESHOLD, FormatCurrencyForInterface(cyThreshold, TRUE, TRUE));
			m_criCurrentInfo.cyBasedOnMoneyThreshold = cyThreshold;
	
			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnKillfocusMoneyThreshold");
}

void CCommissionRuleSetupDlg::OnKillfocusCommissionPercentage() 
{
	try {
		// (a.wetta 2007-05-18 13:27) - PLID 25394 - We can only have gotten this far if we have permission to write or write
		// with password, so check the password now if needed
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			SetDlgItemText(IDC_COMMISSION_PERCENTAGE, FormatString("%g%%", m_criCurrentInfo.dblCommissionPercentage));
		}
		else {
			CString strPercent;
			GetDlgItemText(IDC_COMMISSION_PERCENTAGE, strPercent);
			strPercent.TrimRight("%");
			double dblPercent = atof(strPercent);
			if (m_criCurrentInfo.dblCommissionPercentage == dblPercent) {
				// No changes were made
				SetDlgItemText(IDC_COMMISSION_PERCENTAGE, FormatString("%g%%", dblPercent));
				return;
			}
			else if (dblPercent < 0) {
				MessageBox("The commission percentage cannot be less than zero. Please fix this before continuing.", NULL, MB_OK|MB_ICONWARNING);
				// Reset the text
				SetDlgItemText(IDC_COMMISSION_PERCENTAGE, FormatString("%g%%", m_criCurrentInfo.dblCommissionPercentage));
				GetDlgItem(IDC_COMMISSION_PERCENTAGE)->SetFocus();
				return;
			}

			SetDlgItemText(IDC_COMMISSION_PERCENTAGE, FormatString("%g%%", dblPercent));
			m_criCurrentInfo.dblCommissionPercentage = dblPercent;

			m_bChanged = true;	
		}
	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnKillfocusCommissionPercentage");
}

// (j.jones 2009-11-19 10:20) - PLID 29046 - supported OverwritePriorRules
void CCommissionRuleSetupDlg::OnRadioThisTierOnly()
{
	try {

		BOOL bOverwritePriorRules = m_radioAllLowerTiers.GetCheck();
		if(m_criCurrentInfo.bOverwritePriorRules == bOverwritePriorRules) {
			//nothing changed
			return;
		}

		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			//reset the screen
			m_radioThisTierOnly.SetCheck(!m_criCurrentInfo.bOverwritePriorRules);
			m_radioAllLowerTiers.SetCheck(m_criCurrentInfo.bOverwritePriorRules);
			return;
		}
		else {
			m_criCurrentInfo.bOverwritePriorRules = bOverwritePriorRules;
			m_bChanged = true;
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnRadioThisTierOnly");
}

void CCommissionRuleSetupDlg::OnRadioAllLowerTiers()
{
	try {

		OnRadioThisTierOnly();

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnRadioAllLowerTiers");
}

void CCommissionRuleSetupDlg::OnChangeMoneyThreshold() 
{
	try {
		// If we are intentionally making changes to the Money Threshold box
		if (!m_bUpdating) {
			m_bChanged = true; // (a.vengrofski 2009-12-22 09:55) - PLID <36362> - When the user changed the threshold, it is changed; mark for save.
		}	

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeMoneyThreshold");
}

void CCommissionRuleSetupDlg::OnChangeCommissionPercentage() 
{
	try {
		// If we are intentionally making changes to the Commission Percentage box
		if (!m_bUpdating) {
			m_bChanged = true; // (a.vengrofski 2009-12-22 09:55) - PLID <36362> - When the user changed the commission rate, it is changed; mark for save.
		}	

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeCommissionPercentage");
}

// (b.eyers 2015-05-14) - PLID 65978 - ability to switch between % and $ when hyperlink is clicked
LRESULT CCommissionRuleSetupDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{

		if (m_bPercent) {
			//currently set to %, change to $
			m_nxlabelCommissionLabel.SetText("Give the provider the following $ commission:");
			m_bPercent = FALSE;
			//TES 7/20/2015 - PLID 66600 - Show the label for $ commissions (if we're showing tiers), hide the radio buttons
			GetDlgItem(IDC_COMMISSION_PERCENTAGE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COMMISSION_AMOUNT)->ShowWindow(SW_SHOW);
			if (m_bUseTieredRules) {
				GetDlgItem(IDC_DOLLAR_TIER_COMMISSION_LABEL)->ShowWindow(SW_SHOW);
			}
			GetDlgItem(IDC_APPLY_COMMISSION_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_THIS_TIER_ONLY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_ALL_LOWER_TIERS)->ShowWindow(SW_HIDE);
		}
		else {
			m_nxlabelCommissionLabel.SetText("Give the provider the following % commission:");
			m_bPercent = TRUE;
			//TES 7/20/2015 - PLID 66600 - Hide the label for $ commissions (if we're showing tiers), show the radio buttons
			GetDlgItem(IDC_COMMISSION_PERCENTAGE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMMISSION_AMOUNT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DOLLAR_TIER_COMMISSION_LABEL)->ShowWindow(SW_HIDE);
			if (m_bUseTieredRules) {
				GetDlgItem(IDC_APPLY_COMMISSION_LABEL)->ShowWindow(SW_SHOWNA);
				GetDlgItem(IDC_RADIO_THIS_TIER_ONLY)->ShowWindow(SW_SHOWNA);
				GetDlgItem(IDC_RADIO_ALL_LOWER_TIERS)->ShowWindow(SW_SHOWNA);
			}
		}

	}NxCatchAll("Error in OnLabelClick");
	return 0;
}

// (b.eyers 2015-05-14) - PLID 65978 - check that $ amount is entered correctly
void CCommissionRuleSetupDlg::OnKillfocusCommissionAmount()
{
	try {
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			// Reset the text
			SetDlgItemText(IDC_COMMISSION_AMOUNT, FormatCurrencyForInterface(m_criCurrentInfo.cyCommissionAmount, TRUE, TRUE));
		}
		else {
			CString strValue;
			COleCurrency cyTmp;
			GetDlgItemText(IDC_COMMISSION_AMOUNT, strValue);
			cyTmp = ParseCurrencyFromInterface(strValue);

			if (strValue == "") {
				cyTmp = COleCurrency(0, 0);
			}

			BOOL bFailed = FALSE;
			if (cyTmp.GetStatus() == COleCurrency::invalid) {
				AfxMessageBox("Please enter a valid amount greater than $0.00.");
				bFailed = TRUE;
			}

			if (!bFailed && cyTmp < COleCurrency(0, 0)) {
				AfxMessageBox("Please enter a valid amount greater than $0.00.");
				bFailed = TRUE;
			}

			if (!bFailed && cyTmp > COleCurrency(100000000, 0)) {
				CString str;
				str.Format("Practice does not allow an amount greater than %s.", FormatCurrencyForInterface(COleCurrency(100000000, 0), TRUE, TRUE));
				AfxMessageBox(str);
				bFailed = TRUE;
			}

			if (bFailed) {
				//load the previous amount
				SetDlgItemText(IDC_COMMISSION_AMOUNT, FormatCurrencyForInterface(m_criCurrentInfo.cyCommissionAmount));
			}
			else {
				SetDlgItemText(IDC_COMMISSION_AMOUNT, FormatCurrencyForInterface(cyTmp));
				m_criCurrentInfo.cyCommissionAmount = cyTmp;
			}

			m_bChanged = true;
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnKillfocusCommissionAmount");
}

// (b.eyers 2015-05-14) - PLID 65978
void CCommissionRuleSetupDlg::OnChangeCommissionAmount()
{
	try {
		if (!m_bUpdating) {

			m_bChanged = true; 
		}

	}NxCatchAll("Error in CCommissionRuleSetupDlg::OnChangeCommissionAmount");
}