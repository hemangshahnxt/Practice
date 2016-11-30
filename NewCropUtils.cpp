#include "stdafx.h"
#include "NewCropUtils.h"
#include "VersionInfo.h"
#include "Soaputils.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2009-02-12 09:42) - PLID 33163 - created

using namespace ADODB;

// (j.jones 2009-02-27 09:56) - PLID 33163 - root function for creating NewCrop XML
// from a given Patient ID, to log in to that patient's account (submitting no new info.)
// (j.gruber 2009-05-15 13:55) - PLID 28541 - changed to not add the ending NScript optionally
// (j.gruber 2009-05-15 17:36) - PLID 28541 - added ability to specify route page
// (j.gruber 2009-06-08 09:47) - PLID 34515 - added role
// (j.jones 2009-08-25 09:03) - PLID 35203 - split LinkedProviderID and SupervisingProviderID
// (j.jones 2011-06-17 11:03) - PLID 41709 - split LinkedProviderID into LicensedPrescriberID and MidlevelProviderID
// (j.jones 2013-01-03 16:24) - PLID 49624 - this function is now obsolete, its logic is now in the API
/*
BOOL GenerateNewCropPatientLoginXML(OUT CString &strXmlDocument, long nPatientID, long nLicensedPrescriberID, long nMidlevelProviderID, long nSupervisingProviderID,
									long nLocationID, NewCropUserTypes ncuTypeID, BOOL bAddNxScriptFooter,
									NewCropRequestedPageType ncrptPage)
{
	try {

		strXmlDocument = "";
		
		if(nPatientID == -1) {
			ThrowNxException("Invalid Patient ID");
		}

		// (j.gruber 2009-06-08 09:48) - PLID 34515 - only if its a licensed prescriber
		// (j.jones 2009-08-18 17:13) - PLID 35203 - supported Midlevel Prescriber, which is handled by IsNewCropPrescriberRole()
		// (j.jones 2011-06-17 11:05) - PLID 41709 - split LinkedProviderID into LicensedPrescriberID and MidlevelProviderID 
		if(ncuTypeID == ncutLicensedPrescriber && nLicensedPrescriberID == -1 ) {
			ThrowNxException("Invalid Licensed Prescriber ID");
		}

		if(ncuTypeID == ncutMidlevelProvider && nMidlevelProviderID == -1 ) {
			ThrowNxException("Invalid Midlevel Provider ID");
		}

		// (j.jones 2009-08-25 09:05) - PLID 35203 - check the nSupervisingProviderID
		if(IsNewCropSupervisedRole(ncuTypeID) && nSupervisingProviderID == -1 ) {
			ThrowNxException("Invalid Supervising Provider ID");
		}

		if(nLocationID == -1) {
			ThrowNxException("Invalid Location ID");
		}

		// (j.jones 2011-06-17 11:05) - PLID 41709 - our caller sends in LicensedPrescriberID and MidlevelProviderID,
		// but now that we are ready to export, we will only have one or the other, so use nLinkedProviderID from now on
		long nLinkedProviderID = -1;

		if(ncuTypeID == ncutLicensedPrescriber) {
			nLinkedProviderID = nLicensedPrescriberID;

			if(nLinkedProviderID == -1) {
				ThrowNxException("Invalid linked provider ID");
			}
		}

		if(ncuTypeID == ncutMidlevelProvider) {
			nLinkedProviderID = nMidlevelProviderID;

			if(nLinkedProviderID == -1) {
				ThrowNxException("Invalid linked provider ID");
			}
		}

		// (j.jones 2010-07-29 12:07) - PLID 39880 - changed the suffix logic, we now check it here,
		// and apply on load (despite the name, this applies to all suffixes, including patients)
		BOOL bHideSuffix = (GetRemotePropertyInt("NewCropHideProviderSuffix", 0, 0, "<None>", true) == 1);

		// (j.gruber 2009-05-12 12:50) - PLID 29906 - added diagnosis codes
		// (j.gruber 2009-06-08 09:54) - PLID 34515 - added staff information
		// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
		// (j.jones 2010-06-03 15:04) - PLID 38994 - supported sending the insurance company names		
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientsT.UserDefinedID, "
			"PatientPersonT.Last AS PatientLast, PatientPersonT.First AS PatientFirst, PatientPersonT.Middle AS PatientMiddle, "
			"PatientPersonT.Address1 AS PatientAddress1, PatientPersonT.Address2 AS PatientAddress2, "
			"PatientPersonT.City AS PatientCity, PatientPersonT.State AS PatientState, PatientPersonT.Zip AS PatientZip, "
			"PatientPersonT.HomePhone AS PatientHomePhone, PatientPersonT.BirthDate AS PatientBirthDate, PatientPersonT.Gender AS PatientGender, "
			"PrefixT.Prefix AS PatientPrefix, PatientPersonT.Title AS PatientTitle, "
			"PrimaryInsuranceCoT.Name AS PatientPrimaryInsuranceCoName, "
			"SecondaryInsuranceCoT.Name AS PatientSecondaryInsuranceCoName "
			"FROM PatientsT "
			"INNER JOIN PersonT PatientPersonT ON PatientsT.PersonID = PatientPersonT.ID "
			"LEFT JOIN PrefixT ON PatientPersonT.PrefixID = PrefixT.ID "
			"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 1) AS PrimaryInsuredPartyT ON PatientsT.PersonID = PrimaryInsuredPartyT.PatientID "
			"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 2) AS SecondaryInsuredPartyT ON PatientsT.PersonID = SecondaryInsuredPartyT.PatientID "
			"LEFT JOIN InsuranceCoT PrimaryInsuranceCoT ON PrimaryInsuredPartyT.InsuranceCoID = PrimaryInsuranceCoT.PersonID "
			"LEFT JOIN InsuranceCoT SecondaryInsuranceCoT ON SecondaryInsuredPartyT.InsuranceCoID = SecondaryInsuranceCoT.PersonID "
			"WHERE PatientsT.PersonID = {INT} "
			""
			// (j.jones 2010-07-29 12:12) - PLID 39880 - providers & users now have overrides for each name field
			"SELECT "
			"CASE WHEN ProvidersT.NewCropLastOver <> '' THEN ProvidersT.NewCropLastOver ELSE ProviderPersonT.Last END AS ProviderLast, "
			"CASE WHEN ProvidersT.NewCropFirstOver <> '' THEN ProvidersT.NewCropFirstOver ELSE ProviderPersonT.First END AS ProviderFirst, "
			"CASE WHEN ProvidersT.NewCropMiddleOver <> '' THEN ProvidersT.NewCropMiddleOver ELSE ProviderPersonT.Middle END AS ProviderMiddle, "
			"CASE WHEN ProvidersT.NewCropPrefixOver <> '' THEN ProvidersT.NewCropPrefixOver ELSE PrefixT.Prefix END AS ProviderPrefix, "
			//we need to return both the normal suffix and the override
			"ProvidersT.NewCropSuffixOver, ProviderPersonT.Title AS ProviderTitle, "
			"ProvidersT.[DEA Number] AS DEANumber, ProvidersT.UPIN, ProviderPersonT.State AS ProviderState, "
			"ProvidersT.License, ProvidersT.NPI AS ProviderNPI "			
			"FROM ProvidersT "
			"INNER JOIN PersonT ProviderPersonT ON ProvidersT.PersonID = ProviderPersonT.ID "
			"LEFT JOIN PrefixT ON ProviderPersonT.PrefixID = PrefixT.ID "
			"WHERE ProvidersT.PersonID = {INT} "
			""
			// (j.jones 2009-08-25 09:08) - PLID 35203 - added Supervising Provider
			// (j.jones 2010-07-29 12:12) - PLID 39880 - providers & users now have overrides for each name field
			"SELECT "
			"CASE WHEN ProvidersT.NewCropLastOver <> '' THEN ProvidersT.NewCropLastOver ELSE ProviderPersonT.Last END AS ProviderLast, "
			"CASE WHEN ProvidersT.NewCropFirstOver <> '' THEN ProvidersT.NewCropFirstOver ELSE ProviderPersonT.First END AS ProviderFirst, "
			"CASE WHEN ProvidersT.NewCropMiddleOver <> '' THEN ProvidersT.NewCropMiddleOver ELSE ProviderPersonT.Middle END AS ProviderMiddle, "
			"CASE WHEN ProvidersT.NewCropPrefixOver <> '' THEN ProvidersT.NewCropPrefixOver ELSE PrefixT.Prefix END AS ProviderPrefix, "
			//we need to return both the normal suffix and the override
			"ProvidersT.NewCropSuffixOver, ProviderPersonT.Title AS ProviderTitle, "
			"ProvidersT.[DEA Number] AS DEANumber, ProvidersT.UPIN, ProviderPersonT.State AS ProviderState, "
			"ProvidersT.License, ProvidersT.NPI AS ProviderNPI "			
			"FROM ProvidersT "
			"INNER JOIN PersonT ProviderPersonT ON ProvidersT.PersonID = ProviderPersonT.ID "
			"LEFT JOIN PrefixT ON ProviderPersonT.PrefixID = PrefixT.ID "
			"WHERE ProvidersT.PersonID = {INT} "
			""
			"SELECT "
			"LocationsT.Name AS LocationName, LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2, "
			"LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationZip, "
			"LocationsT.Phone AS LocationPhone, LocationsT.Fax AS LocationFax "			
			"FROM LocationsT "
			"WHERE LocationsT.ID = {INT} "
			""
			// (j.jones 2009-06-09 09:14) - PLID 34535 - support the 20 code limit, ordering by EMR date, treating default
			// diag codes as one year from today's date (no reasonable office would have EMR dates a year the future, but if they did, so be it)
			// The sort order is not important here, we're just trying to include the 20 most pertinent diagnoses.
			"SELECT TOP 20 CodeNumber, CodeDesc "
			"FROM DiagCodes INNER JOIN ("
			"	SELECT DefaultDiagID1 AS ID, DateAdd(year, 1, GetDate()) AS Date, 0 AS OrderIndex "
			"	FROM PatientsT WHERE PersonID = {INT} AND DefaultDiagID1 Is Not Null "
			"	UNION "
			"	SELECT DefaultDiagID2 AS ID, DateAdd(year, 1, GetDate()) AS Date, 0 AS OrderIndex "
			"	FROM PatientsT WHERE PersonID = {INT} AND DefaultDiagID2 Is Not Null "
			"	UNION "
			"	SELECT DefaultDiagID3 AS ID, DateAdd(year, 1, GetDate()) AS Date, 0 AS OrderIndex "
			"	FROM PatientsT WHERE PersonID = {INT} AND DefaultDiagID3 Is Not Null "
			"	UNION "
			"	SELECT DefaultDiagID4 AS ID, DateAdd(year, 1, GetDate()) AS Date, 0 AS OrderIndex "
			"	FROM PatientsT WHERE PersonID = {INT} AND DefaultDiagID4 Is Not Null "
			"	UNION "
			"	SELECT EMRDiagCodesT.DiagCodeID AS ID, EMRMasterT.Date, EMRDiagCodesT.OrderIndex FROM EMRDiagCodesT "
			"	INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
			"	WHERE EMRMasterT.PatientID = {INT} AND EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
			") AS PatientDiagsQ ON DiagCodes.ID = PatientDiagsQ.ID "
			"GROUP BY CodeNumber, CodeDesc "
			"ORDER BY Max(Date) DESC, Min(OrderIndex) ASC "
			""
			// (j.jones 2010-07-29 12:12) - PLID 39880 - providers & users now have overrides for each name field
			"SELECT "
			"CASE WHEN UsersT.NewCropLastOver <> '' THEN UsersT.NewCropLastOver ELSE PersonT.Last END AS Last, "
			"CASE WHEN UsersT.NewCropFirstOver <> '' THEN UsersT.NewCropFirstOver ELSE PersonT.First END AS First, "
			"CASE WHEN UsersT.NewCropMiddleOver <> '' THEN UsersT.NewCropMiddleOver ELSE PersonT.Middle END AS Middle, "
			"CASE WHEN UsersT.NewCropPrefixOver <> '' THEN UsersT.NewCropPrefixOver ELSE PrefixT.Prefix END AS Prefix, "
			//we need to return both the normal suffix and the override
			"UsersT.NewCropSuffixOver, PersonT.Title "
			"FROM PersonT "
			"INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
			"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
			"WHERE PersonT.ID = {INT} "
			, nPatientID, nLinkedProviderID, nSupervisingProviderID, nLocationID,
			nPatientID, nPatientID, nPatientID, nPatientID, nPatientID, GetCurrentUserID());

		//Patient
		long nUserDefinedID = -1;
		CString strPatientLast, strPatientFirst, strPatientMiddle, strPatientHomePhone;
		CString strPatientPrefix, strPatientSuffix;
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		COleDateTime dtPatientBirthDate = dtInvalid;
		long nPatientGender = 0;
		CString strPatientAddress1, strPatientAddress2, strPatientCity, strPatientState, strPatientZip, strPatientCountry;
		CString strPatientPrimaryInsuranceCoName, strPatientSecondaryInsuranceCoName;
		
		if(rs->eof) {
			ThrowNxException("Could not find a patient ID of %li!", nPatientID);
		}
		else {

			nUserDefinedID = AdoFldLong(rs, "UserDefinedID", -1);
			strPatientLast = AdoFldString(rs, "PatientLast", "");
			strPatientFirst = AdoFldString(rs, "PatientFirst", "");
			strPatientMiddle = AdoFldString(rs, "PatientMiddle", "");
			// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix (Title)
			strPatientPrefix = AdoFldString(rs, "PatientPrefix", "");

			// (j.jones 2010-07-29 12:07) - PLID 39880 - changed the suffix logic, we now check it here,
			// and apply on load (and yes, this applies to patients despite the name of the preference)
			if(bHideSuffix) {
				strPatientSuffix = "";
			}
			else {
				strPatientSuffix = AdoFldString(rs, "PatientTitle", "");
			}

			strPatientHomePhone = AdoFldString(rs, "PatientHomePhone", "");
			dtPatientBirthDate = AdoFldDateTime(rs, "PatientBirthDate", dtInvalid);
			nPatientGender = (long)AdoFldByte(rs, "PatientGender", 0);
			strPatientAddress1 = AdoFldString(rs, "PatientAddress1", "");
			strPatientAddress2 = AdoFldString(rs, "PatientAddress2", "");
			strPatientCity = AdoFldString(rs, "PatientCity", "");
			strPatientState = AdoFldString(rs, "PatientState", "");
			strPatientZip = AdoFldString(rs, "PatientZip", "");
			strPatientPrimaryInsuranceCoName = AdoFldString(rs, "PatientPrimaryInsuranceCoName", "");
			strPatientSecondaryInsuranceCoName = AdoFldString(rs, "PatientSecondaryInsuranceCoName", "");

			//the Country code cannot be blank, and NewCrop only allows values of US and CA,
			//and yet they don't allow Canadian provinces or zipcodes, so the only option
			//truly allowed is US
			strPatientCountry = "US";
		}

		rs = rs->NextRecordset(NULL);

		//Linked Provider
		CString strLinkedProviderLast, strLinkedProviderFirst, strLinkedProviderMiddle, strLinkedProviderDEANumber, strLinkedProviderUPIN,
			strLinkedProviderState, strLinkedProviderLicenseNumber, strLinkedProviderNPI, strLinkedProviderPrefix, strLinkedProviderSuffix;
		
		// (j.gruber 2009-06-08 09:49) - PLID 34515 - added role
		// (j.jones 2009-08-18 17:13) - PLID 35203 - supported Midlevel Prescriber, which is handled by IsNewCropPrescriberRole()
		if(IsNewCropPrescriberRole(ncuTypeID)) {
			if(rs->eof) {
				ThrowNxException("Could not find a linked provider ID of %li!", nLinkedProviderID);
			}
			else {
				strLinkedProviderLast = AdoFldString(rs, "ProviderLast", "");
				strLinkedProviderFirst = AdoFldString(rs, "ProviderFirst", "");
				strLinkedProviderMiddle = AdoFldString(rs, "ProviderMiddle", "");				
				// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix (Title)
				strLinkedProviderPrefix = AdoFldString(rs, "ProviderPrefix", "");

				// (j.jones 2010-07-29 12:07) - PLID 39880 - changed the suffix logic, we now check it here,
				// and apply on load, but if there is an override suffix we use that
				if(bHideSuffix) {
					strLinkedProviderSuffix = "";
				}
				else {
					strLinkedProviderSuffix = AdoFldString(rs, "ProviderTitle", "");				
				}
				//if there is an override, always use it
				CString strOverrideSuffix = AdoFldString(rs, "NewCropSuffixOver", "");
				if(!strOverrideSuffix.IsEmpty()) {
					strLinkedProviderSuffix = strOverrideSuffix;
				}

				strLinkedProviderDEANumber = AdoFldString(rs, "DEANumber", "");
				strLinkedProviderUPIN = AdoFldString(rs, "UPIN", "");
				strLinkedProviderState = AdoFldString(rs, "ProviderState", "");
				strLinkedProviderLicenseNumber = AdoFldString(rs, "License", "");
				strLinkedProviderNPI = AdoFldString(rs, "ProviderNPI", "");
			}
		}

		rs = rs->NextRecordset(NULL);

		// (j.jones 2009-08-25 09:08) - PLID 35203 - added Supervising Provider

		//Supervising Provider
		CString strSupervisingProviderLast, strSupervisingProviderFirst, strSupervisingProviderMiddle,
			strSupervisingProviderDEANumber, strSupervisingProviderUPIN,
			strSupervisingProviderState, strSupervisingProviderLicenseNumber, strSupervisingProviderNPI,
			strSupervisingProviderPrefix, strSupervisingProviderSuffix;

		if(IsNewCropSupervisedRole(ncuTypeID)) {
			if(rs->eof) {
				ThrowNxException("Could not find a supervising provider ID of %li!", nSupervisingProviderID);
			}
			else {
				strSupervisingProviderLast = AdoFldString(rs, "ProviderLast", "");
				strSupervisingProviderFirst = AdoFldString(rs, "ProviderFirst", "");
				strSupervisingProviderMiddle = AdoFldString(rs, "ProviderMiddle", "");
				// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix (Title)
				strSupervisingProviderPrefix = AdoFldString(rs, "ProviderPrefix", "");

				// (j.jones 2010-07-29 12:07) - PLID 39880 - changed the suffix logic, we now check it here,
				// and apply on load, but if there is an override suffix we use that
				if(bHideSuffix) {
					strSupervisingProviderSuffix = "";
				}
				else {					
					strSupervisingProviderSuffix = AdoFldString(rs, "ProviderTitle", "");
				}
				//if there is an override, always use it
				CString strOverrideSuffix = AdoFldString(rs, "NewCropSuffixOver", "");
				if(!strOverrideSuffix.IsEmpty()) {
					strSupervisingProviderSuffix = strOverrideSuffix;
				}

				strSupervisingProviderDEANumber = AdoFldString(rs, "DEANumber", "");
				strSupervisingProviderUPIN = AdoFldString(rs, "UPIN", "");
				strSupervisingProviderState = AdoFldString(rs, "ProviderState", "");
				strSupervisingProviderLicenseNumber = AdoFldString(rs, "License", "");
				strSupervisingProviderNPI = AdoFldString(rs, "ProviderNPI", "");
			}
		}

		rs = rs->NextRecordset(NULL);

		//Location
		CString strLocationName, strLocationAddress1, strLocationAddress2,
			strLocationCity, strLocationState, strLocationZip, strLocationPhone, strLocationFax, strLocationCountry;

		if(rs->eof) {
			ThrowNxException("Could not find a location ID of %li!", nLocationID);
		}
		else {

			strLocationName = AdoFldString(rs, "LocationName", "");
			strLocationAddress1 = AdoFldString(rs, "LocationAddress1", "");
			strLocationAddress2 = AdoFldString(rs, "LocationAddress2", "");
			strLocationCity = AdoFldString(rs, "LocationCity", "");
			strLocationState = AdoFldString(rs, "LocationState", "");
			strLocationZip = AdoFldString(rs, "LocationZip", "");
			strLocationPhone = AdoFldString(rs, "LocationPhone", "");
			strLocationFax = AdoFldString(rs, "LocationFax", "");

			//the Country code cannot be blank, and NewCrop only allows values of US and CA,
			//and yet they don't allow Canadian provinces or zipcodes, so the only option
			//truly allowed is US
			strLocationCountry = "US";
		}

		// (j.gruber 2009-05-12 12:50) - PLID 29906 - added diagnosis codes
		rs = rs->NextRecordset(NULL);

		CString strDiagCode, strDiagCodeDesc, strDiagCodeXML;
		CString strAllDiagCodesXML;
		while (! rs->eof) {

			strDiagCode = AdoFldString(rs, "CodeNumber", "");
			strDiagCodeDesc = AdoFldString(rs, "CodeDesc", "");

			if (GenerateNewCropDiagCodeXML(strDiagCodeXML, strDiagCode, strDiagCodeDesc)) {
				
				strAllDiagCodesXML += strDiagCodeXML;
			}
			else {
				// (j.jones 2011-06-17 12:53) - PLID 38517 - return FALSE if this failed
				return FALSE;
			}		

			rs->MoveNext();
		}

		// (j.gruber 2009-06-08 09:56) - PLID 34515 - user info for staff
		rs = rs->NextRecordset(NULL);

		CString strUserFirst, strUserLast, strUserMiddle, strUserID,
			strUserPrefix, strUserSuffix;

		if (ncuTypeID == ncutStaff_Nurse) {
			if (! rs->eof) {
				strUserID = AsString(GetCurrentUserID());
				strUserFirst = AdoFldString(rs, "First", "");
				strUserMiddle = AdoFldString(rs, "Middle", "");
				strUserLast = AdoFldString(rs, "Last", "");
				// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix (Title)
				strUserPrefix = AdoFldString(rs, "Prefix", "");

				// (j.jones 2010-07-29 12:07) - PLID 39880 - changed the suffix logic, we now check it here,
				// and apply on load, but if there is an override suffix we use that
				if(bHideSuffix) {
					strUserSuffix = "";
				}
				else {					
					strUserSuffix = AdoFldString(rs, "Title", "");
				}
				//if there is an override, always use it
				CString strOverrideSuffix = AdoFldString(rs, "NewCropSuffixOver", "");
				if(!strOverrideSuffix.IsEmpty()) {
					strUserSuffix = strOverrideSuffix;
				}
			}
		}		

		rs->Close();

		//this setting will default to whatever location you're currently using
		long nAccountID = GetRemotePropertyInt("NewCropAccountLocationID", GetCurrentLocationID(), 0, "<None>", true);

		CString strAccountName, strAccountAddress1, strAccountAddress2, strAccountCity,
		strAccountState, strAccountZip, strAccountCountry, strAccountPhone, strAccountFax;

		//in a lot of cases, the AccountID will be the LocationID, so just copy that information,
		//otherwise, we will need to run one extra recordset		
		if(nAccountID == nLocationID) {
			strAccountName = strLocationName;
			strAccountAddress1 = strLocationAddress1;
			strAccountAddress2 = strLocationAddress2;
			strAccountCity = strLocationCity;
			strAccountState = strLocationState;
			strAccountZip = strLocationZip;
			strAccountCountry = strLocationCountry;
			strAccountPhone = strLocationPhone;
			strAccountFax = strLocationFax;
		}
		else {
			rs = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, Phone, Fax "
				"FROM LocationsT WHERE ID = {INT}", nAccountID);
			if(rs->eof) {
				ThrowNxException("Could not find an account ID of %li!", nAccountID);
			}
			else {
				strAccountName = AdoFldString(rs, "Name", "");
				strAccountAddress1 = AdoFldString(rs, "Address1", "");
				strAccountAddress2 = AdoFldString(rs, "Address2", "");
				strAccountCity = AdoFldString(rs, "City", "");
				strAccountState = AdoFldString(rs, "State", "");
				strAccountZip = AdoFldString(rs, "Zip", "");
				strAccountCountry = "US";
				strAccountPhone = AdoFldString(rs, "Phone", "");
				strAccountFax = AdoFldString(rs, "Fax", "");
			}
			rs->Close();
		}

		CString strPatientRequest;
		// (j.gruber 2009-06-08 09:59) - PLID 34515 - added user Infomation and role
		// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
		// (j.jones 2010-06-03 15:04) - PLID 38994 - supported sending the insurance company names
		if(!GenerateLoginToPatientAccountRequest(strPatientRequest,
			nPatientID, nUserDefinedID, strPatientLast, strPatientFirst, strPatientMiddle,
			strPatientPrefix, strPatientSuffix,
			strPatientHomePhone, dtPatientBirthDate, nPatientGender, strPatientAddress1, strPatientAddress2,
			strPatientCity, strPatientState, strPatientZip, strPatientCountry,			
			strPatientPrimaryInsuranceCoName, strPatientSecondaryInsuranceCoName,
			nLinkedProviderID, strLinkedProviderLast, strLinkedProviderFirst, strLinkedProviderMiddle,
			strLinkedProviderPrefix, strLinkedProviderSuffix,
			strLinkedProviderDEANumber, strLinkedProviderUPIN, strLinkedProviderState, strLinkedProviderLicenseNumber, strLinkedProviderNPI,
			nSupervisingProviderID, strSupervisingProviderLast, strSupervisingProviderFirst, strSupervisingProviderMiddle,
			strSupervisingProviderPrefix, strSupervisingProviderSuffix,
			strSupervisingProviderDEANumber, strSupervisingProviderUPIN, strSupervisingProviderState, strSupervisingProviderLicenseNumber, strSupervisingProviderNPI,
			strAccountName, strAccountAddress1, strAccountAddress2, strAccountCity,
			strAccountState, strAccountZip, strAccountCountry, strAccountPhone, strAccountFax,
			nLocationID, strLocationName, strLocationAddress1, strLocationAddress2,
			strLocationCity, strLocationState, strLocationZip, strLocationCountry,
			strLocationPhone, strLocationFax, strAllDiagCodesXML, 
			ncuTypeID, strUserID, strUserFirst, strUserMiddle, strUserLast,
			strUserPrefix, strUserSuffix,
			bAddNxScriptFooter, ncrptPage)) {
			return FALSE;
		}

		strXmlDocument = strPatientRequest;

		return TRUE;

	}NxCatchAll("Error in GenerateNewCropPatientLoginXML");

	return FALSE;
}
*/

