#include "stdafx.h"
#include "CCHITReportInfoListing.h"
#include "CCHITReportInfo.h"
#include "FirstDataBankUtils.h"
#include "PrescriptionUtilsNonAPI.h"

using namespace CCHITReportMeasures;

// (j.dinatale 2012-11-01 11:05) - PLID 53505 - moved the code define to the header

//(e.lally 2012-02-24) PLID 48266 - Moved all these report declarations from CCCHITReportsDlg
// (r.farnworth 2013-10-14 17:27) - PLID 58995 - Added a default parameter for determining what Meaningful Use Stage the user wanted to run reports on.
void CCHITReportInfoListing::LoadReportListing(MU::Stage eMeaningfulUseStage)
{
	//All reports to be displayed should call a function here to load their data.  Here is a rough model of what your 
	//	function should include.
/*
	{
		CCCHITReportInfo riReport;
		riReport.m_strInternalName = "Original report name sample"; //Identifier for the report internally
		riReport.m_strDisplayName = "Most current report name";	//Many times a requirement is given a new description and should be reflected here
		riReport.SetReportType(crtCCHIT); //Is this for CCHIT cert., Meaningful Use, etc.
		riReport.SetPercentToPass(x); //Use in the MU Progress bar in the EMR for measures that are not a count of patients
		//General:  This should describe the report and explain how date filters apply.
		//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
		//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
		riReport.SetHelpText("This help text describes the report in general.", 
			"This help text describes how the numerator is calculated.", 
			"This help text describes how the denominator is calculated.");
		//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
		riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT WHERE Last = 'Gremlin' "
			"AND FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo");
		//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
		riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
			"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo");
		m_aryReports.Add(riReport);
	}
*/
	//Note:  For 2011 CCHIT cert, please try to order the reports by their test scripts, so it is easier to demonstrate to the jurors.

	CWaitCursor pWait;

	// (d.thompson 2010-01-18) - PLID 36939 - ADM.15
	CreateA1CReport();
	// (d.thompson 2010-01-18) - PLID 36939 - ADM.16
	CreateHypertensiveReport();
	// (d.thompson 2010-01-18) - PLID 36939 - ADM.17
	CreateLDLReport();
	// (j.jones 2010-01-20 08:49) - PLID 36940 - ADM.18
	CreateSmokingCessationReport();
	// (j.jones 2010-01-20 08:49) - PLID 36940 - ADM.19
	CreateBMIReport();
	// (j.jones 2010-01-22 11:18) - PLID 37009 - ADM.20
	CreateCPOEReport_All();
	// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
	CreateCPOEReport_Labs();
	// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
	CreateCPOEReport_Prescriptions();
	// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
	CreateCPOEReport_ReferralOrders();
	// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.22
	CreateColorectalScreeningReport();
	// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.23
	CreateMammogramReport();
	// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.24
	CreateCardiacAspirinReport();
	// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.25
	CreateFluVaccineReport();
	
	// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.27 - requires FirstDataBank
	//(e.lally 2012-04-23) PLID 48266 - Only ensure the database is useable if the setting is enabled.
	bool bIsFDBDatabaseValid = false;
	// (z.manning 2013-10-09 11:35) - PLID 58927 - This now checks the license instead of a preference
	// that is now deprecated.
	if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent))
	{
		if(!m_bHasEnsuredFirstDataBankOnce) {
			m_bHasEnsuredFirstDataBankOnce = TRUE;
		}
		bIsFDBDatabaseValid = FirstDataBank::EnsureDatabase(NULL, true); // no UI
	}
	
	if(bIsFDBDatabaseValid){
		CreateGenericPrescriptionsReport();
	}
	
	// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.28
	CreateImagingServicesReport();
	// (j.jones 2010-01-20 08:56) - PLID 36943 - ADM.35
	CreateClinicalSummaryReport();
	// (j.jones 2010-01-20 08:56) - PLID 36943 - ADM.36
	CreateMedicationReconciliationReport();

	// (r.farnworth 2013-10-14 17:27) - PLID 58995 - Added a default parameter for determining what Meaningful Use Stage the user wanted to run reports on.
	if(eMeaningfulUseStage == MU::Stage1)
	{
		// (j.gruber 2010-09-20 11:50) - PLID 40789 - Use CPOE for medication orders
		CreateCPOEReport_MedicationOrders();

		// (j.gruber 2010-09-14 09:03) - PLID 40475 - Problems
		CreateProblemListReport();

		// (j.gruber 2010-09-10 13:14) - PLID 40477 - eprescriptions
		CreateEPrescriptionsReport();

		// (j.gruber 2010-09-10 10:32) - PLID 40472 - Current Medications
		CreateCurrentMedicationsReport();

		// (j.gruber 2010-09-10 10:33) - PLID 40472 - medication allergies
		CreateMedicationAllergiesReport();

		// (j.gruber 2010-09-10 09:27) - PLID 40473 - Demographic Reports
		//I took the others out because I thought it'd be confusing and I made sure we didn't need them.
		/*CreatePreferredLanguageReport();
		CreateGenderReport();
		CreateRaceReport();
		CreateEthnicityReport();
		CreateBirthDateReport();*/
		CreateDemographicsReport();
		
		// (j.gruber 2010-09-13 13:08) - PLID 40478 - Height, Weight, Blood Pressure
		CreateHeightWeightBPReport();

		// (j.gruber 2010-09-10 13:14) - PLID 40477 - smoking status
		CreateSmokingStatusReport();

		// (j.gruber 2010-09-14 08:58) - PLID 40479
		CreateElectronicCopyRequestReport();
		CreateTimelyClinicalSummaryReport();

		// (j.gruber 2010-09-13 11:52) - PLID 40477 - lab results
		CreateClinicalLabResultsReport();

		// (j.gruber 2010-09-20 14:37) - PLID 40479 - Reminders
		CreateRemindersReport();

		// (j.gruber 2010-09-13 14:20) - PLID 40476
		//CreateEducationResourceWellnessReport();
		CreateTimelyElectronicAccessReport();
		CreateEducationResourceEMRReport();

		// (j.gruber 2010-09-21 10:59) - PLID 40789 - Number of discharge patient who recieve summary of care
		CreateReferralMedReconReport();
		CreateDischargeSummaryReport();

		// (j.gruber 2010-09-13 13:08) - PLID 40478 - Height, Weight, Blood Pressure
		CreateBloodPressureReport();
		// (j.dinatale 2013-03-11 11:59) - PLID 54947 - need a report for patients with height and weight
		CreateHeightWeightReport();
		// (j.gruber 2010-09-13 13:08) - PLID 40478 - Height, Weight, Blood Pressure
		CreateHeightReport();
		CreateWeightReport();
	} 
	else if (eMeaningfulUseStage == MU::Stage2 || eMeaningfulUseStage == MU::ModStage2)
	{
		bool bUseModified = (eMeaningfulUseStage == MU::ModStage2 ? true : false);
		CreateStage2Reports(bUseModified);
	}

}

/************************************************************************************************************
							All Reports should be declared below here
************************************************************************************************************/


#pragma region CCHIT_REPORTS_OLD


// (d.thompson 2010-01-18) - PLID 36939 - ADM.15
void CCHITReportInfoListing::CreateA1CReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eA1C;
	// (j.gruber 2011-11-04 12:49) - PLID 45692
	riReport.m_strInternalName = "Diabetics with A1c under control";
	riReport.m_strDisplayName = "Diabetics with A1c under control";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many diabetic patients have their A1c levels under control.  Date "
		"filtering is applied to the patients First Contact Date.", 
		"All patients who are diabetic, having ICD-9 codes of '250.X' on General 2, Billing, or an EMR or EMR Problem and "
		"who also have a lab result named 'A1c' with a value > 9.0.", 
		"All patients who are diabetic, having ICD-9 codes of '250.X' on General 2, Billing, or an EMR or EMR Problem.");
	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (j.gruber 2014-03-17 13:03) - PLID 61398 - update billing structure
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "		
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT WHERE Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber LIKE '250%')) "
		"AND PersonT.ID IN "
		"(SELECT LabsT.PatientID FROM LabResultsT INNER JOIN LabsT ON LabResultsT.LabID = LabsT.ID "
		"WHERE LabResultsT.Name = 'A1c' AND "
		"LabsT.Deleted = 0 AND LabsT.Discontinued = 0 AND LabResultsT.Deleted = 0 AND "
		/*Note that we add the + 'e0' to confirm it's a floating point convertable number.  IsNumeric
			will return true for things like '$10' or '4,200', which are then not convertable to int/float/etc.*/
		"CASE WHEN isnumeric(convert(nvarchar, LabResultsT.Value) + 'e0') <> 0 THEN convert(float, convert(nvarchar, LabResultsT.Value)) ELSE 0 END > 9.0)");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:05) - PLID 61398 - update billing structure
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "	
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID from EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT WHERE Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber LIKE '250%'))");
	m_aryReports.Add(riReport);
}

// (d.thompson 2010-01-18) - PLID 36939 - ADM.16
void CCHITReportInfoListing::CreateHypertensiveReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	riReport.SetConfigureType(crctEMRDataGroup);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eHypertensive;
	// (j.gruber 2011-11-04 12:49) - PLID 45692
	riReport.m_strInternalName = "Hypertensive patients with BP under control";	
	riReport.m_strDisplayName = "Hypertensive patients with BP under control";	
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many hypertensive patients have their A1c levels under control.  Date "
		"filtering is applied to the patients First Contact Date.", 
		"All patients who are hypertensive, having ICD-9 codes of '401.X' through '405.X' on General 2, Billing, or an EMR or EMR Problem and "
		"who also have the EMR Item (as configured) for Blood Pressure under Control.",
		"All patients who are hypertensive, having ICD-9 codes of '401.X' through '405.X on General 2, Billing, or an EMR or EMR Problem.");
	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:05) - PLID 61398 - update billing structure
	riReport.SetNumeratorSql(FormatString("SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "		
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID from EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT WHERE Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber LIKE '401%%' OR CodeNumber LIKE '402%%' OR CodeNumber LIKE '403%%' "
		"OR CodeNumber LIKE '404%%' OR CodeNumber LIKE '405%%')) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"				WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"			)"
		"		OR "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"				WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) "
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable));

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:05) - PLID 61398 - update billing structure
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "				
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID from EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT WHERE Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber LIKE '401%' OR CodeNumber LIKE '402%' OR CodeNumber LIKE '403%' "
		"OR CodeNumber LIKE '404%' OR CodeNumber LIKE '405%'))");
	m_aryReports.Add(riReport);
}

// (d.thompson 2010-01-18) - PLID 36939 - ADM.17
void CCHITReportInfoListing::CreateLDLReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eLDL;
	// (j.gruber 2011-11-04 12:49) - PLID 45692
	riReport.m_strInternalName = "Patients with LDL under control";
	riReport.m_strDisplayName = "Patients with LDL under control";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many patients have their LDL-CHOLESTEROL levels under control.  Date "
		"filtering is applied to the patients First Contact Date.", 
		"All patients in the system who have a lab result of 'LDL-CHOLESTEROL' with a value under 100.",
		"All patients in the system.");
	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN "
		"(SELECT LabsT.PatientID FROM LabResultsT INNER JOIN LabsT ON LabResultsT.LabID = LabsT.ID "
		"WHERE LabResultsT.Name = 'LDL-CHOLESTEROL' AND "
		"LabsT.Deleted = 0 AND LabsT.Discontinued = 0 AND LabResultsT.Deleted = 0 AND "
		/*Note that we add the + 'e0' to confirm it's a floating point convertable number.  IsNumeric
			will return true for things like '$10' or '4,200', which are then not convertable to int/float/etc.*/
		"CASE WHEN isnumeric(convert(nvarchar, LabResultsT.Value) + 'e0') <> 0 THEN convert(float, convert(nvarchar, LabResultsT.Value)) ELSE 0 END < 100)");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	m_aryReports.Add(riReport);

}

// (j.jones 2010-01-20 08:49) - PLID 36940 - ADM.18
void CCHITReportInfoListing::CreateSmokingCessationReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eSmokingCessation;
	// (j.gruber 2011-11-04 12:49) - PLID 45692
	riReport.m_strInternalName = "Smokers offered cessation counseling";
	riReport.m_strDisplayName = "Smokers offered cessation counseling";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many smokers have been offered cessation counseling. Date "
		"filtering is applied to the patient's First Contact Date.",
		"All patients having ICD-9 codes of V15.82 or 305.1 on General 2, Billing, or an EMR or EMR Problem, "
		"with an EMN stating that smoking cessation counseling has been offered, defined by the selected EMR item in the configuration for this report.",
		"All patients having ICD-9 codes of V15.82 or 305.1 on General 2, Billing, or an EMR or EMR Problem.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:06) - PLID 61398 - update billing structure
	CString strSql;
	strSql.Format("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "		
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
		"WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		" "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT "
		"WHERE EMRProblemsT.Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber IN ('V15.82', '305.1'))) "
		" "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"				WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"			)"
		"		OR "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"				WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) "
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:06) - PLID 61398 - update billing structure
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "		
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
		"WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		" "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT "
		"WHERE EMRProblemsT.Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber IN ('V15.82', '305.1'))) ");

	m_aryReports.Add(riReport);	
}

// (j.jones 2010-01-20 08:49) - PLID 36940 - ADM.19
void CCHITReportInfoListing::CreateBMIReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eBMI;
	// (j.gruber 2011-11-04 12:50) - PLID 45692
	riReport.m_strInternalName = "Patients with BMI recorded";
	riReport.m_strDisplayName = "Patients with BMI recorded";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many patients have BMI recorded. Date "
		"filtering is applied to the patient's First Contact Date.", 
		"All patients with an EMN that has recorded the patient's BMI, defined by the selected EMR item in the configuration for this report.",
		"All patients in the system.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	CString strSql;
	strSql.Format("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"				WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"			)"
		"		OR "
		// (j.jones 2010-01-20 17:32) - For BMI only, if a table, we're going to look to see if any cell in that
		// table is filled in, because right now we test CCHIT with a BMI formula, and the formula cells in tables
		// do not currently save to data. So we can't check for that cell. The agreed-upon solution for CCHIT purposes
		// is to just find if any cell has been filled in for the same table that the selected EMRDataT record is in.
		// Since editing just one cell in the formula makes the result non-empty, and our CCHIT test only has cells
		// used in the formula, the outcome is the same.
		"			(EMRInfoT.DataType = %li "
		"				AND EMRDetailsT.EMRInfoID IN (SELECT EMRInfoID FROM EMRDataT WHERE EMRDataGroupID = @EMRDataGroupID) "
		"				AND EMRDetailsT.ID IN (SELECT EMRDetailID FROM EMRDetailTableDataT) "
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-22 11:18) - PLID 37009 - ADM.20
void CCHITReportInfoListing::CreateCPOEReport_All()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eCPOEReport_All;
	// (j.gruber 2011-11-04 12:50) - PLID 45692
	riReport.m_strInternalName = "Orders entered by physicians through CPOE";
	riReport.m_strDisplayName = "Orders entered by physicians through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many labs, medications, or referral orders were entered into order sets by physicians. Date "
		"filtering is applied to the lab, prescription, or referral order date.", 
		"All labs, medications, or referral orders entered into an order set by a physician.",
		"All labs, medications, or referral orders entered into an order set.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM ("
		"SELECT LabsT.ID FROM LabsT "
			"INNER JOIN UsersT ON LabsT.InputBy = UsersT.PersonID "
			"WHERE LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo "
			"AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
			"AND LabsT.OrderSetID IS NOT NULL "
			"AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 "
			//a NewCropUserTypeID of 1 is licensed prescriber, this determines the user was a provider
			//through the NewCrop setup, which is the only way this currently works
			"AND UsersT.NewCropUserTypeID = 1 AND UsersT.NewCropProviderID Is Not Null "
		"UNION SELECT PatientMedications.ID FROM PatientMedications "
			"INNER JOIN UsersT ON PatientMedications.InputByUserID = UsersT.PersonID "
			"WHERE PatientMedications.PrescriptionDate >= @DateFrom AND PatientMedications.PrescriptionDate < @DateTo "
			"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
			"AND PatientMedications.OrderSetID IS NOT NULL "
			"AND PatientMedications.Deleted = 0 "
			//a NewCropUserTypeID of 1 is licensed prescriber, this determines the user was a provider
			//through the NewCrop setup, which is the only way this currently works
			"AND UsersT.NewCropUserTypeID = 1 AND UsersT.NewCropProviderID Is Not Null "
		"UNION SELECT ReferralOrdersT.ID FROM ReferralOrdersT "
			"INNER JOIN UsersT ON ReferralOrdersT.InputUserID = UsersT.PersonID "
			"WHERE ReferralOrdersT.Date >= @DateFrom AND ReferralOrdersT.Date < @DateTo "
			"AND ReferralOrdersT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
			"AND ReferralOrdersT.OrderSetID IS NOT NULL "
			//a NewCropUserTypeID of 1 is licensed prescriber, this determines the user was a provider
			//through the NewCrop setup, which is the only way this currently works
			"AND UsersT.NewCropUserTypeID = 1 AND UsersT.NewCropProviderID Is Not Null "
		") CPOEQ ");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM ("
		"SELECT LabsT.ID FROM LabsT "
			"WHERE LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo "
			"AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
			"AND LabsT.OrderSetID IS NOT NULL "
			"AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 "
		"UNION SELECT PatientMedications.ID FROM PatientMedications "
			"WHERE PatientMedications.PrescriptionDate >= @DateFrom AND PatientMedications.PrescriptionDate < @DateTo "
			"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
			"AND PatientMedications.OrderSetID IS NOT NULL "
			"AND PatientMedications.Deleted = 0 "
		"UNION SELECT ReferralOrdersT.ID FROM ReferralOrdersT "
			"WHERE ReferralOrdersT.Date >= @DateFrom AND ReferralOrdersT.Date < @DateTo "
			"AND ReferralOrdersT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
			"AND ReferralOrdersT.OrderSetID IS NOT NULL "
		") CPOEQ ");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
void CCHITReportInfoListing::CreateCPOEReport_Labs()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eCPOEReport_Labs;
	// (j.gruber 2011-11-04 12:50) - PLID 45692
	riReport.m_strInternalName = "Lab orders entered by physicians through CPOE";
	riReport.m_strDisplayName = "Lab orders entered by physicians through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many labs were entered into order sets by physicians. Date "
		"filtering is applied to the lab date.", 
		"All labs entered into an order set by a physician.",
		"All labs entered into an order set.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetNumeratorSql("SELECT COUNT(LabsT.ID) AS NumeratorValue FROM LabsT "
		"INNER JOIN UsersT ON LabsT.InputBy = UsersT.PersonID "
		"WHERE LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo "
		"AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND LabsT.OrderSetID IS NOT NULL "
		"AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 "
		//a NewCropUserTypeID of 1 is licensed prescriber, this determines the user was a provider
		//through the NewCrop setup, which is the only way this currently works
		"AND UsersT.NewCropUserTypeID = 1 AND UsersT.NewCropProviderID Is Not Null");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetDenominatorSql("SELECT COUNT(LabsT.ID) AS DenominatorValue FROM LabsT "
		"WHERE LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo "
		"AND LabsT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND LabsT.OrderSetID IS NOT NULL "
		"AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
void CCHITReportInfoListing::CreateCPOEReport_Prescriptions()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eCPOEReport_Prescriptions;
	// (j.gruber 2011-11-04 12:50) - PLID 45962
	riReport.m_strInternalName = "Medication orders entered by physicians through CPOE";
	riReport.m_strDisplayName = "Medication orders entered by physicians through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many medications were entered into order sets by physicians. Date "
		"filtering is applied to the prescription date.", 
		"All medications entered into an order set by a physician.",
		"All medications entered into an order set.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(PatientMedications.ID) AS NumeratorValue FROM PatientMedications "
		"INNER JOIN UsersT ON PatientMedications.InputByUserID = UsersT.PersonID "
		"WHERE PatientMedications.PrescriptionDate >= @DateFrom AND PatientMedications.PrescriptionDate < @DateTo "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PatientMedications.OrderSetID IS NOT NULL "
		"AND PatientMedications.Deleted = 0 "
		//a NewCropUserTypeID of 1 is licensed prescriber, this determines the user was a provider
		//through the NewCrop setup, which is the only way this currently works
		"AND UsersT.NewCropUserTypeID = 1 AND UsersT.NewCropProviderID Is Not Null"
		);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(PatientMedications.ID) AS DenominatorValue FROM PatientMedications "
		"WHERE PatientMedications.PrescriptionDate >= @DateFrom AND PatientMedications.PrescriptionDate < @DateTo "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PatientMedications.OrderSetID IS NOT NULL "
		"AND PatientMedications.Deleted = 0");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
void CCHITReportInfoListing::CreateCPOEReport_ReferralOrders()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eCPOEReport_ReferralOrders;
	// (j.gruber 2011-11-04 12:51) - PLID 45962
	riReport.m_strInternalName = "Referral orders entered by physicians through CPOE";
	riReport.m_strDisplayName = "Referral orders entered by physicians through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many referral orders were entered into order sets by physicians. Date "
		"filtering is applied to the referral order date.", 
		"All referral orders entered into an order set by a physician.",
		"All referral orders entered into an order set.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(ReferralOrdersT.ID) AS NumeratorValue FROM ReferralOrdersT "
		"INNER JOIN UsersT ON ReferralOrdersT.InputUserID = UsersT.PersonID "
		"WHERE ReferralOrdersT.Date >= @DateFrom AND ReferralOrdersT.Date < @DateTo "
		"AND ReferralOrdersT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND ReferralOrdersT.OrderSetID IS NOT NULL "
		//a NewCropUserTypeID of 1 is licensed prescriber, this determines the user was a provider
		//through the NewCrop setup, which is the only way this currently works
		"AND UsersT.NewCropUserTypeID = 1 AND UsersT.NewCropProviderID Is Not Null");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ReferralOrdersT.ID) AS DenominatorValue FROM ReferralOrdersT "
		"WHERE ReferralOrdersT.Date >= @DateFrom AND ReferralOrdersT.Date < @DateTo "
		"AND ReferralOrdersT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND ReferralOrdersT.OrderSetID IS NOT NULL");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.22
void CCHITReportInfoListing::CreateColorectalScreeningReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eColorectalScreening;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Patients over 50 with colorectal cancer screenings";
	riReport.m_strDisplayName = "Patients over 50 with colorectal cancer screenings";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many patients over the age of 50 who have had a colorectal cancer screening. Date "
		"filtering is applied to the patient's First Contact Date.", 
		"All patients who are over the age of 50, having service codes of 45378, 45380, 45381, 45383, 45384, 45385, G0105, or G0121 on a Bill or an EMR.",
		"All patients who are over the age of 50.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.BirthDate IS NOT NULL "
		"AND PersonT.BirthDate < GetDate() "
		"AND YEAR(GetDate()-PersonT.BirthDate)-1900 >= 50 "
		"AND ( "
		"	PersonT.ID IN ("
		"		SELECT BillsT.PatientID FROM BillsT "
		"		INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
		"		AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
				//filter on the ItemCode, not the current CPTCode.Code
		"		AND ChargesT.ItemCode IN ('45378', '45380', '45381', '45383', '45384', '45385', 'G0105', 'G0121') "
		"	) "
		"OR "
		"	PersonT.ID IN ("
		"		SELECT EMRMasterT.PatientID FROM EMRMasterT "
		"		INNER JOIN EMRChargesT ON EMRMasterT.ID = EMRChargesT.EMRID "
		"		INNER JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"		WHERE EMRChargesT.Deleted = 0 AND EMRMasterT.Deleted = 0 "
		"		AND CPTCodeT.Code IN ('45378', '45380', '45381', '45383', '45384', '45385', 'G0105', 'G0121') "
		"	) "
		")");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.BirthDate IS NOT NULL "
		"AND PersonT.BirthDate < GetDate() "
		"AND YEAR(GetDate()-PersonT.BirthDate)-1900 >= 50");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.23
void CCHITReportInfoListing::CreateMammogramReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMammogram;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Female patients over 50 with mammograms";
	riReport.m_strDisplayName = "Female patients over 50 with mammograms";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many female patients over the age of 50 who have have had a mammogram. Date "
		"filtering is applied to the patient's First Contact Date.", 
		"All female patients who are over the age of 50, having service codes of 76082, 76083, 76085, 76090, 76091, 76092, G0202, G0203, G0204, G0205, G0206, G0207, or G0236 on a Bill or an EMR.",
		"All female patients who are over the age of 50.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.Gender = 2 "
		"AND PersonT.BirthDate IS NOT NULL "
		"AND PersonT.BirthDate < GetDate() "
		"AND YEAR(GetDate()-PersonT.BirthDate)-1900 >= 50 "
		"AND ( "
		"	PersonT.ID IN ("
		"		SELECT BillsT.PatientID FROM BillsT "
		"		INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
		"		AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
				//filter on the ItemCode, not the current CPTCode.Code
		"		AND ChargesT.ItemCode IN ('76082', '76083', '76085', '76090', '76091', '76092', 'G0202', 'G0203', 'G0204', 'G0205', 'G0206', 'G0207', 'G0236') "
		"	) "
		"OR "
		"	PersonT.ID IN ("
		"		SELECT EMRMasterT.PatientID FROM EMRMasterT "
		"		INNER JOIN EMRChargesT ON EMRMasterT.ID = EMRChargesT.EMRID "
		"		INNER JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"		WHERE EMRChargesT.Deleted = 0 AND EMRMasterT.Deleted = 0 "
		"		AND CPTCodeT.Code IN ('76082', '76083', '76085', '76090', '76091', '76092', 'G0202', 'G0203', 'G0204', 'G0205', 'G0206', 'G0207', 'G0236') "
		"	) "
		")");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.Gender = 2 "
		"AND PersonT.BirthDate IS NOT NULL "
		"AND PersonT.BirthDate < GetDate() "
		"AND YEAR(GetDate()-PersonT.BirthDate)-1900 >= 50");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.24
void CCHITReportInfoListing::CreateCardiacAspirinReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eCardiacAspirin;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Patients at high-risk for cardiac events, on aspirin prophylaxis";
	riReport.m_strDisplayName = "Patients at high-risk for cardiac events, on aspirin prophylaxis";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many patients at high-risk for cardiac events are taking aspirin prophylazis medication.  Date "
		"filtering is applied to the patients First Contact Date.", 
		"All patients who are at high-risk for cardiac events, having ICD-9 codes of '414.X' on General 2, Billing, or an EMR or EMR Problem and "
		"who also have a current medication or prescription for aspirin.", 
		"All patients who are at high-risk for cardiac events, having ICD-9 codes of '414.X' on General 2, Billing, or an EMR or EMR Problem.");
	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:07) - PLID 61398 - update billing structure
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
		"WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		" "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT "
		"WHERE EMRProblemsT.Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber LIKE '414.%' OR CodeNumber = '414')) "
		" "
		"AND ("
		"	PersonT.ID IN ("
		"		SELECT PatientID FROM PatientMedications "
		"		INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"		WHERE DrugList.DrugName = 'Aspirin' OR DrugList.DrugName LIKE 'Aspirin %' OR DrugList.DrugName LIKE 'Aspirin-%' "
		"	)"
		"OR "
		"	PersonT.ID IN ("
		"		SELECT PatientID FROM CurrentPatientMedsT "
		"		INNER JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID "
		"		WHERE DrugList.DrugName = 'Aspirin' OR DrugList.DrugName LIKE 'Aspirin %' OR DrugList.DrugName LIKE 'Aspirin-%' "
		"	)"
		")");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2014-03-17 13:07) - PLID 61398 - update billing structure
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND PersonT.ID IN (SELECT PersonID FROM "
		"(SELECT PersonID, DefaultDiagID1 AS DiagID FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID2 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID3 FROM PatientsT "
		"UNION SELECT PersonID, DefaultDiagID4 FROM PatientsT "
		" "
		"UNION SELECT BillsT.PatientID, BillDiagCodeT.ICD9DiagID FROM BillsT INNER JOIN BillDiagCodeT ON BillsT.ID = BillDiagCodeT.BillID WHERE BillsT.Deleted = 0 AND BillDiagCodeT.OrderIndex >= 1 AND BillDiagCodeT.OrderIndex <=4  "
		" "
		"UNION SELECT EMRMasterT.PatientID, DiagCodeID FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
		"WHERE EMRMasterT.Deleted = 0 AND EMRDiagCodesT.Deleted = 0 "
		" "
		"UNION SELECT PatientID, DiagCodeID FROM EMRProblemsT "
		"WHERE EMRProblemsT.Deleted = 0 "
		") SubQ "
		"WHERE DiagID IN (SELECT ID FROM DiagCodes WHERE CodeNumber LIKE '414.%' OR CodeNumber = '414'))");
	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.25
