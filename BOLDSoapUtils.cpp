//BoldSoapUtils.cpp
#include "stdafx.h"
#include "BOLDSoapUtils.h"
#include "SOAPUtils.h"
#include "VersionInfo.h"
#include "InternationalUtils.h"

// (j.gruber 2010-06-01 11:52) - PLID 38211
BOOL IsProductionAccount() {
	
	long nProduction = GetRemotePropertyInt("BOLDProductionMode", 0, 0, "<None>", true);
	if (nProduction == 1) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBoldURL() {

	if (IsProductionAccount()) {
		// (j.gruber 2011-05-17 14:10) - PLID 43744 - they changed their service address
		CString strProductionServices = GetRemotePropertyText("BOLDProductionServices", "https://service.surgicalreview.org/bold/BoldService.asmx", 0, "<None>");
		return strProductionServices;
		 //return "https://www.surgicalreview.org/BoldServices/BoldService.asmx";
	}
	else {
		// (j.gruber 2011-05-17 14:12) - PLID 43744 - this changed too
		CString strPreProductionServices = GetRemotePropertyText("BOLDPreProductionServices", "https://service.surgicalreview.org/boldstaging/BoldService.asmx", 0, "<None>");
		return strPreProductionServices;
		//return  "https://209.34.248.71/BoldServices/BoldService.asmx";
	}

}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBoldURI() {
	return "http://www.surgicalreview.org/Bold";
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBOLDUsername() {
	CString strUserName;
	if (IsProductionAccount()) {
		strUserName = "NextechProduction";
		//strUserName = GetRemotePropertyText("BOLDUsername", "", 0, "<None>", true);
	}
	else {
		strUserName = "NextechStage";
	}
	return strUserName;

}

// (j.gruber 2010-06-01 11:52) - PLID 38211
// (j.gruber 2012-09-14 11:22) - PLID 52652 - renamed to take out "test"
CString GetBOLDPassword() {
	
	if (IsProductionAccount()) {
		return "N3xt3chW5*461";
	}
	else {
		return "N3xTSt@ge*715";
	}

}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetPracticeCOEID() {

	if (IsProductionAccount()) {
		CString strCOEID = GetRemotePropertyText("BOLDPracticeCOEID", "", 0, "<None>", true);
		return strCOEID;
	}
	else {
		return "2000DBB";
	}
	
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
//this isn't used, but just in case it ever is, I'm leaving the function
CString GetRequestID() {
	//figure out what this is
	return "";
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
//this isn't used, but just in case it ever is, I'm leaving the function
CString GetSecurityToken() {
	return "";
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetVendorCode() {

	if (IsProductionAccount()) {
		return "6CFB5CD3-F3F2-4E03-96C3-437B5209F5EB";
	}
	else {
		return "ED16B617-7183-499F-9782-CDEFA0AB2125";
	}
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBMUValue(BOLDMetricUnitType bmuType) {

	switch (bmuType) {
		case bmutStandard:
			return "Standard";
		break;
		case bmutMetric:
			return "Metric";
		break;

		default:
			return "";
		break;
	}
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBTMValue(BOLDTimeUnit btuVal) {

	switch (btuVal) {
		case btuHours:
			return "HOURS";
		break;
		case btuDays:
			return "DAYS";
		break;

		default:
			return "";
		break;
	}
}

// (j.gruber 2010-06-01 11:52) - PLID 38211
CString GetBOLDBOOLValue(BOLDBOOL bBool) {

	switch (bBool) {
		case bTrue:
			return "true";
		break;
		case bFalse:
			return "false";
		break;

		default:
			return "";
		break;
	}
}

// (j.gruber 2010-06-01 11:53) - PLID 38949
CString GenerateBOLDInsuranceXML(BOLDInsurance *pboldIns) {

	CString strXML;

	if (pboldIns->straryPaymentCodes.GetSize() > 0) {
		CString strPayCodes;
		for (int i = 0; i < pboldIns->straryPaymentCodes.GetSize(); i++) {
			strPayCodes  += GetXMLElementValuePair("string", pboldIns->straryPaymentCodes.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("PaymentCodes", strPayCodes);
	}

	if (!pboldIns->strPrecertCode.IsEmpty()) {
		strXML += GetXMLElementValuePair("PreCertProgramCode", pboldIns->strPrecertCode);
	}

	CString strWeightLoss;
	//check to see if its 0
	//returns 0 if dblValue = 0 (double-wise)
	if (LooseCompareDouble(pboldIns->bmuWeightLossAmt.dblValue, 0.0, 0.0000001) != 0) {	
		strWeightLoss += GetXMLElementValuePair("UnitType", GetBMUValue(pboldIns->bmuWeightLossAmt.bmuType));
		strWeightLoss += GetXMLElementValuePair("MetricValue", AsString(pboldIns->bmuWeightLossAmt.dblValue));
		strWeightLoss += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pboldIns->bmuWeightLossAmt.bEstimated));

		strXML += GetXMLElementValuePair_Embedded("WeightLossAmount", strWeightLoss);
	}

	if (pboldIns->bPreCertMentalHealth != bNone) {
		strXML += GetXMLElementValuePair("PreCertMentalHealth", GetBOLDBOOLValue(pboldIns->bPreCertMentalHealth));
	}

	return strXML;	
}


// (j.gruber 2010-06-01 11:53) - PLID 38949
CString GenerateBOLDPrevBarSurgXML(CPtrArray &paryPrevSurgs) 
{
	CString strXML;
	
	for (int i = 0; i < paryPrevSurgs.GetSize(); i++) {

		BOLDPrevBarSurg *prevSurg = (BOLDPrevBarSurg *)paryPrevSurgs.GetAt(i);

		CString strSurg;
		strSurg += GetXMLElementValuePair("Code", prevSurg->strCode);
		strSurg += GetXMLElementValuePair("Name", prevSurg->strName);
		
		CString strOrigWeight;		
		strOrigWeight += GetXMLElementValuePair("UnitType", GetBMUValue(prevSurg->bmuOriginalWt.bmuType));
		strOrigWeight += GetXMLElementValuePair("MetricValue", AsString(prevSurg->bmuOriginalWt.dblValue));
		strOrigWeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(prevSurg->bmuOriginalWt.bEstimated));
		strSurg += GetXMLElementValuePair_Embedded("OriginalWeight", strOrigWeight);

		CString strLowWeight;
		strLowWeight += GetXMLElementValuePair("UnitType", GetBMUValue(prevSurg->bmuLowestWt.bmuType));
		strLowWeight += GetXMLElementValuePair("MetricValue", AsString(prevSurg->bmuLowestWt.dblValue));
		strLowWeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(prevSurg->bmuLowestWt.bEstimated));
		strSurg += GetXMLElementValuePair_Embedded("LowestWeightAchieved", strLowWeight);

		strSurg += GetXMLElementValuePair("Year", AsString(prevSurg->nYear));
		strSurg += GetXMLElementValuePair("SurgeonID", prevSurg->strSurgeonID);
		
		if (prevSurg->straryAdverseEventCodes.GetSize() > 0) {
			CString strAECodes;
			for (int i = 0; i < prevSurg->straryAdverseEventCodes.GetSize(); i++) {
				strAECodes += GetXMLElementValuePair("string", prevSurg->straryAdverseEventCodes.GetAt(i));
			}
			strSurg += GetXMLElementValuePair_Embedded("AdverseEventCodes", strAECodes);		
		}

		strXML += GetXMLElementValuePair_Embedded("dtoPreviousBariatricSurgery", strSurg);
	}
		
	return strXML;
}

// (j.gruber 2010-06-01 11:53) - PLID 38949
CString GenerateBOLDPatientVisitXML(BOLDPatientVisitInfo *pboldPatInfo) {

	CString strXML;

	if (!pboldPatInfo->strFirst.IsEmpty()) {
		strXML += GetXMLElementValuePair("FirstName", pboldPatInfo->strFirst);
	}
	if (!pboldPatInfo->strLast.IsEmpty()) {
		strXML += GetXMLElementValuePair("LastName", pboldPatInfo->strLast);
	}
	if (!pboldPatInfo->strMiddleInit.IsEmpty()) {
		strXML += GetXMLElementValuePair("MiddleInitial", pboldPatInfo->strMiddleInit);
	}

	if (!pboldPatInfo->strSuffix.IsEmpty()) {
		strXML += GetXMLElementValuePair("Suffix", pboldPatInfo->strSuffix);
	}
	strXML += GetXMLElementValuePair("YearOfBirth", AsString(pboldPatInfo->nYearOfBirth));
	strXML += GetXMLElementValuePair("GenderCode", pboldPatInfo->strGenderCode);
	strXML += GetXMLElementValuePair("EmploymentStatusCode", pboldPatInfo->strEmploymentCode);
	
	if (!pboldPatInfo->strStateCode.IsEmpty()) {
		strXML += GetXMLElementValuePair("StateCode", pboldPatInfo->strStateCode);
	}

	if (!pboldPatInfo->strCountryCode.IsEmpty()) {
		strXML += GetXMLElementValuePair("CountryCode", pboldPatInfo->strCountryCode);
	}

	//Please note: recieved is spelled wrong in their service functions
	strXML += GetXMLElementValuePair("ConsentRecieved", GetBOLDBOOLValue(pboldPatInfo->bConsentReceived));
	
	CString strRaceCodes;
	for (int i = 0; i < pboldPatInfo->straryRaceCodes.GetSize(); i++) {
		strRaceCodes += GetXMLElementValuePair("string", pboldPatInfo->straryRaceCodes.GetAt(i));
	}
	strXML += GetXMLElementValuePair_Embedded("RaceCodes", strRaceCodes);

	if (pboldPatInfo->straryPrevNonBarCodes.GetSize() > 0) {
		CString strPrevSurgeries;
		for (int i = 0; i < pboldPatInfo->straryPrevNonBarCodes.GetSize(); i++) {
			strPrevSurgeries += GetXMLElementValuePair("string", pboldPatInfo->straryPrevNonBarCodes.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("PreviousNonBariatricSurgeryCodes", strPrevSurgeries);
	}
	
	CString strInsurance =  GenerateBOLDInsuranceXML(pboldPatInfo->pboldPatIns);
	if (!strInsurance.IsEmpty()) {
		strXML += GetXMLElementValuePair_Embedded("Insurance", strInsurance);
	}

	if (pboldPatInfo->prevBariatricSurgeries.GetSize() > 0) {
		strXML += GetXMLElementValuePair_Embedded("PreviousBariatricSurgeries", GenerateBOLDPrevBarSurgXML(pboldPatInfo->prevBariatricSurgeries));
	}

	return strXML;
}

// (j.gruber 2010-06-01 11:54) - PLID 38211
CString GenerateRequestBase(long nPatientID) {
	
	CString strXML;

	strXML += GetXMLElementValuePair("PatientChartNumber", AsString(nPatientID));
	strXML += GetXMLElementValuePair("PracticeCOEID", GetPracticeCOEID());
	strXML += GetXMLElementValuePair("RequestID", GetRequestID());
	strXML += GetXMLElementValuePair("SecurityToken", GetSecurityToken());
	strXML += GetXMLElementValuePair("VendorCode", GetVendorCode());
	strXML += GetXMLElementValuePair("Version", PRODUCT_VERSION_TEXT);

	return strXML;
}

/*This function takes a validated patient Info, generates the correct XML and sends the SOAP call*/
// (j.gruber 2010-06-01 11:54) - PLID 38949
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPatient(long nPatientID, BOLDPatientVisitInfo *pboldPatInfo, CStringArray *straryMessages) 
{

	//generate the XML
	CString strXML, strPatientXML;
	strPatientXML = GetXMLElementValuePair_Embedded("Patient", GenerateBOLDPatientVisitXML(pboldPatInfo));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strPatientXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;

	if (IsProductionAccount()) {

		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction(GetBoldURL(),
			GetBoldURI(),
			"SavePatient", strXML, GetBOLDUsername(), GetBOLDPassword()	);
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
			GetBoldURI(),
			"SavePatient", strXML, GetBOLDUsername(), GetBOLDPassword());
	}


	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");
		
		CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
		
		if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
			return FALSE;
		}
	
		if (strStatus.MakeUpper() != "SUCCESS") {
			return FALSE;
		}
	}

	return TRUE;

}

// (j.gruber 2010-06-01 11:54) - PLID 38211
CString GetXMLDateString(COleDateTime dt) {

	if (dt.GetStatus() == COleDateTime::valid) {
		SYSTEMTIME sysTime;
		dt.GetAsSystemTime(sysTime);
		return FormatLocalTime_ISO8601(sysTime);
	}
	else {
		return "";
	}

}

// (j.gruber 2010-06-01 11:54) - PLID 38950
CString GenerateBOLDPreOpVisitXML(BOLDPreOpVisitInfo *pPreOpVisit) {

	CString strXML;

//#pragma TODO("Find out if we need to send a visit ID and if so what they want because it didn't like the EMNID")
	strXML += GetXMLElementValuePair("ID", pPreOpVisit->strVisitID);
	strXML += GetXMLElementValuePair("VisitDate", GetXMLDateString(pPreOpVisit->dtVisitDate));
	
	CString strWeight;
	strWeight += GetXMLElementValuePair("UnitType", GetBMUValue(pPreOpVisit->bmuWeight.bmuType));
	strWeight += GetXMLElementValuePair("MetricValue", AsString(pPreOpVisit->bmuWeight.dblValue));
	strWeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pPreOpVisit->bmuWeight.bEstimated));
	strXML += GetXMLElementValuePair_Embedded("Weight", strWeight);

	CString strHeight;
	strHeight += GetXMLElementValuePair("UnitType", GetBMUValue(pPreOpVisit->bmuHeight.bmuType));
	strHeight += GetXMLElementValuePair("MetricValue", AsString(pPreOpVisit->bmuHeight.dblValue));
	strHeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pPreOpVisit->bmuHeight.bEstimated));
	strXML += GetXMLElementValuePair_Embedded("Height", strHeight);

	if (pPreOpVisit->straryVitamins.GetSize() > 0) {
		CString strVitamins;
		for (int i = 0; i < pPreOpVisit->straryVitamins.GetSize(); i++) {
			strVitamins += GetXMLElementValuePair("string", pPreOpVisit->straryVitamins.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("VitaminCodes", strVitamins);
	}

	if (pPreOpVisit->straryMedications.GetSize() > 0) {
		CString strMeds;
		for (int i = 0; i < pPreOpVisit->straryMedications.GetSize(); i++) {
			strMeds += GetXMLElementValuePair("string", pPreOpVisit->straryMedications.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("MedicationCodes", strMeds);
	}

	CString strMorbidity;
	CString strCoMorbids;
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "HYPERT");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strHYPERT);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "CONGHF");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strCONGHF);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ISCHHD");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strISCHHD);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ANGASM");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strANGASM);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PEVASD");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strPEVASD);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);
	
	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "LOEXED");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strLOEXED);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "DVTPE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strDVTPE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "GLUMET");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strGLUMET);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "LIPDYH");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strLIPDYH);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "GOUHYP");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strGOUHYP);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "OBSSYN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strOBSSYN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "OBHSYN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strOBHSYN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PULHYP");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strPULHYP);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ASTHMA");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strASTHMA);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "GERD");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strGERD);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "CHOLEL");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strCHOLEL);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "LVRDIS");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strLVRDIS);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "BCKPAIN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strBCKPAIN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "MUSDIS");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strMUSDIS);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "FBMGIA");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strFBMGIA);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PLOVSYN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strPLOVSYN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "MENIRG");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strMENIRG);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PSYIMP");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strPSYIMP);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "DEPRSN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strDEPRSN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "CONMEN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strCONMEN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ALCUSE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strALCUSE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "TOBUSE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strTOBUSE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "SUBUSE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strSUBUSE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "STURIN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strSTURIN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PSCRBR");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strPSCRBR);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ADBHER");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strADBHER);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "FUNSTAT");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strFUNSTAT);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ABDSKN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPreOpVisit->strABDSKN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strXML += GetXMLElementValuePair_Embedded("Comorbidities", strCoMorbids);

	return strXML;
}

