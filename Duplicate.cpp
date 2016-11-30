y// Duplicate.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "Duplicate.h"
#include "globalutils.h"
#include "pracprops.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
extern CPracticeApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CDuplicate dialog

//(e.lally 2008-02-27) PLID 27379 - Removed text parameters for buttons - their are not used and 
//it is easy to add back in if we ever need it.
CDuplicate::CDuplicate(CWnd* pParent, bool bForceSelectionOnIgnore /*= false*/)
	: CNxDialog(CDuplicate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDuplicate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bForceSelectionOnIgnore = bForceSelectionOnIgnore;
	m_nSelPatientId = -1;
	// (d.moore 2007-09-05) - PLID 25455 - I modified the dialog to allow filtering the
	//  search and contents by the PatientsT.CurrentStatus field. Use SetStatusFilter() to
	//  set the filter. Use EStatusFilterTypes to set values.
	m_nStatusFilter = 0;
	//(e.lally 2008-02-27) PLID 27379 - Added button for merging into selected entry
	m_bUseMergeBtn = FALSE;
	m_bEnableGoToPatientBtn = TRUE;
	m_bEnableSaveBtn = TRUE; // (r.goldschmidt 2014-07-17 18:46) - PLID 62774
}


void CDuplicate::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDuplicate)
	DDX_Control(pDX, IDC_GOTO_SELECTED, m_btnGoToSelected);
	DDX_Control(pDX, IDC_DUP_MERGE_WITH_SELECTED, m_btnMergeWithSelected);
	DDX_Control(pDX, IDC_DUP_CHANGE_NAME, m_btnChangeName);
	DDX_Control(pDX, IDC_DUP_ADD_NEW_PATIENT, m_btnAddNewPatient);
	DDX_Control(pDX, IDC_CANCEL_NEW_PATIENT, m_btnCancelNewPatient);
	DDX_Control(pDX, IDC_NAME_LABEL, m_nxstaticNameLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDuplicate, CNxDialog)
	//{{AFX_MSG_MAP(CDuplicate)
	ON_BN_CLICKED(IDC_DUP_ADD_NEW_PATIENT, OnSaveAsNewPatient)
	ON_BN_CLICKED(IDC_DUP_CHANGE_NAME, OnChangeNewPatientName)
	ON_BN_CLICKED(IDC_CANCEL_NEW_PATIENT, OnCancelNewPatientEntry)
	ON_BN_CLICKED(IDC_GOTO_SELECTED, OnGoto)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_DUP_MERGE_WITH_SELECTED, OnMergeWithSelectedPatient)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDuplicate, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDuplicate)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
bool CDuplicate::FindDuplicates(CString first, CString last, CString middle)
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	return FindDuplicates(first, last, middle, dtInvalid);
}

