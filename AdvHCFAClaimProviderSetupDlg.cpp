// AdvHCFAClaimProviderSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvHCFAClaimProviderSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-08-01 09:52) - PLID 30918 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdvHCFAClaimProviderSetupDlg dialog

enum InsuranceListColumns {

	ilcID = 0,
	ilcName,
	ilcHCFAGroupID,
};

//each combo is the same, so there is only one enumeration
enum ProviderComboColumn {

	pccID = 0,
	pccName,
};

CAdvHCFAClaimProviderSetupDlg::CAdvHCFAClaimProviderSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvHCFAClaimProviderSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvHCFAClaimProviderSetupDlg)
		m_nColor = OLE_COLOR(0x00AAAAFF);
		m_nDefaultHCFAGroupID = -1;
		m_nDefaultInsCoID = -1;
		m_nDefaultProviderID = -1;
		m_nDefault2010AA = -1;
		m_nDefault2310B = -1;
		m_nDefaultBox33 = -1;
		m_nDefaultBox24J = -1;
	//}}AFX_DATA_INIT
}


void CAdvHCFAClaimProviderSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvHCFAClaimProviderSetupDlg)
	DDX_Control(pDX, IDC_DEFAULT_PROVIDER_LABEL, m_nxstaticDefaultProviderLabel);
	DDX_Control(pDX, IDC_BTN_ADV_CLAIM_PROV_UNSELECT_ONE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_ADV_CLAIM_PROV_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_ADV_CLAIM_PROV_SELECT_ONE, m_btnSelectOne);
	DDX_Control(pDX, IDC_BTN_ADV_CLAIM_PROV_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_APPLY_CLAIM_PROV, m_btnApply);
	DDX_Control(pDX, IDC_BTN_ADV_CLAIM_PROV_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_ADV_CLAIM_PROV_COLOR, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvHCFAClaimProviderSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvHCFAClaimProviderSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADV_CLAIM_PROV_CLOSE, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_ADV_CLAIM_PROV_SELECT_ONE, OnBtnAdvClaimProvSelectOne)
	ON_BN_CLICKED(IDC_BTN_ADV_CLAIM_PROV_SELECT_ALL, OnBtnAdvClaimProvSelectAll)
	ON_BN_CLICKED(IDC_BTN_ADV_CLAIM_PROV_UNSELECT_ALL, OnBtnAdvClaimProvUnselectAll)
	ON_BN_CLICKED(IDC_BTN_ADV_CLAIM_PROV_UNSELECT_ONE, OnBtnAdvClaimProvUnselectOne)
	ON_BN_CLICKED(IDC_BTN_APPLY_CLAIM_PROV, OnBtnApplyClaimProv)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvHCFAClaimProviderSetupDlg message handlers

