// NexWebEditObjectsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practiceRC.h"
#include "NexWebEditObjectsDlg.h"
#include "NexWebPatientInfoDlg.h"
#include "NexWebExtraPatientInfoDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "Duplicate.h"
#include "GlobalNexWebUtils.h"
#include "WellnessDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNexWebEditObjectsDlg dialog


CNexWebEditObjectsDlg::CNexWebEditObjectsDlg(long nPersonID, BOOL bIsNewPatient, CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebEditObjectsDlg::IDD, pParent)
	, m_PatientInfoDlg(pParent)
	, m_ExtraPatientInfoDlg(pParent)
	, m_ApptListDlg(pParent)
	, m_PatientNotesDlg(pParent)
	, m_ToDoTaskDlg(pParent)
{
	m_nPersonID = nPersonID;
	m_bIsNewPatient = bIsNewPatient;
	//{{AFX_DATA_INIT(CNexWebEditObjectsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNexWebEditObjectsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebEditObjectsDlg)
	DDX_Control(pDX, IDC_NEXWEB_PREVIEW, m_Preview);
	DDX_Control(pDX, IDC_NEXWEB_CANCEL, m_Cancel);
	DDX_Control(pDX, IDC_NEXWEB_DELETE, m_Delete);
	DDX_Control(pDX, IDC_NEXWEB_IMPORT, m_Import);
	//}}AFX_DATA_MAP
}

