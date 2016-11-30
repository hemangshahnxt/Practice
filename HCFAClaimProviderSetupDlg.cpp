// HCFAClaimProviderSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HCFAClaimProviderSetupDlg.h"
#include "AdvHCFAClaimProviderSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-04-03 11:37) - PLID 28995 - created

/////////////////////////////////////////////////////////////////////////////
// CHCFAClaimProviderSetupDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

//each combo is the same, so there is only one enumeration
enum ProviderComboColumn {

	pccID = 0,
	pccName,
};

CHCFAClaimProviderSetupDlg::CHCFAClaimProviderSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHCFAClaimProviderSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHCFAClaimProviderSetupDlg)
		m_nInsCoID = -1;
		m_bChanged = FALSE;
		m_nProviderID = -1;
		m_nColor = OLE_COLOR(0x00FFCC99);
	//}}AFX_DATA_INIT
}


void CHCFAClaimProviderSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHCFAClaimProviderSetupDlg)
	DDX_Control(pDX, IDC_BTN_ADVANCED_CLAIM_PROV_SETUP, m_btnAdvancedSetup);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_DEFAULT_PROVIDER_LABEL, m_DefaultInfoLabel);
	DDX_Control(pDX, IDC_HCFA_CLAIM_PROV_COLOR, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHCFAClaimProviderSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHCFAClaimProviderSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADVANCED_CLAIM_PROV_SETUP, OnBtnAdvancedClaimProvSetup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHCFAClaimProviderSetupDlg message handlers

BOOL CHCFAClaimProviderSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (j.jones 2008-08-01 12:55) - PLID 30918 - added m_btnAdvancedSetup
		m_btnAdvancedSetup.AutoSet(NXB_MODIFY);

		SetColor(m_nColor);

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
		pRow = m_2310BCombo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_2310BCombo->AddRowSorted(pRow, NULL);
		pRow = m_Box33Combo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_Box33Combo->AddRowSorted(pRow, NULL);
		pRow = m_Box24JCombo->GetNewRow();
		pRow->PutValue(pccID, (long)-1);
		pRow->PutValue(pccName, (LPCTSTR)" <Use Default Provider>");
		m_Box24JCombo->AddRowSorted(pRow, NULL);

		//the provider's "default" claim provider label is blank
		//for the moment since we don't have one selected
		SetDlgItemText(IDC_DEFAULT_PROVIDER_LABEL, "");

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CHCFAClaimProviderSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHCFAClaimProviderSetupDlg)
	ON_EVENT(CHCFAClaimProviderSetupDlg, IDC_MAIN_PROVIDER_COMBO, 16 /* SelChosen */, OnSelChosenMainProviderCombo, VTS_DISPATCH)
	ON_EVENT(CHCFAClaimProviderSetupDlg, IDC_MAIN_PROVIDER_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedMainProviderCombo, VTS_I2)
	ON_EVENT(CHCFAClaimProviderSetupDlg, IDC_2010AA_COMBO, 16 /* SelChosen */, OnSelChosen2010aaCombo, VTS_DISPATCH)
	ON_EVENT(CHCFAClaimProviderSetupDlg, IDC_BOX33_COMBO, 16 /* SelChosen */, OnSelChosenBox33Combo, VTS_DISPATCH)
	ON_EVENT(CHCFAClaimProviderSetupDlg, IDC_2310B_COMBO, 16 /* SelChosen */, OnSelChosen2310bCombo, VTS_DISPATCH)
	ON_EVENT(CHCFAClaimProviderSetupDlg, IDC_BOX24J_COMBO, 16 /* SelChosen */, OnSelChosenBox24jCombo, VTS_DISPATCH)	
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CHCFAClaimProviderSetupDlg::OnSelChosenMainProviderCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_MainProviderCombo->SetSelByColumn(pccID, m_nProviderID);
			return;
		}

		long nNewProviderID = VarLong(pRow->GetValue(pccID), -1);

		//prompt to save if anything changed
		if(m_nProviderID != -1) {
			if(nNewProviderID != m_nProviderID) {
				if(m_bChanged) {
					if(IDYES == MessageBox("Any changes made to the previous provider will be saved.\n"
						"Do you still wish to switch providers?","Practice",MB_ICONQUESTION|MB_YESNO)) {
		
						if(!Save()) {
							//the save failed, don't load and overwrite their changes yet
							m_MainProviderCombo->SetSelByColumn(pccID, (long)m_nProviderID);
							return;
						}
					}
					else {
						//don't load and overwrite their changes yet
						m_MainProviderCombo->SetSelByColumn(pccID, (long)m_nProviderID);
						return;
					}
				}
			}
			else {
				//don't load and overwrite the changes
				return;
			}
		}

		m_nProviderID = nNewProviderID;
		Load();

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnSelChosenMainProviderCombo");	
}

