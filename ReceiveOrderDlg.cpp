// ReceiveOrderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Mainfrm.h"
#include "ReceiveOrderDlg.h"
#include "InternationalUtils.h"
#include "Barcode.h"
#include "InvUtils.h"
#include "AuditTrail.h"
#include "GlobalSchedUtils.h"
#include "InvPatientAllocationDlg.h"
#include "InventoryRc.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.




//DRT 11/6/2007 - PLID 19682 - Created

#define TIMER_AUTO_BARCODE_CHECK	1

//Column enums for main list
enum eListColumns {
	elcProductID = 0,
	elcBarCode,
	elcHasSerial,
	elcHasExp,
	elcSerialNumIsLotNum,	//TES 8/20/2008 - PLID 24726 - Does this product treat serial numbers as lot numbers?
	elcName,
	elcReceiveTo,		//DRT 1/8/2008 - PLID 28473 - Where is it destined?  Purchased Inv?  Consignment?
	elcQtyOrdered,
	elcQtyReceived,
	elcDlgPtr,
	elcUseUU,
	elcSerialPerUO,
	elcConversion,
	elcDetailID,
	elcStatus, // (c.haag 2007-12-03 11:31) - PLID 28204 - This used to be a consignment bit. Now it's an integer flag.
};


#define COLOR_UNDER	RGB(255, 0, 0)
#define COLOR_OK RGB(0, 0, 0)

//Copied from InvOrderDlg, it's needed for saving.
static COleDateTime GetDate()
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return COleDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////
// CReceiveOrderDlg dialog


CReceiveOrderDlg::CReceiveOrderDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CReceiveOrderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReceiveOrderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nOrderID = -1;
	m_bstrFromBarcode = NULL;
	m_nLinkedApptID = -1;
	m_nLocationID = -1;
}

// (j.jones 2008-12-02 16:44) - PLID 31526 - added destructor
CReceiveOrderDlg::~CReceiveOrderDlg()
{
	try {

		// (j.jones 2008-12-02 16:42) - PLID 31526 - re-allow notification popups
		if(GetMainFrame()) {
			GetMainFrame()->AllowPopup();
		}

	}NxCatchAll("Error in CReceiveOrderDlg::~CReceiveOrderDlg()");
}

void CReceiveOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReceiveOrderDlg)
	DDX_Control(pDX, IDC_RAD_RECEIVE_ORDER_BARCODE_PROMPT, m_btnPromptInc);
	DDX_Control(pDX, IDC_SCAN_ONLY_MODE, m_btnScanOnlyMode);
	DDX_Control(pDX, IDC_RAD_RECEIVE_ORDER_BARCODE_INC, m_btnAutoInc);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ORDER_PLACED_DATE, m_nxstaticOrderPlacedDate);
	DDX_Control(pDX, IDC_ORDER_PLACED_FROM, m_nxstaticOrderPlacedFrom);
	DDX_Control(pDX, IDC_ORDER_TRACKING_NUM, m_nxstaticOrderTrackingNum);
	DDX_Control(pDX, IDC_MARK_ALL_RECEIVED, m_btnMarkAllReceived);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReceiveOrderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CReceiveOrderDlg)
	ON_BN_CLICKED(IDC_RAD_RECEIVE_ORDER_BARCODE_INC, OnRadReceiveOrderBarcodeInc)
	ON_BN_CLICKED(IDC_RAD_RECEIVE_ORDER_BARCODE_PROMPT, OnRadReceiveOrderBarcodePrompt)
	ON_BN_CLICKED(IDC_SCAN_ONLY_MODE, OnScanOnlyMode)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_MESSAGE(NXM_POST_EDIT_PRODUCT_ITEMS_DLG, OnPostEditProductItemsDlg)
	ON_BN_CLICKED(IDC_MARK_ALL_RECEIVED, OnMarkAllReceived)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReceiveOrderDlg message handlers

