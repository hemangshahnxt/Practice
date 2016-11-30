// AdvPreferenceCardEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvPreferenceCardEditDlg.h"

// CAdvPreferenceCardEditDlg dialog

// (j.jones 2009-08-27 09:51) - PLID 35283 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CAdvPreferenceCardEditDlg, CNxDialog)

CAdvPreferenceCardEditDlg::CAdvPreferenceCardEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvPreferenceCardEditDlg::IDD, pParent)
{

}

void CAdvPreferenceCardEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_PREF_CARD, m_btnSelectOnePrefCard);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_PREF_CARD, m_btnUnselectOnePrefCard);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_PREF_CARDS, m_btnUnselectAllPrefCards);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_PRODUCT, m_btnSelectOneProduct);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_PRODUCT, m_btnUnselectOneProduct);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_PRODUCTS, m_btnUnselectAllProducts);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_PERSON, m_btnSelectOnePerson);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_PERSON, m_btnUnselectOnePerson);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_PERSONS, m_btnUnselectAllPersons);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_SERVICE, m_btnSelectOneService);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_SERVICE, m_btnUnselectOneService);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_SERVICES, m_btnUnselectAllServices);
	DDX_Control(pDX, IDC_APPLY, m_btnApply);
	DDX_Control(pDX, IDC_BTN_CLOSE_PC_EDIT, m_btnClose);
}


BEGIN_MESSAGE_MAP(CAdvPreferenceCardEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_PREF_CARD, OnBnClickedBtnSelectOnePrefCard)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_PREF_CARD, OnBnClickedBtnUnselectOnePrefCard)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_PREF_CARDS, OnBnClickedBtnUnselectAllPrefCards)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_PRODUCT, OnBnClickedBtnSelectOneProduct)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_PRODUCT, OnBnClickedBtnUnselectOneProduct)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_PRODUCTS, OnBnClickedBtnUnselectAllProducts)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_PERSON, OnBnClickedBtnSelectOnePerson)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_PERSON, OnBnClickedBtnUnselectOnePerson)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_PERSONS, OnBnClickedBtnUnselectAllPersons)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_SERVICE, OnBnClickedBtnSelectOneService)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_SERVICE, OnBnClickedBtnUnselectOneService)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_SERVICES, OnBnClickedBtnUnselectAllServices)
	ON_BN_CLICKED(IDC_APPLY, OnBnClickedApply)
	ON_BN_CLICKED(IDC_BTN_CLOSE_PC_EDIT, OnBnClickedBtnClosePcEdit)
END_MESSAGE_MAP()


