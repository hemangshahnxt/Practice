// InactivateMultiplePatientsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "InactivateMultiplePatientsDlg.h"
#include "FilterEditDlg.h"
#include "groups.h"
#include "filter.h"
#include "Internationalutils.h"
#include "GlobalUtils.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_INACTIVATE_PATS_REMOVE 42358

#define COLUMN_PERSON_ID		0
#define COLUMN_USER_DEFINED_ID	1
#define COLUMN_PATIENT_NAME		2
#define COLUMN_LAST_APPT_DATE	3
#define COLUMN_LAST_BILL_DATE	4
#define COLUMN_LAST_NOTE_DATE	5
#define COLUMN_LAST_MODIFIED_DATE	6

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInactivateMultiplePatientsDlg dialog


CInactivateMultiplePatientsDlg::CInactivateMultiplePatientsDlg(CWnd* pParent)
	: CNxDialog(CInactivateMultiplePatientsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInactivateMultiplePatientsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInactivateMultiplePatientsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInactivateMultiplePatientsDlg)
	DDX_Control(pDX, IDC_REMIND_MONTHLY, m_btnRemindMonthly);
	DDX_Control(pDX, IDC_PATS_IN_LIST, m_nxeditPatsInList);
	DDX_Control(pDX, IDC_ADD_TO_INACTIVATE_LIST, m_btnAddToInactiveList);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EDIT_FILTER_LIST, m_btnEditFilterList);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CInactivateMultiplePatientsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInactivateMultiplePatientsDlg)
	ON_BN_CLICKED(IDC_EDIT_FILTER_LIST, OnEditFilter)
	ON_COMMAND(ID_INACTIVATE_PATS_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_ADD_TO_INACTIVATE_LIST, OnAddToInactivateList)
	ON_BN_CLICKED(IDC_REMIND_MONTHLY, OnRemindMonthly)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInactivateMultiplePatientsDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CInactivateMultiplePatientsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInactivateMultiplePatientsDlg)
	ON_EVENT(CInactivateMultiplePatientsDlg, IDC_PATS_TO_INACTIVATE_LIST, 7 /* RButtonUp */, OnRButtonUpPatsToInactivateList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInactivateMultiplePatientsDlg, IDC_FILTER_LIST, 16 /* SelChosen */, OnSelChosenFilterList, VTS_I4)
	ON_EVENT(CInactivateMultiplePatientsDlg, IDC_FILTER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedFilterList, VTS_I2)
	ON_EVENT(CInactivateMultiplePatientsDlg, IDC_USER_REMIND_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUserRemindList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInactivateMultiplePatientsDlg::OnRButtonUpPatsToInactivateList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_pInactivateList->CurSel = nRow;

	CMenu menPopup;
	menPopup.m_hMenu = CreatePopupMenu();

	if (nRow != -1) {
		menPopup.InsertMenu(1, MF_BYPOSITION, ID_INACTIVATE_PATS_REMOVE, "Remove");
	}

	CPoint pt(x,y);
	CWnd* pWnd = GetDlgItem(IDC_PATS_TO_INACTIVATE_LIST);
	if (pWnd != NULL)
	{	pWnd->ClientToScreen(&pt);
		menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
	else HandleException(NULL, "An error ocurred while creating menu");

	
}

BOOL CInactivateMultiplePatientsDlg::ValidateData() {

	//all we really have to check is that if they checked the box to remind them, that they also picked a username
	if (IsDlgButtonChecked(IDC_REMIND_MONTHLY)) {

		//they checked the box, so make sure they picked a user
		if (m_pUserList->CurSel == -1) {
			MsgBox("Please choose a username to remind");
			return FALSE;
		}
	}

	return TRUE;

}
void CInactivateMultiplePatientsDlg::SaveRemindSettings() {

	try {
		if (m_pUserList->CurSel == -1) {
			SetRemotePropertyInt("InactivatePatientsUser", -1, 0, "<None>");
		}
		else {
			SetRemotePropertyInt("InactivatePatientsUser", VarLong(m_pUserList->GetValue(m_pUserList->CurSel, 0), -1), 0, "<None>");
		}
		if (m_pIntervalList->CurSel == -1) {
			SetRemotePropertyInt("InactivatePatientsInterval", -1, 0, "<None>");
		}
		else {
			SetRemotePropertyInt("InactivatePatientsInterval", VarLong(m_pIntervalList->GetValue(m_pIntervalList->CurSel, 0), -1), 0, "<None>");
		}
		
		SetRemotePropertyInt("InactivatePatientsRemind", IsDlgButtonChecked(IDC_REMIND_MONTHLY), 0, "<None>");
	}NxCatchAll("Error setting reminder settings");


}
void CInactivateMultiplePatientsDlg::InactivatePatients() {


	try {
		//if we got here, they want to do it
		//lets start a transaction
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("InactivatePatients");
		trans.Begin();

		CStringArray aryFieldNames;
		CStringArray aryFieldTypes;
		CString strIn;
			
		//Limit the size of a single XML statement, otherwise it can cause errors.
		CStringArray saXMLStatements;
			
		long p = m_pInactivateList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		if (p) {
			//start the XML
			strIn = "<ROOT>";
		}
		while (p) {
			CString str;
			m_pInactivateList->GetNextRowEnum(&p, &lpDisp);
			IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
			str.Format("<P ID=\"%li\"/>", VarLong(pRow->GetValue(0)));			
			strIn += str;
			if(strIn.GetLength() > 2000) {
				//End this statement.
				strIn += "</ROOT>";
				saXMLStatements.Add(strIn);
				//Start a new one.
				strIn = "<ROOT>";
			}
		}

		if (!strIn.IsEmpty()) {
			//now add the trailer
			strIn += "</ROOT>";
			saXMLStatements.Add(strIn);
		}

		aryFieldNames.Add("ID");
		aryFieldTypes.Add("int");

		//now create our temp table
		CString strTempT = CreateTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(0));
		for(int i = 1; i < saXMLStatements.GetSize(); i++) {
			AppendToTempTableFromXML(aryFieldNames, aryFieldTypes, saXMLStatements.GetAt(i), strTempT);
		}

		//now that we have our temp table, we can run our query
		ExecuteSql("Update PersonT SET Archived = 1 WHERE ID IN (SELECT ID FROM %s)", strTempT);

		trans.Commit();

		//if we made it here, we are all good
		MsgBox("Patients Inactivated Successfully!");

	}NxCatchAll("Error Inactivating Patients");

}