BOOL CReceiveOrderDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (j.jones 2008-12-02 16:42) - PLID 31526 - disallow notification popups while receiving an order
		if(GetMainFrame()) {
			GetMainFrame()->DisallowPopup();
		}

		m_pList = BindNxDataList2Ctrl(this, IDC_RECEIVE_ORDER_LIST, GetRemoteData(), false);
		m_pLocationList = BindNxDataList2Ctrl(this, IDC_RECEIVE_ORDER_LOCATION, GetRemoteData(), true);

		// (c.haag 2008-04-29 11:45) - PLID 29820 - NxIconify the icons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnMarkAllReceived.AutoSet(NXB_MODIFY);

		// (c.haag 2007-12-03 14:53) - PLID 28204 - Set the combo source for the status column
		CString strStatusSource = FormatString("%d;Purchased Inv.;%d;Consignment", 
			InvUtils::odsPurchased, InvUtils::odsConsignment);
		m_pList->GetColumn(elcStatus)->ComboSource = _bstr_t(strStatusSource);
		
		// (a.walling 2008-02-18 16:02) - PLID 28946 - Hide this column. After they choose the items they've recieved, they will
		// all be purchased inventory anyway, assuming they have no license.
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pList->GetColumn(elcReceiveTo);
			pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
			pCol->PutEditable(VARIANT_FALSE);
			pCol->StoredWidth = 0;

			pCol = m_pList->GetColumn(elcStatus);
			pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
			pCol->PutEditable(VARIANT_FALSE);
			pCol->StoredWidth = 0;
		}

		//
		//Setup so that we are receiving barcode scan messages
		if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
			AfxMessageBox("Failed to register for barcode messages.  You may be unable to use a barcode scanner at this time.");
		}

		//Retrieve the preference
		//	0 = Increment, 1 = Prompt
		if(GetRemotePropertyInt("ReceiveOrderBarcodePref", 0, 0, GetCurrentUserName(), true) == 0) {
			CheckDlgButton(IDC_RAD_RECEIVE_ORDER_BARCODE_INC, TRUE);
		}
		else {
			CheckDlgButton(IDC_RAD_RECEIVE_ORDER_BARCODE_PROMPT, TRUE);
		}

		//DRT 11/20/2007 - PLID 28138 - New ability to scan once and have the dialog close.
		if(GetRemotePropertyInt("ReceiveOrderScanOnlyMode", 0, 0, GetCurrentUserName(), true) == 0) {
			CheckDlgButton(IDC_SCAN_ONLY_MODE, FALSE);
		}
		else {
			CheckDlgButton(IDC_SCAN_ONLY_MODE, TRUE);
		}

		// (c.haag 2008-06-25 11:12) - PLID 28438 - If an order is at least partially received, we forbid
		// the user from changing location-related information. m_nOrderID should never be invalid, but do
		// a safety check anyway		
		if (m_nOrderID > 0) {
			BOOL bEnable;
			if (InvUtils::IsOrderPartiallyOrFullyReceived(m_nOrderID)) {
				bEnable = FALSE;
			} else {
				bEnable = TRUE;
			}
			GetDlgItem(IDC_RECEIVE_ORDER_LOCATION)->EnableWindow(bEnable);
		}

		//
		//Load the order from the given ID
		LoadCurrentOrder();

		//DRT 12/4/2007 - PLID 28235 - If we've opened because of a barcode scan, simulate that
		//	barcode scan so the user doesn't have to do it twice.  We use PostMessage so that the interface
		//	of this dialog will show up before the scan is processed.
		//DRT 1/4/2008 - PLID 28235 - I hate to do this, but cannot find a better way.  If you just post a message
		//	for the barcode scan, then most of the time (maybe always, I can't confirm) your popup window will not get 
		//	focus, because this one is still being created.
		SetTimer(TIMER_AUTO_BARCODE_CHECK, 100, NULL);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReceiveOrderDlg::OnOK() 
{
	//For auditing
	long nNewAuditID = -1;

	try {
		//In the old behavior, this looped through all details in the order, and if they were
		//	serialized / expirable, it prompted for the product items dialog, then immediately saved.  We've already 
		//	allowed the user to prompt for the serial / expire info, so we can skip this.  We will, however, need to
		//	implement our own saving.

		//For later, save the location.  It must be selected to continue.
		NXDATALIST2Lib::IRowSettingsPtr pRowLocation = m_pLocationList->CurSel;
		if(pRowLocation == NULL) {
			AfxMessageBox("You must select a location to commit the order.");
			return;
		}

		//We'll setup a batch of CString queries to execute.  Each one should be created in a manner to use ExecuteSqlBatch().
		CStringArray aryProductItemQueries;

		//Everything non-product will be batched together to run as 1 query
		CString strProductSplitQueries;

		//The old methodology required that the entire order was received.  We're relaxing that, and partials are allowed.  But we want
		//	to warn them that this is going to happen.
		bool bAnyPartialDetails = false;

		//Needed to see if we should warn users about not picking any quantities.
		bool bAllItemsZero = true;

		//Neetedt to see if any items are not received
		bool bSomeItemsZero = false;

		//Needed for query generation if we have to split any products.
		bool bAddDetailDeclaration = false;

		//Needed for anything that is 0 received, we want to skip those rows entirely
		CDWordArray aryIDsToSkip;

		//(e.lally 2008-03-25) PLID 29335 - Need a map of ProductID to map pointer (managing secondary mapping memory allocation)
		CMap <long, long, CMapStringToString*, CMapStringToString*> mapAllSerials;
		//CMap <long, long, CString, LPCSTR> mapSerialDuplicates;
		CArray <ItemSerialList, ItemSerialList> aryListDuplicates; //ProductID, ProductName, Serial#

		std::vector<CString> aryAlreadyReceivedProducts;

		nNewAuditID = BeginAuditTransaction();

		//Iterate through the datalist.  Every row that is a serialized / expirable item needs to have a product items query generated.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow != NULL) {
			long nQtyOrdered = VarLong(pRow->GetValue(elcQtyOrdered));
			long nQtyReceived = VarLong(pRow->GetValue(elcQtyReceived));

			//First off, we only operate on values that are not 0.  If it's 0, we'll just entirely skip this detail.
			long nOrderDetailID = VarLong(pRow->GetValue(elcDetailID));
			if(nQtyReceived > 0) {
				bAllItemsZero = false;
				BOOL bHasSerial = VarBool(pRow->GetValue(elcHasSerial));
				BOOL bHasExp = VarBool(pRow->GetValue(elcHasExp));

				if(nQtyOrdered != nQtyReceived) {
					bAnyPartialDetails = true;
				}

				if(bHasSerial || bHasExp) {
					//We should have a dialog that has all the data to save.
					CProductItemsDlg *pDlg = (CProductItemsDlg*)VarLong(pRow->GetValue(elcDlgPtr));
					CString strProductName = VarString(pRow->GetValue(elcName), "");
					if(pDlg == NULL) {
						//This is OK, it just means that they never clicked on this item to receive any.  Leave it as unreceived.
					}
					else {
						if(nQtyReceived == 0) {
							//Still possible they clicked on the dialog, but still received nothing.  Again, these are left unreceived.
						}
						else {
							//TES 8/20/2008 - PLID 24726 - If this product treats serial numbers as lot numbers, then there's no
							// need to check for duplicates, duplicates are fine.
							if(!VarBool(pRow->GetValue(elcSerialNumIsLotNum))) {
								//(e.lally 2008-03-25) PLID 29335 - Check for duplicate serial numbers. The ProductItemsDlg code should be
								//checking for duplicates against its own list and in data. We can assume that was done correctly. It is now
								//our job to check the serial number against the full order of non-commited data.
								CMapStringToString* pmapItemSerials = NULL;
								if(bHasSerial != FALSE){
									//Get the ProductID for the current Item.
									long nProductID = pDlg->m_ProductID;
									//check if we have a serial mapping for it.
									if(!mapAllSerials.Lookup(nProductID, pmapItemSerials)){
										//if not create one. Possibly check for duplicates against itself?
										pmapItemSerials = new CMapStringToString();
										//and add it to our main mapping
										mapAllSerials.SetAt(nProductID, pmapItemSerials);

									}
									//(e.lally 2008-03-25) PLID 29335 - Now go through each serial number for this order detail, 
										//and check for duplicates against the product mapping
									CString strSerialNumber, strTempValue;
									for(int iIndex = 0, iCount = pDlg->GetSelectedProductItemCount(); iIndex<iCount; iIndex++){
										//if so, check for duplicates in that map.
										strSerialNumber = VarString(pDlg->GetSelectedProductItemSerialNum(iIndex),"");

										if(mapAllSerials.Lookup(nProductID, pmapItemSerials)){
											//(e.lally 2008-03-25) PLID 29335 - We have the right item mapping
											//look for an existing entry of this serial number
											if(pmapItemSerials->Lookup(strSerialNumber, strTempValue)){
												//we have a duplicate serial number for this product.
												//Check our list of duplicates and add it if it is not already there
												BOOL bAlreadyListed = FALSE;
												for(int iDupListIndex=0; iDupListIndex < aryListDuplicates.GetSize() && bAlreadyListed == FALSE; iDupListIndex++){
													if(nProductID == aryListDuplicates.GetAt(iDupListIndex).nProductID 
														&& strSerialNumber == aryListDuplicates.GetAt(iDupListIndex).strSerialNum){
														bAlreadyListed = TRUE;
													}
												}
												//We should never have a blank s/n here, but check just in case.
												if(bAlreadyListed == FALSE && !strSerialNumber.IsEmpty()){
													ItemSerialList islNew;
													islNew.nProductID = nProductID;
													islNew.strProductName = strProductName;
													islNew.strSerialNum = strSerialNumber;
													aryListDuplicates.Add(islNew);
												}
											}
											else if(!strSerialNumber.IsEmpty()){
												//This serial number does not exist on their item's map, add it
												pmapItemSerials->SetAt(strSerialNumber, strSerialNumber);
											}
										}
										else {
											//this should not be possible. Our check to add it to our main mapping 
												//above must have failed.
											ASSERT(FALSE);
										}
									}
								}
							}

							//safety check: has another user already received these products?
							if (InvUtils::HasOrderDetailReceivedProductItems(nOrderDetailID)) {
								//someone has already received these products, track the product name,
								//we will warn later
								aryAlreadyReceivedProducts.push_back(strProductName);
							}
							else {
								//These are the real deal.  The dialog has a query to insert them into data, how nice of it.
								aryProductItemQueries.Add(pDlg->m_strSavedDataEntryQuery);
							}
						}
					}
				}
				else {
					//This row is not serialized, and will be satisfied with us just marking it received.
				}

				//For auditing.  We generate this ahead of time so that the conversion of UU quantity doesn't confuse things.
				CString strAudit;
				strAudit.Format("Received quantity %li for date '%s'.", nQtyReceived, FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), NULL, dtoDate));

				//
				//For the above product items, as well as all normal items, if the received number does not match up
				//	with the ordered, we are going to need to make a split.  This code mirrors what happens in the order
				//	when you right click and mark 'partial received'.
				if(nQtyReceived != nQtyOrdered) {
					bAddDetailDeclaration = true;

					CString strSql;
					//1)  Change the qty of the existing detail to be what we received.
					//	Don't forget:  For UU items, we have to multiply the Qty shown by the Conversion value.  We'll just do this in the insert portion
					BOOL bIsUU = VarBool(pRow->GetValue(elcUseUU));
					long nConversion = VarLong(pRow->GetValue(elcConversion), 1);
					if(bIsUU) {
						nQtyReceived *= nConversion;
						nQtyOrdered *= nConversion;
					}
					
					//TES 7/24/2008 - PLID 30802 - We need to also keep @ReceivedOrderDetailsT up to date, at the end we're
					// going to pull from it and pass the information into InvUtils::UpdateLinkedAllocations();
					strSql.Format("UPDATE OrderDetailsT SET QuantityOrdered = %li WHERE ID = %li;\r\n"
						"UPDATE @ReceivedOrderDetailsT SET Quantity = %li WHERE ID = %li;\r\n"
						//2)  Insert a record that's a copy with the qty set to the difference.  The 'ExtraCost' gets set to $0.  I'm not entirely
						//	sure why, but that is the behavior when you split an order manually with a "partial receive" on the order dialog.
						//The 'DateReceived' data will be NULL.
						// (c.haag 2007-12-03 11:34) - PLID 28204 - Changed ForConsignment bit to integer ForStatus flag
						// (j.jones 2008-06-19 16:42) - PLID 10394 - bring over only the percent off, not the discount,
						// if they need to discount the new line item, they would need to manually edit it
						"INSERT INTO OrderDetailsT (ID, OrderID, ProductID, QuantityOrdered, Amount, PercentOff, ExtraCost, DateReceived, ModifiedBy, Deleted, UseUU, Conversion, Catalog, ForStatus) "
						"SELECT @NewDetailID, OrderID, ProductID, %li, Amount, PercentOff, convert(money, 0), NULL, ModifiedBy, Deleted, UseUU, Conversion, "
						"Catalog, ForStatus FROM OrderDetailsT WHERE ID = %li;\r\n"

						"INSERT INTO @ReceivedOrderDetailsT (ID, Quantity, SourceDetailID, Received) "
						"VALUES (@newDetailID, %li, %li, 0)\r\n"
						"SET @NewDetailID = @NewDetailID + 1;\r\n", 
						nQtyReceived, nOrderDetailID, nQtyReceived, nOrderDetailID, (nQtyOrdered - nQtyReceived), nOrderDetailID, (nQtyOrdered - nQtyReceived), nOrderDetailID);

					strProductSplitQueries += strSql;
				}
				else {
					//The full complement of items was received.  This item will fall in with our normal "mark everything received" behavior below.
				}

				//DRT 11/12/2007 - PLID 28052 - We need to audit
				AuditEvent(-1, VarString(pRow->GetValue(elcName)), nNewAuditID, aeiOrderReceived, VarLong(pRow->GetValue(elcProductID)), "", strAudit, aepMedium, aetChanged);
			}
			else {
				//If there is a 0 quantity, we want to skip this row entirely when marking them as received.
				aryIDsToSkip.Add(nOrderDetailID);

				bSomeItemsZero = true;
			}

			pRow = pRow->GetNextRow();
		}

		//(e.lally 2008-03-25) PLID 29335 - Clean up any memory allocation from our duplicates check
		POSITION pos = mapAllSerials.GetStartPosition();
		CMapStringToString* mapItemSerials;
		long nProductID=0;
		while(pos){
			mapAllSerials.GetNextAssoc(pos, nProductID, mapItemSerials);
			if(mapItemSerials != NULL){
				delete mapItemSerials;
			}
		}
		mapAllSerials.RemoveAll();

		//warn if we determined another user has already received these products
		if (aryAlreadyReceivedProducts.size() > 0)
		{
			CString strMessage = "The following products have already been fully received:\r\n";
			long nCount = 0;
			for each(CString strProductName in aryAlreadyReceivedProducts)
			{
				strMessage += "\r\n" + strProductName;
				nCount++;
				if (nCount > 10) {
					break;
				}
			}

			if (nCount < (long)aryAlreadyReceivedProducts.size()) {
				strMessage += "\r\n<More...>";
			}

			strMessage += "\r\n\r\nYou may need to close this window and attempt receiving the order again to ensure the order status has been updated.";

			AfxMessageBox(strMessage, MB_ICONERROR | MB_OK);
			
			if (nNewAuditID != -1) {
				RollbackAuditTransaction(nNewAuditID);
			}
			return;
		}

		//(e.lally 2008-03-25) PLID 29335 - Check our critical validations first, then optional warnings.
		//If there are duplicate serial numbers, tell the user and stop here.
		int iDupListSize = aryListDuplicates.GetSize();
		if(iDupListSize > 0 ){
			CString strMessage = "The following serial numbers have duplicate entries and need to be corrected:";
			CString strSerialNumber, strProductName;

			for(int j =0; j<iDupListSize; j++){
				ItemSerialList islCurrent;
				islCurrent = aryListDuplicates.GetAt(j);
				strProductName = islCurrent.strProductName;
				strSerialNumber = islCurrent.strSerialNum;

				strMessage += "\r\n" + strProductName + " - " + strSerialNumber;
			}

			AfxMessageBox(strMessage, MB_ICONERROR | MB_OK);
			//(e.lally 2008-03-25) PLID 29335 - Undo anything that needs it and return.
			if(nNewAuditID != -1){
				RollbackAuditTransaction(nNewAuditID);
			}
			return;
		}

		//I decided this message will not pop up if you have any 0 quantities.
		if(bAnyPartialDetails) {
			if(AfxMessageBox("At least one product in this order was not received in full quantity.  Are you sure you wish to commit this order?\r\n"
				"If you do so, the received products will be put into stock, and the products which were not received will remain on an open order.", MB_YESNO) == IDNO)
			{
				//Stop saving and quit
				RollbackAuditTransaction(nNewAuditID);
				return;
			}
		}
		else {
			//If there are no partials, then it's either fully received, or not received at all.  It has been suggested that we additionally warn
			//	the user if they don't hit anything, since this new functionality requires work as opposed to the old.
			if(bAllItemsZero) {
				if(AfxMessageBox("You have chosen to commit this order, but have not entered your quantities received.\r\n"
					"Do you wish to go back and enter quantities now?", MB_YESNO) == IDYES) 
				{
					//send them back
					RollbackAuditTransaction(nNewAuditID);
					return;
				}
				else {
					//If they aren't committing any data, it's a cancel, not an 'OK'.  This will close the dialog without running
					//	any queries.  Make sure we cancel our auditing.
					RollbackAuditTransaction(nNewAuditID);
					OnCancel();
					return;
				}
			}
		}

		//TES 7/24/2008 - PLID 30802 - We need to track which Allocations are updated (which they can be if they're linked
		// to this order), so that we can notify the user about them AFTER we're out of the transaction.
		CArray<long,long> arUpdatedAllocationIDs;
		//This all needs to run in a transaction
		BEGIN_TRANS("InvReceive")
		{
			CString strExecute;
			long nLocationID = VarLong(pRowLocation->GetValue(0));
			
			// (j.jones 2012-02-01 09:08) - PLID 34840 - declare the ID, and execute all our saves
			if (aryProductItemQueries.GetSize() > 0)
			{
				// (z.manning 2015-11-04 15:29) - PLID 67405 - The way this dialog interacts with the product items dialog
				// is far from ideal. Basically, the SQL to insert the product items is generated when when closing the
				// product items dialog. However, it's possible to change the receiving location on this dialog anytime
				// after that but the generated SQL will still be using whatever location ID was selected at the time.
				// Let's go ahead and find the point where the new product items will be added and then update the location
				// of the newly added items to the currently selected location.
				strExecute +=
					"DECLARE @maxExistingProductItemID INT \r\n"
					"SET @maxExistingProductItemID = (SELECT ISNULL(MAX(I.ID), 0) FROM ProductItemsT I) \r\n";

				// (b.spivey, February 17, 2012) - PLID 48080 - Removed declaration of @nNewProductItemID
				for (int i = 0; i < aryProductItemQueries.GetSize(); i++) {
					strExecute += aryProductItemQueries.GetAt(i) + "\r\n";
				}

				strExecute += FormatString(
					"UPDATE ProductItemsT SET LocationID = %li WHERE ID > @maxExistingProductItemID \r\n"
					, nLocationID);
			}

			//
			//Now that we have saved the ProductItems queries, we can save the DateReceived queries.  These are much better and can be batched up instead of run in a loop.
			//
			//Execute this query BEFORE we do the split queries.  That way any new records that are not received will remain unreceived.
			{
				//TES 7/23/2008 - PLID 30802 - Need to track information about received details in order to update linked allocations.
				CString strDeclareTable = "DECLARE @ReceivedOrderDetailsT TABLE ( \r\n"
					"	ID int, \r\n"
					"	Quantity int, \r\n"
					"	SourceDetailID int, \r\n"
					"	Received bit) \r\n";

				//We do allow them to modify the location as well, that needs an update too
				CString strLoc;
				strLoc.Format("UPDATE OrderT SET LocationID = %li WHERE ID = %li;\r\n", nLocationID, m_nOrderID);

				CString strFillTableSql;
				//TES 7/24/2008 - PLID 30802 - We need to also keep @ReceivedOrderDetailsT up to date, at the end we're
				// going to pull from it and pass the information into InvUtils::UpdateLinkedAllocations();
				strFillTableSql.Format("INSERT INTO @ReceivedOrderDetailsT (ID, Quantity, SourceDetailID, Received) "
					"SELECT ID, QuantityOrdered, NULL, 1 FROM OrderDetailsT WHERE OrderID = %li AND Deleted = 0 AND DateReceived IS NULL AND ID NOT IN (%s);\r\n",
					m_nOrderID, ArrayAsString(aryIDsToSkip));
				
				CString strSql;
				strSql.Format("UPDATE OrderDetailsT SET DateReceived = '%s' WHERE OrderID = %li AND Deleted = 0 AND DateReceived IS NULL AND ID NOT IN (%s);\r\n", 
					FormatDateTimeForSql(GetDate(), dtoDate), m_nOrderID, ArrayAsString(aryIDsToSkip));

				strExecute += strDeclareTable + strLoc + strFillTableSql + strSql;
				
				if(bAddDetailDeclaration) {
					strSql.Format("DECLARE @NewDetailID INT;\r\n"
						"SET @NewDetailID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM OrderDetailsT);\r\n");
					strExecute += strSql;
				}
				strExecute += strProductSplitQueries;
			}
			//TES 7/24/2008 - PLID 30802 - Execute the query, and pull everything from @ReceivedOrderDetailsT.
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushPerformanceWarningLimit ppw(-1);
			_RecordsetPtr rsDetails = CreateRecordsetStd("SET NOCOUNT ON\r\n" + strExecute + "\r\nSET NOCOUNT OFF\r\n"
				"SELECT * FROM @ReceivedOrderDetailsT\r\n");

			//TES 7/24/2008 - PLID 30802 - Now load that information into an array, and tell InvUtils to take care of it.
			CArray<InvUtils::ReceivedOrderDetailInfo,InvUtils::ReceivedOrderDetailInfo&> arReceivedDetails;
			while(!rsDetails->eof) {
				InvUtils::ReceivedOrderDetailInfo rodi;
				rodi.nOrderDetailID = AdoFldLong(rsDetails, "ID");
				rodi.nQuantity = AdoFldLong(rsDetails, "Quantity");
				rodi.nSourceOrderDetailID = AdoFldLong(rsDetails, "SourceDetailID", -1);
				rodi.bReceived = AdoFldBool(rsDetails, "Received")?true:false;
				arReceivedDetails.Add(rodi);
				rsDetails->MoveNext();
			}
			InvUtils::UpdateLinkedAllocationDetails(arReceivedDetails, arUpdatedAllocationIDs);

			//
			//DRT 11/12/2007 - PLID 28052 - Lastly, audit it all.
			CommitAuditTransaction(nNewAuditID);
			nNewAuditID = -1;		//Just in case something happens after this, we do not want to rollback

			// (c.haag 2007-11-14 11:07) - PLID 28094 - Fire a table checker for the order record
			CClient::RefreshTable(NetUtils::OrderT, m_nOrderID);

			// (j.jones 2008-03-24 15:25) - PLID 29388 - need to update the linked appointment
			if(m_nLinkedApptID != -1) {
				TrySendAppointmentTablecheckerForInventory(m_nLinkedApptID, FALSE);
			}

		} END_TRANS("InvReceive");

		//
		//Update the LastCost values - Do this outside the transaction, as this function can pop up message boxes asking
		//	the user what they wish to do.
		InvUtils::TryUpdateLastCostByOrder(m_nOrderID, GetDate());

		//TES 7/23/2008 - PLID 30802 - Now pop up the allocations.
		if(arUpdatedAllocationIDs.GetSize()) {
			//TES 7/23/2008 - PLID 30802 - Let them know what's going on.
			MsgBox("Marking this order received has caused details to be updated on %i allocation(s).  Each updated allocation will now "
				"be displayed, so that you can review and confirm the changes.", arUpdatedAllocationIDs.GetSize());
		}
		for(int nAllocation = 0; nAllocation < arUpdatedAllocationIDs.GetSize(); nAllocation++) {
			CInvPatientAllocationDlg dlg(this);
			dlg.m_nID = arUpdatedAllocationIDs[nAllocation];
			dlg.DoModal();
		}

		// (c.haag 2008-06-11 14:43) - PLID 28474 - Update inventory todo alarms
		InvUtils::UpdateOrderTodoAlarms(m_nOrderID);

		CDialog::OnOK();

		// (j.jones 2008-03-19 15:04) - PLID 29311 - Now that this order has been received,
		// try to create an allocation from it if it has a linked appointment.
		// Do not bother creating an allocation unless the order was fully received.
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (j.jones 2008-09-29 13:28) - PLID 30636 - we shouldn't be prompting if the receipt of the order
		// already updated any existing allocations
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(g_pLicense->HasCandAModule(CLicense::cflrSilent)
			&& m_nLinkedApptID != -1
			&& arUpdatedAllocationIDs.GetSize() == 0
			&& !bSomeItemsZero && !bAnyPartialDetails
			&& IDYES == MessageBox("This completed order has a linked appointment. Would you like to create an allocation with these products for that appointment?",
			"Practice", MB_ICONQUESTION|MB_YESNO)) {

			InvUtils::TryCreateAllocationFromOrder(m_nOrderID);
		}

	} NxCatchAllCall("Error in OnOK", if(nNewAuditID != -1) {	RollbackAuditTransaction(nNewAuditID);	});
}

