// ConcTsMachinePoolDlg.cpp : implementation file
// (d.thompson 2011-10-05) - PLID 45791 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "ConcTsMachinePoolDlg.h"
#include "GlobalDataUtils.h"
using namespace ADODB;

//////////////////////////////////////////////////
//			CConcTSList Implementation			//
//////////////////////////////////////////////////

//Adds a machine with an unknown ID to the list of available machines.
//	Optionally copies the data to a provided pointer to a machine object (this is a copy of the data, the pointer does
//	not reference the internally-maintained machine)
bool CConcTSList::AddNewMachine(IN CString strName, OPTIONAL OUT CConcTSMachine *pMachine)
{
	//Always trim the name
	strName.Trim();

	CConcTSMachine machine;
	machine.nID = -1;			//All "new" machines are given -1 to signify the need to generate an ID
	machine.strName = strName;

	//Optional by the caller
	if(pMachine) {
		//COPY (this does not reassign the pointer) the data from our internal machine to the pointer provided
		*pMachine = machine;
	}

	//Now add our machine to the internally tracked array if it's valid
	CString strReason;	//reason for failure -- we're ignoring in this function
	if(IsNameValid(strName, strReason)) {
		m_aryActiveMachines.Add(machine);
		return true;
	}
	else {
		//Invalid name, fail
		return false;
	}
}

//Given an array of machines and a machine ID, finds the first index of the given machine ID in the array.
//	Returns -1 on no-match condition
long CConcTSList::FindMachineOrdinalByID(CArray<CConcTSMachine, CConcTSMachine&> *pAry, long nID)
{
	for(int i = 0; i < pAry->GetSize(); i++) {
		if(pAry->GetAt(i).nID == nID) {
			//Match!
			return i;
		}
	}

	return -1;
}

//Given an array of machines and a machine name, finds the first index of the given machine ID in the array.
//	Searches are case insensitive
//	Returns -1 on no-match condition
long CConcTSList::FindMachineOrdinalByName(CArray<CConcTSMachine, CConcTSMachine&> *pAry, CString strName)
{
	for(int i = 0; i < pAry->GetSize(); i++) {
		if(pAry->GetAt(i).strName.CompareNoCase(strName) == 0) {
			//Match!
			return i;
		}
	}

	return -1;
}

//Given a machine ID, removes that machine from the active list, and queues it in the removed list
bool CConcTSList::RemoveMachineByID(IN long nID)
{
	//"removing" a machine moves it from the active list to the removed list
	long nIndex = FindMachineOrdinalByID(&m_aryActiveMachines, nID);
	if(nIndex >= 0) {
		//Save a copy of the machine
		CConcTSMachine machine = m_aryActiveMachines.GetAt(nIndex);
		//Remove from active duty
		m_aryActiveMachines.RemoveAt(nIndex);

		//Add it to the "to delete" list
		m_aryDeletedMachines.Add(machine);

		//We have success
		return true;
	}

	//Not found in the list
	return false;
}

//Given a machine name, removes that machine from the active list, and queues it in the removed list
bool CConcTSList::RemoveMachineByName(IN CString strName)
{
	//"removing" a machine moves it from the active list to the removed list
	long nIndex = FindMachineOrdinalByName(&m_aryActiveMachines, strName);
	if(nIndex >= 0) {
		//Save a copy of the machine
		CConcTSMachine machine = m_aryActiveMachines.GetAt(nIndex);
		//Remove from active duty
		m_aryActiveMachines.RemoveAt(nIndex);

		//Add it to the "to delete" list
		m_aryDeletedMachines.Add(machine);

		//We have success
		return true;
	}

	//Not found in the list
	return false;
}