void CCHITReportInfoListing::CreateFluVaccineReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eFluVaccine;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Patients who have received a flu vaccine";
	riReport.m_strDisplayName = "Patients who have received a flu vaccine";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many patients have received a flu vaccine. Date "
		"filtering is applied to the patient's First Contact Date.", 
		"All patients having service codes of G0008, 90655, 90656, 90657, 90658, or 90660 on a Bill or an EMR.",
		"All patients in the system.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND ( "
		"	PersonT.ID IN ("
		"		SELECT BillsT.PatientID FROM BillsT "
		"		INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
		"		AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
				//filter on the ItemCode, not the current CPTCode.Code
		"		AND ChargesT.ItemCode IN ('G0008', '90655', '90656', '90657', '90658', '90660') "
		"	) "
		"OR "
		"	PersonT.ID IN ("
		"		SELECT EMRMasterT.PatientID FROM EMRMasterT "
		"		INNER JOIN EMRChargesT ON EMRMasterT.ID = EMRChargesT.EMRID "
		"		INNER JOIN CPTCodeT ON EMRChargesT.ServiceID = CPTCodeT.ID "
		"		WHERE EMRChargesT.Deleted = 0 AND EMRMasterT.Deleted = 0 "
		"		AND CPTCodeT.Code IN ('G0008', '90655', '90656', '90657', '90658', '90660') "
		"	) "
		")");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE FirstContactDate >= @DateFrom AND FirstContactDate < @DateTo "
		"AND PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.27
void CCHITReportInfoListing::CreateGenericPrescriptionsReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eGenericPrescriptions;

	// (j.jones 2011-04-21 10:48) - PLID 43285 - because this report query references FirstDataBank,
	// we cannot use the snapshot connection, so set that flag to false
	riReport.m_bCanUseSnapshot = false;

	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Prescriptions for generic medications";
	riReport.m_strDisplayName = "Prescriptions for generic medications";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many prescriptions were entered for generic medications, when a generic option was available.  Date "
		"filtering is applied to the date of the prescription.", 
		"All prescriptions for coded generic medications, where a non-generic was an available option. A medication is considered to be coded if it has a valid NDC code entered.", 
		"All prescriptions for coded medications where generic was an available option. A medication is considered to be coded if it has a valid NDC code entered.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(PatientMedications.ID) AS NumeratorValue FROM PatientMedications "
		"WHERE PatientMedications.PrescriptionDate >= @DateFrom AND PatientMedications.PrescriptionDate < @DateTo "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND ("		
			//search by NDC
		"	PatientMedications.ID IN (SELECT PatientMedications.ID FROM PatientMedications "
		"		INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				//RMINDC1_NDC_MEDID is a one-to-many lookup of NDC->MEDID. NDC is unique, and MEDID can exist multiple times.
				//This query works since we are searching by NDC.
		"		INNER JOIN FirstDataBank..RMINDC1_NDC_MEDID ON DrugList.NDCNumber = FirstDataBank..RMINDC1_NDC_MEDID.NDC "
		"		INNER JOIN FirstDataBank..RMIID1_MED ON FirstDataBank..RMINDC1_NDC_MEDID.MEDID = FirstDataBank..RMIID1_MED.MEDID "
				//this checks that the medication is a generic version of another medication
		"		AND FirstDataBank..RMIID1_MED.GENERIC_MEDID = 0 AND FirstDataBank..RMIID1_MED.MEDID IN (SELECT GENERIC_MEDID FROM FirstDataBank..RMIID1_MED) "
		"		) "
		"	OR "
			//search by NewCropID - now FDBID			
		"	PatientMedications.ID IN (SELECT PatientMedications.ID FROM PatientMedications "
		"		INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				//NewCropID is the FirstDataBank ID -- now FDBID
				// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
		"		INNER JOIN FirstDataBank..RMIID1_MED ON DrugList.FDBID = FirstDataBank..RMIID1_MED.MEDID "
				//this checks that the medication is a generic version of another medication
		"		AND FirstDataBank..RMIID1_MED.GENERIC_MEDID = 0 AND FirstDataBank..RMIID1_MED.MEDID IN (SELECT GENERIC_MEDID FROM FirstDataBank..RMIID1_MED) "
		"		) "
		")");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(PatientMedications.ID) AS DenominatorValue FROM PatientMedications "
		"WHERE PatientMedications.PrescriptionDate >= @DateFrom AND PatientMedications.PrescriptionDate < @DateTo "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND ("		
			//search by NDC
		"	PatientMedications.ID IN (SELECT PatientMedications.ID FROM PatientMedications "
		"		INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				//RMINDC1_NDC_MEDID is a one-to-many lookup of NDC->MEDID. NDC is unique, and MEDID can exist multiple times.
				//This query works since we are searching by NDC.
		"		INNER JOIN FirstDataBank..RMINDC1_NDC_MEDID ON DrugList.NDCNumber = FirstDataBank..RMINDC1_NDC_MEDID.NDC "
		"		INNER JOIN FirstDataBank..RMIID1_MED ON FirstDataBank..RMINDC1_NDC_MEDID.MEDID = FirstDataBank..RMIID1_MED.MEDID "
		"		WHERE ("
					//this checks that the medication is a generic version of another medication
		"			(FirstDataBank..RMIID1_MED.GENERIC_MEDID = 0 AND FirstDataBank..RMIID1_MED.MEDID IN (SELECT GENERIC_MEDID FROM FirstDataBank..RMIID1_MED)) "
		"			OR "
					//this checks that the medication is non-generic, and a generic version exists
		"			FirstDataBank..RMIID1_MED.GENERIC_MEDID > 0 "
		"		)"
		"	) "
		"	OR "
			//search by NewCropID
			// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
		"	PatientMedications.ID IN (SELECT PatientMedications.ID FROM PatientMedications "
		"		INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
				//NewCropID is the FirstDataBank ID - now FDBID
		"		INNER JOIN FirstDataBank..RMIID1_MED ON DrugList.FDBID = FirstDataBank..RMIID1_MED.MEDID "
		"		WHERE ("
					//this checks that the medication is a generic version of another medication
		"			(FirstDataBank..RMIID1_MED.GENERIC_MEDID = 0 AND FirstDataBank..RMIID1_MED.MEDID IN (SELECT GENERIC_MEDID FROM FirstDataBank..RMIID1_MED)) "
		"			OR "
					//this checks that the medication is non-generic, and a generic version exists
		"			FirstDataBank..RMIID1_MED.GENERIC_MEDID > 0 "
		"		)"
		"	) "
		")");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.28
void CCHITReportInfoListing::CreateImagingServicesReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eImagingServices;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Patients with imaging services and indications recorded";
	riReport.m_strDisplayName = "Patients with imaging services and indications recorded";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many patients who have had high-cost imaging services also have specific structured indications recorded. Date "
		"filtering is applied to the service date of the charge.", 
		"All patients who have service codes of 70450, 70460, 70486, 70490, 70491, 70543, 70551-70544, 70547, 71250, 71270, "
		"72125, 72128, 72195, 72131, 72141, 72142, 72146-72149, 72191, 72193, 73040, 73221, 73222, 73721, 74150, 74160, 74170, 74175, 74183, "
		"76375, 78007, 78010, 78070, 78223, 78264, 78306, 78468, 78473, 78708, 78709, 78815, A9517, or C8902 on a Bill, "
		"and also have a diagnosis code selected for that service on the Bill",
		"All patients who have service codes of 70450, 70460, 70486, 70490, 70491, 70543, 70551-70544, 70547, 71250, 71270, "
		"72125, 72128, 72195, 72131, 72141, 72142, 72146-72149, 72191, 72193, 73040, 73221, 73222, 73721, 74150, 74160, 74170, 74175, 74183, "
		"76375, 78007, 78010, 78070, 78223, 78264, 78306, 78468, 78473, 78708, 78709, 78815, A9517, or C8902 on a Bill.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE ID IN (SELECT PatientsT.PersonID FROM PatientsT "
		"	INNER JOIN BillsT ON PatientsT.PersonID = BillsT.PatientID "
		"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			// (j.gruber 2014-02-28 13:21) - PLID 61104 - change whichcode structure
			// this will do the same as the previous whichcode where clause did, ensure there are whichcodes selected		
		"   INNER JOIN ChargeWhichCodesT ON ChargesT.ID = ChargeWhichCodesT.ChargeID "
		"	WHERE LineItemT.Date >= @DateFrom AND LineItemT.Date < @DateTo "
		"	AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"	AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
		"	AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			//filter on the ItemCode, not the current CPTCode.Code
		"	AND ChargesT.ItemCode IN ('70450', '70460', '70486', '70490', '70491', '70543', '70551', '70552', '70553', '70544', '70547', '71250', '71270', "
		"	'72125', '72195', '72128', '72131', '72141', '72142', '72146', '72147', '72149', '72148', '72191', '72193', '73040', '73221', '73222', '73721', "
		"	'74150', '74160', '74170', '74175', '74183', '76375', '78007', '78010', '78070', '78223', '78264', '78306', '78468', '78473', '78708', '78709', "
		"	'78815', 'A9517', 'C8902') "		
			//I can't find any recorded occurrence of (none) actually saving, but since this is not
			//constrained by any foreign key, I figure it's safe to also throw in that check
		// (j.gruber 2014-02-28 13:23) - PLID 61104 - accounted for with inner join above
		//	AND ChargesT.WhichCodes <> '' AND WhichCodes <> '(none)' "
		")");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM PersonT "
		"WHERE ID IN (SELECT PatientsT.PersonID FROM PatientsT "
		"	INNER JOIN BillsT ON PatientsT.PersonID = BillsT.PatientID "
		"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	WHERE LineItemT.Date >= @DateFrom AND LineItemT.Date < @DateTo "
		"	AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		"	AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
		"	AND BillsT.EntryType = 1 AND LineItemT.Type = 10 "
			//filter on the ItemCode, not the current CPTCode.Code
		"	AND ChargesT.ItemCode IN ('70450', '70460', '70486', '70490', '70491', '70543', '70551', '70552', '70553', '70544', '70547', '71250', '71270', "
		"	'72125', '72195', '72128', '72131', '72141', '72142', '72146', '72147', '72149', '72148', '72191', '72193', '73040', '73221', '73222', '73721', "
		"	'74150', '74160', '74170', '74175', '74183', '76375', '78007', '78010', '78070', '78223', '78264', '78306', '78468', '78473', '78708', '78709', "
		"	'78815', 'A9517', 'C8902') "
		")");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:56) - PLID 36943 - ADM.35
void CCHITReportInfoListing::CreateClinicalSummaryReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eClinicalSummary;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Encounters where clinical summaries were provided";
	riReport.m_strDisplayName = "Encounters where clinical summaries were provided";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many EMNs state that clinical summaries were provided to the patient. Date "
		"filtering is applied to the EMN Date.", 
		"All EMNs that have clinical summaries provided, defined by the selected EMR item in the configuration for this report.",
		"All EMNs in the system.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	CString strSql;
	strSql.Format("SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo "
		"AND EMRMasterT.Deleted = 0 "
		"AND EMRMasterT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND EMRMasterT.ID IN ("
		"	SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"	WHERE EMRDetailsT.Deleted = 0 "
		"	AND ("
		"		(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"			SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"			INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"			WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"		)"
		"	OR "
		"		(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"			SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"			INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"			INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"			WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) "
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo "
		"AND EMRMasterT.Deleted = 0 "
		"AND EMRMasterT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])");

	m_aryReports.Add(riReport);
}

// (j.jones 2010-01-20 08:56) - PLID 36943 - ADM.36
void CCHITReportInfoListing::CreateMedicationReconciliationReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtCCHIT);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMedicationReconciliation;
	// (j.gruber 2011-11-04 12:52) - PLID 45962
	riReport.m_strInternalName = "Encounters where medication reconciliation was performed";
	riReport.m_strDisplayName = "Encounters where medication reconciliation was performed";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	riReport.SetHelpText("This report calculates how many EMNs state that medication reconciliation was performed. Date "
		"filtering is applied to the EMN Date.", 
		"All EMNs that have medication reconciliation performed, defined by the selected EMR item in the configuration for this report.",
		"All EMNs in the system.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	CString strSql;
	strSql.Format("SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo "
		"AND EMRMasterT.Deleted = 0 "
		"AND EMRMasterT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"AND EMRMasterT.ID IN ("
		"	SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"	INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"	WHERE EMRDetailsT.Deleted = 0 "
		"	AND ("
		"		(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"			SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"			INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"			WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"		)"
		"	OR "
		"		(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"			SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"			INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"			INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"			WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) "
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo "
		"AND EMRMasterT.Deleted = 0 "
		"AND EMRMasterT.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])");

	m_aryReports.Add(riReport);
}
#pragma endregion //CCHIT_REPORTS_OLD


// (j.gruber 2010-09-10 09:27) - PLID 40473 - Demographic Reports
void CCHITReportInfoListing::CreateDemographicsReport()
{

	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUDemographics;
	// (j.gruber 2011-11-04 12:53) - PLID 45692
	riReport.m_strInternalName = "MU.04 - Patients who have all Demographic fields filled out.";
	riReport.m_strDisplayName = "MU.CORE.07 - Patients who have all Demographic fields filled out.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 13:50) - PLID 43764 - set location filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have the Language, Gender, Race, Ethnicity, and Birthdate fields filled out. Filtering is applied on EMN date, EMN location, and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients in the denominator who have the Birthdate, Gender, Race, Ethnicity, and Language fields filled out.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity structure to support "Declined".  Declined DOES count as a positive numerator
	// (d.thompson 2012-08-14) - PLID 52165 - Reworked language structure to support "Declined".  Declined DOES count as a positive numerator
	riReport.SetNumeratorSql("/*MU.04/MU.CORE.07*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		// (b.spivey, May 28, 2013) - PLID 56869 - Changed this to check the tag table for races. 
		"LEFT JOIN ( "		
		"	SELECT DISTINCT PersonID, Count(RaceID) AS PatientHasRace "
		"	FROM PersonRaceT "
		"	GROUP BY PersonID ) RaceSubQ ON RaceSubQ.PersonID = PatientsT.PersonID "
		"WHERE PersonT.LanguageID IS NOT NULL AND PersonT.Gender IN (1,2) AND RaceSubQ.PatientHasRace IS NOT NULL AND PersonT.Ethnicity IS NOT NULL AND PersonT.Birthdate IS NOT NULL "
		"	AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "		
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (j.gruber 2011-10-27 16:28) - PLID 46160 - use the default
	riReport.SetDefaultDenominatorSql();		

	// (j.gruber 2011-11-10 10:01) - PLID 46503
	CString strNum, strSelect, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity to support declined.  Declined DOES count as a positive numerator
	// (d.thompson 2012-08-14) - PLID 52165 - Reworked language structure to support "Declined".  Declined DOES count as a positive numerator
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Has Language' as MiscDesc, CASE WHEN PersonT.LanguageID IS NOT NULL then 'Yes' else 'No' END as ItemMisc, "
		" 'Has Gender' as Misc2Desc, CASE WHEN Gender IN (1,2) THEN 'Yes' ELSE 'No' END as ItemMisc2, "
		" 'Has Race' as Misc3Desc, CASE WHEN RaceSubQ.HasRaceID IS NOT NULL THEN 'Yes' ELSE 'No' END as ItemMisc3, "
		" 'Has Ethnicity' as Misc4Desc, CASE WHEN PersonT.Ethnicity IS NOT NULL THEN 'Yes' ELSE 'No' END  as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		// (b.spivey, May 28, 2013) - PLID 56869 - change this to check the tag table for races. 
		" LEFT JOIN ( "
		"		SELECT DISTINCT PersonID, Count(RaceID) AS HasRaceID "
		"		FROM PersonRaceT "
		"		GROUP BY PersonID ) RaceSubQ ON RaceSubQ.PersonID = PatientsT.PersonID "
		" WHERE PersonT.LanguageID IS NOT NULL AND PersonT.Gender IN (1,2) AND RaceSubQ.HasRaceID IS NOT NULL AND PersonT.Ethnicity IS NOT NULL AND PersonT.Birthdate IS NOT NULL "
		" AND EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity to support declined.  Declined DOES count as a positive numerator
	// (d.thompson 2012-08-14) - PLID 52165 - Reworked language structure to support "Declined".  Declined DOES count as a positive numerator
		strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Has Language' as MiscDesc, CASE WHEN PersonT.LanguageID IS NOT NULL then 'Yes' else 'No' END as ItemMisc, "
		" 'Has Gender' as Misc2Desc, CASE WHEN Gender IN (1,2) THEN 'Yes' ELSE 'No' END as ItemMisc2, "
		" 'Has Race' as Misc3Desc, CASE WHEN RaceSubQ.HasRaceID IS NOT NULL THEN 'Yes' ELSE 'No' END as ItemMisc3, "
		" 'Has Ethnicity' as Misc4Desc, CASE WHEN PersonT.Ethnicity IS NOT NULL THEN 'Yes' ELSE 'No' END  as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "		
		// (b.spivey, May 28, 2013) - PLID 56869 - Changed this to check the tag table for races. 
		" LEFT JOIN ( "
		"		SELECT DISTINCT PersonID, Count(RaceID) AS HasRaceID "
		"		FROM PersonRaceT "
		"		GROUP BY PersonID ) RaceSubQ ON RaceSubQ.PersonID = PatientsT.PersonID "
		" WHERE EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}


// (j.gruber 2010-09-10 10:33) - PLID 40472 - Current Medications
void CCHITReportInfoListing::CreateCurrentMedicationsReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUCurrentMedications;
	// (j.gruber 2011-11-04 12:54) - PLID 45692
	riReport.m_strInternalName = "MU.02 - Patients who have active Current Medications.";
	riReport.m_strDisplayName = "MU.CORE.05 - Patients who have active Current Medications.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 13:52) - PLID 43764
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have at least one active current medication or have an indication that the patient is not taking any medications. Filtering is applied on EMN Date, EMN location and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients in the denominator who have a current medication or have indicated that they are taking no medications.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.jones 2012-11-05 09:57) - PLID 53563 - added support for PatientsT.HasNoMeds, which indicates that they have no current meds
	riReport.SetNumeratorSql("/*MU.02/MU.CORE.05*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE (PatientsT.HasNoMeds = 1 OR PersonT.ID IN (SELECT PatientID FROM CurrentPatientMedsT WHERE Discontinued = 0)) "
		"	AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "		
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (j.gruber 2011-10-27 16:28) - PLID 46160 - use the default
	riReport.SetDefaultDenominatorSql();					

	// (j.gruber 2011-11-16 14:11) - PLID 46502
	CString strNum;

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.jones 2013-03-26 11:21) - PLID 53563 - added support for PatientsT.HasNoMeds, which indicates that they have no current meds
	strNum = " FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE (PatientsT.HasNoMeds = 1 OR PersonT.ID IN (SELECT PatientID FROM CurrentPatientMedsT WHERE Discontinued = 0)) "
		" AND EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ";

	riReport.SetReportInfo("", strNum, "");


	m_aryReports.Add(riReport);
}

// (j.gruber 2010-09-10 10:33) - PLID 40472 - Medication Allergies
void CCHITReportInfoListing::CreateMedicationAllergiesReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUMedicationAllergies;
	// (j.gruber 2011-11-04 12:55) - PLID 45692
	riReport.m_strInternalName = "MU.03 - Patients who have active Allergies.";
	riReport.m_strDisplayName = "MU.CORE.06 - Patients who have active Allergies.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 13:52) - PLID 43764 - added location filtering and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have at least one active allergy or have an indication that the patient has no allergies. Filtering is applied on EMN Date, EMN location, and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients in the denominator who have an allergy or have indicated that they have no allergies.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.jones 2012-11-05 10:24) - PLID 53564 - added support for PatientsT.HasNoAllergies, which indicates that they have no allergies
	riReport.SetNumeratorSql("/*MU.03/MU.CORE.06*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE (PatientsT.HasNoAllergies = 1 OR PersonT.ID IN (SELECT PersonID FROM PatientAllergyT WHERE Discontinued = 0)) "
		"	AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "				
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (j.gruber 2011-10-27 16:28) - PLID 46160 - use the default
	riReport.SetDefaultDenominatorSql();				

	// (j.gruber 2011-11-10 09:46) - PLID 46503 - reports
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.jones 2013-03-26 11:22) - PLID 53564 - added support for PatientsT.HasNoAllergies, which indicates that they have no allergies
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Allergies' as MiscDesc, dbo.GetActiveAllergyList(PatientsT.PersonID) as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE (PatientsT.HasNoAllergies = 1 OR PersonT.ID IN (SELECT PersonID FROM PatientAllergyT WHERE Discontinued = 0)) "
		" AND EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}

// (j.gruber 2010-10-04 12:25) - PLID 40477
void CCHITReportInfoListing::CreateEPrescriptionsReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUEPrescriptions;
	//(e.lally 2012-03-21) PLID 48707
	riReport.SetPercentToPass(40);
	// (j.gruber 2011-11-04 13:17) - PLID 45692
	riReport.m_strInternalName = "MU.08 - Prescriptions sent electronically.";
	riReport.m_strDisplayName = "MU.CORE.04 - Prescriptions sent electronically.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 13:55) - PLID 43764 - set location filter and descriptions	
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many active permissible prescriptions have been sent electronically through NewCrop. Filtering is applied on the prescription date, prescription location, and the prescription provider. ",
		"The number of all active permissible prescriptions in the denominator that were prescribed electronically through NewCrop or NexERx.",
		"The number of all active permissible prescriptions with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (j.gruber 2011-11-18 10:12) - PLID 46532 - we are including fax, electronic retail, and electronic mail order
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.jones 2016-02-05 10:54) - PLID 67981 - added DispensedInHouse as a valid status,
	// and switched the hardcoded statuses to use the enums
	riReport.SetNumeratorSql("/*MU.08/MU.CORE.04*/\r\n"
		"SELECT COUNT(PatientMedications.ID) AS NumeratorValue FROM PatientMedications  "
		" LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"WHERE DELETED = 0 AND Discontinued = 0  "
		"AND DEASchedule NOT IN ('II', 'III', 'IV', 'V') "
		" AND ( (NewCropGUID IS NOT NULL AND FinalDestinationType IN (2,3,4)) "
		"	OR (NewCropGUID IS NULL AND  QueueStatus IN (" +
			FormatString("%li, %li, %li, %li", (long)PrescriptionQueueStatus::pqseTransmitSuccess, (long)PrescriptionQueueStatus::pqseTransmitPending, (long)PrescriptionQueueStatus::pqseFaxed, (long)PrescriptionQueueStatus::pqseDispensedInHouse)
		+ "	)) ) "
		//"AND FinalDestinationType IN (2,3,4) "
		//take out final status type
		/*"AND FinalStatusType IN (1,5) "*/
		"AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 [PatientIDFilter]) "
		);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("/*MU.08/MU.CORE.04*/\r\n"
		"SELECT COUNT(PatientMedications.ID) AS DenominatorValue FROM PatientMedications  "
		" LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"WHERE DELETED = 0 AND Discontinued = 0  "
		"AND DEASchedule NOT IN ('II', 'III', 'IV', 'V') "
		"AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 [PatientIDFilter]) "
		);
	
	// (j.gruber 2011-11-10 09:46) - PLID 46502 - reports
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.jones 2016-02-05 10:54) - PLID 67981 - added DispensedInHouse as a valid status,
	// and switched the hardcoded statuses to use the enums
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'Prescription' as ItemDescriptionLine, PrescriptionDate as ItemDate, LEFT(EMRDataT.Data,255) as ItemDescription, \r\n"
		" PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ItemProvider, '' as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, 'Destination Type' as MiscDesc, \r\n"
		" CASE WHEN FinalDestinationType = 0 THEN 'Not Transmitted' WHEN FinalDestinationType = 1 THEN 'Print' WHEN FinalDestinationType = 2 THEN 'Fax' WHEN FinalDestinationType = 3 THEN 'Electronic Retail' WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder' WHEN FinalDestinationType = 5 THEN 'Test' END as ItemMisc, \r\n"
		" 'DEA Schedule' as Misc2Desc, DrugList.DEASchedule as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN PatientMedications ON PersonT.ID = PatientMedications.PatientID \r\n"		
		" INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID \r\n"				
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		" LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID \r\n"
		" LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
		" WHERE DELETED = 0 AND Discontinued = 0  \r\n"
		//" AND FinalDestinationType IN (2,3, 4) "
		" AND ( (NewCropGUID IS NOT NULL AND FinalDestinationType IN (2,3,4)) "
		"	OR (NewCropGUID IS NULL AND  QueueStatus IN (" +
			FormatString("%li, %li, %li, %li", (long)PrescriptionQueueStatus::pqseTransmitSuccess, (long)PrescriptionQueueStatus::pqseTransmitPending, (long)PrescriptionQueueStatus::pqseFaxed, (long)PrescriptionQueueStatus::pqseDispensedInHouse)
		+ ")) ) "
		" AND DEASchedule NOT IN ('II', 'III', 'IV', 'V') \r\n"
		" AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N \r\n";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'Prescription' as ItemDescriptionLine, PrescriptionDate as ItemDate, LEFT(EMRDataT.Data,255) as ItemDescription, \r\n"
		" PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ItemProvider, '' as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, 'Destination Type' as MiscDesc, \r\n"
		" CASE WHEN FinalDestinationType = 0 THEN 'Not Transmitted' WHEN FinalDestinationType = 1 THEN 'Print' WHEN FinalDestinationType = 2 THEN 'Fax' WHEN FinalDestinationType = 3 THEN 'Electronic Retail' WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder' WHEN FinalDestinationType = 5 THEN 'Test' END as ItemMisc, \r\n"
		" 'DEA Schedule' as Misc2Desc, DrugList.DEASchedule as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN PatientMedications ON PersonT.ID = PatientMedications.PatientID \r\n"
		" INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID \r\n"				
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		" LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID \r\n"
		" LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
		" WHERE DELETED = 0 AND Discontinued = 0  \r\n"		
		" AND DEASchedule NOT IN ('II', 'III', 'IV', 'V') \r\n"
		" AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P \r\n";
	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}


// (j.gruber 2010-09-10 16:46) - PLID 40477 - created
void CCHITReportInfoListing::CreateSmokingStatusReport()
{
	
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUSmokingStatus;
	riReport.SetConfigureType(crctEMRItem);
	// (j.gruber 2011-11-04 13:18) - PLID 45692
	riReport.m_strInternalName = "MU.10 - Patients who have smoking status reported.";
	riReport.m_strDisplayName = "MU.CORE.09 - Patients who have smoking status reported.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 13:57) - PLID 43764 - set location filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients who were 13 years of age or older when they were seen and had a smoking status recorded. Filtering is applied on EMN date, EMN location, and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients age 13 or older in the denominator where smoking status has been recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients age 13 or older who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (r.gonet 06/12/2013) - PLID 55151 - Get the patient age temp table table name.
	CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	CString strNum;
	strNum.Format("/*MU.10/MU.CORE.09*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"
		"	 WHERE EMRMasterT.Deleted = 0 		"
		"    [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "
		//(s.dhole 8/1/2014 3:59 PM ) - PLID 63043 it should join on emrid not patient id
		" AND EMRMasterT.ID IN (SELECT EMRMasterT.ID FROM EMRMasterT "
		" INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0 AND PatientAgeT.PatientAgeNumber >= 13 ) "
		" ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, strPatientAgeTempTableName);

	riReport.SetNumeratorSql(strNum);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	riReport.SetDenominatorSql(FormatString("/*MU.10/MU.CORE.09*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"	
		"	 INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	 WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND PatientAgeT.PatientAgeNumber >= 13) "		
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ",
		strPatientAgeTempTableName));
	

	// (j.gruber 2011-11-10 13:56) - PLID 45603
	CString strReportNum, strReportDenom;
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE  "
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "
		//(s.dhole 8/1/2014 3:59 PM ) - PLID  63043 it should join on emrid not patient id
		" AND EMRMasterT.ID IN (SELECT EMRMasterT.ID FROM EMRMasterT "
		" INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0 AND PatientAgeT.PatientAgeNumber >= 13 ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, strPatientAgeTempTableName);

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND PatientAgeT.PatientAgeNumber >= 13 "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName);

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);

}

