// NexERxQuickListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexERxQuickListDlg.h"
#include "NexERxAddQuickListDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "NexERxSetupDlg.h"
#include <foreach.h>
#include "MultiSelectDlg.h"

enum EUserQuickList{
	uqlID = 0,
	uqlName,
};

enum EUserMedicationQuickList{
	umqlID = 0,
	umqlOrderIndex,		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Added OrderIndex, which controls the display order of the quicklist meds.
	umqlDrugListID,
	umqlName,
	umqlRefill,
	umqlQuantity,
	umqlSig,
	umqlDosRouteID,		// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added RouteID
	umqlDosRoute,		// (j.fouts 2013-02-01 15:07) - PLID 54986 - Renamed to Route
	umqlDosFreq,
	umqlDosQuantity,
	umqlDosUnitID,		// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added UnitID
	umqlDosUnit,		// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added Unit
	umqlNotes,
};

// CNexERxQuickListDlg dialog
// (b.savon 2013-01-24 14:44) - PLID 54831 - Created

IMPLEMENT_DYNAMIC(CNexERxQuickListDlg, CNxDialog)

CNexERxQuickListDlg::CNexERxQuickListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexERxQuickListDlg::IDD, pParent)
{

}

CNexERxQuickListDlg::~CNexERxQuickListDlg()
{
}

BOOL CNexERxQuickListDlg::OnInitDialog()
{
	
	try{
		
		CNxDialog::OnInitDialog();
		
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnImport.AutoSet(NXB_IMPORT_QUICK_LIST);
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Make the up and down buttons have arrow icons.
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);
		m_nxcBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		SetTitleBarIcon(IDI_QUICK_LIST);
		m_nxdlUserList = BindNxDataList2Ctrl(IDC_NXDL_USER_LIST, true);
		m_nxdlUserMedQuickList = BindNxDataList2Ctrl(IDC_NXDL_USER_MED_QUICK_LIST, false);

		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Initialize the enabled state of the controls.
		EnsureControls();
	}NxCatchAll(__FUNCTION__);

	
	return TRUE;
}

void CNexERxQuickListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_NXC_BACK, m_nxcBack);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_IMPORT_QUICK_LIST, m_btnImport);
	DDX_Control(pDX, IDC_BTN_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_QUICK_LIST_UPDATE, m_btnEdit);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_MOVE_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BTN_MOVE_DOWN, m_btnMoveDown);
}


BEGIN_MESSAGE_MAP(CNexERxQuickListDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNexERxQuickListDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_ADD, &CNexERxQuickListDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_IMPORT_QUICK_LIST, &CNexERxQuickListDlg::OnBnClickedBtnImportQuickList)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CNexERxQuickListDlg::OnBnClickedBtnDelete)
	ON_BN_CLICKED(IDC_BTN_QUICK_LIST_UPDATE, &CNexERxQuickListDlg::OnBnClickedBtnQuickListUpdate)
	ON_BN_CLICKED(IDC_BTN_MOVE_UP, &CNexERxQuickListDlg::OnBnClickedBtnMoveUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_DOWN, &CNexERxQuickListDlg::OnBnClickedBtnMoveDown)
END_MESSAGE_MAP()


// CNexERxQuickListDlg message handlers


