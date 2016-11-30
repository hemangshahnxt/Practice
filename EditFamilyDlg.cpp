// EditFamilyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditFamilyDlg.h"
#include "AuditTrail.h"
#include "PatientsRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.




/////////////////////////////////////////////////////////////////////////////
//FamilyUtils implementation

namespace FamilyUtils {
	// (a.walling 2006-11-20 12:04) - PLID 22715
	long GetFamilyID(long nPersonID) {
		// return the FamilyID for the given Person, -1 if none.
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT FamilyID FROM PersonFamilyT WHERE PersonID = %li", nPersonID);

		ASSERT(prs->GetRecordCount() <= 1);

		long nFamilyID = -1;

		if (!prs->eof) {
			nFamilyID = AdoFldLong(prs, "FamilyID", -1);
		}
		
		return nFamilyID;
	}

	long CreateNewFamilyAndRelationship(long nPersonID, long nRelativeID)
	{
		// create a new family and relation and return the family id
		long nFamilyID = NewNumber("FamiliesT", "ID");
		ExecuteSql("INSERT INTO FamiliesT(ID) VALUES(%li)", nFamilyID);

		// insert the relative into this family, since e's not already in one
		// don't audit since we take care of it below
		InsertRelationship(nPersonID, nRelativeID, nFamilyID, false);

		// and insert the relative since e has no family either (otherwise we would not have created one!)
		// don't audit since we take care of it below
		InsertRelationship(nRelativeID, nPersonID, nFamilyID, false);

		// (a.walling 2006-11-21 13:29) - PLID 23621 - Audit the new family for both people

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientFamily, nPersonID, "<none>", AsString(nFamilyID), aepMedium, aetCreated);

		nAuditID = BeginNewAuditEvent();
		AuditEvent(nRelativeID, GetExistingPatientName(nRelativeID), nAuditID, aeiPatientFamily, nRelativeID, "<none>", AsString(nFamilyID), aepMedium, aetCreated);

		return nFamilyID;
	}

	void ClearRelationships(long nPersonID) // remove all relationships for this person
	{
		// (a.walling 2006-11-21 13:29) - PLID 23621 - Audit the removal of families
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT FamilyID FROM PersonFamilyT WHERE PersonID = %li", nPersonID);

		if (!prs->eof) {
			ASSERT(prs->GetRecordCount() <= 1);
			long nAuditID = BeginNewAuditEvent();
			long nFamilyID = AdoFldLong(prs, "FamilyID");

			AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientFamily, nPersonID, AsString(nFamilyID), "<none>", aepMedium, aetDeleted);
			
			ExecuteSql("DELETE FROM PersonFamilyT WHERE PersonID = %li", nPersonID);
		}
	}

	void InsertRelationship(long nPersonID, long nRelativeID, long nFamilyID, bool bAudit /* = true */) // Create the relationship between these two for this family
	{
		// (a.walling 2006-11-21 13:29) - PLID 23621 - Audit the changes in families
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT FamilyID FROM PersonFamilyT WHERE PersonID = %li", nPersonID);

		long nAuditID = BeginNewAuditEvent();

		if (!prs->eof) {
			ASSERT(prs->GetRecordCount() <= 1);
		
			long nOldFamilyID = AdoFldLong(prs, "FamilyID");


			if (bAudit)
				AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientFamily, nPersonID, AsString(nOldFamilyID), AsString(nFamilyID), aepMedium, aetChanged);
			
			ExecuteSql("UPDATE PersonFamilyT SET RelativeID = %li, FamilyID = %li WHERE PersonID = %li", 
				nRelativeID, nFamilyID, nPersonID);
		} else {
			if (bAudit)
				AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientFamily, nPersonID, "<none>", AsString(nFamilyID), aepMedium, aetCreated);

			ExecuteSql("INSERT INTO PersonFamilyT(PersonID, RelativeID, FamilyID) VALUES(%li, %li, %li)",
				nPersonID, nRelativeID, nFamilyID);
		}
	}

	void CleanUp() // cleans up all family entries, such as empty and single-person families
	{
		// (a.walling 2006-11-21 13:29) - PLID 23621 - Audit the deletion of single-person families
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT PersonID, FamilyID FROM PersonFamilyT WHERE FamilyID IN (SELECT FamilyID FROM PersonFamilyT GROUP BY FamilyID HAVING COUNT(FamilyID) < 2)");
		if (!prs->eof) {
			while (!prs->eof) {
				long nAuditID = BeginNewAuditEvent();

				long nPersonID = AdoFldLong(prs, "PersonID", -1);
				long nFamilyID = AdoFldLong(prs, "FamilyID", -1);

				if (nPersonID >= 0) {
					AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientFamily, nPersonID, AsString(nFamilyID), "<none>", aepMedium, aetDeleted);
				}
				
				prs->MoveNext();
			}
			// delete entries for families with only 1 person in them
			ExecuteSql("DELETE FROM PersonFamilyT WHERE FamilyID IN (SELECT FamilyID FROM PersonFamilyT GROUP BY FamilyID HAVING COUNT(FamilyID) < 2)");
		}
		// clean up any dangling families that may be empty
		ExecuteSql("DELETE FROM FamiliesT WHERE ID NOT IN (SELECT DISTINCT FamilyID FROM PersonFamilyT)");

		// update any invalid RelativeIDs to next patient in the family
		ExecuteSql("UPDATE PersonFamilyT"
			" SET Auto = 1, RelativeID = (SELECT TOP 1 PersonID FROM PersonFamilyT NewRelativeQ WHERE NewRelativeQ.FamilyID = PersonFamilyT.FamilyID AND NewRelativeQ.PersonID <> PersonFamilyT.PersonID)"
			" WHERE RelativeID NOT IN (SELECT PersonID FROM PersonFamilyT CurrentFamilyQ WHERE CurrentFamilyQ.FamilyID = PersonFamilyT.FamilyID)");
	}

	long CountRelatives(long nPersonID) 
	{
		// (a.walling 2006-11-22 15:18) - PLID 22715 - Return the number of patients who have this patient marked as their immediate relative
		ADODB::_RecordsetPtr prs = CreateRecordset("SELECT COUNT(PersonID) AS Count FROM PersonFamilyT WHERE RelativeID = %li", nPersonID);

		if (prs->eof)
			return 0;
		else
			return AdoFldLong(prs, "Count", 0);
	}

	// (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT - This function was missing NOCOUNT, but was also broken and unreferenced, so removed
	//long UpdateRelatives(long nPersonID, long nNewRelativeID) 
}


