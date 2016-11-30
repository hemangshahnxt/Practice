// NexWebPatientInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexWebPatientInfoDlg.h"
#include "NexWebImportDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "NexWebImportDlg.h"
#include "GlobalReportUtils.h"
#include "NexWebUtils.h"
#include "ReferralTreeDlg.h"
#include "WellnessDataUtils.h"
#include "HL7Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_NEXWEB_REF_REMOVE 42897
#define ID_NEXWEB_REF_MAKE_PRIMARY 42898


using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CNexWebPatientInfoDlg dialog


CNexWebPatientInfoDlg::CNexWebPatientInfoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebPatientInfoDlg::IDD, pParent)
{
	
	m_nPersonID = -1;
	//{{AFX_DATA_INIT(CNexWebPatientInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNexWebPatientInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebPatientInfoDlg)
	DDX_Control(pDX, IDC_NEXWEB_NICKNAME, m_nxeditNexwebNickName);
	DDX_Control(pDX, IDC_CHANGE_ADD_REF_SOUR, m_btnAddRefSource);
	DDX_Control(pDX, IDC_NEXWEB_ID, m_nxeditNexwebId);
	DDX_Control(pDX, IDC_NEXWEB_FIRST_NAME, m_nxeditNexwebFirstName);
	DDX_Control(pDX, IDC_NEXWEB_MIDDLE_NAME, m_nxeditNexwebMiddleName);
	DDX_Control(pDX, IDC_NEXWEB_LAST_NAME, m_nxeditNexwebLastName);
	DDX_Control(pDX, IDC_NEXWEB_ADDRESS1, m_nxeditNexwebAddress1);
	DDX_Control(pDX, IDC_NEXWEB_ADDRESS2, m_nxeditNexwebAddress2);
	DDX_Control(pDX, IDC_NEXWEB_CITY, m_nxeditNexwebCity);
	DDX_Control(pDX, IDC_NEXWEB_STATE, m_nxeditNexwebState);
	DDX_Control(pDX, IDC_NEXWEB_ZIP, m_nxeditNexwebZip);
	DDX_Control(pDX, IDC_NEXWEB_SSN, m_nxeditNexwebSsn);
	DDX_Control(pDX, IDC_NEXWEB_SPOUSE_NAME, m_nxeditNexwebSpouseName);
	DDX_Control(pDX, IDC_NEXWEB_HOME_PHONE, m_nxeditNexwebHomePhone);
	DDX_Control(pDX, IDC_NEXWEB_WORK_PHONE, m_nxeditNexwebWorkPhone);
	DDX_Control(pDX, IDC_NEXWEB_EXT_PHONE, m_nxeditNexwebExtPhone);
	DDX_Control(pDX, IDC_NEXWEB_CELL_PHONE, m_nxeditNexwebCellPhone);
	DDX_Control(pDX, IDC_NEXWEB_EMAIL, m_nxeditNexwebEmail);
	DDX_Control(pDX, IDC_NEXWEB_NOTES, m_nxeditNexwebNotes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebPatientInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebPatientInfoDlg)
	ON_WM_KILLFOCUS()
	ON_BN_CLICKED(IDC_CHANGE_ADD_REF_SOUR, OnChangeAddRefSour)
	ON_COMMAND(ID_NEXWEB_REF_REMOVE, OnRemoveReferral)
	ON_COMMAND(ID_NEXWEB_REF_MAKE_PRIMARY, OnMakePrimaryReferral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebPatientInfoDlg message handlers

BOOL CNexWebPatientInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_btnAddRefSource.AutoSet(NXB_NEW);
	
		if (m_nPersonID == -1) {
			MsgBox("Invalid Patient ID");
			return FALSE;
		}

		//BIND AWAY!!!!
		m_pLocationList = BindNxDataListCtrl(this, IDC_NEXWEB_LOCATION_LIST, GetRemoteData(), TRUE);
		m_pPatientTypeList = BindNxDataListCtrl(this, IDC_NEXWEB_PATIENT_TYPE_LIST, GetRemoteData(), TRUE);
		m_pPrefixList = BindNxDataListCtrl(this, IDC_NEXWEB_PT_PREFIX_LIST, GetRemoteData(), TRUE);
		m_pReferralList = BindNxDataListCtrl(this, IDC_NEXWEB_REFERRAL_LIST, GetRemoteData(), FALSE);
		m_pGenderList = BindNxDataListCtrl(this, IDC_NEXWEB_GENDERLIST, GetRemoteData(), FALSE);
		m_pMaritalStatusList = BindNxDataListCtrl(this, IDC_NEXWEB_MARITAL_STATUS, GetRemoteData(), FALSE);
		//(e.lally 2007-05-21) PLID 26017 - handled patient coordinators
		m_pPatCoordList = BindNxDataList2Ctrl(this, IDC_NEXWEB_PAT_COORD, GetRemoteData(), TRUE);
		m_dtBDay = GetDlgItemUnknown(IDC_NEXWEB_DOB);
		


		//set up the gender list
		IRowSettingsPtr pRow;
		pRow = m_pGenderList->GetRow(-1);

		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, _variant_t(""));
		m_pGenderList->AddRow(pRow);

		pRow = m_pGenderList->GetRow(-1);
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, _variant_t("Male"));
		m_pGenderList->AddRow(pRow);

		
		pRow = m_pGenderList->GetRow(-1);
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, _variant_t("Female"));
		m_pGenderList->AddRow(pRow);


		//set up the Marital Status List
		pRow = m_pMaritalStatusList->GetRow(-1);
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, _variant_t("Single"));
		m_pMaritalStatusList->AddRow(pRow);


		pRow = m_pMaritalStatusList->GetRow(-1);
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, _variant_t("Married"));
		m_pMaritalStatusList->AddRow(pRow);

		pRow = m_pMaritalStatusList->GetRow(-1);
		pRow->PutValue(0, (long)3);
		pRow->PutValue(1, _variant_t("Other"));
		m_pMaritalStatusList->AddRow(pRow);


		//Initialize all the the boxes to be disabled and blank
		InitializeControls();

		
		//Now set the windows that we want to set
		Load();
		
	}NxCatchAll("Error in InitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNexWebPatientInfoDlg::SetPersonID(long nPersonID) {

	m_nPersonID = nPersonID;
}


#define SET_CONTROL(strField) SetDlgItemText(pWnd->GetDlgCtrlID(), AdoFldString(rsPerson, strField, ""))

