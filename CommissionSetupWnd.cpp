// CommissionSetupWnd.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "CommissionSetupWnd.h"
#include "globalfinancialutils.h"
#include "InternationalUtils.h"
#include "GlobalUtils.h"
#include "AdvCommissionSetupDlg.h"
#include "CommissionRuleSetupDlg.h"
#include "AuditTrail.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum CommissionListFields {
	clfBufferSpace = 0,
	clfID,
	clfParentID,
	clfRuleID,
	clfType,
	clfCategory,
	clfCode,
	clfName,
	clfAmount, // (b.eyers 2015-05-11) - PLID 65976
	clfOldAmount, // (b.eyers 2015-05-11) - PLID 65976
	clfPercentage,
	clfOldPercent,
	clfTypeOfChange,
};

// (j.jones 2009-11-18 16:08) - PLID 29046 - added tiered commissions
enum TieredRuleListColumns {

	trlcRuleID = 0,
	trlcTypeID,
	trlcTypeName,
	trlcName,
	trlcAmount, // (b.eyers 2015-05-15) - PLID 65982
	trlcPercent,
	trlcMoneyThreshold,
	trlcOverwritePriorRules,
	trlcStartDate,
	trlcEndDate,
};

// (j.jones 2009-11-19 11:53) - PLID 29046 - added provider enum
enum ProviderComboColumns {

	pccID = 0,
	pccName,
	pccUseTieredCommissions,
	pccIgnoreShopFee,
};

enum RuleListFields {
	rlfID = 0,
	rlfName,
	rlfPercentage,
};

// (a.wetta 2007-03-28 16:09) - PLID 25360 - This window was created to make the commission setup more portable.  I copied some functions
// from the previous commission setup dialog, but for the most part everything in this file is new or has been modified.

void RoundDouble(double &dbl)
{
	//Josh says they do this everywhere in Billing, it's the most reliable way of
	//	rounding a double that we've found.
	_RecordsetPtr prs = CreateRecordset("SELECT convert(float, Round(%g, 2)) AS Value", dbl);
	if(prs->eof)
		AfxThrowNxException("Failure to round double value!");

	dbl = AdoFldDouble(prs, "Value");
}

/////////////////////////////////////////////////////////////////////////////
// CCommissionSetupWnd dialog


CCommissionSetupWnd::CCommissionSetupWnd(CWnd* pParent)
	: CNxDialog(CCommissionSetupWnd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCommissionSetupWnd)
	//}}AFX_DATA_INIT
	m_nProviderID = -1;
	m_bChanged = false;
	m_backgroundColor = GetNxColor(GNC_ADMIN, 0);
	m_bTabMode = FALSE;
	m_dblPercent = 0;
	m_cyAmount = COleCurrency(0, 0); // (b.eyers 2015-05-13) - PLID 65977
}


void CCommissionSetupWnd::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommissionSetupWnd)
	DDX_Control(pDX, IDC_SHOW_OUTDATED_RULES, m_btnShowOutdatedRules);
	DDX_Control(pDX, IDC_HIDESHOW_RULES, m_btnHideShowRules);
	DDX_Control(pDX, IDC_ADV_SETUP, m_btnAdvSetup);
	DDX_Control(pDX, IDC_COMM_RULE_SETUP, m_btnCommRuleSetup);
	DDX_Control(pDX, IDC_APPLY_QUICK_RULE, m_btnApplyQuickRule);
	DDX_Control(pDX, IDC_APPLY_QUICK_COMM, m_btnApplyQuickComm);
	DDX_Control(pDX, IDC_COMM_COLOR, m_bkgColor);
	DDX_Control(pDX, IDC_QUICK_COMM, m_nxeditQuickComm);
	DDX_Control(pDX, IDC_COMMISSION_TITLE, m_nxstaticCommissionTitle);
	DDX_Control(pDX, IDC_RADIO_BASIC_COMMISSIONS, m_radioBasicCommissions);
	DDX_Control(pDX, IDC_RADIO_TIERED_COMMISSIONS, m_radioTieredCommissions);
	DDX_Control(pDX, IDC_CHECK_IGNORE_SHOP_FEE, m_checkIgnoreShopFee);
	DDX_Control(pDX, IDC_COMMISSION_QUICK_SETUP_LABEL, m_nxstaticQuickSetupLabel);
	DDX_Control(pDX, IDC_SET_COMMISSION_LABEL_1, m_nxstaticSetCommissionLabel1);
	DDX_Control(pDX, IDC_SET_COMMISSION_LABEL_2, m_nxstaticSetCommissionLabel2);
	DDX_Control(pDX, IDC_CURRENT_COMMISSION_LABEL, m_nxstaticCurrentCommissionLabel);	
	DDX_Control(pDX, IDC_QUICK_COMM_AMOUNT, m_nxeditQuickCommAmount); // (b.eyers 2015-05-13) - PLID 65977
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCommissionSetupWnd, CNxDialog)
	//{{AFX_MSG_MAP(CCommissionSetupWnd)
	ON_BN_CLICKED(IDC_ADV_SETUP, OnAdvSetup)
	ON_BN_CLICKED(IDC_COMM_RULE_SETUP, OnCommRuleSetup)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_SHOW_OUTDATED_RULES, OnShowOutdatedRules)
	ON_BN_CLICKED(IDC_APPLY_QUICK_COMM, OnApplyQuickComm)
	ON_BN_CLICKED(IDC_APPLY_QUICK_RULE, OnApplyQuickRule)
	ON_BN_CLICKED(IDC_HIDESHOW_RULES, OnHideshowRules)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_QUICK_COMM, OnKillfocusQuickComm)
	ON_BN_CLICKED(IDC_RADIO_BASIC_COMMISSIONS, OnRadioBasicCommissions)
	ON_BN_CLICKED(IDC_RADIO_TIERED_COMMISSIONS, OnRadioTieredCommissions)
	ON_BN_CLICKED(IDC_CHECK_IGNORE_SHOP_FEE, OnCheckIgnoreShopFee)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick) // (b.eyers 2015-05-13) - PLID 65977
	ON_EN_KILLFOCUS(IDC_QUICK_COMM_AMOUNT, OnKillfocusQuickCommAmount) // (b.eyers 2015-05-13) - PLID 65977
	//}}AFX_MSG_MAP		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommissionSetupWnd message handlers