/////////////////////////////////////////////////////////////////////////////
// CEditFamilyDlg dialog

// datalist enum

enum EFamilyListColumns {
	flcPersonID = 0,
	flcFamilyID,
	flcName,
	flcUserDefinedID,
	flcBirthdate,
	flcColour,
	flcRelatedNum
};


CEditFamilyDlg::CEditFamilyDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditFamilyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditFamilyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nFamilyID = -1;
	m_nPersonID = -1;
	m_nRelativeID = -1;

	m_btnAdd.AutoSet(NXB_RIGHT);
	m_btnRemove.AutoSet(NXB_LEFT);

}


void CEditFamilyDlg::DoDataExchange(CDataExchange* pDX)
{
	// (a.walling 2008-04-03 10:29) - PLID 29497 - Added NxStatic controls
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditFamilyDlg)
	DDX_Control(pDX, IDC_MESSAGELABEL, m_labelMessage);
	DDX_Control(pDX, IDC_FAMILY_COUNT, m_labelFamilyCount);
	DDX_Control(pDX, IDC_REMOVEPATIENT, m_btnRemove);
	DDX_Control(pDX, IDC_ADDPATIENT, m_btnAdd);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditFamilyDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditFamilyDlg)
	ON_BN_CLICKED(IDC_ADDPATIENT, OnAdd)
	ON_BN_CLICKED(IDC_REMOVEPATIENT, OnRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditFamilyDlg message handlers

BOOL CEditFamilyDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 09:53) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		extern CPracticeApp theApp;
		GetDlgItem(IDC_FAMILY_COUNT)->SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		m_brush.CreateSolidBrush(PaletteColor(0x00FFD8CE));
		
		m_dlFamily = BindNxDataList2Ctrl(this, IDC_FAMILYMEMBERS, GetRemoteData(), false);
		m_dlPatients = BindNxDataList2Ctrl(this, IDC_PATIENTLIST, GetRemoteData(), false);

		m_btnAdd.EnableWindow(false);
		m_btnRemove.EnableWindow(false);

		m_bRemoved = false;
		m_nRelationRemoved = 0;
		m_nRelationRemovedCount = 0;

		if (m_nFamilyID == -1) {
			// no family? load from PersonID
			if (m_nPersonID == -1) {
				ASSERT(FALSE); // at least set ONE of the fields, if not both!
			} else {
				m_nFamilyID = FamilyUtils::GetFamilyID(m_nPersonID);
			}
		}

		ASSERT(m_nFamilyID != -1);
		if (m_nFamilyID == -1) {
			OnCancel();
		}

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_dlFamily->GetColumn(flcColour);
		CString strField;
		COLORREF nRelativeColour = RGB(0, 0, 128);
		COLORREF nPersonColour = RGB(0, 0, 128);
		COLORREF nFamilyColour = RGB(0, 0, 255);
		strField.Format("CASE WHEN PersonFamilyT.PersonID = %li THEN %li WHEN PersonFamilyT.PersonID = %li THEN %li ELSE %li END",
			m_nPersonID, nPersonColour, m_nRelativeID, nRelativeColour, nFamilyColour);
		pCol->PutFieldName((_bstr_t)strField);

		ResetWhereClause();

		// (b.eyers 2013-03-26) - PLID 53984 - Ethnicity, affiliate physician, related to read only permission
		if(!(GetCurrentUserPermissions(bioPatient) & SPT___W________ANDPASS)) {
			GetDlgItem(IDC_ADDPATIENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REMOVEPATIENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_FAMILYMEMBERS)->EnableWindow(FALSE);
			GetDlgItem(IDC_PATIENTLIST)->EnableWindow(FALSE);
		}

		m_dlFamily->Requery();
		m_dlPatients->Requery();
	} NxCatchAll("Error intializing Family dialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditFamilyDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditFamilyDlg)
	ON_EVENT(CEditFamilyDlg, IDC_FAMILYMEMBERS, 18 /* RequeryFinished */, OnRequeryFinishedFamilyMembers, VTS_I2)
	ON_EVENT(CEditFamilyDlg, IDC_PATIENTLIST, 3 /* DblClickCell */, OnDblClickCellPatientList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEditFamilyDlg, IDC_FAMILYMEMBERS, 3 /* DblClickCell */, OnDblClickCellFamilyMembers, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditFamilyDlg::ResetWhereClause()
{
	CString strWhere;
	strWhere.Format("PersonT.ID > 0 AND PatientsT.CurrentStatus <> 4 AND PersonFamilyT.FamilyID = %li", m_nFamilyID);
	m_dlFamily->PutWhereClause((_bstr_t)strWhere);

	strWhere.Format("PersonT.ID > 0 AND PatientsT.CurrentStatus <> 4 AND (PersonFamilyT.FamilyID <> %li OR PersonFamilyT.FamilyID IS NULL)", m_nFamilyID);
	m_dlPatients->PutWhereClause((_bstr_t)strWhere);
}

void CEditFamilyDlg::OnRequeryFinishedFamilyMembers(short nFlags) 
{
	if(!(GetCurrentUserPermissions(bioPatient) & SPT___W________ANDPASS)) {
		m_btnAdd.EnableWindow(false);
		m_btnRemove.EnableWindow(false);
		}
	else {
		m_btnAdd.EnableWindow(true);
		m_btnRemove.EnableWindow(true);
	}

	UpdateLabel();
}

void CEditFamilyDlg::UpdateLabel()
{
	CString strCount;
	strCount.Format("%li members of this family", m_dlFamily->GetRowCount());

	SetDlgItemText(IDC_FAMILY_COUNT, strCount);	

	if (m_bRemoved) {
		SetMessage("Saving will remove the current patient from this family.");
	} else if (m_nRelationRemoved > 0) {
		CString strMsg;
		strMsg.Format("Saving will move %li immediate relative(s) for %li patient(s) out of this family.", m_nRelationRemoved, m_nRelationRemovedCount);
		SetMessage(strMsg);
	} else {
		SetMessage("");
	}
}

void CEditFamilyDlg::OnAdd() 
{
	try {
		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_dlPatients->GetFirstSelRow();
		while (pCurSelRow != NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pPatientRow = pCurSelRow;
			pCurSelRow = pPatientRow->GetNextSelRow();

			NXDATALIST2Lib::IRowSettingsPtr pFamilyRow = m_dlFamily->GetNewRow();	

			long nPersonID = VarLong(pPatientRow->GetValue(flcPersonID));
			long nRelationCount = VarLong(pPatientRow->GetValue(flcRelatedNum), 0);
			long nFamilyID = VarLong(pPatientRow->GetValue(flcFamilyID), -1);
			long nRelatedNum = VarLong(pPatientRow->GetValue(flcRelatedNum), 0);


			if ( (nPersonID == m_nPersonID) && m_bRemoved) {
				m_bRemoved = false;
			}
			
			if ( (nRelationCount > 0) && (nFamilyID == m_nFamilyID) ) {
				m_nRelationRemoved--;
				m_nRelationRemovedCount -= nRelatedNum;
			}

			pFamilyRow->PutValue(flcPersonID, pPatientRow->GetValue(flcPersonID));
			pFamilyRow->PutValue(flcFamilyID, pPatientRow->GetValue(flcFamilyID));
			pFamilyRow->PutValue(flcName, pPatientRow->GetValue(flcName));
			pFamilyRow->PutValue(flcUserDefinedID, pPatientRow->GetValue(flcUserDefinedID));
			pFamilyRow->PutValue(flcBirthdate, pPatientRow->GetValue(flcBirthdate));
			pFamilyRow->PutValue(flcRelatedNum, pPatientRow->GetValue(flcRelatedNum));
			pFamilyRow->PutValue(flcColour, pPatientRow->GetValue(flcColour));
			//You would think the above would work, but it does not.
			pFamilyRow->PutForeColor(VarLong(pPatientRow->GetValue(flcColour), 0));

			POSITION p = m_listRemoveMembers.Find(nPersonID);
			if (p != NULL) {
				m_listRemoveMembers.RemoveAt(p);
			} else {
				m_listAddMembers.AddTail(nPersonID);
			}

			m_dlFamily->AddRowSorted(pFamilyRow, NULL);
			m_dlPatients->RemoveRow(pPatientRow);
		}

		UpdateLabel();
	} NxCatchAll("Error moving patients to family");
}

void CEditFamilyDlg::OnRemove() 
{
	try {
		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_dlFamily->GetFirstSelRow();
		while (pCurSelRow) {
			NXDATALIST2Lib::IRowSettingsPtr pFamilyRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();

			NXDATALIST2Lib::IRowSettingsPtr pPatientRow = m_dlPatients->GetNewRow();

			long nPersonID = VarLong(pFamilyRow->GetValue(flcPersonID));
			long nRelationCount = VarLong(pFamilyRow->GetValue(flcRelatedNum), 0);
			long nFamilyID = VarLong(pFamilyRow->GetValue(flcFamilyID), -1);
			long nRelatedNum = VarLong(pFamilyRow->GetValue(flcRelatedNum), 0);

			if ( (nPersonID == m_nPersonID) ) {
				m_bRemoved = true;
			}
			
			if ( (nRelationCount > 0) && (nFamilyID == m_nFamilyID) ) {
				m_nRelationRemoved++;
				m_nRelationRemovedCount += nRelatedNum;
			}

			pPatientRow->PutValue(flcPersonID, pFamilyRow->GetValue(flcPersonID));
			pPatientRow->PutValue(flcFamilyID, pFamilyRow->GetValue(flcFamilyID));
			pPatientRow->PutValue(flcName, pFamilyRow->GetValue(flcName));
			pPatientRow->PutValue(flcUserDefinedID, pFamilyRow->GetValue(flcUserDefinedID));
			pPatientRow->PutValue(flcBirthdate, pFamilyRow->GetValue(flcBirthdate));
			pPatientRow->PutValue(flcRelatedNum, pFamilyRow->GetValue(flcRelatedNum));
			pPatientRow->PutValue(flcColour, pFamilyRow->GetValue(flcColour));
			//You would think the above would work, but it does not.
			pPatientRow->PutForeColor(VarLong(pFamilyRow->GetValue(flcColour), 0));

			POSITION p = m_listAddMembers.Find(nPersonID);
			if (p != NULL) {
				m_listAddMembers.RemoveAt(p);
			} else {
				m_listRemoveMembers.AddTail(nPersonID);
			}

			m_dlPatients->AddRowSorted(pPatientRow, NULL);
			m_dlFamily->RemoveRow(pFamilyRow);
		}	

		UpdateLabel();
	} NxCatchAll("Error moving patients from family");
}

void CEditFamilyDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CEditFamilyDlg::OnOK() 
{
	if(Save())
		CDialog::OnOK();
}

bool CEditFamilyDlg::Save()
{
	try {
		ASSERT(m_nPersonID != -1); // we MUST have a relation!

		long nBrokenRelationships = CalculateBrokenRelationships();

		if (nBrokenRelationships > 0) {
			CString strMessage;
			strMessage.Format("%li patients have had their immediate relative removed from their family.\r\n\r\n"
				"Their immediate relative will be updated to another member of the family, and the family "
				"will be deleted if it contains only 1 member. Would you like to continue?", nBrokenRelationships);
			if (nBrokenRelationships == 1) {
				strMessage.Replace("patients", "patient");
				strMessage.Replace("have", "has");
			}
			long nResult = MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION | MB_YESNO);

			if (nResult == IDNO)
				return false;
		}

		POSITION p = m_listRemoveMembers.GetHeadPosition();
		//TES 11/5/2007 - PLID 27978 - VS 2008 - for() loops
		int i = 0;
		for (i = 0; i < m_listRemoveMembers.GetCount(); i++) {
			long nRemovedPersonID = m_listRemoveMembers.GetNext(p);
			FamilyUtils::ClearRelationships(nRemovedPersonID);
		}
		
		p = m_listAddMembers.GetHeadPosition();
		for (i = 0; i < m_listAddMembers.GetCount(); i++)
		{
			long nSelectedPersonID = m_listAddMembers.GetNext(p);
			
			FamilyUtils::InsertRelationship(nSelectedPersonID, m_nPersonID, m_nFamilyID);
		}

		FamilyUtils::CleanUp();

		return true;
	} NxCatchAll("Error saving family changes");

	return false;
}

void CEditFamilyDlg::SetMessage(CString strMessage) 
{
	SetDlgItemText(IDC_MESSAGELABEL, strMessage);
}

long CEditFamilyDlg::CalculateBrokenRelationships()
{
	long nTotalBroken = 0;

	POSITION p = m_listRemoveMembers.GetHeadPosition();
	//TES 11/5/2007 - PLID 27978 - VS 2008 - for() loops
	int i = 0;
	for (i = 0; i < m_listRemoveMembers.GetCount(); i++) {
		long nRemovedPersonID = m_listRemoveMembers.GetNext(p);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlPatients->FindByColumn(flcPersonID, nRemovedPersonID, NULL, false);
		if (pRow) {
			nTotalBroken += VarLong(pRow->GetValue(flcRelatedNum), 0);
		}
	}
	
	p = m_listAddMembers.GetHeadPosition();
	for (i = 0; i < m_listAddMembers.GetCount(); i++)
	{
		long nSelectedPersonID = m_listAddMembers.GetNext(p);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlFamily->FindByColumn(flcPersonID, nSelectedPersonID, NULL, false);
		if (pRow) {
			nTotalBroken += VarLong(pRow->GetValue(flcRelatedNum), 0);
		}
	}

	return nTotalBroken;
}

void CEditFamilyDlg::OnDblClickCellPatientList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (!pRow->IsHighlighted()) {
			return; // return if this row is not selected
		}

		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_dlPatients->GetFirstSelRow();
		if (pCurSelRow != NULL) { // there is at least one selection... are there two?
			if (pCurSelRow->GetNextSelRow() != NULL) { // this means there are multiple selections
				return;
			}
			else { // exactly one 
				OnAdd(); // the function looks at the cursel, so that should equal lpRow
			}
		}
	} NxCatchAll("Error in OnDblClickCellPatientList");
	return;
}

void CEditFamilyDlg::OnDblClickCellFamilyMembers(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (!pRow->IsHighlighted()) {
			return; // return if this row is not selected
		}

		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_dlFamily->GetFirstSelRow();
		if (pCurSelRow != NULL) { // there is at least one selection... are there two?
			if (pCurSelRow->GetNextSelRow() != NULL) { // this means there are multiple selections
				return;
			}
			else { // exactly one 
				OnRemove(); // the function looks at the cursel, so that should equal lpRow
			}
		}
	} NxCatchAll("Error in OnDblClickCellFamilyMembers");
	return;
}
