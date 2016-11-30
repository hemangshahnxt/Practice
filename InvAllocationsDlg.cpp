// InvAllocationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvAllocationsDlg.h"
#include "InvPatientAllocationDlg.h"
#include "InvUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "InvAllocationUsageDlg.h"
#include "InvBatchCompleteAllocationsDlg.h"
#include "GlobalSchedUtils.h"
#include "ApptsRequiringAllocationsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-11-06 11:12) - PLID 28003 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CInvAllocationsDlg dialog

enum AllocationListColumn {

	alcID = 0,
	alcParentID,
	alcAllocationID,
	alcStatus,
	alcPersonID,
	alcUserDefinedID,
	alcPatientName,
	alcLocation,
	alcApptDate,
	alcApptStatus,		// (j.jones 2008-09-08 16:13) - PLID 30259 - added appt. status
	alcCreatedDate,	
	alcCompletedDate,	// (j.jones 2007-11-29 09:32) - PLID 28196 - added completion info
	alcCompletedBy,
};

CInvAllocationsDlg::CInvAllocationsDlg(CWnd* pParent)
	: CNxDialog(CInvAllocationsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvAllocationsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInvAllocationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvAllocationsDlg)
	DDX_Control(pDX, IDC_CONFIGURE_REQUIRED_ALLOCATIONS, m_nxbConfigureRequiredAllocations);
	DDX_Control(pDX, IDC_BTN_COMPLETE_MULTIPLE_ALLOCATIONS, m_btnCompleteMultipleAllocations);
	DDX_Control(pDX, IDC_RADIO_COMPLETED_NOT_BILLED, m_radioCompleteNotBilled);
	DDX_Control(pDX, IDC_RADIO_COMPLETED_AND_BILLED, m_radioCompleteBilled);
	DDX_Control(pDX, IDC_RADIO_ACTIVE_ALLOCATIONS, m_radioActive);
	DDX_Control(pDX, IDC_BTN_CREATE_NEW_ALLOCATION, m_btnCreateNew);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvAllocationsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvAllocationsDlg)
	ON_BN_CLICKED(IDC_BTN_CREATE_NEW_ALLOCATION, OnBtnCreateNewAllocation)
	ON_BN_CLICKED(IDC_RADIO_ACTIVE_ALLOCATIONS, OnRadioActiveAllocations)
	ON_BN_CLICKED(IDC_RADIO_COMPLETED_AND_BILLED, OnRadioCompletedAndBilled)
	ON_BN_CLICKED(IDC_RADIO_COMPLETED_NOT_BILLED, OnRadioCompletedNotBilled)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_COMPLETE_MULTIPLE_ALLOCATIONS, OnBtnCompleteMultipleAllocations)
	ON_BN_CLICKED(IDC_CONFIGURE_REQUIRED_ALLOCATIONS, OnConfigureRequiredAllocations)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvAllocationsDlg message handlers

