// WaitingListEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerRc.h"
#include "WaitingListEntryDlg.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"
#include "MultiSelectDlg.h"
#include "InternationalUtils.h"
#include "WaitingListResourceDlg.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWaitingListEntryDlg dialog

// (d.moore 2007-05-23 11:31) - PLID 4013

enum ewlPatientCols {
	ewlpcID, 
	ewlpcLastName,
	ewlpcFirstName, 
	ewlpcMiddleInit,
	// ewlpcFullName, // (a.walling 2010-06-01 08:33) - PLID 38917 - no more FullName column
	ewlpcPatientID, 
	ewlpcBirthDate, 
	ewlpcSocialSecurity
};

enum ewlApointmentTypeCols {
	ewlatcID,
	ewlatcName
};

enum ewlApointmentPurposeCols {
	ewlapcID, 
	ewlapcName
};

enum ewlResourceListCols {
	ewlrlcID, 
	ewlrlcText, 
	ewlrlcTempID
};

CWaitingListEntryDlg::CWaitingListEntryDlg(CWnd* pParent)
	: CNxDialog(CWaitingListEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWaitingListEntryDlg)
	//}}AFX_DATA_INIT
	m_WaitListID = -1;
	m_nAppointmentID = -1;
	m_nPatientID = -1;
	//TES 3/26/2008 - PLID 29426 - Renamed this from m_nInnitialApptTypeID
	m_nInitialApptTypeID = -1;
}

CWaitingListEntryDlg::~CWaitingListEntryDlg()
{
}

void CWaitingListEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaitingListEntryDlg)
	DDX_Control(pDX, IDC_WL_PURPOSE_MULTI_LIST, m_nxlPurposeLabel);
	DDX_Control(pDX, IDC_DATE_ENTERED, m_nxeditDateEntered);
	DDX_Control(pDX, IDC_WAIT_LIST_NOTES, m_nxeditWaitListNotes);
	DDX_Control(pDX, IDC_STATIC_PATIENT, m_nxstaticPatient);
	DDX_Control(pDX, IDC_STATIC_APPT_TYPE, m_nxstaticApptType);
	DDX_Control(pDX, IDC_STATIC_PURPOSE, m_nxstaticPurpose);
	DDX_Control(pDX, IDC_STATIC_DATE_ENTERED, m_nxstaticDateEntered);
	DDX_Control(pDX, IDC_STATIC_WAIT_LIST_ITEMS, m_nxstaticWaitListItems);
	DDX_Control(pDX, IDC_STATIC_NOTES, m_nxstaticNotes);
	DDX_Control(pDX, IDC_ADD_REQUEST_ITEM, m_btnAddRequestItem);
	DDX_Control(pDX, IDC_EDIT_REQUEST_ITEM, m_btnEditRequestItem);
	DDX_Control(pDX, IDC_REMOVE_REQUEST_ITEM, m_btnRemoveRequestItem);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CWaitingListEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CWaitingListEntryDlg)
	ON_BN_CLICKED(IDC_ADD_REQUEST_ITEM, OnAddRequestedItem)
	ON_BN_CLICKED(IDC_REMOVE_REQUEST_ITEM, OnRemoveRequestedItem)
	ON_BN_CLICKED(IDC_EDIT_REQUEST_ITEM, OnEditRequestedItem)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWaitingListEntryDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CWaitingListEntryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CWaitingListEntryDlg)
	ON_EVENT(CWaitingListEntryDlg, IDC_REQUEST_LIST, 2 /* SelChanged */, OnSelChangedRequestList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CWaitingListEntryDlg, IDC_WL_APPT_TYPE, 16 /* SelChosen */, OnSelChosenWlApptType, VTS_DISPATCH)
	ON_EVENT(CWaitingListEntryDlg, IDC_WL_APPT_TYPE, 18 /* RequeryFinished */, OnRequeryFinishedWlApptType, VTS_I2)
	ON_EVENT(CWaitingListEntryDlg, IDC_WL_PURPOSE, 18 /* RequeryFinished */, OnRequeryFinishedWlPurpose, VTS_I2)
	ON_EVENT(CWaitingListEntryDlg, IDC_WL_PURPOSE, 16 /* SelChosen */, OnSelChosenWlPurpose, VTS_DISPATCH)
	ON_EVENT(CWaitingListEntryDlg, IDC_REQUEST_LIST, 3 /* DblClickCell */, OnDblClickCellRequestList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CWaitingListEntryDlg, IDC_WL_PATIENT, 18 /* RequeryFinished */, OnRequeryFinishedWlPatient, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


BOOL CWaitingListEntryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call
	
	try {
		// (z.manning, 04/29/2008) - PLID 29814 - Set button styles
		m_btnAddRequestItem.AutoSet(NXB_NEW);
		m_btnEditRequestItem.AutoSet(NXB_MODIFY);
		m_btnRemoveRequestItem.AutoSet(NXB_DELETE);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Patient Dropdown
		m_pPatient = BindNxDataList2Ctrl(IDC_WL_PATIENT, false);
		m_pPatient->Requery();
		// Appointment Type Dropdown
		m_pApptType = BindNxDataList2Ctrl(IDC_WL_APPT_TYPE, false);
		m_pApptType->Requery();
		// Appontment Purpose Dropdown
		m_pApptPurpose = BindNxDataList2Ctrl(IDC_WL_PURPOSE, false);
		GetDlgItem(IDC_WL_PURPOSE)->EnableWindow(FALSE);
		// Resource Line Items Dropdown
		m_pRequestItems = BindNxDataList2Ctrl(IDC_REQUEST_LIST, false);

		// These buttons are initially not usable.
		GetDlgItem(IDC_REMOVE_REQUEST_ITEM)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_REQUEST_ITEM)->EnableWindow(FALSE);
		
		// Limit the size of the Notes field.
		CNxEdit *pNotesField = (CNxEdit*)GetDlgItem(IDC_WAIT_LIST_NOTES);
		if (pNotesField != NULL) {
			pNotesField->SetLimitText(2000);
		}

		// Hyperlink for displaying multiple selected purpose values.
		m_nxlPurposeLabel.SetText("");
		m_nxlPurposeLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_HIDE);
		m_nxlPurposeLabel.SetColor(GetSysColor(COLOR_3DFACE));

		// Set the values for all of the controls.
		if (m_WaitListID > 0) {
			QueryData();
		} else {
			SetDefaultValues();
		}
		
		return TRUE;  // return TRUE unless you set the focus to a control
					  // EXCEPTION: OCX Property Pages should return FALSE
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnInitDialog");
	
	return FALSE;
}