void CInactivateMultiplePatientsDlg::OnOK() 
{
	if (ValidateData()) {
		
		//save the remind settings
		SaveRemindSettings();

		if (m_pInactivateList->GetRowCount() > 0) {
			//first, lets make sure they want to do this
			if (IDNO == MessageBox("This will inactivate all the patients in the list. Are you sure you wish to do this?", "Practice", MB_YESNO)) {
				return;
			}
			InactivatePatients();
		}
	}
	
	CDialog::OnOK();
}

void CInactivateMultiplePatientsDlg::OnEditFilter() 
{
	long nCurSel = m_pFilterList->CurSel;

	if (nCurSel == -1 || VarLong(m_pFilterList->GetValue(nCurSel, 0)) == -1) {
		return;
	}
	
	//check their permissions and their license
	if (!CheckCurrentUserPermissions(bioLWFilter, sptWrite)) {
		return;
	}

	if (!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse)) {
			return;
	}

	//open up the filter dialog
	//we only support patient filters for this
	CFilterEditDlg dlg(NULL, 1, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
	long nResult;
	nResult = dlg.EditFilter(VarLong(m_pFilterList->GetValue(nCurSel, 0)));

	if (nResult == 1) {
		//they clicked OK
		long nID = dlg.GetFilterId();
	
		m_pFilterList->Requery();
		m_pFilterList->SetSelByColumn(0, nID);
		
	}
	
	
}

void CInactivateMultiplePatientsDlg::OnSelChosenFilterList(long nRow) 
{

	long nValue = VarLong(m_pFilterList->GetValue(nRow, 0));

	if (nValue == -1) {
		if (!AddNewFilter()) {
			
			//set the list to not have anything selected and grey out the buttons
			GetDlgItem(IDC_EDIT_FILTER_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADD_TO_INACTIVATE_LIST)->EnableWindow(FALSE);
		}
	}
	else {
		GetDlgItem(IDC_EDIT_FILTER_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADD_TO_INACTIVATE_LIST)->EnableWindow(TRUE);
	}

}