void CHCFAClaimProviderSetupDlg::Load()
{
	try {

		if(m_nProviderID == -1) {
			m_2010AACombo->PutCurSel(NULL);
			m_2310BCombo->PutCurSel(NULL);
			m_Box33Combo->PutCurSel(NULL);
			m_Box24JCombo->PutCurSel(NULL);

			SetDlgItemText(IDC_DEFAULT_PROVIDER_LABEL, "");
			return;
		}

		// (j.jones 2008-08-01 09:45) - PLID 30917 - converted to be per insurance company, not per group
		_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS ClaimProviderName, "
			"HCFAClaimProvidersT.ANSI_2010AA_ProviderID, HCFAClaimProvidersT.ANSI_2310B_ProviderID, "
			"HCFAClaimProvidersT.Box33_ProviderID, HCFAClaimProvidersT.Box24J_ProviderID "
			"FROM ProvidersT "
			"INNER JOIN PersonT ON ProvidersT.ClaimProviderID = PersonT.ID "
			"LEFT JOIN (SELECT * FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}) AS HCFAClaimProvidersT ON ProvidersT.PersonID = HCFAClaimProvidersT.ProviderID "
			"WHERE ProvidersT.PersonID = {INT}", m_nInsCoID, m_nProviderID);
		if(!rs->eof) {

			{
				long n2010AA_ProviderID = AdoFldLong(rs, "ANSI_2010AA_ProviderID", -1);
				IRowSettingsPtr pRow = m_2010AACombo->SetSelByColumn(pccID, n2010AA_ProviderID);
				if(pRow == NULL && n2010AA_ProviderID != -1) {
					//maybe it's inactive?
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS Name "
						"FROM PersonT WHERE ID = {INT}", n2010AA_ProviderID);
					if(!rsProv->eof) {
						m_2010AACombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
					}
					else {
						m_2010AACombo->SetSelByColumn(pccID, (long)-1);
					}
				}
			}

			{
				long n2310B_ProviderID = AdoFldLong(rs, "ANSI_2310B_ProviderID", -1);
				IRowSettingsPtr pRow = m_2310BCombo->SetSelByColumn(pccID, n2310B_ProviderID);
				if(pRow == NULL && n2310B_ProviderID != -1) {
					//maybe it's inactive?
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS Name "
						"FROM PersonT WHERE ID = {INT}", n2310B_ProviderID);
					if(!rsProv->eof) {
						m_2310BCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
					}
					else {
						m_2310BCombo->SetSelByColumn(pccID, (long)-1);
					}
				}
			}

			{
				long nBox33_ProviderID = AdoFldLong(rs, "Box33_ProviderID", -1);
				IRowSettingsPtr pRow = m_Box33Combo->SetSelByColumn(pccID, nBox33_ProviderID);
				if(pRow == NULL && nBox33_ProviderID != -1) {
					//maybe it's inactive?
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS Name "
						"FROM PersonT WHERE ID = {INT}", nBox33_ProviderID);
					if(!rsProv->eof) {
						m_Box33Combo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
					}
					else {
						m_Box33Combo->SetSelByColumn(pccID, (long)-1);
					}
				}
			}

			{
				long nBox24J_ProviderID = AdoFldLong(rs, "Box24J_ProviderID", -1);
				IRowSettingsPtr pRow = m_Box24JCombo->SetSelByColumn(pccID, nBox24J_ProviderID);
				if(pRow == NULL && nBox24J_ProviderID != -1) {
					//maybe it's inactive?
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS Name "
						"FROM PersonT WHERE ID = {INT}", nBox24J_ProviderID);
					if(!rsProv->eof) {
						m_Box24JCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
					}
					else {
						m_Box24JCombo->SetSelByColumn(pccID, (long)-1);
					}
				}
			}

			CString strLabel;
			strLabel.Format("The current default Claim Provider for this provider '%s'.", AdoFldString(rs, "ClaimProviderName",""));
			SetDlgItemText(IDC_DEFAULT_PROVIDER_LABEL, strLabel);
		}
		rs->Close();

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::Load");	
}

