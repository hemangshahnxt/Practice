// CLIASetup.cpp : implementation file
//

#include "stdafx.h"
#include "CLIASetup.h"
#include "GlobalDrawingUtils.h"
#include "CLIAServicesSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CCLIASetup dialog

// (j.jones 2011-10-20 10:29) - PLID 45784 - added insurance lists
enum InsuranceListColumns {

	ilcID = 0,
	ilcName,
	ilcCLIA,
	ilcAddress,
};

CCLIASetup::CCLIASetup(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCLIASetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCLIASetup)
	m_nDefaultInsuranceCoID = -1;
	m_nBkgColor = GetNxColor(GNC_ADMIN, 0);
	//}}AFX_DATA_INIT
}


void CCLIASetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCLIASetup)
	DDX_Control(pDX, IDC_CHECK_REF_PHY, m_btnRefPhys);
	DDX_Control(pDX, IDC_CHECK_BOX_32, m_btnBox32);
	DDX_Control(pDX, IDC_CHECK_BOX_23, m_btnBox23);
	DDX_Control(pDX, IDC_CHECK_USE_MODIFIER, m_btnUseModifier);
	DDX_Control(pDX, IDC_EDIT_CLIA_NUMBER, m_nxeditEditCliaNumber);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_CLIA_BILLING_PROV, m_radioBillingProv);
	DDX_Control(pDX, IDC_RADIO_CLIA_RENDERING_PROV, m_radioRenderingProv);
	DDX_Control(pDX, IDC_BKG_CLIA1, m_bkg1);
	DDX_Control(pDX, IDC_BKG_CLIA2, m_bkg2);
	DDX_Control(pDX, IDC_BKG_CLIA3, m_bkg3);
	DDX_Control(pDX, IDC_BTN_CLIA_SERVICES, m_btnConfigServices);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_INSCO, m_btnSelectOneInsCo);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_INSCOS, m_btnSelectAllInsCo);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_INSCO, m_btnUnselectOneInsCo);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_INSCOS, m_btnUnselectAllInsCo);
	DDX_Control(pDX, IDC_CLIA_INSCO_NAME_LABEL, m_nxstaticInsuranceCoNameLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCLIASetup, CNxDialog)
	//{{AFX_MSG_MAP(CCLIASetup)
	ON_EN_KILLFOCUS(IDC_EDIT_CLIA_NUMBER, OnKillfocusEditCliaNumber)
	ON_BN_CLICKED(IDC_CHECK_USE_MODIFIER, OnCheckUseModifier)
	ON_BN_CLICKED(IDC_BTN_CLIA_SERVICES, OnBtnCliaServices)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_INSCO, OnBtnSelectOneInsco)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_INSCOS, OnBtnSelectAllInscos)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_INSCO, OnBtnUnselectOneInsco)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_INSCOS, OnBtnUnselectAllInscos)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCLIASetup message handlers

// (j.jones 2011-04-05 11:00) - PLID 42372 - added m_nInsuranceCoID, required
// (j.jones 2011-10-20 10:31) - PLID 45784 - renamed to nDefaultInsuranceCoID, as this 
// is all it is used for now
int CCLIASetup::DoModal(int nDefaultInsuranceCoID, OLE_COLOR nBkgColor)
{
	try {

		if(nDefaultInsuranceCoID == -1) {
			//not currently supported
			ThrowNxException("CLIA Setup opened with invalid InsuranceCoID!");
		}

		m_nDefaultInsuranceCoID = nDefaultInsuranceCoID;
		m_nBkgColor = nBkgColor;

		return CNxDialog::DoModal();

	}NxCatchAll(__FUNCTION__);
	
	return 0;
}

