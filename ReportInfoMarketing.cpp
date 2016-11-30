////////////////
// DRT 8/6/03 - GetSqlMarketing() function from ReportInfoCallback
//

#include "stdafx.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "DateTimeUtils.h"

using namespace ADODB;

// TODO: Use the actual SQL limits here
const COleDateTime dtMin(1800, 1, 1, 0, 0, 0);
const COleDateTime dtMax(2030, 12, 31, 0, 0, 0);
const COleDateTime dtToday = COleDateTime::GetCurrentTime();



// (d.thompson 2012-09-14) - PLID 52122 - Only used for the 2 weekly advertising reports
CString BuildWeeklyAdvertisingLocationFilter(long nLocation, CDWordArray *pdwLocations)
{
	CString strLocationFilter;

	// (d.thompson 2012-09-13) - PLID 52122 - We allow multiple location filters, so we need to support them
	//	in the report data.  I had to update the UDF to support an XML document as the only way to pass this in
	//	(UDF's don't current support temp tables, exec statements, or table variable parameters)
	if (nLocation > 0 || nLocation == -3) {
		//This has to be an XML document formatted just so...
		strLocationFilter = "'<root>";
		for(int i=0; i < pdwLocations->GetSize(); i++) {
			strLocationFilter += FormatString("<loc ID=\"%li\" />", pdwLocations->GetAt(i));
		}
		strLocationFilter += "</root>'";
	} else if(nLocation == -1) {
		// No location filter chosen. Do not do any kind of location filtering. It's perfectly fine even if
		// the patient's location is not the same as the referral's location.
		// (d.thompson 2012-09-14) - PLID 52122 - An empty string results in "all" behavior (it will also work 
		//	with an empty set of xml data);
		strLocationFilter = "''";
	}
	else {
		AfxThrowNxException("Unsupported location filtering.  Please choose a valid location.");
	}

	return strLocationFilter;
}

// (d.thompson 2012-09-14) - PLID 52122 - Only used for the 2 weekly advertising reports
CString BuildWeeklyAdvertisingPatientLocationFilter(long nLocation, CDWordArray *pdwLocations)
{
	CString strPatientLocationFilter = "";

	if (nLocation > 0 || nLocation == -3) {
		// (d.thompson 2012-09-14) - PLID 52122 - Also need to support multiple locations here
		strPatientLocationFilter.Format("AND PersonT.Location IN (%s)", GenerateDelimitedListFromLongArray(*pdwLocations, ","));
	} else if(nLocation == -1) {
		// No location filter chosen. Do not do any kind of location filtering. It's perfectly fine even if
		// the patient's location is not the same as the referral's location.
		strPatientLocationFilter = "";
	}
	else {
		AfxThrowNxException("Unsupported location filtering.  Please choose a valid location.");
	}

	return strPatientLocationFilter;
}


// (j.luckoski 2013-03-26 16:15) - PLID 53755 - Set up for multiple locations
CString BuildAdvertisingCostLocationFilter(long nLocation, CDWordArray *pdwLocations)
{
	CString strLocationFilter = "";

	if (nLocation > 0 || nLocation == -3) {
		strLocationFilter.Format("AND MarketingCostsT.LocationID IN (%s)", GenerateDelimitedListFromLongArray(*pdwLocations, ","));
	} else if(nLocation == -1) {
		strLocationFilter = "";
	}
	else if(nLocation == -2) {
		strLocationFilter.Format("AND MarketingCostsT.LocationID IS NULL");
	}
	else {
		AfxThrowNxException("Unsupported location filtering.  Please choose a valid location.");
	}

	return strLocationFilter;
}

// (j.luckoski 2013-03-26 16:15) - PLID 53755 - Set up for multiple locations
CString BuildAdvertisingCostMasterLocationFilter(long nLocation, CDWordArray *pdwLocations)
{
	CString strMasterLocationFilter = "";

	if (nLocation > 0 || nLocation == -3) {
		strMasterLocationFilter.Format("AND ReferralSourceT.PersonID IN (SELECT ReferralSource FROM MarketingCostsT M WHERE M.LocationID IN (%s))", GenerateDelimitedListFromLongArray(*pdwLocations, ","));
	} else if(nLocation == -1) {
		strMasterLocationFilter = "";
	} 
	else if(nLocation == -2) {
		strMasterLocationFilter.Format("AND ReferralSourceT.PersonID IN (SELECT ReferralSource FROM MarketingCostsT M WHERE M.LocationID IS NULL)");
	}
	else {
		AfxThrowNxException("Unsupported location filtering.  Please choose a valid location.");
	}

	return strMasterLocationFilter;
}