// (j.gruber 2010-06-01 11:54) - PLID 38950
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPreOpVisit(long nPatientID, BOLDPreOpVisitInfo *pPreOpVisit, CString &strVisitID, CStringArray *straryMessages) {

	//generate the XML
	CString strXML, strPatientXML;
	strPatientXML = GetXMLElementValuePair_Embedded("PreOperativeVisit", GenerateBOLDPreOpVisitXML(pPreOpVisit));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strPatientXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;

	if (IsProductionAccount()) {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction(GetBoldURL(),
		GetBoldURI(),
		"SavePreOperativeVisit", strXML, GetBOLDUsername(), GetBOLDPassword()	);
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
		GetBoldURI(),
		"SavePreOperativeVisit", strXML, GetBOLDUsername(), GetBOLDPassword());
	}

	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");


		if (xmlResponse != NULL) {

			CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");

			//first get the response node
			CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
			
			if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
				return FALSE;
			}
		
			if (strStatus.MakeUpper() != "SUCCESS") {
				return FALSE;
			}

			//get the visitID
			/*MSXML2::IXMLDOMNodePtr xmlResult;
			xmlResult = FindChildNode(xmlResponse, "SavePreOperativeVisitResult");
			if (xmlResult) {*/
				strVisitID = GetXMLNodeText(xmlResponse, "VisitID");
			//}
		}
	}
	return TRUE;

}