void CHCFAClaimProviderSetupDlg::OnRequeryFinishedMainProviderCombo(short nFlags) 
{
	try {

		//if no provider is selected, attempt to select the first provider

		if(m_MainProviderCombo->GetCurSel()) {
			return;
		}

		IRowSettingsPtr pSelRow = m_MainProviderCombo->GetFirstRow();
		if(pSelRow) {
			m_MainProviderCombo->PutCurSel(pSelRow);
			
			m_nProviderID = VarLong(pSelRow->GetValue(pccID), -1);

			Load();
		}

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnRequeryFinishedMainProviderCombo");
}

void CHCFAClaimProviderSetupDlg::OnOK() 
{
	try {

		if(!Save()) {
			return;
		}
	
		CDialog::OnOK();

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnOK");
}

BOOL CHCFAClaimProviderSetupDlg::Save()
{
	try {

		//if nothing is selected, silently return TRUE, allowing them to save nothing
		if(m_nProviderID == -1) {
			return TRUE;
		}

		long nANSI_2010AA_ProviderID = -1;
		long nANSI_2310B_ProviderID = -1;
		long nBox33_ProviderID = -1;
		long nBox24J_ProviderID = -1;

		CString str2010AA_ProviderID = "NULL";
		CString str2310B_ProviderID = "NULL";
		CString strBox33_ProviderID = "NULL";
		CString strBox24J_ProviderID = "NULL";

		IRowSettingsPtr pRow = m_2010AACombo->GetCurSel();
		if(pRow) {
			nANSI_2010AA_ProviderID = VarLong(pRow->GetValue(pccID),-1);
		}
		pRow = m_2310BCombo->GetCurSel();
		if(pRow) {
			nANSI_2310B_ProviderID = VarLong(pRow->GetValue(pccID),-1);
		}
		pRow = m_Box33Combo->GetCurSel();
		if(pRow) {
			nBox33_ProviderID = VarLong(pRow->GetValue(pccID),-1);
		}
		pRow = m_Box24JCombo->GetCurSel();
		if(pRow) {
			nBox24J_ProviderID = VarLong(pRow->GetValue(pccID),-1);
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

		long nRecordsAffected = 0;

		//our NULL filters make the param SQL not workable here
		// (j.jones 2008-08-01 10:21) - PLID 30917 - this is now per insurance company, not per group
		ExecuteSql(&nRecordsAffected, adCmdText, 
			"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
			"UPDATE HCFAClaimProvidersT SET "
			"ANSI_2010AA_ProviderID = %s, ANSI_2310B_ProviderID = %s, Box33_ProviderID = %s, Box24J_ProviderID = %s "
			"WHERE ProviderID = %li AND InsuranceCoID = %li", str2010AA_ProviderID, str2310B_ProviderID, strBox33_ProviderID, strBox24J_ProviderID,
			m_nProviderID, m_nInsCoID);

		//if we don't have a record yet, we need to create one, but only if at least one value is non-empty
		if(nRecordsAffected == 0 &&
			(nANSI_2010AA_ProviderID != -1 || nANSI_2310B_ProviderID != -1
			|| nBox33_ProviderID != -1 || nBox24J_ProviderID != -1)) {

			// (j.jones 2008-08-01 10:21) - PLID 30917 - this is now per insurance company, not per group
			ExecuteSql("INSERT INTO HCFAClaimProvidersT (ProviderID, InsuranceCoID, ANSI_2010AA_ProviderID, "
				"ANSI_2310B_ProviderID, Box33_ProviderID, Box24J_ProviderID) "
				"VALUES (%li, %li, %s, %s, %s, %s)", m_nProviderID, m_nInsCoID, str2010AA_ProviderID, 
				str2310B_ProviderID, strBox33_ProviderID, strBox24J_ProviderID);
		}

		m_bChanged = FALSE;

		return TRUE;

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::Save");

	return FALSE;
}

void CHCFAClaimProviderSetupDlg::OnSelChosen2010aaCombo(LPDISPATCH lpRow) 
{
	try {

		//don't be too concerned if they actually selected the same row,
		//instead just track as changed if they selected anything, period
		m_bChanged = TRUE;

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnSelChosen2010aaCombo");
}

void CHCFAClaimProviderSetupDlg::OnSelChosenBox33Combo(LPDISPATCH lpRow) 
{
	try {

		//don't be too concerned if they actually selected the same row,
		//instead just track as changed if they selected anything, period
		m_bChanged = TRUE;

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnSelChosenBox33Combo");
}

void CHCFAClaimProviderSetupDlg::OnSelChosen2310bCombo(LPDISPATCH lpRow) 
{
	try {

		//don't be too concerned if they actually selected the same row,
		//instead just track as changed if they selected anything, period
		m_bChanged = TRUE;

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnSelChosen2310bCombo");
}

void CHCFAClaimProviderSetupDlg::OnSelChosenBox24jCombo(LPDISPATCH lpRow) 
{
	try {

		//don't be too concerned if they actually selected the same row,
		//instead just track as changed if they selected anything, period
		m_bChanged = TRUE;

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnSelChosenBox24jCombo");
}

void CHCFAClaimProviderSetupDlg::SetColor(OLE_COLOR nNewColor)
{
	try {

		m_bkg.SetColor(nNewColor);

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::SetColor");
}

// (j.jones 2008-08-01 12:55) - PLID 30918 - added OnBtnAdvancedClaimProvSetup
void CHCFAClaimProviderSetupDlg::OnBtnAdvancedClaimProvSetup() 
{
	try {

		CAdvHCFAClaimProviderSetupDlg dlg(this);
		dlg.m_nColor = m_nColor;

		//select the current insco
		dlg.m_nDefaultInsCoID = m_nInsCoID;

		//tell the dialog to pre-select the providers we currently have selected		
		dlg.m_nDefaultProviderID = m_nProviderID;
		
		IRowSettingsPtr pRow = m_2010AACombo->GetCurSel();
		if(pRow) {
			dlg.m_nDefault2010AA = VarLong(pRow->GetValue(pccID),-1);
		}
		pRow = m_2310BCombo->GetCurSel();
		if(pRow) {
			dlg.m_nDefault2310B = VarLong(pRow->GetValue(pccID),-1);
		}
		pRow = m_Box33Combo->GetCurSel();
		if(pRow) {
			dlg.m_nDefaultBox33 = VarLong(pRow->GetValue(pccID),-1);
		}
		pRow = m_Box24JCombo->GetCurSel();
		if(pRow) {
			dlg.m_nDefaultBox24J = VarLong(pRow->GetValue(pccID),-1);
		}
		
		dlg.DoModal();

		//reload the screen, incase anything changed
		Load();

	}NxCatchAll("Error in CHCFAClaimProviderSetupDlg::OnBtnAdvancedClaimProvSetup");
}
