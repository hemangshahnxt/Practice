// NexWebExtraPatientInfoDlg.cpp : implementation file
//
 
#include "stdafx.h"
#include "practice.h"
#include "NexWebExtraPatientInfoDlg.h"
#include "NexWebImportDlg.h"
#include "GlobalReportUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CNexWebExtraPatientInfoDlg dialog


CNexWebExtraPatientInfoDlg::CNexWebExtraPatientInfoDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebExtraPatientInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexWebExtraPatientInfoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNexWebExtraPatientInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebExtraPatientInfoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_NEXWEB_OCCUPATION, m_nxeditNexwebOccupation);
	DDX_Control(pDX, IDC_NEXWEB_COMPANY, m_nxeditNexwebCompany);
	DDX_Control(pDX, IDC_NEXWEB_MANAGER_FNAME, m_nxeditNexwebManagerFname);
	DDX_Control(pDX, IDC_NEXWEB_MANAGER_MNAME, m_nxeditNexwebManagerMname);
	DDX_Control(pDX, IDC_NEXWEB_MANAGER_LNAME, m_nxeditNexwebManagerLname);
	DDX_Control(pDX, IDC_NEXWEB_EMP_ADDRESS1, m_nxeditNexwebEmpAddress1);
	DDX_Control(pDX, IDC_NEXWEB_EMP_ADDRESS2, m_nxeditNexwebEmpAddress2);
	DDX_Control(pDX, IDC_NEXWEB_EMP_CITY, m_nxeditNexwebEmpCity);
	DDX_Control(pDX, IDC_NEXWEB_EMP_STATE, m_nxeditNexwebEmpState);
	DDX_Control(pDX, IDC_NEXWEB_EMP_ZIP, m_nxeditNexwebEmpZip);
	DDX_Control(pDX, IDC_NEXWEB_EMERGENCY_FIRST_NAME, m_nxeditNexwebEmergencyFirstName);
	DDX_Control(pDX, IDC_NEXWEB_EMERGENCY_LAST_NAME, m_nxeditNexwebEmergencyLastName);
	DDX_Control(pDX, IDC_NEXWEB_EMERGENCY_RELATE, m_nxeditNexwebEmergencyRelate);
	DDX_Control(pDX, IDC_NEXWEB_EMERGENCY_HOME, m_nxeditNexwebEmergencyHome);
	DDX_Control(pDX, IDC_NEXWEB_EMERGENCY_WORK, m_nxeditNexwebEmergencyWork);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebExtraPatientInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebExtraPatientInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebExtraPatientInfoDlg message handlers

BOOL CNexWebExtraPatientInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		//bind the date control
		m_dtIllness = GetDlgItemUnknown(IDC_NEXWEB_CURRENT_ILLNESS_DATE);


		InitializeControls(FALSE);

		Load();
	}NxCatchAll("Error Loading");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CNexWebExtraPatientInfoDlg::SetPersonID(long nPersonID) {

	m_nPersonID = nPersonID;

}
 
#define SET_CONTROL(strField) SetDlgItemText(pWnd->GetDlgCtrlID(), AdoFldString(rsFields, strField))
	
	