void CReceiveOrderDlg::OnCancel() 
{
	try {

	} NxCatchAll("Error in OnCancel");

	CDialog::OnCancel();
}

BOOL CReceiveOrderDlg::DestroyWindow() 
{
	try {
		//No longer receive barcode scan messages
		GetMainFrame()->UnregisterForBarcodeScan(this);

		//We also need to iterate through the main datalist, and destroy any windows which were allocated
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow != NULL) {
			CProductItemsDlg *pDlg = (CProductItemsDlg*)VarLong(pRow->GetValue(elcDlgPtr));
			if(pDlg != NULL) {
				pDlg->DestroyWindow();
				delete pDlg;
				pRow->PutValue(elcDlgPtr, (long)0);
			}

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in DestroyWindow");

	return CDialog::DestroyWindow();
}

void CReceiveOrderDlg::LoadCurrentOrder()
{
	//Must have set the OrderID
	if(m_nOrderID <= 0) {
		AfxThrowNxException("Invalid OrderID %li when attempting to receive order.", m_nOrderID);
	}

	//
	//Select a recordset of all the products in this order.  We can't let the datalist do it because
	//	we want to split out the serialized items into individual lines.
	//2 queries, 1 for the order information, the other for the individual products
	// (c.haag 2007-12-03 11:29) - PLID 28204 - Replaced ForConsignment bit with ForStatus integer flag
	// (j.jones 2008-03-20 09:09) - PLID 29311 - added AppointmentID
	//TES 8/20/2008 - PLID 24726 - Added SerialNumIsLotNum
	_RecordsetPtr prs = CreateParamRecordset("SELECT OrderT.Date AS OrderDate, OrderT.TrackingID, OrderT.LocationID, "
		"PersonSupplierT.Company AS SupplierName, OrderAppointmentsT.AppointmentID "
		"FROM OrderT "
		"LEFT JOIN PersonT PersonSupplierT ON OrderT.Supplier = PersonSupplierT.ID "
		"LEFT JOIN OrderAppointmentsT ON OrderT.ID = OrderAppointmentsT.OrderID "
		"WHERE OrderT.ID = {INT};\r\n"

		"SELECT OrderDetailsT.ID, OrderDetailsT.ProductID, ProductT.HasSerialNum, ProductT.HasExpDate, ProductT.SerialNumIsLotNum, ServiceT.BarCode, "
		"CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,(QuantityOrdered / Convert(float,OrderDetailsT.Conversion))) ELSE QuantityOrdered END AS QuantityOrdered, "
		"ServiceT.Name AS ProductName, ProductT.UseUU, ProductT.Conversion, ProductT.SerialPerUO, OrderDetailsT.ForStatus "
		"FROM OrderT "
		"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
		"INNER JOIN ProductT ON OrderDetailsT.ProductID = ProductT.ID "
		"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
		"WHERE OrderDetailsT.Deleted = 0 AND OrderT.ID = {INT} AND OrderDetailsT.DateReceived IS NULL", m_nOrderID, m_nOrderID);

	if(!prs->eof) {
		//Should just be 1 record in the first
		CString strTrackingID = AdoFldString(prs, "TrackingID", "");
		// (j.jones 2008-07-03 16:28) - PLID 30609 - store this location ID
		m_nLocationID = AdoFldLong(prs, "LocationID", -1);
		CString strSupplier = AdoFldString(prs, "SupplierName", "");
		COleDateTime dtOrderDate = AdoFldDateTime(prs, "OrderDate");

		// (j.jones 2008-03-20 09:09) - PLID 29311 - added AppointmentID
		m_nLinkedApptID = AdoFldLong(prs, "AppointmentID", -1);

		//Set these at the top of the dialog
		SetDlgItemText(IDC_ORDER_PLACED_DATE, ConvertToControlText(FormatDateTimeForInterface(dtOrderDate, dtoDate)));
		SetDlgItemText(IDC_ORDER_PLACED_FROM, ConvertToControlText(strSupplier));
		SetDlgItemText(IDC_ORDER_TRACKING_NUM, ConvertToControlText(strTrackingID));
		m_pLocationList->SetSelByColumn(0, (long)m_nLocationID);
	}

	//move to the next recordset
	prs = prs->NextRecordset(NULL);

	//Loop over all of our product details
	while(prs != NULL && !prs->eof) {
		FieldsPtr pFlds = prs->Fields;

		long nProductID = AdoFldLong(pFlds, "ProductID");
		long nQty = AdoFldLong(pFlds, "QuantityOrdered", 0);
		BOOL bHasSerialNum = AdoFldBool(pFlds, "HasSerialNum", FALSE);
		BOOL bHasExpDate = AdoFldBool(pFlds, "HasExpDate", FALSE);
		BOOL bSerialNumIsLotNum = AdoFldBool(pFlds, "SerialNumIsLotNum", FALSE);
		CString strBarCode = AdoFldString(pFlds, "BarCode", "");
		CString strName = AdoFldString(pFlds, "ProductName", "");

		BOOL bUseUU = AdoFldBool(pFlds, "UseUU", FALSE);
		BOOL bSerialPerUO = AdoFldBool(pFlds, "SerialPerUO", FALSE);
		long nConversion = AdoFldLong(pFlds, "Conversion", 1);
		long nStatus = AdoFldLong(pFlds, "ForStatus", InvUtils::odsPurchased);

		long nOrderDetailID = AdoFldLong(pFlds, "ID");

		//
		//Fill in the datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
		pRow->PutValue(elcProductID, (long)nProductID);
		pRow->PutValue(elcBarCode, _bstr_t(strBarCode));
		pRow->PutValue(elcHasSerial, bHasSerialNum == FALSE ? g_cvarFalse : g_cvarTrue);
		pRow->PutValue(elcHasExp, bHasExpDate == FALSE ? g_cvarFalse : g_cvarTrue);
		pRow->PutValue(elcSerialNumIsLotNum, bSerialNumIsLotNum == FALSE ? g_cvarFalse : g_cvarTrue);
		pRow->PutValue(elcName, _bstr_t(strName));
		//DRT 1/8/2008 - PLID 28473 - Default the "receive to" to whatever the status is.  If they
		//	decide to change this in the dialog, we will then change this value later.
		pRow->PutValue(elcReceiveTo, _bstr_t(nStatus == 0 ? "Purchased Inv." : "Consignment"));
		pRow->PutValue(elcQtyOrdered, (long)nQty);
		pRow->PutValue(elcQtyReceived, (long)0);
		pRow->PutValue(elcDlgPtr, (long)0);
		pRow->PutValue(elcUseUU, bUseUU == FALSE ? g_cvarFalse : g_cvarTrue);
		pRow->PutValue(elcSerialPerUO, bSerialPerUO == FALSE ? g_cvarFalse : g_cvarTrue);
		pRow->PutValue(elcConversion, (long)nConversion);
		pRow->PutValue(elcDetailID, (long)nOrderDetailID);
		// (c.haag 2007-12-03 11:30) - PLID 28204 - Replaced elcConsignment bit with elcStatus integer flag
		pRow->PutValue(elcStatus, nStatus);
		UpdateColorsForRow(pRow);
		m_pList->AddRowSorted(pRow, NULL);

		prs->MoveNext();
	}
}

LRESULT CReceiveOrderDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-11-09 09:58) - PLID 27476 - Need to convert this correctly from a bstr
		_bstr_t bstr = (BSTR)lParam;
		CString strCode = (LPCTSTR)bstr;

		//Search through the datalist looking for this code, where the qty is NOT yet filled (in case of
		//	multi serialized items)
		//(c.copits 2010-09-14) PLID 40317 - Allow duplicate UPC codes for FramesData certification
		//NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindByColumn(elcBarCode, _bstr_t(strCode), NULL, VARIANT_FALSE);
		NXDATALIST2Lib::IRowSettingsPtr pRow = GetBestUPCProduct(strCode);

		if(pRow) {
			//Let the "on click" behavior handle it.  We have to set the cur sel to do that.
			m_pList->CurSel = pRow;

			HandleReceivedClickForCurrentRow(true);
		}
		else {
			//The row was not found... they scanned an item not in this order
		}

	} NxCatchAll("Error in OnBarcodeScan");

	return 0;
}

