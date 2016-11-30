// InquiryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "InquiryDlg.h"
#include "InternationalUtils.h"
#include "datetimeutils.h"
#include "ProcedureInfo.h"
#include "WellnessDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInquiryDlg dialog

//(e.lally 2009-03-11) PLID 29895 - Added column enums
enum ProcedureListColumns
{
	plcID=0,
	plcChecked=1,
	plcName=2,
};

enum ProcedureGroupColumns
{
	pgcID=0,
	pgcChecked=1,
	pgcName=2,
};

CInquiryDlg::CInquiryDlg(CWnd* pParent)
	: CNxDialog(IDD, pParent)
{
	m_pReferralSubDlg = NULL;

	m_strLoadFirst = "";
	m_strLoadMiddle = "";
	m_strLoadLast = "";
	m_strLoadEmail = "";
	m_nLoadReferralID = -1;
	m_bLoadProcedures = TRUE;
	//{{AFX_DATA_INIT(CInquiryDlg)
	//}}AFX_DATA_INIT
}

// (j.gruber 2008-09-08 16:08) - PLID 30899 - added ability to load fields from the new patient dialog
CInquiryDlg::CInquiryDlg(CString strFirst, CString strMiddle, CString strLast, CString strEmail, long nReferralID, CArray<int,int> *paryProcs, CArray<int,int> *paryProcGroups, BOOL bLoadProcedures, CString strNote, CWnd* pParent)
	: CNxDialog(IDD, pParent)
{
	m_pReferralSubDlg = NULL;

	m_strLoadFirst = strFirst;
	m_strLoadMiddle = strMiddle;
	m_strLoadLast = strLast;
	m_strLoadEmail = strEmail;
	m_strLoadNote = strNote;
	m_nLoadReferralID = nReferralID;

	// (j.gruber 2009-06-01 11:22) - PLID 34424 - check if this exists
	if (paryProcs) {
		for (int i = 0; i < paryProcs->GetSize(); i++) {
			m_arProcIDs.Add(paryProcs->GetAt(i));
		}
	}

	// (j.gruber 2009-06-01 11:22) - PLID 34424 - check if this exists
	if (paryProcGroups) {
		for (int i = 0; i < paryProcGroups->GetSize(); i++) {
			m_arProcGroupIDs.Add(paryProcGroups->GetAt(i));
		}
	}

	m_bLoadProcedures = bLoadProcedures;


	//{{AFX_DATA_INIT(CInquiryDlg)
	//}}AFX_DATA_INIT
}


void CInquiryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInquiryDlg)
	DDX_Control(pDX, IDC_SHOW_PROCEDURE_GROUPS, m_btnShowProcGroups);
	DDX_Control(pDX, IDC_SHOW_PROCEDURES, m_btShowProcedures);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SAVE_INQUIRY, m_btnSave);
	DDX_Control(pDX, IDC_ID_BOX, m_nxeditIdBox);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_EMAIL, m_nxeditEmail);
	DDX_Control(pDX, IDC_REFERRAL_DLG, m_nxstaticReferralDlg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInquiryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInquiryDlg)
	ON_BN_CLICKED(IDC_SAVE_INQUIRY, OnSaveInquiry)
	ON_BN_CLICKED(IDC_SHOW_PROCEDURES, OnShowProcedures)
	ON_BN_CLICKED(IDC_SHOW_PROCEDURE_GROUPS, OnShowProcedureGroups)
	ON_EN_KILLFOCUS(IDC_ID_BOX, OnKillFocusID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInquiryDlg message handlers


void CInquiryDlg::OnSaveInquiry() 
{
	BOOL bInTrans = FALSE;

	try {
		
		//write it to personT and then InquiryT
		CString strFirst, strLast, strMiddle, strNote, strEmail;
		long nPersonID =0;
		GetDlgItemText(IDC_FIRST_NAME_BOX, strFirst);
		GetDlgItemText(IDC_MIDDLE_NAME_BOX, strMiddle);
		GetDlgItemText(IDC_LAST_NAME_BOX, strLast);
		GetDlgItemText(IDC_NOTES, strNote);
		GetDlgItemText(IDC_EMAIL, strEmail);
		long referralID = m_pReferralSubDlg->GetSelectedReferralID();
		
		long nCurSelLocation = m_pLocation->GetCurSel();
		// (s.tullis 2016-05-25 15:41) - NX-100760  
		_variant_t vtLocationID = g_cvarNull;
		if(nCurSelLocation != sriNoRow){
			vtLocationID = m_pLocation->GetValue(nCurSelLocation, 0);
		}

		// (j.jones 2010-01-14 17:27) - PLID 31927 - check the default text message privacy field
		long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);

		//DRT 6/13/2007 - PLID 25100 - We no longer require a procedure be selected.  However, if they have the tracking license, we'll
		//	encourage them to select one for marketing purposes.
		if(g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
			if(!IsProcedureChosen()) {
				if(MessageBox("You have not chosen a procedure for this inquiry.  It is recommended that you always choose a procedure "
					"for the system to properly report your inquiry levels.  Are you sure you wish to save with no procedure?\r\n"
					" - Pressing YES will save the inquiry with no procedure selected.\r\n"
					" - Pressing NO will cancel the save and return you to the Inquiry editor to choose a procedure.", "NexTech Practice", MB_YESNO) != IDYES)
					return;
			}
		}

		COleDateTime dt = COleDateTime::GetCurrentTime();
		CString strDate;
		strDate = FormatDateTimeForSql(dt, dtoDate);

		long nUserDefinedID = GetDlgItemInt(IDC_ID_BOX);
		
		// make sure no one else has used this ID since they entered the dialog
		if(!IsRecordsetEmpty("SELECT UserDefinedID FROM PatientsT WHERE UserDefinedID = %li", nUserDefinedID)){
			// it's been taken, see if they want a new one
			CString strMsg = "The automatically generated patient ID has just been used by someone else "
						"to create a new patient.  Click OK to use the next available ID or "
						"click Cancel to go back and change to a different patient ID.";
			if(MessageBox(strMsg, "Patient Inquiry", MB_OKCANCEL) == IDOK){
				nUserDefinedID = NewNumber("PatientsT", "UserDefinedID");
			}
			else{
				return;
			}
		}

		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray aryParams;
		//(e.lally 2009-03-11) PLID 29895 - I already did all the work to change this to a parameterized sql batch,
		//	so I will go ahead and leave it that way.
		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON\r\n"
			"DECLARE @nNewPersonID INT\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewPersonID = (SELECT COALESCE (Max(ID),0)+1 FROM PersonT)\r\n");
		// (s.tullis 2016-05-25 15:41) - NX-100760 
		// (j.jones 2010-01-14 17:27) - PLID 31927 - supported defaulting the text message privacy field
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO PersonT (ID, Location, First, Middle, Last, FirstContactDate, InputDate, Note, UserID, Email, TextMessage) "
			" VALUES (@nNewPersonID, {VT_I4}, {STRING}, {STRING}, {STRING}, GetDate(), GetDate(), {STRING}, {INT}, {STRING}, {INT})\r\n", 
			vtLocationID, strFirst, strMiddle, strLast, strNote, GetCurrentUserID(), strEmail, nTextMessagePrivacy);
		
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO PatientsT (PersonID, UserDefinedID, CurrentStatus, ReferralID) "
			"VALUES (@nNewPersonID, {INT}, 4, {INT})\r\n", nUserDefinedID, referralID);

		// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
		AddParamStatementToSqlBatch(strSqlBatch, aryParams,
			"DECLARE @SecurityGroupID INT\r\n"
			"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
			"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
			"BEGIN\r\n"
			"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, @nNewPersonID)\r\n"
			"END\r\n");
				
		if (referralID != -1){
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) "
				"VALUES ({INT}, @nNewPersonID, {STRING})\r\n", referralID, strDate);
		}

		AddStatementToSqlBatch(strSqlBatch, "SET NOCOUNT OFF\r\n"
			"SELECT @nNewPersonID AS PersonID ");

		// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
		_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch,aryParams);

		if(!rs->eof){
			nPersonID = AdoFldLong(rs, "PersonID", 0);

			//what procedures do they want?
			UpdateArray(nPersonID);

			//(e.lally 2009-03-11) PLID 33470 - Auditing of the procedure additions will happen in the phase tracking functions
			PhaseTracking::AddPtntProcedures(m_arProcIDs, nPersonID, false);
			PhaseTracking::AddProcedureGroups(m_arProcGroupIDs, nPersonID, false);

			// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
			UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), nPersonID);
		}
		
		CNxDialog::OnOK();
		
	}NxCatchAll("Error saving inquiry");

}

