// FaxServiceSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "FaxServiceSetupDlg.h"
#include "FileUtils.h"

using namespace ADODB;

// CFaxServiceSetupDlg dialog

//(e.lally 2011-03-18) PLID 42767 - Created.  I basically just copied the FaxSetupDlg except that these values
//  are all global instead of per Practice user and are used by the NexTech Fax Service for downloading received faxes.
//	I tried to make this as generic as possible for any number
//	of fax services, but unfortunately we just don't have any info on how other fax services work.
//	If they are similar, this should work out pretty well, but as we support others, we may need to
//	rework a little bit how things show up in this dialog.  Some options may need to be added / removed
//	depending on the service.

enum serviceConfigListCols
{
	sclcID =0,
	sclcFaxType = 1,
	sclcName = 2, 
	sclcUser = 3, 
	//the password is saved in the datalist unencrypted
	sclcPass = 4, 
	sclcEnabled = 5, 
	sclcDirectory = 6, 

};

IMPLEMENT_DYNAMIC(CFaxServiceSetupDlg, CNxDialog)

CFaxServiceSetupDlg::CFaxServiceSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFaxServiceSetupDlg::IDD, pParent)
{
	m_nCurrentLoadedID = -1;
}

CFaxServiceSetupDlg::~CFaxServiceSetupDlg()
{
}

void CFaxServiceSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaxServiceSetupDlg)
	DDX_Control(pDX, IDC_FAX_SERVICE_CONFIG_NAME, m_editName);
	DDX_Control(pDX, IDC_FAX_SERVICE_USER, m_editUser);
	DDX_Control(pDX, IDC_FAX_SERVICE_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_FAX_SERVICE_TARGET_DIRECTORY, m_editTargetDirectory);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_FAX_SERVICE_CONFIG_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_FAX_SERVICE_CONFIG_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_FAX_SERVICE_ENABLE_CHECK, m_btnEnableChk);
	DDX_Control(pDX, IDC_FAX_SERVICE_BROWSE_BTN, m_btnBrowse);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFaxServiceSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFaxServiceSetupDlg)
	ON_BN_CLICKED(IDC_FAX_SERVICE_CONFIG_ADD, OnFaxServiceConfigAdd)
	ON_BN_CLICKED(IDC_FAX_SERVICE_CONFIG_REMOVE, OnFaxServiceConfigRemove)
	ON_EN_CHANGE(IDC_FAX_SERVICE_CONFIG_NAME, OnChangeFaxServiceConfigName)
	ON_BN_CLICKED(IDC_FAX_SERVICE_BROWSE_BTN, OnBnClickedFaxServiceBrowseBtn)
	ON_BN_CLICKED(IDC_FAX_SERVICE_ENABLE_CHECK, OnBnClickedFaxServiceEnableCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CFaxServiceSetupDlg message handlers

BOOL CFaxServiceSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		//Bind datalist
		m_pServiceList = BindNxDataList2Ctrl(IDC_FAX_SERVICE_SERVICE_LIST, false);
		m_pConfigList = BindNxDataList2Ctrl(IDC_FAX_SERVICE_CONFIG_LIST, false);

		//Set text limits on text fields
		m_editName.SetLimitText(255);
		m_editUser.SetLimitText(255);
		m_editPassword.SetLimitText(255);
		m_editTargetDirectory.SetLimitText(255);

		//Setup button icons
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);

		//Load the list of supported services
		LoadServiceList(m_pServiceList);


		//Load the list of configurations.
		{
			m_pConfigList->Requery();
			m_pConfigList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		}

		//store decrypted passwords in the datalist
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Password FROM FaxServiceConfigT ");
		while(!rs->eof) {

			long nID = AdoFldLong(rs, "ID", -1);
			_variant_t varPassword = rs->Fields->Item["Password"]->Value;

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->FindByColumn(sclcID, (long)nID, m_pConfigList->GetFirstRow(), g_cvarFalse);
			if(pRow) {
				pRow->PutValue(sclcPass, _bstr_t(DecryptStringFromVariant(varPassword)));
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

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFaxServiceSetupDlg::EnsureControls()
{
	BOOL bEnable = TRUE;
	BOOL bEnableCheckbox = TRUE;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetCurSel();
	if(pRow == NULL) {
		//No selection, disable everything.
		bEnable = FALSE;
		bEnableCheckbox = FALSE;
	}
	else {
		//only enable everything if the checkbox is checked
		GetDlgItemCheck(IDC_FAX_SERVICE_ENABLE_CHECK, bEnable);
	}

	//top controls are editable even when this configuration is no longer "enabled"
	GetDlgItem(IDC_FAX_SERVICE_CONFIG_NAME)->EnableWindow(bEnableCheckbox);
	GetDlgItem(IDC_FAX_SERVICE_ENABLE_CHECK)->EnableWindow(bEnableCheckbox);

	//bottom group of controls
	GetDlgItem(IDC_FAX_SERVICE_SERVICE_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_SERVICE_USER)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_SERVICE_PASSWORD)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_SERVICE_TARGET_DIRECTORY)->EnableWindow(bEnable);
	GetDlgItem(IDC_FAX_SERVICE_BROWSE_BTN)->EnableWindow(bEnable);
}