void CNexERxQuickListDlg::OnBnClickedOk()
{
	try{
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxQuickListDlg::LoadUserList()
{
	try{

		//Select the current logged in user (if in the list) else the first row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserList->FindByColumn(uqlID, GetCurrentUserID(), m_nxdlUserList->GetFirstRow(), VARIANT_TRUE);
		if( pRow ){
			SelChosenNxdlUserList(pRow);
		}else{
			SelChosenNxdlUserList(m_nxdlUserList->GetFirstRow());
		}

	}NxCatchAll(__FUNCTION__);
}

void CNexERxQuickListDlg::LoadUserQuickList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		//The caller is responsible for passing us a valid row, but let's be safe
		if( pRow == NULL ){
			MessageBox("Invalid user selected. Please select a valid user", "NexTech Practice", MB_ICONERROR);
		}

		CString strUserID;
		long nCurrentUserID = VarLong(pRow->GetValue(uqlID), -1);
		strUserID.Format("%li", nCurrentUserID);
		CString strUserName = VarString(pRow->GetValue(uqlName), "");

		if( nCurrentUserID != -1 ){
			//Call the API
			NexTech_Accessor::_ERxQuickListPtr userQuickList = GetAPI()->GetQuickList(GetAPISubkey(), GetAPILoginToken(), AsBstr(strUserID) /*UserID*/);
			
			if( userQuickList->Medications == NULL ){
				return;
			}

			Nx::SafeArray<IUnknown *> saryMedications(userQuickList->Medications);
			InsertItemIntoQuickListDatalist(saryMedications);
		}

		// (r.gonet 2016-02-10 13:34) - PLID 58689 - The up and down buttons depend on the current selection
		// of the medications list. The medications list just changed. Ensure the controls reflect that.
		EnsureControls();
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CNexERxQuickListDlg, CNxDialog)
ON_EVENT(CNexERxQuickListDlg, IDC_NXDL_USER_LIST, 16, CNexERxQuickListDlg::SelChosenNxdlUserList, VTS_DISPATCH)
ON_EVENT(CNexERxQuickListDlg, IDC_NXDL_USER_LIST, 1, CNexERxQuickListDlg::SelChangingNxdlUserList, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CNexERxQuickListDlg, IDC_NXDL_USER_LIST, 18, CNexERxQuickListDlg::RequeryFinishedNxdlUserList, VTS_I2)
ON_EVENT(CNexERxQuickListDlg, IDC_NXDL_USER_MED_QUICK_LIST, 28, CNexERxQuickListDlg::CurSelWasSetNxdlUserMedQuickList, VTS_NONE)
END_EVENTSINK_MAP()

void CNexERxQuickListDlg::SelChosenNxdlUserList(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			m_nxdlUserList->PutCurSel(pRow);
			m_nxdlUserList->PutComboBoxText(_bstr_t(pRow->GetValue(uqlName)));
			m_nxdlUserMedQuickList->Clear();
			LoadUserQuickList(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxQuickListDlg::OnBnClickedBtnAdd()
{
	try{

		// (b.savon 2013-01-23 10:02) - PLID 54778 - Add to the doctors quick list
		CNexERxAddQuickListDlg dlg(this);
		if( IDOK == dlg.DoModal() ){
			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Made the add dialog construct a quicklist medication object for us,
			// which is easier to work with.
			QuickListRx quickListRx = dlg.GetQuickListRx();
			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Make sure to initialize the order index to be the next available position.
			quickListRx.nOrderIndex = m_nxdlUserMedQuickList->GetRowCount() + 1;

			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Refactored to use the AddQuickListRxs function, which
			// has a lot of commonly used code. Interesting story. I originally had AddQuickListRxs and UpdateQuickListRxs as the same
			// function, which is the primary reason this spot was changed. Split up because the resulting function was ugly and
			// basically had two independent parts anyway, adding and updating.
			SaveQuickListRxResult result = AddQuickListRxs({ quickListRx });
			if (!result.bSuccess) {
				if (result.eErrorCode == SaveQuickListRxResult::EErrorCode::InvalidUser) {
					MessageBox("Invalid selected user. Please select a user and try again.", "NexTech Practice", MB_ICONEXCLAMATION);
				} else if (result.eErrorCode == SaveQuickListRxResult::EErrorCode::SaveFailed) {
					MessageBox("Item was not added to the quick list successfully. Please try again.", "NexTech Practice", MB_ICONERROR);
				} else {
					// Saving failed but no specific error was reported.
				}
				return;
			}
			// Add it to the list
			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Pass the first (and only) resultant quicklist medication.
			NXDATALIST2Lib::IRowSettingsPtr pRow = InsertItemIntoQuickListDatalist(result.vecQuickListMedication[0]);
			if (pRow != nullptr) {
				m_nxdlUserMedQuickList->EnsureRowInView(pRow);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

CString CNexERxQuickListDlg::GetCurrentSelectedUserID()
{
	//Use the selected user
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserList->GetCurSel();
	CString strID = "-1";
	if( pRow ){		
		strID.Format("%li", VarLong(pRow->GetValue(uqlID), -1));
	}
	return strID;
}

void CNexERxQuickListDlg::OnBnClickedBtnImportQuickList()
{
	try{
		// (b.savon 2013-01-24 14:45) - PLID 54758
		//Get the arrays of IDs and Names from the loaded user list
		CArray<long, long> aryIDs;
		CStringArray aryNames;
		PopulateUserArrays(aryIDs, aryNames);

		if( aryIDs.GetCount() <= 1 ){
			MessageBox("You need at least 2 users configured to import a qucklist.", "NexTech Practice", MB_ICONINFORMATION);
			return;
		}

		CString strSelectedIDs;
		CArray<CString, LPCTSTR> aryStringIDs;

		CMultiSelectDlg dlg(this, "NexERxMultiImportQuickList");
		if( IDOK == dlg.Open(
						/* ID Array */
						aryIDs, 
						/* Name Array */
						aryNames,
						/* Description */
						"The list below contains all the users that have configured NexERx prescribing roles. Please select the user(s) you'd like to import from."
						) ){
			strSelectedIDs = dlg.GetMultiSelectIDString(",");
			SplitNames(strSelectedIDs, aryStringIDs, ",");
			CArray<_bstr_t,_bstr_t> aryBstrIDs;
			foreach(CString str, aryStringIDs){
				aryBstrIDs.Add(_bstr_t(str));
			}

			Nx::SafeArray<BSTR> sarySourceIDs = Nx::SafeArray<BSTR>::From(aryBstrIDs);
			CString strCurrentSelectedUserID = GetCurrentSelectedUserID();
			if( strCurrentSelectedUserID != "-1" ){
				NexTech_Accessor::_ERxQuickListMedicationArrayPtr pOutput = GetAPI()->ImportQuickList(GetAPISubkey(), GetAPILoginToken(), sarySourceIDs, AsBstr(strCurrentSelectedUserID));
				if( pOutput->quickListMedications == NULL ){
					MessageBox("Unable to import successfully. Please try again.", "NexTech Practice", MB_ICONERROR);
					return;
				}
				//Add to list
				Nx::SafeArray<IUnknown *> saryMedications(pOutput->quickListMedications);
				InsertItemIntoQuickListDatalist(saryMedications);
			}else{
				MessageBox("Invalid selected user. Please select a user and try again.", "NexTech Practice", MB_ICONERROR);
				return;
			}

			

		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxQuickListDlg::PopulateUserArrays(CArray<long, long> &aryIDs, CStringArray &aryNames)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserList->GetFirstRow();
		while(pRow){
			if( pRow != m_nxdlUserList->GetCurSel() ){
				long nID = VarLong(pRow->GetValue(uqlID));
				CString strName = VarString(pRow->GetValue(uqlName));
				aryIDs.Add(nID);
				aryNames.Add(strName);
			}
			pRow = pRow->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-24 14:47) - PLID 54833
void CNexERxQuickListDlg::OnBnClickedBtnDelete()
{
	try{
		if( m_nxdlUserMedQuickList->GetCurSel() == NULL ){
			MessageBox("There is no item selected. Please select an item to delete.", "NexTech Practice", MB_ICONINFORMATION);
			return;
		}

		if( IDNO == MessageBox("Are you sure you would like to remove the selected item from the user's quick list?",
			"NexTech Practice", MB_ICONQUESTION | MB_YESNO) ){
			return;
		}
		CArray<NexTech_Accessor::_ERxQuickListMedicationPtr, NexTech_Accessor::_ERxQuickListMedicationPtr> aryMedication;
		NexTech_Accessor::_ERxQuickListMedicationPtr med(__uuidof(NexTech_Accessor::ERxQuickListMedication));
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->GetFirstRow();
		CString strCurrentSelectedUserID = GetCurrentSelectedUserID();
		if( !strCurrentSelectedUserID.IsEmpty() && strCurrentSelectedUserID != "-1" ){
			med->userID = AsBstr((LPCTSTR)strCurrentSelectedUserID);
		}else{
			MessageBox("Invalid selected user. Please select a user and try again.", "NexTech Practice", MB_ICONINFORMATION);
			return;
		}

		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Refactoring a bit as I go. Since the move up and move
		// down buttons restored the old selection after reloading the list, I added the same here since it
		// would have been inconsistent, plus was annoying while testing.
		NXDATALIST2Lib::IRowSettingsPtr pSelectedRow = m_nxdlUserMedQuickList->CurSel;
		CString strQuickListID = FormatString("%li", VarLong(pSelectedRow->GetValue(umqlID)));
		med->QuickListID = AsBstr(strQuickListID);
		aryMedication.Add(med);
		
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - See what the ID of the next row is. We'll select that after the deletion occurs.
		NXDATALIST2Lib::IRowSettingsPtr pNextSelectedRow = pSelectedRow->GetNextRow();
		if (pNextSelectedRow == nullptr) {
			pNextSelectedRow = pSelectedRow->GetPreviousRow();
		}
		long nNextSelectedID = (pNextSelectedRow != nullptr ? VarLong(pNextSelectedRow->GetValue(umqlID)) : -1);

		Nx::SafeArray<IUnknown *> saryQuickListMed = Nx::SafeArray<IUnknown *>::From(aryMedication);

		// Call the API
		GetAPI()->DeleteQuickListMedication(GetAPISubkey(), GetAPILoginToken(), saryQuickListMed);

		//Refresh
		SelChosenNxdlUserList(m_nxdlUserList->GetCurSel());

		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Select the next row.
		if (nNextSelectedID != -1) {
			m_nxdlUserMedQuickList->FindByColumn(umqlID, _variant_t(nNextSelectedID, VT_I4), m_nxdlUserMedQuickList->GetFirstRow(), VARIANT_TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Created an overload which inserts a single quicklist medication row into the datalist given an ERxQuickListMedication 
// object from the API. Once done with my editing of this dialog to support customizable quicklist medication ordering, this function was no longer
// crucially needed, but it makes other code easier where it is now used. No passing a safearray of IUNKNOWNs pointers. Ew.
// Returns the datalist row it creates.
NXDATALIST2Lib::IRowSettingsPtr CNexERxQuickListDlg::InsertItemIntoQuickListDatalist(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication)
{
	if (pMedication == nullptr) {
		ThrowNxException("%s : pMedication is NULL", __FUNCTION__);
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->GetNewRow();
	pRow->PutValue(umqlID, atoi(CString((LPCTSTR)pMedication->QuickListID)));
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Add the order index to the row.
	pRow->PutValue(umqlOrderIndex, pMedication->OrderIndex);
	pRow->PutValue(umqlDrugListID, atoi(CString((LPCTSTR)pMedication->DrugListID)));
	pRow->PutValue(umqlName, pMedication->MedicationName);
	pRow->PutValue(umqlRefill, pMedication->Refills);
	pRow->PutValue(umqlQuantity, pMedication->Quantity);
	pRow->PutValue(umqlSig, pMedication->Sig);
	// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added/Renamed method to route
	pRow->PutValue(umqlDosRouteID, atol(CString((LPCTSTR)pMedication->DosageRoute->ID)));
	pRow->PutValue(umqlDosRoute, pMedication->DosageRoute->Route);
	pRow->PutValue(umqlDosFreq, pMedication->DosageFrequency);
	pRow->PutValue(umqlDosQuantity, pMedication->DosageQuantity);
	// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added Unit
	pRow->PutValue(umqlDosUnitID, atol(CString((LPCTSTR)pMedication->DosageUnit->ID)));
	pRow->PutValue(umqlDosUnit, pMedication->DosageUnit->Unit);
	pRow->PutValue(umqlNotes, pMedication->NoteToPharmacist);
	m_nxdlUserMedQuickList->AddRowSorted(pRow, NULL);

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - The medication quicklist has now changed and the
	// states of the move up and move down buttons are potentially invalid. Ensure the controls.
	EnsureControls();

	return pRow;
}

void CNexERxQuickListDlg::InsertItemIntoQuickListDatalist(Nx::SafeArray<IUnknown *> saryMedications)
{
	foreach(NexTech_Accessor::_ERxQuickListMedicationPtr pMedication, saryMedications){
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Use the overload to insert a single quicklist med into the datalist.
		InsertItemIntoQuickListDatalist(pMedication);
	}
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Takes two quicklist medication rows and swaps them in the datalist, thus changing
// their order index. This immediately saves to the database.
void CNexERxQuickListDlg::SwapRowPositions(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2)
{
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Some sanity checks.
	if (pRow1 == nullptr) {
		ThrowNxException("%s : pRow1 is NULL", __FUNCTION__);
	} else if (pRow2 == nullptr) {
		ThrowNxException("%s : pRow2 is NULL", __FUNCTION__);
	} else if (pRow1->IsSameRow(pRow2)) {
		ThrowNxException("%s : pRow1 is pRow2", __FUNCTION__);
	}

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Swap the order indexes of the rows.
	// No need to actually move the rows around though, since SaveRows will remove the rows
	// and re-add them.
	long nRow1OldOrderIndex = VarLong(pRow1->GetValue(umqlOrderIndex));
	long nRow2OldOrderIndex = VarLong(pRow2->GetValue(umqlOrderIndex));
	pRow1->PutValue(umqlOrderIndex, _variant_t(nRow2OldOrderIndex, VT_I4));
	pRow2->PutValue(umqlOrderIndex, _variant_t(nRow1OldOrderIndex, VT_I4));

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Saving needs to be done atomically on both
	// rows at once.
	SaveRows({ pRow1, pRow2 });
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Saves potentially multiple medication datalist
// rows to the database, updating their ERxQuickListT records.
void CNexERxQuickListDlg::SaveRows(std::vector<NXDATALIST2Lib::IRowSettingsPtr> vecRows)
{
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Put the records in the right format.
	std::vector<QuickListRx> vecQuickListRxs;
	for (NXDATALIST2Lib::IRowSettingsPtr pRow : vecRows) {
		vecQuickListRxs.push_back(GetQuickListRxFromRow(pRow));
	}

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Update them. We don't care about the return value.
	UpdateQuickListRxs(vecQuickListRxs);

	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Save the current selection.
	long nSelectedQuickListID = -1;
	if (m_nxdlUserMedQuickList->CurSel != nullptr) {
		nSelectedQuickListID = VarLong(m_nxdlUserMedQuickList->CurSel->GetValue(umqlID), -1);
	}
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Reload the rows.
	m_nxdlUserMedQuickList->Clear();
	LoadUserQuickList(m_nxdlUserList->CurSel);
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - Restore the selection.
	if (nSelectedQuickListID != -1) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->FindByColumn(umqlID, _variant_t(nSelectedQuickListID, VT_I4), m_nxdlUserMedQuickList->GetFirstRow(), VARIANT_TRUE);
		if (pRow != nullptr) {
			m_nxdlUserMedQuickList->EnsureRowInView(pRow);
		}
	}
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Gets a QuickListRx object from a datalist row.
// This is a refactor by encapsulating some ad-hoc code that was sitting in the add button
// handler function. Its useful to have it elsewhere.
QuickListRx CNexERxQuickListDlg::GetQuickListRxFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if (pRow == nullptr) {
		ThrowNxException("%s : pRow is NULL", __FUNCTION__);
	}

	QuickListRx quickListRx;
	quickListRx.nID = VarLong(pRow->GetValue(umqlID), -1);
	quickListRx.nOrderIndex = VarLong(pRow->GetValue(umqlOrderIndex));
	quickListRx.nDrugListID = VarLong(pRow->GetValue(umqlDrugListID), -1);
	quickListRx.strName = VarString(pRow->GetValue(umqlName), "");
	// (r.gonet 2016-02-10 13:34) - PLID 58689 - atol'ing an empty string in past releases was resulting in 0, which
	// caused editing an existing quicklist medication to set the refill quantity to 0 if it was blank before. This was a bug.
	CString strRefill = VarString(pRow->GetValue(umqlRefill), "-1");
	quickListRx.nRefill = strRefill.IsEmpty() ? -1 : atol(strRefill);
	quickListRx.strQuantity = VarString(pRow->GetValue(umqlQuantity), "");
	quickListRx.strSig = VarString(pRow->GetValue(umqlSig), "");
	quickListRx.nDosRouteID = VarLong(pRow->GetValue(umqlDosRouteID), -1);
	quickListRx.strDosFreq = VarString(pRow->GetValue(umqlDosFreq), "-1");
	quickListRx.strDosQuantity = VarString(pRow->GetValue(umqlDosQuantity), "-1");
	quickListRx.nDosUnitID = VarLong(pRow->GetValue(umqlDosUnitID), -1);
	quickListRx.strNotes = VarString(pRow->GetValue(umqlNotes), "");

	return quickListRx;
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Gets an API ERxQuickListMedication given a QuickListRx object.
// QuickListRx is the common intermediate format we are using to hold data from a datalist row or the add quicklist medication dialog.
// This takes that intermediate object and puts it in a form that can be passed to the API. Refactored this by encapsulating
// some code from the add button handler, which was getting the values from the add quicklist medication dialog.
NexTech_Accessor::_ERxQuickListMedicationPtr CNexERxQuickListDlg::GetERxQuickListMedicationFromQuickListRx(QuickListRx quickListRx)
{
	NexTech_Accessor::_ERxQuickListMedicationPtr med(__uuidof(NexTech_Accessor::ERxQuickListMedication));
	med->userID = AsBstr((LPCTSTR)GetCurrentSelectedUserID());
	med->QuickListID = AsBstr(FormatString("%li", quickListRx.nID));
	med->OrderIndex = VarLong(quickListRx.nOrderIndex);
	med->DrugListID = AsBstr(FormatString("%li", quickListRx.nDrugListID));
	med->Refills = AsBstr(quickListRx.nRefill == -1 ? "" : FormatString("%li", quickListRx.nRefill));
	med->Quantity = AsBstr(quickListRx.strQuantity);
	med->Sig = AsBstr(quickListRx.strSig);
	med->DosageFrequency = AsBstr(quickListRx.strDosFreq);
	med->DosageQuantity = AsBstr(quickListRx.strDosQuantity);
	med->NoteToPharmacist = AsBstr(quickListRx.strNotes);

	// (j.fouts 2013-02-01 15:07) - PLID 54986 - Added unit and route
	long nRouteID = quickListRx.nDosRouteID;
	if (nRouteID >= 0) {
		NexTech_Accessor::_NexERxDosageRoutePtr pRoute(__uuidof(NexTech_Accessor::NexERxDosageRoute));
		CString strRouteID;
		strRouteID.Format("%li", nRouteID);
		pRoute->ID = _bstr_t(strRouteID);
		med->DosageRoute = pRoute;
	}

	long nDosageUnitID = quickListRx.nDosUnitID;
	if (nDosageUnitID >= 0) {
		NexTech_Accessor::_NexERxDosageUnitPtr pUnit(__uuidof(NexTech_Accessor::NexERxDosageUnit));
		CString strDosageID;
		strDosageID.Format("%li", nDosageUnitID);
		pUnit->ID = _bstr_t(strDosageID);
		med->DosageUnit = pUnit;
	}

	return med;
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Commits new quicklist medications to the database in one atomic action.
// Returns a SaveQuickListRxResult, which contains a flag indicating whether the save was successful, an error code, and
// the resultant saved ERxQuickListMedication records.
CNexERxQuickListDlg::SaveQuickListRxResult CNexERxQuickListDlg::AddQuickListRxs(std::vector<QuickListRx> vecQuickListRxs)
{
	SaveQuickListRxResult result;
	CArray<NexTech_Accessor::_ERxQuickListMedicationPtr, NexTech_Accessor::_ERxQuickListMedicationPtr> aryNewMedications;

	if (GetCurrentSelectedUserID() == "-1") {
		// Some callers report the errors in message boxes.
		result.bSuccess = false;
		result.eErrorCode = SaveQuickListRxResult::EErrorCode::InvalidUser;
		return result;
	}

	for (const QuickListRx& quickListRx : vecQuickListRxs)
	{
		NexTech_Accessor::_ERxQuickListMedicationPtr pMed = GetERxQuickListMedicationFromQuickListRx(quickListRx);
		aryNewMedications.Add(pMed);
	}

	if (aryNewMedications.GetCount() > 0) { 
		Nx::SafeArray<IUnknown *> saryNewMedications = Nx::SafeArray<IUnknown *>::From(aryNewMedications);
		NexTech_Accessor::_ERxQuickListMedicationArrayPtr results = GetAPI()->AddQuickListMedication(GetAPISubkey(), GetAPILoginToken(), saryNewMedications);

		if (results->quickListMedications == NULL) {
			result.bSuccess = false;
			result.eErrorCode = SaveQuickListRxResult::EErrorCode::SaveFailed;
			return result;
		} else {
			Nx::SafeArray<IUnknown *> saryResults(results->quickListMedications);
			for (NexTech_Accessor::_ERxQuickListMedicationPtr p : saryResults)
			{
				result.vecQuickListMedication.push_back(p);
			}
		}
	}

	if (result.vecQuickListMedication.size() < vecQuickListRxs.size()) {
		// We saved N records. We expect to get back N records.
		result.bSuccess = false;
		result.eErrorCode = SaveQuickListRxResult::EErrorCode::SaveFailed;
	} else {
		result.bSuccess = true;
		result.eErrorCode = SaveQuickListRxResult::EErrorCode::None;
	}
	
	return result;
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Commits updated quicklist medications to the database in one atomic action.
// Returns a SaveQuickListRxResult, which contains a flag indicating whether the save was successful, an error code, and
// the resultant saved ERxQuickListMedication records.
CNexERxQuickListDlg::SaveQuickListRxResult CNexERxQuickListDlg::UpdateQuickListRxs(std::vector<QuickListRx> vecQuickListRxs)
{
	SaveQuickListRxResult result;
	CArray<NexTech_Accessor::_ERxQuickListMedicationPtr, NexTech_Accessor::_ERxQuickListMedicationPtr> aryUpdatedMedications;

	if (GetCurrentSelectedUserID() == "-1") {
		// Some callers report the errors in message boxes.
		result.bSuccess = false;
		result.eErrorCode = SaveQuickListRxResult::EErrorCode::InvalidUser;
		return result;
	}

	for (const QuickListRx& quickListRx : vecQuickListRxs) {
		NexTech_Accessor::_ERxQuickListMedicationPtr pMed = GetERxQuickListMedicationFromQuickListRx(quickListRx);
		aryUpdatedMedications.Add(pMed);
	}

	if (aryUpdatedMedications.GetCount() > 0) {
		Nx::SafeArray<IUnknown *> saryUpdatedMedications = Nx::SafeArray<IUnknown *>::From(aryUpdatedMedications);
		NexTech_Accessor::_ERxQuickListMedicationArrayPtr results = GetAPI()->UpdateQuickListMedication(GetAPISubkey(), GetAPILoginToken(), saryUpdatedMedications);

		if (results->quickListMedications == NULL) {
			result.bSuccess = false;
			result.eErrorCode = SaveQuickListRxResult::EErrorCode::SaveFailed;
			return result;
		} else {
			Nx::SafeArray<IUnknown *> saryResults(results->quickListMedications);
			result.bSuccess = true;
			for (NexTech_Accessor::_ERxQuickListMedicationPtr p : saryResults) {
				result.vecQuickListMedication.push_back(p);
			}
		}
	}

	if (result.vecQuickListMedication.size() < vecQuickListRxs.size()) {
		// We saved N records. We expect to get back N records.
		result.bSuccess = false;
		result.eErrorCode = SaveQuickListRxResult::EErrorCode::SaveFailed;
	} else {
		result.bSuccess = true;
		result.eErrorCode = SaveQuickListRxResult::EErrorCode::None;
	}

	return result;
}

// (b.savon 2013-02-05 12:27) - PLID 55020 - Edit item
void CNexERxQuickListDlg::OnBnClickedBtnQuickListUpdate()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->GetCurSel();
		if( pRow == NULL ){
			MessageBox("Please select an item from the user's quick list to edit.", "NexTech Practice", MB_ICONINFORMATION);
			return;
		}

		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Refactored code to use a common function to create a QuickListRx object
		// from the row.
		QuickListRx qlRx = GetQuickListRxFromRow(pRow);

		CNexERxAddQuickListDlg dlg(this, qlRx);
		if( IDOK == dlg.DoModal() ){
			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Refactored to use a common function UpdateQuickListRxs to save the
			// quicklist medication update.
			SaveQuickListRxResult result = UpdateQuickListRxs({ dlg.GetQuickListRx() });
			if (!result.bSuccess) {
				if (result.eErrorCode == SaveQuickListRxResult::EErrorCode::InvalidUser) {
					MessageBox("Invalid selected user. Please select a user and try again.", "NexTech Practice", MB_ICONEXCLAMATION);
				} else if (result.eErrorCode == SaveQuickListRxResult::EErrorCode::SaveFailed) {
					MessageBox("Unable to edit the selected item successfully. Please try again.", "NexTech Practice", MB_ICONERROR);
				} else {
					// Saving failed somehow but no error was reported.
				}
				return;
			}

			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Save the current selection.
			long nSelectedQuickListID = -1;
			if (m_nxdlUserMedQuickList->CurSel != nullptr) {
				nSelectedQuickListID = VarLong(pRow->GetValue(umqlID), -1);
			}

			//Remove the original
			m_nxdlUserMedQuickList->RemoveRow(pRow);
			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Add it to the list. Use the overload that takes
			// a single object, since that's what we have.
			InsertItemIntoQuickListDatalist(result.vecQuickListMedication[0]);

			// (r.gonet 2016-02-10 13:34) - PLID 58689 - Restore the selection.
			if (nSelectedQuickListID != -1) {
				m_nxdlUserMedQuickList->FindByColumn(umqlID, _variant_t(nSelectedQuickListID, VT_I4), m_nxdlUserMedQuickList->GetFirstRow(), VARIANT_TRUE);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-21 14:55) - PLID 54833 - Prevent the NULL row.
void CNexERxQuickListDlg::SelChangingNxdlUserList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
	
		if( *lppNewSel == NULL ){
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-03-21 15:46) - PLID 54831
void CNexERxQuickListDlg::RequeryFinishedNxdlUserList(short nFlags)
{
	try{

		LoadUserList();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Handles when the user clicks the Move Up button.
void CNexERxQuickListDlg::OnBnClickedBtnMoveUp()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->CurSel;
		if (pRow == nullptr) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pPreviousRow = pRow->GetPreviousRow();
		if (pPreviousRow == nullptr) {
			// They shouldn't have been able to click this.
		} else {
			SwapRowPositions(pRow, pPreviousRow);
		}
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Handles when the user clicks the Move Down button.
void CNexERxQuickListDlg::OnBnClickedBtnMoveDown()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->CurSel;
		if (pRow == nullptr) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
		if (pNextRow == nullptr) {
			// They shouldn't have been able to click this.
		} else {
			SwapRowPositions(pRow, pNextRow);
		}
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Ensures that the control states are consistent with the
// rest of the controls on the dialog.
void CNexERxQuickListDlg::EnsureControls()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserMedQuickList->CurSel;
	m_btnMoveUp.EnableWindow(FALSE);
	m_btnMoveDown.EnableWindow(FALSE);
	if (pRow == nullptr) {
		return;
	} else {
		if (pRow->GetPreviousRow()) {
			m_btnMoveUp.EnableWindow(TRUE);
		}
		if (pRow->GetNextRow()) {
			m_btnMoveDown.EnableWindow(TRUE);
		}
	}
}

// (r.gonet 2016-02-10 13:34) - PLID 58689 - Handles when the datalist selection is changed anywhere, by anyone.
void CNexERxQuickListDlg::CurSelWasSetNxdlUserMedQuickList()
{
	try {
		// (r.gonet 2016-02-10 13:34) - PLID 58689 - Any selection change needs to update the Move Up/Move Down buttons.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}
