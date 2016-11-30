// InvPatientAllocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvPatientAllocationDlg.h"
#include "barcode.h"
#include "ProductItemsDlg.h"
#include "invutils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "InvAllocationUsageDlg.h"
#include "GlobalSchedUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "globalReportUtils.h"

// (j.jones 2007-11-05 17:08) - PLID 27987 - created

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CInvPatientAllocationDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum PatientComboColumn {

	patccID = 0,
	patccUserDefinedID,
	patccLast,
	patccFirst,
	patccMiddle,
	//patccFullname, // (a.walling 2010-06-01 08:31) - PLID 38917 - No more FullName column
};

enum LocationComboColumn {

	lccID = 0,
	lccName,
};

// (j.jones 2008-03-05 11:53) - PLID 29201 - added provider combo
enum ProviderComboColumn {
	
	provccID  = 0,
	provccName,
};

enum SelectionComboColumn {

	selccID = 0,
	selccName,
};

enum ProductComboColumn {

	pccID = 0,
	pccCategory,
	pccSupplier,
	pccName,
	pccPrice,
	pccBarcode,
	pccHasSerialNum,
	pccHasExpDate,
	pccSerialNumIsLotNum, //TES 7/3/2008 - PLID 24726 - Needed for barcode processing
};

enum SurgeryComboColumn {

	sccID = 0,
	sccSurgeryName,
	sccTotal,
};

enum QuoteComboColumn {

	qccID = 0,
	qccDate,
	qccDescription,
	qccTotal,
	qccHasBeenBilled,
};

enum AppointmentComboColumn {

	accID = 0,
	accDateTime,
	accTypePurpose,
	accResource,
	accDateOnly,
};

enum AllocationListColumn {

	alcID = 0,
	alcServiceID,
	alcProductItemID,
	alcName,
	alcOldQuantity,	// (j.jones 2007-11-16 10:55) - PLID 28043 - added for auditing
	alcQuantity,
	alcSerialNumber,
	alcExpDate,
	alcProductType,	// (c.haag 2008-03-11 12:25) - PLID 29255 - Used to identify purchased inv./consignment status
	alcOldStatus,
	alcOrder,		//TES 7/18/2008 - PLID 29478 - A new status
	alcAllocated,
	alcUsed,
	alcReleased,
	alcReturn,	//TES 6/20/2008 - PLID 26152 - Added a column for the ToBeReturned status
	alcBilled,	// (j.jones 2007-12-12 11:57) - PLID 27988 - used for caching the billed status
	alcCaseHistory,		// (j.jones 2008-03-03 08:38) - PLID 29125 - caches the case history status
	alcNotes,
	alcOldNotes, // (j.jones 2007-12-06 10:33) - PLID 28043 - for auditing
	alcProductItemStatus, //TES 6/20/2008 - PLID 26152 - Added to store the actual purchased inv./consignement status, not the display name
	alcOldToBeReturned, //TES 6/20/2008 - PLID 26152 - For auditing
	alcSerialNumIsLotNum, //TES 7/3/2008 - PLID 24726 - Needed for barcode processing
	alcIsSerialized, //TES 7/18/2008 - PLID 29478 - Needed sometimes, now that alcProductItemID isn't a sure indicator of whether the product is serialized
};

enum SelectionTypes {

	seltypeProduct = 0,
	seltypeSurgery,
	seltypeQuote,
};

// (j.jones 2008-02-27 11:27) - PLID 29102 - added case history combo
enum CaseHistoryComboColumns
{
	chccID = 0,
	chccName,
	chccSurgeryDate,
	chccCompletedDate,
	chccAppointmentID,
};

CInvPatientAllocationDlg::CInvPatientAllocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvPatientAllocationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvPatientAllocationDlg)
		m_nID = -1;
		m_nCurPatientID = -1;
		m_nCurLocationID = -1;
		m_nPendingPatientID = -1;
		m_nPendingLocationID = -1;
		m_nPendingAppointmentID = -1;
		m_bSurgeriesRequeried = FALSE;
		m_bQuotesRequeried = FALSE;
		m_bDisableBarcode = FALSE;
		m_nOrigPatientID = -1;
		m_nOrigLocationID = -1;
		m_strOrigLocationName = "";
		m_nOrigAppointmentID = -1;
		m_strOrigApptDate = "";
		m_strOrigNotes = "";
		m_iasOrigStatus = InvUtils::iasActive;
		m_nDefaultPatientID = -1;
		m_nDefaultAppointmentID = -1;
		m_nDefaultLocationID = -1;
		m_bReadOnly = FALSE;
		m_nOrigCaseHistoryID = -1;		
		m_nDefaultCaseHistoryID = -1;
		m_nCurProviderID = -1;
		m_nDefaultProviderID = -1;
		m_nOrigProviderID = -1;
		m_nPendingProviderID = -1;
		m_strOrigProviderName = "";
		m_nCreateFromOrderID = -1;
	//}}AFX_DATA_INIT

	m_dtInputDate.m_status = COleDateTime::invalid;
}


void CInvPatientAllocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvPatientAllocationDlg)
	DDX_Control(pDX, IDC_BTN_PREVIEW_ALLOCATIONS, m_btnPreview);
	DDX_Control(pDX, IDC_BTN_COMPLETE_ALLOCATION, m_btnCompleteAllocation);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EDIT_ALLOCATION_NOTES, m_nxeditEditAllocationNotes);
	DDX_Control(pDX, IDC_CASE_HISTORY_SELECT_LABEL, m_nxstaticCaseHistorySelectLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvPatientAllocationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvPatientAllocationDlg)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_BTN_COMPLETE_ALLOCATION, OnBtnCompleteAllocation)
	ON_BN_CLICKED(IDC_BTN_PREVIEW_ALLOCATIONS, OnBtnPreviewAllocations)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvPatientAllocationDlg message handlers

