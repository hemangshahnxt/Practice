#pragma once

//(e.lally 2012-04-03) PLID 48264 - These values are stored in data so do NOT change them.
namespace CCHITReportMeasures
{
	enum CCHITReportInternalID {
		eInvalid = -1,
		eA1C = 1,
		eHypertensive = 2,
		eLDL = 3,
		eSmokingCessation = 4,
		eBMI = 5,
		eCPOEReport_All = 6,
		eCPOEReport_Labs = 7,
		eCPOEReport_Prescriptions = 8,
		eCPOEReport_ReferralOrders = 9,
		eColorectalScreening = 10,
		eMammogram = 11,
		eCardiacAspirin = 12,
		eFluVaccine = 13,
		eGenericPrescriptions = 14,
		eImagingServices = 15,
		eClinicalSummary = 16,
		eMedicationReconciliation = 17,

		eMUDemographics = 18,
		eMUCurrentMedications = 19,
		eMUMedicationAllergies = 20,
		eMUEPrescriptions = 21,
		eMUSmokingStatus = 22,
		eMUClinicalLabResults = 23,
		eMUHeight = 24,
		eMUWeight = 25,
		eMUBloodPressure = 26,
		eMUHeightWeightBP = 27,
		eMUEducationResourceEMR = 28,
		eMUTimelyElectronicAccess = 29,
		eMUTimelyClinicalSummary = 30,
		eMUElectronicCopyRequest = 31,
		eMUReminders = 32,
		eMUProblemList = 33,
		eMUCPOEReport_MedicationOrders = 34,
		eMUDischargeSummary = 35,
		eMUReferralMedRecon = 36,
		eMUHeightWeight = 37,	// (j.dinatale 2013-03-11 11:24) - PLID 54947 - need a row for just height and weight

		eMUStage2Demographics = 38, //TES 10/15/2013 - PLID 58993 - MU.CORE.03

		eMUStage2HeightWeightBP = 39, //TES 10/15/2013 - PLID 59006 - MU.CORE.04

		eMUStage2ClinicalSummary = 40, // (r.farnworth 2013-10-15 15:45) - PLID 59014 - MU.CORE.08

		//TES 10/16/2013 - PLID 59016
		eMUStage2Height = 41,
		eMUStage2Weight = 42,
		eMUStage2HeightWeight = 43,
		eMUStage2BloodPressure = 44,

		eMUStage2SmokingStatus = 45, //TES 10/16/2013 - PLID 59007 - MU.CORE.05

		eMUStage2ClinicalLabResults = 46, // (r.farnworth 2013-10-16 13:40) - PLID 59036 - MU.CORE.10
		eMUStage2PreventiveCare = 47, // (r.farnworth 2013-10-16 15:53) - PLID 59055 - MU.CORE.12
		eMUStage2EducationResourceEMRReport = 48, // (r.farnworth 2013-10-21 10:15) - PLID 59116 - MU.CORE.13
		eMUStage2CPOEReport_MedicationOrders = 49, // (r.farnworth 2013-10-21 15:35) - PLID 59127 - MU.CORE.01.A

		eMUStage2FamilyHistory = 50, //TES 10/16/2013 - PLID 59047 - MU.MENU.04
		eMUStage2CPOEReport_RadiologyOrders = 51, // (r.farnworth 2013-10-22 15:50) - PLID 59145 - MU.CORE.01.B
		eMUStage2CPOEReport_LaboratoryOrders = 52, // (r.farnworth 2013-10-23 10:54) - PLID 59146 - MU.CORE.01.C
		eMUStage2EPrescription = 53, // (r.farnworth 2013-10-23 16:17) - PLID 59158 - MU.CORE.02
		eMUStage2ElectronicMessaging = 54, // (r.farnworth 2013-10-24 15:53) - PLID 59170 - MU.CORE.17
		eMUStage2ElectronicNotes = 55, // (r.farnworth 2013-10-29 11:03) - PLID 59217 - MU.MENU.02
		eMUStage2ImagingResults = 56, // (r.farnworth 2013-10-30 13:39) - PLID 59231 - MU.MENU.03
		eMUStage2MedicalReconciliation = 57, // (r.farnworth 2013-10-30 15:12) - PLID 59236 - MU.CORE.14
		eMUStage2ElectronicAccess = 58, // (r.farnworth 2013-11-13 11:24) - PLID 59455 - MU.CORE.07.A
		eMUStage2SummaryOfCareProvided = 59, // (r.farnworth 2013-11-14 11:17) - PLID 59489 - MU.CORE.15.A
		eMUStage2SummaryOfCareTransmitted = 60, // (r.farnworth 2013-11-14 14:21) - PLID 59490 - MU.CORE.15.B
		eMUStage2ElectronicInformationViewed = 61, // (r.farnworth 2013-11-14 16:22) - PLID 59501 - MU.CORE.07.B

	};
}