//Takes all data currently on screen and saves it to the datalist, where it will be read when 
//	eventually saving to data.  This should be called before any loading (like changing the selection).
void CFaxServiceSetupDlg::SyncInterfaceToDatalist()
{
	//If our controls are disabled, we haven't started yet.
	if(GetDlgItem(IDC_FAX_SERVICE_CONFIG_NAME)->IsWindowEnabled() == FALSE) {
		return;
	}

	//Pull all off the interface
	eSupportedFaxServices esfsType;
	CString strName, strUser, strPass, strTargetDir;
	GetDlgItemText(IDC_FAX_SERVICE_CONFIG_NAME, strName);
	GetDlgItemText(IDC_FAX_SERVICE_USER, strUser);
	GetDlgItemText(IDC_FAX_SERVICE_PASSWORD, strPass);
	GetDlgItemText(IDC_FAX_SERVICE_TARGET_DIRECTORY, strTargetDir);
	BOOL bEnabled;
	GetDlgItemCheck(IDC_FAX_SERVICE_ENABLE_CHECK, bEnabled);
	NXDATALIST2Lib::IRowSettingsPtr pRowType = m_pServiceList->GetCurSel();
	if(pRowType != NULL) {
		esfsType = (eSupportedFaxServices)VarLong(pRowType->GetValue(0));
	}
	else {
		//This shouldn't happen
		AfxThrowNxException("Invalid fax service chosen.  Please refresh and try again.");
		return;
	}
	

	//Now put it all in the datalist, at the previously selected row
	NXDATALIST2Lib::IRowSettingsPtr pRowList = m_pConfigList->FindByColumn(sclcID, (long)m_nCurrentLoadedID, NULL, VARIANT_FALSE);
	if(pRowList == NULL) {
		//Can't happen, or nothing was loaded to begin with
		AfxThrowNxException("Invalid configuration selected.  Please refresh and try again.");
		return;
	}

	//Load it all into the datalist, where it will later be saved
	pRowList->PutValue(sclcFaxType, (long)esfsType);
	pRowList->PutValue(sclcName, _bstr_t(strName));
	pRowList->PutValue(sclcUser, _bstr_t(strUser));
	//the password is saved in the datalist unencrypted
	pRowList->PutValue(sclcPass, _bstr_t(strPass));
	pRowList->PutValue(sclcEnabled, bEnabled ?  VARIANT_TRUE : VARIANT_FALSE);
	pRowList->PutValue(sclcDirectory, _bstr_t(strTargetDir));
}