void CNexWebExtraPatientInfoDlg::InitializeControls(BOOL bEnabled /*=FALSE*/) {

	CWnd *pWnd;
	long i;

	_RecordsetPtr rsFields = CreateRecordset("SELECT EmployerAddress1, EmployerAddress2, EmployerCity, EmployerState, EmployerZip, "
		" EmployerFirst, EmployerMiddle, EmployerLast, EmergFirst, EmergLast, EmergRelation, EmergHPhone, EmergWPhone, DefaultInjuryDate "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE ID = %li", m_nPersonID);
	
	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT)) {

		if (pWnd && pWnd->m_hWnd) {
			if (pWnd->GetDlgCtrlID() != IDC_STATIC) {
				if (! rsFields->eof) {

					if (pWnd->GetDlgCtrlID() != IDC_STATIC) {

						switch (pWnd->GetDlgCtrlID()) {
							case IDC_NEXWEB_EMP_ADDRESS1:
								SET_CONTROL("EmployerAddress1");
							break;
							case IDC_NEXWEB_EMP_ADDRESS2:
								SET_CONTROL("EmployerAddress2");
							break;
							case IDC_NEXWEB_EMP_CITY:
								SET_CONTROL("EmployerCity");
							break;
							case IDC_NEXWEB_EMP_STATE:
								SET_CONTROL("EmployerState");
							break;
							case IDC_NEXWEB_EMP_ZIP:
								SET_CONTROL("EmployerZip");
							break;

							case IDC_NEXWEB_EMERGENCY_FIRST_NAME:
								SET_CONTROL("EmergFirst");
							break;
							case IDC_NEXWEB_EMERGENCY_LAST_NAME:
								SET_CONTROL("EmergLast");						
							break;
							case IDC_NEXWEB_EMERGENCY_RELATE:
								SET_CONTROL("EmergRelation");						
							break;
							case IDC_NEXWEB_EMERGENCY_HOME:
								SET_CONTROL("EmergHPhone");						
							break;
							case IDC_NEXWEB_EMERGENCY_WORK:	
								SET_CONTROL("EmergWPhone");
							break;
							case IDC_NEXWEB_CURRENT_ILLNESS_DATE:
								{
									COleDateTime dt;
									COleDateTime dtNULL;
									dtNULL.SetDate(1900, 01, 01);
									dt = AdoFldDateTime(rsFields, "DefaultInjuryDate", dtNULL);
									
									if (dt != dtNULL) {
										m_dtIllness->SetDateTime(dt);
									}
								}
							break;
						}
					}
					pWnd->EnableWindow(bEnabled);
				}
				else {
					pWnd->EnableWindow(TRUE);
				}
			}
			else {
				pWnd->EnableWindow(TRUE);
			}

		}
	}

}

void CNexWebExtraPatientInfoDlg::Load() {

	try  {
		CString strTransType;
		TransactionType  transField;
		CString strEntry;
		_RecordsetPtr rs = CreateRecordset("SELECT Value, Sub.* FROM NexWebTransactionsT INNER JOIN "
			" (SELECT MAX(ID) AS ID, TransType, Field FROM NexWebTransactionsT WHERE PersonID = %li GROUP BY TransType, Field) Sub ON "
			" NexWEbTransactionsT.ID = Sub.Id ", m_nPersonID);

		while (! rs->eof) {

			strTransType = AdoFldString(rs, "TransType");
			transField = (TransactionType)AdoFldLong(rs, "Field");
			strEntry = AdoFldString(rs, "Value");

			ProcessField(transField, strEntry);
		
			rs->MoveNext();

		}
	}NxCatchAll("Error loading information");
}



