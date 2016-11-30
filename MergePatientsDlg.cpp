// MergePatientsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MergePatientsDlg.h"
#include "EditFamilyDlg.h" // (a.walling 2006-11-21 13:38) - PLID 23621 - FamilyUtils:: namespace
#include "AuditTrail.h"
#include "GlobalUtils.h"
#include "EmrUtils.h"
#include "Rewards.h"
#include "HL7Utils.h"
#include "WellnessDataUtils.h"
#include "PatientsRc.h"
#include "HL7Client_Practice.h"  // (b.savon 2014-12-02 07:34) - PLID 58850

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.




using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMergePatientsDlg dialog


CMergePatientsDlg::CMergePatientsDlg(CWnd* pParent)
	: CNxDialog(CMergePatientsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMergePatientsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nCurrentID = -1;
	m_nNewID = -1;
	
	m_nCurrentUserDefinedID = -1; // (b.savon 2014-12-02 07:34) - PLID 58850 

	m_nCurrentFamilyID = -1;
	m_nNewFamilyID = -1;

	m_pNameFont = NULL;
}


void CMergePatientsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMergePatientsDlg)
	DDX_Control(pDX, IDC_UPDATE_DEMOGRAPHICS, m_btnUpdateDemographics);
	DDX_Control(pDX, IDC_NO_DEMOGRAPHICS, m_btnNoDemographics);
	DDX_Control(pDX, IDC_STATUS_PATIENT_NAME, m_nxstaticStatusPatientName);
	DDX_Control(pDX, IDC_STATUS_PATIENT_ID, m_nxstaticStatusPatientId);
	DDX_Control(pDX, IDC_STATUS_PATIENT_SSN, m_nxstaticStatusPatientSsn);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_PROMPT, m_rdoPrompt);
	DDX_Control(pDX, IDC_RADIO_USE_FROM_PT, m_rdoUseFrom);
	DDX_Control(pDX, IDC_RADIO_USE_INTO_PT, m_rdoUseInto);
	DDX_Control(pDX, IDC_CHK_SEND_MERGE_PATIENT_HL7, m_chkSendViaHL7);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMergePatientsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMergePatientsDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMergePatientsDlg message handlers

