// InvReconcileHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvReconcileHistoryDlg.h"
#include "InvReconciliationDlg.h"
#include "SingleSelectDlg.h"

// (j.jones 2009-01-14 11:17) - PLID 32707 - created

// CInvReconcileHistoryDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum LocationComboColumns {

	lccID = 0,
	lccName = 1,
};

enum CurrentListColumns {

	clcID = 0,
	clcStartDate,
	clcStartedBy,
	clcProductsCounted,
};

enum HistoryListColumns {

	hlcID = 0,
	hlcStartDate,
	hlcStartedBy,
	hlcTotalProducts,
	hlcProductsCounted,
	hlcCompleteDate,
	hlcCompletedBy,
	hlcCancelDate,
	hlcCancelledBy,
};

CInvReconcileHistoryDlg::CInvReconcileHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvReconcileHistoryDlg::IDD, pParent)
{

}

void CInvReconcileHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CLOSE_INV_REC, m_btnClose);
	DDX_Control(pDX, IDC_BTN_NEW_RECONCILIATION, m_btnNew);
	DDX_Control(pDX, IDC_CHECK_SHOW_CANCELLED_INV_REC, m_checkShowCancelled);
	DDX_Control(pDX, IDC_BTN_EDIT_CUR_INV_REC, 	m_btnEditActive);
	DDX_Control(pDX, IDC_BTN_VIEW_HIST_INV_REC, m_btnViewClosed);
}


BEGIN_MESSAGE_MAP(CInvReconcileHistoryDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE_INV_REC, OnBtnCloseInvRec)
	ON_BN_CLICKED(IDC_BTN_NEW_RECONCILIATION, OnBtnNewReconciliation)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CANCELLED_INV_REC, OnCheckShowCancelledInvRec)
	ON_BN_CLICKED(IDC_BTN_EDIT_CUR_INV_REC, OnBtnEditCurInvRec)
	ON_BN_CLICKED(IDC_BTN_VIEW_HIST_INV_REC, OnBtnViewHistInvRec)
END_MESSAGE_MAP()


// CInvReconcileHistoryDlg message handlers

BOOL CInvReconcileHistoryDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnNew.AutoSet(NXB_NEW);
		m_btnEditActive.AutoSet(NXB_MODIFY);
		m_btnViewClosed.AutoSet(NXB_INSPECT);

		m_CurrentList = BindNxDataList2Ctrl(IDC_RECONCILE_CURRENT_LIST, false);
		m_HistoryList = BindNxDataList2Ctrl(IDC_RECONCILE_HISTORY_LIST, false);
		m_LocationCombo = BindNxDataList2Ctrl(IDC_RECONCILE_LOCATION, true);

		//the from clause is in the code only for readability purposes
		CString strFrom;
		strFrom = "InvReconciliationsT "
				"INNER JOIN ("
					"SELECT Min(InvReconciliationsT.ID) AS ID, "
					"Sum(Coalesce(InvReconciliationProductsT.CalculatedAmount, 0.0)) AS TotalCalculatedQuantity, "
					"Sum(Coalesce(InvReconciliationProductsT.CountedAmount, 0.0)) AS TotalCountedQuantity, "
					"Count(InvReconciliationProductsT.ID) AS TotalProducts, "
					"Sum(CASE WHEN InvReconciliationProductsT.CountedAmount Is Not Null THEN 1 ELSE 0 END) AS TotalProductsCounted "
					"FROM InvReconciliationsT "
					"LEFT JOIN InvReconciliationProductsT ON InvReconciliationsT.ID = InvReconciliationProductsT.InvReconciliationID "
					"GROUP BY InvReconciliationsT.ID "
					") AS InvRecSummaryQ ON InvReconciliationsT.ID = InvRecSummaryQ.ID "
				"LEFT JOIN UsersT StartedUsersT ON InvReconciliationsT.StartedBy = StartedUsersT.PersonID "
				"LEFT JOIN UsersT CompletedUsersT ON InvReconciliationsT.CompletedBy = CompletedUsersT.PersonID "
				"LEFT JOIN UsersT CancelledUsersT ON InvReconciliationsT.CancelledBy = CancelledUsersT.PersonID ";
		m_CurrentList->PutFromClause(_bstr_t(strFrom));
		m_HistoryList->PutFromClause(_bstr_t(strFrom));

		m_LocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());

		//set the 'show cancelled' check
		m_checkShowCancelled.SetCheck(GetRemotePropertyInt("InvReconciliations_ShowCancelled", 0, 0, GetCurrentUserName(), true) == 1);

		ReloadCurrentList();
		ReloadHistoryList();

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvReconcileHistoryDlg::OnBtnCloseInvRec()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnBtnCloseInvRec");
}

