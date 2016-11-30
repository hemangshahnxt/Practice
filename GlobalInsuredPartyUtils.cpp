//GlobalInsuredPartyUtils.cpp
//

#include "stdafx.h"
#include "GlobalInsuredPartyUtils.h"
#include "NewInsuredPartyDlg.h"
#include "AuditTrail.h"
#include "HL7Utils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


// (r.goldschmidt 2014-07-24 11:24) - PLID 62775 - need to add pay group list
bool InsuredPartyPayGroupValues::isCoInsPercentValid()
{
	return m_nCoInsPercent != -1;
}

bool InsuredPartyPayGroupValues::isCopayMoneyValid()
{
	return m_cyCopayMoney.GetStatus() == COleCurrency::valid;
}

bool InsuredPartyPayGroupValues::isCopayPercentValid()
{
	return m_nCopayPercent != -1;
}

bool InsuredPartyPayGroupValues::isTotalDeductibleValid()
{
	return m_cyTotalDeductible.GetStatus() == COleCurrency::valid;
}

bool InsuredPartyPayGroupValues::isTotalOOPValid()
{
	return m_cyTotalOOP.GetStatus() == COleCurrency::valid;
}

// default constructor, initialize to all invalid values (this is required; invalid values get saved to null in db)
InsuredPartyPayGroupValues::InsuredPartyPayGroupValues()
	: m_nCoInsPercent(-1)
	, m_cyCopayMoney(COleCurrency())
	, m_nCopayPercent(-1)
	, m_cyTotalDeductible(COleCurrency())
	, m_cyTotalOOP(COleCurrency())
{
	m_cyCopayMoney.SetStatus(COleCurrency::invalid);
	m_cyTotalDeductible.SetStatus(COleCurrency::invalid);
	m_cyTotalOOP.SetStatus(COleCurrency::invalid);
}



// (d.thompson 2009-03-19) - PLID 33590 - This function just sets up the prereqs for adding an insured party
//Parameters:
//	nPatientPersonID:  The PatientsT.PersonID value for the patient having an insured party added
//	nOverrideInsCoPersonID:  An override for the InsuranceCoT.PersonID value.  -1 if not overriding.  If this is not
//		set, a dialog box will prompt the user to select an insurance company.
//	nOverrideRespTypeID:  An override for the InsuredPartyT.RespTypeID value.  -2 if not overriding.  If this is not
//		set, the system will detect if there are any available active resp types.  If not, inactive will be assigned.
//		If so, the system will ask the user if they want this insured party placed at the highest level or inactive.
//	nInsCoPersonID:			The InsuranceCoT.PersonID that is selected by the user (or the override).
//	nRespTypeID:			The RespTypeT.ID that is selected by the user (or the override).
//	nInsContactPersonID:	The InsuranceContactsT.PersonID that is chosen by the system (no user input).
//Return value:
//	If the user cancels any prompts or any failures in data are detected, the return value will be false.  An insured
//	party record should not be created.  Otherwise, the return value will be true.
bool DetermineInsuredPartyPreRequisites(IN long nPatientPersonID, IN long nOverrideInsCoPersonID, IN long nOverrideRespTypeID,
										OUT long &nInsCoPersonID, OUT long &nRespTypeID, OUT long &nInsContactPersonID)
{
/*	This code existed in the previous code, but I'm dropping it.  The dialog to prompt handles this case fine 
			(they just can't proceed), and it's not worth the database access for the 1 in 1 trillion chance this
			actually happens.

	if(IsRecordsetEmpty("SELECT PersonID FROM InsuranceCoT")) {
		AfxMessageBox("You have no insurance companies to choose from. Please add an insurance company by clicking 'Edit Insurance List'.");
		return 0;
	}
	*/


	//1)  The first thing we must determine is what InsuranceCoT.PersonID we are referencing.  If there is an override, use it.  Otherwise, 
	//	prompt the user.
	nInsCoPersonID = nOverrideInsCoPersonID;
	nRespTypeID = nOverrideRespTypeID;

	// (j.jones 2010-08-17 14:39) - PLID 40128 - the CNewInsuredPartyDlg now selects a resp type ID,
	// in the rare case (bad design) where we have an InsCoID and no RespTypeID, or vice versa,
	// we should auto-select what we have been given already, and let them finish the selections

	if(nInsCoPersonID == -1 || nRespTypeID == -2) {
		
		CNewInsuredPartyDlg dlg(NULL);
		dlg.m_nPatientID = nPatientPersonID;
		dlg.m_nDefaultInsCoID = nInsCoPersonID;
		dlg.m_nDefaultRespTypeID = nRespTypeID;

		if(dlg.DoModal() == IDCANCEL) {
			//User has cancelled selecting an insurance company, we can abort
			return false;
		}

		//Otherwise, they chose an appropriate insurance company, and we may proceed.
		nInsCoPersonID = dlg.m_nSelectedInsuranceCoID;
		nRespTypeID = dlg.m_nSelectedRespTypeID;
	}

	//We'll generate the rest into a single database access.
	nInsContactPersonID = -1;	

	{
		//2a)  Next, we have a requirement that an insurance contact must exist.  If this insurance co has none, we have no choice 
		//	but to abort the process and make them fix the data.

		_RecordsetPtr prsCheck = CreateParamRecordset("SELECT TOP 1 PersonID "
			"FROM InsuranceContactsT "
			"WHERE InsuranceCoID = {INT} "
			"ORDER BY [Default] DESC", nInsCoPersonID);

		//
		//Now handle the data in each
		//
		//2b)  contact
		if(prsCheck->eof) {
			//There is no insurance contact for this particular insurance company.  This is considered
			//	bad data, and must be fixed.
			AfxMessageBox("The insurance company you selected does not have an insurance contact.\n"
				"Please go to 'Edit Insurance List' to add a contact prior to assigning to a patient.");
			return false;
		}
		else {
			//We found a contact that we'll be happy to use.
			nInsContactPersonID = AdoFldLong(prsCheck, "PersonID");		//Never NULL
		}
		prsCheck->Close();
	}

	//No errors, we succeed
	return true;
}