BEGIN_EVENTSINK_MAP(CNexWebEditObjectsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebEditObjectsDlg)
	ON_EVENT(CNexWebEditObjectsDlg, IDC_NEXWEB_TAB, 1 /* SelectTab */, OnSelectTabNexWebTab, VTS_I2 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CNexWebEditObjectsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebEditObjectsDlg)
	ON_BN_CLICKED(IDC_NEXWEB_IMPORT, OnNexwebImport)
	ON_BN_CLICKED(IDC_NEXWEB_DELETE, OnNexwebDelete)
	ON_BN_CLICKED(IDC_NEXWEB_PREVIEW, OnNexwebPreview)
	ON_BN_CLICKED(IDC_NEXWEB_CANCEL, OnNexwebCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebEditObjectsDlg message handlers

BOOL CNexWebEditObjectsDlg::OnInitDialog() 
{
	// (e.lally 2009-01-26) PLID 32813 - Added try/catch
	try{
		CNxDialog::OnInitDialog();
		
		//TODO: Pull the information from the settings table

		m_Preview.AutoSet(NXB_PRINT_PREV);
		m_Cancel.AutoSet(NXB_CANCEL);
		m_Delete.AutoSet(NXB_DELETE);
		m_Import.AutoSet(NXB_OK);
		
		//For right now, we are going to default to the Patients and Emergency Contact Info only
		//First tab will be patient, second tab contact info
		m_tab = GetDlgItemUnknown(IDC_NEXWEB_TAB);
		if (m_tab == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDCANCEL);
			return 0;
		}

		//TODO: load this from settings
		m_tab->Size = 5;

		m_tab->PutLabel(0, "Patient");
		m_tab->PutLabel(1, "Employer/Emerg. Contact Info");
		m_tab->PutLabel(2, "Appointment");
		m_tab->PutLabel(3, "Patient Notes");
		m_tab->PutLabel(4, "ToDo Tasks");


		m_PatientInfoDlg.SetPersonID(m_nPersonID);
		m_ExtraPatientInfoDlg.SetPersonID(m_nPersonID);
		m_ApptListDlg.SetPersonID(m_nPersonID, m_bIsNewPatient);
		m_PatientNotesDlg.SetPersonID(m_nPersonID, m_bIsNewPatient);
		//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
		m_ToDoTaskDlg.SetPersonID(m_nPersonID, m_bIsNewPatient);

		CRect rect; 
		GetDlgItem(IDC_NEXWEB_TAB)->GetClientRect(rect);

		m_PatientInfoDlg.Create(IDD_NEXWEB_PATIENT_INFO_DLG, this);
		m_ExtraPatientInfoDlg.Create(IDD_NEXWEB_EXTRA_PATIENT_INFO_DLG, this);
		m_ApptListDlg.Create(IDD_NEXWEB_APPT_LIST_DLG, this);
		m_PatientNotesDlg.Create(IDD_NEXWEB_PATIENT_NOTES_DLG, this);
		//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
		m_ToDoTaskDlg.Create(IDD_NEXWEB_TODO_DLG, this);
		
		m_PatientInfoDlg.SetWindowPos(&wndTop, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		m_ExtraPatientInfoDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
		m_ApptListDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
		m_PatientNotesDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
		//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
		m_ToDoTaskDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);

		// (e.lally 2009-01-26) PLID 32813 - Bulk cache preferences
		g_propManager.CachePropertiesInBulk("NexWebEditObjects", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AssignNewPatientSecurityCode' OR "
			// (j.jones 2010-01-15 09:02) - PLID 31927 - added NewPatientsDefaultTextMessagePrivacy
			"Name = 'NewPatientsDefaultTextMessagePrivacy' "
			")",
			_Q(GetCurrentUserName()));
		}NxCatchAll("Error loading NexWebEditObjects")
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CNexWebEditObjectsDlg::OnSelectTabNexWebTab(short newTab, short oldTab)  {

	if (newTab != oldTab) {

		CRect rect;
		GetDlgItem(IDC_NEXWEB_TAB)->GetClientRect(rect);
		
		switch(newTab) {

			case 0:
				m_PatientInfoDlg.SetWindowPos(&wndTop, 8, rect.Height() + 12, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);
				m_ExtraPatientInfoDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ApptListDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_PatientNotesDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ToDoTaskDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				
			break;
			case 1:	
				m_PatientInfoDlg.SetWindowPos(&wndTop, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
				m_ExtraPatientInfoDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				m_ApptListDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_PatientNotesDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ToDoTaskDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
			break;

			case 2: //Appt dlg
				m_PatientInfoDlg.SetWindowPos(&wndTop, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
				m_ExtraPatientInfoDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ApptListDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				m_PatientNotesDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ToDoTaskDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
			break;
			case 3:  //Patient Notes
				m_PatientInfoDlg.SetWindowPos(&wndTop, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
				m_ExtraPatientInfoDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ApptListDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_PatientNotesDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
				m_ToDoTaskDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
			break;
			//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
			case 4:  //ToDo tasks
				m_PatientInfoDlg.SetWindowPos(&wndTop, 8, rect.Height() + 12, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
				m_ExtraPatientInfoDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ApptListDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_PatientNotesDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_HIDEWINDOW);
				m_ToDoTaskDlg.SetWindowPos(&wndBottom, 8, rect.Height() + 12, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
			break;
			
			default:
			break;
		}
	}


}

long CNexWebEditObjectsDlg::GetPersonID() {

	return m_nPersonID;
}


BOOL CNexWebEditObjectsDlg::CommitNexWebTransaction() 
{


	//check to make sure they filled out their appts tab before they tried to import
	//this function takes care of our message boxes and lets us know whether we continue or not
	if (! m_ApptListDlg.CheckExistingApptData()) {
		//it told us that we shouldn't continue
		return FALSE;
	}

	//check to make sure all the tabs have valid information on them
	//currently this only checks that dates are valid
	CString strError = "";
	if (! (m_PatientInfoDlg.ValidateData() && m_ExtraPatientInfoDlg.ValidateData())) {
		
		strError.Format("The following fields have invalid information and must be fixed before this patient can be imported: \n%s%s\n",
			m_PatientInfoDlg.m_strError, m_ExtraPatientInfoDlg.m_strError);
	}
	if (! m_ApptListDlg.ValidateData()) {

		CString strTmp;
		strTmp.Format("The following appointments have invalid information and must be fixed before this patient can be imported: \n%s\n", 
			m_ApptListDlg.m_strError);
		strError += strTmp;
	}
	if (! m_PatientNotesDlg.ValidateData()) {

		CString strTmp;
		strTmp.Format("The following patient notes have invalid information and must be fixed before this patient can be imported: \n%s", m_PatientNotesDlg.m_strError);
		strError += strTmp;
	}
	//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
	if (! m_ToDoTaskDlg.ValidateData()) {

		CString strTmp;
		strTmp.Format("The following ToDo tasks have invalid information and must be fixed before this patient can be imported: \n%s", m_ToDoTaskDlg.m_strError);
		strError += strTmp;
	}
	if (! strError.IsEmpty()) {
		
		MessageBox(strError);
		
		//now clear it so it doesn't double
		strError = "";
		m_PatientNotesDlg.m_strError = "";
		m_ApptListDlg.m_strError = "";
		m_PatientInfoDlg.m_strError = "";
		m_ExtraPatientInfoDlg.m_strError = "";
		m_ToDoTaskDlg.m_strError = "";
		return FALSE;
	}

	
	

	//They are sure, let's proceed
		long nNewPersonID, nNewUserDefinedID;
		BOOL bSuccess = TRUE;

		BOOL bMergeWithExisting = FALSE;

		if(m_bIsNewPatient){
			//(e.lally 2008-02-28) PLID 27379 - check for duplicates before we create a new patient
			CDuplicate dlgDuplicate(this); //patients
			dlgDuplicate.SetStatusFilter(EStatusFilterTypes(esfPatient|esfProspect|esfPatientProspect));
			dlgDuplicate.SetUseMergeButton(TRUE);

			CString strFirst, strMiddle, strLast;
			strFirst = m_PatientInfoDlg.GetParameterFieldValue(IDC_NEXWEB_FIRST_NAME);
			strMiddle = m_PatientInfoDlg.GetParameterFieldValue(IDC_NEXWEB_MIDDLE_NAME);
			strLast = m_PatientInfoDlg.GetParameterFieldValue(IDC_NEXWEB_LAST_NAME);

			//(e.lally 2008-02-28) PLID 27379 - Use just the current name parameters for finding duplicates
			if(dlgDuplicate.FindDuplicates(_Q(strFirst), _Q(strLast), _Q(strMiddle))){
				int iResult = dlgDuplicate.DoModal();
				
				if (iResult == ID_CANCEL_NEW_PATIENT) {
					//cancel the duplicate dlg and go back to editing
					return FALSE;
				}
				else if (iResult == ID_CREATE_NEW_PATIENT) {
					//Save as new patient
				}
				else if(iResult == ID_MERGE_WITH_SELECTED) {
					//Update existing patient
					m_bIsNewPatient = FALSE;
					bMergeWithExisting = TRUE;
					nNewPersonID = dlgDuplicate.m_nSelPatientId;
				}
				else{
					//cancel this screen and go to selected patient
					CMainFrame *p = GetMainFrame();
					CNxTabView *pView;

					nNewPersonID = dlgDuplicate.m_nSelPatientId;

					if (nNewPersonID != GetActivePatientID()) {
						if(!p->m_patToolBar.DoesPatientExistInList(nNewPersonID)) {
							if(IDNO == MessageBox("This patient is not in the current lookup. \n"
								"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
								return FALSE;
							}
						}
						//TES 1/7/2010 - PLID 36761 - This function may fail now
						if(!p->m_patToolBar.TrySetActivePatientID(nNewPersonID)) {
							return FALSE;
						}
					}	

					if(p->FlipToModule(PATIENT_MODULE_NAME)) {

						pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
						if (pView) 
						{	if(pView->GetActiveTab()!=0)
								pView->SetActiveTab(0);
						}

						p->UpdateAllViews();

						//cancel the NexWeb edit dlg, user will have to exit the NexWeb import dlg.
						EndDialog(0);
					}
					return FALSE;

				}
			}
		}

		if (m_bIsNewPatient) {

			// (b.cardillo 2007-09-21 17:55) - PLID 27489 - Made a friendly message for creating a new patient
			//warn them what they are doing
			if (MsgBox(MB_YESNO|MB_ICONQUESTION, 
				"You have chosen to import data for a new patient.  All selected information from all tabs will be included in the import.\r\n\r\n"
				"Are you ready to add the new patient?") == IDNO) {
				
				//they aren't sure
				return FALSE;
			}

			
			try { 
				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				CSqlTransaction trans(__FUNCTION__);
				trans.Begin();
				//we have to create a record for them in PersonT AND PatientT
				nNewPersonID = NewNumber("PersonT", "ID");
				nNewUserDefinedID = NewNumber("PatientsT", "UserDefinedID");
				//(e.lally 2009-01-26) PLID 32813 - Ideally, we'd get the new IDs and insert in the same sql call,
				//but since we are inside a transaction already, I am going to leave it be. I will, however, combine the next
				//couple sql statements.
				//We can assume they have the NexWeb license.

				CString strRandomSecurityCode = "NULL";
				// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Always (1)
				if(GetRemotePropertyInt("AssignNewPatientSecurityCode", 1, 0, "<None>", true) != 0){
					//(e.lally 2009-01-26) PLID 32813 - This gives us a random, unique security code, but does not
					// *guarantee* that it will remain unique before we can store it. The probably of there being 
					//a conflict is super low though.
					strRandomSecurityCode = "'" + GenerateUniquePatientSecurityCode(GetRemoteData()) + "'";
				}

				// (j.jones 2010-01-15 09:02) - PLID 31927 - check the default text message privacy field
				long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);

				// (e.lally 2009-06-30) PLID 34000 - Capture the userID of the user committing this data to the patient list.
				// (j.jones 2010-01-15 09:01) - PLID 31927 - supported defaulting the text message privacy field
				ExecuteSql("INSERT INTO PersonT (ID, UserID, TextMessage) VALUES (%li, %li, %li)\r\n"
				// (b.cardillo 2007-09-21 14:14) - PLID 26244 - Made it default the record to a CurrentStatus of 2, or 
				// "prospect", in keeping with the default of NxWeb.dll.
				//(e.lally 2009-01-26) PLID 32813 - Include our SecurityCode preference
				"INSERT INTO PatientsT (PersonID, UserDefinedID, CurrentStatus, SecurityCode) "
				"VALUES (%li, %li, 2, %s) \r\n", nNewPersonID, GetCurrentUserID(), nTextMessagePrivacy,
				nNewPersonID, nNewUserDefinedID, strRandomSecurityCode);

				// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
				ExecuteParamSql(
					"DECLARE @SecurityGroupID INT\r\n"
					"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
					"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
					"BEGIN\r\n"
					"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, {INT})\r\n"
					"END\r\n", nNewPersonID);

				// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
				UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), nNewPersonID);

				m_PatientInfoDlg.SaveInfo(nNewPersonID);
				m_ExtraPatientInfoDlg.SaveInfo(nNewPersonID);
				m_PatientNotesDlg.SaveInfo(nNewPersonID);
				//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
				m_ToDoTaskDlg.SaveInfo(nNewPersonID);
				trans.Commit();
				
				// (d.singleton 2011-10-11 14:29) - PLID 42102 - audit for security code
				strRandomSecurityCode.Replace("'", "");
				AuditEvent(nNewPersonID, GetExistingPatientName(nNewPersonID) , BeginNewAuditEvent(), aeiPatientSecurityCode, nNewPersonID, "", strRandomSecurityCode, aepHigh, aetChanged);

			}NxCatchAllCall("Error Saving Patient", bSuccess = FALSE;);

			if (bSuccess) {
				//take all the patient information out of the table
				ExecuteSql("Delete FROM NexWebTransactionsT WHERE PersonID = %li AND (Field < 3000 OR Field > 4100)", m_nPersonID);
				
				//save the appointments
				bSuccess = m_ApptListDlg.SaveInfo(nNewPersonID);

				if (! bSuccess) {
					//something failed, update NexwebTransactionsT with the new username and password for the appt information
					//so they can run it again
					ExecuteSql("UPDATE NexWebTransactionsT SET PersonID = %li WHERE PersonID = %li", nNewPersonID, m_nPersonID);
				}
				else {

					//we were successful, delete the appt info from NexwebTransactionsT
					ExecuteSql("DELETE FROM NexWebTransactionsT WHERE PersonID = %li AND Field > 3000 AND Field < 4000", m_nPersonID);
				}
				
			}
		}
		//(e.lally 2008-02-28) PLID 27379 - Merge NexWeb data with an existing patient
		else if(bMergeWithExisting) {
			if (MsgBox(MB_YESNO|MB_ICONEXCLAMATION, 
				"This will merge all fields in all tabs into the database for this patient, "
				"updating only the blank information for the existing patient that is selected.\r\n\r\n"
				"This action is unrecoverable, are you sure you wish to do this?") == IDNO) {
				
				//they aren't sure
				return FALSE;
			}


			try {
				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				CSqlTransaction trans(__FUNCTION__);
				trans.Begin();
				m_PatientInfoDlg.SaveInfo(nNewPersonID,TRUE);
				m_ExtraPatientInfoDlg.SaveInfo(nNewPersonID, TRUE);
				m_PatientNotesDlg.SaveInfo(nNewPersonID);
				//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
				m_ToDoTaskDlg.SaveInfo(nNewPersonID);
				trans.Commit();
			}NxCatchAllCall("Error Saving Patient", bSuccess=FALSE;);
			
			if (bSuccess) {

				//take the patient information out of nexwebtransactionst
				ExecuteSql("DELETE FROM NexWebTransactionsT WHERE PersonID = %li AND (Field < 3000 OR Field > 4100) ", m_nPersonID);
				
				bSuccess = m_ApptListDlg.SaveInfo(nNewPersonID);

				if (bSuccess) {
					//delete from nexwebtransactionst
					ExecuteSql("DELETE FROM NexWebTransactionsT WHERE PersonID = %li AND Field > 3000 AND Field < 4000", m_nPersonID);
				}

				//We wait to update our member variable here so that the table checkers work correctly.
				m_nPersonID = nNewPersonID;
			}


		}
		else { //update existing patient

			// (b.cardillo 2007-09-21 17:55) - PLID 27489 - Moved this message inside the else block so it only gives such a 
			// strong warning if they're actually overwriting fields for an existing patient.
			//warn them what they are doing
			if (MsgBox(MB_YESNO|MB_ICONEXCLAMATION, 
				"This will import all enabled fields in all tabs into the database for this patient, overwriting any information that previously existed for these fields.  Disabled fields will be left intact.\r\n\r\n"
				"This action is unrecoverable, are you sure you wish to do this?") == IDNO) {
				
				//they aren't sure
				return FALSE;
			}

			
			try {
				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				CSqlTransaction trans(__FUNCTION__);
				trans.Begin();
				m_PatientInfoDlg.SaveInfo();
				m_ExtraPatientInfoDlg.SaveInfo();
				m_PatientNotesDlg.SaveInfo();
				//(e.lally 2007-05-17) PLID 26014 - Added ability to import ToDo Tasks
				m_ToDoTaskDlg.SaveInfo();
				trans.Commit();
			}NxCatchAllCall("Error Saving Patient", bSuccess=FALSE;);
			
			if (bSuccess) {

				//take the patient information out of nexwebtransactionst
				ExecuteSql("DELETE FROM NexWebTransactionsT WHERE PersonID = %li AND (Field < 3000 OR Field > 4100) ", m_nPersonID);
				
				bSuccess = m_ApptListDlg.SaveInfo();

				if (bSuccess) {
					//delete from nexwebtransactionst
					ExecuteSql("DELETE FROM NexWebTransactionsT WHERE PersonID = %li AND Field > 3000 AND Field < 4000", m_nPersonID);
				}
			}
		}
	
	
		//lets go ahead and call the table change messages
		if (m_bIsNewPatient) {
			//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here (NexWeb always imports as an Active Prospect)
			CClient::RefreshPatCombo(nNewPersonID, false, CClient::pcatActive, CClient::pcstProspect);
			CClient::RefreshTable(NetUtils::PatG1, nNewPersonID);
			CClient::RefreshTable(NetUtils::ReferralSourceT, nNewPersonID);
			CClient::RefreshTable(NetUtils::NoteCatsF, nNewPersonID);
			//ToDo list sends table checkers from its own dlg.
		}
		else {
			CClient::RefreshTable(NetUtils::PatCombo, m_nPersonID);
			CClient::RefreshTable(NetUtils::PatG1, m_nPersonID);
			CClient::RefreshTable(NetUtils::ReferralSourceT, m_nPersonID);
			CClient::RefreshTable(NetUtils::NoteCatsF, m_nPersonID);
			//ToDo list sends table checkers from its own dlg.
		}

		return bSuccess;
}

void CNexWebEditObjectsDlg::OnNexwebImport() 
{
	try{
		//(e.lally 2009-09-22) PLID 15116 - Moved the save code into the new commit function
		if(CommitNexWebTransaction()){
			//close the dialog
			EndDialog(0);
		}

	}NxCatchAll("Error in OnNexwebImport");
}

void CNexWebEditObjectsDlg::OnNexwebDelete() 
{

	if (MsgBox(MB_YESNO, "This will delete all this patient's information that was entered via NexWeb. \n Are you sure you wish to do this?") == IDNO) {
		return;
	}

	try {

		ExecuteSql("DELETE FROM NexWebTransactionsT WHERE PersonID = %li", m_nPersonID);

		//close the window
		OnOK();	
	}NxCatchAll("Error Deleting Transactions");
		
}

void CNexWebEditObjectsDlg::OnNexwebPreview() 
{

	try {

		CPtrArray paryFields;

		m_PatientInfoDlg.FillArrayWithFields(paryFields);
		m_ExtraPatientInfoDlg.FillArrayWithFields(paryFields);


		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(548)]);

		//(e.lally 2009-09-22) PLID 15116 - Commit the transaction before running the report
		if(!CommitNexWebTransaction()){
			//We didn't commit the data, don't run the report
			ClearRPIParameterList(&paryFields);
			return;
		}
		//close the dialogs
		EndDialog(2);


		RunReport(&infReport, &paryFields, true, GetMainFrame(), "NexWeb Imported Information");
		ClearRPIParameterList(&paryFields);	//DRT - PLID 18085 - Cleanup after ourselves

	}NxCatchAll("Error running report");
		
	
}

void CNexWebEditObjectsDlg::OnNexwebCancel() 
{
	EndDialog(0);
	
}

