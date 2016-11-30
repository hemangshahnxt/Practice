// InvAllocationUsageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvAllocationUsageDlg.h"
#include "barcode.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "GlobalAuditUtils.h"
#include "GlobalSchedUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-11-27 09:40) - PLID 28196 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CInvAllocationUsageDlg dialog

enum AllocationListColumns {

	alcDetailObjectPtr = 0,
	alcProductID,
	alcProductItemID,
	alcProductName,
	alcQuantity,
	alcSerialNum,
	alcExpDate,
	alcProductType, // (c.haag 2008-03-11 14:11) - PLID 29255 - Product type (Consignment / Purchased Inv.)
	alcUsed,
	alcReleased,
	alcReturn,		//TES 6/19/2008 - PLID 26152 - Return this item.
	alcBilled,
	alcDisabled,
	alcNotes,
	alcIsSerialized,//TES 7/19/2008 - PLID 29478 - Hidden column, needed now that ProductItemID can be -1 for serialized items.
};

CInvAllocationUsageDlg::CInvAllocationUsageDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvAllocationUsageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvAllocationUsageDlg)
		m_pAllocationMasterInfo = NULL;
		m_nAllocationID = -1;
		m_bFreeInfoObject = TRUE;
		m_bSaveToData = TRUE;
		m_nInitialProductID = -1;
		m_strInitialProductName = "";
		m_dblInitialQuantity = 1.0;
		m_bForceCompletion = TRUE;
		m_bIsCompletedByBill = FALSE;
		m_bIsCompletedByCaseHistory = FALSE;
		m_nInitialAutoUseDetailID = -1;
		m_parInitialAutoUseProductItemIDs = NULL;
		m_bDisableBarcode = FALSE;
	//}}AFX_DATA_INIT
}


void CInvAllocationUsageDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvAllocationUsageDlg)
	DDX_Control(pDX, IDC_BTN_SAVE_LEAVE_UNCOMPLETED, m_btnSaveLeaveUncompleted);
	DDX_Control(pDX, IDC_BTN_SAVE_AND_COMPLETE, m_btnSaveAndComplete);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_BARCODE_RELEASED, m_radioBarcodeReleased);
	DDX_Control(pDX, IDC_RADIO_BARCODE_USED, m_radioBarcodeUsed);
	DDX_Control(pDX, IDC_ALLOC_PATIENT_NAME, m_nxeditAllocPatientName);
	DDX_Control(pDX, IDC_LOCATION_NAME, m_nxeditLocationName);
	DDX_Control(pDX, IDC_CREATE_DATE, m_nxeditCreateDate);
	DDX_Control(pDX, IDC_APPT_INFO, m_nxeditApptInfo);
	DDX_Control(pDX, IDC_ALLOCATION_NOTES, m_nxeditAllocationNotes);
	DDX_Control(pDX, IDC_HEADER_LABEL, m_nxstaticHeaderLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvAllocationUsageDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvAllocationUsageDlg)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_RADIO_BARCODE_USED, OnRadioBarcodeUsed)
	ON_BN_CLICKED(IDC_RADIO_BARCODE_RELEASED, OnRadioBarcodeReleased)
	ON_BN_CLICKED(IDC_BTN_SAVE_AND_COMPLETE, OnBtnSaveAndComplete)
	ON_BN_CLICKED(IDC_BTN_SAVE_LEAVE_UNCOMPLETED, OnBtnSaveLeaveUncompleted)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvAllocationUsageDlg message handlers

BOOL CInvAllocationUsageDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-29 10:46) - PLID 29820 - NxIconify the buttons
		m_btnSaveLeaveUncompleted.AutoSet(NXB_OK);
		m_btnSaveAndComplete.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));

		//TES 6/23/2008 - PLID 26152 - We've got two preferences on this dialog now, let's go ahead and bulk cache.
		g_propManager.CachePropertiesInBulk("InvAllocationUsage", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AllocationUsage_BarcodeUsedItems' OR "
			"Name = 'ReturnReleasedGeneralSerializedItems' "
			")",
			_Q(GetCurrentUserName()));

		//check the last barcode option this user had set
		BOOL bBarcodeUsedItems = GetRemotePropertyInt("AllocationUsage_BarcodeUsedItems", 1, 0, GetCurrentUserName(), true) == 1;
		if(bBarcodeUsedItems) {
			m_radioBarcodeUsed.SetCheck(TRUE);
		}
		else {
			m_radioBarcodeReleased.SetCheck(TRUE);
		}

		//register for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
				MsgBox("Error registering for barcode scans.  You may not be able to scan.");
			}
		}

		// (j.jones 2008-02-19 15:50) - PLID 28948 - disable the "save and leave uncompleted" option
		// when accessed from a bill or case history
		if(m_bIsCompletedByCaseHistory || m_bIsCompletedByBill) {
			GetDlgItem(IDC_BTN_SAVE_LEAVE_UNCOMPLETED)->EnableWindow(FALSE);
		}

		//update the header appropriately
		CString strHeader;
		if(m_nInitialProductID != -1) {

			if(!m_bIsCompletedByCaseHistory && !m_bIsCompletedByBill) {
				//this should not be possible, means the dialog was called poorly
				ASSERT(FALSE);
			}

			strHeader.Format("Please select a product from this allocation to represent the\n"
				"'%s' product that is being added to the %s.",
				m_strInitialProductName, m_bIsCompletedByCaseHistory ? "case history" : "bill");

			//also put this product name in the title bar
			CString strTitle;
			strTitle.Format("Inventory Allocation - %s", m_strInitialProductName);
			SetWindowText(strTitle);
		}
		else if(m_bForceCompletion) {
			CString strUsedOn = " by the patient";
			if(m_bIsCompletedByBill) {
				strUsedOn = " on this bill";
			}
			else if(m_bIsCompletedByCaseHistory) {
				strUsedOn = " on this case history";
			}

			strHeader.Format("Please review the list of remaining products in this allocation and determine\n"
				"whether they will be used%s or released to purchased stock.", strUsedOn);
		}
		else {

			if(!m_bIsCompletedByCaseHistory && !m_bIsCompletedByBill) {
				//this should not be possible, means the dialog was called poorly
				ASSERT(FALSE);
			}

			strHeader.Format("Please select a product from this allocation to add to the %s.",
				m_bIsCompletedByCaseHistory ? "case history" : "bill");
		}
		SetDlgItemText(IDC_HEADER_LABEL, strHeader);

		m_AllocationList = BindNxDataList2Ctrl(this, IDC_ALLOCATED_ITEMS_LIST, GetRemoteData(), false);

		//LoadAllocationInfo will handle populating the memory object (if needed)
		//and then populate data on the screen accordingly
		if(!LoadAllocationInfo()) {
			//if FALSE was returned, we would have already warned and
			//closed the dialog, so just leave this function silently
			return TRUE;
		}

		if(m_pAllocationMasterInfo && m_pAllocationMasterInfo->iasStatus == InvUtils::iasCompleted) {
			//if the allocation is already completed, then we can't edit the
			//used & released status on this screen - barcoding would only be
			//for the "billed" status, so disable the radio buttons
			m_radioBarcodeUsed.EnableWindow(FALSE);
			m_radioBarcodeReleased.EnableWindow(FALSE);
		}

		// (j.jones 2008-06-11 15:33) - PLID 28379 - try to auto-select a given detail
		//TES 7/16/2008 - PLID 27983 - Also try to auto-fill from a list of ProductItemIDs
		if(m_nInitialAutoUseDetailID != -1 || m_parInitialAutoUseProductItemIDs != NULL) {
			TryResolveInitialDetailID();
		}
		//if we were given an initial product ID, try to "use" it if possible
		else if(m_nInitialProductID != -1) {
			TryResolveInitialProductID();
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CInvAllocationUsageDlg::~CInvAllocationUsageDlg()
{
	try {

		//unregister for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

		//deallocate m_pAllocationMasterInfo here only if m_bFreeInfoObject is TRUE
		if(m_bFreeInfoObject && m_pAllocationMasterInfo) {

			//use the shared function for freeing this object
			InvUtils::FreeAllocationMasterInfoObject(m_pAllocationMasterInfo);
			m_pAllocationMasterInfo = NULL;
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::~CInvAllocationUsageDlg");	
}

// (j.jones 2008-02-19 16:17) - PLID 28948 - save the allocation as completed
BOOL CInvAllocationUsageDlg::SaveCompleted()
{
	long nAuditTransactionID = -1;

	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			ThrowNxException("CInvAllocationUsageDlg::SaveCompleted called with empty allocation data!");
		}

		if(!m_bSaveToData) {
			//this function should not have been called
			ASSERT(FALSE);
			ThrowNxException("CInvAllocationUsageDlg::SaveCompleted called when data should not have been saved!");
		}

		//if editing an existing allocation, disallow a mix of "allocated" and
		//"used" details in the same allocation, and never allow saving
		//details without a status
		BOOL bHasUsedProducts = FALSE;
		BOOL bHasReleasedProducts = FALSE;
		BOOL bHasProductsWithoutAStatus = FALSE;

		int i = 0;
		for(i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)(m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i));
			if(pInfo) {

				//we just need to know if we have any products with a given status
				InvUtils::InventoryAllocationDetailStatus iadsStatus = pInfo->iadsCurrentStatus;

				if(iadsStatus == InvUtils::iadsUsed) {
					bHasUsedProducts = TRUE;
				}
				else if(iadsStatus == InvUtils::iadsReleased) {
					bHasReleasedProducts = TRUE;
				}
				else {
					//shouldn't have been possible, means our
					//VerifyAllocationCompletedOnClose function failed
					ASSERT(FALSE);
					bHasProductsWithoutAStatus = TRUE;
				}
			}
		}

		//if any products do not have a status, disallow saving
		if(bHasProductsWithoutAStatus) {
			//shouldn't have been possible, means our
			//VerifyAllocationCompletedOnClose function failed
			ASSERT(FALSE);
			AfxMessageBox("This allocation has products that have not been marked as 'Used', or 'Released'.\n\n"
				"An allocation cannot be completed unless each product has a status chosen.");
			return FALSE;
		}

		//if we only have released products, warn
		if(!bHasUsedProducts && bHasReleasedProducts) {
			
			// (j.jones 2008-03-12 11:47) - PLID 29102 - we can't allow this if it is linked to a case history			
			if(m_pAllocationMasterInfo->nCaseHistoryID != -1 || m_bIsCompletedByCaseHistory) {
				AfxMessageBox("This completed allocation is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n"
					"However, this allocation is linked to a case history. A completely released allocation cannot be linked to a case history.\n\n"
					"You must either edit the allocation or case history to unlink the two, or leave a product as not released.");
				return FALSE;
			}
			else {			
				if(IDNO == MessageBox("This completed allocation is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n\n"
					"Are you sure you wish to complete this allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}

		CString strSqlBatch = BeginSqlBatch();

		// (j.jones 2008-02-19 17:54) - PLID 28948 - this function now takes in a parameter to determine
		// if the allocation should be saved as completed, and in this case we are completing it
		if(!InvUtils::GenerateAllocationSaveSql(TRUE, m_pAllocationMasterInfo, strSqlBatch, nAuditTransactionID)) {

			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}

			return FALSE;
		}

		ExecuteSqlBatch(strSqlBatch);
	
		// (j.jones 2007-11-29 12:01) - PLID 28043 - supported auditing
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		// (c.haag 2008-02-29 16:25) - PLID 29115 - Update inventory todo alarms
		const long nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
		try {
			InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, m_pAllocationMasterInfo->nAllocationID);
			//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
			InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
		}
		NxCatchAllSilentCallThrow(InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID));

		// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
		if(m_pAllocationMasterInfo->nAppointmentID != -1) {
			TrySendAppointmentTablecheckerForInventory(m_pAllocationMasterInfo->nAppointmentID, TRUE);
		}

		return TRUE;

	}NxCatchAllCall("Error in CInvAllocationUsageDlg::SaveCompleted",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return FALSE;
}

// (j.jones 2008-02-19 16:17) - PLID 28948 - save the allocation as uncompleted
BOOL CInvAllocationUsageDlg::SaveUncompleted()
{
	long nAuditTransactionID = -1;

	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			ThrowNxException("CInvAllocationUsageDlg::SaveUncompleted called with empty allocation data!");
		}

		if(!m_bSaveToData) {
			//this function should not have been called
			ASSERT(FALSE);
			ThrowNxException("CInvAllocationUsageDlg::SaveUncompleted called when data should not have been saved!");
		}

		if(IDNO == MessageBox("This action will save the allocation but not complete it. Are you sure you wish to do this?", "Practice", MB_YESNO|MB_ICONQUESTION)) {
			return FALSE;
		}

		CString strSqlBatch = BeginSqlBatch();

		// (j.jones 2008-02-19 17:54) - PLID 28948 - this function now takes in a parameter to determine
		// if the allocation should be saved as completed, and in this case we are not completing it
		if(!InvUtils::GenerateAllocationSaveSql(FALSE, m_pAllocationMasterInfo, strSqlBatch, nAuditTransactionID)) {

			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}

			return FALSE;
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}
	
		// (j.jones 2007-11-29 12:01) - PLID 28043 - supported auditing
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		// (c.haag 2008-02-29 16:25) - PLID 29115 - Update inventory todo alarms
		const long nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
		try {
			InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, m_pAllocationMasterInfo->nAllocationID);
			//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
			InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
		}
		NxCatchAllSilentCallThrow(InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID));

		// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
		if(m_pAllocationMasterInfo->nAppointmentID != -1) {
			TrySendAppointmentTablecheckerForInventory(m_pAllocationMasterInfo->nAppointmentID, TRUE);
		}

		return TRUE;

	}NxCatchAllCall("Error in CInvAllocationUsageDlg::SaveUncompleted",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return FALSE;
}


