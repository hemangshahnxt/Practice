// EMNStatusConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMNStatusConfigDlg.h"

// (j.jones 2011-07-05 14:57) - PLID 43603 - created

// CEMNStatusConfigDlg dialog

enum StatusColumns
{
	scID = 0,
	scName,
	scOldName,
	scSortColumn,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CEMNStatusConfigDlg, CNxDialog)

CEMNStatusConfigDlg::CEMNStatusConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMNStatusConfigDlg::IDD, pParent)
{

}

CEMNStatusConfigDlg::~CEMNStatusConfigDlg()
{
}

void CEMNStatusConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_REMOVE_EMN_STATUS, m_btnRemove);
	DDX_Control(pDX, IDC_BTN_ADD_EMN_STATUS, m_btnAdd);
}


BEGIN_MESSAGE_MAP(CEMNStatusConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_BTN_ADD_EMN_STATUS, OnBtnAddEmnStatus)
	ON_BN_CLICKED(IDC_BTN_REMOVE_EMN_STATUS, OnBtnRemoveEmnStatus)
END_MESSAGE_MAP()


// CEMNStatusConfigDlg message handlers

BOOL CEMNStatusConfigDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnAdd.AutoSet(NXB_NEW);

		m_List = BindNxDataList2Ctrl(IDC_EMN_STATUS_LIST, false);

		//this method of building the list keeps our built-in statuses in their intended order, before any custom statuses
		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
		_RecordsetPtr rsStatus = CreateRecordset(GetRemoteDataSnapshot(), "SELECT ID, Name FROM EMRStatusListT "
			"WHERE ID IN (0,1,2) ORDER BY ID "
			""
			"SELECT ID, Name FROM EMRStatusListT "
			"WHERE ID NOT IN (0,1,2) ORDER BY Name");
		while(!rsStatus->eof) {
			IRowSettingsPtr pRow = m_List->GetNewRow();
			pRow->PutValue(scID, rsStatus->Fields->Item["ID"]->Value);
			pRow->PutValue(scName, rsStatus->Fields->Item["Name"]->Value);
			pRow->PutValue(scOldName, rsStatus->Fields->Item["Name"]->Value);
			//this bizarre sort ensures that, barring any ridiculous names,
			//these fixed columns are always on top
			pRow->PutValue(scSortColumn, " * 12345");
			pRow->PutForeColor(RGB(128,128,128));
			m_List->AddRowAtEnd(pRow, NULL);

			rsStatus->MoveNext();
		}
		rsStatus = rsStatus->NextRecordset(NULL);
		while(!rsStatus->eof) {
			IRowSettingsPtr pRow = m_List->GetNewRow();
			pRow->PutValue(scID, rsStatus->Fields->Item["ID"]->Value);
			pRow->PutValue(scName, rsStatus->Fields->Item["Name"]->Value);
			pRow->PutValue(scOldName, rsStatus->Fields->Item["Name"]->Value);
			pRow->PutValue(scSortColumn, rsStatus->Fields->Item["Name"]->Value);
			m_List->AddRowAtEnd(pRow, NULL);

			rsStatus->MoveNext();
		}
		rsStatus->Close();

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CEMNStatusConfigDlg::OnOk()
{
	try {

		CWaitCursor pWait;

		//this cannot be a parameterized batch because it is in a loop
		//with an unknown length
		CString strSqlBatch;

		if(m_List->FindByColumn(scID, (long)-1, m_List->GetFirstRow(), FALSE)) {
			//at least one row is new, so we will need a @nNewID
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewID INT");
		}

		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			long nID = VarLong(pRow->GetValue(scID));
			CString strName = VarString(pRow->GetValue(scName));
			CString strOldName = VarString(pRow->GetValue(scOldName));

			if(nID == -1) {
				//new record (this table does not have an identity column)
				AddStatementToSqlBatch(strSqlBatch, "SET @nNewID = COALESCE((SELECT Max(ID) FROM EMRStatusListT), 0) + 1");
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EMRStatusListT (ID, Name) VALUES (@nNewID, '%s')", _Q(strName));
			}
			else if(strName != strOldName) {
				//the name changed, update it
				AddStatementToSqlBatch(strSqlBatch, "UPDATE EMRStatusListT SET Name = '%s' WHERE ID = %li", _Q(strName), nID);
			}

			pRow = pRow->GetNextRow();
		}
		
		//handle deletions
		for(int i=0; i<m_aryDeletedStatuses.GetSize(); i++) {
			int nDeletedStatus = m_aryDeletedStatuses.GetAt(i);
			// (z.manning 2011-09-02 16:41) - PLID 32123 - If a preference references this status then delete it.
			// (It will then just use the default next time.)
			// (j.jones 2013-03-01 12:14) - PLID 55122 - if transcriptions use this status as a default, remove it,
			// it will just revert back to the default next time they use it
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ConfigRT WHERE (Name = 'EmrPostSignatureInsertStatus' OR Name = 'ParseOptionsDefaultEMNStatus') "
				"AND IntParam = %li", nDeletedStatus);
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EMRStatusListT WHERE ID = %li", nDeletedStatus);
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);

			m_aryDeletedStatuses.RemoveAll();
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

