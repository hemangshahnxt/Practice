// HL7LinkMultiplePatientsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "HL7LinkMultiplePatientsDlg.h"
//TES 4/17/2008 - PLID 29595 - Some declarations were moved to (sharable) HL7ParseUtils.
#include "HL7ParseUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHL7LinkMultiplePatientsDlg dialog


CHL7LinkMultiplePatientsDlg::CHL7LinkMultiplePatientsDlg(CWnd* pParent)
	: CNxDialog(CHL7LinkMultiplePatientsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHL7LinkMultiplePatientsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bPatientListFiltered = true;
}


void CHL7LinkMultiplePatientsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7LinkMultiplePatientsDlg)
	DDX_Control(pDX, IDC_LINK, m_nxbLink);
	DDX_Control(pDX, IDC_OK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MATCHING_INFO, m_btnHelp);
	DDX_Control(pDX, IDC_MASS_LINK_CAPTION, m_nxstaticMassLinkCaption);
	DDX_Control(pDX, IDC_HL7_PATIENTS_CAPTION, m_nxstaticHl7PatientsCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7LinkMultiplePatientsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7LinkMultiplePatientsDlg)
	ON_BN_CLICKED(IDC_LINK, OnLink)
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_BN_CLICKED(IDC_MATCHING_INFO, OnMatchingInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7LinkMultiplePatientsDlg message handlers

enum HL7ListColumns {
	hlcHL7ID = 0,
	hlcLast = 1,
	hlcFirst = 2,
	hlcMiddle = 3,
	hlcBirthDate = 4,
	hlcSSN = 5,
	hlcLinkedTo = 6,
	hlcPersonID = 7,
};

enum NextechListColumns {
	nlcPersonID = 0,
	nlcName = 1,
	nlcUserDefinedID = 2,
	nlcBirthDate = 3,
	nlcSSN = 4,
	nlcMatchQuality = 5,
};

using namespace NXDATALIST2Lib;
BOOL CHL7LinkMultiplePatientsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		m_pPracticePatientList = BindNxDataList2Ctrl(IDC_PRACTICE_PATIENT_LIST, false);
		m_pHL7PatientList = BindNxDataList2Ctrl(IDC_HL7_PATIENT_LIST, false);
		
		m_nxbLink.AutoSet(NXB_LEFT);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		_variant_t varNull;
		varNull.vt = VT_NULL;
		//TES 8/9/2007 - PLID 26892 - Add all the PIDs we've been asked to link to the HL7 Patients list.
		for(int i = 0; i < m_arPIDsToLink.GetSize(); i++) {
			HL7_PIDFields PID = m_arPIDsToLink[i];
			IRowSettingsPtr pRow = m_pHL7PatientList->GetNewRow();
			pRow->PutValue(hlcHL7ID, _bstr_t(PID.strHL7ID));
			pRow->PutValue(hlcLast, _bstr_t(PID.strLast));
			pRow->PutValue(hlcFirst, _bstr_t(PID.strFirst));
			pRow->PutValue(hlcMiddle, _bstr_t(PID.strMiddle));
			if(PID.dtBirthDate.GetStatus() == COleDateTime::valid) {
				_variant_t varDate(PID.dtBirthDate, VT_DATE);
				pRow->PutValue(hlcBirthDate, varDate);
			}
			else {
				pRow->PutValue(hlcBirthDate, varNull);
			}
			pRow->PutValue(hlcSSN, _bstr_t(PID.strSSN));
			pRow->PutValue(hlcLinkedTo, _bstr_t("{Create New}"));
			pRow->PutValue(hlcPersonID, (long)-1);
			m_pHL7PatientList->AddRowSorted(pRow, NULL);
		}

		//TES 8/9/2007 - PLID 26892 - Set up our captions, incorporating the name of the HL7 group in question.
		SetWindowText("Link " + m_strHL7GroupName + " Patients");

		SetDlgItemText(IDC_MASS_LINK_CAPTION, m_strHL7GroupName + " is trying to update information for the listed patients, "
			"but they could not be found in the Nextech database.  For each row in the '" + m_strHL7GroupName + " Patients' "
			"list, please click on the row to see a list of similar Nextech patient records (click the ? button to see how this list is calculated). If you determine that one of the "
			"similar Nextech records corresponds to the incoming " + m_strHL7GroupName + " record, highlight that record in "
			"the 'Nextech Patients' list and click the 'Link' button.  You may also click '<Show All Patients>' to see the "
			"full list of all patients in the Nextech database.\r\n"
			"WARNING: If you link an incoming " + m_strHL7GroupName + " patient to an existing Nextech patient, then when "
			"you click 'OK', the information in the incoming " + m_strHL7GroupName + " record, including the name, will "
			"PERMANENTLY overwrite the information on the existing Nextech patient!");
		SetDlgItemText(IDC_HL7_PATIENTS_CAPTION, m_strHL7GroupName + " Patients");

	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CHL7LinkMultiplePatientsDlg::AddPID(HL7_PIDFields &PID)
{
	//TES 8/9/2007 - PLID 26892 - Add to our list, unless we already have this ID in our list (we can only have one
	// Nextech ID for each HL7 ID).
	bool bMatched = false;
	for(int i = 0; i < m_arPIDsToLink.GetSize() && !bMatched; i++) {
		if(PID.strHL7ID == m_arPIDsToLink[i].strHL7ID) bMatched = true;
	}
	if(!bMatched) {
		m_arPIDsToLink.Add(PID);
	}
}

int CHL7LinkMultiplePatientsDlg::GetPatientCount()
{
	return m_arPIDsToLink.GetSize();
}

BEGIN_EVENTSINK_MAP(CHL7LinkMultiplePatientsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHL7LinkMultiplePatientsDlg)
	ON_EVENT(CHL7LinkMultiplePatientsDlg, IDC_HL7_PATIENT_LIST, 2 /* SelChanged */, OnSelChangedHl7PatientList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CHL7LinkMultiplePatientsDlg, IDC_PRACTICE_PATIENT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPracticePatientList, VTS_I2)
	ON_EVENT(CHL7LinkMultiplePatientsDlg, IDC_PRACTICE_PATIENT_LIST, 19 /* LeftClick */, OnLeftClickPracticePatientList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7LinkMultiplePatientsDlg, IDC_PRACTICE_PATIENT_LIST, 2 /* SelChanged */, OnSelChangedPracticePatientList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CHL7LinkMultiplePatientsDlg::OnSelChangedHl7PatientList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		IRowSettingsPtr pRow(lpNewSel);
		if(pRow) {

			//TES 8/9/2007 - PLID 26892 - Filter our list just on Nextech patients that are a relatively close match.
			m_bPatientListFiltered = true;
			
			long nLinkedPersonID = VarLong(pRow->GetValue(hlcPersonID),-1);
			CString strLast = VarString(pRow->GetValue(hlcLast));
			CString strFirst = VarString(pRow->GetValue(hlcFirst));
			CString strMiddle = VarString(pRow->GetValue(hlcMiddle));
			_variant_t varBirthDate = pRow->GetValue(hlcBirthDate);
			CString strBirthDate = varBirthDate.vt == VT_DATE ? "(BirthDate Is Null OR BirthDate = '" + FormatDateTimeForSql(varBirthDate) + "')" : "1=1";
			CString strSSN = VarString(pRow->GetValue(hlcSSN));
			strSSN.TrimRight();
			CString strSSNWhere = strSSN.IsEmpty() ? "1=1" : "(SocialSecurity Is Null OR SocialSecurity = '' OR SocialSecurity = '" + _Q(strSSN) + "')";

			IRowSettingsPtr pExisting;
			CString strQuery;
			//TES 8/9/2007 - PLID 26892 - First, if we're already linked to a Nextech patient, that's our best match.
			if(nLinkedPersonID != -1) {
				strQuery += FormatString("SELECT ID, 1 AS MatchQuality FROM PersonT WHERE ID = %li UNION ", nLinkedPersonID);
			}
			//TES 8/9/2007 - PLID 26892 - Second, any Nextech patient with the exact same First, Middle, Last, DOB, and SSN.
			strQuery += FormatString("SELECT ID, 2 AS MatchQuality FROM PersonT WHERE Last = '%s' AND First = '%s' AND "
				"Middle = '%s' AND %s AND %s", _Q(strLast), _Q(strFirst), _Q(strMiddle), strBirthDate, strSSNWhere);
			//Third, any Nextech patient with the exact same First, Last, DOB, and SSN.
			strQuery += FormatString(" UNION SELECT ID, 3 AS MatchQuality FROM PersonT WHERE Last = '%s' AND First = '%s' AND "
				"%s AND %s", _Q(strLast), _Q(strFirst), strBirthDate, strSSNWhere);
			//Fourth, any patient with the exact same, non-empty SSN.
			if(!strSSN.IsEmpty()) {
				strQuery += FormatString(" UNION SELECT ID, 4 AS MatchQuality FROM PersonT WHERE SocialSecurity = '%s'", _Q(strSSN));
			}
			//Fifth, any patient with the exact same First, Middle, and Last.
			strQuery += FormatString(" UNION SELECT ID, 5 AS MatchQuality FROM PersonT WHERE First = '%s' AND Middle = '%s' "
				"AND Last = '%s'", _Q(strFirst), _Q(strMiddle), _Q(strLast));
			//Sixth, any patient with the same First OR Last name, and the same non-empty DOB (maybe it's a variant on their
			// first name, like Catherine vs. Katherine, or maybe they got married).
			if(varBirthDate.vt == VT_DATE) {
				strQuery += FormatString(" UNION SELECT ID, 6 AS MatchQuality FROM PersonT WHERE (Last = '%s' OR First = '%s') AND "
					"BirthDate = '%s'", _Q(strLast), _Q(strFirst), FormatDateTimeForSql(VarDateTime(varBirthDate)));
			}

			//TES 8/9/2007 - PLID 26892 - Now requery the list to have those good matches.
			CString strFromClause = FormatString("(SELECT PersonT.ID, Last + ', ' + First + ' ' + Middle AS FullName, UserDefinedID, "
				"BirthDate, SocialSecurity, MatchQuality FROM PersonT INNER JOIN PatientsT ON PersonT.ID = "
				"PatientsT.PersonID INNER JOIN (SELECT ID, Min(MatchQuality) AS MatchQuality FROM (%s) MatchQualitySubQ GROUP BY ID) AS MatchQualityQ "
				"ON PersonT.ID = MatchQualityQ.ID) AS SubQ", strQuery);
			m_pPracticePatientList->FromClause = _bstr_t(strFromClause);
			m_pPracticePatientList->Requery();
		}
		EnableButtons();
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnSelChangedHl7PatientList()");
}

void CHL7LinkMultiplePatientsDlg::OnCancel() 
{	
	try {
		if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to cancel?  All messages for the listed patients will not be committed.")) {
			CDialog::OnCancel();
		}
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnCancel()");
}

void CHL7LinkMultiplePatientsDlg::OnLink() 
{
	try {
		IRowSettingsPtr pHL7Row = m_pHL7PatientList->CurSel;
		IRowSettingsPtr pNxRow = m_pPracticePatientList->CurSel;
		
		if(pHL7Row == NULL || pNxRow == NULL) {
			ASSERT(FALSE);
			MsgBox("Please select both an HL7 Patient and a Nextech Patient to link.");
			return;
		}

		long nLinkedToID = VarLong(pNxRow->GetValue(nlcPersonID), -1);
		//TES 8/9/2007 - PLID 26892 - They can't link to the "<Show All Patients>" row!
		if(nLinkedToID != -2) {
			pHL7Row->PutValue(hlcLinkedTo, pNxRow->GetValue(nlcName));
			pHL7Row->PutValue(hlcPersonID, nLinkedToID);
		}
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnLink()");
}

void CHL7LinkMultiplePatientsDlg::OnOk() 
{
	try {
		//TES 8/9/2007 - PLID 26892 - Fill in our output map.
		IRowSettingsPtr pRow = m_pHL7PatientList->GetFirstRow();
		while(pRow) {
			m_mapHL7IDToNextechID.SetAt(VarString(pRow->GetValue(0)), (LPVOID)VarLong(pRow->GetValue(7)));
			pRow = pRow->GetNextRow();
		}
		CDialog::OnOK();
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnOk()");
}

void CHL7LinkMultiplePatientsDlg::OnRequeryFinishedPracticePatientList(short nFlags) 
{
	try {
		//TES 8/9/2007 - PLID 26892 - Add a row to "Show All" or "Filter" (depending whether they're filtered already).
		IRowSettingsPtr pRow = m_pPracticePatientList->GetNewRow();
		pRow->PutValue(nlcPersonID, (long)-2);
		pRow->PutValue(nlcName, m_bPatientListFiltered ? _bstr_t("<Show All Patients>") : _bstr_t("<Filter Patients>"));
		pRow->PutCellLinkStyle(nlcName, dlLinkStyleTrue);
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow->PutValue(nlcUserDefinedID, varNull);
		pRow->PutValue(nlcBirthDate, varNull);
		pRow->PutValue(nlcSSN, varNull);
		pRow->PutValue(nlcMatchQuality, (long)99);
		m_pPracticePatientList->AddRowAtEnd(pRow, NULL);

		//TES 8/9/2007 - PLID 26892 - And put the "{Create New}" option at the top of the list.
		pRow = m_pPracticePatientList->GetNewRow();
		pRow->PutValue(nlcPersonID, (long)-1);
		pRow->PutValue(nlcName, _bstr_t("{Create New}"));
		pRow->PutValue(nlcUserDefinedID, varNull);
		pRow->PutValue(nlcBirthDate, varNull);
		pRow->PutValue(nlcSSN, varNull);
		pRow->PutValue(nlcMatchQuality, (long)0);
		m_pPracticePatientList->AddRowBefore(pRow, m_pPracticePatientList->GetFirstRow());

		//TES 8/9/2007 - PLID 26892 - If we have a row with a match quality of 1 or 2, automatically select it, otherwise
		// select the {Create New} option.
		m_pPracticePatientList->SetSelByColumn(nlcMatchQuality, (long)1);
		if(m_pPracticePatientList->CurSel == NULL) {
			m_pPracticePatientList->SetSelByColumn(nlcMatchQuality, (long)2);
			if(m_pPracticePatientList->CurSel == NULL) {
				m_pPracticePatientList->SetSelByColumn(nlcMatchQuality, (long)0);
			}
		}
		EnableButtons();
	}NxCatchAll("CHL7LinkMultiplePatientsDlg::OnRequeryFinishedPracticePatientList()");
}

void CHL7LinkMultiplePatientsDlg::OnLeftClickPracticePatientList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL && nCol == 1 && VarLong(pRow->GetValue(0)) == -2) {
			if(m_bPatientListFiltered) {
				//TES 8/9/2007 - PLID 26892 - They want to show all patients.
				m_bPatientListFiltered = false;
				m_pPracticePatientList->FromClause = _bstr_t("((SELECT ID, Last + ', ' + First + ' ' + Middle AS FullName, UserDefinedID, BirthDate, SocialSecurity, -1 AS MatchQuality FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PersonT.ID > 0)) AS PersonQ");
				m_pPracticePatientList->Requery();
				EnableButtons();
			}
			else {
				//TES 8/9/2007 - PLID 26892 - The selchanged handler will filter the list properly.
				OnSelChangedHl7PatientList(NULL, m_pHL7PatientList->CurSel);
			}
		}
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnLeftClickPracticePatientList()");
}

void CHL7LinkMultiplePatientsDlg::OnMatchingInfo() 
{
	try {
		MessageBox("When you select an incoming message, the Nextech Patients list will be filtered to show patients that meet any of the following criteria:\r\n"
			"-If you have already linked the incoming message to a specific Nextech record, that record will always be included.\r\n"
			"-Any patients whose First, Middle, and Last names are the same as the incoming record, and whose SSN and Birth Date do not conflict.\r\n"
			"-Any patients whose First and Last names are the same as the incoming record, and whose SSN and Birth Date do not conflict.\r\n"
			"-Any patients with the exact same, non-empty Social Security Number.\r\n"
			"-Any patients whose First, Middle, and Last names are the same as the incoming record.\r\n"
			"-Any patients with the exact same, non-empty Birth Date AND either the same First or Last name. \r\n"
			"Note: A field is treated as conflicting if it is non-empty on both the existing and incoming data, and is not identical.  Otherwise, it does not conflict.");
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnMatchingInfo()");
}

void CHL7LinkMultiplePatientsDlg::OnSelChangedPracticePatientList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		EnableButtons();
	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnSelChangedPracticePatientList()");
}

void CHL7LinkMultiplePatientsDlg::EnableButtons()
{
	//TES 10/2/2007 - PLID 26892 - The Link button should be enabled iff there is a row selected in both lists.

	IRowSettingsPtr pHL7Row = m_pHL7PatientList->CurSel;
	IRowSettingsPtr pNxRow = m_pPracticePatientList->CurSel;
	if(pNxRow != NULL && pHL7Row != NULL) {
		m_nxbLink.EnableWindow(TRUE);
	}
	else {
		m_nxbLink.EnableWindow(FALSE);
	}
}

// (j.jones 2008-04-14 09:40) - PLID 29596 - added OnCtlColor function
HBRUSH CHL7LinkMultiplePatientsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	try {

		HANDLE_GENERIC_TRANSPARENT_CTL_COLOR();

	}NxCatchAll("Error in CHL7LinkMultiplePatientsDlg::OnCtlColor");

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