//(d.thompson 2009-03-19) - PLID 33590 - This used to exist as CInsuranceDlg::AddInsuredPartyRecord().  
//	I made the following changes:
//	- Moved it here so it can be called globally.
//	- Redesigned it so it can operate without the insurance tab interface.
//	- Provided an override for insurance company PersonID and Insurance Plan ID if you do not want to prompt
//		the user.  InsCoPersonID may be used without plan, but plan cannot be used without InsCoPersonID.
//	- Reworked to be much more efficient.  Worst case previously was about 15 queries to create the record!
// (j.jones 2012-10-25 09:35) - PLID 36305 - added Title
// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
// (r.goldschmidt 2014-07-24 16:33) - PLID 63111 - added insurance pay group information
// (r.gonet 2015-11-12 11:14) - PLID 66907 - New optional parameter bUpdatePatientInHL7.
//
//Parameters:
//	nPatientPersonID:		The PatientsT.PersonID value for the insured party being created.  Input only.
//	strPatientName:			The name of the patient for auditing purposes.  Input only.
//	nNewPersonID:			Upon success, this will hold the InsuredPartyT.PersonID value that was newly created.  Output
//							only.
//	varNewPlanID:			Upon success, this will hold a variant of the InsuredPartyT.InsPlan value, which may be NULL or
//							a VT_I4.  Output only.
//	nRespTypeID:			Upon success, this will hold the InsuredPartyT.RespTypeID value for the insured party that 
//							was entered.  Output only.
//	nInsCoPersonID:			Upon success, this will hold the InsuredPartyT.InsCoID value for the insured party that
//							was entered.  Output only.
//	nOverrideInsCoPersonID:	You may use this parameter to optionally override the InsuredPartyT.InsCoID value.  If this is 
//							not overridden, the user will be prompted for an insurance company.  Input only.
//	nOverrideInsPlanID:		You may use this parameter to optionally override the InsuredPartyT.InsPlan value.  This may
//							only be used if you also used nOverrideInsCoPersonID.  If an InsPlanID is given which does not
//							match the nOverrideInsCoPersonID, it will be set to NULL. Input only.
//	nOverrideRespTypeID:	You may use this parameter to optionally override the InsuredPartyT.RespTypeID value.  If you
//							do not override this value, the system will detect available active resp types.  If none, it
//							will automatically set to Inactive, otherwise the user will be prompted to choose between the highest 
//							available resp type, or inactive.  Input only.
//	strInsPartyIDForInsurance:		You may use this parameter to optionally override the InsuredParty's Insurance ID value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyPolicyGroupNum:		You may use this parameter to optionally override the InsuredParty's Policy Number value.  If you
//							do not override this value, blank will be entered.  Input only.
//	bPerPayGroup:
//	cyTotalDeductible:
//	cyTotalOOP:
//	mapInsPartyPayGroupValues:
//	dtInsPartyRelationToPatient:	You may use this parameter to optionally override the InsuredParty's Relation to Patient value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyFirst:		You may use this parameter to optionally override the InsuredParty's First Name value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyMiddle:		You may use this parameter to optionally override the InsuredParty's Middle Name value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyLast:		You may use this parameter to optionally override the InsuredParty's Last Name value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyTitle:		You may use this parameter to optionally override the InsuredParty's Title value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyAddress1:	You may use this parameter to optionally override the InsuredParty's Address1 value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyAddress2:	You may use this parameter to optionally override the InsuredParty's Address2 value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyCity:		You may use this parameter to optionally override the InsuredParty's City value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyState:		You may use this parameter to optionally override the InsuredParty's State Name value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyZip:			You may use this parameter to optionally override the InsuredParty's ZipCode value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyCountry:		You may use this parameter to optionally override the InsuredParty's Country value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyPhone:		You may use this parameter to optionally override the InsuredParty's Phone Number value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyEmployerSchool:		You may use this parameter to optionally override the InsuredParty's Employer/School Name value.  If you
//							do not override this value, blank will be entered.  Input only.
//	strInsPartyGender:		You may use this parameter to optionally override the InsuredParty's Gender value.  If you
//							do not override this value, 0 will be entered.  Input only.
//	dtInsPartyBirthdate:	You may use this parameter to optionally override the InsuredParty's BirthDate value.  If you
//							do not override this value, null will be entered.  
//							The calling function is in charge of checking that any valid states that it wants.
//							This function checks the COleDateTime::valid flag only. Input only.
//	strInsPartySSN:			You may use this parameter to optionally override the InsuredParty's SSN value.  If you
//							do not override this value, blank will be entered.  Input only.
//	bUpdatePatientInHL7:	Flag controlling whether an HL7 message is generated after the insured party isupdated.
//							When true is passed, the function sends an HL7 patient update message after creating the 
//							new insured party. When false is pased, no HL7 patient update message is sent.
//							Optional. Defaults to true because that is legacy behavior and most places calling this function
//							would want an HL7 message sent after the save to data.
//
//Return values:
//		false				This will be returned if there is a failure in determining the proper values for the Insured Party, or
//							if the user cancels an attempt to prompt for one of the values.  Nothing is written to data.
//		true				This will be returned if everything succeeds and the InsuredParty record is fully written to data.
//		<exception>			Not a return value, but this function will throw exceptions.
bool CreateNewInsuredPartyRecord(IN long nPatientPersonID, IN CString strPatientName,
								 OUT long &nNewPersonID, OUT _variant_t &varNewPlanID, OUT long &nRespTypeID, OUT long &nInsCoPersonID,
								 OPTIONAL IN long nOverrideInsCoPersonID /*= -1*/, OPTIONAL IN long nOverrideInsPlanID /*= -1*/,
								 OPTIONAL IN long nOverrideRespTypeID /*= -2*/,
								 OPTIONAL IN CString strInsPartyIDForIns /*= ""*/,
								 OPTIONAL IN CString strInsPartyPolicyGroupNum /*= ""*/,
								 OPTIONAL IN bool bPerPayGroup /*= false*/,
								 OPTIONAL IN COleCurrency cyTotalDeductible /*= InsuredPartyPayGroupValues().m_cyTotalDeductible*/,
								 OPTIONAL IN COleCurrency cyTotalOOP /*= InsuredPartyPayGroupValues().m_cyTotalOOP*/,
								 OPTIONAL IN const CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues> &mapInsPartyPayGroupValues /*= CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues>(10)*/,
								 OPTIONAL IN CString strInsPartyRelationToPatient, /*= ""*/
				 				 OPTIONAL IN CString strInsPartyFirst /*= ""*/,
								 OPTIONAL IN CString strInsPartyMiddle /*= ""*/,
								 OPTIONAL IN CString strInsPartyLast /*= ""*/,
								 OPTIONAL IN CString strInsPartyTitle /*= ""*/,
								 OPTIONAL IN CString strInsPartyAddress1 /*= ""*/,
								 OPTIONAL IN CString strInsPartyAddress2 /*= ""*/,
								 OPTIONAL IN CString strInsPartyCity /*= ""*/,
								 OPTIONAL IN CString strInsPartyState /*= ""*/,
								 OPTIONAL IN CString strInsPartyZip /*= ""*/,
								 OPTIONAL IN CString strInsPartyCountry /*= ""*/,
								 OPTIONAL IN CString strInsPartyPhone /*= ""*/,
								 OPTIONAL IN CString strInsPartyEmployerSchool /*= ""*/,
								 OPTIONAL IN long nInsPartyGender /*= 0*/,
								 OPTIONAL IN COleDateTime dtInsPartyBirthDate /*= COleDateTime(0,0,0,0,0,0)*/,
								 OPTIONAL IN CString strInsPartySSN /*= ""*/,
								 OPTIONAL IN bool bUpdatePatientHL7 /*= true*/)

