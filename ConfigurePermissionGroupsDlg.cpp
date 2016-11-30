// ConfigurePermissionGroupsDlg.cpp : implementation file
//

// (j.gruber 2010-04-13 16:51) - PLID 37948 - created for
// (d.thompson 2013-03-26) - PLID 55847 - There were invisible Move All Right & Left buttons with code that noone is
//	sure ever worked.  I deleted all traces.

#include "stdafx.h"
#include "Practice.h"
#include "ConfigurePermissionGroupsDlg.h"
#include "ContactsRc.h"
#include "PermissionUtils.h"
#include "AuditTrail.h"
#include "GeneratePermissionUpdateBatch.h"

enum selListCol {
	slcID = 0,
	slcName,
};

// CConfigurePermissionGroupsDlg dialog

IMPLEMENT_DYNAMIC(CConfigurePermissionGroupsDlg, CNxDialog)

CConfigurePermissionGroupsDlg::CConfigurePermissionGroupsDlg(BOOL bConfigureUser, long nUserGroupID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigurePermissionGroupsDlg::IDD, pParent)
{
	m_bConfigureUser = bConfigureUser;
	m_nUserGroupID = nUserGroupID;

}

CConfigurePermissionGroupsDlg::~CConfigurePermissionGroupsDlg()
{
}

void CConfigurePermissionGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PERMS_ONE_LEFT, m_btnOneLeft);
	DDX_Control(pDX, IDC_PERMS_ONE_RIGHT, m_btnOneRight);
	DDX_Control(pDX, IDC_GOTO_OTHER, m_btnSwitch);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CConfigurePermissionGroupsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PERMS_ONE_RIGHT, &CConfigurePermissionGroupsDlg::OnBnClickedPermsOneRight)
	ON_BN_CLICKED(IDC_PERMS_ONE_LEFT, &CConfigurePermissionGroupsDlg::OnBnClickedPermsOneLeft)
	ON_BN_CLICKED(IDC_GOTO_OTHER, &CConfigurePermissionGroupsDlg::OnBnClickedGotoOther)		
END_MESSAGE_MAP()