BOOL CCommissionSetupWnd::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnApplyQuickRule.AutoSet(NXB_MODIFY);
		m_btnApplyQuickComm.AutoSet(NXB_MODIFY);
		m_btnCommRuleSetup.AutoSet(NXB_MODIFY);
		m_btnAdvSetup.AutoSet(NXB_MODIFY);
		
		//bind datalists
		m_pProvCombo = BindNxDataList2Ctrl(IDC_COMMISSION_PROV_LIST, false);		

		m_pQuickRuleCombo = BindNxDataList2Ctrl(IDC_QUICK_RULE_LIST, false);

		m_pCommTypeCombo = BindNxDataList2Ctrl(IDC_COMM_ITEM_TYPE, true);

		m_pRuleTypeCombo = BindNxDataList2Ctrl(IDC_RULE_ITEM_TYPE, true);

		m_pCurrentList = BindNxDataList2Ctrl(IDC_CURRENT_LIST, false);

		// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
		m_pTieredRuleList = BindNxDataList2Ctrl(IDC_TIERED_RULE_LIST, false);

		// (b.savon 2011-12-07 08:38) - PLID 46902 - Enforce Provider Commission Permissions
		CPermissions pmProviderCommission = GetCurrentUserPermissions(bioProviderCommission);
		if (pmProviderCommission & (sptWrite|sptRead) ){

			Load();

			m_aryAddedBasicRules.RemoveAll();
			m_aryRemovedBasicRules.RemoveAll();
			m_aryAddedTieredRules.RemoveAll();
			m_aryRemovedTieredRules.RemoveAll();

			// (a.wetta 2007-03-29 13:37) - PLID 25407 - Make sure that if there is no provider selected the controls are not enabled
			EnableControls(m_nProviderID == -1 ? FALSE : TRUE);

			// (b.savon 2011-12-07 09:41) - PLID 46902 - Enforce Provider Commission Permissions
			BOOL bWrite = pmProviderCommission & (sptWrite);
			GetDlgItem(IDC_ADV_SETUP)->EnableWindow(bWrite);
			GetDlgItem(IDC_COMM_RULE_SETUP)->EnableWindow(bWrite);

		} else{
			EnforceNoProviderCommissionPermissions();
		}

		// (a.wetta 2007-03-30 11:04) - PLID 24872 - Set the background color
		m_bkgColor.SetColor(m_backgroundColor);

		if (m_bTabMode) {
			// (a.wetta 2007-05-21 14:23) - PLID 25960 - Bold the title
			extern CPracticeApp theApp;
			GetDlgItem(IDC_COMMISSION_TITLE)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		}
		else {
			// This is a pop up dialog, so hide the title
			GetDlgItem(IDC_COMMISSION_TITLE)->ShowWindow(FALSE);
		}

		// (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
		SetControlPositions();

		// Set the percent edit box default text
		CString strPercent;
		strPercent.Format("%.2f", m_dblPercent); 
		SetDlgItemText(IDC_QUICK_COMM, strPercent);

		// (b.eyers 2015-05-13) - PLID 65977 - Changed to hyperlink to allow changing between applying % and $
		m_nxstaticSetCommissionLabel2.SetType(dtsHyperlink);
		m_nxstaticSetCommissionLabel2.SetText("% for all");
		m_bPercent = TRUE; //default is %


	}NxCatchAll("Error in CCommissionSetupWnd::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CCommissionSetupWnd, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCommissionSetupWnd)
	ON_EVENT(CCommissionSetupWnd, IDC_CURRENT_LIST, 10 /* EditingFinished */, OnEditingFinishedCurrentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCommissionSetupWnd, IDC_CURRENT_LIST, 6 /* RButtonDown */, OnRButtonDownCurrentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCommissionSetupWnd, IDC_COMMISSION_PROV_LIST, 1 /* SelChanging */, OnSelChangingCommissionProvList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CCommissionSetupWnd, IDC_CURRENT_LIST, 8 /* EditingStarting */, OnEditingStartingCurrentList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CCommissionSetupWnd, IDC_CURRENT_LIST, 9 /* EditingFinishing */, OnEditingFinishingCurrentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CCommissionSetupWnd, IDC_COMMISSION_PROV_LIST, 16, OnSelChosenCommissionProvList, VTS_DISPATCH)
	ON_EVENT(CCommissionSetupWnd, IDC_TIERED_RULE_LIST, 6, OnRButtonDownTieredRuleList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP		
END_EVENTSINK_MAP()

// (a.wetta 2007-03-27 14:04) - PLID 25360 - This function returns TRUE if it's safe to close the window, FALSE if not
BOOL CCommissionSetupWnd::CloseWindow(BOOL bSaveData)
{
	try{
		if(m_bChanged) {
			if (bSaveData) {
				if (m_nProviderID == -1)
					// There is nothing to save
					return TRUE;
				else
					return Save();
			}
			else {
				if(AfxMessageBox("You have changed the values of some items.  If you continue, these changes will be lost.  Are you sure you wish to cancel?", MB_YESNO) == IDYES)
					return TRUE;
				else
					return FALSE;
			}
		}

	}NxCatchAll("Error in CCommissionSetupWnd::CloseWindow");
	return TRUE;
}

// (j.jones 2009-11-18 11:05) - PLID 29046 - renamed RefreshCurrentList() to Load()
// as it now does much more than simply requery a list
void CCommissionSetupWnd::Load()
{
	try {

		CWaitCursor pWait;

		BOOL bUseTieredCommissions = FALSE;
		BOOL bIgnoreShopFee = FALSE;

		// (j.jones 2009-11-19 11:55) - PLID 29046 - reload the provider combo,
		// which really should have been happening all along
		m_pProvCombo->Requery();

		// Let's see if m_nProviderID is an inactive provider
		IRowSettingsPtr pProviderRow = NULL;		
		if(m_nProviderID != -1) {
			_RecordsetPtr prsProvInfo = CreateParamRecordset("SELECT Archived, Last, First, Middle, Title, "
				"UseTieredCommissions, IgnoreShopFee "
				"FROM PersonT "
				"INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID = {INT}", m_nProviderID);
			if (!prsProvInfo->eof) {
				if (AdoFldBool(prsProvInfo, "Archived")) {
					// This provider is inactive, let's add them to the provider list before selecting them
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProvCombo->GetNewRow();
					pRow->PutValue(pccID, _variant_t(m_nProviderID));
					CString strProvName;
					strProvName.Format("%s, %s %s %s", AdoFldString(prsProvInfo, "Last"),
													AdoFldString(prsProvInfo, "First"),
													AdoFldString(prsProvInfo, "Middle"),
													AdoFldString(prsProvInfo, "Title"));
					pRow->PutValue(pccName, _variant_t(strProvName));
					// (j.jones 2009-11-19 11:55) - PLID 29046 - load new commission fields
					pRow->PutValue(pccUseTieredCommissions, prsProvInfo->Fields->Item["UseTieredCommissions"]->Value);
					pRow->PutValue(pccIgnoreShopFee, prsProvInfo->Fields->Item["IgnoreShopFee"]->Value);
					m_pProvCombo->AddRowSorted(pRow, NULL);
				}	
				// Select the provider in the combo
				pProviderRow = m_pProvCombo->SetSelByColumn(pccID, m_nProviderID);
			}
			else {
				// This provider ID doesn't seem to be valid
				m_nProviderID = -1;
			}
		}

		// (a.wetta 2007-05-17 10:59) - PLID 25960 - If there is no provider information selected there is no reason to 
		// requery the datalist, this just makes it slow to load
		if (m_nProviderID != -1) {
			CString str, strWhere1, strWhere2;

			// (j.jones 2009-11-18 11:16) - PLID 29046 - get the current commission type and shop fee setting
			// for the selected provider
			bUseTieredCommissions = VarBool(pProviderRow->GetValue(pccUseTieredCommissions), FALSE);
			bIgnoreShopFee = VarBool(pProviderRow->GetValue(pccIgnoreShopFee), FALSE);

			if(!IsDlgButtonChecked(IDC_SHOW_OUTDATED_RULES)) {
				strWhere2 = "AND EndDate >= dbo.AsDateNoTime(GetDate())";
			}
			else {
				strWhere2 = "";
			}

			strWhere1.Format("IsTieredCommission = 1 AND ProviderID = %li %s", m_nProviderID, strWhere2);

			m_pTieredRuleList->PutWhereClause(_bstr_t(strWhere1));
			//requery later in this function
		
			str.Format("(SELECT ServiceT.ID AS ID, NULL AS ParentID, NULL AS RuleID,"
					"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 'Service Code' WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' WHEN GCTypesT.ServiceID IS NOT NULL THEN 'Gift Certificate' ELSE 'Service Code' END AS Type, "
					"CASE WHEN CPTCodeT.ID IS NOT NULL THEN CPTCodeT.Code WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' WHEN GCTypesT.ServiceID IS NOT NULL THEN 'Gift Certificate' END AS Code, ServiceT.Name AS Name, Percentage, CategoriesT.Name AS Category, Amount " // (b.eyers 2015-05-11) - PLID 65976
					"FROM ServiceT "
					"LEFT JOIN (SELECT * FROM CommissionT WHERE ProvID = %li) CommissionT ON ServiceT.ID = CommissionT.ServiceID "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN GCTypesT ON ServiceT.ID = GCTypesT.ServiceID "
					"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
					// (j.jones 2010-05-03 10:49) - PLID 37747 - Exclude inactive codes
					"WHERE ServiceT.Active = 1"
					"UNION "
					"SELECT NULL AS ID, ServiceID AS ParentID, CommissionRulesT.ID AS RuleID, '{Rule}' AS Type, '' AS Code, Name, Percentage, '' AS Category, Amount " // (b.eyers 2015-05-14) - PLID 65978
					"FROM CommissionRulesT "
					// (b.eyers 07/18/2014) - PLID 62393 - Added check on active in CommissionRulesLinkT
					"LEFT JOIN (SELECT RuleID, ProvID, ServiceID FROM CommissionRulesLinkT LEFT JOIN ServiceT ON CommissionRulesLinkT.ServiceID = ServiceT.ID WHERE ProvID = %li and Active = 1) CommissionRulesLinkT ON CommissionRulesLinkT.RuleID = CommissionRulesT.ID "
					"WHERE ProvID = %li %s) CommissionQ", m_nProviderID, m_nProviderID, m_nProviderID, strWhere2);
			m_pCurrentList->PutFromClause(_bstr_t(str));
			//requery later in this function
		}
		else {
			m_pCurrentList->Clear();
			m_pTieredRuleList->Clear();
		}

		// The list has just been requeried, so all of the rules are hidden
		m_btnHideShowRules.SetWindowText("Show All Rules");

		// (j.jones 2009-11-18 11:10) - PLID 29046 - set the commission type,
		// then call DisplayControls to show/hide the appropriate interface
		m_radioBasicCommissions.SetCheck(!bUseTieredCommissions);
		m_radioTieredCommissions.SetCheck(bUseTieredCommissions);
		DisplayControls(bUseTieredCommissions);

		m_checkIgnoreShopFee.SetCheck(bIgnoreShopFee);

		//requery the commission lists last
		if(m_nProviderID != -1) {
			m_pTieredRuleList->Requery();
			m_pCurrentList->Requery();
		}

		m_bChanged = false;

	}NxCatchAll("Error in CCommissionSetupWnd::RefreshCurrentList");
}

void CCommissionSetupWnd::OnAdvSetup() 
{
	try {
		if (m_nProviderID != -1 && m_bChanged) {
			if(AfxMessageBox("Would you like to save the changes made to the current provider?\n"
				"Any unsaved changes will be lost.", MB_YESNO) == IDYES) {
				Save();
			}
		}

		CAdvCommissionSetupDlg dlg(this);
		// (a.wetta 2007-03-30 10:59) - PLID 24872 - Set the background color
		dlg.m_backgroundColor = m_backgroundColor;
		dlg.DoModal();
		
		// Changes may have been made to the rules, so the datalists need to be requeried
		Load();

	}NxCatchAll("Error in CCommissionSetupWnd::OnAdvSetup");
}

void CCommissionSetupWnd::OnCommRuleSetup() 
{
	LaunchCommissionRuleEditor();
}

void CCommissionSetupWnd::LaunchCommissionRuleEditor()
{
	try {
		// (a.wetta 2007-04-24 09:09) - PLID 24872 - Be sure to save any changes
		if (m_nProviderID != -1 && m_bChanged) {
			if(AfxMessageBox("Would you like to save the changes made to the current provider?\n"
				"Any unsaved changes will be lost.", MB_YESNO) == IDYES) {
				Save();
			}
		}

		CCommissionRuleSetupDlg dlg(this);
		// (a.wetta 2007-03-30 10:59) - PLID 24872 - Set the background color
		dlg.m_backgroundColor = m_backgroundColor;
		// (j.jones 2009-11-18 14:48) - PLID 29046 - tell the dialog which rule types to load
		dlg.m_bUseTieredRules = m_radioTieredCommissions.GetCheck();
		if (IDOK == dlg.DoModal()) {
			// Changes may have been made to the rules, so the datalists need to be requeried

			//we don't need to call Load() because only the rules changed,
			//we can just directly requery the rule lists with the current
			//where clauses

			m_pQuickRuleCombo->Requery();

			if(m_nProviderID != -1) {

				// (j.jones 2009-11-20 11:56) - PLID 29046 - we could have only edited one type
				// of list, so there's no point of requerying both
				if(m_radioTieredCommissions.GetCheck()) {
					m_pTieredRuleList->Requery();
				}
				else {
					m_pCurrentList->Requery();
				}
			}
		}

	}NxCatchAll("Error in CCommissionSetupWnd::LaunchCommissionRuleEditor");
}

void CCommissionSetupWnd::OnEditingStartingCurrentList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (a.wetta 2007-05-17 15:33) - PLID 25394 - Make sure they have permission to write
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE)) {
			*pbContinue = false;
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow->GetValue(clfID).vt == VT_NULL) {
			// This is a rule row and you can't edit a rule from the datalist
			*pbContinue = false;
		}

	}NxCatchAll("Error in CCommissionSetupWnd::OnEditingStartingCurrentList");
}

void CCommissionSetupWnd::OnEditingFinishedCurrentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if (bCommit && nCol == clfPercentage) {
			// The user is changing the base commission for this service item
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			//(e.lally 2007-10-12) PLID 25360 - If one is null and the other is 0, nothing has actually changed.
			if ((varOldValue.vt == VT_NULL && varNewValue.vt == VT_NULL) ||
				(varOldValue.vt != VT_EMPTY && varNewValue.vt != VT_EMPTY && VarDouble(varOldValue, 0.00) == VarDouble(varNewValue, 0.00))) {
				// They didn't actually make a change
				if(VarDouble(varNewValue, 0.00) == 0.00){
					_variant_t varNull;
					varNull.vt = VT_NULL;
					//update this row to have no percentage
					pRow->PutValue(clfPercentage, varNull);
				}
				return;
			}

			if (VarDouble(pRow->GetValue(clfPercentage)) == 0){
				RemovePercentage(pRow);
			}
			else{
				ApplyPercentage(pRow, VarDouble(varNewValue));
			}

			if (m_nProviderID != -1 && m_bTabMode) {
				Save();
			}
		}
		// (b.eyers 2015-05-11) - PLID 65976 - Changes were made to the Amount column
		else if (bCommit && nCol == clfAmount) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if ((varOldValue.vt == VT_NULL && varNewValue.vt == VT_NULL) ||
				(varOldValue.vt != VT_EMPTY && varNewValue.vt != VT_EMPTY && VarCurrency(varOldValue, COleCurrency(0, 0)) == VarCurrency(varNewValue, COleCurrency(0, 0)))) {
				if (VarCurrency(varNewValue, COleCurrency(0, 0)) == COleCurrency(0, 0)){
					_variant_t varNull;
					varNull.vt = VT_NULL;
					pRow->PutValue(clfAmount, varNull);
				}
				return;
			}
			if (VarCurrency(pRow->GetValue(clfAmount)) == COleCurrency(0, 0)) {
				RemoveAmount(pRow);
			}
			else {
				ApplyAmount(pRow, VarCurrency(varNewValue));
			}

			if (m_nProviderID != -1 && m_bTabMode) {
				Save();
			}
		}
	}NxCatchAll("Error in CCommissionSetupWnd::OnEditingFinishedCurrentList");
}

void CCommissionSetupWnd::OnRButtonDownCurrentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		// Set the row to whatever they right clicked on
		// Actual work is done in the ContextMenu function
		GetDlgItem(IDC_CURRENT_LIST)->SetFocus();
		m_pCurrentList->PutCurSel(pRow);
	}NxCatchAll("Error in CCommissionSetupWnd::OnRButtonDownCurrentList");
}

void CCommissionSetupWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		const int ADD_RULE = 1;

		// (j.jones 2009-11-19 09:33) - PLID 29046 - have a different menu for each list
		if(!m_radioTieredCommissions.GetCheck()) {

			//basic commissions

			//Check to see if the focus is on our current commission list
			CWnd* pCommissionListWnd = GetDlgItem(IDC_CURRENT_LIST);
			if(pWnd->GetSafeHwnd() == pCommissionListWnd->GetSafeHwnd()) {
				//The commission list has focus, so create our menu
				CMenu mnu;
				mnu.LoadMenu(IDR_COMMISSION_POPUP);
				CMenu* pSubMenu = mnu.GetSubMenu(0);
				if(pSubMenu) {
					NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pCurrentList->GetCurSel();
					if (pCurSel == NULL) {
						// Nothing is selected
						pSubMenu->EnableMenuItem(ADD_RULE, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_REMOVERULE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_CLEARCOMMISSION, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					}
					else {
						// Now let's see if we're on a service item or a rule
						long nServiceID = -1;
						if (pCurSel->GetValue(clfID).vt != VT_NULL && pCurSel->GetValue(clfID).vt != VT_EMPTY) {
							nServiceID = VarLong(pCurSel->GetValue(clfID));
						}

						// (a.wetta 2007-05-17 15:49) - PLID 25394 - Get the user's permissions so we know which menu options are allowed 
						// and then only enable the appropriate menu options.  We won't prompt for a password yet.  They'll be prompted when
						// they actually select the menu option.
						BOOL bWritePermission = CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE);
				
						if (nServiceID == -1) {
							// This row is a rule and you can't add a rule or commission to a rule, so disable those choices
							pSubMenu->EnableMenuItem(ADD_RULE, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
							pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_CLEARCOMMISSION, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
							// But you can remove this rule
							pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_REMOVERULE, MF_BYCOMMAND|(bWritePermission ? MF_ENABLED : MF_DISABLED|MF_GRAYED));
						}
						else {
							// They right clicked on a service item
							pSubMenu->EnableMenuItem(ADD_RULE, MF_BYPOSITION|MF_ENABLED);
							// Determine if this service item has commission
							// (b.eyers 2015-05-11) - PLID 65976
							if ((pCurSel->GetValue(clfPercentage).vt != VT_NULL || pCurSel->GetValue(clfAmount).vt != VT_NULL) && pCurSel->GetValue(clfPercentage).vt != VT_EMPTY)
								pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_CLEARCOMMISSION, MF_BYCOMMAND|(bWritePermission ? MF_ENABLED : MF_DISABLED|MF_GRAYED));
							else
								pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_CLEARCOMMISSION, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							// This isn't a rule, so it can't be removed
							pSubMenu->EnableMenuItem(ID_COMMISSIONPOPUP_REMOVERULE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							// Add the submenu to add a rule to the currently selected service item
							CMenu *pAddRule = pSubMenu->GetSubMenu(ADD_RULE);

							if (bWritePermission) {
								// Create a recordset of all the rules that are no longer out-of-date
								CString str, strWhere;
								if (!IsDlgButtonChecked(IDC_SHOW_OUTDATED_RULES)) {
									strWhere = "AND EndDate >= dbo.AsDateNoTime(GetDate())";
								}
								else {
									strWhere = "";
								}
								_RecordsetPtr prsCommissionRules = CreateRecordset("SELECT * FROM CommissionRulesT WHERE IsTieredCommission = 0 %s ORDER BY Name", strWhere);

								while(!prsCommissionRules->eof) {
									long nRuleID = AdoFldLong(prsCommissionRules, "ID");
									CString strName = AdoFldString(prsCommissionRules, "Name");

									pAddRule->InsertMenu(0, MF_BYPOSITION|MF_ENABLED, -nRuleID, ConvertToControlText(strName));

									prsCommissionRules->MoveNext();
								}
							}
							else {
								// (a.wetta 2007-05-17 15:53) - PLID 25394 - The don't have permission to add rules
								pSubMenu->EnableMenuItem(ADD_RULE, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
							}
						}
					}

					//Ensure the position given is a valid one (if they use the keyboard, it won't be)
					if (point.x == -1) {
						CRect rc;
						pWnd->GetWindowRect(&rc);
						GetCursorPos(&point);
						if (!rc.PtInRect(point)) {
							point.x = rc.left+5;
							point.y = rc.top+5;
						}
					}

					//pop up the menu at the given position 
					long nResult = pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, point.x, point.y, this, NULL);

					// Process the result
					if (nResult == ID_COMMISSIONPOPUP_CLEARCOMMISSION) {
						ClearSelected();
					}
					else if (nResult == ID_COMMISSIONPOPUP_REMOVERULE) {
						RemoveBasicRule(m_pCurrentList->GetCurSel());
					}
					else if (nResult == ID_COMMISSIONPOPUP_ADDRULE_COMMISSIONRULESETUP) {
						LaunchCommissionRuleEditor();
					}
					else if (nResult < 0) {
						// The want to add a new rule to the selected service item, nResult is the RuleID but negative
						AddBasicRuleToRow(m_pCurrentList->GetCurSel(), -nResult);
					}
					else if(nResult != 0) {
						//fire the normal response
						PostMessage(WM_COMMAND, MAKEWPARAM(nResult, BN_CLICKED));
					}
				}
			}
		}
		else {

			//tiered commissions

			//Check to see if the focus is on our current commission list
			CWnd* pCommissionListWnd = GetDlgItem(IDC_TIERED_RULE_LIST);
			if(pWnd->GetSafeHwnd() == pCommissionListWnd->GetSafeHwnd()) {
				//The commission list has focus, so create our menu

				NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pTieredRuleList->GetCurSel();
				if(pCurSel == NULL) {
					return;
				}

				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, ID_COMMISSIONPOPUP_ADDRULE_COMMISSIONRULESETUP, "Commission Rule &Setup...");
				BOOL bWritePermission = CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE);
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|(bWritePermission ? 0 : MF_GRAYED), ID_COMMISSIONPOPUP_REMOVERULE, "&Remove Rule");

				//Ensure the position given is a valid one (if they use the keyboard, it won't be)
				if (point.x == -1) {
					CRect rc;
					pWnd->GetWindowRect(&rc);
					GetCursorPos(&point);
					if (!rc.PtInRect(point)) {
						point.x = rc.left+5;
						point.y = rc.top+5;
					}
				}

				//pop up the menu at the given position 
				long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, point.x, point.y, this, NULL);

				// Process the result
				if (nResult == ID_COMMISSIONPOPUP_REMOVERULE) {
					RemoveTieredRule(m_pTieredRuleList->GetCurSel());
				}
				else if (nResult == ID_COMMISSIONPOPUP_ADDRULE_COMMISSIONRULESETUP) {
					LaunchCommissionRuleEditor();
				}
			}
		}

	} NxCatchAll("CCommissionSetupWnd::OnContextMenu");
}

void CCommissionSetupWnd::OnSelChangingCommissionProvList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CCommissionSetupWnd::OnSelChangingCommissionProvList");	
}

void CCommissionSetupWnd::OnSelChosenCommissionProvList(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL && VarLong(pRow->GetValue(pccID)) == m_nProviderID) {
			//nothing changed
			return;
		}

		// Let's save the current data
		if (m_nProviderID != -1) {
			if(m_bChanged && AfxMessageBox("Would you like to save the changes made to the current provider before proceeding?", MB_YESNO) == IDYES) {
				if (!Save()) {
					// The save was not successful, don't switch providers
					m_pProvCombo->SetSelByColumn(pccID, m_nProviderID);
					return;
				}
			}
		}

		if(pRow) {
			m_nProviderID = VarLong(pRow->GetValue(pccID));
			Load();
			EnableControls(TRUE);
		}
		else {
			m_nProviderID = -1;
			Load();
			EnableControls(FALSE);
		}

		m_aryAddedBasicRules.RemoveAll();
		m_aryRemovedBasicRules.RemoveAll();
		m_aryAddedTieredRules.RemoveAll();
		m_aryRemovedTieredRules.RemoveAll();

		m_bChanged = false;

	}NxCatchAll("Error in CCommissionSetupWnd::OnSelChosenCommissionProvList");	
}

void CCommissionSetupWnd::EnableControls(bool bEnable)
{
	try {
		// (a.wetta 2007-05-17 16:23) - PLID 25394 - If they don't have write permission, don't enable the quick setup controls
		BOOL bWritePermission = CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE);

		GetDlgItem(IDC_CURRENT_LIST)->EnableWindow(bEnable);		
		GetDlgItem(IDC_QUICK_RULE_LIST)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_QUICK_COMM)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_COMM_ITEM_TYPE)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_RULE_ITEM_TYPE)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_APPLY_QUICK_COMM)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_APPLY_QUICK_RULE)->EnableWindow(bWritePermission && bEnable);
		// (a.wetta 2007-03-29 13:45) - PLID 25407 - Make sure that the show outdated checkbox and show all rules button also get inactivated
		GetDlgItem(IDC_SHOW_OUTDATED_RULES)->EnableWindow(bEnable);
		GetDlgItem(IDC_HIDESHOW_RULES)->EnableWindow(bEnable);
		GetDlgItem(IDC_QUICK_COMM_AMOUNT)->EnableWindow(bWritePermission && bEnable); // (b.eyers 2015-05-13) - PLID 65977

		// (j.jones 2009-11-18 11:31) - PLID 29046 - added tiered commissions
		GetDlgItem(IDC_TIERED_RULE_LIST)->EnableWindow(bEnable);
		GetDlgItem(IDC_RADIO_BASIC_COMMISSIONS)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_RADIO_TIERED_COMMISSIONS)->EnableWindow(bWritePermission && bEnable);
		GetDlgItem(IDC_CHECK_IGNORE_SHOP_FEE)->EnableWindow(bWritePermission && bEnable);

		// (a.wetta 2007-03-29 13:58) - PLID 25407 - Make sure the window refreshes correctly
		Invalidate(TRUE);

	}NxCatchAll("Error in CCommissionSetupWnd::EnableControls");
}

void CCommissionSetupWnd::OnShowOutdatedRules() 
{
	try {
		// Make sure we save before refreshing the current list
		if(m_bChanged && AfxMessageBox("Would you like to save the changes made to the current provider before refreshing the commission list?", MB_YESNO) == IDYES) {
			Save();
		}

		Load();

	}NxCatchAll("Error in CCommissionSetupWnd::OnShowOutdatedRules");
}

// (a.wetta 2007-03-27 13:44) - PLID 25360 - For the most part the following code was copied from the old commission setup dialog with 
// changes to make in work in the new dialog

//Updates the clfPercentage column in the pRow argument to the dblPercent value
//throws exception on error
void CCommissionSetupWnd::ApplyPercentage(NXDATALIST2Lib::IRowSettingsPtr pRow, double dblPercent)
{
	try {
		//update this row to have the new percentage
		pRow->PutValue(clfPercentage, (double)dblPercent);

		// (b.eyers 2015-05-12) - PLID 65976 - need to keep track that this row was changed from one type to the other
		// (b.eyers 2015-05-13) - PLID 65977 - the apply quick comm uses this too if there use to be a $ amount
		_variant_t varOldAmount = pRow->GetValue(clfOldAmount);
		if (varOldAmount.vt != VT_NULL && varOldAmount.vt != VT_EMPTY) {
			pRow->PutValue(clfTypeOfChange, (long)tocChange);
			//clear out amounts if there are any
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->PutValue(clfAmount, varNull);
		}
		else {
			pRow->PutValue(clfTypeOfChange, (long)tocUpdate);
		}

		m_bChanged = true;
	}NxCatchAll("Error in CCommissionSetupWnd::ApplyPercentage");
}

//Updates the clfPercentage column in the pRow argument to the VT_NULL value
//throws exception on error
void CCommissionSetupWnd::RemovePercentage(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		//update this row to have no percentage
		pRow->PutValue(clfPercentage, varNull);

		//also need to flag this row as changed
		pRow->PutValue(clfTypeOfChange, (long)tocRemove);

		m_bChanged = true;
	}NxCatchAll("Error in CCommissionSetupWnd::RemovePercentage");
}

// (b.eyers 2015-05-11) - PLID 65976
void CCommissionSetupWnd::RemoveAmount(NXDATALIST2Lib::IRowSettingsPtr pRow) {
	try {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow->PutValue(clfAmount, varNull);

		pRow->PutValue(clfTypeOfChange, (long)tocRemove);

		m_bChanged = true;
	}NxCatchAll("Error in CCommissionSetupWnd::RemoveAmount");
}

// (b.eyers 2015-05-11) - PLID 65976
void CCommissionSetupWnd::ApplyAmount(NXDATALIST2Lib::IRowSettingsPtr pRow, COleCurrency cyAmount) {
	try {
		pRow->PutValue(clfAmount, _variant_t(cyAmount));

		// (b.eyers 2015-05-12) - PLID 65976 - When in a separate window, need to keep track that this row was changed from one type to the other
		// (b.eyers 2015-05-13) - PLID 65977 - the apply quick comm uses this too if there use to be a % amount
		_variant_t varOldPercent = pRow->GetValue(clfOldPercent);
		if (varOldPercent.vt != VT_NULL && varOldPercent.vt != VT_EMPTY) {
			pRow->PutValue(clfTypeOfChange, (long)tocChange);
			//clear out the percentages if there are any
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->PutValue(clfPercentage, varNull);
		}
		else {
			pRow->PutValue(clfTypeOfChange, (long)tocUpdate);
		}

		m_bChanged = true;
	}NxCatchAll("Error in CCommissionSetupWnd::ApplyAmount");
}

