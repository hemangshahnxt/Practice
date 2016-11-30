// MainSurgeries.cpp : implementation file
//

#include "stdafx.h"
#include "MainSurgeries.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "NxStandard.h"
#include "GetNewIDName.h"
#include "QuoteAdminDlg.h"
#include "AuditTrail.h"
#include "SurgeryProviderLinkDlg.h"
#include "InternationalUtils.h"
#include "AdvSurgeryItemsDlg.h"
#include "GlobalFinancialUtils.h"
#include "ProcedureDescriptionDlg.h"
#include "DiscountCategorySelectDlg.h"
#include "ChargeDiscountDlg.h"
#include "BillingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



using namespace NXDATALISTLib;
using namespace NXTIMELib;
using namespace ADODB;

// (j.jones 2008-07-01 10:51) - PLID 30578
enum ESurgeryItemTypes {

	sitCPTCode = 1,
	sitProduct = 2,
	//sitPersonnel = 3,	// (j.jones 2009-08-21 09:24) - PLID 35271
};

// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
// which is now its own dialog and data structure
enum ESurgeryItemColumns {
	sicDetailID = 0,
	sicItem,
	sicItemType,			// (j.jones 2008-07-01 10:51) - PLID 30578
	sicProviderID,
	sicCPTCodeProductID,
	sicName,	
	sicAmount,
	//sicVisibleCost,
	//sicTrueCost,			// (j.jones 2008-07-01 10:40) - PLID 18744
	sicQuantity,
	//sicPercentOff,       // (j.gruber 2009-03-19 10:56) - PLID 33361 - implement new discount structure
	//sicDiscount,
	//sicHasDiscountCategory,
	//sicDiscountCategoryID,
	//sicCustomDiscountDesc, 
	sicTotalDiscountAmt,
	sicPayToPractice,
	//sicBillable,
	sicLineOrder,
	sicLineTotalAmt,
	sicProcDescription,
};

// (j.jones 2008-07-01 09:40) - PLID 18744 - added enum for personnel combo columns
enum PersonnelComboColumn {

	pccID = 0,
	pccName,
	pccCost,
	pccType,
};

// (j.jones 2008-09-08 17:23) - PLID 15345 - added enum for product combo columns
// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
// which is now its own dialog and data structure
enum ProductComboColumn {

	prodID = 0,
	prodCategory,
	prodSupplier,
	prodName,
	prodBarcode,
	prodPrice,
	//prodLastCost,
};

/////////////////////////////////////////////////////////////////////////////
// CMainSurgeries dialog


CMainSurgeries::CMainSurgeries(CWnd* pParent)
	: CNxDialog(CMainSurgeries::IDD, pParent)
	,
	m_CPTChecker(NetUtils::CPTCodeT),
	m_InventoryChecker(NetUtils::Products),
	m_SrgyChecker(NetUtils::SurgeriesT, false)
{
	//{{AFX_DATA_INIT(CMainSurgeries)
	//}}AFX_DATA_INIT

	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Billing_Setup/setup_surgery_templates.htm";

	m_bManualChange = false;
	
	// (j.jones 2010-01-05 15:18) - PLID 15997 - added more "changed" booleans
	m_bNotesChanged = FALSE;
	m_bPackageCostChanged = FALSE;
	m_bPackageCountChanged = FALSE;

	// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
	m_bAnesthMinutesChanged = FALSE;
	m_bAnesthStartTimeChanged = FALSE;
	m_bAnesthEndTimeChanged = FALSE;
	m_bFacilityMinutesChanged = FALSE;
	m_bFacilityStartTimeChanged = FALSE;
	m_bFacilityEndTimeChanged = FALSE;
}

void CMainSurgeries::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMainSurgeries)
	DDX_Control(pDX, IDC_SEARCH_VIEW, m_btnSearchView);
	DDX_Control(pDX, IDC_SURGERY_SAVE_AS, m_btnSaveAs);
	DDX_Control(pDX, IDC_BTN_ADV_SURGERY_EDIT, m_btnAdvSurgeryEdit);
	DDX_Control(pDX, IDC_QUOTE_ADMIN, m_btnQuoteAdmin);
	DDX_Control(pDX, IDC_ADD, m_addButton);
	DDX_Control(pDX, IDC_RENAME, m_renameButton);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_NOTES_LABEL, m_nxstaticNotesLabel);
	DDX_Control(pDX, IDC_PRACTICE_TOTAL_LABEL, m_nxstaticPracticeTotalLabel);
	DDX_Control(pDX, IDC_PRACTICE_TOTAL, m_nxstaticPracticeTotal);
	DDX_Control(pDX, IDC_OUTSIDE_TOTAL_LABEL, m_nxstaticOutsideTotalLabel);
	DDX_Control(pDX, IDC_OUTSIDE_TOTAL, m_nxstaticOutsideTotal);
	DDX_Control(pDX, IDC_TOTAL_LABEL, m_nxstaticTotalLabel);
	DDX_Control(pDX, IDC_TOTAL, m_nxstaticTotal);
	DDX_Control(pDX, IDC_SURGERY_PACKAGE_CHECK, m_checkPackage);
	DDX_Control(pDX, IDC_RADIO_SURGERY_REPEAT_PACKAGE, m_radioRepeatPackage);
	DDX_Control(pDX, IDC_RADIO_SURGERY_MULTIUSE_PACKAGE, m_radioMultiUsePackage);
	DDX_Control(pDX, IDC_SURGERY_PACKAGE_TOTAL_COUNT, m_nxeditPackageTotalCount);
	DDX_Control(pDX, IDC_SURGERY_PACKAGE_TOTAL_COST, m_nxeditPackageTotalCost);
	DDX_Control(pDX, IDC_SURGERY_PACKAGE_TOTAL_COUNT_LABEL, m_nxstaticPackageTotalCountLabel);
	DDX_Control(pDX, IDC_SURGERY_PACKAGE_TOTAL_COST_LABEL, m_nxstaticPackageTotalCostLabel);
	DDX_Control(pDX, IDC_RADIO_SHOW_ALL_SURGERIES, m_radioShowAll);
	DDX_Control(pDX, IDC_RADIO_ONLY_SURGERIES, m_radioShowSurgeries);
	DDX_Control(pDX, IDC_RADIO_ONLY_PACKAGES, m_radioShowPackages);
	DDX_Control(pDX, IDC_EDIT_DEF_ANESTH_MINUTES, m_nxeditDefAnesthMinutes);
	DDX_Control(pDX, IDC_EDIT_DEF_FACILITY_MINUTES, m_nxeditDefFacilityMinutes);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMainSurgeries, CNxDialog)
	//{{AFX_MSG_MAP(CMainSurgeries)
	ON_EN_KILLFOCUS(IDC_NOTES, OnKillfocusNotes)
	ON_COMMAND(ID_DELETE_ITEM, OnDeleteItem)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_QUOTE_ADMIN, OnQuoteAdmin)
	ON_BN_CLICKED(IDC_BTN_ADV_SURGERY_EDIT, OnBtnAdvSurgeryEdit)
	ON_BN_CLICKED(IDC_SEARCH_VIEW, OnSearchVew)
	ON_BN_CLICKED(IDC_SURGERY_SAVE_AS, OnSurgerySaveAs)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_SURGERY_PACKAGE_CHECK, OnSurgeryPackageCheck)
	ON_BN_CLICKED(IDC_RADIO_SURGERY_REPEAT_PACKAGE, OnRadioSurgeryRepeatPackage)
	ON_BN_CLICKED(IDC_RADIO_SURGERY_MULTIUSE_PACKAGE, OnRadioSurgeryMultiusePackage)
	ON_EN_KILLFOCUS(IDC_SURGERY_PACKAGE_TOTAL_COST, OnKillfocusSurgeryPackageTotalCost)
	ON_EN_KILLFOCUS(IDC_SURGERY_PACKAGE_TOTAL_COUNT, OnKillfocusSurgeryPackageTotalCount)
	ON_BN_CLICKED(IDC_RADIO_SHOW_ALL_SURGERIES, OnRadioSurgeryFilterChanged)
	ON_BN_CLICKED(IDC_RADIO_ONLY_SURGERIES, OnRadioSurgeryFilterChanged)
	ON_BN_CLICKED(IDC_RADIO_ONLY_PACKAGES, OnRadioSurgeryFilterChanged)
	ON_EN_KILLFOCUS(IDC_EDIT_DEF_ANESTH_MINUTES, OnKillfocusEditDefAnesthMinutes)
	ON_EN_KILLFOCUS(IDC_EDIT_DEF_FACILITY_MINUTES, OnKillfocusEditDefFacilityMinutes)
	ON_EN_CHANGE(IDC_EDIT_DEF_ANESTH_MINUTES, OnChangeEditDefAnesthMinutes)
	ON_EN_CHANGE(IDC_EDIT_DEF_FACILITY_MINUTES, OnChangeEditDefFacilityMinutes)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CMainSurgeries, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMainSurgeries)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_NAMES, 16 /* SelChosen */, OnSelChosenSurgeryNames, VTS_I4)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_CPTCODES, 16 /* SelChosen */, OnSelChosenSurgeryCptcodes, VTS_I4)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_PRODUCTS, 16 /* SelChosen */, OnSelChosenSurgeryProducts, VTS_I4)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 6 /* RButtonDown */, OnRButtonDownSurgeryItems, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 10 /* EditingFinished */, OnEditingFinishedSurgeryItems, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 18 /* RequeryFinished */, OnRequeryFinishedSurgeryItems, VTS_I2)	
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 9 /* EditingFinishing */, OnEditingFinishingSurgeryItems, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 8 /* EditingStarting */, OnEditingStartingSurgeryItems, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_NAMES, 18 /* RequeryFinished */, OnRequeryFinishedSurgeryNames, VTS_I2)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 14 /* DragEnd */, OnDragEndSurgeryItems, VTS_I4 VTS_I2 VTS_I4 VTS_I2 VTS_I4)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 17 /* ColumnClicking */, OnColumnClickingSurgeryItems, VTS_I2 VTS_PBOOL)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 5 /* LButtonUp */, OnLButtonUpSurgeryItems, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMainSurgeries, IDC_SURGERY_ITEMS, 19 /* LeftClick */, OnLeftClickSurgeryItems, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CMainSurgeries, IDC_ANESTH_DEF_START_TIME, 1, OnKillFocusAnesthDefStartTime, VTS_NONE)
	ON_EVENT(CMainSurgeries, IDC_ANESTH_DEF_END_TIME, 1, OnKillFocusAnesthDefEndTime, VTS_NONE)
	ON_EVENT(CMainSurgeries, IDC_FACILITY_DEF_START_TIME, 1, OnKillFocusFacilityDefStartTime, VTS_NONE)
	ON_EVENT(CMainSurgeries, IDC_FACILITY_DEF_END_TIME, 1, OnKillFocusFacilityDefEndTime, VTS_NONE)
	ON_EVENT(CMainSurgeries, IDC_ANESTH_DEF_START_TIME, 2, OnChangedAnesthDefStartTime, VTS_NONE)
	ON_EVENT(CMainSurgeries, IDC_ANESTH_DEF_END_TIME, 2, OnChangedAnesthDefEndTime, VTS_NONE)	
	ON_EVENT(CMainSurgeries, IDC_FACILITY_DEF_START_TIME, 2, OnChangedFacilityDefStartTime, VTS_NONE)
	ON_EVENT(CMainSurgeries, IDC_FACILITY_DEF_END_TIME, 2, OnChangedFacilityDefEndTime, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainSurgeries message handlers

BOOL CMainSurgeries::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_addButton.AutoSet(NXB_NEW);
	m_renameButton.AutoSet(NXB_MODIFY);
	m_deleteButton.AutoSet(NXB_DELETE);
	// (z.manning, 04/25/2008) - PLID 29566 - Added style for Save Copy button
	m_btnSaveAs.AutoSet(NXB_NEW);

	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	//m_btnLinkToProviders.AutoSet(NXB_MODIFY);

	try{

		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure
		//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
		m_strManualLocation = "NexTech_Practice_Manual.chm";
		m_strManualBookmark = "System_Setup/Billing_Setup/setup_surgery_templates.htm";

		// (j.jones 2010-01-06 09:00) - PLID 36763 - added package/surgery filters
		m_radioShowAll.SetCheck(TRUE);

		// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
		m_nxtDefAnesthStart = BindNxTimeCtrl(this, IDC_ANESTH_DEF_START_TIME);
		m_nxtDefAnesthEnd = BindNxTimeCtrl(this, IDC_ANESTH_DEF_END_TIME);
		m_nxtDefFacilityStart = BindNxTimeCtrl(this, IDC_FACILITY_DEF_START_TIME);
		m_nxtDefFacilityEnd = BindNxTimeCtrl(this, IDC_FACILITY_DEF_END_TIME);
		
		m_pSurgeryNames = BindNxDataListCtrl(IDC_SURGERY_NAMES);

		m_pCPTCodes = BindNxDataListCtrl(IDC_SURGERY_CPTCODES);
		IColumnSettingsPtr(m_pCPTCodes->GetColumn(0))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_pCPTCodes->GetColumn(1))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_pCPTCodes->GetColumn(2))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_pCPTCodes->GetColumn(3))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));
		IColumnSettingsPtr(m_pCPTCodes->GetColumn(4))->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));

		m_pProducts = BindNxDataListCtrl(IDC_SURGERY_PRODUCTS);
		IColumnSettingsPtr(m_pProducts->GetColumn(prodID))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_pProducts->GetColumn(prodCategory))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_pProducts->GetColumn(prodSupplier))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_pProducts->GetColumn(prodName))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_pProducts->GetColumn(prodBarcode))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		IColumnSettingsPtr(m_pProducts->GetColumn(prodPrice))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure
		/*
		IColumnSettingsPtr(m_pProducts->GetColumn(prodLastCost))->PutBackColor(GetNxColor(GNC_PRODUCT,-1));

		m_pPersons = BindNxDataListCtrl(IDC_SURGERY_PERSONS);
		IColumnSettingsPtr(m_pPersons->GetColumn(pccID))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		IColumnSettingsPtr(m_pPersons->GetColumn(pccName))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		IColumnSettingsPtr(m_pPersons->GetColumn(pccCost))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		IColumnSettingsPtr(m_pPersons->GetColumn(pccType))->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));

		// (j.jones 2008-07-01 09:34) - PLID 18744 - hide the cost column if they do not have
		// permission to view the personnel cost
		if(!(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
			IColumnSettingsPtr(m_pPersons->GetColumn(pccCost))->PutStoredWidth(0);
			IColumnSettingsPtr(m_pPersons->GetColumn(pccCost))->ColumnStyle = csFixedWidth|csVisible;
		}
		*/

		m_pSurgeryItems = BindNxDataListCtrl(IDC_SURGERY_ITEMS,false);

		// (j.gruber 2009-03-19 13:22) - PLID 33361 - moved the from clause from the resources to here for easier reading/updating
		m_pSurgeryItems->FromClause = "(SELECT SurgeryDetailsT.ID as SurgeryDetailID, SurgeryDetailsT.SurgeryID as SurgeryID,  "
			" CASE WHEN CPTCodeT.ID Is Null THEN 'Inventory' ELSE 'Service Code' END as Item, "
			" Convert(int, CASE WHEN CPTCodeT.ID Is Not Null THEN 1 ELSE 2 END) as ItemType, "
			" SurgeryDetailsT.ProviderID as ProviderID, (CASE WHEN ProductT.ID Is Not Null THEN Coalesce(ProductT.InsCode, '') ELSE Coalesce(CPTCodeT.Code,'') END) as ServiceCode, "
			" ServiceT.Name, "
			" SurgeryDetailsT.Amount, "
			" SurgeryDetailsT.Quantity,  "
			" Convert(money, ((SurgeryDetailsT.Amount * SurgeryDetailsT.Quantity) *  "
			" CASE WHEN TotalPercentOff IS NULL THEN 0 ELSE CONVERT(float, TotalPercentOff)/100 END ) + "
			" CASE WHEN TotalDiscount IS NULL THEN 0 ELSE TotalDiscount END) As TotalDiscountAmt,  "
			" PayToPractice AS PayToPrac, "
			" SurgeryDetailsT.LineOrder as LineOrder, "
			" Round(Convert(money,Quantity * Amount),2) as TotalCost, "
			" '...' AS ProcDesc "
			" FROM SurgeryDetailsT  "
			" INNER JOIN ServiceT ON SurgeryDetailsT.ServiceID = ServiceT.ID "
			" LEFT JOIN (SELECT SurgeryDetailID, SUM(Percentoff) as TotalPercentOff, SUM(Discount) as TotalDiscount FROM SurgeryDetailDiscountsT GROUP BY SurgeryDetailID) TotalDiscountsQ ON SurgeryDetailsT.ID = TotalDiscountsQ.SurgeryDetailID  "
			" LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID  "
			" LEFT JOIN ProductT ON SurgeryDetailsT.ServiceID = ProductT.ID) Q";

		m_pSurgeryItems->GridVisible = TRUE;

		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure

		//PLID 14099 - take out the inventory items if they don't have surgery
		if(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			GetDlgItem(IDC_SURGERY_PRODUCTS)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_SURGERY_PRODUCTS)->EnableWindow(FALSE);
		}

		m_bManualChange = false;
		m_pSurgeryItems->PutAllowSort(TRUE);
		CheckDlgButton(IDC_SEARCH_VIEW, TRUE);

		m_pSurgeryNames->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		if(m_pSurgeryNames->GetRowCount() == 0){
			GetDlgItem(IDC_SURGERY_NAMES)->EnableWindow(FALSE);
		}

		((CNxEdit*)GetDlgItem(IDC_NOTES))->SetLimitText(1000);

		RefreshButtons();

	}NxCatchAll("Error in OnInitDialog()");
	return TRUE;
}