BEGIN_EVENTSINK_MAP(CReceiveOrderDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CReceiveOrderDlg)
	ON_EVENT(CReceiveOrderDlg, IDC_RECEIVE_ORDER_LIST, 19 /* LeftClick */, OnLeftClickReceiveOrderList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CReceiveOrderDlg, IDC_RECEIVE_ORDER_LOCATION, 1 /* SelChanging */, OnSelChangingReceiveOrderLocation, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReceiveOrderDlg, IDC_RECEIVE_ORDER_LOCATION, 16 /* SelChosen */, OnSelChosenReceiveOrderLocation, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CReceiveOrderDlg::OnLeftClickReceiveOrderList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		//When they click the received hyperlink, we want to pop up an appropriate message
		if(nCol == elcQtyReceived) {
			//Set this row to be the cur sel.  For some reason the datalist does not highlight when you click a hyperlinked cell
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			m_pList->CurSel = pRow;
			HandleReceivedClickForCurrentRow(false);
		}

	} NxCatchAll("Error in OnLeftClickReceiveOrderList");
}

void CReceiveOrderDlg::HandleReceivedClickForCurrentRow(bool bIsBarcode)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;

	if(pRow == NULL) {
		return;
	}

	//Is this a serialized or expirable item?  If it is not, then we just need a count, and go on our merry way
	if(VarBool(pRow->GetValue(elcHasSerial)) || VarBool(pRow->GetValue(elcHasExp)) != 0) {

		long nOrderDetailID = VarLong(pRow->GetValue(elcDetailID));

		//safety check: has another user already received these products?
		if (InvUtils::HasOrderDetailReceivedProductItems(nOrderDetailID)) {
			//someone has already received this product
			AfxMessageBox("This product has already been received. You may need to close this window and attempt receiving the order again to ensure the order status has been updated.", MB_ICONERROR | MB_OK);
			return;
		}

		CProductItemsDlg *pDlg = NULL;

		//First, see if we've previously created this dialog
		pDlg = (CProductItemsDlg*)VarLong(pRow->GetValue(elcDlgPtr));

		if(pDlg == NULL) {
			
			//The dialog does not yet exist.  Let us create it.
			pDlg = new CProductItemsDlg(this);
			pDlg->m_bModeless = true;
			pDlg->m_bUseCloseOnEntryType = true;

			//Initialize the dialog
			pDlg->m_bUseSerial = VarBool(pRow->GetValue(elcHasSerial));
			pDlg->m_bUseExpDate = VarBool(pRow->GetValue(elcHasExp));
			pDlg->m_ProductID = VarLong(pRow->GetValue(elcProductID));
			//TES 6/18/2008 - PLID 29578 - This now takes an OrderDetailID rather than an OrderID
			pDlg->m_nOrderDetailID = nOrderDetailID;
			pDlg->m_bUseUU = VarBool(pRow->GetValue(elcUseUU));
			pDlg->m_bSerialPerUO = VarBool(pRow->GetValue(elcSerialPerUO));
			pDlg->m_nConversion = VarLong(pRow->GetValue(elcConversion));

			//Calculate the qty for UU
			long nQty = VarLong(pRow->GetValue(elcQtyOrdered));
			if(pDlg->m_bUseUU) {
				nQty *= pDlg->m_nConversion;
			}
			pDlg->m_NewItemCount = nQty;

			// (a.walling 2008-02-15 12:22) - PLID 28946 - Hide the consignment info if not licensed
			// (c.haag 2007-12-03 11:32) - PLID 28204 - This used to be a consignment flag; now it's a general status integer
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			pDlg->m_nDefaultStatus = g_pLicense->HasCandAModule(CLicense::cflrSilent) ? VarLong(pRow->GetValue(elcStatus)) : InvUtils::odsPurchased;

			pDlg->m_EntryType = PI_ENTER_DATA;
			pDlg->m_bSaveDataEntryQuery = true;						//DRT 11/12/2007 - PLID 27999
			pDlg->m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!
			// (j.jones 2012-02-01 09:06) - PLID 34840 - do not declare the new Product Item ID
			pDlg->m_bDeclareNewProductItemID = FALSE;
			pDlg->m_bDisallowQtyChange = FALSE;						//new with PLID 28008 to use in entry dialogs
			pDlg->m_bDisallowLocationChange = TRUE;					// (c.haag 2008-06-25 11:31) - PLID 28438 - We only let users change locations from this dialog

			// (c.haag 2008-06-04 10:53) - PLID 29013 - Don't let the user select "< No Location >"
			pDlg->m_bDisallowNonLocations = TRUE;

			//Get the location
			long nLocationID = GetCurrentLocation();
			NXDATALIST2Lib::IRowSettingsPtr pLocationRow = m_pLocationList->CurSel;
			// (j.jones 2008-07-03 13:05) - PLID 30609 - check the location row, not the product row
			if(pLocationRow != NULL) {
				nLocationID = VarLong(pLocationRow->GetValue(0));
			}
			pDlg->m_nLocationID = nLocationID;

			//Save it back in the datalist
			pRow->PutValue(elcDlgPtr, (long)pDlg);

			//First time, do a modeless opening
			pDlg->Create(IDD_PRODUCT_ITEMS, this);
		}

		//DRT 11/20/2007 - PLID 28138 - Set this flag each time, they can change the checkbox
		if(IsDlgButtonChecked(IDC_SCAN_ONLY_MODE)) {
			pDlg->m_bCloseAfterScan = true;
		}
		else {
			pDlg->m_bCloseAfterScan = false;
		}

		//Regardless of new or existing, show the window
		pDlg->ShowWindow(SW_SHOW);
	}
	else {
		//This is not a serialized item.  There is a setting for whether they wish to just inc by 1 or to prompt.  This will allow offices
		//	to easily barcode every item that comes in (no counting means less mistakes), but still lets those who don't have a barcode
		//	to just enter in the number received.

		//Number ordered, for comparisons
		long nQtyOrdered = VarLong(pRow->GetValue(elcQtyOrdered));

		//Start off with the old qty
		long nNewQty = VarLong(pRow->GetValue(elcQtyReceived));
		if(bIsBarcode && IsDlgButtonChecked(IDC_RAD_RECEIVE_ORDER_BARCODE_INC)) {
			//Just increment the quantity, if it won't max us out
			if(nNewQty + 1 > nQtyOrdered) {
				MessageBox("You have attempted to scan an item that will increase the total quantity to be higher than what was ordered.  If you received "
					"more items than ordered, you will need to either edit the order to reflect the new quantity, or create a new order which contains the new items.", "Practice");
				return;
			}

			//otherwise just increment it
			nNewQty++;
		}
		else {
			//We want to prompt the user to enter the qty

			//Get the item name for message boxes
			CString strName = VarString(pRow->GetValue(elcName));

			CString strResult;
			//Preset the value to whatever is currently in the list.
			strResult.Format("%li", nNewQty);
			if(InputBox(this, "What quantity of '" + strName + "' was received?", strResult, "", false, false, NULL, TRUE) == IDOK) {
				nNewQty = atoi(strResult);

				//The qty cannot be higher than what was ordered.  If you really got more in than you ordered, you need to go edit 
				//	the order and add an extra item to it first.  I think this is rare enough we don't need a whole bunch of stuff here for it.
				//	There are too many other things to think about to just add it to the order from here -- total qty, amount, taxes, etc.
				if(nNewQty > nQtyOrdered) {
					MessageBox("You may not enter a quantity that is higher than what was ordered.  If you received more items than ordered, you "
						"will need to either edit the order to reflect the new quantity, or create a new order which contains the new items.", "Practice");
					return;
				}
			}
			else {
				//Did not press OK, do nothing
			}
		}

		//Set this in the datalist
		pRow->PutValue(elcQtyReceived, (long)nNewQty);

		UpdateColorsForRow(pRow);
	}
}