// (j.gruber 2010-06-01 11:54) - PLID 38951
CString GenerateAdverseEventsBeforeDischargeXML(CPtrArray *aryAdverseEvents){

	CString strXML;

	for (int i = 0; i < aryAdverseEvents->GetSize(); i++) {

		CString strAE;
		BOLDAdverseEvents *pAE = ((BOLDAdverseEvents*)aryAdverseEvents->GetAt(i));

		if (pAE) {

			strAE += GetXMLElementValuePair("AdverseEventCode", pAE->strCode);
			strAE += GetXMLElementValuePair("TimeAfterSurgery", AsString(pAE->nTimeAfterSurgery));
			
			CString strTime;
			strAE += GetXMLElementValuePair("TimeAfterMeasurement", GetBTMValue(pAE->btuTimeAfterMeasurement));			
			
			if (! pAE->strSurgeonID.IsEmpty()) {
				strAE += GetXMLElementValuePair("SurgeonCOEID", pAE->strSurgeonID);
			}

			if (pAE->strarySurgeryCodes.GetSize() > 0 ) {
				CString strSurgeryCodes;
				for (int i = 0; i < pAE->strarySurgeryCodes.GetSize(); i++) {
					strSurgeryCodes += GetXMLElementValuePair("string", pAE->strarySurgeryCodes.GetAt(i));
				}
				strAE += GetXMLElementValuePair_Embedded("SurgeryCodes", strSurgeryCodes);
			}
		}

		strXML += GetXMLElementValuePair_Embedded("dtoAdverseEventBeforeDischarge", strAE);
	}

	return strXML;
}