BOOL CInvAllocationsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-24 16:32) - PLID 29778 - NxIconify buttons
		m_btnCreateNew.AutoSet(NXB_NEW);
		m_btnCompleteMultipleAllocations.AutoSet(NXB_MODIFY);

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));
		
		// (j.jones 2007-11-12 15:23) - PLID 28074 - disable the "create" button if no permissions
		if(!(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS)) {
			GetDlgItem(IDC_BTN_CREATE_NEW_ALLOCATION)->EnableWindow(FALSE);
		}

		m_AllocationsList = BindNxDataList2Ctrl(IDC_ALLOCATIONS_LIST, false);

		// (j.jones 2007-11-08 14:52) - PLID 28003 - I intentionally set the from clause
		// here in code for easier readibility, future changes, etc. It also helps
		// with the status filtering.

		CString strQuery;
		//TES 7/18/2008 - PLID 29478 - Included "To Be Ordered" details
		// (j.jones 2008-09-08 16:13) - PLID 30259 - included appt. status
		strQuery.Format("(SELECT PatientInvAllocationsT.ID, NULL AS ParentID, "
			"PatientInvAllocationsT.ID AS AllocationID, PatientInvAllocationsT.Status, NULL AS DetailStatus, "
			"PatientsT.PersonID, PatientsT.UserDefinedID, Last + ', ' + First + ' ' + Middle AS PatientName, "
			"LocationsT.ID AS LocID, LocationsT.Name AS LocName, "
			"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime, "
			"CASE WHEN AppointmentsT.Status = 4 THEN CASE WHEN ShowState = 3 THEN 'Cancelled (No Show)' ELSE 'Cancelled' END ELSE AptShowStateT.Name END AS ApptStatus, "
			"PatientInvAllocationsT.InputDate, PatientInvAllocationsT.CompletionDate, UsersT.Username "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
			"LEFT JOIN UsersT ON PatientInvAllocationsT.CompletedBy = UsersT.PersonID "
			"WHERE PatientInvAllocationsT.Status <> %li "
			"UNION "
			"SELECT NULL AS ID, PatientInvAllocationDetailsT.AllocationID AS ParentID, "
			"PatientInvAllocationDetailsT.AllocationID, PatientInvAllocationsT.Status, "
			"PatientInvAllocationDetailsT.Status AS DetailStatus, "
			"PatientInvAllocationsT.PatientID AS PersonID, NULL AS UserDefinedID, "
			"ServiceT.Name + ' (' + Convert(nvarchar, Sum(Quantity)) + ')' AS ProductDesc, "
			"PatientInvAllocationsT.LocationID AS LocID, NULL AS LocName, NULL AS ApptDateTime, NULL AS ApptStatus, NULL AS InputDate, NULL AS CompletionDate, NULL AS Username "
			"FROM PatientInvAllocationDetailsT "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN ProductT ON PatientInvAllocationDetailsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE PatientInvAllocationsT.Status <> %li AND PatientInvAllocationDetailsT.Status IN (%li, %li, %li) "
			"GROUP BY ProductT.ID, ServiceT.Name, PatientInvAllocationDetailsT.AllocationID, PatientInvAllocationsT.PatientID, PatientInvAllocationsT.Status, PatientInvAllocationDetailsT.Status, PatientInvAllocationsT.LocationID) "
			"AS AllocationsQ", InvUtils::iasDeleted, InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed, InvUtils::iadsOrder);

		m_AllocationsList->PutFromClause(_bstr_t(strQuery));

		// (j.jones 2007-11-27 16:51) - PLID 28196 - default to all active allocations
		m_radioActive.SetCheck(TRUE);
		
		//the list will be requeried in Refresh()

	}NxCatchAll("Error in CInvAllocationsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvAllocationsDlg::OnBtnCreateNewAllocation() 
{
	try {

		// (j.jones 2007-11-12 15:24) - PLID 28074 - added permissions
		if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
			return;
		}

		CInvPatientAllocationDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//requery the list to reflect the new allocation
			m_AllocationsList->Requery();
		}

	}NxCatchAll("Error in CInvAllocationsDlg::OnBtnCreateNewAllocation");
}

