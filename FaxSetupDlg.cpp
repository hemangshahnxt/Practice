// FaxSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "FaxSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

//DRT 6/26/2008 - PLID 30524 - Created.  I tried to make this as generic as possible for any number
//	of fax services, but unfortunately we just don't have any info on how other fax services work.
//	If they are similar, this should work out pretty well, but as we support others, we may need to
//	rework a little bit how things show up in this dialog.  Some options may need to be added / removed
//	depending on the service.


/////////////////////////////////////////////////////////////////////////////
// CFaxSetupDlg dialog

//(r.farnworth 2013-04-22) PLID 54667 - Added column enumerations
enum FaxConfigColumns{
	alcConfigID = 0,
	alcFaxType = 1,
	alcConfigName = 2,
	alcUserName = 3,
	alcPassword = 4,
	alcFromName = 5,
	alcResolution = 6
};


CFaxSetupDlg::CFaxSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFaxSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaxSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	checkRow = NULL;
	m_nCurrentLoadedID = -1;
}


void CFaxSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaxSetupDlg)
	DDX_Control(pDX, IDC_FAX_USER, m_editUser);
	DDX_Control(pDX, IDC_FAX_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_FAX_FROM_NAME, m_editFromName);
	DDX_Control(pDX, IDC_FAX_CONFIG_NAME, m_editName);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_FAX_CONFIG_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_FAX_CONFIG_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_ERX_SELECT, m_checkERX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFaxSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFaxSetupDlg)
	ON_BN_CLICKED(IDC_FAX_CONFIG_ADD, OnFaxConfigAdd)
	ON_BN_CLICKED(IDC_FAX_CONFIG_REMOVE, OnFaxConfigRemove)
	ON_EN_CHANGE(IDC_FAX_CONFIG_NAME, OnChangeFaxConfigName)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ERX_SELECT, &CFaxSetupDlg::OnBnClickedErxSelect)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaxSetupDlg message handlers

BOOL CFaxSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Bind datalist
		m_pServiceList = BindNxDataList2Ctrl(IDC_FAX_SERVICE_LIST, false);
		m_pResolutionList = BindNxDataList2Ctrl(IDC_FAX_RESOLUTION, false);
		m_pConfigList = BindNxDataList2Ctrl(IDC_FAX_CONFIG_LIST, false);

		//Set text limits on text fields
		m_editName.SetLimitText(255);
		m_editUser.SetLimitText(255);
		m_editPassword.SetLimitText(255);
		m_editFromName.SetLimitText(255);

		//Setup button icons
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);

		//Load the list of supported services
		LoadServiceList(m_pServiceList);

		//Load the list of supported resolutions
		LoadResolutions(m_pResolutionList);

		//Load the list of configurations.  These apply ONLY to the current user
		{
			m_pConfigList->WhereClause = _bstr_t(FormatString("FaxConfigT.UserID = %li", GetCurrentUserID()));
			m_pConfigList->Requery();
			m_pConfigList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		g_propManager.CachePropertiesInBulk("FaxSetupDlg", propNumber, 
			"("
			"("
			"Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ERxFaxConfig' "
			")"
			")", 
			_Q(GetCurrentUserName()));

		long nERxConfig = GetRemotePropertyInt("ERxFaxConfig", 0, 0, GetCurrentUserName());

		// (j.jones 2009-05-01 08:46) - PLID 34132 - store decrypted passwords in the datalist
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Password FROM FaxConfigT WHERE UserID = {INT}", GetCurrentUserID());
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID", -1);


			_variant_t varPassword = rs->Fields->Item["Password"]->Value;

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->FindByColumn(0, (long)nID, m_pConfigList->GetFirstRow(), g_cvarFalse);
			if(pRow) {
				pRow->PutValue(4, _bstr_t(DecryptStringFromVariant(varPassword)));
				
				//(r.farnworth 2013-04-23) PLID 54667 - If this row is the default config, we'll want it to be checked.
				if(nID == nERxConfig)
					checkRow = pRow;
			}
			else {
				//why isn't the row in the list?
				ASSERT(FALSE);
			}
			
			rs->MoveNext();
		}
		rs->Close();

		//Default to the first configuration, if there is one
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pConfigList->FindAbsoluteFirstRow(VARIANT_TRUE);
		if(pCurSel != NULL) {
			m_pConfigList->PutCurSel(pCurSel);

			//Now load the data
			SyncDatalistToInterface();
		}

		//If there aren't any configs, this will disable the screen
		EnsureControls();

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFaxSetupDlg::EnsureControls()
{
	BOOL bEnable = TRUE;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetCurSel();
	if(pRow == NULL) {
		//No selection, disable everything.
		m_checkERX.SetCheck(BST_UNCHECKED);
		bEnable = FALSE;
	}

	GetDlgItem(IDC_FAX_CONFIG_NAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_SERVICE_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_USER)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_PASSWORD)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_FROM_NAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_RESOLUTION)->EnableWindow(bEnable);
	GetDlgItem(IDC_ERX_SELECT)->EnableWindow(bEnable);
}