void CNexWebExtraPatientInfoDlg::ProcessField(TransactionType transField, CString strEntry) {


	UINT nWindowID;
	switch (transField) {

		case transTypeCompany:
			nWindowID = IDC_NEXWEB_COMPANY; 
		break;

		case transTypeOccupation:
			nWindowID = IDC_NEXWEB_OCCUPATION;
		break;
		case transTypeManagerFirst:
			nWindowID = IDC_NEXWEB_MANAGER_FNAME;
		break;
		case transTypeManagerMiddle:
			nWindowID = IDC_NEXWEB_MANAGER_MNAME;
		break;
		case transTypeManagerLast:
			nWindowID = IDC_NEXWEB_MANAGER_LNAME;
		break;
		case transTypeEmployerAddress1:
			nWindowID = IDC_NEXWEB_EMP_ADDRESS1; 
		break;
		case transTypeEmployerAddress2:
			nWindowID =  IDC_NEXWEB_EMP_ADDRESS2;
		break;
		case transTypeEmployerCity:
			nWindowID = IDC_NEXWEB_EMP_CITY;
		break;
		case transTypeEmployerState:
			nWindowID = IDC_NEXWEB_EMP_STATE; 
		break;
		case transTypeEmployerZipCode:
			nWindowID =IDC_NEXWEB_EMP_ZIP; 
		break;

		case transTypeEmergencyContactFirst:
			nWindowID = IDC_NEXWEB_EMERGENCY_FIRST_NAME;
		break;
		case transTypeEmergencyContactLast:
			nWindowID = IDC_NEXWEB_EMERGENCY_LAST_NAME; 
		break;
		case transTypeEmergencyContactRelation:
			nWindowID = IDC_NEXWEB_EMERGENCY_RELATE;
		break;
		case transTypeEmergencyContactHomePhone:
			nWindowID = IDC_NEXWEB_EMERGENCY_HOME;
		break;
		case transTypeEmergencyContactWorkPhone:	
			nWindowID = IDC_NEXWEB_EMERGENCY_WORK;
		break;
		case transTypeCurrentIllnessDate:	 
		{
			//convert it from a string to a date
			GetDlgItem(IDC_NEXWEB_CURRENT_ILLNESS_DATE)->EnableWindow(TRUE);
			COleDateTime dtDate;
			dtDate.ParseDateTime(strEntry, VAR_DATEVALUEONLY);

			//because of PLID 15963, we have to give them a chance to fix it, so let the bad data
			// because we aren't letting them inport it if it is there
			m_dtIllness->SetDateTime(dtDate);
			// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
			AddToSaveMap(IDC_NEXWEB_CURRENT_ILLNESS_DATE);
			
		}
			nWindowID = -1;
		break;

		default:
			nWindowID = -1;
		break;
	
	}

	if (nWindowID != -1) {
		GetDlgItem(nWindowID)->EnableWindow(TRUE);
		SetDlgItemText(nWindowID, strEntry);
		// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
		AddToSaveMap(nWindowID);
	}
}


//this is where all the fields need to be checked and reported back,
//currectly it only does date fields
BOOL CNexWebExtraPatientInfoDlg::ValidateData() {

	COleDateTime dtCheck, dtMin;
	BOOL bValid = TRUE;
	dtMin.SetDate(1793, 1, 1);
	m_strError = "";

	dtCheck = m_dtIllness->GetDateTime();
	CString strCheck = dtCheck.Format("%Y-%m-%d");
	strCheck = dtMin.Format("%Y-%m-%d");
	if (dtCheck.GetStatus() != 0 || dtCheck < dtMin) {
		m_strError += "Patient Current Illness Date \n";
		bValid = FALSE;
	}

	return bValid;


}

//(e.lally 2008-03-18) PLID 27379 - Added flag for overwriting existing data vs. updating only if the data is blank
void CNexWebExtraPatientInfoDlg::SaveInfo(long nPersonID /*=-1*/, BOOL bSkipOverwrites /*= FALSE*/) {


	if (nPersonID == -1) {
		nPersonID = m_nPersonID;
	}

	CString strPersonUpdate = "UPDATE PersonT SET ";
	CString strPatientUpdate = "UPDATE PatientsT SET ";
	
	// (j.gruber 2006-11-02 16:31) - PLID 22978 - changed to be a map instead of an array and made a function to add
	POSITION pos = m_mapIDsToSave.GetStartPosition();
	while(pos){

		long nID;
		m_mapIDsToSave.GetNextAssoc(pos, nID, nID);

		//check to see if the window is enabled, it if is, we need to save it
		SaveWindowData(nID, strPatientUpdate, strPersonUpdate, bSkipOverwrites);
		
	}

	//time to save!
	
	//first remove the ending commas
	//and add the ending part of the string
	CString strTemp;

	if (strPersonUpdate != "UPDATE PersonT SET ") {
		strPersonUpdate = strPersonUpdate.Left(strPersonUpdate.GetLength() - 2);
		strTemp.Format(" WHERE ID = %li", nPersonID);
		strPersonUpdate += strTemp;
		ExecuteSqlStd(strPersonUpdate);
	}
	
	
	
	if (strPatientUpdate != "UPDATE PatientsT SET ") {
		strPatientUpdate = strPatientUpdate.Left(strPatientUpdate.GetLength() - 2);	
		strTemp.Format(" WHERE PersonID = %li", nPersonID);
		strPatientUpdate += strTemp;
		ExecuteSqlStd(strPatientUpdate);
	}

	
	

	//audit stuff
	for (int k=0; k < m_paryAuditEvents.GetSize(); k++) {

		AuditEventStruct *pAudit = ((AuditEventStruct*)m_paryAuditEvents.GetAt(k));
		CString strPersonName = GetExistingPatientName(nPersonID);
		long nAuditID = BeginNewAuditEvent(pAudit->strUserName);
		AuditEvent(nPersonID, strPersonName, nAuditID, pAudit->aeiItem, m_nPersonID, pAudit->strOldValue, pAudit->strNewValue, pAudit->nPriority, pAudit->nType);
	}

	// (j.gruber 2006-11-03 10:56) - PLID 23341 - Clean up the memory leaks!
	//now that we are done we can get rid of the array
	long nCount = m_paryAuditEvents.GetSize();
	for (int i = nCount-1; i >= 0; i--) {
		AuditEventStruct *pAudit = ((AuditEventStruct*)m_paryAuditEvents.GetAt(i));
		delete pAudit;
		m_paryAuditEvents.RemoveAt(i);
	}

}