void CNexWebPatientInfoDlg::InitializeControls(BOOL bEnabled /*=FALSE*/) {

	CWnd *pWnd;
	long i;


	//here's the plan, we put the values into the windows disabled that are in the database and then overwrite that data
	//with what comes from Practice

	// (j.gruber 2006-11-08 10:39) - PLID 23380 - added extension to the query
	//(e.lally 2007-05-21) PLID 26017 - added patient coordinators(employeeID)
	// (j.gruber 2008-06-03 10:18) - PLID 30235 - added nickname
	_RecordsetPtr rsPerson = CreateRecordset("SELECT UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, SocialSecurity, Birthdate, MaritalStatus, SpouseName, HomePhone, "
		" WorkPhone, Email, CellPhone, Gender, TypeOfPatient, EmergFirst, EmergLast, EmergRelation, EmergHPhone, EmergWPhone, Location, Note, EmployerFirst, EmployerLast, PrefixId, TypeofPatient, "
		" EmployerMiddle, Occupation, Company, EmployerAddress1, EmployerAddress2, EmployerCity, EmployerState, EmployerZip, DefaultInjuryDate, Extension, EmployeeID, NickName "
		"FROM PersonT INNER JOIN PatientsT ON  PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID = %li", m_nPersonID);

	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT)) {

		if (pWnd && pWnd->m_hWnd) {

			if (pWnd->GetDlgCtrlID() != IDC_STATIC) {

				if (! rsPerson->eof) {

					switch (pWnd->GetDlgCtrlID()) {

						case IDC_NEXWEB_FIRST_NAME:
							SET_CONTROL("First");										
						break;

						case IDC_NEXWEB_MIDDLE_NAME:
							SET_CONTROL("Middle");					
						break;

						case IDC_NEXWEB_LAST_NAME:
							SET_CONTROL("Last");					
						break;

						case IDC_NEXWEB_EMAIL:
							SET_CONTROL("Email");
						break;
						
						case IDC_NEXWEB_ADDRESS1:
							SET_CONTROL("Address1");
						break;
						
						case IDC_NEXWEB_ADDRESS2:
							SET_CONTROL("Address2");					
						break;
						
						case IDC_NEXWEB_CITY:
							SET_CONTROL("City");
						break;

						case IDC_NEXWEB_STATE:
							SET_CONTROL("State");					
						break;
						
						case IDC_NEXWEB_ZIP:
							SET_CONTROL("Zip");					
						break;

						case IDC_NEXWEB_NOTES:
							SET_CONTROL("Note");					
						break;
						
						case IDC_NEXWEB_HOME_PHONE:
							SET_CONTROL("HomePhone");
						break;

						case IDC_NEXWEB_WORK_PHONE:
							SET_CONTROL("WorkPhone");					
						break;
									
						case IDC_NEXWEB_SSN:
							SET_CONTROL("SocialSecurity");					
						break;

						case IDC_NEXWEB_DOB:
						{
							COleDateTime dtBDay;
							COleDateTime dtNULL;
							dtNULL.SetDate(1900, 01, 01);

							dtBDay = AdoFldDateTime(rsPerson, "Birthdate", dtNULL);
							if (dtBDay != dtNULL) {
								m_dtBDay->SetDateTime(dtBDay);						
							}
						
						}
						break;
						case IDC_NEXWEB_GENDERLIST:	
						{
							m_pGenderList->SetSelByColumn(0, AdoFldByte(rsPerson, "Gender", -1));
												
						}
						break;

						case IDC_NEXWEB_CELL_PHONE:
							SET_CONTROL("CellPhone");					
						break;

						case IDC_NEXWEB_LOCATION_LIST:
						{
							m_pLocationList->SetSelByColumn(0, AdoFldLong(rsPerson, "Location", -1));
						}

						break;
							
						case IDC_NEXWEB_ID:
							SetDlgItemText(IDC_NEXWEB_ID, AsString(rsPerson->Fields->Item["UserDefinedID"]->Value));
							// (j.gruber 2006-11-09 13:29) - PLID 23337 - disable this field always
							GetDlgItem(IDC_NEXWEB_ID)->EnableWindow(FALSE);
							bEnabled = FALSE;
						break;
						
						case IDC_NEXWEB_MARITAL_STATUS:
						{
							// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - -1 as a default for the string?
							m_pMaritalStatusList->SetSelByColumn(0, atol(AdoFldString(rsPerson, "MaritalStatus", "-1")));
												
						}
						break;
						case IDC_NEXWEB_SPOUSE_NAME:
							SET_CONTROL("SpouseName");					
						break;
						
						case IDC_NEXWEB_PT_PREFIX_LIST:
						{
							m_pPrefixList->SetSelByColumn(0, AdoFldLong(rsPerson, "PrefixID", -1));
							
						}
						break;

						case IDC_NEXWEB_PATIENT_TYPE_LIST:
						{
							m_pPatientTypeList->SetSelByColumn(0, AdoFldLong(rsPerson, "TypeOfPatient", -1));
							
						}
						break;

						// (j.gruber 2006-11-07 17:32) - PLID 23380 - handle ext field
						case IDC_NEXWEB_EXT_PHONE:
							SET_CONTROL("Extension");
						break;

						//(e.lally 2007-05-21) PLID 26017 - handle patient coordinators
						case IDC_NEXWEB_PAT_COORD:
							// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
							m_pPatCoordList->TrySetSelByColumn_Deprecated(0, AdoFldLong(rsPerson, "EmployeeID", -1));
						break;

						case IDC_NEXWEB_NICKNAME:
							SET_CONTROL("NickName");
						break;

					}
				}

				// (j.gruber 2006-11-09 13:29) - PLID 23337 - disable this field always
				if (pWnd->GetDlgCtrlID() != IDC_NEXWEB_ID) {
					pWnd->EnableWindow(bEnabled);
				}
				else {
					pWnd->EnableWindow(FALSE);
				}
				
			}
			else {
				pWnd->EnableWindow(TRUE);
				
			}

		}
	}


	//now that we are done with that, we need to add the referral sources
	_RecordsetPtr rsRefs = CreateRecordset("SELECT PersonID, Name, Date FROM MultiReferralsT LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID WHERE MultiReferralsT.PatientID = %li", m_nPersonID);
	IRowSettingsPtr pRow;
	GetDlgItem(IDC_NEXWEB_REFERRAL_LIST)->EnableWindow(TRUE);
	while (! rsRefs->eof) {
		pRow = m_pReferralList->GetRow(-1);
		pRow->PutValue(0, rsRefs->Fields->Item["PersonID"]->Value);
		pRow->PutValue(1, rsRefs->Fields->Item["Name"]->Value);
		pRow->PutValue(2, rsRefs->Fields->Item["Date"]->Value);

		//check the primary status
		if (IsRecordsetEmpty("SELECT ReferralID FROM PatientsT WHERE PersonID = %li AND ReferralID = %li", m_nPersonID, AdoFldLong(rsRefs, "PersonID"))) {
			pRow->PutValue(3, (long)0);
		}
		else {
			pRow->PutValue(3, (long)1);
		}

		pRow->PutValue(4, (long) 0);

		m_pReferralList->AddRow(pRow);
		GetDlgItem(IDC_NEXWEB_REFERRAL_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHANGE_ADD_REF_SOUR)->EnableWindow(TRUE);
		rsRefs->MoveNext();
	}
		


}

void CNexWebPatientInfoDlg::Load() {

	try  {
		CString strTransType;
		TransactionType  transField;
		CString strEntry;
		long nObjectID;
		//04-18-05 - JMM - Added union for referral source because they can add more than one from the web
		// also added the order by because we need to add the referral source to list and then the 
		// primary selection, also we want the last primary selection
		_RecordsetPtr rs = CreateRecordset("SELECT Value, Sub.* FROM NexWebTransactionsT INNER JOIN "
			" (SELECT MAX(ID) AS ID, TransType, Field, ObjectID FROM NexWebTransactionsT WHERE PersonID = %li AND FIELD NOT IN (2032, 2036) GROUP BY TransType, Field, ObjectID "
			" UNION SELECT ID, TransType, Field, ObjectID FROM NexWebTransactionsT WHERE PersonID = %li AND FIELD IN (2032,2036)  "
			"  ) Sub ON "
			" NexWEbTransactionsT.ID = Sub.Id "
			" ORDER BY Sub.Field ASC ", m_nPersonID, m_nPersonID);

		while (! rs->eof) {


			strTransType = AdoFldString(rs, "TransType");
			transField = (TransactionType)AdoFldLong(rs, "Field");
			strEntry = AdoFldString(rs, "Value");
			nObjectID = AdoFldLong(rs, "ObjectID");

			ProcessField(transField, strEntry, nObjectID);
		
			rs->MoveNext();

		}
	}NxCatchAll("Error loading information");
}


