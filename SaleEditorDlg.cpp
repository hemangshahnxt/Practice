// SaleEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "SaleEditorDlg.h"
#include <afxtempl.h>
#include "datetimeutils.h"
#include "internationalutils.h"
#include "AuditTrail.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.wetta 2007-05-07 15:11) - PLID 15998 - Created the dialog

/////////////////////////////////////////////////////////////////////////////
// CSaleEditorDlg dialog


CSaleEditorDlg::CSaleEditorDlg(CWnd* pParent)
	: CNxDialog(CSaleEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaleEditorDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nSaleID = -1;
	m_strSaleName = "";
	m_dtStartDate = COleDateTime::GetCurrentTime();
	m_dtEndDate = COleDateTime::GetCurrentTime();
	m_dtCurStartDate = COleDateTime::GetCurrentTime();
	m_dtCurEndDate = COleDateTime::GetCurrentTime();
	m_nDiscountCategory = -1;
	m_bStartDateCalandarOpen = FALSE;
	m_bEndDateCalandarOpen = FALSE;
	m_bChanged = FALSE;
	m_bLoading = FALSE;
	m_strDiscountCategoryName = "";
}


void CSaleEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaleEditorDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_APPLY_QUICK_DISCOUNT, m_btnApplyQuickDiscount);
	DDX_Control(pDX, IDC_SALE_END_DATE, m_ctrlEndDate);
	DDX_Control(pDX, IDC_SALE_START_DATE, m_ctrlStartDate);
	DDX_Control(pDX, IDC_SALE_NAME, m_nxeditSaleName);
	DDX_Control(pDX, IDC_QUICK_DISCOUNT_AMOUNT, m_nxeditQuickDiscountAmount);
	DDX_Control(pDX, IDC_SUPPLIER_SALE_TEXT, m_nxstaticSupplierSaleText);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CSaleEditorDlg, IDC_SALE_START_DATE, 2 /* Change */, OnChangeSaleStartDate, VTS_NONE)
//	ON_EVENT(CSaleEditorDlg, IDC_SALE_END_DATE, 2 /* Change */, OnChangeSaleEndDate, VTS_NONE)
//	ON_EVENT(CSaleEditorDlg, IDC_SALE_START_DATE, 3 /* CloseUp */, OnCloseUpSaleStartDate, VTS_NONE)
//	ON_EVENT(CSaleEditorDlg, IDC_SALE_START_DATE, 4 /* DropDown */, OnDropDownSaleStartDate, VTS_NONE)
//	ON_EVENT(CSaleEditorDlg, IDC_SALE_END_DATE, 3 /* CloseUp */, OnCloseUpSaleEndDate, VTS_NONE)
//	ON_EVENT(CSaleEditorDlg, IDC_SALE_END_DATE, 4 /* DropDown */, OnDropDownSaleEndDate, VTS_NONE)

// (a.walling 2008-05-28 11:35) - PLID 27591 - Use the new notify events

