// WaitingListMatches.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerRc.h"
#include "GlobalDataUtils.h"
#include "SchedulerView.h"
#include "WaitingListMatches.h"
#include "WaitingListEntryDlg.h"
#include "AuditTrail.h"
#include "SharedScheduleUtils.h"

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaitingListMatches dialog

// (d.moore 2007-07-12 10:36) - PLID 26546 - This dialog displays a set of matches
//  in the waitng list bases on the patient ID, resources selected, date, and time.

enum WaitingListMatchColumns {
	wlmWaitListID, 
	wlmAppointmentID, 
	wlmLineItemID, 
	wlmPatientID, 
	wlmTypeID, 
	wlmColor, 
	wlmPhone, 
	wlmDeleteCheck, 
	wlmName, 
	wlmApptFlag, 
	wlmApptType, 
	wlmPurpose
};

CWaitingListMatches::CWaitingListMatches(
		long nAppointmentID, 
		const CArray<long, long> &arWaitingListIDs, 
		CWnd* pParent)
	: CNxDialog(CWaitingListMatches::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaitingListMatches)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nAppointmentID = nAppointmentID;
	m_arWaitingListIDs.Copy(arWaitingListIDs);
	m_nMode = emNoMatch;
}


void CWaitingListMatches::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitingListMatches)
	DDX_Control(pDX, IDC_WL_BTN_DELETE_MATCHES, m_btnDeleteMatches);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_WL_MATCH_LABEL, m_nxstaticWlMatchLabel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CWaitingListMatches, CNxDialog)
	//{{AFX_MSG_MAP(CWaitingListMatches)
	ON_BN_CLICKED(IDC_WL_BTN_DELETE_MATCHES, OnWlBtnDeleteMatches)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitingListMatches message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CWaitingListMatches, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CWaitingListMatches)
	ON_EVENT(CWaitingListMatches, IDC_WAITING_LIST_MATCHES, 18 /* RequeryFinished */, OnRequeryFinishedWaitingListMatches, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


BOOL CWaitingListMatches::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	try {
		// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnDeleteMatches.AutoSet(NXB_DELETE);

		m_pMatchList = BindNxDataList2Ctrl(IDC_WAITING_LIST_MATCHES, false);

		// Configure the dialog based on the number of ID values that were passed in.
		long nNumIDs = m_arWaitingListIDs.GetSize();
		if (nNumIDs == 0) {
			// The dialog really shouldn't have been opened at all if there are
			//  no ID values, but deal with it anyway.
			ConfigureForNoMatch();
			return TRUE; // There is nothing to display, so just exit.
		}
		else if (nNumIDs == 1) {
			// Single match in the list.
			ConfigureForSingleMatch();
		}
		else {
			// Multiple matches for the list.
			ConfigureForMultipleMatch();
		}

		// Put all of the id values into a comma seperated list that can 
		//  be used later in queries.
		CString strId;
		for (long i = 0; i < nNumIDs; i++) {
			strId.Format("%li, ", m_arWaitingListIDs[i]);
			m_strCommaSeparatedIdList += strId;
		}
		m_strCommaSeparatedIdList = m_strCommaSeparatedIdList.Left(m_strCommaSeparatedIdList.GetLength() - 2);
		
		// Do the initial query for the list of matches.
		QueryMatchList();

	} NxCatchAll("Error In: CWaitingListMatches::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWaitingListMatches::OnWlBtnDeleteMatches() 
{
	try {
		if (m_nMode == emNoMatch) {
			// Can't do anything here, then list is empty.
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		if (m_nMode == emAppointmentMatch || m_nMode == emSingleMatch) {
			// If m_nMode is not emMultipleMatches, then there is only one row.
			//  So just delete the row.
			pRow = m_pMatchList->GetFirstRow();
			RemoveFromWaitList(pRow);
			// The list is now empty, so go ahead and close the dialog.
			CDialog::OnOK();
		} else {
			// If m_nMode is emMultipleMatches then we need to delete all checked rows.
			BOOL bChecked;
			long nLineItemID;
			//CArray<long, long> arIdList;
			CArray<NXDATALIST2Lib::IRowSettings*, NXDATALIST2Lib::IRowSettings*> arRowList;
			pRow = m_pMatchList->GetFirstRow();
			while (pRow != NULL) {
				nLineItemID = VarLong(pRow->GetValue(wlmLineItemID), -1);
				if (pRow->GetParentRow() == NULL) {

					if (nLineItemID <= 0) {
						bChecked = VarBool(pRow->GetValue(wlmDeleteCheck), FALSE);
						if (bChecked) {
							arRowList.Add(pRow);
						}
					}
				}
				pRow = pRow->GetNextRow();
			}
			
			// (d.moore 2007-10-26) - PLID 26546 - Check to make sure that something was
			//  actually checked. Prompt the user if not.
			//if (arIdList.GetSize()) {
			if (arRowList.GetSize()) {
				RemoveFromWaitList(arRowList);
				QueryMatchList();
			} else {
				MessageBox("Please check one or more boxes to delete items from the list.", NULL, MB_OK|MB_ICONINFORMATION);
			}
		}

		CSchedulerView* pView = (CSchedulerView*)(GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME));
		if (pView) {
			pView->UpdateView();
		}
		
	} NxCatchAll("Error In: CWaitingListMatches::OnWlBtnDeleteMatches");
}

void CWaitingListMatches::OnOK() 
{
	CDialog::OnOK();
}

void CWaitingListMatches::OnCancel() 
{
	CDialog::OnCancel();
}

void CWaitingListMatches::OnRequeryFinishedWaitingListMatches(short nFlags) 
{
	try {
		SetAppointmentColors();
		QueryLineItemCollection();
	} NxCatchAll("Error In: CWaitingListMatches::OnRequeryFinishedWaitingListMatches");
}

void CWaitingListMatches::QueryMatchList()
{
	CString strQuery;
	strQuery.Format(
		"SELECT "
			"WaitingListT.ID AS WaitListID, WaitingListT.PatientID, "
			"WaitingListT.AppointmentID, Convert(nvarchar(10), "
			"WaitingListT.CreatedDate, 101) AS CreatedDate, "
			"dbo.GetWaitListPurposeString(WaitingListT.ID) AS Purpose, "
			"CASE WHEN WaitingListT.AppointmentID IS NOT NULL THEN 'Yes' ELSE 'No' END AS ApptFlag, "
			"AptTypeT.NAME AS ApptType, AptTypeT.COLOR, WaitingListT.TypeID, "
			"PersonT.[Last] + ', ' + PersonT.[First] AS PersonName, "
			"CASE WHEN PersonT.PrivHome = 1 THEN '' ELSE PersonT.HomePhone END AS HomePhone " 
		"FROM WaitingListT "
		"LEFT JOIN AptTypeT "
		"ON WaitingListT.TypeID = AptTypeT.ID "
		"LEFT JOIN PersonT "
		"ON WaitingListT.PatientID = PersonT.ID "
		"LEFT JOIN AppointmentsT "
		"ON WaitingListT.AppointmentID = AppointmentsT.ID "
		"WHERE WaitingListT.ID IN (%s) "
			"AND (AppointmentsT.Status IS NULL "
					"OR AppointmentsT.Status <> 4)", // (d.moore 2007-10-18) - PLID 26546 - Added a filter to prevent canceled appointments from being selected.
		m_strCommaSeparatedIdList);

	m_pMatchList->Clear();
	_RecordsetPtr prs = CreateRecordsetStd(strQuery);
	CString strName, strPhone, strFormattedPhone;
	while (!prs->eof) {
		strName = AdoFldString(prs, "PersonName", "");
		strPhone = AdoFldString(prs, "HomePhone", "");
		strPhone.TrimRight();
		strFormattedPhone = "";
		if (!strPhone.IsEmpty()) {
			FormatText (strPhone, strFormattedPhone, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
		}
		strName += " " + strFormattedPhone;
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMatchList->GetNewRow();
		pRow->PutValue(wlmWaitListID, AdoFldLong(prs, "WaitListID", -1));
		pRow->PutValue(wlmAppointmentID, AdoFldLong(prs, "AppointmentID", -1));
		pRow->PutValue(wlmLineItemID, (long)-1);
		pRow->PutValue(wlmPatientID, AdoFldLong(prs, "PatientID", -1));
		pRow->PutValue(wlmTypeID, AdoFldLong(prs, "TypeID", -1));
		pRow->PutValue(wlmColor, AdoFldLong(prs, "COLOR", -1));
		pRow->PutValue(wlmPhone, (_variant_t)strFormattedPhone);
		pRow->PutValue(wlmDeleteCheck, false);
		pRow->PutValue(wlmName, (_variant_t)strName);
		pRow->PutValue(wlmApptFlag, (_variant_t)AdoFldString(prs, "ApptFlag", ""));
		pRow->PutValue(wlmApptType, (_variant_t)AdoFldString(prs, "ApptType", ""));
		pRow->PutValue(wlmPurpose, (_variant_t)AdoFldString(prs, "Purpose", ""));
		m_pMatchList->AddRowAtEnd(pRow, NULL);
		prs->MoveNext();
	}

	SetAppointmentColors();
	QueryLineItemCollection();
}

void CWaitingListMatches::QueryLineItemCollection()
{	
	// This function queries the database for every resource line 
	//  item associated with a list of waiting list entry match.
	
	try {
		// Get data for every line item.
		CString strQuery;
		strQuery.Format(
			"SELECT ID, StartDate, EndDate, StartTime, EndTime, AllResources, WaitingListID "
			"FROM WaitingListItemT WHERE WaitingListID IN (%s)", 
			m_strCommaSeparatedIdList);
		
		_RecordsetPtr prs = CreateRecordsetStd(strQuery);

		long nCount = prs->GetRecordCount();
		if (nCount < 1) {
			// Nothing was returned, so exit the function without doing anything else.
			return;
		}
		
		WaitListLineItem *arLineItems = new WaitListLineItem[nCount];
		long nIndex = 0;
		while (!prs->eof) {
			arLineItems[nIndex].nLineItemID = AdoFldLong(prs, "ID");
			arLineItems[nIndex].bAllResources = (AdoFldBool(prs, "AllResources") > 0)?true:false;
			arLineItems[nIndex].dtStartDate = AdoFldDateTime(prs, "StartDate");
			arLineItems[nIndex].dtEndDate = AdoFldDateTime(prs, "EndDate");
			arLineItems[nIndex].dtStartTime = AdoFldDateTime(prs, "StartTime");
			arLineItems[nIndex].dtEndTime = AdoFldDateTime(prs, "EndTime");
			arLineItems[nIndex].nWaitListID = AdoFldLong(prs, "WaitingListID");
			
			nIndex++;
			prs->MoveNext();
		}
		
		// Get data for resources associated with line items.
		strQuery.Format(
			"SELECT WaitingListItemT.ID AS LineItemID, "
				"WaitingListItemResourceT.ResourceID AS ResourceID, "
				"ResourceT.Item AS Name "
			"FROM WaitingListT INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"INNER JOIN WaitingListItemResourceT "
				"ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
				"INNER JOIN ResourceT "
				"ON WaitingListItemResourceT.ResourceID = ResourceT.ID "
			"WHERE WaitingListT.ID IN (%s)", 
			m_strCommaSeparatedIdList);
		
		prs = CreateRecordsetStd(strQuery);

		long nLineItemID;
		while (!prs->eof) {
			// First try to find a matching entry in the arLineItems array.
			//  There should only ever be a few entries, so just loop and look.
			nLineItemID = AdoFldLong(prs, "LineItemID", 0);
			for (long i = 0; i < nCount; i++) {
				if (arLineItems[i].nLineItemID == nLineItemID)
				{
					arLineItems[i].arResourceNames.Add(AdoFldString(prs, "Name", ""));
					break;
				}
			}
			prs->MoveNext();
		}

		// Get all of the days associated with each line item.
		strQuery.Format(
			"SELECT WaitingListItemT.ID AS LineItemID, "
				"WaitingListItemDaysT.DayOfWeekNum AS DayID "
			"FROM WaitingListT INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"INNER JOIN WaitingListItemDaysT "
				"ON WaitingListItemT.ID = WaitingListItemDaysT.ItemID "
			"WHERE WaitingListT.ID IN (%s) "
			"ORDER BY WaitingListItemDaysT.DayOfWeekNum", 
			m_strCommaSeparatedIdList);
		
		prs = CreateRecordsetStd(strQuery);

		while (!prs->eof) {
			// Again, try to find a match for the line item in the array.
			nLineItemID = AdoFldLong(prs, "LineItemID", 0);
			for (long i = 0; i < nCount; i++) {
				if (arLineItems[i].nLineItemID == nLineItemID)
				{
					arLineItems[i].arDayNames.Add(CWaitingListUtils::GetDayName(AdoFldLong(prs, "DayID", 0)));
					break;
				}
			}
			prs->MoveNext();
		}
		
		// Search for items in the list that match the line items in the collection.
		NXDATALIST2Lib::IRowSettingsPtr pRow, pNewRow;
		for (long i = 0; i < nCount; i++) {
			
			pRow = m_pMatchList->FindByColumn(wlmWaitListID, arLineItems[i].nWaitListID, NULL, FALSE);
			if (pRow != NULL) {
				pNewRow = m_pMatchList->GetNewRow();
				pNewRow->PutValue(wlmWaitListID, arLineItems[i].nWaitListID);
				pNewRow->PutValue(wlmAppointmentID, (long)-1);
				pNewRow->PutValue(wlmLineItemID, arLineItems[i].nLineItemID);
				pNewRow->PutValue(wlmName, (_variant_t)CWaitingListUtils::FormatLineItem(arLineItems[i]));
				m_pMatchList->AddRowAtEnd(pNewRow, pRow);
				pRow->PutExpanded(TRUE);
			}
		}

		delete [] arLineItems;

	} NxCatchAll("Error in CWaitingListMatches::QueryLineItemCollection");

	return;
}

void CWaitingListMatches::SetAppointmentColors()
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMatchList->GetFirstRow();
	while(pRow != NULL) {
		// (d.moore 2007-05-10 13:23) - PLID 4013 - Change to NxDatalist2
		OLE_COLOR color = pRow->GetValue(wlmColor).lVal;
		// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
		color = GetDarkerColorForApptText(color);
		pRow->PutForeColor(color);
		pRow = pRow->GetNextRow();
	}

}

void CWaitingListMatches::RemoveFromWaitList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// This function removes the row from the m_pMatchList and then removes
	//  all associated records from the database.
	
	if (pRow == NULL) {
		return;
	}

	try {
		long nWaitListID = VarLong(pRow->GetValue(wlmWaitListID), 0);
		long nLineItemID = VarLong(pRow->GetValue(wlmLineItemID), 0);

		// Make sure that we have the parent row.
		if (nLineItemID > 0) {
			pRow = pRow->GetParentRow();
		}
		m_pMatchList->RemoveRow(pRow);

		
		if (nWaitListID <= 0) {
			// The ID is invalid, can't do anything else.
			return;
		}

		// Resource query.
		CString strQuery;
		strQuery.Format(
			"DECLARE @Wait_List_ID INT; \r\n"
			"SET @Wait_List_ID = %li; \r\n", 
			nWaitListID);

		strQuery +=
			// Delete Resources Query.
			"DELETE FROM WaitingListItemResourceT "
			"WHERE ItemID IN "
				"(SELECT ID FROM WaitingListItemT "
				"WHERE WaitingListID = @Wait_List_ID); "
			"\r\n"
			// Delete Days Query.
			"DELETE FROM WaitingListItemDaysT "
			"WHERE ItemID IN "
				"(SELECT ID FROM WaitingListItemT "
				"WHERE WaitingListID = @Wait_List_ID); "
			"\r\n"
			// Delete Line Items Query.
			"DELETE FROM WaitingListItemT "
			"WHERE WaitingListID = @Wait_List_ID; "
			"\r\n"
			// Delete Purpose Query.
			"DELETE FROM WaitingListPurposeT "
			"WHERE WaitingListID = @Wait_List_ID; "
			"\r\n"
			// Delete Waiting List Entry Query.
			"DELETE FROM WaitingListT WHERE ID = @Wait_List_ID;"
			"\r\n";
			
		
		// Update the appointment modified date.
		CString strApptUpdate;
		strApptUpdate.Format(
			"UPDATE AppointmentsT "
					"SET ModifiedDate = GetDate(), "
					"ModifiedLogin = '%s' "
				"WHERE AppointmentsT.ID IN "
					"(SELECT AppointmentID "
						"FROM WaitingListT "
						"WHERE ID = @Wait_List_ID)",
			_Q(GetCurrentUserName()));

		strQuery += strApptUpdate;

		ExecuteSqlStd(strQuery);

		// Do auditing.
		CString strPatientName = VarString(pRow->GetValue(wlmName), "");
		CString strPhone = VarString(pRow->GetValue(wlmPhone), "");
		strPatientName.Replace(" " + strPhone, "");

		long nPatientID = VarLong(pRow->GetValue(wlmPatientID), -1);
		
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptMoveUp, nWaitListID, "", "Remove Appointment Move Up", aepMedium);

	} NxCatchAll("Error IN: CWaitingListMatches::RemoveFromWaitList");
}