BOOL CMainSurgeries::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam))
	{	
		case EN_CHANGE:
		{
			switch (LOWORD(wParam))
			{	case IDC_NOTES:
					m_bNotesChanged = TRUE;			
					break;
				// (j.jones 2010-01-05 15:18) - PLID 15997 - added more "changed" booleans
				case IDC_SURGERY_PACKAGE_TOTAL_COST:
					m_bPackageCostChanged = TRUE;			
					break;
				case IDC_SURGERY_PACKAGE_TOTAL_COUNT:
					m_bPackageCountChanged = TRUE;			
					break;
			}
		}
		break;
	}	

	return CNxDialog::OnCommand(wParam, lParam);
}

// (j.jones 2010-01-05 15:52) - PLID 15997 - if bUpdatePackageCost is true, if the practice
// total mismatches the package total, the package total will be replaced with the package total
void CMainSurgeries::UpdateTotals(BOOL bUpdatePackageCost)
{	
	try {

		if(m_pSurgeryNames->CurSel==-1)
			return;

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));
		
		COleCurrency cyToPractice(0,0), cyOutside(0,0);

		//PayToPractice total
		// (j.gruber 2009-03-19 11:04) - PLID 33361 - updated discoutn structure
		// (j.jones 2009-08-21 12:43) - PLID 35271 - removed Billable
		_RecordsetPtr rs = CreateParamRecordset("SELECT Sum(Round(Convert(money, (Amount * Quantity *  "
			"	(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END))  "
			"	- (CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END)),2)) AS Total FROM SurgeryDetailsT  "
			"	LEFT JOIN (SELECT SurgeryDetailID, SUM(Percentoff) as TotalPercentOff, SUM(Discount) as TotalDiscount FROM SurgeryDetailDiscountsT GROUP BY SurgeryDetailID) TotalDiscountsQ ON SurgeryDetailsT.ID = TotalDiscountsQ.SurgeryDetailID "
			"	WHERE PayToPractice = 1 AND SurgeryID = {INT}",nSurgeryID);
		if(!rs->eof) {
			cyToPractice = AdoFldCurrency(rs, "Total",COleCurrency(0,0));
		}
		rs->Close();

		// (j.jones 2009-08-21 12:43) - PLID 35271 - removed Cost
		/*
		//Cost total
		rs = CreateRecordset("SELECT Sum(Round(Convert(money,Cost * Quantity),2)) AS Total FROM SurgeryDetailsT "
			"WHERE SurgeryID = %li",nSurgeryID);
		if(!rs->eof) {
			cyCost = AdoFldCurrency(rs, "Total",COleCurrency(0,0));
		}
		rs->Close();
		*/

		//Outside Fee Total
		// (j.gruber 2009-03-19 11:04) - PLID 33361 - updated discount structure
		// (j.jones 2009-08-21 12:43) - PLID 35271 - removed Billable
		// (j.jones 2009-12-23 09:14) - PLID 36269 - fixed discount calculation,
		// previously if you had no percentage it multiplied by zero, not one
		// (j.jones 2010-01-05 15:44) - PLID 15997 - we don't need this if a package, so save a recordset
		if(!m_checkPackage.GetCheck()) {
			rs = CreateParamRecordset("SELECT Sum(Round(Convert(money, (Amount * Quantity * "
				"(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)) "
				"- (CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END)),2)) AS Total FROM SurgeryDetailsT "
				"LEFT JOIN (SELECT SurgeryDetailID, SUM(Percentoff) as TotalPercentOff, SUM(Discount) as TotalDiscount FROM SurgeryDetailDiscountsT GROUP BY SurgeryDetailID) TotalDiscountsQ ON SurgeryDetailsT.ID = TotalDiscountsQ.SurgeryDetailID "
				"WHERE PayToPractice = 0 AND SurgeryID = {INT}",nSurgeryID);
			if(!rs->eof) {
				cyOutside = AdoFldCurrency(rs, "Total",COleCurrency(0,0));
			}
			rs->Close();
		}

		//Sort the list
		EnsureLineOrder();

		// (j.jones 2009-08-21 12:43) - PLID 35271 - removed Cost
		//SetDlgItemText(IDC_PRACTICE_COST, FormatCurrencyForInterface(cyCost));

		SetDlgItemText(IDC_PRACTICE_TOTAL, FormatCurrencyForInterface(cyToPractice));
		SetDlgItemText(IDC_OUTSIDE_TOTAL, FormatCurrencyForInterface(cyOutside));
		SetDlgItemText(IDC_TOTAL, FormatCurrencyForInterface(cyToPractice + cyOutside));

		// (j.jones 2010-01-05 15:44) - PLID 15997 - if a package, and we want to update the package total,
		// update the package total with the practice total
		if(m_checkPackage.GetCheck() && bUpdatePackageCost) {
			COleCurrency cyNewPackageTotal = cyToPractice;
			if(m_radioRepeatPackage.GetCheck()) {
				long nCount = GetDlgItemInt(IDC_SURGERY_PACKAGE_TOTAL_COUNT);
				if(nCount < 1 || nCount > 25000) {
					nCount = 1;
				}
				cyNewPackageTotal *= nCount;
			}

			//now compare to the existing total
			CString strExistingTotal;
			GetDlgItemText(IDC_SURGERY_PACKAGE_TOTAL_COST, strExistingTotal);
			
			COleCurrency cyCurrentPackageTotal;
			//replace if the total is invalid, or mismatches the practice total
			if(!cyCurrentPackageTotal.ParseCurrency(strExistingTotal) || cyCurrentPackageTotal < COleCurrency(0,0)
				|| cyCurrentPackageTotal != cyNewPackageTotal) {
				SetDlgItemText(IDC_SURGERY_PACKAGE_TOTAL_COST, FormatCurrencyForInterface(cyNewPackageTotal));
				ExecuteParamSql("UPDATE SurgeriesT SET PackageTotalAmount = Convert(money, {STRING}) WHERE ID = {INT}",
					FormatCurrencyForSql(cyNewPackageTotal), nSurgeryID);
				m_bPackageCostChanged = FALSE;
				m_SrgyChecker.Refresh();
			}
		}

	} NxCatchAllCall("CMainSurgeries::UpdateTotals", {
		// There was an error, reflect that fact on screen by blanking the totals

		// (j.jones 2009-08-21 12:43) - PLID 35271 - removed Cost
		//SetDlgItemText(IDC_PRACTICE_COST, "");
		SetDlgItemText(IDC_PRACTICE_TOTAL, "");
		SetDlgItemText(IDC_OUTSIDE_TOTAL, "");
		SetDlgItemText(IDC_TOTAL, "");		

		// (j.jones 2010-01-05 15:44) - PLID 15997 - leave the package total unchanged
	});

	Invalidate();
}

void CMainSurgeries::Refresh() 
{	
	try {
		
		int id;
		_RecordsetPtr rs;

		EnsureRemoteData();

		// Network code that requeries combos
		if (m_CPTChecker.Changed())
		{
			_variant_t tmpVar;

			if(m_pCPTCodes->CurSel != -1)
				tmpVar = m_pCPTCodes->GetValue(m_pCPTCodes->CurSel, 0);
			m_pCPTCodes->Requery();
			m_pCPTCodes->TrySetSelByColumn(sicDetailID, tmpVar);
		}

		if (m_InventoryChecker.Changed())
		{
			_variant_t tmpVar;

			if(m_pProducts->CurSel != -1)
				tmpVar = m_pProducts->GetValue(m_pProducts->CurSel, prodID);
			m_pProducts->Requery();
			m_pProducts->TrySetSelByColumn(sicDetailID, tmpVar);
		}

		if (m_SrgyChecker.Changed())
		{
			_variant_t tmpVar;

			if(m_pSurgeryNames->CurSel != -1)
				tmpVar = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0);
			
			// (j.jones 2010-01-06 09:24) - PLID 36763 - apply the necessary filter
			CString strWhere = "";
			if(m_radioShowSurgeries.GetCheck()) {
				strWhere.Format("SurgeriesT.IsPackage = 0");
			}
			else if(m_radioShowPackages.GetCheck()) {
				strWhere.Format("SurgeriesT.IsPackage = 1");
			}
			m_pSurgeryNames->PutWhereClause((LPCTSTR)strWhere);
			m_pSurgeryNames->Requery();

			if(m_pSurgeryNames->SetSelByColumn(0, tmpVar) == -1
				&& m_pSurgeryNames->GetRowCount() > 0) {
				m_pSurgeryNames->PutCurSel(0);			
			}

			RefreshButtons();

			if(m_pSurgeryNames->GetCurSel() == -1) {
				m_pSurgeryItems->Clear();
				SetDlgItemText(IDC_NOTES, "");
				// (j.jones 2010-01-18 09:35) - PLID 24479 - clear the default anes/facility times
				SetDlgItemText(IDC_EDIT_DEF_ANESTH_MINUTES, "");
				SetDlgItemText(IDC_EDIT_DEF_FACILITY_MINUTES, "");
				m_nxtDefAnesthStart->Clear();
				m_nxtDefAnesthEnd->Clear();
				m_nxtDefFacilityStart->Clear();
				m_nxtDefFacilityEnd->Clear();
			}
			else {
				long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));
				CString tmpStr;
				tmpStr.Format("SurgeryID = %li", nSurgeryID);
				m_pSurgeryItems->WhereClause = (LPCTSTR)tmpStr;
				m_pSurgeryItems->Requery();
			}
		}

		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure
		/*
		if (m_UserChecker.Changed())
		{
			_variant_t tmpVar;

			if(m_pPersons->CurSel != -1)
				tmpVar = m_pPersons->GetValue(m_pPersons->CurSel, pccID);
			m_pPersons->Requery();
			m_pPersons->TrySetSelByColumn(sicDetailID, tmpVar);
		}

		if (m_ContactChecker.Changed())
		{
			_variant_t tmpVar;

			if(m_pPersons->CurSel != -1)
				tmpVar = m_pPersons->GetValue(m_pPersons->CurSel, pccID);
			m_pPersons->Requery();
			m_pPersons->TrySetSelByColumn(sicDetailID, tmpVar);
		}
		if(m_ProviderChecker.Changed()) {
			m_pSurgeryItems->GetColumn(sicProviderID)->ComboSource = "SELECT -1 AS ID, '<No Provider>' AS Name, 1 UNION SELECT PersonT.ID, (PersonT.[First] + ' ' + PersonT.Middle + ' ' + PersonT.[Last] + ' ' + PersonT.Title) AS Name, "
											"CASE WHEN PersonT.Archived = 0 THEN 1 ELSE 0 END "
											"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID";

			//DRT 2/12/04 - PLID 6373 - Requery if the table checker changes, might be an inactive provider.
			_variant_t tmpVar;
			if(m_pPersons->CurSel != -1)
				tmpVar = m_pPersons->GetValue(m_pPersons->CurSel, pccID);
			m_pPersons->Requery();
			m_pPersons->TrySetSelByColumn(sicDetailID, tmpVar);
		}
		*/

		if(m_pSurgeryNames->CurSel == -1)
			m_pSurgeryNames->CurSel = 0;

		if(m_pSurgeryNames->CurSel == -1) {

			// (j.jones 2010-03-17 14:06) - PLID 36763 - the totals at the bottom need hidden if using the package filter

			COleCurrency cyZero(0,0);
			SetDlgItemText(IDC_PRACTICE_TOTAL, FormatCurrencyForInterface(cyZero));
			SetDlgItemText(IDC_OUTSIDE_TOTAL, FormatCurrencyForInterface(cyZero));
			SetDlgItemText(IDC_TOTAL, FormatCurrencyForInterface(cyZero));

			BOOL bShowTotals = !m_radioShowPackages.GetCheck();
			if(m_radioShowPackages.GetCheck()) {
				GetDlgItem(IDC_PRACTICE_TOTAL_LABEL)->ShowWindow(bShowTotals ? SW_SHOW : SW_HIDE);
				GetDlgItem(IDC_PRACTICE_TOTAL)->ShowWindow(bShowTotals ? SW_SHOW : SW_HIDE);
				GetDlgItem(IDC_OUTSIDE_TOTAL_LABEL)->ShowWindow(bShowTotals ? SW_SHOW : SW_HIDE);
				GetDlgItem(IDC_OUTSIDE_TOTAL)->ShowWindow(bShowTotals ? SW_SHOW : SW_HIDE);
				GetDlgItem(IDC_TOTAL_LABEL)->ShowWindow(bShowTotals ? SW_SHOW : SW_HIDE);
				GetDlgItem(IDC_TOTAL)->ShowWindow(bShowTotals ? SW_SHOW : SW_HIDE);				
			}
			Invalidate();

			return;
		}

		_variant_t tmpVar = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0);
		if(tmpVar.vt == VT_I4)
			id = tmpVar.lVal;
		else
			id = -1;
		CString tmpStr;
		tmpStr.Format("SurgeryID = %li",id);
		if(CString((LPCTSTR)m_pSurgeryItems->WhereClause) != tmpStr) {
			m_pSurgeryItems->WhereClause = (LPCTSTR)tmpStr;
			m_pSurgeryItems->Requery();
		}
		
		// (j.jones 2010-01-05 14:37) - PLID 15997 - supported packages
		rs = CreateParamRecordset("SELECT Description, IsPackage, PackageType, PackageTotalAmount, PackageTotalCount, "
			"AnesthStartTime, AnesthEndTime, AnesthMinutes, "
			"FacilityStartTime, FacilityEndTime, FacilityMinutes "
			"FROM SurgeriesT WHERE ID = {INT}", id);
		if(!rs->eof){
			tmpVar = rs->Fields->GetItem("Description")->Value;
			if (tmpVar.vt == VT_BSTR)
				SetDlgItemVar(IDC_NOTES, tmpVar, true, true);
			else
				SetDlgItemText(IDC_NOTES, "");

			// (j.jones 2010-01-18 09:35) - PLID 24479 - load the default anes/facility times
			_variant_t var = rs->Fields->Item["AnesthStartTime"]->Value;
			if (var.vt != VT_NULL) {
				m_nxtDefAnesthStart->SetDateTime(VarDateTime(var));
			}
			else {
				m_nxtDefAnesthStart->Clear();
			}

			var = rs->Fields->Item["AnesthEndTime"]->Value;
			if (var.vt != VT_NULL) {
				m_nxtDefAnesthEnd->SetDateTime(VarDateTime(var));
			}
			else {
				m_nxtDefAnesthEnd->Clear();
			}

			var = rs->Fields->Item["FacilityStartTime"]->Value;
			if (var.vt != VT_NULL) {
				m_nxtDefFacilityStart->SetDateTime(VarDateTime(var));
			}
			else {
				m_nxtDefFacilityStart->Clear();
			}

			var = rs->Fields->Item["FacilityEndTime"]->Value;
			if (var.vt != VT_NULL) {
				m_nxtDefFacilityEnd->SetDateTime(VarDateTime(var));
			}
			else {
				m_nxtDefFacilityEnd->Clear();
			}

			var = rs->Fields->Item["AnesthMinutes"]->Value;
			if (var.vt != VT_NULL) {			
				long nTotalMinutes = VarLong(var);
				SetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES, nTotalMinutes);
			}
			else {
				SetDlgItemText(IDC_EDIT_DEF_ANESTH_MINUTES, "");
			}

			var = rs->Fields->Item["FacilityMinutes"]->Value;
			if (var.vt != VT_NULL) {
				long nTotalMinutes = VarLong(var);
				SetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES, nTotalMinutes);
			}
			else {
				SetDlgItemText(IDC_EDIT_DEF_FACILITY_MINUTES, "");
			}

			// (j.jones 2010-01-05 14:37) - PLID 15997 - supported packages			
			BOOL bIsPackage = AdoFldBool(rs, "IsPackage", FALSE);
			long nPackageType = AdoFldLong(rs, "PackageType", 1);
			COleCurrency cyPackageTotal = AdoFldCurrency(rs, "PackageTotalAmount", COleCurrency(0,0));
			long nPackageCount = AdoFldLong(rs, "PackageTotalCount", 1);

			m_checkPackage.SetCheck(bIsPackage);
			if(nPackageType == 2) {
				//multi-use
				m_radioRepeatPackage.SetCheck(FALSE);
				m_radioMultiUsePackage.SetCheck(TRUE);			
			}
			else {
				//repeatable
				m_radioRepeatPackage.SetCheck(TRUE);
				m_radioMultiUsePackage.SetCheck(FALSE);				
			}

			SetDlgItemText(IDC_SURGERY_PACKAGE_TOTAL_COST, FormatCurrencyForInterface(cyPackageTotal));
			SetDlgItemInt(IDC_SURGERY_PACKAGE_TOTAL_COUNT, nPackageCount);
		}
		else {
			//should be impossible, but hide the package info anyways
			m_checkPackage.SetCheck(FALSE);
			m_radioRepeatPackage.SetCheck(TRUE);
			m_radioMultiUsePackage.SetCheck(FALSE);	
		}
		rs->Close();

		//now display the package fields appropriately
		DisplayPackageInfo();

		RefreshButtons();

		m_bNotesChanged = FALSE;
		m_bPackageCostChanged = FALSE;
		m_bPackageCountChanged = FALSE;

		// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
		m_bAnesthMinutesChanged = FALSE;
		m_bAnesthStartTimeChanged = FALSE;
		m_bAnesthEndTimeChanged = FALSE;
		m_bFacilityMinutesChanged = FALSE;
		m_bFacilityStartTimeChanged = FALSE;
		m_bFacilityEndTimeChanged = FALSE;

	}NxCatchAll("Error in Refresh()");
}