//Takes all data currently in the datalist row and puts it into the interface
void CFaxServiceSetupDlg::SyncDatalistToInterface()
{
	eSupportedFaxServices esfsType;
	CString strName, strUser, strPass, strTargetDir;

	//Pull it all out of the interface
	NXDATALIST2Lib::IRowSettingsPtr pRowList = m_pConfigList->GetCurSel();
	if(pRowList == NULL) {
		//Just quit, this is possible
		return;
	}

	//Set the member variable, we're now officially loaded on this item
	m_nCurrentLoadedID = VarLong(pRowList->GetValue(sclcID));

	//Other data
	esfsType = (eSupportedFaxServices)VarLong(pRowList->GetValue(sclcFaxType));
	strName = VarString(pRowList->GetValue(sclcName));
	strUser = VarString(pRowList->GetValue(sclcUser));
	strPass = VarString(pRowList->GetValue(sclcPass));
	BOOL bEnabled = AsBool(pRowList->GetValue(sclcEnabled));
	strTargetDir = VarString(pRowList->GetValue(sclcDirectory));

	//Fill in the interface
	SetDlgItemText(IDC_FAX_SERVICE_CONFIG_NAME, strName);
	SetDlgItemText(IDC_FAX_SERVICE_USER, strUser);
	SetDlgItemText(IDC_FAX_SERVICE_PASSWORD, strPass);
	SetDlgItemText(IDC_FAX_SERVICE_TARGET_DIRECTORY, strTargetDir);
	SetDlgItemCheck(IDC_FAX_SERVICE_ENABLE_CHECK, bEnabled);

	m_pServiceList->SetSelByColumn(0, (long)esfsType);

}

//Saves all data on the interface for each row in the datalist.  All saving is done at the same time, not as
//	the user makes changes.
//Returns true if the save was successful, false if something known was wrong, or may throw an exception.
bool CFaxServiceSetupDlg::SaveData()
{
	//First, sync the current text to the datalist.  All saving is done from there
	SyncInterfaceToDatalist();

	CNxParamSqlArray params;
	CString strBatchSql = BeginSqlBatch();

	NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pConfigList->FindAbsoluteFirstRow(VARIANT_TRUE);

	//track passwords in an array
	CArray<VARIANT, VARIANT> aryVarPasswords;

	while(pCurRow != NULL) {

		//Get the ID
		long nID = VarLong(pCurRow->GetValue(sclcID));


		//Get all the other data
		//
		long nFaxType = VarLong(pCurRow->GetValue(sclcFaxType));
		//Get the fields from the interface
		CString strName, strUser, strPass, strTargetDir;
		strName = VarString(pCurRow->GetValue(sclcName));
		strUser = VarString(pCurRow->GetValue(sclcUser));
		strPass = VarString(pCurRow->GetValue(sclcPass));
		BOOL bEnabled = AsBool(pCurRow->GetValue(sclcEnabled));
		strTargetDir = VarString(pCurRow->GetValue(sclcDirectory));

		//add our password to the array
		_variant_t varPass = EncryptStringToVariant(strPass);
		VARIANT variantPass = varPass.Detach();
		aryVarPasswords.Add(variantPass);

		//Positive IDs are existing, edited data.  Negative IDs are new records.
		if(nID > 0) {
			//Update data with them, we're modifying an existing
			//used AES encryption on the password
			AddParamStatementToSqlBatch(strBatchSql, params, 
				"UPDATE FaxServiceConfigT SET ConfigName = {STRING}, Username = {STRING}, Password = {VARBINARY}, "
				"Enabled = {BOOL}, TargetDirectory = {STRING}, FaxType = {INT} WHERE ID = {INT}", 
				strName, strUser, variantPass, bEnabled, strTargetDir, nFaxType, nID);
		}
		else {
			//Add a new record
			//used AES encryption on the password
			AddParamStatementToSqlBatch(strBatchSql, params, 
				"INSERT INTO FaxServiceConfigT (FaxType, ConfigName, Username, Password, Enabled, TargetDirectory ) values "
				"({INT}, {STRING}, {STRING}, {VARBINARY}, {BOOL}, {STRING})", 
				nFaxType, strName, strUser, variantPass, bEnabled, strTargetDir );
		}

		//advance to the next row
		pCurRow = m_pConfigList->FindAbsoluteNextRow(pCurRow, VARIANT_TRUE);
	}

	//If we have some text, save it
	if(!strBatchSql.IsEmpty()) {
		ExecuteParamSqlBatch(GetRemoteData(), strBatchSql, params);
	}

	//now clear the variants
	for(int i=0; i<aryVarPasswords.GetSize(); i++) {
		VariantClear(&(VARIANT)aryVarPasswords.GetAt(i));
	}

	//Success!
	return true;
}