void CNexWebPatientInfoDlg::ProcessField(TransactionType transField, CString strEntry, long nObjectID) {

	switch (transField) {

	case transTypePatient:

		//they added a new patient
		InitializeControls(TRUE);
		break;

	case transTypePrefix:
		{
		//convert it to an int
		GetDlgItem(IDC_NEXWEB_PT_PREFIX_LIST)->EnableWindow(TRUE);
		long nPrefix = atoi(strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_PT_PREFIX_LIST);
		
		m_pPrefixList->SetSelByColumn(0, nPrefix);
		}
	break;
	break;

	case transTypeFirstName:
		GetDlgItem(IDC_NEXWEB_FIRST_NAME)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_FIRST_NAME, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_FIRST_NAME);
		break;
	case transTypeLastName:
		GetDlgItem(IDC_NEXWEB_LAST_NAME)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_LAST_NAME, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_LAST_NAME);
		break;
	case transTypeEmailAddress:
		GetDlgItem(IDC_NEXWEB_EMAIL)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_EMAIL, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_EMAIL);
		break;
	case transTypePassword:
		break;
	case transTypeAddress1:
		GetDlgItem(IDC_NEXWEB_ADDRESS1)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_ADDRESS1, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_ADDRESS1);
		break;
	case transTypeAddress2:
		GetDlgItem(IDC_NEXWEB_ADDRESS2)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_ADDRESS2, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_ADDRESS2);
		break;
	case transTypeCity:
		GetDlgItem(IDC_NEXWEB_CITY)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_CITY, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_CITY);
		break;
	case transTypeState:
		GetDlgItem(IDC_NEXWEB_STATE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_STATE, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_STATE);
		break;
	case transTypeZipCode:
		GetDlgItem(IDC_NEXWEB_ZIP)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_ZIP, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_ZIP);
		break;
	case transTypeNotes:
		GetDlgItem(IDC_NEXWEB_NOTES)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_NOTES, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_NOTES);
		break;
	case transTypeHomePhone:
		GetDlgItem(IDC_NEXWEB_HOME_PHONE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_HOME_PHONE, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_HOME_PHONE);
		break;
	case transTypeWorkPhone:
		GetDlgItem(IDC_NEXWEB_WORK_PHONE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_WORK_PHONE, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_WORK_PHONE);
		break;
	case transTypeFax:
		//SetDlgItemText(IDC_NEXWEB_FAX, strEntry);
		break;
	case transTypeSocialSecurity:
		GetDlgItem(IDC_NEXWEB_SSN)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_SSN, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_SSN);
		break;
	case transTypeBirthDate:
		{		
		//convert it from a string to a date
		GetDlgItem(IDC_NEXWEB_DOB)->EnableWindow(TRUE);
		COleDateTime dtBDay;		

		// (j.gruber 2009-10-01 15:58) - PLID 35602 - ParseDateTime doesn't like the miliseconds, so parse them off
		CString strDate;
		strDate.Format("%s-%s-%s",strEntry.Left(4), strEntry.Mid(5,2), strEntry.Mid(8,2));
		dtBDay.ParseDateTime(strDate);

		//because of PLID 15963, we have to give them a chance to fix it, so let the bad data
		// because we aren't letting them import it if it is there		
		m_dtBDay->SetDateTime(dtBDay);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_DOB);
		}
		break;
	case transTypeGender: 
		{
		//convert it to an int
		GetDlgItem(IDC_NEXWEB_GENDERLIST)->EnableWindow(TRUE);
		long nGender = atoi(strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_GENDERLIST);
		
		m_pGenderList->SetSelByColumn(0, nGender);
		}
	break;
	case transTypeMiddleName:
		GetDlgItem(IDC_NEXWEB_MIDDLE_NAME)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_MIDDLE_NAME, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_MIDDLE_NAME);
		break;
	case transTypeCellPhone:
		GetDlgItem(IDC_NEXWEB_CELL_PHONE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_CELL_PHONE, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_CELL_PHONE);
		break;
	case transTypeLocation:
		{
		GetDlgItem(IDC_NEXWEB_LOCATION_LIST)->EnableWindow(TRUE);
		long nLocation = atoi(strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_LOCATION_LIST);


		m_pLocationList->SetSelByColumn(0, nLocation);
		}
		break;
	case transTypeInactive:
	break;

	case transTypeID:
		// (j.gruber 2006-11-09 15:55) - PLID 23337 - Disable this field always
		GetDlgItem(IDC_NEXWEB_ID)->EnableWindow(FALSE);
		SetDlgItemText(IDC_NEXWEB_ID, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		// (j.gruber 2006-11-09 15:55) - PLID 23337 - Since its disabled, we aren't going to save it
		//we don't need to save it because if it''s an already existing patient then it won't change and we aren't
		// allowing them to add the ID of a new patient
		//AddToSaveMap(IDC_NEXWEB_ID);
		break;
	case transTypeMaritalStatus: 
		{
		long nMS = atoi(strEntry);
		GetDlgItem(IDC_NEXWEB_MARITAL_STATUS)->EnableWindow(TRUE);
			
		m_pMaritalStatusList->SetSelByColumn(0, nMS);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_MARITAL_STATUS);
		
		}
		break;
	case transTypeSpouseName:
		GetDlgItem(IDC_NEXWEB_SPOUSE_NAME)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_SPOUSE_NAME, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_SPOUSE_NAME);
		break;	

	case transTypePatientType:
		{
		GetDlgItem(IDC_NEXWEB_PATIENT_TYPE_LIST)->EnableWindow(TRUE);
		long nPatTypeID = atoi(strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_PATIENT_TYPE_LIST);

		m_pPatientTypeList->SetSelByColumn(0, nPatTypeID);
		}
		break;	

	case transTypePatientReferralSource:
		{
		//check to see if it already exists in the list
			long nRefID = atoi(strEntry);

			if (m_pReferralList->FindByColumn(0, (long)nRefID, 0, FALSE) == -1) {

				//its not there, we can add it
				IRowSettingsPtr pRow = m_pReferralList->GetRow(-1);

				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				CString strNow;
				strNow = FormatDateTimeForInterface(dtNow, NULL, dtoDate);

				pRow = m_pReferralList->GetRow(-1);
				pRow->PutValue(0, (long)nRefID);
				pRow->PutValue(1, _variant_t(GetReferralSource(strEntry)));
				pRow->PutValue(2, _variant_t(strNow));
				pRow->PutValue(3, (long)0);
				pRow->PutValue(4, (long)1);

				m_pReferralList->AddRow(pRow);
			}
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_REFERRAL_LIST);
		}
	break;

	case transTypePrimaryReferralSourceName:
		{
			long nRow = m_pReferralList->FindByColumn(0, (long)atoi(strEntry), 0, FALSE);

			if (nRow == -1) {
				ASSERT(FALSE);
			}
			else {
				//set all the colors to no color
				for (int i = 0; i < m_pReferralList->GetRowCount(); i++ ) {
					if (VarLong(m_pReferralList->GetValue(i, 3)) != 0) {
						m_pReferralList->PutValue(i, 3, (long)0);
					}
				}					

				//now set our value
				m_pReferralList->PutValue(nRow, 3, (long)1);

				SetRefListColors();
			}
		}
	break;

	case transTypePatientReferralSourceDate:
		{
			long nRow = m_pReferralList->FindByColumn(0, (long)nObjectID, 0, FALSE);

			if (nRow == -1) {
				ASSERT(FALSE);
			}
			else {
				//now set our value
				COleDateTime dtEntry;
				long nYear;
				long nMonth;
				long nDay;

				nYear = atoi(strEntry.Left(4));
				//(e.lally 2009-04-07) PLID 33878 - The month starts on the 5th index.
				nMonth = atoi(strEntry.Mid(5,2));
				//(e.lally 2009-04-07) PLID 33878 - If the time is part of the value string, the day is not at the end.
				//	We can continue to ignore the time.
				nDay = atoi(strEntry.Mid(8,2));
				dtEntry.SetDate(nYear, nMonth, nDay);

				m_pReferralList->PutValue(nRow, 2, _variant_t(FormatDateTimeForInterface(dtEntry)));
			}
		}
	break;

	// (j.gruber 2006-11-07 17:33) -  PLID 23380 - handle extension field
	case transTypeExtension:
		GetDlgItem(IDC_NEXWEB_EXT_PHONE)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_EXT_PHONE, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(IDC_NEXWEB_EXT_PHONE);
	break;

	//(e.lally 2007-05-21) PLID 26017 - handled patient coordinators
	case transTypePatientCoord:
		GetDlgItem(IDC_NEXWEB_PAT_COORD)->EnableWindow(TRUE);
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pPatCoordList->TrySetSelByColumn_Deprecated(0, (long)atoi(strEntry));
		//(we don't have to handle inactive patient coordinators)
		AddToSaveMap(IDC_NEXWEB_PAT_COORD);
	break;

	// (j.gruber 2008-06-03 10:20) - PLID 30235 - added nickname
	case transTypeNickName:
		GetDlgItem(IDC_NEXWEB_NICKNAME)->EnableWindow(TRUE);
		SetDlgItemText(IDC_NEXWEB_NICKNAME, strEntry);
		AddToSaveMap(IDC_NEXWEB_NICKNAME);
	break;

	}

}

//this function needs to check the fields and make sure that it isn't
//letting invalid data into the data.  Currently it only checks for invalid dates
BOOL CNexWebPatientInfoDlg::ValidateData() {

	COleDateTime dtCheck, dtMin;
	dtMin.SetDate(1753,1,1);
	BOOL bValid = TRUE;
	m_strError = "";

	dtCheck = m_dtBDay->GetDateTime();
	if (dtCheck.GetStatus() != 0 || dtCheck < dtMin) {
		m_strError += "Patient BirthDate \n";
		bValid = FALSE;
	}

	return bValid;

}