void CMainSurgeries::OnKillfocusNotes() 
{
	if(m_pSurgeryNames->CurSel == -1 || !m_bNotesChanged)
		return;

	CString desc;
	int id = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0).lVal;

	GetDlgItemText(IDC_NOTES, desc);
	try
	{	
		if(desc.GetLength() > 1000){
			MessageBox("A surgery's notes can have no more than 1000 characters. These notes will be truncated, so please check the new text.");
			desc = desc.Left(1000);
			SetDlgItemText(IDC_NOTES,desc);
		}

		ExecuteParamSql("UPDATE SurgeriesT SET Description = {STRING} WHERE ID = {INT}", desc, id);

		m_bNotesChanged = FALSE;

		// Network code
		m_SrgyChecker.Refresh();

	}NxCatchAll("Error in OnKillfocusNotes()");
}

void CMainSurgeries::OnSelChosenSurgeryNames(long nNewSel) 
{
	//we've changed items, so we no longer know the status of m_bManualChange
	//the Refresh handles figuring it out
	m_bManualChange = false;
	m_pSurgeryItems->PutAllowSort(TRUE);
	CheckDlgButton(IDC_SEARCH_VIEW, TRUE);	//set this check - the refresh process will uncheck it if necessary

	Refresh();
}

void CMainSurgeries::OnSelChosenSurgeryCptcodes(long nNewSel) 
{
	if(m_pSurgeryNames->CurSel==-1)
		return;

	if(nNewSel == -1)
		return;

	int nNewID = NewNumber("SurgeryDetailsT", "ID");

	try {

		long ServiceID = m_pCPTCodes->GetValue(nNewSel,0).lVal;
		long SurgeryID = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0).lVal;

		//check and see if this is an anesthesia code or facility code, and already exists in the charge list
		if(!CheckAllowAddAnesthesiaFacilityCharge(ServiceID))
			return;

		//first see if it is a duplicate
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM SurgeryDetailsT WHERE ServiceID = %li AND SurgeryID = %li ORDER BY ID",ServiceID,SurgeryID);
		if(!rs->eof) {
			//it is, now use the preference
			long nAddIncreaseSurgeryItems = GetRemotePropertyInt("AddIncreaseSurgeryItems",1,0,"<None>",TRUE);
			
			int nResult = IDNO;
			if(nAddIncreaseSurgeryItems == 3) {
				nResult = MessageBox("The selected Service Code is already in the current surgery.\n"
					"Would you like to update the existing item's quantity?\n\n"
					"(If 'No', then it will be added as a new line.)","Practice",MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL)
					return;
			}

			if(nAddIncreaseSurgeryItems == 2 || nResult == IDYES) {

				//if we are updating the quantity, do it here, then return, otherwise continue as normal

				long ID = AdoFldLong(rs, "ID");
				ExecuteSql("UPDATE SurgeryDetailsT SET Quantity = Quantity + 1.0 WHERE ID = %li",ID);

				//now update the datalist
				long nRow = m_pSurgeryItems->FindByColumn(sicDetailID,(long)ID,0,FALSE);

				if(nRow != -1) {
					m_pSurgeryItems->PutValue(nRow,sicQuantity,(double)(VarDouble(m_pSurgeryItems->GetValue(nRow,sicQuantity)) + 1.0));
				}

				UpdateTotals(TRUE);
				m_SrgyChecker.Refresh();
				return;
			}
		}
		rs->Close();
		
		//add new
		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure
		ExecuteSql("INSERT INTO SurgeryDetailsT (ID, SurgeryID, ServiceID, Amount, Quantity, PayToPractice) "
		"SELECT %li, %li, ID, Price, 1.0, -1 FROM ServiceT WHERE ID = %li;", nNewID, SurgeryID, ServiceID);
		m_SrgyChecker.Refresh();
	}NxCatchAll("Error in OnSelChosenSurgeryCptcodes()");

	try {
		m_pSurgeryItems->SetRedraw(VARIANT_FALSE);

		// find the order to add the item
		long nAddLineOrder = PrepareOrderForAdd(1);

		//add the items manually to the list
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow;
		pRow = m_pSurgeryItems->GetRow(-1);
		pRow->PutValue(sicDetailID, (long)nNewID);
		pRow->PutValue(sicItem, _bstr_t("Service Code"));
		// (j.jones 2008-07-01 10:52) - PLID 30578 - added ItemType column
		pRow->PutValue(sicItemType, (long)sitCPTCode);
		pRow->PutValue(sicProviderID, varNull);
		pRow->PutValue(sicCPTCodeProductID, m_pCPTCodes->GetValue(nNewSel, 1));
		pRow->PutValue(sicName, m_pCPTCodes->GetValue(nNewSel, 3));
		pRow->PutValue(sicAmount, m_pCPTCodes->GetValue(nNewSel, 4));
		pRow->PutValue(sicQuantity, (double)1.0);
		// (j.gruber 2009-03-19 11:08) - PLID 33361 - take out old discount fields and add total discount
		pRow->PutValue(sicTotalDiscountAmt, _variant_t(COleCurrency(0,0)));
		//pRow->PutValue(sicPercentOff, (long)0);
		//pRow->PutValue(sicDiscount, _variant_t(COleCurrency(0,0)));
		// (j.gruber 2007-05-14 15:09) - PLID 25173 - add discount categories
		//extern CPracticeApp theApp;
		//HICON hCat = theApp.LoadIcon(IDI_DISCOUNT_CATEGORY);
		//pRow->PutValue(sicHasDiscountCategory,(long)hCat);
		//pRow->PutValue(sicDiscountCategoryID, varNull);
		//pRow->PutValue(sicCustomDiscountDesc, varNull);
		pRow->PutValue(sicPayToPractice, g_cvarTrue);
		pRow->PutValue(sicLineOrder, long(nAddLineOrder));
		pRow->PutValue(sicLineTotalAmt, varNull);
		pRow->PutValue(sicProcDescription, _variant_t("..."));
		pRow->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));

		m_pSurgeryItems->AddRow(pRow);

		//calculate the totals instead of requerying
		UpdateTotals(TRUE);

		m_pSurgeryItems->SetRedraw(VARIANT_TRUE);
	} NxCatchAll("Error ordering service code into surgery.");
}

void CMainSurgeries::OnSelChosenSurgeryProducts(long nNewSel) 
{
	if(m_pSurgeryNames->CurSel==-1)
		return;

	if(nNewSel == -1)
		return;

	int nNewID = NewNumber("SurgeryDetailsT", "ID");

	BOOL bWarnProductBillable = FALSE;

	try	{

		long ServiceID = m_pProducts->GetValue(nNewSel, prodID).lVal;
		long SurgeryID = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0).lVal;

		//first see if it is a duplicate
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM SurgeryDetailsT WHERE ServiceID = %li AND SurgeryID = %li ORDER BY ID",ServiceID,SurgeryID);
		if(!rs->eof) {
			//it is, now use the preference
			long nAddIncreaseSurgeryItems = GetRemotePropertyInt("AddIncreaseSurgeryItems",1,0,"<None>",TRUE);
			
			int nResult = IDNO;
			if(nAddIncreaseSurgeryItems == 3) {
				nResult = MessageBox("The selected Inventory Item is already in the current surgery.\n"
					"Would you like to update the existing item's quantity?\n\n"
					"(If 'No', then it will be added as a new line.)","Practice",MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL)
					return;
			}

			if(nAddIncreaseSurgeryItems == 2 || nResult == IDYES) {

				//if we are updating the quantity, do it here, then return, otherwise continue as normal

				long ID = AdoFldLong(rs, "ID");
				ExecuteSql("UPDATE SurgeryDetailsT SET Quantity = Quantity + 1.0 WHERE ID = %li",ID);

				//now update the datalist
				long nRow = m_pSurgeryItems->FindByColumn(sicDetailID,(long)ID,0,FALSE);

				if(nRow != -1) {
					m_pSurgeryItems->PutValue(nRow,sicQuantity,(double)(VarDouble(m_pSurgeryItems->GetValue(nRow,sicQuantity)) + 1.0));
				}

				UpdateTotals(TRUE);
				m_SrgyChecker.Refresh();
				return;
			}
		}
		rs->Close();
				
		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure
		ExecuteSql("INSERT INTO SurgeryDetailsT (ID, SurgeryID, ServiceID, Amount, Quantity, PayToPractice) "
				"SELECT %li, %li, ServiceT.ID, Price, 1.0, -1 "
				"FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
				"WHERE ServiceT.ID = %li", nNewID, SurgeryID, ServiceID);
		m_SrgyChecker.Refresh();
	}NxCatchAll("Error in OnSelChosenSurgeryProducts()");

	try {

		m_pSurgeryItems->SetRedraw(VARIANT_FALSE);

		// find the order to add the item
		long nAddLineOrder = PrepareOrderForAdd(2);

		//add the items manually to the list
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow;
		pRow = m_pSurgeryItems->GetRow(-1);
		pRow->PutValue(sicDetailID, (long)nNewID);
		pRow->PutValue(sicItem, _bstr_t("Inventory"));
		// (j.jones 2008-07-01 10:52) - PLID 30578 - added ItemType column
		pRow->PutValue(sicItemType, (long)sitProduct);
		pRow->PutValue(sicProviderID, varNull);
		_RecordsetPtr rs = CreateRecordset("SELECT InsCode FROM ProductT WHERE InsCode <> '' AND ID = %li",VarLong(m_pProducts->GetValue(nNewSel, prodID)));
		if(!rs->eof) {
			pRow->PutValue(sicCPTCodeProductID, rs->Fields->Item["InsCode"]->Value);
		}
		else {
			pRow->PutValue(sicCPTCodeProductID, varNull);
		}
		rs->Close();
		pRow->PutValue(sicName, m_pProducts->GetValue(nNewSel, prodName));
		pRow->PutValue(sicAmount, m_pProducts->GetValue(nNewSel, prodPrice));
		pRow->PutValue(sicQuantity, (double)1.0);
		// (j.gruber 2009-03-19 11:10) - PLID 33361 - take out old discount fields and add total discount
		pRow->PutValue(sicTotalDiscountAmt, _variant_t(COleCurrency(0,0)));
		//pRow->PutValue(sicPercentOff, (long)0);
		//pRow->PutValue(sicDiscount, _variant_t(COleCurrency(0,0)));
		// (j.gruber 2007-05-14 15:09) - PLID 25173 - add discount categories
		//extern CPracticeApp theApp;
		//HICON hCat = theApp.LoadIcon(IDI_DISCOUNT_CATEGORY);
		//pRow->PutValue(sicHasDiscountCategory,(long)hCat);
		//pRow->PutValue(sicDiscountCategoryID, varNull);
		//pRow->PutValue(sicCustomDiscountDesc, varNull);
		pRow->PutValue(sicPayToPractice, g_cvarTrue);

		long nServiceID = VarLong(m_pProducts->GetValue(nNewSel, prodID));
		
		// (j.jones 2007-02-21 09:52) - PLID 24197 - warn the user if the product is
		// marked as not billable for any location, then mark it billable accordingly
		// (j.jones 2009-08-20 17:19) - PLID 35271 - the surgery's billable status is gone, so just
		// warn if it is not a billable product anywhere.
		rs = CreateParamRecordset("SELECT Count(ProductID) AS NumberOfEntries, "
			"Sum(CASE WHEN Billable = 1 THEN 1 ELSE 0 END) AS NumberBillable "
			"FROM ProductLocationInfoT WHERE ProductID = {INT}", nServiceID);
		if(!rs->eof) {
			long nNumEntries = AdoFldLong(rs, "NumberOfEntries",0);
			long nNumBillable = AdoFldLong(rs, "NumberBillable",0);
			if(nNumEntries != nNumBillable) {
				//it's not billable somewhere, so warn
				bWarnProductBillable = TRUE;				
			}
		}
		rs->Close();		

		pRow->PutValue(sicLineOrder, long(nAddLineOrder));
		pRow->PutValue(sicLineTotalAmt, varNull);
		pRow->PutValue(sicProcDescription, _variant_t("..."));
		pRow->PutBackColor(GetNxColor(GNC_PRODUCT,-1));

		m_pSurgeryItems->AddRow(pRow);

		//calculate the totals instead of requerying
		UpdateTotals(TRUE);

		m_pSurgeryItems->SetRedraw(VARIANT_TRUE);

		//now warn
		if(bWarnProductBillable)
			AfxMessageBox("Warning: this product is not marked as billable for all locations.\n"
					"This surgery will behave differently per location unless corrected in Inventory.");

	} NxCatchAll("Error updating surgery list");
}

// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
// which is now its own dialog and data structure
/*
void CMainSurgeries::OnSelChosenSurgeryPersons(long nRow) 
{
	if(m_pSurgeryNames->CurSel==-1)
		return;

	if(nRow == -1)
		return;

	long nNewID = -1;

	COleCurrency cyCost = COleCurrency(0,0);

	try {

		long PersonID = VarLong(m_pPersons->GetValue(nRow, pccID));
		long SurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0));

		cyCost = VarCurrency(m_pPersons->GetValue(nRow, pccCost),COleCurrency(0,0));

		//first see if it is a duplicate
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM SurgeryDetailsT WHERE PersonID = %li AND SurgeryID = %li ORDER BY ID",PersonID,SurgeryID);
		if(!rs->eof) {
			//it is, now use the preference
			long nAddIncreaseSurgeryItems = GetRemotePropertyInt("AddIncreaseSurgeryItems",1,0,"<None>",TRUE);
			
			int nResult = IDNO;
			if(nAddIncreaseSurgeryItems == 3) {
				nResult = MessageBox("The selected Person is already in the current surgery.\n"
					"Would you like to update the existing person's quantity?\n\n"
					"(If 'No', then the person will be added as a new line.)","Practice",MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nResult == IDCANCEL)
					return;
			}

			if(nAddIncreaseSurgeryItems == 2 || nResult == IDYES) {

				//if we are updating the quantity, do it here, then return, otherwise continue as normal

				long ID = AdoFldLong(rs, "ID");
				ExecuteSql("UPDATE SurgeryDetailsT SET Quantity = Quantity + 1.0 WHERE ID = %li",ID);

				//now update the datalist
				long nRow = m_pSurgeryItems->FindByColumn(sicDetailID,(long)ID,0,FALSE);

				if(nRow != -1) {
					m_pSurgeryItems->PutValue(nRow,sicQuantity,(double)(VarDouble(m_pSurgeryItems->GetValue(nRow,sicQuantity)) + 1.0));
				}

				UpdateTotals();
				m_SrgyChecker.Refresh();
				return;
			}
		}
		rs->Close();

		nNewID = NewNumber("SurgeryDetailsT", "ID");
		ExecuteSql("INSERT INTO SurgeryDetailsT (ID, SurgeryID, PersonID, Amount, Cost, Quantity, PayToPractice, Billable) "
			"SELECT %li, %li, %li, Convert(money,'$0.00'), Convert(money,'%s'), 1.0, -1, 0", nNewID,SurgeryID,PersonID,FormatCurrencyForSql(cyCost));
			m_SrgyChecker.Refresh();
	}NxCatchAll("Error in OnSelChosenSurgeryPersons()");

	m_pSurgeryItems->SetRedraw(VARIANT_FALSE);

	try {
		// find the order to add the item
		long nAddLineOrder = PrepareOrderForAdd(3);

		//add the items manually to the list
		_variant_t varNull;
		varNull.vt = VT_NULL;
		IRowSettingsPtr pRow;
		pRow = m_pSurgeryItems->GetRow(-1);
		pRow->PutValue(sicDetailID, (long)nNewID);
		pRow->PutValue(sicItem, _bstr_t("Personnel"));
		// (j.jones 2008-07-01 10:52) - PLID 30578 - added ItemType column
		pRow->PutValue(sicItemType, (long)sitPersonnel);
		pRow->PutValue(sicProviderID, varNull);
		pRow->PutValue(sicCPTCodeProductID, varNull);
		pRow->PutValue(sicName, m_pPersons->GetValue(nRow, pccName));
		pRow->PutValue(sicAmount, varNull);
		// (j.jones 2008-07-01 10:47) - PLID 18744 - don't fill the cost if they can't read it
		if(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead) {
			pRow->PutValue(sicVisibleCost, _variant_t(cyCost));
		}
		else {
			pRow->PutValue(sicVisibleCost, varNull);
			pRow->PutCellBackColor(sicVisibleCost,RGB(230,230,230));
		}
		pRow->PutValue(sicTrueCost, _variant_t(cyCost));
		pRow->PutValue(sicQuantity, (double)1.0);
		// (j.gruber 2009-03-19 11:12) - PLID 33361 - take out discount fields and add total discount
		pRow->PutValue(sicTotalDiscountAmt, _variant_t(COleCurrency(0,0)));
		//pRow->PutValue(sicPercentOff, varNull);
		//pRow->PutValue(sicDiscount, varNull);
		// (j.gruber 2007-05-14 15:09) - PLID 25173 - add discount categories
		//pRow->PutValue(sicHasDiscountCategory,varNull);
		//pRow->PutValue(sicDiscountCategoryID, varNull);
		//pRow->PutValue(sicCustomDiscountDesc, varNull);
		pRow->PutValue(sicPayToPractice, varNull);
		pRow->PutValue(sicBillable, varNull);
		pRow->PutValue(sicLineOrder, long(nAddLineOrder));
		pRow->PutValue(sicLineTotalAmt, varNull);
		pRow->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));
		pRow->PutCellBackColor(sicAmount,RGB(230,230,230));
		// (j.gruber 2009-03-19 12:37) - PLID 33361 - take out percent, add total discount
		pRow->PutCellBackColor(sicTotalDiscountAmt,RGB(230,230,230));
//		pRow->PutCellBackColor(sicPercentOff,RGB(230,230,230));
		//pRow->PutCellBackColor(sicDiscount,RGB(230,230,230));

		m_pSurgeryItems->AddRow(pRow);

		//calculate the totals instead of requerying
		UpdateTotals();

		m_pSurgeryItems->SetRedraw(VARIANT_TRUE);

	} NxCatchAll("Error updating surgery for person.");
}
*/

void CMainSurgeries::OnRButtonDownSurgeryItems(long nRow, long nCol, long x, long y, long nFlags) 
{
	if(nRow==-1)
		return;

	m_pSurgeryItems->CurSel = nRow;
	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();
	pMenu->InsertMenu(-1, MF_BYPOSITION, ID_DELETE_ITEM, "Delete");
	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}

void CMainSurgeries::OnDeleteItem()
{
	try	{

		if(m_pSurgeryNames->CurSel==-1)
			return;

		if(IDNO == MessageBox("Are you sure you wish to delete the selected item?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		// (j.gruber 2009-03-19 11:13) - PLID 33361 - delete any discount this item might have
		ExecuteParamSql("DELETE FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = {INT}", m_pSurgeryItems->GetValue(m_pSurgeryItems->CurSel, sicDetailID).lVal);

		ExecuteSql("DELETE FROM SurgeryDetailsT WHERE ID = %li", m_pSurgeryItems->GetValue(m_pSurgeryItems->CurSel, sicDetailID).lVal);				

		m_pSurgeryItems->RemoveRow(m_pSurgeryItems->CurSel);
		UpdateTotals(TRUE);

		m_SrgyChecker.Refresh();
	}NxCatchAll("Error in OnDeleteItem()");
}

void CMainSurgeries::OnEditingFinishingSurgeryItems(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	try {

		if(nCol == sicQuantity && pvarNewValue->vt == VT_R8) {

			if(pvarNewValue->dblVal <= 0.0) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You must have a quantity greater than zero.");
				return;
			}
		}

		// (j.gruber 2009-03-19 11:14) - PLID 33361 - take out old discount fields
		/*if(nCol == sicPercentOff && pvarNewValue->vt == VT_I4) {

			if(pvarNewValue->lVal > 100) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("Please enter a value no greater than 100.");
				return;
			}
			else if(pvarNewValue->lVal < 0) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter a negative value.");
				return;
			}
		}

		if(nCol == sicDiscount && pvarNewValue->vt == VT_CY) {

			if(COleCurrency(pvarNewValue->cyVal) < COleCurrency(0,0)) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You cannot enter a negative value.");
				return;
			}
		}

		//if the discount is not null, then it is possible we could enter
		//a situation where the total value of this line is negative,
		//which must be stopped
		if(m_pSurgeryItems->GetValue(nRow,sicDiscount).vt != VT_NULL) {
			COleCurrency cyAmount = VarCurrency(m_pSurgeryItems->GetValue(nRow,sicAmount), COleCurrency(0,0));
			if(nCol == sicAmount)
				cyAmount = VarCurrency(pvarNewValue,COleCurrency(0,0));

			double dblQuantity = VarDouble(m_pSurgeryItems->GetValue(nRow,sicQuantity), 0.0);
			if(nCol == sicQuantity)
				dblQuantity = VarDouble(pvarNewValue,0.0);

			long nPercentOff = VarLong(m_pSurgeryItems->GetValue(nRow,sicPercentOff), 0);
			if(nCol == sicPercentOff)
				nPercentOff = VarLong(pvarNewValue,0);

			COleCurrency cyDiscount = VarCurrency(m_pSurgeryItems->GetValue(nRow,sicDiscount), COleCurrency(0,0));
			if(nCol == sicDiscount)
				cyDiscount = VarCurrency(pvarNewValue,COleCurrency(0,0));

			_RecordsetPtr rs = CreateRecordset("SELECT Round(Convert(money, (Convert(money,'%s') * %g * "
				"((100-Convert(float,%li))/100) "
				"- Convert(money,'%s'))),2) AS LineTotal",
				FormatCurrencyForSql(cyAmount), dblQuantity, nPercentOff, FormatCurrencyForSql(cyDiscount));
			if(!rs->eof) {
				COleCurrency cyLineTotal = AdoFldCurrency(rs, "LineTotal",COleCurrency(0,0));
				if(cyLineTotal < COleCurrency(0,0)) {
					*pvarNewValue = varOldValue;
					*pbCommit = FALSE;
					AfxMessageBox("You cannot have a line total less than zero. (Check your discount amounts.)");
					return;
				}
			}
			rs->Close();
		}*/

	}NxCatchAll("Error in CMainSurgeries::OnEditingFinishingSurgeryItems");
}

void CMainSurgeries::OnEditingFinishedSurgeryItems(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		_variant_t tmpVar = m_pSurgeryItems->GetValue(nRow, nCol);
		switch(nCol){
		case sicProviderID: {
			CString strProviderID = "NULL";
			if(varNewValue.vt == VT_I4 && varNewValue.lVal != -1)
				strProviderID.Format("%li",varNewValue.lVal);
			ExecuteSql("UPDATE SurgeryDetailsT SET ProviderID = %s WHERE ID = %li", strProviderID, VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));
			break;
		}
		case sicAmount:
			ExecuteSql("UPDATE SurgeryDetailsT SET Amount = Convert(money,'%s') WHERE ID = %li", _Q(FormatCurrencyForSql(VarCurrency(tmpVar, COleCurrency(0,0)))), VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));
			// (j.gruber 2009-03-19 11:21) - PLID 33361 - this would change the discount amount, so we need to recalculate
			UpdateTotalDiscountAmt(nRow);
			break;
		case sicQuantity:
			ExecuteSql("UPDATE SurgeryDetailsT SET Quantity = %g WHERE ID = %li", VarDouble(tmpVar, 1.0), VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));
			// (j.gruber 2009-03-19 11:21) - PLID 33361 - this would change the discount amount, so we need to recalculate
			UpdateTotalDiscountAmt(nRow);
			break;
       /*// (j.gruber 2009-03-19 11:23) - PLID 33361 - take out old discount fields
		case sicPercentOff:
			ExecuteSql("UPDATE SurgeryDetailsT SET PercentOff = %li WHERE ID = %li", VarLong(tmpVar,0), VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));
			break;
		case sicDiscount:
			ExecuteSql("UPDATE SurgeryDetailsT SET Discount = Convert(money,'%s') WHERE ID = %li", _Q(FormatCurrencyForSql(VarCurrency(tmpVar, COleCurrency(0,0)))), VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));
			break;*/
		case sicPayToPractice:
			ExecuteSql("UPDATE SurgeryDetailsT SET PayToPractice = %li WHERE ID = %li", VarBool(tmpVar,TRUE) ? -1:0, VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));			
			break;

		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure
		/*
		case sicVisibleCost:
			ExecuteSql("UPDATE SurgeryDetailsT SET Cost = Convert(money,'%s') WHERE ID = %li", _Q(FormatCurrencyForSql(VarCurrency(tmpVar, COleCurrency(0,0)))), VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1));
			// (j.jones 2008-07-01 10:44) - PLID 18744 - also copy to the TrueCost column
			m_pSurgeryItems->PutValue(nRow, sicTrueCost, varNewValue);
			break;

		case sicBillable:

			//this code must be last because it fires an AfxMessageBox, while permitting the change
			if(nCol == sicBillable && varNewValue.vt == VT_BOOL && VarBool(varNewValue) && IsSurgeryCenter(true)) {

				long nID = VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1);

				// (j.jones 2007-02-21 09:52) - PLID 24197 - if enabling Billable, and the item is a
				// product, warn the user if the product is marked as not billable for any location
				_RecordsetPtr rs = CreateRecordset("SELECT Count(ProductID) AS NumberOfEntries, "
					"Sum(CASE WHEN Billable = 1 THEN 1 ELSE 0 END) AS NumberBillable "
					"FROM ProductLocationInfoT WHERE ProductID IN "
					"(SELECT ServiceID FROM SurgeryDetailsT WHERE ID = %li)", nID);
				if(!rs->eof) {
					long nNumEntries = AdoFldLong(rs, "NumberOfEntries",0);
					long nNumBillable = AdoFldLong(rs, "NumberBillable",0);
					if(nNumEntries != nNumBillable) {
						//the product is not billable for at least one location
						AfxMessageBox("Warning: this product is not marked as billable for all locations.\n"
							"This surgery will behave differently per location unless corrected in Inventory.");
					}
				}
				rs->Close();			
			}

			if (IsSurgeryCenter(true)) {
				ExecuteSql("UPDATE SurgeryDetailsT SET Billable = %li WHERE ID = %li", VarBool(tmpVar,FALSE) ? -1:0, VarLong(m_pSurgeryItems->GetValue(nRow, sicDetailID), -1)); // TODO: I'm not sure why we allow NULL IDs here (by the extra parameter "-1"), but I'm just following the above examples.
			} else {
				AfxThrowNxException("This feature is only available if the Surgery Center license is installed.");
			}
			break;
		*/
		}

		// (j.jones 2010-01-05 16:00) - PLID 15997 - only update the package total if the amount,
		//quantity, or pay-to-practice changed
		UpdateTotals(nCol == sicAmount || nCol == sicQuantity || nCol == sicPayToPractice);

		m_SrgyChecker.Refresh();
	} NxCatchAll("Error 100: CMainSurgeries::OnEditingFinishedSurgeryItems");
	
}

void CMainSurgeries::OnRequeryFinishedSurgeryItems(short nFlags) 
{

	//Requery has occurred.  We need to determine if m_bManualChange should be set
	//If the items are not in default sorted order, we assume they have been 
	//changed, and as such, set that variable.
	if(!m_bManualChange) {
		if(!IsListDefaultSorted()) {
			m_bManualChange = true;
		}
		else {
		}
	}
	else {
		//we already know it's been manually changed, so don't bother doing any checking
		CheckDlgButton(IDC_SEARCH_VIEW, FALSE);
	}

	//Make sure our sort status is up to date
	if(!IsDlgButtonChecked(IDC_SEARCH_VIEW))
		m_pSurgeryItems->PutAllowSort(FALSE);
	else
		m_pSurgeryItems->PutAllowSort(TRUE);

	// (j.jones 2008-07-01 10:42) - PLID 18744 - track this permission rather
	// than checking it during each iteration of the loop
	//BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

	//gray out invalid cells
	long p = m_pSurgeryItems->GetFirstRowEnum();
	LPDISPATCH lpDisp = NULL;
	while (p) {
		m_pSurgeryItems->GetNextRowEnum(&p, &lpDisp);
		IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

		// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
		// which is now its own dialog and data structure

		/*

		//for persons, we don't want to let them edit the amount,
		
		// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
		if(VarLong(pRow->GetValue(sicItemType)) == sitPersonnel) {
			//color the Person row yellow-orange, like the Contacts module
			pRow->PutBackColor(GetNxColor(GNC_PERSONNEL,-1));

			//gray the Amount cell
			pRow->PutCellBackColor(sicAmount,RGB(230,230,230));
			// (j.gruber 2009-03-19 12:38) - PLID 33361 - take out percent fields, add totaldiscount
			//pRow->PutCellBackColor(sicPercentOff,RGB(230,230,230));
			//pRow->PutCellBackColor(sicDiscount,RGB(230,230,230));			
			pRow->PutCellBackColor(sicTotalDiscountAmt ,RGB(230,230,230));			

			//gray out the desc field
			pRow->PutValue(sicProcDescription, _variant_t(""));
			pRow->PutCellBackColor(sicProcDescription,RGB(230,230,230));
			pRow->PutCellLinkStyle(sicProcDescription, dlLinkStyleFalse);

			// (j.jones 2008-07-01 10:42) - PLID 18744 - the visible cost column loads as null by default for personnel,
			// so we must copy the true cost into the visible cost column, but only if they have read permissions
			if(bCanViewPersonCosts) {
				_variant_t varCost = pRow->GetValue(sicTrueCost);
				pRow->PutValue(sicVisibleCost, varCost);
			}
			else {
				//color the cost column gray
				pRow->PutCellBackColor(sicVisibleCost,RGB(230,230,230));
			}
		}

		*/

		//for CPT codes, we don't want to let them edit the cost,
		//and if there is a CPT ID, then we have a code

		// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
		if(VarLong(pRow->GetValue(sicItemType)) == sitCPTCode) {
			//color the CPT row red, like the Admin. module
			pRow->PutBackColor(GetNxColor(GNC_CPT_CODE,-1));

			//gray the Cost cell
			//pRow->PutCellBackColor(sicVisibleCost,RGB(230,230,230));

			pRow->PutCellLinkStyle(sicProcDescription, dlLinkStyleTrue);
		}

		//for Products, both are editable, just change the color of the row
		// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
		if(VarLong(pRow->GetValue(sicItemType)) == sitProduct) {
			//color the Product row blue, like the Inventory module
			pRow->PutBackColor(GetNxColor(GNC_PRODUCT,-1));
			pRow->PutCellLinkStyle(sicProcDescription, dlLinkStyleTrue);
		}

		// (j.gruber 2009-03-19 11:24) - PLID 33361 - take out
		// (j.gruber 2007-05-14 12:16) - PLID 25173 - set the discount category icon
		/*long nDiscountCategoryID = VarLong(pRow->GetValue(sicDiscountCategoryID), -9999);

		// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
		if(VarLong(pRow->GetValue(sicItemType)) == sitPersonnel) {

			//it's a personnel and they can't edit the discount, so they shouldn't edit the category
			pRow->PutCellBackColor(sicHasDiscountCategory,RGB(230,230,230));			
		}
		else {
			if (nDiscountCategoryID != -9999) {
				//set it to the green one
				extern CPracticeApp theApp;
				HICON hCat = theApp.LoadIcon(IDI_DISCOUNT_CAT_USED);
				pRow->PutValue(sicHasDiscountCategory,(long)hCat);
						
			}
			else {
				//set it to be not set
				extern CPracticeApp theApp;
				HICON hCat = theApp.LoadIcon(IDI_DISCOUNT_CATEGORY);
				pRow->PutValue(sicHasDiscountCategory,(long)hCat);
			}
		}*/
	
	}

	// (j.jones 2010-01-05 16:01) - PLID 15997 - do not update the package cost
	UpdateTotals(FALSE);
}

