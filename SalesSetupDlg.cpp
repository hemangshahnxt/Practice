// SalesSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "SalesSetupDlg.h"
#include "SaleEditorDlg.h"
#include "datetimeutils.h"
#include "AuditTrail.h"
#include "internationalutils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.wetta 2007-05-07 15:11) - PLID 15998 - Created the dialog

/////////////////////////////////////////////////////////////////////////////
// CSalesSetupDlg dialog


CSalesSetupDlg::CSalesSetupDlg(CWnd* pParent)
	: CNxDialog(CSalesSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSalesSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSalesSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSalesSetupDlg)
	DDX_Control(pDX, IDC_SHOW_OUTDATED_SALES, m_btnShowOutdatedSales);
	DDX_Control(pDX, IDC_EDIT_SALE, m_btnEdit);
	DDX_Control(pDX, IDC_REMOVE_SALE, m_btnRemove);
	DDX_Control(pDX, IDC_ADD_SALE, m_btnAdd);
	DDX_Control(pDX, IDC_SERVICE_ITEM_SALES_TITLE, m_nxstaticServiceItemSalesTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSalesSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSalesSetupDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_ADD_SALE, OnAddSale)
	ON_BN_CLICKED(IDC_REMOVE_SALE, OnRemoveSale)
	ON_BN_CLICKED(IDC_EDIT_SALE, OnEditSale)
	ON_BN_CLICKED(IDC_SHOW_OUTDATED_SALES, OnShowOutdatedSales)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSalesSetupDlg message handlers

BOOL CSalesSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/25/2008) - PLID 29566 - Set button styles
	m_btnEdit.AutoSet(NXB_MODIFY);
	m_btnRemove.AutoSet(NXB_DELETE);
	m_btnAdd.AutoSet(NXB_NEW);

	GetDlgItem(IDC_EDIT_SALE)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMOVE_SALE)->EnableWindow(FALSE);
	
	// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
	m_pSalesList = BindNxDataList2Ctrl(IDC_SALE_LIST, false);
	RefreshSalesList();

	// (a.wetta 2007-05-17 17:49) - PLID 25394 - Check the permissions and enable the add button appropriately
	GetDlgItem(IDC_ADD_SALE)->EnableWindow(CheckCurrentUserPermissions(bioServiceItemSales, sptCreate, FALSE, 0, TRUE, TRUE));
	
	// (a.wetta 2007-05-21 14:23) - PLID 25960 - Bold the title
	extern CPracticeApp theApp;
	GetDlgItem(IDC_SERVICE_ITEM_SALES_TITLE)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

	SetControlPositions();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSalesSetupDlg::RefreshSalesList()
{
	try {
		GetDlgItem(IDC_EDIT_SALE)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_SALE)->EnableWindow(FALSE);

		if (IsDlgButtonChecked(IDC_SHOW_OUTDATED_SALES)) {
			m_pSalesList->PutWhereClause(_bstr_t(""));
		}
		else {
			m_pSalesList->PutWhereClause(_bstr_t("EndDate > GetDate()-1"));
		}
		m_pSalesList->Requery();

	}NxCatchAll("Error in CSalesSetupDlg::RefreshSalesList");
}


void CSalesSetupDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh 
{
	try {
		RefreshSalesList();
	
	}NxCatchAll("Error in CSalesSetupDlg::UpdateView");
}

void CSalesSetupDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	SetControlPositions();
	Invalidate();
}