BOOL CInquiryDlg::IsProcedureChosen() {
	//(e.lally 2009-03-11) PLID 29895 - Added column enums
	//Update the procedures list.
	long	i = 0,
			p = m_procedureList->GetFirstRowEnum(),
			g = m_pProcGroupList->GetFirstRowEnum();

	LPDISPATCH pDisp = NULL;
	BOOL bChecked = FALSE;

	while (p)
	{
		m_procedureList->GetNextRowEnum(&p, &pDisp);

		IRowSettingsPtr pRow(pDisp);

		if (VarBool(pRow->Value[plcChecked]) != FALSE) {
			bChecked = TRUE;
		}
		pDisp->Release();
	}
	while (g)
	{
		m_pProcGroupList->GetNextRowEnum(&g, &pDisp);

		IRowSettingsPtr pRow(pDisp);

		if (VarBool(pRow->Value[pgcChecked]) != FALSE) {
			bChecked = TRUE;
		}
		pDisp->Release();
	}
	return bChecked;

}

void CInquiryDlg::UpdateArray(long nPersonID) 
{
	try
	{
		//(e.lally 2009-03-11) PLID 29895 - Added column enums
		//Update the procedures list.
		long	i = 0,
				p = m_procedureList->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		m_arProcIDs.RemoveAll();
		//(e.lally 2009-03-11) PLID 29895 - Make sure we update the string array of names
		m_arProcString.RemoveAll();

		while (p)
		{
			m_procedureList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if (VarBool(pRow->Value[plcChecked]) != FALSE) {
				m_arProcIDs.Add(VarLong(pRow->Value[plcID]));
				m_arProcString.Add(VarString(pRow->Value[plcName]));
			}
			pDisp->Release();
		}

		//Now, update the procedure groups list.
		i = 0;
		p = m_pProcGroupList->GetFirstRowEnum();

		pDisp = NULL;

		m_arProcGroupIDs.RemoveAll();
		//(e.lally 2009-03-11) PLID 29895 - Make sure we update the string array of groups
		m_arProcGroupString.RemoveAll();

		while(p) {
			m_pProcGroupList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if(VarBool(pRow->Value[pgcChecked]) != FALSE) {
				m_arProcGroupIDs.Add(VarLong(pRow->Value[pgcID]));
				m_arProcGroupString.Add(VarString(pRow->Value[pgcName]));
			}
			pDisp->Release();
		}
	}
	NxCatchAll("Could not remove all rows from export list");
}