void CInvAllocationUsageDlg::OnCancel() 
{
	try {

		CString strWarn;
		if(m_bForceCompletion) {
			strWarn.Format("If you cancel, your changes will not be saved and the allocation will not be completed.\n\n"
				"Are you sure you wish to cancel the changes to this allocation?");
		}
		else {
			//otherwise, warn that the requested product won't be satisfied
			if(m_nInitialProductID != -1) {
				strWarn.Format("Are you sure you do not want to pull the '%s' product from this allocation?", m_strInitialProductName);
			}
			else {
				strWarn.Format("Are you sure you do not want to pull any products from this allocation?");
			}
		}

		if(IDNO == MessageBox(strWarn, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		// (j.jones 2007-12-21 11:33) - PLID 27988 - if they are actually cancelling,
		// we will undo anything they may have marked as "billed", but don't undo anything else
		for(int i=m_paryDetailsBilled.GetSize()-1;i>=0;i--) {
			InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)m_paryDetailsBilled.GetAt(i);
			pDetail->bBilled = FALSE;
			//remove from the list (do not delete!)
			m_paryDetailsBilled.RemoveAt(i);
		}

		CDialog::OnCancel();

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnCancel");
}

//SetInitialProductInfo takes in a product ID, name, and optional quantity and populates the
//"Initial" member variables for all three
// (j.jones 2008-06-11 15:31) - PLID 28379 - added ability to force selection of a detail
//TES 7/16/2008 - PLID 27983 - Added an optional list of ProductItemIDs to pre-fill as "used" on the allocation.
void CInvAllocationUsageDlg::SetInitialProductInfo(long nProductID, CString strProductName, double dblQuantity /*= 1.0*/, long nAutoUseDetailID /*= -1*/, OPTIONAL IN CArray<long,long> *parAutoUseProductItemIDs /*= NULL*/)
{
	try {

		m_nInitialProductID = nProductID;
		m_strInitialProductName = strProductName;
		m_dblInitialQuantity = dblQuantity;
		m_nInitialAutoUseDetailID = nAutoUseDetailID;
		m_parInitialAutoUseProductItemIDs = parAutoUseProductItemIDs;

	}NxCatchAll("Error in CInvAllocationUsageDlg::SetInitialProductInfo");
}

//SetAllocationInfo takes in either a pointer to allocation info, or an ID
void CInvAllocationUsageDlg::SetAllocationInfo(InvUtils::AllocationMasterInfo *pAllocationMasterInfo)
{
	try {

		//this should never be already filled at the time this is called
		ASSERT(m_pAllocationMasterInfo == NULL);

		m_pAllocationMasterInfo = pAllocationMasterInfo;

		//if the caller passed in a pointer, then we shouldn't attempt
		//to free it ourselves
		m_bFreeInfoObject = FALSE;

		//update the ID
		if(m_pAllocationMasterInfo) {
			m_nAllocationID = m_pAllocationMasterInfo->nAllocationID;
		}
		else {
			m_nAllocationID = -1;
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::SetAllocationInfo (Pointer)");
}

void CInvAllocationUsageDlg::SetAllocationInfo(long nAllocationID)
{
	try {

		//this should never be already filled at the time this is called
		ASSERT(m_pAllocationMasterInfo == NULL);

		m_nAllocationID = nAllocationID;
		m_pAllocationMasterInfo = NULL;

		//TryPopulateAllocationInfo will later fill m_pAllocationMasterInfo

	}NxCatchAll("Error in CInvAllocationUsageDlg::SetAllocationInfo (ID)");
}

//GetAllocationInfo returns the pointer to the allocation info
InvUtils::AllocationMasterInfo* CInvAllocationUsageDlg::GetAllocationInfo()
{
	try {

		//this should never be NULL by the time this is called
		ASSERT(m_pAllocationMasterInfo != NULL);

		return m_pAllocationMasterInfo;

	}NxCatchAll("Error in CInvAllocationUsageDlg::GetAllocationInfo");

	return NULL;
}

//TryPopulateAllocationInfo will populate m_pAllocationMasterInfo based on m_nAllocationID,
//if m_nAllocationID is not -1 and m_pAllocationMasterInfo is NULL
void CInvAllocationUsageDlg::TryPopulateAllocationInfo()
{
	try {
		
		//if already filled, then we don't need to do anything here
		if(m_pAllocationMasterInfo) {
			return;
		}

		//if m_pAllocationMasterInfo is NULL, then m_nAllocationID should never be -1!
		if(m_nAllocationID == -1) {
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2008-02-20 09:23) - PLID 28948 - added a flag for whether to include released items (set to FALSE)
		InvUtils::PopulateAllocationInfo(m_nAllocationID, m_pAllocationMasterInfo, FALSE);

	}NxCatchAll("Error in CInvAllocationUsageDlg::TryPopulateAllocationInfo");
}

//LoadAllocationInfo will call TryPopulateAllocationInfo,
//and then reflect the contents of m_pAllocationMasterInfo on screen
//(returns FALSE if it couldn't be loaded)
BOOL CInvAllocationUsageDlg::LoadAllocationInfo()
{
	try {

		//populate our m_pAllocationMasterInfo object, if not already populated
		TryPopulateAllocationInfo();

		//it should be impossible for m_pAllocationMasterInfo to be NULL at this point,
		//unless in the rare case the record was deleted prior to our load
		if(m_pAllocationMasterInfo == NULL) {
			AfxMessageBox("The allocation could not be loaded. Another user could have been deleted this allocation.");
			CDialog::OnCancel();
			return FALSE;
		}

		//now load the contents of m_pAllocationMasterInfo to the screen
		SetDlgItemText(IDC_ALLOC_PATIENT_NAME, m_pAllocationMasterInfo->strPatientName);
		SetDlgItemText(IDC_LOCATION_NAME, m_pAllocationMasterInfo->strLocationName);
		SetDlgItemText(IDC_CREATE_DATE, FormatDateTimeForInterface(m_pAllocationMasterInfo->dtInputDate, NULL, dtoDate));
		SetDlgItemText(IDC_APPT_INFO, m_pAllocationMasterInfo->strAppointmentDesc);
		SetDlgItemText(IDC_ALLOCATION_NOTES, m_pAllocationMasterInfo->strNotes);

		//and now the details
		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)(m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i));
			if(pInfo) {

				AddDatalistRowFromDetail(pInfo);
			}
		}

		// (j.jones 2008-02-29 12:36) - PLID 29126 - hid this column when completed by a case history

		//hide the "Billed" column if we are completing independently of a bill
		if(!m_bIsCompletedByBill) {
			m_AllocationList->GetColumn(alcBilled)->PutStoredWidth(0);
		}

		return TRUE;

	}NxCatchAll("Error in CInvAllocationUsageDlg::LoadAllocationInfo");

	return FALSE;
}

//takes in an AllocationDetailInfo pointer and adds a new datalist row for it
void CInvAllocationUsageDlg::AddDatalistRowFromDetail(InvUtils::AllocationDetailInfo *pInfo, BOOL bDisableIfBilled /*= TRUE*/)
{
	try {

		if(pInfo == NULL) {
			//should never have been called this way
			ASSERT(FALSE);
			return;
		}

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		IRowSettingsPtr pRow = m_AllocationList->GetNewRow();

		pRow->PutValue(alcDetailObjectPtr, (long)pInfo);
		pRow->PutValue(alcProductID, pInfo->nProductID);
		pRow->PutValue(alcProductItemID, pInfo->nProductItemID);
		pRow->PutValue(alcProductName, _bstr_t(pInfo->strProductName));
		pRow->PutValue(alcQuantity, pInfo->dblCurQuantity);
		pRow->PutValue(alcSerialNum, pInfo->varSerialNum);
		pRow->PutValue(alcExpDate, pInfo->varExpDate);
		// (c.haag 2008-03-11 14:09) - PLID 29255 - Include product item status
		switch ((InvUtils::ProductItemStatus)VarLong(pInfo->varProductItemStatus, -1)) {
		case InvUtils::pisPurchased:
			pRow->PutValue(alcProductType, "Purchased Inv.");
			break;
		case InvUtils::pisConsignment:
			pRow->PutValue(alcProductType, "Consignment");
			break;
		case InvUtils::pisWarranty:
			pRow->PutValue(alcProductType, "Warranty");
			break;
		default:
			if (VarLong(pInfo->varProductItemStatus, -1) <= 0) {
				// If there is no valid product item ID, it must be a non-serializeable product.
				// Call it purchased inventory.
				pRow->PutValue(alcProductType, "Purchased Inv.");
			} else {
				// Unexpected value; leave null
				pRow->PutValue(alcProductType, g_cvarNull);
			}
			break;
		}		
		pRow->PutValue(alcUsed, pInfo->iadsCurrentStatus == InvUtils::iadsUsed);
		pRow->PutValue(alcReleased, pInfo->iadsCurrentStatus == InvUtils::iadsReleased);

		//TES 6/20/2008 - PLID 26152 - If this is a released, serialized, purchased inventory item, then load the ToBeReturned
		// flag, otherwise leave the column blank.
		if(pInfo->iadsCurrentStatus != InvUtils::iadsReleased || 
			(InvUtils::ProductItemStatus)VarLong(pInfo->varProductItemStatus, -1) != InvUtils::pisPurchased ||
			pInfo->nProductItemID == -1) {
			pRow->PutValue(alcReturn, g_cvarNull);
		}
		else {
			pRow->PutValue(alcReturn, pInfo->bToBeReturned == TRUE);
		}
		_variant_t varBilled;
		// (j.jones 2008-01-07 16:49) - PLID 28479 - force varBilled to be NULL if the product
		// is not billable, or if completed & released
		if((m_pAllocationMasterInfo->iasStatus == InvUtils::iasCompleted &&
			pInfo->iadsCurrentStatus == InvUtils::iadsReleased) || !pInfo->bIsProductBillable) {
			//if the allocation was already completed, "billed"
			//is not an option on released items
			varBilled = g_cvarNull;
		}
		else {
			//load the billed status
			varBilled.vt = VT_BOOL;
			varBilled.boolVal = pInfo->bBilled;
		}
		pRow->PutValue(alcBilled, varBilled);
		pRow->PutValue(alcNotes, _bstr_t(pInfo->strNotes));

		//if bDisableIfBilled is TRUE, and we are completing a bill,
		//then see if the allocation has been billed, in which case we can't modify
		//the row anymore
		if(bDisableIfBilled && m_bIsCompletedByBill && pInfo->bBilled && pInfo->bIsProductBillable) {
			pRow->PutValue(alcDisabled, g_cvarTrue);
			pRow->PutForeColor(RGB(127,127,127));
		}
		else {
			pRow->PutValue(alcDisabled, g_cvarFalse);
		}

		//TES 7/18/2008 - PLID 29478 - Fill in whether this product is serialized.
		pRow->PutValue(alcIsSerialized, pInfo->bIsSerialized?g_cvarTrue:g_cvarFalse);

		m_AllocationList->AddRowSorted(pRow, NULL);

	}NxCatchAll("Error in CInvAllocationUsageDlg::AddDatalistRowFromDetail");
}

LRESULT CInvAllocationUsageDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		//TES 7/18/2008 - PLID 29478 - We may not be processing barcode scans at the moment.
		if(m_bDisableBarcode) {
			return 0;
		}

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return 0;
		}

		//barcode scanning in this screen is limited to serial numbers only

		//if the product is in the list, mark it as used,
		//but if the product is different from a previously selected
		//product, and we're based on an m_nInitialProductID,
		//warn and do not select

		//if the product is NOT in the list, check the database
		//to see if the product is in a different allocation,
		//and warn accordingly

		_bstr_t bstr = (BSTR)lParam;
		_variant_t varBarcode(bstr);

		//check whether they want to have the barcode scan auto-check Used, or Released
		BOOL bBarcodeUsedItems = TRUE;

		if(m_radioBarcodeReleased.GetCheck()) {
			bBarcodeUsedItems = FALSE;
		}

		//if the allocation is already completed, they can't change the used/released status,
		//so the barcode can only be used to mark a used item as billed, nothing else
		BOOL bBarcodeBillingOnly = m_pAllocationMasterInfo->iasStatus == InvUtils::iasCompleted;

		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i);
			if(pDetail && pDetail->varSerialNum == varBarcode && pDetail->iadsCurrentStatus != InvUtils::iadsDeleted) {

				//found it - but see if it is already our intended status

				if(pDetail->iadsCurrentStatus == InvUtils::iadsReleased
					&& !bBarcodeUsedItems && !bBarcodeBillingOnly) {

					//it's already released, and we want to release on barcode scan,
					//so continue rather than return, it may exist twice
					continue;
				}

				if(pDetail->iadsCurrentStatus == InvUtils::iadsUsed && pDetail->bBilled
					&& bBarcodeUsedItems && !bBarcodeBillingOnly) {

					//it's already used AND billed, and we want to use on barcode scan,
					//so continue rather than return, it may exist twice
					continue;
				}
				
				//find the row that this item is in
				IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcDetailObjectPtr, (long)pDetail, m_AllocationList->GetFirstRow(), FALSE);
				if(pRow == NULL) {
					continue;
				}

				//don't allow changing boxes on disabled rows
				//if not active, you can't edit this row anymore
				if(VarBool(pRow->GetValue(alcDisabled))) {
					continue;
				}

				//if the allocation was previously completed, then barcoding is used only for billing,
				//so ignore this item if the status is released or the status is billed
				// (j.jones 2008-01-07 16:57) - PLID 28479 - or if the product is not billable
				if(bBarcodeBillingOnly && m_bIsCompletedByBill
					&& (pDetail->iadsCurrentStatus == InvUtils::iadsReleased || pDetail->bBilled
					|| !pDetail->bIsProductBillable)) {
					continue;
				}

				//now we have a detail object and a datalist row,
				//and can finally try to change the status

				//check the product ID we're trying to use
				long nProductID = VarLong(pRow->GetValue(alcProductID));

				//if we are barcoding only billed products, and we aren't forcing the user to
				//complete the allocation yet, and a m_nInitialProductID exists,
				//then we don't have to enforce that product ID, but we do have to enforce
				//that only ONE product ID is billed in this edit session (multiple product
				//items are OK), so check what has been marked as "billed" in this edit
				//session
				if(bBarcodeBillingOnly) {

					// (j.jones 2008-01-07 16:59) - PLID 28479 - return now if the product
					// is not billable to this location
					if(!pDetail->bIsProductBillable) {
						return 0;
					}
					
					//will give a message and return FALSE if not billable					
					if(!CanBillThisProduct(nProductID)) {
						return 0;
					}

					//else check the box as billed, update the memory object accordingly
					if(pDetail->iadsCurrentStatus == InvUtils::iadsUsed) {
						pDetail->bBilled = TRUE;
						pRow->PutValue(alcBilled, g_cvarTrue);
						//add to our list of tracked "billed" objects for this edit session
						TryAddDetailToBilledList(pDetail);
					}
					else {
						//shouldn't be allowed
						ASSERT(FALSE);
					}
					return 0;
				}

				//if we get here, we're allowed to change the status, so do so,
				//and update the memory object and datalist

				//update the memory object accordingly
				if(bBarcodeUsedItems) {
					pDetail->iadsCurrentStatus = InvUtils::iadsUsed;
					pRow->PutValue(alcUsed, g_cvarTrue);
						
					//try to check the "billed" box, but if we aren't forcing the user to
					//complete the allocation yet, and a m_nInitialProductID exists,
					//then we don't have to enforce that product ID, but we do have to enforce
					//that only ONE product ID is used in this edit session (multiple product
					//items are OK), so check what has been marked as "billed" in this edit
					//session, and silently not check the billed box if they can't use it in
					//this session

					// (j.jones 2008-01-07 16:59) - PLID 28479 - don't mark as billed if the product
					// is not billable to this location
					if(pDetail->bIsProductBillable && CanBillThisProduct(nProductID, TRUE)) {
						pDetail->bBilled = TRUE;
						pRow->PutValue(alcBilled, g_cvarTrue);
						//add to our list of tracked "billed" objects for this edit session
						TryAddDetailToBilledList(pDetail);
					}

					//force the Released column to be unchecked
					pRow->PutValue(alcReleased, g_cvarFalse);

					//TES 6/20/2008 - PLID 26152 - If this isn't released, it can't be returned.
					pRow->PutValue(alcReturn, g_cvarNull);
				}
				else {
					pDetail->iadsCurrentStatus = InvUtils::iadsReleased;
					pRow->PutValue(alcReleased, g_cvarTrue);

					//TES 6/20/2008 - PLID 26152 - If we're releasing a serialized item to purchased inventory, then they
					// can flag it to be returned, so load their preference for that.  Otherwise, leave the column blank.
					if((InvUtils::ProductItemStatus)VarLong(pDetail->varProductItemStatus, -1) == InvUtils::pisPurchased &&
						pDetail->nProductItemID != -1) {
						BOOL bReturn = GetRemotePropertyInt("ReturnReleasedGeneralSerializedItems", 1, 0, "<None>", true);
						pRow->PutValue(alcReturn, bReturn?g_cvarTrue:g_cvarFalse);
						pDetail->bToBeReturned = bReturn;
					}
					else {
						pRow->PutValue(alcReturn, g_cvarNull);
					}

					//force it to be unbilled
					pDetail->bBilled = FALSE;
					// (j.jones 2008-01-07 17:27) - PLID 28479 - don't change the row status from NULL
					// if not billable
					if(pDetail->bIsProductBillable) {
						pRow->PutValue(alcBilled, g_cvarFalse);
					}
					//make sure it is not in our tracked "billed" list
					TryRemoveDetailFromBilledList(pDetail);

					//force the Used column to be unchecked
					pRow->PutValue(alcUsed, g_cvarFalse);
				}

				//success, leave this function
				return 0;								
			}
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnBarcodeScan");

	return 0;
}