void CMainSurgeries::OnAdd() 
{
	CString sql;
	long nID;

	BOOL bSaved = FALSE;
	
	while (!bSaved)
	{
		CGetNewIDName NewSurgery(this);
		CString NewName;
		NewSurgery.m_pNewName = &NewName;
		NewSurgery.m_nMaxLength = 255;
		if(NewSurgery.DoModal() == IDOK) {

			try {
				//Make sure there is no surgery with this name
				if(ExistsInTable("SurgeriesT", "Name = '%s'", _Q(NewName))){
					// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
					// which is now its own dialog and data structure					
					MessageBox("A surgery exists with this name.  Please select a different name for this surgery.", "Nextech", MB_OK);
					continue;
				}
				if(NewName.GetLength() > 255){
					MessageBox("A surgery's name can have no more than 255 characters.");
					continue;
				}
				nID = NewNumber ("SurgeriesT", "ID");
				ExecuteSql("INSERT INTO SurgeriesT (ID, Name, Description) VALUES (%li, '%s', '');", nID, _Q(NewName));
				m_pSurgeryItems->Clear();

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, "", nAuditID, aeiSurgeryCreated, nID, "", NewName, aepMedium, aetCreated);				

				bSaved = TRUE;

				IRowSettingsPtr pRow = m_pSurgeryNames->GetRow(-1);
				pRow->PutValue(0, (long)nID);
				pRow->PutValue(1, _bstr_t(NewName));
				m_pSurgeryNames->AddRow(pRow);

				m_pSurgeryNames->SetSelByColumn(0,_variant_t(nID));

				// (j.jones 2010-01-06 09:31) - PLID 36763 - if we are filtered on packages,
				// change the filter to "all", since all new entries are surgeries
				// that the current surgery/package should no longer display, change the filter
				// to show all surgeries/packages
				if(m_radioShowPackages.GetCheck()) {
					m_radioShowPackages.SetCheck(FALSE);
					m_radioShowSurgeries.SetCheck(FALSE);
					m_radioShowAll.SetCheck(TRUE);

					//refilter and requery the surgery list by
					//tricking it into thinking network code changed
					m_SrgyChecker.m_changed = true;
				}

				// Network code
				m_SrgyChecker.Refresh();

				Refresh();

				GetDlgItem(IDC_SURGERY_NAMES)->EnableWindow(TRUE);
				
			}NxCatchAll("Error adding surgery.");
		}
		else
			return;
	}
}

void CMainSurgeries::OnRename() 
{
	try {

		if(m_pSurgeryNames->CurSel==-1)
			return;

		_variant_t var;
		long nID;

		BOOL bSaved = FALSE;

		var = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0);
		if (var.vt == VT_I4)
			nID = var.lVal;
		else return;//no item selected or error

		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM SurgeriesT WHERE ID = %li",nID);
		if(!rs->eof) {
			strOld = AdoFldString(rs, "Name","");
		}
		rs->Close();
		
		while (!bSaved)
		{
			CGetNewIDName NewSurgery(this);
			CString NewName = strOld;
			NewSurgery.m_pNewName = &NewName;
			NewSurgery.m_nMaxLength = 255;
			if(NewSurgery.DoModal() == IDOK) {

				//Make sure there is no surgery with this name
				if(!IsRecordsetEmpty("SELECT Name FROM SurgeriesT WHERE Name = '%s' AND SurgeriesT.ID <> %li", _Q(NewName),nID)) {
					// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
					// which is now its own dialog and data structure
					MessageBox("A surgery exists with this name.  Please select a different name for this surgery.", "Nextech", MB_OK);
					continue;
				}
				if(NewName.GetLength() > 255){
					MessageBox("A surgery's name can have no more than 255 characters.");
					continue;
				}

				
				ExecuteSql("UPDATE SurgeriesT SET Name = '%s' WHERE ID = %li;", _Q(NewName), nID);

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, "", nAuditID, aeiSurgeryName, nID, strOld, NewName, aepMedium, aetChanged);

				bSaved = TRUE;

				// (j.jones 2006-05-04 09:53) - we do need to requery, due to provider linking
				// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
				// which is now its own dialog and data structure
				/*
				if(IsSurgeryCenter(false)) {
					//if ASC, requery due to the provider naming
					m_pSurgeryNames->Requery();
					m_pSurgeryNames->SetSelByColumn(0,_variant_t(nID));
				}
				else {
				*/
				//otherwise just change the name in place
				m_pSurgeryNames->PutValue(m_pSurgeryNames->CurSel,1,_bstr_t(NewName));
				//}

				//JMJ - 6/23/2003 - Since we are only renaming the surgery, there is no need
				//to refresh the tab entirely
				//Refresh();

				// Network code
				m_SrgyChecker.Refresh();

			}
			else
				return;
		}

	}NxCatchAll("Error renaming surgery.");
}

void CMainSurgeries::OnDelete() 
{
	if(m_pSurgeryNames->CurSel==-1)
		return;

	if(MessageBox("Are you SURE you wish to remove this surgery?", "Remove Surgery", MB_YESNO) == IDNO)
		return;
	
	_variant_t		var;
	var = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0);
	if(var.vt == VT_I4)
	{	EnsureRemoteData();
		CString sql;
		sql.Format("DELETE FROM SurgeriesT WHERE ID = %i;", var.lVal);//referencial integrity will delete details
		try
		{	
			//for auditing
			CString strOld = CString(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 1).bstrVal);
			// (j.jones 2009-08-24 11:39) - PLID 35271 - removed SurgeryProvidersT
			//ExecuteSql("DELETE FROM SurgeryProvidersT WHERE SurgeryID = %li;", var.lVal);
			// (j.gruber 2009-03-19 11:26) - PLID 33361 - surgery discounts
			ExecuteSql("DELETE FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID IN (SELECT ID FROM SurgeryDetailsT WHERE SurgeryID = %li);", var.lVal);
			ExecuteSql("DELETE FROM SurgeryDetailsT WHERE SurgeryID = %li;", var.lVal);
			ExecuteSql("DELETE FROM SurgeriesT WHERE ID = %li;", var.lVal);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiSurgeryDeleted, long(var.lVal), strOld, "<Deleted>", aepMedium, aetDeleted);

			m_pSurgeryNames->RemoveRow(m_pSurgeryNames->CurSel);
			m_pSurgeryNames->CurSel = 0;
			Refresh();

			// Network code
			m_SrgyChecker.Refresh();

			if(m_pSurgeryNames->GetRowCount() == 0){
				GetDlgItem(IDC_SURGERY_NAMES)->EnableWindow(FALSE);
			}
			RefreshButtons();

		}NxCatchAllCall("Error in OnClickDelete()",{return;});		
	}	
}


void CMainSurgeries::OnQuoteAdmin() 
{
	CQuoteAdminDlg dlg(this);
	dlg.m_nColor = GetNxColor(GNC_ADMIN, 0);
	dlg.DoModal();
}

void CMainSurgeries::OnEditingStartingSurgeryItems(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		switch(nCol) {
			// (j.gruber 2009-03-19 11:27) - PLID 33361 - old percent fields taken out
			case sicAmount:
			/*case sicPercentOff:
			case sicDiscount:*/
				//for persons, we don't want to let them edit the amount,
				// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
				// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
				// which is now its own dialog and data structure
				/*
				if(VarLong(m_pSurgeryItems->GetValue(nRow, sicItemType)) == sitPersonnel) {
					*pbContinue = FALSE;
				}
				*/
				break;

			// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
			// which is now its own dialog and data structure
			/*
			case sicVisibleCost:
				//for CPT codes, we don't want to let them edit the cost,
				//and if there is a CPT ID, then we have a code
				// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
				if(VarLong(m_pSurgeryItems->GetValue(nRow, sicItemType)) == sitCPTCode) {
					*pbContinue = FALSE;
				}

				// (j.jones 2008-07-01 10:36) - PLID 18744 - if editing the cost for a person
				// check and see if they have permissions for that
				if(VarLong(m_pSurgeryItems->GetValue(nRow, sicItemType)) == sitPersonnel
					&& (!(GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)
					|| !(GetCurrentUserPermissions(bioContactsDefaultCost) & sptWrite))) {
					*pbContinue = FALSE;
				}
				break;

			//for persons, billable is null, which means no checkbox appears, therefore they can't check it,
			//so we don't need to stop them here

			*/
		}

	}NxCatchAll("Error in OnEditingStartingSurgeryItems()");
}

// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
// which is now its own dialog and data structure
/*
void CMainSurgeries::OnLinkToProviders() 
{
	try {

		if(m_pSurgeryNames->CurSel == -1) {
			AfxMessageBox("Please choose a surgery first.");
			return;
		}

		long SurgeryID = m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0).lVal;

		CSurgeryProviderLinkDlg dlg;
		dlg.m_SurgeryID = SurgeryID;
		if(dlg.DoModal() == IDOK) {
			m_pSurgeryNames->Requery();
			m_pSurgeryNames->SetSelByColumn(0,_variant_t(SurgeryID));
			
			//DRT 4/16/03 - no reason to refresh the whole tab, we're just updating the name
			//Refresh();

			// Network code
			m_SrgyChecker.Refresh();
		}
	
	}NxCatchAll("Error loading the surgery provider link.");
}
*/

void CMainSurgeries::OnRequeryFinishedSurgeryNames(short nFlags) 
{

}

void CMainSurgeries::EnsureLineOrder() {

	//DRT 10/20/2003 - PLID 9143 - If they are in the search view, we are not going to save any changes to the order of items
	if(IsDlgButtonChecked(IDC_SEARCH_VIEW))
		return;

	//if some item has been manually drug, we don't want to auto-order,
	//but we still want to save the order
	if(!m_bManualChange) {

		//////////////////////
		//Process for sorting:
		// 1)  Let the datalist sort on the sicLineTotalAmt column, descending.  Take the sort off the sicLineOrder column
		// 2)  Renumber all the IDs.  See the ReNumberIDs() function in billingdlg.cpp for a blueprint.
		// 3)  Set the sort back to sicLineOrder.
		// 4)  Save the current surgery to assure the order is saved.

		m_pSurgeryItems->SetRedraw(VARIANT_FALSE);

		//1)
		IColumnSettingsPtr pCol;
		
		pCol = m_pSurgeryItems->GetColumn(sicLineOrder);
		pCol->SortPriority = -1;
		pCol = m_pSurgeryItems->GetColumn(sicLineTotalAmt);
		pCol->SortPriority = 0;
		pCol->PutSortAscending(FALSE);
		m_pSurgeryItems->Sort();

		//2)
		RenumberIDs();

		//3)
		pCol = m_pSurgeryItems->GetColumn(sicLineTotalAmt);
		pCol->SortPriority = -1;
		pCol = m_pSurgeryItems->GetColumn(sicLineOrder);
		pCol->SortPriority = 0;
		pCol->PutSortAscending(TRUE);
		m_pSurgeryItems->Sort();

		m_pSurgeryItems->SetRedraw(VARIANT_TRUE);
	}
	else {
		//this will ensure that there are no gaps or duplicates before saving
		IColumnSettingsPtr pCol;
		pCol = m_pSurgeryItems->GetColumn(sicLineOrder);
		pCol->SortPriority = 0;
		pCol->PutSortAscending(TRUE);
		m_pSurgeryItems->Sort();	//sort by LineID
		//this is safe to call, because it does not do any re-ordering, it just
		//loops through the items and makes sure they are in correct number order
		RenumberIDs();
	}

	//4)
	SaveCurrentSurgeryOrder();

}

void CMainSurgeries::RenumberIDs() {
	//Renumbers the IDs starting at the top of the list and moving down
	//Borrowed most of the code from BillingDlg::ReNumberIDs()

	try {
		long nNewID = 0;

		for(int i = 0; i < m_pSurgeryItems->GetRowCount(); i++) {
			//we could use i, but this makes it more understandable
			nNewID++;
			m_pSurgeryItems->PutValue(i, sicLineOrder, long(nNewID));
		}

	} NxCatchAll("Error in ReNumberIDs");
}

bool CMainSurgeries::SaveCurrentSurgeryOrder() {
	//Saves the LineOrder ID to each record of this surgery

	try {

		CString str = "", strExec = "";
		for(int i = 0; i < m_pSurgeryItems->GetRowCount(); i++) {
			long nID = -1, nOrder = -1;
			nID = VarLong(m_pSurgeryItems->GetValue(i, sicDetailID));
			nOrder = VarLong(m_pSurgeryItems->GetValue(i, sicLineOrder));

			str.Format("UPDATE SurgeryDetailsT SET LineOrder = %li WHERE ID = %li;", nOrder, nID);

			strExec += str + "\n";
		}

		if(!strExec.IsEmpty()) {
			//now execute it all
			BEGIN_TRANS("UpdateSurgeryOrder") {
				ExecuteSql("%s", strExec);
			} END_TRANS_CATCH_ALL("UpdateSurgeryOrder");
		}

		return true;

	} NxCatchAll("Error saving surgery order.");

	return false;
}

bool CMainSurgeries::IsListDefaultSorted() {

	long nCount = m_pSurgeryItems->GetRowCount();

	if(nCount < 2) {
		//0 or 1 items, it's in order
		return true;
	}

	COleCurrency cyPrev(0, 0), cyCur(0, 0);
	cyPrev = VarCurrency(m_pSurgeryItems->GetValue(0, sicLineTotalAmt), COleCurrency(0, 0));

	for(int i = 1; i < nCount; i++) {
		//the comparison is fairly simple, we loop through each row in the list, 
		//and if our current value is greater than the previous value, we return
		//false.  If we finish the loop, we must return true;
		cyCur = VarCurrency(m_pSurgeryItems->GetValue(i, sicLineTotalAmt), COleCurrency(0, 0));

		if(cyCur > cyPrev) {
			//out of order
			return false;
		}

		//these ones are ok.  move cyCur into previous before the next go-round
		cyPrev = cyCur;
		cyCur = COleCurrency(0, 0);
	}

	return true;

}

void CMainSurgeries::IncrementListOrder(long nStartAt /*= 1*/) {
	//DRT 5/2/03 - Increments the order of all items in the list, starting at the given number
	//		nStartAt is the ROW NUMBER, now the LineOrder

	//DRT 11/20/2003 - PLID 10190 - we can't start before the first row, so if someone tried, fix it
	if(nStartAt < 0)
		nStartAt = 0;

	//start at the end, in case something bad happens.  We're better off with a gap in between then 
	//2 with the same ID
	for(int i = m_pSurgeryItems->GetRowCount()-1; i >= nStartAt; i--) {
		long nCurOrder = VarLong(m_pSurgeryItems->GetValue(i, sicLineOrder));	//get the order
		nCurOrder++;	//inc it
		m_pSurgeryItems->PutValue(i, sicLineOrder, long(nCurOrder));	//put it back
	}
}

long CMainSurgeries::FindPositionByAmount(COleCurrency cyFind) {
	//DRT 5/2/03 - Given an amount, it finds the position that amount should be placed at by looking
	//		for the first item with an amount less than the parameter.  The LineOrder position is 
	//		returned to the caller.

	long nOrder = -1;

	for(int i = 0; i < m_pSurgeryItems->GetRowCount() && nOrder == -1; i++) {
		COleCurrency cyList(0, 0);
		cyList = VarCurrency(m_pSurgeryItems->GetValue(i, sicLineTotalAmt), COleCurrency(0, 0));

		if(cyList < cyFind) {
			//current list value is less than what we're adding
			nOrder = VarLong(m_pSurgeryItems->GetValue(i, sicLineOrder));
		}
	}

	if(nOrder == -1)
		nOrder = m_pSurgeryItems->GetRowCount() + 1;

	return nOrder;
}

