////////////////
// DRT 8/6/03 - GetSqlPatients() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"

CString CReportInfo::GetSqlPatients(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql, strFormatString;

	// (f.dinatale 2010-10-15) - PLID 40876 - SSN Masking Permissions
	BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

	switch (nID) {
	case 14:
		// Patient List
		/*	Version History
			1/16/03 - DRT - Added Currentstatus as an extended filter field, so now you can choose patient, prospect, or pt/pros, instead of being
					forced into just using patients.

			1/20/03 - TES - Added code to get the "format string" for phone numbers.
			DRT 7/30/03 - Added Privacy fields
			// (j.jones 2009-10-19 16:47) - PLID 35994 - split race and ethnicity
			// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
			// (f.dinatale 2010-12-15) - PLID 40876 - Fixed an issue which was due to the phone formatting to be put into the MaskSSN procedure call.
			// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure, changed to practice name field
		*/
		if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
			strFormatString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
		}
		else {
			strFormatString = "";
		}
		strSQL.Format("SELECT PatientsT.UserDefinedID, " 
                "  PersonT.ID AS PatID,  PersonT.Location as LocID, "
          		"	ProvidersT.PersonID AS ProvID, "
				"	PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle "
				"	AS PatName, "
		 		"	PersonDoctor.Last + ', ' + PersonDoctor.First + ' ' + PersonDoctor.Middle "
          		"	AS ProvName, "
				"	PersonT.FirstContactDate AS Date, "
          		"	PatientsT.PersonID, PatientsT.CurrentStatus AS CurrentStatus, "
          		"	PatientsT.MaritalStatus, PatientsT.SpouseName, " 
          		"	PatientsT.Occupation, PatientsT.EmployeeID,  "
          		"	PatientsT.ReferralID,  "
          		"	PatientsT.MainPhysician, PersonT.Company, "
          		"	PatientsT.EmployerFirst, PatientsT.EmployerMiddle, "
          		"	PatientsT.EmployerLast, PatientsT.EmployerAddress1, "
          		"	PatientsT.EmployerAddress2, PatientsT.EmployerCity, "
          		"	PatientsT.EmployerState, PatientsT.EmployerZip, "
          		"	PatientsT.TypeOfPatient,  "
          		"	PatientsT.DefaultReferringPhyID,  "
				"	PersonT.EmergRelation, PersonT.EmergHPhone, "
          		"	PersonT.EmergWPhone, PersonT.Archived, "
          		"	PersonT.First, PersonT.Middle, "
          		"	PersonT.Last, PersonT.Address1, "
          		"	PersonT.Address2, PersonT.City, "
          		"	PersonT.State, PersonT.Zip, "
          		"	PersonT.Gender, PrefixT.Prefix,  "
          		"	PersonT.Suffix, PersonT.Title, "
          		"	PersonT.HomePhone, PersonT.WorkPhone, "
          		"	PersonT.Extension, PersonT.CellPhone, "
          		"	PersonT.OtherPhone, PersonT.Email, "
          		"	PersonT.Pager, PersonT.Fax, "
				"	PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
         		"	PersonT.FirstContactDate, "
				"	PersonT.UserID, "
				"	PersonT.Note, "
				"	PersonT.PrivHome, PersonT.PrivWork,  "
				"	Convert(nvarchar(50), '%s') AS FormatString, PersonT.PrivFax, PersonT.PrivOther, PersonT.PrivPager, PersonT.PrivEmail, "
				// (b.spivey, May 28, 2013) - PLID 56871 
				"	LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS Race, "
				"	LEFT(OfficialRaceSubQ.OfficialName, LEN(OfficialRaceSubQ.OfficialName) -1) AS CDCRace, "
				"	EthnicityT.Name AS CDCEthnicity "
				"	FROM PersonT "
				"	LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"	LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID "
				"	LEFT JOIN PersonT PersonDoctor ON ProvidersT.PersonID = PersonDoctor.ID "
          		"	LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
				// (b.spivey, May 28, 2013) - PLID 56871 - creates the list of CDC and custom races. 
				"	CROSS APPLY "
				"	( "
				"		SELECT ( "
				"			SELECT RT.Name + ', ' "
				"			FROM PersonRaceT PRT "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
				"			WHERE PRT.PersonID = PersonT.ID "
				"			FOR XML PATH(''), TYPE "
				"		).value('/', 'nvarchar(max)') "
				"	) RaceSubQ (RaceName) "
				"	CROSS APPLY "
				"	( "
				"		SELECT ( "
				"			SELECT RCT.OfficialRaceName + ', ' "
				"			FROM PersonRaceT PRT "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID "
				"			WHERE PRT.PersonID = PersonT.ID  "
				"			FOR XML PATH(''), TYPE "
				"		).value('/', 'nvarchar(max)') "
				"	) OfficialRaceSubQ (OfficialName) "
				"	LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID "
				"	LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID "
				"	WHERE PatientsT.CurrentStatus <> 4", ((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), 
				 _Q(strFormatString));
		return _T(strSQL);
		break;
		

	/*case 4:
		//Individual Patient Notes
		return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
		"    Notes.Date AS Date, Notes.Date AS Time, NoteCatsF.Description AS Category, Notes.Note, LocationsT.Name AS Practice, LocationsT.ID AS LocID, PatientsT.MainPhysician AS ProvID "
		"FROM PersonT LEFT OUTER JOIN "
		"    LocationsT ON  "
		"    PersonT.Location = LocationsT.ID RIGHT OUTER JOIN "
		"    PatientsT ON  "
		"    PersonT.ID = PatientsT.PersonID LEFT OUTER JOIN "
		"    Notes ON PatientsT.PersonID = Notes.PersonID LEFT JOIN "
		"	 NoteCatsF ON Notes.Category = NoteCatsF.ID "
		"WHERE (PatientsT.PersonID > 0) AND (Notes.Note IS NOT NULL)");
		break;*/

	
	case 25:
		//Patient Count By Zip Code
		return _T("SELECT CASE WHEN Right(PersonT.Zip,1) = '-' THEN Left(PersonT.Zip, Len(PersonT.Zip)-1) ELSE PersonT.Zip END AS ZipCode, MIN(PersonT.City) AS City,  "
		"    MIN(PersonT.State) AS State, COUNT(PatientsT.UserDefinedID)  "
		"    AS CountOfID, PatientsT.[MainPhysician] AS ProvID,  "
		"    PersonT.FirstContactDate AS Date,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, "
		"PersonT.Location AS LocID, LocationsT.Name AS Location "
		"FROM PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4)"
		"GROUP BY PersonT.Zip, PatientsT.[MainPhysician],  "
		"    PersonT.FirstContactDate, PatientsT.UserDefinedID, PatientsT.PersonID, PersonT.Location, LocationsT.Name "
		"HAVING (PersonT.Zip <> N'0')");
		break;

	case 42:
		//Patients By Appointments
		/*	Version History
			DRT 6/19/03 - Removed AptPurposeT from the FROM clause, added dbo.GetPurposeString() in the SELECT.
			TES 3/4/04 - Implemented multi-resource support (!)
		*/
		{
		CString strSql = "SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     AppointmentsT.StartTime "
		"    AS StartTime, AppointmentsT.EndTime "
		"    AS EndTime, dbo.GetPurposeString(AppointmentsT.ID) AS PurpName, AppointmentsT.Notes,  "
		"    AptTypeT.Name, ResourceT.ID AS ResourceID, dbo.GetResourceString(AppointmentsT.ID) AS Item, "
		"    AppointmentsT.Date AS Date,  "
		"    PatientsT.[MainPhysician] AS ProvID,  "
		"    AppointmentsT.Status, PersonT.Location AS LocID, LocationsT.Name AS Location "
		"FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
		"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
		"LEFT JOIN "
		"    AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID LEFT OUTER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    AppointmentsT.PatientID = PatientsT.PersonID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4) AND (AppointmentsT.Status <> 4)";
		return strSql;
		}
		break;

	case 49:
		//Prospect List
		/*	Version History
			1/16/03 - DRT - Removed.  Patient List now has capabilities to filter on your choice of status, making this report obsolete.
			// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
		*/
		strSQL.Format("SELECT PatientsT.UserDefinedID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT.FirstContactDate AS Date, PatientsT.PersonID AS PatID,  "
		"    PatientsT.[MaritalStatus], PatientsT.SpouseName,  "
		"    PatientsT.Occupation, PatientsT.EmployeeID,  "
		"    PatientsT.ReferralID, PatientsT.Nickname,   "
		"    PatientsT.[MainPhysician] AS ProvID, PersonT.Company,  "
		"    PatientsT.EmployerFirst, PatientsT.EmployerMiddle,  "
		"    PatientsT.EmployerLast, PatientsT.EmployerAddress1,  "
		"    PatientsT.EmployerAddress2, PatientsT.EmployerCity,  "
		"    PatientsT.EmployerState, PatientsT.EmployerZip,  "
		"    PatientsT.TypeOfPatient, PatientsT.DefaultInjuryDate,  "
		"	 DiagCodes1.CodeNumber AS DefaultICD9, "
		"	 DiagCodes2.CodeNumber AS DefaultICD92,  "
		"    DiagCodes3.CodeNumber AS DefaultICD93,  "
		"    DiagCodes4.CodeNumber AS DefaultICD94,  "
		"    PatientsT.FinancialNotes, PatientsT.SuppressStatement,  "
		"    PatientsT.DefaultReferringPhyID, PatientsT.MirrorID,  "
		"    PatientsT.ImageIndex, PatientsT.InformID, PersonT.EmergFirst,  "
		"    PersonT.EmergLast, PersonT.EmergRelation,  "
		"    PersonT.EmergHPhone, PersonT.EmergWPhone,  "
		"    PersonT.Archived, PersonT.First, PersonT.Middle,  "
		"    PersonT.Last, PersonT.Address1, PersonT.Address2,  "
		"    PersonT.City, PersonT.State, CASE WHEN Right(PersonT.Zip,1) = '-' THEN Left(PersonT.Zip, Len(PersonT.Zip)-1) ELSE PersonT.Zip END AS ZipCode, PersonT.Gender,  "
		"    PrefixT.Prefix, PersonT.Suffix, PersonT.Title,  "
		"    PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.Extension, PersonT.CellPhone, PersonT.OtherPhone,  "
		"    PersonT.Email, PersonT.Pager, PersonT.Fax,  "
		"    PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
		"    PersonT.FirstContactDate, PersonT.InputDate,  "
		"     PersonT.WarningMessage,  "
		"    PersonT.DisplayWarning, PersonT.Note, "
		"PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.PrivHome, PersonT.PrivWork "
		"FROM PatientsT INNER JOIN "
		"    ((PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) "
		"	 LEFT JOIN DiagCodes DiagCodes1 ON PatientsT.DefaultDiagID1 = DiagCodes1.ID "
		"	 LEFT JOIN DiagCodes DiagCodes2 ON PatientsT.DefaultDiagID2 = DiagCodes2.ID "
		"	 LEFT JOIN DiagCodes DiagCodes3 ON PatientsT.DefaultDiagID3 = DiagCodes3.ID "
		"	 LEFT JOIN DiagCodes DiagCodes4 ON PatientsT.DefaultDiagID4 = DiagCodes4.ID "
		"	LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus = 2)",
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));

		return _T(strSQL);
		break;
	

	case 58:
		//Custom Tab Check Boxes
		return _T("SELECT CustomFieldsT.Name, CustomFieldDataT.FieldID,  "
		"    CustomFieldDataT.IntParam AS Data, CustomFieldsT.Type,  "
		"    CustomFieldDataT.PersonID,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.FirstContactDate AS Date,  "
		"    PatientsT.MainPhysician AS ProvID,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.PrivHome, PersonT.PrivWork "
		"FROM PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
		"    PersonT PersonT1 ON  "
		"    PatientsT.MainPhysician = PersonT1.ID RIGHT OUTER JOIN "
		"    CustomFieldsT INNER JOIN "
		"    CustomFieldDataT ON  "
		"    CustomFieldsT.ID = CustomFieldDataT.FieldID ON  "
		"    PatientsT.PersonID = CustomFieldDataT.PersonID "
		"WHERE (CustomFieldDataT.FieldID > 40) AND  "
		"    (CustomFieldDataT.FieldID < 47) AND  "
		"    (CustomFieldsT.Type = 13) AND  "
		"    (CustomFieldDataT.IntParam <> 0)");
		break;

	case 59:
		//Custom Tab Combo Boxes
		/*	Version History
			// (d.thompson 2010-01-04) - PLID 10341 - Added an external filter on custom field
			// (j.armen 2011-06-27 17:00) - PLID 44253 - Updated to use new custom list data structure
		*/
		return _T("SELECT CustomFieldsT.Type, PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS "
		"     ProvName, PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.FirstContactDate AS Date,  "
		"    PatientsT.MainPhysician AS ProvID, CustomFieldsT.Name,  "
		"    CustomListDataT.FieldID AS CustomListFieldID, CustomListItemsT.Text,  "
		"    CustomListDataT.CustomListItemsID AS IntParam, PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.PrivHome, PersonT.PrivWork "
		"FROM CustomListItemsT LEFT OUTER JOIN "
		"    PatientsT INNER JOIN "
		"    CustomListDataT ON  "
		"    PatientsT.PersonID = CustomListDataT.PersonID INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
		"    PersonT PersonT1 ON  "
		"    PatientsT.MainPhysician = PersonT1.ID ON  "
		"    CustomListItemsT.ID = CustomListDataT.CustomListItemsID RIGHT OUTER "
		"     JOIN "
		"    CustomFieldsT ON  "
		"    CustomListDataT.FieldID = CustomFieldsT.ID "
		"WHERE (CustomFieldsT.Type = 11)");
		break;
	case 60:
		//Custom Tab Text Boxes
		// (a.walling 2007-07-03 09:44) - PLID 15491 - Added new custom text boxes
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"CustomFieldDataT.TextParam, "
		"CustomFieldsT.Name, "
		"PatientsT.[MainPhysician] AS ProvID, "
		"PersonT.FirstContactDate AS Date, PersonT.Location AS LocID, LocationsT.Name AS Location "
		"FROM ((PatientsT RIGHT JOIN (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID) INNER JOIN CustomFieldDataT ON PatientsT.PersonID = CustomFieldDataT.PersonID) INNER JOIN CustomFieldsT ON CustomFieldDataT.FieldID = CustomFieldsT.ID "
		"WHERE ( ( (CustomFieldDataT.FieldID>10 AND CustomFieldDataT.FieldID<17) OR (CustomFieldDataT.FieldID >= 90 AND CustomFieldDataT.FieldID <= 95) ) AND ((CustomFieldDataT.TextParam) Is Not Null) And ((CustomFieldDataT.TextParam) != ''))");
		break;
	case 61:
		//Custom Tab Date Boxes
		return _T("SELECT CustomFieldsT.Type, CustomFieldsT.Name,  "
		"    CustomFieldDataT.FieldID, CustomFieldDataT.DateParam,  "
		"    PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.FirstContactDate AS Date,  "
		"    PatientsT.MainPhysician AS ProvID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"     PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.PrivHome, PersonT.PrivWork "
		"FROM PersonT PersonT1 RIGHT JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) LEFT JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID ON  "
		"    PersonT1.ID = PatientsT.MainPhysician RIGHT OUTER JOIN "
		"    CustomFieldDataT ON  "
		"    PatientsT.PersonID = CustomFieldDataT.PersonID RIGHT OUTER "
		"     JOIN "
		"    CustomFieldsT ON  "
		"    CustomFieldDataT.FieldID = CustomFieldsT.ID "
		"WHERE (CustomFieldsT.Type = 21)");
		break;
	

	case 74:
		//Patients by Login
		/*	Version History
			1/17/03 - DRT - Added a pt/prospect/pt-pros extended filter, so people have more control over what they are seeing.  Removed patient-only
					filter from this query.
			DRT 7/30/03 - Added Privacy fields
			DRT 7/13/2004 - PLID 13439 - Reverified for warning field cutting off / editable issues.
			// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
			// (b.savon 2014-03-24 09:17) - PLID 61454 - Modify Patients by Login for ICD-10.
		*/
		strSQL.Format("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT.FirstContactDate AS Date, PatientsT.PersonID,  "
		"    PatientsT.CurrentStatus AS CurrentStatus, PatientsT.[MaritalStatus],  "
		"    PatientsT.SpouseName, PatientsT.Occupation,  "
		"    PatientsT.EmployeeID, PatientsT.ReferralID,  "
		"    PatientsT.Nickname,  "
		"    PatientsT.[MainPhysician] AS ProvID, PersonT.Company,  "
		"    PatientsT.EmployerFirst, PatientsT.EmployerMiddle,  "
		"    PatientsT.EmployerLast, PatientsT.EmployerAddress1,  "
		"    PatientsT.EmployerAddress2, PatientsT.EmployerCity,  "
		"    PatientsT.EmployerState, PatientsT.EmployerZip,  "
		"    PatientsT.TypeOfPatient, PatientsT.DefaultInjuryDate,  "
		"	 DiagCodes1.CodeNumber AS ICD9Code1, "
		"	 DiagCodes2.CodeNumber AS ICD9Code2,  "
		"    DiagCodes3.CodeNumber AS ICD9Code3,  "
		"    DiagCodes4.CodeNumber AS ICD9Code4,  "
		"	 DiagCodes1_ICD10.CodeNumber AS ICD10Code1, "
		"	 DiagCodes2_ICD10.CodeNumber AS ICD10Code2,  "
		"    DiagCodes3_ICD10.CodeNumber AS ICD10Code3,  "
		"    DiagCodes4_ICD10.CodeNumber AS ICD10Code4,  "
		"    PatientsT.FinancialNotes, PatientsT.SuppressStatement,  "
		"    PatientsT.DefaultReferringPhyID, PatientsT.MirrorID,  "
		"    PatientsT.ImageIndex, PatientsT.InformID, PersonT.EmergFirst,  "
		"    PersonT.EmergLast, PersonT.EmergRelation,  "
		"    PersonT.EmergHPhone, PersonT.EmergWPhone,  "
		"    PersonT.Archived, PersonT.First, PersonT.Middle,  "
		"    PersonT.Last, PersonT.Address1, PersonT.Address2,  "
		"    PersonT.City, PersonT.State, PersonT.Zip, PersonT.Gender,  "
		"    PrefixT.Prefix, PersonT.Suffix, PersonT.Title,  "
		"    PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.Extension, PersonT.CellPhone, PersonT.OtherPhone,  "
		"    PersonT.Email, PersonT.Pager, PersonT.Fax,  "
		"    PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity,  "
		"    PersonT.FirstContactDate, PersonT.InputDate,  "
		"    PersonT.WarningMessage, PersonT.DisplayWarning,  "
		"    PersonT.Note, UsersT.PersonID AS UserID,  "
		"    UsersT.UserName AS LoginName, PersonT.Location AS LocID, LocationsT.Name AS Location, "
		"	 PersonT.PrivFax, PersonT.PrivOther, PersonT.PrivPager, PersonT.PrivEmail "
		"FROM PatientsT INNER JOIN (PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) ON PatientsT.PersonID = PersonT.ID "
		"	 LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID "
		"	 LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
		"	 LEFT JOIN DiagCodes DiagCodes1 ON PatientsT.DefaultDiagID1 = DiagCodes1.ID "
		"	 LEFT JOIN DiagCodes DiagCodes2 ON PatientsT.DefaultDiagID2 = DiagCodes2.ID "
		"	 LEFT JOIN DiagCodes DiagCodes3 ON PatientsT.DefaultDiagID3 = DiagCodes3.ID "
		"	 LEFT JOIN DiagCodes DiagCodes4 ON PatientsT.DefaultDiagID4 = DiagCodes4.ID "
		"	 LEFT JOIN DiagCodes DiagCodes1_ICD10 ON PatientsT.DefaultICD10DiagID1 = DiagCodes1_ICD10.ID "
		"	 LEFT JOIN DiagCodes DiagCodes2_ICD10 ON PatientsT.DefaultICD10DiagID2 = DiagCodes2_ICD10.ID "
		"	 LEFT JOIN DiagCodes DiagCodes3_ICD10 ON PatientsT.DefaultICD10DiagID3 = DiagCodes3_ICD10.ID "
		"	 LEFT JOIN DiagCodes DiagCodes4_ICD10 ON PatientsT.DefaultICD10DiagID4 = DiagCodes4_ICD10.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4)",
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
		
		return _T(strSQL);
		break;


	case 75:
		//Prospects by Login
		/*	Version History
			1/17/03 - DRT - Removed report.  Patients by login has the option of filtering for this info.
			// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
		*/
		strSQL.Format("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"    PatientsT.CurrentStatus,  "
		"     PatientsT.[MainPhysician] AS ProvID,  "
		"     PersonT.Address1, PersonT.Address2,  "
		"    PersonT.City, PersonT.State, PersonT.Zip,  "
		"    PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
		"   PersonT.UserID AS UserID, "
		"     CASE WHEN UsersT.PersonID IS NULL THEN 'No User' ELSE UsersT.UserName END AS LoginName, PersonT.InputDate AS Date, PersonT.Location AS LocID, LocationsT.Name AS Location, "
		"PersonT.HomePhone, PersonT.WorkPhone, PersonT.PrivHome, PersonT.PrivWork "
		"FROM PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
		"    UsersT ON PersonT.UserID = UsersT.PersonID "
		"WHERE (PatientsT.CurrentStatus = 2) AND (PatientsT.PersonID > 0)", 
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));

		return _T(strSQL);
		break;
	

	case 78:
		//Patient Birth Dates
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PatientsT.MainPhysician AS ProvID,  "
		"    PersonDoctor.Last + ', ' + PersonDoctor.First + ' ' + PersonDoctor.Middle "
		"     AS ProvName,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT.BirthDate AS Date, PersonT.Address1, PersonT.Address2,  "
		"    PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, PersonT.Email,  "
		"    PatientsT.CurrentStatus, PersonT.Location AS LocID, LocationsT.Name AS Location, "
		"    Month(PersonT.BirthDate) * 100 + Day(PersonT.BirthDate) AS MonthDay "
		"FROM PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
		"    PersonT PersonDoctor ON  "
		"    PatientsT.MainPhysician = PersonDoctor.ID "
		"WHERE (PersonT.BirthDate IS NOT NULL)");
		break;
	

	case 88:
		//Patients By Zip Code
		// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
		strSQL.Format("SELECT CASE WHEN Right(PersonT.Zip,1) = '-' THEN Left(PersonT.Zip, Len(PersonT.Zip)-1) ELSE PersonT.Zip END AS ZipCode, PersonT.City, PersonT.State,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PatientsT.[MainPhysician] AS ProvID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"    PersonDoctor.Last + ', ' + PersonDoctor.First + ' ' + PersonDoctor.Middle AS ProvName, "
		"    PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
		"    PersonT.EmergFirst, PersonT.EmergLast, PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.FirstContactDate AS Date, PersonT.PrivHome, PersonT.PrivWork, PersonT.EmergHPhone "
		"FROM PatientsT INNER JOIN PersonT ON "
		"    PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN PersonT PersonDoctor ON "
		"    PatientsT.MainPhysician = PersonDoctor.ID LEFT OUTER JOIN LocationsT ON "
		"    PersonT.Location = LocationsT.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4)",
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));

		return _T(strSQL);
		break;

	case 110:
		//Patient To Do List
		/*	Version History
			DRT 3/11/03 - Added EnteredByName field.  There's now a new report that uses this same query, but groups by Assigned To
			MSC 3/12/03 - Added Email and HomePhone (both of the patient) fields
			DRT 7/21/03 - Added a "by entered by" report that uses this query
			TES 7/23/03 - Added option to run by Start Reminding Date
			DRT 5/26/2004 - PLID 12592 - Removed Inquiries
			DRT 7/13/2004 - PLID 13439 - Reverified all these reports for editing purposes / notes cutting off.
			(c.haag 2008-06-30 12:28) - PLID 30565 - Updated to new todo multi-assignee structure
			(b.spivey August 21, 2014) - PLID 57119 - Added BirthDate
		*/
		return _T("SELECT TaskType, Task, Priority AS Priority, UserDefinedID, SubQ.PatID AS PatID, NULL AS AssignID, "
		"SubQ.Date AS Date, Notes, Patient, BirthDate, SubQ.Done AS Done, SubQ.LocID AS LocID, SubQ.Location, SubQ.ProvID AS ProvID, AssignedTo, SubQ.StateID AS StateID, "
		"EnteredByName, SubQ.HomePhone AS HomePhone, SubQ.Email AS Email, SubQ.EnteredByID AS EnteredByID, "
		"SubQ.Remind AS RemindDate, TaskID, AssignFullName, AssignIDs "
		"FROM "
		"(SELECT NoteCatsF.Description AS TaskType, ToDoList.Task, "
		"CASE WHEN ToDoList.Priority = 1 THEN 'High' ELSE "
		"	CASE WHEN Priority = 2 THEN 'Medium' ELSE "
		"		CASE WHEN Priority = 3 THEN 'Low' END END END AS Priority,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignIDs,  "
		"    ToDoList.Deadline AS Date, ToDoList.Notes,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS Patient, "
		"		 PersonT1.BirthDate, "
		"     ToDoList.Done, "
		"PersonT1.Location AS LocID, LocationsT.Name AS Location, "
		"PatientsT.MainPhysician AS ProvID, dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedTo, "
		"CASE WHEN ToDoList.Done Is Null THEN 1 ELSE 2 END AS StateID, "
		"EnteredPersonT.Last + ', ' + EnteredPersonT.First + ' ' + EnteredPersonT.Middle AS EnteredByName, PersonT1.HomePhone AS HomePhone, PersonT1.Email AS Email, "
		"EnteredPersonT.ID AS EnteredByID, "
		"ToDoList.Remind, ToDoList.TaskID, "
		"dbo.GetTodoAssignToFullNamesString(ToDoList.TaskID) AS AssignFullName "
		"FROM ToDoList "
		"INNER JOIN PatientsT ON ToDoList.PersonID = PatientsT.PersonID INNER JOIN "
		"    (PersonT PersonT1 LEFT JOIN LocationsT ON PersonT1.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonT1.ID "
		"	 LEFT JOIN NoteCatsF ON ToDoList.CategoryID = NoteCatsF.ID LEFT JOIN "
		"	 UsersT AssignByT ON ToDoList.EnteredBy = AssignByT.PersonID LEFT JOIN PersonT EnteredPersonT ON ToDoList.EnteredBy = EnteredPersonT.ID "
		"	 WHERE PatientsT.CurrentStatus <> 4) AS SubQ");
		break;

	case 408:
		//Patient To Do List By Entered By
	case 419:
		//Patient To Do List By Category
		//Note:  Make sure you up the size of the 'Notes' field to memo / 2000 when making a .ttx file.
		/*	Version History
			DRT 3/11/03 - Added EnteredByName field.  There's now a new report that uses this same query, but groups by Assigned To
			MSC 3/12/03 - Added Email and HomePhone (both of the patient) fields
			DRT 7/21/03 - Added a "by entered by" report that uses this query
			TES 7/23/03 - Added option to run by Start Reminding Date
			DRT 5/26/2004 - PLID 12592 - Removed Inquiries
			DRT 7/13/2004 - PLID 13439 - Reverified all these reports for editing purposes / notes cutting off.
			(c.haag 2008-06-30 12:28) - PLID 30565 - Updated to new todo multi-assignee structure
			(b.spivey August 21, 2014) - PLID 57119 - Added BirthDate
		*/
		return _T("SELECT TaskType, Task, Priority AS Priority, UserDefinedID, SubQ.PatID AS PatID, NULL AS AssignID, "
		"SubQ.Date AS Date, Notes, Patient, BirthDate, SubQ.Done AS Done, SubQ.LocID AS LocID, SubQ.Location, SubQ.ProvID AS ProvID, AssignedTo, SubQ.StateID AS StateID, "
		"EnteredByName, SubQ.HomePhone AS HomePhone, SubQ.Email AS Email, SubQ.EnteredByID AS EnteredByID, "
		"SubQ.Remind AS RemindDate, TaskID, AssignFullName, AssignIDs "
		"FROM "
		"(SELECT NoteCatsF.Description AS TaskType, ToDoList.Task, "
		"CASE WHEN ToDoList.Priority = 1 THEN 'High' ELSE "
		"	CASE WHEN Priority = 2 THEN 'Medium' ELSE "
		"		CASE WHEN Priority = 3 THEN 'Low' END END END AS Priority,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignIDs,  "
		"    ToDoList.Deadline AS Date, ToDoList.Notes,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS Patient, "
		"		 PersonT1.BirthDate, "
		"     ToDoList.Done, "
		"PersonT1.Location AS LocID, LocationsT.Name AS Location, "
		"PatientsT.MainPhysician AS ProvID, dbo.GetTodoAssignToNamesString(ToDoList.TaskID) AS AssignedTo, "
		"CASE WHEN ToDoList.Done Is Null THEN 1 ELSE 2 END AS StateID, "
		"EnteredPersonT.Last + ', ' + EnteredPersonT.First + ' ' + EnteredPersonT.Middle AS EnteredByName, PersonT1.HomePhone AS HomePhone, PersonT1.Email AS Email, "
		"EnteredPersonT.ID AS EnteredByID, "
		"ToDoList.Remind, ToDoList.TaskID, "
		"dbo.GetTodoAssignToFullNamesString(ToDoList.TaskID) AS AssignFullName "
		"FROM ToDoList "
		"INNER JOIN PatientsT ON ToDoList.PersonID = PatientsT.PersonID INNER JOIN "
		"    (PersonT PersonT1 LEFT JOIN LocationsT ON PersonT1.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonT1.ID "
		"	 LEFT JOIN NoteCatsF ON ToDoList.CategoryID = NoteCatsF.ID LEFT JOIN "
		"	 UsersT AssignByT ON ToDoList.EnteredBy = AssignByT.PersonID LEFT JOIN PersonT EnteredPersonT ON ToDoList.EnteredBy = EnteredPersonT.ID "
		"	 WHERE PatientsT.CurrentStatus <> 4) AS SubQ");
		break;

	case 372:
		//Patient To Do List By Assign To
		//Note:  Make sure you up the size of the 'Notes' field to memo / 2000 when making a .ttx file.
		/*	Version History
			DRT 3/11/03 - Added EnteredByName field.  There's now a new report that uses this same query, but groups by Assigned To
			MSC 3/12/03 - Added Email and HomePhone (both of the patient) fields
			DRT 7/21/03 - Added a "by entered by" report that uses this query
			TES 7/23/03 - Added option to run by Start Reminding Date
			DRT 5/26/2004 - PLID 12592 - Removed Inquiries
			DRT 7/13/2004 - PLID 13439 - Reverified all these reports for editing purposes / notes cutting off.
			(c.haag 2008-06-30 12:28) - PLID 30565 - Updated to new todo multi-assignee structure
			(b.spivey August 21, 2014) - PLID 57119 - Added BirthDate
		*/
		return _T("SELECT TaskType, Task, Priority AS Priority, UserDefinedID, SubQ.PatID AS PatID, SubQ.AssignID AS AssignID, "
		"SubQ.Date AS Date, Notes, Patient, BirthDate, SubQ.Done AS Done, SubQ.LocID AS LocID, SubQ.Location, SubQ.ProvID AS ProvID, AssignedTo, SubQ.StateID AS StateID, "
		"EnteredByName, SubQ.HomePhone AS HomePhone, SubQ.Email AS Email, SubQ.EnteredByID AS EnteredByID, "
		"SubQ.Remind AS RemindDate, FullAssignName "
		"FROM "
		"(SELECT NoteCatsF.Description AS TaskType, ToDoList.Task, "
		"CASE WHEN ToDoList.Priority = 1 THEN 'High' ELSE "
		"	CASE WHEN Priority = 2 THEN 'Medium' ELSE "
		"		CASE WHEN Priority = 3 THEN 'Low' END END END AS Priority,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, TodoAssignToT.AssignTo AS AssignID,  "
		"    ToDoList.Deadline AS Date, ToDoList.Notes,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS Patient, "
		"		 PersonT1.BirthDate, "
		"     ToDoList.Done, "
		"PersonT1.Location AS LocID, LocationsT.Name AS Location, "
		"PatientsT.MainPhysician AS ProvID, UsersT.Username AS AssignedTo, "
		"CASE WHEN ToDoList.Done Is Null THEN 1 ELSE 2 END AS StateID, "
		"EnteredPersonT.Last + ', ' + EnteredPersonT.First + ' ' + EnteredPersonT.Middle AS EnteredByName, PersonT1.HomePhone AS HomePhone, PersonT1.Email AS Email, "
		"EnteredPersonT.ID AS EnteredByID, "
		"ToDoList.Remind, "
		"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullAssignName "
		"FROM ToDoList "
		"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = TodoList.TaskID "
		"INNER JOIN PatientsT ON ToDoList.PersonID = PatientsT.PersonID INNER JOIN "
		"    (PersonT PersonT1 LEFT JOIN LocationsT ON PersonT1.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonT1.ID INNER JOIN "
		"    UsersT ON TodoAssignToT.AssignTo = UsersT.PersonID LEFT JOIN PersonT ON TodoAssignToT.AssignTo = PersonT.ID "
		"	 LEFT JOIN NoteCatsF ON ToDoList.CategoryID = NoteCatsF.ID LEFT JOIN "
		"	 UsersT AssignByT ON ToDoList.EnteredBy = AssignByT.PersonID LEFT JOIN PersonT EnteredPersonT ON ToDoList.EnteredBy = EnteredPersonT.ID "
		"	 WHERE PatientsT.CurrentStatus <> 4) AS SubQ");
		break;

	case 163:
		//Patients By Patient Type
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonDoctor.Last + ', ' + PersonDoctor.First + ' ' + PersonDoctor.Middle "
		"     AS ProvName, PersonT.Address1, PersonT.Address2,  "
		"    PersonT.City, PersonT.State, PersonT.Zip,  "
		"    PersonT.HomePhone, PersonT.WorkPhone,  "
		"    GroupTypes.GroupName, PatientsT.MainPhysician AS ProvID,  "
		"    GroupTypes.TypeIndex AS TypeIndex,  "
		"    PersonT.FirstContactDate AS Date, "
		"PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.PrivHome, PersonT.PrivWork "
		"FROM PatientsT LEFT OUTER JOIN "
		"    GroupTypes ON  "
		"    PatientsT.TypeOfPatient = GroupTypes.TypeIndex LEFT OUTER JOIN "
		"    ContactsT RIGHT OUTER JOIN "
		"    PersonT PersonDoctor ON  "
		"    ContactsT.PersonID = PersonDoctor.ID ON  "
		"    PatientsT.MainPhysician = PersonDoctor.ID LEFT OUTER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4)");
		break;
	case 164:
		//Patient Custom Info (General One)
		/*	Version History
			DRT 7/27/2004 - PLID 13698 - Fixed it to no longer INNER JOIN on providers, effectively
				preventing any data for patients who didn't have a provider.
		*/
		return _T("SELECT PersonT.ID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     CustomFieldDataT.TextParam, CustomFieldsT.Name,  "
		"    PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.HomePhone,  "
		"    PersonT.WorkPhone, PersonT.FirstContactDate AS Date,  "
		"    PersonT1.ID AS ProvID,  "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, "
		"PatientsT.UserDefinedID as UID2, "
		"PersonT.Location AS LocID, "
		"LocationsT.Name AS Location "
		"FROM (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) INNER JOIN "
		"    PatientsT ON PersonT.ID = PatientsT.PersonID INNER JOIN "
		"    CustomFieldDataT ON  "
		"    PersonT.ID = CustomFieldDataT.PersonID INNER JOIN "
		"    CustomFieldsT ON  "
		"    CustomFieldDataT.FieldID = CustomFieldsT.ID LEFT JOIN "
		"    PersonT PersonT1 ON  "
		"    PatientsT.[MainPhysician] = PersonT1.ID "
		"WHERE (CustomFieldDataT.FieldID >= 1) AND  "
		"    (CustomFieldDataT.FieldID <= 5)");
		break;


	/*case 178:
		//Patients By Billing
		return _T("SELECT PatientsT.UserDefinedID AS PatientID,  "
		"    PatientInfo.Last + ', ' + PatientInfo.First + ' ' + PatientInfo.Middle AS "
		"     PatName,  "
		"    SUM(LineItemT.Amount * ChargesT.Quantity * (ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1) "
		"     * (CASE WHEN CPTModifierT.Multiplier Is Null THEN 1 ELSE CPTModifierT.Multiplier End)) AS BillInfo, BillsT.Description,  "
		"    DocInfo.ID AS ProvID,  "
		"    DocInfo.Last + ', ' + DocInfo.First + ' ' + DocInfo.Middle AS ProvName, "
		"     BillsT.Date AS Date, CPTModifierT.Number, LineItemT.Amount,  "
		"    ChargesT.TaxRate, PatientInfo.First, PatientInfo.Middle,  "
		"    PatientInfo.Last, DocInfo.First AS DocFirst,  "
		"    DocInfo.Middle AS DocMiddle, DocInfo.Last AS DocLast, "
		"PatientInfo.Location AS LocID, "
		"LocationsT.Name AS Location, PatientsT.PersonID AS PatID "
		"FROM PersonT PatientInfo LEFT OUTER JOIN "
		"LocationsT ON "
		"PatientInfo.Location = LocationsT.ID RIGHT OUTER JOIN "
		"ProvidersT LEFT OUTER JOIN "
		"PersonT DocInfo ON "
		"ProvidersT.PersonID = DocInfo.ID RIGHT OUTER JOIN "
		"PatientsT RIGHT OUTER JOIN "
		"LineItemT ON "
		"PatientsT.PersonID = LineItemT.PatientID RIGHT OUTER JOIN "
		"ChargesT LEFT OUTER JOIN "
		"CPTModifierT ON "
		"ChargesT.CPTModifier = CPTModifierT.Number ON "
		"LineItemT.ID = ChargesT.ID RIGHT OUTER JOIN "
		"BillsT ON ChargesT.BillID = BillsT.ID ON "
		"ProvidersT.PersonID = PatientsT.MainPhysician ON "
		"PatientInfo.ID = PatientsT.PersonID "
		"WHERE (BillsT.EntryType = 1) AND (BillsT.Deleted = 0) "
		"GROUP BY BillsT.Description, DocInfo.ID, BillsT.Date,  "
		"    CPTModifierT.Number, LineItemT.Amount, ChargesT.TaxRate,  "
		"    PatientInfo.First, PatientInfo.Middle, PatientInfo.Last,  "
		"    DocInfo.First, DocInfo.Middle, DocInfo.Last, PatientsT.UserDefinedID, PatientInfo.Location, LocationsT.Name, PatientsT.PersonID");
		break;*/
	

	case 183:
	case 240: //Patient Notes (PP) - Uses same .rpt file.
		//Patient Notes
		//NOTE:  Must set the note field to "memo" when creating a new ttx file for this report
		/*	Version History
			DRT 7/8/2004 - PLID 13376 - Added provider field for editing report.
			JMJ 7/26/2005 - PLID 15854 - Added address fields, ref phy, allergies, and insurance names
			e.lally 6/30/2006 - PLID 21295 - Because of incident 90104, we found an issue where this query can return
				the errors:"Could not insert a row larger than the page size into a hash table. 
				Resubmit the query with the ROBUST PLAN hint." which 
				turns out to be a bug that is fixed in SQL server 2005, but we must change our query as a workaround
				to support sql server 2000. One workaround we could have implemented was adding "OPTION (ROBUST PLAN)"
				to the end of the query, but that was not ideal because it more or less ignores the problem
				rather than address any part of it. So instead we got rid of 
				several left joins for one field each and made them sub-select statements. This is not a general
				rule for query improvements, but it just so happens to alleviate this particular problem!
				Doing a comparison on the before and after of this query shows that there is little or no speed 
				hit with this change.

		*/

		{

		CString strSql = "SELECT PersonT.First, PersonT.Middle, PersonT.Last, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
			"Notes.Date AS Date, CONVERT(varchar, Notes.Date, 14) AS Time, NoteCatsF.Description AS Category, Notes.Note, "
			"PersonT.Location AS LocID, "
			"(SELECT LocationsT.Name FROM LocationsT WHERE LocationsT.ID = PersonT.Location) AS Location, "
			"PatientsT.MainPhysician AS ProvID, "
			"(SELECT PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle "
			"	FROM PersonT PersonProv "
			"	WHERE PersonProv.ID = PatientsT.MainPhysician) AS ProvName, "
			"(SELECT PersonRefPhy.Last + ', ' + PersonRefPhy.First + ' ' + PersonRefPhy.Middle "
			"	FROM PersonT PersonRefPhy "
			"	WHERE PersonRefPhy.ID = PatientsT.DefaultReferringPhyID) AS ReferringPhysName, "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
			"PersonT.WorkPhone, PersonT.Extension, PersonT.CellPhone, "
			"(SELECT InsuranceCoT.[Name] FROM InsuranceCoT "
			"   INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"   WHERE InsuredpartyT.PatientID = PatientsT.PersonID AND InsuredpartyT.RespTypeID = 1) AS PrimaryInsCo, "
			"(SELECT InsuranceCoT.[Name] FROM InsuranceCoT "
			"   INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
			"   WHERE InsuredpartyT.PatientID = PatientsT.PersonID AND InsuredpartyT.RespTypeID = 2) AS SecondaryInsCo, "
			"dbo.GetAllergyList(PatientsT.PersonID) AS Allergies "
			"FROM PatientsT "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN Notes ON PatientsT.PersonID = Notes.PersonID "
			"LEFT JOIN NoteCatsF ON Notes.Category = NoteCatsF.ID "
			"WHERE (PatientsT.PersonID > 0) AND (Notes.Note IS NOT NULL) ";

		if(saExtraValues.GetSize()) {
			strSql += " AND ";
			strSql += saExtraValues[0];//This was passed in by NotesDlg.cpp
		}

		return _T(strSql);

		}

		break; 
	case 184:
		//Patient Notes by Category
		//NOTE:  Must set the note field to "memo" when creating a new ttx file for this report
		/*	Version History
			DRT 7/8/2004 - PLID 13376 - Added provider field for editing report.
			// (j.dinatale 2011-10-10 17:11) - PLID 45899 - added city, state, and patient coordinator name to the list of available fields
		*/
		return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
		"    Notes.Date AS Date, CONVERT(varchar,  "
		"    Notes.Date, 14) AS Time,  Notes.Note, NoteCatsF.Description AS Category, "
		"PersonT.Location AS LocID, LocationsT.Name AS Location, PatientsT.MainPhysician AS ProvID, Notes.Category AS CatID, "
		"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, "
		"PersonT.City, PersonT.State, PtCoord.First AS PtCoorFirst, PtCoord.Middle AS PtCoorMiddle, PtCoord.Last AS PtCoorLast "
		"FROM PatientsT LEFT OUTER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN "
		"    (Notes LEFT JOIN NoteCatsF ON Notes.Category = NoteCatsF.ID) ON PatientsT.PersonID = Notes.PersonID "
		"	LEFT JOIN PersonT PersonProv ON PatientsT.MainPhysician = PersonProv.ID "
		"	LEFT JOIN PersonT PtCoord ON PatientsT.EmployeeID = PtCoord.ID "
		"WHERE (PatientsT.PersonID > 0) AND (Notes.Note IS NOT NULL)");
		break;
	case 339:
		//Patient Notes by User
		//NOTE:  Must set the note field to "memo" when creating a new ttx file for this report
		/*	Version History
			DRT 7/8/2004 - PLID 13376 - Added provider field for editing report.
			DRT 1/11/2005 - PLID 15249 - Added field for patient status and a count of the active todo
				alarms for that user.  For internal use, not added into the default report.  Verified new ttx.
				Note that the count is *active* todos - filters out completed, remind times in the future, and also
				filters out inquiry pt types (currenstatus = 4).  Not sure if this is bad data or something required for all.
			DRT 1/12/2005 - PLID 15249 - Also ended up adding Address1, 2, City, State, Zip fields for editing.
			(c.haag 2008-06-30 12:28) - PLID 30565 - Updated to new todo multi-assignee structure
		*/
		return _T("SELECT PersonT.First, PersonT.Middle, PersonT.Last,  "
		"    PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  "
		"   Notes.Date AS Date, CONVERT(varchar,  "
		"    Notes.Date, 14) AS Time, NoteCatsF.Description AS Category, Notes.Note, "
		"PersonT.Location AS LocID, LocationsT.Name AS Location, PatientsT.MainPhysician AS ProvID, Notes.UserID AS UserID, UsersT.UserName, "
		"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, CASE WHEN GroupTypes.GroupName IS NULL THEN '<No Status>' ELSE GroupTypes.GroupName END AS PatientStatus, "
		"CASE WHEN TodoCountQ.Cnt IS NULL THEN 0 ELSE TodoCountQ.Cnt END AS UserActiveAlarmCount, PersonT.Address1, PersonT.Address2,  "
		"PersonT.State, PersonT.City, PersonT.Zip "
		"FROM PatientsT LEFT OUTER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN "
		"    Notes ON PatientsT.PersonID = Notes.PersonID LEFT JOIN "
		"	 NoteCatsF ON Notes.Category = NoteCatsF.ID "
		"	 LEFT JOIN UsersT ON Notes.UserID = UsersT.PersonID "
		"	LEFT JOIN PersonT PersonProv ON PatientsT.MainPhysician = PersonProv.ID "
		"	LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
		"	LEFT JOIN "
		"		(SELECT AssignTo, COUNT(*) AS Cnt FROM ToDoList "
		"		INNER JOIN TodoAssignToT ON ToDoList.TaskID = TodoAssignToT.TaskID "
		"		LEFT JOIN PatientsT ON ToDoList.PersonID = PatientsT.PersonID "
		"		WHERE Done IS NULL AND Remind <= GetDate() AND (PatientsT.CurrentStatus <> 4 or PatientsT.CurrentStatus IS NULL) "
		"		GROUP BY AssignTo "
		"		) TodoCountQ "
		"	ON UsersT.PersonID = TodoCountQ.AssignTo "
		"WHERE (PatientsT.PersonID > 0) AND (Notes.Note IS NOT NULL)");
		break; 
	case 185:
		//Patients By Referring Physician
		/*	Version History
			DRT 8/5/03 - Added summary option.  Renamed the original report to ...Dtld (PLID 5740)
		*/
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonPat.Last + ', ' + PersonPat.First + ' ' + PersonPat.Middle AS "
		"     PatName,  "
		"    PersonPhys.Last + ', ' + PersonPhys.First + ' ' + PersonPhys.Middle "
		"     AS RefPhysName, ReferringPhyST.PersonID AS RefPhysID,  "
		"    PersonPat.Address1, PersonPat.HomePhone,  "
		"    PersonPat.WorkPhone, PersonPat.City, PersonPat.State,  "
		"    PersonPat.Zip, "
		"PersonPat.Location AS LocID, LocationsT.Name AS Location, PersonPat.FirstContactDate AS Date, PatientsT.MainPhysician AS ProvID, PersonPat.PrivHome, PersonPat.PrivWork "
		"FROM PersonT PersonPhys LEFT OUTER JOIN "
		"    ContactsT ON  "
		"    PersonPhys.ID = ContactsT.PersonID LEFT OUTER JOIN "
		"    ReferringPhyST ON  "
		"    PersonPhys.ID = ReferringPhyST.PersonID RIGHT OUTER JOIN "
		"    PatientsT ON  "
		"    ReferringPhyST.PersonID = PatientsT.DefaultReferringPhyID LEFT "
		"     OUTER JOIN "
		"    (PersonT PersonPat LEFT JOIN LocationsT ON PersonPat.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonPat.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4)");
		break;
	
	case 373:
		//Patients By PCP
		/*	Version History
			DRT 3/11/03 - Copied from Patients by Referring Physician query
		*/
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonPat.Last + ', ' + PersonPat.First + ' ' + PersonPat.Middle AS "
		"     PatName,  "
		"    PersonPhys.Last + ', ' + PersonPhys.First + ' ' + PersonPhys.Middle "
		"     AS PCPName, ReferringPhyST.PersonID AS PCPID,  "
		"    PersonPat.Address1, PersonPat.HomePhone,  "
		"    PersonPat.WorkPhone, PersonPat.City, PersonPat.State,  "
		"    PersonPat.Zip, "
		"PersonPat.Location AS LocID, LocationsT.Name AS Location, PersonPat.FirstContactDate AS Date, PatientsT.MainPhysician AS ProvID, PersonPat.PrivHome, PersonPat.PrivWork "
		"FROM PersonT PersonPhys LEFT OUTER JOIN "
		"    ContactsT ON  "
		"    PersonPhys.ID = ContactsT.PersonID LEFT OUTER JOIN "
		"    ReferringPhyST ON  "
		"    PersonPhys.ID = ReferringPhyST.PersonID RIGHT OUTER JOIN "
		"    PatientsT ON  "
		"    ReferringPhyST.PersonID = PatientsT.PCP LEFT "
		"     OUTER JOIN "
		"    (PersonT PersonPat LEFT JOIN LocationsT ON PersonPat.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonPat.ID "
		"WHERE (PatientsT.PersonID > 0) AND (PatientsT.CurrentStatus <> 4)");
		break;

	case 192:
		//Deleted Patients
		// (j.jones 2010-02-04 14:40) - PLID 36500 - fixed Date alias
		// (f.dinatale 2011-02-01) - PLID 40876 - Added SSN Masking
		strSQL.Format("SELECT FirstName, MiddleName, LastName, CAST(dbo.MaskSSN(SS#, %s) AS NVARCHAR(15)) AS SS#, OldID, "
			"LoginName, PatientsDeleted.DeleteDate AS Date "
			"FROM PatientsDeleted", 
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
		return _T(strSQL);
		break;
	

	case 201:
		//Patients by Insurance Company
		/*	Version History
			(d.thompson 2009-12-17) - PLID 32578 - Fixed the ContactName, InsPhone, InsFax, and InsExt fields to actually pull from the contact.
			// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
			// (j.jones 2012-10-19 10:50) - PLID 51551 - stopped sending just a comma if the insurance contact name is blank,
			// also removed middle name because contacts don't have that field
		*/
		strSQL.Format("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,   "
			"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"     PersonT.Address1 AS PatAddr1,   "
			"    PersonT.Address2 AS PatAddr2, PersonT.City AS PatCity,   "
			"    PersonT.State AS PatState, PersonT.Zip AS PatZip,   "
			"    PersonT.HomePhone AS PatHPhone,   "
			"    PersonT.WorkPhone AS PatWPhone,   "
			"    PersonT.Extension AS PatExt, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity,  "
			"    InsuranceCoT.Name,   "
			"    InsuredPartyT.InsuranceCoID AS InsCoID,   "
			"    CASE WHEN Coalesce(PersonInsContact.Last, '') = '' AND Coalesce(PersonInsContact.First, '') = '' THEN '' "
			"    ELSE PersonInsContact.Last + CASE WHEN Coalesce(PersonInsContact.Last, '') <> '' AND Coalesce(PersonInsContact.First, '') <> '' THEN ', ' ELSE '' END + PersonInsContact.First END AS ContactName, "
			"    PersonInsContact.WorkPhone AS InsPhone,   "
			"    PersonInsContact.Extension AS InsExt, PersonInsContact.Fax AS InsFax,   "
			"    PersonT1.Address1 AS BillingAddress1,   "
			"    PersonT1.Address2 AS BillingAddress2,   "
			"    PersonT1.City AS BillingCity, PersonT1.State AS BillingState,   "
			"    PersonT1.Zip AS BillingZip, "
			"    InsuredPartyT.RespTypeID, "
			"	 RespTypeT.TypeName AS RespTypeName, "
			"    (CASE WHEN InsChargeRespQ.Amount Is Null THEN 0 ELSE InsChargeRespQ.Amount END) AS ChargeAmount,   "
			"    (CASE WHEN InsPaymentsQ.Amount Is Null THEN 0 ELSE InsPaymentsQ.Amount END) AS PayAmount,   "
			"    (CASE WHEN InsChargeRespQ.Amount Is Null THEN 0 ELSE InsChargeRespQ.Amount END) - (CASE WHEN InsPaymentsQ.Amount Is Null THEN 0 ELSE InsPaymentsQ.Amount END) AS InsBalance,  "
			"PersonT.Location AS LocID,  "
			"LocationsT.Name AS Location, PatientsT.MainPhysician AS ProvID, PersonT.FirstContactDate AS Date,  "
			"InsuredPartyT.IDForInsurance, InsuredPartyT.PolicyGroupNum, InsurancePlansT.PlanName, InsurancePlansT.PlanType, PersonT.PrivHome, PersonT.PrivWork "
			"FROM PersonT PersonT1 INNER JOIN  "
			"    InsuranceCoT ON   "
			"    PersonT1.ID = InsuranceCoT.PersonID RIGHT OUTER JOIN  "
			"        (SELECT PatientsT.PersonID, ChargeRespT.InsuredPartyID,   "
			"           SUM(CASE WHEN ChargeRespT.Amount Is Null THEN 0 ELSE ChargeRespT.Amount END) AS Amount  "
			"      FROM ChargeRespT RIGHT OUTER JOIN  "
			"           ChargesT ON   "
			"           ChargeRespT.ChargeID = ChargesT.ID RIGHT OUTER JOIN  "
			"           PatientsT LEFT OUTER JOIN  "
			"           BillsT ON PatientsT.PersonID = BillsT.PatientID ON   "
			"           ChargesT.BillID = BillsT.ID INNER JOIN LineItemT  "
			" 			ON ChargesT.ID = LineItemT.ID   "
			"	   WHERE LineItemT.Deleted = 0  "
			"      GROUP BY PatientsT.PersonID,   "
			"           ChargeRespT.InsuredPartyID  "
			"      HAVING (ChargeRespT.InsuredPartyID IS NOT NULL))   "
			"    InsChargeRespQ RIGHT OUTER JOIN  "
			"    InsuredPartyT ON   "
			"    InsChargeRespQ.InsuredPartyID = InsuredPartyT.PersonID RIGHT  "
			"     OUTER JOIN  "
			"    PatientsT LEFT OUTER JOIN  "
			"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID ON   "
			"    InsuredPartyT.PatientID = PatientsT.PersonID LEFT OUTER JOIN  "
			"        (SELECT PatientsT.PersonID, SUM(CASE WHEN LineItemT.Amount Is Null THEN 0 ELSE LineItemT.Amount END)   "
			"           AS Amount, PaymentsT.InsuredPartyID  "
			"      FROM PaymentsT RIGHT OUTER JOIN  "
			"           LineItemT ON   "
			"           PaymentsT.ID = LineItemT.ID RIGHT OUTER JOIN  "
			"           PatientsT ON   "
			"           LineItemT.PatientID = PatientsT.PersonID  "
			"	   WHERE LineItemT.Deleted = 0  "
			"      GROUP BY PatientsT.PersonID,   "
			"           PaymentsT.InsuredPartyID  "
			"      HAVING (PaymentsT.InsuredPartyID IS NOT NULL))   "
			"    InsPaymentsQ ON   "
			"    InsChargeRespQ.InsuredPartyID = InsPaymentsQ.InsuredPartyID  "
			"     AND   "
			"    InsChargeRespQ.PersonID = InsPaymentsQ.PersonID ON   "
			"    InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID  "
			"    LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"	 LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"	 LEFT JOIN InsuranceContactsT ON InsuredPartyT.InsuranceContactID = InsuranceContactsT.PersonID "
			"	 LEFT JOIN PersonT PersonInsContact ON InsuranceContactsT.PersonID = PersonInsContact.ID "
			"WHERE (PatientsT.UserdefinedID > 0) AND (PatientsT.CurrentStatus <> 4)",
			((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));

		return _T(strSQL);
		break;
	case 202:
		//Diagnosis Codes By Patient
		/*	Version History
			DRT 8/4/03 - Added external filter for icd9 code.  Any code you choose will filter patients who have
				that code as ANY of the 4 default codes.  See special cases in ExternalForm
			TES 3/26/2014 - PLID 61455 - Renamed to Diagnosis Codes By Patient, updated for ICD-10
		*/
		return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"    PatientsT.MainPhysician AS ProvID, "
		"	ICD9T1.CodeNumber as ICD9Code1, \r\n "
		"	ICD9T2.CodeNumber as ICD9Code2, \r\n "
		"	ICD9T3.CodeNumber as ICD9Code3, \r\n "
		"	ICD9T4.CodeNumber as ICD9Code4, \r\n "
		"	ICD10T1.CodeNumber as ICD10Code1, \r\n "
		"	ICD10T2.CodeNumber as ICD10Code2, \r\n "
		"	ICD10T3.CodeNumber as ICD10Code3, \r\n "
		"	ICD10T4.CodeNumber as ICD10Code4, \r\n "
		"	ICD9T1.CodeDesc as ICD9CodeDesc1, \r\n "
		"	ICD9T2.CodeDesc as ICD9CodeDesc2, \r\n "
		"	ICD9T3.CodeDesc as ICD9CodeDesc3, \r\n "
		"	ICD9T4.CodeDesc as ICD9CodeDesc4, \r\n "
		"	ICD10T1.CodeDesc as ICD10CodeDesc1, \r\n "
		"	ICD10T2.CodeDesc as ICD10CodeDesc2, \r\n "
		"	ICD10T3.CodeDesc as ICD10CodeDesc3, \r\n "
		"	ICD10T4.CodeDesc as ICD10CodeDesc4,  \r\n "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, "
		"PersonT.Location AS LocID, "
		"LocationsT.Name AS Location, PersonT.FirstContactDate AS Date "
		"FROM PatientsT LEFT OUTER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON  "
		"    PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN "
		"    PersonT PersonT1 ON  "
		"    PatientsT.MainPhysician = PersonT1.ID "
		" LEFT JOIN DiagCodes ICD9T1 ON PatientsT.DefaultDiagID1 = ICD9T1.ID \r\n"
		" LEFT JOIN DiagCodes ICD9T2 ON PatientsT.DefaultDiagID2 = ICD9T2.ID \r\n"
		" LEFT JOIN DiagCodes ICD9T3 ON PatientsT.DefaultDiagID3 = ICD9T3.ID \r\n"
		" LEFT JOIN DiagCodes ICD9T4 ON PatientsT.DefaultDiagID4 = ICD9T4.ID \r\n"
		" LEFT JOIN DiagCodes ICD10T1 ON PatientsT.DefaultICD10DiagID1 = ICD10T1.ID \r\n"
		" LEFT JOIN DiagCodes ICD10T2 ON PatientsT.DefaultICD10DiagID2 = ICD10T2.ID \r\n"
		" LEFT JOIN DiagCodes ICD10T3 ON PatientsT.DefaultICD10DiagID3 = ICD10T3.ID \r\n"
		" LEFT JOIN DiagCodes ICD10T4 ON PatientsT.DefaultICD10DiagID4 = ICD10T4.ID \r\n"
		"WHERE (PatientsT.PersonID > 0) AND  "
		"(   (PatientsT.DefaultDiagID1 IS NOT NULL) OR "
		"    (PatientsT.DefaultDiagID2 IS NOT NULL) OR "
		"    (PatientsT.DefaultDiagID3 IS NOT NULL) OR "
		"    (PatientsT.DefaultDiagID4 IS NOT NULL) OR "
		"	 (PatientsT.DefaultICD10DiagID1 IS NOT NULL) OR "
		"	 (PatientsT.DefaultICD10DiagID2 IS NOT NULL) OR "
		"	 (PatientsT.DefaultICD10DiagID3 IS NOT NULL) OR "
		"	 (PatientsT.DefaultICD10DiagID4 IS NOT NULL) "
		")");
		break;


	case 282:
		//Patients by Procedure
		return _T("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID,  "
			"ProcedureT.ID AS ProcID, ProcedureT.Name AS ProcName, LaddersT.FirstInterestDate AS Date, LaddersT.Notes,  "
			"LaddersT.Status, CASE WHEN LaddersT.Status = 2 OR StepTemplatesT.ID Is Null THEN 'Done' ELSE StepTemplatesT.StepName END AS Step, "
			"PersonT1.Last + PersonT1.First + PersonT1.Middle AS Provider, PersonT1.ID AS ProvID, PersonT.Location AS LocID, "
			" (CASE WHEN PersonT.BirthDate IS NOT NULL THEN (CASE WHEN PersonT.BirthDate < LaddersT.FirstInterestDate THEN YEAR(LaddersT.FirstInterestDate-1-PersonT.BirthDate)-1900 ELSE 0 END) ELSE NULL END) AS Age, PersonT.Zip, "
			" CASE WHEN ProcedureT.MasterProcedureID IS NULL THEN 1 ELSE ProcInfoDetailsT.Chosen END AS Chosen,  "
			" ProcedureT.MasterProcedureID, CASE WHEN MasterProcedureID IS NULL THEN '' ELSE (SELECT Name From ProcedureT ProcInner WHERE ProcInner.ID = ProcedureT.MasterProcedureID) END AS MasterProcName "
			"FROM "
			"(LaddersT LEFT JOIN (ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) INNER JOIN PatientsT ON LaddersT.PersonID = PatientsT.PersonID  "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN (SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.StepTemplateID Is Null THEN -1 ELSE StepsT.StepTemplateID END AS NextStep "
			"FROM LaddersT INNER JOIN ( "
			"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
			"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
			"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
			"GROUP BY LaddersT.ID "
			") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
			"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) NextStepQ ON LaddersT.ID = NextStepQ.LadderID  "
			"LEFT JOIN StepTemplatesT ON NextStepQ.NextStep = StepTemplatesT.ID "
			"LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID "
			"WHERE PatientsT.CurrentStatus <> 4 "
			" AND(ProcInfoDetailsT.Chosen = 1 OR (ProcedureT.MasterProcedureID Is Null AND NOT EXISTS  "
			" (SELECT ID FROM ProcInfoDetailsT OtherDetails WHERE OtherDetails.ProcInfoID = LaddersT.ProcInfoID AND Chosen = 1  "
			"	AND ProcedureID IN (SELECT ID FROM ProcedureT DetailProcs WHERE DetailProcs.MasterProcedureID =  "
			"	ProcedureT.ID)))) "
			"GROUP BY PersonT.ID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, PatientsT.UserDefinedID,  "
			"LaddersT.FirstInterestDate, LaddersT.Notes,  "
			"ProcedureT.ID, ProcedureT.Name, LaddersT.Status, CASE WHEN LaddersT.Status = 2 OR StepTemplatesT.ID Is Null THEN 'Done' ELSE StepTemplatesT.StepName END, "
			"PersonT1.Last + PersonT1.First + PersonT1.Middle, PersonT1.ID, PersonT.Location, PersonT.Zip, PersonT.BirthDate, ProcInfoDetailsT.Chosen, ProcedureT.MasterProcedureID"
			);
		break;

	case 285:
		//Patients by Coordinator
		// (f.dinatale 2010-10-15) - PLID 40876 - Added SSN Masking
		// (b.savon 2014-03-24 13:56) - PLID 61456 - Modify Patients by Coordinator for ICD-10.
		strSQL.Format("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT.FirstContactDate AS Date, PatientsT.PersonID,  "
		"    PatientsT.CurrentStatus, PatientsT.[MaritalStatus],  "
		"    PatientsT.SpouseName, PatientsT.Occupation,  "
		"    PatientsT.EmployeeID, PatientsT.ReferralID,  "
		"    PatientsT.Nickname,  "
		"    PatientsT.[MainPhysician] AS ProvID, PersonT.Company,  "
		"    PatientsT.EmployerFirst, PatientsT.EmployerMiddle,  "
		"    PatientsT.EmployerLast, PatientsT.EmployerAddress1,  "
		"    PatientsT.EmployerAddress2, PatientsT.EmployerCity,  "
		"    PatientsT.EmployerState, PatientsT.EmployerZip,  "
		"    PatientsT.TypeOfPatient, PatientsT.DefaultInjuryDate,  "
		"	 DiagCodes1.CodeNumber AS ICD9Code1, "
		"	 DiagCodes2.CodeNumber AS ICD9Code2,  "
		"    DiagCodes3.CodeNumber AS ICD9Code3,  "
		"    DiagCodes4.CodeNumber AS ICD9Code4,  "
		"	 DiagCodes1_ICD10.CodeNumber AS ICD10Code1, "
		"	 DiagCodes2_ICD10.CodeNumber AS ICD10Code2,  "
		"    DiagCodes3_ICD10.CodeNumber AS ICD10Code3,  "
		"    DiagCodes4_ICD10.CodeNumber AS ICD10Code4,  "
		"    PatientsT.FinancialNotes, PatientsT.SuppressStatement,  "
		"    PatientsT.DefaultReferringPhyID, PatientsT.MirrorID,  "
		"    PatientsT.ImageIndex, PatientsT.InformID, PersonT.EmergFirst,  "
		"    PersonT.EmergLast, PersonT.EmergRelation,  "
		"    PersonT.EmergHPhone, PersonT.EmergWPhone,  "
		"    PersonT.Archived, PersonT.First, PersonT.Middle,  "
		"    PersonT.Last, PersonT.Address1, PersonT.Address2,  "
		"    PersonT.City, PersonT.State, PersonT.Zip, PersonT.Gender,  "
		"    PrefixT.Prefix, PersonT.Suffix, PersonT.Title,  "
		"    PersonT.HomePhone, PersonT.WorkPhone,  "
		"    PersonT.Extension, PersonT.CellPhone, PersonT.OtherPhone,  "
		"    PersonT.Email, PersonT.Pager, PersonT.Fax,  "
		"    PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
		"    PersonT.FirstContactDate, PersonT.InputDate,  "
		"    PersonT.WarningMessage, PersonT.DisplayWarning,  "
		"    PersonT.Note, UsersT.PersonID AS UserID,  "
		"    CoordT.Last + ', ' + CoordT.First + ' ' + CoordT.Middle AS LoginName, PersonT.Location AS LocID, LocationsT.Name AS Location "
		"FROM PatientsT INNER JOIN "
		"    ((PersonT LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
		"    (UsersT INNER JOIN PersonT CoordT ON UsersT.PersonID = CoordT.ID) ON PatientsT.EmployeeID = UsersT.PersonID "
		"	 LEFT JOIN DiagCodes DiagCodes1 ON PatientsT.DefaultDiagID1 = DiagCodes1.ID "
		"	 LEFT JOIN DiagCodes DiagCodes2 ON PatientsT.DefaultDiagID2 = DiagCodes2.ID "
		"	 LEFT JOIN DiagCodes DiagCodes3 ON PatientsT.DefaultDiagID3 = DiagCodes3.ID "
		"	 LEFT JOIN DiagCodes DiagCodes4 ON PatientsT.DefaultDiagID4 = DiagCodes4.ID "
		"	 LEFT JOIN DiagCodes DiagCodes1_ICD10 ON PatientsT.DefaultICD10DiagID1 = DiagCodes1_ICD10.ID "
		"	 LEFT JOIN DiagCodes DiagCodes2_ICD10 ON PatientsT.DefaultICD10DiagID2 = DiagCodes2_ICD10.ID "
		"	 LEFT JOIN DiagCodes DiagCodes3_ICD10 ON PatientsT.DefaultICD10DiagID3 = DiagCodes3_ICD10.ID "
		"	 LEFT JOIN DiagCodes DiagCodes4_ICD10 ON PatientsT.DefaultICD10DiagID4 = DiagCodes4_ICD10.ID "
		"WHERE (PatientsT.PersonID > 0) AND UsersT.PatientCoordinator <> 0", 
		((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));

		return _T(strSQL);
		break;

	case 301:
		//Patients with Active Procedures
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.StepName Is Null THEN '<All Steps Completed>' ELSE StepTemplatesT.StepName END AS PtStepName, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, LaddersT.Status AS StatusID, LadderStatusT.Name AS StatusName, "
				"ProcInfoDetailsT.ProcedureID AS ProcID, dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS ProcName, LaddersT.ID AS LadderID "
				"FROM PersonT INNER JOIN ((PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT LEFT JOIN ProcInfoDetailsT ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) LEFT JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID ON PersonT.ID = LaddersT.PersonID LEFT JOIN ((SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.StepTemplateID Is Null THEN -1 ELSE StepsT.StepTemplateID END AS NextStep "
				"FROM LaddersT INNER JOIN ( "
				"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
				"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
				"GROUP BY LaddersT.ID "
				") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) AS NextStepQ INNER JOIN (StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON NextStepQ.NextStep = StepTemplatesT.ID) ON LaddersT.ID = NextStepQ.LadderID "
				"WHERE LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1)"
				"AND PatientsT.CurrentStatus <> 4 AND CASE WHEN (SELECT ActiveDate FROM StepsT WHERE StepsT.LadderID = LaddersT.ID AND StepTemplateID = NextStepQ.NextStep) Is Null THEN '1/1/1900' ELSE "
				"(SELECT ActiveDate FROM StepsT WHERE StepsT.LadderID = LaddersT.ID AND StepTemplateID = NextStepQ.NextStep) "
				"END < getdate()");
		break;

	case 302:
		//Patients with Finished Procedures
		return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.StepName Is Null THEN '<All Steps Completed>' ELSE StepTemplatesT.StepName END AS PtStepName, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, LaddersT.Status AS StatusID, LadderStatusT.Name AS StatusName, "
				"ProcInfoDetailsT.ProcedureID AS ProcID, dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS ProcName, LaddersT.ID AS LadderID "
				"FROM PersonT INNER JOIN ((PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT LEFT JOIN (ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) LEFT JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID ON PersonT.ID = LaddersT.PersonID LEFT JOIN ((SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.StepTemplateID Is Null THEN -1 ELSE StepsT.StepTemplateID END AS NextStep "
				"FROM LaddersT INNER JOIN ( "
				"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
				"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
				"GROUP BY LaddersT.ID "
				") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) AS NextStepQ INNER JOIN (StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON NextStepQ.NextStep = StepTemplatesT.ID) ON LaddersT.ID = NextStepQ.LadderID "
				"WHERE LaddersT.Status = 2");
		break;

	case 321:
		//Contact Report		(Nextech Only)
		/*	Version History
			DRT 5/25/2004 - PLID 11829 - Added new fields - NexTrack through NexEMR
			// (m.cable 7/19/2004 16:54) - Added Patient Coordinator
			TES 6/20/2005 - Updating for new structure.
			// (z.manning, 02/28/2007) - PLID 24984 - Added email fields.
			// (z.manning, 08/20/2007) - PLID 25011 - Changed the note filed to ntext so that crystal
			// can show more than 255 chars of it.
			(d.thompson 2009-08-06) - PLID 35135 - Added all the missing license options that have been
				made since we last updated them.
			// (s.dhole 2010-03-26 10:15) - PLID 37035 - Added all the missing license options that have been
				made since we last updated them(FirstDataBank ,NexPhoto,TOPSLink,NexWebLeads, NexWebPortal). 
			// (c.haag 2010-06-30 10:08) - PLID 39423 - Added Frames and DeviceImport
			// (j.gruber 2010-07-15 11:58) - PLID 38987 - added celltrust and BOLD
			// (s.dhole 2010-07-15 16:46) - PLID  39509 added Issue_Category and oder to sub report
			// (s.dhole 2010-07-15 16:47) - PLID 39509 added contact, Data_Conversion_user, Data_Conversion_from,  Install_user
			// (d.lange 2010-09-20 11:40) - PLID 40445 - added Chase CC Processing
			// (d.thompson 2010-10-18) - PLID 40973 - Fixed "data conversion user" and "install user" fields to be correct.  Removed unnecessary old query while here.
			//TES 12/9/2010 - PLID 41701 - Glasses Orders licensing
			// (z.manning 2011-01-11 10:50) - PLID 42057 - Patient check-in licensing
			// (a.wilson 2011-10-12) PLID 45550 - adding PQRS licensing and ConcurrentTSLicensesBought
			// (a.wilson 2011-11-10) PLID 46408 - adding the Eyemaginations License
			// (z.manning 2011-12-05 17:08) - PLID 44162 - Added NexWeb EMR license
			// (d.singleton 2012-03-21 11:49) - PLID 49086 license for code correct
			// (j.armen 2012-03-27 16:48) - PLID 49244 - Recall
			// (j.armen 2012-05-30 13:57) - PLID 50688 - NexEMRCosmetic
			// (z.manning 2012-06-12 16:13) - PLID 50878 - iPad counts
			// (j.luckoski 2013-01-21 11:12) - PLID 49892 - Add ERXPrescribers
			// (z.manning 2013-01-31 15:21) - PLID 54954 - Added Nuance bought
			// (a.wilson 2013-03-27 17:17) - PLID 44474 - Added contact Specialty
			// (r.goldschmidt 2014-04-02 11:32) - PLID 60011 - Added NexSync, Practice Website, and EMR specialist
			// (a.wilson 2014-08-15 10:58) - PLID 62516 - vision payments.
			// (z.manning 2015-05-12 15:50) - PLID 65961 - StateASCReports
			// (z.manning 2015-06-17 09:10) - PLID 66407 - MPVPatients
			// (z.manning 2015-06-17 15:01) - PLID 66278 - PortalProviders
			// (z.manning 2015-07-21 13:21) - PLID 66599 - ICCP
			// (r.farnworth 2015-11-11 11:30) - PLID 66407 - MPVMessaging
			// (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
			// (b.cardillo 2016-03-08 14:02) - PLID 68409 - Iagnosis provider count
			// (r.gonet 2016-05-10) - NX-100478 - Added AzureRemoteApp to the main report.
		*/
		switch(nSubLevel){
		case 1:	//subreport
			return _T("SELECT     Notes.PersonID AS PatID, Notes.Date, CONVERT(ntext, Notes.Note) AS Note, NoteCatsF.Description AS Category,  \r\n"
                      "CASE WHEN IssueCategoryT.ParentID IS NOT NULL THEN \r\n"
                      "  (SELECT     Description \r\n"
                      "      FROM          IssueCategoryT AS a \r\n"
                      "      WHERE      a.id = IssueCategoryT.ParentID) + ' - ' + IssueCategoryT.Description ELSE IssueCategoryT.Description END AS Issue_Category  \r\n"
					  "FROM IssueT INNER JOIN \r\n"
                      "IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID INNER JOIN \r\n"
                      "IssueDetailsT ON IssueT.ID = IssueDetailsT.IssueID RIGHT OUTER JOIN \r\n"
                      "Notes ON IssueDetailsT.NoteID = Notes.ID LEFT OUTER JOIN \r\n"
                      "NoteCatsF ON Notes.Category = NoteCatsF.ID  \r\n"
					  " ORDER BY (Case  When NoteCatsF.id =41 then -1 else 0 end),  NoteCatsF.Description ,Issue_Category , Notes.Date desc");
			break;
		default:	//main
			return _T(
				"SELECT PatientsT.PersonID AS PatID, PatientsT.UserDefinedID,  \r\n"
				" PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last AS PatName,  \r\n"
				" WorkPhone, HomePhone, CellPhone, Fax, Pager, OtherPhone, Address1, Address2,  \r\n"
				" City, State, Zip, Note, LicenseBought, DoctorsBought, DoctorsUsed,  \r\n"
				" PalmPilotBought, Patients, Scheduler, Billing, HCFA, Letters, Quotes,  \r\n"
				" Marketing, Inventory, EBilling, MirrorLink, InformLink, UnitedLink,  \r\n"
				" PersonT.EmergWPhone, NexTrack, NexForms, NexWeb, ASCModule, NexSpa, NexVoice,  \r\n"
				" NexEMR, UsersT.UserName AS PatCoord, PersonT.Email AS PatientEmail,  \r\n"
				" NxClientsT.PracEmail AS PracticeEmail,  \r\n"
				" PPCBought, databasesbought, emrprovidersbought, emrpro, emrprodoctornames, \r\n"
				" ccprocessing, advinventory, eremittance, eeligibility, carecreditlink, \r\n"
				" quickbookslink, unicornlink, hl7, transcriptions, customrecords, retention, \r\n"
				" labs, faxing, barcoderead, schedstandard, emrstandard, cycleofcare, eprescribe, newcrop, \r\n"
				" FirstDataBank ,NexPhoto,TOPSLink,NexWebLeads, NexWebPortal, Frames, DeviceImport , bold, celltrust , \r\n" 
				" PatientCheckIn, \r\n"
				" (SELECT     TextParam FROM         CustomFieldDataT\r\n"
				" WHERE     (PersonID = PersonT.ID) AND (FieldID = 1) ) as Contact , \r\n"
				" (SELECT TextParam FROM CustomFieldDataT \r\n"
				" WHERE (PersonID = PersonT.ID) AND (FieldID = 3)) as Website, \r\n"
				" (SELECT     TOP (1) UsersT.UserName  \r\n"
				" FROM \r\n"
				" IssueT INNER JOIN \r\n"
				" IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID \r\n"
				" LEFT JOIN UsersT ON IssueT.UserID = UsersT.PersonID \r\n"
				" WHERE     (IssueT.ClientID = PersonT.ID) AND (IssueCategoryT.Inactive = 0) AND (IssueCategoryT.ParentID = 29) \r\n"
				" ) as  Data_Conversion_user,  \r\n "
				" ( SELECT     TOP (1) IssueCategoryT.Description    \r\n "
				" FROM   \r\n"
				" IssueT INNER JOIN  \r\n "
				" IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID   \r\n "
				" WHERE     (IssueT.ClientID = PersonT.ID ) AND (IssueCategoryT.Inactive = 0) AND (IssueCategoryT.ParentID = 29)  \r\n "
				" ) as Data_Conversion_from ,  \r\n "
				" (SELECT     TOP (1) UsersT.UserName   \r\n "
				" FROM \r\n" 
				" IssueT INNER JOIN  \r\n "
				" IssueCategoryT ON IssueT.CategoryID = IssueCategoryT.ID   \r\n "
				" LEFT JOIN UsersT ON IssueT.UserID = UsersT.PersonID \r\n"
				" WHERE     (IssueT.ClientID = PersonT.ID) AND (IssueCategoryT.Inactive = 0) AND (IssueCategoryT.ParentID= 2)  \r\n "
				" ) as Install_user, \r\n "
				" (SELECT STUFF((SELECT ', ' +  CustomListItemsT.[Text] FROM CustomListDataT \r\n"
				" LEFT JOIN CustomListItemsT ON CustomListDataT.CustomListItemsID = CustomListItemsT.ID \r\n"
				" WHERE CustomListDataT.PersonID = PersonT.ID AND CustomListDataT.FieldID = 24 ORDER BY [Text] \r\n"
				" FOR XML PATH(''), TYPE).value('/', 'NVARCHAR(MAX)'), 1, 2, '') \r\n"
				" ) AS Speciality, \r\n"
				" (SELECT TOP (1) UsersT.UserName \r\n"
				" FROM UsersT WHERE UsersT.PersonID = NxClientsT.EMRSpecialistID) as EMRSpecialist, \r\n"
				" ChaseCCProcessing, GlassesOrders, PQRS, ConcurrentTSLicensesBought, Eyemaginations, NexWebEmr, CodeCorrect, Recall, NexEMRCosmetic, \r\n"
				" IpadBought, IpadEnabled, NuanceBought, NexSyncBought, \r\n"
				" ERXLicPrescribersBought, ERXLicPres, ERXLicPrescriberNames, \r\n"
				" ERXMidPrescribersBought, ERXMidPres, ERXMidPrescriberNames, \r\n"
				" ERXStaffPrescribersBought, ERXStaffPres, ERXStaffPrescriberNames, \r\n"
				" VisionPayments, LockboxPayments, StateASCReports, MPVPatients, \r\n"
				" PortalProvidersBought, ICCP, MPVMessaging, SubscriptionLocalHosted, \r\n"
				" IagnosisProviderCount, \r\n"
				" AzureRemoteApp, ActiveDirectoryLogin \r\n"
				"FROM PersonT  \r\n"
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
				"INNER JOIN NxClientsT ON PersonT.ID = NxClientsT.PersonID  \r\n"
				"LEFT JOIN UsersT ON PatientsT.EmployeeID = UsersT.PersonID ");
			break;
		}


		case 367:
		//Patients with Procedures On Hold
			return _T("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.StepName Is Null THEN '<All Steps Completed>' ELSE StepTemplatesT.StepName END AS PtStepName, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, LaddersT.Status AS StatusID, LadderStatusT.Name AS StatusName, "
				"ProcInfoDetailsT.ProcedureID AS ProcID, ProcedureT.Name AS ProcName,  "
				"(SELECT ActiveDate FROM StepsT WHERE StepsT.LadderID = LaddersT.ID AND StepTemplateID = NextStepQ.NextStep) AS ActiveDate, LaddersT.ID AS LadderID "
				"FROM PersonT INNER JOIN ((PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT LEFT JOIN (ProcInfoDetailsT INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) LEFT JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID ON PersonT.ID = LaddersT.PersonID LEFT JOIN ((SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.StepTemplateID Is Null THEN -1 ELSE StepsT.StepTemplateID END AS NextStep "
				"FROM LaddersT INNER JOIN ( "
				"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
				"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
				"GROUP BY LaddersT.ID "
				") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) AS NextStepQ INNER JOIN (StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON NextStepQ.NextStep = StepTemplatesT.ID) ON LaddersT.ID = NextStepQ.LadderID "
				"WHERE LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1) "
				"AND (SELECT ActiveDate FROM StepsT WHERE StepsT.LadderID = LaddersT.ID AND StepTemplateID = NextStepQ.NextStep) Is Null OR "
				"(SELECT ActiveDate FROM StepsT WHERE StepsT.LadderID = LaddersT.ID AND StepTemplateID = NextStepQ.NextStep) > getdate()"
				" AND (ProcedureT.MasterProcedureID IS NULL OR ProcInfoDetailsT.Chosen <> 0)" );
		

		case 375:
		//Patients By Ins Co by Ins Contact
		/*	Version History
			DRT 3/12/03 - Created
		*/
			return _T("SELECT PatientsT.UserDefinedID, PatientsT.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, InsuranceCoT.PersonID AS InsCoID,  "
			"InsuranceCoT.Name AS InsCoName, InsuranceContactsT.PersonID AS ContactID,  "
			"ContactPersonT.Last + ', ' + ContactPersonT.First + ' ' + ContactPersonT.Middle AS ContactName, PatientsT.MainPhysician AS ProvID, PersonT.Location AS LocID, "
			"LocationsT.Name AS LocName, ProvPersonT.Last + ', ' + ProvPersonT.First + ' ' + ProvPersonT.Middle AS ProvName "
			" "
			"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PatientID "
			"LEFT JOIN InsuranceContactsT ON InsuredPartyT.InsuranceContactID = InsuranceContactsT.PersonID "
			"LEFT JOIN PersonT ContactPersonT ON InsuranceContactsT.PersonID = ContactPersonT.ID "
			"LEFT JOIN InsuranceCoT ON InsuranceContactsT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"LEFT JOIN PersonT ProvPersonT ON PatientsT.MainPhysician = ProvPersonT.ID "
			" "
			"WHERE InsuranceCoT.PersonID IS NOT NULL");
			break;

		case 397:
			//Patients by Resp Type
			/*	Version History
				DRT 6/12/03 - Created.  Shows basic information, and has some extra fields (insurance addr/phone, pt addr/phone) that can be
						used in a custom report.
			*/
			return _T("SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone,  "
			"InsuranceCoT.Name, RespTypeT.TypeName, PersonInsCoT.Address1 AS InsAddr1, PersonInsCoT.Address2 AS InsAddr2,  "
			"PersonInsCoT.City AS InsCity, PersonInsCoT.State AS InsState, PersonInsCoT.Zip AS InsZip, PersonInsCoT.WorkPhone AS InsPhone,  "
			"InsuredPartyT.InsuranceCoID AS InsCoID, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID,  "
			"PersonT.FirstContactDate AS FCDate, RespTypeT.ID AS RespTypeID "
			"FROM "
			"InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"INNER JOIN PersonT PersonInsCoT ON InsuranceCoT.PersonID = PersonInsCoT.ID ");
			break;

		case 401:
			//Patient Warnings
			/*	Version History
				DRT 7/16/03 - Created.
			*/
			return _T("SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"PersonT.WarningMessage, PatientsT.MainPhysician AS ProvID, PersonProvT.Last + ', ' + PersonProvT.First + ' ' + PersonProvT.Middle AS ProvName, "
				"PersonT.FirstContactDate AS Date, PersonT.Location AS LocID, "
				"/* Fields for custom usage*/ "
				"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, "
				"LocationsT.Name AS LocName "
				"FROM "
				"PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN PersonT PersonProvT ON PatientsT.MainPhysician = PersonProvT.ID "
				"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
				"WHERE PersonT.DisplayWarning = 1 ");
			break;

		case 413:
			//Patient Medications
			/*	Version History
				DRT 7/29/03 - Created.
				TES 7/30/03 - Added Unit field
				(c.haag 2007-02-02 17:03) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
				(d.thompson 2008-12-01) - PLID 32174 - DefaultPills is now DefaultQuantity, Description is now PatientInstructions
				(d.thompson 2009-01-15) - PLID 32176 - DrugList.Unit is now DrugStrengthUnitsT.Name, joined from DrugList.StrengthUnitID
				// (d.thompson 2009-03-11) - PLID 33481 - Actually this should use QuantityUnitID, not StrengthUnitID
			*/
			return _T("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
				"LEFT(EMRDataT.Data,255) AS MedName, PatientInstructions, DefaultRefills, DefaultQuantity, "
				"COALESCE(DrugStrengthUnitsT.Name, '') AS Unit , CurrentPatientMedsT.MedicationID AS MedID, "
				"PersonT.FirstContactDate AS Date, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID "
				"FROM CurrentPatientMedsT "
				"INNER JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID "
				"INNER JOIN PersonT ON CurrentPatientMedsT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"LEFT JOIN DrugStrengthUnitsT ON DrugList.QuantityUnitID = DrugStrengthUnitsT.ID ");
			break;

		case 414:
			//Patient Prescriptions
			/*	Version History
				DRT 7/29/03 - Created.
				TES 7/30/03 - Added Unit field
				(c.haag 2007-02-02 17:03) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
				TES 2/10/2009 - PLID 33002 - Renamed Description to PatientExplanation and PillsPerBottle to Quantity, but
					kept them aliased to the old names to avoid messing up any custom reports out there.
				TES 3/5/2009 - PLID 33068 - Added SureScripts fields
				(d.thompson 2009-04-02) - PLID 33571 - Added strength unit
				TES 4/2/2009 - PLID 33750 - Strength and DosageForm now pull from DrugList
				TES 5/11/2009 - PLID 28519 - Added SampleExpirationDate
			*/
			return _T("SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				"LEFT(EMRDataT.Data,255) AS MedName, PatientMedications.PatientExplanation AS Description, PatientMedications.RefillsAllowed,  "
				"PatientMedications.Quantity AS PillsPerBottle, PatientMedications.Unit, PersonT.FirstContactDate AS Date, PatientMedications.LocationID AS LocID,  "
				"PatientMedications.ProviderID AS ProvID, LocationsT.Name AS LocName, "
				"PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ProvName, PatientMedications.PrescriptionDate AS PDate, "
				"PatientMedications.MedicationID AS MedID, "
				"PatientMedications.DaysSupply, PatientMedications.NoteToPharmacist, PatientMedications.AllowSubstitutions, "
				"PatientMedications.PriorAuthorization, PatientMedications.PriorAuthorizationIsSample, DrugList.Strength, "
				"DrugDosageFormsT.Name AS DosageForm, dbo.GetPrescriptionDiagList(PatientMedications.ID) AS DiagnosisCodeList, "
				"StrengthUnitT.Name AS StrengthUnit, PatientMedications.SampleExpirationDate "
				"FROM PatientMedications "
				"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				"INNER JOIN PersonT ON PatientMedications.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN DrugDosageFormsT ON DrugList.DosageFormID = DrugDosageFormsT.ID "
				"LEFT JOIN DrugStrengthUnitsT AS StrengthUnitT ON DrugList.StrengthUnitID = StrengthUnitT.ID "
				"LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
				"LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				" WHERE PatientMedications.Deleted = 0");
			break;

		case 472:
			//Patients By Responsible Party
			/*  Version History
				Jennie created this on 2/9/04, but didn't add a Version History!
				TES 3/8/04 - Fixed the Provider filter.
			*/
			return _T(" SELECT RespInfo.ID as RespPartyID, RespInfo.First as RespFirst, RespInfo.Last as RespLast, RespInfo.Middle as RespMiddle,  RespInfo.Address1 As RespAdd1, "
				" RespInfo.Address2 As RespAdd2, Respinfo.City as respCity, RespInfo.State As RespState, RespInfo.Zip as RespZip, ResponsiblePartyT.Employer as RespEmployer, "
				" RespInfo.Birthdate as RespBirthdate, ResponsiblePartyT.RelationToPatient as RespRelation, RespInfo.Gender as RespGender, "
				" PatientsT.UserdefinedID as PatUserDefID, PatientsT.PersonID As PatID,  PatInfo.Location as LocID, PatInfo.First As PatFirst, PatInfo.Middle As PatMiddle, "
				" PatInfo.Last as PatLast, PatInfo.FirstContactDate As Date, PatInfo.Address1 as PatAdd1, PatInfo.Address2 as PatAdd2, "
				" PatInfo.City as PatCity, PatInfo.State as PatState, PatInfo.Zip as PatZip, PatInfo.HomePhone as PatHPhone, PatInfo.WorkPhone as PatWPhone, "
				" PatientsT.MainPhysician AS ProvID "
				" FROM ResponsiblePartyT LEFT JOIN PatientsT ON ResponsiblePartyT.PatientId = PatientsT.PersonID "
				" LEFT JOIN PersonT RespInfo ON ResponsiblePartyT.PersonID = RespInfo.ID "
				" LEFT JOIN PersonT PatInfo ON PatientsT.PersonId = PatInfo.ID ");
			break;

		case 571:
			//Patient Outcomes
			/*  Version History
				// (m.hancock 2006-08-07 15:14) - PLID 15451 - Created.  Report is supposed to show patient demographics
				// with detailed results of refractive visits.
				// (m.hancock 2006-10-02 14:34) - PLID 15451 - The output for the provider name had the comma after the first
				// name, but it should have been after the last name.
				// (f.dinatale 2010-10-18) - PLID 40876 - Add SSN Masking.
			*/
			strSQL.Format(
				" SELECT "
				""
				" /* Eye Procedure Fields */"
				" EyeProceduresT.ID AS EyeProcedureID, ProcedureT.Name AS ProcedureName, EyeProceduresT.ProcedureDate AS ProcedureDate, "
				""
				" /* Patient Fields */"
				" EyeProceduresT.PatientID AS PatID, PatientsT.UserDefinedID AS UserDefinedID, "
				" PatientQ.Last + ', ' + PatientQ.First + ' ' + PatientQ.Middle AS PatientName, PatientQ.Birthdate AS BirthDate, "
				" dbo.MaskSSN(PatientQ.SocialSecurity, %s) AS SSN, PatientQ.Address1 AS Address1, PatientQ.Address2 AS Address2, "
				" PatientQ.City AS City, PatientQ.State AS State, PatientQ.Zip AS Zip, PatientQ.Gender AS Gender, "
				" PatientQ.HomePhone AS HomePhone, PatientQ.PrivHome AS PrivHome, PatientQ.WorkPhone AS WorkPhone, "
				" PatientQ.Extension AS Extension, PatientQ.PrivWork AS PrivWork, PatientQ.CellPhone AS CellPhone, PatientQ.PrivCell AS PrivCell, "
				""
				" /* Provider and Location */"
				" EyeProceduresT.ProviderID AS ProvID, ProviderQ.Last + ', ' + ProviderQ.First + ' ' + ProviderQ.Middle AS ProviderName, "
				" EyeProceduresT.LocationID AS LocID, LocationsT.Name AS LocationName, "
				""
				" /* Additional Eye Procedure Fields */"
				" CASE WHEN EyeProceduresT.Monovision = 1 THEN 'Yes' ELSE 'No' END AS Monovision, "
				" CASE WHEN EyeProceduresT.Contacts = 1 THEN 'Yes' ELSE 'No' END AS Contacts, EyeProceduresT.Complaint AS Complaint, "
				" EyeProceduresT.Age AS Age, EyeProceduresT.ProcType AS ProcType, EyeProceduresT.OpType AS OpType, "
				" EyeProceduresT.LaserUsed AS LaserUsed, EyeProceduresT.KeratomeType AS KeratomeType, EyeProceduresT.BladeType AS BladeType, "
				" EyeProceduresT.EquipmentUsed AS EquipmentUsed, EyeProceduresT.Power AS Power, EyeProceduresT.NumPulses AS NumPulses, "
				" EyeProceduresT.PupilLight AS PupilLight, EyeProceduresT.PupilDark AS PupilDark, EyeProceduresT.CLType AS CLType, "
				" EyeProceduresT.CLUse AS CLUse, "
				""
				" /* Visit Fields */"
				" EyeVisitsT.ID AS VisitID, EyeVisitsT.Date AS VisitDate, EyeVisitTypesT.Type AS VisitType, "
				" EyeVisitRatingsT.Rating AS VisitRatingName, EyeVisitsT.Notes AS VisitNotes, "
				""
				" /* Eye Test Fields */"
				" EyeTestsT.ID AS EyeTestID, EyeTestsT.TestID AS TestID, EyeTestTypesT.TestName AS TestName, "
				" CASE WHEN EyeTestsT.EyeType = 1 THEN 'Left' ELSE 'Right' END AS EyeType, EyeTestsT.Sphere AS Sphere, EyeTestsT.Cyl AS Cyl, "
				" EyeTestsT.Axis AS Axis, EyeTestsT.VA AS VA, EyeTestsT.BCVA AS BCVA "
				""
				" FROM EyeTestsT "
				" LEFT JOIN EyeVisitsT ON EyeTestsT.VisitID = EyeVisitsT.ID "
				" LEFT JOIN EyeProceduresT ON EyeVisitsT.EyeProcedureID = EyeProceduresT.ID  "
				" LEFT JOIN LocationsT ON EyeProceduresT.LocationID = LocationsT.ID  "
				" LEFT JOIN PersonT PatientQ ON EyeProceduresT.PatientID = PatientQ.ID  "
				" LEFT JOIN PersonT ProviderQ ON EyeProceduresT.ProviderID = ProviderQ.ID  "
				" LEFT JOIN ProcedureT ON EyeProceduresT.ProcedureID = ProcedureT.ID "
				" LEFT JOIN EyeVisitTypesT ON EyeVisitsT.VisitType = EyeVisitTypesT.ID  "
				" LEFT JOIN EyeTestTypesT ON EyeTestsT.TestID = EyeTestTypesT.TestID "
				" LEFT JOIN EyeVisitRatingsT ON EyeVisitsT.RatingType = EyeVisitRatingsT.ID "
				" LEFT JOIN PatientsT ON PatientQ.ID = PatientsT.PersonID ", 
				((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));
			return _T(strSQL);
			break;

		case 578: 
			//Patients by Family
			/* Version History
				// (a.walling 2006-11-21 17:53) - PLID 23618 - Created. Show basic patient and relative demographics and group by family.
				//		Include several fields for customization/export/etc.
				// (j.jones 2010-02-04 14:46) - PLID 36500 - cleaned up report query so "ProviderFirst" and friends aren't exposed as "AS ProviderFirst"
				//in the main exposed fields
				*/

			return _T("SELECT FamilyID, First, Middle, Last, "
				"	UserDefinedID, City, State, "
				"	Zip, Gender, BirthDate, FCDDate AS FirstContactDate, "
				"	ProviderFirst, ProviderMiddle, ProviderLast, "
				"	LocationName AS Location, Location AS LocID, MainPhysician AS ProvID, "
				"	CreatedDate, "

				"	RelFirst, RelMiddle, RelLast, "
				"	RelUserDefinedID, RelCity, "
				"	RelState, RelZip, RelGender, "
				"	RelBirthDate, RelFirstContactDate, "
				"	RelProviderFirst, RelProviderMiddle, "
				"	RelProviderLast "
				"FROM (SELECT "
				"	PersonFamilyT.FamilyID, PersonT.First, PersonT.Middle, PersonT.Last, "
				"	PatientsT.UserDefinedID, PersonT.City, PersonT.State, "
				"	PersonT.Zip, PersonT.Gender, PersonT.BirthDate, PersonT.FirstContactDate AS FCDDate, "
				"	PatPhysQ.First AS ProviderFirst, PatPhysQ.Middle AS ProviderMiddle, PatPhysQ.Last AS ProviderLast, "
				"	LocationsT.Name AS LocationName, PersonT.Location, PatientsT.MainPhysician, "
				"	FamiliesT.CreatedDate, "

				"	RelativeQ.First AS RelFirst, RelativeQ.Middle AS RelMiddle, RelativeQ.Last AS RelLast, "
				"	RelatedPatQ.UserDefinedID AS RelUserDefinedID, RelativeQ.City AS RelCity, "
				"	RelativeQ.State AS RelState, RelativeQ.Zip AS RelZip, RelativeQ.Gender AS RelGender, "
				"	RelativeQ.BirthDate AS RelBirthDate, RelativeQ.FirstContactDate AS RelFirstContactDate, "
				"	RelPatPhysQ.First AS RelProviderFirst, RelPatPhysQ.Middle AS RelProviderMiddle, "
				"	RelPatPhysQ.Last AS RelProviderLast "

				"FROM PersonFamilyT "
				"LEFT JOIN PersonT ON PersonT.ID = PersonFamilyT.PersonID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN PersonT PatPhysQ ON PatPhysQ.ID = PatientsT.MainPhysician "
				"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "

				"LEFT JOIN PersonT RelativeQ ON RelativeQ.ID = PersonFamilyT.RelativeID "
				"LEFT JOIN PatientsT RelatedPatQ ON RelativeQ.ID = RelatedPatQ.PersonID "
				"LEFT JOIN PersonT RelPatPhysQ ON RelPatPhysQ.ID = RelatedPatQ.MainPhysician "

				"LEFT JOIN FamiliesT ON PersonFamilyT.FamilyID = FamiliesT.ID "
				") AS PatientsByFamilyQ "
				"ORDER BY FamilyID, Zip, Last, First, Middle, UserDefinedID");
			break;

			// (a.walling 2007-09-25 17:19) - PLID 25976 - Moved Patient Reward Points to Financial

			case 629:
			/* Version History 
			// (j.gruber 2008-06-03 13:19) - PLID 25447 - Inquiries Report
				(d.thompson 2009-12-04) - PLID 36502 - Disabled the 'use filter' option.  It didn't work and inquiries can't be in filters anyways.
			*/
			{

			CString strSql;
			return _T("SELECT PatientsT.PersonID, UserDefinedID, First, Middle, Last, Email, Note, dbo.CalcProcInfoName(ProcInfoT.ID) as ProcList, ReferralID, "
				" ReferralSourceT.Name as RefSource, LocationsT.ID as LocID, LocationsT.Name as LocName, PersonT.InputDate as InputDate, UsersT.UserName, PersonT.UserID as InputEmpID "
				" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				" LEFT JOIN ProcInfoT ON PatientsT.PersonID = ProcInfoT.PatientID "
				" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
				" LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID "
				" WHERE CurrentStatus = 4 ");
			
			}
		break;	

		case 653:
		// Patient Barcode Sheet
		/*	Version History
			(e.lally 2008-07-25) PLID 30837 - Created - (based on scaled down version of patient list)
			// (j.jones 2009-10-19 17:21) - PLID 35994 - split race & ethnicity
			// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking.
			// (d.thompson 2012-08-09) - PLID 52045 - Reworked Ethnicity table layout, now pulling the "Practice name" version
		*/
		if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
			strFormatString = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
		}
		else {
			strFormatString = "";
		}
		strSQL.Format("SELECT PatientsT.UserDefinedID, " 
                "PersonT.ID AS PatID,  "
				"PersonT.Location as LocID, "
          		"ProvidersT.PersonID AS ProvID, "
				"PersonT.Archived, "
				"PersonT.Last AS LastName, "
				"PersonT.First AS FirstName, "
				"PersonT.Middle AS MiddleName, "
          		"PersonT.Address1, "
          		"PersonT.Address2, PersonT.City, "
          		"PersonT.State, PersonT.Zip, "
          		"PersonT.Gender, PrefixT.Prefix,  "
          		"PersonT.Title, "
          		"PersonT.HomePhone, PersonT.WorkPhone, "
          		"PersonT.Extension, PersonT.CellPhone, "
          		"PersonT.OtherPhone, PersonT.Email, "
          		"PersonT.Pager, PersonT.Fax, "
				"PersonT.BirthDate, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, "
         		"PersonT.FirstContactDate AS FirstContactDate, "
				"PersonT.PrivHome, PersonT.PrivWork,  "
				"PersonT.PrivFax, PersonT.PrivOther, PersonT.PrivPager, PersonT.PrivEmail, "
				// (b.spivey, May 28, 2013) - PLID 56871
				"	LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS Race, "
				"	LEFT(OfficialRaceSubQ.OfficialName, LEN(OfficialRaceSubQ.OfficialName) -1) AS CDCRace, "
				"EthnicityT.Name AS CDCEthnicity, "
				"PersonMainPhysician.Last + ', ' + PersonMainPhysician.First + ' ' + PersonMainPhysician.Middle AS ProvName, "
				"LocationsT.Name AS LocationName, "
          		"PatientsT.CurrentStatus AS CurrentStatus, "
          		"PatientsT.MainPhysician AS MainPhysicianID, "
          		"GroupTypes.GroupName AS TypeOfPatient, "
				"CONVERT(NVARCHAR(50), '%s') AS PhoneFormatString "
				"FROM PersonT "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN ProvidersT ON PatientsT.MainPhysician = ProvidersT.PersonID "
				"LEFT JOIN PersonT PersonMainPhysician ON ProvidersT.PersonID = PersonMainPhysician.ID "
				"LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "
				// (b.spivey, May 28, 2013) - PLID 56871 - creates the list of CDC and custom races. 
				"	CROSS APPLY "
				"	( "
				"		SELECT ( "
				"			SELECT RT.Name + ', ' "
				"			FROM PersonRaceT PRT "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
				"			WHERE PRT.PersonID = PersonT.ID "
				"			FOR XML PATH(''), TYPE "
				"		).value('/', 'nvarchar(max)') "
				"	) RaceSubQ (RaceName) "
				"	CROSS APPLY "
				"	( "
				"		SELECT ( "
				"			SELECT RCT.OfficialRaceName + ', ' "
				"			FROM PersonRaceT PRT "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID "
				"			WHERE PRT.PersonID = PersonT.ID  "
				"			FOR XML PATH(''), TYPE "
				"		).value('/', 'nvarchar(max)') "
				"	) OfficialRaceSubQ (OfficialName) "
				"LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID "
				"LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID "
				"LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
				"WHERE PatientsT.CurrentStatus <> 4 /*inquiries*/ AND PersonT.ID > 0 ", 
				((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"), 
				_Q(strFormatString));
		return _T(strSQL);
		break;

		case 692:
			// Patient Allergies
			/*	Version History
				// (c.haag 2010-01-04 12:31) - PLID 36658 - Created 
			*/
			return _T("SELECT PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
						"LEFT(EMRDataT.Data,255) AS AllergyName, "
						"PatientAllergyT.Description, PatientAllergyT.FromNewCrop, PatientAllergyT.Discontinued, "
						"PatientAllergyT.EnteredDate AS AllergyEnteredDate, "
						"PatientAllergyT.AllergyID AS AllergyID, PersonT.FirstContactDate AS Date, "
						"PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID "
						"FROM "
						"PatientAllergyT "
						"INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
						"INNER JOIN PersonT ON PatientAllergyT.PersonID = PersonT.ID "
						"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
						"LEFT JOIN EMRDataT ON AllergyT.EMRDataID = EMRDataT.ID ");
			break;

		// (j.jones 2010-04-28 12:13) - PLID 35591 - moved from Preview to Patients tab of reports,
		//and renamed from History Tab (PP) to Patient History
		case 407: {
			//Patient History
			/*	Version History
				DRT 7/21/03 - Created.
				TES 2/2/04 - Made accessible from Contacts.
				// (j.jones 2008-09-05 10:27) - PLID 30288 - supported MailSentNotesT
				// (j.jones 2010-04-28 12:09) - PLID 35591 - exposed ServiceDate as TDate, AttachDate as IDate
				// (f.dinatale 2010-10-18) - PLID 40876 - Added SSN Masking.
			*/
			CString strSql;
			strSql.Format("SELECT MailSent.PersonID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
				"MailSentNotesT.Note, MailSent.PathName, MailSent.Sender, MailSent.ServiceDate AS TDate, MailSent.Date AS IDate,  "
				"/* Extra information for editable-ness */ "
				"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone,  "
				"PersonT.WorkPhone, PersonT.Extension, dbo.MaskSSN(PersonT.SocialSecurity, %s) AS SocialSecurity, PersonT.BirthDate "
				"FROM MailSent "
				"INNER JOIN PersonT ON MailSent.PersonID = PersonT.ID "
				"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID ",
				((bSSNReadPermission && bSSNDisableMasking) ? "-1" : (bSSNReadPermission && !bSSNDisableMasking) ? "0" : "1"));

			// (j.jones 2010-04-28 12:20) - PLID 35591 - if we're not filtering on a patient ID,
			// then we need to run the report on patients only, because this can be run from the contacts module
			// as a print preview, but only for patients in the reports module
			if(nPatient <= 0) {
				strSql += "INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID ";
			}
			
			return _T(strSql);
		}
		break;

		case 700:  //Patient Messaging
			/*Version History
			// (j.gruber 2010-10-28 16:19) - PLID 35817 - created
			*/
			{

			CString strSql = "SELECT PersonT.First, PersonT.Middle, PersonT.Last, PatientsT.PersonID AS PatID, PatientsT.UserDefinedID, "
				"Notes.Date AS Date, Notes.Date AS Time, NoteCatsF.Description AS Category, Notes.Note, "
				"PersonT.Location AS LocID, "
				"(SELECT LocationsT.Name FROM LocationsT WHERE LocationsT.ID = PersonT.Location) AS Location, "
				"PatientsT.MainPhysician AS ProvID, "
				"(SELECT PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle "
				"	FROM PersonT PersonProv "
				"	WHERE PersonProv.ID = PatientsT.MainPhysician) AS ProvName, "
				"(SELECT PersonRefPhy.Last + ', ' + PersonRefPhy.First + ' ' + PersonRefPhy.Middle "
				"	FROM PersonT PersonRefPhy "
				"	WHERE PersonRefPhy.ID = PatientsT.DefaultReferringPhyID) AS ReferringPhysName, "
				"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, "
				"PersonT.WorkPhone, PersonT.Extension, PersonT.CellPhone, "
				" PatientMessagingThreadT.ID as ThreadID, PatientMessagingThreadT.Subject, "
				" CASE WHEN PatientMessagingThreadT.Status = 0 THEN 'Open' ELSE 'Closed' END As Status, PatientMessagingThreadT.CreatedDate, Notes.IsPatientCreated "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "				
				"INNER JOIN Notes ON PatientsT.PersonID = Notes.PersonID "
				"LEFT JOIN PatientMessagingThreadT ON Notes.PatientMessagingThreadID = PatientMessagingThreadT.ID "
				"LEFT JOIN NoteCatsF ON Notes.Category = NoteCatsF.ID "				
				"WHERE (PatientsT.PersonID > 0) AND (Notes.Note IS NOT NULL) AND Notes.PatientMessagingThreadID IS NOT NULL ";
			

			return _T(strSql);
			}
		break;

		case 743:
			//Prescriptions By Provider Report
			/* Version History
				(r.wilson 10/24/2012) plid 51770 -  Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			
			CString strSql =						
			"				    "
			" SELECT     "
			"		PatientMedications.ID AS PatientMedicationID,    "
			"		PrescriptionDate AS PrescriptionDate,    "
			"		Patient.ID AS PatID,    "
			"		Patient.[First] AS PatientFirst,    "
			"		Patient.[Last] AS PatientLast,    "
			"		Patient.HomePhone AS PatientPhone,    "
			"		DrugList.DrugName AS Drug,    "
			"		PrescriptionQueueStatusT.InternalID AS StatusID,    "
			"		PrescriptionQueueStatusT.Status AS [Status],    "
			"		LocationsT.ID AS LocID,    "
			"		LocationsT.Name AS Location,    "
			"		UserEnteredBy.ID AS UserEnteredByID,    "
			"		UserEnteredBy.[First] AS UserEnteredByFirst,	    "
			"		UserEnteredBy.[Last] AS UserEnteredByLast,	    "
			"		UsersT.[Username] AS EnteredByUsername,	    "
			"		EmrDataT.Data AS Description, "			
			"		PatientMedications.EnglishDescription AS EnglishDescription,  "
			"		PatientMedications.PatientExplanation AS SigPatientExplanation,    "
			"		CASE  "
			"		WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"		ELSE 1  "
			"		END AS SigPatientExplanationExists,   "
			"		PatientMedications.Quantity AS Quantity,  "
			"		PatientMedications.Unit AS Unit,  "
			"		PatientMedications.NoteToPharmacist AS Notes,    "
			"		CASE   "
			"		WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"		WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"		ELSE 1  "
			"		END AS NoteToPharmacistExists,	 "
			"		coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"		coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"		coalesce(Pharmacy.Phone, '') AS PharmacyPhone,					  "
			"		CASE     "
			"		WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"		WHEN FinalDestinationType = 1 THEN 'Print'     "
			"		WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"		WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"		WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"		WHEN FinalDestinationType = 5 THEN 'Test'    "
			"		ELSE ''     "
			"		END AS Route,    "
			"		DrugList.DEASchedule AS Sch,    "
			"		ProvT.ID AS ProvID,    "
			"		ProvT.First AS ProvFirst,    "
			"		ProvT.Last AS ProvLast,    "
			"		coalesce(ProvPrefix.Prefix, '') AS ProvPrefix	   "
			"		FROM PatientMedications "
			"		INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID "
			"		INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus "
			"		INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"		LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID "
			"		LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"		LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID "
			"		INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"		LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID "
			"		LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"		LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID "			
			"		WHERE PatientMedications.Deleted <> 1 ";			

				return _T(strSql);
			}
		break;

		case 744:
			//Prescriptions by User Report
			/* Version History
				(r.wilson 10/29/2012) plid 51771 -  Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			
			CString strSql =			
			"			    "			
			"			SELECT     "
			"			PatientMedications.ID AS PatientMedicationID,    "
			"			PrescriptionDate AS PrescriptionDate,    "
			"			Patient.ID AS PatID,    "
			"			Patient.[First] AS PatientFirst,    "
			"			Patient.[Last] AS PatientLast,    "
			"			Patient.HomePhone AS PatientPhone,    "
			"			DrugList.DrugName AS Drug,    "
			"			PrescriptionQueueStatusT.InternalID AS StatusID,    "
			"			PrescriptionQueueStatusT.Status AS [Status],    "
			"			LocationsT.ID AS LocID,    "
			"			LocationsT.Name AS Location,    "
			"			UserEnteredBy.ID AS UserEnteredByID,    "
			"			UserEnteredBy.[First] AS UserEnteredByFirst,	    "
			"			UserEnteredBy.[Last] AS UserEnteredByLast,	    "
			"			UsersT.[Username] AS EnteredByUsername,	   						 	 "
			"			EMRDataT.Data AS Description, "
			"			PatientMedications.EnglishDescription AS EnglishDescription,   "
			"			PatientMedications.PatientExplanation AS SigPatientExplanation,     "
			"			CASE  "
			"			WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"			ELSE 1  "
			"			END AS SigPatientExplanationExists,   "
			"			PatientMedications.Quantity AS Quantity,  "
			"			PatientMedications.Unit AS Unit,    "
			"			PatientMedications.NoteToPharmacist AS Notes,    "
			"			CASE   "
			"			WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"			WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"			ELSE 1  "
			"			END AS NoteToPharmacistExists,	 "
			"			coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"			coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"			coalesce(Pharmacy.Phone,'') AS PharmacyPhone,	    "
			"			CASE     "
			"			WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"			WHEN FinalDestinationType = 1 THEN 'Print'     "
			"			WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"			WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"			WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"			WHEN FinalDestinationType = 5 THEN 'Test'    "
			"			ELSE ''     "
			"			END AS Route,    "
			"			DrugList.DEASchedule AS Sch,    "
			"			ProvT.ID AS ProvID,    "
			"			ProvT.First AS ProvFirst,    "
			"			ProvT.Last AS ProvLast,    "
			"			ProvPrefix.Prefix AS ProvPrefix,    "
			"			PatientMedications.RefillsAllowed AS Refills    "
			"						 	  			  "
			"		FROM PatientMedications "
			"			INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID "
			"			INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus "
			"			INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"			LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID "
			"			LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"			LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID "
			"			INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"			LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID "
			"			LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"			LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID "
			"		WHERE PatientMedications.Deleted <> 1  ";	

				return _T(strSql);
			}
		break;

			case 745:
			//Prescriptions by Diagnosis Code Report
			/* Version History
				(r.wilson 10/30/2012) plid 53462 -  Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			
			CString strSql =			
			"			    "	
			"	SELECT     "
			"		PatientMedications.ID AS PatientMedicationID,    "
			"		PrescriptionDate AS PrescriptionDate,    "
			"		Patient.ID AS PatID,    "
			"		Patient.[First] AS PatientFirst,    "
			"		Patient.[Last] AS PatientLast,    "
			"		Patient.HomePhone AS PatientPhone,    "
			"		DrugList.DrugName AS Drug,    "
			"		PrescriptionQueueStatusT.InternalID AS StatusID,    "
			"		PrescriptionQueueStatusT.Status AS [Status],    "
			"		LocationsT.ID AS LocID,    "
			"		LocationsT.Name AS Location,  			   "
			"		UserEnteredBy.[First] AS UserEnteredByFirst,  "
			"		UserEnteredBy.[Last] AS UserEnteredByLast,  "
			"		UsersT.UserName AS EnteredByUsername, 						 	 "
			"		EmrDataT.Data AS Description,  "
			"		PatientMedications.EnglishDescription AS EnglishDescription,  "
			"		PatientMedications.PatientExplanation AS SigPatientExplanation,  "
			"		CASE  "
			"		WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"		ELSE 1  "
			"		END AS SigPatientExplanationExists,   "
			"		PatientMedications.Quantity AS Quantity,  "
			"		PatientMedications.Unit AS Unit,       "
			"		PatientMedications.NoteToPharmacist AS Notes,    "
			"		CASE   "
			"		WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"		WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"		ELSE 1  "
			"		END AS NoteToPharmacistExists,	 "
			"		coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"		coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"		coalesce(Pharmacy.Phone, '') AS PharmacyPhone,	    "
			"		CASE     "
			"		WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"		WHEN FinalDestinationType = 1 THEN 'Print'     "
			"		WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"		WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"		WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"		WHEN FinalDestinationType = 5 THEN 'Test'    "
			"		ELSE ''     "
			"		END AS Route,    "
			"		DrugList.DEASchedule AS Sch,    "
			"		ProvT.ID AS ProvID,    "
			"		coalesce(ProvT.First, '') AS ProvFirst,    "
			"		coalesce(ProvT.Last, '') AS ProvLast,    "
			"		coalesce(ProvPrefix.Prefix, '') AS ProvPrefix,    "
			"		DiagCodes.ID AS DiagCodeID,    "
			"		DiagCodes.CodeNumber AS DiagCode,    "
			"		DiagCodes.CodeDesc AS DiagCodeDesc	   						    						 "
			" "
			"	FROM	 "
			"		PatientMedications INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID "
			"		INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus "
			"		INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"		LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID "
			"		LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"		LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID "
			"		INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"		LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID "
			"		LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"		LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID "
			"		INNER JOIN PatientMedicationDiagCodesT ON PatientMedications.ID = PatientMedicationDiagCodesT.PatientMedicationID		 "
			"		INNER JOIN DiagCodes ON PatientMedicationDiagCodesT.DiagCodeID =  DiagCodes.ID						 "
			"	WHERE "
			"		PatientMedications.Deleted <> 1 AND DiagCodes.Active <> 0 ";			
		
				return _T(strSql);
			}
		break;

		case 746:
			//Prescriptions by Patients Report
			/* Version History
				//(r.wilson 10/30/2012) plid 53463- Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			
			CString strSql =							
			"			  "
			"	SELECT     "
			"		PatientMedications.ID AS PatientMedicationID,    "
			"		PrescriptionDate AS PrescriptionDate,    "
			"		Patient.ID AS PatID,    "
			"		Patient.[First] AS PatientFirst,    "
			"		Patient.[Last] AS PatientLast,    "
			"		Patient.HomePhone AS PatientPhone,    "
			"		DrugList.DrugName AS Drug,    "
			"		PrescriptionQueueStatusT.InternalID AS StatusID,    "
			"		PrescriptionQueueStatusT.Status AS [Status],    "
			"		LocationsT.ID AS LocID,    "
			"		LocationsT.Name AS Location,    "
			"		UserEnteredBy.ID AS UserEnteredByID,    "
			"		UserEnteredBy.[First] AS UserEnteredByFirst,	    "
			"		UserEnteredBy.[Last] AS UserEnteredByLast,	    "
			"		UsersT.[Username] AS EnteredByUsername,	    "
			"		EmrDataT.Data AS Description,  "
			"		PatientMedications.EnglishDescription AS EnglishDescription,   "
			"		PatientMedications.PatientExplanation AS SigPatientExplanation,   "
			"		CASE  "
			"		WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"		ELSE 1  "
			"		END AS SigPatientExplanationExists,   "
			"		PatientMedications.Quantity AS Quantity,   "
			"		PatientMedications.Unit AS Unit,          "
			"		PatientMedications.NoteToPharmacist AS Notes,    "
			"		CASE   "
			"		WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"		WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"		ELSE 1  "
			"		END AS NoteToPharmacistExists,	 "
			"		coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"		coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"		coalesce(Pharmacy.Phone, '') AS PharmacyPhone,	    "
			"		CASE     "
			"		WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"		WHEN FinalDestinationType = 1 THEN 'Print'     "
			"		WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"		WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"		WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"		WHEN FinalDestinationType = 5 THEN 'Test'    "
			"		ELSE ''     "
			"		END AS Route,    "
			"		DrugList.DEASchedule AS Sch,    "
			"		ProvT.ID AS ProvID,    "
			"		ProvT.First AS ProvFirst,    "
			"		ProvT.Last AS ProvLast,    "
			"		ProvPrefix.Prefix AS ProvPrefix,    "
			"		PatientMedications.RefillsAllowed AS Refills    "
			"						 	  			  "
			"	FROM    "
			"		PatientMedications INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID   		 "
			"		INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus    "
			"		INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID  "
			"		INNER JOIN EMRDataT ON DrugList.EmrDataID = EmrDataT.ID   "
			"		LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID   "
			"		LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"		LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID			    "
			"		LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID  "
			"		LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID  "
			"		LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID =  ProvPrefix.ID 		  "
			"	WHERE  "
			"		PatientMedications.Deleted <> 1  ";
				return _T(strSql);
			}
		break;
		

		case 747:
			//Prescriptions by Transmission Type Report
			/* Version History
				//(r.wilson 11/1/2012) plid 53464 - Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			
			CString strSql =			
			"						     "		
			"	SELECT     "
			"		PatientMedications.ID AS PatientMedicationID,    "
			"		PrescriptionDate AS PrescriptionDate,    "
			"		Patient.ID AS PatID,    "
			"		Patient.[First] AS PatientFirst,    "
			"		Patient.[Last] AS PatientLast,    "
			"		Patient.HomePhone AS PatientPhone,    "
			"		DrugList.DrugName AS Drug,    "
			"		PrescriptionQueueStatusT.InternalID AS StatusID,    "
			"		PrescriptionQueueStatusT.Status AS [Status],    "
			"		LocationsT.ID AS LocID,    "
			"		LocationsT.Name AS Location,    "
			"		UserEnteredBy.ID AS UserEnteredByID,    "
			"		UserEnteredBy.[First] AS UserEnteredByFirst,	    "
			"		UserEnteredBy.[Last] AS UserEnteredByLast,	    "
			"		UsersT.[Username] AS EnteredByUsername,	    "
			"		EmrDataT.Data AS Description,  "
			"		PatientMedications.EnglishDescription AS EnglishDescription,    "
			"		PatientMedications.PatientExplanation AS SigPatientExplanation,    "
			"		CASE  "
			"		WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"		ELSE 1  "
			"		END AS SigPatientExplanationExists,   "
			"		PatientMedications.Quantity AS Quantity,    "
			"		PatientMedications.Unit AS Unit,             "
			"		PatientMedications.NoteToPharmacist AS Notes,    "
			"		CASE   "
			"		WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"		WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"		ELSE 1  "
			"		END AS NoteToPharmacistExists,	 "
			"		coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"		coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"		coalesce(Pharmacy.Phone, '') AS PharmacyPhone,	    "
			"		coalesce(FinalDestinationType, -2) AS RouteID,    "
			"		CASE     "
			"		WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"		WHEN FinalDestinationType = 1 THEN 'Print'     "
			"		WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"		WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"		WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"		WHEN FinalDestinationType = 5 THEN 'Test'    "
			"		ELSE ''     "
			"		END AS Route,    "
			"		DrugList.DEASchedule AS Sch,    "
			"		ProvT.ID AS ProvID,    "
			"		ProvT.First AS ProvFirst,    "
			"		ProvT.Last AS ProvLast,    "
			"		ProvPrefix.Prefix AS ProvPrefix,    "
			"		PatientMedications.RefillsAllowed AS Refills    "
			"						 	  			  "
			"	FROM    "
			"		PatientMedications INNER JOIN  PersonT AS Patient ON PatientMedications.PatientID = Patient.ID 			    "
			"		INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus    "
			"		INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"		INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID    "
			"		LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID  "
			"		LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID   "
			"		LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID		 		 "
			"		LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID "
			"		LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"		LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID    		 "
			"		WHERE  "
			"			PatientMedications.Deleted <> 1  ";

				return _T(strSql);
			}
		break;


		case 748:
			//Prescriptions Report by Insurance Company Report
			/* Version History
				//(r.wilson 11/13/2012) plid 53715- Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			
			CString strSql =		
			"															"
			"		SELECT      "
			"			PatientMedications.ID AS PatientMedicationID,     "
			"			PrescriptionDate AS PrescriptionDate,     "
			"			Patient.ID AS PatID,     "
			"			Patient.[First] AS PatientFirst,     "
			"			Patient.[Last] AS PatientLast,     "
			"			Patient.HomePhone AS PatientPhone,     "
			"			DrugList.DrugName AS Drug,     "
			"			PrescriptionQueueStatusT.InternalID AS StatusID,     "
			"			PrescriptionQueueStatusT.Status AS [Status],     "
			"			LocationsT.ID AS LocID,     "
			"			LocationsT.Name AS Location,     "
			"			coalesce(UserEnteredBy.ID, -1) AS UserEnteredByID,     "
			"			coalesce(UserEnteredBy.[First], '') AS UserEnteredByFirst,	     "
			"			coalesce(UserEnteredBy.[Last], '') AS UserEnteredByLast,	     "
			"			coalesce(UsersT.[Username], '') AS EnteredByUsername,	     "
			"			EmrDataT.Data AS Description,   "
			"			PatientMedications.EnglishDescription AS EnglishDescription,    "
			"			PatientMedications.PatientExplanation AS SigPatientExplanation,      "
			"			CASE   "
			"			WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0   "
			"			ELSE 1   "
			"			END AS SigPatientExplanationExists,    "
			"			PatientMedications.Quantity AS Quantity,   "
			"			PatientMedications.Unit AS Unit,     "
			"			PatientMedications.NoteToPharmacist AS Notes,     "
			"			CASE    "
			"			WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0   "
			"			WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"			ELSE 1   "
			"			END AS NoteToPharmacistExists,	  "
			"			coalesce(Pharmacy.ID, -1) AS PharmacyID,     "
			"			coalesce(Pharmacy.Name, '') AS Pharmacy,   "
			"			coalesce(Pharmacy.Phone,'') AS PharmacyPhone,	     "
			"			CASE      "
			"			WHEN FinalDestinationType = 0 THEN 'Not Transmitted'      "
			"			WHEN FinalDestinationType = 1 THEN 'Print'      "
			"			WHEN FinalDestinationType = 2 THEN 'Fax'      "
			"			WHEN FinalDestinationType = 3 THEN 'Electronic Retail'      "
			"			WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'      "
			"			WHEN FinalDestinationType = 5 THEN 'Test'     "
			"			ELSE ''      "
			"			END AS Route,     "
			"			DrugList.DEASchedule AS Sch,     "
			"			ProvT.ID AS ProvID,     "
			"			ProvT.First AS ProvFirst,     "
			"			ProvT.Last AS ProvLast,     "
			"			ProvPrefix.Prefix AS ProvPrefix,     "
			"			PatientMedications.RefillsAllowed AS Refills,  "
			"			coalesce(InsuranceCoT.PersonID, -2) AS InsuranceCoID,  "
			"			InsuranceCoT.Name AS InsuranceCoName,     "
			"			InsuranceCoT.InsType AS InsuranceCoType  "
			"									 	  			   "
			"		FROM     "
			"			PatientMedications INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID "			
			"			INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus     "
			"			INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID     "
			"			INNER JOIN EMRDataT ON DrugList.EmrDataID = EmrDataT.ID "
			"			LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID     "
			"			LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"			LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID	    			     "
			"			LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID     "
			"			LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"			LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID    "
			"			LEFT JOIN InsuredPartyT ON PatientMedications.PatientID = InsuredPartyT.PatientID  "
			"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID						  "
			"		WHERE  "
			"			PatientMedications.Deleted <> 1  ";

				return _T(strSql);
			}
		break;

		case 749:
			// Prescription Statistics
			/* Version History 	
				//(r.wilson 11/16/2012) plid 53804 - Created
				(j.fouts 2013-02-04 17:36) - PLID 53025 - Altered PrescriptionQueueStatusT structure
			*/
			{
			CString strSql =			
			"															"			
			"		SELECT      "
			"			PatientMedications.ID AS PatientMedicationID,     "
			"			PrescriptionDate AS PrescriptionDate,     "
			"			Patient.ID AS PatID,   						   "
			"			DrugList.DrugName AS Drug,     "
			"			PrescriptionQueueStatusT.InternalID AS StatusID,     "
			"			PrescriptionQueueStatusT.Status AS [Status],     "
			"			LocationsT.ID AS LocID,     "
			"			LocationsT.Name AS Location,   						 		     "
			"			EmrDataT.Data AS Description, 							  																					  "
			"			coalesce(Pharmacy.ID, -1) AS PharmacyID,     "
			"			coalesce(Pharmacy.Name, '') AS Pharmacy,   "
			"			coalesce(Pharmacy.Phone,'') AS PharmacyPhone,	     "
			"			FinalDestinationType AS RouteID, "
			"			CASE      "
			"			WHEN FinalDestinationType = 0 THEN 'Not Transmitted'      "
			"			WHEN FinalDestinationType = 1 THEN 'Print'      "
			"			WHEN FinalDestinationType = 2 THEN 'Fax'      "
			"			WHEN FinalDestinationType = 3 THEN 'Electronic Retail'      "
			"			WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'      "
			"			WHEN FinalDestinationType = 5 THEN 'Test'     "
			"			ELSE ''      "
			"			END AS Route, "
			"			 "
			"			DrugList.DEASchedule AS Sch,     "
			"			ProvT.ID AS ProvID,     "
			"			ProvT.First AS ProvFirst,     "
			"			ProvT.Last AS ProvLast,     "
			"			ProvPrefix.Prefix AS ProvPrefix,   						 	  "
			"			coalesce(InsuranceCoT.PersonID, -2) AS InsuranceCoID,  "
			"			InsuranceCoT.Name AS InsuranceCoName,  "
			"			InsuranceCoT.InsType AS InsuranceCoType,  "
			"			PatientMedications.NewCropGUID									 	  			   "
			"		FROM     "
			"			PatientMedications INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID    "
			"			INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.InternalID = PatientMedications.QueueStatus     "
			"			INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"			INNER JOIN EmrDataT ON DrugList.EmrDataID = EmrDataT.ID     "
			"			LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID     "
			"			LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"			LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID	  		  "
			"			LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID     "
			"			LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"			LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID   "
			"			LEFT JOIN InsuredPartyT ON PatientMedications.PatientID = InsuredPartyT.PatientID  "
			"			LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID					  "
			"		WHERE  "
			"			PatientMedications.Deleted <> 1  		 ";			

			return _T(strSql);
			}
		break;
		
		case 750:
			//Prescriptions By DEA Schedule
			/*
			Version History
			//(r.wilson 1/25/2013) pl 54856 - Created
			*/
			{
			CString strSql = ""
			"		SELECT     "
			"		PatientMedications.ID AS PatientMedicationID,    "
			"		PrescriptionDate AS PrescriptionDate,    "
			"		Patient.ID AS PatID,    "
			"		Patient.[First] AS PatientFirst,    "
			"		Patient.[Last] AS PatientLast,    "
			"		Patient.HomePhone AS PatientPhone,    "
			"		DrugList.DrugName AS Drug,    "
			"		PrescriptionQueueStatusT.ID AS StatusID,    "
			"		PrescriptionQueueStatusT.Status AS [Status],    "
			"		LocationsT.ID AS LocID,    "
			"		LocationsT.Name AS Location,    "
			"		UserEnteredBy.ID AS UserEnteredByID,    "
			"		UserEnteredBy.[First] AS UserEnteredByFirst,	    "
			"		UserEnteredBy.[Last] AS UserEnteredByLast,	    "
			"		UsersT.[Username] AS EnteredByUsername,	    "
			"		EmrDataT.Data AS Description,					 "
			"		PatientMedications.EnglishDescription AS EnglishDescription,  "
			"		PatientMedications.PatientExplanation AS SigPatientExplanation,    "
			"		CASE  "
			"		WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"		ELSE 1  "
			"		END AS SigPatientExplanationExists,   "
			"		PatientMedications.Quantity AS Quantity,  "
			"		PatientMedications.Unit AS Unit,  "
			"		PatientMedications.NoteToPharmacist AS Notes,    "
			"		CASE   "
			"		WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"		WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"		ELSE 1  "
			"		END AS NoteToPharmacistExists,	 "
			"		coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"		coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"		coalesce(Pharmacy.Phone, '') AS PharmacyPhone,					  "
			"		CASE     "
			"		WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"		WHEN FinalDestinationType = 1 THEN 'Print'     "
			"		WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"		WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"		WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"		WHEN FinalDestinationType = 5 THEN 'Test'    "
			"		ELSE ''     "
			"		END AS Route,    "
			"		DrugList.DEASchedule AS Sch, "
			"		CASE DrugList.DEASchedule "
			"		WHEN 'I' THEN 1 "
			"		WHEN 'II' THEN 2 "
			"		WHEN 'III' THEN 3 "
			"		WHEN 'IV' THEN 4 "
			"		WHEN 'V' THEN 5 "
			"		ELSE -1 "
			"		END AS SchInt,		  "
			"		ProvT.ID AS ProvID,    "
			"		ProvT.First AS ProvFirst,    "
			"		ProvT.Last AS ProvLast,    "
			"		coalesce(ProvPrefix.Prefix, '') AS ProvPrefix	   "
			"		FROM PatientMedications "
			"		INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID "
			"		INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.ID = PatientMedications.QueueStatus "
			"		INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"		LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID "
			"		LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"		LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID "
			"		INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"		LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID "
			"		LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"		LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID		 "
			"		WHERE PatientMedications.Deleted <> 1 ";
			return _T(strSql);
			}

			case 751:
			//Duplicate Medications Report
			/*
			Version History
			//(r.wilson 1/28/2013) pl 54896 - Created
			*/
			{
				CString strSql = ""
			" SELECT     "
			"		PatientMedications.ID AS PatientMedicationID,    "
			"		PrescriptionDate AS PrescriptionDate,    "
			"		Patient.ID AS PatID,    "
			"		Patient.[First] AS PatientFirst,    "
			"		Patient.[Last] AS PatientLast,    "
			"		Patient.HomePhone AS PatientPhone, "
			"		DrugList.ID AS DrugID,    "
			"		DrugList.DrugName AS Drug,    "
			"		PrescriptionQueueStatusT.ID AS StatusID,    "
			"		PrescriptionQueueStatusT.Status AS [Status],    "
			"		LocationsT.ID AS LocID,    "
			"		LocationsT.Name AS Location,    "
			"		UserEnteredBy.ID AS UserEnteredByID,    "
			"		UserEnteredBy.[First] AS UserEnteredByFirst,	    "
			"		UserEnteredBy.[Last] AS UserEnteredByLast,	    "
			"		UsersT.[Username] AS EnteredByUsername,	    "
			"		EmrDataT.Data AS Description,					 "
			"		PatientMedications.EnglishDescription AS EnglishDescription,  "
			"		PatientMedications.PatientExplanation AS SigPatientExplanation,    "
			"		CASE  "
			"		WHEN datalength(PatientMedications.PatientExplanation) = 0 THEN 0  "
			"		ELSE 1  "
			"		END AS SigPatientExplanationExists,   "
			"		PatientMedications.Quantity AS Quantity,  "
			"		PatientMedications.Unit AS Unit,  "
			"		PatientMedications.NoteToPharmacist AS Notes,    "
			"		CASE   "
			"		WHEN datalength(PatientMedications.NoteToPharmacist) = 0 THEN 0  "
			"		WHEN PatientMedications.NoteToPharmacist IS NULL THEN 0 "
			"		ELSE 1  "
			"		END AS NoteToPharmacistExists,	 "
			"		coalesce(Pharmacy.ID, -1) AS PharmacyID,    "
			"		coalesce(Pharmacy.Name, '') AS Pharmacy,  "
			"		coalesce(Pharmacy.Phone, '') AS PharmacyPhone,					  "
			"		CASE     "
			"		WHEN FinalDestinationType = 0 THEN 'Not Transmitted'     "
			"		WHEN FinalDestinationType = 1 THEN 'Print'     "
			"		WHEN FinalDestinationType = 2 THEN 'Fax'     "
			"		WHEN FinalDestinationType = 3 THEN 'Electronic Retail'     "
			"		WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder'     "
			"		WHEN FinalDestinationType = 5 THEN 'Test'    "
			"		ELSE ''     "
			"		END AS Route,    "
			"		DrugList.DEASchedule AS Sch,	  "
			"		ProvT.ID AS ProvID,    "
			"		ProvT.First AS ProvFirst,    "
			"		ProvT.Last AS ProvLast,    "
			"		coalesce(ProvPrefix.Prefix, '') AS ProvPrefix	   "
			"		FROM PatientMedications "
			"		INNER JOIN PersonT AS Patient ON PatientMedications.PatientID = Patient.ID "
			"		INNER JOIN PrescriptionQueueStatusT ON PrescriptionQueueStatusT.ID = PatientMedications.QueueStatus "
			"		INNER JOIN DrugList ON DrugList.ID = PatientMedications.MedicationID "
			"		LEFT JOIN LocationsT ON LocationsT.ID = PatientMedications.LocationID "
			"		LEFT JOIN PersonT AS UserEnteredBy ON PatientMedications.InputByUserID = UserEnteredBy.ID "
			"		LEFT JOIN UsersT ON UserEnteredBy.ID = UsersT.PersonID "
			"		INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"		LEFT JOIN LocationsT AS Pharmacy ON Pharmacy.ID = PatientMedications.PharmacyID "
			"		LEFT JOIN PersonT AS ProvT ON PatientMedications.ProviderID = ProvT.ID "
			"		LEFT JOIN PrefixT AS ProvPrefix ON ProvT.PrefixID = ProvPrefix.ID		 "
			"		INNER JOIN ( "
			"						SELECT PatientMedications.MedicationID, PatientMedications.PatientID "
			"						FROM PatientMedications "
			"						WHERE DELETED <> 1 "
			"						GROUP BY PatientMedications.MedicationID, PatientMedications.PatientID						 "
			"						HAVING COUNT(*) > 1												 "
			"					)  DuplicateMeds1 ON DuplicateMeds1.MedicationID = PatientMedications.MedicationID AND DuplicateMeds1.PatientID = PatientMedications.PatientID	 "
			"		WHERE PatientMedications.Deleted <> 1  "
			"		";
			
				return _T(strSql);
			}

		default:
			return _T("");
			break;
	}
}
