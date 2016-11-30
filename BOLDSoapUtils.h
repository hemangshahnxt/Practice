//BoldSoapUtils.h

#pragma once

#include "SoapUtils.h"

// (j.gruber 2010-06-01 11:52) - PLID 38211
enum BOLDBOOL {
	bNone = -1,
	bFalse = 0,
	bTrue = 1,
};

enum BOLDMetricUnitType {
	bmutNone = -1,
	bmutStandard = 0,
	bmutMetric,
};

struct BOLDMetricUnit {
	BOLDMetricUnitType bmuType;
	double dblValue;
	BOLDBOOL bEstimated;
};


struct BOLDInsurance {
	CStringArray straryPaymentCodes;
	CString strPrecertCode;
	BOLDMetricUnit bmuWeightLossAmt;
	BOLDBOOL bPreCertMentalHealth;
};

struct BOLDPrevBarSurg {
	long nInternalID;
	CString strCode;
	CString strName;
	BOLDMetricUnit bmuOriginalWt;
	BOLDMetricUnit bmuLowestWt;
	long nYear;
	CString strSurgeonID;
	CStringArray straryAdverseEventCodes;
};

struct BOLDPatientVisitInfo {

	CString strFirst;
	CString strLast;
	CString strMiddleInit;
	CString strSuffix;
	long nYearOfBirth;
	CString strGenderCode;
	CString strEmploymentCode;
	CString strStateCode;
	CString strCountryCode;
	BOLDBOOL bConsentReceived;
	CStringArray straryRaceCodes;
	CStringArray straryPrevNonBarCodes;
	BOLDInsurance *pboldPatIns;
	CPtrArray prevBariatricSurgeries;
};

struct BOLDPreOpVisitInfo {
	CString strVisitID;
	COleDateTime dtVisitDate;
	BOLDMetricUnit bmuWeight;
	BOLDMetricUnit bmuHeight;
	CStringArray straryVitamins;
	CStringArray straryMedications;
	CString strHYPERT;
	CString strCONGHF;
	CString strISCHHD;
	CString strANGASM;
	CString strPEVASD;
	CString strLOEXED;
	CString strDVTPE;
	CString strGLUMET;
	CString strLIPDYH;
	CString strGOUHYP;
	CString strOBSSYN;
	CString strOBHSYN;
	CString strPULHYP;
	CString strASTHMA;
	CString strGERD;
	CString strCHOLEL;
	CString strLVRDIS;
	CString strBCKPAIN;
	CString strMUSDIS;
	CString strFBMGIA;
	CString strPLOVSYN;
	CString strMENIRG;
	CString strPSYIMP;
	CString strDEPRSN;
	CString strCONMEN;
	CString strALCUSE;
	CString strTOBUSE;
	CString strSUBUSE;
	CString strSTURIN;
	CString strPSCRBR;
	CString strADBHER;
	CString strFUNSTAT;
	CString strABDSKN;
};

enum BOLDTimeUnit {
	btuNone = -1,
	btuHours = 0,
	btuDays = 1,
};

struct BOLDAdverseEvents {
	long nInternalID;
	CString strCode;
	long nTimeAfterSurgery;
	BOLDTimeUnit btuTimeAfterMeasurement;
	CString strSurgeonID;
	CStringArray strarySurgeryCodes;
};

struct BOLDHospitalVisit {

	CString strHospitalID;
	COleDateTime dtSurgery;
	COleDateTime dtAdmission;
	COleDateTime dtLastWeight;
	BOLDMetricUnit bmuHeight;
	BOLDBOOL bRevision;
	CString strFacilityID;
	CString strSurgeonID;
	double dblDurationSurgery;
	double dblDurationAnesthesia;
	CString sdblEstBloodLoss;
	CString sdblBloodTransfusionUnits;
	BOLDMetricUnit bmuLastWeightBeforeSurgery;
	BOLDBOOL bSurgicalResidentParticipated;
	BOLDBOOL bSurgicalFellowParticipated;
	COleDateTime dtDischargeDate;
	CString strDischargeLocation;
	CString strASAClassificationCode;
	CString strBariatricProcedureCode;
	CString strBariatricTechniqueCode;
	CStringArray straryDVTTherapies;
	CStringArray straryConcurrentProcs;
	CStringArray straryIntraOpAdverseEvents;
	CPtrArray aryAdverseEventsBeforeDischarge;
};