//(e.lally 2008-02-28) PLID 27379 - Added flag for overwriting existing data vs. updating only if the data is blank
void CNexWebPatientInfoDlg::SaveInfo(long nPersonID /*= -1*/, BOOL bSkipOverwrites /*= FALSE*/) {

		
	if (nPersonID == -1) {
		nPersonID = m_nPersonID;
	}
	
	CString strPersonUpdate;
	CString strPatientUpdate = "UPDATE PatientsT SET ";
	
	// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
	POSITION pos = m_mapIDsToSave.GetStartPosition();
	while(pos){

		long nID;
		m_mapIDsToSave.GetNextAssoc(pos, nID, nID);

		//check to see if the window is enabled, if it is, we need to save it
		SaveWindowData(nID, strPatientUpdate, strPersonUpdate, nPersonID, bSkipOverwrites);


		
	}

	//time to save!
	
	//first remove the ending commas
	//and add the ending part of the string

	// (b.cardillo 2009-07-09 15:39) - PLIDs 34369 and 34368 - We weren't updating patient wellness qualification records 
	// when a gender or dob changed; now we have our update query tell us if either changed, and if so we update them below.
	BOOL bUpdatedGender = FALSE, bUpdatedBirthdate = FALSE;
	if (!strPersonUpdate.IsEmpty()) {
		strPersonUpdate.Delete(strPersonUpdate.GetLength() - 2, 2);
		_RecordsetPtr prs = CreateRecordset(
			"SET NOCOUNT ON \r\n"
			"DECLARE @nPersonID INT \r\n"
			"SET @nPersonID = %li \r\n"
			"DECLARE @nOldGender TINYINT, @dtOldBirthdate DATETIME \r\n"
			"SELECT @nOldGender = Gender, @dtOldBirthdate = Birthdate FROM PersonT WHERE ID = @nPersonID \r\n"
			"UPDATE PersonT SET %s WHERE ID = @nPersonID \r\n"
			"SET NOCOUNT OFF \r\n"
			"SELECT \r\n"
			" CONVERT(BIT, (CASE WHEN @nOldGender = Gender THEN 0 ELSE 1 END)) AS UpdatedGender, \r\n"
			" CONVERT(BIT, (CASE WHEN @dtOldBirthdate IS NULL THEN (CASE WHEN Birthdate IS NOT NULL THEN 1 ELSE 0 END) ELSE (CASE WHEN @dtOldBirthdate = Birthdate THEN 0 ELSE 1 END) END)) AS UpdatedBirthdate \r\n"
			"FROM PersonT WHERE ID = @nPersonID \r\n"
			, nPersonID, strPersonUpdate);
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
	
	
	
	if (strPatientUpdate != "UPDATE PatientsT SET ") {
		CString strTemp;
		strPatientUpdate = strPatientUpdate.Left(strPatientUpdate.GetLength() - 2);	
		strTemp.Format(" WHERE PersonID = %li", nPersonID);
	
		strPatientUpdate += strTemp;
		ExecuteSqlStd(strPatientUpdate);
	}

	// (b.cardillo 2009-07-09 15:39) - PLIDs 34369 and 34368 - Now we know which field(s), if any, were updated, 
	// so update the corresponding patient wellness qualification records.
	if (bUpdatedGender) {
		UpdatePatientWellnessQualification_Gender(GetRemoteData(), nPersonID);
	}
	if (bUpdatedBirthdate) {
		UpdatePatientWellnessQualification_Age(GetRemoteData(), nPersonID);
	}
	

	//audit stuff
	for (int k=0; k < m_paryAuditEvents.GetSize(); k++) {

		AuditEventStruct *pAudit = ((AuditEventStruct*)m_paryAuditEvents.GetAt(k));
		CString strPersonName = GetExistingPatientName(nPersonID);
		long nAuditID = BeginNewAuditEvent(pAudit->strUserName);
		AuditEvent(nPersonID, strPersonName, nAuditID, pAudit->aeiItem, nPersonID, pAudit->strOldValue, pAudit->strNewValue, pAudit->nPriority, pAudit->nType);
	}

	// (j.gruber 2006-11-03 10:56) - PLID 23341 - Clean up the memory leaks!
	//now that we are done we can get rid of the array
	long nCount = m_paryAuditEvents.GetSize();
	for (int i = nCount - 1; i >= 0; i--) {
		AuditEventStruct *pAudit = ((AuditEventStruct*)m_paryAuditEvents.GetAt(i));
		delete pAudit;
		m_paryAuditEvents.RemoveAt(i);
	}
}


//(e.lally 2008-02-28) PLID 27379 - Added flag for overwriting existing data vs. updating only if the data is blank
void CNexWebPatientInfoDlg::SaveWindowData(long nWindowID, CString &strPatientUpdate, CString &strPersonUpdate, long nPersonID, BOOL bSkipOverwrites) {

	CString strEntry, strTemp;

	GetDlgItemText(nWindowID, strEntry);

	switch(nWindowID) {

		case IDC_NEXWEB_FIRST_NAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" First = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" First = CASE WHEN First = '' THEN '%s' ELSE First END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;

		case IDC_NEXWEB_MIDDLE_NAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Middle = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Middle = CASE WHEN Middle = '' THEN '%s' ELSE Middle END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;

		case IDC_NEXWEB_LAST_NAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Last = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Last = CASE WHEN Last = '' THEN '%s' ELSE Last END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;

		case IDC_NEXWEB_EMAIL:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Email = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Email = CASE WHEN Email = '' THEN '%s' ELSE Email END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;
		
		case IDC_NEXWEB_ADDRESS1:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Address1 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Address1 = CASE WHEN Address1 = '' THEN '%s' ELSE Address1 END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;
		
		case IDC_NEXWEB_ADDRESS2:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Address2 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Address2 = CASE WHEN Address2 = '' THEN '%s' ELSE Address2 END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;
		
		case IDC_NEXWEB_CITY:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" City = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" City = CASE WHEN City = '' THEN '%s' ELSE City END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;

		case IDC_NEXWEB_STATE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" State = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" State = CASE WHEN State = '' THEN '%s' ELSE State END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;
		
		case IDC_NEXWEB_ZIP:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Zip = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Zip = CASE WHEN Zip = '' THEN '%s' ELSE Zip END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;

		case IDC_NEXWEB_NOTES:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Note = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Note = CASE WHEN CONVERT(VARCHAR(4000),Note) = '' THEN '%s' ELSE Note END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;
		
		case IDC_NEXWEB_HOME_PHONE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" HomePhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" HomePhone = CASE WHEN HomePhone = '' THEN '%s' ELSE HomePhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;

		case IDC_NEXWEB_WORK_PHONE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" WorkPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" WorkPhone = CASE WHEN WorkPhone = '' THEN '%s' ELSE WorkPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;
	//	case IDC_NEXWEB_FAX:
	//		break;
		
		case IDC_NEXWEB_SSN:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" SocialSecurity = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" SocialSecurity = CASE WHEN SocialSecurity = '' THEN '%s' ELSE SocialSecurity END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;

		break;

		case IDC_NEXWEB_DOB:
		{
			COleDateTime dtMin, dtBDay = m_dtBDay->GetDateTime();
			dtMin.SetDate(1753,1,1);
			
			if (dtBDay.GetStatus() == 0 && dtBDay > dtMin) {
				if(bSkipOverwrites == FALSE){
					strTemp.Format(" BirthDate = '%s', ", FormatDateTimeForSql(dtBDay, dtoDate));
				}
				else{
					strTemp.Format(" BirthDate = CASE WHEN BirthDate IS NULL THEN '%s' ELSE BirthDate END, ", FormatDateTimeForSql(dtBDay, dtoDate));
				}
				strPersonUpdate += strTemp;
			}
			strEntry.Format("%s", FormatDateTimeForInterface(dtBDay, dtoDate));

		}
			break;
		case IDC_NEXWEB_GENDERLIST:	
		{
			long nGender = VarLong(m_pGenderList->GetValue(m_pGenderList->CurSel, 0));

			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Gender = %li, ", nGender);
			}
			else{
				strTemp.Format(" Gender = CASE WHEN Gender = 0 THEN %li ELSE Gender END, ", nGender);
			}
			strPersonUpdate += strTemp;

			//(e.lally 2010-10-18) PLID 35603 - Use the GetGender and the integer value instead of from the interface
			strEntry = GetGender(AsString(nGender));

		}
		break;

		case IDC_NEXWEB_CELL_PHONE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" CellPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" CellPhone = CASE WHEN CellPhone = '' THEN '%s' ELSE CellPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;
		// (s.tullis 2016-05-25 15:41) - NX-100760 -  Location Can be null now
		case IDC_NEXWEB_LOCATION_LIST:
		{
			long nLocation = VarLong(m_pLocationList->GetValue(m_pLocationList->CurSel, 0),-1);

			if (nLocation == -1)
			{
				return;
			}

			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Location = %li, ", nLocation);
			}
			else{
				strTemp.Format(" Location = CASE WHEN Location IS NULL THEN %li ELSE Location END, ", nLocation );
			}
			strPersonUpdate += strTemp;

			strEntry = VarString(m_pLocationList->GetValue(m_pLocationList->CurSel, 1));

		}

		break;
			
	//	case IDC_NEXWEB_INACTIVE:
	//	break;

		case IDC_NEXWEB_ID:
			// (j.gruber 2006-11-09 15:55) - PLID 23337 - Don't save this field
			//strTemp.Format(" UserDefinedID  = '%s', ", _Q(strEntry));
			//strPatientUpdate += strTemp;

			break;
		case IDC_NEXWEB_MARITAL_STATUS:
		{
			long nMS = VarLong(m_pMaritalStatusList->GetValue(m_pMaritalStatusList->CurSel, 0));
			
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" MaritalStatus = %li, ", nMS);
			}
			else{
				//Oy, this is actually an nvarchar field, but we treat it like an INT most all other places.
				strTemp.Format(" MaritalStatus = CASE WHEN MaritalStatus = '' THEN %li ELSE MaritalStatus END, ", nMS);
			}
			strPatientUpdate += strTemp;

			strEntry = VarString(m_pMaritalStatusList->GetValue(m_pMaritalStatusList->CurSel, 1));
			break;
		}
		case IDC_NEXWEB_SPOUSE_NAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" SpouseName = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" SpouseName = CASE WHEN SpouseName = '' THEN '%s' ELSE SpouseName END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
			
		break;	

		case IDC_NEXWEB_PT_PREFIX_LIST:
		{
			long nPrefix = VarLong(m_pPrefixList->GetValue(m_pPrefixList->CurSel, 0));

			if(bSkipOverwrites == FALSE){
				strTemp.Format(" PrefixID = %li, ", nPrefix);
			}
			else{
				strTemp.Format(" PrefixID = CASE WHEN PrefixID IS NULL THEN %li ELSE PrefixID END, ", nPrefix);
			}
			strPersonUpdate += strTemp;

			strEntry = VarString(m_pPrefixList->GetValue(m_pPrefixList->CurSel, 1));

		}

		break;

		case IDC_NEXWEB_PATIENT_TYPE_LIST:
		{
			long nPatType = VarLong(m_pPatientTypeList->GetValue(m_pPatientTypeList->CurSel, 0));

			if(bSkipOverwrites == FALSE){
				strTemp.Format(" TypeOfPatient = %li, ", nPatType);
			}
			else{
				strTemp.Format(" TypeOfPatient = CASE WHEN TypeOfPatient IS NULL THEN %li ELSE TypeOfPatient END, ", nPatType);
			}
			strPatientUpdate += strTemp;

			strEntry = VarString(m_pPatientTypeList->GetValue(m_pPatientTypeList->CurSel, 1));

		}

		break;

		case IDC_NEXWEB_REFERRAL_LIST:
			{

				//loop through the list, checking differences, updating if anything is different and adding if it doesn't exist
				for (int i=0; i < m_pReferralList->GetRowCount(); i++) {
					ProcessReferral(i, nPersonID, bSkipOverwrites);
				}
				return;
			}
		break;

		// (j.gruber 2006-11-08 10:42) - PLID 23380 - handle extension field
		case IDC_NEXWEB_EXT_PHONE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Extension = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Extension = CASE WHEN Extension = '' THEN '%s' ELSE Extension END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;
		break;

		//(e.lally 2007-05-21) PLID 26017 - handle patient coordinators
		case IDC_NEXWEB_PAT_COORD:
			{
			NXDATALIST2Lib::IRowSettingsPtr pRow, pCurSel;
			pRow = m_pPatCoordList->FindByColumn(1, "<No Patient Coordinator>", 0, FALSE);
			pCurSel = m_pPatCoordList->CurSel;

			strEntry="";

			if(pCurSel == NULL) {
				if(bSkipOverwrites == FALSE){
					strPatientUpdate += " EmployeeID = NULL, ";
				}
			}
			else if(pCurSel->IsSameRow(pRow)){
				if(bSkipOverwrites == FALSE){
					strPatientUpdate += " EmployeeID = NULL, ";
				}
			}
			else{
				if(bSkipOverwrites == FALSE){
					strTemp.Format(" EmployeeID = %li, ", VarLong(pCurSel->GetValue(0)));
				}
				else{
					strTemp.Format(" EmployeeID = CASE WHEN EmployeeID IS NULL THEN %li ELSE EmployeeID END, ", VarLong(pCurSel->GetValue(0)));
				}
				strPatientUpdate += strTemp;
				strEntry = VarString(pCurSel->GetValue(1));
			}
			}
		break;

		// (j.gruber 2008-06-03 10:21) - PLID 30235 - added NickName
		case IDC_NEXWEB_NICKNAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" NickName = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" NickName = CASE WHEN NickName = '' THEN '%s' ELSE NickName END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
		break;



	}


	//Audit
	// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
	AddToAuditArray(nWindowID, GetCurrentUserName(), strEntry);


}