void CSalesSetupDlg::OnAddSale() 
{
	try {
		// (a.wetta 2007-05-17 17:35) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioServiceItemSales, sptCreate))
			return;

		// Create the new row
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pSalesList->GetNewRow();
		
		long nNewID = NewNumber("SalesT", "ID");
		CString strNewName = "<New Sale>";

		// Get the current date
		COleDateTime dtCurrentDate;
		dtCurrentDate.ParseDateTime(FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), VAR_DATEVALUEONLY);

		// Add the new row to the data
		CString strSQL;
		strSQL.Format("INSERT INTO SalesT (ID, Name, StartDate, EndDate) "
			"VALUES (%li, '%s', '%s', '%s')", nNewID, strNewName, FormatDateTimeForSql(dtCurrentDate, dtoDate), 
			FormatDateTimeForSql(dtCurrentDate, dtoDate));
		ExecuteSqlStd(strSQL);

		// Add the new row to the datalist
		pNewRow->PutValue(slfID, _variant_t(nNewID));
		pNewRow->PutValue(slfName, _variant_t(strNewName));
		_variant_t varCurrentDate = dtCurrentDate;
		varCurrentDate.vt = VT_DATE;
		pNewRow->PutValue(slfStartDate, varCurrentDate);
		pNewRow->PutValue(slfEndDate, varCurrentDate);
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pNewRow->PutValue(slfDiscountCategory, varNull);
		m_pSalesList->AddRowSorted(pNewRow, NULL);

		// Audit the new sale
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiAdminSaleAdded, nNewID, "", strNewName, 2, aetCreated);

		// Open the sale editor for the new sale
		m_pSalesList->SetSelByColumn(slfID, _variant_t(nNewID));
		OnEditSale();
	
	}NxCatchAll("Error in CSalesSetupDlg::OnAddSale");
}

void CSalesSetupDlg::OnRemoveSale() 
{
	try {
		// (a.wetta 2007-05-17 17:35) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioServiceItemSales, sptDelete))
			return;

		// Get the current row
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pSalesList->GetCurSel();

		if (pCurRow) {
			CString strMsg;
			strMsg.Format("Are you sure you want to delete the sale '%s'?", VarString(pCurRow->GetValue(slfName)));

			if (IDNO == MessageBox(strMsg, NULL, MB_YESNO|MB_ICONQUESTION))
				return;

			// Delete the row from the data
			long lSaleID = VarLong(pCurRow->GetValue(slfID));
			CString strSql = BeginSqlBatch(), strSqlStatement = "";
			strSqlStatement.Format("DELETE FROM SaleItemsT WHERE SaleID = %li", lSaleID);
			AddStatementToSqlBatch(strSql, strSqlStatement);
			strSqlStatement.Format("DELETE FROM SalesT WHERE ID = %li", lSaleID);
			AddStatementToSqlBatch(strSql, strSqlStatement);
			ExecuteSqlBatch(strSql);

			// Delete the row from the datalist
			m_pSalesList->RemoveRow(pCurRow);

			// Audit the delete
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiAdminSaleDeleted, lSaleID, VarString(pCurRow->GetValue(slfName)), "", 2, aetDeleted);
		}
	
	}NxCatchAll("Error in CSalesSetupDlg::OnRemoveSale");
}

void CSalesSetupDlg::OnEditSale() 
{
	try {
		// (a.wetta 2007-05-17 17:35) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioServiceItemSales, sptWrite))
			return;

		CWaitCursor wait;

		CSaleEditorDlg dlg(this);

		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pSalesList->GetCurSel();

		if (pCurRow) {
			// Set the sale ID
			dlg.m_nSaleID = VarLong(pCurRow->GetValue(slfID));
			dlg.m_strSaleName = VarString(pCurRow->GetValue(slfName));
			dlg.m_dtStartDate = VarDateTime(pCurRow->GetValue(slfStartDate));
			dlg.m_dtEndDate = VarDateTime(pCurRow->GetValue(slfEndDate));
			dlg.m_nDiscountCategory = VarLong(pCurRow->GetValue(slfDiscountCategory), -1);

			// Start the editor
			dlg.DoModal();

			// Refresh the info on the row
			pCurRow->PutValue(slfName, _variant_t(dlg.m_strSaleName));
			_variant_t varStartDate = dlg.m_dtStartDate;
			varStartDate.vt = VT_DATE;
			pCurRow->PutValue(slfStartDate, varStartDate);
			_variant_t varEndDate = dlg.m_dtEndDate;
			varEndDate.vt = VT_DATE;
			pCurRow->PutValue(slfEndDate, varEndDate);	
			pCurRow->PutValue(slfDiscountCategory, _variant_t(dlg.m_nDiscountCategory));
		}

	}NxCatchAll("Error in CSalesSetupDlg::OnEditSale");
}

void CSalesSetupDlg::OnShowOutdatedSales() 
{
	try {
		RefreshSalesList();

	}NxCatchAll("Error in CSalesSetupDlg::OnShowOutdatedSales");
}