//(e.lally 2008-03-18) PLID 27379 - Added flag for overwriting existing data vs. updating only if the data is blank
void CNexWebExtraPatientInfoDlg::SaveWindowData(long nWindowID, CString &strPatientUpdate, CString &strPersonUpdate, BOOL bSkipOverwrites /*= FALSE*/) {

	CString strEntry, strTemp;

	GetDlgItemText(nWindowID, strEntry);

	switch(nWindowID) {

		case IDC_NEXWEB_COMPANY:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Company = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Company = CASE WHEN Company = '' THEN '%s' ELSE Company END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;			
		break;

		case IDC_NEXWEB_OCCUPATION:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" Occupation = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" Occupation = CASE WHEN Occupation = '' THEN '%s' ELSE Occupation END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;			
		break;
		case IDC_NEXWEB_MANAGER_FNAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerFirst = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerFirst = CASE WHEN EmployerFirst = '' THEN '%s' ELSE EmployerFirst END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
		break;
		case IDC_NEXWEB_MANAGER_MNAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerMiddle = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerMiddle = CASE WHEN EmployerMiddle = '' THEN '%s' ELSE EmployerMiddle END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
		break;
		case IDC_NEXWEB_MANAGER_LNAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerLast = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerLast = CASE WHEN EmployerLast = '' THEN '%s' ELSE EmployerLast END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
		break;
		case IDC_NEXWEB_EMP_ADDRESS1:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerAddress1 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerAddress1 = CASE WHEN EmployerAddress1 = '' THEN '%s' ELSE EmployerAddress1 END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
		break;
		case IDC_NEXWEB_EMP_ADDRESS2:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerAddress2 = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerAddress2 = CASE WHEN EmployerAddress2 = '' THEN '%s' ELSE EmployerAddress2 END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;
		break;
		case IDC_NEXWEB_EMP_CITY:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerCity= '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerCity = CASE WHEN EmployerCity = '' THEN '%s' ELSE EmployerCity END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;			
		break;
		case IDC_NEXWEB_EMP_STATE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerState = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerState = CASE WHEN EmployerState = '' THEN '%s' ELSE EmployerState END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;			
		break;
		case IDC_NEXWEB_EMP_ZIP:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmployerZip= '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmployerZip = CASE WHEN EmployerZip = '' THEN '%s' ELSE EmployerZip END, ", _Q(strEntry));
			}
			strPatientUpdate += strTemp;			
		break;

		case IDC_NEXWEB_EMERGENCY_FIRST_NAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergFirst = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergFirst = CASE WHEN EmergFirst = '' THEN '%s' ELSE EmergFirst END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;			
		break;
		case IDC_NEXWEB_EMERGENCY_LAST_NAME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergLast = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergLast = CASE WHEN EmergLast = '' THEN '%s' ELSE EmergLast END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;			
			
		break;
		case IDC_NEXWEB_EMERGENCY_RELATE:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergRelation = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergRelation = CASE WHEN EmergRelation = '' THEN '%s' ELSE EmergRelation END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;			
			
		break;
		case IDC_NEXWEB_EMERGENCY_HOME:
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergHPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergHPhone = CASE WHEN EmergHPhone = '' THEN '%s' ELSE EmergHPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;			
			
		break;
		case IDC_NEXWEB_EMERGENCY_WORK:	
			if(bSkipOverwrites == FALSE){
				strTemp.Format(" EmergWPhone = '%s', ", _Q(strEntry));
			}
			else{
				strTemp.Format(" EmergWPhone = CASE WHEN EmergWPhone = '' THEN '%s' ELSE EmergWPhone END, ", _Q(strEntry));
			}
			strPersonUpdate += strTemp;			
		break;
		case IDC_NEXWEB_CURRENT_ILLNESS_DATE:	
			{
				COleDateTime dtDate;
				dtDate = m_dtIllness->GetDateTime();

				if(bSkipOverwrites == FALSE){
					strTemp.Format(" DefaultInjuryDate = '%s', ", FormatDateTimeForSql(dtDate, dtoDate));
			}
			else{
				strTemp.Format(" DefaultInjuryDate = CASE WHEN DefaultInjuryDate IS NULL THEN '%s' ELSE DefaultInjuryDate END, ", FormatDateTimeForSql(dtDate, dtoDate));
			}
				strPatientUpdate += strTemp;			

				strEntry = FormatDateTimeForSql(dtDate, dtoDate);

			}			
		break;
	
	}


	//Audit
	// (j.gruber 2009-03-03 15:28) - PLID 33248 - change from using nexweb to the current username
	AddToAuditArray(nWindowID, GetCurrentUserName(), strEntry);


}



