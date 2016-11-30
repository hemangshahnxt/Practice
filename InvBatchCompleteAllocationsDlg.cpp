// InvBatchCompleteAllocationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvBatchCompleteAllocationsDlg.h"
#include "InvUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "barcode.h"
#include "InvAllocationUsageDlg.h"
#include "GlobalSchedUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-12-04 10:05) - PLID 28269 - created

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

/////////////////////////////////////////////////////////////////////////////
// CInvBatchCompleteAllocationsDlg dialog

enum MultiAllocationListColumn {

	malcID = 0,
	malcParentID,
	malcAllocationID,
	malcDetailID,
	malcPersonID,
	malcUserDefinedID,
	malcPatientName,
	malcProductID,
	malcProductName,
	malcCurQuantity,
	malcOldQuantity,
	malcSerialNumber,
	malcExpDate,
	malcUsed,
	malcReleased,
	malcReturn, //TES 6/20/2008 - PLID 26152 - Added To Be Returned flag
	malcCompleted,
	malcNotes,
	malcOldNotes,
	malcApptDate,
	malcApptID,	// (j.jones 2008-03-24 17:15) - PLID 29388
	malcCaseHistoryID,	// (j.jones 2008-03-12 12:11) - PLID 29102	
	malcProductItemID,		//TES 6/20/2008 - PLID 26152 - ProductItemID and ProductItemStatus used to calculate ToBeReturned
	malcProductItemStatus,
	malcIsSerialized,	//TES 7/18/2008 - PLID 29478 - Track whether the product is serialized.
	malcLocationID,		//TES 7/18/2008 - PLID 29478 - Track this allocation's location.
	malcOldStatus,		//TES 7/18/2008 - PLID 29478 - Track the original status (it no longer was necessarily Active)
};

CInvBatchCompleteAllocationsDlg::CInvBatchCompleteAllocationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvBatchCompleteAllocationsDlg::IDD, pParent)
{
	m_bDisableBarcode = FALSE;
	//{{AFX_DATA_INIT(CInvBatchCompleteAllocationsDlg)
	m_nNextNewDetailID = -2; //intentionally starting at -2, not -1
	//}}AFX_DATA_INIT
}


void CInvBatchCompleteAllocationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvBatchCompleteAllocationsDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_RADIO_BATCH_BARCODE_USED, m_radioBarcodeUsed);
	DDX_Control(pDX, IDC_RADIO_BATCH_BARCODE_RELEASED, m_radioBarcodeReleased);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvBatchCompleteAllocationsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvBatchCompleteAllocationsDlg)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_RADIO_BATCH_BARCODE_USED, OnRadioBatchBarcodeUsed)
	ON_BN_CLICKED(IDC_RADIO_BATCH_BARCODE_RELEASED, OnRadioBatchBarcodeReleased)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvBatchCompleteAllocationsDlg message handlers

BOOL CInvBatchCompleteAllocationsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-29 10:52) - PLID 29820 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
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

		m_AllocationsList = BindNxDataList2Ctrl(this, IDC_MULTI_ALLOCATION_LIST, GetRemoteData(), false);

		//set the from clause from code so it's easier to maintain,
		//and can use InvUtils defines
		// (j.jones 2008-03-12 12:14) - PLID 29102 - added CaseHistoryID
		// (j.jones 2008-03-24 17:21) - PLID 29388 - added AppointmentID
		//TES 6/20/2008 - PLID 26152 - Added ToBeReturned, ProductItemID, and ProductItemStatus
		//TES 7/18/2008 - PLID 29478 - Added IsSerialized, LocationID, and Status
		CString strFrom;
		strFrom.Format("(SELECT PatientInvAllocationsT.ID, NULL AS ParentID, "
			"PatientInvAllocationsT.ID AS AllocationID, NULL AS DetailID, "
			"PatientsT.PersonID, PatientsT.UserDefinedID, "
			"Last + ', ' + First + ' ' + Middle AS PatientName, "
			"NULL AS ProductID, NULL AS ProductName, NULL AS Quantity, NULL AS SerialNum, NULL AS ExpDate, "
			"NULL AS Used, NULL AS Released, NULL AS ToBeReturned, 'No' AS Completed, "
			"LocationsT.Name AS Notes, " //no need to show the alloc. notes, so show the loc. name instead
			"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime, "
			"AppointmentsT.ID AS AppointmentID, "
			"CaseHistoryAllocationLinkT.CaseHistoryID, "
			"NULL AS ProductItemID, NULL AS ProductItemStatus, NULL AS IsSerialized, "
			"PatientInvAllocationsT.LocationID, PatientInvAllocationsT.Status "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN UsersT ON PatientInvAllocationsT.CompletedBy = UsersT.PersonID "
			"LEFT JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
			"WHERE PatientInvAllocationsT.Status = %li "
			"AND (AppointmentsT.ID Is Null OR CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) <= GetDate()) "
			""
			"UNION SELECT NULL AS ID, PatientInvAllocationDetailsT.AllocationID AS ParentID, "
			"PatientInvAllocationDetailsT.AllocationID, PatientInvAllocationDetailsT.ID AS DetailID, "
			"PatientInvAllocationsT.PatientID AS PersonID, NULL AS UserDefinedID, "
			"ServiceT.Name + CASE WHEN Quantity <> 1.0 THEN ' (' + Convert(nvarchar, Quantity) + ')' ELSE '' END AS ProductDesc, "
			"ServiceT.ID AS ProductID, ServiceT.Name AS ProductName, Quantity, SerialNum, ExpDate, "
			"Convert(bit,0) AS Used, Convert(bit,0) AS Released, NULL AS ToBeReturned, NULL AS Completed, "
			"PatientInvAllocationDetailsT.Notes, NULL AS ApptDateTime, NULL AS AppointmentID, NULL AS CaseHistoryID, "
			"ProductItemsT.ID AS ProductID, COALESCE(ProductItemsT.Status,0) AS ProductItemStatus, "
			"Convert(bit,CASE WHEN ProductT.HasSerialNum = 1 OR ProductT.HasExpDate = 1 THEN 1 ELSE 0 END) AS IsSerialized, "
			"PatientInvAllocationsT.LocationID, PatientInvAllocationDetailsT.STatus "
			"FROM PatientInvAllocationDetailsT "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN ProductT ON PatientInvAllocationDetailsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"WHERE PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status IN (%li, %li) "
			//needs same filter as above so we won't have leftover children
			"AND (AppointmentsT.ID Is Null OR CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) <= GetDate())"
			") AS AllocationsQ",
			InvUtils::iasActive, InvUtils::iasActive, InvUtils::iadsActive, InvUtils::iadsOrder);

		m_AllocationsList->PutFromClause(_bstr_t(strFrom));
		m_AllocationsList->Requery();

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CInvBatchCompleteAllocationsDlg::~CInvBatchCompleteAllocationsDlg() 
{
	try {

		//unregister for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::~CInvBatchCompleteAllocationsDlg");
}

void CInvBatchCompleteAllocationsDlg::OnOK() 
{
	try {

		if(!Save()) {
			return;
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::OnOK");
	
	CDialog::OnOK();
}

BOOL CInvBatchCompleteAllocationsDlg::Save() 
{
	long nAuditTransactionID = -1;
	long nInvTodoTransactionID = -1; // (c.haag 2008-02-27 16:54) - PLID 29115 - Used for inventory todo alarm generation

	try {

		//validate that all changed allocations are resolved
		//(will warn appropriately)
		if(!VerifyAllocationsResolved()) {
			return FALSE;
		}

		//save the changes to each allocation

		CString strSqlBatch = BeginSqlBatch();

		// (c.haag 2008-02-27 13:32) - PLID 29115 - Begin to track information for 
		// generating inventory todo alarms
		nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();

		//loop through each parent allocation

		IRowSettingsPtr pParentRow = m_AllocationsList->GetFirstRow();
		while(pParentRow) {

			//we only save changed allocations,
			//but we still need to save if the notes changed on something
			BOOL bSaveThisAllocation = FALSE;

			long nAllocationID = VarLong(pParentRow->GetValue(malcAllocationID), -1);
			CString strPatientName = VarString(pParentRow->GetValue(malcPatientName), "");
			long nPatientID = VarLong(pParentRow->GetValue(malcPersonID), -1);
			

			//for this allocation, loop through each child detail

			// (j.jones 2008-02-20 09:13) - PLID 28948 - need to know whether it can be saved
			// as a completed allocation or not

			//first let's confirm whether it can be saved
			BOOL bHasActive = FALSE;
			BOOL bHasUsed = FALSE;
			BOOL bHasReleased = FALSE;
			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while(pChildRow) {

				//determine the status
				BOOL bUsed = VarBool(pChildRow->GetValue(malcUsed), FALSE);
				BOOL bReleased = VarBool(pChildRow->GetValue(malcReleased), FALSE);

				if(bUsed) {
					bHasUsed = TRUE;
				}
				else if(bReleased) {
					bHasReleased = TRUE;
				}
				else {
					bHasActive = TRUE;
				}

				//move to the next detail
				pChildRow = pChildRow->GetNextRow();
			}

			if(bHasUsed && bHasActive) {
				//VerifyAllocationsResolved should have rendered this impossible
				ASSERT(FALSE);
				ThrowNxException("Error: Tried to save a partially completed allocation!");
			}

			//now determine whether we save this as completed
			BOOL bSaveCompleted = !bHasActive;
			
			//now re-loop through the details
			pChildRow = pParentRow->GetFirstChildRow();
			while(pChildRow) {

				long nDetailID = VarLong(pChildRow->GetValue(malcDetailID), -1);

				//determine the status
				BOOL bUsed = VarBool(pChildRow->GetValue(malcUsed), FALSE);
				BOOL bReleased = VarBool(pChildRow->GetValue(malcReleased), FALSE);

				//TES 6/20/2008 - PLID 26152 - We only check the ToBeReturned flag if this is a released, serialized, 
				// Purchased Inventory item.
				BOOL bCheckedToBeReturned = FALSE;
				BOOL bToBeReturned = FALSE;
				long nProductItemID = VarLong(pChildRow->GetValue(malcProductItemID),-1);
				if(bReleased &&  nProductItemID != -1 &&
					(InvUtils::ProductItemStatus)VarLong(pChildRow->GetValue(malcProductItemStatus),-1) == InvUtils::pisPurchased) {
					bCheckedToBeReturned = TRUE;
					bToBeReturned = VarBool(pChildRow->GetValue(malcReturn),FALSE);
				}

				//TES 7/18/2008 - PLID 29478 - Pull the old status.  If it was "To Be Ordered", but it has a ProductItemID now,
				// go ahead and save it as Active.
				InvUtils::InventoryAllocationDetailStatus iadsStatus = (InvUtils::InventoryAllocationDetailStatus)VarLong(pChildRow->GetValue(malcOldStatus));
				if(iadsStatus == InvUtils::iadsOrder) {
					if(VarLong(pChildRow->GetValue(malcProductItemID),-1) != -1) {
						iadsStatus = InvUtils::iadsActive;
						bSaveThisAllocation = TRUE;
					}
				}
				if(bUsed) {
					iadsStatus = InvUtils::iadsUsed;

					bSaveThisAllocation = TRUE;
				}
				else if(bReleased) {
					iadsStatus = InvUtils::iadsReleased;

					bSaveThisAllocation = TRUE;
				}

				//now grab the notes they entered, if any
				CString strNotes = VarString(pChildRow->GetValue(malcNotes), "");
				strNotes.TrimLeft();
				strNotes.TrimRight();
				CString strOldNotes = VarString(pChildRow->GetValue(malcOldNotes), "");
				strOldNotes.TrimLeft();
				strOldNotes.TrimRight();

				double dblOriginalQty = VarDouble(pChildRow->GetValue(malcOldQuantity), 1.0);
				double dblCurQuantity = VarDouble(pChildRow->GetValue(malcCurQuantity), 1.0);

				CString strDesc;
				//strDesc is needed if we're completing or if notes change
				if(bSaveThisAllocation || strNotes != strOldNotes) {
					_variant_t varSerialNumber = pChildRow->GetValue(malcSerialNumber);
					_variant_t varExpDate = pChildRow->GetValue(malcExpDate);					
					CString strProductName = VarString(pChildRow->GetValue(malcProductName), "");
					
					strDesc = strProductName;
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
					if(dblOriginalQty != 1.0) {
						//only show the quantity if not 1.0
						CString str;
						str.Format(", Quantity: %g", dblOriginalQty);
						strDesc += str;
					}
				}

				//now, should we be completing the detail?
				if(bSaveThisAllocation) {	

					if(nDetailID < 0) {

						//create a new detail

						long nProductID = VarLong(pChildRow->GetValue(malcProductID), -1);

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationDetailsT "
							"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes) "
							"VALUES (%li, %li, NULL, %g, %li, '%s')", nAllocationID, nProductID,
							dblCurQuantity, iadsStatus, _Q(strNotes));

						if(nProductItemID != -1) {
							//TES 6/20/2008 - PLID 26152 - Save the ToBeReturned flag
							AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = %i WHERE ID = %li",
								bToBeReturned ? 1 : 0, nProductItemID);
						}

						// (j.jones 2007-12-07 15:37) - PLID 28043 - audit the creation
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailCreated, nAllocationID, "", strDesc, aepMedium, aetCreated);

						//TES 7/18/2008 - PLID 29478 - Count "To Be Ordered" as active.
						if(iadsStatus != InvUtils::iadsActive && iadsStatus != InvUtils::iadsOrder) {
							//if we just created a new detail, it might not be active
							//create a separate audit record to show the status change

							CString strOldValue, strNewValue;
							AuditEventItems aeiItem = aeiInvAllocationActive;
							
							strOldValue = strDesc;
							strOldValue += ", Status: <New>";

							if(iadsStatus == InvUtils::iadsUsed) {
								strNewValue = "Status: Used";
								aeiItem = aeiInvAllocationDetailUsed;
							}
							else if(iadsStatus == InvUtils::iadsReleased) {
								//TES 6/20/2008 - PLID 26152 - If we updated the ToBeReturned flag, include it in the auditing.
								if(bCheckedToBeReturned && bToBeReturned) {
									strNewValue = "Status: Released - To Be Returned";
								}
								else {
									strNewValue = "Status: Released";
								}
								aeiItem = aeiInvAllocationDetailReleased;
							}
							else {
								//should be impossible
								ASSERT(FALSE);
							}
							AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiItem, nAllocationID, strOldValue, strNewValue, aepMedium, aetChanged);
						}
					}
					else {

						//update the detail in data
						//TES 7/21/2008 - PLID 29478 - The ProductItemID may have changed.
						long nProductItemID = VarLong(pChildRow->GetValue(malcProductItemID),-1);
						CString strProductItemID = (nProductItemID == -1 ? "NULL" : AsString(nProductItemID));
						AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Status = %li, Quantity = %g, Notes = '%s', ProductItemID = %s WHERE ID = %li", iadsStatus, dblCurQuantity, _Q(strNotes), strProductItemID, nDetailID);

						if(nProductItemID != -1) {
							//TES 6/20/2008 - PLID 26152 - Save the ToBeReturned flag
							AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = %i WHERE ID = %li",
								bToBeReturned ? 1 : 0, nProductItemID);
						}

						// (j.jones 2007-12-04 17:04) - PLID 28043 - audit the status change
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}

						CString strOldValue, strNewValue;
						AuditEventItems aeiItem = aeiInvAllocationDetailUsed;

						strOldValue = strDesc + ", Status: Active";

						if(iadsStatus == InvUtils::iadsActive) {
							//should be impossible if saving as completed
							ASSERT(!bSaveCompleted);
							strNewValue = "Status: Active";
							aeiItem = aeiInvAllocationDetailActive;
						}
						else if(iadsStatus == InvUtils::iadsUsed) {
							//should be impossible if saving as uncompleted
							ASSERT(bSaveCompleted);
							strNewValue = "Status: Used";
							aeiItem = aeiInvAllocationDetailUsed;
						}
						else if(iadsStatus == InvUtils::iadsReleased) {
							//TES 6/20/2008 - PLID 26152 - If we updated the ToBeReturned flag, include it in the auditing.
							if(bCheckedToBeReturned && bToBeReturned) {
								strNewValue = "Status: Released - To Be Returned";
							}
							else {
								strNewValue = "Status: Released";
							}
							aeiItem = aeiInvAllocationDetailReleased;
						}
						//TES 7/18/2008 - PLID 29478 - New status
						else if(iadsStatus == InvUtils::iadsOrder) {
							//should be impossible if saving as completed
							ASSERT(!bSaveCompleted);
							strNewValue = "Status: To Be Ordered";
							aeiItem = aeiInvAllocationDetailToBeOrdered;
						}
						else {
							//should be impossible
							ASSERT(FALSE);
						}
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiItem, nAllocationID, strOldValue, strNewValue, aepMedium, aetChanged);

						//audit the notes, if they were changed
						if(strNotes != strOldNotes) {
							strOldValue = strDesc + ", Notes: " + strOldNotes;
							AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailNotes, nAllocationID, strOldValue, strNotes, aepMedium, aetChanged);
						}

						//audit the quantity change
						if(dblCurQuantity != dblOriginalQty) {
							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailQuantity, nAllocationID, strDesc, AsString(dblCurQuantity), aepMedium, aetChanged);
						}
					}

					// (c.haag 2008-02-27 16:50) - PLID 29115 - Add this element to the todo alarm transaction
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_ProductID, VarLong(pChildRow->GetValue(malcProductID)));

				} // if(bSaveThisAllocation) {
				else if(nDetailID > 0 && strNotes != strOldNotes) {
					//even if not completing the detail, if the notes changed, we have to save that change
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Notes = '%s' WHERE ID = %li", _Q(strNotes), nDetailID);
					CString strOldValue = strDesc + ", Notes: " + strOldNotes;
					// (j.jones 2007-12-06 10:50) - PLID 28043 - audit this
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailNotes, nAllocationID, strOldValue, strNotes, aepMedium, aetChanged);
				}

				//move to the next detail
				pChildRow = pChildRow->GetNextRow();
			}

			//now should we complete the allocation?

			// (j.jones 2008-02-20 09:13) - PLID 28948 - need to know whether it can be saved
			// as a completed allocation or not
			if(bSaveThisAllocation && bSaveCompleted) {

				//mark this allocation as completed
				AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationsT "
					"SET Status = %li, CompletionDate = GetDate(), CompletedBy = %li "
					"WHERE ID = %li", InvUtils::iasCompleted, GetCurrentUserID(), nAllocationID);

				// (j.jones 2007-12-04 16:55) - PLID 28043 - audit the status change
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCompleted, nAllocationID, "Active", "Completed", aepMedium, aetChanged);

				// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
				long nAppointmentID = VarLong(pParentRow->GetValue(malcApptID), -1);
				if(nAppointmentID != -1) {
					TrySendAppointmentTablecheckerForInventory(nAppointmentID, TRUE);
				}
			}

			//move to the next allocation
			pParentRow = pParentRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		// (j.jones 2007-12-04 16:55) - PLID 28043 - audit if needed
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		// (c.haag 2008-02-27 16:53) - PLID 29115 - Update inventory todo lists
		if (-1 != nInvTodoTransactionID) {
			//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
			InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
		}

		return TRUE;

	}NxCatchAllCall("Error in CInvBatchCompleteAllocationsDlg::Save",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		// (c.haag 2008-02-27 17:30) - PLID 29115 - Also roll back inventory todo transactions
		if(nInvTodoTransactionID != -1) {
			InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
		}
	);

	return FALSE;
}