BEGIN_MESSAGE_MAP(CSaleEditorDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSaleEditorDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SALE_START_DATE, OnChangeSaleStartDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_SALE_END_DATE, OnChangeSaleEndDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_SALE_START_DATE, OnDropDownSaleStartDate)
	ON_NOTIFY(DTN_DROPDOWN, IDC_SALE_END_DATE, OnDropDownSaleEndDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_SALE_START_DATE, OnCloseUpSaleStartDate)
	ON_NOTIFY(DTN_CLOSEUP, IDC_SALE_END_DATE, OnCloseUpSaleEndDate)
	ON_BN_CLICKED(IDC_APPLY_QUICK_DISCOUNT, OnApplyQuickDiscount)
	ON_EN_KILLFOCUS(IDC_QUICK_DISCOUNT_AMOUNT, OnKillfocusQuickDiscountAmount)
	ON_EN_CHANGE(IDC_SALE_NAME, OnChangeSaleName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaleEditorDlg message handlers

BOOL CSaleEditorDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_bLoading = TRUE;

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnApplyQuickDiscount.AutoSet(NXB_MODIFY);

		m_ctrlStartDate.SetMinDate(0);
		m_ctrlEndDate.SetMinDate(0);

		// Set the basic sale information
		GetDlgItem(IDC_SALE_NAME)->SetWindowText(m_strSaleName);
		m_ctrlStartDate.SetValue(_variant_t(m_dtStartDate));
		m_ctrlEndDate.SetValue(_variant_t(m_dtEndDate));
		m_dtCurStartDate = m_dtStartDate;
		m_dtCurEndDate = m_dtEndDate;

		// Bind and requery the datalists
		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pDiscountTypeList = BindNxDataList2Ctrl(IDC_SALE_DISCOUNT_TYPE_COMBO, true);
		m_pItemTypeList = BindNxDataList2Ctrl(IDC_SALE_ITEM_TYPE_COMBO, true);
		m_pSupplierList = BindNxDataList2Ctrl(IDC_SALE_SUPPLIER_COMBO, true);
		m_pSupplierList->SetSelByColumn(0, _variant_t((long)-1));

		m_pDiscountList = BindNxDataList2Ctrl(IDC_SALE_DISCOUNT_LIST, false);
		// Set the currency symbol in the money column header
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pDiscountList->GetColumn(dlfMoney);
		pCol->ColumnTitle = AsBstr(GetCurrencySymbol() + " Discount");
		// Set the from clause
		// (r.farnworth 2013-08-07 16:14) - PLID 45994 - Removing the ability for users to manually set Gift Certificates to be on sale
		CString str;
		str.Format("(SELECT ServiceT.ID AS ID, "
				"CASE WHEN CPTCodeT.ID IS NOT NULL THEN 'Service Code' WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' ELSE 'Service Code' END AS Type, "
				"CASE WHEN CPTCodeT.ID IS NOT NULL THEN CPTCodeT.Code WHEN ProductT.ID IS NOT NULL THEN 'Inventory Item' END AS Code, ServiceT.Name AS Name, CategoriesT.Name AS Category, SaleItemsT.PercentDiscount, SaleItemsT.MoneyDiscount, ServiceT.Active "
				"FROM ServiceT "
				"LEFT JOIN (SELECT * FROM SaleItemsT WHERE SaleID = %li) SaleItemsT ON ServiceT.ID = SaleItemsT.ServiceID "
				"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
				"WHERE ServiceT.ID NOT IN (SELECT ServiceID FROM GCTypesT)) DiscountQ ", m_nSaleID);
		m_pDiscountList->PutFromClause(AsBstr(str));

		// (a.walling 2007-07-27 08:56) - PLID 15998 - Need to hide inactive. Marking an item as such will remove it from the sale.
		str.Format("Active = 1");
		m_pDiscountList->PutWhereClause(AsBstr(str));

		m_pDiscountList->Requery();

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pDiscountCategoryList = BindNxDataList2Ctrl(IDC_SALE_DISCOUNT_CAT_COMBO, false);
		str.Format("(select -1 AS ID, '{None}' AS Description union select ID, Description from DiscountCategoriesT where Active = 1 union select ID, "
					"Description from DiscountCategoriesT where ID = %li) DiscountCatQ", m_nDiscountCategory);
		m_pDiscountCategoryList->PutFromClause(AsBstr(str));
		m_pDiscountCategoryList->Requery();
		// Get the discount category for this sale
		_RecordsetPtr prs = CreateRecordset("select DiscountCategoryID from SalesT where ID = %li", m_nSaleID);
		if (!prs->eof) {
			m_nDiscountCategory = AdoFldLong(prs, "DiscountCategoryID", -1);
		}
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nResult = m_pDiscountCategoryList->TrySetSelByColumn_Deprecated(0,_variant_t(m_nDiscountCategory));
		if (nResult == NXDATALIST2Lib::sriNoRow) {
			// The discount category still could not be selected
			MessageBox("The discount category for this sale could not be selected. Please reselect the discount category.", NULL, MB_OK|MB_ICONEXCLAMATION);
			m_pDiscountCategoryList->SetSelByColumn(0,_variant_t((long)-1));
		}
		else if (nResult != NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
			m_strDiscountCategoryName = VarString(m_pDiscountCategoryList->GetCurSel()->GetValue(1));
		}

		// Hide the supplier info
		GetDlgItem(IDC_SUPPLIER_SALE_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_SALE_SUPPLIER_COMBO)->EnableWindow(FALSE);

		m_bLoading = FALSE;

	}NxCatchAll("Error in CSaleEditorDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSaleEditorDlg::OnOK() 
{
	if (!Save())
		return;
	
	CDialog::OnOK();
}

void CSaleEditorDlg::OnCancel() 
{
	if (m_bChanged == TRUE && MessageBox("Are you sure you wish to close without saving your changes?", NULL, MB_YESNO|MB_ICONQUESTION) != IDYES)
		return;

	CDialog::OnCancel();
}

BOOL CSaleEditorDlg::Save()
{
	try {
		CWaitCursor wait;

		if (m_nSaleID == -1)
			return TRUE;

		// Check the discount category
		if (m_pDiscountCategoryList->GetCurSel() == NULL) {
			// Somehow no discount category row is selected
			MessageBox("No discount category has been selected.  Please select a discount category.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}

		// Get the basic data on the dialog
		CString strName;
		GetDlgItemText(IDC_SALE_NAME, strName);
		COleDateTime dtStartDate, dtEndDate;
		dtStartDate = VarDateTime(m_ctrlStartDate.GetValue());
		dtEndDate = VarDateTime(m_ctrlEndDate.GetValue());
		long nDiscountCategory = VarLong(m_pDiscountCategoryList->GetCurSel()->GetValue(0));

		CString strSql = BeginSqlBatch();
		CString strSqlStatement = "";

		// Update the basic information for the sale
		if (strName != m_strSaleName) {
			strSqlStatement.Format("UPDATE SalesT SET Name = '%s' WHERE ID = %li", 
				_Q(strName), m_nSaleID);
			AddStatementToSqlBatch(strSql, "%s", strSqlStatement);
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiAdminSaleName, m_nSaleID, m_strSaleName, strName, 2, aetChanged);
			m_strSaleName = strName;
		}

		if (dtStartDate != m_dtStartDate) {
			strSqlStatement.Format("UPDATE SalesT SET StartDate = '%s' WHERE ID = %li", 
				FormatDateTimeForSql(dtStartDate, dtoDate), m_nSaleID);
			AddStatementToSqlBatch(strSql, "%s", strSqlStatement);
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, m_strSaleName, nAuditID, aeiAdminSaleStartDate, m_nSaleID, FormatDateTimeForInterface(m_dtStartDate, NULL, dtoDate), FormatDateTimeForInterface(dtStartDate, NULL, dtoDate), 2, aetChanged);
			m_dtStartDate = dtStartDate;
		}

		if (dtEndDate != m_dtEndDate) {
			strSqlStatement.Format("UPDATE SalesT SET EndDate = '%s' WHERE ID = %li", 
				FormatDateTimeForSql(dtEndDate, dtoDate), m_nSaleID);
			AddStatementToSqlBatch(strSql, "%s", strSqlStatement);
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, m_strSaleName, nAuditID, aeiAdminSaleEndDate, m_nSaleID, FormatDateTimeForInterface(m_dtEndDate, NULL, dtoDate), FormatDateTimeForInterface(dtEndDate, NULL, dtoDate), 2, aetChanged);
			m_dtEndDate = dtEndDate;
		}

		if (nDiscountCategory != m_nDiscountCategory) {
			if (nDiscountCategory != -1) {
				strSqlStatement.Format("UPDATE SalesT SET DiscountCategoryID = %li WHERE ID = %li", nDiscountCategory, m_nSaleID);
				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
			else {
				strSqlStatement.Format("UPDATE SalesT SET DiscountCategoryID = NULL WHERE ID = %li", m_nSaleID);
				AddStatementToSqlBatch(strSql, strSqlStatement);
			}
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, m_strSaleName, nAuditID, aeiAdminSaleDiscountCategory, m_nSaleID, m_strDiscountCategoryName, VarString(m_pDiscountCategoryList->GetCurSel()->GetValue(1)), 2, aetChanged);
			m_nDiscountCategory = nDiscountCategory;
			m_strDiscountCategoryName = VarString(m_pDiscountCategoryList->GetCurSel()->GetValue(1));
		}

		CArray<long, long> aryNewSaleItems;

		// Go through all of the service items in the list and determine what needs to be done
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountList->GetFirstRow();
		while (pRow) {
			strSqlStatement = "";

			// Get the discount values
			double dblPercent = VarDouble(pRow->GetValue(dlfPercent), 0);
			double dblOldPercent = VarDouble(pRow->GetValue(dlfOldPercent), 0);
			COleCurrency cyMoney = VarCurrency(pRow->GetValue(dlfMoney), COleCurrency(0,0));
			COleCurrency cyOldMoney = VarCurrency(pRow->GetValue(dlfOldMoney), COleCurrency(0,0));
			long nServiceID = VarLong(pRow->GetValue(dlfID));
			CString strOldDiscount = "", strNewDiscount = "";

			// Determine what needs to be done
			if (dblOldPercent == 0 && cyOldMoney == COleCurrency(0,0) && (dblPercent != 0 || cyMoney != COleCurrency(0,0))) {
				// Add a new discount record
				strOldDiscount = "{none}";
				if (dblPercent == 0) {
					strSqlStatement.Format("INSERT INTO SaleItemsT (SaleID, ServiceID, MoneyDiscount) VALUES "
							"(%li, %li, Convert(money,'%s'))", 
							m_nSaleID, nServiceID, FormatCurrencyForSql(cyMoney));
					strNewDiscount = FormatCurrencyForInterface(cyMoney, TRUE, TRUE);
				}
				else {
					strSqlStatement.Format("INSERT INTO SaleItemsT (SaleID, ServiceID, PercentDiscount) VALUES "
							"(%li, %li, Round(%f, 2))", 
							m_nSaleID, nServiceID, dblPercent);
					strNewDiscount = FormatString("%.0f%%", dblPercent);
				}
				aryNewSaleItems.Add(nServiceID);
			}
			else if ((dblOldPercent != 0 || cyOldMoney != COleCurrency(0,0)) && dblPercent == 0 && cyMoney == COleCurrency(0,0)) {
				// Delete the discount record
				strSqlStatement.Format("DELETE FROM SaleItemsT "
						"WHERE SaleID = %li AND ServiceID = %li", m_nSaleID, nServiceID);
				strNewDiscount = "{none}";
				// Get info for audit
				if (dblOldPercent != 0)
					strOldDiscount = FormatString("%.0f%%", dblOldPercent);
				else
					strOldDiscount = FormatCurrencyForInterface(cyOldMoney, TRUE, TRUE);
			}
			else if (dblOldPercent != dblPercent || cyOldMoney != cyMoney) {
				// Update the existing discount record
				if (dblPercent == 0) {
					strSqlStatement.Format("UPDATE SaleItemsT SET MoneyDiscount = Convert(money,'%s'), PercentDiscount = NULL "
								"WHERE SaleID = %li AND ServiceID = %li", 
								FormatCurrencyForSql(cyMoney), m_nSaleID, nServiceID);
					strNewDiscount = FormatCurrencyForInterface(cyMoney, TRUE, TRUE);
				}
				else {
					strSqlStatement.Format("UPDATE SaleItemsT SET PercentDiscount = Round(%f, 2), MoneyDiscount = NULL "
								"WHERE SaleID = %li AND ServiceID = %li", 
								dblPercent, m_nSaleID, nServiceID);
					strNewDiscount = FormatString("%.0f%%", dblPercent);
				}
				// Get info for audit
				if (dblOldPercent != 0)
					strOldDiscount = FormatString("%.0f%%", dblOldPercent);
				else
					strOldDiscount = FormatCurrencyForInterface(cyOldMoney, TRUE, TRUE);
			}
			
			if (strSqlStatement != "") {
				AddStatementToSqlBatch(strSql, strSqlStatement);

				// Audit change
				CString strServiceName = FormatString("\'%s\' (%s)", VarString(pRow->GetValue(dlfName)), VarString(pRow->GetValue(dlfType)));
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, m_strSaleName, nAuditID, aeiAdminSaleItemDiscount, m_nSaleID, FormatString("Item: %s; Discount: %s", strServiceName, strOldDiscount), strNewDiscount, 2, aetChanged);
			}

			pRow = pRow->GetNextRow();
		}

		// Check to see if they are trying to put some items on sale that are already on sale elsewhere
		if (aryNewSaleItems.GetSize() > 0) {
			// Create the query
			CString strQuery;
			strQuery.Format("select SalesT.Name AS SaleName, ServiceT.Name AS ItemName "
								"from SaleItemsT "
								"left join SalesT on SalesT.ID = SaleItemsT.SaleID "
								"left join ServiceT on SaleItemsT.ServiceID = ServiceT.ID "
								"where StartDate <= '%s' and EndDate >= '%s' and ServiceID in (",
								FormatDateTimeForSql(m_dtStartDate, dtoDate), FormatDateTimeForSql(m_dtEndDate, dtoDate));
			for (int i = 0; i < aryNewSaleItems.GetSize(); i++) {
				CString strTemp;
				strTemp.Format("%li,", aryNewSaleItems.GetAt(i));
				strQuery += strTemp;
			}
			strQuery.TrimRight(",");
			strQuery += ")";
			// Run the query to find items that are already on sale
			_RecordsetPtr prs = CreateRecordset(strQuery);
			CString strSaleConflicts = "";
			long nConflictCount = 0;
			while(!prs->eof && nConflictCount < 20) {
				nConflictCount++;
				CString strTemp;
				strTemp.Format("%s (%s)\n", AdoFldString(prs, "ItemName", ""), 
					                      AdoFldString(prs, "SaleName", ""));

				strSaleConflicts += strTemp;

				if (nConflictCount == 20)
					strSaleConflicts += "...\n";

				prs->MoveNext();
			}

			if (strSaleConflicts != "") {
				CString strMsg;
				strMsg.Format("The changes cannot be saved because the following items have been given a discount when they already "
							"have a discount in another sale that overlaps with this sale:\n\n"
							"%s\n"
							"Please remove the discount from the items in this sale or in the conflicting sale.", strSaleConflicts);
				MessageBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
		}

		ExecuteSqlBatch(strSql);

		m_bChanged = FALSE;

		return TRUE;
	}NxCatchAll("Error in CSaleEditorDlg::Save");
	return FALSE;
}

