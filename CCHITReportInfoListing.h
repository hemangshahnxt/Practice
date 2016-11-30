#pragma once

#include "CCHITReportInfo.h"
#include "CCHITReportMeasures.h"
#include "MUMeasureBase.h"

// (j.jones 2012-08-29 08:44) - PLID 52346 - added four eye office visit codes to the defaults
// (d.thompson 2012-10-18) - PLID 48423 - Added 3 more codes!
// (j.dinatale 2012-11-01 11:05) - PLID 53505 - moved the code define to the header
#define MU_13_DEFAULT_CODES "'99201', '99202', '99203', '99204', '99205', '99211', '99212', '99213', '99214', '99215', '99241', '99242', '99243', '99244', '99245', '99381', '99382', '99383', '99384', '99385', '99386', '99387', '99391', '99392', '99393', '99394', '99395', '99396', '99397', '99401', '99402', '99403', '99404', '99411', '99412', '99420', '99421', '99422', '99423', '99424', '99425', '99426', '99427', '99429', '99429', '99431', '99432', '99433', '99434', '99435', '99436', '99437', '99438', '99439', '99440', '99450', '99451', '99452', '99453', '99454', '99455', '99456', '99499', '99024', '92002', '92004', '92012', '92014', '92225', '92226', '92499'"

//(e.lally 2012-02-24) PLID 48266 - Created class dedicated to loading the array of CCHIT/MU report queries/info
class CCHITReportInfoListing {
public:
	// (r.gonet 06/12/2013) - PLID 55151 - Moved the measure listing to CCHITReportMeasures.h

	CCHITReportInfoListing(CWnd* pParent) {
		m_bHasEnsuredFirstDataBankOnce = FALSE;
		m_pMessageParent = pParent;
	}
	CArray<CCCHITReportInfo, CCCHITReportInfo&> m_aryReports;
	void LoadReportListing(MU::Stage eMeaningfulUseStage);
protected:
	CWnd* m_pMessageParent;
/************************************************************************************************************
							All Reports should be declared here
************************************************************************************************************/
//(e.lally 2012-02-24) PLID 48266 - Moved all these declarations from CCCHITReportsDlg
	// (d.thompson 2010-01-18) - PLID 36939 - ADM.15
	void CreateA1CReport();
	// (d.thompson 2010-01-18) - PLID 36939 - ADM.16
	void CreateHypertensiveReport();
	// (d.thompson 2010-01-18) - PLID 36939 - ADM.17
	void CreateLDLReport();
	// (j.jones 2010-01-20 08:49) - PLID 36940 - ADM.18
	void CreateSmokingCessationReport();
	// (j.jones 2010-01-20 08:49) - PLID 36940 - ADM.19
	void CreateBMIReport();
	// (j.jones 2010-01-22 11:18) - PLID 37009 - ADM.20
	void CreateCPOEReport_All();
	// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
	void CreateCPOEReport_Labs();
	// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
	void CreateCPOEReport_Prescriptions();
	// (j.jones 2010-01-21 12:05) - PLID 37009 - ADM.20
	void CreateCPOEReport_ReferralOrders();
	// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.22
	void CreateColorectalScreeningReport();
	// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.23
	void CreateMammogramReport();
	// (j.jones 2010-01-20 08:52) - PLID 36941 - ADM.24
	void CreateCardiacAspirinReport();
	// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.25
	void CreateFluVaccineReport();
	// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.27 - requires FirstDataBank
	BOOL m_bHasEnsuredFirstDataBankOnce;
	void CreateGenericPrescriptionsReport();
	// (j.jones 2010-01-20 08:54) - PLID 36942 - ADM.28
	void CreateImagingServicesReport();
	// (j.jones 2010-01-20 08:56) - PLID 36943 - ADM.35
	void CreateClinicalSummaryReport();
	// (j.jones 2010-01-20 08:56) - PLID 36943 - ADM.36
	void CreateMedicationReconciliationReport();

	// (j.gruber 2010-09-10 09:27) - PLID 40473 - Demographic Reports
	void CreatePreferredLanguageReport();
	void CreateGenderReport();
	void CreateRaceReport();
	void CreateEthnicityReport();
	void CreateBirthDateReport();
	void CreateDemographicsReport();

	// (j.gruber 2010-09-10 10:50) - PLID 40472 - Active medications
	void CreateCurrentMedicationsReport();

	// (j.gruber 2010-09-10 10:50) - PLID 40472 - Medication Allergies
	void CreateMedicationAllergiesReport();

	// (j.gruber 2010-09-10 13:15) - PLID 40477 - eprescriptions
	void CreateEPrescriptionsReport();
	void CreateSmokingStatusReport();
	void CreateClinicalLabResultsReport();


	// (j.gruber 2010-09-13 13:11) - PLID 40478 - Height, WEight, BP
	void CreateHeightReport();
	void CreateWeightReport();
	void CreateBloodPressureReport();
	void CreateHeightWeightBPReport();
	// (j.dinatale 2013-03-11 11:59) - PLID 54947 - need a report for patients with height and weight
	void CreateHeightWeightReport();

	// (j.gruber 2010-09-13 14:20) - PLID 40476 - provider patient specific resources
	//void CreateEducationResourceWellnessReport();
	void CreateEducationResourceEMRReport();
	void CreateTimelyElectronicAccessReport();

	// (j.gruber 2010-09-14 08:58) - PLID 40479 
	void CreateTimelyClinicalSummaryReport();
	void CreateElectronicCopyRequestReport();
	void CreateRemindersReport();