// (j.gruber 2010-09-10 16:46) - PLID 40477 - created
void CCHITReportInfoListing::CreateClinicalLabResultsReport()
{
	
	CCCHITReportInfo riReport;	
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUClinicalLabResults;
	//(e.lally 2012-03-21) PLID 48707
	riReport.SetPercentToPass(40);
	riReport.SetConfigureType(crctMailsentCat);
	// (j.gruber 2011-11-04 13:18) - PLID 45692
	riReport.m_strInternalName = "MU.11 - Clinical Labs having results in structured data.";
	// (s.dhole 2014-05-9 15:33) - PLID 62184 - Change display name
	riReport.m_strDisplayName = "MU.MENU.04 - Clinical Labs having results in structured data.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 13:58) - PLID 43765 - set location filter and descriptions	
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	riReport.SetHelpText("This report calculates how many clinical labs have positive, negative, or numerical results entered in as structured data.  Clinical labs are those associated with a lab type that is Biopsy, Lab Work, or Cultures. Filters on Lab Result Received Date, Lab Location, and Lab Provider for lab data and History Service Date, Patient Location, and Patient Provider for document data.",
		"The number of all biopsy, labwork, and culture type labs in the denominator that have a flag selected or a number as the first character in the value field.",
		"The number of all biopsy, labwork, and culture type labs with the selected provider(s) and location and any history document for a patient with the selected provider(s) and location with a category stating it is a lab not entered into the Labs tab, defined by the history category in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 10:50) - PLID 62064 - Trim leading/trailing spaces for LabResultsT.Value
	riReport.SetNumeratorSql("/*MU.11/MU.MENU.02*/\r\n"
		" SELECT Count(*) as NumeratorValue FROM LabsT "
		"WHERE LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo  "
		"	 AND (LabResultsT.FlagID IS NOT NULL "
		"	 OR CASE WHEN isnumeric(convert(nvarchar, Left(CONVERT(nVarChar(255), LTRIM(RTRIM(LabResultsT.Value))), 1)) + 'e0') <> 0 THEN 1 ELSE 0 END = 1) "
		") "
		" [LabProvFilter] [LabLocFilter] "
		"AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		"AND PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 08:34) - PLID 62051 - Fix Stage 1  - Clinical Lab Results Summary and Detailed Report so that the denominator includes all labs; 
	// not only the ones with a Flag set or a first character numeric value
	riReport.SetDenominatorSql("/*MU.11/MU.MENU.02*/\r\n"
		"SELECT Sum(DenominatorValue) AS DenominatorValue FROM ( "
		"SELECT Count(*) as DenominatorValue FROM LabsT "
		"WHERE LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0 "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo  "		
		") "
		" [LabProvFilter] [LabLocFilter] "
		"AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		"AND PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" UNION ALL SELECT Count(*) FROM MailSent WHERE  "
		"			PersonID IN (SELECT PersonID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter] ) "
		"			AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		"			AND CategoryID = @MailSentCatID "
		")Q"
		);

	// (j.gruber 2011-11-16 14:23) - PLID 46505
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 10:50) - PLID 62064 - Trim leading/trailing spaces for LabResultsT.Value
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate, "
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 "
		" AND LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo  "		
		"	 AND (LabResultsT.FlagID IS NOT NULL "
		"	 OR CASE WHEN isnumeric(convert(nvarchar, Left(CONVERT(nVarChar(255), LTRIM(RTRIM(LabResultsT.Value))), 1)) + 'e0') <> 0 THEN 1 ELSE 0 END = 1) "
		") "
		" [LabProvFilter] [LabLocFilter] "
		" AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 08:34) - PLID 62051 - Fix Stage 1  - Clinical Lab Results Summary and Detailed Report so that the denominator includes all labs; 
	// not only the ones with a Flag set or a first character numeric value
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate,	"
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 "
		"	AND LabsT.DELETED = 0 AND LabsT.Discontinued = 0 "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo  "		
		") "
		" [LabProvFilter] [LabLocFilter] "
		" AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter]  "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "		
		") P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
	
}

// (j.gruber 2010-09-13 13:10) - PLID 40478 - Created
void CCHITReportInfoListing::CreateHeightReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUHeight;
	// (j.gruber 2011-11-04 13:18) - PLID 45692
	riReport.m_strInternalName = "Patients with Height recorded";
	riReport.m_strDisplayName = "Patients with Height recorded";
	
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items
	riReport.SetConfigureType(crctEMRDataGroupOrSlider);

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:01) - PLID 43765 - set location filter and descriptions
	// (j.gruber 2011-11-07 16:23) - PLID 45365 - exclusions
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Height recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have the height recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (r.gonet 06/12/2013) - PLID 55151 - Get the patient age temp table table name.
	//CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.PH*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	riReport.SetDenominatorSql("/*MU.PH*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	// (j.gruber 2011-11-10 13:56) - PLID 46504
	CString strReportNum, strReportDenom;

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE  \r\n"
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"	
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND EMRMasterT.ID IN (\r\n"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT \r\n"
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)\r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, eitSlider);

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0  \r\n"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);
}

void CCHITReportInfoListing::CreateWeightReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUWeight;
	// (j.gruber 2011-11-04 13:06) - PLID 45692
	riReport.m_strInternalName = "Patients with Weight recorded";
	riReport.m_strDisplayName = "Patients with Weight recorded";
	
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items
	riReport.SetConfigureType(crctEMRDataGroupOrSlider);

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:04) - PLID 43765 - set location filter and descriptions
	// (j.gruber 2011-11-07 16:23) - PLID 45365 - exclusions
	// (j.gruber 2012-01-27 11:20) - PLID 47840 - fixed typo
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Weight recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have the weight recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (r.gonet 06/12/2013) - PLID 55151 - Get the patient age temp table table name.
	//CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.PW*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	riReport.SetDenominatorSql("/*MU.PW*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	// (j.gruber 2011-11-10 13:56) - PLID 46504
	CString strReportNum, strReportDenom;

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE "
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"		
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"	AND EMRMasterT.ID IN ( \r\n"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT \r\n"
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	) \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, eitSlider);

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - This no longer filters on age
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0  \r\n"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] \r\n");

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);
}

void CCHITReportInfoListing::CreateBloodPressureReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUBloodPressure;
	// (j.gruber 2011-11-04 13:06) - PLID 45692
	riReport.m_strInternalName = "Patients with Blood Pressure recorded";
	riReport.m_strDisplayName = "Patients with Blood Pressure recorded";

	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items
	riReport.SetConfigureType(crctEMRDataGroupOrSlider);

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:05) - PLID 43765 - set location filter and descriptions
	// (j.gruber 2011-11-07 16:23) - PLID 45365 - exclusions
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Blood Pressure recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients age 3 or older in the denominator that have blood pressure recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients age 3 or older who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (r.gonet 06/12/2013) - PLID 55151 - Get the patient age temp table table name.
	CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.PBP*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] and PatientAgeT.PatientAgeNumber >= 3 "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")", strPatientAgeTempTableName, eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3
	riReport.SetDenominatorSql(FormatString("/*MU.PBP*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] and PatientAgeT.PatientAgeNumber >=3 ) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName));

	// (j.gruber 2011-11-10 13:56) - PLID 46504
	CString strReportNum, strReportDenom;

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE  \r\n"
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"		
		" AND PatientAgeT.PatientAgeNumber >= 3  \r\n"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"	AND EMRMasterT.ID IN ( \r\n"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT \r\n"
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)\r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName, eitSingleList, eitMultiList, eitTable, eitSlider);

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0  \r\n"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   \r\n"
		" AND PatientAgeT.PatientAgeNumber >= 3  \r\n"		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName);

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);
}

// (j.gruber 2010-10-04 12:26) - PLID 40478
void CCHITReportInfoListing::CreateHeightWeightBPReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUHeightWeightBP;
	// (j.gruber 2011-11-04 13:06) - PLID 45692
	riReport.m_strInternalName = "MU.09 - Patients with Height, Weight, and Blood Pressure recorded";
	riReport.m_strDisplayName = "MU.CORE.08 - Patients with Vitals recorded";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:06) - PLID 43764 - set location filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3 for BP, and removed age filter for Height and Weight
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Height and Weight recorded, and how many patients older than 3 have Blood Pressure recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).",
		"The number of unique patients in the denominator where the height and weight have been recorded, and if over the age of 3, the blood pressure has also been recorded, defined by the selected EMR item in the configuration for those reports.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (r.gonet 06/12/2013) - PLID 55151 - Get the patient age temp table table name.
	CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	// (j.jones 2014-11-06 10:28) - PLID 63983 - cached ConfigRT values to streamline query plans
	// (j.jones 2014-11-07 10:08) - PLID 63993 - moved the EMR detail searches to return their EMR IDs in one temp table
	riReport.SetInitializationSQL(FormatString("SET NOCOUNT ON \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded int; \r\n"
		"SET @cchitPatientsWithHeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Height recorded') \r\n"
		"\r\n"
		"DECLARE @cchitPatientsWithHeightRecorded2 int; \r\n"
		"SET @cchitPatientsWithHeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Height recorded2') \r\n"
		"\r\n"
		"DECLARE @cchitPatientsWithWeightRecorded int; \r\n"
		"SET @cchitPatientsWithWeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Weight recorded') \r\n"
		"\r\n"
		"DECLARE @cchitPatientsWithWeightRecorded2 int; \r\n"
		"SET @cchitPatientsWithWeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Weight recorded2') \r\n"
		"\r\n"
		"DECLARE @cchitPatientsWithBloodPressureRecorded int; \r\n"
		"SET @cchitPatientsWithBloodPressureRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Blood Pressure recorded') \r\n"
		"\r\n"
		"DECLARE @cchitPatientsWithBloodPressureRecorded2 int; \r\n"
		"SET @cchitPatientsWithBloodPressureRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Blood Pressure recorded2') \r\n"

		////////////////////
		//We need a list of all EMNs (unfiltered), that have Height, Weight, or Blood Pressure tracked
		"DECLARE @EMRMasterT_Match TABLE (ID INT, Height BIT, Weight BIT, BP BIT) \r\n"
		"\r\n"
		"INSERT INTO @EMRMasterT_Match (ID, Height, Weight, BP) \r\n"
		//Get all single/multi-select lists
		"SELECT EMRMasterT.ID, \r\n"
		"	(CASE WHEN EMRDataT.EMRDataGroupID = @cchitPatientsWithHeightRecorded THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRDataT.EMRDataGroupID = @cchitPatientsWithWeightRecorded THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRDataT.EMRDataGroupID = @cchitPatientsWithBloodPressureRecorded THEN 1 ELSE 0 END) \r\n"
		"FROM EMRMasterT WITH(NOLOCK) \r\n"
		"INNER JOIN EMRDetailsT WITH(NOLOCK) ON EMRMasterT.ID = EMRDetailsT.EMRID \r\n"
		"INNER JOIN EMRSelectT WITH(NOLOCK) ON EMRDetailsT.ID = EMRSelectT.EmrDetailID \r\n"
		"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"INNER JOIN EMRDataT WITH(NOLOCK) ON EMRSelectT.EMRDataID = EMRDataT.ID \r\n"
		"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType IN (%li, %li) \r\n"
		"AND EMRDataT.EMRDataGroupID IN (@cchitPatientsWithHeightRecorded, @cchitPatientsWithWeightRecorded, @cchitPatientsWithBloodPressureRecorded) \r\n"
		"\r\n"
		//Get all tables
		"UNION "
		"SELECT EMRMasterT.ID, \r\n"
		"	(CASE WHEN(EMRDataT_X.EMRDataGroupID = @cchitPatientsWithHeightRecorded OR EMRDataT_Y.EMRDataGroupID = @cchitPatientsWithHeightRecorded) THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN(EMRDataT_X.EMRDataGroupID = @cchitPatientsWithWeightRecorded OR EMRDataT_Y.EMRDataGroupID = @cchitPatientsWithWeightRecorded) THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN(EMRDataT_X.EMRDataGroupID = @cchitPatientsWithBloodPressureRecorded OR EMRDataT_Y.EMRDataGroupID = @cchitPatientsWithBloodPressureRecorded) THEN 1 ELSE 0 END) \r\n"
		"FROM EMRMasterT WITH(NOLOCK) \r\n"
		"INNER JOIN EMRDetailsT WITH(NOLOCK) ON EMRMasterT.ID = EMRDetailsT.EMRID \r\n"
		"INNER JOIN EMRDetailTableDataT WITH(NOLOCK) ON EMRDetailsT.ID = EMRDetailTableDataT.EMRDetailID \r\n"
		"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"INNER JOIN EMRDataT EMRDataT_X WITH(NOLOCK) ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID \r\n"
		"INNER JOIN EMRDataT EMRDataT_Y WITH(NOLOCK) ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID \r\n"
		"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType = %li \r\n"
		"AND (\r\n"
		"	EMRDataT_X.EMRDataGroupID IN (@cchitPatientsWithHeightRecorded, @cchitPatientsWithWeightRecorded, @cchitPatientsWithBloodPressureRecorded) \r\n"
		"	OR \r\n"
		"	EMRDataT_Y.EMRDataGroupID IN (@cchitPatientsWithHeightRecorded, @cchitPatientsWithWeightRecorded, @cchitPatientsWithBloodPressureRecorded) \r\n"
		") \r\n"
		"\r\n"
		//Get all sliders
		"UNION \r\n"
		"SELECT EMRMasterT.ID, \r\n"
		"	(CASE WHEN EMRInfoT.EMRInfoMasterID = @cchitPatientsWithHeightRecorded2 THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRInfoT.EMRInfoMasterID = @cchitPatientsWithWeightRecorded2 THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRInfoT.EMRInfoMasterID = @cchitPatientsWithBloodPressureRecorded2 THEN 1 ELSE 0 END) \r\n"
		"FROM EMRMasterT WITH(NOLOCK) \r\n"
		"INNER JOIN EMRDetailsT WITH(NOLOCK) ON EMRMasterT.ID = EMRDetailsT.EMRID \r\n"
		"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType = %li AND EMRDetailsT.SliderValue Is Not Null \r\n"
		"AND EMRInfoT.EMRInfoMasterID IN (@cchitPatientsWithHeightRecorded2, @cchitPatientsWithWeightRecorded2, @cchitPatientsWithBloodPressureRecorded2) \r\n"
		"\r\n"
		"SET NOCOUNT OFF \r\n",
		eitSingleList, eitMultiList, eitTable, eitSlider));

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3 for BP, and removed age filter for Height and Weight
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.09/MU.CORE.08*/\r\n"

		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter]  "
		"	AND EMRMasterT.ID IN ("
		"		SELECT ID FROM @EMRMasterT_Match WHERE Height = 1 "
		"	)"
		" )"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"   [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT ID FROM @EMRMasterT_Match WHERE Weight = 1 "
		"	)"
		" )"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND (PatientAgeT.PatientAgeNumber < 3 OR (EMRMasterT.ID IN ("
		"		SELECT ID FROM @EMRMasterT_Match WHERE BP = 1 "
		"	)))"
		")",
		strPatientAgeTempTableName);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3 for BP, and removed age filter for Height and Weight
	riReport.SetDenominatorSql("/*MU.09/MU.CORE.08*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");


	// (j.gruber 2011-11-10 09:46) - PLID 46504 - reports
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 11/26/2013 - PLID 59838 - This was including people who had height OR weight OR bp, fixed it to AND them
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3 for BP, and removed age filter for Height and Weight
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND EMRMasterT.ID IN ("
		"	SELECT ID FROM @EMRMasterT_Match WHERE Height = 1 "
		" )"
		" AND EMRMasterT.ID IN ("
		"	SELECT ID FROM @EMRMasterT_Match WHERE Weight = 1 "
		" )"
		" AND (PatientAgeT.PatientAgeNumber < 3 OR (EMRMasterT.ID IN ("
		"	SELECT ID FROM @EMRMasterT_Match WHERE BP = 1 "
		")))"
		") N "
		, strPatientAgeTempTableName);

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/11/2013 - PLID 59972 - Changed age filter from 2 to 3 for BP, and removed age filter for Height and Weight
	strDenom.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ");


	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}

// (j.gruber 2010-09-13 16:07) - PLID 40476
void CCHITReportInfoListing::CreateEducationResourceEMRReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUEducationResourceEMR;
	// (j.gruber 2011-11-04 13:07) - PLID 45692
	riReport.m_strInternalName = "MU.05 - Patients with Educational Resources Sent";
	// (b.savon 2014-05-14 07:19) - PLID 62129 - Rename MUS1 Menu 6 to MUS1 Menu 5 and update the numerator text
	// (s.dhole 2014-05-9 15:33) - PLID 62184 - Change display name
	riReport.m_strDisplayName = "MU.MENU.07 - Patients with Educational Resources Sent";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:08) - PLID 43764 - set location filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have been provided patient-specific educational resources. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have been provided with educational resources, defined by the selected EMR item in the configuration for this report.  The EMR item can be substituted with patient education generated through the blue informational buttons or hyperlinks.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (b.savon 2014-05-14 07:19) - PLID 62129 - Rename MUS1 Menu 6 to MUS1 Menu 5 and update the numerator text
	// (r.farnworth 2014-05-14 15:06) - PLID 62137 - Patient Education Resources Stage 1 Menu 6 should automatically count in the numerator when a user clicks on any of our informational buttons or hyperlinks.
	CString strSql;
	strSql.Format("/*MU.05/MU.MENU.05*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "				
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND ( "
		"		EMRMasterT.ID IN ("
		"			SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"			INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"			WHERE EMRDetailsT.Deleted = 0 "		
		"			AND ("
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"					WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"				)"
		"			OR "	
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"					WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) "
		"				)"
		"			)"
		"		)"
		"		OR EMRMasterT.PatientID IN (SELECT PatientID from EducationResourceAccessT) "	
		"	)"
		//(s.dhole 8/1/2014 3:36 PM ) - PLID 63088  Should apply to main Query and aslo on EMRID
		" AND EMRMasterT.ID IN (SELECT EMRMasterT.ID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		") "
		, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (j.gruber 2011-10-27 16:28) - PLID 46160 - use the default
	riReport.SetDefaultDenominatorSql();				

	
	// (j.gruber 2011-11-10 13:56) - PLID 46506
	CString strNum;
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (r.farnworth 2014-05-14 15:06) - PLID 62137 - Patient Education Resources Stage 1 Menu 6 should automatically count in the numerator when a user clicks on any of our informational buttons or hyperlinks.
	strNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE  "
		"  EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	AND ( "
		"		EMRMasterT.ID IN ("
		"			SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"			INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"			WHERE EMRDetailsT.Deleted = 0 "		
		"			AND ("
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"					WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) "
		"				)"
		"			OR "	
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"					WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) "
		"				)"
		"			)"
		"		)"
		"		OR EMRMasterT.PatientID IN (SELECT PatientID from EducationResourceAccessT) "
		"	)"
		//(s.dhole 8/1/2014 3:40 PM ) - PLID 63088 Should be EMRID
		" AND EMRMasterT.ID IN (SELECT EMRMasterT.ID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable);

	riReport.SetReportInfo("", strNum, "");

	m_aryReports.Add(riReport);
}

void CCHITReportInfoListing::CreateTimelyElectronicAccessReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUTimelyElectronicAccess;
	riReport.m_strInternalName = "MU.06 - Patients provided with timely Electronic Access ";
	// (b.savon 2014-05-14 07:10) - PLID 62128 - Rename MUS1 Menu 5 to MUS1 Core 11
	riReport.m_strDisplayName = "MU.CORE.11 - Patients provided with timely Electronic Access ";	
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:10) - PLID 43764 - set location filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	// (s.dhole 2014-05-19 14:22) - PLID 62187 - Change the numerator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have been provided electronic access in at least four business days. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s). A business day is defined as Monday - Friday.", 
		"The number of unique patients in the denominator who have a merged clinical summary or summary of care plus a security code or a NexWeb patient login created on or before 4 business days of the EMN date.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (b.savon 2014-05-14 07:10) - PLID 62128 - Rename MUS1 Menu 5 to MUS1 Core 11
	CString strSql;
	strSql.Format("/*MU.06/MU.CORE.11*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "				
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN "
		"	(SELECT PatientID FROM ( "
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the InitialSecurityCodeCreationDate since SecurityCodeCreationDate is cleared out when the patient creates a username with the security code.
		"	SELECT EMRMasterT.PatientID, "
		"	DATEDIFF(d,Date, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) "
		"	- DATEDIFF(wk,Date, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) * 2  "
		"	 - CASE  "
		"	WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) = 'Saturday' THEN 1  "
		"	WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) <> 'Saturday' THEN -1  "
		"	ELSE 0 "
		"	END AS BusinessDays, "
		"  "
		// (r.farnworth 2014-05-19 11:13) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"	DATEDIFF(d,Date, MergeDocsT.MergeDate) "
		"	- DATEDIFF(wk,Date, MergeDocsT.MergeDate) * 2  "
		"	 - CASE  "
		"	WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) = 'Saturday' THEN 1  "
		"	WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) <> 'Saturday' THEN -1  "
		"	ELSE 0 "
		"	END AS MergeDays "
		"  "
		"	FROM EMRMasterT LEFT JOIN  "
		"	(SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT "
		"	ON EMRMasterT.PatientID = NexwebLoginT.PersonID "
		// (r.farnworth 2014-05-06 11:35) - PLID 61910 - Fix Meaningful Use Detailed and Summary Reporting measure for MU.MENU.05 for Stage 1
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the first date a security code was ever created for the patient rather than the current security code creation date.
		// This ensures we are using the date when the patient first had access to the portal since SecurityCode and SecurityCodeCreationDate are nulled out when the patient creates a login from the security code.
		"	LEFT JOIN "
		"	( SELECT InitialSecurityCodeCreationDate, PersonID FROM PatientsT WHERE CurrentStatus <> 4 "
		"		AND PersonID > 0 AND InitialSecurityCodeCreationDate IS NOT NULL "
		"	) SecurityCodeT ON EMRMasterT.PatientID = SecurityCodeT.PersonID  "
		// (r.farnworth 2014-05-19 11:13) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"	INNER JOIN "
		"	( SELECT Min(Date) as MergeDate, PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField IN (1,2) "
		"		GROUP BY PersonID ) MergeDocsT ON EMRMasterT.PatientID = MergeDocsT.PersonID "
		"	WHERE EMRMasterT.Deleted = 0 "
		"		AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	) Q WHERE BusinessDays <= 4 AND MergeDays <= 4 "
		"	GROUP BY PatientID) " 
		);
		
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (j.gruber 2011-10-27 16:28) - PLID 46160 - use the default
	riReport.SetDefaultDenominatorSql();				

	// (j.gruber 2011-11-10 14:01) - PLID 46505
	CString strSelect, strDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strNum = "FROM ( "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, \r\n"
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, 'First Nexweb Login Date' as MiscDesc, CONVERT(nVarChar, dbo.AsDateNoTime(MinDate), 101) as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"

		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the InitialSecurityCodeCreationDate since SecurityCodeCreationDate is cleared out when the patient creates a username with the security code.
		" LEFT JOIN ( SELECT EMRMasterT.*, \r\n"
		" DATEDIFF(d,Date, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) \r\n"
		" - DATEDIFF(wk,Date, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) * 2  \r\n"
		" - CASE  \r\n"
		" WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) = 'Saturday' THEN 1  \r\n"
		" WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) <> 'Saturday' THEN -1  \r\n"
		" ELSE 0 \r\n"
		" END AS BusinessDays, CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END AS MinDate, \r\n"
		"  \r\n"
		// (r.farnworth 2014-05-19 11:13) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		" DATEDIFF(d,Date, MergeDocsT.MergeDate) \r\n"
		" - DATEDIFF(wk,Date,  MergeDocsT.MergeDate) * 2  \r\n"
		" - CASE  \r\n"
		" WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) = 'Saturday' THEN 1  \r\n"
		" WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) <> 'Saturday' THEN -1  \r\n"
		" ELSE 0 \r\n"
		" END AS MergeDays "
		"  \r\n"
		" FROM EMRMasterT LEFT JOIN  \r\n"
		" (SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT \r\n"
		" ON EMRMasterT.PatientID = NexwebLoginT.PersonID \r\n"
		// (r.farnworth 2014-05-06 11:35) - PLID 61910 - Fix Meaningful Use Detailed and Summary Reporting measure for MU.MENU.05 for Stage 1
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the first date a security code was ever created for the patient rather than the current security code creation date.
		// This ensures we are using the date when the patient first had access to the portal since SecurityCode and SecurityCodeCreationDate are nulled out when the patient creates a login from the security code.
		" LEFT JOIN "
		" (SELECT InitialSecurityCodeCreationDate, PersonID FROM PatientsT WHERE CurrentStatus <> 4 "
		" AND PersonID > 0 AND InitialSecurityCodeCreationDate IS NOT NULL) SecurityCodeT  "
		" ON EMRMasterT.PatientID = SecurityCodeT.PersonID  "
		// (r.farnworth 2014-05-19 11:13) - PLID 62188 - Patients Electronic Access Stage 1 Core 11 need to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		" INNER JOIN "
		" ( SELECT Min(Date) as MergeDate, PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField IN (1,2) "
		"	 GROUP BY PersonID ) MergeDocsT ON EMRMasterT.PatientID = MergeDocsT.PersonID "
		" WHERE EMRMasterT.Deleted = 0 \r\n"
		"	AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"
		" ) EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] AND BusinessDays <= 4 AND MergeDays <= 4"
		" ) N \r\n";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'First NexWeb Login Date' as MiscDesc, dbo.AsDateNoTime(MinDate) as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" LEFT JOIN  "
		" (SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT "
		" ON EMRMasterT.PatientID = NexwebLoginT.PersonID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";


	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}