BEGIN_EVENTSINK_MAP(CSaleEditorDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSaleEditorDlg)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_DISCOUNT_TYPE_COMBO, 2 /* SelChanged */, OnSelChangedSaleDiscountTypeCombo, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_ITEM_TYPE_COMBO, 2 /* SelChanged */, OnSelChangedSaleItemTypeCombo, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_DISCOUNT_LIST, 10 /* EditingFinished */, OnEditingFinishedSaleDiscountList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_DISCOUNT_LIST, 9 /* EditingFinishing */, OnEditingFinishingSaleDiscountList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_DISCOUNT_CAT_COMBO, 1 /* SelChanging */, OnSelChangingSaleDiscountCatCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_DISCOUNT_CAT_COMBO, 2 /* SelChanged */, OnSelChangedSaleDiscountCatCombo, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_SUPPLIER_COMBO, 1 /* SelChanging */, OnSelChangingSaleSupplierCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CSaleEditorDlg, IDC_SALE_DISCOUNT_CAT_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedSaleDiscountCatCombo, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSaleEditorDlg::OnSelChangedSaleDiscountTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try{
		if (lpNewSel) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

			long nSelection = VarLong(pRow->GetValue(0));

			// Set the text appropriately
			if (nSelection == dtMoney) {
				SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
			}
			else if (nSelection == dtPercent) {
				SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, "0%");
			}
		}
	}NxCatchAll("Error in CSaleEditorDlg::OnSelChangedSaleDiscountTypeCombo");
}