	// (j.gruber 2010-09-14 09:03) - PLID 40475 - Problems
	void CreateProblemListReport();

	// (j.gruber 2010-09-20 11:58) - PLID 40789
	void CreateCPOEReport_MedicationOrders();
	void CreateDischargeSummaryReport();
	void CreateReferralMedReconReport();

	/// <summary>
	/// Creates the CPOE Medication Orders (MU.CORE.01.A/Measure.3.1) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2CPOEReport_MedicationOrders(bool bUseModifiedStage2); // (r.farnworth 2013-10-21 15:29) - PLID 59127 - MU.CORE.01.A
	/// <summary>
	/// Creates the CPOE Radiology Orders (MU.CORE.01.B/Measure.3.3) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2CPOEReport_RadiologyOrders(bool bUseModifiedStage2); // (r.farnworth 2013-10-22 15:48) - PLID 59145 - MU.CORE.01.B
	/// <summary>
	/// Creates the CPOE Laboratory Orders (MU.CORE.01.C/Measure.3.2) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2CPOEReport_LaboratoryOrders(bool bUseModifiedStage2); // (r.farnworth 2013-10-23 10:41) - PLID 59146 - MU.CORE.01.C
	/// <summary>
	/// Creates the E-Prescription (MU.CORE.02/Measure.4) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2EPrescriptionReport(bool bUseModifiedStage2); // (r.farnworth 2013-10-23 16:17) - PLID 59158 - MU.CORE.02
	void CreateStage2DemographicsReport(); //TES 10/15/2013 - PLID 58993 - MU.CORE.03
	void CreateStage2HeightWeightBPReport(); //TES 10/15/2013 - PLID 59006 - MU.CORE.04
	void CreateStage2SmokingStatusReport(); //TES 10/16/2013 - PLID 59007 - MU.CORE.05
	/// <summary>
	/// Creates the Electronic Access Report (MU.CORE.07.A/Measure.8.1) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2ElectronicAccessReport(bool bUseModifiedStage2); // (r.farnworth 2013-11-13 11:24) - PLID 59455 - MU.CORE.07.A
	/// <summary>
	/// Creates the Electronic Information Viewed (MU.CORE.07.B/Measure.8.2) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2ElectronicInformationViewedReport(bool bUseModifiedStage2); // (r.farnworth 2013-11-14 16:21) - PLID 59501 - MU.CORE.07.B
	void CreateStage2ClinicalSummaryReport(); // (r.farnworth 2013-10-15 15:06) - PLID 59014 - MU.CORE.08
	void CreateStage2ClinicalLabResultsReport(); // (r.farnworth 2013-10-16 11:43) - PLID 59036 - MU.CORE.10
	void CreateStage2RemindersReport(); // (r.farnworth 2013-10-16 15:53) - PLID 59055 - MU.CORE.12
	/// <summary>
	/// Creates the Education Resource EMR (MU.CORE.13/Measure.6) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2EducationResourceEMRReport(bool bUseModifiedStage2); // (r.farnworth 2013-10-21 10:12) - PLID 59116 - MU.CORE.13
	/// <summary>
	/// Creates the Medical Reconciliation (MU.CORE.14/Measure.7) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2MedicalReconciliationReport(bool bUseModifiedStage2); // (r.farnworth 2013-10-30 15:12) - PLID 59236 - MU.CORE.14
	void CreateStage2SummaryOfCareProvidedReport(); // (r.farnworth 2013-11-14 11:17) - PLID 59489 - MU.CORE.15.A
	/// <summary>
	/// Creates the Summary of Care Transmitted (MU.CORE.15.B/Measure.5) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2SummaryOfCareTransmittedReport(bool bUseModifiedStage2); // (r.farnworth 2013-11-14 14:07) - PLID 59490 - MU.CORE.15.B
	/// <summary>
	/// Creates the Electronic Messaging (MU.CORE.17/Measure.9) report.
	/// </summary>
	/// <param name="bUseModifiedStage2">Indicates whether the modified Stage 2 name should be used for the report.</param>
	void CreateStage2ElectronicMessagingReport(bool bUseModifiedStage2); // (r.farnworth 2013-10-24 15:45) - PLID 59170 - MU.CORE.17
	void CreateStage2ElectronicNotesReport(); // (r.farnworth 2013-10-29 10:56) - PLID 59217 - MU.MENU.02
	void CreateStage2ImagingResultsReport(); // (r.farnworth 2013-10-30 13:39) - PLID 59231 - MU.MENU.03
	void CreateStage2FamilyHistoryReport(); //TES 10/16/2013 - PLID 59047 - MU.MENU.04
	
	//TES 10/15/2013 - PLID 59016 - These are all calculated slightly differently for Stage 2
	void CreateStage2HeightReport();
	void CreateStage2WeightReport();
	void CreateStage2BloodPressureReport();
	void CreateStage2HeightWeightReport();
	/// <summary>
	/// Creates the appropriate stage 2 reports based on whether or not modified stage 2 reporting is being used.
	/// </summary>
	/// <param name="bUseModified">Indicates whether or not modified stage 2 reporting is being used.</param>
	void CreateStage2Reports(bool bUseModified);
	
/************************************************************************************************************
							End Reports
************************************************************************************************************/
	//(s.dhole 9/8/2014 10:41 AM ) - PLID 62794
	CString GetSQLReminderMethods();

	//(s.dhole 9/26/2014 8:38 AM ) - PLID 63765
	CString GetClinicalSummaryCommonDenominatorSQL(CString strReport);
};