//Is the given name valid for our machine pool?  Compares:
//	- non-empty name requirement
//	- max length requirement
//	- ensures the name is unique
//
//If failure, strMessage contains the reason why
bool CConcTSList::IsNameValid(IN CString strNewName, OUT CString &strMessage)
{
	//Trim it for comparison purposes
	strNewName.Trim();

	//Cannot be empty
	if(strNewName.IsEmpty()) {
		strMessage = "Names cannot be empty, please enter a computer name.";
		return false;
	}

	//Cannot be longer than size
	if(strNewName.GetLength() > 50) {
		//According to MSDN documentation, the real name for a windows networked computer is somewhere around 24 characters... less for NetBIOS names
		strMessage = "Computer names may not exceed 50 characters, please check your name and try again.";
		return false;
	}

	//Do not allow : characters (these are not valid according to the spec anyways) because we are using them as delimiters when transferring data
	//	There are other invalid chars, but we are not going to bother checking them.  They won't match a machine name anyways.
	if(strNewName.Find(":") > -1) {
		strMessage = "Computer names may not include the : character, please check your name and try again.";
		return false;
	}

	//Ensure that the name is not already in use
	if(!IsNameUnique(strNewName)) {
		strMessage = "Computer names must be unique, this name already exists.";
		return false;
	}

	//All tests passed, we're happy
	return true;
}

//Case insensitive search for the given name to see if it would be unique
//	returns true if the name is not found in existing active lists ('removed' lists are ignored)
//	returns false if the name already exists
bool CConcTSList::IsNameUnique(CString strName)
{
	for(int i = 0; i < m_aryActiveMachines.GetSize(); i++) {
		if(m_aryActiveMachines.GetAt(i).strName.CompareNoCase(strName) == 0) {
			//Exact match already, fail
			return false;
		}
	}

	//no matches, we're good
	return true;
}

//Adds a machine with a known ID to the list of available machines.
//	Optionally copies the data to a provided pointer to a machine object (this is a copy of the data, the pointer does
//	not reference the internally-maintained machine)
void CConcTSList::AddExistingMachine(IN long nID, IN CString strName, OPTIONAL OUT CConcTSMachine *pMachine)
{
	//Always trim the name
	strName.Trim();

	CConcTSMachine machine;
	machine.nID = nID;
	machine.strName = strName;

	//Optional by the caller
	if(pMachine) {
		//COPY (this does not reassign the pointer) the data from our internal machine to the pointer provided
		*pMachine = machine;
	}

	//Now add our machine to the internally tracked array if it's valid
	CString strReason;	//reason for failure -- we're ignoring in this function

	// (d.thompson 2011-10-05) - PLID 45791 - Validity.  This function is designed to load from the database and 
	//	put the data on screen.  So we are not going to test for validity of the data.  If for some reason the
	//	data in the table gets corrupt, we'll just have to deal with it and let it load.  The user will have to
	//	correct it manually somehow.  I can only think of 2 legit cases for this:
	//	1)  A developer manually renames some data and causes bad data.
	//	2)  We change the requirements such that some data that was once valid no longer is.  If we were to do this, 
	//		the dev in charge should be responsible for fixing the data anyways.
	m_aryActiveMachines.Add(machine);
}

//Copies the current active machine list to the given paryMachines
void CConcTSList::GetActiveMachineArray(OUT CArray<CConcTSMachine, CConcTSMachine&> *paryMachines)
{
	for(int i = 0; i < m_aryActiveMachines.GetSize(); i++) {
		paryMachines->Add(m_aryActiveMachines.GetAt(i));
	}
}

void CConcTSList::GetDeletedMachineArray(OUT CArray<CConcTSMachine, CConcTSMachine&> *paryMachines)
{
	for(int i = 0; i < m_aryDeletedMachines.GetSize(); i++) {
		paryMachines->Add(m_aryDeletedMachines.GetAt(i));
	}
}




//////////////////////////////////////
//		CConcTSDataManipulation		//
//////////////////////////////////////