BOOL CInvPatientAllocationDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		CWaitCursor pWait;

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));

		//register for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
				MsgBox("Error registering for barcode scans.  You may not be able to scan.");
			}
		}

		//TES 6/30/2008 - PLID 26152 - We've got two preferences on this dialog now, let's go ahead and bulk cache.
		g_propManager.CachePropertiesInBulk("InvAllocationUsage", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'Allocation_DefaultLocation' OR "
			"Name = 'ReturnReleasedGeneralSerializedItems' "
			"OR Name = 'PromptToBeOrderedAllocation' " // (d.thompson 2009-12-16) - PLID 35903
			")",
			_Q(GetCurrentUserName()));

		// (c.haag 2008-04-29 11:00) - PLID 29820 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCompleteAllocation.AutoSet(NXB_OK);

		// (j.gruber 2008-09-02 10:25) - PLID 30814 - added a preview button
		m_btnPreview.AutoSet(NXB_PRINT_PREV);

		//now set up our datalists
		m_PatientCombo = BindNxDataList2Ctrl(this, IDC_PATIENT_COMBO, GetRemoteData(), true);
		m_LocationCombo = BindNxDataList2Ctrl(this, IDC_LOCATION_ALLOC_COMBO, GetRemoteData(), true);
		m_SelectionCombo = BindNxDataList2Ctrl(this, IDC_ALLOC_ADD_COMBO, GetRemoteData(), false);
		m_ProductCombo = BindNxDataList2Ctrl(this, IDC_PRODUCT_ALLOC_COMBO, GetRemoteData(), true);
		m_SurgeryCombo = BindNxDataList2Ctrl(this, IDC_SURGERY_ALLOC_COMBO, GetRemoteData(), true);
		m_QuoteCombo = BindNxDataList2Ctrl(this, IDC_QUOTE_ALLOC_COMBO, GetRemoteData(), false);
		m_AppointmentCombo = BindNxDataList2Ctrl(this, IDC_APPOINTMENT_COMBO, GetRemoteData(), false);
		m_AllocationList = BindNxDataList2Ctrl(this, IDC_ALLOCATION_LIST, GetRemoteData(), false);
		// (j.jones 2008-02-27 11:14) - PLID 29102 - added the case history link
		m_CaseHistoryCombo = BindNxDataList2Ctrl(this, IDC_CASE_HISTORY_COMBO, GetRemoteData(), false);
		// (j.jones 2008-03-05 11:47) - PLID 29201 - added provider to allocations
		m_ProviderCombo = BindNxDataList2Ctrl(this, IDC_PROVIDER_ALLOC_COMBO, GetRemoteData(), true);

		//having no provider is allowed, so add that row
		IRowSettingsPtr pRow = m_ProviderCombo->GetNewRow();
		pRow->PutValue(provccID, (long)-1);
		pRow->PutValue(provccName, _bstr_t(" < No Provider Selected >"));
		m_ProviderCombo->AddRowSorted(pRow, NULL);

		//hide the surgery and quote combos
		GetDlgItem(IDC_SURGERY_ALLOC_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_QUOTE_ALLOC_COMBO)->ShowWindow(SW_HIDE);

		//set up the selection combo
		pRow = m_SelectionCombo->GetNewRow();
		pRow->PutValue(selccID, (long)seltypeProduct);
		pRow->PutValue(selccName, _bstr_t("Inventory Item"));
		m_SelectionCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_SelectionCombo->GetNewRow();
		pRow->PutValue(selccID, (long)seltypeSurgery);
		pRow->PutValue(selccName, _bstr_t("Surgery"));
		m_SelectionCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_SelectionCombo->GetNewRow();
		pRow->PutValue(selccID, (long)seltypeQuote);
		pRow->PutValue(selccName, _bstr_t("Existing Quote"));
		m_SelectionCombo->AddRowAtEnd(pRow, NULL);

		//and default to Inventory Item
		m_SelectionCombo->SetSelByColumn(selccID, (long)seltypeProduct);

		// (j.jones 2008-02-27 11:15) - PLID 29102 - hide the case history combo
		// if they do not have the ASC license
		if(!IsSurgeryCenter(FALSE)) {
			//they don't have the license, so hide the controls
			GetDlgItem(IDC_CASE_HISTORY_SELECT_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CASE_HISTORY_COMBO)->ShowWindow(SW_HIDE);
		}

		if(m_nID != -1) {
			//load an existing batch

			// (j.jones 2007-11-12 15:40) - PLID 28074 - check for write permissions
			BOOL bAccess = GetCurrentUserPermissions(bioInventoryAllocation) & sptWrite;
			BOOL bAccessWithPass = GetCurrentUserPermissions(bioInventoryAllocation) & sptWriteWithPass;
			if (!bAccess && bAccessWithPass) {
				// Prompt for password
				if (CheckCurrentUserPassword())	{
					bAccess = TRUE;
				}
			}
			if(!bAccess) {
				//give a clean warning
				AfxMessageBox("You do not have permissions to edit patient allocations. The allocation will be read-only.");
				m_bReadOnly = TRUE;
				SecureControls();
			}

			//first load the master allocation information, then we also need quantity counts per product
			// (j.jones 2007-11-16 11:41) - PLID 28043 - altered the query to pull more data for auditing needs
			// (j.jones 2008-02-27 11:45) - PLID 29102 - pulled in the linked Case History ID
			// (j.jones 2008-03-05 12:03) - PLID 29201 - added ProviderID
			_RecordsetPtr rs = CreateParamRecordset("SELECT PatientInvAllocationsT.PatientID, PatientInvAllocationsT.InputDate, "
				"PatientInvAllocationsT.LocationID, LocationsT.Name AS LocationName, PatientInvAllocationsT.AppointmentID, "
				"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime, "
				"PatientInvAllocationsT.Notes, PatientInvAllocationsT.Status, CaseHistoryAllocationLinkT.CaseHistoryID, "
				"PatientInvAllocationsT.ProviderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName "
				"FROM PatientInvAllocationsT "
				"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
				"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
				"LEFT JOIN CaseHistoryAllocationLinkT ON PatientInvAllocationsT.ID = CaseHistoryAllocationLinkT.AllocationID "
				"LEFT JOIN PersonT ON PatientInvAllocationsT.ProviderID = PersonT.ID "
				"WHERE PatientInvAllocationsT.Status <> {INT} AND PatientInvAllocationsT.ID = {INT};"
				""
				//this section of the query is only to calculate the existing quantities in use / on hold
				"SELECT ProductID, Sum(Quantity) AS TotalQuantity "
				"FROM PatientInvAllocationDetailsT WHERE (Status = {INT} OR Status = {INT}) AND AllocationID = {INT} "
				"GROUP BY ProductID", InvUtils::iasDeleted, m_nID, InvUtils::iadsActive, InvUtils::iadsUsed, m_nID);

			if(rs->eof) {
				//this allocation no longer exists!
				AfxMessageBox("The selected inventory allocation no longer exists. It may have been deleted or used by the patient.");
				CDialog::OnCancel();
				return TRUE;
			}
			else {

				//requery the allocation list
				// (c.haag 2007-12-03 15:45) - PLID 28220 - Now that ProductItemsT has a Status field, we need to
				// prevent any ambiguities with table names
				CString strWhere;
				strWhere.Format("AllocationID = %li AND PatientInvAllocationDetailsT.Status <> %li", m_nID, InvUtils::iadsDeleted);
				m_AllocationList->PutWhereClause(_bstr_t(strWhere));
				m_AllocationList->Requery();

				m_dtInputDate = AdoFldDateTime(rs, "InputDate");

				//load the allocation data onto the screen

				//Status
				m_iasOrigStatus = (InvUtils::InventoryAllocationStatus)AdoFldLong(rs, "Status", InvUtils::iasActive);

				//LocationID
				m_nCurLocationID = AdoFldLong(rs, "LocationID");
				// (j.jones 2007-11-16 11:33) - PLID 28043 - needed for auditing
				m_nOrigLocationID = m_nCurLocationID;
				m_strOrigLocationName = AdoFldString(rs, "LocationName", "");
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				long nSel = m_LocationCombo->TrySetSelByColumn_Deprecated(lccID, m_nCurLocationID);
				if(nSel == sriNoRow) {
					//maybe it's inactive?
					_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_nCurLocationID);
					if(!rsLoc->eof) {
						m_nPendingLocationID = m_nCurLocationID;
						m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
					}
					else 
						m_LocationCombo->PutCurSel(NULL);
				}
				else if(nSel == sriNoRowYet_WillFireEvent) {
					m_nPendingLocationID = m_nCurLocationID;
				}

				//PatientID
				m_nCurPatientID = AdoFldLong(rs, "PatientID");
				// (j.jones 2007-11-16 11:33) - PLID 28043 - needed for auditing
				m_nOrigPatientID = m_nCurPatientID;
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				nSel = m_PatientCombo->TrySetSelByColumn_Deprecated(patccID, m_nCurPatientID);
				if(nSel == sriNoRow) {
					//maybe it's inactive?
					_RecordsetPtr rsPatient = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
						"WHERE ID = {INT}", m_nCurPatientID);
					if(!rsPatient->eof) {
						m_nPendingPatientID = m_nCurPatientID;
						m_PatientCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPatient, "Name", "")));
					}
					else {
						m_PatientCombo->PutCurSel(NULL);
					}
				}
				else if(nSel == sriNoRowYet_WillFireEvent) {
					m_nPendingPatientID = m_nCurPatientID;
				}

				// (j.jones 2008-02-27 11:20) - PLID 29102 - requery the case history list (if they have ASC)
				if(IsSurgeryCenter(FALSE)) {
					strWhere.Format("PersonID = %li "
						"AND ID NOT IN (SELECT CaseHistoryID FROM CaseHistoryAllocationLinkT WHERE AllocationID <> %li) "
						"AND LocationID = %li", m_nCurPatientID, m_nID, m_nCurLocationID);
					m_CaseHistoryCombo->PutWhereClause(_bstr_t(strWhere));
					m_CaseHistoryCombo->Requery();

					long nCaseHistoryID = AdoFldLong(rs, "CaseHistoryID", -1);
					if(nCaseHistoryID != -1) {
						IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->SetSelByColumn(chccID, nCaseHistoryID);
						if(pCaseRow) {
							// (j.jones 2008-02-27 11:57) - PLID 29104 - store this information for auditing purposes
							m_nOrigCaseHistoryID = nCaseHistoryID;
							m_strOrigCaseHistoryName = VarString(pCaseRow->GetValue(chccName), "");
						}
						else {
							//why isn't the case in this list? is it bad data?
							ASSERT(FALSE);
						}
					}
				}

				//set the quote from clause to include this patient ID
				ResetQuotesFromClause();

				//requery the appt. list for this patient
				strWhere.Format("PatientID = %li", m_nCurPatientID);
				m_AppointmentCombo->PutWhereClause(_bstr_t(strWhere));
				m_AppointmentCombo->Requery();

				//AppointmentID (may be NULL)
				long nAppointmentID = AdoFldLong(rs, "AppointmentID", -1);
				// (j.jones 2007-11-16 11:33) - PLID 28043 - needed for auditing
				m_nOrigAppointmentID = nAppointmentID;
				_variant_t varApptDate = rs->Fields->Item["ApptDateTime"]->Value;
				if(varApptDate.vt == VT_DATE) {
					m_strOrigApptDate = FormatDateTimeForInterface(VarDateTime(varApptDate), NULL, dtoDateTime);
				}
				if(nAppointmentID != -1) {
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					nSel = m_AppointmentCombo->TrySetSelByColumn_Deprecated(accID, nAppointmentID);
					//we do NOT support such a thing as an "inactive" appointment,
					//but we do need to know if our trysetsel is pending
					if(nSel == sriNoRowYet_WillFireEvent) {
						m_nPendingAppointmentID = nAppointmentID;
					}
				}

				// (j.jones 2008-03-05 12:04) - PLID 29201 - added ProviderID
				m_nCurProviderID = AdoFldLong(rs, "ProviderID", -1);
				m_nOrigProviderID = m_nCurProviderID;
				m_strOrigProviderName = AdoFldString(rs, "ProviderName", "");
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				nSel = m_ProviderCombo->TrySetSelByColumn_Deprecated(provccID, m_nCurProviderID);
				if(nSel == sriNoRow) {
					//maybe it's inactive?
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nCurProviderID);
					if(!rsProv->eof) {
						m_nPendingProviderID = m_nCurProviderID;
						m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
					}
					else 
						m_ProviderCombo->PutCurSel(NULL);
				}
				else if(nSel == sriNoRowYet_WillFireEvent) {
					m_nPendingProviderID = m_nCurProviderID;
				}
				
				//Notes
				m_strOrigNotes = AdoFldString(rs, "Notes",""); 
				SetDlgItemText(IDC_EDIT_ALLOCATION_NOTES, m_strOrigNotes);
			}

			//now get the quantity count recordset
			rs = rs->NextRecordset(NULL);

			//there shouldn't be an allocation with no products in it,
			//but it's not necessarily an error state, so for now
			//we are not checking for eof

			m_mapExistingQuantities.RemoveAll();

			while(!rs->eof) {
				//populate m_mapExistingQuantities with the totals

				long nProductID = AdoFldLong(rs, "ProductID");
				double dblTotalQty = AdoFldDouble(rs, "TotalQuantity", 0.0);

				//track this info. in our map
				m_mapExistingQuantities.SetAt(nProductID, dblTotalQty);

				rs->MoveNext();
			}
			rs->Close();
		}
		else {
			//default the display for a new batch

			//hide the active/used/released columns
			m_AllocationList->GetColumn(alcAllocated)->PutStoredWidth(0);
			m_AllocationList->GetColumn(alcUsed)->PutStoredWidth(0);
			m_AllocationList->GetColumn(alcReleased)->PutStoredWidth(0);
			//TES 6/20/2008 - PLID 26152 - Added a Return column.
			m_AllocationList->GetColumn(alcReturn)->PutStoredWidth(0);

			//TES 7/18/2008 - PLID 29478 - If we're creating this from an order, then the "To Be Ordered" column doesn't
			// make sense.
			if(m_nCreateFromOrderID != -1) {
				m_AllocationList->GetColumn(alcOrder)->PutStoredWidth(0);
			}

			//TES 6/23/2008 - PLID 30469 - FIRST of all, if we have an overriding default location, use that.
			long nOverrideDefaultLocation = GetRemotePropertyInt("Allocation_DefaultLocation", -1, 0, "<None>", true);
			if(nOverrideDefaultLocation != -1) {
				m_nCurLocationID = nOverrideDefaultLocation;
			}
			else {
				//set the default location, if one was given to us
				if(m_nDefaultLocationID != -1) {
					m_nCurLocationID = m_nDefaultLocationID;
				}
				else {
					//set the location to our current location			
					m_nCurLocationID = GetCurrentLocationID();
				}
			}

			//set the default patient, if one was given to us
			if(m_nDefaultPatientID != -1) {
				m_nCurPatientID = m_nDefaultPatientID;
				//we do not need to set any "original" info for auditing
				//we don't need to check for the patient not being loaded
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_PatientCombo->TrySetSelByColumn_Deprecated(patccID, m_nCurPatientID);

				//and requery the appt. list for this patient
				CString strWhere;
				strWhere.Format("PatientID = %li", m_nCurPatientID);
				m_AppointmentCombo->PutWhereClause(_bstr_t(strWhere));
				m_AppointmentCombo->Requery();

				// (j.jones 2008-02-27 11:20) - PLID 29102 - requery the case history list (if they have ASC)
				if(IsSurgeryCenter(FALSE)) {
					strWhere.Format("PersonID = %li "
						"AND ID NOT IN (SELECT CaseHistoryID FROM CaseHistoryAllocationLinkT) "
						"AND LocationID = %li", m_nCurPatientID, m_nCurLocationID);
					m_CaseHistoryCombo->PutWhereClause(_bstr_t(strWhere));
					m_CaseHistoryCombo->Requery();
				}

				//TES 8/25/2008 - PLID 31164 - Initialize the "Existing Quote" dropdown for this patient.
				ResetQuotesFromClause();
			}

			//set the default appointment, if one was given to us,
			//though if we weren't given a patient ID, we can't select the appt

			if(m_nDefaultPatientID == -1 && m_nDefaultAppointmentID != -1) {
				//we should never pass in these bad parameters
				ASSERT(FALSE);
			}
			else if(m_nDefaultPatientID != -1 && m_nDefaultAppointmentID != -1) {
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				long nSel = m_AppointmentCombo->TrySetSelByColumn_Deprecated(accID, m_nDefaultAppointmentID);
				//we do not need to set any "original" info for auditing
				//we do NOT support such a thing as an "inactive" appointment,
				//but we do need to know if our trysetsel is pending
				if(nSel == sriNoRowYet_WillFireEvent) {
					m_nPendingAppointmentID = m_nDefaultAppointmentID;
				}
			}

			// (j.jones 2008-02-27 16:48) - PLID 29108 - added m_nDefaultCaseHistoryID
			if(m_nDefaultCaseHistoryID != -1 && IsSurgeryCenter(FALSE)) {

				//we must have been given a patient ID, which would have
				//requeried the case history list already
				ASSERT(m_nDefaultPatientID != -1);
				
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_CaseHistoryCombo->TrySetSelByColumn_Deprecated(chccID, m_nDefaultCaseHistoryID);
				//do not assert or fail if the case isn't in the list - that would
				//just mean that the user decided to make a new allocation from a
				//case history that already had one, after being told that it would
				//not be linked to that case history

				//now try to add those products from the case history

				// (j.jones 2008-03-07 12:24) - PLID 29108 - we removed this ability, though perhaps it may
				// return in the future
				//TryAddProductsFromCaseHistory(m_nDefaultCaseHistoryID, m_strDefaultCaseHistoryName);

				m_nDefaultCaseHistoryID = -1;
				m_strDefaultCaseHistoryName = "";
			}

			// (z.manning 2008-06-23 12:03) - PLID 30467 - If this is a new allocation and we do not yet
			// have a default provider ID, then let's see if we can pull one another way.
			if(m_nID == -1 && m_nDefaultProviderID == -1)
			{
				if(m_nDefaultAppointmentID != -1)
				{
					// (z.manning 2008-06-23 12:03) - PLID 30467 - We have an appt linked with this allocation
					// so let's use a provider ID associated with one of the resource(s) of that appt.
					_RecordsetPtr prsProviderID = CreateParamRecordset(
						"SELECT TOP 1 ResourceProviderLinkT.ProviderID "
						"FROM AppointmentsT "
						"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
						"LEFT JOIN ResourceProviderLinkT ON AppointmentResourceT.ResourceID = ResourceProviderLinkT.ResourceID "
						"WHERE AppointmentsT.ID = {INT} AND ResourceProviderLinkT.ResourceID IS NOT NULL "
						, m_nDefaultAppointmentID);
					if(!prsProviderID->eof) {
						m_nDefaultProviderID = AdoFldLong(prsProviderID, "ProviderID", -1);
					}
					prsProviderID->Close();
				}
				else if(m_nDefaultPatientID != -1)
				{
					// (z.manning 2008-06-23 12:04) - PLID 30467 - We don't have a linked appt, but we do
					// have a patient, so let's use the patient G1 provider as the allocation's provider.
					_RecordsetPtr prsProviderID = CreateParamRecordset(
						"SELECT MainPhysician "
						"FROM PatientsT "
						"WHERE PersonID = {INT} "
						, m_nDefaultPatientID);
					if(!prsProviderID->eof) {
						m_nDefaultProviderID = AdoFldLong(prsProviderID, "MainPhysician", -1);
					}
					prsProviderID->Close();
				}
			}

			// (j.jones 2008-03-05 12:04) - PLID 29201 - set the default provider, if one was given to us
			if(m_nDefaultProviderID != -1) {
				m_nCurProviderID = m_nDefaultProviderID;

				//don't bother trying to load an inactive provider, or caching info. for auditing
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_ProviderCombo->TrySetSelByColumn_Deprecated(provccID, m_nCurProviderID);
			}
			else {
				m_nCurProviderID = -1;
			}

			//we do not need to set any "original" info for auditing
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_LocationCombo->TrySetSelByColumn_Deprecated(lccID, m_nCurLocationID);

			// (j.jones 2008-03-20 09:20) - PLID 29311 - added ability to create from an inventory order
			if(m_nCreateFromOrderID != -1) {
				TryAddProductsFromCompletedOrder(m_nCreateFromOrderID);
			}

			// (j.jones 2007-11-30 10:28) - PLID 28196 - enable the "complete" button accordingly
			CheckEnableCompleteAllocationBtn();
		}

		// (j.jones 2008-01-07 14:41) - PLID 28479 - we no longer require the product to be billable
		// to be used in an allocation - which means we don't need this function

		//regardless of how it was set, now requery the products based on the selected location ID
		//ReflectProductsByLocationID();

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CInvPatientAllocationDlg::~CInvPatientAllocationDlg()
{
	try {

		//unregister for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::~CInvPatientAllocationDlg");	
}

// (j.jones 2007-11-27 10:48) - PLID 28196 - added a standalone Save function
BOOL CInvPatientAllocationDlg::Save()
{
	// (j.jones 2007-11-08 13:35) - PLID 28043 - supported auditing
	long nAuditTransactionID = -1;
	// (c.haag 2008-02-29 12:34) - PLID 29115 - Used for inventory todo alarm transactions
	int nInvTodoTransactionID = -1;

	try {

		//used for auditing
		CString strPatientName = GetExistingPatientName(m_nCurPatientID);

		//save the data and close the dialog

		if(m_nCurPatientID == -1) {
			AfxMessageBox("You must select a patient before this allocation can be saved.");
			return FALSE;
		}

		//TES 6/24/2008 - PLID 30469 - With this new preference it's theoretically possible that m_nCurLocationID could be set
		// to an invalid or even deleted location when initializing, so detect that case as well.
		if(m_nCurLocationID == -1 || (m_LocationCombo->CurSel == NULL && !m_LocationCombo->IsComboBoxTextInUse)) {
			AfxMessageBox("You must select a location before this allocation can be saved.");
			return FALSE;
		}

		//special handling for when no products are selected
		if(m_AllocationList->GetRowCount() == 0) {
			if(m_nID == -1) {
				//new allocation, no products selected
				//just tell them they can't save an empty allocation
				AfxMessageBox("You must have at least one product allocated before saving.");
				return FALSE;
			}
			else {
				//existing allocation, no products selected
				//prompt if they want to delete the allocation
				if(IDYES == MessageBox("The allocation cannot be saved without at least one product allocated.\n"
					"Do you wish to delete this allocation instead?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)
					&& IDYES == MessageBox("Are you SURE you wish to delete this allocation?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

					//ok, they want to delete it

					//delete the allocation, and all of its products
					//(remember, they should not be allowed to enter this dialog if the allocation has been used)

					CString strSqlBatch = BeginSqlBatch();					

					// (j.jones 2008-02-27 14:27) - PLID 29102 - remove links from case histories
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CaseHistoryAllocationLinkT WHERE AllocationID = %li", m_nID);

					// (j.jones 2008-02-27 14:37) - PLID 29104 - audit those link deletions
					_RecordsetPtr rsCaseHistInfo = CreateParamRecordset("SELECT CaseHistoryT.ID AS CaseHistoryID, CaseHistoryT.Name, "
						"PatientInvAllocationsT.ID AS AllocationID, PatientInvAllocationsT.InputDate "
						"FROM CaseHistoryT "
						"INNER JOIN CaseHistoryAllocationLinkT ON CaseHistoryT.ID = CaseHistoryAllocationLinkT.CaseHistoryID "
						"INNER JOIN PatientInvAllocationsT ON CaseHistoryAllocationLinkT.AllocationID = PatientInvAllocationsT.ID "
						"WHERE PatientInvAllocationsT.ID = {INT}", m_nID);
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
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nCaseHistoryID, strCaseHistoryName + ", " + strAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, nAllocationID, strAllocationDescription + ", Case History: " + strCaseHistoryName, "<No Linked Case History>", aepMedium, aetDeleted);
						rsCaseHistInfo->MoveNext();
					}
					rsCaseHistInfo->Close();

					//now mark all details as deleted
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Status = %li WHERE AllocationID = %li", InvUtils::iadsDeleted, m_nID);
					//and the allocation as deleted
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationsT SET Status = %li WHERE ID = %li", InvUtils::iasDeleted, m_nID);
					ExecuteSqlBatch(strSqlBatch);

					// (j.jones 2007-11-16 11:24) - PLID 28043 - audit these deletions
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					//first audit deleting the master record
					//no real description to give it, it's defined by its details
					AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDeleted, m_nID, "", "<Deleted>", aepMedium, aetDeleted);					

					//now audit all the deleted details
					for(int i=0; i<m_aryDeletedDetails.GetSize();i++) {
						long nDetailID = (long)m_aryDeletedDetails.GetAt(i);

						CString strOldValue, strDesc;
						if(m_mapDeletedDetailDesc.Lookup(nDetailID, strDesc) && !strDesc.IsEmpty()) {
							//we have a description
							strOldValue = strDesc;
						}
						else {
							//why didn't we have a description?
							ASSERT(FALSE);
						}
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailDeleted, m_nID, strOldValue, "<Deleted>", aepMedium, aetDeleted);
					}

					//m_aryDeletedDetails has us covered, because we can't get to this
					//block of code if we still had details in our master list

					//commit the audit
					if(nAuditTransactionID != -1) {
						CommitAuditTransaction(nAuditTransactionID);
						nAuditTransactionID = -1;
					}

					//now clear the deleted details list
					m_aryDeletedDetails.RemoveAll();
					// (j.jones 2007-11-16 11:09) - PLID 28043 - and their audit descriptions
					m_mapDeletedDetailDesc.RemoveAll();

					// (c.haag 2008-02-29 12:25) - PLID 29115 - Update inventory todo alarms
					nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, m_nID);
					//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
					InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);

					// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
					// it would be the previously saved appt. ID, not necessarily the current one
					if(m_nOrigAppointmentID != -1) {
						TrySendAppointmentTablecheckerForInventory(m_nOrigAppointmentID, TRUE);
					}
					
					//we did save something, so return TRUE
					return TRUE;
				}
				else {
					//they aren't deleting, so refuse to save with no products
					AfxMessageBox("You must have at least one product allocated before saving.");
					return FALSE;
				}
			}
		}

		//if editing an existing allocation, disallow a mix of "allocated" and
		//"used" details in the same allocation, and never allow saving
		//details without a status
		//TES 7/18/2008 - PLID 29478 - Added in the "To Be Ordered" column.
		BOOL bHasOrderProducts = FALSE;
		BOOL bHasAllocatedProducts = FALSE;
		BOOL bHasUsedProducts = FALSE;
		BOOL bHasReleasedProducts = FALSE;
		BOOL bHasProductsWithoutAStatus = FALSE;
		if(m_nID != -1) {
			IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
			while(pRow != NULL) {

				//we just need to know if we have any products with a given status
				BOOL bOrder = VarBool(pRow->GetValue(alcOrder), FALSE);
				BOOL bAllocated = VarBool(pRow->GetValue(alcAllocated), FALSE);
				BOOL bUsed = VarBool(pRow->GetValue(alcUsed), FALSE);
				BOOL bReleased = VarBool(pRow->GetValue(alcReleased), FALSE);

				if(bOrder) {
					bHasOrderProducts = TRUE;
				}

				if(bAllocated) {
					bHasAllocatedProducts = TRUE;
				}

				if(bUsed) {
					bHasUsedProducts = TRUE;
				}

				if(bReleased) {
					bHasReleasedProducts = TRUE;
				}

				if(!bOrder && !bAllocated && !bUsed && !bReleased) {
					bHasProductsWithoutAStatus = TRUE;
				}

				pRow = pRow->GetNextRow();
			}

			//if any products do not have a status, disallow saving
			if(bHasProductsWithoutAStatus) {
				AfxMessageBox("This allocation has products that are not marked as 'Allocated', 'Used', or 'Released'.\n\n"
					"An allocation cannot be saved unless each product has a status chosen.");
				return FALSE;
			}

			//if we have a mix of allocated and used/released products, disallow saving
			// (j.jones 2008-02-18 09:37) - PLID 28948 - changed to allow releasing on an unfinished
			// allocation, instead we only disallow using items
			if((bHasOrderProducts || bHasAllocatedProducts) && (bHasUsedProducts /*|| bHasReleasedProducts*/)) {
				AfxMessageBox("This allocation has products that are still marked as 'Allocated' and also has products that are marked as 'Used' and/or 'Released'.\n\n"
					"An allocation cannot be saved while partially completed. All the products in this allocation must be marked as 'Allocated' in order to be\n"
					"marked as 'non-completed', or instead they need to all be marked as 'Used' or 'Released' for a completed allocation.");
				return FALSE;
			}

			//if we only have released products, warn
			if(!bHasOrderProducts && !bHasUsedProducts && !bHasAllocatedProducts && bHasReleasedProducts) {

				// (j.jones 2008-03-12 11:47) - PLID 29102 - we can't allow this if it is linked to a case history
				long nCaseHistoryID = -1;
				IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->GetCurSel();
				if(pCaseRow) {
					nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
				}
				if(nCaseHistoryID != -1) {
					AfxMessageBox("This completed allocation is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n"
						"However, this allocation is linked to a case history. A completely released allocation cannot be linked to a case history.\n\n"
						"You must either unlink from the case history, or leave a product as not released.");
					return FALSE;
				}
				else {
					if(IDNO == MessageBox("This completed allocation is releasing all of its products back to Inventory, and is not saving any products as 'Used'.\n\n"
						"Are you sure you wish to save this allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						return FALSE;
					}
				}
			}

			//if we were previously completed, and now we're not, warn
			//TES 7/18/2008 - PLID 29478 - "To Be Ordered" products count as active.
			if((bHasOrderProducts || bHasAllocatedProducts) && m_iasOrigStatus == InvUtils::iasCompleted) {
				if(IDNO == MessageBox("Your changes to this allocation have changed its status from 'Completed' to 'Active'.\n\n"
					"Are you sure you wish to save this allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}

		//don't need to require an appointment
		long nAppointmentID = -1;
		CString strApptID = "NULL";
		CString strApptDate = "";
		IRowSettingsPtr pApptRow = m_AppointmentCombo->GetCurSel();
		if(pApptRow) {
			nAppointmentID = VarLong(pApptRow->GetValue(accID));
			if(nAppointmentID != -1) {
				strApptID.Format("%li", nAppointmentID);
				strApptDate = FormatDateTimeForInterface(VarDateTime(pApptRow->GetValue(accDateTime)), NULL, dtoDateTime);
			}
		}
		else {
			//if we still have a pending appt. ID,
			//use that ID
			if(m_nPendingAppointmentID != -1) {
				nAppointmentID = m_nPendingAppointmentID;
				strApptID.Format("%li", m_nPendingAppointmentID);
			}
		}

		// (j.jones 2008-03-05 12:34) - PLID 29201 - added ProviderID, which is nullable
		CString strProvID = "NULL";
		CString strProvName = "";
		if(m_nCurProviderID != -1) {
			strProvID.Format("%li", m_nCurProviderID);

			if(m_ProviderCombo->IsComboBoxTextInUse) {
				strProvName = (LPCTSTR)m_ProviderCombo->GetComboBoxText();
			}
			else {
				IRowSettingsPtr pProvRow = m_ProviderCombo->GetCurSel();
				if(pProvRow) {
					strProvName = VarString(pProvRow->GetValue(provccName), "");
				}
			}
		}

		CString strNotes = "";
		GetDlgItemText(IDC_EDIT_ALLOCATION_NOTES, strNotes);

		//if they get here, it is safe to save the allocation

		if(m_nID == -1) {
			//save a new allocation batch

			// (c.haag 2008-02-29 12:25) - PLID 29115 - Begin an inventory todo alarm transaction
			nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();			

			CString strSqlBatch = BeginSqlBatch();

			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nAllocationID INT");

			//create the master record
			// (j.jones 2008-03-05 12:33) - PLID 29201 - added ProviderID
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationsT "
				"(PatientID, LocationID, AppointmentID, ProviderID, Notes, InputDate, Status) "
				"VALUES (%li, %li, %s, %s, '%s', GetDate(), %li)", m_nCurPatientID, m_nCurLocationID,
				strApptID, strProvID, _Q(strNotes), InvUtils::iasActive);

			AddStatementToSqlBatch(strSqlBatch, "SET @nAllocationID = SCOPE_IDENTITY()");

			//now create each detail
			IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
			while(pRow != NULL) {

				//since this is a new allocation, all the details in the list
				//are going to be new, and there won't be any deleted details
				//to remove

				long nServiceID = VarLong(pRow->GetValue(alcServiceID));
				
				long nProductItemID = VarLong(pRow->GetValue(alcProductItemID), -1);
				CString strProductItemID = "NULL";
				if(nProductItemID != -1) {
					strProductItemID.Format("%li", nProductItemID);

					//TES 6/23/2008 - PLID 26152 - If this item was previously flagged as To Be Returned, it shouldn't
					// be any more.
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = 0 WHERE ID = %li", nProductItemID);
				}

				double dblQuantity = VarDouble(pRow->GetValue(alcQuantity), 1.0);

				CString strDetailNotes = VarString(pRow->GetValue(alcNotes), "");
				strDetailNotes.TrimLeft();
				strDetailNotes.TrimRight();

				//TES 7/18/2008 - PLID 29478 - New details can now be "To Be Ordered", as well as "Allocated"
				BOOL bOrder = VarBool(pRow->GetValue(alcOrder), FALSE);

				//create the detail record
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationDetailsT "
					"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes) "
					"VALUES (@nAllocationID, %li, %s, %g, %li, '%s')", nServiceID,
					strProductItemID, dblQuantity, bOrder ? InvUtils::iadsOrder : InvUtils::iadsActive, _Q(strDetailNotes));

				// (c.haag 2008-02-29 12:29) - PLID 29115 - Add this element to the inventory todo alarm transaction
				if (-1 != nProductItemID) {
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_ProductItemID, nProductItemID);
				}
				else if (-1 != nServiceID) {
					InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_ProductID, nServiceID);
				}

				pRow = pRow->GetNextRow();
			}

			// (j.jones 2008-02-27 11:58) - PLID 29102 - save the case history link, if there is one
			long nCaseHistoryID = -1;
			CString strCaseHistoryName = "";
			{
				IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->GetCurSel();
				if(pCaseRow) {
					nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
					strCaseHistoryName = VarString(pCaseRow->GetValue(chccName), "");
				}

				if(nCaseHistoryID != -1) {
					AddStatementToSqlBatch(strSqlBatch, 
						"INSERT INTO CaseHistoryAllocationLinkT (AllocationID, CaseHistoryID) "
						"VALUES (@nAllocationID, %li) ",nCaseHistoryID);
				}
			}

			//no, not a param recordset
			_RecordsetPtr rs = CreateRecordset(
				"SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n"
				"%s\r\n"
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nAllocationID AS NewAllocationID, GetDate() AS TodaysDate", strSqlBatch);

			if(!rs->eof) {
				//grab the new ID, since it is a public variable our caller may want to use
				m_nID = AdoFldLong(rs, "NewAllocationID");
				//grab the server time as well
				m_dtInputDate = AdoFldDateTime(rs, "TodaysDate");
			}
			rs->Close();

			// (j.jones 2007-11-16 10:37) - PLID 28043 - now audit the creation
			{
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				
				//we couldn't do this earlier because we needed m_nID
				//no real description to give it, it's defined by its details
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCreated, m_nID, "", "<Created New Allocation>", aepMedium, aetCreated);

				pRow = m_AllocationList->GetFirstRow();
				while(pRow != NULL) {
					//audit the creation of each detail in the list
					CString strDesc = GenerateAuditDescFromAllocationRow(pRow);

					//audit the new detail
					AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailCreated, m_nID, "", strDesc, aepMedium, aetCreated);

					pRow = pRow->GetNextRow();
				}

				// (j.jones 2008-02-27 12:01) - PLID 29104 - audit linking to a case history, if we have one
				if(nCaseHistoryID != -1) {
					//audit for both this allocation and for the case
					CString strAllocationDescription;
					strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(m_dtInputDate, NULL, dtoDate));

					AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkCreated, m_nID, strAllocationDescription + ", No Linked Case History", "Case History: " + strCaseHistoryName, aepMedium, aetCreated);
					AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkCreated, nCaseHistoryID, strCaseHistoryName + ", No Linked Allocation", strAllocationDescription, aepMedium, aetCreated);					
				}
			}
		
			//commit the audit
			if(nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
				nAuditTransactionID = -1;
			}

			// (c.haag 2008-02-29 12:31) - PLID 29115 - Complete the todo alarm transaction
			//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
			InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);

			// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
			if(nAppointmentID != -1) {
				TrySendAppointmentTablecheckerForInventory(nAppointmentID, TRUE);
			}
		}
		else {
			//update an existing allocation batch

			CString strSqlBatch = BeginSqlBatch();

			InvUtils::InventoryAllocationStatus iasStatus = InvUtils::iasActive;

			// (j.jones 2007-11-29 09:08) - PLID 28196 - added completion fields
			CString strUpdateCompletionFields = ", CompletionDate = NULL, CompletedBy = NULL";

			//if all the items are no-longer allocated, mark completed
			//TES 7/18/2008 - PLID 29478 - "To Be Ordered" products count as active.
			if(!bHasOrderProducts && !bHasAllocatedProducts) {
				iasStatus = InvUtils::iasCompleted;

				//only update these fields if the status changed
				if(iasStatus != m_iasOrigStatus) {
					strUpdateCompletionFields.Format(", CompletionDate = GetDate(), CompletedBy = %li", GetCurrentUserID());
				}
				else {
					strUpdateCompletionFields = "";
				}
			}

			//update the master record
			// (j.jones 2008-03-05 12:33) - PLID 29201 - added ProviderID
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationsT "
				"SET PatientID = %li, LocationID = %li, AppointmentID = %s, ProviderID = %s, Notes = '%s', Status = %li %s "
				"WHERE ID = %li", m_nCurPatientID, m_nCurLocationID, strApptID, strProvID, _Q(strNotes), iasStatus, strUpdateCompletionFields, m_nID);

			// (j.jones 2007-11-16 11:28) - PLID 28043 - audit anything that changed
			
			//audit if the patient changed
			if(m_nCurPatientID != m_nOrigPatientID) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOldValue;
				strOldValue = GetExistingPatientName(m_nOrigPatientID);
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationPatient, m_nID, strOldValue, strPatientName, aepHigh, aetChanged); //this intentionally uses aepHigh
			}
			//audit if the location changed
			if(m_nCurLocationID != m_nOrigLocationID) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strNewValue;
				IRowSettingsPtr pLocRow = m_LocationCombo->GetCurSel();
				if(pLocRow == NULL) {
					//how is it possible for no selection
					//and the location ID to differ from
					//the original location ID?
					ASSERT(FALSE);
				}
				else {
					strNewValue = VarString(pLocRow->GetValue(lccName));
				}
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationLocation, m_nID, m_strOrigLocationName, strNewValue, aepMedium, aetChanged);
			}
			//audit if the appointment changed
			if(nAppointmentID != m_nOrigAppointmentID) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOldValue, strNewValue;
				if(m_nOrigAppointmentID == -1) {
					strOldValue = "<No Appointment>";
				}
				else {
					strOldValue.Format("Appointment: %s", m_strOrigApptDate);
				}
				if(nAppointmentID == -1) {
					strNewValue = "<No Appointment>";
				}
				else {
					strNewValue.Format("Appointment: %s", strApptDate);
				}
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationAppointment, m_nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
			// (j.jones 2008-03-05 12:35) - PLID 29201 - audit if the provider changed
			if(m_nCurProviderID != m_nOrigProviderID) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOldValue, strNewValue;
				if(m_strOrigProviderName.IsEmpty()) {
					strOldValue = "< No Provider >";
				}
				else {
					strOldValue = m_strOrigProviderName;
				}
				if(strProvName.IsEmpty()) {
					strNewValue = "< No Provider >";
				}
				else {
					strNewValue = strProvName;
				}
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationProvider, m_nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}
			//audit if the notes changed
			if(strNotes != m_strOrigNotes) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationNotes, m_nID, m_strOrigNotes, strNotes, aepLow, aetChanged);	//this intentionally uses aepLow
			}
			//audit if the status changed
			if(iasStatus != m_iasOrigStatus) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOldValue, strNewValue;
				AuditEventItems aeiItem = aeiInvAllocationActive;
				if(m_iasOrigStatus == InvUtils::iasActive) {
					strOldValue = "Active";
					strNewValue = "Completed";
					aeiItem = aeiInvAllocationCompleted;
				}
				else if(m_iasOrigStatus == InvUtils::iasCompleted) {
					strOldValue = "Completed";
					strNewValue = "Active";
					aeiItem = aeiInvAllocationActive;
				}
				else {
					//should be impossible
					ASSERT(FALSE);
				}
				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiItem, m_nID, strOldValue, strNewValue, aepMedium, aetChanged);
			}

			//now delete details
			for(int i=0; i<m_aryDeletedDetails.GetSize();i++) {
				long nDetailID = (long)m_aryDeletedDetails.GetAt(i);
				AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET Status = %li WHERE ID = %li", InvUtils::iadsDeleted, nDetailID);

				// (j.jones 2007-11-16 11:16) - PLID 28043 - audit the deletion
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOldValue, strDesc;
				if(m_mapDeletedDetailDesc.Lookup(nDetailID, strDesc) && !strDesc.IsEmpty()) {
					//we have a description
					strOldValue = strDesc;
				}
				else {
					//why didn't we have a description?
					ASSERT(FALSE);
				}

				AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailDeleted, m_nID, strOldValue, "<Deleted>", aepMedium, aetDeleted);
			}

			//now loop through all our current details
			IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
			while(pRow != NULL) {
				
				long nDetailID = VarLong(pRow->GetValue(alcID), -1);

				long nServiceID = VarLong(pRow->GetValue(alcServiceID));

				CString strProductName = VarString(pRow->GetValue(alcName), "");
				
				long nProductItemID = VarLong(pRow->GetValue(alcProductItemID), -1);
				CString strProductItemID = "NULL";
				if(nProductItemID != -1) {
					strProductItemID.Format("%li", nProductItemID);
				}

				double dblQuantity = VarDouble(pRow->GetValue(alcQuantity), 1.0);

				//determine the status
				InvUtils::InventoryAllocationDetailStatus iadsStatus = InvUtils::iadsActive;

				if(VarBool(pRow->GetValue(alcUsed), FALSE)) {
					iadsStatus = InvUtils::iadsUsed;
				}
				else if(VarBool(pRow->GetValue(alcReleased), FALSE)) {
					iadsStatus = InvUtils::iadsReleased;
				}
				else if(VarBool(pRow->GetValue(alcOrder), FALSE)) {
					//TES 7/18/2008 - PLID 29478 - New status
					iadsStatus = InvUtils::iadsOrder;
				}

				//TES 6/20/2008 - PLID 26152 - Only check the value of the Return column if this is a Released, serialized,
				// Purchased Inventory item.
				BOOL bUpdatedToBeReturned = FALSE;
				BOOL bToBeReturned = FALSE;
				if(iadsStatus == InvUtils::iadsReleased &&
					(InvUtils::ProductItemStatus)VarLong(pRow->GetValue(alcProductItemStatus), -1) == InvUtils::pisPurchased &&
					nProductItemID != -1) {
					bUpdatedToBeReturned = TRUE;
					bToBeReturned = VarBool(pRow->GetValue(alcReturn));
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = %i WHERE ID = %li",
						bToBeReturned, nProductItemID);
				}
				else if(nProductItemID != -1) {
					//TES 6/20/2008 - PLID 26152 - Ensure the flag is turned off
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET ToBeReturned = 0 WHERE ID = %li",
						nProductItemID);
				}

				//grab the notes
				CString strDetailNotes = VarString(pRow->GetValue(alcNotes), "");
				strDetailNotes.TrimLeft();
				strDetailNotes.TrimRight();

				if(nDetailID != -1) {
					//it's an existing detail, so update, that is if something changed

					double dblOldQuantity = VarDouble(pRow->GetValue(alcOldQuantity), 1.0);
					InvUtils::InventoryAllocationDetailStatus iadsOldStatus = (InvUtils::InventoryAllocationDetailStatus)VarLong(pRow->GetValue(alcOldStatus), (long)InvUtils::iadsActive);
					//TES 6/20/2008 - PLID 26152 - Get the old value for auditing
					BOOL bOldToBeReturned = VarBool(pRow->GetValue(alcOldToBeReturned), FALSE);
					
					CString strOldDetailNotes = VarString(pRow->GetValue(alcOldNotes), "");
					strOldDetailNotes.TrimLeft();
					strOldDetailNotes.TrimRight();

					//TES 7/18/2008 - PLID 29478 - We also need to set the ProductItemID, it can be changed if they change
					// the "To Be Ordered" status.
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT "
						"SET Quantity = %g, Status = %li, Notes = '%s', ProductItemID = %s WHERE ID = %li", 
						dblQuantity, iadsStatus, _Q(strDetailNotes), strProductItemID, nDetailID);

					// (j.jones 2007-11-16 10:52) - PLID 28043 - audit the changes						

					//audit the quantity change
					if(dblOldQuantity != dblQuantity) {
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						CString strOldValue, strNewValue;
						strOldValue.Format("Product: '%s', Quantity: %g", strProductName, dblOldQuantity);
						strNewValue.Format("Quantity: %g", dblQuantity);
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailQuantity, m_nID, strOldValue, strNewValue, aepMedium, aetChanged);
					}

					CString strDesc = GenerateAuditDescFromAllocationRow(pRow);

					//audit the status change
					//TES 6/20/2008 - PLID 26152 - The status also changed if the "To Be Returned" flag changed
					if(iadsStatus != iadsOldStatus ||
						(bUpdatedToBeReturned && bOldToBeReturned != bToBeReturned) ) {
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}

						CString strOldValue, strNewValue;
						AuditEventItems aeiItem = aeiInvAllocationDetailActive;
						
						strOldValue = strDesc;
						if(iadsOldStatus == InvUtils::iadsActive) {
							strOldValue += ", Status: Active";
						}
						else if(iadsOldStatus == InvUtils::iadsUsed) {
							strOldValue += ", Status: Used";
						}
						else if(iadsOldStatus == InvUtils::iadsReleased) {
							//TES 6/20/2008 - PLID 26152 - If it used to have the To Be Returned flag, include it in our auditing
							if(bOldToBeReturned) {
								strOldValue += ", Status: Released - To Be Returned";
							}
							else {
								strOldValue += ", Status: Released";
							}
						}
						else if(iadsOldStatus == InvUtils::iadsOrder) {
							//TES 7/18/2008 - PLID 29478 - New status
							strOldValue += ", Status: To Be Ordered";
						}
						else {
							//should be impossible
							ASSERT(FALSE);
						}

						if(iadsStatus == InvUtils::iadsActive) {
							strNewValue = "Status: Active";
							aeiItem = aeiInvAllocationDetailActive;
						}
						else if(iadsStatus == InvUtils::iadsUsed) {
							strNewValue = "Status: Used";
							aeiItem = aeiInvAllocationDetailUsed;
						}
						else if(iadsStatus == InvUtils::iadsReleased) {
							//TES 6/20/2008 - PLID 26152 - If we updated the To Be Returned flag, include it in our auditing.
							if(bUpdatedToBeReturned && VarBool(pRow->GetValue(alcReturn))) {
								strNewValue = "Status: Released - To Be Returned";
							}
							else {
								strNewValue = "Status: Released";
							}
							aeiItem = aeiInvAllocationDetailReleased;
						}
						else if(iadsStatus == InvUtils::iadsOrder) {
							//TES 7/18/2008 - PLID 29478 - New status
							strNewValue = "Status: To Be Ordered";
							aeiItem = aeiInvAllocationDetailToBeOrdered;
						}
						else {
							//should be impossible
							ASSERT(FALSE);
						}
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiItem, m_nID, strOldValue, strNewValue, aepMedium, aetChanged);
					}

					//audit the notes change
					if(strDetailNotes != strOldDetailNotes) {

						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}

						CString strOldValue;
						strOldValue = strDesc + ", Notes: " + strOldDetailNotes;
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailNotes, m_nID, strOldValue, strDetailNotes, aepMedium, aetChanged);
					}
				}
				else {
					//create a new detail
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationDetailsT "
						"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes) "
						"VALUES (%li, %li, %s, %g, %li, '%s')", m_nID, nServiceID,
						strProductItemID, dblQuantity, iadsStatus, _Q(strDetailNotes));

					// (j.jones 2007-11-16 10:48) - PLID 28043 - audit the creation of each new detail
					{
						CString strDesc = GenerateAuditDescFromAllocationRow(pRow);

						//audit the new detail
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailCreated, m_nID, "", strDesc, aepMedium, aetCreated);

						if(iadsStatus != InvUtils::iadsActive) {
							//if we just created a new detail, and it is not active,
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
								//TES 6/20/2008 - PLID 26152 - If we updated the To Be Returned flag, include that in our auditing.
								if(bUpdatedToBeReturned && VarBool(pRow->GetValue(alcReturn))) {
									strNewValue = "Status: Released - To Be Returned";
								}
								else {
									strNewValue = "Status: Released";
								}
								aeiItem = aeiInvAllocationDetailReleased;
							}
							else if(iadsStatus == InvUtils::iadsOrder) {
								//TES 7/18/2008 - PLID 29478 - New status
								strNewValue = "Status: To Be Ordered";
								aeiItem = aeiInvAllocationDetailToBeOrdered;
							}
							else {
								//should be impossible
								ASSERT(FALSE);
							}
							AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiItem, m_nID, strOldValue, strNewValue, aepMedium, aetChanged);
						}
					}
				}

				pRow = pRow->GetNextRow();
			}

			// (j.jones 2008-02-27 12:07) - PLID 29102 - update the case history link
			long nCaseHistoryID = -1;
			CString strCaseHistoryName = "";
			{
				IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->GetCurSel();
				if(pCaseRow) {
					nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
					strCaseHistoryName = VarString(pCaseRow->GetValue(chccName), "");
				}

				//did we change the linking?
				if(nCaseHistoryID != m_nOrigCaseHistoryID) {

					//if we previously had one, we have to remove that link
					if(m_nOrigCaseHistoryID != -1) {
						AddStatementToSqlBatch(strSqlBatch, 
							"DELETE FROM CaseHistoryAllocationLinkT WHERE AllocationID = %li AND CaseHistoryID = %li",
							m_nID, m_nOrigCaseHistoryID);
					}
					
					//if we have a new one, we have to add that link
					if(nCaseHistoryID != -1) {
						AddStatementToSqlBatch(strSqlBatch, 
							"INSERT INTO CaseHistoryAllocationLinkT (AllocationID, CaseHistoryID) "
							"VALUES (%li, %li) ", m_nID, nCaseHistoryID);
					}

					// (j.jones 2008-02-27 12:30) - PLID 29104 - audit the case history link
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strAllocationDescription;
					strAllocationDescription.Format("Allocation From %s", FormatDateTimeForInterface(m_dtInputDate, NULL, dtoDate));

					//need to audit exactly how it changed, both on the allocation and for the case history
					if(m_nOrigCaseHistoryID == -1) {
						//we made a new link
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkCreated, m_nID, strAllocationDescription + ", No Linked Case History", "Case History: " + strCaseHistoryName, aepMedium, aetCreated);
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkCreated, nCaseHistoryID, strCaseHistoryName + ", No Linked Allocation", strAllocationDescription, aepMedium, aetCreated);
					}
					else if(nCaseHistoryID == -1) {
						//we removed the link
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkDeleted, m_nID, strAllocationDescription + ", Case History: " + m_strOrigCaseHistoryName, "<No Linked Case History>", aepMedium, aetDeleted);
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nCaseHistoryID, m_strOrigCaseHistoryName + ", " + strAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
					}
					else {
						//we changed the link
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationCaseHistoryLinkChanged, m_nID, strAllocationDescription + ", Case History: " + m_strOrigCaseHistoryName, "Case History: " + strCaseHistoryName, aepMedium, aetChanged);
						
						//that means we removed from one allocation and linked another
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkDeleted, nCaseHistoryID, m_strOrigCaseHistoryName + ", " + strAllocationDescription, "<No Linked Allocation>", aepMedium, aetDeleted);
						AuditEvent(m_nCurPatientID, strPatientName, nAuditTransactionID, aeiCaseHistoryInvAllocationLinkCreated, nCaseHistoryID, strCaseHistoryName + ", No Linked Allocation", strAllocationDescription, aepMedium, aetCreated);
					}
				}
			}

			ExecuteSqlBatch(strSqlBatch);
		
			// (j.jones 2007-11-16 12:21) - PLID 28043 - supported auditing
			if(nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
				nAuditTransactionID = -1;
			}

			// (c.haag 2008-02-29 12:25) - PLID 29115 - Update inventory todo alarms
			nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
			InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_AllocationID, m_nID);
			//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
			InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);

			//now clear the deleted details list
			m_aryDeletedDetails.RemoveAll();
			// (j.jones 2007-11-16 11:09) - PLID 28043 - and their audit descriptions
			m_mapDeletedDetailDesc.RemoveAll();

			// (j.jones 2008-03-24 16:49) - PLID 29388 - need to update the linked appointment
			if(nAppointmentID != -1) {
				TrySendAppointmentTablecheckerForInventory(nAppointmentID, TRUE);
			}
			//and the old appointment as well
			if(m_nOrigAppointmentID != -1 && m_nOrigAppointmentID != nAppointmentID) {
				TrySendAppointmentTablecheckerForInventory(m_nOrigAppointmentID, TRUE);
			}
		}	

		return TRUE;

	}NxCatchAllCall("Error in CInvPatientAllocationDlg::Save",
		// (j.jones 2007-11-16 12:21) - PLID 28043 - supported auditing
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		// (c.haag 2008-02-29 12:23) - PLID 29115 - Roll back any inventory todo transaction
		if(nInvTodoTransactionID != -1) {
			InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID);
		}
	);

	return FALSE;
}