BOOL CCLIASetup::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_ComboProviders = BindNxDataListCtrl(this,IDC_COMBO_PROVIDERS,GetRemoteData(),true);
		// (j.jones 2009-02-06 15:48) - PLID 32951 - added m_ComboLocations
		m_ComboLocations = BindNxDataListCtrl(IDC_COMBO_LOCATIONS, true);
		m_ComboModifiers = BindNxDataListCtrl(this,IDC_COMBO_MODIFIERS,GetRemoteData(),true);

		// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
		m_UnselectedInsCoList = BindNxDataList2Ctrl(IDC_UNSELECTED_CLIA_INSCO_LIST, false);
		m_SelectedInsCoList = BindNxDataList2Ctrl(IDC_SELECTED_CLIA_INSCO_LIST, false);

		m_btnSelectOneInsCo.AutoSet(NXB_RIGHT);
		m_btnSelectAllInsCo.AutoSet(NXB_RRIGHT);
		m_btnUnselectOneInsCo.AutoSet(NXB_LEFT);
		m_btnUnselectAllInsCo.AutoSet(NXB_LLEFT);

		// (j.jones 2011-10-19 17:36) - PLID 45784 - filter on the selected insurance ID, if any
		CString strUnselectedWhere = "Archived = 0";
		
		if(m_nDefaultInsuranceCoID != -1) {
			//exclude our InsCoID from the unselected list
			strUnselectedWhere.Format("Archived = 0 AND InsuranceCoT.PersonID <> %li", m_nDefaultInsuranceCoID);

			//requery the selected list to include our company (might be inactive)
			CString strSelectedWhere;
			strSelectedWhere.Format("InsuranceCoT.PersonID = %li", m_nDefaultInsuranceCoID);
			m_SelectedInsCoList->PutWhereClause(_bstr_t(strSelectedWhere));
			m_SelectedInsCoList->Requery();

			//force this to finish requerying before LoadCLIANumber is called
			m_SelectedInsCoList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		m_bkg1.SetColor(m_nBkgColor);
		m_bkg2.SetColor(m_nBkgColor);
		m_bkg3.SetColor(m_nBkgColor);
		
		// (z.manning, 04/30/2008) - PLID 29852 - Set more button styles
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (j.jones 2011-10-19 10:14) - PLID 46023 - moved the service code setup to its own dialog
		m_btnConfigServices.AutoSet(NXB_MODIFY);

		((CNxEdit*)GetDlgItem(IDC_EDIT_CLIA_NUMBER))->LimitText(50);

		m_ComboProviders->CurSel = 0;
		// (j.jones 2009-02-06 15:48) - PLID 32951 - added m_ComboLocations
		m_ComboLocations->SetSelByColumn(0, GetCurrentLocationID());
		if(m_ComboLocations->CurSel == -1) {
			m_ComboLocations->CurSel = 0;
		}

		if(m_ComboProviders->CurSel != -1 && m_ComboLocations->CurSel != -1) {
			LoadCLIANumber();
		}
		else {
			//LoadCLIANumber will always requery the unselected list,
			//so don't requery unless for some reason it's not called
			m_UnselectedInsCoList->PutWhereClause(_bstr_t(strUnselectedWhere));
			m_UnselectedInsCoList->Requery();
		}

		// (j.jones 2011-04-05 13:25) - PLID 42372 - the old ConfigRT settings are now stored in InsuranceCoT
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, UseCLIAModifier, CLIAModifier, "
			"UseCLIAInHCFABox23, UseCLIAInHCFABox32, CLIAUseBillProvInHCFABox17, "
			"CLIAUseRenderingProvider "
			"FROM InsuranceCoT WHERE PersonID = {INT}", m_nDefaultInsuranceCoID);
		if(rs->eof) {
			//should be impossible
			CNxDialog::OnCancel();
			ThrowNxException("CLIA Setup loaded with invalid InsuranceCoID %li!", m_nDefaultInsuranceCoID);
		}

		m_nxstaticInsuranceCoNameLabel.SetWindowText(VarString(rs->Fields->Item["Name"]->Value, ""));

		CheckDlgButton(IDC_CHECK_USE_MODIFIER, VarBool(rs->Fields->Item["UseCLIAModifier"]->Value, FALSE));
		OnCheckUseModifier();

		m_strOriginalModifier = VarString(rs->Fields->Item["CLIAModifier"]->Value, "");
		// (z.manning, 05/01/2007) - PLID 16623 - Changed this to a TrySetSel in part because we need to handle
		// inactive modifiers which are no longer loaded in the list.
		if(m_strOriginalModifier != "") 
		{
			switch(m_ComboModifiers->TrySetSelByColumn(0, _bstr_t(m_strOriginalModifier)))
			{
			case NXDATALIST2Lib::sriNoRowYet_WillFireEvent:
				// (z.manning, 05/01/2007) - Handle this in the TrySetSelFinished handler.
				break;

			case NXDATALIST2Lib::sriNoRow:
				// (z.manning, 05/01/2007) - Did not find it, so it could be inactive.
				SetModifierSelectionFromData(m_strOriginalModifier);
				break;

			default:
				// (z.manning, 05/01/2007) - We found a row and are good to go.
				break;
			}
		}

		CheckDlgButton(IDC_CHECK_BOX_23, VarBool(rs->Fields->Item["UseCLIAInHCFABox23"]->Value, TRUE));
		CheckDlgButton(IDC_CHECK_BOX_32, VarBool(rs->Fields->Item["UseCLIAInHCFABox32"]->Value, TRUE));
		CheckDlgButton(IDC_CHECK_REF_PHY, VarBool(rs->Fields->Item["CLIAUseBillProvInHCFABox17"]->Value, TRUE));

		// (j.jones 2009-02-23 10:42) - PLID 33167 - added the ability to determine whether we pull the
		// CLIA number from the claim's rendering provider or their billing provider, though in most
		// cases this is the same person
		BOOL bCLIAUseRenderingProvider = VarBool(rs->Fields->Item["CLIAUseRenderingProvider"]->Value, FALSE);
		m_radioBillingProv.SetCheck(!bCLIAUseRenderingProvider);
		m_radioRenderingProv.SetCheck(bCLIAUseRenderingProvider);

	}NxCatchAll("CCLIASetup::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCLIASetup::OnOK() 
{
	try {

		// (j.jones 2011-10-20 11:06) - PLID 45784 - don't save if no companies are selected
		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("You must have at least one insurance company selected to save the CLIA information.");
			return;
		}

		//we don't need to warn the multiple companies will be changed, because
		//SaveCLIANumber() is about to do that itself

		SaveCLIANumber();

		CWaitCursor pWait;

		BOOL bCLIAModifier = IsDlgButtonChecked(IDC_CHECK_USE_MODIFIER);

		_variant_t varModifier = g_cvarNull;
		if(m_ComboModifiers->CurSel != -1) {
			varModifier = m_ComboModifiers->GetValue(m_ComboModifiers->CurSel,0);
		}

		BOOL bUseCLIAInHCFABox23 = IsDlgButtonChecked(IDC_CHECK_BOX_23);
		BOOL bUseCLIAInHCFABox32 = IsDlgButtonChecked(IDC_CHECK_BOX_32);
		BOOL bCLIAUseBillProvInHCFABox17 = IsDlgButtonChecked(IDC_CHECK_REF_PHY);

		// (j.jones 2009-02-23 10:42) - PLID 33167 - added the ability to determine whether we pull the
		// CLIA number from the claim's rendering provider or their billing provider, though in most
		// cases this is the same person
		BOOL bCLIAUseRenderingProvider = m_radioRenderingProv.GetCheck() ? 1 : 0;

		// (j.jones 2011-04-05 13:32) - PLID 42372 - this is all saved per insurance company now
		// (j.jones 2011-10-20 11:06) - PLID 45784 - now we save across multiple insurance companies

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_SelectedInsCoList->GetFirstRow();
		CArray<long, long> arySelectedIDs;
		while(pRow) {
			long nInsuranceCoID = VarLong(pRow->GetValue(ilcID));

			arySelectedIDs.Add(nInsuranceCoID);

			pRow = pRow->GetNextRow();
		}

		ExecuteParamSql("UPDATE InsuranceCoT SET UseCLIAModifier = {INT}, CLIAModifier = {VT_BSTR}, "
			"UseCLIAInHCFABox23 = {INT}, UseCLIAInHCFABox32 = {INT}, CLIAUseBillProvInHCFABox17 = {INT}, "
			"CLIAUseRenderingProvider = {INT} "
			"WHERE PersonID IN ({INTARRAY})",
			bCLIAModifier, varModifier,
			bUseCLIAInHCFABox23, bUseCLIAInHCFABox32, bCLIAUseBillProvInHCFABox17,
			bCLIAUseRenderingProvider,
			arySelectedIDs);

		CNxDialog::OnOK();

	}NxCatchAll("Error saving CLIA settings.");
}