//Given a CConcTSList class, fills it from the database
//	Also requires the client ID to lookup
//	Callers who wish to get a list of machines loaded should provide paryMachines
void CConcTSDataManipulation::LoadFromData(CConcTSList *pList, long nClientID)
{
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, MachineName "
		"FROM NxClientsT_TSLicenseMachinePool "
		"WHERE ClientID = {INT}", nClientID);

	while(!prs->eof) {
		pList->AddExistingMachine(AdoFldLong(prs->Fields, "ID"), AdoFldString(prs->Fields, "MachineName"), NULL);
		prs->MoveNext();
	}
}

//Given a CConcTSList class, saves it to the database
//	The pList is NOT updated with new IDs after the save completes
bool CConcTSDataManipulation::SaveToData(CConcTSList *pList, long nClientID)
{
	//Saving is actually a pretty simple activity.  Since we do not allow any modification of the computers
	//	(you must remove & re-add), we basically come out with only 2 queries:
	//	INSERT all new records (anything with a -1 ID)
	//	DELETE all "removed" records (anything in that array)

	CSqlFragment sql;
	sql += GenerateInsertQuery(pList, nClientID);
	sql += GenerateDeleteQuery(pList);

	//Execute the save
	if(!sql.IsEmpty()) {
		ExecuteParamSql("{SQL}", sql);
	}
	else {
		//No sql to execute, so the save works by default
	}

	return true;
}

CSqlFragment CConcTSDataManipulation::GenerateInsertQuery(CConcTSList *pList, long nClientID)
{
	CSqlFragment sql;

	//Get our currently active machines
	CArray<CConcTSMachine, CConcTSMachine&> aryMachines;
	pList->GetActiveMachineArray(&aryMachines);

	//Iterate through looking for anything new (ID < 0)
	for(int i = 0; i < aryMachines.GetSize(); i++) {
		CConcTSMachine machine = aryMachines.GetAt(i);
		if(machine.nID < 0) {
			//Yep, it's new, so let's save it
			sql += CSqlFragment("INSERT INTO NxClientsT_TSLicenseMachinePool (ClientID, MachineName) values ({INT}, {STRING});", 
				nClientID, machine.strName);
		}
		else {
			//A machine, but not a new machine.  As long as we do not allow renaming, there is
			//	no work that needs done with these.
		}
	}

	return sql;
}

CSqlFragment CConcTSDataManipulation::GenerateDeleteQuery(CConcTSList *pList)
{
	CSqlFragment sql;

	//Get our deleted machines
	CArray<CConcTSMachine, CConcTSMachine&> aryMachines;
	pList->GetDeletedMachineArray(&aryMachines);

	//Iterate through looking for anything at all
	for(int i = 0; i < aryMachines.GetSize(); i++) {
		CConcTSMachine machine = aryMachines.GetAt(i);
		//Anything that is new (ID < 0) were never saved, so we do not need to delete them
		if(machine.nID > 0) {
			sql += CSqlFragment("DELETE FROM NxClientsT_TSLicenseMachinePool WHERE ID = {INT};", 
				machine.nID);
		}
		else {
			//A machine that was added new then removed before ever getting saved
		}
	}

	return sql;
}



//////////////////////////////////
// CConcTsMachinePoolDlg dialog	//
//////////////////////////////////

enum eListColumns {
	elcID = 0,
	elcName,
};

IMPLEMENT_DYNAMIC(CConcTsMachinePoolDlg, CNxDialog)

CConcTsMachinePoolDlg::CConcTsMachinePoolDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConcTsMachinePoolDlg::IDD, pParent)
{
	m_nClientID = -1;
	m_bIsTestAccount = false;
	// (b.eyers 2015-03-10) - PLID 65208
	m_nBConc = -1;
	m_nUConc = -1;
}

CConcTsMachinePoolDlg::~CConcTsMachinePoolDlg()
{
}

void CConcTsMachinePoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_MACHINE, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_MACHINE, m_btnRemove);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CConcTsMachinePoolDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_MACHINE, &CConcTsMachinePoolDlg::OnBnClickedAddMachine)
	ON_BN_CLICKED(IDC_REMOVE_MACHINE, &CConcTsMachinePoolDlg::OnBnClickedRemoveMachine)