//Saves all the rows which have been flagged as Changed
//returns true if the save is a success.
//returns false if any errors or cancelation occurs
bool CCommissionSetupWnd::Save()
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:58) - PLID 25394 - Make sure they have permission to write, the permissions should have been
		// checked by now, but do it once more just in case
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite, FALSE, 0, TRUE, TRUE))
			return false;

		//make sure the current provider is set
		if(m_nProviderID == -1) {
			MsgBox("There is no current provider set.  Save cannot continue.");
			return false;
		}

		BeginWaitCursor();

		// (j.jones 2006-05-05 16:58) - PLID 20465 - changed saving behavior to
		// update all services with the same percentage in one statement, instead
		// of a statement per service code (however this is broken down to 100 IDs
		// per IN statement)
		CArray<ServiceUpdate*,ServiceUpdate*> aryUpdateList;
		//(e.lally 2006-10-30) We are going to track records to delete from the data as well.
		CArray<ServiceUpdate*,ServiceUpdate*> aryRemoveList;

		//we have to look at all the list items to save
		IRowSettingsPtr pRow = m_pCurrentList->GetFirstRow();
		while (pRow) {
			long nChangeValue = VarLong(pRow->GetValue(clfTypeOfChange), tocNone);
			if(nChangeValue == tocUpdate) {
				//this row has changed, we need to save it
				CreateSaveRecord(pRow, aryUpdateList, nAuditTransactionID);
			}
			else if(nChangeValue == tocRemove) {
				//this row is going to be removed from the commissions
				CreateDeleteRecord(pRow, aryRemoveList, nAuditTransactionID);
			}
			// (b.eyers 2015-05-12) - PLID 65976 - When in a separate window, need to keep track that this row was changed from one type to the other
			else if (nChangeValue == tocChange) {
				CreateDeleteRecord(pRow, aryRemoveList, nAuditTransactionID);
				CreateSaveRecord(pRow, aryUpdateList, nAuditTransactionID);
			}

			if (m_bTabMode) {
				// Clean up the TypeOfChange and OldPercentage columns
				pRow->PutValue(clfTypeOfChange, (long)tocNone);
				pRow->PutValue(clfOldPercent, pRow->GetValue(clfPercentage));
				pRow->PutValue(clfOldAmount, pRow->GetValue(clfAmount)); // (b.eyers 2015-05-11) - PLID 65976
			}
			
			pRow = pRow->GetNextRow();
		}

		CString strSql = BeginSqlBatch();

		// (a.walling 2007-11-05 13:07) - PLID 27977 - VS2008 - for() loops
		int i = 0;

		// (b.eyers 2015-05-12) - PLID 65976 - This needs to run before inserts happen
		for (i = 0; i<aryRemoveList.GetSize(); i++) {
			//we should have one aryRemoveList entry for each sql statement needed
			CString strSqlStatement;
			ServiceUpdate *su = aryRemoveList.GetAt(i);

			if (su->aryServiceIDs.GetSize() > 0) {

				strSqlStatement.Format("DELETE FROM CommissionT "
					"WHERE ProvID = %li AND ServiceID IN (", m_nProviderID);

				//now add the service IDs
				for (int j = 0; j<su->aryServiceIDs.GetSize(); j++) {
					strSqlStatement += AsString(su->aryServiceIDs.GetAt(j));
					strSqlStatement += ",";
				}
				strSqlStatement.TrimRight(",");
				strSqlStatement += ")";

				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
		}

		for(i=0;i<aryUpdateList.GetSize();i++) {
			//we should have one aryUpdateList entry for each sql statement needed
			CString strSqlStatement;
			ServiceUpdate *su = aryUpdateList.GetAt(i);

			if(su->aryServiceIDs.GetSize() > 0) {

				// (b.eyers 2015-05-11) - PLID 65976 - Only insert/update for the one that changed
				if (su->bNew && su->dblPercentage != NULL) {
					strSqlStatement.Format("INSERT INTO CommissionT (ProvID, ServiceID, Percentage) SELECT "
						"%li, ID, Round(%f, 2) FROM ServiceT WHERE ID IN (", m_nProviderID, su->dblPercentage);
				}
				else if (su->dblPercentage != NULL) {
					strSqlStatement.Format("UPDATE CommissionT SET Percentage = Round(%f, 2) "
						"WHERE ProvID = %li AND ServiceID IN (", su->dblPercentage, m_nProviderID);
				}
				else if (su->bNew && su->cyAmount != COleCurrency(0, 0)) {
					strSqlStatement.Format("INSERT INTO CommissionT (ProvID, ServiceID, Amount) SELECT "
						"%li, ID, CONVERT(money, '%s') FROM ServiceT WHERE ID IN (", m_nProviderID, FormatCurrencyForSql(su->cyAmount));
				}
				else if (su->cyAmount != COleCurrency(0, 0)) {
					strSqlStatement.Format("UPDATE CommissionT SET Amount = CONVERT(money, '%s') "
						"WHERE ProvID = %li AND ServiceID IN (", FormatCurrencyForSql(su->cyAmount), m_nProviderID);
				}

				//now add the service IDs
				for(int j=0;j<su->aryServiceIDs.GetSize();j++) {
					strSqlStatement += AsString(su->aryServiceIDs.GetAt(j));
					strSqlStatement += ",";
				}
				strSqlStatement.TrimRight(",");
				strSqlStatement += ")";

				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
		}

		//for(i=0;i<aryRemoveList.GetSize();i++) {
		//	//we should have one aryRemoveList entry for each sql statement needed
		//	CString strSqlStatement;
		//	ServiceUpdate *su = aryRemoveList.GetAt(i);

		//	if(su->aryServiceIDs.GetSize() > 0) {

		//		strSqlStatement.Format("DELETE FROM CommissionT "
		//			"WHERE ProvID = %li AND ServiceID IN (", m_nProviderID);

		//		//now add the service IDs
		//		for(int j=0;j<su->aryServiceIDs.GetSize();j++) {
		//			strSqlStatement += AsString(su->aryServiceIDs.GetAt(j));
		//			strSqlStatement += ",";
		//		}
		//		strSqlStatement.TrimRight(",");
		//		strSqlStatement += ")";

		//		AddStatementToSqlBatch(strSql, strSqlStatement);
		//	}
		//}

		// (a.wetta 2007-03-28 10:27) - PLID 25360 - Update the data accordingly for added or removed rules
		CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));
		CString strSqlRule;
		for (i = 0; i < m_aryAddedBasicRules.GetSize(); i++) {
			strSqlRule.Format("INSERT INTO CommissionRulesLinkT (RuleID, ProvID, ServiceID) VALUES (%li, %li, %li)",
							m_aryAddedBasicRules.GetAt(i).nRuleID, m_aryAddedBasicRules.GetAt(i).nProvID, m_aryAddedBasicRules.GetAt(i).nServiceID);
			AddStatementToSqlBatch(strSql, strSqlRule);

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strAuditStr;
			strAuditStr.Format("Item: \'%s\'; Rule: \'%s\'", m_aryAddedBasicRules.GetAt(i).strServiceName, m_aryAddedBasicRules.GetAt(i).strRuleName);
			// (j.jones 2009-11-19 11:01) - PLID 29046 - audit using the "basic" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionRuleAdded, m_aryAddedBasicRules.GetAt(i).nServiceID, "", strAuditStr, aepMedium, aetChanged);
		}

		for (i = 0; i < m_aryRemovedBasicRules.GetSize(); i++) {
			strSqlRule.Format("DELETE FROM CommissionRulesLinkT WHERE RuleID = %li AND ProvID = %li AND ServiceID = %li",
				m_aryRemovedBasicRules.GetAt(i).nRuleID, m_aryRemovedBasicRules.GetAt(i).nProvID, m_aryRemovedBasicRules.GetAt(i).nServiceID);
			AddStatementToSqlBatch(strSql, strSqlRule);

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strAuditStr;
			strAuditStr.Format("Item: \'%s\'; Rule: \'%s\'", m_aryRemovedBasicRules.GetAt(i).strServiceName, m_aryRemovedBasicRules.GetAt(i).strRuleName);
			// (j.jones 2009-11-19 11:01) - PLID 29046 - audit using the "basic" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionRuleRemoved, m_aryRemovedBasicRules.GetAt(i).nServiceID, strAuditStr, "", aepMedium, aetChanged);
		}

		// (j.jones 2009-11-18 11:31) - PLID 29046 - added tiered commissions
		BOOL bUseTieredCommissions = m_radioTieredCommissions.GetCheck();
		BOOL bIgnoreShopFee = m_checkIgnoreShopFee.GetCheck();
		AddStatementToSqlBatch(strSql, "UPDATE ProvidersT SET UseTieredCommissions = %li, IgnoreShopFee = %li WHERE PersonID = %li",
			bUseTieredCommissions ? 1 : 0, bIgnoreShopFee ? 1 : 0, m_nProviderID);

		IRowSettingsPtr pProviderRow = m_pProvCombo->GetCurSel();
		if(pProviderRow) {
			if(bUseTieredCommissions != VarBool(pProviderRow->GetValue(pccUseTieredCommissions), FALSE)) {

				// Audit the change
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvCommissionType, m_nProviderID, bUseTieredCommissions ? "Use Basic Commissions" : "Use Tiered Commissions", bUseTieredCommissions ? "Use Tiered Commissions" : "Use Basic Commissions", aepMedium, aetChanged);
			}

			if(bIgnoreShopFee != VarBool(pProviderRow->GetValue(pccIgnoreShopFee), FALSE)) {

				// Audit the change
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvTieredCommissionIgnoreShopFee, m_nProviderID, bIgnoreShopFee ? "Subtract Shop Fee" : "Do Not Subtract Shop Fee", bIgnoreShopFee ? "Do Not Subtract Shop Fee" : "Subtract Shop Fee", aepMedium, aetChanged);
			}
		}

		for (i = 0; i < m_aryAddedTieredRules.GetSize(); i++) {
			strSqlRule.Format("INSERT INTO TieredCommissionRulesLinkT (RuleID, ProviderID, RuleType) VALUES (%li, %li, %li)",
				m_aryAddedTieredRules.GetAt(i).nRuleID, m_aryAddedTieredRules.GetAt(i).nProvID, m_aryAddedTieredRules.GetAt(i).nRuleType);
			AddStatementToSqlBatch(strSql, strSqlRule);

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strAuditStr, strRuleType;

			long nTypeID = m_aryAddedTieredRules.GetAt(i).nRuleType;
			if (nTypeID == 0) {
				strRuleType = "Service Codes";
			}
			else if (nTypeID == 1) {
				strRuleType = "Inventory Items";
			}
			else if (nTypeID == 2) {
				strRuleType = "Gift Certificates";
			}
			else {
				strRuleType = "All Sales";
			}

			strAuditStr.Format("Rule: \'%s\', Type: \'%s\'", m_aryAddedTieredRules.GetAt(i).strRuleName, strRuleType);
			// (j.jones 2009-11-19 11:01) - PLID 29046 - audit using the "tiered" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvTieredCommissionRuleAdded, m_aryAddedTieredRules.GetAt(i).nProvID, "", strAuditStr, aepMedium, aetChanged);
		}

		for (i = 0; i < m_aryRemovedTieredRules.GetSize(); i++) {
			strSqlRule.Format("DELETE FROM TieredCommissionRulesLinkT WHERE RuleID = %li AND ProviderID = %li AND RuleType = %li",
				m_aryRemovedTieredRules.GetAt(i).nRuleID, m_aryRemovedTieredRules.GetAt(i).nProvID, m_aryRemovedTieredRules.GetAt(i).nRuleType);
			AddStatementToSqlBatch(strSql, strSqlRule);

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			CString strAuditStr, strRuleType;
			long nTypeID = m_aryRemovedTieredRules.GetAt(i).nRuleType;
			if (nTypeID == 0) {
				strRuleType = "Service Codes";
			}
			else if (nTypeID == 1) {
				strRuleType = "Inventory Items";
			}
			else if (nTypeID == 2) {
				strRuleType = "Gift Certificates";
			}
			else {
				strRuleType = "All Sales";
			}

			strAuditStr.Format("Rule: \'%s\', Type: \'%s\'", m_aryRemovedTieredRules.GetAt(i).strRuleName, strRuleType);
			// (j.jones 2009-11-19 11:01) - PLID 29046 - audit using the "tiered" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvTieredCommissionRuleRemoved, m_aryRemovedTieredRules.GetAt(i).nProvID, strAuditStr, "", aepMedium, aetChanged);
		}

		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

		//commit auditing
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}
		
		//set our provider combo columns
		if(pProviderRow) {
			pProviderRow->PutValue(pccUseTieredCommissions, bUseTieredCommissions ? g_cvarTrue : g_cvarFalse);
			pProviderRow->PutValue(pccIgnoreShopFee, bIgnoreShopFee ? g_cvarTrue : g_cvarFalse);
		}

		m_bChanged = false;

		//clean up our data
		// (a.walling 2007-11-05 13:07) - PLID 27977 - VS2008 - for() loops

		for(i=aryUpdateList.GetSize()-1;i>=0;i--) {
			delete aryUpdateList.GetAt(i);
		}
		for(i=aryRemoveList.GetSize()-1; i>=0; i--) {
			delete aryRemoveList.GetAt(i);
		}

		m_aryAddedBasicRules.RemoveAll();
		m_aryRemovedBasicRules.RemoveAll();
		m_aryAddedTieredRules.RemoveAll();
		m_aryRemovedTieredRules.RemoveAll();

		//(e.lally 2007-10-12) PLID 25360 - we have to get rid of the change flag for all the rows that 
			//were marked update or remove now that they have been saved.
		pRow = m_pCurrentList->GetFirstRow();
		while (pRow) {
			long nChangeValue = VarLong(pRow->GetValue(clfTypeOfChange), tocNone);
			if(nChangeValue != tocNone) {
				// Clean up the TypeOfChange column and update the oldPercent column
				pRow->PutValue(clfTypeOfChange, (long)tocNone);
				pRow->PutValue(clfOldPercent, pRow->GetValue(clfPercentage));
				pRow->PutValue(clfOldAmount, pRow->GetValue(clfAmount)); // (b.eyers 2015-05-11) - PLID 65976
			}
			
			pRow = pRow->GetNextRow();
		}

		EndWaitCursor();

		//successfully saved everything
		return true;

	} NxCatchAllCall("Error saving data.", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););

	return false;
}

