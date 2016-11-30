// BillTabSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "BillTabSettingsDlg.h"
#include "financialdlg.h"
#include "NxView.h"
#include "PatientView.h"

// (j.dinatale 2010-11-05) - PLID 39226 - Created


// CBillTabSettingsDlg dialog

IMPLEMENT_DYNAMIC(CBillTabSettingsDlg, CNxDialog)

// column enum
enum BillingTabConfigColumns{
	btccID = 0,
	btccColumnName,
	btccVisible,
	btccColumnType,
	btccOrderIndex,
};

CBillTabSettingsDlg::CBillTabSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillTabSettingsDlg::IDD, pParent)
{
}

CBillTabSettingsDlg::~CBillTabSettingsDlg()
{
}

BOOL CBillTabSettingsDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		// (j.dinatale 2011-01-18) - PLID 39226 - use bind
		m_dlColumnList = BindNxDataList2Ctrl(IDC_BILLTABCOLLIST, false);

		m_btnMoveColUp.AutoSet(NXB_UP);
		m_btnMoveColDown.AutoSet(NXB_DOWN);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// set up the list to only query for this users column information and ignore an OrderIndex if its less than 0
		CString strWhere;
		strWhere.Format("UserID = %li AND OrderIndex > -1", GetCurrentUserID());
		m_dlColumnList->WhereClause = _bstr_t(strWhere);
		m_dlColumnList->FromClause = _bstr_t("BillingColumnsT");
		m_dlColumnList->Requery();

		// wait for our requery since it shouldnt take that long
		m_dlColumnList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		// if for some reason our row count is less than 22, then something must be wrong, so ensure the SQL structure, and requery
		// (j.jones 2011-04-27 13:07) - PLID 43449 - it's now btcMaxValue - 8, instead of 22
		if(m_dlColumnList->GetRowCount() < (btcMaxValue - 8)){
			EnsureBillColSQLStructure();
			m_dlColumnList->Requery();
			m_dlColumnList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		// loop through our rows, and fill in the names based on column types
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_dlColumnList->GetFirstRow();

		while(pFirstRow){
			long nColType = VarLong(pFirstRow->GetValue(btccColumnType));

			CString strColName = GetColumnName(nColType);

			pFirstRow->PutValue(btccColumnName, _variant_t(strColName));

			// if the column is any of these types, ensure that they have to be visible
			if(nColType == btcDate || nColType == btcDescription || nColType == btcCharge || nColType == btcPayment
				|| nColType == btcAdjustment || nColType == btcRefund || nColType == btcInsurance || nColType == btcBalance){
					pFirstRow->PutValue(btccVisible, g_cvarNull);
			}

			pFirstRow = pFirstRow->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// returns the column name based on column type
CString CBillTabSettingsDlg::GetColumnName(long nColType)
{
	CString strName = "";

	switch(nColType)
	{
	case btcBillID:
		strName = "Bill ID";
		break;
	case btcDate:
		strName = "Date";
		break;
	case btcNoteIcon:
		strName = "Notes";
		break;
	case btcDiscountIcon:
		strName = "Discounts";
		break;
	case btcOnHoldIcon:	// (a.wilson 2014-07-24 11:37) - PLID 63015
		strName = "On Hold";
		break;
	case btcDescription:
		strName = "Description";
		break;
	case btcCharge:
		strName = "Charge Amount";
		break;
	case btcPayment:
		strName = "Payment Amount";
		break;
	case btcAdjustment:
		strName = "Adjustment Amount";
		break;
	case btcRefund:
		strName = "Refund Amount";
		break;
	case btcInsurance:
		strName = "Pat. Resp. and Insurance Amount(s)";
		break;
	case btcBalance:
		strName = "Balance";
		break;
	case btcProvider:
		strName = "Provider";
		break;
	case btcLocation:
		strName = "Location";
		break;
	case btcPOSName:
		strName = "Place Of Service Name";
		break;
	case btcPOSCode:
		strName = "Place of Service Code";
		break;
	case btcInputDate:
		strName = "Input Date";
		break;
	case btcCreatedBy:
		strName = "Created By";
		break;
	case btcFirstChargeDate: 
		strName = "First Charge Date";
		break;
	case btcDiagCode1:
		strName = "Diagnosis Code 1";
		break;
	case btcDiagCode1WithName:
		strName = "Diagnosis Code 1 With Description";
		break;
	case btcDiagCodeList:
		strName = "Diagnosis Code List";
		break;
	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
	// and added ChargeAllowable (called the EOB Allowable)
	case btcFeeSchedAllowable:
		strName = "Fee Schedule Allowable";
		break;
	case btcChargeAllowable:
		strName = "EOB Allowable";
		break;
	// (j.jones 2011-12-20 11:37) - PLID 47119 - added deductible and coinsurance
	case btcChargeDeductible:
		strName = "EOB Deductible";
		break;
	case btcChargeCoinsurance:
		strName = "EOB Coinsurance";
		break;
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	case btcInvoiceNumber:
		strName = "Invoice Number";
		break;
	// (j.jones 2015-02-27 08:44) - PLID 64944 - added TotalPaymentAmount
	case btcTotalPaymentAmount:
		strName = "Total Payment Amount";
		break;
	// (j.dinatale 2010-11-11) - PLID 39226 - Use this assert if you wish to ensure that any new columns added have a string associated with them.
	// If this ASSERT is hit, the current DB either has bad data in it or someone has added a column without updating this dialog to handle the new
	// enum value. If a column was added, add a case here to return a string thats associated with it. Also ensure that all the EnsureColStructure functions
	// are tweaked in order to accomodate the new column.
	default:
		ASSERT(FALSE);
	}

	return strName;
}

void CBillTabSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK , m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MOVECOLUP, m_btnMoveColUp);
	DDX_Control(pDX, IDC_MOVECOLDOWN, m_btnMoveColDown);
}


