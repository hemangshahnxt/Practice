// PreferenceCardsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PreferenceCardsDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "SurgeryProviderLinkDlg.h"
#include "ProcedureDescriptionDlg.h"
#include "AdvPreferenceCardEditDlg.h"

// CPreferenceCardsDlg dialog

// (j.jones 2009-08-20 15:00) - PLID 35338 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//these are saved in data, do NOT change them
enum PreferenceCardItemTypes {

	pcitCPTCode = 1,
	pcitProduct = 2,
	pcitPersonnel = 3,
};

enum PreferenceCardComboColumns {

	pcccID = 0,
	pcccName,
};

enum PreferenceCardItemColumns {
	pcicDetailID = 0,
	pcicItem,
	pcicItemType,
	pcicServiceCode,
	pcicName,
	pcicAmount,
	pcicVisibleCost,
	pcicTrueCost,
	pcicQuantity,
	pcicBillable,
	pcicProcDescription,
	pcicColor,
};

enum ProductComboColumn {

	prodID = 0,
	prodCategory,
	prodSupplier,
	prodName,
	prodBarcode,
	prodPrice,
	prodLastCost,
	prodColor,
};

enum PersonnelComboColumn {

	pccID = 0,
	pccName,
	pccCost,
	pccType,
	pccColor,
};

enum ServiceCodeComboColumn {

	cptID = 0,
	cptCode,
	cptSubCode,
	cptName,
	cptPrice,
	cptColor,
};

// (j.jones 2009-08-31 11:10) - PLID 29531 - added provider filter enum
enum ProviderFilterComboColumn {

	pfccID = 0,
	pfccName,
};

// (j.jones 2009-08-31 16:43) - PLID 17732 - added procedure lists
enum ProcedureListComboColumn {

	plccID = 0,
	plccName,
};

IMPLEMENT_DYNAMIC(CPreferenceCardsDlg, CNxDialog)

CPreferenceCardsDlg::CPreferenceCardsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPreferenceCardsDlg::IDD, pParent),
	m_CPTChecker(NetUtils::CPTCodeT),
	m_InventoryChecker(NetUtils::Products),
	m_ProviderChecker(NetUtils::Providers),
	m_UserChecker(NetUtils::Coordinators),
	m_ContactChecker(NetUtils::ContactsT),
	// (j.jones 2009-08-31 16:09) - PLID 17732 - added procedure checker
	m_ProcedureChecker(NetUtils::AptPurposeT)
{
	m_bEditChanged = FALSE;
}

void CPreferenceCardsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LINK_TO_PROVIDERS, m_btnLinkToProviders);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_RENAME, m_btnRename);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_PREF_CARD_SAVE_AS, m_btnSaveAs);
	DDX_Control(pDX, IDC_PRACTICE_COST_LABEL, m_nxstaticPracticeCostLabel);
	DDX_Control(pDX, IDC_PRACTICE_COST, m_nxstaticPracticeCost);
	DDX_Control(pDX, IDC_PRACTICE_TOTAL_LABEL, m_nxstaticPracticeTotalLabel);
	DDX_Control(pDX, IDC_PRACTICE_TOTAL, m_nxstaticPracticeTotal);
	DDX_Control(pDX, IDC_TOTAL_LABEL, m_nxstaticTotalLabel);
	DDX_Control(pDX, IDC_TOTAL, m_nxstaticTotal);
	DDX_Control(pDX, IDC_NOTES, m_editNotes);
	DDX_Control(pDX, IDC_BTN_ADV_PREF_CARD_EDIT, m_btnAdvPrefCardEdit);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_PREF_CARD_PROCEDURE, m_btnSelectOneProcedure);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_PREF_CARD_PROCEDURE, m_btnUnselectOneProcedure);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_PREF_CARD_PROCEDURES, m_btnUnselectAllProcedures);
	DDX_Control(pDX, IDC_PREF_CARD_PREV, m_btnPrefCardPrev);
	DDX_Control(pDX, IDC_PREF_CARD_NEXT, m_btnPrefCardNext);
}


BEGIN_MESSAGE_MAP(CPreferenceCardsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_LINK_TO_PROVIDERS, OnLinkToProviders)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_PREF_CARD_SAVE_AS, OnPrefCardSaveAs)
	ON_BN_CLICKED(IDC_BTN_ADV_PREF_CARD_EDIT, OnBtnAdvPrefCardEdit)
	ON_EN_CHANGE(IDC_NOTES, OnEnChangeNotes)
	ON_EN_KILLFOCUS(IDC_NOTES, OnEnKillfocusNotes)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_PREF_CARD_PROCEDURE, OnBtnSelectOneProcedure)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_PREF_CARD_PROCEDURES, OnBtnUnselectOneProcedure)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_PREF_CARD_PROCEDURES, OnBtnUnselectAllProcedures)
	ON_BN_CLICKED(IDC_PREF_CARD_PREV, OnBtnPrefCardPrev)
	ON_BN_CLICKED(IDC_PREF_CARD_NEXT, OnBtnPrefCardNext)
END_MESSAGE_MAP()


// CPreferenceCardsDlg message handlers