//takes the given row and generates an update statement which is appended to the
//strOut parameter to be inserted into the data
//throws an exception on error
void CCommissionSetupWnd::CreateSaveRecord(IN NXDATALIST2Lib::IRowSettingsPtr pRow, IN OUT CArray<ServiceUpdate*,ServiceUpdate*> &aryUpdateList, long &nAuditTransactionID)
{
	try {
		CString str;

		//we keep track of the old commission (what was loaded from data) and the current commission (may or may not have been modified)
		//If the old one is null, then we know this does not exist in data and need to run an INSERT statement.  Otherwise we need
		//to run an UPDATE query
		// (b.eyers 2015-05-11) - PLID 65976 
		_variant_t varOldPercent = pRow->GetValue(clfOldPercent);
		_variant_t varOldAmount = pRow->GetValue(clfOldAmount);
		_variant_t varPercent = pRow->GetValue(clfPercentage);
		_variant_t varAmount = pRow->GetValue(clfAmount);
		// (b.eyers 2015-05-11) - PLID 65976 - If old percentage is empty and there is data in percentage, new percentage record
		if ((varOldPercent.vt == VT_NULL || varOldPercent.vt == VT_EMPTY) && varPercent.vt != VT_EMPTY && varPercent.vt != VT_NULL) {
			//nothing, we need to INSERT

			long nServiceID = VarLong(pRow->GetValue(clfID));
			double dblPercentage = VarDouble(pRow->GetValue(clfPercentage));
			CString strServiceName = VarString(pRow->GetValue(clfName)), strAuditNewStr, strAuditOldStr;
			CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			strAuditOldStr.Format("Item: \'%s\'; Commission: {none}", strServiceName);
			strAuditNewStr.Format("%.2f%%", dblPercentage);
			// (j.jones 2009-11-19 11:01) - PLID 29046 - this is the "basic" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionPercentage, nServiceID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

			for(int i=0;i<aryUpdateList.GetSize(); i++) {
				ServiceUpdate *su = aryUpdateList.GetAt(i);
				if(su->bNew && su->dblPercentage == dblPercentage) {
					//add to this list if it is not full

					//limit to 100 IDs per statement
					if(su->aryServiceIDs.GetSize() < 100) {

						//if we got here, we have the right update record, and the list isn't full
						su->aryServiceIDs.Add(nServiceID);
						return;
					}
				}
			}

			//if we got here, we need a new update record for this percentage

			ServiceUpdate *pNewUpdate = new ServiceUpdate;
			pNewUpdate->aryServiceIDs.Add(nServiceID);
			pNewUpdate->dblPercentage = dblPercentage;
			pNewUpdate->bNew = TRUE;
			pNewUpdate->cyAmount = COleCurrency(0,0); // (b.eyers 2015-05-11) - PLID 65976

			aryUpdateList.Add(pNewUpdate);

			return;
		}
		// (b.eyers 2015-05-11) - PLID 65976 - if there is data in the percentage field, update existing record
		else if (varPercent.vt != VT_EMPTY && varPercent.vt != VT_NULL) { 
			//it existed previously, and it changed, so UPDATE
			
			long nServiceID = VarLong(pRow->GetValue(clfID));
			double dblPercentage = VarDouble(pRow->GetValue(clfPercentage));
			double dblOldPercentage = VarDouble(pRow->GetValue(clfOldPercent));
			CString strServiceName = VarString(pRow->GetValue(clfName)), strAuditOldStr, strAuditNewStr;
			CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			strAuditOldStr.Format("Item: \'%s\'; Commission: %.2f%%", strServiceName, dblOldPercentage);
			strAuditNewStr.Format("%.2f%%", dblPercentage);
			// (j.jones 2009-11-19 11:01) - PLID 29046 - this is the "basic" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionPercentage, nServiceID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

			for(int i=0;i<aryUpdateList.GetSize(); i++) {
				ServiceUpdate *su = aryUpdateList.GetAt(i);
				if(!su->bNew && su->dblPercentage == dblPercentage) {
					//add to this list if it is not full

					//limit to 100 IDs per statement
					if(su->aryServiceIDs.GetSize() < 100) {

						//if we got here, we have the right update record, and the list isn't full
						su->aryServiceIDs.Add(nServiceID);
						return;
					}
				}
			}

			//if we got here, we need a new update record for this percentage

			ServiceUpdate *pNewUpdate = new ServiceUpdate;
			pNewUpdate->aryServiceIDs.Add(nServiceID);
			pNewUpdate->dblPercentage = dblPercentage;
			pNewUpdate->bNew = FALSE;
			pNewUpdate->cyAmount = COleCurrency(0,0); // (b.eyers 2015-05-11) - PLID 65976

			aryUpdateList.Add(pNewUpdate);

			return;
		}
		// (b.eyers 2015-05-11) - PLID 65976 - if there is no data in old amount and new data in amount, new record
		else if ((varOldAmount.vt == VT_NULL || varOldAmount.vt == VT_EMPTY) && varAmount.vt != VT_EMPTY && varAmount.vt != VT_NULL) {
			//new

			long nServiceID = VarLong(pRow->GetValue(clfID));
			CString strServiceName = VarString(pRow->GetValue(clfName)), strAuditNewStr, strAuditOldStr;
			CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));
			COleCurrency cyAmount = VarCurrency(pRow->GetValue(clfAmount)); 

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			strAuditOldStr.Format("Item: \'%s\'; Commission: {none}", strServiceName);
			strAuditNewStr.Format("%s", FormatCurrencyForInterface(cyAmount));
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionAmount, nServiceID, strAuditOldStr, strAuditNewStr, aepMedium, aetCreated);

			for (int i = 0; i<aryUpdateList.GetSize(); i++) {
				ServiceUpdate *su = aryUpdateList.GetAt(i);
				if (su->bNew && su->cyAmount == cyAmount) {

					if (su->aryServiceIDs.GetSize() < 100) {

						su->aryServiceIDs.Add(nServiceID);
						return;
					}
				}
			}

			ServiceUpdate *pNewUpdate = new ServiceUpdate;
			pNewUpdate->aryServiceIDs.Add(nServiceID);
			pNewUpdate->dblPercentage = NULL;
			pNewUpdate->bNew = TRUE;
			pNewUpdate->cyAmount = cyAmount; 

			aryUpdateList.Add(pNewUpdate);

			return;
		}
		// (b.eyers 2015-05-11) - PLID 65976 - data in amount, update existing record
		else if (varAmount.vt != VT_EMPTY && varAmount.vt != VT_NULL) {
			//update

			long nServiceID = VarLong(pRow->GetValue(clfID));
			CString strServiceName = VarString(pRow->GetValue(clfName)), strAuditOldStr, strAuditNewStr;
			CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));
			COleCurrency cyAmount = VarCurrency(pRow->GetValue(clfAmount)); 
			COleCurrency cyOldAmount = VarCurrency(pRow->GetValue(clfOldAmount));

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			strAuditOldStr.Format("Item: \'%s\'; Commission: %s", strServiceName, FormatCurrencyForInterface(cyOldAmount));
			strAuditNewStr.Format("%s", FormatCurrencyForInterface(cyAmount));
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionAmount, nServiceID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

			for (int i = 0; i<aryUpdateList.GetSize(); i++) {
				ServiceUpdate *su = aryUpdateList.GetAt(i);
				if (!su->bNew && su->cyAmount == cyAmount) {

					if (su->aryServiceIDs.GetSize() < 100) {

						su->aryServiceIDs.Add(nServiceID);
						return;
					}
				}
			}

			ServiceUpdate *pNewUpdate = new ServiceUpdate;
			pNewUpdate->aryServiceIDs.Add(nServiceID);
			pNewUpdate->dblPercentage = NULL;
			pNewUpdate->bNew = FALSE;
			pNewUpdate->cyAmount = cyAmount;

			aryUpdateList.Add(pNewUpdate);

			return;
		}

	}NxCatchAll("Error in CCommissionSetupDlg::CreateSaveRecord");
}

//takes the given row and generates an delete statement which is appended to the
//strOut parameter to be run on the data
//throws an exception on error
void CCommissionSetupWnd::CreateDeleteRecord(IN NXDATALIST2Lib::IRowSettingsPtr pRow, IN OUT CArray<ServiceUpdate*,ServiceUpdate*> &aryRemoveList, long &nAuditTransactionID)
{
	try {
		CString str;

		// (b.eyers 2015-05-11) - PLID 65976
		_variant_t varPercent = pRow->GetValue(clfOldPercent);
		_variant_t varAmount = pRow->GetValue(clfOldAmount);
		//if (varPercent.vt == VT_NULL || varPercent.vt == VT_EMPTY) {
			//there is no previous record, do nothing.

		//	return;
		//}
		// (b.eyers 2015-05-11) - PLID 65976 - data in old percentage, there is a record to delete
		if (varPercent.vt != VT_EMPTY && varPercent.vt != VT_NULL) {
			//it existed previously, and we need to remove it.
			
			long nServiceID = VarLong(pRow->GetValue(clfID));
			double dblOldPercentage = VarDouble(pRow->GetValue(clfOldPercent));
			CString strServiceName = VarString(pRow->GetValue(clfName)), strAuditOldStr, strAuditNewStr;
			CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));

			// Audit the change
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			strAuditOldStr.Format("Item: \'%s\'; Commission: %.2f%%", strServiceName, dblOldPercentage);
			strAuditNewStr.Format("{none}");
			// (j.jones 2009-11-19 11:01) - PLID 29046 - this is the "basic" audit
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionPercentage, nServiceID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

			for(int i=0;i<aryRemoveList.GetSize(); i++) {
				ServiceUpdate *su = aryRemoveList.GetAt(i);
				//add to this list if it is not full

				//limit to 100 IDs per statement
				if(su->aryServiceIDs.GetSize() < 100) {

					//if we got here, we have the right record, and the list isn't full
					su->aryServiceIDs.Add(nServiceID);
					return;
				}
			}

			//if we got here, we need a new delete record

			ServiceUpdate *pNewRemoval = new ServiceUpdate;
			pNewRemoval->aryServiceIDs.Add(nServiceID);

			aryRemoveList.Add(pNewRemoval);

			return;
		}
		// (b.eyers 2015-05-11) - PLID 65976 - data in old amount, delete the record
		else if (varAmount.vt != VT_EMPTY && varAmount.vt != VT_NULL) {
			long nServiceID = VarLong(pRow->GetValue(clfID));
			COleCurrency cyOldAmount = VarCurrency(pRow->GetValue(clfOldAmount));
			CString strServiceName = VarString(pRow->GetValue(clfName)), strAuditOldStr, strAuditNewStr;
			CString strProvName = VarString(m_pProvCombo->GetCurSel()->GetValue(pccName));

			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			strAuditOldStr.Format("Item: \'%s\'; Commission: %s", strServiceName, FormatCurrencyForInterface(cyOldAmount));
			strAuditNewStr.Format("{none}");
			AuditEvent(-1, strProvName, nAuditTransactionID, aeiProvBasicCommissionAmount, nServiceID, strAuditOldStr, strAuditNewStr, aepMedium, aetDeleted);

			for (int i = 0; i<aryRemoveList.GetSize(); i++) {
				ServiceUpdate *su = aryRemoveList.GetAt(i);

				if (su->aryServiceIDs.GetSize() < 100) {

					su->aryServiceIDs.Add(nServiceID);
					return;
				}
			}

			ServiceUpdate *pNewRemoval = new ServiceUpdate;
			pNewRemoval->aryServiceIDs.Add(nServiceID);

			aryRemoveList.Add(pNewRemoval);

			return;
		}
		else {
			return;
		}

	}NxCatchAll("Error in CCommissionSetupDlg::CreateDeleteRecord");
}

void CCommissionSetupWnd::ClearSelected() 
{
	try{
		// If there is nothing selected, do nothing
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCurrentList->GetCurSel();
		if (pRow == NULL)
			return;

		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Confirm
		CString str;
		str.Format("Are you sure you wish to remove the commission from the selected item?");

		if(AfxMessageBox(str, MB_YESNO) != IDYES)
			return;

		CWaitCursor pWait;

		// Remove the commission from this row
		RemovePercentage(pRow);
		RemoveAmount(pRow); // (b.eyers 2015-05-11) - PLID 65976

		//update our flag for things changing
		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}

	}NxCatchAll("Error removing selected commission");
	
}

// (a.wetta 2007-03-28 10:28) - PLID 25360 - End of functions copied from old commission dialog
/////////////////////////////////////////////////////