//Takes all data currently on screen and saves it to the datalist, where it will be read when 
//	eventually saving to data.  This should be called before any loading (like changing the selection).
void CFaxSetupDlg::SyncInterfaceToDatalist()
{
	//If our controls are disabled, we haven't started yet.
	if(GetDlgItem(IDC_FAX_CONFIG_NAME)->IsWindowEnabled() == FALSE) {
		return;
	}

	//Pull all off the interface
	eSupportedFaxServices esfsType;
	CString strName, strUser, strPass, strFrom, strResolution;
	GetDlgItemText(IDC_FAX_CONFIG_NAME, strName);
	GetDlgItemText(IDC_FAX_USER, strUser);
	GetDlgItemText(IDC_FAX_PASSWORD, strPass);
	GetDlgItemText(IDC_FAX_FROM_NAME, strFrom);
	NXDATALIST2Lib::IRowSettingsPtr pRowType = m_pServiceList->GetCurSel();
	if(pRowType != NULL) {
		esfsType = (eSupportedFaxServices)VarLong(pRowType->GetValue(0));
	}
	else {
		//This shouldn't happen
		AfxThrowNxException("Invalid fax service chosen.  Please refresh and try again.");
		return;
	}
	NXDATALIST2Lib::IRowSettingsPtr pRowRes = m_pResolutionList->GetCurSel();
	if(pRowRes != NULL) {
		strResolution = VarString(pRowRes->GetValue(0));
	}

	//Now put it all in the datalist, at the previously selected row
	NXDATALIST2Lib::IRowSettingsPtr pRowList = m_pConfigList->FindByColumn(0, (long)m_nCurrentLoadedID, NULL, VARIANT_FALSE);
	if(pRowList == NULL) {
		//Can't happen, or nothing was loaded to begin with
		AfxThrowNxException("Invalid configuration selected.  Please refresh and try again.");
		return;
	}

	//Load it all into the datalist, where it will later be saved
	pRowList->PutValue(1, (long)esfsType);
	pRowList->PutValue(2, _bstr_t(strName));
	pRowList->PutValue(3, _bstr_t(strUser));
	// (j.jones 2009-04-30 14:19) - PLID 34132 - the password is saved in the datalist unencrypted
	pRowList->PutValue(4, _bstr_t(strPass));
	pRowList->PutValue(5, _bstr_t(strFrom));
	pRowList->PutValue(6, _bstr_t(strResolution));
}

//Takes all data currently in the datalist row and puts it into the interface
void CFaxSetupDlg::SyncDatalistToInterface()
{
	eSupportedFaxServices esfsType;
	CString strName, strUser, strPass, strFrom, strResolution;

	//Pull it all out of the interface
	NXDATALIST2Lib::IRowSettingsPtr pRowList = m_pConfigList->GetCurSel();
	if(pRowList == NULL) {
		//Just quit, this is possible
		return;
	}

	//Set the member variable, we're nowo fficially loaded on this item
	m_nCurrentLoadedID = VarLong(pRowList->GetValue(0));

	//Other data
	esfsType = (eSupportedFaxServices)VarLong(pRowList->GetValue(1));
	strName = VarString(pRowList->GetValue(2));
	strUser = VarString(pRowList->GetValue(3));
	strPass = VarString(pRowList->GetValue(4));
	strFrom = VarString(pRowList->GetValue(5));
	strResolution = VarString(pRowList->GetValue(6));

	//Fill in the interface
	SetDlgItemText(IDC_FAX_CONFIG_NAME, strName);
	SetDlgItemText(IDC_FAX_USER, strUser);
	SetDlgItemText(IDC_FAX_PASSWORD, strPass);
	SetDlgItemText(IDC_FAX_FROM_NAME, strFrom);

	m_pServiceList->SetSelByColumn(0, (long)esfsType);

	//(r.farnworth 2013-01-21) PLID - 54667 Allows checkboxes to be unique for each configuration
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetCurSel();
	
	//(r.farnworth 2013-04-23) PLID 54667 - Check the default box on the appropriate row
	if(pRow == checkRow)
	{
		m_checkERX.SetCheck(BST_CHECKED);
	} else {
		m_checkERX.SetCheck(BST_UNCHECKED);
	}

	m_pResolutionList->SetSelByColumn(0, _bstr_t(strResolution));
}