// (j.gruber 2010-06-01 11:55) - PLID 38951
CString GenerateBOLDHospitalVisitXML(BOLDHospitalVisit *pHospVisit) {

	CString strXML; 

	strXML += GetXMLElementValuePair("ID", pHospVisit->strHospitalID);
	strXML += GetXMLElementValuePair("SurgeryDate", GetXMLDateString(pHospVisit->dtSurgery));
	strXML += GetXMLElementValuePair("DateOfAdmission", GetXMLDateString(pHospVisit->dtAdmission));
	strXML += GetXMLElementValuePair("DateOfLastWeight", GetXMLDateString(pHospVisit->dtLastWeight));
	
	
	CString strHeight;
	strHeight += GetXMLElementValuePair("UnitType", GetBMUValue(pHospVisit->bmuHeight.bmuType));
	strHeight += GetXMLElementValuePair("MetricValue", AsString(pHospVisit->bmuHeight.dblValue));
	strHeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pHospVisit->bmuHeight.bEstimated));
	strXML += GetXMLElementValuePair_Embedded("Height", strHeight);
	

	strXML += GetXMLElementValuePair("Revision", GetBOLDBOOLValue(pHospVisit->bRevision));	

	strXML += GetXMLElementValuePair("FacilityCOEID", pHospVisit->strFacilityID);
	strXML += GetXMLElementValuePair("SurgeonCOEID", pHospVisit->strSurgeonID);
	strXML += GetXMLElementValuePair("DurationOfSurgery", AsString(pHospVisit->dblDurationSurgery));
	strXML += GetXMLElementValuePair("DurationOfAnesthesia", AsString(pHospVisit->dblDurationAnesthesia));
	strXML += GetXMLElementValuePair("EstimatedBloodLossInCC", (pHospVisit->sdblEstBloodLoss));
	strXML += GetXMLElementValuePair("BloodTransfusionInUnits", (pHospVisit->sdblBloodTransfusionUnits));

	CString strWeight;
	strWeight += GetXMLElementValuePair("UnitType", GetBMUValue(pHospVisit->bmuLastWeightBeforeSurgery.bmuType));
	strWeight += GetXMLElementValuePair("MetricValue", AsString(pHospVisit->bmuLastWeightBeforeSurgery.dblValue));
	strWeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pHospVisit->bmuLastWeightBeforeSurgery.bEstimated));
	strXML += GetXMLElementValuePair_Embedded("LastWeightBeforeSurgery", strWeight);

	strXML += GetXMLElementValuePair("SurgicalResidentParticipated", GetBOLDBOOLValue(pHospVisit->bSurgicalResidentParticipated));	
	strXML += GetXMLElementValuePair("SurgicalFellowParticiated", GetBOLDBOOLValue(pHospVisit->bSurgicalFellowParticipated));	

	strXML += GetXMLElementValuePair("DischargeDate", GetXMLDateString(pHospVisit->dtDischargeDate));
	strXML += GetXMLElementValuePair("DischargeLocationCode", pHospVisit->strDischargeLocation);
	strXML += GetXMLElementValuePair("ASAClassificationCode", pHospVisit->strASAClassificationCode);
	strXML += GetXMLElementValuePair("BariatricProcedureCode", pHospVisit->strBariatricProcedureCode);
	strXML += GetXMLElementValuePair("BariatricTechniqueCode", pHospVisit->strBariatricTechniqueCode);

	if (pHospVisit->straryDVTTherapies.GetSize() > 0 ) {
		CString strDVTTherapies;
		for (int i = 0; i < pHospVisit->straryDVTTherapies.GetSize(); i++) {
			strDVTTherapies += GetXMLElementValuePair("string", pHospVisit->straryDVTTherapies.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("DVTProphylaxisTherapyCodes", strDVTTherapies);
	}

	if (pHospVisit->straryConcurrentProcs.GetSize() > 0) {
		CString strConcurrentProcs;
		for (int i = 0; i < pHospVisit->straryConcurrentProcs.GetSize(); i++) {
			strConcurrentProcs += GetXMLElementValuePair("string", pHospVisit->straryConcurrentProcs.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("ConcurrentProcedureCodes", strConcurrentProcs);
	}

	if (pHospVisit->straryIntraOpAdverseEvents.GetSize() > 0 ) {
		CString strIntraOpAE;
		for (int i = 0; i < pHospVisit->straryIntraOpAdverseEvents.GetSize(); i++) {
			strIntraOpAE += GetXMLElementValuePair("string", pHospVisit->straryIntraOpAdverseEvents.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("IntraOpAdverseEventCodes", strIntraOpAE);
	}
	
	if (pHospVisit->aryAdverseEventsBeforeDischarge.GetSize() > 0) {
		strXML += GetXMLElementValuePair_Embedded("AdverseEventsBeforeDischarge", GenerateAdverseEventsBeforeDischargeXML(&pHospVisit->aryAdverseEventsBeforeDischarge));	
	}

	return strXML;

}

// (j.gruber 2010-06-01 11:55) - PLID 38951
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take otu password param
BOOL SendBoldHospVisit(long nPatientID, BOLDHospitalVisit *pHospVisit, CString &strHospitalID, CStringArray *straryMessages) {

	//generate the XML
	CString strXML, strVisitXML;
	strVisitXML = GetXMLElementValuePair_Embedded("HospitalVisit", GenerateBOLDHospitalVisitXML(pHospVisit));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strVisitXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;
	
	if (IsProductionAccount()) {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction(GetBoldURL(),
		GetBoldURI(),
		"SaveHospitalVisit", strXML, GetBOLDUsername(), GetBOLDPassword());
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
		GetBoldURI(),
		"SaveHospitalVisit", strXML, GetBOLDUsername(), GetBOLDPassword()	);
	}

	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");


		if (xmlResponse != NULL) {

			CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");

			//first get the response node
			CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
			
			if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
				return FALSE;
			}
		
			if (strStatus.MakeUpper() != "SUCCESS") {
				return FALSE;
			}

			//get the visitID
			/*MSXML2::IXMLDOMNodePtr xmlResult;
			xmlResult = FindChildNode(xmlResponse, "SavePreOperativeVisitResult");
			if (xmlResult) {*/
				strHospitalID = GetXMLNodeText(xmlResponse, "HospitalID");
			//}
		}
	}
	return TRUE;

}

// (j.gruber 2010-06-01 11:55) - PLID 38211
BOOL GetResponseInfo(MSXML2::IXMLDOMNodePtr xmlResponse, CString strResultName, CString &strStatus, CStringArray *straryMessages, CString &strBuildID, CString &strCorrelationID, CString &strReservationExpires, CString &strReservationID, CString &strVersion)
{

	MSXML2::IXMLDOMNodePtr xmlResult;
	if (strResultName.IsEmpty()) {
		xmlResult = xmlResponse;
	}
	else {
		xmlResult = FindChildNode(xmlResponse, strResultName);
	}
	if (xmlResult) {
		
		strStatus = GetXMLNodeText(xmlResult, "Acknowledge");			
		strBuildID = GetXMLNodeText(xmlResult, "Build");			
		strCorrelationID = GetXMLNodeText(xmlResult, "CorrelationId");			
		
		strReservationExpires = GetXMLNodeText(xmlResult, "ReservationExpires");			
		strReservationID = GetXMLNodeText(xmlResult, "ReservationId");			
		strVersion = GetXMLNodeText(xmlResult, "Version");			

		MSXML2::IXMLDOMNodePtr xmlMessages;
		xmlMessages = FindChildNode(xmlResult, "Messages");
		if (xmlMessages) {
			MSXML2::IXMLDOMNodePtr xmlMessage;
			xmlMessage = xmlMessages->GetfirstChild();
			while (xmlMessage) {
				CString str = (LPCTSTR)xmlMessage->text;
				straryMessages->Add(str);

				xmlMessage = xmlMessage->GetnextSibling();
			}
		}
		return TRUE;
	}
	return FALSE;
}

// (j.gruber 2010-06-01 11:55) - PLID 38953
CString GenerateBOLDPostOpVisitXML(BOLDPostOpVisitInfo *pPostOpVisit) {

	CString strXML;

//#pragma TODO("Find out if we need to send a visit ID and if so what they want because it didn't like the EMNID")
	strXML += GetXMLElementValuePair("ID", pPostOpVisit->strVisitID);
	strXML += GetXMLElementValuePair("VisitDate", GetXMLDateString(pPostOpVisit->dtVisitDate));
	
	CString strWeight;
	strWeight += GetXMLElementValuePair("UnitType", GetBMUValue(pPostOpVisit->bmuWeight.bmuType));
	strWeight += GetXMLElementValuePair("MetricValue", AsString(pPostOpVisit->bmuWeight.dblValue));
	strWeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pPostOpVisit->bmuWeight.bEstimated));
	strXML += GetXMLElementValuePair_Embedded("Weight", strWeight);

	CString strHeight;
	strHeight += GetXMLElementValuePair("UnitType", GetBMUValue(pPostOpVisit->bmuHeight.bmuType));
	strHeight += GetXMLElementValuePair("MetricValue", AsString(pPostOpVisit->bmuHeight.dblValue));
	strHeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pPostOpVisit->bmuHeight.bEstimated));
	strXML += GetXMLElementValuePair_Embedded("Height", strHeight);

	if (pPostOpVisit->straryVitamins.GetSize() > 0 ) {
		CString strVitamins;
		for (int i = 0; i < pPostOpVisit->straryVitamins.GetSize(); i++) {
			strVitamins += GetXMLElementValuePair("string", pPostOpVisit->straryVitamins.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("VitaminCodes", strVitamins);
	}

	if (pPostOpVisit->straryMedications.GetSize() > 0) {
		CString strMeds;
		for (int i = 0; i < pPostOpVisit->straryMedications.GetSize(); i++) {
			strMeds += GetXMLElementValuePair("string", pPostOpVisit->straryMedications.GetAt(i));
		}
		strXML += GetXMLElementValuePair_Embedded("MedicationCodes", strMeds);
	}

	strXML += GetXMLElementValuePair("SupportGroupFrequency", pPostOpVisit->strSupportGroup);

	CString strMorbidity;
	CString strCoMorbids;
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "HYPERT");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strHYPERT);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "CONGHF");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strCONGHF);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	/*strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ISCHHD");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strISCHHD);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);*/

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ANGASM");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strANGASM);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PEVASD");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strPEVASD);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);
	
	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "LOEXED");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strLOEXED);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	/*strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "DVTPE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strDVTPE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);*/

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "GLUMET");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strGLUMET);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "LIPDYH");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strLIPDYH);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "GOUHYP");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strGOUHYP);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "OBSSYN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strOBSSYN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "OBHSYN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strOBHSYN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PULHYP");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strPULHYP);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ASTHMA");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strASTHMA);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "GERD");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strGERD);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "CHOLEL");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strCHOLEL);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "LVRDIS");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strLVRDIS);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "BCKPAIN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strBCKPAIN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "MUSDIS");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strMUSDIS);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "FBMGIA");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strFBMGIA);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PLOVSYN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strPLOVSYN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "MENIRG");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strMENIRG);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PSYIMP");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strPSYIMP);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "DEPRSN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strDEPRSN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "CONMEN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strCONMEN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ALCUSE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strALCUSE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "TOBUSE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strTOBUSE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "SUBUSE");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strSUBUSE);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "STURIN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strSTURIN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "PSCRBR");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strPSCRBR);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ADBHER");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strADBHER);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "FUNSTAT");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strFUNSTAT);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strMorbidity = "";
	strMorbidity += GetXMLElementValuePair("ComorbidityCode", "ABDSKN");
	strMorbidity += GetXMLElementValuePair("StratificationCode", pPostOpVisit->strABDSKN);
	strCoMorbids += GetXMLElementValuePair_Embedded("dtoComorbidity", strMorbidity);

	strXML += GetXMLElementValuePair_Embedded("Comorbidities", strCoMorbids);

	return strXML;
}