{
	//
	//First, we need to do all the proper checks for data.  Anything that causes a question or failure should
	//	be determined before we attempt to insert.  This function will do all tests, and fill in our
	//	InsCo, RespType, and InsContact values for us.
	//
	long nInsContactPersonID;
	if(!DetermineInsuredPartyPreRequisites(nPatientPersonID, nOverrideInsCoPersonID, nOverrideRespTypeID, 
		nInsCoPersonID, nRespTypeID, nInsContactPersonID))
	{
		//If our checks failed, we must fail
		return false;
	}



	//
	//Now that we have passed all of our data checks, we can create the actual record.
	CString strSqlBatch;
	CNxParamSqlArray args;

	AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON;\r\n");

	//Replacement for NewNumber, determine the next PersonID we'll be using.
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewPersonID int;\r\n");
	AddStatementToSqlBatch(strSqlBatch, "SET @NewPersonID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM PersonT);\r\n");

	//Set the InsCo PersonID as a variable, just makes it easier to write the query.
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @InsCoPersonID int;\r\n");
	AddParamStatementToSqlBatch(strSqlBatch, args, "SET @InsCoPersonID = {INT};\r\n", nInsCoPersonID);

	//Patient PersonID too
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @PatientPersonID int;\r\n");
	AddParamStatementToSqlBatch(strSqlBatch, args, "SET @PatientPersonID = {INT};\r\n", nPatientPersonID);


	//Matching past behavior:  If there are more than 1 Plan IDs for this ins co, we do not set a default, and force the user
	//	to choose one in the interface.
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @InsurancePlanID int;\r\n");
	//We do allow an override of the plan, in which case the past behavior is ignored
	//	Safety Check:  If the InsPlanID does not match the InsCoID, don't let them insert bad data, instead we'll insert NULL.
	if(nOverrideInsCoPersonID != -1 && nOverrideInsPlanID != -1) {
		AddParamStatementToSqlBatch(strSqlBatch, args, "SET @InsurancePlanID = (SELECT ID FROM InsurancePlansT WHERE InsCoID = @InsCoPersonID AND ID = {INT});\r\n", nOverrideInsPlanID);
	}
	else {
		//Old behavior
		AddStatementToSqlBatch(strSqlBatch, 
			"IF (SELECT COUNT(*) FROM InsurancePlansT WHERE InsCoID = @InsCoPersonID) = 1 \r\n"
				//There is only 1 insurance plan, so choose it
				"	BEGIN SET @InsurancePlanID = (SELECT ID FROM InsurancePlansT WHERE InsCoID = @InsCoPersonID); END\r\n"
			"ELSE \r\n"
				//There are many insurance plans, so we have to go NULL
			"	BEGIN SET @InsurancePlanID = NULL; END\r\n");
	}

	//RespTypeID
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @RespTypeID int;\r\n");
	AddParamStatementToSqlBatch(strSqlBatch, args, "SET @RespTypeID = {INT};\r\n", nRespTypeID);

	_variant_t varBirthdate;
	varBirthdate.vt = VT_NULL;
	if (dtInsPartyBirthDate.GetStatus() == COleDateTime::valid) {
		varBirthdate = COleVariant(dtInsPartyBirthDate);
	}

	// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country, which is nullable
	CString strCountry = strInsPartyCountry;
	strCountry.TrimLeft(); strCountry.TrimRight();
	_variant_t varInsPartyCountry = g_cvarNull;
	if(!strCountry.IsEmpty()) {
		varInsPartyCountry = (LPCSTR)strCountry;
	}

	//Now just simply insert the records to PersonT and InsuredPartyT
	// (j.gruber 2009-10-12 13:49) - PLID 10723 - added demographic fields
	// (j.jones 2012-10-25 09:35) - PLID 36305 - added Title
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
	AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO PersonT (ID, First, Middle, Last, Title, "
		"Address1, Address2, City, State, Zip, Country, HomePhone, "
		"UserID, Gender, Birthdate, SocialSecurity) VALUES "
		"(@NewPersonID, {STRING}, {STRING}, {STRING}, {STRING}, "
		"{STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {VT_BSTR}, {STRING}, "
		"{INT}, {INT}, {VT_DATE}, {STRING});\r\n", strInsPartyFirst, strInsPartyMiddle, strInsPartyLast, strInsPartyTitle,
		strInsPartyAddress1, strInsPartyAddress2, strInsPartyCity, strInsPartyState, strInsPartyZip, varInsPartyCountry, strInsPartyPhone,
		GetCurrentUserID(), nInsPartyGender, varBirthdate, strInsPartySSN);
		
	//DRT 5/7/03 - put in an inactive ins resp by default, in case
	// (j.gruber 2009-10-12 13:55) - PLID 10723 - added employer, IDForinsurance, PolicyGroupNum, and RelationToPatient

	// (j.jones 2011-06-24 12:28) - PLID 29885 - added a default secondary reason code
	CString strSecondaryReasonCode = GetRemotePropertyText("DefaultMedicareSecondaryReasonCode", "47", 0, "<None>", true);
	if(strSecondaryReasonCode.IsEmpty()) {
		strSecondaryReasonCode = "47";
	}

	// (r.goldschmidt 2014-07-31 15:26) - PLID 63111 - add total deductible, total oop, and per pay group bit
	AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO InsuredPartyT "
		"(PersonID, InsuranceCoID, InsuranceContactID, PatientID, AssignDate, RelationToPatient, RespTypeID, InsPlan, "
		"Employer, IDForInsurance, PolicyGroupNum, SecondaryReasonCode, TotalDeductible, TotalOOP, DeductiblePerPayGroup) VALUES "
		"(@NewPersonID, @InsCoPersonID, {INT}, @PatientPersonID, GetDate(), {STRING}, "
		"@RespTypeID, @InsurancePlanID, {STRING}, {STRING}, {STRING}, {STRING}, {VT_CY}, {VT_CY}, {BOOL});\r\n", 
		nInsContactPersonID, 
		strInsPartyRelationToPatient, 
		strInsPartyEmployerSchool, 
		strInsPartyIDForIns, 
		strInsPartyPolicyGroupNum, 
		strSecondaryReasonCode,
		(!bPerPayGroup && cyTotalDeductible.GetStatus() == COleCurrency::valid) ? variant_t(cyTotalDeductible) : g_cvarNull,
		(!bPerPayGroup && cyTotalOOP.GetStatus() == COleCurrency::valid) ? variant_t(cyTotalOOP) : g_cvarNull,
		bPerPayGroup);

	//This used to be done afterwards when the resp type was set.  Let's add it in now -- If the resp type is inactive, 
	//	we want to set the expiration date to immediate.
	if(nRespTypeID == -1) {
		AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuredPartyT SET ExpireDate = GetDate() WHERE PersonID = @NewPersonID;\r\n");
	}

	//This was also done afterwrads, if the insurance company is Workers comp, set the default injury date as the effective date.  I removed
	//	the check for NULL date for efficiency -- the default value of EffectiveDate is NULL, so there's no sense writing an extra
	//	queries to get around it.
	AddStatementToSqlBatch(strSqlBatch, 
		"IF EXISTS(SELECT PersonID FROM InsuranceCoT WHERE WorkersComp = 1 AND InsuranceCoT.PersonID = @InsCoPersonID) \r\n"
		"BEGIN "
		"	UPDATE InsuredPartyT SET EffectiveDate = (SELECT DefaultInjuryDate FROM PatientsT WHERE PatientsT.PersonID = @PatientPersonID) "
		"	WHERE InsuredPartyT.PersonID = @NewPersonID "
		"END ");

	//
	//Lastly, we need to pull some of this information back out to return to the caller
	//
	AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT OFF;\r\n");
	AddStatementToSqlBatch(strSqlBatch, "SELECT @NewPersonID AS NewPersonID;\r\n");
	AddStatementToSqlBatch(strSqlBatch, "SELECT @InsurancePlanID AS InsPlanID;\r\n");
	//For auditing
	AddStatementToSqlBatch(strSqlBatch, "SELECT Name FROM InsuranceCoT WHERE PersonID = @InsCoPersonID;\r\n");

	//
	//Execute the query
	//
	// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
	_RecordsetPtr prsCreation = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
	if(prsCreation->eof) {
		//This should not be possible, we need those results.
		AfxThrowNxException("The insured party record was successfully created, but results could not be determined.  Please "
			"check the insurance information for this patient manually.");
	}

	//
	//Now that we're done, pull back the data we wanted
	//
	nNewPersonID = AdoFldLong(prsCreation, "NewPersonID");		//Cannot be NULL
	prsCreation = prsCreation->NextRecordset(NULL);
	varNewPlanID = prsCreation->Fields->Item["InsPlanID"]->Value;	//May be NULL
	prsCreation = prsCreation->NextRecordset(NULL);
	CString strCompanyName = AdoFldString(prsCreation, "Name");


	// The insurance billing dialog needs to be updated
	// through the network code
	CClient::RefreshTable(NetUtils::PatInsParty);


	//
	//	Auditing!  slightly modified -- I don't see why it was making 2 audit events before, there should be only 1.
	//
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(nPatientPersonID, strPatientName, nAuditID, aeiInsuredPartyAdded, nPatientPersonID, "", strCompanyName, aepMedium, aetCreated);
	AuditEvent(nPatientPersonID, strPatientName, nAuditID, aeiInsPartyCopay, nPatientPersonID, "", "0", aepMedium, aetChanged);

	// (r.goldschmidt 2014-07-31 17:41) - PLID 63111 - Rework saving of Insured Party Pay Groups and Insurance Co Deductibles when creating New Patient
	CreateInsuredPartyPayGroups(nNewPersonID, bPerPayGroup, mapInsPartyPayGroupValues);

	// (r.goldschmidt 2014-08-01) - PLID 63111 - InsertDefaultDeductibles no longer called automatically on new insured party creation.
	//                                           Must be manually called in situations when default deductibles are desired.

	// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
	// (r.gonet 2015-11-12 11:14) - PLID 66907 - Only if the caller wants it sent.
	if (bUpdatePatientHL7){
		UpdateExistingPatientInHL7(nPatientPersonID, TRUE);
	}

	//Success!
	return true;
}