void CInvPatientAllocationDlg::OnOK() 
{
	try {

		// (j.jones 2007-11-27 10:50) - PLID 28196 - converted saving into its own function
		if(!Save()) {
			return;
		}

		//if we succeeded, we can close the dialog
		CDialog::OnOK();

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnOK");	
}

BEGIN_EVENTSINK_MAP(CInvPatientAllocationDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvPatientAllocationDlg)
	ON_EVENT(CInvPatientAllocationDlg, IDC_PRODUCT_ALLOC_COMBO, 16 /* SelChosen */, OnSelChosenProductAllocCombo, VTS_DISPATCH)	
	ON_EVENT(CInvPatientAllocationDlg, IDC_ALLOCATION_LIST, 6 /* RButtonDown */, OnRButtonDownAllocationList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvPatientAllocationDlg, IDC_LOCATION_ALLOC_COMBO, 16 /* SelChosen */, OnSelChosenLocationAllocCombo, VTS_DISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenPatientCombo, VTS_DISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_PATIENT_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedPatientCombo, VTS_I4 VTS_I4)
	ON_EVENT(CInvPatientAllocationDlg, IDC_LOCATION_ALLOC_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedLocationAllocCombo, VTS_I4 VTS_I4)
	ON_EVENT(CInvPatientAllocationDlg, IDC_APPOINTMENT_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedAppointmentCombo, VTS_I4 VTS_I4)
	ON_EVENT(CInvPatientAllocationDlg, IDC_ALLOCATION_LIST, 9 /* EditingFinishing */, OnEditingFinishingAllocationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvPatientAllocationDlg, IDC_ALLOCATION_LIST, 8 /* EditingStarting */, OnEditingStartingAllocationList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInvPatientAllocationDlg, IDC_ALLOCATION_LIST, 10 /* EditingFinished */, OnEditingFinishedAllocationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvPatientAllocationDlg, IDC_APPOINTMENT_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedAppointmentCombo, VTS_I2)
	ON_EVENT(CInvPatientAllocationDlg, IDC_SURGERY_ALLOC_COMBO, 16 /* SelChosen */, OnSelChosenSurgeryAllocCombo, VTS_DISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_QUOTE_ALLOC_COMBO, 16 /* SelChosen */, OnSelChosenQuoteAllocCombo, VTS_DISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_ALLOC_ADD_COMBO, 16 /* SelChosen */, OnSelChosenAllocAddCombo, VTS_DISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_ALLOCATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedAllocationList, VTS_I2)
	ON_EVENT(CInvPatientAllocationDlg, IDC_CASE_HISTORY_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCaseHistoryCombo, VTS_I2)
	ON_EVENT(CInvPatientAllocationDlg, IDC_LOCATION_ALLOC_COMBO, 1 /* SelChanging */, OnSelChangingLocationAllocCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_CASE_HISTORY_COMBO, 1 /* SelChanging */, OnSelChangingCaseHistoryCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_PROVIDER_ALLOC_COMBO, 16 /* SelChosen */, OnSelChosenProviderAllocCombo, VTS_DISPATCH)
	ON_EVENT(CInvPatientAllocationDlg, IDC_PROVIDER_ALLOC_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedProviderAllocCombo, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvPatientAllocationDlg::OnSelChosenProductAllocCombo(LPDISPATCH lpRow) 
{
	try {

		// (j.jones 2007-11-07 17:54) - PLID 27987 - I decided there's no reason
		// why a patient has to exist prior to adding products. You just won't
		// be able to save the allocation until a patient is selected.

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//if no product selected, leave this function
			return;
		}

		long nProductID = VarLong(pRow->GetValue(pccID));
		CString strProductName = VarString(pRow->GetValue(pccName), "");

		//now check and see if the selected product is a serialized item
		BOOL bHasSerialNum = VarBool(pRow->GetValue(pccHasSerialNum), FALSE);
		BOOL bHasExpDate = VarBool(pRow->GetValue(pccHasExpDate), FALSE);
		BOOL bSerialNumIsLotNum = VarBool(pRow->GetValue(pccSerialNumIsLotNum), FALSE);

		//try to add this product to the list
		//TES 7/3/2008 - PLID 24726 - Pass in bSerialNumIsLotNum
		TryAddProductToList(nProductID, strProductName, 1.0, bHasSerialNum, bHasExpDate, bSerialNumIsLotNum);

		//now clear the product selection
		m_ProductCombo->PutCurSel(NULL);

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenProductAllocCombo");
}

void CInvPatientAllocationDlg::OnRButtonDownAllocationList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		//create a right-click menu with the option to delete a product from the allocation

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		m_AllocationList->PutCurSel(pRow);

		enum AllocationListMenuItems
		{
			almiDelete = 1,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, almiDelete, "Remove Product");

		CPoint pt;
		GetCursorPos(&pt);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

		switch(nResult)
		{
			case almiDelete: {
				//delete the selected product from the allocation
				if(IDYES == MessageBox("Are you sure you wish to delete this product from the allocation?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

					//track the ID as deleted, if it was a saved product
					long nID = VarLong(pRow->GetValue(alcID), -1);
					if(nID != -1) {
						m_aryDeletedDetails.Add(nID);
						// (j.jones 2007-11-16 11:15) - PLID 28043 - track the description
						CString strDesc = GenerateAuditDescFromAllocationRow(pRow);
						m_mapDeletedDetailDesc.SetAt(nID, strDesc);
					}
					//and now remove the row
					m_AllocationList->RemoveRow(pRow);

					// (j.jones 2007-11-28 16:45) - PLID 28196 - enable the "complete" button accordingly
					CheckEnableCompleteAllocationBtn();
				}
				break;
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnRButtonDownAllocationList");
}

void CInvPatientAllocationDlg::OnSelChosenLocationAllocCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		//get the newly selected locaton ID
		long nNewLocationID = -1;
		if(pRow != NULL) {
			nNewLocationID = VarLong(pRow->GetValue(lccID));
		}
		else {
			AfxMessageBox("You must select a location from the list.");
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_LocationCombo->TrySetSelByColumn_Deprecated(lccID, m_nCurLocationID);
			return;
		}

		//if they selected the same location, do nothing
		if(nNewLocationID == m_nCurLocationID) {
			return;
		}

		// (j.jones 2008-02-29 10:20) - PLID 29102 - see if we have a case history selected, as it will be losing that selection
		if(IsSurgeryCenter(FALSE)) {
			long nCaseHistoryID = -1;
			IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->GetCurSel();
			if(pCaseRow) {
				nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
			}
			if(nCaseHistoryID != -1) {
				//yes, they will get two warnings if they have a linked case and products
				//(2nd warning is further down)
				if(IDNO == MessageBox("The currently linked case history will be unlinked if you switch locations.\n"
					"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					
					//reset the selection
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					m_LocationCombo->TrySetSelByColumn_Deprecated(lccID, m_nCurLocationID);
					return;
				}
			}
		}

		//we need to stop them from changing locations if we have products in our list
		if(m_AllocationList->GetRowCount() > 0) {

			//because serial numbers are per location, and products may not be useable at all locations,
			//simply disallow switching locations while retaining the current product list

			//warn them twice, to give a strong warning in this scenario
			if(IDNO == MessageBox("Because products are tied to a location, you may not switch locations without clearing the currently allocated products from the list.\n"
				"Do you wish to continue with the new location?\n\n"
				"Clicking 'Yes' will clear the current products from the allocation list and switch locations.\n"
				"Clicking 'No' will revert to the previously selected location.", "Practice", MB_ICONEXCLAMATION|MB_YESNO)
				|| IDNO == MessageBox("Are you absolutely sure you wish to clear the current product allocation list?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

				//they don't want to switch locations after all, so reset the selection
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_LocationCombo->TrySetSelByColumn_Deprecated(lccID, m_nCurLocationID);
				return;
			}

			//clear the list, while tracking any saved items as deleted

			IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
			while(pRow != NULL) {
				
				long nID = VarLong(pRow->GetValue(alcID), -1);
				if(nID != -1) {
					m_aryDeletedDetails.Add(nID);
					// (j.jones 2007-11-16 11:15) - PLID 28043 - track the description
					CString strDesc = GenerateAuditDescFromAllocationRow(pRow);
					m_mapDeletedDetailDesc.SetAt(nID, strDesc);
				}

				pRow = pRow->GetNextRow();
			}
			//now clear the list
			m_AllocationList->Clear();

			// (j.jones 2007-11-28 16:45) - PLID 28196 - disable the "complete" button
			GetDlgItem(IDC_BTN_COMPLETE_ALLOCATION)->EnableWindow(FALSE);
		}

		//if we made it here, the user was allowed to change selections, so
		//assign the new location ID to the member variable
		m_nCurLocationID = nNewLocationID;

		// (j.jones 2008-01-07 14:41) - PLID 28479 - we no longer require the product to be billable
		// to be used in an allocation - which means we don't need this function
		//ReflectProductsByLocationID();

		// (j.jones 2008-02-28 17:45) - PLID 29102 - requery the case history list
		if(IsSurgeryCenter(FALSE)) {

			CString strWhere;
			strWhere.Format("PersonID = %li "
				"AND ID NOT IN (SELECT CaseHistoryID FROM CaseHistoryAllocationLinkT WHERE AllocationID <> %li) "
				"AND LocationID = %li", m_nCurPatientID, m_nID, m_nCurLocationID);
			m_CaseHistoryCombo->PutWhereClause(_bstr_t(strWhere));
			m_CaseHistoryCombo->Requery();
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenLocationAllocCombo");
}

// (j.jones 2008-01-07 14:41) - PLID 28479 - we no longer require the product to be billable
// to be used in an allocation - which means we don't need this function

/*
//ReflectProductsByLocationID will requery the product list for those billable by a certain location
void CInvPatientAllocationDlg::ReflectProductsByLocationID()
{
	try {

		if(m_nCurLocationID == -1) {
			//this should not be called with a -1 LocationID
			ASSERT(FALSE);
			m_ProductCombo->Clear();
			return;
		}

		// (j.jones 2008-01-07 14:41) - PLID 28479 - we no longer require the product to be billable
		// to be used in an allocation - which means we don't need to requery any of these lists

		CString strWhere;
		strWhere.Format("ProductLocationInfoT.LocationID = %li ServiceT.Active = 1", m_nCurLocationID);
		m_ProductCombo->PutWhereClause(_bstr_t(strWhere));
		m_ProductCombo->Requery();

		//and surgeries - show any surgery that has at least one product
		strWhere.Format("ID IN (SELECT SurgeryID FROM SurgeryDetailsT "
			"WHERE PayToPractice = 1 AND Billable = 1 "
			"AND ServiceID IN (SELECT ProductID FROM ProductLocationInfoT WHERE LocationID = %li) "
			"AND ServiceID IN (SELECT ID FROM ServiceT WHERE Active = 1))", m_nCurLocationID);
		m_SurgeryCombo->PutWhereClause(_bstr_t(strWhere));
		//don't requery unless we've selected the surgeries before
		if(m_bSurgeriesRequeried) {
			m_SurgeryCombo->Requery();
		}

		//and quotes - show any quote that has at least one product
		strWhere.Format("ID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE LineItemT.Deleted = 0 "
			"AND ServiceID IN (SELECT ProductID FROM ProductLocationInfoT WHERE LocationID = %li) "
			"AND ServiceID IN (SELECT ID FROM ServiceT WHERE Active = 1))", m_nCurLocationID);
		m_QuoteCombo->PutWhereClause(_bstr_t(strWhere));
		//don't requery unless we've selected the quotes before
		if(m_bQuotesRequeried) {
			m_QuoteCombo->Requery();
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::ReflectProductsByLocationID");
}
*/

void CInvPatientAllocationDlg::OnSelChosenPatientCombo(LPDISPATCH lpRow) 
{
	try {

		//if this is an existing allocation, give a severe warning before changing patients

		IRowSettingsPtr pRow(lpRow);
		
		long nNewPatientID = -1;
		if(pRow != NULL) {
			nNewPatientID = VarLong(pRow->GetValue(patccID));
		}

		//if they selected the same patient, do nothing
		if(nNewPatientID == m_nCurPatientID) {
			return;
		}

		//we need to stop them from changing patients if this is an existing allocation
		if(m_nID != -1) {

			//warn the user before switching patients
			if(IDNO == MessageBox("If you change patients on this allocation, all currently allocated products will be moved to the new patient.\n"
				"Are you sure you wish to change the patient that these products are allocated to?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)
				|| IDNO == MessageBox("Are you absolutely SURE you wish to change the patient for this allocation?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				//they don't want to switch patients after all, so reset the selection
				// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
				m_PatientCombo->TrySetSelByColumn_Deprecated(patccID, m_nCurPatientID);
				return;
			}
		}

		//if we made it here, the user was allowed to change selections, so
		//assign the new patient ID to the member variable
		m_nCurPatientID = nNewPatientID;

		//set the quote from clause to include this patient ID
		ResetQuotesFromClause();

		if(m_bQuotesRequeried) {
			//if we have already requeried the quote combo once, we need to do it again
			m_QuoteCombo->Requery();
		}
		else {
			//see if the "existing quote" is selected, it may not be requeried yet
			//if it was selected before a patient was selected

			IRowSettingsPtr pSelRow = m_SelectionCombo->GetCurSel();
			if(pSelRow != NULL) {
				SelectionTypes seltypeSelected = (SelectionTypes)VarLong(pSelRow->GetValue(selccID), (long)seltypeProduct);
				if(seltypeSelected == seltypeQuote) {
					//it is selected, so requery accordingly
					m_QuoteCombo->Requery();
					m_bQuotesRequeried = TRUE;
				}
			}
		}

		//requery the appt. list for this patient
		if(m_nCurPatientID == -1) {
			m_AppointmentCombo->Clear();
		}
		else {
			CString strWhere;
			strWhere.Format("PatientID = %li", m_nCurPatientID);
			m_AppointmentCombo->PutWhereClause(_bstr_t(strWhere));
			m_AppointmentCombo->Requery();
		}

		// (j.jones 2008-02-27 11:20) - PLID 29102 - requery the case history list (if they have ASC)
		if(IsSurgeryCenter(FALSE)) {
			if(m_nCurPatientID == -1) {
				m_CaseHistoryCombo->Clear();
			}
			else {
				CString strWhere;
				//we will use the allocation filter even if the ID is -1
				strWhere.Format("PersonID = %li "
					"AND ID NOT IN (SELECT CaseHistoryID FROM CaseHistoryAllocationLinkT WHERE AllocationID <> %li) "
					"AND LocationID = %li", m_nCurPatientID, m_nID, m_nCurLocationID);
				m_CaseHistoryCombo->PutWhereClause(_bstr_t(strWhere));
				m_CaseHistoryCombo->Requery();
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenPatientCombo");
}

//this function will take in a product ID and quantity and request product items, etc.,
//adding to the list if successful
//TES 7/3/2008 - PLID 24726 - Added bSerialNumIsLotNum parameter
void CInvPatientAllocationDlg::TryAddProductToList(long nProductID, CString strProductName, double dblDesiredQuantity, BOOL bHasSerialNum, BOOL bHasExpDate, BOOL bSerialNumIsLotNum)
{
	try {

		//but first check that there is inventory on hand for this product

		// (j.jones 2008-03-20 10:22) - PLID 29311 - put the out-of-stock check in its own function
		//TES 7/18/2008 - PLID 29478 - The user can choose to add the item as "To Be Ordered" if it's out of stock.
		BOOL bAddOnOrder = FALSE;
		CheckWarnLowStock(nProductID, strProductName, dblDesiredQuantity, &bAddOnOrder);

		if(!bAddOnOrder && (bHasSerialNum || bHasExpDate)) {
			//if serialized, open the product items dialog

			//GetExistingProductItemWhereClause finds all the ProductItemsT records in the
			//current allocation that match the passed-in ServiceID,
			//and returns a string of "ProductItemsT.ID NOT IN (1, 2, 3....)" etc.
			CString strExistingProductItemWhereClause = GetExistingProductItemWhereClause(nProductID);
			CString strWhere = strExistingProductItemWhereClause;
			if(strWhere.GetLength() > 0)
				strWhere += " AND ";

			//only open the dialog if there are product items available, including those not in
			//any allocations, even if they may have been deleted and not yet saved in this allocation
			if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE %s ProductID = %li "
				"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"			    WHERE (Status = %li OR Status = %li) "
				"				AND ProductItemID Is Not Null) "
				"AND Deleted = 0 "
				"AND (ProductItemsT.LocationID = %li OR ProductItemsT.LocationID Is Null)",
				strWhere, nProductID, InvUtils::iadsActive, InvUtils::iadsUsed, m_nCurLocationID)) {

				BOOL bCancel = FALSE;
				BOOL bLoop = TRUE;

				//while we still need to prompt
				while(bLoop) {

					//prompt
					CProductItemsDlg dlg(this);
					dlg.m_EntryType = PI_SELECT_DATA;
					dlg.m_strOverrideTitleBarText = "Select Items to be Allocated";
					dlg.m_strOverrideSelectQtyText = "Quantity to be allocated:";
					dlg.m_ProductID = nProductID;
					dlg.m_nLocationID = m_nCurLocationID;
					dlg.m_CountOfItemsNeeded = (long)dblDesiredQuantity;
					dlg.m_strWhere = strExistingProductItemWhereClause;
					dlg.m_bAllowQtyGrow = TRUE;

					//disable barcode messages on this screen while the product items dialog is up
					m_bDisableBarcode = TRUE;

					if(IDCANCEL == dlg.DoModal()) {
						//if they cancelled, warn them!
						CString strWarn;
						//TES 7/18/2008 - PLID 29478 - Give them the option to add the item as "To Be Ordered"
						strWarn.Format("You have chosen to cancel adding '%s'."
							"\nThe item will not be added to the list if you do not fill in the requested information."
							"\nWould you like to add the item with a 'To Be Ordered' status?"
							"\nIf not, the product will not be added.", strProductName);
						if(IDYES != MessageBox(strWarn,"Practice",MB_ICONQUESTION|MB_YESNO)) {
							//if they wish to not allocate the item, stop looping, cancel adding this item
							bLoop = FALSE;
							bCancel = TRUE;
						}
						else {
							bLoop = FALSE;
							bAddOnOrder = TRUE;
						}
					}
					else {
						//if they selected an item, then stop looping
						bLoop = FALSE;
						
						//loop through dlg.m_adwProductItemIDs and add to the list
						for(int i=0; i<dlg.m_adwProductItemIDs.GetSize();i++) {

							_variant_t varProductItemID = (long)(dlg.m_adwProductItemIDs.GetAt(i));
							_variant_t varSerialNumber = dlg.GetSelectedProductItemSerialNum(i);
							_variant_t varExpDate = dlg.GetSelectedProductItemExpDate(i);
							// (c.haag 2008-03-11 13:52) - PLID 29255 - Acquire the status, too
							_variant_t varStatus = dlg.GetSelectedProductItemStatus(i);

							//TES 7/3/2008 - PLID 24726 - Pass in bSerialNumIsLotNum
							//TES 7/18/2008 - PLID 29478 - Also pass in bAddOnOrder, (which is FALSE), and TRUE for bIsSerialized
							AddProductToList(nProductID, strProductName, 1.0, varProductItemID, varSerialNumber, varExpDate, varStatus, bSerialNumIsLotNum, bAddOnOrder, TRUE);
						}
					}

					//re-enable barcoding
					m_bDisableBarcode = FALSE;
				}
				if(bCancel) {
					//if the loop ended with a cancellation, quit

					//and clear the product selection
					m_ProductCombo->PutCurSel(NULL);
					return;
				}
			}
			else {

				CString strLocationName = "";
				//if multiple locations exist, clarify it's the selected location that is out of stock
				if(m_LocationCombo->GetRowCount() > 1) {
					IRowSettingsPtr pLocRow = m_LocationCombo->GetCurSel();
					if(pLocRow != NULL) {
						strLocationName.Format(" at %s", VarString(pLocRow->GetValue(lccName),""));
					}
					else {
						//not likely to be possible unless the list hasn't finished requerying yet
						strLocationName = " at the current location";
					}
				}
				
				CString str;
				//TES 7/18/2008 - PLID 29478 - Give them the option to add it "To Be Ordered"
				str.Format("The product '%s' requires %s%s%s, but has no items in stock%s.\n"
					"This product cannot be allocated until there are items in stock%s."
					"\nWould you like to add this product with a 'To Be Ordered' status?",
					strProductName,
					bHasSerialNum ? "a serial number" : "",
					(bHasSerialNum && bHasExpDate) ? " and " : "",
					bHasExpDate ? "an expiration date" : "",
					strLocationName, strLocationName);
				if(IDYES == AfxMessageBox(str, MB_YESNO)) {
					bAddOnOrder = TRUE;
				}
				else {
					//clear the product selection
					m_ProductCombo->PutCurSel(NULL);
					return;
				}
			}

			if(bAddOnOrder) {
				//TES 7/18/2008 - PLID 29478 - Add the item, with NULL for all the serialized information, and TRUE for bAddToBeOrdered
				AddProductToList(nProductID, strProductName, 1.0, g_cvarNull, g_cvarNull, g_cvarNull, g_cvarNull, bSerialNumIsLotNum, TRUE, TRUE);
			}

		}
		else {
			//if not serialized, add this product to the allocation list

			//TES 7/24/2008 - PLID 29478 - Don't prompt to increment the quantity if it's serialized.
			if(!bHasSerialNum && !bHasExpDate) {
				//see if the product is in the list already, and prompt to increment the qty.
				IRowSettingsPtr pRow = m_AllocationList->FindByColumn(alcServiceID, nProductID, m_AllocationList->GetFirstRow(), FALSE);
				if(pRow) {
					//it does exist, so prompt
					CString strWarn;
					strWarn.Format("The product '%s' already exists in the current allocation. Would you like to increase its quantity by %g?\n\n"
						"'Yes' will add %g to the existing quantity.\n"
						"'No' will add the product again as a new row.\n"
						"'Cancel' will cancel adding the product again.", strProductName, dblDesiredQuantity, dblDesiredQuantity);
					int nResult = MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);

					if(nResult == IDCANCEL) {
						//clear the product selection
						m_ProductCombo->PutCurSel(NULL);
						return;
					}
					else if(nResult == IDYES) {
						//add to the quantity
						double dblQty = VarDouble(pRow->GetValue(alcQuantity), dblDesiredQuantity);
						dblQty += dblDesiredQuantity;
						pRow->PutValue(alcQuantity, dblQty);

						//clear the product selection
						m_ProductCombo->PutCurSel(NULL);
						return;
					}

					//otherwise continue on and add a new row
				}
			}

			//now add the product to the list
			// (c.haag 2008-03-11 13:12) - PLID 29255 - Pass in variant nulls for serial-related values since this is not a serializeable product
			//TES 7/3/2008 - PLID 24726 - Pass in bSerialNumIsLotNum
			//TES 7/18/2008 - PLID 29478 - Pass in bAddOnOrder, as well as the bIsSerialized parameter
			AddProductToList(nProductID, strProductName, dblDesiredQuantity, g_cvarNull, g_cvarNull, g_cvarNull, g_cvarNull, bSerialNumIsLotNum, bAddOnOrder, bHasSerialNum || bHasExpDate);
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::TryAddProductToList");
}

//this function is called only from TryAddProductToList and
//will take all the product information we need to add to the list, and do so
// (c.haag 2008-03-11 12:39) - PLID 29255 - Added a type (Purchased Inv./Consignment) field
//TES 7/3/2008 - PLID 24726 - Added bSerialNumIsLotNum parameter
//TES 7/18/2008 - PLID 29478 - Added bAddToBeOrdered and bSerialized parameters
void CInvPatientAllocationDlg::AddProductToList(long nProductID, CString strProductName, double dblQuantity,
												_variant_t varProductItemID, _variant_t varSerialNumber, _variant_t varExpDate, _variant_t varProductType, BOOL bSerialNumIsLotNum,
												BOOL bAddToBeOrdered, BOOL bSerialized)
{
	try {

		if(nProductID == -1) {
			//we should never call this function with no product ID
			ASSERT(FALSE);
			return;
		}

		//TryAddProductToList should have checked in-stock amounts,
		//offered to increase quantities on existing items, etc.,
		//so this function only needs to add the product as new
		IRowSettingsPtr pRow = m_AllocationList->GetNewRow();
		pRow->PutValue(alcID, (long)-1);
		pRow->PutValue(alcServiceID, nProductID);
		pRow->PutValue(alcProductItemID, varProductItemID);
		pRow->PutValue(alcName, _bstr_t(strProductName));
		pRow->PutValue(alcOldQuantity, g_cvarNull); //unused on new details
		pRow->PutValue(alcQuantity, dblQuantity);
		pRow->PutValue(alcSerialNumber, varSerialNumber);
		pRow->PutValue(alcExpDate, varExpDate);
		// (c.haag 2008-03-11 12:41) - PLID 29255 - Product type (Purchased Inv./Consignment)
		switch ((InvUtils::ProductItemStatus)VarLong(varProductType, -1)) {
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
		//TES 8/21/2008 - PLID 26152 - Also fill the actual status
		pRow->PutValue(alcProductItemStatus, varProductType);
		pRow->PutValue(alcOldStatus, g_cvarNull); //unused on new details
		//TES 7/18/2008 - PLID 29478 - Set the Order checkbox
		pRow->PutValue(alcOrder, bAddToBeOrdered?g_cvarTrue:g_cvarFalse);
		pRow->PutValue(alcAllocated, bAddToBeOrdered?g_cvarFalse:g_cvarTrue);
		pRow->PutValue(alcUsed, g_cvarFalse);
		pRow->PutValue(alcReleased, g_cvarFalse);
		//TES 6/20/2008 - PLID 26152 - Initialize to blank.
		pRow->PutValue(alcReturn, g_cvarNull);
		pRow->PutValue(alcBilled, g_cvarFalse);
		pRow->PutValue(alcCaseHistory, g_cvarFalse);
		pRow->PutValue(alcNotes, "");
		pRow->PutValue(alcOldNotes, "");	//unused on new details
		//TES 7/3/2008 - PLID 24726 - Set the SerialNumIsLotNum parameter
		pRow->PutValue(alcSerialNumIsLotNum, bSerialNumIsLotNum?g_cvarTrue:g_cvarFalse);
		//TES 7/18/2008 - PLID 29478 - Set the IsSerialized value
		pRow->PutValue(alcIsSerialized, bSerialized?g_cvarTrue:g_cvarFalse);
		m_AllocationList->AddRowSorted(pRow, NULL);

		// (j.jones 2007-11-28 16:45) - PLID 28196 - see if we can enable the "completion" button
		CheckEnableCompleteAllocationBtn();

	}NxCatchAll("Error in CInvPatientAllocationDlg::AddProductToList");
}

//CalculateCurrentQuantity will search the current makeup of the Allocation and
//tell you how many total products are in use, per ServiceID
double CInvPatientAllocationDlg::CalculateCurrentQuantity(long nServiceID)
{
	try {

		double dblQuantity = 0.0;

		//find each row with the matching service ID, and sum up the quantity
		IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
		while(pRow != NULL) {
			
			BOOL bReleased = VarBool(pRow->GetValue(alcReleased), FALSE);
			long nServiceIDToCheck = VarLong(pRow->GetValue(alcServiceID), -1);
			if(nServiceIDToCheck == nServiceID && !bReleased) {
				//this is the right product, so add up its quantity
				double dblRowQty = VarDouble(pRow->GetValue(alcQuantity), 1.0);
				dblQuantity += dblRowQty;
			}

			pRow = pRow->GetNextRow();
		}

		return dblQuantity;

	}NxCatchAll("Error in CInvPatientAllocationDlg::CalculateCurrentQuantity");

	return 0.0;
}

LRESULT CInvPatientAllocationDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2007-11-12 12:02) - PLID 27987 - barcoding may be disabled
		// when the product items dialog is displayed
		if(m_bDisableBarcode) {
			return 0;
		}

		//note: it is intentional that we will add a product even if the
		//"add new" dropdown says surgery or quote

		// (a.walling 2007-11-08 17:37) - PLID 27476 - Need to convert this correctly from a bstr
		//grab the incoming barcode
		_bstr_t bstr = (BSTR)lParam;
		_variant_t varBarcode(bstr);
		//varBarcode.SetString((const char*)bstr);

		//select the product, if it exists, using SetSelByColumn which will intentionally
		//wait for the list to finish requerying
		//(c.copits 2010-09-14) PLID 40317 - Allow duplicate UPC codes for FramesData certification
		//IRowSettingsPtr pRow = m_ProductCombo->SetSelByColumn(pccBarcode, varBarcode);
		IRowSettingsPtr pRow = GetBestUPCProduct(varBarcode);

		if(pRow != NULL) {
			//we found a product, so now act as though we selected it normally
			OnSelChosenProductAllocCombo(pRow);
			return 0;
		}
		else if(varBarcode.vt == VT_BSTR) {
			//no row found, so check the database to see if it is a serial number

			CString strSerialNumber = VarString(varBarcode,"");

			//might as well save a DB access, check our allocation,
			//if it's in the allocation, say so, then return
			IRowSettingsPtr pExistingRow = m_AllocationList->FindByColumn(alcSerialNumber, varBarcode, m_AllocationList->GetFirstRow(), FALSE);
			//TES 7/3/2008 - PLID 24726 - If the serial number exists, but it's actually a Lot Number, then we can proceed.
			if(pExistingRow != NULL && !VarBool(pExistingRow->GetValue(alcSerialNumIsLotNum))) {
				//it's in our allocation, say so, and get out
				//(if we're in a completed allocation and the item is released, we're still not letting them
				//add it again as a new row, they need to just change the status on the existing row)
				AfxMessageBox("The barcode you scanned is a serial number for a product already in this allocation.");
				return 0;
			}

			//let's see if it's a serial number in the system, and whether it's available or not
			//(remember, like all other product item searches, this will still consider items
			//deleted but not saved on this allocation as still being in use)
			long nProductItemID = -1;
			long nProductID = -1;			
			CString strProductName = "";
			_variant_t varExpDate = g_cvarNull;
			// (c.haag 2008-03-11 13:10) - PLID 29255 - Also fetch the consignment status of the item
			InvUtils::Barcode_CheckExistenceOfSerialNumberResultEx result;
			//TES 7/7/2008 - PLID 24726 - Pass in the product items that we've already allocated to 
			// Barcode_CheckExistenceofSerialNumber(), so it knows not to return any of them.
			CArray<long,long> arExistingProductItemIDs;
			IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
			while(pRow) {
				arExistingProductItemIDs.Add(VarLong(pRow->GetValue(alcProductItemID),-1));
				pRow = pRow->GetNextRow();
			}
			if(InvUtils::Barcode_CheckExistenceOfSerialNumber(strSerialNumber, m_nCurLocationID, FALSE, nProductItemID, nProductID, strProductName, varExpDate, &result, FALSE, FALSE, TRUE, TRUE, &arExistingProductItemIDs)) {

				//the product is available, so skip the ProductItems screen, and add it immediately

				//TES 7/3/2008 - PLID 24726 - We need to pass in bSerialNumIsLotNum.  Try and pull it from the product list.
				BOOL bSerialNumIsLotNum = FALSE;
				IRowSettingsPtr pProductRow = m_ProductCombo->FindByColumn(pccID, nProductID, NULL, FALSE);
				if(pProductRow == FALSE) {
					//TES 7/3/2008 - PLID 24726 - That's weird.  Well, we have no choice but to access data.
					bSerialNumIsLotNum = VarBool(GetTableField("ProductT", "SerialNumIsLotNum", "ID", nProductID));
				}
				else {
					bSerialNumIsLotNum = VarBool(pProductRow->GetValue(pccSerialNumIsLotNum));
				}

				//we are intentionally skipping the out of stock warning here, because they should never
				//actually be out of stock yet still have this product available,
				//TES 7/18/2008 - PLID 29478 - We don't want to add it To Be Ordered, and this is Serialized
				AddProductToList(nProductID, strProductName, 1.0, (long)nProductItemID, varBarcode, varExpDate, (long)result.pisStatus, bSerialNumIsLotNum, FALSE, TRUE);
			}
			else {
				//DRT 7/23/2008 - PLID 30811 - If no barcode was found, inform the user
				AfxMessageBox("The barcode scanned could not be matched to any serial number or product barcode.  Please ensure the barcode has been entered into the system.");
				return 0;
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnBarcodeScan");

	return 0;
}

void CInvPatientAllocationDlg::OnTrySetSelFinishedPatientCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsPatient = CreateParamRecordset("SELECT Last +  ', ' + First + ' ' + Middle AS Name FROM PersonT "
				"WHERE ID = {INT}", m_nPendingPatientID);
			if(!rsPatient->eof) {
				m_PatientCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPatient, "Name", "")));
			}
			else 
				m_PatientCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingPatientID = -1;
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnTrySetSelFinishedPatientCombo");
}

void CInvPatientAllocationDlg::OnTrySetSelFinishedLocationAllocCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT "
				"WHERE ID = {INT}", m_nPendingLocationID);
			if(!rsLoc->eof) {
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLoc, "Name", "")));
			}
			else 
				m_LocationCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingLocationID = -1;
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnTrySetSelFinishedLocationAllocCombo");
}

void CInvPatientAllocationDlg::OnTrySetSelFinishedAppointmentCombo(long nRowEnum, long nFlags) 
{
	try {

		// (j.jones 2007-11-07 16:37) - PLID 27987 - We do not support "inactive" appointments,
		// this code is only so we can track whether the trysetsel function is still running when
		// we attempt to save an existing allocation batch. If it is, then we know to continue
		// using m_nPendingAppointmentID as the appt.

		m_nPendingAppointmentID = -1;

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnTrySetSelFinishedAppointmentCombo");
}

//GetExistingProductItemWhereClause finds all the ProductItemsT records in the
//current allocation that match the passed-in ServiceID,
//and returns a string of "ProductItemsT.ID NOT IN (1, 2, 3....)" etc.
CString CInvPatientAllocationDlg::GetExistingProductItemWhereClause(long nServiceID)
{
	//returns a query of AND statements so unsaved assigned ProductItems aren't in the list

	try {

		CString strIDList = "";

		//find each row with the matching service ID, see if it has
		//a ProductItemID, and isn't "released".
		//if so, use it to build our where clause
		IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
		while(pRow != NULL) {
			
			long nServiceIDToCheck = VarLong(pRow->GetValue(alcServiceID), -1);
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

	}NxCatchAll("Error in CInvPatientAllocationDlg::GetExistingProductItemWhereClause");

	return "";
}

void CInvPatientAllocationDlg::OnEditingFinishingAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
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

		if(nCol == alcQuantity && pvarNewValue->vt == VT_R8) {

			//disallow changing the quantity to be less than 0
			if(pvarNewValue->dblVal <= 0.0) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You must have a quantity greater than zero.");
				return;
			}

			//OnEditingFinished will check amt. in stock when changing quantity
			//(editing qty is not allowed on serialized items, unless they are on order)

			// (j.jones 2009-12-31 15:34) - PLID 36080 - we will allow it if it is "to be ordered",
			// but we have to enforce whole numbers
			if((long)pvarNewValue->dblVal != pvarNewValue->dblVal) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("This product must have a whole number for the quantity as it requires a serial number / expiration date.");
				return;
			}
		}

		//if they are trying to mark an "unreleased" item as any other status,
		//disallow that if it is a serialized item has been used, or if it is
		//a standard item then give an out of stock warning

		// (j.jones 2007-12-12 11:38) - PLID 27988 - similarly, don't let them change
		//the 'used' status if the detail is in use on an undeleted charge

		// (j.jones 2008-03-03 08:36) - PLID 29125 - and don't let them change the 'used'
		// status if the detail is in use on a case history

		if((nCol == alcOrder || nCol == alcAllocated || nCol == alcUsed || nCol == alcReleased || nCol == alcReturn) && pvarNewValue->vt == VT_BOOL) {

			//TES 7/18/2008 - PLID 29478 - New status
			BOOL bWasOrder = VarBool(pRow->GetValue(alcOrder), FALSE);
			BOOL bWasReleased = VarBool(pRow->GetValue(alcReleased), FALSE);
			BOOL bWasUsed = VarBool(pRow->GetValue(alcUsed), FALSE);
			BOOL bWasReturned = VarBool(pRow->GetValue(alcReturn), FALSE);

			if(m_nID != -1) {

				if(bWasReleased) {

					if(nCol == alcOrder && VarBool(pvarNewValue)) {
						//TES 7/18/2008 - PLID 29478 - No need to check stock if they're just putting it on order.
						return;
					}

					//it shouldn't be possible for the interface to let them check
					//and already checked value, but let's check for that anyways,
					//and get out of here if so
					if(nCol == alcReleased && VarBool(pvarNewValue)) {
						return;
					}

					//TES 6/23/2008 - PLID 26152 - They can't re-flag an item as To Be Returned if it's been used (the query
					// below), however, they can de-flag it, so if they're doing that we don't need to continue.  Also,
					// change the wording on the message.
					CString strAction = "used on this allocation";
					if(nCol == alcReturn) {
						if(!VarBool(pvarNewValue)) {
							return;
						}
						else {
							strAction = "returned";
						}
					}

					//now, whatever it is we're trying to do, it's unreleasing an item,
					//so see if we can actually permit that

					long nDetailID = VarLong(pRow->GetValue(alcID), -1);
					long nProductID = VarLong(pRow->GetValue(alcServiceID));
					if(nDetailID == -1) {
						//new item, don't need to check anything
						return;
					}

					long nProductItemID = VarLong(pRow->GetValue(alcProductItemID), -1);
					if(nProductItemID != -1) {
						//if serialized, check that the item is available

						//GetExistingProductItemWhereClause finds all the ProductItemsT records in the
						//current allocation that match the passed-in ServiceID,
						//and returns a string of "ProductItemsT.ID NOT IN (1, 2, 3....)" etc.
						CString strExistingProductItemWhereClause = GetExistingProductItemWhereClause(nProductID);
						CString strWhere = strExistingProductItemWhereClause;
						if(strWhere.GetLength() > 0) {
							strWhere += " AND ";
						}

						//only open the dialog if there are product items available, including those not in
						//any allocations, even if they may have been deleted and not yet saved in this allocation
						if(!ReturnsRecords("SELECT ID FROM ProductItemsT WHERE %s ProductID = %li "
							"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
							"			    WHERE (Status = %li OR Status = %li) AND AllocationID <> %li "
							"				AND ProductItemID Is Not Null) "
							"AND Deleted = 0 "
							"AND (ProductItemsT.LocationID = %li OR ProductItemsT.LocationID Is Null) "
							"AND ID = %li",
							strWhere, nProductID, InvUtils::iadsActive, InvUtils::iadsUsed, m_nID, m_nCurLocationID, nProductItemID)) {

							//it is not available! so tell the user
							*pvarNewValue = varOldValue;
							*pbCommit = FALSE;
							AfxMessageBox("This product is no longer available to be " + strAction + ".\n"
								"It may have since been billed, or associated with another allocation.");
							return;
						}
					}
					else {

						//non-serialized, so check amt. in stock

						//use m_mapExistingQuantities and CalculateCurrentQuantity to find out
						//what was previously saved on this allocation (the map is empty if a new allocation),
						//and the current quantities on this allocation, respectively
						double dblThisQuantity = VarDouble(pRow->GetValue(alcQuantity), 1.0);
						double dblPreviousQuantity = 0.0;
						long nProductID = VarLong(pRow->GetValue(alcServiceID));
						CString strProductName = VarString(pRow->GetValue(alcName), "");
						m_mapExistingQuantities.Lookup(nProductID, dblPreviousQuantity);
						double dblCurQuantity = CalculateCurrentQuantity(nProductID);
						double dblOffset = dblPreviousQuantity - dblCurQuantity - dblThisQuantity;

						double dblQuantityOnHand = 0.0;
						double dblAllocated = 0.0;

						double dblUsedOffset = 0.0;
						double dblAllocatedOffset = 0.0;

						if(nCol == alcUsed && VarBool(pvarNewValue)) {
							dblUsedOffset = dblOffset;
						}
						else {
							dblAllocatedOffset = -dblOffset;
						}

						// (j.jones 2007-12-18 11:42) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
						if(InvUtils::CalcAmtOnHand(nProductID, m_nCurLocationID, dblQuantityOnHand, dblAllocated, dblUsedOffset, dblAllocatedOffset) && dblQuantityOnHand - dblAllocated <= 0.0) {
							//Out of stock! Give a warning but let them continue.
							//If serialized, and there truly are no items available, they will get a second warning
							//(much like the way it works in billing).

							CString strAllocated = "";
							if(dblAllocated > 0.0) {
								strAllocated.Format(", and %g allocated to patients", dblAllocated);
							}

							CString str;
							if(nCol == alcUsed && VarBool(pvarNewValue)) {
								//tweak the message accordingly to reflect a "used" item decrements immediately
								str.Format("Warning: Practice shows you are now out of stock of %s\n"
									"(after saving the allocation, there will be %g on hand%s).", strProductName, dblQuantityOnHand, strAllocated);					
							}
							else {
								//is an allocated and unused item
								str.Format("Warning: Practice shows that with this allocation, you will be out of stock of %s\n"
									"(after reserving this allocation, there will be %g on hand%s).", strProductName, dblQuantityOnHand, strAllocated);
							}
							AfxMessageBox(str);
						}

					}
				}				
				else if(bWasUsed) {

					// (j.jones 2007-12-12 11:39) - PLID 27988 - now see if it was billed

					// (j.jones 2008-03-03 08:36) - PLID 29125 - or on a case history

					//it shouldn't be possible for the interface to let them check
					//and already checked value, but let's check for that anyways,
					//and get out of here if so
					if(nCol == alcUsed && VarBool(pvarNewValue)) {
						return;
					}

					//now, whatever it is we're trying to do, it's un-using an item,
					//so see if we can actually permit that

					long nDetailID = VarLong(pRow->GetValue(alcID), -1);
					if(nDetailID == -1) {
						//new item, don't need to check anything
						return;
					}

					//ok, has this detail been billed?
					if(VarBool(pRow->GetValue(alcBilled), FALSE)) {
						//this detail was billed, we cannot mark it as unused
						*pvarNewValue = varOldValue;
						*pbCommit = FALSE;
						AfxMessageBox("This product has been billed to the patient. You cannot remove the 'Used' status from this allocation.");
						return;
					}
					
					//what about a case history?
					if(VarBool(pRow->GetValue(alcCaseHistory), FALSE)) {
						//this detail is on a case history, we cannot mark it as unused
						*pvarNewValue = varOldValue;
						*pbCommit = FALSE;
						AfxMessageBox("This product has been linked to a case history for the patient. You cannot remove the 'Used' status from this allocation.");
						return;
					}
				}
				else if(bWasOrder) {
					//TES 7/18/2008 - PLID 29478 - If we were already on order, and still are, do nothing.
					if(nCol == alcOrder && VarBool(pvarNewValue)) {
						return;
					}
					else if(nCol == alcReleased) {
						//TES 7/18/2008 - PLID 29478 - We don't allow this.
						*pvarNewValue = varOldValue;
						*pbCommit = FALSE;
						AfxMessageBox("You cannot release a product that is 'To Be Ordered.'  Please select a different status, or delete this product from the allocation.");
						return;
					}
					else {
						//TES 7/18/2008 - PLID 29478 - We need to assign a ProductItem to this detail now.
						if(VarBool(pRow->GetValue(alcIsSerialized))) {
							AfxMessageBox("Now that this product is no longer 'To Be Ordered', you must choose items to allocate.");
							//prompt
							long nProductID = VarLong(pRow->GetValue(alcServiceID));
							
							CProductItemsDlg dlg(this);
							dlg.m_EntryType = PI_SELECT_DATA;
							dlg.m_strOverrideTitleBarText = "Select Item to be Allocated";
							dlg.m_strOverrideSelectQtyText = "Quantity to be allocated:";
							dlg.m_ProductID = nProductID;
							dlg.m_nLocationID = m_nCurLocationID;

							// (j.jones 2009-12-31 15:34) - PLID 36080 - "to be ordered" status entries allow multiple quantities,
							// we will also allow them to select a different amount if they wish
							long nQuantityOrdered = (long)VarDouble(pRow->GetValue(alcQuantity));
							dlg.m_CountOfItemsNeeded = nQuantityOrdered;
							dlg.m_bDisallowQtyChange = FALSE;

							dlg.m_strWhere = GetExistingProductItemWhereClause(nProductID);							

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
							
							// (j.jones 2009-12-31 15:43) - PLID 36080 - handle multiple results,
							// including the case where they assigned less than they ordered
							for(int i=0; i<dlg.m_adwProductItemIDs.GetSize(); i++) {
								_variant_t varProductItemID = (long)(dlg.m_adwProductItemIDs.GetAt(i));
								_variant_t varSerialNumber = dlg.GetSelectedProductItemSerialNum(i);
								_variant_t varExpDate = dlg.GetSelectedProductItemExpDate(i);

								// (c.haag 2008-03-11 13:52) - PLID 29255 - Acquire the status, too
								_variant_t varStatus = dlg.GetSelectedProductItemStatus(i);

								//add as a new row
								CString strProductName = VarString(pRow->GetValue(alcName), "");
								BOOL bSerialNumIsLotNum = VarBool(pRow->GetValue(alcSerialNumIsLotNum), FALSE);
								AddProductToList(nProductID, strProductName, 1.0, varProductItemID, varSerialNumber, varExpDate, varStatus,
									bSerialNumIsLotNum, FALSE, TRUE);
							}

							//now, should this row be removed or modified?
							if(dlg.m_adwProductItemIDs.GetSize() >= nQuantityOrdered) {
								//they added all (or more) items than ordered, so the ordered row can be deleted

								//track the ID as deleted, if it was a saved product
								long nID = VarLong(pRow->GetValue(alcID), -1);
								if(nID != -1) {
									m_aryDeletedDetails.Add(nID);
									// (j.jones 2007-11-16 11:15) - PLID 28043 - track the description
									CString strDesc = GenerateAuditDescFromAllocationRow(pRow);
									m_mapDeletedDetailDesc.SetAt(nID, strDesc);
								}
								//and now remove the row (j.jones - I confirmed with Bob that this is safe in OnEditingFinishing)
								m_AllocationList->RemoveRow(pRow);

								// (j.jones 2007-11-28 16:45) - PLID 28196 - enable the "complete" button accordingly
								CheckEnableCompleteAllocationBtn();
							}
							else {
								//there is a balance, we need to update the quantity still on order
								double dblNewQuantity = (double)(nQuantityOrdered - dlg.m_adwProductItemIDs.GetSize());
								pRow->PutValue(alcQuantity, (double)dblNewQuantity);

								//reset this row so it is still on order
								pRow->PutValue(alcAllocated, g_cvarFalse);
								pRow->PutValue(alcUsed, g_cvarFalse);
								pRow->PutValue(alcReleased, g_cvarFalse);
								pRow->PutValue(alcReturn, g_cvarNull);
								pRow->PutValue(alcOrder, g_cvarTrue);

								//since we are in the OnEditingFinishing handler, we have to set the new value,
								//just updating the row is not sufficient
								if(nCol == alcAllocated || nCol == alcUsed || nCol == alcReleased) {
									*pvarNewValue = g_cvarFalse;
								}
								else if(nCol == alcReturn) {
									*pvarNewValue = g_cvarNull;
								}
								else if(nCol == alcOrder) {
									*pvarNewValue = g_cvarTrue;
								}
							}
							
							//re-enable barcoding
							m_bDisableBarcode = FALSE;
						}
					}
				}
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnEditingFinishingAllocationList");	
}

void CInvPatientAllocationDlg::OnEditingStartingAllocationList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL && nCol == alcQuantity) {

			//don't let them edit quantity on a serialized item

			// (j.jones 2009-12-31 15:34) - PLID 36080 - we will allow it if it is "to be ordered"
			if(VarBool(pRow->GetValue(alcIsSerialized),FALSE) && !VarBool(pRow->GetValue(alcOrder), FALSE)) {
				//it's a product item row that is not "to be ordered", so silently stop them from editing
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnEditingStartingAllocationList");
}

void CInvPatientAllocationDlg::OnEditingFinishedAllocationList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		
		if(nCol == alcQuantity && varNewValue.vt == VT_R8) {
			//check amt. in stock when changing quantity
			//(editing qty is not allowed on serialized items, unless they are on order)

			//use m_mapExistingQuantities and CalculateCurrentQuantity to find out
			//what was previously saved on this allocation (the map is empty if a new allocation),
			//and the current quantities on this allocation, respectively
			double dblPreviousQuantity = 0.0;
			long nProductID = VarLong(pRow->GetValue(alcServiceID));
			CString strProductName = VarString(pRow->GetValue(alcName), "");
			m_mapExistingQuantities.Lookup(nProductID, dblPreviousQuantity);
			double dblCurQuantity = CalculateCurrentQuantity(nProductID);
			double dblOffset = dblPreviousQuantity - dblCurQuantity;

			//we will give context-sensitive out-of-stock warnings based on status
			BOOL bAllocated = VarBool(pRow->GetValue(alcAllocated), FALSE);
			BOOL bUsed = VarBool(pRow->GetValue(alcUsed), FALSE);
			BOOL bReleased = VarBool(pRow->GetValue(alcReleased), FALSE);

			double dblQuantityOnHand = 0.0;
			double dblAllocated = 0.0;

			double dblUsedOffset = 0.0;
			double dblAllocatedOffset = 0.0;

			if(bUsed) {
				dblUsedOffset = dblOffset;
			}
			else if(bAllocated) {
				dblAllocatedOffset = -dblOffset;
			}

			//don't even bother checking the stock if the item is released
			// (j.jones 2007-12-18 11:42) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
			if(!bReleased && InvUtils::CalcAmtOnHand(nProductID, m_nCurLocationID, dblQuantityOnHand, dblAllocated, dblUsedOffset, dblAllocatedOffset) && dblQuantityOnHand - dblAllocated <= 0.0) {
				//Out of stock! Give a warning but let them continue.
				//If serialized, and there truly are no items available, they will get a second warning
				//(much like the way it works in billing).

				CString strAllocated = "";
				if(dblAllocated > 0.0) {
					strAllocated.Format(", and %g allocated to patients", dblAllocated);
				}

				CString str;
				if(bUsed) {
					//tweak the message accordingly to reflect a "used" item decrements immediately
					str.Format("Warning: Practice shows you are now out of stock of %s\n"
						"(after saving the allocation, there will be %g on hand%s).", strProductName, dblQuantityOnHand, strAllocated);					
				}
				else {
					//is an allocated and unused item
					str.Format("Warning: Practice shows that with this allocation, you will be out of stock of %s\n"
						"(after reserving this allocation, there will be %g on hand%s).", strProductName, dblQuantityOnHand, strAllocated);
				}
				AfxMessageBox(str);
			}
		}

		//toggle between allocated / used / released so they behave like radio buttons
		//TES 7/18/2008 - PLID 29478 - Added an Order column that also behaves like a radio button.
		if((nCol == alcOrder || nCol == alcAllocated || nCol == alcUsed || nCol == alcReleased) && varNewValue.vt == VT_BOOL) {
			
			//toggle the checkboxes like radio buttons
			if(VarBool(varNewValue)) {
				if(nCol == alcAllocated) {
					//they checked the Allocated column, so force the
					//Used and Released columns to be unchecked
					pRow->PutValue(alcUsed, g_cvarFalse);
					pRow->PutValue(alcReleased, g_cvarFalse);
					//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
					pRow->PutValue(alcReturn, g_cvarNull);
					pRow->PutValue(alcOrder, g_cvarFalse);
				}
				else if(nCol == alcUsed) {
					//they checked the Used column, so force the
					//Allocated and Released columns to be unchecked
					pRow->PutValue(alcAllocated, g_cvarFalse);
					pRow->PutValue(alcReleased, g_cvarFalse);
					//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
					pRow->PutValue(alcReturn, g_cvarNull);
					pRow->PutValue(alcOrder, g_cvarFalse);
				}
				else if(nCol == alcReleased) {
					//they checked the Released column, so force the
					//Allocated and Used columns to be unchecked
					pRow->PutValue(alcAllocated, g_cvarFalse);
					pRow->PutValue(alcUsed, g_cvarFalse);
					pRow->PutValue(alcOrder, g_cvarFalse);

					//TES 6/20/2008 - PLID 26152 - If we're releasing a serialized, purchased inventory item, they can flag
					// it to be returned, so check their preference, otherwise make the column blank.
					if(VarLong(pRow->GetValue(alcProductItemID),-1) != -1 && 
						(InvUtils::ProductItemStatus)VarLong(pRow->GetValue(alcProductItemStatus), -1) == InvUtils::pisPurchased) {
						pRow->PutValue(alcReturn, GetRemotePropertyInt("ReturnReleasedGeneralSerializedItems", 1, 0, "<None>", true) == TRUE);
					}
					else {
						pRow->PutValue(alcReturn, g_cvarNull);
					}
				}
				else if(nCol == alcOrder) {
					pRow->PutValue(alcAllocated, g_cvarFalse);
					pRow->PutValue(alcUsed, g_cvarFalse);
					pRow->PutValue(alcReleased, g_cvarFalse);
					pRow->PutValue(alcReturn, g_cvarNull);

					pRow->PutValue(alcProductItemID, g_cvarNull);
					pRow->PutValue(alcSerialNumber, g_cvarNull);
					pRow->PutValue(alcExpDate, g_cvarNull);
				}
			}
			else if(nCol == alcReleased) {
				//TES 6/20/2008 - PLID 26152 - If it's not released, it can't be returned.
				pRow->PutValue(alcReturn, g_cvarNull);
			}

			// (j.jones 2007-11-28 16:45) - PLID 28196 - enable the "complete" button accordingly
			CheckEnableCompleteAllocationBtn();
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnEditingFinishedAllocationList");
}

void CInvPatientAllocationDlg::OnRequeryFinishedAppointmentCombo(short nFlags) 
{
	try {

		//add a "no appointment" row - can only be done when the requery is done
		//because of the way we sort this combo
		IRowSettingsPtr pRow = m_AppointmentCombo->GetNewRow();
		pRow->PutValue(accID, (long)-1);
		pRow->PutValue(accDateTime, g_cvarNull);
		pRow->PutValue(accTypePurpose, _bstr_t("<No Appointment Selected>"));
		pRow->PutValue(accResource, g_cvarNull);
		pRow->PutValue(accDateOnly, g_cvarNull);
		m_AppointmentCombo->AddRowBefore(pRow, m_AppointmentCombo->GetFirstRow());

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnRequeryFinishedAppointmentCombo");
}

void CInvPatientAllocationDlg::OnSelChosenSurgeryAllocCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//if no surgery selected, leave this function
			return;
		}

		long nSurgeryID = VarLong(pRow->GetValue(sccID));
		CString strSurgeryName = VarString(pRow->GetValue(sccSurgeryName));

		// (j.jones 2008-01-07 14:43) - PLID 28479 - we no longer require products to be billable

		//find all the products in the surgery,
		//and get their names and serialized statii,
		//then call TryAddProductToList

		ASSERT(m_nCurLocationID != -1); //shouldn't be possible, but let's confirm that

		//TES 7/3/2008 - PLID 24726 - Added SerialNumIsLotNum
		_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceID, Name, Quantity, HasSerialNum, HasExpDate, SerialNumIsLotNum "
			//"Convert(bit, CASE WHEN ProductT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE Billable = 1 AND LocationID = {INT}) THEN 1 ELSE 0 END) AS BillableForLocation "
			"FROM SurgeryDetailsT "
			"INNER JOIN ProductT ON SurgeryDetailsT.ServiceID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE PayToPractice = 1 AND ServiceT.Active = 1 AND SurgeryID = {INT}", nSurgeryID);

		if(rs->eof) {
			//no active products in this surgery that are pay to practice, so tell the user

			//if we can't bill it, why is it in the list at all?
			ASSERT(FALSE);

			CString strWarn;
			strWarn.Format("The surgery '%s' does not contain any products that are marked pay to practice.\n\n"
				"No products will be added to this allocation.", strSurgeryName);
			AfxMessageBox(strWarn);

			//and clear the selection
			m_SurgeryCombo->PutCurSel(NULL);
			return;
		}
		
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ServiceID");
			CString strProductName = AdoFldString(rs, "Name", "");
			double dblQuantity = AdoFldDouble(rs, "Quantity", 1.0);
			BOOL bHasSerialNum = AdoFldBool(rs, "HasSerialNum", FALSE);
			BOOL bHasExpDate = AdoFldBool(rs, "HasExpDate", FALSE);
			BOOL bSerialNumIsLotNum = AdoFldBool(rs, "SerialNumIsLotNum", FALSE);

			// (j.jones 2008-01-07 14:44) - PLID 28479 - we removed this column from the recordset
			/*
			//before we try to add the product, we have to confirm it's billable for the location,
			//since the surgery may contain products that are billable and those that are not
			BOOL bBillableForLoc = AdoFldBool(rs, "BillableForLocation", FALSE);
			if(!bBillableForLoc) {
				//the product is not billable at this location
				CString strWarn;
				strWarn.Format("The product '%s' is not billable for your current location, and will be skipped.", strProductName);
				AfxMessageBox(strWarn);
				
				rs->MoveNext();
				continue;
			}
			*/

			//try to add this product to the list
			//TES 7/3/2008 - PLID 24726 - Pass in bSerialNumIsLotNum
			TryAddProductToList(nProductID, strProductName, dblQuantity, bHasSerialNum, bHasExpDate, bSerialNumIsLotNum);

			rs->MoveNext();
		}
		rs->Close();

		//now clear the surgery selection
		m_SurgeryCombo->PutCurSel(NULL);

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenSurgeryAllocCombo");
}