void CWaitingListEntryDlg::OnSelChosenWlApptType(LPDISPATCH lpRow) 
{
	try {
		if (lpRow != NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			// Get the ID value for the row selected in the Type dropdown.
			long nTypeID = VarLong(pRow->GetValue(ewlatcID), -1);
			LoadPurposeDropdown(nTypeID);
		}
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnSelChosenWlApptType");
}

void CWaitingListEntryDlg::OnSelChosenWlPurpose(LPDISPATCH lpRow) 
{
	try {

		if (lpRow != NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			// Get the selected type ID. Needed for the multi-select where query.
			long nTypeID = VarLong(m_pApptType->CurSel->GetValue(ewlatcID), -1);
			// (c.haag 2008-12-17 17:36) - PLID 32376 - Removed a previous definition to strWhere because it wasn't
			// actually used (and I wasn't paying attention)
			if (nTypeID <= 0) {
				return;
			}
			
			// Get the ID value for the row selected in the purpose dropdown.
			long nSelectedID = VarLong(pRow->GetValue(ewlapcID), -1);
			if (nSelectedID > 0) {
				// A Single selection was made.
				m_arPurposeIDs.RemoveAll();
				m_arPurposeIDs.Add(nSelectedID);
			} 
			else if (nSelectedID == -2) {
				// {Multiple Purposes}
				OpenPurposeMultiList();
			}
			else {
				// {No Purpose} selected.
				m_arPurposeIDs.RemoveAll();
			}
		}

	} NxCatchAll("Error In: CWaitingListEntryDlg::OnSelChosenWlPurpose");
}

void CWaitingListEntryDlg::OpenPurposeMultiList() 
{
	try {
		// Get the selected type ID. Needed for the multi-select where query.
		long nTypeID = VarLong(m_pApptType->CurSel->GetValue(ewlatcID), -1);
		CString strWhere;
		if (nTypeID <= 0) {
			return;
		} else {
			// (c.haag 2008-12-17 17:36) - PLID 32376 - Filter out inactive procedures except for ones assigned here
			strWhere.Format("AptPurposeTypeT.AptTypeID = %li AND ("
				"AptPurposeTypeT.AptPurposeID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
				"OR AptPurposeTypeT.AptPurposeID IN (%s)) "
				, nTypeID, ArrayAsString(m_arPurposeIDs));
		}
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptPurposeT");
		dlg.PreSelect(m_arPurposeIDs);
		int nResult = dlg.Open(
			"AptPurposeT INNER JOIN AptPurposeTypeT "
			"ON AptPurposeT.ID = AptPurposeTypeT.AptPurposeID", 
			strWhere, "AptPurposeT.ID", "AptPurposeT.Name", 
			"Select Purposes");
		
		if (nResult == IDOK) {
			m_arPurposeIDs.RemoveAll();
			dlg.FillArrayWithIDs(m_arPurposeIDs);
			// Display the selected items in a hyperlink.
			if (m_arPurposeIDs.GetSize() > 1) {
				// Multiple items were selected form the dialog.
				// Get their ID values.
				CString strIDList, strID;
				for (long i = 0; i < m_arPurposeIDs.GetSize(); i++) {
					strID.Format("%li, ", m_arPurposeIDs[i]);
					strIDList += strID;
				}
				strIDList = strIDList.Left(strIDList.GetLength()-2);

				// Get the names that match the ID values.
				CString strPurpList;
				_RecordsetPtr prs = CreateRecordset(
					"SELECT Name FROM AptPurposeT WHERE ID IN (%s)", strIDList);
				while(!prs->eof) {
					strPurpList += AdoFldString(prs, "Name") + ", ";
					prs->MoveNext();
				}
				strPurpList = strPurpList.Left(strPurpList.GetLength()-2);

				m_nxlPurposeLabel.SetText(strPurpList);
				m_nxlPurposeLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_WL_PURPOSE, SW_HIDE);
				ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_SHOW);
				InvalidateDlgItem(IDC_WL_PURPOSE_MULTI_LIST);
			} 
			else if (m_arPurposeIDs.GetSize() == 1) {
				// Only a single item was selected from the dialog.
				GetDlgItem(IDC_WL_PURPOSE)->EnableWindow(TRUE);
				m_pApptPurpose->SetSelByColumn(ewlapcID, m_arPurposeIDs[0]);
				m_nxlPurposeLabel.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_WL_PURPOSE, SW_SHOW);
				ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_HIDE);
				InvalidateDlgItem(IDC_WL_PURPOSE_MULTI_LIST);
			} else {
				// No selection made.
				GetDlgItem(IDC_WL_PURPOSE)->EnableWindow(TRUE);
				m_pApptPurpose->SetSelByColumn(ewlapcID, (long)-1);
				m_nxlPurposeLabel.SetType(dtsDisabledHyperlink);
				ShowDlgItem(IDC_WL_PURPOSE, SW_SHOW);
				ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_HIDE);
				InvalidateDlgItem(IDC_WL_PURPOSE_MULTI_LIST);
			}
		}
	} NxCatchAll("Error in CWaitingListEntryDlg::OpenPurposeMultiList");
}