// (j.jones 2009-11-18 16:13) - PLID 29046 - renamed this function to reflect that this is
// for "basic" rules stored in CommissionRulesLinkT 
// (b.eyers 2015-05-15) - PLID 65978  - Need to add dollar amount to rows now from rules
void CCommissionSetupWnd::AddBasicRuleToRow(NXDATALIST2Lib::IRowSettingsPtr pRow, long nRuleID, CString strRuleName /*= ""*/, double dblRulePercentage /*= 0*/, COleCurrency cyRuleAmount /*= COleCurrency(0, 0)*/) 
{
	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Check to make sure that this row doesn't already have this rule
		NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
		while (pChildRow) {
			if (VarLong(pChildRow->GetValue(clfRuleID)) == nRuleID) {
				// This row already has this rule
				return;
			}
			pChildRow = pChildRow->GetNextRow();
		}

		// Get the service ID of the row
		long nServiceID = VarLong(pRow->GetValue(clfID), -1);

		// Make sure that this row is a service item
		if (nServiceID == -1) 
			return;

		// Get the information for the rule if it was not passed into the function.  The rule information may be passed into this function
		// if the same rule is added to many service items at once.  It would prevent the following sql statement from being called multiple
		// times when it would be the same query every time.
		if (strRuleName == "") {
			_RecordsetPtr prsRuleInfo = CreateRecordset("SELECT * FROM CommissionRulesT WHERE ID = %li", nRuleID);
			if (prsRuleInfo->eof) {
				// We weren't able to find the rule information for some reason.
				return;
			}
			else {
				// Get the rule information
				strRuleName = AdoFldString(prsRuleInfo, "Name");
				dblRulePercentage = AdoFldDouble(prsRuleInfo, "Percentage", NULL); // (b.eyers 2015-05-15) - PLID 65978
				cyRuleAmount = AdoFldCurrency(prsRuleInfo, "Amount", COleCurrency(0, 0)); // (b.eyers 2015-05-15) - PLID 65978
			}
		}

		BasicRuleInfo riRule;
		riRule.nProvID = m_nProviderID;
		riRule.nRuleID = nRuleID;
		riRule.nServiceID = nServiceID;
		riRule.strRuleName = strRuleName;
		riRule.strServiceName = VarString(pRow->GetValue(clfName));
		// Check to make sure it isn't in the remove array, if so take it out and don't add it to the
		// added rules array
		bool bFoundRule = false;
		for (int i = 0; i < m_aryRemovedBasicRules.GetSize() && !bFoundRule; i++) {
			if (riRule.nProvID == m_aryRemovedBasicRules.GetAt(i).nProvID &&
				riRule.nRuleID == m_aryRemovedBasicRules.GetAt(i).nRuleID &&
				riRule.nServiceID == m_aryRemovedBasicRules.GetAt(i).nServiceID) {
				bFoundRule = true;
				m_aryRemovedBasicRules.RemoveAt(i);
			}
		}
		if (!bFoundRule) {
			// Add the rule to the added rules array
			m_aryAddedBasicRules.Add(riRule);
		}		

		_variant_t varNull;
		varNull.vt = VT_NULL;

		// Now add the rule row to the current list
		NXDATALIST2Lib::IRowSettingsPtr pRuleRow = m_pCurrentList->GetNewRow();
		pRuleRow->PutValue(clfParentID, _variant_t(nServiceID));
		pRuleRow->PutValue(clfRuleID, _variant_t(nRuleID));
		pRuleRow->PutValue(clfType, _variant_t("{Rule}"));
		pRuleRow->PutValue(clfName, _variant_t(strRuleName));
		//pRuleRow->PutValue(clfPercentage, _variant_t(dblRulePercentage));

		// (b.eyers 2015-05-15) - PLID 65978 - display empty column for the one that doesn't have data
		if (cyRuleAmount != COleCurrency(0,0)) {
			pRuleRow->PutValue(clfPercentage, varNull);
			pRuleRow->PutValue(clfAmount, _variant_t(cyRuleAmount));
		}
		else {
			pRuleRow->PutValue(clfPercentage, _variant_t(dblRulePercentage));
			pRuleRow->PutValue(clfAmount, varNull);
		}

		// (a.wetta 2007-05-21 09:17) - PLID 25360 - Be sure to set the ID to NULL
		pRuleRow->PutValue(clfID, varNull);
		m_pCurrentList->AddRowSorted(pRuleRow, pRow);

		// Make sure that the parent row is expanded
		pRow->PutExpanded(VARIANT_TRUE);

		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}
		
	}NxCatchAll("Error in CCommissionSetupWnd::AddBasicRuleToRow");
}

// (j.jones 2009-11-18 16:13) - PLID 29046 - renamed this function to reflect that this is
// for "basic" rules stored in CommissionRulesLinkT
void CCommissionSetupWnd::RemoveBasicRule(NXDATALIST2Lib::IRowSettingsPtr pRuleRow)
{
	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the information from the rule row
		long nServiceID = VarLong(pRuleRow->GetValue(clfParentID));
		long nRuleID = VarLong(pRuleRow->GetValue(clfRuleID));

		// Add the rule to the array of removed rules
		BasicRuleInfo riRule;
		riRule.nProvID = m_nProviderID;
		riRule.nRuleID = nRuleID;
		riRule.nServiceID = nServiceID;
		riRule.strRuleName = VarString(pRuleRow->GetValue(clfName));
		riRule.strServiceName = VarString(pRuleRow->GetParentRow()->GetValue(clfName));
		// First check to make sure it isn't in the added array, if so take it out and don't add it to the
		// removed rules array
		bool bFoundRule = false;
		for (int i = 0; i < m_aryAddedBasicRules.GetSize() && !bFoundRule; i++) {
			if (riRule.nProvID == m_aryAddedBasicRules.GetAt(i).nProvID &&
				riRule.nRuleID == m_aryAddedBasicRules.GetAt(i).nRuleID &&
				riRule.nServiceID == m_aryAddedBasicRules.GetAt(i).nServiceID) {
				bFoundRule = true;
				m_aryAddedBasicRules.RemoveAt(i);
			}
		}
		if (!bFoundRule) {
			// Add the rule to the added rules array
			m_aryRemovedBasicRules.Add(riRule);
		}

		// Remove the row from the current list
		m_pCurrentList->RemoveRow(pRuleRow);

		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}

	}NxCatchAll("Error in CCommissionSetupWnd::RemoveBasicRule");
}

// (j.jones 2009-11-18 16:13) - PLID 29046 - added to track "tiered" rules
// stored in TieredCommissionRulesLinkT
void CCommissionSetupWnd::RemoveTieredRule(NXDATALIST2Lib::IRowSettingsPtr pRuleRow)
{
	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the information from the rule row		
		long nRuleID = VarLong(pRuleRow->GetValue(trlcRuleID));
		long nRuleType = VarLong(pRuleRow->GetValue(trlcTypeID));
		CString strRuleName = VarString(pRuleRow->GetValue(trlcName));

		// Add the rule to the array of removed rules
		TieredRuleInfo riRule;
		riRule.nProvID = m_nProviderID;
		riRule.nRuleID = nRuleID;
		riRule.nRuleType = nRuleType;
		riRule.strRuleName = strRuleName;
		// First check to make sure it isn't in the added array, if so take it out and don't add it to the
		// removed rules array
		bool bFoundRule = false;
		for (int i = 0; i < m_aryAddedTieredRules.GetSize() && !bFoundRule; i++) {
			if (riRule.nProvID == m_aryAddedTieredRules.GetAt(i).nProvID &&
				riRule.nRuleID == m_aryAddedTieredRules.GetAt(i).nRuleID &&
				riRule.nRuleType == m_aryAddedTieredRules.GetAt(i).nRuleType) {
				bFoundRule = true;
				m_aryAddedTieredRules.RemoveAt(i);
			}
		}
		if (!bFoundRule) {
			// Add the rule to the removed rules array
			m_aryRemovedTieredRules.Add(riRule);
		}

		// Remove the row from the list
		m_pTieredRuleList->RemoveRow(pRuleRow);

		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}

	}NxCatchAll("Error in CCommissionSetupWnd::RemoveTieredRule");
}

void CCommissionSetupWnd::ExpandAllRows(VARIANT_BOOL bExpand)
{	
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCurrentList->GetFirstRow();
		while (pRow) {
			pRow->PutExpanded(bExpand);
			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CCommissionSetupWnd::ExpandAllRows");
}

void CCommissionSetupWnd::OnApplyQuickComm() 
{
	try{
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		if (m_bPercent) { // (b.eyers 2015-05-13) - PLID 65977 - if the percentage text box is active

			CString strPercent;
			GetDlgItemText(IDC_QUICK_COMM, strPercent);

			if (strPercent.IsEmpty()) {
				MsgBox("You must enter a percentage before applying.");
				return;
			}

			//convert this string into a double value
			double dblPercent = atof(strPercent);

			//Do not allow them to assign negative percentages
			//(e.lally 2007-10-12) PLID 25360 - Prevent 0% from being applied too.
			if (dblPercent <= 0.0) {
				MsgBox("You may only assign a commission percent that is greater than 0.00%%.");
				return;
			}
			

			//round it to 2 digits
			RoundDouble(dblPercent);

			// Get the type of service item
			long nTypeID = -1;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCommTypeCombo->GetCurSel();
			if (pRow != NULL && pRow->GetValue(0).vt != VT_NULL) {
				nTypeID = VarLong(pRow->GetValue(0));
			}
			else {
				MsgBox("You must select a type of service item before applying.");
				return;
			}

			CString strType;
			if (nTypeID == 0)
				strType = "Service Code";
			else if (nTypeID == 1)
				strType = "Inventory Item";
			else if (nTypeID == 2)
				strType = "Gift Certificate";

			//confirm
			CString str;
			str.Format("Are you sure you wish to update all %ss to %.2f%%? "
				"Note, items with a dollar amount will be cleared and replaced with the percentage if you continue.", strType, dblPercent);
			if (AfxMessageBox(str, MB_YESNO) != IDYES)
				return;
			

			CWaitCursor pWait;

			NXDATALIST2Lib::IRowSettingsPtr pUpdateRow = m_pCurrentList->GetFirstRow();

			while (pUpdateRow) {
				if (VarString(pUpdateRow->GetValue(clfType)) == strType) 
					ApplyPercentage(pUpdateRow, dblPercent);
		
				pUpdateRow = pUpdateRow->GetNextRow();
			}

		}
		else { // (b.eyers 2015-05-13) - PLID 65977 - if the $ text box is active
			
			CString strAmount;
			COleCurrency cyTmp;
			GetDlgItemText(IDC_QUICK_COMM_AMOUNT, strAmount);

			if (strAmount.IsEmpty()) {
				MsgBox("You must enter an amount before applying.");
				return;
			}

			cyTmp = ParseCurrencyFromInterface(strAmount);

			if (cyTmp.GetStatus() == COleCurrency::invalid) {
				AfxMessageBox("Please enter a valid amount.");
				return;
			}

			if (cyTmp <= COleCurrency(0, 0)) {
				AfxMessageBox("You may only assign a commission amount that is greater than $0.00.");
				return;
			}

			if (cyTmp > COleCurrency(100000000, 0)) {
				CString str;
				str.Format("Practice does not allow an amount greater than %s.", FormatCurrencyForInterface(COleCurrency(100000000, 0), TRUE, TRUE));
				AfxMessageBox(str);
				return;
			}

			// Get the type of service item
			long nTypeID = -1;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCommTypeCombo->GetCurSel();
			if (pRow != NULL && pRow->GetValue(0).vt != VT_NULL) {
				nTypeID = VarLong(pRow->GetValue(0));
			}
			else {
				MsgBox("You must select a type of service item before applying.");
				return;
			}

			CString strType;
			if (nTypeID == 0)
				strType = "Service Code";
			else if (nTypeID == 1)
				strType = "Inventory Item";
			else if (nTypeID == 2)
				strType = "Gift Certificate";

			CString str;
			str.Format("Are you sure you wish to update all %ss to %s? "
				"Note, items with a percentages will be cleared and replaced with the dollar amount if you continue.", strType, FormatCurrencyForInterface(cyTmp));
			if (AfxMessageBox(str, MB_YESNO) != IDYES)
				return;

			CWaitCursor pWait;

			NXDATALIST2Lib::IRowSettingsPtr pUpdateRow = m_pCurrentList->GetFirstRow();
		
			while (pUpdateRow) {
				if (VarString(pUpdateRow->GetValue(clfType)) == strType) 
					ApplyAmount(pUpdateRow, cyTmp);

				pUpdateRow = pUpdateRow->GetNextRow();
			}
			
		}

		//update our flag for things changing
		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}
			
	}NxCatchAll("Error in CCommissionSetupWnd::OnApplyQuickComm");	
}