void CReceiveOrderDlg::UpdateColorsForRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(pRow == NULL)
		return;

	//Get the current values
	long nOrdered = VarLong(pRow->GetValue(elcQtyOrdered));
	long nReceived = VarLong(pRow->GetValue(elcQtyReceived));

	//If not enough, it's marked one way.
	if(nReceived < nOrdered) {
		pRow->PutForeColor(COLOR_UNDER);
	}
	else {
		//default color
		pRow->PutForeColor(COLOR_OK);
	}

}

void CReceiveOrderDlg::OnSelChangingReceiveOrderLocation(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//Don't allow blank selections
		if(lppNewSel == NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pOldSel(lpOldSel);
			m_pLocationList->CurSel = pOldSel;
		}

	} NxCatchAll("Error in OnSelChangingReceiveOrderLocation");
}

LRESULT CReceiveOrderDlg::OnPostEditProductItemsDlg(WPARAM wParam, LPARAM lParam)
{
	try {
		//The ProductItemsDlg has been closed, we want to update our list of quantities with whatever they filled in.
		CProductItemsDlg *pDlg = (CProductItemsDlg*)wParam;
		if(pDlg != NULL) {
			//Search for it in the datalist
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->FindByColumn(elcDlgPtr, (long)pDlg, NULL, VARIANT_FALSE);
			if(pRow != NULL) {
				//This is the row.  Get the qty from the dialog
				long nNewQty = pDlg->CountValidRows();

				//Is this a UU item?  We need to divide it down if so.  The dialog forces us to have even multiples, so we only
				//	need worry about full int quantities.
				BOOL bUseUU = VarBool(pRow->GetValue(elcUseUU), FALSE);
				BOOL bSerialPerUO = VarBool(pRow->GetValue(elcSerialPerUO), FALSE);
				if(bUseUU && !bSerialPerUO) {
					nNewQty /= VarLong(pRow->GetValue(elcConversion), 1);
				}

				//Update our datalist
				pRow->PutValue(elcQtyReceived, (long)nNewQty);

				//DRT 1/8/2008 - PLID 28473 - We need to determine the status where items are going.
				long nNewItemStatuses = pDlg->GetLabelForItemStatus();
				CString strText;
				switch(nNewItemStatuses) {
				case -1:
					strText = "Multiple";
					break;
				case 0:
					strText = "Purchased Inv.";
					break;
				case 1:
					strText = "Consignment";
					break;
				default:
					//Just in case / default handler for new types
					strText = "<Unknown>";
					break;
				}
				pRow->PutValue(elcReceiveTo, _bstr_t(strText));

				//Update colors
				UpdateColorsForRow(pRow);
			}
			else {
				//Hrm, the dialog wasn't found?  I'm not sure what would cause this.
				ASSERT(FALSE);
			}
		}
		else {
			//Dialog is NULL?  We must be getting a bad message from someone.  Alert them if debugging.
			ASSERT(FALSE);
		}

	} NxCatchAll("Error in OnPostEditProductItemsDlg");

	return 0;
}

