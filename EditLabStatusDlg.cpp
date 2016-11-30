// EditLabStatusDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "EditLabStatusDlg.h"


// CEditLabStatusDlg dialog

//TES 12/4/2008 - PLID 32191 - Created, based off of CEditLabResultsDlg.
IMPLEMENT_DYNAMIC(CEditLabStatusDlg, CNxDialog)

CEditLabStatusDlg::CEditLabStatusDlg(CWnd* pParent)
	: CNxDialog(CEditLabStatusDlg::IDD, pParent)
{
	//TES 5/2/2011 - PLID 43428 - Default to editing the Result status (the original purpose of this dialog.
	m_bEditOrderStatus = false;
}

CEditLabStatusDlg::~CEditLabStatusDlg()
{
}

void CEditLabStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbClose);
	DDX_Control(pDX, IDC_EDIT_RESULT_STATUS, m_nxbEdit);
	DDX_Control(pDX, IDC_DELETE_STATUS, m_nxbDelete);
	DDX_Control(pDX, IDC_ADD_STATUS, m_nxbAdd);
	
}


BEGIN_MESSAGE_MAP(CEditLabStatusDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_STATUS, &CEditLabStatusDlg::OnAddStatus)
	ON_BN_CLICKED(IDC_DELETE_STATUS, &CEditLabStatusDlg::OnDeleteStatus)
	ON_BN_CLICKED(IDC_EDIT_RESULT_STATUS, &CEditLabStatusDlg::OnEditStatus)
END_MESSAGE_MAP()

enum StatusListColumns {
	slcID = 0,
	slcDescription = 1,
	slcHL7Flag = 2,
	slcActionCode = 3, //TES 9/3/2013 - PLID 58402
};

using namespace NXDATALIST2Lib;
using namespace ADODB;


// CEditLabStatusDlg message handlers
BOOL CEditLabStatusDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try{
		//TES 12/4/2008 - PLID 32191 - Set up our NxIconButtons.
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbEdit.AutoSet(NXB_MODIFY);
		m_nxbDelete.AutoSet(NXB_DELETE);
		m_nxbAdd.AutoSet(NXB_NEW);
	
		//TES 12/4/2008 - PLID 32191 - Load our status list
		m_pStatusList = BindNxDataList2Ctrl(IDC_RESULT_STATUS_LIST, false);
		//TES 5/2/2011 - PLID 43428 - Check which table to use
		if(m_bEditOrderStatus) {
			m_pStatusList->FromClause = _bstr_t("LabOrderStatusT");
			//TES 9/3/2013 - PLID 58402 - Order statuses don't have an ActionCode, so hide that column.
			IColumnSettingsPtr pCol;
			pCol = m_pStatusList->GetColumn(slcActionCode);
			pCol->PutColumnStyle(csVisible | csFixedWidth);
			pCol->PutStoredWidth(0);
			pCol->FieldName = _bstr_t("NULL");
		}
		else {
			m_pStatusList->FromClause = _bstr_t("LabResultStatusT");
		}
		m_pStatusList->Requery();

		return TRUE; 
	}NxCatchAll("Error in CEditLabStatusDlg::OnInitDialog()");
	return FALSE;
}

void CEditLabStatusDlg::OnOK() 
{
	CDialog::OnOK();
}

void CEditLabStatusDlg::OnCancel() 
{
	CDialog::OnCancel();
}