// (d.thompson 2009-03-19) - PLID 33590 - Brought here and globalized from InsuranceDlg.cpp, optimized as well.
// (r.gonet 2015-11-12 11:14) - PLID 66907 - New optional parameter bUpdatePatientInHL7.
//Parameters:
//	nPatientPersonID:		The PatientsT.PersonID for the insured party record.  This is used to identify auditing.  Input Only.
//	nInsPartyPersonID:		The InsuredPartyT.PersonID value where the info should be copied to.  Input Only.
//	strPatientName:			The patient's name (PatientsT record), used for auditing.  Input Only.
//	bUpdatePatientInHL7:	Flag controlling whether an HL7 message is generated after the insured party isupdated.
//							When true is passed, the function sends an HL7 patient update message after creating the 
//							new insured party. When false is pased, no HL7 patient update message is sent.
//							Optional. Defaults to true because that is legacy behavior and most places calling this function
//							would want an HL7 message sent after the save to data.
//
//Return Values:
//		false				Will only happen if an exception occurs and is caught internally.
//		true				Returned if the function succeeds in copying info.
//	This function does not throw exceptions.
bool CopyPatientInfoToInsuredParty(IN long nPatientPersonID, IN long nInsPartyPersonID, IN CString strPatientName, OPTIONAL IN bool bUpdatePatientHL7 /*= true*/)
{
	//TES 5/1/2008 - PLID 27576 - Need to be able to rollback this transaction if there is an exception.
	long nAuditTransactionID = -1;
	try {
		// m.carlson 5/18/2004, PL 11504 (for auditing)

		CString strOldBDay,strOldFirst,strOldMiddle,strOldLast,strOldTitle,strOldAddr1,strOldAddr2,strOldCity,strOldState,strOldZip,strOldHomePhone,strOldEmployer,strOldRelation, strOldSSN;
		COleDateTime dtOldBirthDate;
		CString strNewBDay,strNewFirst,strNewMiddle,strNewLast,strNewTitle,strNewAddr1,strNewAddr2,strNewCity,strNewState,strNewZip,strNewHomePhone,strNewEmployer,strNewRelation, strNewSSN;
		COleDateTime dtNewBirthDate;
		_variant_t vOldGender,vNewGender;
		CString strOldGender,strNewGender;
		//PLID 20361: make these 3 queries into 1
		// (d.thompson 2009-03-19) - PLID 33590 - Parameterized
		// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
		_RecordsetPtr rsPatient = CreateParamRecordset("SELECT InsPersonT.Company, InsPersonT.First,InsPersonT.Middle, "
			" InsPersonT.Last,InsPersonT.Title,InsPersonT.Address1,InsPersonT.Address2,InsPersonT.City,InsPersonT.State,InsPersonT.Zip, "
			" InsPersonT.HomePhone,InsPersonT.BirthDate,InsPersonT.Gender, InsPersonT.SocialSecurity AS SSN, "
			" Employer,RelationToPatient,PatientID, "
			" PatPersonT.Company as PatCompany, PatPersonT.First as PatFirst,PatPersonT.Middle as PatMiddle, "
			" PatPersonT.Last as PatLast, PatPersonT.Title AS PatTitle, PatPersonT.Address1 as PatAddress1, PatPersonT.Address2 as PatAddress2, "
			" PatPersonT.City as PatCity, PatPersonT.State as PatState, PatPersonT.Zip as PatZip, "
			" PatPersonT.BirthDate as PatBirthDate,PatPersonT.HomePhone as PatHomePhone ,PatPersonT.Gender as PatGender, "
			" PatPersonT.SocialSecurity AS PatSSN "
			" FROM PersonT InsPersonT INNER JOIN InsuredPartyT ON "
			" InsPersonT.ID = InsuredPartyT.PersonID  "
			" INNER JOIN PersonT PatPersonT ON InsuredPartyT.PatientID = PatPersonT.ID "
			" WHERE InsPersonT.ID = {INT}", nInsPartyPersonID);
		//_RecordsetPtr rsOld = CreateRecordset("SELECT Company,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,BirthDate,Gender, SocialSecurity AS SSN FROM PersonT WHERE ID=%li",m_CurrentID);	//for auditing

		if (! rsPatient->eof) {
			FieldsPtr flds = rsPatient->Fields;

			strOldFirst = AdoFldString(flds,"First");
			strOldMiddle = AdoFldString(flds,"Middle");
			strOldLast = AdoFldString(flds,"Last");
			// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
			strOldTitle = AdoFldString(flds,"Title");
			strOldAddr1 = AdoFldString(flds,"Address1");
			strOldAddr2 = AdoFldString(flds,"Address2");
			strOldCity = AdoFldString(flds,"City");
			strOldState = AdoFldString(flds,"State");
			strOldZip = AdoFldString(flds,"Zip");
			strOldSSN = AdoFldString(flds,"SSN");
			strOldHomePhone = AdoFldString(flds,"HomePhone");
			dtOldBirthDate = AdoFldDateTime(flds,"BirthDate",COleDateTime::COleDateTime(1800,1,1,1,25,25));
			vOldGender = flds->Item["Gender"]->Value;

			//_RecordsetPtr rsOld2 = CreateRecordset("SELECT Employer,RelationToPatient,PatientID FROM InsuredPartyT WHERE PersonID=%li",m_CurrentID);

			strOldEmployer = AdoFldString(flds,"Employer");
			strOldRelation = AdoFldString(flds,"RelationToPatient");
			// (d.thompson 2009-03-19) - PLID 33590 - Not needed at all
			//nPatID = AdoFldLong(flds,"PatientID");

			//_RecordsetPtr rsNew = CreateRecordset("SELECT Company,First,Middle,Last,Address1,Address2,City,State,Zip,BirthDate,HomePhone,Gender, SocialSecurity AS SSN FROM PersonT WHERE ID = %li",nPatID);

			strNewEmployer = AdoFldString(flds,"PatCompany");
			strNewFirst = AdoFldString(flds,"PatFirst");
			strNewMiddle = AdoFldString(flds,"PatMiddle");
			strNewLast = AdoFldString(flds,"PatLast");
			// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
			strNewTitle = AdoFldString(flds,"PatTitle");
			strNewAddr1 = AdoFldString(flds,"PatAddress1");
			strNewAddr2 = AdoFldString(flds,"PatAddress2");
			strNewCity = AdoFldString(flds,"PatCity");
			strNewState = AdoFldString(flds,"PatState");
			strNewZip = AdoFldString(flds,"PatZip");
			strNewSSN = AdoFldString(flds,"PatSSN");
			dtNewBirthDate = AdoFldDateTime(flds,"PatBirthDate",COleDateTime::COleDateTime(1800,1,1,1,25,25));
			strNewHomePhone = AdoFldString(flds,"PatHomePhone");
			vNewGender = flds->Item["PatGender"]->Value;

			strNewRelation = "Self";
		}
		// (r.gonet 2015-11-12 11:14) - PLID 66907 - Close the recordset. Free up resources.
		rsPatient->Close();

		/*
		long nAuditTransactionID = BeginAuditTransaction();
		AuditEvent.... normal but use the nAuditTransactionID
		if(nAuditTransactionID != -1)
		CommitAuditTransaction()
		Obviously initialize the ID to -1 and for each AuditEvent check to see if the ID is -1, if so then call the BeginAuditTransaction() at that point, so in the end you'll only commit it if it's not -1.
*/
		//Batch the AuditTransactions
		if (strOldFirst != strNewFirst) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyFirst, nPatientPersonID, strOldFirst, strNewFirst, aepMedium, aetChanged);
		}
		if (strOldMiddle != strNewMiddle) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyMiddle, nPatientPersonID, strOldMiddle, strNewMiddle, aepMedium, aetChanged);
		}
		if (strOldLast != strNewLast) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyLast, nPatientPersonID, strOldLast, strNewLast, aepMedium, aetChanged);
		}
		// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
		if (strOldTitle != strNewTitle) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}					 
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyTitle, nPatientPersonID, strOldTitle, strNewTitle, aepMedium, aetChanged);
		}
		if (strOldAddr1 != strNewAddr1) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyAddress1, nPatientPersonID, strOldAddr1, strNewAddr1, aepMedium, aetChanged);
		}
		if (strOldAddr2 != strNewAddr2) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyAddress2, nPatientPersonID, strOldAddr2, strNewAddr2, aepMedium, aetChanged);
		}
		if (strOldCity != strNewCity) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyCity, nPatientPersonID, strOldCity, strNewCity, aepMedium, aetChanged);
		}
		if (strOldState != strNewState) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyState, nPatientPersonID, strOldState, strNewState, aepMedium, aetChanged);
		}
		if (strOldZip != strNewZip) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyZip, nPatientPersonID, strOldZip, strNewZip, aepMedium, aetChanged);
		}
		if (strOldHomePhone != strNewHomePhone) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyPhone, nPatientPersonID, strOldHomePhone, strNewHomePhone, aepMedium, aetChanged);
		}
		if (strOldRelation != strNewRelation) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyRelation, nPatientPersonID, strOldRelation, strNewRelation, aepMedium, aetChanged);
		}
		if (strOldEmployer != strNewEmployer) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}				
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyEmployer, nPatientPersonID, strOldEmployer, strNewEmployer, aepMedium, aetChanged);
		}

		if (vOldGender.vt != VT_NULL)
		{
			if (VarByte(vOldGender,0) == 1) strOldGender = "Male";
			else if (VarByte(vOldGender,0) == 2) strOldGender = "Female";
		}

		if (vNewGender.vt != VT_NULL)
		{
			if (VarByte(vNewGender,0) == 1) strNewGender = "Male";
			else if (VarByte(vNewGender,0) == 2) strNewGender = "Female";
		}

		if ((strOldGender != strNewGender)) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyGender, nPatientPersonID, strOldGender, strNewGender, aepMedium, aetChanged);
		}

		if (dtOldBirthDate != COleDateTime::COleDateTime(1800,1,1,1,25,25)) {
			strOldBDay = FormatDateTimeForInterface(COleDateTime(dtOldBirthDate), dtoDate);
		}
		// (d.thompson 2009-09-21) - PLID 33645 - Audit the birthdate correctly
		if(dtNewBirthDate != COleDateTime::COleDateTime(1800,1,1,1,25,25) && dtNewBirthDate.GetStatus() == COleDateTime::valid) {
			strNewBDay = FormatDateTimeForInterface(dtNewBirthDate, dtoDate);
		}

		if (strOldBDay != strNewBDay) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartyDateofBirth, nPatientPersonID, strOldBDay, strNewBDay, aepMedium, aetChanged);
		}

		if (strOldSSN != strNewSSN) {
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientPersonID, strPatientName, nAuditTransactionID, aeiInsPartySSN, nPatientPersonID, strOldSSN, strNewSSN, aepMedium, aetChanged);
		}

		// m.carlson 5/18/2004 : end auditing stuff
		// (d.thompson 2009-03-19) - PLID 33590 - Combined and parameterized these queries
		CString strSqlBatch;
		CNxParamSqlArray args;
		// (j.jones 2012-10-24 15:24) - PLID 36305 - added Title
		// (j.jones 2012-11-12 14:58) - PLID 53622 - added Country
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			//Query 1 - PersonT.* data
			"UPDATE PersonT SET PersonT.[First] = PersonT_1.[First], PersonT.Middle = PersonT_1.Middle, PersonT.[Last] = PersonT_1.[Last], PersonT.Title = PersonT_1.Title, PersonT.Address1 = PersonT_1.Address1, PersonT.Address2 = PersonT_1.Address2, PersonT.City = PersonT_1.City, PersonT.State = PersonT_1.State, PersonT.Zip = PersonT_1.Zip, PersonT.HomePhone = PersonT_1.HomePhone, PersonT.BirthDate = PersonT_1.BirthDate, PersonT.Gender = PersonT_1.Gender, PersonT.SocialSecurity = PersonT_1.SocialSecurity, PersonT.Country = PersonT_1.Country "
				"FROM PatientsT RIGHT JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID INNER JOIN (SELECT * FROM PersonT) AS PersonT_1 ON PatientsT.PersonID = PersonT_1.ID INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
				"WHERE InsuredPartyT.PersonID = {INT};\r\n", nInsPartyPersonID);
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			//Query 2 - InsuredPartyT.* data
			"UPDATE InsuredPartyT SET InsuredPartyT.Employer = PersonT_1.Company, InsuredPartyT.RelationToPatient = 'Self' "
				"FROM PatientsT RIGHT JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID INNER JOIN (SELECT * FROM PersonT) AS PersonT_1 ON PatientsT.PersonID = PersonT_1.ID INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
				"WHERE InsuredPartyT.PersonID = {INT}", nInsPartyPersonID);
		// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

		//commit auditing
		//TES 5/1/2008 - PLID 27576 - Do this AFTER the changes have been successfully saved.
		if (nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
		// (r.gonet 2015-11-12 11:14) - PLID 66907 - Only if the caller wants it sent.
		if (bUpdatePatientHL7){
			UpdateExistingPatientInHL7(nPatientPersonID, TRUE);
		}

		//Success!
		return true;

	}NxCatchAllCall("Error copying patient info: ",
		if(nAuditTransactionID != -1) {
			//TES 5/1/2008 - PLID 27576 - Need to rollback
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	//Exception failure
	return false;
}


//r.wilson (8/23/2012) PLID 52222 - Insert Default Paygroup values for the given insurance co for the given insured party
// (r.goldschmidt 2014-08-01 17:19) - PLID 63111 - must now be called manually since creation of new insured party does not automatically set defaults
void InsertDefaultDeductibles(long nInsuranceCoID, long nInsuredParty)
{
	CSqlFragment sqlFrag;	
	
	_RecordsetPtr rs = CreateParamRecordset(" SELECT DefaultDeductiblePerPayGroup, DefaultTotalDeductible, DefaultTotalOOP FROM InsuranceCoT WHERE PersonID = {INT} ; \r\n"
											" "
											" SELECT ServicePaygroupsT.ID AS ServicePaygroupID, PayGroupDefaultsT.ID AS PayGroupDefaultsID , CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP "
											" FROM ServicePaygroupsT "
											" INNER JOIN (SELECT ID, PayGroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP " 
											"				FROM InsuranceCoPayGroupsDefaultsT  "
											"				WHERE InsuranceCoID = {INT} ) "
											" AS PayGroupDefaultsT ON ServicePaygroupsT.ID = PayGroupDefaultsT.PaygroupID ;",
											nInsuranceCoID,nInsuranceCoID);

	_variant_t vtCopayMoney, vtCopayPercentage, vtCoInsurance, vtTotalDeductible, vtTotalOOP;
	_variant_t vtDefaultDeductiblePerPayGroup, vtDefaultTotalDeductible, vtDefaultTotalOOP;
	
	//r.wilson (8/23/2012) PLID 52222 - Get default values from the given insurance company
	if(!rs->eof)
	{	
		
		vtDefaultDeductiblePerPayGroup = rs->Fields->Item["DefaultDeductiblePerPayGroup"]->Value;
		vtDefaultTotalDeductible = rs->Fields->Item["DefaultTotalDeductible"]->Value;
		vtDefaultTotalOOP = rs->Fields->Item["DefaultTotalOOP"]->Value;
	}

	//r.wilson (8/23/2012) PLID 52222 - Move to next recordset
	rs = rs->NextRecordset(NULL);

	//r.wilson (8/23/2012) PLID 52222 - for each paygroup that exists do the following
	while(!rs->eof)
	{
		long nPaygroupID = VarLong(rs->Fields->Item["ServicePaygroupID"]->Value);
		long nPayGroupDefaultsID = VarLong(rs->Fields->Item["PayGroupDefaultsID"]->Value, (long) -1);

		//r.wilson (8/23/2012) PLID 52222 - Only insert default values for ServicePaygroups with entries in the InsuranceCoPayGroupsDefaultsT table
		if(nPayGroupDefaultsID != -1){
			vtCopayMoney = rs->Fields->Item["CopayMoney"]->Value;
			vtCopayPercentage = rs->Fields->Item["CopayPercentage"]->Value;
			vtCoInsurance = rs->Fields->Item["CoInsurance"]->Value;
			vtTotalDeductible = rs->Fields->Item["TotalDeductible"]->Value;
			vtTotalOOP = rs->Fields->Item["TotalOOP"]->Value;

			if(vtTotalDeductible.vt != VT_CY){
				vtTotalDeductible = g_cvarNull;
			}

			if(vtTotalOOP.vt != VT_CY){
				vtTotalOOP = g_cvarNull;
			}

			// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
			if (vtCopayMoney.vt == VT_CY) {
				COleCurrency cyCopay = VarCurrency(vtCopayMoney);
				RoundCurrency(cyCopay);
				vtCopayMoney = _variant_t(cyCopay);
			}

			//r.wilson (8/23/2012) PLID 52222 - Insert pay group values
			sqlFrag += CSqlFragment(" INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PaygroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP) "
									" VALUES "
									" ({INT},{INT},{VT_CY},{VT_I4},{VT_I4},{VT_CY},{VT_CY}) ; \r\n",
									nInsuredParty, nPaygroupID,vtCopayMoney, vtCopayPercentage, vtCoInsurance, 
									VarBool(vtDefaultDeductiblePerPayGroup) == TRUE ? vtTotalDeductible : g_cvarNull , 
									VarBool(vtDefaultDeductiblePerPayGroup) == TRUE ? vtTotalOOP : g_cvarNull
									);										
		}

		rs->MoveNext();
	}

	//r.wilson (8/23/2012) PLID 52222 - This puts the appropriate values in for the insured party's TotalDeductible and TotalOOP
	if(VarBool(vtDefaultDeductiblePerPayGroup) == FALSE)
	{
			sqlFrag += CSqlFragment(" UPDATE InsuredPartyT "
									" SET "
									"     TotalDeductible = {VT_CY}, TotalOOP = {VT_CY}, DeductiblePerPayGroup = {VT_BOOL}, LastModifiedDate = GetDate() "
									" WHERE "
									" PersonID = {INT} AND InsuranceCoID = {INT}; \r\n",																																		
									vtDefaultTotalDeductible , 
									vtDefaultTotalOOP,
									vtDefaultDeductiblePerPayGroup,
									nInsuredParty, nInsuranceCoID
									);
	}
	else{
			sqlFrag += CSqlFragment(" UPDATE InsuredPartyT "
									" SET "
									"	 DeductiblePerPayGroup = {VT_BOOL} "
									" WHERE "
									" PersonID = {INT} AND InsuranceCoID = {INT}; \r\n",																																		
					
									vtDefaultDeductiblePerPayGroup,
									nInsuredParty, nInsuranceCoID
									);
	}
	
	if(sqlFrag.IsEmpty() == false){
		ExecuteParamSql(GetRemoteData(), sqlFrag);
	}

}

// (r.goldschmidt 2014-07-30) - PLID 63111 - create insured party pay group records
void CreateInsuredPartyPayGroups(long nInsuredPartyID, bool bPerPayGroup,
	const CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues> &mapInsPartyPayGroupValues)
{

	// return if there are no insured party pay groups to insert
	if (mapInsPartyPayGroupValues.IsEmpty()){
		return;
	}

	CString strSqlBatch;
	CNxParamSqlArray args;

	POSITION pos = mapInsPartyPayGroupValues.GetStartPosition();
	long nPayGroupID;
	InsuredPartyPayGroupValues sPayGroupVals;

	// Insert record to InsuredPartyPayGroupsT for every InsuredPartyPayGroupValues in the CMap
	while (pos != NULL) {
		mapInsPartyPayGroupValues.GetNextAssoc(pos, nPayGroupID, sPayGroupVals);
		
		// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
		_variant_t varCopay = g_cvarNull;
		if (sPayGroupVals.isCopayMoneyValid()) {
			RoundCurrency(sPayGroupVals.m_cyCopayMoney);
			varCopay = _variant_t(sPayGroupVals.m_cyCopayMoney);
		}

		AddParamStatementToSqlBatch(strSqlBatch, args, " "
			" INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PaygroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP) "
			" VALUES "
			" ({INT},{INT},{VT_CY},{VT_I4},{VT_I4},{VT_CY},{VT_CY}) ; \r\n",
			nInsuredPartyID,
			nPayGroupID,
			varCopay,
			sPayGroupVals.isCopayPercentValid() ? variant_t(sPayGroupVals.m_nCopayPercent) : g_cvarNull,
			sPayGroupVals.isCoInsPercentValid() ? variant_t(sPayGroupVals.m_nCoInsPercent) : g_cvarNull,
			(bPerPayGroup && sPayGroupVals.isTotalDeductibleValid()) ? variant_t(sPayGroupVals.m_cyTotalDeductible) : g_cvarNull,
			(bPerPayGroup && sPayGroupVals.isTotalOOPValid()) ? variant_t(sPayGroupVals.m_cyTotalOOP) : g_cvarNull
			);
	}

	ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);

}

