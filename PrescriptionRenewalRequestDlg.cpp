// PrescriptionRenewalRequestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "PrescriptionRenewalRequestDlg.h"
#include "NewCropSoapFunctions.h"
#include "NewCropUtils.h"
#include "NewCropBrowserDlg.h"
#include "Selectdlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "PatientView.h"
#include "ReconcileMedicationsUtils.h"
#include "DecisionRuleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CPrescriptionRenewalRequestDlg dialog
// (j.gruber 2009-06-16 12:42) - PLID 28541 - added functionality for renewal requests

enum RenewalListColumn 
{
	rlcRenewalGUID = 0,
	rlcPatientID = 1,
	rlcUserDefinedID,
	rlcPatientName,
	rlcBirthdate,
	rlcGender,
	rlcTimeReceived,
	rlcPharmacy,
	rlcDrug,
	rlcQuanitity,	
	rlcRefills,
	rlcSig,
	rlcProcess,	
};

IMPLEMENT_DYNAMIC(CPrescriptionRenewalRequestDlg, CNxDialog) // (a.walling 2011-01-14 13:50) - no PLID - fixed bad base class

CPrescriptionRenewalRequestDlg::CPrescriptionRenewalRequestDlg(CWnd* pParent)
	: CNxDialog(IDD, pParent)
{

}

void CPrescriptionRenewalRequestDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RENEWAL_REQUEST_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_PRR_ASSIGN_TO_PATIENT, m_btnAssignToPatient);	
}


BEGIN_MESSAGE_MAP(CPrescriptionRenewalRequestDlg, CNxDialog) // (a.walling 2011-01-14 13:50) - no PLID - fixed bad base class
	ON_BN_CLICKED(IDC_BUTTON2, &CPrescriptionRenewalRequestDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_PRR_ASSIGN_TO_PATIENT, &CPrescriptionRenewalRequestDlg::OnBnClickedPrrAssignToPatient)
	ON_BN_CLICKED(IDC_RENEWAL_REQUEST_CLOSE, &CPrescriptionRenewalRequestDlg::OnBnClickedRenewalRequestClose)
	ON_MESSAGE(NXM_NEWCROP_BROWSER_DLG_CLOSED, OnNewCropBrowserClosed)
END_MESSAGE_MAP()