// (z.manning 2009-08-24 12:49) - PLID 31135 - Added an overload with a birth date parameter
bool CDuplicate::FindDuplicates (CString first, CString last, CString middle, COleDateTime dtBirthDate)
{
	_RecordsetPtr rs;
	CString	value;

	try
	{
		//JJ - 5/14/2001 - we don't search on middle name anymore, but I will keep it in the parameter list for
		//easy reintegration should we ever use it again
		//// (r.goldschmidt 2014-07-17 15:48) - PLID 62774 - If middle name is used, need to correct CNewPatient::CheckForDuplicatePatients()
		// (d.moore 2007-08-16) - PLID 25455 - I modified the dialog to allow filtering the
		//  search and contents by the PatientsT.CurrentStatus field.
		CString strQuery;
		strQuery.Format(
			"SELECT ID "
			"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"WHERE (First = '%s' AND Last = '%s'", 
			first, last);

		CString strWhere, strOr;
		if ((m_nStatusFilter & esfPatient) > 0) {
			strWhere += strOr + "PatientsT.CurrentStatus = 1";
			strOr = " OR ";
		}
		if ((m_nStatusFilter & esfProspect) > 0) {
			strWhere += strOr + "PatientsT.CurrentStatus = 2";
			strOr = " OR ";
		}
		if ((m_nStatusFilter & esfPatientProspect) > 0) {
			strWhere += strOr + "PatientsT.CurrentStatus = 3";
			strOr = " OR ";
		}
		if ((m_nStatusFilter & esfInquiry) > 0) {
			strWhere += strOr + "PatientsT.CurrentStatus = 4";
			strOr = " OR ";
		}
		
		if (strWhere.GetLength() > 0) {
			strQuery += " AND (" + strWhere + ")";
		}
		strQuery += ") ";

		// (z.manning 2009-08-24 12:38) - PLID 31135 - We now check birthdate against first OR last name too.
		if(dtBirthDate.GetStatus() == COleDateTime::valid && GetRemotePropertyInt("NewPatientsCheckBirthDate", 1, 0, "<None>") == 1)
		{
			CString strBirthDateWhere = FormatString(
				" BirthDate = '%s' AND (First = '%s' OR Last = '%s') "
				, FormatDateTimeForSql(dtBirthDate), first, last);
			strQuery += " OR (" + strBirthDateWhere + ") ";
		}

		rs = CreateRecordsetStd(strQuery);

		if (rs->eof)
			return false;
		sql = "ID = ";
		while (!rs->eof)
		{	value.Format("%i", AdoFldLong(rs, "ID"));
			sql += value + " OR ID = ";
			rs->MoveNext();
		}
		rs->Close();
		sql.TrimRight (" OR ID = ");
		if (middle.IsEmpty()){
			m_name = first + ' ' + last;
		}
		else { // (r.goldschmidt 2014-07-17 15:48) - PLID 62774 - Should put full entered name
			m_name = first + ' ' + middle + ' ' + last;
		}
	}NxCatchAllCall("Error 100: CDuplicate::FindDuplicates", return false;);
	return true;
}

BOOL CDuplicate::OnInitDialog() 
{
	try {
		//(e.lally 2008-02-27) PLID 27379 - Changed this to a NxDialog
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 10:57) - PLID 29863 - NxIconify buttons
		m_btnGoToSelected.AutoSet(NXB_CANCEL);
		m_btnMergeWithSelected.AutoSet(NXB_OK);
		m_btnChangeName.AutoSet(NXB_OK);
		m_btnAddNewPatient.AutoSet(NXB_OK);
		m_btnCancelNewPatient.AutoSet(NXB_CANCEL);

		//(e.lally 2008-02-27) PLID 27379 - added a color control with the same background
		//color as the new patient screen.
		m_backgroundColor = 0x009CE3F5; //New patient screen color
		m_brush.CreateSolidBrush(PaletteColor(m_backgroundColor));

		m_pDupList = BindNxDataListCtrl(IDC_DUPLICATES, false);
		m_pDupList->WhereClause = (LPCTSTR)sql;
		m_pDupList->Requery();
		SetDlgItemText(IDC_NAME_LABEL, m_name);
		CWnd *pWnd = GetDlgItem(IDC_NAME_LABEL);
		pWnd->SetFont(&theApp.m_boldFont);

		EnsureControls();

		// Select the patient if specified
		if (m_nSelPatientId != -1) {
			m_pDupList->SetSelByColumn(0, (long)m_nSelPatientId);
		}
	}
	NxCatchAll("Error in CDuplicate::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

long CDuplicate::GetSelPatientId()
{
	if(m_pDupList->CurSel == -1) return -1;
	else return VarLong(m_pDupList->GetValue(m_pDupList->CurSel, 0), -1);
}



void CDuplicate::OnSaveAsNewPatient() 
{
	m_nSelPatientId = GetSelPatientId();
	if (m_bForceSelectionOnIgnore && m_nSelPatientId == -1) {
		MsgBox(MB_OK|MB_ICONINFORMATION, RCS(IDS_DUPLICATE_NO_PAT_SEL));
		return;
	}

	//(e.lally 2008-02-27) PLID 27379 - Updated return IDs to more descriptive names
	EndDialog(ID_CREATE_NEW_PATIENT);
}

void CDuplicate::OnChangeNewPatientName() 
{
	m_nSelPatientId = -1;
	
	//(e.lally 2008-02-27) PLID 27379 - Updated return IDs to more descriptive names
	EndDialog(ID_CHANGE_NEW_PATIENT_NAME);	
}

void CDuplicate::OnCancelNewPatientEntry() 
{
	m_nSelPatientId = -1;
	
	//(e.lally 2008-02-27) PLID 27379 - Updated return IDs to more descriptive names
	EndDialog(ID_CANCEL_NEW_PATIENT);	
}

void CDuplicate::OnGoto() 
{
	//cancel the new patient dialog and goto the one they have selected
	//check to see if something is selected
	if(m_pDupList->CurSel == -1) {
		AfxMessageBox("Please select a patient.");
		return;
	}

	//set member variable so we know which one was selected at the time
	m_nSelPatientId = GetSelPatientId();

	//(e.lally 2008-02-27) PLID 27379 - Updated return IDs to more descriptive names
	//pretend to cancel, the new patient dialog will see if anything is set here and handle appropriately
	EndDialog(ID_CANCEL_AND_GOTO_SELECTED);
}

// (d.moore 2007-08-17) - PLID 25455 - Used to set a filter for the dialog based on the
//  PatientsT.CurrentStatus field. You can OR several EStatusFilterTypes values together
//  to search on more than one type.
void CDuplicate::SetStatusFilter(EStatusFilterTypes nFilterType)
{
	m_nStatusFilter = nFilterType;
}

//(e.lally 2008-02-27) PLID 27379 - Made this an NxDialog
HBRUSH CDuplicate::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_DUP_BACKGROUND:
		case IDC_NAME_LABEL:
		case IDC_STATIC:
			//extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(m_backgroundColor));
			return m_brush;
	}
	
	return hbr;
	*/

	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

//(e.lally 2008-02-27) PLID 27379 - Added a button for merging into the selected patient
void CDuplicate::OnMergeWithSelectedPatient()
{
	long nSelPatientID = GetSelPatientId();
	if(nSelPatientID == -1){
		AfxMessageBox("You must select a patient in order to perform this action.");
		return;
	}
	m_nSelPatientId = nSelPatientID;

	EndDialog(ID_MERGE_WITH_SELECTED);
}


//(e.lally 2008-02-27) PLID 27379 - This lets us know if the merge button or change patient name
//button is going to be visible. Eventually the new patient dlg might want that ability too
//in which case we can show all buttons.
void CDuplicate::SetUseMergeButton(BOOL bUseMergeBtn)
{
	m_bUseMergeBtn = bUseMergeBtn;
}

//(e.lally 2008-02-29) PLID 27379 - Allow the Go To Patient button to be disabled.
void CDuplicate::EnableGoToPatientBtn(BOOL bEnable)
{
	m_bEnableGoToPatientBtn = bEnable;
}

// (r.goldschmidt 2014-07-17 18:45) - PLID 62774 - If this is false, then the functionality of the dialog
//  is changed to not allow saving from duplicate dialog. In this case, clicking the change patient name
//  button results in confirming that a new patient is being created and continuing in new patient dialog.
//  Default is true.
void CDuplicate::EnableSaveBtn(BOOL bEnable)
{
	m_bEnableSaveBtn = bEnable;
}

//(e.lally 2008-02-27) PLID 27379 - Eventually the new patient dlg might want that ability too
//in which case we can show all buttons. Until then, show one or the other, defaulting to "change name"
void CDuplicate::EnsureControls()
{
	//show/hide the change patient name button
	GetDlgItem(IDC_DUP_CHANGE_NAME)->ShowWindow(!m_bUseMergeBtn);
	//hide/show the merge button.
	GetDlgItem(IDC_DUP_MERGE_WITH_SELECTED)->ShowWindow(m_bUseMergeBtn);

	//(e.lally 2008-02-29) PLID 27379 - use the set BOOL to enable/disable the Go To button
	GetDlgItem(IDC_GOTO_SELECTED)->EnableWindow(m_bEnableGoToPatientBtn);

	// (r.goldschmidt 2014-07-17 18:46) - PLID 62774 - Disable/Enable Save button
	GetDlgItem(IDC_DUP_ADD_NEW_PATIENT)->EnableWindow(m_bEnableSaveBtn);
	if (!m_bEnableSaveBtn){ // if save button disabled, Change Name Button text and dialog caption get altered
		GetDlgItem(IDC_DUP_CHANGE_NAME)->SetWindowText("Confirm New Patient and Continue");
		SetWindowText("Potential Duplicate Patients");
	}
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK and OnCancel handlers to keep behavior the same as pre-NxDialog changes
void CDuplicate::OnOK()
{
	//Eat the message
}

void CDuplicate::OnCancel()
{
	//Eat the message
}
