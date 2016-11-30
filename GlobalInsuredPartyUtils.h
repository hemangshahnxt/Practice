//GlobalInsuredPartyUtils.h
//

#ifndef GLOBAL_INSURED_PARTY_UTILS_H
#define GLOBAL_INSURED_PARTY_UTILS_H

#pragma once

// (r.goldschmidt 2014-07-24 11:24) - PLID 62775 - need to add pay group list
struct InsuredPartyPayGroupValues
{

	CString m_strName;
	long m_nCoInsPercent; // when entered into datalist2, var is enforced to be empty/null or >= 0 and <= 100.
	COleCurrency m_cyCopayMoney; // when entered into datalist2, var is enforced to be empty/null or >= 0.
	long m_nCopayPercent; // when entered into datalist2, var is enforced to be empty/null or >= 0 and <= 100.
	COleCurrency m_cyTotalDeductible; // when entered into datalist2, var is enforced to be empty/null or >= 0.
	COleCurrency m_cyTotalOOP; // when entered into datalist2, var is enforced to be empty/null or >= 0.

	bool isCoInsPercentValid();

	bool isCopayMoneyValid();

	bool isCopayPercentValid();

	bool isTotalDeductibleValid();

	bool isTotalOOPValid();

	// default constructor, initialize to all invalid values (this is required)
	InsuredPartyPayGroupValues();

};


// (d.thompson 2009-03-19) - PLID 33590
// (j.gruber 2009-10-12 13:40) - PLID 10723 - added optional demographic fields
// (j.jones 2012-10-25 09:35) - PLID 36305 - added Title
// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
// (r.goldschmidt 2014-07-24 16:33) - PLID 63111- added insurance pay group information
// (r.gonet 2015-11-12 11:14) - PLID 66907 - New optional parameter bUpdatePatientInHL7.
bool CreateNewInsuredPartyRecord(IN long nPatientPersonID, IN CString strPatientName,
								 OUT long &nNewPersonID, OUT _variant_t &varNewPlanID, OUT long &nRespTypeID, OUT long &nInsCoPersonID,
								 OPTIONAL IN long nOverrideInsCoPersonID = -1, OPTIONAL IN long nOverrideInsPlanID = -1,
								 OPTIONAL IN long nOverrideRespTypeID = -2,
								 OPTIONAL IN CString strInsPartyIDForIns = "",
								 OPTIONAL IN CString strInsPartyPolicyGroupNum = "",
								 OPTIONAL IN bool bPerPayGroup = false,
								 OPTIONAL IN COleCurrency cyTotalDeductible = InsuredPartyPayGroupValues().m_cyTotalDeductible, // hacky way of sticking an invalid currency in here
								 OPTIONAL IN COleCurrency cyTotalOOP = InsuredPartyPayGroupValues().m_cyTotalOOP, // hacky way of sticking an invalid currency in here
								 OPTIONAL IN const CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues> &mapInsPartyPayGroupValues = CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues>(10),
								 OPTIONAL IN CString strInsPartyRelationToPatient = "",
								 OPTIONAL IN CString strInsPartyFirst = "",
								 OPTIONAL IN CString strInsPartyMiddle = "",
								 OPTIONAL IN CString strInsPartyLast = "",
								 OPTIONAL IN CString strInsPartyTitle = "",
								 OPTIONAL IN CString strInsPartyAddress1 = "",
								 OPTIONAL IN CString strInsPartyAddress2 = "",
								 OPTIONAL IN CString strInsPartyCity = "",
								 OPTIONAL IN CString strInsPartyState = "",
								 OPTIONAL IN CString strInsPartyZip = "",
								 OPTIONAL IN CString strInsPartyCountry = "",
								 OPTIONAL IN CString strInsPartyPhone = "",
								 OPTIONAL IN CString strInsPartyEmployerSchool = "",
								 OPTIONAL IN long nInsPartyGender = 0,
								 OPTIONAL IN COleDateTime dtInsPartyBirthDate = COleDateTime(0,0,0,0,0,0),
								 OPTIONAL IN CString strInsPartySSN = "",
								 OPTIONAL IN bool bUpdatePatientHL7 = true);
								 


// (d.thompson 2009-03-19) - PLID 33590
// (r.gonet 2015-11-12 11:14) - PLID 66907 - New optional parameter bUpdatePatientInHL7.
bool CopyPatientInfoToInsuredParty(IN long nPatientPersonID, IN long nInsPartyPersonID, IN CString strPatientName, OPTIONAL IN bool bUpdatePatientHL7 = true);

//r.wilson (8/23/2012) PLID 52222 
void InsertDefaultDeductibles(long nInsuranceCoID, long nInsuredParty);

// (r.goldschmidt 2014-07-30) - PLID 63111
void CreateInsuredPartyPayGroups(long nInsuredPartyID, bool bPerPayGroup,
	const CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues> &mapInsPartyPayGroupValues);

//global enumerations for identical pay group lists that
//exist in multiple places in the UI
enum PayGroupsColumn_Global {
	gpgcID = 0,
	gpgcPayGroupID,
	gpgcName,
	gpgcCoIns,
	gpgcCopayMoney,
	gpgcCopayPercent,
};

//The UI to edit pay groups and immediately save is in multiple places in the program.
//These functions ensure those places behave identically.
void OnEditingStartingPayGroupList_Global(CWnd *pParentWnd, NXDATALIST2Lib::_DNxDataListPtr &pPayGroupList, NXDATALIST2Lib::IRowSettingsPtr &pRow,
	short nCol, VARIANT* pvarValue, BOOL* pbContinue);
void OnEditingFinishingPayGroupList_Global(CWnd *pParentWnd, NXDATALIST2Lib::_DNxDataListPtr &pPayGroupList, NXDATALIST2Lib::IRowSettingsPtr &pRow,
	short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
void OnEditingFinishedPayGroupList_Global(CWnd *pParentWnd, NXDATALIST2Lib::_DNxDataListPtr &pPayGroupList, NXDATALIST2Lib::IRowSettingsPtr &pRow,
	long nPatientID, CString strPatientName, long nInsuredPartyID,
	short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);

//global functions to update pay group information
void UpdatePayGroupRecord(CString strField, CString strType, _variant_t varValue, long nPayGroupID, long nPatientID, CString strPatientName, long nInsuredPartyID, long aeiAudit, CString strOldVal, CString strPayGroup);
void InsertPayGroupRecord(NXDATALIST2Lib::IRowSettingsPtr &pRow, CString strField, CString strType, _variant_t varValue, long nPayGroupID, long nPatientID, CString strPatientName, long nInsuredPartyID, long aeiAudit);
bool CanDeletePayGroup(NXDATALIST2Lib::IRowSettingsPtr &pRow, short nCol);
void DeletePayGroupRow(NXDATALIST2Lib::IRowSettingsPtr &pRow, long nPayGroupID, long nPatientID, CString strPatientName, long nInsuredPartyID, long aeiAudit, CString strOldVal);
bool PayGroupValuesChanged(_variant_t varOldValue, _variant_t varNewValue);

#endif	//GLOBAL_INSURED_PARTY_UTILS_H