// (j.gruber 2009-05-12 13:01) - PLID 29906 - Generate DiagCode XML
// (j.jones 2013-01-03 16:24) - PLID 49624 - this function is now obsolete, its logic is now in the API
/*
BOOL GenerateNewCropDiagCodeXML(OUT CString &strXMLDiagCode, CString strDiagCode, CString strDiagCodeDesc) {

	try {
		strXMLDiagCode = "";

		CString strCodeXML, strTemp;

		strTemp = GetXMLElementValuePair_NewCrop("diagnosisID", strDiagCode);
		strCodeXML += strTemp;

		strTemp = GetXMLElementValuePair_NewCrop("diagnosisType", "ICD9");
		strCodeXML += strTemp;

		// (j.jones 2011-06-17 12:53) - PLID 38517 - fail if the diagnosis code is too long
		if(strDiagCodeDesc.GetLength() > 80) {
			CString strWarning;
			strWarning.Format("This patient has an invalid diagnosis code of:\n\n"
				"%s: %s\n\n"
				"This code's description is too long to be submitted via E-Prescribing.\n\n"
				"Please edit this diagnosis code's description to be no more than 80 characters.", strDiagCode, strDiagCodeDesc);
			AfxMessageBox(strWarning);
			return FALSE;
		}

		strTemp = GetXMLElementValuePair_NewCrop("diagnosisName", strDiagCodeDesc);
		strCodeXML += strTemp;
		

		strXMLDiagCode = GetXMLElementValuePair_NewCrop("PatientDiagnosis", strCodeXML, TRUE);

		return TRUE;
	}NxCatchAll("Error in GenerateNewCropDiagCodeXML");

	return FALSE;
}
*/