END_MESSAGE_MAP()


// CConcTsMachinePoolDlg message handlers

BOOL CConcTsMachinePoolDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Data check -- we must have a client id
		if(m_nClientID < 0) {
			AfxThrowNxException("Invalid client ID passed to machine pool dialog - %li", m_nClientID);
		}

		//Setup controls
		m_pList = BindNxDataList2Ctrl(IDC_MACHINE_POOL_LIST, false);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Handle current permissions
		ReflectUIPermissions();

		//Load the info from the database
		LoadSourceData();

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConcTsMachinePoolDlg::ReflectUIPermissions()
{
	//All permissions are based on support bought flags (per support management)
	// (b.eyers 2015-03-05) - PLID 65137 - Allow access to the Machine Pool dialog if the user has the Support TS Machine Pool permission
	BOOL bEnable = FALSE;
	if ((GetCurrentUserPermissions(bioSupportBought, sptWrite) || GetCurrentUserPermissions(bioSupportTSMachinePool, sptWrite)) || m_bIsTestAccount) {
		bEnable = TRUE;
	}

	m_btnOK.EnableWindow(bEnable);
	m_btnAdd.EnableWindow(bEnable);
	m_btnRemove.EnableWindow(bEnable);
	m_pList->PutReadOnly(bEnable ? VARIANT_FALSE : VARIANT_TRUE);
}

void CConcTsMachinePoolDlg::LoadSourceData()
{
	//Load the list data from the database appropriately
	CConcTSDataManipulation::LoadFromData(&m_data, m_nClientID);

	//Now pull the array of active machines so we can actually load them on the interface
	AddActiveMachinesToInterface();
}

void CConcTsMachinePoolDlg::AddActiveMachinesToInterface()
{
	CArray<CConcTSMachine, CConcTSMachine&> aryMachines;
	m_data.GetActiveMachineArray(&aryMachines);

	//Now put all this in the interface
	for(int i = 0; i < aryMachines.GetSize(); i++) {
		AddMachineToUI(&aryMachines.GetAt(i));
	}
}

bool CConcTsMachinePoolDlg::SaveToData()
{
	return CConcTSDataManipulation::SaveToData(&m_data, m_nClientID);
}

void CConcTsMachinePoolDlg::OnOK()
{
	try {
		if(SaveToData()) {
			//Success, you may proceed
			CDialog::OnOK();
		}

	} NxCatchAll(__FUNCTION__);
}