struct BOLDPostOpVisitInfo {
	CString strVisitID;
	COleDateTime dtVisitDate;
	BOLDMetricUnit bmuWeight;
	BOLDMetricUnit bmuHeight;
	CStringArray straryVitamins;
	CStringArray straryMedications;
	CString strSupportGroup;
	CString strHYPERT;
	CString strCONGHF;
	CString strANGASM;
	CString strPEVASD;
	CString strLOEXED;
	CString strGLUMET;
	CString strLIPDYH;
	CString strGOUHYP;
	CString strOBSSYN;
	CString strOBHSYN;
	CString strPULHYP;
	CString strASTHMA;
	CString strGERD;
	CString strCHOLEL;
	CString strLVRDIS;
	CString strBCKPAIN;
	CString strMUSDIS;
	CString strFBMGIA;
	CString strPLOVSYN;
	CString strMENIRG;
	CString strPSYIMP;
	CString strDEPRSN;
	CString strCONMEN;
	CString strALCUSE;
	CString strTOBUSE;
	CString strSUBUSE;
	CString strSTURIN;
	CString strPSCRBR;
	CString strADBHER;
	CString strFUNSTAT;
	CString strABDSKN;
};

struct BOLDPostOpAEVisitInfo {

	CString strVisitID;
	CString strCode;
	COleDateTime dtEvent;
	CString strFacilityID;
	CString strSurgeonID;
	CStringArray strarySurgeryCodes;
};

struct BOLDGeneralVisitInfo {
	CString strVisitID;
	COleDateTime dtVisit;
	BOLDMetricUnit bmuWeight;
	BOLDMetricUnit bmuHeight;
};

struct BOLDOtherVisitInfo {
	CString strVisitID;
	COleDateTime dtVisit;	
};

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBoldURL();
CString GetBoldURI();
CString GetBOLDBOOLValue(BOLDBOOL bBool);
CString GetBTMValue(BOLDTimeUnit btuVal);
// (j.gruber 2010-06-01 11:53) - PLID 38949
CString GenerateBOLDInsuranceXML(BOLDInsurance *pboldIns);
// (j.gruber 2010-06-01 11:53) - PLID 38949
CString GenerateBOLDPrevBarSurgXML(CPtrArray &paryPrevSurgs);
// (j.gruber 2010-06-01 11:53) - PLID 38949
CString GenerateBOLDPatientVisitXML(BOLDPatientVisitInfo *pboldPatInfo);
// (j.gruber 2010-06-01 11:54) - PLID 38950
CString GenerateBOLDPreOpVisitXML(BOLDPreOpVisitInfo *pPreOpVisit);
// (j.gruber 2010-06-01 11:54) - PLID 38951
CString GenerateBOLDHospitalVisitXML(BOLDHospitalVisit *pHospVisit);
// (j.gruber 2010-06-01 11:54) - PLID 38951
CString GenerateAdverseEventsBeforeDischargeXML(CPtrArray *aryAdverseEvents);
// (j.gruber 2010-06-01 11:55) - PLID 38953
CString GenerateBOLDPostOpVisitXML(BOLDPostOpVisitInfo *pPostOpVisit);
// (j.gruber 2010-06-01 11:56) - PLID 38954
CString GenerateBOLDPostOpAdverseEventsXML(BOLDPostOpAEVisitInfo *pPOAEVisit);
// (j.gruber 2010-06-01 11:55) - PLID 38955
CString GenerateBOLDGeneralVisitXML(BOLDGeneralVisitInfo *pGenVisit);
CString GenerateBOLDOtherVisitXML(BOLDOtherVisitInfo *pOtherVisit);

// (j.gruber 2010-06-01 11:54) - PLID 38949
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPatient(long nPatientID, BOLDPatientVisitInfo *pboldPatInfo, CStringArray *straryMessages);
// (j.gruber 2010-06-01 11:54) - PLID 38950
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPreOpVisit(long nPatientID, BOLDPreOpVisitInfo *pPreOpVisit, CString &strVisitID, CStringArray *straryMessages);
// (j.gruber 2010-06-01 11:54) - PLID 38951
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldHospVisit(long nPatientID, BOLDHospitalVisit *pHospVisit, CString &strVisitID, CStringArray *straryMessages);
// (j.gruber 2010-06-01 11:55) - PLID 38953
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPostOpVisit(long nPatientID, BOLDPostOpVisitInfo *pPostOpVisit, CString &strVisitID, CStringArray *straryMessages);
// (j.gruber 2010-06-01 11:56) - PLID 38954
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPostOpAdverseEventVisit(long nPatientID, BOLDPostOpAEVisitInfo *pPOAEVisit, CString &strVisitID, CStringArray *straryMessages);
// (j.gruber 2010-06-01 11:55) - PLID 38955
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldGeneralVisit(CString strVisitType, CString strSOAPCall, long nPatientID, BOLDGeneralVisitInfo *pGenVisit, CString &strVisitID, CStringArray *straryMessages);
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldOtherVisit(long nPatientID, BOLDOtherVisitInfo *pOtherVisit, CString &strVisitID, CStringArray *straryMessages);

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetXMLDateString(COleDateTime dt);
BOOL GetResponseInfo(MSXML2::IXMLDOMNodePtr xmlResponse, CString strResultName, CString &strStatus, CStringArray *straryMessages, CString &strBuildID, CString &strCorrelationID, CString &ReservationExpires, CString &strReservationID, CString &strVersion);