// CAdvPreferenceCardEditDlg message handlers
BOOL CAdvPreferenceCardEditDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnSelectOnePrefCard.AutoSet(NXB_RIGHT);
		m_btnUnselectOnePrefCard.AutoSet(NXB_LEFT);
		m_btnUnselectAllPrefCards.AutoSet(NXB_LLEFT);
		m_btnSelectOneProduct.AutoSet(NXB_RIGHT);
		m_btnUnselectOneProduct.AutoSet(NXB_LEFT);
		m_btnUnselectAllProducts.AutoSet(NXB_LLEFT);
		m_btnSelectOnePerson.AutoSet(NXB_RIGHT);
		m_btnUnselectOnePerson.AutoSet(NXB_LEFT);
		m_btnUnselectAllPersons.AutoSet(NXB_LLEFT);
		m_btnSelectOneService.AutoSet(NXB_RIGHT);
		m_btnUnselectOneService.AutoSet(NXB_LEFT);
		m_btnUnselectAllServices.AutoSet(NXB_LLEFT);
		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_UnselectedPreferenceCardList = BindNxDataList2Ctrl(IDC_UNSELECTED_PREFERENCE_CARD_LIST);
		m_SelectedPreferenceCardList = BindNxDataList2Ctrl(IDC_SELECTED_PREFERENCE_CARD_LIST, false);
		m_UnselectedProductList = BindNxDataList2Ctrl(IDC_UNSELECTED_INVENTORY_LIST);
		m_SelectedProductList = BindNxDataList2Ctrl(IDC_SELECTED_INVENTORY_LIST, false);
		m_UnselectedPersonnelList = BindNxDataList2Ctrl(IDC_UNSELECTED_PERSON_LIST);
		m_SelectedPersonnelList = BindNxDataList2Ctrl(IDC_SELECTED_PERSON_LIST, false);
		m_UnselectedServiceList = BindNxDataList2Ctrl(IDC_UNSELECTED_SERVICE_LIST);
		m_SelectedServiceList = BindNxDataList2Ctrl(IDC_SELECTED_SERVICE_LIST, false);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnSelectOnePrefCard()
{
	try {

		m_SelectedPreferenceCardList->TakeCurrentRowAddSorted(m_UnselectedPreferenceCardList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectOnePrefCard()
{
	try {

		m_UnselectedPreferenceCardList->TakeCurrentRowAddSorted(m_SelectedPreferenceCardList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectAllPrefCards()
{
	try {

		m_UnselectedPreferenceCardList->TakeAllRows(m_SelectedPreferenceCardList);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnSelectOneProduct()
{
	try {

		m_SelectedProductList->TakeCurrentRowAddSorted(m_UnselectedProductList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectOneProduct()
{
	try {

		m_UnselectedProductList->TakeCurrentRowAddSorted(m_SelectedProductList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectAllProducts()
{
	try {

		m_UnselectedProductList->TakeAllRows(m_SelectedProductList);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnSelectOnePerson()
{
	try {

		m_SelectedPersonnelList->TakeCurrentRowAddSorted(m_UnselectedPersonnelList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectOnePerson()
{
	try {

		m_UnselectedPersonnelList->TakeCurrentRowAddSorted(m_SelectedPersonnelList, NULL);		

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectAllPersons()
{
	try {

		m_UnselectedPersonnelList->TakeAllRows(m_SelectedPersonnelList);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnSelectOneService()
{
	try {

		m_SelectedServiceList->TakeCurrentRowAddSorted(m_UnselectedServiceList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectOneService()
{
	try {

		m_UnselectedServiceList->TakeCurrentRowAddSorted(m_SelectedServiceList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnUnselectAllServices()
{
	try {

		m_UnselectedServiceList->TakeAllRows(m_SelectedServiceList);

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedApply()
{
	try {

		//first make sure at least one item to add is selected
		if(m_SelectedProductList->GetRowCount() == 0 && m_SelectedPersonnelList->GetRowCount() == 0
			&& m_SelectedServiceList->GetRowCount() == 0) {
			AfxMessageBox("You must have at least one Inventory Item, Personnel, or Service Code selected.");
			return;
		}

		//now make sure at least one preference card is selected
		if(m_SelectedPreferenceCardList->GetRowCount() == 0) {
			AfxMessageBox("You must select at least one Preference Card to add the selected items.");
			return;
		}

		//warn them about what they are doing, give them a chance to cancel
		if(IDNO == MessageBox("You are about to add all of the selected Inventory Items, Personnel, and Service Codes to all of the selected Preference Cards.\n\n"
			"Are you sure you wish to do this?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		CWaitCursor pWait;

		//okay, now we can proceed

		CString strSqlBatch;

		//loop through each preference card
		IRowSettingsPtr pPrefCardRow = m_SelectedPreferenceCardList->GetFirstRow();
		while(pPrefCardRow) {

			long nPreferenceCardID = VarLong(pPrefCardRow->GetValue(0));
			
			//now loop through each Product and add it
			IRowSettingsPtr pProductRow = m_SelectedProductList->GetFirstRow();
			while(pProductRow) {

				long nProductID = VarLong(pProductRow->GetValue(0));

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PreferenceCardDetailsT "
					"(PreferenceCardID, ServiceID, Amount, Cost, Quantity, Billable) "					
					"SELECT %li, ServiceT.ID, Price, ProductT.LastCostPerUU, 1.0, 1 "
					"	FROM ServiceT "
					"	INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"	WHERE ServiceT.ID = %li", nPreferenceCardID, nProductID);

				pProductRow = pProductRow->GetNextRow();
			}

			//now loop through each Person and add it
			IRowSettingsPtr pPersonRow = m_SelectedPersonnelList->GetFirstRow();
			while(pPersonRow) {

				long nPersonID = VarLong(pPersonRow->GetValue(0));

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PreferenceCardDetailsT "
					"(PreferenceCardID, PersonID, Amount, Cost, Quantity, Billable) "					
					"SELECT %li, PersonT.ID, Convert(money, 0), "
					"CASE WHEN ProvidersT.PersonID Is Not Null THEN ProvidersT.DefaultCost "
					"	WHEN ContactsT.Nurse = 1 OR ContactsT.Anesthesiologist = 1 THEN ContactsT.DefaultCost "
					"	WHEN UsersT.PersonID Is Not Null THEN UsersT.DefaultCost ELSE 0 END, "
					"1.0, 0 "
					"FROM PersonT "
					"	LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
					"	LEFT JOIN ContactsT ON PersonT.ID = ContactsT.PersonID "
					"	LEFT JOIN UsersT ON PersonT.ID = UsersT.PersonID "
					"	WHERE PersonT.ID = %li", nPreferenceCardID, nPersonID);

				pPersonRow = pPersonRow->GetNextRow();
			}

			//now loop through each service code and add it
			IRowSettingsPtr pServiceRow = m_SelectedServiceList->GetFirstRow();
			while(pServiceRow) {

				long nServiceID = VarLong(pServiceRow->GetValue(0));

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PreferenceCardDetailsT "
					"(PreferenceCardID, ServiceID, Amount, Cost, Quantity, Billable) "					
					"SELECT %li, ServiceT.ID, Price, Convert(money, 0), 1.0, 1 "
					"	FROM ServiceT "
					"	INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"	WHERE ServiceT.ID = %li", nPreferenceCardID, nServiceID);

				pServiceRow = pServiceRow->GetNextRow();
			}

			pPrefCardRow = pPrefCardRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		AfxMessageBox("The selected items have been added to each of the selected Preference Cards.");

		//now that we are done, clear the selected lists
		OnBnClickedBtnUnselectAllPrefCards();
		OnBnClickedBtnUnselectAllProducts();
		OnBnClickedBtnUnselectAllPersons();
		OnBnClickedBtnUnselectAllServices();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnBnClickedBtnClosePcEdit()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CAdvPreferenceCardEditDlg, CNxDialog)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_UNSELECTED_PREFERENCE_CARD_LIST, 3, OnDblClickCellUnselectedPreferenceCardList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_SELECTED_PREFERENCE_CARD_LIST, 3, OnDblClickCellSelectedPreferenceCardList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_UNSELECTED_INVENTORY_LIST, 3, OnDblClickCellUnselectedInventoryList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_SELECTED_INVENTORY_LIST, 3, OnDblClickCellSelectedInventoryList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_UNSELECTED_PERSON_LIST, 3, OnDblClickCellUnselectedPersonList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_SELECTED_PERSON_LIST, 3, OnDblClickCellSelectedPersonList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_UNSELECTED_SERVICE_LIST, 3, OnDblClickCellUnselectedServiceList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvPreferenceCardEditDlg, IDC_SELECTED_SERVICE_LIST, 3, OnDblClickCellSelectedServiceList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CAdvPreferenceCardEditDlg::OnDblClickCellUnselectedPreferenceCardList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnSelectOnePrefCard();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellSelectedPreferenceCardList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnUnselectOnePrefCard();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellUnselectedInventoryList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnSelectOneProduct();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellSelectedInventoryList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnUnselectOneProduct();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellUnselectedPersonList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnSelectOnePerson();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellSelectedPersonList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnUnselectOnePerson();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellUnselectedServiceList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnSelectOneService();

	}NxCatchAll(__FUNCTION__);
}

void CAdvPreferenceCardEditDlg::OnDblClickCellSelectedServiceList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBnClickedBtnUnselectOneService();

	}NxCatchAll(__FUNCTION__);
}