BOOL CEMNStatusConfigDlg::DoesStatusExist(const CString& strStatus, NXDATALIST2Lib::IRowSettingsPtr pRowToSkip)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_List->GetFirstRow();
	while(NULL != pRow) {
		//if this is the row we're editing, skip it
		if(pRowToSkip != pRow) {
			if(!strStatus.CompareNoCase(VarString(pRow->GetValue(scName)))) {
				return TRUE;
			}
		}

		pRow = pRow->GetNextRow();
	}
	return FALSE;
}

void CEMNStatusConfigDlg::OnBtnAddEmnStatus()
{
	try {

		CString strNewStatus;
		BOOL bRetry = TRUE;
		while(bRetry) {

			bRetry = FALSE;

			if(IDCANCEL == InputBoxLimited(this, "Enter a name for the new status:", strNewStatus, "", 255, false, false, NULL)) {
				return;
			}

			strNewStatus.TrimLeft();
			strNewStatus.TrimRight();

			if(strNewStatus.IsEmpty()) {
				MsgBox(MB_OK | MB_ICONEXCLAMATION, "You must enter a name for the new status.");
				bRetry = TRUE;
			}
			if(DoesStatusExist(strNewStatus, NULL)) {
				MsgBox(MB_OK | MB_ICONEXCLAMATION, "This status already exists in the list. Please choose another name.");
				bRetry = TRUE;
			}
		}

		IRowSettingsPtr pRow = m_List->GetNewRow();
		pRow->PutValue(scID, (long)-1);
		pRow->PutValue(scName, (LPCTSTR)strNewStatus);
		pRow->PutValue(scOldName, (LPCTSTR)strNewStatus);
		pRow->PutValue(scSortColumn, (LPCTSTR)strNewStatus);
		m_List->AddRowSorted(pRow, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CEMNStatusConfigDlg::OnBtnRemoveEmnStatus()
{
	try {

		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		long nID = VarLong(pRow->GetValue(scID));

		//if this is a built-in status, disallow editing
		if(nID == 0 || nID == 1 || nID == 2) {
			MsgBox(MB_OK | MB_ICONEXCLAMATION, "This is a built-in status, and cannot be removed.");
			return;
		}

		//don't let the user edit a status name if the status is in use
		if(ReturnsRecordsParam("SELECT ID FROM EMRMasterT WHERE Status = {INT}", nID)) {
			MsgBox(MB_OK | MB_ICONEXCLAMATION, "You may not remove this status because it is in use by at least one EMN.");
			return;
		}

		//remove the row
		m_List->RemoveRow(pRow);

		//if the row was not new, track that it needs to be deleted
		if(nID != -1) {
			m_aryDeletedStatuses.Add(nID);
		}

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CEMNStatusConfigDlg, CNxDialog)
	ON_EVENT(CEMNStatusConfigDlg, IDC_EMN_STATUS_LIST, 8, OnEditingStartingEmnStatusList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEMNStatusConfigDlg, IDC_EMN_STATUS_LIST, 9, OnEditingFinishingEmnStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEMNStatusConfigDlg, IDC_EMN_STATUS_LIST, 2, OnSelChangedEmnStatusList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEMNStatusConfigDlg::OnEditingStartingEmnStatusList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol != scName) {
			return;
		}

		long nID = VarLong(pRow->GetValue(scID));

		//if this is a built-in status, silently disallow editing
		if(nID == 0 || nID == 1 || nID == 2) {
			*pbContinue = FALSE;
			return;
		}

		//don't let the user edit a status name if the status is in use
		if(ReturnsRecordsParam("SELECT ID FROM EMRMasterT WHERE Status = {INT}", nID)) {
			MsgBox(MB_OK | MB_ICONEXCLAMATION, "You may not change the name of this status because it is use by at least one EMN.");
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMNStatusConfigDlg::OnEditingFinishingEmnStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol != scName) {
			return;
		}

		long nID = VarLong(pRow->GetValue(scID));

		//if this is a built-in status, leave
		if(nID == 0 || nID == 1 || nID == 2) {
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

		CString strNewStatus = VarString(pvarNewValue);

		strNewStatus.TrimLeft();
		strNewStatus.TrimRight();

		_variant_t varNew(strNewStatus);
		*pvarNewValue = varNew.Detach();

		if(strNewStatus.IsEmpty()) {
			MsgBox(MB_OK | MB_ICONEXCLAMATION, "You cannot enter a blank status.");
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

		if(DoesStatusExist(strNewStatus, pRow)) {
			MsgBox(MB_OK | MB_ICONEXCLAMATION, "This status already exists in the list. Please choose another name.");
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

void CEMNStatusConfigDlg::OnSelChangedEmnStatusList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		BOOL bEnabled = TRUE;

		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			bEnabled = FALSE;
		}
		else {
			long nID = VarLong(pRow->GetValue(scID));
			//if this is a built-in status, it cannot be removed
			if(nID == 0 || nID == 1 || nID == 2) {
				bEnabled = FALSE;
			}
		}

		GetDlgItem(IDC_BTN_REMOVE_EMN_STATUS)->EnableWindow(bEnabled);

	}NxCatchAll(__FUNCTION__);
}