void CMergePatientsDlg::OnOK()
{
	try {
		if(m_pPatList->GetCurSel() < 0) {
			MsgBox("You must choose a patient from the list.");
			return;
		}

		m_nCurrentID = GetActivePatientID();
		m_nNewID = VarLong(m_pPatList->GetValue(m_pPatList->GetCurSel(), 0));

		if(m_nCurrentID == m_nNewID) {
			MsgBox("You cannot merge a patient to themself.  Please choose another patient.");
			return;
		}

		if(MessageBox("This is an unrecoverable operation.  Are you SURE you wish to do this?", "Practice", MB_YESNO) != IDYES)
			return;

		// (b.savon 2013-08-12 13:35) - PLID 55400 - only save if the user says they want to do the merge
		SaveInsuredPartyMergeBehavior();

		if(!EnsureCurrentPatient())
			return;

		if(!MovePatient())
			return;

		// (b.savon 2014-12-02 07:36) - PLID 58850 - Connectathon - Support new HL7 message type ADT^A40^ADT_A39 - Merge patient
		if (m_chkSendViaHL7.GetCheck() == BST_CHECKED){
			SendViaHL7();
		}

		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

void CMergePatientsDlg::OnCancel() 
{
	CDialog::OnCancel();
}

// (j.armen 2012-04-10 14:25) - PLID 49553 - Parameratized sql calls
BOOL CMergePatientsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (b.savon 2013-08-09 11:41) - PLID 55400 - Cache the insured party preference
		g_propManager.CachePropertiesInBulk("MergePatientsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'MergePatients_InsuredPartyInformation' "
			" OR Name = 'ShowMergePatientHL7' "
			")",
			_Q(GetCurrentUserName()));
		ApplyInsuredPartyMergeControl();

		long nPersonID = GetActivePatientID();

		// (c.haag 2008-05-01 12:18) - PLID 29866 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (b.savon 2014-12-02 07:36) - PLID 58850 - Connectathon - Support new HL7 message type ADT^A40^ADT_A39 - Merge patient
		if (GetRemotePropertyInt("ShowMergePatientHL7", 0, 0, "<None>") == 0){
			m_chkSendViaHL7.EnableWindow(FALSE);
			m_chkSendViaHL7.ShowWindow(SW_HIDE);
		}
		else{
			m_chkSendViaHL7.EnableWindow();
			m_chkSendViaHL7.ShowWindow(SW_SHOW);
		}

		CString strMessage;
		_RecordsetPtr prs = CreateParamRecordset("SELECT Count(*) AS ItemCount FROM NexWebTransactionsT WHERE PersonID = {INT}", nPersonID);
		long nCount = AdoFldLong(prs, "ItemCount");
		if (nCount == 1) {
			strMessage += FormatString("The current patient has 1 NexWeb transaction and cannot be merged into any other patient. \r\n");
		}
		else if (nCount > 1) {
			strMessage += FormatString("The current patient has %li NexWeb transactions and cannot be merged into any other patient. \r\n", nCount);
		}
		prs->Close();

		prs = CreateParamRecordset("SELECT Count(*) AS ItemCount FROM NexWebTransactionSourceT WHERE PersonID = {INT}", nPersonID);
		nCount = AdoFldLong(prs, "ItemCount");
		if (nCount == 1) {
			strMessage += FormatString("The current patient has 1 NexWeb transaction source entry and cannot be merged into any other patient. \r\n");
		}
		else if (nCount > 1) {
			strMessage += FormatString("The current patient has %li NexWeb transaction source entries and cannot be merged into any other patient. \r\n");
		}
		prs->Close();

		// (b.cardillo 2006-11-29 09:58) - PLID 23674 - Make sure the current patient doesn't have locked 
		// EMNs.  If it does, we return here and make it so the dialog never appears, because the user 
		// would never be able to merge this patient, no matter who he tries to merge it with.
		prs = CreateParamRecordset("SELECT Count(*) AS ItemCount FROM EMRMasterT WHERE PatientID = {INT} AND Status = 2", nPersonID);
		nCount = AdoFldLong(prs, "ItemCount");
		if (nCount > 0) {
			strMessage += FormatString("The current patient has locked EMNs and cannot be merged into any other patient.\r\n");
		}
		prs->Close();

		if (!strMessage.IsEmpty()) {
			strMessage = strMessage + "However, the reverse may be possible (another patient may be able to be merged into this one) as long as the other patient doesn't have locked EMNs or NexWeb transactions of its own.";
			MessageBox(strMessage);
			CDialog::OnCancel();
			return FALSE;
		}


		m_pPatList = BindNxDataListCtrl(IDC_PAT_LIST, false);

		//DRT 10/6/2006 - PLID 22585 - Remove the current patient from the list, that's just more confusing.
		CString strWhere;
		strWhere.Format("ID > 0 AND CurrentStatus <> 4 AND ID <> %li", nPersonID);
		m_pPatList->WhereClause = _bstr_t(strWhere);
		m_pPatList->Requery();

		//leave no selection, force the user to act
		//m_pPatList->SetSelByColumn(0, long(GetActivePatientID()));

		CheckDlgButton(IDC_NO_DEMOGRAPHICS, TRUE);


		//DRT 10/6/2006 - plid 22585 - Clarify which patients are being merged.
		_RecordsetPtr prsLoad = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name, UserDefinedID, SocialSecurity "
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"WHERE PersonT.ID = {INT}", nPersonID);
		if(!prsLoad->eof) {
			CString strName = AdoFldString(prsLoad, "Name", "");
			CString strSSN = AdoFldString(prsLoad, "SocialSecurity", "");
			// (b.savon 2014-12-02 11:54) - PLID 58850 - store in member
			m_nCurrentUserDefinedID = AdoFldLong(prsLoad, "UserDefinedID");

			SetDlgItemText(IDC_STATUS_PATIENT_NAME, strName);
			CString str;
			// (b.savon 2014-12-02 11:54) - PLID 58850 - use member
			str.Format("ID:  %li", m_nCurrentUserDefinedID);
			SetDlgItemText(IDC_STATUS_PATIENT_ID, str);
			str.Format("SSN:  %s", strSSN);
			SetDlgItemText(IDC_STATUS_PATIENT_SSN, str);
		}
		prsLoad->Close();

		//set the font for the name to be larger
		if(m_pNameFont == NULL) {
			m_pNameFont = new CFont;
			m_pNameFont->CreateFont(18, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
				DEFAULT_PITCH, _T("Arial"));
			GetDlgItem(IDC_STATUS_PATIENT_NAME)->SetFont(m_pNameFont);
		}

		// (a.walling 2006-11-21 14:11) - PLID 23621 - initialize the family member variables
		m_nCurrentFamilyID = -1;
		m_nNewFamilyID = -1;

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Runs various checks on the data to ensure that the current patient is allowed to be moved.
//Any checks that will cause a patient to fail should be placed here.
//Return true if we should proceed, false otherwise.  If return false here, place a message
//box to let the user know why it failed.
// (j.armen 2012-04-10 14:25) - PLID 49553 - Parameratized sql calls
BOOL CMergePatientsDlg::EnsureCurrentPatient()
{
	BeginWaitCursor();

	try {
		//	We are going to loop through all resp types and compare between the 2 patients.  If any resp type
		//exists in both, but has the same ins co, we can just merge them together.  If any resp type exists in
		//both, but has differing insurance companies, we will stop and prevent the merge from happening.  Inactive
		//responsibilities do not need to be checked, since you can have as many of those as you like.
		// (b.savon 2013-08-09 11:52) - PLID 55400 - Now controlled by a preference; if the pref is prompt, then and only
		// then, run the query, do the logic, etc.
		if( PromptForInsurance() ){
			_RecordsetPtr prsResps = CreateParamRecordset("SELECT ID FROM RespTypeT WHERE ID <> -1");
			while(!prsResps->eof) {
				long nRespType = AdoFldLong(prsResps, "ID");

				//If this query returns records, then the 2 patients have the same ins co for this resp type, and
				//we are safe to continue.  If it does not return records, then they have a mismatched insurance.
				if(!ReturnsRecordsParam("SELECT * "
					"FROM "
					"(SELECT * FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT}) OldPartyT "
					"INNER JOIN "
					"(SELECT * FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT}) NewPartyT "
					"ON OldPartyT.InsuranceCoID = NewPartyT.InsuranceCoID", m_nCurrentID, nRespType, m_nNewID, nRespType))
				{
					//we also need to check for null values, if anything is null we're gtg
					if(!ReturnsRecordsParam("SELECT * FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT}", m_nCurrentID, nRespType) ||
						!ReturnsRecordsParam("SELECT * FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT}", m_nNewID, nRespType)) {
							//one or both have no insurance, we're all set to go, do nothing
					}
					else {
						MsgBox("These patients cannot be merged because there is conflicting insurance information.\r\n"
							"Please examine the patients to ensure that the insurance companies match for all responsibilities.");
						return false;		//cannot continue
					}
				}

				prsResps->MoveNext();
			}
		}

		// (a.walling 2006-11-21 13:38) - PLID 23621 - Family information
		{
			m_nCurrentFamilyID = FamilyUtils::GetFamilyID(m_nCurrentID);
			m_nNewFamilyID = FamilyUtils::GetFamilyID(m_nNewID);

			if ( (m_nNewFamilyID != -1) && (m_nCurrentFamilyID != -1) )
			{
				// two families!
				if (m_nNewFamilyID == m_nCurrentFamilyID) {
					// but they are equal.. no problem.
					// overwrite with the new.
					MsgBox("These patients are in the same family, but have different immediate relatives. The old one will be overwritten.");
				} else {
					MsgBox("These patients are in different families. Please remove one or both from a family before merging.");
					return false; // cannot continue
				}
			}
		}

		//if we reach here, all is well
		return true;
	} NxCatchAll("Error ensuring patient information.");

	return false;
}

// (j.armen 2012-04-10 14:14) - PLID 49553 - Parameratized
CSqlFragment CMergePatientsDlg::GenerateMoveString(const CString& strTable, const CString& strField)
{
	if (m_nNewID == -1 || m_nCurrentID == -1) {
		ThrowNxException("New / current ID is invalid (%li, %li)", m_nNewID, m_nCurrentID);
	}

	// (a.walling 2009-09-08 09:28) - PLID 31094 - We already have a current and new ID in member variables, set from the OnOK handler!
	return CSqlFragment("UPDATE {CONST_STRING} SET {CONST_STRING} = @NewID WHERE {CONST_STRING} = @CurrentID\r\n",
		strTable, strField, strField);
}

//Does the physical moving of the data for this patient.  Returns true if the patient successfully
//moved, false otherwise.  Prompt the user with a message telling them why if failure.
BOOL CMergePatientsDlg::MovePatient()
{
	BeginWaitCursor();

	//This function is split into 3 phases.  1) Moving things that require checking the data and possible user
	//interaction to determine the correct outcome.  2)  Data we can cleanly move without error checking.
	//3)  Auditing the move and deleting the old patient.

	bool bReturn = false;		//return false if any failures
	CSqlFragment sqlExecute;

	try {
		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		// (j.armen 2012-04-10 14:15) - PLID 49553 - Parameratize using a CSqlFragment
		CSqlTransaction trans;
		trans.Begin();

		//groups are special, there's a constraint that a pt can't exist twice, so if the dest
		//patient is already in a group, we can't update this new one.  Generate a query to delete
		//the new person from all groups that the old person exists in.  then we'll do the regular
		//update string and all will be well.
		sqlExecute += CSqlFragment("DELETE FROM GroupDetailsT WHERE PersonID = @NewID AND GroupID IN (SELECT GroupID FROM GroupDetailsT WHERE PersonID = @CurrentID)\r\n");

		//Generate a really big SQL statement of everything we need to do
		// (j.jones 2011-07-29 13:36) - PLID 30971 - change the RegardingID of PracYaks
		sqlExecute += GenerateMoveString("MessagesT", "RegardingID");
		sqlExecute += GenerateMoveString("ToDoList", "PersonID");
		
		//(s.dhole 9/3/2014 2:53 PM ) - PLID 63552 Merge PatientRemindersSentT history
		sqlExecute += GenerateMoveString("PatientRemindersSentT", "PatientID");

		sqlExecute += GenerateMoveString("Notes", "PersonID");
		sqlExecute += GenerateMoveString("AppointmentsT", "PatientID");
		sqlExecute += GenerateMoveString("WaitingListT", "PatientID");
		sqlExecute += GenerateMoveString("MailSent", "PersonID");
		sqlExecute += GenerateMoveString("PatientMedications", "PatientID");
		// (a.walling 2009-04-28 09:40) - PLID 34046 - Also surescripts messages
		sqlExecute += GenerateMoveString("SureScriptsMessagesT", "PatientID");
		sqlExecute += GenerateMoveString("EMRMasterT", "PatientID");
		sqlExecute += GenerateMoveString("EMRGroupsT", "PatientID");
		sqlExecute += GenerateMoveString("ProcInfoT", "PatientID");
		sqlExecute += GenerateMoveString("LaddersT", "PersonID");
		sqlExecute += GenerateMoveString("EventsT", "PatientID");
		sqlExecute += GenerateMoveString("PatientInterestsT", "PatientID");		//TODO:  This table is no longer used, but a report or 2 reads from it, so we should update it.
		sqlExecute += GenerateMoveString("PrintedSuperbillsT", "PatientID");
		sqlExecute += GenerateMoveString("CaseHistoryT", "PersonID");
		// (z.manning 2009-05-06 17:15) - PLID 28529 - Referral orders
		sqlExecute += GenerateMoveString("ReferralOrdersT", "PatientID");
		// (z.manning 2009-05-14 14:04) - PLID 28554 - Order sets
		sqlExecute += GenerateMoveString("OrderSetsT", "PatientID");
		// (d.thompson 2009-05-18) - PLID 34232 - Immunization history
		sqlExecute += GenerateMoveString("PatientImmunizationsT", "PersonID");
		// (a.walling 2009-06-05 17:27) - PLID 34410
		sqlExecute += GenerateMoveString("AdvanceDirectivesT", "PatientID");

		// (z.manning 2010-01-06 11:05) - PLID 36767 - Handle EMR problems
		sqlExecute += GenerateMoveString("EmrProblemsT", "PatientID");
		sqlExecute += CSqlFragment(
			"UPDATE EmrProblemLinkT SET EmrRegardingID = @NewID "
			"WHERE EmrRegardingType = {CONST_INT} AND EmrRegardingID = @CurrentID\r\n", eprtUnassigned);

		// (a.walling 2007-05-21 15:15) - PLID 26079 - Handle reward points
		// it is possible that the patients being merged have references to eachother
		// for referral points.
		sqlExecute += CSqlFragment(
			"UPDATE RewardHistoryT SET Deleted = {CONST_INT}, DeletedDate = GetDate()\r\n"
			"WHERE Deleted = 0 AND ( (PatientID = @NewID AND RefPatientID = @CurrentID) OR (PatientID = @CurrentID AND RefPatientID = @NewID) );\r\n",
			Rewards::erdrRemoved);
		
		sqlExecute += GenerateMoveString("RewardHistoryT", "PatientID");
		sqlExecute += GenerateMoveString("RewardHistoryT", "RefPatientID");
		
		//we cannot have duplicate medications
		sqlExecute += CSqlFragment(
			"UPDATE CurrentPatientMedsT SET PatientID = @NewID "
			"WHERE PatientID = @CurrentID AND MedicationID NOT IN (SELECT MedicationID FROM CurrentPatientMedsT WHERE PatientID = @NewID)\r\n");
		
		sqlExecute += GenerateMoveString("MedScheduleT", "PatientID");
		sqlExecute += GenerateMoveString("PatientAllergyT", "PersonID");
		sqlExecute += GenerateMoveString("MultiReferralsT", "PatientID");
		sqlExecute += GenerateMoveString("ResponsiblePartyT", "PatientID");
		sqlExecute += GenerateMoveString("HL7IDLinkT", "PersonID");
		sqlExecute += GenerateMoveString("GroupDetailsT", "PersonID");

		//DRT 7/1/2004 - PLID 13291 - Gift certificates should move too.
		sqlExecute += GenerateMoveString("GiftCertificatesT", "PurchasedBy");
		sqlExecute += GenerateMoveString("GiftCertificatesT", "ReceivedBy");
		// (s.dhole 05/15/2014) - PLID 62157 - move medication reconciliation history
		sqlExecute += GenerateMoveString("PatientReconciliationT", "PatientID");
		
		//Custom Field Data - We can't duplicate keys here if something already exists
		sqlExecute += CSqlFragment("UPDATE CustomFieldDataT SET PersonID = @NewID WHERE PersonID = @CurrentID AND FieldID NOT IN (SELECT FieldID FROM CustomFieldDataT WHERE PersonID = @NewID)\r\n");
		// (j.armen 2011-06-27 13:39) - PLID 44253 - update to use new custom list structure
		sqlExecute += CSqlFragment("UPDATE CustomListDataT SET PersonID = @NewID WHERE PersonID = @CurrentID AND CustomListItemsID NOT IN (SELECT CustomListItemsID FROM CustomListDataT WHERE PersonID = @NewID)\r\n");

		//If we try to combine the patient & their partner, we get a silly record.  Delete it if so
		sqlExecute += GenerateMoveString("PartnerLinkT", "PatientID");
		sqlExecute += GenerateMoveString("PartnerLinkT", "PartnerID");
		sqlExecute += CSqlFragment("DELETE FROM PartnerLinkT WHERE PatientID = PartnerID;\r\n");

		//If we try to make a patient his own donor, update the silly record.
		sqlExecute += GenerateMoveString("PatientsT", "DonorID");
		sqlExecute += CSqlFragment("UPDATE PatientsT SET DonorID = NULL WHERE DonorID = PersonID;\r\n");

		//Insured Parties
		//all error checking was done in the Ensure function.  If we reach this point, then all insured parties are 
		//set, we just have to determine whether we need to transfer an existing one, or change the ID numbers
		AddInsuredMoveInfo(sqlExecute);

		//Billing Info
		//If we reach this point, then the above insurance checks have succeeded, and we can just simply move the data over
		//from the current patient to the new patient
		sqlExecute += GenerateMoveString("LineItemT", "PatientID");
		sqlExecute += GenerateMoveString("BillsT", "PatientID");
		sqlExecute += GenerateMoveString("ERemittanceHistoryT", "PatientID");

		//PLID 20183: Make it merge any NexWebLoginInfoT Records
		sqlExecute += GenerateMoveString("NexwebLoginInfoT", "PersonID");

		//PLID 21480: Merge Patient Labs
		sqlExecute += GenerateMoveString("LabsT", "PatientID");

		// (r.farnworth 2014-05-15 14:19) - PLID 62151 - Merge Education Resource Access
		sqlExecute += GenerateMoveString("EducationResourceAccessT", "PatientID");

		// (s.dhole 2011-02-24 11:25) - PLID 40535 Merge Patient Eye Precription
		sqlExecute += GenerateMoveString("LensRxT", "PersonID");
		// (s.dhole 2011-02-24 11:25) - PLID 40535 Merge Patient Eye Precription Order
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		sqlExecute += GenerateMoveString("GlassesOrderT", "PersonID");
		// (j.armen 2012-04-03 11:00) - PLID 48299 - Merge Recalls
		sqlExecute += GenerateMoveString("RecallT", "PatientID");
		// (v.maida 2016-03-09 17:41) - PLID 68564 - IagnosisOnlineVisitT
		sqlExecute += GenerateMoveString("IagnosisOnlineVisitT", "PatientID");

		sqlExecute += GenerateMoveString("PatientInvAllocationsT", "PatientID");
		sqlExecute += GenerateMoveString("EyeProceduresT", "PatientID");
		sqlExecute += GenerateMoveString("CycleDayInfoT", "PatientID");
		sqlExecute += GenerateMoveString("PostIVFT", "PatientID");
		sqlExecute += GenerateMoveString("IVFT", "PatientID");

		// option to update demographics or not
		UINT nUpdate = IsDlgButtonChecked(IDC_UPDATE_DEMOGRAPHICS);

		// (a.walling 2006-11-21 13:38) - PLID 23621 - Family information
		if (m_nCurrentFamilyID == m_nNewFamilyID) {
			// both the same family
			if (nUpdate) {
				// delete new, keep current
				sqlExecute += CSqlFragment("DELETE FROM PersonFamilyT WHERE PersonID = @NewID\r\n");
			} else {
				// delete current, keep new.
				sqlExecute += CSqlFragment("DELETE FROM PersonFamilyT WHERE PersonID = @CurrentID\r\n");
			}
		}
		// Ensure took care of the situation where the two patients were in different families.
		// and above took care of when they are the same.
		sqlExecute += GenerateMoveString("PersonFamilyT", "PersonID");
		
		//Move data in PatientsT and PersonT that needs transferred.,
		{
			//We need to be aware of differences in various links (mirror, inform, qb, etc)
			AddLinkMoveInfo(sqlExecute, "MirrorID");
			AddLinkMoveInfo(sqlExecute, "InformID");
			AddLinkMoveInfo(sqlExecute, "UnitedID");

			//Now we need to handle moving all the patient information, but do it based on the options chosen
		
			//we are only updating fields that are null in the new patient
			if(nUpdate) {
				sqlExecute += GeneratePatientDataString("MaritalStatus", "PatientsT");
				sqlExecute += GeneratePatientDataString("SpouseName", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployeeID", "PatientsT");
				sqlExecute += GeneratePatientDataString("NickName", "PatientsT");
				sqlExecute += GeneratePatientDataString("MainPhysician", "PatientsT");
				sqlExecute += GeneratePatientDataString("Occupation", "PatientsT");
				sqlExecute += GeneratePatientDataString("Employment", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerFirst", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerMiddle", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerLast", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerAddress1", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerAddress2", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerCity", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerState", "PatientsT");
				sqlExecute += GeneratePatientDataString("EmployerZip", "PatientsT");
				sqlExecute += GeneratePatientDataString("TypeOfPatient", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultReferringPhyID", "PatientsT");
				sqlExecute += GeneratePatientDataString("SuppressStatement", "PatientsT");
				sqlExecute += GeneratePatientDataString("ReferringPatientID", "PatientsT");
				sqlExecute += GeneratePatientDataString("PreferredContact", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultDiagID1", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultDiagID2", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultDiagID3", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultDiagID4", "PatientsT");
				// (j.jones 2014-03-14 13:34) - PLID 60719 - support default ICD-10 codes
				sqlExecute += GeneratePatientDataString("DefaultICD10DiagID1", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultICD10DiagID2", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultICD10DiagID3", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultICD10DiagID4", "PatientsT");
				// (a.walling 2006-10-18 11:26) - PLID 16059 - Move over the primary resp. party
				sqlExecute += GeneratePatientDataString("PrimaryRespPartyID", "PatientsT"); 
				sqlExecute += GeneratePatientDataString("First", "PersonT");
				sqlExecute += GeneratePatientDataString("Middle", "PersonT");
				sqlExecute += GeneratePatientDataString("Last", "PersonT");
				sqlExecute += GeneratePatientDataString("Address1", "PersonT");
				sqlExecute += GeneratePatientDataString("Address2", "PersonT");
				sqlExecute += GeneratePatientDataString("City", "PersonT");
				sqlExecute += GeneratePatientDataString("State", "PersonT");
				sqlExecute += GeneratePatientDataString("Zip", "PersonT");
				sqlExecute += GeneratePatientDataString("Gender", "PersonT");
				sqlExecute += GeneratePatientDataString("Suffix", "PersonT");
				sqlExecute += GeneratePatientDataString("Title", "PersonT");
				sqlExecute += GeneratePatientDataString("HomePhone", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivHome", "PersonT");
				sqlExecute += GeneratePatientDataString("WorkPhone", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivWork", "PersonT");
				sqlExecute += GeneratePatientDataString("Extension", "PersonT");
				sqlExecute += GeneratePatientDataString("CellPhone", "PersonT");
				sqlExecute += GeneratePatientDataString("OtherPhone", "PersonT");
				sqlExecute += GeneratePatientDataString("Email", "PersonT");
				sqlExecute += GeneratePatientDataString("Pager", "PersonT");
				sqlExecute += GeneratePatientDataString("Fax", "PersonT");
				sqlExecute += GeneratePatientDataString("BirthDate", "PersonT");
				sqlExecute += GeneratePatientDataString("SocialSecurity", "PersonT");
				sqlExecute += GeneratePatientDataString("WarningMessage", "PersonT");
				sqlExecute += GeneratePatientDataString("DisplayWarning", "PersonT");
				sqlExecute += GeneratePatientDataString("Company", "PersonT");
				sqlExecute += GeneratePatientDataString("CompanyID", "PersonT");
				sqlExecute += GeneratePatientDataString("EmergFirst", "PersonT");
				sqlExecute += GeneratePatientDataString("EmergLast", "PersonT");
				sqlExecute += GeneratePatientDataString("EmergHPhone", "PersonT");
				sqlExecute += GeneratePatientDataString("EmergWPhone", "PersonT");
				sqlExecute += GeneratePatientDataString("EmergRelation", "PersonT");
				sqlExecute += GeneratePatientDataString("Spouse", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivCell", "PersonT");
				sqlExecute += GeneratePatientDataString("PrefixID", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivEmail", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivOther", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivFax", "PersonT");
				sqlExecute += GeneratePatientDataString("PrivPager", "PersonT");
				sqlExecute += GeneratePatientDataString("TextMessage", "PersonT"); // (z.manning 2008-07-11 10:15) - PLID 30678

				// (j.politis 2015-07-07 09:07) - PLID 65201 - The following patient-related fields need to merge into the target patient when doing a patient merge, so long as they are blank in the target patient:
				sqlExecute += GeneratePatientDataString("Ethnicity", "PersonT");
				sqlExecute += GeneratePatientDataString("LanguageID", "PersonT");
				sqlExecute += GeneratePatientDataString("WarningCategoryID", "PersonT");
				sqlExecute += GeneratePatientDataString("Country", "PersonT");
				sqlExecute += GeneratePatientDataString("PCP", "PatientsT");
				sqlExecute += GeneratePatientDataString("OccupationCensusCodeID", "PatientsT");
				sqlExecute += GeneratePatientDataString("IndustryCensusCodeID", "PatientsT");
				sqlExecute += GeneratePatientDataString("DefaultInjuryDate", "PatientsT");
				sqlExecute += GeneratePatientDataString("AffiliatePhysID", "PatientsT");
			}
			else {
				// (a.walling 2006-10-18 11:26) - PLID 16059 - Move over the primary resp. party.
				// There MUST be a Primary Resp Party. We could easily add this to strExecute regardless
				// since it is run in the other case above, but this drives home the point.
				sqlExecute += GeneratePatientDataString("PrimaryRespPartyID", "PatientsT");
			}
		}

		// (j.fouts 2013-09-09 12:45) - PLID 58043 - Merge Eligibility
		sqlExecute += GenerateMoveString("SureScriptsEligibilityRequestT", "PatientID");

		//Now execute the big sql statement we've been generating
		// (b.cardillo 2009-07-09 15:39) - PLIDs 34369 and 34368 - We weren't updating patient wellness qualification records 
		// when a gender or dob changed; now we have our update query tell us if either changed, and if so we update them below.
		BOOL bUpdatedGender = FALSE, bUpdatedBirthdate = FALSE;
		{
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"DECLARE @NewID INT\r\n"
				"DECLARE @CurrentID INT\r\n"
				"SET @NewID = {INT}\r\n"
				"SET @CurrentID = {INT}\r\n"
				"DECLARE @nOldGender TINYINT, @dtOldBirthdate DATETIME \r\n"
				"SELECT @nOldGender = Gender, @dtOldBirthdate = Birthdate FROM PersonT WHERE ID = @NewID \r\n"
				"{SQL} \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT \r\n"
				" CONVERT(BIT, (CASE WHEN @nOldGender = Gender THEN 0 ELSE 1 END)) AS UpdatedGender, \r\n"
				" CONVERT(BIT, (CASE WHEN @dtOldBirthdate IS NULL THEN (CASE WHEN Birthdate IS NOT NULL THEN 1 ELSE 0 END) ELSE (CASE WHEN @dtOldBirthdate = Birthdate THEN 0 ELSE 1 END) END)) AS UpdatedBirthdate \r\n"
				"FROM PersonT WHERE ID = @NewID \r\n",
				m_nNewID, m_nCurrentID, sqlExecute);
			if (!prs->eof) {
				FieldsPtr pflds = prs->GetFields();
				bUpdatedGender = AdoFldBool(pflds, "UpdatedGender");
				bUpdatedBirthdate = AdoFldBool(pflds, "UpdatedBirthdate");
			} else {
				// We didn't update any records!  This condition was never caught before, so I'm not doing anything 
				// about it now other than ASSERT()ing because it's possible it was decided to be preferable to let 
				// the user silently discover that the patient they were updating no longer exists.  For debug it 
				// seemed worthwhile to be notified of this situation thought.
				ASSERT(FALSE);
			}
		}
		
		// (b.cardillo 2009-07-09 15:39) - PLIDs 34369 and 34368 - Now we know which field(s), if any, were updated, 
		// so update the corresponding patient wellness qualification records.
		if (bUpdatedGender) {
			UpdatePatientWellnessQualification_Gender(GetRemoteData(), m_nNewID);
		}
		if (bUpdatedBirthdate) {
			UpdatePatientWellnessQualification_Age(GetRemoteData(), m_nNewID);
		}

		//History documents - All files which are in the old patients folder need to be 
		//manually moved to the new patients folder.  Anything that is outside that should
		//remain where it is
		{
			//really, we just need to move all files out of the old and into the new, we already updated MailSent to have them correctly
			CString strNewPath = GetPatientDocumentPath(m_nNewID);
			CString strCurPath = GetPatientDocumentPath(m_nCurrentID);

			// (j.jones 2008-04-30 09:36) - PLID 28809 - moving documents now will
			// have better error checking, if MoveFile fails it will be logged,
			// and we'll try CopyFile instead, and if that fails it is logged and
			// also warned to the user

			CStringArray aryCouldNotMove;
			CStringArray aryCouldNotCopy;

			CFileFind ff;
			BOOL bFound = ff.FindFile(strCurPath ^ "*.*");
			while(bFound) {
				bFound = ff.FindNextFile();

				if(!ff.IsDots()) {

					CString strFile = ff.GetFileName();

					if (!MoveFile(strCurPath ^ strFile, strNewPath ^ strFile)) {
						//if moving failed, track that it failed
						aryCouldNotMove.Add(strFile);

						//try copying instead
						if (!CopyFile(strCurPath ^ strFile, strNewPath ^ strFile, TRUE)) {
							//even the copy failed, so track that it failed
							aryCouldNotCopy.Add(strFile);
						}
					}
				}
			}

			if (aryCouldNotMove.GetSize() > 0) {
				// Some files couldn't be moved automatically, only log this
				CString strErr;
				strErr.Format("The following file(s) could not be moved from '%s' to '%s'.\r\n\r\n", 
					strCurPath, strNewPath);
				for (long i=0; i<aryCouldNotMove.GetSize(); i++) {
					strErr += "   " + aryCouldNotMove.GetAt(i) + "\r\n";
				}
				Log("MovePatient document MoveFile failure: " + strErr);
			}

			if (aryCouldNotCopy.GetSize() > 0) {
				// Some files couldn't be copied at all, warn and log this.
				CString strErr;
				strErr.Format("The following file(s) could not be moved from '%s' to '%s'.\r\n\r\n", 
					strCurPath, strNewPath);
				for (long i=0; i<aryCouldNotMove.GetSize(); i++) {
					strErr += "   " + aryCouldNotMove.GetAt(i) + "\r\n";
				}
				strErr += "\r\nPlease move these files manually.";
				MsgBox("%s", strErr);
				Log("MovePatient document CopyFile failure: " + strErr);
			}
		}

		//Auditing & Deletion of old patient
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			CString str;
			//(e.lally 2005-11-08) PLID 18257 - The audit trail should better identify which patient
				//they are being merged into. The userdefined ID is in parentheses.
			str.Format("Merged into:  %s (%li)", VarString(m_pPatList->GetValue(m_pPatList->GetCurSel(), 1)), 
				VarLong(m_pPatList->GetValue(m_pPatList->GetCurSel(), 2)) );
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientMerged, m_nCurrentID, "", str, aepHigh, aetChanged);
		}

		// (a.walling 2006-11-21 13:38) - PLID 23621 - Cleanup family info
		FamilyUtils::CleanUp();

		// (a.walling 2010-09-08 13:16) - PLID 40377 - Use CSqlTransaction
		trans.Commit();

		bReturn = true;	//once we commit, this move is good to go.

		//Delete the patient
		GetMainFrame()->DeleteCurrentPatient(true);
		MsgBox("Patient has been successfully merged!");

		//now try to set the new person as the active patient
		//TES 1/7/2010 - PLID 36761 - This function may fail now
		GetMainFrame()->m_patToolBar.TrySetActivePatientID(m_nNewID);
		GetMainFrame()->UpdateAllViews();

		// (c.haag 2007-02-07 16:04) - PLID 24420 - Now that all is said and done, warn
		// the user if the new patient's current medications list differs from any medical
		// records
		// (c.haag 2007-04-05 17:52) - PLID 25524 - Do the same for allergies
		// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
			WarnMedicationDiscrepanciesWithEMR(m_nNewID);
			WarnAllergyDiscrepanciesWithEMR(m_nNewID);
		}

		// (z.manning 2009-01-09 10:15) - PLID 32663 - Update the new patient in HL7
		// (We don't currently support deleting patients through HL7)
		try {
			UpdateExistingPatientInHL7(m_nNewID);
		}NxCatchAll("CMergePatientsDlg::MovePatient - HL7");

	} NxCatchAll("Error merging into existing patient!");

	return bReturn;
}

// (j.armen 2012-04-10 14:17) - PLID 49553 - Parameratized
void CMergePatientsDlg::AddInsuredMoveInfo(CSqlFragment& sqlExecute)
{
	// (b.savon 2013-08-09 12:45) - PLID 55400 - Use preference now
	sqlExecute += CSqlFragment("/*Begin Insured Move*/");
	switch(GetInsuranceMergeBehavior())
	{
		case mpiiMergedIntoPatient:
			UsePatientToInsurance(sqlExecute);
			break;
		case mpiiMergedFromPatient:
			UsePatientFromInsurance(sqlExecute);
			break;
		case mpiiPrompt:
		default:
			PerformLegacyInsuranceBehavior(sqlExecute);
			break;
	}
	sqlExecute += CSqlFragment("/*End Insured Move*/");
}

//Takes a string (we will append SQL queries to it if necessary) and a field that we are looking up 
//information for comparison.
// (j.armen 2012-04-10 14:18) - PLID 49553 - Parameratized
void CMergePatientsDlg::AddLinkMoveInfo(CSqlFragment& sqlExecute, const CString& strField)
{
	//Generate the queries we need
	_RecordsetPtr prsCurrent = CreateParamRecordset("SELECT PersonID, {CONST_STRING} "
		"FROM PatientsT WHERE PersonID = {INT}", strField, m_nCurrentID);
	_RecordsetPtr prsNew = CreateParamRecordset("SELECT PersonID, {CONST_STRING} "
		"FROM PatientsT WHERE PersonID = {INT}", strField, m_nNewID);

	_variant_t varCurID = prsCurrent->Fields->Item[_bstr_t(strField)]->Value;
	_variant_t varNewID = prsNew->Fields->Item[_bstr_t(strField)]->Value;

	if(!strField.CompareNoCase("InformID")) {
		//for some silly reason, the informid = 0 when the patient is not linked, unlike all the others, so
		//we need to modify these appropriately
		if(varNewID.vt == VT_I4 && VarLong(varNewID) == 0)
			varNewID.vt = VT_NULL;
		if(varCurID.vt == VT_I4 && VarLong(varCurID) == 0)
			varCurID.vt = VT_NULL;
	}

	if(varNewID.vt == VT_I4) {
		//the new patient has an ID
		if(varCurID.vt == VT_I4) {
			//the current patient also has an ID
			//We can't update this, 
			MsgBox("Both patients have a %s number.  You will have to manually fix the current patient in the linked software.", strField);
		}
		//else, leave it as is
	}
	else {
		//the new patient has no ID
		if(varCurID.vt == VT_I4) {
			//the current patient has an ID, copy it over
			sqlExecute += CSqlFragment("UPDATE PatientsT SET {CONST_STRING} = {INT} WHERE PersonID = @NewID;\r\n", strField, VarLong(varCurID));
		}
		//else, noone has an ID
	}
}

// (j.armen 2012-04-10 14:19) - PLID 49553 - Parameratized
CSqlFragment CMergePatientsDlg::GeneratePatientDataString(const CString& strField, const CString& strTable)
{
	if (m_nNewID == -1 || m_nCurrentID == -1) {
		ThrowNxException("New / current ID is invalid (%li, %li)", m_nNewID, m_nCurrentID);
	}

	CString strID;
	if(strTable == "PatientsT")
		strID = "PersonID";
	else
		strID = "ID";

	//using the CASE in the where clause let's us just always filter on "IS NULL", no matter what type of
	//field it is
	return CSqlFragment(
		"UPDATE {CONST_STRING} SET {CONST_STRING} = \r\n"
		"	(SELECT {CONST_STRING} FROM {CONST_STRING} WHERE {CONST_STRING} = @CurrentID)\r\n"
		"WHERE {CONST_STRING} = @NewID AND "
		"	(CASE WHEN {CONST_STRING} = '' THEN NULL ELSE {CONST_STRING} END) IS NULL;\r\n", 
		strTable, strField, 
		strField, strTable, strID, 
		strID, 
		strField, strField);
}

void CMergePatientsDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();

	try {
		//cleanup font -- we have it saved in m_pNameFont
		if(m_pNameFont != NULL) {
			delete m_pNameFont;
			m_pNameFont = NULL;
		}

	} NxCatchAll("Error in OnDestroy");
}

// (b.savon 2013-08-09 11:51) - PLID 55400
BOOL CMergePatientsDlg::UseMergedIntoPatientInsurance()
{
	return GetInsuranceMergeBehavior() == mpiiMergedIntoPatient;
}

// (b.savon 2013-08-09 11:51) - PLID 55400
BOOL CMergePatientsDlg::PromptForInsurance()
{
	return GetInsuranceMergeBehavior() == mpiiPrompt;
}

// (b.savon 2013-08-09 11:51) - PLID 55400
BOOL CMergePatientsDlg::UseMergedFromPatientInsurance()
{
	return GetInsuranceMergeBehavior() == mpiiMergedFromPatient;
}

// (b.savon 2013-08-09 11:51) - PLID 55400
MergePatientInsuranceInformation CMergePatientsDlg::GetInsuranceMergeBehavior()
{
	try{
		return (MergePatientInsuranceInformation)GetRemotePropertyInt("MergePatients_InsuredPartyInformation", 0);
	}NxCatchAll(__FUNCTION__);

	return mpiiPrompt;
}

// (b.savon 2013-08-12 13:22) - PLID 55400
void CMergePatientsDlg::ApplyInsuredPartyMergeControl()
{
	switch(GetInsuranceMergeBehavior())
	{
		case mpiiMergedFromPatient:
			m_rdoUseFrom.SetCheck(BST_CHECKED);
			break;
		case mpiiMergedIntoPatient:
			m_rdoUseInto.SetCheck(BST_CHECKED);
			break;
		case mpiiPrompt:
		default:		
			m_rdoPrompt.SetCheck(BST_CHECKED);
			break;
	}
}

// (b.savon 2013-08-12 13:31) - PLID 55400
void CMergePatientsDlg::SaveInsuredPartyMergeBehavior()
{
	int nBehavior = (int)mpiiPrompt; //default
	if( m_rdoUseFrom.GetCheck() == BST_CHECKED ){
		nBehavior = (int)mpiiMergedFromPatient;
	}else if( m_rdoUseInto.GetCheck() == BST_CHECKED ){
		nBehavior = (int)mpiiMergedIntoPatient;
	}else{
		//Leave it at 0; default; prompt
	}
	
	if( nBehavior != (int)GetInsuranceMergeBehavior() ){
		SetRemotePropertyInt("MergePatients_InsuredPartyInformation", nBehavior);
	}
}

// (b.savon 2013-08-12 11:17) - PLID 55400 - Move this to a function
void CMergePatientsDlg::PerformLegacyInsuranceBehavior(CSqlFragment& sqlExecute)
{
//	(e.lally 2005-11-11) PLID 18272 - We need to account for inactive insurance or else there is a foreign key error
		//when the old patient is deleted.
	//Possible Conditions:
	//	1)  Current patient has an insured party, new patient does not.  Change the PatientID to move it over
	//	2)  Current patient has an insured party, new patient also has one.  The old insured party is lost, all bills
	//		and other related information must be transferred from old to new.
	//	3)  Current does not have an insured party, new patient does have one.  No change is required.
	//	4)  Neither the current nor the new patient have an insured party.  No changes are required.
	//	5)	Current patient has inactive insured parties, new patient does not. Change the PatientID to move it over.
	//	6)	Current patient has inactive insured parties, new patient also has inactive insured parties
	//		but not for the same insurance company. Change the PatientID to move it over.
	//	7)	Current patient has inactive insured parties, new patient also has an inactive insured party for the 
	//		SAME insurance company. Transfer bills and other related information from old to new for that particular 
	//		insured party.

	
	_RecordsetPtr prsResps = CreateParamRecordset("SELECT ID FROM RespTypeT");
	while(!prsResps->eof) {
		long nRespType = AdoFldLong(prsResps, "ID");

		//get the insured party for each of our patients
		// (b.savon 2013-08-09 12:06) - PLID 55400 - Get the Current and New pt ID based on the insured party behavior pref
		_RecordsetPtr prsCurrent = CreateParamRecordset("SELECT * FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT}", m_nCurrentID, nRespType);
		_RecordsetPtr prsNew = CreateParamRecordset("SELECT * FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT}", m_nNewID, nRespType);

		if(prsCurrent->eof) {
			//Condition 3 or 4
			//No changes need to happen
		}
		else{
			//Condition 1,2,5,6,or 7
			//There can be multiple insured parties that are inactive. Primary, secondary, and tertiary COULD have
			//multiple because the data allows it even though Practice tries to prevent it,
			//so it is possible for them to loop more than once as well.
			while(!prsCurrent->eof){
				long nCurrentInsuranceCoID = AdoFldLong(prsCurrent, "InsuranceCoID");
				if(prsNew->eof || !ReturnsRecordsParam("SELECT PersonID FROM InsuredPartyT WHERE PatientID = {INT} AND RespTypeID = {INT} AND InsuranceCoID = {INT}", m_nNewID, nRespType, nCurrentInsuranceCoID)){
					//Condition 1,5,or 6
					//New patient has inactive insured parties, but not this insurance co
					sqlExecute += CSqlFragment("UPDATE InsuredPartyT SET PatientID = @NewID WHERE PatientID = @CurrentID AND InsuranceCoID = {INT}\r\n", nCurrentInsuranceCoID);
				}
				else {
					//Condition 2 or 7
					//We will be losing the old insured party.  However, they may have financial information that relates to it,
					//and we must update all that information to point to the new insured party that will be merged into.
					long nCurrentPartyID = AdoFldLong(prsCurrent, "PersonID");
					long nNewPartyID = AdoFldLong(prsNew, "PersonID");

					// (b.savon 2013-08-12 10:44) - PLID 55400 - break this out into a function
					MoveInsuredPartyTransferInformation(nCurrentPartyID, nNewPartyID, sqlExecute);
				}//end else
				prsCurrent->MoveNext();
			}//end while prsCurrent is not eof
		}

		prsResps->MoveNext();
	}
}

// (b.savon 2013-08-12 11:17) - PLID 55400 - Handle to new case
void CMergePatientsDlg::UsePatientFromInsurance(CSqlFragment& sqlExecute)
{
	MigrateInsuredPartyInformation(mpiiMergedFromPatient, m_nCurrentID, m_nNewID, sqlExecute);
}

// (b.savon 2013-08-12 11:17) - PLID 55400 - Handle to new case
void CMergePatientsDlg::UsePatientToInsurance(CSqlFragment& sqlExecute)
{
	MigrateInsuredPartyInformation(mpiiMergedIntoPatient, m_nCurrentID, m_nNewID, sqlExecute);
}

// (b.savon 2013-08-12 11:17) - PLID 55400 - Create Utility
void CMergePatientsDlg::MigrateInsuredPartyInformation(MergePatientInsuranceInformation keepWho, long nDeletePatient, long nKeepPatient, CSqlFragment& sqlExecute)
{
	// Scenarios:
	//
	//	A -> B [Merge A Into B]
	//	
	//	Keep A [Keep Merged From]
	//  -------------------------
	//	1. A Empty + B Empty	-> Do Nothing
	//	2. A Has   + B Empty	-> UPDATE InsuredPartyT SET PatientID = B.PatientID WHERE PatientID = A.PatientID
	//	3. A Empty + B Has		-> UPDATE InsuredPartyT SET RespTypeID = -1 /*Inactive*/ WHERE PatientID = B.PatientID
	//	4. A Has   + B	Has		-> Merge
	//
	//	Keep B [Keep Merged Into]
	//	-------------------------
	//  1. A Empty + B Empty	-> Do Nothing
	//	2. A Has   + B Empty	-> UPDATE InsuredPartyT SET PatientID = B.PatientID WHERE PatientID = A.PatientID;  Switch the PatientIDs
	//							   UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PatientID = B.PatientID; Inactivate
	//	3. A Empty + B Has		-> Do Nothing
	//	4. A Has   + B Has		-> Merge
	_RecordsetPtr prsDeletePatient = CreateParamRecordset("SELECT PersonID, PatientID, InsuranceCoID FROM InsuredPartyT WHERE PatientID = {INT}\r\n", nDeletePatient);
	_RecordsetPtr prsKeepPatient = CreateParamRecordset("SELECT PersonID, PatientID, InsuranceCoID FROM InsuredPartyT WHERE PatientID = {INT}\r\n", nKeepPatient);

	//No Insured party info; Do Nothing 
	//Scenario A1 and B1
	if( prsDeletePatient->eof && prsKeepPatient->eof ){
		return;
	}

	//The Merged From Patient has Insurance Information; the Merged Into Patient *doesn't*
	//Scenario A2 and B2
	if( !prsDeletePatient->eof && prsKeepPatient->eof ){
		sqlExecute += CSqlFragment("UPDATE InsuredPartyT SET PatientID = {INT} WHERE PatientID = {INT}\r\n", nKeepPatient, nDeletePatient);
		//Scenario B2 ONLY
		if( keepWho == mpiiMergedIntoPatient ){
			sqlExecute += CSqlFragment("UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PatientID = {INT}\r\n", nKeepPatient);
		}
		return;
	}

	//The Merged From Patient has no Insurance Information; the Merged Into Patient *does*
	//Scenario A3 and B3
	if( prsDeletePatient->eof && !prsKeepPatient->eof ){
		//Secnario A3
		if( keepWho == mpiiMergedFromPatient ){
			sqlExecute += CSqlFragment("UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PatientID = {INT}\r\n", nKeepPatient);
		}
		return;
	}

	//If we made it this far; then both pts have insured party information
	//Go through the keeper and store the info
	MergeInsuredPartyInformation keepInsuredPartyInformation;
	keepInsuredPartyInformation.nPatientID = (keepWho == mpiiMergedFromPatient ? nDeletePatient : nKeepPatient);
	_RecordsetPtr prsKeepInsurance = (keepWho == mpiiMergedFromPatient ? prsDeletePatient : prsKeepPatient);
	_RecordsetPtr prsPatientInsurance = (keepWho == mpiiMergedFromPatient ? prsKeepPatient : prsDeletePatient);
	while(!prsKeepInsurance->eof){
		//Store the insurance co id
		keepInsuredPartyInformation.mapInsuranceCoID[AdoFldLong(prsKeepInsurance->Fields, "InsuranceCoID")] = true;
		//Store the insured party id
		keepInsuredPartyInformation.mapInsuredPartyID[AdoFldLong(prsKeepInsurance->Fields, "InsuranceCoID")] = AdoFldLong(prsKeepInsurance->Fields, "PersonID");
		prsKeepInsurance->MoveNext();
	}

	//Now, run through the Patient Insurance
	while(!prsPatientInsurance->eof){
		long nInsuranceCoID = AdoFldLong(prsPatientInsurance->Fields, "InsuranceCoID");
		long nInsuredPartyID = AdoFldLong(prsPatientInsurance->Fields, "PersonID");
		bool bInsuranceCoExists;
		//If the insurance company matches...
		if( keepInsuredPartyInformation.mapInsuranceCoID.Lookup(nInsuranceCoID, bInsuranceCoExists) ){
			// Bring over the bills, etc.
			long nKeepInsuredPartyID;
			keepInsuredPartyInformation.mapInsuredPartyID.Lookup(nInsuranceCoID, nKeepInsuredPartyID);
			MoveInsuredPartyTransferInformation(nInsuredPartyID, nKeepInsuredPartyID, sqlExecute);
			// Then, point the new patient to the old pts insured party
			sqlExecute += CSqlFragment(
							"UPDATE InsuredPartyT SET PatientID = {INT} WHERE PatientID = {INT} AND InsuranceCoID = {INT}\r\n",
							nKeepPatient, 
							nDeletePatient,
							nInsuranceCoID
						  );
		}
		//..Inactivate it.
		sqlExecute += CSqlFragment("UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PersonID = {INT}\r\n", nInsuredPartyID);
		prsPatientInsurance->MoveNext();
	}

	//Finally, we've gone through all the Patient Insurance, merging and inactivating; now move over all the From that haven't been satisfied.
	//I began overthinking, creating a map of all the merged, but its much easier than that.  Just set the patient id
	sqlExecute += CSqlFragment(
					"UPDATE InsuredPartyT SET PatientID = {INT} WHERE PatientID = {INT}\r\n", 
					nKeepPatient, 
					nDeletePatient
				  );
}

void CMergePatientsDlg::MoveInsuredPartyTransferInformation(long nFromParty, long nToParty, CSqlFragment& sqlExecute)
{
	//Now update the information that we need to transfer from one to the other
	{
		// (j.jones 2015-11-24 09:31) - PLID 67613 - handle ChargesT.AllowableInsuredPartyID
		sqlExecute += CSqlFragment("UPDATE ChargesT SET AllowableInsuredPartyID = {INT} WHERE AllowableInsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		sqlExecute += CSqlFragment("UPDATE ChargeRespT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		//(r.wilson 2/20/2013) pl 55262 - Make sure we transfer the insured party information over as well						
		sqlExecute += CSqlFragment("Update EMRChargeRespT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		sqlExecute += CSqlFragment("UPDATE PaymentsT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		// (j.jones 2009-03-11 08:46) - PLID 32864 - we intentionally do not audit this
		sqlExecute += CSqlFragment("UPDATE BillsT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		sqlExecute += CSqlFragment("UPDATE InsuranceReferralsT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		sqlExecute += CSqlFragment("UPDATE BillsT SET OthrInsuredPartyID = {INT} WHERE OthrInsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		sqlExecute += CSqlFragment("UPDATE GroupDetailsT SET PersonID = {INT} WHERE PersonID = {INT}\r\n", nToParty, nFromParty);
		sqlExecute += CSqlFragment("UPDATE ClaimHistoryT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		// (z.manning 2010-10-22 10:29) - PLID 36822 - LabsT.InsuredPartyID
		sqlExecute += CSqlFragment("UPDATE LabsT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		// (j.jones 2011-05-03 14:30) - PLID 41594 - handle eligibility requests
		sqlExecute += CSqlFragment("UPDATE EligibilityRequestsT SET InsuredPartyID = {INT} WHERE InsuredPartyID = {INT}\r\n", nToParty, nFromParty);
		// (j.gruber 2010-08-04 16:40) - PLID 39729 - keeping old ins party information, not updating pay groups here

		//We are now done with this insured party.  The patient delete function can handle removing it (since it's still tied to the patient).
	}
}

// (b.savon 2014-12-02 11:54) - PLID 58850 - Connectathon - Support new HL7 message type ADT^A40^ADT_A39 - Merge patient
void CMergePatientsDlg::SendViaHL7()
{
	try{

		CArray<long, long> arDefaultGroupIDs;
		GetAllHL7SettingsGroups(arDefaultGroupIDs);
		if (arDefaultGroupIDs.GetSize()) {
			for (int i = 0; i< arDefaultGroupIDs.GetSize(); i++){
				GetMainFrame()->GetHL7Client()->SendMergePatientHL7Message(m_nCurrentUserDefinedID, m_nNewID, arDefaultGroupIDs[i], false);
			}
		}

	}NxCatchAll(__FUNCTION__);
}