//(e.lally 2008-02-28) PLID 27379 - Added flag for overwriting existing data vs. updating if the data is blank
void CNexWebPatientInfoDlg::ProcessReferral(long nRow, long nPersonID, BOOL bSkipOverwrites) {

	try {

		//we aren't changing anything on existing referrals except maybe primary depending on if they set a new one
		long nIsNew = VarLong(m_pReferralList->GetValue(nRow, 4));
		if (nIsNew == 0) {
			return;
		}


		long nRefID, nPrimary;
		COleDateTime dtRefDate;
		CString strRefDate;

		nRefID = VarLong(m_pReferralList->GetValue(nRow, 0));
		nPrimary = VarLong(m_pReferralList->GetValue(nRow, 3));
		strRefDate = AsString(m_pReferralList->GetValue(nRow, 2));
		dtRefDate.ParseDateTime(strRefDate);
		
		
		//first see if this exists in the data
		_RecordsetPtr rsCheck = CreateRecordset("SELECT Date FROM MultiReferralsT WHERE PatientID = %li AND ReferralID = %li", nPersonID, nRefID);
		if (! rsCheck->eof) {

			if(bSkipOverwrites == FALSE){
				//get the date
				COleDateTime dtDate = AdoFldDateTime(rsCheck, "Date");
				if (dtDate != dtRefDate) {
					ExecuteSql("UPDATE MultiReferralsT SET Date = '%s' WHERE PatientID = %li and ReferralID = %li", FormatDateTimeForSql(dtRefDate, dtoDate), nPersonID, nRefID);
				}
			}
		}
		else {

			//it doesn't exist so add it
			ExecuteSql("INSERT INTO MultiReferralsT (PatientID, ReferralID, Date) VALUES (%li, %li, '%s')", nPersonID, nRefID, FormatDateTimeForSql(dtRefDate, dtoDate));
			// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
			AddToAuditArray(IDC_NEXWEB_REFERRAL_LIST, GetCurrentUserName(), AsString((long)nRefID));
		}

		//set the primary status
		if (nPrimary != 0) {
			ExecuteSql("Update PatientsT SET ReferralID = %li WHERE PersonID = %li", nRefID, nPersonID);
			//(j.camacho 2016-02-11) PLID 68228 - no hl7 in this functionality anywhere else, BUT if it does need it here is what needs to happen.
			//UpdateExistingPatientInHL7(nPersonID);
		}

	}NxCatchAll("Error Processing Referral");

}


CString CNexWebPatientInfoDlg::GetFieldString(CString strFieldName, CString strTableName, CString strIDField) {

	_RecordsetPtr rs = CreateRecordset("SELECT %s as Field FROM %s WHERE %s = %li", strFieldName, strTableName, strIDField, m_nPersonID);

	CString strField;

	if (!rs->eof) {

		strField = AdoFldString(rs, "Field");

	}

	return strField;
}


_variant_t CNexWebPatientInfoDlg::GetField(CString strFieldName, CString strTableName, CString strIDField) {

	_RecordsetPtr rs = CreateRecordset("SELECT %s as Field FROM %s WHERE %s = %li", strFieldName, strTableName, strIDField, m_nPersonID);

	_variant_t varField;

	if (!rs->eof) {

		varField = rs->Fields->Item["Field"]->Value;

	}

	return varField;
}