void CCommissionSetupWnd::OnApplyQuickRule() 
{
	try{
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the rule
		long nRuleID = -1;
		CString strRuleName = "";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pQuickRuleCombo->GetCurSel();
		if (pRow != NULL && pRow->GetValue(0).vt != VT_NULL) {
			nRuleID = VarLong(pRow->GetValue(0));
			strRuleName = VarString(pRow->GetValue(1));
		}
		else {
			MsgBox("You must select a rule before applying.");
			return;
		}

		// Get the type of service item
		long nTypeID = -1;
		pRow = m_pRuleTypeCombo->GetCurSel();
		if (pRow != NULL && pRow->GetValue(0).vt != VT_NULL) {
			nTypeID = VarLong(pRow->GetValue(0));
		}
		else {
			MsgBox("You must select a type of service item before applying.");
			return;
		}

		// Get the information for the rule
		double dblPercentage = 0;
		COleCurrency cyMoneyThreshold = COleCurrency(0,0);
		BOOL bOverwritePriorRules = TRUE;
		COleDateTime dtStartDate = COleDateTime::GetCurrentTime();
		COleDateTime dtEndDate = COleDateTime::GetCurrentTime();
		COleCurrency cyAmount = COleCurrency(0, 0); // (b.eyers 2015-05-15) - PLID 65978
		_RecordsetPtr prsRuleInfo = CreateParamRecordset("SELECT * FROM CommissionRulesT WHERE ID = {INT}", nRuleID);
		//TES 7/20/2015 - PLID 66601 - Track whether this rule is $ vs. %
		_variant_t varCommissionAmount = g_cvarNull;
		if (!prsRuleInfo->eof) {
			// Get the rule information
			dblPercentage = AdoFldDouble(prsRuleInfo, "Percentage", NULL); // (b.eyers 2015-05-15) - PLID 65978
			cyMoneyThreshold = AdoFldCurrency(prsRuleInfo, "MoneyThreshold");
			bOverwritePriorRules = AdoFldBool(prsRuleInfo, "OverwritePriorRules");
			dtStartDate = AdoFldDateTime(prsRuleInfo, "StartDate");
			dtEndDate = AdoFldDateTime(prsRuleInfo, "EndDate");
			cyAmount = AdoFldCurrency(prsRuleInfo, "Amount", COleCurrency(0, 0)); // (b.eyers 2015-05-15) - PLID 65978
			varCommissionAmount = prsRuleInfo->Fields->GetItem("Amount")->Value;
		}

		CString strType;
		if(m_radioTieredCommissions.GetCheck()) {
			if (nTypeID == 0) {
				strType = "Service Codes";
			}
			else if (nTypeID == 1) {
				strType = "Inventory Items";
			}
			else if (nTypeID == 2) {
				strType = "Gift Certificates";
			}
			else {
				strType = "All Sales";
			}

			CString strWarn;
			strWarn.Format("Are you sure you wish to add the rule '%s' to %s?", strRuleName, strType);

			//ensure the rule doesn't already exist for the same type
			//also ensure you cannot mix & match "all sales" with anything else
			IRowSettingsPtr pRow = m_pTieredRuleList->GetFirstRow();
			CString strWarnDateList;
			long nWarnedDates = 0;
			while (pRow) {

				long nCurTypeID = VarLong(pRow->GetValue(trlcTypeID));
				long nCurRuleID = VarLong(pRow->GetValue(trlcRuleID));

				if (nRuleID == nCurRuleID && nCurTypeID == nTypeID) {
					//duplicate rule
					CString str;
					str.Format("The rule '%s' already exists for %s. Please select a different rule.", strRuleName, strType);
					AfxMessageBox(str);
					return;
				}
				else if (nCurTypeID == -1 && nTypeID != -1 || nCurTypeID != -1 && nTypeID == -1) {
					//cannot mix and match "All Sales"
					AfxMessageBox("You cannot add rules for All Sales and any other type at the same time for the same provider.\n"
						"If All Sales is used, it must be the only rule type used.");
					return;
				}

				CString strCurRuleName = VarString(pRow->GetValue(trlcName));
				COleDateTime dtCurStart = VarDateTime(pRow->GetValue(trlcStartDate));
				COleDateTime dtCurEnd = VarDateTime(pRow->GetValue(trlcEndDate));
				COleCurrency cyCurMoneyThreshold = VarCurrency(pRow->GetValue(trlcMoneyThreshold));
				//TES 7/20/2015 - PLID 66601 - Track whether this rule is $ vs. %
				_variant_t varCurCommissionAmount = pRow->GetValue(trlcAmount);

				//TES 7/20/2015 - PLID 66601 - Don't allow a % rule to overlap a $ rule
				if (nCurTypeID == nTypeID && varCommissionAmount.vt != varCurCommissionAmount.vt) {
					//ignore rules that end before this one starts, or starts after this one ends
					if (dtCurEnd >= dtStartDate && dtCurStart <= dtEndDate) {
						CString str;
						if (varCurCommissionAmount.vt == VT_NULL) {
							str = "You cannot add a rule with a $ commission to a tier with an existing % commission during the same date range.";
						}
						else {
							str = "You cannot add a rule with a % commission to a tier with an existing $ commission during the same date range.";
						}
						AfxMessageBox(str);
						return;
					}
				}

				// (j.jones 2009-11-20 12:22) - PLID 29046 - disallow adding two rules to the same type, with overlapping dates,
				// with the same money threshhold
				// (note: they can still change rules that are already linked, if they want to be ridiculous about it)
				if (nCurTypeID == nTypeID && cyCurMoneyThreshold == cyMoneyThreshold) {
					//ignore rules that end before this one starts, or starts after this one ends
					if(dtCurEnd >= dtStartDate && dtCurStart <= dtEndDate) {
						CString str;
						str.Format("The rule '%s' already exists for %s with a required sales limit of %s for the date range of %s - %s. "
							"You cannot add two tiers with the same rule type and sales limit in the same date range.",
							strRuleName, strType,
							FormatCurrencyForInterface(cyMoneyThreshold),
							FormatDateTimeForInterface(dtCurStart, NULL, dtoDate), FormatDateTimeForInterface(dtCurEnd, NULL, dtoDate));						
						AfxMessageBox(str);
						return;
					}
				}

				// (j.jones 2009-11-20 10:07) - PLID 29046 - we must warn if there are overlapping rule dates
				//for the same service type, which could make for wacky reports, we can't stop them from this,
				//however, and we can't stop them from editing rules later in the setup, but we can at least warn
							
				//ignore rules that match our dates 100%, which is the ideal case
				if(nCurTypeID == nTypeID && (dtCurStart != dtStartDate || dtCurEnd != dtEndDate)) {
					//- look for rules that start within our range, but not exactly when ours starts
					//- look for rules that end within our range, but not exactly when ours ends
					//- look for rules that start before ours, and end after ours
					if((dtCurStart > dtStartDate && dtCurStart <= dtEndDate)
						|| (dtCurEnd >= dtStartDate && dtCurEnd < dtEndDate)
						|| (dtCurStart < dtStartDate && dtCurEnd > dtEndDate)) {

						nWarnedDates++;

						if(nWarnedDates == 6) {
							//add "more" just once
							strWarnDateList += "<More...>\n";
						}
						else {
							//add to our warning list
							CString str;
							str.Format("%s (%s - %s)\n", strCurRuleName, FormatDateTimeForInterface(dtCurStart, NULL, dtoDate), FormatDateTimeForInterface(dtCurEnd, NULL, dtoDate));
							strWarnDateList += str;
						}
					}
				}

				pRow = pRow->GetNextRow();
			}

			if(!strWarnDateList.IsEmpty()) {
				strWarn.Format("This rule is set to be applied to %s between %s and %s.\n"
					"The following rules for this provider have overlapping dates for %s that do not match this rule:\n\n"
					"%s\n"
					"When commission reports are run, they will calculate all rules that are valid on the last date in the report range. "
					"It is recommended that all overlapping tiers use the same date range.\n\n"
					"Are you sure you wish to add the rule '%s' to %s?",
					strType, FormatDateTimeForInterface(dtStartDate, NULL, dtoDate), FormatDateTimeForInterface(dtEndDate, NULL, dtoDate),
					strType, strWarnDateList, strRuleName, strType);
			}

			//confirm			
			if(IDNO == MessageBox(strWarn, "Practice", MB_YESNO)) {
				return;
			}
		}
		else {
			if (nTypeID == 0)
				strType = "Service Code";
			else if (nTypeID == 1)
				strType = "Inventory Item";
			else if (nTypeID == 2)
				strType = "Gift Certificate";

			//confirm
			CString str;
			str.Format("Are you sure you wish to add the rule '%s' to all %ss?", strRuleName, strType);
			if(AfxMessageBox(str, MB_YESNO) != IDYES)
				return;
		}

		CWaitCursor pWait;

		// (j.jones 2009-11-18 15:58) - PLID 29046 - add to the correct datalist
		if(m_radioTieredCommissions.GetCheck()) {
			//add to the tiered commission list

			IRowSettingsPtr pRow = m_pTieredRuleList->GetNewRow();
			pRow->PutValue(trlcRuleID, (long)nRuleID);
			pRow->PutValue(trlcTypeID, (long)nTypeID);
			pRow->PutValue(trlcTypeName, _bstr_t(strType));
			pRow->PutValue(trlcName, _bstr_t(strRuleName));
			//pRow->PutValue(trlcPercent, _variant_t(dblPercentage));
			pRow->PutValue(trlcMoneyThreshold, _variant_t(cyMoneyThreshold));
			pRow->PutValue(trlcOverwritePriorRules, bOverwritePriorRules ? "This tier and lower" : "This tier only");
			pRow->PutValue(trlcStartDate, _variant_t(dtStartDate, VT_DATE));
			pRow->PutValue(trlcEndDate, _variant_t(dtEndDate, VT_DATE));

			// (b.eyers 2015-05-15) - PLID 65982 - Display the correct column and make sure the other column doesn't display anything, $ column added
			_variant_t varNull;
			varNull.vt = VT_NULL;

			if (cyAmount != COleCurrency(0, 0)) {
				pRow->PutValue(trlcPercent, varNull);
				pRow->PutValue(trlcAmount, _variant_t(cyAmount));
			}
			else {
				pRow->PutValue(trlcPercent, _variant_t(dblPercentage));
				pRow->PutValue(trlcAmount, varNull);
			}


			m_pTieredRuleList->AddRowSorted(pRow, NULL);

			TieredRuleInfo riRule;
			riRule.nProvID = m_nProviderID;
			riRule.nRuleID = nRuleID;
			riRule.nRuleType = nTypeID;
			riRule.strRuleName = strRuleName;

			// Check to make sure it isn't in the remove array, if so take it out and don't add it to the
			// added rules array
			bool bFoundRule = false;
			for (int i = 0; i < m_aryRemovedTieredRules.GetSize() && !bFoundRule; i++) {
				if (riRule.nProvID == m_aryRemovedTieredRules.GetAt(i).nProvID &&
					riRule.nRuleID == m_aryRemovedTieredRules.GetAt(i).nRuleID &&
					riRule.nRuleType == m_aryRemovedTieredRules.GetAt(i).nRuleType) {
					bFoundRule = true;
					// (s.tullis 2015-07-22 09:37) - PLID 66645 - This should remove from Tiered Rules not the basic Rules removed array
					m_aryRemovedTieredRules.RemoveAt(i);
				}
			}
			if (!bFoundRule) {
				// Add the rule to the added rules array
				m_aryAddedTieredRules.Add(riRule);
			}
		}
		else {
			//update the basic commission list

			// Update the rows
			NXDATALIST2Lib::IRowSettingsPtr pUpdateRow = m_pCurrentList->GetFirstRow();
			while (pUpdateRow) {
				if(VarString(pUpdateRow->GetValue(clfType)) == strType)
					AddBasicRuleToRow(pUpdateRow, nRuleID, strRuleName, dblPercentage, cyAmount); // (b.eyers 2015-05-15) - PLID 65978

				pUpdateRow = pUpdateRow->GetNextRow();
			}
		}

		//update our flag for things changing
		m_bChanged = true;

		//clear the rule combo
		m_pQuickRuleCombo->SetSelByColumn(0, _variant_t((long)-1));

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}
			
	}NxCatchAll("Error in CCommissionSetupWnd::OnApplyQuickComm");		
}

void CCommissionSetupWnd::OnHideshowRules() 
{
	try {
		CString strBtnText;
		m_btnHideShowRules.GetWindowText(strBtnText);

		if (strBtnText == "Hide All Rules") {
			// Let's collapse all the rows in the current list
			ExpandAllRows(VARIANT_FALSE);
			m_btnHideShowRules.SetWindowText("Show All Rules");
		}
		else {
			// Let's expand all the rows in the current list
			ExpandAllRows(VARIANT_TRUE);
			m_btnHideShowRules.SetWindowText("Hide All Rules");
		}

	}NxCatchAll("Error in CCommissionSetupWnd::OnHideshowRules");
}