//Saves all data on the interface for each row in the datalist.  All saving is done at the same time, not as
//	the user makes changes.
//Returns true if the save was successful, false if something known was wrong, or may throw an exception.
bool CFaxSetupDlg::SaveData()
{
	//First, sync the current text to the datalist.  All saving is done from there
	SyncInterfaceToDatalist();

	//(r.farnworth 2013-04-22) PLID 54667 - Switched to a CParamSqlBatch object to use record sets
	CParamSqlBatch sql;

	NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pConfigList->FindAbsoluteFirstRow(VARIANT_TRUE);

	// (j.jones 2009-05-01 09:42) - PLID 34132 - track passwords in an array
	CArray<VARIANT, VARIANT> aryVarPasswords;
	long confID;

	sql.Add("SET NOCOUNT ON "
		"DECLARE @NewConfigID INT; ");

	while(pCurRow != NULL) {

		//Get the ID
		long nID = VarLong(pCurRow->GetValue(0));


		//Get all the other data
		//
		long nFaxType = VarLong(pCurRow->GetValue(1));
		//Get the fields from the interface
		CString strName, strUser, strPass, strFrom, strResolution;
		strName = VarString(pCurRow->GetValue(2));
		strUser = VarString(pCurRow->GetValue(3));
		strPass = VarString(pCurRow->GetValue(4));
		strFrom = VarString(pCurRow->GetValue(5));
		strResolution = VarString(pCurRow->GetValue(6));
		//(r.farnworth 2013-01-7) PLID 54667 Resolution must be lowercase or Protus will not accept it
		strResolution.MakeLower();

		// (j.jones 2009-05-01 10:07) - PLID 34132 - add our password to the array
		_variant_t varPass = EncryptStringToVariant(strPass);
		VARIANT variantPass = varPass.Detach();
		aryVarPasswords.Add(variantPass);

		//Positive IDs are existing, edited data.  Negative IDs are new records.
		if(nID > 0) {
			//Update data with them, we're modifying an existing
			// (j.jones 2009-04-30 14:19) - PLID 34132 - used AES encryption on the password

			sql.Add("UPDATE FaxConfigT SET ConfigName = {STRING}, Username = {STRING}, Password = {VARBINARY}, "
				"FromName = {STRING}, Resolution = {STRING}, FaxType = {INT} WHERE ID = {INT} ", 
				strName, strUser, variantPass, strFrom, strResolution, nFaxType, nID);

			//(r.farnworth 2013-04-22) PLID 54667 - If this row is the one that we checked, we're going to save it
			if(pCurRow == checkRow)
			{
				sql.Add("SET @NewConfigID = {INT} ", nID);
			}
		} else {
			//Add a new record
			// (j.jones 2009-04-30 14:19) - PLID 34132 - used AES encryption on the password

			sql.Add("INSERT INTO FaxConfigT (UserID, ConfigName, Username, Password, FromName, Resolution, FaxType) VALUES "
				"({INT}, {STRING}, {STRING}, {VARBINARY}, {STRING}, {STRING}, {INT}) ", 
				GetCurrentUserID(), strName, strUser, variantPass, strFrom, strResolution, nFaxType, nID);

			//(r.farnworth 2013-04-22) PLID 54667 - If this row is the one that we checked, we're going to save it
			if(pCurRow == checkRow)
			{
				sql.Add("SET @NewConfigID = SCOPE_IDENTITY() ");
			}
				
		}		
		
		//advance to the next row
		pCurRow = m_pConfigList->FindAbsoluteNextRow(pCurRow, VARIANT_TRUE);
	}

	//If we have some text, save it
	if(!sql.IsEmpty()) {
		sql.Add("SET NOCOUNT OFF "
			"SELECT @NewConfigID AS NewID ");

		_RecordsetPtr prs = sql.CreateRecordset(GetRemoteData());
		if(!prs->eof) {
			confID = AdoFldLong(prs, "NewID", -1);
		}

		//(r.farnworth 2013-04-22) PLID 54667 - Allows for us to remember freshly made configs.
		SetRemotePropertyInt("ERxFaxConfig", confID, 0, GetCurrentUserName());

		// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
		//ExecuteParamSqlBatch(GetRemoteData(), strBatchSql, params);
	}

	// (j.jones 2009-05-01 09:42) - PLID 34132 - now clear the variants
	for(int i=0; i<aryVarPasswords.GetSize(); i++) {
		VariantClear(&(VARIANT)aryVarPasswords.GetAt(i));
	}

	//Success!
	return true;
}