CString CReportInfo::GetSqlMarketing(long nSubLevel, long nSubRepNum) const
{
	CString strSQL, strArSql;
	switch (nID) {

		/*case 13:
		//Patient Interests
		return _T("SELECT PatientInterestsT.PatientID AS PatID,  "
		"    InterestsT.SurgeryList AS Interest, PatientInterestsT.InterestID AS InterestID,  "
		"    PatientsT.[MainPhysician] AS ProvID, PersonT.Address1,  "
		"    PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
		"    PersonT.HomePhone, InterestsT.Cost, "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS "
		"     ProvName, PatientInterestsT.[LevelNum], "
		"PatientsT.UserDefinedID, PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.FirstContactDate AS Date "
		"FROM InterestsT RIGHT OUTER JOIN "
		"    PersonT PersonT1 INNER JOIN "
		"    PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID ON  "
		"    PersonT1.ID = PatientsT.[MainPhysician] INNER JOIN "
		"    PatientInterestsT ON  "
		"    PersonT.ID = PatientInterestsT.PatientID ON  "
		"    InterestsT.ID = PatientInterestsT.InterestID "
		"WHERE (PatientsT.PersonID > 0)");
		break;*/
	

	
case 24:
		//Referrals
		/*Version History
			TES 2/11/2008 - PLID 28720 - Revamped to include the ancestors of every item, down to four levels, thus
				enabling the report to show a tree and cumulative totals.  Similar to what the Effectiveness report
				does, though I chose to implement the query in a much different way.
			DRT 8/11/2008 - PLID 30876 - Added the inputting user of the patient who is referred.  I also added the userID
				in case we wanted to do a filter on that user in the future, but it's unused as of now.  Neither field
				is in the default report.
	    */
		return _T(
			/*We'll iterate more or less "recursively" through the referral tree, filling out a table variable with the 
				parents	of each referral up to 4 levels deep.*/
			"SET NOCOUNT ON "
			"DECLARE @ReferralTree TABLE (ReferralID INT NOT NULL, Level1ID INT NOT NULL, "
			"Level1Name nvarchar(50) NOT NULL, Level2ID INT, Level2Name nvarchar(50), Level3ID INT, "
			"Level3Name nvarchar(50), Level4ID INT, Level4Name nvarchar(50) ) "
			" "
			"INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name) "
			"    SELECT PersonID, PersonID, Name FROM ReferralSourceT WHERE Parent = -1; "
			" "
			"DECLARE @Level1ID INT "
			"DECLARE @Level1Name nvarchar(50) "
			"DECLARE Level1Cursor CURSOR STATIC OPTIMISTIC FOR "
			"SELECT Level1ID, Level1Name FROM @ReferralTree "
			"OPEN Level1Cursor  "
			"FETCH NEXT FROM Level1Cursor INTO @Level1ID, @Level1Name "
			"WHILE @@FETCH_STATUS = 0 BEGIN "
			"    INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name, Level2ID, Level2Name) "
			"        SELECT PersonID, @Level1ID, @Level1Name, PersonID, Name FROM ReferralSourceT WHERE Parent = @Level1ID; "
			" "
			"    DECLARE @Level2ID INT "
			"    DECLARE @Level2Name nvarchar(50) "
			"    DECLARE Level2Cursor CURSOR STATIC OPTIMISTIC FOR "
			"    SELECT Level2ID, Level2Name FROM @ReferralTree WHERE Level1ID = @Level1ID "
			"    OPEN Level2Cursor "
			"    FETCH NEXT FROM Level2Cursor INTO @Level2ID, @Level2Name "
			"    WHILE @@FETCH_STATUS = 0 BEGIN "
			"        INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name, Level2ID, Level2Name, Level3ID, Level3Name) "
			"            SELECT PersonID, @Level1ID, @Level1Name, @Level2ID, @Level2Name, PersonID, Name FROM ReferralSourceT WHERE Parent = @Level2ID; "
			" "
			"        DECLARE @Level3ID INT "
			"        DECLARE @Level3Name nvarchar(50) "
			"        DECLARE Level3Cursor CURSOR STATIC OPTIMISTIC FOR "
			"        SELECT Level3ID, Level3Name FROM @ReferralTree WHERE Level2ID = @Level2ID "
			"        OPEN Level3Cursor "
			"        FETCH NEXT FROM Level3Cursor INTO @Level3ID, @Level3Name "
			"        WHILE @@FETCH_STATUS = 0 BEGIN "
			"            INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name, Level2ID, Level2Name, Level3ID, Level3Name, Level4ID, Level4Name) "
			"                SELECT PersonID, @Level1ID, @Level1Name, @Level2ID, @Level2Name, @Level3ID, @Level3Name, PersonID, Name FROM ReferralSourceT WHERE Parent = @Level3ID; "
			"         "
			"            FETCH NEXT FROM Level3Cursor INTO @Level3ID, @Level3Name "
			"        END "
			"        CLOSE Level3Cursor "
			"        DEALLOCATE Level3Cursor "
			"         "
			"        FETCH NEXT FROM Level2Cursor INTO @Level2ID, @Level2Name "
			"    END "
			"    CLOSE Level2Cursor "
			"    DEALLOCATE Level2Cursor "
			"     "
			" "
			"    FETCH NEXT FROM Level1Cursor INTO @Level1ID, @Level1Name "
			"END "
			"CLOSE Level1Cursor "
			"DEALLOCATE Level1Cursor "
			" "
			"SET NOCOUNT OFF "
			" "
			/*OK, now run our actual query, pulling from the table variable we just filled to give us the ancestors of 
				each referral.*/
			"SELECT RefTree.Level1Name, RefTree.Level2Name, RefTree.Level3Name, RefTree.Level4Name, "
			" PersonT.Last + ', ' + PersonT.Middle + ' ' + PersonT.First AS PatName,  "
			"     CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS ReferralName,   "
			"     PersonT.FirstContactDate AS Date, PersonT.Location AS LocID, LocationsT.Name AS Location,  "
			"     PersonT.ID AS PatID, PatientsT.MainPhysician AS ProvID,  "
			"CASE WHEN PatientsT.CurrentStatus = 1 THEN 'Patient'   "
			"     WHEN PatientsT.CurrentStatus = 2 THEN 'Prospect'  "
			"     WHEN PatientsT.CurrentStatus = 3 THEN 'Patient Prospect' "
			"	  WHEN PatientsT.CurrentStatus = 4 THEN 'Inquiry' "
			"END AS CurrentStatus, PatientsT.CurrentStatus AS StatusID, PatientsT.ReferralID AS ReferralID, "
			"     (SELECT TOP 1 Date FROM AppointmentsT WHERE AppointmentsT.PatientID = PatientsT.PersonID AND Status <> 4 ORDER BY DATE ASC) AS FirstApptDate, "
			" (SELECT Top 1 AptTypeT.Name FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID WHERE AppointmentsT.PatientID = PatientsT.PersonID AND Status <> 4 AND AppointmentsT.Date > "
			"	(SELECT Top 1 Date FROM AppointmentsT WHERE AppointmentsT.PatientID = PatientsT.PersonID AND Status <> 4 ORDER BY DATE ASC) ORDER BY DATE ASC) "
			" AS ApptTypeAfterFirstAppt, "
			"UsersT.PersonID AS InputtingUserID, UsersT.Username AS InputtingUser "
			" FROM @ReferralTree AS RefTree INNER JOIN ReferralSourceT ON RefTree.ReferralID = ReferralSourceT.PersonID "
			"    INNER JOIN  PatientsT ON   "
			"    ReferralSourceT.PersonID = PatientsT.ReferralID INNER JOIN  "
			"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID  "
			"	 LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID "
//			"WHERE (PersonT.FirstContactDate IS NOT NULL)  "
			/*The above WHERE clause was commented out long ago for whatever reason, but we do need a WHERE clause here.
				Otherwise, when the code adds the filters, it will find one of the WHERE clauses up in the code where we
				fill the table variable, and add the filters there.*/
			" WHERE 1=1 "
			"GROUP BY RefTree.Level1Name, RefTree.Level2Name, RefTree.Level3Name, RefTree.Level4Name, PersonT.First, PersonT.Middle, PersonT.Last,   "
			"    CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END, PersonT.FirstContactDate, PersonT.Location, LocationsT.Name, PersonT.ID, PatientsT.MainPhysician,  "
			"    CASE WHEN PatientsT.CurrentStatus = 1 THEN 'Patient'   "
			"    WHEN PatientsT.CurrentStatus = 2 THEN 'Prospect'  "
			"    WHEN PatientsT.CurrentStatus = 3 THEN 'Patient Prospect'  "
			"	 WHEN PatientsT.CurrentStatus = 4 THEN 'Inquiry' "
			"    END, PatientsT.CurrentStatus, PatientsT.ReferralID, "
			"    PatientsT.PersonID, UsersT.PersonID, UsersT.Username ");
		break;

	case 378:
		//Referrals (all)
		/*	Version History
			DRT 3/14/03 - Created.  Based off the Referrals report, but includes non-primary referrals
			DRT 6/23/03 - AHA!!  The filter is supposed to work off the multireferral id, but there was also selected
					PatientsT.ReferralID AS ReferralID.  Since the parsing works in reverse, it decided that was what
					we wanted to filter on, and the report was showing all referrals where the patient had a main referral
					of what was chosen.
			DRT 8/11/2008 - PLID 30876 - Added the inputting user of the patient who is referred.  I also added the userID
				in case we wanted to do a filter on that user in the future, but it's unused as of now.  Neither field
				is in the default report.
		*/
		return _T("SELECT PersonT.Last + ', ' + PersonT.Middle + ' ' + PersonT.First AS PatName, MultiReferralsT.ReferralID AS ReferralID, "
			"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS ReferralName, "
			"PersonT.FirstContactDate AS FCDate, PersonT.Location AS LocID, LocationsT.Name AS Location, "
			"PersonT.ID AS PatID, PatientsT.MainPhysician AS ProvID, "
			"CASE WHEN PatientsT.CurrentStatus = 1 THEN 'Patient' "
			"     WHEN PatientsT.CurrentStatus = 2 THEN 'Prospect' "
			"     WHEN PatientsT.CurrentStatus = 3 THEN 'Patient Prospect' "
			"	  WHEN PatientsT.CurrentStatus = 4 THEN 'Inquiry' "
			"END AS CurrentStatus, PatientsT.CurrentStatus AS StatusID, PatientsT.ReferralID AS PrimaryReferralID, "
			"CASE WHEN PatientsT.ReferralID = ReferralSourceT.PersonID THEN 1 ELSE 0 END AS PrimarySrc, "
			"MultiReferralsT.Date AS Date, "
			"PriReferralsT.Name AS PriRefSource, "
			"(SELECT TOP 1 Date FROM AppointmentsT WHERE AppointmentsT.PatientID = PatientsT.PersonID AND Status <> 4 ORDER BY DATE ASC) AS FirstApptDate, "
			"UsersT.PersonID AS InputtingUserID, UsersT.Username AS InputtingUser "
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN ReferralSourceT PriReferralsT ON PatientsT.ReferralID = PriReferralsT.PersonID "
			"LEFT JOIN MultiReferralsT ON PersonT.ID = MultiReferralsT.PatientID "
			"LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
			"LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID "
			" "
			"WHERE MultiReferralsT.ReferralID IS NOT NULL");
		break;

	/*case 106:
		//Patient Interests By Interest Level
		return _T("SELECT PatientInterestsT.PatientID AS PatID,  "
		"    InterestsT.SurgeryList AS Interest, PatientInterestsT.InterestID AS InterestID,  "
		"    PatientsT.[MainPhysician] AS ProvID, PersonT.Address1,  "
		"    PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip,  "
		"    PersonT.HomePhone, InterestsT.Cost,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"     PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS "
		"     ProvName, PatientInterestsT.[LevelNum], PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.FirstContactDate AS Date "
		"FROM InterestsT RIGHT OUTER JOIN "
		"    PersonT PersonT1 INNER JOIN "
		"    PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID ON  "
		"    PersonT1.ID = PatientsT.[MainPhysician] INNER JOIN "
		"    PatientInterestsT ON  "
		"    PersonT.ID = PatientInterestsT.PatientID ON  "
		"    InterestsT.ID = PatientInterestsT.InterestID "
		"WHERE (PatientsT.PersonID > 0)"); 
		break;	   */
	

	/*Patient Interest By Zip Code
	case 107:
		return _T("SELECT PatientInterestsT.PatientID AS PatID,  "
		"    InterestsT.SurgeryList AS Interest, PatientInterestsT.InterestID,  "
		"    PatientsT.[MainPhysician] AS ProvID, PersonT.Address1,  "
		"    PersonT.Address2, PersonT.City, PersonT.State, CASE WHEN Right(PersonT.Zip,1) = '-' THEN Left(PersonT.Zip, Len(PersonT.Zip)-1) ELSE PersonT.Zip END AS ZipCode,  "
		"    PersonT.HomePhone, InterestsT.Cost,  "
		"    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
		"    PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS "
		"    ProvName, PatientInterestsT.LevelNum, PersonT.Location AS LocID, LocationsT.Name AS Location, PersonT.FirstContactDate AS Date "
		"FROM InterestsT RIGHT OUTER JOIN "
		"    PersonT PersonT1 INNER JOIN "
		"    PatientsT INNER JOIN "
		"    (PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID) ON PatientsT.PersonID = PersonT.ID ON  "
		"    PersonT1.ID = PatientsT.[MainPhysician] INNER JOIN "
		"    PatientInterestsT ON  "
		"    PersonT.ID = PatientInterestsT.PatientID ON  "
		"    InterestsT.ID = PatientInterestsT.InterestID "
		"WHERE (PatientsT.PersonID > 0)");	  
		break;			   */
	

	case 190:
		//Marketing Effectiveness
		/*	Version History
			DRT 10/17/2003 - PLID 9856 - Fixed a number of issues in the MEChargesQ subquery.  It was listing "Last Bill Date", and "Description", 
				but it was really pulling Max(Charge date), and Max(Charge description).  I fixed those to actually be the last bill date / description
				fields.  I also fixed a bug where it was calculating the total wrong - The query was only looking at unit cost and tax, ignoring all
				modifiers and other such things.  I changed it to SUM(ChargeRespT), which is safer.
		*/
		return _T("	/* Start query from hell */ "
			"SELECT PatientsT.PersonID AS PatID,  "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName,  "
			"PersonT.FirstContactDate AS Date,  "
			"MeChargesQ.Date AS ChargeDate,  "
			"MeChargesQ.Description AS ChargeDesc,  "
			"MeChargesQ.ChargeAmt,  "
			"MePaymentsQ.SumOfAmount AS PayAmt,  "
			"MeLevelsQ.F1, MeLevelsQ.F1ID,  "
			"MeLevelsQ.F2, MeLevelsQ.F2ID,  "
			"MeLevelsQ.F3, MeLevelsQ.F3ID,  "
			"MeLevelsQ.F4 AS ReferralName,  "
			"MeLevelsQ.F4ID AS ReferralID, "
			"(SELECT TOP 1 Date FROM AppointmentsT WHERE AppointmentsT.PatientID = PersonT.ID AND Status <> 4 ORDER BY DATE ASC) AS FirstApptDate "
			" "
			"FROM "
			"(/*MELevelsQ*/ "
			"/*MEf1_f2_f3_f4q*/ "
			"(SELECT F1_F2_F3Q.F1, F1_F2_F3Q.F1ID,  "
			"F1_F2_F3Q.F1Parent, F1_F2_F3Q.F2,  "
			"F1_F2_F3Q.F2ID, F1_F2_F3Q.F2Parent,  "
			"F1_F2_F3Q.F3, F1_F2_F3Q.F3ID,  "
			"F1_F2_F3Q.F3Parent,  "
			" "
			"/*IIf([Parent]=[F1ID],[F3], "
			"IIf([Parent]=[F2ID],[F3], "
			"IIf([Parent]=[F3ID],[Name]))) AS F4, */ "
			"(CASE "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F3 "
			" ELSE  "
			"   (CASE "
			"    WHEN RSD.Parent = F2ID  "
			"    THEN F3  "
			"    ELSE "
			"       (CASE  "
			"        WHEN RSD.Parent = F3ID "
			"        THEN NAME END) "
			"    END) "
			"END) F4, "
			" "
			"/*IIf([Parent]=[F1ID],[F3ID], "
			"IIf([Parent]=[F2ID],[F3ID], "
			"IIf([Parent]=[F3ID],[ID]))) AS F4ID, */ "
			"(CASE "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F1_F2_F3Q.F3ID "
			" ELSE "
			"   (CASE  "
			"    WHEN RSD.Parent = F1_F2_F3Q.F2ID "
			"    THEN F1_F2_F3Q.F3ID "
			"    ELSE "
			"      (CASE "
			"       WHEN RSD.Parent = F1_F2_F3Q.F3ID "
			"       THEN RSD.PersonID END) "
			"    END) "
			"END) F4ID, "
			" "
			"/*IIf([Parent]=[F1ID],[F3Parent], "
			"IIf([Parent]=[F2ID],[F3Parent], "
			"IIf([Parent]=[F3ID],[Parent]))) AS F4Parent*/ "
			"(CASE  "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F1_F2_F3Q.F3Parent "
			" ELSE "
			"   (CASE "
			"    WHEN RSD.Parent = F2ID "
			"    THEN F1_F2_F3Q.F3Parent "
			"    ELSE "
			"      (CASE  "
			"       WHEN RSD.Parent = F1_F2_F3Q.F3ID "
			"       THEN RSD.Parent END) "
			"    END) "
			" END) F4Parent "
			" "
			"FROM  "
			"ReferralSourceT AS RSD,  "
			"( "
			"SELECT F1_F2Q.F1, F1_F2Q.F1ID, F1_F2Q.F1Parent,  "
			"F1_F2Q.F2, F1_F2Q.F2ID, F1_F2Q.F2Parent,  "
			"/* IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2], "
			"IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[Name])) AS F3, */ "
			"(CASE "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2 "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.Name END) "
			"END) F3, "
			" "
			"/*IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2ID], "
			"IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[ID])) AS F3ID, */ "
			"(CASE "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2ID "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.PersonID END) "
			"END) F3ID, "
			" "
			"/*IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2Parent], "
			"IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[Parent])) AS F3Parent */ "
			" "
			"(CASE  "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2Parent "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.Parent END) "
			"END) F3Parent "
			" "
			"FROM  "
			"[ReferralSourceT] AS RSC, "
			"(SELECT RSA.Name AS F1, RSA.PersonID AS F1ID, RSA.Parent AS F1Parent,  "
			"(CASE  "
			"WHEN RSB.Parent = RSA.PersonID  "
			"THEN RSB.Name  "
			"ELSE  "
			"   (CASE WHEN RSB.Parent = -1 THEN RSA.Name END) END) F2, "
			"(CASE  "
			"WHEN RSB.Parent = -1  "
			"THEN RSA.PersonID "
			"ELSE  "
			"    RSB.PersonID END) F2ID, "
			"RSB.Parent AS F2Parent "
			"FROM [ReferralSourceT] AS RSA, [ReferralSourceT] AS RSB "
			"WHERE (RSA.Parent=-1) "
			"GROUP BY RSA.Name, RSA.PersonID, RSA.Parent,  "
			"(CASE  "
			"WHEN RSB.Parent = RSA.PersonID  "
			"THEN RSB.Name  "
			"ELSE  "
			"   (CASE WHEN RSB.Parent = -1 THEN RSA.Name END) END), "
			"(CASE  "
			"WHEN RSB.Parent = -1  "
			"THEN RSA.PersonID "
			"ELSE  "
			"    RSB.PersonID END), "
			"RSB.Parent "
			"HAVING (CASE  "
			"WHEN RSB.Parent = RSA.PersonID  "
			"THEN RSB.Name "
			"ELSE "
			"   (CASE "
			"    WHEN RSB.Parent = -1  "
			"    THEN RSA.Name END) "
			"END) Is Not Null) AS F1_F2Q "
			" "
			"GROUP BY  "
			"F1_F2Q.F1, F1_F2Q.F1ID,  "
			"F1_F2Q.F1Parent, F1_F2Q.F2, F1_F2Q.F2ID,  "
			"F1_F2Q.F2Parent, "
			"/*IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2],IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[Name])), */ "
			"(CASE "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2 "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.Name END) "
			"END), "
			" "
			"/*IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2ID],IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[ID])), */ "
			"(CASE "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2ID "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.PersonID END) "
			"END), "
			" "
			"/*IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2Parent],IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[Parent]))*/ "
			"(CASE  "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2Parent "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.Parent END) "
			"END) "
			" "
			"HAVING  "
			"/*(((IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2], "
			"IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[Name]))) Is Not Null) */ "
			"(CASE "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2 "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.Name END) "
			"END) Is Not Null "
			"AND  "
			"/*((IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F1ID],[_MarketingEffectivenessF1_F2Q].[F2Parent], "
			"IIf([RSC].[Parent]=[_MarketingEffectivenessF1_F2Q].[F2ID],[RSC].[Parent]))) Is Not Null))*/ "
			"(CASE "
			"WHEN RSC.Parent = F1_F2Q.F1ID "
			"THEN F1_F2Q.F2Parent "
			"ELSE "
			"   (CASE "
			"    WHEN RSC.Parent = F1_F2Q.F2ID "
			"    THEN RSC.Parent END) "
			"END) Is Not Null "
			") F1_F2_F3Q "
			" "
			"GROUP BY  "
			"F1_F2_F3Q.F1, F1_F2_F3Q.F1ID, F1_F2_F3Q.F1Parent, F1_F2_F3Q.F2, F1_F2_F3Q.F2ID,  "
			"F1_F2_F3Q.F2Parent, F1_F2_F3Q.F3, F1_F2_F3Q.F3ID, F1_F2_F3Q.F3Parent,  "
			" "
			"/*IIf([Parent]=[F1ID],[F3], "
			"IIf([Parent]=[F2ID],[F3], "
			"IIf([Parent]=[F3ID],[Name]))) AS F4, */ "
			"(CASE "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F3 "
			" ELSE  "
			"   (CASE "
			"    WHEN RSD.Parent = F2ID  "
			"    THEN F3  "
			"    ELSE "
			"       (CASE  "
			"        WHEN RSD.Parent = F3ID "
			"        THEN NAME END) "
			"    END) "
			"END), "
			" "
			"/*IIf([Parent]=[F1ID],[F3ID], "
			"IIf([Parent]=[F2ID],[F3ID], "
			"IIf([Parent]=[F3ID],[ID]))) AS F4ID, */ "
			"(CASE "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F1_F2_F3Q.F3ID "
			" ELSE "
			"   (CASE  "
			"    WHEN RSD.Parent = F1_F2_F3Q.F2ID "
			"    THEN F1_F2_F3Q.F3ID "
			"    ELSE "
			"      (CASE "
			"       WHEN RSD.Parent = F1_F2_F3Q.F3ID "
			"       THEN RSD.PersonID END) "
			"    END) "
			"END), "
			" "
			"/*IIf([Parent]=[F1ID],[F3Parent], "
			"IIf([Parent]=[F2ID],[F3Parent], "
			"IIf([Parent]=[F3ID],[Parent]))) AS F4Parent*/ "
			"(CASE  "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F1_F2_F3Q.F3Parent "
			" ELSE "
			"   (CASE "
			"    WHEN RSD.Parent = F2ID "
			"    THEN F1_F2_F3Q.F3Parent "
			"    ELSE "
			"      (CASE  "
			"       WHEN RSD.Parent = F1_F2_F3Q.F3ID "
			"       THEN RSD.Parent END) "
			"    END) "
			" END) "
			" "
			"HAVING  "
			"/*(((IIf([Parent]=[F1ID],[F3], "
			"IIf([Parent]=[F2ID],[F3], "
			"IIf([Parent]=[F3ID],[Name])))) "
			" Is Not Null));*/ "
			"(CASE  "
			" WHEN RSD.Parent = F1_F2_F3Q.F1ID "
			" THEN F1_F2_F3Q.F3 "
			" ELSE "
			"   (CASE "
			"    WHEN RSD.Parent = F1_F2_F3Q.F2ID "
			"    THEN F1_F2_F3Q.F3 "
			"    ELSE "
			"      (CASE  "
			"       WHEN RSD.Parent = F1_F2_F3Q.F3ID "
			"       THEN RSD.Name END) "
			"    END) "
			" END) Is Not Null) "
			" "
			"/*MERootsQ*/ "
			"UNION  "
			"(SELECT ReferralSourceT.Name AS F1, ReferralSourceT.PersonID AS F1ID,  "
			"ReferralSourceT.Parent AS F1Parent, ReferralSourceT.Name AS F2Name,  "
			"ReferralSourceT.PersonID AS F2ID, ReferralSourceT.Parent AS F2Parent,  "
			"ReferralSourceT.Name AS F3Name, ReferralSourceT.PersonID AS F3ID,  "
			"ReferralSourceT.Parent AS F3Parent, ReferralSourceT.Name AS F4Name,  "
			"ReferralSourceT.PersonID AS F4ID, ReferralSourceT.Parent AS F4Parent "
			" "
			"FROM ReferralSourceT LEFT JOIN  "
			"ReferralSourceT AS [Referral Source_2] ON  "
			"ReferralSourceT.PersonID = [Referral Source_2].Parent "
			" "
			"GROUP BY ReferralSourceT.Name, ReferralSourceT.PersonID, ReferralSourceT.Parent,  "
			"ReferralSourceT.Name, ReferralSourceT.PersonID,  "
			"ReferralSourceT.Parent, ReferralSourceT.Name, ReferralSourceT.PersonID,  "
			"ReferralSourceT.Parent, ReferralSourceT.Name, ReferralSourceT.PersonID,  "
			"ReferralSourceT.Parent, [Referral Source_2].PersonID "
			" "
			"HAVING (((ReferralSourceT.Parent)=-1) AND (([Referral Source_2].PersonID) Is Null))) "
			"/*End MELevelsQ */ "
			") MELevelsQ LEFT OUTER JOIN "
			"PatientsT ON "
			"MELevelsQ.F4ID = PatientsT.ReferralID INNER JOIN "
			"PersonT ON "
			"PatientsT.PersonID = PersonT.ID LEFT OUTER JOIN "
			" "
			" "
			"(  "
			"/*MeChargesQ*/  "
			"  "
			"SELECT PatientsT.PersonID AS PatID, BillsT.Date AS Date, BillsT.Description AS Description,  "
			"SUM(ChargeRespT.Amount) AS ChargeAmt, LineItemT.PatientID "
			"FROM "
			"PatientsT LEFT JOIN LineItemT ON PatientsT.PersonID = LineItemT.PatientID "
			"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
			"LEFT JOIN  "
			"	(SELECT Max(ID) AS BillID, PatientID FROM BillsT WHERE Deleted = 0 GROUP BY PatientID) LastBillQ "
			"ON PatientsT.PersonID = LastBillQ.PatientID "
			"LEFT JOIN BillsT ON LastBillQ.BillID = BillsT.ID "
			" "
			"WHERE LineItemT.Type = 10 AND LineItemT.Deleted = 0 "
			"GROUP BY PersonID, BillsT.Date, BillsT.Description, LineItemT.PatientID "
			"  "
			"/*End MeChargesQ*/  "
			")MEChargesQ ON "
			" "
			"PatientsT.PersonID = MEChargesQ.PatientID LEFT OUTER JOIN "
			" "
			"( "
			"/*MePaymentsQ*/ "
			" "
			"SELECT  "
			"PatientsT.PersonID AS PatID, Max(LineItemT.Date) AS Date,  "
			"Max(LineItemT.Description) AS Description,  "
			"Sum(Round(Convert(money,LineItemT.Amount),2)) AS SumOfAmount, LineItemT.PatientID "
			" "
			"FROM  "
			"(PatientsT LEFT JOIN  "
			"LineItemT ON  "
			"PatientsT.PersonID = LineItemT.PatientID) LEFT JOIN  "
			"PaymentsT ON  "
			"LineItemT.ID = PaymentsT.ID "
			" "
			"WHERE (((LineItemT.Type=1) Or (LineItemT.Type=3)) AND (LineItemT.Deleted = 0)) "
			" "
			"GROUP BY PatientsT.PersonID, LineItemT.PatientID "
			" "
			"/*End MePaymentsQ*/ "
			") MEPaymentsQ ON "
			" "
			"PatientsT.PersonID = MEPaymentsQ.PatientID "
			" "
			"/* Group by */ "
			"GROUP BY PatientsT.PersonID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle, "
			"PersonT.FirstContactDate,  "
			"MeChargesQ.Date,  "
			"MeChargesQ.Description,  "
			"MeChargesQ.ChargeAmt,  "
			"MePaymentsQ.SumOfAmount,  "
			"MeLevelsQ.F1, MeLevelsQ.F1ID,  "
			"MeLevelsQ.F2, MeLevelsQ.F2ID,  "
			"MeLevelsQ.F3, MeLevelsQ.F3ID, MELevelsQ.F4, MELevelsQ.F4ID, "
			"PersonT.ID "
			"/* having*/ "
			"HAVING ((PersonT.FirstContactDate Is Not Null) "
			"AND  "
			"(MeChargesQ.Date Is Not Null)); ");
			break;


			case 258:
				//New Patient Procedures
				return _T("SELECT Last + ', ' + First + ' ' + Middle AS Name, dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS Procedur, FirstContactDate AS Date, PersonID AS PatID "
			"FROM LaddersT  "
			"INNER JOIN PersonT ON LaddersT.PersonID = PersonT.ID "
			"WHERE LaddersT.Status = 1 ");
				break;	 

			//:  This is the same as Patients by Procedure, is it not?
/*			case 259:
				//Procedure Steps
				return _T("SELECT Last + ', ' + First + ' ' + Middle AS Name, ProcedureT.Name AS Procedur, StepTemplatesT.StepName, "
					"FirstContactDate AS Date, StepTemplatesT.Note, LaddersT.FirstInterestDate, LaddersT.PersonID AS PatID, LaddersT.Notes AS PatNote  "
			"FROM LaddersT  "
			"INNER JOIN PersonT ON LaddersT.PersonID = PersonT.ID "
			"INNER JOIN ProcedureT ON ProcedureT.ID = LaddersT.ProcedureID "
			"INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID  "
			"WHERE LaddersT.Status = 1 ");
				break;*/

			case 272:
				//Patients by Referring Patient
				/*	Version History
					DRT - 1/30/03 - Added Receipts column, which is the sum of amount in lineitemt where type = 1, 2, 3 (net payments) for Dr. Bolton.
							Also made a change so that the PatID is the patient DOING the referring, not the patient BEING referred.  This has the effect
							of:  1) the patient filter works on referring patient, and 2)  the create merge group makes the group out of the same folks.

					JMM - 4/10/2006 - PLID 20067 - fixed the report so it doesn't show inquires
				*/
				return _T("SELECT "
			"PersonT.ID AS PersonID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName,  "
			"PersonT1.ID AS PatID, PatientsT1.UserDefinedID AS RefUserID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS RefName,  "
			"PersonT.FirstContactDate AS Date, PersonT.HomePhone, PersonT.WorkPhone, PatientsT.MainPhysician AS ProvID,  "
			"PersonT2.Last + ', ' + PersonT2.First + ' ' + PersonT2.Middle AS ProvName, PersonT.Location AS LocID, "
			"CASE WHEN ReceiptSubQ.Amount IS NULL THEN 0 ELSE ReceiptSubQ.Amount END AS Amount "
			"FROM "
			"PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"INNER JOIN PersonT PersonT1 ON PatientsT.ReferringPatientID = PersonT1.ID "
			"INNER JOIN PatientsT PatientsT1 ON PersonT1.ID = PatientsT1.PersonID "
			"LEFT JOIN PersonT PersonT2 ON PatientsT.MainPhysician = PersonT2.ID "
			"LEFT JOIN "
			"( "
			"SELECT PersonT.ID AS PersonID, Sum(Amount) AS Amount "
			"FROM PersonT INNER JOIN LineItemT ON PersonT.ID = LineItemT.PatientID "
			"WHERE LineItemT.Type >= 1 AND LineItemT.Type <= 3 AND LineItemT.Deleted = 0 "
			"GROUP BY PersonT.ID "
			") ReceiptSubQ ON PersonT.ID = ReceiptSubQ.PersonID"
			" WHERE PatientsT.CurrentStatus <> 4 ");
				break;
	
			case 273:
				//Consults Without Surgeries
				//Note: When generating .ttx file, the Purpose field needs to be manually set to a memo field (length 500)
				/*	Version History
					1/14/03 - DRT - Consults that are marked no show were incorrectly showing up in the report.  The report should only show consults (meaning, they
							actually happened).  If it was no showed, then it shouldn't show.  Also did the same for surgeries, if they didn't show for the surgery
							(which I can't imagine happens too often) then it shouldn't count.
					DRT 6/19/03 - Removed references to AptPurposeID, they are obsolete
					// (j.jones 2008-06-25 09:17) - PLID 27246 - ensured that surgeries that had a procedure that
					// used the same master procedure as the consult's procedure, would be accounted for
				*/
				strSQL = _T("SELECT "
					"ConsultsSubQ.PatientID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
					"dbo.GetPurposeString(ConsultsSubQ.ID) AS PurposeName,  PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS DocName, "
					"PatientsT.MainPhysician AS ProvID, PersonT.Location AS LocID, AppointmentPurposeT.PurposeID AS PurposeID, ConsultsSubQ.ID AS AppointmentID "
					" "
					"FROM "
					"( "
					"	SELECT AppointmentsT.ID, AppointmentsT.PatientID "
					"	FROM AppointmentsT "
					"	WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentsT.StartTime >= @ReportDateFrom "
					"	AND AppointmentsT.StartTime < @ReportDateTo AND "
					"	AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND NOT EXISTS "
					"	 "
					"		/* SubQuery for Surgery appointments */ "
					"	(	SELECT PatientID FROM AppointmentsT AptsSurgeryQ LEFT JOIN AppointmentPurposeT ON AptsSurgeryQ.ID = AppointmentPurposeT.PurposeID "
					"		WHERE (AptsSurgeryQ.Status <> 4) AND (AptsSurgeryQ.ShowState <> 3) AND (AptsSurgeryQ.PatientID = AppointmentsT.PatientID) /*AND ((figure out purposes ><) OR (check for no purposes))*/ "
					"		AND (AptsSurgeryQ.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4)) AND (AptsSurgeryQ.StartTime >= AppointmentsT.StartTime) "
					"		/* This makes an attempt to see if any appointment in the consult is also in the surgery, so there won't be problems w/ 1 of many consults being the only 1 of many surgeries */ "
					"		AND ( "
					"		    (SELECT TOP 1 ID FROM ProcedureT ProcedureQ WHERE Coalesce(MasterProcedureID, ID) IN (SELECT Coalesce(MasterProcedureID, AptPurposeT.ID) FROM AppointmentPurposeT "
					"									INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
					"									LEFT JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID WHERE AppointmentID = AptsSurgeryQ.ID) "
					"									AND Coalesce(MasterProcedureID, ID) IN (SELECT Coalesce(MasterProcedureID, AptPurposeT.ID) FROM AppointmentPurposeT "
					"									INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
					"									LEFT JOIN ProcedureT ON AptPurposeT.ID = ProcedureT.ID WHERE AppointmentID = AppointmentsT.ID)) IS NOT NULL "
					"		    /* If both the consult and surgery have no purpose, they will not be in the above case, so we've got to handle that */ "
					"		    OR "
					"		    (AppointmentsT.ID NOT IN (SELECT AppointmentID FROM AppointmentPurposeT) AND AptsSurgeryQ.ID NOT IN (SELECT AppointmentID FROM AppointmentPurposeT)) "
					"		    ) "
					"		/* end SubQuery for surgery appts */ "
					"	) "
					") ConsultsSubQ "
					"LEFT JOIN PersonT ON ConsultsSubQ.PatientID = PersonT.ID "
					"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID "
					"LEFT JOIN AppointmentPurposeT ON ConsultsSubQ.ID = AppointmentPurposeT.AppointmentID "
					"WHERE PersonT.ID > 0");
				if(nDateRange > 0){	//date range chosen
					strSQL.Replace("@ReportDateFrom", DateFrom.Format("'%m/%d/%Y'"));
					COleDateTimeSpan dtSpan;
					COleDateTime dt;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dt = DateTo;
					dt += dtSpan;
					strSQL.Replace("@ReportDateTo", dt.Format("'%m/%d/%Y'"));
				}
				else{	//no date range chosen, so select from whatever the low date is, up until today ... there isn't any reason to show consults w/o surgeries when the consult hasn't even happened yet
					COleDateTime dtToday;
					dtToday = COleDateTime::GetCurrentTime();
					strSQL.Replace("@ReportDateFrom", dtMin.Format("'%m/%d/%Y'"));
					strSQL.Replace("@ReportDateTo", dtToday.Format("'%m/%d/%Y'"));
				}
				return strSQL;
				break;

			case 283: //Tracking Steps by Procedure
			case 291: //Patients by Tracking Step
			case 292: //Procedures by Tracking Step
				/*	Version History
					DRT 4/23/03 - (after the one below) Rewrote a bit of the query (with Tom's assistance), that was causing 'Done' steps to just never
							show up, even though there is a special case for filtering them.  I had to recreate the .ttx file for all 3 reports.
					DRT 4/23/03 - Fixed a bug with the extended filter of all 3 that was not correctly naming the 'Done' column (it didn't use QUOTENAME)
					TES 5/9/03 - Added fields for Master procedure.
					TES 10/27/2004 - Don't show unchosen details.
					TES 12/28/2004 - Forgot to alias the StepOrder field, which was messing up verifying the report.  Fixed.
					// (z.manning 2008-12-02 10:04) - PLID 32187 - Added InputUsername and address fields
				*/
				return _T("SELECT CASE WHEN StepTemplatesT.ID IS NULL THEN -1 ELSE StepTemplatesT.ID END AS StepTemplateID,  "
					"(SELECT Name FROM LadderTemplatesT WHERE ID IN (SELECT LadderTemplateID FROM StepTemplatesT WHERE ID IN (SELECT StepTemplateID FROM StepsT WHERE LadderID = LaddersT.ID))) AS LadderName, "
					"(SELECT ID FROM LadderTemplatesT WHERE ID IN (SELECT LadderTemplateID FROM StepTemplatesT WHERE ID IN (SELECT StepTemplateID FROM StepsT WHERE LadderID = LaddersT.ID))) AS LadderID, "
					"ProcInfoDetailsT.ProcedureID AS ProcID, ProcedureT.Name AS ProcName,  dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS ProcList, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.ID Is Null THEN 'Done' ELSE StepTemplatesT.StepName  "
					"END AS PtStepID, CASE WHEN StepTemplatesT.StepName Is Null THEN 'Done' ELSE StepTemplatesT.StepName END AS PtStepName, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID,  "
					"LaddersT.FirstInterestDate AS Date, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ReferralSourceT.Name As ReferralName, CASE WHEN StepsT.StepOrder Is Null THEN 999999 ELSE StepsT.StepOrder END AS StepOrder, LaddersT.ID,  "
					"MasterProcsT.ID AS MasterID, MasterProcsT.Name AS MasterName, InputUser.UserName AS InputUsername, "
					"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip "
					"FROM PersonT INNER JOIN ((PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID)  "
					"LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT INNER JOIN  "
					"(ProcInfoDetailsT INNER JOIN (ProcedureT LEFT JOIN ProcedureT MasterProcsT ON ProcedureT.MasterProcedureID = MasterProcsT.ID) ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) ON LaddersT.ProcInfoID = ProcInfoDetailsT.ProcInfoID) ON PersonT.ID = LaddersT.PersonID  "
					"LEFT JOIN UsersT InputUser ON LaddersT.InputUserID = InputUser.PersonID "
					"INNER JOIN  "
					"	(SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.ID Is Null THEN -1 ELSE StepsT.ID END AS NextStep  "
					"	FROM LaddersT LEFT JOIN  "
					"		(SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep  "
					"		FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID  "
					"		WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT)  "
					"		GROUP BY LaddersT.ID  "
					"		) NextStepSubQ  "
					"	ON LaddersT.ID = NextStepSubQ.LadderID LEFT JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID AND NextStepSubQ.NextStep = StepsT.StepOrder LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID  "
					"	) AS NextStepQ  "
					"ON LaddersT.ID = NextStepQ.LadderID  "
					"LEFT JOIN (StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID  "
					"INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON NextStepQ.NextStep = StepsT.ID  "
					"WHERE LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1 OR LadderStatusT.ID = 2) "
					"AND (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null)");
				break;

			case 293: //Patient Tracking by Coordinator
				return _T("SELECT LadderTemplatesT.Name AS LadderName, LadderTemplatesT.ID AS LadderTemplateID, dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS ProcName, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.ID Is Null THEN -1 ELSE StepTemplatesT.ID END AS PtStepID, CASE WHEN StepTemplatesT.StepName Is Null THEN 'Done' ELSE StepTemplatesT.StepName END AS PtStepName, PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ReferralSourceT.Name As ReferralName, StepsT.StepOrder, UsersT.PersonID AS CoordID, UsersT.UserName AS CoordName, "
					"ProcInfoDetailsT.ProcedureID AS ProcID, LaddersT.ID AS LadderID "
					"FROM PersonT INNER JOIN (((PatientsT LEFT JOIN UsersT ON PatientsT.EmployeeID = UsersT.PersonID) LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID) ON PersonT.ID = LaddersT.PersonID INNER JOIN (SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.ID Is Null THEN -1 ELSE StepsT.ID END AS NextStep "
					"FROM LaddersT INNER JOIN ( "
					"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
					"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
					"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
					"GROUP BY LaddersT.ID "
					") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
					"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) AS NextStepQ ON LaddersT.ID = NextStepQ.LadderID INNER JOIN StepsT ON NextStepQ.LadderID = StepsT.LadderID AND NextStepQ.NextStep = StepsT.ID INNER JOIN (StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON StepsT.StepTemplateID = StepTemplatesT.ID "
					"WHERE (StepTemplatesT.ID Is Null OR StepsT.ActiveDate <= getdate() OR StepsT.ActiveDate Is Null) AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1)");
				break;

			case 295: //Active Steps by Age
				switch(nSubLevel) {
				case 1:
					return _T("SELECT LadderTemplatesT.Name AS LadderName, LadderTemplatesT.ID AS LadderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
						"PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.ID Is Null THEN -1 ELSE StepTemplatesT.ID END AS PtStepID, "
						"CASE WHEN StepTemplatesT.StepName Is Null THEN 'Done' ELSE StepTemplatesT.StepName END AS PtStepName, "
						"PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, "
						"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ReferralSourceT.Name As ReferralName, "
						"StepsT.StepOrder, UsersT.PersonID AS CoordID, UsersT.UserName AS CoordName, StepsT.ActiveDate, "
						"ProcedureT.Name AS ProcName, ProcedureT.ID AS ProcID, "
						"CASE WHEN LEN(CONVERT(nvarchar, StepTemplatesT.StepOrder)) = 1 THEN '0' ELSE '' END + CONVERT(nvarchar, StepTemplatesT.StepOrder) + StepTemplatesT.StepName AS StepIdentifier "
						"FROM PersonT INNER JOIN (((PatientsT LEFT JOIN UsersT ON PatientsT.EmployeeID = UsersT.PersonID) LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) ON PersonT.ID = LaddersT.PersonID INNER JOIN (SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.StepTemplateID Is Null THEN -1 ELSE StepsT.StepTemplateID END AS NextStep "
						"FROM LaddersT INNER JOIN ( "
						"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
						"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
						"GROUP BY LaddersT.ID "
						") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) AS NextStepQ ON LaddersT.ID = NextStepQ.LadderID INNER JOIN StepsT ON NextStepQ.LadderID = StepsT.LadderID AND NextStepQ.NextStep = StepsT.StepTemplateID INNER JOIN (StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON NextStepQ.NextStep = StepTemplatesT.ID "
						"WHERE (StepTemplatesT.ID Is Null OR StepsT.ActiveDate <= getdate() OR StepsT.ActiveDate Is Null) AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1) AND MasterProcedureID Is Null");
					break;
				default: //main
					return _T("SELECT LadderTemplatesT.ID AS PhaseID, LadderTemplatesT.Name AS PhaseName, StepTemplatesT.ID AS PhaseDetailID, StepTemplatesT.StepName AS PhaseDetailName, StepTemplatesT.StepOrder, ProcedureT.Name AS ProcName, ProcedureT.ID AS ProcID, "
							"CASE WHEN LEN(CONVERT(nvarchar, StepTemplatesT.StepOrder)) = 1 THEN '0' ELSE '' END + CONVERT(nvarchar, StepTemplatesT.StepOrder) + StepTemplatesT.StepName AS StepIdentifier "
							"FROM (StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) INNER JOIN StepsT ON StepTemplatesT.ID = StepsT.StepTemplateID INNER JOIN (LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) ON StepsT.LadderID = LaddersT.ID "
							"WHERE ProcedureT.MasterProcedureID Is Null "
							"GROUP BY LadderTemplatesT.ID, LadderTemplatesT.Name, StepTemplatesT.ID, StepTemplatesT.StepName, StepTemplatesT.StepOrder, ProcedureT.Name, ProcedureT.ID");
					break;
				}
				break; 
				
			case 300://Procedures by Status
				return _T("SELECT dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS LadderName, LadderTemplatesT.ID AS LadderID, "
						"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, PersonT.ID AS PatID, PatientsT.UserDefinedID, "
						"CASE WHEN StepTemplatesT.ID Is Null THEN -1 ELSE StepTemplatesT.ID END AS PtStepID, CASE WHEN StepTemplatesT.StepName Is Null THEN '<All Steps Completed>' ELSE StepTemplatesT.StepName END AS PtStepName, "
						"PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, "
						"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ReferralSourceT.Name As ReferralName, "
						"StepsT.StepOrder, LaddersT.Status AS StatusID, LadderStatusT.Name AS StatusName, "
						"ProcedureT.ID AS ProcID, ProcedureT.Name AS ProcName, MasterProcsT.ID AS MasterID, MasterProcsT.Name AS MasterName, "
						"LaddersT.ID "
						"FROM PersonT INNER JOIN ((PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID) LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID) ON PersonT.ID = PatientsT.PersonID INNER JOIN (LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID INNER JOIN (ProcedureT LEFT JOIN ProcedureT MasterProcsT On ProcedureT.MasterProcedureID = MasterProcsT.ID) ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID) LEFT JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID ON PersonT.ID = LaddersT.PersonID LEFT JOIN ((SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.ID Is Null THEN -1 ELSE StepsT.ID END AS NextStep "
						"FROM LaddersT INNER JOIN ( "
						"SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep "
						"FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT) "
						"GROUP BY LaddersT.ID "
						") NextStepSubQ ON LaddersT.ID = NextStepSubQ.LadderID INNER JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
						"WHERE StepsT.StepOrder = NextStepSubQ.NextStep) AS NextStepQ INNER JOIN (StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID) ON NextStepQ.NextStep = StepsT.ID) ON LaddersT.ID = NextStepQ.LadderID");
				break;

			case 330:
				{
				//Surgery List w/ Referrals
				/*	Version History
					DRT 1/18/2005 - PLID 15310 - Added "Next Appointment Date" as an available field (not in default report).
				*/
				CString sql = "SELECT AppointmentsT.ID, "
					"AppointmentsT.Date AS Date, "
					"CASE WHEN PersonT.ID < 0 THEN 'No Patient' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS PatName, "
					"CASE WHEN AptTypeT.ID IS NULL THEN 'No Type' ELSE AptTypeT.Name END AS TypeName, "
					"CASE WHEN AptPurposeT.ID IS NULL THEN 'No Purpose' ELSE AptPurposeT.Name END AS PurposeName, "
					"ResourceT.Item AS ResourceName, "
					"LocationsT.Name AS LocationName, "
					"CASE WHEN PatientsT.CurrentStatus = 0 THEN 'Patient Prospect' ELSE CASE WHEN PatientsT.CurrentStatus = 1 THEN 'Patient' ELSE 'Prospect' END END AS Status, "
					"(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 1) AS PatientCustom1, "
					"(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonT.ID AND FieldID = 4) AS PatientCustom4, "
					"CASE WHEN ReferralSourceT.PersonID IS NULL THEN 'No Referral' ELSE ReferralSourceT.Name END AS ReferralName, "
					"PersonT.ID AS PatID, ReferralSourceT.PersonID AS ReferralID, LocationsT.ID AS LocID, ResourceT.ID AS ResourceID, AptTypeT.ID AS TypeID, AptPurposeT.ID AS PurposeID, "
					" PersonT.HomePhone, PersonT.WorkPhone, PersonT.CellPhone, "
					" PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, ";

					//there's an ASC-related note in the report, suppress it if this is 0
					sql += "1 AS Suppress, ";

					sql += "(SELECT MIN(StartTime) FROM AppointmentsT ApptsT WHERE ApptsT.StartTime > GetDate() AND ApptsT.PatientID = PersonT.ID AND ApptsT.Status <> 4) AS NextApptDate "
					"FROM "
					"AppointmentsT "
					"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
					"LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID "
					"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
					"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
					"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
					"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
					" "
					"WHERE "
					"AppointmentsT.Status <> 4 AND "
					"AppointmentsT.ShowState <> 3 AND "
					"AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4)";
					return _T(sql);
				break;
				}

			case 334:
				//Total Patient Revenue
				//Note:  This uses the location and provider from the Patient, not from the individual line items (payments in this case)
				/*	Version History
					DRT 5/29/03 - Added filters for 'Insurance Payments Only' and 'Patient Payments Only'
					(a.walling 2007-02-20 09:32) - PLID 24337 - Adding UserDefinedID in here; currently it uses our internal ID (and on 
						the default report!). Personally I would change the name it selects as, but I'll leave it as is to be consistent.
						Also increased version in Callback.
				*/
				return _T("SELECT "
					"PatientsT.UserDefinedID, PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, ReferralSourceT.Name AS ReferralName, ReferralSourceT.PersonID AS ReferralID, "
					"LineItemT.Amount, LineItemT.Description, LineItemT.Date AS Date, "
					"CASE WHEN PersonT1.ID IS NULL THEN 'No Provider' ELSE PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle END AS ProvName, PersonT1.ID AS ProvID, "
					"LocationsT.ID AS LocID, CASE WHEN LocationsT.ID IS NULL THEN 'No Location' ELSE LocationsT.Name END AS LocName, "
					"CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 1 ELSE 0 END AS IsPatPay "
					" "
					"FROM "
					"PersonT "
					"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN LineItemT ON PersonT.ID = LineItemT.PatientID "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
					"LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID "
					"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
					" "
					"WHERE (LineItemT.Type = 1 OR LineItemT.Type = 3) AND LineItemT.Deleted = 0");
				break;

			case 349:
				//Patients by Original Referral Source
				/*	Version History
					DRT 7/16/03 - Changed around the from clause slightly - There was an incredibly wierd error that arose when trying to create
							a merge group, because of the fact that the dbo. formula was used in a join (join ... on <formula> = something) and in
							a subquery.  I re-arranged it so that the formula was put into a subquery of it's own, then joined into it.
					TES 4/20/06 - PLID 20165 - Filtered out inquiries
				*/
				return _T("SELECT "
					"PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, "
					"ReferralSourceT.PersonID AS ReferralID, ReferralSourceT.Name, PersonT.FirstContactDate AS Date, dbo.GetReferringPatients(PersonT.ID, '') AS RefList "
					"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN (SELECT PersonT.ID, dbo.GetOriginalReferral(PersonT.ID, '', 1) OriginalID FROM PersonT) OriginalSubQ ON PersonT.ID = OriginalSubQ.ID "
					"LEFT JOIN ReferralSourceT ON OriginalSubQ.OriginalID = ReferralSourceT.PersonID "
					"WHERE PersonT.ID <> -25 AND PatientsT.CurrentStatus <> 4");
				break;

			case 350:
				//Patients by Original Referring Physician
				/*	Version history
					DRT 7/16/03 - Changed around the from clause slightly - There was an incredibly wierd error that arose when trying to create
							a merge group, because of the fact that the dbo. formula was used in a join (join ... on <formula> = something) and in
							a subquery.  I re-arranged it so that the formula was put into a subquery of it's own, then joined into it.
					TES 4/20/06 - PLID 20165 - Filtered out inquiries
				*/
				return _T("SELECT "
					"PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, "
					"ReferringPhysT.PersonID AS RefPhysID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS RefPhysName, PersonT.FirstContactDate AS Date, dbo.GetReferringPatients(PersonT.ID, '') AS RefList "
					"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN (SELECT PersonT.ID, dbo.GetOriginalReferral(PersonT.ID, '', 2) AS OriginalID FROM PersonT) OriginalSubQ ON PersonT.ID = OriginalSubQ.ID "
					"LEFT JOIN (PersonT PersonT1 INNER JOIN ReferringPhysT ON PersonT1.ID = ReferringPhysT.PersonID) ON OriginalSubQ.OriginalID = ReferringPhysT.PersonID "
					"WHERE PersonT.ID <> -25 AND PatientsT.CurrentStatus <> 4");
				break;

			case 351:
				//Referred Patients by Referral Source
				return _T("SELECT "
					"PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, "
					"ReferralSourceT.PersonID AS ReferralID, ReferralSourceT.Name, PersonT.FirstContactDate AS Date, dbo.GetNestedNames(PersonT.ID, '') AS RefList "
					"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
					"WHERE PersonT.ID <> -25");
				break;

			case 420:
				return _T("SELECT "
					"PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, "
					"ReferralSourceT.PersonID AS ReferralID, ReferralSourceT.Name, PersonT.FirstContactDate AS Date, dbo.GetNestedNames(PersonT.ID, '') AS RefList "
					"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
					"WHERE PersonT.ID <> -25");
				break;
			break;
			case 352:
				//Referred Patients by Referring Physician
				/*
					DRT 2/2/2004 - PLID 9922 - Made report editable.
				*/
				return _T("SELECT "
					"PersonT.ID AS PatID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, PatientsT.UserDefinedID, "
					"ReferringPhysT.PersonID AS RefPhysID, PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS RefPhysName, PersonT.FirstContactDate AS Date, dbo.GetNestedNames(PersonT.ID, '') AS RefList "
					"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"LEFT JOIN (PersonT PersonT1 INNER JOIN ReferringPhysT ON PersonT1.ID = ReferringPhysT.PersonID) ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
					"WHERE PersonT.ID <> -25");
				break;

			case 357:
				//Projected Surgery Income
				/*	Version History
					DRT 6/19/03 - Removed references to AptPurposeID, it's obsolete.
					TES 3/3/04 - Changed to group ladders together (not split out by procedure).
					TES 3/4/04 - Added Prepayments total
					TES 3/4/04 - Correctly implemented the resource filter.
					DRT 4/22/2004 - PLID 11816 - Fixed some bad select *'s
					TES 8/23/2005 - Changed the resource filtering.
					JMM 9/6/05 - Split out Anesthesia, Facility, and Inventory Fees
					TES 1/12/06 - PLID 18823 - Filtered out inactive ladders.
					// (j.gruber 2007-02-22 16:08) - PLID 24699 Added Scheduled Date as a filter
					// (a.walling 2007-08-16 09:44) - PLID 27088 - Fixed prepays to be consistent with what is shown in the PIC
					// (a.walling 2007-09-10 10:15) - PLID 27088 - Fixed to return $0.00 instead of NULL
					// (j.gruber 2009-03-25 13:52) - PLID 33696 - changed discount structure
					// (z.manning 2009-11-12 11:55) - PLID 36281 - Fixed the discount amount calculation
					// (j.jones 2013-03-12 09:28) - PLID 55156 - we now do not filter out "Done" ladders by default,
					// we now have a preference to control that filter
					// (z.manning 2015-11-12 12:58) - PLID 66431 - Fixed some issues related to discounts on practice
					// vs. outside charges. Also formatted the query using a T-SQL formatter so it was easier to work with.
				*/
				{
					CString strSql;
					strSql.Format(R"(
SELECT AppointmentsT.StartTime AS StartTime
	, AppointmentsT.EndTime AS EndTime
	, Round((Convert(FLOAT, (Convert(DATETIME, Convert(NVARCHAR, EndTime, 8)) - Convert(DATETIME, Convert(NVARCHAR, StartTime, 8)))) * 24), 2) AS Hours
	, AppointmentsT.Date AS Date
	, AppointmentsT.LocationID AS LocID
	, AptTypeT.NAME
	, dbo.GetResourceString(AppointmentsT.ID) AS Item
	, AppointmentsT.AptTypeID AS SetID
	, PersonT.HomePhone
	, CASE 
		WHEN dbo.GetPurposeString(AppointmentsT.ID) IS NULL
			THEN 'No Purpose'
		ELSE dbo.GetPurposeString(AppointmentsT.ID)
		END AS Purpose
	, AptTypeT.ID AS AptTypeID
	, - 1 AS ResourceID
	, PersonT.ID AS PatID
	, ProcInfoT.SurgeonID AS ProvID /*, ProcInfoDetailsT.ProcedureID AS ProcedureID*/
	, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName
	, DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle AS DocName
	, dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS ProcedureName
	, QuotesQ.TotalLessDiscounts
	, QuotesQ.Discounts
	, QuotesQ.Total
	, QuotesQ.PracTotalLessDiscounts
	, QuotesQ.PracDiscounts
	, QuotesQ.PracTotal
	, QuotesQ.TotalOutsideAmount
	, (QuotesQ.Discounts - QuotesQ.PracDiscounts) AS OutsideDiscounts
	, (QuotesQ.TotalOutsideAmount - (QuotesQ.Discounts - QuotesQ.PracDiscounts)) OutsideNetTotal
	, COALESCE((
			SELECT SUM(Amount)
			FROM (
				SELECT PaymentsT.ID AS PayID
					, LineItemT.Amount AS Amount
				FROM ProcInfoT ProcInfoPrepayT
				INNER JOIN BillsT ON ProcInfoPrepayT.BillID = BillsT.ID
				LEFT JOIN ChargesT ON ChargesT.BillID = BillsT.ID
				INNER JOIN AppliesT ON AppliesT.DestID = ChargesT.ID
				INNER JOIN PaymentsT ON PaymentsT.ID = AppliesT.SourceID
				INNER JOIN LineItemT ON LineItemT.ID = PaymentsT.ID
				WHERE ProcInfoPrepayT.ID = ProcInfoT.ID
					AND PaymentsT.Prepayment = 1
					AND LineItemT.Deleted = 0
				GROUP BY PaymentsT.ID
					, LineItemT.Amount
				
				UNION
				
				SELECT ProcInfoPaymentsT.PayID AS PayID
					, LineItemT.Amount AS Amount
				FROM ProcInfoPaymentsT
				INNER JOIN LineItemT ON ProcInfoPaymentsT.PayID = LineItemT.ID
				WHERE LineItemT.Deleted = 0
					AND ProcInfoPaymentsT.ProcInfoID = ProcInfoT.ID
				GROUP BY ProcInfoPaymentsT.PayID
					, LineItemT.Amount
				) TotalPrepayQ
			), $0.00) AS Prepays
	, QuotesQ.TotalAnesFee
	, QuotesQ.TotalFacFee
	, QuotesQ.InventoryFee
	, AppointmentsT.CreatedDate AS ScheduledDate
FROM AptTypeT
INNER JOIN AppointmentsT ON AppointmentsT.AptTypeID = AptTypeT.ID
INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID
INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID
INNER JOIN ProcInfoT ON AppointmentsT.ID = ProcInfoT.SurgeryApptID
INNER JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID
LEFT JOIN PersonT AS DoctorsT ON ProcInfoT.SurgeonID = DoctorsT.ID
INNER JOIN (
	SELECT PatientBillsQ.ID AS QuoteID
		, PatientBillsQ.Date AS QuoteDate
		, PatientBillsQ.Description AS FirstOfNotes
		, Sum(Round(Convert(MONEY, (([Amount] + [OthrBillFee]) * [Quantity]) + ([Amount] * [Quantity] * (TaxRate - 1)) + ([Amount] * [Quantity] * (TaxRate2 - 1))), 2)) AS TotalLessDiscounts
		, Sum(Round(Convert(MONEY, (
						(
							(([Amount] + [OthrBillFee]) * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							)
						) + (
						(
							(([Amount] + [OthrBillFee]) * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							(([Amount] + [OthrBillFee]) * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS Discounts
		, Sum(Round(Convert(MONEY, (
						(
							([Amount] + [OthrBillFee]) * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							)
						) + (
						(
							([Amount] + [OthrBillFee]) * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							([Amount] + [OthrBillFee]) * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS Total
		, Sum(Round(Convert(MONEY, ([Amount] * [Quantity]) + ([Amount] * [Quantity] * (TaxRate - 1)) + ([Amount] * [Quantity] * (TaxRate2 - 1))), 2)) AS PracTotalLessDiscounts
		, Sum(Round(Convert(MONEY, (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPracPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							)
						) + (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPracPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPracPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS PracDiscounts
		, Sum(Round(Convert(MONEY, (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPracPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPracPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							)
						) + (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPracPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPracPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPracPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPracPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS PracTotal
		, Sum(Round(Convert(MONEY, ([OthrBillFee] * [Quantity] + ([OthrBillFee] * [Quantity] * (TaxRate - 1)) + ([OthrBillFee] * [Quantity] * (TaxRate2 - 1)))), 2)) AS TotalOutsideAmount
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalAnesFee
		, Sum(CASE 
				WHEN CPTCodesT.FacilityFee <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalFacFee
		, Sum(CASE 
				WHEN ProductT.ID IS NOT NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS InventoryFee
	FROM (
		SELECT BillsT.*
		FROM BillsT
		WHERE BillsT.Deleted = 0
		) AS PatientBillsQ
	LEFT JOIN (
		SELECT LineItemT.ID
			, LineItemT.PatientID
			, LineItemT.Type
			, LineItemT.Amount
			, LineItemT.Description
			, LineItemT.Date
			, LineItemT.InputDate
			, LineItemT.InputName
			, LineItemT.Deleted
			, LineItemT.DeleteDate
			, LineItemT.DeletedBy
			, LineItemT.LocationID
			, LineItemT.GiftID
			, LineItemT.DrawerID
			, ChargesT.BillID
			, ChargesT.DoctorsProviders
		FROM LineItemT
		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID
		WHERE LineItemT.Deleted = 0
			AND LineItemT.Type >= 10
		) AS PatientChargesQ ON PatientBillsQ.ID = PatientChargesQ.BillID
	LEFT JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID
	LEFT JOIN (
		SELECT ChargeID
			, CONVERT(FLOAT, SUM(Percentoff)) AS TotalPercentOff
			, CONVERT(MONEY, Sum(Discount)) AS TotalDiscount
		FROM ChargeDiscountsT
		WHERE ChargeDiscountsT.Deleted = 0
		GROUP BY ChargeID
		) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID
	LEFT JOIN (
		SELECT ChargeID
			, CONVERT(FLOAT, SUM(Percentoff)) AS TotalPracPercentOff
			, CONVERT(MONEY, Sum(Discount)) AS TotalPracDiscount
		FROM ChargeDiscountsT
		INNER JOIN LineItemT ON ChargeDiscountsT.ChargeID = LineItemT.ID
		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID
		WHERE ChargeDiscountsT.Deleted = 0 AND NOT (LineItemT.Amount = 0 AND ChargesT.OthrBillFee > 0)
		GROUP BY ChargeID
		) TotalPracDiscountsQ ON ChargesT.ID = TotalPracDiscountsQ.ChargeID
	LEFT JOIN PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID
	LEFT JOIN (
		SELECT CPTCodeT.ID
			, ServiceT.Anesthesia
			, ServiceT.FacilityFee
		FROM CPTCodeT
		INNER JOIN ServiceT ON CPtCodeT.ID = ServiceT.ID
		) AS CPTCodesT ON ChargesT.ServiceID = CPTCodesT.ID
	LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID
	GROUP BY PatientBillsQ.ID
		, PatientBillsQ.Date
		, PatientBillsQ.EntryType
		, PatientBillsQ.Description
		, PackagesT.QuoteID
		, TotalAmount
		, CurrentAmount
		, TotalCount
		, CurrentCount
	HAVING (((PatientBillsQ.EntryType) = 2))
	) AS QuotesQ ON ProcInfoT.ActiveQuoteID = QuotesQ.QuoteID
WHERE (
		AppointmentsT.Status <> 4
		AND PersonT.ID > 0
		AND AppointmentsT.ShowState <> 3
		AND AptTypeT.Category = 4
		AND LaddersT.Status IN (
			-- (j.jones 2013-03-12 09:28) - PLID 55156 - The status of 'Done' is hard-coded to 2, and its IsActive status is 0.
			-- A preference controls whether 'Done' ladders are included in this report.
			SELECT ID
			FROM LadderStatusT
			WHERE IsActive = 1
				OR (
					%li = 0
					AND ID = 2
					)
			)
		)
)"
, GetRemotePropertyInt("Reports_ProjectedSurgeryIncome_ExcludeFinishedLadders", 0, 0, "<None>", true));
					return _T(strSql);
				}
				break;

			case 358:
				//Actual Surgery Income
				/*	Version History
					DRT 6/19/03 - Removed references to AptPurposeID, it's obsolete.
					TES 3/4/04 - Correctly implemented the resource filter.
					DRT 4/22/2004 - PLID 11816 - Fixed some issues with select *'s
					TES 12/21/2004 - PLID 15030 - Changed to group ladders together (not split out by procedure).
					TES 8/23/2005 - Changed some resource filtering.
					JMM 9/6/2005 - Split out columns for Facilty Fees, Anesthesia, and Inventory
					JMM 2/6/2006 - Took out inactive quotes
					// (j.gruber 2007-01-15 10:04) - PLID 12600 - added field for removing tax
					// (j.gruber 2009-03-25 13:59) - PLID 33696 - change discount structure
					// (z.manning 2009-11-12 11:55) - PLID 36281 - Fixed the discount amount calculation
					// (z.manning 2015-11-12 12:58) - PLID 66431 - Fixed some issues related to discounts on practice
					// vs. outside charges. Also formatted the query using a T-SQL formatter so it was easier to work with.
				*/
				{
					CString strSql = R"(
SELECT AppointmentsT.StartTime AS StartTime
	, AppointmentsT.EndTime AS EndTime
	, Round((Convert(FLOAT, (Convert(DATETIME, Convert(NVARCHAR, EndTime, 8)) - Convert(DATETIME, Convert(NVARCHAR, StartTime, 8)))) * 24), 2) AS Hours
	, AppointmentsT.DATE AS DATE
	, AppointmentsT.LocationID AS LocID
	, AptTypeT.NAME
	, dbo.GetResourceString(AppointmentsT.ID) AS Item
	, AppointmentsT.AptTypeID AS SetID
	, PersonT.HomePhone
	, CASE 
		WHEN dbo.GetPurposeString(AppointmentsT.ID) IS NULL
			THEN 'No Purpose'
		ELSE dbo.GetPurposeString(AppointmentsT.ID)
		END AS Purpose
	, AptTypeT.ID AS AptTypeID
	, - 1 AS ResourceID
	, PersonT.ID AS PatID
	, ProcInfoT.SurgeonID AS ProvID
	, /*ProcInfoDetailsT.ProcedureID AS ProcedureID,*/ PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName
	, DoctorsT.Last + ', ' + DoctorsT.First + ' ' + DoctorsT.Middle AS DocName
	, dbo.CalcProcInfoName(ProcInfoT.ID) AS ProcedureName
	, QuotesQ.TotalLessDiscounts
	, QuotesQ.Discounts
	, QuotesQ.Total
	, QuotesQ.TotalPracLessDiscounts
	, QuotesQ.PracDiscounts
	, QuotesQ.TotalPrac
	, QuotesQ.TotalOutsideAmount
	, (QuotesQ.Discounts - QuotesQ.PracDiscounts) AS OutsideDiscounts
	, (QuotesQ.TotalOutsideAmount - (QuotesQ.Discounts - QuotesQ.PracDiscounts)) AS OutsideNetTotal
	, dbo.GetBillTotal(ProcInfoT.BillID) AS BillAmount
	, QuotesQ.TotalAnesFee
	, QuotesQ.TotalFacFee
	, QuotesQ.InventoryFee
	, QuotesQ.TotalLessDiscountsNoTax
	, QuotesQ.DiscountsNoTax
	, QuotesQ.TotalNoTax
	, QuotesQ.TotalOutsideAmountNoTax
	, QuotesQ.TotalAnesFeeNoTax
	, QuotesQ.TotalFacFeeNoTax
	, QuotesQ.InventoryFeeNoTax
	, QuotesQ.TotalLessDiscountsTaxOnly
	, QuotesQ.DiscountsTaxOnly
	, QuotesQ.TotalTaxOnly
	, QuotesQ.TotalOutsideAmountTaxOnly
	, QuotesQ.TotalAnesFeeTaxOnly
	, QuotesQ.TotalFacFeeTaxOnly
	, QuotesQ.InventoryFeeTaxOnly
	, QuotesQ.TotalProcFees
	, QuotesQ.TotalProcFeesNoTax
	, QuotesQ.TotalProcFeesTaxOnly
FROM AptTypeT
INNER JOIN AppointmentsT ON AppointmentsT.AptTypeID = AptTypeT.ID
INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID
INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID
INNER JOIN ProcInfoT ON AppointmentsT.ID = ProcInfoT.SurgeryApptID
LEFT JOIN PersonT AS DoctorsT ON ProcInfoT.SurgeonID = DoctorsT.ID
INNER JOIN (
	SELECT PatientBillsQ.ID AS QuoteID
		, PatientBillsQ.DATE AS QuoteDate
		, PatientBillsQ.Description AS FirstOfNotes
		, Sum(Round(Convert(MONEY, (([Amount] + [OthrBillFee]) * [Quantity]) + ([Amount] * [Quantity] * (TaxRate - 1)) + ([Amount] * [Quantity] * (TaxRate2 - 1))), 2)) AS TotalLessDiscounts
		, Sum(Round(Convert(MONEY, (
						(
							(([Amount] + [OthrBillFee]) * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							)
						) + (
						(
							(([Amount] + [OthrBillFee]) * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							(([Amount] + [OthrBillFee]) * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS Discounts
		, Sum(Round(Convert(MONEY, (
						(
							([Amount] + [OthrBillFee]) * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							)
						) + (
						(
							([Amount] + [OthrBillFee]) * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							([Amount] + [OthrBillFee]) * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS Total
		, Sum(Round(Convert(MONEY, ([Amount] * [Quantity]) + ([Amount] * [Quantity] * (TaxRate - 1)) + ([Amount] * [Quantity] * (TaxRate2 - 1))), 2)) AS TotalPracLessDiscounts
		, Sum(Round(Convert(MONEY, (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPracPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							)
						) + (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPracPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPracPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS PracDiscounts
		, Sum(Round(Convert(MONEY, (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPracPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPracPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							)
						) + (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPracPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPracPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPracPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPracPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalPracDiscount] IS NULL)
										THEN 0
									ELSE [TotalPracDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS TotalPrac
		, Sum(Round(Convert(MONEY, ([OthrBillFee] * [Quantity] + ([OthrBillFee] * [Quantity] * (TaxRate - 1)) + ([OthrBillFee] * [Quantity] * (TaxRate2 - 1)))), 2)) AS TotalOutsideAmount
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalAnesFee
		, Sum(CASE 
				WHEN CPTCodesT.FacilityFee <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalFacFee
		, Sum(CASE 
				WHEN ProductT.ID IS NOT NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS InventoryFee
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia = 0
					AND CPTCodesT.FacilityFee = 0
					AND ProductT.ID IS NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalProcFees
		, Sum(Round(Convert(MONEY, ([Amount] * [Quantity])), 2)) AS TotalLessDiscountsNoTax
		, Sum(Round(Convert(MONEY, (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							)
						)), 2)) AS DiscountsNoTax
		, Sum(Round(Convert(MONEY, (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							)
						)), 2)) AS TotalNoTax
		, Sum(Round(Convert(MONEY, ([OthrBillFee] * [Quantity])), 2)) AS TotalOutsideAmountNoTax
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									)), 2)
				ELSE 0
				END) AS TotalAnesFeeNoTax
		, Sum(CASE 
				WHEN CPTCodesT.FacilityFee <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									)), 2)
				ELSE 0
				END) AS TotalFacFeeNoTax
		, Sum(CASE 
				WHEN ProductT.ID IS NOT NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									)), 2)
				ELSE 0
				END) AS InventoryFeeNoTax
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia = 0
					AND CPTCodesT.FacilityFee = 0
					AND ProductT.ID IS NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										)
									)), 2)
				ELSE 0
				END) AS TotalProcFeesNoTax
		, Sum(Round(Convert(MONEY, ([Amount] * [Quantity] * (TaxRate - 1)) + ([Amount] * [Quantity] * (TaxRate2 - 1))), 2)) AS TotalLessDiscountsTaxOnly
		, Sum(Round(Convert(MONEY, (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							([Amount] * [Quantity] * (COALESCE([TotalPercentOff], 0) / 100)) + (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS DiscountsTaxOnly
)"
// (z.manning 2015-11-12 14:06) - PLID 66431 - Had to split this up to prevent compile error in VC++
+ CString(R"(
		, Sum(Round(Convert(MONEY, (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate - 1)
						) + (
						(
							[Amount] * [Quantity] * (
								CASE 
									WHEN ([TotalPercentOff] IS NULL)
										THEN 1
									ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
									END
								) - (
								CASE 
									WHEN ([TotalDiscount] IS NULL)
										THEN 0
									ELSE [TotalDiscount]
									END
								)
							) * (TaxRate2 - 1)
						)), 2)) AS TotalTaxOnly
		, Sum(Round(Convert(MONEY, (([OthrBillFee] * [Quantity] * (TaxRate - 1)) + ([OthrBillFee] * [Quantity] * (TaxRate2 - 1)))), 2)) AS TotalOutsideAmountTaxOnly
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalAnesFeeTaxOnly
		, Sum(CASE 
				WHEN CPTCodesT.FacilityFee <> 0
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalFacFeeTaxOnly
		, Sum(CASE 
				WHEN ProductT.ID IS NOT NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS InventoryFeeTaxOnly
		, Sum(CASE 
				WHEN CPTCodesT.Anesthesia = 0
					AND CPTCodesT.FacilityFee = 0
					AND ProductT.ID IS NULL
					THEN Round(Convert(MONEY, (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate - 1)
									) + (
									(
										([Amount] + [OthrBillFee]) * [Quantity] * (
											CASE 
												WHEN ([TotalPercentOff] IS NULL)
													THEN 1
												ELSE ((100 - Convert(FLOAT, [TotalPercentOff])) / 100)
												END
											) - (
											CASE 
												WHEN ([TotalDiscount] IS NULL)
													THEN 0
												ELSE [TotalDiscount]
												END
											)
										) * (TaxRate2 - 1)
									)), 2)
				ELSE 0
				END) AS TotalProcFeesTaxOnly
	FROM (
		SELECT BillsT.*
		FROM BillsT
		WHERE BillsT.Deleted = 0
			AND Active <> 0
		) AS PatientBillsQ
	LEFT JOIN (
		SELECT LineItemT.ID
			, LineItemT.PatientID
			, LineItemT.Type
			, LineItemT.Amount
			, LineItemT.Description
			, LineItemT.DATE
			, LineItemT.InputDate
			, LineItemT.InputName
			, LineItemT.Deleted
			, LineItemT.DeleteDate
			, LineItemT.DeletedBy
			, LineItemT.LocationID
			, LineItemT.GiftID
			, LineItemT.DrawerID
			, ChargesT.BillID
			, ChargesT.DoctorsProviders
		FROM LineItemT
		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID
		WHERE LineItemT.Deleted = 0
			AND LineItemT.Type >= 10
		) AS PatientChargesQ ON PatientBillsQ.ID = PatientChargesQ.BillID
	LEFT JOIN ChargesT ON PatientChargesQ.ID = ChargesT.ID
	LEFT JOIN PackagesT ON PatientBillsQ.ID = PackagesT.QuoteID
	LEFT JOIN (
		SELECT CPTCodeT.ID
			, ServiceT.Anesthesia
			, ServiceT.FacilityFee
		FROM CPTCodeT
		INNER JOIN ServiceT ON CPtCodeT.ID = ServiceT.ID
		) AS CPTCodesT ON ChargesT.ServiceID = CPTCodesT.ID
	LEFT JOIN (
		SELECT ChargeID
			, CONVERT(FLOAT, SUM(Percentoff)) AS TotalPercentOff
			, CONVERT(MONEY, Sum(Discount)) AS TotalDiscount
		FROM ChargeDiscountsT
		WHERE DELETED = 0
		GROUP BY ChargeID
		) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID
	LEFT JOIN (
		SELECT ChargeID
			, CONVERT(FLOAT, SUM(Percentoff)) AS TotalPracPercentOff
			, CONVERT(MONEY, Sum(Discount)) AS TotalPracDiscount
		FROM ChargeDiscountsT
		INNER JOIN LineItemT ON ChargeDiscountsT.ChargeID = LineItemT.ID
		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID
		WHERE ChargeDiscountsT.Deleted = 0 AND NOT (LineItemT.Amount = 0 AND ChargesT.OthrBillFee > 0)
		GROUP BY ChargeID
		) TotalPracDiscountsQ ON ChargesT.ID = TotalPracDiscountsQ.ChargeID
	LEFT JOIN ProductT ON ChargesT.ServiceID = ProductT.ID
	GROUP BY PatientBillsQ.ID
		, PatientBillsQ.DATE
		, PatientBillsQ.EntryType
		, PatientBillsQ.Description
		, PackagesT.QuoteID
		, TotalAmount
		, CurrentAmount
		, TotalCount
		, CurrentCount
	HAVING (((PatientBillsQ.EntryType) = 2))
	) AS QuotesQ ON ProcInfoT.ActiveQuoteID = QuotesQ.QuoteID
WHERE (
		AppointmentsT.STATUS <> 4
		AND PersonT.ID > 0
		AND AppointmentsT.ShowState <> 3
		AND AptTypeT.Category = 4
		)
)");
					return _T(strSql);
				}
				break;
			
			case 359: //Referring Patients

				return _T("SELECT PersonT.ID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
					"CASE WHEN ReferredPatientsT.CurrentStatus = 1 OR ReferredPatientsT.CurrentStatus = 3 THEN 1 ELSE 0 END AS ReferredPatient, "
					"CASE WHEN ReferredPatientsT.CurrentStatus = 2 OR ReferredPatientsT.CurrentStatus = 3 THEN 1 ELSE 0 END AS ReferredProspect, "
					"ReferredPersonT.FirstContactDate AS Date "
					"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"INNER JOIN PatientsT ReferredPatientsT ON PatientsT.PersonID = ReferredPatientsT.ReferringPatientID "
					"INNER JOIN PersonT ReferredPersonT ON ReferredPatientsT.PersonID = ReferredPersonT.ID ");
				break;

			case 396: {
				//PIC Report
				/*	Version History
					DRT 6/5/03 - Created.  I win the award for most date filters, with 5.
					DRT 6/17/03 - Fixed location filter.
					DRT 6/17/03 - Fixed an issue with the FirstConsultQ subquery, it was returning the first consult
							which was tracked for any ladder, not necessarily this ladder.  Now it works better.
					DRT 7/2/03 - Made all values show as $0 even if they are null.  That also fixed a problem where the balance
							would not calculate if the payments were null.
					DRT 7/7/03 - Added Gen1Note field, fixed a formatting issue with coords.  Made a dtld/smry option, dtld is the regular
							report, smry is a version setup to easily export to excel, with all data in a row - landscape style.
					TES 8/4/03 - Included EndTime
					// (j.gruber 2009-03-25 14:10) - PLID 33696 - updated discount structure
					// (j.jones 2009-08-18 12:35) - PLID 35139 - added case history information, costs are hidden if
					// the user cannot see person costs, and a personnel item exists
					// (j.jones 2009-08-19 15:30) - PLID 35124 - removed PayToPractice
					// (j.jones 2009-10-22 10:24) - PLID 36022 - fixed an error that would occur in the rare case that
					// a consult was linked to multiple ladders, we now join both ladders onto our main ladder query
					// (j.jones 2011-03-28 16:38) - PLID 42575 - ensured we referenced CaseHistoryDetailsT.Billable
					// (j.gruber 2011-09-22 11:50) - PLID 45350 - added some addition surgery and consult fields
				*/

				BOOL bCanViewPersonCosts = (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead);

				CString strSql;
				strSql.Format("SELECT LaddersT.ID AS LadderID, LaddersT.FirstInterestDate AS LadderDate, ProcInfoT.ID AS ProcInfoID, "
					"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = ProcInfoT.SurgeonID) AS SurgeonName, "
					"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = ProcInfoT.NurseID) AS NurseName, "
					"(SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = ProcInfoT.AnesthesiologistID) AS AnesthName, "
					"CASE WHEN PatientsT.EmployeeID IS NULL THEN '<No Coordinator>' ELSE (SELECT Last + ', ' + First + ' ' + Middle FROM PersonT WHERE ID = PatientsT.EmployeeID) END AS PatCoordName, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
					"ProcInfoT.Anesthesia, SurgApptsQ.ArrivalTime, dbo.CalcProcInfoName(LaddersT.ProcInfoID) AS LadderName, ProcInfoT.PatientID AS PatID, "
					"PatientsT.UserDefinedID, CnsltApptsQ.Date AS CnsltDate, CnsltApptsQ.StartTime AS CnsltStart, CnsltApptsQ.EndTime AS CnsltEnd, CnsltTypeT.Name AS CnsltType, "
					"CnsltApptsQ.Purpose as CnsltPurpShort, CnsltApptsQ.Purpose as CnsltPurpMemo, CnsltApptsQ.ArrivalTime as CnsltArrivalTime, CnsltApptsQ.Resource as CnsltResourceShort, CnsltApptsQ.Resource as CnlstResourceMemo, "
					"SurgApptsQ.Date AS SurgDate, SurgApptsQ.StartTime AS SurgStart, SurgApptsQ.EndTime AS SurgEnd, SurgTypeT.Name AS SurgType, "
					"SurgApptsQ.Purpose as SurgPurpShort, SurgApptsQ.Purpose as SurgPurpMemo, SurgApptsQ.Resource as SurgResourceShort, SurgApptsQ.Resource as SurgResourceMemo, "
					"ReferralSourceT.Name AS ReferralName, ReferralSourceT.PersonID AS ReferralID, CASE WHEN ActiveQuoteQ.QuoteTotal IS NULL THEN 0 ELSE ActiveQuoteQ.QuoteTotal END AS QuoteTotal, "
					"CASE WHEN ActiveBillQ.BillTotal IS NULL THEN 0 ELSE ActiveBillQ.BillTotal END AS BillTotal, ActiveBillQ.BillDate AS BillDate, "
					"CASE WHEN ProcAppliedPaysToBillQ.TotalApplied IS NULL THEN 0 ELSE ProcAppliedPaysToBillQ.TotalApplied END AS TotalApplied, "
					"CASE WHEN ActiveBillQ.BillTotal IS NULL THEN 0 ELSE ActiveBillQ.BillTotal END - CASE WHEN ProcAppliedPaysToBillQ.TotalApplied IS NULL THEN 0 ELSE ProcAppliedPaysToBillQ.TotalApplied END AS BillBalance, "
					"CASE WHEN ProcPrePaymentsQ.TotalPays IS NULL THEN 0 ELSE ProcPrePaymentsQ.TotalPays END AS TotalPays, "
					"  "
					"/* Various patient info for custom reporting */  "
					"PersonT.HomePhone, PersonT.WorkPhone, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Note,  "
					"PersonT.FirstContactDate AS FirstContactDate, ProcInfoT.SurgeonID AS ProvID, LaddersT.Status AS LadderStatus, PersonT.Location AS LocID, "
					"PersonT.Note AS Gen1Note, "
					" "
					"/* Case History Information */ "
					"CaseHistoryQ.Name AS CaseHistoryName, CaseHistoryQ.SurgeryDate AS CaseHistorySurgeryDate, CaseHistoryQ.CompletedDate AS CaseHistoryCompletedDate, "
					"CaseHistoryQ.TotalPrice, CaseHistoryQ.TotalCost, Coalesce(CaseHistoryQ.HasHiddenPersonCosts, 0) AS HasHiddenPersonCosts "
					""
					"FROM LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID  "
					"LEFT JOIN   "
					"	/* Get:  ID, Min-StartTime for each Purpose, Patient combo (which are consults) */ "
					"	(SELECT ApptQ.ID AS ApptID, ApptQ.PatientID, MinQ.MinStart, MinQ.PurposeID, "
					"	CASE WHEN ProcProcedureQ.ProcInfoID IS NULL THEN ProcLinkedQ.ProcInfoID ELSE ProcProcedureQ.ProcInfoID END AS ProcInfoID "
					"	FROM "
					"		(SELECT Min(AppointmentsT.StartTime) AS MinStart, AppointmentsT.PatientID, AppointmentPurposeT.PurposeID "
					"		FROM AppointmentsT LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
					"		WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND Status <> 4  "
					"		GROUP BY AppointmentsT.PatientID, AppointmentPurposeT.PurposeID "
					"		) MinQ "
					"	INNER JOIN  "
					"		(SELECT *  "
					"		FROM AppointmentsT  "
					"		WHERE AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1) AND Status <> 4  "
					"		) ApptQ "
					"	ON MinQ.MinStart = ApptQ.StartTime AND MinQ.PatientID = ApptQ.PatientID "
					"	LEFT JOIN  "
					"		(SELECT ProcInfoT.PatientID, ProcInfoDetailsT.ProcedureID, ProcInfoID "
					"		FROM "
					"		ProcInfoT INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoID "
					"		) ProcProcedureQ "
					"	ON ApptQ.PatientID = ProcProcedureQ.PatientID AND MinQ.PurposeID = ProcProcedureQ.ProcedureID "
					"	LEFT JOIN  "
					"		(SELECT ProcInfoID, AppointmentID "
					"		FROM ProcInfoAppointmentsT "
					"		) ProcLinkedQ "
					"	ON ApptQ.ID = ProcLinkedQ.AppointmentID "
					"	) FirstConsultQ "
					"ON ProcInfoT.PatientID = FirstConsultQ.PatientID AND ProcInfoT.ID = FirstConsultQ.ProcInfoID "
					" "
					"LEFT JOIN PatientsT ON ProcInfoT.PatientID = PatientsT.PersonID  "
					"LEFT JOIN PersonT ON ProcInfoT.PatientID = PersonT.ID  "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID  "
					"LEFT JOIN (SELECT *, dbo.GetPurposeString(AppointmentsT.ID) as Purpose, dbo.GetResourceString(AppointmentsT.ID) as Resource FROM AppointmentsT) CnsltApptsQ ON FirstConsultQ.ApptID = CnsltApptsQ.ID  "
					"LEFT JOIN AptTypeT CnsltTypeT ON CnsltApptsQ.AptTypeID = CnsltTypeT.ID  "
					"LEFT JOIN (SELECT *, dbo.GetPurposeString(AppointmentsT.ID) as Purpose, dbo.GetResourceString(AppointmentsT.ID) as Resource FROM AppointmentsT) SurgApptsQ ON ProcInfoT.SurgeryApptID = SurgApptsQ.ID  "
					"LEFT JOIN AptTypeT SurgTypeT ON SurgApptsQ.AptTypeID = SurgTypeT.ID  "
					"LEFT JOIN   "
					"	(SELECT BillsT.ID AS BillID,   "
					"	Sum(Round(Convert(money,(([Amount]*[Quantity]*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100.0) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(ChargesT.[TaxRate]+ChargesT.[TaxRate2]-1))),2)) AS QuoteTotal  "
					"	FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "
					"   LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
					"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
					"	WHERE EntryType = 2 AND LineItemT.Type = 11 AND LineItemT.Deleted = 0  "
					"	GROUP BY BillsT.ID  "
					"	) ActiveQuoteQ  "
					"ON ActiveQuoteID = ActiveQuoteQ.BillID  "
					"LEFT JOIN   "
					"	( SELECT BillsT.ID AS BillID, BillsT.Date AS BillDate,  "
					"	dbo.GetBillTotal(BillsT.ID) AS BillTotal  "
					"	FROM BillsT "
					"	) ActiveBillQ  "
					"ON ProcInfoT.BillID = ActiveBillQ.BillID  "
					"LEFT JOIN   "
					"	(/*Total Prepayments for this procedure*/  "
					"	SELECT ProcInfoID, Sum(LineItemT.Amount) AS TotalPays  "
					"	FROM ProcInfoPaymentsT INNER JOIN PaymentsT ON ProcInfoPaymentsT.PayID = PaymentsT.ID  "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID  "
					"	WHERE LineItemT.Type = 1 AND LineItemT.Deleted = 0  "
					"	GROUP BY ProcInfoID  "
					"	) ProcPrePaymentsQ  "
					"ON ProcInfoT.ID = ProcPrePaymentsQ.ProcInfoID  "
					"LEFT JOIN   "
					"	(/*Total payments applied to the Active bill*/  "
					"	SELECT BillsT.ID AS BillID, Sum(AppliesT.Amount) AS TotalApplied  "
					"	FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID  "					
					"	INNER JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID  "
					"	LEFT JOIN AppliesT ON LineChargesT.ID = AppliesT.DestID  "
					"	LEFT JOIN LineItemT LinePaysT ON AppliesT.SourceID = LinePaysT.ID  "
					"	WHERE EntryType = 1 AND LineChargesT.Type = 10 AND LineChargesT.Deleted = 0  "
					"	GROUP BY BillsT.ID  "
					"	) ProcAppliedPaysToBillQ  "
					"ON ActiveBillQ.BillID = ProcAppliedPaysToBillQ.BillID "
					"LEFT JOIN "
					"	/*information on the linked case history*/ "
					"	(SELECT CaseHistoryT.ID, Name, SurgeryDate, CompletedDate, "
					"	Sum(Round(Convert(money,CASE WHEN CaseHistoryDetailsT.Billable = 1 THEN Amount*Quantity ELSE 0 END) ,2)) AS TotalPrice, "
					"	(CASE WHEN SUM(CASE WHEN ItemType = -3 THEN 1 ELSE 0 END) > 0 AND %li=0 THEN NULL "
					"	ELSE "
					"	Sum(Round(Convert(money,Cost*Quantity),2)) "
					"	END) AS TotalCost, "
					"	(CASE WHEN SUM(CASE WHEN ItemType = -3 THEN 1 ELSE 0 END) > 0 AND %li = 0 THEN 1 ELSE 0 END) AS HasHiddenPersonCosts "
					"	FROM CaseHistoryT "
					"	INNER JOIN CaseHistoryDetailsT ON CaseHistoryT.ID = CaseHistoryDetailsT.CaseHistoryID "
					"	GROUP BY CaseHistoryT.ID, Name, SurgeryDate, CompletedDate) AS CaseHistoryQ "
					"ON ProcInfoT.CaseHistoryID = CaseHistoryQ.ID", bCanViewPersonCosts ? 1 : 0, bCanViewPersonCosts ? 1 : 0);

				return _T(strSql);
			break;
		}

		case 402:
			{
				//Marketing Costs
				/*	Version History
					DRT 7/16/03 - Created.
					(a.walling 2006-08-08 13:43) - PLID 3897 - Allow filtering on location and dates from print preview.
				*/

				CString sql = "SELECT "
				"MarketingCostsT.DatePaid AS DatePaid, MarketingCostsT.PaidTo, MarketingCostsT.Description, MarketingCostsT.RefNumber,  "
				"MarketingCostsT.Amount, MarketingCostsT.EffectiveFrom AS EffFrom, MarketingCostsT.EffectiveTo AS EffTo,  "
				"ReferralSourceT.Name, ReferralSourceT.PersonID AS RefSrcID "
				"FROM "
				"MarketingCostsT "
				"LEFT JOIN ReferralSourceT ON MarketingCostsT.ReferralSource = ReferralSourceT.PersonID";

				if (saExtraValues.GetSize()) {
					sql += " WHERE " + GetExtraValue();
				}
				return _T(sql);
			}
			break;

		case 477: //Completed Steps by Age
			/* Version History
				TES 2/18/04 - Created, more or less plagiarized from Active Steps by Age
				DRT 9/28/2005 - PLID 17691 - Removed a bonus StepsT.ActiveDate field that had no use
			*/
			return _T("SELECT LadderTemplatesT.Name AS LadderName, LadderTemplatesT.ID AS LadderID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, "
				"PersonT.ID AS PatID, PatientsT.UserDefinedID, CASE WHEN StepTemplatesT.ID Is Null THEN -1 ELSE StepTemplatesT.ID END AS PtStepID, "
				"CASE WHEN StepTemplatesT.StepName Is Null THEN 'Done' ELSE StepTemplatesT.StepName END AS PtStepName, "
				"PersonT.Location AS LocID, PatientsT.MainPhysician AS ProvID, LaddersT.FirstInterestDate AS Date, "
				"PersonT1.Last + ', ' + PersonT1.First + ' ' + PersonT1.Middle AS ProvName, ReferralSourceT.Name As ReferralName, "
				"StepsT.StepOrder, UsersT.PersonID AS CoordID, UsersT.UserName AS CoordName, StepsT.ActiveDate, "
				"ProcedureT.Name AS ProcName, ProcedureT.ID AS ProcID, "
				"CASE WHEN LEN(CONVERT(nvarchar, StepTemplatesT.StepOrder)) = 1 THEN '0' ELSE '' END + CONVERT(nvarchar, StepTemplatesT.StepOrder) + StepTemplatesT.StepName AS StepIdentifier, CASE WHEN StepsT.CompletedDate < StepsT.ActiveDate THEN StepsT.ActiveDate ELSE StepsT.CompletedDate END AS CompletedDate, CASE WHEN DATEDIFF(dd,StepsT.ActiveDate,StepsT.CompletedDate) < 0 THEN 0 ELSE DATEDIFF(dd,StepsT.ActiveDate,StepsT.CompletedDate) END AS Age "
				" "
				"FROM PersonT INNER JOIN PatientsT LEFT JOIN UsersT ON PatientsT.EmployeeID = UsersT.PersonID LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
				"LEFT JOIN PersonT PersonT1 ON PatientsT.MainPhysician = PersonT1.ID ON PersonT.ID = PatientsT.PersonID  "
				"INNER JOIN LaddersT INNER JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
				"INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID ON PersonT.ID = LaddersT.PersonID INNER JOIN  "
				"StepsT INNER JOIN EventAppliesT ON StepsT.ID = EventAppliesT.StepID INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT INNER JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"WHERE (StepTemplatesT.ID Is Null OR StepsT.ActiveDate <= getdate() OR StepsT.ActiveDate Is Null)  "
				"AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1) AND MasterProcedureID Is Null "
				" "
				"AND StepsT.ActiveDate Is Not Null AND EventsT.Type <> 9");
			break;

		case 540: //Conversion Rate by Procedure
			/* Version History
				TES 2/19/04 - Created
				TES 5/26/04 - Changed the ID to 540, since Josh overwrote my original ADD_REPORT macro.
				JMM 4/4/2005 - Made the report filter by resource
				// (j.gruber 2006-11-01 10:57) - PLID 23250 - made detailed procedures show under masters
				TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
			*/
			{
				CString strSQL;
				strSQL = _T("SELECT SubQ.Name, SubQ.ProcID AS ProcID, SubQ.PatID AS PatID, SubQ.Date AS Date, SubQ.LocID AS LocID, "
					"SubQ.Converted, SubQ.ApptID, SubQ.ResourceString "
					"FROM (SELECT ProcedureT.Name, AppointmentPurposeT.PurposeID AS ProcID, AppointmentsT.PatientID AS PatID,  "
					"AppointmentsT.Date AS Date, AppointmentsT.LocationID AS LocID,  "
					"CASE WHEN EXISTS ( "
					"    SELECT AppointmentsT.ID FROM AppointmentsT AppointmentProcedureT INNER JOIN AppointmentPurposeT  "
					"    AppointmentPurposeProcedureT ON AppointmentProcedureT.ID = AppointmentPurposeProcedureT.AppointmentID  "
					"    WHERE AppointmentProcedureT.Status <> 4 AND AppointmentProcedureT.ShowState <> 3 "
					"    AND (AppointmentPurposeProcedureT.PurposeID = AppointmentPurposeT.PurposeID  "
					"    OR AppointmentPurposeProcedureT.PurposeID IN (SELECT ID FROM ProcedureT WHERE MasterProcedureID = AppointmentPurposeT.PurposeID)) "
					"    AND AppointmentsT.PatientID = AppointmentProcedureT.PatientID "
					"    AND AppointmentProcedureT.StartTime > AppointmentsT.StartTime AND AppointmentProcedureT.AptTypeID IN ( "
					"       SELECT ID FROM AptTypeT WHERE Category IN (3,4) "
					"    ) "
					") THEN 1 ELSE 0 END AS Converted,  "
					" AppointmentsT.ID As ApptID, dbo.GetResourceString(AppointmentsT.ID) AS ResourceString "
					"FROM ProcedureT INNER JOIN AppointmentPurposeT ON ProcedureT.ID = AppointmentPurposeT.PurposeID  "
					"INNER JOIN AppointmentsT ON AppointmentPurposeT.AppointmentID = AppointmentsT.ID "
					" "
					"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 1)  "
					") SubQ");
				CString strDateFilter;
				if(nDateRange > 0){	//date range chosen
					COleDateTimeSpan dtSpan;
					COleDateTime dt;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dt = DateTo;
					dt += dtSpan;
					strDateFilter.Format(" WHERE SubQ.Date >= '%s' AND SubQ.Date < '%s'", DateFrom.Format("%m/%d/%Y"), dt.Format("%m/%d/%Y"));
				}
				else{	//no date range chosen, 
					//If a consult is in the future, it shouldn't count as unconverted
					strDateFilter.Format(" WHERE (SubQ.Date <= getdate() OR SubQ.Converted = 1)");
				}

				//JMM - PLID 15178 - set the extended filter to be the resource
				CString strExtendedFilter;
				if (saExtraValues.GetSize()) {
					strExtendedFilter.Format(" AND (SubQ.ApptID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s)))", GetExtraValue());
				}

				return strSQL + strDateFilter + strExtendedFilter;
			}
			break;


			case 480 :
				/* Conversion Rate By Referral Source
				Version History
					JMM = 2/23/04 - Created
					TES - 3/1/04 - Modified to use my new recursive function.
					TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				*/

				{
					CString strDateFilter = "";
					if(nDateRange > 0) {
						strDateFilter.Format("WHERE PersonT.FirstContactDate >= '%s' AND PersonT.FirstContactDate < '%s'", FormatDateTimeForSql(DateFrom, dtoDate), FormatDateTimeForSql(DateTo + COleDateTimeSpan(1,0,0,0), dtoDate));
					}
					CString strSql = "SELECT PatientsT.ReferralID AS ReferralID, GRT.ReferralName, "
						"CAST (100.0 * SUM(CASE WHEN SurgeriesQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Surgeries, "
						"CAST (100.0 * SUM(CASE WHEN ConsultsQ.PatientID IS NULL THEN 0 ELSE 1 END) AS FLOAT) AS Consults, "
						"CAST (COUNT(DISTINCT ConsultsQ.PatientID)-1 AS FLOAT) AS ConsultCount, "
						"CAST (COUNT(DISTINCT PatientsT.PersonID) AS FLOAT) AS Referrals "
						"FROM dbo.GetReferralTree(-1) AS GRT LEFT JOIN (PatientsT "
						"INNER JOIN (SELECT * FROM PersonT " + strDateFilter + ") AS PersonT ON PatientsT.PersonID = PersonT.ID) "
						"ON GRT.ReferralID = PatientsT.ReferralID "
						"LEFT JOIN ( "
							"SELECT AppointmentsT.PatientID "
							"FROM AppointmentsT "
							"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
							"WHERE (AptTypeT.Category = 4 "	//surgery
							"OR AptTypeT.Category = 3) "		//Other procedure
							"AND AppointmentsT.ShowState <> 3 " // 3 = no show
							"AND AppointmentsT.Status <> 4 "
							"GROUP BY AppointmentsT.PatientID "
						") AS SurgeriesQ ON PatientsT.PersonID = SurgeriesQ.PatientID "
						"LEFT JOIN ( "
							"SELECT AppointmentsT.PatientID "
							"FROM AppointmentsT "
							"INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
							"WHERE (AptTypeT.Category = 1) " //consult
							"AND AppointmentsT.ShowState <> 3 " //3 = no show
							"AND AppointmentsT.STatus <> 4 " //4 = cancelled
							"GROUP BY AppointmentsT.PatientID "
						") AS ConsultsQ ON PatientsT.PersonID = ConsultsQ.PatientID "
						" GROUP BY PatientsT.ReferralID, GRT.ReferralName, GRT.Priority "
						"ORDER BY GRT.Priority";
					return _T(strSql);
				}
				break;

			case 538:

				/* Consult To Surgery Counts By Resource
					Version History 
					5-11-2004 JMM - Created
					TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
				*/

				return _T("SELECT ResourceT.Item, ResourceT.ID AS Resource, FirstContactDate As Date, PersonT.ID AS PatID, PatientsT.MainPhysician AS ProvID, "
					" ConsultsQ.Date AS ConsDate, ConsultsQ.ID As ConsID, ConsultsWithSurgQ.ID AS ConsSurgID, PersonT.Location As LocID FROM  "
					"  AppointmentsT LEFT JOIN AppointmentResourceT ON "
					" AppointmentsT.ID = AppointmentResourceT.AppointmentID "
					" LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
					" LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
					"  LEFT JOIN ( 	SELECT AppointmentsT.ID, AppointmentsT.Date  "
					"		FROM AppointmentsT  "
					"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"		WHERE (AptTypeT.Category = 1)   "
					"		AND AppointmentsT.ShowState <> 3   "
					"		AND AppointmentsT.Status <> 4   "
					"		GROUP BY AppointmentsT.ID, AppointmentsT.Date  "
					"	) AS ConsultsQ ON AppointmentsT.ID = ConsultsQ.ID  "
					"  LEFT JOIN ( 	SELECT AppointmentsT.ID  "
					"		FROM AppointmentsT  "
					"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"		WHERE (AptTypeT.Category = 1)   "
					"		AND AppointmentsT.ShowState <> 3   "
					"		AND AppointmentsT.Status <> 4   "
					"		AND AppointmentsT.PatientID IN (SELECT AppointmentsT.PatientID  "
					"		FROM AppointmentsT  "
					"		INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID  "
					"		WHERE (AptTypeT.Category = 4 "
					"		OR AptTypeT.Category = 3) 	 "
					"		AND AppointmentsT.ShowState <> 3  "
					"		AND AppointmentsT.Status <> 4  "
					"		GROUP BY AppointmentsT.PatientID) "
					"		GROUP BY AppointmentsT.ID  "
					"	) AS ConsultsWithSurgQ ON AppointmentsT.ID = ConsultsWithSurgQ.ID  "
					"  WHERE ConsultsQ.ID IS NOT NULL ");
				break;


			case 609: 
				/* Version History
				// (j.gruber 2007-08-15 12:53) - PLID 13873 - created
				// (j.gruber 2009-03-25 14:11) - PLID 33696 - updated discount structure
				*/
				{
					CString strSql;
					strSql.Format("	SELECT ProcID as ProcID, SumMoney, Tax1, Tax2, Name as ProcName, PersonID as PatID, First, Last, Middle, HomePhone, WorkPhone, CellPhone, "
						" Code, SubCode, UserdefinedID as PatientID, ReferralID as ReferralID, ReferralName, ProvID as ProvID, LocID as LocID, "
						" ChargeTDate as ChargeTDate, ChargeIDate as ChargeIDate, PayTDate as PayTDate, PayIDate as PayIDate, "
						" ProvFirst, ProvMiddle, ProvLast, LocationName "
						" FROM ( "
						" SELECT CASE WHEN ProcedureID IS NULL THEN -1 ELSE ServiceT.ProcedureID END AS ProcID,   "
						" 		 SUM(AppliesT.Amount) AS SumMoney,    "
						" 		 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,   "
						" 		Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, ProcedureT.Name, "
						" 		PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, "
						"	    CASE WHEN CPTCodeT.Code IS NULL THEN ServiceT.Name ELSE CPTCodeT.Code END AS Code, "
						"       CASE WHEN CPTCodeT.SubCode IS NULL THEN '' ELSE CPTCodeT.SubCode END AS SubCode, "
						"       PatientsT.UserDefinedID, PatientsT.ReferralID AS ReferralID, "
						"       ReferralSourceT.Name as ReferralName, ChargesT.DoctorsProviders as ProvID, LineChargesT.LocationID AS LocID, "
						"       LineChargesT.Date as ChargeTDate, LineChargesT.InputDate As ChargeIDate, LinePaymentsT.Date as PayTDate, LinePaymentsT.InputDate as PayIDate, "
						"	    ProvPersonT.First as ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, LocationsT.Name As LocationName "
						"		 FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID     "
						"		  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
						"		  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
						"		LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
						"		  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
						"		  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
						"		  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
						"		 LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
						"		 LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID  "
						"		  LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCOdeT.ID "
						"		LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "	
						"       LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"		 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND    "
						"		 AppliesT.ID IS NOT NULL  AND ServiceT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID Is Null  "
						"		 GROUP BY ChargesT.ID, ServiceT.ProcedureID, ServiceT.Name, ProcedureT.Name, PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID, PatientsT.ReferralID, ReferralSourceT.Name, "
						"    	ChargesT.DoctorsProviders, LineChargesT.LocationID, "
						"       LineChargesT.Date, LineChargesT.InputDate, LinePaymentsT.Date, LinePaymentsT.InputDate,ProvPersonT.First, ProvPersonT.Middle, ProvPersonT.Last, LocationsT.Name"
						" UNION ALL "
						"		 SELECT CASE WHEN ServiceT.ProcedureID IS NULL THEN -1 ELSE MasterProcT.ID END AS ProcID,   "
						" 		 SUM(AppliesT.Amount) AS SumMoney,    "
						" 		 Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate - 1)) AS Tax1,   "
						" 		Sum(Convert(money,(((LineChargesT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)* (CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))))) * (ChargesT.TaxRate2 - 1)) AS Tax2, "
						"		MasterProcT.Name, "
						" 		PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, "
						"	    CASE WHEN CPTCodeT.Code IS NULL THEN ServiceT.Name ELSE CPTCodeT.Code END AS Code, "
						"       CASE WHEN CPTCodeT.SubCode IS NULL THEN '' ELSE CPTCodeT.SubCode END AS SubCode, "
						"		PatientsT.UserDefinedID, PatientsT.ReferralID AS ReferralID,  "
						"       ReferralSourceT.Name as ReferralName, ChargesT.DoctorsProviders  as ProvID, LineChargesT.LocationID AS LocID, "
						"       LineChargesT.Date as ChargeTDate, LineChargesT.InputDate As ChargeIDate, LinePaymentsT.Date as PayTDate, LinePaymentsT.InputDate as PayIDate, "
						"	    ProvPersonT.First as ProvFirst, ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, LocationsT.Name As LocationName "
						"		 FROM LineItemT LinePaymentsT INNER JOIN PaymentsT ON LinePaymentsT.ID = PaymentsT.ID     "
						"		  LEFT JOIN AppliesT ON LinePaymentsT.ID = AppliesT.SourceID LEFT JOIN ChargesT ON AppliesT.DestID = ChargesT.ID    "
						"		  LEFT JOIN LineItemT LineChargesT ON ChargesT.ID = LineChargesT.ID    "
						"		LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff, Sum(Discount) As TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountsQ ON ChargesT.ID = TotalDiscountsQ.ChargeID "
						"		  LEFT JOIN PatientsT ON LinePaymentsT.PatientID = PatientsT.PersonID LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
						"		  LEFT JOIN PersonT CoordPersonT ON ChargesT.PatCoordID = CoordPersonT.ID    "
						"		  LEFT JOIN PersonT ProvPersonT ON ChargesT.DoctorsProviders = ProvPersonT.ID LEFT JOIN LocationsT ON LineChargesT.LocationID = LocationsT.ID     "
						"		 LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID  "
						"		 LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID  "
						"        LEFT JOIN ProcedureT MasterProcT ON ProcedureT.MasterProcedureID = MasterProcT.ID "
						"		 LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
						"		 LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
						"       LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"		 WHERE LinePaymentsT.Deleted = 0 AND LineChargesT.Deleted = 0 AND LinePaymentsT.Type = 1 AND LineChargesT.Type = 10 AND    "
						"		 AppliesT.ID IS NOT NULL  AND ServiceT.ProcedureID IS NOT NULL AND ProcedureT.MasterProcedureID Is NOT Null  "
						"		 GROUP BY ChargesT.ID, ServiceT.ProcedureID, MasterProcT.Name, MasterProcT.ID, ServiceT.Name, ProcedureT.Name, PatientsT.PersonID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, CPTCodeT.Code, CPTCodeT.SubCode, PatientsT.UserDefinedID, PatientsT.ReferralID, ReferralSourceT.Name, "
						"    	ChargesT.DoctorsProviders, LineChargesT.LocationID, "
						"       LineChargesT.Date, LineChargesT.InputDate, LinePaymentsT.Date, LinePaymentsT.InputDate, "
						"	    ProvPersonT.First, ProvPersonT.Middle, ProvPersonT.Last, LocationsT.Name)Q");
						

					return strSql;
				}
				break;


				case 610: //Tracking Conversions Summary
					{
						/*Version History - 
						// (j.gruber 2007-08-20 15:18) - PLID 24698 - Created
						// (j.gruber 2008-02-14 16:31) - PLID 28365 - made it filter/group by provider
						// (j.gruber 2008-02-14 16:32) -  - PLID 28938 - changed the where clause of the TotalEnd/BeginDateComplete calculations and the TotalBegin/EndDateActive calculations

						*/

						switch (nSubLevel) {

							case 0:
								{
								CString strSql;
								COleDateTime dtFrom, dtTo;

								if (nDateRange == -1) {
									dtFrom.SetDate(1800,01,01);
									dtTo.SetDate(5000,12,31);
								}
								else {
								
									dtFrom = DateFrom;
									COleDateTimeSpan day(1,0,0,0);
									dtTo = DateTo + day;
								}

								CString strInnerProcFilter;
								if (! strExternalFilter.IsEmpty()) {
									long nResult = strExternalFilter.Find("[#Temp");
									if (nResult != -1) {

										CString strTemp;
										strTemp = strExternalFilter.Right(strExternalFilter.GetLength() - (nResult));

										CString strTemp2 = " ((SELECT ID FROM " + strTemp;

										strInnerProcFilter.Format("   AND StepsT.LadderID IN (SELECT LaddersT.ID FROM LaddersT  "
											" 			LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID  "
											" 			LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
											" 			LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
											" 			WHERE (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null) AND ProcedureT.ID IN %s)", strTemp2);
									}
								}
									
								
								strSql.Format("  "
									" DECLARE @DateFrom datetime; "
									" SET @DateFrom = convert(datetime, '%s'); "
									" DECLARE @DateTo datetime; "
									" SET @DateTo = convert(datetime, '%s'); "
																		
									" SELECT  UserDefinedID, TrackingConversionT.LadderTemplateID as LadderTemplateID, BeginStepTemplateID, EndStepTemplateID, TrackingConversionT.StepOrder, TrackingConversionT.Name as ConvName, BeginStepTemplatesT.StepName as BeginStepName, EndStepTemplatesT.StepName EndStepName, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder))))  AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) %s ) AS TotalBeginStepActive, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo) %s ) AS TotalBeginStepCompleted, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID)  AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) %s ) As TotalEndStepActive,  "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) %s ) As TotalEndStepCompleted,  "
									""
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) AS TotalBeginStepActiveProv, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) AS TotalBeginStepCompletedProv, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID)  AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom  AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) As TotalEndStepActiveProv,  "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) As TotalEndStepCompletedProv,  "
									""
									//" CASE WHEN BeginEventsT.Type IS NULL THEN CASE WHEN (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) = BeginStepsT.ID THEN BeginStepsT.ActiveDate ELSE NULL END ELSE CASE WHEN BeginStepsT.ActiveDate IS NULL THEN (SELECT EventsT.Date FROM StepsT InnerStepsT INNER JOIN EventAppliesT ON InnerStepsT.ID = EventAppliesT.StepID INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE InnerStepsT.LadderID = BeginStepsT.LadderID AND InnerStepsT.StepOrder = (BeginStepsT.StepOrder - 1)) ELSE BeginStepsT.ActiveDate END END as BeginStepActiveDate,  "
									" BeginStepsT.ActiveDate as BeginStepActiveDate, "
									" CASE WHEN BeginEventsT.Type IS NULL THEN NULL  "
									" 	ELSE CASE WHEN BeginStepsT.CompletedDate IS NULL THEN  "
									" 		(SELECT EventsT.Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID WHERE EventAppliesT.StepID = BeginStepsT.ID AND EventsT.Type = 9)  "
									" 		ELSE BeginStepsT.CompletedDate END  "
									" 	END as BeginStepCompleteDate,  "
									//" CASE WHEN BeginStepsT.CompletedDate IS NULL THEN (SELECT EventsT.Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID WHERE EventAppliesT.StepID = BeginStepsT.ID AND EventsT.Type = 9) ELSE BeginStepsT.CompletedDate END as BeginStepCompleteDate, "
									//" CASE WHEN EndEventsT.Type IS NULL THEN CASE WHEN (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) = BeginStepsT.ID THEN	BeginStepsT.ActiveDate ELSE NULL END ELSE CASE WHEN EndStepsT.ActiveDate IS NULL THEN (SELECT EventsT.Date FROM StepsT InnerStepsT INNER JOIN EventAppliesT ON InnerStepsT.ID = EventAppliesT.StepID INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE InnerStepsT.LadderID = EndStepsT.LadderID AND InnerStepsT.StepOrder = (EndStepsT.StepOrder - 1)) ELSE EndStepsT.ActiveDate END END as EndStepActiveDate,  "
									" EndStepsT.ActiveDate as EndStepActiveDate, "
									
									" CASE WHEN EndEventsT.Type IS NULL THEN NULL "
									"          ELSE CASE WHEN EndStepsT.CompletedDate IS NULL THEN "
									" 		(SELECT Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID  "
									" 		 WHERE EventAppliesT.StepID = EndStepsT.ID AND EventsT.Type = 9) "
									" 		ELSE EndStepsT.CompletedDate  "
									" 		END  "
									" 	 END as EndStepCompleteDate,  "
									
									//" CASE WHEN EndStepsT.CompletedDate IS NULL THEN (SELECT Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID WHERE EventAppliesT.StepID = EndStepsT.ID AND EventsT.Type = 9) ELSE EndStepsT.CompletedDate END as EndStepCompleteDate, "
									" ProcedureT.ID as ProcID, ProcedureT.Name as ProcName, LaddersT.FirstInterestDate as Date, LaddersT.ID AS LadderID, LadderTemplatesT.Name as LadderTemplateName, "
									" PatientsQ.ProviderID as ProvID, ProvPersonT.First as ProvFirst, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle "
									" FROM TrackingConversionT  "
									" LEFT JOIN StepTemplatesT BeginStepTemplatesT ON TrackingConversionT.BeginStepTemplateID = BeginStepTemplatesT.ID "
									" LEFT JOIN StepsT BeginStepsT ON BeginStepTemplatesT.ID = BeginStepsT.StepTemplateID "
									" LEFT JOIN (EventAppliesT BeginEventAppliesT INNER JOIN EventsT BeginEventsT ON BeginEventAppliesT.EventID = BeginEventsT.ID) ON BeginStepsT.ID = BeginEventAppliesT.StepID "
									" LEFT JOIN StepTemplatesT EndStepTemplatesT ON TrackingConversionT.EndStepTemplateID = EndStepTemplatesT.ID "
									" LEFT JOIN StepsT EndStepsT ON EndStepTemplatesT.ID = EndStepsT.StepTemplateID "
									" LEFT JOIN (EventAppliesT EndEventAppliesT INNER JOIN EventsT EndEventsT ON EndEventAppliesT.EventID = EndEventsT.ID) ON EndStepsT.ID = EndEventAppliesT.StepID "
									" LEFT JOIN LaddersT ON BeginStepsT.LadderID = LaddersT.ID "
									" LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID "
									" LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
									" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
									" LEFT JOIN LadderTemplatesT ON BeginStepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
									" LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID "
									" LEFT JOIN (SELECT PatientsT.*, CASE WHEN PatientsT.MainPhysician IS NULL THEN -999 ELSE PatientsT.MainPhysician END AS ProviderID FROM PatientsT) PatientsQ ON PersonT.ID = PatientsQ.PersonID "
									" LEFT JOIN PersonT ProvPersonT ON PatientsQ.MainPhysician = ProvPersonT.ID "
									" WHERE BeginStepsT.LadderID = EndStepsT.LadderID AND (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null)",
									FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate), 
									strInnerProcFilter, strInnerProcFilter, strInnerProcFilter, strInnerProcFilter, 
									strInnerProcFilter, strInnerProcFilter, strInnerProcFilter, strInnerProcFilter);
									
								return strSql;
								}
						
							break;

							case 1:

								switch (nSubRepNum) {

									case 0: 
										return _T("SELECT LaddersT.FirstInterestDate as Date, LaddersT.Status, LadderStatusT.Name, "
											" ProcedureT.ID as ProcID, ProcedureT.Name as ProcName, "
											" StepTemplatesT.StepName as StepTemplateName, StepTemplatesT.ID as StepTemplateID, "
											" LadderTemplatesT.ID as LadderTemplateID, LadderTemplatesT.Name as LadderTemplateName, "
											" NextStepQ.LastStepTemplateID, StepTemplatesT.StepOrder, PatientsT.MainPhysician as ProvID "
											" FROM LadderStatusT LEFT JOIN LaddersT ON   "
											" LadderStatusT.ID = LaddersT.Status  "
											" LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID  "
											" LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
											" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
											" LEFT JOIN StepsT ON LaddersT.ID = StepsT.LadderID "
											" LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
											" LEFT JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
											" LEFT JOIN  "
											" 	(SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.ID Is Null THEN (SELECT Top 1 StepTemplateID FROM StepsT WHERE LadderID = LaddersT.ID ORDER BY StepOrder DESC)  ELSE StepsT.StepTemplateID END AS LastStepTemplateID   "
											" 		FROM LaddersT LEFT JOIN   "
											" 			(SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep   "
											" 			FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID   "
											" 			WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT)   "
											" 			GROUP BY LaddersT.ID   "
											" 			) NextStepSubQ   "
											" 	ON LaddersT.ID = NextStepSubQ.LadderID LEFT JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID AND NextStepSubQ.NextStep = StepsT.StepOrder LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID   "
											" 	) AS NextStepQ  ON LaddersT.ID = NextStepQ.LadderID "
											" LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID "
											" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
											" WHERE StepTemplatesT.InActive = 0 AND LadderStatusT.IsActive = 0 AND LaddersT.Status <> 1 AND (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null)  "); 

									break;

									default:
										return "";
									break;
								}
							break;

							default:
								return "";
							break;

						}
					}

				break;


				case 622:  
					{
						/*Version History - 
						// (j.gruber 2008-02-14 16:32) - PLID 28367 - created by patient coordinator report
						
						*/

						switch (nSubLevel) {

							case 0:
								{
								CString strSql;
								COleDateTime dtFrom, dtTo;

								if (nDateRange == -1) {
									dtFrom.SetDate(1800,01,01);
									dtTo.SetDate(5000,12,31);
								}
								else {
								
									dtFrom = DateFrom;
									COleDateTimeSpan day(1,0,0,0);
									dtTo = DateTo + day;
								}

								CString strInnerProcFilter;
								if (! strExternalFilter.IsEmpty()) {
									long nResult = strExternalFilter.Find("[#Temp");
									if (nResult != -1) {

										CString strTemp;
										strTemp = strExternalFilter.Right(strExternalFilter.GetLength() - (nResult));

										CString strTemp2 = " ((SELECT ID FROM " + strTemp;

										strInnerProcFilter.Format("   AND StepsT.LadderID IN (SELECT LaddersT.ID FROM LaddersT  "
											" 			LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID  "
											" 			LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
											" 			LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
											" 			WHERE (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null) AND ProcedureT.ID IN %s)", strTemp2);
									}
								}
									
								
								strSql.Format(" DECLARE @DateFrom datetime; "
									" SET @DateFrom = convert(datetime, '%s'); "
									" DECLARE @DateTo datetime; "
									" SET @DateTo = convert(datetime, '%s'); "

									"SELECT  UserDefinedID, TrackingConversionT.LadderTemplateID as LadderTemplateID, BeginStepTemplateID, EndStepTemplateID, TrackingConversionT.StepOrder, TrackingConversionT.Name as ConvName, BeginStepTemplatesT.StepName as BeginStepName, EndStepTemplatesT.StepName EndStepName, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder))))  AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) %s ) AS TotalBeginStepActive, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo) %s ) AS TotalBeginStepCompleted, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID)  AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) %s ) As TotalEndStepActive,  "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) %s ) As TotalEndStepCompleted,  "
									""
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) AS TotalBeginStepActiveProv, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) AS TotalBeginStepCompletedProv, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID)  AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom  AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) As TotalEndStepActiveProv,  "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) %s ) As TotalEndStepCompletedProv,  "
									""
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN EmployeeID IS NULL THEN -999 ELSE EmployeeID END AS EmployeeID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.EmployeeID = PatientsQ.EmpID)  %s ) AS TotalBeginStepActiveProvPatCoord, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN EmployeeID IS NULL THEN -999 ELSE EmployeeID END AS EmployeeID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.EmployeeID = PatientsQ.EmpID)  %s ) AS TotalBeginStepCompletedProvPatCoord, "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID)  AND (StepsT.ActiveDate IS NOT NULL AND ((EventsT.Type IS NOT NULL) OR (StepsT.ID = (SELECT Top 1 ID FROM StepsT InnerStepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND InnerStepsT.LadderID = StepsT.LadderID ORDER BY InnerStepsT.StepOrder)))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom  AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN EmployeeID IS NULL THEN -999 ELSE EmployeeID END AS EmployeeID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.EmployeeID = PatientsQ.EmpID)  %s ) As TotalEndStepActiveProvPatCoord,  "
									" (SELECT Count(StepsT.ID) FROM StepsT LEFT JOIN (EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID) ON StepsT.ID = EventAppliesT.StepID WHERE StepTemplateID = TrackingConversionT.EndStepTemplateID AND StepsT.LadderID IN (SELECT LadderID FROM StepsT WHERE StepTemplateID = TrackingConversionT.BeginStepTemplateID) AND ((StepsT.CompletedDate IS NOT NULL AND EventsT.Type IS NOT NULL) OR (StepsT.CompletedDate IS NULL AND StepsT.ID IN (SELECT StepID FROM EventAppliesT INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE StepID = StepsT.ID AND EventsT.Type = 9 ))) AND StepsT.LadderID IN (SELECT ID FROM LaddersT WHERE LaddersT.FirstInterestDate >= @DateFrom AND LaddersT.FirstInterestDate < @DateTo ) AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN MainPhysician IS NULL THEN -999 ELSE MainPhysician END AS ProviderID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.ProviderID = PatientsQ.ProviderID)  AND StepsT.LadderID IN (SELECT ID FROM LaddersT LEFT JOIN (SELECT PatientsT.PersonID, CASE WHEN EmployeeID IS NULL THEN -999 ELSE EmployeeID END AS EmployeeID FROM PatientsT) PatInnerT ON LaddersT.PersonID = PatInnerT.PersonID WHERE PatInnerT.EmployeeID = PatientsQ.EmpID) %s ) As TotalEndStepCompletedProvPatCoord,  "
									//" CASE WHEN BeginEventsT.Type IS NULL THEN CASE WHEN (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) = BeginStepsT.ID THEN BeginStepsT.ActiveDate ELSE NULL END ELSE CASE WHEN BeginStepsT.ActiveDate IS NULL THEN (SELECT EventsT.Date FROM StepsT InnerStepsT INNER JOIN EventAppliesT ON InnerStepsT.ID = EventAppliesT.StepID INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE InnerStepsT.LadderID = BeginStepsT.LadderID AND InnerStepsT.StepOrder = (BeginStepsT.StepOrder - 1)) ELSE BeginStepsT.ActiveDate END END as BeginStepActiveDate,  "
									" BeginStepsT.ActiveDate as BeginStepActiveDate, "
									" CASE WHEN BeginEventsT.Type IS NULL THEN NULL  "
									" 	ELSE CASE WHEN BeginStepsT.CompletedDate IS NULL THEN  "
									" 		(SELECT EventsT.Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID WHERE EventAppliesT.StepID = BeginStepsT.ID AND EventsT.Type = 9)  "
									" 		ELSE BeginStepsT.CompletedDate END  "
									" 	END as BeginStepCompleteDate,  "
									//" CASE WHEN BeginStepsT.CompletedDate IS NULL THEN (SELECT EventsT.Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID WHERE EventAppliesT.StepID = BeginStepsT.ID AND EventsT.Type = 9) ELSE BeginStepsT.CompletedDate END as BeginStepCompleteDate, "
									//" CASE WHEN EndEventsT.Type IS NULL THEN CASE WHEN (SELECT Top 1 ID FROM StepsT WHERE ID NOT IN (SELECT StepID FROM EventAppliesT) AND LadderID = LaddersT.ID ORDER BY StepOrder) = BeginStepsT.ID THEN	BeginStepsT.ActiveDate ELSE NULL END ELSE CASE WHEN EndStepsT.ActiveDate IS NULL THEN (SELECT EventsT.Date FROM StepsT InnerStepsT INNER JOIN EventAppliesT ON InnerStepsT.ID = EventAppliesT.StepID INNER JOIN EventsT ON EventAppliesT.EventID = EventsT.ID WHERE InnerStepsT.LadderID = EndStepsT.LadderID AND InnerStepsT.StepOrder = (EndStepsT.StepOrder - 1)) ELSE EndStepsT.ActiveDate END END as EndStepActiveDate,  "
									" EndStepsT.ActiveDate as EndStepActiveDate, "
									
									" CASE WHEN EndEventsT.Type IS NULL THEN NULL "
									"          ELSE CASE WHEN EndStepsT.CompletedDate IS NULL THEN "
									" 		(SELECT Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID  "
									" 		 WHERE EventAppliesT.StepID = EndStepsT.ID AND EventsT.Type = 9) "
									" 		ELSE EndStepsT.CompletedDate  "
									" 		END  "
									" 	 END as EndStepCompleteDate,  "
									
									//" CASE WHEN EndStepsT.CompletedDate IS NULL THEN (SELECT Date FROM EventsT INNER JOIN EventAppliesT ON EventsT.ID = EventAppliesT.EventID WHERE EventAppliesT.StepID = EndStepsT.ID AND EventsT.Type = 9) ELSE EndStepsT.CompletedDate END as EndStepCompleteDate, "
									" ProcedureT.ID as ProcID, ProcedureT.Name as ProcName, LaddersT.FirstInterestDate as Date, LaddersT.ID AS LadderID, LadderTemplatesT.Name as LadderTemplateName, "
									" PatientsQ.ProviderID as ProvID, ProvPersonT.First as ProvFirst, ProvPersonT.Last as ProvLast, ProvPersonT.Title as ProvTitle, "
									" PatientsQ.EmpID as EmployeeID, EmpPersonT.First as EmpFirst, EmpPersonT.Last as EmpLast "
									" FROM TrackingConversionT  "
									" LEFT JOIN StepTemplatesT BeginStepTemplatesT ON TrackingConversionT.BeginStepTemplateID = BeginStepTemplatesT.ID "
									" LEFT JOIN StepsT BeginStepsT ON BeginStepTemplatesT.ID = BeginStepsT.StepTemplateID "
									" LEFT JOIN (EventAppliesT BeginEventAppliesT INNER JOIN EventsT BeginEventsT ON BeginEventAppliesT.EventID = BeginEventsT.ID) ON BeginStepsT.ID = BeginEventAppliesT.StepID "
									" LEFT JOIN StepTemplatesT EndStepTemplatesT ON TrackingConversionT.EndStepTemplateID = EndStepTemplatesT.ID "
									" LEFT JOIN StepsT EndStepsT ON EndStepTemplatesT.ID = EndStepsT.StepTemplateID "
									" LEFT JOIN (EventAppliesT EndEventAppliesT INNER JOIN EventsT EndEventsT ON EndEventAppliesT.EventID = EndEventsT.ID) ON EndStepsT.ID = EndEventAppliesT.StepID "
									" LEFT JOIN LaddersT ON BeginStepsT.LadderID = LaddersT.ID "
									" LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID "
									" LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
									" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
									" LEFT JOIN LadderTemplatesT ON BeginStepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
									" LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID "
									" LEFT JOIN (SELECT PatientsT.*, CASE WHEN PatientsT.MainPhysician IS NULL THEN -999 ELSE PatientsT.MainPhysician END AS ProviderID, CASE WHEN PatientsT.EmployeeID IS NULL THEN -999 ELSE PatientsT.EmployeeID END AS EmpID FROM PatientsT) PatientsQ ON PersonT.ID = PatientsQ.PersonID "
									" LEFT JOIN PersonT ProvPersonT ON PatientsQ.MainPhysician = ProvPersonT.ID "
									" LEFT JOIN PersonT EmpPersonT ON PatientsQ.EmpID = EmpPersonT.ID "
									" WHERE BeginStepsT.LadderID = EndStepsT.LadderID AND (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null)",
									FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate), 
									strInnerProcFilter, strInnerProcFilter, strInnerProcFilter, strInnerProcFilter,
									strInnerProcFilter, strInnerProcFilter, strInnerProcFilter, strInnerProcFilter,
									strInnerProcFilter, strInnerProcFilter, strInnerProcFilter, strInnerProcFilter);									
									
								return strSql;
								}
						
							break;

							case 1:

								switch (nSubRepNum) {

									case 0: 
										return _T("SELECT LaddersT.FirstInterestDate as Date, LaddersT.Status, LadderStatusT.Name, "
											" ProcedureT.ID as ProcID, ProcedureT.Name as ProcName, "
											" StepTemplatesT.StepName as StepTemplateName, StepTemplatesT.ID as StepTemplateID, "
											" LadderTemplatesT.ID as LadderTemplateID, LadderTemplatesT.Name as LadderTemplateName, "
											" NextStepQ.LastStepTemplateID, StepTemplatesT.StepOrder, PatientsT.MainPhysician as ProvID, PatientsT.EmployeeID as EmpID "
											" FROM LadderStatusT LEFT JOIN LaddersT ON   "
											" LadderStatusT.ID = LaddersT.Status  "
											" LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID  "
											" LEFT JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID  "
											" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID  "
											" LEFT JOIN StepsT ON LaddersT.ID = StepsT.LadderID "
											" LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
											" LEFT JOIN LadderTemplatesT ON StepTemplatesT.LadderTemplateID = LadderTemplatesT.ID "
											" LEFT JOIN  "
											" 	(SELECT LaddersT.ID AS LadderID, CASE WHEN StepsT.ID Is Null THEN (SELECT Top 1 StepTemplateID FROM StepsT WHERE LadderID = LaddersT.ID ORDER BY StepOrder DESC)  ELSE StepsT.StepTemplateID END AS LastStepTemplateID   "
											" 		FROM LaddersT LEFT JOIN   "
											" 			(SELECT LaddersT.ID AS LadderID, Min(StepsT.StepOrder) AS NextStep   "
											" 			FROM LaddersT INNER JOIN StepsT ON LaddersT.ID = StepsT.LadderID INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID   "
											" 			WHERE StepsT.ID NOT IN (SELECT StepID FROM EventAppliesT)   "
											" 			GROUP BY LaddersT.ID   "
											" 			) NextStepSubQ   "
											" 	ON LaddersT.ID = NextStepSubQ.LadderID LEFT JOIN StepsT ON NextStepSubQ.LadderID = StepsT.LadderID AND NextStepSubQ.NextStep = StepsT.StepOrder LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID   "
											" 	) AS NextStepQ  ON LaddersT.ID = NextStepQ.LadderID "
											" LEFT JOIN PersonT ON LaddersT.PersonID = PersonT.ID "
											" LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
											" WHERE StepTemplatesT.InActive = 0 AND LadderStatusT.IsActive = 0 AND LaddersT.Status <> 1 AND (ProcInfoDetailsT.Chosen = 1 OR ProcedureT.MasterProcedureID Is Null)  "); 

									break;

									default:
										return "";
									break;
								}
							break;

							default:
								return "";
							break;

						}
					}

				break;

				case 614 : //Marketing Review
					/* Version History 
					// (j.gruber 2007-11-01 11:58) - PLID 27814 - Created
					// (j.gruber 2008-10-23 08:39) - PLID 31796 - Added short Purposestring and some extra sub queries
					TES 8/4/2009 - PLID 35047 - Other Procedural category (AptTypeT.Category = 6) should not be treated as a procedure.
					// (d.thompson 2009-08-12) - PLID 35187 - Fixed a join to AppointmentPurposeT using the ID, not the PurposeID.
					*/
					{
					CString strTemp1, strTemp2;
					strTemp1 = "SELECT FirstContactDate as Date, 'Prospects' as RecordType, CurrentStatus, EmployeeID, -1 as AptTypeID, CoordFirst, CoordMiddle, CoordLast, -1 as ApptID, -1 as ShowState, -1 as AptCategory, ReferralID, PrimaryReferralSourceName, "
						" PatFirst, PatMiddle, PatLast, PatAdd1, PatAdd2, PatCity, PatState, PatZip, ProvID as ProvID, LocID as LocID, LocName, ProvFirst, ProvMiddle, ProvLast, UserID as InputUserID, InputFirst, InputLast, -1 as ApptStatus,  -1 as SurgAptType,  '-1' As ApptPurposeIDString    "
						" FROM ( "
						"     SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last As CoordLast, "
						"     PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"    PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						"     PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName, PersonT.UserID, InputUserT.First as InputFirst, InputUserT.Last as InputLast "
						"     FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"     LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
						"     LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"     LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"     LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						"     LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"     WHERE CurrentStatus = 2   "
						"     AND PersonT.FirstContactDate [From]  "
						"     AND PersonT.FirstContactDate [To] "
						"     [ProvPat] "
						"     [LocPat]  "
						"     UNION  	  "
						"     SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last As CoordLast, "
						"     PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"     PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						"     PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName, PersonT.UserID, InputUserT.First as InputUser, InputUserT.Last "
						"    FROM PatientStatusHistoryT INNER JOIN PatientsT   "
						"     ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"     LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
						"     LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"    LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"     LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						"     LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"     WHERE OldStatus = 2  "
						"     AND PersonT.FirstContactDate [From] "
						"     AND PersonT.FirstContactDate [To] "
						"     [ProvPat] "
						"     [LocPat]  "
						"  ) Q "
						"  UNION ALL "
						" SELECT CreatedDate, 'InputAppt', PatientsT.CurrentStatus, PatientsT.EmployeeID, AppointmentsT.AptTypeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, AppointmentsT.ID as ApptID, ShowState, AptTypeT.Category, PatientsT.ReferralID, ReferralSourceT.Name, "
						" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						" PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						" PersonT.UserID, InputUserT.First, InputUserT.Last, AppointmentsT.Status,  -1 as SurgAptType, dbo.GetPurposeIDString(AppointmentsT.ID) AS ApptPurposeIDString    "
						" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						" LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PersonT.ID "
						" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
						" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						" LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						" LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
						" LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
						" LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						" WHERE (1=1) "
						"   AND AppointmentsT.CreatedDate [From] "
						"   AND AppointmentsT.CreatedDate [To] "
						"   [ProvPat] "
						"   [LocAppt]  "
						"   [Resource] "
						" UNION ALL "
						" SELECT CreatedDate, 'FirstInputAppt', PatientsT.CurrentStatus, PatientsT.EmployeeID, AppointmentsT.AptTypeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, AppointmentsT.ID as ApptID, ShowState, AptTypeT.Category, PatientsT.ReferralID, ReferralSourceT.Name, "
						" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						" PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						" PersonT.UserID, InputUserT.First, InputUserT.Last, AppointmentsT.Status,  -1 as SurgAptType, dbo.GetPurposeIDString(AppointmentsT.ID) AS ApptPurposeIDString   "
						" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						" LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PersonT.ID "
						" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
						" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						" LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						" LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
						" LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
						" LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						" WHERE (1=1) "
						"   AND AppointmentsT.ID = (SELECT Min(ID) FROM AppointmentsT InnerApptsT WHERE InnerApptsT.PatientID = AppointmentsT.PatientID AND InnerApptsT.StartTime = (SELECT Min(StartTime) FROM AppointmentsT Apt2 WHERE Apt2.PatientID = InnerApptsT.PatientID)) "
						"   AND AppointmentsT.CreatedDate [From] "
						"   AND AppointmentsT.CreatedDate [To] "
						"   [ProvPat] "
						"   [LocAppt]  "
						"   [Resource] ";
					strTemp2 = 

						" UNION ALL  "
						" SELECT AppointmentsT.Date, 'SeenAppt', PatientsT.CurrentStatus, PatientsT.EmployeeID, AptTypeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, AppointmentsT.ID as ApptID, ShowState, AptTypeT.Category, PatientsT.ReferralID, ReferralSourceT.Name, "
						" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						" PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						" PersonT.UserID, InputUserT.First, InputUserT.Last, -1 as Status,  -1 as SurgAptType, dbo.GetPurposeIDString(AppointmentsT.ID) AS ApptPurposeIDString    "
						" FROM AppointmentsT LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						" LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PersonT.ID "
						" LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
						" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						" LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						" LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
						" LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
						" LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						" WHERE AppointmentsT.Status <> 4 "
						"   AND AppointmentsT.Date [From] "
						"   AND AppointmentsT.Date [To] "
						"   [ProvPat] "
						"   [LocAppt]  "
						"   [Resource] "
						" UNION ALL "
						" SELECT ApptsOuterQ.Date, 'SurgeryFromConsult', PatientsT.CurrentStatus, PatientsT.EmployeeID, ApptsOuterQ.AptTypeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, ApptsOuterQ.ID, ApptsOuterQ.ShowState, AptTypeT.Category, PatientsT.ReferralID, ReferralSourceT.Name, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast,  "
						"  PersonT.UserID, InputUserT.First, InputUserT.Last, -1 as Status,  -1 as SurgAptType, dbo.GetPurposeIDString(ApptsOuterQ.ID) AS ApptPurposeIDString   "
						"   FROM AppointmentsT ApptsOuterQ    "
						"   INNER JOIN AptTypeT ON ApptsOuterQ.AptTypeID = AptTypeT.ID     "
						"   LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID     "
						"   LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
						"   LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PersonT.ID "
						"   LEFT JOIN AppointmentPurposeT ON ApptsOuterQ.ID = AppointmentPurposeT.AppointmentID  "
						"   LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"   LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"   LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID  "
						"   LEFT JOIN AppointmentResourceT ON ApptsOuterQ.ID = AppointmentResourceT.AppointmentID "
						"   LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
						"   LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"   WHERE (AptTypeT.Category = 4 	/*surgery */  "
						"   OR AptTypeT.Category = 3) 		/*Minor procedure  */  "
						"   AND ApptsOuterQ.Status <> 4     "
						"   AND PersonT.ID > 0 "
						"   AND ApptsOuterQ.Date [From] "
						"   AND ApptsOuterQ.Date [To] "
						"   [ProvPat] "
						"   [LocAppt2]  "
						"   [Resource] "
						"   AND ApptsOuterQ.PatientID IN (   "
						" 	SELECT AppointmentsT.PatientID FROM AppointmentsT    "
						" 	INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID     "
						" 	LEFT JOIN PatientsT PatientsInnerQ ON AppointmentsT.PatientID = PatientsInnerQ.PersonID    "
						" 	LEFT JOIN PersonT PersonInnerQ ON PatientsInnerQ.PersonID = PersonInnerQ.ID    "
						" 	LEFT JOIN AppointmentPurposeT ApptsPurposeInner ON AppointmentsT.ID = ApptsPurposeInner.AppointmentID  "
						" 	INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID  "
						" 	WHERE (AptTypeT.Category = 1)  /*consult  */  "
						" 	AND AppointmentsT.ShowState <> 3  /*3 = no show*/   "
						" 	AND AppointmentsT.Status <> 4  /*4 = cancelled  */  "
						"   AND AppointmentsT.Date          <= ApptsOuterQ.Date  "
						" 	AND AppointmentsT.Date <= ApptsOuterQ.Date  "
						" 	AND PersonInnerQ.ID > 0  "
						"	AND (ApptsPurposeInner.PurposeID = AppointmentPurposeT.PurposeID OR  "
						" 	ApptsPurposeInner.PurposeID = ProcedureT.MasterProcedureID)  "
						" 	[ProvPat] [LocAppt] "
						"   /*Not filtering consult resource */ "
						" 	)   "
						" UNION ALL "
						" SELECT ApptsOuterQ.Date, 'ConsultToSurgery', PatientsT.CurrentStatus, PatientsT.EmployeeID, ApptsOuterQ.AptTypeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, ApptsOuterQ.ID, ApptsOuterQ.ShowState, AptTypeT.Category, PatientsT.ReferralID, ReferralSourceT.Name, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast,  "
						"  PersonT.UserID, InputUserT.First, InputUserT.Last, -1 as Status,  -1 as SurgAptType, dbo.GetPurposeIDString(ApptsOuterQ.ID) AS ApptPurposeIDString "
						"   FROM AppointmentsT ApptsOuterQ    "
						"   INNER JOIN AptTypeT ON ApptsOuterQ.AptTypeID = AptTypeT.ID     "
						"   LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID     "
						"   LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
						"   LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PersonT.ID "
						"   LEFT JOIN AppointmentPurposeT ON ApptsOuterQ.ID = AppointmentPurposeT.AppointmentID  "
						"   LEFT JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID "
						"   LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"   LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"   LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID  "
						"   LEFT JOIN AppointmentResourceT ON ApptsOuterQ.ID = AppointmentResourceT.AppointmentID "
						"   LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
						"   LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"   WHERE (AptTypeT.Category = 1)  /*consult  */  "
						"   AND ApptsOuterQ.Status <> 4     "
						"   AND PersonT.ID > 0 "
						"   AND ApptsOuterQ.Date [From] "
						"   AND ApptsOuterQ.Date [To] "
						"   [ProvPat] "
						"   [LocAppt2]  "
						"   [Resource] "
						"   AND ApptsOuterQ.PatientID IN (   "
						" 	SELECT AppointmentsT.PatientID FROM AppointmentsT    "
						" 	INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID     "
						" 	LEFT JOIN PatientsT PatientsInnerQ ON AppointmentsT.PatientID = PatientsInnerQ.PersonID    "
						" 	LEFT JOIN PersonT PersonInnerQ ON PatientsInnerQ.PersonID = PersonInnerQ.ID    "
						" 	LEFT JOIN AppointmentPurposeT ApptsPurposeInner ON AppointmentsT.ID = ApptsPurposeInner.AppointmentID  "
						" 	INNER JOIN ProcedureT ON ApptsPurposeInner.PurposeID = ProcedureT.ID  "
						" 	WHERE "
						"   (AptTypeT.Category = 4 	/*surgery */  "
						"   OR AptTypeT.Category = 3) 		/*Minor procedure  */  "
						" 	AND AppointmentsT.ShowState <> 3  /*3 = no show*/   "
						" 	AND AppointmentsT.Status <> 4  /*4 = cancelled  */  "
						"   AND AppointmentsT.Date          > ApptsOuterQ.Date  "
						" 	AND AppointmentsT.Date > ApptsOuterQ.Date  "
						" 	AND PersonInnerQ.ID > 0  "
						"	AND (ApptsPurposeInner.PurposeID = AppointmentPurposeT.PurposeID OR  "
						" 	AppointmentPurposeT.PurposeID = ProcedureT.MasterProcedureID)  "
						" 	[ProvPat] [LocAppt] "
						"   /*Not filtering surgery resource */ "
						" 	)   "
						" UNION ALL "
						" SELECT ApptsOuterQ.CreatedDate, 'SurgeryFromConsultSeen', PatientsT.CurrentStatus, PatientsT.EmployeeID, ApptsOuterQ.AptTypeID, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, ApptsOuterQ.ID, ApptsOuterQ.ShowState, AptTypeT.Category, PatientsT.ReferralID, ReferralSourceT.Name, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast,  "
						"  PersonT.UserID, InputUserT.First, InputUserT.Last, -1 as Status,  -1 as SurgAptType, dbo.GetPurposeIDString(ApptsOuterQ.ID) AS ApptPurposeIDString   "
						"   FROM AppointmentsT ApptsOuterQ    "
						"   INNER JOIN AptTypeT ON ApptsOuterQ.AptTypeID = AptTypeT.ID     "
						"   LEFT JOIN PatientsT ON ApptsOuterQ.PatientID = PatientsT.PersonID     "
						"   LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID    "
						"   LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PersonT.ID "
						"   LEFT JOIN AppointmentPurposeT ON ApptsOuterQ.ID = AppointmentPurposeT.AppointmentID  "
						"   LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"   LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"   LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID  "
						"   LEFT JOIN AppointmentResourceT ON ApptsOuterQ.ID = AppointmentResourceT.AppointmentID "
						"   LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
						"   LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"   WHERE (AptTypeT.Category = 4 	/*surgery */  "
						"   OR AptTypeT.Category = 3) 		/*Minor procedure  */  "
						"   AND ApptsOuterQ.Status <> 4     "
						"   AND PersonT.ID > 0 "
						"   AND ApptsOuterQ.CreatedDate [From] "
						"   AND ApptsOuterQ.CreatedDate [To] "
						"   [ProvPat] "
						"   [LocAppt2]  "
						"   [Resource] "
						"   AND ApptsOuterQ.PatientID IN (   "
						" 	SELECT AppointmentsT.PatientID FROM AppointmentsT    "
						" 	INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID     "
						" 	LEFT JOIN PatientsT PatientsInnerQ ON AppointmentsT.PatientID = PatientsInnerQ.PersonID    "
						" 	LEFT JOIN PersonT PersonInnerQ ON PatientsInnerQ.PersonID = PersonInnerQ.ID    "
						" 	LEFT JOIN AppointmentPurposeT ApptsPurposeInner ON AppointmentsT.ID = ApptsPurposeInner.AppointmentID  "
						" 	INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID  "
						" 	WHERE (AptTypeT.Category = 1)  /*consult  */  "
						" 	AND AppointmentsT.ShowState <> 3  /*3 = no show*/   "
						" 	AND AppointmentsT.Status <> 4  /*4 = cancelled  */  "
						"   AND AppointmentsT.Date          <= ApptsOuterQ.Date  "
						" 	AND AppointmentsT.Date [From] "
						" 	AND AppointmentsT.Date [To] "
						" 	AND PersonInnerQ.ID > 0  "
						"	AND (ApptsPurposeInner.PurposeID = AppointmentPurposeT.PurposeID OR  "
						" 	ApptsPurposeInner.PurposeID = ProcedureT.MasterProcedureID)  "
						" 	[ProvPat] [LocAppt] "
						"   /*Not filtering consult resource */ "
						" 	)   "
						" UNION ALL "
						"  SELECT FirstContactDate, 'ProsWithCons' as RecordType, CurrentStatus, EmployeeID, -1 as AptTypeID, CoordFirst, CoordMiddle, CoordLast, -1 as ApptID, -1 as ShowState, -1, ReferralID, PrimaryReferralSourceName, "
						" PatFirst, PatMiddle, PatLast, PatAdd1, PatAdd2, PatCity, PatState, PatZip, ProvID as ProvID, LocID as LocID, LocName, ProvFirst, ProvMiddle, ProvLast, "
						" InputUserID, InputFirst, InputLast, -1 as Status,  -1 as SurgAptType,  '-1' as ApptPurposeIDString   "
						" FROM (  "
						"  SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						" PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName,  "
						" PersonT.UserID as InputUserID, InputUserT.First as InputFirst, InputUserT.Last as InputLast "
						"  FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"  LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID 	 "
						"  LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"  LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"  LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID		  "
						"  LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"  WHERE CurrentStatus = 2     "
						"  AND PersonT.FirstContactDate [From] "
						"  AND PersonT.FirstContactDate [To] "
						"  [ProvPat] [LocPat]  "
						"  UNION     "
						"  SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First AS CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last AS CoordLast, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						"  PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName, "
						"  PersonT.UserID, InputUserT.First, InputUserT.Last "
						"  FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
						"  LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
						"  LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"  LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						"  LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"  WHERE OldStatus = 2  "
						"  AND PersonT.FirstContactDate [From] "
						"  AND PersonT.FirstContactDate [To] "
						"  [ProvPat] [LocPat] "
						"  ) AS ProsWithConsQ   "
						"  WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
						" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
						"   LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
						"   WHERE (AptTypeT.Category = 1)   "
						" 	AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [LocAppt] [Resource]  )  "
						" UNION ALL "
						" SELECT Date, 'Pays', CurrentStatus, EmployeeID, -1, PatCoordT.First, PatCoordT.Middle, PatCoordT.Last, -1, -1, -1, ReferralID, ReferralSourceT.Name, "
						" PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						" PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						" PersonT.UserID, InputUserT.First, InputUserT.Last, -1 as Status, "

						" (SELECT top 1 AppointmentsT.AptTypeID "
						"         FROM    AppointmentsT  "
						"         LEFT JOIN AptTypeT  "
						"         ON      AppointmentsT.AptTypeID = AptTypeT.ID  "
						"         WHERE (AptTypeT.Category IN (3,4))  "
						"             AND AppointmentsT.ShowState <> 3 "
						"             AND AppointmentsT.Status <> 4 "
						"             AND AppointmentsT.Date [From] "
						"             AND AppointmentsT.Date  [To]  "
						" 	    AND AppointmentsT.PatientID = PatientsT.PersonID "
						"         "
						"         )  AS SurgeryApptTypeID, "
						" (SELECT top 1 dbo.GetPurposeIDString(AppointmentsT.ID) "
						"         FROM    AppointmentsT  "
						"         LEFT JOIN AptTypeT  "
						"         ON      AppointmentsT.AptTypeID = AptTypeT.ID  "
						"         WHERE (AptTypeT.Category IN (3,4))  "
						"             AND AppointmentsT.ShowState <> 3 "
						"             AND AppointmentsT.Status <> 4 "
						"             AND AppointmentsT.Date [From] "
						"             AND AppointmentsT.Date  [To]  "
						" 	    AND AppointmentsT.PatientID = PatientsT.PersonID "
						"         "
						"         )  AS ApptPurposeIDString  "
						
						" FROM PaymentsT INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
						" LEFT JOIN PatientsT ON LineItemT.PatientID = PatientsT.PersonID "
						" LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
						" LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						" LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
						" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						" LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						" LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						" "
						
						" WHERE LineItemT.Deleted = 0 "
						" AND LineItemT.Type = 1 "
						" AND LineItemT.Date [From] " 
						" AND LineItemT.Date [To] "
						" [ProvPay] [LocPay]  "
						
						" UNION ALL "
						"SELECT FirstContactDate as Date, 'ProspectsInput' as RecordType, CurrentStatus, InputUserID, -1 as AptTypeID, InputFirst, '', InputLast, -1 as ApptID, -1 as ShowState, -1 as AptCategory, ReferralID, PrimaryReferralSourceName, "
						" PatFirst, PatMiddle, PatLast, PatAdd1, PatAdd2, PatCity, PatState, PatZip, ProvID as ProvID, LocID as LocID, LocName, ProvFirst, ProvMiddle, ProvLast, InputUserID, InputFirst, InputLast, -1 as Status, -1 as SurgAptType, '-1' as ApptPurposeIDString      "
						" FROM ( "
						"     SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last As CoordLast, "
						"     PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"    PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						"     PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName, PersonT.UserID as InputUserID, InputUserT.First as InputFirst, InputUserT.Last as InputLast "
						"     FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"     LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
						"     LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"     LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"     LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						"     LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"     WHERE CurrentStatus = 2   "
						"     AND PersonT.FirstContactDate [From]  "
						"     AND PersonT.FirstContactDate [To] "
						"     [ProvPat] "
						"     [LocPat]  "
						"     UNION  	  "
						"     SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First as CoordFirst, PatCoordT.Middle as CoordMiddle, PatCoordT.Last As CoordLast, "
						"     PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"     PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						"     PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName, PersonT.UserID, InputUserT.First as InputUser, InputUserT.Last "
						"    FROM PatientStatusHistoryT INNER JOIN PatientsT   "
						"     ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"     LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID "
						"     LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"    LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"     LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						"     LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"     WHERE OldStatus = 2  "
						"     AND PersonT.FirstContactDate [From] "
						"     AND PersonT.FirstContactDate [To] "
						"     [ProvPat] "
						"     [LocPat]  "
						"  ) Q "
						" UNION ALL "
						"  SELECT FirstContactDate, 'ProsWithConsInput' as RecordType, CurrentStatus, InputUserID, -1 as AptTypeID, InputFirst, '', InputLast, -1 as ApptID, -1 as ShowState, -1, ReferralID, PrimaryReferralSourceName, "
						" PatFirst, PatMiddle, PatLast, PatAdd1, PatAdd2, PatCity, PatState, PatZip, ProvID as ProvID, LocID as LocID, LocName, ProvFirst, ProvMiddle, ProvLast, "
						" InputUserID, InputFirst, InputLast, -1 as Status,  -1 as SurgAptType, '-1' as ApptPurposeIDString    "
						" FROM (  "
						"  SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First As CoordFirst, PatCoordT.Middle AS CoordMiddle, PatCoordT.Last AS CoordLast, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						" PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName,  "
						" PersonT.UserID as InputUserID, InputUserT.First as InputFirst, InputUserT.Last as InputLast "
						"  FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
						"  LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID 	 "
						"  LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						"  LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"  LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID		  "
						"  LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"  WHERE CurrentStatus = 2     "
						"  AND PersonT.FirstContactDate [From] "
						"  AND PersonT.FirstContactDate [To] "
						"  [ProvPat] [LocPat]  "
						"  UNION     "
						"  SELECT PersonT.FirstContactDate, PatientsT.PersonID, PatientsT.CurrentStatus, PatientsT.EmployeeID, PatCoordT.First AS CoordFirst, PatCoordT.Middle As CoordMiddle, PatCoordT.Last AS CoordLast, "
						"  PersonT.First as PatFirst, PersonT.Middle as PatMiddle, PersonT.Last as PatLast, PersonT.Address1 as PatAdd1, PersonT.Address2 as PatAdd2, PersonT.City as PatCity, PersonT.State as PatState, "
						"  PersonT.Zip as PatZip, PatientsT.MainPhysician as ProvID, PersonT.Location as LocID, LocationsT.Name as LocName, ProvT.First as ProvFirst, ProvT.Middle as ProvMiddle, ProvT.Last as ProvLast, "
						"  PatientsT.ReferralID, ReferralSourceT.Name as PrimaryReferralSourceName, "
						"  PersonT.UserID, InputUserT.First, InputUserT.Last "
						"  FROM PatientStatusHistoryT INNER JOIN PatientsT ON PatientStatusHistoryT.PersonID = PatientsT.PersonID INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID   "
						"  LEFT JOIN PersonT PatCoordT ON PatientsT.EmployeeID = PatCoordT.ID  "
						"  LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
						" LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID "
						"  LEFT JOIN PersonT ProvT ON PatientsT.MainPhysician = ProvT.ID "
						"  LEFT JOIN PersonT InputUserT ON PersonT.UserID = InputUserT.ID "
						"  WHERE OldStatus = 2  "
						"  AND PersonT.FirstContactDate [From] "
						"  AND PersonT.FirstContactDate [To] "
						"  [ProvPat] [LocPat] "
						"  ) AS ProsWithConsQ   "
						"  WHERE ProsWithConsQ.PersonID IN ( SELECT PatientID FROM AppointmentsT  INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID    "
						" 	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
						"   LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
						"   WHERE (AptTypeT.Category = 1)   "
						" 	AND AppointmentsT.ShowState <> 3  AND AppointmentsT.Status <> 4 [LocAppt] [Resource]  )  "
						
						
						;

						COleDateTime dtFrom, dtTo;
						if (nDateRange == -1) {
							dtFrom.SetDate(1800,01,01);
							dtTo.SetDate(2020,12,31);
						}
						else {
						
							dtFrom = DateFrom;
							dtTo = DateTo;
						}

						CString strDateTo, strDateFrom;
						strDateTo.Format(" < DateAdd(day, 1, '%s')", FormatDateTimeForSql(dtTo, dtoDate));
						strDateFrom.Format(" >=  '%s'", FormatDateTimeForSql(dtFrom, dtoDate));

						//location
						//(e.lally 2008-10-01) PLID 31541 - Support filtering on multiple locations
						CString strLocPat, strLocAppt, strLocAppt2, strLocPay;
						if (nLocation > 0) {
							strLocPat.Format(" AND PersonT.Location = %li", nLocation);
							strLocAppt.Format(" AND AppointmentsT.LocationID = %li", nLocation);
							strLocAppt2.Format(" AND ApptsOuterQ.LocationID = %li", nLocation);
							strLocPay.Format(" AND LineItemT.LocationID = %li", nLocation);
						}
						else if(nLocation == -2){
							//(e.lally 2008-10-01) PLID 31541 - We use -1 in PersonT (poorly) for no location. I'm throwing in the null for robustness
							//This is also the only part of the report that makes sense to filter on No Location, but since
							//the figures in this report are all unrelated, we decided it is acceptible to for this one to contain numbers
							//and not the others.
							strLocPat.Format(" AND (PersonT.Location IS NULL OR PersonT.Location = -1)");
							//These will always be false
							strLocAppt.Format(" AND AppointmentsT.LocationID IS NULL");
							strLocAppt2.Format(" AND ApptsOuterQ.LocationID IS NULL");
							strLocPay.Format(" AND LineItemT.LocationID IS NULL");

						}
						else if(nLocation == -3){
							strLocPat.Format(" AND PersonT.Location IN");
							strLocAppt.Format(" AND AppointmentsT.LocationID IN");
							strLocAppt2.Format(" AND ApptsOuterQ.LocationID IN");
							strLocPay.Format(" AND LineItemT.LocationID IN");

							CString strPart;
							CString strIDs="";
							for(int i=0; i < m_dwLocations.GetSize(); i++) {
								strPart.Format("%li, ", (long)m_dwLocations.GetAt(i));
								strIDs += strPart;
								
							}
							strIDs = "("+ strIDs.Left(strIDs.GetLength()-2) + ")";
							strLocPat += strIDs;
							strLocAppt += strIDs;
							strLocAppt2 += strIDs;
							strLocPay += strIDs;

						}

						//provider
						CString strProvPat, strProvPay;
						if (nProvider > 0) {
							strProvPat.Format(" AND PatientsT.MainPhysician = %li", nProvider);
							strProvPay.Format(" AND PaymentsT.ProviderID = %li", nProvider);
							
						} else if(nProvider == -2) {
							strProvPat.Format(" AND PatientsT.MainPhysician IS NULL OR PatientsT.MainPhysician = -1");
							strProvPay.Format(" AND PaymentsT.ProviderID IS NULL OR PaymentsT.ProviderID = -1");
						}
						else if(nProvider == -3) {
							CString strAns;
							CString strPart;
							for(int i=0; i < m_dwProviders.GetSize(); i++) {
								strPart.Format("%li, ", (long)m_dwProviders.GetAt(i));
								strAns += strPart;
							}
							strProvPat.Format(" AND PatientsT.MainPhysician IN (%s)", strAns.Left(strAns.GetLength()-2));
							strProvPay.Format(" AND PaymentsT.ProviderID IN (%s)", strAns.Left(strAns.GetLength()-2));
						}

						//resource
						CString strResource;
						if (bExtended && saExtraValues.GetSize()) {
							CString strAns;
							CString strPart;
							for(int i=0; i < saExtraValues.GetSize(); i++) {
								strPart.Format("%s, ", saExtraValues[i]);
								strAns += strPart;
							}
							strResource.Format(" AND AppointmentResourceT.ResourceID IN (%s)", strAns.Left(strAns.GetLength()-2));
						}
						
						CString strTemp = strTemp1 + strTemp2;

						strTemp.Replace("[To]", strDateTo);
						strTemp.Replace("[From]", strDateFrom);
						strTemp.Replace("[LocPat]", strLocPat);
						strTemp.Replace("[LocAppt2]", strLocAppt2);
						strTemp.Replace("[LocAppt]", strLocAppt);
						strTemp.Replace("[LocPay]", strLocPay);						
						strTemp.Replace("[ProvPat]", strProvPat);
						strTemp.Replace("[ProvPay]", strProvPay);
						strTemp.Replace("[Resource]", strResource);							
						
						return strTemp;

						}

		case 656:	//Advertising Cost Analysis
			/* Version History
			// (c.haag 2009-01-22 16:53) - PLID 32188 - Created
			// (c.haag 2009-03-02 10:32) - PLID 33158 - We now include referrals that are not tied to any marketing costs at all.
				(d.thompson 2009-10-26) - PLID 35254 - This report used to display all referral sources for each interest, 
					but I have now changed it to only show the referral that is closest in time to the interest.
				(d.thompson 2009-11-18) - PLID 36332 - I removed the filtering on referral that controls what is included.
					This report should display all interests in the given date range.  No longer is it limited to
					just interests with referrals that have costs in the date range.

			*/
			{
				CString strSql = FormatString(
						"SELECT LadderID, FirstInterestDate AS Date, SubQ.PatID AS PatID, PatientLocation AS LocID, ProcedureID, ProcedureName, HasConsult, HasSurgery,  \r\n"
						"ReferralName, ReferralSourceID AS ReferralID, ReferralCosts \r\n"
						"FROM (  \r\n"
						"	SELECT LaddersT.ID AS LadderID, LaddersT.FirstInterestDate, ProcInfoT.PatientID AS PatID,   \r\n"
						"	ProcInfoDetailsT.ProcedureID, ProcedureT.Name AS ProcedureName,  \r\n"
						"	ReferralSourceT.Name AS ReferralName, ReferralSourceT.PersonID AS ReferralSourceID,  \r\n"
						"	@ReferralCosts AS ReferralCosts, PersonT.Location AS PatientLocation,  \r\n"
						"	/* Non-zero if a non-no-show consult exists for the ladder, or zero if otherwise */  \r\n"
						"	CASE WHEN EXISTS (  \r\n"
						"		SELECT AppointmentsT.ID FROM AppointmentsT  \r\n"
						"		INNER JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID  \r\n"
						"		INNER JOIN EventsT ON EventsT.ItemID = AppointmentsT.ID  \r\n"
						"		INNER JOIN EventAppliesT ON EventAppliesT.EventID = EventsT.ID  \r\n"
						"		INNER JOIN StepsT ON StepsT.ID = EventAppliesT.StepID  \r\n"
						"		WHERE StepsT.LadderID = LaddersT.ID  \r\n"
						"		AND EventsT.Type IN (4,6) /* Is event appointment-like */  \r\n"
						"		AND AptTypeT.Category = 1 /* Consult */  \r\n"
						"		AND AppointmentsT.Status <> 4 /* Cancelled */  \r\n"
						"		AND AppointmentsT.ShowState <> 3 /* No-show */  \r\n"
						"		AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Consult has matching procedure, or detail with matching master procedure */ \r\n"
						"	)  \r\n"
						"	THEN 1 ELSE 0 END AS HasConsult,   \r\n"
						"	/* Non-zero if a non-no-show surgery exists for the ladder, or zero if otherwise */ \r\n"
						"	CASE WHEN EXISTS ( \r\n"
						"		SELECT AppointmentsT.ID FROM AppointmentsT  \r\n"
						"		WHERE AppointmentsT.ID = ProcInfoT.SurgeryApptID /* Appointment is the surgery for the ladder. No need to check the appointment type category. */  \r\n"
						"		AND AppointmentsT.Status <> 4 /* Cancelled */  \r\n"
						"		AND AppointmentsT.ShowState <> 3 /* No-show */  \r\n"
						"		AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Surgery has matching procedure, or detail with matching master procedure */ \r\n"
						"	)  \r\n"
						"	THEN 1 ELSE 0 END AS HasSurgery \r\n"
						"	FROM LaddersT  \r\n"
						"	LEFT JOIN ProcInfoT ON ProcInfoT.ID = LaddersT.ProcInfoID  \r\n"
						"	LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID  \r\n"
						"	LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID  \r\n"
						"	LEFT JOIN PatientsT ON PatientsT.PersonID = ProcInfoT.PatientID  \r\n"
						"	LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID  \r\n"
						"	LEFT JOIN ReferralSourceT ON dbo.GetClosestReferralByDate(LaddersT.FirstInterestDate, ProcInfoT.PatientID) = ReferralSourceT.PersonID  \r\n"
						"	/* Must have existing referrals */ \r\n"
						"	WHERE ReferralSourceT.PersonID IS NOT NULL 	\r\n"
						"	/* Disallow ladders with no procedures. They don't make any real sense in this report */ \r\n"
						"	AND ProcedureT.ID IS NOT NULL \r\n"
						"	/* Include only master procedures, or else we get duplication in the results */  \r\n"
						"	AND ProcedureT.MasterProcedureID IS NULL \r\n"
						"	@MasterLocFilter \r\n"
						") SubQ \r\n"
					);

				COleDateTime dtFrom, dtTo;
				short nUseDateRange;

				switch(nSubLevel) {
				case 0: // Main report
					dtFrom = DateFrom;
					dtTo = DateTo;
					nUseDateRange = nDateRange;
					break;
				case 1: // YTD report. This runs from the first day of the current year through the end of today.
					dtFrom = COleDateTime::GetCurrentTime();
					dtTo.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
					dtTo += COleDateTimeSpan(1,0,0,0);
					dtFrom.SetDate(dtFrom.GetYear(),1,1);
					nUseDateRange = 1;
					break;
				default:
					ASSERT(FALSE); // This should never happen
					break;
				} // switch(nSubLevel) {

				if(nUseDateRange > 0){
					/* A date range was chosen. You'll need to follow the logic carefully on this:

					The user wants to run the report from DateFrom to DateTo. However, marketing costs
					themselves also have effective dates: we'll call those EffectiveFrom and EffectiveTo.
					So, how do we figure out the marketing costs with a date range? By factoring in the
					cost-per-day. For a given marketing cost line item, the cost-per-day equals:

						CPD = MarketingCostsT.Amount / (DATEDIFF(d, MarketingCostsT.EffectiveFrom, MarketingCostsT.EffectiveTo) + 1)

					Now, we need to consider the fact that the cost date range won't be the same as the
					report date range. The amount of days which overlap both the effective dates and report
					dates is:

						DAYS = DATEDIFF(d, MAX(MarketingCostsT.EffectiveFrom, DateFrom), MIN(MarketingCostsT.EffectiveTo, DateTo)) + 1

					of course, because I'm using MIN and MAX in the C++ way of thinking, this expands to

						DAYS = DATEDIFF(d, (CASE WHEN MarketingCostsT.EffectiveFrom > DateFrom THEN MarketingCostsT.EffectiveFrom ELSE DateFrom END),
											(CASE WHEN MarketingCostsT.EffectiveTo < DateTo THEN MarketingCostsT.EffectiveTo ELSE DateTo END)) + 1

					Putting it all together, the final value for the marketing costs of a referral in the main query is:

						COST = CPD * DAYS

					*/
					strSql.Replace("@ReferralCosts", 
						"(SELECT COALESCE(SUM( "
						"	CASE WHEN (MarketingCostsT.EffectiveFrom > '@DateTo' OR MarketingCostsT.EffectiveTo < '@DateFrom') THEN 0 ELSE "
						"		(MarketingCostsT.Amount / (DATEDIFF(d, MarketingCostsT.EffectiveFrom, MarketingCostsT.EffectiveTo) + 1)) * "
						"		(DATEDIFF(d, (CASE WHEN MarketingCostsT.EffectiveFrom > '@DateFrom' THEN MarketingCostsT.EffectiveFrom ELSE '@DateFrom' END), "
						"					(CASE WHEN MarketingCostsT.EffectiveTo < '@DateTo' THEN MarketingCostsT.EffectiveTo ELSE '@DateTo' END)) +1 ) "
						"	END "
						"),0) "
						"FROM MarketingCostsT "
						"WHERE MarketingCostsT.ReferralSource = ReferralSourceT.PersonID @ReferralWhereEx)");
					strSql.Replace("@MarketCostDateFilter", "WHERE NOT (EffectiveTo < '@DateFrom' OR EffectiveFrom > '@DateTo')");
					strSql.Replace("@DateFrom", FormatDateTimeForSql(dtFrom));
					strSql.Replace("@DateTo", FormatDateTimeForSql(dtTo));
				}
				else{
					// No date range chosen. Include all marketing costs for this referral for all time.
					strSql.Replace("@ReferralCosts", "(SELECT COALESCE(SUM(Amount),0) FROM MarketingCostsT WHERE MarketingCostsT.ReferralSource = ReferralSourceT.PersonID @ReferralWhereEx)");
					strSql.Replace("@MarketCostDateFilter", "");
				}

				// Filter on location
				// (j.luckoski 2013-03-26 16:14) - PLID 53755 - Filter on multiple locations if need be.
				strSql.Replace("@ReferralWhereEx", BuildAdvertisingCostLocationFilter(nLocation, (CDWordArray*)&m_dwLocations));
				strSql.Replace("@MasterLocFilter", BuildAdvertisingCostMasterLocationFilter(nLocation, (CDWordArray*)&m_dwLocations));

				return strSql;
			}
			break;

		case 657: // Media Cost Analysis
			/* Version History
			// (c.haag 2009-02-09 16:30) - PLID 32189 - Created
				(d.thompson 2009-10-26) - PLID 35254 - This report used to display all referral sources for each interest, 
					but I have now changed it to only show the referral that is closest in time to the interest.
				(d.thompson 2009-11-18) - PLID 36332 - I removed the filtering on referral that controls what is included.
					This report should display all interests in the given date range.  No longer is it limited to
					just interests with referrals that have costs in the date range.
				(d.thompson 2009-11-18) - PLID 36332 - Second rule change:  Media was previously defined as "any referral
					source".  It is not defined as "any referral source that has a cost (at any time in the past)".  I added
					an IsMedia flag to the subquery, and used that, instead of checking for the existence of a referral, 
					in the main query.  Also updated IsReferral and IsNonReferral to be more accurately named 
					IsMedia and IsNotMedia
				// (d.thompson 2009-11-18) - PLID 36332 - Renamed ReferralName to show as < No Referral > when NULL.
				// (d.thompson 2009-11-19) - PLID 36332 - Reworked the HasConsult and HasSurgery flags to not care if they're
				//	for media or non-media, we have a separate flag for that.
			*/
			{
				CString strSql = FormatString(
						"SELECT LadderID, FirstInterestDate AS Date, SubQ.PatID AS PatID, PatientLocation AS LocID, ProcedureID AS ProcID, ProcedureName,  "
						"HasConsult, "
						"HasSurgery, "
						"CASE WHEN ReferralSourceID IS NULL THEN '< No Referral >' ELSE ReferralName END AS ReferralName, "
						"ReferralSourceID AS ReferralID, ReferralCosts, "
						"IsMedia, "
						"CASE WHEN IsMedia = 0 THEN 1 ELSE 0 END AS IsNonMedia "
						"\r\n"
						"FROM (  \r\n"
						"	SELECT LaddersT.ID AS LadderID, LaddersT.FirstInterestDate, ProcInfoT.PatientID AS PatID,   \r\n"
						"	ProcInfoDetailsT.ProcedureID, ProcedureT.Name AS ProcedureName,  \r\n"
						"	ReferralSourceT.Name AS ReferralName, ReferralSourceT.PersonID AS ReferralSourceID,  \r\n"
						"	@ReferralCosts AS ReferralCosts, PersonT.Location AS PatientLocation,  \r\n"
						"	/* Non-zero if a non-no-show consult exists for the ladder, or zero if otherwise */  \r\n"
						"	CASE WHEN EXISTS (  \r\n"
						"		SELECT AppointmentsT.ID FROM AppointmentsT  \r\n"
						"		INNER JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID  \r\n"
						"		INNER JOIN EventsT ON EventsT.ItemID = AppointmentsT.ID  \r\n"
						"		INNER JOIN EventAppliesT ON EventAppliesT.EventID = EventsT.ID  \r\n"
						"		INNER JOIN StepsT ON StepsT.ID = EventAppliesT.StepID  \r\n"
						"		WHERE StepsT.LadderID = LaddersT.ID  \r\n"
						"		AND EventsT.Type IN (4,6) /* Is event appointment-like */  \r\n"
						"		AND AptTypeT.Category = 1 /* Consult */  \r\n"
						"		AND AppointmentsT.Status <> 4 /* Cancelled */  \r\n"
						"		AND AppointmentsT.ShowState <> 3 /* No-show */  \r\n"
						"		AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Consult has matching procedure, or detail with matching master procedure */ \r\n"
						"	)  \r\n"
						"	THEN 1 ELSE 0 END AS HasConsult,   \r\n"
						"	/* Non-zero if a non-no-show surgery exists for the ladder, or zero if otherwise */ \r\n"
						"	CASE WHEN EXISTS ( \r\n"
						"		SELECT AppointmentsT.ID FROM AppointmentsT  \r\n"
						"		WHERE AppointmentsT.ID = ProcInfoT.SurgeryApptID /* Appointment is the surgery for the ladder. No need to check the appointment type category. */  \r\n"
						"		AND AppointmentsT.Status <> 4 /* Cancelled */  \r\n"
						"		AND AppointmentsT.ShowState <> 3 /* No-show */  \r\n"
						"		AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Surgery has matching procedure, or detail with matching master procedure */ \r\n"
						"	)  \r\n"
						"	THEN 1 ELSE 0 END AS HasSurgery, \r\n"
						"	CASE 	WHEN ReferralSourceT.PersonID IS NULL THEN 0 \r\n"
						"		WHEN ReferralSourceT.PersonID IN (SELECT ReferralSource FROM MarketingCostsT) THEN 1 \r\n"
						"		ELSE 0 END AS IsMedia \r\n"
						"	FROM LaddersT  \r\n"
						"	LEFT JOIN ProcInfoT ON ProcInfoT.ID = LaddersT.ProcInfoID  \r\n"
						"	LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID  \r\n"
						"	LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID  \r\n"
						"	LEFT JOIN PatientsT ON PatientsT.PersonID = ProcInfoT.PatientID  \r\n"
						"	LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID  \r\n"
						"	LEFT JOIN ReferralSourceT ON dbo.GetClosestReferralByDate(LaddersT.FirstInterestDate, ProcInfoT.PatientID) = ReferralSourceT.PersonID \r\n"
						"	/* Master where clause */ \r\n"
						"	WHERE \r\n"
						"	/* Disallow ladders with no procedures. They don't make any real sense in this report */ \r\n"
						"	ProcedureT.ID IS NOT NULL \r\n"
						"	/* Include only master procedures, or else we get duplication in the results */  \r\n"
						"	AND ProcedureT.MasterProcedureID IS NULL \r\n"
						"	@MasterLocFilter \r\n"
						") SubQ \r\n"
					);

				COleDateTime dtFrom, dtTo;
				short nUseDateRange;

				switch(nSubLevel) {
				case 0: // Main report
					dtFrom = DateFrom;
					dtTo = DateTo;
					nUseDateRange = nDateRange;
					break;
				case 1: // YTD report. This runs from the first day of the current year through the end of today.
					dtFrom = COleDateTime::GetCurrentTime();
					dtTo.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
					dtTo += COleDateTimeSpan(1,0,0,0);
					dtFrom.SetDate(dtFrom.GetYear(),1,1);
					nUseDateRange = 1;
					break;
				default:
					ASSERT(FALSE); // This should never happen
					break;
				} // switch(nSubLevel) {

				if(nUseDateRange > 0){
					/* A date range was chosen. You'll need to follow the logic carefully on this:

					The user wants to run the report from DateFrom to DateTo. However, marketing costs
					themselves also have effective dates: we'll call those EffectiveFrom and EffectiveTo.
					So, how do we figure out the marketing costs with a date range? By factoring in the
					cost-per-day. For a given marketing cost line item, the cost-per-day equals:

						CPD = MarketingCostsT.Amount / (DATEDIFF(d, MarketingCostsT.EffectiveFrom, MarketingCostsT.EffectiveTo) + 1)

					Now, we need to consider the fact that the cost date range won't be the same as the
					report date range. The amount of days which overlap both the effective dates and report
					dates is:

						DAYS = DATEDIFF(d, MAX(MarketingCostsT.EffectiveFrom, DateFrom), MIN(MarketingCostsT.EffectiveTo, DateTo)) + 1

					of course, because I'm using MIN and MAX in the C++ way of thinking, this expands to

						DAYS = DATEDIFF(d, (CASE WHEN MarketingCostsT.EffectiveFrom > DateFrom THEN MarketingCostsT.EffectiveFrom ELSE DateFrom END),
											(CASE WHEN MarketingCostsT.EffectiveTo < DateTo THEN MarketingCostsT.EffectiveTo ELSE DateTo END)) + 1

					Putting it all together, the final value for the marketing costs of a referral in the main query is:

						COST = CPD * DAYS


					.....and now we're half way there.....

					Up to now, we have included all marketing costs for the given referral source. Now we must factor in the procedures for each marketing cost. 
					Because the granularity of each record is one procedure big, we can simply test whether the marketing cost included that procedure, or if the
					marketing cost is not associated with any procedure (in which case we treat it like it's assigned to all procedures). You can see how we do this
					in the WHERE clause below.

					*/
					strSql.Replace("@ReferralCosts", 
						"("
						"SELECT COALESCE(SUM( "
						"	CASE WHEN (MarketingCostsT.EffectiveFrom > '@DateTo' OR MarketingCostsT.EffectiveTo < '@DateFrom') THEN 0 ELSE "
						"		(MarketingCostsT.Amount / (DATEDIFF(d, MarketingCostsT.EffectiveFrom, MarketingCostsT.EffectiveTo) + 1)) * "
						"		(DATEDIFF(d, (CASE WHEN MarketingCostsT.EffectiveFrom > '@DateFrom' THEN MarketingCostsT.EffectiveFrom ELSE '@DateFrom' END), "
						"					(CASE WHEN MarketingCostsT.EffectiveTo < '@DateTo' THEN MarketingCostsT.EffectiveTo ELSE '@DateTo' END)) +1 ) "
						"	END "
						"),0) "
						"FROM MarketingCostsT "
						"WHERE MarketingCostsT.ReferralSource = ReferralSourceT.PersonID "
						"AND ( "
						"        (MarketingCostsT.ID IN (SELECT MarketingCostID FROM MarketingCostProcedureT WHERE MarketingCostProcedureT.ProcedureID = ProcInfoDetailsT.ProcedureID)) "
						"        OR "
						"        (MarketingCostsT.ID NOT IN (SELECT MarketingCostID FROM MarketingCostProcedureT)) "
						"    ) "
						"@ReferralWhereEx "
						")");
					strSql.Replace("@MarketCostDateFilter", "AND NOT (EffectiveTo < '@DateFrom' OR EffectiveFrom > '@DateTo')");
					strSql.Replace("@DateFrom", FormatDateTimeForSql(dtFrom));
					strSql.Replace("@DateTo", FormatDateTimeForSql(dtTo));
				}
				else{
					// No date range chosen. Include all marketing costs for this referral for all time. We must factor in the procedures for each marketing cost. 
					// Because the granularity of each record is one procedure big, we can simply test whether the marketing cost included that procedure, or if the
					// marketing cost is not associated with any procedure (in which case we treat it like it's assigned to all procedures). You can see how we do this
					// in the WHERE clause below.
					strSql.Replace("@ReferralCosts", 
						"(SELECT COALESCE(SUM(Amount),0) FROM MarketingCostsT "
						"WHERE MarketingCostsT.ReferralSource = ReferralSourceT.PersonID "
						"AND ( "
						"        (MarketingCostsT.ID IN (SELECT MarketingCostID FROM MarketingCostProcedureT WHERE MarketingCostProcedureT.ProcedureID = ProcInfoDetailsT.ProcedureID)) "
						"        OR "
						"        (MarketingCostsT.ID NOT IN (SELECT MarketingCostID FROM MarketingCostProcedureT)) "
						"    ) "
						"@ReferralWhereEx "
						")");
					strSql.Replace("@MarketCostDateFilter", "");
				}

				// Filter on location
				if (nLocation > 0) {
					strSql.Replace("@ReferralWhereEx", FormatString("AND MarketingCostsT.LocationID = %d", nLocation));
					strSql.Replace("@MasterLocFilter", FormatString("AND ReferralSourceT.PersonID IN (SELECT ReferralSource FROM MarketingCostsT M WHERE M.LocationID = %d)", nLocation));
				} else {
					// No location filter chosen. Do not do any kind of location filtering. It's perfectly fine even if
					// the patient's location is not the same as the referral's location.
					strSql.Replace("@ReferralWhereEx", "");
					strSql.Replace("@MasterLocFilter", "");
				}

				return strSql;
			}
			break;

			case 674:	//Interest to Consult to Procedure by Referral Source
			case 675:	//Interest to Consult to Procedure by Patient Coordinator
				/*	Version History
					(d.thompson 2009-08-27) - PLID 35068 - Created.
					(d.thompson 2009-08-27) - PLID 35068 - Also created a by Patient Coordinator version at the same time.  Same
						query is used, the report is just grouped differently.
					(d.thompson 2009-10-23) - PLID 35683 - Changed how referral source is joined.  Instead of always choosing the primary
						referral, it will now detect the closest referral source to the interest date, and use that referral source.
				*/
				return _T("SELECT  \n"
					"/*Patient related data*/ \n"
					"ProcInfoT.PatientID AS PatID, PatientsT.UserDefinedID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, \n"
					"ReferralSourceT.PersonID AS ReferralSourceID, \n"
					"CASE WHEN ReferralSourceT.PersonID IS NULL THEN '<No Referral>' ELSE ReferralSourceT.Name END AS ReferralName, \n"
					"PersonT.Location AS LocID, LocationsT.Name AS PersonLocationName, PatientsT.EmployeeID AS PatCoordID, \n"
					"CASE WHEN PatientsT.EmployeeID IS NULL THEN '<No Coordinator>' ELSE CoordPersonT.Last + ', ' + CoordPersonT.First + ' ' + CoordPersonT.Middle END AS PatCoordName, \n"
					"PatientsT.MainPhysician AS ProvID, ProvPersonT.Last + ', ' + ProvPersonT.First + ' ' + ProvPersonT.Middle AS PatientProviderName, \n"
					" \n"
					"/*PIC/Ladder related data*/ \n"
					"LaddersT.FirstInterestDate AS FirstInterestDate, ProcInfoDetailsT.ProcedureID AS ProcedureID, ProcedureT.Name AS ProcedureName, \n"
					"ProcInfoT.SurgeonID AS PICSurgeonID, SurgeonPersonT.Last + ', ' + SurgeonPersonT.First + ' ' + SurgeonPersonT.Middle AS PICSurgeonName, \n"
					"ProcInfoT.CoSurgeonID AS PICCoSurgeonID, CoSurgeonPersonT.Last + ', ' + CoSurgeonPersonT.First + ' ' + CoSurgeonPersonT.Middle AS PICCoSurgeonName, \n"
					"NursePersonT.Last + ', ' + NursePersonT.First + ' ' + NursePersonT.Middle AS PICNurse,  \n"
					"AnesthesiologistPersonT.Last + ', ' + AnesthesiologistPersonT.First + ' ' + AnesthesiologistPersonT.Middle AS PICAnesthesiologist, \n"
					"(SELECT TOP 1 LadderTemplateID FROM StepsT LEFT JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE LadderID = LaddersT.ID) AS LadderTemplateID, \n"
					" \n"
					"/*Consult related data*/ \n"
					"CASE WHEN ConsultSubQ.ApptID IS NULL THEN 0 ELSE 1 END AS HasConsult, \n"
					"ConsultSubQ.Date AS ConsultDate, ConsultSubQ.LocationID AS ConsultLocationID, ConsultSubQ.ConsultLocationName, \n"
					"ConsultSubQ.AptTypeID AS ConsultTypeID, ConsultSubQ.ConsultTypeName, \n"
					" \n"
					"/*Surgery related data*/ \n"
					"CASE WHEN SurgerySubQ.ApptID IS NULL THEN 0 ELSE 1 END AS HasSurgery, \n"
					"SurgerySubQ.Date AS SurgeryDate, SurgerySubQ.LocationID AS SurgeryLocationID, SurgerySubQ.SurgeryLocationName, \n"
					"SurgerySubQ.AptTypeID AS SurgeryTypeID, SurgerySubQ.SurgeryTypeName \n"
					" \n"
					"FROM LaddersT   \n"
					"LEFT JOIN ProcInfoT ON ProcInfoT.ID = LaddersT.ProcInfoID   \n"
					"LEFT JOIN PersonT SurgeonPersonT ON ProcInfoT.SurgeonID = SurgeonPersonT.ID \n"
					"LEFT JOIN PersonT CoSurgeonPersonT ON ProcInfoT.CoSurgeonID = CoSurgeonPersonT.ID \n"
					"LEFT JOIN PersonT NursePersonT ON ProcInfoT.NurseID = NursePersonT.ID \n"
					"LEFT JOIN PersonT AnesthesiologistPersonT ON ProcInfoT.AnesthesiologistID = AnesthesiologistPersonT.ID \n"
					"LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID   \n"
					"LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID   \n"
					"LEFT JOIN PatientsT ON PatientsT.PersonID = ProcInfoT.PatientID   \n"
					"LEFT JOIN PersonT CoordPersonT ON PatientsT.EmployeeID = CoordPersonT.ID \n"
					"LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID   \n"
					"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID \n"
					"LEFT JOIN PersonT ProvPersonT ON PatientsT.MainPhysician = ProvPersonT.ID \n"
					"LEFT JOIN ReferralSourceT ON dbo.GetClosestReferralByDate(LaddersT.FirstInterestDate, ProcInfoT.PatientID) = ReferralSourceT.PersonID \n"
					"LEFT JOIN \n"
					"	/*The consult matched here must be tied to the PIC and must match the procedure (or be a detail of) of the PIC exactly*/ \n"
					"	(SELECT AppointmentsT.ID AS ApptID, StepsT.LadderID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS MasterID, \n"
					"	AppointmentsT.Date, AppointmentsT.LocationID, LocationsT.Name AS ConsultLocationName, \n"
					"	AppointmentsT.AptTypeID, AptTypeT.Name AS ConsultTypeName \n"
					"	FROM AppointmentsT   \n"
					"	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID \n"
					"	INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID \n"
					"	INNER JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID   \n"
					"	INNER JOIN EventsT ON EventsT.ItemID = AppointmentsT.ID   \n"
					"	INNER JOIN EventAppliesT ON EventAppliesT.EventID = EventsT.ID   \n"
					"	INNER JOIN StepsT ON StepsT.ID = EventAppliesT.StepID   \n"
					"	LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID \n"
					"	WHERE EventsT.Type IN (4,6) /* Is event appointment-like */   \n"
					"	AND AptTypeT.Category = 1 /* Consult */   \n"
					"	AND AppointmentsT.Status <> 4 /* Cancelled */   \n"
					"	AND AppointmentsT.ShowState <> 3 /* No-show */   \n"
					"	GROUP BY AppointmentsT.ID, StepsT.LadderID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID), \n"
					"	AppointmentsT.Date, AppointmentsT.LocationID, LocationsT.Name, \n"
					"	AppointmentsT.AptTypeID, AptTypeT.Name \n"
					"	) ConsultSubQ \n"
					"ON LaddersT.ID = ConsultSubQ.LadderID AND COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) = ConsultSubQ.MasterID \n"
					"LEFT JOIN \n"
					"	/*The surgery matched here must be tied to the PIC and match the procedure (or be a detail of) the PIC exactly*/  \n"
					"	(SELECT AppointmentsT.ID AS ApptID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) AS MasterID, \n"
					"	AppointmentsT.Date, AppointmentsT.LocationID, LocationsT.Name AS SurgeryLocationName, \n"
					"	AppointmentsT.AptTypeID, AptTypeT.Name AS SurgeryTypeName \n"
					"	FROM AppointmentsT   \n"
					"	LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID \n"
					"	INNER JOIN ProcedureT ON AppointmentPurposeT.PurposeID = ProcedureT.ID \n"
					"	LEFT JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID   \n"
					"	LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID \n"
					"	WHERE AppointmentsT.Status <> 4 /* Cancelled */   \n"
					"	AND AppointmentsT.ShowState <> 3 /* No-show */   \n"
					"	GROUP BY AppointmentsT.ID, COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID),  \n"
					"	AppointmentsT.Date, AppointmentsT.LocationID, LocationsT.Name,   \n"
					"	AppointmentsT.AptTypeID, AptTypeT.Name   \n"
					"	) SurgerySubQ \n"
					"ON ProcInfoT.SurgeryApptID = SurgerySubQ.ApptID AND COALESCE(ProcedureT.MasterProcedureID, ProcedureT.ID) = SurgerySubQ.MasterID \n"
					"/* Master where clause */  \n"
					"WHERE  \n"
					"/* Disallow ladders with no procedures. They don't make any real sense in this report */  \n"
					"ProcedureT.ID IS NOT NULL  \n"
					"/* Include only master procedures, or else we get duplication in the results */   \n"
					"AND ProcedureT.MasterProcedureID IS NULL \n"
					"/*Filter out inquiries*/ \n"
					"AND PatientsT.CurrentStatus <> 4 \n");
				break;

		case 686:	//Patients By Media By Primary Insurance Financial Class

			// (j.jones 2009-10-29 16:21) - PLID 35793 - created
			// (j.gruber 2010-07-16 09:45) - PLID 38075 - added costs
			// (c.haag 2010-10-25 16:31) - PLID 41024 - Added two fields:
			// CrossTabCost: This is the total number of patients for a referral for a primary financial class 
			//		divided by the total marketing costs for the referral.
			// CrossTabTotal: This is the total marketing referral costs for the referral.
			// The way I came to the formulas is described below.
			{

				CString strSql;
				COleDateTime dtFrom, dtTo;

				if (nDateRange == -1) {
					dtFrom.SetDate(1800,01,01);
					dtTo.SetDate(5000,12,31);
				}
				else {				
					dtFrom = DateFrom;
					COleDateTimeSpan day(1,0,0,0);
					dtTo = DateTo + day;
				}


			strSql.Format(
			//This is the same base query from the Referrals report, that will iterate pseudo-recursively through the referral tree,
			//filling out a table variable with the parents	of each referral up to 4 levels deep.
			"SET NOCOUNT ON\r\n"
			" DECLARE @dtFrom datetime; "
			" SET @dtFrom = convert(datetime, '%s'); "
			" DECLARE @dtTo datetime; "
			" SET @dtTo = convert(datetime, '%s'); "
			"DECLARE @ReferralTree TABLE (ReferralID INT NOT NULL, Level1ID INT NOT NULL, "
			"Level1Name nvarchar(50) NOT NULL, Level2ID INT, Level2Name nvarchar(50), Level3ID INT, "
			"Level3Name nvarchar(50), Level4ID INT, Level4Name nvarchar(50) ) "
			" "
			"INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name) "
			"    SELECT PersonID, PersonID, Name FROM ReferralSourceT WHERE Parent = -1;\r\n"
			""
			/* (c.haag 2010-10-25 16:31) - PLID 41024 - This table variable allows us to calculate the "Cost Per Class"
			in the crosstab object of the summary report. Each row in this variable is a unique combination of referral and
			primary financial class. The Patient Count is the number of patients that have that primary referral and a primary
			insurance company that puts them in that financial class. It's easier to understand if you run this in NxQuery and
			just select from this table variable. */
			"DECLARE @ReferralGroups TABLE ( "
			"	RefID INT, "
			"	PrimaryFinancialClassName NVARCHAR(255), "
			"	PatientCount INT "
			")\r\n"
			"INSERT INTO @ReferralGroups (RefID, PrimaryFinancialClassName, PatientCount) "
			"SELECT RefID, PrimaryFinancialClassName, Sum(PatientCount) AS PatientCount FROM ( "
			"SELECT R.PersonID AS RefID, "
			"CASE WHEN F.Name Is Null THEN  "
			"	CASE WHEN IC.Name Is Null THEN '< No Insurance >'  "
			"	ELSE '< No Financial Class >'  "
			"	END  "
			"ELSE F.Name  "
			"END AS PrimaryFinancialClassName, "
			"Count(COALESCE(PatientsT.PersonID,0)) AS PatientCount "
			"FROM ReferralSourceT R "
			"LEFT JOIN PatientsT ON PatientsT.ReferralID = R.PersonID "
			"LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 1) IP ON PatientsT.PersonID = IP.PatientID  "
			"LEFT JOIN InsuranceCoT IC ON IP.InsuranceCoID = IC.PersonID  "
			"LEFT JOIN FinancialClassT F ON IC.FinancialClassID = F.ID  "
			/* The filter for @ReferralGroups must be identical to the report's base filter. This is updated in FinalizeReportSqlQuery */
			"#REPFILTERPLACEHOLDER "
			"GROUP BY R.PersonID, F.Name, IC.Name "
			") SubQ "
			"GROUP BY RefID, PrimaryFinancialClassName\r\n"
			" "

			" "
			"DECLARE @Level1ID INT\r\n"
			"DECLARE @Level1Name nvarchar(50)\r\n"
			"DECLARE Level1Cursor CURSOR STATIC OPTIMISTIC FOR "
			"SELECT Level1ID, Level1Name FROM @ReferralTree "
			"OPEN Level1Cursor  "
			"FETCH NEXT FROM Level1Cursor INTO @Level1ID, @Level1Name "
			"WHILE @@FETCH_STATUS = 0 BEGIN "
			"    INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name, Level2ID, Level2Name) "
			"        SELECT PersonID, @Level1ID, @Level1Name, PersonID, Name FROM ReferralSourceT WHERE Parent = @Level1ID; "
			" "
			"    DECLARE @Level2ID INT "
			"    DECLARE @Level2Name nvarchar(50) "
			"    DECLARE Level2Cursor CURSOR STATIC OPTIMISTIC FOR "
			"    SELECT Level2ID, Level2Name FROM @ReferralTree WHERE Level1ID = @Level1ID "
			"    OPEN Level2Cursor "
			"    FETCH NEXT FROM Level2Cursor INTO @Level2ID, @Level2Name "
			"    WHILE @@FETCH_STATUS = 0 BEGIN "
			"        INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name, Level2ID, Level2Name, Level3ID, Level3Name) "
			"            SELECT PersonID, @Level1ID, @Level1Name, @Level2ID, @Level2Name, PersonID, Name FROM ReferralSourceT WHERE Parent = @Level2ID; "
			" "
			"        DECLARE @Level3ID INT "
			"        DECLARE @Level3Name nvarchar(50) "
			"        DECLARE Level3Cursor CURSOR STATIC OPTIMISTIC FOR "
			"        SELECT Level3ID, Level3Name FROM @ReferralTree WHERE Level2ID = @Level2ID "
			"        OPEN Level3Cursor "
			"        FETCH NEXT FROM Level3Cursor INTO @Level3ID, @Level3Name "
			"        WHILE @@FETCH_STATUS = 0 BEGIN "
			"            INSERT INTO @ReferralTree (ReferralID, Level1ID, Level1Name, Level2ID, Level2Name, Level3ID, Level3Name, Level4ID, Level4Name) "
			"                SELECT PersonID, @Level1ID, @Level1Name, @Level2ID, @Level2Name, @Level3ID, @Level3Name, PersonID, Name FROM ReferralSourceT WHERE Parent = @Level3ID; "
			"         "
			"            FETCH NEXT FROM Level3Cursor INTO @Level3ID, @Level3Name "
			"        END "
			"        CLOSE Level3Cursor "
			"        DEALLOCATE Level3Cursor "
			"         "
			"        FETCH NEXT FROM Level2Cursor INTO @Level2ID, @Level2Name "
			"    END "
			"    CLOSE Level2Cursor "
			"    DEALLOCATE Level2Cursor "
			"     "
			" "
			"    FETCH NEXT FROM Level1Cursor INTO @Level1ID, @Level1Name "
			"END "
			"CLOSE Level1Cursor "
			"DEALLOCATE Level1Cursor\r\n"
			" "
			"SET NOCOUNT OFF\r\n"
			" "
			//And now we run our actual query, pulling from the table variable we just filled to give us the ancestors of each referral.
			"SELECT 1 AS GroupID, Coalesce(RefTree.Level1Name, '< No Referral >') AS Level1Name, RefTree.Level2Name, RefTree.Level3Name, RefTree.Level4Name, "
			"PersonT.Last + ', ' + PersonT.Middle + ' ' + PersonT.First AS PatName, "
			"CASE WHEN ReferralSourceT.PersonID Is Null THEN '< No Referral >' ELSE ReferralSourceT.Name END AS ReferralName, "
			"PersonT.FirstContactDate AS Date, PersonT.Location AS LocID, LocationsT.Name AS Location, "
			"PersonT.ID AS PatID, PatientsT.MainPhysician AS ProvID, "
			"PatientsT.ReferralID AS ReferralID, "
			"(SELECT TOP 1 Date FROM AppointmentsT WHERE AppointmentsT.PatientID = PatientsT.PersonID AND Status <> 4 ORDER BY Date ASC) AS FirstApptDate, "
			"UsersT.Username AS InputtingUser, "
			"CASE WHEN PrimaryInsuranceCoT.Name Is Null THEN '< No Insurance >' ELSE PrimaryInsuranceCoT.Name END AS PrimaryInsuranceCoName, "
			"CASE WHEN SecondaryInsuranceCoT.Name Is Null THEN '< No Insurance >' ELSE SecondaryInsuranceCoT.Name END AS SecondaryInsuranceCoName, "
			"CASE WHEN PrimaryFinancialClassT.Name Is Null THEN "
			"	CASE WHEN PrimaryInsuranceCoT.Name Is Null THEN '< No Insurance >' ELSE '< No Financial Class >' END "
			"ELSE PrimaryFinancialClassT.Name END AS PrimaryFinancialClassName, "
			"CASE WHEN SecondaryFinancialClassT.Name Is Null THEN "
			"	CASE WHEN SecondaryInsuranceCoT.Name Is Null THEN '< No Insurance >' ELSE '< No Financial Class >' END "
			"ELSE SecondaryFinancialClassT.Name END AS SecondaryFinancialClassName, ReferralCostsQ.Cost "
			/* (c.haag 2010-10-25 16:31) - PLID 41024 - Calculate the "Cost Per Class" to be used in a cell of the crosstab object in the
			summary report. In the detailed report, the Cost Per Class field is calculated as TotalReferralCost / Count(PatientID).
			In the summary report, each cell in the crosstab contains Count(PatientID); that is, the count of the number of records
			in the report query that fit the referral-class combination. To get the cost per class, we have to calculate a number such
			that:
			N * Count(PatientID) = Cost Per Class
			and by the definition of "Cost Per Class", we know that
			N * Count(PatientID) = TotalReferralCost / Count(PatientID)
			ergo
			N = TotalReferralCost / (Count(PatientID)*Count(PatientID))
			*/
			",COALESCE(Cost,convert(money,0)) / (R.PatientCount*R.PatientCount) AS CrossTabCost "
			/* (c.haag 2010-10-25 16:31) - PLID 41024 - The crosstab total column must be the total Cost of the referral. So,
			we do something a little more simple here: Because each record is one patient, we simply do 
			TotalReferralCost / Count(PatientID with the referral).
			*/
			",COALESCE(Cost,convert(money,0)) / (SELECT Sum(PatientCount) FROM @ReferralGroups S WHERE S.RefID = R.RefID) AS CrossTabTotal "

			"FROM PersonT\r\n "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID\r\n "
			"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID\r\n "
			"LEFT JOIN @ReferralTree AS RefTree ON ReferralSourceT.PersonID = RefTree.ReferralID\r\n "
			"LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID\r\n "
			"LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID\r\n "
			"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 1) AS PrimaryInsuredPartyT ON PatientsT.PersonID = PrimaryInsuredPartyT.PatientID\r\n "
			"LEFT JOIN (SELECT PatientID, InsuranceCoID FROM InsuredPartyT WHERE RespTypeID = 2) AS SecondaryInsuredPartyT ON PatientsT.PersonID = SecondaryInsuredPartyT.PatientID\r\n "
			"LEFT JOIN InsuranceCoT PrimaryInsuranceCoT ON PrimaryInsuredPartyT.InsuranceCoID = PrimaryInsuranceCoT.PersonID\r\n "
			"LEFT JOIN InsuranceCoT SecondaryInsuranceCoT ON SecondaryInsuredPartyT.InsuranceCoID = SecondaryInsuranceCoT.PersonID\r\n "
			"LEFT JOIN FinancialClassT PrimaryFinancialClassT ON PrimaryInsuranceCoT.FinancialClassID = PrimaryFinancialClassT.ID\r\n "
			"LEFT JOIN FinancialClassT SecondaryFinancialClassT ON SecondaryInsuranceCoT.FinancialClassID = SecondaryFinancialClassT.ID\r\n "
			" LEFT JOIN\r\n "
			"(SELECT ReferralSource AS ReferralID,\r\n"
			"CAST (SUM (CostSubQ.Cost) AS FLOAT) AS Cost\r\n"
			"FROM\r\n"		
		
			"(\r\n"
			"SELECT Cost1+Cost2+Cost3+Cost4 AS Cost, ReferralSource\r\n"
			"FROM (\r\n"
			"  SELECT\r\n"
			"    (CASE WHEN (EffectiveFrom >= @dtFrom AND EffectiveFrom < @dtTo AND EffectiveTo >= @dtFrom AND EffectiveTo < @dtTo)\r\n"
			"	  THEN (Amount) ELSE 0 END) AS Cost1,\r\n"
			"    (CASE WHEN (EffectiveFrom < @dtTo AND EffectiveFrom > @dtFrom AND EffectiveTo >= @dtTo)\r\n"
			"	  THEN (Amount * DATEDIFF(day, @dtTo, EffectiveFrom)) / DATEDIFF(day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost2,\r\n"
			"    (CASE WHEN (EffectiveFrom <= @dtFrom AND EffectiveTo >= @dtTo AND (EffectiveFrom <> @dtFrom OR EffectiveTo <> @dtTo))\r\n"
			"	  THEN (Amount * DATEDIFF(day, @dtTo, @dtFrom)) / DATEDIFF(day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost3,\r\n"
			"    (CASE WHEN (EffectiveFrom < @dtFrom AND EffectiveTo > @dtFrom AND EffectiveTo < @dtTo)\r\n"
			"	  THEN (Amount * DATEDIFF(day, EffectiveTo, @dtFrom)) / DATEDIFF (day, EffectiveTo, EffectiveFrom) ELSE 0 END) AS Cost4,\r\n"
			"    ReferralSource\r\n"
			"  FROM MarketingCostsT\r\n"	
			") RangedCostsQ\r\n"
			") AS CostSubQ\r\n"
			"GROUP BY ReferralSource\r\n "
			" ) ReferralCostsQ ON RefTree.ReferralID = ReferralCostsQ.ReferralID\r\n "
			/* (c.haag 2010-10-25 16:31) - PLID 41024 - Left join on @ReferralGroups for summary calculations */
			"LEFT JOIN @ReferralGroups R ON (R.RefID = PatientsT.ReferralID AND R.PrimaryFinancialClassName = "
			"(CASE WHEN PrimaryFinancialClassT.Name Is Null THEN "
			"	CASE WHEN PrimaryInsuranceCoT.Name Is Null THEN '< No Insurance >' ELSE '< No Financial Class >' END "
			"ELSE PrimaryFinancialClassT.Name END))\r\n"
			/* The filter for @ReferralGroups must be identical to the report's base filter. This is updated in FinalizeReportSqlQuery */
			"#BEGINREPORTFILTER"
			"WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID <> -25 "
			,FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));

			return strSql;

			}
			break;

		case 687: // Weekly Advertising Analysis
			/* Version History
				(d.thompson 2009-11-19) - PLID 32190 - Created.  Start of query came from the Marketing Cost Analysis.
				(d.thompson 2012-07-10) - PLID 51456 - Now that the PIC supports multiple bills, updated the revenue
					calculation to be the revenue for all bills tied to the PIC.
				(d.thompson 2012-09-13) - PLID 52122 - Changed the MarketingCostInRangeNoProcedure function to accept an xml
					document of location IDs instead of only a single ID.  This allows the report to work on multiple location
					filters.
			*/
			{
				CString strTableVars =
					//Part 1.  The goal of this report is to have a display record for each week.  To do that, we have to have
					//	some kind of data for each week.  We may not have interests for each week, and the costs just give us
					//	a beginning and ending date.  Therefore, we need to generate a table of all dates in the date range of
					//	the report.
					"SET NOCOUNT ON; \n"
					"DECLARE @NumbersBeginDate datetime; \n"
					"SET @NumbersBeginDate = @BeginDate@; \n"
					"DECLARE @Numbers table (Number int identity) \n"
					"DECLARE @i int; \n"
					"SET @i = 1; \n"
					"WHILE @i <= datediff(d, @NumbersBeginDate, @EndDate@) + 1 \n"
					"BEGIN  \n"
					"	INSERT @Numbers DEFAULT VALUES  \n"
					"	SET @i = @i + 1; \n"
					"END \n"
					"DECLARE @Dates table (Date datetime); \n"
				//TODO - top one is every date, bottom is every sunday only
					//"INSERT INTO @Dates (Date) (SELECT DATEADD(d, Number-1, @BeginDate@) from @Numbers) \n"
					"INSERT INTO @Dates (Date) (SELECT dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DATEADD(d, Number-1, @NumbersBeginDate)) - 1), DATEADD(d, Number-1, @NumbersBeginDate))) from @Numbers GROUP BY dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DATEADD(d, Number-1, @NumbersBeginDate)) - 1), DATEADD(d, Number-1, @NumbersBeginDate)))); \n"
					"SET NOCOUNT OFF; \n"
					"\n\n";
				//If we are filtering on a date range, the dates are easy to pick out.  However if we're not, we need to
				//	limit the number of dates in the table (or the report will run really slowly).  In testing, I could fill
				//	about 10 years of dates this way in 3-4 seconds, which is reasonable for a report.  1 year of data was 
				//	nearly instantaneous.
				if(nDateRange > 0) {
					strTableVars.Replace("@BeginDate@", "'" + FormatDateTimeForSql(DateFrom) + "'");
					strTableVars.Replace("@EndDate@", "'" + FormatDateTimeForSql(DateTo) + "'");
				}
				else {
					//In the attempt for "all dates", the report will display all weeks between the first ever
					//	marketing cost From date and the last ever marketing cost To date.  There may be no-cost weeks in between, 
					//	but we're guaranteed to have no cost data outside these weeks.
					strTableVars.Replace("@BeginDate@", "(SELECT MIN(EffectiveFrom) FROM MarketingCostsT)");
					strTableVars.Replace("@EndDate@", "(SELECT MAX(EffectiveTo) FROM MarketingCostsT)");
				}


				CString strSql = strTableVars + FormatString(
					"SELECT DateTableQ.Date AS FilterDate,  \n"
					"/*This finds the sunday previous to the first interest date.  This function operates on the 'SET DATEFIRST' sql property.  By default this is \n"
					"	Sunday in the US, but it can be changed.*/ \n"
					"dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date)) AS PreviousSunday, \n"
					"/*Add 6 to get the following Saturday*/ \n"
					"DATEADD(d, 6,  (dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date)))) AS FollowingSaturday, \n"
					"CASE WHEN LadderID IS NULL THEN 0 ELSE 1 END AS HasInterest, \n"
					"LadderID, FirstInterestDate, SubQ.PatientID, PatientLocation AS LocID,  \n"
					"ProcedureID AS ProcID, ProcedureName,   \n"
					"COALESCE(HasConsult, 0) AS HasConsult,  \n"
					"COALESCE(HasSurgery, 0) AS HasSurgery,  \n"
					"CASE WHEN DateTableQ.ReferralSourceID IS NULL THEN '< No Referral >' ELSE DateTableQ.ReferralName END AS ReferralName, \n"
					"DateTableQ.ReferralSourceID AS ReferralID, \n"
					"COALESCE(dbo.MarketingCostInRangeNoProcedure( \n"
					"(dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date)))/*Sunday*/, \n"
					"	DATEADD(d, 6,  (dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date))))/*Saturday*/, \n"
					"	DateTableQ.ReferralSourceID, \n"
					"	@LocationFilter@), 0) AS AllReferralCostsThisWeek, \n"
					"convert(money, COALESCE((SELECT SUM(AppliesT.Amount) \n"
					"FROM LaddersT LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID \n"
					"LEFT JOIN ProcInfoBillsT ON ProcInfoT.ID = ProcInfoBillsT.ProcInfoID "
					"LEFT JOIN BillsT ON ProcInfoBillsT.BillID = BillsT.ID \n"
					"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID \n"
					"LEFT JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID \n"
					"LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID \n"
					"LEFT JOIN LineItemT LinePays ON AppliesT.SourceID = LinePays.ID \n"
					"WHERE BillsT.Deleted = 0 AND LineCharges.Deleted = 0 AND LinePays.Deleted = 0 AND LinePays.Type = 1 AND LaddersT.ID = SubQ.LadderID \n"
					"GROUP BY LaddersT.ID), 0)) AS RevenueForSurg \n"
					"\r\n"
					"FROM 	\n"
					"	(SELECT DateTable.Date, ReferralSourceT.PersonID AS ReferralSourceID, ReferralSourceT.Name AS ReferralName \n"
					"	FROM @Dates DateTable, (SELECT PersonID, Name FROM ReferralSourceT UNION SELECT NULL, '< No Referral >') ReferralSourceT \n"
					"	) DateTableQ \n"
					"	LEFT JOIN \n"
					"	(  \r\n"
					"		SELECT LaddersT.ID AS LadderID, LaddersT.FirstInterestDate, ProcInfoT.PatientID,   \r\n"
					"		dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, LaddersT.FirstInterestDate) - 1), LaddersT.FirstInterestDate)) AS IntPreviousSunday, \n"
					"		ProcInfoDetailsT.ProcedureID, ProcedureT.Name AS ProcedureName,  \r\n"
					"		ReferralSourceT.Name AS ReferralName, ReferralSourceT.PersonID AS ReferralSourceID,  \r\n"
					"		PersonT.Location AS PatientLocation,  \r\n"
					"		/* Non-zero if a non-no-show consult exists for the ladder, or zero if otherwise */  \r\n"
					"		CASE WHEN EXISTS (  \r\n"
					"			SELECT AppointmentsT.ID FROM AppointmentsT  \r\n"
					"			INNER JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID  \r\n"
					"			INNER JOIN EventsT ON EventsT.ItemID = AppointmentsT.ID  \r\n"
					"			INNER JOIN EventAppliesT ON EventAppliesT.EventID = EventsT.ID  \r\n"
					"			INNER JOIN StepsT ON StepsT.ID = EventAppliesT.StepID  \r\n"
					"			WHERE StepsT.LadderID = LaddersT.ID  \r\n"
					"			AND EventsT.Type IN (4,6) /* Is event appointment-like */  \r\n"
					"			AND AptTypeT.Category = 1 /* Consult */  \r\n"
					"			AND AppointmentsT.Status <> 4 /* Cancelled */  \r\n"
					"			AND AppointmentsT.ShowState <> 3 /* No-show */  \r\n"
					"			AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Consult has matching procedure, or detail with matching master procedure */ \r\n"
					"		)  \r\n"
					"		THEN 1 ELSE 0 END AS HasConsult,   \r\n"
					"		/* Non-zero if a non-no-show surgery exists for the ladder, or zero if otherwise */ \r\n"
					"		CASE WHEN EXISTS ( \r\n"
					"			SELECT AppointmentsT.ID FROM AppointmentsT  \r\n"
					"			WHERE AppointmentsT.ID = ProcInfoT.SurgeryApptID /* Appointment is the surgery for the ladder. No need to check the appointment type category. */  \r\n"
					"			AND AppointmentsT.Status <> 4 /* Cancelled */  \r\n"
					"			AND AppointmentsT.ShowState <> 3 /* No-show */  \r\n"
					"			AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Surgery has matching procedure, or detail with matching master procedure */ \r\n"
					"		)  \r\n"
					"		THEN 1 ELSE 0 END AS HasSurgery \r\n"
					"		FROM LaddersT  \r\n"
					"		LEFT JOIN ProcInfoT ON ProcInfoT.ID = LaddersT.ProcInfoID  \r\n"
					"		LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID  \r\n"
					"		LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID  \r\n"
					"		LEFT JOIN PatientsT ON PatientsT.PersonID = ProcInfoT.PatientID  \r\n"
					"		LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID  \r\n"
					"		LEFT JOIN ReferralSourceT ON dbo.GetClosestReferralByDate(LaddersT.FirstInterestDate, ProcInfoT.PatientID) = ReferralSourceT.PersonID \r\n"
					"		/* Master where clause */ \r\n"
					"		WHERE \r\n"
					"		/* Disallow ladders with no procedures. They don't make any real sense in this report */ \r\n"
					"		ProcedureT.ID IS NOT NULL \r\n"
					"		/* Include only master procedures, or else we get duplication in the results */  \r\n"
					"		AND ProcedureT.MasterProcedureID IS NULL \r\n"
					"		@DateOptimization@ \n"
					"		@PatientLocFilter@ \n"
					"	) SubQ \r\n"
					"ON DateTableQ.Date = SubQ.IntPreviousSunday AND DateTableQ.ReferralSourceID = SubQ.ReferralSourceID; \n"
					/*Aside:  This query must end in a semicolon for TTX purposes.  Because there is no WHERE, GROUP BY, etc, it does a reverse
						lookup on the semicolon, which does exist in the table variable queries at the top*/
				);

				//Optimization.  Because we're filtering on Date, but joining the SubQ on "PreviousSunday" (calculated data), SQL 
				//	server isn't able to optimize the SubQ to only look at relevant records, instead it has to query the entire
				//	database.  To improve the speed, we'll apply a date filter manually to the SubQ.
				if(nDateRange > 0) {
					strSql.Replace("@DateOptimization@", 
						"AND LaddersT.FirstInterestDate >= '" + FormatDateTimeForSql(DateFrom) + 
						"' AND LaddersT.FirstInterestDate < DATEADD(d, 1, '" + FormatDateTimeForSql(DateTo) + "')");
				}
				else {
					strSql.Replace("@DateOptimization@", "");
				}

				// Filter on location
				strSql.Replace("@LocationFilter@", BuildWeeklyAdvertisingLocationFilter(nLocation, (CDWordArray*)&m_dwLocations));
				strSql.Replace("@PatientLocFilter@", BuildWeeklyAdvertisingPatientLocationFilter(nLocation, (CDWordArray*)&m_dwLocations));

				return strSql;
			}
			break;

		case 725: // Media Cost Analysis By Top Level Referral
			/* Version History
				(r.gonet 02/24/2012) - PLID 47646 - Created. Split off of the Media Cost Analysis report. This report is identical to the Media Cost Analysis report except it
					retrieves the top level referral source for each row (as well as the actual referral source). The intent is to have the report group on the top level 
					referral sources the actual referral sources fall under instead of the actual referral sources. Not really a summary of Media Cost Analysis, so I made a new
					report.
			*/
			{
				CString strSql = FormatString(
					"SET NOCOUNT ON; \r\n"
					"/* This is kind of a hack table that will contain all of the referral sources with HasChildren = 0 and then the referral sources which are parents with HasChildren = 1. */\r\n"
					"/* Its purpose is simply to let us retain a parent referral source as a separate row when we left join on its children in the ReferralSourceSubQ */ \r\n"
					"DECLARE @ReferralSourcesVar TABLE \r\n"
					"( \r\n"
					"	PersonID INT NOT NULL, \r\n"
					"	Name NVARCHAR(255) NOT NULL, \r\n"
					"	Parent INT NOT NULL, \r\n"
					"	HasChildren BIT NOT NULL \r\n"
					") \r\n"
					"INSERT INTO @ReferralSourcesVar (PersonId, Name, Parent, HasChildren) \r\n"
					"( \r\n"
					"	SELECT PersonID, Name, Parent, 1 AS HasChildren \r\n"
					"	FROM ReferralSourceT \r\n"
					"	WHERE ReferralSourceT.PersonID IN \r\n"
					"	( \r\n"
					"		SELECT A.Parent \r\n"
					"		FROM ReferralSourceT A \r\n"
					"	) \r\n"
					"	 \r\n"
					"	UNION \r\n"
					"	 \r\n"
					"	SELECT PersonID, Name, Parent, 0 AS HasChildren \r\n"
					"	FROM ReferralSourceT \r\n"
					") \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT LadderID, FirstInterestDate AS Date, SubQ.PatID AS PatID, PatientLocation AS LocID, ProcedureID AS ProcID, ProcedureName, HasConsult, HasSurgery, CASE WHEN ReferralSourceID IS NULL THEN '< No Referral >' ELSE ReferralName END AS ReferralName, ReferralSourceID AS ReferralID, CASE WHEN TopLevelReferralID IS NULL THEN '<No Referral>' ELSE TopLevelReferralName END AS TopLevelReferralName, TopLevelReferralID, ReferralCosts, IsMedia, CASE WHEN IsMedia = 0 THEN 1 ELSE 0 END AS IsNonMedia \r\n"
					"FROM ( \r\n"
					"	SELECT LaddersT.ID AS LadderID, LaddersT.FirstInterestDate, ProcInfoT.PatientID AS PatID, \r\n"
					"	ProcInfoDetailsT.ProcedureID, ProcedureT.Name AS ProcedureName, \r\n"
					"	ReferralSourceSubQ.Name AS ReferralName, ReferralSourceSubQ.PersonID AS ReferralSourceID, ReferralSourceSubQ.RootID AS ToplevelReferralID, ReferralSourceSubQ.RootName AS TopLevelReferralName, \r\n"
					"	@ReferralCosts AS ReferralCosts, PersonT.Location AS PatientLocation, \r\n"
					"	/* Non-zero if a non-no-show consult exists for the ladder, or zero if otherwise */ \r\n"
					"	CASE WHEN EXISTS ( \r\n"
					"		SELECT AppointmentsT.ID FROM AppointmentsT \r\n"
					"		INNER JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID \r\n"
					"		INNER JOIN EventsT ON EventsT.ItemID = AppointmentsT.ID \r\n"
					"		INNER JOIN EventAppliesT ON EventAppliesT.EventID = EventsT.ID \r\n"
					"		INNER JOIN StepsT ON StepsT.ID = EventAppliesT.StepID \r\n"
					"		WHERE StepsT.LadderID = LaddersT.ID \r\n"
					"		AND EventsT.Type IN (4,6) /* Is event appointment-like */ \r\n"
					"		AND AptTypeT.Category = 1 /* Consult */ \r\n"
					"		AND AppointmentsT.Status <> 4 /* Cancelled */ \r\n"
					"		AND AppointmentsT.ShowState <> 3 /* No-show */ \r\n"
					"		AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Consult has matching procedure, or detail with matching master procedure */ \r\n"
					"	) \r\n"
					"	THEN 1 ELSE 0 END AS HasConsult, \r\n"
					"	/* Non-zero if a non-no-show surgery exists for the ladder, or zero if otherwise */ \r\n"
					"	CASE WHEN EXISTS ( \r\n"
					"		SELECT AppointmentsT.ID FROM AppointmentsT \r\n"
					"		WHERE AppointmentsT.ID = ProcInfoT.SurgeryApptID /* Appointment is the surgery for the ladder. No need to check the appointment type category. */ \r\n"
					"		AND AppointmentsT.Status <> 4 /* Cancelled */ \r\n"
					"		AND AppointmentsT.ShowState <> 3 /* No-show */ \r\n"
					"		AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Surgery has matching procedure, or detail with matching master procedure */ \r\n"
					"	) \r\n"
					"	THEN 1 ELSE 0 END AS HasSurgery, \r\n"
					"	CASE 	WHEN ReferralSourceSubQ.PersonID IS NULL THEN 0 \r\n"
					"		WHEN ReferralSourceSubQ.PersonID IN (SELECT ReferralSource FROM MarketingCostsT) THEN 1 \r\n"
					"		ELSE 0 END AS IsMedia \r\n"
					"	FROM LaddersT \r\n"
					"	LEFT JOIN ProcInfoT ON ProcInfoT.ID = LaddersT.ProcInfoID \r\n"
					"	LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID \r\n"
					"	LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID \r\n"
					"	LEFT JOIN PatientsT ON PatientsT.PersonID = ProcInfoT.PatientID \r\n"
					"	LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID \r\n"
					"	LEFT JOIN ( \r\n"
					"		/* There is a maximum of four referral levels. Trees stored like this are always a pain to work with. Tree paths like file paths would have been much better. */ \r\n"
					"		SELECT \r\n"
					"			CASE WHEN ReferralSourceT4.PersonID IS NULL THEN \r\n"
					"				CASE WHEN ReferralSourceT3.PersonID IS NULL THEN \r\n"
					"					CASE WHEN ReferralSourceT2.PersonID IS NULL THEN \r\n"
					"						ReferralSourceT1.PersonID \r\n"
					"					ELSE \r\n"
					"						ReferralSourceT2.PersonID \r\n"
					"					END \r\n"
					"				ELSE \r\n"
					"					ReferralSourceT3.PersonID \r\n"
					"				END \r\n"
					"			ELSE \r\n"
					"				ReferralSourceT4.PersonID \r\n"
					"			END AS PersonID, \r\n"
					"			CASE WHEN ReferralSourceT4.PersonID IS NULL THEN \r\n"
					"				CASE WHEN ReferralSourceT3.PersonID IS NULL THEN \r\n"
					"					CASE WHEN ReferralSourceT2.PersonID IS NULL THEN \r\n"
					"						ReferralSourceT1.Name \r\n"
					"					ELSE \r\n"
					"						ReferralSourceT2.Name \r\n"
					"					END \r\n"
					"				ELSE \r\n"
					"					ReferralSourceT3.Name \r\n"
					"				END \r\n"
					"			ELSE \r\n"
					"				ReferralSourceT4.Name \r\n"
					"			END AS Name, \r\n"
					"			ReferralSourceT1.PersonID AS RootID, \r\n"
					"			ReferralSourceT1.Name AS RootName \r\n"
					"		FROM @ReferralSourcesVar ReferralSourceT1 \r\n"
					"			/* HasChildren condition, a hack, because otherwise we \"lose\" the separate row for the parent referral source */ \r\n"
					"			LEFT JOIN @ReferralSourcesVar ReferralSourceT2 ON ReferralSourceT1.PersonID = ReferralSourceT2.Parent AND ReferralSourceT1.HasChildren = 1 \r\n"
					"			LEFT JOIN @ReferralSourcesVar ReferralSourceT3 ON ReferralSourceT2.PersonID = ReferralSourceT3.Parent AND ReferralSourceT2.HasChildren = 1 \r\n"
					"			LEFT JOIN @ReferralSourcesVar ReferralSourceT4 ON ReferralSourceT3.PersonID = ReferralSourceT4.Parent AND ReferralSourceT3.HasChildren = 1 \r\n"
					"		WHERE ReferralSourceT1.Parent = -1 /* Otherwise we can get non top level referral sources in ReferralSourceT1 */\r\n"
					"		) ReferralSourceSubQ ON dbo.GetClosestReferralByDate(LaddersT.FirstInterestDate, ProcInfoT.PatientID) = ReferralSourceSubQ.PersonID \r\n"
					"	/* Master where clause */ \r\n"
					"	WHERE \r\n"
					"	/* Disallow ladders with no procedures. They don't make any real sense in this report */ \r\n"
					"	ProcedureT.ID IS NOT NULL \r\n"
					"	/* Include only master procedures, or else we get duplication in the results */ \r\n"
					"	AND ProcedureT.MasterProcedureID IS NULL \r\n"
					"	@MasterLocFilter\r\n"
					") SubQ WHERE (1=1) \r\n");


				COleDateTime dtFrom, dtTo;
				short nUseDateRange;

				switch(nSubLevel) {
				case 0: // Main report
					dtFrom = DateFrom;
					dtTo = DateTo;
					nUseDateRange = nDateRange;
					break;
				case 1: // YTD report. This runs from the first day of the current year through the end of today.
					dtFrom = COleDateTime::GetCurrentTime();
					dtTo.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
					dtTo += COleDateTimeSpan(1,0,0,0);
					dtFrom.SetDate(dtFrom.GetYear(),1,1);
					nUseDateRange = 1;
					break;
				default:
					ASSERT(FALSE); // This should never happen
					break;
				} // switch(nSubLevel) {

				if(nUseDateRange > 0){
					/* A date range was chosen. You'll need to follow the logic carefully on this:

					The user wants to run the report from DateFrom to DateTo. However, marketing costs
					themselves also have effective dates: we'll call those EffectiveFrom and EffectiveTo.
					So, how do we figure out the marketing costs with a date range? By factoring in the
					cost-per-day. For a given marketing cost line item, the cost-per-day equals:

						CPD = MarketingCostsT.Amount / (DATEDIFF(d, MarketingCostsT.EffectiveFrom, MarketingCostsT.EffectiveTo) + 1)

					Now, we need to consider the fact that the cost date range won't be the same as the
					report date range. The amount of days which overlap both the effective dates and report
					dates is:

						DAYS = DATEDIFF(d, MAX(MarketingCostsT.EffectiveFrom, DateFrom), MIN(MarketingCostsT.EffectiveTo, DateTo)) + 1

					of course, because I'm using MIN and MAX in the C++ way of thinking, this expands to

						DAYS = DATEDIFF(d, (CASE WHEN MarketingCostsT.EffectiveFrom > DateFrom THEN MarketingCostsT.EffectiveFrom ELSE DateFrom END),
											(CASE WHEN MarketingCostsT.EffectiveTo < DateTo THEN MarketingCostsT.EffectiveTo ELSE DateTo END)) + 1

					Putting it all together, the final value for the marketing costs of a referral in the main query is:

						COST = CPD * DAYS


					.....and now we're half way there.....

					Up to now, we have included all marketing costs for the given referral source. Now we must factor in the procedures for each marketing cost. 
					Because the granularity of each record is one procedure big, we can simply test whether the marketing cost included that procedure, or if the
					marketing cost is not associated with any procedure (in which case we treat it like it's assigned to all procedures). You can see how we do this
					in the WHERE clause below.

					*/
					strSql.Replace("@ReferralCosts", 
						"("
						"SELECT COALESCE(SUM( "
						"	CASE WHEN (MarketingCostsT.EffectiveFrom > '@DateTo' OR MarketingCostsT.EffectiveTo < '@DateFrom') THEN 0 ELSE "
						"		(MarketingCostsT.Amount / (DATEDIFF(d, MarketingCostsT.EffectiveFrom, MarketingCostsT.EffectiveTo) + 1)) * "
						"		(DATEDIFF(d, (CASE WHEN MarketingCostsT.EffectiveFrom > '@DateFrom' THEN MarketingCostsT.EffectiveFrom ELSE '@DateFrom' END), "
						"					(CASE WHEN MarketingCostsT.EffectiveTo < '@DateTo' THEN MarketingCostsT.EffectiveTo ELSE '@DateTo' END)) +1 ) "
						"	END "
						"),0) "
						"FROM MarketingCostsT "
						"WHERE MarketingCostsT.ReferralSource = ReferralSourceSubQ.PersonID "
						"AND ( "
						"        (MarketingCostsT.ID IN (SELECT MarketingCostID FROM MarketingCostProcedureT WHERE MarketingCostProcedureT.ProcedureID = ProcInfoDetailsT.ProcedureID)) "
						"        OR "
						"        (MarketingCostsT.ID NOT IN (SELECT MarketingCostID FROM MarketingCostProcedureT)) "
						"    ) "
						"@ReferralWhereEx "
						")");
					strSql.Replace("@MarketCostDateFilter", "AND NOT (EffectiveTo < '@DateFrom' OR EffectiveFrom > '@DateTo')");
					strSql.Replace("@DateFrom", FormatDateTimeForSql(dtFrom));
					strSql.Replace("@DateTo", FormatDateTimeForSql(dtTo));
				}
				else{
					// No date range chosen. Include all marketing costs for this referral for all time. We must factor in the procedures for each marketing cost. 
					// Because the granularity of each record is one procedure big, we can simply test whether the marketing cost included that procedure, or if the
					// marketing cost is not associated with any procedure (in which case we treat it like it's assigned to all procedures). You can see how we do this
					// in the WHERE clause below.
					strSql.Replace("@ReferralCosts", 
						"(SELECT COALESCE(SUM(Amount),0) FROM MarketingCostsT "
						"WHERE MarketingCostsT.ReferralSource = ReferralSourceSubQ.PersonID "
						"AND ( "
						"        (MarketingCostsT.ID IN (SELECT MarketingCostID FROM MarketingCostProcedureT WHERE MarketingCostProcedureT.ProcedureID = ProcInfoDetailsT.ProcedureID)) "
						"        OR "
						"        (MarketingCostsT.ID NOT IN (SELECT MarketingCostID FROM MarketingCostProcedureT)) "
						"    ) "
						"@ReferralWhereEx "
						")");
					strSql.Replace("@MarketCostDateFilter", "");
				}

				// Filter on location
				if (nLocation > 0) {
					strSql.Replace("@ReferralWhereEx", FormatString("AND MarketingCostsT.LocationID = %d", nLocation));
					strSql.Replace("@MasterLocFilter", FormatString("AND ReferralSourceSubQ.PersonID IN (SELECT ReferralSource FROM MarketingCostsT M WHERE M.LocationID = %d)", nLocation));
				} else {
					// No location filter chosen. Do not do any kind of location filtering. It's perfectly fine even if
					// the patient's location is not the same as the referral's location.
					strSql.Replace("@ReferralWhereEx", "");
					strSql.Replace("@MasterLocFilter", "");
				}

				return strSql;
			}
			break;

		case 738: // Weekly Advertising Analysis By Top Level Referral
			/* Version History
				(r.gonet 06/11/2012) - PLID 47647 - Created.  Query came from combining Weekly Adertising Analysis with Media Cost Analysis by Top Level Referral.
				(d.thompson 2012-07-10) - PLID 51456 - Now that the PIC supports multiple bills, updated the revenue
					calculation to be the revenue for all bills tied to the PIC.
				(d.thompson 2012-09-13) - PLID 52122 - Changed the MarketingCostInRangeNoProcedure function to accept an xml
					document of location IDs instead of only a single ID.  This allows the report to work on multiple location
					filters.
			*/
			{
				CString strReferralSourceTableInit = 
					"SET NOCOUNT ON; \n"
					"/* This is kind of a hack table that will contain all of the referral sources with HasChildren = 0 and then the referral sources which are parents with HasChildren = 1. */ \n"
					"/* Its purpose is simply to let us retain a parent referral source as a separate row when we left join on its children in the ReferralSourceSubQ */ \n"
					"DECLARE @TempReferralSourceT TABLE \n"
					"( \n"
					"	PersonID INT NOT NULL, \n"
					"	Name NVARCHAR(255) NOT NULL, \n"
					"	Parent INT NOT NULL, \n"
					"	HasChildren BIT NOT NULL \n"
					") \n"
					"INSERT INTO @TempReferralSourceT (PersonId, Name, Parent, HasChildren) \n"
					"( \n"
					"	SELECT PersonID, Name, Parent, 1 AS HasChildren \n"
					"	FROM ReferralSourceT \n"
					"	WHERE ReferralSourceT.PersonID IN \n"
					"	( \n"
					"		SELECT A.Parent \n"
					"		FROM ReferralSourceT A \n"
					"	) \n"
					"	  \n"
					"	UNION \n"
					"	  \n"
					"	SELECT PersonID, Name, Parent, 0 AS HasChildren \n"
					"	FROM ReferralSourceT \n"
					") \n"
					"SET NOCOUNT OFF;\n"
					"\n\n";

				CString strTableVars =
					//Part 1.  The goal of this report is to have a display record for each week.  To do that, we have to have
					//	some kind of data for each week.  We may not have interests for each week, and the costs just give us
					//	a beginning and ending date.  Therefore, we need to generate a table of all dates in the date range of
					//	the report.
					"SET NOCOUNT ON; \n"
					"DECLARE @NumbersBeginDate datetime; \n"
					"SET @NumbersBeginDate = @BeginDate@; \n"
					"DECLARE @Numbers table (Number int identity) \n"
					"DECLARE @i int; \n"
					"SET @i = 1; \n"
					"WHILE @i <= datediff(d, @NumbersBeginDate, @EndDate@) + 1 \n"
					"BEGIN  \n"
					"	INSERT @Numbers DEFAULT VALUES  \n"
					"	SET @i = @i + 1; \n"
					"END \n"
					"DECLARE @Dates table (Date datetime); \n"
				//TODO - top one is every date, bottom is every sunday only
					//"INSERT INTO @Dates (Date) (SELECT DATEADD(d, Number-1, @BeginDate@) from @Numbers) \n"
					"INSERT INTO @Dates (Date) (SELECT dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DATEADD(d, Number-1, @NumbersBeginDate)) - 1), DATEADD(d, Number-1, @NumbersBeginDate))) from @Numbers GROUP BY dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DATEADD(d, Number-1, @NumbersBeginDate)) - 1), DATEADD(d, Number-1, @NumbersBeginDate)))); \n"
					"SET NOCOUNT OFF; \n"
					"\n\n";
				//If we are filtering on a date range, the dates are easy to pick out.  However if we're not, we need to
				//	limit the number of dates in the table (or the report will run really slowly).  In testing, I could fill
				//	about 10 years of dates this way in 3-4 seconds, which is reasonable for a report.  1 year of data was 
				//	nearly instantaneous.
				if(nDateRange > 0) {
					strTableVars.Replace("@BeginDate@", "'" + FormatDateTimeForSql(DateFrom) + "'");
					strTableVars.Replace("@EndDate@", "'" + FormatDateTimeForSql(DateTo) + "'");
				}
				else {
					//In the attempt for "all dates", the report will display all weeks between the first ever
					//	marketing cost From date and the last ever marketing cost To date.  There may be no-cost weeks in between, 
					//	but we're guaranteed to have no cost data outside these weeks.
					strTableVars.Replace("@BeginDate@", "(SELECT MIN(EffectiveFrom) FROM MarketingCostsT)");
					strTableVars.Replace("@EndDate@", "(SELECT MAX(EffectiveTo) FROM MarketingCostsT)");
				}


				CString strSql = strReferralSourceTableInit + strTableVars + FormatString(
					"SELECT DateTableQ.Date AS FilterDate, \n"
					"/*This finds the sunday previous to the first interest date.  This function operates on the 'SET DATEFIRST' sql property.  By default this is \n"
					"	Sunday in the US, but it can be changed.*/ \n"
					"dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date)) AS PreviousSunday, \n"
					"/*Add 6 to get the following Saturday*/ \n"
					"DATEADD(d, 6,  (dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date)))) AS FollowingSaturday, \n"
					"CASE WHEN LadderID IS NULL THEN 0 ELSE 1 END AS HasInterest, \n"
					"LadderID, FirstInterestDate, SubQ.PatientID, PatientLocation AS LocID, \n"
					"ProcedureID AS ProcID, ProcedureName, \n"
					"COALESCE(HasConsult, 0) AS HasConsult, \n"
					"COALESCE(HasSurgery, 0) AS HasSurgery, \n"
					"CASE WHEN DateTableQ.ReferralSourceID IS NULL THEN '< No Referral >' ELSE DateTableQ.ReferralName END AS ReferralName, \n"
					"DateTableQ.ReferralSourceID AS ReferralID, \n"
					"CASE WHEN ReferralSourceSubQ.RootID IS NULL THEN '<No Referral>' ELSE ReferralSourceSubQ.RootName END AS TopLevelReferralName, \n"
					"ReferralSourceSubQ.RootID AS TopLevelReferralID, \n"
					"COALESCE(dbo.MarketingCostInRangeNoProcedure( \n"
					"(dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date)))/*Sunday*/, \n"
					"	DATEADD(d, 6,  (dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, DateTableQ.Date) - 1), DateTableQ.Date))))/*Saturday*/, \n"
					"	DateTableQ.ReferralSourceID, \n"
					"	@LocationFilter@), 0) AS AllReferralCostsThisWeek, \n"
					"convert(money, COALESCE((SELECT SUM(AppliesT.Amount) \n"
					"FROM LaddersT LEFT JOIN ProcInfoT ON LaddersT.ProcInfoID = ProcInfoT.ID \n"
					"LEFT JOIN ProcInfoBillsT ON ProcInfoT.ID = ProcInfoBillsT.ProcInfoID "
					"LEFT JOIN BillsT ON ProcInfoBillsT.BillID = BillsT.ID \n"
					"LEFT JOIN ChargesT ON BillsT.ID = ChargesT.BillID \n"
					"LEFT JOIN LineItemT LineCharges ON ChargesT.ID = LineCharges.ID \n"
					"LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID \n"
					"LEFT JOIN LineItemT LinePays ON AppliesT.SourceID = LinePays.ID \n"
					"WHERE BillsT.Deleted = 0 AND LineCharges.Deleted = 0 AND LinePays.Deleted = 0 AND LinePays.Type = 1 AND LaddersT.ID = SubQ.LadderID \n"
					"GROUP BY LaddersT.ID), 0)) AS RevenueForSurg \n"
					"FROM 	 \n"
					"	(SELECT DateTable.Date, ReferralSourceT.PersonID AS ReferralSourceID, ReferralSourceT.Name AS ReferralName \n"
					"	FROM @Dates DateTable, (SELECT PersonID, Name FROM ReferralSourceT UNION SELECT NULL, '< No Referral >') ReferralSourceT \n"
					"	) DateTableQ \n"
					"	LEFT JOIN \n"
					"	( \n"
					"		SELECT LaddersT.ID AS LadderID, LaddersT.FirstInterestDate, ProcInfoT.PatientID, \n"
					"		dbo.AsDateNoTime(dateadd(d, -1 * (datepart(dw, LaddersT.FirstInterestDate) - 1), LaddersT.FirstInterestDate)) AS IntPreviousSunday, \n"
					"		ProcInfoDetailsT.ProcedureID, ProcedureT.Name AS ProcedureName, \n"
					"		ReferralSourceT.Name AS ReferralName, ReferralSourceT.PersonID AS ReferralSourceID,-- ReferralSourceSubQ.RootID AS ToplevelReferralID, ReferralSourceSubQ.RootName AS TopLevelReferralName, \n"
					"		PersonT.Location AS PatientLocation, \n"
					"		/* Non-zero if a non-no-show consult exists for the ladder, or zero if otherwise */ \n"
					"		CASE WHEN EXISTS ( \n"
					"			SELECT AppointmentsT.ID FROM AppointmentsT \n"
					"			INNER JOIN AptTypeT ON AptTypeT.ID = AppointmentsT.AptTypeID \n"
					"			INNER JOIN EventsT ON EventsT.ItemID = AppointmentsT.ID \n"
					"			INNER JOIN EventAppliesT ON EventAppliesT.EventID = EventsT.ID \n"
					"			INNER JOIN StepsT ON StepsT.ID = EventAppliesT.StepID \n"
					"			WHERE StepsT.LadderID = LaddersT.ID \n"
					"			AND EventsT.Type IN (4,6) /* Is event appointment-like */ \n"
					"			AND AptTypeT.Category = 1 /* Consult */ \n"
					"			AND AppointmentsT.Status <> 4 /* Cancelled */ \n"
					"			AND AppointmentsT.ShowState <> 3 /* No-show */ \n"
					"			AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Consult has matching procedure, or detail with matching master procedure */ \n"
					"		) \n"
					"		THEN 1 ELSE 0 END AS HasConsult, \n"
					"		/* Non-zero if a non-no-show surgery exists for the ladder, or zero if otherwise */ \n"
					"		CASE WHEN EXISTS ( \n"
					"			SELECT AppointmentsT.ID FROM AppointmentsT \n"
					"			WHERE AppointmentsT.ID = ProcInfoT.SurgeryApptID /* Appointment is the surgery for the ladder. No need to check the appointment type category. */ \n"
					"			AND AppointmentsT.Status <> 4 /* Cancelled */ \n"
					"			AND AppointmentsT.ShowState <> 3 /* No-show */ \n"
					"			AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT INNER JOIN ProcedureT P ON P.ID = AppointmentPurposeT.PurposeID WHERE P.ID = ProcedureT.ID OR P.MasterProcedureID = ProcedureT.ID) /* Surgery has matching procedure, or detail with matching master procedure */ \n"
					"		) \n"
					"		THEN 1 ELSE 0 END AS HasSurgery \n"
					"		FROM LaddersT \n"
					"		LEFT JOIN ProcInfoT ON ProcInfoT.ID = LaddersT.ProcInfoID \n"
					"		LEFT JOIN ProcInfoDetailsT ON ProcInfoDetailsT.ProcInfoID = ProcInfoT.ID \n"
					"		LEFT JOIN ProcedureT ON ProcedureT.ID = ProcInfoDetailsT.ProcedureID \n"
					"		LEFT JOIN PatientsT ON PatientsT.PersonID = ProcInfoT.PatientID \n"
					"		LEFT JOIN PersonT ON PersonT.ID = PatientsT.PersonID \n"
					"		LEFT JOIN ReferralSourceT ON dbo.GetClosestReferralByDate(LaddersT.FirstInterestDate, ProcInfoT.PatientID) = ReferralSourceT.PersonID \n"
					"		/* Master where clause */ \n"
					"		WHERE \n"
					"		/* Disallow ladders with no procedures. They don't make any real sense in this report */ \n"
					"		ProcedureT.ID IS NOT NULL \n"
					"		/* Include only master procedures, or else we get duplication in the results */ \n"
					"		AND ProcedureT.MasterProcedureID IS NULL \n"
					"		@DateOptimization@ \n"
					"		@PatientLocFilter@ \n"
					"	) SubQ \n"
					"ON DateTableQ.Date = SubQ.IntPreviousSunday AND DateTableQ.ReferralSourceID = SubQ.ReferralSourceID \n"
					"LEFT JOIN \n"
					"( \n"
					"	/* There is a maximum of four referral levels. Trees stored like this are always a pain to work with. Tree paths like file paths would have been much better. */ \n"
					"	SELECT \n"
					"		CASE WHEN ReferralSourceT4.PersonID IS NULL THEN \n"
					"			CASE WHEN ReferralSourceT3.PersonID IS NULL THEN \n"
					"				CASE WHEN ReferralSourceT2.PersonID IS NULL THEN \n"
					"					ReferralSourceT1.PersonID \n"
					"				ELSE \n"
					"					ReferralSourceT2.PersonID \n"
					"				END \n"
					"			ELSE \n"
					"				ReferralSourceT3.PersonID \n"
					"			END \n"
					"		ELSE \n"
					"			ReferralSourceT4.PersonID \n"
					"		END AS PersonID, \n"
					"		CASE WHEN ReferralSourceT4.PersonID IS NULL THEN \n"
					"			CASE WHEN ReferralSourceT3.PersonID IS NULL THEN \n"
					"				CASE WHEN ReferralSourceT2.PersonID IS NULL THEN \n"
					"					ReferralSourceT1.Name \n"
					"				ELSE \n"
					"					ReferralSourceT2.Name \n"
					"				END \n"
					"			ELSE \n"
					"				ReferralSourceT3.Name \n"
					"			END \n"
					"		ELSE \n"
					"			ReferralSourceT4.Name \n"
					"		END AS Name, \n"
					"		ReferralSourceT1.PersonID AS RootID, \n"
					"		ReferralSourceT1.Name AS RootName \n"
					"	FROM @TempReferralSourceT ReferralSourceT1 \n"
					"		/* HasChildren condition, a hack, because otherwise we \"lose\" the separate row for the parent referral source */ \n"
					"		LEFT JOIN @TempReferralSourceT ReferralSourceT2 ON ReferralSourceT1.PersonID = ReferralSourceT2.Parent AND ReferralSourceT1.HasChildren = 1 \n"
					"		LEFT JOIN @TempReferralSourceT ReferralSourceT3 ON ReferralSourceT2.PersonID = ReferralSourceT3.Parent AND ReferralSourceT2.HasChildren = 1 \n"
					"		LEFT JOIN @TempReferralSourceT ReferralSourceT4 ON ReferralSourceT3.PersonID = ReferralSourceT4.Parent AND ReferralSourceT3.HasChildren = 1 \n"
					"	WHERE ReferralSourceT1.Parent = -1 /* Otherwise we can get non top level referral sources in ReferralSourceT1 */ \n"
					") ReferralSourceSubQ ON DateTableQ.ReferralSourceID = ReferralSourceSubQ.PersonID; \n"
					/*Aside:  This query must end in a semicolon for TTX purposes.  Because there is no WHERE, GROUP BY, etc, it does a reverse
						lookup on the semicolon, which does exist in the table variable queries at the top*/
				);

				//Optimization.  Because we're filtering on Date, but joining the SubQ on "PreviousSunday" (calculated data), SQL 
				//	server isn't able to optimize the SubQ to only look at relevant records, instead it has to query the entire
				//	database.  To improve the speed, we'll apply a date filter manually to the SubQ.
				if(nDateRange > 0) {
					strSql.Replace("@DateOptimization@", 
						"AND LaddersT.FirstInterestDate >= '" + FormatDateTimeForSql(DateFrom) + 
						"' AND LaddersT.FirstInterestDate < DATEADD(d, 1, '" + FormatDateTimeForSql(DateTo) + "')");
				}
				else {
					strSql.Replace("@DateOptimization@", "");
				}

				// Filter on location
				strSql.Replace("@LocationFilter@", BuildWeeklyAdvertisingLocationFilter(nLocation, (CDWordArray*)&m_dwLocations));
				strSql.Replace("@PatientLocFilter@", BuildWeeklyAdvertisingPatientLocationFilter(nLocation, (CDWordArray*)&m_dwLocations));

				return strSql;
			}
			break;

			case 739:
				// (j.luckoski 2012-05-24 14:28) - PLID 48538 - Created
				// (j.luckoski 2012-07-06 11:10) - PLID 48538 - Re-wrote query to utilize a single charge line instead of multi-payment lines
				// (j.luckoski 2012-07-06 12:28) - PLID 48538 - Added more test cases to the CASE for toplevelCatID
				// (j.luckoski 2012-07-09 13:40) - PLID 48538 - Made null revenue lines say $0 instead of being blank.
				return _T("SELECT ChargeID, PatientID AS PatID,  PatientsT.UserDefinedID,  PersonT.First, PersonT.Last, PersonT.Middle, "
					"PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, PatientsT.ReferralID AS ReferralID, "
					"ReferralSourceT.Name AS ReferralName, PatientsT.AffiliatePhysID AS AffiliatePhysID, AffiliatePersonT.FullName AS "
					"AffiliatePhysName, PatientsT.DefaultReferringPhyID AS DefaultReferringPhyID, "
					"ReferringPersonT.FullName AS ReferringPhysName,  ISNULL(SUM(AmountApplied),0) AS Revenue, Quantity AS Units, "
					"CASE "
					"WHEN (FirstCategoryT.Parent = 0 OR FirstCategoryT.Parent = -1 OR FirstCategoryT.Parent IS NULL) THEN FirstCategoryT.ID "
					"ELSE "
					"(CASE WHEN (SecondCategoryT.Parent = 0 OR SecondCategoryT.Parent = -1 OR SecondCategoryT.Parent IS NULL) THEN SecondCategoryT.ID "
					"ELSE "
					"(CASE WHEN (ThirdCategoryT.Parent = 0 OR ThirdCategoryT.Parent = -1 OR ThirdCategoryT.Parent IS NULL) THEN ThirdCategoryT.ID "
					"ELSE FourthCategoryT.ID END) "
					"END) "
					"END AS TopLevelCatID, "			
					"CASE "
					"WHEN (FirstCategoryT.Parent = 0 OR FirstCategoryT.Parent = -1 OR FirstCategoryT.Parent IS NULL) THEN FirstCategoryT.Name "
					"ELSE "
					"(CASE WHEN (SecondCategoryT.Parent = 0 OR SecondCategoryT.Parent = -1 OR SecondCategoryT.Parent IS NULL) THEN SecondCategoryT.Name "
					"ELSE "
					"(CASE WHEN (ThirdCategoryT.Parent = 0 OR ThirdCategoryT.Parent = -1 OR ThirdCategoryT.Parent IS NULL) THEN ThirdCategoryT.Name "
					"ELSE FourthCategoryT.Name END) "
					"END) "
					"END AS TopLevelCatName, "
					"CASE WHEN CPTCodeT.Code IS NULL THEN ServiceT.Name ELSE CPTCodeT.Code END AS Code, "
					"CASE WHEN CPTCodeT.SubCode IS NULL THEN '' ELSE CPTCodeT.SubCode END AS SubCode, "
					"AppliedPaymentsT.DoctorsProviders as ProvID, AppliedPaymentsT.Date as ChargeTDate, "
					"AppliedPaymentsT.InputDate As ChargeIDate, ProvPersonT.First as ProvFirst, "
					"ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, "
					"AppliedPaymentsT.Location AS LocID, LocationsT.Name As	LocationName, "
					"FirstCategoryT.ID AS FirstCatID, FirstCategoryT.Name AS FirstCatName, "
					"SecondCategoryT.ID AS SecondCatID, SecondCategoryT.Name AS SecondCatName, "
					"ThirdCategoryT.ID AS ThirdCatID, ThirdCategoryT.Name AS ThirdCatName, "						
					"FourthCategoryT.ID AS FourthCatID, FourthCategoryT.Name AS FourthCatName "
					"FROM "
					"(SELECT * FROM  "
					"(  "
					"SELECT  " 
					"ChargesT.ID AS ChargeID, ChargesT.Quantity, LineItemT.PatientID, ChargesT.ServiceID, ChargesT.PatCoordID, "
					"ChargesT.DoctorsProviders, ValidBillsQ.Location, LineItemT.Date, LineItemT.InputDate "
					"FROM ChargesT  "
					"LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
					"INNER JOIN (  " 
					"SELECT DISTINCT BillsT.ID AS BillID, BillsT.Location   "
					"FROM BillsT   "
					"LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID  " 
					"INNER JOIN (  " 
					"SELECT DISTINCT BillID   "
					"FROM   "
					"LineItemT  " 
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID   "
					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
					") ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID  " 
					"WHERE  " 
					"BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL "  
					"AND BillsT.EntryType = 1  "
					") AS ValidBillsQ ON ChargesT.BillID = ValidBillsQ.BillID  "
					"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID " 
					"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID  "
					"WHERE  " 
					"LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
					"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) "  
					") ValidChargesT  "
					"LEFT JOIN (  "
					"SELECT  " 
					"AppliesT.SourceID AS AppliedPaymentID,   "
					"AppliesT.DestID AS DestinationID,   "
					"SUM(COALESCE(AppliesT.Amount, 0)) AS AmountApplied  "
					"FROM  "
					"LineItemT " 
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
					"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
					"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON PaymentsT.ID = OrigLineItemsT.OriginalLineItemID  "
					"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON PaymentsT.ID = VoidingLineItemsT.VoidingLineItemID  "
					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL) " 
					"GROUP BY AppliesT.DestID, AppliesT.SourceID  "
					") ChargePaysT ON ValidChargesT.ChargeID = ChargePaysT.DestinationID "
					") AppliedPaymentsT " 
					"LEFT JOIN PatientsT ON AppliedPaymentsT.PatientID = PatientsT.PersonID "
					"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
					"LEFT JOIN PersonT AS ReferringPersonT ON PatientsT.DefaultReferringPhyID = ReferringPersonT.ID "
					"LEFT JOIN PersonT AS AffiliatePersonT ON PatientsT.AffiliatePhysID = AffiliatePersonT.ID  "  
					"LEFT JOIN PersonT CoordPersonT ON AppliedPaymentsT.PatCoordID = CoordPersonT.ID  "
					"LEFT JOIN PersonT ProvPersonT ON AppliedPaymentsT.DoctorsProviders = ProvPersonT.ID "
					"LEFT JOIN LocationsT ON AppliedPaymentsT.Location = LocationsT.ID  "   
					"LEFT JOIN ServiceT ON AppliedPaymentsT.ServiceID = ServiceT.ID  "
					"LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID  "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN CategoriesT AS FirstCategoryT ON ServiceT.Category = FirstCategoryT.ID "
					"LEFT JOIN CategoriesT AS SecondCategoryT ON FirstCategoryT.Parent = SecondCategoryT.ID "
					"LEFT JOIN CategoriesT AS ThirdCategoryT ON SecondCategoryT.Parent = ThirdCategoryT.ID "
					"LEFT JOIN CategoriesT AS FourthCategoryT ON ThirdCategoryT.Parent = FourthCategoryT.ID  "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "	
					"WHERE PatientsT.AffiliatePhysID = PatientsT.DefaultReferringPhyID "
					"GROUP BY ChargeID, PatientID, Quantity, PatientsT.UserDefinedID,  PersonT.First, PersonT.Last, PersonT.Middle, "
					"PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, PatientsT.ReferralID, "
					"ReferralSourceT.Name, PatientsT.AffiliatePhysID, AffiliatePersonT.FullName, PatientsT.DefaultReferringPhyID, "
					"ReferringPersonT.FullName, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name, ServiceT.Category, FirstCategoryT.ID, "
					"SecondCategoryT.ID, ThirdCategoryT.ID, FourthCategoryT.ID, FirstCategoryT.Parent,  SecondCategoryT.Parent, "
					"ThirdCategoryT.Parent, FourthCategoryT.Parent, "
					"FirstCategoryT.Name,  SecondCategoryT.Name, ThirdCategoryT.Name, FourthCategoryT.Name, "
					"AppliedPaymentsT.DoctorsProviders,  AppliedPaymentsT.Date, AppliedPaymentsT.InputDate, ProvPersonT.First, ProvPersonT.Middle, ProvPersonT.Last,	"					
					"AppliedPaymentsT.Location, LocationsT.Name ");
					break;

			case 740:
				// (j.luckoski 2012-06-21 10:40) - PLID 49354 - Created
				// (j.luckoski 2012-07-06 15:03) - PLID 49354 - Re-worked entire query to fix filter by charges and not payments
				// (j.luckoski 2012-07-09 13:40) - PLID 49354 - Made null revenue lines say $0 instead of being blank.
				return _T("SELECT  RevenueQ.PatientID AS PatID, RevenueQ.UserDefinedID, FirstAptQ.AptID, "
					"AppointmentsT.Date as AptDate,CASE WHEN AptTypeID IS NULL THEN -1 ELSE AptTypeID END AS "
					"TypeID, AptTypeT.Name AS AptTypeName, ResourceID AS ResID, ResourceT.Item AS ResourceName, LocationID AS LocID, LocationsT.Name AS LocationName, "
					"RevenueQ.ChargeID, RevenueQ.First, RevenueQ.Last, RevenueQ.Middle, "
					"RevenueQ.Homephone, RevenueQ.WorkPhone, RevenueQ.CellPhone, RevenueQ.ReferralID, "
					"RevenueQ.ReferralName, RevenueQ.AffiliatePhysID, RevenueQ.AffiliatePhysName, RevenueQ.DefaultReferringPhyID, "
					"RevenueQ.ReferringPhysName,  ISNULL(RevenueQ.Revenue, 0) AS Revenue, RevenueQ.Units, NULL AS LocationRevenue, NULL AS LocationUnits, RevenueQ.TopLevelCatID AS TopLevelCatID, RevenueQ.TopLevelCatName, "
					"RevenueQ.Code, RevenueQ.SubCode, RevenueQ.ProvID, RevenueQ.ChargeTDate AS ChargeTDate, "
					"RevenueQ.ChargeIDate, RevenueQ.ProvFirst, RevenueQ.ProvMiddle, RevenueQ.ProvLast, "
					"RevenueQ.LocID AS PlaceOfServiceID, RevenueQ.LocationName AS PlaceOfService,RevenueQ.FirstCatID, "
					"RevenueQ.FirstCatName,RevenueQ.SecondCatID, RevenueQ.SecondCatName, "
					"RevenueQ.ThirdCatID, RevenueQ.ThirdCatName,RevenueQ.FourthCatID, RevenueQ.FourthCatName "
					"FROM "
					"(SELECT PersonT.ID, "
					"(SELECT TOP 1 ID FROM AppointmentsT AptsT "
					"WHERE AptsT.PatientID = PersonT.ID AND Status <> 4 "
					"ORDER BY StartTime ASC) AS AptID "
					"FROM PersonT) FirstAptQ "
					"LEFT JOIN AppointmentsT ON FirstAptQ.AptID = AppointmentsT.ID "
					"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
					"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
					"LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
					"LEFT JOIN "
					"(SELECT ChargeID, PatientID,  PatientsT.UserDefinedID,  PersonT.First, PersonT.Last, PersonT.Middle, "
					"PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, PatientsT.ReferralID AS ReferralID, "
					"ReferralSourceT.Name AS ReferralName, PatientsT.AffiliatePhysID AS AffiliatePhysID, AffiliatePersonT.FullName AS "
					"AffiliatePhysName, PatientsT.DefaultReferringPhyID AS DefaultReferringPhyID, "
					"ReferringPersonT.FullName AS ReferringPhysName,  SUM(AmountApplied) AS Revenue, Quantity AS Units, "
					"CASE "
					"WHEN (FirstCategoryT.Parent = 0 OR FirstCategoryT.Parent = -1 OR FirstCategoryT.Parent IS NULL) THEN FirstCategoryT.ID "
					"ELSE "
					"(CASE WHEN (SecondCategoryT.Parent = 0 OR SecondCategoryT.Parent = -1 OR SecondCategoryT.Parent IS NULL) THEN SecondCategoryT.ID "
					"ELSE "
					"(CASE WHEN (ThirdCategoryT.Parent = 0 OR ThirdCategoryT.Parent = -1 OR ThirdCategoryT.Parent IS NULL) THEN ThirdCategoryT.ID "
					"ELSE FourthCategoryT.ID END) "
					"END) "
					"END AS TopLevelCatID, "	
					"CASE "
					"WHEN (FirstCategoryT.Parent = 0 OR FirstCategoryT.Parent = -1 OR FirstCategoryT.Parent IS NULL) THEN FirstCategoryT.Name "
					"ELSE "
					"(CASE WHEN (SecondCategoryT.Parent = 0 OR SecondCategoryT.Parent = -1 OR SecondCategoryT.Parent IS NULL) THEN SecondCategoryT.Name "
					"ELSE "
					"(CASE WHEN (ThirdCategoryT.Parent = 0 OR ThirdCategoryT.Parent = -1 OR ThirdCategoryT.Parent IS NULL) THEN ThirdCategoryT.Name "
					"ELSE FourthCategoryT.Name END) "
					"END) "
					"END AS TopLevelCatName, "
					"CASE WHEN CPTCodeT.Code IS NULL THEN ServiceT.Name ELSE CPTCodeT.Code END AS Code, "
					"CASE WHEN CPTCodeT.SubCode IS NULL THEN '' ELSE CPTCodeT.SubCode END AS SubCode, "
					"AppliedPaymentsT.DoctorsProviders as ProvID, AppliedPaymentsT.Date as ChargeTDate, "
					"AppliedPaymentsT.InputDate As ChargeIDate, ProvPersonT.First as ProvFirst, "
					"ProvPersonT.Middle as ProvMiddle, ProvPersonT.Last as ProvLast, "
					"AppliedPaymentsT.Location AS LocID, LocationsT.Name As	LocationName, "
					"FirstCategoryT.ID AS FirstCatID, FirstCategoryT.Name AS FirstCatName, "
					"SecondCategoryT.ID AS SecondCatID, SecondCategoryT.Name AS SecondCatName, "
					"ThirdCategoryT.ID AS ThirdCatID, ThirdCategoryT.Name AS ThirdCatName, 	"					
					"FourthCategoryT.ID AS FourthCatID, FourthCategoryT.Name AS FourthCatName "
					"FROM "
					"(SELECT * FROM  "
					"(  "
					"SELECT   "
					"ChargesT.ID AS ChargeID, ChargesT.Quantity, LineItemT.PatientID, ChargesT.ServiceID, ChargesT.PatCoordID, "
					"ChargesT.DoctorsProviders, ValidBillsQ.Location, LineItemT.Date, LineItemT.InputDate "
					"FROM ChargesT  "
					"LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID  "
					"INNER JOIN (   "
					"SELECT DISTINCT BillsT.ID AS BillID, BillsT.Location   "
					"FROM BillsT   "
					"LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID   "
					"INNER JOIN (   "
					"SELECT DISTINCT BillID   "
					"FROM  " 
					"LineItemT  " 
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID   "
					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
					") ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID   "
					"WHERE   "
					"BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL   "
					"AND BillsT.EntryType = 1  "
					") AS ValidBillsQ ON ChargesT.BillID = ValidBillsQ.BillID  "
					"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON ChargesT.ID = OrigLineItemsT.OriginalLineItemID  "
					"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON ChargesT.ID = VoidingLineItemsT.VoidingLineItemID  "
					"WHERE   "
					"LineItemT.Deleted = 0 AND LineItemT.Type = 10  "
					"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL)   "
					") ValidChargesT  "
					"LEFT JOIN (  "
					"SELECT   "
					"AppliesT.SourceID AS AppliedPaymentID,   "
					"AppliesT.DestID AS DestinationID,   "
					"SUM(COALESCE(AppliesT.Amount, 0)) AS AmountApplied  "
					"FROM  "
					"LineItemT  "
					"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID  "
					"INNER JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID  "
					"LEFT JOIN LineItemCorrectionsT OrigLineItemsT ON PaymentsT.ID = OrigLineItemsT.OriginalLineItemID  "
					"LEFT JOIN LineItemCorrectionsT VoidingLineItemsT ON PaymentsT.ID = VoidingLineItemsT.VoidingLineItemID  "
					"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
					"AND (OrigLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL)  "
					"GROUP BY AppliesT.DestID, AppliesT.SourceID  "
					") ChargePaysT ON ValidChargesT.ChargeID = ChargePaysT.DestinationID "
					") AppliedPaymentsT "
					"LEFT JOIN PatientsT ON AppliedPaymentsT.PatientID = PatientsT.PersonID "
					"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
					"LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID "
					"LEFT JOIN PersonT AS ReferringPersonT ON PatientsT.DefaultReferringPhyID = ReferringPersonT.ID "
					"LEFT JOIN PersonT AS AffiliatePersonT ON PatientsT.AffiliatePhysID = AffiliatePersonT.ID "   
					"LEFT JOIN PersonT CoordPersonT ON AppliedPaymentsT.PatCoordID = CoordPersonT.ID  "
					"LEFT JOIN PersonT ProvPersonT ON AppliedPaymentsT.DoctorsProviders = ProvPersonT.ID "
					"LEFT JOIN LocationsT ON AppliedPaymentsT.Location = LocationsT.ID  " 
					"LEFT JOIN ServiceT ON AppliedPaymentsT.ServiceID = ServiceT.ID  "
					"LEFT JOIN ProcedureT ON ServiceT.ProcedureID = ProcedureT.ID  "
					"LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
					"LEFT JOIN CategoriesT AS FirstCategoryT ON ServiceT.Category = FirstCategoryT.ID "
					"LEFT JOIN CategoriesT AS SecondCategoryT ON FirstCategoryT.Parent = SecondCategoryT.ID "
					"LEFT JOIN CategoriesT AS ThirdCategoryT ON SecondCategoryT.Parent = ThirdCategoryT.ID "
					"LEFT JOIN CategoriesT AS FourthCategoryT ON ThirdCategoryT.Parent = FourthCategoryT.ID  "
					"LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"GROUP BY ChargeID, PatientID, Quantity, PatientsT.UserDefinedID,  PersonT.First, PersonT.Last, PersonT.Middle, "
					"PersonT.Homephone, PersonT.WorkPhone, PersonT.CellPhone, PatientsT.ReferralID, "
					"ReferralSourceT.Name, PatientsT.AffiliatePhysID, AffiliatePersonT.FullName, PatientsT.DefaultReferringPhyID, "
					"ReferringPersonT.FullName, CPTCodeT.Code, CPTCodeT.SubCode, ServiceT.Name, ServiceT.Category, FirstCategoryT.ID, "
					"SecondCategoryT.ID, ThirdCategoryT.ID, FourthCategoryT.ID, FirstCategoryT.Parent,  SecondCategoryT.Parent, "
					"ThirdCategoryT.Parent, FourthCategoryT.Parent, "
					"FirstCategoryT.Name,  SecondCategoryT.Name, ThirdCategoryT.Name, FourthCategoryT.Name, "
					"AppliedPaymentsT.DoctorsProviders,  AppliedPaymentsT.Date, AppliedPaymentsT.InputDate, ProvPersonT.First, ProvPersonT.Middle, "										
					"ProvPersonT.Last, AppliedPaymentsT.Location, LocationsT.Name) RevenueQ ON FirstAptQ.ID = RevenueQ.PatientID "
					"WHERE AptID IS NOT NULL AND RevenueQ.PatientID <> -25  AND RevenueQ.ChargeTDate >= AppointmentsT.Date");
				break;

		default:
		return _T("");
		break;
	}
}