BEGIN_EVENTSINK_MAP(CBillTabSettingsDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CFinancialDlg)
	ON_EVENT(CBillTabSettingsDlg, IDC_BILLTABCOLLIST, 10, CBillTabSettingsDlg::OnEditFinished, VTS_DISPATCH  VTS_I2  VTS_VARIANT  VTS_VARIANT  VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CBillTabSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CBillTabSettingsDlg::OnOK)
	ON_BN_CLICKED(IDCANCEL, &CBillTabSettingsDlg::OnCancel)
	ON_BN_CLICKED(IDC_MOVECOLUP, &CBillTabSettingsDlg::OnBnClickedMovecolup)
	ON_BN_CLICKED(IDC_MOVECOLDOWN, &CBillTabSettingsDlg::OnBnClickedMovecoldown)
END_MESSAGE_MAP()

// CBillTabSettingsDlg message handlers

void CBillTabSettingsDlg::OnEditFinished(LPDISPATCH lpRow, short nCol, const VARIANT &varOldValue, const VARIANT &varNewValue, BOOL bCommit)
{
	// (j.dinatale 2010-11-12) - PLID 39226
	try{
		if(lpRow)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if(nCol == btccVisible){
				_variant_t NewVal(varNewValue);

				// if we are changing any value to false, just return, no need for changes
				if (NewVal == g_cvarFalse){
					return;
				}

				long nColType = VarLong(pRow->GetValue(btccColumnType));

				switch(nColType){
					// diag code 1 column
				case btcDiagCode1:
					{
						// ensure that the diag code 1 with name and diag code list columns are disabled
						NXDATALIST2Lib::IRowSettingsPtr pDiag1WithNameRow = 
							m_dlColumnList->FindByColumn(btccColumnType, (long)btcDiagCode1WithName, m_dlColumnList->GetFirstRow(), FALSE);
						NXDATALIST2Lib::IRowSettingsPtr pDiagCodeListRow = 
							m_dlColumnList->FindByColumn(btccColumnType, (long)btcDiagCodeList, m_dlColumnList->GetFirstRow(), FALSE);

						if(pDiag1WithNameRow){
							pDiag1WithNameRow->PutValue(btccVisible, g_cvarFalse);
						}
						if(pDiagCodeListRow){
							pDiagCodeListRow->PutValue(btccVisible, g_cvarFalse);
						}
					}
					break;
					// diag code with name
				case btcDiagCode1WithName:
					{
						// ensure that the diag 1 and diag code list columns are disabled
						NXDATALIST2Lib::IRowSettingsPtr pDiag1Row = 
							m_dlColumnList->FindByColumn(btccColumnType, (long)btcDiagCode1, m_dlColumnList->GetFirstRow(), FALSE);
						NXDATALIST2Lib::IRowSettingsPtr pDiagCodeListRow = 
							m_dlColumnList->FindByColumn(btccColumnType, (long)btcDiagCodeList, m_dlColumnList->GetFirstRow(), FALSE);

						if(pDiag1Row){
							pDiag1Row->PutValue(btccVisible, g_cvarFalse);
						}
						if(pDiagCodeListRow){
							pDiagCodeListRow->PutValue(btccVisible, g_cvarFalse);
						}
					}
					break;
					// diag code list
				case btcDiagCodeList:
					{
						// ensure diag 1 with name and diag 1 columns are disabled
						NXDATALIST2Lib::IRowSettingsPtr pDiag1WithNameRow = 
							m_dlColumnList->FindByColumn(btccColumnType, (long)btcDiagCode1WithName, m_dlColumnList->GetFirstRow(), FALSE);
						NXDATALIST2Lib::IRowSettingsPtr pDiag1Row = 
							m_dlColumnList->FindByColumn(btccColumnType, (long)btcDiagCode1, m_dlColumnList->GetFirstRow(), FALSE);

						if(pDiag1WithNameRow){
							pDiag1WithNameRow->PutValue(btccVisible, g_cvarFalse);
						}
						if(pDiag1Row){
							pDiag1Row->PutValue(btccVisible, g_cvarFalse);
						}
					}
					break;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CBillTabSettingsDlg::OnOK()
{
	try{
		// save our updated column info
		Save();

		// get the patient module window if possible
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
		if (pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) { 
			CPatientView* pPatientView = (CPatientView*)pView;
			// if possible, get the finanical dialog, and reconstruct the view
			if (pPatientView->GetFinancialDlg()->GetSafeHwnd())
				pPatientView->GetFinancialDlg()->ReconstructView();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CBillTabSettingsDlg::OnCancel()
{
	try{
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}

// saves the column information to our sql structure
void CBillTabSettingsDlg::Save()
{
	try{
		long nIndex = 0;
		NXDATALIST2Lib::IRowSettingsPtr pFirstRow;

		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray args;

		pFirstRow = m_dlColumnList->GetFirstRow();

		// for each row, add a query to our batch to update the sql structure with info about that column
		while(pFirstRow){
			long nColType = VarLong(pFirstRow->GetValue(btccColumnType));
			long nID = VarLong(pFirstRow->GetValue(btccID));

			// enforce that these column types are always visible
			if(nColType == btcDate || nColType == btcDescription || nColType == btcCharge || nColType == btcPayment
				|| nColType == btcAdjustment || nColType == btcRefund || nColType == btcInsurance || nColType == btcBalance){
					AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE BillingColumnsT SET OrderIndex = {INT}, Visible = NULL WHERE ID = {INT}", nIndex, nID);
			}else{
				// otherwise, update the structure normally
				// (j.jones 2013-07-12 16:35) - PLID 57550 - if the visible flag is on, and the StoredWidth is 0, force it back to -1
				AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE BillingColumnsT SET OrderIndex = {INT}, Visible = {VT_BOOL}, "
					"StoredWidth = (CASE WHEN {VT_BOOL} = 1 AND StoredWidth = 0 THEN -1 ELSE StoredWidth END) "
					"WHERE ID = {INT}",
					nIndex, pFirstRow->GetValue(btccVisible), pFirstRow->GetValue(btccVisible), nID);
			}

			// increment our index
			nIndex++;
			pFirstRow = pFirstRow->GetNextRow();
		}

		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
	}NxCatchAll(__FUNCTION__);
}

// moves a column up on the list
void CBillTabSettingsDlg::OnBnClickedMovecolup()
{
	// (j.dinatale 2010-11-12) - PLID 39226
	try{
		// get curr row selection
		NXDATALIST2Lib::IRowSettingsPtr pCurrSel = m_dlColumnList->CurSel;

		if(pCurrSel){
			// get prev row selection
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pCurrSel->GetPreviousRow();

			if(pPrevRow){
				// swap orderindex values
				_variant_t varOrderIndex = pCurrSel->GetValue(btccOrderIndex);
				pCurrSel->PutValue(btccOrderIndex, pPrevRow->GetValue(btccOrderIndex));
				pPrevRow->PutValue(btccOrderIndex, varOrderIndex);

				// resort our list
				m_dlColumnList->Sort();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// moves a column down the list
void CBillTabSettingsDlg::OnBnClickedMovecoldown()
{
	// (j.dinatale 2010-11-12) - PLID 39226
	try{
		// get curr selection
		NXDATALIST2Lib::IRowSettingsPtr pCurrSel = m_dlColumnList->CurSel;

		if(pCurrSel){
			// get next row
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pCurrSel->GetNextRow();

			if(pNextRow){
				// swap the order index values
				_variant_t varOrderIndex = pCurrSel->GetValue(btccOrderIndex);
				pCurrSel->PutValue(btccOrderIndex, pNextRow->GetValue(btccOrderIndex));
				pNextRow->PutValue(btccOrderIndex, varOrderIndex);

				// resort our list
				m_dlColumnList->Sort();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

int CBillTabSettingsDlg::DoModal() 
{
	int result;
	GetMainFrame()->DisableHotKeys();	
	result = CNxDialog::DoModal();
	GetMainFrame()->EnableHotKeys();
	return result;
}

// (j.dinatale 2010-10-29) - PLID 28773 - This will fix/set up the sql structure if for some reason it is necessary. It adds column info to our sql structure if missing.
void CBillTabSettingsDlg::EnsureBillColSQLStructure()
{
	try{

		// (j.jones 2011-04-27 13:17) - PLID 43449 - ChargeAllowable defaults to not visible,
		// in the process of handling it I fixed how we handle all visible columns by default,
		// since most columns default to invisible
		CArray<long, long> aryVisibleColumnDefaults;
		aryVisibleColumnDefaults.Add((long)btcProvider);
		aryVisibleColumnDefaults.Add((long)btcNoteIcon);
		aryVisibleColumnDefaults.Add((long)btcDiscountIcon);
		aryVisibleColumnDefaults.Add((long)btcOnHoldIcon);	// (a.wilson 2014-07-24 11:38) - PLID 63015

		long nUserID = GetCurrentUserID();
		BEGIN_TRANS("BillingTabColumnSetUp"){
			// (j.jones 2011-04-27 13:05) - PLID 43449 - used btcMaxValue for building new columns
			ExecuteParamSql(
				"DECLARE @CurrCol int; \r\n"
				"SET @CurrCol = 8; \r\n"
				"DECLARE @nNextOrderIndex int; \r\n"

				"IF NOT EXISTS(SELECT 1 FROM BillingColumnsT WHERE ColumnType = 6 AND UserID = {INT}) \r\n"
				"	INSERT INTO BillingColumnsT(OrderIndex, UserID, StoredWidth, Visible, ColumnType) VALUES (-1, {INT}, -1, NULL, 6) \r\n"
				"IF NOT EXISTS(SELECT 1 FROM BillingColumnsT WHERE ColumnType = 7 AND UserID = {INT}) \r\n"
				"	INSERT INTO BillingColumnsT(OrderIndex, UserID, StoredWidth, Visible, ColumnType) VALUES (-1, {INT}, -1, NULL, 7) \r\n"

				// (j.dinatale 2010-11-18) - PLID 39226 - Pulled out the Select COALESCE statement because sql 2005 doesnt like it
				"WHILE @CurrCol < {INT} BEGIN \r\n"
				"IF NOT EXISTS(SELECT 1 FROM BillingColumnsT WHERE ColumnType = @CurrCol AND UserID = {INT}) BEGIN \r\n"
				"	SET @nNextOrderIndex = (SELECT COALESCE(MAX(OrderIndex), 0) + 1 FROM BillingColumnsT WHERE UserID = {INT}); \r\n"
				"	INSERT INTO BillingColumnsT(OrderIndex, UserID, StoredWidth, Visible, ColumnType) VALUES \r\n"
				"	(@nNextOrderIndex, {INT}, -1, "
				"	CASE WHEN @CurrCol IN ({INTARRAY}) THEN 1 ELSE 0 END, "
				"	@CurrCol) END \r\n"
				"SET @CurrCol = @CurrCol +1 \r\n"
				"END \r\n",
				nUserID, nUserID, nUserID, nUserID,
				btcMaxValue,
				nUserID, nUserID, nUserID,
				aryVisibleColumnDefaults);
		}END_TRANS_CATCH_ALL("BillingTabColumnSetUp");
	}NxCatchAll(__FUNCTION__);
}