void CCommissionSetupWnd::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	// (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
	SetControlPositions();
	Invalidate();
	
}

void CCommissionSetupWnd::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		Load();

	}NxCatchAll("Error in CCommissionSetupWnd::UpdateView");
}


void CCommissionSetupWnd::OnEditingFinishingCurrentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		// (a.wetta 2007-05-18 12:43) - PLID 25394 - If they have write permission with password, have them enter the password
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
			*pbContinue = TRUE;
			*pbCommit = FALSE;
			return;
		}

		// (b.eyers 2015-05-11) - PLID 65976
		_variant_t varNewValue(pvarNewValue);
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		CString strEntered = strUserEntered;
		if (nCol == clfPercentage) {
			// (b.eyers 2015-05-11) - PLID 65976 - cannot allow data in both amount and percent columns for a row
			if (pRow->GetValue(clfAmount).vt != VT_NULL && pRow->GetValue(clfAmount).vt != VT_EMPTY && (!strEntered.IsEmpty())) {
				MsgBox("You cannot enter both a currency amount and a percent amount.");
				*pvarNewValue = g_cvarNull;
				return;
			}

			if (*pbCommit != FALSE && (VarDouble(varNewValue, 0) > 100.00 ||
				VarDouble(varNewValue, 0) < 0.00)){
				MsgBox("Please enter a valid percentage from 1 - 100, or enter 0 to clear out the commission.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
		else if (nCol == clfAmount) {
			// (b.eyers 2015-05-11) - PLID 65976 - cannot allow data in both amount and percent columns for a row
			if (pRow->GetValue(clfPercentage).vt != VT_NULL && pRow->GetValue(clfPercentage).vt != VT_EMPTY && (!strEntered.IsEmpty())) {
				MsgBox("You cannot enter both a currency amount and a percent amount.");
				*pvarNewValue = g_cvarNull;
				return;
			}
			// (b.eyers 2015-05-11) - PLID 65976 - do not allow negative amounts
			if (*pbCommit != FALSE && VarCurrency(varNewValue) < COleCurrency(0,0)) {
				MsgBox("Please enter an amount greater than 0, or enter 0 to clear out the commission.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll("Error in CCommissionSetupWnd::OnEditingFinishingCurrentList");
}

void CCommissionSetupWnd::OnKillfocusQuickComm() 
{
	try {
		CString strValue;
		GetDlgItemText(IDC_QUICK_COMM, strValue);

		// Make sure it is a valid number
		if (strValue.SpanIncluding("1234567890.").GetLength() != strValue.GetLength() ||
			atof(strValue) > 100.00) {
			// It's not a valid number
			MsgBox("Please enter a valid percentage from 1 - 100.");
			CString strPercent;
			strPercent.Format("%.2f", m_dblPercent);
			SetDlgItemText(IDC_QUICK_COMM, strPercent);
		}
		else {
			// Round the number
			CString strPercent;
			strPercent.Format("%.2f", atof(strValue));
			SetDlgItemText(IDC_QUICK_COMM, strPercent);
			m_dblPercent = atof(strPercent);
		}

	}NxCatchAll("Error in CCommissionSetupWnd::OnKillfocusQuickComm");
}

// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
void CCommissionSetupWnd::OnRadioBasicCommissions()
{
	try {

		//flag that we need to save
		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}

		//properly display the screen
		DisplayControls(m_radioTieredCommissions.GetCheck());

	}NxCatchAll("Error in CCommissionSetupWnd::OnRadioBasicCommissions");
}

void CCommissionSetupWnd::OnRadioTieredCommissions()
{
	try {

		OnRadioBasicCommissions();

	}NxCatchAll("Error in CCommissionSetupWnd::OnRadioBasicCommissions");
}

// (j.jones 2009-11-18 11:10) - PLID 29046 - added DisplayControls(), which will
// display the proper controls for basic or tiered commissions, as indicated
// by the parameter
void CCommissionSetupWnd::DisplayControls(BOOL bUseTieredCommissions)
{
	try {

		if(!bUseTieredCommissions) {
			//show the controls for "basic commissions"
			GetDlgItem(IDC_CURRENT_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_COMM_ITEM_TYPE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_HIDESHOW_RULES)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ADV_SETUP)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_APPLY_QUICK_COMM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SET_COMMISSION_LABEL_1)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SET_COMMISSION_LABEL_2)->ShowWindow(SW_SHOW);						
			GetDlgItem(IDC_COMMISSION_QUICK_SETUP_LABEL)->ShowWindow(SW_SHOW);

			// (b.eyers 2015-05-13) - PLID 65977
			if (m_bPercent) {
				GetDlgItem(IDC_QUICK_COMM)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_QUICK_COMM_AMOUNT)->ShowWindow(SW_HIDE);
			}
			else {
				GetDlgItem(IDC_QUICK_COMM)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_QUICK_COMM_AMOUNT)->ShowWindow(SW_SHOW);
			}

			m_nxstaticCurrentCommissionLabel.SetWindowText("Current Commissions");
			GetDlgItem(IDC_COMM_RULE_SETUP)->SetWindowText("Basic Commission Rule Setup...");

			GetDlgItem(IDC_TIERED_RULE_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_IGNORE_SHOP_FEE)->ShowWindow(SW_HIDE);

			//Refresh the rule list
			CString strRuleWhere = "IsTieredCommission = 0";
			if(!IsDlgButtonChecked(IDC_SHOW_OUTDATED_RULES)) {
				strRuleWhere += " AND EndDate >= dbo.AsDateNoTime(GetDate())";
			}
			
			m_pQuickRuleCombo->PutWhereClause(_bstr_t(strRuleWhere));
			m_pQuickRuleCombo->Requery();

			//need to remove the "all sales" rule row, if it exists
			IRowSettingsPtr pRow = m_pRuleTypeCombo->FindByColumn(0, (long)-1, m_pRuleTypeCombo->GetFirstRow(), FALSE);
			if(pRow) {
				m_pRuleTypeCombo->RemoveRow(pRow);
			}
		}
		else {
			//show the controls for "tiered commissions"
			GetDlgItem(IDC_CURRENT_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COMM_ITEM_TYPE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_HIDESHOW_RULES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ADV_SETUP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_APPLY_QUICK_COMM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUICK_COMM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SET_COMMISSION_LABEL_1)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SET_COMMISSION_LABEL_2)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_COMMISSION_QUICK_SETUP_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUICK_COMM_AMOUNT)->ShowWindow(SW_HIDE); // (b.eyers 2015-05-13) - PLID 65977

			m_nxstaticCurrentCommissionLabel.SetWindowText("Current Tiered Commission Rules");
			GetDlgItem(IDC_COMM_RULE_SETUP)->SetWindowText("Tiered Commission Rule Setup...");

			GetDlgItem(IDC_TIERED_RULE_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_CHECK_IGNORE_SHOP_FEE)->ShowWindow(SW_SHOW);

			//Refresh the rule list
			CString strRuleWhere = "IsTieredCommission = 1";
			if(!IsDlgButtonChecked(IDC_SHOW_OUTDATED_RULES)) {
				strRuleWhere += " AND EndDate >= dbo.AsDateNoTime(GetDate())";
			}
			
			m_pQuickRuleCombo->PutWhereClause(_bstr_t(strRuleWhere));
			m_pQuickRuleCombo->Requery();

			//need to add the "all sales" rule row, if it does not exist
			IRowSettingsPtr pRow = m_pRuleTypeCombo->FindByColumn(0, (long)-1, m_pRuleTypeCombo->GetFirstRow(), FALSE);
			if(pRow == NULL) {
				//need to add this row
				pRow = m_pRuleTypeCombo->GetNewRow();
				pRow->PutValue(0, (long)-1);
				pRow->PutValue(1, _bstr_t("All Sales"));
				m_pRuleTypeCombo->AddRowSorted(pRow, NULL);
			}
		}

		Invalidate(TRUE);

	}NxCatchAll("Error in CCommissionSetupWnd::DisplayControls");
}

// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
void CCommissionSetupWnd::OnCheckIgnoreShopFee()
{
	try {

		//flag that we need to save
		m_bChanged = true;

		if (m_nProviderID != -1 && m_bTabMode) {
			Save();
		}

	}NxCatchAll("Error in CCommissionSetupWnd::OnCheckIgnoreShopFee");
}

// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
void CCommissionSetupWnd::OnRButtonDownTieredRuleList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		// Set the row to whatever they right clicked on
		// Actual work is done in the ContextMenu function
		GetDlgItem(IDC_TIERED_RULE_LIST)->SetFocus();
		m_pTieredRuleList->PutCurSel(pRow);

	}NxCatchAll("Error in CCommissionSetupWnd::OnRButtonDownTieredRuleList");
}

// (b.savon 2011-12-06 17:34) - PLID 46902 - Enforce Read Only Permissions
void CCommissionSetupWnd::EnforceNoProviderCommissionPermissions()
{
	GetDlgItem(IDC_COMMISSION_PROV_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_ADV_SETUP)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMM_RULE_SETUP)->EnableWindow(FALSE);
	GetDlgItem(IDC_SHOW_OUTDATED_RULES)->EnableWindow(FALSE);
	GetDlgItem(IDC_HIDESHOW_RULES)->EnableWindow(FALSE);
	GetDlgItem(IDC_APPLY_QUICK_COMM)->EnableWindow(FALSE);
	GetDlgItem(IDC_APPLY_QUICK_RULE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_BASIC_COMMISSIONS)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_TIERED_COMMISSIONS)->EnableWindow(FALSE);
	GetDlgItem(IDC_QUICK_COMM)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMM_ITEM_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_QUICK_RULE_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_RULE_ITEM_TYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_IGNORE_SHOP_FEE)->EnableWindow(FALSE);
	GetDlgItem(IDC_TIERED_RULE_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_QUICK_COMM_AMOUNT)->EnableWindow(FALSE); // (b.eyers 2015-05-13) - PLID 65977
}

// (b.eyers 2015-05-13) - PLID 65977 - Switch between % and $
LRESULT CCommissionSetupWnd::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		
		if (m_bPercent) {
			//currently set to %, change to $
			m_nxstaticSetCommissionLabel2.SetText("$ for all");
			m_bPercent = FALSE; 
			GetDlgItem(IDC_QUICK_COMM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUICK_COMM_AMOUNT)->ShowWindow(SW_SHOW);
			//reset default text to $0
			CString tmpStr;
			tmpStr = FormatCurrencyForInterface(m_cyAmount);
			SetDlgItemText(IDC_QUICK_COMM_AMOUNT, tmpStr);
			//set % back to 0
			m_dblPercent = 0;
		}
		else {
			m_nxstaticSetCommissionLabel2.SetText("% for all");
			m_bPercent = TRUE; 
			GetDlgItem(IDC_QUICK_COMM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_QUICK_COMM_AMOUNT)->ShowWindow(SW_HIDE);
			//reset default text to 0
			CString strPercent;
			strPercent.Format("%.2f", m_dblPercent);
			SetDlgItemText(IDC_QUICK_COMM, strPercent);
			//set $ back to 0 
			m_cyAmount = COleCurrency(0, 0);
		}

	}NxCatchAll("Error in OnLabelClick");
	return 0;
}

// (b.eyers 2015-05-13) - PLID 65977 - Check that the $ amount is a valid, non-negative amount
void CCommissionSetupWnd::OnKillfocusQuickCommAmount()
{
	try {
		CString strValue;
		COleCurrency cyTmp;
		GetDlgItemText(IDC_QUICK_COMM_AMOUNT, strValue);
		cyTmp = ParseCurrencyFromInterface(strValue);

		BOOL bFailed = FALSE;

		if (strValue == "") {
			cyTmp = COleCurrency(0, 0);
		}

		if (cyTmp.GetStatus() == COleCurrency::invalid) {
			AfxMessageBox("Please enter a valid amount.");
			bFailed = TRUE;
		}

		if (!bFailed && cyTmp < COleCurrency(0, 0)) {
			AfxMessageBox("Please enter an amount greater than $0.00.");
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
			SetDlgItemText(IDC_QUICK_COMM_AMOUNT, FormatCurrencyForInterface(m_cyAmount));
		}
		else {
			SetDlgItemText(IDC_QUICK_COMM_AMOUNT, FormatCurrencyForInterface(cyTmp));
			m_cyAmount = cyTmp;
		}

	}NxCatchAll("Error in CCommissionSetupWnd::OnKillfocusQuickCommAmount");
}