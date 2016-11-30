// PayGroupsEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PayGroupsEditDlg.h"
#include "GlobalFinancialUtils.h"

// CPayGroupsEditDlg dialog

// (j.jones 2010-07-30 10:29) - PLID 39728 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum PayGroupListColumn {

	pglcID = 0,
	pglcOldName,	
	pglcName,
	pglcOldCategory,	// (r.wilson 2012-07-23) PLID 50488 
	pglcCategory,		// (r.wilson 2012-07-23) PLID 50488 
	pglcColor,	// (j.jones 2010-08-04 16:16) - PLID 39991
};

IMPLEMENT_DYNAMIC(CPayGroupsEditDlg, CNxDialog)

CPayGroupsEditDlg::CPayGroupsEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPayGroupsEditDlg::IDD, pParent)
{

}

CPayGroupsEditDlg::~CPayGroupsEditDlg()
{
}

void CPayGroupsEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ADD_PAY_GROUP, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_DELETE_PAY_GROUP, m_btnDelete);
}

BEGIN_MESSAGE_MAP(CPayGroupsEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_PAY_GROUP, OnBtnAddPayGroup)
	ON_BN_CLICKED(IDC_BTN_DELETE_PAY_GROUP, OnBtnDeletePayGroup)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, &CPayGroupsEditDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

// CPayGroupsEditDlg message handlers

BOOL CPayGroupsEditDlg::OnInitDialog() 
{	
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);		

		m_PayGroupList = BindNxDataList2Ctrl(IDC_PAY_GROUPS_LIST, false);

		// (j.jones 2010-08-04 16:16) - PLID 39991 - color the Copay group gray
		CString strColor;
		strColor.Format("CASE WHEN Name = 'Copay' THEN %li ELSE 0 END", RGB(127,127,127));
		IColumnSettingsPtr pColorCol = m_PayGroupList->GetColumn(pglcColor);
		pColorCol->PutFieldName(_bstr_t(strColor));
		m_PayGroupList->Requery();

		UpdateDeleteButtonEnabledState();

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPayGroupsEditDlg::OnBtnAddPayGroup()
{
	try {

		CString strName;
		if (IDOK == InputBoxLimited(this, "Enter a new Pay Group name:", strName, "",255,false,false,NULL)) {

			strName.TrimLeft();
			strName.TrimRight();

			// (j.jones 2010-08-04 16:16) - PLID 39991 - disallow anything like Copay
			if(strName.CompareNoCase("Copay") == 0) {
				AfxMessageBox("'Copay' is a system table and cannot be added more than once.");
				return;
			}

			if(strName.IsEmpty()) {
				AfxMessageBox("A Pay Group name cannot be blank.");
				return;
			}
			else if(strName.GetLength() > 255) {
				AfxMessageBox("You may not enter a Pay Group name longer than 255 characters.");
				return;
			}

			//does it exist already in the list?
			{
				IRowSettingsPtr pRow = m_PayGroupList->GetFirstRow();
				while(pRow) {

					CString strExistName = VarString(pRow->GetValue(pglcName), "");
					//case insensitive comparison
					if(strExistName.CompareNoCase(strName) == 0) {
						AfxMessageBox("There is already a Pay Group with this name.");
						//select it to show them
						m_PayGroupList->PutCurSel(pRow);
						UpdateDeleteButtonEnabledState();
						return;
					}

					pRow = pRow->GetNextRow();
				}
			}

			//add the row			
			IRowSettingsPtr pNewRow = m_PayGroupList->GetNewRow();
			pNewRow->PutValue(pglcID, (long)-1);
			pNewRow->PutValue(pglcOldName, _bstr_t(""));
			pNewRow->PutValue(pglcName, _bstr_t(strName));
			// (r.wilson 2012-07-23) PLID 50488 - Default the drop down column to {No Category}
			pNewRow->PutValue(pglcOldCategory, (long) PayGroupCategory::NoCategory);
			pNewRow->PutValue(pglcCategory, (long) PayGroupCategory::NoCategory);
			m_PayGroupList->AddRowSorted(pNewRow, NULL);
			m_PayGroupList->PutCurSel(pNewRow);	
			UpdateDeleteButtonEnabledState();
		}

	}NxCatchAll(__FUNCTION__);
}