// (j.jones 2009-03-04 17:00) - PLID 33345 - root function for creating NewCrop XML
// from a given Prescription ID, to submit as an OutsidePrescription
/* temporarily removed
BOOL GenerateNewCropPrescriptionXML(OUT CString &strXmlDocument, long nPrescriptionID)
{
	try {

		strXmlDocument = "";
		
		if(nPrescriptionID == -1) {
			ThrowNxException("Invalid Prescription ID");
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientMedications.PatientID, PatientsT.UserDefinedID, "
			"PatientPersonT.Last AS PatientLast, PatientPersonT.First AS PatientFirst, PatientPersonT.Middle AS PatientMiddle, "
			"PatientPersonT.Address1 AS PatientAddress1, PatientPersonT.Address2 AS PatientAddress2, "
			"PatientPersonT.City AS PatientCity, PatientPersonT.State AS PatientState, PatientPersonT.Zip AS PatientZip, "
			"PatientPersonT.HomePhone AS PatientHomePhone, PatientPersonT.BirthDate AS PatientBirthDate, PatientPersonT.Gender AS PatientGender, "
			"PharmacyLocationT.Phone AS PharmacyPhone, PharmacyLocationT.Fax AS PharmacyFax, "			
			"EMRDataT.Data AS DrugName, PatientMedications.Quantity, PatientMedications.RefillsAllowed, "
			"PatientMedications.PatientExplanation, PatientMedications.NoteToPharmacist, PatientMedications.AllowSubstitutions, "
			"PatientMedications.Strength, DrugDosageFormsT.Name AS DosageForm, PatientMedications.PrescriptionDate, "
			"ProviderPersonT.ID AS ProviderID, "
			"ProviderPersonT.Last AS ProviderLast, ProviderPersonT.First AS ProviderFirst, ProviderPersonT.Middle AS ProviderMiddle, "
			"ProvidersT.[DEA Number] AS DEANumber, ProvidersT.UPIN, ProviderPersonT.State AS ProviderState, "
			"ProvidersT.License, ProvidersT.NPI AS ProviderNPI, "
			"LocationsT.ID AS LocationID, "
			"LocationsT.Name AS LocationName, LocationsT.Address1 AS LocationAddress1, LocationsT.Address2 AS LocationAddress2, "
			"LocationsT.City AS LocationCity, LocationsT.State AS LocationState, LocationsT.Zip AS LocationZip, "
			"LocationsT.Phone AS LocationPhone, LocationsT.Fax AS LocationFax "
			"FROM PatientMedications "
			"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"INNER JOIN PatientsT ON PatientMedications.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT PatientPersonT ON PatientsT.PersonID = PatientPersonT.ID "
			"LEFT JOIN ProvidersT ON PatientMedications.ProviderID = ProvidersT.PersonID "
			"LEFT JOIN PersonT ProviderPersonT ON ProvidersT.PersonID = ProviderPersonT.ID "
			"LEFT JOIN LocationsT PharmacyLocationT ON PatientMedications.PharmacyID = PharmacyLocationT.ID "
			"LEFT JOIN DrugDosageFormsT ON PatientMedications.DosageFormID = DrugDosageFormsT.ID "
			"LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID "
			"WHERE PatientMedications.ID = {INT} "
			"AND PatientMedications.Deleted = 0",
			nPrescriptionID);

		if(rs->eof) {
			ThrowNxException("Could not find a prescription ID of %li!", nPrescriptionID);
		}
		else {

			long nPatientID = AdoFldLong(rs, "PatientID", -1);
			long nUserDefinedID = AdoFldLong(rs, "UserDefinedID", -1);
			CString strPatientLast = AdoFldString(rs, "PatientLast", "");
			CString strPatientFirst = AdoFldString(rs, "PatientFirst", "");
			CString strPatientMiddle = AdoFldString(rs, "PatientMiddle", "");
			CString strPatientHomePhone = AdoFldString(rs, "PatientHomePhone", "");
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			COleDateTime dtPatientBirthDate = AdoFldDateTime(rs, "PatientBirthDate", dtInvalid);
			long nPatientGender = (long)AdoFldByte(rs, "PatientGender", 0);
			CString strPatientAddress1 = AdoFldString(rs, "PatientAddress1", "");
			CString strPatientAddress2 = AdoFldString(rs, "PatientAddress2", "");
			CString strPatientCity = AdoFldString(rs, "PatientCity", "");
			CString strPatientState = AdoFldString(rs, "PatientState", "");
			CString strPatientZip = AdoFldString(rs, "PatientZip", "");

			//the Country code cannot be blank, and NewCrop only allows values of US and CA,
			//and yet they don't allow Canadian provinces or zipcodes, so the only option
			//truly allowed is US
			CString strPatientCountry = "US";

			//TODO: when we actually implement this, we will need to support pharmacy IDs")
			CString strNewCropPharmacyID = "9988800";

			CString strPharmacyPhone = AdoFldString(rs, "PharmacyPhone", "");
			CString strPharmacyFax = AdoFldString(rs, "PharmacyFax", "");
			CString strDrugName = AdoFldString(rs, "DrugName", "");
			CString strQuantity = AdoFldString(rs, "Quantity", "");
			CString strRefillsAllowed = AdoFldString(rs, "RefillsAllowed", "");
			CString strPatientExplanation = AdoFldString(rs, "PatientExplanation", "");
			CString strPharmacistNote = AdoFldString(rs, "NoteToPharmacist", "");
			BOOL bAllowSubstitutions = AdoFldBool(rs, "AllowSubstitutions", FALSE);
			CString strDrugStrengthWithUOM = AdoFldString(rs, "Strength", "");
			CString strDrugDosageForm = AdoFldString(rs, "DosageForm", "");
			COleDateTime dtPrescriptionDate = AdoFldDateTime(rs, "PrescriptionDate", dtInvalid);

			//it doesn't appear we have a field for this yet
			CString strDrugRoute = "";

			long nProviderID = AdoFldLong(rs, "ProviderID", -1);
			CString strProviderLast = AdoFldString(rs, "ProviderLast", "");
			CString strProviderFirst = AdoFldString(rs, "ProviderFirst", "");
			CString strProviderMiddle = AdoFldString(rs, "ProviderMiddle", "");
			CString strDEANumber = AdoFldString(rs, "DEANumber", "");
			CString strUPIN = AdoFldString(rs, "UPIN", "");
			CString strProviderState = AdoFldString(rs, "ProviderState", "");
			CString strLicenseNumber = AdoFldString(rs, "License", "");
			CString strProviderNPI = AdoFldString(rs, "ProviderNPI", "");

			long nLocationID = AdoFldLong(rs, "LocationID", -1);
			CString strLocationName = AdoFldString(rs, "LocationName", "");
			CString strLocationAddress1 = AdoFldString(rs, "LocationAddress1", "");
			CString strLocationAddress2 = AdoFldString(rs, "LocationAddress2", "");
			CString strLocationCity = AdoFldString(rs, "LocationCity", "");
			CString strLocationState = AdoFldString(rs, "LocationState", "");
			CString strLocationZip = AdoFldString(rs, "LocationZip", "");
			CString strLocationPhone = AdoFldString(rs, "LocationPhone", "");
			CString strLocationFax = AdoFldString(rs, "LocationFax", "");

			//the Country code cannot be blank, and NewCrop only allows values of US and CA,
			//and yet they don't allow Canadian provinces or zipcodes, so the only option
			//truly allowed is US
			CString strLocationCountry = "US";

			//this setting will default to whatever location you're currently using
			long nAccountID = GetRemotePropertyInt("NewCropAccountLocationID", GetCurrentLocationID(), 0, "<None>", true);

			CString strAccountName, strAccountAddress1, strAccountAddress2, strAccountCity,
			strAccountState, strAccountZip, strAccountCountry, strAccountPhone, strAccountFax;

			//in a lot of cases, the AccountID will be the LocationID, so just copy that information,
			//otherwise, we will need to run one extra recordset		
			if(nAccountID == nLocationID) {
				strAccountName = strLocationName;
				strAccountAddress1 = strLocationAddress1;
				strAccountAddress2 = strLocationAddress2;
				strAccountCity = strLocationCity;
				strAccountState = strLocationState;
				strAccountZip = strLocationZip;
				strAccountCountry = strLocationCountry;
				strAccountPhone = strLocationPhone;
				strAccountFax = strLocationFax;
			}
			else {
				rs = CreateParamRecordset("SELECT Name, Address1, Address2, City, State, Zip, Phone, Fax "
					"FROM LocationsT WHERE ID = {INT}", nAccountID);
				if(rs->eof) {
					ThrowNxException("Could not find an account ID of %li!", nAccountID);
				}
				else {
					strAccountName = AdoFldString(rs, "Name", "");
					strAccountAddress1 = AdoFldString(rs, "Address1", "");
					strAccountAddress2 = AdoFldString(rs, "Address2", "");
					strAccountCity = AdoFldString(rs, "City", "");
					strAccountState = AdoFldString(rs, "State", "");
					strAccountZip = AdoFldString(rs, "Zip", "");
					strAccountCountry = "US";
					strAccountPhone = AdoFldString(rs, "Phone", "");
					strAccountFax = AdoFldString(rs, "Fax", "");
				}
				rs->Close();
			}

			CString strPrescriptionRequest;
			if(!GenerateFullOutsidePrescriptionRequest(strPrescriptionRequest,
				nPatientID, nUserDefinedID, strPatientLast, strPatientFirst, strPatientMiddle,
				strPatientHomePhone, dtPatientBirthDate, nPatientGender, strPatientAddress1, strPatientAddress2,
				strPatientCity, strPatientState, strPatientZip, strPatientCountry,
				nPrescriptionID, strNewCropPharmacyID, strPharmacyPhone,
				strPharmacyFax, dtPrescriptionDate, strDoctorFullName,
				strDrugName, strQuantity, strRefillsAllowed,
				strPatientExplanation, strPharmacistNote, bAllowSubstitutions,
				strDrugStrengthWithUOM, strDrugDosageForm, strDrugRoute,
				nProviderID, strProviderLast, strProviderFirst, strProviderMiddle,
				strDEANumber, strUPIN, strProviderState, strLicenseNumber, strProviderNPI,
				strAccountName, strAccountAddress1, strAccountAddress2, strAccountCity,
				strAccountState, strAccountZip, strAccountCountry, strAccountPhone, strAccountFax,
				nLocationID, strLocationName, strLocationAddress1, strLocationAddress2,
				strLocationCity, strLocationState, strLocationZip, strLocationCountry,
				strLocationPhone, strLocationFax)) {
				return FALSE;
			}

			strXmlDocument = strPrescriptionRequest;
		}

		return TRUE;

	}NxCatchAll("Error in GenerateNewCropPrescriptionXML");

	return FALSE;
}
*/

// (j.jones 2009-02-18 17:16) - PLID 33163 - web-browser requests

// (j.jones 2009-02-27 09:53) - PLID 33163 - this function builds the full XML necessary to login
// to a paient's account, with a destination of the ComposeRx page
// (j.gruber 2009-05-12 13:08) - PLID 29906 - added diagnosis codes
// (j.gruber 2009-05-15 13:58) - PLID 28541 - added ability not to add nxscript footer
// (j.gruber 2009-05-15 17:33) - PLID 28541 - added ability to change route page
// (j.gruber 2009-06-08 10:01) - PLID 34515 - added user information and role
// (j.jones 2009-08-25 09:03) - PLID 35203 - split Linked Provider and Supervising Provider information
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
// (j.jones 2010-06-03 15:04) - PLID 38994 - supported sending the insurance company names
// (j.jones 2013-01-03 16:24) - PLID 49624 - this function is now obsolete, its logic is now in the API
/*
BOOL GenerateLoginToPatientAccountRequest(OUT CString &strXmlDocument,
						long nPatientID, long nUserDefinedID,
						CString strPatientLast, CString strPatientFirst, CString strPatientMiddle,
						CString strPatientPrefix, CString strPatientSuffix,
						CString strPatientHomePhone, COleDateTime dtPatientBirthDate, long nPatientGender,
						CString strPatientAddress1, CString strPatientAddress2,
						CString strPatientCity, CString strPatientState, CString strPatientZip,
						CString strPatientCountry,
						CString strPatientPrimaryInsuranceCoName, CString strPatientSecondaryInsuranceCoName,
						long nLinkedProviderID, CString strLinkedProviderLast, CString strLinkedProviderFirst, CString strLinkedProviderMiddle,
						CString strLinkedProviderPrefix, CString strLinkedProviderSuffix,
						CString strLinkedProviderDEANumber, CString strLinkedProviderUPIN, CString strLinkedProviderState, CString strLinkedProviderLicenseNumber,
						CString strLinkedProviderNPI,
						long nSupervisingProviderID, CString strSupervisingProviderLast, CString strSupervisingProviderFirst, CString strSupervisingProviderMiddle,
						CString strSupervisingProviderPrefix, CString strSupervisingProviderSuffix,
						CString strSupervisingProviderDEANumber, CString strSupervisingProviderUPIN, CString strSupervisingProviderState, CString strSupervisingProviderLicenseNumber,
						CString strSupervisingProviderNPI,
						CString strAccountName,
						CString strAccountAddress1, CString strAccountAddress2,
						CString strAccountCity, CString strAccountState, CString strAccountZip,
						CString strAccountCountry, CString strAccountPhone, CString strAccountFax,
						long nLocationID, CString strLocationName,
						CString strLocationAddress1, CString strLocationAddress2,
						CString strLocationCity, CString strLocationState, CString strLocationZip,
						CString strLocationCountry, CString strLocationPhone, CString strLocationFax, CString strAllDiagCodesXML, 
						NewCropUserTypes ncuTypeID, 
						CString strUserID, CString strUserFirst, CString strUserMiddle, CString strUserLast,
						CString strUserPrefix, CString strUserSuffix,
						BOOL bAddNxScriptFooter,
						NewCropRequestedPageType ncrptPage)
{
	CString strXml;
	
	//get our header
	strXml = "";
	GenerateNCScriptHeader(strXml);
	strXmlDocument = strXml;

	//get our Credentials
	strXml = "";
	if(!GenerateCredentials(strXml)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our UserRole
	strXml = "";
	if(!GenerateUserRole(strXml, ncuTypeID)) {
		return FALSE;
	}
	strXmlDocument += strXml;
  
	//get our Destination

	//"compose" = goes to the compose page for this patient
	// (j.gruber 2009-05-15 17:35) - PLID 28541 - changed to the one specified in call
	strXml = "";
	if(!GenerateDestination(strXml, ncrptPage)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Account
	strXml = "";
	if(!GenerateAccount(strXml, strAccountName,
						strAccountAddress1, strAccountAddress2,
						strAccountCity, strAccountState, strAccountZip,
						strAccountCountry, strAccountPhone, strAccountFax)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Location
	strXml = "";
	if(!GenerateLocation(strXml, nLocationID, strLocationName,
						strLocationAddress1, strLocationAddress2,
						strLocationCity, strLocationState, strLocationZip,
						strLocationCountry, strLocationPhone, strLocationFax)) {
		return FALSE;
	}
	strXmlDocument += "\r\n" + strXml;

	// (j.jones 2009-08-25 09:15) - PLID 35203 - supported Supervising Provider, which is essentially an additional
	// Licensed Prescriber segment sent prior to a "supervised" role (Midlevel Prescribers and Staff)
	// The Licensed Prescriber has to be sent BEFORE the other roles.
	if(IsNewCropSupervisedRole(ncuTypeID)) {
		strXml = "";
		// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
		if(!GeneratePrescriber(strXml, ncutLicensedPrescriber, TRUE, nSupervisingProviderID,
				strSupervisingProviderLast, strSupervisingProviderFirst, strSupervisingProviderMiddle,
				strSupervisingProviderPrefix, strSupervisingProviderSuffix,
				strSupervisingProviderDEANumber, strSupervisingProviderUPIN, strSupervisingProviderState, strSupervisingProviderLicenseNumber, strSupervisingProviderNPI)) {
			return FALSE;
		}
		strXmlDocument += "\r\n" + strXml;
	}

	//get our Licensed Prescriber
	// (j.gruber 2009-06-08 10:02) - PLID 34515 - if we are a license prescriber, otherwise, do the staff
	// (j.jones 2009-08-18 16:23) - PLID 35203 - supported Midlevel Prescriber, which uses the same export as the Licensed Prescriber
	if(IsNewCropPrescriberRole(ncuTypeID)) {
		strXml = "";
		// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
		if(!GeneratePrescriber(strXml, ncuTypeID, FALSE, nLinkedProviderID,
				strLinkedProviderLast, strLinkedProviderFirst, strLinkedProviderMiddle,
				strLinkedProviderPrefix, strLinkedProviderSuffix,
				strLinkedProviderDEANumber, strLinkedProviderUPIN, strLinkedProviderState, strLinkedProviderLicenseNumber, strLinkedProviderNPI)) {
			return FALSE;
		}
		strXmlDocument += "\r\n" + strXml;
	}
	else if (ncuTypeID == ncutStaff_Nurse) {
		strXml = "";
		// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
		if(!GenerateStaff_Nurse(strXml, strUserID, strUserFirst, strUserMiddle, strUserLast,
			strUserPrefix, strUserSuffix)) {
			return FALSE;
		}
		strXmlDocument += "\r\n" + strXml;
	}
	else {
		//this should never happen
		ASSERT(FALSE);
		return FALSE;
	}

	//get our Patient
	strXml = "";
	// (j.gruber 2009-05-12 13:09) - PLID 29906 - added diagnosis codes
	// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
	if(!GeneratePatient(strXml, nPatientID, nUserDefinedID, strPatientLast, strPatientFirst, strPatientMiddle,
				strPatientPrefix, strPatientSuffix,
				strPatientHomePhone, dtPatientBirthDate, nPatientGender, strPatientAddress1, strPatientAddress2,
				strPatientCity, strPatientState, strPatientZip, strPatientCountry, strAllDiagCodesXML,
				strPatientPrimaryInsuranceCoName, strPatientSecondaryInsuranceCoName)) {
		return FALSE;
	}
	strXmlDocument += "\r\n" + strXml;

	//get our footer
	if (bAddNxScriptFooter) {
		GenerateNCScriptFooter(strXml);
		strXmlDocument += "\r\n" + strXml;
	}

	return TRUE;
}
*/