//Close button, force a save of the current interface.
void CFaxServiceSetupDlg::OnOK() 
{
	try {
		if(!ValidateCurrentConfig()){
			return;
		}
		if(!SaveData()) {
			return;
		}

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

//There is no cancel, but we want it to save.
void CFaxServiceSetupDlg::OnCancel() 
{
	OnOK();
}

BEGIN_EVENTSINK_MAP(CFaxServiceSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFaxServiceSetupDlg)
	ON_EVENT(CFaxServiceSetupDlg, IDC_FAX_SERVICE_SERVICE_LIST, 1 /* SelChanging */, OnSelChangingFaxServiceServiceList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFaxServiceSetupDlg, IDC_FAX_SERVICE_SERVICE_LIST, 16 /* SelChosen */, OnSelChosenFaxServiceServiceList, VTS_DISPATCH)
	ON_EVENT(CFaxServiceSetupDlg, IDC_FAX_SERVICE_CONFIG_LIST, 16 /* SelChosen */, OnSelChosenFaxServiceConfigList, VTS_DISPATCH)
	ON_EVENT(CFaxServiceSetupDlg, IDC_FAX_SERVICE_CONFIG_LIST, 1 /* SelChanging */, OnSelChangingFaxServiceConfigList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//The user is trying to change something in the dropped down state.  Don't let them pick nothing.
void CFaxServiceSetupDlg::OnSelChangingFaxServiceServiceList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel);

		//Do not let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

void CFaxServiceSetupDlg::OnSelChosenFaxServiceServiceList(LPDISPATCH lpRow) 
{
	try {
		//first, ensure there's a row.  This should be impossible to be NULL due to the OnSelChanging code
		if(lpRow == NULL) {
			AfxThrowNxException("Invalid row chosen in OnSelChosenFaxServiceServiceList, your data may not have been saved.  Please close this dialog and try again.");
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	} NxCatchAll(__FUNCTION__);
}

void CFaxServiceSetupDlg::OnFaxServiceConfigAdd() 
{
	try {
		//Save everything in the interface to the datalist
		SyncInterfaceToDatalist();

		if(!ValidateCurrentConfig()){
			return;
		}

		//Start a new row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->GetNewRow();
		pRow->PutValue(sclcID, (long)GetNewID());
		pRow->PutValue(sclcFaxType, (long)esfsMyFax);	//Only option at this point
		pRow->PutValue(sclcName, _bstr_t(""));
		pRow->PutValue(sclcUser, _bstr_t(""));
		pRow->PutValue(sclcPass, _bstr_t(""));
		pRow->PutValue(sclcEnabled, VARIANT_TRUE);
		pRow->PutValue(sclcDirectory, _bstr_t(""));
		m_pConfigList->AddRowAtEnd(pRow, NULL);

		//Tell the row it has been selected
		m_pConfigList->PutCurSel(pRow);
		HandleSelChosenFaxServiceConfigList();

		//Re-enable the interface
		EnsureControls();

	} NxCatchAll(__FUNCTION__);
}

void CFaxServiceSetupDlg::OnFaxServiceConfigRemove() 
{
	try {
		//Removals are instant deletion from the database, not done on save
		NXDATALIST2Lib::IRowSettingsPtr pRowToDelete = m_pConfigList->GetCurSel();
		if(pRowToDelete == NULL) {
			return;
		}

		long nID = VarLong(pRowToDelete->GetValue(sclcID));

		//(e.lally 2011-04-21) PLID 42767 - Make sure it hasn't been used. If so, they have to disable it.
		if(ReturnsRecordsParam("SELECT ID FROM FaxServiceLogT WHERE FaxServiceConfigID = {INT}", nID)){
			AfxMessageBox("This account has fax download records associated with it and cannot be removed.\n"
				"Please uncheck the 'Enable This Configuration For Automated Downloads' box to disable it instead.");
			return;
		}

		//Confirm it
		if(AfxMessageBox("Are you sure you wish to remove this configuration?", MB_YESNO) != IDYES) {
			return;
		}

		if(nID < 0) {
			//Subzero IDs are new records, and need no deletion
		}
		else {
			//Remove it from the database
			ExecuteParamSql("DELETE FROM FaxServiceConfigT WHERE ID = {INT}", nID);
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
			HandleSelChosenFaxServiceConfigList();
		}
		else {
			//There are no other rows
			//Clear all the text
			SetDlgItemText(IDC_FAX_SERVICE_CONFIG_NAME, "");
			SetDlgItemText(IDC_FAX_SERVICE_USER, "");
			SetDlgItemText(IDC_FAX_SERVICE_PASSWORD, "");
			SetDlgItemText(IDC_FAX_SERVICE_TARGET_DIRECTORY, "");

			SetDlgItemCheck(IDC_FAX_SERVICE_ENABLE_CHECK, FALSE);

			m_pServiceList->PutCurSel(NULL);
		}

		//Remove row from datalist
		m_pConfigList->RemoveRow(pRowToDelete);

		//In case we cleared the last row
		EnsureControls();

	} NxCatchAll(__FUNCTION__);
}

void CFaxServiceSetupDlg::OnSelChosenFaxServiceConfigList(LPDISPATCH lpRow) 
{
	try {
		if(lpRow == NULL){
			EnsureControls();
			return;
		}
		if(HasCurrentConfigChanged() && !ValidateCurrentConfig()){
			NXDATALIST2Lib::IRowSettingsPtr pRowList = m_pConfigList->FindByColumn(sclcID, (long)m_nCurrentLoadedID, NULL, VARIANT_TRUE);
			return;
		}

		HandleSelChosenFaxServiceConfigList();

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-21) PLID 42767
void CFaxServiceSetupDlg::HandleSelChosenFaxServiceConfigList()
{
	//Force the previous data to save into the appropriate datalist row.
	SyncInterfaceToDatalist();

	//Now load the existing datalist info into the interface.  This will handle
	//	setting the member variable for our current selection.
	SyncDatalistToInterface();

	//Make sure everything is enabled appropriately
	EnsureControls();
}

//Gets an ID to use for a new record in the list.  These IDs are always negative, and
//	should never duplicate.
long CFaxServiceSetupDlg::GetNewID()
{
	//I think it's fastest to just traverse the whole list and save the lowest ID we find.
	long nMin = 0;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConfigList->FindAbsoluteFirstRow(VARIANT_TRUE);
	while(pRow != NULL) {
		long nID = VarLong(pRow->GetValue(sclcID));

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

void CFaxServiceSetupDlg::OnChangeFaxServiceConfigName() 
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
		GetDlgItemText(IDC_FAX_SERVICE_CONFIG_NAME, strName);
		pRow->PutValue(sclcName, _bstr_t(strName));

	} NxCatchAll(__FUNCTION__);
}

void CFaxServiceSetupDlg::OnSelChangingFaxServiceConfigList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//Do not allow them to choose nothing.
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}


void CFaxServiceSetupDlg::OnBnClickedFaxServiceBrowseBtn()
{
	try {
		CString strTargetDir = "";
		//Browse to the Additional path
		if (BrowseToFolder(&strTargetDir, "Select Folder for the Fax Service Directory", GetSafeHwnd(), NULL, NULL)) {
			//Set the editbox with the returned value
			SetDlgItemText(IDC_FAX_SERVICE_TARGET_DIRECTORY, strTargetDir);
		}

	} NxCatchAll(__FUNCTION__);
}
void CFaxServiceSetupDlg::OnBnClickedFaxServiceEnableCheck()
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-21) PLID 42767 - Checks to see if the current configuration is valid.
//Disabled configuration status ignores most of the validations.
bool CFaxServiceSetupDlg::ValidateCurrentConfig()
{
	CString strName, strUser, strPass, strTargetDir;
	GetDlgItemText(IDC_FAX_SERVICE_CONFIG_NAME, strName);
	GetDlgItemText(IDC_FAX_SERVICE_USER, strUser);
	GetDlgItemText(IDC_FAX_SERVICE_PASSWORD, strPass);
	GetDlgItemText(IDC_FAX_SERVICE_TARGET_DIRECTORY, strTargetDir);
	BOOL bEnabled;
	GetDlgItemCheck(IDC_FAX_SERVICE_ENABLE_CHECK, bEnabled);
	eSupportedFaxServices esfsType;

	//settings that validate on being enabled
	if(bEnabled){
		//User ID is required
		if(strUser.IsEmpty()){
			AfxMessageBox("You must enter a 'User ID' or disable this configuration.", MB_OK|MB_ICONERROR);
			return false;
		}

		
		//fax service type is required
		NXDATALIST2Lib::IRowSettingsPtr pRowType = m_pServiceList->GetCurSel();
		if(pRowType != NULL) {
			esfsType = (eSupportedFaxServices)VarLong(pRowType->GetValue(0));
		}
		else {
			AfxMessageBox("You must select a valid 'Fax Service' or disable this configuration.", MB_OK|MB_ICONERROR);
			return false;
		}

		//destination folder is required
		if(strTargetDir.IsEmpty()){
			AfxMessageBox("You must enter a destination folder for incoming faxes to be saved to or disable this configuration.", MB_OK|MB_ICONERROR);
			return false;
		}

		//warn if user id already in use for this type
		if(ReturnsRecordsParam("SELECT ID FROM FaxServiceConfigT "
			"WHERE ID <> {INT} AND FaxType = {INT} AND Username = {STRING} ", m_nCurrentLoadedID, (long)esfsType, strUser)){
				if(IDYES != AfxMessageBox(FormatString("The User ID '%s' is already configured for this Fax Service.\n"
					"Are you sure you wish to continue?", strUser), MB_YESNO|MB_ICONWARNING)){
					return false;
				}
		}

		//warn for empty password
		if(strPass.IsEmpty() && IDYES != AfxMessageBox("You have not entered a password.\n"
			"Are you sure you wish to continue?", MB_YESNO|MB_ICONWARNING)){ 
				return false;
		}

		//warn if folder doesn't exist
		if(!FileUtils::DoesFileOrDirExist(strTargetDir) && IDYES != AfxMessageBox("The destination folder for incoming faxes could not be found.\n"
			"Are you sure you wish to continue?", MB_YESNO|MB_ICONWARNING)){
				return false;
		}
	}
	else {
		//Nothing to validate when disabled at this time
	}

	return true;
}

//(e.lally 2011-04-21) PLID 42767 - Compares current interface values against saved data so we know if any values were changed.
bool CFaxServiceSetupDlg::HasCurrentConfigChanged()
{
	CString strName, strUser, strPass, strTargetDir;
	GetDlgItemText(IDC_FAX_SERVICE_CONFIG_NAME, strName);
	GetDlgItemText(IDC_FAX_SERVICE_USER, strUser);
	GetDlgItemText(IDC_FAX_SERVICE_PASSWORD, strPass);
	GetDlgItemText(IDC_FAX_SERVICE_TARGET_DIRECTORY, strTargetDir);
	BOOL bEnabled;
	GetDlgItemCheck(IDC_FAX_SERVICE_ENABLE_CHECK, bEnabled);
	eSupportedFaxServices esfsType;
	NXDATALIST2Lib::IRowSettingsPtr pRowType = m_pServiceList->GetCurSel();
	if(pRowType != NULL) {
		esfsType = (eSupportedFaxServices)VarLong(pRowType->GetValue(0));
	}
	else {
		//No fax service? Something must have changed.
		return true;
	}


	//Do we have a record in data that matches this info - ignore the config name
	_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID, Password "
		"FROM FaxServiceConfigT "
		"WHERE ID = {INT} AND FaxType = {INT} AND [Username] = {STRING} AND [Enabled] = {BOOL} AND TargetDirectory = {STRING} "
		, m_nCurrentLoadedID, (long)esfsType, strUser, bEnabled, strTargetDir);
	if(!rs->eof){
		_variant_t varPassword = rs->Fields->Item["Password"]->Value;
		if(DecryptStringFromVariant(varPassword) == strPass){
			//Everything matches
			return false;
		}
	}

	//Something did not match
	return true;
}