BEGIN_EVENTSINK_MAP(CInvAllocationUsageDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvAllocationUsageDlg)
	ON_EVENT(CInvAllocationUsageDlg, IDC_ALLOCATED_ITEMS_LIST, 10 /* EditingFinished */, OnEditingFinishedAllocatedItemsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvAllocationUsageDlg, IDC_ALLOCATED_ITEMS_LIST, 8 /* EditingStarting */, OnEditingStartingAllocatedItemsList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInvAllocationUsageDlg, IDC_ALLOCATED_ITEMS_LIST, 9 /* EditingFinishing */, OnEditingFinishingAllocatedItemsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvAllocationUsageDlg::OnEditingFinishedAllocatedItemsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//this should be impossible
		if(m_pAllocationMasterInfo == NULL) {
			ASSERT(FALSE);
			return;
		}

		if(nCol == alcNotes) {
			//update our memory object
			CString strNotes = VarString(varNewValue,"");
			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)VarLong(pRow->GetValue(alcDetailObjectPtr));
			if(pInfo) {
				pInfo->strNotes = strNotes;
			}
			else {
				//why didn't we have a memory object?
				ASSERT(FALSE);
			}
		}

		//update the memory object, and then toggle between
		//used & released so they act like radio buttons

		if((nCol == alcUsed || nCol == alcReleased || nCol == alcBilled) && varNewValue.vt == VT_BOOL) {

			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)VarLong(pRow->GetValue(alcDetailObjectPtr));

			//update the memory object accordingly
			if(pInfo) {
				if(VarBool(varNewValue)) {
					//they checked a box
					if(nCol == alcUsed) {
						pInfo->iadsCurrentStatus = InvUtils::iadsUsed;
						
						//try to check the "billed" box, but if we aren't forcing the user to
						//complete the allocation yet, and a m_nInitialProductID exists,
						//then we don't have to enforce that product ID, but we do have to enforce
						//that only ONE product ID is used in this edit session (multiple product
						//items are OK), so check what has been marked as "billed" in this edit
						//session, and silently not check the billed box if they can't use it in
						//this session
						long nProductID = VarLong(pRow->GetValue(alcProductID));
						// (j.jones 2008-01-07 16:59) - PLID 28479 - don't mark as billed if the product
						// is not billable to this location
						if(pInfo->bIsProductBillable && CanBillThisProduct(nProductID, TRUE)) {
							pInfo->bBilled = TRUE;
							pRow->PutValue(alcBilled, g_cvarTrue);
							//add to our list of tracked "billed" objects for this edit session
							TryAddDetailToBilledList(pInfo);
						}

						//force the Released column to be unchecked
						pRow->PutValue(alcReleased, g_cvarFalse);

						//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
						pRow->PutValue(alcReturn, g_cvarNull);
					}
					else if(nCol == alcReleased) {
						pInfo->iadsCurrentStatus = InvUtils::iadsReleased;

						//force it to be unbilled
						pInfo->bBilled = FALSE;
						// (j.jones 2008-01-07 17:27) - PLID 28479 - don't change the row status from NULL
						// if not billable
						if(pInfo->bIsProductBillable) {
							pRow->PutValue(alcBilled, g_cvarFalse);
						}
						//make sure it is not in our tracked "billed" list
						TryRemoveDetailFromBilledList(pInfo);

						//force the Used column to be unchecked
						pRow->PutValue(alcUsed, g_cvarFalse);

						//TES 6/20/2008 - PLID 26152 - If we're releasing a serialized item to purchased inventory, then they
						// can flag it to be returned, so load their preference for that.  Otherwise, leave the column blank.
						if(pInfo->nProductItemID != -1 && 
							(InvUtils::ProductItemStatus)VarLong(pInfo->varProductItemStatus, -1) == InvUtils::pisPurchased) {
							BOOL bReturn = GetRemotePropertyInt("ReturnReleasedGeneralSerializedItems", 1, 0, "<None>", true);
							pRow->PutValue(alcReturn, bReturn?g_cvarTrue:g_cvarFalse);
							pInfo->bToBeReturned = bReturn;
						}
						else {
							pRow->PutValue(alcReturn, g_cvarNull);
						}
					}
					else if(nCol == alcBilled) {

						// (j.jones 2008-01-07 16:59) - PLID 28479 - don't mark as billed if the product
						// is not billable to this location
						if(!pInfo->bIsProductBillable) {
							//they shouldn't have been able to check this box!
							ASSERT(FALSE);
						}
						else {						
							pInfo->bBilled = TRUE;						
							pRow->PutValue(alcBilled, g_cvarTrue);

							//make absolutely sure that "used" is checked
							pInfo->iadsCurrentStatus = InvUtils::iadsUsed;
							pRow->PutValue(alcUsed, g_cvarTrue);
							pRow->PutValue(alcReleased, g_cvarFalse);
							//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
							pRow->PutValue(alcReturn, g_cvarNull);
							
							//add to our list of tracked "billed" objects for this edit session
							TryAddDetailToBilledList(pInfo);
						}
					}
				}
				else {

					if(nCol == alcUsed) {
						//if they unchecked the used box it becomes active
						//TES 8/21/2008 - PLID 29478 - If it was originally on order, and they haven't assigned a product item,
						// then set it back to on order.
						if(pInfo->iadsOriginalStatus == InvUtils::iadsOrder && pInfo->nProductItemID == -1) {
							pInfo->iadsCurrentStatus = InvUtils::iadsOrder;
						}
						else {
							pInfo->iadsCurrentStatus = InvUtils::iadsActive;
						}

						//but also ensure the billed box is unchecked
						pInfo->bBilled = FALSE;
						// (j.jones 2008-01-07 17:27) - PLID 28479 - don't change the row status from NULL
						// if not billable
						if(pInfo->bIsProductBillable) {
							pRow->PutValue(alcBilled, g_cvarFalse);
						}

						//make sure it is not in our tracked "billed" list
						TryRemoveDetailFromBilledList(pInfo);
					}
					else if(nCol == alcReleased) {						
						//if they unchecked the released box it becomes active
						//TES 8/21/2008 - PLID 29478 - If it was originally on order, and they haven't assigned a product item,
						// then set it back to on order.
						if(pInfo->iadsOriginalStatus == InvUtils::iadsOrder && pInfo->nProductItemID == -1) {
							pInfo->iadsCurrentStatus = InvUtils::iadsOrder;
						}
						else {
							pInfo->iadsCurrentStatus = InvUtils::iadsActive;
						}

						//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
						pRow->PutValue(alcReturn, g_cvarNull);
					}
					else if(nCol == alcBilled) {
						//if they unchecked billed, leave it used, juse remove the billed status
						pInfo->bBilled = FALSE;

						//make sure it is not in our tracked "billed" list
						TryRemoveDetailFromBilledList(pInfo);
					}
				}
			}
			else {
				//why didn't we have a memory object?
				ASSERT(FALSE);
			}
		}
		else if(nCol == alcReturn) {
			if(varNewValue.vt != VT_NULL) {
				//TES 6/20/2008 - PLID 26152 - Save this flag to our memory object.
				InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)VarLong(pRow->GetValue(alcDetailObjectPtr));
				pInfo->bToBeReturned = VarBool(varNewValue);
			}
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnEditingFinishedAllocatedItemsList");
}