//The UI to edit pay groups and immediately save is in multiple places in the program.
//These functions ensure those places behave identically.
void OnEditingStartingPayGroupList_Global(CWnd *pParentWnd, NXDATALIST2Lib::_DNxDataListPtr &pPayGroupList, NXDATALIST2Lib::IRowSettingsPtr &pRow, 
	short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	//throw all exceptions to the caller

	if (pRow == NULL) {
		return;
	}

	if (PayGroupsColumn_Global::gpgcCopayPercent < nCol) {
		//should be impossible, you must have added a new column without updating the global enum!
		ThrowNxException("OnEditingStartingPayGroupList_Global called with an invalid column ID.");
	}

	_variant_t varCopay;
	_variant_t varPercent;

	varCopay = pRow->GetValue(gpgcCopayMoney);
	varPercent = pRow->GetValue(gpgcCopayPercent);

	BOOL bError = FALSE;

	switch (nCol) {

		case gpgcCopayMoney:
			//we can't have money if the percent is already filled in
			if (varPercent.vt != VT_NULL && varPercent.vt != VT_EMPTY && (varCopay.vt == VT_NULL || varCopay.vt == VT_EMPTY)) {
				bError = TRUE;
			}
			break;

		case gpgcCopayPercent:
			//we can't have percent if the money is already filled in
			if (varCopay.vt != VT_NULL && varCopay.vt != VT_EMPTY && (varPercent.vt == VT_NULL || varPercent.vt == VT_EMPTY)) {
				bError = TRUE;
			}
			break;
	}

	if (bError) {
		MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
			"You cannot enter both a copay currency amount and a copay percent amount.", "Practice", MB_ICONEXCLAMATION|MB_OK);
		*pbContinue = FALSE;
	}
}