void CSaleEditorDlg::OnSelChangedSaleItemTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try{
		if (lpNewSel) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

			long nSelection = VarLong(pRow->GetValue(0));

			// Show the	supplier info if needed
			if (nSelection == itInventoryItem) {
				// Enable the supplier information
				// For some reason the text has to be hidden and then re-shown or it will not redraw correctly, rather annoying
				GetDlgItem(IDC_SUPPLIER_SALE_TEXT)->ShowWindow(FALSE);
				GetDlgItem(IDC_SUPPLIER_SALE_TEXT)->EnableWindow(TRUE);
				GetDlgItem(IDC_SUPPLIER_SALE_TEXT)->ShowWindow(TRUE);
				GetDlgItem(IDC_SALE_SUPPLIER_COMBO)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_SUPPLIER_SALE_TEXT)->EnableWindow(FALSE);
				GetDlgItem(IDC_SALE_SUPPLIER_COMBO)->EnableWindow(FALSE);
			}
		}
		else {
			// Hide the supplier info
			GetDlgItem(IDC_SUPPLIER_SALE_TEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_SALE_SUPPLIER_COMBO)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CSaleEditorDlg::OnSelChangedSaleDiscountTypeCombo");	
}

void CSaleEditorDlg::OnEditingFinishingSaleDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try{
		CString strTemp = "";

		// Do not allow them to assign negative amounts
		if (((nCol == dlfPercent && VarDouble(*pvarNewValue) < 0) ||
			(nCol == dlfMoney && VarCurrency(*pvarNewValue) < COleCurrency(0,0))) && *pbCommit) {
			if (nCol == dlfPercent)
				MessageBox("You may not assign a discount percent less than 0%.", NULL, MB_OK|MB_ICONEXCLAMATION);
			else {
				CString strMsg;
				strMsg.Format("You may not assign a discount amount less than %s.", FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
				MsgBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
			}
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

		// Also make sure that the percent is not over 100
		if (nCol == dlfPercent && VarDouble(*pvarNewValue) > 100 && *pbCommit) {
			MessageBox("You may not assign a discount percent more than 100%.", NULL, MB_OK|MB_ICONEXCLAMATION);
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

		if ((nCol == dlfPercent && VarDouble(*pvarNewValue) == 0) ||
			(nCol == dlfMoney && VarCurrency(*pvarNewValue) == COleCurrency(0,0))) {
			// If the value is 0, it should just be null
			pvarNewValue->vt = VT_NULL;
		}
		else if (nCol == dlfPercent) {
			// Round off the discount amount accordingly
			strTemp.Format("%.0f", VarDouble(*pvarNewValue));
			*pvarNewValue = _variant_t(atof(strTemp)).Detach();
		}
	
	}NxCatchAll("Error in CSaleEditorDlg::OnEditingFinishingSaleDiscountList");
}

void CSaleEditorDlg::OnEditingFinishedSaleDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (lpRow && bCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			_variant_t varNull;
			varNull.vt = VT_NULL;

			if (varOldValue.vt == VT_NULL && 
				((nCol == dlfPercent && VarDouble(varNewValue, 0) == 0) ||
				(nCol == dlfMoney && VarCurrency(varNewValue, COleCurrency(0,0)) == COleCurrency(0,0)))) {
				// Nothing was actually changed
				pRow->PutValue(nCol, varNull);
				return;
			}

			// There can't be both a money and a percent discount at the same time, so check for this
			if (nCol == dlfPercent && varNewValue.vt != VT_NULL) {
				// Make sure that there isn't already a value in the money column
				if (pRow->GetValue(dlfMoney).vt != VT_NULL) {
					if (MessageBox("There is already a money discount for this item.  There cannot be both a money and "
									"percent discount for an item.  Would you like to remove the money discount?  "
									"If not the percent discount will be removed.", NULL, MB_YESNO|MB_ICONQUESTION) == IDYES) {
						pRow->PutValue(dlfMoney, varNull);
					}
					else {
						pRow->PutValue(dlfPercent, varNull);
					}
				}
			}
			else if (nCol == dlfMoney && varNewValue.vt != VT_NULL) {
				// Make sure that there isn't already a value in the money column
				if (pRow->GetValue(dlfPercent).vt != VT_NULL) {
					if (MessageBox("There is already a percent discount for this item.  There cannot be both a money and "
						"percent discount for an item.  Would you like to remove the percent discount?  "
						"If not the money discount will be removed.", NULL, MB_YESNO|MB_ICONQUESTION) == IDYES) {
						pRow->PutValue(dlfPercent, varNull);
					}
					else {
						pRow->PutValue(dlfMoney, varNull);
					}
				}
			}

			m_bChanged = TRUE;
		}

	}NxCatchAll("Error in CSaleEditorDlg::OnEditingFinishedSaleDiscountList");
}