void CReceiveOrderDlg::OnRadReceiveOrderBarcodeInc() 
{
	try {
		//Save this setting - 0 for increment, 1 for prompt
		SetRemotePropertyInt("ReceiveOrderBarcodePref", 0, 0, GetCurrentUserName());

	} NxCatchAll("Error in OnRadReceiveOrderBarcodeInc");
}

void CReceiveOrderDlg::OnRadReceiveOrderBarcodePrompt() 
{
	try {
		//Save this setting - 0 for increment, 1 for prompt
		SetRemotePropertyInt("ReceiveOrderBarcodePref", 1, 0, GetCurrentUserName());

	} NxCatchAll("Error in OnRadReceiveOrderBarcodePrompt");
}

//DRT 11/20/2007 - PLID 28138 - New ability to scan once and have the dialog close.
void CReceiveOrderDlg::OnScanOnlyMode() 
{
	try {
		//Save this setting 
		SetRemotePropertyInt("ReceiveOrderScanOnlyMode", IsDlgButtonChecked(IDC_SCAN_ONLY_MODE), 0, GetCurrentUserName());

	} NxCatchAll("Error in OnScanOnlyMode");
}

void CReceiveOrderDlg::OnTimer(UINT nIDEvent) 
{
	try {
		switch(nIDEvent) {
		//DRT 1/4/2008 - PLID 28235 - See notes in OnInitDialog
		case TIMER_AUTO_BARCODE_CHECK:
			{
				KillTimer(TIMER_AUTO_BARCODE_CHECK);
				if(m_bstrFromBarcode != NULL) {
					PostMessage(WM_BARCODE_SCAN, 0, (LPARAM)m_bstrFromBarcode);

					//Reset the member to NULL, just in case something bizarrely triggers this timer again
					m_bstrFromBarcode = NULL;
				}
			}
			break;
		}


	} NxCatchAll("Error in OnTimer");

	CDialog::OnTimer(nIDEvent);
}