// CConfigurePermissionGroupsDlg message handlers
BOOL CConfigurePermissionGroupsDlg::OnInitDialog() 
{

	CNxDialog::OnInitDialog();

	try {

		m_pAvailList = BindNxDataList2Ctrl(IDC_PERMS_AVAIL_LIST, false);
		m_pSelList = BindNxDataList2Ctrl(IDC_PERMS_SEL_LIST, false);
		m_pPickList = BindNxDataList2Ctrl(IDC_PERMS_PICK_LIST, false);

		m_btnOneLeft.AutoSet(NXB_LEFT);
		m_btnOneRight.AutoSet(NXB_RIGHT);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnSwitch.AutoSet(NXB_MODIFY);

		ReloadDialog();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CConfigurePermissionGroupsDlg::ReloadDialog() {

	try {

		if (!m_bConfigureUser) {			
			// (j.gruber 2010-08-17 09:41) - PLID 40138 - change template to Groups
			SetWindowText("Configure Permission Groups");
			m_btnSwitch.SetWindowText("User Configuration");
		}
		else {
			SetWindowText("Configure Users");
			// (j.gruber 2010-08-17 09:41) - PLID 40138 - change template to Groups
			m_btnSwitch.SetWindowText("Group Configuration");
		}

		CString strFromSelLists, strFromPickList;
		if (!m_bConfigureUser) {

			strFromSelLists = "(SELECT PersonID as ID, UserName as Name FROM UsersT WHERE Administrator = 0 AND PersonID > 0) Q";
			strFromPickList = "(SELECT PersonID as ID, Name FROM UserGroupsT)Q ";
		}
		else {
			strFromSelLists = "(SELECT PersonID as ID, Name FROM UserGroupsT)Q ";
			strFromPickList = "(SELECT PersonID as ID, UserName as Name FROM UsersT WHERE Administrator = 0 AND PersonID > 0) Q";
		}

		m_pAvailList->FromClause = _bstr_t(strFromSelLists);
		m_pSelList->FromClause = _bstr_t(strFromSelLists);
		m_pPickList->FromClause = _bstr_t(strFromPickList);

		m_pPickList->Requery();

		//the other two lists are requeried once the other is

	}NxCatchAll(__FUNCTION__);

}
void CConfigurePermissionGroupsDlg::OnBnClickedPermsOneRight()
{
	try {
	
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAvailList->GetFirstSelRow();
		CWaitCursor cwait;

		while (pRow) {
			MoveOneRight(pRow);

			pRow = pRow->GetNextSelRow();
		}

		m_pSelList->TakeCurrentRowAddSorted(m_pAvailList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePermissionGroupsDlg::OnBnClickedPermsOneLeft()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelList->GetFirstSelRow();
		CWaitCursor cwait;

		while (pRow) {
			MoveOneLeft(pRow);
			pRow = pRow->GetNextSelRow();
		}
		m_pAvailList->TakeCurrentRowAddSorted(m_pSelList, NULL);

	}NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CConfigurePermissionGroupsDlg, CNxDialog)
	ON_EVENT(CConfigurePermissionGroupsDlg, IDC_PERMS_PICK_LIST, 16, CConfigurePermissionGroupsDlg::SelChosenPermsPickList, VTS_DISPATCH)
	ON_EVENT(CConfigurePermissionGroupsDlg, IDC_PERMS_PICK_LIST, 18, CConfigurePermissionGroupsDlg::RequeryFinishedPermsPickList, VTS_I2)
	ON_EVENT(CConfigurePermissionGroupsDlg, IDC_PERMS_AVAIL_LIST, 3, CConfigurePermissionGroupsDlg::DblClickCellPermsAvailList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigurePermissionGroupsDlg, IDC_PERMS_SEL_LIST, 3, CConfigurePermissionGroupsDlg::DblClickCellPermsSelList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CConfigurePermissionGroupsDlg::SelChosenPermsPickList(LPDISPATCH lpRow)
{
	try {	
		LoadLists();
	}NxCatchAll(__FUNCTION__);
}

void CConfigurePermissionGroupsDlg::OnBnClickedGotoOther()
{
	if (m_bConfigureUser) {
		m_bConfigureUser = FALSE;
	}
	else {
		m_bConfigureUser = TRUE;
	}

	ReloadDialog();
}



void CConfigurePermissionGroupsDlg::RequeryFinishedPermsPickList(short nFlags)
{
	try {

		//set the cursel to be the first in the list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPickList->FindByColumn(slcID, m_nUserGroupID, NULL, FALSE);
		if (pRow) {
			m_pPickList->CurSel = pRow;
		}
		else {
			m_pPickList->CurSel = m_pPickList->GetFirstRow();
		}

		LoadLists();

	}NxCatchAll(__FUNCTION__);

}


void CConfigurePermissionGroupsDlg::LoadLists() 
{

	try {

		NXDATALIST2Lib::IRowSettingsPtr pPickRow = m_pPickList->CurSel;

		if (pPickRow) {

			long nPickID = VarLong(pPickRow->GetValue(slcID));

			CString strAvailWhere, strSelWhere;
			if(!m_bConfigureUser) {
				strAvailWhere.Format(" ID NOT IN (SELECT UserID FROM UserGroupDetailsT WHERE GroupID = %li) ", nPickID);
				strSelWhere.Format(" ID IN (SELECT UserID FROM UserGroupDetailsT WHERE GroupID = %li) ", nPickID);
			}
			else {
				strAvailWhere.Format(" ID NOT IN (SELECT GroupID FROM UserGroupDetailsT WHERE UserID = %li) ", nPickID);
				strSelWhere.Format(" ID IN (SELECT GroupID FROM UserGroupDetailsT WHERE UserID = %li) ", nPickID);
			}

			m_pAvailList->WhereClause = _bstr_t(strAvailWhere);
			m_pSelList->WhereClause = _bstr_t(strSelWhere);

			m_pAvailList->Requery();
			m_pSelList->Requery();
		}

	}NxCatchAll(__FUNCTION__);


}


void CConfigurePermissionGroupsDlg::MoveOneLeft(NXDATALIST2Lib::IRowSettingsPtr pRow) 
{
	long nAuditTransactionID = -1; 

	try {

		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pPickRow = m_pPickList->CurSel;

			if (pPickRow) {

				long nUserID, nGroupID;
				CString strUserName;

				if (!m_bConfigureUser) {
					nUserID = VarLong(pRow->GetValue(slcID));					
					strUserName = VarString(pRow->GetValue(slcName));
					nGroupID = VarLong(pPickRow->GetValue(slcID));					 	
				}
				else {
					nUserID = VarLong(pPickRow->GetValue(slcID));					
					strUserName = VarString(pPickRow->GetValue(slcName));
					nGroupID = VarLong(pRow->GetValue(slcID));					 	
				}
				
				CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, DWORD, DWORD> mapUser;				
				CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, DWORD, DWORD> mapOtherTemplates;
				CString strTableName, strSql, strBlank;				

				LoadPermissionsIntoMap(strTableName, nUserID, &mapUser, -1, TRUE);				
				LoadPermissionsIntoMap(strTableName, nUserID, &mapOtherTemplates, nGroupID, FALSE);

				//(e.lally 2010-10-14) PLID 40912 - Moved this to AFTER the load so we have an accurate mapUser for the permissions that were outside
				//the previously assigned groups. The load for mapOtherTemplates is excluding this group anyways, so it will already reflect the change.
				ExecuteParamSql("DELETE FROM UserGroupDetailsT WHERE UserID = {INT} AND GroupID = {INT}", nUserID, nGroupID);	

				//(e.lally 2010-10-14) PLID 40912 - We need to reapply the saved user permissions, so the first parameter is true.
				// (d.thompson 2013-03-25) - PLID 55847 - Converted to CGeneratePermissionUpdateBatch
				CGeneratePermissionUpdateBatch batch;
				batch.AddPermissionUpdate(true, nUserID, &mapOtherTemplates, &mapUser, &mapOtherTemplates, strUserName, nAuditTransactionID, strBlank);
				CSqlFragment sql = batch.ToSqlFragment();

				if (!sql.IsEmpty()) {
					// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
					NxAdo::PushMaxRecordsWarningLimit pmr(100000);
					ExecuteParamSql(sql);

					if (nAuditTransactionID != -1) {
						CommitAuditTransaction(nAuditTransactionID);
					}
				}

				//m_pAvailList->TakeRowAddSorted(pRow);
			}
		}

	}NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

}

void CConfigurePermissionGroupsDlg::MoveOneRight(NXDATALIST2Lib::IRowSettingsPtr pRow) 
{
	long nAuditTransactionID = -1;
	try {
		
		if (pRow) {
			NXDATALIST2Lib::IRowSettingsPtr pPickRow = m_pPickList->CurSel;

			if (pPickRow) {

				long nUserID, nGroupID;
				CString strUserName;

				if (!m_bConfigureUser) {
					nUserID = VarLong(pRow->GetValue(slcID));					
					strUserName = VarString(pRow->GetValue(slcName));					
					nGroupID = VarLong(pPickRow->GetValue(slcID));					 	
				}
				else {
					nUserID = VarLong(pPickRow->GetValue(slcID));					
					strUserName = VarString(pPickRow->GetValue(slcName));					
					nGroupID = VarLong(pRow->GetValue(slcID));					 	
				}
				
				ExecuteParamSql("INSERT INTO UserGroupDetailsT (UserID, GroupID) VALUES ({INT}, {INT})", nUserID, nGroupID);	

				//we have to reset their permissions per user
				CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, DWORD, DWORD> mapUser;
				CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, DWORD, DWORD> mapTemplates;
				CMap<EBuiltInObjectIDs, EBuiltInObjectIDs, DWORD, DWORD> mapOtherTemplates;
				CString strTableName, strBlank, strSql;				

				LoadPermissionsIntoMap(strTableName, nUserID, &mapUser, -1, TRUE);
				LoadPermissionsIntoMap(strTableName, nGroupID, &mapTemplates, -1, TRUE);
				LoadPermissionsIntoMap(strTableName, nUserID, &mapOtherTemplates, nGroupID, FALSE);

				//(e.lally 2010-10-14) PLID 40912 - We need to reapply the saved user permissions, so the first parameter is true.
				// (d.thompson 2013-03-25) - PLID 55847 - Converted to CGeneratePermissionUpdateBatch
				CGeneratePermissionUpdateBatch batch;
				batch.AddPermissionUpdate(true, nUserID, &mapTemplates, &mapUser, &mapOtherTemplates, strUserName, nAuditTransactionID, strBlank);
				CSqlFragment sql = batch.ToSqlFragment();
				
				if (!sql.IsEmpty()) {
					// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
					NxAdo::PushMaxRecordsWarningLimit pmr(100000);
					ExecuteParamSql(sql);

					if (nAuditTransactionID != -1) {
						CommitAuditTransaction(nAuditTransactionID);
					}
				}
				
				//m_pSelList->TakeRowAddSorted(pRow);
			}
		}

	}NxCatchAllCall(__FUNCTION__,
		if (nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		});

}

void CConfigurePermissionGroupsDlg::DblClickCellPermsAvailList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveOneRight(pRow);
			m_pSelList->TakeCurrentRowAddSorted(m_pAvailList, NULL);
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePermissionGroupsDlg::DblClickCellPermsSelList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			MoveOneLeft(pRow);
			m_pAvailList->TakeCurrentRowAddSorted(m_pSelList, NULL);
		}
	}NxCatchAll(__FUNCTION__);
}

void CConfigurePermissionGroupsDlg::OnCancel()
{
	try {
		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePermissionGroupsDlg::OnOK()
{
	try {
		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}