// (j.gruber 2010-06-01 11:55) - PLID 38953
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPostOpVisit(long nPatientID, BOLDPostOpVisitInfo *pPostOpVisit, CString &strVisitID, CStringArray *straryMessages) {

	//generate the XML
	CString strXML, strPatientXML;
	strPatientXML = GetXMLElementValuePair_Embedded("PostOperativeVisit", GenerateBOLDPostOpVisitXML(pPostOpVisit));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strPatientXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;

	if (IsProductionAccount()) {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction(GetBoldURL(),
		GetBoldURI(),
		"SavePostOperativeVisit", strXML, GetBOLDUsername(), GetBOLDPassword());
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
		GetBoldURI(),
		"SavePostOperativeVisit", strXML, GetBOLDUsername(), GetBOLDPassword());
	}


	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");


		if (xmlResponse != NULL) {

			CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");

			//first get the response node
			CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
			
			if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
				return FALSE;
			}
		
			if (strStatus.MakeUpper() != "SUCCESS") {
				return FALSE;
			}

			//get the visitID
			/*MSXML2::IXMLDOMNodePtr xmlResult;
			xmlResult = FindChildNode(xmlResponse, "SavePreOperativeVisitResult");
			if (xmlResult) {*/
				strVisitID = GetXMLNodeText(xmlResponse, "VisitID");
			//}
		}
	}
	return TRUE;

}