void CInvAllocationsDlg::Refresh()
{
	try {

		// (j.jones 2007-11-27 16:51) - PLID 28196 - added filters

		//in all these where clauses, the "DetailStatus Is Null" filter
		//accounts for the parent rows
		
		CString strWhere;
		if(m_radioCompleteNotBilled.GetCheck()) {
			// (j.jones 2007-12-12 11:20) - PLID 27988 - filter on unbilled, completed allocations
			// for the purposes of this tab, a partially billed allocation is considered "unbilled"
			// also include completed allocations with no used items (all released)
			// (j.jones 2008-01-08 11:00) - PLID 28479 - this filter should ignore allocations
			// that have all billable items billed, but also have unbillable items
			// (j.jones 2008-05-19 09:22) - PLID 29492 - this filter should take into account
			// whether the allocation is linked to a billed case history
			// (j.jones 2009-08-06 09:50) - PLID 35120 - added BilledCaseHistoriesT
			strWhere.Format("Status = %li AND (DetailStatus Is Null OR DetailStatus = %li) "
				//is not linked to a billed case history
				"AND AllocationID NOT IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT "
				"			INNER JOIN CaseHistoryT ON CaseHistoryAllocationLinkT.CaseHistoryID = CaseHistoryT.ID "
				"			INNER JOIN BilledCaseHistoriesT ON CaseHistoryT.ID = BilledCaseHistoriesT.CaseHistoryID "
				"			INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
				"			WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND CaseHistoryT.CompletedDate Is Not Null) "
				//has unbilled products that are billable
				"AND (AllocationID IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE Status = %li "
				"			AND ID NOT IN (SELECT AllocationDetailID FROM ChargedAllocationDetailsT "
				"			WHERE ChargeID IN (SELECT ID FROM LineItemT WHERE Deleted = 0))"
				"			AND ProductID IN (SELECT ProductID FROM ProductLocationInfoT WHERE ProductLocationInfoT.Billable = 1 AND ProductLocationInfoT.LocationID = LocID)) "
				//has only released products
				"OR AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE Status = %li "
				"			AND ID NOT IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE Status = %li))"
				//has only non-billable products
				"OR AllocationID IN (SELECT ID FROM PatientInvAllocationsT WHERE Status = %li "
				"			AND ID NOT IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE ProductID IN "
				"				(SELECT ProductID FROM ProductLocationInfoT WHERE ProductLocationInfoT.Billable = 1 AND ProductLocationInfoT.LocationID = LocID)))) ",
				InvUtils::iasCompleted, InvUtils::iadsUsed,
				InvUtils::iadsUsed,
				InvUtils::iasCompleted, InvUtils::iadsUsed,
				InvUtils::iasCompleted);

			//show the completion columns
			m_AllocationsList->GetColumn(alcCompletedDate)->PutStoredWidth(90);
			m_AllocationsList->GetColumn(alcCompletedBy)->PutStoredWidth(90);
		}
		else if(m_radioCompleteBilled.GetCheck()) {
			// (j.jones 2007-12-12 11:20) - PLID 27988 - filter on billed, completed allocations
			// for the purposes of this tab, the allocation must have all of its used items billed
			// also filter out completed allocations with no used items (all released)
			// (j.jones 2008-01-08 11:00) - PLID 28479 - this filter should include allocations
			// that have all billable items billed (ignoring unbillable items)
			// (j.jones 2008-05-19 09:22) - PLID 29492 - this filter should take into account
			// whether the allocation is linked to a billed case history
			// (j.jones 2009-08-06 09:50) - PLID 35120 - added BilledCaseHistoriesT
			strWhere.Format("Status = %li AND (DetailStatus Is Null OR DetailStatus = %li) "
				//is linked to a billed case history
				"AND (AllocationID IN (SELECT AllocationID FROM CaseHistoryAllocationLinkT "
				"			INNER JOIN CaseHistoryT ON CaseHistoryAllocationLinkT.CaseHistoryID = CaseHistoryT.ID "
				"			INNER JOIN BilledCaseHistoriesT ON CaseHistoryT.ID = BilledCaseHistoriesT.CaseHistoryID "
				"			INNER JOIN BillsT ON BilledCaseHistoriesT.BillID = BillsT.ID "
				"			WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND CaseHistoryT.CompletedDate Is Not Null) "
				"OR ("
				//has no unbilled products that are billable
				"AllocationID NOT IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE Status = %li "
				"			AND ID NOT IN (SELECT AllocationDetailID FROM ChargedAllocationDetailsT "
				"			WHERE ChargeID IN (SELECT ID FROM LineItemT WHERE Deleted = 0)) "
				"			AND ProductID IN (SELECT ProductID FROM ProductLocationInfoT WHERE ProductLocationInfoT.Billable = 1 AND ProductLocationInfoT.LocationID = LocID)) "
				//does not have only released products
				"AND AllocationID NOT IN (SELECT ID FROM PatientInvAllocationsT WHERE Status = %li "
				"			AND ID NOT IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE Status = %li))"
				//does not have only non-billable products
				"AND AllocationID NOT IN (SELECT ID FROM PatientInvAllocationsT WHERE Status = %li "
				"			AND ID NOT IN (SELECT AllocationID FROM PatientInvAllocationDetailsT WHERE ProductID IN "
				"				(SELECT ProductID FROM ProductLocationInfoT WHERE ProductLocationInfoT.Billable = 1 AND ProductLocationInfoT.LocationID = LocID))) "
				"))",
				InvUtils::iasCompleted, InvUtils::iadsUsed,
				InvUtils::iadsUsed,
				InvUtils::iasCompleted, InvUtils::iadsUsed,
				InvUtils::iasCompleted);

			//show the completion columns
			m_AllocationsList->GetColumn(alcCompletedDate)->PutStoredWidth(90);
			m_AllocationsList->GetColumn(alcCompletedBy)->PutStoredWidth(90);
		}
		else {
			//TES 7/18/2008 - PLID 29478 - Include "To Be Ordered" details as Active.
			strWhere.Format("Status = %li AND (DetailStatus Is Null OR DetailStatus IN (%li,%li))",
				InvUtils::iasActive, InvUtils::iadsActive, InvUtils::iadsOrder);

			//hide the completion columns
			m_AllocationsList->GetColumn(alcCompletedDate)->PutStoredWidth(0);
			m_AllocationsList->GetColumn(alcCompletedBy)->PutStoredWidth(0);
		}

		m_AllocationsList->PutWhereClause(_bstr_t(strWhere));

		//reload the list
		m_AllocationsList->Requery();

	}NxCatchAll("Error in CInvAllocationsDlg::Refresh");
}