BEGIN_EVENTSINK_MAP(CCLIASetup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCLIASetup)
	ON_EVENT(CCLIASetup, IDC_COMBO_PROVIDERS, 16 /* SelChosen */, OnSelChosenComboProviders, VTS_I4)
	ON_EVENT(CCLIASetup, IDC_COMBO_LOCATIONS, 16 /* SelChosen */, OnSelChosenComboLocations, VTS_I4)
	ON_EVENT(CCLIASetup, IDC_COMBO_MODIFIERS, 20 /* TrySetSelFinished */, OnTrySetSelFinishedComboModifiers, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CCLIASetup, IDC_UNSELECTED_CLIA_INSCO_LIST, 3, OnDblClickCellUnselectedCliaInscoList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CCLIASetup, IDC_SELECTED_CLIA_INSCO_LIST, 3, OnDblClickCellSelectedCliaInscoList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CCLIASetup::OnSelChosenComboProviders(long nRow) 
{
	try {
	
		if(nRow == -1) {
			SetDlgItemText(IDC_EDIT_CLIA_NUMBER,"");
			return;
		}

		LoadCLIANumber();

	}NxCatchAll("Error in CCLIASetup::OnSelChosenComboProviders");
}

// (j.jones 2009-02-06 15:46) - PLID 32951 - added OnSelChosenComboLocations
void CCLIASetup::OnSelChosenComboLocations(long nRow) 
{
	try {
	
		if(nRow == -1) {
			SetDlgItemText(IDC_EDIT_CLIA_NUMBER,"");
			return;
		}

		LoadCLIANumber();

	}NxCatchAll("Error in CCLIASetup::OnSelChosenComboLocations");
}

void CCLIASetup::LoadCLIANumber()
{
	_RecordsetPtr rs;
	_variant_t var;
	CString str;
		
	try {

		if(m_ComboProviders->CurSel == -1 || m_ComboLocations->CurSel == -1) {
			SetDlgItemText(IDC_EDIT_CLIA_NUMBER,"");
			return;
		}
		long nProviderID = VarLong(m_ComboProviders->GetValue(m_ComboProviders->GetCurSel(),0));
		long nLocationID = VarLong(m_ComboLocations->GetValue(m_ComboLocations->GetCurSel(),0));

		// (j.jones 2009-02-06 16:15) - PLID 32951 - converted to be stored per provider, per location
		rs = CreateParamRecordset("SELECT CLIANumber FROM CLIANumbersT WHERE ProviderID = {INT} AND LocationID = {INT} AND InsuranceCoID = {INT}",nProviderID, nLocationID, m_nDefaultInsuranceCoID);
		if (!rs->eof) {
			SetDlgItemText(IDC_EDIT_CLIA_NUMBER, AdoFldString(rs, "CLIANumber", ""));
		}
		else {
			SetDlgItemText(IDC_EDIT_CLIA_NUMBER, "");
		}
		rs->Close();

		// (j.jones 2011-10-20 10:55) - PLID 45784 - we need to requery each list, with the currently selected
		// companies, and the CLIA for this provider, this location

		CString strCLIAColumn;
		strCLIAColumn.Format("Coalesce((SELECT TOP 1 CLIANumber FROM CLIANumbersT WHERE ProviderID = %li AND LocationID = %li AND InsuranceCoID = InsuranceCoT.PersonID), '')",nProviderID, nLocationID);
		NXDATALIST2Lib::IColumnSettingsPtr pUnselectedCliaCol = m_UnselectedInsCoList->GetColumn(ilcCLIA);
		NXDATALIST2Lib::IColumnSettingsPtr pSelectedCliaCol = m_SelectedInsCoList->GetColumn(ilcCLIA);
		pUnselectedCliaCol->PutFieldName((LPCTSTR)strCLIAColumn);
		pSelectedCliaCol->PutFieldName((LPCTSTR)strCLIAColumn);

		CString strSelectedIDs;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_SelectedInsCoList->GetFirstRow();
		while(pRow) {
			long nInsuranceCoID = VarLong(pRow->GetValue(ilcID));

			if(!strSelectedIDs.IsEmpty()) {
				strSelectedIDs += ",";
			}
			strSelectedIDs += AsString(nInsuranceCoID);

			pRow = pRow->GetNextRow();
		}

		CString strUnselectedWhere, strSelectedWhere;
		if(strSelectedIDs.IsEmpty()) {
			//nothing selected
			strUnselectedWhere = "Archived = 0";
			m_SelectedInsCoList->Clear();
			m_UnselectedInsCoList->PutWhereClause(_bstr_t(strUnselectedWhere));
			m_UnselectedInsCoList->Requery();
		}
		else {
			strUnselectedWhere.Format("Archived = 0 AND InsuranceCoT.PersonID NOT IN (%s)", strSelectedIDs);
			strSelectedWhere.Format("InsuranceCoT.PersonID IN (%s)", strSelectedIDs);
			m_SelectedInsCoList->PutWhereClause(_bstr_t(strSelectedWhere));
			m_SelectedInsCoList->Requery();
			m_UnselectedInsCoList->PutWhereClause(_bstr_t(strUnselectedWhere));
			m_UnselectedInsCoList->Requery();
		}
	}
	NxCatchAll("Error in CCLIASetup::LoadCLIANumber");
}

void CCLIASetup::SaveCLIANumber()
{
	try {

		// (j.jones 2009-02-06 15:48) - PLID 32951 - added m_ComboLocations
		if(m_ComboProviders->CurSel == -1 || m_ComboLocations->CurSel == -1) {
			return;
		}

		CString str, strCLIANumber;
		GetDlgItemText(IDC_EDIT_CLIA_NUMBER,strCLIANumber);

		// (j.jones 2011-10-20 11:06) - PLID 45784 - don't save if no companies are selected
		if(m_SelectedInsCoList->GetRowCount() == 0) {
			AfxMessageBox("You must have at least one insurance company selected to save the CLIA information.");
			return;
		}
		else if(m_SelectedInsCoList->GetRowCount() > 1) {
			//warn when we are about to change the number for more than one row,
			//don't worry if they are only changing settings and not the number

			CWaitCursor pWait;

			long nCountChanged = 0;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_SelectedInsCoList->GetFirstRow();
			while(pRow) {
				CString strOldCLIA = VarString(pRow->GetValue(ilcCLIA),"");
				if(strOldCLIA.CompareNoCase(strCLIANumber) != 0) {
					nCountChanged++;
				}

				pRow = pRow->GetNextRow();
			}

			if(nCountChanged > 1) {
				CString strWarning;
				strWarning.Format("You are about to change the CLIA number for the current provider and location "
					"for %li selected insurance companies.\n"
					"Are you sure you wish to change the CLIA for these companies?", nCountChanged);
				if(IDNO == MessageBox(strWarning, "Practice", MB_YESNO|MB_ICONQUESTION)) {
					return;
				}
			}
		}

		CWaitCursor pWait;

		long nProviderID = VarLong(m_ComboProviders->GetValue(m_ComboProviders->GetCurSel(),0));
		long nLocationID = VarLong(m_ComboLocations->GetValue(m_ComboLocations->GetCurSel(),0));

		// (j.jones 2011-10-20 11:06) - PLID 45784 - now we save across multiple insurance companies,
		// which means this cannot be parameterized due to an unknown amount of statements

		CString strSqlBatch;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_SelectedInsCoList->GetFirstRow();
		while(pRow) {
			long nInsuranceCoID = VarLong(pRow->GetValue(ilcID));

			AddStatementToSqlBatch(strSqlBatch, "IF EXISTS (SELECT ProviderID FROM CLIANumbersT WHERE ProviderID = %li AND LocationID = %li AND InsuranceCoID = %li) "
				"	BEGIN "
				"		UPDATE CLIANumbersT SET CLIANumber = '%s' "
				"		WHERE ProviderID = %li AND LocationID = %li AND InsuranceCoID = %li "
				"	END "
				"	ELSE BEGIN "
				"		INSERT INTO CLIANumbersT (ProviderID, LocationID, InsuranceCoID, CLIANumber) "
				"		VALUES (%li, %li, %li, '%s') "
				"	END ",
				nProviderID, nLocationID, nInsuranceCoID,
				_Q(strCLIANumber), nProviderID, nLocationID, nInsuranceCoID,
				nProviderID, nLocationID, nInsuranceCoID, _Q(strCLIANumber));

			pRow = pRow->GetNextRow();
		}

		ExecuteSqlBatch(strSqlBatch);

		//if this succeeded, update each row in the selected list
		pRow = m_SelectedInsCoList->GetFirstRow();
		while(pRow) {
			pRow->PutValue(ilcCLIA, (LPCTSTR)strCLIANumber);
			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in CCLIASetup::SaveCLIANumber");
}

// (j.jones 2011-10-19 09:38) - PLID 46023 - removed service code controls

// (j.jones 2011-10-19 09:38) - PLID 46023 - removed service code controls

void CCLIASetup::OnKillfocusEditCliaNumber() 
{
	SaveCLIANumber();
}

void CCLIASetup::OnCheckUseModifier() 
{
	m_ComboModifiers->Enabled = IsDlgButtonChecked(IDC_CHECK_USE_MODIFIER);
}

BOOL CCLIASetup::SetModifierSelectionFromData(CString strModifierID)
{
	// (z.manning, 05/01/2007) - PLID 16623 - Make sure the modifier exists before we set the selection.
	if(ReturnsRecords("SELECT Number FROM CptModifierT WHERE Number = '%s'", _Q(strModifierID))) {
		m_ComboModifiers->PutComboBoxText(_bstr_t(strModifierID));
		return TRUE;
	}
	else {
		m_ComboModifiers->PutCurSel(-1);
		return FALSE;
	}
}

void CCLIASetup::OnTrySetSelFinishedComboModifiers(long nRowEmum, long nFlags)
{
	try {

		// (z.manning, 05/02/2007) - PLID 16623 - If the set sel failed, it may be because the modifier is inactive.
		if(nFlags == NXDATALISTLib::dlTrySetSelFinishedFailure) {
			SetModifierSelectionFromData(m_strOriginalModifier);
		}

	}NxCatchAll("CCLIASetup::OnTrySetSelFinishedComboModifiers");
}

// (j.jones 2011-10-19 10:14) - PLID 46023 - moved the service code setup to its own dialog
void CCLIASetup::OnBtnCliaServices()
{
	try {

		CCLIAServicesSetupDlg dlg(this);
		dlg.m_nBkgColor = m_nBkgColor;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
void CCLIASetup::OnDblClickCellUnselectedCliaInscoList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		m_UnselectedInsCoList->CurSel = pRow;

		if(pRow) {
			OnBtnSelectOneInsco();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
void CCLIASetup::OnDblClickCellSelectedCliaInscoList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		m_SelectedInsCoList->CurSel = pRow;

		if(pRow) {
			OnBtnUnselectOneInsco();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
void CCLIASetup::OnBtnSelectOneInsco()
{
	try {

		m_SelectedInsCoList->TakeCurrentRowAddSorted(m_UnselectedInsCoList, NULL);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
void CCLIASetup::OnBtnSelectAllInscos()
{
	try {

		m_SelectedInsCoList->TakeAllRows(m_UnselectedInsCoList);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
void CCLIASetup::OnBtnUnselectOneInsco()
{
	try {

		m_UnselectedInsCoList->TakeCurrentRowAddSorted(m_SelectedInsCoList, NULL);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
void CCLIASetup::OnBtnUnselectAllInscos()
{
	try {

		m_UnselectedInsCoList->TakeAllRows(m_SelectedInsCoList);

	}NxCatchAll(__FUNCTION__);
}