void CWaitingListEntryDlg::OnRequeryFinishedWlPatient(short nFlags) 
{
	try {
		if (m_nAppointmentID > 0) {
			// (d.moore 2007-10-22) - PLID 4013 - I added the ability to have No Patient, but only
			//  when there is already an appointment already.
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatient->GetNewRow();
			pRow = m_pPatient->GetNewRow();
			pRow->PutValue(ewlpcID, (long) -25);
			// (a.walling 2010-06-01 08:33) - PLID 38917 - no more FullName column
			//pRow->PutValue(ewlpcFullName, _variant_t(" { No Patient Selected } "));
			pRow->PutValue(ewlpcLastName, _variant_t(" { No Patient Selected } "));
			m_pPatient->AddRowBefore(pRow, m_pPatient->GetFirstRow());
			
			GetDlgItem(IDC_WL_PATIENT)->EnableWindow(FALSE);
			m_pPatient->SetSelByColumn(ewlpcID, m_nPatientID);
		}
		
	} NxCatchAll("Error In: OnRequeryFinishedWlPatient");
}

void CWaitingListEntryDlg::OnRequeryFinishedWlApptType(short nFlags) 
{
	try {
		if (m_nAppointmentID > 0) {
			GetDlgItem(IDC_WL_APPT_TYPE)->EnableWindow(FALSE);
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pApptType->GetNewRow();
		pRow->PutValue(ewlatcID, (long) -1);
		pRow->PutValue(ewlatcName, _variant_t("{No Type}"));
		m_pApptType->AddRowBefore(pRow, m_pApptType->GetFirstRow());
		//TES 3/26/2008 - PLID 29426 - This was being set to -1; instead, set it to the actual type we loaded from data.
		m_pApptType->SetSelByColumn(ewlatcID, m_nInitialApptTypeID);
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnRequeryFinishedWlApptType");
}

void CWaitingListEntryDlg::OnRequeryFinishedWlPurpose(short nFlags) 
{
	try {
		// Add the {Multiple Purposes}
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pApptPurpose->GetNewRow();
		pRow->PutValue(ewlapcID, (long) -2);
		pRow->PutValue(ewlapcName, _variant_t("{Multiple Purposes}"));
		m_pApptPurpose->AddRowBefore(pRow, m_pApptPurpose->GetFirstRow());
		// Add the {No Purpose}
		pRow = m_pApptPurpose->GetNewRow();
		pRow->PutValue(ewlapcID, (long) -1);
		pRow->PutValue(ewlapcName, _variant_t("{No Purpose}"));
		m_pApptPurpose->AddRowBefore(pRow, m_pApptPurpose->GetFirstRow());

		// Set a row as selected.
		if (m_arPurposeIDs.GetSize() <= 0) {
			// Select 'No Purpose'
			m_pApptPurpose->SetSelByColumn(ewlapcID, (long)-1);
			m_nxlPurposeLabel.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_WL_PURPOSE, SW_SHOW);
			ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_HIDE);
			InvalidateDlgItem(IDC_WL_PURPOSE_MULTI_LIST);
		} 
		else if (m_arPurposeIDs.GetSize() > 1) {
			// Select 'Multiple Purposes'.
			CString strIDList, strID;
			for (long i = 0; i < m_arPurposeIDs.GetSize(); i++) {
				strID.Format("%li, ", m_arPurposeIDs[i]);
				strIDList += strID;
			}
			strIDList = strIDList.Left(strIDList.GetLength()-2);

			// Get the names that match the ID values.
			CString strPurpList;
			_RecordsetPtr prs = CreateRecordset(
				"SELECT Name FROM AptPurposeT WHERE ID IN (%s)", strIDList);
			while(!prs->eof) {
				strPurpList += AdoFldString(prs, "Name") + ", ";
				prs->MoveNext();
			}
			strPurpList = strPurpList.Left(strPurpList.GetLength()-2);

			// Set the Hyperlink.
			m_nxlPurposeLabel.SetText(strPurpList);
			m_nxlPurposeLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_WL_PURPOSE, SW_HIDE);
			ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_WL_PURPOSE_MULTI_LIST);
		} else {
			// Only one item selected.
			if (NULL == m_pApptPurpose->SetSelByColumn(ewlapcID, m_arPurposeIDs[0])) {
				// (c.haag 2008-12-29 12:08) - PLID 32376 - The procedure may just be inactive. I cannot think of a way how
				// to execute this code other than forcing execution through the debugger; but it is a failsafe in case
				// somehow the purpose does not show up because the purpose query didn't include it.
				// Try adding it
				_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}", m_arPurposeIDs[0]);
				if (!prs->eof) {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptPurpose->GetNewRow();
					pRow->Value[0L] = (long)m_arPurposeIDs[0];
					pRow->Value[1L] = prs->Fields->Item["Name"]->Value;
					NXDATALIST2Lib::IRowSettingsPtr pAddedRow = m_pApptPurpose->AddRowSorted(pRow, NULL);
					m_pApptPurpose->CurSel = pAddedRow;
				}
			}
			m_nxlPurposeLabel.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_WL_PURPOSE, SW_SHOW);
			ShowDlgItem(IDC_WL_PURPOSE_MULTI_LIST, SW_HIDE);
			InvalidateDlgItem(IDC_WL_PURPOSE_MULTI_LIST);
		}

		if (m_nAppointmentID > 0) {
			GetDlgItem(IDC_WL_PURPOSE)->EnableWindow(FALSE);
			return;
		}
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnRequeryFinishedWlPurpose");
}

void CWaitingListEntryDlg::OnSelChangedRequestList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRequestItems->GetCurSel();
		if (pRow != NULL) {
			GetDlgItem(IDC_REMOVE_REQUEST_ITEM)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_REQUEST_ITEM)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_REMOVE_REQUEST_ITEM)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_REQUEST_ITEM)->EnableWindow(FALSE);
		}
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnSelChangedRequestList");
}

void CWaitingListEntryDlg::OnDblClickCellRequestList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		OnEditRequestedItem();
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnDblClickCellRequestList");
}