// (j.jones 2009-03-04 17:00) - PLID 33345 - this function builds the full XML necessary to login
// to a paient's account, and submit a given prescription to be edited on the ComposeRx page
/* temporarily removed
BOOL GenerateFullOutsidePrescriptionRequest(OUT CString &strXmlDocument,
						long nPatientID, long nUserDefinedID,
						CString strPatientLast, CString strPatientFirst, CString strPatientMiddle,
						CString strPatientHomePhone, COleDateTime dtPatientBirthDate, long nPatientGender,
						CString strPatientAddress1, CString strPatientAddress2,
						CString strPatientCity, CString strPatientState, CString strPatientZip,
						CString strPatientCountry,
						long nPrescriptionID, CString strNewCropPharmacyID,
						CString strPharmacyPhone, CString strPharmacyFax,
						COleDateTime dtPrescriptionDate, CString strDoctorFullName,
						CString strDrugName, CString strQuantity,
						CString strRefillsAllowed, CString strPatientExplanation,
						CString strPharmacistNote, BOOL bAllowSubstitutions,
						CString strDrugStrengthWithUOM, CString strDrugDosageForm, CString strDrugRoute,
						long nProviderID, CString strProviderLast, CString strProviderFirst, CString strProviderMiddle,
						CString strDEANumber, CString strUPIN, CString strProviderState, CString strLicenseNumber,
						CString strProviderNPI,
						CString strAccountName,
						CString strAccountAddress1, CString strAccountAddress2,
						CString strAccountCity, CString strAccountState, CString strAccountZip,
						CString strAccountCountry, CString strAccountPhone, CString strAccountFax,
						long nLocationID, CString strLocationName,
						CString strLocationAddress1, CString strLocationAddress2,
						CString strLocationCity, CString strLocationState, CString strLocationZip,
						CString strLocationCountry, CString strLocationPhone, CString strLocationFax)
{
	CString strXml;
	
	//get our header
	strXml = "";
	GenerateNCScriptHeader(strXml);
	strXmlDocument = strXml;

	//get our Credentials
	strXml = "";
	if(!GenerateCredentials(strXml)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our UserRole
	strXml = "";
	if(!GenerateUserRole(strXml)) {
		return FALSE;
	}
	strXmlDocument += strXml;
  
	//get our Destination

	//"compose" = submits the prescription in NewCrop's Current Meds, and then goes to the compose page
	//"route" submits directly to the RouteRx page, where you can then transfer
	//"ws-rx-send" gives an error that it is not compatible with OutsidePrescription
	strXml = "";
	if(!GenerateDestination(strXml, ncrptCompose)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Account
	strXml = "";
	if(!GenerateAccount(strXml, strAccountName,
						strAccountAddress1, strAccountAddress2,
						strAccountCity, strAccountState, strAccountZip,
						strAccountCountry, strAccountPhone, strAccountFax)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Location
	strXml = "";
	if(!GenerateLocation(strXml, nLocationID, strLocationName,
						strLocationAddress1, strLocationAddress2,
						strLocationCity, strLocationState, strLocationZip,
						strLocationCountry, strLocationPhone, strLocationFax)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Licensed Prescriber
	strXml = "";
	if(!GenerateLicensedPrescriber(strXml, nProviderID, strProviderLast, strProviderFirst, strProviderMiddle,
			strDEANumber, strUPIN, strProviderState, strLicenseNumber, strProviderNPI)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Patient
	strXml = "";
	if(!GeneratePatient(strXml, nPatientID, nUserDefinedID, strPatientLast, strPatientFirst, strPatientMiddle,
				strPatientHomePhone, dtPatientBirthDate, nPatientGender, strPatientAddress1, strPatientAddress2,
				strPatientCity, strPatientState, strPatientZip, strPatientCountry)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//now generate our OutsidePrescription
	strXml = "";

	if(!GenerateOutsidePrescription(strXml, nPrescriptionID,
		strNewCropPharmacyID, strPharmacyPhone, strPharmacyFax, dtPrescriptionDate,
		strDoctorFullName, strDrugName, strQuantity, strRefillsAllowed,
		strPatientExplanation, strPharmacistNote, bAllowSubstitutions,
		strDrugStrengthWithUOM, strDrugDosageForm, strDrugRoute)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our footer
	GenerateNCScriptFooter(strXml);
	strXmlDocument += strXml;

	return TRUE;
}
*/

// (j.jones 2009-02-18 17:26) - PLID 33163 - web-browser request components
BOOL GenerateAddress(CString strAddressName, OUT CString &strXmlDocument,
					 CString strAddress1, CString strAddress2,
					 CString strCity, CString strState, CString strZip,
					 CString strCountry)
{
	CString strAddressNode;

	//for errors to make sense, find the word "Address" and insert a space
	CString strAddressNameForErrors = strAddressName;
	int nPos = strAddressNameForErrors.Find("Address");
	if(nPos != -1) {
		strAddressNameForErrors.Insert(nPos, " ");
	}
	
	if(strAddress1.IsEmpty()) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have Address 1 filled in.", strAddressNameForErrors);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	else if(strAddress1.GetLength() > 35) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: Address 1 values cannot be greater than 35 characters in length.\n"
			"The Address 1 \"%s\" is invalid.", strAddressNameForErrors, strAddress1);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	CString strAddress1XML = GetXMLElementValuePair_NewCrop("address1", strAddress1);
	strAddressNode += strAddress1XML;

	if(!strAddress2.IsEmpty()) {

		if(strAddress2.GetLength() > 35) {
			CString strWarn;
			strWarn.Format("There is invalid information in the %s: Address 2 values cannot be greater than 35 characters in length.\n"
				"The Address 2 \"%s\" is invalid.", strAddressNameForErrors, strAddress2);
			AfxMessageBox(strWarn);
			return FALSE;
		}

		CString strAddress2XML = GetXMLElementValuePair_NewCrop("address2", strAddress2);
		strAddressNode += strAddress2XML;
	}

	if(strCity.IsEmpty()) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have the City filled in.", strAddressNameForErrors);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	else if(strCity.GetLength() > 35) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: City names cannot be greater than 35 characters in length.\n"
			"The City \"%s\" is invalid.", strAddressNameForErrors, strCity);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	CString strCityXML = GetXMLElementValuePair_NewCrop("city", strCity);
	strAddressNode += strCityXML;

	strState.MakeUpper();
	if(strState.IsEmpty()) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have the State filled in.", strAddressNameForErrors);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	//Validate that it is a legitimate US State or territory, and nothing else. Sorry Canada.
	else if(!VerifyIsNewCropValidState(strState)) {		
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have a valid US state or territory. The state \"%s\" is invalid.", strAddressNameForErrors, strState);
		AfxMessageBox(strWarn);
		return FALSE;
	}	
	CString strStateXML = GetXMLElementValuePair_NewCrop("state", strState);
	strAddressNode += strStateXML;

	CString strZip5, strZip4;
	if(strZip.GetLength() > 5) {
		strZip5 = strZip.Left(5);
		strZip4 = strZip.Right(strZip.GetLength() - 5);
		strZip4.TrimLeft("-");
		strZip4.TrimLeft();
	}
	else {
		strZip5 = strZip;
	}

	if(strZip5.IsEmpty()) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have the Zip Code filled in.", strAddressNameForErrors);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	else if(strZip5.GetLength() != 5) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have at least a 5 digit Zip Code. The zip code \"%s\" is invalid.", strAddressNameForErrors, strZip);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	CString strZipXML = GetXMLElementValuePair_NewCrop("zip", strZip5);
	strAddressNode += strZipXML;

	if(!strZip4.IsEmpty()) {

		if(strZip4.GetLength() != 4) {
			CString strWarn;
			strWarn.Format("There is invalid information in the %s: All US addresses must have a 5 digit Zip Code, or a 9 digit Zip Code. The zip code \"%s\" is invalid.", strAddressNameForErrors, strZip);
			AfxMessageBox(strWarn);
			return FALSE;
		}

		CString strZip4XML = GetXMLElementValuePair_NewCrop("zip4", strZip4);
		strAddressNode += strZip4XML;
	}

	if(strCountry.IsEmpty()) {
		CString strWarn;
		strWarn.Format("There is invalid information in the %s: All addresses must have country codes.", strAddressNameForErrors);
		AfxMessageBox(strWarn);
		return FALSE;
	}
	CString strCountryXML = GetXMLElementValuePair_NewCrop("country", strCountry);
	strAddressNode += strCountryXML;

	strXmlDocument = GetXMLElementValuePair_NewCrop(strAddressName, strAddressNode, TRUE);

	return TRUE;
}

void GenerateNCScriptHeader(OUT CString &strXmlDocument)
{
	strXmlDocument = "<?xml version=\"1.0\"?>\r\n"
		"<NCScript xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\r\n"
          "\txmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n"
          "\txmlns=\"http://secure.newcropaccounts.com/interfaceV7\">\r\n";
}

void GenerateNCScriptFooter(OUT CString &strXmlDocument)
{
	strXmlDocument = "</NCScript>";
}

// (j.jones 2013-04-17 09:54) - PLID 56275 - this code is all in the API now
/*
BOOL GenerateCredentials(OUT CString &strXmlDocument)
{	
	CString strCredentials;

	CString strPartnerName = GetXMLElementValuePair_NewCrop("partnerName", "NEXTECH");
	strCredentials += strPartnerName;

	CString strName = "demo";
	CString strPassword = "demo";
	// (j.jones 2010-06-09 11:04) - PLID 39013 - production status is calculated by GetNewCropIsProduction()
	BOOL bIsProduction = GetNewCropIsProduction();
	if(bIsProduction) {
		//the production account has its own credentials
		//j.jones 2009-03-02 15:22) - PLID 33163 - these are our own hardcoded credentials for a production account,
		//they may change after future releases, but it is unlikely
		strName = "91d3130e-8d8c-4af7-b937-fb623b9feca7";
		strPassword = "fa78416b-e6b7-49c2-9bf5-b260ae02aad9";
	}
	
	CString strNameXML = GetXMLElementValuePair_NewCrop("name", strName);
	strCredentials += strNameXML;

	CString strPasswordXML = GetXMLElementValuePair_NewCrop("password", strPassword);
	strCredentials += strPasswordXML;

	CString strProductName = GetXMLElementValuePair_NewCrop("productName", "NexTech Practice");
	strCredentials += strProductName;

	CString strProductVersion = GetXMLElementValuePair_NewCrop("productVersion", PRODUCT_VERSION_TEXT);
	strCredentials += strProductVersion;

	//now wrap our content in the Credentials header
	strXmlDocument = GetXMLElementValuePair_NewCrop("Credentials", strCredentials, TRUE);

	return TRUE;
}

// (j.gruber 2009-06-08 11:03) - PLID 34515 - support different types
BOOL GenerateUserRole(OUT CString &strXmlDocument, NewCropUserTypes ncuTypeID)
{	
	CString strUserRole;

	if (ncuTypeID == ncutLicensedPrescriber) {

		//<!-- See XML Schema: UserType for valid values -->
		CString strUser = GetXMLElementValuePair_NewCrop("user", "LicensedPrescriber");
		strUserRole += strUser;

		//<!-- See XML Schema: RoleType for valid values -->
		CString strRole = GetXMLElementValuePair_NewCrop("role", "doctor");
		strUserRole += strRole;

	}
	// (j.gruber 2009-06-08 11:01) - PLID 34515 - added nurse/staff
	else if (ncuTypeID == ncutStaff_Nurse) {
		//<!-- See XML Schema: UserType for valid values -->
		CString strUser = GetXMLElementValuePair_NewCrop("user", "Staff");
		strUserRole += strUser;

		//<!-- See XML Schema: RoleType for valid values -->
		CString strRole = GetXMLElementValuePair_NewCrop("role", "nurse");
		strUserRole += strRole;
	}
	// (j.jones 2009-08-18 16:23) - PLID 35203 - supported Midlevel Prescriber
	else if (ncuTypeID == ncutMidlevelProvider) {
		//<!-- See XML Schema: UserType for valid values -->
		CString strUser = GetXMLElementValuePair_NewCrop("user", "MidlevelPrescriber");
		strUserRole += strUser;

		//<!-- See XML Schema: RoleType for valid values -->
		CString strRole = GetXMLElementValuePair_NewCrop("role", "midlevelPrescriber");
		strUserRole += strRole;
	}
	else {
		ASSERT(FALSE);
		return FALSE;
	}

		
	//now wrap our content in the UserRole header
	strXmlDocument = GetXMLElementValuePair_NewCrop("UserRole", strUserRole, TRUE);

	return TRUE;
}

BOOL GenerateDestination(OUT CString &strXmlDocument, NewCropRequestedPageType ncrptPageType)
{
	CString strDestination;

	CString strPageType = "";

	switch(ncrptPageType) {

		case ncrptCompose:
			strPageType = "compose";
			break;
		case ncrptAdmin:
			strPageType = "admin";
			break;
		case ncrptManager:
			strPageType = "manager";
			break;
		case ncrptStatus:
			strPageType = "status";
			break;
		case ncrptWsRxSend:
			strPageType = "ws-rx-send";
			break;
		case ncrptWsRxBulk:
			strPageType = "ws-rx-bulk";
			break;
		case ncrptWsRxBulkNoMatch:
			strPageType = "ws-rx-bulk-no-match";
			break;
		case ncrptWsRenewalSend:
			strPageType = "ws-renewal-send";
			break;
		case ncrptWsRegisterLicensedPrescriber:
			strPageType = "ws-register-licensedPrescriber";
			break;
		case ncrptWsImageRxSend:
			strPageType = "ws-image-rx-send";
			break;
		case ncrptWsPbmEligibility:
			strPageType = "ws-pbm-eligibility";
			break;
		case ncrptMedEntry:
			strPageType = "medentry";
			break;
		case ncrptPatientDetail:
			strPageType = "patientDetail";
			break;
		case ncrptTransmit:
			strPageType = "transmit";
			break;
		case ncrptResource:
			strPageType = "resource";
			break;
		case ncrptMaintainHealthPlans:
			strPageType = "maintainHealthplans";
			break;
		case ncrptWsRegister:
			strPageType = "ws-register";
			break;
		case ncrptRoute:
			strPageType = "route";
			break;
		case ncrptRxHistoryExternal:
			strPageType = "rxhistory-external";
			break;
		case ncrptRenewalConfirmation:
			strPageType = "renewal-confirmation";
			break;
		case ncrptWsProcessRenewal:
			strPageType = "ws-process-renewal";
			break;
		case ncrptRxDetail:
			strPageType = "rxdetail";
			break;
		case ncrptRenewal:
			strPageType = "renewal";
			break;
		case ncrptWsGenTestRenewal:
			strPageType = "ws-gen-test-renewal";
			break;
		case ncrptDosing:
			strPageType = "dosing";
			break;
		case ncrptFormularyCoverageDetail:
			strPageType = "formulary-coverage-detail";
			break;
		case ncrptWsPatientBulk:
			strPageType = "ws-patient-bulk";
			break;
		case ncrptInvalid:
		default:
			//invalid page type
			ASSERT(FALSE);
			return FALSE;
	}
  
	//<!-- See XML Schema: RequestedPageType for valid values -->
	CString strPage = GetXMLElementValuePair_NewCrop("requestedPage", strPageType);
	strDestination += strPage;

	//now wrap our content in the Destination header
	strXmlDocument = GetXMLElementValuePair_NewCrop("Destination", strDestination, TRUE);

	return TRUE;
}
*/