// (j.gruber 2010-06-01 11:55) - PLID 38955
CString GenerateBOLDGeneralVisitXML(BOLDGeneralVisitInfo *pGenVisit) {

	CString strXML;

//	#pragma TODO("Find out if we need to send a visit ID and if so what they want because it didn't like the EMNID")
	strXML += GetXMLElementValuePair("ID", pGenVisit->strVisitID);
	strXML += GetXMLElementValuePair("VisitDate", GetXMLDateString(pGenVisit->dtVisit));
	
	CString strWeight;
	//returns 1 if dblValue > 0 (double-wise)
	if (LooseCompareDouble(pGenVisit->bmuWeight.dblValue, 0.0, 0.0000001) == 1) {
		strWeight += GetXMLElementValuePair("UnitType", GetBMUValue(pGenVisit->bmuWeight.bmuType));
		strWeight += GetXMLElementValuePair("MetricValue", AsString(pGenVisit->bmuWeight.dblValue));
		strWeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pGenVisit->bmuWeight.bEstimated));
		strXML += GetXMLElementValuePair_Embedded("Weight", strWeight);
	}

	CString strHeight;
	if (LooseCompareDouble(pGenVisit->bmuHeight.dblValue, 0.0, 0.0000001) == 1) {
		strHeight += GetXMLElementValuePair("UnitType", GetBMUValue(pGenVisit->bmuHeight.bmuType));
		strHeight += GetXMLElementValuePair("MetricValue", AsString(pGenVisit->bmuHeight.dblValue));
		strHeight += GetXMLElementValuePair("Estimated", GetBOLDBOOLValue(pGenVisit->bmuHeight.bEstimated));
		strXML += GetXMLElementValuePair_Embedded("Height", strHeight);
	}

	return strXML;
}