void CReceiveOrderDlg::OnSelChosenReceiveOrderLocation(LPDISPATCH lpRow) 
{
	try {
		// (c.haag 2008-06-25 11:22) - PLID 28438 - Make sure the location ID of every
		// product items dialog is consistent with the new selction
		long nLocationID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			nLocationID = VarLong(pRow->Value[0], -1);
			if (nLocationID > -1) {
				pRow = m_pList->GetFirstRow();
				while (NULL != pRow) {
					CProductItemsDlg *pDlg = (CProductItemsDlg*)VarLong(pRow->GetValue(elcDlgPtr));
					if (NULL != pDlg) {
						pDlg->ChangeLocationID(nLocationID); // Jackpot
					} else {
						// No dialog
					}
					pRow = pRow->GetNextRow();
				}
			} else {
				// Invalid location -- should never happen

				// (j.jones 2008-07-03 16:27) - PLID 30609 - force a selection, using the order's location
				pRow = m_pLocationList->SetSelByColumn(0, (long)m_nLocationID);
				if(pRow == NULL) {
					//attempt to set to the current location
					pRow = m_pLocationList->SetSelByColumn(0, (long)GetCurrentLocationID());
				}

				if(pRow) {
					nLocationID = VarLong(pRow->Value[0], -1);
				}
			}
		} else {
			// (j.jones 2008-07-03 16:27) - PLID 30609 - force a selection, using the order's location
			pRow = m_pLocationList->SetSelByColumn(0, (long)m_nLocationID);
			if(pRow == NULL) {
				//attempt to set to the current location
				pRow = m_pLocationList->SetSelByColumn(0, (long)GetCurrentLocationID());
			}

			if(pRow) {
				nLocationID = VarLong(pRow->Value[0], -1);
			}
		}

		if (nLocationID > -1) {
			pRow = m_pList->GetFirstRow();
			while (NULL != pRow) {
				CProductItemsDlg *pDlg = (CProductItemsDlg*)VarLong(pRow->GetValue(elcDlgPtr));
				if (NULL != pDlg) {
					pDlg->ChangeLocationID(nLocationID); // Jackpot
				} else {
					// No dialog
				}
				pRow = pRow->GetNextRow();
			}
		}
		else {
			ASSERT(FALSE);
		}
	}
	NxCatchAll("Error in CReceiveOrderDlg::OnSelChosenReceiveOrderLocation");
}