_variant_t CNexWebExtraPatientInfoDlg::GetField(CString strFieldName, CString strTableName, CString strIDField) {

	_RecordsetPtr rs = CreateRecordset("SELECT %s as Field FROM %s WHERE %s = %li", strFieldName, strTableName, strIDField, m_nPersonID);

	_variant_t varField;

	if (!rs->eof) {

		varField = rs->Fields->Item["Field"]->Value;

	}

	return varField;
}


void CNexWebExtraPatientInfoDlg::AddToAuditArray(UINT nWindowID, CString strUserName, CString strEntry) {

	AuditEventStruct *pAudit = new AuditEventStruct;

	switch(nWindowID) {

		case IDC_NEXWEB_COMPANY:
			pAudit->aeiItem = aeiPatientCompany;
			pAudit->strOldValue = AsString(GetField("Company", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_OCCUPATION:
			pAudit->aeiItem = aeiPatientOccupation;
			pAudit->strOldValue = AsString(GetField("Occupation", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		case IDC_NEXWEB_EMP_ADDRESS1:
			pAudit->aeiItem = aeiPatientEmployerAddress1;
			pAudit->strOldValue = AsString(GetField("EmployerAddress1", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		case IDC_NEXWEB_EMP_ADDRESS2:
			pAudit->aeiItem = aeiPatientEmployerAddress2;
			pAudit->strOldValue = AsString(GetField("EmployerAddress2", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		case IDC_NEXWEB_EMP_CITY:
			pAudit->aeiItem = aeiPatientEmployerCity;
			pAudit->strOldValue = AsString(GetField("EmployerCity", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		case IDC_NEXWEB_EMP_STATE:
			pAudit->aeiItem = aeiPatientEmployerState;
			pAudit->strOldValue = AsString(GetField("EmployerState", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		case IDC_NEXWEB_EMP_ZIP:
			pAudit->aeiItem = aeiPatientEmployerZip;
			pAudit->strOldValue = AsString(GetField("EmployerZip", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;

		case IDC_NEXWEB_EMERGENCY_FIRST_NAME:
			pAudit->aeiItem = aeiPatientEmergFirst;
			pAudit->strOldValue = AsString(GetField("EmergFirst", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
		break;
		case IDC_NEXWEB_EMERGENCY_LAST_NAME:
			pAudit->aeiItem = aeiPatientEmergLast;
			pAudit->strOldValue = AsString(GetField("EmergLast", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
		case IDC_NEXWEB_EMERGENCY_RELATE:
			pAudit->aeiItem = aeiPatientEmergRelation;
			pAudit->strOldValue = AsString(GetField("EmergRelation", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
		case IDC_NEXWEB_EMERGENCY_HOME:
			pAudit->aeiItem = aeiPatientEmergHPhone;
			pAudit->strOldValue = AsString(GetField("EmergHPhone", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
		case IDC_NEXWEB_EMERGENCY_WORK:	
			pAudit->aeiItem = aeiPatientEmergWPhone;
			pAudit->strOldValue = AsString(GetField("EmergWPhone", "PersonT", "ID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
		case IDC_NEXWEB_MANAGER_FNAME:	
			pAudit->aeiItem = aeiPatientEmployerFirst;
			pAudit->strOldValue = AsString(GetField("EmployerFirst", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
		case IDC_NEXWEB_MANAGER_MNAME:	
			pAudit->aeiItem = aeiPatientEmployerMiddle;
			pAudit->strOldValue = AsString(GetField("EmployerMiddle", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
		case IDC_NEXWEB_MANAGER_LNAME:	
			pAudit->aeiItem = aeiPatientEmployerLast;
			pAudit->strOldValue = AsString(GetField("EmployerLast", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;

		case IDC_NEXWEB_CURRENT_ILLNESS_DATE:	
			pAudit->aeiItem = aeiPatientDefaultInjuryDate;
			pAudit->strOldValue = AsString(GetField("DefaultInjuryDate", "PatientsT", "PersonID"));
			pAudit->strNewValue = strEntry;
			pAudit->nPriority = 1;
			pAudit->nType = 1;
			m_paryAuditEvents.Add((AuditEventStruct*) pAudit);
			
		break;
	
	}

	pAudit->strUserName = strUserName;



}

void CNexWebExtraPatientInfoDlg::FillArrayWithFields(CPtrArray &paryFields) {
	
	
	CWnd *pWnd;
	long i;

	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT))  {
		
		CRParameterInfo *pField = new CRParameterInfo;
		pField->m_Name = GetParameterName(pWnd->GetDlgCtrlID());
		CString strValue;
		strValue = GetParameterFieldValue(pWnd->GetDlgCtrlID()); 
		pField->m_Data = strValue;

		paryFields.Add(((CRParameterInfo*)pField));
	}
}

CString CNexWebExtraPatientInfoDlg::GetParameterFieldValue(DWORD nWindowID) {

	switch(nWindowID) {
		case IDC_NEXWEB_COMPANY:
		case IDC_NEXWEB_OCCUPATION:
		case IDC_NEXWEB_MANAGER_FNAME:
		case IDC_NEXWEB_MANAGER_MNAME:
		case IDC_NEXWEB_MANAGER_LNAME:
		case IDC_NEXWEB_EMP_ADDRESS1:
		case IDC_NEXWEB_EMP_ADDRESS2:
		case IDC_NEXWEB_EMP_CITY:
		case IDC_NEXWEB_EMP_STATE:
		case IDC_NEXWEB_EMP_ZIP:
		case IDC_NEXWEB_EMERGENCY_FIRST_NAME:
		case IDC_NEXWEB_EMERGENCY_LAST_NAME:
		case IDC_NEXWEB_EMERGENCY_RELATE:
		case IDC_NEXWEB_EMERGENCY_HOME:
		case IDC_NEXWEB_EMERGENCY_WORK:	
			{
				CString strValue;
				GetDlgItemText(nWindowID, strValue);
				return strValue;
			}
		break;
		case IDC_NEXWEB_CURRENT_ILLNESS_DATE:	
			{
				if(m_dtIllness->GetStatus() == 1) {
					COleDateTime dtDate;
					dtDate = m_dtIllness->GetDateTime();

					CString strValue;
					strValue = FormatDateTimeForInterface(dtDate, dtoDate);

					return strValue;
				}
				else {
					return "";
				}
			}
		break;

		default:
			return "";
		break;
	}




}

CString CNexWebExtraPatientInfoDlg::GetParameterName(DWORD nWindowID) {

	switch(nWindowID) {
		case IDC_NEXWEB_COMPANY:
			return "Person_Company";
		break;
		
		case IDC_NEXWEB_OCCUPATION:
			return "Person_Occupation";
		break;
		
		case IDC_NEXWEB_MANAGER_FNAME:
			return "Person_EmployerFirst";
		break;

		case IDC_NEXWEB_MANAGER_MNAME:
			return "Person_EmployerMiddle";
		break;
		case IDC_NEXWEB_MANAGER_LNAME:
			return "Person_EmployerLast";
		break;
		case IDC_NEXWEB_EMP_ADDRESS1:
			return "Person_EmployerAddress1";
		break;
		case IDC_NEXWEB_EMP_ADDRESS2:
			return "Person_EmployerAddress2";
		break;
		case IDC_NEXWEB_EMP_CITY:
			return "Person_EmployerCity";
		break;
		case IDC_NEXWEB_EMP_STATE:
			return "Person_EmployerState";
		break;
		case IDC_NEXWEB_EMP_ZIP:
			return "Person_EmployerZip";
		break;
		case IDC_NEXWEB_EMERGENCY_FIRST_NAME:
			return "Person_EmergFirst";
		break;
		case IDC_NEXWEB_EMERGENCY_LAST_NAME:
			return "Person_EmergLast";
		break;
		case IDC_NEXWEB_EMERGENCY_RELATE:
			return "Person_EmergRelation";
		break;
		case IDC_NEXWEB_EMERGENCY_HOME:
			return "Person_EmergHomePhone";
		break;
		case IDC_NEXWEB_EMERGENCY_WORK:	
			return "Person_EmergWorkPhone";
		break;
		case IDC_NEXWEB_CURRENT_ILLNESS_DATE:	
			return "Person_CurrentIllnessDate";
		break;

		default:
			return "";
		break;
	}
			
}

void CNexWebExtraPatientInfoDlg::OnOK()
{
	//Do nothing, we're just a tab in a larger dialog.
}

void CNexWebExtraPatientInfoDlg::OnCancel()
{
	//Do nothing, we're just a tab in a larger dialog.
}


// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebExtraPatientInfoDlg::AddToSaveMap(long nID) {

	//first check to see if this ID is already in our map
	if (m_mapIDsToSave.Lookup(nID, nID) == 0) {

		//we can add it
		m_mapIDsToSave.SetAt(nID, nID);
	}
}

BEGIN_EVENTSINK_MAP(CNexWebExtraPatientInfoDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebExtraPatientInfoDlg)
	ON_EVENT(CNexWebExtraPatientInfoDlg, IDC_NEXWEB_CURRENT_ILLNESS_DATE, 1 /* KillFocus */, OnKillFocusNexwebCurrentIllnessDate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
void CNexWebExtraPatientInfoDlg::OnKillFocusNexwebCurrentIllnessDate() 
{
	AddToSaveMap(IDC_NEXWEB_CURRENT_ILLNESS_DATE);
	
}

// (j.gruber 2006-11-02 16:31) - PLID 22978 - made enabled items save if its a new patient
BOOL CNexWebExtraPatientInfoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam)) {
		case EN_KILLFOCUS:
			AddToSaveMap(LOWORD(wParam));
		break;
	}

	
	return CDialog::OnCommand(wParam, lParam);
}
