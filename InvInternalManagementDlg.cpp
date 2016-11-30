// r.wilson (2011-4-8) - PLID 43188 - CREATED By ..
// InvInternalManagementDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvInternalManagementDlg.h"  // r.wilson (2011-4-8) - PLID 43188
#include "InventoryRc.h" 
#include "InvInternalManagementRequestDlg.h"
#include "GlobalUtils.h"
#include "InvInternalReturnChecklistDlg.h"
#include "InvInternalSignEquipmentDlg.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CInvInternalManagementDlg dialog

enum ManagementStatusListColumn {
	mstReqID = 0,
	mstID,
	mstParentProduct,
	//mstIsRequestable,
	mstItem,
	mstCategory,
	mstCategoryID,
	mstRequestFor,		// (j.armen 2011-06-02 16:36) - PLID 43259 - Added a request for column
	mstRequestBy,
	mstCheckoutDate,
	mstReturnDate,
	mstReturnDateDisplay,	//// (j.fouts 2012-05-08 09:55) - PLID 50210 - Added this to hide ReturnDate from user and show a collumn that can be "Indefinite"
	mstStatus,
	mstSigned,			// (j.fouts 2012-05-18 09:26) - PLID 50300 - Added a signed feild
	//mstItemManager,	// (d.thompson 2012-01-23) - PLID 47714 - Removed, we now support multiple owners and it's not in this display
	mstColor,
};

enum OwnerComboColumn {
	occID = 0,
	occName,
};

enum StatusComboColumn {
	stccID = 0,
	stccName,
};

enum CategoryComboColumn {
	ctccID = 0,
	ctccName,
};

enum LocationComboColumn {
	lccID = 0,
	lccName,
};

#define IDM_APPROVE_REQUEST		32785
#define IDM_DENY_REQUEST		32786
#define IDM_EDIT_REQUEST		32787
#define IDM_CHECKOUT_ITEM		32789
#define IDM_CHECKIN_ITEM		32790
#define IDM_DELETE_REQUEST      32791
#define IDM_SIGN_REQUEST		32792
#define COLOR_BLACK		RGB(0, 0, 0)
#define COLOR_RED		RGB(255, 0, 0)
#define COLOR_BLUE		RGB(0, 0, 255)
#define COLOR_YELLOW	RGB(218, 165, 32) 
#define COLOR_GREEN		RGB(34, 139, 34)


IMPLEMENT_DYNAMIC(CInvInternalManagementDlg, CNxDialog)

CInvInternalManagementDlg::CInvInternalManagementDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvInternalManagementDlg::IDD, pParent)
{

}

CInvInternalManagementDlg::~CInvInternalManagementDlg()
{
}

void CInvInternalManagementDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_INV_MANAGEMENT_SHOW_MY_REQUESTS_CHECKBOX, m_MyRequests);
	DDX_Control(pDX, IDC_INV_MANAGEMENT_REMEMBER_FILTERS_CHECKBOX, m_Filters); // (j.luckoski 2012-03-29 10:48) - PLID 49279
	DDX_Control(pDX, IDC_SHOW_HISTORY_CHECKBOX, m_History); // (j.luckoski 2012-04-02 12:25) - PLID 48195
	DDX_Control(pDX, IDC_HIDE_INDEFINITE, m_Indefinite); // (j.fouts 2012-05-08 08:43) - PLID 50210
	DDX_Control(pDX, IDC_INV_MANAGEMENT_REQUEST_ITEM_BUTTON, m_CreateRequest); 
}


BEGIN_MESSAGE_MAP(CInvInternalManagementDlg, CNxDialog)
	ON_BN_CLICKED(IDC_INV_MANAGEMENT_REQUEST_ITEM_BUTTON, &CInvInternalManagementDlg::OnBnClickedBtnRequest)
	ON_BN_CLICKED(IDC_INV_MANAGEMENT_SHOW_MY_REQUESTS_CHECKBOX, &CInvInternalManagementDlg::OnChangeMyRequests)
	ON_BN_CLICKED(IDC_INV_MANAGEMENT_REMEMBER_FILTERS_CHECKBOX, &CInvInternalManagementDlg::OnRememberFilters) // (j.luckoski 2012-03-29 10:47) - PLID 49279 
	ON_BN_CLICKED(IDC_SHOW_HISTORY_CHECKBOX, &CInvInternalManagementDlg::OnShowHistory) // (j.luckoski 2012-04-02 12:26) - PLID 48195
	ON_COMMAND(IDM_APPROVE_REQUEST, OnApproveRequest)
	ON_COMMAND(IDM_DENY_REQUEST, OnDenyRequest)
	ON_COMMAND(IDM_EDIT_REQUEST, OnEditRequest)
	ON_COMMAND(IDM_CHECKOUT_ITEM, OnCheckOutItem)
	ON_COMMAND(IDM_CHECKIN_ITEM, OnCheckItemIn)
	ON_COMMAND(IDM_DELETE_REQUEST, OnDeleteRequest)
	ON_COMMAND(IDM_SIGN_REQUEST, OnSignRequest)
	ON_BN_CLICKED(IDC_HIDE_INDEFINITE, &CInvInternalManagementDlg::OnBnClickedHideIndefinite)
END_MESSAGE_MAP()