void CWaitingListEntryDlg::OnAddRequestedItem() 
{
	try {
		// Adding a new line item to the entry for a resource.
		CWaitingListResourceDlg dlg(this);
		if (dlg.DoModal() == IDOK) {
			WaitListLineItem wlItem;
			dlg.GetLineItemData(wlItem);
			AddNewLineItemData(wlItem);
		}
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnAddRequestedItem");
}

void CWaitingListEntryDlg::OnRemoveRequestedItem() 
{
	try {
		DeleteLineItem();
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnRemoveRequestedItem");
}

void CWaitingListEntryDlg::OnEditRequestedItem() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRequestItems->GetCurSel();
		if (pRow != NULL) {
			CWaitingListResourceDlg dlg(this);
			WaitListLineItem wlItem;
			long nTempID = VarLong(pRow->GetValue(ewlrlcTempID));
			for (long i = 0; i < m_arLineItems.GetSize(); i++) {
				if (nTempID == m_arLineItems[i].nTempID) {
					wlItem = m_arLineItems[i];
					break;
				}
			}

			dlg.SetLineItemData(wlItem);
			if (dlg.DoModal() == IDOK) {
				dlg.GetLineItemData(wlItem);
				ReplaceLineItem(wlItem, pRow);
			}
		}
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnEditRequestedItem");
}

void CWaitingListEntryDlg::OnOK() 
{
	try {
		if (m_WaitListID > 0) {
			if (!UpdateData()) {
				return;
			}
		} else {
			if (!SaveNewData()) {
				return;
			}
		}
	} NxCatchAll("Error In: CWaitingListEntryDlg::OnOK");
	
	CDialog::OnOK();
}

void CWaitingListEntryDlg::OnCancel() 
{
	CDialog::OnCancel();
}


void CWaitingListEntryDlg::LoadPurposeDropdown(long nTypeID)
{
	/// Load purpose values appropriate to the value currently selected in the Type dropdown.

	try {
		CString strWhere;
		if (nTypeID > 0) {
			QueryPurposeData(); // Fills the m_arPurposeIDs array.
			// (c.haag 2008-12-17 17:34) - PLID 32376 - Omit inactive procedures
			strWhere.Format(
				"AptPurposeT.ID IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE AptTypeID = %li) AND ("
				"AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
				"OR AptPurposeT.ID IN (%s))", nTypeID, ArrayAsString(m_arPurposeIDs));
			GetDlgItem(IDC_WL_PURPOSE)->EnableWindow(TRUE);
			m_pApptPurpose->WhereClause = (_bstr_t)strWhere;
			m_pApptPurpose->Requery();
		} else {
			// No type so there can't be a purpose either.
			GetDlgItem(IDC_WL_PURPOSE)->EnableWindow(FALSE);
			m_pApptPurpose->Clear();
			m_arPurposeIDs.RemoveAll();
		}
		
	} NxCatchAll("Eorror loading Purpose drop down.");
}

void CWaitingListEntryDlg::QueryData()
{
	// Set all controls from database values.
	
	if (m_WaitListID <= 0) {
		return;
	}

	try {

		_RecordsetPtr prs = CreateRecordset(
			"SELECT AppointmentID, PatientID, TypeID, CreatedDate, Notes "
			"FROM WaitingListT WHERE ID = %li", m_WaitListID);

		if (!prs->eof) {
			FieldsPtr flds = prs->Fields;
			
			// Appointment button
			m_nAppointmentID = AdoFldLong(flds, "AppointmentID", -1);
			
			// Notes field
			SetDlgItemText(IDC_WAIT_LIST_NOTES, AdoFldString(flds, "Notes",""));

			// Date entered
			COleDateTime dtCreatedDate = AdoFldDateTime(flds, "CreatedDate");
			SetDlgItemText(IDC_DATE_ENTERED, FormatDateTimeForInterface(dtCreatedDate));
			
			// Patient dropdown
			m_nPatientID = AdoFldLong(flds, "PatientID", -1);
			m_pPatient->SetSelByColumn(ewlpcID, m_nPatientID);
			
			// Appointment type dropdown.
			long nApptTypeID = AdoFldLong(flds, "TypeID", -1);
			//TES 3/26/2008 - PLID 29426 - Renamed this variable from m_nInnitialApptTypeID
			m_nInitialApptTypeID = nApptTypeID;
			m_pApptType->SetSelByColumn(ewlatcID, nApptTypeID);
			
			// Purpose dropdown.
			LoadPurposeDropdown(nApptTypeID);

			// Resource line item entries.
			QueryLineItemData();
		}

	} NxCatchAll("CWaitingListEntryDlg::QueryData");
}

void CWaitingListEntryDlg::SetDefaultValues()
{
	// Set all controls with basic defaults.
	
	// Patient Name.
	m_nPatientID = GetActivePatientID();
	m_pPatient->SetSelByColumn(ewlpcID, m_nPatientID);

	// Created date.
	COleDateTime dtToday = COleDateTime::GetCurrentTime();
	// (c.haag 2008-10-31 09:06) - PLID 31856 - Use international-compliant formatting
	SetDlgItemText(IDC_DATE_ENTERED, FormatDateTimeForInterface(dtToday, 0, dtoDate));
}

bool CWaitingListEntryDlg::SaveNewData()
{
	// Create a new Waiting List item entry in the database.
	
	try {

		// Get and validate the data from all of the dropdown boxes and other fields.
		WaitListEntry wlData;
		if (!GetFormData(wlData)) {
			return false; // Data was not complete.
		}
		
		// It is easier to use these ID values as CStrings in the Query because 
		//  they may need to be replaced with 'NULL' in some cases.
		CString strApptID, strTypeID;
		strApptID.Format("%li", m_nAppointmentID);
		strTypeID.Format("%li", wlData.nApptType);

		// Set the created date to today.
		// (c.haag 2008-10-31 09:09) - PLID 31856 - Because we're about to use this actual string
		// in a query, we need to format it using an international-compliant function
		wlData.strCreatedDate = FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate);
		
		// Since this is a new Waiting List entry create a variable
		//  to store the ID value of the new record when it is created.
		CString strIDVar = "DECLARE @Wait_List_ID INT; \r\n";
		
		// Create the main query for inserting the Wait List data.
		CString strQuery;
		strQuery.Format(
			"INSERT INTO WaitingListT (AppointmentID, PatientID, TypeID, CreatedDate, Notes) "
			"VALUES (%s, %li, %s, '%s', '%s'); \r\n", 
			(m_nAppointmentID <= 0)? "NULL" : strApptID, 
			wlData.nPatientID, 
			(wlData.nApptType <= 0)? "NULL" : strTypeID, 
			_Q(wlData.strCreatedDate), 
			_Q(wlData.strNotes));
		
		// Add a query to capture the new ID for the record.
		strQuery += "SET @Wait_List_ID = @@IDENTITY; \r\n";

		// Build the query for purpose values.
		CString strPurposeQuery = 
			"DELETE FROM WaitingListPurposeT WHERE WaitingListID=@Wait_List_ID; \r\n"
			+ BuildPurposeTableInsertQuery();

		// Put the final queries together.
		strQuery = strIDVar + strQuery + strPurposeQuery + BuildLineItemSaveQuerys();
		
		// Add a final query to retrieve the new record ID value.
		strQuery = 
			"SET NOCOUNT ON; \r\n"
			+ strQuery + 
			"SET NOCOUNT OFF; \r\n"
			"SELECT @Wait_List_ID AS NewID;";

		// Save the data and attempt to get the new ID value.
		_RecordsetPtr prs = CreateRecordsetStd(strQuery);
		if (!prs->eof) {
			FieldsPtr flds = prs->Fields;
			m_WaitListID = AdoFldLong(flds, "NewID", 0);
		}
		
		return true;

	} NxCatchAll("Error in CWaitingListEntryDlg::SaveNewData");

	return false;
}