BOOL CInquiryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		//r.wilson PLID 39357 10/28/2011
		BOOL bUserCanWrite = CheckCurrentUserPermissions(bioPatientID, SPT___W_______, FALSE, 0, TRUE);
		BOOL bUserCanWriteOnlyWithPass = CheckCurrentUserPermissions(bioPatientID, SPT___W________ONLYWITHPASS, FALSE, 0, TRUE);

		// (c.haag 2008-04-25 12:28) - PLID 29790 - NxIconified buttons
		m_btnSave.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		if(	m_pReferralSubDlg == NULL) {
			m_pReferralSubDlg = new CReferralSubDlg(this);
			m_pReferralSubDlg->Create(IDD_REFERRAL_SUBDIALOG, this);

			CRect rc;
			GetDlgItem(IDC_REFERRAL_DLG)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_pReferralSubDlg->MoveWindow(rc);
			m_pReferralSubDlg->UseBackgroundColor(0x0046A3FF);
			m_pReferralSubDlg->BringWindowToTop();
		}

		if(m_strProcWhereClause == "") {
			m_procedureList = BindNxDataListCtrl(IDC_PROCEDURE_LIST);
		}
		else {
			m_procedureList = BindNxDataListCtrl(IDC_PROCEDURE_LIST, false);
			m_procedureList->WhereClause = _bstr_t(m_strProcWhereClause);
			m_procedureList->Requery();
		}


		if(m_strGroupWhereClause == "") {
			m_pProcGroupList = BindNxDataListCtrl(IDC_PROCEDURE_GROUP_LIST);
		}
		else {
			m_pProcGroupList = BindNxDataListCtrl(IDC_PROCEDURE_GROUP_LIST, false);
			m_pProcGroupList->WhereClause = _bstr_t(m_strGroupWhereClause);
			m_pProcGroupList->Requery();
		}

		m_pLocation = BindNxDataListCtrl(IDC_INQUIRY_LOCATION);
		//find the default location, otherwise use the current location
		long LocationID = GetCurrentLocationID();
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND IsDefault = 1 AND TypeID = 1");
		if(!rs->eof) {
			LocationID = AdoFldLong(rs, "ID",GetCurrentLocationID());
		}
		rs->Close();
		m_pLocation->SetSelByColumn(0, LocationID);

		
		//set the ID
		long nNumber = NewNumber("PatientsT", "UserDefinedID");
		m_nOriginalUserDefinedID = nNumber;
		SetDlgItemInt(IDC_ID_BOX, nNumber);
		

		// (j.gruber 2008-09-08 16:08) - PLID 30899 - added ability to load fields from the new patient dialog
		SetDlgItemText(IDC_FIRST_NAME_BOX, m_strLoadFirst);
		SetDlgItemText(IDC_MIDDLE_NAME_BOX, m_strLoadMiddle);
		SetDlgItemText(IDC_LAST_NAME_BOX, m_strLoadLast);
		SetDlgItemText(IDC_EMAIL, m_strLoadEmail);
		SetDlgItemText(IDC_NOTES, m_strLoadNote);
		m_pReferralSubDlg->SelectReferralID(m_nLoadReferralID);		

		
		//check to see what we are loading
		if (m_bLoadProcedures) {
		
			// set the radio button to be procedures, not the groups initially
			CheckRadioButton(IDC_SHOW_PROCEDURES, IDC_SHOW_PROCEDURE_GROUPS, IDC_SHOW_PROCEDURES);
			OnShowProcedures();
		}
		else {
			// set the radio button to be procedures, not the groups initially
			CheckRadioButton(IDC_SHOW_PROCEDURES, IDC_SHOW_PROCEDURE_GROUPS, IDC_SHOW_PROCEDURE_GROUPS);
			OnShowProcedureGroups();

		}

		//we already set the procedure arrays
		LoadFromArray();


		//DRT 6/13/2007 - PLID 25100 - If they do not have a license for tracking, just disable the box of procedures.
		if(!g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
			GetDlgItem(IDC_PROCEDURE_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_SHOW_PROCEDURES)->EnableWindow(FALSE);
			GetDlgItem(IDC_SHOW_PROCEDURE_GROUPS)->EnableWindow(FALSE);
		}

	
	if (!bUserCanWrite && !bUserCanWriteOnlyWithPass)
	{
		m_nxeditIdBox.SetReadOnly(TRUE);
	}
	
	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CInquiryDlg::LoadFromArray()
{
try {
	//(e.lally 2009-03-11) PLID 29895 - Added column enums
		variant_t varTrue;
		varTrue.vt = VT_BOOL;
		varTrue.boolVal = TRUE;
		for (int i = 0; i < m_arProcIDs.GetSize(); i++)
		{	
			long id = m_procedureList->FindByColumn(plcID, (long)m_arProcIDs[i], 0, FALSE);
			if(id >= 0)
				m_procedureList->Value[id][plcChecked] = varTrue;
		}

		for (i = 0; i < m_arProcGroupIDs.GetSize(); i++)
		{
			long id = m_pProcGroupList->FindByColumn(pgcID, (long)m_arProcGroupIDs[i], 0, FALSE);
			if(id >= 0) 
				m_pProcGroupList->Value[id][pgcChecked] = varTrue;
		}
	}NxCatchAll("Error in CInquiryDlg::LoadFromArray()");
}

void CInquiryDlg::OnCancel() 
{
	CNxDialog::OnCancel();
}

void CInquiryDlg::OnShowProcedures() 
{
	GetDlgItem(IDC_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_PROCEDURE_GROUP_LIST)->ShowWindow(SW_HIDE);
	m_bProcedures = true;
}

void CInquiryDlg::OnShowProcedureGroups() 
{
	GetDlgItem(IDC_PROCEDURE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PROCEDURE_GROUP_LIST)->ShowWindow(SW_SHOW);
	m_bProcedures = false;		
}