void CReceiveOrderDlg::OnMarkAllReceived()
{
	try {
		// (d.thompson 2009-08-20) - PLID 31884
		//This button attempts to mark all the non-serialized items as received.  Some offices just don't care to
		//	do in depth tracking, or they may just count all the items manually and not want to type it all in.
		bool bAtLeastOneHasSerial = false;

		if(AfxMessageBox("This will automatically set the 'Received' quantity equal to the 'Ordered' quantity for all "
			"items which do not require a serial number to be entered.  Are you sure you wish to do this?", MB_YESNO) != IDYES)
		{
			return;
		}

		//Iterate through all the items to receive, and for anything that is not serialized, set the received amount
		//	equal to the due amount.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow != NULL) {

			if(VarBool(pRow->GetValue(elcHasSerial)) || VarBool(pRow->GetValue(elcHasExp))) {
				bAtLeastOneHasSerial = true;
			}
			else {
				//This is not a serialized item.  Thus, just set the received == due

				//Number due
				long nQtyOrdered = VarLong(pRow->GetValue(elcQtyOrdered));

				//Set it right back in the datalist
				pRow->PutValue(elcQtyReceived, (long)nQtyOrdered);

				//Fix any coloring
				UpdateColorsForRow(pRow);
			}

			pRow = pRow->GetNextRow();
		}

		//If any item is serialized, we're just going to park here and they have to receive them
		//	as normal.  Otherwise, this function should save everything as we've left it and close out.
		if(!bAtLeastOneHasSerial) {
			OnOK();
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-09-14) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns an IRowSettingsPtr corresponding to the first found UPC order.

NXDATALIST2Lib::IRowSettingsPtr CReceiveOrderDlg::GetBestUPCProduct(CString strCode) 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	try {
		pRow = m_pList->FindByColumn(elcBarCode, _bstr_t(strCode), NULL, VARIANT_FALSE);
	} NxCatchAll(__FUNCTION__);

	return pRow;
}