// (j.gruber 2010-06-01 11:55) - PLID 38955
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldGeneralVisit(CString strVisitType, CString strSOAPCall, long nPatientID, BOLDGeneralVisitInfo *pGenVisit, CString &strVisitID, CStringArray *straryMessages) {

	//generate the XML
	CString strXML, strPatientXML;
	strPatientXML = GetXMLElementValuePair_Embedded(strVisitType, GenerateBOLDGeneralVisitXML(pGenVisit));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strPatientXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;

	if (IsProductionAccount()) {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction(GetBoldURL(),
			GetBoldURI(),
			strSOAPCall, strXML, GetBOLDUsername(), GetBOLDPassword());
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
			GetBoldURI(),
			strSOAPCall, strXML, GetBOLDUsername(), GetBOLDPassword());
	}
		

	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");


		if (xmlResponse != NULL) {

			CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");

			//first get the response node
			CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
			
			if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
				return FALSE;
			}
		
			if (strStatus.MakeUpper() != "SUCCESS") {
				return FALSE;
			}

			//get the visitID
			/*MSXML2::IXMLDOMNodePtr xmlResult;
			xmlResult = FindChildNode(xmlResponse, "SavePreOperativeVisitResult");
			if (xmlResult) {*/
				strVisitID = GetXMLNodeText(xmlResponse, "VisitID");
			//}
		}
	}
	return TRUE;

}

// (j.gruber 2010-06-01 11:56) - PLID 38955
CString GenerateBOLDOtherVisitXML(BOLDOtherVisitInfo *pOthVisit) {

	CString strXML;

//	#pragma TODO("Find out if we need to send a visit ID and if so what they want because it didn't like the EMNID")
	strXML += GetXMLElementValuePair("ID", pOthVisit->strVisitID);
	strXML += GetXMLElementValuePair("VisitDate", GetXMLDateString(pOthVisit->dtVisit));
	
	return strXML;
}