BEGIN_EVENTSINK_MAP(CInquiryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInquiryDlg)
	ON_EVENT(CInquiryDlg, IDC_PROCEDURE_LIST, 4 /* LButtonDown */, OnLButtonDownProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInquiryDlg::OnLButtonDownProcedureList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow == -1)
		return;

	//(e.lally 2009-03-11) PLID 29895 - Added column enums
	switch(nCol)
	{
		case plcName:
		{	
			CProcedureInfo dlg(this);
			//When they click on the procedure ane, show the procedure's quick view info
			dlg.m_arProcIDs.Add(VarLong(m_procedureList->Value[nRow][plcID]));
			dlg.DoModal();
			break;
		}
	}	
}

BOOL CInquiryDlg::DestroyWindow() 
{
	if(m_pReferralSubDlg != NULL) {
		m_pReferralSubDlg->DestroyWindow();
		delete m_pReferralSubDlg;
		m_pReferralSubDlg = NULL;
	}

	return CNxDialog::DestroyWindow();
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CInquiryDlg::OnOK()
{
	//Eat the message
}


/*r.wilson  PLID 39357 10/28/2011*/
void CInquiryDlg::OnKillFocusID()
{
	try
	{
		//These two variables are for keeping track in any errors occurred, and which ones occurred
		BOOL bErrorsFound = FALSE;
		CString strErrors;
		BOOL bUserCanWrite = CheckCurrentUserPermissions(bioPatientID, SPT___W_______, FALSE, 0, TRUE);
		BOOL bUserCanWriteOnlyWithPass = CheckCurrentUserPermissions(bioPatientID, SPT___W________ONLYWITHPASS, FALSE, 0, TRUE);


		//nNewID holds the id that the user is TRYING to change the Patient to
		long nNewID = -1;
		nNewID = GetDlgItemInt(IDC_ID_BOX);


		//If the id didn't change then do nothing
		if(nNewID == m_nOriginalUserDefinedID)
		{
			//no change was made
			return;
		}
		
		/* r.wilson  PLID 39357 10/28/2011 - If a user has write priv. then let the id be changed provided that
											  it is in the correct format*/
		if(bUserCanWrite)
		{
			bErrorsFound = IsIdInvalid(nNewID,strErrors);
			if(!bErrorsFound)
			{
				//The user has the appropraite permissions and no errors were found so the data goes untouched
				m_nOriginalUserDefinedID = nNewID;
				return;
			}
			/*
			If a error is found then the last IF in this function will reset the ID 
			in the text box to its original value
			*/
		}

		//r.wilson  PLID 39357 10/28/2011
		else if (bUserCanWriteOnlyWithPass)
		{
			if (!CheckCurrentUserPassword())
			{
				//If we didn't have a passsword of if it failed
				CString strPatientID;
				strPatientID.Format("%d",m_nOriginalUserDefinedID);
				m_nxeditIdBox.SetWindowTextA(strPatientID);
			}
			else 
			{
				//The password was correct
				strErrors = "";
				bErrorsFound = IsIdInvalid(nNewID,strErrors);

				if(!bErrorsFound){
					m_nOriginalUserDefinedID = nNewID;
				}
			}
			
		}
	 /* 
		r.wilson  PLID 39357 10/28/2011
		(IF WE FOUND ERRORS...)
		-Show a message box that tells of any errors that pertain to the ID
		-This also sets the text box back to the original value */
		if(bErrorsFound)
		{
			CString strPatientID = "";
			strPatientID.Format("%d",m_nOriginalUserDefinedID);
			m_nxeditIdBox.SetWindowTextA(strPatientID);	
			AfxMessageBox(strErrors);
		}
	
	}NxCatchAll(__FUNCTION__)
}


/*
r.wilson  PLID 39357 10/28/2011
This function only does "client" side verification to ensure that the id is not invalid
This function will use the referenced string to hold the actual error
*/
BOOL CInquiryDlg::IsIdInvalid(int id, CString& strErrors )
{
	BOOL bErrorsFound = FALSE;
	
	if (id <= 0)
	{
		//if negative or 0, don't change
		strErrors += "Patient ID must be greater than 0";
		bErrorsFound = TRUE;
	}

	else if (id >= 2147483646)
	{
		strErrors += "Practice cannot store Patient IDs greater than 2,147,483,645.\nPlease enter a smaller Patient ID number.";
		bErrorsFound = TRUE;
	}

	return bErrorsFound;

}