void CInvPatientAllocationDlg::OnSelChosenQuoteAllocCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//if no quote selected, leave this function
			return;
		}

		long nQuoteID = VarLong(pRow->GetValue(qccID));

		// (j.jones 2008-01-07 14:43) - PLID 28479 - we no longer require products to be billable

		//find all the products in the quote,
		//and get their names and serialized statii,
		//then call TryAddProductToList

		ASSERT(m_nCurLocationID != -1); //shouldn't be possible, but let's confirm that

		//TES 7/3/2008 - PLID 24726 - Added SerialNumIsLotNum
		_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceID, Name, Quantity, HasSerialNum, HasExpDate, SerialNumIsLotNum "
			//"Convert(bit, CASE WHEN ProductT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE Billable = 1 AND LocationID = {INT}) THEN 1 ELSE 0 END) AS BillableForLocation "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE LineItemT.Deleted = 0 AND ServiceT.Active = 1 AND BillID = {INT}", nQuoteID);

		if(rs->eof) {
			//no active products in this quote that are billable and pay to practice, so tell the user

			//if we can't bill it, why is it in the list at all?
			ASSERT(FALSE);

			CString strWarn;
			strWarn.Format("This quote does not contain any active products. No products will be added to this allocation.");
			AfxMessageBox(strWarn);

			//and clear the selection
			m_QuoteCombo->PutCurSel(NULL);
			return;
		}
		
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ServiceID");
			CString strProductName = AdoFldString(rs, "Name", "");
			double dblQuantity = AdoFldDouble(rs, "Quantity", 1.0);
			BOOL bHasSerialNum = AdoFldBool(rs, "HasSerialNum", FALSE);
			BOOL bHasExpDate = AdoFldBool(rs, "HasExpDate", FALSE);
			BOOL bSerialNumIsLotNum = AdoFldBool(rs, "SerialNumIsLotNum", FALSE);

			// (j.jones 2008-01-07 14:44) - PLID 28479 - we removed this column from the recordset
			/*
			//before we try to add the product, we have to confirm it's billable for the location,
			//since the quote may contain products that are billable and those that are not
			BOOL bBillableForLoc = AdoFldBool(rs, "BillableForLocation", FALSE);
			if(!bBillableForLoc) {
				//the product is not billable at this location
				CString strWarn;
				strWarn.Format("The product '%s' is not billable for your current location, and will be skipped.", strProductName);
				AfxMessageBox(strWarn);
				
				rs->MoveNext();
				continue;
			}
			*/

			//try to add this product to the list
			//TES 7/3/2008 - PLID 24726 - Pass in bSerialNumIsLotNum
			TryAddProductToList(nProductID, strProductName, dblQuantity, bHasSerialNum, bHasExpDate, bSerialNumIsLotNum);

			rs->MoveNext();
		}
		rs->Close();

		//now clear the quote selection
		m_QuoteCombo->PutCurSel(NULL);

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenQuoteAllocCombo");
}

