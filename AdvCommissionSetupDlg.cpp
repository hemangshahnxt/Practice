// AdvCommissionSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "contacts.h"
#include "AdvCommissionSetupDlg.h"
#include "CommissionRuleSetupDlg.h"
#include <afxtempl.h>
#include "AuditTrail.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.wetta 2007-03-28 16:35) - PLID 25326 - Dialog created

/////////////////////////////////////////////////////////////////////////////
// CAdvCommissionSetupDlg dialog


CAdvCommissionSetupDlg::CAdvCommissionSetupDlg(CWnd* pParent)
	: CNxDialog(CAdvCommissionSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvCommissionSetupDlg)
	//}}AFX_DATA_INIT
	m_backgroundColor = GetNxColor(GNC_ADMIN, 0);
	m_dblPercent = 0;
	m_cyAmount = COleCurrency(0, 0); // (b.eyers 2015-05-13) - PLID 65981
}


void CAdvCommissionSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvCommissionSetupDlg)
	DDX_Control(pDX, IDC_SHOW_OUT_DATED_RULES, m_btnShowOutdatedRules);
	DDX_Control(pDX, IDC_SHOW_INACTIVE_PROV, m_btnShowInactiveProv);
	DDX_Control(pDX, IDC_COMM_RULE_SETUP, m_btnCommRuleSetup);
	DDX_Control(pDX, IDC_CLEAR_SELECTED_RULES, m_btnClearSelectedRules);
	DDX_Control(pDX, IDC_CLEAR_SELECTED, m_btnClearSelected);
	DDX_Control(pDX, IDC_REMOVE_SELECTED_RULE, m_btnRemoveSelectedRule);
	DDX_Control(pDX, IDC_APPLY_SELECTED_RULE, m_btnApplySelectedRule);
	DDX_Control(pDX, IDC_APPLY_SELECTED, m_btnApplySelected);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_SELECT_ONE_CITEM, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_CITEMS, m_btnSelectAll);
	DDX_Control(pDX, IDC_REMOVE_ONE_CITEM, m_btnRemoveOne);
	DDX_Control(pDX, IDC_REMOVE_ALL_CITEMS, m_btnRemoveAll);
	DDX_Control(pDX, IDC_REMOVE_PROV, m_btnRemoveProvider);
	DDX_Control(pDX, IDC_ADD_PROV, m_btnAddProvider);
	DDX_Control(pDX, IDC_COMM_COLOR2, m_bkgColor2);
	DDX_Control(pDX, IDC_COMM_COLOR3, m_bkgColor3);
	DDX_Control(pDX, IDC_COMM_COLOR4, m_bkgColor4);
	DDX_Control(pDX, IDC_APPLY_PERCENTAGE, m_nxeditApplyPercentage);
	DDX_Control(pDX, IDC_APPLY_LABEL, m_nxlabelApply);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvCommissionSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvCommissionSetupDlg)
	ON_BN_CLICKED(IDC_ADD_PROV, OnAddProv)
	ON_BN_CLICKED(IDC_REMOVE_PROV, OnRemoveProv)
	ON_BN_CLICKED(IDC_SELECT_ALL_CITEMS, OnSelectAllCitems)
	ON_BN_CLICKED(IDC_SELECT_ONE_CITEM, OnSelectOneCitem)
	ON_BN_CLICKED(IDC_REMOVE_ONE_CITEM, OnRemoveOneCitem)
	ON_BN_CLICKED(IDC_REMOVE_ALL_CITEMS, OnRemoveAllCitems)
	ON_BN_CLICKED(IDC_COMM_RULE_SETUP, OnCommRuleSetup)
	ON_BN_CLICKED(IDC_REMOVE_SELECTED_RULE, OnRemoveSelectedRule)
	ON_BN_CLICKED(IDC_APPLY_SELECTED, OnApplySelected)
	ON_BN_CLICKED(IDC_APPLY_SELECTED_RULE, OnApplySelectedRule)
	ON_BN_CLICKED(IDC_CLEAR_SELECTED_RULES, OnClearSelectedRules)
	ON_BN_CLICKED(IDC_CLEAR_SELECTED, OnClearSelected)
	ON_BN_CLICKED(IDC_SHOW_INACTIVE_PROV, OnShowInactiveProv)
	ON_BN_CLICKED(IDC_SHOW_OUT_DATED_RULES, OnShowOutDatedRules)
	ON_WM_CTLCOLOR()
	ON_EN_KILLFOCUS(IDC_APPLY_PERCENTAGE, OnKillfocusApplyPercentage)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick) // (b.eyers 2015-05-13) - PLID 65981
	ON_EN_KILLFOCUS(IDC_APPLY_AMOUNT, OnKillfocusApplyAmount) // (b.eyers 2015-05-13) - PLID 65981
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvCommissionSetupDlg message handlers