// CPrescriptionRenewalRequestDlg message handlers
BOOL CPrescriptionRenewalRequestDlg::OnInitDialog() {
	
	CNxDialog::OnInitDialog();

	try {

		CWaitCursor cwait;

		m_pRenewalList = BindNxDataList2Ctrl(IDC_PRESCRIPTION_RENEWAL_REQUEST_LIST, false);

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAssignToPatient.AutoSet(NXB_MODIFY);

		m_btnAssignToPatient.EnableWindow(FALSE);
		
		//load the list
		LoadPatientRenewals();

	//	LoadAccountStatus();

	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::OnInitDialog");
	return TRUE;
}

void CPrescriptionRenewalRequestDlg::LoadAccountStatus() {

	try {
		/*CString strCredentials, strAccountRequest, strLocationID, strUserType, strUserID, strErrorMessage;

		if (GetCredentials(strCredentials)) {

			if (GetAccountRequest(strAccountRequest)) {

				strLocationID = "1";
				strUserType = "1";
				strUserID = "1";

				NewCropAccountStatus pStatus;
				
				if (GetAccountStatus(strCredentials, strAccountRequest, strLocationID, strUserType, strUserID, &pStatus, strErrorMessage)) {

				}
			}
		}*/
	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::LoadAccountStatus");


}

long CPrescriptionRenewalRequestDlg::FindPatientByRenewalInfo(long &nUserDefinedID, CString strPatientFirstName, CString strPatientMiddleName, CString strPatientLastName, CString strPatientGender, CString strPatientDOB) 
{
	try {

		CString strWhere;

		COleDateTime dtBirthdate;
		dtBirthdate.SetDate(atoi(strPatientDOB.Left(4)), 
			atoi(strPatientDOB.Mid(4, 2)),
			atoi(strPatientDOB.Right(2)));

		if (dtBirthdate.GetStatus() != COleDateTime::valid) {

			//fail			
			ASSERT(FALSE);
			return -1;
		}

		//first, last, DOB, and Gender have to match exactly
		//first get the count of how many patients this finds
		ADODB::_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(ID) as CountID "
			" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			" WHERE First = {STRING} AND Last = {STRING} "
			" AND Gender = {INT} AND Birthdate = {STRING} ", strPatientFirstName, strPatientLastName,
			strPatientGender == "M" ? 1 : strPatientGender == "F" ? 2 : -1,
			FormatDateTimeForSql(dtBirthdate));

		
		long nCount = 0;
		if (rsCount->eof) {
			ASSERT(FALSE);
			return -1;
		}
		else {
			nCount = AdoFldLong(rsCount, "CountID", 0);
		}
		rsCount->Close();

		//now check to see if we have 0 or 1
		if (nCount == 0) {

			//we couldn't find the person, return -1;
			return -1;
		}
		else if (nCount == 1) {
			//we have exactly 1 person, we are good to go
			
			ADODB::_RecordsetPtr rsPerson = CreateParamRecordset("SELECT ID, UserDefinedID "
					" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					" WHERE First = {STRING} AND Last = {STRING} "
					" AND Gender = {INT} AND Birthdate = {STRING} ", strPatientFirstName, strPatientLastName,
					strPatientGender == "M" ? 1 : strPatientGender == "F" ? 2 : -1,
					FormatDateTimeForSql(dtBirthdate));

			if (! rsPerson->eof) {

				nUserDefinedID = AdoFldLong(rsPerson, "UserDefinedID", -1);
				return AdoFldLong(rsPerson, "ID", -1);
			}
		}
		else {

			//there must be more then one, add the middle name and see if we get a match
			ADODB::_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(ID) as CountID "
				" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				" WHERE First = {STRING} AND Last = {STRING} AND Middle = {STRING} "
				" AND Gender = {INT} AND Birthdate = {STRING} ", strPatientFirstName, strPatientLastName, strPatientMiddleName,
				strPatientGender == "M" ? 1 : strPatientGender == "F" ? 2 : -1,
				FormatDateTimeForSql(dtBirthdate));


			nCount = 0;
			if (rsCount->eof) {
				ASSERT(FALSE);
				return -1;
			}
			else {
				nCount = AdoFldLong(rsCount, "CountID", 0);
			}
			rsCount->Close();

			if (nCount == 1) {

				//we found one person, woo hoo!!
				ADODB::_RecordsetPtr rsPerson = CreateParamRecordset("SELECT ID, UserDefinedID "
					" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					" WHERE First = {STRING} AND Last = {STRING} AND Middle = {STRING} "
					" AND Gender = {INT} AND Birthdate = {STRING} ", strPatientFirstName, strPatientLastName, strPatientMiddleName,
					strPatientGender == "M" ? 1 : strPatientGender == "F" ? 2 : -1,
					FormatDateTimeForSql(dtBirthdate));

				if (! rsPerson->eof) {

					nUserDefinedID = AdoFldLong(rsPerson, "UserDefinedID", -1);
					return AdoFldLong(rsPerson, "ID", -1);
				}
			}
			else {
				//we can't find just one match, return failure
				return -1;
			}
		}	

	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::FindPatientByRenewalInfo");

	//if we got here, we failed
	return -1;

}

void CPrescriptionRenewalRequestDlg::LoadPatientRenewals() {

	try {
		CWaitCursor cwait;
		CPtrArray pArray;
		CString strCredentials, strAccountRequest, strLocationID, strLicensedPrescriberID, strErrorMessage;

		if (GetCredentials(strCredentials)) {

			if (GetAccountRequest(strAccountRequest)) {

				strLocationID = "";
				strLicensedPrescriberID = "";
				
				if (GetAllRenewalRequests(strCredentials, strAccountRequest, strLocationID, strLicensedPrescriberID, &pArray, strErrorMessage)) {

					//load the list
					for (int i = 0 ; i < pArray.GetCount(); i++) {

						NewCropRenewalSummary *pRenewal = ((NewCropRenewalSummary*)pArray.GetAt(i));

						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRenewalList->GetNewRow();
						pRow->PutValue(rlcRenewalGUID, _variant_t(pRenewal->strRenewalRequestGUID));
						pRow->PutValue(rlcTimeReceived, _variant_t(pRenewal->strReceivedTimeStamp));
						pRow->PutValue(rlcPharmacy, _variant_t(pRenewal->strPharmacyStoreName));
						pRow->PutValue(rlcDrug, _variant_t(pRenewal->strDrugInfo));
						pRow->PutValue(rlcRefills, _variant_t(pRenewal->strNumberofRefills));
						pRow->PutValue(rlcProcess, _variant_t("Process"));					
						pRow->PutValue(rlcQuanitity, _variant_t(pRenewal->strQuantity));
						pRow->PutValue(rlcSig, _variant_t(pRenewal->strSig));


						//now we need to find the patient based on the information provided
						long nUserDefinedID;
						long nPatientID = FindPatientByRenewalInfo(nUserDefinedID, pRenewal->strPatientFirstName, pRenewal->strPatientMiddleName, pRenewal->strPatientLastName, pRenewal->strPatientGender, pRenewal->strPatientDOB);
						if (nPatientID == -1) {
							//we didn't find the patient, they'll have to find it
							pRow->PutValue(rlcPatientID, (long)-1);
							pRow->PutValue(rlcUserDefinedID, _variant_t("<Unknown - Click Assign To Patient to Specify>"));						
							pRow->PutValue(rlcPatientName, _variant_t(pRenewal->strPatientFirstName + ' ' + pRenewal->strPatientMiddleName + ' ' + pRenewal->strPatientLastName));
							pRow->PutForeColor(RGB(255,0,0));
						}
						else {
							pRow->PutValue(rlcPatientID, (long)nPatientID);
							pRow->PutValue(rlcUserDefinedID, nUserDefinedID);
							pRow->PutValue(rlcPatientName, _variant_t(pRenewal->strPatientFirstName + ' ' + pRenewal->strPatientMiddleName + ' ' + pRenewal->strPatientLastName));
						}

						pRow->PutValue(rlcGender, _variant_t(pRenewal->strPatientGender));
												
						COleDateTime dtBirthdate;
						CString strPatientDOB = pRenewal->strPatientDOB;
						dtBirthdate.SetDate(atoi(strPatientDOB.Left(4)), 
							atoi(strPatientDOB.Mid(4, 2)),
							atoi(strPatientDOB.Right(2)));

						pRow->PutValue(rlcBirthdate, _variant_t(FormatDateTimeForInterface(dtBirthdate)));				

						m_pRenewalList->AddRowAtEnd(pRow, NULL);						
					}

					//remove everything from the list
					for(int i=0; i<pArray.GetSize(); i++) {
						NewCropRenewalSummary *pRenewal = ((NewCropRenewalSummary*)pArray.GetAt(i));
						delete pRenewal;
					}
					pArray.RemoveAll();
				
				}			
				
			}
		}
	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::LoadPatientRenewals()");
}

void CPrescriptionRenewalRequestDlg::OnBnClickedButton2()
{

	/*CString strCredentials, strAccountRequest;
	if (GetCredentials(strCredentials)) {

		if (GetAccountRequest(strAccountRequest)) {

			CString strRenewalRequestIdentifier, strErrorMessage;
			strRenewalRequestIdentifier = "05e91df7-3091-4eff-9cc5-0379f0cc9b71";
			
			NewCropRenewalResponseStatus pStatus;

			if (GetRenewalResponseStatus(strCredentials, strAccountRequest, strRenewalRequestIdentifier,
				&pStatus, strErrorMessage)) {


			}
		}
	}*/	
}
BEGIN_EVENTSINK_MAP(CPrescriptionRenewalRequestDlg, CNxDialog)
	ON_EVENT(CPrescriptionRenewalRequestDlg, IDC_PRESCRIPTION_RENEWAL_REQUEST_LIST, 19, CPrescriptionRenewalRequestDlg::LeftClickPrescriptionRenewalRequestList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPrescriptionRenewalRequestDlg, IDC_PRESCRIPTION_RENEWAL_REQUEST_LIST, 18, CPrescriptionRenewalRequestDlg::RequeryFinishedPrescriptionRenewalRequestList, VTS_I2)
	ON_EVENT(CPrescriptionRenewalRequestDlg, IDC_PRESCRIPTION_RENEWAL_REQUEST_LIST, 2, CPrescriptionRenewalRequestDlg::SelChangedPrescriptionRenewalRequestList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CPrescriptionRenewalRequestDlg::LeftClickPrescriptionRenewalRequestList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);		

		if (pRow) {		

			//set the cur sel to this row
			m_pRenewalList->CurSel = pRow;

			switch (nCol) {

				case rlcProcess: {

					//let's confirm they want to do this
					/*if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to approve this Renewal Request?")) {
						return;
					}*/

					//now let's make sure there is a patient
					long nPatientID = VarLong(pRow->GetValue(rlcPatientID), -1);
					if (nPatientID == -1) {
						//make sure the box is enabled
						m_btnAssignToPatient.EnableWindow(TRUE);

						MessageBox("There is no patient assigned to this renewal request.\n"
							"Please highlight the row and click the Assign To Patient button before continuing.");						
					
						return;
					}

					//if we got here, the patient exists, sso make sure the box is disabled
					m_btnAssignToPatient.EnableWindow(FALSE);

					//now we need to popup the browser and send them to newcrop
					CString strWindowDesc;
					strWindowDesc.Format("Processing Renewal Request for %s", VarString(pRow->GetValue(rlcPatientName), ""));

					//generate the response
					CString strXmlDocument;
					GeneratePrescriptionRenewalResponse(strXmlDocument, 
						VarString(pRow->GetValue(rlcRenewalGUID), ""),
						ncrctUndetermined, VarString(pRow->GetValue(rlcRefills), ""));

					// (j.gruber 2009-06-08 10:40) - PLID 34515 - get the user back
					NewCropUserTypes ncuTypeID;
					if (!CheckEPrescribingStatus(ncuTypeID, GetCurrentUserID(), GetCurrentUserName())) {
						//this pops up the message for us
						return;
					}

					// (j.jones 2011-04-11 09:22) - PLID 43215 - we now allow anyone who
					// has a NewCrop user role to renew prescriptions
					/*
					if (ncuTypeID != ncutLicensedPrescriber) {
						//they are a nurse or staff, tell them they can't do it
						MsgBox("Only users marked as Licensed Prescribers in the NewCrop settings can process renewal requests.");
						return;
					}
					*/
		
					// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
					long nUserDefinedID = GetExistingPatientUserDefinedID(nPatientID);
					GetMainFrame()->OpenNewCropBrowser(strWindowDesc, ncatProcessRenewalRequest, -1, nPatientID, nUserDefinedID, -1, this, strXmlDocument);


				}
				break;

				/*case rlcDeny:
					{
						//let's confirm they want to do this
						if (IDNO == MsgBox(MB_YESNO, "Are you sure you want to deny this Renewal Request?")) {
							return;
						}

						//now let's make sure there is a patient
						long nPatientID = VarLong(pRow->GetValue(rlcPatientID), -1);
						if (nPatientID == -1) {
							MessageBox("There is no patient assigned to this renewal request.\n"
								"Please highlight the row and click assign patient before continuing.");
							return;
						}

						//pop up a box that let's them choose a denial reason
						CSelectDlg dlg(this);
						dlg.m_strTitle = "Select a denial reason";
						dlg.m_strCaption = "Select a reason for denying this refill request";
						dlg.m_strFromClause = "(SELECT 1 as ID, 'Patient Unknown To The Prescriber' as Reason "
							" 	UNION  "
							" 	SELECT 2, 'Patient Never Under Prescriber Care' "
							" 	UNION  "
							"	SELECT 3, 'Patient Unknown to the Prescriber' "
							" 	UNION "
							" 	SELECT 4, 'Patient No Longer Under Prescriber Care' "
							" 	UNION "
							" 	SELECT 5, 'Patient Has Requested Refill Too Soon' "
							" 	UNION "
							" 	SELECT 6, 'Medication Never Prescribed For The Patient' "
							" 	UNION "
							" 	SELECT 7, 'Patient Should Contact Prescriber First' "
							" 	UNION "
							" 	SELECT 8, 'Refill Not Appropriate' "
							" 	UNION "
							" 	SELECT 9, 'Patient Has Picked Up Prescription' "
							" 	UNION "
							" 	SELECT 10, 'Patient Has Picked Up Partial Fill Of Prescription'  "
							" 	UNION "
							" 	SELECT 11, 'Patient Has Not Picked Up Prescription Drug Returned To Stock' "
							" 	UNION "
							" 	SELECT 12, 'Change Not Appropriate' "
							" 	UNION "
							" 	SELECT 13, 'Patient Needs Appointment' "
							" 	UNION "
							"	SELECT 14, 'Prescriber Not Associated With This Practice Or Location' "
							"	UNION "
							"	SELECT 15, 'No Attempt Will Be Made To Obtain Prior Authorization' "
							"	UNION "
							"	SELECT 16, 'Denied New Prescription To Follow') Q ";					
						
						// (a.walling 2009-04-28 12:32) - PLID 33951 - The CSelectDlg assigns sort priorities in order of visible columns added,
						// so I rearranged these a bit to sort by last, first, middle, ID, birthdate, gender, etc.
						dlg.AddColumn("ID", "ID", FALSE, FALSE);
						dlg.AddColumn("Reason", "Reason", TRUE, FALSE, TRUE);						

						NewCropResponseDenyCodeType ncrdct = ncrdctInvalid;
						if (IDOK == dlg.DoModal()) {
							ncrdct = (NewCropResponseDenyCodeType) VarLong(dlg.m_arSelectedValues[0], ncrdctInvalid);
						}

						//now we need to popup the browser and send them to newcrop
						CString strWindowDesc;
						strWindowDesc.Format("Denying Renewal Request for %s", VarString(pRow->GetValue(rlcPatientName), ""));

						//generate the response
						CString strXmlDocument;
						GeneratePrescriptionRenewalResponse(strXmlDocument, 
							VarString(pRow->GetValue(rlcRenewalGUID), ""),
							ncrctDeny, VarString(pRow->GetValue(rlcRefills), ""),
							ncrdct);
						
						GetMainFrame()->OpenNewCropBrowser(strWindowDesc, ncatProcessRenewalRequest, -1, nPatientID, -1, this, strXmlDocument);
					}
				break;*/
			}
		}
		


	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::LeftClickPrescriptionRenewalRequestList");
}

void CPrescriptionRenewalRequestDlg::RequeryFinishedPrescriptionRenewalRequestList(short nFlags)
{
	// TODO: Add your message handler code here
}

void CPrescriptionRenewalRequestDlg::OnBnClickedPrrAssignToPatient()
{

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pRenewalList->CurSel;

		if (pRow) {

			//open the list o' patients
			CSelectDlg dlg(this);
			dlg.m_strTitle = "Select a patient";
			dlg.m_strCaption = "Select a patient to associate with this refill";
			dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
			// Exclude the -25 built-in patient, and inquiries
			dlg.m_strWhereClause = " PersonT.ID >= 0 AND CurrentStatus <> 4";
			
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			dlg.AddColumn("Last", "Last", TRUE, FALSE, TRUE);
			dlg.AddColumn("First", "First", TRUE, FALSE, TRUE);
			dlg.AddColumn("Middle", "Middle", TRUE, FALSE, TRUE);
			dlg.AddColumn("UserDefinedID", "PatientID", TRUE, FALSE, TRUE);
			dlg.AddColumn("BirthDate", "Birth Date", TRUE, FALSE, TRUE);
			dlg.AddColumn("(CASE WHEN Gender = 2 THEN 'F' WHEN Gender = 1 THEN 'M' END)", "Gender", TRUE, FALSE, TRUE);
			dlg.AddColumn("Address1", "Address1", TRUE, FALSE, TRUE);
			dlg.AddColumn("Address2", "Address2", TRUE, FALSE, TRUE);
			dlg.AddColumn("City", "City", TRUE, FALSE, TRUE);
			dlg.AddColumn("State", "State", TRUE, FALSE, TRUE);
			dlg.AddColumn("Zip", "Zip", TRUE, FALSE, TRUE);
			
			if(dlg.DoModal() == IDOK) {
				long nPatientID = VarLong(dlg.m_arSelectedValues[0], -1);
				long nUserDefinedID = VarLong(dlg.m_arSelectedValues[4], -1);
				if(nPatientID != -1) {				

					//set the columns
					pRow->PutValue(rlcPatientID, nPatientID);
					pRow->PutValue(rlcUserDefinedID, nUserDefinedID);
					pRow->PutForeColor(RGB(0,0,0));				
				}
		
			}
		}
		else {
			MsgBox("Please choose a renewal request to assign a patient to.");
		}

	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::OnBnClickedPrrAssignToPatient()");
}

void CPrescriptionRenewalRequestDlg::SelChangedPrescriptionRenewalRequestList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {

			long nPatientID = VarLong(pRow->GetValue(rlcPatientID), -1);
			if (nPatientID == -1) {
				m_btnAssignToPatient.EnableWindow(TRUE);
			}
			else {
				m_btnAssignToPatient.EnableWindow(FALSE);
			}
		}
		else {
			m_btnAssignToPatient.EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::SelChangedPrescriptionRenewalRequestList");
}

void CPrescriptionRenewalRequestDlg::OnBnClickedRenewalRequestClose()
{
	try {
		OnOK();

	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::OnBnClickedRenewalRequestClose()");
}


LRESULT CPrescriptionRenewalRequestDlg::OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam) 
{

	try {
		NewCropBrowserResult* pNCBR = (NewCropBrowserResult*)wParam;
		if (pNCBR) {
			long nPatientID = pNCBR->nPatientID;

			// (c.haag 2010-02-18 09:57) - PLID 37424 - Let the user add newly added prescriptions to the current medication list
			CStringArray astrNewCropRxGUIDs;
			for (int i=0; i < pNCBR->aNewlyAddedPatientPrescriptions.GetSize(); i++) {
				astrNewCropRxGUIDs.Add( pNCBR->aNewlyAddedPatientPrescriptions[i].strPrescriptionGUID );
			}

			// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also now takes in an
			// optional prescription ID list, but it is not used here
			//TES 10/31/2013 - PLID 59251 - Track any inteventions that get triggered
			CDWordArray arNewCDSInterventions;
			if(astrNewCropRxGUIDs.GetSize() > 0) {
				CArray<long, long> aryNewPrescriptionIDs;
				if(ReconcileCurrentMedicationsWithMultipleNewPrescriptions(nPatientID, aryNewPrescriptionIDs, astrNewCropRxGUIDs, GetSysColor(COLOR_BTNFACE), this, arNewCDSInterventions)) {
					// Refresh the current view if it's the patients view; the medications tab will need updating
					CNxTabView *pView = GetMainFrame()->GetActiveView();
					if (pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) {
						pView->UpdateView();
					}
				}
			}
			//TES 10/31/2013 - PLID 59251 - If this triggered any interventions, notify the user
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}

		//clear out the list and reload it
		m_pRenewalList->Clear();
		LoadPatientRenewals();

		//need to delete this
		if (pNCBR) {
			delete pNCBR;
		}

/*		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pRenewalList->CurSel;
		
		if (pRow) {

			CString strCredentials, strAccountRequest, strRenewalGUID;

			if (GetCredentials(strCredentials)) {

				if (GetAccountRequest(strAccountRequest)) {

					strRenewalGUID = VarString(pRow->GetValue(rlcRenewalGUID), "");

					ASSERT(!strRenewalGUID.IsEmpty());
					if (!strRenewalGUID.IsEmpty()) {

						//the browser closed, let's see what our status is
						NewCropRenewalResponseStatus *pStatus = new NewCropRenewalResponseStatus;

						CString strErrorMessage;

						if (!GetRenewalResponseStatus(strCredentials, strAccountRequest, strRenewalGUID, pStatus, strErrorMessage)) {
							
							AfxMessageBox(strErrorMessage);
							
						}
						else {

							//see if we can take out this row
							if (pStatus->strResponseCode == "A" || pStatus->strResponseCode == "D") {
								//it's accepted or denied, we can get rid of this row
								m_pRenewalList->RemoveRow(pRow);
							}
							else if (pStatus->strResponseCode.IsEmpty()) {
								//do nothing
							}
							else {
								//TODO: figure out what the other responses are
								ASSERT(FALSE);

								//clear out the list and reload it
								m_pRenewalList->Clear();
								LoadPatientRenewals();
							}
						}

						//delete the status
						delete pStatus;
					}
				}
			}
		}	*/
	}NxCatchAll("Error in CPrescriptionRenewalRequestDlg::OnNewCropBrowserClosed");

	return 0;
}