void CInactivateMultiplePatientsDlg::OnRequeryFinishedFilterList(short nFlags) 
{
	//add the {New Filter} 
	IRowSettingsPtr pRow = m_pFilterList->GetRow(-1);

	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _variant_t("{New Filter}"));
	pRow->PutValue(2, _variant_t(""));

	m_pFilterList->InsertRow(pRow, 0);
	
}

BOOL CInactivateMultiplePatientsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 11:10) - PLID 29863 - NxIconify the buttons
		m_btnAddToInactiveList.AutoSet(NXB_NEW);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pFilterList = BindNxDataListCtrl(IDC_FILTER_LIST, true);
		m_pInactivateList = BindNxDataListCtrl(IDC_PATS_TO_INACTIVATE_LIST, false);
		m_pIntervalList = BindNxDataListCtrl(IDC_INTERVAL_LIST, true);
		m_pUserList = BindNxDataListCtrl(IDC_USER_REMIND_LIST, true);
		SetDlgItemInt(IDC_PATS_IN_LIST, 0);

		//set the interval, defaulting to month if they don't have one set

		/*1 = day
		2 = week
		3 = month
		4 = year these can't change since they are stored in data, if add to the list, make the IDs greater*/
		long nInterval = GetRemotePropertyInt("InactivatePatientsInterval", 3, 0, "<None>", true);
		if (nInterval == -1) {
			//this shouldn't happen but just in case the last time they saved they managed to not choose something
			nInterval = 3;
		}
		//set the setting
		m_pIntervalList->SetSelByColumn(0, nInterval);

		long nUser = GetRemotePropertyInt("InactivatePatientsUser", -1, 0, "<None>", true);
		m_pUserList->SetSelByColumn(0, nUser);

		//gray out the selections if the box isn't checked
		if (GetRemotePropertyInt("InactivatePatientsRemind", 0, 0, "<None>", true) != 0) {
			GetDlgItem(IDC_INTERVAL_LIST)->EnableWindow(TRUE);
			GetDlgItem(IDC_USER_REMIND_LIST)->EnableWindow(TRUE);
			CheckDlgButton(IDC_REMIND_MONTHLY, 1);
		}
		else {
			//gray out the selections
			GetDlgItem(IDC_INTERVAL_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_USER_REMIND_LIST)->EnableWindow(FALSE);
			CheckDlgButton(IDC_REMIND_MONTHLY, 0);
		}

		m_brush.CreateSolidBrush(PaletteColor(0x00FFCC99));
	}
	NxCatchAll("Error in CInactivateMultiplePatientsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInactivateMultiplePatientsDlg::OnRemove() {

	m_pInactivateList->RemoveRow(m_pInactivateList->CurSel);
}

BOOL CInactivateMultiplePatientsDlg::AddNewFilter() {
	if (!CheckCurrentUserPermissions(bioLWFilter, sptWrite))
		return FALSE;
		
	CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
	long nResult;
	nResult = dlg.DoModal();

	if (nResult == 1) {
		//they clicked OK
		long nID = dlg.GetFilterId();
	
		m_pFilterList->Requery();
		m_pFilterList->SetSelByColumn(0, nID);
				
		return TRUE;
	}

	return FALSE;	

}