//Close button, force a save of the current interface.
void CFaxSetupDlg::OnOK() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr lRow = m_pConfigList->GetLastRow();
		NXDATALIST2Lib::IRowSettingsPtr fRow = m_pConfigList->GetFirstRow();

		if(lRow && fRow)
		{
			long lFaxID = lRow->GetValue(0);
			long fFaxID = fRow->GetValue(0);

			//(r.farnworth 2013-01-25) PLID 54667 - If theres only one configuration, force it to be the default
			if(lFaxID == fFaxID)
			{
				checkRow = lRow;
			}

			if(!SaveData())
			{
				return;
			}

		// (r.farnworth 2013-04-22) PLID 54667 - If either the pointers are null, we set the value to -1
		} else if (lRow == NULL || fRow == NULL){
			SetRemotePropertyInt("ERxFaxConfig", -1, 0, GetCurrentUserName());
		}

		
		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

//There is no cancel, but we want it to save.
void CFaxSetupDlg::OnCancel() 
{
	OnOK();
}

BEGIN_EVENTSINK_MAP(CFaxSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFaxSetupDlg)
	ON_EVENT(CFaxSetupDlg, IDC_FAX_SERVICE_LIST, 1 /* SelChanging */, OnSelChangingFaxServiceList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFaxSetupDlg, IDC_FAX_SERVICE_LIST, 16 /* SelChosen */, OnSelChosenFaxServiceList, VTS_DISPATCH)
	ON_EVENT(CFaxSetupDlg, IDC_FAX_CONFIG_LIST, 16 /* SelChosen */, OnSelChosenFaxConfigList, VTS_DISPATCH)
	ON_EVENT(CFaxSetupDlg, IDC_FAX_CONFIG_LIST, 1 /* SelChanging */, OnSelChangingFaxConfigList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//The user is trying to change something in the dropped down state.  Don't let them pick nothing.
void CFaxSetupDlg::OnSelChangingFaxServiceList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel);

		//Do not let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll("Error in OnSelChangingFaxServiceList");
}