BOOL CAdvHCFAClaimProviderSetupDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		SetColor(m_nColor);		

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnSelectAll.AutoSet(NXB_RRIGHT);

		// (a.walling 2008-10-03 16:09) - PLID 31588 - These are NxDatalist2s, must call BindNxDataList2Ctrl
		m_UnselectedList = BindNxDataList2Ctrl(IDC_ADV_CLAIM_PROV_UNSELECTED_LIST, true);
		m_SelectedList = BindNxDataList2Ctrl(IDC_ADV_CLAIM_PROV_SELECTED_LIST, false);

		m_MainProviderCombo = BindNxDataList2Ctrl(this, IDC_MAIN_PROVIDER_COMBO, GetRemoteData(), true);
		m_2010AACombo = BindNxDataList2Ctrl(this, IDC_2010AA_COMBO, GetRemoteData(), true);
		m_2310BCombo = BindNxDataList2Ctrl(this, IDC_2310B_COMBO, GetRemoteData(), true);
		m_Box33Combo = BindNxDataList2Ctrl(this, IDC_BOX33_COMBO, GetRemoteData(), true);
		m_Box24JCombo = BindNxDataList2Ctrl(this, IDC_BOX24J_COMBO, GetRemoteData(), true);

		//add "default" rows
		IRowSettingsPtr pRow = m_2010AACombo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_2010AACombo->AddRowSorted(pRow, NULL);

		//see if we need to select a default
		if(m_nDefault2010AA != -1) {
			m_2010AACombo->SetSelByColumn(pccID, m_nDefault2010AA);
		}
		if(m_2010AACombo->GetCurSel() == NULL) {
			m_2010AACombo->PutCurSel(pRow);
		}

		pRow = m_2310BCombo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_2310BCombo->AddRowSorted(pRow, NULL);

		//see if we need to select a default
		if(m_nDefault2310B != -1) {
			m_2310BCombo->SetSelByColumn(pccID, m_nDefault2310B);
		}
		if(m_2310BCombo->GetCurSel() == NULL) {
			m_2310BCombo->PutCurSel(pRow);
		}

		pRow = m_Box33Combo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_Box33Combo->AddRowSorted(pRow, NULL);
		
		//see if we need to select a default
		if(m_nDefaultBox33 != -1) {
			m_Box33Combo->SetSelByColumn(pccID, m_nDefaultBox33);
		}
		if(m_Box33Combo->GetCurSel() == NULL) {
			m_Box33Combo->PutCurSel(pRow);
		}

		pRow = m_Box24JCombo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_Box24JCombo->AddRowSorted(pRow, NULL);
		
		//see if we need to select a default
		if(m_nDefaultBox24J != -1) {
			m_Box24JCombo->SetSelByColumn(pccID, m_nDefaultBox24J);
		}
		if(m_Box24JCombo->GetCurSel() == NULL) {
			m_Box24JCombo->PutCurSel(pRow);
		}

		//the provider's "default" claim provider label is blank
		//for the moment since we don't have one selected
		SetDlgItemText(IDC_DEFAULT_PROVIDER_LABEL, "");
	
	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvHCFAClaimProviderSetupDlg::OnOK() 
{
	try {
	
		CNxDialog::OnOK();

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnOK");
}

void CAdvHCFAClaimProviderSetupDlg::OnCancel() 
{
	try {
	
		CNxDialog::OnCancel();

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnCancel");
}

void CAdvHCFAClaimProviderSetupDlg::OnBtnClose() 
{
	try {
	
		CNxDialog::OnOK();

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnBtnClose");
}

BEGIN_EVENTSINK_MAP(CAdvHCFAClaimProviderSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvHCFAClaimProviderSetupDlg)
	ON_EVENT(CAdvHCFAClaimProviderSetupDlg, IDC_ADV_CLAIM_PROV_UNSELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellAdvClaimProvUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvHCFAClaimProviderSetupDlg, IDC_ADV_CLAIM_PROV_SELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellAdvClaimProvSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvHCFAClaimProviderSetupDlg, IDC_MAIN_PROVIDER_COMBO, 16 /* SelChosen */, OnSelChosenMainProviderCombo, VTS_DISPATCH)
	ON_EVENT(CAdvHCFAClaimProviderSetupDlg, IDC_MAIN_PROVIDER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedMainProviderCombo, VTS_I2)
	ON_EVENT(CAdvHCFAClaimProviderSetupDlg, IDC_ADV_CLAIM_PROV_UNSELECTED_LIST, 18 /* RequeryFinished */, OnRequeryFinishedAdvClaimProvUnselectedList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvHCFAClaimProviderSetupDlg::OnDblClickCellAdvClaimProvUnselectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedList->PutCurSel(pRow);
		OnBtnAdvClaimProvSelectOne();

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnDblClickCellAdvClaimProvUnselectedList");
}

void CAdvHCFAClaimProviderSetupDlg::OnDblClickCellAdvClaimProvSelectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedList->PutCurSel(pRow);
		OnBtnAdvClaimProvUnselectOne();

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnDblClickCellAdvClaimProvSelectedList");
}

void CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvSelectOne() 
{
	try {

		if(m_UnselectedList->GetCurSel() == NULL) {
			return;
		}

		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvSelectOne");
}

void CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvSelectAll()
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvSelectAll");
}

void CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvUnselectAll()
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvUnselectAll");
}

void CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvUnselectOne()
{
	try {

		if(m_SelectedList->GetCurSel() == NULL) {
			return;
		}

		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnBtnAdvClaimProvUnselectOne");
}

void CAdvHCFAClaimProviderSetupDlg::OnBtnApplyClaimProv()
{
	try {

		//require a provider to be selected		
		IRowSettingsPtr pProvRow = m_MainProviderCombo->GetCurSel();
		if(pProvRow == NULL) {
			AfxMessageBox("You must have a main provider selected in order to define claim providers.");
			return;
		}

		long nCountCompanies = m_SelectedList->GetRowCount();
		if(nCountCompanies == 0) {
			AfxMessageBox("You must place at least one insurance company in the selected list in order to define claim providers.");
			return;
		}

		long nProviderID = VarLong(pProvRow->GetValue(pccID), -1);
		CString strProviderName = VarString(pProvRow->GetValue(pccName), "<Use Default Provider>");

		long nANSI_2010AA_ProviderID = -1;
		long nANSI_2310B_ProviderID = -1;
		long nBox33_ProviderID = -1;
		long nBox24J_ProviderID = -1;

		CString str2010AA_ProviderID = "NULL";
		CString str2310B_ProviderID = "NULL";
		CString strBox33_ProviderID = "NULL";
		CString strBox24J_ProviderID = "NULL";

		CString str2010AA_ProviderName = "<Use Default Provider>";
		CString str2310B_ProviderName = "<Use Default Provider>";
		CString strBox33_ProviderName = "<Use Default Provider>";
		CString strBox24J_ProviderName = "<Use Default Provider>";

		IRowSettingsPtr pRow = m_2010AACombo->GetCurSel();
		if(pRow) {
			nANSI_2010AA_ProviderID = VarLong(pRow->GetValue(pccID),-1);
			str2010AA_ProviderName = VarString(pRow->GetValue(pccName), "<Use Default Provider>");
			str2010AA_ProviderName.TrimLeft();
		}
		pRow = m_2310BCombo->GetCurSel();
		if(pRow) {
			nANSI_2310B_ProviderID = VarLong(pRow->GetValue(pccID),-1);
			str2310B_ProviderName = VarString(pRow->GetValue(pccName), "<Use Default Provider>");
			str2310B_ProviderName.TrimLeft();
		}
		pRow = m_Box33Combo->GetCurSel();
		if(pRow) {
			nBox33_ProviderID = VarLong(pRow->GetValue(pccID),-1);
			strBox33_ProviderName = VarString(pRow->GetValue(pccName), "<Use Default Provider>");
			strBox33_ProviderName.TrimLeft();
		}
		pRow = m_Box24JCombo->GetCurSel();
		if(pRow) {
			nBox24J_ProviderID = VarLong(pRow->GetValue(pccID),-1);
			strBox24J_ProviderName = VarString(pRow->GetValue(pccName), "<Use Default Provider>");
			strBox24J_ProviderName.TrimLeft();
		}

		if(nANSI_2010AA_ProviderID != -1) {
			str2010AA_ProviderID.Format("%li", nANSI_2010AA_ProviderID);
		}
		if(nANSI_2310B_ProviderID != -1) {
			str2310B_ProviderID.Format("%li", nANSI_2310B_ProviderID);
		}
		if(nBox33_ProviderID != -1) {
			strBox33_ProviderID.Format("%li", nBox33_ProviderID);
		}
		if(nBox24J_ProviderID != -1) {
			strBox24J_ProviderID.Format("%li", nBox24J_ProviderID);
		}

		//now before we try to save, warn the user about what it is they are trying to do
		CString strWarn;
		strWarn.Format("You are about to apply the following changes for the provider '%s' for %li insurance %s.\r\n\r\n"
			"ANSI 2010AA Billing Provider: %s\r\n"
			"ANSI 2310B Rendering Provider: %s\r\n"
			"HCFA Box 33 Provider: %s\r\n"
			"HCFA Box 24J Provider: %s\r\n\r\n"
			"Are you sure you wish to continue with this change?",
			strProviderName, nCountCompanies, nCountCompanies == 1 ? "company" : "companies",
			str2010AA_ProviderName, str2310B_ProviderName, strBox33_ProviderName, strBox24J_ProviderName);

		if(IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		CWaitCursor pWait;

		//now apply the changes in a batch

		CString strSqlBatch;
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nClaimProviderRecordID INT");

		IRowSettingsPtr pInsCoRow = m_SelectedList->GetFirstRow();
		while(pInsCoRow) {

			long nInsCoID = VarLong(pInsCoRow->GetValue(ilcID), -1);

			//determine in the batch execute whether to update or insert a new row		
			
			AddStatementToSqlBatch(strSqlBatch, "SET @nClaimProviderRecordID = "
				"(SELECT ID FROM HCFAClaimProvidersT "
				"WHERE ProviderID = %li AND InsuranceCoID = %li)", nProviderID, nInsCoID);
			
			AddStatementToSqlBatch(strSqlBatch,
				"IF @nClaimProviderRecordID Is Not Null \r\n"
				""
					"UPDATE HCFAClaimProvidersT SET "
					"ANSI_2010AA_ProviderID = %s, ANSI_2310B_ProviderID = %s, Box33_ProviderID = %s, Box24J_ProviderID = %s "
					"WHERE ID = @nClaimProviderRecordID \r\n"
				""
				"ELSE \r\n"
				""
					"INSERT INTO HCFAClaimProvidersT (ProviderID, InsuranceCoID, ANSI_2010AA_ProviderID, "
					"ANSI_2310B_ProviderID, Box33_ProviderID, Box24J_ProviderID) "
					"VALUES (%li, %li, %s, %s, %s, %s)",
				str2010AA_ProviderID, str2310B_ProviderID, strBox33_ProviderID, strBox24J_ProviderID,
				nProviderID, nInsCoID, str2010AA_ProviderID, str2310B_ProviderID, strBox33_ProviderID, strBox24J_ProviderID);

			pInsCoRow = pInsCoRow->GetNextRow();
		}

		ExecuteSqlBatch(strSqlBatch);

		AfxMessageBox("Your claim provider changes have been saved.");

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnBtnApplyClaimProv");
}

void CAdvHCFAClaimProviderSetupDlg::SetColor(OLE_COLOR nNewColor)
{
	try {

		m_bkg.SetColor(nNewColor);

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::SetColor");
}

void CAdvHCFAClaimProviderSetupDlg::OnSelChosenMainProviderCombo(LPDISPATCH lpRow) 
{
	try {



		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_MainProviderCombo->GetFirstRow();
			if(pRow) {
				m_MainProviderCombo->PutCurSel(pRow);
			}
			else {
				SetDlgItemText(IDC_DEFAULT_PROVIDER_LABEL, "");
				return;
			}
		}

		long nProviderID = VarLong(pRow->GetValue(pccID), -1);

		_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS ClaimProviderName "
			"FROM ProvidersT "
			"INNER JOIN PersonT ON ProvidersT.ClaimProviderID = PersonT.ID "
			"WHERE ProvidersT.PersonID = {INT}", nProviderID);
		if(!rs->eof) {

			CString strLabel;
			strLabel.Format("The current default Claim Provider for this provider '%s'.", AdoFldString(rs, "ClaimProviderName",""));
			SetDlgItemText(IDC_DEFAULT_PROVIDER_LABEL, strLabel);
		}
		rs->Close();

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnSelChosenMainProviderCombo");	
}

void CAdvHCFAClaimProviderSetupDlg::OnRequeryFinishedMainProviderCombo(short nFlags) 
{
	try {

		if(m_MainProviderCombo->GetCurSel()) {
			return;
		}

		//if no provider is selected, attempt to select our default provider,
		//otherwise select the first provider
		
		IRowSettingsPtr pSelRow = NULL;
		if(m_nDefaultProviderID != -1) {
			pSelRow = m_MainProviderCombo->SetSelByColumn(pccID, m_nDefaultProviderID);
		}

		if(pSelRow == NULL) {
			pSelRow = m_MainProviderCombo->GetFirstRow();
		}

		if(pSelRow) {
			m_MainProviderCombo->PutCurSel(pSelRow);
			OnSelChosenMainProviderCombo(pSelRow);
		}

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnRequeryFinishedMainProviderCombo");
}

void CAdvHCFAClaimProviderSetupDlg::OnRequeryFinishedAdvClaimProvUnselectedList(short nFlags) 
{
	try {

		//try to select our defaults, if any were given to us

		if(m_nDefaultInsCoID != -1) {
			IRowSettingsPtr pRow = m_UnselectedList->FindByColumn(ilcID, m_nDefaultInsCoID, m_UnselectedList->GetFirstRow(), FALSE);
			if(pRow) {
				m_SelectedList->TakeRowAddSorted(pRow);
			}
		}

		if(m_nDefaultHCFAGroupID != -1) {

			IRowSettingsPtr pInsCoRow = m_UnselectedList->GetFirstRow();
			while(pInsCoRow) {

				//copy this row and get the next row now, before we move the row
				IRowSettingsPtr pRow = pInsCoRow;
				pInsCoRow = pInsCoRow->GetNextRow();

				long nHCFAGroupID = VarLong(pRow->GetValue(ilcHCFAGroupID), -1);
				if(nHCFAGroupID == m_nDefaultHCFAGroupID) {
					m_SelectedList->TakeRowAddSorted(pRow);
				}
			}
		}

	}NxCatchAll("Error in CAdvHCFAClaimProviderSetupDlg::OnRequeryFinishedAdvClaimProvUnselectedList");
}