//sets the quote from clause with the current patient ID
void CInvPatientAllocationDlg::ResetQuotesFromClause()
{
	try {
		
		// (j.jones 2007-11-12 09:15) - PLID 27987 - Replicating the bill's quote dropdown, such that
		// if a quote is all outside fees, it will not be selectable in the list.
		// We define a charge as all outside fees if the practice fee is $0.00 and the outside fee is > $0.00
		// (j.gruber 2009-03-18 10:58) - PLID 33360 - update inventory structure
		CString strQuoteFrom;
		strQuoteFrom.Format("(SELECT [PatientBillsQ].ID, [PatientBillsQ].Description, [PatientBillsQ].Date, CASE WHEN HasBeenBilled Is Null THEN 0 ELSE 1 END AS HasBeenBilled, "
					"(CASE WHEN PackagesT.QuoteID Is Not Null THEN PackagesT.CurrentAmount ELSE "
					"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
					"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
					"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
					")),2)) END) AS Total "
					"FROM ((SELECT BillsT.*, (SELECT TOP 1 ID FROM BilledQuotesT WHERE BilledQuotesT.QuoteID = BillsT.ID AND BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) AS HasBeenBilled FROM BillsT "
					"WHERE BillsT.PatientID=%li AND BillsT.Deleted=0 AND BillsT.Active = 1) AS PatientBillsQ "
					"INNER JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"WHERE LineItemT.PatientID=%li AND LineItemT.Deleted=0 AND LineItemT.Type >= 10 AND (ChargesT.OthrBillFee = 0 OR LineItemT.Amount > 0)) AS PatientChargesQ ON [PatientBillsQ].ID = [PatientChargesQ].BillID) "
					"INNER JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
					"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID  "
					"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID "
					"LEFT JOIN PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID "
					"INNER JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
					"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE (([PatientBillsQ].EntryType)=2) AND ServiceT.Active = 1 "
					"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, PatientBillsQ.HasBeenBilled, PackagesT.QuoteID, PackagesT.CurrentAmount) AS Q", m_nCurPatientID, m_nCurPatientID);
		m_QuoteCombo->FromClause = _bstr_t(strQuoteFrom);
		
		//do not requery inside this function

	}NxCatchAll("Error in CInvPatientAllocationDlg::ResetQuotesFromClause");
}