void CConcTsMachinePoolDlg::OnCancel()
{
	try {
		//No work to be done
		CDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}

void CConcTsMachinePoolDlg::OnBnClickedAddMachine()
{
	try {

		// (b.eyers 2015-03-10) - PLID 65208 - Warning if the ratio of TS licenses to machine pool isn't right (25 to 1, really 20 for the warning though)
		CArray<CConcTSMachine, CConcTSMachine&> aryMachines;
		m_data.GetActiveMachineArray(&aryMachines);
		long nActiveCount, nCount;
		nActiveCount = aryMachines.GetSize();
		if (m_nBConc > m_nUConc) {
			nCount = m_nBConc;
		}
		else
			nCount = m_nUConc;

		if (nCount == 0) {
			CString strWarn;
			strWarn.Format("This feature is only used for clients with Concurrent TS Licenses. "
				"This client does not currently have any Concurrent TS Licenses.\n\n"
				"Are you sure you want to add a new server?");
			if (IDNO == MessageBox(strWarn, "Warning", MB_ICONQUESTION | MB_YESNO)) {
				return;
			}
		}
		else if ((nCount / 20) < nActiveCount) {
			CString strWarn;
			strWarn.Format("This client has %li Concurrent TS Licenses. "
				"New Terminal Servers are typically not needed until the client has about 25 licenses per server.\n\n"
				"Are you sure you want to add a new server?", nCount);
			if (IDNO == MessageBox(strWarn, "Warning", MB_ICONQUESTION | MB_YESNO)) {
				return;
			}
		}

		CString strName;
		while(strName == "" && InputBox(this, "Please enter the machine name", strName, "") == IDOK)
		{
			//Ensure the name is unique for this customer
			if(!IsValidName(strName)) {
				//loop again
				strName = "";
			}
			else {
				//Name is unique, so we can save it
				if(AddMachineByName(strName)) {
					//success!
				}
				else {
					AfxMessageBox("Failed to add machine to the list.  Please reload and try again.");
				}

				//Jump out of the loop
				break;
			}
		}

	} NxCatchAll(__FUNCTION__);
}

void CConcTsMachinePoolDlg::OnBnClickedRemoveMachine()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetCurSel();
		if(pRow == NULL) {
			//no selection, just quit
			return;
		}

		//confirm
		if(AfxMessageBox("Are you sure you wish to remove this machine?  It will no longer operate as a concurrently licensed Terminal Server.", MB_YESNO) != IDYES) {
			return;
		}

		//Get the values from the list
		long nID = VarLong(pRow->GetValue(elcID));
		CString strName = VarString(pRow->GetValue(elcName));

		if(!RemoveMachine(nID, strName)) {
			AfxMessageBox("Failed to remove the machine, it may have already been deleted.  Please reload and try again.");
			return;
		}

		//Just remove it ourselves from the interface
		m_pList->RemoveRow(pRow);

	} NxCatchAll(__FUNCTION__);
}

void CConcTsMachinePoolDlg::AddMachineToUI(CConcTSMachine *pMachine)
{
	//Must have a valid machine
	if(pMachine == NULL) {
		AfxThrowNxException("Cannot add machine -- NULL pointer");
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
	pRow->PutValue(elcID, pMachine->nID);					//All "new" data is flagged as -1
	pRow->PutValue(elcName, _bstr_t(pMachine->strName));	//Setup the name of the machine
	m_pList->AddRowSorted(pRow, NULL);
}

//Is the name valid?  This function will alert the user why the check failed if necessary
//Compares the given name with what is in the list of available names already to ensure uniqueness
//Ensures not empty
//Ensures length
//Add any future validation here
bool CConcTsMachinePoolDlg::IsValidName(CString strName)
{
	CString strMessage;
	if(!m_data.IsNameValid(strName, strMessage)) {
		AfxMessageBox(strMessage);
		return false;
	}

	return true;
}

//Adds a new machine with the given name
bool CConcTsMachinePoolDlg::AddMachineByName(CString strName)
{
	CConcTSMachine machine;
	if(m_data.AddNewMachine(strName, &machine)) {
		//We added it to our data list, so let's put it in the UI
		AddMachineToUI(&machine);
		return true;
	}

	//Some kind of failure event
	return false;
}

//Removes a machine with the given info from the list of available machines.
//	nID is used first as a comparison for previously-saved data.
//	If nID is negative (indicating a new record), the search will be done by name.
//	Names are required to be unique.
bool CConcTsMachinePoolDlg::RemoveMachine(long nID, CString strName)
{
	//First, try to remove by ID.  Any "new" values (negative IDs) will not be able to check, so
	//	we'll have to do a name comparison on them.
	if(nID < 0 || !m_data.RemoveMachineByID(nID)) {
		//Either it's new or somehow failed to remove by ID for another reason, try it by name
		if(!m_data.RemoveMachineByName(strName)) {
			return false;
		}
	}

	return true;
}

// (b.eyers 2015-03-10) - PLID 65208 - Machine Pool dlg needs to know the bought and in use for Con. TS
void CConcTsMachinePoolDlg::SetBConc(long nConc)
{
	m_nBConc = nConc;
}

void CConcTsMachinePoolDlg::SetUConc(long nConc)
{
	m_nUConc = nConc;
}
