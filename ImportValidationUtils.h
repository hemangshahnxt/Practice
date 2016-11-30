
#ifndef IMPORTVALIDATIONUTILS_H
#define IMPORTVALIDATIONUTILS_H

#pragma once

#include "ImportUtils.h"


// (r.farnworth 2015-03-27 11:33) - PLID 65163
// (b.savon 2015-04-10 07:40) - PLID 65223 - Added record type so that validation messages are dictated by requirements per type
CString LookupErrorMessage(const CString &strColumnTitle, int nMaxLength, bool bIncludeLength, ImportRecordType irtRecordType);

// (b.savon 2015-03-20 07:22) - PLID 65153 - Add validation for Race
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateRace(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (r.goldschmidt 2016-02-09 18:27) - PLID 68163 - Add validation for Race CDC Code
bool ValidateRaceCDCCode(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
bool ValidateRacePreferredName(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn);
// (b.savon 2015-03-20 14:54) - PLID 65154 - Add validation for Ethnicity
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateEthnicity(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-23 07:32) - PLID 65155 - Add validation for Language
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateLanguage(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-23 08:13) - PLID 65156 - Add validation for Patients Current Status
bool ValidatePatientCurrentStatus(const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-23 10:18) - PLID 65157 - Add validation for Location
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateLocation(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-23 11:28) - PLID 65158 - Add validation for Referral Source Name
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateReferralSource(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-23 13:00) - PLID 65159 - Add validation for First Contact Date
bool ValidateFirstContactDate(CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-23 15:55) - PLID 65161 - Add validation for Marital Status
bool ValidateMaritalStatus(const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-24 08:15) - PLID 65160 - Modify validation Patient Last Name
bool ValidatePatientLastName(const CString &strData, const CString &strColumnTitle);
// (r.farnworth 2015-03-16 16:18) - PLID 65200 - Add validation for Resource object
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidateResourceName(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn, bool bCanExist = false);
// (r.farnworth 2015-03-20 11:12) - PLID 65240 - Add validation for Products object -- Name field and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateProductName(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn);
// (r.farnworth 2015-03-20 11:34) - PLID 65241 - Add validation for Products object -- Price field and ensure valid values are saved to data.
bool ValidateProductPrice(const CString &strData, const CString &strColumnTitle);
// (r.farnworth 2015-03-20 11:55) - PLID 65242 - Add validation for Products object -- Barcode field and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateProductBarcode(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn);
// (r.farnworth 2015-03-20 14:14) - PLID 65244 - Add validation for Products object -- On Hand Amount field and ensure valid values are saved to data.
bool ValidateProductOnHand(const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-25 13:26) - PLID 65150 - Add validation for Patients import -- Provider Name
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidatePatientProvider(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-26 13:26) - PLID 65151 - Add validation for Patients import -- Referring Physician Name
// (b.savon 2015-03-26 14:23) - PLID 65152 - Add validation for Patients import -- PCP Name 
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidatePatientReferringOrPCPName(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-03-31 11:04) - PLID 65235 - Add validation for Recalls object -- Recall date
bool ValidateRecallDate(CString &strData, const CString &strColumnTitle);
// (b.savon 2015-04-02 09:11) - PLID 65236 - Add validation for Recalls object -- Template Name
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateRecallTemplateName(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);
// (r.farnworth 2015-04-06 14:49) - PLID 65168 - Add validation for Insurance Company object -- Insurance Company Name field and ensure valid values are saved to data.
bool ValidateInsCoName(const CString &strData, const CString &strColumnTitle);
// (r.farnworth 2015-04-06 15:06) - PLID 65169 - Add validation for Insurance Company object -- Insurance Company Conversion ID field and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65859 - Use caching framework.
bool ValidateConversionID(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, NXDATALIST2Lib::_DNxDataList *pdl, const int &nColumn);
// (r.farnworth 2015-04-06 15:58) - PLID 65189 - Add validation for Insurance Company object -- Default HCFA PayerID, Default UB PayerID, and Eligbility PayerID fields and ensure valid values are saved to data.
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidatePayerIDs(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);

//(s.dhole 4/7/2015 8:38 AM ) - PLID 65229 check if priority is valid
bool ValidatePatientPriority(const CString &strData, const CString &strColumnTitle);
//(s.dhole 4/7/2015 8:51 AM ) - PLID 65227 This function  will check valid date field , if date is empty then set todays date as date
bool ValidateDate(int  nColumnField,CString &strData, const CString &strColumnTitle);
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateNoteCategory(class CCachedData &cache, const CString &strData, const CString &strColumnTitle);

// (b.savon 2015-04-07 14:59) - PLID 65218 - Add validation for Appointment Date
bool ValidateAppointmentDate(const CString &strData, const CString &strColumnTitle);
// (b.savon 2015-04-07 15:24) - PLID 65220 - Add validation for Appointment Type
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
// (r.goldschmidt 2016-01-26 19:42) - PLID 67976 - allow for forcing to conversion
// (r.goldschmidt 2016-03-15 12:34) - PLID 67976 - allow for sticking into note only
bool ValidateAppointmentType(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, bool bForceToConversion = false, bool bForceToNotes = false);
// (b.savon 2015-04-10 08:52) - PLID 65221 - Add validation for Appointment Purpose
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
// (r.goldschmidt 2016-01-26 19:42) - PLID 67976 - allow for forcing to conversion
// (r.goldschmidt 2016-03-15 12:34) - PLID 67976 - disallow when conversion is forced but not notes
bool ValidateAppointmentPurpose(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, bool bForceToConversion = false, bool bForceToNotes = false);
// (b.savon 2015-04-10 13:02) - PLID 65219 - Add validation Appointment Start Time
bool ValidateAppointmentStartEndTime(const CString &strData, const CString &strColumnTitle);
//(s.dhole 4/15/2015 2:21 PM ) - PLID 65195
bool ValidateInsuredPartyRespoType(const CString &strData, const CString &strColumnTitle, const NXDATALIST2Lib::IRowSettingsPtr &pRow, NXDATALIST2Lib::_DNxDataListPtr &pRecordList, CSqlFragment &PatientSql);
//(s.dhole 4/15/2015 2:25 PM ) - PLID 65196
bool ValidateInsuredPartyRelations(const CString &strData, const CString &strColumnTitle);
// (v.maida 2015-05-06 16:33) - PLID 65860 - Use caching framework.
bool ValidateInsuredPartyInsuComp(class CCachedData &cache, const CString &strData, const CString &strColumnTitle, const NXDATALIST2Lib::IRowSettingsPtr &pRow, NXDATALIST2Lib::_DNxDataListPtr &pRecordList);
// (r.farnworth 2015-04-29 10:51) - PLID 65525 - Ensure boolean values are validated before the preview screen silently changes them to false if they are invalid
bool ValidateBooleanValue(const CString &strData, const CString &strColumnTitle);
//(s.dhole 5/1/2015 2:15 PM ) - PLID  65755 
bool ValidateCopayPercent(const CString &strData, const CString &strColumnTitle);
bool ValidateInsuredPartyInActiveDate(const CString &strData, const CString &strColumnTitle, const NXDATALIST2Lib::IRowSettingsPtr &pRow, NXDATALIST2Lib::_DNxDataListPtr &pRecordList);
bool ValidateBirthDate(const CString &strData, const CString &strColumnTitle);



#endif