// (j.jones 2013-01-03 16:24) - PLID 49624 - these functions are all now obsolete, their logic is now in the API
/*
BOOL GenerateAccount(OUT CString &strXmlDocument, CString strAccountName,
					  CString strAddress1, CString strAddress2,
					  CString strCity, CString strState, CString strZip,
					  CString strCountry, CString strPhone, CString strFax)
{
	//build the Account message
	CString strAccount;

	//use the database name as the account ID
	//PracData_Main is our permanent "demo" account for production purposes
	CString strAccountID;
	if(GetSubRegistryKey().IsEmpty()) {
		strAccountID = "PracData";
	} else  {
		strAccountID.Format("PracData_%s", GetSubRegistryKey());
	}
	strAccount.Format("<Account ID=\"%s\">", strAccountID);
	
	if(strAccountName.IsEmpty()) {
		AfxMessageBox("Your Account Name is empty. You must select an Account Location in the Tools menu, E-Prescribing Setup.");
		return FALSE;
	}

	// (j.jones 2009-08-19 09:41) - PLID 35264 - ensure we do not send a location name greater than 50 characters,
	// it is safe to truncate, as NewCrop says it is only used for display purposes on the website	
	if(strAccountName.GetLength() > 50) {
		strAccountName = strAccountName.Left(50);
	}

	CString strAccountNameXML = GetXMLElementValuePair_NewCrop("accountName", strAccountName);
	strAccount += "\r\n" + strAccountNameXML;

	//use the license key as the site ID
	//70116 is our permanent "demo" account for production purposes
	long nLicenseKey = g_pLicense->GetLicenseKey();
	CString strLicenseKey;
	strLicenseKey.Format("%li", nLicenseKey);

	CString strSiteID = GetXMLElementValuePair_NewCrop("siteID", strLicenseKey);
	strAccount += strSiteID;

	//now add the address
	CString strAccountAddress;
	if(!GenerateAddress("AccountAddress", strAccountAddress,
		strAddress1, strAddress2, strCity,
		strState, strZip, strCountry)) {
		return FALSE;
	}
	strAccount += strAccountAddress;

	if(strPhone.IsEmpty()) {		
		AfxMessageBox("To access E-Prescribing, your Account Location must have a phone number filled in in the Locations Tab of the Administrator module.");
		return FALSE;
	}
	CString strPhoneXML = GetXMLElementValuePair_NewCrop("accountPrimaryPhoneNumber", strPhone);
	strAccount += strPhoneXML;

	if(strFax.IsEmpty()) {
		AfxMessageBox("To access E-Prescribing, your Account Location must have a fax number filled in in the Locations Tab of the Administrator module.");
		return FALSE;
	}
	CString strFaxXML = GetXMLElementValuePair_NewCrop("accountPrimaryFaxNumber", strFax);
	strAccount += strFaxXML;

	strAccount += "</Account>";

	strXmlDocument = strAccount;

	return TRUE;
}

BOOL GenerateLocation(OUT CString &strXmlDocument, long nLocationID, CString strLocationName,
					  CString strAddress1, CString strAddress2,
					  CString strCity, CString strState, CString strZip,
					  CString strCountry, CString strPhone, CString strFax)
{
	//build the Location message
	CString strLocation;

	//use our internal location ID
	strLocation.Format("<Location ID=\"%li\">", nLocationID);

	if(strLocationName.IsEmpty()) {
		AfxMessageBox("To access E-Prescribing, your current location must have a name filled in in the Locations Tab of the Administrator module.");
		return FALSE;
	}

	// (j.jones 2009-08-19 09:41) - PLID 35264 - ensure we do not send a location name greater than 50 characters,
	// it is safe to truncate, as NewCrop says it is only used for display purposes on the website
	if(strLocationName.GetLength() > 50) {
		strLocationName = strLocationName.Left(50);
	}

	CString strLocationNameXML = GetXMLElementValuePair_NewCrop("locationName", strLocationName);
	strLocation += "\r\n" + strLocationNameXML;

	//now add the address
	CString strLocationAddress;
	if(!GenerateAddress("LocationAddress", strLocationAddress,
		strAddress1, strAddress2, strCity,
		strState, strZip, strCountry)) {
		return FALSE;
	}
	strLocation += strLocationAddress;

	if(strPhone.IsEmpty()) {
		AfxMessageBox("To access E-Prescribing, your current location must have a phone number filled in in the Locations Tab of the Administrator module.");
		return FALSE;
	}
	CString strPhoneXML = GetXMLElementValuePair_NewCrop("primaryPhoneNumber", strPhone);
	strLocation += strPhoneXML;

	if(strFax.IsEmpty()) {
		AfxMessageBox("To access E-Prescribing, your current location must have a fax number filled in in the Locations Tab of the Administrator module.");
		return FALSE;
	}
	CString strFaxXML = GetXMLElementValuePair_NewCrop("primaryFaxNumber", strFax);
	strLocation += strFaxXML;

	//pharmacyContactNumber is not defined, but I assume it's the number the pharmacy should call,
	//which may differ from the main office number. For now, send the same number.
	//Note: this is required, so if we ever do use a different number, we have to enforce
	//that it is filled in.
	CString strPharmacyContactXML = GetXMLElementValuePair_NewCrop("pharmacyContactNumber", strPhone);
	strLocation += strPharmacyContactXML;

	strLocation += "</Location>";

	strXmlDocument = strLocation;

	return TRUE;
}

// (j.gruber 2009-06-08 10:11) - PLID 34515 - add function for staff/nurse
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
BOOL GenerateStaff_Nurse(OUT CString &strXmlDocument, CString strUserID, CString strUserFirst, CString strUserMiddle, CString strUserLast,
						 CString strUserPrefix, CString strUserSuffix)
{

	CString strUserName;

	if(strUserLast.IsEmpty()) {		
		AfxMessageBox("The current user does not have a Last name. You must correct this in the Contacts module.");
		return FALSE;
	}
	CString strLastXML = GetXMLElementValuePair_NewCrop("last", strUserLast);
	strUserName += strLastXML;

	if(strUserFirst.IsEmpty()) {		
		AfxMessageBox("The current user does not have a First name. You must correct this in the Contacts module.");
		return FALSE;
	}

	CString strFirstXML = GetXMLElementValuePair_NewCrop("first", strUserFirst);
	strUserName += strFirstXML;

	if(!strUserMiddle.IsEmpty()) {
		CString strMiddleXML = GetXMLElementValuePair_NewCrop("middle", strUserMiddle);
		strUserName += strMiddleXML;
	}

	// (j.jones 2009-12-18 11:09) - PLID 36391 - supported prefix
	if(!strUserPrefix.IsEmpty()) {

		if(!VerifyIsNewCropValidNamePrefix(strUserPrefix)) {
			AfxMessageBox("The current user does not have a valid Prefix entered. You must correct this in the Contacts module.");
			return FALSE;
		}

		CString strPrefixXML = GetXMLElementValuePair_NewCrop("prefix", strUserPrefix);
		strUserName += strPrefixXML;
	}

	// (j.jones 2009-12-18 11:09) - PLID 36391 - supported suffix
	// (j.jones 2010-04-30 14:00) - PLID 38183 - added ability to disable sending the suffix (for everyone, not just providers)
	// (j.jones 2010-07-29 12:06) - PLID 39880 - removed the setting to hide the suffix, now we just use the passed-in value
	// (the setting is applied upon load)
	if(!strUserSuffix.IsEmpty()) {

		// (j.jones 2010-04-30 14:01) - PLID 38183 - if the suffix sans-periods is MD,
		// change it to MD (as M.D. is invalid), but do not try to modify anything else
		CString strTestSuffix = strUserSuffix;
		strTestSuffix.Replace(".","");
		strTestSuffix.MakeUpper();
		if(strTestSuffix == "MD") {
			//it is MD, use it
			strUserSuffix = strTestSuffix;
		}

		if(!VerifyIsNewCropValidNameSuffix(strUserSuffix)) {
			AfxMessageBox("The current user does not have a valid Title (Suffix) entered. You must correct this in the Contacts module.");
			return FALSE;
		}

		CString strSuffixXML = GetXMLElementValuePair_NewCrop("suffix", strUserSuffix);
		strUserName += strSuffixXML;
	}

	//ID
	CString strIDXML;
	strIDXML.Format("<Staff ID=\"%s\">", strUserID);

	//build the whole thing
	CString strUser;
	strUser += strIDXML;
	strUser += GetXMLElementValuePair_NewCrop("StaffName", strUserName, TRUE);	
	
	strUser += "</Staff>";

	strXmlDocument = strUser;

	return TRUE;
}

// (j.jones 2009-08-18 16:23) - PLID 35203 - I added support for Midlevel Prescriber, which uses the same XML
// output as Licensed Prescriber, except that DEA number is not required for Midlevel Prescribers.
// So I renamed this function appropriately and made it more modular.
// Also added bIsSupervising for better context-sensitive warnings when sending a supervising provider XML segment.
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
BOOL GeneratePrescriber(OUT CString &strXmlDocument, NewCropUserTypes ncutSendAsTypeID,
						BOOL bIsSupervising, long nProviderID,
						CString strLast, CString strFirst, CString strMiddle,
						CString strPrefix, CString strSuffix,
						CString strDEANumber, CString strUPIN, CString strState, CString strLicenseNumber,
						CString strNPI)
{
	//first we need the name

	CString strPrescriberName;
	
	if(strLast.IsEmpty()) {
		// (j.jones 2009-06-08 09:02) - PLID 34514 - we now use the provider linked to the logged-in user
		// (j.jones 2009-08-25 09:20) - PLID 35203 - warn for the appropriate provider
		if(bIsSupervising) {
			AfxMessageBox("The supervising provider selected for your user account does not have a Last name. You must correct this in the Contacts module.");			
		}
		else {
			AfxMessageBox("The linked provider selected for your user account does not have a Last name. You must correct this in the Contacts module.");
		}
		return FALSE;
	}
	CString strLastXML = GetXMLElementValuePair_NewCrop("last", strLast);
	strPrescriberName += strLastXML;

	if(strFirst.IsEmpty()) {
		// (j.jones 2009-06-08 09:02) - PLID 34514 - we now use the provider linked to the logged-in user
		// (j.jones 2009-08-25 09:20) - PLID 35203 - warn for the appropriate provider
		if(bIsSupervising) {
			AfxMessageBox("The supervising provider selected for your user account does not have a First name. You must correct this in the Contacts module.");
		}
		else {
			AfxMessageBox("The linked provider selected for your user account does not have a First name. You must correct this in the Contacts module.");
		}
		return FALSE;
	}
	CString strFirstXML = GetXMLElementValuePair_NewCrop("first", strFirst);
	strPrescriberName += strFirstXML;

	if(!strMiddle.IsEmpty()) {
		CString strMiddleXML = GetXMLElementValuePair_NewCrop("middle", strMiddle);
		strPrescriberName += strMiddleXML;
	}

	// (j.jones 2009-12-18 11:09) - PLID 36391 - supported prefix
	if(!strPrefix.IsEmpty()) {

		if(!VerifyIsNewCropValidNamePrefix(strPrefix)) {
			if(bIsSupervising) {
				AfxMessageBox("The supervising provider selected for your user account does not have a valid Prefix entered. You must correct this in the Contacts module.");
			}
			else {
				AfxMessageBox("The linked provider selected for your user account does not have a valid Prefix entered. You must correct this in the Contacts module.");
			}
			return FALSE;
		}

		CString strPrefixXML = GetXMLElementValuePair_NewCrop("prefix", strPrefix);
		strPrescriberName += strPrefixXML;
	}

	// (j.jones 2009-12-18 11:09) - PLID 36391 - supported suffix
	// (j.jones 2010-04-13 15:50) - PLID 38183 - added ability to disable sending the suffix
	// (j.jones 2010-07-29 12:06) - PLID 39880 - removed the setting to hide the suffix, now we just use the passed-in value
	// (the setting is applied upon load)
	if(!strSuffix.IsEmpty()) {

		// (j.jones 2010-04-13 15:51) - PLID 38183 - if the suffix sans-periods is MD,
		// change it to MD (as M.D. is invalid), but do not try to modify anything else
		CString strTestSuffix = strSuffix;
		strTestSuffix.Replace(".","");
		strTestSuffix.MakeUpper();
		if(strTestSuffix == "MD") {
			//it is MD, use it
			strSuffix = strTestSuffix;
		}

		if(!VerifyIsNewCropValidNameSuffix(strSuffix)) {
			if(bIsSupervising) {
				AfxMessageBox("The supervising provider selected for your user account does not have a valid Title (Suffix) entered. You must correct this in the Contacts module.");
			}
			else {
				AfxMessageBox("The linked provider selected for your user account does not have a valid Title (Suffix) entered. You must correct this in the Contacts module.");
			}
			return FALSE;
		}

		CString strSuffixXML = GetXMLElementValuePair_NewCrop("suffix", strSuffix);
		strPrescriberName += strSuffixXML;
	}
	
	//now build the Licensed Prescriber / Midlevel Prescriber message
	CString strPrescriber;

	//use our internal provider ID

	// (j.jones 2009-08-18 17:08) - PLID 35203 - supported Midlevel Prescribers
	if(ncutSendAsTypeID == ncutLicensedPrescriber) {
		strPrescriber.Format("<LicensedPrescriber ID=\"%li\">", nProviderID);
	}
	else if(ncutSendAsTypeID == ncutMidlevelProvider) {
		strPrescriber.Format("<MidlevelPrescriber ID=\"%li\">", nProviderID);
	}
	else {
		//this should not be possible
		ThrowNxException("GeneratePrescriber called without a valid prescriber role!");
	}

	//now add the name (the element "LicensedPrescriberName" is used even for Midlevel Prescribers)
	strPrescriber += "\r\n" + GetXMLElementValuePair_NewCrop("LicensedPrescriberName", strPrescriberName, TRUE);

	if(!strDEANumber.IsEmpty()) {
		CString strDEA = GetXMLElementValuePair_NewCrop("dea", strDEANumber);
		strPrescriber += strDEA;
	}
	// (j.jones 2009-08-18 17:08) - PLID 35203 - if a Midlevel Prescriber, a blank DEA is acceptable,
	// but it is required for a Licensed Prescriber
	else if(ncutSendAsTypeID == ncutLicensedPrescriber) {
		// (j.jones 2009-06-08 09:02) - PLID 34514 - we now use the provider linked to the logged-in user
		// (j.jones 2009-08-25 09:20) - PLID 35203 - warn for the appropriate provider
		if(bIsSupervising) {
			AfxMessageBox("The supervising provider selected for your user account does not have a DEA Number configured. You must correct this in the Contacts module.");
		}
		else {
			AfxMessageBox("The linked provider selected for your user account does not have a DEA Number configured. You must correct this in the Contacts module.");
		}
		return FALSE;
	}
	
	if(!strUPIN.IsEmpty()) {
		CString strUPINXML = GetXMLElementValuePair_NewCrop("upin", strUPIN);
		strPrescriber += strUPINXML;
	}

	if(!strState.IsEmpty()) {
		strState.MakeUpper();
		CString strStateXML = GetXMLElementValuePair_NewCrop("licenseState", strState);
		strPrescriber += strStateXML;
	}

	if(!strLicenseNumber.IsEmpty()) {
		CString strLicenseNumXML = GetXMLElementValuePair_NewCrop("licenseNumber", strLicenseNumber);
		strPrescriber += strLicenseNumXML;
	}

	if(!strNPI.IsEmpty()) {
		CString strNPIXML = GetXMLElementValuePair_NewCrop("npi", strNPI);
		strPrescriber += strNPIXML;
	}

	// (j.jones 2009-08-18 17:08) - PLID 35203 - supported Midlevel Prescribers
	if(ncutSendAsTypeID == ncutLicensedPrescriber) {
		strPrescriber += "</LicensedPrescriber>";
	}
	else if(ncutSendAsTypeID == ncutMidlevelProvider) {
		strPrescriber += "</MidlevelPrescriber>";
	}

	strXmlDocument = strPrescriber;

	return TRUE;
}

// (j.gruber 2009-05-12 13:10) - PLID 29906 - Added diagcodes
// (j.jones 2009-12-18 11:30) - PLID 36391 - supported prefix and suffix
// (j.jones 2010-06-03 15:04) - PLID 38994 - supported sending the insurance company names
BOOL GeneratePatient(OUT CString &strXmlDocument, long nPatientID, long nUserDefinedID,
					 CString strLast, CString strFirst, CString strMiddle,
					 CString strPrefix, CString strSuffix,
					 CString strHomePhone, COleDateTime dtBirthDate, long nGender,
					 CString strAddress1, CString strAddress2,
					 CString strCity, CString strState, CString strZip,
					 CString strCountry, CString strAllDiagCodesXML,
					 CString strPrimaryInsuranceCoName, CString strSecondaryInsuranceCoName)
{
	//first we need the name
	CString strPatientName;
	
	if(strLast.IsEmpty()) {
		AfxMessageBox("The current patient does not have a Last name. You must correct this in the General 1 Tab of the Patients Module.");
		return FALSE;
	}
	CString strLastXML = GetXMLElementValuePair_NewCrop("last", strLast);
	strPatientName += strLastXML;

	if(strFirst.IsEmpty()) {
		AfxMessageBox("The current patient does not have a First name. You must correct this in the General 1 Tab of the Patients Module.");
		return FALSE;
	}
	CString strFirstXML = GetXMLElementValuePair_NewCrop("first", strFirst);
	strPatientName += strFirstXML;

	if(!strMiddle.IsEmpty()) {
		CString strMiddleXML = GetXMLElementValuePair_NewCrop("middle", strMiddle);
		strPatientName += strMiddleXML;
	}

	// (j.jones 2009-12-18 11:09) - PLID 36391 - supported prefix
	if(!strPrefix.IsEmpty()) {

		if(!VerifyIsNewCropValidNamePrefix(strPrefix)) {
			AfxMessageBox("The current patient does not have a valid Prefix entered. You must correct this in the General 1 Tab of the Patients Module.");
			return FALSE;
		}

		CString strPrefixXML = GetXMLElementValuePair_NewCrop("prefix", strPrefix);
		strPatientName += strPrefixXML;
	}

	// (j.jones 2009-12-18 11:09) - PLID 36391 - supported suffix
	// (j.jones 2010-04-30 14:00) - PLID 38183 - added ability to disable sending the suffix (for everyone, not just providers)
	// (j.jones 2010-07-29 12:06) - PLID 39880 - removed the setting to hide the suffix, now we just use the passed-in value
	// (the setting is applied upon load)
	if(!strSuffix.IsEmpty()) {

		// (j.jones 2010-04-30 14:01) - PLID 38183 - if the suffix sans-periods is MD,
		// change it to MD (as M.D. is invalid), but do not try to modify anything else
		CString strTestSuffix = strSuffix;
		strTestSuffix.Replace(".","");
		strTestSuffix.MakeUpper();
		if(strTestSuffix == "MD") {
			//it is MD, use it
			strSuffix = strTestSuffix;
		}

		if(!VerifyIsNewCropValidNameSuffix(strSuffix)) {
			AfxMessageBox("The current patient does not have a valid Title (Suffix) entered. You must correct this in the General 1 Tab of the Patients Module.");
			return FALSE;
		}

		CString strSuffixXML = GetXMLElementValuePair_NewCrop("suffix", strSuffix);
		strPatientName += strSuffixXML;
	}

	//build the Patient message
	CString strPatient;

	//use our internal patient ID
	strPatient.Format("<Patient ID=\"%li\">", nPatientID);
	
	//add the patient name
	CString strPatientNameXml = "\r\n" + GetXMLElementValuePair_NewCrop("PatientName", strPatientName, TRUE);
	strPatient += strPatientNameXml;

	//this ID is displayed as the chart number in NewCrop
	CString strMedRecordNum = GetXMLElementValuePair_NewCrop("medicalRecordNumber", AsString(nUserDefinedID));
	strPatient += strMedRecordNum;

	// (j.jones 2009-02-26 17:23) - remove this, NewCrop doesn't want it
	//StripNonNumericChars(strSocialSecurity);
	//if(!strSocialSecurity.IsEmpty()) {
	//	CString strSocialSecurityXML = GetXMLElementValuePair_NewCrop("socialSecurityNumber", strSocialSecurity);
	//	strPatient += strSocialSecurityXML;
	//}

	//now add the address
	CString strPatientAddress;
	if(!GenerateAddress("PatientAddress", strPatientAddress,
		strAddress1, strAddress2, strCity,
		strState, strZip, strCountry)) {
		return FALSE;
	}
	strPatient += strPatientAddress;

	StripNonNumericChars(strHomePhone);
	if(!strHomePhone.IsEmpty()) {
		CString strPhoneXML = GetXMLElementValuePair_NewCrop("homeTelephone", strHomePhone);
		strPatient += GetXMLElementValuePair_NewCrop("PatientContact", strPhoneXML, TRUE);
	}

	CString strCharacteristics;

	if(dtBirthDate.GetStatus() != COleDateTime::invalid) {
		CString strBirthDate;
		strBirthDate = dtBirthDate.Format("%Y%m%d");
		CString strDOBXML = GetXMLElementValuePair_NewCrop("dob", strBirthDate);
		strCharacteristics += strDOBXML;
	}
	else {
		AfxMessageBox("The current patient does not have a valid Birthdate. You must correct this in the General 1 Tab of the Patients Module.");
		return FALSE;
	}

	CString strGender = "";
	if(nGender == 1) {
		strGender = "M";
	}
	else if(nGender == 2) {
		strGender = "F";
	}
	
	// (j.jones 2011-03-07 11:46) - PLID 42313 - added an option to not send patient gender
	if(!strGender.IsEmpty() && GetRemotePropertyInt("NewCrop_DoNotSendPatientGender", 0, 0, "<None>", false) == 0) {
		CString strGenderXML = GetXMLElementValuePair_NewCrop("gender", strGender);
		strCharacteristics += strGenderXML;
	}

	//the contents of PatientCharacteristics are optional, but the segment itself is not optional,
	//so send it even if strCharacteristics is empty
	strPatient += GetXMLElementValuePair_NewCrop("PatientCharacteristics", strCharacteristics, TRUE);

	// (j.gruber 2009-05-12 13:10) - PLID 29906 - added diagcodes
	strPatient += strAllDiagCodesXML;

	strPatient += "</Patient>";

	strXmlDocument = strPatient;

	return TRUE;
}
*/