bool CWaitingListEntryDlg::UpdateData()
{
	// Update the database entry for an existing Waiting List item.
	
	try {

		// Get and validate the data from all of the dropdown boxes and other fields.
		WaitListEntry wlData;
		if (!GetFormData(wlData)) {
			return false; // Data was not complete.
		}
		
		// It is easier to use these ID values as CStrings in the Query because 
		//  they may need to be replaced with 'NULL' in some cases.
		CString strApptID, strTypeID;
		strApptID.Format("%li", m_nAppointmentID);
		strTypeID.Format("%li", wlData.nApptType);
		
		// Since this is a pre-existing Waiting List entry create a variable
		//  to store its ID value. This allows the BuildLineItemInsertQuery()
		//  function to be shared by both this function and SaveNewData().
		CString strIDVar;
		strIDVar.Format(
			"DECLARE @Wait_List_ID INT; \r\n"
			"SET @Wait_List_ID = %li; \r\n",
			m_WaitListID);
		
		// Create the main query for updating the Wait List data.
		CString strQuery;
		strQuery.Format(
			"UPDATE WaitingListT SET "
				"AppointmentID=%s, "
				"PatientID=%li, "
				"TypeID=%s, "
				"CreatedDate='%s', "
				"Notes='%s' "
			"WHERE ID=@Wait_List_ID; \r\n", 
			(m_nAppointmentID <= 0)? "NULL" : strApptID, 
			wlData.nPatientID, 
			(wlData.nApptType <= 0)? "NULL" : strTypeID, 
			_Q(wlData.strCreatedDate), 
			_Q(wlData.strNotes));


		// Build the query for purpose values.
		CString strPurposeQuery = 
			"DELETE FROM WaitingListPurposeT WHERE WaitingListID=@Wait_List_ID; \r\n"
			+ BuildPurposeTableInsertQuery();

		// Put the final queries together.
		strQuery = 
			"SET NOCOUNT ON; \r\n" + 
			strIDVar + 
			strQuery + 
			strPurposeQuery + 
			BuildLineItemSaveQuerys() +
			"SET NOCOUNT OFF; \r\n";
		
		ExecuteSqlStd(strQuery);

		return true;

	} NxCatchAll("Error in CWaitingListEntryDlg::UpdateData");
	return false;
}

bool CWaitingListEntryDlg::GetFormData(WaitListEntry &wlData)
{
	// Get data from all of the dropdown boxes and other fields.
	//  If data is missing then the user is prompted and NULL is returned.
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	// Patient ID
	wlData.nPatientID = 0;
	pRow = m_pPatient->CurSel;
	if (pRow != NULL) {
		wlData.nPatientID = VarLong(pRow->GetValue(ewlpcID), -1);	
	}
	if (wlData.nPatientID <= 0 && wlData.nPatientID != -25) {
		MessageBox("You must select a patient.");
		return false;
	}
	
	// Appointment type ID.
	wlData.nApptType = 0;
	pRow = m_pApptType->CurSel;
	if (pRow != NULL) {
		wlData.nApptType = VarLong(pRow->GetValue(ewlatcID), -1);	
	}
	if (wlData.nApptType <= 0 && m_nAppointmentID <= 0) {
		MessageBox("You must select an appointment type.");
		return false;
	}

	// Request line item list check.
	if (m_pRequestItems->GetRowCount() <= 0) {
		MessageBox("You must create at least one request.");
		return false;
	}

	// Created Date
	GetDlgItemText(IDC_DATE_ENTERED, wlData.strCreatedDate);
	// (c.haag 2008-10-31 09:26) - PLID 31856 - This string is used to form
	// SQL statements. Try to make sure it's in the proper SQL format.
	COleDateTime dt;
	dt.ParseDateTime(wlData.strCreatedDate);
	if (dt.GetStatus() == COleDateTime::valid) {
		wlData.strCreatedDate = FormatDateTimeForSql(dt, dtoDate);
	} else {
		// This shouldn't happen; it means the COleDateTime object couldn't even
		// parse what should be properly formatted display text. Just leave
		// strCreatedDate alone; maybe the SQL Server can interpret it.
	}

	// Notes
	GetDlgItemText(IDC_WAIT_LIST_NOTES, wlData.strNotes);

	return true;
}