BOOL CInvInternalManagementDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	
// (d.singleton 2011-04-12 17:40) - PLID 43259
	try {

		

		bLoadedParentCategoryID = FALSE; //PLID 43188 r.wilson 6/10/2011
		//had to make the button a nxbutton,  set to have add new icon
		m_CreateRequest.AutoSet(NXB_NEW);

		m_StatusList = BindNxDataList2Ctrl(IDC_INVENTORY_STATUS_LIST, false);
		m_OwnerCombo = BindNxDataList2Ctrl(IDC_INV_MANAGEMENT_OWNER_COMBO, true);
		m_StatusCombo = BindNxDataList2Ctrl(IDC_INV_MANAGEMENT_STATUS_COMBO, false);
		m_CategoryCombo = BindNxDataList2Ctrl(IDC_INV_MANAGEMENT_CATEGORY_COMBO, true);
		m_LocationCombo = BindNxDataList2Ctrl(IDC_INV_MANAGEMENT_LOCATION_COMBO, true);

		
		// (j.luckoski 2012-03-29 10:36) - PLID 49279 - Chance properties for the filters on this page.
		
		g_propManager.CachePropertiesInBulk("InvInternalManagementDlg", propNumber,
			"Username = '%s' AND Name IN ("
			"	'InvManagerOwnerFilter', "
			"	'InvManagerStatusFilter', "	
			"	'InvManagerCategoryFilter', " 
			"	'InvManagerLocationFilter', "
			"	'InvManagerShowMyRequestsFilter', "	
			"	'InvManagerRememberFilter',	"
			"	'InvManagerShowReturns', "
			// (j.fouts 2012-05-08 08:50) - PLID 50210 - Added an Indefinite filter
			"	'InvManagerHideIndefinite' "
			")"
			, _Q(GetCurrentUserName()));

		// (j.luckoski 2012-03-29 10:37) - PLID 49279 - Check if the remember filter button was checked
		// if not then restore all user filters to default -1. Which stands for nothing.
		 long nRememberFilter = GetRemotePropertyInt("InvManagerRememberFilter", 0,0, GetCurrentUserName());
		 long nChecked = 1;
		 if(nRememberFilter == 0) {
			SetRemotePropertyInt("InvManagerOwnerFilter",-1,0,GetCurrentUserName());
			SetRemotePropertyInt("InvManagerStatusFilter",-1,0,GetCurrentUserName());
			SetRemotePropertyInt("InvManagerCategoryFilter",-1,0,GetCurrentUserName());
			SetRemotePropertyInt("InvManagerLocationFilter",-1,0,GetCurrentUserName());
			SetRemotePropertyInt("InvManagerShowMyRequestsFilter",-1,0,GetCurrentUserName());
			SetRemotePropertyInt("InvManagerShowReturns", -1, 0, GetCurrentUserName());
			// (j.fouts 2012-05-08 08:50) - PLID 50210 - Added an Indefinite filter
			SetRemotePropertyInt("InvManagerHideIndefinite", -1, 0, GetCurrentUserName());
			nChecked = 0;
		 }

		 // (j.luckoski 2012-03-29 10:39) - PLID 49279 - Assign properties to variables to change combo boxes.
		 long nOwnerFilter = GetRemotePropertyInt("InvManagerOwnerFilter", -1,0, GetCurrentUserName(), true);
		 long nStatusFilter = GetRemotePropertyInt("InvManagerStatusFilter", -1,0, GetCurrentUserName(), true);
		 long nCategoryFilter = GetRemotePropertyInt("InvManagerCategoryFilter", -1,0, GetCurrentUserName(), true);
		 long nLocationFilter = GetRemotePropertyInt("InvManagerLocationFilter", -1,0, GetCurrentUserName(), true);
		 long nShowMyRequestsFilter = GetRemotePropertyInt("InvManagerShowMyRequestsFilter", -1,0, GetCurrentUserName(), true);
		 long nShowReturns = GetRemotePropertyInt("InvManagerShowReturns", -1, 0, GetCurrentUserName(), true);
		 long nHideIndefinite = GetRemotePropertyInt("InvManagerHideIndefinite", -1, 0, GetCurrentUserName(), true);
		 
		

		//r.wilson 5/23/2011 PLID 43470
		//Find the categoryId for all internal inventory items
		_RecordsetPtr reResults = CreateParamRecordset("SELECT ID from CategoriesT where Name = 'NexTech Tracked Inventory'");
		
		//PLID 43188 r.wilson 6/10/2011
		if(!reResults->eof)
		{
			nNEXTECH_PARENT_CATEGORY = AdoFldLong(reResults, "ID");
			bLoadedParentCategoryID = TRUE;
		}
		else
		{
			m_StatusList->Enabled = false;
			m_OwnerCombo->Enabled = false;
			m_StatusCombo->Enabled = false;
			m_CategoryCombo->Enabled = false;
			m_LocationCombo->Enabled = false;
			bLoadedParentCategoryID = FALSE;
			AfxMessageBox("Error:\n Failed to load the parent category ID. There needs to be an entry in the CategoriesT table's [Name] column with the value \'NexTech Tracked Inventory\' in order for this tab to work correctly. Please add this value and then reload the inventory module.");
		}
		
		if(bLoadedParentCategoryID)
		{
			//Add the All Owners row
			IRowSettingsPtr pRow = m_OwnerCombo->GetNewRow();

			pRow->PutValue(occID, (long)cnAllOwners);
			pRow->PutValue(occName, _bstr_t(" <All Owners> "));
			m_OwnerCombo->AddRowSorted(pRow, NULL);

			// (j.luckoski 2012-03-29 10:42) - PLID 49279 - Set Owner dropdown if property given
			if(nOwnerFilter > -1) {
				m_OwnerCombo->SetSelByColumn(occID, nOwnerFilter);
			} else {
				m_OwnerCombo->SetSelByColumn(occID, (long)cnAllOwners);
			}

			// (j.luckoski 2012-03-29 10:42) - PLID 49279 - Set Checkboxes if property given
			if(nChecked == 1) {
				CheckDlgButton(IDC_INV_MANAGEMENT_REMEMBER_FILTERS_CHECKBOX, BST_CHECKED);
			}

			if(nShowMyRequestsFilter == 1) {
				CheckDlgButton(IDC_INV_MANAGEMENT_SHOW_MY_REQUESTS_CHECKBOX, BST_CHECKED);
			}

			if(nShowReturns == 1) {
				CheckDlgButton(IDC_SHOW_HISTORY_CHECKBOX, BST_CHECKED);
			}

			if(nHideIndefinite == 1)
			{
				CheckDlgButton(IDC_HIDE_INDEFINITE, BST_CHECKED);
			}
			
			//build the status combo
			pRow = m_StatusCombo->GetNewRow();
			pRow->PutValue(stccID, (long)mstAll);
			pRow->PutValue(stccName, _bstr_t(" <All Statuses> "));
			m_StatusCombo->AddRowAtEnd(pRow, NULL);
			pRow = m_StatusCombo->GetNewRow();
			pRow->PutValue(stccID, (long)mstDenied);
			pRow->PutValue(stccName, _bstr_t("Denied Request"));
			pRow->PutBackColor(COLOR_RED);
			m_StatusCombo->AddRowAtEnd(pRow, NULL);
			pRow = m_StatusCombo->GetNewRow();
			pRow->PutValue(stccID, (long)mstPending);
			pRow->PutValue(stccName, _bstr_t("Pending Request"));
			pRow->PutBackColor(COLOR_YELLOW);
			m_StatusCombo->AddRowAtEnd(pRow, NULL);
			pRow = m_StatusCombo->GetNewRow();
			pRow->PutValue(stccID, (long)mstApproved);
			pRow->PutValue(stccName, _bstr_t("Approved Request"));
			pRow->PutBackColor(COLOR_GREEN);
			m_StatusCombo->AddRowAtEnd(pRow, NULL);

			// (j.luckoski 2012-03-29 10:43) - PLID 49279 - Set Status dropdown if property given.
			if(nStatusFilter > -1) {
				m_StatusCombo->SetSelByColumn(stccID, nStatusFilter);
			} else {
				m_StatusCombo->SetSelByColumn(stccID, (long)mstAll);
			}

			//set the category drop down where clause
			CString strCatWhereClause;
			strCatWhereClause.Format("Active = 1 and CategoriesT.Parent = %li", nNEXTECH_PARENT_CATEGORY);
			m_CategoryCombo->PutWhereClause(_bstr_t(strCatWhereClause));
			m_CategoryCombo->Requery();
			//add the "All Catagories" row
			pRow = m_CategoryCombo->GetNewRow();
			pRow->PutValue(ctccID, (long)cnAllCategories);
			pRow->PutValue(ctccName, _bstr_t(" <All Categories> "));
			m_CategoryCombo->AddRowSorted(pRow, NULL);


			// (j.luckoski 2012-03-29 10:43) - PLID 49279 - Set Category dropdown if property given.
			if(nCategoryFilter > -1) {
				m_CategoryCombo->SetSelByColumn(ctccID, nCategoryFilter);
			} else {
				m_CategoryCombo->SetSelByColumn(ctccID, (long)cnAllCategories);
			}

			//add the "All Locations" row
			pRow = m_LocationCombo->GetNewRow();
			pRow->PutValue(lccID, (long)cnAllLocations);
			pRow->PutValue(lccName, _bstr_t(" <All Locations> "));
			m_LocationCombo->AddRowSorted(pRow, NULL);
			
			// (j.luckoski 2012-03-29 10:43) - PLID 49279 - Set Location dropdown if property given.
			if(nLocationFilter > -1) {
				m_LocationCombo->SetSelByColumn(lccID, nLocationFilter);
			} else {
				m_LocationCombo->SetSelByColumn(lccID, (long)cnAllLocations);
			} 


			//(e.lally 2012-01-05) PLID 46492 - There is no LocationID field. Disabling this and adding a new item to support it.
			m_LocationCombo->Enabled = VARIANT_FALSE;

			RequeryMainDataList();
			m_StatusList->DragVisible = TRUE;
			bDragAllowed = TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;

}

void CInvInternalManagementDlg::RequeryMainDataList()
{
	CString strFromClause;

	// (j.armen 2011-06-02 16:34) - PLID 43470 - Refactored and attempted to simplify/speed up from clause to work better with filters
	CString strWhere;// = RefilterStatusList(FALSE);

	// (j.luckoski 2012-04-02 12:26) - PLID 48195 - Created strOld to hold where clause to find old returned checkouts
	CString strOld;
	if(m_History.GetCheck()) {
		strOld = "OR (ActualEnd IS NOT NULL)";
	}
	else {
		strOld = "";
	}
		strWhere = BuildWhere();


		// (j.armen 2011-06-02 16:34) - PLID 43470 - New From Clause
		// (j.armen 2011-07-02 16:34) - PLID 43470 - Added more information to From Clause
		//(e.lally 2012-01-05) PLID 46492 - Added comments to identify pieces of the query. Renamed a couple ambiguous names.
		// (d.thompson 2012-01-23) - PLID 47714 - Removed Owners from this clause, they are now contained in their own table and multiple are supported
		// (j.fouts 2012-05-07 17:09) - PLID 50210 - Added ProductRequestsTRequestToDisplay
		// (j.fouts 2012-05-18 09:03) - PLID 50300 - Added Signed
		strFromClause.Format(
			"( \r\n"
			//Current item status
			"	SELECT \r\n"
			"		NULL AS ProductRequestID, \r\n"
			"		ProductT.ID AS ProductID, \r\n"
			"		NULL AS MyParentID, \r\n"
			"		ServiceT.Name AS Name, \r\n"
			"		CategoriesT.Name AS Category, \r\n"
			"		CategoriesT.ID AS CategoryID, \r\n"
			"		NULL AS RequestUsersTUserName, \r\n"
			"		Recepient AS RecepientTUserName, \r\n"
			"		CheckOutTime AS ProductRequestsTRequestFrom, \r\n"
			"		RequestTo AS ProductRequestsTRequestTo, \r\n"
			"		CASE WHEN Indefinite = 1 THEN 'Indefinite' ELSE CONVERT(VARCHAR, RequestTo, 101) END AS ProductRequestsTRequestToDisplay, \r\n"
			//CheckoutStatus means status of the product itself here
			"		Case \r\n"
			"			WHEN CheckoutStatusQ.StatusEnum = 0 THEN 'In Stock' \r\n"
			"			WHEN CheckoutStatusQ.StatusEnum = 1 THEN 'Assigned' \r\n"
			"			WHEN CheckoutStatusQ.StatusEnum = 2 THEN 'Overdue' \r\n"
			"			ELSE 'In Stock' \r\n"
			"		END AS Status, \r\n"
			"		ProductRequestableT.IsRequestable AS IsRequestable, \r\n"
			"		NULL AS Signed, "
			"		CASE \r\n"
			"			WHEN CheckoutStatusQ.StatusEnum = 0 THEN %li \r\n"
			"			WHEN CheckoutStatusQ.StatusEnum = 1 THEN %li \r\n"
			"			WHEN CheckoutStatusQ.StatusEnum = 2 THEN %li \r\n"
			"			ELSE %li \r\n"
			"		END AS Color \r\n"
			"	FROM ProductT \r\n"
			"	INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID \r\n"
			"	INNER JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID \r\n"
			"	INNER JOIN ProductRequestableT ON ProductT.ID = ProductRequestableT.ProductID \r\n"
			"	LEFT JOIN \r\n"
			"	( \r\n"
			"		SELECT ProductID, \r\n"
			" 			MAX \r\n"
			"			( \r\n"
							//(e.lally 2012-01-05) PLID 46492 - Altered cases slightly
							// (j.fouts 2012-05-14 10:54) - PLID 50210 - Edited cases for Indefinite never being overdue
			"				CASE \r\n"
			"					WHEN (ActualStart IS NULL OR ActualEnd IS NOT NULL) THEN 0 /*No one has it*/ \r\n"
			"					WHEN (dbo.AsDateNoTime(ActualStart) <= dbo.AsDateNoTime(GetDate()) AND ActualEnd IS NULL AND (RequestTo >= dbo.AsDateNoTime(GetDate()) OR Indefinite = 1) ) THEN 1 /*Still Checked out, not due yet*/  \r\n"
			"					WHEN (ActualStart IS NOT NULL AND ActualEnd IS NULL AND dbo.AsDateNoTime(GetDate()) > RequestTo) THEN 2 /*Someone was supposed to return it in the past, but hasn't */ \r\n"
			"					ELSE -1 \r\n" //Unknown?
			"				END \r\n"
			"			) AS StatusEnum \r\n"
			"		FROM  (SELECT * FROM ProductRequestsT WHERE (dbo.AsDateNoTime(GetDate()) <= RequestTo) AND (ActualEnd IS NULL) OR (ActualStart IS NOT NULL AND ActualEnd IS NULL)) AS CurrentProductRequestsT \r\n"
			" \r\n"
			"		GROUP BY ProductID \r\n"
			"	) CheckoutStatusQ ON ProductT.ID = CheckoutStatusQ.ProductID \r\n"
			"	LEFT JOIN \r\n"
			"	( \r\n"
			"		SELECT \r\n"
			"			CurrentProductRequestsT.ProductID, \r\n"
			"			COUNT(*) AS Count \r\n"
			// (j.luckoski 2012-04-02 12:27) - PLID 48195 - Added strOld to next line to show items with no active requests
			"		FROM  (SELECT * FROM ProductRequestsT WHERE (dbo.AsDateNoTime(GetDate()) <= RequestTo) AND (ActualEnd IS NULL) OR (ActualStart IS NOT NULL AND ActualEnd IS NULL) %s) AS CurrentProductRequestsT \r\n"
			"		GROUP BY CurrentProductRequestsT.ProductID \r\n"
			"	) CountProdQ ON ProductT.ID = CountProdQ.ProductID \r\n"
			"	LEFT JOIN \r\n"
			"	( \r\n"
			"		SELECT \r\n"
			"			UsersT.UserName AS Recepient, \r\n"
			"			ProductRequestsT.ActualStart AS CheckOutTime, \r\n"
			"			ProductRequestsT.RequestTo, \r\n"
			"			ProductRequestsT.Indefinite, \r\n"
			"			ProductRequestsT.Signed, \r\n"
			"			ProductRequestsT.ProductID FROM ProductRequestsT  \r\n"
			"		INNER JOIN UsersT ON ProductRequestsT.Recipient = UsersT.PersonID \r\n"
			"		WHERE ActualStart IS NOT NULL AND ActualEnd IS NULL \r\n"
			"	) RecepientQ ON ProductT.ID = RecepientQ.ProductID \r\n"
			"	INNER JOIN \r\n"
			"	( \r\n"
			"		SELECT DISTINCT ServiceT.ID FROM ServiceT \r\n"
			"		INNER JOIN ProductRequestableT ON ServiceT.ID = ProductRequestableT.ProductID \r\n"
			"		INNER JOIN ProductRequestsT ON ProductRequestableT.ProductID = ProductRequestsT.ProductID \r\n"
			"		%s"
			"	) CatQ ON ProductT.ID = CatQ.ID \r\n"
			"	WHERE CountProdQ.[Count] > 0 \r\n"
			"UNION \r\n"
			//Parent line item for requests based on any item in a category
			"	SELECT \r\n"
			"		NULL AS ProductRequestID, \r\n"
			"		(CategoriesT.ID * -1) AS ProductID, \r\n"
			"		NULL AS MyParentID, \r\n"
			"		NULL AS Name, \r\n"
			"		'{ANY} ' + CategoriesT.Name AS Category, \r\n"
			"		CategoriesT.ID AS CategoryID, \r\n"
			"		NULL AS RequestUsersTUserName, \r\n"
			"		NULL AS RecepientTUserName, \r\n"
			"		NULL AS ProductRequestsTRequestFrom, \r\n"
			"		NULL AS ProductRequestsTRequestTo, \r\n"
			"		NULL AS ProductRequestsTRequestToDisplay, \r\n"
			"		NULL AS Status, \r\n"
			"		NULL AS IsRequestable, \r\n"
			"		NULL AS Signed, "
			"		%li AS Color \r\n"
			"	FROM CategoriesT \r\n"
			"	INNER JOIN \r\n"
			"	( \r\n"
			"		SELECT \r\n"
			"			CurrentProductRequestsT.CategoryID, \r\n"
			"			COUNT(*) AS Count \r\n"
			"		FROM  (SELECT * FROM ProductRequestsT WHERE (dbo.AsDateNoTime(GetDate()) <= RequestTo) AND (ActualEnd IS NULL) OR (ActualStart IS NOT NULL AND ActualEnd IS NULL) OR ItemStatus = 4) AS CurrentProductRequestsT \r\n"
			"		GROUP BY CurrentProductRequestsT.CategoryID \r\n"
			"	) CountCatQ ON CategoriesT.ID = CountCatQ.CategoryID \r\n"
			"	INNER JOIN \r\n"
			"	( \r\n"
			"		SELECT DISTINCT CategoriesT.ID FROM CategoriesT \r\n"
			"		INNER JOIN ServiceT ON CategoriesT.ID = ServiceT.Category \r\n"
			"		INNER JOIN ProductRequestableT ON ServiceT.ID = ProductRequestableT.ProductID \r\n"
			"		INNER JOIN ProductRequestsT ON CategoriesT.ID = ProductRequestsT.CategoryID \r\n"
			"		%s"
			"	) CatQ ON CategoriesT.ID = CatQ.ID \r\n"
			"UNION \r\n"
			//Current or future request details for specific products
			"	SELECT \r\n"
			"		CurrentProductRequestsT.ID AS ProductRequestID, \r\n"
			"		NULL AS ProductID, \r\n"
			"		ServiceT.ID AS MyParentID, \r\n"
			"		NULL AS Name, \r\n"
			"		NULL AS Category, \r\n"
			"		ServiceT.Category AS CategoryID, \r\n"
			"		RequestersT.UserName AS RequestUsersTUserName, \r\n"
			"		RecepientT.UserName AS RecepientTUserName, \r\n"
			"		CurrentProductRequestsT.RequestFrom AS ProductRequestsTRequestFrom, \r\n"
			"		CurrentProductRequestsT.RequestTo AS ProductRequestsTRequestTo, \r\n"
			"		CASE WHEN CurrentProductRequestsT.Indefinite = 1 THEN 'Indefinite' ELSE CONVERT(VARCHAR, CurrentProductRequestsT.RequestTo, 101) END AS ProductRequestsTRequestToDisplay, \r\n"
			"		NULL AS Status, \r\n"
			"		NULL AS IsRequestable, \r\n"
			"		CurrentProductRequestsT.Signed AS Signed, "
					//ItemStatus means status of the request here
			"		CASE \r\n"
			"			WHEN ItemStatus = 0 THEN %li \r\n"
			"			WHEN ItemStatus = 1 THEN %li \r\n"
			"			WHEN ItemStatus = 2 THEN %li \r\n"
			"			WHEN ItemStatus = 3 THEN %li \r\n"
			"			Else %li \r\n"
			"		END AS Color \r\n"
			// (j.luckoski 2012-04-02 12:27) - PLID 48195 - Added strOld to next line to show returned requests.
			"	FROM  (SELECT * FROM ProductRequestsT WHERE (dbo.AsDateNoTime(GetDate()) <= RequestTo) AND (ActualEnd IS NULL) OR (ActualStart IS NOT NULL AND ActualEnd IS NULL) %s) AS CurrentProductRequestsT \r\n"
			"	INNER JOIN ProductRequestableT ON CurrentProductRequestsT.ProductID = ProductRequestableT.ProductID \r\n"
			"	INNER JOIN ServiceT ON ProductRequestableT.ProductID = ServiceT.ID \r\n"
			"	INNER JOIN UsersT RequestersT ON CurrentProductRequestsT.RequestedBy = RequestersT.PersonID \r\n"
			"	INNER JOIN UsersT RecepientT ON CurrentProductRequestsT.Recipient = RecepientT.PersonID \r\n"
			"	%s"
			"UNION \r\n"
			//Current or future request details for any item in a given category
			"	SELECT \r\n"
			"		CurrentProductRequestsT.ID AS ProductRequestID, \r\n"
			"		NULL AS ProductID, \r\n"
			"		(CategoriesT.ID * -1) AS MyParentID, \r\n"
			"		NULL AS Name, \r\n"
			"		NULL AS Category, \r\n"
			"		CategoriesT.ID AS CategoryID, \r\n"
			"		Requester.UserName AS RequestUsersTUserName, \r\n"
			"		RecepientT.UserName AS RecepientTUserName, \r\n"
			"		CurrentProductRequestsT.RequestFrom AS ProductRequestsTRequestFrom, \r\n"
			"		CurrentProductRequestsT.RequestTo AS ProductRequestsTRequestTo, \r\n"
			"		CASE WHEN CurrentProductRequestsT.Indefinite = 1 THEN 'Indefinite' ELSE CONVERT(VARCHAR, CurrentProductRequestsT.RequestTo, 101) END AS ProductRequestsTRequestToDisplay, \r\n"
			"		NULL AS Status, \r\n"
			"		NULL AS IsRequestable, \r\n"
			"		CurrentProductRequestsT.Signed AS Signed, "
			"		CASE \r\n"
			"			WHEN ItemStatus = 0 THEN %li \r\n"
			"			Else %li \r\n"
			"		END AS Color \r\n"
			"	FROM  (SELECT * FROM ProductRequestsT WHERE (dbo.AsDateNoTime(GetDate()) <= RequestTo) AND (ActualEnd IS NULL) OR (ActualStart IS NOT NULL AND ActualEnd IS NULL)) AS CurrentProductRequestsT \r\n"
			"	INNER JOIN CategoriesT ON CurrentProductRequestsT.CategoryID = CategoriesT.ID \r\n"
			"	INNER JOIN UsersT Requester ON CurrentProductRequestsT.RequestedBy = Requester.PersonID \r\n"
			"	INNER JOIN UsersT RecepientT ON CurrentProductRequestsT.Recipient = RecepientT.PersonID \r\n"
			"	INNER JOIN \r\n"
			"	( \r\n"
			"		SELECT DISTINCT ProductRequestsT.ID FROM CategoriesT \r\n"
			"		INNER JOIN ServiceT ON CategoriesT.ID = ServiceT.Category \r\n"
			"		INNER JOIN ProductRequestableT ON ServiceT.ID = ProductRequestableT.ProductID \r\n"
			"		INNER JOIN ProductRequestsT ON CategoriesT.ID = ProductRequestsT.CategoryID \r\n"
			"		%s"
			"	) CatQ ON CurrentProductRequestsT.ID = CatQ.ID \r\n"
			") AS MYTableQ", COLOR_BLACK, COLOR_BLUE, COLOR_RED, COLOR_BLACK, strOld, strWhere, COLOR_BLACK, strWhere, COLOR_YELLOW, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_BLACK, strOld, strWhere, COLOR_YELLOW, COLOR_YELLOW , strWhere);
			
	m_StatusList->PutFromClause(_bstr_t(strFromClause));
	m_StatusList->Requery();

			
}

// (d.singleton 2011-04-12 17:40) - PLID 43259 - build the where clause
CString CInvInternalManagementDlg::BuildWhere()
{
	CString strWhere;
	try
	{
		//Filter on Location
		IRowSettingsPtr pRow = NULL;
		//(e.lally 2012-01-05) PLID 46492 - There is no LocationID field. Disabling this and adding a new item to support it.
		/*
		long nLocation;
		pRow = m_LocationCombo->GetCurSel();
		if(pRow != NULL)
		{
			nLocation = VarLong(pRow->GetValue(lccID), cnAllLocations);
			if(nLocation != -1)
			{
				CString strLocation;
				strLocation.Format("LocationID = %li", nLocation);
			
				if(!strWhere.IsEmpty())
				{
					strWhere += " AND ";
				}
				else
				{
					strWhere = "WHERE ";
				}
				strWhere += strLocation;
			}
		}
		*/

		//Filter on Category
		long nCategory;
		pRow = m_CategoryCombo->GetCurSel();
		if(pRow != NULL)
		{
			nCategory = VarLong(pRow->GetValue(ctccID), cnAllCategories);
			if(nCategory != -1)
			{
				CString strCategory;
				strCategory.Format("Category = %li", nCategory);

				if(!strWhere.IsEmpty())
				{
					strWhere += " AND ";
				}
				else
				{
					strWhere = "WHERE ";
				}
				strWhere += strCategory;
			}
		}

		//Filter on Owner
		long nOwner;
		pRow = m_OwnerCombo->GetCurSel();
		if(pRow != NULL)
		{
			nOwner = VarLong(pRow->GetValue(occID), cnAllOwners);
			if(nOwner != -1)
			{
				CString strOwner;
				// (d.thompson 2012-01-23) - PLID 47714 - Support multiple owners
				strOwner.Format("ProductRequestableT.ID IN (SELECT ProductRequestableID FROM ProductRequestableOwnersT WHERE OwnerID = %li)", nOwner);
				if(!strWhere.IsEmpty())
				{
					strWhere += " AND ";
				}
				else
				{
					strWhere = "WHERE ";
				}
				strWhere += strOwner;
			}
		}

		//Filter on Status
		CString strStatus;
		pRow = m_StatusCombo->GetCurSel();
		if(pRow != NULL)
		{
			ManagementStatusType msType = (ManagementStatusType)VarLong(pRow->GetValue(stccID), (long)mstAll);
			if(msType != mstAll)
			{
				strStatus.Format("ItemStatus = %li", msType);

				if(!strWhere.IsEmpty())
				{
					strWhere += " AND ";
				}
				else
				{
					strWhere = "WHERE ";
				}
			}
			strWhere += strStatus;
		}

		//Filter on User checkbox
		if(m_MyRequests.GetCheck())
		{
			CString strUser;
			strUser.Format("Recipient = %li", GetCurrentUserID());

			if(!strWhere.IsEmpty())
			{
				strWhere += " AND ";
			}
			else
			{
				strWhere = "WHERE ";
			}

			strWhere += strUser;
		}

		// (j.luckoski 2012-06-11 11:23) - PLID 48195 - Correct parents showing without children.
		if(m_History.GetCheck())
		{
			if(!strWhere.IsEmpty())
			{
				strWhere += " AND ((ActualEnd IS NOT NULL) OR (ActualEnd IS NULL))";
			}
			else
			{
				strWhere = "WHERE ((ActualEnd IS NOT NULL) OR (ActualEnd IS NULL))";
			}
		} else {
			if(!strWhere.IsEmpty())
			{
				strWhere += " AND (ActualEnd IS NULL)";
			}
			else
			{
				strWhere = "WHERE (ActualEnd IS NULL)";
			}
		}



		// (j.fouts 2012-05-08 09:04) - PLID 50210 - Filter on Indefinite Checkbox
		if(m_Indefinite.GetCheck())
		{
			if(!strWhere.IsEmpty())
			{
				strWhere += " AND Indefinite = 0";
			}
			else
			{
				strWhere = "WHERE Indefinite = 0";
			}
		}

	}NxCatchAll(__FUNCTION__);
	return strWhere;
}


// CInvInternalManagementDlg message handlers
void CInvInternalManagementDlg::OnBnClickedBtnRequest()
{
	try {
		//PLID 43188 r.wilson 6/10/2011
		if(!bLoadedParentCategoryID)
		{
			return;
		}
		CInvInternalManagementRequestDlg dlg(this);
		//(c.copits 2011-04-28) PLID 43243 - Implement request dialog functionality
		dlg.SetRequest(-1);		// Pass in request ID here (-1 for new request)
		dlg.SetCategory(-1);	// Pass in new default category ID here (-1 for no default category)
		if(dlg.DoModal() == IDOK)
		{
			//r.wilson PLID 43353
			RequeryMainDataList();
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvInternalManagementDlg::OnChangeMyRequests()
{
	try {
			if(!bLoadedParentCategoryID)
			{
				//r.wilson -> if the Parent Category couldn't be loaded then this is unusable
				return;
			}
			else
			{

				// (j.luckoski 2012-03-29 11:09) - PLID 49279 - Set properties for the request checkbox
				if(m_MyRequests.GetCheck()) {
					SetRemotePropertyInt("InvManagerShowMyRequestsFilter", 1, 0, GetCurrentUserName()); 
				} else {
					SetRemotePropertyInt("InvManagerShowMyRequestsFilter", 0, 0, GetCurrentUserName());
				}

				RequeryMainDataList();
			}
	}NxCatchAll(__FUNCTION__)
}


BEGIN_EVENTSINK_MAP(CInvInternalManagementDlg, CNxDialog)
	ON_EVENT(CInvInternalManagementDlg, IDC_INV_MANAGEMENT_CATEGORY_COMBO, 16 /* on sel chosen */, OnSelChosenInvManagementCategoryCombo, VTS_DISPATCH)
	ON_EVENT(CInvInternalManagementDlg, IDC_INV_MANAGEMENT_OWNER_COMBO, 16 /* on sel chosen */, OnSelChosenInvManagementOwnerCombo, VTS_DISPATCH)
	ON_EVENT(CInvInternalManagementDlg, IDC_INV_MANAGEMENT_LOCATION_COMBO, 16 /* on sel chosen */, OnSelChosenInvManagementLocationCombo, VTS_DISPATCH)
	ON_EVENT(CInvInternalManagementDlg, IDC_INV_MANAGEMENT_STATUS_COMBO, 16 /* on sel chosen */, OnSelChosenInvManagementStatusCombo, VTS_DISPATCH)
//	ON_EVENT(CInvInternalManagementDlg, IDC_INVENTORY_STATUS_LIST,10, CInvInternalManagementDlg::EditingFinishedStatusDataList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvInternalManagementDlg, IDC_INVENTORY_STATUS_LIST, 7, RButtonUpStatusList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvInternalManagementDlg, IDC_INVENTORY_STATUS_LIST, 14, CInvInternalManagementDlg::DragEndInventoryStatusList, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CInvInternalManagementDlg, IDC_INVENTORY_STATUS_LIST,12 , CInvInternalManagementDlg::FireDragBegin, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CInvInternalManagementDlg, IDC_INVENTORY_STATUS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedStatusList, VTS_I2)
END_EVENTSINK_MAP()

void CInvInternalManagementDlg::OnSelChosenInvManagementCategoryCombo(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
		{
			pRow = m_CategoryCombo->SetSelByColumn(ctccID, (long)cnAllCategories);
		}
			// (j.luckoski 2012-03-29 10:47) - PLID 49279 - Set property in database

		long nCategoryFilter = VarLong(pRow->GetValue(ctccID));
		SetRemotePropertyInt("InvManagerCategoryFilter", nCategoryFilter,0,GetCurrentUserName());

		RequeryMainDataList();

	}NxCatchAll(__FUNCTION__);
}

void CInvInternalManagementDlg::OnSelChosenInvManagementOwnerCombo(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
		{
			pRow = m_OwnerCombo->SetSelByColumn(occID, (long)cnAllOwners);
		}
		// (j.luckoski 2012-03-29 10:47) - PLID 49279 - Set property in database
		long nOwnerFilter = VarLong(pRow->GetValue(occID));
		SetRemotePropertyInt("InvManagerOwnerFilter", nOwnerFilter, 0, GetCurrentUserName());

		RequeryMainDataList();

	}NxCatchAll(__FUNCTION__);
}

void CInvInternalManagementDlg::OnSelChosenInvManagementLocationCombo(LPDISPATCH lpRow)
{
	try{
		//(e.lally 2012-01-05) PLID 46492 - There is no LocationID field. Disabling this and adding a new item to support it.
		/*
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
		{
			pRow = m_LocationCombo->SetSelByColumn(lccID, (long)cnAllLocations);
		}
		// (j.luckoski 2012-03-29 10:47) - PLID 49279 - Set property in database
		long nLocationFilter = pRow->GetValue(lccID);
		SetRemotePropertyInt("InvManagerLocationFilter", nLocationFilter,0, GetCurrentUserName());

		RequeryMainDataList();
		*/

	}NxCatchAll(__FUNCTION__);
}
	
void CInvInternalManagementDlg::OnSelChosenInvManagementStatusCombo(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
		{
			pRow = m_StatusCombo->SetSelByColumn(stccID, (long)mstAll);
		}
		// (j.luckoski 2012-03-29 10:47) - PLID 49279 - Set property in database
		long nStatusFilter = VarLong(pRow->GetValue(stccID));
		SetRemotePropertyInt("InvManagerStatusFilter", nStatusFilter,0, GetCurrentUserName());

		RequeryMainDataList();

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2011-04-22 15:24) - PLID 43353 
void CInvInternalManagementDlg::RButtonUpStatusList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL)
		{
			m_StatusList->CurSel = pRow;

			//only open menu on child row
			if(pRow->GetParentRow() != NULL)
			{
				CMenu mnu;
				mnu.CreatePopupMenu();

				long nIndex = 0;

				//check to see if they have ability to use admin features
				BOOL bAllowAdminFeatures = FALSE;
				//check to see if its an "any" item
				BOOL bIsAny = FALSE;
				// (j.luckoski 2012-04-02 12:28) - PLID 48195 - Determine if the requessts is a return or not.
				// check to see if returned requests
				BOOL bIsReturned = FALSE;
				IRowSettingsPtr pRowParent = pRow->GetParentRow();
				// (d.thompson 2012-01-23) - PLID 47714 - Just moved this up and out of the below if/else, removed a duplicate lookup
				long nReqID = VarLong(pRow->GetValue(mstReqID));
				// (d.thompson 2012-01-23) - PLID 47714 - Pulled ProductID out for re-use
				long nProductID = VarLong(pRowParent->GetValue(mstID));
				// (j.luckoski 2012-03-30 09:35) - PLID 49094 - Pulled out CategoryID to pass to IsAdmin and then created that function which returns
				// a true or false based on if you own a product or are a category owner and also assigns takes the reference to bIsAny and assigns it.
				long nCategoryID = VarLong(pRowParent->GetValue(mstCategoryID));
				bAllowAdminFeatures = IsAdmin(nProductID, nCategoryID, bIsAny);

				

				//check to see if they can edit their own request
				BOOL bAllowEditRequest = FALSE;
				CString strRequestBy = VarString(pRow->GetValue(mstRequestFor));
				if(strRequestBy.CompareNoCase(GetCurrentUserName()) == 0)
				{
					bAllowEditRequest = TRUE;
				}

				//check to see if the request is approved or denied
			    BOOL bIsDenied = FALSE;
				BOOL bIsApproved = FALSE;

				// (j.luckoski 2012-04-02 12:29) - PLID 48195 Added ActualEnd to query to determine when not null
				// (j.fouts 2012-05-14 14:23) - PLID 50210 - Added Indefinite
				// (j.fouts 2012-05-18 10:48) - PLID 50300 - Added Signed and Recipient
				_RecordsetPtr rsResults = CreateParamRecordset("SELECT ItemStatus, RequestFrom, ActualEnd, Indefinite, Signed, Recipient FROM ProductRequestsT WHERE ID = {INT}", nReqID);
				//(e.lally 2012-01-05) PLID 46492 - Assume default value of pending, use enum values
				long nApproved = (long)AdoFldByte(rsResults, "ItemStatus", mstPending);
				COleDateTime dtEndDate = AdoFldDateTime(rsResults, "ActualEnd", g_cdtInvalid);

				BOOL bIndefinite = AdoFldBool(rsResults, "Indefinite");
				BOOL bSigned = AdoFldBool(rsResults, "Signed");

				if(nApproved == mstApproved)
				{
					bIsApproved = TRUE;
				}
				else if(nApproved == mstDenied)
				{
					bIsDenied = TRUE;
				}

				// (j.fouts 2012-05-18 16:13) - PLID 50300 - Check if the current user is the recipient
				BOOL bRecipient =  AdoFldLong(rsResults, "Recipient", -1) == GetCurrentUserID();

				// (j.luckoski 2012-04-02 12:30) - PLID 48195 - If not invalid, null, or a weird 1899 date return true
				if(dtEndDate.m_status == COleDateTime::valid && dtEndDate.m_dt > 0)
				{
					if(dtEndDate > COleDateTime(1900, 1, 1, 0, 0, 0)) {
						bIsReturned = TRUE;
					} 
				}
	
				// (j.luckoski 2012-03-29 16:03) - PLID  49305 - Added in a query to look for any requests for this item because 
				// the current request might not be the only requests.
				_RecordsetPtr rsItemCheckedOut = CreateParamRecordset("SELECT ItemStatus FROM ProductRequestsT WHERE ProductID = {INT} AND ActualStart IS NOT NULL AND ActualEnd IS NULL", nProductID);

				// (j.luckoski 2012-03-29 16:06) - PLID 49305 - Check to see if any requests for this item are checkedout
				BOOL bItemInStock = FALSE;
				if(!rsItemCheckedOut->eof)
				{} else {
					bItemInStock = TRUE;
				}

				// (j.luckoski 2012-03-29 16:24) - PLID 49305 - Check to see if this requests is assigned or not so a user can delete it.	
				BOOL bRequestCheckedOut = FALSE;
				long nCheckOut = (long)AdoFldByte(rsResults, "ItemStatus", mstPending);
				if(nCheckOut == mstCheckedOut) 
				{
					bRequestCheckedOut = TRUE;
				}

				//check to see if todays date is same as the ActualStart date of the request or at least greater than (use same query as above)
				BOOL bDatesMatch = FALSE;
				COleDateTime dtActualStart = AdoFldDateTime(rsResults, "RequestFrom");
				CString strReqDate = dtActualStart.Format("%m/%d/%Y");
				COleDateTime dtTodaysDate = COleDateTime::GetCurrentTime();
				CString strTodaysDate = dtTodaysDate.Format("%m/%d/%Y");
				if(strReqDate.CompareNoCase(strTodaysDate) == 0 || dtTodaysDate > dtActualStart)
				{
					bDatesMatch = TRUE;
				}
				rsResults->Close();

				// (j.luckoski 2012-04-02 12:31) - PLID 48195 - Added the bIsReturned to each line so returned items have no context menu
				//create the pop up menu items and enable/disable depending on above criteria
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | (bAllowAdminFeatures && !bIsApproved && (bItemInStock || !bRequestCheckedOut) && !bIsAny && !bIsReturned ? MF_ENABLED : MF_DISABLED|MF_GRAYED), IDM_APPROVE_REQUEST, "&Approve Request");
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | (bAllowAdminFeatures && !bIsDenied && (bItemInStock || !bRequestCheckedOut) && !bIsAny && !bIsReturned ? MF_ENABLED : MF_DISABLED|MF_GRAYED), IDM_DENY_REQUEST, "&Deny Request");
				// (j.luckoski 2012-03-29 14:14) - PLID 49312 - Allow people to check out an item without approving it first since it is quicker.
				// (j.luckoski 2012-04-23 09:03) - PLID 49305 - Don't allow item to be checked out if the item is any. Only when you specifically own an item.
				// (j.luckoski 2012-06-15 17:31) - PLID 49312 - Placed correct PLID on line where checkout's don't require approval first.
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | (bAllowAdminFeatures && !bIsDenied && bItemInStock && bDatesMatch && !bIsReturned && !bIsAny ? MF_ENABLED : MF_DISABLED|MF_GRAYED), IDM_CHECKOUT_ITEM, "&Check Out Item");
				// (j.luckoski 2012-03-30 09:37) - PLID 49094 - Use bRequestCheckedOut to see if the request is checked out instead of the item as you can't return an item on a 
				// request that is not checked out.
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | (bAllowAdminFeatures && bRequestCheckedOut && !bIsReturned ? MF_ENABLED : MF_DISABLED|MF_GRAYED), IDM_CHECKIN_ITEM, "&Return Item");
				// (j.luckoski 2012-03-29 15:59) - PLID 49305 - Allow admins to edit request after they have been entered.
				// (j.luckoski 2012-03-30 09:38) - PLID 49094 - Use bRequestCheckedOut to see is single request is checkedout.
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | (!bIsReturned && ((bAllowAdminFeatures) || (bAllowEditRequest && !bRequestCheckedOut)) ? MF_ENABLED : MF_DISABLED|MF_GRAYED), IDM_EDIT_REQUEST, "&Edit Request");
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | ((bAllowAdminFeatures || bAllowEditRequest) && !bRequestCheckedOut && !bIsReturned ? MF_ENABLED : MF_DISABLED|MF_GRAYED), IDM_DELETE_REQUEST, "&Delete Request");
				// (j.fouts 2012-05-18 10:06) - PLID 50300 - Added a Sign button
				mnu.InsertMenu(nIndex++, MF_BYPOSITION | ((bAllowAdminFeatures || bRecipient) && (bRequestCheckedOut || bIsReturned) && !bSigned ? MF_ENABLED : MF_DISABLED | MF_GRAYED), IDM_SIGN_REQUEST, "&Sign For Item");


				CRect rc;
				CWnd *pWnd = GetDlgItem(IDC_NEW);
				if (pWnd) 
				{
					pWnd->GetWindowRect(&rc);
					mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
				} 
				else 
				{
					CPoint pt;
					GetCursorPos(&pt);
					mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//update request to be approved
void CInvInternalManagementDlg::OnApproveRequest()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;

		if(pRow)
		{
			//get the requestID
			long nReqID = VarLong(pRow->GetValue(mstReqID));
			long nToProductID = VarLong(pRow->GetParentRow()->GetValue(mstID));
			COleDateTime dtFrom = VarDateTime(pRow->GetValue(mstCheckoutDate));
			COleDateTime dtTo = VarDateTime(pRow->GetValue(mstReturnDate));
			CString strIndefinite = VarString(pRow->GetValue(mstReturnDateDisplay), "");
			BOOL bIndefinite = strIndefinite == "Indefinite";
			
			if(!DoRequestDateTimesConflict(nToProductID,nReqID,dtFrom, dtTo, bIndefinite))
			{
				//update record to be approved
				ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT set ItemStatus = 2 WHERE ID = {INT}", nReqID);
			}
			else
			{
				AfxMessageBox("Cannot fulfill request because the dates overlap.");
			}

			RequeryMainDataList();
		}
	}NxCatchAll(__FUNCTION__);
}