BEGIN_EVENTSINK_MAP(CInvAllocationsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvAllocationsDlg)
	ON_EVENT(CInvAllocationsDlg, IDC_ALLOCATIONS_LIST, 19 /* LeftClick */, OnLeftClickAllocationsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvAllocationsDlg, IDC_ALLOCATIONS_LIST, 6 /* RButtonDown */, OnRButtonDownAllocationsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvAllocationsDlg::OnLeftClickAllocationsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2007-11-08 15:54) - PLID 28003 - The Patient/Product column is the only hyperlinked
		// column, although that is just an arbitrary decision.
		if(nCol == alcPatientName) {
			//edit the allocation

			//alcAllocationID represents the master allocation ID
			//regardless of whether they clicked on a parent row or child row
			long nAllocationID = VarLong(pRow->GetValue(alcAllocationID));
			
			CInvPatientAllocationDlg dlg(this);
			dlg.m_nID = nAllocationID;
			if(dlg.DoModal() == IDOK) {
				//requery the list to reflect any changes to the allocation
				m_AllocationsList->Requery();
			}
		}

	}NxCatchAll("Error in CInvAllocationsDlg::OnLeftClickAllocationsList");
}

void CInvAllocationsDlg::OnRButtonDownAllocationsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	long nAuditTransactionID = -1;
	int nInvTodoTransactionID = -1; // (c.haag 2008-02-29 14:41) - PLID 29115 - Support for inventory todo transactions

	try {

		//create a right-click menu with the option to delete an allocation

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		m_AllocationsList->PutCurSel(pRow);

		enum AllocationsListMenuItems
		{
			almiGoToPatient = 1,
			almiComplete = 2,	// (j.jones 2007-11-27 10:33) - PLID 28196
			almiDelete = 3,
		};

		// (j.jones 2007-11-12 15:24) - PLID 28074 - if they don't have permissions, show
		// that the options exist, but gray them out
		BOOL bDeletePermission = (GetCurrentUserPermissions(bioInventoryAllocation) & SPT_____D______ANDPASS);
		BOOL bWritePermission = (GetCurrentUserPermissions(bioInventoryAllocation) & SPT___W________ANDPASS);

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, almiGoToPatient, "Go To Patient");
		// (j.jones 2007-11-27 10:34) - PLID 28196 - added ability to right click and complete an active allocation
		if((InvUtils::InventoryAllocationStatus)VarLong(pRow->GetValue(alcStatus)) != InvUtils::iasCompleted) {
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|(bWritePermission ? 0 : MF_GRAYED), almiComplete, "Complete Allocation");
		}
		mnu.AppendMenu(MF_SEPARATOR);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|(bDeletePermission ? 0 : MF_GRAYED), almiDelete, "Delete Allocation");

		CPoint pt;
		GetCursorPos(&pt);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		switch(nResult)
		{
			case almiGoToPatient: {

				long nPatientID = VarLong(pRow->GetValue(alcPersonID));

				if (nPatientID != -1) {
					//Set the active patient
					CMainFrame *pMainFrame;
					pMainFrame = GetMainFrame();
					if (pMainFrame != NULL) {

						if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
							if(IDNO == MessageBox("This patient is not in the current lookup. \n"
								"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return;
							}
						}
						//TES 1/7/2010 - PLID 36761 - This function may fail now
						if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {
							return;
						}

						//Now just flip to the patient's module and set the active Patient
						pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
						CNxTabView *pView = pMainFrame->GetActiveView();
						if(pView)
							pView->UpdateView();

					}//end if MainFrame
					else {
						MsgBox(MB_ICONSTOP|MB_OK, "Error switching to patient - InvAllocationsDlg.cpp: Cannot Open Mainframe");
					}//end else pMainFrame
				}//end if nPatientID

			}
			break;

			// (j.jones 2007-11-27 10:34) - PLID 28196 - added ability to right click and complete an allocation
			case almiComplete: {

				//confirm they have permission
				if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptWrite)) {
					return;
				}

				//alcAllocationID represents the master allocation ID
				//regardless of whether they clicked on a parent row or child row
				long nAllocationID = VarLong(pRow->GetValue(alcAllocationID));

				CInvAllocationUsageDlg dlg(this);
				//the dialog will create a new object based on the ID
				dlg.SetAllocationInfo(nAllocationID);
				//they can't save the dialog without completing the allocation
				dlg.m_bForceCompletion = TRUE;
				if(dlg.DoModal() == IDOK) {
					Refresh();
				}
			}
			break;

			case almiDelete: {

				// (j.jones 2007-11-12 15:26) - PLID 28074 - confirm they have permission
				if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptDelete)) {
					return;
				}

				//delete the allocation, but not without a strong warning first
				CString strWarn = "Are you sure you wish to delete this allocation and return all of its products to inventory?";
				if(!m_radioActive.GetCheck()) {
					//it is possible for part of an allocation to be billed and be in the "unbilled" list,
					//so just note that all the unbilled products will be returned to inventory
					strWarn = "Are you sure you wish to delete this allocation and return all of its unbilled products to inventory?";
				}

				if(IDYES == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO)
					&& IDYES == MessageBox("Are you SURE you wish to delete this allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					//alcAllocationID represents the master allocation ID
					//regardless of whether they clicked on a parent row or child row
					long nAllocationID = VarLong(pRow->GetValue(alcAllocationID));

					// (j.jones 2007-11-16 12:18) - PLID 28043 - audit this deletion
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					//for auditing, we need to look up the information from the allocation in data
					long nPatientID = -1;
					long nAppointmentID = -1;
					CString strPatientName = "";
					_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, AppointmentID FROM PatientInvAllocationsT WHERE ID = {INT}\r\n"
						""
						"SELECT ServiceT.Name AS ProductName, SerialNum, ExpDate, Quantity "
						"FROM PatientInvAllocationDetailsT "
						"LEFT JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
						"LEFT JOIN ProductItemsT ON PatientInvAllocationDetailsT.ProductItemID = ProductItemsT.ID "
						"WHERE PatientInvAllocationDetailsT.AllocationID = {INT} AND PatientInvAllocationDetailsT.Status <> {INT}", nAllocationID, nAllocationID, InvUtils::iadsDeleted);
					if(!rs->eof) {
						nPatientID = AdoFldLong(rs, "PatientID");
						nAppointmentID = AdoFldLong(rs, "AppointmentID", -1);
						//first audit deleting the master record
						//no real description to give it, it's defined by its details
						strPatientName = GetExistingPatientName(nPatientID);
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDeleted, nAllocationID, "", "<Deleted>", aepMedium, aetDeleted);
					}

					rs = rs->NextRecordset(NULL);
					
					//now loop through any undeleted details
					while(!rs->eof) {
						
						CString strDesc;

						CString strProductName = AdoFldString(rs, "ProductName","");
						_variant_t varSerialNumber = rs->Fields->Item["SerialNum"]->Value;
						_variant_t varExpDate = rs->Fields->Item["ExpDate"]->Value;
						double dblQuantity = AdoFldDouble(rs, "Quantity",1.0);
						
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
						if(dblQuantity != 1.0) {
							//only show the quantity if not 1.0
							CString str;
							str.Format(", Quantity: %g", dblQuantity);
							strDesc += str;
						}

						//and now audit using this description
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailDeleted, nAllocationID, strDesc, "<Deleted>", aepMedium, aetDeleted);

						rs->MoveNext();
					}
					rs->Close();					

					//delete the allocation
					CString strSqlBatch = BeginSqlBatch();

					// (j.jones 2008-02-27 14:27) - PLID 29102 - remove links from case histories
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CaseHistoryAllocationLinkT WHERE AllocationID = %li", nAllocationID);

					// (j.jones 2008-02-27 14:37) - PLID 29104 - audit those link deletions
					_RecordsetPtr rsCaseHistInfo = CreateParamRecordset("SELECT CaseHistoryT.ID AS CaseHistoryID, CaseHistoryT.Name, "
						"PatientInvAllocationsT.ID AS AllocationID, PatientInvAllocationsT.InputDate "
						"FROM CaseHistoryT "
						"INNER JOIN CaseHistoryAllocationLinkT ON CaseHistoryT.ID = CaseHistoryAllocationLinkT.CaseHistoryID "
						"INNER JOIN PatientInvAllocationsT ON CaseHistoryAllocationLinkT.AllocationID = PatientInvAllocationsT.ID "
						"WHERE PatientInvAllocationsT.ID = {INT}", nAllocationID);
					//with the current design, there should never be more than one linked case history, but the 
					//structure allows it, so let's make it a while loop for accuracy
					while(!rsCaseHistInfo->eof) {
						long nCaseHistoryID = AdoFldLong(rsCaseHistInfo, "CaseHistoryID");
						CString strCaseHistoryName = AdoFldString(rsCaseHistInfo, "Name");
						long nAllocationID = AdoFldLong(rsCaseHistInfo, "AllocationID");
						COleDateTime dtInput = AdoFldDateTime(rsCaseHistInfo, "InputDate");

						CString strAllocationDescription;
						strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(dtInput, NULL, dtoDate));

						//need to audit that we removed the link from both the case history and the allocation
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nCaseHistoryID, strCaseHistoryName + ", " + strAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, nAllocationID, strAllocationDescription + ", Case History: " + strCaseHistoryName, "<No Linked Case History>", aepMedium, aetDeleted);
						rsCaseHistInfo->MoveNext();
					}
					rsCaseHistInfo->Close();

					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Status = %li WHERE AllocationID = %li", InvUtils::iadsDeleted, nAllocationID);
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationsT SET Status = %li WHERE ID = %li", InvUtils::iasDeleted, nAllocationID);
					ExecuteSqlBatch(strSqlBatch);

					//commit the audit
					if(nAuditTransactionID != -1) {
						CommitAuditTransaction(nAuditTransactionID);
						nAuditTransactionID = -1;
					}

					// (c.haag 2008-02-29 14:44) - PLID 29115 - Update inventory todo alarms
					nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, nAllocationID);
					//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
					InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);

					// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
					if(nAppointmentID != -1) {
						TrySendAppointmentTablecheckerForInventory(nAppointmentID, TRUE);
					}
					
					if(pRow->GetParentRow() == NULL) {
						//we must be the parent, so remove ourselves
						m_AllocationsList->RemoveRow(pRow);
					}
					else {
						//otherwise remove our parent
						m_AllocationsList->RemoveRow(pRow->GetParentRow());
					}
				}
				break;

			} // case almiDelete:

		} // switch(nResult)

	}NxCatchAllCall("Error in CInvAllocationsDlg::OnRButtonDownAllocationsList",
		// (j.jones 2007-11-16 12:19) - PLID 28043 - supported auditing
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		// (c.haag 2008-02-29 14:43) - PLID 29115 - Roll back any inventory todo transactions
		if (-1 != nInvTodoTransactionID) {
			InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
		}
	);
}

