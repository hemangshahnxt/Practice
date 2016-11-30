// AdvServiceCodePayGroupSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvServiceCodePayGroupSetupDlg.h"
#include "PayGroupsEditDlg.h"

// (j.jones 2010-08-02 12:04) - PLID 39912 - created

// CAdvServiceCodePayGroupSetupDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum InsCoComboColumns {

	icccID = 0,
	icccCompanyName,
	icccAddress,
};

enum PayGroupComboColumns {

	pgccID = 0,
	pgccName,
};

enum CodeListColumns {

	clcID = 0,
	clcCode,
	clcSubCode,
	clcDescription,
	clcDefPayGroup,
	clcInsPayGroup,
};

IMPLEMENT_DYNAMIC(CAdvServiceCodePayGroupSetupDlg, CNxDialog)

CAdvServiceCodePayGroupSetupDlg::CAdvServiceCodePayGroupSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvServiceCodePayGroupSetupDlg::IDD, pParent)
{

}

CAdvServiceCodePayGroupSetupDlg::~CAdvServiceCodePayGroupSetupDlg()
{
}

void CAdvServiceCodePayGroupSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_ONE_CODE, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_CODES, m_btnSelectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_CODE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_UNSELECT_ALL_CODES, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_CLOSE_ADV_PAY_GROUPS, m_btnClose);
	DDX_Control(pDX, IDC_BTN_EDIT_PAY_GROUPS, m_btnEditPayGroups);
	DDX_Control(pDX, IDC_BTN_APPLY_PAY_GROUPS, m_btnApply);
}


BEGIN_MESSAGE_MAP(CAdvServiceCodePayGroupSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SELECT_ONE_CODE, OnBtnSelectOneCode)
	ON_BN_CLICKED(IDC_SELECT_ALL_CODES, OnBtnSelectAllCodes)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_CODE, OnBtnUnselectOneCode)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_CODES, OnBtnUnselectAllCodes)
	ON_BN_CLICKED(IDC_BTN_CLOSE_ADV_PAY_GROUPS, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_EDIT_PAY_GROUPS, OnBtnEditPayGroups)
	ON_BN_CLICKED(IDC_BTN_APPLY_PAY_GROUPS, OnBtnApplyPayGroups)
END_MESSAGE_MAP()