// (j.gruber 2010-09-14 08:58) - PLID 40479 
//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 Update query to call Common code
void CCHITReportInfoListing::CreateTimelyClinicalSummaryReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	// (j.gruber 2011-05-16 14:47) - PLID 43709 - needs to be count of clinical summaries, not patients
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUTimelyClinicalSummary;
	//(e.lally 2012-03-21) PLID 48707
	riReport.SetPercentToPass(50);
	riReport.m_strInternalName = "MU.13 - Clinical summaries provided within 3 business days";
	// (b.savon 2014-05-14 07:51) - PLID 62127 - Rename MUS1 Core 13 to Core 12.  Also update the numerator text
	riReport.m_strDisplayName = "MU.CORE.12 - Clinical summaries provided within 3 business days";
	riReport.SetConfigureType(crctEMRItem); // (r.farnworth 2013-11-26 11:33) - PLID 59834 - This report now looks for an EMR item rather than a specifc element
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:11) - PLID 43765 - set location filter and descritions
	// (j.gruber 2011-07-29 15:11) - PLID 44755 - filter for office visit codes.
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (d.thompson 2012-10-18) - PLID 48423 - Since we've changed the default set of codes, I'm going to change the name of the
	//	property so we don't ever use the old cache values. (which also should have been done by PLID 52346 last scope)
	//TES 7/24/2013 - PLID 57603 - Updated to check for EMNs up to 3 business days after the qualifying service code
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	// (b.savon 2014-05-14 07:53) - PLID 62127 - Rename MUS1 Core 13 to Core 12.  Also update the numerator text
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);

	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code

	CString strCodeDesc;
	strCodeDesc.Format("either with the following service codes or on the same day as an EMN or charge with a Service Code in %s", strCodes); 
	riReport.SetHelpText("This report calculates how many EMNs state that clinical summaries were provided to the patient within 3 business days. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of non-excluded EMNs in the denominator that state a clinical summary was provided within 3 business days, defined by the selected EMR item in the configuration for this report.  The EMR item can be substituted for a clinical summary file generated through the EMN.",
		"The number of non-excluded EMNs " + strCodeDesc + " starting at the start date and ending four business days prior to the end of the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2013-11-26 11:33) - PLID 59834 - Changes to Clinical Summary Stage 1 report needed to pass testing
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (r.farnworth 2014-04-29 10:35) - PLID 61821 - Clinical Summaries should automatically count in the numerator when a user merges the clinical summary.
	// (b.savon 2014-05-14 07:53) - PLID 62127 - Rename MUS1 Core 13 to Core 12.  Also update the numerator text
	// (s.dhole 2014-06-3 07:53) - PLID 62274 - exclude void charges
	//(s.dhole 8/1/2014 2:44 PM ) - PLID 63087 there are two main issue
	//1)  when it scan charges  and emn, if both have service code than it return duplicate record and table join will return same duplicate record which cause issue to increase count
	//2) join was on patient id not on emr id, which any include non-filter EMNs IN a list
	// (d.singleton 2014-09-17 14:50) - PLID 63450 - MU Core 12 (clinical summaries) - Bills not filtering on Provider - summary
	//(s.dhole 9/26/2014 8:55 AM ) - PLID 63765 update query with common  function
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We now generate qualifying numerator and denominator values in temp tables
	// and aggregate them to produce our results.
	CString strSql;
	strSql.Format(R"(
/*MU.13/MU.CORE.12 Numerator*/
SELECT COUNT(*) AS NumeratorValue FROM %s
)"
	, riReport.GetClinicalSummaryNumeratorTempTableName());
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//TES 7/24/2013 - PLID 57603 - Updated to check for EMNs up to 3 business days after the qualifying service code
	// (r.farnworth 2013-11-26 11:33) - PLID 59834 - Changes to Clinical Summary Stage 1 report needed to pass testing
	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (b.savon 2014-05-14 07:53) - PLID 62127 - Rename MUS1 Core 13 to Core 12.  Also update the numerator text
	// (s.dhole 2014-06-3 07:53) - PLID 62274 - exclude void charges
	//(s.dhole 8/1/2014 2:46 PM ) - PLID 63087 Cahnge sql to return only those Emn Which has same date as bill using Codes(99XXX) 
	// (d.singleton 2014-09-17 14:50) - PLID 63450 - MU Core 12 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We now generate qualifying numerator and denominator values in temp tables
	// and aggregate them to produce our results.
	strSql.Format(R"(
/*MU.13/MU.CORE.12 Denominator*/
SELECT COUNT(*) AS DenominatorValue FROM %s
)"
	, riReport.GetClinicalSummaryDenominatorTempTableName());
	riReport.SetDenominatorSql(strSql);

	// (j.gruber 2011-11-10 14:01) - PLID 46505
	CString strSelect, strReportDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//TES 7/24/2013 - PLID 57603 - Updated to check for EMNs up to 3 business days after the qualifying service code
	// (r.farnworth 2013-11-26 11:33) - PLID 59834 - Changes to Clinical Summary Stage 1 report needed to pass testing
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (r.farnworth 2014-04-29 10:35) - PLID 61821 - Clinical Summaries should automatically count in the numerator when a user merges the clinical summary.
	//(s.dhole 8/1/2014 2:46 PM ) - PLID 63087 Cahnge sql to return only those Emn Which has same date as bill using Codes(99XXX)
	// (d.singleton 2014-09-17 14:50) - PLID 63450 - MU Core 12 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We now generate qualifying numerator and denominator values in temp tables
	// and aggregate them to produce our results.
	strNum.Format(R"(
FROM 
(
	SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate, 
		 PersonT.HomePhone, PersonT.WorkPhone, 
		 'Visit Date' as ItemDescriptionLine, Numerator.Date as ItemDate, '' as ItemDescription, 
		 '' AS ItemProvider, '' as ItemSecProvider, 
		 '' as ItemLocation, 'Visit Date' as MiscDesc, Numerator.Date as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 
	FROM %s Numerator
	INNER JOIN PersonT ON PersonT.ID = Numerator.PatientID
	INNER JOIN PatientsT ON PatientsT.PersonID = Numerator.PatientID
) N 
)"
	, riReport.GetClinicalSummaryNumeratorTempTableName());

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//TES 7/24/2013 - PLID 57603 - Updated to check for EMNs up to 3 business days after the qualifying service code
	// (r.farnworth 2013-11-26 11:33) - PLID 59834 - Changes to Clinical Summary Stage 1 report needed to pass testing
	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	//(s.dhole 8/1/2014 2:46 PM ) - PLID 63087 Cahnge sql to return only those Emn Which has same date as bill using Codes(99XXX) 
	// (d.singleton 2014-09-17 14:50) - PLID 63450 - MU Core 12 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We're completely pivoting on @clinicalSummaryDenominator which
	// contains all the qualifying visits
	strReportDenom.Format(R"(
FROM 
(
	SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  
		 PersonT.HomePhone, PersonT.WorkPhone, 
		 'Visit Date' as ItemDescriptionLine, Denominator.Date as ItemDate, '' as ItemDescription, 
		 '' AS ItemProvider, '' as ItemSecProvider, 
		 '' as ItemLocation, 'Visit Date' as MiscDesc, Denominator.Date as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 
	FROM %s Denominator
	INNER JOIN PersonT ON PersonT.ID = Denominator.PatientID
	INNER JOIN PatientsT ON PatientsT.PersonID = Denominator.PatientID
) P 
)"
	, riReport.GetClinicalSummaryDenominatorTempTableName());

	riReport.SetReportInfo(strSelect, strNum, strReportDenom);
	
	m_aryReports.Add(riReport);
}
// (j.gruber 2010-09-14 08:58) - PLID 40479 
void CCHITReportInfoListing::CreateElectronicCopyRequestReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUElectronicCopyRequest;
	// (j.gruber 2011-11-04 13:12) - PLID 45692
	riReport.m_strInternalName = "MU.12 - Patients who request electronic copies of health information were provided it within 3 business days";
	// (b.savon 2014-05-14 08:00) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”.  Move the measure to the very bottom of the list.
	riReport.m_strDisplayName = "[OBSOLETE] - Patients who request electronic copies of health information were provided it within 3 business days";
	riReport.SetConfigureType(crctEMRMultiItems);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:12) - PLID 43765 - set lcoation filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have EMNs that state that an electronic copy of health information was requested and there is a subsequent EMN stating that the information was provided. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of patients in the denominator who were provided electronic health information within 3 business days of when it was requested, defined by the selected bottom EMR item in the configuration for this report.",
		"The number of patients who have a non-excluded EMN with the selected provider(s) and location and with an EMN date starting at the start date and ending four business days prior to the end of the date range that have electronic health information requested, defined by the top EMR item in the configuration of this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
	// (b.savon 2014-05-14 08:00) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”.  Move the measure to the very bottom of the list.
	CString strSql;
	strSql.Format("/*MU.12/MU.CORE.12.[OBSOLETE]*/\r\n"
		" SET @4BusDays = (SELECT DateAdd(day, -1*(CASE WHEN DateName(dw, @DateTo) = 'Sunday' THEN 5 ELSE "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Saturday' THEN 4 ELSE  "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Friday' THEN 4 ELSE "
		"	  6 END END END), @DateTo));\r\n " 

		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		" 	WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" 	AND PersonT.ID IN ( "
		" SELECT PatientID FROM ( "
		" SELECT EMRRequestQ.PatientID,  "
		"  DATEDIFF(d,EMRRequestQ.Date,EMRProvidedQ.Date)  "
		"   - DATEDIFF(wk,EMRRequestQ.Date,EMRProvidedQ.Date) * 2   "
		"   - CASE   "
		"   WHEN DATENAME(dw, EMRRequestQ.Date) <> 'Saturday' AND DATENAME(dw, EMRProvidedQ.Date) = 'Saturday' THEN 1   "
		"   WHEN DATENAME(dw, EMRRequestQ.Date) = 'Saturday' AND DATENAME(dw, EMRProvidedQ.Date) <> 'Saturday' THEN -1   "
		"   ELSE 0  "
		"   END AS BusinessDays  "
		"  FROM  "
		" (SELECT PatientID, Date FROM EMRMasterT  "
		" 			WHERE EMRMasterT.Deleted = 0  "
		" 			AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @4BusDays [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" 			AND EMRMasterT.ID IN ( "
		" 				SELECT EMRDetailsT.EMRID FROM EMRDetailsT  "
		" 				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		" 				WHERE EMRDetailsT.Deleted = 0 		 "
		"			AND ( "
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"				) "
		"			OR  "
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"				) "
		"			) "		
		"		) "		
		" ) EMRRequestQ  "
		" LEFT JOIN  "
		" (SELECT PatientID, Date FROM EMRMasterT  "
		" 			WHERE EMRMasterT.Deleted = 0  "
		" 			AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" 			AND EMRMasterT.ID IN ( "
		" 				SELECT EMRDetailsT.EMRID FROM EMRDetailsT  "
		" 				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		" 				WHERE EMRDetailsT.Deleted = 0 		 "
		"			AND ( "
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			OR  "
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			) "		
		"		) "		
		" ) EMRProvidedQ "
		" ON EMRRequestQ.PatientID = EMRProvidedQ.PatientID) Q WHERE BusinessDays IS NOT NULL AND BusinessDays >=0 AND BusinessDays < 4 "
		" )", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable); 
			riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
	// (b.savon 2014-05-14 08:00) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”.  Move the measure to the very bottom of the list.
	strSql.Format("/*MU.12/MU.CORE.12.[OBSOLETE]*/\r\n"
		" SET @4BusDays = (SELECT DateAdd(day, -1*(CASE WHEN DateName(dw, @DateTo) = 'Sunday' THEN 5 ELSE "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Saturday' THEN 4 ELSE  "
		"	  CASE WHEN DateName(dw, @DateTo) = 'Friday' THEN 4 ELSE "
		"	  6 END END END), @DateTo));\r\n " 	
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"	
		"	 WHERE EMRMasterT.Deleted = 0 		"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @4BusDays [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "	
		" ) "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable);
	riReport.SetDenominatorSql(strSql);

	// (j.gruber 2011-11-10 14:01) - PLID 46503
	CString strSelect, strDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strNum.Format("FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" '' as ItemDescriptionLine, NULL as ItemDate, '' as ItemDescription, \r\n"
		" '' AS ItemProvider, '' as ItemSecProvider, \r\n"
		" '' as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" 	WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" 	AND PersonT.ID IN ( "
		" SELECT PatientID FROM ( "
		" SELECT EMRRequestQ.PatientID,  "
		"  DATEDIFF(d,EMRRequestQ.Date,EMRProvidedQ.Date)  "
		"   - DATEDIFF(wk,EMRRequestQ.Date,EMRProvidedQ.Date) * 2   "
		"   - CASE   "
		"   WHEN DATENAME(dw, EMRRequestQ.Date) <> 'Saturday' AND DATENAME(dw, EMRProvidedQ.Date) = 'Saturday' THEN 1   "
		"   WHEN DATENAME(dw, EMRRequestQ.Date) = 'Saturday' AND DATENAME(dw, EMRProvidedQ.Date) <> 'Saturday' THEN -1   "
		"   ELSE 0  "
		"   END AS BusinessDays  "
		"  FROM  "
		" (SELECT PatientID, Date FROM EMRMasterT  "
		" 			WHERE EMRMasterT.Deleted = 0  "
		" 			AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @4BusDays [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" 			AND EMRMasterT.ID IN ( "
		" 				SELECT EMRDetailsT.EMRID FROM EMRDetailsT  "
		" 				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		" 				WHERE EMRDetailsT.Deleted = 0 		 "
		"			AND ( "
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"				) "
		"			OR  "
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"				) "
		"			) "		
		"		) "		
		" ) EMRRequestQ  "
		" LEFT JOIN  "
		" (SELECT PatientID, Date FROM EMRMasterT  "
		" 			WHERE EMRMasterT.Deleted = 0  "
		" 			AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" 			AND EMRMasterT.ID IN ( "
		" 				SELECT EMRDetailsT.EMRID FROM EMRDetailsT  "
		" 				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		" 				WHERE EMRDetailsT.Deleted = 0 		 "
		"			AND ( "
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			OR  "
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			) "		
		"		) "		
		" ) EMRProvidedQ "
		" ON EMRRequestQ.PatientID = EMRProvidedQ.PatientID) Q WHERE BusinessDays IS NOT NULL AND BusinessDays >=0 AND BusinessDays < 4)  "
		" ) N ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable); 

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, '' as ItemDescriptionLine, "
		" NULL as ItemDate, '' as ItemDescription, "
		" '' AS ItemProvider, "
		" '' as ItemSecProvider, "
		" '' as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"	
		"	 WHERE EMRMasterT.Deleted = 0 		"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @4BusDays [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "	
		" ) "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" ) P ", eitSingleList, eitMultiList, eitTable);


	riReport.SetReportInfo(strSelect, strNum, strDenom);
	
	m_aryReports.Add(riReport);
}

// (j.gruber 2010-09-14 09:03) - PLID 40475 - Problems
void CCHITReportInfoListing::CreateProblemListReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUProblemList;
	// (j.gruber 2011-11-04 13:12) - PLID 45692
	riReport.m_strInternalName = "MU.01 - Patients who have current or up to date diagnoses";
	riReport.m_strDisplayName = "MU.CORE.03 - Patients who have current or up to date diagnoses";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:14) - PLID 43764 - set loction filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - Exclusions
	// (r.gonet 04/16/2014) - PLID 61120 - Updated the text from More Info topic to Codes topic
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have at least one diagnosis code assigned on the General 2 tab, a bill, EMR problems, or on the <Codes> topic of an EMN. Filtering is done on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).",
		"The number of unique patients in the denominator who have a diagnosis code on General 2, a bill, an EMR Problem, or on the Codes topic of an EMN.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 03/03/2013) - PLID 61120 - The measure now looks at ICD-10 codes as well.
	riReport.SetNumeratorSql("/*MU.01/MU.CORE.03*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0  "
		" 		 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] )  "
		" 		 AND (PersonID IN (SELECT PersonID FROM PatientsT WHERE DefaultDiagID1 IS NOT NULL OR DefaultDiagID2 IS NOT NULL OR DefaultDiagID3 IS NOT NULL OR DefaultDiagID4 IS NOT NULL OR DefaultICD10DiagID1 IS NOT NULL OR DefaultICD10DiagID2 IS NOT NULL OR DefaultICD10DiagID3 IS NOT NULL OR DefaultICD10DiagID4 IS NOT NULL)  "
		" 			 OR PersonID IN (SELECT PatientID FROM BillsT WHERE DELETED = 0 AND BillsT.ID IN (SELECT BillDiagCodeT.BillID FROM BillDiagCodeT WHERE BillDiagCodeT.ICD9DiagID IS NOT NULL OR BillDiagCodeT.ICD10DiagID IS NOT NULL))  "
		" 			 OR PersonID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND ID IN (SELECT EMRID FROM EMRDiagCodesT WHERE DELETED = 0 AND (DiagCodeID IS NOT NULL OR DiagCodeID_ICD10 IS NOT NULL)))  "
		" 			 OR PersonID IN (SELECT PatientID FROM EMRProblemsT WHERE DELETED = 0 AND (DiagCodeID IS NOT NULL OR DiagCodeID_ICD10 IS NOT NULL))  "
		" 		 )  "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		);
		
	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (j.gruber 2011-10-27 16:28) - PLID 46160 - use the default
	riReport.SetDefaultDenominatorSql();				

	// (j.gruber 2011-11-16 14:10) - PLID 46502
	CString strNum;
	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.gonet 03/03/2013) - PLID 61120 - The measure now looks at ICD-10 codes as well.
	strNum = " FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND (PersonID IN (SELECT PersonID FROM PatientsT WHERE DefaultDiagID1 IS NOT NULL OR DefaultDiagID2 IS NOT NULL OR DefaultDiagID3 IS NOT NULL OR DefaultDiagID4 IS NOT NULL OR DefaultICD10DiagID1 IS NOT NULL OR DefaultICD10DiagID2 IS NOT NULL OR DefaultICD10DiagID3 IS NOT NULL OR DefaultICD10DiagID4 IS NOT NULL)  "
		"	 OR PersonID IN (SELECT PatientID FROM BillsT WHERE DELETED = 0 AND BillsT.ID IN (SELECT BillDiagCodeT.BillID FROM BillDiagCodeT WHERE BillDiagCodeT.ICD9DiagID IS NOT NULL OR BillDiagCodeT.ICD10DiagID IS NOT NULL))  "
		" 	 OR PersonID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND ID IN (SELECT EMRID FROM EMRDiagCodesT WHERE DELETED = 0 AND (DiagCodeID IS NOT NULL OR DiagCodeID_ICD10 IS NOT NULL)))  "
		" 	 OR PersonID IN (SELECT PatientID FROM EMRProblemsT WHERE DELETED = 0 AND (DiagCodeID IS NOT NULL OR DiagCodeID_ICD10 IS NOT NULL))  "
		" )  "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ";

	//use the default numerator and denominator
	riReport.SetReportInfo("", strNum, "");
		
	m_aryReports.Add(riReport);

}

// (j.gruber 2010-09-20 11:57) - PLID 40789
void CCHITReportInfoListing::CreateCPOEReport_MedicationOrders()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUCPOEReport_MedicationOrders;
	riReport.SetConfigureType(crctMailsentCat);
	// (j.gruber 2011-11-04 13:13) - PLID 45692
	riReport.m_strInternalName = "MU.07 - Patients who have medication orders entered through CPOE";
	riReport.m_strDisplayName = "MU.CORE.01 - Patients who have medication orders entered through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43716 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:16) - PLID 43764 - set location filter and descriptions
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:56) - PLID 61815 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have medications ordered entered through CPOE. "
		"Filtering is applied to EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator with at least one prescription entered.",
		"The number of unique patients with a non-excluded EMN in the date range and with the selected provider(s) and location with at least one prescription entered or who have a written prescription scanned in, defined by the history category in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetNumeratorSql("/*MU.07/MU.CORE.01*/\r\n"
		"SELECT COUNT(PersonT.ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		" AND PersonT.ID IN (SELECT PatientID FROM PatientMedications "
		"	WHERE PatientMedications.Deleted = 0) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	riReport.SetDenominatorSql("/*MU.07/MU.CORE.01*/\r\n"
		"SELECT Count(PatientID) as DenominatorValue FROM ( "
		"SELECT (PersonT.ID) AS PatientID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "
		" AND PersonT.ID IN (SELECT PatientID FROM PatientMedications "
		"	WHERE PatientMedications.Deleted = 0) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION "
		"SELECT PersonT.ID as PatientID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "
		" AND PersonT.ID IN (SELECT PersonID FROM MailSent WHERE CategoryID = @MailSentCatID) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" )Q "
		);

	// (j.gruber 2011-11-08 13:58) - PLID 46502 - Report	
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First,Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2,  Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strNum = " FROM (SELECT PersonT.ID as PatientID, PersonT.BirthDate, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, dbo.GetEMNProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2,  "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]  "
		" AND PersonT.ID IN (SELECT PatientID FROM PatientMedications WHERE PatientMedications.Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";
	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom = " FROM ( "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, "
		" PersonT.Address2, PersonT.City, PersonT.State, PersonT.Birthdate, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEMNProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEMNSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND PersonT.ID IN (SELECT PatientID FROM PatientMedications WHERE PatientMedications.Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, PersonT.Last, PersonT.Middle, PersonT.Address1, "
		" PersonT.Address2, PersonT.City, PersonT.State, PersonT.Birthdate, PersonT.Zip, PersonT.HomePhone, PersonT.WorkPhone, "
		" 'History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "				
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE "
		" PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "		
		" AND MailSent.CategoryID = @MailSentCatID "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" )P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}
//(s.dhole 9/8/2014 10:54 AM ) - PLID 62794 return case stament with all valid reminder methode
CString CCHITReportInfoListing::GetSQLReminderMethods()
{
	return " (CASE WHEN PatientRemindersSentT.ReminderMethod = 1 THEN 'Home Phone' "
		" WHEN PatientRemindersSentT.ReminderMethod =2 THEN 'Work Phone' "
		" WHEN PatientRemindersSentT.ReminderMethod =3 THEN 'Mobile Phone' "
		" WHEN PatientRemindersSentT.ReminderMethod =4 THEN 'Pager' "
		" WHEN PatientRemindersSentT.ReminderMethod =5 THEN 'Other Phone' "
		" WHEN PatientRemindersSentT.ReminderMethod =6 THEN 'Email' "
		" WHEN PatientRemindersSentT.ReminderMethod =7 THEN 'Text Messaging' "
		" WHEN PatientRemindersSentT.ReminderMethod =8 THEN 'TeleVox' "
		" WHEN PatientRemindersSentT.ReminderMethod =9 THEN 'Letterwriting' "
		" WHEN PatientRemindersSentT.ReminderMethod =10 THEN 'Recalls' "
		" WHEN PatientRemindersSentT.ReminderMethod =11 THEN 'In-Person' "
		" END ) ";

}
 
// (j.gruber 2010-10-01 14:16) - PLID 40479
void CCHITReportInfoListing::CreateRemindersReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUReminders;
	//(e.lally 2012-03-21) PLID 48707
	riReport.SetPercentToPass(20);
	// (j.gruber 2011-11-04 13:13) - PLID 45692
	riReport.m_strInternalName = "MU.14 - Patients who were sent appropriate reminders";
	// (s.dhole 2014-05-9 15:33) - PLID 62184 - Change display name
	riReport.m_strDisplayName = "MU.MENU.06 - Patients who were sent appropriate reminders";
	riReport.SetConfigureType(crctEMRItem);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:20) - PLID 43765 - set location filter and descriptions
	// (j.gruber 2011-07-29 15:06) - PLID 44753 - changed to use age during reporting time rather than age at service
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (j.gruber 2012-10-25 10:52) - PLID 53134 - apply dates in the numerator, still not in the denominator
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	//(s.dhole 9/8/2014 3:29 PM ) - PLID 63580 Change the numerator description to support sent reminder data
	riReport.SetHelpText("This report calculates how many patients were sent an appropriate reminder. There is no date filtering applied to the denominator of this report. Date filtering on the numerator is on EMN Date or the General 1 reminder date. Provider and location filtering is on EMN Primary or Secondary Provider(s) and EMN Location.", 
		"The number of unique patients in the denominator who have appropriate reminders sent to them within the date range, defined by the selected EMR item in the configuration for this report or the reminders on the patients’ General 1 tabs.",
		"The number of unique patients that have a non-excluded EMN with the selected provider(s) and location who were 65 years old or older or 5 years old or younger during the reporting period.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	CString strSql;
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
	// (j.gruber 2012-10-25 11:49) - PLID 53134 - add date filter to numerator only
	//(s.dhole 9/4/2014 4:32 PM ) - PLID 62794 Added additional union to display remindersent data
	strSql.Format("/*MU.14/MU.MENU.04*/\r\n"
		//we have to subtract one from DateTo since it is auto incremented
		" SET @DateToUse = DateAdd(dd,-1,@DateTo);  \r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonID IN (SELECT PatientID FROM EMRMasterT  "
		"	WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo  [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		)"
		"	)"	
		" UNION "
		// (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
		" SELECT PatientID FROM PatientRemindersSentT WHERE (Deleted = 0 AND ReminderDate >= @DateFrom  AND ReminderDate < @DateTo)  "
		" AND PatientRemindersSentT.PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		" )"
		
		" AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		" AND ((CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateFrom) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateFrom) OR (MONTH(PersonT.Birthdate) = MONTH(@DateFrom)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateFrom))  "
		" THEN 1 ELSE 0 END END) <= 5 OR " 
		" (CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateToUse) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateToUse) OR (MONTH(PersonT.Birthdate) = MONTH(@DateToUse)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateToUse))  "
		" THEN 1 ELSE 0 END END) >= 65) "		
		, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//(e.lally 2012-04-24) PLID 48266 - Moved declarations into separate function
	//(s.dhole 9/4/2014 4:32 PM ) - PLID 62794 Added additional union to display remindersent data
	riReport.SetDenominatorSql("/*MU.14/MU.MENU.04*/\r\n"
		//we have to subtract one from DateTo since it is auto incremented
		" SET @DateToUse = DateAdd(dd,-1,@DateTo);  \r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter]  "
		") "
		"AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		" AND ((CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateFrom) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateFrom) OR (MONTH(PersonT.Birthdate) = MONTH(@DateFrom)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateFrom))  "
		" THEN 1 ELSE 0 END END) <= 5 OR " 
		" (CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateToUse) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateToUse) OR (MONTH(PersonT.Birthdate) = MONTH(@DateToUse)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateToUse))  "
		" THEN 1 ELSE 0 END END) >= 65) ");


	// (j.gruber 2011-11-10 14:01) - PLID 46505
	CString strSelect, strDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (j.gruber 2012-10-25 11:49) - PLID 53134 - add date filter to numerator only
	//(s.dhole 9/4/2014 4:32 PM ) - PLID 62794 Added additional union to display remindersent data
	strNum.Format( "FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'EMNs/Reminder'  AS ItemDescriptionLine, EMRMasterT.Date   AS ItemDate,"
		"  EMRMasterT.Description AS ItemDescription, \r\n"
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID \r\n "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE (EMRMasterT.DELETED = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"		
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		)"
		"	)"		
		")"		
		" AND ((CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateFrom) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateFrom) OR (MONTH(PersonT.Birthdate) = MONTH(@DateFrom)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateFrom))  "
		" THEN 1 ELSE 0 END END) <= 5 OR " 
		" (CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateToUse) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateToUse) OR (MONTH(PersonT.Birthdate) = MONTH(@DateToUse)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateToUse))  "
		" THEN 1 ELSE 0 END END) >= 65) "	
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'EMNs/Reminder'  AS ItemDescriptionLine, PatientRemindersSentT.ReminderDate   AS ItemDate,"
		"  %s AS ItemDescription, \r\n"
		" NULL AS ItemProvider, NULL as ItemSecProvider, \r\n"
		" '' as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"
		" INNER JOIN PatientRemindersSentT ON PatientsT.PersonID = PatientRemindersSentT.PatientID \r\n "
		// (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
		" WHERE PatientRemindersSentT.Deleted = 0 AND PatientRemindersSentT.ReminderDate >=  @DateFrom AND PatientRemindersSentT.ReminderDate < @DateTo "
		" AND PatientRemindersSentT.PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		" AND ((CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateFrom) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateFrom) OR (MONTH(PersonT.Birthdate) = MONTH(@DateFrom)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateFrom))  "
		" THEN 1 ELSE 0 END END) <= 5 OR "
		" (CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateToUse) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateToUse) OR (MONTH(PersonT.Birthdate) = MONTH(@DateToUse)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateToUse))  "
		" THEN 1 ELSE 0 END END) >= 65) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		") N \r\n"
		, eitSingleList, eitMultiList, eitTable, GetSQLReminderMethods());


	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	//(s.dhole 9/4/2014 4:32 PM ) - PLID 62794 Rollback  union to display remindersent data
	strDenom =  " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' AS ItemDescriptionLine, "
		" EMRMasterT.Date  AS ItemDate, "
		" EMRMasterT.Description  AS  ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "				
		" WHERE ((EMRMasterT.DELETED = 0   [ProvFilter] [LocFilter] [ExclusionsFilter] )  "		
		") "
		" AND ((CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateFrom) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateFrom) OR (MONTH(PersonT.Birthdate) = MONTH(@DateFrom)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateFrom))  "
		" THEN 1 ELSE 0 END END) <= 5 OR " 
		" (CASE WHEN PersonT.BirthDate Is Null then NULL ELSE DATEDIFF(YYYY, PersonT.Birthdate, @DateToUse) - "
		" CASE WHEN MONTH(PersonT.Birthdate) > MONTH(@DateToUse) OR (MONTH(PersonT.Birthdate) = MONTH(@DateToUse)  "
		" AND DAY(PersonT.Birthdate) > DAY(@DateToUse))  "
		" THEN 1 ELSE 0 END END) >= 65) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" ) P ";


	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);

}