// (j.gruber 2010-06-01 11:56) - PLID 38955
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldOtherVisit(long nPatientID, BOLDOtherVisitInfo *pOthVisit, CString &strVisitID, CStringArray *straryMessages) {

	//generate the XML
	CString strXML, strPatientXML;
	strPatientXML = GetXMLElementValuePair_Embedded("OtherVisit", GenerateBOLDOtherVisitXML(pOthVisit));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strPatientXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;
	
	if (IsProductionAccount()) {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		 xmlResponse = CallSoapFunction(GetBoldURL(),
			GetBoldURI(),
			"SaveOtherVisit", strXML, GetBOLDUsername(), GetBOLDPassword());
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
			GetBoldURI(),
			"SaveOtherVisit", strXML, GetBOLDUsername(), GetBOLDPassword());
	}

	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");


		if (xmlResponse != NULL) {

			CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");

			//first get the response node
			CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
			
			if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
				return FALSE;
			}
		
			if (strStatus.MakeUpper() != "SUCCESS") {
				return FALSE;
			}

			//get the visitID
			/*MSXML2::IXMLDOMNodePtr xmlResult;
			xmlResult = FindChildNode(xmlResponse, "SavePreOperativeVisitResult");
			if (xmlResult) {*/
				strVisitID = GetXMLNodeText(xmlResponse, "VisitID");
			//}
		}
	}
	return TRUE;

}

// (j.gruber 2010-06-01 11:56) - PLID 38954
CString GenerateBOLDPostOpAdverseEventsXML(BOLDPostOpAEVisitInfo *pPOAEVisit){

	CString strAE;	
	
	//strAE += GetXMLElementValuePair("AdverseEventID", pPOAEVisit->strID);
	strAE += GetXMLElementValuePair("AdverseEventID", pPOAEVisit->strVisitID);
	strAE += GetXMLElementValuePair("AdverseEventCode", pPOAEVisit->strCode);
	strAE += GetXMLElementValuePair("DateOfEvent", GetXMLDateString(pPOAEVisit->dtEvent));
	
	if (!pPOAEVisit->strFacilityID.IsEmpty()) {
		strAE += GetXMLElementValuePair("FacilityCOEID", pPOAEVisit->strFacilityID);
	}

	if (!pPOAEVisit->strSurgeonID.IsEmpty()) {
		strAE += GetXMLElementValuePair("SurgeonCOEID", pPOAEVisit->strSurgeonID);
	}

	if (pPOAEVisit->strarySurgeryCodes.GetSize() > 0) {
		CString strSurgeryCodes;
		for (int i = 0; i < pPOAEVisit->strarySurgeryCodes.GetSize(); i++) {
			strSurgeryCodes += GetXMLElementValuePair("string", pPOAEVisit->strarySurgeryCodes.GetAt(i));
		}
		strAE += GetXMLElementValuePair_Embedded("SurgeryCodes", strSurgeryCodes);
	}

	return strAE;
}

// (j.gruber 2010-06-01 11:56) - PLID 38954
// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
BOOL SendBoldPostOpAdverseEventVisit(long nPatientID, BOLDPostOpAEVisitInfo *pPOAEVisit, CString &strVisitID, CStringArray *straryMessages) {

	//generate the XML
	CString strXML, strPatientXML;
	strPatientXML = GetXMLElementValuePair_Embedded("PostOperativeAdverseEvent", GenerateBOLDPostOpAdverseEventsXML(pPOAEVisit));
	strXML = GetXMLElementValuePair_Embedded("request", GenerateRequestBase(nPatientID) + "\r\n" + strPatientXML);
	
	MSXML2::IXMLDOMNodePtr xmlResponse = NULL;

	if (IsProductionAccount()) {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction(GetBoldURL(),
		GetBoldURI(),
		"SavePostOperativeAdverseEvent", strXML, GetBOLDUsername(), GetBOLDPassword());
	}
	else {
		// (j.gruber 2012-09-14 11:22) - PLID 52652 - use password function
		xmlResponse = CallSoapFunction_IgnoreCertificate(GetBoldURL(),
		GetBoldURI(),
		"SavePostOperativeAdverseEvent", strXML, GetBOLDUsername(), GetBOLDPassword());
	}

	if (xmlResponse != NULL) {

		CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
		strXMLResponse.Replace("><", ">\r\n<");


		if (xmlResponse != NULL) {

			CString strXMLResponse = (LPCTSTR)xmlResponse->Getxml();
			strXMLResponse.Replace("><", ">\r\n<");

			//first get the response node
			CString strStatus, strReservationExpires, strReservationID, strVersion, strBuild, strCorrelationID;	
			
			if (! GetResponseInfo(xmlResponse, "", strStatus, straryMessages, strBuild, strCorrelationID, strReservationExpires, strReservationID, strVersion)) {
				return FALSE;
			}
		
			if (strStatus.MakeUpper() != "SUCCESS") {
				return FALSE;
			}

			//get the visitID
			/*MSXML2::IXMLDOMNodePtr xmlResult;
			xmlResult = FindChildNode(xmlResponse, "SavePreOperativeVisitResult");
			if (xmlResult) {*/
				strVisitID = GetXMLNodeText(xmlResponse, "AdverseEventID");
			//}
		}
	}
	return TRUE;

}