void CInvBatchCompleteAllocationsDlg::OnRadioBatchBarcodeUsed() 
{
	OnRadioBarcodeToggle();
}

void CInvBatchCompleteAllocationsDlg::OnRadioBatchBarcodeReleased() 
{
	OnRadioBarcodeToggle();
}

void CInvBatchCompleteAllocationsDlg::OnRadioBarcodeToggle()
{
	try {

		//save this setting into ConfigRT immediately
		
		BOOL bBarcodeUsedItems = TRUE;

		if(m_radioBarcodeReleased.GetCheck()) {
			bBarcodeUsedItems = FALSE;
		}

		//now track this as the last barcode option this user had set
		SetRemotePropertyInt("AllocationUsage_BarcodeUsedItems", bBarcodeUsedItems ? 1 : 0, 0, GetCurrentUserName());

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::OnRadioBarcodeToggle");
}

BEGIN_EVENTSINK_MAP(CInvBatchCompleteAllocationsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvBatchCompleteAllocationsDlg)
	ON_EVENT(CInvBatchCompleteAllocationsDlg, IDC_MULTI_ALLOCATION_LIST, 10 /* EditingFinished */, OnEditingFinishedMultiAllocationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvBatchCompleteAllocationsDlg, IDC_MULTI_ALLOCATION_LIST, 8 /* EditingStarting */, OnEditingStartingMultiAllocationList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInvBatchCompleteAllocationsDlg, IDC_MULTI_ALLOCATION_LIST, 9 /* EditingFinishing */, OnEditingFinishingMultiAllocationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvBatchCompleteAllocationsDlg::OnEditingStartingMultiAllocationList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == malcNotes) {
			//if the parent row, disallow editing
			if(pRow->GetParentRow() == NULL) {
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll("CInvBatchCompleteAllocationsDlg::OnEditingStartingMultiAllocationList");
}

CString CInvBatchCompleteAllocationsDlg::GetExistingProductItemWhereClause(long nServiceID)
{
	//TES 7/18/2008 - PLID 29478 - returns a query of AND statements so if and when we pop up the ProductItemsDlg, any 
	// items on this dialog aren't included on it.

	try {

		CString strIDList = "";

		//find each row with the matching service ID, see if it has
		//a ProductItemID, and isn't "released".
		//if so, use it to build our where clause
		IRowSettingsPtr pRow = m_AllocationsList->GetFirstRow();
		while(pRow != NULL) {
			
			long nServiceIDToCheck = VarLong(pRow->GetValue(malcProductID), -1);
			BOOL bReleased = VarBool(pRow->GetValue(malcReleased), FALSE);
			if(nServiceIDToCheck == nServiceID && !bReleased) {
				//this is the right product, now check its ProductItemID
				long nProductItemID = VarLong(pRow->GetValue(malcProductItemID), -1);
				if(nProductItemID != -1) {
					//we have one, so add it to our where clause
					if(!strIDList.IsEmpty()) {
						strIDList += ",";
					}
					strIDList += AsString(nProductItemID);
				}
			}

			//TES 8/18/2008 - PLID 29478 - Use FindAbsoluteNextRow(), not GetNextRow(), otherwise you'll only get the top-level
			// rows!
			pRow = m_AllocationsList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}

		//now build the clause to return
		if(!strIDList.IsEmpty()) {
			CString strWhere;
			strWhere.Format("ProductItemsT.ID NOT IN (%s)", strIDList);
			return strWhere;
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::GetExistingProductItemWhereClause");

	return "";
}

void CInvBatchCompleteAllocationsDlg::OnEditingFinishingMultiAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == malcNotes) {
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

		//if checking used or released (not unchecking), see if the detail has a quantity <> 1.0,
		//if so, prompt for how much to use or release, with the option to cancel
		//(don't change the status here, that is handled in OnEditingFinished)
		if((nCol == malcUsed || nCol == malcReleased) && pvarNewValue->vt == VT_BOOL && VarBool(pvarNewValue, FALSE)) 
		{
			//TES 7/18/2008 - PLID 29478 - If this is a serialized item, and it doesn't have a ProductItemID, we need to give
			// it on (it was "To Be Ordered" before).
			if(VarBool(pRow->GetValue(malcIsSerialized)) && VarLong(pRow->GetValue(malcProductItemID),-1) == -1) {
				if(nCol == malcReleased) {
					//TES 7/18/2008 - PLID 29478 - We don't allow this change.
					*pvarNewValue = varOldValue;
					*pbCommit = FALSE;
					AfxMessageBox("You cannot release a product that is 'To Be Ordered.'  Please select a different status, or delete this product from the allocation.");
					return;
				}
				AfxMessageBox("This product was previously flagged as 'To Be Ordered', you must now choose an item to allocate.");
				//prompt
				long nProductID = VarLong(pRow->GetValue(malcProductID));
				
				CProductItemsDlg dlg(this);
				dlg.m_EntryType = PI_SELECT_DATA;
				dlg.m_strOverrideTitleBarText = "Select Item to be Allocated";
				dlg.m_strOverrideSelectQtyText = "Quantity to be allocated:";
				dlg.m_ProductID = nProductID;
				dlg.m_nLocationID = VarLong(pRow->GetValue(malcLocationID));
				dlg.m_CountOfItemsNeeded = 1;
				dlg.m_strWhere = GetExistingProductItemWhereClause(nProductID);
				dlg.m_bDisallowQtyChange = TRUE;

				//disable barcode messages on this screen while the product items dialog is up
				m_bDisableBarcode = TRUE;

				if(IDCANCEL == dlg.DoModal()) {
					//re-enable barcoding
					m_bDisableBarcode = FALSE;
					AfxMessageBox("You cannot change this item's status without selecting an item to allocate.");
					*pvarNewValue = varOldValue;
					*pbCommit= FALSE;
					return;
				}
				//loop through dlg.m_adwProductItemIDs and add to the list
				ASSERT(dlg.m_adwProductItemIDs.GetSize() == 1);

				_variant_t varProductItemID = (long)(dlg.m_adwProductItemIDs.GetAt(0));
				_variant_t varSerialNumber = dlg.GetSelectedProductItemSerialNum(0);
				_variant_t varExpDate = dlg.GetSelectedProductItemExpDate(0);
				pRow->PutValue(malcProductItemID, varProductItemID);
				pRow->PutValue(malcSerialNumber, varSerialNumber);
				pRow->PutValue(malcExpDate, varExpDate);
				
				//re-enable barcoding
				m_bDisableBarcode = FALSE;
			}

			//make sure this isn't a parent
			if(pRow->GetParentRow() == NULL) {
				ASSERT(FALSE);
				ThrowNxException("Tried to edit used/released status on parent row!");
			}

			double dblCurQuantity = VarDouble(pRow->GetValue(malcCurQuantity), 1.0);
			
			if(dblCurQuantity > 0.0 && dblCurQuantity != 1.0) {
				
				//okay they have a quantity that is not 1.0 (remember, 0.5 is something we will prompt on),
				//so ask them how much to use or return

				CString strProductName = VarString(pRow->GetValue(malcProductName), "");
				double dblNewQuantity = 0.0;

				if(!PromptForQtyChange(nCol == malcUsed, strProductName, dblCurQuantity, dblNewQuantity)) {
					//nothing was committed, so don't let this column remain checked
					*pbCommit = FALSE;
					return;
				}

				if(dblNewQuantity == 0.0 || dblNewQuantity > dblCurQuantity) {
					//shouldn't be possible
					ASSERT(FALSE);
					*pbCommit = FALSE;
					return;
				}

				//if they are approving the whole quantity, then we can continue normally
				if(dblNewQuantity == dblCurQuantity) {
					return;
				}

				//if we get here, they are only using/releasing some of the original quantity,
				//which means we need to change the current detail's quantity, and create a new
				//detail with the balance, marking the new detail with the opposite status
				//as the current one

				double dblBalance = dblCurQuantity - dblNewQuantity;

				//update the current quantity
				dblCurQuantity = dblNewQuantity;

				//and update in the datalist
				pRow->PutValue(malcCurQuantity, dblNewQuantity);

				//also recreate the product name, with the new quantity
				CString strNewName = strProductName;
				if(dblCurQuantity != 1.0) {
					strNewName.Format("%s (%g)", strProductName, dblCurQuantity);
				}
				pRow->PutValue(malcPatientName, _bstr_t(strNewName));

				//now we need to create the new datalist row, it will copy most of the data
				//from the existing row
				IRowSettingsPtr pNewRow = m_AllocationsList->GetNewRow();				

				//generate a new name, with the new quantity
				strNewName = strProductName;
				if(dblBalance != 1.0) {
					strNewName.Format("%s (%g)", strProductName, dblBalance);
				}

				pNewRow->PutValue(malcID, pRow->GetValue(malcID));
				pNewRow->PutValue(malcParentID, pRow->GetValue(malcParentID));
				pNewRow->PutValue(malcAllocationID, pRow->GetValue(malcAllocationID));
				//instead of setting to -1, we track unique negative IDs, starting with -2					
				pNewRow->PutValue(malcDetailID, m_nNextNewDetailID);
				m_nNextNewDetailID--;
				pNewRow->PutValue(malcPersonID, pRow->GetValue(malcPersonID));
				pNewRow->PutValue(malcUserDefinedID, pRow->GetValue(malcUserDefinedID));
				pNewRow->PutValue(malcPatientName, _bstr_t(strNewName));				
				pNewRow->PutValue(malcProductID, pRow->GetValue(malcProductID));
				pNewRow->PutValue(malcProductName, _bstr_t(strProductName));
				//shouldn't be possible for the serialized stuff to be non-null
				pNewRow->PutValue(malcSerialNumber, g_cvarNull);
				pNewRow->PutValue(malcExpDate, g_cvarNull);
				pNewRow->PutValue(malcCompleted, pRow->GetValue(malcCompleted));
				pNewRow->PutValue(malcNotes, pRow->GetValue(malcNotes));
				pNewRow->PutValue(malcOldNotes, "");
				pNewRow->PutValue(malcApptDate, pRow->GetValue(malcApptDate));
				// (j.jones 2008-03-12 12:12) - PLID 29102 - added CaseHistoryID
				pNewRow->PutValue(malcCaseHistoryID, pRow->GetValue(malcCaseHistoryID));
				// (j.jones 2008-03-24 17:22) - PLID 29388 - added AppointmentID
				pNewRow->PutValue(malcApptID, pRow->GetValue(malcApptID));

				//now for quantity, it should create with our balance
				pNewRow->PutValue(malcCurQuantity, dblBalance);
				pNewRow->PutValue(malcOldQuantity, dblBalance);

				//and status should be the opposite of what we checked
				pNewRow->PutValue(malcUsed, nCol == malcUsed ? g_cvarFalse : g_cvarTrue);
				pNewRow->PutValue(malcReleased, nCol == malcUsed ? g_cvarTrue: g_cvarFalse);

				//TES 7/21/2008 - PLID 26152 - Fill the Return and ProductItemID columns
				if(nCol == malcReleased && VarLong(pNewRow->GetValue(malcProductItemID),-1) != -1 &&
					(InvUtils::ProductItemStatus)VarLong(pNewRow->GetValue(malcProductItemStatus),-1) == InvUtils::pisPurchased) {
					pNewRow->PutValue(malcReturn, GetRemotePropertyInt("ReturnReleasedGeneralSerializedItems", 1, 0, "<None>", true) == 1);
				}
				else {
					pNewRow->PutValue(malcReturn, g_cvarNull);
				}
				pNewRow->PutValue(malcProductItemID, g_cvarNull);
				pNewRow->PutValue(malcProductItemStatus, g_cvarNull);

				//TES 7/21/2008 - PLID 29478 - Fill the IsSerialized, LocationID, and OldStatus columns.
				pNewRow->PutValue(malcIsSerialized, pRow->GetValue(malcIsSerialized));
				pNewRow->PutValue(malcLocationID, pRow->GetValue(malcLocationID));
				pNewRow->PutValue(malcOldStatus, pRow->GetValue(malcOldStatus));

				//add to the same parent
				m_AllocationsList->AddRowSorted(pNewRow, pRow->GetParentRow());
			}
		}

	}NxCatchAll("CInvBatchCompleteAllocationsDlg::OnEditingStartingMultiAllocationList");
}

void CInvBatchCompleteAllocationsDlg::OnEditingFinishedMultiAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//if they checked the Used or Released box, uncheck the other

		if((nCol == malcUsed || nCol == malcReleased) && varNewValue.vt == VT_BOOL) {

			if(VarBool(varNewValue)) {
				if(nCol == malcUsed) {
					//they checked the Used column, so force
					//the Released column to be unchecked
					pRow->PutValue(malcReleased, g_cvarFalse);

					//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
					pRow->PutValue(malcReturn, g_cvarNull);
				}
				else if(nCol == malcReleased) {
					//they checked the Released column, so force
					//the Used column to be unchecked
					pRow->PutValue(malcUsed, g_cvarFalse);

					//TES 6/20/2008 - PLID 26152 - If we're releasing a serialized, Purchased Inventory item, they can flag it
					// as To Be Returned, so load that preference.  Otherwise, leave the column blank.
					if(VarLong(pRow->GetValue(malcProductItemID),-1) != -1 &&
						(InvUtils::ProductItemStatus)VarLong(pRow->GetValue(malcProductItemStatus),-1) == InvUtils::pisPurchased) {
						pRow->PutValue(malcReturn, GetRemotePropertyInt("ReturnReleasedGeneralSerializedItems", 1, 0, "<None>", true) == 1);
					}
					else {
						pRow->PutValue(malcReturn, g_cvarNull);
					}
				}
			}
			else {
				if(nCol == malcReleased) {
					//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
					pRow->PutValue(malcReturn, g_cvarNull);
				}
			}

			//now update this allocation's completion status in the parent row
			UpdateAllocationCompletionStatus_ByDetailRow(pRow);
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::OnEditingFinishedMultiAllocationList");
}

//called after a "used" or "released" status is changed, this will
//update the completion status on the parent row, and color that
//cell appropriately
void CInvBatchCompleteAllocationsDlg::UpdateAllocationCompletionStatus_ByParentRow(NXDATALIST2Lib::IRowSettingsPtr pParentRow)
{
	try {

		if(pParentRow == NULL) {
			ThrowNxException("No parent row given!");
		}

		BOOL bIsAllocationCompleted = TRUE;

		//track how many details exist, and how many are filled
		long nCountDetails = 0;
		long nCountCompletedDetails = 0;

		//this function will calculate the completion status of the details
		// (j.jones 2008-03-12 12:09) - PLID 29102 - we now track whether the allocation
		// was fully released, but in this function we don't care what the result is
		BOOL bIsFullyReleased = FALSE;
		CheckAllocationCompletion(pParentRow, nCountDetails, nCountCompletedDetails, bIsFullyReleased);

		//these colors mimic the default EMR colors, for continuity
		COLORREF cCompleted = RGB(192, 255, 192);
		COLORREF cPartial = RGB(255, 253, 170);		
		COLORREF cUnchanged = RGB(255,255,255);

		if(nCountDetails > 0 && nCountDetails == nCountCompletedDetails) {
			//it is completed, so color green
			pParentRow->PutValue(malcCompleted, "Yes");

			pParentRow->PutBackColor(cCompleted);
			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while(pChildRow) {
				pChildRow->PutBackColor(cCompleted);
				pChildRow = pChildRow->GetNextRow();
			}
		}
		else if(nCountCompletedDetails > 0 && nCountDetails > nCountCompletedDetails) {
			//partially completed, color yellow
			pParentRow->PutValue(malcCompleted, "Partial");

			pParentRow->PutBackColor(cPartial);
			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while(pChildRow) {
				pChildRow->PutBackColor(cPartial);
				pChildRow = pChildRow->GetNextRow();
			}
		}
		else {
			//not completed, color white
			pParentRow->PutValue(malcCompleted, "No");

			pParentRow->PutBackColor(cUnchanged);
			IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
			while(pChildRow) {
				pChildRow->PutBackColor(cUnchanged);
				pChildRow = pChildRow->GetNextRow();
			}
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::UpdateAllocationCompletionStatus_ByParentRow");
}

//called after a "used" or "released" status is changed, this will
//update the completion status on the parent row, and color that
//cell appropriately
void CInvBatchCompleteAllocationsDlg::UpdateAllocationCompletionStatus_ByDetailRow(NXDATALIST2Lib::IRowSettingsPtr pDetailRow)
{
	try {

		IRowSettingsPtr pParentRow = pDetailRow->GetParentRow();
		if(pParentRow == NULL) {
			//should be impossible for us to be editing the parent row already,
			//but apparently we are
			ASSERT(FALSE);
			pParentRow = pDetailRow;

			if(pParentRow == NULL) {
				ThrowNxException("Could not find parent row!");
			}
		}

		UpdateAllocationCompletionStatus_ByParentRow(pParentRow);

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::UpdateAllocationCompletionStatus_ByDetailRow");
}

//takes in the datalist pointer to the parent allocation,
//then fills nCountDetails with the total number of child details
//and fills nCountCompletedDetails with the number of child details
//that have Used or Released checked off
// (j.jones 2008-03-12 12:07) - PLID 29102 - need to know if the allocation is fully released
void CInvBatchCompleteAllocationsDlg::CheckAllocationCompletion(NXDATALIST2Lib::IRowSettingsPtr pParentRow, long &nCountDetails, long &nCountCompletedDetails, BOOL &bIsFullyReleased)
{
	try {

		if(pParentRow == NULL) {
			//shouldn't be allowed
			ASSERT(FALSE);
			ThrowNxException("Attempted to calculate completion of an empty parent row.");
		}

		//for this allocation, loop through each child detail
		IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
		
		bIsFullyReleased = TRUE;

		while(pChildRow) {

			//increment our detail count
			nCountDetails++;

			//get the "Used" and "Released" statuses
			BOOL bUsed = VarBool(pChildRow->GetValue(malcUsed), FALSE);
			BOOL bReleased = VarBool(pChildRow->GetValue(malcReleased), FALSE);

			if(bUsed || bReleased) {
				//something was filled, so increment the filled count
				nCountCompletedDetails++;
			}

			if(bUsed || !bReleased) {
				bIsFullyReleased = FALSE;
			}

			//move to the next detail
			pChildRow = pChildRow->GetNextRow();
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::CheckAllocationCompletion");
}

//Runs through all allocations in the list, if any have been modified,
//ensure they are completed - ignore any that aren't modified.
//Return FALSE if some are modified and uncompleted, and the user
//declined to auto-update the remaining items. Will warn if none are
//modified.
BOOL CInvBatchCompleteAllocationsDlg::VerifyAllocationsResolved()
{
	try {

		//track the count of changed allocations, the count fully completed,
		//and the count partially completed
		long nCountTotalAllocations = 0;
		long nCountChangedAllocations = 0;
		long nCountCompletedAllocations = 0;

		//tracks the first partially completed row, if any
		IRowSettingsPtr pPartialAllocationRow = NULL;

		//loop through each parent allocation

		CDWordArray dwaryPartialAllocationIDs;

		{
			IRowSettingsPtr pParentRow = m_AllocationsList->GetFirstRow();
			while(pParentRow) {

				//increment our allocation count
				nCountTotalAllocations++;

				//track how many details exist, and how many are filled
				long nCountDetails = 0;
				long nCountCompletedDetails = 0;

				//this function will calculate the completion status of the details
				// (j.jones 2008-03-12 12:09) - PLID 29102 - we now track whether the allocation
				// was fully released, because that is sometimes not allowed
				BOOL bIsFullyReleased = FALSE;
				CheckAllocationCompletion(pParentRow, nCountDetails, nCountCompletedDetails, bIsFullyReleased);

				if(VarLong(pParentRow->GetValue(malcCaseHistoryID), -1) != -1
					&& bIsFullyReleased) {

					// (j.jones 2008-03-12 12:17) - PLID 29102 - disallow a fully released
					// allocation if linked to a case history
					CString strPatientName = VarString(pParentRow->GetValue(malcPatientName), "");
					CString str;
					str.Format("An allocation for patient '%s' is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n"
						"However, this allocation is linked to a case history. A completely released allocation cannot be linked to a case history.\n\n"
						"You must either edit the allocation or case history to unlink the two, or leave a product as not released.", strPatientName);
					AfxMessageBox(str);
					return FALSE;
				}

				//was this allocation completed?
				if(nCountCompletedDetails == nCountDetails) {
					//it was, so increment our completed count
					nCountCompletedAllocations++;

					//and increment our changed count
					nCountChangedAllocations++;
				}
				//was it partially completed?
				else if(nCountCompletedDetails > 0) {
					//it was, so only increment our changed count
					nCountChangedAllocations++;

					long nID = VarLong(pParentRow->GetValue(malcAllocationID));
					dwaryPartialAllocationIDs.Add(nID);

					//and track this allocation, if we don't already have one
					if(pPartialAllocationRow == NULL) {
						pPartialAllocationRow = pParentRow;
					}

					//expand all partially completed allocations
					pParentRow->PutExpanded(VARIANT_TRUE);
				}
				
				//if no details are filled, do nothing

				//move to the next allocation
				pParentRow = pParentRow->GetNextRow();
			}
		}

		//ok, now let's see where we stand

		if(nCountTotalAllocations == 0) {
			//we never had any allocations to begin with,
			//so return silently
			return TRUE;
		}
		else if(nCountChangedAllocations == 0) {
			//nothing was changed, so warn the user, and ask if they wish to continue

			return IDYES == MessageBox("You have not begun to complete any of these allocations. None of the allocations will be saved as 'Completed'.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO);
		}
		else if(nCountChangedAllocations == nCountCompletedAllocations) {
			//every allocation we changed is completed, so return silently
			return TRUE;
		}
		else if(nCountCompletedAllocations < nCountChangedAllocations) {
			//here's our true problem case - they changed some allocations,
			//but didn't complete them, so warn accordingly and ask the user
			//what they want to do

			//highlight the first partially allocated row, and ensure it is expanded
			if(pPartialAllocationRow) {
				m_AllocationsList->PutCurSel(pPartialAllocationRow);
				m_AllocationsList->EnsureRowInView(pPartialAllocationRow);
				pPartialAllocationRow->PutExpanded(VARIANT_TRUE);
			}

			long nIncompletedAllocations = nCountChangedAllocations - nCountCompletedAllocations;
			CString strWarn;
			//properly word the warning based on the amount of allocations in question
			if(nCountChangedAllocations == 1) {
				strWarn = "The allocation you modified has not been fully completed.\n"
					"On the following screen, you will need to complete this allocation.";
			}
			else if(nIncompletedAllocations == 1) {
				strWarn.Format("You modified %li allocations, but one of them has not been completed.\n"
					"On the following screen, you will need to complete this allocation.", nCountChangedAllocations);
			}
			else {
				strWarn.Format("You modified %li allocations, but %li of them have not been completed.\n"
					"On the following screens, you will need to complete these allocations.", nCountChangedAllocations, nIncompletedAllocations);
			}

			AfxMessageBox(strWarn);

			//now loop through and edit each allocation
			for(int i=0;i<dwaryPartialAllocationIDs.GetSize();i++) {
				long nID = (long)dwaryPartialAllocationIDs.GetAt(i);

				//we will use this parent row a few times in this loop
				IRowSettingsPtr pAllocationRow = m_AllocationsList->FindByColumn(malcID, (long)nID, m_AllocationsList->GetFirstRow(), FALSE);
				if(pAllocationRow == NULL || pAllocationRow->GetParentRow() != NULL) {
					//should be impossible
					ThrowNxException("Could not find master allocation row in datalist.");
				}

				InvUtils::AllocationMasterInfo *pInfo = NULL;
				//load this object from data
				// (j.jones 2008-02-20 09:23) - PLID 28948 - added a flag for whether to include released items (set to FALSE)
				InvUtils::PopulateAllocationInfo(nID, pInfo, FALSE);

				if(pInfo == NULL) {
					//this should be impossible
					ThrowNxException("Failed to load allocation information.");
				}

				//the pInfo we just loaded came from data, but we changed
				//the status on some of these details, and potentially the
				//notes and even the quantity, so update accordingly
				int j = 0;
				for(j=0;j<pInfo->paryAllocationDetailInfo.GetSize();j++) {
					InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)pInfo->paryAllocationDetailInfo.GetAt(j);
					if(pDetail == NULL) {
						ThrowNxException("Encountered invalid allocation detail. (1)");
					}

					//find the detail row
					IRowSettingsPtr pRow = m_AllocationsList->FindByColumn(malcDetailID, (long)pDetail->nDetailID, m_AllocationsList->GetFirstRow(), FALSE);
					if(pRow == NULL || pRow->GetParentRow() == NULL) {
						//should be impossible
						ThrowNxException("Could not find allocation detail in datalist. (1)");
					}

					//now update the status based on the settings in the detail row
					BOOL bUsed = VarBool(pRow->GetValue(malcUsed), FALSE);
					BOOL bReleased = VarBool(pRow->GetValue(malcReleased), FALSE);

					if(bUsed) {
						pDetail->iadsCurrentStatus = InvUtils::iadsUsed;
					}
					else if(bReleased) {
						pDetail->iadsCurrentStatus = InvUtils::iadsReleased;
					}
					else {
						//TES 7/18/2008 - PLID 29478 - Pull the original status.  If it was "To Be Ordered", but it has a 
						// ProductItemID now, then it is now Active.
						pDetail->iadsCurrentStatus = (InvUtils::InventoryAllocationDetailStatus)VarLong(pRow->GetValue(malcOldStatus),FALSE);
						if(pDetail->iadsCurrentStatus == InvUtils::iadsOrder) {
							if(VarLong(pRow->GetValue(malcProductItemID),-1) != -1) {
								pDetail->iadsCurrentStatus = InvUtils::iadsActive;
							}
						}
					}

					if(bReleased) {
						//TES 6/20/2008 - PLID 26152 - If it's released, check whether it's flagged to be returned.
						pDetail->bToBeReturned = VarBool(pRow->GetValue(malcReturn),FALSE);
					}
					else {
						pDetail->bToBeReturned = FALSE;
					}

					//update the notes
					pDetail->strNotes = VarString(pRow->GetValue(malcNotes), "");

					//and update the quantity, in the event that may have changed
					pDetail->dblCurQuantity = VarDouble(pRow->GetValue(malcCurQuantity), 1.0);
				}

				//now we have the pInfo object loaded and reflecting any changes we've made,
				//except for any new items that were split out

				//find all new items, create a detail object, and map the rows
				//so we can update changes later			

				//now, find new child rows
				IRowSettingsPtr pChildRow = pAllocationRow->GetFirstChildRow();
				while(pChildRow) {

					long nDetailID = VarLong(pChildRow->GetValue(malcDetailID), -1);
					if(nDetailID > 0) {
						//skip rows with IDs
						pChildRow = pChildRow->GetNextRow();
						continue;
					}

					//any negative number means we're on a new row, so create a new object
					InvUtils::AllocationDetailInfo *pNewDetail = new InvUtils::AllocationDetailInfo;
					pNewDetail->nDetailID = nDetailID;
					pNewDetail->nProductID = VarLong(pChildRow->GetValue(malcProductID), -1);
					pNewDetail->strProductName = VarString(pChildRow->GetValue(malcProductName), "");
					//a product item should be impossible on a new item in this screen
					pNewDetail->nProductItemID = -1;
					pNewDetail->varSerialNum = g_cvarNull;
					pNewDetail->varExpDate = g_cvarNull;
					pNewDetail->varProductItemStatus = g_cvarNull; // (c.haag 2008-03-11 14:06) - PLID 29255 - Product item status
					pNewDetail->strOriginalNotes = VarString(pChildRow->GetValue(malcOldNotes), "");
					pNewDetail->strNotes = VarString(pChildRow->GetValue(malcNotes), "");
					pNewDetail->dblOriginalQty = VarDouble(pChildRow->GetValue(malcOldQuantity), -1);
					pNewDetail->dblCurQuantity = VarDouble(pChildRow->GetValue(malcCurQuantity), -1);
					pNewDetail->bBilled = FALSE;
					//TES 6/20/2008 - PLID 26152 - Load the ToBeReturned flag
					pNewDetail->bOldToBeReturned = VarBool(pChildRow->GetValue(malcReturn), FALSE);
					pNewDetail->bToBeReturned = pNewDetail->bOldToBeReturned;

					// (j.jones 2008-01-07 17:06) - PLID 28479 - added bIsProductBillable, but
					// it's not used in this screen so we don't need to look up the accurate value
					pNewDetail->bIsProductBillable = TRUE;

					//TES 7/18/2008 - PLID 29478 - According to the comment on nProductItemID above, this should never be TRUE.
					pNewDetail->bIsSerialized = FALSE;

					BOOL bUsed = VarBool(pChildRow->GetValue(malcUsed), FALSE);
					BOOL bReleased = VarBool(pChildRow->GetValue(malcReleased), FALSE);

					InvUtils::InventoryAllocationDetailStatus iadsStatus = InvUtils::iadsActive;
					if(bUsed) {
						iadsStatus = InvUtils::iadsUsed;
					}
					else if(bReleased) {
						iadsStatus = InvUtils::iadsReleased;
					}

					//new item, so make both statii be the same
					pNewDetail->iadsOriginalStatus = iadsStatus;
					pNewDetail->iadsCurrentStatus = iadsStatus; //use the same status
					
					//add it to our memory object
					pInfo->paryAllocationDetailInfo.Add(pNewDetail);					

					//move to the next detail
					pChildRow = pChildRow->GetNextRow();
				}

				CInvAllocationUsageDlg dlg(this);
				//pass in the memory object we created
				dlg.SetAllocationInfo(pInfo);
				//force the allocation to be completed
				dlg.m_bForceCompletion = TRUE;
				//we are going to be responsible for saving the changes,
				//not the allocation usage dialog
				dlg.m_bSaveToData = FALSE;
				//ensure the dialog clears the memory object, and not us
				dlg.m_bFreeInfoObject = TRUE;

				if(dlg.DoModal() == IDOK) {

					//pInfo = dlg.GetAllocationInfo();

					//our pInfo has been updated, so now update the datalist					
					for(j=0;j<pInfo->paryAllocationDetailInfo.GetSize();j++) {
						InvUtils::AllocationDetailInfo *pDetail = (InvUtils::AllocationDetailInfo*)pInfo->paryAllocationDetailInfo.GetAt(j);
						if(pDetail == NULL) {
							ThrowNxException("Encountered invalid allocation detail. (2)");
						}

						long nDetailID = pDetail->nDetailID;

						if(nDetailID != -1) {

							//the detail is either preexisting (> 0) or a new detail that
							//this dialog previously created (< -1)

							//find the detail row
							IRowSettingsPtr pRow = m_AllocationsList->FindByColumn(malcDetailID, (long)pDetail->nDetailID, m_AllocationsList->GetFirstRow(), FALSE);
							if(pRow == NULL || pRow->GetParentRow() == NULL) {
								//should be impossible
								ThrowNxException("Could not find allocation detail in datalist. (2)");
							}
							
							//now update the row based on the new status
							if(pDetail->iadsCurrentStatus == InvUtils::iadsUsed) {
								pRow->PutValue(malcUsed, g_cvarTrue);
								pRow->PutValue(malcReleased, g_cvarFalse);
								//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
								pRow->PutValue(malcReturn, g_cvarNull);
							}
							else if(pDetail->iadsCurrentStatus == InvUtils::iadsReleased) {
								pRow->PutValue(malcUsed, g_cvarFalse);
								pRow->PutValue(malcReleased, g_cvarTrue);
								//TES 6/20/2008 - PLID 26152 - Pull the ToBeReturned flag
								if(pDetail->nProductItemID != -1 && (InvUtils::ProductItemStatus)VarLong(pDetail->varProductItemStatus,-1) == InvUtils::pisPurchased) {
									pRow->PutValue(malcReturn, pDetail->bToBeReturned == TRUE);
								}
								else {
									pRow->PutValue(malcReturn, g_cvarNull);
								}
							}
							else {
								pRow->PutValue(malcUsed, g_cvarFalse);
								pRow->PutValue(malcReleased, g_cvarFalse);
								//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
								pRow->PutValue(malcReturn, g_cvarNull);
							}

							//also the notes may have changed
							pRow->PutValue(malcNotes, _bstr_t(pDetail->strNotes));

							//and potentially the quantity
							pRow->PutValue(malcCurQuantity, pDetail->dblCurQuantity);

							//also recreate the product name, with the new quantity
							CString strProductName = pDetail->strProductName;
							CString strNewName = strProductName;
							if(pDetail->dblCurQuantity != 1.0) {
								strNewName.Format("%s (%g)", strProductName, pDetail->dblCurQuantity);
							}
							pRow->PutValue(malcPatientName, _bstr_t(strNewName));
						}
						else {
							//If the detail ID is -1, the only way that can happen is if
							//the usage dialog split apart a multiple-qty item and created
							//a new detail. Had this dialog done so, the ID would have been
							//-2 or less.

							//create a new detail row

							//now we need to create the new datalist row, it will copy most of the data
							//from the existing row
							IRowSettingsPtr pNewRow = m_AllocationsList->GetNewRow();		

							//generate a new name, with the new quantity
							CString strProductName = pDetail->strProductName;
							CString strNewName = strProductName;
							if(pDetail->dblCurQuantity != 1.0) {
								strNewName.Format("%s (%g)", strProductName, pDetail->dblCurQuantity);
							}

							pNewRow->PutValue(malcID, g_cvarNull);
							pNewRow->PutValue(malcParentID, pInfo->nAllocationID);
							pNewRow->PutValue(malcAllocationID, pInfo->nAllocationID);
							//instead of setting to -1, we track unique negative IDs, starting with -2					
							pNewRow->PutValue(malcDetailID, m_nNextNewDetailID);
							m_nNextNewDetailID--;
							pNewRow->PutValue(malcPersonID, pInfo->nPatientID);
							pNewRow->PutValue(malcUserDefinedID, g_cvarNull);
							pNewRow->PutValue(malcPatientName, _bstr_t(strNewName));				
							pNewRow->PutValue(malcProductID, pDetail->nProductID);
							pNewRow->PutValue(malcProductName, _bstr_t(strProductName));
							pNewRow->PutValue(malcCurQuantity, pDetail->dblCurQuantity);
							pNewRow->PutValue(malcOldQuantity, pDetail->dblOriginalQty);
							//shouldn't be possible for the serialized stuff to be non-null
							pNewRow->PutValue(malcSerialNumber, g_cvarNull);
							pNewRow->PutValue(malcExpDate, g_cvarNull);
							pNewRow->PutValue(malcUsed, pDetail->iadsCurrentStatus == InvUtils::iadsUsed);
							pNewRow->PutValue(malcReleased, pDetail->iadsCurrentStatus == InvUtils::iadsReleased);
							//TES 6/20/2008 - PLID 26152 - Load the ToBeReturned flag
							if(pDetail->iadsCurrentStatus == InvUtils::iadsReleased && pDetail->nProductItemID != -1 && (InvUtils::ProductItemStatus)VarLong(pDetail->varProductItemStatus,-1) == InvUtils::pisPurchased) {
								pNewRow->PutValue(malcReturn, pDetail->bToBeReturned == TRUE);
							}
							else {
								pNewRow->PutValue(malcReturn, g_cvarNull);
							}
							pNewRow->PutValue(malcCompleted, g_cvarNull);
							pNewRow->PutValue(malcNotes, _bstr_t(pDetail->strNotes));
							pNewRow->PutValue(malcOldNotes, _bstr_t(pDetail->strOriginalNotes));
							pNewRow->PutValue(malcApptDate, g_cvarNull);
							// (j.jones 2008-03-12 12:12) - PLID 29102 - added CaseHistoryID, null on details
							pNewRow->PutValue(malcCaseHistoryID, g_cvarNull);
							// (j.jones 2008-03-24 17:22) - PLID 29388 - added AppointmentID
							pNewRow->PutValue(malcApptID, g_cvarNull);

							//TES 7/21/2008 - PLID 26152 - Fill the ProductItemID columns
							pNewRow->PutValue(malcProductItemID, g_cvarNull);
							pNewRow->PutValue(malcProductItemStatus, pDetail->varProductItemStatus);

							//TES 7/21/2008 - PLID 29478 - Fill in the IsSerialized, LocationID, and OldStatus columns.
							pNewRow->PutValue(malcIsSerialized, pDetail->bIsProductBillable ? g_cvarTrue : g_cvarFalse);
							pNewRow->PutValue(malcLocationID, g_cvarNull);
							pNewRow->PutValue(malcOldStatus, g_cvarNull);

							//add to the same parent
							m_AllocationsList->AddRowSorted(pNewRow, pAllocationRow);
						}
					}

					//find the parent row
					IRowSettingsPtr pParentRow = m_AllocationsList->FindByColumn(malcAllocationID, (long)nID, m_AllocationsList->GetFirstRow(), FALSE);
					if(pParentRow == NULL || pParentRow->GetParentRow() != NULL) {
						//should be impossible
						ThrowNxException("Could not find allocation row in datalist.");
					}

					//update our status and coloring
					UpdateAllocationCompletionStatus_ByParentRow(pParentRow);
				}
				else {
					//they cancelled, so warn the user they have to manually fix it
					AfxMessageBox("You must complete all remaining partially-completed allocations before saving.");
					return FALSE;
				}
			}

			//if we get here, we're good to go
			return TRUE;

			/* not currently used, replaced by the above functionality
			strWarn += "You must check the 'Used' or 'Released' box on every product in all allocations you wish to complete,\n"
				"or leave all the products unchecked on that allocation.\n\n"
				"Would you like to mark the remaining products on partially completed allocations as 'Used'?\n"
				"(Allocations that you did not modify will not be affected.)\n\n"
				"Clicking YES will update each unchecked product on partially completed allocations as 'Used'.\n"
				"Clicking NO will update each unchecked product on partially completed allocations as 'Released'.\n"
				"Clicking CANCEL will cancel closing this screen and allow you to complete these allocations manually.";

			long nRet = MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);
			if(nRet == IDCANCEL) {
				//they don't want to auto-update, so return FALSE
				//to indicate we can't save these allocations
				return FALSE;
			}
			else {

				//auto-update per their wishes, then return TRUE
				//to indicate that the allocations are now completed
				AutoCompletePartialAllocations(nRet == IDYES);
				return TRUE;
			}
			*/
		}	

		//otherwise, no special handling is needed
		return TRUE;

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::VerifyAllocationsResolved");	

	return FALSE;
}

LRESULT CInvBatchCompleteAllocationsDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		//TES 7/18/2008 - PLID 29478 - We may not be processing barcode scans right now.
		if(m_bDisableBarcode) {
			return 0;
		}

		//barcode scanning in this screen is limited to serial numbers only

		//if the product is in the list, expand that allocation,
		//and mark the status of that product based on our setting
		//for "used" or "released" upon barcoding

		_bstr_t bstr = (BSTR)lParam;
		_variant_t varBarcode(bstr);

		//check whether they want to have the barcode scan auto-check Used, or Released
		BOOL bBarcodeUsedItems = TRUE;

		if(m_radioBarcodeReleased.GetCheck()) {
			bBarcodeUsedItems = FALSE;
		}

		//find the row that this item is in
		IRowSettingsPtr pRow = m_AllocationsList->FindByColumn(malcSerialNumber, varBarcode, m_AllocationsList->GetFirstRow(), FALSE);
		if(pRow == NULL) {
			//If a serial number is not found, first see if it is in use somewhere,
			//but don't include active allocations, and don't filter on location
			long nProductItemID = -1;
			long nProductID = -1;
			CString strProductName = "";
			_variant_t varExpDate = g_cvarNull;
			if(InvUtils::Barcode_CheckExistenceOfSerialNumber(VarString(varBarcode), -1, FALSE, nProductItemID, nProductID, strProductName, varExpDate, NULL, TRUE)) {
				//the number is available, and we didn't give a message,
				//so now we need to see if it is in a future allocation
				_RecordsetPtr rs = CreateParamRecordset("SELECT "
					"Last + ', ' + First + ' ' + Middle AS PatientName, "
					"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime, "
					"Convert(bit, CASE WHEN AppointmentsT.ID Is Not Null AND CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) > GetDate() THEN 1 ELSE 0 END) AS IsFutureAppt "
					"FROM PatientInvAllocationsT "
					"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
					"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
					"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "					
					"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
					"WHERE PatientInvAllocationsT.Status = {INT} AND PatientInvAllocationDetailsT.Status = {INT} "
					"AND PatientInvAllocationDetailsT.ProductItemID = {INT}",
					InvUtils::iasActive, InvUtils::iadsActive, nProductItemID);
				if(!rs->eof) {
					//this serial number is on an allocation, but it's not in our list,
					//so it's either in the future, or was just created

					CString strPatientName = AdoFldString(rs, "PatientName","");
					_variant_t varApptDate = rs->Fields->Item["ApptDateTime"]->Value;
					BOOL bIsFutureAppt = AdoFldBool(rs, "IsFutureAppt", FALSE);

					CString strWarn;

					if(bIsFutureAppt && varApptDate.vt == VT_DATE) {
						//it's an allocation in the future
						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s',\n"
							"but that serial numbered product is on a patient allocation for an appointment in the future.\n"
							"(Patient Name: %s, Appointment Date: %s)\n\n"
							"If you wish to complete that allocation, you must manually complete it from the Allocations tab.",
							strProductName, strPatientName, FormatDateTimeForInterface(VarDateTime(varApptDate), NULL, dtoDateTime));
					}
					else {
						//This should be impossible unless the allocation was created or modified after this
						//screen was originally opened. If so, tell the user they must manually complete it.
						strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s', allocated to patient '%s'.\n"
							"That allocation is not in the current list (it may have just been created or modified).\n\n"
							"If you wish to complete that allocation, you must manually complete it from the Allocations tab, or reopen this screen to reload the list.", strProductName, strPatientName);
					}
					AfxMessageBox(strWarn);
				}
				else {
					//otherwise, it's just not in an active allocation, and we can't do anything about it,
					//so tell them that (technically it could be in an allocation created after this
					//window was opened, but we're not going to support that situation)
					CString strWarn;
					strWarn.Format("The barcode you scanned matches up with a serial number for the product '%s',\n"
						"but that serial numbered product is not in any existing active patient allocation.", strProductName);
					AfxMessageBox(strWarn);
				}
			}
			else {
				//do nothing, we don't need to tell them it's not a valid serial number
			}

			//leave now, we can't add anything
			return 0;
		}

		//TES 7/3/2008 - PLID 24726 - They may have multiple items with the same serial number (aka "Lot Number").  So, loop
		// through until we find one that needs changed.  This way, they can scan the same barcode several times, and it will
		// do the appropriate action for each one, until it runs out.
		bool bActionTaken = FALSE;
		while(!bActionTaken && pRow != NULL) {
			//we found it, so check the appropriate value
			if(bBarcodeUsedItems) {
				//TES 7/3/2008 - PLID 24726 - Is this used already?
				if(!VarBool(pRow->GetValue(malcUsed))) {
					bActionTaken = TRUE;
					pRow->PutValue(malcUsed, g_cvarTrue);
					pRow->PutValue(malcReleased, g_cvarFalse);
					//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
					pRow->PutValue(malcReturn, g_cvarNull);
				}
			}
			else {
				//TES 7/3/2008 - PLID 24726 - Is this released already?
				if(!VarBool(pRow->GetValue(malcReleased))) {
					bActionTaken = TRUE;
					pRow->PutValue(malcUsed, g_cvarFalse);
					pRow->PutValue(malcReleased, g_cvarTrue);
					//TES 6/20/2008 - PLID 26152 - If we're releasing a serialized, purchased inventory item, then it can be flagged
					// as to be returned, so check the preference, otherwise make the column blank.
					if(VarLong(pRow->GetValue(malcProductItemID),-1) != -1 && 
						(InvUtils::ProductItemStatus)VarLong(pRow->GetValue(malcProductItemStatus),-1) == InvUtils::pisPurchased) {
						pRow->PutValue(malcReturn, VarBool(pRow->GetValue(malcReturn),FALSE) == TRUE);
					}
					else {
						pRow->PutValue(malcReturn, g_cvarNull);
					}
				}
			}
			if(!bActionTaken) 
			{
				//TES 7/3/2008 - PLID 24726 - Advance to the next row.
				pRow = pRow->GetNextRow();
				if(pRow) {
					pRow = m_AllocationsList->FindByColumn(malcSerialNumber, varBarcode, pRow, FALSE);
				}
			}
		}

		if(pRow) {
			m_AllocationsList->EnsureRowInView(pRow);

			//now update this allocation's completion status in the parent row
			UpdateAllocationCompletionStatus_ByDetailRow(pRow);
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::OnBarcodeScan");

	return 0;
}

/* not currently used
//Run through every allocation that is partially complete, and update
//incompleted details such that if the parameter is TRUE, mark them used,
//otherwise mark them released
void CInvBatchCompleteAllocationsDlg::AutoCompletePartialAllocations(BOOL bMarkAsUsed)
{
	try {

		//loop through all allocations, double-check their completeness
		IRowSettingsPtr pParentRow = m_AllocationsList->GetFirstRow();
		while(pParentRow) {

			//track how many details exist, and how many are filled
			long nCountDetails = 0;
			long nCountCompletedDetails = 0;

			//this function will calculate the completion status of the details
			CheckAllocationCompletion(pParentRow, nCountDetails, nCountCompletedDetails);

			//is this allocation partially completed?
			if(nCountCompletedDetails > 0 && nCountCompletedDetails < nCountDetails) {
				//yes, it's partially complete, so let's update each incompleted row

				IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
				while(pChildRow) {
					
					//check the status
					BOOL bUsed = VarBool(pChildRow->GetValue(malcUsed), FALSE);
					BOOL bReleased = VarBool(pChildRow->GetValue(malcReleased), FALSE);

					if(!bUsed && !bReleased) {
						//it's incomplete, so complete it accordingly

						if(bMarkAsUsed) {
							pChildRow->PutValue(malcUsed, g_cvarTrue);
							pChildRow->PutValue(malcReleased, g_cvarFalse);
						}
						else {
							pChildRow->PutValue(malcUsed, g_cvarFalse);
							pChildRow->PutValue(malcReleased, g_cvarTrue);
						}
					}

					//move to the next detail
					pChildRow = pChildRow->GetNextRow();
				}

				//now update this allocation's completion status in the parent row
				UpdateAllocationCompletionStatus_ByParentRow(pParentRow);
			}

			//move to the next allocation
			pParentRow = pParentRow->GetNextRow();
		}

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::AutoCompletePartialAllocations");
}
*/

//PromptForQtyChange will take in a boolean for whether we're using or releasing a given product,
//takes in the product name and current quantity, and then passes back the quantity they are using,
//which must be greater than zero and not greater than the current quantity.
//Return FALSE if the user cancelled, return TRUE if we are returning a valid new quantity.
BOOL CInvBatchCompleteAllocationsDlg::PromptForQtyChange(BOOL bUsed, CString strProductName, double dblCurQuantity, double &dblNewQuantity)
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

		return TRUE;

	}NxCatchAll("Error in CInvBatchCompleteAllocationsDlg::PromptForQtyChange");

	return FALSE;
}