void CInvPatientAllocationDlg::OnSelChosenAllocAddCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//hide all the dropdowns until they select something
			GetDlgItem(IDC_PRODUCT_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SURGERY_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUOTE_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			return;
		}

		SelectionTypes seltypeSelected = (SelectionTypes)VarLong(pRow->GetValue(selccID), (long)seltypeProduct);

		//display and requery the proper list
		switch(seltypeSelected) {

		case seltypeProduct:

			//the product combo would already be requeried

			//show the product combo, hide the surgery and quote combos
			GetDlgItem(IDC_PRODUCT_ALLOC_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SURGERY_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUOTE_ALLOC_COMBO)->ShowWindow(SW_HIDE);			
			break;
		
		case seltypeSurgery:

			////requery if it hasn't already been requeried
			if(!m_bSurgeriesRequeried) {			
				m_SurgeryCombo->Requery();
				m_bSurgeriesRequeried = TRUE;
			}

			//show the surgery combo, hide the product and quote combos
			GetDlgItem(IDC_PRODUCT_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SURGERY_ALLOC_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_QUOTE_ALLOC_COMBO)->ShowWindow(SW_HIDE);	
			break;

		case seltypeQuote:

			//if we have not already requeried the quote combo once, do it now,
			//provided we have set the from clause first (otherwise it would mean
			//we have no patient selected)
			if(!m_bQuotesRequeried && m_QuoteCombo->FromClause != _bstr_t("")) {			
				m_QuoteCombo->Requery();
				m_bQuotesRequeried = TRUE;
			}

			//show the quote combo, hide the product and surgery combos
			GetDlgItem(IDC_PRODUCT_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SURGERY_ALLOC_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUOTE_ALLOC_COMBO)->ShowWindow(SW_SHOW);	
			break;
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenAllocAddCombo");
}

