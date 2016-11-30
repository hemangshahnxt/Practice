// SecondaryANSIAllowedAdjConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SecondaryANSIAllowedAdjConfigDlg.h"
#include "GlobalFinancialUtils.h"

// CSecondaryANSIAllowedAdjConfigDlg dialog

// (j.jones 2010-02-03 08:49) - PLID 37159 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum CodeListColumns {

	clcID = 0,
	clcGroupCodeID,
	clcReasonCodeID,
};


IMPLEMENT_DYNAMIC(CSecondaryANSIAllowedAdjConfigDlg, CNxDialog)

CSecondaryANSIAllowedAdjConfigDlg::CSecondaryANSIAllowedAdjConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSecondaryANSIAllowedAdjConfigDlg::IDD, pParent)
{
	m_nGroupID = -1;
	m_bIsUB = FALSE;
}

CSecondaryANSIAllowedAdjConfigDlg::~CSecondaryANSIAllowedAdjConfigDlg()
{
}

void CSecondaryANSIAllowedAdjConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_ADJ_CODE, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_ADJ_CODE, m_btnRemove);
}


BEGIN_MESSAGE_MAP(CSecondaryANSIAllowedAdjConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_ADD_ADJ_CODE, OnBtnAddAdjCode)
	ON_BN_CLICKED(IDC_REMOVE_ADJ_CODE, OnBtnRemoveAdjCode)
END_MESSAGE_MAP()

// CSecondaryANSIAllowedAdjConfigDlg message handlers

BOOL CSecondaryANSIAllowedAdjConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
	
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);

		m_List = BindNxDataList2Ctrl(IDC_ALLOWED_AMT_CODES_LIST, false);

		// (j.jones 2010-09-23 15:09) - PLID 40653 - these are now stored in data
		//insert a blank line
		CString strGroupCodesSql = "SELECT -1 AS ID, '<Select a Group Code>' AS Name "
			"UNION SELECT ID, Code + ' - ' + Convert(nvarchar(4000), Description) AS Name FROM AdjustmentCodesT WHERE Type = 1 AND Inactive = 0 ORDER BY Name";
		//for reason codes, we want to display all the possible codes, even expired ones
		CString strReasonCodesSql = "SELECT -1 AS ID, '<Select a Reason Code>' AS Name "
			"UNION SELECT ID, Code + ' - ' + Convert(nvarchar(4000), Description) AS Name FROM AdjustmentCodesT WHERE Type = 2 AND Inactive = 0 ORDER BY Name";

		m_List->GetColumn(clcGroupCodeID)->PutComboSource((LPCTSTR)strGroupCodesSql);
		m_List->GetColumn(clcReasonCodeID)->PutComboSource((LPCTSTR)strReasonCodesSql);

		CString strFrom, strWhere;
		if(m_bIsUB) {
			strFrom = "UB_EbillingAllowedAdjCodesT";
			strWhere.Format("UBSetupID = %li", m_nGroupID);
		}
		else {
			strFrom = "HCFA_EbillingAllowedAdjCodesT";
			strWhere.Format("HCFASetupID = %li", m_nGroupID);
		}

		m_List->PutFromClause((LPCTSTR)strFrom);
		m_List->PutWhereClause((LPCTSTR)strWhere);

		m_List->Requery();

	}NxCatchAll("Error in CSecondaryANSIAllowedAdjConfigDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CSecondaryANSIAllowedAdjConfigDlg::OnOk()
{
	try {

		//Disallow saving if -1 is selected in either column
		//Disallow saving duplicates
		{
			IRowSettingsPtr pRow = m_List->GetFirstRow();
			while(pRow) {

				// (j.jones 2010-09-23 15:16) - PLID 40653 - these are now IDs
				long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID),-1);
				long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID),-1);

				if(nGroupCodeID == -1) {
					AfxMessageBox("At least one entry in your list has no Group Code selected.\n"
						"All entries must have both a Group Code and a Reason Code selected.");
					return;
				}

				if(nReasonCodeID == -1) {
					AfxMessageBox("At least one entry in your list has no Reason Code selected.\n"
						"All entries must have both a Group Code and a Reason Code selected.");
					return;
				}

				//make sure this is not a duplicated group
				IRowSettingsPtr pRow2 = m_List->GetFirstRow();
				while(pRow2) {

					//be sure not to check the current row
					if(pRow2 != pRow) {

						// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
						long nGroupCodeID2 = VarLong(pRow2->GetValue(clcGroupCodeID),-1);
						long nReasonCodeID2 = VarLong(pRow2->GetValue(clcReasonCodeID),-1);

						if(nGroupCodeID == nGroupCodeID2 && nReasonCodeID == nReasonCodeID2) {
							CString str, strGroupCode, strReasonCode;
							_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
								"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nGroupCodeID2, nReasonCodeID2);
							if(!rs->eof) {
								strGroupCode = AdoFldString(rs, "Code", "");
							}
							rs = rs->NextRecordset(NULL);
							if(!rs->eof) {
								strReasonCode = AdoFldString(rs, "Code", "");
							}
							rs->Close();
							str.Format("The following Group Code '%s' and Reason Code '%s' exist more than once in the list.\n"
								"Each code combination can only be used once. Please remove the duplicate code combination before saving.", strGroupCode, strReasonCode);
							AfxMessageBox(str);
							return;
						}
					}

					pRow2 = pRow2->GetNextRow();
				}

				pRow = pRow->GetNextRow();
			}
		}

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		//Delete any codes we removed
		int i=0;
		for(i=0; i<m_aryDeletedCodes.GetSize(); i++) {
			long nDeletedID = (long)m_aryDeletedCodes.GetAt(i);
			if(nDeletedID != -1) {
				if(m_bIsUB) {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM UB_EbillingAllowedAdjCodesT WHERE ID = {INT}", nDeletedID);
				}
				else {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM HCFA_EbillingAllowedAdjCodesT WHERE ID = {INT}", nDeletedID);
				}
			}
		}
		//do not clear the array, there could be an exception, plus we're closing this dialog anyways

		//Update any changes we made
		for(i=0; i<m_aryChangedCodes.GetSize(); i++) {
			long nChangedID = (long)m_aryChangedCodes.GetAt(i);
			if(nChangedID != -1) {

				IRowSettingsPtr pRow = m_List->FindByColumn(clcID, (long)nChangedID, m_List->GetFirstRow(), FALSE);
				if(pRow) {
					
					// (j.jones 2010-09-23 15:17) - PLID 40653 - these are now IDs
					long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID),-1);
					long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID),-1);

					if(nGroupCodeID == -1 || nReasonCodeID == -1) {						
						//should be impossible
						ASSERT(FALSE);
						ThrowNxException("Attempted to save invalid group/reason codes! (Changed)");
					}

					if(m_bIsUB) {
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE UB_EbillingAllowedAdjCodesT "
							"SET GroupCodeID = {INT}, ReasonCodeID = {INT} "
							"WHERE ID = {INT}", nGroupCodeID, nReasonCodeID, nChangedID);
					}
					else {
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE HCFA_EbillingAllowedAdjCodesT "
							"SET GroupCodeID = {INT}, ReasonCodeID = {INT} "
							"WHERE ID = {INT}", nGroupCodeID, nReasonCodeID, nChangedID);
					}
				}
				else {
					//don't throw an exception, just assert
					//this shouldn't be possible unless we changed a code,
					//then removed it, and didn't update m_aryChangedCodes
					ASSERT(FALSE);
				}
			}
		}
		//do not clear the array, there could be an exception, plus we're closing this dialog anyways

		//now create new codes
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {
			long nID = VarLong(pRow->GetValue(clcID), -1);
			if(nID == -1) {
				//new code combination

				// (j.jones 2010-09-23 15:18) - PLID 40653 - these are now IDs
				long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID),-1);
				long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID),-1);

				if(nGroupCodeID == -1 || nReasonCodeID == 1) {
					//should be impossible
					ASSERT(FALSE);
					ThrowNxException("Attempted to save invalid group/reason codes! (New)");
				}

				if(m_bIsUB) {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO UB_EbillingAllowedAdjCodesT "
						"(UBSetupID, GroupCodeID, ReasonCodeID) "
						"VALUES ({INT}, {INT}, {INT})", m_nGroupID, nGroupCodeID, nReasonCodeID);
				}
				else {
					AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO HCFA_EbillingAllowedAdjCodesT "
						"(HCFASetupID, GroupCodeID, ReasonCodeID) "
						"VALUES ({INT}, {INT}, {INT})", m_nGroupID, nGroupCodeID, nReasonCodeID);
				}
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {			
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CSecondaryANSIAllowedAdjConfigDlg::OnBtnAddAdjCode()
{
	try {

		IRowSettingsPtr pRow = m_List->GetNewRow();
		pRow->PutValue(clcID, (long)-1);
		pRow->PutValue(clcGroupCodeID, (long)-1);
		pRow->PutValue(clcReasonCodeID, (long)-1);
		m_List->AddRowAtEnd(pRow, NULL);
		m_List->PutCurSel(pRow);
		m_List->StartEditing(pRow, clcGroupCodeID);

	}NxCatchAll(__FUNCTION__);
}

void CSecondaryANSIAllowedAdjConfigDlg::OnBtnRemoveAdjCode()
{
	try {

		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select an entry from the list to delete.");
			return;
		}

		long nID = VarLong(pRow->GetValue(clcID), -1);		

		//is it an existing code?
		if(nID != -1) {

			//add to m_aryDeletedCodes, it should be impossible to already be in the list
			m_aryDeletedCodes.Add(nID);			
			
			//remove from m_aryChangedCodes, if it exists
			int i=0;
			for(i=m_aryChangedCodes.GetSize() - 1; i>=0; i--) {
				if(nID == (long)m_aryChangedCodes.GetAt(i)) {
					//remove from the list
					m_aryChangedCodes.RemoveAt(i);
				}
			}
		}

		//remove the row
		m_List->RemoveRow(pRow);

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CSecondaryANSIAllowedAdjConfigDlg, CNxDialog)
	ON_EVENT(CSecondaryANSIAllowedAdjConfigDlg, IDC_ALLOWED_AMT_CODES_LIST, 10, OnEditingFinishedAllowedAmtCodesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSecondaryANSIAllowedAdjConfigDlg, IDC_ALLOWED_AMT_CODES_LIST, 6, OnRButtonDownAllowedAmtCodesList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CSecondaryANSIAllowedAdjConfigDlg::OnEditingFinishedAllowedAmtCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//if they changed something, add to our changed list
		//(note: we can't tell if they changed something, then
		//changed it back to what it used to be - it's not 
		//really a big deal)
		if((nCol == clcGroupCodeID || nCol == clcReasonCodeID)
			&& VarLong(varOldValue) != VarLong(varNewValue)) {

			long nID = VarLong(pRow->GetValue(clcID), -1);
			if(nID != -1) {
				//they changed an existing code, so add it to our list
				int i=0;
				for(i=0; i<m_aryChangedCodes.GetSize(); i++) {
					if(nID == (long)m_aryChangedCodes.GetAt(i)) {
						//they already changed this item, return
						return;
					}
				}

				//if we're still here, add our ID to the list
				m_aryChangedCodes.Add(nID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CSecondaryANSIAllowedAdjConfigDlg::OnRButtonDownAllowedAmtCodesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_List->CurSel = pRow;

		//add an ability to remove the row
		enum {
			eRemoveFromList = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveFromList, "&Remove From List");

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveFromList) {

			OnBtnRemoveAdjCode();
		}

	}NxCatchAll(__FUNCTION__);
}