// (j.gruber 2010-10-04 12:38) - PLID 40789
void CCHITReportInfoListing::CreateDischargeSummaryReport() 
{

	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUDischargeSummary;
	//(e.lally 2012-03-21) PLID 48707
	riReport.SetPercentToPass(50);
	// (j.gruber 2011-11-04 13:15) - PLID 45692
	riReport.m_strInternalName = "MU.16 - Transitions of Care are provided summary of care.";
	// (b.savon 2014-05-14 07:36) - PLID 62132 - Rename MUS1 Menu 8 to MUS1 Menu 7 and update the numerator text
	// (s.dhole 2014-05-9 15:33) - PLID 62184 - Change display name
	riReport.m_strDisplayName = "MU.MENU.09 - Transitions of Care are provided summary of care.";
	riReport.SetConfigureType(crctEMRMultiItems);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:23) - PLID 43765 - set location filter and descriptions
	// (j.kuziel 2011-05-19 16:01) - PLID 43787 - fixing typos
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	// (b.savon 2014-05-14 07:46) - PLID 62132 - Rename MUS1 Menu 8 to MUS1 Menu 7 and update the numerator text
	riReport.SetHelpText("This report calculates how many transitions of care state that a summary of care was provided. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of non-excluded EMNs in the denominator who have been provided a summary of care, defined by the selected bottom EMR item in the configuration for this report.  The bottom item can be substituted for a transition of care file generated through the EMN or history tab.",
		"The number of non-excluded EMNs within the date range and with the selected provider(s) and location that state the patient is transitioning, defined by the selected top EMR item in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (b.savon 2014-05-14 07:46) - PLID 62132 - Rename MUS1 Menu 8 to MUS1 Menu 7 and update the numerator text
	// (r.farnworth 2014-05-14 08:03) - PLID 62133 - Transitions of Care Stage 1 Menu 7 should automatically count in the numerator when a user merges a summary of care for a transitioning patient.
	CString strSql;
	strSql.Format("/*MU.16/MU.MENU.07*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"   AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND  ( "
		//(s.dhole 8/1/2014 4:11 PM ) - PLID  63069 change join on Emr ID from PatientID
		"		EMRMasterT.ID IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "
		"	) "
		"	OR "
		"		EMRMasterT.PatientID IN ( SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField = 1) "
		"	) "
		" ) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (b.savon 2014-05-14 07:46) - PLID 62132 - Rename MUS1 Menu 8 to MUS1 Menu 7 and update the numerator text
	strSql.Format("/*MU.16/MU.MENU.07*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "		
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ",  eitSingleList, eitMultiList, eitTable);
	riReport.SetDenominatorSql(strSql);

	// (j.gruber 2011-11-16 14:28) - PLID 46506
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (r.farnworth 2014-05-14 08:03) - PLID 62133 - Transitions of Care Stage 1 Menu 7 should automatically count in the numerator when a user merges a summary of care for a transitioning patient.
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Transition' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE 	"
		" EMRMasterT.DELETED = 0 "
		"	AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND ( "
		//(s.dhole 8/1/2014 4:11 PM ) - PLID 63069 change join on Emr ID from PatientID
		"		EMRMasterT.ID  IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "
		"	) "
		"	OR "
		"		EMRMasterT.PatientID IN ( SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField = 1) "
		"	) "
		") "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N  ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
		
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom.Format( " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'Transition' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ",  eitSingleList, eitMultiList, eitTable);

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);

}

// (j.gruber 2010-10-04 12:38) - PLID 40789
void CCHITReportInfoListing::CreateReferralMedReconReport()
{
	CCCHITReportInfo riReport;
	// (j.gruber 2011-05-13 12:26) - PLID 43694
	riReport.SetReportType(crtMU);
	//(e.lally 2012-04-03) PLID 48264
	riReport.m_nInternalID = (long)eMUReferralMedRecon;
	//(e.lally 2012-03-21) PLID 48707
	riReport.SetPercentToPass(50);
	// (j.gruber 2011-11-04 13:15) - PLID 45692
	riReport.m_strInternalName = "MU.15 - Referrals that have medication reconciliation performed.";
	// (b.savon 2014-05-14 07:26) - PLID 62131 - Rename MUS1 Menu 7 to MUS1 Menu 6 and update the numerator text
	// (s.dhole 2014-05-9 15:33) - PLID 62184 - Change display name
	riReport.m_strDisplayName = "MU.MENU.08 - Referrals that have medication reconciliation performed.";
	riReport.SetConfigureType(crctEMRMultiItems);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (j.gruber 2011-05-16 12:26) - PLID 43717 - set provider filter and descriptions
	// (j.gruber 2011-05-18 14:26) - PLID 43765 - set location filter and descriptions
	// (j.kuziel 2011-05-19 16:01) - PLID 43787 - fixing typos
	// (j.gruber 2011-11-07 12:45) - PLID 45365 - exclusions
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	// (b.savon 2014-05-14 07:26) - PLID 62131 - Rename MUS1 Menu 7 to MUS1 Menu 6 and update the numerator text
	riReport.SetHelpText("This report calculates how many referrals of care state that medication reconciliation was performed. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of non-excluded EMNs in the denominator who have a medication reconciliation performed, defined by the selected bottom EMR item in the configuration for this report.  A medication reconciliation can be substituted for the bottom EMR item.",
		"The number of non-excluded EMNs within the date range and with the selected provider(s) and location that state the patient is a referral, defined by the selected top EMR item in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (b.savon 2014-05-14 07:26) - PLID 62131 - Rename MUS1 Menu 7 to MUS1 Menu 6 and update the numerator text
	// (s.dhole 2014-05-15 07:26) - PLID 62135 - Added Reconciliation data to Numerator
	CString strSql;
	strSql.Format("/*MU.15/MU.MENU.06*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		//"    AND Date >= @DateFrom AND Date < @DateTo "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND Date >= @DateFrom AND Date < @DateTo "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		//(s.dhole 8/1/2014 4:21 PM ) - PLID 63040 Change  emrid to filter records rather than patientid
		"	 AND (EMRMasterT.ID IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "
		"	) "
		"	  OR (EMRMasterT.PatientID  IN(SELECT PatientID FROM PatientReconciliationT WHERE ReconciliationDate >= @DateFrom AND ReconciliationDate<@DateTo)) "
		"   ) "
		" ) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	// (b.savon 2014-05-14 07:26) - PLID 62131 - Rename MUS1 Menu 7 to MUS1 Menu 6 and update the numerator text
	strSql.Format("/*MU.15/MU.MENU.06*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "		
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ",  eitSingleList, eitMultiList, eitTable);
	riReport.SetDenominatorSql(strSql);

	// (j.gruber 2011-11-16 14:27) - PLID 46506
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Referral' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE 	"
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		//(s.dhole 8/1/2014 4:21 PM ) - PLID 63040 Change  emrid to filter records rather than patientid
		"	 AND (EMRMasterT.ID IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "
		"	  OR(EMRMasterT.PatientID  IN(SELECT PatientID FROM PatientReconciliationT WHERE ReconciliationDate >= @DateFrom AND ReconciliationDate<@DateTo)) "
		"   ) "
		"	) "
		") "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N  ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
		

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom.Format( " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'Referral' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ",  eitSingleList, eitMultiList, eitTable);

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);

}

// (j.dinatale 2013-03-11 11:59) - PLID 54947 - need a report for patients with height and weight
void CCHITReportInfoListing::CreateHeightWeightReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUHeightWeight;
	riReport.m_strInternalName = "Patients with Height and Weight recorded";
	riReport.m_strDisplayName = "Patients with Height and Weight recorded";

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 11:33) - PLID 61817 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 1 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Height and Weight recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).",
		"The number of unique patients in the denominator who have height and weight recorded, defined by the selected EMR items in the configuration for those reports.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (r.gonet 06/12/2013) - PLID 55151 - Get the patient age temp table table name.
	//CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	// (j.jones 2014-11-06 10:40) - PLID 63983 - cached ConfigRT values to streamline query plans
	riReport.SetInitializationSQL("SET NOCOUNT ON \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded int; "
		"SET @cchitPatientsWithHeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Height recorded') \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded2 int; "
		"SET @cchitPatientsWithHeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Height recorded2') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded int; "
		"SET @cchitPatientsWithWeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Weight recorded') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded2 int; "
		"SET @cchitPatientsWithWeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_Patients with Weight recorded2') \r\n"

		"SET NOCOUNT OFF \r\n");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (r.gonet 06/14/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	
	CString strSql;
	strSql.Format("/*MU.PHW*/\r\n"

		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithHeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithHeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithHeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"   [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithWeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithWeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithWeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")",
		eitSingleList, eitMultiList, eitTable, eitSlider,
		eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	riReport.SetDenominatorSql(FormatString("/*MU.PHW*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "));

	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";
	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	//TES 11/26/2013 - PLID 59838 - Applied a couple fixes found during Stage 2 testing
	//TES 12/27/2013 - PLID 60104 - Fixed the numerator filtering to prevent >100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" AND PatientsT.PersonID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithHeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithHeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithHeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		" WHERE EMRMasterT.Deleted = 0 "
		" [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithWeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithWeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithWeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		" ) "
		") N ",
		eitSingleList, eitMultiList, eitTable, eitSlider,
		eitSingleList, eitMultiList, eitTable, eitSlider);

	// (r.gonet 06/12/2013) - PLID 55151 - Replaced filtering on EMRMasterT.PatientAge with filtering on the precomputed patient age table.
	strDenom = FormatString(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) eP ");

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}

//TES 10/15/2013 - PLID 58993 - MU.CORE.03
void CCHITReportInfoListing::CreateStage2DemographicsReport()
{

	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2Demographics;
	riReport.SetPercentToPass(80);
	//TES 10/15/2013 - PLID 58993 - MU.CORE.03 - We use the MU2 internally to differentiate, the internal name should never display to the user
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.C03 - Patients who have all Demographic fields filled out.";
	riReport.m_strDisplayName = "MU.CORE.03 - Patients who have all Demographic fields filled out.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	//TES 10/15/2013 - PLID 58993 - MU.CORE.03 - Basically the same as stage 1 MU.CORE.07.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have the Language, Gender, Race, Ethnicity, and Birthdate fields filled out. Filtering is applied on EMN date, EMN location, and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients in the denominator who have the Birthdate, Gender, Race, Ethnicity, Language fields filled out.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 10/15/2013 - PLID 58993 - MU.CORE.03 - Basically the same as stage 1 MU.CORE.07.
	riReport.SetNumeratorSql("/*MU.04/MU.CORE.03*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"LEFT JOIN ( "		
		"	SELECT DISTINCT PersonID, Count(RaceID) AS PatientHasRace "
		"	FROM PersonRaceT "
		"	GROUP BY PersonID ) RaceSubQ ON RaceSubQ.PersonID = PatientsT.PersonID "
		"WHERE PersonT.LanguageID IS NOT NULL AND PersonT.Gender IN (1,2) AND RaceSubQ.PatientHasRace IS NOT NULL AND PersonT.Ethnicity IS NOT NULL AND PersonT.Birthdate IS NOT NULL "
		"	AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "		
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDefaultDenominatorSql();		

	CString strNum, strSelect, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//TES 10/15/2013 - PLID 58993 - MU.CORE.03 - Basically the same as stage 1 MU.CORE.07.
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Has Language' as MiscDesc, CASE WHEN PersonT.LanguageID IS NOT NULL then 'Yes' else 'No' END as ItemMisc, "
		" 'Has Gender' as Misc2Desc, CASE WHEN Gender IN (1,2) THEN 'Yes' ELSE 'No' END as ItemMisc2, "
		" 'Has Race' as Misc3Desc, CASE WHEN RaceSubQ.HasRaceID IS NOT NULL THEN 'Yes' ELSE 'No' END as ItemMisc3, "
		" 'Has Ethnicity' as Misc4Desc, CASE WHEN PersonT.Ethnicity IS NOT NULL THEN 'Yes' ELSE 'No' END  as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		// (b.spivey, May 28, 2013) - PLID 56869 - change this to check the tag table for races. 
		" LEFT JOIN ( "
		"		SELECT DISTINCT PersonID, Count(RaceID) AS HasRaceID "
		"		FROM PersonRaceT "
		"		GROUP BY PersonID ) RaceSubQ ON RaceSubQ.PersonID = PatientsT.PersonID "
		" WHERE PersonT.LanguageID IS NOT NULL AND PersonT.Gender IN (1,2) AND RaceSubQ.HasRaceID IS NOT NULL AND PersonT.Ethnicity IS NOT NULL AND PersonT.Birthdate IS NOT NULL "
		" AND EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	//TES 10/15/2013 - PLID 58993 - MU.CORE.03 - Basically the same as stage 1 MU.CORE.07.
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Has Language' as MiscDesc, CASE WHEN PersonT.LanguageID IS NOT NULL then 'Yes' else 'No' END as ItemMisc, "
		" 'Has Gender' as Misc2Desc, CASE WHEN Gender IN (1,2) THEN 'Yes' ELSE 'No' END as ItemMisc2, "
		" 'Has Race' as Misc3Desc, CASE WHEN RaceSubQ.HasRaceID IS NOT NULL THEN 'Yes' ELSE 'No' END as ItemMisc3, "
		" 'Has Ethnicity' as Misc4Desc, CASE WHEN PersonT.Ethnicity IS NOT NULL THEN 'Yes' ELSE 'No' END  as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "		
		// (b.spivey, May 28, 2013) - PLID 56869 - Changed this to check the tag table for races. 
		" LEFT JOIN ( "
		"		SELECT DISTINCT PersonID, Count(RaceID) AS HasRaceID "
		"		FROM PersonRaceT "
		"		GROUP BY PersonID ) RaceSubQ ON RaceSubQ.PersonID = PatientsT.PersonID "
		" WHERE EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}

// (r.farnworth 2013-10-15 15:49) - PLID 59014 - MU.CORE.08
void CCHITReportInfoListing::CreateStage2ClinicalSummaryReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2ClinicalSummary;
	// (r.farnworth 2013-10-15 15:52) - PLID 59014 - We use the MU2 internally to differentiate, the internal name should never display to the user
	riReport.SetPercentToPass(50);
	// (r.farnworth 2013-10-15 15:49) - PLID 59014 - MU.CORE.13 is the stage 1 variant of this measur
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.e
	riReport.m_strInternalName = "MU2.C08 - Clinical summaries provided within 3 business days";
	riReport.m_strDisplayName = "MU.CORE.08 - Clinical summaries provided within 1 business day";
	riReport.SetConfigureType(crctEMRItem);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.


	// (r.farnworth 2013-10-15 16:55) - PLID 59014 - We will likely use the same codes since the measures are so similar
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);

	CString strCodeDesc;
	strCodeDesc.Format("either with the following service codes or on the same day as an EMN or charge with a Service Code in %s", strCodes); 
	// (r.farnworth 2013-10-15 16:02) - PLID 59014 - Basically the same as MU.CORE.13 but with 1 business day
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	// (r.farnworth 2014-05-14 07:44) - PLID 62134 - Change the numerator text in MUS2 Core 8, MUS2 Core 13, MUS2 Core 14
	riReport.SetHelpText("This report calculates how many EMNs state that clinical summaries were provided to the patient within 1 business day. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of non-excluded EMNs in the denominator that state a clinical summary was provided within 1 business day, defined by the selected EMR item in the configuration for this report. The EMR item can be substituted for a clinical summary file generated through the EMN.",
		"The number of non-excluded EMNs " + strCodeDesc + " within the date range and with the selected provider(s) and location.");

	// (r.farnworth 2013-10-15 16:02) - PLID 59014 - Query changes for 1 business day
	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (r.farnworth 2014-01-31 08:56) - PLID 60532 - The 'within 1 business day' check was occuring at the wrong time and has been moved into the subquery for the configured emn.
	// (r.farnworth 2014-04-29 10:35) - PLID 61821 - Clinical Summaries should automatically count in the numerator when a user merges the clinical summary.
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (s.dhole 2014-06-09 9:35) - PLID 62349 - Fix issue related to date join which was returning all emn from same date
	// (s.dhole 2014-06-10 14:35) - PLID 62349 - should count 1 emn per day even there are multiple emn with service code 99XXXXX
	// (d.singleton 2014-09-17 14:50) - PLID 63451 -  MU2 Core 8 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We now generate qualifying numerator and denominator values in temp tables
	// and aggregate them to produce our results.
	CString strSql;
	strSql.Format(R"(
/*MU.13/MU.CORE.08 Numerator*/
SELECT COUNT(*) AS NumeratorValue FROM %s
)"
	, riReport.GetClinicalSummaryNumeratorTempTableName());
	riReport.SetNumeratorSql(strSql);


	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (s.dhole 2014-06-09 9:35) - PLID 62349 - Fix issue related to date join which was returning all emn from same date
	// (s.dhole 2014-06-10 14:35) - PLID 62349 - should count 1 emn per day even there are multiple emn with service code 99XXXXX
	// (d.singleton 2014-09-17 14:50) - PLID 63451 -  MU2 Core 8 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We now generate qualifying numerator and denominator values in temp tables
	// and aggregate them to produce our results.
	CString strDenom;
	strDenom.Format(R"(
/*MU.13/MU.CORE.08 Denominator*/
SELECT COUNT(*) AS DenominatorValue FROM %s
)"
	, riReport.GetClinicalSummaryDenominatorTempTableName());
	riReport.SetDenominatorSql(strDenom);

	CString strSelect, strReportDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (r.farnworth 2014-01-31 08:56) - PLID 60532 - The 'within 1 business day' check was occuring at the wrong time and has been moved into the subquery for the configured emn.
	// (r.farnworth 2014-04-29 10:35) - PLID 61821 - Clinical Summaries should automatically count in the numerator when a user merges the clinical summary.
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (s.dhole 2014-06-09 9:35) - PLID 62349 - Fix issue related to date join which was returning all emn from same date
	// (s.dhole 2014-06-10 14:35) - PLID 62349 - should count 1 emn per day even there are multiple emn with service code 99XXXXX
	// (d.singleton 2014-09-17 14:50) - PLID 63451 -  MU2 Core 8 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We now generate qualifying numerator and denominator values in temp tables
	// and aggregate them to produce our results.
	strNum.Format(R"(
FROM 
(
	SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate, 
		 PersonT.HomePhone, PersonT.WorkPhone, 
		 'Visit Date' as ItemDescriptionLine, Numerator.Date as ItemDate, '' as ItemDescription, 
		 '' AS ItemProvider, '' as ItemSecProvider, 
		 '' as ItemLocation, 'Visit Date' as MiscDesc, Numerator.Date as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 
	FROM %s Numerator
	INNER JOIN PersonT ON PersonT.ID = Numerator.PatientID
	INNER JOIN PatientsT ON PatientsT.PersonID = Numerator.PatientID
) N 
)"
	, riReport.GetClinicalSummaryNumeratorTempTableName());

	// (r.farnworth 2014-01-30 10:48) - PLID 60532 - Clinical Summary report queries were filtering on the wrong fields.
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (s.dhole 2014-06-09 9:35) - PLID 62349 - Fix issue related to date join which was returning all emn from same date
		// (s.dhole 2014-06-10 14:35) - PLID 62349 - should count 1 emn per day even there are multiple emn with service code 99XXXXX
		// (d.singleton 2014-09-17 14:50) - PLID 63451 -  MU2 Core 8 (clinical summaries) - Bills not filtering on Provider - summary
	// (c.haag 2015-08-31) - PLID 65056 - Complete rewrite. We're completely pivoting on @clinicalSummaryDenominator which
	// contains all the qualifying visits
	strReportDenom.Format(R"(
FROM 
(
	SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  
		 PersonT.HomePhone, PersonT.WorkPhone, 
		 'Visit Date' as ItemDescriptionLine, Denominator.Date as ItemDate, '' as ItemDescription, 
		 '' AS ItemProvider, '' as ItemSecProvider, 
		 '' as ItemLocation, 'Visit Date' as MiscDesc, Denominator.Date as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 
	FROM %s Denominator
	INNER JOIN PersonT ON PersonT.ID = Denominator.PatientID
	INNER JOIN PatientsT ON PatientsT.PersonID = Denominator.PatientID
) P 
)"
	, riReport.GetClinicalSummaryDenominatorTempTableName());

	riReport.SetReportInfo(strSelect, strNum, strReportDenom);

	m_aryReports.Add(riReport);
}

//TES 10/15/2013 - PLID 59016 - Like the Stage 1 version, except with no Age qualification
void CCHITReportInfoListing::CreateStage2HeightReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2Height;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2 - Patients with Height recorded";
	riReport.m_strDisplayName = "Patients with Height recorded";
	
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items
	riReport.SetConfigureType(crctEMRDataGroupOrSlider);

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	
	riReport.SetHelpText("This report calculates how many patients have Height recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have the height recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.PH*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	riReport.SetDenominatorSql(FormatString("/*MU.PH*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "));

	CString strReportNum, strReportDenom;

	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE  \r\n"
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"	
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND EMRMasterT.ID IN (\r\n"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT \r\n"
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)\r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, eitSlider);

	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0  \r\n"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);
}

//TES 10/15/2013 - PLID 59016 - Like the Stage 1 version, except with no Age qualification
void CCHITReportInfoListing::CreateStage2WeightReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2Weight;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2 - Patients with Weight recorded";
	riReport.m_strDisplayName = "Patients with Weight recorded";
	
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items
	riReport.SetConfigureType(crctEMRDataGroupOrSlider);

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Weight recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have the weight recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.PW*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")", eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	riReport.SetDenominatorSql(FormatString("/*MU.PW*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "));

	CString strReportNum, strReportDenom;

	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE "
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"	AND EMRMasterT.ID IN ( \r\n"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT \r\n"
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	) \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, eitSlider);

	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0  \r\n"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] \r\n");

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);
}

//TES 10/15/2013 - PLID 59016 - Like the Stage 1 version, except filtered on Age > 3 instead of Age > 2
void CCHITReportInfoListing::CreateStage2BloodPressureReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2BloodPressure;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2 - Patients with Blood Pressure recorded";
	riReport.m_strDisplayName = "Patients with Blood Pressure recorded";
	
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items
	riReport.SetConfigureType(crctEMRDataGroupOrSlider);

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	//TES 10/15/2013 - PLID 59016 - Filtered on Age > 3 instead of Age > 2
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Blood Pressure recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients age 3 or older in the denominator that have blood pressure recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients age 3 or older who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 10/15/2013 - PLID 59016 - Filtered on Age > 3 instead of Age > 2
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	CString strSql;
	strSql.Format("/*MU.PBP*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] and PatientAgeT.PatientAgeNumber >= 3 "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")", strPatientAgeTempTableName, eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//TES 10/15/2013 - PLID 59016 - Filtered on Age > 3 instead of Age > 2
	riReport.SetDenominatorSql(FormatString("/*MU.PBP*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] and PatientAgeT.PatientAgeNumber >=3 ) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName));

	CString strReportNum, strReportDenom;

	//TES 10/15/2013 - PLID 59016 - Filtered on Age > 3 instead of Age > 2
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE  \r\n"
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND PatientAgeT.PatientAgeNumber >= 3  \r\n"		
			"	AND EMRMasterT.ID IN ( \r\n"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT \r\n"
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull(@EMRDataGroupID,-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull(@EMRMasterID,-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)\r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName, eitSingleList, eitMultiList, eitTable, eitSlider);

	//TES 10/15/2013 - PLID 59016 - Filtered on Age > 3 instead of Age > 2
	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  \r\n"
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0  \r\n"
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   \r\n"
		" AND PatientAgeT.PatientAgeNumber >= 3  \r\n"		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName);

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);
}

//TES 10/15/2013 - PLID 59016 - Like the Stage 1 version, except with no Age qualification
void CCHITReportInfoListing::CreateStage2HeightWeightReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2HeightWeight;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2 - Patients with Height and Weight recorded";	
	riReport.m_strDisplayName = "Patients with Height and Weight recorded";	

	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Height and Weight recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have height and weight recorded, defined by the selected EMR items in the configuration for those reports.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	// (j.jones 2014-11-06 11:00) - PLID 64087 - cached ConfigRT values to streamline query plans
	riReport.SetInitializationSQL("SET NOCOUNT ON \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded int; "
		"SET @cchitPatientsWithHeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Height recorded') \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded2 int; "
		"SET @cchitPatientsWithHeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Height recorded2') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded int; "
		"SET @cchitPatientsWithWeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Weight recorded') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded2 int; "
		"SET @cchitPatientsWithWeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Weight recorded2') \r\n"

		"SET NOCOUNT OFF \r\n");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1	
	CString strSql;
	strSql.Format("/*MU.PHW*/\r\n"

		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "		
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithHeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithHeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithHeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")"
			"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"   [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithWeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithWeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithWeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"
		")",
		eitSingleList, eitMultiList, eitTable, eitSlider,
		eitSingleList, eitMultiList, eitTable, eitSlider);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	riReport.SetDenominatorSql(FormatString("/*MU.PHW*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "));

	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";
	
//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" AND PatientsT.PersonID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0 ) "
		" AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithHeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithHeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithHeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithHeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"		
		//")"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"   [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
		"		WHERE EMRDetailsT.Deleted = 0 "
		"		AND ("
		"				(IsNull((@cchitPatientsWithWeightRecorded),-1) <> -1 "
		"					AND ("
		"						(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ("
		"							SELECT EMRSelectT.EMRDetailID FROM EMRSelectT "
		"							INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID "
		"							WHERE EMRDataT.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"					OR "
		"					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ("
		"						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT "
		"						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID "
		"						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID "
		"						WHERE EMRDataT_X.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) OR EMRDataT_Y.EMRDataGroupID = (@cchitPatientsWithWeightRecorded) "
		"						)"
		"					)"
		"				)"
		"				OR "
		"				(IsNull((@cchitPatientsWithWeightRecorded2),-1) <> -1 AND EMRInfoT.DataType = %li "
		"					AND EMRDetailsT.ID IN ( "
		"						SELECT EMRDetailsT.ID FROM EMRDetailsT "
		"						INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID	"
		"						WHERE EMRInfoT.EMRInfoMasterID = (@cchitPatientsWithWeightRecorded2) "
		"						AND EMRDetailsT.SliderValue Is Not Null "
		"					) "
		"				)"
		"			)"
		"		)"
		"	)"	
		") "
		") N "
		, eitSingleList, eitMultiList, eitTable, eitSlider,
		eitSingleList, eitMultiList, eitTable, eitSlider);
	
	//TES 10/15/2013 - PLID 59016 - Stage 2 doesn't filter this by age
	strDenom = FormatString(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ");

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}