void CInvReconcileHistoryDlg::OnBtnNewReconciliation()
{
	try {

		//we don't need to check for a license, because the license was checked
		//in order to access the dialog we are in now

		long nLocationID = GetLocationID();

		if(nLocationID == -1) {
			//should be impossible
			ASSERT(FALSE);
			m_CurrentList->Clear();
			return;
		}

		// (j.jones 2009-07-09 10:05) - PLID 34826 - check the adjustment permission
		if(!CheckCurrentUserPermissions(bioInvItem, sptDynamic0)) {
			return;
		}

		CWaitCursor pWait;

		CInvReconciliationDlg dlg(this);
		dlg.m_nID = -1;
		dlg.m_nLocationID = nLocationID;
		if(dlg.DoModal() == IDOK) {
			//reload both lists
			ReloadCurrentList();
			ReloadHistoryList();
		}

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnBtnNewReconciliation");
}
BEGIN_EVENTSINK_MAP(CInvReconcileHistoryDlg, CNxDialog)
	ON_EVENT(CInvReconcileHistoryDlg, IDC_RECONCILE_LOCATION, 16, OnSelChosenReconcileLocation, VTS_DISPATCH)
	ON_EVENT(CInvReconcileHistoryDlg, IDC_RECONCILE_CURRENT_LIST, 3, OnDblClickCellReconcileCurrentList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvReconcileHistoryDlg, IDC_RECONCILE_HISTORY_LIST, 3, OnDblClickCellReconcileHistoryList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CInvReconcileHistoryDlg::OnSelChosenReconcileLocation(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//set the current location
			pRow = m_LocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());

			if(pRow == NULL) {
				//should be impossible
				ASSERT(FALSE);

				//select the first location
				pRow = m_LocationCombo->GetFirstRow();
				if(pRow == NULL) {
					//should be impossible
					ASSERT(FALSE);
					return;
				}
				else {
					m_LocationCombo->PutCurSel(pRow);
				}
			}
		}

		//reload both lists
		ReloadCurrentList();
		ReloadHistoryList();

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnSelChosenReconcileLocation");
}

void CInvReconcileHistoryDlg::ReloadCurrentList()
{
	try {

		//get the location ID
		long nLocationID = GetLocationID();

		if(nLocationID == -1) {
			//should be impossible
			ASSERT(FALSE);
			m_CurrentList->Clear();
			return;
		}

		//aside from filtering by location, we also need to filter
		//only on active reconciliations, and there should only
		//really be one for this location
		CString strWhere;
		strWhere.Format("InvReconciliationsT.LocationID = %li AND InvReconciliationsT.CancelDate Is Null "
			"AND InvReconciliationsT.CompleteDate Is Null", nLocationID);
		m_CurrentList->PutWhereClause(_bstr_t(strWhere));
		m_CurrentList->Requery();

	}NxCatchAll("Error in CInvReconcileHistoryDlg::ReloadCurrentList");
}