//update status of the request to be denied
void CInvInternalManagementDlg::OnDenyRequest()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;
		
		if(pRow)
		{
			long nReqID = VarLong(pRow->GetValue(mstReqID));

			ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT set ItemStatus = 1 WHERE ID = {INT}", nReqID);
			RequeryMainDataList();
		}
	}NxCatchAll(__FUNCTION__);
}

//update the status of the item to be checked out
void CInvInternalManagementDlg::OnCheckOutItem()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;
		if(pRow)
		{
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if(pParentRow)
			{
				long nParentRowID = VarLong(pParentRow->GetValue(mstID));
				long nChildRowID = VarLong(pRow->GetValue(mstReqID));
				CString strCheckoutUsername = VarString(pRow->GetValue(mstRequestFor));

				// (j.fouts 2012-05-17 17:42) - PLID 50300 - Display the prompt to sign for the equipment
				CInvInternalSignEquipmentDlg dlgSign;
				
				dlgSign.m_bShowSkip = true;
				dlgSign.m_strCheckoutUsername = strCheckoutUsername;

				if(dlgSign.DoModal() == IDOK)
				{
					// (j.fouts 2012-05-14 13:13) - PLID 50299 - Gathering Data for an email
					CString strMailTo = VarString(pRow->GetValue(mstRequestFor), "") + "@nextech.com";
					CString strProductName  = VarString(pParentRow->GetValue(mstItem), "");
					CString strCategoryName = VarString(pParentRow->GetValue(mstCategory), "");
					COleDateTime dtCheckout = VarDateTime(pRow->GetValue(mstCheckoutDate));
					CString strReturnDate = VarString(pRow->GetValue(mstReturnDateDisplay), "");
					COleDateTime dtNow = COleDateTime::GetCurrentTime();
					CString strSubject;
					strSubject.Format("Inventory Checkout Confirmation (%s)", dtNow.Format("%m/%d/%Y"));
					CString strBody;
					CString strSignSkippedWarning = "";
					// (j.fouts 2012-06-11 16:55) - PLID 50299 - If they didn't sign for it warn them in the email
					if(dlgSign.m_bSkipedSign)
					{
						strSignSkippedWarning = "<br><br><font color=\"red\">WARNING: This checkout was not signed. Please visit Internal Inventory and sign for this promptly.</color>";
					}

					_RecordsetPtr rsResults = CreateParamRecordset(
						"SELECT ',' + UserName + '@nextech.com' AS Email "
						"FROM ProductRequestableOwnersT "
						"INNER JOIN UsersT ON UsersT.PersonID = ProductRequestableOwnersT.OwnerID "
						"INNER JOIN ProductRequestableT ON ProductRequestableT.ID = ProductRequestableOwnersT.ProductRequestableID "
						"INNER JOIN ProductRequestsT ON ProductRequestsT.ProductID = ProductRequestableT.ProductID "
						"WHERE ProductRequestsT.ID = {INT}", nChildRowID
						);

					while(!rsResults->eof)
					{
						strMailTo += AdoFldString(rsResults, "Email", "");

						rsResults->MoveNext();
					}

					strBody.Format("<h2>Confirmation of Checkout</h2><h3>Please review and report any problems immediately.</h3><p>You have checked out %s: %s from %s until %s.%s</p>", strCategoryName, strProductName, dtCheckout.Format("%m/%d/%Y"), strReturnDate, strSignSkippedWarning);

					ExecuteParamSql(GetRemoteData(), 
						"UPDATE ProductRequestableT SET CurrentStatus = 1 WHERE ProductID = {INT} "
						"UPDATE ProductRequestsT SET ItemStatus = 3, ActualStart = GetDate() WHERE ID = {INT} "
						// (j.fouts 2012-05-18 12:21) - PLID 50300 - Set the Signed flag
						"UPDATE ProductRequestsT SET Signed = {BOOL} WHERE ID = {INT} "
						// (j.fouts 2012-05-14 13:13) - PLID 50299 - Send the email after someone checks an item out
						"INSERT INTO \r\n "
							"EAEmailT (EmailTo, EmailSendTries, Subject,  Body ) \r\n "
							"VALUES   ({STRING}, '0', {STRING}, {STRING})\r\n ",
							nParentRowID, nChildRowID, !dlgSign.m_bSkipedSign, nChildRowID, strMailTo, strSubject, strBody);

					RequeryMainDataList();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//update the status of the item to be checked in
void CInvInternalManagementDlg::OnCheckItemIn()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;
		if(pRow)
		{
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if(pParentRow)
			{
				long nParentRowID = VarLong(pParentRow->GetValue(mstID));
				long nChildRowID = VarLong(pRow->GetValue(mstReqID));

				// (j.fouts 2012-05-15 12:12) - PLID 50297 - Display a dialog with item details
				CInvInternalReturnChecklistDlg CheckListdlg;
				CheckListdlg.m_nReqID = nChildRowID;
				if(CheckListdlg.DoModal() == IDOK)
				{
					ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT SET ItemStatus = 4,ActualEnd = GetDate() WHERE ID = {INT}", nChildRowID);
					RequeryMainDataList();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}


//load the original request dialog so you can edit your request
void CInvInternalManagementDlg::OnEditRequest()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;
		if(pRow)
		{
			//if the request is already approved we need to warn them it will need to be re-approved once edited
			BOOL bAllowAdminFeatures =FALSE;
			BOOL bIsAny = FALSE;
			long nReqID = VarLong(pRow->GetValue(mstReqID));
			// (j.luckoski 2012-03-30 09:40) - PLID 49094 - AddCategoryID and ProductID in order to pass to IsAdmin
			long nCategoryID = VarLong(pRow->GetParentRow()->GetValue(mstCategoryID));
			long nProductID = VarLong(pRow->GetParentRow()->GetValue(mstID));
			bAllowAdminFeatures = IsAdmin(nProductID, nCategoryID, bIsAny);
			


			_RecordsetPtr rsResults = CreateParamRecordset("SELECT ItemStatus from ProductRequestsT WHERE ID = {INT}", nReqID);
			//(e.lally 2012-01-05) PLID 46492 - Assume default value of pending, use enum values
			long nReqStatus = (long)AdoFldByte(rsResults, "ItemStatus", mstPending);
			// (j.luckoski 2012-03-30 09:42) - PLID 49094 - Check to see if if this is approved and that you are not admin to show this warning
			// and to reset the ItemStatus else just allow them to edit and keep the status
			if(nReqStatus == mstApproved && !bAllowAdminFeatures)
			{
				if(IDYES == MsgBox(MB_YESNO, "If you choose to edit this request you will need to have it re-approved,  do you wish to continue?"))
				{
					CInvInternalManagementRequestDlg dlg(this);
					dlg.SetRequest(nReqID);
					if(dlg.DoModal() == IDOK)
					{
						ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT set ItemStatus = 0 WHERE ID = {INT}", nReqID);
						RequeryMainDataList();
					}
				}
			}
			else
			{
				//the request was denied so no need to warn them it will need to be re-approved
				CInvInternalManagementRequestDlg dlg(this);
				dlg.SetRequest(nReqID);
				if(dlg.DoModal() == IDOK)
				{
					RequeryMainDataList();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//add ability to delete a request
void CInvInternalManagementDlg::OnDeleteRequest()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;
		if(pRow)
		{
			long nReqID = VarLong(pRow->GetValue(mstReqID));
			if(IDYES == MsgBox(MB_YESNO, "Are you sure you want to delete this request?"))
			{
				ExecuteParamSql(GetRemoteData(), "DELETE FROM ProductRequestsT where ID = {INT}", nReqID);
				RequeryMainDataList();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-05-18 10:31) - PLID 50300 - Open the sign request dlg
void CInvInternalManagementDlg::OnSignRequest()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_StatusList->CurSel;
		if(pRow)
		{
				long nRowID = VarLong(pRow->GetValue(mstReqID));
				CString strCheckoutUsername = VarString(pRow->GetValue(mstRequestFor));

				//Display the prompt to sign for the equipment
				CInvInternalSignEquipmentDlg dlgSign;
				
				dlgSign.m_bShowSkip = false;
				dlgSign.m_strCheckoutUsername = strCheckoutUsername;

				if(dlgSign.DoModal() == IDOK)
				{
					ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT SET Signed = 1 WHERE ID = {INT}", nRowID);

					RequeryMainDataList();
				}
		}
	}NxCatchAll(__FUNCTION__);
}


 //(r.wilson) 5/2/2011 PL ID = 43353
 void CInvInternalManagementDlg::DragEndInventoryStatusList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
 {
	 // TODO: Add your message handler code here
	 long nFromReqID,nToProductID,nFromCategoryID,nToCategoryID,nToProductIDParent,nFromItemStatus;
	 BOOL bFoundDateTimeConflicts = FALSE;
	 BOOL bAcceptStatusChange = FALSE;
	 COleDateTime dtFrom, dtTo;

	 try
	 {
		if(lpRow == NULL || lpFromRow == NULL || bDragAllowed == FALSE )
		{
			//If either row somehow became Null during the fucntion call Exit function
			return;
		}

		 /* (r.wilson) 5/2/2011 --------- */
		IRowSettingsPtr pRowTo(lpRow);			// Row that the drag ended up on 
		IRowSettingsPtr pRowFrom(lpFromRow);    // Row that the drag was initiated from
		IRowSettingsPtr pRowToParent;			// Parent of the row that the drag initiated from (if it has one)
		IRowSettingsPtr pRowFromParent;			// Parent of the row that the drag end up on

		/* The the FROM Row is a parent row then we know that it is now a request and therefore cannot be dragged and dropped */
		if(pRowFrom->GetParentRow() == NULL)
		{
			return;
		}
		/* The Item gets dragged under the same parent that it is already at */
		else if(pRowFrom->GetParentRow() == pRowTo->GetParentRow())
		{
			return;
		}
		else
		{
			//Continue [Here Just for sake of clarity]
		}

		// This if gets hit if the item got dragged to a parent row
		if(pRowTo->GetParentRow() == NULL)
		{
			// We know that pRowTo has NO parent so go ahead and use pRowTo as the Parent
			pRowToParent = pRowTo;

			// Get ProductID of the row that we are dragging To
			nToProductID = VarLong(pRowTo->GetValue(mstID),-1);
		}
		else
		{
			pRowToParent = pRowTo->GetParentRow(); 
			IRowSettingsPtr pRowToParent = pRowTo->GetParentRow();
			nToProductID = VarLong(pRowToParent->GetValue(mstID),-1);
		}

		nFromReqID = VarLong(pRowFrom->GetValue(mstReqID), -1);
		dtFrom = VarDateTime(pRowFrom->GetValue(mstCheckoutDate));
		dtTo = VarDateTime(pRowFrom->GetValue(mstReturnDate));
		// (j.fouts 2012-05-08 11:08) - PLID 50210 - Load the Indefinite status from the datalist
		CString strIndefinite = VarString(pRowFrom->GetValue(mstReturnDateDisplay), "");
		BOOL bIndefinite = strIndefinite == "Indefinite";

		nFromCategoryID = VarLong(pRowFrom->GetValue(mstCategoryID),-1);
		nToCategoryID = VarLong(pRowTo->GetValue(mstCategoryID),-1);
		
		//r.wilson plid 43470 - 5/27/2011
		nFromItemStatus = VarLong(pRowFrom->GetValue(mstStatus),-1);
		
//		nToProductIDParent = VarLong(pRowTo->GetParentRow()->GetValue(mstID),-1);
		nToProductIDParent = VarLong(pRowToParent->GetValue(mstID),-1);		

		// (r.wilson) 5/16/2011 -> Check To See If Categories match between the drag and drop rows
		if(nFromCategoryID != nToCategoryID )
		{
			AfxMessageBox("Request cannot be moved unless the categories are the same.");
			return;
		}
		if( nToProductIDParent < 0)
		{
			AfxMessageBox("You can't move a request back to an {ANY} row.");
			return;
		}

		
		// (j.fouts 2012-05-08 11:14) - PLID 50210 - Added bIndefinite
		bFoundDateTimeConflicts = DoRequestDateTimesConflict(nToProductID,nFromReqID,dtFrom, dtTo, bIndefinite);
		//bFoundDateTimeConflicts = FALSE;

			//r.wilson plid 43470  - 5/27/2011
			// Checks to see if current request is already approved and ask the user if they 
			// are sure that they want to change their request despite the fact that they the request will have
			// to be reapproved
		_RecordsetPtr rsRecords = CreateParamRecordset("SELECT ItemStatus FROM ProductRequestsT WHERE ID = {INT}", nFromReqID);
			//(e.lally 2012-01-05) PLID 46492 - Assume default value of pending, use enum values
			 nFromItemStatus = AdoFldByte(rsRecords,"ItemStatus", mstPending);
			
		if(nFromItemStatus == mstApproved)
		{
			
			if(MessageBox("If you choose to accept this move then a manager will have to reapprove this request. Are you sure you wan't to continue?",NULL,MB_YESNO|MB_ICONSTOP) == IDNO)
			{
				bAcceptStatusChange = FALSE;
			}
			else
			{
				nFromItemStatus = 0;
				bAcceptStatusChange = TRUE;
			}
		}

		if(!bFoundDateTimeConflicts && bAcceptStatusChange)
		{
		
			/*r.wilson 5/16/2011 -> 
			  For each request we either want to know a ProductID or a Category of an item that a user wants (AKA We want to know if the user 
			  wants a laptop of any kind or if they need a specific one ). When a request gets bound to a certain Product then we don't care what category it 
			  is in since we can get it by joining the category with the products. If the Category is populated then the request will be a child of an ANY row 
			  while if the ProductID is populated then we know that the request is bound to an actual physical item*/
			ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT set ProductID = {INT}, CategoryID = NULL,ItemStatus = {INT} WHERE ID = {INT}", nToProductID,nFromItemStatus,nFromReqID);
			
			/* The Actual datalist is being updated
			   and the row is added under its new Parent*/
			m_StatusList->RemoveRow(pRowFrom);
			m_StatusList->AddRowAtEnd(pRowFrom ,pRowToParent);
			
		}
		else if(bFoundDateTimeConflicts)
		{
			AfxMessageBox("Cannot fufill request because because the dates overlap.");
		}
		//(e.lally 2012-01-05) PLID 46492 - Use enum values
		else if(nFromItemStatus != mstApproved && nFromItemStatus != mstCheckedOut)
		{
			/*r.wilson 6/1/2011 PLID 43470 
			        Fixed a problem with the drag and drop feature*/
			ExecuteParamSql(GetRemoteData(), "UPDATE ProductRequestsT set ProductID = {INT}, CategoryID = NULL,ItemStatus = {INT} WHERE ID = {INT}", nToProductID,nFromItemStatus,nFromReqID);
		}

	 }NxCatchAll(__FUNCTION__);
	
		RequeryMainDataList();

	 return;

 }


 /* (r.wilson) 5/2/2011 PL ID = 43353 */
 // (j.fouts 2012-05-08 10:37) - PLID 50210 - Added the bIndefinite paramater
 BOOL CInvInternalManagementDlg::DoRequestDateTimesConflict(int nProductID, int nRequestID,COleDateTime dtFrom, COleDateTime dtTo, BOOL bIndefinite)
 {
	 /* (r.wilson) 5/2/2011 updated on 5/9/2011 */
	 try{
		int nNumberOfConflicts = -1;

		_RecordsetPtr rsResults;
		// (j.fouts 2012-05-08 10:41) - PLID 50210 - Create the query based on whether this is an indefinite checkout of not
		if(bIndefinite)
		{	
			rsResults = CreateParamRecordset(
				"SELECT Count(ProductRequestsT.ID) AS Conflicts "
				"FROM ProductRequestsT WHERE ProductID = {INT} "
				"AND ActualEnd IS NULL AND ID <> {INT} "
				"AND ItemStatus <> 1 "
				"AND NOT ((Indefinite = 0 AND RequestFrom < {OLEDATETIME} AND RequestTo < {OLEDATETIME} ))",
				nProductID, nRequestID, dtFrom, dtFrom);

		} else {
			rsResults = CreateParamRecordset(
				"SELECT Count(ProductRequestsT.ID) AS Conflicts "
				"FROM ProductRequestsT WHERE ProductID = {INT} "
				"AND ActualEnd IS NULL AND ID <> {INT} "
				"AND ItemStatus <> 1 "
				"AND NOT ((Indefinite = 0 AND RequestFrom < {OLEDATETIME} AND RequestTo < {OLEDATETIME} ) "
				"OR (Indefinite = 0 AND RequestFrom > {OLEDATETIME} AND RequestTo > {OLEDATETIME}) "
				"OR (Indefinite = 1 AND RequestFrom > {OLEDATETIME}))",
				nProductID, nRequestID, dtFrom, dtFrom, dtTo, dtTo, dtTo);
		}

		while(!rsResults->eof)
		{
			nNumberOfConflicts = AdoFldLong(rsResults,"Conflicts",-1);
			rsResults->MoveNext();
		}
		


		if(nNumberOfConflicts > 0)
		{
			//Conflict has been found
			return TRUE;
		}

		else
		{
			//No conflicts found
			return FALSE;
		}

	}NxCatchAll(__FUNCTION__);
	
	 return TRUE;
 }

 /* r.wilson PLID 43353 
	5/10/2011*/
 void CInvInternalManagementDlg::FireDragBegin(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags)
 {

	 /*This Function checks to see if the row being dragged from is a Parent or a Child.
	      For Parents the visible drag and drop is disabled since they can't be dragged.
		  For Children the visible drag and drop is turned on.*/
	
	 try{
			
			IRowSettingsPtr pRow(lpRow);

			if(pRow->GetParentRow() == NULL)
			{
				m_StatusList->DragVisible = FALSE;
				bDragAllowed = FALSE;
				return;
			}

			else
			{
				m_StatusList->DragVisible = TRUE;
				bDragAllowed = TRUE;
			}

			long productID = VarLong(pRow->GetValue(mstParentProduct));
			// (d.thompson 2012-01-23) - PLID 47714 - We now support multiple owners.  And we can always get the category ID
			//	from the parent, it doesn't matter if it's an {Any} or a regular item.
			long nCategoryID = VarLong(pRow->GetParentRow()->GetValue(mstCategoryID));
			
			//r.wilson PLID 43353 -> Referenced Code from d.singleton
				//item is "any" meaning no owner, so check to see who is the owner of items in the category of the "any" item
				
				long nReqID = VarLong(pRow->GetValue(mstReqID));
				CString strRequestedBy = VarString(pRow->GetValue(mstRequestBy));
				//r.wilson PLID 43353 -> Added 6/3/2011
				CString strRequestedFor = VarString(pRow->GetValue(mstRequestFor));

				// (d.thompson 2012-01-23) - PLID 47714 - We now support multiple owners
				_RecordsetPtr reResults = CreateParamRecordset("SELECT OwnerID "
					"FROM ProductRequestableT "
					"INNER JOIN ServiceT on ProductRequestableT.ProductID = ServiceT.ID "
					"INNER JOIN ProductRequestableOwnersT ON ProductRequestableT.ID = ProductRequestableOwnersT.ProductRequestableID "
					"WHERE ServiceT.Category = {INT} AND OwnerID = {INT}", nCategoryID, GetCurrentUserID());
				//I moved the Owner match into the query
				bool bIsCurrentUserAnOwner = false;
				if(!reResults->eof) {
					//Any results, we're the owner of something in this category
					bIsCurrentUserAnOwner = true;
				}
				else {
					//No results, the current user did not match any ownership in this category
					bIsCurrentUserAnOwner = false;
				}

			if(productID < 0)
			{
				// r.wilson 6/3/2011 PLID 43353 -------------changed from--  strRequestedBy 
				// (d.thompson 2012-01-23) - PLID 47714 - We now support multiple owners
				if(bIsCurrentUserAnOwner || GetCurrentUserName() == strRequestedFor)
				{
					m_StatusList->DragVisible = TRUE;
					bDragAllowed = TRUE;
				}
				else
				{
					m_StatusList->DragVisible = FALSE;
					bDragAllowed = FALSE;
				}
			}

			else 
			{	//r.wilson PLID 43353 ->  tweaked 6/3/2011
				if(GetCurrentUserName() == VarString(pRow->GetValue(mstRequestFor)) || bIsCurrentUserAnOwner )
				{
					m_StatusList->DragVisible = TRUE;
					bDragAllowed = TRUE;
				}
				else
				{
					m_StatusList->DragVisible = FALSE;
					bDragAllowed = FALSE;
				}
			}
			
			// r.wilson - plid 43470 5/27/2011
			// If an item has a status of 'Checked Out' aka 3 then it can not be drag n dropped
			_RecordsetPtr rsResults = CreateParamRecordset("SELECT ItemStatus FROM ProductRequestsT WHERE ID = {INT} ", nReqID);
			//(e.lally 2012-01-05) PLID 46492 - Assume default value of pending, use enum values
			int nItemStatus = AdoFldByte(rsResults,"ItemStatus", mstPending);

			if(nItemStatus == mstCheckedOut)
			{
				m_StatusList->DragVisible = FALSE;
				bDragAllowed = FALSE;
			}

	}NxCatchAll(__FUNCTION__);

 }

  /* r.wilson PLID 43353 
	Finished on 5/13/2011*/
void CInvInternalManagementDlg::OnRequeryFinishedStatusList(short nFlags)
{
	try{
		
		
		IRowSettingsPtr pRowStatus = m_StatusCombo->GetCurSel();
		ManagementStatusType msType = (ManagementStatusType)VarLong(pRowStatus->GetValue(stccID), (long)mstAll);

		//Expand All Rows That have children
		IRowSettingsPtr pRowNext;
		IRowSettingsPtr pRow = m_StatusList->GetFirstRow();

		while(pRow != NULL)
		{
			if(pRow->GetFirstChildRow() != NULL)
			{
				pRow->PutExpanded(TRUE);
			} 
			
			/* r.wilson PLID 43353 5/16/2011 If/Else statements for when the Status combo is not set to ALL so that the user won't
			  see stray items with no requests associated with them */
			if(msType != mstAll && pRow->GetParentRow() == NULL && pRow->GetFirstChildRow() == NULL)
			{
				pRowNext = pRow->GetNextRow();
				m_StatusList->RemoveRow(pRow);
				pRow = pRowNext;
			}
			else
			{
				pRow = pRow->GetNextRow();
			}
		}

	}NxCatchAll(__FUNCTION__);

	return ;

}


// (j.luckoski 2012-03-29 11:10) - PLID 49279 - Set property for remembering filters.

void CInvInternalManagementDlg::OnRememberFilters() 
{
	try {
		if(m_Filters.GetCheck()) {
			SetRemotePropertyInt("InvManagerRememberFilter", 1, 0, GetCurrentUserName()); 
		} else {
			SetRemotePropertyInt("InvManagerRememberFilter", 0, 0, GetCurrentUserName());
		}
	
	}NxCatchAll(__FUNCTION__); 
	return;
}

// (j.luckoski 2012-03-30 08:51) - PLID 49094 - Since I used this code in two spots I made a new function
BOOL CInvInternalManagementDlg::IsAdmin(long nProductID, long nCategoryID, BOOL& bIsAny) 
{
	try {
		BOOL bAllowAdminFeatures = FALSE;
				if(nProductID > 0)
				{
					// (d.thompson 2012-01-23) - PLID 47714 - We now support multiple managers for each item.  We cannot keep
					//	this in the datalist, so we'll have to do a lookup.
					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ProductRequestableOwnersT.ID "
						"FROM ProductRequestableOwnersT "
						"INNER JOIN ProductRequestableT ON ProductRequestableOwnersT.ProductRequestableID = ProductRequestableT.ID "
						"WHERE OwnerID = {INT} AND ProductRequestableT.ProductID = {INT} ", 
						GetCurrentUserID(), nProductID);

					if(!prs->eof)
					{
						//There is a record, so the current user is an admin for this particular product
						bAllowAdminFeatures = TRUE;
					}
					else {
						//There is no record, so the current user is not an admin for this product
					}
				}
				else
				{
					//item is "any" meaning no owner, so check to see who is the owner of items in the category of the "any" item
					bIsAny = TRUE;

					// (d.thompson 2012-01-23) - PLID 47714 - Changed to support multiple owners per product.  If you own any item in this
					//	category you can admin it here.  Also removed a duplicate trip to the database.
					

					//select category for this request id (only exists for {Any} records)
					//lookup Owner for all requestable products that have that category
					//iterate through all results, if we find the current user, break out and set flag true

					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ProductRequestableOwnersT.ID "
						"FROM ProductRequestableOwnersT "
						"INNER JOIN ProductRequestableT ON ProductRequestableOwnersT.ProductRequestableID = ProductRequestableT.ID "
						"INNER JOIN ServiceT ON ProductRequestableT.ProductID = ServiceT.ID "
						"WHERE ProductRequestableOwnersT.OwnerID = {INT} AND ServiceT.Category = {INT};",
						GetCurrentUserID(), nCategoryID);

					if(!prs->eof)
					{
						//There is at least 1 record here, then at least 1 product that shares this category also is owned by the current user
						bAllowAdminFeatures = TRUE;
					}
					else {
						//There are no records, so no products owned by the current user share this category
					}
				}
				return bAllowAdminFeatures;
	} NxCatchAll(__FUNCTION__);
	return false;
}


// (j.luckoski 2012-04-02 12:32) - PLID 48195 - For show history button but only has requery for now. May add features later.
void CInvInternalManagementDlg::OnShowHistory() 
{
	try {
			if(!bLoadedParentCategoryID)
			{
				//r.wilson -> if the Parent Category couldn't be loaded then this is unusable
				return;
			}
			else
			{

				// (j.luckoski 2012-03-29 11:09) - PLID 49279 - Set properties for the request checkbox
				if(m_History.GetCheck()) {
					SetRemotePropertyInt("InvManagerShowReturns", 1, 0, GetCurrentUserName()); 
				} else {
					SetRemotePropertyInt("InvManagerShowReturns", 0, 0, GetCurrentUserName());
				}

				RequeryMainDataList();
			}
	} NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-05-08 08:46) - PLID 50210 - Handling the Indefinite filter
void CInvInternalManagementDlg::OnBnClickedHideIndefinite()
{
	try {
		if(m_Indefinite.GetCheck())
		{
			SetRemotePropertyInt("InvManagerHideIndefinite", 1, 0, GetCurrentUserName()); 
		} else {
			SetRemotePropertyInt("InvManagerHideIndefinite", 0, 0, GetCurrentUserName());
		}
		
		RequeryMainDataList();

	} NxCatchAll(__FUNCTION__);
}