//TES 10/15/2013 - PLID 59006 - MU.CORE.04
void CCHITReportInfoListing::CreateStage2HeightWeightBPReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2HeightWeightBP;
	//TES 10/15/2013 - PLID 59006 - Use MU2 internally to distinguish
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.C04 - Patients with Height, Weight, and Blood Pressure recorded";	
	riReport.m_strDisplayName = "MU.CORE.04 - Patients with Vitals recorded";	
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	//TES 10/15/2013 - PLID 59006 - MU.CORE.04 - This is similar to Stage 1 MU.CORE.08. 
	// However, the age filter is removed for Height and Weight, and changed from >2 to >3 for Blood Pressure
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have Height and Weight recorded, and how many patients older than 3 have Blood Pressure recorded. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator where the height and weight have been recorded, and if over the age of 3, the blood pressure has also been recorded, defined by the selected EMR item in the configuration for those reports.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	// (j.jones 2014-11-06 11:00) - PLID 64087 - cached ConfigRT values to streamline query plans
	// (j.jones 2014-11-07 15:48) - PLID 64101 - moved the EMR detail searches to return their EMR IDs in one temp table
	riReport.SetInitializationSQL(FormatString("SET NOCOUNT ON \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded int; "
		"SET @cchitPatientsWithHeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Height recorded') \r\n"

		"DECLARE @cchitPatientsWithHeightRecorded2 int; "
		"SET @cchitPatientsWithHeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Height recorded2') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded int; "
		"SET @cchitPatientsWithWeightRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Weight recorded') \r\n"

		"DECLARE @cchitPatientsWithWeightRecorded2 int; "
		"SET @cchitPatientsWithWeightRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Weight recorded2') \r\n"

		"DECLARE @cchitPatientsWithBloodPressureRecorded int; "
		"SET @cchitPatientsWithBloodPressureRecorded = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Blood Pressure recorded') \r\n"

		"DECLARE @cchitPatientsWithBloodPressureRecorded2 int; "
		"SET @cchitPatientsWithBloodPressureRecorded2 = (SELECT IntParam FROM ConfigRT WHERE NAME = 'CCHITReportInfo_MU2 - Patients with Blood Pressure recorded2') \r\n"

		////////////////////
		//We need a list of all EMNs (unfiltered), that have Height, Weight, or Blood Pressure tracked
		"DECLARE @EMRMasterT_Match TABLE (ID INT, Height BIT, Weight BIT, BP BIT) \r\n"
		"\r\n"
		"INSERT INTO @EMRMasterT_Match (ID, Height, Weight, BP) \r\n"
		//Get all single/multi-select lists
		"SELECT EMRMasterT.ID, \r\n"
		"	(CASE WHEN EMRDataT.EMRDataGroupID = @cchitPatientsWithHeightRecorded THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRDataT.EMRDataGroupID = @cchitPatientsWithWeightRecorded THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRDataT.EMRDataGroupID = @cchitPatientsWithBloodPressureRecorded THEN 1 ELSE 0 END) \r\n"
		"FROM EMRMasterT WITH(NOLOCK) \r\n"
		"INNER JOIN EMRDetailsT WITH(NOLOCK) ON EMRMasterT.ID = EMRDetailsT.EMRID \r\n"
		"INNER JOIN EMRSelectT WITH(NOLOCK) ON EMRDetailsT.ID = EMRSelectT.EmrDetailID \r\n"
		"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"INNER JOIN EMRDataT WITH(NOLOCK) ON EMRSelectT.EMRDataID = EMRDataT.ID \r\n"
		"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType IN (%li, %li) \r\n"
		"AND EMRDataT.EMRDataGroupID IN (@cchitPatientsWithHeightRecorded, @cchitPatientsWithWeightRecorded, @cchitPatientsWithBloodPressureRecorded) \r\n"
		"\r\n"
		//Get all tables
		"UNION "
		"SELECT EMRMasterT.ID, \r\n"
		"	(CASE WHEN(EMRDataT_X.EMRDataGroupID = @cchitPatientsWithHeightRecorded OR EMRDataT_Y.EMRDataGroupID = @cchitPatientsWithHeightRecorded) THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN(EMRDataT_X.EMRDataGroupID = @cchitPatientsWithWeightRecorded OR EMRDataT_Y.EMRDataGroupID = @cchitPatientsWithWeightRecorded) THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN(EMRDataT_X.EMRDataGroupID = @cchitPatientsWithBloodPressureRecorded OR EMRDataT_Y.EMRDataGroupID = @cchitPatientsWithBloodPressureRecorded) THEN 1 ELSE 0 END) \r\n"
		"FROM EMRMasterT WITH(NOLOCK) \r\n"
		"INNER JOIN EMRDetailsT WITH(NOLOCK) ON EMRMasterT.ID = EMRDetailsT.EMRID \r\n"
		"INNER JOIN EMRDetailTableDataT WITH(NOLOCK) ON EMRDetailsT.ID = EMRDetailTableDataT.EMRDetailID \r\n"
		"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"INNER JOIN EMRDataT EMRDataT_X WITH(NOLOCK) ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID \r\n"
		"INNER JOIN EMRDataT EMRDataT_Y WITH(NOLOCK) ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID \r\n"
		"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType = %li \r\n"
		"AND (\r\n"
		"	EMRDataT_X.EMRDataGroupID IN (@cchitPatientsWithHeightRecorded, @cchitPatientsWithWeightRecorded, @cchitPatientsWithBloodPressureRecorded) \r\n"
		"	OR \r\n"
		"	EMRDataT_Y.EMRDataGroupID IN (@cchitPatientsWithHeightRecorded, @cchitPatientsWithWeightRecorded, @cchitPatientsWithBloodPressureRecorded) \r\n"
		") \r\n"
		"\r\n"
		//Get all sliders
		"UNION \r\n"
		"SELECT EMRMasterT.ID, \r\n"
		"	(CASE WHEN EMRInfoT.EMRInfoMasterID = @cchitPatientsWithHeightRecorded2 THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRInfoT.EMRInfoMasterID = @cchitPatientsWithWeightRecorded2 THEN 1 ELSE 0 END), \r\n"
		"	(CASE WHEN EMRInfoT.EMRInfoMasterID = @cchitPatientsWithBloodPressureRecorded2 THEN 1 ELSE 0 END) \r\n"
		"FROM EMRMasterT WITH(NOLOCK) \r\n"
		"INNER JOIN EMRDetailsT WITH(NOLOCK) ON EMRMasterT.ID = EMRDetailsT.EMRID \r\n"
		"INNER JOIN EMRInfoT WITH(NOLOCK) ON EMRDetailsT.EMRInfoID = EMRInfoT.ID \r\n"
		"WHERE EMRMasterT.Deleted = 0 AND EMRDetailsT.Deleted = 0 AND EMRInfoT.DataType = %li AND EMRDetailsT.SliderValue Is Not Null \r\n"
		"AND EMRInfoT.EMRInfoMasterID IN (@cchitPatientsWithHeightRecorded2, @cchitPatientsWithWeightRecorded2, @cchitPatientsWithBloodPressureRecorded2) \r\n"
		"\r\n"
		"SET NOCOUNT OFF \r\n",
		eitSingleList, eitMultiList, eitTable, eitSlider));

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 10/15/2013 - PLID 59006 - MU.CORE.04 - Numerator => Has Height AND Has Weight AND (Age < 3 OR Has Blood Pressure)
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	// (j.jones 2014-11-06 11:14) - PLID 64087 - cached ConfigRT values to streamline query plans
	CString strSql;
	strSql.Format("/*MU.04/MU.CORE.04*/\r\n"

		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "		
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		"AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter]  "
		"	AND EMRMasterT.ID IN ("
		"		SELECT ID FROM @EMRMasterT_Match WHERE Height = 1 "
		"	)"
		" )"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 "
		"   [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND EMRMasterT.ID IN ("
		"		SELECT ID FROM @EMRMasterT_Match WHERE Weight = 1 "
		"	)"
		" )"
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	AND (PatientAgeT.PatientAgeNumber < 3 OR (EMRMasterT.ID IN ("
		"		SELECT ID FROM @EMRMasterT_Match WHERE BP = 1 "
		"	)))"
		")", strPatientAgeTempTableName);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	//TES 10/15/2013 - PLID 59006 - MU.CORE.04 - No age filter (if they're younger than 3, it won't look at blood pressure for the numerator, just height and weight)
	riReport.SetDenominatorSql(FormatString("/*MU.09/MU.CORE.08*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName));


	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//TES 10/15/2013 - PLID 59006 - MU.CORE.04 - Numerator => Has Height AND Has Weight AND (Age < 3 OR Has Blood Pressure)
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (j.jones 2014-02-05 10:23) - PLID 60651 - this now allows sliders in addition to list & table items,
	// mutually exclusive, either @EMRDataGroupID is -1 or @EMRMasterID is -1
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND EMRMasterT.ID IN ("
		"	SELECT ID FROM @EMRMasterT_Match WHERE Height = 1 "
		" )"		
		" AND EMRMasterT.ID IN ("
		"	SELECT ID FROM @EMRMasterT_Match WHERE Weight = 1 "
		" )"
		" AND (PatientAgeT.PatientAgeNumber < 3 OR (EMRMasterT.ID IN ("
		"	SELECT ID FROM @EMRMasterT_Match WHERE BP = 1 "
		"))) "
		") N "
		, strPatientAgeTempTableName);

	//TES 10/15/2013 - PLID 59006 - MU.CORE.04 - No age filter (if they're younger than 3, it won't look at blood pressure for the numerator, just height and weight)
	strDenom.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ");

	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}

//TES 10/16/2013 - PLID 59007 - MU.CORE.05
void CCHITReportInfoListing::CreateStage2SmokingStatusReport()
{
	
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.SetPercentToPass(80);
	riReport.m_nInternalID = (long)eMUStage2SmokingStatus;
	riReport.SetConfigureType(crctEMRItem);
	//TES 10/16/2013 - PLID 59007 - Keep the same internal name, so that they share the same configured EMR item
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.C05 - Patients who have smoking status reported.";
	riReport.m_strDisplayName = "MU.CORE.05 - Patients who have smoking status reported.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients who were 13 years of age or older when they were seen and had a smoking status recorded. Filtering is applied on EMN date, EMN location, and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients age 13 or older in the denominator where smoking status has been recorded, defined by the selected EMR item in the configuration for this report.",
		"The number of unique patients age 13 or older who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	CString strPatientAgeTempTableName = riReport.EnsurePatientAgeTempTableName();

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	CString strNum;
	strNum.Format("/*MU.10/MU.CORE.05*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"
		"	 WHERE EMRMasterT.Deleted = 0 		"
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "	
		" ) "
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		" INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0 AND PatientAgeT.PatientAgeNumber >= 13 ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, strPatientAgeTempTableName);
	riReport.SetNumeratorSql(strNum);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDenominatorSql(FormatString("/*MU.10/MU.CORE.05*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"	
		"	 INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		"	 WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND PatientAgeT.PatientAgeNumber >= 13) "		
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ",
		strPatientAgeTempTableName));
	

	CString strReportNum, strReportDenom;
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE  "
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		" INNER JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0 AND PatientAgeT.PatientAgeNumber >= 13 ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable, strPatientAgeTempTableName);

	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN %s PatientAgeT ON EMRMasterT.ID = PatientAgeT.EMRID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND PatientAgeT.PatientAgeNumber >= 13 "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", strPatientAgeTempTableName);

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);

}


// (r.farnworth 2013-10-16 14:09) - PLID 59036 - MU.CORE.10
void CCHITReportInfoListing::CreateStage2ClinicalLabResultsReport()
{

	CCCHITReportInfo riReport;	
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2ClinicalLabResults;
	riReport.SetPercentToPass(55);
	riReport.SetConfigureType(crctMailsentCat);
	// (r.farnworth 2013-10-16 14:09) - PLID 59036 - MU.CORE.02 is the stage 1 variant of this measure
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.C10 - Clinical Labs having results in structured data.";
	riReport.m_strDisplayName = "MU.CORE.10 - Clinical Labs having results in structured data.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many clinical labs have positive, negative, or numerical results entered in as structured data. Clinical labs are those associated with a lab type that is Biopsy, Lab Work, or Cultures. Filters on Lab Result Received Date, Lab Location, and Lab Provider for lab data and History Service Date, Patient Location, and Patient Provider for document data.",
		"The number of all biopsy, labwork, and culture type labs in the denominator that have a flag selected or a number as the first character in the value field.",
		"The number of all biopsy, labwork, and culture type labs with the selected provider(s) and location and any history document for a patient with the selected provider(s) and location with a category stating it is not a lab not entered into the Labs tab, defined by the history category in the configuration for this report.");

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 10:50) - PLID 62064 - Trim leading/trailing spaces for LabResultsT.Value
	riReport.SetNumeratorSql("/*MU.11/MU.CORE.10*/\r\n"
		" SELECT Count(*) as NumeratorValue FROM LabsT "
		"WHERE LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0  "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo  "
		"	 AND (LabResultsT.FlagID IS NOT NULL "
		"	 OR CASE WHEN isnumeric(convert(nvarchar, Left(CONVERT(nVarChar(255), LTRIM(RTRIM(LabResultsT.Value))), 1)) + 'e0') <> 0 THEN 1 ELSE 0 END = 1) "
		") "
		" [LabProvFilter] [LabLocFilter] "
		"AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		"AND PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		);

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (r.farnworth 2014-03-13 11:27) - PLID 61331 - MU2 Report Core.10 denominator was counting labs with multiple results more than once.
	riReport.SetDenominatorSql("/*MU.11/MU.CORE.10*/\r\n"
		"SELECT Sum(DenominatorValue) AS DenominatorValue FROM ( "
		"SELECT Count(*) as DenominatorValue FROM LabsT "
		"WHERE LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0  "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo ) "
		" [LabProvFilter] [LabLocFilter] "
		"AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		"AND PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" UNION ALL "
		"SELECT Count(*) FROM MailSent WHERE  "
		"			PersonID IN (SELECT PersonID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter] ) "
		"			AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		"			AND CategoryID = @MailSentCatID "
		")Q"
		);

	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (b.savon 2014-05-07 10:50) - PLID 62064 - Trim leading/trailing spaces for LabResultsT.Value
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate, "
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 "
		" AND LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo  "		
		"	 AND (LabResultsT.FlagID IS NOT NULL "
		"	 OR CASE WHEN isnumeric(convert(nvarchar, Left(CONVERT(nVarChar(255), LTRIM(RTRIM(LabResultsT.Value))), 1)) + 'e0') <> 0 THEN 1 ELSE 0 END = 1) "
		") "
		" [LabProvFilter] [LabLocFilter] "
		" AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	// (r.farnworth 2013-12-17 12:38) - PLID 59036 - Removed joining on patient EMNs
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	// (r.farnworth 2014-03-13 11:27) - PLID 61331 - MU2 Report Core.10 denominator was counting labs with multiple results more than once.
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate,	"
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.ID IN (SELECT LabID FROM LabResultsT "
		"	 WHERE LabResultsT.Deleted = 0 AND LabsT.Deleted = 0 AND LabsT.Discontinued = 0  "
		"	 AND DateReceived >= @DateFrom AND DateReceived < @DateTo ) "
		" [LabProvFilter] [LabLocFilter] "
		" AND LabProcedureID IN (SELECT ID FROM LabProceduresT WHERE Type IN (1,2,3)) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter]  "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "		
		") P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);

}

// (r.farnworth 2013-10-16 15:58) - PLID 59055 - MU.CORE.12
//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 Update query to call Common code
void CCHITReportInfoListing::CreateStage2RemindersReport()
{

	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2PreventiveCare;
	riReport.SetPercentToPass(10);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.C12 - Patients who were sent appropriate reminders";
	riReport.m_strDisplayName = "MU.CORE.12 - Patients who were sent appropriate reminders";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);
	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
	CString strCommonDenominatorSQL = GetClinicalSummaryCommonDenominatorSQL(strCodes);
	CString strCodeDesc;
	strCodeDesc.Format("either with the following service codes or on the same day as an EMN or charge with a Service Code in %s", strCodes);
	// (b.savon 2014-04-22 07:30) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	//(s.dhole 9/8/2014 3:29 PM ) - PLID 63580 Change the numerator description to support sentreminder date
	riReport.SetHelpText("This report calculates how many patients were sent an appropriate reminder. Date filtering on the numerator is on EMN Date or the General 1 reminder date. Provider and location filtering is on EMN Primary or Secondary Provider(s) and EMN Location. ",
		"The number of unique patients in the denominator who have appropriate reminders sent to them within the date range, defined by the selected EMR item in the configuration for this report or the reminders on the patients’ General 1 tabs.",
		"The number of unique patients that have 2 non-excluded EMNs " + strCodeDesc + " within the last two years of the beginning of the date range with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//(s.dhole 9/8/2014 11:54 AM ) - PLID 62795 Added SentReminder union
	CString strSqlNum;
	strSqlNum.Format(R"(/*MU.14/MU.CORE.12*/
		--we have to subtract one from DateTo since it is auto incremented
		 SET @DateToUse = DateAdd(dd,-1,@DateTo); 
		 SELECT COUNT(PersonT.ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		 WHERE PersonID IN (SELECT PatientID FROM EMRMasterT  
			WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] 
			AND EMRMasterT.ID IN (
				SELECT EMRDetailsT.EMRID FROM EMRDetailsT 
				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID 
				WHERE EMRDetailsT.Deleted = 0 
				AND (
					(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN (
						SELECT EMRSelectT.EMRDetailID FROM EMRSelectT 
						INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID 
						WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) 
					)
			OR 
					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN (
						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT 
						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID 
						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID 
						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) 
					)
				)
			)
		--(s.dhole 9/8/2014 11:54 AM ) - PLID 62795  PatientRemindersSentT  union
		UNION 
		--(s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
		 SELECT PatientID FROM PatientRemindersSentT WHERE ( PatientRemindersSentT.Deleted =0 AND ReminderDate >= @DateFrom  AND ReminderDate < @DateTo) 
		 ) 
		 AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] 
		-- (r.farnworth 2013-10-17 17:18) - PLID 59055 - Need to check if they've had two or more office visits in the past two years
		-- (r.farnworth 2014-05-02 16:22) - PLID 62024 - Stage 2 Reminders Report was incorrectly looking for office visits in the 24 months prior to the reporting period.
		-- (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude void charges
		-- (d.singleton 2014-09-17 15:44) - PLID 63454 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Summary
		 AND PersonID IN ( 
			SELECT Q.PatientID 
			FROM( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				FROM EMRMasterT 
				INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			 ) Q 
			INNER JOIN ( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				 FROM EMRMasterT 
				INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			) N ON Q.PatientID = N.PatientID 
			WHERE Q.DATE >= DATEADD(year, -2, @DateFrom) 
			AND Q.DATE < @DateFrom 
			AND  N.DATE >= DATEADD(year, -2, @DateFrom) 
			AND N.DATE < @DateFrom 
			AND Q.ChargeID <> N.ChargeID 
			AND Q.ID <> N.ID 
		  ) )"
		, eitSingleList, eitMultiList, eitTable,  strCommonDenominatorSQL, strCommonDenominatorSQL);
	riReport.SetNumeratorSql(strSqlNum);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	CString strSqlDen;
	strSqlDen.Format(R"(/*MU.14/MU.CORE.12*/
		--we have to subtract one from DateTo since it is auto incremented
		 SET @DateToUse = DateAdd(dd,-1,@DateTo);  
		SELECT COUNT(PersonT.ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID
		 WHERE  
		 (PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE EMRMasterT.Deleted = 0 
		[ProvFilter] [LocFilter] [ExclusionsFilter] ) 
		AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] 
		-- (r.farnworth 2013-10-17 17:18) - PLID 59055 - Need to check if they've had two or more office visits in the past two years
		-- (r.farnworth 2014-05-02 16:22) - PLID 62024 - Stage 2 Reminders Report was incorrectly looking for office visits in the 24 months prior to the reporting period.
		-- (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
		--(s.dhole 9/8/2014 11:54 AM ) - PLID 62795 Added sentreminder  union
		-- (d.singleton 2014-09-17 15:44) - PLID 63454 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Summary
		 AND PersonID IN ( 
			SELECT Q.PatientID 
			FROM( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				FROM EMRMasterT 
				INNER JOIN %s  ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			 ) Q 
			INNER JOIN ( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				 FROM EMRMasterT 
				INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			) N ON Q.PatientID = N.PatientID 
			WHERE Q.DATE >= DATEADD(year, -2, @DateFrom) 
			AND Q.DATE < @DateFrom 
			AND  N.DATE >= DATEADD(year, -2, @DateFrom) 
			AND N.DATE < @DateFrom 
			AND Q.ChargeID <> N.ChargeID 
			AND Q.ID <> N.ID 
		 )) )"
		, strCommonDenominatorSQL, strCommonDenominatorSQL);

	riReport.SetDenominatorSql(strSqlDen);

	CString strSelect, strDenom, strNum;
	//(s.dhole 9/8/2014 11:54 AM ) - PLID 62795 Added SentReminder to union and change ItemDescriptionLine from 'EMN' to 'EMNs/Reminder'
	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	strNum.Format(R"(FROM 
		 ( SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  
		 PersonT.HomePhone, PersonT.WorkPhone, 
		 'EMNs/Reminder' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, 
		 dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, 
		 LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 
		 FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		 LEFT JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID 
		 LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID 
		 WHERE 
		   (  
		     ( EMRMasterT.ID IN 
		        ( SELECT EMRMasterT.ID FROM EMRMasterT  
			    WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] 
			    AND EMRMasterT.ID IN (
				SELECT EMRDetailsT.EMRID FROM EMRDetailsT 
				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID 
				WHERE EMRDetailsT.Deleted = 0 
				AND (
				     	(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN (
						SELECT EMRSelectT.EMRDetailID FROM EMRSelectT 
					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID 
						WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID) 
					    )
				      OR 
					    (EMRInfoT.DataType = %li AND EMRDetailsT.ID IN (
						SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT 
						INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID
						INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID 
						WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID) 
					    )
				    )
			                    )
		       )
		      ) 
		     AND EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom and EMRMasterT.Date < @DateTo    
		   ) 
		--(s.dhole 9/8/2014 11:54 AM ) - PLID 62795  PatientRemindersSentT  union
		
		 AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] 
		 UNION ALL
		 SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  
		 PersonT.HomePhone, PersonT.WorkPhone, 
		 'EMNs/Reminder' as ItemDescriptionLine, PatientRemindersSentT.ReminderDate as ItemDate, %s  as ItemDescription, 
		 NULL AS ItemProvider, NULL as ItemSecProvider, 
		 '' as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 
		 FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		  INNER JOIN PatientRemindersSentT ON PatientRemindersSentT.PatientID = PersonT.ID 
		-- (s.tullis 2015-06-22 15:27) - PLID 66442 - Change patients reminder structure to have a deleted flag instead of permanently deleting the reminder.
		 WHERE PatientRemindersSentT.Deleted = 0 AND  ReminderDate >= @DateFrom AND PatientRemindersSentT.ReminderDate < @DateTo AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] 
		 )  AS N 
		-- (r.farnworth 2013-10-17 17:18) - PLID 59055 - Need to check if they've had two or more office visits in the past two years
		-- (r.farnworth 2014-05-02 16:22) - PLID 62024 - Stage 2 Reminders Report was incorrectly looking for office visits in the 24 months prior to the reporting period.
		-- (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
		--(s.dhole 9/8/2014 11:54 AM ) - PLID 62795 Added sent reminde union
		-- (d.singleton 2014-09-17 15:44) - PLID 63454 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Summary
		 WHERE N.PatientID IN ( 
			SELECT QQ.PatientID 
			FROM( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS  ChargeID, EMRMasterT.ID 
				FROM EMRMasterT 
				INNER JOIN %s  ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			 ) QQ 
			INNER JOIN ( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				 FROM EMRMasterT 
				INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			) NN ON QQ.PatientID = NN.PatientID 
			WHERE QQ.DATE >= DATEADD(year, -2, @DateFrom) 
			AND QQ.DATE < @DateFrom 
			AND  NN.DATE >= DATEADD(year, -2, @DateFrom) 
			AND NN.DATE < @DateFrom 
			AND QQ.ChargeID <> NN.ChargeID 
			AND QQ.ID <> NN.ID 
		 )  )"
		, eitSingleList, eitMultiList, eitTable, GetSQLReminderMethods(), strCommonDenominatorSQL, strCommonDenominatorSQL);

		//(s.dhole 9/8/2014 11:54 AM ) - PLID 62795 Added SentReminder union
	strDenom.Format(R"( FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, 
		 PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMNs/Reminder' as ItemDescriptionLine, 
		 EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, 
		 dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, 
		 dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, 
		 LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  
		 FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  
		 LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID 
		 LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID 
		 WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] ) 
		 AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] 
		 AND EMRMasterT.Date >= DATEADD(year, -2, @DateFrom) and EMRMasterT.Date < @DateFrom 
		 AND Deleted = 0 
		-- (r.farnworth 2013-10-17 17:18) - PLID 59055 - Need to check if they've had two or more office visits in the past two years
		-- (r.farnworth 2014-05-02 16:22) - PLID 62024 - Stage 2 Reminders Report was incorrectly looking for office visits in the 24 months prior to the reporting period.
		-- (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
		-- (d.singleton 2014-09-17 15:44) - PLID 63454 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Summary
		 AND EMRMasterT.ID IN ( 
			SELECT Q.ID 
			FROM( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				FROM EMRMasterT 
				INNER JOIN %s  ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			 ) Q 
			INNER JOIN ( 
				SELECT EMRMasterT.PatientID, EMRMasterT.DATE, ChargeFilterQ.ID AS ChargeID, EMRMasterT.ID 
				 FROM EMRMasterT 
				INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID 
					AND ChargeFilterQ.DATE = EmrMasterT.DATE 
				WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			) N ON Q.PatientID = N.PatientID 
			WHERE Q.DATE >= DATEADD(year, -2, @DateFrom) 
			AND Q.DATE < @DateFrom 
			AND  N.DATE >= DATEADD(year, -2, @DateFrom) 
			AND N.DATE < @DateFrom 
			AND Q.ChargeID <> N.ChargeID 
			AND Q.ID <> N.ID 
		 )
		
		 ) P )"
		, strCommonDenominatorSQL, strCommonDenominatorSQL);


	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}


// (r.farnworth 2013-10-21 10:22) - PLID 59116 - MU.CORE.13
//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 Update query to call Common code
void CCHITReportInfoListing::CreateStage2EducationResourceEMRReport(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2EducationResourceEMRReport;
	riReport.SetPercentToPass(10);
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.6" : "MU.CORE.13");
	riReport.m_strInternalName = "MU2.C13 - Patients with Educational Resources Sent";
	riReport.m_strDisplayName = strMeasurePrefix + " - Patients with Educational Resources Sent";
	riReport.SetConfigureType(crctEMRDataGroup);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);

	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
	CString strCommonDenominatorSQL = GetClinicalSummaryCommonDenominatorSQL(strCodes);

	CString strCodeDesc;
	strCodeDesc.Format("either with the following service codes or on the same day as an EMN or charge with a Service Code in %s", strCodes); 
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	// (r.farnworth 2014-05-14 07:44) - PLID 62134 - Change the numerator text in MUS2 Core 8, MUS2 Core 13, MUS2 Core 14
	riReport.SetHelpText("This report calculates how many patients with office visits have been provided patient-specific educational resources. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator that have been provided with educational resources, defined by the selected EMR item in the configuration for this report. The EMR item can be substituted with patient education generated through the blue informational buttons or hyperlinks.",
		"The number of unique patients with a non-excluded EMN with one of the following service codes or a charge on the same date as the EMN with a service code in " + strCodes + " within the date range and with the selected providers(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (r.farnworth 2014-05-14 16:03) - PLID 62138 - Patient Education Resources Stage 2 Core 13 should automatically count in the numerator when a user clicks on any of our informational buttons or hyperlinks
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (d.singleton 2014-09-17 15:00) - PLID 63452 - MU2 Core 13 (Education) - Bills not filtering on Provider - summary
	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 Update query to call Common
	CString strSql;
	strSql.Format(R"(/*MU.05/MU.CORE.13*/
		 SELECT COUNT(*) AS NumeratorValue FROM (  
		 SELECT EMRMasterT.PatientID FROM EMRMasterT  
		 INNER JOIN PatientsT ON EMRMasterT.PatientID = PatientsT.PersonID 	 
		 WHERE  
		 PatientsT.CurrentStatus <> 4 AND EMRMasterT.PatientID > 0 [PatientIDFilter] [ProvFilter] [LocFilter] [ExclusionsFilter]  
		 AND EMRMasterT.Deleted = 0 		 
		 AND ( 
			EMRMasterT.ID IN ( 
				SELECT EMRDetailsT.EMRID FROM EMRDetailsT  
				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  
				WHERE EMRDetailsT.Deleted = 0  
				AND ( 
					(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( 
					 SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  
					 INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  
					 WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID)  
					) 
				OR  
					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( 
					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  
					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID 
					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID
					WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID)  
					) 
				) 
			) 
		 OR EMRMasterT.PatientID IN(SELECT PatientID FROM EducationResourceAccessT) 
		 ) 
		 AND PatientsT.PersonID IN 
		   (SELECT EMRMasterT.PatientID FROM EMRMasterT 
			INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
			WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND EMRMasterT.Deleted = 0) 
		 GROUP BY EMRMasterT.PatientID  
		 )Q )"  
		, eitSingleList, eitMultiList, eitTable, strCommonDenominatorSQL);
	riReport.SetNumeratorSql(strSql);

	// (r.farnworth 2013-10-21 11:53) - PLID 59116 - Can no longer call the default denominator here
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (d.singleton 2014-09-17 15:00) - PLID 63452 - MU2 Core 13 (Education) - Bills not filtering on Provider - summary
	CString strDenom;
	strDenom.Format(R"(/*MU.05/MU.CORE.13*/
		SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  
		 WHERE PersonT.ID IN (SELECT EMRMasterT.PatientID FROM EMRMasterT 
			INNER JOIN %s  ON EMRMasterT.PatientID = ChargeFilterQ.PatientID  AND ChargeFilterQ.Date = EmrMasterT.Date 
		 WHERE DELETED = 0  
		 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] )  
		AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] )"
		, strCommonDenominatorSQL);
	riReport.SetDenominatorSql(strDenom);

	
	CString strNum;
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	// (r.farnworth 2014-05-14 16:03) - PLID 62138 - Patient Education Resources Stage 2 Core 13 should automatically count in the numerator when a user clicks on any of our informational buttons or hyperlinks
	// (s.dhole 2014-05-03 10:35) - PLID 62293 - exclude voide charges
	// (d.singleton 2014-09-17 15:06) - PLID 63452 - MU2 Core 13 (Education) - Bills not filtering on Provider - summary
	strNum.Format(R"( FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID
		 LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID 
		 LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID 
		 WHERE  
		 PatientsT.CurrentStatus <> 4 AND EMRMasterT.PatientID > 0 [PatientIDFilter] [ProvFilter] [LocFilter] [ExclusionsFilter]  
		 AND EMRMasterT.Deleted = 0 		 
		 AND ( 
			EMRMasterT.ID IN ( 
				SELECT EMRDetailsT.EMRID FROM EMRDetailsT  
				INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  
				WHERE EMRDetailsT.Deleted = 0  
				AND ( 
					(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( 
					 SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  
					 INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  
					 WHERE EMRDataT.EMRDataGroupID = @EMRDataGroupID)  
					) 
				OR  
					(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( 
					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  
					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID 
					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID
					WHERE EMRDataT_X.EMRDataGroupID = @EMRDataGroupID OR EMRDataT_Y.EMRDataGroupID = @EMRDataGroupID)  
					) 
				) 
			) 
		 OR EMRMasterT.PatientID IN(SELECT PatientID FROM EducationResourceAccessT) 
		 ) 
		 AND 
			PatientsT.PersonID IN 
			(SELECT EMRMasterT.PatientID FROM EMRMasterT 
				INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
			WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND EMRMasterT.Deleted = 0 
			) )"
		, eitSingleList, eitMultiList, eitTable, strCommonDenominatorSQL);

	CString strDenomR;
	strDenomR.Format(R"( FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		 LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID 
			INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
		 LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID 
		 WHERE  
		 EMRMasterT.DELETED = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] 
		 AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] )", strCommonDenominatorSQL);

		riReport.SetReportInfo("", strNum, strDenomR);

	m_aryReports.Add(riReport);
}

//TES 10/16/2013 - PLID 59047 - MU.MENU.04
void CCHITReportInfoListing::CreateStage2FamilyHistoryReport()
{
	
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.SetPercentToPass(20);
	riReport.m_nInternalID = (long)eMUStage2FamilyHistory;
	riReport.SetConfigureType(crctEMRItem);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.M04 - Patients who have family history reported.";
	riReport.m_strDisplayName = "MU.MENU.04 - Patients who have family history reported.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients had a family history recorded. Filtering is applied on EMN date, EMN location, and EMN Primary or Secondary Provider(s). ",
		"The number of unique patients in the denominator that have a value for family history, defined by the EMR item in the configuration for this report.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");


	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	CString strNum;
	strNum.Format("/*MU2.MENU.04/MU.MENU.04*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"
		"	 WHERE EMRMasterT.Deleted = 0 		"
		"    [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "	
		" ) "
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strNum);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDenominatorSql(FormatString("/*MU2.MENU.04/MU.MENU.04*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN "
		"	(SELECT PatientID FROM EMRMasterT 	"	
		"	 WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] )"
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "));
	

	CString strReportNum, strReportDenom;
	//TES 12/26/2013 - PLID 60109 - Fixed numerator calculation to prevent possible > 100% results
	strReportNum.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE  "
		" EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	) "	
		" AND PersonT.ID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ", eitSingleList, eitMultiList, eitTable);

	strReportDenom.Format(" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ");

	riReport.SetReportInfo("", strReportNum, strReportDenom);

	m_aryReports.Add(riReport);

}

