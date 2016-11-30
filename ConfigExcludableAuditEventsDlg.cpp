// ConfigExcludableAuditEventsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigExcludableAuditEventsDlg.h"
#include "AuditTrail.h"

// CConfigExcludableAuditEventsDlg dialog

// (j.jones 2010-01-08 10:46) - PLID 35778 - created

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum AuditListColums {

	alcItemID = 0,
	alcAuditDesc,
	alcLastExcludedStatus,
};

CConfigExcludableAuditEventsDlg::CConfigExcludableAuditEventsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigExcludableAuditEventsDlg::IDD, pParent)
{

}

CConfigExcludableAuditEventsDlg::~CConfigExcludableAuditEventsDlg()
{
}

void CConfigExcludableAuditEventsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BKG_AUDIT_EXCLUDE, m_bkg);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_AUDIT_ITEM, m_btnSelectOne);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_AUDIT_ITEM, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_AUDIT_ITEMS, m_btnUnselectAll);
}


BEGIN_MESSAGE_MAP(CConfigExcludableAuditEventsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_AUDIT_ITEM, OnBtnSelectOneAuditItem)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_AUDIT_ITEM, OnBtnUnselectOneAuditItem)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_AUDIT_ITEMS, OnBtnUnselectAllAuditItems)
END_MESSAGE_MAP()


// CConfigExcludableAuditEventsDlg message handlers

BOOL CConfigExcludableAuditEventsDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_bkg.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);

		m_UnselectedList = BindNxDataList2Ctrl(IDC_UNSELECTED_EXCLUDABLE_AUDIT_LIST, false);
		m_SelectedList = BindNxDataList2Ctrl(IDC_SELECTED_EXCLUDABLE_AUDIT_LIST, false);

		//force a reload of the excluded audit event list
		LoadExcludedAuditEvents(GetRemoteData(), TRUE);

		//Now we need to find which audits are potentially excludable,
		//by looping through IDs 1 through 10,000. If we ever have audit IDs
		//that surpass 10,000 (our entire structure, including reporting,
		//assumes they are in fixed ranges), then this loading code will need
		//to be modified. I noted as much in GlobalAuditUtils.h
		
		// (a.walling 2010-06-08 10:02) - PLID 38558 - Audit flags
		EnsureAuditFlags();

		for(int i=1;i<=10000;i++) {
			if(CanAuditItemBeExcluded((AuditEventItems)i)) {
				//this audit item can be excluded, so it's going to display
				//on this screen, just need to decide which list
				CString strName = GetAuditItemDescription((long)i);

				if(IsAuditItemExcluded((AuditEventItems)i,GetRemoteConnection() )) {
					//it is currently excluded, place it in the excluded list
					IRowSettingsPtr pRow = m_SelectedList->GetNewRow();
					pRow->PutValue(alcItemID, (long)i);
					pRow->PutValue(alcAuditDesc, (LPCTSTR)strName);
					pRow->PutValue(alcLastExcludedStatus, g_cvarTrue);
					m_SelectedList->AddRowSorted(pRow, NULL);
				}
				else {
					//it is not excluded, but it can be configured to do so,
					//so place it in the included list
					IRowSettingsPtr pRow = m_UnselectedList->GetNewRow();
					pRow->PutValue(alcItemID, (long)i);
					pRow->PutValue(alcAuditDesc, (LPCTSTR)strName);
					pRow->PutValue(alcLastExcludedStatus, g_cvarFalse);
					m_UnselectedList->AddRowSorted(pRow, NULL);
				}
			}
		}

	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CConfigExcludableAuditEventsDlg::OnOk()
{
	long nAuditTransactionID = -1;

	try {

		//loop through each list, and check the alcLastExcludedStatus column,
		//if it changed, save to data and audit the change

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		BOOL bChanged = FALSE;

		IRowSettingsPtr pRow = m_UnselectedList->GetFirstRow();
		while(pRow) {

			if(VarBool(pRow->GetValue(alcLastExcludedStatus))) {
				//this now-included audit item was previously excluded,
				//so we must save the change to data

				long nItemID = VarLong(pRow->GetValue(alcItemID));
				CString strName = VarString(pRow->GetValue(alcAuditDesc));

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ExcludedAuditsT WHERE ExcludedAuditItemID = {INT}", nItemID);
				
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOld = strName += ": Excluded";
				AuditEvent(-1, "", nAuditTransactionID, aeiAuditItemExclusion, nItemID, strOld, "Included", aepHigh, aetChanged);

				//flag that we changed something
				bChanged = TRUE;
			}

			pRow = pRow->GetNextRow();
		}

		pRow = m_SelectedList->GetFirstRow();
		while(pRow) {

			if(!VarBool(pRow->GetValue(alcLastExcludedStatus))) {
				//this now-excluded audit item was previously included,
				//so we must save the change to data

				long nItemID = VarLong(pRow->GetValue(alcItemID));
				CString strName = VarString(pRow->GetValue(alcAuditDesc));

				//since the audit ID is the primary key, make sure we don't insert a record
				//if someone has just done the same on another station (the double-audit is acceptable)
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO ExcludedAuditsT "
					"(ExcludedAuditItemID, ExcludedByUserName) "
					"SELECT {INT}, {STRING} "
					"WHERE NOT EXISTS (SELECT ExcludedAuditItemID FROM ExcludedAuditsT WHERE ExcludedAuditItemID = {INT})",
					nItemID, GetCurrentUserName(), nItemID);
				
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOld = strName += ": Included";
				AuditEvent(-1, "", nAuditTransactionID, aeiAuditItemExclusion, nItemID, strOld, "Excluded", aepHigh, aetChanged);

				//flag that we changed something
				bChanged = TRUE;
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		if(bChanged) {
			AfxMessageBox("The changes you have made to the excludable audit events will not affect other users until they close and reopen Practice.");

			//since they changed something, force another reload of the global exclusion list
			//to reflect the changes that were just made
			LoadExcludedAuditEvents(GetRemoteData(), TRUE);
		}

		CNxDialog::OnOK();

	} NxCatchAllCall(__FUNCTION__,
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}
BEGIN_EVENTSINK_MAP(CConfigExcludableAuditEventsDlg, CNxDialog)
	ON_EVENT(CConfigExcludableAuditEventsDlg, IDC_UNSELECTED_EXCLUDABLE_AUDIT_LIST, 3, OnDblClickCellUnselectedExcludableAuditList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigExcludableAuditEventsDlg, IDC_SELECTED_EXCLUDABLE_AUDIT_LIST, 3, OnDblClickCellSelectedExcludableAuditList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CConfigExcludableAuditEventsDlg::OnDblClickCellUnselectedExcludableAuditList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedList->PutCurSel(pRow);
		OnBtnSelectOneAuditItem();

	} NxCatchAll(__FUNCTION__);
}

void CConfigExcludableAuditEventsDlg::OnDblClickCellSelectedExcludableAuditList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedList->PutCurSel(pRow);
		OnBtnUnselectOneAuditItem();

	} NxCatchAll(__FUNCTION__);
}

void CConfigExcludableAuditEventsDlg::OnBtnSelectOneAuditItem()
{
	try {

		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

	} NxCatchAll(__FUNCTION__);
}

void CConfigExcludableAuditEventsDlg::OnBtnUnselectOneAuditItem()
{
	try {

		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

	} NxCatchAll(__FUNCTION__);
}

void CConfigExcludableAuditEventsDlg::OnBtnUnselectAllAuditItems()
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

	} NxCatchAll(__FUNCTION__);
}