void CInvAllocationUsageDlg::OnEditingStartingAllocatedItemsList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2008-01-08 09:10) - PLID 28479 - if the checkbox is not present,
		// leave silently
		if(pvarValue->vt == VT_NULL && nCol == alcBilled) {
			*pbContinue = FALSE;
			return;
		}

		//don't allow checking boxes on disabled rows
		//TES 6/20/2008 - PLID 26152 - Added the Return column.
		if(VarBool(pRow->GetValue(alcDisabled)) && (nCol == alcUsed || nCol == alcReleased || nCol == alcBilled || nCol == alcReturn)) {
			//disable and leave this function
			*pbContinue = FALSE;

			//if directly checking a box, tell them why they can't change it
			AfxMessageBox("You cannot modify the 'Used', 'Released', 'Return', or 'Billed' status on a product that has already been billed.");
			return;
		}

		//if the allocation was previously completed, disable editing all used/released statuses,
		//and also disable billing released items
		if(m_pAllocationMasterInfo->iasStatus == InvUtils::iasCompleted) {
			if(nCol == alcUsed || nCol == alcReleased) {
				//can't change used/released status, warn and leave this function
				*pbContinue = FALSE;
				AfxMessageBox("You cannot modify the 'Used' or 'Released' status on an already completed allocation.");
				return;
			}
			else if(nCol == alcBilled && VarBool(pRow->GetValue(alcReleased), FALSE)) {
				//can't bill released items, warn and leave this function
				*pbContinue = FALSE;
				//the billed checkbox shouldn't be there, so we don't need to be redundant
				//and tell them that they can't check it
				//AfxMessageBox("You cannot mark a 'Released' product as billed on an already completed allocation.");
				return;
			}
		}

		//if we are checking the "Billed" box, and we aren't forcing the user to
		//complete the allocation yet, and a m_nInitialProductID exists,
		//then we don't have to enforce that product ID, but we do have to enforce
		//that only ONE product ID is used in this edit session (multiple product
		//items are OK), so check what has been marked as "billed" in this edit
		//session
		long nProductID = VarLong(pRow->GetValue(alcProductID));
		if(nCol == alcBilled && !VarBool(pvarValue) && !CanBillThisProduct(nProductID)) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnEditingStartingAllocatedItemsList");
}

CString CInvAllocationUsageDlg::GetExistingProductItemWhereClause(long nServiceID)
{
	//TES 7/18/2008 - PLID 29478 - returns a query of AND statements so unsaved assigned ProductItems aren't in the list
	// if and when the user gets prompted to select some.
	try {

		CString strIDList = "";

		//find each row with the matching service ID, see if it has
		//a ProductItemID, and isn't "released".
		//if so, use it to build our where clause
		IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
		while(pRow != NULL) {
			
			long nServiceIDToCheck = VarLong(pRow->GetValue(alcProductID), -1);
			BOOL bReleased = VarBool(pRow->GetValue(alcReleased), FALSE);
			if(nServiceIDToCheck == nServiceID && !bReleased) {
				//this is the right product, now check its ProductItemID
				long nProductItemID = VarLong(pRow->GetValue(alcProductItemID), -1);
				if(nProductItemID != -1) {
					//we have one, so add it to our where clause
					if(!strIDList.IsEmpty()) {
						strIDList += ",";
					}
					strIDList += AsString(nProductItemID);
				}
			}

			pRow = pRow->GetNextRow();
		}

		//now build the clause to return
		if(!strIDList.IsEmpty()) {
			CString strWhere;
			strWhere.Format("ProductItemsT.ID NOT IN (%s)", strIDList);
			return strWhere;
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::GetExistingProductItemWhereClause");

	return "";
}

void CInvAllocationUsageDlg::OnEditingFinishingAllocatedItemsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//we shouldn't be allowed to get here if we couldn't load an allocation
		if(m_pAllocationMasterInfo == NULL) {
			ASSERT(FALSE);
			return;
		}

		if(nCol == alcNotes) {
			//ensure the length is not greater than 255
			CString str = strUserEntered;
			if(str.GetLength() > 255) {
				AfxMessageBox("The text you entered is longer than the maximum amount (255) and has been shortened.\n"
					"Please double-check your note and make changes as needed.");
				// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks
				::VariantClear(pvarNewValue);
				*pvarNewValue = _variant_t(str.Left(255)).Detach();
			}
		}

		//if checking used/released/billed (not unchecking), see if the detail has a quantity <> 1.0,
		//if so, prompt for how much to use or release, with the option to cancel
		//(don't change the status here, that is handled in OnEditingFinished)
		if((nCol == alcUsed || nCol == alcReleased || nCol == alcBilled) && pvarNewValue->vt == VT_BOOL && VarBool(pvarNewValue, FALSE)) 
		{
			//TES 7/18/2008 - PLID 29478 - If this is a serialized item, and it doesn't have a ProductItemID associated with
			// it, it needs to have one now (it was "To Be Ordered" before).
			if(VarBool(pRow->GetValue(alcIsSerialized)) && VarLong(pRow->GetValue(alcProductItemID),-1) == -1) {
				//TES 7/18/2008 - PLID 29478 - We don't allow this.
				if(nCol == alcReleased) {
					*pvarNewValue = varOldValue;
					*pbCommit = FALSE;
					AfxMessageBox("You cannot release a product that is 'To Be Ordered.'  Please select a different status, or delete this product from the allocation.");
					return;
				}
				AfxMessageBox("This product was previously flagged as 'To Be Ordered', you must now choose an item to allocate.");
				
				//TES 7/21/2008 - PLID 29478 - Broke the code to select a serial number out into its own function.
				if(!PromptSelectItem(pRow)) {
					//TES 7/21/2008 - PLID 29478 - They cancelled, and were told that they wouldn't be able to change the status.
					*pvarNewValue = varOldValue;
					*pbCommit = FALSE;
					return;
				}
			}

			//we're NOT going to do this if we're billing a column already marked used!
			if(!(nCol == alcBilled && VarBool(pRow->GetValue(alcUsed), FALSE))) {

				if(!TrySplitByQuantity(pRow, nCol != alcReleased)) {
					*pbCommit = FALSE;
					return;
				}
			}
		}
		
	}NxCatchAll("Error in CInvAllocationUsageDlg::OnEditingFinishingAllocatedItemsList");
}