void CEditLabStatusDlg::OnAddStatus()
{
	try{
		//TES 12/4/2008 - PLID 32191 - Add a new row into the list and set it to be edited
		IRowSettingsPtr pRow;
		pRow = m_pStatusList->GetNewRow();

		//TES 12/16/2008 - PLID 32191 - The new value will be written to data immediately, so we need to make sure
		// that it's not a duplicate.
		CString strNewName = "Enter New Status";
		BOOL bFound = (m_pStatusList->FindByColumn(slcDescription, _bstr_t(strNewName), NULL, VARIANT_FALSE) != NULL);
		for(int i = 1; i <= 100 && bFound; i++) {
			strNewName.Format("Enter New Status%i", i);
			bFound = (m_pStatusList->FindByColumn(slcDescription, _bstr_t(strNewName), NULL, VARIANT_FALSE) != NULL);
		}
		if(bFound) {
			//TES 12/16/2008 - PLID 32391 - They have more than 100 "Enter New Status"s already, there's no valid reason
			// for them to do that.
			AfxThrowNxException("Could not generate new Result Status name!");
		}

		//TES 5/2/2011 - PLID 43428 - Check which table we're editing
		long nNewID = -1;
		if(m_bEditOrderStatus) {
			_RecordsetPtr rsNewRecord = CreateParamRecordset("SET NOCOUNT ON\r\n"
				"INSERT INTO LabOrderStatusT (Description) "
				"VALUES ({STRING}) \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT Convert(int,SCOPE_IDENTITY()) AS NewID",
				strNewName);
			nNewID = AdoFldLong(rsNewRecord, "NewID");
		}
		else {
			nNewID = NewNumber("LabResultStatusT", "ID");
			ExecuteParamSql("INSERT INTO LabResultStatusT (ID, Description) VALUES ({INT}, {STRING})",
				nNewID, strNewName);
		}

		pRow->PutValue(slcID, nNewID);
		pRow->PutValue(slcDescription, _variant_t(strNewName));
		pRow->PutValue(slcHL7Flag, _variant_t(""));
		//TES 9/3/2013 - PLID 58402 - Default to no action
		pRow->PutValue(slcActionCode, (long)racNone);
		
		m_pStatusList->AddRowBefore(pRow, NULL);
		m_pStatusList->SetSelByColumn(0, nNewID);
		m_pStatusList->StartEditing(m_pStatusList->GetCurSel(), 1);
	}NxCatchAll("Error in CEditLabStatusDlg::OnAddResult()");
}

void CEditLabStatusDlg::OnDeleteStatus()
{
	try{
		//TES 12/4/2008 - PLID 32191 - check to see that something is selected
		IRowSettingsPtr pCurSel = m_pStatusList->GetCurSel();
	
		if (pCurSel != NULL) {
			if (IDYES == MessageBox("Are you sure you want to delete this status?", "NexTech Practice", MB_YESNO|MB_ICONQUESTION)) {
	
				long nDelID = VarLong(pCurSel->GetValue(slcID));

				//TES 12/4/2008 - PLID 32191 - Check to see that no results use this status.

				_RecordsetPtr rs;
				//TES 5/2/2011 - PLID 43428 - Check which table we're editing
				if(m_bEditOrderStatus) {
					rs = CreateRecordset("SELECT Count(ID) as nCount FROM LabsT WHERE OrderStatusID = %li", nDelID);
				}
				else {
					rs = CreateRecordset("SELECT Count(ResultID) as nCount FROM LabResultsT WHERE StatusID = %li", nDelID);
				}
				long nCount  = AdoFldLong(rs, "nCount");
				
				if (nCount > 0 ) {
					//TES 12/4/2008 - PLID 32191 - We can't delete
					CString str;
					//TES 5/2/2011 - PLID 43428 - Check which table we're editing
					str.Format("There are %li %s using this status, you cannot delete it.", nCount, CString(m_bEditOrderStatus?"labs":"results"));
					MessageBox(str);
				}
				else {
					//TES 12/4/2008 - PLID 32191 - We can delete it!
					//TES 5/2/2011 - PLID 43428 - Check which table we're editing
					ExecuteSql("DELETE FROM %s WHERE ID = %li", CString(m_bEditOrderStatus?"LabOrderStatusT":"LabResultStatusT"),nDelID);
					m_pStatusList->RemoveRow(pCurSel);
				}
			}
			else {
				//TES 12/4/2008 - PLID 32191 - They don't want to delete, so do nothing
			}
		}
		else {
			//TES 12/4/2008 - PLID 32191 - They haven't selected anything
			MessageBox("Please select a status to delete");
		}
	}NxCatchAll("Error in CEditLabStatusDlg::OnDeleteStatus()");
}