CString CWaitingListEntryDlg::BuildLineItemSaveQuerys()
{
	// Build a batch SQL query for adding all new line item
	//  entries to the database in the same connection.
	// @Wait_List_ID is set to the ID value of the main entry in the 
	//  WaitingListT table that all other records are associated with.
	
	// Build a statement to delete any rows that have been droped.
	CString strLineItemIDs, strID;


	// This SQL variable is used when adding resources and days.
	CString strQuery = "DECLARE @Wait_List_Item_ID INT; \r\n";
	
	// The m_arLineItems array stores a WaitListLineItem pointer for
	//  every new line item that has been added.
	WaitListLineItem wlItem;
	for (long i = 0; i < m_arLineItems.GetSize(); i++) {
		wlItem = m_arLineItems[i];
		if (wlItem.nLineItemID <= 0) {
			// New unsaved item.
			CString strLineItemQuery;
			// Add the main query;
			strLineItemQuery.Format(
				"INSERT INTO WaitingListItemT "
					"(WaitingListID, StartDate, EndDate, StartTime, EndTime, AllResources) "
				"VALUES (@Wait_List_ID, '%s', '%s', '%s', '%s', %d); \r\n", 
				// (c.haag 2008-10-31 09:11) - PLID 31856 - Use international formatting
				FormatDateTimeForSql(wlItem.dtStartDate, dtoDate), 
				FormatDateTimeForSql(wlItem.dtEndDate, dtoDate), 
				FormatDateTimeForSql(wlItem.dtStartTime, dtoTime), 
				FormatDateTimeForSql(wlItem.dtEndTime, dtoTime), 
				(wlItem.bAllResources)?1:0);
				
			// Get the ID value for the record just added.
			strLineItemQuery += "SET @Wait_List_Item_ID = @@IDENTITY; \r\n";
			
			// Add an insert query for each resource.
			CString strResourceQuery;
			long nNumIDs = wlItem.arResourceIDs.GetSize();
			for (long j = 0; j < nNumIDs; j++) {
				strResourceQuery.Format(
					"INSERT INTO WaitingListItemResourceT (ItemID, ResourceID) "
					"VALUES (@Wait_List_Item_ID, %li); \r\n", 
					wlItem.arResourceIDs[j]);
				strLineItemQuery += strResourceQuery;
			}

			// Add an insert query for each day.
			CString strDayQuery;
			nNumIDs = wlItem.arDayIDs.GetSize();
			for (long k = 0; k < nNumIDs; k++) {
				strDayQuery.Format(
					"INSERT INTO WaitingListItemDaysT (ItemID, DayOfWeekNum) "
					"VALUES (@Wait_List_Item_ID, %li); \r\n", 
					wlItem.arDayIDs[k]);
				strLineItemQuery += strDayQuery;
			}

			strQuery += strLineItemQuery;
		}
		else {
			// Collect all of the valid IDs, these are used later to determine rows that
			// need to be deleted from the database (dropped rows).
			strID.Format("%li, ", wlItem.nLineItemID);
			strLineItemIDs += strID;

			if (wlItem.bModified) {
			// Previous item already in the database, but with changed data.
				CString strUpdateQuery;
				strUpdateQuery.Format(
					"UPDATE WaitingListItemT SET "
						"StartDate='%s', EndDate='%s', "
						"StartTime='%s', EndTime='%s', "
						"AllResources=%d "
					"WHERE ID=%li;\r\n", 
					// (c.haag 2008-10-31 09:12) - PLID 31856 - Use our SQL format functions
					FormatDateTimeForSql(wlItem.dtStartDate, dtoDate), 
					FormatDateTimeForSql(wlItem.dtEndDate, dtoDate), 
					FormatDateTimeForSql(wlItem.dtStartTime, dtoTime), 
					FormatDateTimeForSql(wlItem.dtEndTime, dtoTime), 
					(wlItem.bAllResources)?1:0, 
					wlItem.nLineItemID);
				
				strQuery += strUpdateQuery 
					+ GetResourceQueryString(wlItem) 
					+ GetDayQueryString(wlItem);
			}
		}
	}
	
	// Add a query to delete any rows that have been dropped.
	strLineItemIDs = strLineItemIDs.Left(strLineItemIDs.GetLength()-2);
	if (strLineItemIDs.GetLength() > 0) {
		CString strDelete;
		strDelete.Format(
			"DELETE FROM WaitingListItemResourceT "
			"WHERE ItemID NOT IN (%s) "
			"AND ItemID IN "
				"(SELECT WaitingListItemT.ID "
				"FROM WaitingListItemT INNER JOIN "
				"WaitingListT ON WaitingListItemT.WaitingListID = WaitingListT.ID "
				"WHERE WaitingListT.ID = @Wait_List_ID);\r\n"
			"DELETE FROM WaitingListItemDaysT WHERE ItemID NOT IN (%s) "
			"AND ItemID IN "
				"(SELECT WaitingListItemT.ID "
				"FROM WaitingListItemT INNER JOIN "
				"WaitingListT ON WaitingListItemT.WaitingListID = WaitingListT.ID "
				"WHERE WaitingListT.ID = @Wait_List_ID);\r\n"
			"DELETE FROM WaitingListItemT WHERE ID NOT IN (%s) "
			"AND WaitingListID = @Wait_List_ID;\r\n", 
			strLineItemIDs, strLineItemIDs, strLineItemIDs);
		strQuery = strDelete + strQuery;
	} else {
		// There were no id values left in the list, this may possibly mean that
		//  every line item was dropped at some point.
		CString strDelete = 
			"DELETE FROM WaitingListItemResourceT "
			"WHERE ItemID IN "
				"(SELECT WaitingListItemT.ID "
				"FROM WaitingListItemT INNER JOIN "
				"WaitingListT ON WaitingListItemT.WaitingListID = WaitingListT.ID "
				"WHERE WaitingListT.ID = @Wait_List_ID);\r\n"
			"DELETE FROM WaitingListItemDaysT WHERE ItemID IN "
				"(SELECT WaitingListItemT.ID "
				"FROM WaitingListItemT INNER JOIN "
				"WaitingListT ON WaitingListItemT.WaitingListID = WaitingListT.ID "
				"WHERE WaitingListT.ID = @Wait_List_ID);\r\n"
			"DELETE FROM WaitingListItemT WHERE WaitingListID = @Wait_List_ID;\r\n";
		strQuery = strDelete + strQuery;
	}

	return strQuery;
}