long CMainSurgeries::PrepareOrderForAdd(long nType) {
	//nType is the list we're adding from
	//1 == CPTCodes
	//2 == Products
	//3 == Personnel
	long nPriceCol;
	_DNxDataListPtr pList;
	if(nType == 1) {
		pList = m_pCPTCodes;
		nPriceCol = 4;
	}
	else if(nType == 2) {
		pList = m_pProducts;
		nPriceCol = prodPrice;
	}
	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	/*
	else if(nType == 3) {
		//there is no cost for persons, return it at the end
		return m_pSurgeryItems->GetRowCount();
	}
	*/
	else {
		//invalid
		AfxThrowNxException("Invalid type in PrepareOrderForAdd");
		return -1;
	}

	/////////////////////////////////////////////
	//For the ordering to work correctly, we need to do a few checks.  If m_bManualChange is false, the UpdateTotals()
	//		will handle re-sorting everything just dandy, so nothing to do there.  However, if m_bManualChange is set, then
	//		something has changed.  We need to "guess" where this item goes by searching through the list (top-down), and finding
	//		the first item with a total amount less than our new value.  Then we push everything down 1 LineOrder, and insert this
	//		one with the new LineOrder
	long nAddLineOrder = -1;	//if we don't need to do anything, this will be handled automatically.  Otherwise we need to set it.
	if(!m_bManualChange) {
		//don't need to do anything
	}
	else {
		//1)  Find the order of the first item with an amount below this one
		long nOrder = FindPositionByAmount(VarCurrency(pList->GetValue(pList->GetCurSel(), (short)nPriceCol), COleCurrency(0, 0)));

		//2)  Push all the items from that point down
		IncrementListOrder(nOrder-1);	//need to pass in the row number

		//3)  Insert our item with the correct line order
		//this is handled below regardless, so just give it the right ID to use
		nAddLineOrder = nOrder;
	}

	return nAddLineOrder;

	// End ordering for new items code
	/////////////////////////////////////////////
}

void CMainSurgeries::OnDragEndSurgeryItems(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags) 
{
	//////////////////////////////////
	//We have to handle our moving in 2 ways here:
	//	1)  We are moving from a lower ordered item (ex 8) to a higher order (ex 2).  In this case, we need to set the moved item
	//		to a temp value of -1, then shift everything down starting at where we drug to (nRow).  Then change
	//		the temp ID to the value it was drug to (nRow).
	//	2)  We are moving from a higher ordered item (ex 2) to a lower ordered item (ex 8).  In this case, we need to move everything down, starting
	//		where we drug to (nRow).  Then move our item from it's current row (nFromRow) to it's new value (nRow).  
	//
	//	Finally, we need to set m_bManualChange to true, and then call EnsureLineOrder to save changes (it will not
	//		do any sorting if m_bManualChange is true).

	if(nFromRow == -1 || nRow == -1 || nRow == nFromRow)
		return;

	if(IsDlgButtonChecked(IDC_SEARCH_VIEW))
		//Don't do any moving if the search view is checked - it won't save anyways, so don't confuse them.
		return;

	m_pSurgeryItems->SetRedraw(VARIANT_FALSE);

	IRowSettingsPtr pRow;
	pRow = m_pSurgeryItems->GetRow(nFromRow);

	//TES 2003-12-10: I am redoing all the logic so that a.) if you drag to row -1, nothing happens, and b.) when
	//you drag a row, it ends up visually in the spot where you dragged it, meaning that when you drag down, it ends
	//up below the row you dragged it onto, rather than above.
	
		//1)  Moving up in the list
		if(nRow < nFromRow) {
	//		pRow->PutValue(sicLineOrder, long(-1));	//set a temp value
			IncrementListOrder(nRow);	//increment all ids
			pRow->PutValue(sicLineOrder, long(nRow));	//move it to the new position
		}
		//2)  Moving down in the list (if they were equal, we would have returned by now).
		else {
			IncrementListOrder(nRow+1);	//move everything down
			pRow->PutValue(sicLineOrder, long(nRow+2));	//move the row to where it wants to be
			//EnsureLineOrder handles filling in the gap
		}
//	}

	m_bManualChange = true;
	m_pSurgeryItems->PutAllowSort(FALSE);
	EnsureLineOrder();
	
	// Set the selection to the index of the row we just dropped on.  The row we dropped on now has a 
	// different index, but the row we dragged FROM has now been repositioned into the spot the row we 
	// dropped on WAS taking, or nRow.  For a smooth UI we want the selection to follow the dragged row.
	m_pSurgeryItems->PutCurSel(nRow);

	m_pSurgeryItems->SetRedraw(VARIANT_TRUE);

}

void CMainSurgeries::OnColumnClickingSurgeryItems(short nCol, BOOL FAR* bAllowSort) 
{
	if(!IsDlgButtonChecked(IDC_SEARCH_VIEW) && m_bManualChange)
		*bAllowSort = FALSE;
}

void CMainSurgeries::OnBtnAdvSurgeryEdit() 
{
	CAdvSurgeryItemsDlg dlg(this);
	dlg.DoModal();

	Refresh();
}

void CMainSurgeries::OnSearchVew() 
{
	if(IsDlgButtonChecked(IDC_SEARCH_VIEW)) {
		//they are going into search view.  Enable the ability to sort by any column.
		m_pSurgeryItems->PutAllowSort(TRUE);
	}
	else{
		//they are going back to normal view.  Disable the sorting abilities, and
		//re-order everything as it should be.
		m_pSurgeryItems->PutAllowSort(FALSE);
		Refresh();
	}
}

void CMainSurgeries::OnSurgerySaveAs() 
{
	
	try {
		//first make sure that there is a surgery selected
		if (m_pSurgeryNames->CurSel == -1) {
			MessageBox("Please select a surgery from the drop down");
			return;
		}


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


			if(ExistsInTable("SurgeriesT", "Name = '%s'", _Q(strNewName))){
				// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
				// which is now its own dialog and data structure
				MessageBox("A surgery exists with this name.  Please select a different name for this surgery.", "Nextech", MB_OK);
				return;
			}
			if(strNewName.GetLength() > 255){
				MessageBox("A surgery's name can have no more than 255 characters.");
				return;
			}

			//get the ID of the surgery
			long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 0));

			long nNewSurgID = NewNumber("SurgeriesT", "ID");

			//copy everything from that surgery into one with the new name
			// (j.jones 2010-01-06 09:42) - PLID 15997 - supported package information
			ExecuteParamSql("INSERT INTO SurgeriesT (ID, Name, Description, IsPackage, PackageType, PackageTotalAmount, PackageTotalCount) "
				"SELECT {INT}, {STRING}, Description, IsPackage, PackageType, PackageTotalAmount, PackageTotalCount "
				"FROM SurgeriesT WHERE ID = {INT}", nNewSurgID, strNewName, nSurgeryID);

			_RecordsetPtr rsSurgLineItems = CreateParamRecordset("SELECT ID FROM SurgeryDetailsT WHERE SurgeryID = {INT}", nSurgeryID);

			// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
			// which is now its own dialog and data structure
			while (!rsSurgLineItems->eof) {

				long nSurgDetailID = NewNumber("SurgeryDetailsT", "ID");
				long nOldSurgDetailID = AdoFldLong(rsSurgLineItems, "ID");

				// (j.gruber 2007-05-14 12:36) - PLID 25173 - add applicable fields for discount category
				// (j.gruber 2009-03-19 11:32) - PLID 33361 - take out discount fields
				ExecuteSql("INSERT INTO SurgeryDetailsT (ID, SurgeryID, ServiceID, Amount, Quantity, PayToPractice, LineOrder, ProviderID) "
					" SELECT %li, %li, ServiceID, Amount, Quantity, PayToPractice, LineOrder, ProviderID  FROM  "
					"  SurgeryDetailsT WHERE ID = %li ", nSurgDetailID, nNewSurgID, nOldSurgDetailID);

				// (j.gruber 2009-03-19 11:32) - PLID 33361 - copy the discounts also
				//(e.lally 2010-01-21) PLID 37019 - We were using the wrong ID column to copy from. Use the SurgeryDetailID
				ExecuteParamSql("INSERT INTO SurgeryDetailDiscountsT(SurgeryDetailID, PercentOff, Discount, DiscountCategoryID, CustomDiscountDesc) "
					" SELECT {INT}, PercentOff, Discount, DiscountCategoryID, CustomDiscountDesc FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = {INT}", nSurgDetailID, nOldSurgDetailID);

				rsSurgLineItems->MoveNext();
			}

			/*
			ExecuteSql("INSERT INTO SurgeryProvidersT (SurgeryID, ProviderID) "
				" SELECT %li, ProviderID FROM SurgeryProvidersT WHERE SurgeryID = %li", 
				nNewSurgID, nSurgeryID);
			*/

			//input the new surgery into the surgery list
			IRowSettingsPtr pRow = m_pSurgeryNames->GetRow(-1);

			pRow->PutValue(0, (long)nNewSurgID);
			pRow->PutValue(1, _variant_t(strNewName));
			long nIndex = m_pSurgeryNames->AddRow(pRow);

			// (j.jones 2010-01-06 09:48) - PLID 36763 - no change to the filters are needed here since a copy of a
			// surgery/package will have the same package status, and therefore apply to whatever the current filter is

			//fire the selection event
			m_pSurgeryNames->SetSelByColumn(0, (long)nNewSurgID);
			UpdateView();
		}
		
		
		}NxCatchAll("Error Copying Surgery");

	
}

void CMainSurgeries::RefreshButtons()
{
	if(m_pSurgeryNames->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_SURGERY_SAVE_AS)->EnableWindow(FALSE);
		// (j.jones 2010-01-05 15:02) - PLID 15997 - disable and hide package info
		m_checkPackage.SetCheck(FALSE);
		DisplayPackageInfo();
		GetDlgItem(IDC_SURGERY_PACKAGE_CHECK)->EnableWindow(FALSE);
		m_pCPTCodes->PutEnabled(FALSE);
		m_pProducts->PutEnabled(FALSE);
		// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
		GetDlgItem(IDC_EDIT_DEF_ANESTH_MINUTES)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEF_FACILITY_MINUTES)->EnableWindow(FALSE);
		m_nxtDefAnesthStart->PutEnabled(FALSE);	
		m_nxtDefAnesthEnd->PutEnabled(FALSE);
		m_nxtDefFacilityStart->PutEnabled(FALSE);
		m_nxtDefFacilityEnd->PutEnabled(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RENAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_SURGERY_SAVE_AS)->EnableWindow(TRUE);
		// (j.jones 2010-01-05 15:02) - PLID 15997 - enable and display package info
		DisplayPackageInfo();
		GetDlgItem(IDC_SURGERY_PACKAGE_CHECK)->EnableWindow(TRUE);
		m_pCPTCodes->PutEnabled(TRUE);
		m_pProducts->PutEnabled(TRUE);
		// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
		GetDlgItem(IDC_EDIT_DEF_ANESTH_MINUTES)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_DEF_FACILITY_MINUTES)->EnableWindow(TRUE);
		m_nxtDefAnesthStart->PutEnabled(TRUE);
		m_nxtDefAnesthEnd->PutEnabled(TRUE);
		m_nxtDefFacilityStart->PutEnabled(TRUE);
		m_nxtDefFacilityEnd->PutEnabled(TRUE);
	}
} 

void CMainSurgeries::OnLButtonUpSurgeryItems(long nRow, short nCol, long x, long y, long nFlags) 
{
	 // (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	if (nCol == sicProcDescription /*&& VarLong(m_pSurgeryItems->GetValue(nRow, sicItemType)) != sitPersonnel*/) {

		_RecordsetPtr rsID = CreateParamRecordset("SELECT ServiceID FROM SurgeryDetailsT WHERE ID = {INT}", VarLong(m_pSurgeryItems->GetValue(nRow, 0))); 
		if (!rsID->eof) {
			//pop up the dialog based on the link they click on
			long nCPTID = AdoFldLong(rsID, "ServiceID");

			CProcedureDescriptionDlg dlg(nCPTID, this);

			dlg.DoModal();
		}
	}
	
}

BOOL CMainSurgeries::CheckAllowAddAnesthesiaFacilityCharge(long nServiceID)
{
	//check and see if nServiceID is an anesthesia code or facility code, and already exists in the charge list
	_RecordsetPtr rs = CreateRecordset("SELECT Anesthesia, FacilityFee, Name, Code FROM ServiceT "
		"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
		"WHERE ServiceT.ID = %li AND ((Anesthesia = 1 AND UseAnesthesiaBilling = 1) OR (FacilityFee = 1 AND UseFacilityBilling = 1))", nServiceID);
	if(!rs->eof) {
		//they are adding an anesthesia or facility fee, so see if any matching charge types exist

		BOOL bAnesthesia = AdoFldBool(rs, "Anesthesia",FALSE);
		BOOL bFacilityFee = AdoFldBool(rs, "FacilityFee",FALSE);

		BOOL bWarned = FALSE;

		for(int i=0; i<m_pSurgeryItems->GetRowCount() && !bWarned;i++) {
			long nID = VarLong(m_pSurgeryItems->GetValue(i, sicDetailID), -1);
			if(bAnesthesia && ReturnsRecords("SELECT ID FROM ServiceT WHERE Anesthesia = 1 AND UseAnesthesiaBilling = 1 AND ID IN (SELECT ServiceID FROM SurgeryDetailsT WHERE ID = %li)",nID)) {

				//there is an anesthesia code

				bWarned = TRUE;
				CString str;
				str.Format("You are trying to add Service Code '%s - %s' to the list, which is an Anesthesia Billing charge.\n"
					"However, there is already an Anesthesia Billing charge on this Surgery. There should never be more than one Anesthesia Billing charge on a Surgery.\n\n"
					"If you continue to add this charge, the Anesthesia setup will calculate the same value for each Anesthesia Billing charge in the list.\n\n"
					"Are you sure you wish to add this charge?",
					AdoFldString(rs, "Code",""),
					AdoFldString(rs, "Name",""));

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
			else if(bFacilityFee && ReturnsRecords("SELECT ID FROM ServiceT WHERE FacilityFee = 1 AND UseFacilityBilling = 1 AND ID IN (SELECT ServiceID FROM SurgeryDetailsT WHERE ID = %li)",nID)) {

				//there is a facility code

				bWarned = TRUE;
				CString str;
				str.Format("You are trying to add Service Code '%s - %s' to the list, which is a Facility Billing charge.\n"
					"However, there is already a Facility Billing charge on this Surgery. There should never be more than one Facility Billing charge on a Surgery.\n\n"
					"If you continue to add this charge, the Facility Fee setup will calculate the same value for each Facility Billing charge in the list.\n\n"
					"Are you sure you wish to add this charge?",
					AdoFldString(rs, "Code",""),
					AdoFldString(rs, "Name",""));

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					return FALSE;
				}
			}
		}
	}
	rs->Close();

	return TRUE;
}

LRESULT CMainSurgeries::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::CPTCodeT:
			case NetUtils::Products:
			case NetUtils::Providers:
			case NetUtils::SurgeriesT:
			case NetUtils::Coordinators:
			case NetUtils::ContactsT: {
				try {
					UpdateView();
				} NxCatchAll("Error in CMainSurgeries::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CMainSurgeries::OnTableChanged");

	return 0;
}

