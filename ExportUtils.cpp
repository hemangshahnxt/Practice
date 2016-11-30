#include "stdafx.h"
#include "ExportUtils.h"
#include "EmrUtils.h"
#include "SharedInsuranceUtils.h"  //(r.wilson 10/2/2012) plid 52970
#include "GlobalFinancialUtils.h"

#define BEGIN_ADD_FIELDS(reps)			const CExportField reps[] = {
#define ADD_FIELD						CExportField
#define END_ADD_FIELDS(reps, repcnt)	}; const long repcnt = sizeof(reps) / sizeof(CExportField)


//The ID is stored to data, so don't change it.  The order they are on screen is the order they'll show in the Available Fields
//list.  Keep NEXT_EXPORT_ID up to date.
#define NEXT_EXPORT_ID	549
BEGIN_ADD_FIELDS(g_arExportFields)
ADD_FIELD(1, "{Placeholder}", "''", eftPlaceholder, 0xFFFF, FALSE),
ADD_FIELD(8, "Today's Date", "getdate()", eftDateTime, 0xFFFF, FALSE),
ADD_FIELD(2, "Patient ID", "PatientsT.UserDefinedID", eftGenericNumber, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(86, "Patient Prefix", "PrefixPat.Prefix", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(87, "Patient First Name", "PersonPat.First", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(88, "Patient Middle Name", "PersonPat.Middle", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(89, "Patient Middle Initial", "Left(LTrim(PersonPat.Middle), 1)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(90, "Patient Last Name", "PersonPat.Last", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(91, "Patient Location", "(SELECT LocationsT.Name FROM LocationsT WHERE ID = PersonPat.Location)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(92, "Patient Full Name", "PersonPat.First + ' ' + PersonPat.Middle + CASE WHEN Len(PersonPat.Middle) = 1 THEN '.' ELSE '' END + CASE WHEN PersonPat.Middle = '' OR PersonPat.Middle IS NULL THEN '' ELSE  + ' ' END + PersonPat.Last", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(468, "Patient Title", "PersonPat.Title", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(93, "Patient Formal Name", "CASE WHEN PrefixPat.Prefix Is Null THEN '' ELSE PrefixPat.Prefix + ' ' END + PersonPat.Last", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(94, "Patient Nickname", "PatientsT.Nickname", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),

ADD_FIELD(95, "Patient Address 1", "PersonPat.Address1", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(96, "Patient Address 2", "PersonPat.Address2", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(97, "Patient City", "PersonPat.City", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(98, "Patient State", "PersonPat.State", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(99, "Patient Zip", "PersonPat.Zip", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(6, "Patient Home Phone", "PersonPat.HomePhone", eftPhoneNumber, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(100, "Patient Work Phone", "PersonPat.WorkPhone", eftPhoneNumber, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(101, "Patient Extension", "PersonPat.Extension", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(102, "Patient Mobile Phone", "PersonPat.CellPhone", eftPhoneNumber, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(103, "Patient Fax", "PersonPat.Fax", eftPhoneNumber, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(469, "Patient E-Mail", "PersonPat.Email", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(104, "Patient Gender", "PersonPat.Gender", eftGender, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
// (j.gruber 2010-11-03 15:20) - PLID 40877 - mask SSN
// (a.walling 2011-03-11 15:24) - PLID 42786 - Need to mask for all exports with patient info rather than simply patient exports!
// This is now handled via CExportField for any eftSSN type fields
ADD_FIELD(7, "Patient SSN", "PersonPat.SocialSecurity", eftSSN, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(106, "Patient Birth Date", "PersonPat.BirthDate", eftDateTime, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(107, "Patient Age", "CASE WHEN PersonPat.BirthDate > GetDate() THEN 0 ELSE YEAR(GETDATE()-PersonPat.BirthDate)-1900 END", eftGenericNumber, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(108, "Patient Occupation", "PatientsT.Occupation", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(109, "Patient Marital Status", "PatientsT.MaritalStatus", eftMarital, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(110, "Patient Spouse", "PatientsT.SpouseName", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(139, "Patient Status", "CASE WHEN PatientsT.CurrentStatus=1 THEN 'Patient' WHEN PatientsT.CurrentStatus=3 THEN 'Patient/Prospect' ELSE 'Prospect' END", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(140, "Patient First Contact Date", "PersonPat.FirstContactDAte", eftDateTime, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(141, "Patient Note", "PersonPat.Note", eftGenericNtext, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),

//DRT 6/18/2007 - PLID 25333 - Added all of these provider fields to the charges query
ADD_FIELD(46, "Provider Prefix", "PrefixProv.Prefix", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(47, "Provider First Name", "PersonProv.First", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(48, "Provider Middle Name", "PersonProv.Middle", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(49, "Provider Last Name", "PersonProv.Last", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(50, "Provider Title", "PersonProv.Title", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(51, "Provider Full Name", "PersonProv.First + ' ' + PersonProv.Middle + CASE WHEN LEN(PersonProv.Middle) = 1 THEN '.' ELSE '' END + CASE WHEN PersonProv.Middle = '' THEN '' ELSE ' ' END + PersonProv.Last + CASE WHEN PersonProv.Title = '' THEN '' ELSE ', ' + PersonProv.Title END", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(52, "Provider Formal Name", "CASE WHEN PrefixProv.Prefix IS NULL THEN '' ELSE PrefixProv.Prefix  END + ' ' + PersonProv.Last", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(53, "Provider Address 1", "PersonProv.Address1", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(54, "Provider Address 2", "PersonProv.Address2", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(55, "Provider City", "PersonProv.City", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(56, "Provider State", "PersonProv.State", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(57, "Provider Zip", "PersonProv.Zip", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(58, "Provider Phone", "PersonProv.WorkPhone", eftPhoneNumber, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(59, "Provider Extension", "PersonProv.Extension", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(60, "Provider Fax", "PersonProv.Fax", eftPhoneNumber, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(61, "Provider E-Mail", "PersonProv.Email", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(62, "Provider Gender", "PersonProv.Gender", eftGender, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(63, "Provider UPIN", "ProvidersT.UPIN", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(64, "Provider License Number", "ProvidersT.License", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(65, "Provider DEA Number", "ProvidersT.[DEA Number]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(66, "Provider EIN", "ProvidersT.[Fed Employer ID]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(67, "Provider Medicare Number", "ProvidersT.[Medicare Number]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(68, "Provider Medicaid Number", "ProvidersT.[Medicaid Number]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(69, "Provider BCBS Number", "ProvidersT.[BCBS Number]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(70, "Provider Worker's Comp Number", "ProvidersT.[Workers Comp Number]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),
ADD_FIELD(71, "Provider Other ID Number", "ProvidersT.[Other ID Number]", eftGenericText, EFS_PATIENTS|EFS_CHARGES, FALSE),

ADD_FIELD(121, "Patient Coordinator Prefix", "PrefixCoord.Prefix", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(122, "Patient Coordinator First Name", "PersonCoord.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(123, "Patient Coordinator Middle Name", "PersonCoord.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(124, "Patient Coordinator Last Name", "PersonCoord.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(125, "Patient Coordinator Title", "PersonCoord.Title", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(126, "Patient Coordinator Full Name", "PersonCoord.First + ' ' + PersonCoord.Middle + CASE WHEN LEN(PersonCoord.Middle) = 1 THEN '.' ELSE '' END + CASE WHEN PersonCoord.Middle = '' THEN '' ELSE ' ' END + PersonCoord.Last + CASE WHEN PersonCoord.Title = '' THEN '' ELSE ', ' + PersonCoord.Title END", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(127, "Patient Coordinator Formal Name", "CASE WHEN PrefixCoord.Prefix IS NULL THEN '' ELSE PrefixCoord.Prefix END + ' ' + PersonCoord.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(128, "Patient Coordinator Address 1", "PersonCoord.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(129, "Patient Coordinator Address 2", "PersonCoord.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(130, "Patient Coordinator City", "PersonCoord.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(131, "Patient Coordinator State", "PersonCoord.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(132, "Patient Coordinator Zip", "PersonCoord.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(133, "Patient Coordinator Phone", "PersonCoord.WorkPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(134, "Patient Coordinator Email", "PersonCoord.Email", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(72, "Emergency Contact First Name", "PersonPat.EmergFirst", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(73, "Emergency Contact Last Name", "PersonPat.EmergLast", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(74, "Emergency Contact Home Phone", "PersonPat.EmergHPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(75, "Emergency Contact Work Phone", "PersonPat.EmergWPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(76, "Emergency Contact Relationship", "PersonPat.EmergRelation", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(135, "Patient Referral Source (Primary)", "(SELECT Name FROM ReferralSourceT WHERE PersonID = PatientsT.ReferralID)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
// (d.moore 2007-05-02 14:03) - PLID 23602 - 'CurrentStatus = 3' was used to determine both the number of Patients and the number of Prospects. It is now only counted as a Patient.
ADD_FIELD(136, "# of Patients Referred", "(SELECT Count(ReferringPatientID) AS CountOfReferredPatients FROM PatientsT WITH(NOLOCK) WHERE ReferringPatientID = PersonPat.ID AND (CurrentStatus = 1 OR CurrentStatus = 3))", eftGenericNumber, EFS_PATIENTS, TRUE),
ADD_FIELD(137, "# of Prospects Referred", "(SELECT Count(ReferringPatientID) AS CountOfReferredProspects FROM PatientsT WITH(NOLOCK) WHERE ReferringPatientID = PersonPat.ID AND CurrentStatus = 2)", eftGenericNumber, EFS_PATIENTS, TRUE),
ADD_FIELD(138, "Patient Type", "GroupTypes.GroupName", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(111, "Patient Warning Message", "PersonPat.WarningMessage", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
// (b.spivey, May 22, 2013) - PLID 56892 - The query that pulled these fields has changed. 
ADD_FIELD(105, "Patient Race", "RaceTextSubQ.RaceName", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
ADD_FIELD(503, "Patient CDC Race", "RaceTextSubQ.OfficialRaceName", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure, this now pulls the "practice" name
ADD_FIELD(504, "Patient CDC Ethnicity", "EthnicityPat.Name", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),
//(e.lally 2011-06-17) PLID 43992 - Patient language
ADD_FIELD(505, "Patient Language", "LanguagePat.Name", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, FALSE),

ADD_FIELD(77, "Employer Company", "PersonPat.Company", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(78, "Manager's First Name", "PatientsT.EmployerFirst", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(79, "Manager's Middle Name", "PatientsT.EmployerMiddle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(80, "Manager's Last Name", "PatientsT.EmployerLast", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(81, "Employer Address 1", "PatientsT.EmployerAddress1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(82, "Employer Address 2", "PatientsT.EmployerAddress2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(83, "Employer City", "PatientsT.EmployerCity", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(84, "Employer State", "PatientsT.EmployerState", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(85, "Employer Zip Code", "PatientsT.EmployerZip", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(112, "Patient Default ICD-9 Code 1", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID1)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(113, "Patient Default ICD-9 Description 1", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID1)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(114, "Patient Default ICD-9 Code 2", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID2)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(115, "Patient Default ICD-9 Description 2", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID2)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(116, "Patient Default ICD-9 Code 3", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID3)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(117, "Patient Default ICD-9 Description 3", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID3)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(118, "Patient Default ICD-9 Code 4", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID4)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(119, "Patient Default ICD-9 Description 4", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultDiagID4)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
// (a.wilson 2014-03-04 17:19) - PLID 60874 - Adding Patient Default ICD-10 Codes
ADD_FIELD(539, "Patient Default ICD-10 Code 1", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID1)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(540, "Patient Default ICD-10 Description 1", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID1)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(541, "Patient Default ICD-10 Code 2", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID2)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(542, "Patient Default ICD-10 Description 2", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID2)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(543, "Patient Default ICD-10 Code 3", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID3)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(544, "Patient Default ICD-10 Description 3", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID3)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(545, "Patient Default ICD-10 Code 4", "(SELECT DiagCodes.CodeNumber FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID4)", eftDiag, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(546, "Patient Default ICD-10 Description 4", "(SELECT DiagCodes.CodeDesc FROM DiagCodes WHERE DiagCodes.ID = PatientsT.DefaultICD10DiagID4)", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS|EFS_CHARGES|EFS_PAYMENTS|EFS_EMNS, TRUE),
ADD_FIELD(120, "Patient Date of Current Illness", "PatientsT.DefaultInjuryDate", eftDateTime, EFS_PATIENTS, FALSE),

ADD_FIELD(142, "Referring Physician Prefix", "PrefixRefPhys.Prefix", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(143, "Referring Physician First Name", "PersonRefPhys.First", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(144, "Referring Physician Middle Name", "PersonRefPhys.Middle", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(145, "Referring Physician Last Name", "PersonRefPhys.Last", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(146, "Referring Physician Title", "PersonRefPhys.Title", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(147, "Referring Physician Formal Name", "CASE WHEN PrefixRefPhys.Prefix IS NULL THEN '' ELSE PrefixRefPhys.Prefix  END + ' ' + PersonRefPhys.Last", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(148, "Referring Physician Address 1", "PersonRefPhys.Address1", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(149, "Referring Physician Address 2", "PersonRefPhys.Address2", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(150, "Referring Physician City", "PersonRefPhys.City", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(151, "Referring Physician State", "PersonRefPhys.State", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(152, "Referring Physician Zip", "PersonRefPhys.Zip", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(153, "Referring Physician Phone", "PersonRefPhys.WorkPhone", eftPhoneNumber, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(154, "Referring Physician Extension", "PersonRefPhys.Extension", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(155, "Referring Physician Fax", "PersonRefPhys.Fax", eftPhoneNumber, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(156, "Referring Physician Company", "PersonRefPhys.Company", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),
ADD_FIELD(157, "Referring Physician UPIN", "ReferringPhysT.UPIN", eftGenericText, EFS_PATIENTS|EFS_APPOINTMENTS, FALSE),

ADD_FIELD(158, "Primary Care Physician Prefix", "PrefixPCP.Prefix", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(159, "Primary Care Physician First Name", "PersonPCP.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(160, "Primary Care Physician Middle Name", "PersonPCP.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(161, "Primary Care Physician Last Name", "PersonPCP.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(162, "Primary Care Physician Title", "PersonPCP.Title", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(163, "Primary Care Physician Address 1", "PersonPCP.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(164, "Primary Care Physician Address 2", "PersonPCP.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(165, "Primary Care Physician City", "PersonPCP.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(166, "Primary Care Physician State", "PersonPCP.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(167, "Primary Care Physician Zip", "PersonPCP.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(168, "Primary Care Physician Phone", "PersonPCP.WorkPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(169, "Primary Care Physician Extension", "PersonPCP.Extension", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(170, "Primary Care Physician Fax", "PersonPCP.Fax", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(171, "Primary Care Physician UPIN", "ReferringPhysPCP.UPIN", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(172, "Referring Patient Prefix", "PrefixRefPat.Prefix", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(173, "Referring Patient First Name", "PersonRefPat.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(174, "Referring Patient Middle Name", "PersonRefPat.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(175, "Referring Patient Last Name", "PersonRefPat.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(176, "Referring Patient Address 1", "PersonRefPat.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(177, "Referring Patient Address 2", "PersonRefPat.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(178, "Referring Patient City", "PersonRefPat.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(179, "Referring Patient State", "PersonRefPat.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(180, "Referring Patient Zip", "PersonRefPat.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(181, "Referring Patient Home Phone", "PersonRefPat.HomePhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(182, "Referring Patient Work Phone", "PersonRefPat.WorkPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),

ADD_FIELD(183, "Last Referred Patient Prefix", "PrefixLastRefPat.Prefix", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(184, "Last Referred Patient First Name", "PersonLastRefPat.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(185, "Last Referred Patient Middle Name", "PersonLastRefPat.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(186, "Last Referred Patient Last Name", "PersonLastRefPat.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(187, "Last Referred Patient Address 1", "PersonLastRefPat.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(188, "Last Referred Patient Address 2", "PersonLastRefPat.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(189, "Last Referred Patient City", "PersonLastRefPat.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(190, "Last Referred Patient State", "PersonLastRefPat.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(191, "Last Referred Patient Zip", "PersonLastRefPat.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(192, "Last Referred Patient Home Phone", "PersonLastRefPat.HomePhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(193, "Last Referred Patient Work Phone", "PersonLastRefPat.WorkPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),

ADD_FIELD(224, "Location", "LocationsT.Name", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(225, "Location Address 1", "LocationsT.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(226, "Location Address 2", "LocationsT.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(227, "Location City", "LocationsT.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(228, "Location State", "LocationsT.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(229, "Location Zip", "LocationsT.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(230, "Location Main Phone", "LocationsT.Phone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(231, "Location Alternate Phone", "LocationsT.Phone2", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(232, "Location Fax", "LocationsT.Fax", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(233, "Location Toll Free Phone", "LocationsT.TollFreePhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(234, "Location EIN", "LocationsT.EIN", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(235, "Location E-Mail", "LocationsT.OnLineAddress", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(236, "Location Website", "LocationsT.Website", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(237, "Location Notes", "LocationsT.Notes", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(9, "CUSTOM_FIELD_NAME(1)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 1)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(10, "CUSTOM_FIELD_NAME(2)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 2)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(11, "CUSTOM_FIELD_NAME(3)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 3)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(12, "CUSTOM_FIELD_NAME(4)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 4)", eftGenericText, EFS_PATIENTS, TRUE),
//I'm fairly certain that the CustomFieldsT.ID of 5 is never accessible to the user.
ADD_FIELD(13, "CUSTOM_FIELD_NAME(11)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 11)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(14, "CUSTOM_FIELD_NAME(12)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 12)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(15, "CUSTOM_FIELD_NAME(13)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 13)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(16, "CUSTOM_FIELD_NAME(14)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 14)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(17, "CUSTOM_FIELD_NAME(15)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 15)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(18, "CUSTOM_FIELD_NAME(16)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 16)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(19, "CUSTOM_FIELD_NAME(17)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 17)", eftGenericText, EFS_PATIENTS, TRUE),
// (j.armen 2011-06-27 16:52) - PLID 44253 - Update the export utils to get a list of all selected custom list items
ADD_FIELD(20, "CUSTOM_FIELD_NAME(21)", "dbo.GetCustomList(PersonPat.ID, 21)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(21, "CUSTOM_FIELD_NAME(22)", "dbo.GetCustomList(PersonPat.ID, 22)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(22, "CUSTOM_FIELD_NAME(23)", "dbo.GetCustomList(PersonPat.ID, 23)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(23, "CUSTOM_FIELD_NAME(24)", "dbo.GetCustomList(PersonPat.ID, 24)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(24, "CUSTOM_FIELD_NAME(25)", "dbo.GetCustomList(PersonPat.ID, 25)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(25, "CUSTOM_FIELD_NAME(26)", "dbo.GetCustomList(PersonPat.ID, 26)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(26, "CUSTOM_FIELD_NAME(31) First Name", "CustomContactPerson.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(27, "CUSTOM_FIELD_NAME(31) Middle Name", "CustomContactPerson.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(28, "CUSTOM_FIELD_NAME(31) Last Name", "CustomContactPerson.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(29, "CUSTOM_FIELD_NAME(31) Address 1", "CustomContactPerson.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(30, "CUSTOM_FIELD_NAME(31) Address 2", "CustomContactPerson.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(31, "CUSTOM_FIELD_NAME(31) City", "CustomContactPerson.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(32, "CUSTOM_FIELD_NAME(31) State", "CustomContactPerson.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(33, "CUSTOM_FIELD_NAME(31) Zip", "CustomContactPerson.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(34, "CUSTOM_FIELD_NAME(31) Work Phone", "CustomContactPerson.WorkPhone", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(35, "CUSTOM_FIELD_NAME(31) Extension", "CustomContactPerson.Extension", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(36, "CUSTOM_FIELD_NAME(41)", "(SELECT convert(bit,IntParam) FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 41)", eftBool, EFS_PATIENTS, TRUE),
ADD_FIELD(37, "CUSTOM_FIELD_NAME(42)", "(SELECT convert(bit,IntParam) FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 42)", eftBool, EFS_PATIENTS, TRUE),
ADD_FIELD(38, "CUSTOM_FIELD_NAME(43)", "(SELECT convert(bit,IntParam) FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 43)", eftBool, EFS_PATIENTS, TRUE),
ADD_FIELD(39, "CUSTOM_FIELD_NAME(44)", "(SELECT convert(bit,IntParam) FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 44)", eftBool, EFS_PATIENTS, TRUE),
ADD_FIELD(40, "CUSTOM_FIELD_NAME(45)", "(SELECT convert(bit,IntParam) FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 45)", eftBool, EFS_PATIENTS, TRUE),
ADD_FIELD(41, "CUSTOM_FIELD_NAME(46)", "(SELECT convert(bit,IntParam) FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 46)", eftBool, EFS_PATIENTS, TRUE),
ADD_FIELD(42, "CUSTOM_FIELD_NAME(51)", "(SELECT DateParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 51)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(43, "CUSTOM_FIELD_NAME(52)", "(SELECT DateParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 52)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(44, "CUSTOM_FIELD_NAME(53)", "(SELECT DateParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 53)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(45, "CUSTOM_FIELD_NAME(54)", "(SELECT DateParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 54)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(496, "CUSTOM_FIELD_NAME(90)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 90)", eftGenericText, EFS_PATIENTS, TRUE), // (a.walling 2007-07-03 09:34) - PLID 15491 - Added more custom text fields
ADD_FIELD(497, "CUSTOM_FIELD_NAME(91)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 91)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(498, "CUSTOM_FIELD_NAME(92)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 92)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(499, "CUSTOM_FIELD_NAME(93)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 93)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(500, "CUSTOM_FIELD_NAME(94)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 94)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(501, "CUSTOM_FIELD_NAME(95)", "(SELECT TextParam FROM CustomFieldDataT WHERE PersonID = PersonPat.ID AND FieldID = 95)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(194, "Primary Ins. Party First Name", "PersonPri.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(195, "Primary Ins. Party Middle Name", "PersonPri.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(196, "Primary Ins. Party Last Name", "PersonPri.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(197, "Primary Ins. Party Address 1", "PersonPri.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(198, "Primary Ins. Party Address 2", "PersonPri.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(199, "Primary Ins. Party City", "PersonPri.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(200, "Primary Ins. Party State", "PersonPri.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(201, "Primary Ins. Party Zip", "PersonPri.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(202, "Primary Ins. Party Phone", "PersonPri.HomePhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(203, "Primary Ins. Party Gender", "PersonPri.Gender", eftGender, EFS_PATIENTS, FALSE),
ADD_FIELD(204, "Primary Ins. Party Group Number", "InsuredPartyPri.PolicyGroupNum", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(205, "Primary Ins. Party Effective Date", "InsuredPartyPri.EffectiveDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(206, "Primary Ins. Party Copay Amount", "InsuredPartyPri.CoPay", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(481, "Primary Ins. Party Copay Percent", "InsuredPartyPri.CopayPercent", eftGenericNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(207, "Primary Ins. Party ID Number", "InsuredPartyPri.IDForInsurance", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(208, "Primary Ins. Party Company", "InsuranceCoPri.Name", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(209, "Primary Ins. Party Plan", "InsurancePlansPri.PlanName", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(210, "Primary Ins. Party Birth Date", "PersonPri.BirthDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(211, "Primary Ins. Party Employer", "InsuredPartyPri.Employer", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(212, "Primary Ins. Party Relationship", "InsuredPartyPri.RelationToPatient", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(213, "Primary Ins. Party Note", "PersonPri.Note", eftGenericNtext, EFS_PATIENTS, FALSE),
ADD_FIELD(238, "Primary Ins. Co. Address 1", "PersonPriCo.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(239, "Primary Ins. Co. Address 2", "PersonPriCo.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(240, "Primary Ins. Co. City", "PersonPriCo.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(241, "Primary Ins. Co. State", "PersonPriCo.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(242, "Primary Ins. Co. Zip", "PersonPriCo.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(243, "Primary Ins. Co. Phone", "PersonPriContact.WorkPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(244, "Primary Ins. Co. Fax", "PersonPriContact.Fax", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(245, "Primary Ins. Co. Contact First Name", "PersonPriContact.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(246, "Primary Ins. Co. Contact Last Name", "PersonPriContact.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(247, "Primary Ins. Co. Contact Extension", "PersonPriContact.Extension", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(214, "Secondary Ins. Party First Name", "PersonSec.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(215, "Secondary Ins. Party Middle Name", "PersonSec.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(216, "Secondary Ins. Party Last Name", "PersonSec.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(217, "Secondary Ins. Party Address 1", "PersonSec.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(218, "Secondary Ins. Party Address 2", "PersonSec.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(219, "Secondary Ins. Party City", "PersonSec.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(220, "Secondary Ins. Party State", "PersonSec.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(221, "Secondary Ins. Party Zip", "PersonSec.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(222, "Secondary Ins. Party Phone", "PersonSec.HomePhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(223, "Secondary Ins. Party Gender", "PersonSec.Gender", eftGender, EFS_PATIENTS, FALSE),
ADD_FIELD(488, "Secondary Ins. Party Group Number", "InsuredPartySec.PolicyGroupNum", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(489, "Secondary Ins. Party Effective Date", "InsuredPartySec.EffectiveDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(483, "Secondary Ins. Party Copay Amount", "InsuredPartySec.CoPay", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(482, "Secondary Ins. Party Copay Percent", "InsuredPartySec.CopayPercent", eftGenericNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(490, "Secondary Ins. Party ID Number", "InsuredPartySec.IDForInsurance", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(491, "Secondary Ins. Party Company", "InsuranceCoSec.Name", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(492, "Secondary Ins. Party Plan", "InsurancePlansSec.PlanName", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(484, "Secondary Ins. Party Birth Date", "PersonSec.BirthDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(485, "Secondary Ins. Party Employer", "InsuredPartySec.Employer", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(486, "Secondary Ins. Party Relationship", "InsuredPartySec.RelationToPatient", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(487, "Secondary Ins. Party Note", "PersonSec.Note", eftGenericNtext, EFS_PATIENTS, FALSE),
ADD_FIELD(248, "Secondary Ins. Co. Address 1", "PersonSecCo.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(249, "Secondary Ins. Co. Address 2", "PersonSecCo.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(250, "Secondary Ins. Co. City", "PersonSecCo.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(251, "Secondary Ins. Co. State", "PersonSecCo.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(252, "Secondary Ins. Co. Zip", "PersonSecCo.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(253, "Secondary Ins. Co. Phone", "PersonSecContact.WorkPhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(254, "Secondary Ins. Co. Fax", "PersonSecContact.Fax", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(255, "Secondary Ins. Co. Contact First Name", "PersonSecContact.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(256, "Secondary Ins. Co. Contact Last Name", "PersonSecContact.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(257, "Secondary Ins. Co. Contact Extension", "PersonSecContact.Extension", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(258, "Responsible Party First Name", "PersonRespParty.First", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(259, "Responsible Party Middle Name", "PersonRespParty.Middle", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(260, "Responsible Party Middle Initial", "LEFT(LTRIM(PersonRespParty.Middle),1)", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(261, "Responsible Party Last Name", "PersonRespParty.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(262, "Responsible Party Full Name", "PersonRespParty.First + CASE WHEN PersonRespParty.Middle = '' THEN '' ELSE ' ' END + PersonRespParty.Middle + ' ' + PersonRespParty.Last", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(263, "Responsible Party Address 1", "PersonRespParty.Address1", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(264, "Responsible Party Address 2", "PersonRespParty.Address2", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(265, "Responsible Party City", "PersonRespParty.City", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(266, "Responsible Party State", "PersonRespParty.State", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(267, "Responsible Party Zip", "PersonRespParty.Zip", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(268, "Responsible Party Home Phone", "PersonRespParty.HomePhone", eftPhoneNumber, EFS_PATIENTS, FALSE),
ADD_FIELD(269, "Responsible Party Employer", "ResponsiblePartyT.Employer", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(270, "Responsible Party Birth Date", "PersonRespParty.Birthdate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(271, "Responsible Party Relation", "ResponsiblePartyT.RelationToPatient", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(272, "Responsible Party Gender", "PersonRespParty.Gender", eftGender, EFS_PATIENTS, FALSE),

ADD_FIELD(359, "Allergy List", "dbo.GetAllergyList(PatientsT.PersonID)", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(360, "Current Medications List", "dbo.GetCurrentMedsList(PatientsT.PersonID)", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(361, "Serial Number List", "dbo.GetSerialNumberList(PatientsT.PersonID)", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(362, "Groups", "dbo.GetGroupString(PatientsT.PersonID)", eftGenericText, EFS_PATIENTS, FALSE),

ADD_FIELD(363, "Next Appointment Date", "(SELECT Min(AppointmentsT.Date) FROM AppointmentsT WITH(NOLOCK) WHERE AppointmentsT.PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(364, "Next Appointment Start Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, NextStartTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, NextStartTime, 100), 18, 2))) AS NextStartTime FROM (SELECT TOP 1 StartTime AS NextStartTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date ASC) NextApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(365, "Next Appointment End Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, NextEndTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, NextEndTime, 100), 18, 2))) AS NextEndTime FROM (SELECT TOP 1 EndTime AS NextEndTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date ASC) NextApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(366, "Next Appointment Purpose", "(SELECT dbo.GetPurposeString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date ASC) NextApptQ)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(367, "Next Appointment Resource", "(SELECT dbo.GetResourceString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date ASC) NextApptQ)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(368, "Last Appointment Date", "(SELECT Max(AppointmentsT.Date) FROM AppointmentsT WITH(NOLOCK) WHERE AppointmentsT.PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(369, "Last Appointment Start Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, LastStartTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, LastStartTime, 100), 18, 2))) AS LastStartTime FROM (SELECT TOP 1 StartTime AS LastStartTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date Desc) LastApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(370, "Last Appointment End Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, LastEndTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, LastEndTime, 100), 18, 2))) AS LastEndTime FROM (SELECT TOP 1 EndTime AS LastEndTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date Desc) LastApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(371, "Last Appointment Purpose", "(SELECT dbo.GetPurposeString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date Desc) LastApptQ)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(372, "Last Appointment Resource", "(SELECT dbo.GetResourceString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 ORDER BY Date Desc) LastApptQ)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(373, "Next Procedure Date", "(SELECT Min(AppointmentsT.Date) FROM AppointmentsT WITH(NOLOCK) WHERE AppointmentsT.PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6))", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(374, "Next Procedure Start Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, NextStartTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, NextStartTime, 100), 18, 2))) AS NextStartTime FROM (SELECT TOP 1 StartTime AS NextStartTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date ASC) NextApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(375, "Next Procedure End Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, NextEndTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, NextEndTime, 100), 18, 2))) AS NextEndTime FROM (SELECT TOP 1 EndTime AS NextEndTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date ASC) NextApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(376, "Next Procedure Purpose", "(SELECT dbo.GetPurposeString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date ASC) NextApptQ)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(377, "Next Procedure Resource", "(SELECT dbo.GetResourceString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date ASC) NextApptQ)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(378, "Last Procedure Date", "(SELECT Max(AppointmentsT.Date) FROM AppointmentsT WITH(NOLOCK) WHERE AppointmentsT.PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6))", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(379, "Last Procedure Start Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, LastStartTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, LastStartTime, 100), 18, 2))) AS LastStartTime FROM (SELECT TOP 1 StartTime AS LastStartTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date Desc) LastApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(380, "Last Procedure End Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, LastEndTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, LastEndTime, 100), 18, 2))) AS LastEndTime FROM (SELECT TOP 1 EndTime AS LastEndTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date Desc) LastApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(381, "Last Procedure Purpose", "(SELECT dbo.GetPurposeString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date Desc) LastApptQ)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(382, "Last Procedure Resource", "(SELECT dbo.GetResourceString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 3 OR Category = 4 OR Category = 6) ORDER BY Date Desc) LastApptQ)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(383, "Next Surgery Date", "(SELECT Min(AppointmentsT.Date) FROM AppointmentsT WITH(NOLOCK) WHERE AppointmentsT.PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4))", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(384, "Next Surgery Start Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, NextStartTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, NextStartTime, 100), 18, 2))) AS NextStartTime FROM (SELECT TOP 1 StartTime AS NextStartTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date ASC) NextApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(385, "Next Surgery End Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, NextEndTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, NextEndTime, 100), 18, 2))) AS NextEndTime FROM (SELECT TOP 1 EndTime AS NextEndTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date ASC) NextApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(386, "Next Surgery Purpose", "(SELECT dbo.GetPurposeString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date ASC) NextApptQ)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(387, "Next Surgery Resource", "(SELECT dbo.GetResourceString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date > GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date ASC) NextApptQ)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(388, "Last Surgery Date", "(SELECT Max(AppointmentsT.Date) FROM AppointmentsT WITH(NOLOCK) WHERE AppointmentsT.PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4))", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(389, "Last Surgery Start Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, LastStartTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, LastStartTime, 100), 18, 2))) AS LastStartTime FROM (SELECT TOP 1 StartTime AS LastStartTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date Desc) LastApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(390, "Last Surgery End Time", "(SELECT convert(datetime, LTRIM(SUBSTRING(CONVERT(char, LastEndTime, 100), 13, 5) + ' ' + SUBSTRING(CONVERT(char, LastEndTime, 100), 18, 2))) AS LastEndTime FROM (SELECT TOP 1 EndTime AS LastEndTime FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date Desc) LastApptQ)", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(391, "Last Surgery Purpose", "(SELECT dbo.GetPurposeString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date Desc) LastApptQ)", eftGenericText, EFS_PATIENTS, TRUE),
ADD_FIELD(392, "Last Surgery Resource", "(SELECT dbo.GetResourceString(LastApptID) FROM (SELECT TOP 1 ID AS LastApptID FROM AppointmentsT WITH(NOLOCK) WHERE PatientID = PatientsT.PersonID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4 AND AppointmentsT.AptTypeID IN (SELECT ID FROM AptTypeT WHERE Category = 4) ORDER BY Date Desc) LastApptQ)", eftGenericText, EFS_PATIENTS, TRUE),

ADD_FIELD(393, "Last Payment Date", "LastPaymentQ.Date", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(394, "Last Payment Amount", "LastPaymentQ.Amount", eftCurrency, EFS_PATIENTS, FALSE),
// (j.jones 2008-09-05 12:40) - PLID 30288 - supported MailSentNotesT
ADD_FIELD(395, "Last Statement Date", "(SELECT Max(Date) FROM MailSent INNER JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID WHERE MailSentNotesT.Note Like '%Patient Statement%Printed%' AND MailSent.PersonID = PatientsT.PersonID AND MailSent.Date <= GetDate())", eftDateTime, EFS_PATIENTS, TRUE),
ADD_FIELD(396, "Total Balance", "BalanceQ.TotalPatCharges-BalanceQ.TotalPatPays + BalanceQ.TotalPriInsCharges-BalanceQ.TotalPriPays + BalanceQ.TotalSecInsCharges-BalanceQ.TotalSecPays + BalanceQ.TotalOthInsCharges-BalanceQ.TotalOthPays", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(397, "Balance (Patient Responsibility)", "BalanceQ.TotalPatCharges-BalanceQ.TotalPatPays", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(398, "Balance (Insurance Responsibility)", "BalanceQ.TotalPriInsCharges-BalanceQ.TotalPriPays + BalanceQ.TotalSecInsCharges-BalanceQ.TotalSecPays + BalanceQ.TotalOthInsCharges-BalanceQ.TotalOthPays", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(399, "Balance (Pri. Ins. Responsibility)", "BalanceQ.TotalPriInsCharges-BalanceQ.TotalPriPays", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(400, "Balance (Sec. Ins. Responsibility)", "BalanceQ.TotalSecInsCharges-BalanceQ.TotalSecPays", eftCurrency, EFS_PATIENTS, FALSE),
ADD_FIELD(401, "Balance (Other Ins. Responsibility)", "BalanceQ.TotalOthInsCharges-BalanceQ.TotalOthPays", eftCurrency, EFS_PATIENTS, FALSE),

//TES 1/22/2009 - PLID 32841 - COALESCE(0,LastBillQ.ID) is NOT equivalent to COALESCE(LastBillQ.ID,0)
ADD_FIELD(402, "Last Service Codes Billed", "dbo.GetCPTListFromBill(COALESCE(LastBillQ.ID,0))", eftGenericText, EFS_PATIENTS, FALSE),
// (a.wilson 2014-03-05 09:28) - PLID 61115 - rename to "ICD-9", duplicate for a ICD-10 code.
ADD_FIELD(403, "Last ICD-9 Codes Billed", "LastBillQ.ICD9CodeList", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(547, "Last ICD-10 Codes Billed", "LastBillQ.ICD10CodeList", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(404, "Global Period Service Code", "GlobalPeriodQ.Code", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(405, "Global Period Service Code Description", "GlobalPeriodQ.Name", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(406, "Global Period Start Date", "GlobalPeriodQ.Date", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(407, "Global Period End Date", "GlobalPeriodQ.ExpDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(408, "Global Period Days", "GlobalPeriodQ.GlobalPeriod", eftGenericNumber, EFS_PATIENTS, FALSE),

ADD_FIELD(409, "Ins. Referral Auth. Num.", "ActiveInsReferralQ.AuthNum", eftGenericText, EFS_PATIENTS, FALSE),
ADD_FIELD(410, "Ins. Referral Start Date", "ActiveInsReferralQ.StartDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(411, "Ins. Referral End Date", "ActiveInsReferralQ.EndDate", eftDateTime, EFS_PATIENTS, FALSE),
ADD_FIELD(412, "Ins. Referral # of Visits", "ActiveInsReferralQ.NumVisits", eftGenericNumber, EFS_PATIENTS, FALSE),

ADD_FIELD(4, "Charge Amount", "dbo.GetChargeTotal(ChargesT.ID)", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(273, "Charge Description", "LineChargeT.Description", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(274, "Charge Service Date", "LineChargeT.Date", eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(275, "Charge Input Date", "LineChargeT.InputDate", eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(276, "Charge Unit Price", "LineChargeT.Amount", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(277, "Charge Quantity", "ChargesT.Quantity", eftGenericNumber, EFS_CHARGES, FALSE),
// (j.gruber 2009-03-17 17:46) - PLID 33360 - changed for new discount structure
ADD_FIELD(278, "Charge PreTax Amount", "convert(money,LineChargeT.Amount * ChargesT.Quantity * COALESCE(CPTModifierT.Multiplier,1)*COALESCE(CPTModifierT2.Multiplier,1)*COALESCE(CPTModifierT3.Multiplier,1)*COALESCE(CPTModifierT4.Multiplier,1)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(279, "Charge Balance", "dbo.GetChargeTotal(ChargesT.ID) - COALESCE((SELECT Sum(Amount) FROM AppliesT WHERE DestID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(280, "Charge Amount Paid", "COALESCE((SELECT Sum(Amount) FROM AppliesT WHERE DestID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(281, "Charge Tax Rate", "ChargesT.TaxRate", eftGenericNumber, EFS_CHARGES, FALSE),
// (j.gruber 2009-03-17 17:46) - PLID 33360 - changed for new discount structure
ADD_FIELD(466, "Charge Tax Amount", "Round(convert(money, Convert(money,((LineChargeT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate-1)))),2)", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(282, "Charge Tax Rate 2", "ChargesT.TaxRate2", eftGenericNumber, EFS_CHARGES, FALSE),
// (j.gruber 2009-03-17 17:46) - PLID 33360 - changed for new discount structure
ADD_FIELD(467, "Charge Tax Amount 2", "Round(convert(money, Convert(money,((LineChargeT.[Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null) THEN 1 ELSE ((100-Convert(float,(SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID)))/100) END)-(CASE WHEN (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) Is Null THEN 0 ELSE (SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID) END))*(ChargesT.TaxRate2-1)))),2)", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(283, "Charge CPT Code", "COALESCE(CptCodeT.Code,'')", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(495, "Charge CPT Modifier", "ChargesT.CPTModifier", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(284, "Charge CPT Modifier 2", "ChargesT.CPTModifier2", eftGenericText, EFS_CHARGES, FALSE),
// (a.wilson 2014-03-05 09:28) - PLID 61115 - rename to "Charge Diagnosis Pointers"
ADD_FIELD(285, "Charge Diagnosis Pointers", "ChargeWhichCodes.WhichCodes", eftGenericText, EFS_CHARGES, FALSE),
// (j.gruber 2009-03-17 17:46) - PLID 33360 - changed for new discount structure
ADD_FIELD(286, "Charge Percent Off Total", "COALESCE((SELECT Sum(PercentOff) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID), 0) ", eftGenericNumber, EFS_CHARGES, FALSE),
// (j.gruber 2009-03-17 17:46) - PLID 33360 - changed for new discount structure
ADD_FIELD(287, "Charge Discount Total", "COALESCE((SELECT Sum(Discount) FROM ChargeDiscountsT WHERE DELETED = 0 AND ChargeID = ChargesT.ID), 0)", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(415, "Charge Amount (Pat. Resp.)", "COALESCE((SELECT Sum(Amount) FROM ChargeRespT WHERE InsuredPartyID Is Null AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(416, "Charge Amount (Ins. Resp.)", "COALESCE((SELECT Sum(Amount) FROM ChargeRespT WHERE InsuredPartyID Is Not Null AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(417, "Charge Amount (Pri. Ins. Resp.)", "COALESCE((SELECT Sum(Amount) FROM ChargeRespT WHERE InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = 1 AND PatientID = LineChargeT.PatientID) AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
// (r.gonet 08/13/2014) - PLID 63086 - This field was reporting the primary insurance's amount rather than the secondary's. Fixed.
ADD_FIELD(418, "Charge Amount (Sec. Ins. Resp.)", "COALESCE((SELECT Sum(Amount) FROM ChargeRespT WHERE InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = 2 AND PatientID = LineChargeT.PatientID) AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(419, "Charge Amount (Other Ins. Resp.)", "COALESCE((SELECT Sum(Amount) FROM ChargeRespT WHERE InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT WHERE RespTypeID NOT IN (1,2) AND PatientID = LineChargeT.PatientID) AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),

// (r.gonet 08/13/2014) - PLID 63086 - Account for the fact that AppliesT.Amount can be NULL if there are no applies for the responsibility. Also fixed the respid for secondary insurance. It was reporting as primary.
ADD_FIELD(420, "Charge Balance (Pat. Resp.)", "COALESCE((SELECT Sum(Balance) FROM (SELECT ChargeRespT.ID, ChargeRespT.Amount-Sum(ISNULL(AppliesT.Amount,0)) AS Balance FROM ChargeRespT LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID GROUP BY ChargeRespT.ID,ChargeRespT.Amount) RespBalQ INNER JOIN ChargeRespT ON RespBalQ.ID = ChargeRespT.ID WHERE InsuredPartyID Is Null AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(421, "Charge Balance (Ins. Resp.)", "COALESCE((SELECT Sum(Balance) FROM (SELECT ChargeRespT.ID, ChargeRespT.Amount-Sum(ISNULL(AppliesT.Amount,0)) AS Balance FROM ChargeRespT LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID GROUP BY ChargeRespT.ID,ChargeRespT.Amount) RespBalQ INNER JOIN ChargeRespT ON RespBalQ.ID = ChargeRespT.ID WHERE InsuredPartyID Is Not Null AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(422, "Charge Balance (Pri. Ins. Resp.)", "COALESCE((SELECT Sum(Balance) FROM (SELECT ChargeRespT.ID, ChargeRespT.Amount-Sum(ISNULL(AppliesT.Amount,0)) AS Balance FROM ChargeRespT LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID GROUP BY ChargeRespT.ID,ChargeRespT.Amount) RespBalQ INNER JOIN ChargeRespT ON RespBalQ.ID = ChargeRespT.ID WHERE InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = 1 AND PatientID = LineChargeT.PatientID) AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(423, "Charge Balance (Sec. Ins. Resp.)", "COALESCE((SELECT Sum(Balance) FROM (SELECT ChargeRespT.ID, ChargeRespT.Amount-Sum(ISNULL(AppliesT.Amount, 0)) AS Balance FROM ChargeRespT LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID GROUP BY ChargeRespT.ID,ChargeRespT.Amount) RespBalQ INNER JOIN ChargeRespT ON RespBalQ.ID = ChargeRespT.ID WHERE InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = 2 AND PatientID = LineChargeT.PatientID) AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(424, "Charge Balance (Other Ins. Resp.)", "COALESCE((SELECT Sum(Balance) FROM (SELECT ChargeRespT.ID, ChargeRespT.Amount-Sum(ISNULL(AppliesT.Amount,0)) AS Balance FROM ChargeRespT LEFT JOIN AppliesT ON ChargeRespT.ID = AppliesT.RespID GROUP BY ChargeRespT.ID,ChargeRespT.Amount) RespBalQ INNER JOIN ChargeRespT ON RespBalQ.ID = ChargeRespT.ID WHERE InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT WHERE RespTypeID NOT IN (1,2) AND PatientID = LineChargeT.PatientID) AND ChargeID = ChargesT.ID),0)", eftCurrency, EFS_CHARGES, TRUE),

//TES 5/9/2008 - PLID 27177 - Added the charge category
ADD_FIELD(502, "Charge Category", "ChargeCategoriesT.Name", eftGenericText, EFS_CHARGES, FALSE),

// Show all bill information
ADD_FIELD(288, "Bill Description", "BillsT.ExtraDesc", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(289, "Bill Date", "BillsT.Date", eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(290, "Bill Input Date", "BillsT.InputDate", eftDateTime, EFS_CHARGES, FALSE),
//(r.wilson 10/3/2012) plid 52970 - Changed SendType = -1 to Enum
ADD_FIELD(291, "Bill Last Claim Sent Date", _T(FormatString("(SELECT Max(Date) AS Date FROM ClaimHistoryT WHERE BillID = BillsT.ID AND SendType <> %li)",ClaimSendType::TracerLetter)), eftDateTime, EFS_CHARGES, TRUE),
ADD_FIELD(292, "Bill Total Amount", "dbo.GetBillTotal(BillsT.ID)", eftCurrency, EFS_CHARGES, FALSE),
ADD_FIELD(293, "Bill Balance", "dbo.GetBillTotal(BillsT.ID) - COALESCE((SELECT Sum(Amount) FROM AppliesT WHERE DestID IN (SELECT ID FROM ChargesT WHERE BillID = BillsT.ID)),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(294, "Bill Amount Paid", "COALESCE((SELECT Sum(Amount) FROM AppliesT WHERE DestID IN (SELECT ID FROM ChargesT WHERE BillID = BillsT.ID)),0)", eftCurrency, EFS_CHARGES, TRUE),
ADD_FIELD(295, "Bill Prior Auth. Num.", "BillsT.PriorAuthNum", eftGenericText, EFS_CHARGES, FALSE),

// (a.walling 2014-02-28 17:35) - PLID 61126 - BillDiagCodeT - ExportUtils. Now using BillDiagCodeFlat12V, adding 12 diag codes and ICD9/ICD10 variants thereof
ADD_FIELD(296, "Bill Diagnosis Code 1", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag1ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(297, "Bill Diagnosis Code 2", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag2ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(298, "Bill Diagnosis Code 3", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag3ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(299, "Bill Diagnosis Code 4", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag4ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(507, "Bill Diagnosis Code 5", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag5ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(508, "Bill Diagnosis Code 6", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag6ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(509, "Bill Diagnosis Code 7", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag7ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(510, "Bill Diagnosis Code 8", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag8ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(511, "Bill Diagnosis Code 9", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag9ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(512, "Bill Diagnosis Code 10", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag10ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(513, "Bill Diagnosis Code 11", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag11ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(514, "Bill Diagnosis Code 12", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.Diag12ID)", eftDiag, EFS_CHARGES, TRUE),

ADD_FIELD(515, "Bill ICD-9 Diagnosis Code 1", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag1ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(516, "Bill ICD-9 Diagnosis Code 2", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag2ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(517, "Bill ICD-9 Diagnosis Code 3", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag3ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(518, "Bill ICD-9 Diagnosis Code 4", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag4ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(519, "Bill ICD-9 Diagnosis Code 5", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag5ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(520, "Bill ICD-9 Diagnosis Code 6", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag6ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(521, "Bill ICD-9 Diagnosis Code 7", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag7ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(522, "Bill ICD-9 Diagnosis Code 8", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag8ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(523, "Bill ICD-9 Diagnosis Code 9", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag9ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(524, "Bill ICD-9 Diagnosis Code 10", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag10ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(525, "Bill ICD-9 Diagnosis Code 11", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag11ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(526, "Bill ICD-9 Diagnosis Code 12", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD9Diag12ID)", eftDiag, EFS_CHARGES, TRUE),

ADD_FIELD(527, "Bill ICD-10 Diagnosis Code 1", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag1ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(528, "Bill ICD-10 Diagnosis Code 2", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag2ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(529, "Bill ICD-10 Diagnosis Code 3", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag3ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(530, "Bill ICD-10 Diagnosis Code 4", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag4ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(531, "Bill ICD-10 Diagnosis Code 5", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag5ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(532, "Bill ICD-10 Diagnosis Code 6", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag6ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(533, "Bill ICD-10 Diagnosis Code 7", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag7ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(534, "Bill ICD-10 Diagnosis Code 8", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag8ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(535, "Bill ICD-10 Diagnosis Code 9", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag9ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(536, "Bill ICD-10 Diagnosis Code 10", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag10ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(537, "Bill ICD-10 Diagnosis Code 11", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag11ID)", eftDiag, EFS_CHARGES, TRUE),
ADD_FIELD(538, "Bill ICD-10 Diagnosis Code 12", "(SELECT CodeNumber FROM DiagCodes WHERE ID = BillDiagCodeFlat12V.ICD10Diag12ID)", eftDiag, EFS_CHARGES, TRUE),

ADD_FIELD(300, "Bill Unable To Work From", "BillsT.NoWorkFrom", eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(301, "Bill Unable To Work To", "BillsT.NoWorkTo", eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(302, "Bill Hospitalized From", "BillsT.HospFrom", eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(303, "Bill Hospitalized To", "BillsT.HospTo", eftDateTime, EFS_CHARGES, FALSE),
// (r.gonet 2016-04-07) - NX-100072 - Split FirstConditionDate into multiple date fields.
ADD_FIELD(304, "Bill First Date Of Illness", FormatString(
	"CASE BillsT.ConditionDateType "
	"	WHEN %li THEN BillsT.FirstVisitOrConsultationDate "
	"	WHEN %li THEN BillsT.InitialTreatmentDate "
	"	WHEN %li THEN BillsT.LastSeenDate "
	"	WHEN %li THEN BillsT.AcuteManifestationDate "
	"	WHEN %li THEN BillsT.LastXRayDate "
	"	WHEN %li THEN BillsT.HearingAndPrescriptionDate "
	"	WHEN %li THEN BillsT.AssumedCareDate "
	"	WHEN %li THEN BillsT.RelinquishedCareDate "
	"	WHEN %li THEN BillsT.AccidentDate "
	"	ELSE NULL "
	"END",
	ConditionDateType::cdtFirstVisitOrConsultation444,
	ConditionDateType::cdtInitialTreatmentDate454,
	ConditionDateType::cdtLastSeenDate304,
	ConditionDateType::cdtAcuteManifestation453,
	ConditionDateType::cdtLastXray455,
	ConditionDateType::cdtHearingAndPrescription471,
	ConditionDateType::cdtAssumedCare090,
	ConditionDateType::cdtRelinquishedCare91,
	ConditionDateType::cdtAccident439),
	eftDateTime, EFS_CHARGES, FALSE),
ADD_FIELD(305, "Bill Date Of Current Illness", "BillsT.ConditionDate", eftDateTime, EFS_CHARGES, FALSE),
		
ADD_FIELD(306, "Bill POS", "BillPlaceOfServiceT.Name", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(307, "Bill POS Address 1", "BillPlaceOfServiceT.Address1", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(308, "Bill POS Address 2", "BillPlaceOfServiceT.Address2", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(309, "Bill POS City", "BillPlaceOfServiceT.City", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(310, "Bill POS State", "BillPlaceOfServiceT.State", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(311, "Bill POS Zip Code", "BillPlaceOfServiceT.Zip", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(312, "Bill POS Main Phone", "BillPlaceOfServiceT.Phone", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(313, "Bill POS Alternate Phone", "BillPlaceOfServiceT.Phone2", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(314, "Bill POS Fax", "BillPlaceOfServiceT.Fax", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(315, "Bill POS Toll-Free Phone", "BillPlaceOfServiceT.TollFreePhone", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(316, "Bill POS EIN", "BillPlaceOfServiceT.EIN", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(317, "Bill POS E-mail", "BillPlaceOfServiceT.OnLineAddress", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(318, "Bill POS Website", "BillPlaceOfServiceT.Website", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(319, "Bill POS Notes", "BillPlaceOfServiceT.Notes", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(320, "Bill POS Designation", "PlaceOfServiceCodesT.PlaceCodes", eftGenericText, EFS_CHARGES, FALSE),

ADD_FIELD(321, "Bill Location", "BillLocationT.Name", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(322, "Bill Location Address 1", "BillLocationT.Address1", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(323, "Bill Location Address 2", "BillLocationT.Address2", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(324, "Bill Location City", "BillLocationT.City", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(325, "Bill Location State", "BillLocationT.State", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(326, "Bill Location Zip Code", "BillLocationT.Zip", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(327, "Bill Location Main Phone", "BillLocationT.Phone", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(328, "Bill Location Alternate Phone", "BillLocationT.Phone2", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(329, "Bill Location Fax", "BillLocationT.Fax", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(330, "Bill Location Toll-Free Phone", "BillLocationT.TollFreePhone", eftPhoneNumber, EFS_CHARGES, FALSE),
ADD_FIELD(331, "Bill Location EIN", "BillLocationT.EIN", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(332, "Bill Location E-mail", "BillLocationT.OnLineAddress", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(333, "Bill Location Website", "BillLocationT.Website", eftGenericText, EFS_CHARGES, FALSE),
ADD_FIELD(334, "Bill Location Notes", "BillLocationT.Notes", eftGenericText, EFS_CHARGES, FALSE),

ADD_FIELD(3, "Appointment Date", "AppointmentsT.Date", eftDateTime, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(335, "Appointment Notes", "AppointmentsT.Notes", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(336, "Appointment Purpose", "dbo.GetPurposeString(AppointmentsT.ID)", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(337, "Appointment Resource", "dbo.GetResourceString(AppointmentsT.ID)", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(338, "Appointment Start Time", "AppointmentsT.StartTime", eftDateTime, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(339, "Appointment End Time", "AppointmentsT.EndTime", eftDateTime, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(340, "Appointment Type", "AptTypeT.Name", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(341, "Appointment Confirmed", "CASE WHEN AppointmentsT.Confirmed = 0 THEN 'No' WHEN AppointmentsT.Confirmed = 1 THEN 'Yes' WHEN AppointmentsT.COnfirmed = 2 THEN 'LM' END", eftGenericText, EFS_APPOINTMENTS, FALSE),
// (j.armen 2011-07-06 11:07) - PLID 44205 - Added Confirmed By to export
ADD_FIELD(506, "Appointment Confirmed By", "AppointmentsT.ConfirmedBy", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(342, "Appointment Location", "AppointmentLocationT.Name", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(343, "Appointment Location Address 1", "AppointmentLocationT.Address1", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(344, "Appointment Location Address 2", "AppointmentLocationT.Address2", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(345, "Appointment Location City", "AppointmentLocationT.City", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(346, "Appointment Location State", "AppointmentLocationT.State", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(347, "Appointment Location Zip Code", "AppointmentLocationT.Zip", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(348, "Appointment Location Main Phone", "AppointmentLocationT.Phone", eftPhoneNumber, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(349, "Appointment Location Alternate Phone", "AppointmentLocationT.Phone2", eftPhoneNumber, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(350, "Appointment Location Fax", "AppointmentLocationT.Fax", eftPhoneNumber, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(351, "Appointment Location Toll-Free Phone", "AppointmentLocationT.TollFreePhone", eftPhoneNumber, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(352, "Appointment Location EIN", "AppointmentLocationT.EIN", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(353, "Appointment Location E-mail", "AppointmentLocationT.OnLineAddress", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(354, "Appointment Location Website", "AppointmentLocationT.Website", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(355, "Appointment Location Notes", "AppointmentLocationT.Notes", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(356, "Appointment Status", "AptShowStateT.Name", eftGenericText, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(357, "Appointment NoShow Date", "CASE WHEN ShowState = 3 THEN AppointmentsT.NoShowDate ELSE NULL END", eftDateTime, EFS_APPOINTMENTS, FALSE),
ADD_FIELD(358, "Appointment Is Event?", "convert(bit,CASE WHEN AppointmentsT.StartTime = AppointmentsT.EndTime AND DATEPART(hh, AppointmentsT.StartTime) = 0 AND DATEPART(mi, AppointmentsT.StartTime) = 0 AND DATEPART(ss, AppointmentsT.StartTime) = 0 THEN 1 ELSE 0 END)", eftBool, EFS_APPOINTMENTS, FALSE),
		
ADD_FIELD(5, "Payment Description", "LinePayT.Description", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(451, "Payment Date", "LinePayT.Date", eftDateTime, EFS_PAYMENTS, FALSE),
ADD_FIELD(452, "Payment Amount", "LinePayT.Amount", eftCurrency, EFS_PAYMENTS, FALSE),
ADD_FIELD(453, "Payment Method", "CASE WHEN PaymentsT.PayMethod = 1 THEN 'Cash' WHEN PaymentsT.PayMethod = 2 THEN 'Check'  WHEN PaymentsT.PayMethod = 3 THEN 'Credit' END", eftGenericText, EFS_PAYMENTS, FALSE),

ADD_FIELD(425, "Provider Prefix", "PrefixProv.Prefix", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(426, "Provider First Name", "PaymentProv.First", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(427, "Provider Middle Name", "PaymentProv.Middle", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(428, "Provider Last Name", "PaymentProv.Last", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(429, "Provider Title", "PaymentProv.Title", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(430, "Provider Full Name", "PaymentProv.First + ' ' + PaymentProv.Middle + CASE WHEN LEN(PaymentProv.Middle) = 1 THEN '.' ELSE '' END + CASE WHEN PaymentProv.Middle = '' THEN '' ELSE ' ' END + PaymentProv.Last + CASE WHEN PaymentProv.Title = '' THEN '' ELSE ', ' + PaymentProv.Title END", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(431, "Provider Formal Name", "CASE WHEN PrefixProv.Prefix IS NULL THEN '' ELSE PrefixProv.Prefix  END + ' ' + PaymentProv.Last", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(432, "Provider Address 1", "PaymentProv.Address1", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(433, "Provider Address 2", "PaymentProv.Address2", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(434, "Provider City", "PaymentProv.City", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(435, "Provider State", "PaymentProv.State", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(436, "Provider Zip", "PaymentProv.Zip", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(437, "Provider Phone", "PaymentProv.WorkPhone", eftPhoneNumber, EFS_PAYMENTS, FALSE),
ADD_FIELD(438, "Provider Extension", "PaymentProv.Extension", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(439, "Provider Fax", "PaymentProv.Fax", eftPhoneNumber, EFS_PAYMENTS, FALSE),
ADD_FIELD(440, "Provider E-Mail", "PaymentProv.Email", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(441, "Provider Gender", "PaymentProv.Gender", eftGender, EFS_PAYMENTS, FALSE),
ADD_FIELD(442, "Provider UPIN", "ProvidersT.UPIN", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(443, "Provider License Number", "ProvidersT.License", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(444, "Provider DEA Number", "ProvidersT.[DEA Number]", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(445, "Provider EIN", "ProvidersT.[Fed Employer ID]", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(446, "Provider Medicare Number", "ProvidersT.[Medicare Number]", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(447, "Provider Medicaid Number", "ProvidersT.[Medicaid Number]", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(448, "Provider BCBS Number", "ProvidersT.[BCBS Number]", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(449, "Provider Worker's Comp Number", "ProvidersT.[Workers Comp Number]", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(450, "Provider Other ID Number", "ProvidersT.[Other ID Number]", eftGenericText, EFS_PAYMENTS, FALSE),

ADD_FIELD(454, "Payment Check Number", "PaymentPlansT.CheckNo", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(455, "Payment Bank Number", "PaymentPlansT.BankNo", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(456, "Payment Account Number", "PaymentPlansT.CheckAcctNo", eftGenericText, EFS_PAYMENTS, FALSE),
// (e.lally 2007-07-09) PLID 25993 - Use the new credit card ID field
ADD_FIELD(457, "Payment CC Type", "CreditCardNamesT.CardName", eftGenericText, EFS_PAYMENTS, FALSE),
// (j.gruber 2007-05-01 17:24) - PLID 25745 - only show the last 4 digits of the ccnumber
ADD_FIELD(458, "Payment CC Number", "CASE WHEN Len(PaymentPlansT.CCNumber) = 0 then '' else 'XXXXXXXXXXXX' + Right(PaymentPlansT.CCNumber, 4) END ", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(459, "Payment CC Name", "PaymentPlansT.CCHoldersName", eftGenericText, EFS_PAYMENTS, FALSE),
// (j.gruber 2007-05-15 09:06) - PLID 25987 - need to remove credit card exp. dates
ADD_FIELD(460, "Payment Exp. Date", " Convert(DateTime, NULL) ", eftDateTime, EFS_PAYMENTS, FALSE),
ADD_FIELD(461, "Payment CC. Auth. No.", "PaymentPlansT.CCAuthNo", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(462, "Is Prepayment?", "PaymentsT.PrePayment", eftBool, EFS_PAYMENTS, FALSE),
ADD_FIELD(463, "Payment Type", "CASE WHEN LinePayT.Type = 1 THEN 'Payment' WHEN LinePayT.Type = 2 THEN 'Adjustment' WHEN LinePayT.Type = 3 THEN 'Refund' END", eftGenericText, EFS_PAYMENTS, FALSE),
ADD_FIELD(464, "Is Insurance?", "convert(bit,(CASE WHEN PaymentsT.InsuredPartyID = -1 THEN 0 ELSE 1 END))", eftBool, EFS_PAYMENTS, FALSE),
ADD_FIELD(465, "Insurance Company", "PayInsCoT.Name", eftGenericText, EFS_PAYMENTS, FALSE),

ADD_FIELD(470, "EMN Description", "EmrMasterT.Description", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(471, "EMN Date", "EmrMasterT.Date", eftDateTime, EFS_EMNS, FALSE),
ADD_FIELD(472, "EMN Input Date", "EmrMasterT.InputDate", eftDateTime, EFS_EMNS, FALSE),
ADD_FIELD(473, "EMN Location", "LocEmr.Name", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(474, "EMN Provider", "dbo.GetEmnProviderList(EmrMasterT.ID)", eftGenericText, EFS_EMNS, FALSE),
// (j.jones 2011-07-05 17:24) - PLID 44432 - supported custom statuses
ADD_FIELD(475, "EMN Status", "EMRStatusListT.Name", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(476, "EMN Collection", "EmrCollectionT.Name", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(493, "EMN Service Codes", "dbo.GetCPTListFromEMN(EmrMasterT.ID)", eftGenericText, EFS_EMNS, FALSE),
// (a.wilson 2014-03-05 09:28) - PLID 60874 - rename to "ICD-9", duplicate for ICD-10 Codes.
ADD_FIELD(494, "EMN ICD-9 Codes", "EMRDiagCodes.ICD9Codes", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(548, "EMN ICD-10 Codes", "EMRDiagCodes.ICD10Codes", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(477, "EMR Description", "EmrGroupsT.Description", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(478, "EMR Status", "CASE WHEN EmrGroupsT.Status = 0 THEN 'Open' WHEN EmrGroupsT.Status = 1 THEN 'Closed' ELSE 'Invalid Status!' END", eftGenericText, EFS_EMNS, FALSE),
ADD_FIELD(479, "EMR Input Date", "EmrGroupsT.InputDate", eftDateTime, EFS_EMNS, FALSE),
ADD_FIELD(480, "", "(SELECT TOP 1 EmrDetailsT.ID FROM EmrDetailsT INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID WHERE EmrMasterT.Deleted = 0 AND EmrDetailsT.Deleted = 0 AND EmrDetailsT.EmrID = EmrMasterT.ID AND EmrInfoT.EmrInfoMasterID = DYNAMIC_ID)", eftEmnItem, EFS_EMNS, TRUE),

ADD_FIELD(413, "{Advanced}", "", eftAdvanced, 0xFFFF, FALSE),

END_ADD_FIELDS(g_arExportFields, g_nExportFieldCount);

CExportField GetFieldByID(int nID)
{
	for(int i = 0; i < g_nExportFieldCount; i++) {
		if(g_arExportFields[i].nID == nID)
			return g_arExportFields[i];
	}
	ASSERT(FALSE);
	CExportField ef(-1, "", "", eftPlaceholder, 0, FALSE);
	return ef;
}

//TES 6/29/2007 - PLID 26396 - Similar to GetFieldByID(), this function returns the index in the global array of the
// given ID.  At the moment, this is only used in an ASSERT statement to check for duplicated Field IDs.
int GetFieldIndex(int nID)
{
	for(int i = 0; i < g_nExportFieldCount; i++) {
		if(g_arExportFields[i].nID == nID)
			return i;
	}
	return -1;
}

CExportField::CExportField(int nID, CString strDisplayName, CString strField, ExportFieldType eftType, DWORD dwSupportedRecordTypes, BOOL bHasAJoinOrFromClause)
{
	//TES 6/29/2007 - PLID 26396 - If there is an assertion failure on this line, it is an indication
	// that the id used in this ADD_FIELD statement is out of range
	// It must be greater than 0 and less than NEXT_EXPORT_ID; 
	// this is just an aid to help ensure the appropriate use of info ids
	ASSERT((nID > 0 && nID < NEXT_EXPORT_ID));

	//TES 6/29/2007 - PLID 26396 - If there is an assertion failure on this line, it is an indication 
	// that the id used in this ADD_FIELD statement is already in 
	// use by a different one
	ASSERT(GetFieldIndex(nID) == -1);

	this->nID = nID;
	this->strDisplayName = strDisplayName;
	this->strField = strField;
	this->eftType = eftType;
	this->dwSupportedRecordTypes = dwSupportedRecordTypes;
	this->bHasAJoinOrFromClause = bHasAJoinOrFromClause;
}

CString CExportField::GetDisplayName()
{
	int nCustomFieldBegin = strDisplayName.Find("CUSTOM_FIELD_NAME(");
	if(nCustomFieldBegin != -1) {
		int nCustomFieldEnd = strDisplayName.Find(")", nCustomFieldBegin);
		long nFieldID = atol(strDisplayName.Mid(nCustomFieldBegin+18, nCustomFieldEnd-nCustomFieldBegin-18));
		// (b.cardillo 2006-05-26 14:40) - PLID 20828 - Use the cached custom field names instead 
		// of running a separate query for each one.
		return strDisplayName.Left(nCustomFieldBegin) + 
			GetCustomFieldName(nFieldID) + strDisplayName.Mid(nCustomFieldEnd+1);
	}
	else {
		return strDisplayName;
	}
}

// (a.walling 2011-03-11 15:24) - PLID 42786 - Need to mask SSN for all exports with patient info rather than simply patient exports!
// It is much simpler to just handle it here rather than mess with the query, since the SSNMask thing is not dependent on data at all.
// Now GetField() will format as appropriate if the ExportFieldType is eftSSN.
CString CExportField::GetField()
{
	if (eftType == eftSSN) {
		BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
		BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);

		// this is more intuitive than the nested ternary ?: operators
		long nMaskPatientSSN = 1; // hide by default

		if (bSSNReadPermission) {
			if (bSSNDisableMasking) {
				nMaskPatientSSN = -1; // show all
			} else {
				nMaskPatientSSN = 0; // show 4
			}
		}

		// (a.walling 2011-03-11 15:24) - PLID 42786 - May as well use the MaskSSN macro
		if (nMaskPatientSSN == -1) {
			// no masking
			return strField;
		} else {
			return FormatString(
				"dbo.MaskSSN(%s, %li)", strField, nMaskPatientSSN);
		}
	} else {
		return strField;
	}
}

CString GetExportFromClause(ExportRecordType ert)
{
	// (a.walling 2011-03-11 15:24) - PLID 42786 - Need to mask for all exports with patient info rather than simply patient exports!
	// (j.gruber 2010-11-03 15:20) - PLID 40877 - mask SSN
	//BOOL bSSNReadPermission = CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE);
	//BOOL bSSNDisableMasking = CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE);
	//long nMaskPatientSSN = ((bSSNReadPermission && bSSNDisableMasking) ? -1 : (bSSNReadPermission && !bSSNDisableMasking) ? 0 : 1);

	CString strFrom;
	switch(ert) {
	case ertPatients:
		{
			// (j.jones 2012-07-24 10:45) - PLID 51737 - added a preference to only track global periods for
			// surgical codes only, if it is disabled when we would look at all codes
			// (this is cached in CExportDlg)
			long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

			// (j.jones 2012-07-26 15:21) - PLID 51827 - added another preference to NOT track global periods
			// if the charge uses modifier 78
			long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

			//TES 4/30/2008 - PLID 29753 - For some reason, the insurance referral StartDate and EndDate fields were converted
			// to nvarchars, even though their native type is datetime, and the field expects them to be datetime.  I took that out.

			//TES 1/26/2009 - PLID 32852 - The BalanceQ charge amounts were including charges from quotes, I fixed it
			// to filter them out.

			//Guaranteed tables: PatientsT, PersonPat, PrefixPat, PersonProv, PrefixProv, ProvidersT, PersonCoord, PrefixCoord, 
			//GroupTypes, RacePat, PersonRefPhys, PrefixRefPhys, ReferringPhysT, PersonPCP, PrefixPCP, ReferringPhysPCP, 
			//PersonRefPat, PrefixRefPat, PersonLastRefPat, PrefixLastRefPat, LocationsT, CustomContactPerson, PersonPri, 
			//InsuredPartyPri, InsuranceCoPri, PersonPriCo, PersonPriContact, InsurancePlansPri, PersonSec, InsuredPartySec, 
			//InsuranceCoSec, PersonSecCo, PersonSecContact, InsurancePlansSec, PersonRespParty, ResponsiblePartyT,
			//LastPaymentQ, BalanceQ, LastBillQ, GlobalPeriodQ
			// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
			// (j.gruber 2010-08-03 12:32) - PLID 39948 - change copay structure
			//(e.lally 2011-06-17) PLID 43992 - Patient gen. 2 language
			// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
			// (j.jones 2012-07-24 10:45) - PLID 51737 - global periods now optionally filter on surgical codes only
			// (j.jones 2012-07-26 15:14) - PLID 51827 - global periods now optionally exclude modifier 78
			// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
			// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
			// (a.wilson 2014-03-05 11:40) - PLID 61115 - altered for ICD-10 change.
			// (j.jones 2015-02-18 08:45) - PLID 51775 - ensured that global periods ignore voided line items
			strFrom.Format("FROM (SELECT PersonT.* FROM PersonT WITH(NOLOCK) ) AS PersonPat INNER JOIN PatientsT WITH(NOLOCK) ON PersonPat.ID = PatientsT.PersonID "
				"LEFT JOIN PrefixT AS PrefixPat ON PersonPat.PrefixID = PrefixPat.ID "
				"LEFT JOIN (PersonT AS PersonProv INNER JOIN ProvidersT ON PersonProv.ID = ProvidersT.PersonID LEFT JOIN PrefixT AS PrefixProv on PersonProv.PrefixID = PrefixProv.ID) "
				"	ON PatientsT.MainPhysician = PersonProv.ID "
				"LEFT JOIN PersonT AS PersonCoord WITH(NOLOCK) ON PatientsT.EmployeeID = PersonCoord.ID "
				"LEFT JOIN PrefixT AS PrefixCoord ON PersonCoord.PrefixID = PrefixCoord.ID "
				"LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				// (b.spivey, May 28, 2013) - PLID 56892 - Old column is gone, have to pull this information with the new tag table
				" LEFT JOIN ( "
				" SELECT CAST(LEFT(RacePat.Name, LEN(RacePat.Name) -1) AS NVARCHAR(MAX)) AS RaceName, "
				"	CAST(LEFT(CDCRacePat.OfficialRaceName, LEN(CDCRacePat.OfficialRaceName) -1) AS NVARCHAR(MAX)) AS OfficialRaceName, "
				"	Pat.PersonID AS PatientPersonID "
				"	FROM PersonT Per "
				"	INNER JOIN PatientsT Pat ON Per.ID = Pat.PersonID "
				"	CROSS APPLY  "
				"	(  "
				"		SELECT ( "
				"			SELECT RT.Name + ', '  "
			 	"			FROM PersonRaceT PRT  "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			WHERE PRT.PersonID = Pat.PersonID  "
			 	"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) RacePat (Name)  "
				"	CROSS APPLY   "
				"	(   "
				"		SELECT ( "
				"		 	SELECT RCT.OfficialRaceName + ', '   "
				"		 	FROM PersonRaceT PRT   "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID    "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID   "
				"			WHERE PRT.PersonID =  Pat.PersonID  "
				"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) CDCRacePat (OfficialRaceName)  "
				") RaceTextSubQ ON PersonPat.ID	= RaceTextSubQ.PatientPersonID "
				"LEFT JOIN EthnicityT EthnicityPat ON PersonPat.Ethnicity = EthnicityPat.ID "
				"LEFT JOIN LanguageT AS LanguagePat ON PersonPat.LanguageID = LanguagePat.ID "
				"LEFT JOIN (PersonT AS PersonRefPhys INNER JOIN ReferringPhysT ON PersonRefPhys.ID = ReferringPhysT.PersonID "
				"	LEFT JOIN PrefixT AS PrefixRefPhys ON PersonRefPhys.PrefixID = PrefixRefPhys.ID) "
				"	ON PatientsT.DefaultReferringPhyID = PersonRefPhys.ID "
				"LEFT JOIN (PersonT AS PersonPCP INNER JOIN ReferringPhysT AS ReferringPhysPCP ON PersonPCP.ID = ReferringPhysPCP.PersonID "
				"	LEFT JOIN PrefixT AS PrefixPCP ON PersonPCP.PrefixID = PrefixPCP.ID) "
				"	ON PatientsT.PCP = PersonPCP.ID "
				"LEFT JOIN PersonT AS PersonRefPat WITH(NOLOCK) ON PatientsT.ReferringPatientID = PersonRefPat.ID "
				"LEFT JOIN PrefixT AS PrefixRefPat ON PersonRefPat.PrefixID = PrefixRefPat.ID "
				"LEFT JOIN (SELECT PersonT.*, PatientsT.ReferringPatientID FROM PersonT WITH(NOLOCK) INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID WHERE ID IN (SELECT PersonID FROM PatientsT WITH(NOLOCK) WHERE PatientsT.ReferringPatientID Is Not NULL "
				"	AND NOT EXISTS (SELECT PersonID FROM PatientsT AS OthrPatients WITH(NOLOCK) INNER JOIN PersonT AS OthrPerson WITH(NOLOCK) ON OthrPAtients.PersonID = OthrPerson.ID WHERE OthrPatients.ReferringPatientID = PatientsT.ReferringPatientID "
				"	AND OthrPerson.FirstContactDate > PersonT.FirstContactDate))) AS PersonLastRefPat ON "
				"	PersonPat.ID = PersonLastRefPat.ReferringPatientID "
				"	LEFT JOIN PrefixT AS PrefixLastRefPat ON PersonLastRefPat.PrefixID = PrefixLastRefPat.ID "
				"LEFT JOIN LocationsT ON PersonPat.Location = LocationsT.ID "
				"LEFT JOIN (SELECT PersonID, IntParam FROM CustomFieldDataT WHERE FieldID = 31) AS CustomContactLinkQ "
				"	ON PersonPat.ID = CustomContactLinkQ.PersonID "
				"	LEFT JOIN PersonT AS CustomContactPerson WITH(NOLOCK) ON CustomContactLinkQ.IntParam = CustomContactPerson.ID "
				"LEFT JOIN ((SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID WHERE RespTypeID = 1) AS InsuredPartyPri "
				"	INNER JOIN PersonT AS PersonPri WITH(NOLOCK) ON InsuredPartyPri.PersonID = PersonPri.ID "
				"	INNER JOIN (InsuranceCoT AS InsuranceCoPri INNER JOIN PersonT AS PersonPriCo WITH(NOLOCK) ON InsuranceCoPri.PersonID = PersonPriCo.ID) "
				"	ON InsuredPartyPri.InsuranceCoID = InsuranceCoPri.PersonID "
				"	LEFT JOIN InsurancePlansT AS InsurancePlansPri ON InsuredPartyPri.InsPlan = InsurancePlansPri.ID "
				"	LEFT JOIN ((SELECT PersonID, InsuranceCoID FROM InsuranceContactsT WHERE [Default] = 1) AS PriInsuranceContactsQ "
				"		INNER JOIN PersonT AS PersonPriContact WITH(NOLOCK) ON PriInsuranceContactsQ.PersonID = PersonPriContact.ID) "
				"	ON InsuranceCoPri.PersonID = PriInsuranceContactsQ.InsuranceCoID) "
				"	ON PersonPat.ID = InsuredPartyPri.PatientID "
				"LEFT JOIN ((SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
				"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
				" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
				" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID WHERE RespTypeID = 2) AS InsuredPartySec "
				"	INNER JOIN PersonT AS PersonSec WITH(NOLOCK) ON InsuredPartySec.PersonID = PersonSec.ID "
				"	INNER JOIN (InsuranceCoT AS InsuranceCoSec INNER JOIN PersonT AS PersonSecCo WITH(NOLOCK) ON InsuranceCoSec.PersonID = PersonSecCo.ID) "
				"	ON InsuredPartySec.InsuranceCoID = InsuranceCoSec.PersonID "
				"	LEFT JOIN InsurancePlansT AS InsurancePlansSec ON InsuredPartySec.InsPlan = InsurancePlansSec.ID "
				"	LEFT JOIN ((SELECT PersonID, InsuranceCoID FROM InsuranceContactsT WHERE [Default] = 1) AS SecInsuranceContactsQ "
				"		INNER JOIN PersonT AS PersonSecContact WITH(NOLOCK) ON SecInsuranceContactsQ.PersonID = PersonSecContact.ID) "
				"	ON InsuranceCoSec.PersonID = SecInsuranceContactsQ.InsuranceCoID) "
				"	ON PersonPat.ID = InsuredPartySec.PatientID "
				"LEFT JOIN (ResponsiblePartyT INNER JOIN PersonT AS PersonRespParty WITH(NOLOCK) ON ResponsiblePartyT.PersonID = PersonRespParty.ID) "
				"	ON PatientsT.PrimaryRespPartyID = ResponsiblePartyT.PersonID " // (a.walling 2006-10-18 10:22) - PLID 23124 - Only use the primary resp party
				"LEFT JOIN (SELECT LineItemT.PatientID, LineItemT.Date, LineItemT.Amount FROM LineItemT WITH(NOLOCK) "
				"	LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"	LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Date < getdate() AND LineItemT.Type = 1 "
				"	AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"	AND NOT EXISTS "
				"		(SELECT ID FROM LineItemT AS OtherPays WITH(NOLOCK) WHERE OtherPays.PatientID = LineItemT.PatientID AND "
				"		OtherPays.Deleted = 0 AND OtherPays.Type = 1 AND OtherPays.Date < getdate() AND "
				"		(OtherPays.Date > LineItemT.Date OR (OtherPays.Date = LineItemT.Date AND OtherPays.ID > LineItemT.ID)) "
				"	) "
				") AS LastPaymentQ ON PersonPat.ID = LastPaymentQ.PatientID "
				"LEFT JOIN (SELECT COALESCE((SELECT Sum(ChargeRespT.Amount) "
				"		FROM ChargeRespT INNER JOIN LineItemT WITH(NOLOCK) ON ChargeRespT.ChargeID = LineItemT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE ChargeRespT.InsuredPartyID Is Null AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND LineItemT.PatientID = PatientsT.PersonID),0) AS TotalPatCharges, "
				"	COALESCE((SELECT Sum(ChargeRespT.Amount) "
				"		FROM ChargeRespT INNER JOIN LineItemT WITH(NOLOCK) ON ChargeRespT.ChargeID = LineItemT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE ChargeRespT.InsuredPartyID IN "
				"			(SELECT PersonID FROM InsuredPartyT "
				"			WHERE RespTypeID = 1 AND InsuredPartyT.PatientID = LineItemT.PatientID) "
				"		AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND LineItemT.PatientID = PatientsT.PersonID),0) AS TotalPriInsCharges, "
				"	COALESCE((SELECT Sum(ChargeRespT.Amount) "
				"		FROM ChargeRespT INNER JOIN LineItemT WITH(NOLOCK) ON ChargeRespT.ChargeID = LineItemT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE ChargeRespT.InsuredPartyID IN "
				"			(SELECT PersonID FROM InsuredPartyT "
				"			WHERE RespTypeID = 2 AND InsuredPartyT.PatientID = LineItemT.PatientID) "
				"		AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND LineItemT.PatientID = PatientsT.PersonID),0) AS TotalSecInsCharges, "
				"	COALESCE((SELECT Sum(ChargeRespT.Amount) "
				"		FROM ChargeRespT INNER JOIN LineItemT WITH(NOLOCK) ON ChargeRespT.ChargeID = LineItemT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE ChargeRespT.InsuredPartyID IN "
				"			(SELECT PersonID FROM InsuredPartyT "
				"			WHERE RespTypeID NOT IN (1,2) AND InsuredPartyT.PatientID = LineItemT.PatientID) "
				"		AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND LineItemT.PatientID = PatientsT.PersonID),0) AS TotalOthInsCharges, "
				"	COALESCE((SELECT Sum(LineItemT.Amount) FROM LineItemT WITH(NOLOCK) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE LineItemT.Type IN (1,2,3) AND LineItemT.Deleted = 0 AND LineItemT.PatientID = PatientsT.PersonID "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND PaymentsT.InsuredPartyID = -1),0) AS TotalPatPays, "
				"	COALESCE((SELECT Sum(LineItemT.Amount) FROM LineItemT WITH(NOLOCK) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE LineItemT.Type IN (1,2,3) AND LineItemT.Deleted = 0 AND LineItemT.PatientID = PatientsT.PersonID "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND PaymentsT.InsuredPartyID IN "
				"			(SELECT PersonID FROM InsuredPartyT "
				"			WHERE RespTypeID = 1 AND InsuredPartyT.PatientID = LineItemT.PatientID) "
				"		),0) AS TotalPriPays, "
				"	COALESCE((SELECT Sum(LineItemT.Amount) FROM LineItemT WITH(NOLOCK) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE LineItemT.Type IN (1,2,3) AND LineItemT.Deleted = 0 AND LineItemT.PatientID = PatientsT.PersonID "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND PaymentsT.InsuredPartyID IN "
				"			(SELECT PersonID FROM InsuredPartyT "
				"			WHERE RespTypeID = 2 AND InsuredPartyT.PatientID = LineItemT.PatientID) "
				"		),0) AS TotalSecPays, "
				"	COALESCE((SELECT Sum(LineItemT.Amount) FROM LineItemT WITH(NOLOCK) INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE LineItemT.Type IN (1,2,3) AND LineItemT.Deleted = 0 AND LineItemT.PatientID = PatientsT.PersonID "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND PaymentsT.InsuredPartyID IN "
				"			(SELECT PersonID FROM InsuredPartyT "
				"			WHERE RespTypeID NOT IN (1,2) AND InsuredPartyT.PatientID = LineItemT.PatientID) "
				"		),0) AS TotalOthPays, PatientsT.PersonID "
				"	FROM PatientsT) AS BalanceQ ON PersonPat.ID = BalanceQ.PersonID "
				"LEFT JOIN (SELECT BillsT.ID, BillsT.PatientID, BillDiagCodes.ICD9Codes AS ICD9CodeList, "
				"	BillDiagCodes.ICD10Codes AS ICD10CodeList, BillDiagCodes.ICDAllCodes AS AllCodeList"
				"	FROM BillsT "
				"	INNER JOIN BillDiagCodeNumberListV BillDiagCodes ON BillsT.ID = BillDiagCodes.BillID "
				"	LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
				"	WHERE BillsT.Deleted = 0 AND BillsT.Date < getdate() AND BillsT.EntryType = 1 "
				"	AND BillCorrectionsT.ID IS NULL "
				"	AND NOT EXISTS "
				"		(SELECT ID FROM BillsT OtherBills WHERE OtherBills.PatientID = BillsT.PatientID AND "
				"		OtherBills.Deleted = 0 AND OtherBills.EntryType = 1 AND OtherBills.Date < getdate() AND "
				"		(OtherBills.Date > BillsT.Date OR (OtherBills.Date = BillsT.Date AND OtherBills.ID > BillsT.ID)) "
				"	) "
				") AS LastBillQ ON PersonPat.ID = LastBillQ.PatientID "
				"LEFT JOIN (SELECT CPTCodeT.Code AS Code, ServiceT.Name, dbo.AsDateNoTime(LineItemT.Date) AS Date, CPTCodeT.GlobalPeriod, "
				"	DATEADD(day,GlobalPeriod,dbo.AsDateNoTime(LineItemT.Date)) AS ExpDate, (CASE WHEN (DATEADD(day,GlobalPeriod,dbo.AsDateNoTime(LineItemT.Date)) > GetDate()) THEN 0 ELSE 1 END) AS Expired, LineItemT.PatientID "
				"	FROM ChargesT INNER JOIN LineItemT WITH(NOLOCK) ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"	INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
				"	INNER JOIN PersonT WITH(NOLOCK) ON LineItemT.PatientID = PersonT.ID "
				"	LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"	LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"	WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 "
				"	AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"	AND DATEADD(day,GlobalPeriod,dbo.AsDateNoTime(LineItemT.Date)) > GetDate() AND LineItemT.ID IN "
				"		(SELECT Max(LineItemT.ID) AS ID FROM ChargesT INNER JOIN LineItemT WITH(NOLOCK) ON ChargesT.ID = LineItemT.ID "
				"		INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
				"		INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
				"		LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
				"		LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID "
				"		LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID "
				"		WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND CPTCodeT.GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 "
				"		AND VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL "
				"		AND LineItemT.PatientID = PersonT.ID AND DATEADD(day,GlobalPeriod,dbo.AsDateNoTime(LineItemT.Date)) > GetDate() "
				"		AND (%li <> 1 OR ServicePayGroupsT.Category = %li) "
				"		AND (%li <> 1 OR (Coalesce(ChargesT.CPTModifier, '') <> '78' AND Coalesce(ChargesT.CPTModifier2, '') <> '78' AND Coalesce(ChargesT.CPTModifier3, '') <> '78' AND Coalesce(ChargesT.CPTModifier4, '') <> '78')) "
				")) AS GlobalPeriodQ ON PersonPat.ID = GlobalPeriodQ.PatientID "
				"LEFT JOIN (SELECT InsuranceReferralsT.ID, InsuredPartyT.PatientID, AuthNum, StartDate, EndDate, NumVisits FROM InsuranceReferralsT "
				"	INNER JOIN InsuredPartyT ON InsuranceReferralsT.InsuredPartyID = InsuredPartyT.PersonID "
				"	WHERE InsuranceReferralsT.ID IN "
				"		(SELECT Max(InsuranceReferralsT.ID) AS MaxID FROM InsuranceReferralsT "
				"		LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
				"		GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
				"		INNER JOIN InsuredPartyT ON InsuranceReferralsT.InsuredPartyID = InsuredPartyT.PersonID "
				"		WHERE StartDate <= GetDate() AND GetDate() < DATEADD(day,1,EndDate) AND NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) "
				"		GROUP BY InsuredPartyT.PatientID)"
				"	) AS ActiveInsReferralQ ON PersonPat.ID = ActiveInsReferralQ.PatientID ",
				nSurgicalCodesOnly, PayGroupCategory::SurgicalCode, nIgnoreModifier78);
		}
		break;

	case ertAppointments:
		{
			//Guaranteed tables: AppointmentsT, PatientsT, PersonPat, PrefixPat, GroupTypes, RacePat, AptTypeT, AptShowStateT,
			//AppointmentLocationT, PersonRefPhys, PrefixRefPhys, ReferringPhysT
			// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
			//(e.lally 2011-06-17) PLID 43992 - Patient gen. 2 language
			// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
			// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
			strFrom = "FROM AppointmentsT WITH(NOLOCK) "
				"LEFT JOIN (PersonT AS PersonPat WITH(NOLOCK) INNER JOIN PatientsT WITH(NOLOCK) ON PersonPat.ID = PatientsT.PersonID "
				"	LEFT JOIN PrefixT AS PrefixPat ON PersonPat.PrefixID = PrefixPat.ID "
				"	LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				// (b.spivey, May 28, 2013) - PLID 56892 - Old column is gone, have to pull this information with the new tag table
				" LEFT JOIN ( "
				" SELECT CAST(LEFT(RacePat.Name, LEN(RacePat.Name) -1) AS NVARCHAR(MAX)) AS RaceName, "
				"	CAST(LEFT(CDCRacePat.OfficialRaceName, LEN(CDCRacePat.OfficialRaceName) -1) AS NVARCHAR(MAX)) AS OfficialRaceName, "
				"	Pat.PersonID AS PatientPersonID "
				"	FROM PersonT Per "
				"	INNER JOIN PatientsT Pat ON Per.ID = Pat.PersonID "
				"	CROSS APPLY  "
				"	(  "
				"		SELECT ( "
				"			SELECT RT.Name + ', '  "
			 	"			FROM PersonRaceT PRT  "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			WHERE PRT.PersonID = Pat.PersonID  "
			 	"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) RacePat (Name)  "
				"	CROSS APPLY   "
				"	(   "
				"		SELECT ( "
				"		 	SELECT RCT.OfficialRaceName + ', '   "
				"		 	FROM PersonRaceT PRT   "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID    "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID   "
				"			WHERE PRT.PersonID =  Pat.PersonID  "
				"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) CDCRacePat (OfficialRaceName)  "
				") RaceTextSubQ ON PersonPat.ID	= RaceTextSubQ.PatientPersonID  "
				"	LEFT JOIN EthnicityT EthnicityPat ON PersonPat.Ethnicity = EthnicityPat.ID "
				"	LEFT JOIN LanguageT AS LanguagePat ON PersonPat.LanguageID = LanguagePat.ID "
				"	LEFT JOIN (PersonT AS PersonRefPhys WITH(NOLOCK) INNER JOIN ReferringPhysT WITH(NOLOCK) ON PersonRefPhys.ID = ReferringPhysT.PersonID "
				"	LEFT JOIN PrefixT AS PrefixRefPhys ON PersonRefPhys.PrefixID = PrefixRefPhys.ID) "
				"	ON PatientsT.DefaultReferringPhyID = PersonRefPhys.ID) "
				"ON AppointmentsT.PatientID = PersonPat.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"INNER JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
				"INNER JOIN LocationsT AS AppointmentLocationT ON AppointmentsT.LocationID = AppointmentLocationT.ID";
		}
		break;

	case ertCharges:
		{
			//Guaranteed tables: ChargesT, LineChargeT, BillsT, PatientsT, PersonPat, PrefixPat, GroupTypes, RacePat,
			//BillPlaceOfServiceT, PlaceOfServiceCodesT, CptCodeT, CptModifierT, CptModifierT2, BillLocationT,
			//CptModifierT3, CptModifierT4
			//DRT 6/18/2007 - PLID 25333 - Added provider tables to the query - ProvidersT, PersonProv, PrefixProv
			//TES 5/9/2008 - PLID 27177 - Added ChargeCategoriesT, ServiceT
			// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
			//(e.lally 2011-06-17) PLID 43992 - Patient gen. 2 language
			// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
			// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
			// (a.walling 2014-02-28 17:35) - PLID 61126 - BillDiagCodeT - ExportUtils. Now using BillDiagCodeFlat12V
			// (a.wilson 2014-03-05 12:10) - PLID 61115 - join ChargeWhichCodesFlatV for ICD-10 Change.
			strFrom = "FROM ChargesT WITH(NOLOCK) "
				"INNER JOIN LineItemT AS LineChargeT WITH(NOLOCK) ON ChargesT.ID = LineChargeT.ID "
				"INNER JOIN BillsT WITH(NOLOCK) ON ChargesT.BillID = BillsT.ID "
				"INNER JOIN LocationsT AS BillPlaceOfServiceT ON BillsT.Location = BillPlaceOfServiceT.ID "
				"LEFT JOIN BillDiagCodeFlat12V ON BillsT.ID = BillDiagCodeFlat12V.BillID "
				"LEFT JOIN PlaceOfServiceCodesT ON BillPlaceOfServiceT.POSID = PlaceOfServiceCodesT.ID "
				"LEFT JOIN (PersonT AS PersonPat INNER JOIN PatientsT WITH(NOLOCK) ON PersonPat.ID = PatientsT.PersonID "
				"	LEFT JOIN PrefixT AS PrefixPat ON PersonPat.PrefixID = PrefixPat.ID "
				"	LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				// (b.spivey, May 28, 2013) - PLID 56892 -  Old column is gone, have to pull this information with the new tag table
				" LEFT JOIN ( "
				" SELECT CAST(LEFT(RacePat.Name, LEN(RacePat.Name) -1) AS NVARCHAR(MAX)) AS RaceName, "
				"	CAST(LEFT(CDCRacePat.OfficialRaceName, LEN(CDCRacePat.OfficialRaceName) -1) AS NVARCHAR(MAX)) AS OfficialRaceName, "
				"	Pat.PersonID AS PatientPersonID "
				"	FROM PersonT Per "
				"	INNER JOIN PatientsT Pat ON Per.ID = Pat.PersonID "
				"	CROSS APPLY  "
				"	(  "
				"		SELECT ( "
				"			SELECT RT.Name + ', '  "
			 	"			FROM PersonRaceT PRT  "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			WHERE PRT.PersonID = Pat.PersonID  "
			 	"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) RacePat (Name)  "
				"	CROSS APPLY   "
				"	(   "
				"		SELECT ( "
				"		 	SELECT RCT.OfficialRaceName + ', '   "
				"		 	FROM PersonRaceT PRT   "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID    "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID   "
				"			WHERE PRT.PersonID =  Pat.PersonID  "
				"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) CDCRacePat (OfficialRaceName)  "
				") RaceTextSubQ ON PersonPat.ID	= RaceTextSubQ.PatientPersonID "
				"	LEFT JOIN EthnicityT EthnicityPat ON PersonPaT.Ethnicity = EthnicityPaT.ID "
				"	LEFT JOIN LanguageT AS LanguagePat ON PersonPat.LanguageID = LanguagePat.ID "
				") "
				"ON LineChargeT.PatientID = PersonPat.ID "
				"LEFT JOIN CptCodeT ON ChargesT.ServiceID = CptCodeT.ID "
				"LEFT JOIN ServiceT ON ChargesT.ServiceID = ServiceT.ID "
				"LEFT JOIN CategoriesT AS ChargeCategoriesT ON ServiceT.Category = ChargeCategoriesT.ID "
				"LEFT JOIN CptModifierT ON ChargesT.CptModifier = CptModifierT.Number "
				"LEFT JOIN CptModifierT AS CptModifierT2 ON ChargesT.CptModifier2 = CptModifierT2.Number "
				"LEFT JOIN CptModifierT AS CptModifierT3 ON ChargesT.CptModifier3 = CptModifierT3.Number "
				"LEFT JOIN CptModifierT AS CptModifierT4 ON ChargesT.CptModifier4 = CptModifierT4.Number "
				"LEFT JOIN LocationsT AS BillLocationT ON LineChargeT.LocationID = BillLocationT.ID "
				"LEFT JOIN ProvidersT ON ChargesT.DoctorsProviders = ProvidersT.PersonID "
				"LEFT JOIN PersonT PersonProv WITH(NOLOCK) ON ProvidersT.PersonID = PersonProv.ID "
				"LEFT JOIN PrefixT PrefixProv ON PersonProv.PrefixID = PrefixProv.ID "
				"LEFT JOIN ChargeWhichCodesFlatV ChargeWhichCodes ON ChargesT.ID = ChargeWhichCodes.ChargeID ";
			break;
		}

	case ertPayments:
		{
			// (e.lally 2007-07-09) PLID 25993 - Added a  join to CreditCardNamesT
			//Guaranteed tables: PaymentsT, LinePayT, PatientsT, PersonPat, PrefixPat, GroupTypes, RacePat, PaymentProv,
			//ProvidersT, PrefixProv, PaymentPlansT, PayInsPartyT, PayInsCoT
			// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
			//(e.lally 2011-06-17) PLID 43992 - Patient gen. 2 language
			// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
			// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
			strFrom = "FROM PaymentsT INNER JOIN LineItemT AS LinePayT WITH(NOLOCK) ON PaymentsT.ID = LinePayT.ID "
				"LEFT JOIN (PersonT AS PersonPat WITH(NOLOCK) INNER JOIN PatientsT WITH(NOLOCK) ON PersonPat.ID = PatientsT.PersonID "
				"	LEFT JOIN PrefixT AS PrefixPat ON PersonPat.PrefixID = PrefixPat.ID "
				"	LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				// (b.spivey, May 28, 2013) - PLID 56892 - Old column is gone, have to pull this information with the new tag table
				" LEFT JOIN ( "
				" SELECT CAST(LEFT(RacePat.Name, LEN(RacePat.Name) -1) AS NVARCHAR(MAX)) AS RaceName, "
				"	CAST(LEFT(CDCRacePat.OfficialRaceName, LEN(CDCRacePat.OfficialRaceName) -1) AS NVARCHAR(MAX)) AS OfficialRaceName, "
				"	Pat.PersonID AS PatientPersonID "
				"	FROM PersonT Per "
				"	INNER JOIN PatientsT Pat ON Per.ID = Pat.PersonID "
				"	CROSS APPLY  "
				"	(  "
				"		SELECT ( "
				"			SELECT RT.Name + ', '  "
				"			FROM PersonRaceT PRT  "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			WHERE PRT.PersonID = Pat.PersonID  "
				"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) RacePat (Name)  "
				"	CROSS APPLY   "
				"	(   "
				"		SELECT ( "
				"		 	SELECT RCT.OfficialRaceName + ', '   "
				"		 	FROM PersonRaceT PRT   "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID    "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID   "
				"			WHERE PRT.PersonID =  Pat.PersonID  "
				"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) CDCRacePat (OfficialRaceName)  "
				") RaceTextSubQ ON PersonPat.ID	= RaceTextSubQ.PatientPersonID 	"
				"	LEFT JOIN EthnicityT EthnicityPat ON PersonPat.Ethnicity = EthnicityPat.ID "
				"	LEFT JOIN LanguageT AS LanguagePat ON PersonPat.LanguageID = LanguagePat.ID "
				") "
				"ON LinePayT.PatientID = PersonPat.ID "
				"LEFT JOIN PersonT AS PaymentProv WITH(NOLOCK) ON PaymentsT.ProviderID = PaymentProv.ID "
				"LEFT JOIN ProvidersT ON PaymentProv.ID = ProvidersT.PersonID "
				"LEFT JOIN PrefixT AS PrefixProv ON PaymentProv.PrefixID = PrefixProv.ID "
				"LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
				"LEFT JOIN CreditCardNamesT ON PaymentPlansT.CreditCardID = CreditCardNamesT.ID "
				"LEFT JOIN InsuredPartyT AS PayInsPartyT ON PaymentsT.InsuredPartyID = PayInsPartyT.PersonID "
				"LEFT JOIN InsuranceCoT AS PayInsCoT ON PayInsPartyT.InsuranceCoID = PayInsCoT.PersonID ";
			break;
		}

	case ertEMNs:
		{
			//Guaranteed tables: EmrMasterT, PatientsT, PersonPat, PrefixPat, GroupTypes, RacePat, 
			//PrefixProv, LocEmr, EmrGroupsT, EmrCollectionT
			// (j.jones 2009-10-19 10:58) - PLID 35994 - race and ethnicity are now separate fields
			// (z.manning 2011-05-24 09:21) - PLID 33114 - Added left join of EmnTabChartsLinkT
			//(e.lally 2011-06-17) PLID 43992 - Patient gen. 2 language
			// (j.jones 2011-07-05 17:24) - PLID 44432 - supported custom statuses
			// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
			// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
			// (a.wilson 2014-03-05 16:43) - PLID 60874 - created new view to get diagnosis code lists.
			strFrom = "FROM (SELECT * FROM EmrMasterT WITH(NOLOCK) WHERE Deleted = 0) AS EmrMasterT LEFT JOIN (PersonT AS PersonPat WITH(NOLOCK) "
				"	INNER JOIN PatientsT WITH(NOLOCK) ON PersonPat.ID = PatientsT.PersonID "
				"	LEFT JOIN PrefixT AS PrefixPat ON PersonPat.PrefixID = PrefixPat.ID "
				"	LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				// (b.spivey, May 28, 2013) - PLID 56892 - Old column is gone, have to pull this information with the new tag table
				" LEFT JOIN ( "
				" SELECT CAST(LEFT(RacePat.Name, LEN(RacePat.Name) -1) AS NVARCHAR(MAX)) AS RaceName, "
				"	CAST(LEFT(CDCRacePat.OfficialRaceName, LEN(CDCRacePat.OfficialRaceName) -1) AS NVARCHAR(MAX)) AS OfficialRaceName, "
				"	Pat.PersonID AS PatientPersonID "
				"	FROM PersonT Per "
				"	INNER JOIN PatientsT Pat ON Per.ID = Pat.PersonID "
				"	CROSS APPLY  "
				"	(  "
				"		SELECT ( "
				"			SELECT RT.Name + ', '  "
			 	"			FROM PersonRaceT PRT  "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID  "
				"			WHERE PRT.PersonID = Pat.PersonID  "
			 	"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) RacePat (Name)  "
				"	CROSS APPLY   "
				"	(   "
				"		SELECT ( "
				"		 	SELECT RCT.OfficialRaceName + ', '   "
				"		 	FROM PersonRaceT PRT   "
				"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID    "
				"			INNER JOIN RaceCodesT RCT ON RCT.ID = RT.RaceCodeID   "
				"			WHERE PRT.PersonID =  Pat.PersonID  "
				"			FOR XML PATH(''), TYPE   "
				"		).value('/', 'nvarchar(max)') "
				"	) CDCRacePat (OfficialRaceName)  "
				") RaceTextSubQ ON PersonPat.ID	= RaceTextSubQ.PatientPersonID 	"
				"	LEFT JOIN EthnicityT EthnicityPat ON PersonPat.Ethnicity = EthnicityPat.ID "
				"	LEFT JOIN LanguageT AS LanguagePat ON PersonPat.LanguageID = LanguagePat.ID "
				") "
				"ON EmrMasterT.PatientID = PersonPat.ID "
				"LEFT JOIN LocationsT AS LocEMR ON EmrMasterT.LocationID = LocEMR.ID "
				"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				"INNER JOIN EmrGroupsT WITH(NOLOCK) ON EmrMasterT.EmrGroupID = EmrGroupsT.ID "
				"LEFT JOIN EmrCollectionT ON EmrMasterT.EmrCollectionID = EmrCollectionT.ID "
				"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				"LEFT JOIN EMRDiagCodeNumberListV EMRDiagCodes ON EmrMasterT.ID = EMRDiagCodes.EMRID ";
			break;
		}

	case ertHistory: // (z.manning 2009-12-11 12:42) - PLID 36519
		{
			strFrom = 
				"FROM MailSent WITH(NOLOCK) \r\n"
				"LEFT JOIN PersonT WITH(NOLOCK) ON MailSent.PersonID = PersonT.ID \r\n"
				"LEFT JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID \r\n";
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return strFrom;
}

CString GetBaseWhereClause(ExportRecordType ertBasedOn)
{
	CString strWhere;
	switch(ertBasedOn) {
	case ertPatients:
		//TES 6/5/2007 - PLID 26125 - Filter out inquiries.
		strWhere = "(PatientsT.PersonID <> -25 AND PatientsT.CurrentStatus IN (1,2,3))";
		break;
	case ertAppointments:
		//TES 6/5/2007 - PLID 26233 - Filter out cancelled appointments.
		strWhere = "AppointmentsT.Status <> 4";
		break;
	case ertCharges:
		strWhere = "(LineChargeT.Deleted = 0 AND LineChargeT.Type = 10)";
		break;
	case ertPayments:
		strWhere = "(LinePayT.Deleted = 0 AND LinePayT.Type IN (1,2,3))";
		break;
	case ertEMNs:
		// (z.manning 2011-05-24 09:15) - PLID 33114 - This also needs to factor in EMR charting permissions
		strWhere = "EmrMasterT.Deleted = 0 " + GetEmrChartPermissionFilter().Flatten();
		break;
	case ertHistory: 
		// (z.manning 2009-12-11 10:14) - PLID 36519 - Only export files (something with
		// a path) and skip documents that aren't associated with a single person (i.e. don't
		// export multi-patient docs). Also, do not export any attached folders.
		strWhere = FormatString("MailSent.PathName <> '' AND COALESCE(MailSent.PersonID, -25) <> -25 "
			"AND MailSent.Selection NOT IN ('%s')"
			, SELECTION_FOLDER);
		break;
	default:
		ASSERT(FALSE);
		break;
	}
	return strWhere;
}
	
CString GetNthField(const CString &strFull, int n, CString strDefault /*= ""*/)
{
	//TES 6/4/2007 - PLID 26160 - I fixed all the string searching in this function to use FindDelimiter() and 
	// ReadFromDelimitedField(), so that | and \ embedded in the fields would be properly parsed.
	int nFirstPipe = FindDelimiter(strFull, '|', '\\');
	if(nFirstPipe == -1) {
		if(n == 0) return strFull == "" ? strDefault : ReadFromDelimitedField(strFull, '|', '\\');
		else return strDefault;
	}
	
	if(n == 0) return strFull.Left(nFirstPipe);
	
	int i = 1;
	int nCurrentPipe = nFirstPipe;
	int nNextPipe = FindDelimiter(strFull, '|', '\\', nCurrentPipe+1);
	while(i < n) {
		if(nNextPipe == -1) {
			nCurrentPipe = -1;
		}
		else {
			nCurrentPipe = nNextPipe;
			nNextPipe = FindDelimiter(strFull, '|', '\\', nCurrentPipe+1);
		}
		i++;
	}

	if(nCurrentPipe == -1) {
		return strDefault;
	}
	else if(nNextPipe == -1) {
		return ReadFromDelimitedField(strFull.Right(strFull.GetLength()-nCurrentPipe-1), '|', '\\');
	}
	else {
		return ReadFromDelimitedField(strFull.Mid(nCurrentPipe+1, nNextPipe-nCurrentPipe-1), '|', '\\');
	}
}

CString FormatExportField(_variant_t varField, ExportFieldType eft, const CString &strFormat, bool bFixedWidth, CArray<SpecialChar,SpecialChar> &arSpecialChars)
{
	//First, do the formatting, then if it's fixed width handle that.
	CString strOutput;
	int nFixedWidthLength = 0;
	BOOL bRightJustified = FALSE;
	CString strFillCharacter;
	switch(eft) {
	case eftPlaceholder:
		//Length|Right-justified|fill character|Text for the placeholder
		//5     |0              |_             |""
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "5"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";

		strOutput = GetNthField(strFormat, 3, "");
		break;
		
	case eftGenericText:
	case eftGenericNtext:
		//Length|Right-justified|fill character|Capitalized
		//10    |0              |_             |0
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "10"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";

		strOutput = VarString(varField, "");
		if(GetNthField(strFormat, 3, "0") == "1") strOutput.MakeUpper();
		break;

	case eftGenericNumber:
		{
			//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()
			//10    |1              |0             |0     |2          |.                |0

			nFixedWidthLength = atoi(GetNthField(strFormat, 0, "10"));
			bRightJustified = (GetNthField(strFormat, 1, "1") == "1");
			strFillCharacter = GetNthField(strFormat, 2, "0");
			if(strFillCharacter == "") strFillCharacter = "0";
			
			//It's one of the numeric types.
			if(varField.vt == VT_NULL) {
				strOutput = "";
				break;
			}
			double dValue;
			switch(varField.vt) {
			case VT_I4:
				dValue = VarLong(varField);
				break;
			case VT_UI4:
				dValue = VarULong(varField);
				break;
			case VT_I2:
				dValue = VarShort(varField);
				break;
			case VT_UI2:
				dValue = VarUShort(varField);
				break;
			case VT_R4:
				dValue = VarFloat(varField);
				break;
			case VT_R8:
				dValue = VarDouble(varField);
				break;
			default:
				ASSERT(FALSE);
				dValue = 0.0;
				break;
			}
			CString strFmt;
			if(GetNthField(strFormat, 3, "0") == "1") {
				strFmt = "%.0" + GetNthField(strFormat, 4, "2") + "f";
				strOutput.Format(strFmt, dValue);
			}
			else {
				strFmt = "%f";
				strOutput.Format(strFmt, dValue);
				if(strOutput.Find(".") != -1) {
					while(strOutput.Right(1) == "0") strOutput = strOutput.Left(strOutput.GetLength()-1);
					if(strOutput.Right(1) == ".") strOutput = strOutput.Left(strOutput.GetLength()-1);
				}
			}
			strOutput.Replace(".", GetNthField(strFormat, 5, "."));
			if(strOutput.Left(1) == "-" && GetNthField(strFormat, 6, "0") == "1") {
				strOutput = "(" + strOutput.Mid(1) + ")";
			}
		}
		break;

	case eftDateTime:
		//Length|Right-justified|Fill character|Format
		//10    |0              |_             |%c
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "10"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";
		if(varField.vt == VT_NULL) strOutput = "";
		else strOutput = VarDateTime(varField).Format(GetNthField(strFormat, 3, "%c"));
		break;

	case eftCurrency:
		{
			//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character|Use ()|Currency Symbol|Symbol Placement
			//10    |1              |_             |1     |2          |.                |1     |$              |1
			nFixedWidthLength = atoi(GetNthField(strFormat, 0, "10"));
			bRightJustified = (GetNthField(strFormat, 1, "1") == "1");
			strFillCharacter = GetNthField(strFormat, 2, " ");
			if(strFillCharacter == "") strFillCharacter = " ";

			if(varField.vt == VT_NULL) {
				strOutput = "";
				break;
			}
			COleCurrency cy = VarCurrency(varField);
			CString strCy;
			if(cy.m_cur.int64 % (__int64)10000 < 0) {
				if(cy.m_cur.int64 / (__int64)10000 == 0) {
					strCy.Format("-0.%0.04I64i", (__int64)-1 * cy.m_cur.int64 % (__int64)10000);
				}
				else {
					strCy.Format("%I64i.%0.04I64i", cy.m_cur.int64 / (__int64)10000, (__int64)-1 * cy.m_cur.int64 % (__int64)10000);
				}
			}
			else {
				strCy.Format("%I64i.%0.04I64i", cy.m_cur.int64 / (__int64)10000, cy.m_cur.int64 % (__int64)10000);
			}
			double dValue = atof(strCy);
			CString strFmt;
			if(GetNthField(strFormat, 3, "1") == "1") {
				strFmt = "%.0" + GetNthField(strFormat, 4, "2") + "f";
				strOutput.Format(strFmt, dValue);
			}
			else {
				strFmt = "%f";
				strOutput.Format(strFmt, dValue);
				if(strOutput.Find(".") != -1) {
					while(strOutput.Right(1) == "0") strOutput = strOutput.Left(strOutput.GetLength()-1);
					if(strOutput.Right(1) == ".") strOutput = strOutput.Left(strOutput.GetLength()-1);
				}
			}
			strOutput.Replace(".", GetNthField(strFormat, 5, "."));
			if(strOutput.Left(1) == "-" && GetNthField(strFormat, 6, "1") == "1") {
				strOutput = "(" + strOutput.Mid(1) + ")";
			}
			CString strSymbol = GetNthField(strFormat, 7, "$");
			int nPlacement = atoi(GetNthField(strFormat, 8, "1"));
			switch(nPlacement) {
			case 0:
				strOutput = strSymbol + strOutput;
				break;
			case 1:
				if(strOutput.Left(1) == "-" || strOutput.Left(1) == "(") {
					strOutput = strOutput.Left(1) + strSymbol + strOutput.Mid(1);
				}
				else {
					strOutput = strSymbol + strOutput;
				}
				break;
			case 2:
				if(strOutput.Right(1) == ")") {
					strOutput = strOutput.Left(strOutput.GetLength()-1) + strSymbol + strOutput.Right(1);
				}
				else {
					strOutput += strSymbol;
				}
				break;
			case 3:
				strOutput += strSymbol;
				break;
			default:
				ASSERT(FALSE);
				break;
			}
		}
		break;

	case eftPhoneNumber:
		{
			//Length|Right-justified|Fill character|Format
			//14    |0              |_             |(###) ###-####
			nFixedWidthLength = atoi(GetNthField(strFormat, 0, "14"));
			bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
			strFillCharacter = GetNthField(strFormat, 2, " ");
			if(strFillCharacter == "") strFillCharacter = " ";
			CString strField = VarString(varField, "");
			if(strField.IsEmpty()) {
				strOutput = "";
			}
			else {
				strOutput = FormatPhone(strField, GetNthField(strFormat, 3, "(###) ###-####"));
			}
		}
		break;

	case eftSSN:
		//Length|Right-justified|Fill character|Include hyphens
		//11    |0              |_             |1
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "11"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";
		strOutput = VarString(varField, "");
		if(GetNthField(strFormat, 3, "1") != "1") strOutput.Replace("-", "");
		break;

	case eftBool:
		//Length|Right-justified|Fill character|True value|False value|Unknown value
		//1     |0              |_             |T         |F          |U
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "1"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";
		if(varField.vt == VT_NULL) {
			strOutput = GetNthField(strFormat, 5, "U");
		}
		else if(VarBool(varField)) {
			strOutput = GetNthField(strFormat, 3, "T");
		}
		else {
			strOutput = GetNthField(strFormat, 4, "F");
		}
		break;

	case eftGender:
		//Length|Right-justified|Fill character|Male value|Female value|Unknown value
		//1     |0              |_             |M         |F           |U
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "1"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";
		if(varField.vt == VT_NULL) {
			strOutput = GetNthField(strFormat, 5, "U");
		}
		else if(VarByte(varField) == 1) {
			strOutput = GetNthField(strFormat, 3, "M");
		}
		else if(VarByte(varField) == 2) {
			strOutput = GetNthField(strFormat, 4, "F");
		}
		else {
			strOutput = GetNthField(strFormat, 5, "U");
		}
		break;
	
	case eftMarital:
		//Length|Right-justified|Fill character|Single value|Married value|Unknown value
		//1     |0              |_             |S           |M            |U
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "1"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";
		if(varField.vt == VT_NULL) {
			strOutput = GetNthField(strFormat, 5, "U");
		}
		else if(VarString(varField) == "1") {
			strOutput = GetNthField(strFormat, 3, "S");
		}
		else if(VarString(varField) == "2") {
			strOutput = GetNthField(strFormat, 4, "M");
		}
		else {
			strOutput = GetNthField(strFormat, 5, "U");
		}
		break;

	case eftDiag:
		{
			//Length|Right-justified|Fill character|Fixed?|# of places|Decimal character
			//6     |0              |_             |0     |2          |.
			nFixedWidthLength = atoi(GetNthField(strFormat, 0, "6"));
			bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
			strFillCharacter = GetNthField(strFormat, 2, " ");
			if(strFillCharacter == "") strFillCharacter = " ";
			
			strOutput = VarString(varField, "");
			if(strOutput != "") {
				int nDec = strOutput.Find(".");
				CString strPreDec, strPostDec;
				if(nDec == -1) {
					strPreDec = strOutput;
					strPostDec = "";
				}
				else {
					strPreDec = strOutput.Left(nDec);
					strPostDec = strOutput.Mid(nDec+1);
				}
				if(GetNthField(strFormat, 3, "0") == "1") {
					int nPlaces = atoi(GetNthField(strFormat, 4, "2"));
					if(nPlaces == 0) {
						strOutput = strPreDec;
					}
					else {
						strOutput = strPreDec + GetNthField(strFormat, 5, ".");
						for(int i = 0; i < nPlaces; i++) {
							if(strPostDec.GetLength() > i) {
								strOutput += strPostDec.GetAt(i);
							}
							else {
								strOutput += "0";
							}
						}
					}
				}
				else {
					if(strPostDec == "") {
						strOutput = strPreDec;
					}
					else {
						strOutput = strPreDec + GetNthField(strFormat, 5, ".") + strPostDec;
					}
				}
			}
		}
		break;

	case eftAdvanced:
		//Length|Right-justified|fill character|Field
		nFixedWidthLength = atoi(GetNthField(strFormat, 0, "5"));
		bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
		strFillCharacter = GetNthField(strFormat, 2, " ");
		if(strFillCharacter == "") strFillCharacter = " ";

		strOutput = AsString(varField);
		break;
		
	case eftEmnItem:
		{
			//Length|Right-justified|fill character|Sentence format
			nFixedWidthLength = atoi(GetNthField(strFormat, 0, "5"));
			bRightJustified = (GetNthField(strFormat, 1, "0") == "1");
			strFillCharacter = GetNthField(strFormat, 2, " ");
			if(strFillCharacter == "") strFillCharacter = " ";

			if(varField.vt == VT_I4) {
				BOOL bSentenceFormat = (GetNthField(strFormat, 3, "0") == "1");
				CStringArray saTempFiles;
				if(bSentenceFormat)
					strOutput = GetSentence(VarLong(varField), NULL, false, false, saTempFiles);
				else
					strOutput = GetDataOutput(VarLong(varField), NULL, false, false, saTempFiles);
			
				ASSERT(!saTempFiles.GetSize());
				for(int i = 0; i < saTempFiles.GetSize(); i++) DeleteFile(saTempFiles[i]);
			}
			else {
				strOutput = "";
			}
		}
		break;

	default:
		ASSERT(FALSE);
		break;
	}

	//OK, strOutput is now the formatted output.  Now we need to process any special characters.
	CArray<SpecialChar,SpecialChar> arSpecialChars1, arSpecialChars2;
	//Here's the deal.  If any of the special characters is found in the replacement of another special character, it needs to 
	//be done first.
	//For example, if we're replacing " with \", and \ with \\, we need to do the \->\\ replacement first.
	// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
	int i = 0;
	for(i = 0; i < arSpecialChars.GetSize(); i++) {
		bool bPlaced = false;
		for(int j = 0; j < arSpecialChars.GetSize() && !bPlaced; j++) {
			if(i != j && arSpecialChars.GetAt(j).strReplaceChar.Find(arSpecialChars.GetAt(i).strSourceChar) != -1) {
				arSpecialChars1.Add(arSpecialChars.GetAt(i));
				bPlaced = true;
			}
		}
		if(!bPlaced) {
			arSpecialChars2.Add(arSpecialChars.GetAt(i));
		}
	}
	for(i = 0; i < arSpecialChars1.GetSize(); i++) {
		strOutput.Replace(arSpecialChars1.GetAt(i).strSourceChar, arSpecialChars1.GetAt(i).strReplaceChar);
	}
	for(i = 0; i < arSpecialChars2.GetSize(); i++) {
		strOutput.Replace(arSpecialChars2.GetAt(i).strSourceChar, arSpecialChars2.GetAt(i).strReplaceChar);
	}

	//Now, if it's fixed width, deal with that.
	if(bFixedWidth) {
		if(strOutput.GetLength() > nFixedWidthLength) {
			if(bRightJustified) {
				strOutput = strOutput.Right(nFixedWidthLength);
			}
			else {
				strOutput = strOutput.Left(nFixedWidthLength);
			}
		}
		else {
			for(i = strOutput.GetLength(); i < nFixedWidthLength; i++) {
				if(bRightJustified) {
					strOutput = strFillCharacter.Left(1) + strOutput;
				}
				else {
					strOutput = strOutput + strFillCharacter.Left(1);
				}
			}
		}
	}

	return strOutput;

}

CPatientExportRestrictions g_patientExportRestrictions;

CPatientExportRestrictions& GetPatientExportRestrictions()
{
	return g_patientExportRestrictions;
}

CPatientExportRestrictions::CPatientExportRestrictions()
	: m_nLimit(-1), m_bEnabled(true)
{
}

// (a.walling 2010-10-04 13:30) - PLID 40738 - Is patient export restricted?
// (a.walling 2010-10-05 13:25) - PLID 40822 - bSecureCheck will contact the license activation server to verify the timestamp
bool CPatientExportRestrictions::Enabled(bool bSecureCheck)
{
	try {
		LoadSettings();

		// (z.manning 2011-02-14 15:33) - PLID 42443 - We now have an option for whether or not this is enabled at all.
		if(!m_bEnabled) {
			return false;
		}

		// (j.gruber 2011-01-26 13:07) - PLID 42236 - check if this is internal and if so, let it through
		if (IsNexTechInternal()) {
			return false;
		}

		if (m_dtUtcSunrise.GetStatus() != COleDateTime::valid || m_dtUtcSunrise.m_dt == 0) {
			return true;
		}
		
		// (a.walling 2010-10-05 13:25) - PLID 40822 - Please note that, although I did my best to avoid casual tampering, implementing
		// more security for this would involve significant effort. Someone can easily circumvent this by modifying the database.
		// However, if they are already able to do so, then they would have no problem dumping out all the patient export info from
		// the database itself. So I am content with this for now.

		COleDateTime dtUtcNow;

		if (bSecureCheck) {
			dtUtcNow = GetLicenseActivationServerUTCTime(GetSubRegistryKey());
		} else {
			dtUtcNow = GetRemoteServerUTCTime();
		}

		if (dtUtcNow > m_dtUtcSunrise) {
			m_dtUtcSunrise.m_dt = 0;
			m_dtUtcSunrise.SetStatus(COleDateTime::valid);

			SetRemotePropertyDateTime("PatientExportRestrictions_Sunrise", m_dtUtcSunrise, 0, "<None>");

			return true;
		} else {
			return false;
		}
	} NxCatchAll("Could not check patient export restrictions");

	return true;
}

// (a.walling 2010-10-04 13:30) - PLID 40738 - Explain the current export restrictions
CString CPatientExportRestrictions::Description()
{
	// (j.jones 2010-12-07 11:10) - PLID 41736 - We no longer restrict fields, I commented out the old text and
	// changed the description to refer to a record count restriction only.

	return FormatString(
		"When this export is run, a maximum of %li records will be returned. "
		"Please contact NexTech Technical Support for further assistance with or information regarding patient exports."
		, Limit()
	);

	/*
	return FormatString(
		"When this export is run, a maximum of %li records will be returned, limited to name, address, and contact information only. "
		"Please contact NexTech Technical Support for further assistance with or information regarding patient exports."
		, Limit()
	);
	*/
}

// (a.walling 2010-10-05 13:25) - PLID 40822 - Set the 'sunrise' when restrictions come back into effect
void CPatientExportRestrictions::LiftRestrictions()
{
	try {
		if (-26 != GetCurrentUserID()) {
			ThrowNxException("Unauthorized access");
		}

		COleDateTime dtUtcNow = GetLicenseActivationServerUTCTime(GetSubRegistryKey());

		// 3 days from now
		COleDateTime dtUtcRestrictionsSunrise = dtUtcNow + COleDateTimeSpan(3, 0, 0, 0);

		SetRemotePropertyDateTime("PatientExportRestrictions_Sunrise", dtUtcRestrictionsSunrise, 0, "<None>");

		m_dtUtcSunrise = dtUtcRestrictionsSunrise;

	} NxCatchAll("Could not lift patient export restrictions");
}

long CPatientExportRestrictions::Limit()
{
	LoadSettings();
	return m_nLimit;
}


// (a.walling 2010-10-04 13:30) - PLID 40738 - Is this field allowed in a restricted export?
bool CPatientExportRestrictions::IsFieldRestricted(int nID)
{
	// (j.jones 2010-12-07 11:03) - PLID 41736 - We are removing the restrictions on fields,
	// but if we ever wanted to bring them back, just comment in the remainder of this
	// function, and correct the list of restricted field IDs.
	// You will also need to re-enable showing the IDC_EXPORT_RESTRICTED_FIELDS label in ExportWizardFieldsDlg.cpp,
	// change the CPatientExportRestrictions::Description() text in this cpp file, and comment in the code
	// to color fields blue in CExportWizardFieldsDlg::LoadActiveFields() and CExportWizardFieldsDlg::OnSetActive().
	return false;

	/*
	if (!Enabled(false)) {
		return false;
	}

	switch(nID) {

		// Misc (ID, Today's Date)
		case 8:
		case 2:
		case 1: // placeholder

		// Name
		case 86:
		case 87:
		case 88:
		case 89:
		case 90:
		case 92:
		case 468:
		case 93:
		case 94:
		case 104:

		// Address
		case 95:
		case 96:
		case 97:
		case 98:
		case 99:

		// Contact (Phone & Email)
		case 6:
		case 100:
		case 101:
		case 102:
		case 103:
		case 469:
			return false;

		default:
			return true;
	}
	*/
}

// (a.walling 2010-10-05 13:25) - PLID 40822 - Load the limit and the sunrise time
void CPatientExportRestrictions::LoadSettings()
{
	if (m_nLimit == -1) {
		COleDateTime dtDefault;
		dtDefault.m_dt = 0;
		dtDefault.SetStatus(COleDateTime::valid);
		m_dtUtcSunrise = GetRemotePropertyDateTime("PatientExportRestrictions_Sunrise", &dtDefault, 0, "<None>", true);
		m_nLimit = GetRemotePropertyInt("PatientExportRestrictions_Limit", 100, 0, "<None>", true);
		// (z.manning 2011-02-14 15:31) - PLID 42443 - Add an option for whether or not is is enabled at all.
		m_bEnabled = (GetRemotePropertyInt("PatientExportRestrictions_Enabled", 1, 0, "<None>", true) != 0);
	}
}