// (j.jones 2007-11-12 15:49) - PLID 28074 - used to make the allocation read-only, if necessary
void CInvPatientAllocationDlg::SecureControls()
{
	try {

		_variant_t varEnabled;
		varEnabled.vt = VT_BOOL;
		varEnabled.boolVal = !m_bReadOnly;

		m_PatientCombo->Enabled = varEnabled;
		m_LocationCombo->Enabled = varEnabled;
		m_SelectionCombo->Enabled = varEnabled;
		m_ProductCombo->Enabled = varEnabled;
		m_SurgeryCombo->Enabled = varEnabled;
		m_QuoteCombo->Enabled = varEnabled;
		m_AppointmentCombo->Enabled = varEnabled;
		m_AllocationList->Enabled = varEnabled;		

		((CNxEdit*)GetDlgItem(IDC_EDIT_ALLOCATION_NOTES))->SetReadOnly(m_bReadOnly);

		// (j.jones 2007-11-27 11:53) - PLID 28196 - added the completion buttons
		if(m_bReadOnly) {
			//forcibly disable the completion button
			GetDlgItem(IDC_BTN_COMPLETE_ALLOCATION)->EnableWindow(FALSE);
		}
		else {
			//enable the "complete" button accordingly
			CheckEnableCompleteAllocationBtn();
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::SecureControls");
}

// (j.jones 2007-11-16 11:05) - PLID 28043 - added for streamlined auditing
CString CInvPatientAllocationDlg::GenerateAuditDescFromAllocationRow(NXDATALIST2Lib::IRowSettingsPtr pAllocationRow)
{
	try {

		if(pAllocationRow == NULL) {
			ASSERT(FALSE);
			return "";
		}

		CString strDesc;

		CString strProductName = VarString(pAllocationRow->GetValue(alcName),"");
		_variant_t varSerialNumber = pAllocationRow->GetValue(alcSerialNumber);
		_variant_t varExpDate = pAllocationRow->GetValue(alcExpDate);
		double dblQuantity = VarDouble(pAllocationRow->GetValue(alcQuantity), 1.0);
		
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

		return strDesc;		

	}NxCatchAll("Error in CInvPatientAllocationDlg::GenerateAuditDescFromAllocationRow");

	return "";
}

// (j.jones 2007-11-27 10:52) - PLID 28196 - added ability to complete an allocation from this screen
void CInvPatientAllocationDlg::OnBtnCompleteAllocation() 
{
	try {

		//confirm they have permission
		if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptWrite)) {
			return;
		}

		if(IDNO == MessageBox("Before the allocation can be completed, it must first be saved and closed.\n"
			"Do you wish to save and close this allocation now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//first save
		if(!Save()) {
			return;
		}

		ASSERT(m_nID != -1);

		//if we succeeded, we can close this dialog
		CDialog::OnOK();

		//now open the completion dialog

		CInvAllocationUsageDlg dlg(this);
		//the dialog will create a new object based on the ID
		dlg.SetAllocationInfo(m_nID);
		//they can't save the dialog without completing the allocation
		dlg.m_bForceCompletion = TRUE;
		dlg.DoModal();

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnBtnCompleteAllocation");
}

void CInvPatientAllocationDlg::OnRequeryFinishedAllocationList(short nFlags) 
{
	try {

		// (j.jones 2007-11-28 16:44) - PLID 28196 - enables/disable the complete allocation button
		CheckEnableCompleteAllocationBtn();

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnRequeryFinishedAllocationList");	
}

// (j.jones 2007-11-28 16:39) - PLID 28196 - enables/disables the complete allocation button
void CInvPatientAllocationDlg::CheckEnableCompleteAllocationBtn()
{
	try {

		if(m_bReadOnly) {
			//if read-only, definitely do not enable this button
			GetDlgItem(IDC_BTN_COMPLETE_ALLOCATION)->EnableWindow(FALSE);
			return;
		}

		//disable the completion button if no items are "allocated",
		//or if some are "allocated" and some are either "used", "released", or have no status
		//TES 7/18/2008 - PLID 29478 - For these purposes, we'll treat "To Be Ordered" as "allocated"

		// (j.jones 2008-02-18 09:42) - PLID 28948 - we now allow "released" on an
		// active allocation, so we don't have to restrict on that anymore

		//TES 7/18/2008 - PLID 29478 - New status
		BOOL bHasOrderProducts = FALSE;
		BOOL bHasAllocatedProducts = FALSE;
		BOOL bHasUsedProducts = FALSE;
		BOOL bHasReleasedProducts = FALSE;
		BOOL bHasProductsWithoutAStatus = FALSE;
		IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
		while(pRow != NULL) {

			//we just need to know if we have any products with a given status
			BOOL bOrder = VarBool(pRow->GetValue(alcOrder), FALSE);
			BOOL bAllocated = VarBool(pRow->GetValue(alcAllocated), FALSE);
			BOOL bUsed = VarBool(pRow->GetValue(alcUsed), FALSE);
			BOOL bReleased = VarBool(pRow->GetValue(alcReleased), FALSE);

			if(bOrder) {
				bHasOrderProducts = TRUE;
			}

			if(bAllocated) {
				bHasAllocatedProducts = TRUE;
			}

			if(bUsed) {
				bHasUsedProducts = TRUE;
			}

			if(bReleased) {
				bHasReleasedProducts = TRUE;
			}

			if(!bOrder && !bAllocated && !bUsed && !bReleased) {
				bHasProductsWithoutAStatus = TRUE;
			}

			pRow = pRow->GetNextRow();
		}

		// (j.jones 2008-02-18 09:42) - PLID 28948 - allow released products
		GetDlgItem(IDC_BTN_COMPLETE_ALLOCATION)->EnableWindow((bHasOrderProducts || bHasAllocatedProducts) && !bHasUsedProducts /*&& !bHasReleasedProducts*/ && !bHasProductsWithoutAStatus);

	}NxCatchAll("Error in CInvPatientAllocationDlg::CheckEnableCompleteAllocationBtn");	
}

// (j.jones 2008-02-27 11:14) - PLID 29102 - added the case history link
void CInvPatientAllocationDlg::OnRequeryFinishedCaseHistoryCombo(short nFlags) 
{
	try {
		//add an ability to unselect the case history
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_CaseHistoryCombo->GetNewRow();
		pRow->PutValue(chccID, (long)-1);
		pRow->PutValue(chccName, _bstr_t("< No Case History Selected >"));
		pRow->PutValue(chccSurgeryDate, g_cvarNull);
		pRow->PutValue(chccCompletedDate, g_cvarNull);
		pRow->PutValue(chccAppointmentID, g_cvarNull);
		m_CaseHistoryCombo->AddRowSorted(pRow, NULL);

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnRequeryFinishedCaseHistoryCombo");
}

// (j.jones 2008-02-27 16:51) - PLID 29108 - try to add products from a given case history
// (j.jones 2008-03-07 12:24) - PLID 29108 - we removed this ability, though perhaps it may
// return in the future
/*
void CInvPatientAllocationDlg::TryAddProductsFromCaseHistory(long nCaseHistoryID, CString strCaseName)
{
	try {

		//find all the products in the case history that are pay to practice,
		//(we don't mind if they are billable or not) and get their names
		//and serialized statii, then call TryAddProductToList

		ASSERT(m_nCurLocationID != -1); //shouldn't be possible, but let's confirm that

		//This should not be called from a completed case history, so this function
		//will make no attempt to try to use serial numbers from a case history.
		//If a product requires serialized information, it will request it from
		//available stock.

		_RecordsetPtr rs = CreateParamRecordset("SELECT ServiceT.ID, Name, Quantity, HasSerialNum, HasExpDate "
			"FROM CaseHistoryDetailsT "
			"INNER JOIN ProductT ON CaseHistoryDetailsT.ItemID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE ItemType = -1 AND PayToPractice = 1 AND ServiceT.Active = 1 "
			"AND CaseHistoryDetailsT.CaseHistoryID = {INT}", nCaseHistoryID);

		if(rs->eof) {
			//no active products in this case that are pay to practice, so tell the user
			CString strWarn;
			strWarn.Format("The case history '%s' does not contain any active products that are marked pay to practice.\n\n"
				"No products will be added to this allocation.", strCaseName);
			AfxMessageBox(strWarn);
			return;
		}
		
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ID");
			CString strProductName = AdoFldString(rs, "Name", "");
			double dblQuantity = AdoFldDouble(rs, "Quantity", 1.0);
			BOOL bHasSerialNum = AdoFldBool(rs, "HasSerialNum", FALSE);
			BOOL bHasExpDate = AdoFldBool(rs, "HasExpDate", FALSE);

			//try to add this product to the list
			TryAddProductToList(nProductID, strProductName, dblQuantity, bHasSerialNum, bHasExpDate);

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error in CInvPatientAllocationDlg::TryAddProductsFromCaseHistory");
}
*/

void CInvPatientAllocationDlg::OnSelChangingLocationAllocCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {

		IRowSettingsPtr pOldRow(lpOldSel);
		IRowSettingsPtr pNewRow(*lppNewSel);
		if(pOldRow == pNewRow) {
			//do nothing
			return;
		}

		// (j.jones 2008-02-29 10:08) - PLID 29102 - disallow changing the location
		// if there is a linked case history that is completed and using the same
		// serialized items

		long nCaseHistoryID = -1;
		IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->GetCurSel();
		if(pCaseRow) {
			nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
		}

		//call CanUnlinkCaseHistory to confirm whether we have to force uncompleting the case
		if(nCaseHistoryID != -1) {
			if(!CanUnlinkCaseHistory()) {

				//disallow changing locations
				SafeSetCOMPointer(lppNewSel, lpOldSel);

				//explain why
				AfxMessageBox("Changing locations would unlink the attached case history from this allocation, but the case history cannot be unlinked while it is completed and the two are sharing the same serial numbered / exp. date items.\n"
					"You must edit the case history and uncheck its 'Completed' box before you can change locations or unlink it from this allocation.");
			}
			else {
				//we can't give a general warning about unlinking the case, because the presence of a message box
				// causes OnSelChosen to not fire after this selection is changed. So instead we have to just
				// warn the user after the case history is cleared in OnSelChosen, that such an event occurred.
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChangingLocationAllocCombo");
}

// (j.jones 2008-02-29 08:47) - PLID 29102 - lets us know if we're allowed to unlink the case history
BOOL CInvPatientAllocationDlg::CanUnlinkCaseHistory()
{
	try {


		if(!IsSurgeryCenter(FALSE)) {
			//if no license, don't let this function stop anything
			return TRUE;
		}

		long nCaseHistoryID = -1;
		IRowSettingsPtr pCaseRow = m_CaseHistoryCombo->GetCurSel();
		if(pCaseRow) {
			nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
		}

		if(nCaseHistoryID != -1) {

			//we have a linked case history, so now we have to see if
			//is completed and sharing serial numbers

			CString strProductItemIDs;

			IRowSettingsPtr pRow = m_AllocationList->GetFirstRow();
			while(pRow != NULL) {
				
				long nProductItemID = VarLong(pRow->GetValue(alcProductItemID), -1);
				if(nProductItemID != -1) {
					if(!strProductItemIDs.IsEmpty()) {
						strProductItemIDs += ",";
					}
					strProductItemIDs += AsString(nProductItemID);
				}
				pRow = pRow->GetNextRow();
			}

			if(!strProductItemIDs.IsEmpty()) {
				//see if the case history is completed and also uses any of these product items
				if(ReturnsRecords("SELECT CaseHistoryT.ID FROM CaseHistoryT "
					"INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
					"WHERE CaseHistoryT.ID = %li "
					"AND CaseHistoryT.CompletedDate Is Not Null "
					"AND CaseHistoryDetailsT.ID IN (SELECT CaseHistoryDetailID FROM ChargedProductItemsT "
					"	WHERE ProductItemID IN (%s))",
					nCaseHistoryID, _Q(strProductItemIDs))) {

					//they are indeed sharing at least one product item, and thus cannot
					//be unlinked until the case is uncompleted
					return FALSE;
				}
			}

			//if we get here, we're good to go
			return TRUE;
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::CanUnlinkCaseHistory");

	return FALSE;
}

void CInvPatientAllocationDlg::OnSelChangingCaseHistoryCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		// (j.jones 2008-02-29 09:00) - PLID 29102 - disallow changing the case history if 
		// it is completed and using the same serialized items

		long nCaseHistoryID = -1;
		IRowSettingsPtr pCaseRow(lpOldSel);
		if(pCaseRow) {
			nCaseHistoryID = VarLong(pCaseRow->GetValue(chccID), -1);
		}

		//call CanUnlinkCaseHistory to confirm whether we have to force uncompleting the case
		if(nCaseHistoryID != -1	&& !CanUnlinkCaseHistory()) {

			//disallow this
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			
			//explain why
			AfxMessageBox("The attached case history cannot be unlinked from this allocation because the case history is completed, and the two are sharing the same serial numbered / exp. date items.\n"
				"You must edit the case history and uncheck its 'Completed' box before you can unlink it from this allocation.");
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChangingCaseHistoryCombo");	
}

// (j.jones 2008-03-05 12:15) - PLID 29201 - added provider combo
void CInvPatientAllocationDlg::OnSelChosenProviderAllocCombo(LPDISPATCH lpRow) 
{
	try {

		//simply track the provider ID as a member variable

		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			m_nCurProviderID = VarLong(pRow->GetValue(provccID), -1);
		}
		else {
			m_nCurProviderID = -1;
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnSelChosenProviderAllocCombo");
}

// (j.jones 2008-03-05 12:15) - PLID 29201 - added provider combo
void CInvPatientAllocationDlg::OnTrySetSelFinishedProviderAllocCombo(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure) {
			//maybe it's inactive?
			_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nPendingProviderID);
			if(!rsProv->eof) {
				m_ProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
			else 
				m_ProviderCombo->PutCurSel(NULL);
		}
		else {
			m_nPendingProviderID = -1;
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::OnTrySetSelFinishedProviderAllocCombo");
}

// (j.jones 2008-03-20 09:20) - PLID 29311 - added ability to create from an inventory order
void CInvPatientAllocationDlg::TryAddProductsFromCompletedOrder(long nOrderID)
{
	try {

		ASSERT(m_nCurLocationID != -1); //shouldn't be possible, but let's confirm that

		//find all the products in the order that are received, get their names
		//and serial numbers, then call AddProductToList

		//currently this is only called at the moment the order is received, not later,
		//but we should confirm that serialized products have not been used

		//This query is a little tricky because product items are tied to the order, not the order details,
		//and we really want to sum up order details anyways in the off-chance they may have had multiple
		//line items for the same product.
		//TES 6/18/2008 - PLID 29578 - Updated the query now that ProductItemsT has an OrderDetailID instead
		// of an OrderID
		//TES 7/3/2008 - PLID 24726 - Added SerialNumIsLotNum
		_RecordsetPtr rs = CreateParamRecordset("SELECT ProductT.ID AS ProductID, ServiceT.Name AS ProductName, "
			"Sum(OrderDetailsT.QuantityOrdered) AS Quantity, ProductT.HasSerialNum, ProductT.HasExpDate, "
			"ProductItemsT.ID AS ProductItemID, ProductItemsT.SerialNum, ProductItemsT.ExpDate, ProductItemsT.Status, "
			"ProductT.SerialNumIsLotNum "
			"FROM OrderT "
			"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
			"INNER JOIN ServiceT ON OrderDetailsT.ProductID = ServiceT.ID "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN (SELECT * FROM ProductItemsT "
			"	WHERE ProductItemsT.Deleted = 0 "
			"	AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"	AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"		AND PatientInvAllocationsT.Status <> " + AsString((long)InvUtils::iasDeleted) + " "
			"		AND PatientInvAllocationDetailsT.Status IN (" + AsString((long)InvUtils::iadsActive) + "," + AsString((long)InvUtils::iadsUsed) + ") "
			"		) "
			"	) AS ProductItemsT ON OrderDetailsT.ID = ProductItemsT.OrderDetailID "
			"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 AND ServiceT.Active = 1 "
			"AND OrderDetailsT.DateReceived Is Not Null "
			"AND OrderT.ID = {INT} "
			"AND (ProductItemsT.Deleted Is Null OR ProductItemsT.Deleted = 0) "
			"GROUP BY OrderT.ID, ProductT.ID, ServiceT.Name, ProductT.HasSerialNum, ProductT.HasExpDate, "
			"ProductItemsT.ID, ProductItemsT.SerialNum, ProductItemsT.ExpDate, ProductItemsT.Status, ProductT.SerialNumIsLotNum "
			"ORDER BY ProductT.ID ",
			nOrderID);

		if(rs->eof) {
			//no available products in this order, so tell the user
			AfxMessageBox("This order does not contain any active products that have been received and are still available for use.\n\n"
				"No products will be added to this allocation.");
			return;
		}

		long nLastProductID = -1;
		CString strLastProductName = "";
		
		while(!rs->eof) {

			//we should be getting one record per product if it is not serialized,
			//and one record per product item if it is serialized

			long nProductID = AdoFldLong(rs, "ProductID");
			CString strProductName = AdoFldString(rs, "ProductName", "");
			double dblQuantity = (double)AdoFldLong(rs, "Quantity", 1);
			BOOL bHasSerialNum = AdoFldBool(rs, "HasSerialNum", FALSE);
			BOOL bHasExpDate = AdoFldBool(rs, "HasExpDate", FALSE);
			long nProductItemID = AdoFldLong(rs, "ProductItemID", -1);
			_variant_t varSerialNumber = rs->Fields->Item["SerialNum"]->Value;
			_variant_t varExpDate = rs->Fields->Item["ExpDate"]->Value;
			_variant_t varStatus = rs->Fields->Item["Status"]->Value;
			BOOL bSerialNumIsLotNum = AdoFldBool(rs, "SerialNumIsLotNum", FALSE);

			//now update the quantity based on the product item data			
			if(nProductItemID != -1) {
				dblQuantity = 1.0;
			}

			//UU/UO is not needed because the UU quantity is what is saved in the order,
			//and is what we want here.

			//because we are bypassing TryAddProductToList, we need to perform the out-of-stock
			//check that it normally does, but we only want to do it once per product
			if(nLastProductID == -1) {
				//first product, so just cache its value
				nLastProductID = nProductID;
				strLastProductName = strProductName;
			}
			else if(nLastProductID != nProductID) {
				//we are on a different product now, so we need to check the amt. on hand
				//for the last product, and warn accordingly
				//need not pass in a quantity because the product is already in the list
				CheckWarnLowStock(nLastProductID, strLastProductName, 0.0);

				//now update our "last" information with the current product
				nLastProductID = nProductID;
				strLastProductName = strProductName;
			}
			else {
				//it's still the same product, so do nothing				
			}

			//now add the current product to the list
			//TES 7/3/2008 - PLID 24726 - Pass in bSerialNumIsLotNum
			//TES 7/18/2008 - PLID 29478 - Also pass in FALSE for ToBeOrdered, and whether the product is serialized
			AddProductToList(nProductID, strProductName, dblQuantity, nProductItemID, varSerialNumber, varExpDate, varStatus, bSerialNumIsLotNum, FALSE, bHasSerialNum || bHasExpDate);

			rs->MoveNext();
		}
		rs->Close();

		//the previous loop checked the amt. in stock for each previous product,
		//which means that now we still need to check one more time for the last product
		if(nLastProductID != -1) {
			//check the amt. on hand for the last product, and warn accordingly
			//need not pass in a quantity because the product is already in the list
			CheckWarnLowStock(nLastProductID, strLastProductName, 0.0);
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::TryAddProductsFromCompletedOrder");
}

// (j.jones 2008-03-20 10:22) - PLID 29311 - put the out-of-stock check in its own function
// Note: this is intentionally not called in OnEditingFinishing and OnEditingFinished functions
// as they run this check in specialized ways unique to their functionality
//TES 7/18/2008 - PLID 29478 - Added an optional output parameter, pbAddOnOrder.  If this is a valid pointer, then the user
// will be given the option to add the item with a "To Be Ordered" status, and that parameter will be filled with whether
// or not they chose to do so.
void CInvPatientAllocationDlg::CheckWarnLowStock(long nProductID, CString strProductName, double dblQuantityUsed, OUT BOOL *pbAddOnOrder /*= NULL*/)
{
	try {

		//use m_mapExistingQuantities and CalculateCurrentQuantity to find out
		//what was previously saved on this allocation (the map is empty if a new allocation),
		//and the current quantities on this allocation, respectively
		double dblPreviousQuantity = 0.0;
		m_mapExistingQuantities.Lookup(nProductID, dblPreviousQuantity);
		double dblCurQuantity = CalculateCurrentQuantity(nProductID);

		double dblOffset = dblPreviousQuantity - dblCurQuantity - dblQuantityUsed;

		double dblQuantityOnHand = 0.0;
		double dblAllocated = 0.0;

		// (j.jones 2007-12-18 11:42) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
		if(InvUtils::CalcAmtOnHand(nProductID, m_nCurLocationID, dblQuantityOnHand, dblAllocated, 0.0, -dblOffset) && dblQuantityOnHand - dblAllocated <= 0.0) {
			//Out of stock! Give a warning but let them continue.
			//If serialized, and there truly are no items available, they will get a second warning
			//(much like the way it works in billing).

			CString strAllocated = "";
			if(dblAllocated > 0.0) {
				strAllocated.Format(", and %g allocated to patients", dblAllocated);
			}

			CString str;
			if(pbAddOnOrder) {
				// (d.thompson 2009-12-16) - PLID 35903 - This warning is unnecessary for most, they always want to order.  Follow
				//	the preference.
				if(GetRemotePropertyInt("PromptToBeOrderedAllocation", 0, 0, GetCurrentUserName(), true) == 0) {
					//They do not want prompted
					*pbAddOnOrder = TRUE;
				}
				else if (GetRemotePropertyInt("PromptToBeOrderedAllocation", 0, 0, GetCurrentUserName(), true) == 1){
					//They do want to be prompted, this is the pre-preference behavior

					//TES 7/18/2008 - PLID 29478 - Give them the opportunity to add it as "To Be Ordered"
					str.Format("Warning: Practice shows that with this allocation, you will be out of stock of %s\n"
						"(after reserving this allocation, there will be %g on hand%s).\r\n"
						"Would you like to add this item with a 'To Be Ordered' status?", strProductName, dblQuantityOnHand, strAllocated);
					//TES 7/18/2008 - PLID 29478 - Let our caller know what they decided.
					if(IDYES == AfxMessageBox(str, MB_YESNO)) {
						*pbAddOnOrder = TRUE;
					}
					else {
						*pbAddOnOrder = FALSE;
					}
				}
				else {
					// (v.maida 2014-08-15 11:53) - PLID 54298 - The user does not want the status to be changed to 'To Be Ordered' when the item is being added to the allocation.
					*pbAddOnOrder = FALSE;
				}
			}
			else {
				str.Format("Warning: Practice shows that with this allocation, you will be out of stock of %s\n"
					"(after reserving this allocation, there will be %g on hand%s).\r\n", strProductName, dblQuantityOnHand, strAllocated);
				AfxMessageBox(str);
			}
		}

	}NxCatchAll("Error in CInvPatientAllocationDlg::CheckWarnLowStock");
}

// (j.gruber 2008-09-02 10:27) - PLID 30814 - added preview button
void CInvPatientAllocationDlg::OnBtnPreviewAllocations() 
{

	try {

		//first save the allocation
		if(IDNO == MessageBox("Before the allocation can be previewed, it must first be saved and closed.\n"
			"Do you wish to save and close this allocation now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		if (!Save()) {
			return;
		}
		
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(619)]);
		CString strExtraQuery;
		CPtrArray paramList;
		long nStatus = -1;

		strExtraQuery.Format(" WHERE AllocationID = %li ", m_nID);

		
		infReport.strReportName = strExtraQuery;

		//Add our date parameters as the defaults.  This is needed for the report to understand correctly
		CRParameterInfo *pFilter = new CRParameterInfo;
		pFilter->m_Name = "DateFrom";
		pFilter->m_Data = "01/01/1000";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		pFilter->m_Name = "DateTo";
		pFilter->m_Data = "12/31/5000";
		paramList.Add((void *)pFilter);

		RunReport(&infReport, &paramList, true, (CWnd *)this, "Allocation List", NULL);
		ClearRPIParameterList(&paramList);

		CNxDialog::OnOK();
		
	}NxCatchAll("Error in CInvPatientAllocationDlg::OnBtnPreviewAllocations() ");

	
	
}

//(c.copits 2010-09-14) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns an IRowSettingsPtr corresponding to the first found UPC order.

NXDATALIST2Lib::IRowSettingsPtr CInvPatientAllocationDlg::GetBestUPCProduct(_variant_t varBarcode)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	
	try {
		pRow = m_ProductCombo->SetSelByColumn(pccBarcode, varBarcode);
	} NxCatchAll(__FUNCTION__);

	return pRow;
}