void CEditLabStatusDlg::OnEditStatus()
{
	try{
		//TES 12/4/2008 - PLID 32191 - Check to see that something is selected

		IRowSettingsPtr pCurSel = m_pStatusList->GetCurSel();

		if (pCurSel != NULL) {

			//TES 12/4/2008 - PLID 32191 - Start editing.
			m_pStatusList->StartEditing(pCurSel,1);
		}
		else {
			MessageBox("Please select a status to edit");
		}
	}NxCatchAll("Error in CEditLabStatusDlg::OnEditStatus()");
}
BEGIN_EVENTSINK_MAP(CEditLabStatusDlg, CNxDialog)
	ON_EVENT(CEditLabStatusDlg, IDC_RESULT_STATUS_LIST, 9, CEditLabStatusDlg::OnEditingFinishingResultStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditLabStatusDlg, IDC_RESULT_STATUS_LIST, 10, CEditLabStatusDlg::OnEditingFinishedResultStatusList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CEditLabStatusDlg::OnEditingFinishingResultStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		//TES 12/4/2008 - PLID 32191 - If they're not committing this change, we don't
		// have to validate it.
		if(!(*pbCommit)) {
			return;
		}
		IRowSettingsPtr pRow(lpRow);
		switch (nCol) {
			case slcDescription:
			{
				//TES 12/4/2008 - PLID 32191 - Make sure it's unique and non-blank.
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if (strTemp.IsEmpty()) {
					MessageBox("Please enter a valid status description .");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
				//TES 5/2/2011 - PLID 43428 - Check which table we're editing
				if(!IsRecordsetEmpty("SELECT Description FROM %s WHERE ID <> %li AND Description = '%s'",
					CString(m_bEditOrderStatus?"LabOrderStatusT":"LabResultStatusT"), VarLong(pRow->GetValue(slcID)), _Q(strTemp))) {
					MessageBox("The status you entered already exists in the list. Please enter a unique status.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
			}
			break;
			case slcHL7Flag:
			{
				//TES 12/4/2008 - PLID 32191 - Make sure that it is either blank or
				// unique.
				CString strTemp = strUserEntered;
				strTemp.TrimLeft();
				strTemp.TrimRight();
				if(!strTemp.IsEmpty()) {
					//TES 5/2/2011 - PLID 43428 - Check which table we're editing
					if(!IsRecordsetEmpty("SELECT HL7Flag FROM %s WHERE ID <> %li AND HL7Flag = '%s'",
						CString(m_bEditOrderStatus?"LabOrderStatusT":"LabResultStatusT"), VarLong(pRow->GetValue(slcID)), _Q(strTemp))) {
						MessageBox("The HL7 Flag you entered is already being used by a different status. Please enter a unique HL7 Flag.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
			}
			break;

			default:
			break;
		}
		
	}NxCatchAll("Error in CEditLabStatusDlg::OnEditingFinishingResultStatusList()");
}

void CEditLabStatusDlg::OnEditingFinishedResultStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		//TES 12/4/2008 - PLID 32191 - If the user doesn't want to commit this change, 
		// don't.
		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if (pRow != NULL) {
			long nID;
			nID = VarLong(pRow->GetValue(slcID));

			switch (nCol){
			case slcDescription:
			{
				//TES 5/2/2011 - PLID 43428 - Check which table we're editing
				ExecuteSql("UPDATE %s SET Description = '%s' WHERE ID = %li", 
					CString(m_bEditOrderStatus?"LabOrderStatusT":"LabResultStatusT"), _Q(VarString(varNewValue)), nID);
			}
			break;
			case slcHL7Flag:
			{
				//TES 5/2/2011 - PLID 43428 - Check which table we're editing
				ExecuteSql("UPDATE %s SET HL7Flag = '%s' WHERE ID = %li", 
					CString(m_bEditOrderStatus?"LabOrderStatusT":"LabResultStatusT"), _Q(VarString(varNewValue)), nID);
			}
			break;
			case slcActionCode:
			{
				//TES 9/3/2013 - PLID 58402 - Action codes don't exist on Order Statuses, so this should never happen
				if(m_bEditOrderStatus) {
					ThrowNxException("Attempt to set Action Code on Order Status!");
				}
				//TES 9/3/2013 - PLID 58402 - Save the new value to data.
				ExecuteParamSql("UPDATE LabResultStatusT SET ActionCode = {VT_I4} WHERE ID = {INT}", varNewValue, nID);
			}
			default:
				break; //do nothing
			}
		}
		
	}NxCatchAll("Error in CEditLabStatusDlg::OnEditingFinishedResultStatusList()");
}