//TES 7/21/2008 - PLID 29478 - Selects a ProductItemsT record to assign to the specified row.  A return value of FALSE means
// that the user cancelled, and was told that this would prevent them from changing the item's status.
BOOL CInvAllocationUsageDlg::PromptSelectItem(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	//prompt
	long nProductID = VarLong(pRow->GetValue(alcProductID));
	
	CProductItemsDlg dlg(this);
	dlg.m_EntryType = PI_SELECT_DATA;
	dlg.m_strOverrideTitleBarText = "Select Item to be Allocated";
	dlg.m_strOverrideSelectQtyText = "Quantity to be allocated:";
	dlg.m_ProductID = nProductID;
	dlg.m_nLocationID = m_pAllocationMasterInfo->nLocationID;
	dlg.m_CountOfItemsNeeded = 1;
	dlg.m_strWhere = GetExistingProductItemWhereClause(nProductID);
	dlg.m_bDisallowQtyChange = TRUE;

	//disable barcode messages on this screen while the product items dialog is up
	m_bDisableBarcode = TRUE;

	if(IDCANCEL == dlg.DoModal()) {
		//re-enable barcoding
		m_bDisableBarcode = FALSE;
		//TES 7/18/2008 - PLID 29478 - Abandon this edit.
		AfxMessageBox("You cannot change this item's status without selecting an item to allocate.");
		return FALSE;
	}
	//loop through dlg.m_adwProductItemIDs and add to the list
	ASSERT(dlg.m_adwProductItemIDs.GetSize() == 1);

	_variant_t varProductItemID = (long)(dlg.m_adwProductItemIDs.GetAt(0));
	_variant_t varSerialNumber = dlg.GetSelectedProductItemSerialNum(0);
	_variant_t varExpDate = dlg.GetSelectedProductItemExpDate(0);
	pRow->PutValue(alcProductItemID, varProductItemID);
	InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)VarLong(pRow->GetValue(alcDetailObjectPtr));
	pInfo->nProductItemID = VarLong(varProductItemID,-1);
	pRow->PutValue(alcSerialNum, varSerialNumber);
	pRow->PutValue(alcExpDate, varExpDate);
	// (c.haag 2008-03-11 13:52) - PLID 29255 - Acquire the status, too
	long nStatus = dlg.GetSelectedProductItemStatus(0);
		
	switch (nStatus) {
	case InvUtils::pisPurchased:
		pRow->PutValue(alcProductType, "Purchased Inv.");
		break;
	case InvUtils::pisConsignment:
		pRow->PutValue(alcProductType, "Consignment");
		break;
	case InvUtils::pisWarranty:
		pRow->PutValue(alcProductType, "Warranty");
		break;
	default:
		if (VarLong(varProductItemID, -1) <= 0) {
			// If there is no valid product item ID, it must be a non-serializeable product.
			// Call it purchased inventory.
			pRow->PutValue(alcProductType, "Purchased Inv.");
		} else {
			// Unexpected value; leave null
			pRow->PutValue(alcProductType, g_cvarNull);
		}
		break;
	}	
	
	//re-enable barcoding
	m_bDisableBarcode = FALSE;

	return TRUE;
}
//PromptForQtyChange will take in a boolean for whether we're using or releasing a given product,
//takes in the product name and current quantity, and then passes back the quantity they are using,
//which must be greater than zero and not greater than the current quantity.
//Return FALSE if the user cancelled, return TRUE if we are returning a valid new quantity.
BOOL CInvAllocationUsageDlg::PromptForQtyChange(BOOL bUsed, CString strProductName, double dblCurQuantity, double &dblNewQuantity)
{
	try {

		if(dblCurQuantity <= 0.0) {
			//bad data
			ASSERT(FALSE);
			return FALSE;
		}

		if(dblCurQuantity == 1.0) {
			//this function should not have been called,
			//but as such, nothing will happen
			dblNewQuantity = 1.0;
			return TRUE;
		}

		CString strQty, strMsg;
		strQty.Format("%g", dblCurQuantity);
		strMsg.Format("How much of the '%s' product was %s?", strProductName, bUsed ? "used" : "released");
		int nRet = InputBox(this, strMsg, strQty, "");
		while(nRet != IDOK || atof(strQty) > dblCurQuantity || atof(strQty) <= 0) {
			if(nRet == IDCANCEL) {				
				return FALSE;
			}
			if(atof(strQty) <= 0) {
				AfxMessageBox("Please enter a quantity greater than zero.");
			}
			else if(atof(strQty) > dblCurQuantity) {
				CString str;
				str.Format("Please enter a quantity no greater than %g", dblCurQuantity);
				AfxMessageBox(str);
			}
			strQty.Format("%g", dblCurQuantity);
			nRet = InputBox(this, strMsg, strQty,"");
		}
		dblNewQuantity = atof(strQty);

		// (j.jones 2007-12-13 15:21) - PLID 27988 - if this dialog was called from a
		// bill or case history, warn that any change will save immediately
		if(dblNewQuantity < dblCurQuantity && (m_bIsCompletedByBill || m_bIsCompletedByCaseHistory)) {
			if(IDNO == MessageBox("This new quantity will be saved to data immediately. Are you sure you wish to change it?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return FALSE;
			}
		}

		return TRUE;

	}NxCatchAll("Error in CInvAllocationUsageDlg::PromptForQtyChange");

	return FALSE;
}

//TrySplitByQuantity will take in a row, and if it has multiple quantity, prompt for the amt. to use,
//and create a new row with the balance - returns FALSE if the process should be aborted
BOOL CInvAllocationUsageDlg::TrySplitByQuantity(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bMarkingUsed)
{
	long nAuditTransactionID = -1;

	try {

		InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)VarLong(pRow->GetValue(alcDetailObjectPtr));
			
		if(pInfo) {

			if(pInfo->dblCurQuantity > 0.0 && pInfo->dblCurQuantity != 1.0) {
				
				//okay they have a quantity that is not 1.0 (remember, 0.5 is something we will prompt on),
				//so ask them how much to use or return

				CString strProductName = pInfo->strProductName;
				double dblCurQuantity = pInfo->dblCurQuantity;
				double dblNewQuantity = 0.0;

				if(!PromptForQtyChange(bMarkingUsed, strProductName, dblCurQuantity, dblNewQuantity)) {
					//nothing was committed, so don't let this column remain checked
					return FALSE;
				}

				if(dblNewQuantity == 0.0 || dblNewQuantity > dblCurQuantity) {
					//shouldn't be possible
					ASSERT(FALSE);
					return FALSE;
				}

				//if they are approving the whole quantity, then we can continue normally
				if(dblNewQuantity == dblCurQuantity) {
					return TRUE;
				}

				//if we get here, they are only using/releasing some of the original quantity,
				//which means we need to change the current detail's quantity, and create a new
				//detail with the balance, marking the new detail with the opposite status
				//as the current one

				//update the current quantity
				pInfo->dblCurQuantity = dblNewQuantity;

				//and update in the datalist
				pRow->PutValue(alcQuantity, dblNewQuantity);

				//now we need to create the new detail
				{
					InvUtils::AllocationDetailInfo *pNewInfo = new InvUtils::AllocationDetailInfo;

					//populate it with *almost* all the data of our current detail
					pNewInfo->nDetailID = -1;
					pNewInfo->nProductID = pInfo->nProductID;
					pNewInfo->strProductName = pInfo->strProductName;
					pNewInfo->nProductItemID = pInfo->nProductItemID;
					//shouldn't be possible for the serialized stuff to be non-null, but copy anyways
					pNewInfo->varSerialNum = pInfo->varSerialNum;
					pNewInfo->varExpDate = pInfo->varExpDate;
					pNewInfo->varProductItemStatus = pInfo->varProductItemStatus; // (c.haag 2008-03-11 14:05) - PLID 29255 - We now track the product item status
					pNewInfo->strOriginalNotes = ""; //new item, there is no "original"
					pNewInfo->strNotes = pInfo->strNotes; //use the current notes

					//now for quantity, it should create with our balance
					pNewInfo->dblOriginalQty = dblCurQuantity - dblNewQuantity;
					pNewInfo->dblCurQuantity = pNewInfo->dblOriginalQty;

					//and status should be the opposite of what we checked
					pNewInfo->iadsOriginalStatus = bMarkingUsed ? InvUtils::iadsReleased : InvUtils::iadsUsed;
					pNewInfo->iadsCurrentStatus = pNewInfo->iadsOriginalStatus; //use the same status
					
					//if we had checked released, the new row is used, 
					//so mark it billed if we are permitted to do so
					// (j.jones 2008-01-07 16:59) - PLID 28479 - don't mark as billed if the product
					// is not billable to this location
					if(!bMarkingUsed && pInfo->bIsProductBillable && CanBillThisProduct(pInfo->nProductID, TRUE)) {
						pNewInfo->bBilled = TRUE;
					}
					else {
						pNewInfo->bBilled = FALSE;
					}

					// (j.jones 2008-01-07 17:06) - PLID 28479 - added bIsProductBillable
					pNewInfo->bIsProductBillable = pInfo->bIsProductBillable;

					//TES 7/18/2008 - PLID 29478 - Fill in whether this is a serialized product.
					pNewInfo->bIsSerialized = pInfo->bIsSerialized;

					//we will add to our array and the datalist after the following block of code,
					//which may save the detail and get a new ID

					// (j.jones 2007-12-13 15:21) - PLID 27988 - if this dialog was called from a
					// bill or case history, save the changes immediately
					if(dblNewQuantity < dblCurQuantity && (m_bIsCompletedByBill || m_bIsCompletedByCaseHistory)) {

						CString strSqlBatch = BeginSqlBatch();				

						//save the quantity change
						{
							//used for auditing
							CString strDesc = pInfo->strProductName;
							_variant_t varSerialNumber = pInfo->varSerialNum;
							_variant_t varExpDate = pInfo->varExpDate;

							if(varSerialNumber.vt == VT_BSTR) {
								CString str;
								str.Format(", Serial Num: %s", VarString(varSerialNumber));
								strDesc += str;
							}
							if(varExpDate.vt == VT_DATE) {
								CString str;
								str.Format(", Exp. Date: %s", FormatDateTimeForInterface(VarDateTime(varExpDate), NULL, dtoDate));
								strDesc += str;
							}
							if(pInfo->dblOriginalQty != 1.0) {
								//only show the quantity if not 1.0
								CString str;
								str.Format(", Quantity: %g", pInfo->dblOriginalQty);
								strDesc += str;
							}

							//save the quantity
							AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Quantity = %g WHERE ID = %li", pInfo->dblCurQuantity, pInfo->nDetailID);

							if(pInfo->dblCurQuantity != pInfo->dblOriginalQty) {
								if(nAuditTransactionID == -1) {
									nAuditTransactionID = BeginAuditTransaction();
								}
								AuditEvent(m_pAllocationMasterInfo->nPatientID, m_pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiInvAllocationDetailQuantity, m_pAllocationMasterInfo->nAllocationID, strDesc, AsString(pInfo->dblCurQuantity), aepMedium, aetChanged);
							}

							//update the memory object so we don't have to audit again later
							pInfo->dblOriginalQty = pInfo->dblCurQuantity;
						}

						//now save the new detail
						CString strProductItemID = "NULL";
						if(pNewInfo->nProductItemID != -1) {
							//it should be logically impossible for us to be
							//creating a new serialized detail here
							ASSERT(FALSE);
							strProductItemID.Format("%li", pNewInfo->nProductItemID);
						}

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationDetailsT "
							"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes) "
							"VALUES (%li, %li, %s, %g, %li, '%s')", m_pAllocationMasterInfo->nAllocationID, pNewInfo->nProductID,
							strProductItemID, pNewInfo->dblCurQuantity, InvUtils::iadsActive, _Q(pNewInfo->strNotes));

						CString strDesc = pNewInfo->strProductName;
						_variant_t varSerialNumber = pNewInfo->varSerialNum;
						_variant_t varExpDate = pNewInfo->varExpDate;

						if(varSerialNumber.vt == VT_BSTR) {
							CString str;
							str.Format(", Serial Num: %s", VarString(varSerialNumber));
							strDesc += str;
						}
						if(varExpDate.vt == VT_DATE) {
							CString str;
							str.Format(", Exp. Date: %s", FormatDateTimeForInterface(VarDateTime(varExpDate), NULL, dtoDate));
							strDesc += str;
						}
						if(pNewInfo->dblOriginalQty != 1.0) {
							//only show the quantity if not 1.0
							CString str;
							str.Format(", Quantity: %g", pNewInfo->dblOriginalQty);
							strDesc += str;
						}

						//audit the creation
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(m_pAllocationMasterInfo->nPatientID, m_pAllocationMasterInfo->strPatientName, nAuditTransactionID, aeiInvAllocationDetailCreated, m_pAllocationMasterInfo->nAllocationID, "", strDesc, aepMedium, aetCreated);

						if(!strSqlBatch.IsEmpty()) {
							_RecordsetPtr rs = CreateRecordset(
								"SET NOCOUNT ON "
								"%s"
								"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID "
								"SET NOCOUNT OFF ", strSqlBatch);
							if(!rs->eof) {
								//grab the new detail ID
								long nID = AdoFldLong(rs, "ID");
								pNewInfo->nDetailID = nID;
							}
							rs->Close();
						}

						if(nAuditTransactionID != -1) {
							CommitAuditTransaction(nAuditTransactionID);
							nAuditTransactionID = -1;
						}

						// (c.haag 2008-02-27 13:32) - PLID 29115 - Update inventory todo alarms
						const long nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
						try {
							if (pNewInfo->nProductItemID > 0) {
								InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_ProductItemID, pNewInfo->nProductItemID);
							} else {
								InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_ProductID, pNewInfo->nProductID);
							}
							//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
							InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
						}
						NxCatchAllSilentCallThrow(InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID));
					}

					//now add it to our memory object
					m_pAllocationMasterInfo->paryAllocationDetailInfo.Add(pNewInfo);

					//and add a new datalist row with this data,
					//but pass in FALSE so it isn't auto-disabled
					AddDatalistRowFromDetail(pNewInfo, FALSE);
				}
			}
		}
		else {
			//why didn't we have a memory object?
			ASSERT(FALSE);
			return FALSE;
		}

		return TRUE;

	}NxCatchAllCall("Error in CInvAllocationUsageDlg::TrySplitByQuantity",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return FALSE;
}