// (r.farnworth 2013-10-21 15:38) - PLID 59127 - MU.CORE.01.A
void CCHITReportInfoListing::CreateStage2CPOEReport_MedicationOrders(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2CPOEReport_MedicationOrders;
	riReport.SetConfigureType(crctMailsentCat);
	riReport.SetPercentToPass(60);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.3.1" : "MU.CORE.01.A");
	riReport.m_strInternalName = "MU2.C01A - Medications orders which have been entered through CPOE";
	riReport.m_strDisplayName = strMeasurePrefix + " - Medication orders which have been entered through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many medications have been ordered through CPOE. "
		"Filtering is applied to EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of prescriptions in the denominator entered electronically.",
		"The number of prescriptions entered in the date range and with the selected provider(s) and location and all written prescriptions scanned in, defined by the selected history category in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (r.farnworth 2013-10-21 16:28) - PLID 59127 - Need to change to number of medication orders instead of patients with at least one prescription entered. 
	riReport.SetNumeratorSql("/*MU.07/MU.CORE.01.A*/\r\n"
		"SELECT COUNT(MedicationID) AS 'NumeratorValue' FROM PatientMedications "
		"WHERE DELETED = 0 AND Discontinued = 0 "
		"AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter]  "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 [PatientIDFilter]) "
		);

	////Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.farnworth 2013-10-21 16:28) - PLID 59127 - Need to change to number of medication orders instead of patients with at least one prescription entered.
	riReport.SetDenominatorSql("/*MU.07/MU.CORE.01.A*/\r\n"
		"SELECT SUM(PrescriptionC) AS 'DenominatorValue' FROM ( "
		"SELECT Count(*) AS PrescriptionC FROM PatientMedications "
		"WHERE DELETED = 0 AND Discontinued = 0 "
		"AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter]  "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 [PatientIDFilter]) "
		"UNION ALL "
		"SELECT Count(*) AS PrescriptionC FROM MailSent WHERE "
		"PersonID IN (SELECT PersonID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PatientsT.PersonID "
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter] ) "
		"AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		"AND CategoryID = @MailSentCatID "
		" )Q "
		);

	
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First,Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2,  Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'Prescription' as ItemDescriptionLine, PrescriptionDate as ItemDate, LEFT(EMRDataT.Data,255) as ItemDescription, \r\n"
		" PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ItemProvider, '' as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, 'Destination Type' as MiscDesc, \r\n"
		" CASE WHEN FinalDestinationType = 0 THEN 'Not Transmitted' WHEN FinalDestinationType = 1 THEN 'Print' WHEN FinalDestinationType = 2 THEN 'Fax' WHEN FinalDestinationType = 3 THEN 'Electronic Retail' WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder' WHEN FinalDestinationType = 5 THEN 'Test' END as ItemMisc, \r\n"
		" 'DEA Schedule' as Misc2Desc, DrugList.DEASchedule as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN PatientMedications ON PersonT.ID = PatientMedications.PatientID \r\n"		
		" INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID \r\n"				
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		" LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID \r\n"
		" LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
		" WHERE DELETED = 0 AND Discontinued = 0  \r\n"
		" AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N \r\n";

	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'Prescription' as ItemDescriptionLine, PrescriptionDate as ItemDate, LEFT(EMRDataT.Data,255) as ItemDescription, \r\n"
		" PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ItemProvider, '' as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, 'Destination Type' as MiscDesc, \r\n"
		" CASE WHEN FinalDestinationType = 0 THEN 'Not Transmitted' WHEN FinalDestinationType = 1 THEN 'Print' WHEN FinalDestinationType = 2 THEN 'Fax' WHEN FinalDestinationType = 3 THEN 'Electronic Retail' WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder' WHEN FinalDestinationType = 5 THEN 'Test' END as ItemMisc, \r\n"
		" 'DEA Schedule' as Misc2Desc, DrugList.DEASchedule as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN PatientMedications ON PersonT.ID = PatientMedications.PatientID \r\n"
		" INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID \r\n"				
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n"
		" LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID \r\n"
		" LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
		" WHERE DELETED = 0 AND Discontinued = 0  \r\n"		
		" AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] \r\n"
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] \r\n"
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Prescription(s)/History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter]  "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "		
		") P ";
		;

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}

// (r.farnworth 2013-10-22 15:56) - PLID 59145 - Implement MU.CORE.01.B for Stage 2
void CCHITReportInfoListing::CreateStage2CPOEReport_RadiologyOrders(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2CPOEReport_RadiologyOrders;
	riReport.SetConfigureType(crctMailsentCat);
	riReport.SetPercentToPass(30);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.3.3" : "MU.CORE.01.B");
	riReport.m_strInternalName = "MU2.C01B - Radiology orders which have been entered through CPOE";
	riReport.m_strDisplayName = strMeasurePrefix + " - Radiology orders which have been entered through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many radiology labs have been ordered through CPOE. Radiology orders are labs associated with the type Diagnostics "
		"Filtering is applied to EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of radiology orders in the denominator entered electronically.",
		"The number of radiology orders in the date range and with the selected provider(s) and location and all written radiology orders scanned in, defined by the selected history category in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetNumeratorSql("/*MU.CORE.01.B/MU.CORE.01.B*/\r\n"
		"SELECT Count(LabsT.ID) AS 'NumeratorValue' FROM LabsT "
		"WHERE LabsT.Discontinued = 0 AND LabsT.Deleted = 0 "
		"AND LabsT.Type = 4 "
		"AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		"AND LabsT.PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		);

	////Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetDenominatorSql("/*MU.CORE.01.B/MU.CORE.01.B*/\r\n"
		" SELECT Sum(LabsC) AS 'DenominatorValue' FROM ( "
		" SELECT Count(*) AS 'LabsC' FROM LabsT "
		" WHERE LabsT.Discontinued = 0 AND LabsT.Deleted = 0 AND LabsT.Type = 4 "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND LabsT.PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" UNION ALL "
		" SELECT Count(*) AS 'LabsC' FROM MailSent "
		" WHERE PersonID IN (SELECT PersonID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PatientsT.PersonID "
		" AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter] ) "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "
		" ) Q "
		);


	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First,Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2,  Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate, "
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 AND LabsT.Type = 4 "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate,	"
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 AND LabsT.Type = 4 "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter]  "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "		
		") P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);

}


// (r.farnworth 2013-10-23 11:11) - PLID 59146 - MU.CORE.01.C 
void CCHITReportInfoListing::CreateStage2CPOEReport_LaboratoryOrders(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2CPOEReport_LaboratoryOrders;
	riReport.SetConfigureType(crctMailsentCat);
	riReport.SetPercentToPass(30);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.3.2" : "MU.CORE.01.C");
	riReport.m_strInternalName = "MU2.C01C - Laboratory orders which have been entered through CPOE";
	riReport.m_strDisplayName = strMeasurePrefix + " - Laboratory orders which have been entered through CPOE";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many laboratory labs have been ordered through CPOE. Laboratory orders are those associated with a lab type that is Biopsy, Lab Work, or Cultures."
		"Filtering is applied to EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of laboratory orders in the denominator entered electronically.",
		"The number of laboratory orders in the date range and with the selected provider(s) and location and all written laboratory orders scanned in, defined by the selected history category in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetNumeratorSql("/*MU.CORE.01.C/MU.CORE.01.C*/\r\n"
		"SELECT Count(LabsT.ID) AS 'NumeratorValue' FROM LabsT "
		"WHERE LabsT.Discontinued = 0 AND LabsT.Deleted = 0 "
		"AND LabsT.Type IN (1,2,3) "
		"AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		"AND LabsT.PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		);

	////Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	riReport.SetDenominatorSql("/*MU.CORE.01.C/MU.CORE.01.C*/\r\n"
		" SELECT Sum(LabsC) AS 'DenominatorValue' FROM ( "
		" SELECT Count(*) AS 'LabsC' FROM LabsT "
		" WHERE LabsT.Discontinued = 0 AND LabsT.Deleted = 0 AND LabsT.Type IN (1,2,3) "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND LabsT.PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" UNION ALL "
		" SELECT Count(*) AS 'LabsC' FROM MailSent "
		" WHERE PersonID IN (SELECT PersonID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PatientsT.PersonID "
		" AND CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter] ) "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "
		" ) Q "
		);


	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First,Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2,  Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate, "
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 AND LabsT.Type IN (1,2,3) "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.BiopsyDate as ItemDate,	"
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 AND LabsT.Type IN (1,2,3) "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter]  "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "		
		") P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);

}

// (r.farnworth 2013-10-23 16:17) - PLID 59158 - MU.CORE.02
void CCHITReportInfoListing::CreateStage2EPrescriptionReport(bool bUseModifiedStage2)
{
CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2EPrescription;
	riReport.SetPercentToPass(50);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.4" : "MU.CORE.02");
	riReport.m_strInternalName = "MU2.C02 - Prescriptions sent electronically.";
	riReport.m_strDisplayName = strMeasurePrefix + " - Prescriptions sent electronically.";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many active permissible prescriptions have been sent electronically through NewCrop or NexERx. Filtering is applied on the prescription date, prescription location, and the prescription provider. ",
		"The number of all active prescriptions in the denominator that were prescribed electronically through NewCrop or NexERx AND queried for a drug formulary.",
		"The number of all active prescriptions written for drugs requiring a prescription in order to be dispensed other than controlled substances during the reporting period that have the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.farnworth 2014-01-10 14:13) - PLID 60268 - Need to check for the new column NewCropFormularyChecked when checking NewCrop prescriptions
	// (j.jones 2016-02-05 10:54) - PLID 67981 - added DispensedInHouse as a valid status,
	// and switched the hardcoded statuses to use the enums
	riReport.SetNumeratorSql("/*MU.08/MU.CORE.02*/\r\n"
		"SELECT COUNT(PatientMedications.ID) AS NumeratorValue FROM PatientMedications  "
		" LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"WHERE DELETED = 0 AND Discontinued = 0 "
		"AND ( (NewCropGUID IS NOT NULL AND FinalDestinationType IN (2,3,4) AND NewCropFormularyChecked = 1) "
		"	OR (NewCropGUID IS NULL AND QueueStatus IN (" +
			FormatString("%li, %li, %li, %li", (long)PrescriptionQueueStatus::pqseTransmitSuccess, (long)PrescriptionQueueStatus::pqseTransmitPending, (long)PrescriptionQueueStatus::pqseFaxed, (long)PrescriptionQueueStatus::pqseDispensedInHouse)
		+ ") AND SureScriptsEligibilityDetailID IS NOT NULL ) ) "
		"AND RTRIM(LTRIM(DEASchedule)) = '' "
		"AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 [PatientIDFilter]) "
		);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDenominatorSql("/*MU.08/MU.CORE.02*/\r\n"
		"SELECT COUNT(PatientMedications.ID) AS DenominatorValue FROM PatientMedications  "
		" LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
		"WHERE DELETED = 0 AND Discontinued = 0 AND QueueStatus <> 14 "
		"AND RTRIM(LTRIM(DEASchedule)) = '' "
		"AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] "
		"AND PatientMedications.PatientID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 and PersonID > 0 [PatientIDFilter]) "
		);
	
	// (j.gruber 2011-11-10 09:46) - PLID 46502 - reports
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (j.jones 2016-02-05 10:54) - PLID 67981 - added DispensedInHouse as a valid status,
	// and switched the hardcoded statuses to use the enums
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Prescription' as ItemDescriptionLine, PrescriptionDate as ItemDate, LEFT(EMRDataT.Data,255) as ItemDescription, "
		" PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Destination Type' as MiscDesc, "
		" CASE WHEN FinalDestinationType = 0 THEN 'Not Transmitted' WHEN FinalDestinationType = 1 THEN 'Print' WHEN FinalDestinationType = 2 THEN 'Fax' WHEN FinalDestinationType = 3 THEN 'Electronic Retail' WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder' WHEN FinalDestinationType = 5 THEN 'Test' END as ItemMisc, "
		" 'DEA Schedule' as Misc2Desc, DrugList.DEASchedule as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN PatientMedications ON PersonT.ID = PatientMedications.PatientID "		
		" INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "				
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
		" LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID "
		" LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
		" WHERE DELETED = 0 AND Discontinued = 0  "
		" AND ( (NewCropGUID IS NOT NULL AND FinalDestinationType IN (2,3,4) AND NewCropFormularyChecked = 1) "
		"	OR (NewCropGUID IS NULL AND QueueStatus IN (" +
			FormatString("%li, %li, %li, %li", (long)PrescriptionQueueStatus::pqseTransmitSuccess, (long)PrescriptionQueueStatus::pqseTransmitPending, (long)PrescriptionQueueStatus::pqseFaxed, (long)PrescriptionQueueStatus::pqseDispensedInHouse)
		+ ") AND SureScriptsEligibilityDetailID IS NOT NULL ) ) "
		" AND RTRIM(LTRIM(DEASchedule)) = '' "
		" AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N ";


	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Prescription' as ItemDescriptionLine, PrescriptionDate as ItemDate, LEFT(EMRDataT.Data,255) as ItemDescription, "
		" PersonProv.Last + ', ' + PersonProv.First + ' ' + PersonProv.Middle AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'Destination Type' as MiscDesc, "
		" CASE WHEN FinalDestinationType = 0 THEN 'Not Transmitted' WHEN FinalDestinationType = 1 THEN 'Print' WHEN FinalDestinationType = 2 THEN 'Fax' WHEN FinalDestinationType = 3 THEN 'Electronic Retail' WHEN FinalDestinationType = 4 THEN 'Electronic MailOrder' WHEN FinalDestinationType = 5 THEN 'Test' END as ItemMisc, "
		" 'DEA Schedule' as Misc2Desc, DrugList.DEASchedule as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN PatientMedications ON PersonT.ID = PatientMedications.PatientID "
		" INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "				
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
		" LEFT JOIN LocationsT ON PatientMedications.LocationID = LocationsT.ID "
		" LEFT JOIN PersonT PersonProv ON PatientMedications.ProviderID = PersonProv.ID "
		" WHERE DELETED = 0 AND Discontinued = 0 AND QueueStatus <> 14 "		
		" AND PrescriptionDate >= @DateFrom AND PrescriptionDate < @DateTo [ProvPrescripFilter] [LocPrescripFilter] "
		" AND RTRIM(LTRIM(DEASchedule)) = '' "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";
	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}

// (r.farnworth 2013-10-24 16:11) - PLID 59170 - MU.CORE.17
void CCHITReportInfoListing::CreateStage2ElectronicMessagingReport(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.SetPercentToPass(5);
	riReport.m_nInternalID = (long)eMUStage2ElectronicMessaging;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.9" : "MU.CORE.17");
	riReport.m_strInternalName = "MU2.C17 - Patients who have sent secure electronic messages to the doctor through NexWeb";	
	riReport.m_strDisplayName = strMeasurePrefix + " - Patients who have sent secure electronic messages";	
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have sent secure electronic messages to the doctor through NexWeb. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator where the patient has sent at least one secure electronic message to the doctor through NexWeb.",
		"The number of unique patients with a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	CString strSql;
	strSql.Format("/*MU2.CORE.17/MU.CORE.17*/\r\n"
		"SELECT COUNT(DISTINCT PersonT.ID) AS NumeratorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"INNER JOIN Notes on Notes.PersonID = PersonT.ID "
		"WHERE PersonT.ID IN ( "
		"	SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo "
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		"AND Notes.IsPatientCreated = 1 AND Notes.Date >= @DateFrom AND Notes.Date < @DateTo "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		);
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDenominatorSql("/*MU2.CORE.17/MU.CORE.17*/\r\n"
		"SELECT COUNT(DISTINCT ID) AS DenominatorValue FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		"WHERE PersonT.ID IN (SELECT PatientID FROM EMRMasterT "
		"	WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		);


	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "	
		" INNER JOIN Notes on Notes.PersonID = PersonT.ID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND Notes.IsPatientCreated = 1 AND Notes.Date >= @DateFrom AND Notes.Date < @DateTo "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ) N");

	strDenom.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] ) P");

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}

// (r.farnworth 2013-10-29 11:36) - PLID 59217 - Implement Stage 2 MU.MENU.02
//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 Update query to call Common code
void CCHITReportInfoListing::CreateStage2ElectronicNotesReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.SetPercentToPass(30);
	riReport.m_nInternalID = (long)eMUStage2ElectronicNotes;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.M02 - Patients who have at least one electronic progress note as text searchable data";	
	riReport.m_strDisplayName = "MU.MENU.02 - Patients who have at least one electronic progress note as text searchable data";
	CString strCodes = GetRemotePropertyText("CCHIT_MU.13_CODES_v3", MU_13_DEFAULT_CODES, 0, "<None>", true);
	//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
	//  Return all emns who have provider filter  and  visit codes  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
	CString strCommonDenominatorSQL = GetClinicalSummaryCommonDenominatorSQL(strCodes);

	CString strCodeDesc;
	strCodeDesc.Format("with one of the following service codes or a charge on the same date as the EMN with a service code in %s", strCodes); 
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-23 07:17) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions 
	// (s.dhole 2014-05-20 10:17) - PLID 62214 - MU.MENU.02 sumary report should ignore void charges
	// (d.singleton 2014-09-17 14:53) - PLID 63453 - MU2 Menu 2 (Progress Notes) - Bills not filtering on Provider - summary
	riReport.SetHelpText("This report calculates how many patients have at least one electronic progress note as text searchable data. "
		"Filtering is on EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of unique patients in the denominator who have at least one signed electronic progress note as text searchable data.",
		"The number of unique patients with a non-excluded EMN " + strCodeDesc + " within the date range and with the selected providers(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (d.singleton 2014-09-17 14:53) - PLID 63453 - MU2 Menu 2 (Progress Notes) - Bills not filtering on Provider - summary
	CString strSql;
	strSql.Format(R"(/*MU2.MENU.02/MU.MENU.02*/
		 SELECT COUNT(DISTINCT PersonT.ID) AS NumeratorValue FROM PersonT 
		 INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		 INNER JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID 
		-- INNER JOIN EMRTopicsT ON EMRTopicsT.EMRID = EMRMasterT.ID 
		 WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] 
		 AND EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
		-- AND EMRTopicsT.Deleted = 0 AND (EMRTopicsT.ShowIfEmpty = 1 OR EXISTS (SELECT * FROM EMRDetailsT WHERE EMRDetailsT.EMRTopicID = EMRTopicsT.ID)) 
		 AND PatientsT.PersonID IN 
		 ( 
			SELECT EMRMasterT.PatientID FROM EMRMasterT INNER JOIN EMRTopicsT ON EMRTopicsT.EMRID = EMRMasterT.ID 
			WHERE EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
			AND (EMRTopicsT.ShowIfEmpty = 1 OR EXISTS (SELECT * FROM EMRDetailsT WHERE EMRDetailsT.EMRTopicID = EMRTopicsT.ID)) 
		 ) 
		 AND PatientsT.PersonID IN 
		 ( SELECT  EMRMasterT.PatientID  FROM  EMRMasterT 
		 INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
		 AND EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] ) )"
		, strCommonDenominatorSQL);

	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (d.singleton 2014-09-17 14:53) - PLID 63453 - MU2 Menu 2 (Progress Notes) - Bills not filtering on Provider - summary
	CString strSqlD;
	strSqlD.Format(R"(/*MU2.MENU.02/MU.MNU.02*/
		
		 SELECT COUNT(PatientsT.PersonID) AS DenominatorValue FROM PatientsT 
		 WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] 
		 AND PatientsT.PersonID IN 
		 (
			SELECT  EMRMasterT.PatientID  FROM  EMRMasterT 
			INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
			AND EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] 
		 ) 
)"
		, strCommonDenominatorSQL);
	riReport.SetDenominatorSql(strSqlD);

	//report
	CString strSelect, strNum, strDenom;

	// (r.farnworth 2014-04-29 15:01) - PLID 61820 - Electronic Notes Stage 2 Menu 2 crystal report should only display the qualifying EMNs 1 time no matter how many signatures are inserted on that EMN.
	// (d.singleton 2014-09-17 14:53) - PLID 63453 - MU2 Menu 2 (Progress Notes) - Bills not filtering on Provider - summary
	strSelect = " SELECT DISTINCT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

		strNum.Format(R"( FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, 
		 PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, 
		 EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, 
		 dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, 
		 dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, 
		 LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  
		 FROM PersonT 
		 INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		 INNER JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID 
		 INNER JOIN EMRTopicsT ON EMRTopicsT.EMRID = EMRMasterT.ID 
		 LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID 
		 WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] 
		 AND EMRMasterT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] 
		 AND EMRTopicsT.Deleted = 0 AND (EMRTopicsT.ShowIfEmpty = 1 OR EXISTS (SELECT * FROM EMRDetailsT WHERE EMRDetailsT.EMRTopicID = EMRTopicsT.ID)) 
		 AND PatientsT.PersonID IN 
		 (SELECT EMRMasterT.PatientID FROM EMRMasterT 
			INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
		 WHERE EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] AND EMRMasterT.Deleted = 0) 
		 ) N )"
		, strCommonDenominatorSQL);

		strDenom.Format(R"( FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, 
		 PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, 
		 PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, 
		 EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, 
		 dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, 
		 dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, 
		 LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, 
		 '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  
		 FROM PersonT 
		 INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID 
		 INNER JOIN EMRMasterT ON PatientsT.PersonID = EMRMasterT.PatientID 
		 INNER JOIN %s ON EMRMasterT.PatientID = ChargeFilterQ.PatientID AND ChargeFilterQ.Date = EmrMasterT.Date 
		 LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID 
		 WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] 
		 AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] AND EMRMasterT.Deleted = 0 ) P )"
		, strCommonDenominatorSQL);

		// (r.farnworth 2013-11-12 16:20) - PLID 59217 - Numerator will be the Denominator here
		riReport.SetReportInfo(strSelect, strNum, strDenom);

		m_aryReports.Add(riReport);
}

// (r.farnworth 2013-10-30 13:55) - PLID 59231 - Implement Stage 2 MU.MENU.03
void CCHITReportInfoListing::CreateStage2ImagingResultsReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2ImagingResults;
	riReport.SetConfigureType(crctMailsentCat);
	riReport.SetPercentToPass(10);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.M03 - Number of test whose result is one or more images that were received";
	riReport.m_strDisplayName = "MU.MENU.03 - Number of test whose result is one or more images that were received";
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many tests have a result that is one or more images and is accessible through Practice. "
		"Filtering is applied to EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of Diagnostic labs in the denominator whose result contains one or more images accessible through the lab.",
		"The number of Diagnostics labs with the selected provider(s) and location and the number of images scanned in, based on the configured category.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	CString strSql;
	strSql.Format("/*MU2.MENU.03/MU.MENU.03*/\r\n"
		"SELECT SUM(NumeratorValue) AS 'NumeratorValue' FROM ( "
		"SELECT Count(LabsT.ID) AS 'NumeratorValue' FROM LabsT "
		"INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
		"INNER JOIN MailSent on LabResultsT.MailID = MailSent.MailID "
		"WHERE LabsT.Discontinued = 0 AND LabsT.Deleted = 0 "
		"AND LabsT.Type = 4 "
		"AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		"AND LabsT.PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" ) Q "
		);
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strSql.Format("/*MU2.MENU.03/MU.MENU.03*/\r\n"
		"SELECT SUM(DenominatorValue) AS 'DenominatorValue' FROM ( "
		"SELECT Count(LabsT.ID) AS 'DenominatorValue' FROM LabsT "
		"WHERE LabsT.Discontinued = 0 AND LabsT.Deleted = 0 "
		"AND LabsT.Type = 4 "
		"AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		"AND LabsT.PatientID IN (SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		"UNION ALL "
		"SELECT COUNT(*) AS 'DenominatorValue' FROM MailSent WHERE "
		"PersonID IN (SELECT PersonID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PatientsT.PersonID "
		"WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter] ) "
		"AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		"AND CategoryID = @MailSentCatID "
		" ) Q "
		);
	riReport.SetDenominatorSql(strSql);

	//report
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strNum = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.InputDate as ItemDate, "
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" INNER JOIN LabResultsT ON LabsT.ID = LabResultsT.LabID "
		" INNER JOIN MailSent on LabResultsT.MailID = MailSent.MailID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 AND LabsT.Type = 4 "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		") N ";

	// (r.farnworth 2013-12-17 14:18) - PLID 59231 - Filtering on the same date as the rest of the report
	// (r.farnworth 2014-02-27 09:36) - PLID 61060 - Meaningful Use Lab Reports were not properly filtering out Discontinued Labs
	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Lab(s)/History Document' as ItemDescriptionLine, LabsT.InputDate as ItemDate, "
		"	CASE WHEN LabsT.Specimen IS NULL THEN"
				"		LabsT.FormNumberTextID"
				"	ELSE"
				"		LabsT.FormNumberTextID + ' - ' + LabsT.Specimen"
				"	END as ItemDescription, "
		" dbo.GetLabProviderString(LabsT.ID) AS ItemProvider, '' as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, NULL as ItemMisc, '' as Misc2Desc, NULL as ItemMisc2, "
		" '' as Misc3Desc, NULL as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN LabsT ON PersonT.ID = LabsT.PatientID "
		" LEFT JOIN LocationsT ON LabsT.LocationID = LocationsT.ID "
		" WHERE LabsT.DELETED = 0 AND LabsT.Discontinued = 0 AND LabsT.Type = 4 "
		" AND LabsT.BiopsyDate >= @DateFrom AND LabsT.BiopsyDate < @DateTo [LabProvFilter] [LabLocFilter] "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]"
		" UNION ALL "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Prescription(s)/History Document' as ItemDescriptionLine, Mailsent.Date as ItemDate, MailsentNotesT.Note as ItemDescription, "
		" '' AS ItemProvider, '' as ItemSecProvider, '' as ItemLocation, 'Category' as MiscDesc, NoteCatsF.Description as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "		
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN Mailsent ON PersonT.ID = MailSent.PersonID "
		" LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
		" LEFT JOIN NoteCatsF ON Mailsent.CategoryID = NoteCatsF.ID "
		" WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] [PatientProvFilter] [PatientLocFilter]  "
		" AND ServiceDate >= @DateFrom AND ServiceDate < @DateTo "
		" AND CategoryID = @MailSentCatID "	
		") P ";

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);

}