// (j.gruber 2007-05-14 16:53) - PLID 25173 - adding discount categories
void CMainSurgeries::OnLeftClickSurgeryItems(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//see what column it is on
		// (j.gruber 2009-03-19 11:36) - PLID 33361 - take out discount categories
		/*if (nCol == sicHasDiscountCategory) {

			IRowSettingsPtr pRow;

			pRow = m_pSurgeryItems->GetRow(nRow);

			if (pRow) {

				
				
				//pop up the discount category selection mechanism
				long nDiscountCategoryID, nSurgeryID;
				CString strCustomDesc;
				BOOL bSetIconToUsed = FALSE;

				nSurgeryID = pRow->GetValue(sicDetailID);
				nDiscountCategoryID = VarLong(m_pSurgeryItems->GetValue(nRow, sicDiscountCategoryID), -3);
				strCustomDesc = VarString(m_pSurgeryItems->GetValue(nRow, sicCustomDiscountDesc), "");

				_variant_t varNull;
				varNull.vt = VT_NULL;				
				
				CDiscountCategorySelectDlg dlg(nDiscountCategoryID, strCustomDesc, -1, FALSE);
				
				long nResult = dlg.DoModal();

				if (nResult == IDOK) {

					nDiscountCategoryID = dlg.m_nDiscountCatID;
					strCustomDesc = dlg.m_strCustomDescription;

					if (nDiscountCategoryID == -1) {

							//that means we have a custom description
							ExecuteSql("UPDATE SurgeryDetailsT SET DiscountCategoryID = -1, CustomDiscountDesc = '%s' WHERE ID = %li",
								_Q(strCustomDesc), nSurgeryID);

							//now update the row in the datalist
							pRow->PutValue(sicDiscountCategoryID, nDiscountCategoryID);
							pRow->PutValue(sicCustomDiscountDesc, _variant_t(strCustomDesc));
							

							bSetIconToUsed = TRUE;

						
					}
					else if (nDiscountCategoryID == -3) {
						
						//that means they aren't doing anything with it
						ExecuteSql("UPDATE SurgeryDetailsT SET DiscountCategoryID = NULL, CustomDiscountDesc = NULL WHERE ID = %li",
								nSurgeryID);

						//now update the row in the datalist
						pRow->PutValue(sicDiscountCategoryID, varNull);
						pRow->PutValue(sicCustomDiscountDesc, varNull);
						

						bSetIconToUsed = FALSE;
					}
					else if (nDiscountCategoryID == -2) {
						//its a coupon

						//they shouldn't be able to pick a coupon
						ASSERT(FALSE);
					}
					else {

						ExecuteSql("UPDATE SurgeryDetailsT SET DiscountCategoryID = %li, CustomDiscountDesc = NULL WHERE ID = %li",
							nDiscountCategoryID, nSurgeryID);

						//now update the row in the datalist
						pRow->PutValue(sicDiscountCategoryID, nDiscountCategoryID);
						pRow->PutValue(sicCustomDiscountDesc, varNull);
						
						bSetIconToUsed = TRUE;
					}

					
					if (bSetIconToUsed) {
						//set it to the green one
						extern CPracticeApp theApp;
						HICON hCat = theApp.LoadIcon(IDI_DISCOUNT_CAT_USED);
						pRow->PutValue((short)sicHasDiscountCategory,(long)hCat);
							
					}
					else {
						//set it to be not set
						extern CPracticeApp theApp;
						HICON hCat = theApp.LoadIcon(IDI_DISCOUNT_CATEGORY);
						pRow->PutValue((short)sicHasDiscountCategory,(long)hCat);
					}
				}
			}
			else {
				ThrowNxException("Error in CMainSurgeries::OnLeftClickSurgeryItems, Could not get row");
			}
		}*/

		// (j.gruber 2009-03-19 11:37) - PLID 33361 - add total discount
		if (nCol == sicTotalDiscountAmt) {

			IRowSettingsPtr pRow = m_pSurgeryItems->GetRow(nRow);

			// (j.jones 2008-07-01 10:53) - PLID 30578 - changed to use ItemType
			// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
			// which is now its own dialog and data structure
			/*
			if(VarLong(pRow->GetValue(sicItemType)) == sitPersonnel) {
				//they can't edit a personnel item
				return;
			}
			*/
			
			if (pRow) {

				long nDetailID = VarLong(pRow->GetValue(sicDetailID));
				double dblQuantity = VarDouble(pRow->GetValue(sicQuantity), 1.0);
				COleCurrency cyLineTotal = VarCurrency(pRow->GetValue(sicAmount), COleCurrency(0,0));

				DiscountList *pList = new DiscountList;
				_RecordsetPtr rsDiscounts = CreateParamRecordset("SELECT * FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = {INT}", nDetailID);
				while (! rsDiscounts->eof) {

					//load the discount list
					stDiscount Disc;
					Disc.ID = rsDiscounts->Fields->Item["ID"]->Value;
					Disc.PercentOff = rsDiscounts->Fields->Item["PercentOff"]->Value;
					Disc.Discount = rsDiscounts->Fields->Item["Discount"]->Value;
					Disc.DiscountCategoryID = rsDiscounts->Fields->Item["DiscountCategoryID"]->Value;
					Disc.CustomDiscountDescription = rsDiscounts->Fields->Item["CustomDiscountDesc"]->Value;

					pList->aryDiscounts.Add(Disc);

					rsDiscounts->MoveNext();
				}

				//now open the discount screen
				// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts

				// (j.gruber 2009-04-22 10:54) - PLID 34042 - added charge description
				CString strDescription = VarString(pRow->GetValue(sicName), "");
				// (j.jones 2011-01-21 14:40) - PLID 42156 - this function takes in a charge ID, but we don't
				// need one here since this is not a bill
				CChargeDiscountDlg dlg(this, -2, pList, FALSE, FALSE, FALSE, cyLineTotal, COleCurrency(0,0), dblQuantity, strDescription);

				long nResult = dlg.DoModal();

				if (nResult == IDOK) {
					
					//save the discounts and recalculate the discount field
					CString strSqlBatch = BeginSqlBatch();

					//first, we'll need to get rid of all the existing discounts for this item
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = %li ", nDetailID);

					//now add our new records
					long nTotalPercentOff = 0;
					COleCurrency cyTotalDiscount;

					for (int i = 0; i < pList->aryDiscounts.GetCount(); i++) {

						stDiscount Disc;
						Disc = pList->aryDiscounts.GetAt(i);

						long nPercentOff;
						COleCurrency cyDiscountAmt;
						CString strDiscountCategoryID, strCustomDiscountDesc;

						if (Disc.PercentOff.vt == VT_I4) {
							nPercentOff = VarLong(Disc.PercentOff);
						}
						else {
							nPercentOff = 0;
						}

						if (Disc.Discount.vt == VT_CY) {
							cyDiscountAmt = VarCurrency(Disc.Discount);
						}
						else {
							cyDiscountAmt = COleCurrency(0,0);
						}

						if (Disc.DiscountCategoryID.vt == VT_I4) {				
							
							if (VarLong(Disc.DiscountCategoryID.lVal) == -2) {
								if (Disc.CustomDiscountDescription.vt == VT_BSTR) {
									strCustomDiscountDesc = VarString(Disc.CustomDiscountDescription);
								}
								else {
									strCustomDiscountDesc = "";
								}
							}
							else {
								strDiscountCategoryID.Format("%li", VarLong(Disc.DiscountCategoryID));
							}
						}
						else {
							strDiscountCategoryID = "NULL";
						}

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO SurgeryDetailDiscountsT (SurgeryDetailID, PercentOff, Discount, DiscountCategoryID, CustomDiscountDesc) "
							" VALUES (%li, %li, CONVERT(money, '%s'), %s, '%s') ",
							nDetailID, nPercentOff, FormatCurrencyForSql(cyDiscountAmt), strDiscountCategoryID, strCustomDiscountDesc);
						
						nTotalPercentOff += nPercentOff;
						cyTotalDiscount += cyDiscountAmt;
					}

					//execute the batch
					ExecuteSqlBatch(strSqlBatch);

					//now update the total discount amt 
					COleCurrency cyDiscountTotal = CalculateAmtQuantity(cyLineTotal, dblQuantity);
					if (nTotalPercentOff > 0) {
						cyDiscountTotal = CalculateAmtQuantity(cyDiscountTotal, (double)nTotalPercentOff/100);
					}
					else {
						cyDiscountTotal = COleCurrency(0,0);
					}

					cyDiscountTotal += cyTotalDiscount;

					pRow->PutValue(sicTotalDiscountAmt, _variant_t(cyDiscountTotal));
				
				}
				//now clear the list
				pList->aryDiscounts.RemoveAll();
				
				if (pList) {
					delete pList;
				}

				// (j.jones 2009-06-18 09:02) - PLID 34656 - ensure the totals at the bottom are updated
				UpdateTotals(TRUE);
			}
		}
	}NxCatchAll("Error In OnLeftClickSurgeryItems");
	
}

// (j.gruber 2009-03-19 12:51) - PLID 33361 - update total discount amount
void CMainSurgeries::UpdateTotalDiscountAmt(long nRow) 
{
	try {
		IRowSettingsPtr pRow = m_pSurgeryItems->GetRow(nRow);
		if (pRow) {

			long nDetailID = pRow->GetValue(sicDetailID);

			_RecordsetPtr rsDiscounts = CreateParamRecordset("SELECT Sum(PercentOff) as TotalPercentOff, Sum(Discount) as TotalDiscount FROM SurgeryDetailDiscountsT WHERE SurgeryDetailID = {INT}", nDetailID);

			if (! rsDiscounts->eof) {

				long nPercentOff = AdoFldLong(rsDiscounts, "TotalPercentOff", 0);
				COleCurrency cyTotalDiscount = AdoFldCurrency(rsDiscounts, "TotalDiscount", COleCurrency(0,0));

				COleCurrency cyLineAmount = VarCurrency(pRow->GetValue(sicAmount), COleCurrency(0,0));
				double dblQuantity = VarDouble(pRow->GetValue(sicQuantity), 1.0);

				COleCurrency cyDiscountTotal = CalculateAmtQuantity(cyLineAmount, dblQuantity);
				if (nPercentOff > 0) {
					cyDiscountTotal = CalculateAmtQuantity(cyDiscountTotal, (double)nPercentOff/100);
				}
				else {
					cyDiscountTotal = COleCurrency(0,0);
				}
				cyDiscountTotal += cyTotalDiscount;

				pRow->PutValue(sicTotalDiscountAmt, _variant_t(cyDiscountTotal));
			}
		}
	}NxCatchAll("Error in CMainSurgeries::UpdateTotalDiscountAmt");
}

// (j.jones 2010-01-05 13:52) - PLID 15997 - added package info
void CMainSurgeries::OnSurgeryPackageCheck()
{
	try {

		if(m_pSurgeryNames->CurSel == -1) {
			RefreshButtons();
			return;
		}

		DisplayPackageInfo();

		BOOL bIsPackage = m_checkPackage.GetCheck();

		if(bIsPackage) {
			//if the package was just checked, re-calculate the total cost
			UpdateTotals(TRUE);
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));
		CString strName = VarString(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel, 1));
		ExecuteParamSql("UPDATE SurgeriesT SET IsPackage = {INT} WHERE ID = {INT}", bIsPackage ? 1 : 0, nSurgeryID);

		//audit this
		long nAuditID = BeginNewAuditEvent();
		CString strOld, strNew;
		strOld.Format("%s: %s", strName, !bIsPackage ? "Package" : "Non-Package");
		strNew.Format("%s", bIsPackage ? "Package" : "Non-Package");
		if(nAuditID != -1) {
			AuditEvent(-1, "", nAuditID, aeiSurgeryPackage, nSurgeryID, strOld, strNew, aepMedium, aetChanged);
		}

		// Network code
		m_SrgyChecker.Refresh();

		// (j.jones 2010-01-06 09:31) - PLID 36763 - if we are filtered in such a way
		// that the current surgery/package should no longer display, change the filter
		// to show all surgeries/packages
		if((bIsPackage && m_radioShowSurgeries.GetCheck())
			|| (!bIsPackage && m_radioShowPackages.GetCheck())) {

			m_radioShowPackages.SetCheck(FALSE);
			m_radioShowSurgeries.SetCheck(FALSE);
			m_radioShowAll.SetCheck(TRUE);

			//refilter and requery the surgery list by
			//tricking it into thinking network code changed
			m_SrgyChecker.m_changed = true;

			Refresh();
		}

	}NxCatchAll("Error in CMainSurgeries::OnSurgeryPackageCheck");
}

void CMainSurgeries::OnRadioSurgeryRepeatPackage()
{
	try {

		// (j.jones 2010-03-17 10:54) - PLID 15997 - due to NxButton failures (see PLID 33858)
		// these options are not grouped, so we have to forcibly change their values
		m_radioRepeatPackage.SetCheck(TRUE);
		m_radioMultiUsePackage.SetCheck(FALSE);

		if(m_pSurgeryNames->CurSel == -1) {
			RefreshButtons();
			return;
		}

		DisplayPackageInfo();

		if(!m_checkPackage.GetCheck()) {
			//do nothing, this should be impossible
			return;
		}

		BOOL bIsMultiUse = m_radioMultiUsePackage.GetCheck();

		//in all cases, recalculate the total
		UpdateTotals(TRUE);

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));
		ExecuteParamSql("UPDATE SurgeriesT SET PackageType = {INT} WHERE ID = {INT}", bIsMultiUse ? 2 : 1, nSurgeryID);

		// Network code
		m_SrgyChecker.Refresh();

	}NxCatchAll("Error in CMainSurgeries::OnRadioSurgeryRepeatPackage");
}

void CMainSurgeries::OnRadioSurgeryMultiusePackage()
{
	try {

		// (j.jones 2010-03-17 10:54) - PLID 15997 - due to NxButton failures (see PLID 33858)
		// these options are not grouped, so we have to forcibly change their values
		m_radioRepeatPackage.SetCheck(FALSE);
		m_radioMultiUsePackage.SetCheck(TRUE);

		if(m_pSurgeryNames->CurSel == -1) {
			RefreshButtons();
			return;
		}

		DisplayPackageInfo();

		if(!m_checkPackage.GetCheck()) {
			//do nothing, this should be impossible
			return;
		}

		BOOL bIsMultiUse = m_radioMultiUsePackage.GetCheck();

		//in all cases, recalculate the total
		UpdateTotals(TRUE);

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));
		ExecuteParamSql("UPDATE SurgeriesT SET PackageType = {INT} WHERE ID = {INT}", bIsMultiUse ? 2 : 1, nSurgeryID);

		// Network code
		m_SrgyChecker.Refresh();

	}NxCatchAll("Error in CMainSurgeries::OnRadioSurgeryMultiusePackage");
}

// (j.jones 2010-01-05 14:59) - PLID 15997 - added package info
void CMainSurgeries::DisplayPackageInfo()
{
	try {

		//show/hide the appropriate fields based on the currently checked values
		BOOL bIsPackage = m_checkPackage.GetCheck();

		if(m_pSurgeryNames->CurSel == -1) {
			bIsPackage = FALSE;
		}

		BOOL bIsRepeatable = m_radioRepeatPackage.GetCheck();
		GetDlgItem(IDC_RADIO_SURGERY_REPEAT_PACKAGE)->ShowWindow(bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_RADIO_SURGERY_MULTIUSE_PACKAGE)->ShowWindow(bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SURGERY_PACKAGE_TOTAL_COST)->ShowWindow(bIsPackage ? SW_SHOW : SW_HIDE);		
		GetDlgItem(IDC_SURGERY_PACKAGE_TOTAL_COST_LABEL)->ShowWindow(bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SURGERY_PACKAGE_TOTAL_COUNT)->ShowWindow(bIsPackage && bIsRepeatable ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_SURGERY_PACKAGE_TOTAL_COUNT_LABEL)->ShowWindow(bIsPackage && bIsRepeatable ? SW_SHOW : SW_HIDE);

		//the totals at the bottom need hidden if this is a package
		GetDlgItem(IDC_PRACTICE_TOTAL_LABEL)->ShowWindow(!bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_PRACTICE_TOTAL)->ShowWindow(!bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_OUTSIDE_TOTAL_LABEL)->ShowWindow(!bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_OUTSIDE_TOTAL)->ShowWindow(!bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_TOTAL_LABEL)->ShowWindow(!bIsPackage ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC_TOTAL)->ShowWindow(!bIsPackage ? SW_SHOW : SW_HIDE);
		Invalidate();

	}NxCatchAll("Error in CMainSurgeries::DisplayPackageInfo");
}

void CMainSurgeries::OnKillfocusSurgeryPackageTotalCost()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bPackageCostChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		CString strAmount;
		GetDlgItemText(IDC_SURGERY_PACKAGE_TOTAL_COST, strAmount);
		
		COleCurrency cy;
		if(!cy.ParseCurrency(strAmount) || cy < COleCurrency(0,0)) {
			MessageBox("Please enter a valid package cost that is not less than zero.");
			//UpdateTotals will replace the package cost with the practice total, and save to data
			UpdateTotals(TRUE);
			m_bPackageCostChanged = FALSE;
			return;
		}

		SetDlgItemText(IDC_SURGERY_PACKAGE_TOTAL_COST, FormatCurrencyForInterface(cy));

		ExecuteParamSql("UPDATE SurgeriesT SET PackageTotalAmount = Convert(money, {STRING}) WHERE ID = {INT}",
			FormatCurrencyForSql(cy), nSurgeryID);

		m_bPackageCostChanged = FALSE;

		// Network code
		m_SrgyChecker.Refresh();

	}NxCatchAll("Error in CMainSurgeries::OnKillfocusSurgeryPackageTotalCost");
}

void CMainSurgeries::OnKillfocusSurgeryPackageTotalCount()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bPackageCountChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		long nCount = GetDlgItemInt(IDC_SURGERY_PACKAGE_TOTAL_COUNT);
		
		COleCurrency cy;
		if(nCount < 1) {
			MessageBox("Please enter a package count greater than zero.");
			nCount = 1;
		}
		else if(nCount > 25000) { //completely arbitrary upper limit
			MessageBox("Please enter a package count no more than 25000");
			nCount = 1;
		}

		SetDlgItemInt(IDC_SURGERY_PACKAGE_TOTAL_COUNT, nCount);

		ExecuteParamSql("UPDATE SurgeriesT SET PackageTotalCount = {INT} WHERE ID = {INT}", nCount, nSurgeryID);

		m_bPackageCountChanged = FALSE;

		//update the cost
		UpdateTotals(TRUE);

		// Network code
		m_SrgyChecker.Refresh();

	}NxCatchAll("Error in CMainSurgeries::OnKillfocusSurgeryPackageTotalCount");
}