// (j.jones 2009-03-04 17:00) - PLID 33345 - support the ExternalOverrideDrug element
/* temporarily removed
BOOL GenerateExternalOverrideDrug(OUT CString &strXmlDocument, CString strDrugName,
								  CString strDrugStrengthWithUOM, CString strDrugDosageForm,
								  CString strDrugRoute)
{
	//build the external override drug node
	CString strExternalOverrideDrug;

	//externalDrugConcept - we don't use this

	if(!strDrugName.IsEmpty()) {
		CString strDrugNameXML = GetXMLElementValuePair_NewCrop("externalDrugName", strDrugName);
		strExternalOverrideDrug += strDrugNameXML;
	}

	//externalDrugStrength - not used separately from UOM
	//externalDrugStrengthWithUOM - not used separately from drug strength

	if(!strDrugStrengthWithUOM.IsEmpty()) {
		CString strDrugStrengthWithUOMXML = GetXMLElementValuePair_NewCrop("externalDrugStrengthWithUOM", strDrugStrengthWithUOM);
		strExternalOverrideDrug += strDrugStrengthWithUOMXML;
	}

	if(!strDrugDosageForm.IsEmpty()) {
		CString strDrugDosageFormXML = GetXMLElementValuePair_NewCrop("externalDrugDosageForm", strDrugDosageForm);
		strExternalOverrideDrug += strDrugDosageFormXML;
	}

	if(!strDrugRoute.IsEmpty()) {
		CString strDrugRouteXML = GetXMLElementValuePair_NewCrop("externalDrugRoute", strDrugRoute);
		strExternalOverrideDrug += strDrugRouteXML;
	}

	//externalDrugIdentifier - we don't use this

	//externalDrugIdentifierType - we don't use this

	//<!-- Drug Enforcement Agency (DEA) drug schedule.  Typically None, 2, 3, 4, or 5 -->
	CString strDrugSchedule = GetXMLElementValuePair_NewCrop("externalDrugSchedule", "None");
	strExternalOverrideDrug += strDrugSchedule;

	//externalDrugOverTheCounter - we don't use this

	//externalDrugNdc - we don't use this

	strXmlDocument = GetXMLElementValuePair_NewCrop("externalOverrideDrug", strExternalOverrideDrug, TRUE);

	return TRUE;
}
*/

// (j.jones 2009-03-04 17:00) - PLID 33345 - support the OutsidePrescription element
/* temporarily removed
BOOL GenerateOutsidePrescription(OUT CString &strXmlDocument, long nPrescriptionID,
								 CString strNewCropPharmacyID, CString strPharmacyPhone, CString strPharmacyFax,
								 COleDateTime dtPrescriptionDate,
								 CString strDoctorFullName, CString strDrugName, CString strQuantity,
								 CString strRefillsAllowed, CString strPatientExplanation,
								 CString strPharmacistNote, BOOL bAllowSubstitutions,
								 CString strDrugStrengthWithUOM, CString strDrugDosageForm, CString strDrugRoute)
{
	//build the OutsidePrescription message
	CString strOutsidePrescription;
	
	//the external ID is our internal ID
	CString strExternalIDXML = GetXMLElementValuePair_NewCrop("externalId", AsString(nPrescriptionID));
	strOutsidePrescription += strExternalIDXML;

	if(!strNewCropPharmacyID.IsEmpty()) {
		CString strPharmacyIdentifierXML = GetXMLElementValuePair_NewCrop("pharmacyIdentifier", strNewCropPharmacyID);
		strOutsidePrescription += strPharmacyIdentifierXML;
	}

	if(!strPharmacyPhone.IsEmpty()) { 
		CString strPharmacyPhoneXML = GetXMLElementValuePair_NewCrop("pharmacyPhone", strPharmacyPhone);
		strOutsidePrescription += strPharmacyPhone;
	}

	if(!strPharmacyFax.IsEmpty()) { 
		CString strPharmacyFaxXML = GetXMLElementValuePair_NewCrop("pharmacyFax", strPharmacyFax);
		strOutsidePrescription += strPharmacyFaxXML;
	}

	if(dtPrescriptionDate.GetStatus() != COleDateTime::invalid) {
		CString strPrescriptionDate;
		strPrescriptionDate = dtPrescriptionDate.Format("%Y%m%d");
		CString strPrescriptionDateXML = GetXMLElementValuePair_NewCrop("date", strPrescriptionDate);
		strOutsidePrescription += strPrescriptionDateXML;
	}

	if(!strDoctorFullName.IsEmpty()) { 
		CString strDoctorFullNameXML = GetXMLElementValuePair_NewCrop("doctorName", strDoctorFullName);
		strOutsidePrescription += strDoctorFullNameXML;
	}

	if(!strDrugName.IsEmpty()) { 
		CString strDrugNameXML = GetXMLElementValuePair_NewCrop("drug", strDrugName);
		strOutsidePrescription += strDrugNameXML;
	}

	//dosage - not currently filled in	

	if(!strQuantity.IsEmpty()) {
		CString strDispenseNumberXML = GetXMLElementValuePair_NewCrop("dispenseNumber", strQuantity);
		strOutsidePrescription += strDispenseNumberXML;
	}

	if(!strPatientExplanation.IsEmpty()) {
		CString strSigXML = GetXMLElementValuePair_NewCrop("sig", strPatientExplanation);
		strOutsidePrescription += strSigXML;
	}

	if(!strRefillsAllowed.IsEmpty()) {
		CString strRefillCountXML = GetXMLElementValuePair_NewCrop("refillCount", strRefillsAllowed);
		strOutsidePrescription += strRefillCountXML;
	}

	CString strSubstitution = "DispenseAsWritten";
	if(bAllowSubstitutions) {
		strSubstitution = "SubstitutionAllowed";
	}
	CString strSubstitutionXML = GetXMLElementValuePair_NewCrop("substitution", strSubstitution);
	strOutsidePrescription += strSubstitutionXML;

	if(!strPharmacistNote.IsEmpty()) {
		CString strPharmacistMessageXML = GetXMLElementValuePair_NewCrop("pharmacistMessage", strPharmacistNote);
		strOutsidePrescription += strPharmacistMessageXML;
	}

	CString strDrugIdentifierXML = GetXMLElementValuePair_NewCrop("drugIdentifier", "0");
	strOutsidePrescription += strDrugIdentifierXML;

	CString strDrugIdentifierTypeXML = GetXMLElementValuePair_NewCrop("drugIdentifierType", "Z");
	strOutsidePrescription += strDrugIdentifierTypeXML;
	
	CString strPrescriptionTypeXML = GetXMLElementValuePair_NewCrop("prescriptionType", "reconcile");
	strOutsidePrescription += strPrescriptionTypeXML;

	//get the ExternalOverrideDrug
	CString strExternalOverrideDrug;
	if(!GenerateExternalOverrideDrug(strExternalOverrideDrug, strDrugName, strDrugStrengthWithUOM, strDrugDosageForm, strDrugRoute)) {
		return FALSE;
	}

	strOutsidePrescription += strExternalOverrideDrug;

	strXmlDocument = GetXMLElementValuePair_NewCrop("OutsidePrescription", strOutsidePrescription, TRUE);

	return TRUE;
}
*/