// (j.jones 2008-06-11 15:31) - PLID 28379 - TryResolveInitialProductID will try to auto
// "use" the passed in m_nInitialAutoUseDetailID, if one was given to us
void CInvAllocationUsageDlg::TryResolveInitialDetailID()
{
	try {

		//this function has no meaning if we weren't
		//given a m_nInitialAutoUseDetailID
		//TES 7/16/2008 - PLID 27983 - We could also pull from m_parInitialAutoUseProductItemIDs
		if(m_nInitialAutoUseDetailID == -1 && m_parInitialAutoUseProductItemIDs == NULL) {
			return;
		}

		//we shouldn't be allowed to get here if we couldn't load an allocation
		if(m_pAllocationMasterInfo == NULL) {
			ASSERT(FALSE);
			return;
		}

		//Run through the allocation, if the product is available then mark it used/billed
		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			bool bMatched = false;
			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)(m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i));
			if(pInfo && m_nInitialAutoUseDetailID != -1 && pInfo->nDetailID == m_nInitialAutoUseDetailID && (pInfo->iadsCurrentStatus == InvUtils::iadsActive || pInfo->iadsCurrentStatus == InvUtils::iadsUsed)) {
				//found it, and it is able to be billed, so let's track that
				bMatched = true;
			}
			//TES 7/16/2008 - PLID 27983 - Also check against our list of product items.
			else if(pInfo && m_parInitialAutoUseProductItemIDs && (pInfo->iadsCurrentStatus == InvUtils::iadsActive || pInfo->iadsCurrentStatus == InvUtils::iadsUsed)) {
				for(int nProductItem = 0; !bMatched && nProductItem < m_parInitialAutoUseProductItemIDs->GetSize(); nProductItem++) {
					if(pInfo->nProductItemID == m_parInitialAutoUseProductItemIDs->GetAt(nProductItem)) {
						//TES 7/16/2008 - PLID 27983 - OK, it's one of our product items, and it is able to be billed
						bMatched = true;
					}
				}
			}
			if(bMatched) {
				//mark the item used and billed

				if(pInfo == NULL) {
					//this should be impossible
					ASSERT(FALSE);
				}
				else {

					pInfo->iadsCurrentStatus = InvUtils::iadsUsed;

					//update the datalist accordingly
					IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcDetailObjectPtr, (long)pInfo, m_AllocationList->GetFirstRow(), FALSE);
					if(pRow == NULL) {
						//this should be impossible
						ASSERT(FALSE);
					}
					else {

						//we will auto-check used & billed on this row, but TrySplitByQuantity
						//will handle appropriately if we have a quantity that is not 1.0
						if(TrySplitByQuantity(pRow, TRUE)) {

							pRow->PutValue(alcUsed, g_cvarTrue);
							pRow->PutValue(alcReleased, g_cvarFalse);

							//update the memory object accordingly
							pInfo->iadsCurrentStatus = InvUtils::iadsUsed;

							// (j.jones 2008-01-07 16:59) - PLID 28479 - don't mark as billed if the product
							// is not billable to this location
							if(pInfo->bIsProductBillable) {
								pRow->PutValue(alcBilled, g_cvarTrue);

								//update the memory object accordingly
								pInfo->bBilled = TRUE;
									
								//add to our list of tracked "billed" objects for this edit session
								TryAddDetailToBilledList(pInfo);
							}
						}
					}
				}
			}
		}

		//no warning needed if not found, though why would that ID be passed into this allocation?

	}NxCatchAll("Error in CInvAllocationUsageDlg::TryResolveInitialDetailID");
}

//TryResolveInitialProductID will try to auto "use" the passed in m_nInitialProductID,
//if one was given to us, if only one product matches on the allocation
void CInvAllocationUsageDlg::TryResolveInitialProductID()
{
	try {

		//this function has no meaning if we weren't
		//given a m_nInitialProductID
		if(m_nInitialProductID == -1) {
			return;
		}

		//we shouldn't be allowed to get here if we couldn't load an allocation
		if(m_pAllocationMasterInfo == NULL) {
			ASSERT(FALSE);
			return;
		}

		//Run through the allocation, if the product is available
		//only once, then mark it used. If the product exists
		//more than once but all the other entries are unavailable,
		//then ignore those entries.
		long nCountFound = 0;
		InvUtils::AllocationDetailInfo *pInfoFound = NULL;

		//stop looping if we find more than one, we can't do anything if that's the case
		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize() && nCountFound < 2;i++) {
			InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)(m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i));
			if(pInfo && pInfo->nProductID == m_nInitialProductID && pInfo->iadsCurrentStatus == InvUtils::iadsActive) {
				//found it, and it is available, so let's track that
				nCountFound++;
				pInfoFound = pInfo;
			}
		}

		if(nCountFound == 1) {
			//mark the item used

			if(pInfoFound == NULL) {
				//this should be impossible
				ASSERT(FALSE);
			}
			else {

				pInfoFound->iadsCurrentStatus = InvUtils::iadsUsed;

				//update the datalist accordingly
				IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcDetailObjectPtr, (long)pInfoFound, m_AllocationList->GetFirstRow(), FALSE);
				if(pRow == NULL) {
					//this should be impossible
					ASSERT(FALSE);
				}
				else {

					//we will auto-check used & billed on this row, but TrySplitByQuantity
					//will handle appropriately if we have a quantity that is not 1.0
					if(TrySplitByQuantity(pRow, TRUE)) {

						pRow->PutValue(alcUsed, g_cvarTrue);
						pRow->PutValue(alcReleased, g_cvarFalse);

						//update the memory object accordingly
						pInfoFound->iadsCurrentStatus = InvUtils::iadsUsed;

						// (j.jones 2008-01-07 16:59) - PLID 28479 - don't mark as billed if the product
						// is not billable to this location
						if(pInfoFound->bIsProductBillable) {
							pRow->PutValue(alcBilled, g_cvarTrue);

							//update the memory object accordingly
							pInfoFound->bBilled = TRUE;
								
							//add to our list of tracked "billed" objects for this edit session
							TryAddDetailToBilledList(pInfoFound);
						}
					}
				}
			}
		}
		else if(nCountFound > 1) {
			//no warning for the time being
			/*
			CString str;
			str.Format("The product '%s' exists multiple times in this allocation. You must select which allocated items you wish to pull from the allocation.", pInfoFound->strProductName);
			AfxMessageBox(str);
			*/
		}

		//no warning needed if not found, though why would that ID be passed into this allocation?

	}NxCatchAll("Error in CInvAllocationUsageDlg::TryResolveInitialProductID");
}