//The UI to edit pay groups and immediately save is in multiple places in the program.
//These functions ensure those places behave identically.
void OnEditingFinishingPayGroupList_Global(CWnd *pParentWnd, NXDATALIST2Lib::_DNxDataListPtr &pPayGroupList, NXDATALIST2Lib::IRowSettingsPtr &pRow,
	short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	//throw all exceptions to the caller

	if (pRow == NULL) {
		return;
	}

	if (PayGroupsColumn_Global::gpgcCopayPercent < nCol) {
		//should be impossible, you must have added a new column without updating the global enum!
		ThrowNxException("OnEditingFinishingPayGroupList_Global called with an invalid column ID.");
	}

	if (*pbContinue && *pbCommit) {

		CString strEntered = strUserEntered;

		_variant_t varCopay;
		_variant_t varPercent;

		varCopay = pRow->GetValue(gpgcCopayMoney);
		varPercent = pRow->GetValue(gpgcCopayPercent);

		switch (nCol) {

			case gpgcCoIns:
			{
				if (strEntered.IsEmpty()) {
					//put a null value in
					*pvarNewValue = g_cvarNull;
					return;
				}

				//check to make sure its a number
				if (!IsNumeric(strEntered)) {
					*pvarNewValue = g_cvarNull;
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"Please enter a valid percentage value.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}

				long nPercent = atoi(strEntered);
				if (nPercent < 0 || nPercent > 100) {
					*pvarNewValue = g_cvarNull;
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"Please enter a valid percentage value.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}
			}

			break;

			case gpgcCopayMoney:
			{
				// (b.spivey, March 26, 2015) - PLID 56838 - Don't allow them to save a value in here if there's one in the other column
				//we can't have percent if the money is already filled in
				if (varPercent.vt != VT_NULL && varPercent.vt != VT_EMPTY && (!strEntered.IsEmpty())) {
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"You cannot enter both a copay currency amount and a copay percent amount.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pvarNewValue = g_cvarNull;
					return;
				}

				if (strEntered.IsEmpty()) {
					//put a null value in
					*pvarNewValue = g_cvarNull;
					return;
				}

				//check to make sure its a valid currency
				COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
				if (cyAmt.GetStatus() != COleCurrency::valid) {
					*pvarNewValue = g_cvarNull;
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"Please enter a valid currency.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}

				if (cyAmt < COleCurrency(0, 0)) {
					*pvarNewValue = g_cvarNull;
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"Please enter a currency greater than 0.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}
			}

			break;

			case gpgcCopayPercent:
			{
				// (b.spivey, March 26, 2015) - PLID 56838 - Don't allow them to save a value in here if there's one in the other column
				//we can't have money if the percent is already filled in
				if (varCopay.vt != VT_NULL && varCopay.vt != VT_EMPTY && (!strEntered.IsEmpty())) {
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"You cannot enter both a copay currency amount and a copay percent amount.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pvarNewValue = g_cvarNull;
					return;
				}

				if (strEntered.IsEmpty()) {
					//put a null value in
					*pvarNewValue = g_cvarNull;
					return;
				}

				//check to make sure its a number
				if (!IsNumeric(strEntered)) {
					*pvarNewValue = g_cvarNull;
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"Please enter a valid percentage value.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}

				long nPercent = atoi(strEntered);
				if (nPercent < 0 || nPercent > 100) {
					*pvarNewValue = g_cvarNull;
					MessageBox(pParentWnd ? pParentWnd->GetSafeHwnd() : GetActiveWindow(),
						"Please enter a valid percentage value.", "Practice", MB_ICONEXCLAMATION | MB_OK);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}
			}
			break;
		}
	}
}

//The UI to edit pay groups and immediately save is in multiple places in the program.
//These functions ensure those places behave identically.
void OnEditingFinishedPayGroupList_Global(CWnd *pParentWnd, NXDATALIST2Lib::_DNxDataListPtr &pPayGroupList, NXDATALIST2Lib::IRowSettingsPtr &pRow,
	long nPatientID, CString strPatientName, long nInsuredPartyID,
	short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	//throw all exceptions to the caller

	if (pRow == NULL) {
		return;
	}

	if (PayGroupsColumn_Global::gpgcCopayPercent < nCol) {
		//should be impossible, you must have added a new column without updating the global enum!
		ThrowNxException("OnEditingFinishedPayGroupList_Global called with an invalid column ID.");
	}

	if (bCommit) {

		_variant_t varID = pRow->GetValue(gpgcID);
		long nPayGroupID = VarLong(pRow->GetValue(gpgcPayGroupID));

		switch (nCol) {

		case gpgcCoIns:
			if (varID.vt == VT_NULL || varID.vt == VT_EMPTY) {
				//we need to add		
				if (varNewValue.vt == VT_NULL || varNewValue.vt == VT_EMPTY) {
					//check to see if we can delete
					if (CanDeletePayGroup(pRow, nCol)) {
						//do nothing since we haven't inserted yet
					}
					else {
						//only do this if the value actually changed
						if (PayGroupValuesChanged(varOldValue, varNewValue)) {
							InsertPayGroupRecord(pRow, "CoInsurance", "VT_I4", varNewValue, nPayGroupID, nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCoInsurance);
						}
					}
				}
				else {
					//only do this if the value actually changed
					if (PayGroupValuesChanged(varOldValue, varNewValue)) {
						InsertPayGroupRecord(pRow, "CoInsurance", "VT_I4", varNewValue, nPayGroupID, nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCoInsurance);
					}
				}
			}
			else {
				if (varNewValue.vt == VT_NULL || varNewValue.vt == VT_EMPTY) {
					if (CanDeletePayGroup(pRow, nCol)) {
						if (PayGroupValuesChanged(varOldValue, varNewValue)) {
							DeletePayGroupRow(pRow, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCoInsurance, AsString(varOldValue));
						}
					}
					else {
						//update
						if (PayGroupValuesChanged(varOldValue, varNewValue)) {
							UpdatePayGroupRecord("CoInsurance", "VT_I4", varNewValue, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCoInsurance, AsString(varOldValue), VarString(pRow->GetValue(gpgcName)));
						}
					}
				}
				else {
					//Update 
					if (PayGroupValuesChanged(varOldValue, varNewValue)) {
						UpdatePayGroupRecord("CoInsurance", "VT_I4", varNewValue, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCoInsurance, AsString(varOldValue), VarString(pRow->GetValue(gpgcName)));
					}
				}
			}
			break;

		case gpgcCopayMoney: {

			// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
			_variant_t varCopay = varNewValue;
			if (varCopay.vt == VT_CY) {
				COleCurrency cyCopay = VarCurrency(varCopay);
				RoundCurrency(cyCopay);
				varCopay = _variant_t(cyCopay);
			}

			if (varID.vt == VT_NULL || varID.vt == VT_EMPTY) {
				//we need to add		
				if (varCopay.vt == VT_NULL || varCopay.vt == VT_EMPTY) {
					//check to see if we can delete
					if (CanDeletePayGroup(pRow, nCol)) {
						//do nothing since we haven't inserted yet
					}
					else {
						if (PayGroupValuesChanged(varOldValue, varCopay)) {
							InsertPayGroupRecord(pRow, "CopayMoney", "VT_CY", varCopay, nPayGroupID, nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayMoney);
						}
					}
				}
				else {
					if (PayGroupValuesChanged(varOldValue, varCopay)) {
						InsertPayGroupRecord(pRow, "CopayMoney", "VT_CY", varCopay, nPayGroupID, nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayMoney);
					}
				}
			}
			else {
				if (varCopay.vt == VT_NULL || varCopay.vt == VT_EMPTY) {
					if (CanDeletePayGroup(pRow, nCol)) {
						if (PayGroupValuesChanged(varOldValue, varCopay)) {
							DeletePayGroupRow(pRow, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayMoney, AsString(varOldValue));
						}
					}
					else {
						//update
						if (PayGroupValuesChanged(varOldValue, varCopay)) {
							UpdatePayGroupRecord("CopayMoney", "VT_CY", varCopay, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayMoney, AsString(varOldValue), VarString(pRow->GetValue(gpgcName)));
						}
					}
				}
				else {
					//Update 
					if (PayGroupValuesChanged(varOldValue, varCopay)) {
						UpdatePayGroupRecord("CopayMoney", "VT_CY", varCopay, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayMoney, AsString(varOldValue), VarString(pRow->GetValue(gpgcName)));
					}
				}
			}
			break;
		}

		case gpgcCopayPercent:
			if (varID.vt == VT_NULL || varID.vt == VT_EMPTY) {
				//we need to add		
				if (varNewValue.vt == VT_NULL || varNewValue.vt == VT_EMPTY) {
					//check to see if we can delete
					if (CanDeletePayGroup(pRow, nCol)) {
						//do nothing since we haven't inserted yet
					}
					else {
						if (PayGroupValuesChanged(varOldValue, varNewValue)) {
							InsertPayGroupRecord(pRow, "CopayPercentage", "VT_I4", varNewValue, nPayGroupID, nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayPercent);
						}
					}
				}
				else {
					if (PayGroupValuesChanged(varOldValue, varNewValue)) {
						InsertPayGroupRecord(pRow, "CopayPercentage", "VT_I4", varNewValue, nPayGroupID, nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayPercent);
					}
				}
			}
			else {
				if (varNewValue.vt == VT_NULL || varNewValue.vt == VT_EMPTY) {
					if (CanDeletePayGroup(pRow, nCol)) {
						if (PayGroupValuesChanged(varOldValue, varNewValue)) {
							DeletePayGroupRow(pRow, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayPercent, AsString(varOldValue));
						}
					}
					else {
						//update
						if (PayGroupValuesChanged(varOldValue, varNewValue)) {
							UpdatePayGroupRecord("CopayPercentage", "VT_I4", varNewValue, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayPercent, AsString(varOldValue), VarString(pRow->GetValue(gpgcName)));
						}
					}
				}
				else {
					//Update 
					if (PayGroupValuesChanged(varOldValue, varNewValue)) {
						UpdatePayGroupRecord("CopayPercentage", "VT_I4", varNewValue, VarLong(varID), nPatientID, strPatientName, nInsuredPartyID, aeiInsPartyPayGroupCopayPercent, AsString(varOldValue), VarString(pRow->GetValue(gpgcName)));
					}
				}
			}
			break;
		}

		//sort away!
		pPayGroupList->Sort();
	}
}

//global functions to update pay group information
bool CanDeletePayGroup(NXDATALIST2Lib::IRowSettingsPtr &pRow, short nCol)
{
	if (pRow) {

		_variant_t varCoIns, varCopayMoney, varCopayPercent;

		varCoIns = pRow->GetValue(gpgcCoIns);
		varCopayMoney = pRow->GetValue(gpgcCopayMoney);
		varCopayPercent = pRow->GetValue(gpgcCopayPercent);

		switch (nCol) {

		case gpgcCoIns:
			//we have to check the copays
			if ((varCopayMoney.vt == VT_NULL || varCopayMoney.vt == VT_EMPTY) &&
				varCopayPercent.vt == VT_NULL || varCopayPercent.vt == VT_EMPTY) {
				return TRUE;
			}
			break;

		case gpgcCopayMoney:
			if ((varCoIns.vt == VT_NULL || varCoIns.vt == VT_EMPTY) &&
				varCopayPercent.vt == VT_NULL || varCopayPercent.vt == VT_EMPTY) {
				return TRUE;
			}
			break;

		case gpgcCopayPercent:
			if ((varCoIns.vt == VT_NULL || varCoIns.vt == VT_EMPTY) &&
				varCopayMoney.vt == VT_NULL || varCopayMoney.vt == VT_EMPTY) {
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

//global functions to update pay group information
void InsertPayGroupRecord(NXDATALIST2Lib::IRowSettingsPtr &pRow, CString strField, CString strType, _variant_t varValue, long nPayGroupID, long nPatientID, CString strPatientName, long nInsuredPartyID, long aeiAudit)
{
	CString strSql;
	strSql.Format("SET NOCOUNT ON "
		" INSERT INTO InsuredPartyPayGroupsT (PayGroupID, InsuredPartyID, %s) "
		" VALUES ({INT}, {INT}, {%s}) "
		" SET NOCOUNT OFF "
		" SELECT Convert(int, SCOPE_IDENTITY()) AS NewID "
		, strField, strType);

	_RecordsetPtr rs = CreateParamRecordset(strSql,
		nPayGroupID, nInsuredPartyID, varValue);

	if (!rs->eof) {
		long nNewID = AdoFldLong(rs, "NewID");

		if (pRow) {
			pRow->PutValue(gpgcID, (long)nNewID);

			CString strNewVal, strOldVal;
			strNewVal = "Pay Group: " + VarString(pRow->GetValue(gpgcName)) + " Value: ";
			strOldVal = "Pay Group: " + VarString(pRow->GetValue(gpgcName)) + " Value: ";
			if (varValue.vt == VT_NULL || varValue.vt == VT_EMPTY) {
				strNewVal += "";
			}
			else {
				strNewVal += AsString(varValue);
			}

			//audit
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(nPatientID, strPatientName, nAuditID, aeiAudit, nInsuredPartyID, strOldVal, strNewVal, aepMedium, aetCreated);
		}
		else {
			ASSERT(FALSE);
		}
	}
	else {
		ASSERT(FALSE);
	}
}

//global functions to update pay group information
void UpdatePayGroupRecord(CString strField, CString strType, _variant_t varValue, long nPayGroupID, long nPatientID, CString strPatientName, long nInsuredPartyID, long aeiAudit, CString strOldVal, CString strPayGroup)
{
	CString strSql;

	strSql.Format("UPDATE InsuredPartyPayGroupsT SET %s = {%s} WHERE ID = {INT}", strField, strType);
	ExecuteParamSql(strSql,
		varValue, nPayGroupID);

	CString strNewVal = "Pay Group: " + strPayGroup + " Value: " + AsString(varValue);
	CString strOld = "Pay Group: " + strPayGroup + " Value: " + strOldVal;

	//audit
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(nPatientID, strPatientName, nAuditID, aeiAudit, nInsuredPartyID, strOld, strNewVal, aepMedium, aetCreated);
}

//global functions to update pay group information
void DeletePayGroupRow(NXDATALIST2Lib::IRowSettingsPtr &pRow, long nPayGroupID, long nPatientID, CString strPatientName, long nInsuredPartyID, long aeiAudit, CString strOldVal)
{
	ExecuteParamSql("DELETE FROM InsuredPartyPayGroupsT WHERE ID = {INT}", nPayGroupID);
	if (pRow) {
		//make sure to clear everything out
		pRow->PutValue(gpgcID, g_cvarNull);
		pRow->PutValue(gpgcCoIns, g_cvarNull);
		pRow->PutValue(gpgcCopayMoney, g_cvarNull);
		pRow->PutValue(gpgcCopayPercent, g_cvarNull);

		CString strNewVal, strOld;
		strNewVal = "Pay Group: " + VarString(pRow->GetValue(gpgcName)) + " Value: ";
		strOld = "Pay Group: " + VarString(pRow->GetValue(gpgcName)) + " Value: " + strOldVal;

		//audit
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, strPatientName, nAuditID, aeiAudit, nInsuredPartyID, strOld, strNewVal, aepMedium, aetCreated);
	}
}

//global functions to update pay group information
bool PayGroupValuesChanged(_variant_t varOldValue, _variant_t varNewValue)
{
	if (varOldValue.vt != varNewValue.vt) {
		return true;
	}
	else if (varOldValue.vt == VT_I4 || varNewValue.vt == VT_I4) {
		if (varOldValue.lVal != varNewValue.lVal) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (varOldValue.vt == VT_CY && varNewValue.vt == VT_CY) {
		if (VarCurrency(varOldValue) != VarCurrency(varNewValue)) {
			return true;
		}
		else {
			return false;
		}
	}
	else if (varOldValue.vt == VT_NULL && varNewValue.vt == VT_NULL) {
		return false;
	}
	else if (varOldValue.vt == VT_EMPTY && varNewValue.vt == VT_EMPTY) {
		return false;
	}
	else {
		ASSERT(FALSE);
		//return true by default so that it will be saved and audited
		return true;
	}
}