// (j.jones 2010-01-06 09:00) - PLID 36763 - added package/surgery filters
void CMainSurgeries::OnRadioSurgeryFilterChanged()
{
	try {

		//refilter and requery the surgery list by
		//tricking it into thinking network code changed
		m_SrgyChecker.m_changed = true;

		Refresh();

	}NxCatchAll("Error in CMainSurgeries::OnRadioSurgeryFilterChanged");
}

// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
void CMainSurgeries::OnKillfocusEditDefAnesthMinutes()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bAnesthMinutesChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		long nTotalMinutes = GetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES);

		if(nTotalMinutes > 1440) {
			AfxMessageBox("You cannot have more than 1440 anesthesia minutes (24 hours).");
			nTotalMinutes = 1440;
			SetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES, nTotalMinutes);
		}

		_variant_t varMinutes = (long)nTotalMinutes;

		if(nTotalMinutes == 0) {
			//treat zero as NULL
			varMinutes = g_cvarNull;
			SetDlgItemText(IDC_EDIT_DEF_ANESTH_MINUTES, "");
		}

		ExecuteParamSql("UPDATE SurgeriesT SET AnesthMinutes = {VT_I4} WHERE ID = {INT}", varMinutes, nSurgeryID);

		if(nTotalMinutes > 0) {
			//see if this matches our times
			COleDateTime dtStart, dtEnd;
			dtStart.SetStatus(COleDateTime::invalid);
			dtEnd.SetStatus(COleDateTime::invalid);

			if(m_nxtDefAnesthStart->GetStatus() == 1) {
				dtStart = m_nxtDefAnesthStart->GetDateTime();
			}
			if(m_nxtDefAnesthEnd->GetStatus() == 1) {
				dtEnd = m_nxtDefAnesthEnd->GetDateTime();
			}
			if(dtStart.GetStatus() != COleDateTime::invalid && dtEnd.GetStatus() != COleDateTime::invalid
				&& dtStart < dtEnd) {
				COleDateTimeSpan dtSpan = dtEnd - dtStart;
				long nCalcMinutes = (long)dtSpan.GetTotalMinutes();
				//don't warn if the calculated minutes are not valid
				if(nCalcMinutes > 0 && nCalcMinutes != nTotalMinutes) {
					CString strWarning;
					strWarning.Format("You entered %li minutes for the default anesthesia time, but your entered times span %li minutes. "
						"Please ensure that these times are accurate.", nTotalMinutes, nCalcMinutes);
					AfxMessageBox(strWarning);
				}
			}
		}

		m_bAnesthMinutesChanged = FALSE;

		m_SrgyChecker.Refresh();

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnKillFocusAnesthDefStartTime()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bAnesthStartTimeChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		COleDateTime dtStart;
		dtStart.SetStatus(COleDateTime::invalid);

		if(m_nxtDefAnesthStart->GetStatus() == 3) {
			//blank, that's okay
		}

		if(m_nxtDefAnesthStart->GetStatus() == 2) {
			MessageBox("You have entered an invalid time.");
			m_nxtDefAnesthStart->Clear();
		}

		if(m_nxtDefAnesthStart->GetStatus() == 1) {
			dtStart = m_nxtDefAnesthStart->GetDateTime();
			if(dtStart.GetStatus() == COleDateTime::invalid) {
				MessageBox("You have entered an invalid time.");
				m_nxtDefAnesthStart->Clear();
			}
		}

		_variant_t varStartTime = g_cvarNull;
		if(dtStart.GetStatus() != COleDateTime::invalid) {
			varStartTime = _variant_t(dtStart, VT_DATE);
		}

		ExecuteParamSql("UPDATE SurgeriesT SET AnesthStartTime = {VT_DATE} WHERE ID = {INT}", varStartTime, nSurgeryID);

		//check the end time, do we have one?
		COleDateTime dtEnd;
		dtEnd.SetStatus(COleDateTime::invalid);

		if(m_nxtDefAnesthEnd->GetStatus() == 1) {
			dtEnd = m_nxtDefAnesthEnd->GetDateTime();
		}
		if(dtStart.GetStatus() != COleDateTime::invalid && dtEnd.GetStatus() != COleDateTime::invalid) {
			//we have valid times

			//do the times mismatch?
			if(dtEnd <= dtStart) {
				AfxMessageBox("The anesthesia start time you entered is not earlier than your anesthesia end time. Please ensure that these times are corrected.");
				//let them continue so they can edit either the start or the end time accordingly
			}
			else {
				COleDateTimeSpan dtSpan = dtEnd - dtStart;
				long nMinutes = (long)dtSpan.GetTotalMinutes();
				long nExistingMinutes = GetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES);
				if(nMinutes != nExistingMinutes) {
					//the minutes have changed
					_variant_t varMinutes = (long)nMinutes;
					if(nMinutes > 0) {
						SetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES, nMinutes);						
					}
					else {
						SetDlgItemText(IDC_EDIT_DEF_ANESTH_MINUTES, "");
						varMinutes = g_cvarNull;
					}

					ExecuteParamSql("UPDATE SurgeriesT SET AnesthMinutes = {VT_I4} WHERE ID = {INT}", varMinutes, nSurgeryID);
				}
			}
		}

		m_bAnesthStartTimeChanged = FALSE;

		m_SrgyChecker.Refresh();

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnKillFocusAnesthDefEndTime()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bAnesthEndTimeChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		COleDateTime dtEnd;
		dtEnd.SetStatus(COleDateTime::invalid);

		if(m_nxtDefAnesthEnd->GetStatus() == 3) {
			//blank, that's okay
		}

		if(m_nxtDefAnesthEnd->GetStatus() == 2) {
			MessageBox("You have entered an invalid time.");
			m_nxtDefAnesthEnd->Clear();
		}

		if(m_nxtDefAnesthEnd->GetStatus() == 1) {
			dtEnd = m_nxtDefAnesthEnd->GetDateTime();
			if(dtEnd.GetStatus() == COleDateTime::invalid) {
				MessageBox("You have entered an invalid time.");
				m_nxtDefAnesthEnd->Clear();
			}
		}

		_variant_t varEndTime = g_cvarNull;
		if(dtEnd.GetStatus() != COleDateTime::invalid) {
			varEndTime = _variant_t(dtEnd, VT_DATE);
		}

		ExecuteParamSql("UPDATE SurgeriesT SET AnesthEndTime = {VT_DATE} WHERE ID = {INT}", varEndTime, nSurgeryID);

		//check the start time, do we have one?
		COleDateTime dtStart;
		dtStart.SetStatus(COleDateTime::invalid);

		if(m_nxtDefAnesthStart->GetStatus() == 1) {
			dtStart = m_nxtDefAnesthStart->GetDateTime();
		}
		if(dtStart.GetStatus() != COleDateTime::invalid && dtEnd.GetStatus() != COleDateTime::invalid) {
			//we have valid times

			//do the times mismatch?
			if(dtEnd <= dtStart) {
				AfxMessageBox("The anesthesia end time you entered is not later than your anesthesia start time. Please ensure that these times are corrected.");
				//let them continue so they can edit either the start or the end time accordingly
			}
			else {
				COleDateTimeSpan dtSpan = dtEnd - dtStart;
				long nMinutes = (long)dtSpan.GetTotalMinutes();
				long nExistingMinutes = GetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES);
				if(nMinutes != nExistingMinutes) {
					//the minutes have changed
					_variant_t varMinutes = (long)nMinutes;
					if(nMinutes > 0) {
						SetDlgItemInt(IDC_EDIT_DEF_ANESTH_MINUTES, nMinutes);						
					}
					else {
						SetDlgItemText(IDC_EDIT_DEF_ANESTH_MINUTES, "");
						varMinutes = g_cvarNull;
					}

					ExecuteParamSql("UPDATE SurgeriesT SET AnesthMinutes = {VT_I4} WHERE ID = {INT}", varMinutes, nSurgeryID);
				}
			}
		}

		m_bAnesthEndTimeChanged = FALSE;

		m_SrgyChecker.Refresh();

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnKillfocusEditDefFacilityMinutes()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bFacilityMinutesChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		long nTotalMinutes = GetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES);

		if(nTotalMinutes > 1440) {
			AfxMessageBox("You cannot have more than 1440 facility minutes (24 hours).");
			nTotalMinutes = 1440;
			SetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES, nTotalMinutes);
		}

		_variant_t varMinutes = (long)nTotalMinutes;

		if(nTotalMinutes == 0) {
			//treat zero as NULL
			varMinutes = g_cvarNull;
			SetDlgItemText(IDC_EDIT_DEF_FACILITY_MINUTES, "");
		}

		ExecuteParamSql("UPDATE SurgeriesT SET FacilityMinutes = {VT_I4} WHERE ID = {INT}", varMinutes, nSurgeryID);

		if(nTotalMinutes > 0) {
			//see if this matches our times
			COleDateTime dtStart, dtEnd;
			dtStart.SetStatus(COleDateTime::invalid);
			dtEnd.SetStatus(COleDateTime::invalid);

			if(m_nxtDefFacilityStart->GetStatus() == 1) {
				dtStart = m_nxtDefFacilityStart->GetDateTime();
			}
			if(m_nxtDefFacilityEnd->GetStatus() == 1) {
				dtEnd = m_nxtDefFacilityEnd->GetDateTime();
			}
			if(dtStart.GetStatus() != COleDateTime::invalid && dtEnd.GetStatus() != COleDateTime::invalid
				&& dtStart < dtEnd) {
				COleDateTimeSpan dtSpan = dtEnd - dtStart;
				long nCalcMinutes = (long)dtSpan.GetTotalMinutes();
				//don't warn if the calculated minutes are not valid
				if(nCalcMinutes > 0 && nCalcMinutes != nTotalMinutes) {
					CString strWarning;
					strWarning.Format("You entered %li minutes for the default facility time, but your entered times span %li minutes. "
						"Please ensure that these times are accurate.", nTotalMinutes, nCalcMinutes);
					AfxMessageBox(strWarning);
				}
			}
		}

		m_bFacilityMinutesChanged = FALSE;

		m_SrgyChecker.Refresh();

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnKillFocusFacilityDefStartTime()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bFacilityStartTimeChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		COleDateTime dtStart;
		dtStart.SetStatus(COleDateTime::invalid);

		if(m_nxtDefFacilityStart->GetStatus() == 3) {
			//blank, that's okay
		}

		if(m_nxtDefFacilityStart->GetStatus() == 2) {
			MessageBox("You have entered an invalid time.");
			m_nxtDefFacilityStart->Clear();
		}

		if(m_nxtDefFacilityStart->GetStatus() == 1) {
			dtStart = m_nxtDefFacilityStart->GetDateTime();
			if(dtStart.GetStatus() == COleDateTime::invalid) {
				MessageBox("You have entered an invalid time.");
				m_nxtDefFacilityStart->Clear();
			}
		}

		_variant_t varStartTime = g_cvarNull;
		if(dtStart.GetStatus() != COleDateTime::invalid) {
			varStartTime = _variant_t(dtStart, VT_DATE);
		}

		ExecuteParamSql("UPDATE SurgeriesT SET FacilityStartTime = {VT_DATE} WHERE ID = {INT}", varStartTime, nSurgeryID);

		//check the end time, do we have one?
		COleDateTime dtEnd;
		dtEnd.SetStatus(COleDateTime::invalid);

		if(m_nxtDefFacilityEnd->GetStatus() == 1) {
			dtEnd = m_nxtDefFacilityEnd->GetDateTime();
		}
		if(dtStart.GetStatus() != COleDateTime::invalid && dtEnd.GetStatus() != COleDateTime::invalid) {
			//we have valid times

			//do the times mismatch?
			if(dtEnd <= dtStart) {
				AfxMessageBox("The facility start time you entered is not earlier than your facility end time. Please ensure that these times are corrected.");
				//let them continue so they can edit either the start or the end time accordingly
			}
			else {
				COleDateTimeSpan dtSpan = dtEnd - dtStart;
				long nMinutes = (long)dtSpan.GetTotalMinutes();
				long nExistingMinutes = GetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES);
				if(nMinutes != nExistingMinutes) {
					//the minutes have changed
					_variant_t varMinutes = (long)nMinutes;
					if(nMinutes > 0) {
						SetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES, nMinutes);						
					}
					else {
						SetDlgItemText(IDC_EDIT_DEF_FACILITY_MINUTES, "");
						varMinutes = g_cvarNull;
					}

					ExecuteParamSql("UPDATE SurgeriesT SET FacilityMinutes = {VT_I4} WHERE ID = {INT}", varMinutes, nSurgeryID);
				}
			}
		}

		m_bFacilityStartTimeChanged = FALSE;

		m_SrgyChecker.Refresh();

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnKillFocusFacilityDefEndTime()
{
	try {

		if(m_pSurgeryNames->CurSel == -1 || !m_bFacilityEndTimeChanged) {
			return;
		}

		long nSurgeryID = VarLong(m_pSurgeryNames->GetValue(m_pSurgeryNames->CurSel,0));

		COleDateTime dtEnd;
		dtEnd.SetStatus(COleDateTime::invalid);

		if(m_nxtDefFacilityEnd->GetStatus() == 3) {
			//blank, that's okay
		}

		if(m_nxtDefFacilityEnd->GetStatus() == 2) {
			MessageBox("You have entered an invalid time.");
			m_nxtDefFacilityEnd->Clear();
		}

		if(m_nxtDefFacilityEnd->GetStatus() == 1) {
			dtEnd = m_nxtDefFacilityEnd->GetDateTime();
			if(dtEnd.GetStatus() == COleDateTime::invalid) {
				MessageBox("You have entered an invalid time.");
				m_nxtDefFacilityEnd->Clear();
			}
		}

		_variant_t varEndTime = g_cvarNull;
		if(dtEnd.GetStatus() != COleDateTime::invalid) {
			varEndTime = _variant_t(dtEnd, VT_DATE);
		}

		ExecuteParamSql("UPDATE SurgeriesT SET FacilityEndTime = {VT_DATE} WHERE ID = {INT}", varEndTime, nSurgeryID);

		//check the start time, do we have one?
		COleDateTime dtStart;
		dtStart.SetStatus(COleDateTime::invalid);

		if(m_nxtDefFacilityStart->GetStatus() == 1) {
			dtStart = m_nxtDefFacilityStart->GetDateTime();
		}
		if(dtStart.GetStatus() != COleDateTime::invalid && dtEnd.GetStatus() != COleDateTime::invalid) {
			//we have valid times

			//do the times mismatch?
			if(dtEnd <= dtStart) {
				AfxMessageBox("The facility end time you entered is not later than your facility start time. Please ensure that these times are corrected.");
				//let them continue so they can edit either the start or the end time accordingly
			}
			else {
				COleDateTimeSpan dtSpan = dtEnd - dtStart;
				long nMinutes = (long)dtSpan.GetTotalMinutes();
				long nExistingMinutes = GetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES);
				if(nMinutes != nExistingMinutes) {
					//the minutes have changed
					_variant_t varMinutes = (long)nMinutes;
					if(nMinutes > 0) {
						SetDlgItemInt(IDC_EDIT_DEF_FACILITY_MINUTES, nMinutes);						
					}
					else {
						SetDlgItemText(IDC_EDIT_DEF_FACILITY_MINUTES, "");
						varMinutes = g_cvarNull;
					}

					ExecuteParamSql("UPDATE SurgeriesT SET FacilityMinutes = {VT_I4} WHERE ID = {INT}", varMinutes, nSurgeryID);
				}
			}
		}

		m_bFacilityEndTimeChanged = FALSE;

		m_SrgyChecker.Refresh();

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnChangeEditDefAnesthMinutes()
{
	try {

		m_bAnesthMinutesChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnChangedAnesthDefStartTime()
{
	try {

		m_bAnesthStartTimeChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnChangedAnesthDefEndTime()
{
	try {

		m_bAnesthEndTimeChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnChangeEditDefFacilityMinutes()
{
	try {

		m_bFacilityMinutesChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnChangedFacilityDefStartTime()
{
	try {

		m_bFacilityStartTimeChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CMainSurgeries::OnChangedFacilityDefEndTime()
{
	try {

		m_bFacilityEndTimeChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}