// (j.dinatale 2010-10-04) - PLID 40604 - Moved to NxNewCropSOAPUtils static lib
//FormatForNewCrop will take in a value and strip out characters NewCrop does not support
//CString FormatForNewCrop(CString strValue)
//{
//	//NewCrop only allows the following characters in its content: a-zA-Z0-9 '.,()#:/\-@_%\r\n
//	CString strNewValue;
//	for(int i = 0; i < strValue.GetLength(); i++) 
//	{
//		char ch = strValue.GetAt(i);
//		if((ch >= '0' && ch <= '9')
//			|| (ch >= 'a' && ch <= 'z')
//			|| (ch >= 'A' && ch <= 'Z')
//			|| (ch == ' ') || (ch == '\'')
//			|| (ch == '.') || (ch == ',')
//			|| (ch == '(') || (ch == ')')
//			|| (ch == '#') || (ch == ':')
//			|| (ch == '/') || (ch == '\\')
//			|| (ch == '-') || (ch == '@')
//			|| (ch == '_') || (ch == '%')
//			|| (ch == '\r') || (ch == '\n')) {
//
//			strNewValue += ch;
//		}
//	}
//
//	return strNewValue;
//}

// (j.dinatale 2010-10-04) - PLID 40604 - Moved to NxNewCropSOAPUtils static lib
//GetXMLElementValuePair_NewCrop will build an XML pair of <strElement>strValue</strElement>,
//and will format strValue to be NewCrop-friendly unless bIsValueXML is TRUE, in which case
//we are merely wrapping existing XML in a new strElement
//CString GetXMLElementValuePair_NewCrop(CString strElement, CString strValue, BOOL bIsValueXML /*= FALSE*/)
//{
//	//Converts "user_id" and "nextech" into 
//	//	<user_id>nextech</user_id>
//
//	//if bIsValueXML is FALSE, then our content is not XML content, and needs converted as such,
//	//otherwise our content contains XML subnodes, and needs no manipulation
//	if(!bIsValueXML) {
//		strValue = FormatForNewCrop(strValue);
//	}
//	else {
//		//insert a newline to help cleanly separate embedded XML
//		strValue = "\r\n" + strValue;
//	}
//
//	CString strValuePair;
//	strValuePair.Format("<%s>%s</%s>\r\n", strElement, strValue, strElement);
//
//	return strValuePair;
//}



// (j.jones 2009-02-25 10:16) - PLID 33232 - added ability to generate a renewal
//this function builds the full XML necessary to login to a paient's account, and respond to a renewal request
// (j.gruber 2009-05-15 13:45) - PLID 28541 - we are implementing this now
// (j.gruber 2009-05-15 13:46) - PLID  28541 - I changed this because otherwise we'd be duplicating a lot of code from the login and 
//really we are using the same information as the login
BOOL GenerateFullPrescriptionRenewalResponse(OUT CString &strXmlDocument,
						CString strPatientLoginXML,
						CString strRenewalRequestIdentifier, NewCropResponseCodeType ncrctResponseCode /*=ncrctUndetermined*/,
						CString strRefillCount /*""*/, NewCropResponseDenyCodeType ncrdctDenyCode /*= ncrdctInvalid*/,
						CString strMessageToPharmacist /*= ""*/)
{
	CString strXml;
	
	/*//get our header
	strXml = "";
	GenerateNCScriptHeader(strXml);
	strXmlDocument = strXml;

	//get our Credentials
	strXml = "";
	if(!GenerateCredentials(strXml)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our UserRole
	strXml = "";
	if(!GenerateUserRole(strXml)) {
		return FALSE;
	}
	strXmlDocument += strXml;
  
	//get our Destination

	strXml = "";
	if(!GenerateDestination(strXml, ncrptRenewal)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Account
	strXml = "";
	if(!GenerateAccount(strXml, strAccountName,
						strAccountAddress1, strAccountAddress2,
						strAccountCity, strAccountState, strAccountZip,
						strAccountCountry, strAccountPhone, strAccountFax)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Location
	strXml = "";
	if(!GenerateLocation(strXml, nLocationID, strLocationName,
						strLocationAddress1, strLocationAddress2,
						strLocationCity, strLocationState, strLocationZip,
						strLocationCountry, strLocationPhone, strLocationFax)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Licensed Prescriber
	strXml = "";
	if(!GenerateLicensedPrescriber(strXml, nProviderID, strProviderLast, strProviderFirst, strProviderMiddle,
			strDEANumber, strUPIN, strProviderState, strLicenseNumber, strProviderNPI)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our Patient
	strXml = "";
	if(!GeneratePatient(strXml, nPatientID, nUserDefinedID, strPatientLast, strPatientFirst, strPatientMiddle,
				strPatientHomePhone, dtPatientBirthDate, nPatientGender, strPatientAddress1, strPatientAddress2,
				strPatientCity, strPatientState, strPatientZip, strPatientCountry)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//now generate our PrescriptionRenewalResponse
	strXml = "";*/

	strXmlDocument += strPatientLoginXML;

	if(!GeneratePrescriptionRenewalResponse(strXml, strRenewalRequestIdentifier, ncrctResponseCode, 
		strRefillCount, ncrdctDenyCode, strMessageToPharmacist)) {
		return FALSE;
	}
	strXmlDocument += strXml;

	//get our footer
	GenerateNCScriptFooter(strXml);
	strXmlDocument += strXml;

	return TRUE;
}


// (j.jones 2009-02-25 10:16) - PLID 33232 - added ability to generate a renewal
// (j.gruber 2009-05-15 14:06) - PLID 28541 - added back in
BOOL GeneratePrescriptionRenewalResponse(OUT CString &strXmlDocument,
										 CString strRenewalRequestIdentifier, NewCropResponseCodeType ncrctResponseCode /*=ncrctUndefined*/,
										 CString strRefillCount /*=""*/, NewCropResponseDenyCodeType ncrdctDenyCode /*=ncrdtInvalid*/,
										 CString strMessageToPharmacist /*=""*/)
{
	//build the PrescriptionRenewalResponse message
	CString strPrescriptionRenewalResponse;
	
	if(strRenewalRequestIdentifier.IsEmpty()) {
		AfxMessageBox("All renewal responses must have a request identifier.");
		return FALSE;
	}
	CString strRenewalRequestIdentifierXML = GetXMLElementValuePair_NewCrop("renewalRequestIdentifier", strRenewalRequestIdentifier);
	strPrescriptionRenewalResponse += strRenewalRequestIdentifierXML;

	CString strResponseCode = "";
	switch(ncrctResponseCode) {
		case ncrctAccept:
			strResponseCode = "Accept";
			break;
		case ncrctDeny:
			strResponseCode = "Deny";
			break;
		case ncrctUnableToProcess:
			strResponseCode = "UnableToProcess";
			break;
		case ncrctUndetermined:
			strResponseCode = "Undetermined";
			break;
		case ncrctInvalid:
		default:
			strResponseCode = "";
			break;
	}

	if(strResponseCode.IsEmpty()) {
		AfxMessageBox("All renewal responses must have a response code.");
		return FALSE;
	}
	CString strResponseCodeXML = GetXMLElementValuePair_NewCrop("responseCode", strResponseCode);
	strPrescriptionRenewalResponse += strResponseCodeXML;

	if(!strRefillCount.IsEmpty()) {
		CString strRefillCountXML = GetXMLElementValuePair_NewCrop("refillCount", strRefillCount);
		strPrescriptionRenewalResponse += strRefillCountXML;
	}

	// (j.gruber 2009-05-18 08:41) - PLID 28541 - took this out because I was getting errors in validation saying this was unexpected
	/*CString strDrugScheduleXML = GetXMLElementValuePair_NewCrop("externalDrugSchedule", "None");
	strPrescriptionRenewalResponse += strDrugScheduleXML;*/

	CString strResponseDenyCode = "";
	switch(ncrdctDenyCode) {
		case ncrdctPatientUnknownToThePrescriber:
			strResponseDenyCode = "PatientUnknownToThePrescriber";
			break;
		case ncrdctPatientNeverUnderPrescriberCare:
			strResponseDenyCode = "PatientNeverUnderPrescriberCare";
			break;
		case ncrdctPatientNoLongerUnderPrescriberCare:
			strResponseDenyCode = "PatientNoLongerUnderPrescriberCare";
			break;
		case ncrdctPatientHasRequestedRefillTooSoon:
			strResponseDenyCode = "PatientHasRequestedRefillTooSoon";
			break;
		case ncrdctMedicationNeverPrescribedForThePatient:
			strResponseDenyCode = "MedicationNeverPrescribedForThePatient";
			break;
		case ncrdctRefillNotAppropriate:
			strResponseDenyCode = "RefillNotAppropriate";
			break;
		case ncrdctPatientHasPickedUpPrescription:
			strResponseDenyCode = "PatientHasPickedUpPrescription";
			break;
		case ncrdctPatientHasPickedUpPartialFillOfPrescription:
			strResponseDenyCode = "PatientHasPickedUpPartialFillOfPrescription";
			break;
		case ncrdctPatientHasNotPickedUpPrescriptionDrugReturnedToStock:
			strResponseDenyCode = "PatientHasNotPickedUpPrescriptionDrugReturnedToStock";
			break;
		case ncrdctChangeNotAppropriate:
			strResponseDenyCode = "ChangeNotAppropriate";
			break;
		case ncrdctPatientNeedsAppointment:
			strResponseDenyCode = "PatientNeedsAppointment";
			break;
		case ncrdctPrescriberNotAssociatedWithThisPracticeOrLocation:
			strResponseDenyCode = "PrescriberNotAssociatedWithThisPracticeOrLocation";
			break;
		case ncrdctNoAttemptWillBeMadeToObtainPriorAuthorization:
			strResponseDenyCode = "NoAttemptWillBeMadeToObtainPriorAuthorization";
			break;
		case ncrdctDeniedNewPrescriptionToFollow:
			strResponseDenyCode = "DeniedNewPrescriptionToFollow";
			break;
		case ncrctInvalid:
		default:
			strResponseDenyCode = "";
			break;
	}

	if(!strResponseDenyCode.IsEmpty()) {
		CString strResponseDenyCodeXML = GetXMLElementValuePair_NewCrop("responseDenyCode", strResponseDenyCode);
		strPrescriptionRenewalResponse += strResponseDenyCodeXML;
	}

	if(!strMessageToPharmacist.IsEmpty()) {
		CString strMessageToPharmacistXML = GetXMLElementValuePair_NewCrop("messageToPharmacist", strMessageToPharmacist);
		strPrescriptionRenewalResponse += strMessageToPharmacistXML;
	}

	strXmlDocument = GetXMLElementValuePair_NewCrop("PrescriptionRenewalResponse", strPrescriptionRenewalResponse, TRUE);

	return TRUE;
}


// (j.jones 2009-02-27 14:59) - PLID 33163 - added function to get the RxEntry URL for production or pre-production
// The RxEntry URL is used for the web-based click-through interface.
CString GetNewCropRxEntryURL()
{
	//throw exceptions to the caller

	// (j.jones 2010-06-09 11:04) - PLID 39013 - production status is calculated by GetNewCropIsProduction()
	BOOL bIsProduction = GetNewCropIsProduction();

	if(!bIsProduction) {
		//return the preproduction URL
		return "https://preproduction.newcropaccounts.com/InterfaceV7/RxEntry.aspx";
	}
	else {
		//return the live, production URL
		return "https://secure.newcropaccounts.com/InterfaceV7/RxEntry.aspx";
	}
}

// (j.gruber 2009-03-02 16:34) - PLID 33273 - added xml schema validation
// (a.walling 2009-04-07 13:20) - PLID 33306 - const CString reference
BOOL ValidateNewCropXMLSchemas(const CString &strXML) {

	//create our array
	CPtrArray pSchemaArray;

	SchemaType *pSchema = new SchemaType;

	
	pSchema->nResourceID = IDR_NEWCROP_NCSTANDARD_XSD;
	pSchema->strNameSpace = "http://secure.newcropaccounts.com/interfaceV7:NCStandard";
	pSchema->strResourceFolder = "XSD";

	pSchemaArray.Add(pSchema);

	
	//now the second one
	pSchema = new SchemaType;
	pSchema->nResourceID = IDR_NEWCROP_NCSCRIPT_XSD;
	pSchema->strNameSpace = "http://secure.newcropaccounts.com/interfaceV7";
	pSchema->strResourceFolder = "XSD";

	pSchemaArray.Add(pSchema);	

	//throws its own errors and returns false

	BOOL bReturn = FALSE;
	try {
		bReturn = ValidateXMLWithSchema(strXML, &pSchemaArray);
	}NxCatchAllCall("Error Validating XML",
		//delete the array
		for(int i = 0; i < pSchemaArray.GetSize(); i++) {
			SchemaType* pSchema= (SchemaType*)pSchemaArray.GetAt(i);
			if(pSchema)
				delete pSchema;
		}
		pSchemaArray.RemoveAll();
		return FALSE;
	);


	//now delete the array
	for(int i = 0; i < pSchemaArray.GetSize(); i++) {
		SchemaType* pSchema= (SchemaType*)pSchemaArray.GetAt(i);
		if(pSchema)
			delete pSchema;
	}
	pSchemaArray.RemoveAll();

	return bReturn;
}