CString CWaitingListEntryDlg::GetResourceQueryString(const WaitListLineItem &wlItem)
{
	// This function should usually only be called from UpdateData().
	long nResourceCount = wlItem.arResourceIDs.GetSize();
	
	// Query to delete all previous entries.
	CString strDeleteQuery;
	strDeleteQuery.Format(
		"DELETE FROM WaitingListItemResourceT "
		"WHERE ItemID=%li; \r\n", wlItem.nLineItemID);

	// Build the queries to insert each new value.
	CString strInsertQueries;
	CString strQuery;
	for (long i = 0; i < nResourceCount; i++) {
		strQuery.Format(
			"INSERT INTO WaitingListItemResourceT (ItemID, ResourceID) "
				"VALUES (%li, %li); ", wlItem.nLineItemID, wlItem.arResourceIDs[i]);
		strInsertQueries += strQuery += "\r\n";
	}
	
	return strDeleteQuery + strInsertQueries;
}


CString CWaitingListEntryDlg::GetDayQueryString(const WaitListLineItem &wlItem)
{
	// This function should usually only be called from UpdateData().
	long nDayCount = wlItem.arDayIDs.GetSize();

	// Query to delete all previous entries.
	CString strDeleteQuery;
	strDeleteQuery.Format(
		"DELETE FROM WaitingListItemDaysT "
		"WHERE ItemID=%li; \r\n", wlItem.nLineItemID);

	// Build the queries to insert each new value.
	CString strInsertQueries;
	CString strQuery;
	for (long i = 0; i < nDayCount; i++) {
		strQuery.Format(
			"INSERT INTO WaitingListItemDaysT (ItemID, DayOfWeekNum) "
				"VALUES (%li, %li); ", wlItem.nLineItemID, wlItem.arDayIDs[i]);
		strInsertQueries += strQuery += "\r\n";
	}
	
	return strDeleteQuery + strInsertQueries;
}

CString CWaitingListEntryDlg::BuildPurposeTableInsertQuery()
{
	// Build a batch SQL query for adding all new appointment 
	//  purpose entries to the database in the same connection.
	// @Wait_List_ID is set to the ID value of the main entry in the 
	//  WaitingListT table that all other records are associated with.
	
	CString strQuery;
	CString strInsert;
	for (long i = 0; i < m_arPurposeIDs.GetSize(); i++) {
		strInsert.Format(
			"INSERT INTO WaitingListPurposeT (WaitingListID, PurposeID) "
			"VALUES (@Wait_List_ID, %li); \r\n", m_arPurposeIDs[i]);
		strQuery += strInsert;
	}
	
	return strQuery;
}

void CWaitingListEntryDlg::QueryPurposeData()
{
	try {

		// Fill the rgPurposeIDs array from the database.
		
		// Empty any previous values from the array.
		m_arPurposeIDs.RemoveAll();

		_RecordsetPtr prs = CreateRecordset(
			"SELECT PurposeID "
			"FROM WaitingListPurposeT "
			"WHERE WaitingListID = %li", m_WaitListID);

		while (!prs->eof) {
			FieldsPtr flds = prs->Fields;
			m_arPurposeIDs.Add(AdoFldLong(flds, "PurposeID", 0));
			prs->MoveNext();
		}
		
	} NxCatchAll("Error in CWaitingListEntryDlg::QueryPurposeData");
}