// (r.farnworth 2013-10-30 15:16) - PLID 59236 - Implement MU.CORE.14 for Stage 2
void CCHITReportInfoListing::CreateStage2MedicalReconciliationReport(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2MedicalReconciliation;
	riReport.SetPercentToPass(50);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.7" : "MU.CORE.14");
	riReport.m_strInternalName = "MU2.C14 - Referrals that have medication reconciliation performed.";
	riReport.m_strDisplayName = strMeasurePrefix + " - Referrals that have medication reconciliation performed.";
	riReport.SetConfigureType(crctEMRMultiItems);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	// (r.farnworth 2014-05-14 07:44) - PLID 62134 - Change the numerator text in MUS2 Core 8, MUS2 Core 13, MUS2 Core 14
	// (s.dhole 2014-05-15 07:44) - PLID 62136 - Change the numerator to add Medication reconciliation data
	riReport.SetHelpText("This report calculates how many referrals of care state that medication reconciliation was performed. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of non-excluded EMNs in the denominator who have a medication reconciliation performed, defined by the selected bottom EMR item in the configuration for this report. A medication reconciliation can be substituted for the bottom EMR item.",
		"The number of non-excluded EMNs within the date range and with the selected provider(s) and location that state the patient is a referral, defined by the selected top EMR item in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	CString strSql;
	strSql.Format("/*MU.15/MU.CORE.14*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 "
		"[ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND (EMRMasterT.PatientID IN  "
		"        ("
		"		SELECT PatientID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "		
		"	  ) "		
		"       OR (EMRMasterT.PatientID  IN (SELECT PatientID FROM PatientReconciliationT WHERE ReconciliationDate>= @DateFrom AND ReconciliationDate<@DateTo )) "
		"    ) "
		" ) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ",  eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	strSql.Format("/*MU.15/MU.CORE.14*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 "
		"[ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ",  eitSingleList, eitMultiList, eitTable);
	riReport.SetDenominatorSql(strSql);

	// report
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Referral' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE 	"	
		" EMRMasterT.DELETED = 0 "
		" [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND (EMRMasterT.PatientID IN (	"
		"		SELECT PatientID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "		
		"	) "		
		"    OR ( EMRMasterT.PatientID  IN(SELECT PatientID FROM PatientReconciliationT WHERE ReconciliationDate >= @DateFrom AND ReconciliationDate<@DateTo)) "
		"    ) "
		") "	
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N  ",  eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
		

	strDenom.Format( " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'Referral' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		" [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0 AND Date >= @DateFrom AND Date < @DateTo "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ",  eitSingleList, eitMultiList, eitTable);

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);

}

// (r.farnworth 2013-11-13 11:27) - PLID 59455 - Implement MU.CORE.07.A for Stage 2
void CCHITReportInfoListing::CreateStage2ElectronicAccessReport(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.SetPercentToPass(50);
	riReport.m_nInternalID = (long)eMUStage2ElectronicAccess;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.8.1" : "MU.CORE.07.A");
	riReport.m_strInternalName = "MU2.C07A - Patients provided with timely Electronic Access ";	
	riReport.m_strDisplayName = strMeasurePrefix + " - Patients provided with timely Electronic Access ";	
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	// (s.dhole 2014-05-19 14:22) - PLID 62187 - Change the numerator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have been provided electronic access in at least four business days. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s). A business day is defined as Monday - Friday.", 
		"The number of unique patients in the denominator who have a merged clinical summary or summary of care plus a security code or a NexWeb patient login created on or before 4 business days of the EMN date.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	CString strSql;
	strSql.Format("/*MU.06/MU.CORE.07.A*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM PersonT "
		"WHERE PersonT.ID IN (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter])"
		"AND PersonT.ID IN "
		"	(SELECT PatientID FROM ( "
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the InitialSecurityCodeCreationDate since SecurityCodeCreationDate is cleared out when the patient creates a username with the security code.
		"	SELECT EMRMasterT.PatientID, "
		"	DATEDIFF(d,Date, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) "
		"	- DATEDIFF(wk,Date, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) * 2  "
		"	 - CASE  "
		"	WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) = 'Saturday' THEN 1  "
		"	WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, "
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL "
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) "
		"		ELSE " 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) "
		"		END) <> 'Saturday' THEN -1  "
		"	ELSE 0 "
		"	END AS BusinessDays, "
		"  "
		// (r.farnworth 2014-05-19 10:37) - PLID 62189 - Patients Electronic Access Stage 2 Core 7.A needs to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"	DATEDIFF(d,Date, MergeDocsT.MergeDate) "
		"	- DATEDIFF(wk,Date, MergeDocsT.MergeDate) * 2  "
		"	 - CASE  "
		"	WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) = 'Saturday' THEN 1  "
		"	WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) <> 'Saturday' THEN -1  "
		"	ELSE 0 "
		"	END AS MergeDays "
		"  "
		"	FROM EMRMasterT LEFT JOIN  "
		"	(SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT "
		"	ON EMRMasterT.PatientID = NexwebLoginT.PersonID "
		// (r.farnworth 2014-05-06 11:35) - PLID 61910 - Fix Meaningful Use Detailed and Summary Reporting measure for MU.MENU.05 for Stage 1
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the first date a security code was ever created for the patient rather than the current security code creation date.
		// This ensures we are using the date when the patient first had access to the portal since SecurityCode and SecurityCodeCreationDate are nulled out when the patient creates a login from the security code.
		"	LEFT JOIN "
		"	( SELECT InitialSecurityCodeCreationDate, PersonID FROM PatientsT WHERE CurrentStatus <> 4 "
		"		AND PersonID > 0 AND InitialSecurityCodeCreationDate IS NOT NULL "
		"	) SecurityCodeT ON EMRMasterT.PatientID = SecurityCodeT.PersonID  "
		// (r.farnworth 2014-05-19 10:37) - PLID 62189 - Patients Electronic Access Stage 2 Core 7.A needs to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		"	INNER JOIN "
		"	( SELECT Min(Date) as MergeDate, PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField IN (1,2) "
		"		GROUP BY PersonID ) MergeDocsT ON EMRMasterT.PatientID = MergeDocsT.PersonID "
		"	WHERE EMRMasterT.Deleted = 0 "
		"		AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	) Q WHERE BusinessDays <= 4 AND MergeDays <= 4 "
		"	GROUP BY PatientID) "
		);
		
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDefaultDenominatorSql();				

	// report
	CString strSelect, strDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	strNum = "FROM ( "
		" SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, \r\n"
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, 'First Nexweb Login Date' as MiscDesc, CONVERT(nVarChar, dbo.AsDateNoTime(MinDate), 101) as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the InitialSecurityCodeCreationDate since SecurityCodeCreationDate is cleared out when the patient creates a username with the security code.
		" LEFT JOIN ( SELECT EMRMasterT.*, \r\n"
		" DATEDIFF(d,Date, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) \r\n"
		" - DATEDIFF(wk,Date, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) * 2  \r\n"
		" - CASE  \r\n"
		" WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) = 'Saturday' THEN 1  \r\n"
		" WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, \r\n"
		"		CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL AND NexWebLoginT.MinDate IS NOT NULL \r\n"
		"			THEN (CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate < NexWebLoginT.MinDate THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END) \r\n"
		"		ELSE \r\n" 
		"			ISNULL(SecurityCodeT.InitialSecurityCodeCreationDate, NexWebLoginT.MinDate) \r\n"
		"		END) <> 'Saturday' THEN -1  \r\n"
		" ELSE 0 \r\n"
		" END AS BusinessDays, CASE WHEN SecurityCodeT.InitialSecurityCodeCreationDate IS NOT NULL THEN SecurityCodeT.InitialSecurityCodeCreationDate ELSE NexWebLoginT.MinDate END AS MinDate, \r\n"
		"  \r\n"
		// (r.farnworth 2014-05-19 10:39) - PLID 62189 - Patients Electronic Access Stage 2 Core 7.A needs to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		" DATEDIFF(d,Date, MergeDocsT.MergeDate) \r\n"
		" - DATEDIFF(wk,Date,  MergeDocsT.MergeDate) * 2  \r\n"
		" - CASE  \r\n"
		" WHEN DATENAME(dw, Date) <> 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) = 'Saturday' THEN 1  \r\n"
		" WHEN DATENAME(dw, Date) = 'Saturday' AND DATENAME(dw, MergeDocsT.MergeDate) <> 'Saturday' THEN -1  \r\n"
		" ELSE 0 \r\n"
		" END AS MergeDays "
		"  \r\n"
		" FROM EMRMasterT LEFT JOIN  \r\n"
		" (SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT \r\n"
		" ON EMRMasterT.PatientID = NexwebLoginT.PersonID \r\n"
		// (r.gonet 2014-12-31 10:13) - PLID 64498 - Use the first date a security code was ever created for the patient rather than the current security code creation date.
		// This ensures we are using the date when the patient first had access to the portal since SecurityCode and SecurityCodeCreationDate are nulled out when the patient creates a login from the security code.
		" LEFT JOIN "
		" (SELECT InitialSecurityCodeCreationDate, PersonID FROM PatientsT WHERE CurrentStatus <> 4 "
		" AND PersonID > 0 AND InitialSecurityCodeCreationDate IS NOT NULL) SecurityCodeT  "
		" ON EMRMasterT.PatientID = SecurityCodeT.PersonID  "
		// (r.farnworth 2014-05-19 10:37) - PLID 62189 - Patients Electronic Access Stage 2 Core 7.A needs to check if either a summary of care or clinical sumamry was merged within 4 business days of the EMN.
		" INNER JOIN "
		" ( SELECT Min(Date) as MergeDate, PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField IN (1,2) "
		"	 GROUP BY PersonID ) MergeDocsT ON EMRMasterT.PatientID = MergeDocsT.PersonID "
		" WHERE EMRMasterT.Deleted = 0 \r\n"
		"	AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"
		" ) EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0 AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] AND BusinessDays <= 4 AND MergeDays <= 4"
		" ) N \r\n";

	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'First NexWeb Login Date' as MiscDesc, dbo.AsDateNoTime(MinDate) as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" LEFT JOIN  "
		" (SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT "
		" ON EMRMasterT.PatientID = NexwebLoginT.PersonID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";


	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}

// (r.farnworth 2013-11-14 11:24) - PLID 59489 - Implement MU.CORE.15.A for Stage 2
void CCHITReportInfoListing::CreateStage2SummaryOfCareProvidedReport()
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUDischargeSummary;
	riReport.SetPercentToPass(50);
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	riReport.m_strInternalName = "MU2.C15A - Transitions of Care are provided summary of care.";
	riReport.m_strDisplayName = "MU.CORE.15.A - Transitions of Care are provided summary of care.";
	riReport.SetConfigureType(crctEMRMultiItems);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions
	riReport.SetHelpText("This report calculates how many transitions of care state that a summary of care was provided. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of EMNs in the denominator that have a summary of care provided, defined by the selected bottom EMR item on the configuration screen for this report. The bottom item can be substituted for a transition of care file generated through the EMN.",
		"The number of non-excluded EMNs within the date range and with the selected provider(s) and location that state the patient is transitioning, defined by the top EMR item in the configuration of this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	CString strSql;
	strSql.Format("/*MU.16/MU.CORE.15.A*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"   AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND  ( "
		//(s.dhole 8/1/2014 4:27 PM ) - PLID 63068 
		"		EMRMasterT.ID IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "
		"	) "
		"	OR "
		"		EMRMasterT.PatientID IN ( SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField = 1) "
		"	) "
		" ) "
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	strSql.Format("/*MU.16/MU.CORE.15.A*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "		
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ",  eitSingleList, eitMultiList, eitTable);
	riReport.SetDenominatorSql(strSql);

	//report
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Transition' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE 	"
		" EMRMasterT.DELETED = 0 "
		"	AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND ( "
		//(s.dhole 8/1/2014 4:36 PM ) - PLID 63068
		"		EMRMasterT.ID IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"			) "
		"		) "
		"	) "
		"	OR "
		"		EMRMasterT.PatientID IN ( SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND CCDATypeField = 1) "
		"	) "
		") "
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N  ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
		
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom.Format( " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'Transition' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		"   AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ",  eitSingleList, eitMultiList, eitTable);

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
}


// (r.farnworth 2013-11-14 14:31) - PLID 59490 - Implement MU.CORE.15.B for Stage 2
void CCHITReportInfoListing::CreateStage2SummaryOfCareTransmittedReport(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.m_nInternalID = (long)eMUStage2SummaryOfCareTransmitted;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.5" : "MU.CORE.15.B");
	riReport.m_strInternalName = "MU2.C15B - Transitions of Care have transmitted summary of care.";
	riReport.m_strDisplayName = strMeasurePrefix + " -  Transitions of Care have transmitted summary of care.";
	riReport.SetConfigureType(crctEMRMultiItems);
	riReport.SetPercentToPass(10);
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 07:36) - PLID 61819 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 reports to new descriptions	
	riReport.SetHelpText("This report calculates how many transitions of care state that a summary of care was transmitted. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s).", 
		"The number of EMNs in the denominator that have a summary of care transmitted, defined by the selected bottom EMR item on the configuration screen for this report. The bottom item can be substituted for a transition of care file generated through the EMN.",
		"The number of non-excluded EMNs within the date range and with the selected provider(s) and location that state the patient is transitioned, defined by the selected top EMR item in the configuration for this report.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	CString strSql;
	strSql.Format("/*MU.15.B/MU.CORE.15.B*/\r\n"
		"SELECT COUNT(ID) AS NumeratorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"   AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND  ( "
		//(s.dhole 7/25/2014 4:55 PM ) - PLID 63068
		"		EMRMasterT.ID IN (	"
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			OR  "
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			) "
		"		) "
		"		OR "
		"			EMRMasterT.PatientID IN ( SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND MailSent.MailID IN (SELECT MailSentID From DirectAttachmentT)) "
		"	) "
		" ) "
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
	riReport.SetNumeratorSql(strSql);


	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	strSql.Format("/*MU.15.B/MU.CORE.15.B*/\r\n"
		"SELECT COUNT(ID) AS DenominatorValue FROM EMRMasterT "
		"WHERE EMRMasterT.Deleted = 0 		"
		"    AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "		
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "		
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		" AND PatientID IN (SELECT PersonID FROM PatientsT WHERE PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) ",  eitSingleList, eitMultiList, eitTable);
	riReport.SetDenominatorSql(strSql);

	//report
	CString strSelect, strNum, strDenom;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strNum.Format(" FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  "
		" PersonT.HomePhone, PersonT.WorkPhone, "
		" 'Transition' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE 	"
		" EMRMasterT.DELETED = 0 "
		"	AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter] "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "
		"	 AND ( "
		"		EMRMasterT.ID IN (	"
		//(s.dhole 8/1/2014 4:37 PM ) - PLID 63068
		"		SELECT EMRMasterT.ID FROM EMRMasterT "
		"		INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"				(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"					SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"					INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			OR  "
		"				(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"					SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"					INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"					INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"					INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"					WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID2)  "
		"				) "
		"			) "
		"		) "
		"		OR "
		"			EMRMasterT.PatientID IN ( SELECT PersonID FROM MailSent WHERE Selection = 'BITMAP:CCDA' AND MailSent.MailID IN (SELECT MailSentID From DirectAttachmentT)) "
		"	) "
		") "
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		"AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) N  ", eitSingleList, eitMultiList, eitTable, eitSingleList, eitMultiList, eitTable);
		
	//(e.lally 2012-02-24) PLID 48268 - Added PatientIDFilter
	strDenom.Format( " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'Transition' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" WHERE EMRMasterT.DELETED = 0  "
		"   AND Date >= @DateFrom AND Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		"	 AND EMRMasterT.ID IN (	"
		"		SELECT EMRDetailsT.EMRID FROM EMRDetailsT "
		"		INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID  "
		"		WHERE EMRDetailsT.Deleted = 0  "
		"		AND ( "
		"			(EMRInfoT.DataType IN (%li, %li) AND EMRDetailsT.ID IN ( "
		"				SELECT EMRSelectT.EMRDetailID FROM EMRSelectT  "
		"				INNER JOIN EMRDataT ON EMRSelectT.EMRDataID = EMRDataT.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID					 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		OR  "
		"			(EMRInfoT.DataType = %li AND EMRDetailsT.ID IN ( "
		"				SELECT EMRDetailTableDataT.EMRDetailID FROM EMRDetailTableDataT  "
		"				INNER JOIN EMRDataT EMRDataT_X ON EMRDetailTableDataT.EMRDataID_X = EMRDataT_X.ID  "
		"				INNER JOIN EMRDataT EMRDataT_Y ON EMRDetailTableDataT.EMRDataID_Y = EMRDataT_Y.ID  "
		"				INNER JOIN EMRInfoT ON EMRDataT_X.EMRInfoID = EMRInfoT.ID 				 "
		"				WHERE EMRInfoT.EMRInfoMasterID = @EMRMasterID)  "
		"			) "
		"		) "		
		" ) "
		//" AND PatientID IN (SELECT PatientID FROM EMRMasterT WHERE Date >= @DateFrom AND Date < @DateTo AND Deleted = 0) "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ",  eitSingleList, eitMultiList, eitTable);

	riReport.SetReportInfo(strSelect, strNum, strDenom);

	m_aryReports.Add(riReport);
	
}


//(r.farnworth 2013-11-14 16:23) - PLID 59501 - Implement MU.CORE.07.B for Stage 2
void CCHITReportInfoListing::CreateStage2ElectronicInformationViewedReport(bool bUseModifiedStage2)
{
	CCCHITReportInfo riReport;
	riReport.SetReportType(crtMU);
	riReport.SetPercentToPass(5);
	riReport.m_nInternalID = (long)eMUStage2ElectronicInformationViewed;
	// (r.farnworth 2014-01-27 16:36) - PLID 60221 - Stage 1 and Stage 2 Reporting need to configure seperately.
	CString strMeasurePrefix = (bUseModifiedStage2 ? "Measure.8.2" : "MU.CORE.07.B");
	riReport.m_strInternalName = "MU2.C07B - Patients viewed online, downloaded, or transmitted to a third party the patient\''s health information.";	
	riReport.m_strDisplayName = strMeasurePrefix + " - Patients  viewed online, downloaded, or transmitted to a third party the patient\''s health information. ";	
	//General:  This should describe the report and explain how date filters apply.
	//Numerator:  This should describe how the numerator is calculated, including what filters are applied.
	//Denominator:  This should describe how the denominator is calculated, including what filters are applied.
	// (b.savon 2014-04-22 08:55) - PLID 61818 - Change the numerator and denominator descriptions of the indicated Meaningful Use Stage 2 Core reports to new descriptions
	riReport.SetHelpText("This report calculates how many patients have viewed online, downloaded, or transmitted to a third party their health information. "
		"Filtering is applied to the EMN Date, EMN Location, and EMN Primary or Secondary Provider(s). A business day is defined as Monday - Friday.", 
		"The number of unique patients in the denominator who have logged into NexWeb and viewed, downloaded, or transmitted to a third party their health information.",
		"The number of unique patients who have a non-excluded EMN within the date range and with the selected provider(s) and location.");

	//Must call the value 'NumeratorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.	
	// (r.gonet 2015-02-18 13:04) - PLID 64437 - We now join the NexWebCcdaAccessHistoryT onto the MailSent record and from there get the PersonID
	// rather than joining NexWebCcdaAccessHistoryT to the NexWebLoginInfoT on the username column. This is because we now keep access history 
	// instead of deleting it when a NexWeb username is deleted. Removed Check for an enabled NexWebLoginInfoT record since if they have a NexWebCcdaAccessHistoryT
	// record, then they had access to NexWeb at some point, even if that login is now disabled or deleted.
	CString strSql;
	strSql.Format("/*MU.07.B/MU.CORE.07.B*/\r\n"
		" SELECT COUNT(ID) AS NumeratorValue FROM PersonT "	
		" WHERE PersonT.ID IN  (SELECT PersonID FROM PatientsT WHERE CurrentStatus <> 4 AND PersonID > 0 [PatientIDFilter]) "
		" AND PersonT.ID IN  "
		" ( "
		" 	SELECT EMRMasterT.PatientID "	
		"	FROM EMRMasterT  "
		"	WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo "	
		"	[ProvFilter] [LocFilter] [ExclusionsFilter] "
		" ) "
		" AND PersonT.ID IN "
		" ( "
		" 	SELECT MailSent.PersonID FROM MailSent "
		" 	INNER JOIN NexWebCcdaAccessHistoryT ON NexWebCcdaAccessHistoryT.MailSentMailID = MailSent.MailID "
		" 	WHERE NexWebCcdaAccessHistoryT.AccessType IN (-1, -2, -3) "
		" ) "
		);
		
	riReport.SetNumeratorSql(strSql);

	//Must call the value 'DenominatorValue'.  Must apply date filters of >= @DateFrom and < @DateTo.
	riReport.SetDefaultDenominatorSql();				

	// report
	CString strSelect, strDenom,strNum;

	strSelect = " SELECT PatientID, UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Birthdate, HomePhone, WorkPhone, ItemDescriptionLine, ItemDate, ItemDescription, ItemProvider, ItemSecProvider, ItemLocation, MiscDesc, ItemMisc, Misc2Desc, ItemMisc2, Misc3Desc, ItemMisc3, Misc4Desc, ItemMisc4";

	// (r.gonet 2015-02-18 13:04) - PLID 64437 - We now join the NexWebCcdaAccessHistoryT onto the MailSent record and from there get the PersonID
	// rather than joining NexWebCcdaAccessHistoryT to the NexWebLoginInfoT on the username column. This is because we now keep access history 
	// instead of deleting it when a NexWeb username is deleted. Removed Check for an enabled NexWebLoginInfoT record since if they have a NexWebCcdaAccessHistoryT
	// record, then they had access to NexWeb at some point, even if that login is now disabled or deleted. The old left join on NexWebLoginInfoT may have also
	// been double counting patients if they had more than one login. Left it in place for the denominator since it is to get the
	// misc description than to filter.
	strNum = "FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, \r\n"
		" PersonT.Middle, PersonT.Last, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Birthdate,  \r\n"
		" PersonT.HomePhone, PersonT.WorkPhone, \r\n"
		" 'EMN' as ItemDescriptionLine, EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, \r\n"
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, \r\n"
		" LocationsT.Name as ItemLocation, '' as MiscDesc, '' as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, \r\n"
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4 \r\n"
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n"		
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID \r\n"
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID \r\n"
		" WHERE EMRMasterT.DELETED = 0 [ProvFilter] [LocFilter] [ExclusionsFilter] \r\n"		
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter] "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo "
		" AND PersonT.ID IN "
		" ( "
		" 	SELECT MailSent.PersonID FROM MailSent "
		" 	INNER JOIN NexWebCcdaAccessHistoryT ON NexWebCcdaAccessHistoryT.MailSentMailID = MailSent.MailID "
		" 	WHERE NexWebCcdaAccessHistoryT.AccessType IN (-1, -2, -3) "
		" ) "
		" ) N";

	strDenom = " FROM (SELECT PersonT.ID as PatientID, PatientsT.UserDefinedID, PersonT.First, "
		" PersonT.Last, PersonT.Middle, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
		" PersonT.Zip, PersonT.Birthdate, PersonT.HomePhone, PersonT.WorkPhone, 'EMN' as ItemDescriptionLine, "
		" EMRMasterT.Date as ItemDate, EMRMasterT.Description as ItemDescription, "
		" dbo.GetEmnProviderList(EMRMasterT.ID) AS ItemProvider, "
		" dbo.GetEmnSecondaryProviderList(EMRMasterT.ID) as ItemSecProvider, "
		" LocationsT.Name as ItemLocation, 'First NexWeb Login Date' as MiscDesc, dbo.AsDateNoTime(MinDate) as ItemMisc, '' as Misc2Desc, '' as ItemMisc2, "
		" '' as Misc3Desc, '' as ItemMisc3, '' as Misc4Desc, '' as ItemMisc4  "
		" FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID  "
		" LEFT JOIN EMRMasterT ON PersonT.ID = EMRMasterT.PatientID "
		" LEFT JOIN LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
		" LEFT JOIN  "
		" (SELECT Min(CreatedDate) as MinDate, PersonID FROM NexwebLoginInfoT WHERE Enabled = 1 GROUP BY PersonID) NexWebLoginT "
		" ON EMRMasterT.PatientID = NexwebLoginT.PersonID "
		" WHERE EMRMasterT.DELETED = 0  "
		" AND EMRMasterT.Date >= @DateFrom AND EMRMasterT.Date < @DateTo [ProvFilter] [LocFilter] [ExclusionsFilter]   "
		" AND PatientsT.CurrentStatus <> 4 AND PatientsT.PersonID > 0 [PatientIDFilter]) P ";


	riReport.SetReportInfo(strSelect, strNum, strDenom);


	m_aryReports.Add(riReport);
}


//(s.dhole 9/24/2014 1:14 PM ) - PLID 63765 This query return denominator 
//  Return all emn who have provider filter  and all visit code  OR Those emn who have provider filter and Emn date matches with Bill(Same provider filter) with Visit code
CString CCHITReportInfoListing::GetClinicalSummaryCommonDenominatorSQL(CString strCodes)
{

	CString strCommonDenominatorSQL;
	strCommonDenominatorSQL.Format(R"( ( 
		   SELECT EMRInnerT.PatientID, EMRInnerT.Date,EMRInnerT.ID FROM EMRMasterT EMRInnerT LEFT JOIN EMRChargesT ON EMRInnerT.ID = EMRChargesT.EMRID 
		   INNER JOIN ServiceT ON EMRChargesT.ServiceID = ServiceT.ID 
			INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID 
		   WHERE CPTCodeT.Code IN (%s) AND EMRChargesT.Deleted = 0 AND EMRInnerT.DELETED = 0 
		   [InnerProvFilter] 
			UNION 
			SELECT LineItemT.PatientID, LineItemT.Date,EMRMasterQ.ID FROM LineItemT INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID 
			INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID 
				 LEFT JOIN LineItemCorrectionsT OrigLineItems ON LineItemT.ID = OrigLineItems.OriginalLineItemID 
				 LEFT JOIN LineItemCorrectionsT AS VoidingLineItemsQ ON LineItemT.ID = VoidingLineItemsQ.VoidingLineItemID 
		  CROSS APPLY(SELECT TOP 1 EMRInnerT.ID, EMRInnerT.Deleted ,EMRInnerT.Date FROM EMRMasterT EMRInnerT 
		   WHERE EMRInnerT.Deleted = 0  [InnerProvFilter]  
		  AND  LineItemT.Date = EMRInnerT.Date AND EMRInnerT.PatientID = LineItemT.PatientID 
		  ORDER BY EMRInnerT.ID  ) AS EMRMasterQ  
				 WHERE (VoidingLineItemsQ.VoidingLineItemID IS NULL AND OrigLineItems.OriginalLineItemID IS NULL) 
			AND LineItemT.Type = 10 AND LineItemT.Deleted = 0 AND EMRMasterQ.Deleted = 0  AND EMRMasterQ.Date IS NOT NULL AND CPTCodeT.Code IN (%s) 
		   [ProvChargeFilter] 
		-- (d.singleton 2014-01-30 12:01) - PLID 59677 - bug with mu core 13 not filtering on charge date correctly
		 ) ChargeFilterQ )", strCodes, strCodes
		);
	return strCommonDenominatorSQL;
}

void CCHITReportInfoListing::CreateStage2Reports(bool bUseModified)
{
	if (bUseModified) {
		// only create reports related to Mod. Stage 2

		// Measure.3.1
		CreateStage2CPOEReport_MedicationOrders(bUseModified);

		// Measure.3.2
		CreateStage2CPOEReport_LaboratoryOrders(bUseModified);

		// Measure.3.3
		CreateStage2CPOEReport_RadiologyOrders(bUseModified);

		// Measure.4
		CreateStage2EPrescriptionReport(bUseModified);

		// Measure.5
		CreateStage2SummaryOfCareTransmittedReport(bUseModified);

		// Measure.6
		CreateStage2EducationResourceEMRReport(bUseModified);

		// Measure.7
		CreateStage2MedicalReconciliationReport(bUseModified);

		// Measure.8.1
		CreateStage2ElectronicAccessReport(bUseModified);

		// Measure.8.2
		CreateStage2ElectronicInformationViewedReport(bUseModified);

		// Measure.9
		CreateStage2ElectronicMessagingReport(bUseModified);
	}
	else {
		// create all stage 2 reports
		//Stage 2 Reporting functions will go here

		// (r.farnworth 2013-10-21 15:37) - PLID 59127 - MU.CORE.01.A
		CreateStage2CPOEReport_MedicationOrders(bUseModified);

		// (r.farnworth 2013-10-22 15:52) - PLID 59145 - MU.CORE.01.B
		CreateStage2CPOEReport_RadiologyOrders(bUseModified);

		// (r.farnworth 2013-10-23 10:59) - PLID 59146 - MU.CORE.01.C
		CreateStage2CPOEReport_LaboratoryOrders(bUseModified);

		// (r.farnworth 2013-10-23 16:17) - PLID 59158 - MU.CORE.02
		CreateStage2EPrescriptionReport(bUseModified);

		//TES 10/15/2013 - PLID 58993 - MU.CORE.03
		CreateStage2DemographicsReport();

		//TES 10/15/2013 - PLID 59006 - MU.CORE.04
		CreateStage2HeightWeightBPReport();

		//TES 10/16/2013 - PLID 59007 - MU.CORE.05
		CreateStage2SmokingStatusReport();

		// (r.farnworth 2013-11-13 11:27) - PLID 59455 - MU.CORE.07.A
		CreateStage2ElectronicAccessReport(bUseModified);

		// (r.farnworth 2013-11-14 16:23) - PLID 59501 - MU.CORE.07.B
		CreateStage2ElectronicInformationViewedReport(bUseModified);

		// (r.farnworth 2013-10-15 15:47) - PLID 59014 -MU.CORE.08
		CreateStage2ClinicalSummaryReport();

		// (r.farnworth 2013-10-16 13:46) - PLID 59036 - MU.CORE.10
		CreateStage2ClinicalLabResultsReport();

		// (r.farnworth 2013-10-16 15:56) - PLID 59055 - MU.CORE.12
		CreateStage2RemindersReport();

		// (r.farnworth 2013-10-21 10:20) - PLID 59116 - MU.CORE.13
		CreateStage2EducationResourceEMRReport(bUseModified);

		// (r.farnworth 2013-10-30 15:15) - PLID 59236 - MU.CORE.14
		CreateStage2MedicalReconciliationReport(bUseModified);

		// (r.farnworth 2013-11-14 11:24) - PLID 59489 - MU.CORE.15.A
		CreateStage2SummaryOfCareProvidedReport();

		// (r.farnworth 2013-11-14 14:23) - PLID 59490 - MU.CORE.15.B
		CreateStage2SummaryOfCareTransmittedReport(bUseModified);

		// (r.farnworth 2013-10-24 16:10) - PLID 59170 - MU.CORE.17
		CreateStage2ElectronicMessagingReport(bUseModified);

		// (r.farnworth 2013-10-29 11:15) - PLID 59217 - MU.MENU.02
		CreateStage2ElectronicNotesReport();

		// (r.farnworth 2013-10-30 13:54) - PLID 59231 - MU.MENU.03
		CreateStage2ImagingResultsReport();

		//TES 10/16/2013 - PLID 59047 - MU.MENU.04
		CreateStage2FamilyHistoryReport();

		//TES 10/15/2013 - PLID 59016 - Implemented the "generic" vitals reports, which are calculated differently for stage 2
		CreateStage2BloodPressureReport();
		CreateStage2HeightWeightReport();
		CreateStage2HeightReport();
		CreateStage2WeightReport();
	}
}