BEGIN_EVENTSINK_MAP(CSalesSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSalesSetupDlg)
	ON_EVENT(CSalesSetupDlg, IDC_SALE_LIST, 3 /* DblClickCell */, OnDblClickCellSaleList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSalesSetupDlg, IDC_SALE_LIST, 9 /* EditingFinishing */, OnEditingFinishingSaleList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSalesSetupDlg, IDC_SALE_LIST, 10 /* EditingFinished */, OnEditingFinishedSaleList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSalesSetupDlg, IDC_SALE_LIST, 2 /* SelChanged */, OnSelChangedSaleList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSalesSetupDlg, IDC_SALE_LIST, 8 /* EditingStarting */, OnEditingStartingSaleList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CSalesSetupDlg::OnDblClickCellSaleList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		// (a.wetta 2007-05-17 17:35) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioServiceItemSales, sptWrite, FALSE, 0, TRUE, TRUE))
			return;

		OnEditSale();

	}NxCatchAll("Error in CSalesSetupDlg::OnDblClickCellSaleList");	
}

void CSalesSetupDlg::OnEditingFinishingSaleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (lpRow && *pbCommit) {
			// (a.wetta 2007-05-21 09:26) - PLID 25394 - Make sure that something was actually changed
			if (_variant_t(varOldValue) == _variant_t(*pvarNewValue)) {
				*pbContinue = TRUE;
				*pbCommit = FALSE;
				return;
			}

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			// (a.wetta 2007-05-18 12:43) - PLID 25394 - If they have write permission with password, have them enter the password
			if (!CheckCurrentUserPermissions(bioProviderCommission, sptWrite)) {
				*pbContinue = TRUE;
				*pbCommit = FALSE;
				return;
			}
			
			CString strSaleConflicts;
			switch (nCol) {
			case slfName:
				// Make sure that the name is not blank
				if (VarString(*pvarNewValue) == "") {
					MessageBox("The name cannot be blank.  Please enter a name for this sale.", NULL, MB_OK|MB_ICONEXCLAMATION);
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
				break;
			case slfStartDate:
				// Make sure that the start date is before the end date
				if (VarDateTime(*pvarNewValue) > VarDateTime(pRow->GetValue(slfEndDate))) {
					MessageBox("The start date cannot be after the end date.", NULL, MB_OK|MB_ICONEXCLAMATION);
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					break;
				}
				// Make sure the new date range doesn't conflict with another sale
				if (!VerifyDateRange(VarLong(pRow->GetValue(slfID)), VarDateTime(*pvarNewValue),
					VarDateTime(pRow->GetValue(slfEndDate)), strSaleConflicts)) {
					CString strMsg;
					strMsg.Format("The start date cannot be saved because the following items have a discount in this sale that would overlap "
								"with the discounts for the same items in another sale:\n\n"
								"%s\n"
								"Please remove the discount from the items in this sale or in the conflicting sale, or select another start date "
								"that doesn't overlap with another sale that has the same items discounted.", strSaleConflicts);
					MessageBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					break;
				}
				break;
			case slfEndDate:
				// Make sure that the end date is after the start date
				if (VarDateTime(pRow->GetValue(slfStartDate)) > VarDateTime(*pvarNewValue)) {
					MessageBox("The end date cannot be before the start date.", NULL, MB_OK|MB_ICONEXCLAMATION);
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					break;
				}
				// Make sure the new date range doesn't conflict with another sale
				if (!VerifyDateRange(VarLong(pRow->GetValue(slfID)), VarDateTime(pRow->GetValue(slfStartDate)),
					VarDateTime(*pvarNewValue), strSaleConflicts)) {
					CString strMsg;
					strMsg.Format("The end date cannot be saved because the following items have a discount in this sale that would overlap "
								"with the discounts for the same items in another sale:\n\n"
								"%s\n"
								"Please remove the discount from the items in this sale or in the conflicting sale, or select another end date "
								"that doesn't overlap with another sale that has the same items discounted.", strSaleConflicts);
					MessageBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					break;
				}
				break;
			}		
		}
	}NxCatchAll("Error in CSalesSetupDlg::OnEditingFinishingSaleList");
}