void CSaleEditorDlg::OnApplyQuickDiscount() 
{
	try {
		CWaitCursor wait;

		/////////////////////////////
		// First check to make sure that everything that is selected has been selected
		// Check the discount type
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountTypeList->GetCurSel();
		long nDiscountType = -1;
		if (pRow) {	
			nDiscountType = VarLong(pRow->GetValue(0));
		}
		else {
			MessageBox("No discount type has been selected (Percent or Money).  Please select a discount type.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Check the item type
		pRow = m_pItemTypeList->GetCurSel();
		long nItemType = -1;
		if (pRow) {	
			nItemType = VarLong(pRow->GetValue(0));
		}
		else {
			MessageBox("No item type has been selected (Service Code, Inventory Item, or Gift Certificate).  "
						"Please select an item type.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// Get the discount amount
		CString strDiscountAmount;
		GetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, strDiscountAmount);

		if(strDiscountAmount.IsEmpty()) {
			MessageBox("You must enter a discount amount before applying.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		double dblPercentDiscount = 0;
		COleCurrency cyMoneyDiscount = COleCurrency(0,0);
		if (nDiscountType == dtPercent) {
			// Convert this string into a double value
			double dblDiscountAmount = atof(strDiscountAmount);

			// Do not allow them to assign negative amounts
			if(dblDiscountAmount < 0.0) {
				if (nDiscountType == dtPercent)
					MessageBox("You may not assign a discount percent less than 0%.", NULL, MB_OK|MB_ICONEXCLAMATION);
				else {
					CString strMsg;
					strMsg.Format("You may not assign a discount amount less than %s.", FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
					MsgBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
				}
				return;
			}

			// Also make sure that the percent is not over 100
			if (nDiscountType == dtPercent && dblDiscountAmount > 100) {
				MessageBox("You may not assign a discount percent more than 100%.", NULL, MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			// Let's round the percent discount
			CString strTemp = "";
			if (nDiscountType == dtPercent) {
				strTemp.Format("%.0f", dblDiscountAmount);
			}
			dblPercentDiscount = atof(strTemp);
		}
		else if (nDiscountType == dtMoney) {
			cyMoneyDiscount.ParseCurrency(strDiscountAmount);
		}

		// Check the supplier
		if (nItemType == itInventoryItem && m_pSupplierList->GetCurSel() == NULL) {
			// Somehow no supplier row is selected
			MessageBox("No supplier has been selected.  Please select a supplier.", NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		///////////////////////////////////

		///////////////////////////////////
		// Ok, everything checks out, now let's apply the discount 
		// Get the list of inventory items for a particular supplier, if this is an inventory item
		CMap<long, long, long, long> mapSupplierItems;
		long nSupplierID = -1;
		CString strSupplierName = "";
		if (nItemType == itInventoryItem) {
			pRow = m_pSupplierList->GetCurSel();
			nSupplierID = VarLong(pRow->GetValue(0), -1);
			strSupplierName = VarString(pRow->GetValue(1));
			if (nSupplierID != -1) {
				// Get the inventory items that have this supplier
				_RecordsetPtr rs = CreateRecordset("select ProductID from MultiSupplierT where SupplierID = %li", nSupplierID);
				while (!rs->eof) {
					mapSupplierItems[AdoFldLong(rs, "ProductID")] = TRUE;
					rs->MoveNext();
				}
				rs->Close();
			}
		}

		// Warn the user before updating
		CString strMsg, strItemType;
		strItemType = nItemType == itServiceCode ? "Service Code" : (nItemType == itInventoryItem ? "Inventory Item" : "Gift Certificate");
		if (nDiscountType == dtPercent) {
			strMsg.Format("Are you sure you want to update all of the %ss%s to have a discount amount of %.0f%%?  Any discount these items "
				"currently have will be removed.", strItemType, 
				(nItemType == itInventoryItem && nSupplierID != -1) ? " with the supplier \'" + strSupplierName + "\'" : "", dblPercentDiscount);
		}
		else {
			strMsg.Format("Are you sure you want to update all of the %ss%s to have a discount amount of %s?  Any discount these items "
				"currently have will be removed.", strItemType, 
				(nItemType == itInventoryItem && nSupplierID != -1) ? " with the supplier \'" + strSupplierName + "\'" : "",
				(FormatCurrencyForInterface(cyMoneyDiscount, TRUE, TRUE)));
		}

		if(MessageBox(strMsg, NULL, MB_YESNO|MB_ICONQUESTION) != IDYES)
			return;

		wait.Restore();

		// Go through all of the service items in the list and update them accordingly
		short nUpdateCol = nDiscountType == dtMoney ? dlfMoney : dlfPercent;
		short nClearCol = nDiscountType == dtMoney ? dlfPercent : dlfMoney;
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow = m_pDiscountList->GetFirstRow();
		while (pRow) {
			// Make sure that this inventory item is offered by the selected supplier or is not an inventory item
			if (VarString(pRow->GetValue(dlfType)) == strItemType && (nSupplierID == -1 || mapSupplierItems[VarLong(pRow->GetValue(dlfID))])) {
				if (dblPercentDiscount == 0 && cyMoneyDiscount == COleCurrency(0,0))
					pRow->PutValue(nUpdateCol, _variant_t(varNull));
				else {
					if (nDiscountType == dtMoney)
						pRow->PutValue(nUpdateCol, _variant_t(cyMoneyDiscount));
					else
						pRow->PutValue(nUpdateCol, _variant_t(dblPercentDiscount));
				}
				pRow->PutValue(nClearCol, varNull);
			}
			pRow = pRow->GetNextRow();
		}

		/////////////////////////////////

		m_bChanged = TRUE;

	}NxCatchAll("Error in CSaleEditorDlg::OnApplyQuickDiscount");
}

void CSaleEditorDlg::OnKillfocusQuickDiscountAmount() 
{
	try {
		// Get the discount amount
		CString strDiscountAmount;
		GetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, strDiscountAmount);
		strDiscountAmount.TrimLeft();
		strDiscountAmount.TrimRight();

		// Check the discount type
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiscountTypeList->GetCurSel();
		long nDiscountType = -1;
		if (pRow) {	
			nDiscountType = VarLong(pRow->GetValue(0));
		}

		if (nDiscountType == dtPercent) {
			// Convert this string into a double value
			double dblDiscountAmount = atof(strDiscountAmount);
			// Do not allow them to assign negative amounts
			if(dblDiscountAmount < 0) {
				MessageBox("You may not assign a discount percent less than 0%.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, "0%");
				GetDlgItem(IDC_QUICK_DISCOUNT_AMOUNT)->SetFocus();
				return;
			}
			// Also make sure that the percent is not over 100
			if (dblDiscountAmount > 100) {
				MessageBox("You may not assign a discount percent more than 100%.", NULL, MB_OK|MB_ICONEXCLAMATION);
				SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, "0%");
				GetDlgItem(IDC_QUICK_DISCOUNT_AMOUNT)->SetFocus();
				return;
			}

			// Let's round the percent discount
			CString strTemp = "";
			strTemp.Format("%.0f", dblDiscountAmount);
			SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, strTemp + "%");
		}
		else if (nDiscountType == dtMoney) {
			//make sure that it is a valid currency
			if (!strDiscountAmount.IsEmpty() ) {
				COleCurrency cyDiscount = ParseCurrencyFromInterface(strDiscountAmount);
				if (cyDiscount.GetStatus() == COleCurrency::invalid) {
					MessageBox("Please enter a valid money amount.", NULL, MB_OK|MB_ICONEXCLAMATION);
					SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, FormatCurrencyForInterface(COleCurrency(0,0), TRUE, TRUE));
					GetDlgItem(IDC_QUICK_DISCOUNT_AMOUNT)->SetFocus();
					return;
				}

				SetDlgItemText(IDC_QUICK_DISCOUNT_AMOUNT, FormatCurrencyForInterface(cyDiscount, TRUE, TRUE));
			}
		}	
	}NxCatchAll("Error in CSaleEditorDlg::OnKillfocusQuickDiscountAmount");
}

// (a.walling 2008-05-28 11:36) - PLID 27591 - Use the new notify events
void CSaleEditorDlg::OnDropDownSaleStartDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		m_bStartDateCalandarOpen = TRUE;
	}NxCatchAll("Error in CSaleEditorDlg::OnDropDownSaleStartDate");

	*pResult = 0;
}

// (a.walling 2008-05-28 11:36) - PLID 27591 - Use the new notify events
void CSaleEditorDlg::OnCloseUpSaleStartDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		m_bStartDateCalandarOpen = FALSE;
		LRESULT dummy = 0;
		OnChangeSaleStartDate(NULL, &dummy);
	}NxCatchAll("Error in CSaleEditorDlg::OnCloseUpSaleStartDate");	

	*pResult = 0;
}

// (a.walling 2008-05-28 11:36) - PLID 27591 - Use the new notify events
void CSaleEditorDlg::OnChangeSaleStartDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// Don't check the dates if the calandar is still down
		if (!m_bStartDateCalandarOpen) {
			COleDateTime dtStartDate = VarDateTime(m_ctrlStartDate.GetValue());
			COleDateTime dtEndDate = VarDateTime(m_ctrlEndDate.GetValue());

			// Check the start date
			if (dtStartDate > dtEndDate) {
				MessageBox("The start date cannot be after the end date.", NULL, MB_OK|MB_ICONEXCLAMATION);
				m_ctrlStartDate.SetValue(_variant_t(m_dtCurStartDate));
			}
			else {
				m_dtCurStartDate = dtStartDate;
			}

			m_bChanged = TRUE;
		}	
	}NxCatchAll("Error in CSaleEditorDlg::OnChangeSaleStartDate");

	*pResult = 0;
}

// (a.walling 2008-05-28 11:36) - PLID 27591 - Use the new notify events
void CSaleEditorDlg::OnDropDownSaleEndDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		m_bEndDateCalandarOpen = TRUE;
	}NxCatchAll("Error in CSaleEditorDlg::OnDropDownSaleEndDate");

	*pResult = 0;
}