void CNexWebPatientInfoDlg::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);

	UINT nID = pNewWnd->GetDlgCtrlID();

	CString strEntry;
	
	GetDlgItemText(nID, strEntry);
	
	AddToAuditArray(nID, GetCurrentUserName(), strEntry);
	
}


void CNexWebPatientInfoDlg::AddToAuditArray(UINT nWindowID, CString strUserName, CString strEntry) {

	AuditEventStruct *pAudit = new AuditEventStruct;

	switch(nWindowID) {

		case IDC_NEXWEB_FIRST_NAME:
			pAudit->aeiItem = aeiPatientPersonFirst;
			pAudit->strOldValue = GetFieldString("First", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_MIDDLE_NAME:
			pAudit->aeiItem = aeiPatientPersonMiddle;
			pAudit->strOldValue = GetFieldString("Middle", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_LAST_NAME:
			pAudit->aeiItem = aeiPatientPersonLast;
			pAudit->strOldValue = GetFieldString("Last", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_EMAIL:
			pAudit->aeiItem = aeiPatientEmail;
			pAudit->strOldValue = GetFieldString("Email", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		
		case IDC_NEXWEB_ADDRESS1:
			pAudit->aeiItem = aeiPatientAddress;
			pAudit->strOldValue = GetFieldString("Address1", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		
		case IDC_NEXWEB_ADDRESS2:
			pAudit->aeiItem = aeiPatientAddress2;
			pAudit->strOldValue = GetFieldString("Address2", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		
		case IDC_NEXWEB_CITY:
			pAudit->aeiItem = aeiPatientCity;
			pAudit->strOldValue = GetFieldString("City", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_STATE:
			pAudit->aeiItem = aeiPatientState;
			pAudit->strOldValue = GetFieldString("State", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		
		case IDC_NEXWEB_ZIP:
			
			pAudit->aeiItem = aeiPatientZip;
			pAudit->strOldValue = GetFieldString("Zip", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_NOTES:
			
			pAudit->aeiItem = aeiPatientG1Note;
			pAudit->strOldValue = GetFieldString("Note", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		
		case IDC_NEXWEB_HOME_PHONE:
			
			pAudit->aeiItem = aeiPatientHPhone;
			pAudit->strOldValue = GetFieldString("HomePhone", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_WORK_PHONE:
			
			pAudit->aeiItem = aeiPatientWPhone;
			pAudit->strOldValue = GetFieldString("WorkPhone", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
	//	case IDC_NEXWEB_FAX:
	//		break;
		
		case IDC_NEXWEB_SSN:
			
			pAudit->aeiItem = aeiPatientSSN;
			pAudit->strOldValue = GetFieldString("SocialSecurity", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_DOB:
		{
			pAudit->aeiItem = aeiPatientBirthDate;
			pAudit->strOldValue = AsString(GetField("BirthDate", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		}
			break;
		case IDC_NEXWEB_GENDERLIST:	
		{
			pAudit->aeiItem = aeiPatientGender;
			pAudit->strOldValue = GetGender(AsString(GetField("Gender", "PersonT", "ID")));
			//(e.lally 2010-10-18) PLID 35603 - strEntry is already the string value for auditing
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);

		}
		break;

		case IDC_NEXWEB_CELL_PHONE:
			pAudit->aeiItem = aeiPatientMobilePhone;
			pAudit->strOldValue = GetFieldString("CellPhone", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_LOCATION_LIST:
		{
			pAudit->aeiItem = aeiPatientLocation;
			pAudit->strOldValue = GetLocation(AsString(GetField("Location", "PersonT", "ID")));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		}

		break;
			
	//	case IDC_NEXWEB_INACTIVE:
	//	break;

		case IDC_NEXWEB_ID:
			// (j.gruber 2006-11-09 15:57) - We aren't saving it, so is should never audit
			/*pAudit->aeiItem = aeiPatientUserID;
			pAudit->strOldValue = AsString(GetField("UserDefinedID", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);*/
			//just in case we get here somehow, just get rid of the object to not cause a memopry leak
			delete pAudit;
			return;
			break;
		case IDC_NEXWEB_MARITAL_STATUS:
		{
			pAudit->aeiItem = aeiPatientMaritalStatus;
			pAudit->strOldValue = GetMaritalStatus(AsString(GetField("MaritalStatus", "PatientsT", "PersonID")));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);

			break;
		}
		case IDC_NEXWEB_SPOUSE_NAME:
			
			pAudit->aeiItem = aeiPatientSpouseStatus;
			pAudit->strOldValue = AsString(GetField("SpouseName", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;	

		case IDC_NEXWEB_PATIENT_TYPE_LIST:
			pAudit->aeiItem = aeiPatientType;
			pAudit->strOldValue = GetPatientType(AsString(GetField("TypeOfPatient", "PatientsT", "PersonID")));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;	

		case IDC_NEXWEB_REFERRAL_LIST:
			pAudit->aeiItem = aeiPatientReferralAdd; 
			pAudit->strOldValue = ""; 
			pAudit->strNewValue = GetReferralSource(strEntry); 
			pAudit->nPriority = aepMedium; 
			pAudit->nType = aetChanged; 
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		// (j.gruber 2006-11-07 17:25) - PLID 23341  - fixed memory leaks
		case IDC_NEXWEB_PT_PREFIX_LIST:

			//this is not audited, so delete the object
			delete pAudit;
			return;
		break;

		// (j.gruber 2006-11-07 17:25) - PLID 23341  - fixed memory leaks
		// (j.gruber 2006-11-08 10:43) - PLID 23380 - handle extension field
		case IDC_NEXWEB_EXT_PHONE:
			pAudit->aeiItem = aeiPatientExtension;
			pAudit->strOldValue = GetFieldString("Extension", "PersonT", "ID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		// (j.gruber 2008-06-03 10:23) - PLID 30235 - added nickname
		case IDC_NEXWEB_NICKNAME:
			pAudit->aeiItem = aeiPatientNickname;
			pAudit->strOldValue = GetFieldString("NickName", "PatientsT", "PersonID");
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		//(e.lally 2007-05-21) PLID 26017 - added patient coordinators
		case IDC_NEXWEB_PAT_COORD:
		{
			//This is audited after all
			pAudit->aeiItem = aeiPatientCoord;
			//Get the name of the patient coordinator for our current PatientID
			_RecordsetPtr rs = CreateRecordset("SELECT Last + ', ' + First as Field, PatientsT.EmployeeID as PatCoordID "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.EmployeeID = PersonT.ID "
				"WHERE PatientsT.PersonID = %li",m_nPersonID);
			CString strField;
			long nNewPatCoord =0, nOldPatCoord=0;
			if (!rs->eof) {
				strField = AdoFldString(rs, "Field");
				nOldPatCoord = AdoFldLong(rs, "PatCoordID");
			}
			NXDATALIST2Lib::IRowSettingsPtr pRow, pCurSel;
			pRow = m_pPatCoordList->FindByColumn(1, "<No Patient Coordinator>", 0, FALSE);
			pCurSel = m_pPatCoordList->CurSel;


			if(pCurSel == NULL){
				nNewPatCoord = 0;
				strEntry = "<No Patient Coordinator>";
			}
			else if(pCurSel->IsSameRow(pRow)){
				nNewPatCoord = 0;
				strEntry = "<No Patient Coordinator>";
			}
			else{
				nNewPatCoord = VarLong(pCurSel->GetValue(0), 0);
			}



			if(nNewPatCoord != nOldPatCoord){
				pAudit->strOldValue = strField;
				pAudit->strNewValue = strEntry;
				pAudit->nPriority = aepMedium;
				pAudit->nType = aetChanged;
				m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			}
			else{
				delete pAudit;
				return;
			}
		break;
		}

		default:
			//We sent in an unknown ID
			ASSERT(FALSE);
			delete pAudit; //Avoid a memory leak
			return;
		break;

	}

	pAudit->strUserName = strUserName;



}


void CNexWebPatientInfoDlg::FillArrayWithFields(CPtrArray &paryFields) {
	
	
	CWnd *pWnd;
	long i;

	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT))  {
		
		CRParameterInfo *pField = new CRParameterInfo;
		pField->m_Name = GetParameterName(pWnd->GetDlgCtrlID());
		CString strValue = GetParameterFieldValue(pWnd->GetDlgCtrlID());
		pField->m_Data = strValue;

		paryFields.Add(((CRParameterInfo*)pField));
	}
}

CString CNexWebPatientInfoDlg::GetParameterFieldValue(DWORD nWindowID) {

	switch(nWindowID) {
		case IDC_NEXWEB_FIRST_NAME:
		case IDC_NEXWEB_MIDDLE_NAME:
		case IDC_NEXWEB_LAST_NAME:
		case IDC_NEXWEB_EMAIL:
		case IDC_NEXWEB_ADDRESS1:
		case IDC_NEXWEB_ADDRESS2:
		case IDC_NEXWEB_CITY:
		case IDC_NEXWEB_STATE:
		case IDC_NEXWEB_ZIP:
		case IDC_NEXWEB_NOTES:
		case IDC_NEXWEB_HOME_PHONE:
		case IDC_NEXWEB_WORK_PHONE:
		case IDC_NEXWEB_SSN:
		case IDC_NEXWEB_CELL_PHONE:
		case IDC_NEXWEB_ID:
		case IDC_NEXWEB_SPOUSE_NAME:
			// (j.gruber 2006-11-07 17:37) - PLID 23380 - handle extension field
		case IDC_NEXWEB_EXT_PHONE:
		case IDC_NEXWEB_NICKNAME:
			{
				CString strValue;
				GetDlgItemText(nWindowID, strValue);
				return strValue;
			}
		break;

		case IDC_NEXWEB_DOB:
			{
				if(m_dtBDay->GetStatus() == 1) {
					COleDateTime dtDOB;
					dtDOB = m_dtBDay->GetDateTime();
					return FormatDateTimeForInterface(dtDOB, dtoDate);
				}
				else {
					return "";
				}
			}
		break;
		case IDC_NEXWEB_GENDERLIST:	
			if (m_pGenderList->CurSel != -1) {
				return VarString(m_pGenderList->GetValue(m_pGenderList->CurSel, 1));
			}
			else {
				return "";
			}
		break;
		case IDC_NEXWEB_LOCATION_LIST:
			if (m_pLocationList->CurSel != -1) {
				return VarString(m_pLocationList->GetValue(m_pLocationList->CurSel, 1));
			}
			else {
				return "";
			}
		break;
			
		case IDC_NEXWEB_MARITAL_STATUS:
			if (m_pMaritalStatusList->CurSel != -1) {
				return VarString(m_pMaritalStatusList->GetValue(m_pMaritalStatusList->CurSel, 1));
			}
			else {
				return "";
			}
		break;
		
		case IDC_NEXWEB_PATIENT_TYPE_LIST:
			if (m_pPatientTypeList->CurSel != -1) {
				return VarString(m_pPatientTypeList->GetValue(m_pPatientTypeList->CurSel, 1));
			}
			else {
				return "";
			}
		break;
		case IDC_NEXWEB_PT_PREFIX_LIST:
			if (m_pPrefixList->CurSel != -1) {
				return VarString(m_pPrefixList->GetValue(m_pPrefixList->CurSel, 1));
			}
			else {
				return "";
			}
		break;
		//(e.lally 2007-05-21) PLID 26017 - added patient coordinators
		case IDC_NEXWEB_PAT_COORD:{
			NXDATALIST2Lib::IRowSettingsPtr pRow, pCurSel;
			pCurSel = m_pPatCoordList->GetCurSel();
			pRow = m_pPatCoordList->FindByColumn(0, (long)-1, 0, FALSE);
			if (pCurSel != NULL) {
				if(pCurSel->IsSameRow(pRow))
					return "";
				else
					return VarString(m_pPatCoordList->GetCurSel()->GetValue(1));
			}
			else {
				return "";
			}
		break;
		}
		default:
			return "";
		break;
	}

}



CString CNexWebPatientInfoDlg::GetParameterName(DWORD nWindowID) {

	switch(nWindowID) {
		case IDC_NEXWEB_FIRST_NAME:
			return "Person_FirstName";
		break;

		case IDC_NEXWEB_MIDDLE_NAME:
			return "Person_MiddleName";
		break;

		case IDC_NEXWEB_LAST_NAME:
			return "Person_LastName";
		break;

		case IDC_NEXWEB_EMAIL:
			return "Person_Email";
		break;
		
		case IDC_NEXWEB_ADDRESS1:
			return "Person_Address1";
		break;
		
		case IDC_NEXWEB_ADDRESS2:
			return "Person_Address2";
		break;
		
		case IDC_NEXWEB_CITY:
			return "Person_City";
		break;

		case IDC_NEXWEB_STATE:
			return "Person_State";
		break;
		
		case IDC_NEXWEB_ZIP:
			return "Person_Zip";
		break;

		case IDC_NEXWEB_NOTES:
			return "Person_Notes";
		break;
		
		case IDC_NEXWEB_HOME_PHONE:
			return "Person_HomePhone";
		break;

		case IDC_NEXWEB_WORK_PHONE:
			return "Person_WorkPhone";
		break;
		
		case IDC_NEXWEB_SSN:
			return "Person_SSN";
		break;
		
		case IDC_NEXWEB_DOB:
			return "Person_BirthDate";
		break;
		case IDC_NEXWEB_GENDERLIST:	
			return "Person_Gender";
		
		break;

		case IDC_NEXWEB_CELL_PHONE:
			return "Person_CellPhone";
		break;

		case IDC_NEXWEB_LOCATION_LIST:
			return "Person_Location";		
		break;
			
		case IDC_NEXWEB_ID:
			return "Person_UserDefinedID";
		break;
		case IDC_NEXWEB_MARITAL_STATUS:
			return "Person_MaritalStatus";
		break;
		
		case IDC_NEXWEB_SPOUSE_NAME:
			return "Person_SpouseName";			
		break;	

		case IDC_NEXWEB_PATIENT_TYPE_LIST:
			return "Person_PatientType";
		break;

		case IDC_NEXWEB_PT_PREFIX_LIST:
			return "Person_Prefix";
		break;

		// (j.gruber 2006-11-07 17:37) - PLID 23380 - handle extension field
		case IDC_NEXWEB_EXT_PHONE:
			return "Person_Extension";
		break;
		//(e.lally 2007-05-21) PLID 26017 - added patient coordinators
		case IDC_NEXWEB_PAT_COORD:
			return "Person_PatientCoordinator";
		break;
		// (j.gruber 2008-06-03 10:25) - PLID 30235 - added NickName
		case IDC_NEXWEB_NICKNAME:
			return "Person_NickName";
		break;

		default:
			return "";
		break;
	}
			
}

void CNexWebPatientInfoDlg::OnOK()
{
	//Do nothing, we're just a tab in a larger dialog.
}

void CNexWebPatientInfoDlg::OnCancel()
{
	//Do nothing, we're just a tab in a larger dialog.
}
void CNexWebPatientInfoDlg::OnChangeAddRefSour() 
{
	try {
	
		//open up the dialog 
		CReferralTreeDlg dlg(this);
		long result = dlg.DoModal();	//returns the id selected
		
		if (result > 0) {
				// (j.gruber 2006-11-02 16:39) - PLID 22978 - made this save if the referrals changed at all
				AddToSaveMap(IDC_NEXWEB_REFERRAL_LIST);
				//loop through the datalist and make sure that this referral isn't already in there
				if (m_pReferralList->FindByColumn(0, result, 0, FALSE) == -1) {

					COleDateTime dt = COleDateTime::GetCurrentTime();
					CString str;
					str = FormatDateTimeForInterface(dt, NULL, dtoDate);

					BOOL bNewReferral;
					if (m_pReferralList->GetRowCount() > 0) {
						bNewReferral = FALSE;
					}
					else {
						bNewReferral = TRUE;
					}

					//get the name
					CString strRefName;
					_RecordsetPtr rsName = CreateRecordset("SELECT Name FROM ReferralSourceT WHERE PersonID = %li", result);
					if (!rsName->eof) {
						strRefName = AdoFldString(rsName, "Name");
					}
					else {
						MsgBox("The Referral Name could not be found, it may have been deleted by another user\n The selection will not be added.");
						return;
					}
					rsName->Close();

					//put the entry in the list
					IRowSettingsPtr pRow = m_pReferralList->GetRow(-1);

					pRow->PutValue(0, (long)result);
					pRow->PutValue(1, _variant_t(strRefName));
					pRow->PutValue(2, _variant_t(str));
					

					//this preference lets them leave existing referrals as primary
					//1 - make new referral primary (default), 2 - leave existing as primary
					long nPrimaryReferralPref = GetRemotePropertyInt("PrimaryReferralPref",1,0,"<None>",TRUE);

					//now "select" it as the primary referral source
					if(bNewReferral || nPrimaryReferralPref == 1) {
						//set the row to be primary
						pRow->PutValue(3, (long)1);

						//set all existing rows to not be primary
						for (int i = 0; i < m_pReferralList->GetRowCount(); i++ ) {
							
							if (VarLong(m_pReferralList->GetValue(i, 3)) != 0) {
								m_pReferralList->PutValue(i, 3, (long)0);
							}
						}					
					}
					else {
						//no primary
						pRow->PutValue(3, (long)0);
					}

					pRow->PutValue(4, (long) 1);

					//add the row and set the row colors
					m_pReferralList->AddRow(pRow);

					SetRefListColors();				
					
				}
				else {
					MsgBox("You cannot add a referral source more than once per patient.");
				}
		}
	}NxCatchAll("Error adding referral source");
	
	
}

void CNexWebPatientInfoDlg::SetRefListColors() {

	//loop through the list and get the 
	IRowSettingsPtr pRow;
	for (int i = 0; i < m_pReferralList->GetRowCount(); i++ ) {

		if (VarLong(m_pReferralList->GetValue(i, 3)) == 0) {

			//set the color to be black
			pRow = m_pReferralList->GetRow(i);
			pRow->PutForeColor(dlColorNotSet);
		}
		else {
			pRow = m_pReferralList->GetRow(i);
			pRow->PutForeColor(RGB(255,0,0));
		}
	}
}




BEGIN_EVENTSINK_MAP(CNexWebPatientInfoDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebPatientInfoDlg)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_REFERRAL_LIST, 7 /* RButtonUp */, OnRButtonUpNexwebReferralList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_REFERRAL_LIST, 9 /* EditingFinishing */, OnEditingFinishingNexwebReferralList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_REFERRAL_LIST, 10 /* EditingFinished */, OnEditingFinishedNexwebReferralList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_REFERRAL_LIST, 8 /* EditingStarting */, OnEditingStartingNexwebReferralList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_PT_PREFIX_LIST, 16 /* SelChosen */, OnSelChosenNexwebPtPrefixList, VTS_I4)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_PATIENT_TYPE_LIST, 16 /* SelChosen */, OnSelChosenNexwebPatientTypeList, VTS_I4)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_LOCATION_LIST, 16 /* SelChosen */, OnSelChosenNexwebLocationList, VTS_I4)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_MARITAL_STATUS, 16 /* SelChosen */, OnSelChosenNexwebMaritalStatus, VTS_I4)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_GENDERLIST, 16 /* SelChosen */, OnSelChosenNexwebGenderlist, VTS_I4)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_DOB, 1 /* KillFocus */, OnKillFocusNexwebDob, VTS_NONE)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_PAT_COORD, 18 /* RequeryFinished */, OnRequeryFinishedNexwebPatCoord, VTS_I2)
	ON_EVENT(CNexWebPatientInfoDlg, IDC_NEXWEB_PAT_COORD, 16 /* SelChosen */, OnSelChosenNexwebPatCoord, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexWebPatientInfoDlg::OnRButtonUpNexwebReferralList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2007-09-24 10:42) - PLID 27494 - Use exception handling
	try {
		m_pReferralList->CurSel = nRow;

		// (c.haag 2007-09-24 10:44) - PLID 27494 - Quit if there is no valid row
		if (-1 == nRow) {
			return;
		}

		long nIsNew = m_pReferralList->GetValue(nRow, 4);
		if (nIsNew == 0) {
			return;
		}
		
		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		menPopup.InsertMenu(2, MF_BYPOSITION, ID_NEXWEB_REF_REMOVE, "Remove");

		BOOL bEnabled;
		if (VarLong(m_pReferralList->GetValue(nRow, 3)) == 0) {
			bEnabled = TRUE;
		}
		else {
			bEnabled = FALSE;
		}

		menPopup.InsertMenu(3, MF_BYPOSITION|(bEnabled?MF_ENABLED:MF_DISABLED|MF_GRAYED), ID_NEXWEB_REF_MAKE_PRIMARY, "Make Primary");


		CPoint pt(x,y);
		CWnd* pWnd = GetDlgItem(IDC_NEXWEB_REFERRAL_LIST);
		if (pWnd != NULL)
		{	pWnd->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else {
			ThrowNxException("Could not get a handle to the referral list window");
		}
	}
	NxCatchAll("Error in CNexWebPatientInfoDlg::OnRButtonUpNexwebReferralList");
}

void CNexWebPatientInfoDlg::OnMakePrimaryReferral() {
	
	//remove any other primary selection
	for (int i = 0; i < m_pReferralList->GetRowCount(); i++ ) {
		if (VarLong(m_pReferralList->GetValue(i, 3)) != 0) {
			m_pReferralList->PutValue(i, 3, (long)0);
		}
	}	

	//set this one to have primary
	m_pReferralList->PutValue(m_pReferralList->CurSel, 3, (long)1);

	//set the colors
	SetRefListColors();

	// (j.gruber 2006-11-02 16:39) - PLID 22978 - made this save if the referrals changed at all
	AddToSaveMap(IDC_NEXWEB_REFERRAL_LIST);

}

void CNexWebPatientInfoDlg::OnRemoveReferral() {

	m_pReferralList->RemoveRow(m_pReferralList->CurSel);
	// (j.gruber 2006-11-02 16:39) - PLID 22978 - made this save if the referrals changed at all
	AddToSaveMap(IDC_NEXWEB_REFERRAL_LIST);
}

void CNexWebPatientInfoDlg::OnEditingFinishingNexwebReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(*pbCommit == FALSE)
		return;

	switch (nCol) {
		case 2:	
			if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() < 1900) {
				MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved");
				*pbCommit = FALSE;
			}
		break;
	}		
	
	
	
}

void CNexWebPatientInfoDlg::OnEditingFinishedNexwebReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if (bCommit) {
		// (j.gruber 2006-11-02 16:39) - PLID 22978 - made this save if the referrals changed at all
		AddToSaveMap(IDC_NEXWEB_REFERRAL_LIST);
	}
	
}

void CNexWebPatientInfoDlg::OnEditingStartingNexwebReferralList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	long nIsNew = m_pReferralList->GetValue(nRow, 4);

	if (nIsNew == 0) {
		*pbContinue = FALSE;
	}

		
	
}

// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
BOOL CNexWebPatientInfoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam)) {
		case EN_KILLFOCUS:
			AddToSaveMap(LOWORD(wParam));
		break;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::OnSelChosenNexwebPtPrefixList(long nRow) 
{
	AddToSaveMap(IDC_NEXWEB_PT_PREFIX_LIST);
	
}

// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::OnSelChosenNexwebPatientTypeList(long nRow) 
{
	AddToSaveMap(IDC_NEXWEB_PATIENT_TYPE_LIST);
	
}

// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::OnSelChosenNexwebLocationList(long nRow) 
{
	AddToSaveMap(IDC_NEXWEB_LOCATION_LIST);
	
}

// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::OnSelChosenNexwebMaritalStatus(long nRow) 
{
	AddToSaveMap(IDC_NEXWEB_MARITAL_STATUS);
	
}
// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::OnSelChosenNexwebGenderlist(long nRow) 
{
	AddToSaveMap(IDC_NEXWEB_GENDERLIST);
	
}
// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::OnKillFocusNexwebDob() 
{
	AddToSaveMap(IDC_NEXWEB_DOB);
	
}
//(e.lally 2007-05-21) - PLID 26017 - added support for Patient Coordinator
void CNexWebPatientInfoDlg::OnSelChosenNexwebPatCoord(LPDISPATCH lpRow) 
{
	AddToSaveMap(IDC_NEXWEB_PAT_COORD);
	
}
// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebPatientInfoDlg::AddToSaveMap(long nID) {

	//first check to see if this ID is already in our map
	if (m_mapIDsToSave.Lookup(nID, nID) == 0) {

		//we can add it
		m_mapIDsToSave.SetAt(nID, nID);
	}
}

//(e.lally 2007-05-21) PLID 26017 - handle patient coordinators list and No Selection entry
void CNexWebPatientInfoDlg::OnRequeryFinishedNexwebPatCoord(short nFlags) 
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatCoordList->GetNewRow();

		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "<No Patient Coordinator>");
		pRow->PutValue(2, "");

		m_pPatCoordList->AddRowBefore(pRow, m_pPatCoordList->GetFirstRow());

		// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - GetCurSel should be GetCurSel()
		if(m_pPatCoordList->GetCurSel() == NULL)
			m_pPatCoordList->CurSel = pRow;


	}NxCatchAll("Error in OnRequeryFinishedNexwebPatCoord");
	
}