// (j.jones 2007-11-27 16:11) - PLID 28196 - added allocation filters
void CInvAllocationsDlg::OnRadioActiveAllocations() 
{
	Refresh();
}

void CInvAllocationsDlg::OnRadioCompletedAndBilled() 
{
	Refresh();
}

void CInvAllocationsDlg::OnRadioCompletedNotBilled() 
{
	Refresh();	
}

HBRUSH CInvAllocationsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	try {
	
		if (nCtlColor == CTLCOLOR_STATIC)
		{
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00FFDBDB));
			return m_brush;
		}
		
	}NxCatchAll("Error in CInvAllocationsDlg::OnCtlColor");

	return hbr;
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (j.jones 2007-12-04 10:34) - PLID 28269 - added ability to mass complete allocations
void CInvAllocationsDlg::OnBtnCompleteMultipleAllocations() 
{
	try {

		//confirm they have permission
		if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptWrite)) {
			return;
		}

		CInvBatchCompleteAllocationsDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//refresh the list to reflect changes
			Refresh();
		}

	}NxCatchAll("Error in CInvAllocationsDlg::OnBtnCompleteMultipleAllocations");
}

void CInvAllocationsDlg::GetCurrentFilters(long& nStatus)
{
	// (c.haag 2008-01-07 16:18) - PLID 28084 - Calculate the filters for the Allocation List report
	if (m_radioCompleteNotBilled.GetCheck()) {
		nStatus = 1;
	}
	else if (m_radioCompleteBilled.GetCheck()) {
		nStatus = 2;
	}
	else if (m_radioActive.GetCheck()) {
		nStatus = 3;
	}
	else {
		nStatus = -1;
	}
}

void CInvAllocationsDlg::OnConfigureRequiredAllocations() 
{
	try {
		//TES 6/12/2008 - PLID 28078 - Open up the configuration dialog.
		CApptsRequiringAllocationsDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll("Error in CInvAllocationsDlg::OnConfigureRequiredAllocations()");
}
