// BillableCPTCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BillableCPTCodesDlg.h"

// CBillableCPTCodesDlg dialog

// (j.jones 2011-03-28 10:31) - PLID 43012 - created

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum ListColumns {

	lcID = 0,
	lcCode,
	lcSubCode,
	lcDescription,
	lcStdFee,
	lcBillable,
};

IMPLEMENT_DYNAMIC(CBillableCPTCodesDlg, CNxDialog)

CBillableCPTCodesDlg::CBillableCPTCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBillableCPTCodesDlg::IDD, pParent)
{

}

CBillableCPTCodesDlg::~CBillableCPTCodesDlg()
{
}

void CBillableCPTCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_CPT, m_btnSelectOne);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_CPT, m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_CPT, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_CPT, m_btnUnselectAll);
}


BEGIN_MESSAGE_MAP(CBillableCPTCodesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_CPT, OnBtnSelectOneCpt)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_CPT, OnBtnSelectAllCpt)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_CPT, OnBtnUnselectOneCpt)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_CPT, OnBtnUnselectAllCpt)
	ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()


// CBillableCPTCodesDlg message handlers
BOOL CBillableCPTCodesDlg::OnInitDialog() 
{

	try {

		CNxDialog::OnInitDialog();

		m_UnselectedList = BindNxDataList2Ctrl(IDC_UNSELECTED_SERVICE_CODES, true);
		m_SelectedList = BindNxDataList2Ctrl(IDC_SELECTED_SERVICE_CODES, true);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSelectOne.AutoSet(NXB_DOWN);
		m_btnSelectAll.AutoSet(NXB_DDOWN);
		m_btnUnselectOne.AutoSet(NXB_UP);
		m_btnUnselectAll.AutoSet(NXB_UUP);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_EVENTSINK_MAP(CBillableCPTCodesDlg, CNxDialog)
ON_EVENT(CBillableCPTCodesDlg, IDC_UNSELECTED_SERVICE_CODES, 3, OnDblClickCellUnselectedServiceCodes, VTS_DISPATCH VTS_I2)
ON_EVENT(CBillableCPTCodesDlg, IDC_SELECTED_SERVICE_CODES, 3, CBillableCPTCodesDlg::OnDblClickCellSelectedServiceCodes, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CBillableCPTCodesDlg::OnDblClickCellUnselectedServiceCodes(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedList->PutCurSel(pRow);

		OnBtnSelectOneCpt();

	}NxCatchAll(__FUNCTION__);
}

void CBillableCPTCodesDlg::OnDblClickCellSelectedServiceCodes(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedList->PutCurSel(pRow);

		OnBtnUnselectOneCpt();

	}NxCatchAll(__FUNCTION__);
}

void CBillableCPTCodesDlg::OnBtnSelectOneCpt()
{
	try {

		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CBillableCPTCodesDlg::OnBtnSelectAllCpt()
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

	}NxCatchAll(__FUNCTION__);
}

void CBillableCPTCodesDlg::OnBtnUnselectOneCpt()
{
	try {

		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CBillableCPTCodesDlg::OnBtnUnselectAllCpt()
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

	}NxCatchAll(__FUNCTION__);
}

void CBillableCPTCodesDlg::OnOK()
{
	try {

		//save changes and warn on codes we changed to be billable

		CArray<long, long> aryBillableCodes;
		CArray<long, long> aryNonBillableCodes;

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		IRowSettingsPtr pRow = m_UnselectedList->GetFirstRow();
		while(pRow) {

			if(!VarBool(pRow->GetValue(lcBillable))) {
				//the old value was non-billable, but it's
				//now in the billable list, so track it
				aryBillableCodes.Add(VarLong(pRow->GetValue(lcID)));
			}

			pRow = pRow->GetNextRow();
		}

		pRow = m_SelectedList->GetFirstRow();
		while(pRow) {

			if(VarBool(pRow->GetValue(lcBillable))) {
				//the old value was billable, but it's
				//now in the non-billable list, so track it
				aryNonBillableCodes.Add(VarLong(pRow->GetValue(lcID)));
			}

			pRow = pRow->GetNextRow();
		}
		
		if(aryNonBillableCodes.GetSize() > 0) {

			//warn how many new codes are going to be non-billable
			CString strMessage;
			if(aryNonBillableCodes.GetSize() == 1) {
				strMessage = "You are about to mark one service code as not billable. "
					"This code will not be available to add to bills.\n\n"
					"Are you sure you wish to do this?";
			}
			else {
				strMessage.Format("You are about to mark %li new service codes as not billable. "
					"These codes will not be available to add to bills.\n\n"
					"Are you sure you wish to do this?", aryNonBillableCodes.GetSize());
			}
			if(IDNO == MessageBox(strMessage,"Practice",MB_YESNO|MB_ICONINFORMATION)) {
				return;
			}

			//for each code newly flagged as non-billable, we must warn if they can't
			//be flagged as non-billable

			//disallow if the code exists in a surgery
			{
				CSqlFragment sqlQuery("SELECT ServiceID "
					"FROM SurgeryDetailsT WHERE ServiceID IN ({INTARRAY}) "
					"GROUP BY ServiceID", aryNonBillableCodes);
				if(!ValidateCodes(sqlQuery,
					"The following service codes exist in at least one Surgery:",
					"These codes will be moved back to the Billable Service Codes list.\n"
					"You will need to remove these codes from all Surgeries before removing their billable status.",
					FALSE)) {
					
					//the bad codes will have already been unselected
					return;
				}
			}

			//disallow if the code exists in a preference card
			{
				CSqlFragment sqlQuery("SELECT ServiceID "
					"FROM PreferenceCardDetailsT WHERE ServiceID IN ({INTARRAY}) "
					"GROUP BY ServiceID", aryNonBillableCodes);
				if(!ValidateCodes(sqlQuery,
					"The following service codes exist in at least one Preference Card:",
					"These codes will be moved back to the Billable Service Codes list.\n"
					"You will need to remove these codes from all Preference Cards before removing their billable status.",
					FALSE)) {
					
					//the bad codes will have already been unselected
					return;
				}
			}
			
			//disallow if the code is linked to a product
			{
				CSqlFragment sqlQuery("SELECT CPTID AS ServiceID "
					"FROM ServiceToProductLinkT WHERE CPTID IN ({INTARRAY}) "
					"GROUP BY CPTID", aryNonBillableCodes);
				if(!ValidateCodes(sqlQuery,
					"The following service codes have inventory items linked to them:",
					"Services with inventory items linked to them cannot have their billable status removed.\n"
					"These codes will be moved back to the Billable Service Codes list.",
					FALSE)) {
					
					//the bad codes will have already been unselected
					return;
				}
			}
			
			//disallow if the code is linked to a procedure
			{
				CSqlFragment sqlQuery("SELECT ID AS ServiceID FROM ServiceT WHERE ProcedureID Is Not Null AND ID IN ({INTARRAY})", aryNonBillableCodes);
				if(!ValidateCodes(sqlQuery,
					"The following service codes are linked to procedures:",
					"Services linked to procedures cannot have their billable status removed.\n"
					"These codes will be moved back to the Billable Service Codes list.",
					FALSE)) {
					
					//the bad codes will have already been unselected
					return;
				}
			}
			
			//warn if the code is in any previous quote or previous case history (even if they have been billed)
			{
				CSqlFragment sqlQuery("SELECT ChargesT.ServiceID "
					"FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"WHERE Deleted = 0 AND Type = 11 AND ServiceID IN ({INTARRAY}) "
					"GROUP BY ServiceID", aryNonBillableCodes);
				if(!ValidateCodes(sqlQuery,
					"The following service codes exist in patients' quotes:",
					"If you remove the billable status from these codes, billing these quotes "
					"will skip adding these services.\n\n"
					"Are you sure you wish to mark these codes as not billable?",
					TRUE)) {
					
					//the bad codes will have already been unselected
					return;
				}
			}

			{
				CSqlFragment sqlQuery("SELECT ItemID AS ServiceID "
					"FROM CaseHistoryDetailsT "
					"WHERE ItemType = -2 AND ItemID IN ({INTARRAY}) "
					"GROUP BY ItemID", aryNonBillableCodes);
				if(!ValidateCodes(sqlQuery,
					"The following service codes exist in patients' case histories:",
					"If you remove the billable status from these codes, billing these case histories "
					"will skip adding these services.\n\n"
					"Are you sure you wish to mark these codes as not billable?",
					TRUE)) {
					
					//the bad codes will have already been unselected
					return;
				}
			}
		}

		CWaitCursor pWait;

		if(aryBillableCodes.GetSize() > 0) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE CPTCodeT SET Billable = 1 WHERE ID IN ({INTARRAY})", aryBillableCodes);
		}

		if(aryNonBillableCodes.GetSize() > 0) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE CPTCodeT SET Billable = 0 WHERE ID IN ({INTARRAY})", aryNonBillableCodes);
		}

		if(!strSqlBatch.IsEmpty()) {
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(100);
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			//send a tablechecker
			CClient::RefreshTable(NetUtils::CPTCodeT, -1);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

//validates whether the codes can be marked non-billable, due to a given rule
BOOL CBillableCPTCodesDlg::ValidateCodes(CSqlFragment sqlQuery, CString strWarningStart, CString strWarningEnd, BOOL bCanSave)
{
	try {

		CWaitCursor pWait;

		_RecordsetPtr rs = CreateParamRecordset("{SQL}", sqlQuery);
		CString strBadCodes = "";
		CArray<long, long> aryBadCodes;
		while(!rs->eof) {
			long nID = VarLong(rs->Fields->Item["ServiceID"]->Value);
			aryBadCodes.Add(nID);
			IRowSettingsPtr pFoundRow = m_SelectedList->FindByColumn(lcID, nID, m_SelectedList->GetFirstRow(), VARIANT_FALSE);
			if(pFoundRow) {
				CString strCode = VarString(pFoundRow->GetValue(lcCode));
				CString strSub = VarString(pFoundRow->GetValue(lcSubCode));
				CString strDesc = VarString(pFoundRow->GetValue(lcDescription));
				CString strCodeInfo;
				if(!strSub.IsEmpty()) {
					strCodeInfo.Format("%s %s - %s\n", strCode, strSub, strDesc);
				}
				else {
					strCodeInfo.Format("%s - %s\n", strCode, strDesc);
				}
				strBadCodes += strCodeInfo;
			}
			rs->MoveNext();
		}
		rs->Close();

		BOOL bFoundBadCodes = FALSE;

		if(!strBadCodes.IsEmpty()) {
			CString strMessage;
			strMessage.Format("%s\n\n%s\n%s", strWarningStart, strBadCodes, strWarningEnd);
			if(bCanSave) {
				//they are allowed to save, so this is a yes/no prompt
				if(IDNO == MessageBox(strMessage, "Practice", MB_YESNO|MB_ICONINFORMATION)) {
					MessageBox("These codes will be moved back to the Billable Service Codes list.", "Practice", MB_OK|MB_ICONINFORMATION);
					bFoundBadCodes = TRUE;
				}
			}
			else {
				//they are not allowed to save
				MessageBox(strMessage, "Practice", MB_OK|MB_ICONINFORMATION);
				bFoundBadCodes = TRUE;
			}
		}

		if(bFoundBadCodes) {
			//move the bad codes back to the Billable list
			for(int i=0; i<aryBadCodes.GetSize(); i++) {
				IRowSettingsPtr pFoundRow = m_SelectedList->FindByColumn(lcID, aryBadCodes.GetAt(i), m_SelectedList->GetFirstRow(), VARIANT_FALSE);
				if(pFoundRow) {
					m_UnselectedList->TakeRowAddSorted(pFoundRow);
				}
			}

			return FALSE;
		}

		return TRUE;

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}