//TryAddDetailToBilledList will add a given detail object to the
//m_paryDetailsBilled list if it isn't already in the list
void CInvAllocationUsageDlg::TryAddDetailToBilledList(InvUtils::AllocationDetailInfo *pInfo)
{
	try {

		for(int i=0;i<m_paryDetailsBilled.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pInfoToCheck = (InvUtils::AllocationDetailInfo*)m_paryDetailsBilled.GetAt(i);
			if(pInfoToCheck == pInfo) {
				//it's already in the list, so return
				return;
			}
		}

		//if we get here, it wasn't in the list, so add it
		m_paryDetailsBilled.Add(pInfo);

	}NxCatchAll("Error in CInvAllocationUsageDlg::TryAddDetailToBilledList");
}

//TryRemoveDetailFromBilledList will remove a given detail object from
//the m_paryDetailsBilled list, if it is in the list
void CInvAllocationUsageDlg::TryRemoveDetailFromBilledList(InvUtils::AllocationDetailInfo *pInfo)
{
	try {

		for(int i=m_paryDetailsBilled.GetSize()-1;i>=0;i--) {
			InvUtils::AllocationDetailInfo *pInfoToCheck = (InvUtils::AllocationDetailInfo*)m_paryDetailsBilled.GetAt(i);
			if(pInfoToCheck == pInfo) {
				//remove from the list (do not delete!)
				m_paryDetailsBilled.RemoveAt(i);
				return;
			}
		}

		//if we get here, it wasn't in the list to begin with, which is fine

	}NxCatchAll("Error in CInvAllocationUsageDlg::TryRemoveDetailFromBilledList");
}

//VerifyAllocationCompletedOnClose will see if the allocation is
//fully completed, force it to be completed if not, and return FALSE
//if the user refused to complete the allocation. TRUE otherwise.
BOOL CInvAllocationUsageDlg::VerifyAllocationCompletedOnClose()
{
	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return FALSE;
		}

		//we're forcing them to resolve the allocation, which means
		//every item must be resolved. If any are not resolved, ask
		//the user if they just wish to release the remaining products.

		// (j.jones 2008-05-22 10:26) - PLID 29457 - now we track a count of each status,
		// rather than only whether or not we have any at all
		long nCountActive = 0;
		long nCountUsed = 0;
		long nCountReleased = 0;
		//TES 7/18/2008 - PLID 29478 - We also now have a "To Be Ordered" status
		long nCountOrder = 0;

		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i);
			if(pDetail) {
				//is it active?
				if(pDetail->iadsCurrentStatus == InvUtils::iadsActive) {
					nCountActive++;
				}
				else if(pDetail->iadsCurrentStatus == InvUtils::iadsOrder) {
					nCountOrder++;
				}
				//has it been used?
				else if(pDetail->iadsCurrentStatus == InvUtils::iadsUsed) {
					nCountUsed++;
				}
				else if(pDetail->iadsCurrentStatus == InvUtils::iadsReleased) {
					nCountReleased++;
				}
			}
		}

		//if we only have released products, warn
		if(nCountOrder == 0 && nCountActive == 0 && nCountUsed == 0 && nCountReleased > 0) {
			
			// (j.jones 2008-03-12 11:47) - PLID 29102 - we can't allow this if it is linked to a case history			
			if(m_pAllocationMasterInfo->nCaseHistoryID != -1 || m_bIsCompletedByCaseHistory) {
				AfxMessageBox("This completed allocation is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n"
					"However, this allocation is linked to a case history. A completely released allocation cannot be linked to a case history.\n\n"
					"You must either edit the allocation or case history to unlink the two, or leave a product as not released.");
				return FALSE;
			}
			else {			
				if(IDNO == MessageBox("This completed allocation is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n\n"
					"Are you sure you wish to complete this allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}

		//this allocation is unresolved if there are active items
		//TES 7/18/2008 - PLID 29478 - Also count "To Be Ordered" items.
		if(nCountActive > 0 || nCountOrder > 0) {

			CString strWarn, strOtherNote;
			// (j.jones 2008-02-19 17:40) - PLID 28948 - if not completed by a bill or case,
			// and no items have been used, let the user know they could just save this uncompleted
			// (otherwise it is not an option)
			if(nCountUsed == 0 && !m_bIsCompletedByBill && !m_bIsCompletedByCaseHistory) {
				strOtherNote = "\n\nSince no products have been used on this allocation, you may wish to CANCEL and instead click 'Save and Leave Uncompleted'.";
			};

			// (j.jones 2008-05-22 10:22) - PLID 29457 - reworded to be more concise,
			// and made intelligent, such that "yes" reflects the opposite of what you already marked
			// the majority of your items

			//TES 7/18/2008 - PLID 29478 - The warning also needs to be different if there are any "To Be Ordered" details,
			// as we do not allow those to be released.
			if(nCountOrder == 0) {
				CString strYesValue = "Used";
				CString strNoValue = "Released";
				BOOL bYesMarkUsed = TRUE;

				//The idea here is if they marked some used, and none released, prompt to release the rest
				//but if they marked some released, and none used, prompt to use the rest. However since
				//they can do a combination of both, just look at the higher value, erring on "releasing" the
				//rest if the count is equal.			
				if(nCountUsed > nCountReleased) {
					strYesValue = "Released";
					strNoValue = "Used";
					bYesMarkUsed = FALSE;
				}

				strWarn.Format("Do you wish to mark all unchecked products on this allocation as '%s'?\n\n"
					"Click YES to mark remaining products as '%s'.\n"
					"Click NO to mark remaining products as '%s'.\n"
					"Click CANCEL to cancel to complete these products manually."
					"%s", strYesValue, strYesValue, strNoValue, strOtherNote);

				int nRet = MessageBox(strWarn,"Practice", MB_ICONQUESTION|MB_YESNOCANCEL);

				if(nRet == IDCANCEL) {
					//they don't wish to auto-complete, so tell them why we aren't letting them leave the dialog
					if(nCountUsed == 0 && !m_bIsCompletedByBill && !m_bIsCompletedByCaseHistory) {
						AfxMessageBox("You must mark each product as 'Used' or 'Released' before the allocation can be saved as completed.\n"
							"Alternatively, you may choose to 'Save and Leave Uncompleted' if you are not changing products, or are only releasing them.");
					}
					else {
						AfxMessageBox("You must mark each product as 'Used' or 'Released' before the allocation can be completed.");
					}
					return FALSE;
				}
				else {

					BOOL bMarkUsed = TRUE;

					//now nRet has different meanings:
					//- if bYesMarkUsed is false and nRet is IDYES, then we need to mark remaining products as released
					//- if bYesMarkUsed is true and nRet is IDNO, then we need to mark remaining products as relased
					//- otherwise we mark remaining products as used
					if((!bYesMarkUsed && nRet == IDYES)
						|| (bYesMarkUsed && nRet == IDNO)) {
						bMarkUsed = FALSE;
					}

					//TES 7/21/2008 - PLID 29478 - This function can now fail, the user will be told why.
					if(!ResolveAllUnresolvedProducts(bMarkUsed)) {
						return FALSE;
					}
				}
			}
			else {
				//TES 7/18/2008 - PLID 29478 - We don't allow "To Be Ordered" items to be Released, therefore, don't
				// give them that option.
				int nRet = MsgBox(MB_YESNO, "Do you wish to mark all unchecked products on this allocation as 'Used'?\n\n"
					"Click YES to mark remaining products as 'Used'.\n"
					"Click NO to cancel and complete these products manually.");
				if(nRet == IDNO) {
					//they don't wish to auto-complete, so tell them why we aren't letting them leave the dialog
					if(nCountUsed == 0 && !m_bIsCompletedByBill && !m_bIsCompletedByCaseHistory) {
						AfxMessageBox("You must mark each product as 'Used' or 'Released' before the allocation can be saved as completed.\n"
							"Alternatively, you may choose to 'Save and Leave Uncompleted' if you are not changing products, or are only releasing them.");
					}
					else {
						AfxMessageBox("You must mark each product as 'Used' or 'Released' before the allocation can be completed.");
					}
					return FALSE;
				}
				else {
					//TES 7/21/2008 - PLID 29478 - This function can now fail, the user will be told why.
					if(!ResolveAllUnresolvedProducts(TRUE)) {
						return FALSE;
					}
				}
			}
		}

		//CheckForUnbilledProducts returns TRUE if any are used and unbilled
		if(m_bIsCompletedByBill && CheckForUnbilledProducts()) {

			//don't give the option to auto-bill, a simple warning will do
			if(IDNO == MessageBox("There are still products on this allocation that have been marked as 'Used' but not 'Billed'.\n"
				"Are you sure you wish to complete this allocation without billing these products?",
				"Practice", MB_ICONQUESTION|MB_YESNO)) {
				
				return FALSE;
			}
		}

		return TRUE;

	}NxCatchAll("Error in CInvAllocationUsageDlg::VerifyAllocationCompletedOnClose");

	return FALSE;
}

//CheckForUnbilledProducts will see if there are any products in the list
//marked as used, but not billed
BOOL CInvAllocationUsageDlg::CheckForUnbilledProducts()
{
	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return FALSE;
		}

		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i);
			if(pDetail) {
				//if used and not billed, return TRUE
				// (j.jones 2008-01-07 17:03) - PLID 28479 - only if the product is billable
				if(pDetail->iadsCurrentStatus == InvUtils::iadsUsed && !pDetail->bBilled && pDetail->bIsProductBillable) {
					return TRUE;
				}
			}
		}

	}NxCatchAll("Error in CInvAllocationUsageDlg::CheckForUnbilledProducts");

	return FALSE;
}