BOOL CSalesSetupDlg::VerifyDateRange(long nSaleID, COleDateTime dtStartDate, COleDateTime dtEndDate, CString &strSaleConflicts)
{
	try{
		// Check to see if the date range of this sale interferes with any other sales
		// Create the query
		CString strQuery;
		strQuery.Format("select SalesT.Name AS SaleName, ServiceT.Name AS ItemName "
							"from SaleItemsT "
							"left join SalesT on SalesT.ID = SaleItemsT.SaleID "
							"left join ServiceT on SaleItemsT.ServiceID = ServiceT.ID "
							"where StartDate <= '%s' and EndDate >= '%s' and SalesT.ID <> %li",
							FormatDateTimeForSql(dtEndDate, dtoDate), FormatDateTimeForSql(dtStartDate, dtoDate), nSaleID);
		// Run the query to find items that are already on sale at the same time
		_RecordsetPtr prs = CreateRecordset(strQuery);
		strSaleConflicts = "";
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
			return FALSE;
		}
	}NxCatchAll("Error in CSalesSetupDlg::VerifyDateRange");
	return TRUE;
}

void CSalesSetupDlg::OnEditingFinishedSaleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (lpRow && bCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (_variant_t(varOldValue) != _variant_t(varNewValue)) {
				long lSaleID = VarLong(pRow->GetValue(slfID));
				CString strSaleName = VarString(pRow->GetValue(slfName));
				CString strSQL = "";

				long nAuditID = BeginNewAuditEvent();

				switch (nCol) {
				case slfName:			
					strSQL.Format("UPDATE SalesT SET Name = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), lSaleID);
					AuditEvent(-1, "", nAuditID, aeiAdminSaleName, lSaleID, VarString(varOldValue), VarString(varNewValue), 2, aetChanged);
					break;
				case slfStartDate:
					strSQL.Format("UPDATE SalesT SET StartDate = '%s' WHERE ID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), lSaleID);
					AuditEvent(-1, strSaleName, nAuditID, aeiAdminSaleStartDate, lSaleID, FormatDateTimeForInterface(VarDateTime(varOldValue), NULL, dtoDate), FormatDateTimeForInterface(VarDateTime(varNewValue), NULL, dtoDate), 2, aetChanged);
					break;
				case slfEndDate:
					strSQL.Format("UPDATE SalesT SET EndDate = '%s' WHERE ID = %li", FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate), lSaleID);
					AuditEvent(-1, strSaleName, nAuditID, aeiAdminSaleEndDate, lSaleID, FormatDateTimeForInterface(VarDateTime(varOldValue), NULL, dtoDate), FormatDateTimeForInterface(VarDateTime(varNewValue), NULL, dtoDate), 2, aetChanged);
					break;
				}		

				ExecuteSqlStd(strSQL);
			}
		}

	}NxCatchAll("Error in CSalesSetupDlg::OnEditingFinishedSaleList");
}

void CSalesSetupDlg::OnSelChangedSaleList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		if (lpNewSel) {
			// (a.wetta 2007-05-17 17:52) - PLID 25394 - Check permissions when enabling buttons
			GetDlgItem(IDC_EDIT_SALE)->EnableWindow(CheckCurrentUserPermissions(bioServiceItemSales, sptWrite, FALSE, 0, TRUE, TRUE));
			GetDlgItem(IDC_REMOVE_SALE)->EnableWindow(CheckCurrentUserPermissions(bioServiceItemSales, sptDelete, FALSE, 0, TRUE, TRUE));
		}
		else {
			GetDlgItem(IDC_EDIT_SALE)->EnableWindow(FALSE);
			GetDlgItem(IDC_REMOVE_SALE)->EnableWindow(FALSE);
		}
	
	}NxCatchAll("Error in CSalesSetupDlg::OnSelChangedSaleList");
}

void CSalesSetupDlg::OnEditingStartingSaleList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (a.wetta 2007-05-17 17:35) - PLID 25394 - Check permissions
		if (!CheckCurrentUserPermissions(bioServiceItemSales, sptWrite, FALSE, 0, TRUE, TRUE)) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll("Error in CSalesSetupDlg::OnSelChangedSaleList");	
}