void CWaitingListMatches::RemoveFromWaitList(const CArray<NXDATALIST2Lib::IRowSettings*, NXDATALIST2Lib::IRowSettings*> &arIdList)
{
	// (d.moore 2007-10-26) - PLID 26546 - Check to see if the array has anything in it
	//  before proceeding.
	long nNumIDs = arIdList.GetSize();
	if (nNumIDs <= 0) {
		return;
	}

	// Delete multiple items in one query.
	long nWaitListID;
	CString strItemQuery, strDeleteQuery, strPatientName, strPhone;
	long nAuditTransactionID = -1;

	NXDATALIST2Lib::IRowSettingsPtr pRow;

	for (long i = 0; i < nNumIDs; i++) {
		pRow = arIdList.GetAt(i);
		if (pRow != NULL) {
		
			nWaitListID = VarLong(pRow->GetValue(wlmWaitListID), -1);
			if (nWaitListID > 0) {

				strItemQuery.Format(
					// Delete Resources Query.
					"DELETE FROM WaitingListItemResourceT "
					"WHERE ItemID IN "
						"(SELECT ID FROM WaitingListItemT "
						"WHERE WaitingListID = %li); "
					"\r\n"
					// Delete Days Query.
					"DELETE FROM WaitingListItemDaysT "
					"WHERE ItemID IN "
						"(SELECT ID FROM WaitingListItemT "
						"WHERE WaitingListID = %li); "
					"\r\n"
					// Delete Line Items Query.
					"DELETE FROM WaitingListItemT "
					"WHERE WaitingListID = %li; "
					"\r\n"
					// Delete Purpose Query.
					"DELETE FROM WaitingListPurposeT "
					"WHERE WaitingListID = %li; "
					"\r\n"
					// Delete Waiting List Entry Query.
					"DELETE FROM WaitingListT WHERE ID = %li;"
					"\r\n"
					// Update Appointment modified date.
					"UPDATE AppointmentsT "
							"SET ModifiedDate = GetDate(), "
							"ModifiedLogin = '%s' "
						"WHERE AppointmentsT.ID IN "
							"(SELECT AppointmentID "
								"FROM WaitingListT "
								"WHERE ID = %li)",
					nWaitListID, nWaitListID, nWaitListID, nWaitListID, nWaitListID,
					_Q(GetCurrentUserName()), nWaitListID);
				
				strDeleteQuery += strItemQuery;
		
				// Audit the removal.
				CString strPatientName = VarString(pRow->GetValue(wlmName), "");
				CString strPhone = VarString(pRow->GetValue(wlmPhone), "");
				strPatientName.Replace(" " + strPhone, "");
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				long nPatientID = VarLong(pRow->GetValue(wlmPatientID), -1);
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiApptMoveUp, nWaitListID, "", "Remove Appointment Move Up", aepMedium);
			}
		}
	}		

	ExecuteSqlStd(strDeleteQuery);

	// Commit auditing
	if (nAuditTransactionID != -1) {
		CommitAuditTransaction(nAuditTransactionID);
	}

}