BOOL CAdvCommissionSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		//bind datalists
		
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pAvailProviders = BindNxDataList2Ctrl(IDC_AVAIL_PROV_LIST, true);
		m_pSelProviders = BindNxDataList2Ctrl(IDC_SEL_PROV_LIST, false);
		m_pAvailList = BindNxDataList2Ctrl(IDC_AVAIL_LIST, false);

		CString str;
		str.Format("(SELECT ServiceT.ID AS ID, "
					"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 'Service Code' WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' WHEN GCTypesT.ServiceID IS NOT NULL THEN 'Gift Certificate' ELSE 'Service Code' END AS Type, "
					"CASE WHEN CPTCodeT.ID IS NOT NULL THEN CPTCodeT.Code WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' WHEN GCTypesT.ServiceID IS NOT NULL THEN 'Gift Certificate' END AS Code, ServiceT.Name AS Name, CategoriesT.Name AS Category "
					"FROM ServiceT "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN GCTypesT ON ServiceT.ID = GCTypesT.ServiceID "
					"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
					// (c.haag 2007-10-12 11:36) - PLID 24872 - Exclude inactive codes
					"WHERE ServiceT.Active = 1"
					") CommissionQ");
		m_pAvailList->PutFromClause(_bstr_t(str));
		m_pAvailList->Requery();

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pChangesList = BindNxDataList2Ctrl(IDC_CHANGES_LIST, false);
		m_pAddRuleCombo = BindNxDataList2Ctrl(IDC_ADD_RULE_LIST, true);
		m_pRemoveRuleCombo = BindNxDataList2Ctrl(IDC_REMOVE_RULE_LIST, true);

		//setup nxiconbuttons
		m_btnSelectOne.AutoSet(NXB_DOWN);
		m_btnSelectAll.SetIcon(IDI_DDARROW);
		m_btnRemoveOne.AutoSet(NXB_UP);
		m_btnRemoveAll.SetIcon(IDI_UUARROW);
		m_btnAddProvider.AutoSet(NXB_RIGHT);
		m_btnRemoveProvider.AutoSet(NXB_LEFT);
		m_btnOk.AutoSet(NXB_CLOSE);
		m_btnApplySelectedRule.AutoSet(NXB_MODIFY);
		m_btnApplySelected.AutoSet(NXB_MODIFY);
		m_btnClearSelectedRules.AutoSet(NXB_DELETE);
		m_btnClearSelected.AutoSet(NXB_DELETE);
		m_btnRemoveSelectedRule.AutoSet(NXB_DELETE);

		// Set the background color
		m_bkgColor2.SetColor(m_backgroundColor);
		m_bkgColor3.SetColor(m_backgroundColor);
		m_bkgColor4.SetColor(m_backgroundColor);

		// (b.eyers 2015-05-13) - PLID 65981 - $ text box needs to be hidden when dialog opens
		GetDlgItem(IDC_APPLY_AMOUNT)->ShowWindow(SW_HIDE);

		// Set the percent edit box default text
		CString strPercent;
		strPercent.Format("%.2f%%", m_dblPercent); 
		SetDlgItemText(IDC_APPLY_PERCENTAGE, strPercent);

		// (b.eyers 2015-05-13) - PLID 65981 - set default text for label
		m_nxlabelApply.SetType(dtsHyperlink);
		m_nxlabelApply.SetText("Apply % commission:");
		m_bPercent = TRUE; //default is %

	}NxCatchAll("Error in CAdvCommissionSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvCommissionSetupDlg::OnCancel() 
{
	
	CDialog::OnCancel();
}

void CAdvCommissionSetupDlg::OnOK() 
{
	
	CDialog::OnOK();
}

// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Removed the MoveSelectedRows() method, and for efficiency 
// used the TakeCurrentRowAddSorted() method directly where needed instead.

void CAdvCommissionSetupDlg::OnAddProv() 
{
	CWaitCursor wait;

	try {
		//(e.lally 2007-10-02) PLID 24872 - Fixed this to move all selected rows instead of just one
		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Switched to the TakeCurrentRowAddSorted() method
		m_pSelProviders->TakeCurrentRowAddSorted(m_pAvailProviders, NULL);

	} NxCatchAll("Error adding provider");	
}

void CAdvCommissionSetupDlg::OnRemoveProv() 
{
	CWaitCursor wait;

	try {
		//(e.lally 2007-10-02) PLID 24872 - Fixed this to move all selected rows instead of just one
		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Switched to the TakeCurrentRowAddSorted() method
		m_pAvailProviders->TakeCurrentRowAddSorted(m_pSelProviders, NULL);
	} NxCatchAll("Error removing provider");
}

void CAdvCommissionSetupDlg::OnSelectAllCitems() 
{
	CWaitCursor wait;

	try {
		m_pChangesList->TakeAllRows(m_pAvailList);
	} NxCatchAll("Error selecting all items")
}

void CAdvCommissionSetupDlg::OnSelectOneCitem() 
{
	CWaitCursor wait;

	try {
		//(e.lally 2007-10-02) PLID 24872 - Fixed this to move all selected rows instead of just one
		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Switched to the TakeCurrentRowAddSorted() method
		m_pChangesList->TakeCurrentRowAddSorted(m_pAvailList, NULL);
	} NxCatchAll("Error selecting single items");
}

void CAdvCommissionSetupDlg::OnRemoveOneCitem() 
{
	CWaitCursor wait;

	try {
		//(e.lally 2007-10-02) PLID 24872 - Fixed this to remove all selected rows instead of just one
		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Switched to the TakeCurrentRowAddSorted() method
		m_pAvailList->TakeCurrentRowAddSorted(m_pChangesList, NULL);
	} NxCatchAll("Error removing single items");
}

void CAdvCommissionSetupDlg::OnRemoveAllCitems() 
{
	CWaitCursor wait;

	try {
		m_pAvailList->TakeAllRows(m_pChangesList);
	} NxCatchAll("Error removing all items");
}

void CAdvCommissionSetupDlg::OnCommRuleSetup() 
{
	try {
		CCommissionRuleSetupDlg dlg(this);
		dlg.m_backgroundColor = m_backgroundColor;
		// (j.jones 2009-11-18 14:48) - PLID 29046 - this dialog is only
		// available for the basic commission structure, so the rule editor
		// will also be basic only
		dlg.m_bUseTieredRules = FALSE;
		dlg.DoModal();

		// Update the rule combos
		m_pAddRuleCombo->Requery();
		m_pRemoveRuleCombo->Requery();
	}NxCatchAll("Error in CAdvCommissionSetupDlg::OnCommRuleSetup");
}

BEGIN_EVENTSINK_MAP(CAdvCommissionSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvCommissionSetupDlg)
	ON_EVENT(CAdvCommissionSetupDlg, IDC_AVAIL_PROV_LIST, 3 /* DblClickCell */, OnDblClickCellAvailProvList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvCommissionSetupDlg, IDC_SEL_PROV_LIST, 3 /* DblClickCell */, OnDblClickCellSelProvList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvCommissionSetupDlg, IDC_AVAIL_LIST, 3 /* DblClickCell */, OnDblClickCellAvailList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvCommissionSetupDlg, IDC_CHANGES_LIST, 3 /* DblClickCell */, OnDblClickCellChangesList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvCommissionSetupDlg::OnDblClickCellAvailProvList(LPDISPATCH lpRow, short nColIndex) 
{
	OnAddProv();	
}

void CAdvCommissionSetupDlg::OnDblClickCellSelProvList(LPDISPATCH lpRow, short nColIndex) 
{
	OnRemoveProv();	
}

void CAdvCommissionSetupDlg::OnDblClickCellAvailList(LPDISPATCH lpRow, short nColIndex) 
{
	OnSelectOneCitem();	
}

void CAdvCommissionSetupDlg::OnDblClickCellChangesList(LPDISPATCH lpRow, short nColIndex) 
{
	OnRemoveOneCitem();
}

BOOL CAdvCommissionSetupDlg::GetSelProvidersAndServiceItems(CArray<IDNameInfo,IDNameInfo> &aryProviders, CArray<IDNameInfo,IDNameInfo> &aryServiceItems)
{
	try {
		// Get all of the selected providers
		NXDATALIST2Lib::IRowSettingsPtr pProvRow = m_pSelProviders->GetFirstRow();
		IDNameInfo iniProvInfo;
		while (pProvRow) {
			iniProvInfo.nID = VarLong(pProvRow->GetValue(0));
			iniProvInfo.strName = VarString(pProvRow->GetValue(1));
			aryProviders.Add(iniProvInfo);
			pProvRow = pProvRow->GetNextRow();
		}
		if (aryProviders.GetSize() == 0) {
			MessageBox("You must select at least one provider before continuing.", NULL, MB_OK|MB_ICONWARNING);
			return FALSE;
		}

		// Get all of the service items
		NXDATALIST2Lib::IRowSettingsPtr pServiceRow = m_pChangesList->GetFirstRow();
		IDNameInfo iniItemInfo;
		while (pServiceRow) {
			iniItemInfo.nID = VarLong(pServiceRow->GetValue(aclfID));
			iniItemInfo.strName = VarString(pServiceRow->GetValue(aclfName));
			aryServiceItems.Add(iniItemInfo);
			pServiceRow = pServiceRow->GetNextRow();
		}
		if (aryServiceItems.GetSize() == 0) {
			MessageBox("You must select at least one service item before continuing.", NULL, MB_OK|MB_ICONWARNING);
			return FALSE;
		}
	}NxCatchAll("Error in CAdvCommissionSetupDlg::GetSelProvidersAndServiceItems");
	return TRUE;
}


CString CAdvCommissionSetupDlg::GetIDStringFromArray(CArray<IDNameInfo,IDNameInfo> &aryIDs)
{
	CString strIDString;
	for(int i = 0; i < aryIDs.GetSize(); i++)
	{
		strIDString += AsString(aryIDs.GetAt(i).nID) + ",";
	}
	strIDString.TrimRight(",");

	return strIDString;
}

void CAdvCommissionSetupDlg::OnApplySelected() 
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the provider and service item IDs
		CArray<IDNameInfo,IDNameInfo> aryProviders, aryServiceItems;
		if (!GetSelProvidersAndServiceItems(aryProviders, aryServiceItems)) {
			// Either no provider or no service item was selected
			return;
		}

		// (b.eyers 2015-05-13) - PLID 65981 - if percent, else amount
		if (m_bPercent) {

			// Get the commission percentage
			CString strPercent;
			GetDlgItemText(IDC_APPLY_PERCENTAGE, strPercent);

			if (strPercent.IsEmpty()) {
				MessageBox("You must enter a percentage before continuing.", NULL, MB_OK | MB_ICONWARNING);
				return;
			}

			//convert this string into a double value
			double dblPercent = atof(strPercent);

			if (dblPercent <= 0.00 || dblPercent > 100.00){
				MsgBox("You may only apply a percentage from 1 - 100.");
				return;
			}

			// Confirm that they want to update the commissions
			CString str;
			str.Format("Are you sure you wish to update all the selected service items for all the selected providers to %.2f%%? \n"
				"Note, items with a dollar amount will be cleared and replaced with the percentage if you continue.", dblPercent); // (b.eyers 2015-05-13) - PLID 65981 - Updated warning
			if (MessageBox(str, NULL, MB_YESNO) != IDYES) {
				return;
			}

			CWaitCursor wait;

			// Get the current provider commissions for auditing
			CMap<CString, LPCTSTR, CommissionInfo, CommissionInfo> mapProvCommissionInfo;
			_RecordsetPtr prs = CreateRecordset("SELECT * FROM CommissionT WHERE ProvID IN (%s) AND ServiceID IN (%s) ORDER BY ProvID, ServiceID",
				GetIDStringFromArray(aryProviders), GetIDStringFromArray(aryServiceItems));
			while (!prs->eof) {
				// Get the records info
				long nProvID = AdoFldLong(prs, "ProvID");
				long nServiceID = AdoFldLong(prs, "ServiceID");
				double dblPercentage = AdoFldDouble(prs, "Percentage", NULL); // (b.eyers 2015-05-13) - PLID 65981
				COleCurrency cyAmount = AdoFldCurrency(prs, "Amount", COleCurrency(0, 0)); // (b.eyers 2015-05-13) - PLID 65981

				// Add the service item's info
				CommissionInfo ciCommissionInfo;
				ciCommissionInfo.dblPercent = dblPercentage;
				ciCommissionInfo.cyAmount = cyAmount; // (b.eyers 2015-05-13) - PLID 65981

				// Create the key
				CString strKey;
				strKey.Format("%li-%li", nProvID, nServiceID);
				mapProvCommissionInfo[strKey] = ciCommissionInfo;

				prs->MoveNext();
			}

			// Begin the SQL statment
			CString strSqlStatement, strSql = BeginSqlBatch();

			// Go through all of the providers that need updated
			for (int p = 0; p < aryProviders.GetSize(); p++) {
				// Detemine what needs to be done and audit it
				CArray<long, long> aryUpdate, aryInsert, aryChange; // (b.eyers 2015-05-13) - PLID 65981 - aryDelete is gone, aryChange is for $ going to %
				for (int s = 0; s < aryServiceItems.GetSize(); s++) {
					CString strAuditNewStr = "", strAuditOldStr = "";
					// Is the provider and service ID combination in the map?
					CommissionInfo ciCommissionInfo;
					CString strKey;
					strKey.Format("%li-%li", aryProviders.GetAt(p).nID, aryServiceItems.GetAt(s).nID);
					if (mapProvCommissionInfo.Lookup(strKey, ciCommissionInfo)) {
						// The provide already has a commission for this item

						if (dblPercent != 0) {
							// This is an update record
							// Make sure the commission really changed
							if (dblPercent != ciCommissionInfo.dblPercent && ciCommissionInfo.dblPercent != 0) { // (b.eyers 2015-05-13) - PLID 65981, this use to be a %
								aryUpdate.Add(aryServiceItems.GetAt(s).nID);
								strAuditOldStr.Format("Item: \'%s\'; Commission: %.2f%%", aryServiceItems.GetAt(s).strName, ciCommissionInfo.dblPercent);
								strAuditNewStr.Format("%.2f%%", dblPercent);
								// Audit the change
								if (nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
								AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionPercentage, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
							}
							else {
								// (b.eyers 2015-05-13) - PLID 65981- this use to be a $ amount but now its going to be %
								aryChange.Add(aryServiceItems.GetAt(s).nID);
								strAuditOldStr.Format("Item: \'%s\'; Commission: %s", aryServiceItems.GetAt(s).strName, FormatCurrencyForInterface(ciCommissionInfo.cyAmount));
								strAuditNewStr.Format("{none}");
								if (nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionAmount, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

								strAuditOldStr.Format("Item: \'%s\'; Commission: {none}", aryServiceItems.GetAt(s).strName, ciCommissionInfo.dblPercent);
								strAuditNewStr.Format("%.2f%%", dblPercent);
								if (nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionPercentage, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
							}
						}
						//else { // (b.eyers 2015-05-13) - PLID 65981 - this never runs, you get a warning if trying to proceed with a 0 in the field
						//	// This is a delete record
						//	aryDelete.Add(aryServiceItems.GetAt(s).nID);
						//	strAuditOldStr.Format("Item: \'%s\'; Commission: %.2f%%", aryServiceItems.GetAt(s).strName, ciCommissionInfo.dblPercent);
						//	strAuditNewStr.Format("{none}");
						//	// Audit the change
						//	if (nAuditTransactionID == -1) {
						//		nAuditTransactionID = BeginAuditTransaction();
						//	}
						//	// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
						//	AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionPercentage, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
						//}

					}
					else {
						// The provider does not yet have a commission for this item
						if (dblPercent != 0) {
							// This is an insert record
							aryInsert.Add(aryServiceItems.GetAt(s).nID);
							strAuditOldStr.Format("Item: \'%s\'; Commission: {none}", aryServiceItems.GetAt(s).strName);
							strAuditNewStr.Format("%.2f%%", dblPercent);
							// Audit the change
							if (nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
							AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionPercentage, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
						}
					}
				}

				// Now do what has to be done for this provider
				if (aryUpdate.GetSize() > 0) {
					strSqlStatement.Format("UPDATE CommissionT SET Percentage = Round(%f, 2) "
						"WHERE ProvID = %li AND ServiceID IN (", dblPercent, aryProviders.GetAt(p).nID);

					// Add the service IDs to the sql statment
					for (int u = 0; u < aryUpdate.GetSize(); u++) {
						strSqlStatement += AsString(aryUpdate.GetAt(u));

						// Limit each statement to have at most 100 IDs in it
						if (((u + 1) % 100) == 0) {
							// Let's finish off this statement and start a new one
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							// Start a new statement
							strSqlStatement.Format("UPDATE CommissionT SET Percentage = Round(%f, 2) "
								"WHERE ProvID = %li AND ServiceID IN (", dblPercent, aryProviders.GetAt(p).nID);
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}

				// (b.eyers 2015-05-13) - PLID 65981 - $ already existed but being changed to %
				if (aryChange.GetSize() > 0) {
					strSqlStatement.Format("UPDATE CommissionT SET Percentage = Round(%f, 2), Amount = NULL "
						"WHERE ProvID = %li AND ServiceID IN (", dblPercent, aryProviders.GetAt(p).nID);

					for (int u = 0; u < aryChange.GetSize(); u++) {
						strSqlStatement += AsString(aryChange.GetAt(u));

						if (((u + 1) % 100) == 0) {
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							strSqlStatement.Format("UPDATE CommissionT SET Percentage = Round(%f, 2), Amount = NULL "
								"WHERE ProvID = %li AND ServiceID IN (", dblPercent, aryProviders.GetAt(p).nID);
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}

				// (b.eyers 2015-05-13) - PLID 65981 - delete never runs from here
				//if (aryDelete.GetSize() > 0) {
				//	strSqlStatement.Format("DELETE FROM CommissionT WHERE ProvID = %li AND ServiceID IN (", aryProviders.GetAt(p).nID);

				//	// Add the service IDs to the sql statment
				//	for (int d = 0; d < aryDelete.GetSize(); d++) {
				//		strSqlStatement += AsString(aryDelete.GetAt(d));

				//		// Limit each statement to have at most 100 IDs in it
				//		if (((d + 1) % 100) == 0) {
				//			// Let's finish off this statement and start a new one
				//			strSqlStatement += ")";
				//			AddStatementToSqlBatch(strSql, strSqlStatement);

				//			// Start a new statement
				//			strSqlStatement.Format("DELETE FROM CommissionT WHERE ProvID = %li AND ServiceID IN (", aryProviders.GetAt(p).nID);
				//		}
				//		else {
				//			strSqlStatement += ",";
				//		}
				//	}
				//	strSqlStatement.TrimRight(",");
				//	strSqlStatement += ")";

				//	AddStatementToSqlBatch(strSql, strSqlStatement);
				//}

				if (aryInsert.GetSize() > 0) {
					strSqlStatement.Format("INSERT INTO CommissionT (ProvID, ServiceID, Percentage) SELECT "
						"%li, ID, Round(%f, 2) FROM ServiceT WHERE ID IN (", aryProviders.GetAt(p).nID, dblPercent);

					// Add the service IDs to the sql statment
					for (int i = 0; i < aryInsert.GetSize(); i++) {
						strSqlStatement += AsString(aryInsert.GetAt(i));

						// Limit each statement to have at most 100 IDs in it
						if (((i + 1) % 100) == 0) {
							// Let's finish off this statement and start a new one
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							// Start a new statement
							strSqlStatement.Format("INSERT INTO CommissionT (ProvID, ServiceID, Percentage) SELECT "
								"%li, ID, Round(%f, 2) FROM ServiceT WHERE ID IN (", aryProviders.GetAt(p).nID, dblPercent);
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}
			}

			// Execute the sql statements
			if (!strSql.IsEmpty()) {
				ExecuteSqlBatch(strSql);
			}

			// Commit auditing
			if (nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
			}

			MessageBox("Service items successfully updated!", NULL, MB_OK);
		}
		else { // (b.eyers 2015-05-13) - PLID 65981 - save the amount applied
			CString strAmount;
			COleCurrency cyTmp;
			GetDlgItemText(IDC_APPLY_AMOUNT, strAmount);

			if (strAmount.IsEmpty()) {
				MsgBox("You must enter an amount before continuing.");
				return;
			}

			cyTmp = ParseCurrencyFromInterface(strAmount);

			if (cyTmp.GetStatus() == COleCurrency::invalid) {
				AfxMessageBox("Please enter a valid amount greater than $0.00.");
				return;
			}

			if (cyTmp <= COleCurrency(0, 0)) {
				AfxMessageBox("Please enter a valid amount greater than $0.00.");
				return;
			}

			if (cyTmp > COleCurrency(100000000, 0)) {
				CString str;
				str.Format("Practice does not allow an amount greater than %s.", FormatCurrencyForInterface(COleCurrency(100000000, 0), TRUE, TRUE));
				AfxMessageBox(str);
				return;
			}

			CString str;
			str.Format("Are you sure you wish to update all the selected service items for all the selected providers to %s? \n"
				"Note, items with a percentage will be cleared and replaced with the dollar amount if you continue.", FormatCurrencyForInterface(cyTmp)); 
			if (MessageBox(str, NULL, MB_YESNO) != IDYES) {
				return;
			}

			CWaitCursor wait;

			CMap<CString, LPCTSTR, CommissionInfo, CommissionInfo> mapProvCommissionInfo;
			_RecordsetPtr prs = CreateRecordset("SELECT * FROM CommissionT WHERE ProvID IN (%s) AND ServiceID IN (%s) ORDER BY ProvID, ServiceID",
				GetIDStringFromArray(aryProviders), GetIDStringFromArray(aryServiceItems));
			while (!prs->eof) {
				long nProvID = AdoFldLong(prs, "ProvID");
				long nServiceID = AdoFldLong(prs, "ServiceID");
				double dblPercentage = AdoFldDouble(prs, "Percentage", NULL); 
				COleCurrency cyAmount = AdoFldCurrency(prs, "Amount", COleCurrency(0, 0)); 

				CommissionInfo ciCommissionInfo;
				ciCommissionInfo.dblPercent = dblPercentage;
				ciCommissionInfo.cyAmount = cyAmount; 

				CString strKey;
				strKey.Format("%li-%li", nProvID, nServiceID);
				mapProvCommissionInfo[strKey] = ciCommissionInfo;

				prs->MoveNext();
			}

			CString strSqlStatement, strSql = BeginSqlBatch();

			for (int p = 0; p < aryProviders.GetSize(); p++) {
				CArray<long, long> aryUpdate, aryInsert, aryChange;
				for (int s = 0; s < aryServiceItems.GetSize(); s++) {
					CString strAuditNewStr = "", strAuditOldStr = "";
					CommissionInfo ciCommissionInfo;
					CString strKey;
					strKey.Format("%li-%li", aryProviders.GetAt(p).nID, aryServiceItems.GetAt(s).nID);
					if (mapProvCommissionInfo.Lookup(strKey, ciCommissionInfo)) {

						if (cyTmp != COleCurrency(0,0)) {
							if (cyTmp != ciCommissionInfo.cyAmount && ciCommissionInfo.cyAmount != COleCurrency(0,0)) { //use to be a $
								aryUpdate.Add(aryServiceItems.GetAt(s).nID);
								strAuditOldStr.Format("Item: \'%s\'; Commission: %s", aryServiceItems.GetAt(s).strName, FormatCurrencyForInterface(ciCommissionInfo.cyAmount));
								strAuditNewStr.Format("%s", FormatCurrencyForInterface(cyTmp));
								if (nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionAmount, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
							}
							else {
								//this use to be a % amount but now its going to be $
								aryChange.Add(aryServiceItems.GetAt(s).nID);
								strAuditOldStr.Format("Item: \'%s\'; Commission: %.2f%%", aryServiceItems.GetAt(s).strName, ciCommissionInfo.dblPercent);
								strAuditNewStr.Format("{none}");
								if (nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionPercentage, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

								strAuditOldStr.Format("Item: \'%s\'; Commission: {none}", aryServiceItems.GetAt(s).strName, ciCommissionInfo.dblPercent);
								strAuditNewStr.Format("%s", FormatCurrencyForInterface(cyTmp));
								if (nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionAmount, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
							}
						}
					}
					else {
						if (cyTmp != COleCurrency(0,0)) {
							aryInsert.Add(aryServiceItems.GetAt(s).nID);
							strAuditOldStr.Format("Item: \'%s\'; Commission: {none}", aryServiceItems.GetAt(s).strName);
							strAuditNewStr.Format("%s", FormatCurrencyForInterface(cyTmp));
							if (nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionAmount, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
						}
					}
				}

				if (aryUpdate.GetSize() > 0) {
					strSqlStatement.Format("UPDATE CommissionT SET Amount = CONVERT(money, '%s') "
						"WHERE ProvID = %li AND ServiceID IN (", FormatCurrencyForSql(cyTmp), aryProviders.GetAt(p).nID);

					for (int u = 0; u < aryUpdate.GetSize(); u++) {
						strSqlStatement += AsString(aryUpdate.GetAt(u));

						if (((u + 1) % 100) == 0) {
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							strSqlStatement.Format("UPDATE CommissionT SET Amount = CONVERT(money, '%s') "
								"WHERE ProvID = %li AND ServiceID IN (", FormatCurrencyForSql(cyTmp), aryProviders.GetAt(p).nID);
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}

				if (aryChange.GetSize() > 0) {
					strSqlStatement.Format("UPDATE CommissionT SET Percentage = NULL, Amount = CONVERT(money, '%s') "
						"WHERE ProvID = %li AND ServiceID IN (", FormatCurrencyForSql(cyTmp), aryProviders.GetAt(p).nID);

					for (int u = 0; u < aryChange.GetSize(); u++) {
						strSqlStatement += AsString(aryChange.GetAt(u));

						if (((u + 1) % 100) == 0) {
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							strSqlStatement.Format("UPDATE CommissionT SET Percentage = NULL, Amount = CONVERT(money, '%s') "
								"WHERE ProvID = %li AND ServiceID IN (", FormatCurrencyForSql(cyTmp), aryProviders.GetAt(p).nID);
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}

				if (aryInsert.GetSize() > 0) {
					strSqlStatement.Format("INSERT INTO CommissionT (ProvID, ServiceID, Amount) SELECT "
						"%li, ID, CONVERT(money, '%s') FROM ServiceT WHERE ID IN (", aryProviders.GetAt(p).nID, FormatCurrencyForSql(cyTmp));

					for (int i = 0; i < aryInsert.GetSize(); i++) {
						strSqlStatement += AsString(aryInsert.GetAt(i));

						if (((i + 1) % 100) == 0) {
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							strSqlStatement.Format("INSERT INTO CommissionT (ProvID, ServiceID, Amount) SELECT "
								"%li, ID, CONVERT(money, '%s') FROM ServiceT WHERE ID IN (", aryProviders.GetAt(p).nID, FormatCurrencyForSql(cyTmp));
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}
			}

			if (!strSql.IsEmpty()) {
				ExecuteSqlBatch(strSql);
			}

			if (nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
			}

			MessageBox("Service items successfully updated!", NULL, MB_OK);

		}

	}NxCatchAllCall("Error in CAdvCommissionSetupDlg::OnApplySelected", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););
}

void CAdvCommissionSetupDlg::OnClearSelected() 
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the provider and service item IDs
		CArray<IDNameInfo,IDNameInfo> aryProviders, aryServiceItems;
		if (!GetSelProvidersAndServiceItems(aryProviders, aryServiceItems)) {
			// Either no provider or no service item was selected
			return;
		}

		// Confirm that they want to update the commissions
		CString str;
		str.Format("Are you sure you wish to remove the commission from all of the selected service items for all the selected providers?");
		if (MessageBox(str, NULL, MB_YESNO) != IDYES) {
			return;
		}

		CWaitCursor wait;

		// Get the current provider commissions for auditing
		CMap<CString, LPCTSTR, CommissionInfo, CommissionInfo> mapProvCommissionInfo;
		_RecordsetPtr prs = CreateRecordset("SELECT * FROM CommissionT WHERE ProvID IN (%s) AND ServiceID IN (%s) ORDER BY ProvID, ServiceID",
										GetIDStringFromArray(aryProviders), GetIDStringFromArray(aryServiceItems));
		while (!prs->eof) {
			// Get the records info
			long nProvID = AdoFldLong(prs, "ProvID");
			long nServiceID = AdoFldLong(prs, "ServiceID");
			double dblPercentage = AdoFldDouble(prs, "Percentage", NULL); // (b.eyers 2015-05-13) - PLID 65981
			COleCurrency cyAmount = AdoFldCurrency(prs, "Amount", COleCurrency(0,0)); // (b.eyers 2015-05-13) - PLID 65981

			// Add the service item's info
			CommissionInfo ciCommissionInfo;
			ciCommissionInfo.dblPercent = dblPercentage;
			ciCommissionInfo.cyAmount = cyAmount;

			// Create the key
			CString strKey;
			strKey.Format("%li-%li", nProvID, nServiceID);
			mapProvCommissionInfo[strKey] = ciCommissionInfo;

			prs->MoveNext();
		}

		// Begin the SQL statment
		CString strSqlStatement, strSql = BeginSqlBatch();

		// Go through all of the providers that need updated
		for (int p = 0; p < aryProviders.GetSize(); p++) {
			// Detemine what needs to be done and audit it
			CArray<long, long> aryDelete;
			for(int s = 0; s < aryServiceItems.GetSize(); s++) {
				CString strAuditNewStr = "", strAuditOldStr = "";
				// Is the provider and service ID combination in the map?
				CommissionInfo ciCommissionInfo;
				CString strKey;
				strKey.Format("%li-%li", aryProviders.GetAt(p).nID, aryServiceItems.GetAt(s).nID);
				if (mapProvCommissionInfo.Lookup(strKey, ciCommissionInfo)) {
					// The provide already has a commission for this item
					// This is a delete record
					
					if (ciCommissionInfo.dblPercent != 0) { // (b.eyers 2015-05-13) - PLID 65981 - use to have a %, audit that

						aryDelete.Add(aryServiceItems.GetAt(s).nID);
						strAuditOldStr.Format("Item: \'%s\'; Commission: %.2f%%", aryServiceItems.GetAt(s).strName, ciCommissionInfo.dblPercent);
						strAuditNewStr.Format("{none}");
						// Audit the change
						if (nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
						AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionPercentage, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);

					}
					else { // (b.eyers 2015-05-13) - PLID 65981 - use to have a $, audit that
						aryDelete.Add(aryServiceItems.GetAt(s).nID);
						strAuditOldStr.Format("Item: \'%s\'; Commission: %s", aryServiceItems.GetAt(s).strName, FormatCurrencyForInterface(ciCommissionInfo.cyAmount));
						strAuditNewStr.Format("{none}");

						if (nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}

						AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionAmount, aryServiceItems.GetAt(s).nID, strAuditOldStr, strAuditNewStr, aepMedium, aetChanged);
					}
				}
			}
			
			// Now delete the commissions that need deleted
			if (aryDelete.GetSize() > 0) {
				strSqlStatement.Format("DELETE FROM CommissionT WHERE ProvID = %li AND ServiceID IN (", aryProviders.GetAt(p).nID); 

				// Add the service IDs to the sql statment
				for(int d = 0; d < aryDelete.GetSize(); d++) {
					strSqlStatement += AsString(aryDelete.GetAt(d));
					
					// Limit each statement to have at most 100 IDs in it
					if (((d+1)%100) == 0) {
						// Let's finish off this statement and start a new one
						strSqlStatement += ")";
						AddStatementToSqlBatch(strSql, strSqlStatement);

						// Start a new statement
						strSqlStatement.Format("DELETE FROM CommissionT WHERE ProvID = %li AND ServiceID IN (", aryProviders.GetAt(p).nID); 
					}
					else {
						strSqlStatement += ",";
					}
				}
				strSqlStatement.TrimRight(",");
				strSqlStatement += ")";

				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
		}

		// Execute the sql statements
		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

		// Commit auditing
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		MessageBox("Service items successfully updated!", NULL, MB_OK);

	}NxCatchAllCall("Error in CAdvCommissionSetupDlg::OnClearSelected", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););	
}

void CAdvCommissionSetupDlg::OnApplySelectedRule() 
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the provider and service item IDs
		CArray<IDNameInfo,IDNameInfo> aryProviders, aryServiceItems;
		if (!GetSelProvidersAndServiceItems(aryProviders, aryServiceItems)) {
			// Either no provider or no service item was selected
			return;
		}

		// Get the selected rule
		long nRuleID = -1;
		CString strRuleName = "";
		NXDATALIST2Lib::IRowSettingsPtr pRuleRow = m_pAddRuleCombo->GetCurSel();
		if (pRuleRow) {
			nRuleID = VarLong(pRuleRow->GetValue(0));
			strRuleName = VarString(pRuleRow->GetValue(1));
		}
		else {
			MessageBox("You must select a rule before continuing.", NULL, MB_OK|MB_ICONWARNING);
			return;
		}

		// Confirm that they want to update the rules
		CString str;
		str.Format("Are you sure you wish to add the rule '%s' to all the selected service items for all the selected providers?", strRuleName);
		if (MessageBox(str, NULL, MB_YESNO) != IDYES) {
			return;
		}

		CWaitCursor wait;

		// Get the current provider commissions for auditing
		CMap<CString, LPCTSTR, CommissionInfo, CommissionInfo> mapProvCommissionInfo;
		_RecordsetPtr prs = CreateRecordset("SELECT * FROM CommissionRulesLinkT LEFT JOIN CommissionRulesT ON CommissionRulesT.ID = CommissionRulesLinkT.RuleID "
										"WHERE ProvID IN (%s) AND ServiceID IN (%s) ORDER BY ProvID, ServiceID",
										GetIDStringFromArray(aryProviders), GetIDStringFromArray(aryServiceItems));
		while (!prs->eof) {
			// Get the records info
			long nProvID = AdoFldLong(prs, "ProvID");
			long nServiceID = AdoFldLong(prs, "ServiceID");
			long nItemRuleID = AdoFldLong(prs, "RuleID");
			CString strRuleName = AdoFldString(prs, "Name");

			// Add the service item's info
			CommissionInfo ciCommissionInfo;
			ciCommissionInfo.nRuleID = nItemRuleID;
			ciCommissionInfo.strRuleName = strRuleName;

			// Create the key
			CString strKey;
			strKey.Format("%li-%li-%li", nProvID, nServiceID, nItemRuleID);
			mapProvCommissionInfo[strKey] = ciCommissionInfo;

			prs->MoveNext();
		}

		// Begin the SQL statment
		CString strSqlStatement, strSql = BeginSqlBatch();

		// Go through all of the providers that need updated
		for (int p = 0; p < aryProviders.GetSize(); p++) {
			// Detemine what needs to be done and audit it
			CArray<long, long> aryInsert;
			for(int s = 0; s < aryServiceItems.GetSize(); s++) {
				CString strAuditStr = "";
				// Is the provider, service, and rule ID combination in the map?
				CommissionInfo ciCommissionInfo;
				CString strKey;
				strKey.Format("%li-%li-%li", aryProviders.GetAt(p).nID, aryServiceItems.GetAt(s).nID, nRuleID);
				if (mapProvCommissionInfo.Lookup(strKey, ciCommissionInfo)) {
					// The provider already has this rule associated with this service item
					// Do nothing because it's already there
				}
				else {
					// The provider does not yet have this rule associated with this service item
					// This is an insert record
					aryInsert.Add(aryServiceItems.GetAt(s).nID);
					strAuditStr.Format("Item: \'%s\'; Rule: \'%s\'", aryServiceItems.GetAt(s).strName, strRuleName);
					// Audit the change
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
					AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionRuleAdded, aryServiceItems.GetAt(s).nID, "", strAuditStr, aepMedium, aetChanged);
				}
			}

			// Now add the rule to the service items that need it added to
			if (aryInsert.GetSize() > 0) {
				strSqlStatement.Format("INSERT INTO CommissionRulesLinkT (RuleID, ProvID, ServiceID) "
								"SELECT %li, %li, ID FROM ServiceT WHERE ID IN (", nRuleID, aryProviders.GetAt(p).nID); 

				// Add the service IDs to the sql statment
				for(int i = 0; i < aryInsert.GetSize(); i++) {
					strSqlStatement += AsString(aryInsert.GetAt(i));
					
					// Limit each statement to have at most 100 IDs in it
					if (((i+1)%100) == 0) {
						// Let's finish off this statement and start a new one
						strSqlStatement += ")";
						AddStatementToSqlBatch(strSql, strSqlStatement);

						// Start a new statement
						strSqlStatement.Format("INSERT INTO CommissionRulesLinkT (RuleID, ProvID, ServiceID) "
								"SELECT %li, %li, ID FROM ServiceT WHERE ID IN (", nRuleID, aryProviders.GetAt(p).nID); 
					}
					else {
						strSqlStatement += ",";
					}
				}
				strSqlStatement.TrimRight(",");
				strSqlStatement += ")";

				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
		}

		// Execute the sql statements
		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

		// Commit auditing
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		MessageBox("Service items successfully updated!", NULL, MB_OK);

	}NxCatchAllCall("Error in CAdvCommissionSetupDlg::OnApplySelectedRule", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););	
}

void CAdvCommissionSetupDlg::OnRemoveSelectedRule() 
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the provider and service item IDs
		CArray<IDNameInfo,IDNameInfo> aryProviders, aryServiceItems;
		if (!GetSelProvidersAndServiceItems(aryProviders, aryServiceItems)) {
			// Either no provider or no service item was selected
			return;
		}

		// Get the selected rule
		long nRuleID = -1;
		CString strRuleName = "";
		NXDATALIST2Lib::IRowSettingsPtr pRuleRow = m_pRemoveRuleCombo->GetCurSel();
		if (pRuleRow) {
			nRuleID = VarLong(pRuleRow->GetValue(0));
			strRuleName = VarString(pRuleRow->GetValue(1));
		}
		else {
			MessageBox("You must select a rule before continuing.", NULL, MB_OK|MB_ICONWARNING);
			return;
		}

		// Confirm that they want to update the rules
		CString str;
		str.Format("Are you sure you wish to remove the rule '%s' from all the selected service items for all the selected providers?", strRuleName);
		if (MessageBox(str, NULL, MB_YESNO) != IDYES) {
			return;
		}

		CWaitCursor wait;

		// Get the current provider commissions for auditing
		CMap<CString, LPCTSTR, CommissionInfo, CommissionInfo> mapProvCommissionInfo;
		_RecordsetPtr prs = CreateRecordset("SELECT * FROM CommissionRulesLinkT LEFT JOIN CommissionRulesT ON CommissionRulesT.ID = CommissionRulesLinkT.RuleID "
										"WHERE ProvID IN (%s) AND ServiceID IN (%s) ORDER BY ProvID, ServiceID",
										GetIDStringFromArray(aryProviders), GetIDStringFromArray(aryServiceItems));
		while (!prs->eof) {
			// Get the records info
			long nProvID = AdoFldLong(prs, "ProvID");
			long nServiceID = AdoFldLong(prs, "ServiceID");
			long nItemRuleID = AdoFldLong(prs, "RuleID");
			CString strRuleName = AdoFldString(prs, "Name");

			// Add the service item's info
			CommissionInfo ciCommissionInfo;
			ciCommissionInfo.nRuleID = nItemRuleID;
			ciCommissionInfo.strRuleName = strRuleName;

			// Create the key
			CString strKey;
			strKey.Format("%li-%li-%li", nProvID, nServiceID, nItemRuleID);
			mapProvCommissionInfo[strKey] = ciCommissionInfo;

			prs->MoveNext();
		}

		// Begin the SQL statment
		CString strSqlStatement, strSql = BeginSqlBatch();

		// Go through all of the providers that need updated
		for (int p = 0; p < aryProviders.GetSize(); p++) {
			// Detemine what needs to be done and audit it
			CArray<long, long> aryRemove;
			for(int s = 0; s < aryServiceItems.GetSize(); s++) {
				CString strAuditStr = "";
				// Is the provider, service, and rule ID combination in the map?
				CommissionInfo ciCommissionInfo;
				CString strKey;
				strKey.Format("%li-%li-%li", aryProviders.GetAt(p).nID, aryServiceItems.GetAt(s).nID, nRuleID);
				if (mapProvCommissionInfo.Lookup(strKey, ciCommissionInfo)) {
					// The provider has this rule associated with this service item, we need to remove it
					aryRemove.Add(aryServiceItems.GetAt(s).nID);
					strAuditStr.Format("Item: \'%s\'; Rule: \'%s\'", aryServiceItems.GetAt(s).strName, ciCommissionInfo.strRuleName);
					// Audit the change
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
					AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionRuleRemoved, aryServiceItems.GetAt(s).nID, strAuditStr, "", aepMedium, aetChanged);
				}
				else {
					// The provider does not have this rule associated with this service item, so do nothing
				}
			}

			// Now add the rule to the service items that need it added to
			if (aryRemove.GetSize() > 0) {
				strSqlStatement.Format("DELETE FROM CommissionRulesLinkT WHERE RuleID = %li AND ProvID = %li AND ServiceID IN (", nRuleID, aryProviders.GetAt(p).nID); 

				// Add the service IDs to the sql statment
				for(int i = 0; i < aryRemove.GetSize(); i++) {
					strSqlStatement += AsString(aryRemove.GetAt(i));
					
					// Limit each statement to have at most 100 IDs in it
					if (((i+1)%100) == 0) {
						// Let's finish off this statement and start a new one
						strSqlStatement += ")";
						AddStatementToSqlBatch(strSql, strSqlStatement);

						// Start a new statement
						strSqlStatement.Format("DELETE FROM CommissionRulesLinkT WHERE RuleID = %li AND ProvID = %li AND ServiceID IN (", nRuleID, aryProviders.GetAt(p).nID);			
					}
					else {
						strSqlStatement += ",";
					}
				}
				strSqlStatement.TrimRight(",");
				strSqlStatement += ")";

				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
		}

		// Execute the sql statements
		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

		// Commit auditing
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		MessageBox("Service items successfully updated!", NULL, MB_OK);

	}NxCatchAllCall("Error in CAdvCommissionSetupDlg::OnRemoveSelectedRule", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););	
}

void CAdvCommissionSetupDlg::OnClearSelectedRules() 
{
	//Batch the AuditTransactions
	long nAuditTransactionID = -1;

	try {
		// (a.wetta 2007-05-17 15:59) - PLID 25394 - Make sure they have permission to make changes
		if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite))
			return;

		// Get the provider and service item IDs
		CArray<IDNameInfo,IDNameInfo> aryProviders, aryServiceItems;
		if (!GetSelProvidersAndServiceItems(aryProviders, aryServiceItems)) {
			// Either no provider or no service item was selected
			return;
		}

		// Confirm that they want to update the rules
		CString str;
		str.Format("Are you sure you wish to remove all rules from all the selected service items for all the selected providers?");
		if (MessageBox(str, NULL, MB_YESNO) != IDYES) {
			return;
		}

		CWaitCursor wait;

		// Get the current provider commissions for auditing
		CMap<CString, LPCTSTR, CommissionInfo, CommissionInfo> mapProvCommissionInfo;
		_RecordsetPtr prs = CreateRecordset("SELECT * FROM CommissionRulesLinkT LEFT JOIN CommissionRulesT ON CommissionRulesT.ID = CommissionRulesLinkT.RuleID "
										"WHERE ProvID IN (%s) AND ServiceID IN (%s) ORDER BY ProvID, ServiceID",
										GetIDStringFromArray(aryProviders), GetIDStringFromArray(aryServiceItems));
		while (!prs->eof) {
			// Get the record's info
			long nProvID = AdoFldLong(prs, "ProvID");
			long nServiceID = AdoFldLong(prs, "ServiceID");
			long nItemRuleID = AdoFldLong(prs, "RuleID");
			CString strRuleName = AdoFldString(prs, "Name");

			// Add the service item's info
			CommissionInfo ciCommissionInfo;
			ciCommissionInfo.nRuleID = nItemRuleID;
			ciCommissionInfo.strRuleName = strRuleName;

			// Create the key
			CString strKey;
			strKey.Format("%li-%li-%li", nProvID, nServiceID, nItemRuleID);
			mapProvCommissionInfo[strKey] = ciCommissionInfo;

			prs->MoveNext();
		}

		// Get all of the rules
		CArray<CommissionInfo, CommissionInfo> aryCommissionRules;
		prs = CreateRecordset("SELECT ID, Name FROM CommissionRulesT");
		while (!prs->eof) {
			// Get the record's info
			long nRuleID = AdoFldLong(prs, "ID");
			CString strRuleName = AdoFldString(prs, "Name");

			// Add the rule info
			CommissionInfo ciCommissionInfo;
			ciCommissionInfo.nRuleID = nRuleID;
			ciCommissionInfo.strRuleName = strRuleName;

			aryCommissionRules.Add(ciCommissionInfo);

			prs->MoveNext();
		}

		// Begin the SQL statment
		CString strSqlStatement, strSql = BeginSqlBatch();

		// Go through all of the providers that need updated
		for (int p = 0; p < aryProviders.GetSize(); p++) {
			// Got through all of the rules 
			for (int r = 0; r < aryCommissionRules.GetSize(); r++) {
				// Detemine what needs to be done and audit it
				CArray<long, long> aryRemove;
				for (int s = 0; s < aryServiceItems.GetSize(); s++) {
					CString strAuditStr = "";
					// Is the provider, service, and rule ID combination in the map?
					CommissionInfo ciCommissionInfo;
					CString strKey;
					strKey.Format("%li-%li-%li", aryProviders.GetAt(p).nID, aryServiceItems.GetAt(s).nID, aryCommissionRules.GetAt(r).nRuleID);
					if (mapProvCommissionInfo.Lookup(strKey, ciCommissionInfo)) {
						// The provider has this rule associated with this service item, we need to remove it
						aryRemove.Add(aryServiceItems.GetAt(s).nID);
						strAuditStr.Format("Item: \'%s\'; Rule: \'%s\'", aryServiceItems.GetAt(s).strName, ciCommissionInfo.strRuleName);
						// Audit the change
						if (nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						// (j.jones 2009-11-19 11:01) - PLID 29046 - this is for "basic" commissions
						AuditEvent(-1, aryProviders.GetAt(p).strName, nAuditTransactionID, aeiProvBasicCommissionRuleRemoved, aryServiceItems.GetAt(s).nID, strAuditStr, "", aepMedium, aetChanged);
					}
					else {
						// The provider does not have this rule associated with this service item, so do nothing
					}
				}

				// Now add the rule to the service items that need it added to
				if (aryRemove.GetSize() > 0) {
					strSqlStatement.Format("DELETE FROM CommissionRulesLinkT WHERE RuleID = %li AND ProvID = %li AND ServiceID IN (", aryCommissionRules.GetAt(r).nRuleID, aryProviders.GetAt(p).nID); 

					// Add the service IDs to the sql statment
					for(int i = 0; i < aryRemove.GetSize(); i++) {
						strSqlStatement += AsString(aryRemove.GetAt(i));
						
						// Limit each statement to have at most 100 IDs in it
						if (((i+1)%100) == 0) {
							// Let's finish off this statement and start a new one
							strSqlStatement += ")";
							AddStatementToSqlBatch(strSql, strSqlStatement);

							// Start a new statement
							strSqlStatement.Format("DELETE FROM CommissionRulesLinkT WHERE RuleID = %li AND ProvID = %li AND ServiceID IN (", aryCommissionRules.GetAt(r).nRuleID, aryProviders.GetAt(p).nID);			
						}
						else {
							strSqlStatement += ",";
						}
					}
					strSqlStatement.TrimRight(",");
					strSqlStatement += ")";

					AddStatementToSqlBatch(strSql, strSqlStatement);
				}
			}
		}

		// Execute the sql statements
		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

		// Commit auditing
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		MessageBox("Service items successfully updated!", NULL, MB_OK);

	}NxCatchAllCall("Error in CAdvCommissionSetupDlg::OnClearSelectedRules", if (nAuditTransactionID != -1) RollbackAuditTransaction(nAuditTransactionID););		
}

void CAdvCommissionSetupDlg::OnShowInactiveProv() 
{
	try {
		// Refresh the provider list accordingly
		if (!IsDlgButtonChecked(IDC_SHOW_INACTIVE_PROV)) {
			m_pAvailProviders->PutWhereClause(_bstr_t("Archived = 0"));
		}
		else {
			m_pAvailProviders->PutWhereClause(_bstr_t(""));
		}
		m_pAvailProviders->Requery();
		m_pSelProviders->Clear();

	}NxCatchAll("Error in CAdvCommissionSetupDlg::OnShowInactiveProv");	
}

void CAdvCommissionSetupDlg::OnShowOutDatedRules() 
{
	try {

		// (j.jones 2009-11-19 11:03) - PLID 29046 - this dialog does not use tiered rules

		// Refresh the rule combos accordingly
		if (!IsDlgButtonChecked(IDC_SHOW_OUT_DATED_RULES)) {
			m_pAddRuleCombo->PutWhereClause(_bstr_t("EndDate >= dbo.AsDateNoTime(GetDate()) AND IsTieredCommission = 0"));
			m_pRemoveRuleCombo->PutWhereClause(_bstr_t("EndDate >= dbo.AsDateNoTime(GetDate()) AND IsTieredCommission = 0"));
		}
		else {
			m_pAddRuleCombo->PutWhereClause(_bstr_t("IsTieredCommission = 0"));
			m_pRemoveRuleCombo->PutWhereClause(_bstr_t("IsTieredCommission = 0"));
		}
		m_pAddRuleCombo->Requery();
		m_pRemoveRuleCombo->Requery();

	}NxCatchAll("Error in CAdvCommissionSetupDlg::OnShowOutDatedRules");	
}

void CAdvCommissionSetupDlg::OnKillfocusApplyPercentage() 
{
	try {
		CString strValue;
		GetDlgItemText(IDC_APPLY_PERCENTAGE, strValue);

		// Make sure it is a valid number
		if(strValue.SpanIncluding("1234567890.%").GetLength() != strValue.GetLength() ||
			atof(strValue) > 100.00){
			// It's not a valid number
			MsgBox("Please enter a valid percentage from 1 - 100.");
			CString strPercent;
			strPercent.Format("%.2f%%", m_dblPercent); 
			SetDlgItemText(IDC_APPLY_PERCENTAGE, strPercent);
		}
		else {
			// Round the number
			CString strPercent;
			strPercent.Format("%.2f%%", atof(strValue)); 
			SetDlgItemText(IDC_APPLY_PERCENTAGE, strPercent);
			m_dblPercent = atof(strPercent);
		}
	}NxCatchAll("Error in CAdvCommissionSetupDlg::OnKillfocusApplyPercentage");	
}

// (b.eyers 2015-05-13) - PLID 65981 - Have the %/$ label update  
LRESULT CAdvCommissionSetupDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{

		if (m_bPercent) {
			//currently set to %, change to $
			m_nxlabelApply.SetText("Apply $ commission:");
			m_bPercent = FALSE;
			GetDlgItem(IDC_APPLY_PERCENTAGE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_APPLY_AMOUNT)->ShowWindow(SW_SHOW);
			//reset default text to $0
			CString tmpStr;
			tmpStr = FormatCurrencyForInterface(m_cyAmount);
			SetDlgItemText(IDC_APPLY_AMOUNT, tmpStr);
			//set % back to 0
			m_dblPercent = 0;
		}
		else {
			m_nxlabelApply.SetText("Apply % commission:");
			m_bPercent = TRUE;
			GetDlgItem(IDC_APPLY_PERCENTAGE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_APPLY_AMOUNT)->ShowWindow(SW_HIDE);
			//reset default text to 0
			CString strPercent;
			strPercent.Format("%.2f%%", m_dblPercent);
			SetDlgItemText(IDC_APPLY_PERCENTAGE, strPercent);
			//set $ back to 0 
			m_cyAmount = COleCurrency(0, 0);
		}

	}NxCatchAll("Error in OnLabelClick");
	return 0;
}

// (b.eyers 2015-05-13) - PLID 65981 - warnings to make sure the $ amount is correct
void CAdvCommissionSetupDlg::OnKillfocusApplyAmount()
{
	try {
		CString strValue;
		COleCurrency cyTmp;
		GetDlgItemText(IDC_APPLY_AMOUNT, strValue);
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
			SetDlgItemText(IDC_APPLY_AMOUNT, FormatCurrencyForInterface(m_cyAmount));
		}
		else {
			SetDlgItemText(IDC_APPLY_AMOUNT, FormatCurrencyForInterface(cyTmp));
			m_cyAmount = cyTmp;
		}

	}NxCatchAll("Error in CAdvCommissionSetupDlg::OnKillfocusApplyAmount");
}