void CPayGroupsEditDlg::OnBtnDeletePayGroup()
{
	try {

		IRowSettingsPtr pRow = m_PayGroupList->GetCurSel();
		if(pRow == NULL) {
			m_btnDelete.EnableWindow(FALSE);
			AfxMessageBox("You must first select a Pay Group before deleting.");
			return;
		}

		long nPayGroupID = VarLong(pRow->GetValue(pglcID), -1);

		long nCountUsing = 0;
		long nInsPartyCount = 0;
			
		//if a new group, we don't need to check if any codes are using it
		if(nPayGroupID != -1) {

			// (j.jones 2010-08-04 16:16) - PLID 39991 - disallow deleting the Copay group
			CString strName = VarString(pRow->GetValue(pglcName), "");
			if(strName.CompareNoCase("Copay") == 0) {
				AfxMessageBox("The Copay group is a system group and cannot be deleted.");
				return;
			}

			// (j.jones 2010-08-03 09:01) - PLID 39912 - include InsCoServicePayGroupLinkT when calculating,
			// we don't need to give a context-sensitive warning saying it's linked via an insurance company,
			// we only need to tell them how widely this group is used

			//are any services using this group?			
			_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ServiceT.ID) AS CountUsing "
				"FROM ServiceT "
				"WHERE ServiceT.PayGroupID = {INT} "
				"OR ServiceT.ID IN (SELECT ServiceID FROM InsCoServicePayGroupLinkT "
				"	WHERE PayGroupID = {INT})", nPayGroupID, nPayGroupID);
			if(!rs->eof) {
				nCountUsing = AdoFldLong(rs, "CountUsing", 0);
			}
			rs->Close();

			// (j.gruber 2010-08-04 15:56) - PLID 39729 - Pay groups for ins parties
			rs = CreateParamRecordset("SELECT Count(*) as CountUsing "
				" FROM InsuredPartyPayGroupsT WHERE PayGroupID = {INT} "
				" AND (Coinsurance IS NOT NULL OR CopayMoney IS NOT NULL "
				" OR CoPayPercentage IS NOT NULL) ", nPayGroupID);
			if (!rs->eof) {
				nInsPartyCount = AdoFldLong(rs, "CountUsing", 0);
			}
			rs->Close();
		}

		CString strWarn = "Are you sure you wish to delete this Pay Group?";
		CString strWarn2;

		if(nInsPartyCount == 1) {
			strWarn2 = "There is one insured party linked with this Pay Group. "
				"If you delete this Pay Group, the co-insurance and/or copay values will be deleted. \n\n"
				"Are you sure you wish to delete this Pay Group?";			
		}
		else if(nInsPartyCount > 0) {
			strWarn2.Format("There are %li insured parties linked with this Pay Group. "
				"If you delete this Pay Group, the co-insurance and/or copay values will be deleted.\n\n"				
				"Are you sure you wish to delete this Pay Group?", nInsPartyCount);			
		}

		if (!strWarn2.IsEmpty()) {
			//don't warn a second time unless for service codes
			strWarn = "";
			if(IDNO == MessageBox(strWarn2, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {				
				return;
			}
		}		

		if(nCountUsing == 1) {
			strWarn = "There is one service code linked with this Pay Group. "
				"If you delete this Pay Group, the service code will not be linked "
				"with any group.\n\n"
				"Are you sure you wish to delete this Pay Group?";
		}
		else if(nCountUsing > 0) {
			strWarn.Format("There are %li service codes linked with this Pay Group. "
				"If you delete this Pay Group, these service code will not be linked "
				"with any group.\n\n"
				"Are you sure you wish to delete this Pay Group?", nCountUsing);			
		}

		if (!strWarn.IsEmpty()) {
			if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		//don't need to keep track of new groups
		if(nPayGroupID != -1) {
			m_aryGroupsToDelete.Add(nPayGroupID);
		}
		
		m_PayGroupList->RemoveRow(pRow);
		m_PayGroupList->PutCurSel(NULL);
		UpdateDeleteButtonEnabledState();

	}NxCatchAll(__FUNCTION__);
}

void CPayGroupsEditDlg::OnOk()
{
	try {

		CWaitCursor pWait;

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		//first delete groups
		for(int i=0; i<m_aryGroupsToDelete.GetSize(); i++) {
			long nPayGroupID = (long)m_aryGroupsToDelete.GetAt(i);
			if(nPayGroupID != -1) {
				// (r.wilson 9/7/2012 ) PLID 50636 - Delete from this table as well
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM InsuranceCoPayGroupsDefaultsT WHERE PayGroupID = {INT}", nPayGroupID);
				// (j.jones 2010-08-03 08:58) - PLID 39912 - clear out InsCoServicePayGroupLinkT
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM InsCoServicePayGroupLinkT WHERE PayGroupID = {INT}", nPayGroupID);
				// (j.gruber 2010-08-04 16:02) - PLID 39729 - insured parties
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM InsuredPartyPayGroupsT WHERE PayGroupID = {INT}", nPayGroupID);
				//unlink any linked codes
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ServiceT SET PayGroupID = NULL WHERE PayGroupID = {INT}", nPayGroupID);
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ServicePayGroupsT WHERE ID = {INT}", nPayGroupID);
			}
		}

		IRowSettingsPtr pRow = m_PayGroupList->GetFirstRow();
		while(pRow) {

			long nPayGroupID = VarLong(pRow->GetValue(pglcID), -1);
			CString strOldName = VarString(pRow->GetValue(pglcOldName), "");
			CString strNewName = VarString(pRow->GetValue(pglcName), "");

			// (r.wilson 2012-07-23) PLID 50488 - We are only interested in this block if the drop down value was the edited value
							
			long nOldCategory = VarLong(pRow->GetValue(pglcOldCategory), -1);
			long nCategory = VarLong(pRow->GetValue(pglcCategory), -1);			

			_variant_t v_nCategory;
			v_nCategory.vt = VT_NULL;

			//save all new groups with a -1 ID
			if(nPayGroupID == -1) {							

				// (r.wilson 2012-07-23) PLID 50488 - We INSERT NULL for Category if {No Category} is selected ELSE We just put in the selected number value from the drop down				
				if(nCategory != PayGroupCategory::NoCategory){
					v_nCategory =  _variant_t(nCategory, VT_I4);
				}

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO ServicePayGroupsT (Name,Category) VALUES ({STRING},{VT_I4})", strNewName,v_nCategory);
			}
			//update the names of any groups we changed
			// (r.wilson 2012-07-23) PLID 50488 - this also includes the categories as well
			else if(strOldName != strNewName || nOldCategory != nCategory) {
				// (r.wilson 2012-07-23) PLID 50488 - We Update Category to NULL if {No Category} is selected ELSE We just put in the selected number value from the drop down				
				if(nCategory != PayGroupCategory::NoCategory){
					v_nCategory =  _variant_t(nCategory, VT_I4);
				}

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ServicePayGroupsT SET Name = {STRING}, Category = {VT_I4} WHERE ID = {INT}", strNewName, v_nCategory,nPayGroupID);
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			//send a tablechecker
			CClient::RefreshTable(NetUtils::ServicePayGroupsT);

			// (j.jones 2012-07-23 14:30) - PLID 51698 - Changing a category on a group would affect
			// CPT Code behavior. We also may have unlinked codes from pay groups. Send a tablechecker
			// for CPTCodeT, because while ServiceT is affected, only CPT Codes use pay groups.
			CClient::RefreshTable(NetUtils::CPTCodeT, -1);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CPayGroupsEditDlg, CNxDialog)
	ON_EVENT(CPayGroupsEditDlg, IDC_PAY_GROUPS_LIST, 2, OnSelChangedPayGroupsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPayGroupsEditDlg, IDC_PAY_GROUPS_LIST, 9, OnEditingFinishingPayGroupsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPayGroupsEditDlg, IDC_PAY_GROUPS_LIST, 8, OnEditingStartingPayGroupsList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

void CPayGroupsEditDlg::OnSelChangedPayGroupsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		IRowSettingsPtr pRow(lpNewSel);
		//enable/disable the delete button
		UpdateDeleteButtonEnabledState();

	}NxCatchAll(__FUNCTION__);
}

void CPayGroupsEditDlg::OnEditingFinishingPayGroupsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == pglcName) {

			CString strName = strUserEntered;

			strName.TrimLeft();
			strName.TrimRight();

			// (j.jones 2010-08-04 16:16) - PLID 39991 - disallow anything like Copay
			if(strName.CompareNoCase("Copay") == 0 && VarString(pRow->GetValue(pglcOldName)).CompareNoCase("Copay") != 0) {
				AfxMessageBox("'Copay' is a system table and cannot be added more than once.");
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}

			if(strName.IsEmpty()) {
				AfxMessageBox("A Pay Group name cannot be blank.");
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}
			else if(strName.GetLength() > 255) {
				AfxMessageBox("You may not enter a Pay Group name longer than 255 characters.");
				return;
			}

			//does it exist already in the list?
			{
				IRowSettingsPtr pExistRow = m_PayGroupList->GetFirstRow();
				while(pExistRow) {

					//skip this row
					if(pExistRow != pRow) {

						CString strExistName = VarString(pExistRow->GetValue(pglcName), "");
						//case insensitive comparison
						if(strExistName.CompareNoCase(strName) == 0) {
							AfxMessageBox("There is already a Pay Group with this name.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}

					pExistRow = pExistRow->GetNextRow();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-08-04 16:16) - PLID 39991 - moved delete button enabling to its own function
void CPayGroupsEditDlg::UpdateDeleteButtonEnabledState()
{
	IRowSettingsPtr pRow = m_PayGroupList->GetCurSel();

	if(pRow == NULL) {
		m_btnDelete.EnableWindow(FALSE);
	}
	else {
		// (j.jones 2010-08-04 16:16) - PLID 39991 - disallow deleting the Copay group
		CString strName = VarString(pRow->GetValue(pglcName), "");
		if(strName.CompareNoCase("Copay") == 0) {
			m_btnDelete.EnableWindow(FALSE);
		}
		else {
			m_btnDelete.EnableWindow(TRUE);
		}
	}
}

// (j.jones 2010-08-04 16:25) - PLID 39991 - added in order to prevent changing the Copay group name
void CPayGroupsEditDlg::OnEditingStartingPayGroupsList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			*pbContinue = FALSE;
			return;
		}

		// (j.jones 2010-08-04 16:16) - PLID 39991 - disallow changing the Copay group name
		CString strName = VarString(pRow->GetValue(pglcName), "");
		if(strName.CompareNoCase("Copay") == 0) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}


// (r.wilson 2012-07-23) PLID 50488 - If you click cancel then nothing will get saved
void CPayGroupsEditDlg::OnBnClickedCancel()
{
	try
	{	
		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}