// (j.jones 2009-03-04 10:12) - PLID 33163 - function to validate that a state code is a US state or territory
BOOL VerifyIsNewCropValidState(CString strState)
{
	strState.MakeUpper();

	if(strState.GetLength() != 2) {
		//not valid
		return FALSE;
	}

	CString strLeft = strState.Left(1);
	CString strRight = strState.Right(1);

	//building off of NewCrop's list, which supports territories and Army postal codes
	if(strLeft == "A") {		
		if(strRight == "L"
			|| strRight == "K"
			|| strRight == "S"
			|| strRight == "Z"
			|| strRight == "R"
			|| strRight == "A"
			|| strRight == "P") {
			return TRUE;
		}
	}
	else if(strLeft == "C") {
		if(strRight == "A"
			|| strRight == "O"
			|| strRight == "T") {
			return TRUE;
		}
	}
	if(strLeft == "D") {
		if(strRight == "E"
			|| strRight == "C") {
			return TRUE;
		}
	}
	if(strLeft == "F") {
		if(strRight == "L"
			|| strRight == "M") {
			return TRUE;
		}
	}
	if(strLeft == "G") {
		if(strRight == "A"
			|| strRight == "U") {
			return TRUE;
		}
	}
	if(strLeft == "H") {
		if(strRight == "I") {
			return TRUE;
		}
	}
	if(strLeft == "I") {
		if(strRight == "A"
			|| strRight == "D"
			|| strRight == "L"
			|| strRight == "N") {
			return TRUE;
		}
	}
	if(strLeft == "K") {
		if(strRight == "S"
			|| strRight == "Y") {
			return TRUE;
		}
	}
	if(strLeft == "L") {
		if(strRight == "A") {
			return TRUE;
		}
	}
	if(strLeft == "M") {
		if(strRight == "A"
			|| strRight == "D"
			|| strRight == "E"
			|| strRight == "H"
			|| strRight == "I"
			|| strRight == "N"
			|| strRight == "O"
			|| strRight == "P"
			|| strRight == "S"
			|| strRight == "T") {
			return TRUE;
		}
	}
	if(strLeft == "N") {		
		if(strRight == "C"
			|| strRight == "D"
			|| strRight == "E"
			|| strRight == "H"
			|| strRight == "J"
			|| strRight == "M"
			|| strRight == "V"
			|| strRight == "Y") {
			return TRUE;
		}
	}
	if(strLeft == "O") {
		if(strRight == "H"
			|| strRight == "K"
			|| strRight == "R") {
			return TRUE;
		}
	}
	if(strLeft == "P") {
		if(strRight == "A"
			|| strRight == "R"
			|| strRight == "W") {
			return TRUE;
		}
	}
	if(strLeft == "R") {
		if(strRight == "I") {
			return TRUE;
		}
	}
	if(strLeft == "S") {
		if(strRight == "C"
			|| strRight == "D") {
			return TRUE;
		}
	}
	if(strLeft == "T") {
		if(strRight == "N"
			|| strRight == "X") {
			return TRUE;
		}
	}
	if(strLeft == "U") {
		if(strRight == "T") {
			return TRUE;
		}
	}
	if(strLeft == "V") {
		if(strRight == "A"
			|| strRight == "I"
			|| strRight == "T") {
			return TRUE;
		}
	}
	if(strLeft == "W") {
		if(strRight == "A"
			|| strRight == "I"
			|| strRight == "V"
			|| strRight == "Y") {
			return TRUE;
		}
	}

	//not valid
	return FALSE;
}

// (j.gruber 2009-03-31 11:43) - PLID 33328 - check whether the user has the corret role
// (j.gruber 2009-06-08 10:34) - PLID 34515 - added role to return
BOOL CheckEPrescribingStatus(NewCropUserTypes &ncuTypeID, long nUserID, CString strUserName) {

	try {
		_RecordsetPtr rsUser = CreateParamRecordset("SELECT NewCropUserTypeID FROM UsersT WHERE PersonID = {INT}", nUserID);

		if (rsUser->eof) {
			//wha?  we couldn't find the user
			ASSERT(FALSE);
			ThrowNxException("Error in CheckEPrescribingStatus, could not find userID");
			return FALSE;
		}
		else {

			NewCropUserTypes ncCurrentUserType = (NewCropUserTypes)AdoFldLong(rsUser, "NewCropUserTypeID", 0);

			// (j.jones 2009-08-18 16:23) - PLID 35203 - supported Midlevel Prescriber
			if (ncCurrentUserType == ncutLicensedPrescriber || ncCurrentUserType == ncutStaff_Nurse
				|| ncCurrentUserType == ncutMidlevelProvider) {
				//we are good
				ncuTypeID = ncCurrentUserType;
				return TRUE;
			}			
			else {
				//can't continue
				MsgBox("The user <%s> does not have access.  You can set user roles for E-Prescribing by going to Tools->Electronic Prescription Settings.\n Note: You must be an administrator to access the E-Prescribing setup.", strUserName);
				return FALSE;
			}
		}
	}NxCatchAll("Error in CheckEPrescribingStatus");
	return FALSE;
}

// (j.jones 2009-08-18 17:37) - PLID 35203 - added simple function to determine whether a user is prescriber type
BOOL IsNewCropPrescriberRole(NewCropUserTypes ncuTypeID)
{
	//If this function is changed in the future, be sure to search for all uses of the function
	//to ensure any successive messageboxes that mention the roles by name include your new role.

	if(ncuTypeID == ncutLicensedPrescriber || ncuTypeID == ncutMidlevelProvider) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (j.jones 2009-08-25 08:34) - PLID 35203 - added function to determined whether a user is a "supervised" type,
// in that it can have a supervising provider
BOOL IsNewCropSupervisedRole(NewCropUserTypes ncuTypeID)
{
	//If this function is changed in the future, be sure to search for all uses of the function
	//to ensure any successive messageboxes that mention the roles by name include your new role.

	if(ncuTypeID == ncutStaff_Nurse || ncuTypeID == ncutMidlevelProvider) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (j.jones 2009-12-18 11:21) - PLID 36391 - added validation for a person's prefix
BOOL VerifyIsNewCropValidNamePrefix(CString strPrefix)
{
	strPrefix.MakeUpper();

	//these are taken from the NewCrop XML schema at:
	//http://preproduction.newcropaccounts.com/InterfaceV7/NCStandard.xsd

	if(strPrefix == "MS.") {
		return TRUE;
	}
	else if(strPrefix == "MS") {
		return TRUE;
	}
	else if(strPrefix == "MISS") {
		return TRUE;
	}
	else if(strPrefix == "MR") {
		return TRUE;
	}
	else if(strPrefix == "MR.") {
		return TRUE;
	}
	else if(strPrefix == "MRS") {
		return TRUE;
	}
	else if(strPrefix == "MRS.") {
		return TRUE;
	}
	else if(strPrefix == "DR") {
		return TRUE;
	}
	else if(strPrefix == "DR.") {
		return TRUE;
	}
	else if(strPrefix == "SR") {
		return TRUE;
	}
	else if(strPrefix == "SR.") {
		return TRUE;
	}
	else if(strPrefix == "SRA") {
		return TRUE;
	}
	else if(strPrefix == "SRA.") {
		return TRUE;
	}

	//if we get here, it's invalid
	return FALSE;
}

// (j.jones 2009-12-18 11:21) - PLID 36391 - added validation for a person's suffix (PersonT.Title)
BOOL VerifyIsNewCropValidNameSuffix(CString strSuffix)
{
	strSuffix.MakeUpper();

	//these are taken from the NewCrop XML schema at:
	//http://preproduction.newcropaccounts.com/InterfaceV7/NCStandard.xsd

	//these compare as uppercase

	// (j.jones 2011-11-21 15:45) - PLID 46555 - added DPM, PAC, CNS, and RD
	if(strSuffix == "DDS" || strSuffix == "DO"|| strSuffix == "JR"|| strSuffix == "LVN"
		|| strSuffix == "MD" || strSuffix == "NP" || strSuffix == "PA"|| strSuffix == "RN"
		|| strSuffix == "SR"|| strSuffix == "I"|| strSuffix == "II"|| strSuffix == "III"
		|| strSuffix == "PHD" || strSuffix == "PHARMD" || strSuffix == "RPH"
		|| strSuffix == "MA" || strSuffix == "OD" || strSuffix == "CNP"
		|| strSuffix == "CNM" || strSuffix == "RPAC" || strSuffix == "FACC"
		|| strSuffix == "FACP" || strSuffix == "LPN" || strSuffix == "JR." || strSuffix == "SR."
		|| strSuffix == "ESQ." || strSuffix == "ESQ" || strSuffix == "IV" || strSuffix == "DPM"
		|| strSuffix == "PAC" || strSuffix == "CNS" || strSuffix == "RD") {
		
		return TRUE;
	}

	//if we get here, it's invalid
	return FALSE;
}

// (j.jones 2010-06-09 11:09) - PLID 39013 - gets the production status, which most of the time
// is hard coded on for clients, off for NexTech, but can be overridden
BOOL GetNewCropIsProduction()
{
	BOOL bIsProduction = TRUE;

	//if the license is 70116 (NexTech development) or 72977 (NexTech sales),
	//make it always be NOT production
	// (j.jones 2010-08-27 08:33) - PLID 40230 - At NewCrop's demand, we made the 72977 license key default to production,
	// and not warn at all. Per NewCrop, only development should use pre-production. Sales should just prescribe to a dummy
	// pharmacy.
	int nLicenseKey = g_pLicense->GetLicenseKey();
	//if development only
	if(nLicenseKey == 70116) {
		bIsProduction = FALSE;
	}

	//if debug mode, always default to false
#ifdef _DEBUG
	bIsProduction = FALSE;
#endif

	//now check our override ConfigRT setting, which should rarely be set
	//(e.lally 2011-08-26) PLID 44950 - Auto-create the pref so we don't have the added round-trip the first time every login.
	// (j.jones 2012-04-10 16:51) - PLID 49561 - This logic depended on never auto-creating. It would never be pre-production
	// unless you're in debug mode or using 70116, but developers were creating this as FALSE on client data when they ran
	// Practice on converted data. The only solution is to make it be tri-state such that the default value means no override.

	//This property now returns -2 if no override is in place, 0 if it is forced pre-production, 1 if forced production.
	long nProductionOverride = GetRemotePropertyInt("NewCrop_ProductionStatusOverride", -2, 0, "<None>", true);
	if(nProductionOverride == 0) {
		//forced pre-production
		return FALSE;
	}
	else if(nProductionOverride == 1) {
		//forced production
		return TRUE;
	}
	else {
		//default, return our pre-calculated value
		return bIsProduction;
	}
}

// (j.jones 2011-06-20 09:20) - PLID 44127 - EnsureUniqueDEANumberForProvider will optionally take in
// a provider ID, and if they have a licensed prescriber NewCrop role it will check to see
// if any other licensed prescriber has the same DEA. Returns TRUE if the provider's DEA is unique.
// Can optionally take in a -1 provider ID to find all duplicate DEA numbers for licensed prescribers.
BOOL EnsureUniqueDEANumber(IN OUT CString &strInvalidProviders, long nProviderID /*= -1*/)
{
	CSqlFragment sqlProvIDFilter("");

	if(nProviderID != -1) {
		CSqlFragment sqlProvID(" AND ProvidersT.PersonID = {INT}", nProviderID);
		sqlProvIDFilter = sqlProvID;
	}

	CMap<long, long, BOOL, BOOL> mapDupedProviders;

	//given a provider ID, see if this provider is a licensed prescriber, and if so, do they
	//have the same DEA as any other licensed prescriber

	// (j.jones 2012-01-31 14:27) - PLID 47874 - 'None' is now permitted to be sent, and duplicates
	// of this word only are permitted (any case, though we force an Upper comparison for safety)
	_RecordsetPtr rs = CreateParamRecordset("SELECT "
		"ProvidersT.PersonID, OtherProviderQ.PersonID AS OtherProvID, "
		"Last + ', ' + First + ' ' + Middle AS ProvName, "
		"OtherProviderQ.ProvName AS OtherProvName, "
		"[DEA Number] AS DEANumber "
		"FROM ProvidersT "
		"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"INNER JOIN ("
		"	SELECT ProvidersT.PersonID, Last + ', ' + First + ' ' + Middle AS ProvName, "
		"	[DEA Number] AS DEANumber "
		"	FROM ProvidersT "
		"	INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		"	WHERE ProvidersT.NewCropRole = {CONST} "
		"	AND RTRIM(LTRIM([DEA Number])) <> '' AND Upper(RTRIM(LTRIM([DEA Number]))) <> 'NONE' "
		") AS OtherProviderQ ON Upper(OtherProviderQ.DEANumber) = Upper(ProvidersT.[DEA Number]) "
		"AND OtherProviderQ.PersonID <> ProvidersT.PersonID "
		"WHERE ProvidersT.NewCropRole = {CONST} "
		"AND RTRIM(LTRIM([DEA Number])) <> '' AND Upper(RTRIM(LTRIM([DEA Number]))) <> 'NONE' "
		"{SQL} "
		"ORDER BY (Last + ', ' + First + ' ' + Middle), OtherProviderQ.ProvName",
		ncprLicensedPrescriber,
		ncprLicensedPrescriber, sqlProvIDFilter);

	BOOL bHasDuplicate = (rs->GetRecordCount() > 0);

	while(!rs->eof) {

		//don't put duplicates in the list
		long nFirstProvID = VarLong(rs->Fields->Item["PersonID"]->Value);
		long nOtherProvID = VarLong(rs->Fields->Item["OtherProvID"]->Value);

		BOOL bFound1 = FALSE;
		mapDupedProviders.Lookup(nFirstProvID, bFound1);
		BOOL bFound2 = FALSE;
		mapDupedProviders.Lookup(nOtherProvID, bFound2);

		if(bFound1 && bFound2) {
			//both providers are already in our list, so skip this record
			rs->MoveNext();
			continue;
		}

		if(!bFound1) {
			//map the first provider
			mapDupedProviders.SetAt(nFirstProvID, TRUE);
		}

		if(!bFound2) {
			//map the second provider
			mapDupedProviders.SetAt(nOtherProvID, TRUE);
		}

		//add the names to the list
		CString strProviderName = VarString(rs->Fields->Item["ProvName"]->Value, "");
		CString strOtherProviderName = VarString(rs->Fields->Item["OtherProvName"]->Value, "");
		CString strDEANumber = VarString(rs->Fields->Item["DEANumber"]->Value, "");

		CString str;
		str.Format("- %s and %s have the same DEA Number of: %s\n", strProviderName, strOtherProviderName, strDEANumber);
		strInvalidProviders += str;

		rs->MoveNext();
	}
	rs->Close();

	return !bHasDuplicate;
}