void CWaitingListMatches::ConfigureForNoMatch()
{
	m_nMode = emNoMatch;

	// Nothing can be done, so just deactivate the main buttons.
	GetDlgItem(IDC_WL_BTN_DELETE_MATCHES)->EnableWindow(FALSE);
	
	// Set the label for the dialog.
	CString strLabel = "There were no matches found in the waiting list.";
	SetDlgItemText(IDC_WL_MATCH_LABEL, strLabel);
	
	// Make the checkboxes visible for this behaviour type only.
	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pMatchList->GetColumn(wlmDeleteCheck);
	long nStyle = NXDATALIST2Lib::csFixedWidth;
	pCol->PutColumnStyle(nStyle);
}

void CWaitingListMatches::ConfigureForSingleMatch()
{
	if (m_arWaitingListIDs.GetSize() == 0) {
		// This should usually be checked before this function is called,
		//  but check it again just in case.
		ConfigureForNoMatch();
	}
	
	GetDlgItem(IDC_WL_BTN_DELETE_MATCHES)->EnableWindow(TRUE);
	
	// Check to see if the match is linked to the appointment.
	CString strQuery;
	strQuery.Format(
		"SELECT WaitingListT.AppointmentID "
		"FROM WaitingListT INNER JOIN WaitingListItemT "
			"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
		"WHERE WaitingListT.ID=%li", 
		m_arWaitingListIDs[0]);

	_RecordsetPtr prs = CreateRecordsetStd(strQuery);
	if (prs->eof) {
		// If a record wasn't returned, then the waiting list entry must be bad data.
		ConfigureForNoMatch();
		return;
	}
	
	CString strLabel;
	long nApptID = AdoFldLong(prs, "AppointmentID", -1);
	
	if (nApptID == m_nAppointmentID) {
		m_nMode = emAppointmentMatch;
		strLabel = 
			"The appointment is in the waiting list. Since the appointments "
			"date, time, or resources have changed, you may want to remove "
			"it from the list or edit its list entry.";
	} else {
		m_nMode = emSingleMatch;
		strLabel = 
			"There is an entry in the waiting list that closely matches the "
			"appointment. If the appointment fulfills the waiting list request, "
			"then you may want to remove the entry from the list.";
	}

	SetDlgItemText(IDC_WL_MATCH_LABEL, strLabel);
	SetDlgItemText(IDC_WL_BTN_DELETE_MATCHES, "Delete Entry");

	// Make the checkboxes visible for this behaviour type only.
	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pMatchList->GetColumn(wlmDeleteCheck);
	long nStyle = NXDATALIST2Lib::csFixedWidth;
	pCol->PutColumnStyle(nStyle);
}

void CWaitingListMatches::ConfigureForMultipleMatch()
{
	m_nMode = emMultipleMatches;
	
	GetDlgItem(IDC_WL_BTN_DELETE_MATCHES)->EnableWindow(TRUE);

	// Set the label for the dialog.
	CString strLabel = 
		"There are several entries in the waiting list that closely "
		"match the appointment. If the appointment fulfills any of "
		"the waiting list requests, then you may want to remove the "
		"entries from the list.";
	SetDlgItemText(IDC_WL_MATCH_LABEL, strLabel);

	SetDlgItemText(IDC_WL_BTN_DELETE_MATCHES, "Delete Checked Items");

	// Make the checkboxes visible for this behaviour type only.
	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pMatchList->GetColumn(wlmDeleteCheck);
	long nStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csEditable;
	pCol->PutColumnStyle(nStyle);
}