void CWaitingListEntryDlg::QueryLineItemData()
{
	
	try {

		m_pRequestItems->Clear();
		m_arLineItems.RemoveAll();

		// Get data for every line item.
		_RecordsetPtr prs = CreateRecordset(
			"SELECT ID, StartDate, EndDate, StartTime, EndTime, AllResources "
			"FROM WaitingListItemT WHERE WaitingListID=%li", m_WaitListID);
		
		long nIndex = 0;
		while (!prs->eof) {
			WaitListLineItem wlLineItem;
			wlLineItem.nLineItemID = AdoFldLong(prs, "ID");
			wlLineItem.bAllResources = (AdoFldBool(prs, "AllResources") > 0)?true:false;
			wlLineItem.dtStartDate = AdoFldDateTime(prs, "StartDate");
			wlLineItem.dtEndDate = AdoFldDateTime(prs, "EndDate");
			wlLineItem.dtStartTime = AdoFldDateTime(prs, "StartTime");
			wlLineItem.dtEndTime = AdoFldDateTime(prs, "EndTime");
			wlLineItem.bModified = false;
			wlLineItem.nTempID = nIndex;
			
			m_arLineItems.Add(wlLineItem);
			nIndex++;
			prs->MoveNext();
		}
		
		// Get data for resources associated with line items.
		prs = CreateRecordset(
			"SELECT WaitingListItemT.ID AS LineItemID, "
				"WaitingListItemResourceT.ResourceID AS ResourceID, "
				"ResourceT.Item AS Name "
			"FROM WaitingListT INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"INNER JOIN WaitingListItemResourceT "
				"ON WaitingListItemT.ID = WaitingListItemResourceT.ItemID "
				"INNER JOIN ResourceT "
				"ON WaitingListItemResourceT.ResourceID = ResourceT.ID "
			"WHERE WaitingListT.ID=%li", m_WaitListID);
		
		long nLineItemID;
		while (!prs->eof) {
			// First try to find a matching entry in the rgLineItems array.
			//  There should only ever be a few entries, so just loop and look.
			nLineItemID = AdoFldLong(prs, "LineItemID", 0);
			for (long i = 0; i < m_arLineItems.GetSize(); i++) {
				if (m_arLineItems[i].nLineItemID == nLineItemID)
				{
					m_arLineItems[i].arResourceIDs.Add(AdoFldLong(prs, "ResourceID", 0));
					m_arLineItems[i].arResourceNames.Add(AdoFldString(prs, "Name", ""));
					break;
				}
			}
			prs->MoveNext();
		}

		// Get all of the days associated with each line item.
		prs = CreateRecordset(
			"SELECT WaitingListItemT.ID AS LineItemID, "
				"WaitingListItemDaysT.DayOfWeekNum AS DayID "
			"FROM WaitingListT INNER JOIN WaitingListItemT "
				"ON WaitingListT.ID = WaitingListItemT.WaitingListID "
				"INNER JOIN WaitingListItemDaysT "
				"ON WaitingListItemT.ID = WaitingListItemDaysT.ItemID "
			"WHERE WaitingListT.ID=%li "
			"ORDER BY WaitingListItemDaysT.DayOfWeekNum", m_WaitListID);
		
		CString strDayName;
		long nDayID;
		while (!prs->eof) {
			// Again, try to find a match for the line item in the array.
			nLineItemID = AdoFldLong(prs, "LineItemID", 0);
			for (long i = 0; i < m_arLineItems.GetSize(); i++) {
				if (m_arLineItems[i].nLineItemID == nLineItemID)
				{
					nDayID = AdoFldLong(prs, "DayID", 0);
					m_arLineItems[i].arDayIDs.Add(nDayID);
					m_arLineItems[i].arDayNames.Add(CWaitingListUtils::GetDayName(nDayID));
					break;
				}
			}
			prs->MoveNext();
		}

		NXDATALIST2Lib::IRowSettingsPtr pNewRow;
		CString strLineItem;

		for (long j = 0; j < m_arLineItems.GetSize(); j++) {
			strLineItem = CWaitingListUtils::FormatLineItem(m_arLineItems[j]);

			pNewRow = m_pRequestItems->GetNewRow();
			pNewRow->PutValue(ewlrlcID, m_arLineItems[j].nLineItemID);
			pNewRow->PutValue(ewlrlcText, (_variant_t)strLineItem);
			pNewRow->PutValue(ewlrlcTempID, m_arLineItems[j].nTempID);
			m_pRequestItems->AddRowAtEnd(pNewRow, NULL);
		}

	} NxCatchAll("Error in CWaitingListEntryDlg::QueryLineItemData");
}

void CWaitingListEntryDlg::AddNewLineItemData(WaitListLineItem &wlItem)
{
	// Adds a new line item to the list but does not actually update the database.
	
	// Add the line item to the array.
	wlItem.nLineItemID = 0;
	long nTempID = 0;
	if (m_arLineItems.GetSize() > 0) {
		nTempID = m_arLineItems[m_arLineItems.GetSize()-1].nTempID + 1;
	}
	wlItem.nTempID = nTempID;
	m_arLineItems.Add(wlItem);

	// Add the line item to the display list.
	CString strLineItem = CWaitingListUtils::FormatLineItem(wlItem);
	
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pRequestItems->GetNewRow();
	pNewRow->PutValue(ewlrlcID, _variant_t((long)0));
	pNewRow->PutValue(ewlrlcText, (_variant_t)strLineItem);
	pNewRow->PutValue(ewlrlcTempID, _variant_t(wlItem.nTempID));

	m_pRequestItems->AddRowAtEnd(pNewRow, NULL);
}

void CWaitingListEntryDlg::ReplaceLineItem(const WaitListLineItem &wlItem, 
	NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// Replaces a line item in the list. It does not actually update the database.
	
	// Replace the text for the line item.
	CString strLineItem = CWaitingListUtils::FormatLineItem(wlItem);
	pRow->PutValue(ewlrlcText, (_variant_t)strLineItem);
	// Then replace the array entry for the line item. This is used
	//  later to save the data.
	long nNumItems =  m_arLineItems.GetSize();
	for (long i = 0; i < nNumItems; i++) {
		if (wlItem.nTempID == m_arLineItems[i].nTempID) {
			m_arLineItems[i] = wlItem;
			break;
		}
	}
}

void CWaitingListEntryDlg::DeleteLineItem()
{
	// Drops an item from the list, but does not remove it from the database.
	
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRequestItems->CurSel;
	if (pRow == NULL) {
		return;
	}
	
	try {

		long nTempID = VarLong(pRow->GetValue(ewlrlcTempID), -1);
		for (long i = 0; i < m_arLineItems.GetSize(); i++) {
			if (nTempID == m_arLineItems[i].nTempID) {
				m_arLineItems.RemoveAt(i);
				break;
			}
		}
		m_pRequestItems->RemoveRow(pRow);

	} NxCatchAll("Error in CWaitingListEntryDlg::DeleteLineItem");
}

LRESULT CWaitingListEntryDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
	case IDC_WL_PURPOSE_MULTI_LIST:
		OpenPurposeMultiList();
		break;
	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}

BOOL CWaitingListEntryDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	//TES 3/26/2008 - PLID 29425 - We only need to set the hand cursor if we actually have multiple purposes selected
	// (which would mean that we're showing the hyperlink).
	if(m_arPurposeIDs.GetSize() > 1) {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		CRect rc;
		GetDlgItem(IDC_WL_PURPOSE_MULTI_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