// (a.walling 2008-05-28 11:36) - PLID 27591 - Use the new notify events
void CSaleEditorDlg::OnCloseUpSaleEndDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		m_bEndDateCalandarOpen = FALSE;
		LRESULT dummy = 0;
		OnChangeSaleEndDate(NULL, &dummy);
	}NxCatchAll("Error in CSaleEditorDlg::OnCloseUpSaleEndDate");	

	*pResult = 0;
}

// (a.walling 2008-05-28 11:36) - PLID 27591 - Use the new notify events
void CSaleEditorDlg::OnChangeSaleEndDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// Don't check the dates if the calandar is still down
		if (!m_bEndDateCalandarOpen) {
			COleDateTime dtStartDate = VarDateTime(m_ctrlStartDate.GetValue());
			COleDateTime dtEndDate = VarDateTime(m_ctrlEndDate.GetValue());

			// Check the end date
			if (dtStartDate > dtEndDate) {
				MessageBox("The end date cannot be before the start date.", NULL, MB_OK|MB_ICONEXCLAMATION);
				m_ctrlEndDate.SetValue(_variant_t(m_dtCurEndDate));
			}
			else {
				m_dtCurEndDate = dtEndDate;
			}

			m_bChanged = TRUE;
		}	
	}NxCatchAll("Error in CSaleEditorDlg::OnChangeSaleEndDate");	
}