void CInactivateMultiplePatientsDlg::OnAddToInactivateList() 
{

	try { 
		long nCurSel = m_pFilterList->CurSel;
		
		if (nCurSel == -1) {
			return;
		}

		long nValue = m_pFilterList->GetValue(nCurSel, 0);
		if (nValue == -1) {
			//this should never happen 
			ASSERT(FALSE);
			return;
		}

		long nCount = GetDlgItemInt(IDC_PATS_IN_LIST);

		CString strSql, strFilter, strFilterWhere, strFilterFrom;

		strFilter = VarString(m_pFilterList->GetValue(nCurSel, 2));

		CWaitCursor cwait;


		//Decrypt it
		if(!CFilter::ConvertFilterStringToClause(nValue, strFilter, 1, &strFilterWhere, &strFilterFrom)) {
			MsgBox("The list could not be generated because it uses an invalid filter.");
			return;
		}

		//this purposely includes cancelled appts and deleted bills
		strSql.Format("Select PersonT.ID AS PersonID, PatsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
			" PatsT.ModifiedDate AS LastModDate, "
			" (SELECT Max(AppointmentsT.StartTime) FROM AppointmentsT WHERE AppointmentsT.PatientID = PatsT.PersonID) AS LastApptDate, "
			" (SELECT Max(BillsT.Date) FROM BillsT WHERE BillsT.PatientID = PatsT.PersonID) AS LastBillDate, "
			" (SELECT Max(Notes.Date) FROM Notes WHERE Notes.PersonID = PatsT.PersonID) AS LastNoteDate "
			"  FROM %s "
			" INNER JOIN PatientsT PatsT ON PersonT.ID = PatsT.PersonID "
			" WHERE %s AND PatsT.PersonID IS NOT NULL AND PatsT.CurrentStatus <> 4"
			" GROUP BY PersonT.ID, PatsT.UserDefinedID, PersonT.Last, PErsonT.First, PersonT.Middle, PatsT.ModifiedDate, PatsT.PersonID", strFilterFrom, strFilterWhere); 
#ifdef _DEBUG
		MessageBox(strSql);
#endif

		//run the query
		_RecordsetPtr rsPats = CreateRecordsetStd(strSql);

		IRowSettingsPtr pRow;
		FieldsPtr flds = rsPats->Fields;
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtTemp;
		while (! rsPats->eof) {

			pRow = m_pInactivateList->GetRow(-1);


			pRow->PutValue(COLUMN_PERSON_ID, AdoFldLong(flds, "PersonID"));
			pRow->PutValue(COLUMN_USER_DEFINED_ID, AdoFldLong(flds, "UserDefinedID"));
			pRow->PutValue(COLUMN_PATIENT_NAME, _variant_t(AdoFldString(flds, "Name")));
			dtTemp = AdoFldDateTime(flds, "LastApptDate", dtNow);
			if (dtNow != dtTemp) {
				pRow->PutValue(COLUMN_LAST_APPT_DATE, flds->Item["LastApptDate"]->Value);
			}
			dtTemp = AdoFldDateTime(flds, "LastBillDate", dtNow);
			if (dtNow != dtTemp) {
				pRow->PutValue(COLUMN_LAST_BILL_DATE, flds->Item["LastBillDate"]->Value);
			}
			dtTemp = AdoFldDateTime(flds, "LastNoteDate", dtNow);
			if (dtNow != dtTemp) {
				pRow->PutValue(COLUMN_LAST_NOTE_DATE, flds->Item["LastNoteDate"]->Value);
			}
			dtTemp = AdoFldDateTime(flds, "LastModDate");
			if (dtNow != dtTemp) {
				pRow->PutValue(COLUMN_LAST_MODIFIED_DATE, flds->Item["LastModDate"]->Value);
			}

			//check to see that the row isn't already in the list
			if (m_pInactivateList->FindByColumn(COLUMN_PERSON_ID, AdoFldLong(flds, "PersonID"), 0, false) == -1) {

				//add the row
				m_pInactivateList->AddRow(pRow);

				//increment the count
			}

			rsPats->MoveNext();
		}

		//now put the new count
		SetDlgItemInt(IDC_PATS_IN_LIST, m_pInactivateList->GetRowCount());

	}NxCatchAll("Error Adding to Inactivat List");
	
}



void CInactivateMultiplePatientsDlg::OnRemindMonthly() 
{
	if (IsDlgButtonChecked(IDC_REMIND_MONTHLY)) {
		GetDlgItem(IDC_INTERVAL_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_USER_REMIND_LIST)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_INTERVAL_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_USER_REMIND_LIST)->EnableWindow(FALSE);
	}

	
}

void CInactivateMultiplePatientsDlg::OnRequeryFinishedUserRemindList(short nFlags) 
{

	//add the {None} row
	IRowSettingsPtr pRow = m_pUserList->GetRow(-1);

	pRow->PutValue(0,(long) -1);
	pRow->PutValue(1, "{None}");

	m_pUserList->InsertRow(pRow, 0);
	
}

void CInactivateMultiplePatientsDlg::OnCancel() 
{
	
	CDialog::OnCancel();
}

HBRUSH CInactivateMultiplePatientsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (pWnd->GetDlgCtrlID())  {
		case IDC_STATIC:
		case IDC_REMIND_MONTHLY:
	
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();F
			pDC->SetBkColor(PaletteColor(0x00FFCC99));
			return m_brush;
		break;
		default:
		break;
	}

	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