void CFaxSetupDlg::OnSelChosenFaxServiceList(LPDISPATCH lpRow) 
{
	try {
		//first, ensure there's a row.  This should be impossible to be NULL due to the OnSelChanging code
		if(lpRow == NULL) {
			AfxThrowNxException("Invalid row chosen in OnSelChosenFaxServiceList, your data may not have been saved.  Please close this dialog and try again.");
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	} NxCatchAll("Error in OnSelChosenFaxServiceList");
}

void CFaxSetupDlg::OnFaxConfigAdd() 
{
	try {
		//Save everything in the interface to the datalist
		SyncInterfaceToDatalist();

		//Start a new row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetNewRow();
		pRow->PutValue(0, (long)GetNewID());
		pRow->PutValue(1, (long)esfsMyFax);	//Only option at this point
		pRow->PutValue(2, _bstr_t(""));
		pRow->PutValue(3, _bstr_t(""));
		pRow->PutValue(4, _bstr_t(""));
		pRow->PutValue(5, _bstr_t(""));
		pRow->PutValue(6, _bstr_t("High"));	//Default to High
		m_pConfigList->AddRowAtEnd(pRow, NULL);

		//Tell the row it has been selected
		m_pConfigList->PutCurSel(pRow);
		OnSelChosenFaxConfigList(pRow);

		//Re-enable the interface
		EnsureControls();
		m_editName.SetFocus(); //(r.farnworth 2013-04-23) PLID 54667

	} NxCatchAll("Error in OnFaxConfigAdd");
}

void CFaxSetupDlg::OnFaxConfigRemove() 
{
	try {
		//Removals are instant deletion from the database, not done on save
		NXDATALIST2Lib::IRowSettingsPtr pRowToDelete = m_pConfigList->GetCurSel();
		if(pRowToDelete == NULL) {
			return;
		}

		//Confirm it
		if(AfxMessageBox("Are you sure you wish to remove this configuration?", MB_YESNO) != IDYES) {
			return;
		}

		long nID = VarLong(pRowToDelete->GetValue(0));

		if(nID < 0) {
			//Subzero IDs are new records, and need no deletion
		}
		else {
			//Remove it from the database
			ExecuteParamSql("DELETE FROM FaxConfigT WHERE ID = {INT}", nID);
		}


		//Before we remove it from the datalist, it's simpler with our loading scheme if we switch to the next 
		//	row, load it, then delete.

		//attempt to reload - try to move to the next row
		NXDATALIST2Lib::IRowSettingsPtr pSelRow = pRowToDelete->GetNextRow();
		if(pSelRow == NULL) {
			//OK there was no next row... let's try previous
			pSelRow = pRowToDelete->GetPreviousRow();
		}
		if(pSelRow != NULL) {
			//Set this row to be current
			m_pConfigList->PutCurSel(pSelRow);
			OnSelChosenFaxConfigList(pSelRow);
		}
		else {
			//There are no other rows
			//Clear all the text
			SetDlgItemText(IDC_FAX_CONFIG_NAME, "");
			SetDlgItemText(IDC_FAX_USER, "");
			SetDlgItemText(IDC_FAX_PASSWORD, "");
			SetDlgItemText(IDC_FAX_FROM_NAME, "");

			m_pServiceList->PutCurSel(NULL);
			m_pResolutionList->PutCurSel(NULL);
		}

		//Remove row from datalist
		m_pConfigList->RemoveRow(pRowToDelete);

		//In case we cleared the last row
		EnsureControls();

	} NxCatchAll("Error in OnFaxConfigRemove");
}

void CFaxSetupDlg::OnSelChosenFaxConfigList(LPDISPATCH lpRow) 
{
	try {
		//Force the previous data to save into the appropriate datalist row.
		SyncInterfaceToDatalist();

		//Now load the existing datalist info into the interface.  This will handle
		//	setting the member variable for our current selection.
		SyncDatalistToInterface();

	} NxCatchAll("Error in OnSelChosenFaxConfigList");
}

//Gets an ID to use for a new record in the list.  These IDs are always negative, and
//	should never duplicate.
long CFaxSetupDlg::GetNewID()
{
	//I think it's fastest to just traverse the whole list and save the lowest ID we find.
	long nMin = 0;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pRow != NULL) {
		long nID = VarLong(pRow->GetValue(0));

		//check if it's lower
		if(nID < nMin) {
			nMin = nID;
		}

		//move to next row
		pRow = m_pConfigList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
	}

	//The next ID will be 1 lower
	return nMin - 1;
}

void CFaxSetupDlg::OnChangeFaxConfigName() 
{
	try {
		//We just want to update the datalist to make it look nice
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetCurSel();
		if(pRow == NULL) {
			//shouldn't happen
			AfxThrowNxException("Invalid current selection while changing fax config name.");
			return;
		}

		//Otherwise, just update the name column
		CString strName;
		GetDlgItemText(IDC_FAX_CONFIG_NAME, strName);
		pRow->PutValue(2, _bstr_t(strName));

	} NxCatchAll("Error in OnChangeFaxConfigName");
}

void CFaxSetupDlg::OnSelChangingFaxConfigList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//Do not allow them to choose nothing.
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll("Error in OnSelChangingFaxConfigList");
}

//(r.farnworth PLID 54667) Added so that we can keep track of the user's default fax configuration
void CFaxSetupDlg::OnBnClickedErxSelect()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetCurSel();
		if(pRow){
			long nFaxID = VarLong(pRow->GetValue(0));
			//(r.farnworth 2013-04-23) PLID 54667 - Added to remember which row we have checked
			checkRow = pRow;
		}
	} NxCatchAll("Error in OnBnClickedErxSelect");
}