void CSaleEditorDlg::OnChangeSaleName() 
{
	try {
		if (!m_bLoading)
			m_bChanged = TRUE;

	}NxCatchAll("Error in CSaleEditorDlg::OnChangeSaleName");	
}

void CSaleEditorDlg::OnSelChangingSaleDiscountCatCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	
	}NxCatchAll("Error in CSaleEditorDlg::OnSelChangingSaleDiscountCatCombo");	
}

void CSaleEditorDlg::OnSelChangedSaleDiscountCatCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
		NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewSel);

		if (pOldRow == pNewRow) {
			// The selection really didn't change
			return;
		}

		m_bChanged = TRUE;			
	
	}NxCatchAll("Error in CSaleEditorDlg::OnSelChangedSaleDiscountCatCombo");	
}

void CSaleEditorDlg::OnSelChangingSaleSupplierCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	
	}NxCatchAll("Error in CSaleEditorDlg::OnSelChangingSaleSupplierCombo");	
}

void CSaleEditorDlg::OnTrySetSelFinishedSaleDiscountCatCombo(long nRowEnum, long nFlags) 
{
	try {
		if (nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			// The discount category still could not be selected
			MessageBox("The discount category for this sale could not be selected. Please reselect the discount category.", NULL, MB_OK|MB_ICONEXCLAMATION);
			m_pDiscountCategoryList->SetSelByColumn(0,_variant_t((long)-1));
		}
		m_strDiscountCategoryName = VarString(m_pDiscountCategoryList->GetCurSel()->GetValue(1));

	}NxCatchAll("Error in CSaleEditorDlg::OnTrySetSelFinishedSaleDiscountCatCombo");
}