BOOL CPreferenceCardsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CPreferenceCardsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AddIncreaseSurgeryItems' OR "
			"Name = 'AddIncreasePreferenceCardItems' "
			")",_Q(GetCurrentUserName()));

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRename.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnSaveAs.AutoSet(NXB_NEW);
		m_btnLinkToProviders.AutoSet(NXB_MODIFY);
		// (j.jones 2009-08-31 16:24) - PLID 17732 - added ability to link procedures
		m_btnSelectOneProcedure.AutoSet(NXB_DOWN);
		m_btnUnselectOneProcedure.AutoSet(NXB_UP);
		m_btnUnselectAllProcedures.AutoSet(NXB_UUP);
		//(e.lally 2010-05-04) PLID 37048 - Added left and right buttons for preference cards
		m_btnPrefCardPrev.AutoSet(NXB_LEFT);
		m_btnPrefCardNext.AutoSet(NXB_RIGHT);

		// (j.jones 2011-09-23 17:20) - PLID 42140 - this is now NTEXT, there is no limit
		//m_editNotes.SetLimitText(1000);

		m_pPreferenceCardCombo = BindNxDataList2Ctrl(IDC_PREFERENCE_CARDS_COMBO, false);
		m_pPreferenceCardList = BindNxDataList2Ctrl(IDC_PREFERENCE_CARD_LIST, false);
		m_pProducts = BindNxDataList2Ctrl(IDC_PREF_CARDS_PRODUCTS_COMBO, false);
		m_pPersons = BindNxDataList2Ctrl(IDC_PREF_CARDS_PERSONNEL_COMBO, false);
		m_pServiceCodes = BindNxDataList2Ctrl(IDC_PREF_CARDS_SERVICE_COMBO, false);
		// (j.jones 2009-08-31 11:04) - PLID 29531 - added provider filter
		m_pProviderFilterCombo = BindNxDataList2Ctrl(IDC_PREFERENCE_CARD_PROV_COMBO);
		// (j.jones 2009-08-31 16:11) - PLID 17732 - added procedure lists
		m_pUnselectedProcedureList = BindNxDataList2Ctrl(IDC_UNSELECTED_PREF_CARD_PROCEDURES_LIST, false);
		m_pSelectedProcedureList = BindNxDataList2Ctrl(IDC_SELECTED_PREF_CARD_PROCEDURES_LIST, false);

		{
			//add provider filters for all and unassigned
			IRowSettingsPtr pNewRow = m_pProviderFilterCombo->GetNewRow();
			pNewRow->PutValue(pfccID, (long)-1);
			pNewRow->PutValue(pfccName, " {Show All Preference Cards}");
			m_pProviderFilterCombo->AddRowSorted(pNewRow, NULL);
			pNewRow = m_pProviderFilterCombo->GetNewRow();
			pNewRow->PutValue(pfccID, (long)-2);
			pNewRow->PutValue(pfccName, " {Show Unassigned Preference Cards}");
			m_pProviderFilterCombo->AddRowSorted(pNewRow, NULL);

			//filter on all preference cards
			m_pProviderFilterCombo->SetSelByColumn(pfccID, (long)-1);
		}

		//hide the cost column if they do not have permission to view the personnel cost
		if(!(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
			IColumnSettingsPtr(m_pPersons->GetColumn(pccCost))->PutStoredWidth(0);
			IColumnSettingsPtr(m_pPersons->GetColumn(pccCost))->ColumnStyle = csFixedWidth|csVisible;
		}

		//set the dropdown colors
		m_pProducts->GetColumn(prodColor)->PutFieldName(_bstr_t(AsString((long)GetNxColor(GNC_PRODUCT,-1))));
		m_pProducts->Requery();
		m_pPersons->GetColumn(pccColor)->PutFieldName(_bstr_t(AsString((long)GetNxColor(GNC_PERSONNEL,-1))));
		m_pPersons->Requery();
		m_pServiceCodes->GetColumn(cptColor)->PutFieldName(_bstr_t(AsString((long)GetNxColor(GNC_CPT_CODE,-1))));
		m_pServiceCodes->Requery();

		CString strFrom;
		// (j.jones 2011-03-28 17:06) - PLID 42575 - properly referenced PreferenceCardDetailsT.Billable
		strFrom.Format("(SELECT PreferenceCardDetailsT.ID AS PreferenceCardDetailID, "
			" PreferenceCardDetailsT.PreferenceCardID AS PreferenceCardID,  "
			" CASE WHEN ServiceID Is Null THEN 'Personnel' WHEN CPTCodeT.ID Is Null THEN 'Inventory' ELSE 'Service Code' END AS Item, "
			" Convert(int, CASE WHEN CPTCodeT.ID Is Not Null THEN 1 WHEN ProductT.ID Is Not Null THEN 2 ELSE 3 END) AS ItemType, "
			" (CASE WHEN ProductT.ID Is Not Null THEN Coalesce(ProductT.InsCode, '') ELSE Coalesce(CPTCodeT.Code,'') END) AS ServiceCode, "
			" CASE WHEN PreferenceCardDetailsT.PersonID Is Null THEN Name ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Name, "
			" CASE WHEN PreferenceCardDetailsT.PersonID Is Not Null THEN NULL ELSE Amount END AS Amount, "
			" CASE WHEN ProductT.ID Is Not Null THEN Cost ELSE NULL END AS Cost, "
			" CASE WHEN CPTCodeT.Code Is Null THEN Cost ELSE NULL END AS TrueCost, "
			" PreferenceCardDetailsT.Quantity,  "
			" CASE WHEN PreferenceCardDetailsT.PersonID Is Not Null THEN NULL ELSE PreferenceCardDetailsT.Billable END AS Billable, "
			" Round(Convert(money,Quantity * Amount),2) AS TotalAmount, "
			" '...' AS ProcDesc, "
			" Convert(int, CASE WHEN CPTCodeT.ID Is Not Null THEN %li WHEN ProductT.ID Is Not Null THEN %li ELSE %li END) AS Color "
			" FROM PreferenceCardDetailsT  "
			" LEFT JOIN ServiceT ON PreferenceCardDetailsT.ServiceID = ServiceT.ID "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID  "
			" LEFT JOIN ProductT ON PreferenceCardDetailsT.ServiceID = ProductT.ID  "
			" LEFT JOIN PersonT ON PreferenceCardDetailsT.PersonID = PersonT.ID) AS Q",
			(long)GetNxColor(GNC_CPT_CODE,-1), (long)GetNxColor(GNC_PRODUCT,-1), (long)GetNxColor(GNC_PERSONNEL,-1));

		m_pPreferenceCardList->FromClause = _bstr_t(strFrom);

		//hide the inventory items if they don't have the license
		if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			GetDlgItem(IDC_PREF_CARDS_PRODUCTS_COMBO)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_PREF_CARDS_PRODUCTS_COMBO)->EnableWindow(FALSE);
		}

		//the preference card combo and list will both requery in Refresh()

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
BEGIN_EVENTSINK_MAP(CPreferenceCardsDlg, CNxDialog)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARDS_COMBO, 16, OnSelChosenPreferenceCardsCombo, VTS_DISPATCH)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREF_CARDS_PRODUCTS_COMBO, 16, OnSelChosenPrefCardsProductsCombo, VTS_DISPATCH)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREF_CARDS_PERSONNEL_COMBO, 16, OnSelChosenPrefCardsPersonnelCombo, VTS_DISPATCH)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREF_CARDS_SERVICE_COMBO, 16, OnSelChosenPrefCardsServiceCombo, VTS_DISPATCH)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_LIST, 8, OnEditingStartingPreferenceCardList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_LIST, 9, OnEditingFinishingPreferenceCardList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_LIST, 10, OnEditingFinishedPreferenceCardList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_LIST, 18, OnRequeryFinishedPreferenceCardList, VTS_I2)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_LIST, 6, OnRButtonDownPreferenceCardList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_LIST, 4, OnLButtonDownPreferenceCardList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPreferenceCardsDlg, IDC_PREFERENCE_CARD_PROV_COMBO, 16, OnSelChosenPreferenceCardProvCombo, VTS_DISPATCH)
	ON_EVENT(CPreferenceCardsDlg, IDC_UNSELECTED_PREF_CARD_PROCEDURES_LIST, 3, OnDblClickCellUnselectedPrefCardProceduresList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CPreferenceCardsDlg, IDC_SELECTED_PREF_CARD_PROCEDURES_LIST, 3, OnDblClickCellSelectedPrefCardProceduresList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CPreferenceCardsDlg::OnSelChosenPreferenceCardsCombo(LPDISPATCH lpRow)
{
	try {
		//(e.lally 2010-05-04) PLID 37048 - moved code for handling the selection to its own function
		HandleSelectedPrefCard();
	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnSelChosenPrefCardsProductsCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pProductRow(lpRow);
		if(pProductRow == NULL) {
			return;
		}

		IRowSettingsPtr pPreferenceCardRow = m_pPreferenceCardCombo->CurSel;
		if(pPreferenceCardRow == NULL) {
			m_pProducts->PutCurSel(NULL);
			return;
		}

		long nPreferenceCardID = VarLong(pPreferenceCardRow->GetValue(pcccID));
		long nProductID = VarLong(pProductRow->GetValue(prodID));

		BOOL bWarnProductBillable = FALSE;

		//first see if it is a duplicate
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PreferenceCardDetailsT "
			"WHERE ServiceID = {INT} AND PreferenceCardID = {INT}", nProductID, nPreferenceCardID);
		if(!rs->eof) {
			//it is, now use the preference
			long nAddIncreasePreferenceCardItems = GetRemotePropertyInt("AddIncreasePreferenceCardItems",
				GetRemotePropertyInt("AddIncreaseSurgeryItems", 1, 0, "<None>", true),
				0,"<None>",TRUE);
			
			int nResult = IDNO;
			if(nAddIncreasePreferenceCardItems == 3) {
				nResult = MessageBox("The selected Product is already in the current Preference Card.\n"
					"Would you like to update the existing item's quantity?\n\n"
					"(If 'No', then it will be added as a new line.)","Practice",MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL) {
					//clear the selection
					m_pProducts->PutCurSel(NULL);
					return;
				}
			}

			if(nAddIncreasePreferenceCardItems == 2 || nResult == IDYES) {

				//if we are updating the quantity, do it here, then return, otherwise continue as normal

				long nID = AdoFldLong(rs, "ID");
				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Quantity = Quantity + 1.0 WHERE ID = {INT}", nID);

				//now update the datalist
				IRowSettingsPtr pExistingRow = m_pPreferenceCardList->FindByColumn(pcicDetailID, (long)nID, m_pPreferenceCardList->GetFirstRow(), FALSE);
				if(pExistingRow) {
					pExistingRow->PutValue(pcicQuantity, (double)(VarDouble(pExistingRow->GetValue(pcicQuantity)) + 1.0));
				}

				UpdateTotals();

				//clear the selection
				m_pProducts->PutCurSel(NULL);
				return;
			}
		}
		rs->Close();
		
		//add the record as new
		_RecordsetPtr rsNew = CreateParamRecordset(
			"SET NOCOUNT ON; \r\n"
			""
			"INSERT INTO PreferenceCardDetailsT (PreferenceCardID, ServiceID, Amount, Cost, Quantity, Billable) "
			"VALUES ({INT}, {INT}, Convert(money, {STRING}), Convert(money, {STRING}),  1.0, 1) "
			""
			"SET NOCOUNT OFF; \r\n"
			""
			"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID \r\n"
			""
			"SELECT InsCode FROM ProductT WHERE InsCode <> '' AND ID = {INT} \r\n"
			""
			"SELECT Count(ProductID) AS NumberOfEntries, "
			"Sum(CASE WHEN Billable = 1 THEN 1 ELSE 0 END) AS NumberBillable "
			"FROM ProductLocationInfoT WHERE ProductID = {INT}",
			nPreferenceCardID, nProductID,
			FormatCurrencyForSql(VarCurrency(pProductRow->GetValue(prodPrice), COleCurrency(0,0))),
			FormatCurrencyForSql(VarCurrency(pProductRow->GetValue(prodLastCost), COleCurrency(0,0))),
			nProductID, nProductID);
		long nNewID = -1;
		if(!rsNew->eof) {
			nNewID = AdoFldLong(rsNew, "NewID");
		}
		else {
			ThrowNxException("Adding a product to PreferenceCardDetailsT failed!");
		}

		m_pPreferenceCardList->SetRedraw(VARIANT_FALSE);

		//add the items manually to the list
		IRowSettingsPtr pNewRow = m_pPreferenceCardList->GetNewRow();
		pNewRow->PutValue(pcicDetailID, (long)nNewID);
		pNewRow->PutValue(pcicItem, _bstr_t("Inventory"));		
		pNewRow->PutValue(pcicItemType, (long)pcitProduct);

		rsNew = rsNew->NextRecordset(NULL);
		if(!rsNew->eof) {
			pNewRow->PutValue(pcicServiceCode, rsNew->Fields->Item["InsCode"]->Value);
		}
		else {
			pNewRow->PutValue(pcicServiceCode, g_cvarNull);
		}

		pNewRow->PutValue(pcicName, pProductRow->GetValue(prodName));
		pNewRow->PutValue(pcicAmount, pProductRow->GetValue(prodPrice));
		pNewRow->PutValue(pcicVisibleCost, pProductRow->GetValue(prodLastCost));
		pNewRow->PutValue(pcicTrueCost, pProductRow->GetValue(prodLastCost));
		pNewRow->PutValue(pcicQuantity, (double)1.0);
		
		//warn the user if the product is marked as not billable for any location, then mark it billable accordingly
		rsNew = rsNew->NextRecordset(NULL);
		if(!rsNew->eof) {
			long nNumEntries = AdoFldLong(rsNew, "NumberOfEntries",0);
			long nNumBillable = AdoFldLong(rsNew, "NumberBillable",0);
			if(nNumEntries == nNumBillable) {
				//billable everywhere, so mark it as billable
				pNewRow->PutValue(pcicBillable, g_cvarTrue);
			}
			else if(nNumBillable == 0) {
				//not billable anywhere, so silently mark it as not billable
				pNewRow->PutValue(pcicBillable, g_cvarFalse);
				//we defaulted to billable in data, so now update it to be false
				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Billable = 0 WHERE ID = {INT}", nNewID);
			}
			else if(nNumBillable > 0 && nNumEntries != nNumBillable) {
				//some locations are billable, some are not,
				//mark it billable, but warn accordingly
				pNewRow->PutValue(pcicBillable, g_cvarTrue);
				//warn later, when the list is reloaded
				bWarnProductBillable = TRUE;
			}
		}
		else {
			//should be impossible, from both a query standpoint and bad data,
			//but mark true anyways in this case, since that would usually be
			//the preferred behavior
			pNewRow->PutValue(pcicBillable, g_cvarTrue);
		}
		rsNew->Close();

		pNewRow->PutValue(pcicProcDescription, _variant_t("..."));
		pNewRow->PutValue(pcicColor, (long)GetNxColor(GNC_PRODUCT,-1));
		pNewRow->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		m_pPreferenceCardList->AddRowSorted(pNewRow, NULL);

		//calculate the totals instead of requerying
		UpdateTotals();

		m_pPreferenceCardList->SetRedraw(VARIANT_TRUE);

		//now warn
		if(bWarnProductBillable) {
			AfxMessageBox("Warning: this product is not marked as billable for all locations.\n"
					"This preference card will behave differently per location unless corrected in Inventory.");
		}

		//clear the selection
		m_pProducts->PutCurSel(NULL);

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnSelChosenPrefCardsPersonnelCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pPersonRow(lpRow);
		if(pPersonRow == NULL) {
			return;
		}

		IRowSettingsPtr pPreferenceCardRow = m_pPreferenceCardCombo->CurSel;
		if(pPreferenceCardRow == NULL) {
			m_pPersons->PutCurSel(NULL);
			return;
		}

		long nPreferenceCardID = VarLong(pPreferenceCardRow->GetValue(pcccID));
		long nPersonID = VarLong(pPersonRow->GetValue(pccID));

		COleCurrency cyCost = VarCurrency(pPersonRow->GetValue(pccCost), COleCurrency(0,0));

		//first see if it is a duplicate
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PreferenceCardDetailsT "
			"WHERE PersonID = {INT} AND PreferenceCardID = {INT}", nPersonID, nPreferenceCardID);
		if(!rs->eof) {
			//it is, now use the preference
			long nAddIncreasePreferenceCardItems = GetRemotePropertyInt("AddIncreasePreferenceCardItems",
				GetRemotePropertyInt("AddIncreaseSurgeryItems", 1, 0, "<None>", true),
				0,"<None>",TRUE);
			
			int nResult = IDNO;
			if(nAddIncreasePreferenceCardItems == 3) {
				nResult = MessageBox("The selected Personnel is already in the current Preference Card.\n"
					"Would you like to update the existing person's quantity?\n\n"
					"(If 'No', then it will be added as a new line.)","Practice",MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL) {
					//clear the selection
					m_pPersons->PutCurSel(NULL);
					return;
				}
			}

			if(nAddIncreasePreferenceCardItems == 2 || nResult == IDYES) {

				//if we are updating the quantity, do it here, then return, otherwise continue as normal

				long nID = AdoFldLong(rs, "ID");
				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Quantity = Quantity + 1.0 WHERE ID = {INT}", nID);

				//now update the datalist
				IRowSettingsPtr pExistingRow = m_pPreferenceCardList->FindByColumn(pcicDetailID, (long)nID, m_pPreferenceCardList->GetFirstRow(), FALSE);
				if(pExistingRow) {
					pExistingRow->PutValue(pcicQuantity, (double)(VarDouble(pExistingRow->GetValue(pcicQuantity)) + 1.0));
				}

				UpdateTotals();

				//clear the selection
				m_pPersons->PutCurSel(NULL);
				return;
			}
		}
		rs->Close();
		
		//add the record as new
		_RecordsetPtr rsNew = CreateParamRecordset(
			"SET NOCOUNT ON; \r\n"
			""
			"INSERT INTO PreferenceCardDetailsT (PreferenceCardID, PersonID, Amount, Cost, Quantity, Billable) "
			"VALUES ({INT}, {INT}, Convert(money, '0.00'), Convert(money, {STRING}),  1.0, 0) "
			""
			"SET NOCOUNT OFF; \r\n"
			""
			"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID",
			nPreferenceCardID, nPersonID, FormatCurrencyForSql(cyCost));
		long nNewID = -1;
		if(!rsNew->eof) {
			nNewID = AdoFldLong(rsNew, "NewID");
		}
		else {
			ThrowNxException("Adding a person to PreferenceCardDetailsT failed!");
		}

		m_pPreferenceCardList->SetRedraw(VARIANT_FALSE);

		//add the items manually to the list
		IRowSettingsPtr pNewRow = m_pPreferenceCardList->GetNewRow();
		pNewRow->PutValue(pcicDetailID, (long)nNewID);
		pNewRow->PutValue(pcicItem, _bstr_t("Personnel"));
		pNewRow->PutValue(pcicItemType, (long)pcitPersonnel);
		pNewRow->PutValue(pcicServiceCode, g_cvarNull);
		pNewRow->PutValue(pcicName, pPersonRow->GetValue(pccName));
		pNewRow->PutValue(pcicAmount, g_cvarNull);
		pNewRow->PutCellBackColor(pcicAmount,RGB(230,230,230));
		//don't fill the cost if they can't read it
		if(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead) {
			pNewRow->PutValue(pcicVisibleCost, _variant_t(cyCost));
		}
		else {
			pNewRow->PutValue(pcicVisibleCost, g_cvarNull);
			pNewRow->PutCellBackColor(pcicVisibleCost,RGB(230,230,230));
		}
		pNewRow->PutValue(pcicTrueCost, _variant_t(cyCost));
		pNewRow->PutValue(pcicQuantity, (double)1.0);
		pNewRow->PutValue(pcicBillable, g_cvarNull);
		pNewRow->PutValue(pcicProcDescription, _variant_t(""));
		pNewRow->PutCellBackColor(pcicProcDescription,RGB(230,230,230));
		pNewRow->PutCellLinkStyle(pcicProcDescription, dlLinkStyleFalse);		
		pNewRow->PutValue(pcicColor, (long)GetNxColor(GNC_PERSONNEL,-1));		
		pNewRow->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		m_pPreferenceCardList->AddRowSorted(pNewRow, NULL);

		//calculate the totals instead of requerying
		UpdateTotals();

		m_pPreferenceCardList->SetRedraw(VARIANT_TRUE);

		//clear the selection
		m_pPersons->PutCurSel(NULL);

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnSelChosenPrefCardsServiceCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pServiceRow(lpRow);
		if(pServiceRow == NULL) {
			return;
		}

		IRowSettingsPtr pPreferenceCardRow = m_pPreferenceCardCombo->CurSel;
		if(pPreferenceCardRow == NULL) {
			m_pServiceCodes->PutCurSel(NULL);
			return;
		}

		long nPreferenceCardID = VarLong(pPreferenceCardRow->GetValue(pcccID));
		long nServiceID = VarLong(pServiceRow->GetValue(cptID));

		//check and see if this is an anesthesia code or facility code, and already exists in the charge list
		if(!CheckAllowAddAnesthesiaFacilityCharge(nPreferenceCardID, nServiceID)) {

			//clear the selection
			m_pServiceCodes->PutCurSel(NULL);
			return;
		}

		//first see if it is a duplicate
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PreferenceCardDetailsT "
			"WHERE ServiceID = {INT} AND PreferenceCardID = {INT}", nServiceID, nPreferenceCardID);
		if(!rs->eof) {
			//it is, now use the preference
			long nAddIncreasePreferenceCardItems = GetRemotePropertyInt("AddIncreasePreferenceCardItems",
				GetRemotePropertyInt("AddIncreaseSurgeryItems", 1, 0, "<None>", true),
				0,"<None>",TRUE);
			
			int nResult = IDNO;
			if(nAddIncreasePreferenceCardItems == 3) {
				nResult = MessageBox("The selected Service Code is already in the current Preference Card.\n"
					"Would you like to update the existing item's quantity?\n\n"
					"(If 'No', then it will be added as a new line.)","Practice",MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL) {

					//clear the selection
					m_pServiceCodes->PutCurSel(NULL);
					return;
				}
			}

			if(nAddIncreasePreferenceCardItems == 2 || nResult == IDYES) {

				//if we are updating the quantity, do it here, then return, otherwise continue as normal

				long nID = AdoFldLong(rs, "ID");
				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Quantity = Quantity + 1.0 WHERE ID = {INT}", nID);

				//now update the datalist
				IRowSettingsPtr pExistingRow = m_pPreferenceCardList->FindByColumn(pcicDetailID, (long)nID, m_pPreferenceCardList->GetFirstRow(), FALSE);
				if(pExistingRow) {
					pExistingRow->PutValue(pcicQuantity, (double)(VarDouble(pExistingRow->GetValue(pcicQuantity)) + 1.0));
				}

				UpdateTotals();

				//clear the selection
				m_pServiceCodes->PutCurSel(NULL);
				return;
			}
		}
		rs->Close();
		
		//add the record as new
		_RecordsetPtr rsNew = CreateParamRecordset(
			"SET NOCOUNT ON; \r\n"
			""
			"INSERT INTO PreferenceCardDetailsT (PreferenceCardID, ServiceID, Amount, Cost, Quantity, Billable) "
			"VALUES ({INT}, {INT}, Convert(money, {STRING}), Convert(money, 0), 1.0, 1);"
			""
			"SET NOCOUNT OFF; \r\n"
			""
			"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID",
			nPreferenceCardID, nServiceID, FormatCurrencyForSql(VarCurrency(pServiceRow->GetValue(cptPrice), COleCurrency(0,0))));
		long nNewID = -1;
		if(!rsNew->eof) {
			nNewID = AdoFldLong(rsNew, "NewID");
		}
		else {
			ThrowNxException("Adding a service to PreferenceCardDetailsT failed!");
		}
		rsNew->Close();


		m_pPreferenceCardList->SetRedraw(VARIANT_FALSE);

		//add the items manually to the list
		IRowSettingsPtr pNewRow = m_pPreferenceCardList->GetNewRow();
		pNewRow->PutValue(pcicDetailID, (long)nNewID);
		pNewRow->PutValue(pcicItem, _bstr_t("Service Code"));		
		pNewRow->PutValue(pcicItemType, (long)pcitCPTCode);
		pNewRow->PutValue(pcicServiceCode, pServiceRow->GetValue(cptCode));
		pNewRow->PutValue(pcicName, pServiceRow->GetValue(cptName));
		pNewRow->PutValue(pcicAmount, pServiceRow->GetValue(cptPrice));
		pNewRow->PutValue(pcicVisibleCost, g_cvarNull);
		pNewRow->PutCellBackColor(pcicVisibleCost, RGB(230,230,230));
		pNewRow->PutValue(pcicTrueCost, g_cvarNull);
		pNewRow->PutValue(pcicQuantity, (double)1.0);
		pNewRow->PutValue(pcicBillable, g_cvarTrue);
		pNewRow->PutValue(pcicProcDescription, _variant_t("..."));
		pNewRow->PutValue(pcicColor, (long)GetNxColor(GNC_CPT_CODE,-1));		
		pNewRow->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		m_pPreferenceCardList->AddRowSorted(pNewRow, NULL);

		//calculate the totals instead of requerying
		UpdateTotals();

		m_pPreferenceCardList->SetRedraw(VARIANT_TRUE);

		//clear the selection
		m_pServiceCodes->PutCurSel(NULL);

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnLinkToProviders()
{
	try {

		IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

		if(pRow == NULL) {
			AfxMessageBox("Please choose a preference card before linking to providers.");
			return;
		}

		long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));

		CSurgeryProviderLinkDlg dlg(this);
		dlg.m_nPreferenceCardID = nPreferenceCardID;
		if(dlg.DoModal() == IDOK) {
			// (j.jones 2009-08-31 11:24) - PLID 29531 - we need to re-select the provider filter,
			// since the currently filtered list may have changed
			OnSelChosenPreferenceCardProvCombo(m_pProviderFilterCombo->GetCurSel());
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnAdd()
{
	try {

		CString strSql;
		long nID = 1;

		BOOL bSaved = FALSE;
		
		while (!bSaved) {
			CString strNewName;
			long nResult = InputBoxLimited(this, "Enter New Name", strNewName, "",255,false,false,NULL);
			if(nResult == IDOK) {

				//Make sure there is no existing preference card with this name					
				_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PreferenceCardsT WHERE Name = {STRING}", strNewName);
				if(!rs->eof) {
					//allow duplicate names, for different providers
					if(IDNO == MessageBox("A preference card exists with this name. This is allowed, since you can link preference cards to specific providers.\n"
						"Do you want to keep this name for this preference card?", "Nextech", MB_ICONQUESTION|MB_YESNO)) {
						continue;
					}
				}
				rs->Close();

				if(strNewName.GetLength() > 255){
					MessageBox("A preference card's name can have no more than 255 characters.");
					continue;
				}

				_RecordsetPtr rsNew = CreateParamRecordset(
					"SET NOCOUNT ON; \r\n"
					""
					"INSERT INTO PreferenceCardsT (Name, Description) VALUES ({STRING}, '') \r\n"
					""
					"SET NOCOUNT OFF; \r\n"
					""
					"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID \r\n", strNewName);
				long nNewID = -1;
				if(!rsNew->eof) {
					nNewID = AdoFldLong(rsNew, "NewID");
				}
				else {
					ThrowNxException("Adding a new PreferenceCardsT record failed!");
				}

				m_pPreferenceCardList->Clear();
				SetDlgItemText(IDC_NOTES, "");
				m_pUnselectedProcedureList->Clear();
				m_pSelectedProcedureList->Clear();

				//auditing
				long nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1) {
					AuditEvent(-1, "", nAuditID, aeiPreferenceCardCreated, nNewID, "", strNewName, aepMedium, aetCreated);				
				}

				bSaved = TRUE;

				// (j.jones 2009-08-31 11:44) - PLID 29531 - if we are filtering on a provider,
				// we must remove the filter, to display our new preference card
				IRowSettingsPtr pProvRow = m_pProviderFilterCombo->GetCurSel();
				long nProviderID = -1;
				if(pProvRow) {
					nProviderID = VarLong(pProvRow->GetValue(pfccID), -1);
				}

				if(nProviderID > 0) {
					//we're filtering on a provider, and must remove the filter
					m_pProviderFilterCombo->SetSelByColumn(pfccID, (long)-1);
					m_pPreferenceCardCombo->PutWhereClause("");
				}

				m_pPreferenceCardCombo->Requery();
				OnSelChosenPreferenceCardsCombo(m_pPreferenceCardCombo->SetSelByColumn(pcccID, nNewID));

				GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(TRUE);
				
				RefreshButtons();
			}
			else {
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnRename()
{
	try {

		IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

		if(pRow == NULL) {
			return;
		}

		long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));

		BOOL bSaved = FALSE;

		CString strOld = VarString(pRow->GetValue(pcccName), "");
		
		while (!bSaved) {
			CString strNewName;
			long nResult = InputBoxLimited(this, "Enter New Name", strNewName, "",255,false,false,NULL);
			if(nResult == IDOK) {
				//Make sure there is no preference card with this name
				_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PreferenceCardsT WHERE Name = {STRING} AND ID <> {INT}", strNewName, nPreferenceCardID);
				if(!rs->eof) {
					//allow duplicate names, for different providers
					if(IDNO == MessageBox("A preference card exists with this name. This is allowed, since you can link preference cards to specific providers.\n"
						"Do you want to keep this name for this preference card?", "Nextech", MB_ICONQUESTION|MB_YESNO)) {
						continue;
					}
				}
				rs->Close();

				if(strNewName.GetLength() > 255){
					MessageBox("A preference card's name can have no more than 255 characters.");
					continue;
				}
				
				ExecuteParamSql("UPDATE PreferenceCardsT SET Name = {STRING} WHERE ID = {INT}", strNewName, nPreferenceCardID);

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1) {
					AuditEvent(-1, "", nAuditID, aeiPreferenceCardName, nPreferenceCardID, strOld, strNewName, aepMedium, aetChanged);
				}

				bSaved = TRUE;

				//we need to requery, due to provider linking
				m_pPreferenceCardCombo->Requery();
				m_pPreferenceCardCombo->SetSelByColumn(pcccID, nPreferenceCardID);

				//we're only renaming, so we don't need to reload the rest of the tab

			}
			else
				return;
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnDelete()
{
	try {

		IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

		if(pRow == NULL) {
			return;
		}

		long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));

		if(MessageBox("Are you SURE you wish to remove this preference card?", "Practice", MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
			return;
		}
	
		//for auditing
		CString strOld = VarString(pRow->GetValue(pcccName), "");

		CString strSqlBatch;
		// (j.jones 2009-08-31 15:48) - PLID 17732 - remove links to procedures
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PreferenceCardProceduresT WHERE PreferenceCardID = %li", nPreferenceCardID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PreferenceCardProvidersT WHERE PreferenceCardID = %li", nPreferenceCardID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PreferenceCardDetailsT WHERE PreferenceCardID = %li", nPreferenceCardID);
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PreferenceCardsT WHERE ID = %li", nPreferenceCardID);
		ExecuteSqlBatch(strSqlBatch);

		//auditing
		long nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			AuditEvent(-1, "", nAuditID, aeiPreferenceCardDeleted, nPreferenceCardID, strOld, "<Deleted>", aepMedium, aetDeleted);
		}

		m_pPreferenceCardCombo->RemoveRow(pRow);

		if(m_pPreferenceCardCombo->GetRowCount() == 0){
			m_pPreferenceCardList->Clear();
			SetDlgItemText(IDC_NOTES, "");
			m_pUnselectedProcedureList->Clear();
			m_pSelectedProcedureList->Clear();
			GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(FALSE);
		}
		else {
			//load the first row
			pRow = m_pPreferenceCardCombo->GetFirstRow();
			m_pPreferenceCardCombo->PutCurSel(pRow);
			OnSelChosenPreferenceCardsCombo(pRow);
		}

		RefreshButtons();

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnPrefCardSaveAs()
{
	try {

		IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

		if(pRow == NULL) {
			MessageBox("Please select a preference card from the dropdown first.");
			return;
		}

		long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));

		//prompt them for a new name 
		CString strNewName;
		long nResult = InputBoxLimited(this, "Enter New Name", strNewName, "",255,false,false,NULL);
		if (nResult == IDOK) {

			strNewName.TrimLeft();
			strNewName.TrimRight();
			if (strNewName.IsEmpty()) {
				MessageBox("Please enter a valid name");
				return;
			}

			//Make sure there is no existing preference card with this name					
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM PreferenceCardsT WHERE Name = {STRING}", strNewName);
			if(!rs->eof) {
				//allow duplicate names, for different providers
				if(IDNO == MessageBox("A preference card exists with this name. This is allowed, since you can link preference cards to specific providers.\n"
					"Do you want to keep this name for this preference card?", "Nextech", MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			rs->Close();

			if(strNewName.GetLength() > 255){
				MessageBox("A preference card's name can have no more than 255 characters.");
				return;
			}

			//copy everything from the current preference card into the new one

			CString strNotes;
			GetDlgItemText(IDC_NOTES, strNotes);

			_RecordsetPtr rsNew = CreateParamRecordset(
				"SET NOCOUNT ON; \r\n"
				""
				"INSERT INTO PreferenceCardsT (Name, Description) VALUES ({STRING}, {STRING}) \r\n"
				""
				"DECLARE @nNewID INT; \r\n"
				"SET @nNewID = (SELECT Convert(int, SCOPE_IDENTITY()) AS NewID) \r\n"
				""
				"INSERT INTO PreferenceCardDetailsT (PreferenceCardID, ServiceID, PersonID, Amount, Cost, Quantity, Billable) "
				"SELECT @nNewID, ServiceID, PersonID, Amount, Cost, Quantity, Billable "
				"	FROM PreferenceCardDetailsT "
				"	WHERE PreferenceCardDetailsT.PreferenceCardID = {INT} \r\n"
				""
				"INSERT INTO PreferenceCardProvidersT (PreferenceCardID, ProviderID) "
				"SELECT @nNewID, ProviderID FROM PreferenceCardProvidersT "
				"	WHERE PreferenceCardProvidersT.PreferenceCardID = {INT} \r\n"
				""
				"SET NOCOUNT OFF; \r\n"
				""
				"SELECT @nNewID AS NewID \r\n", strNewName, strNotes, nPreferenceCardID, nPreferenceCardID);
			long nNewID = -1;
			if(!rsNew->eof) {
				nNewID = AdoFldLong(rsNew, "NewID");
			}
			else {
				ThrowNxException("Copying a Preference Card failed!");
			}

			m_pPreferenceCardList->Clear();
			SetDlgItemText(IDC_NOTES, "");
			m_pUnselectedProcedureList->Clear();
			m_pSelectedProcedureList->Clear();

			//auditing
			long nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) {
				AuditEvent(-1, "", nAuditID, aeiPreferenceCardCreated, nNewID, "", strNewName, aepMedium, aetCreated);				
			}

			m_pPreferenceCardCombo->Requery();
			OnSelChosenPreferenceCardsCombo(m_pPreferenceCardCombo->SetSelByColumn(pcccID, nNewID));

			GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(TRUE);
			
			RefreshButtons();
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnBtnAdvPrefCardEdit()
{
	try {

		// (j.jones 2009-08-27 16:23) - PLID 35283 - added
		CAdvPreferenceCardEditDlg dlg(this);
		dlg.DoModal();

		//reload the current preference card, if one is selected
		IRowSettingsPtr pRow = m_pPreferenceCardCombo->GetCurSel();
		if(pRow) {
			OnSelChosenPreferenceCardsCombo(pRow);
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnEditingStartingPreferenceCardList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
			case pcicAmount:
				//for persons, we don't want to let them edit the amount,
				if(VarLong(pRow->GetValue(pcicItemType)) == pcitPersonnel) {
					*pbContinue = FALSE;
				}
				break;

			case pcicVisibleCost:
				//for CPT codes, we don't want to let them edit the cost,
				//and if there is a CPT ID, then we have a code
				if(VarLong(pRow->GetValue(pcicItemType)) == pcitCPTCode) {
					*pbContinue = FALSE;
				}
				//if editing the cost for a person, check and see if they have permissions for that
				else if(VarLong(pRow->GetValue(pcicItemType)) == pcitPersonnel
					&& (!(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)
					|| !(GetCurrentUserPermissions(bioContactsDefaultCost) & sptWrite))) {
					*pbContinue = FALSE;
				}
				break;

			//for persons, billable is null, which means no checkbox appears, therefore they can't check it,
			//so we don't need to stop them here
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnEditingFinishingPreferenceCardList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == pcicQuantity && pvarNewValue->vt == VT_R8) {

			if(pvarNewValue->dblVal <= 0.0) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You must have a quantity greater than zero.");
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnEditingFinishedPreferenceCardList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		_variant_t tmpVar = pRow->GetValue(nCol);

		long nDetailID = VarLong(pRow->GetValue(pcicDetailID), -1);

		switch(nCol) {

			case pcicAmount:
				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Amount = Convert(money,{STRING}) WHERE ID = {INT}",
					FormatCurrencyForSql(VarCurrency(tmpVar, COleCurrency(0,0))), nDetailID);
				break;
			case pcicQuantity:
				//parameter recordsets do not support double values
				ExecuteSql("UPDATE PreferenceCardDetailsT SET Quantity = %g WHERE ID = %li",
					VarDouble(tmpVar, 1.0), nDetailID);
				break;
			case pcicVisibleCost:
				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Cost = Convert(money,{STRING}) WHERE ID = {INT}",
					FormatCurrencyForSql(VarCurrency(tmpVar, COleCurrency(0,0))),
					nDetailID);

				//also copy to the TrueCost column
				pRow->PutValue(pcicTrueCost, varNewValue);
				break;
			case pcicBillable:

				//this code must be last because it fires an AfxMessageBox, while permitting the change
				if(varNewValue.vt == VT_BOOL && VarBool(varNewValue) && VarLong(pRow->GetValue(pcicItemType)) == pcitProduct) {
					// if enabling Billable, and the item is a product,
					// warn the user if the product is marked as not billable for any location
					_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ProductID) AS NumberOfEntries, "
						"Sum(CASE WHEN Billable = 1 THEN 1 ELSE 0 END) AS NumberBillable "
						"FROM ProductLocationInfoT WHERE ProductID IN "
						"(SELECT ServiceID FROM SurgeryDetailsT WHERE ID = {INT})", nDetailID);
					if(!rs->eof) {
						long nNumEntries = AdoFldLong(rs, "NumberOfEntries",0);
						long nNumBillable = AdoFldLong(rs, "NumberBillable",0);
						if(nNumEntries != nNumBillable) {
							//the product is not billable for at least one location
							AfxMessageBox("Warning: this product is not marked as billable for all locations.\n"
								"This preferece card will behave differently per location unless corrected in Inventory.");
						}
					}
					rs->Close();			
				}

				ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Billable = {INT} WHERE ID = {INT}",
					VarBool(tmpVar,FALSE) ? 1 : 0, nDetailID);
				break;
		}

		UpdateTotals();

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::UpdateTotals()
{	
	try {

		IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

		if(pRow == NULL) {
			SetDlgItemText(IDC_PRACTICE_COST, "");
			SetDlgItemText(IDC_PRACTICE_TOTAL, "");
			SetDlgItemText(IDC_TOTAL, "");
			GetDlgItem(IDC_PRACTICE_COST)->Invalidate();
			GetDlgItem(IDC_PRACTICE_TOTAL)->Invalidate();
			GetDlgItem(IDC_TOTAL)->Invalidate();
			return;
		}

		long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));
		
		COleCurrency cyAmount(0,0), cyCost(0,0), cyBillable(0,0);

		_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(Round(Convert(money, Amount * Quantity),2)) AS TotalAmount, "
			"Sum(Round(Convert(money, Cost * Quantity),2)) AS TotalCost, "
			"Sum(Round(Convert(money, CASE WHEN Billable = 0 THEN 0 ELSE Amount * Quantity END),2)) AS TotalBillable "
			"FROM PreferenceCardDetailsT  "
			"WHERE PreferenceCardID = {INT}", nPreferenceCardID);
		if(!rs->eof) {
			cyAmount = AdoFldCurrency(rs, "TotalAmount", COleCurrency(0,0));
			cyCost = AdoFldCurrency(rs, "TotalCost", COleCurrency(0,0));
			cyBillable = AdoFldCurrency(rs, "TotalBillable", COleCurrency(0,0));
		}
		rs->Close();

		// Put the totals on screen
		SetDlgItemText(IDC_PRACTICE_COST, FormatCurrencyForInterface(cyCost));
		SetDlgItemText(IDC_PRACTICE_TOTAL, FormatCurrencyForInterface(cyBillable));
		SetDlgItemText(IDC_TOTAL, FormatCurrencyForInterface(cyAmount));
		GetDlgItem(IDC_PRACTICE_COST)->Invalidate();
		GetDlgItem(IDC_PRACTICE_TOTAL)->Invalidate();
		GetDlgItem(IDC_TOTAL)->Invalidate();

	} NxCatchAll("Error in CPreferenceCardsDlg::UpdateTotals");
}

void CPreferenceCardsDlg::Refresh() 
{	
	try {

		BOOL bPrefCardsReloaded = FALSE;

		// Network code that requeries combos
		if(m_CPTChecker.Changed()) {
			//there is no point in maintaining the selection, so just requery
			m_pServiceCodes->Requery();
		}

		if(m_InventoryChecker.Changed()) {
			//there is no point in maintaining the selection, so just requery
			m_pProducts->Requery();
		}

		if(m_UserChecker.Changed()) {
			//there is no point in maintaining the selection, so just requery
			m_pPersons->Requery();
		}

		if(m_ContactChecker.Changed()) {
			//there is no point in maintaining the selection, so just requery
			m_pPersons->Requery();
		}

		// (j.jones 2009-08-31 16:09) - PLID 17732 - added procedure checker
		if(m_ProcedureChecker.Changed()) {
			//do nothing, because every refresh reloads the preference card,
			//and every time we load the preference card we reload the procedure lists
			//m_pUnselectedProcedureList->Requery();
			//m_pSelectedProcedureList->Requery();
		}
		
		if(m_ProviderChecker.Changed()) {
			//there is no point in maintaining the selection, so just requery
			m_pPersons->Requery();

			// (j.jones 2009-08-31 11:05) - PLID 29531 - reload the provider filter
			long nProviderID = -1;
			{
				IRowSettingsPtr pRow = m_pProviderFilterCombo->GetCurSel();			
				if(pRow) {
					nProviderID = VarLong(pRow->GetValue(pfccID));
				}
			}
			m_pProviderFilterCombo->Requery();

			{
				//add provider filters for all and unassigned
				IRowSettingsPtr pNewRow = m_pProviderFilterCombo->GetNewRow();
				pNewRow->PutValue(pfccID, (long)-1);
				pNewRow->PutValue(pfccName, " {Show All Preference Cards}");
				m_pProviderFilterCombo->AddRowSorted(pNewRow, NULL);
				pNewRow = m_pProviderFilterCombo->GetNewRow();
				pNewRow->PutValue(pfccID, (long)-2);
				pNewRow->PutValue(pfccName, " {Show Unassigned Preference Cards}");
				m_pProviderFilterCombo->AddRowSorted(pNewRow, NULL);
			}

			IRowSettingsPtr pRow = NULL;
			if(nProviderID != -1) {
				pRow = m_pProviderFilterCombo->SetSelByColumn(pfccID, nProviderID);
			}

			//this code will handle if pRow is NULL, and also re-filtering
			OnSelChosenPreferenceCardProvCombo(pRow);

			//the above will reload the preference card combo and list, so flag that
			//we do not need to do it later in this function
			bPrefCardsReloaded = TRUE;
		}

		//always reload the preference card list
		// (j.jones 2009-08-31 11:12) - PLID 29531 - not always, the provider tablechecker
		// may have already caused the combo and list to reload
		if(!bPrefCardsReloaded) {
			long nPrefCardID = -1;
			{
				IRowSettingsPtr pRow = m_pPreferenceCardCombo->GetCurSel();			
				if(pRow) {
					nPrefCardID = VarLong(pRow->GetValue(pcccID));
				}
			}
			m_pPreferenceCardCombo->Requery();

			IRowSettingsPtr pRow = NULL;
			if(nPrefCardID != -1) {
				pRow = m_pPreferenceCardCombo->SetSelByColumn(pcccID, nPrefCardID);
			}

			if(pRow == NULL) {
				m_pPreferenceCardCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
				if(m_pPreferenceCardCombo->GetRowCount() == 0){
					m_pPreferenceCardList->Clear();
					SetDlgItemText(IDC_NOTES, "");
					m_pUnselectedProcedureList->Clear();
					m_pSelectedProcedureList->Clear();
					GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(FALSE);
				}
				else {
					pRow = m_pPreferenceCardCombo->GetFirstRow();
					m_pPreferenceCardCombo->PutCurSel(pRow);
				}
			}
			
			if(pRow) {
				OnSelChosenPreferenceCardsCombo(pRow);
			}
		}

		RefreshButtons();

		m_bEditChanged = FALSE;

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::RefreshButtons()
{
	IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

	BOOL bEnable = (pRow != NULL);

	GetDlgItem(IDC_DELETE)->EnableWindow(bEnable);
	GetDlgItem(IDC_RENAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_PREF_CARD_SAVE_AS)->EnableWindow(bEnable);
	GetDlgItem(IDC_LINK_TO_PROVIDERS)->EnableWindow(bEnable);
	GetDlgItem(IDC_PREF_CARDS_PRODUCTS_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_PREF_CARDS_PERSONNEL_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_PREF_CARDS_SERVICE_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_NOTES)->EnableWindow(bEnable);
	// (j.jones 2009-08-31 16:44) - PLID 17732 - enable/disable the procedure controls
	GetDlgItem(IDC_BTN_SELECT_ONE_PREF_CARD_PROCEDURE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_UNSELECT_ONE_PREF_CARD_PROCEDURE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_UNSELECT_ALL_PREF_CARD_PROCEDURES)->EnableWindow(bEnable);
	GetDlgItem(IDC_UNSELECTED_PREF_CARD_PROCEDURES_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_SELECTED_PREF_CARD_PROCEDURES_LIST)->EnableWindow(bEnable);

	if(!bEnable) {
		//if there is no selection, clear our screen
		m_pPreferenceCardList->Clear();
		SetDlgItemText(IDC_NOTES, "");
		m_pUnselectedProcedureList->Clear();
		m_pSelectedProcedureList->Clear();
		//(e.lally 2010-05-04) PLID 37048 - Disable Previos and Next preference card buttons.
		m_btnPrefCardPrev.EnableWindow(FALSE);
		m_btnPrefCardNext.EnableWindow(FALSE);
		return;
	}

	//(e.lally 2010-05-04) PLID 37048 - Update Previous and Next preference card buttons
	if(pRow == m_pPreferenceCardCombo->GetFirstRow()){
		m_btnPrefCardPrev.EnableWindow(FALSE);
	}
	else{
		m_btnPrefCardPrev.EnableWindow(TRUE);
	}
	if(pRow == m_pPreferenceCardCombo->GetLastRow()){
		m_btnPrefCardNext.EnableWindow(FALSE);
	}
	else {
		m_btnPrefCardNext.EnableWindow(TRUE);
	}
}

void CPreferenceCardsDlg::OnEnChangeNotes()
{
	m_bEditChanged = TRUE;
}

void CPreferenceCardsDlg::OnEnKillfocusNotes()
{
	try {

		IRowSettingsPtr pRow = m_pPreferenceCardCombo->CurSel;

		if(pRow == NULL || !m_bEditChanged) {
			return;
		}

		CString strNotes;
		long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));

		GetDlgItemText(IDC_NOTES, strNotes);
		// (j.jones 2011-09-23 17:20) - PLID 42140 - this is now NTEXT, there is no limit
		/*
		if(strNotes.GetLength() > 1000){
			MessageBox("A preference card's notes can have no more than 1000 characters. These notes will be truncated, so please check the new text.");
			strNotes = strNotes.Left(1000);
			SetDlgItemText(IDC_NOTES, strNotes);
		}
		*/

		ExecuteParamSql("UPDATE PreferenceCardsT SET Description = {STRING} WHERE ID = {INT}", strNotes, nPreferenceCardID);

		m_bEditChanged = FALSE;

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-08-25 16:53) - PLID 35338 - this is a holdover from the surgeries behavior,
// it's not really critical on a case history, but a valid warning nonetheless
BOOL CPreferenceCardsDlg::CheckAllowAddAnesthesiaFacilityCharge(long nPreferenceCardID, long nServiceID)
{
	//send exceptions to the called

	//check and see if nServiceID is an anesthesia code or facility code, and already exists in the charge list
	_RecordsetPtr rs = CreateParamRecordset("SELECT Anesthesia, FacilityFee, Name, Code FROM ServiceT "
		"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE ServiceT.ID = {INT} "
		"AND ((Anesthesia = 1 AND UseAnesthesiaBilling = 1) "
		"	OR (FacilityFee = 1 AND UseFacilityBilling = 1)) ", nServiceID);
	if(!rs->eof) {

		//they are adding an anesthesia or facility fee that exists in the list, so warn accordingly
		BOOL bAnesthesia = AdoFldBool(rs, "Anesthesia",FALSE);
		BOOL bFacilityFee = AdoFldBool(rs, "FacilityFee",FALSE);

		if(bAnesthesia) {

			_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM ServiceT "
				"WHERE Anesthesia = 1 AND UseAnesthesiaBilling = 1 AND ID IN (SELECT ServiceID FROM PreferenceCardDetailsT WHERE PreferenceCardID = {INT})", nPreferenceCardID);
			if(!rsCheck->eof) {
				CString str;
				str.Format("You are trying to add Service Code '%s - %s' to the list, which is an Anesthesia Billing charge.\n"
					"However, there is already an Anesthesia Billing charge on this Preference Card. "
					"There should never be more than one Anesthesia Billing charge on a Preference Card.\n\n"
					"If you continue to add this charge, the Anesthesia setup will calculate the same value for each Anesthesia Billing charge in the list.\n\n"
					"Are you sure you wish to add this charge?",
					AdoFldString(rs, "Code",""),
					AdoFldString(rs, "Name",""));

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
			rsCheck->Close();
		}
		else if(bFacilityFee) {
			_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM ServiceT "
				"WHERE FacilityFee = 1 AND UseFacilityBilling = 1 AND ID IN (SELECT ServiceID FROM PreferenceCardDetailsT WHERE PreferenceCardID = {INT})", nPreferenceCardID);
			if(!rsCheck->eof) {
				CString str;
				str.Format("You are trying to add Service Code '%s - %s' to the list, which is a Facility Billing charge.\n"
					"However, there is already a Facility Billing charge on this Preference Card. "
					"There should never be more than one Facility Billing charge on a Preference Card.\n\n"
					"If you continue to add this charge, the Facility Fee setup will calculate the same value for each Facility Billing charge in the list.\n\n"
					"Are you sure you wish to add this charge?",
					AdoFldString(rs, "Code",""),
					AdoFldString(rs, "Name",""));

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
			rsCheck->Close();
		}
	}
	rs->Close();

	return TRUE;
}

long CPreferenceCardsDlg::GetCurrentPreferenceCardID()
{
	try {

		IRowSettingsPtr pPreferenceCardRow = m_pPreferenceCardCombo->CurSel;
		if(pPreferenceCardRow == NULL) {
			return -1;
		}

		return VarLong(pPreferenceCardRow->GetValue(pcccID));

	}NxCatchAll(__FUNCTION__);

	return -1;
}

void CPreferenceCardsDlg::OnRequeryFinishedPreferenceCardList(short nFlags)
{
	try {

		// (j.jones 2009-08-31 12:15) - PLID 29541 - disable or enable the buttons based
		// on whether we have any preference cards
		if(m_pPreferenceCardCombo->GetRowCount() == 0){
			m_pPreferenceCardList->Clear();
			SetDlgItemText(IDC_NOTES, "");
			m_pUnselectedProcedureList->Clear();
			m_pSelectedProcedureList->Clear();
			GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(TRUE);
		}

		RefreshButtons();

		//track this permission rather than checking it during each iteration of the loop
		BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

		//gray out invalid cells
		IRowSettingsPtr pRow = m_pPreferenceCardList->GetFirstRow();
		while(pRow) {

			//for persons, we don't want to let them edit the amount,
			if(VarLong(pRow->GetValue(pcicItemType)) == pcitPersonnel) {
				//gray the Amount cell
				pRow->PutCellBackColor(pcicAmount,RGB(230,230,230));		

				//clear and gray out the desc field
				pRow->PutValue(pcicProcDescription, _variant_t(""));
				pRow->PutCellBackColor(pcicProcDescription,RGB(230,230,230));
				pRow->PutCellLinkStyle(pcicProcDescription, dlLinkStyleFalse);

				// the visible cost column loads as null by default for personnel,
				// so we must copy the true cost into the visible cost column,
				// but only if they have read permissions
				if(bCanViewPersonCosts) {
					_variant_t varCost = pRow->GetValue(pcicTrueCost);
					pRow->PutValue(pcicVisibleCost, varCost);
				}
				else {
					//color the cost column gray
					pRow->PutCellBackColor(pcicVisibleCost,RGB(230,230,230));
				}
			}

			//for CPT codes, we don't want to let them edit the cost,
			//and if there is a CPT ID, then we have a code
			if(VarLong(pRow->GetValue(pcicItemType)) == pcitCPTCode) {

				//gray the Cost cell
				pRow->PutCellBackColor(pcicVisibleCost,RGB(230,230,230));
				pRow->PutCellLinkStyle(pcicProcDescription, dlLinkStyleTrue);
			}

			//for Products, both are editable
			if(VarLong(pRow->GetValue(pcicItemType)) == pcitProduct) {
				pRow->PutCellLinkStyle(pcicProcDescription, dlLinkStyleTrue);
			}

			pRow = pRow->GetNextRow();
		}

		UpdateTotals();	

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnRButtonDownPreferenceCardList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_pPreferenceCardList->CurSel = pRow;

		enum MenuItem
		{
			miDelete = 1,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED, miDelete, "&Delete");

		CPoint pt;
		GetCursorPos(&pt);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this);

		switch(nResult)
		{
			case miDelete:
			{
				if(IDNO == MessageBox("Are you sure you wish to delete the selected item?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return;
				}

				ExecuteParamSql("DELETE FROM PreferenceCardDetailsT WHERE ID = {INT}", VarLong(pRow->GetValue(pcicDetailID)));

				m_pPreferenceCardList->RemoveRow(pRow);
				UpdateTotals();
				break;
			}
			default:
				break;
		}	

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnLButtonDownPreferenceCardList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if (nCol == pcicProcDescription && VarLong(pRow->GetValue(pcicItemType)) != pcitPersonnel) {

			_RecordsetPtr rsID = CreateParamRecordset("SELECT ServiceID FROM PreferenceCardDetailsT WHERE ID = {INT} AND ServiceID Is Not Null", VarLong(pRow->GetValue(pcicDetailID))); 
			if (!rsID->eof) {
				//pop up the dialog based on the link they click on
				long nCPTID = AdoFldLong(rsID, "ServiceID");

				CProcedureDescriptionDlg dlg(nCPTID, this);
				dlg.DoModal();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-08-31 11:04) - PLID 29531 - added provider filter
void CPreferenceCardsDlg::OnSelChosenPreferenceCardProvCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			pRow = m_pProviderFilterCombo->SetSelByColumn(pfccID, (long)-1);
			if(pRow == NULL) {
				//should be impossible, do nothing to the screen
				ASSERT(FALSE);
				return;
			}
		}

		long nProviderID = VarLong(pRow->GetValue(pfccID), -1);

		//re-filter the preference card combo
		CString strWhere;
		if(nProviderID == -2) {
			//filter on cards unassigned to any provider
			strWhere.Format("PreferenceCardsQ.ID NOT IN (SELECT PreferenceCardID FROM PreferenceCardProvidersT)");
		}
		else if(nProviderID > 0) {
			//filter on cards assigned to this provider (they may also
			//be assigned to other providers as well)
			strWhere.Format("PreferenceCardsQ.ID IN (SELECT PreferenceCardID FROM PreferenceCardProvidersT WHERE ProviderID = %li)", nProviderID);
		}

		m_pPreferenceCardCombo->PutWhereClause(_bstr_t(strWhere));

		//requery, and try to maintain the current selection if possible
		long nPrefCardID = -1;
		{
			IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->GetCurSel();			
			if(pPrefCardRow) {
				nPrefCardID = VarLong(pPrefCardRow->GetValue(pcccID));
			}
		}

		m_pPreferenceCardCombo->Requery();

		IRowSettingsPtr pPrefCardRow = NULL;
		if(nPrefCardID != -1) {
			pPrefCardRow = m_pPreferenceCardCombo->SetSelByColumn(pcccID, nPrefCardID);
		}

		if(pPrefCardRow == NULL) {
			m_pPreferenceCardCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			if(m_pPreferenceCardCombo->GetRowCount() == 0){
				m_pPreferenceCardList->Clear();
				SetDlgItemText(IDC_NOTES, "");
				m_pUnselectedProcedureList->Clear();
				m_pSelectedProcedureList->Clear();
				GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(FALSE);
			}
			else {
				pPrefCardRow = m_pPreferenceCardCombo->GetFirstRow();
				m_pPreferenceCardCombo->PutCurSel(pPrefCardRow);
			}
		}
		
		if(pPrefCardRow) {
			OnSelChosenPreferenceCardsCombo(pPrefCardRow);
		}

		RefreshButtons();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-08-31 16:24) - PLID 17732 - added ability to link procedures
void CPreferenceCardsDlg::OnBtnSelectOneProcedure()
{
	try {

		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;

		if(pPrefCardRow == NULL) {
			return;
		}

		long nPreferenceCardID = VarLong(pPrefCardRow->GetValue(pcccID));

		CString strSqlBatch;
		IRowSettingsPtr pRow = m_pUnselectedProcedureList->GetFirstSelRow();
		while(pRow) {

			//add each procedure - do so safely so we cannot cause a duplicate
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PreferenceCardProceduresT (PreferenceCardID, ProcedureID) "
					"SELECT %li, ID FROM ProcedureT WHERE ID = %li AND ID NOT IN (SELECT ProcedureID FROM PreferenceCardProceduresT "
					"WHERE PreferenceCardID = %li)",
				nPreferenceCardID, VarLong(pRow->GetValue(plccID)),
				VarLong(pRow->GetValue(plccID)), nPreferenceCardID);

			pRow = pRow->GetNextSelRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		m_pSelectedProcedureList->TakeCurrentRowAddSorted(m_pUnselectedProcedureList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnBtnUnselectOneProcedure()
{
	try {

		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;

		if(pPrefCardRow == NULL) {
			return;
		}

		long nPreferenceCardID = VarLong(pPrefCardRow->GetValue(pcccID));

		CString strSqlBatch;
		IRowSettingsPtr pRow = m_pSelectedProcedureList->GetFirstSelRow();
		while(pRow) {

			//remove each procedure
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PreferenceCardProceduresT "
				"WHERE PreferenceCardID = %li AND ProcedureID = %li", nPreferenceCardID, VarLong(pRow->GetValue(plccID)));

			pRow = pRow->GetNextSelRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		m_pUnselectedProcedureList->TakeCurrentRowAddSorted(m_pSelectedProcedureList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnBtnUnselectAllProcedures()
{
	try {

		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;

		if(pPrefCardRow == NULL) {
			return;
		}

		long nPreferenceCardID = VarLong(pPrefCardRow->GetValue(pcccID));

		//clear all linked procedures
		ExecuteParamSql("DELETE FROM PreferenceCardProceduresT WHERE PreferenceCardID = {INT}", nPreferenceCardID);

		m_pUnselectedProcedureList->TakeAllRows(m_pSelectedProcedureList);

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnDblClickCellUnselectedPrefCardProceduresList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;

		if(pPrefCardRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_pUnselectedProcedureList->PutCurSel(pRow);
			OnBtnSelectOneProcedure();
		}

	}NxCatchAll(__FUNCTION__);
}

void CPreferenceCardsDlg::OnDblClickCellSelectedPrefCardProceduresList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;

		if(pPrefCardRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_pSelectedProcedureList->PutCurSel(pRow);
			OnBtnUnselectOneProcedure();
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-05-04) PLID 37048 - Select the previous preference card in the list
void CPreferenceCardsDlg::OnBtnPrefCardPrev()
{
	try {
		//If we don't have a current pref card, we have to quit
		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;
		if(pPrefCardRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow = pPrefCardRow->GetPreviousRow();
		//If we don't have a previous row, we have to quit
		if(pRow) {
			m_pPreferenceCardCombo->PutCurSel(pRow);
			HandleSelectedPrefCard();
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-05-04) PLID 37048 - Select the next preference card in the list
void CPreferenceCardsDlg::OnBtnPrefCardNext()
{
	try {
		//If we don't have a current pref card, we have to quit
		IRowSettingsPtr pPrefCardRow = m_pPreferenceCardCombo->CurSel;
		if(pPrefCardRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow = pPrefCardRow->GetNextRow();
		//If we don't have a next row, we have to quit
		if(pRow) {
			m_pPreferenceCardCombo->PutCurSel(pRow);
			HandleSelectedPrefCard();
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-05-04) PLID 37048 - Moved code for handling new preference card selection to its own function
void CPreferenceCardsDlg::HandleSelectedPrefCard()
{
	IRowSettingsPtr pRow = m_pPreferenceCardCombo->GetCurSel();
	if(pRow == NULL) {
		if(m_pPreferenceCardCombo->GetRowCount() == 0){
			m_pPreferenceCardList->Clear();
			SetDlgItemText(IDC_NOTES, "");
			m_pUnselectedProcedureList->Clear();
			m_pSelectedProcedureList->Clear();
			GetDlgItem(IDC_PREFERENCE_CARDS_COMBO)->EnableWindow(FALSE);
			RefreshButtons();
			return;
		}
		else {
			pRow = m_pPreferenceCardCombo->GetFirstRow();
			m_pPreferenceCardCombo->PutCurSel(pRow);
		}		
	}
	RefreshButtons();


	long nPreferenceCardID = VarLong(pRow->GetValue(pcccID));

	CString strWhere;
	strWhere.Format("PreferenceCardID = %li", nPreferenceCardID);
	m_pPreferenceCardList->PutWhereClause((LPCTSTR)strWhere);
	m_pPreferenceCardList->Requery();
	
	_RecordsetPtr rs = CreateParamRecordset("SELECT Description FROM PreferenceCardsT WHERE ID = {INT}", nPreferenceCardID);
	if(!rs->eof){
		SetDlgItemText(IDC_NOTES, VarString(rs->Fields->GetItem("Description")->Value, ""));
	}
	rs->Close();

	// (j.jones 2009-08-31 16:11) - PLID 17732 - added procedure lists
	//the unselected list filters out inactive procedures in its from clause
	strWhere.Format("ID NOT IN (SELECT ProcedureID FROM PreferenceCardProceduresT WHERE PreferenceCardID = %li)", nPreferenceCardID);
	m_pUnselectedProcedureList->PutWhereClause((LPCTSTR)strWhere);
	m_pUnselectedProcedureList->Requery();

	strWhere.Format("ID IN (SELECT ProcedureID FROM PreferenceCardProceduresT WHERE PreferenceCardID = %li)", nPreferenceCardID);
	//the selected list allows inactive procedures in its from clause
	m_pSelectedProcedureList->PutWhereClause((LPCTSTR)strWhere);
	m_pSelectedProcedureList->Requery();
}