void CInvReconcileHistoryDlg::ReloadHistoryList()
{
	try {

		//get the location ID
		long nLocationID = GetLocationID();

		if(nLocationID == -1) {
			//should be impossible
			ASSERT(FALSE);
			m_HistoryList->Clear();
			return;
		}

		//show/hide the cancelled columns
		IColumnSettingsPtr pCancelDate = m_HistoryList->GetColumn(hlcCancelDate);
		IColumnSettingsPtr pCancelledBy = m_HistoryList->GetColumn(hlcCancelledBy);
		if(m_checkShowCancelled.GetCheck()) {
			if(pCancelDate) {
				pCancelDate->PutStoredWidth(120);
				pCancelDate->PutColumnStyle(csVisible|csWidthData);
			}
			if(pCancelledBy) {
				pCancelledBy->PutStoredWidth(-1);
				pCancelledBy->PutColumnStyle(csVisible|csWidthAuto);
			}
		}
		else {
			if(pCancelDate) {
				pCancelDate->PutStoredWidth(0);
				pCancelDate->PutColumnStyle(csVisible|csFixedWidth);
			}
			if(pCancelledBy) {
				pCancelledBy->PutStoredWidth(0);
				pCancelledBy->PutColumnStyle(csVisible|csFixedWidth);
			}
		}

		//aside from filtering by location, we also need to filter
		//only on active reconciliations, and there should only
		//really be one for this location
		CString strWhere;
		strWhere.Format("InvReconciliationsT.LocationID = %li AND (%s "
			"OR InvReconciliationsT.CompleteDate Is Not Null)",
			nLocationID, m_checkShowCancelled.GetCheck() ? "InvReconciliationsT.CancelDate Is Not Null" : "1 = 0");
		m_HistoryList->PutWhereClause(_bstr_t(strWhere));
		m_HistoryList->Requery();

	}NxCatchAll("Error in CInvReconcileHistoryDlg::ReloadHistoryList");
}
void CInvReconcileHistoryDlg::OnCheckShowCancelledInvRec()
{
	try {

		//reload just the history list
		ReloadHistoryList();

		//and save our setting
		SetRemotePropertyInt("InvReconciliations_ShowCancelled", m_checkShowCancelled.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnCheckShowCancelledInvRec");
}

long CInvReconcileHistoryDlg::GetLocationID()
{
	try {

		//get the location ID
		IRowSettingsPtr pRow = m_LocationCombo->GetCurSel();
		if(pRow == NULL) {
			//set the current location
			pRow = m_LocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());

			if(pRow == NULL) {
				//should be impossible
				ASSERT(FALSE);

				//select the first location
				pRow = m_LocationCombo->GetFirstRow();
				if(pRow == NULL) {
					//should be impossible
					ASSERT(FALSE);
					return -1;
				}
				else {
					m_LocationCombo->PutCurSel(pRow);
				}
			}
		}

		return VarLong(pRow->GetValue(lccID));

	}NxCatchAll("Error in CInvReconcileHistoryDlg::GetLocationID");

	return -1;
}
void CInvReconcileHistoryDlg::OnDblClickCellReconcileCurrentList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CWaitCursor pWait;

		CInvReconciliationDlg dlg(this);
		dlg.m_nID = VarLong(pRow->GetValue(clcID));

		// (j.jones 2009-07-09 10:05) - PLID 34826 - check the adjustment permission,
		// but we don't need to check if the reconciliation is closed
		BOOL bAccess = GetCurrentUserPermissions(bioInvItem) & sptDynamic0;
		BOOL bAccessWithPass = GetCurrentUserPermissions(bioInvItem) & sptDynamic0WithPass;
		if (!bAccess && bAccessWithPass) {
			// Prompt for password
			if (CheckCurrentUserPassword())	{
				bAccess = TRUE;
			}
		}

		dlg.m_bCanEdit = bAccess;

		if(!dlg.m_bCanEdit) {
			AfxMessageBox("You do not have permission to adjust inventory items, therefore the inventory reconciliation will be read-only.");
		}

		if(dlg.DoModal() == IDOK) {
			//reload both lists
			ReloadCurrentList();
			ReloadHistoryList();
		}

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnDblClickCellReconcileCurrentList");
}

void CInvReconcileHistoryDlg::OnDblClickCellReconcileHistoryList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CWaitCursor pWait;

		CInvReconciliationDlg dlg(this);
		dlg.m_nID = VarLong(pRow->GetValue(hlcID));
		dlg.m_bCanEdit = FALSE;
		if(dlg.DoModal() == IDOK) {
			//reload just the history list, it's impossible for us
			//to have changed the current list
			ReloadHistoryList();
		}

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnDblClickCellReconcileHistoryList");
}

void CInvReconcileHistoryDlg::OnBtnEditCurInvRec()
{
	try {

		IRowSettingsPtr pRow = m_CurrentList->GetCurSel();

		//if NULL, try to auto-select the first row if there is only one row,
		//as there usually will only be one row
		if(pRow == NULL && m_CurrentList->GetRowCount() == 1) {
			pRow = m_CurrentList->GetFirstRow();
		}

		if(pRow == NULL) {
			AfxMessageBox("Please select an active inventory reconciliation to edit.");
			return;
		}

		//fire a double click on the row
		OnDblClickCellReconcileCurrentList(pRow, clcID);

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnBtnEditCurInvRec");
}

void CInvReconcileHistoryDlg::OnBtnViewHistInvRec()
{
	try {

		IRowSettingsPtr pRow = m_HistoryList->GetCurSel();
		if(pRow == NULL) {
			if(m_HistoryList->GetRowCount() > 0) {
				AfxMessageBox("Please select a closed inventory reconciliation to view.");
			}
			else {
				AfxMessageBox("There are no closed inventory reconciliations available to view.");
			}
			return;
		}

		//fire a double click on the row
		OnDblClickCellReconcileHistoryList(pRow, hlcID);

	}NxCatchAll("Error in CInvReconcileHistoryDlg::OnBtnViewHistInvRec");
}
