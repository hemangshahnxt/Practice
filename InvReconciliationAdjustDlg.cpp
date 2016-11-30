// InvReconciliationAdjustDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvReconciliationAdjustDlg.h"
#include "InvUtils.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "Reports.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2009-01-15 11:39) - PLID 32684 - created

// CInvReconciliationAdjustDlg dialog

enum AdjustmentListColumns {

	alcProductID = 0,
	alcProductName,
	alcCalculatedAmt,
	alcUserCount,
	alcDifference,
	alcAdjust,
	alcCountAvailPurchInv,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

CInvReconciliationAdjustDlg::CInvReconciliationAdjustDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvReconciliationAdjustDlg::IDD, pParent)
{
	m_paryProductsNeedingAdjustments = NULL;
	m_nLocationID = -1;
}

void CInvReconciliationAdjustDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_INC_REC_ADJ_CONSIGNMENT_LABEL, m_nxstaticConsignmentLabel);
	DDX_Control(pDX, IDC_BTN_PRINT_INV_REC_ADJ, m_btnPrint);
}

BEGIN_MESSAGE_MAP(CInvReconciliationAdjustDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_PRINT_INV_REC_ADJ, OnBtnPrintInvRecAdj)
END_MESSAGE_MAP()

// CInvReconciliationAdjustDlg message handlers

BOOL CInvReconciliationAdjustDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2009-07-10 09:48) - PLID 34843 - added ability to print this list
		m_btnPrint.AutoSet(NXB_PRINT);

		m_AdjList = BindNxDataList2Ctrl(IDC_ADJUSTED_PRODUCT_LIST, false);

		if(m_paryProductsNeedingAdjustments == NULL) {
			ThrowNxException("No product list was loaded!");
		}

		BOOL bHasOneCantAdjust = FALSE;

		//populate the list
		int i=0;
		for(int i=0; i<m_paryProductsNeedingAdjustments->GetSize(); i++) {

			ProductNeedingAdjustments *pPNA = (ProductNeedingAdjustments*)m_paryProductsNeedingAdjustments->GetAt(i);
			IRowSettingsPtr pRow = m_AdjList->GetNewRow();
			pRow->PutValue(alcProductID, pPNA->nProductID);
			pRow->PutValue(alcProductName, (LPCTSTR)pPNA->strProductName);
			pRow->PutValue(alcCalculatedAmt, pPNA->dblCalculated);
			pRow->PutValue(alcUserCount, pPNA->dblUserCounted);
			pRow->PutValue(alcDifference, pPNA->dblCalculated - pPNA->dblUserCounted);
			if(pPNA->bAdjust) {
				pRow->PutValue(alcAdjust, _variant_t(VARIANT_TRUE, VT_BOOL));
			}
			else {
				pRow->PutValue(alcAdjust, _variant_t(VARIANT_FALSE, VT_BOOL));
			}

			//confirm that this product needs to adjust product items
			_RecordsetPtr rs = CreateParamRecordset("SELECT HasSerialNum, HasExpDate, "
				"Convert(bit, CASE WHEN EXISTS (SELECT ID FROM ProductItemsT WHERE (SerialNum Is Not Null OR ExpDate Is Not Null)"
				"	AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"	AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"				    WHERE (Status = {INT} OR Status = {INT}) "
				"					AND ProductItemID Is Not Null) "
				"	AND Deleted = 0 AND ProductID = {INT} "
				"	AND (ProductItemsT.LocationID = {INT} OR ProductItemsT.LocationID Is Null) "
				"	AND ProductID = {INT}) "
				"	THEN 1 ELSE 0 END) AS HasProductItems "
				"FROM ProductT WHERE ID = {INT}",
				InvUtils::iadsActive, InvUtils::iadsUsed, pPNA->nProductID, m_nLocationID, pPNA->nProductID, pPNA->nProductID);

			BOOL bNeedAdjustProductItems = FALSE;
			if(!rs->eof) {
				bNeedAdjustProductItems = AdoFldBool(rs, "HasSerialNum", FALSE)
					|| AdoFldBool(rs, "HasExpDate", FALSE)
					|| AdoFldBool(rs, "HasProductItems", FALSE);
			}
			rs->Close();

			//if we require adjusting product items, we have to go through
			if(bNeedAdjustProductItems) {
			
				//check and see how many purchased inventory product items are available, and
				//not in use by any allocation			
				rs = CreateParamRecordset("SELECT ProductItemsT.ID "
					"FROM ProductItemsT "
					"WHERE ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
					"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
					"	AND PatientInvAllocationsT.Status <> {INT} "
					"	AND PatientInvAllocationDetailsT.Status IN ({INT}, {INT}) "
					"	) "
					"AND ProductItemsT.Deleted = 0 "
					"AND ProductItemsT.Status = {INT} "
					"AND ProductItemsT.LocationID = {INT} "
					"AND ProductItemsT.ProductID = {INT}",
					(long)InvUtils::iasDeleted,
					(long)InvUtils::iadsUsed, (long)InvUtils::iadsActive, 
					(long)InvUtils::pisPurchased,
					m_nLocationID,
					pPNA->nProductID);
			
				long nCountAvailPurchInv = rs->GetRecordCount();

				//not only can we use this database access to detect how many purchased inventory items
				//are available to adjust off, but we can also use this information to remove any values
				//from our aryProductItemIDsToRemove array that aren't actually available to remove
				CArray<long, long> aryAvailPurchInvItems;
				while(!rs->eof) {
					long nProductItemID = AdoFldLong(rs, "ID");
					aryAvailPurchInvItems.Add(nProductItemID);

					rs->MoveNext();
				}
				rs->Close();

				//see if our product is in the avail. array, and if not, remove it
				int j=0;
				for(j=pPNA->aryProductItemIDsToRemove.GetSize() - 1; j>=0; j--) {
					
					long nProductItemID = pPNA->aryProductItemIDsToRemove.GetAt(j);
					BOOL bFound = FALSE;

					int k=0;
					for(k=0; k<aryAvailPurchInvItems.GetSize() && !bFound; k++) {
					
						long nAvailProductItemID = aryAvailPurchInvItems.GetAt(k);

						if(nProductItemID == nAvailProductItemID) {
							bFound = TRUE;
						}
					}

					if(!bFound) {
						//if not found, we can't adjust off this item, so remove it from our array
						pPNA->aryProductItemIDsToRemove.RemoveAt(j);
					}
				}

				//now track the count available
				pRow->PutValue(alcCountAvailPurchInv, (double)nCountAvailPurchInv);

				//see if we are able to adjust this product
				if((double)nCountAvailPurchInv < (pPNA->dblCalculated - pPNA->dblUserCounted)) {

					//we can't, so track this
					bHasOneCantAdjust = TRUE;

					//remove the adjustment box
					pRow->PutValue(alcAdjust, g_cvarNull);

					//and color this row red
					pRow->PutForeColor(RGB(255,0,0));
				}
			}
			else {
				//no product items, so simply put the regular calculated amount
				pRow->PutValue(alcCountAvailPurchInv, pPNA->dblCalculated);
			}

			m_AdjList->AddRowSorted(pRow, NULL);
		}

		if(bHasOneCantAdjust) {
			//if at least one row is red, show the warning label, and show the Avail column
			m_nxstaticConsignmentLabel.ShowWindow(SW_SHOW);
			
			IColumnSettingsPtr pCol = m_AdjList->GetColumn(alcCountAvailPurchInv);
			pCol->PutStoredWidth(50);
		}

	}NxCatchAll("Error in CInvReconciliationAdjustDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CInvReconciliationAdjustDlg::OnOk()
{
	try {

		//update each product in the array

		BOOL bHasAdjustments = FALSE;

		int i=0;
		for(int i=0; i<m_paryProductsNeedingAdjustments->GetSize(); i++) {

			ProductNeedingAdjustments *pPNA = (ProductNeedingAdjustments*)m_paryProductsNeedingAdjustments->GetAt(i);
			IRowSettingsPtr pRow = m_AdjList->FindByColumn(alcProductID, pPNA->nProductID, m_AdjList->GetFirstRow(), FALSE);
			if(pRow) {
				//update the adjust status (can be NULL, turn that into FALSE)
				pPNA->bAdjust = VarBool(pRow->GetValue(alcAdjust), FALSE);
				if(pPNA->bAdjust) {
					bHasAdjustments = TRUE;

					//for each product to be adjusted, we now need to see if it has any product items,
					//whether they be serial numbered or exp. dated, and force the user to determine
					//which product items will be adjusted off (or entered in)

					//aryProductItemIDsToRemove may already have candidates for removal, so we will
					//default those items in the selected list

					//InvUtils::AdjustProductItems will handle all of these things now

					double dblProdItemsQty = pPNA->dblUserCounted - pPNA->dblCalculated;

					//if the amt. on hand was negative, then we only want to enter in serial numbers
					//for what is physicaly in stock, not the difference of the adjustment
					if(pPNA->dblCalculated < 0.0) {
						dblProdItemsQty = pPNA->dblUserCounted;
					}

					// (j.jones 2009-01-15 15:42) - PLID 32749 put AdjustProductItems to InvUtils, so we can use it here
					// @nNewProductAdjustmentID won't be defined until we execute, but it will be defined before this batch is executed
					CString strSqlBatch;					
					if(!InvUtils::AdjustProductItems(pPNA->nProductID, m_nLocationID, dblProdItemsQty, pPNA->aryProductItemIDsToRemove,
						TRUE, strSqlBatch, "@nNewProductAdjustmentID", FALSE)) {
						return;
					}

					//track this strSqlBatch per product
					pPNA->strSqlBatch = strSqlBatch;
				}
			}
			else {
				//should be impossible
				ASSERT(FALSE);
			}
		}

		if(!bHasAdjustments) {
			//warn if nothing was selected
			if(IDNO == MessageBox("You have chosen to not adjust any products. If you save this reconciliation, no changes will be made to the amounts on hand.\n\n"
				"Are you sure you wish to continue without creating any adjustments?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		else {
			//warn that we will be creating adjustments on these products
			if(IDNO == MessageBox("Practice will now adjust the selected products to the amounts you have entered.\n\n"
				"Are you sure you wish to continue and create these adjustments?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CInvReconciliationAdjustDlg::OnOk");
}

void CInvReconciliationAdjustDlg::OnCancel()
{
	try {

		//warn that they cannot save the reconciliation if the do this

		if(IDNO == MessageBox("Cancelling now will cancel saving this reconciliation.\n\n"
			"Are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CInvReconciliationAdjustDlg::OnCancel");
}

// (j.jones 2009-07-10 09:48) - PLID 34843 - added ability to print this list
void CInvReconciliationAdjustDlg::OnBtnPrintInvRecAdj()
{
	try {

		if(m_AdjList->GetRowCount() == 0) {
			AfxMessageBox("There is no information to print.");
			return;
		}

		CWaitCursor pWait;

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(671)]);

		//we have to create a temp table to run the report off of
		CString strTempTableName;
		strTempTableName.Format("#TempInvRecAdjustmentsT_%lu", GetTickCount());

		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "CREATE TABLE %s ("
			"ProductID INT NOT NULL PRIMARY KEY, "
			"ProductName nvarchar(255) NOT NULL DEFAULT (''), "
			"CalculatedAmt float NOT NULL DEFAULT 0.0, "
			"UserCount float NOT NULL DEFAULT 0.0, "
			"WillAdjust bit NOT NULL DEFAULT 0, "
			"CountAvailable float NOT NULL DEFAULT 0.0"
			") ", strTempTableName);

		//fill our table
		IRowSettingsPtr	pRow = m_AdjList->GetFirstRow();
		while(pRow) {

			long nProductID = VarLong(pRow->GetValue(alcProductID));
			CString strProductName = VarString(pRow->GetValue(alcProductName));
			double dblCalculatedAmt = VarDouble(pRow->GetValue(alcCalculatedAmt), 0.0);
			double dblUserCount = VarDouble(pRow->GetValue(alcUserCount), 0.0);
			//for the report, NULL will be converted to FALSE
			BOOL bWillAdjust = VarBool(pRow->GetValue(alcAdjust), FALSE);
			double dblCountAvail = VarDouble(pRow->GetValue(alcCountAvailPurchInv), 0.0);

			//insert into our temp table
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO %s "
				"(ProductID, ProductName, CalculatedAmt, UserCount, WillAdjust, CountAvailable) "
				"VALUES (%li, '%s', %g, %g, %li, %g)",
				strTempTableName, nProductID, _Q(strProductName), dblCalculatedAmt, dblUserCount, bWillAdjust ? 1 : 0, dblCountAvail);

			pRow = pRow->GetNextRow();
		}
		// (a.walling 2009-09-08 13:32) - PLID 35178 - Run in snapshot connection		
		ExecuteSqlBatch(GetRemoteDataSnapshot(), strSqlBatch);

		infReport.strExtendedSql = strTempTableName;

		//print this report directly

		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		CPrintInfo prInfo;
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if (prInfo.m_pPD) delete prInfo.m_pPD;
		prInfo.m_pPD = dlg;

		RunReport(&infReport, FALSE, (CWnd *)this, infReport.strReportName, &prInfo);

		//now drop the table - were an exception to be thrown previously, the table would auto-drop
		//when practice closed, but since this code is self-contained, we can drop it ourselves
		ExecuteSql(GetRemoteDataSnapshot(), "DROP TABLE %s", strTempTableName);

	}NxCatchAll("Error in CInvReconciliationAdjustDlg::OnBtnPrintInvRecAdj");
}