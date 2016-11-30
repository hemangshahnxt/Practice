// FormLayer.cpp: implementation of the FormLayer class.
//A layer of the form, contains a group of controls
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FormLayer.h"
#include "FormQuery.h"
#include "FormFormat.h"
#include "FormDate.h"
#include "FormEdit.h"
#include "FormCheck.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace ADODB;

extern int MINI_FONT_SIZE;

// (a.walling 2014-03-06 12:39) - PLID 61225 - Removed queries here that are always overridden when used; this way we don't try to maintain untestable code.

// Search for usage of these form ids via (g_pFrame)|(m_pframe)-\>\b*(Refresh)|(Load) since all CFormDisplayDlg appear to be named m_pframe (or g_pFrame, what joy)

CFormQuery g_aryFormQueries[] = {
	//HCFA
	CFormQuery(0,"",4095,85,60),
	//HCFA Demographics
	CFormQuery(1,"SELECT PatientsT.UserDefinedID AS [Patient ID], PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS PatName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS PatAdd, PersonT.City AS PatCity, PersonT.State AS PatState, PersonT.Zip AS PatZip, "
				"PersonT.HomePhone AS PatPhone, PersonT.BirthDate AS PatBD, PersonT.Gender AS PatGender, PersonT.SocialSecurity AS PatSSN, PatientsT.MaritalStatus AS PatMarStatus, PatientsT.Employment AS PatEmployment, [Contact Info].PersonID AS DocID, [Contact Info].[First] AS DocFirstName, [Contact Info].[Middle] AS DocMiddleName, [Contact Info].[Last] + (CASE WHEN ([Contact Info].Title <> '') THEN (', ' + [Contact Info].Title) ELSE ('') END) AS DocName, "
				"[Contact Info].Title AS DocTitle, LocationsT.Name AS PracName, LocationsT.Address1 AS PracAdd1, LocationsT.Address2 AS PracAdd2, LocationsT.[Phone] AS DocPhone, 'Signature On File' AS SignatureOnFile, GetDate() AS TodaysDate, [GRPNumber] AS GRP FROM LocationsT RIGHT JOIN PatientsT ON LocationsT.ID = InLocation LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID LEFT JOIN "
				"(SELECT * FROM PersonT LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS [Contact Info] ON PatientsT.MainPhysician = [Contact Info].ID", 4095,85,60),
	//HCFA Charges (FormChargesT)
	//DRT 4/10/2006 - PLID 11734 - Removed ProcCode
	// (j.gruber 2009-03-18 09:07) - PLID 33360 - changed discount structure
	// (j.jones 2010-04-07 17:29) - PLID 15224 - removed ChargesT.EMG, obsolete field
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by form; cleared
	CFormQuery(2,"",4095,85,60),
	//HCFA Charge Totals
	CFormQuery(3,"SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	//HCFA Payment Totals
	CFormQuery(4,"SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	//HCFA Balance
	CFormQuery(5,"SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	//HCFA Doctor Info
	CFormQuery(6,"SELECT '1' AS FedIDType, FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, DoctorInfo.[Last] AS DocLastName, DoctorInfo.[First] AS DocFirstName, DoctorInfo.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, "
				"CASE WHEN [ShowSignature] = 1 THEN DoctorInfo.[First] + ' ' + DoctorInfo.[Middle] + ' ' + DoctorInfo.[Last] + (CASE WHEN (DoctorInfo.Title <> '') THEN (', ' + DoctorInfo.Title) ELSE ('') END) ELSE Null END AS DocSignature, DoctorInfo.SocialSecurity, DoctorInfo.[Fed Employer ID] FROM FormChargesT "
				"INNER JOIN (SELECT * FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID) AS DoctorInfo ON FormChargesT.DoctorsProviders = DoctorInfo.ID",4095,85,60),
	CFormQuery(7,"",4095,85,60),
	CFormQuery(8,"",4095,85,60),

	//UB92 Box 82 (Provider/Referring Info)
	CFormQuery(9,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], '' AS Box51A, '' AS Box51B, '' AS Box51C "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),

	//UB92 Box 83A (Provider/Referring Info)
	CFormQuery(10,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], '' AS Box51A, '' AS Box51B, '' AS Box51C "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),
	
	//UB92 Demographics
	CFormQuery(11,"SELECT PatientsT.PersonID, PatientsT.UserDefinedID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS Name, (CASE WHEN (PatientsT.[MaritalStatus]='1') THEN 'S' WHEN (PatientsT.[MaritalStatus]='2') THEN 'M' ELSE ' ' END) AS MS, PersonT.[Address1] + ' ' + PersonT.[Address2] + ' ' + PersonT.[City] + ' ' + PersonT.[State] + ' ' + PersonT.[Zip] AS Address, "
				"PersonT.[BirthDate] AS BirthDate, (CASE WHEN (PersonT.[Gender]=1) THEN 'M' WHEN (PersonT.[Gender]=2) THEN 'F' ELSE Null END) AS Sex, '' AS Box4, '' AS Box79 FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID",4095,70,55),
	//UB92 Charges
	CFormQuery(12,"SELECT * FROM FormChargesT",4095,70,55),
	//UB92 Dates and Codes
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by UB92 form; cleared
	CFormQuery(13,"",4095,70,55),
	//UB92 Location Info
	CFormQuery(14,"SELECT TOP 1 LocationsT.*, City + ', ' + State + ' ' + Zip AS CityStateZip FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID",4095,70,55), 
	//UB92 Insured Party
	CFormQuery(15,"SELECT (PersonT.[First] + (CASE WHEN (PersonT.Middle Is NULL OR PersonT.Middle = '') THEN '' ELSE (' ' + PersonT.Middle) END) + ' ' + PersonT.[Last]) AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID",4095,70,55),
	//UB92 Provider Info
	CFormQuery(16,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], '' AS Box51A, '' AS Box51B, '' AS Box51C "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),
	//UB92 Primary Insurance
	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	CFormQuery(17,"SELECT InsuredPartyT.IDforInsurance AS InsID, PersonT.[Last] + ',  ' + PersonT.[First] + ' ' + PersonT.[Middle] AS InsName, PersonT.[Address1] + '  ' + PersonT.[Address2] AS InsAdd, PersonT.City AS InsCity, PersonT.State AS InsState, PersonT.Zip AS InsZip, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip, PersonT.WorkPhone AS InsPhone, InsuredPartyT.PolicyGroupNum AS InsFECA, PersonT.BirthDate AS InsBD, InsuredPartyT.Employer AS InsEmp, InsurancePlansT.PlanName AS InsPlan, PersonT.Gender, PersonT.[Gender] AS InsGender, InsuranceCoT.Name AS InsCoName, InsCoPersonT.Address1 AS InsCoAdd1, InsCoPersonT.Address2 AS InsCoAdd2, InsCoPersonT.City AS InsCoCity, InsCoPersonT.State AS InsCoState, InsCoPersonT.Zip AS InsCoZip, (CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS InsRel, InsurancePlansT.PlanType, "
				"(CASE WHEN ([InsurancePlansT].[PlanType]='Medicare') THEN 1 WHEN ([InsurancePlansT].[PlanType]='Medicaid') THEN 2 WHEN ([InsurancePlansT].[PlanType]='Champus') THEN 3 WHEN ([InsurancePlansT].[PlanType]='Champva') THEN 4 WHEN ([InsurancePlansT].[PlanType]='Group Health Plan') THEN 5 WHEN ([InsurancePlansT].[PlanType]='FECA Black Lung') THEN 6 WHEN ([InsurancePlansT].[PlanType]='Other') THEN 7 ELSE 0 END) AS InsType, "
				"[Accepted], InsuredPartyT.PersonID AS ID, InsuredPartyT.PatientID, 'Y' AS Yes FROM (InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID ",4095,70,55),
	//UB92 Secondary Insurance
	CFormQuery(18,"SELECT InsuredPartyT.IDforInsurance AS OthrInsID, PersonT.[Last] + ',  ' + PersonT.[First] + ' ' + PersonT.[Middle] AS OthrInsName, PersonT.[Address1] + '  ' + PersonT.[Address2] AS OthrInsAdd, PersonT.City AS OthrInsCity, PersonT.State AS OthrInsState, PersonT.Zip AS OthrInsZip, PersonT.WorkPhone AS OthrInsPhone, InsuredPartyT.PolicyGroupNum AS OthrInsFECA, PersonT.BirthDate AS OthrInsBD, InsuredPartyT.Employer AS OthrInsEmp, InsurancePlansT.PlanName AS OthrInsPlan, PersonT.Gender AS OthrGender, PersonT.[Gender] AS OthrInsGender, InsuranceCoT.Name AS OthrInsCoName, InsCoPersonT.Address1 AS OthrInsCoAdd1, InsCoPersonT.Address2 AS OthrInsCoAdd2, InsCoPersonT.City AS OthrInsCoCity, InsCoPersonT.State AS OthrInsCoState, InsCoPersonT.Zip AS OthrInsCoZip, (CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS OthrInsRel "
				"FROM InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID",4095,70,55),
	//UB92 Payments
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
	CFormQuery(19,"SELECT LineItemT.PatientID AS [Patient ID], PaymentsT.InsuredPartyID, "
		"Sum(AppliesT.Amount) AS SumOfAmount "
		"FROM PaymentsT "
		"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
		"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE (((AppliesT.PointsToPayments)<>1)) "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY LineItemT.PatientID, PaymentsT.InsuredPartyID",4095,70,55),

	//HCFA Primary Insurance
	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	// (j.jones 2013-05-09 08:44) - PLID 54511 - removed InsType, replaced with a 1/0 value for each type
	CFormQuery(20,"SELECT InsuredPartyT.IDForInsurance AS InsID, InsPartyPersonT.[Last] + ',  ' + InsPartyPersonT.[First] + ' ' + InsPartyPersonT.[Middle] AS InsName, InsPartyPersonT.[Address1] + '  ' + InsPartyPersonT.[Address2] AS InsAdd, InsPartyPersonT.City AS InsCity, InsPartyPersonT.State AS InsState, InsPartyPersonT.Zip AS InsZip, InsPartyPersonT.HomePhone AS InsPhone, InsuredPartyT.PolicyGroupNum AS InsFECA, InsPartyPersonT.BirthDate AS InsBD, InsuredPartyT.[Employer] AS InsEmp, InsurancePlansT.PlanName AS InsPlan, InsPartyPersonT.Gender, InsPartyPersonT.Gender AS InsGender, InsuranceCoT.Name AS InsCoName, InsCoPersonT.Address1 AS InsCoAdd1, InsCoPersonT.Address2 AS InsCoAdd2, InsCoPersonT.City AS InsCoCity, InsCoPersonT.State AS InsCoState, InsCoPersonT.Zip AS InsCoZip, (CASE WHEN(InsuredPartyT.[RelationToPatient]='Self') THEN 1 WHEN (InsuredPartyT.[RelationToPatient]='Spouse') THEN 2 WHEN(InsuredPartyT.[RelationToPatient]='Child') THEN 3 WHEN (InsuredPartyT.[RelationToPatient] <> '') THEN 4 ELSE Null END) AS InsRel, "
				"RespTypeT.TypeName AS InsCoRespType, InsurancePlansT.PlanType, (CASE WHEN([InsurancePlansT].[PlanType]='Medicare') THEN 1 WHEN ([InsurancePlansT].[PlanType]='Medicaid') THEN 2 WHEN ([InsurancePlansT].[PlanType]='Champus') THEN 3 WHEN([InsurancePlansT].[PlanType]='Champva') THEN 4 WHEN([InsurancePlansT].[PlanType]='Group Health Plan') THEN 5 WHEN([InsurancePlansT].[PlanType]='FECA Black Lung') THEN 6 WHEN([InsurancePlansT].[PlanType]='Other') THEN 7 ELSE 0 END) AS InsType, InsuranceCoT.Accepted, InsuredPartyT.PersonID, InsuredPartyT.PatientID FROM InsuranceCoT LEFT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.[ID] LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.[ID] LEFT JOIN (SELECT * FROM PersonT) AS InsPartyPersonT ON InsuredPartyT.PersonID = InsPartyPersonT.[ID] LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID",4095,85,60),
	//HCFA Secondary Insurance
	// (j.jones 2010-08-31 16:57) - PLID 40303 - added TPLCode
	CFormQuery(21,"SELECT InsuredPartyT.IDForInsurance AS OthrInsID, InsPartyPersonT.[Last] + ',  ' + InsPartyPersonT.[First] + ' ' + InsPartyPersonT.[Middle] AS OthrInsName, InsPartyPersonT.Address1 + '  ' + InsPartyPersonT.Address2 AS OthrInsAdd, InsPartyPersonT.City AS OthrInsCity, InsPartyPersonT.State AS OthrInsState, InsPartyPersonT.Zip AS OthrInsZip, InsPartyPersonT.HomePhone AS OthrInsPhone, InsuredPartyT.IDForInsurance AS OthrInsIDForInsurance, InsPartyPersonT.BirthDate AS OthrInsBD, InsuredPartyT.Employer AS OthrInsEmp, InsurancePlansT.PlanName AS OthrInsPlan, InsPartyPersonT.Gender AS OthrGender, InsPartyPersonT.Gender AS OthrInsGender, InsuranceCoT.Name AS OthrInsCoName, InsCoPersonT.Address1 AS OthrInsCoAdd1, InsCoPersonT.Address2 AS OthrInsCoAdd2, InsCoPersonT.City AS OthrInsCoCity, InsCoPersonT.State AS OthrInsCoState, InsCoPersonT.Zip AS OthrInsCoZip, CASE WHEN (InsuredPartyT.[PersonID] Is Null) Then 0 ELSE 1 END AS Box11d, InsuredPartyT.PersonID, "
				"InsuranceCoT.TPLCode AS OthrInsTPLCode "
				"FROM (InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) "
				"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID LEFT JOIN (SELECT * FROM PersonT) AS InsPartyPersonT ON InsuredPartyT.PersonID = InsPartyPersonT.[ID] LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.[ID]",4095,85,60),
	//HCFA Bill Info
	// (j.jones 2013-08-14 14:57) - PLID 57299 - added Box8, Box9b, Box9c, Box11bQual, Box11b, Box17Qual, Box14Qual, Box15Qual
	// though this query is always overwritten
	// (j.jones 2014-05-05 15:39) - PLID 61993 - removed query 22 since it is always overwritten
	CFormQuery(22,"",4095,85,60),
	CFormQuery(23,"",4095,85,60),
	CFormQuery(24,"",4095,85,60),
	CFormQuery(25,"",4095,85,60),
	CFormQuery(26,"",4095,85,60),
	CFormQuery(27,"",4095,85,60),
	CFormQuery(28,"",4095,85,60),
	CFormQuery(29,"",4095,85,60),
	//UB92 Payments
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
	CFormQuery(30,"SELECT LineItemT.PatientID, PaymentsT.InsuredPartyID, "
		"Sum(AppliesT.Amount) AS SumOfAmount "
		"FROM PaymentsT "
		"LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
		"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY LineItemT.PatientID, PaymentsT.InsuredPartyID",4095,70,55),
	//UB92 Charges
	CFormQuery(31,"SELECT Sum(LineItemT.Amount * Quantity) AS SumOfCharges FROM FormChargesT",4095,70,55),
	CFormQuery(32,"",4095,85,60),

	//HCFA Box33 Doctor Demographic Info
	CFormQuery(33,"SELECT First(FormChargesT.DoctorsProviders) AS FirstOfDoctorsProviders, "
				"[Contact Info].[First Name] + ' ' + [Contact Info].[Middle Name] + ' ' + [Contact Info].[Last Name] + (CASE WHEN ([Contact Info].Title <> '') THEN (', ' + [Contact Info].Title) ELSE ('') END) AS DocSignature, "
				"[Contact Info].[First Name] + ' ' + [Contact Info].[Middle Name] + ' ' + [Contact Info].[Last Name] + (CASE WHEN ([Contact Info].Title <> '') THEN (', ' + [Contact Info].Title) ELSE ('') END) AS DocNameFML, "
				"[Contact Info].[Last Name] + (CASE WHEN ([Contact Info].Title <> '') THEN (', ' + [Contact Info].Title) ELSE ('') END) + ' ' + [Contact Info].[First Name] + ' ' + [Contact Info].[Middle Name] AS DocNameLFM, "
				"ProvidersT.SocialSecurityNumber, ProvidersT.[Fed Employer ID], ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.NPI, ProvidersT.UPIN, [Contact Info].[Address 1] AS DocAdd, [Contact Info].City + ', ' + [Contact Info].StateProv + ' ' + [Contact Info].PostalCode AS DocCityStateZip, "
				"ProvidersT.[WorkPhone] AS DocPhone FROM (SELECT * FROM PersonT) AS [Contact Info] INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.[PersonID]) ON ([Contact Info].ID = ProvidersT.[PersonID]) AND ([Contact Info].ID = ProvidersT.[Contact ID])",4095,85,60),
	//HCFA Box33 Doctor PIN Number
	CFormQuery(34,"SELECT First(FormChargesT.DoctorsProviders) AS FirstOfDoctorsProviders, [Contact Info].[Last Name] AS DocLastName, [Contact Info].[Title] + ' ' + [Contact Info].[First Name] AS DocFirstName, [Contact Info].[Middle Name] AS DocMiddleName, GetDate() AS TodaysDate, [Contact Info].[Title] + ' ' + [Contact Info].[First Name] + ' ' + [Contact Info].[Middle Name] + ' ' + [Contact Info].[Last Name] AS DocSignature, ProvidersT.SocialSecurityNumber, ProvidersT.[Fed Employer ID], ProvidersT.[DEA Number], ProvidersT.[BCBS Number], ProvidersT.[Medicare Number], ProvidersT.[Medicaid Number], ProvidersT.[Workers Comp Number], ProvidersT.[Other ID Number], ProvidersT.NPI, ProvidersT.UPIN, [Contact Info].[Address 1] AS DocAdd, [Contact Info].City + ', ' + [Contact Info].StateProv + ' ' + [Contact Info].PostalCode AS DocCityStateZip "
				"FROM (SELECT * FROM PersonT) AS [Contact Info] INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.[Provider ID]) ON ([Contact Info].ID = ProvidersT.[Contact ID]) AND ([Contact Info].ID = ProvidersT.[Contact ID])",4095,85,60),
	CFormQuery(35,"",4095,85,60),
	CFormQuery(36,"",4095,85,60),
	CFormQuery(37,"",4095,85,60),
	CFormQuery(38,"",4095,85,60),
	CFormQuery(39,"",4095,85,60),

	// (j.jones 2008-10-28 10:52) - PLID 26526 - changed some of these queries to support the new ADA form
	// (j.armen 2014-03-05 09:17) - PLID 60784 - Moved modified queries directly into ADA Dialog
	//ADA Patient Info
	CFormQuery(40,"",4095,85,60),

	//ADA Primary Insured Party Info
	CFormQuery(41,"SELECT Last + ', ' + First + ' ' + Left(Middle, 1) AS PriInsuredName, "
				"Address1 + ' ' + Address2 AS PriInsuredAddress, "
				"City + ', ' + State + ', ' + Zip AS PriInsuredCSZ, "
				"CASE WHEN RelationToPatient = 'Self' THEN 0 WHEN RelationToPatient = 'Spouse' THEN 1 WHEN RelationToPatient = 'Child' THEN 2 ELSE 3 END AS Box18Value, "
				"PersonT.*, InsuredPartyT.*, "
				"InsurancePlansT.PlanName AS PriInsPlanName  "
				"FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID",4095,85,60),

	//ADA Secondary Insured Party Info
	CFormQuery(42,"",4095,85,60),

	//ADA Provider Info
	CFormQuery(43,"SELECT First + ' ' + Middle + ' ' + Last + ' ' + Title AS DocName, "
				"Address1 + ' ' + Address2 AS Address, PersonT.*, "
				"ProvidersT.* "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID",4095,85,60),

	//ADA Bill/Charge Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(44,"",
				4095,85,60),

	//ADA Bill/Charge Totals
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(45,"SELECT dbo.GetClaimTotal(BillsT.ID) AS TotalFee FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID ",
				4095,85,60),

	//ADA Place Of Service Info
	CFormQuery(46,"",4095,85,60),

	//ADA Primary Insurance Company Info
	CFormQuery(47,"SELECT Name AS InsCoName, Address1 + ' ' + Address2 AS InsAddress, "
				"City + ', ' + State + ', ' + Zip AS InsCSZ, "
				"PersonT.*, InsuranceCoT.* "
				"FROM PersonT "
				"INNER JOIN InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID "
				"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID",4095,85,60),

	//ADA Primary Insurance Company Info
	CFormQuery(48,"SELECT Name AS SecInsCoName, Address1 + ' ' + Address2 AS SecInsAddress, "
				"City + ', ' + State + ', ' + Zip AS SecInsCSZ, "
				"PersonT.*, InsuranceCoT.* "
				"FROM PersonT "
				"INNER JOIN InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID "
				"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID",4095,85,60),

	//ADA Diagnosis Codes
	CFormQuery(49,"",4095,85,60),

	//IDPA Patient Info
	CFormQuery(50,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ' ' + EmployerCity + ' ' + EmployerState + ' ' + EmployerZip AS EmpAddress, PersonT.*, PatientsT.*, "
				"'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",4095,85,60),

	//IDPA Primary Insured Party Info
	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	CFormQuery(51,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PriInsuredName, Address1 + ' ' + Address2 AS PriAddress, City + ', ' + State + ' ' + Zip AS PriCityStateZip, "
				"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS InsRel, "
				"PersonT.*, InsuredPartyT.*, InsurancePlansT.PlanName AS InsPlanName  "
				"FROM PersonT INNER JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID",4095,85,60),

	//IDPA Secondary Insured Party Info
	CFormQuery(52,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS SecInsuredName, Address1 + ' ' + Address2 AS SecAddress, City + ', ' + State + ' ' + Zip AS SecCityStateZip, PersonT.*, InsuredPartyT.*, InsurancePlansT.PlanName AS InsPlanName "
				"FROM PersonT LEFT JOIN InsuredPartyT ON PersonT.ID = InsuredPartyT.PersonID LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID",4095,85,60),

	//IDPA Bill/Charge/RefPhy/POS Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by IDPA form; cleared
	CFormQuery(53,"",4095,85,60),

	//IDPA Provider Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(54,"SELECT "
				"First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last + ' ' + Title AS DocNameFMLT, "
				"Last + ', ' + First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END) + ' ' + Title AS DocNameLFMT, "
				"Last + ' ' + First + ' ' + Title AS DocNameLFT, "
				"First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS DocNameFML, "
				"Last + ', ' + First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END) AS DocNameLFM, "
				"Last + ' ' + First AS DocNameLF, "
				"LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress, "
				"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, PersonT.*, ProvidersT.* "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID ",4095,85,60),

	//IDPA Charge Totals
	CFormQuery(55,"SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges, Count(ChargeTotal) AS CountCharges FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	
	//IDPA Payment Totals
	CFormQuery(56,"SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	
	//IDPA Balance
	CFormQuery(57,"SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),

	CFormQuery(58,"",4095,85,60),
	CFormQuery(59,"",4095,85,60),

	//NYWC Patient Info
	CFormQuery(60,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PatFullAddress, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ' ' + EmployerCity + ' ' + EmployerState + ' ' + EmployerZip AS EmpAddress, PersonT.*, PatientsT.*, "
				"'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",4095,85,60),

	//NYWC Primary Insurance Co Info
	CFormQuery(61,"SELECT Address1 + ' ' + Address2 AS PriAddress, City + ', ' + State + ' ' + Zip AS PriCityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PriFullAddress, PersonT.*, InsuranceCoT.*, InsuredPartyT.* "
				"FROM PersonT INNER JOIN InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID ",4095,85,60),

	//NYWC Bill/Charge/RefPhy/POS Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by NYWC form; cleared
	CFormQuery(62,"",4095,85,60),

	//NYWC Provider Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(63,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last + ' ' + Title AS DocName, LocationsT.Name AS LocName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress, "
				"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, "
				"LocationsT.Address1 + ' ' + LocationsT.Address2 + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocFullAddress, LocationsT.Phone AS LocPhone, "
				"PersonT.*, ProvidersT.* "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID",4095,85,60),

	//NYWC Charge Totals
	CFormQuery(64,"SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	
	//NYWC Payment Totals
	CFormQuery(65,"SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	
	//NYWC Balance
	CFormQuery(66,"SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP 6 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),

	CFormQuery(67,"",4095,85,60),
	CFormQuery(68,"",4095,85,60),
	CFormQuery(69,"",4095,85,60),

	//MICR Patient Info
	CFormQuery(70,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PatFullAddress, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ' ' + EmployerCity + ' ' + EmployerState + ' ' + EmployerZip AS EmpAddress, PersonT.*, PatientsT.*, "
				"'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",4095,85,60),

	//MICR Insured Party Info
	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	CFormQuery(71,"SELECT Address1 + ' ' + Address2 AS PriAddress, City + ', ' + State + ' ' + Zip AS PriCityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PriFullAddress, "
				"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS InsRel, "
				"PersonT.*, InsuranceCoT.*, InsuredPartyT.* "
				"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID",4095,85,60),

	//MICR Bill/Charge/RefPhy/POS Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by MICR form; cleared
	CFormQuery(72,"",4095,85,60),

	//MICR Provider Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(73,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last + ' ' + Title AS DocName, LocationsT.Name AS LocName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress, "
				"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, "
				"LocationsT.Address1 + ' ' + LocationsT.Address2 + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocFullAddress, LocationsT.Phone AS LocPhone, "
				"PersonT.*, ProvidersT.* "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID ",4095,85,60),

	CFormQuery(74,"",4095,85,60),

	// (j.jones 2007-05-08 08:49) - PLID 25550 - supported the 2007 version of MICR
	
	//MICR 2007 Patient Info
	CFormQuery(75,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PatFullAddress, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ' ' + EmployerCity + ' ' + EmployerState + ' ' + EmployerZip AS EmpAddress, PersonT.*, PatientsT.*, "
				"'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",4095,85,60),

	//MICR 2007 Insured Party Info
	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	CFormQuery(76,"SELECT Address1 + ' ' + Address2 AS PriAddress, City + ', ' + State + ' ' + Zip AS PriCityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PriFullAddress, "
				"(CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS InsRel, "
				"PersonT.*, InsuranceCoT.*, InsuredPartyT.* "
				"FROM InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID",4095,85,60),

	//MICR 2007 Bill/Charge/RefPhy/POS Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by MICR form; cleared
	CFormQuery(77,"",4095,85,60),

	//MICR 2007 Provider Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(78,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last + ' ' + Title AS DocName, LocationsT.Name AS LocName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress, "
				"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, "
				"LocationsT.Address1 + ' ' + LocationsT.Address2 + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocFullAddress, LocationsT.Phone AS LocPhone, "
				"PersonT.*, ProvidersT.*, CASE WHEN Coalesce([BCBS Number],'') <> '' THEN [ProviderQual] ELSE '' END AS ProviderQual, CASE WHEN Coalesce(License,'') <> '' THEN [Box44Qual] ELSE '' END AS Box44Qual "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID",4095,85,60),
	
	CFormQuery(79,"",4095,85,60),

	// (j.jones 2007-03-05 09:29) - PLID 23939 - added queries for UB04

	//UB04 Box 76 (Provider/Referring Info)
	CFormQuery(80,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], '' AS Box51A, '' AS Box51B, '' AS Box51C "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),

	//UB04 Box 78 (Provider/Referring Info)
	CFormQuery(81,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], '' AS Box51A, '' AS Box51B, '' AS Box51C "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),
	
	//UB04 Demographics
	CFormQuery(82,"SELECT PatientsT.PersonID, PatientsT.UserDefinedID AS ID, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS Name, (CASE WHEN (PatientsT.[MaritalStatus]='1') THEN 'S' WHEN (PatientsT.[MaritalStatus]='2') THEN 'M' ELSE ' ' END) AS MS, PersonT.[Address1] + ' ' + PersonT.[Address2] + ' ' + PersonT.[City] + ' ' + PersonT.[State] + ' ' + PersonT.[Zip] AS Address, "
				"PersonT.[BirthDate] AS BirthDate, (CASE WHEN (PersonT.[Gender]=1) THEN 'M' WHEN (PersonT.[Gender]=2) THEN 'F' ELSE Null END) AS Sex, '' AS Box4, '' AS Box79 FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID",4095,70,55),
	//UB04 Charges
	CFormQuery(83,"SELECT * FROM FormChargesT",4095,70,55),
	//UB04 Dates and Codes
	// (j.jones 2008-06-09 15:40) - PLID 30229 - included the HospFrom field
	// (j.jones 2009-12-22 10:38) - PLID 27131 - included the ConditionDate field
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by UB04 form; cleared
	CFormQuery(84,"",4095,70,55),
	//UB04 Box 1 Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(85,"SELECT TOP 1 LocationsT.*, City + ', ' + State + ' ' + Zip AS CityStateZip FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID",4095,70,55), 
	//UB04 Insured Party
	CFormQuery(86,"SELECT (PersonT.[First] + (CASE WHEN (PersonT.Middle Is NULL OR PersonT.Middle = '') THEN '' ELSE (' ' + PersonT.Middle) END) + ' ' + PersonT.[Last]) AS FullName, PersonT.[Address1] + ' ' + PersonT.[Address2] AS FullAddress, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip FROM (BillsT INNER JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID) INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID",4095,70,55),
	//UB04 Provider Info
	CFormQuery(87,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], "
				"'' AS Box51A, '' AS Box51B, '' AS Box51C, '' AS Box81aQual, '' AS Box81aTaxonomy, '' AS Box81bQual, '' AS Box81bTaxonomy, '' AS Box81cQual, '' AS Box81Taxonomy, '' AS Box56NPI "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),
	//UB04 Primary Insurance
	// (j.jones 2008-05-01 11:00) - PLID 28467 - ensured that any non-empty relation to patient that isn't
	// standard will show as 'other'
	CFormQuery(88,"SELECT InsuredPartyT.IDforInsurance AS InsID, PersonT.[Last] + ',  ' + PersonT.[First] + ' ' + PersonT.[Middle] AS InsName, PersonT.[Address1] + '  ' + PersonT.[Address2] AS InsAdd, PersonT.City AS InsCity, PersonT.State AS InsState, PersonT.Zip AS InsZip, (PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip) AS CityStateZip, PersonT.WorkPhone AS InsPhone, InsuredPartyT.PolicyGroupNum AS InsFECA, PersonT.BirthDate AS InsBD, InsuredPartyT.Employer AS InsEmp, InsurancePlansT.PlanName AS InsPlan, PersonT.Gender, PersonT.[Gender] AS InsGender, InsuranceCoT.Name AS InsCoName, InsCoPersonT.Address1 AS InsCoAdd1, InsCoPersonT.Address2 AS InsCoAdd2, InsCoPersonT.City AS InsCoCity, InsCoPersonT.State AS InsCoState, InsCoPersonT.Zip AS InsCoZip, (CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS InsRel, InsurancePlansT.PlanType, (CASE WHEN ([InsurancePlansT].[PlanType]='Medicare') THEN 1 WHEN ([InsurancePlansT].[PlanType]='Medicaid') THEN 2 WHEN ([InsurancePlansT].[PlanType]='Champus') THEN 3 WHEN ([InsurancePlansT].[PlanType]='Champva') THEN 4 WHEN ([InsurancePlansT].[PlanType]='Group Health Plan') THEN 5 WHEN ([InsurancePlansT].[PlanType]='FECA Black Lung') THEN 6 WHEN ([InsurancePlansT].[PlanType]='Other') THEN 7 ELSE 0 END) AS InsType, "
				"[Accepted], InsuredPartyT.PersonID AS ID, InsuredPartyT.PatientID, 'Y' AS Yes FROM (InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID) LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID ",4095,70,55),
	//UB04 Secondary Insurance
	CFormQuery(89,"SELECT InsuredPartyT.IDforInsurance AS OthrInsID, PersonT.[Last] + ',  ' + PersonT.[First] + ' ' + PersonT.[Middle] AS OthrInsName, PersonT.[Address1] + '  ' + PersonT.[Address2] AS OthrInsAdd, PersonT.City AS OthrInsCity, PersonT.State AS OthrInsState, PersonT.Zip AS OthrInsZip, PersonT.WorkPhone AS OthrInsPhone, InsuredPartyT.PolicyGroupNum AS OthrInsFECA, PersonT.BirthDate AS OthrInsBD, InsuredPartyT.Employer AS OthrInsEmp, InsurancePlansT.PlanName AS OthrInsPlan, PersonT.Gender AS OthrGender, PersonT.[Gender] AS OthrInsGender, InsuranceCoT.Name AS OthrInsCoName, InsCoPersonT.Address1 AS OthrInsCoAdd1, InsCoPersonT.Address2 AS OthrInsCoAdd2, InsCoPersonT.City AS OthrInsCoCity, InsCoPersonT.State AS OthrInsCoState, InsCoPersonT.Zip AS OthrInsCoZip, (CASE WHEN(InsuredPartyT.RelationToPatient='Self') THEN 1 WHEN (InsuredPartyT.RelationToPatient='Spouse') THEN 2 WHEN (InsuredPartyT.RelationToPatient='Child') THEN 3 WHEN (InsuredPartyT.RelationToPatient <> '') THEN 9 ELSE Null END) AS OthrInsRel "
				"FROM InsuranceCoT RIGHT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID LEFT JOIN (SELECT * FROM PersonT) AS InsCoPersonT ON InsuranceCoT.PersonID = InsCoPersonT.ID",4095,70,55),
	//UB04 Payments
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
	CFormQuery(90,"SELECT LineItemT.PatientID AS [Patient ID], PaymentsT.InsuredPartyID, "
		"Sum(AppliesT.Amount) AS SumOfAmount "
		"FROM PaymentsT LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
		"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE (((AppliesT.PointsToPayments)<>1)) "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY LineItemT.PatientID, PaymentsT.InsuredPartyID",4095,70,55),
	//UB04 Payments
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" applies
	CFormQuery(91,"SELECT LineItemT.PatientID, PaymentsT.InsuredPartyID, "
		"Sum(AppliesT.Amount) AS SumOfAmount "
		"FROM PaymentsT LEFT JOIN AppliesT ON PaymentsT.ID = AppliesT.SourceID "
		"LEFT JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"GROUP BY LineItemT.PatientID, PaymentsT.InsuredPartyID",4095,70,55),
	//UB04 Charges
	// (j.jones 2008-06-12 16:49) - PLID 23052 - I added PageFrom and PageTo to form 92
	CFormQuery(92,"SELECT '1' AS PageFrom, '1' AS PageTo, Sum(LineItemT.Amount * Quantity) AS SumOfCharges FROM FormChargesT",4095,70,55),

	//UB04 Box 76 (Provider/Referring Info)
	CFormQuery(93,"SELECT FormChargesT.DoctorsProviders AS FirstOfDoctorsProviders, PersonT.[Last] AS DocLastName, PersonT.[First] AS DocFirstName, PersonT.[Middle] AS DocMiddleName, GetDate() AS TodaysDate, PersonT.[First] + ' ' + PersonT.[Middle] + ' ' + PersonT.[Last] + (CASE WHEN (PersonT.Title <> '') THEN (', ' + PersonT.Title) ELSE ('') END) AS DocSignature, PersonT.Address1 AS DocAdd1, PersonT.Address2 AS DocAdd2, PersonT.City + ', ' + PersonT.State + ' ' + PersonT.Zip AS DocAdd3, PersonT.SocialSecurity, ProvidersT.[Fed Employer ID], ProvidersT.[Medicare Number], '' AS Box51A, '' AS Box51B, '' AS Box51C "
				"FROM PersonT INNER JOIN (FormChargesT INNER JOIN ProvidersT ON FormChargesT.DoctorsProviders = ProvidersT.PersonID) ON (PersonT.ID = ProvidersT.PersonID) AND (PersonT.ID = ProvidersT.PersonID)",4095,70,55),

	// (j.jones 2007-07-12 08:56) - PLID 26621 - added UB04 Box 2 support
	//UB04 Box 2 Info
	// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
	CFormQuery(94,"SELECT TOP 1 LocationsT.*, City + ', ' + State + ' ' + Zip AS CityStateZip FROM LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID",4095,70,55), 

	CFormQuery(95,"",4095,85,60),
	CFormQuery(96,"",4095,85,60),
	CFormQuery(97,"",4095,85,60),
	CFormQuery(98,"",4095,85,60),
	CFormQuery(99,"",4095,85,60),

	// (k.messina 2010-02-16 02:57) - PLID 37323 Following queries 100 - 109 are part of the NY Medicaid Form
	//NewYorkMedicaid Patient Info
	CFormQuery(100,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last AS PatName, Address1 + ' ' + Address2 AS Address, City + ', ' + State + ' ' + Zip AS CityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PatFullAddress, "
				"EmployerAddress1 + ' ' + EmployerAddress2 + ' ' + EmployerCity + ' ' + EmployerState + ' ' + EmployerZip AS EmpAddress, PersonT.*, PatientsT.*, "
				"'Signature On File' AS SigOnFile, GetDate() AS Date "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",4095,85,60),

	//NewYorkMedicaid Primary Insurance Co Info
	CFormQuery(101,"SELECT Address1 + ' ' + Address2 AS PriAddress, City + ', ' + State + ' ' + Zip AS PriCityStateZip, "
				"Address1 + ' ' + Address2 + ', ' + City + ', ' + State + ' ' + Zip AS PriFullAddress, PersonT.*, InsuranceCoT.*, InsuredPartyT.* "
				"FROM PersonT INNER JOIN InsuranceCoT ON PersonT.ID = InsuranceCoT.PersonID INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID ",4095,85,60),

	//NewYorkMedicaid Bill/Charge/RefPhy/POS Info
	// (j.jones 2010-04-08 14:54) - PLID 38094 - added field for Box16A, unfilled in this query
	// (a.walling 2014-03-06 12:39) - PLID 61225 - Query text set by NYMedicaid form; cleared
	CFormQuery(102,"",4095,85,60),

	//NewYorkMedicaid Provider Info
	CFormQuery(103,"SELECT First + (CASE WHEN Middle <> '' AND Middle <> ' ' THEN ' ' + Left(Middle,1) ELSE '' END)  + ' ' + Last + ' ' + Title AS DocName, LocationsT.Name AS LocName, LocationsT.Address1 + ' ' + LocationsT.Address2 AS LocAddress, "
				"Location.Name As LocName, "
				"LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocCityStateZip, "
				"LocationsT.Address1 + ' ' + LocationsT.Address2 + ', ' + LocationsT.City + ', ' + LocationsT.State + ' ' + LocationsT.Zip AS LocFullAddress, LocationsT.Phone AS LocPhone, "
				"PersonT.*, ProvidersT.* "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID CROSS JOIN LocationsT INNER JOIN LineItemT ON LocationsT.ID = LineItemT.LocationID "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID",4095,85,60),

	//NewYorkMedicaid Charge Totals
	CFormQuery(104,"SELECT Convert(money,Sum(ChargeTotal)) AS TotalCharges FROM (SELECT TOP 7 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ ORDER BY ChargeTotal) AS HCFACharge6Q",4095,85,60),
	
	//NewYorkMedicaid Payment Totals
	CFormQuery(105,"SELECT Sum(ApplyTotal) AS TotalApplies FROM (SELECT TOP 7 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),
	
	//NewYorkMedicaid Balance
	CFormQuery(106,"SELECT Sum(Convert(money,(ChargeTotal)-(ApplyTotal))) AS TotalBalance FROM (SELECT TOP 7 HCFAChargeTopQ.ChargeTotal, HCFAChargeTopQ.ApplyTotal "
				"FROM (SELECT ChargeTotal, ApplyTotal FROM FormChargesT) AS HCFAChargeTopQ) AS HCFACharge6Q",4095,85,60),

	CFormQuery(107,"",4095,85,60),
	CFormQuery(108,"",4095,85,60),
	CFormQuery(109,"",4095,85,60),
};

const long g_nFormQueriesSize = sizeof(g_aryFormQueries) / sizeof(CFormQuery);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FormLayer::FormLayer(CWnd *wnd, _RecordsetPtr prsHistory)
{
	m_pWnd = wnd;
	//m_prsHistory = prsHistory;
	m_pFont = m_pStaticFont = m_pItalicFont = m_pMiniFont = NULL;
	m_rs.CreateInstance(__uuidof(Recordset));
}

FormLayer::~FormLayer()
{
	if (m_pFont)
		delete m_pFont;
	if (m_pMiniFont)
		delete m_pMiniFont;
	if (m_pStaticFont)
		delete m_pStaticFont;
	if (m_pItalicFont)
		delete m_pItalicFont;
	if (m_rs->State != adStateClosed)
		m_rs->Close();
}

void FormLayer::Load(int form, CString &where, CString &orderby, std::vector<shared_ptr<class FormControl>>& controls, int *i, long nSetupGroupID /* = -1*/, CStringArray* pastrParams, VarAry* pavarParams)
{		
	CString			formsql, datasql;
	FormControl		control;

	m_formID = form;
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("HCFADlg: Loading form %d WHERE %s", form, where);
	
	EnsureRemoteData();

	try
		{
		//fonts
		CFormQuery currentform;
		currentform = g_aryFormQueries[form];
		m_pFont		= new CFont;
		m_pStaticFont = new CFont;
		m_pItalicFont = new CFont;
		m_pMiniFont = new CFont;

#ifdef SHOW_SOURCE_ONLY
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pFont, 60, "Arial Narrow");
#else
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pFont, currentform.Font, "Arial Narrow");
#endif
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pStaticFont, currentform.StaticFont, "Small Fonts");
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pItalicFont, currentform.StaticFont, "Arial Italic");
		//better for screen
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pMiniFont, currentform.Font, "Arial Narrow");
//		m_pMiniFont->CreatePointFont(MINI_FONT_SIZE, "Arial");
			
		//data and controls
		datasql = currentform.sql;
		m_strWhere = where;
		datasql += where;
		m_strOrderBy = orderby;
		datasql += orderby;

		// (j.jones 2007-01-17 11:40) - PLID 24263 - this never supported multiple parameters before
		if(pastrParams != NULL && pavarParams != NULL &&
			pastrParams->GetSize() == pavarParams->GetSize()) {

			for (int i=0; i < pastrParams->GetSize(); i++) {

				CString temp1 = pastrParams->GetAt(i);
				_variant_t temp2 = pavarParams->GetAt(i);
				if(temp2.vt == VT_BOOL)
					if(temp2.boolVal == TRUE)
						datasql.Replace("["+ temp1 + "]","1");
					else datasql.Replace("["+ temp1 + "]","0");
				else if(temp2.vt == VT_BSTR) // CAH 12/7/01: Added _Q
					datasql.Replace("["+ temp1 + "]","'" + _Q(CString(temp2.bstrVal)) + "'");
				else if(temp2.vt == VT_I4) {
					CString temp;
					temp.Format("%li",temp2.lVal);
					datasql.Replace("["+ temp1 + "]",temp);
				}
				else {
					datasql.Replace("["+ temp1 + "]","''");
				}
			}
		}
		//LogDetail("HCFADlg: Loading data where SQL=%s", datasql);
	
		if (pastrParams) {
			VARIANT var;
 			for (int i=0; i < pastrParams->GetSize(); i++) {
				var = pavarParams->GetAt(i);

				//////////////////////////////////////////////
				// Add for future ::Refresh'es
				m_astrParams.Add(pastrParams->GetAt(i));
				m_avarParams.Add(var);

			}
		}

/*
		//for debugging
#ifdef _DEBUG
			CMsgBox dlg;
			dlg.msg = datasql;
			dlg.DoModal();
#endif
*/
		// (c.haag 2007-02-26 16:07) - PLID 24946 - Use CreateRecordsetStd to avoid formatting
		// any fields with percent signs in them
		//m_rs = CreateRecordset(adOpenStatic,adLockReadOnly,datasql);
		m_rs = CreateRecordsetStd(datasql, adOpenStatic, adLockReadOnly);

		_RecordsetPtr formRS(__uuidof(Recordset));

		formsql.Format("SELECT * FROM FormControlsT WHERE FormID = %i ORDER BY TabOrder", form);

		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("HCFADlg: Loading controls");
		// (c.haag 2007-02-26 16:21) - PLID 24946 - For completeness, same thing here
		formRS = CreateRecordsetStd(formsql);

		while (!formRS->eof)
		{
			control.format	= (int)(AdoFldLong(formRS, "Format",0));
			control.height	= (int)(AdoFldLong(formRS, "Height",0));
			control.id		= (int)(AdoFldLong(formRS, "ID",0));
			control.source	= AdoFldString(formRS, "Source","");//duh!
			control.value	= (int)(AdoFldLong(formRS, "Value",0));
			control.width	= (int)(AdoFldLong(formRS, "Width",0));
			control.x		= (int)(AdoFldLong(formRS, "X",0));
			control.y		= (int)(AdoFldLong(formRS, "Y",0));
			control.form	= form;
			control.color	= m_color;
			control.nID		= *i;
			*i = *i + 1;

			// (j.armen 2014-03-27 16:28) - PLID 60784 - use a shared_ptr
			shared_ptr<FormControl> pNewControl(control.CreateControl(this, nSetupGroupID));
			if (pNewControl)
				controls.push_back(pNewControl);
			formRS->MoveNext();
		}
		formRS->Close();
		m_rs->Close();
	}
	NxCatchAll("Error in loading form.");
}

void FormLayer::UnPunctuate(int form, std::vector<shared_ptr<FormControl>>& controls, CDWordArray *aryIDsToIgnore /* = NULL*/)
{
	for (unsigned int i = 0; i < controls.size(); i++)
	{
		FormControl* pControl = controls[i].get();

		//don't unpunctuate if NYWC
		if(form >= 60 && form < 70)
			return;

		if (pControl->form != form) continue;

		//if the aryIDsToIgnore is populated, do not punctuate the included IDs		
		BOOL bSkipControl = FALSE;
		if(aryIDsToIgnore != NULL) {
			for(int j=0;j<aryIDsToIgnore->GetSize() && !bSkipControl;j++) {
				if(pControl->id == (long)aryIDsToIgnore->GetAt(j))
					bSkipControl = TRUE;
			}
		}
		if(bSkipControl)
			continue;

		if (pControl->format & EDIT) {
			((FormEdit*)pControl)->UnPunctuate();
		}
	}
}

void FormLayer::Capitalize(int form, std::vector<shared_ptr<class FormControl>>& controls)
{
	for (unsigned int i = 0; i < controls.size(); i++) {
		FormControl* pControl = controls[i].get();

		if (pControl->form != form)
			continue;

		if (pControl->format & EDIT)
			((FormEdit*)pControl)->Capitalize();
	}
}

void FormLayer::Refresh(int form, std::vector<shared_ptr<class FormControl>>& controls)
{	
	COleVariant var;
	CString	 formsql, datasql;
	FormControl		*control, *pNewControl = NULL;

	form = m_formID;

	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("HCFADlg: Refreshing form %d WHERE %s", form, m_strWhere);

	for (unsigned int i = 0; i < controls.size(); i++)
	{
		if (((FormControl*)controls[i].get())->form == form)
			break;
	}
	if (i == controls.size())
		return;

	EnsureRemoteData();
	try
		{
		CString str;
		//fonts
		CFormQuery currform;
		currform = g_aryFormQueries[form];

		// Added by Chris 10/29 - Removes m_pFont from all controls
		// before deleting them
		for (unsigned i = 0; i < controls.size(); i++) {
			FormControl* pControl = controls[i].get();
			if (pControl->form != form)
				continue;
			pControl->ResetFont();
		}

		if (m_pFont) { m_pFont->DeleteObject(); delete m_pFont; }
		if (m_pStaticFont) { m_pStaticFont->DeleteObject(); delete m_pStaticFont; }
		if (m_pItalicFont) { m_pItalicFont->DeleteObject(); delete m_pItalicFont; }
		if (m_pMiniFont) { m_pMiniFont->DeleteObject(); delete m_pMiniFont; }

		m_pFont = m_pStaticFont = m_pItalicFont = NULL;

		m_pFont		= new CFont;
		m_pStaticFont = new CFont;
		m_pItalicFont = new CFont;
		m_pMiniFont = new CFont;

		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pFont, currform.Font, "Arial Narrow");
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pStaticFont, currform.StaticFont, "Small Fonts");
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pItalicFont, currform.StaticFont, "Arial Italic");
		//better for screen
		//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
		CreateCompatiblePointFont(m_pMiniFont, currform.Font, "Arial Narrow");
//		m_pMiniFont->CreatePointFont(MINI_FONT_SIZE, "Arial");
			
		//data and controls
		datasql = currform.sql;
		datasql += m_strWhere;
		datasql += m_strOrderBy;

		// (j.jones 2007-01-17 11:40) - PLID 24263 - this never supported multiple parameters before
		if(m_astrParams.GetSize() == m_avarParams.GetSize()) {

			for (int i=0; i < m_astrParams.GetSize(); i++) {
				CString temp1 = m_astrParams.GetAt(i);
				_variant_t temp2 = m_avarParams.GetAt(i);
				if(temp2.vt == VT_BOOL)
					if(temp2.boolVal == TRUE)
						datasql.Replace("["+ temp1 + "]","1");
					else datasql.Replace("["+ temp1 + "]","0");
				else if(temp2.vt == VT_BSTR) // CAH 12/7/01: Added _Q
					datasql.Replace("["+ temp1 + "]","'" + _Q(CString(temp2.bstrVal)) + "'");
				else if(temp2.vt == VT_I4) {
					CString temp;
					temp.Format("%li",temp2.lVal);
					datasql.Replace("["+ temp1 + "]",temp);
				}
				else {
					datasql.Replace("["+ temp1 + "]","''");
				}
			}
		}

		/*QUERYDEF == BAD
		if (m_pQueryDef->IsOpen())
			m_pQueryDef->Close();
		m_pQueryDef->Create(NULL, datasql);

		for (int i=0; i < m_astrParams.GetSize(); i++) {
			VARIANT var = m_avarParams.GetAt(i);

			if (var.vt == VT_NULL || (var.vt == VT_BSTRT && strlen(var.pcVal) == 0) || (var.vt == VT_BSTR && strlen(var.pcVal) == 0) || (var.vt == VT_EMPTY))
				m_pQueryDef->SetParamValueNull(m_astrParams.GetAt(i));
			else {
				m_pQueryDef->SetParamValue(m_astrParams.GetAt(i), var);
			}
		}*/

		/*CMsgBox dlg;  //for debugging
		dlg.msg = datasql;
		dlg.DoModal();*/

		// (c.haag 2007-02-26 16:21) - PLID 24946 - Use CreateRecordsetStd to avoid formatting
		// any fields with percent signs in them
		m_rs = CreateRecordsetStd(datasql);

		formsql.Format ("SELECT * FROM FormControlsT WHERE FormID = %li ORDER BY TabOrder", form);

		_RecordsetPtr formRS(__uuidof(Recordset));
		// (c.haag 2007-02-26 16:21) - PLID 24946 - And here for completeness
		formRS = CreateRecordsetStd(formsql);

		i=0;
		while (!formRS->eof && i < controls.size())
		{	
			control = controls[i].get();
			if (control->form != form) {
				i++;
				continue;
			}

			str.Format("ID = %li", control->id);
			formRS->MoveFirst();
			formRS->Find(_bstr_t(str),0,adSearchForward);

			if(formRS->eof) {
				//not found, meaning the control's form ID was changed while we had the HCFA open
				LogDetail("FormLayer::Refresh - **Unable to load form %li source %s**",form,str);
				AfxMessageBox("The settings for this form have been changed. To display the form with these new settings, you must close and re-open this claim.\n\n"
					"If you continue to get this message, please contact NexTech for assistance,");
				i++;
				continue;
			}

			control->format	= (int)(AdoFldLong(formRS, "Format",0));
			control->height	= (int)(AdoFldLong(formRS, "Height",0));
			control->id		= (int)(AdoFldLong(formRS, "ID",0));
			control->source = AdoFldString(formRS, "Source","");
			control->value	= (int)(AdoFldLong(formRS, "Value",0));
			control->width	= (int)(AdoFldLong(formRS, "Width",0));
			control->x		= (int)(AdoFldLong(formRS, "X",0));
			control->y		= (int)(AdoFldLong(formRS, "Y",0));
			control->color  = m_color;

			if (control->format & EDIT || control->format & STATIC) {
				((FormEdit*)control)->Refresh(this);
			}
			else if (control->format & DATE) {
				((FormDate*)control)->Refresh(this);
			}
			else if (control->format & CHECK) {
				((FormCheck*)control)->Refresh(this);
			}
			else if (control->format & LINE) {
				//do nothing
			}
			else {
				ASSERT(FALSE);
			}

			i++;
			//formRS.MoveNext();
		}
		if (formRS->State != adStateClosed)
			formRS->Close();

		if (m_rs->State != adStateClosed)
			m_rs->Close();
	}
	NxCatchAll("Error in FormLayer::Refresh");
}

void FormLayer::Save(int iDocumentID, int form, std::vector<shared_ptr<class FormControl>>& controls)
{	
	FormControl		*pNewControl = NULL;

	form = m_formID;

	for (unsigned int i = 0; i < controls.size(); i++)
	{
		if (controls[i]->form == form)
			break;
	}
	if (i == controls.size())
		return;

	try
		{
		CString str;
		//fonts

		// Added by Chris 10/29 - Removes m_pFont from all controls
		// before deleting them
		for (unsigned int i = 0; i < controls.size(); i++)
		{
			FormControl* pControl = controls[i].get();
			if (pControl->form != form)
				continue;

			if (pControl->format & EDIT || pControl->format & STATIC) {
				((FormEdit*)pControl)->Save(iDocumentID);
			}
			else if (pControl->format & DATE) {
				((FormDate*)pControl)->Save(iDocumentID);
			}
			else if (pControl->format & CHECK) {
				((FormCheck*)pControl)->Save(iDocumentID);
			}
		}
	}
	NxCatchAll("Error in saving document form");

}

void FormLayer::ChangeParameter(CString strParam, COleVariant var)
{
	for (int i=0 ; i < m_astrParams.GetSize(); i++) {
		if (m_astrParams.GetAt(i) == strParam) {
			m_avarParams.SetAt(i, var);
			break;
		}
	}

}