// CAdvServiceCodePayGroupSetupDlg message handlers
BOOL CAdvServiceCodePayGroupSetupDlg::OnInitDialog() 
{	
	try {

		CNxDialog::OnInitDialog();

		m_btnSelectOne.AutoSet(NXB_DOWN);
		m_btnSelectAll.AutoSet(NXB_DDOWN);
		m_btnUnselectOne.AutoSet(NXB_UP);
		m_btnUnselectAll.AutoSet(NXB_UUP);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnApply.AutoSet(NXB_MODIFY);

		m_InsCoCombo = BindNxDataList2Ctrl(IDC_INSCO_COMBO, true);
		m_PayGroupCombo = BindNxDataList2Ctrl(IDC_PAY_GROUP_COMBO, true);
		m_UnselectedList = BindNxDataList2Ctrl(IDC_UNSELECTED_CPT_LIST, false);
		m_SelectedList = BindNxDataList2Ctrl(IDC_SELECTED_CPT_LIST, false);

		//add a record in the insco combo to update default service code values
		IRowSettingsPtr pInsCoRow = m_InsCoCombo->GetNewRow();
		pInsCoRow->PutValue(icccID, (long)-1);
		pInsCoRow->PutValue(icccCompanyName, " <Default Service Code Values>");
		pInsCoRow->PutValue(icccAddress, "");
		m_InsCoCombo->AddRowSorted(pInsCoRow, NULL);
		m_InsCoCombo->PutCurSel(pInsCoRow);

		//add a record in the pay group combo to clear values
		IRowSettingsPtr pPayGroupRow = m_PayGroupCombo->GetNewRow();
		pPayGroupRow->PutValue(pgccID, (long)-1);
		pPayGroupRow->PutValue(pgccName, " <Clear Pay Group Selection>");
		m_PayGroupCombo->AddRowSorted(pPayGroupRow, NULL);
		//don't auto-select this row

		OnSelChosenInscoCombo(m_InsCoCombo->GetCurSel());

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnSelectOneCode()
{
	try {

		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnSelectAllCodes()
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnUnselectOneCode()
{
	try {

		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnUnselectAllCodes()
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnClose()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CAdvServiceCodePayGroupSetupDlg, CNxDialog)
	ON_EVENT(CAdvServiceCodePayGroupSetupDlg, IDC_UNSELECTED_CPT_LIST, 3, OnDblClickCellUnselectedCptList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvServiceCodePayGroupSetupDlg, IDC_SELECTED_CPT_LIST, 3, OnDblClickCellSelectedCptList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvServiceCodePayGroupSetupDlg, IDC_INSCO_COMBO, 16, OnSelChosenInscoCombo, VTS_DISPATCH)
	ON_EVENT(CAdvServiceCodePayGroupSetupDlg, IDC_PAY_GROUP_COMBO, 16, OnSelChosenPayGroupCombo, VTS_DISPATCH)
	ON_EVENT(CAdvServiceCodePayGroupSetupDlg, IDC_INSCO_COMBO, 1, OnSelChangingInscoCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvServiceCodePayGroupSetupDlg, IDC_PAY_GROUP_COMBO, 1, OnSelChangingPayGroupCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CAdvServiceCodePayGroupSetupDlg::OnDblClickCellUnselectedCptList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedList->PutCurSel(pRow);
		OnBtnSelectOneCode();

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnDblClickCellSelectedCptList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedList->PutCurSel(pRow);
		OnBtnUnselectOneCode();

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnEditPayGroups()
{
	try {

		//store our current selection
		long nCurPayGroupID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_PayGroupCombo->GetCurSel();
		if(pRow) {
			nCurPayGroupID = VarLong(pRow->GetValue(pgccID),-1);
		}

		CPayGroupsEditDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			m_PayGroupCombo->Requery();

			//add a record in the pay group combo to clear values
			IRowSettingsPtr pPayGroupRow = m_PayGroupCombo->GetNewRow();
			pPayGroupRow->PutValue(pgccID, (long)-1);
			pPayGroupRow->PutValue(pgccName, " <Clear Pay Group Selection>");
			m_PayGroupCombo->AddRowSorted(pPayGroupRow, NULL);

			//retain our current selection, if it still exists
			m_PayGroupCombo->SetSelByColumn(pgccID, (long)nCurPayGroupID);
			//don't bother selecting a pay group if the selection is NULL

			//reselect our current insurance row in order to do a full reload
			OnSelChosenInscoCombo(m_InsCoCombo->GetCurSel());
		}

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnSelChosenInscoCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_InsCoCombo->GetFirstRow();
			m_InsCoCombo->PutCurSel(pRow);

			if(pRow == NULL) {
				//should not be possible, we have a -1 row
				ThrowNxException("No insurance row available");
			}
		}

		long nInsCoID = VarLong(pRow->GetValue(icccID), -1);
		IColumnSettingsPtr pUnselectedInsCol = m_UnselectedList->GetColumn(clcInsPayGroup);
		IColumnSettingsPtr pSelectedInsCol = m_SelectedList->GetColumn(clcInsPayGroup);

		CString strFromClause;

		if(nInsCoID == -1) {
			//hide the insurance column
			pUnselectedInsCol->PutColumnStyle(csVisible | csFixedWidth);
			pUnselectedInsCol->PutStoredWidth(0);			
			pUnselectedInsCol->PutFieldName("''");
			pSelectedInsCol->PutColumnStyle(csVisible | csFixedWidth);
			pSelectedInsCol->PutStoredWidth(0);			
			pSelectedInsCol->PutFieldName("''");

			strFromClause = "ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID";
		}
		else {
			//show the insurance column
			pUnselectedInsCol->PutColumnStyle(csVisible | csWidthData);
			pUnselectedInsCol->PutStoredWidth(130);			
			pUnselectedInsCol->PutFieldName("Coalesce(InsServicePayGroupsT.Name, '<Use Default Pay Group>')");
			pSelectedInsCol->PutColumnStyle(csVisible | csWidthData);
			pSelectedInsCol->PutStoredWidth(130);			
			pSelectedInsCol->PutFieldName("Coalesce(InsServicePayGroupsT.Name, '<Use Default Pay Group>')");

			strFromClause.Format("ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
				"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
				"LEFT JOIN (SELECT ServiceID, PayGroupID FROM InsCoServicePayGroupLinkT "
				"	WHERE InsuranceCoID = %li) AS InsPayGroupLinkQ ON ServiceT.ID = InsPayGroupLinkQ.ServiceID "
				"LEFT JOIN ServicePayGroupsT InsServicePayGroupsT ON InsPayGroupLinkQ.PayGroupID = InsServicePayGroupsT.ID", nInsCoID);
		}

		m_UnselectedList->PutFromClause(_bstr_t(strFromClause));
		m_SelectedList->PutFromClause(_bstr_t(strFromClause));

		m_UnselectedList->Requery();
		m_SelectedList->Clear();

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnSelChosenPayGroupCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select whatever the first row is
			pRow = m_PayGroupCombo->GetFirstRow();
			m_PayGroupCombo->PutCurSel(pRow);
		}

		//for now, this function does nothing

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnSelChangingInscoCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnSelChangingPayGroupCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

void CAdvServiceCodePayGroupSetupDlg::OnBtnApplyPayGroups()
{
	try {

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("You must first select service codes to apply a Pay Group to.");
			return;
		}

		IRowSettingsPtr pPayGroupRow = m_PayGroupCombo->GetCurSel();
		if(pPayGroupRow == NULL) {
			AfxMessageBox("You must select a Pay Group to apply to the selected service codes.");
			return;
		}

		long nPayGroupID = VarLong(pPayGroupRow->GetValue(pgccID), -1);

		IRowSettingsPtr pInsCoRow = m_InsCoCombo->GetCurSel();
		if(pInsCoRow == NULL) {
			AfxMessageBox("You must select an insurance company, or the default service code option, before applying changes.");
			return;
		}

		long nInsCoID = VarLong(pInsCoRow->GetValue(icccID), -1);

		//give a different message based on insurance or just default values
		CString strWarn;

		if(nInsCoID == -1) {
			//are they removing or adding a pay group?
			if(nPayGroupID == -1) {
				//removing
				strWarn.Format("This action will remove the default Pay Group for the service codes in the Selected list.\n\n"
					"Are you sure you wish to do this?");
			}
			else {
				//adding
				strWarn.Format("This action will set the %s Pay Group "
					"as the default Pay Group for the service codes in the Selected list.\n\n"
					"Are you sure you wish to do this?", VarString(pPayGroupRow->GetValue(pgccName), ""));
			}			
		}
		else {
			//are they removing or adding a pay group?
			if(nPayGroupID == -1) {
				//removing
				strWarn.Format("This action will will remove the Pay Group for the service codes in the Selected list "
					"for the %s insurance company.\n\n"
					"Are you sure you wish to do this?", VarString(pInsCoRow->GetValue(icccCompanyName), ""));
			}
			else {
				//adding
				strWarn.Format("This action will cause the %s Pay Group "
					"to be used for the service codes in the Selected list "
					"when the %s insurance company is billed.\n\n"
					"Are you sure you wish to do this?", VarString(pPayGroupRow->GetValue(pgccName), ""),
					VarString(pInsCoRow->GetValue(icccCompanyName), ""));
			}
		}

		if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//now apply the changes

		CWaitCursor pWait;

		CString strSqlBatch;

		IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
		while(pRow) {

			long nServiceID = VarLong(pRow->GetValue(clcID));

			if(nInsCoID == -1) {
				//are they removing or adding a pay group?
				if(nPayGroupID == -1) {
					//removing
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET PayGroupID = NULL WHERE ID = %li", nServiceID);
				}
				else {
					//adding
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET PayGroupID = %li WHERE ID = %li", nPayGroupID, nServiceID);
				}
			}
			else {

				//remove in all cases, then re-add if setting a new group
				AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsCoServicePayGroupLinkT "
					"WHERE InsuranceCoID = %li AND ServiceID = %li", nInsCoID, nServiceID);

				if(nPayGroupID != -1) {
					//add the new pay group
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsCoServicePayGroupLinkT "
						"(InsuranceCoID, ServiceID, PayGroupID) VALUES (%li, %li, %li)", nInsCoID, nServiceID, nPayGroupID);
				}
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			//this cannot be a parameterized, there's a limit of 2100 parameters, and we may
			//easily hit that limit in this screen
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(2000);
			ExecuteSqlBatch(strSqlBatch);

			// (j.jones 2012-07-23 14:30) - PLID 51698 - send a tablechecker for CPTCodeT,
			// because while we updated ServiceT, only CPT Codes changed
			CClient::RefreshTable(NetUtils::CPTCodeT, -1);
		}

		m_UnselectedList->Requery();
		m_SelectedList->Clear();

		if(nPayGroupID == -1) {
			AfxMessageBox("The selected service codes have had their Pay Group removed.");
		}
		else {
			AfxMessageBox("The selected service codes have been updated to use the new Pay Group.");
		}

	}NxCatchAll(__FUNCTION__);
}