//ResolveAllUnresolvedProducts will mark any Active products as Used or Released, based on the parameter
//TES 7/21/2008 - PLID 29478 - This now can return FALSE, in which case the user was told the reason for the failure.
BOOL CInvAllocationUsageDlg::ResolveAllUnresolvedProducts(BOOL bMarkUsed)
{

	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return FALSE;
		}

		// (j.jones 2007-12-13 15:51) - PLID 27988 - if called from a bill or case,
		// prompt if they also want to mark the used items as billed
		BOOL bBillWhenUsed = FALSE;
		if(bMarkUsed && m_bIsCompletedByBill) {

			if(IDYES == MessageBox("Do you wish to also mark items as 'Billed'?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				bBillWhenUsed = TRUE;
			}
		}

		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i);
			if(pDetail) {

				if(bMarkUsed) {
					//if active, mark as Used
					//TES 7/18/2008 - PLID 29478 - Likewise if it's To Be Ordered.
					if(pDetail->iadsCurrentStatus == InvUtils::iadsActive || pDetail->iadsCurrentStatus == InvUtils::iadsOrder) {

						//TES 7/21/2008 - PLID 29478 - If they're being auto-set to Used, and they were previously on Order,
						// then we need to assign a product item for this row.
						if(pDetail->bIsSerialized && pDetail->nProductItemID == -1) {
							IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcDetailObjectPtr, (long)pDetail, NULL, VARIANT_FALSE);
							if(pRow == NULL) {
								AfxThrowNxException("Attempted to update detail which could not be found in list!");
							}
							AfxMessageBox("The product '" + pDetail->strProductName + "' was previously marked 'To Be Ordered', and is now being marked as 'Used'.  You must now choose an item to allocate.");
							if(!PromptSelectItem(pRow)) {
								//TES 7/21/2008 - PLID 29478 - They cancelled.
								return FALSE;
							}
						}

						pDetail->iadsCurrentStatus = InvUtils::iadsUsed;

						// (j.jones 2008-01-07 17:03) - PLID 28479 - only if the product is billable
						if(bBillWhenUsed && pDetail->bIsProductBillable) {
							pDetail->bBilled = TRUE;
							TryAddDetailToBilledList(pDetail);
						}

						//update the datalist accordingly, as we may potentially not close the dialog
						IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcDetailObjectPtr, (long)pDetail, m_AllocationList->GetFirstRow(), FALSE);
						if(pRow == NULL) {
							//this should be impossible
							ASSERT(FALSE);
						}
						else {
							pRow->PutValue(alcUsed, g_cvarTrue);
							pRow->PutValue(alcReleased, g_cvarFalse);

							// (j.jones 2008-01-07 17:03) - PLID 28479 - only if the product is billable
							if(bBillWhenUsed && pDetail->bIsProductBillable) {
								pRow->PutValue(alcBilled, g_cvarTrue);								
							}
						}
					}
				}
				else {
					//TES 7/18/2008 - PLID 29478 - Since we don't allow Releasing items that are To Be Ordered, this function
					// should never be called with bMarkUsed as FALSE if there are any To Be Ordered details on the dialog.
					ASSERT(pDetail->iadsCurrentStatus != InvUtils::iadsOrder);

					//if active, mark as Released
					if(pDetail->iadsCurrentStatus == InvUtils::iadsActive) {
						pDetail->iadsCurrentStatus = InvUtils::iadsReleased;
						pDetail->bBilled = FALSE;

						//update the datalist accordingly, as we may potentially not close the dialog
						IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcDetailObjectPtr, (long)pDetail, m_AllocationList->GetFirstRow(), FALSE);
						if(pRow == NULL) {
							//this should be impossible
							ASSERT(FALSE);
						}
						else {
							pRow->PutValue(alcUsed, g_cvarFalse);
							pRow->PutValue(alcReleased, g_cvarTrue);
							// (j.jones 2008-01-07 17:27) - PLID 28479 - don't change the row status from NULL
							// if not billable
							if(pDetail->bIsProductBillable) {
								pRow->PutValue(alcBilled, g_cvarFalse);
							}
							//TES 6/20/2008 - PLID 26152 - If we're releasing a serialized item to purchased inventory, then
							// they can flag it to be returned, so load that preference.  Otherwise, set the column to blank.
							if((InvUtils::ProductItemStatus)VarLong(pDetail->varProductItemStatus, -1) == InvUtils::pisPurchased &&
								pDetail->nProductItemID != -1) {
								BOOL bReturn = GetRemotePropertyInt("ReturnReleasedGeneralSerializedItems", 1, 0, "<None>", true);
								pRow->PutValue(alcReturn, bReturn?g_cvarTrue:g_cvarFalse);
								pDetail->bToBeReturned = bReturn;
							}
							else {
								pRow->PutValue(alcReturn, g_cvarNull);
							}
						}
					}
				}
			}
		}
		return TRUE;

	}NxCatchAll("Error in CInvAllocationUsageDlg::ResolveAllUnresolvedProducts");
	return FALSE;
}

void CInvAllocationUsageDlg::OnRadioBarcodeUsed() 
{
	OnRadioBarcodeChanged();
}

void CInvAllocationUsageDlg::OnRadioBarcodeReleased() 
{
	OnRadioBarcodeChanged();
}

void CInvAllocationUsageDlg::OnRadioBarcodeChanged()
{
	try {

		//save this setting into ConfigRT immediately
		
		BOOL bBarcodeUsedItems = TRUE;

		if(m_radioBarcodeReleased.GetCheck()) {
			bBarcodeUsedItems = FALSE;
		}

		//now track this as the last barcode option this user had set
		SetRemotePropertyInt("AllocationUsage_BarcodeUsedItems", bBarcodeUsedItems ? 1 : 0, 0, GetCurrentUserName());

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnRadioBarcodeChanged");
}


// (j.jones 2007-12-13 08:57) - PLID 27988 - If we are trying to bill a product,
// and the bill is requiring a specific product, ensure we are only billing
// one type of ProductID, not more than one. If so, warn (if !bSilent), and return FALSE;
BOOL CInvAllocationUsageDlg::CanBillThisProduct(long nProductID, BOOL bSilent /*= FALSE*/)
{
	try {

		//if we are checking the "Billed" box, and we aren't forcing the user to
		//complete the allocation yet, and a m_nInitialProductID exists,
		//then we don't have to enforce that product ID, but we do have to enforce
		//that only ONE product ID is used in this edit session (multiple product
		//items are OK), so check what has been marked as "billed" in this edit
		//session
		if(!m_bForceCompletion && m_nInitialProductID != -1) {

			if(!m_bIsCompletedByBill) {
				//this should not be possible, means the dialog was called poorly
				ASSERT(FALSE);
			}

			//find the first product ID used in this session
			if(m_paryDetailsBilled.GetSize() > 0) {
				InvUtils::AllocationDetailInfo *pInfo = (InvUtils::AllocationDetailInfo*)(m_paryDetailsBilled.GetAt(0));
				long nProductIDToCheck = pInfo->nProductID;
				if(pInfo && nProductIDToCheck != nProductID) {
					//they're trying to select a different product, so tell them they cannot
					if(!bSilent) {
						CString strWarn;
						strWarn.Format("You have already chosen the product '%s' to be billed from this allocation "
							"in response to the '%s' product currently being added to the bill.\n\n"
							"You cannot select more than one type of product from this allocation while resolving the '%s' product.\n\n"
							"You will be given the opportunity to fully resolve this allocation once the "
							"current batch of products is finished adding to the bill.",
							pInfo->strProductName, m_strInitialProductName,
							m_strInitialProductName);
						AfxMessageBox(strWarn);
					}
					return FALSE;
				}
			}
		}

		return TRUE;

	}NxCatchAll("Error in CInvAllocationUsageDlg::CanBillThisProduct");

	return FALSE;
}

// (j.jones 2008-02-19 14:23) - PLID 28948 - added OnBtnSaveAndComplete
void CInvAllocationUsageDlg::OnBtnSaveAndComplete() 
{
	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		//this dialog should never be called in such a way that
		//we are saving to data, but NOT forcing a completion
		if(m_bSaveToData && !m_bForceCompletion) {
			ASSERT(FALSE);
			//force it anyways if we get here
			m_bForceCompletion = TRUE;
		}

		if(m_bForceCompletion) {
			//force the allocation to be completed, return if not successful
			if(!VerifyAllocationCompletedOnClose()) {
				return;
			}
		}
		else {
			//otherwise, they must select an item to satisfy the requested product
			if(m_paryDetailsBilled.GetSize() == 0) {
				CString strWarn;
				if(m_nInitialProductID != -1) {

					if(!m_bIsCompletedByBill) {
						//this should not be possible, means the dialog was called poorly
						ASSERT(FALSE);
					}

					strWarn.Format("You cannot save this allocation without first marking a product as billed, in order to satisfy the '%s' charge on this bill.",
						m_strInitialProductName);
				}
				else {
					strWarn.Format("You cannot save this allocation without first marking a product as billed.");
				}
				AfxMessageBox(strWarn);
				return;
			}
		}

		//if true, we must save the results to data
		if(m_bSaveToData) {

			if(!SaveCompleted()) {
				return;
			}
		}

		CDialog::OnOK();

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnBtnSaveAndComplete");
}

// (j.jones 2008-02-19 14:23) - PLID 28948 - added OnBtnSaveLeaveUncompleted
void CInvAllocationUsageDlg::OnBtnSaveLeaveUncompleted() 
{
	try {

		if(m_pAllocationMasterInfo == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(m_bIsCompletedByCaseHistory && m_bIsCompletedByBill) {
			AfxMessageBox("You cannot save this allocation as uncompleted when using it on a bill or case history.\n"
				"You must cancel this allocation instead.");
			return;
		}

		BOOL bHasActive = FALSE;
		BOOL bHasReleased = FALSE;
		BOOL bHasUsed = FALSE;
		for(int i=0;i<m_pAllocationMasterInfo->paryAllocationDetailInfo.GetSize();i++) {
			InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)m_pAllocationMasterInfo->paryAllocationDetailInfo.GetAt(i);
			if(pDetail) {
				//is it active?
				//TES 7/18/2008 - PLID 29478 - Also count "To Be Ordered" items.
				if(pDetail->iadsCurrentStatus == InvUtils::iadsActive || pDetail->iadsCurrentStatus == InvUtils::iadsOrder) {
					bHasActive = TRUE;
				}
				//has it been released?
				else if(pDetail->iadsCurrentStatus == InvUtils::iadsReleased) {
					bHasReleased = TRUE;
				}
				//has it been used?
				else if(pDetail->iadsCurrentStatus == InvUtils::iadsUsed) {
					bHasUsed = TRUE;
				}
			}
		}

		if(bHasUsed) {
			AfxMessageBox("You cannot save this allocation as uncompleted when products have been used.\n"
				"You may only cancel the changes, or save the allocation as completed.");
			return;
		}
		else if(bHasReleased && !bHasActive) {
			AfxMessageBox("You cannot save this allocation as uncompleted when all the products have been released.\n"
				"You may only cancel the changes, or save the allocation as completed.");
			return;
		}

		//if true, we must save the results to data
		if(m_bSaveToData) {

			if(!SaveUncompleted()) {
				return;
			}
		}

		CDialog::OnOK();

	}NxCatchAll("Error in CInvAllocationUsageDlg::OnBtnSaveLeaveUncompleted");
}
