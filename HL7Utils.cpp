// (r.gonet 12/03/2012) - PLID 53798 - Moved export related functions to NxHL7Lib

//HL7Utils.cpp
#include "stdafx.h"
#include "HL7Utils.h"
#include "GlobalDataUtils.h"
#include "Contacts.h"
#include "HL7DuplicateRefPhysDlg.h"
#include "HL7ParseUtils.h"
#include "BillingDlg.h"
#include "PatientView.h"
#include "HL7SelectPatientDlg.h"
#include "FileUtils.h"
#include "SelectDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "HistoryUtils.h"
#include "SingleSelectDlg.h"
#include "TodoUtils.h"
#include "Base64.h" 
#include "WellnessDataUtils.h"
#include "DecisionRuleUtils.h"
#include "PPCLink.h"
#include "HL7SelectInsCoDlg.h"
#include "MsgBox.h"
#include "LabCustomField.h"
#include <NxHL7Lib/HL7DataUtils.h>
#include "ImportUtils.h" // (j.dinatale 2010-09-01) - PLID 39609 - Need this for phone number formatting
#include <NxHL7Lib/HL7CommonUtils.h>
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed
#include "GlobalUtils.h"
#include "GlobalNexWebUtils.h"
#include <NxHL7Lib/HL7Logging.h>
#include <HL7SettingsCache.h>
#include <NxXMLUtils.h>
#include <OpticalUtils.h>
#include <NxSystemUtilitiesLib\NxConnectionUtils.h>

// (b.spivey, October 22, 2012) - PLID 53040 - 
#import "PDFCreatorPilot.tlb"
#include "NxCDO.h"
#include "DiagCodeInfo.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers
using std::vector;
using std::pair;
using std::set;

CHL7SettingsCache *g_pHL7SettingsCache = NULL;

//TES 6/22/2011 - PLID 44261 - Functions for interacting with the global CHL7SettingsCache
void EnsureHL7SettingsCache()
{
	if(g_pHL7SettingsCache == NULL) {
		g_pHL7SettingsCache = new CHL7SettingsCache(GetRemoteData());
	}
}

void DestroyHL7SettingsCache()
{
	if(g_pHL7SettingsCache) {
		delete g_pHL7SettingsCache;
	}
	g_pHL7SettingsCache = NULL;
}

// (r.gonet 2015-02-17 17:11) - PLID 64642 - Defines a library extern to save an HL7 error to a file.
void LogHL7ErrorToFile(LPCTSTR szFormat, ...)
{
	CString strText;
	va_list argList;
	va_start(argList, szFormat);
	strText.FormatV(szFormat, argList);
	va_end(argList);

	Log(strText);
}

CString GetHL7SettingText(long nHL7GroupID, const CString &strSetting)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and load the setting from it
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->GetSettingText(nHL7GroupID, strSetting);
}

long GetHL7SettingInt(long nHL7GroupID, const CString &strSetting)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and load the setting from it
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->GetSettingInt(nHL7GroupID, strSetting);
}

BOOL GetHL7SettingBit(long nHL7GroupID, const CString &strSetting)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and load the setting from it
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->GetSettingBit(nHL7GroupID, strSetting);
}

void SetHL7SettingText(long nHL7GroupID, const CString &strSetting, const CString &strValue, OPTIONAL IN OUT CString *pstrSql /*= NULL*/, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and tell it to update the setting
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->SetSettingText(nHL7GroupID, strSetting, strValue, pstrSql, paryParams);
}

void SetHL7SettingInt(long nHL7GroupID, const CString &strSetting, long nValue, OPTIONAL IN OUT CString *pstrSql /*= NULL*/, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and tell it to update the setting
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->SetSettingInt(nHL7GroupID, strSetting, nValue, pstrSql, paryParams);
}

void SetHL7SettingBit(long nHL7GroupID, const CString &strSetting, BOOL bValue, OPTIONAL IN OUT CString *pstrSql /*= NULL*/, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and tell it to update the setting
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->SetSettingBit(nHL7GroupID, strSetting, bValue, pstrSql, paryParams);
}

CString GetHL7GroupName(long nHL7GroupID)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and load the setting from it
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->GetGroupName(nHL7GroupID);
}

LabType GetHL7LabProcedureType(long nHL7GroupID)
{
	//TES 6/22/2011 - PLID 44261 - Make sure our cache exists, and load the setting from it
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache->GetLabProcedureType(nHL7GroupID);
}

void GetOBR24Values(long nHL7GroupID, CStringArray &saOBR24Values) 
{
	EnsureHL7SettingsCache();
	//TES 6/27/2011 - PLID 44261 - Moved the code to HL7SettingsCache
	g_pHL7SettingsCache->GetOBR24Values(nHL7GroupID, saOBR24Values);
}

//TES 6/22/2011 - PLID 44261 - Flag an HL7Settings group as needing to be reloaded from data
void RefreshHL7Group(long nHL7GroupID)
{
	//TES 6/22/2011 - PLID 44261 - Tell the cache
	EnsureHL7SettingsCache();
	g_pHL7SettingsCache->UnloadGroup(nHL7GroupID);
}

//TES 6/22/2011 - PLID 44261 - Loads all settings groups that have the specified value for the specified setting
void GetHL7SettingsGroupsBySetting(const CString &strSetting, const CString &strValue, OUT CArray<long,long> &arGroups)
{
	EnsureHL7SettingsCache();
	g_pHL7SettingsCache->GetSettingsGroupsBySetting(strSetting, strValue, arGroups);
}

void GetHL7SettingsGroupsBySetting(const CString &strSetting, long nValue, OUT CArray<long,long> &arGroups)
{
	EnsureHL7SettingsCache();
	g_pHL7SettingsCache->GetSettingsGroupsBySetting(strSetting, nValue, arGroups);
}

void GetHL7SettingsGroupsBySetting(const CString &strSetting, BOOL bValue, OUT CArray<long,long> &arGroups)
{
	EnsureHL7SettingsCache();
	g_pHL7SettingsCache->GetSettingsGroupsBySetting(strSetting, bValue, arGroups);
}

//TES 6/22/2011 - PLID 44261 - Get all settings groups
void GetAllHL7SettingsGroups(OUT CArray<long,long> &arGroups)
{
	EnsureHL7SettingsCache();
	g_pHL7SettingsCache->GetAllSettingsGroups(arGroups);
}

CString GetMessageHeader(long nHL7GroupID)
{
	//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
	return GetHL7SettingText(nHL7GroupID, "ExportBeginChars");
}

CString GetMessageFooter(long nHL7GroupID)
{
	//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
	return GetHL7SettingText(nHL7GroupID, "ExportEndChars");
}

// (d.thompson 2012-08-28) - PLID 52129
//	Please make sure the given recordset has a PatientID column in your SELECT clause, this field will be
//	used to decide who needs an HL7 message built.
// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - updating needs to know if this is an attempted mass insurance update
void UpdateExistingHL7PatientsByRecordset(ADODB::_RecordsetPtr prs, bool bMassInsuranceUpdate /* = false */)
{
	if(!g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
		return;
	}

	//TES 6/7/2013 - PLID 55732 - Since we're sending a bunch of requests at once, group them all under the same GUID so that if they all fail, 
	// the user is only notified once.  Note that z.manning's comment below won't work if the messages are being sent asynchronously.
	CString strRequestGroupGUID = NewUUID(true).Left(32);
	//TES 11/12/2015 - PLID 67500 - Add the IDs to an array, and send them all at once
	CArray<long, long> arPatientIDs;
	for(; !prs->eof; prs->MoveNext()) {
		arPatientIDs.Add(AdoFldLong(prs->GetFields(), "PatientID"));
	}
	// (r.goldschmidt 2016-02-04 16:10) - PLID 68161 - add bool for mass insurance update
	UpdateMultipleExistingPatientsInHL7(arPatientIDs, TRUE, false, strRequestGroupGUID, racUnchanged, false, bMassInsuranceUpdate);
}

// (d.thompson 2012-08-28) - PLID 52129 - Shared by the below functions to update hl7 data.  Please
//	make sure the query passed in contains a PatientID field in the SELECT clause.
// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - updating needs to know if this is an attempted mass insurance update
void UpdateExistingHL7PatientsBySql(CSqlFragment sql, bool bMassInsuranceUpdate /* = false */)
{
	//shortcut to avoid a query if not licensed
	if(!g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
		return;
	}

	_RecordsetPtr prs = CreateParamRecordset(sql);
	UpdateExistingHL7PatientsByRecordset(prs, bMassInsuranceUpdate);
}

// (d.thompson 2012-08-28) - PLID 52129 - Similar, but also tie in the insurance plan field.  We only need update
//	patients with this plan, not all patients, if the plan changes.
//If nInsPlanID is < 0, we will only use the ins co id to filter.  It is not possible to use this
//	function to only update patients without a plan.
void UpdateExistingHL7PatientsByInsCoIDAndPlan(const long nInsCoID, const long nInsPlanID)
{
	CWaitCursor wc;
	try
	{
		CSqlFragment sql("SELECT PatientID FROM InsuredPartyT WHERE InsuranceCoID = {INT} ", nInsCoID);

		//If there's a plan, use it.
		if(nInsPlanID >= 0) {
			sql += CSqlFragment(" AND InsPlan = {INT}", nInsPlanID);
		}
		else {
			//No plan, we then only filter on InsuranceCoID
		}

		// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - updating needs to know if this is an attempted mass insurance update
		UpdateExistingHL7PatientsBySql(sql, true);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-01-08 17:03) - PLID 32663 - Update multiple patients at once for a given insurance
// company ID
// (d.thompson 2012-08-28) - PLID 52129 - Merged with above UpdateExistingHL7PatientsByRecordset()
void UpdateExistingHL7PatientsByInsCoID(const long nInsCoID)
{
	CWaitCursor wc;
	try	{
		UpdateExistingHL7PatientsByInsCoIDAndPlan(nInsCoID, -1);
	} NxCatchAll("HL7Utils::UpdateExistingHL7PatientsByInsCoID");
}

// (b.eyers 2015-11-12) - PLID 66977
void UpdateExistingHL7PatientsByInsContact(const long nContactID)
{
	CWaitCursor wc;
	try {
		CSqlFragment sql("SELECT PatientID FROM InsuredPartyT WHERE InsuranceContactID = {INT}", nContactID);
		UpdateExistingHL7PatientsBySql(sql, true);
	} NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-02-04 16:10) - PLID 68161 - split out logic for selecting the HL7SettingsGroups due for updating
// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not return links to anybody but Intellechart
void SelectHL7SettingsGroups(CArray<HL7SettingsGroup, HL7SettingsGroup&> &arDefaultGroups, bool bOnlyUpdateInsuranceHL7Groups, bool bNewPatient, bool bMassInsuranceUpdate, bool bOnlyUpdateIntellechart)
{
	//TES 6/22/2011 - PLID 44261 - New way of accessing HL7 Settings
	CArray<long, long> arDefaultGroupIDs;
	// (z.manning 2011-08-05 12:21) - PLID 40872 - We have different settings to check for new vs. existing patients.
	CString strPatientHL7Setting = bNewPatient ? "ExportNewPatients" : "ExportUpdatedPatients";
	GetHL7SettingsGroupsBySetting(strPatientHL7Setting, TRUE, arDefaultGroupIDs);

	for (int i = 0; i < arDefaultGroupIDs.GetSize(); i++)
	{
		long nGroupID = arDefaultGroupIDs[i];

		bool bExportGroup = false;

		// (z.manning 2009-01-08 15:26) - PLID 32663 - Honor the option to only update HL7 groups
		// that export insurance info.
		// (r.goldschmidt 2015-11-05 18:42) - PLID 67517 - New HL7 setting to not mass auto export ADT messages when any insurace Company specific fields are edited
		// new logic:
		//  export if: not (update is for insurance info)
		//  export if: (not a mass insurance update) and (insurance info gets put in hl7 message [setting])
		//  export if: (auto update insurance changes [setting]) and (insurance info gets put in hl7 message [setting])
		if (!bOnlyUpdateInsuranceHL7Groups || ((!bMassInsuranceUpdate || GetHL7SettingBit(nGroupID, "AutoExportInsurance")) && GetHL7SettingBit(nGroupID, "ExportInsurance"))) {
			bExportGroup = true;
		}
		
		// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not return links to anybody but Intellechart
		if (bExportGroup && bOnlyUpdateIntellechart && !GetHL7SettingBit(nGroupID, "EnableIntelleChart")) {
			//this message is for intellechart only, but this group doesn't have it enabled
			bExportGroup = false;
		}

		if (bExportGroup) {
			HL7SettingsGroup hsg;
			hsg.nID = nGroupID;
			hsg.nExportType = GetHL7SettingInt(nGroupID, "ExportType");
			hsg.bExpectAck = GetHL7SettingBit(nGroupID, "ExpectAck");
			hsg.strName = GetHL7GroupName(nGroupID);
			arDefaultGroups.Add(hsg);
		}

		
	}
}

// (z.manning 2009-01-08 15:12) - PLID 32663 - Added a function to handle the entire process of 
// updating a patient in HL7. Returns true if export is successful and false if it failed.
// (z.manning 2011-08-05 12:14) - PLID 40872 - Added param to send a new patient message instead
//TES 6/7/2013 - PLID 55732 - Added strRequestGroupGUID.  If it is non-empty, and if an error occurs for this message, only one error will be reported to the
// user per RequestGroupGUID.
// (b.eyers 2015-06-22) - PLID - ability to send ROL action code
//TES 9/28/2015 - PLID 66192 - Added bSendPrimaryImage
// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - updating needs to know if this is part of an attempted mass insurance update; added bMassInsuranceUpdate
// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not update any HL7 link except Intellechart
BOOL UpdateExistingPatientInHL7(const long nPatientID, bool bOnlyUpdateInsuranceHL7Groups /* = false */, bool bNewPatient /* = false */, const CString &strRequestGroupGUID /* = "" */, HL7ROLActionCodes eROLCode /*= racUnchanged */, bool bSendPrimaryImage /*= false*/, bool bMassInsuranceUpdate /* = false */, bool bOnlyUpdateIntellechart /*= false*/)
{
	BOOL bSuccess = TRUE;
	//make sure that they have the license
	if(g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
	{
		// (r.gonet 12/04/2012) - PLID 54105 - Eliminated creating the socket since we'll do that
		//  in a deeper function.
		// (j.gruber 2007-08-27 09:39) - PLID 24628 - used for updating HL7
		CArray<HL7SettingsGroup,HL7SettingsGroup&> arDefaultGroups;

		// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
		// so if Intellechart is not required, require it now
		if (bSendPrimaryImage && !bOnlyUpdateIntellechart) {
			//devs should fix the caller to set bOnlyUpdateIntellechart to true
			ASSERT(FALSE);
			bOnlyUpdateIntellechart = true;
		}

		// (r.goldschmidt 2016-02-04 16:10) - PLID 68161 - split out logic for selecting the HL7SettingsGroups due for updating
		// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not update any HL7 link except Intellechart
		SelectHL7SettingsGroups(arDefaultGroups, bOnlyUpdateInsuranceHL7Groups, bNewPatient, bMassInsuranceUpdate, bOnlyUpdateIntellechart);

		if(arDefaultGroups.GetSize()) {
			// (r.gonet 12/04/2012) - PLID 54105 - Get the patient's status since we're not going to 
			//  be getting the whole demographics.
			_RecordsetPtr rsPatients = CreateParamRecordset(
				"SELECT CurrentStatus "
				"FROM PatientsT "
				"WHERE PersonID = {INT}; ",
				nPatientID);
			// (e.frazier 2016-06-24 10:27) - NX-100827 - Throw an exception if rsPatients contains no data
			BYTE nCurrentStatus;
			if (rsPatients->eof)
			{
				ThrowNxException("Error in HL7Utils::UpdateExistingPatientInHL7 - No patient exists for the given patient ID");
			}
			else
			{
				nCurrentStatus = AdoFldByte(rsPatients, "CurrentStatus");
			}
			for(int i = 0; i < arDefaultGroups.GetSize(); i++) {
				HL7SettingsGroup hsg = arDefaultGroups[i];
				BOOL bExpectAck = (hsg.nExportType == 1 && hsg.bExpectAck);
				//TES 9/21/2010 - PLID 40595 - If we're excluding prospects, and this patient is a prospect, then silently skip the export.
				//  I'm not adding this setting to HL7SettingsGroup, as it's been superceded by the global CHL7Settings cache.
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				if(!GetHL7SettingBit(hsg.nID, "ExcludeProspects") || nCurrentStatus != 2) {
					// (r.gonet 12/11/2012) - PLID 54105 - Get a response from the NxServer for these exports.
					HL7ResponsePtr pResponse;
					if(bNewPatient) {
						// (z.manning 2011-08-05 12:17) - PLID 40872 - We can now optionally send a new patient message instead.
						// (r.gonet 12/03/2012) - PLID 54104 - Updated to use the new send function.
						//TES 9/28/2015 - PLID 66192 - Added bSendPrimaryImage
						pResponse = GetMainFrame()->GetHL7Client()->SendNewPatientHL7Message(nPatientID, hsg.nID, true, strRequestGroupGUID, bSendPrimaryImage);
					}
					else {
						// (r.gonet 12/03/2012) - PLID 54105 - Updated to use the new send function.
						// (b.eyers 2015-06-22) - PLID 66213
						//TES 9/28/2015 - PLID 66192 - Added bSendPrimaryImage
						pResponse = GetMainFrame()->GetHL7Client()->SendUpdatePatientHL7Message(nPatientID, hsg.nID, strRequestGroupGUID, true, eROLCode, bSendPrimaryImage);
					}

					// (r.gonet 12/03/2012) - PLID 54105 - If the status says the send failed, then let the caller know so it can terminate any running batch.
					if(pResponse->hmssSendStatus == hmssFailure_NotBatched || pResponse->hmssSendStatus == hmssFailure_Batched || pResponse->hmssSendStatus == hmssSent_AckFailure) {
						bSuccess = FALSE;
					}
				}
			}
		}
	}

	return bSuccess;
}

//TES 11/12/2015 - PLID 67500 - New version for mass exports
// (r.goldschmidt 2016-02-03 16:10) - PLID 68162 -  add bool for mass insurance update
// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not update any HL7 link except Intellechart
BOOL UpdateMultipleExistingPatientsInHL7(const CArray<long, long> &arPatientIDs, bool bOnlyUpdateInsuranceHL7Groups /* = false */, bool bNewPatient /* = false */, const CString &strRequestGroupGUID /* = "" */, HL7ROLActionCodes eROLCode /*= racUnchanged */, bool bSendPrimaryImage /*= false*/, bool bMassInsuranceUpdate /* = false */, bool bOnlyUpdateIntellechart /*= false*/)
{
	BOOL bSuccess = TRUE;
	//make sure that they have the license
	if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
	{
		// (r.gonet 12/04/2012) - PLID 54105 - Eliminated creating the socket since we'll do that
		//  in a deeper function.
		// (j.gruber 2007-08-27 09:39) - PLID 24628 - used for updating HL7
		CArray<HL7SettingsGroup, HL7SettingsGroup&> arDefaultGroups;

		// (j.jones 2016-04-06 14:03) - NX-100095 - currently sending images is only for Intellechart,
		// so if Intellechart is not required, require it now
		if (bSendPrimaryImage && !bOnlyUpdateIntellechart) {
			//devs should fix the caller to set bOnlyUpdateIntellechart to true
			ASSERT(FALSE);
			bOnlyUpdateIntellechart = true;
		}

		// (r.goldschmidt 2016-02-03 16:10) - PLID 68162 - split out logic for selecting the HL7SettingsGroups due for updating
		// (j.jones 2016-04-06 13:48) - NX-100095 - added bOnlyUpdateIntellechart, will not update any HL7 link except Intellechart
		SelectHL7SettingsGroups(arDefaultGroups, bOnlyUpdateInsuranceHL7Groups, bNewPatient, bMassInsuranceUpdate, bOnlyUpdateIntellechart);

		if (arDefaultGroups.GetSize()) {
			for (int i = 0; i < arDefaultGroups.GetSize(); i++) {
				HL7SettingsGroup hsg = arDefaultGroups[i];
				BOOL bExpectAck = (hsg.nExportType == 1 && hsg.bExpectAck);
				long nUpdatePatientIDs[HL7_MULTIPLE_RECORD_LIMIT];
				ZeroMemory(nUpdatePatientIDs, HL7_MULTIPLE_RECORD_LIMIT*sizeof(long));
				long nUpdatePatientCount = 0;
				for (int nPatient = 0; nPatient < arPatientIDs.GetCount(); nPatient++) {
					long nPatientID = arPatientIDs[nPatient];
					// (r.gonet 12/04/2012) - PLID 54105 - Get the patient's status since we're not going to 
					//  be getting the whole demographics.
					_RecordsetPtr rsPatients = CreateParamRecordset(
						"SELECT CurrentStatus "
						"FROM PatientsT "
						"WHERE PersonID = {INT}; ",
						nPatientID);
					BYTE nCurrentStatus = AdoFldByte(rsPatients, "CurrentStatus");
					//TES 9/21/2010 - PLID 40595 - If we're excluding prospects, and this patient is a prospect, then silently skip the export.
					//  I'm not adding this setting to HL7SettingsGroup, as it's been superceded by the global CHL7Settings cache.
					//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
					if (!GetHL7SettingBit(hsg.nID, "ExcludeProspects") || nCurrentStatus != 2) {
						// (r.gonet 12/11/2012) - PLID 54105 - Get a response from the NxServer for these exports.
						HL7ResponsePtr pResponse;
						nUpdatePatientIDs[nUpdatePatientCount] = nPatientID;
						nUpdatePatientCount++;
						if (nUpdatePatientCount == HL7_MULTIPLE_RECORD_LIMIT) {
							// (z.manning 2011-08-05 12:17) - PLID 40872 - We can now optionally send a new patient message instead.
							// (r.gonet 12/03/2012) - PLID 54104 - Updated to use the new send function.
							//TES 9/28/2015 - PLID 66192 - Added bSendPrimaryImage
							if (bNewPatient) {
								pResponse = GetMainFrame()->GetHL7Client()->SendMultipleNewPatientHL7Message(nUpdatePatientIDs, hsg.nID, true, strRequestGroupGUID, bSendPrimaryImage);
							}
							else {
								pResponse = GetMainFrame()->GetHL7Client()->SendMultipleUpdatePatientHL7Message(nUpdatePatientIDs, hsg.nID, strRequestGroupGUID, true, eROLCode, bSendPrimaryImage);
							}
							if (pResponse->hmssSendStatus == hmssFailure_NotBatched || pResponse->hmssSendStatus == hmssFailure_Batched || pResponse->hmssSendStatus == hmssSent_AckFailure) {
								bSuccess = FALSE;
							}
							ZeroMemory(nUpdatePatientIDs, HL7_MULTIPLE_RECORD_LIMIT*sizeof(long));
							nUpdatePatientCount = 0;
						}
					}
				}
				if (nUpdatePatientCount > 0) {
					HL7ResponsePtr pResponse;
					if (bNewPatient) {
						pResponse = GetMainFrame()->GetHL7Client()->SendMultipleNewPatientHL7Message(nUpdatePatientIDs, hsg.nID, true, strRequestGroupGUID, bSendPrimaryImage);
					}
					else {
						pResponse = GetMainFrame()->GetHL7Client()->SendMultipleUpdatePatientHL7Message(nUpdatePatientIDs, hsg.nID, strRequestGroupGUID, true, eROLCode, bSendPrimaryImage);
					}
					if (pResponse->hmssSendStatus == hmssFailure_NotBatched || pResponse->hmssSendStatus == hmssFailure_Batched || pResponse->hmssSendStatus == hmssSent_AckFailure) {
						bSuccess = FALSE;
					}
				}
			}
		}
	}

	return bSuccess;
}

// (v.maida 2014-12-23 12:19) - PLID 64472 - Add or update referring physicians in HL7.
BOOL AddOrUpdateRefPhysInHL7(const long nRefPhysID, bool bNewRefPhys, const CString &strRequestGroupGUID)
{
	BOOL bSuccess = TRUE;
	//make sure that they have the license
	if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
	{
		//TES 6/22/2011 - PLID 44261 - New way of accessing HL7 Settings
		CArray<long, long> arDefaultGroupIDs;

		CString strRefPhysHL7Setting = bNewRefPhys ? "EXPORTNEWREFPHYS" : "EXPORTUPDATEDREFPHYS";
		GetHL7SettingsGroupsBySetting(strRefPhysHL7Setting, TRUE, arDefaultGroupIDs);

		for (int i = 0; i < arDefaultGroupIDs.GetSize(); i++)
		{
			HL7ResponsePtr pResponse;
			if (bNewRefPhys) {
				if (GetHL7SettingBit(arDefaultGroupIDs[i], "EXPORTNEWREFPHYS")) {
					pResponse = GetMainFrame()->GetHL7Client()->SendReferringPhysicianMessage(nRefPhysID, arDefaultGroupIDs[i], false);
				}
			}
			else { // updating an existing referring physician, if they have that HL7 setting checked.
				if (GetHL7SettingBit(arDefaultGroupIDs[i], "EXPORTUPDATEDREFPHYS")) {
					pResponse = GetMainFrame()->GetHL7Client()->SendReferringPhysicianMessage(nRefPhysID, arDefaultGroupIDs[i], false);
				}
			}
			// (r.gonet 12/03/2012) - PLID 54105 - If the status says the send failed, then let the caller know so it can terminate any running batch.
			if (pResponse->hmssSendStatus == hmssFailure_NotBatched || pResponse->hmssSendStatus == hmssFailure_Batched || pResponse->hmssSendStatus == hmssSent_AckFailure) {
				bSuccess = FALSE;
			}
		}
	}

	return bSuccess;
}

// (s.tullis 2015-06-23 14:17) - PLID 66197 - Added Function to send PatientReminder Updates to EnableIntelleChart Links
BOOL SendPatientReminderHL7Message(const long nPatientID, const long nReminderID){
	BOOL bSuccess = TRUE;
	//make sure that they have the license
	if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
	{
		CArray<long, long> arDefaultGroupIDs;

		GetHL7SettingsGroupsBySetting("EnableIntelleChart", TRUE, arDefaultGroupIDs);

		for (int i = 0; i < arDefaultGroupIDs.GetSize(); i++)
		{
			HL7ResponsePtr pResponse;
			if (GetHL7SettingBit(arDefaultGroupIDs[i], "EnableIntelleChart")) {
				pResponse = GetMainFrame()->GetHL7Client()->SendPatientReminderHL7Message(nPatientID, arDefaultGroupIDs[i], nReminderID);
			}
	
			if (pResponse->hmssSendStatus == hmssFailure_NotBatched || pResponse->hmssSendStatus == hmssFailure_Batched || pResponse->hmssSendStatus == hmssSent_AckFailure) {
				bSuccess = FALSE;
			}
		}
	}

	return bSuccess;
}

// (j.jones 2016-04-07 08:35) - NX-100095 - Added a function that sends HL7 messages, for IntelleChart links only,
// which include the patient's primary image. Only called when this image has changed.
BOOL SendPatientPrimaryPhotoHL7Message(const long nPatientID)
{
	//currently sending images is only for Intellechart, so stating to only export to Intellechart will skip sending a pointless update to other links
	return UpdateExistingPatientInHL7(nPatientID, false, false, "", racUnchanged, true, false, true);
}

//TODO: Change all == to .CompareNoCase as appropriate.
//TES 4/21/2008 - PLID 29721 - Added parameters to help with auditing.
// (j.jones 2008-05-05 10:09) - PLID 29600 - added bBatchImports parameter
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
// (z.manning 2010-06-28 15:15) - PLID 38896 - Added message parameter
// (a.wilson 2013-05-09 11:13) - PLID 41682 - new return value of enum
HL7AddMessageToQueueResult AddMessageToQueue(const CString &strMessage, long nHL7GroupID, const CString &strHL7GroupName, BOOL bSendHL7Tablechecker, OUT HL7Message &message)
{
	HL7AddMessageToQueueResult amtqResult;
	amtqResult.bSuccess = true;
	amtqResult.eccErrorCode = eccMessageAccepted;

	try {
		//Now, find out what event this message is for.
		
		//First, is this an acknowledgement?

		//Message is MSH-9
  		CString strMessageType;
		if(COMPONENT_FOUND != GetHL7MessageComponent(strMessage, nHL7GroupID, "MSH", 1, 9, 1, 1, strMessageType)) {
			amtqResult.bSuccess = false;
			amtqResult.eccErrorCode = eccSegmentSequenceError;
			return amtqResult;
		}
		// (r.gonet 05/01/2014) - PLID 61843 - Store the message type since we may have to filter out HL7 log entries for certain message/event types.
		if(GetHL7Transaction()) {
			GetHL7Transaction()->SetMessageType(strMessageType);
		}
		if(strMessageType == "ACK") {
			//Great.  We accept all acknowledgement messages.

			//OK, get the message id that they're acknowledging, and update messagelogt.
			CString strMessageID;
			if(COMPONENT_FOUND != GetHL7MessageComponent(strMessage, nHL7GroupID, "MSA", 1, 2, 1, 1, strMessageID)) {
				amtqResult.bSuccess = false;
				amtqResult.eccErrorCode = eccSegmentSequenceError;
				return amtqResult;
			}
			if(strMessage.GetLength() < 6 || strMessage.Left(5) != "NxMsg") {
				//This wasn't one of our messages!
				amtqResult.bSuccess = false;
				amtqResult.eccErrorCode = eccApplicationInternalError;
				return amtqResult;
			}
			//TES 8/8/2007 - Parameterized.  When I attempted to debug to this point, I found that it never gets called,
			// the proof being that it attempted to change "MessageLogT" instead of "HL7MessageLogT", which isn't a valid
			// table and yet has never caused a problem.  So, I fixed that, there's no reason this line of code won't
			// be called someday, we've just never had an appropriately set-up link.
			long nMessageID = atoi(strMessage.Mid(5));
			if(nMessageID > 0) {
				ExecuteParamSql("UPDATE HL7MessageLogT SET AcknowledgeDate = getdate() WHERE ID = {INT}", nMessageID);

				// (j.jones 2008-04-29 09:26) - PLID 29812 - added a tablechecker
				// (j.jones 2008-05-19 16:35) - PLID 30110 - don't send if the parameter tells us not to
				if(bSendHL7Tablechecker) {
					CClient::RefreshTable(NetUtils::HL7MessageLogT, nMessageID);
				}
			}
		}
		else {
			//OK, get the event.
			CString strEvent = "";
			DWORD dwReturn = GetHL7MessageComponent(strMessage, nHL7GroupID, "MSH", 1, 9, 1, 2, strEvent);
			if(dwReturn != COMPONENT_FOUND && dwReturn != COMPONENT_NOT_FOUND) {
				//There might not have been a second component to that field, and that would be fine,
				//but we have failed in some other way.
				amtqResult.bSuccess = false;
				amtqResult.eccErrorCode = eccApplicationInternalError;
				return amtqResult;
			}
			// (r.gonet 05/01/2014) - PLID 61843 - Store the event type since we may have to filter out HL7 log entries for certain message/event types.
			if(GetHL7Transaction()) {
				GetHL7Transaction()->SetEventType(strEvent);
			}
			//TES 3/10/2011 - PLID 41912 - If they're forcing the facility ID to match, pull it as well.
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			if(GetHL7SettingBit(nHL7GroupID, "ForceFacilityIDMatch")) {
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				CString strExpectedFacilityID = GetHL7SettingText(nHL7GroupID, "LabFacilityID");
				//TES 3/10/2011 - PLID 41912 - If the Facility ID is empty then we don't expect it to match.
				if(!strExpectedFacilityID.IsEmpty()) {
					CString strActualFacilityID;
					GetHL7MessageComponent(strMessage, nHL7GroupID, "MSH", 1, 5, 1, 1, strActualFacilityID);
					if(strActualFacilityID != strExpectedFacilityID) {
						//TES 3/10/2011 - PLID 41912 - 204 = "Unknown key identifier"; not quite accurate, but the closest HL7 has to offer.
						amtqResult.bSuccess = false;
						amtqResult.eccErrorCode = eccUnknownKeyIdentifier;
						// (r.gonet 05/01/2014) - PLID 61843 - Log an error
						LogHL7Error("The MSH-5 (Receiving Facility Code) field value \"%s\" in the HL7 message does not match the Sending Facility Code configured in Nextech, which is \"%s\". "
							"Nextech is rejecting the message with error code 204 (Unknown key identifier)."
							, strActualFacilityID, strExpectedFacilityID);
						// (r.gonet 05/01/2014) - PLID 61843 - Log a possible resolution
						LogHL7Resolution("If the above error just started happening and the HL7 link was functional before, "
							"check with the sender of the HL7 message about why they are sending \"%s\" rather than \"%s\" in MSH-5. "
							"It may be that the sender is sending the HL7 message to the wrong destination. "
							"\r\n\r\n"
							"Otherwise, please ensure that the HL7 link configuration is correct "
							"in Links->HL7->Configure HL7 Settings->Advanced Settings...->Sending Facility Code or consider disabling the setting "
							"\"Require matching code when importing\" in the same window if the Receiving Facility Code varies between messages "
							"or if the vendor is not filling it in."
							, strActualFacilityID, strExpectedFacilityID);
					}
				}
			}

			if(!IsHL7EventSupported(strMessageType, strEvent)) {
				amtqResult.bSuccess = false;
				amtqResult.eccErrorCode = eccEventNotSupported;//HL7-defined "event not supported" error.
			}
			//Pull out the messageid (MSH-10).
			CString strMessageID;
			if(COMPONENT_FOUND != GetHL7MessageComponent(strMessage, nHL7GroupID, "MSH", 1, 10, 1, 1, strMessageID)) {
				// (r.gonet 05/01/2014) - PLID 61843 - Log an error
				LogHL7Error("Could not find MSH-10 (Message Control ID) in the HL7 message. This field is required to be filled.");
				// (r.gonet 05/01/2014) - PLID 61843 - Log a possible resolution
				LogHL7Resolution("Please request that the sender of the HL7 message fill MSH-10 with a code unique for each HL7 message.");
				amtqResult.bSuccess = false;
				amtqResult.eccErrorCode = eccSegmentSequenceError;
				return amtqResult;
			}
			// (r.gonet 12/11/2012) - PLID 54113 - Send an ACK back using the global asynchronous CHL7Client.
			// (a.wilson 2013-05-08 17:41) - PLID 41682 kept the success value the same for passing into this function as well as the other errors.
			// CMainFrame will handle any failures.
			if(GetHL7SettingBit(nHL7GroupID, "SendACK")) {
				HL7ResponsePtr pResponse = GetMainFrame()->GetHL7Client()->SendAcknowledgementMessage(strMessageID, strEvent, nHL7GroupID, (long)amtqResult.eccErrorCode);
				if(pResponse->hmssSendStatus == hmssFailure_NotBatched) {
					amtqResult.bSuccess = false;
					amtqResult.eccErrorCode = eccApplicationInternalError;
					// (r.gonet 05/01/2014) - PLID 61843 - Log an error
					LogHL7Error("Nextech failed to send an HL7 acknowledgement receipt message to the sending party.");
					// (r.gonet 05/01/2014) - PLID 61843 - Log a possible resolution
					LogHL7Resolution("Please ensure that the sender is expecting acknowledgements. If they are not, "
						"go to Links->HL7->Configure HL7 Settings and uncheck the \"Send ACK messages (using Export settings)\" checkbox.");
					return amtqResult;
				}
			}

			//TES 10/9/2006 - PLID 22718 - Parse out the patient's name, that's always useful information.
			CString strName, strFirst, strMiddle, strLast,strDOB,strGender   ;
			GetHL7MessageComponent(strMessage, nHL7GroupID, "PID", 1, 5, 1, 2, strFirst);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "PID", 1, 5, 1, 3, strMiddle);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "PID", 1, 5, 1, 1, strLast);
			// (s.dhole 2013-08-13 12:11) - PLID 58019 Added PatientDOB and gender to parser
			GetHL7MessageComponent(strMessage, nHL7GroupID, "PID", 1, 7, 1, 1, strDOB);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "PID", 1, 8, 1, 1, strGender);
			if(strMiddle.IsEmpty()) {
				strName = strFirst + " " + strLast;
			}
			else {
				strName = strFirst + " " + strMiddle + " " + strLast;
			}

		// (s.dhole 2013-08-13 12:11) - PLID 58019 Added Parse PatientDOB
			_variant_t vNull;
			vNull.vt = VT_NULL;
			_variant_t varBirthDate;
			if(strDOB != "") {
				COleDateTime dtBirthDate;
				if(!ParseHL7Date(strDOB, dtBirthDate)) {
					varBirthDate = vNull;
				}
				else{
					varBirthDate = _variant_t(dtBirthDate,VT_DATE);
				}
			}
			else{
				varBirthDate = vNull;
			}
			// (s.dhole 2013-08-13 12:11) - PLID 58019 Added Parse PatientGender
			long nGender;
			if(strGender == "") nGender = -1;
			else if(strGender.CompareNoCase("M")==0 || strGender.CompareNoCase("Male")==0 ) nGender  = 1;
			else if(strGender.CompareNoCase("F")==0 || strGender.CompareNoCase("Female")==0 ) nGender = 2;
			else nGender  = 0;

			if(amtqResult.bSuccess == true) {
				//Now, let's actually add to our queue.
				//TES 8/9/2007 - PLID 26883 - Parameterized.
				//TES 10/16/2008 - PLID 31712 - Set the input date manually, we'll need to set it in the HL7Message struct
				COleDateTime dtInput = COleDateTime::GetCurrentTime();
				

				// (b.spivey, July 09, 2012) - PLID  49170 - Added GetRemoteData() for GetSpecificDescriptionForHL7Event() 
				// (s.dhole 2013-08-13 09:23) - PLID 58019 Added DOB and gender
				// (j.armen 2014-01-30 16:12) - PLID 60564 - Idenitate HL7MessageQueueT
				_RecordsetPtr prs = CreateParamRecordset(
					"SET NOCOUNT ON\r\n"
					"INSERT INTO HL7MessageQueueT (Message, GroupID, ActionID, MessageType, EventType, Description, PatientName, InputDate ,PatientDOB, PatientGender)\r\n"
					"VALUES ({STRING}, {INT}, NULL, {STRING}, {STRING}, {STRING}, {STRING}, {OLEDATETIME},{VT_DATE},{VT_INT})\r\n"
					"SET NOCOUNT OFF\r\n"
					"SELECT CONVERT(INT, SCOPE_IDENTITY()) AS HL7MessageQueueID",
					_T(strMessage), nHL7GroupID, _T(strMessageType), _T(strEvent), 
					_T(GetSpecificDescriptionForHL7Event(strMessage, nHL7GroupID, GetRemoteData()).Left(1000)), _T(strName.Left(255)),
					dtInput,varBirthDate,(nGender==-1?vNull:nGender));

				long nID = AdoFldLong(prs, "HL7MessageQueueID");

				//TES 4/21/2008 - PLID 29721 - Audit that we've received a message.
				// (a.walling 2010-01-25 16:33) - PLID 37023 - Must include the patient name in the old/new value since we can't display it
				CString strNewDescription;
				strNewDescription.Format("%s - %s for %s", strHL7GroupName, GetActionDescriptionForHL7Event(strMessage, nHL7GroupID), strName);
				// (a.walling 2010-01-21 14:21) - PLID 37023 - We don't even have a patient ID at this point
				AuditEvent(-1, "", BeginNewAuditEvent(), aeiHL7MessageReceived, nID, "", 
					strNewDescription, 
					aepMedium, aetCreated);

				//TES 5/8/2008 - PLID 29685 - Use the new HL7Message structure.
				message.nMessageID = nID;
				message.strMessage = strMessage;
				message.nHL7GroupID = nHL7GroupID;
				message.strHL7GroupName = strHL7GroupName;
				message.strPatientName = strName;
				//TES 10/16/2008 - PLID 31712 - Added a dtInputDate member
				message.dtInputDate = dtInput;

				//TES 4/18/2008 - PLID 29657 - Now send a tablechecker
				// (j.jones 2008-05-19 15:50) - PLID 30110 - only if our setting tells us to
				if(bSendHL7Tablechecker) {
					CClient::RefreshTable(NetUtils::HL7MessageQueueT, nID);
				}

			}
			else {
				//TES 3/10/2011 - PLID 41912 - If we return TRUE, then the file will get deleted, and be lost forever.  We do that for
				// message types we don't accept, but in this case it might just be an HL7 setting we need to change, the message could
				// be perfectly valid.
				// (a.wilson 2013-05-08 17:40) - PLID 41682 - return error code so that we can handle it for each case if necessary.
				return amtqResult;
			}
		}
		return amtqResult;
	// (r.gonet 05/01/2014) - PLID 61843 - Log any exceptions to the HL7 Log
	} NxCatchAllHL7("Error in HL7Utils::AddMessageToQueue()");
	
	amtqResult.bSuccess = false;
	amtqResult.eccErrorCode = eccApplicationInternalError;
	return amtqResult;
}

//TES 4/18/2008 - PLID 29721 - Moved GetActionDescriptionForEvent() and GetSpecificDescriptionForEvent() to HL7ParseUtils.

//TES 4/21/2008 - PLID 29721 - Added parameters to help with auditing.
//TES 5/8/2008 - PLID 29685 - Changed to use the new HL7Message structure.
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL CommitHL7Event(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker, IN OUT CDWordArray &arNewCDSInterventions)
{
	try {
		// (r.gonet 05/01/2014) - PLID 61843 - Store HL7 message ID so that the message may be pulled up when going over the HL7 logs.
		if(GetHL7Transaction()) {
			GetHL7Transaction()->SetHL7MessageID(Message.nMessageID);
		}
		
		//Get the message type.
		//Message is MSH-9
		CString strMessageType;
		if(COMPONENT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MSH", 1, 9, 1, 1, strMessageType)) {
			return FALSE;
		}
		// (r.gonet 05/01/2014) - PLID 61843 - Store the message type since we may have to filter out HL7 log entries for certain message/event types.
		if(GetHL7Transaction()) {
			GetHL7Transaction()->SetMessageType(strMessageType);
		}

		//We need to know the event, which is the next component.
		CString strEvent = "";
		GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MSH", 1, 9, 1, 2, strEvent);
		// (r.gonet 05/01/2014) - PLID 61843 - Store the event type since we may have to filter out HL7 log entries for certain message/event types.
		if(GetHL7Transaction()) {
			GetHL7Transaction()->SetEventType(strEvent);
		}

		BOOL bSuccessfullyCommitted = FALSE;
		//Now branch based on that.
		if(strMessageType == "ADT") {
			//Now branch based on that.
			//TES 6/13/2007 - PLID 26323 - We now also support A28 and A31, which are (for our purposes) identical 
			// to A04 and A08, respectively.
			if(strEvent == "A04" || strEvent == "A28") {//Register new patient.
				long nPatientID = -1;
				// (z.manning 2010-10-05 09:16) - PLID 40795 - We originally interpreted A04 and A28 events
				// as only being used to create new patient records. However, if you read the HL7 specs that
				// is not necessarily the case. A04s can definitely be sent in situations other than creating
				// a new patient. A28s are a bit more debatable so we added an option for that.
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				if(strEvent == "A04" || !GetHL7SettingBit(Message.nHL7GroupID, "A28AddNewOnly")) {
					HL7_PIDFields PID;
					// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
					ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData());
					FindMatchingPatient(GetRemoteData(), PID, 0, Message.nHL7GroupID, nPatientID);
				}

				// (z.manning 2010-10-05 09:19) - PLID 40795 - Only create a new patient if we did not match
				// this HL7 message to an existing patient.
				if(nPatientID == -1) {
					bSuccessfullyCommitted = AddHL7PatientToData(Message, bSendHL7Tablechecker);
				}
				else {
					bSuccessfullyCommitted = UpdateHL7Patient(Message, pMessageParent, bSendHL7Tablechecker);
				}
			}
			else if(strEvent == "A08" || strEvent == "A31") {//Update patient information.
				bSuccessfullyCommitted = UpdateHL7Patient(Message, pMessageParent, bSendHL7Tablechecker);
			}
			else {
				//Unsupported!
				bSuccessfullyCommitted = FALSE;
			}
		}
		else if(strMessageType == "DFT") {
			//We also need to pass in the HL7 version.
			CString strVersion;
			if(COMPONENT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MSH", 1, 12, 1, 1, strVersion)) {
				bSuccessfullyCommitted = FALSE;
			}
			//Now branch based on that.
			if(strEvent == "P03") {//Post transaction.
				bSuccessfullyCommitted = CreateHL7Bill(Message, pMessageParent, strVersion, bSendHL7Tablechecker);
			}
			else {
				//Unsupported!
				bSuccessfullyCommitted = FALSE;
			}
		}
		//TES 9/18/2008 - PLID 21093 - We now support ORU^R01 messages, Unsolicited Lab Results.
		else if(strMessageType == "ORU") {
			if(strEvent == "R01") {
				//TES 9/18/2008 - PLID 21093 - R01 is the only supported event type for ORU, as far as I can tell.
				// (d.singleton 2013-01-25 15:32) - PLID 54781 changed to be a more generic function that checks what type of ORU ( image vs lab result ) 
				//bSuccessfullyCommitted = ImportHL7LabResult(Message, pMessageParent, bSendHL7Tablechecker);
				//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions (this is the only message type that can trigger interventions)
				bSuccessfullyCommitted = HandleORUMessage(Message, pMessageParent, bSendHL7Tablechecker, arNewCDSInterventions);
			}
		}
		// (z.manning 2010-06-29 15:53) - PLID 34572 - We now support SIU messages (scheduler)
		else if(strMessageType == "SIU") {
			bSuccessfullyCommitted = HandleHL7SchedulerMessage(Message, pMessageParent);
		}
		//TES 5/23/2011 - PLID 41353 - Added support for referring physician messages (MFN^M02)
		else if(strMessageType == "MFN" && strEvent == "M02") {
			bSuccessfullyCommitted = HandleHL7RefPhys(Message, bSendHL7Tablechecker);
		}
		// (d.singleton 2012-10-08 17:31) - PLID 53097 Added support for notification messages (MDM^T02)
		else if(strMessageType == "MDM" && strEvent == "T02") {	
			bSuccessfullyCommitted = HandleHL7Notification(Message, pMessageParent);			
		}
		//TES 10/16/2015 - PLID 66371 - Custom type for MDI optical prescriptions
		else if (strMessageType == "ZOP" && strEvent == "Z01") {
			bSuccessfullyCommitted = ImportOpticalPrescription(Message, pMessageParent, bSendHL7Tablechecker);
		}
		else {
			//Unsupported!
			bSuccessfullyCommitted = FALSE;
		}
		
		if(bSuccessfullyCommitted) {
			//TES 5/8/2008 - PLID 29685 - The various functions (AddHL7PatientToData(), etc.) now handle all this themselves,
			// by calling FinalizeHL7Event, so there's nothing we need to do here.
		}

		return bSuccessfullyCommitted;
	// (r.gonet 05/01/2014) - PLID 61843 - Log any exceptions to the HL7 Log
	}NxCatchAllHL7("Error in HL7Utils::CommitHL7Event()");
	return FALSE;
}

// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
// (a.walling 2010-01-21 15:58) - PLID 37023
//TES 7/12/2010 - PLID 39518 - nPatientID is now passed by reference; if you pass in -1, then strSqlBatch must create a new patient,
// and put its ID in a variable named @nPatientID.  After this function is run, nPatientID will then have the value pulled from @nPatientID
//TES 7/17/2010 - PLID 39518 - nAuditTransactionID is now a pointer.  If you want the function to do all auditing itself, leave it null.
// If you have an AuditID already, pass it in, otherwise pass in a pointer to a variable set to -1, and it will begin the auditing and leave
// it up to the caller to commit it.
//TES 5/23/2011 - PLID 41353 - nPatientID can now be set to -2, meaning that this message doesn't have any patient information
//TES 6/10/2011 - PLID 41353 - Changed nPatientID to nPatientIDToAudit, and added an input parameter strNewIDVariable, representing the variable
// in the query that has been set to the new ID being created by this message, which will then be output in pnNewID
// If strNewIDVariable is non-empty, pnNewID must be non-NULL
BOOL FinalizeHL7Event(long &nPatientIDToAudit, const HL7Message &Message, CString &strSqlBatch, BOOL bSendHL7Tablechecker, CNxParamSqlArray *paryParams /*= NULL*/, long *pnAuditTransactionID /*= NULL*/, OPTIONAL IN const CString &strNewIDVariable /*= ""*/, OPTIONAL OUT long *pnNewID /*= NULL*/)
{
	long nAuditTransactionID = -1;
	bool bOwnAuditTransaction = false;
	try {
		//TES 5/8/2008 - PLID 29685 - Copied from CommitHL7Event, we need to update HL7MessageQueueT (as part of the passed-in
		// batch), plus any associated auditing/tablechecker types tuff.

		//TES 4/18/2008 - PLID 29657 - Update HL7MessageQueueT to indicate that the event has been successfully 
		// committed; this used to be done after every call to this function, now we just do it here.			
		CNxParamSqlArray aryParams;
		if(paryParams == NULL) {
			paryParams = &aryParams;
		}
		AddParamStatementToSqlBatch(strSqlBatch, *paryParams, 
			"UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}", (long)mqaCommit, Message.nMessageID);
		
		//TES 4/21/2008 - PLID 29721 - Audit that we've processed this message.
		// (z.manning 2010-08-31 17:08) - PLID 40334 - Fixed the null check here
		if(pnAuditTransactionID != NULL) {
			if(*pnAuditTransactionID == -1) {
				//TES 7/17/2010 - PLID 39518 - We're returning this ID to our caller, so we don't set bOwnAuditTransaction here.
				*pnAuditTransactionID = BeginAuditTransaction();				
			}
			nAuditTransactionID = *pnAuditTransactionID;
		}
		else {
			//TES 5/8/2008 - PLID 29685 - Since we're starting this audit transaction, we're responsible for rolling it back.
			bOwnAuditTransaction = true;
			nAuditTransactionID = BeginAuditTransaction();
		}
		//TES 5/23/2011 - PLID 41353 - If nPatientIDToAudit is -2, that means this isn't patient-related, so pass -1 as the ID
		AuditEvent(nPatientIDToAudit==-2?-1:nPatientIDToAudit, Message.strPatientName, nAuditTransactionID, aeiHL7MessageProcessed, Message.nMessageID, 
			"Pending (" + Message.strHL7GroupName + " - " + GetActionDescriptionForHL7Event(Message.strMessage, Message.nHL7GroupID) + ")", 
			"Committed", aepMedium, aetChanged);

		//TES 6/10/2011 - PLID 41353 - See if we were given a variable to output
		if(!strNewIDVariable.IsEmpty()) {
			//TES 6/10/2011 - PLID 41353 - We must have been given a place to output it.
			ASSERT(pnNewID != NULL);
			//TES 7/12/2010 - PLID 39518 - We weren't given a patient ID, so this batch must be creating it and putting the ID in @nPatientID.
			// So, we want to select that variable at the end so we can output it.
			//TES 6/10/2011 - PLID 41353 - This is now a generic output variable, not necessarily a patient ID
			AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET NOCOUNT OFF");
			AddParamStatementToSqlBatch(strSqlBatch, *paryParams, FormatString("SELECT %s AS NewID", strNewIDVariable));

			//TES 5/8/2008 - PLID 29685 - OK, we can finally commit all the passed-in statements and auditing, plus our own.
			_RecordsetPtr rsNewPersonID = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, *paryParams);
			//TES 7/12/2010 - PLID 39518 - Make sure all the recordsets execute, and pull the new patient ID out of the last one.
			_RecordsetPtr rsPrev = rsNewPersonID;
			while(rsNewPersonID = rsNewPersonID->NextRecordset(NULL)) {
				rsPrev = rsNewPersonID;
			}
			//TES 6/10/2011 - PLID 41353 - Set our output variable
			*pnNewID = AdoFldLong(rsPrev, "NewID");
		}
		else {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, *paryParams);
			CommitAuditTransaction(nAuditTransactionID);
		}


		//TES 4/18/2008 - PLID 29657 - Now send a tablechecker
		// (j.jones 2008-05-19 15:39) - PLID 30110 - only if the parameter tells us to
		if(bSendHL7Tablechecker) {
			CClient::RefreshTable(NetUtils::HL7MessageQueueT, Message.nMessageID);
		}

		return TRUE;
	// (r.gonet 05/01/2014) - PLID 61843 - Log any exceptions to the HL7 Log
	}NxCatchAllHL7SilentCallThrow({
		//TES 5/8/2008 - PLID 29685 - We need to rollback the audit transaction, if we started it, then pass on the error.
		if(bOwnAuditTransaction) RollbackAuditTransaction(nAuditTransactionID);
	});
}

//TES 7/15/2010 - PLID 39518 - Prompts the user to select a Relation To Patient to map to the given code.  If they choose one, it will
// be mapped in data (via adding statements to the passed-in batch), output in strRelation, and it will return TRUE.
// Otherwise, it will return FALSE, meaning processing should be aborted.
BOOL PromptForHL7Relation(const CString &strHL7RelationCode, long nHL7GroupID, const CString &strHL7GroupName, OUT CString &strRelation, IN OUT long &nAuditTransactionID, IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	//TES 7/13/2010 - PLID 39610 - Prompt them to select one.
	CSelectDlg dlg(NULL);
	dlg.m_strTitle = "Select Relationship";
	if(strHL7RelationCode.IsEmpty()) {
		dlg.m_strCaption = "You are attempting to import an insured party with no relationship code.  Please select the relationship for this "
			"insured party.  If you cancel this dialog, the HL7 message will not get imported.\r\n"
			"NOTE: Messages that do not include relationship codes will always require you to manually select the relationship.  Please contact "
			"your software vendor to ensure that relationship codes are always included with their insurance information.";
	}
	else {
		dlg.m_strCaption = "You are attempting to import an insured party with the relationship code '" + strHL7RelationCode + "'.\r\n"
			"Please select a relationship that should correspond with this code (you may need to contact your software vendor for assistance).\r\n\r\n"
			"If you cancel this dialog, the HL7 message will not get imported.";
	}
	CString strFromClause = "(SELECT ";
	//TES 7/13/2010 - PLID 39610 - Loop through everything in the enum (prtSelf will always be first,
	// the unused enum rtp_LastEnum will always be last).
	// (j.jones 2011-06-28 12:03) - PLID 40959 - we can no longer loop through an enum,
	// but fortunately the list is much shorter now, so just add them all individually
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpSelf, GetRelation(rtpSelf));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpChild, GetRelation(rtpChild));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpSpouse, GetRelation(rtpSpouse));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpOther, GetRelation(rtpOther));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpUnknown, GetRelation(rtpUnknown));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpEmployee, GetRelation(rtpEmployee));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpOrganDonor, GetRelation(rtpOrganDonor));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpCadaverDonor, GetRelation(rtpCadaverDonor));
	strFromClause += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpLifePartner, GetRelation(rtpLifePartner));

	//TES 7/13/2010 - PLID 39610 - Remove the last " UNION SELECT "
	strFromClause = strFromClause.Left(strFromClause.GetLength()-14);
	dlg.m_strFromClause = strFromClause + ") AS RelationsQ";
	dlg.AddColumn("ID", "ID", FALSE, FALSE);
	dlg.AddColumn("Name", "Relationship", TRUE, TRUE);
	DatalistColumn dc = dlg.m_arColumns[1];
	dc.nSortPriority = -1;
	dlg.m_arColumns.SetAt(1, dc);
	if(dlg.DoModal() == IDOK)
	{
		long nRelation = dlg.m_arSelectedValues.GetAt(0);
		strRelation = GetRelation((RelationToPatient)nRelation);

		if(!strHL7RelationCode.IsEmpty()) {
			if(paryParams) {
				// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
				AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "{SQL} ", CreateNewHL7CodeLinkT(nHL7GroupID, hclrtRelationToPatient, strHL7RelationCode, nRelation));
			}
			else {
				// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
				AddStatementToSqlBatch(strSqlBatch, "%s ", CreateNewHL7CodeLinkT(nHL7GroupID, hclrtRelationToPatient, strHL7RelationCode, nRelation).Flatten());
			}

			CString strOld, strNew;
			strOld.Format("Relation To Patient Code '%s' (HL7 Group '%s')", strHL7RelationCode, strHL7GroupName);
			strNew.Format("Linked to %s", strRelation);
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(-1, "", nAuditTransactionID, aeiHL7RelationToPatientLink, nHL7GroupID, strOld, strNew, aepLow, aetCreated);
		}
		return TRUE;
	}
	else {
		return FALSE;
	}
}
//TES 5/8/2008 - PLID 29685 - Changed to use the new HL7Message structure.
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
BOOL AddHL7PatientToData(const HL7Message &Message, BOOL bSendHL7Tablechecker, OPTIONAL OUT long *pnNewPersonID /*= NULL*/)
{
	long nAuditTransactionID = -1;
	try {

		// (j.jones 2010-01-14 16:39) - PLID 31927 - check the default text message privacy field
		long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);

		//TODO: Check for duplicates.

		//TODO: Preference for default location for HL7 Patients?

		//Now, parse all the fields.
		HL7_PIDFields PID;
		//TES 7/12/2007 - PLID 26643, 26586 - The code to parse PID segments has now been unified into one function.
		// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
		if(!ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData())) {
			//TES 4/17/2008 - PLID 29595 - ParsePIDSegment no longer notifies the user on failure, so we'll do it ourselves.
			AfxMessageBox("Could not parse Patient information in HL7 message!");
			return FALSE;
		}
		
		//TES 7/12/2010 - PLID 39518 - Parse all the insurance information.
		CArray<HL7_InsuranceFields,HL7_InsuranceFields&> arInsuranceSegments;
		//TES 7/19/2010 - PLID 39720 - Check the setting for whether to actually import.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		if(GetHL7SettingBit(Message.nHL7GroupID, "ImportInsurance")) {
			ParseInsuranceSegments(Message.strMessage, Message.nHL7GroupID, arInsuranceSegments);
		}

		// (d.singleton 2012-08-27 10:49) - PLID 51938 need to grab the ins set numbers and map to the nextech ins types
		CMapStringToString mapInsSetNumbers;
		if(arInsuranceSegments.GetSize()) {
			_RecordsetPtr rsInsSetNumbers = CreateParamRecordset("SELECT ThirdPartyCode, CONVERT(NVARCHAR,PracticeID) AS PracticeID FROM HL7CodeLinkT WHERE TYPE = {INT} AND HL7GroupID = {INT}", hclrtInsuranceCategoryType, Message.nHL7GroupID);
			while(!rsInsSetNumbers->eof) {
				mapInsSetNumbers.SetAt(AdoFldString(rsInsSetNumbers, "ThirdPartyCode", ""), AdoFldString(rsInsSetNumbers, "PracticeID", "1"));
				rsInsSetNumbers->MoveNext();
			}
		}
		
		//TES 5/7/2008 - PLID 29685 - We now batch our SQL statements (and associated parameters).
		CString strSql;
		CNxParamSqlArray aryParams;

		//Generate our ID, and map the NexTech ID to the HL7ID.
		//long nNewPersonID = NewNumber("PersonT", "ID");
		//TES 7/12/2010 - PLID 39518 - We need to handle this better, since an arbitrary number of PersonT records may be getting
		// created here.  Set up a variable to track the IDs as we go along, and assign the first one to @nPatientID (which 
		// will be read out by FinalizeHL7Event()).
		AddParamStatementToSqlBatch(strSql, aryParams, "SET NOCOUNT OFF");
		// (d.singleton 2012-10-04 12:06) - PLID 53032 and added in sql variable to track priority
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nPriority INT");
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nNextPersonID INT");
		AddParamStatementToSqlBatch(strSql, aryParams, "SELECT @nNextPersonID = COALESCE(Max(ID),0)+1 FROM PersonT");
		//TES 7/14/2010 - PLID 39635 - We may also be creating one or more InsurancePlansT records.
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nNextInsPlanID INt");
		AddParamStatementToSqlBatch(strSql, aryParams, "SELECT @nNextInsPlanID = COALESCE(Max(ID),0)+1 FROM InsurancePlansT");

		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nPatientID INT");
		AddParamStatementToSqlBatch(strSql, aryParams, "SET @nPatientID = @nNextPersonID");
		AddParamStatementToSqlBatch(strSql, aryParams, "SET @nNextPersonID = @nNextPersonID+1");

		//r.wilson (8/23/2012) PLID 52222 [HL7]
		AddParamStatementToSqlBatch(strSql, aryParams, DeclareDefaultDeductibleVars());

		////TES 7/12/2007 - PLID 26643, 26586 - The one thing we need to parse ourselves is the message date, MSH-7.1
		CString strMessageDate;
		GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MSH", 1, 7, 1, 1, strMessageDate);
		COleDateTime dtMessage;
		// (a.walling 2010-02-25 14:17) - PLID 37543 - Include time
		if(!ParseHL7DateTime(strMessageDate, dtMessage)) {
			//Just use the current date.
			dtMessage = COleDateTime::GetCurrentTime();
		}

		//Get a valid PrefixID from our prefix.
		_variant_t varPrefixID = g_cvarNull;
		if(PID.strPrefix != "") {
			long nPrefixID;
			//TES 8/9/2007 - PLID 26883 - Parameterized.
			_RecordsetPtr rsPrefix = CreateParamRecordset("SELECT ID FROM PrefixT WHERE Prefix = {STRING}", _T(PID.strPrefix));
			if(rsPrefix->eof) {
				nPrefixID = NewNumber("PrefixT", "ID");
				//TES 8/9/2007 - PLID 26883 - Parameterized.
				//TES 5/7/2008 - PLID 29685 - Batched.
				AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO PrefixT (ID, Prefix, Gender, InformID) "
					"VALUES ({INT}, {STRING}, 0, 0)", nPrefixID, _T(PID.strPrefix));
			}
			else {
				nPrefixID = AdoFldLong(rsPrefix, "ID");
			}
			varPrefixID = nPrefixID;
			varPrefixID.vt = VT_I4;
	}
		//Construct our query
		CString strInsertPerson;
		

		// (b.spivey, May 28, 2013) - PLID 56888 - If we have races, then we can insert them. 
		CSqlFragment sqlRaceInsert("");
		if (!PID.nsetRaceID.empty()) {
			for each(long nRaceID in PID.nsetRaceID) {
				sqlRaceInsert += CSqlFragment(
					"INSERT INTO PersonRaceT (PersonID, RaceID) "
					"VALUES (@nPatientID, {INT}) ", nRaceID); 
			}
		}

		//All done!
		//TES 8/9/2007 - PLID 26883 - Parameterized.
		//TES 5/7/2008 - PLID 29685 - Batched.
		// (j.jones 2010-01-14 16:35) - PLID 31927 - supported defaulting the text message privacy field
		// (b.spivey, April 02, 2012) - PLID 49170 - Language code now supported. 
		// (d.thompson 2012-08-16) - PLID 52165 - Reworked language table structure
		// (d.thompson 2012-08-23) - PLID 52047 - Supported importing Race
		// (d.thompson 2012-08-23) - PLID 52049 - Supported language overrides, use the ID we already gathered
		// (b.spivey, May 22, 2013) - PLID 56888 - Create patient race records. 
		AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO PersonT (ID, Location, First, Middle, Last, Address1, Address2, City, "			//PID-11.3
			"State, Zip, Gender, Suffix, Title, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, Email, Pager, Fax, "				//PID-13.1, repetition where PID-13.3 = 'FX'
			"BirthDate, SocialSecurity, FirstContactDate, InputDate, UserID, Status, PrefixID, TextMessage, LanguageID, Ethnicity) "
			"VALUES (@nPatientID, {INT}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {INT}, "
			"{STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, "
			"{VT_DATE}, {STRING}, {VT_DATE}, {VT_DATE}, {INT}, 1, {VT_I4}, {INT}, {VT_I4}, {VT_I4}) "
			"	"
			"{SQL} "
			,
			GetCurrentLocationID(), _T(PID.strFirst), _T(PID.strMiddle), _T(PID.strLast), _T(PID.strAddress1),
			_T(PID.strAddress2), _T(PID.strCity), _T(PID.strState), _T(PID.strZip), PID.nGender == -1 ? 0 : PID.nGender,
			_T(PID.strSuffix), _T(PID.strTitle), _T(FormatPhoneForSql(PID.strHomePhone)), _T(FormatPhoneForSql(PID.strWorkPhone)), _T(PID.strExtension),
			_T(FormatPhoneForSql(PID.strCellPhone)), _T(FormatPhoneForSql(PID.strOtherPhone)), _T(PID.strEmail), _T(FormatPhoneForSql(PID.strPager)), _T(FormatPhoneForSql(PID.strFax)), 
			(PID.dtBirthDate.GetStatus() == COleDateTime::valid ? _variant_t(PID.dtBirthDate, VT_DATE) : g_cvarNull),
			_T(FormatSSN(PID.strSSN)), _variant_t(dtMessage, VT_DATE), _variant_t(dtMessage, VT_DATE), GetCurrentUserID(),
			varPrefixID, nTextMessagePrivacy, 
			PID.nLanguageID == -1 ? g_cvarNull : _variant_t(PID.nLanguageID, VT_I4),
			PID.nEthnicityID == -1 ? g_cvarNull : _variant_t(PID.nEthnicityID, VT_I4),
			sqlRaceInsert); 

		AddCreateHL7IDLinkSqlToBatch(strSql, aryParams, Message.nHL7GroupID, hilrtPatient, 
			PID.strHL7ID, PID.strAssigningAuthority, "@nPatientID", PID.strFirst, PID.strLast);

		//Now, the PatientsT part.
		_variant_t varProvID = g_cvarNull;
		long nProvID = GetDefaultProviderID();
		if(nProvID != -1) varProvID = nProvID;

		_variant_t varRefID = g_cvarNull;
		//TES 4/17/2008 - PLID 29595 - PID no longer contains any calculated fields, so let's calculate the referring
		// physician ID now.
		//TES 5/7/2008 - PLID 29685 - Passed in our SQL batch, so if it needs to create a new referring physician, it will
		// just add to our batch.
		//TES 5/23/2011 - PLID 41353 - Create an HL7ReferringPhysician struct to pass in
		HL7ReferringPhysician hrp;
		// (j.kuziel 2011-10-28 11:51) - PLID 43822 - See if we can match on the PV1 parsed referring physician ID.
		hrp.strThirdPartyID = PID.strRefPhysThirdPartyID;
		hrp.strFirst = PID.strRefPhysFirst;
		hrp.strLast = PID.strRefPhysLast;
		hrp.strDegree = PID.strRefPhysDegree;
		//TES 4/22/2015 - PLID 61147 - Added support for the prefix
		hrp.strPrefix = PID.strRefPhysPrefix;
		//TES 6/9/2011 - PLID 41353 - Specify that this is for a patient's referring physician
		CString strRefID = GetNextechRefPhys(hrp, true, Message, nAuditTransactionID, strSql, &aryParams);
		if(strRefID != "@nRefPhysID") {
			if(strRefID != "NULL") {
				varRefID = atol(strRefID);
				AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nRefPhysID int;");
				AddParamStatementToSqlBatch(strSql, aryParams, "SELECT @nRefPhysID = {INT};", VarLong(varRefID));
				strRefID = "@nRefPhysID";
			}
		}

		// (z.manning 2010-07-02 11:42) - PLID 39503 - Let's attempt to use their ID as the user defined ID if it's valid
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nUserDefinedID int");
		long nUserDefinedID = 0;
		if(PID.strHL7ID == PID.strHL7ID.SpanIncluding("0123456789")) {
			nUserDefinedID = AsLong(_bstr_t(PID.strHL7ID));
		}
		if(nUserDefinedID > 0) {
			// (d.thompson 2012-05-31) - PLID 50551 - New option:  If we cannot re-use the patient ID given, fail to import
			//	the entire message.
			if(GetHL7SettingBit(Message.nHL7GroupID, "RequirePatientIDMap") == 1) {
				//I can't find a good way to get this embedded in the existing data access, so we'll have to do an extra data 
				//	lookup for every message import, if this option is on.
				_RecordsetPtr prsIDLookup = CreateRecordset("SELECT TOP 1 * "
					"FROM PatientsT "
					"WHERE UserDefinedID = %li", nUserDefinedID);
				if(!prsIDLookup->eof)
				{
					//There is already a patient with this ID!  Due to our setting, we must refuse to import the message.
					AfxMessageBox("You are configured to require patient ID mapping, but the given ID is already in use in NexTech Practice.  Please review this "
						"with the message source.");
					return FALSE;
				}
				else {
					//There is no other patient with this ID, so proceed as usual
				}
			}

			AddParamStatementToSqlBatch(strSql, aryParams,
				"SET @nUserDefinedID = {INT} \r\n"
				"IF EXISTS (SELECT PersonID FROM PatientsT WHERE UserDefinedID = @nUserDefinedID) BEGIN \r\n"
				"	SET @nUserDefinedID = (SELECT COALESCE(MAX(UserDefinedID), 0) + 1 FROM PatientsT) \r\n"
				"END "
				, nUserDefinedID);
		}
		else {
			// (d.thompson 2012-05-31) - PLID 50551 - We know the UserDefinedID is not something we support, so if the
			//	option is on to require them to map, immediately fail.
			if(GetHL7SettingBit(Message.nHL7GroupID, "RequirePatientIDMap") == 0) {
				AddParamStatementToSqlBatch(strSql, aryParams,
					"SET @nUserDefinedID = (SELECT COALESCE(MAX(UserDefinedID), 0) + 1 FROM PatientsT) \r\n"
					);
			}
			else {
				//Map failed
				AfxMessageBox("You are configured to require patient ID mapping, but the given ID is not valid for NexTech Practice.  Please review this "
					"with the message source.");
				return FALSE;
			}
		}
		
		//TES 8/9/2007 - PLID 26883 - Parameterized.
		//TES 5/7/2008 - PLID 29685 - Batched.
		AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO PatientsT (PersonID, UserDefinedID, CurrentStatus, MaritalStatus, "
			"Nickname, MainPhysician, DefaultReferringPhyID) "
			"VALUES (@nPatientID, @nUserDefinedID, 1, {INT}, {STRING}, {VT_I4}, " + strRefID + ")",
			PID.nMarital == -1 ? 0 : PID.nMarital, 
			_T(PID.strNickName), varProvID);

		// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
		AddParamStatementToSqlBatch(strSql, aryParams,
			"DECLARE @SecurityGroupID INT\r\n"
			"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
			"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
			"BEGIN\r\n"
			"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, @nPatientID)\r\n"
			"END\r\n");
			

		//TES 7/12/2010 - PLID 39518 - Now calculate all the InsuranceCoIDs, InsuranceContactIDs, and InsPlans.
		if(!AssignInsCoIDs(PID, -1, arInsuranceSegments, Message.nHL7GroupID, nAuditTransactionID, strSql, &aryParams)) {
			//TES 7/14/2010 - PLID 39635 - They were prompted, and chose to cancel.
			return FALSE;
		}

		//TES 7/19/2010 - PLID 39518 - We need to track any HL7 Relation mappings we prompt the user for, so that if they have the same
		// unmapped code twice in the same message, they're only prompted for it once.
		CMapStringToString mapUserMappedRelations;
		//TES 7/12/2010 - PLID 39518 - Now, go through each insurance segment and add it to our batch.
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nInsuredPartyID INT");
		for(int i = 0; i < arInsuranceSegments.GetSize(); i++) {
			HL7_InsuranceFields Insurance = arInsuranceSegments[i];
			_variant_t varBirthDate = g_cvarNull;
			if(Insurance.dtInsPartyBirthDate.GetStatus() == COleDateTime::valid) {
				varBirthDate = _variant_t(Insurance.dtInsPartyBirthDate, VT_DATE);
			}
			long nGender = 0;
			if(Insurance.nInsPartyGender != -1) {
				nGender = Insurance.nInsPartyGender;
			}
			AddParamStatementToSqlBatch(strSql, aryParams, "SET @nInsuredPartyID = @nNextPersonID");
			AddParamStatementToSqlBatch(strSql, aryParams, "SET @nNextPersonID = @nNextPersonID+1");
			AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO PersonT (ID, First, Middle, Last, BirthDate, Address1, Address2, "
				"City, State, Zip, SocialSecurity, HomePhone, Gender) "
				"VALUES (@nInsuredPartyID, {STRING}, {STRING}, {STRING}, {VT_DATE}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, "
				"{STRING}, {INT})", Insurance.strInsPartyFirst, Insurance.strInsPartyMiddle, Insurance.strInsPartyLast, 
				varBirthDate, Insurance.strInsPartyAddress1, Insurance.strInsPartyAddress2, Insurance.strInsPartyCity, 
				Insurance.strInsPartyState, Insurance.strInsPartyZip, Insurance.strInsPartySSN,	Insurance.strInsPartyHomePhone, nGender);

			_variant_t varEffective = g_cvarNull;
			if(Insurance.dtEffective.GetStatus() == COleDateTime::valid) {
				varEffective = _variant_t(Insurance.dtEffective, VT_DATE);
			}
			_variant_t varExpires = g_cvarNull;
			if(Insurance.dtExpires.GetStatus() == COleDateTime::valid) {
				varExpires = _variant_t(Insurance.dtExpires, VT_DATE);
			}			

			// (d.singleton 2012-08-27 12:03) - PLID 51938 grab the resptypet.categorytype and cross check the ones on the message with the matched insured party
			CString strInsCategoryType;
			long nInsCategoryType;
			if(!mapInsSetNumbers.Lookup(Insurance.strInsSetNumber, strInsCategoryType)) {
				strInsCategoryType = "1";
			}
			nInsCategoryType = atoi(strInsCategoryType);

			//see if a RespType record already exists
			_RecordsetPtr prsPriority = CreateParamRecordset("SELECT Priority FROM RespTypeT WHERE CategoryType = {INT} AND CategoryPlacement = {INT};", nInsCategoryType, i+1);
			long nRespType; 
			if(!prsPriority->eof) {
				nRespType = AdoFldLong(prsPriority, "Priority");
				AddParamStatementToSqlBatch(strSql, aryParams, "SET @nPriority = {INT}", nRespType);
			}
			else {
				// (d.singleton 2012-10-04 12:06) - PLID 53032 got rid of NextRecordset() and added in sql param to track priority
				// also params in wrong order in below query
				//TES 7/20/2010 - PLID 39518 - Ensure the RespType exists.
				//TES 8/11/2010 - PLID 39518 - Ensure the Priority, not the ID.
				AddParamStatementToSqlBatch(strSql, aryParams, "SET @nPriority = (SELECT MAX(Priority) + 1 FROM RespTypeT)");
				AddParamStatementToSqlBatch(strSql, aryParams,  "INSERT INTO RespTypeT (ID, TypeName, Priority, CategoryType, CategoryPlacement) "
					"SELECT (SELECT Max(ID)+1 FROM RespTypeT), {INT}, @nPriority, {INT}, {INT}",
					i+1, nInsCategoryType, i+1);
			}

			//TES 7/12/2010 - PLID 39610 - Find out what relationship this code is mapped to (if it isn't mapped to anything, the
			// PromptForHL7Relation() function will tell the user, so we need to just exit).
			CString strRelation;
			//TES 7/19/2010 - PLID 39610 - First check whether the user's mapped this relation already.
			if(!mapUserMappedRelations.Lookup(Insurance.strInsPartyRelation, strRelation)) {
				if(!GetHL7Relation(GetRemoteData(), Insurance.strInsPartyRelation, strRelation, Message.nHL7GroupID)) {
					if(!PromptForHL7Relation(Insurance.strInsPartyRelation, Message.nHL7GroupID, Message.strHL7GroupName, strRelation, nAuditTransactionID, strSql, &aryParams)) {
						return FALSE;
					}
					else {
						//TES 7/19/2010 - PLID 39610 - Remember what the user chose.
						mapUserMappedRelations.SetAt(Insurance.strInsPartyRelation, strRelation);
					}
				}
			}
			//TES 7/14/2010 - PLID 39635 - AssignInsCoIDs() will have set these variables in the batch.
			CString strInsCoID;
			strInsCoID.Format("@nInsuranceCoID%i", i);
			CString strInsContactID;
			strInsContactID.Format("@nInsuranceContactID%i", i);
			CString strInsPlanID;
			strInsPlanID.Format("@nInsurancePlanID%i", i);

			// (j.jones 2011-06-24 12:35) - PLID 29885 - added a default secondary reason code
			CString strSecondaryReasonCode = GetRemotePropertyText("DefaultMedicareSecondaryReasonCode", "47", 0, "<None>", true);
			if(strSecondaryReasonCode.IsEmpty()) {
				strSecondaryReasonCode = "47";
			}

			// (j.gruber 2010-08-04 10:15) - PLID 39977 - remove copay structure
			//TES 8/11/2010 - PLID 39518 - We have the RespType Priority, but not the ID, so make sure we use the correct ID.
			// (d.singleton 2012-10-09 15:33) - PLID 53032 had to change to use the sql variable @nPriority
			AddParamStatementToSqlBatch(strSql, aryParams, "INSERT INTO InsuredPartyT (PersonID, PatientID, InsuranceCoID, "
				"InsuranceContactID, PolicyGroupNum, EffectiveDate, ExpireDate, InsPlan, RelationToPatient, IDForInsurance, "
				"AssignDate, RespTypeID, Employer, SecondaryReasonCode) "
				"SELECT @nInsuredPartyID, @nPatientID, " + strInsCoID + ", " + strInsContactID + ", {STRING}, {VT_DATE}, {VT_DATE}, "
				+ strInsPlanID + ", {STRING}, {STRING}, getdate(), ID, {STRING}, {STRING} "
				"FROM RespTypeT WHERE Priority = @nPriority",
				Insurance.strGroupNum, varEffective, varExpires, strRelation, 
				Insurance.strIDForInsurance, Insurance.strEmployer, strSecondaryReasonCode);
						
			//r.wilson (8/23/2012) PLID 52222 [HL7]
			CSqlFragment sqlFrag;						
			
			sqlFrag += CSqlFragment(" SELECT @b_DefaultDeductiblePerPayGroup = DefaultDeductiblePerPayGroup, "
									" @money_InsCoDefTotalDeductible = DefaultTotalDeductible, "
									" @money_InsCoDefTotalOOP = DefaultTotalOOP "
									" FROM InsuranceCoT WHERE PersonID = "+ strInsCoID +" ; \r\n"
									" "
									" INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PaygroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP) "
									" SELECT  @nInsuredPartyID AS InsuredPartyID, ServicePaygroupsT.ID AS PayGroupID , CopayMoney, CopayPercentage, CoInsurance, "
									" CASE WHEN @b_DefaultDeductiblePerPayGroup = 1 THEN PayGroupDefaultsT.TotalDeductible "
									" 	 ELSE @money_InsCoDefTotalDeductible "
									"	 END AS TotalDeductible, "
									" CASE WHEN @b_DefaultDeductiblePerPayGroup = 1 THEN PayGroupDefaultsT.TotalOOP "
									"	 ELSE @money_InsCoDefTotalOOP "
									"	 END AS TotalOOP "
									" FROM ServicePaygroupsT "
									" INNER JOIN (SELECT ID, PayGroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP "
									"			  FROM InsuranceCoPayGroupsDefaultsT  "
									"			  WHERE InsuranceCoID = "+ strInsCoID +") "
									" AS PayGroupDefaultsT ON ServicePaygroupsT.ID = PayGroupDefaultsT.PaygroupID; \r\n"
									" "
									" IF @b_DefaultDeductiblePerPayGroup = 0 "
									" BEGIN "
									" UPDATE InsuredPartyT "
									" SET "
									"     TotalDeductible = @money_InsCoDefTotalDeductible, TotalOOP = @money_InsCoDefTotalOOP, DeductiblePerPayGroup = @b_DefaultDeductiblePerPayGroup, LastModifiedDate = GetDate() "
									" WHERE "
									" PersonID = @nInsuredPartyID AND InsuranceCoID = "+ strInsCoID +"; \r\n"
									" END \r\n"
									" ELSE "
									" BEGIN "
									" UPDATE InsuredPartyT "
									" SET "
									" DeductiblePerPayGroup = @b_DefaultDeductiblePerPayGroup "
									" WHERE "
									" PersonID = @nInsuredPartyID AND InsuranceCoID = "+ strInsCoID +"; \r\n"
									" END "
									);

			
			AddParamStatementToSqlBatch(strSql, aryParams, sqlFrag.Flatten()); 



			//TES 7/22/2010 - PLID 39518 - Format the CoPay correctly.
			// (j.gruber 2010-08-04 10:17) - PLID 39977 - change copay structure
			COleCurrency cyCoPay;
			cyCoPay.ParseCurrency(Insurance.strCoPay);
			_variant_t varCoPay = g_cvarNull;
			if(cyCoPay.GetStatus() == COleCurrency::valid) {
				// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
				RoundCurrency(cyCoPay);
				varCoPay = _variant_t(cyCoPay);
				AddParamStatementToSqlBatch(strSql, aryParams,  "DECLARE @nPayGpID INT");
					AddParamStatementToSqlBatch(strSql, aryParams, "SET @nPayGpID = (SELECT ID FROM ServicePayGroupsT WHERE Name = 'Copay');");
					//r.wilson (8/23/2012) PLID 52222 [HL7]
					AddParamStatementToSqlBatch(strSql, aryParams, 
						" IF EXISTS(SELECT InsuredPartyID FROM InsuredPartyPaygroupsT WHERE InsuredPartyID = @nInsuredPartyID AND PayGroupID = @nPayGpID ) "
						" BEGIN "
						" UPDATE InsuredPartyPaygroupsT SET CopayMoney = {VT_CY} "
						" WHERE InsuredPartyID = @nInsuredPartyID AND PayGroupID = @nPayGpID "
						" END "
						" ELSE "
						" BEGIN "						
						" INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PayGroupID, CopayMoney) VALUES "
						" (@nInsuredPartyID, @nPayGpID, {VT_CY} )"
						" END ", varCoPay,varCoPay);					
			}

		}

		//TES 5/7/2008 - PLID 29685 - Now, execute all our batched statements at once.
		//TES 5/8/2008 - PLID 29685 - Actually, we're going to go ahead and pass these in to FinalizeHL7Event().
		//ExecuteParamSqlBatch(GetRemoteData(), strSql, aryParams);

		//TES 5/8/2008 - PLID 29685 - Now call FinalizeHL7Event(), which will commit our batch, as well as updating HL7MessageQueueT.
		//TES 7/12/2010 - PLID 39518 - FinalizeHL7Event() will tell us the Patient ID.
		//TES 7/17/2010 - PLID 39518 - FinalizeHL7Event() now takes a pointer to the AuditID
		long nNewPersonID = -1;
		//TES 6/10/2011 - PLID 41353 - Tell the function to return @nPatientID
		if(!FinalizeHL7Event(nNewPersonID, Message, strSql, bSendHL7Tablechecker, &aryParams, &nAuditTransactionID, "@nPatientID", &nNewPersonID)) {
			return FALSE;
		}

		//TES 4/21/2008 - PLID 27898 - Audit the new patient.
		CString strName;
		strName.Format("%s, %s %s", PID.strLast, PID.strFirst, PID.strMiddle);
		if(nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}
		AuditEvent(nNewPersonID, strName, nAuditTransactionID, aeiPatientCreated, nNewPersonID, "", strName, aepMedium, aetCreated);

		// (j.jones 2010-05-13 10:29) - PLID 36527 - audit the HL7IDLinkT creation
		CString strOld, strNew;
		strOld.Format("Patient Code '%s' (HL7 Group '%s')", PID.strHL7ID, Message.strHL7GroupName);
		strNew.Format("Linked to Patient '%s'", strName);
		AuditEvent(nNewPersonID, strName, nAuditTransactionID, aeiHL7PatientLink, nNewPersonID, strOld, strNew, aepLow, aetCreated);

		CommitAuditTransaction(nAuditTransactionID);

		// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
		UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), nNewPersonID);

		// (r.gonet 04/22/2014) - PLID 53170 - Check the preference to auto-assign a NexWeb security code for the new patient.
		// We don't check the NexWeb license here to be consistent with NxServer. We don't want NxServer dependent on the
		// license server. Generating a security code when they don't have NexWeb is harmless.
		bool bCreateNexWebSecurityCode = false;
		if(GetRemotePropertyInt("AssignNewPatientSecurityCode", 1, 0, "<None>", true) != 0){
			bCreateNexWebSecurityCode = true;
		} else {
			// (r.gonet 04/22/2014) - PLID 53170 - They want to assign the security codes manually
		}

		// (r.gonet 04/22/2014) - PLID 53170 - Create a NexWeb security code for this patient if the preference to do so is on.
		if(bCreateNexWebSecurityCode){
			//(e.lally 2009-01-26) PLID 32813 - Generate and assign a security code. 
			_ConnectionPtr pCon = GetRemoteData();
			//Potential performance hit here depending on how many trips to the sql server this has to make.
			CString strSecurityCode = EnsurePatientSecurityCode(nNewPersonID, pCon);
			// (d.singleton 2011-10-11 11:31) - PLID 42102 - add auditing for security codes.
			AuditEvent(nNewPersonID, GetExistingPatientName(nNewPersonID), BeginNewAuditEvent(), aeiPatientSecurityCode, nNewPersonID, "", strSecurityCode, aepHigh, aetChanged);
		}

		CMainFrame * pMain = GetMainFrame();
		if(pMain) {
			// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
			pMain->m_patToolBar.UpdatePatient(nNewPersonID);
		}
		//TES 8/13/2014 - PLID 63194 - Use the EX tablechecker here (HL7 patients are always imported as Active Patients)
		CClient::RefreshPatCombo(nNewPersonID, false, CClient::pcatActive, CClient::pcstPatient);

		//TES 11/1/2007 - PLID 26892 - Return the newly created ID, if our caller wants us to.
		if(pnNewPersonID) *pnNewPersonID = nNewPersonID;

		//All done!
		return TRUE;
	}NxCatchAll("Error in HL7Utils::AddHL7PatientToData()");
	if(nAuditTransactionID != -1) {
		RollbackAuditTransaction(nAuditTransactionID);
	}

	return FALSE;
}

//TES 5/8/2008 - PLID 29685 - Changed to use the new HL7Message structure.
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
BOOL UpdateHL7Patient(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker)
{
	long nAuditID = -1;
	long nPersonID = -1;
	long nUserdefinedID = -1;
	CString strFirst = "";
	CString strLast = "";
	// (d.singleton 2012-10-15 17:50) - PLID 53037 add params so i dont have to call GetPatientDocumentPath()
	CString strMessageFirst = "";
	CString strMessageLast = "";
	CString strError = "";
	// (d.singleton 2012-09-06 11:30) - PLID 51954 update the patient history folder if name changed
	BOOL bPatientNameChanged = FALSE;
	try {

		//(e.lally 2007-06-15) PLID 26325 - Put in check for whether we need to update patient list via table checker or not
		BOOL bSendPatientChecker = FALSE;		

		//First, parse everything out.
		//TES 7/12/2007 - PLID 26643, 26586 - The code to parse PID segments has now been unified into one function.
		HL7_PIDFields PID;
		// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
		if(!ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData())) {
			//TES 4/17/2008 - PLID 29595 - ParsePIDSegment no longer notifies the user on failure, so we'll do it ourselves.
			AfxMessageBox("Could not parse Patient information in HL7 message!");
			return FALSE;
		}

		// (d.singleton 2012-10-04 14:28) - PLID 53036 need to grab the message name values to use in case there is an exception
		// as PID object will be out of scope
		strMessageFirst = PID.strFirst;
		strMessageLast = PID.strLast;

		//TES 7/15/2010 - PLID 39518 - Parse all the insurance information.
		CArray<HL7_InsuranceFields,HL7_InsuranceFields&> arInsuranceSegments;
		//TES 7/19/2010 - PLID 39720 - Check the setting for whether to actually import.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		// (d.singleton 2012-09-07 12:44) - PLID 51938 we now handle ins in its own function,  so set a BOOL so we know to call that function or not
		BOOL bImportIns = FALSE;
		if(GetHL7SettingBit(Message.nHL7GroupID, "ImportInsurance")) {
			//ParseInsuranceSegments(Message.strMessage, Message.nHL7GroupID, arInsuranceSegments);
			bImportIns = TRUE;
		}
		
		//TES 7/12/2007 - PLID 26642 - The code to calculate the person is now in a single function (CreateHL7Bill() also
		// uses it).  We pass in true for bAllowCreateNew, meaning that we want the user to have the option to create this 
		// record as a new patient, rather than updating an existing one.  We also pass in true for bWillOverwrite, to warn
		// the user that they could potentially overwrite a patient's demographics (including name) with the incoming 
		// HL7 message's demographics.
		//TES 8/9/2007 - PLID 26892 - Pass in an enum to get the result in the case where the patient wasn't found, pass
		// in the enum indicating that the user should get prompted, with the option to create new.
		ENotFoundResult nfr;
		//TES 9/18/2008 - PLID 31414 - Renamed
		nPersonID = GetPatientFromHL7Message(PID, Message.nHL7GroupID, pMessageParent, nfbPromptToLinkAndCreate, &nfr, true);
		if(nPersonID == -1) {
			if(nfr == nfrFailure) {
				//The message couldn't be parsed, or the user cancelled.
				return FALSE;
			}
			else {
				//TES 8/9/2007 - PLID 26892 - They must want us to create new; the only other result is nfrSkipped, 
				// which should only happen if we pass in nfbSkip.
				ASSERT(nfr == nfrCreateNew);
				//TES 7/12/2007 - PLID 26642 - The user chose to create a new patient record, so do that, and return.
				return AddHL7PatientToData(Message, bSendHL7Tablechecker);
			}
		}

		//TES 5/7/2008 - PLID 29685 - We now batch our SQL statements.
		CString strSql;

		//TES 7/12/2010 - PLID 39518 - Now that we're importing insurance, an arbitrary number of PersonT records may be getting
		// created here.  Set up a variable to track the IDs as we go along, and assign the first one to @nPatientID (which 
		// will be read out by FinalizeHL7Event()).
		// (d.singleton 2012-10-09 17:13) - PLID 53031 added variable to track priority
		AddStatementToSqlBatch(strSql, "SET NOCOUNT OFF");
		AddStatementToSqlBatch(strSql, "DECLARE @nPriority INT");
		AddStatementToSqlBatch(strSql, "DECLARE @nNextPersonID INT");
		AddStatementToSqlBatch(strSql, "SELECT @nNextPersonID = COALESCE(Max(ID),0)+1 FROM PersonT");
		//TES 7/14/2010 - PLID 39635 - We may also be creating one or more InsurancePlansT records.
		AddStatementToSqlBatch(strSql, "DECLARE @nNextInsPlanID INt");
		AddStatementToSqlBatch(strSql, "SELECT @nNextInsPlanID = COALESCE(Max(ID),0)+1 FROM InsurancePlansT");

		//r.wilson PLID 52222 [HL7]
		AddStatementToSqlBatch(strSql, DeclareDefaultDeductibleVars());

		CString strUpdatePerson = "UPDATE PersonT SET ";


		//Now, construct an UPDATE query using all the fields we parsed.  Don't update any fields that are blank in the
		// incoming data.
		
		//TES 4/21/2008 - PLID 27898 - We need to audit, so we need to pull all the existing information.
		// (b.spivey, April 02, 2012) - PLID 49170 - Language code now supported.
		// (d.thompson 2012-08-16) - PLID 52165 - Reworked language table structure
		// (b.spivey, May 23, 2013) - PLID 56888 - get the IDs of races. 
		_RecordsetPtr rsExistingPerson = CreateParamRecordset(
			"SET NOCOUNT ON "
			"DECLARE @PersonID INT " 
			"SET @PersonID = {INT} "
			"	"
			"SELECT PatientsT.UserDefinedID, First, Middle, Last, Address1, Address2, City, State, "
			"Zip, Gender, Title, HomePhone, WorkPhone, Extension, CellPhone, OtherPhone, EMail, Pager, Fax, "
			"BirthDate, SocialSecurity, "
			"PersonT.Ethnicity, EthnicityT.Name AS EthnicityName, PersonT.LanguageID, LanguageT.Name AS LanguageName "
			"FROM PersonT "
			"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN LanguageT ON PersonT.LanguageID = LanguageT.ID "
			"LEFT JOIN LanguageCodesT ON LanguageT.LanguageCodeID = LanguageCodesT.ID "
			"LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID "
			"WHERE PersonT.ID = @PersonID "
			"	"
			"SELECT DISTINCT PersonID, RaceID AS PersonRaceID "
			"FROM PersonRaceT "
			"WHERE PersonID = @PersonID "
			"SET NOCOUNT OFF ", nPersonID);

		nUserdefinedID = AdoFldLong(rsExistingPerson, "UserDefinedID", -1);
		strFirst = AdoFldString(rsExistingPerson, "First","");
		CString strMiddle = AdoFldString(rsExistingPerson, "Middle","");
		strLast = AdoFldString(rsExistingPerson, "Last","");
		CString strAddress1 = AdoFldString(rsExistingPerson, "Address1","");
		CString strAddress2 = AdoFldString(rsExistingPerson, "Address2","");
		CString strCity = AdoFldString(rsExistingPerson, "City","");
		CString strState = AdoFldString(rsExistingPerson, "State","");
		CString strZip = AdoFldString(rsExistingPerson, "Zip","");
		BYTE bGender = AdoFldByte(rsExistingPerson, "Gender", 0);
		CString strTitle = AdoFldString(rsExistingPerson, "Title","");
		CString strHomePhone = AdoFldString(rsExistingPerson, "HomePhone","");
		CString strWorkPhone = AdoFldString(rsExistingPerson, "WorkPhone","");
		CString strExtension = AdoFldString(rsExistingPerson, "Extension","");
		CString strCellPhone = AdoFldString(rsExistingPerson, "CellPhone","");
		CString strOtherPhone = AdoFldString(rsExistingPerson, "OtherPhone","");
		CString strEmail = AdoFldString(rsExistingPerson, "Email","");
		CString strPager = AdoFldString(rsExistingPerson, "Pager","");
		CString strFax = AdoFldString(rsExistingPerson, "Fax","");
		// (d.thompson 2012-08-21) - PLID 52048 - Added ethnicity
		long nEthnicityID = AdoFldLong(rsExistingPerson, "Ethnicity", -1);
		CString strEthnicityName = AdoFldString(rsExistingPerson, "EthnicityName", "");
		// (d.thompson 2012-08-23) - PLID 52049 - Added LanguageID
		long nLanguageID = AdoFldLong(rsExistingPerson, "LanguageID", -1);
		CString strLanguageName = AdoFldString(rsExistingPerson, "LanguageName", "");
		COleDateTime dtBirthDate;
		_variant_t varBirthDate = rsExistingPerson->Fields->GetItem("BirthDate")->Value;
		if(varBirthDate.vt == VT_DATE) {
			dtBirthDate = VarDateTime(varBirthDate);
		}
		else {
			dtBirthDate.SetStatus(COleDateTime::invalid);
		}
		CString strSocialSecurity = AdoFldString(rsExistingPerson, "SocialSecurity","");

		// (d.thompson 2012-08-23) - PLID 52047 - Added Race
		// (b.spivey, May 23, 2013) - PLID 56888
		std::set<long> setRaceIDs; 
		rsExistingPerson = rsExistingPerson->NextRecordset(NULL);

		while (!rsExistingPerson->eof) {
			long nRaceID = AdoFldLong(rsExistingPerson, "PersonRaceID", -1);

			if(nRaceID > 0) {
				setRaceIDs.insert(nRaceID); 
			}

			rsExistingPerson->MoveNext(); 
		}

		rsExistingPerson->Close();

		CString strPersonName;
		strPersonName.Format("%s, %s %s", strLast, strFirst, strMiddle);

		nAuditID = BeginAuditTransaction();
		//TES 4/21/2008 - PLID 27898 - Went through each field, and added auditing.
		//First
		if(PID.strFirst != "" && PID.strFirst != strFirst) {
			strUpdatePerson += "First = '" + _Q(PID.strFirst) + "', ";
			bSendPatientChecker = TRUE;
			// (d.singleton 2012-09-06 11:30) - PLID 51954 update the patient history folder if name changed
			bPatientNameChanged = TRUE;
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientPersonFirst, nPersonID, strFirst, PID.strFirst, aepMedium, aetChanged);
		}

		//Middle
		if(PID.strMiddle != "" && PID.strMiddle != strMiddle) {
			strUpdatePerson += "Middle = '" + _Q(PID.strMiddle) + "', ";
			bSendPatientChecker = TRUE;
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientPersonMiddle, nPersonID, strMiddle, PID.strMiddle, aepMedium, aetChanged);
		}

		//Last
		if(PID.strLast != "" && PID.strLast != strLast) {
			strUpdatePerson += "Last = '" + _Q(PID.strLast) + "', ";
			bSendPatientChecker = TRUE;
			// (d.singleton 2012-09-06 11:30) - PLID 51954 update the patient history folder if name changed
			bPatientNameChanged = TRUE;
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientPersonLast, nPersonID, strLast, PID.strLast, aepMedium, aetChanged);
		}

		//Address1
		if(PID.strAddress1 != "" && PID.strAddress1 != strAddress1) {
			strUpdatePerson += "Address1 = '" + _Q(PID.strAddress1) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientAddress, nPersonID, strAddress1, PID.strAddress1, aepMedium, aetChanged);
		}

		//Address2
		if(PID.strAddress2 != "" && PID.strAddress2 != strAddress2) {
			strUpdatePerson += "Address2 = '" + _Q(PID.strAddress2) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientAddress2, nPersonID, strAddress2, PID.strAddress2, aepMedium, aetChanged);
		}

		//City
		if(PID.strCity != "" && PID.strCity != strCity) {
			strUpdatePerson += "City = '" + _Q(PID.strCity) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientCity, nPersonID, strCity, PID.strCity, aepMedium, aetChanged);
		}
		
		//State
		if(PID.strState != "" && PID.strState != strState) {
			strUpdatePerson += "State = '" + _Q(PID.strState) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientState, nPersonID, strState, PID.strState, aepMedium, aetChanged);
		}

		//Zip
		if(PID.strZip!= "" && PID.strZip != strZip) {
			strUpdatePerson += "Zip = '" + _Q(PID.strZip) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientZip, nPersonID, strZip, PID.strZip, aepMedium, aetChanged);
		}

		//Gender
		BOOL bGenderUpdated = FALSE;
		if(PID.nGender != -1 && PID.nGender != bGender) {
			strUpdatePerson += "Gender = " + AsString(PID.nGender) + ", ";
			bGenderUpdated = TRUE;
			CString strOldGender;
			if(bGender == 1) strOldGender = "Male";
			else if(bGender == 2) strOldGender = "Female";
			CString strNewGender;
			if(PID.nGender == 1) strNewGender = "Male";
			else if(PID.nGender == 2) strNewGender = "Female";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientGender, nPersonID, strOldGender, strNewGender, aepMedium, aetChanged);
		}

		//Suffix
		if(PID.strSuffix!= "") {
			strUpdatePerson += "Suffix = '" + _Q(PID.strSuffix) + "', ";
		}

		//Title
		if(PID.strTitle!= "" && PID.strTitle != strTitle) {
			strUpdatePerson += "Title = '" + _Q(PID.strTitle) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientTitle, nPersonID, strTitle, PID.strTitle, aepMedium, aetChanged);
		}

		// (j.dinatale 2010-09-01) - PLID 39609 - Format the phone to be in the format of (###) ###-####
		//HomePhone
		CString strNewHomePhone = FormatPhoneForSql(PID.strHomePhone);

		if(PID.strHomePhone != "" && strNewHomePhone != strHomePhone) {
			strUpdatePerson += "HomePhone = '" + _Q(strNewHomePhone) + "', ";

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientHPhone, nPersonID, strHomePhone, strNewHomePhone, aepMedium, aetChanged);
		}

		// (j.dinatale 2010-09-01) - PLID 39609 - Format the phone to be in the format of (###) ###-####
		//WorkPhone
		CString strNewWorkPhone = FormatPhoneForSql(PID.strWorkPhone);

		if(PID.strWorkPhone != "" && strNewWorkPhone != strWorkPhone) {
			strUpdatePerson += "WorkPhone = '" + _Q(strNewWorkPhone) + "', ";

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientWPhone, nPersonID, strWorkPhone, strNewWorkPhone, aepMedium, aetChanged);
		}

		//Extension
		if(PID.strExtension != "" && PID.strExtension != strExtension) {
			strUpdatePerson += "Extension = '" + _Q(PID.strExtension) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientExtension, nPersonID, strExtension, PID.strExtension, aepMedium, aetChanged);
		}

		// (j.dinatale 2010-09-01) - PLID 39609 - Format the phone to be in the format of (###) ###-####
		//CellPhone
		CString strNewCellPhone = FormatPhoneForSql(PID.strCellPhone);

		if(PID.strCellPhone != "" && strNewCellPhone != strCellPhone) {
			strUpdatePerson += "CellPhone = '" + _Q(strNewCellPhone) + "', ";

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientMobilePhone, nPersonID, strCellPhone, strNewCellPhone, aepMedium, aetChanged);
		}

		// (j.dinatale 2010-09-01) - PLID 39609 - Format the phone to be in the format of (###) ###-####
		//OtherPhone
		CString strNewOtherPhone = FormatPhoneForSql(PID.strOtherPhone);

		if(PID.strOtherPhone != "" & strNewOtherPhone != strOtherPhone) {
			strUpdatePerson += "OtherPhone = '" + _Q(strNewOtherPhone) + "', ";

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientOtherPhone, nPersonID, strOtherPhone, strNewOtherPhone, aepMedium, aetChanged);
		}

		//Email
		if(PID.strEmail != "" && PID.strEmail != strEmail) {
			strUpdatePerson += "Email = '" + _Q(PID.strEmail) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientEmail, nPersonID, strEmail, PID.strEmail, aepMedium, aetChanged);
		}
		
		// (j.dinatale 2010-09-01) - PLID 39609 - Format the phone to be in the format of (###) ###-####
		//Pager
		CString strNewPager = FormatPhoneForSql(PID.strPager);

		if(PID.strPager != "" && strNewPager != strPager) {
			strUpdatePerson += "Pager = '" + _Q(strNewPager) + "', ";

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientPagerNumber, nPersonID, strPager, strNewPager, aepMedium, aetChanged);
		}

		// (j.dinatale 2010-09-01) - PLID 39609 - Format the phone to be in the format of (###) ###-####
		//Fax
		CString strNewFax = FormatPhoneForSql(PID.strFax);

		if(PID.strFax != "" && strNewFax != strFax) {
			strUpdatePerson += "Fax = '" + _Q(strNewFax) + "', ";

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientFaxNumber, nPersonID, strFax, strNewFax, aepMedium, aetChanged);
		}

		//BirthDate
		BOOL bBirthDateUpdated = FALSE;
		if(PID.dtBirthDate.GetStatus() == COleDateTime::valid && PID.dtBirthDate != dtBirthDate) {
			strUpdatePerson += "BirthDate = '" + FormatDateTimeForSql(PID.dtBirthDate) +  "', ";
			bBirthDateUpdated = TRUE;
			bSendPatientChecker = TRUE;
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientBirthDate, nPersonID, FormatDateTimeForInterface(dtBirthDate), FormatDateTimeForInterface(PID.dtBirthDate), aepMedium, aetChanged);
		}

		// (j.dinatale 2010-09-01) - PLID 39609 - Format the SSN to be in the format of ###-##-####, also need to make sure that we remove the format string if thats what is returned
		//SocialSecurity
		CString strNewSSN = FormatSSN(PID.strSSN);
		if(PID.strSSN != "" && strNewSSN != strSocialSecurity) {
			strUpdatePerson += "SocialSecurity = '" + _Q(strNewSSN) + "', ";
			bSendPatientChecker = TRUE;

			// (j.dinatale 2010-09-01) - PLID 39609 - now need to audit the new formatted string instead
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientSSN, nPersonID, strSocialSecurity, strNewSSN, aepMedium, aetChanged);
		}

		//PrefixID
		if(PID.strPrefix != "") {
			long nPrefixID;
			//TES 8/9/2007 - PLID 26883 - Parameterized.
			_RecordsetPtr rsPrefix = CreateParamRecordset("SELECT ID FROM PrefixT WHERE Prefix = {STRING}", _T(PID.strPrefix));
			if(rsPrefix->eof) {
				nPrefixID = NewNumber("PrefixT", "ID");
				//TES 5/7/2008 - PLID 29685 - Batched (and de-parameterized, this should be called almost never anyway..
				AddStatementToSqlBatch(strSql, "INSERT INTO PrefixT (ID, Prefix, Gender, InformID) "
					"VALUES (%li, '%s', 0, 0)", nPrefixID, _Q(PID.strPrefix));
			}
			else {
				nPrefixID = AdoFldLong(rsPrefix, "ID");
			}
			strUpdatePerson += "PrefixID = " + AsString(nPrefixID) + ", ";
		}

		// (b.spivey, April 02, 2012) - PLID 49170 - We support this now.
		// (d.thompson 2012-08-23) - PLID 52049 - We now match on LanguageID, not textual code
		//Language
		if (PID.nLanguageID != -1 && PID.nLanguageID != nLanguageID) {
			// (d.thompson 2012-08-16) - PLID 52165 - Reworked language table structure
			_RecordsetPtr prsLang = CreateParamRecordset("SELECT Name FROM LanguageT WHERE ID = {INT} ", PID.nLanguageID);
			CString strNewLanguage;
			if (!prsLang->eof) {
				strNewLanguage = AdoFldString(prsLang, "Name", ""); 
			}

			// (d.thompson 2012-08-16) - PLID 52165 - Reworked language table structure  (use top 1 for safety, should never be duplicates)
			// (d.thompson 2012-08-23) - PLID 52049 - Forget all that, we now have the language ID
			strUpdatePerson += "LanguageID = " + AsString(PID.nLanguageID) + ", ";

			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientLanguage, nPersonID, strLanguageName, strNewLanguage, aepMedium, aetChanged);
		}

		// (d.thompson 2012-08-21) - PLID 52048 - Added support for importing Ethnicity
		if(PID.nEthnicityID != -1 && PID.nEthnicityID != nEthnicityID) {
			//For auditing
			_RecordsetPtr prsEthName = CreateParamRecordset("SELECT Name FROM EthnicityT WHERE ID = {INT}", PID.nEthnicityID);
			CString strNewEth;

			if(!prsEthName->eof) {
				strNewEth = AdoFldString(prsEthName->Fields, "Name", ""); 
			}
			strUpdatePerson += "Ethnicity = " + AsString(PID.nEthnicityID) + ", ";

			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientEthnicity, nPersonID, strEthnicityName, strNewEth, aepMedium, aetChanged);
		}

		// (d.thompson 2012-08-23) - PLID 52047 - Added support for importing Race
		// (b.spivey, May 23, 2013) - PLID 56888 - If we have a size greater than zero, there may have been a change. 
		// (d.singleton 2013-08-29 16:52) - PLID 58307 - exception when importing a patient message with race changes but no demographic changes.
		bool bPersonTUpdates = true;
		if(strUpdatePerson == "UPDATE PersonT SET ") {
			bPersonTUpdates = false;
		}
		if(PID.nsetRaceID.size() > 0) {


			//Need to compare a vector to a vector and make sure the values are the same. 
			//&& PID.nRaceID != nRaceID) {
			//For auditing

			// (b.spivey, May 28, 2013) - PLID 56888 - Create a statement to update if there is a length. 
			CSqlFragment sqlRaceInsert(""); 
			for each(long nRaceID in PID.nsetRaceID) {
				sqlRaceInsert += CSqlFragment(
					"INSERT INTO PersonRaceT (PersonID, RaceID) " 
					"VALUES (@PersonID, {INT}) ", nRaceID); 
			}
			

			if(!sqlRaceInsert.IsEmpty()) {
				//delete superfluous values. 
				sqlRaceInsert = 
						"DELETE FROM PersonRaceT WHERE PersonID = @PersonID "
						" " 
						" " + sqlRaceInsert +
						"	" ;
			}

			// (b.spivey, June 04, 2013) - PLID 56888 - Agitatingly, I need to fidangle the SQL to run the race stuff before the update, 
			//		otherwise I'd have to move large blocks of code and add unnecessary instability for a simple problem. 
			//	    That said, the delete stuff needs this DECLARE PersonID stuff to run regardless.
			CSqlFragment sqlDeclarePersonID("DECLARE @PersonID INT "
				"SET @PersonID = {INT} ", nPersonID);


			//Need to run this query to get information for auditing. 
			_RecordsetPtr prs = CreateParamRecordset(
				"BEGIN TRAN "
				"SET NOCOUNT ON "
				"	{SQL}	"
				" "
				" "
				"SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS OldRaceName "
				"FROM "
				"( "
				"	SELECT ( " 
				"		SELECT RT.Name + ', ' "
				"		FROM PersonRaceT PRT "
				"		INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
				"		WHERE PRT.PersonID = @PersonID  "
				"		FOR XML PATH(''), TYPE "
				"	).value('/', 'nvarchar(max)') " 
				") RaceSubQ (RaceName) "
				" "
				" "
				" {SQL} "
				" "
				" "
				"SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS NewRaceName "
				"FROM "
				"( "
				"	SELECT ( " 
				"		SELECT RT.Name + ', ' "
				"		FROM PersonRaceT PRT "
				"		INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
				"		WHERE PRT.PersonID = @PersonID  "
				"		FOR XML PATH(''), TYPE "
				"	).value('/', 'nvarchar(max)') " 
				") RaceSubQ (RaceName) "
				"SET NOCOUNT OFF "
				"ROLLBACK TRAN ",
				sqlDeclarePersonID, sqlRaceInsert);

			//We should have gotten something, but better safe than sorry. 
			if(!prs->eof) {
				CString strOldValue = AdoFldString(prs, "OldRaceName", "< No Race Selected >");
				prs = prs->NextRecordset(NULL); 

				CString strNewValue = ""; 

				//This should be impossible because NxServer should never update a patient to have a blank race. 
				if(!prs->eof) {
					strNewValue = AdoFldString(prs, "NewRaceName", "");
				}

				if(setRaceIDs != PID.nsetRaceID && !strNewValue.IsEmpty()) {
					// (d.singleton 2013-08-29 16:53) - PLID 58307 - do not add the persont update text if there is no persont updates
					strUpdatePerson = sqlDeclarePersonID.Flatten() + sqlRaceInsert.Flatten() + (bPersonTUpdates ? strUpdatePerson : ""); 
					AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientRace, nPersonID, strOldValue, strNewValue, aepMedium, aetChanged);
				}
			}
		}
		
		if(strUpdatePerson == "UPDATE PersonT SET ") {
			//Nothing was changed, apparently the info that was updated wasn't anything we track.
		}
		else if(!bPersonTUpdates && strUpdatePerson != "UPDATE PersonT SET ") {
			// (d.singleton 2013-08-27 11:05) - PLID 58307 - need to handle having no update statements but insert statements from handling races
			AddStatementToSqlBatch(strSql, "%s", strUpdatePerson);
		}
		else {
			//All done!
			//TES 5/7/2008 - PLID 29685 - Batched.
			AddStatementToSqlBatch(strSql, "%s WHERE ID = %li", strUpdatePerson.Left(strUpdatePerson.GetLength()-2), nPersonID);
			//(e.lally 2007-06-15) PLID 26325 - Update the patient combo. We would unsure that all patient information
				//gets updated, but manually changing this information in G1 does not refresh the patient information where
				//one would expect it to (like the scheduler), so this is acceptable for now.
		}

		//Now, the PatientsT part.
		CString strUpdatePatient = "UPDATE PatientsT SET ";
		
		//TES 4/21/2008 - PLID 27898 - We need to audit, so we need to pull all the existing information.
		_RecordsetPtr rsExistingPatient = CreateRecordset("SELECT MaritalStatus, NickName, DefaultReferringPhyID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS RefPhysName "
			"FROM PatientsT LEFT JOIN PersonT ON PatientsT.DefaultReferringPhyID = PersonT.ID "
			"WHERE PersonID = %li", nPersonID);
		CString strMaritalStatus = AdoFldString(rsExistingPatient, "MaritalStatus", "");
		CString strNickName = AdoFldString(rsExistingPatient, "NickName", "");
		long nRefPhysID = AdoFldLong(rsExistingPatient, "DefaultReferringPhyID", -1);
		CString strRefPhysName = AdoFldString(rsExistingPatient, "RefPhysName", "");

		//TES 4/21/2008 - PLID 27898 - Went through each field, and added auditing.
		//MaritalStatus
		if(PID.nMarital != -1 && PID.nMarital != atol(strMaritalStatus)) {
			//TES 7/12/2007 - PLID 26645 - This was being put in the strUpdatePerson query, rather than strUpdatePatient.
			strUpdatePatient += "MaritalStatus = '" + _Q(AsString(PID.nMarital)) + "', ";
			CString strOldMarital;
			if(strMaritalStatus == "1") strOldMarital = "Single";
			else if(strMaritalStatus == "2") strOldMarital = "Married";
			else if(strMaritalStatus == "3") strOldMarital = "Unknown";
			CString strNewMarital;
			if(PID.nMarital == 1) strNewMarital = "Single";
			else if(PID.nMarital == 2) strNewMarital = "Married";
			else if(PID.nMarital == 3) strNewMarital = "Unknown";

			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientMaritalStatus, nPersonID, strOldMarital, strNewMarital, aepMedium, aetChanged);

		}
		
		//Nickname
		if(PID.strNickName != "" && PID.strNickName != strNickName) {
			//TES 7/12/2007 - PLID 26645 - This was being put in the strUpdatePerson query, rather than strUpdatePatient.
			strUpdatePatient += "NickName = '" + _Q(PID.strNickName) + "', ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientNickname, nPersonID, strNickName, PID.strNickName, aepMedium, aetChanged);
		}

		//DefaultReferringPhyID
		//TES 4/17/2008 - PLID 29595 - PID no longer contains any calculated fields, so let's calculate the referring
		// physician ID now.
		//TES 5/7/2008 - PLID 29685 - Passed in our SQL batch, so if it needs to create a new referring physician, it will
		// just add to our batch.
		//TES 5/23/2011 - PLID 41353 - Create an HL7ReferringPhysician struct to pass in
		HL7ReferringPhysician hrp;
		// (j.kuziel 2011-10-28 11:51) - PLID 43822 - See if we can match on the PV1 parsed referring physician ID.
		hrp.strThirdPartyID = PID.strRefPhysThirdPartyID;
		hrp.strFirst = PID.strRefPhysFirst;
		hrp.strLast = PID.strRefPhysLast;
		hrp.strDegree = PID.strRefPhysDegree;
		//TES 4/22/2015 - PLID 61147 - Added support for the prefix
		hrp.strPrefix = PID.strRefPhysPrefix;
		CString strRefID = GetNextechRefPhys(hrp, true, Message, nAuditID, strSql);
		//TES 6/9/2011 - PLID 41353 - Specify that this is for a patient's referring physician
		if(strRefID != "NULL" && (strRefID == "@nRefPhysID" || atol(strRefID) != nRefPhysID) ) {
			if(strRefID != "@nRefPhysID") {
				AddStatementToSqlBatch(strSql, "DECLARE @nRefPhysID int;");
				AddStatementToSqlBatch(strSql, "SELECT @nRefPhysID = %s;", strRefID);
				strRefID = "@nRefPhysID";
			}
			strUpdatePatient += "DefaultReferringPhyID = " + strRefID + ", ";
			AuditEvent(nPersonID, strPersonName, nAuditID, aeiPatientRefPhys, nPersonID, strRefPhysName, PID.strRefPhysLast + ", " + PID.strRefPhysFirst + " ", aepMedium, aetChanged);
		}

		if(strUpdatePatient == "UPDATE PatientsT SET ") {
			//Nothing was changed, apparently the info that was updated wasn't anything we track.
		}
		else {
			//All done!
			//TES 5/7/2008 - PLID 29685 - Batched.
			AddStatementToSqlBatch(strSql, "%s WHERE PersonID = %li", strUpdatePatient.Left(strUpdatePatient.GetLength()-2), nPersonID);
		}
		
		CNxParamSqlArray aryParams;
		// (d.singleton 2012-08-24 12:38) - PLID 51938 refactored code to new function usable by appts as well.
		if(bImportIns) {
			// (r.gonet 05/14/2013) - PLID 56675 - Added an output variable to tell us if appt insurance info has been changed. This isn't useful for patients, but since we combine the function, we pass in a dummy variable here.
			bool bInsuranceChanged = false;
			if(!HandleInsuredParty(Message, GetExistingPatientName(nPersonID), nPersonID, -1, nAuditID, bInsuranceChanged, strSql, strError, FALSE, aryParams)) {
				return FALSE;
			}
		}

		// (d.singleton 2012-08-17 15:38) - PLID 51954 rename the patient history folder,  abort if it fails to rename
		if(bPatientNameChanged) {
			// (d.singleton 2012-10-15 17:50) - PLID 53037 add params so i dont have to call GetPatientDocumentPath()
			if(!RenameHistoryFolderSilent(GetRemoteData(), nPersonID, nUserdefinedID, GetSharedPath(), strFirst, strLast, PID.strFirst, PID.strLast, strError)) {
				AfxMessageBox(strError);
				return FALSE;
			}
		}

		/*//TES 5/7/2008 - PLID 29685 - Now, execute all our batched statements at once.
		ExecuteSqlBatch(strSql);

		//TES 4/21/2008 - PLID 27898 - Commit all our audits.  If we didn't actually audit anything, then this will 
		// just close the transaction.
		CommitAuditTransaction(nAuditID);*/
		
		//TES 5/8/2008 - PLID 29685 - Instead of the above code, call FinalizeHL7Event(), which will commit our batch, 
		// as well as updating HL7MessageQueueT.
		//TES 7/17/2010 - PLID 39518 - FinalizeHL7Event() now takes a pointer to the AuditID
		//TES 6/10/2011 - PLID 41353 - This query doesn't create a new record, so there's no output variable
		// (d.singleton 2012-09-20 08:52) - PLID 51954 if the message fails need to roll back the folder rename
		if(!FinalizeHL7Event(nPersonID, Message, strSql, bSendHL7Tablechecker, NULL, &nAuditID)) {
			RollbackAuditTransaction(nAuditID);
			if(bPatientNameChanged) {
				// (d.singleton 2012-10-15 17:50) - PLID 53037 add params so i dont have to call GetPatientDocumentPath()
				RenameHistoryFolderSilent(GetRemoteData(), nPersonID, nUserdefinedID, GetSharedPath(), PID.strFirst, PID.strLast, strFirst, strLast, strError);
			}
			return FALSE;
		}

		// (b.cardillo 2009-07-01 16:19) - PLIDs 34369 and 34368 - I forgot to update qualifications when a 
		// patient is updated via HL7.  This applies to birthdate and gender only, since these are the only 
		// fields related to wellness criteria.
		if (bGenderUpdated) {
			UpdatePatientWellnessQualification_Gender(GetRemoteData(), nPersonID);
		}
		if (bBirthDateUpdated) {
			UpdatePatientWellnessQualification_Age(GetRemoteData(), nPersonID);
		}

		if(bSendPatientChecker == TRUE)
			CClient::RefreshTable(NetUtils::PatCombo, nPersonID);

		return TRUE;

 	}NxCatchAll("Error in HL7Utils::UpdateHL7Patient()");
	if(nAuditID != -1) {
		RollbackAuditTransaction(nAuditID);
		// (d.singleton 2012-09-20 09:07) - PLID 51954 if there is an exception we need to rename the patient folder back
		// (d.singleton 2012-10-04 14:30) - PLID 53036 added two params for old name values as the function cannot call GetPatientDocumentPath() anymore
		if(bPatientNameChanged) {
				// (d.singleton 2012-10-15 17:50) - PLID 53037 add params so i dont have to call GetPatientDocumentPath()
				RenameHistoryFolderSilent(GetRemoteData(), nPersonID, nUserdefinedID, GetSharedPath(), strMessageFirst, strMessageLast, strFirst, strLast, strError);
			}
	}
	return FALSE;
}

//TES 5/7/2008 - PLID 29685 - This now adds to a parameterized SQL batch if a new referring physician is needed (rather than
// just immediately creating it, as it used to).
//TES 7/15/2010 - PLID 39518 - The passed-in batch is now required to declare a variable @nNextPersonID, which must be, as the name
// implies, set to the equivalent of NewNumber("PersonT","ID").  If this function creates a new referring physician, it will
// increment @nNextPersonID.
//TES 5/23/2011 - PLID 41353 - Changed this to take in the new HL7ReferringPhysician struct, as well as nHL7GroupID.
//TES 5/23/2011 - PLID 41353 - Also added auditing, and replaced nHL7GroupID with Message
//TES 6/9/2011 - PLID 41353 - Added bForPatient, used just to give more informative messages to the user.
CString GetNextechRefPhys(const HL7ReferringPhysician &hrp, bool bForPatient, const HL7Message &Message, IN OUT long &nAuditTransactionID, IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	long nRefPhysID = -1;
	//TES 4/17/2008 - PLID 29595 - Most of this functionality was moved into the HL7ParseUtils function 
	// FindMatchingRefPhys(), which is silent, so we may need to take additional action, depending on the return value.
	HL7MatchResult hmr = FindMatchingRefPhys(GetRemoteData(), hrp, Message.nHL7GroupID, nRefPhysID);
	switch(hmr) {
	case hmrSuccess:
		{
			//TES 4/17/2008 - PLID 29595 - Great, just return the result.
			CString strResult = "NULL";
			if(nRefPhysID != -1) {
				strResult.Format("%li",nRefPhysID);
			}
			return strResult;
		}
		break;

	case hmrNoMatchFound:
		{
			//TES 4/17/2008 - PLID 29595 - This referring physician didn't exist, so prompt to create him.
			//TES 6/9/2011 - PLID 41353 - Give a different message depending on whether or not this is for a patient.
			bool bCreate = false;
			if(bForPatient) {
				//TES 4/22/2015 - PLID 61147 - Added support for the prefix
				if(IDYES == MsgBox(MB_YESNO, "This patient is listed as having the referring physician \"%s %s %s, %s.\"\n"
					"Practice does not have a referring physician by that name.  Would you like to create this referring physician?\n"
					"If Yes, the referring physician will be created and assigned to this patient.  If no, the patient will be added with no referring physician.", hrp.strPrefix, hrp.strFirst, hrp.strLast, hrp.strDegree) ) {
						bCreate = true;
				}
			}
			else {
				if(IDYES == MsgBox(MB_YESNO, "This message is attempting to update a referring physician '%s %s' that could not be found "
					"in Practice.  Would you like to create this referring physician?\n"
					"If Yes, the referring physician will be created.  If No, the import will be cancelled.", hrp.strFirst, hrp.strLast)) {
						bCreate = true;
				}
			}
			if(bCreate) {
				long nNewRefPhysID = NewNumber("PersonT", "ID");
				//TES 8/9/2007 - PLID 26883 - Parameterized.
				//TES 5/7/2008 - PLID 29685 - Batched.
				if(paryParams) {
					//TES 4/22/2015 - PLID 61147 - Added support for the prefix
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "DECLARE @nRefPhysPrefixID int;");
					if (hrp.strPrefix != "") {
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SELECT @nRefPhysPrefixID = ID FROM PrefixT WHERE Prefix = {STRING} ", hrp.strPrefix);
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "IF @nRefPhysPrefixID Is NULL BEGIN ");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, " SELECT @nRefPhysPrefixID = COALESCE(Max(ID),0)+1 FROM PrefixT; ");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "	INSERT INTO PrefixT (ID, Prefix, Gender, InformID) "
							"VALUES (@nRefPhysPrefixID, {STRING}, 0, 0)", hrp.strPrefix);
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "END ");
					}
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "DECLARE @nRefPhysID int;");
					//TES 7/12/2010 - PLID 39518 - Pull from @nNextPersonID, which is required to be declared in the passed-in batch.
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET @nRefPhysID = @nNextPersonID;");
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET @nNextPersonID = @nNextPersonID+1;");
					//TES 5/23/2011 - PLID 41353 - We've now got some extra information (potentially) in the HL7ReferringPhysician struct
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO PersonT (ID, First, Middle, Last, Title, "
						"WorkPhone, Address1, Address2, City, State, Zip, PrefixID) "
						"VALUES (@nRefPhysID, {STRING}, {STRING}, {STRING}, {STRING}, "
						"{STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, @nRefPhysPrefixID) ",
						_T(hrp.strFirst), _T(hrp.strMiddle), _T(hrp.strLast), _T(hrp.strDegree),
						_T(FormatPhoneForSql(hrp.strPhone)), _T(hrp.strAddress1), _T(hrp.strAddress2), _T(hrp.strCity), _T(hrp.strState), _T(hrp.strZip));
					//TES 8/9/2007 - PLID 26883 - Parameterized.
					//TES 5/7/2008 - PLID 29685 - Batched.
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO ReferringPhysT (PersonID) VALUES (@nRefPhysID)");
				}
				else {
					//TES 4/22/2015 - PLID 61147 - Added support for the prefix
					AddStatementToSqlBatch(strSqlBatch, "DECLARE @nRefPhysPrefixID int;");
					if (hrp.strPrefix != "") {
						AddStatementToSqlBatch(strSqlBatch, "SELECT @nRefPhysPrefixID = ID FROM PrefixT WHERE Prefix = '%s' ", _Q(hrp.strPrefix));
						AddStatementToSqlBatch(strSqlBatch, "IF @nRefPhysPrefixID Is NULL BEGIN ");
						AddStatementToSqlBatch(strSqlBatch, " SELECT @nRefPhysPrefixID = COALESCE(Max(ID),0)+1 FROM PrefixT; ");
						AddStatementToSqlBatch(strSqlBatch, "	INSERT INTO PrefixT (ID, Prefix, Gender, InformID) "
							"VALUES (@nRefPhysPrefixID, '%s', 0, 0)", _Q(hrp.strPrefix));
						AddStatementToSqlBatch(strSqlBatch, "END ");
					}
					AddStatementToSqlBatch(strSqlBatch, "DECLARE @nRefPhysID int;");
					//TES 7/12/2010 - PLID 39518 - Pull from @nNextPersonID, which is required to be declared in the passed-in batch.
					AddStatementToSqlBatch(strSqlBatch, "SET @nRefPhysID = @nNextPersonID;");
					AddStatementToSqlBatch(strSqlBatch, "SET @nNextPersonID = @nNextPersonID+1;");
					//TES 5/23/2011 - PLID 41353 - We've now got some extra information (potentially) in the HL7ReferringPhysician struct
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PersonT (ID, First, Middle, Last, Title, "
						"WorkPhone, Address1, Address2, City, State, Zip, PrefixID) "
						"VALUES (@nRefPhysID, '%s', '%s', '%s', '%s', "
						"'%s', '%s', '%s', '%s', '%s', '%s', @nRefPhysPrefixID)",
						_Q(hrp.strFirst), _Q(hrp.strMiddle), _Q(hrp.strLast), _Q(hrp.strDegree),
						_Q(FormatPhoneForSql(hrp.strPhone)), _Q(hrp.strAddress1), _Q(hrp.strAddress2), _Q(hrp.strCity), _Q(hrp.strState), _Q(hrp.strZip));
					//TES 8/9/2007 - PLID 26883 - Parameterized.
					//TES 5/7/2008 - PLID 29685 - Batched.
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ReferringPhysT (PersonID) VALUES (@nRefPhysID)");
				}
				//TES 5/23/2011 - PLID 41353 - Audit the creation
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, hrp.strLast + ", " + hrp.strFirst + " " + hrp.strMiddle, nAuditTransactionID, aeiRefPhysCreated, -1, "", hrp.strLast + ", " + hrp.strFirst + " " + hrp.strMiddle, aepMedium);

				//TES 5/23/2011 - PLID 41353 - If we have a third party ID, remember the mapping
				if(!hrp.strThirdPartyID.IsEmpty()) {
					if(paryParams) {
						AddCreateHL7IDLinkSqlToBatch(strSqlBatch, *paryParams, Message.nHL7GroupID, hilrtReferringPhysician, hrp.strThirdPartyID, "", "@nRefPhysID", hrp.strFirst, hrp.strLast);
					}
					else {
						AddCreateHL7IDLinkSqlToBatch(strSqlBatch, Message.nHL7GroupID, hilrtReferringPhysician, hrp.strThirdPartyID, "", "@nRefPhysID", hrp.strFirst, hrp.strLast);
					}
					//TES 5/23/2011 - PLID 41353 - Audit the linking
					CString strOld;
					strOld.Format("Referring Physician Code '%s' (HL7 Group %s)", hrp.strThirdPartyID, Message.strHL7GroupName);
					CString strNew;
					strNew.Format("Linked to %s, %s %s", hrp.strLast, hrp.strFirst, hrp.strMiddle);
					AuditEvent(-1, "", nAuditTransactionID, aeiHL7ReferringPhysicianLink, -1, strOld, strNew, aepMedium);
				}
				return "@nRefPhysID";
			}
			else {
				return "NULL";
			}
		}
		break;

	case hmrMultipleMatchesFound:
		{
			//TES 4/17/2008 - PLID 29595 - Hmm.  Duplicates.  Let's prompt.
			CHL7DuplicateRefPhysDlg dlg(NULL);
			//TES 6/9/2011 - PLID 41353 - If this is just a referring physician message, the record type is just "record"
			long nRefPhysID = dlg.Open(Message.nHL7GroupID, hrp.strThirdPartyID, hrp.strFirst, hrp.strLast, hrp.strDegree, false, bForPatient?"Patient":"record");
			if(nRefPhysID == -1) {
				return "NULL";
			}
			else {
				//TES 5/23/2011 - PLID 41353 - If we have a third party ID, remember the mapping
				if(!hrp.strThirdPartyID.IsEmpty()) {
					if(paryParams) {
						AddCreateHL7IDLinkSqlToBatch(strSqlBatch, *paryParams, Message.nHL7GroupID, hilrtReferringPhysician, hrp.strThirdPartyID, "", nRefPhysID, hrp.strFirst, hrp.strLast);
					}
					else {
						AddCreateHL7IDLinkSqlToBatch(strSqlBatch, Message.nHL7GroupID, hilrtReferringPhysician, hrp.strThirdPartyID, "", nRefPhysID, hrp.strFirst, hrp.strLast);
					}
					//TES 5/23/2011 - PLID 41353 - Audit the linking
					CString strOld;
					strOld.Format("Referring Physician Code '%s' (HL7 Group %s)", hrp.strThirdPartyID, Message.strHL7GroupName);
					CString strNew;
					strNew.Format("Linked to %s, %s %s", hrp.strLast, hrp.strFirst, hrp.strMiddle);
					AuditEvent(-1, "", nAuditTransactionID, aeiHL7ReferringPhysicianLink, -1, strOld, strNew, aepMedium);
				}
				return FormatString("%li", nRefPhysID);
			}
		}
		break;

	default:
		AfxThrowNxException("Invalid result %i returned in GetNextechRefPhys()", hmr);
		//TES 4/17/2008 - PLID 29595 - Need a return value to prevent compiler warnings.
		return "NULL";
		break;
	}
}

//TES 7/12/2010 - PLID 39518 - Goes through all the entries in arInsuranceSegments, and ensures that strNextechInsCoID, 
// strNextechInsContactID, and strNextechInsPlanID are all filled with valid values.
//TES 7/19/2010 - PLID 39518 - Keep in sync with HL7Support::AssignInsCoIDs() in NxServer
BOOL AssignInsCoIDs(const HL7_PIDFields &PID, long nPatientID, CArray<HL7_InsuranceFields,HL7_InsuranceFields&> &arInsuranceSegments, long nHL7GroupID, IN OUT long &nAuditTransactionID, IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	for(int i = 0; i < arInsuranceSegments.GetSize(); i++) {
		HL7_InsuranceFields Insurance = arInsuranceSegments[i];
		CArray<long,long> arInsCoIDs;
		CStringArray saInsCoNames;
		//TES 7/12/2010 - PLID 39518 - First, find everything that matches our ThirdPartyID.
		//TES 7/13/2010 - PLID 39518 - Actually, pass in the name and address, if it's successfully able to match on that, it will
		// return an array with just the one that matches.
		FindMatchingInsCo(GetRemoteData(), nHL7GroupID, Insurance.strInsCoID, Insurance.strInsCoName, Insurance.strInsCoAddress1, Insurance.strInsCoAddress2, Insurance.strInsCoCity, Insurance.strInsCoState, Insurance.strInsCoZip, arInsCoIDs, saInsCoNames);

		//TES 7/14/2010 - PLID 39518 - Track which records we've managed to map.
		bool bInsCoFound = false;
		bool bInsContactFound = false;
		bool bInsPlanFound = false;

		//TES 7/14/2010 - PLID 39518 - Declare the variables that will hold the IDs we find.
		CString strInsCoID;
		strInsCoID.Format("@nInsuranceCoID%i", i);		
		CString strInsContactID;
		strInsContactID.Format("@nInsuranceContactID%i", i);
		CString strInsPlanID;
		strInsPlanID.Format("@nInsurancePlanID%i", i);
		if(paryParams) {
			AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "DECLARE " + strInsCoID + " INT");
			AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "DECLARE " + strInsContactID + " INT");
			AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "DECLARE " + strInsPlanID + " INT");
		}
		else {
			AddStatementToSqlBatch(strSqlBatch, "DECLARE " + strInsCoID + " INT");
			AddStatementToSqlBatch(strSqlBatch, "DECLARE " + strInsContactID + " INT");
			AddStatementToSqlBatch(strSqlBatch, "DECLARE " + strInsPlanID + " INT");
		}
						
		if(arInsCoIDs.GetSize() == 1) {
			//TES 7/12/2010 - PLID 39518 - Great, we've found it.  Set our variable.
			if(paryParams) {
				AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsCoID + " = {INT}", arInsCoIDs[0]);
			}
			else {
				AddStatementToSqlBatch(strSqlBatch, "SET " + strInsCoID + " = %li", arInsCoIDs[0]);
			}
			//TES 7/15/2010 - PLID 39518 - Update the struct with this ID.
			Insurance.nNextechInsCoID = arInsCoIDs[0];
			Insurance.strNextechInsCoName = saInsCoNames[0];
			bInsCoFound = true;
		}
		else {
			//TES 7/12/2010 - PLID 39518 - OK, we'll need to do some extra work here.
			if(nPatientID != -1) {
				//TES 7/12/2010 - PLID 39518 - Check against their existing parties.
				struct InsParty {
					long nID;
					long nInsCoID;
					long nPlacement;
					CString strInsCoName;
				};
				CArray<InsParty,InsParty&> arInsParties;
				//TES 8/11/2010 - PLID 39518 - Pull the Priority, not the RespTypeID.
				_RecordsetPtr rsExistingParties = CreateParamRecordset("SELECT InsuredPartyT.PersonID, InsuredPartyT.InsuranceCoID, "
					"RespTypeT.Priority AS RespTypePriority, InsuranceCoT.Name "
					"FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE InsuredPartyT.PatientID = {INT}", nPatientID);
				while(!rsExistingParties->eof) {
					InsParty party;
					party.nID = AdoFldLong(rsExistingParties, "PersonID");
					party.nInsCoID = AdoFldLong(rsExistingParties, "InsuranceCoID");
					party.nPlacement = AdoFldLong(rsExistingParties, "RespTypePriority");
					party.strInsCoName = AdoFldString(rsExistingParties, "Name");
					arInsParties.Add(party);
					rsExistingParties->MoveNext();
				}
				//TES 7/12/2010 - PLID 39518 - See if they have any parties that are either in our list of mapped companies, or have
				// the same name.  Then if there's one with the same placement, use that one, otherwise, if there's exactly one match,
				// use that one, otherwise just move on.
				CArray<long,long> arExistingMatchingIndices;
				long nExistingMatchWithPlacementIndex = -1;
				for(int nParty = 0; nParty < arInsParties.GetSize() && nExistingMatchWithPlacementIndex == -1; nParty++) {
					InsParty party = arInsParties[nParty];
					bool bThisPartyMatches = false;
					if(party.strInsCoName.CompareNoCase(Insurance.strInsCoName) == 0) {
						bThisPartyMatches = true;
					}
					if(!bThisPartyMatches) {
						for(int j = 0; j < arInsCoIDs.GetSize() && !bThisPartyMatches; j++) {
							if(arInsCoIDs[j] == party.nInsCoID) {
								bThisPartyMatches = true;
							}
						}
					}
					if(bThisPartyMatches) {
						//TES 7/28/2014 - PLID 63075 - This was remembering i (which is an index in arInsuranceSegments), not nParty as it should have been. This was both wrong,
						// and potentially caused an exception when a message is imported with more insurance segments than the patient had parties.
						arExistingMatchingIndices.Add(nParty);
						if(party.nPlacement == i+1) {
							nExistingMatchWithPlacementIndex = nParty;
						}
					}
				}
				if(nExistingMatchWithPlacementIndex == -1) {
					//TES 7/15/2010 - PLID 39518 - We didn't match on placement, but was there only one other match?
					if(arExistingMatchingIndices.GetSize() == 1) {
						//TES 7/15/2010 - PLID 39518 - There was, so use that one.
						nExistingMatchWithPlacementIndex = arExistingMatchingIndices[0];
					}
				}
				if(nExistingMatchWithPlacementIndex != -1) {
					//TES 7/15/2010 - PLID 39518 - We found an acceptable match, let's use it.
					InsParty party = arInsParties[nExistingMatchWithPlacementIndex];
					if(paryParams) {
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsCoID + " = {INT}", party.nInsCoID);
					}
					else {
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsCoID + " = %li", party.nInsCoID);
					}
					//TES 7/15/2010 - PLID 39518 - Update the struct with this ID.
					Insurance.nNextechInsCoID = party.nInsCoID;
					Insurance.strNextechInsCoName = party.strInsCoName;
					bInsCoFound = true;
				}
			}
			if(!bInsCoFound) {
				//TES 7/12/2010 - PLID 39635 - Prompt the user, including an option to create new.
				CHL7SelectInsCoDlg dlg(NULL);
				dlg.m_strGroupName = GetHL7GroupName(nHL7GroupID);
				dlg.m_PID = PID;
				dlg.m_Insurance = Insurance;
				dlg.m_nInsurancePlacement = i+1;
				dlg.m_nPatientID = nPatientID;
				if(dlg.DoModal() != IDOK) {
					return FALSE;
				}
				//TES 7/14/2010 - PLID 39635 - We'll use this variable for auditing.
				CString strInsName = Insurance.strInsCoName;
				if(dlg.m_nInsuranceCoID == -1) {
					return FALSE;
				}
				else if(dlg.m_nInsuranceCoID == -2) {
					//TES 7/14/2010 - PLID 39635 - Create a new insurance company based on this information.
					if(paryParams) {
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsCoID + " = @nNextPersonID");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET @nNextPersonID = @nNextPersonID+1");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO PersonT (ID, UserID, Address1, Address2, City, "
							"State, Zip) "
							"VALUES (" + strInsCoID + ", {INT}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING})", 
							GetCurrentUserID(), Insurance.strInsCoAddress1, Insurance.strInsCoAddress2, Insurance.strInsCoCity, 
							Insurance.strInsCoState, Insurance.strInsCoZip);
						//TES 7/27/2010 - PLID 39635 - InsType now defaults to NULL
						// (j.jones 2012-08-08 15:51) - PLID 39220 - removed EbillingID
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO InsuranceCoT (PersonID, Name, UserDefinedID, "
							"RVUMultiplier, EBillingClaimOffice, InsType, TaxType) "
							"VALUES (" + strInsCoID + ", {STRING}, '', '', '', NULL, {INT})", 
							Insurance.strInsCoName, GetRemotePropertyInt("InsDefaultTaxType", 2, 0, "<None>",TRUE));
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO InsuranceAcceptedT "
							"(InsuranceCoID, ProviderID, Accepted) "
							"SELECT " + strInsCoID + " AS InsCoID, PersonID, {INT} AS Accepted FROM ProvidersT",
							GetRemotePropertyInt("DefaultInsAcceptAssignment",1,0,"<None>",TRUE) == 1 ? 1 : 0);

						//TES 7/14/2010 - PLID 39635 - Add a contact
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = @nNextPersonID");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET @nNextPersonID = @nNextPersonID+1");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO PersonT (ID, UserID) "
							"VALUES (" + strInsContactID + ", {INT})", GetCurrentUserID());
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO InsuranceContactsT "
							"(PersonID, InsuranceCoID, [Default]) VALUES (" + strInsContactID + ", " + strInsCoID + ",1)");

						//TES 7/14/2010 - PLID 39635 - Add a plan
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsPlanID + " = @nNextInsPlanID");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET @nNextInsPlanID = @nNextInsPlanID+1");
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "INSERT INTO InsurancePlansT (ID, PlanName, PlanType, InsCoID) "
							"VALUES (" + strInsPlanID + ", {STRING}, 'Other', " + strInsCoID + ")", Insurance.strInsCoName);
					}
					else {
						//TES 7/14/2010 - PLID 39635 - Same thing, non-parameterized version.
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsCoID + " = @nNextPersonID");
						AddStatementToSqlBatch(strSqlBatch, "SET @nNextPersonID = @nNextPersonID+1");
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PersonT (ID, UserID, Address1, Address2, City, "
							"State, Zip) "
							"VALUES (" + strInsCoID + ", %li, '%s', '%s', '%s', '%s', '%s')", 
							GetCurrentUserID(), _Q(Insurance.strInsCoAddress1), _Q(Insurance.strInsCoAddress2), _Q(Insurance.strInsCoCity), 
							_Q(Insurance.strInsCoState), _Q(Insurance.strInsCoZip));
						//TES 7/27/2010 - PLID 39635 - InsType now defaults to NULL
						// (j.jones 2012-08-08 15:51) - PLID 39220 - removed EbillingID
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceCoT (PersonID, Name, UserDefinedID, "
							"RVUMultiplier, EBillingClaimOffice, InsType, TaxType) "
							"VALUES (" + strInsCoID + ", '%s', '', '', '', NULL, %li)", 
							_Q(Insurance.strInsCoName), GetRemotePropertyInt("InsDefaultTaxType", 2, 0, "<None>",TRUE));
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceAcceptedT "
							"(InsuranceCoID, ProviderID, Accepted) "
							"SELECT " + strInsCoID + " AS InsCoID, PersonID, %li AS Accepted FROM ProvidersT",
							GetRemotePropertyInt("DefaultInsAcceptAssignment",1,0,"<None>",TRUE) == 1 ? 1 : 0);

						//TES 7/14/2010 - PLID 39635 - Add a contact
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = @nNextPersonID");
						AddStatementToSqlBatch(strSqlBatch, "SET @nNextPersonID = @nNextPersonID+1");
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PersonT (ID, UserID) "
							"VALUES (" + strInsContactID + ", %li)", GetCurrentUserID());
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsuranceContactsT "
							"(PersonID, InsuranceCoID, [Default]) VALUES (" + strInsContactID + ", " + strInsCoID + ",1)");

						//TES 7/14/2010 - PLID 39635 - Add a plan
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsPlanID + " = @nNextInsPlanID");
						AddStatementToSqlBatch(strSqlBatch, "SET @nNextInsPlanID = @nNextInsPlanID+1");
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InsurancePlansT (ID, PlanName, PlanType, InsCoID) "
							"VALUES (" + strInsPlanID + ", '%s', 'Other', " + strInsCoID + ")", _Q(Insurance.strInsCoName));
					}
					//TES 7/15/2010 - PLID 39635 - Update the struct, indicating that we've created a new insurance company.
					Insurance.nNextechInsCoID = -2;
					Insurance.strNextechInsCoName = Insurance.strInsCoName;

					//TES 7/15/2010 - PLID 39635 - Likewise for the contact and plan.
					Insurance.nNextechInsContactID = -2;
					Insurance.strNextechInsContactName = Insurance.strContactFirst + " " + Insurance.strContactLast;
					Insurance.nNextechInsPlanID = -2;
					Insurance.strNextechInsPlan = Insurance.strInsCoName;

					//TES 7/14/2010 - PLID 39635 - We've now set all our variables.
					bInsCoFound = true;
					bInsContactFound = true;
					bInsPlanFound = true;

				}
				else {
					//TES 7/14/2010 - PLID 39635 - Remember the name, set our InsCoID
					strInsName = dlg.m_strInsuranceCoName;
					if(paryParams) {
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsCoID + " = {INT}", dlg.m_nInsuranceCoID);
					}
					else {
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsCoID + " = %li", dlg.m_nInsuranceCoID);
					}
					//TES 7/15/2010 - PLID 39635 - Update the struct with this ID
					Insurance.nNextechInsCoID = dlg.m_nInsuranceCoID;
					Insurance.strNextechInsCoName = dlg.m_strInsuranceCoName;
					bInsCoFound = true;					
				}
				ASSERT(bInsCoFound);
				if(!Insurance.strInsCoID.IsEmpty()) {
					//TES 7/14/2010 - PLID 39635 - Remember and audit this link
					// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
					CreateNewHL7CodeLinkT_WithVariableName(nHL7GroupID, hclrtInsCo, Insurance.strInsCoID, strInsCoID, strSqlBatch, paryParams);

					CString strOld, strNew;
					strOld.Format("Insurance Company Code '%s' (HL7 Group '%s')", Insurance.strInsCoID, GetHL7GroupName(nHL7GroupID));
					strNew.Format("Linked to %s", strInsName);
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginNewAuditEvent();
					}
					AuditEvent(-1, "", nAuditTransactionID, aeiHL7InsCoLink, nHL7GroupID, strOld, strNew, aepLow, aetCreated);
				}
			}
		}

		//TES 7/12/2010 - PLID 39518 - Now if we've got an insurance company but no insurance contact, get a contact.
		if(bInsCoFound && !bInsContactFound) {
			//TES 7/12/2010 - PLID 39518 - We're not going to mess around with prompting or anything here.  First, 
			// gather the contacts for this company.
			_RecordsetPtr rsContacts = CreateParamRecordset("SELECT InsuranceContactsT.PersonID, InsuranceContactsT.[Default], "
				"PersonT.First, PersonT.Last, PersonT.WorkPhone, PersonT.Extension, "
				"convert(bit,CASE WHEN InsuranceContactsT.PersonID IN "
				" (SELECT InsuranceContactID FROM InsuredPartyT WHERE InsuredPartyT.PatientID = {INT} "
				"AND InsuredPartyT.InsuranceCoID = {INT}) THEN 1 ELSE 0 END) AS IsExistingContact "
				"FROM PersonT INNER JOIN InsuranceContactsT ON PersonT.ID = InsuranceContactsT.PersonID "
				"WHERE InsuranceContactsT.InsuranceCoID = {INT}", nPatientID, Insurance.nNextechInsCoID, Insurance.nNextechInsCoID);
			struct HL7_InsuranceContact {
				long nPersonID;
				bool bIsDefault;
				CString strFirst;
				CString strLast;
				CString strPhone;
				CString strExtension;
				int nMatch;
				bool bExistingContact;
			};
			CArray<HL7_InsuranceContact,HL7_InsuranceContact&> arContacts;
			while(!rsContacts->eof) {
				HL7_InsuranceContact Contact;
				Contact.nPersonID = AdoFldLong(rsContacts, "PersonID");
				Contact.bIsDefault = AdoFldBool(rsContacts, "Default")?true:false;
				Contact.strFirst = AdoFldString(rsContacts, "First");
				Contact.strLast = AdoFldString(rsContacts, "Last");
				Contact.strPhone = AdoFldString(rsContacts, "WorkPhone");
				Contact.strExtension = AdoFldString(rsContacts, "Extension");
				Contact.bExistingContact = AdoFldBool(rsContacts, "IsExistingContact")?true:false;
				arContacts.Add(Contact);
				rsContacts->MoveNext();
			}
			//TES 7/12/2010 - PLID 39518 - Now, if there's just one, use it.
			if(arContacts.GetSize() == 1) {
				if(paryParams) {
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = {INT}", arContacts[0].nPersonID);
				}
				else {
					AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = %li", arContacts[0].nPersonID);
				}
				bInsContactFound = true;

				//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
				Insurance.nNextechInsContactID = arContacts[0].nPersonID;
				Insurance.strNextechInsContactName = arContacts[0].strFirst + " " + arContacts[0].strLast;
			}
			else {
				//TES 7/15/2010 - PLID 39518 - If this is for an existing patient, and they have a contact for this ID, see if it 
				// doesn't match.
				bool bExistingMismatch = false;
				long nExistingPersonID = -1;
				CString strExistingName;
				if(nPatientID != -1 && Insurance.nNextechInsCoID > 0) {
					for(int nContact = 0; nContact < arContacts.GetSize() && !bExistingMismatch; nContact++) {
						HL7_InsuranceContact Contact = arContacts[nContact];
						if(Contact.bExistingContact) {
							nExistingPersonID = arContacts[nContact].nPersonID;
							strExistingName = arContacts[nContact].strFirst + " " + arContacts[nContact].strLast;
							if(!Insurance.strContactFirst.IsEmpty() && !Contact.strFirst.IsEmpty() && Insurance.strContactFirst.CompareNoCase(Contact.strFirst)) {
								bExistingMismatch = true;
							}
							if(!bExistingMismatch && !Insurance.strContactLast.IsEmpty() && !Contact.strLast.IsEmpty() && Insurance.strContactLast.CompareNoCase(Contact.strLast)) {
								bExistingMismatch = true;
							}
							CString strPhone1 = Insurance.strContactPhone;
							CString strPhone2 = Contact.strPhone;
							StripNonNumericChars(strPhone1);
							StripNonNumericChars(strPhone2);
							if(!bExistingMismatch && !strPhone1.IsEmpty() && !strPhone2.IsEmpty() && strPhone1 != strPhone2) {
								bExistingMismatch = true;
							}
							if(!bExistingMismatch && !Insurance.strContactExt.IsEmpty() && !Contact.strExtension.IsEmpty() && Insurance.strContactExt.CompareNoCase(Contact.strExtension)) {
								bExistingMismatch = true;
							}
						}
					}
				}
				if(!bExistingMismatch && nExistingPersonID != -1) {
					//TES 7/15/2010 - PLID 39518 - The existing one matches all right, so we'll use it..
					if(paryParams) {
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = {INT}", nExistingPersonID);
					}
					else {
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = %li", nExistingPersonID);
					}

					//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
					Insurance.nNextechInsContactID = nExistingPersonID;
					Insurance.strNextechInsContactName = strExistingName;
					bInsContactFound = true;
				}
				else {
					//TES 7/12/2010 - PLID 39518 - Find the default, see if it doesn't match.
					bool bDefaultMismatch = false;
					long nDefaultPersonID = -1;
					CString strDefaultName;
					for(int nContact = 0; nContact < arContacts.GetSize() && !bDefaultMismatch; nContact++) {
						HL7_InsuranceContact Contact = arContacts[nContact];
						if(Contact.bIsDefault) {
							nDefaultPersonID = arContacts[nContact].nPersonID;
							strDefaultName = arContacts[nContact].strFirst + " " + arContacts[nContact].strLast;
							if(!Insurance.strContactFirst.IsEmpty() && !Contact.strFirst.IsEmpty() && Insurance.strContactFirst.CompareNoCase(Contact.strFirst)) {
								bDefaultMismatch = true;
							}
							if(!bDefaultMismatch && !Insurance.strContactLast.IsEmpty() && !Contact.strLast.IsEmpty() && Insurance.strContactLast.CompareNoCase(Contact.strLast)) {
								bDefaultMismatch = true;
							}
							CString strPhone1 = Insurance.strContactPhone;
							CString strPhone2 = Contact.strPhone;
							StripNonNumericChars(strPhone1);
							StripNonNumericChars(strPhone2);
							if(!bDefaultMismatch && !strPhone1.IsEmpty() && !strPhone2.IsEmpty() && strPhone1 != strPhone2) {
								bDefaultMismatch = true;
							}
							if(!bDefaultMismatch && !Insurance.strContactExt.IsEmpty() && !Contact.strExtension.IsEmpty() && Insurance.strContactExt.CompareNoCase(Contact.strExtension)) {
								bDefaultMismatch = true;
							}
						}
					}
					if(!bDefaultMismatch && nDefaultPersonID != -1) {
						//TES 7/12/2010 - PLID 39518 - Great, we'll just use the default.
						if(paryParams) {
							AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = {INT}", nDefaultPersonID);
						}
						else {
							AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = %li", nDefaultPersonID);
						}
						//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
						Insurance.nNextechInsContactID = nDefaultPersonID;
						Insurance.strNextechInsContactName = strDefaultName;
						bInsContactFound = true;
					}
					else {
						//TES 7/12/2010 - PLID 39518 - The default doesn't match, see if any of the others match.
						for(int nContact = 0; nContact < arContacts.GetSize() && !bInsContactFound; nContact++) {
							HL7_InsuranceContact Contact = arContacts[nContact];
							if(!Contact.bIsDefault) {
								bool bMismatch = false;
								//TES 7/12/2010 - PLID 39518 - If this contact is completely empty, don't use it (use the default).
								if(Contact.strFirst.IsEmpty() && Contact.strLast.IsEmpty() && Contact.strPhone.IsEmpty() && Contact.strExtension.IsEmpty()) {
									bMismatch = true;
								}
								if(!bMismatch && !Insurance.strContactFirst.IsEmpty() && !Contact.strFirst.IsEmpty() && Insurance.strContactFirst.CompareNoCase(Contact.strFirst)) {
									bMismatch = true;
								}
								if(!bMismatch && !Insurance.strContactLast.IsEmpty() && !Contact.strLast.IsEmpty() && Insurance.strContactLast.CompareNoCase(Contact.strLast)) {
									bMismatch = true;
								}
								CString strPhone1 = Insurance.strContactPhone;
								CString strPhone2 = Contact.strPhone;
								StripNonNumericChars(strPhone1);
								StripNonNumericChars(strPhone2);
								if(!bDefaultMismatch && !strPhone1.IsEmpty() && !strPhone2.IsEmpty() && strPhone1 != strPhone2) {
									bMismatch = true;
								}
								if(!bMismatch && !Insurance.strContactExt.IsEmpty() && !Contact.strExtension.IsEmpty() && Insurance.strContactExt.CompareNoCase(Contact.strExtension)) {
									bMismatch = true;
								}
								if(!bMismatch) {
									//TES 7/12/2010 - PLID 39518 - This one matches, and the default doesn't, so use this one.
									if(paryParams) {
										AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = {INT}", Contact.nPersonID);
									}
									else {
										AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = %li", Contact.nPersonID);
									}
									//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
									Insurance.nNextechInsContactID = Contact.nPersonID;
									Insurance.strNextechInsContactName = Contact.strFirst + " " + Contact.strLast;
									bInsContactFound = true;
								}
							}
						}
						if(!bInsContactFound) {
							//TES 7/12/2010 - PLID 39518 - OK, we haven't found a match, so just use the default.
							if(nDefaultPersonID != -1) {
								if(paryParams) {
									AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = {INT}", nDefaultPersonID);
								}
								else {
									AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = %li", nDefaultPersonID);
								}
								//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
								Insurance.nNextechInsContactID = nDefaultPersonID;
								Insurance.strNextechInsContactName = strDefaultName;
								bInsContactFound = true;
							}
							else {
								//TES 7/12/2010 - PLID 39518 - If there's no default, just use the first one.  There DOES have to be at 
								// least one contact, otherwise it would cause errors other places than just here.
								if(paryParams) {
									AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsContactID + " = {INT}", arContacts[0].nPersonID);
								}
								else {
									AddStatementToSqlBatch(strSqlBatch, "SET " + strInsContactID + " = %li", arContacts[0].nPersonID);
								}
								//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
								Insurance.nNextechInsContactID = arContacts[0].nPersonID;
								Insurance.strNextechInsContactName = arContacts[0].strFirst + " " + arContacts[0].strLast;
								bInsContactFound = true;
							}
						}
					}
				}
			}
		}

		//TES 7/12/2010 - PLID 39518 - Now, if we have a company ID but not a plan ID, figure out the plan ID
		if(bInsCoFound && !bInsPlanFound) {
			//TES 7/15/2010 - PLID 39518 - First, check if this patient already has a plan, and has the same type (if there are any plans
			// with the same type), and if so, use it.
			if(nPatientID != -1 && Insurance.nNextechInsCoID > 0) {
				_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT TOP 1 ID, PlanName FROM InsurancePlansT WHERE InsCoID = {INT} "
					"AND ID IN (SELECT InsPlan FROM InsuredPartyT WHERE PatientID = {INT} AND InsuranceCoID = {INT}) "
					"AND (PlanType = {STRING} "
					"OR NOT EXISTS (SELECT ID FROM InsurancePlansT WHERE InsCoID = {INT} AND PlanType = {STRING}))",
					Insurance.nNextechInsCoID, nPatientID, Insurance.nNextechInsCoID, Insurance.strPlanType, 
					Insurance.nNextechInsCoID, Insurance.strPlanType);
				if(!rsInsPlan->eof) {
					//TES 7/15/2010 - PLID 39518 - Got one.
					long nID = AdoFldLong(rsInsPlan, "ID");
					CString strName = AdoFldString(rsInsPlan, "PlanName");
					if(paryParams) {
						AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsPlanID + " = {INT}", nID);
					}
					else {
						AddStatementToSqlBatch(strSqlBatch, "SET " + strInsPlanID + " = %li", nID);
					}
					//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
					Insurance.nNextechInsPlanID = nID;
					Insurance.strNextechInsPlan = strName;
					bInsPlanFound = true;
				}
			}
			if(!bInsPlanFound) {
				//TES 7/12/2010 - PLID 39518 - Just take either the first plan with the same PlanType, or the first plan, if none have the
				// same PlanType			
				_RecordsetPtr rsInsPlan = CreateParamRecordset("SELECT TOP 1 ID, PlanName FROM InsurancePlansT WHERE InsCoID = {INT} "
					"AND (PlanType = {STRING} "
					"OR NOT EXISTS (SELECT ID FROM InsurancePlansT WHERE InsCoID = {INT} AND PlanType = {STRING}))",
					Insurance.nNextechInsCoID, Insurance.strPlanType, Insurance.nNextechInsCoID, Insurance.strPlanType);
				//TES 7/12/2010 - PLID 39518 - There has to be at least one plan!
				ASSERT(!rsInsPlan->eof);
				long nID = AdoFldLong(rsInsPlan, "ID");
				CString strName = AdoFldString(rsInsPlan, "PlanName");
				if(paryParams) {
					AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "SET " + strInsPlanID + " = {INT}", nID);
				}
				else {
					AddStatementToSqlBatch(strSqlBatch, "SET " + strInsPlanID + " = %li", nID);
				}
				//TES 7/15/2010 - PLID 39518 - Update the struct with the ID and name
				Insurance.nNextechInsPlanID = nID;
				Insurance.strNextechInsPlan = strName;
				bInsPlanFound = true;
			}
		}

		//TES 7/14/2010 - PLID 39518 - We shouldn't be able to get here without having set all three variables.
		ASSERT(bInsCoFound && bInsContactFound && bInsPlanFound);

		//TES 7/15/2010 - PLID 39635 - Update the array so it has the ID in it.
		arInsuranceSegments.SetAt(i, Insurance);
	}
	return TRUE;
}

//TES 9/18/2008 - PLID 31414 - Takes the given information pulled from an HL7 message, and matches it to a ProvidersT record
// in Nextech.  Interacts with the user.  Pass in strRecord and strField with user-meaningful names describing what field
// in the HL7 message this provider is from.
//TES 10/21/2008 - PLID 21432 - Made the strSqlBatch parameter optional (if it's not filled, then it will go ahead and
// just execute any needed queries), and also added an optional output parameter for whether the function in fact created
// a new provider record.
// (j.jones 2010-05-13 10:34) - PLID 36527 - now this needs to report whether the HL7IDLinkT needs audited
CString GetNextechProvider(const CString &strProvThirdPartyID, const CString &strProvFirst, const CString &strProvMiddle, const CString &strProvLast, long nHL7GroupID, OPTIONAL IN OUT CString *pstrSqlBatch /*= NULL*/, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/, const CString &strRecord /*= "Lab Result"*/, const CString &strField /*= "Ordering Provider"*/, OPTIONAL OUT bool *pbRecordCreated /*= NULL*/, OPTIONAL OUT bool *pbHL7IDLinkT_NeedAudit /*= NULL*/)
{
	//TES 10/21/2008 - PLID 21432 - Initialize our output parameter, if any.
	if(pbRecordCreated) {
		*pbRecordCreated = false;
	}

	// (j.jones 2010-05-13 10:43) - PLID 36527 - initialize the output parameter indicating that the HL7IDLinkT needs audited
	if(pbHL7IDLinkT_NeedAudit) {
		*pbHL7IDLinkT_NeedAudit = false;
	}

	//TES 9/18/2008 - PLID 31414 - This code is basically copied off of GetNextechRefPhys() above.
	long nProviderID = -1;
	//TES 4/17/2008 - PLID 29595 - Most of this functionality was moved into the HL7ParseUtils function 
	// FindMatchingRefPhys(), which is silent, so we may need to take additional action, depending on the return value.
	HL7MatchResult hmr = FindMatchingProvider(GetRemoteData(), strProvThirdPartyID, strProvFirst, strProvMiddle, strProvLast, nHL7GroupID, nProviderID);
	switch(hmr) {
	case hmrSuccess:
		{
			//TES 4/17/2008 - PLID 29595 - Great, just return the result.
			CString strResult = "NULL";
			if(nProviderID != -1) {
				strResult.Format("%li",nProviderID);
			}
			return strResult;
		}
		break;

	case hmrNoMatchFound:
		{
			//TES 9/18/2008 - PLID 31414 - This provider didn't exist, so prompt to create him.  Use our passed-in 
			// descriptions for what role this provider is fulfilling in the HL7 message.
			//TES 10/21/2008 - PLID 21432 - Reworded this message so that if they click no, it will use "default settings,"
			// not no provider.
			//TES 12/9/2008 - PLID 32371 - Give them the opportunity to link this record to an existing provider,
			// rather than just creating a new one (though they'll still have that option).
			// (k.messina 2009-10-29 12:45) - PLID 38631 - fix typo in m_strCaption
			CSelectDlg dlg(NULL);
			dlg.m_strTitle = "Select Provider";
			dlg.m_strCaption = "You are attempting to import a " + strRecord + " with the " + strField + " code '" + strProvThirdPartyID + "' (Name: " + strProvLast + ", " + strProvFirst + " " + strProvMiddle+ ").\r\n"
				"Please select a Practice provider that should correspond with this code (you may need to contact your software vendor for assistance).\r\n\r\n"
				"If you cancel this dialog, the " + strField + " field will be filled using your default settings.", 
			dlg.m_strFromClause = "(SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE (Archived = 0 OR ID IN "
				"(SELECT PersonID FROM HL7IDLinkT WHERE RecordType = " + AsString((long)hilrtProvider) + ")) "
				"UNION SELECT -1, '<Create New Provider>') AS ProvidersQ ";
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			DatalistColumn dcID = dlg.m_arColumns[0];
			dcID.nSortPriority = 0;
			dlg.m_arColumns.SetAt(0,dcID);
			dlg.AddColumn("Name", "Provider", TRUE, TRUE);
			if(dlg.DoModal() == IDOK) {
				ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
				nProviderID = VarLong(dlg.m_arSelectedValues[0]);
				if(nProviderID == -1) {
					//TES 8/9/2007 - PLID 26883 - Parameterized.
					//TES 5/7/2008 - PLID 29685 - Batched.
					if(pstrSqlBatch) {
						if(paryParams) {
							AddParamStatementToSqlBatch(*pstrSqlBatch, *paryParams, "DECLARE @nProviderID int;");
							AddParamStatementToSqlBatch(*pstrSqlBatch, *paryParams, "SELECT @nProviderID = Max(ID)+1 FROM PersonT;");
							AddParamStatementToSqlBatch(*pstrSqlBatch, *paryParams, "INSERT INTO PersonT (ID, First, Middle, Last) VALUES (@nProviderID, {STRING}, {STRING}, {STRING})",
								_T(strProvFirst), _T(strProvMiddle), _T(strProvLast));
							if(!strProvThirdPartyID.IsEmpty()) {
								AddCreateHL7IDLinkSqlToBatch(*pstrSqlBatch, *paryParams, nHL7GroupID, hilrtProvider, strProvThirdPartyID, "", "@nProviderID", strProvFirst, strProvLast);

								// (j.jones 2010-05-13 10:40) - PLID 36527 - set the output parameter indicating that the HL7IDLinkT needs audited
								if(pbHL7IDLinkT_NeedAudit) {
									*pbHL7IDLinkT_NeedAudit = true;
								}
							}
							//TES 8/9/2007 - PLID 26883 - Parameterized.
							//TES 5/7/2008 - PLID 29685 - Batched.
							//TES 11/3/2008 - PLID 31414 - Added the ClaimProviderID to this branch.
							AddParamStatementToSqlBatch(*pstrSqlBatch, *paryParams, "INSERT INTO ProvidersT (PersonID, ClaimProviderID) VALUES (@nProviderID, @nProviderID)");
						}
						else {
							AddStatementToSqlBatch(*pstrSqlBatch, "DECLARE @nProviderID int;");
							AddStatementToSqlBatch(*pstrSqlBatch, "SELECT @nProviderID = Max(ID)+1 FROM PersonT;");
							AddStatementToSqlBatch(*pstrSqlBatch, "INSERT INTO PersonT (ID, First, Middle, Last) VALUES (@nProviderID, '%s', '%s', '%s')",
								_Q(strProvFirst), _Q(strProvMiddle), _Q(strProvLast));
							if(!strProvThirdPartyID.IsEmpty()) {
								AddCreateHL7IDLinkSqlToBatch(*pstrSqlBatch, nHL7GroupID, hilrtProvider, strProvThirdPartyID, "", "@nProviderID", strProvFirst, strProvLast);

								// (j.jones 2010-05-13 10:40) - PLID 36527 - set the output parameter indicating that the HL7IDLinkT needs audited
								if(pbHL7IDLinkT_NeedAudit) {
									*pbHL7IDLinkT_NeedAudit = true;
								}
							}
							//TES 8/9/2007 - PLID 26883 - Parameterized.
							//TES 5/7/2008 - PLID 29685 - Batched.
							AddStatementToSqlBatch(*pstrSqlBatch, "INSERT INTO ProvidersT (PersonID, ClaimProviderID) VALUES (@nProviderID, @nProviderID )");
						}
						return "@nProviderID";
					}
					else {
						//TES 10/21/2008 - PLID 21432 - Let's just go ahead and create the new provider ourselves.
						CString strSqlBatch;
						CNxParamSqlArray aryLocalParams;
						AddParamStatementToSqlBatch(strSqlBatch, aryLocalParams, "DECLARE @nProviderID int;");
						AddParamStatementToSqlBatch(strSqlBatch, aryLocalParams, "SELECT @nProviderID = Max(ID)+1 FROM PersonT;");
						AddParamStatementToSqlBatch(strSqlBatch, aryLocalParams, "INSERT INTO PersonT (ID, First, Middle, Last) VALUES (@nProviderID, {STRING}, {STRING}, {STRING})",
							strProvFirst, strProvMiddle, strProvLast);
						if(!strProvThirdPartyID.IsEmpty()) {
							AddCreateHL7IDLinkSqlToBatch(strSqlBatch, aryLocalParams, nHL7GroupID, hilrtProvider, strProvThirdPartyID, "", "@nProviderID", strProvFirst, strProvLast);

							// (j.jones 2010-05-13 10:40) - PLID 36527 - set the output parameter indicating that the HL7IDLinkT needs audited
							if(pbHL7IDLinkT_NeedAudit) {
								*pbHL7IDLinkT_NeedAudit = true;
							}
						}
						//TES 8/9/2007 - PLID 26883 - Parameterized.
						//TES 5/7/2008 - PLID 29685 - Batched.
						AddParamStatementToSqlBatch(strSqlBatch, aryLocalParams, "INSERT INTO ProvidersT (PersonID, ClaimProviderID) VALUES (@nProviderID, @nProviderID )");

						CString strRecordset;
						strRecordset.Format(
							"SET NOCOUNT ON \r\n"
							"BEGIN TRAN \r\n"
							"%s "
							"COMMIT TRAN \r\n"
							"SET NOCOUNT OFF \r\n"
							"SELECT @nProviderID AS ProviderID \r\n",
							strSqlBatch);
						
						// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
						_RecordsetPtr rsProviderID = CreateParamRecordsetBatch(GetRemoteData(), strRecordset, aryLocalParams);
						long nProviderID = AdoFldLong(rsProviderID, "ProviderID");
						//TES 10/21/2008 - PLID 21432 - Fill our output parameter saying that we created the record.
						if(pbRecordCreated) {
							*pbRecordCreated = true;
						}
						return AsString(nProviderID);
					}
				}
				else {
					//TES 12/9/2008 - PLID 32371 - They selected an existing provider, use them.
					//  First, if we have a third-party ID, map the IDs together.
					if(!strProvThirdPartyID.IsEmpty()) {
						if(pstrSqlBatch) {
							if(paryParams) {
								AddCreateHL7IDLinkSqlToBatch(*pstrSqlBatch, *paryParams, nHL7GroupID, hilrtProvider, strProvThirdPartyID, "", nProviderID, strProvFirst, strProvLast);

								// (j.jones 2010-05-13 10:40) - PLID 36527 - set the output parameter indicating that the HL7IDLinkT needs audited
								if(pbHL7IDLinkT_NeedAudit) {
									*pbHL7IDLinkT_NeedAudit = true;
								}
							}
							else {
								AddCreateHL7IDLinkSqlToBatch(*pstrSqlBatch, nHL7GroupID, hilrtProvider, strProvThirdPartyID, "", "@nProviderID", strProvFirst, strProvLast);

								// (j.jones 2010-05-13 10:40) - PLID 36527 - set the output parameter indicating that the HL7IDLinkT needs audited
								if(pbHL7IDLinkT_NeedAudit) {
									*pbHL7IDLinkT_NeedAudit = true;
								}
							}
						}
						else {
							CreateHL7IDLinkRecord(GetRemoteData(), nHL7GroupID, strProvThirdPartyID, nProviderID, hilrtProvider, strProvFirst, strProvLast, "");

							// (j.jones 2010-05-13 10:40) - PLID 36527 - set the output parameter indicating that the HL7IDLinkT needs audited
							if(pbHL7IDLinkT_NeedAudit) {
								*pbHL7IDLinkT_NeedAudit = true;
							}
						}
					}
					//TES 12/9/2008 - PLID 32371 - Now, return the selected ID.
					return AsString(nProviderID);
				}
			}
			else {
				return "NULL";
			}
		}
		break;

	case hmrMultipleMatchesFound:
		{
			//TES 4/17/2008 - PLID 29595 - Hmm.  Duplicates.  Let's prompt.
			//TES 10/8/2008 - PLID 31414 - I went ahead and modified this dialog to be useable for providers as well as
			// referring physicians.  We'll pass in "Lab Result" as the type of record being modified, and false for
			// bIsRefPhys, indicating that we want providers instead of referring physicians.
			CHL7DuplicateRefPhysDlg dlg(NULL);
			long nProvID = dlg.Open(nHL7GroupID, strProvThirdPartyID, strProvFirst, strProvLast, "" , false, "Lab Result", false);
			if(nProvID == -1) {
				return "NULL";
			}
			else {
				return FormatString("%li", nProvID);
			}
			return "NULL";
		}
		break;

	default:
		AfxThrowNxException("Invalid result %i returned in GetNextechProvider()", hmr);
		//TES 4/17/2008 - PLID 29595 - Need a return value to prevent compiler warnings.
		return "NULL";
		break;
	}
}

// (z.manning 2010-07-01 10:16) - PLID 39422
long GetNextechResource(HL7Message message, const CString &strResourceThirdPartyID, const CString &strResourceName)
{
	long nResourceID = -1;
	HL7MatchResult hmr = FindMatchingResource(GetRemoteData(), message.nHL7GroupID, strResourceThirdPartyID, nResourceID);
	switch(hmr)
	{
		case hmrSuccess:
			return nResourceID;
			break;

		case hmrNoMatchFound:
			// (z.manning 2010-07-01 10:43) - PLID 39422 - Prompt to link to a resource
			CSelectDlg dlg(NULL);
			dlg.m_strTitle = "Select Resource";
			dlg.m_strCaption = "You are attempting to import a scheduler resource with the code '" + strResourceThirdPartyID + "' (Name: " + strResourceName + ").\r\n"
				"Please select a NexTech resource that should correspond with this code (you may need to contact your software vendor for assistance).\r\n\r\n"
				"If you cancel this dialog, the HL7 message will not get imported.", 
			dlg.m_strFromClause = FormatString(
				"(SELECT ID, Item FROM ResourceT \r\n"
				"WHERE (Inactive = 0 OR ID IN (SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %li)) \r\n"
				"UNION \r\n"
				"SELECT -1, '<Create New Resource>') AS ResourceQ \r\n"
				, hclrtResource);
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			dlg.AddColumn("Item", "Resource", TRUE, TRUE);
			DatalistColumn dc = dlg.m_arColumns[1];
			dc.nSortPriority = 0;
			dlg.m_arColumns.SetAt(1, dc);
			if(dlg.DoModal() == IDOK)
			{
				nResourceID = dlg.m_arSelectedValues.GetAt(0);
				CAuditTransaction auditTran;
				if(nResourceID == -1)
				{
					// (z.manning 2010-07-01 11:53) - PLID 39422 - We need to create a new resource
					// First off, does a resource with this name already exist?
					if(ReturnsRecords("SELECT ID FROM ResourceT WHERE Item = '%s'",_Q(strResourceName))) {
						::MessageBox(NULL, FormatString("Cannot create new resource because there is already a resource named '%s.'", strResourceName), NULL, MB_OK);
						return -1;
					}

					CParamSqlBatch sqlBatch;
					sqlBatch.Add("SET NOCOUNT ON");
					sqlBatch.Declare("DECLARE @nResourceID int");
					sqlBatch.Add("SET @nResourceID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ResourceT)");
					sqlBatch.Add(
						"INSERT INTO ResourceT (Item, ID, ItemType) \r\n"
						"VALUES ({STRING}, @nResourceID, {STRING}) "
						, strResourceName, "Doctor");
					sqlBatch.Add(
						"INSERT INTO ResourcePurposeTypeT (ResourceID, AptTypeID, AptPurposeID) \r\n"
						"SELECT @nResourceID, AptTypeID, AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeID NOT IN (SELECT ProcedureT.ID FROM ProcedureT WHERE Inactive = 1) "
						);
					// Add the resource to all custom views
					sqlBatch.Add(
						"INSERT INTO ResourceViewDetailsT (ResourceViewID, ResourceID, Relevence) \r\n"
						"SELECT ID AS ResourceViewID, @nResourceID AS ResourceID, -1 FROM ResourceViewsT "
						);
					// Add the resource to all users' built-in views
					sqlBatch.Add(
						"INSERT INTO UserResourcesT (UserID, ResourceID, Relevence) "
						"SELECT PersonID AS UserID, @nResourceID AS ResourceID, -1 FROM UsersT"
						);

					sqlBatch.Add("SET NOCOUNT OFF");
					sqlBatch.Add("SELECT @nResourceID AS ResourceID");
					_RecordsetPtr prsResource = sqlBatch.CreateRecordset(GetRemoteData());
					nResourceID = AdoFldLong(prsResource->GetFields(), "ResourceID");

					CClient::RefreshTable(NetUtils::Resources, nResourceID);
					AddUserDefinedPermission(nResourceID, 63, strResourceName, "Controls access to view or schedule appointments for this resource.", -94, 21);
				}
				
				if(nResourceID == -1) {
					return nResourceID;
				}

				// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
				ExecuteParamSql("{SQL}", CreateNewHL7CodeLinkT(message.nHL7GroupID, hclrtResource, strResourceThirdPartyID, nResourceID));

				CString strOld, strNew;
				strOld.Format("Resource Code '%s' (HL7 Group '%s')", strResourceThirdPartyID, message.strHL7GroupName);
				strNew.Format("Linked to %s", strResourceName);
				AuditEvent(-1, "", auditTran, aeiHL7ResourceLink, message.nHL7GroupID, strOld, strNew, aepLow, aetCreated);

				auditTran.Commit();

				return nResourceID;
			}
			break;
	}

	return -1;
}

//TES 10/8/2008 - PLID 21093 - Needed to get locations for Labs, so I split GetHL7Location() out into its own function.  Once
// I'd done that, I realized that the lab needed its own function, because it pulls different records and has a different
// prompt, so I added a "copy" called GetHL7LabLocation(), but I'm keeping them as separate functions, both to make
// the functions that call them more readable, and also because changes to the logic of one will likely require similar
// changes in the logic of the other, and this way they're right next to each other.
BOOL GetHL7Location(const HL7Message &Message, const CString &strThirdPartyID, OUT long &nLocationID)
{
	// (j.dinatale 2011-10-21 10:39) - PLID 46048 - exception handling placed here, if an exception happens in here, it is likely because
	//		Hl7CodeLinkT has bad data
	try{
		//Have we already mapped this code?
		//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT, with location mappings defined by the Type enum.
		// (z.manning 2010-07-14 16:54) - PLID 39422 - Now have a shared function to look this up
		if(FindMatchingLocation(GetRemoteData(), Message.nHL7GroupID, strThirdPartyID, nLocationID) == hmrSuccess) {
			return TRUE;
		}
		else {
			//We need to prompt them.
			CSelectDlg dlg(NULL);
			dlg.m_strTitle = "Select Location";
			dlg.m_strCaption = "You are attempting to import a bill with the location code '" + strThirdPartyID + "'.\r\n"
				"Please select a Practice location that should correspond with this location code (you may need to contact your software vendor).\r\n\r\n"
				"If you later decide to map this code to a different location, you can do so by going to Tools->HL7 Settings and selecting 'Configure Locations'.\r\n"
				"If you do not wish to receive this prompt any more, and would like to use the standard billing preferences for bills imported from HL7, you can go to Tools->HL7 Settings, and uncheck the box mark 'Use locations from imported bills'.\r\n"
				"If you cancel this dialog, Practice will use your standard billing preferences to assign this bill's location, but you will continue to be prompted when bills with this code are imported.";
			//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLink.
			dlg.m_strFromClause.Format("(SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND (Active = 1 OR ID IN "
				"(SELECT PracticeID FROM HL7CodeLinkT WHERE ID = %i)) UNION SELECT -1, '{Use Practice Default}') AS LocationsQ", hclrtLocation);
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			DatalistColumn dcID = dlg.m_arColumns[0];
			dcID.nSortPriority = 0;
			dlg.m_arColumns.SetAt(0,dcID);
			dlg.AddColumn("Name", "Location", TRUE, TRUE);
			if(dlg.DoModal() == IDOK) {
				ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
				nLocationID = VarLong(dlg.m_arSelectedValues[0]);

				// (j.dinatale 2011-10-20 12:28) - PLID 46048 - insert into the table, if the user doesnt select "Use Practice Default"
				// Dont make a HL7CodeLinkT record if there is no 3rd party code to link with. This also means dont audit if the code is empty.
				if(nLocationID != -1 && !strThirdPartyID.IsEmpty()) {

					//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
					// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
					ExecuteParamSql("{SQL}", CreateNewHL7CodeLinkT(Message.nHL7GroupID, hclrtLocation, strThirdPartyID, nLocationID));

					// (j.jones 2010-05-12 11:07) - PLID 36527 - audit this
					long nAuditID = BeginNewAuditEvent();

					// (j.dinatale 2011-10-20 15:40) - PLID 46048 - no longer worry about the -1 case, we dont link locations
					//		if "Use Practice Default" is selected
					CString strLocationName;
					_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
					if(!rsLocation->eof) {
						strLocationName = AdoFldString(rsLocation, "Name");
					}
					rsLocation->Close();

					CString strOld, strNew;
					strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, Message.strHL7GroupName);
					strNew.Format("Linked to %s", strLocationName);
					AuditEvent(-1, "", nAuditID, aeiHL7LocationLink, Message.nHL7GroupID, strOld, strNew, aepLow, aetCreated);
				}
			}

			// even if they hit cancel, we can return true.
			return TRUE;
		}
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (j.dinatale 2011-10-19 16:11) - PLID 44823 - made a new function to get the location ID for an appointment
//		The reason for this is because the {Use Practice Default} option was broken and it was intended to utilize
//		the "Default Location on New Appointments" preference. The only trouble is, that preference dictates how the UI
//		should behave in certain circumstances and is not suitable to determine what the default location should be.
//		So now, we force the user to select a location, if they dont we dont import the message.
BOOL GetHL7ApptLocation(const HL7Message &Message, const CString &strThirdPartyID, OUT long &nLocationID)
{
	// (j.dinatale 2011-10-21 10:39) - PLID 46048 - exception handling placed here, if an exception happens in here, it is likely because
	//		Hl7CodeLinkT has bad data
	try{
		//Have we already mapped this code?
		//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT, with location mappings defined by the Type enum.
		// (z.manning 2010-07-14 16:54) - PLID 39422 - Now have a shared function to look this up
		if(FindMatchingLocation(GetRemoteData(), Message.nHL7GroupID, strThirdPartyID, nLocationID) == hmrSuccess) {
			return TRUE;
		} else {
			// (j.dinatale 2011-10-19 16:38) - PLID 44823 - for whatever reason, we were prompting the user telling them they were importing
			//		a bill when they are importing an appointment, so went ahead and fixed this for appointments.
			//We need to prompt them.
			CSelectDlg dlg(NULL);
			dlg.m_strTitle = "Select Location";
			dlg.m_strCaption = "You are attempting to import an appointment with the location code '" + strThirdPartyID + "'.\r\n"
				"Please select a Practice location that should correspond with this location code (you may need to contact your software vendor).\r\n\r\n"
				"If you later decide to map this code to a different location, you can do so by going to Tools->HL7 Settings and selecting 'Configure Locations'.\r\n"
				"If you cancel this dialog, the HL7 message will not get imported.";
			//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLink.
			dlg.m_strFromClause.Format("(SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND (Active = 1 OR ID IN "
				"(SELECT PracticeID FROM HL7CodeLinkT WHERE ID = %i))) AS LocationsQ", hclrtLocation);
			dlg.AddColumn("ID", "ID", FALSE, FALSE);
			DatalistColumn dcID = dlg.m_arColumns[0];
			dcID.nSortPriority = 0;
			dlg.m_arColumns.SetAt(0,dcID);
			dlg.AddColumn("Name", "Location", TRUE, TRUE);
			if(dlg.DoModal() == IDOK) {
				ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
				nLocationID = VarLong(dlg.m_arSelectedValues[0]);

				// (j.dinatale 2011-10-20 12:28) - PLID 46048 - dont insert into CodeLinkT if there is no 3rd party code to link with
				//		This also means dont audit if the code is empty. Also got rid of the check for nLocationID = -1, its not possible
				//		now that the Select dlg doesnt have a the "Use Practice Default" option.
				if(!strThirdPartyID.IsEmpty()) {

					//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
					// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
					ExecuteParamSql("{SQL}", CreateNewHL7CodeLinkT(Message.nHL7GroupID, hclrtLocation, strThirdPartyID, nLocationID));

					// (j.jones 2010-05-12 11:07) - PLID 36527 - audit this
					long nAuditID = BeginNewAuditEvent();

					CString strLocationName;
					_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
					if(!rsLocation->eof) {
						strLocationName = AdoFldString(rsLocation, "Name");
					}
					rsLocation->Close();

					CString strOld, strNew;
					strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, Message.strHL7GroupName);
					strNew.Format("Linked to %s", strLocationName);
					AuditEvent(-1, "", nAuditID, aeiHL7LocationLink, Message.nHL7GroupID, strOld, strNew, aepLow, aetCreated);
				}

				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
	 
	return FALSE;
}

BOOL GetHL7LabLocationID(const HL7Message &Message, const CString &strThirdPartyID, OUT long &nLocationID)
{
	// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
	LogHL7Debug("Attempting to match the HL7 lab location (Third Party ID = \"%s\") with an existing Nextech lab location.",
		strThirdPartyID);

	//Have we already mapped this code?
	//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
	_RecordsetPtr rsLocation = CreateRecordset("SELECT PracticeID FROM HL7CodeLinkT "
		"WHERE HL7GroupID = %li AND ThirdPartyCode = '%s' AND Type = %i AND PracticeID IN (SELECT ID FROM LocationsT WHERE TypeID = 2)", 
		Message.nHL7GroupID, _Q(strThirdPartyID), hclrtLocation);
	if(!rsLocation->eof) {
		//Great, we've got it.
		nLocationID = AdoFldLong(rsLocation, "PracticeID", -1);
		rsLocation->Close();
		// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
		LogHL7Debug("The HL7 lab location is linked to an existing Nextech lab location (ID = %li).", nLocationID);
		return TRUE;
	}
	else {
		// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
		LogHL7Warning("The HL7 lab location is not linked to any existing Nextech lab location. Nextech needs to prompt the user to select a lab location.");
		//We need to prompt them.
		rsLocation->Close();
		CSelectDlg dlg(NULL);
		dlg.m_strTitle = "Select Location";
		//TES 12/3/2008 - PLID 32301 - Changed the message to indicate that it cannot proceed without choosing a
		// Receiving Lab location.
		dlg.m_strCaption = "You are attempting to import a result from a lab whose location code is '" + strThirdPartyID + "'.\r\n"
			"Please select a Receiving Lab location in Practice that should correspond with this location code (you may need to contact your software vendor).\r\n\r\n"
			"If you later decide to map this code to a different location, you can do so by going to Tools->HL7 Settings and selecting 'Configure Locations'.\r\n"
			"If you cancel this dialog,  this message will not be imported.";
		//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
		dlg.m_strFromClause.Format("(SELECT ID, Name FROM LocationsT WHERE TypeID = 2 AND (Active = 1 OR ID IN "
			"(SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %i))) AS LocationsQ", hclrtLocation);
		dlg.AddColumn("ID", "ID", FALSE, FALSE);
		DatalistColumn dcID = dlg.m_arColumns[0];
		dcID.nSortPriority = 0;
		dlg.m_arColumns.SetAt(0,dcID);
		dlg.AddColumn("Name", "Location", TRUE, TRUE);
		if(dlg.DoModal() == IDOK) {
			ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
			nLocationID = VarLong(dlg.m_arSelectedValues[0]);

			// (j.dinatale 2011-10-20 12:28) - PLID 46048 - dont insert into CodeLinkT if there is no 3rd party code to link with
			//		This also means dont audit if the code is empty. Also, no need to check for -1, the value is not even possible here.
			if(!strThirdPartyID.IsEmpty()) {

				//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
				// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
				ExecuteParamSql("{SQL}", CreateNewHL7CodeLinkT(Message.nHL7GroupID, hclrtLocation, strThirdPartyID, nLocationID));

				// (j.jones 2010-05-12 11:07) - PLID 36527 - audit this
				long nAuditID = BeginNewAuditEvent();

				CString strLocationName;
				_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
				if(!rsLocation->eof) {
					strLocationName = AdoFldString(rsLocation, "Name");
				}
				rsLocation->Close();

				CString strOld, strNew;
				strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, Message.strHL7GroupName);
				strNew.Format("Linked to %s", strLocationName);
				AuditEvent(-1, "", nAuditID, aeiHL7LocationLink, Message.nHL7GroupID, strOld, strNew, aepLow, aetCreated);
			} else {
				// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
				LogHL7Warning("Could not link the HL7 third party lab location ID to the Nextech lab location because the HL7 third party ID is blank.");
			}

			return TRUE;
		}
	}
	return FALSE;
}



// (j.armen 2014-03-26 08:47) - PLID 61517 - Helper to ensure a diag code exists in data
// Executed after all codes have been extracted for a single charge
void EnsureDiagCode(HL7DiagCode* pCode, const HL7Message& Message, const HL7DiagCodeDefaults& defaults)
{
	const bool bIsICD10 = (pCode->strSystem == defaults.strCodeSystemFlag10);

	//We might have been given the decimal point, or we might not.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, CodeNumber, CodeDesc\r\n"
		"FROM DiagCodes\r\n"
		"WHERE ICD10 = {BIT} AND (CodeNumber = {STRING} OR REPLACE(CodeNumber, '.', '') = {STRING})",
		bIsICD10, pCode->strCode, pCode->strCode);

	if(prs->eof) {
		//DRT 1/16/2007 - PLID 24177 - We can no longer allow the potential for | in diag codes.  Throw an exception.
		if(pCode->strCode.Find("|") > -1)
			AfxThrowNxException("The character '|' is not allowed to be imported in diagnosis codes.");

		// Hopefully we will have received a description, if not, we'll need a place holder
		if(pCode->strDesc.IsEmpty())
			pCode->strDesc = "<From HL7 Import>";

		// (d.thompson 2014-02-14) - PLID 60716 - I moved creating new diag codes to the API for ICD-10.  Let's use it here.
		pCode->nDiagID = CreateNewAdminDiagnosisCode(pCode->strCode, pCode->strDesc, false, false, bIsICD10);
	}
	else {
		// (j.armen 2014-03-26 08:47) - PLID 61517 - Set the ID
		// Override Code and Description from the database
		pCode->nDiagID = AdoFldLong(prs, "ID");
		pCode->strCode = AdoFldString(prs, "CodeNumber");
		pCode->strDesc = AdoFldString(prs, "CodeDesc");
	}
}

// (b.savon 2015-12-22 15:00) - PLID 67783 - Support creating a HL7 bill with exclusively linking FT1 diag codes
CChargeWhichCodesMapPtr TraverseDiagCodes(const vector<shared_ptr<pair<shared_ptr<HL7DiagCode>, shared_ptr<HL7DiagCode>>>>& aryCodePairs, const HL7DiagCodeDefaults& defaults, CBillingModuleDlg *pDlg)
{
	CChargeWhichCodesMapPtr pMap = make_shared<CChargeWhichCodesMap>();
	for each(const shared_ptr<pair<shared_ptr<HL7DiagCode>, shared_ptr<HL7DiagCode>>> pItem in aryCodePairs)
	{
		CChargeWhichCodePair pair;
		DiagCodeInfoPtr pInfo = make_shared<DiagCodeInfo>();
		pInfo->nID = -1;

		if (pItem->first)
		{
			if (pItem->first->strSystem == defaults.strCodeSystemFlag9) // First Code is I9
			{
				pair.first = pItem->first->nDiagID;
				pInfo->nDiagCode9ID = pItem->first->nDiagID;
				pInfo->strDiagCode9Code = pItem->first->strCode;
				pInfo->strDiagCode9Desc = pItem->first->strDesc;

				if (pItem->second) // Second Code is I10
				{
					pInfo->nOrderIndex = pDlg->m_dlgBilling.AddDiagCode(
						pItem->first->nDiagID, pItem->second->nDiagID,
						pItem->first->strCode, pItem->second->strCode,
						pItem->first->strDesc, pItem->second->strDesc,
						TRUE);

					pair.second = pItem->second->nDiagID;
					pInfo->nDiagCode10ID = pItem->second->nDiagID;
					pInfo->strDiagCode10Code = pItem->second->strCode;
					pInfo->strDiagCode10Desc = pItem->second->strDesc;
				}
				else // Second code does not exist
				{
					pInfo->nOrderIndex = pDlg->m_dlgBilling.AddDiagCode(
						pItem->first->nDiagID, -1,
						pItem->first->strCode, "",
						pItem->first->strDesc, "",
						TRUE);

					pair.second = -1;
					pInfo->nDiagCode10ID = -1;
					pInfo->strDiagCode10Code = "";
					pInfo->strDiagCode10Desc = "";
				}
			}
			else // First Code is I10
			{
				pair.second = pItem->first->nDiagID;
				pInfo->nDiagCode10ID = pItem->first->nDiagID;
				pInfo->strDiagCode10Code = pItem->first->strCode;
				pInfo->strDiagCode10Desc = pItem->first->strDesc;

				if (pItem->second) // Second Code is I9
				{
					pInfo->nOrderIndex = pDlg->m_dlgBilling.AddDiagCode(
						pItem->second->nDiagID, pItem->first->nDiagID,
						pItem->second->strCode, pItem->first->strCode,
						pItem->second->strDesc, pItem->first->strDesc,
						TRUE);

					pair.first = pItem->second->nDiagID;
					pInfo->nDiagCode9ID = pItem->second->nDiagID;
					pInfo->strDiagCode9Code = pItem->second->strCode;
					pInfo->strDiagCode9Desc = pItem->second->strDesc;
				}
				else // Second code does not exist
				{
					pInfo->nOrderIndex = pDlg->m_dlgBilling.AddDiagCode(
						-1, pItem->first->nDiagID,
						"", pItem->first->strCode,
						"", pItem->first->strDesc,
						TRUE);

					pair.first = -1;
					pInfo->nDiagCode9ID = -1;
					pInfo->strDiagCode9Code = "";
					pInfo->strDiagCode9Desc = "";
				}
			}
		}
		else // First code does not exist
		{
			if (pItem->second->strSystem == defaults.strCodeSystemFlag9) // Second code is I9
			{
				pair.first = pItem->second->nDiagID;
				pInfo->nDiagCode9ID = pItem->second->nDiagID;
				pInfo->strDiagCode9Code = pItem->second->strCode;
				pInfo->strDiagCode9Desc = pItem->second->strDesc;

				pInfo->nOrderIndex = pDlg->m_dlgBilling.AddDiagCode(
					pItem->second->nDiagID, -1,
					pItem->first->strCode, "",
					pItem->first->strDesc, "",
					TRUE);

				pair.second = -1;
				pInfo->nDiagCode10ID = -1;
				pInfo->strDiagCode10Code = "";
				pInfo->strDiagCode10Desc = "";
			}
			else // Second code is I10
			{
				pair.first = -1;
				pInfo->nDiagCode9ID = -1;
				pInfo->strDiagCode9Code = "";
				pInfo->strDiagCode9Desc = "";

				pInfo->nOrderIndex = pDlg->m_dlgBilling.AddDiagCode(
					-1, pItem->second->nDiagID,
					"", pItem->second->strCode,
					"", pItem->second->strDesc,
					TRUE);

				pair.second = pItem->second->nDiagID;
				pInfo->nDiagCode10ID = pItem->second->nDiagID;
				pInfo->strDiagCode10Code = pItem->second->strCode;
				pInfo->strDiagCode10Desc = pItem->second->strDesc;
			}
		}

		(*pMap)[pair] = pInfo;
	}

	return pMap;
}

//TES 5/8/2008 - PLID 29685 - Changed to use the new HL7Message structure.
// (j.jones 2008-05-19 15:35) - PLID 30110 - added a parameter to disable sending tablecheckers for HL7 events
// (j.gruber 2016-01-29 12:27) - PLID 68006 - added insured party
BOOL CreateHL7Bill(const HL7Message &Message, CWnd *pMessageParent, const CString &strVersion, BOOL bSendHL7Tablechecker, long nInsuredPartyID /*=-1*/, BillFromType eFromType /* = BillFromType::HL7Batch*/)
{
	CBillingModuleDlg *pDlg;
	try {
		// (b.eyers 2015-06-16) - PLID 66206
		BOOL bEnableIntelleChart = GetHL7SettingBit(Message.nHL7GroupID, "EnableIntelleChart");

		//Make sure we're on a supported version.
		//NOTE: Some versions of Medinotes apparently generate 2.3 messages; this hasn't caused any problems yet, so it's probably the same as 2.4 for our purposes.
		// (b.savon 2014-12-09 07:45) - PLID 64316 - Remove restriction that doesn't allow 2.5.1 DFT messages to be accepted.
		if(strVersion != "2.3" && strVersion != "2.4" && strVersion != "2.5" && strVersion != "2.5.1") {
			MessageBox(pMessageParent->GetSafeHwnd(), "This HL7 message was created using an unsupported version of the HL7 Standard (version " + strVersion + ").  No action will be taken.", "Unsupported Version", MB_OK);
			return FALSE;
		}

		//TES 7/12/2007 - PLID 26642 - This code was moved to a separate function, so that UpdateHL7Patient() could also
		// use it.  We pass in false, because we don't want them to be able to create a new patient (because we're not sure
		// that we have a full patient record in this message).
		//TES 9/18/2008 - PLID 31414 - Renamed
		long nPersonID = GetPatientFromHL7Message(Message.strMessage, Message.nHL7GroupID, pMessageParent);
		if(nPersonID == -1) {
			//TES 7/12/2007 - PLID 26642 - The message couldn't be parsed, or the user cancelled.
			return FALSE;
		}

		long nLocationID = -1;
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		if(GetHL7SettingBit(Message.nHL7GroupID, "UseImportedLocation") || GetHL7SettingBit(Message.nHL7GroupID, "UseImportedLocationAsPOS")) {
			//TES 11/2/2006 - We also need to pull in the location.
			CString strLocationCode;
			if(COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "PV1", 1, 3, 1, 1, strLocationCode)) {
				//TES 10/8/2008 - PLID 21093 - Split into its own function.
				// (j.dinatale 2011-10-21 11:21) - PLID 46048 - if GetHL7Location returns false, then a problem happened,
				//		return FALSE.
				if(!GetHL7Location(Message, strLocationCode, nLocationID)){
					return FALSE;
				}
			}
		}
		// (s.tullis 2015-07-20 15:14) - PLID 66187 - Use the MDI POS if intellichart is enabled
		long nMDILocationID = -1;
		if (bEnableIntelleChart)
		{
			CString strLocationCode;
			if (COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", 1, 16, 1, 1, strLocationCode)) {
				if (!GetHL7Location(Message, strLocationCode, nMDILocationID)){
					return FALSE;
				}
			}
		}
		
		// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
		ASSERT(pMessageParent);
		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return FALSE;
		}

		//Somehow initialize billingmodule dlg.
		if(!GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
			return FALSE;
		}
		// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
		GetMainFrame()->m_patToolBar.UpdatePatient(nPersonID);
		//TES 1/7/2010 - PLID 36761 - This function may fail now
		if(!GetMainFrame()->m_patToolBar.TrySetActivePatientID(nPersonID)) {
			return FALSE;
		}
		CPatientView *pView = ((CPatientView*)(GetMainFrame()->GetActiveView()));
		if(!pView) return FALSE;
		//TES 10/12/2006 - PLID 23023 - Make sure the rest of the screen is up to date, not just the toolbar.
		pView->UpdateView();
		pDlg = pView->GetBillingDlg();
		if(!pDlg) return FALSE;
		CWaitCursor cuWait;
		
		//TES 12/19/2006 - PLID 23930 - We don't want to permanently change the override, just for this one bill.
		long nOldOverrideLocationID = pDlg->m_dlgBilling.m_nOverrideLocationID;
		long nOldOverridePOSID = pDlg->m_dlgBilling.m_nOverridePOSID;
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		if(GetHL7SettingBit(Message.nHL7GroupID, "UseImportedLocation")) pDlg->m_dlgBilling.m_nOverrideLocationID = nLocationID;
		if(GetHL7SettingBit(Message.nHL7GroupID, "UseImportedLocationAsPOS")) pDlg->m_dlgBilling.m_nOverridePOSID = nLocationID;
		// (s.tullis 2015-07-20 15:14) - PLID 66187 - Use the MDI POS if intellichart is enabled
		// (s.tullis 2016-03-23 13:31) - PLID 68628 - Removed Setting current location here only need override
		if (bEnableIntelleChart && nMDILocationID != -1)
		{
			pDlg->m_dlgBilling.m_nOverrideLocationID = nMDILocationID;
		}

		pDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, eFromType, nInsuredPartyID);

		pDlg->m_dlgBilling.m_nOverrideLocationID = nOldOverrideLocationID;
		pDlg->m_dlgBilling.m_nOverridePOSID = nOldOverridePOSID;

		pDlg->EnableWindow(FALSE);
		pDlg->m_nPatientID = nPersonID;

		// (r.farnworth 2014-12-10 14:36) - PLID 64081 - If you cancel a bill from a HL7 charge message (DFT) that was commited, Nextech considers the message as imported.
		pDlg->m_nMessageQueueID = Message.nMessageID;

		// (b.eyers 2015-06-16) - PLID 66206
		BOOL bInventoryItemFound = FALSE;

		// (v.maida 2015-08-27 10:42) - PLID 66962 - Created a vector to hold all charge dates.
		vector<COleDateTime> vecChargeDates;

		//Loop through each charge.
		int nCharge = 1;
		int nChargeStart, nChargeEnd;
		CString strCode;
		while(COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 25, 1, 1, strCode)) {
			//TES 9/2/2011 - PLID 44024 - Deformat
			strCode = DeformatHL7Text(strCode);
			long nServiceID;

			// (b.eyers 2015-06-16) - PLID 66206 - Only check inventory if intellechart is enabled
			if (bEnableIntelleChart) {
				_RecordsetPtr rsInvOverrideID = CreateParamRecordset("SELECT PracticeID FROM HL7CodeLinkT WHERE Type = 14 AND ThirdPartyCode = {STRING}", strCode);

				if (!rsInvOverrideID->eof) {
					//if the code isn't inv, then go check service codes, codes not found will be created as a service code
					nServiceID = AdoFldLong(rsInvOverrideID, "PracticeID");
					bInventoryItemFound = TRUE;
				}

			}

			// (b.eyers 2015-06-16) - PLID 66206 - if no inventory found (or not checked), proceed to checking charges
			if (!bInventoryItemFound) {

				// (r.farnworth 2015-01-20 16:06) - PLID 64625 - When importing Charges through HL7, search for an override CPT Code. If one exists, use that instead of what's in the message
				_RecordsetPtr rsOverrideID = CreateRecordset("SELECT PracticeID FROM HL7CodeLinkT WHERE Type = 13 AND ThirdPartyCode = '%s'", _Q(strCode));
				//long nServiceID;

				if (rsOverrideID->eof) {

					_RecordsetPtr rsServiceID = CreateRecordset("SELECT ID FROM CPTCodeT WHERE Code = '%s'", _Q(strCode));

					if (rsServiceID->eof) {
						//We need to create it.
						CString strDesc;
						if (COMPONENT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 25, 1, 2, strDesc)) {

							strDesc = "<From HL7 Import>";

							// (r.farnworth 2014-12-11 14:30) - PLID 64327 - If we couldn't find a description in FT1 25.2, then let's check PR1 3.2
							CString strPR1Code;

							//Check the PR1 segment in the message and obtain their service code
							if (COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "PR1", nCharge, 3, 1, 1, strPR1Code)) {
								strPR1Code = DeformatHL7Text(strPR1Code);
								//Compare the service code of the PR1 and FT1 to make sure we have the correct PR1 message
								if (strPR1Code == strCode) {
									//Check for a description, if we find one. Use it.
									CString strTempDesc;
									if (COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "PR1", nCharge, 3, 1, 2, strTempDesc)) {
										if (!strTempDesc.IsEmpty()){
											strDesc = DeformatHL7Text(strTempDesc);
										}
									}
								}
							}
							//If our loop doesn't return anything, strDesc is still set to <From HL7 Import>
						}
						else {
							//TES 9/2/2011 - PLID 44024 - Deformat
							strDesc = DeformatHL7Text(strDesc);
						}

						nServiceID = NewNumber("ServiceT", "ID");
						ExecuteSql("INSERT INTO ServiceT (ID, Name) VALUES (%li, '%s')", nServiceID, _Q(strDesc.Left(255)));
						ExecuteSql("INSERT INTO CPTCodeT (ID, Code, SubCode) VALUES (%li, '%s', '')", nServiceID, _Q(strCode.Left(50)));
						// (j.gruber 2012-12-04 11:39) - PLID 48566 - ServiceInfoLocationT
						ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID) \r\n"
							"SELECT {INT}, ID FROM LocationsT WHERE Managed = 1 "
							, nServiceID);
						CClient::RefreshTable(NetUtils::CPTCodeT, nServiceID);
						//I know this is sketchy, but Josh said it should be all right.
						pDlg->m_dlgBilling.m_CPTCombo->Requery();
					}
					else {
						nServiceID = AdoFldLong(rsServiceID, "ID");
					}
				}
				// (r.farnworth 2015-01-20 16:18) - PLID 64625 - When importing Charges through HL7, search for an override CPT Code. If one exists, use that instead of what's in the message
				else {
					nServiceID = AdoFldLong(rsOverrideID, "PracticeID");
				}

			}

			//TES 8/28/2006 - This logic was backwards, expecting the newer format if they had 2.3/2.4, and the older format if 
			// they had 2.5.  I've checked, and in all the cases I can find that we actually link to, they use the new, DG1 segment
			// format for diagnosis codes.  I'm taking out the other branch altogether, unless and until we find some software
			// that actually uses it.
			//TES 10/21/2008 - PLID 31757 - MedFlow uses the old method (actually, it appears to also throw some DG1
			// segments in here and there, but it's flagged as version 2.3, and the version 2.3 fields appeared to be
			// filled correctly, whereas the DG1 segments are somewhat haphazard).  I went back and looked at our other
			// V2.3 samples (from DrNotes), and they also support the 2.3 format.  So I'm restoring the ability to 
			// process the V2.3 format (I had to fix up a couple of things, but mostly it was just a matter of uncommenting
			// that code).  I limited it to 2.3, since we don't have any 2.4 to test.

			//First, where does this charge start?
			nChargeStart = FindNthOccurrence(Message.strMessage, "\rFT1", nCharge);
			if(nChargeStart == -1) {
				//I bet they did it the non-standard way.
				nChargeStart = FindNthOccurrence(Message.strMessage, "\nFT1", nCharge);
			}
			//Now, where does this charge end?
			nChargeEnd = FindNthOccurrence(Message.strMessage, "\rFT1", nCharge+1);
			if(nChargeEnd == -1) {
				//I bet they did it the non-standard way.
				nChargeEnd = FindNthOccurrence(Message.strMessage, "\nFT1", nCharge+1);
			}
			if(nChargeEnd == -1)
				nChargeEnd = Message.strMessage.GetLength();

			// (j.armen 2014-03-26 08:47) - PLID 61517 - Tracks the unique pairs of diag codes for this charge
			vector<shared_ptr<pair<shared_ptr<HL7DiagCode>, shared_ptr<HL7DiagCode>>>> aryCodePairs;

			// (j.armen 2014-03-26 08:47) - PLID 61517 - Tracks all diag codes for this charge
			vector<shared_ptr<HL7DiagCode>> aryCodes;

			// (j.gruber 2016-01-29 14:54) - PLID 68127 - changed a bit because I moved to HL7ParseUtils
			const HL7DiagCodeDefaults defaults(Message,
				GetHL7SettingText(Message.nHL7GroupID, "DefaultCodeSystem"),
				GetHL7SettingText(Message.nHL7GroupID, "DefaultAltCodeSystem"),
				GetHL7SettingText(Message.nHL7GroupID, "CodeSystemFlag9"),
				GetHL7SettingText(Message.nHL7GroupID, "CodeSystemFlag10"));

			// (j.armen 2014-03-26 08:47) - PLID 61517 - Gather all codes from the current FT1 segment
			GetFT1DiagCodes(aryCodePairs, aryCodes, Message, nChargeStart, nChargeEnd, defaults);

			// (j.armen 2014-03-26 08:47) - PLID 61517 - Gather all codes from the DG1 segments following the previous FT1 segment and before the next FT1 segment
			// (b.savon 2015-12-22 09:55) - PLID 67783 - Support creating a HL7 bill with exclusively linking FT1 diag codes
			if (GetHL7SettingBit(Message.nHL7GroupID, "ExclusivelyLinkFTandDG") == FALSE) {
				GetDG1DiagCodes(aryCodePairs, aryCodes, Message, nChargeStart, nChargeEnd, defaults);
			}

			// (j.armen 2014-03-26 08:47) - PLID 61517 - Iterate through our list of diag codes, and ensure they are in data
			for each(shared_ptr<HL7DiagCode> pCode in aryCodes) {
				EnsureDiagCode(pCode.get(), Message, defaults);
			}

			// (j.armen 2014-03-26 08:47) - PLID 61517 - Construct a map of codes to be added to the bill.
			// (b.savon 2015-12-22 15:00) - PLID 67783 - Support creating a HL7 bill with exclusively linking FT1 diag codes
			CChargeWhichCodesMapPtr pMap = TraverseDiagCodes(aryCodePairs, defaults, pDlg);

			//TES 10/20/2008 - PLID 21432 - Performed By, FT1-20
			CString strProviderID, strProviderLast, strProviderFirst, strProviderMiddle;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 20, 1, 1, strProviderID);
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 20, 1, 2, strProviderLast);
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 20, 1, 3, strProviderFirst);
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 20, 1, 4, strProviderMiddle);
			//TES 9/2/2011 - PLID 44024 - Deformat
			strProviderLast = DeformatHL7Text(strProviderLast);
			strProviderFirst = DeformatHL7Text(strProviderFirst);
			strProviderMiddle = DeformatHL7Text(strProviderMiddle);

			bool bRecordCreated = false;
			// (j.jones 2010-05-13 10:43) - PLID 36527 - added a parameter indicating that the HL7IDLinkT needs audited
			bool bHL7IDLinkT_NeedAudit = false;

			CString strProviderPersonID = GetNextechProvider(strProviderID, strProviderFirst, strProviderMiddle, strProviderLast, Message.nHL7GroupID, NULL, NULL, "charge", "provider", &bRecordCreated, &bHL7IDLinkT_NeedAudit);
			long nProviderID = -1;
			if(strProviderPersonID != "NULL") {
				nProviderID = atol(strProviderPersonID);
				ASSERT(nProviderID > 0);
			}
			if(bRecordCreated) {
				pDlg->m_dlgBilling.m_List->GetColumn(5)->ComboSource = pDlg->m_dlgBilling.m_List->GetColumn(5)->ComboSource;
			}

			// (j.jones 2010-05-13 10:49) - PLID 36527 - audit the HL7IDLinkT creation
			if(bHL7IDLinkT_NeedAudit && nProviderID > 0) {
				long nAuditID = BeginNewAuditEvent();
				CString strOld, strNew, strName;
				_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", nProviderID);
				if(!rsProv->eof) {
					strName = AdoFldString(rsProv, "Name", "");
				}
				rsProv->Close();
				strOld.Format("Provider Code '%s' (HL7 Group '%s')", strProviderID, Message.strHL7GroupName);
				strNew.Format("Linked to Provider '%s'", strName);
				AuditEvent(nProviderID, strName, nAuditID, aeiHL7ProviderLink, nProviderID, strOld, strNew, aepLow, aetCreated);
			}

			// (b.eyers 2015-06-19) - PLID 66208 - get ordering provider from FT1-21, mdi only
			long nOrdProviderID = -1; 

			if (bEnableIntelleChart) {
				CString strOrdProviderID, strOrdProviderLast, strOrdProviderFirst, strOrdProviderMiddle;
				GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 21, 1, 1, strOrdProviderID);
				GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 21, 1, 2, strOrdProviderLast);
				GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 21, 1, 3, strOrdProviderFirst);
				GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 21, 1, 4, strOrdProviderMiddle);
				strOrdProviderLast = DeformatHL7Text(strOrdProviderLast);
				strOrdProviderFirst = DeformatHL7Text(strOrdProviderFirst);
				strOrdProviderMiddle = DeformatHL7Text(strOrdProviderMiddle);

				bRecordCreated = false;
				bHL7IDLinkT_NeedAudit = false;

				CString strOrdProviderPersonID = GetNextechProvider(strOrdProviderID, strOrdProviderFirst, strOrdProviderMiddle, strOrdProviderLast, Message.nHL7GroupID, NULL, NULL, "charge", "ordering provider", &bRecordCreated, &bHL7IDLinkT_NeedAudit);
				if (strOrdProviderPersonID != "NULL") {
					nOrdProviderID = atol(strOrdProviderPersonID);
					ASSERT(nOrdProviderID > 0);
				}
				if (bRecordCreated) {
					pDlg->m_dlgBilling.m_List->GetColumn(5)->ComboSource = pDlg->m_dlgBilling.m_List->GetColumn(5)->ComboSource;
				}

				if (bHL7IDLinkT_NeedAudit && nOrdProviderID > 0) {
					long nAuditID = BeginNewAuditEvent();
					CString strOld, strNew, strName;
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", nOrdProviderID);
					if (!rsProv->eof) {
						strName = AdoFldString(rsProv, "Name", "");
					}
					rsProv->Close();
					strOld.Format("Provider Code '%s' (HL7 Group '%s')", strOrdProviderID, Message.strHL7GroupName);
					strNew.Format("Linked to Provider '%s'", strName);
					AuditEvent(nOrdProviderID, strName, nAuditID, aeiHL7ProviderLink, nOrdProviderID, strOld, strNew, aepLow, aetCreated);
				}
			}

			CString strQuantity;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 10, 1, 1, strQuantity);
			// (e.frazier 2016-05-17 10:55) - PLID 67677 - Set nQuantity as a double so that it can accommodate decimal values
			double nQuantity = -1;
			if(strQuantity != "" && strQuantity.SpanIncluding(".1234567890").GetLength() == strQuantity.GetLength()) {
				nQuantity = atof(strQuantity);
			}

			// (z.manning 2008-09-30 11:57) - PLID 31126 - Get the amount
			COleCurrency cyAmount;
			cyAmount.SetStatus(COleCurrency::invalid);
			CString strAmount;
			// (z.manning 2008-09-30 12:02) - PLID 31126 - FT1 field 12 is the per-unit transaction amount.
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 12, 1, 1, strAmount);
			if(strAmount.IsEmpty() || !cyAmount.ParseCurrency(strAmount)) {
				// (z.manning 2008-09-30 12:02) - PLID 31126 - Field 12 isn't valid, however, field 11
				// is the total transaction amount, so let's see if we have that instead.
				GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 11, 1, 1, strAmount);
				if(strAmount.IsEmpty() || !cyAmount.ParseCurrency(strAmount)) {
					// (z.manning 2008-09-30 12:05) - PLID 31126 - Neither amount field is a valid currency
					// so set it to be invalid.
					cyAmount.SetStatus(COleCurrency::invalid);
				}
				else {
					ASSERT(cyAmount.GetStatus() == COleCurrency::valid);
					// (z.manning 2008-09-30 12:06) - PLID 31126 - We have a valid total amount,
					// so let's divide by the quantity;
					if(nQuantity != -1) {
						// (e.frazier 2016-05-17 10:55) - PLID 67677 - COleCurrency can only be scaled by an integer value
						cyAmount = cyAmount / nQuantity;
						RoundCurrency(cyAmount);
					}
					else {
						// (z.manning 2008-09-30 12:08) - PLID 31126 - We don't have a valid quantity
						// but the bill we eventually default to 1, so we'll just leave the amount as is.
					}
				}
			}

			//TES 1/13/2005 - Get the modifiers.
			int nRepetition = 1;
			CString strMod, strMod1, strMod2, strMod3, strMod4;
			int nReturn = GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 26, nRepetition, 1, strMod);
			while(nReturn == COMPONENT_FOUND && nRepetition < 5) {

				//TES 9/2/2011 - PLID 44024 - Deformat
				strMod = DeformatHL7Text(strMod);
				// (j.dinatale 2010-08-30) - PLID 38121 - Need to trim strMod before we use it
				strMod.TrimRight();
				strMod.TrimLeft();

				//TES 6/25/2011 - PLID 24393 - If the modifier is empty, don't use it!
				if(!strMod.IsEmpty()) {

					//Is this in the data?
					if(!ReturnsRecords("SELECT Number FROM CPTModifierT WHERE Number = '%s'", _Q(strMod))) {
						//Nope, let's add it.
						ExecuteSql("INSERT INTO CPTModifierT (Number, Note, Multiplier) VALUES ('%s', '<From HL7 Import>', 1)", 
							_Q(strMod));
						//Copied from the OnShowWindow code in m_dlgBilling, this is admittedly sketchy, like all the other forced
						//refreshes in this function.
						// (z.manning, 05/01/2007) - PLID 16623 - Don't load inactive modifiers.
						CString strModifierList = "SELECT Number AS ID, (Number + '   ' + Note) AS Text, Active FROM CPTModifierT "
								"UNION SELECT '-1' AS ID, '     (None)' AS Text, 1 ORDER BY Text ASC";

						pDlg->m_dlgBilling.m_List->GetColumn(11)->ComboSource = _bstr_t(strModifierList);
						pDlg->m_dlgBilling.m_List->GetColumn(12)->ComboSource = _bstr_t(strModifierList);
						pDlg->m_dlgBilling.m_List->GetColumn(13)->ComboSource = _bstr_t(strModifierList);
						pDlg->m_dlgBilling.m_List->GetColumn(14)->ComboSource = _bstr_t(strModifierList);
						
					}
					switch(nRepetition) {
					case 1:
						strMod1 = strMod;
						break;
					case 2:
						strMod2 = strMod;
						break;
					case 3:
						strMod3 = strMod;
						break;
					case 4:
						strMod4 = strMod;
						break;
					}
				}
				nRepetition++;
				nReturn = GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 26, nRepetition, 1, strMod);
			}

			CString strServiceDate;
			COleDateTime dtService;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "FT1", nCharge, 4, 1, 1, strServiceDate);

			// (a.walling 2014-02-24 11:29) - PLID 61003 - CPtrArray g_aryBillingTabInfoT in CBillingDlg et al should instead be a typed collection: vector<BillingItemPtr> m_billingItems. Now using smart pointers.
			BillingItemPtr pBillItem;

			// (b.eyers 2015-06-17) - PLID 66206 - if intellechart is enabled and inventory item, create line item
			// (b.eyers 2015-06-23) - PLID 66208 - set ordering provider from hl7  
			if (bEnableIntelleChart && bInventoryItemFound) { //if intellechart is enabled and inventory code found, add new inventory item, else add charge as normal

				if (ParseHL7Date(strServiceDate, dtService)){
					pBillItem = pDlg->m_dlgBilling.AddNewProductToBillByServiceID(nServiceID, nQuantity, FALSE, FALSE, false, &cyAmount, -1, -1, FALSE, pMap, strMod1, strMod2, strMod3, strMod4, &dtService, nProviderID, TRUE, nOrdProviderID);
				}
				else {
					pBillItem = pDlg->m_dlgBilling.AddNewProductToBillByServiceID(nServiceID, nQuantity, FALSE, FALSE, false, &cyAmount, -1, -1, FALSE, pMap, strMod1, strMod2, strMod3, strMod4, NULL, nProviderID, TRUE, nOrdProviderID);
				}
			}
			else {
				if (ParseHL7Date(strServiceDate, dtService))
				{
					// (v.maida 2015-08-27 16:31) - PLID 66962 - If there's a charge date, then add it to the charge date vector for later use.
					vecChargeDates.push_back(dtService);
					//TES 10/21/2008 - PLID 21432 - Pass in the provider ID
					// (r.farnworth 2014-12-11 11:07) - PLID 64163 - Added cyAmount paramater so that we could just use that amount instead of trying to calculate it again
					// (v.maida 2014-12-26 09:24) - PLID 64470 - Added Boolean parameter that indicates that the charge is coming from an HL7 bill.
					pBillItem = pDlg->m_dlgBilling.AddNewChargeToBill(nServiceID, cyAmount, nQuantity, pMap, strMod1, strMod2, strMod3, strMod4, &dtService, nProviderID, TRUE, nOrdProviderID);
				}
				else {
					//TES 10/21/2008 - PLID 21432 - Pass in the provider ID
					// (r.farnworth 2014-12-11 11:07) - PLID 64163 - Added cyAmount paramater so that we could just use that amount instead of trying to calculate it again
					// (v.maida 2014-12-26 09:24) - PLID 64470 - Added Boolean parameter that indicates that the charge is coming from an HL7 bill.
					pBillItem = pDlg->m_dlgBilling.AddNewChargeToBill(nServiceID, cyAmount, nQuantity, pMap, strMod1, strMod2, strMod3, strMod4, NULL, nProviderID, TRUE, nOrdProviderID);
				}
			}
			// (s.tullis 2015-06-08 13:38) - PLID 66190 - Support importing charge notes from HL7 Bill DFT messages.
			// Get the NTE segments between this FT1 segment and the next
			CString strChargeNote = "";
			if (COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "NTE", 1, 3, 1, 1, strChargeNote, nChargeStart,nChargeEnd))
			{
				pDlg->m_dlgBilling.AddNewUnsavedChargeNote(pBillItem, strChargeNote, FALSE);
			}

			// (b.eyers 2015-06-17) - PLID 66206 - reset 
			bInventoryItemFound = FALSE;
			nCharge++;

		}

		// (b.savon 2015-12-22 09:55) - PLID 67783 - Support creating a HL7 bill with exclusively linking FT1 diag codes
		if (GetHL7SettingBit(Message.nHL7GroupID, "ExclusivelyLinkFTandDG") != FALSE) {

			vector<shared_ptr<pair<shared_ptr<HL7DiagCode>, shared_ptr<HL7DiagCode>>>> aryCodePairs;
			vector<shared_ptr<HL7DiagCode>> aryCodes;

			// (j.gruber 2016-01-29 14:54) - PLID 68127 - changed a bit because I moved to HL7ParseUtils
			const HL7DiagCodeDefaults defaults(Message,
				GetHL7SettingText(Message.nHL7GroupID, "DefaultCodeSystem"),
				GetHL7SettingText(Message.nHL7GroupID, "DefaultAltCodeSystem"),
				GetHL7SettingText(Message.nHL7GroupID, "CodeSystemFlag9"),
				GetHL7SettingText(Message.nHL7GroupID, "CodeSystemFlag10"));

			GetDG1DiagCodes(aryCodePairs, aryCodes, Message, nChargeStart, nChargeEnd, defaults);

			for each(shared_ptr<HL7DiagCode> pCode in aryCodes) {
				EnsureDiagCode(pCode.get(), Message, defaults);
			}

			TraverseDiagCodes(aryCodePairs, defaults, pDlg);
		}

		// (s.tullis 2015-06-08 13:38) - PLID 66190 - Make Sure the Bill Screen is up to date when done
		pDlg->m_dlgBilling.UpdateView();

		//Use the bill date in the message, if we're set up to do that.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		// (v.maida 2015-08-27 10:42) - PLID 66962 - Moved to after all FT1 charges have been looped through, so that the list of all charge dates is populated.
		if (GetHL7SettingBit(Message.nHL7GroupID, "UseBillDate")) {
			COleDateTime dtBill;
			CString strBillDate;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "EVN", 1, 2, 1, 1, strBillDate);
			if (ParseHL7Date(strBillDate, dtBill)) {
				_variant_t varBillDate(dtBill);
				pDlg->m_date.SetValue(varBillDate);
			}
			else if ((int) vecChargeDates.size() > 0){
				// (v.maida 2015-08-27 10:42) - PLID 66962 - No date in the EVN segment, so try to use the earliest charge date.
				_variant_t varBillDate(*std::min_element(vecChargeDates.begin(), vecChargeDates.end()));
				pDlg->m_date.SetValue(varBillDate);
			}
		}

		//Somehow finalize and display bill.
		pDlg->EnableWindow(TRUE);

		CString strSql;
		//TES 5/8/2008 - PLID 29685 - Now call FinalizeHL7Event(), to update HL7MessageQueueT.
		//TES 6/10/2011 - PLID 41353 - This function doesn't create a new record that we want output.
		if(!FinalizeHL7Event(nPersonID, Message, strSql, bSendHL7Tablechecker)) {
			return FALSE;
		}

		return TRUE;
	}NxCatchAll("Error in HL7Utils::CreateHL7Bill()");

	if(pDlg && pDlg->GetSafeHwnd()) {
		pDlg->CloseWindow();
	}
	return FALSE;
}

COleDateTime GetEarliestChargeDateFromHL7Bill(const CString &strMessage, const long &nHL7GroupID)
{
	CString strCode;
	int nCharge = 1;
	vector<COleDateTime> vecChargeDates;
	// default to global invalid datetime
	COleDateTime dtEarliestCharge = g_cdtInvalid;

	// build list of charge dates, and store them in a vector
	while (COMPONENT_FOUND == GetHL7MessageComponent(strMessage, nHL7GroupID, "FT1", nCharge, 25, 1, 1, strCode))
	{
		CString strServiceDate;
		COleDateTime dtService;
		GetHL7MessageComponent(strMessage, nHL7GroupID, "FT1", nCharge, 4, 1, 1, strServiceDate);
		if (ParseHL7Date(strServiceDate, dtService))
		{
			vecChargeDates.push_back(dtService);
		}
		nCharge++;
	}

	// get earliest charge
	if ((int)vecChargeDates.size() > 0) {
		dtEarliestCharge = *std::min_element(vecChargeDates.begin(), vecChargeDates.end());
	}

	return dtEarliestCharge;
}

//TES 8/9/2007 - PLID 26892 - changed to take an input enum and an output enum, defining the behavior in cases where
// the HL7 message references an ID that doesn't exist in Nextech.
//TES 9/18/2008 - PLID 31414 - Renamed to make it clear that this is only for extracting patient information, not other
// types of PersonT records.
//TES 10/5/2009 - PLID 35695 - Added an optional output parameter for the patient name (to the first overload only, 
// the second one already has it being passed in via the HL7_PIDFields struct).
// (z.manning 2010-05-21 14:59) - PLID 38831 - Added parameter for lab import matching field flags
long GetPatientFromHL7Message(const CString &strMessage, long nHL7GroupID, CWnd *pMessageParent, ENotFoundBehavior nfb /*= nfbPromptToLink*/, OPTIONAL OUT ENotFoundResult *pNfr /*= NULL*/, bool bWillOverwrite /*= false*/, OPTIONAL OUT CString *pstrPatientName /*= NULL*/, OPTIONAL IN DWORD dwLabImportMatchingField /* = 0 */)
{
	//TES 7/12/2007 - PLID 26642 - Parse out the message, then call the appropriate override.
	HL7_PIDFields PID;
	// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() so the libs will have a connection pointer.
	if(!ParsePIDSegment(strMessage, nHL7GroupID, PID, GetRemoteData())) {
		// (r.gonet 05/01/2014) - PLID 61843 - Log an error
		LogHL7Error("Nextech could not get the patient information from the HL7 message's PID segment.");
		LogHL7Resolution("Please check with the sender of the HL7 message and confirm that they are sending PID-2 (Patient ID).");
		//TES 4/17/2008 - PLID 29595 - ParsePIDSegment no longer notifies the user on failure, so we'll do it ourselves.
		AfxMessageBox("Could not parse Patient information in HL7 message!");
		return -1;
	}
	//TES 10/6/2009 - PLID 35695 - If the patient name was requested, set it.
	if(pstrPatientName) {
		*pstrPatientName = PID.strLast + ", " + PID.strFirst + " " + PID.strMiddle;
	}
	return GetPatientFromHL7Message(PID, nHL7GroupID, pMessageParent, nfb, pNfr, false, dwLabImportMatchingField);
}

//TES 8/9/2007 - PLID 26892 - changed to take an input enum and an output enum, defining the behavior in cases where
// the HL7 message references an ID that doesn't exist in Nextech.
//TES 9/18/2008 - PLID 31414 - Renamed to make it clear that this is only for extracting patient information, not other
// types of PersonT records.
// (z.manning 2010-05-21 14:59) - PLID 38831 - Added parameter for lab import matching field flags
long GetPatientFromHL7Message(const HL7_PIDFields &PID, long nHL7GroupID, CWnd *pMessageParent, ENotFoundBehavior nfb, OPTIONAL OUT ENotFoundResult *pNfr, bool bWillOverwrite /*= false*/, OPTIONAL IN DWORD dwLabImportMatchingFieldFlags /* = 0 */)
{
	long nPersonID = -2;
	//TES 4/17/2008 - PLID 29595 - Much of this functionality was moved to the HL7ParseUtils funtion FindMatchingPatient(),
	// which is silent, so we may need to take additional action depending on the return value.
	HL7MatchResult hmr = FindMatchingPatient(GetRemoteData(), PID, dwLabImportMatchingFieldFlags, nHL7GroupID, nPersonID);
	
	switch(hmr) {
	case hmrNoMatchFound:
		//TES 4/17/2008 - PLID 29595 - The patient didn't exist, see what they want us to do.
		if(nfb == nfbSkip) {
			//TES 7/12/2007 - PLID 26642 - They've asked us to just ignore this record if we couldn't link it, so report
			// that that's what we did.
			if(pNfr) *pNfr = nfrSkipped;
			return -1;
		}
		else {
			//We give up, let's give them a chance to select it.
			CHL7SelectPatientDlg dlg(NULL);
			dlg.m_bAllowCreateNew = (nfb == nfbPromptToLinkAndCreate) ? true : false;
			dlg.m_bWillOverwrite = bWillOverwrite;
			dlg.m_strFirstName = PID.strFirst;
			dlg.m_strMiddleName = PID.strMiddle;
			dlg.m_strLastName = PID.strLast;
			dlg.m_dtBirthDate = PID.dtBirthDate;
			dlg.m_strSSN = PID.strSSN;
			// (s.dhole 2013-08-29 10:20) - PLID 54572 Load phone,gennder and zip code
			dlg.m_nGender = PID.nGender;
			//These phone numeber are mya not be accurate or not same as entered in practice. 
			// will show comm saprated
			CString  strPhone ; 
			if (!PID.strHomePhone.IsEmpty() ) strPhone = FormatPhone( PID.strHomePhone ,"(###) ###-####") + "; " ;
			if (!PID.strWorkPhone.IsEmpty() ) strPhone += FormatPhone( PID.strWorkPhone ,"(###) ###-####") + "; " ;
			if (!PID.strCellPhone.IsEmpty() ) strPhone += FormatPhone( PID.strCellPhone ,"(###) ###-####") + "; " ;
			if (!PID.strOtherPhone.IsEmpty() ) strPhone += FormatPhone( PID.strOtherPhone ,"(###) ###-####") + "; " ;
			if (!strPhone.IsEmpty()){
				strPhone= strPhone.Left( strPhone.GetLength()-2);  
			}
			dlg.m_strPhone = strPhone;
			dlg.m_strZip = PID.strZip;
			dlg.m_strGroupName = GetHL7GroupName(nHL7GroupID);
			//TES 9/23/2008 - PLID 21093 - If the HL7 message doesn't have an ID, then this link won't persist, so tell
			// the dialog so it can inform the user.
			dlg.m_bWillLink = !PID.strHL7ID.IsEmpty();

			//TES 2/24/2012 - PLID 48395 - If there is exactly one patient in the database with the exact same name as we were given,
			// preselect it on the dialog.
			_RecordsetPtr rsPatientMatch = CreateParamRecordset("SELECT PersonT.ID "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE PersonT.First = {STRING} AND PersonT.Middle = {STRING} AND PersonT.Last = {STRING}",
				PID.strFirst, PID.strMiddle, PID.strLast);
			long nMatchingID = -1;
			if(!rsPatientMatch->eof) {
				nMatchingID = AdoFldLong(rsPatientMatch, "ID");
				rsPatientMatch->MoveNext();
				if(!rsPatientMatch->eof) {
					nMatchingID = -1;
				}
			}
			dlg.m_nPreselectedPersonID = nMatchingID;
			
			if(IDOK == dlg.DoModal()) {
				nPersonID = dlg.m_nPersonID;
				if(nPersonID == -2) {
					//TES 7/12/2007 - PLID 26642 - This is only a valid return if we set m_bAllowCreateNew to true.
					//TES 7/12/2007 - PLID 26892 - This is now an enum
					ASSERT(nfb == nfbPromptToLinkAndCreate);
					//TES 7/12/2007 - PLID 26642 - Return the sentinel value that tells our caller to create a new patient.
					//TES 7/12/2007 - PLID 26892 - We now do this by setting the output enum, and returning -1
					if(pNfr) *pNfr = nfrCreateNew;
					return -1;
				}
				else {
					nPersonID = dlg.m_nPersonID;
					//TES 9/23/2008 - PLID 21093 - Don't link a patient that didn't have an HL7 ID.
					if(!PID.strHL7ID.IsEmpty()) {
						// (j.jones 2008-04-17 11:30) - PLID 27244 - TryCreateHL7IDLinkRecord will create
						// a HL7IDLinkT record only if there isn't an exact match already
						//TES 9/18/2008 - PLID 31414 - This now needs to know what type of record we're linking.
						// (j.jones 2010-05-13 09:16) - PLID 36527 - now returns TRUE if a new HL7IDLinkT record was created,
						// so we can audit ourselves (this function does not audit)
						//TES 1/3/2011 - PLID 40744 - Pass in first and last name
						// (d.thompson 2013-01-21) - PLID 54732 - Start tracking assigning authority
						if(TryCreateHL7IDLinkRecord(GetRemoteData(), nHL7GroupID, PID.strHL7ID, nPersonID, hilrtPatient, PID.strFirst, PID.strLast, PID.strAssigningAuthority)) {

							// (j.jones 2010-05-13 09:16) - PLID 36527 - audit the HL7IDLinkT creation
							CString strOld, strNew, strPatientName;
							strPatientName = GetExistingPatientName(nPersonID);
							strOld.Format("Patient Code '%s' (HL7 Group '%s')", PID.strHL7ID, GetHL7GroupName(nHL7GroupID));
							strNew.Format("Linked to Patient '%s'", strPatientName);

							long nAuditID = BeginNewAuditEvent();
							AuditEvent(nPersonID, strPatientName, nAuditID, aeiHL7PatientLink, nPersonID, strOld, strNew, aepLow, aetCreated);
						} else {
							// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
							LogHL7Warning("Could not create an HL7 ID Link record for this patient. The link most likely already existed.");
						}
					} else {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
						LogHL7Warning("Could not link the HL7 patient to the Nextech patient because the HL7 third party assigned patient ID is blank.");
					}
				}
			} 
			else {
				//TES 7/12/2007 - PLID 26892 - Set our output enum, if possible.
				if(pNfr) *pNfr = nfrFailure;
				return -1;
			}
		}
	break;
	case hmrMultipleMatchesFound:
		ASSERT(FALSE); //TES 4/17/2008 - PLID 29595 - If there are multiple matches (which there shouldn't be), it should 
						// just be picking the first one, and returning hmrSuccess.  So we'll ASSERT, but then proceed 
						// with the hmrSuccess behavior.
	case hmrSuccess:
		//The function set PersonID properly, so we don't actually need to do anything.
		break;
		
	default:
		AfxThrowNxException("Invalid result %i returned in GetPatientFromHL7Message()", hmr);

	}
	//TES 7/12/2007 - PLID 26892 - If we got here, nPersonID should be a valid ID.
	ASSERT(nPersonID != -1);
	return nPersonID;
}

//(e.lally 2007-08-01) PLID 26326 - Created a utility to tell if we can "commit all" or not
	//P03 events - creating bills is not supported in a commit all
BOOL IsCommitAllEventSupported(const CString& strEventType)
{
	// (j.jones 2008-08-21 13:46) - PLID 29596 - if any other events are added
	// that cannot be committed in a batch, please update the warning appropriately
	// in CHL7BatchDlg::OnBtnHl7Import

	if(strEventType.CompareNoCase("P03") == 0)
		return FALSE;
	else
		return TRUE;
}

//TES 9/18/2008 - PLID 31414 - Moved TryCreateHL7IDLinnkRecord() from HL7Utils to HL7ParseUtils.

// (j.jones 2008-04-18 11:38) - PLID 21675 - given a message, will return the message date
// (z.manning 2011-06-15 15:51) - PLID 40903 - This no longer takes in a message, but rather a string representing MSH-7
COleDateTime GetHL7DateFromStringField(const CString &strDateField, long nHL7GroupID)
{
	if(strDateField.IsEmpty()) {
		return COleDateTime::GetCurrentTime();
	}

	CString strMessageDate = strDateField;

	int nSep = strMessageDate.Find(GetSeparator(stRepetition));
	if(nSep != -1) {
		strMessageDate = strMessageDate.Left(nSep);
	}
	nSep = strMessageDate.Find(GetSeparator(stComponent));
	if(nSep != -1) {
		strMessageDate = strMessageDate.Left(nSep);
	}
	nSep = strMessageDate.Find(GetSeparator(stSubComponent));
	if(nSep != -1) {
		strMessageDate = strMessageDate.Left(nSep);
	}

	COleDateTime dtMessage;
	dtMessage.m_status = COleDateTime::invalid;
	// (a.walling 2010-02-25 14:17) - PLID 37543 - Include time
	if(!ParseHL7DateTime(strMessageDate, dtMessage)) {
		//Just use the current date.
		dtMessage = COleDateTime::GetCurrentTime();
	}

	//ParseHL7Date shouldn't have returned TRUE if it sent back an invalid date,
	//but check for an invalid date anyways
	if(dtMessage.m_status == COleDateTime::invalid) {
		ASSERT(FALSE);
		dtMessage = COleDateTime::GetCurrentTime();
	}

	return dtMessage;
}

// (j.jones 2008-04-21 14:53) - PLID 29597 - determine if we need to
// notify the user that there are pending messages, and do so
void TryNotifyUserPendingHL7Messages()
{
	try {

		CMainFrame *pMainFrm = GetMainFrame();

		if(!pMainFrm) {
			//if this fails, we've got problems, and should give up now
			ASSERT(FALSE);
			return;
		}

		//first see if the HL7 tab is our active window, if so,
		//we don't need to notify the user, so don't bother
		//doing any work
		
		// (d.thompson 2009-11-16) - PLID 36301 - Moved HL7 tab to 'Links' instead
		//	of 'Financial'
		// (a.vengrofski 2010-08-18 12:47) - PLID <38919> - need to handle the two tabs for HL7 messages.
		BOOL bShowHL7 = TRUE, bShowLab = TRUE;
		{
			CNxTabView *pLinksView = (CNxTabView*)pMainFrm->GetOpenView(LINKS_MODULE_NAME);			
			if(pLinksView) {

				CNxTabView *pActiveView = pMainFrm->GetActiveView();

				// (a.vengrofski 2010-08-18 12:50) - PLID <38919> - need to handle the two tabs for HL7 messages.
				if(pLinksView == pActiveView) {
					bShowLab = !(pLinksView->GetActiveTab() == LinksModule::ReceiveLabsTab);
					bShowHL7 = !(pLinksView->GetActiveTab() == LinksModule::HL7Tab);
					//	//the active tab is the HL7 tab, which would already
					//	//be interpreting tablecheckers and updating the screen
					//	//in real-time
					//	return;
				}
			}
		}

		//similarly, see if they have any preferences checked to
		//be notified, and if none are checked, leave
		//(Note: we only have patients right now, later we'll have labs)
		BOOL bPatientNotify = GetRemotePropertyInt("HL7ImportNotifyMe_Patients", 1, 0, GetCurrentUserName(), true) == 1;
		//TES 11/26/2008 - PLID 32197 - We have labs now (actually, we have for a while, we just forgot to update this code).
		BOOL bLabNotify = GetRemotePropertyInt("HL7ImportNotifyMe_Labs", GetRemotePropertyInt("HL7ImportNotifyMe_Patients", 1, 0, GetCurrentUserName(), true), 0, GetCurrentUserName(), true) == 1;
		//TES 11/26/2008 - PLID 32197 - And for that matter, we always had billing messages, so they should have a preference.
		BOOL bBillNotify = GetRemotePropertyInt("HL7ImportNotifyMe_Bills", GetRemotePropertyInt("HL7ImportNotifyMe_Patients", 1, 0, GetCurrentUserName(), true), 0, GetCurrentUserName(), true) == 1;
		// (z.manning 2010-06-29 17:31) - PLID 34572 - Appt notify
		// (b.spivey, February 25, 2013) - PLID 55227 - Wrong preference. 
		BOOL bApptNotify = GetRemotePropertyInt("HL7ImportNotifyMe_Appointments", GetRemotePropertyInt("HL7ImportNotifyMe_Patients", 1, 0, GetCurrentUserName(), true), 0, GetCurrentUserName(), true) == 1;
		// (b.spivey, February 18, 2013) - PLID 55226 - Check pref. 
		BOOL bDocumentsNotify = GetRemotePropertyInt("HL7ImportNotifyMe_PatientDocuments", GetRemotePropertyInt("HL7ImportNotifyMe_Patients", 1, 0, GetCurrentUserName(), true), 0, GetCurrentUserName(), true) == 1;
		//TES 11/5/2015 - PLID 66371 - Added Optical Prescriptions
		BOOL bOpticalPrescriptionsNotify = GetRemotePropertyInt("HL7ImportNotifyMe_OpticalPrescriptions", GetRemotePropertyInt("HL7ImportNotifyMe_Patients", 1, 0, GetCurrentUserName(), true), 0, GetCurrentUserName(), true);

		if(!bPatientNotify && !bLabNotify && !bBillNotify && !bApptNotify && !bDocumentsNotify && !bOpticalPrescriptionsNotify) {
			return;
		}

		//when we have more than just patient messages, this recordset will need
		//to filter only on the messages that the user's preferences want to see
		//(i.e. patients and not labs, or labs and not patients)
		//but for now, count up everything that is not committed
		//TES 11/26/2008 - PLID 32197 - The time has come!  Add a MessageTypeFilter into the WHERE clause.
		// (a.vengrofski 2010-08-18 12:52) - PLID <38919> - Split the query into two; first for the general HL7, and the second for the Lab results.
		//TES 11/5/2015 - PLID 66371 - Added Optical Prescriptions
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountHL7 FROM HL7MessageQueueT "
			"WHERE ActionID Is Null AND ("
			"({INT}=1 AND MessageType = {STRING}) OR "
			"({INT}=1 AND MessageType = {STRING}) OR "
			"({INT}=1 AND MessageType = {STRING}) OR "
			"({INT}=1 AND MessageType = {STRING}) OR "
			// (b.spivey, February 18, 2013) - PLID 55226 - Make sure we're not including from a non-document group. 
			"({INT}=1 AND MessageType = {STRING} AND "
			"	GroupID IN (SELECT HL7GroupID "
			"		FROM HL7GenericSettingsT "
			"		WHERE Name = 'UsePatientHistoryImportMSH_3' AND BitParam = 1 ) "
			") "
			");\r\n"
			"SELECT Count(ID) AS CountLab FROM HL7MessageQueueT "
			"WHERE ActionID Is Null AND ("
			"({INT}=1 AND MessageType = {STRING}) AND "
			// (b.spivey, February 18, 2013) - PLID 55226 - make sure we're not including from a document group. 
			"GroupID NOT IN "
			"(	SELECT HL7GroupID "
			"	FROM HL7GenericSettingsT "
			"	WHERE Name = 'UsePatientHistoryImportMSH_3' AND BitParam = 1 ) "
			");", 
			bPatientNotify?1:0, "ADT",
			bApptNotify?1:0, "SIU", // (z.manning 2010-06-29 17:32) - PLID 34572
			bBillNotify?1:0, "DFT",
			bOpticalPrescriptionsNotify?1:0, "ZOP", //TES 11/5/2015 - PLID 66371
			bDocumentsNotify ? 1 : 0, "ORU", // (b.spivey, February 18, 2013) - PLID 55226 
			bLabNotify?1:0, "ORU");
		long nCountHL7 = 0;
		if(!rs->eof) {
			nCountHL7 = AdoFldLong(rs, "CountHL7",0);		
		}
		//deal with the second record set
		rs = rs->NextRecordset(NULL);
		long nCountLab = 0;
		if (!rs->eof) {
			nCountLab = AdoFldLong(rs, "CountLab",0);
		}

		if(nCountHL7 == 0) {
			//remove any notifications we may have
			pMainFrm->UnNotifyUser(NT_HL7);
		}

		// (a.vengrofski 2010-08-18 12:57) - PLID <38919>
		if (nCountLab == 0) {
			//remove the lab notifications
			pMainFrm->UnNotifyUser(NT_HL7_LAB);
		}

		// (a.vengrofski 2010-08-18 13:02) - PLID <38919> - If there are no pending messages, we have nothing to do.
		if (nCountLab == 0 && nCountHL7 == 0){
			return;
		}

		//notify the user that there are pending messages
		CString strMsg = "";
		int nNotificationType = -1;
		if (nCountLab > 0 && bShowLab) {
			nNotificationType = NT_HL7_LAB;
			strMsg.Format("You have %li pending Lab Result(s).", nCountLab);
		}
		if (nCountHL7 > 0 && bShowHL7) {
			nNotificationType = NT_HL7;
			if (nCountLab > 0 && bShowLab) {
				strMsg += "\n";
				nNotificationType = NT_HL7_LAB;
			}
			strMsg.Format(strMsg + "You have %li pending HL7 Message(s).", nCountHL7);
		}

		if (nNotificationType != -1){
			pMainFrm->NotifyUser(nNotificationType, strMsg, false);
		}
	// (r.gonet 05/01/2014) - PLID 61843 - Log any exceptions to the HL7 Log
	} NxCatchAllHL7("Error in TryNotifyUserPendingHL7Messages");
}

// (r.gonet 12/03/2012) - PLID 54106 - Exports a new appointment to all HL7 links that support automatic exporting of appointments.
//TES 6/7/2013 - PLID 55732 - Added strRequestGroupGUID.  If it is non-empty, and if an error occurs for this message, only one error will be reported to the
// user per RequestGroupGUID.
void SendNewAppointmentHL7Message(long nApptID, bool bSendTableChecker/*= true*/, const CString &strRequestGroupGUID /*= ""*/)
{
	// (r.gonet 12/11/2012) - PLID 54106 - Iterate through all HL7 groups that are set to export appointments automatically.
	CArray<long, long> &arynScheduleHL7GroupIDs = GetMainFrame()->m_arynScheduleHL7GroupIDs;
	for(int i = 0; i < arynScheduleHL7GroupIDs.GetSize(); i++) {
		long nHL7GroupID = arynScheduleHL7GroupIDs[i];
		// (r.gonet 12/11/2012) - PLID 54106 - Send the appointment to this link. Have CMainFrame handle any failures.
		GetMainFrame()->GetHL7Client()->SendNewAppointmentHL7Message(nApptID, nHL7GroupID, bSendTableChecker, strRequestGroupGUID);
	}
}

// (r.gonet 12/03/2012) - PLID 54107 - Exports an updated appointment to all HL7 links that support automatic exporting of appointments.
//TES 6/7/2013 - PLID 55732 - Added strRequestGroupGUID.  If it is non-empty, and if an error occurs for this message, only one error will be reported to the
// user per RequestGroupGUID.
void SendUpdateAppointmentHL7Message(long nApptID, bool bSendTableChecker/*= true*/, const CString &strRequestGroupGUID /* = ""*/)
{
	// (r.gonet 12/11/2012) - PLID 54107 - Iterate through all HL7 groups that are set to export appointments automatically.
	CArray<long, long> &arynScheduleHL7GroupIDs = GetMainFrame()->m_arynScheduleHL7GroupIDs;
	for(int i = 0; i < arynScheduleHL7GroupIDs.GetSize(); i++) {
		long nHL7GroupID = arynScheduleHL7GroupIDs[i];
		// (r.gonet 12/11/2012) - PLID 54107 - Send the appointment to this link. Have CMainFrame handle any failures.
		GetMainFrame()->GetHL7Client()->SendUpdateAppointmentHL7Message(nApptID, nHL7GroupID, strRequestGroupGUID, bSendTableChecker);
	}
}

// (r.gonet 12/03/2012) - PLID 54108 - Exports a cancelled appointment to all HL7 links that support automatic exporting of appointments.
//TES 6/7/2013 - PLID 55732 - Added strRequestGroupGUID.  If it is non-empty, and if an error occurs for this message, only one error will be reported to the
// user per RequestGroupGUID.
void SendCancelAppointmentHL7Message(long nApptID, bool bSendTableChecker/*= true*/, const CString &strRequestGroupGUID /*  = ""*/)
{
	SendCancelAppointmentHL7Message(GetMainFrame()->GetHL7Client(), nApptID, bSendTableChecker, strRequestGroupGUID);
}

/// <summary>
/// Exports a cancelled appointment to all HL7 links that support automatic exporting of appointments
/// </summary>
/// <param name="pClient">The HL7 client to use</param>
/// <param name="nApptID">The ID of the appointment being cancelled or deleted</param>
/// <param name="bSendTableChecker">True if we should send a table checker; otherwise false</param>
/// <param name="strRequestGroupGUID">The HL7 request group GUID, or an empty string if not applicable</param>
/// <returns>A collection of responses from each HL7 link which must be checked for errors</returns>
std::vector<HL7ResponsePtr> SendCancelAppointmentHL7Message(CHL7Client* pClient, long nApptID, bool bSendTableChecker, const CString &strRequestGroupGUID)
{
	std::vector<HL7ResponsePtr> vecResponses;
	for (long nHL7GroupID : GetMainFrame()->m_arynScheduleHL7GroupIDs)
	{
		HL7ResponsePtr pResponse = pClient->SendCancelAppointmentHL7Message(nApptID, nHL7GroupID, strRequestGroupGUID, bSendTableChecker);
		vecResponses.insert(vecResponses.end(), pResponse);
	}
	return vecResponses;
}

//TES 10/22/2010 - PLID 40951 - This function consolidates all the logic for whether a given OBX segment should be combined
// with an existing result.  Should be in HL7ParseUtils, but that doesn't have access to HL7 Settings yet, so for now this
// needs to be kept in sync with the identically-named HL7Support function in NxServer
BOOL CanCombineResult(const HL7_OBXSegment &OBX, const HL7LabResultInfo &Result, const CString &strObr24Value, long nHL7GroupID)
{
	//TES 10/22/2010 - PLID 40951 - First off, if either result is a file, nothing can be combined with it.
	if(!Result.strFileData.IsEmpty() || !OBX.strFileData.IsEmpty()) {
		return false;
	}
	//TES 5/21/2010 - PLID 38785 - If this is a text-type result (ST, TX, or FT), and the setting is on to always combine, 
	// then this is not a new result, regardless of the sub-group ID.
	BOOL bIsTextType = (OBX.strType == "ST" || OBX.strType == "TX" || OBX.strType == "FT");
	//TES 6/14/2010 - PLID 39135 - Changed from a bool to an enum
	//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
	CombineTextSegmentsOptions ctso = (CombineTextSegmentsOptions)GetHL7SettingInt(nHL7GroupID, "CombineTextSegments");
	//TES 6/14/2010 - PLID 39135 - If the setting is to never combine, then we always want to create a new result.
	//TES 7/30/2010 - PLID 39908 - Pull the combine setting, then decide whether to combine based on that setting (we never
	// combine non-text segments)
	BOOL bCombine = FALSE;
	if(bIsTextType) {
		switch(ctso) {
			case ctsoCheckSubGroupID:
				//TES 7/30/2010 - PLID 39908 - Combine if the Sub-Group IDs are the same.
				bCombine = (OBX.strSubGroupID == Result.strSubGroupID);
				break;
			case ctsoAlwaysCombine:
				//TES 7/30/2010 - PLID 39908 - Always combine
				bCombine = TRUE;
				break;
			case ctsoNeverCombine:
				//TES 7/30/2010 - PLID 39908 - Never combine
				bCombine = FALSE;
				break;
			case ctsoCheckObr24:
				{
					//TES 7/30/2010 - PLID 39908 - If the OBR-24 value was one of the ones they specified, then combine,
					// otherwise, don't.
					CStringArray saObr24Values;
					GetOBR24Values(nHL7GroupID, saObr24Values);
					for(int nObrValue = 0; nObrValue < saObr24Values.GetSize() && !bCombine; nObrValue++) {
						CString strTmp = saObr24Values[nObrValue];
						if(saObr24Values[nObrValue].CompareNoCase(strObr24Value) == 0) {
							bCombine = TRUE;
						}
					}
				}
				break;
		}
	}

	//TES 9/18/2008 - PLID 21093 - Now, interpret.  If the observation identifier is empty (or, in NovoLab's case, 
	// "Blank"), then we know we're dealing with a big note, so just append the observation value to the current result.
	//TES 5/21/2010 - PLID 38785 - We also know this is a big note if it's a text type, and we're flagged to combine them
	//TES 6/14/2010 - PLID 39135 - If the setting is to never combine, then don't combine.
	//TES 7/30/2010 - PLID 39908 - Combine if we decided to up above, OR if the observation identifier is empty/blank AND 
	// the combine setting is anything but "Never combine".
	if(bCombine || (ctso != ctsoNeverCombine && (OBX.strObservationIdentifier.IsEmpty() || OBX.strObservationIdentifier == "Blank")) ) {
		bCombine = TRUE;
	}
	else {
		bCombine = FALSE;
	}

	if(!bCombine) {
		//TES 10/22/2010 - PLID 40951 - Next, see if the observation we are adding is going into the value field,
		// and if the current result already has a value (which can be in strValue, strNotes, or strFileData).
		// If so, then we need to commit the current one, otherwise, just keep adding into it, because the values
		// are going into different fields.
		//TES 10/26/2010 - PLID 
		//TES 5/19/2011 - PLID 43781 - From what I can tell, this logic has always been wrong.  It said to combine if the observation was
		// going into the value, and the current result's value was NOT empty!
		if(ctso != ctsoNeverCombine && OBX.hlrf == hlrfInvalid && (Result.strValue.IsEmpty() && Result.strNotes.IsEmpty() && Result.strFileData.IsEmpty())) {
			bCombine = TRUE;
		}
	}

	return bCombine;
}

//TES 10/22/2010 - PLID 40951 - Moved the HL7LabResultInfo struct to HL7ParseUtils.h

//TES 10/22/2010 - PLID 40951 - Moved the CommitLabResult() function to HL7ParseUtils

//TES 10/22/2010 - PLID 40951 - Moved AddObservationToResult() to HL7ParseUtils

//TES 9/18/2008 - PLID 21093 - Processes an ORU^R01 (Unsolicited Lab Result) message, creates a new Lab record if needed,
// and appends the results to the lab.
//TES 12/2/2008 - PLID 32297 - NOTE: This function must be kept in sync with the TryImportLabResult() function in 
// NxServer's HL7Support.cpp.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL ImportHL7LabResult(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7TableChecker, IN OUT CDWordArray &arNewCDSInterventions)
{
	long nAuditTransactionID = -1;

	//TES 11/13/2014 - PLID 55674 - Struct to store all the information needed to commit each lab to data.
	struct LabCommitInfo {
		CString strSqlBatch;
		CNxParamSqlArray *pAryParams;
		//TES 10/13/2008 - PLID 21093 - Keep an array of all the Lab-level things we want to audit (we can't actually audit 
		// them until we get the LabsT.ID)
		CArray<CPendingAuditInfo, CPendingAuditInfo&> *pArPendingLabAudits;
		CArray<CArray<CPendingAuditInfo, CPendingAuditInfo&>*, CArray<CPendingAuditInfo, CPendingAuditInfo&>*> *pArPendingResultAudits;
		CString strProviderID;
		CString strProviderPersonID;
		CString strProvRecordset;
		bool bHL7IDLinkT_NeedAudit;
		CStringArray *psaResults;
		int nTodoPriority;

		LabCommitInfo::LabCommitInfo()
		{
			strSqlBatch = strProviderID = strProviderPersonID = strProvRecordset = "";
			bHL7IDLinkT_NeedAudit = false;
			nTodoPriority = -1;
			pAryParams = NULL;
			pArPendingLabAudits = NULL;
			pArPendingResultAudits = NULL;
			psaResults = NULL;
		}
	};
	CArray<LabCommitInfo, LabCommitInfo&> aryLabCommits;
	CSqlTransaction commitTrans;

	try
	{
		long nPersonID = -1;
		CString strMessage = Message.strMessage;
		long nHL7GroupID = Message.nHL7GroupID;
		DWORD dwLabImportMatchingField = 0;

		//TES 11/13/2014 - PLID 55674 - Moved this declaration in here, the audits will be stored in aryLabCommits
		CArray<CArray<CPendingAuditInfo, CPendingAuditInfo&>*, CArray<CPendingAuditInfo, CPendingAuditInfo&>*> arPendingResultAudits;

		// (c.haag 2010-09-14 15:06) - PLID 40176 - We need to know if and how to parse specimens out of
		// order numbers.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		long nLabSpecimenParseMethod = GetHL7SettingInt(nHL7GroupID, "LabSpecimenParseMethod");

		if(GetHL7SettingBit(nHL7GroupID, "AutoImportLabs")) {
			dwLabImportMatchingField = (DWORD)GetHL7SettingInt(nHL7GroupID, "LabImportMatchingFieldFlags");
		}

		// (r.gonet 2011-07-15) - PLID 37997 - We need to see what the default lab document mail category is set to.
		long nLabAttachmentsDefaultCategory = GetRemotePropertyInt("LabAttachmentsDefaultCategory", -1, 0, 0, false);

		//TES 8/2/2011 - PLID 44814 - Make sure the user has permission for this category
		if(!CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, nLabAttachmentsDefaultCategory, TRUE)) {
			if (nLabAttachmentsDefaultCategory != -1) {
				// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
				LogHL7Warning("Current user lacks permission to view the default lab attachments category (ID = %li) or that permission does not exist. Using no category instead.", nLabAttachmentsDefaultCategory);
				nLabAttachmentsDefaultCategory = -1;
			}
		}

		// (r.gonet 2011-03-10) - PLID 42655 - If this is a single result message and we have the preference to copy results turned on,
		//  then see if there are any other labs with the same form number. If so copy this result to them as well.
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		BOOL bCopyResult = GetHL7SettingBit(nHL7GroupID, "CopyResult");
		// (r.gonet 2011-04-08) - PLID 42655 - Copy results setting needs to know if there are multiple OBRs within this message before they are matched with labs 
		//  in order to handle the case where the lab doesn't send the form number.
		if(bCopyResult &&
			(-1 != FindNthOccurrence(Message.strMessage, "\rOBR", 1) || -1 != FindNthOccurrence(Message.strMessage, "\nOBR", 1)) &&
			(-1 != FindNthOccurrence(Message.strMessage, "\rOBR", 2) || -1 != FindNthOccurrence(Message.strMessage, "\nOBR", 2))) 
		{
			// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
			LogHL7Warning("The setting \"Copy singular results to all specimens\" is enabled, but this message has multiple OBR segments. Proceeding as if the setting was disabled. "
				"Note that this setting should only be enabled if all messages can only contain a single OBR segment each.");
			// (r.gonet 2011-04-08) - PLID 42655 - There are at least two OBRs. So Copy Results can't work.
			bCopyResult = FALSE;
		}

		//TES 10/6/2009 - PLID 35695 - Get the patient name as well.
		CString strPatientName;
		nPersonID = GetPatientFromHL7Message(Message.strMessage, Message.nHL7GroupID, pMessageParent, nfbPromptToLink,
			NULL, false, &strPatientName, dwLabImportMatchingField);
		if(nPersonID == -1) {
			//TES 7/12/2007 - PLID 26642 - The message couldn't be parsed, or the user cancelled.

			// (j.jones 2010-04-08 09:03) - PLID 38101 - if we didn't process the result,
			// clear any audits we may have loaded into memory
			for(int i=0; i<arPendingResultAudits.GetSize(); i++) {
				CArray<CPendingAuditInfo,CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo,CPendingAuditInfo&>*)arPendingResultAudits.GetAt(i);
				if(parAudits) {
					delete parAudits;
					parAudits = NULL;
				}
			}
			arPendingResultAudits.RemoveAll();

			// (r.gonet 05/01/2014) - PLID 61843 - Log an error
			LogHL7Error("Nextech failed to match the patient found in the HL7 PID segment with an existing Nextech patient.");
			// (r.gonet 05/01/2014) - PLID 61843 - Log a possible resolution
			LogHL7Resolution("If this error is happening for all HL7 messages sent by this sender and it is not a known issue "
				"with this particular sender, please ensure that the sender is sending Nextech's required fields and components for patient matching: "
				"PID-2 (Patient ID), PID-5.1 (Patient Last Name), PID-5.2 (Patient First Name), and PID-7 (Patient Birth Date). PID-19 (Patient SSN) is required "
				"to match if it is filled in in the HL7 message and in Nextech. Otherwise it is ignored.");
			return FALSE;
		}

		//TES 10/8/2008 - PLID 21093 - Get the name of the overall result, we'll use that for any notes or the like
		// that don't have an individual "name".
		//TES 8/24/2010 - PLID 37500 - There isn't an overall name any more, it's separate for each lab.
		//CString strLabName = GetSpecificDescriptionForHL7Event(strMessage, nHL7GroupID);

		//TES 4/1/2010 - PLID 37500 - We need to gather all the order numbers from each OBR, and assign a lab to each of them.
		struct LabIdentifier {
			CString strOrderNumber;
			long nLabID;
			int nStart;
			int nEnd;
			CString strSpecimen;
			CString strTestCode; // (r.gonet 03/07/2013) - PLID 55490 - Added the universal service identifier from OBR-4.1
			CString strTestCodeDescription; // (d.singleton 2013-11-25 10:03) - PLID 59379 - added the obr4.2 value for reflex tests
			CString strAnatomicLocation;
			COleDateTime dtExistingBiopsyDate; //TES 7/16/2010 - PLID 39642
			LabType eLabType; //TES 8/2/2010 - PLID 38776
			CString strLabName; //TES 8/24/2010 - PLID 37500
			//TES 4/29/2011 - PLID 43424 - Gather Receiving Lab information from OBR-21
			//TES 2/25/2013 - PLID 54876 - Renamed to Performing Lab
			CString strPerformingLabID;
			CString strPerformingLabName;
			CString strPerformingLabAddress;
			CString strPerformingLabCity;
			CString strPerformingLabState;
			CString strPerformingLabZip;
			//TES 2/26/2013 - PLID 55321 - Added, this is the person at the Performing Lab who was responsible for the test (Quest needs this)
			CString strMedicalDirector;
			CString strExistingOrderStatus; //TES 5/3/2011 - PLID 43536
			CString strExistingFillerOrderNumber; //TES 5/5/2011 - PLID 43575
			CString strExistingComments; //TES 5/11/2011 - PLID 43658
			long nLabProcedureID; // (r.gonet 03/07/2013) - PLID 55489 - Added the lab procedure ID because any reflexed test will need to have the same lab procedure as the others on the requisition.
			CString strFillerOrderNumber; // (d.singleton 2013-11-08 12:02) - PLID 59379 - need to store the obr3.1 value so we can match on it later in case of reflex orders in the same hl7 message
			CString strParentResultSubID; // (d.singleton 2013-11-08 12:02) - PLID 59379 - need to store the obr3.1 value so we can match on it later in case of reflex orders in the same hl7 message
			CString strParentObservationID;
			CString strParentObservationIDText;
			// (r.gonet 2015-11-17 23:37) - PLID 67590 - Track the Practice location ID for use in auditing and creation of new labs.
			long nLocationID;

			BOOL HasParentResult() {
				return (!strParentResultSubID.IsEmpty() && !strParentObservationID.IsEmpty() && !strParentObservationIDText.IsEmpty());
			}
			
		};
		
		CArray<LabIdentifier,LabIdentifier&> arLabs;
		CArray<HL7LabResultInfo,HL7LabResultInfo&> arResults;
		int nLab = 1;
		COleDateTime dtNull;
		dtNull.SetStatus(COleDateTime::null);
		//TES 9/18/2008 - PLID 21093 - Get the Placer Order Number (that's the number we assigned, if indeed this result came 
		// from a request we sent) out of OBR-2
		//TES 5/5/2011 - PLID 43575 - Track the Filler Order Number regardless of whether we use it as the main order number.
		CString strOrderNumber, strFillerOrderNumber;
		//TES 5/11/2011 - PLID 43658 - We'll be pulling the "Report Comments" out of the message
		CString strReportComment;
		//TES 6/3/2011 - PLID 43658 - Let's get it right now, for some reason this code was only in the "new lab" branch
		//TES 5/11/2011 - PLID 43658 - We need to get the "Report Comment".  That's any NTE segments which come before the first OBR
		int nFirstOBR = FindNthOccurrence(Message.strMessage, "\rOBR", 1);
		if(nFirstOBR == -1) {
			nFirstOBR = FindNthOccurrence(Message.strMessage, "\nOBR", 1);
		}

		// Check and see if we do not have any OBR segments in which case we do not consider it a valid 
		// lab result message.
		if (nFirstOBR == -1)
		{
			LogHL7Error("The lab result message does not contain any OBR segments.");
			LogHL7Resolution("The lab company needs to re-send a valid lab result message that contains "
				"at least one OBR segment."
			);
			return FALSE;
		}

		CString strReportCommentLine;
		int nReportCommentLine = 1;
		while(SEGMENT_NOT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "NTE", nReportCommentLine, 3, 1, 1, strReportCommentLine, 0, nFirstOBR)) {
			//(s.dhole 2014-06-04 10:32) PLID 47478 - Aplly DeformatHL7Text to comment
			if(!strReportComment.IsEmpty()) strReportComment += "\r\n";
			strReportComment += DeformatHL7Text(strReportCommentLine);
			nReportCommentLine++;
			strReportCommentLine = "";
		}
		if(!strReportComment.IsEmpty()) {
			strReportComment = "REPORT COMMENTS:\r\n" + strReportComment;
		}

		// (r.gonet 03/13/2013) - PLID 55490 - We need to ensure we get the actual values, since if auto import is off, the bitmask will
		//  be 0.
		DWORD dwLabOrderMatchingFields = (DWORD)GetHL7SettingInt(nHL7GroupID, "LabImportMatchingFieldFlags");

		//TES 4/1/2010 - PLID 37500 - Loop through all the OBR messages we can find.
		while(COMPONENT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", nLab, 2, 1, 1, strOrderNumber))
		{
			long nLabID = -1;
			//TES 7/16/2010 - PLID 39642 - We also need to find the BiopsyDate if we map to an existing lab.
			COleDateTime dtExistingBiopsyDate;
			dtExistingBiopsyDate = dtNull;
			//TES 8/2/2010 - PLID 38776 - Likewise for the LabType
			LabType eLabType = ltInvalid;
			//TES 5/3/2011 - PLID 43536 - And the Order Status
			CString strExistingOrderStatus;
			//TES 5/5/2011 - PLID 43575 - Track the Filler Order Number regardless of whether we use it as the main order number.
			CString strExistingFillerOrderNumber;
			//TES 5/11/2011 - PLID 43658 - Remember the existing Comments
			CString strExistingComments;
			// (r.gonet 03/07/2013) - PLID 55489 - Initialize the lab procedure to no procedure.
			long nLabProcedureID = -1;
			// (r.gonet 2015-11-17 23:37) - PLID 67590 - Initialize the matched lab's location to no location.
			long nLocationID = -1;

			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", nLab, 3, 1, 1, strFillerOrderNumber);
			if(strOrderNumber.IsEmpty()) {
				//TES 10/8/2008 - PLID 21093 - If the Placer Order Number is empty, go ahead and try to use the 
				// Filler Order Number, that the lab assigned, OBR-3
				strOrderNumber = strFillerOrderNumber;				
			}

			// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
			LogHL7Debug("Attempting to match the %s OBR (order number = \"%s\") with an existing lab in Nextech.", GetOrdinalFromInteger(nLab), strOrderNumber);

			// (r.gonet 03/07/2013) - PLID 55490 - Get the universal service identifier from OBR-4.1
			CString strHL7TestCode;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", nLab, 4, 1, 1, strHL7TestCode);

			// (r.gonet 03/07/2013) - PLID 55489 - If this OBR is a reflex then we need to remember some things about the other orders on the same requisition.
			//  Because we'll need to add the reflex as part of the same requsition.
			BOOL bEnableReflexTesting = GetHL7SettingBit(nHL7GroupID, "EnableReflexTesting");
			
			// (r.gonet 2015-11-17 23:37) - PLID 67590 - Group the related lab variables together into a struct.
			struct RelatedLab {
				long nRelatedLabID = -1;
				CString strRelatedFormNumber;
				LabType eRelatedLabType = ltInvalid;
				long nRelatedLabProcedureID = -1;
				// (r.gonet 2015-11-17 23:37) - PLID 67590 - If we have to create a new lab order, try to use the location of the
				// other lab orders. Init to no location.
				long nRelatedLocationID = -1;
			} relatedLab;

			// (d.singleton 2013-11-08 11:28) - PLID 59379 - need to expand the funtionality of reflex tests to match on filler order number and other results of the same hl7 message file
			// an obr11 value of "G" means its a reflex order
			CString strSpecimenActionCode;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", nLab, 11, 1, 1, strSpecimenActionCode);

			//TES 4/5/2010 - PLID 37500 - If we've already mapped any lab IDs to this form number, don't use them again.
			// (r.gonet 03/07/2013) - PLID 55490 - Needs to be more lenient now. We now allow non-unique order numbers in the message if the test code from OBR-4.1 is unique. 
			// (r.gonet 03/13/2013) - PLID 55490 - Use the bitmask with everything in it.
			CArray<long,long> arynMappedLabIDs;
			arynMappedLabIDs.Add(-1);
			for(int i = 0; i < arLabs.GetSize(); i++) {
				if(arLabs[i].strOrderNumber == strOrderNumber && 
					((dwLabOrderMatchingFields & limfTestCode) != limfTestCode || arLabs[i].strTestCode == strHL7TestCode))
				{
					arynMappedLabIDs.Add(arLabs[i].nLabID);
				}
			}
			//TES 4/5/2010 - PLID 37500 - Check if there are multiple labs with this number.
			bool bHasMultiple = false;
			if(!strOrderNumber.IsEmpty())
			{
				//TES 9/18/2008 - PLID 21093 - If there is a lab for this patient with this number, use it.
				//TES 4/5/2010 - PLID 37500 - Filter out Labs we've already mapped to.
				// (z.manning 2010-06-08 11:51) - PLID 39041 - We need to factor in specimen too so
				// load all of this patient's labs and see if we have any matches.
				//TES 7/16/2010 - PLID 39642 - Added BiopsyDate
				//TES 8/2/2010 - PLID 38776 - Added Type
				//TES 5/3/2011 - PLID 43536 - Added OrderStatus
				//TES 5/5/2011 - PLID 43575 - Added FillerOrderNumber
				//TES 5/11/2011 - PLID 43658 - Added ClinicalData (Comments)
				// (r.gonet 03/07/2013) - PLID 55490 - Added LOINC_Code and LabProcedureID
				// (r.gonet 03/07/2013) - PLID 55489 - Added a second recordset which will only
				//  be used if we have failed to match ANY existing labs and reflexing is enabled.
				// (r.gonet 2015-11-17 23:37) - PLID 67590 - Formatted the query and added selection of
				// LabsT.LocationID to both statements. 
				_RecordsetPtr rsLab = CreateParamRecordset(R"(
SELECT 
	LabsT.ID, 
	LabsT.FormNumberTextID, 
	LabsT.Specimen, 
	LabsT.LOINC_Code, 
	LabsT.BiopsyDate, 
	LabsT.Type, 
	LabOrderStatusT.Description AS OrderStatus, 
	LabsT.FillerOrderNumber,
	LabsT.ClinicalData, 
	LabsT.LabProcedureID,
	LabsT.LocationID
FROM LabsT 
LEFT JOIN LabOrderStatusT ON LabsT.OrderStatusID = LabOrderStatusT.ID
WHERE LabsT.PatientID = {INT} 
	AND LabsT.Deleted = 0 
	AND LabsT.ID NOT IN ({INTARRAY})

SELECT 
	MIN(LabsT.ID) AS ID, 
	LabsT.FormNumberTextID, 
	MIN(LabsT.Type) AS Type, 
	MIN(LabsT.LabProcedureID) AS LabProcedureID,
	MIN(LabsT.LocationID) AS LocationID
FROM LabsT
WHERE LabsT.PatientID = {INT} 
	AND LabsT.Deleted = 0
GROUP BY LabsT.FormNumberTextID; 
)"
, nPersonID, arynMappedLabIDs, nPersonID);

				if(rsLab->eof) {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("This patient has no unmatched lab records (labs that have not already been matched during this import) to match this OBR with.");
				}
				for(; !rsLab->eof && !bHasMultiple; rsLab->MoveNext()) {
					if(nLabID != -1) {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("Continuing to see if multiple existing Nextech labs match the HL7 OBR.");
					}
					long nPracticeLabID = AdoFldLong(rsLab->GetFields(), "ID", -1);
					CString strFormNumber = AdoFldString(rsLab->GetFields(), "FormNumberTextID", "");
					CString strSpecimen = AdoFldString(rsLab->GetFields(), "Specimen", "");
					// (r.gonet 03/07/2013) - PLID 55490 - Get the universal service identifier of the Practice lab order.
					CString strTestCode = AdoFldString(rsLab->GetFields(), "LOINC_Code", "");

					if ((dwLabOrderMatchingFields & limfFormNumber) == limfFormNumber && (dwLabOrderMatchingFields & limfSpecimen) == limfSpecimen) {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("Attempting to match the %s HL7 OBR (Order Number = \"%s\") with the existing Nextech lab "
							"(ID = %li; Form Number = \"%s\"; Specimen = \"%s\").",
							GetOrdinalFromInteger(nLab), strOrderNumber, nPracticeLabID, strFormNumber, strSpecimen);
					} else if ((dwLabOrderMatchingFields & limfFormNumber) == limfFormNumber && (dwLabOrderMatchingFields & limfTestCode) == limfTestCode) {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("Attempting to match the %s HL7 OBR (Order Number = \"%s\"; Test Code = \"%s\") with the existing Nextech lab "
							"(ID = %li; Form Number = \"%s\"; Test Code = \"%s\").",
							GetOrdinalFromInteger(nLab), strOrderNumber, strHL7TestCode, nPracticeLabID, strFormNumber, strTestCode);
					} else {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
						LogHL7Warning("Lab Import Matching Fields are non-standard (Flags = %lu). "
							"Attempting to match the %s HL7 OBR (Order Number = \"%s\"; Test Code = \"%s\") with the existing Nextech lab "
							"(ID = %li; Form Number = \"%s\"; Specimen = \"%s\"; Test Code = \"%s\").",
							dwLabOrderMatchingFields, GetOrdinalFromInteger(nLab), strOrderNumber, strHL7TestCode, nPracticeLabID, strFormNumber, strSpecimen, strTestCode);
					}
					// (r.gonet 03/07/2013) - PLID 55490 - Is this OBR for this LabsT record? Compare based on a subset set of fields determined by the matching field flags.
					// (r.gonet 03/13/2013) - PLID 55490 - Use the bitmask with everything in it.
					if(DoLabOrdersMatch(strOrderNumber, strHL7TestCode, strFormNumber, strSpecimen, strTestCode, dwLabOrderMatchingFields)) {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("Successfully matched the HL7 OBR segment with an existing Nextech lab (ID = %li).", nPracticeLabID);
						if(nLabID == -1) {
							// (z.manning 2010-06-07 16:31) - PLID 39041 - First match
							nLabID = AdoFldLong(rsLab->GetFields(), "ID");
							//TES 7/16/2010 - PLID 39642 - Pull the biopsy date too.
							dtExistingBiopsyDate = AdoFldDateTime(rsLab, "BiopsyDate", dtNull);
							//TES 8/2/2010 - PLID 38776 - Get the LabType
							eLabType = (LabType)AdoFldByte(rsLab, "Type");
							//TES 5/3/2011 - PLID 43536 - Get the OrderStatus
							strExistingOrderStatus = AdoFldString(rsLab, "OrderStatus", "");
							//TES 5/5/2011 - PLID 43575 - Get the Filler Order Number
							strExistingFillerOrderNumber = AdoFldString(rsLab, "FillerOrderNumber", "");
							//TES 5/11/2011 - PLID 43658 - Get the Comments
							strExistingComments = AdoFldString(rsLab, "ClinicalData", "");
							// (r.gonet 03/07/2013) - PLID 55489 - Get the lab procedure as well. We don't need it since
							//  we already matched an existing lab in Practice, but this is to be consistent with
							//  what reflexed tests need.
							nLabProcedureID = AdoFldLong(rsLab, "LabProcedureID", -1);
							// (r.gonet 2015-11-17 23:37) - PLID 67590 - Load the existing lab's location.
							nLocationID = AdoFldLong(rsLab, "LocationID", -1);

							if(bCopyResult) {
								// (r.gonet 2011-04-08) - PLID 42655 - All we need is one. The results are copied to all labs with this form number.
								if(!strOrderNumber.IsEmpty() && strOrderNumber == strFormNumber) {
									// (r.gonet 2011-04-08) - PLID 42655 - We only have a form number. Add the specimen ID.
									strOrderNumber = strOrderNumber + " - " + strSpecimen;
								}
								break;
							}
						}
						else {
							// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
							LogHL7Debug("However, there was already a match.");
							// (z.manning 2010-06-07 16:32) - PLID 39041 - Multiple matches, we need to prompt.
							nLabID = -1;
							//TES 7/16/2010 - PLID 39642 - Clear the biopsy date
							dtExistingBiopsyDate = dtNull;
							//TES 8/2/2010 - PLID 38776 - Clear the LabType
							eLabType = ltInvalid;
							//TES 5/3/2011 - PLID 43536 - Clear the OrderStatus
							strExistingOrderStatus = "";
							//TES 5/5/2011 - PLID 43575 - Filler Order number
							strExistingFillerOrderNumber = "";
							//TES 5/11/2011 - PLID 43658 - Comments
							strExistingComments = "";
							// (r.gonet 03/07/2013) - PLID 55489 - Reset the lab procedure as well.
							nLabProcedureID = -1;
							bHasMultiple = true;
							break;
						}
					} else {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("Could not match the HL7 OBR segment with the existing Nextech lab (ID = %li).", nPracticeLabID);
					}
				}

				if(nLabID != -1) {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("Successfully matched exactly one existing Nextech lab (ID = %li) to the HL7 OBR.", nLabID);
				} else if(bHasMultiple) {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("Matched multiple existing Nextech labs to the HL7 OBR.");
				} else {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("Could not match any existing Nextech labs to the HL7 OBR.");
				}

				// (r.gonet 03/07/2013) - PLID 55489 - If reflexing is enabled and we haven't matched anything,
				//  then we'll regard this OBR as a reflex if the form number matches some requisition's form number.
				//  See what requisition it is supposed to go with.
				// (r.gonet 2015-11-17 23:37) - PLID 67590 - Enter this branch under a less strict condition.
				// Now we try to get the related lab if we've failed to match any existing lab, regardless of the reflex setting.
				// There's no harm in doing so and if we find one, we'll be able to use its location for the new lab order. Hooray.
				// Even in the case where there are multiple orders that match, they'll be presented with the option of creating a new lab order.
				if(nLabID == -1) {
					rsLab = rsLab->NextRecordset(NULL);
					// (r.gonet 03/07/2013) - PLID 55489 - Nothing we tried matched! See if this is a reflex off of some requisition.
					for(; !rsLab->eof; rsLab->MoveNext()) {
						CString strFormNumber = AdoFldString(rsLab->GetFields(), "FormNumberTextID", "");
						if(DoFormNumbersMatch(strOrderNumber, strFormNumber, "", limfFormNumber)) {
							// (r.gonet 03/07/2013) - PLID 55489 - This is a reflexed test that belongs to this requisition. So we'll create it.
							relatedLab.nRelatedLabID = AdoFldLong(rsLab->GetFields(), "ID", -1);
							relatedLab.strRelatedFormNumber = strFormNumber;
							relatedLab.eRelatedLabType = (LabType)AdoFldByte(rsLab, "Type");
							relatedLab.nRelatedLabProcedureID = AdoFldLong(rsLab, "LabProcedureID");
							// (r.gonet 2015-11-17 23:37) - PLID 67590 - Cache the related existing lab order's location.
							relatedLab.nRelatedLocationID = AdoFldLong(rsLab, "LocationID");
							break;
						} else {
							// (r.gonet 03/07/2013) - PLID 55489 - Not a reflex of this requisition. Try another.
						}
					}
					// (r.gonet 2015-11-17 23:37) - PLID 67590 - The following is only applicable to reflexed orders.
					if(bEnableReflexTesting && !bHasMultiple && relatedLab.nRelatedLabID != -1) {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("This HL7 OBR has been determined to be a reflex of an existing Nextech lab (ID = %li). A new lab order will automatically be created for the reflex.", relatedLab.nRelatedLabID);
					}
				} else {
					// (r.gonet 03/07/2013) - PLID 55489 - If we have a lab matched, great. If we have multiple, the user will have to choose from them. If reflexing, then we are not creating this lab just
					//  because it in Practice.
				}

			} else {
				// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
				LogHL7Warning("The %s OBR has no order number in OBR-2.1 or OBR-3.1. Nextech requires there to be an order number. Nextech will need to prompt "
					"the user to choose a lab to associate with this OBR.", GetOrdinalFromInteger(nLab));
			}
			LabIdentifier li;
			li.strOrderNumber = strOrderNumber;
			// (r.gonet 03/07/2013) - PLID 55490 - Save the universal service identifier as well since, if we are creating a new lab, we'll be creating the lab with the LOINC field set to it.
			li.strTestCode = strHL7TestCode;
			// (d.singleton 2013-11-08 12:04) - PLID 59379 - need this stored per result in our array so we can match on it later in case of reflex orders in same hl7 message
			li.strFillerOrderNumber = strFillerOrderNumber;

			//TES 8/24/2010 - PLID 37500 - Also get the lab "name", OBR-4.2
			CString strLabName;
			if(COMPONENT_NOT_FOUND == GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 4, 1, 2, strLabName)) {
				//TES 9/18/2008 - Some lab companies (incorrectly) just send a single component, so try pulling that one.
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 4, 1, 1, strLabName);
			}

			//TES 5/11/2011 - PLID 43634 - Check the setting for whether to append the OBR-20.5 value to the name (Quest does this).
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			if(GetHL7SettingBit(nHL7GroupID, "AppendObr20")) {
				CString strExtraName;
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 20, 1, 5, strExtraName);
				if(!strExtraName.IsEmpty()) {
					strLabName += "\r\n" + strExtraName;
				}
			}
			li.strLabName = strLabName;

			//TES 4/29/2011 - PLID 43424 - Get the lab location from OBR-21, we may or may not end up using it, but let's get the
			// information while we're here.
			//TES 2/25/2013 - PLID 54876 - Renamed to Performing Lab, and moved up here, because we'll be using this information whether or not we
			// match the lab.
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 1, li.strPerformingLabID);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 2, li.strPerformingLabName);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 3, li.strPerformingLabAddress);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 4, li.strPerformingLabCity);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 5, li.strPerformingLabState);
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 6, li.strPerformingLabZip);
			//TES 2/26/2013 - PLID 55321 - We also need to pull the 7th component, the "Medical Director"
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 21, 1, 7, li.strMedicalDirector);

			//TES 10/8/2008 - PLID 21093 - If we haven't been able to link it silently, try prompting the user.
			if(nLabID == -1) {
				// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
				LogHL7Debug("Could not match the %s OBR (order number = \"%s\") to any existing lab records in Nextech.", GetOrdinalFromInteger(nLab), strOrderNumber);

				// (r.gonet 2015-11-17 23:37) - PLID 67590 - If we have a related lab order and we have no location, use the related lab's location.
				if (relatedLab.nRelatedLocationID != -1 && nLocationID == -1) {
					nLocationID = relatedLab.nRelatedLocationID;
				}

				// (r.gonet 03/07/2013) - PLID 55489 - OK, so we haven't been able to match up any existing lab order
				//  to this OBR, but is it Related to any existing lab order?
				if(bEnableReflexTesting && !bHasMultiple && relatedLab.nRelatedLabID != -1) {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("The setting \"Enable support for reflex testing\" is enabled. Considering this OBR a reflexed test.");
					// (r.gonet 03/07/2013) - PLID 55489 - This OBR is related to some other order, but doesn't belong to it. 
					//  This is a reflex. Create a LabsT record for it.
					//  We do need to have the reflexed test use the same form number and same lab type so it can belong to the same requisition it was reflexed from.
					li.strOrderNumber = relatedLab.strRelatedFormNumber;
					CString strSplitFormNumber, strSplitSpecimen;
					if(SplitFormNumberAndSpecimen(strOrderNumber, strSplitFormNumber, strSplitSpecimen)) {
						// (r.gonet 03/07/2013) - PLID 55489 - This follows our form number - specimen format, so split it now.
						li.strSpecimen = strSplitSpecimen;
					}
					// (r.gonet 03/07/2013) - PLID 55489 - Set a few variables that will be used in creating the lab later.
					// Since this is related to some requisition, we need to use that requisition's type and procedure.
					eLabType = relatedLab.eRelatedLabType;
					nLabProcedureID = relatedLab.nRelatedLabProcedureID;
				}
				// (d.singleton 2013-11-08 11:32) - PLID 59379 - didnt match to any existing lab requisitions,  but what about other orders on this same hl7 file
				else if(bEnableReflexTesting && !bHasMultiple && strSpecimenActionCode == "G") {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("The setting \"Enable support for reflex testing\" is enabled. There is no existing Nextech lab order with the same form number. "
						"However, the specimen action code is \"G\". Considering this OBR a reflexed test.");
					CString strParentResult, strParentOrder, strParentOrderNum, strSeperator;
					long nFirstSep, nSecondSep;
					strSeperator = GetSeparator(stSubComponent);
					GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 26, 1, 1, strParentResult);
					GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 29, 1, 1, strParentOrder);
					//get the same order num as the parent
					nFirstSep = strParentOrder.Find(strSeperator);
					if(nFirstSep != -1) {							
						strParentOrderNum = strParentOrder.Left(nFirstSep);
					}
					//now we need the test name and code
					nFirstSep = strParentResult.Find(strSeperator);
					if(nFirstSep != -1) {
						li.strParentObservationID = strParentResult.Left(nFirstSep);
					}
					nSecondSep = strParentResult.Find(strSeperator, nFirstSep + 1);
					if(nSecondSep != -1) {
						li.strTestCodeDescription = strParentResult.Mid(nFirstSep + 1, nSecondSep - (nFirstSep + 1));
						li.strParentObservationIDText = strParentResult.Mid(nFirstSep + 1, nSecondSep - (nFirstSep + 1));
					}		

					//see if we have any matching parent orders,  match the last one if more than one match exists
					// (c.haag 2014-04-03) - PLID 61643 - This should default to NULL.
					LabIdentifier* liParentLab = NULL;
					for(int i = 0; i < arLabs.GetCount(); i++) {
						//check to see if their filler order numbers match
						if(arLabs[i].strOrderNumber.CompareNoCase(strParentOrderNum) == 0) {
							liParentLab = &arLabs[i];																												
						}
					}
					if(liParentLab) {
						eLabType = liParentLab->eLabType;
						nLabProcedureID = liParentLab->nLabProcedureID;
						// (r.gonet 2015-11-17 23:37) - PLID 67590 - Use the parent lab's location.
						nLocationID = liParentLab->nLocationID;
						li.strOrderNumber = liParentLab->strOrderNumber;
						//need to use the parent service id text
						GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 26, 1, 2, li.strParentResultSubID);
					}
				}
				else {
					// (r.gonet 03/07/2013) - PLID 55489 - This is no reflex. We don't even have a requisition with a matching form number for the patient. See what the user wants to do.
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("The OBR is not a reflexed test, so Nextech won't create it automatically based on that fact. "
						"Check the setting for how Nextech should deal with unmatched labs.");
				
					//TES 11/2/2009 - PLID 36122 - Gather yet still more information
					//TES 4/2/2010 - PLID 38040 - Check our setting for which component to use.
					//TES 4/5/2010 - PLID 38040 - Also, we need to pull this regardless of whether we're going to be prompting the user.
					//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
					long nOBR15Component = GetHL7SettingInt(nHL7GroupID, "AnatomicLocationComponent");
					CString strAnatomicLocation;
					if(nOBR15Component != -1) {
						GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 15, 1, nOBR15Component, strAnatomicLocation);
					}

					//TES 11/2/2009 - PLID 36122 - We actually have subcomponents!  We need the second subcomponent
					// of the OBR-15.1 field (which we just pulled), to get the text version of the "Specimen Source"
					CString strSeparator = GetSeparator(stSubComponent);
					int nFirstSep = strAnatomicLocation.Find(strSeparator);
					if(nFirstSep != -1) {
						int nSecondSep = strAnatomicLocation.Find(strSeparator, nFirstSep+1);
						if(nSecondSep == -1) {
							strAnatomicLocation = strAnatomicLocation.Mid(nFirstSep+1);
						}
						else {
							strAnatomicLocation = strAnatomicLocation.Mid(nFirstSep+1, nSecondSep-(nFirstSep+1));
						}
					}
					//TES 4/5/2010 - PLID 38040 - Remember the anatomic location.
					li.strAnatomicLocation = strAnatomicLocation;

					_RecordsetPtr rsLab;
					CSqlFragment sqlExtraWhere;
					if(!bHasMultiple) {
						//TES 9/23/2008 - PLID 21093 - Does the patient have ANY labs?
						rsLab = CreateParamRecordset("SELECT ID FROM LabsT WHERE Deleted = 0 AND PatientID = {INT}", nPersonID);
					}
					else {
						// (r.gonet 03/07/2013) - PLID 55490 - The order number is not necessarily the form number.
						CString strFormNumber, strSpecimen;
						if(!SplitFormNumberAndSpecimen(li.strOrderNumber, strFormNumber, strSpecimen)) {
							strFormNumber = li.strOrderNumber;
						}
						//TES 4/5/2010 - PLID 37500 - Just get labs with this form number, that we haven't mapped to.
						sqlExtraWhere = CSqlFragment(" AND FormNumberTextID = {STRING} AND LabsT.ID NOT IN ({INTARRAY})", strFormNumber, arynMappedLabIDs);
						rsLab = CreateParamRecordset("SELECT ID FROM LabsT WHERE Deleted = 0 AND PatientID = {INT} {SQL}"
							, nPersonID, sqlExtraWhere);
					}
					if(!rsLab->eof) {
						CSingleSelectDlg dlgSelectLab(NULL);
						// (z.manning 2008-10-30 11:29) - PLID 31864 - Show to be ordered instead of anatomic location
						// for non-biopsy labs
						//TES 10/6/2009 - PLID 35695 - Gather more information about the message to put on this screen.
						CString strTest;
						if(COMPONENT_NOT_FOUND == GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 4, 1, 2, strTest)) {
							//TES 9/18/2008 - PLID 21093 - Some lab companies (incorrectly) just send a single component, 
							// so try pulling that one.
							GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 4, 1, 1, strTest);
						}

						// (r.gonet 03/07/2013) - PLID 55490 - But we also need the individual values.
						CString strTestCode, strTestDescription;
						GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 4, 1, 1, strTestCode);
						GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 4, 1, 2, strTestDescription);

						//TES 7/29/2011 - PLID 44734 - Also pull the Collection Date (OBR-7)
						CString strDateObserved;
						GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", nLab, 7, 1, 1, strDateObserved);
						COleDateTime dtObserved;
						dtObserved.SetStatus(COleDateTime::invalid);
						if(!strDateObserved.IsEmpty()) {
							// (a.walling 2010-02-25 14:17) - PLID 37543 - Include time (even if we don't end up using it in the interface, better to have more info than less)
							if(ParseHL7DateTime(strDateObserved, dtObserved)) {
								strDateObserved = FormatDateTimeForInterface(dtObserved, 0, dtoDateTime);
							}
							else {
								strDateObserved = "";
							}
						}

						//TES 4/5/2010 - PLID 37500 - If this has a form number, and bHasMultiple is false(which means there aren't any existing labs
						// with this form number), and we've already mapped a lab with this form number to -1, then just go ahead and map this one 
						// to -1 as well.
						bool bMapped = false;
						for(int i = 0; i < arLabs.GetSize() && !bMapped && !bHasMultiple && !strOrderNumber.IsEmpty(); i++) {
							if(arLabs[i].strOrderNumber == strOrderNumber && arLabs[i].nLabID == -1) {
								// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
								LogHL7Debug("The order number \"%s\" has already been encountered at least once before while importing this message. "
									"With that OBR, Nextech (or the user) determined it would create a new lab for it. Since this OBR has the same order number, "
									"Nextech has automatically determined it will create a new lab for this OBR as well.", strOrderNumber);
								bMapped = true;
							}
						}
						if(!bMapped) {
							// (r.gonet 06/05/2012) - PLID 50629 - We couldn't find any matching lab order in our data. We have a setting that
							//  controls what we do next. Do we:
							//  1) Prompt the user to select a lab or create a new one or
							//  2) Create a new one automatically
							//  Note that if we had multiple candidates that we could map to, then we just prompt anyway.
							long nUnmatchedLabBehavior = GetHL7SettingInt(nHL7GroupID, "UnmatchedLabBehavior");
							if(nUnmatchedLabBehavior == ulbPrompt || bHasMultiple) {
								if(nUnmatchedLabBehavior == ulbPrompt) {
									// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
									LogHL7Debug("The setting \"When lab results come in with an order number not matching an order in the system\" is set to \"Prompt to match or create a lab order\", "
										"Nextech will prompt the user to select an existing lab to match with this OBR.");
								} else {
									// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
									LogHL7Debug("Even though the setting \"When lab results come in with an order number not matching an order in the system\" is set to \"Automatically create a lab order\", "
										"Nextech will prompt the user to select an existing lab to match with this OBR because there are multiple matches rather than no matches.");
								}
								// Prompt the user for what to do

								// (r.gonet 03/07/2013) - PLID 55492 - Output the universal service identifier code and description,
								//  which many labs send so that the user can make a more informed decision.
								CString strTestCodeAndDescription;
								if(!strTestCode.IsEmpty()) {
									strTestCodeAndDescription += strTestCode;
								}
								if(!strTestDescription.IsEmpty()) {
									if(!strTestCodeAndDescription.IsEmpty()) {
										strTestCodeAndDescription += " - ";
									}
									strTestCodeAndDescription += strTestDescription;
								}
								
								
								//TES 4/5/2010 - PLID 37500 - 
								CString strDescription;
								//TES 7/29/2011 - PLID 44734 - Show the Observation Date (also added a newline)
								// (r.gonet 03/07/2013) - PLID 55492 - Added the universal service identifier and description.
								strDescription.Format("This patient (%s) has one or more existing lab records.  "
									"Please select a lab to attach this incoming result to, or New Lab to create a new lab record with this result.\r\n\r\n"
									"Form #: %s, Test: %s\r\nSpecimen Source: %s, Observation Date: %s",
									strPatientName, strOrderNumber, strTestCodeAndDescription, strAnatomicLocation, strDateObserved);

								// (z.manning 2010-05-11 14:28) - PLID 37416 - Let's add an option to filter our completed labs
								// (c.haag 2010-12-14 9:30) - PLID 41806 - LabsT no longer has completed flags. We need to calculate
								// it based on whether the lab has results and whether all results are completed. Old filter was simply
								// "Labs Completed Date IS NULL"
								//TES 3/26/2012 - PLID 49222 - Also hide discontinued labs
								dlgSelectLab.UseAdditionalFilter(
									"(LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LabsT.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL)) "
									" AND LabsT.Discontinued = 0 ", 
									"Hide completed/discontinued labs", TRUE);
								// (z.manning 2010-05-11 14:47) - PLID 37416 - Let's also rename the cancel button to make it clearer to the user
								dlgSelectLab.SetCancelButtonStyle("New Lab", NXB_NEW);

								//TES 4/5/2010 - PLID 37500 - Apply order number filtering if we decided to up above.
								//TES 4/7/2010 - PLID 38040 - Added the anatomic side and qualifier to the description.
								// (z.manning 2010-04-30 15:34) - PLID 37553 - We now have a view to load anatomic location
								//TES 8/5/2010 - PLID 39642 - Force the user to make a selection.
								// (c.haag 2011-01-05 16:07) - PLID 41806 - LabsT no longer has completed flags. We need to calculate
								// it based on whether the lab has results and whether all results are completed. Old filter was simply
								// "Labs Completed Date IS NULL"
								//TES 7/29/2011 - PLID 44734 - Include the date (BiopsyDate, or InputDate if there's no BiopsyDate)
								// (r.gonet 03/07/2013) - PLID 55492 - Inculde the lab's LOINC value (aka universal service identifier) to help the user
								//  make an informed decision.
								if(IDOK == dlgSelectLab.Open("LabsT LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID", FormatString("Deleted = 0 AND PatientID = %li %s", nPersonID, sqlExtraWhere.Flatten()), 
									"LabsT.ID", 
									"CASE WHEN (LabsT.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) OR LabsT.ID IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0 AND ResultCompletedDate IS NULL)) THEN '' ELSE '(Done) ' END + " 
									"LTRIM(RTRIM(COALESCE(FormNumberTextID,'') + '-' + COALESCE(Specimen,'') + ' (' + Convert(nvarchar(255),COALESCE(BiopsyDate,InputDate),101) + ') - ' + CASE WHEN LabsT.Type = 1 THEN "
									"LabAnatomicLocationQ.AnatomicLocation ELSE ToBeOrdered END + ' ' + "
									"CASE WHEN (LOINC_Code IS NOT NULL AND LOINC_CODE <> '') AND (LOINC_Description IS NOT NULL AND LOINC_Description <> '') THEN '(' + LOINC_Code + ' - ' + LOINC_Description + ')' "
									"	  WHEN (LOINC_Code IS NOT NULL AND LOINC_CODE <> '') AND (LOINC_Description IS NULL OR LOINC_Description = '') THEN '(' + LOINC_Code + ')' "
									"	  WHEN (LOINC_Code IS NULL OR LOINC_CODE = '') AND (LOINC_Description IS NOT NULL AND LOINC_Description <> '') THEN '(' + LOINC_Description + ')' "
									"	  ELSE '' "
									"END)) "
									, strDescription, true)) {
									nLabID = dlgSelectLab.GetSelectedID();

									// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
									LogHL7Debug("The user chose an existing lab (ID = %li) to match with this OBR.", nLabID);

									//TES 7/16/2010 - PLID 39642 - I know it's not ideal to have another data access here, but it's either that,
									// or rewrite CSingleSelectDlg to return extra fields, and this feature has to be done today.
									//TES 8/2/2010 - PLID 38776 - Also pull the labtype
									//TES 5/3/2011 - PLID 43536 - And the Order Status
									//TES 5/5/2011 - PLID 43575 - And the FillerOrderNumber
									//TES 5/11/2011 - PLID 43658 - And ClinicalData (Comments)
									// (r.gonet 03/07/2013) - PLID 55489 - Added the lab procedure ID
									// (r.gonet 2015-11-17 23:37) - PLID 67590 - Also load the existing lab's location.
									_RecordsetPtr rsExistingLab = CreateParamRecordset("SELECT BiopsyDate, Type, LabOrderStatusT.Description AS OrderStatus, FillerOrderNumber, "
										"ClinicalData, LabProcedureID, LabsT.LocationID "
										"FROM LabsT LEFT JOIN LabOrderStatusT ON LabsT.OrderStatusID = LabOrderStatusT.ID "
										"WHERE LabsT.ID = {INT}", nLabID);
									if(!rsExistingLab->eof) {
										dtExistingBiopsyDate = AdoFldDateTime(rsExistingLab, "BiopsyDate", dtNull);
										eLabType = (LabType)AdoFldByte(rsExistingLab, "Type");
										strExistingOrderStatus = AdoFldString(rsExistingLab, "OrderStatus", "");
										strExistingFillerOrderNumber = AdoFldString(rsExistingLab, "FillerOrderNumber", "");
										strExistingComments = AdoFldString(rsExistingLab, "ClinicalData", "");
										// (r.gonet 03/07/2013) - PLID 55489 - Since we use the lab procedure for reflexes
										//  now, gather it here too to be consistent.
										nLabProcedureID = AdoFldLong(rsExistingLab, "LabProcedureID", -1);
										// (r.gonet 2015-11-17 23:37) - PLID 67590 - Use the existing lab's location.
										nLocationID = AdoFldLong(rsExistingLab, "LocationID");
									}
									else {
										// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
										LogHL7Warning("The lab the user selected does not exist in Nextech!");
										//TES 7/16/2010 - PLID 39642 - What?  How can this lab not exist?
										ASSERT(FALSE);
										dtExistingBiopsyDate = dtNull;
										eLabType = ltInvalid;
										strExistingOrderStatus = "";
										strExistingFillerOrderNumber = "";
										strExistingComments = "";
										// (r.gonet 03/07/2013) - PLID 55489 - Since we use the lab procedure for reflexes
										//  now, gather it here too to be consistent.
										nLabProcedureID = -1;
									}

								} else {
									// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
									LogHL7Debug("The user chose to create a New Lab for this OBR.");
								}
							} else if(nUnmatchedLabBehavior == ulbAutocreateOrder) {
								// (r.gonet 06/05/2012) - PLID 50629 - Create a new lab order automatically

								// Don't do anything. This will be handled later when we go to actually create the
								//  lab and find that we have nothing matched.
								// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
								LogHL7Debug("The setting \"When lab results come in with an order number not matching an order in the system\" is set to \"Automatically create a lab order\". "
									"Nextech will skip prompting the user to select an exising lab in Nextech and instead automatically create a new lab.");
							}
						}
					} else {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("This patient has no labs (or at least has no labs that Nextech or the user has not mapped to yet during this import) to match this OBR with. "
							"A lab will be created automatically for this OBR.");
					}
				}
			}
			li.nLabID = nLabID;
			//TES 7/16/2010 - PLID 39642 - Remember the biopsy date
			li.dtExistingBiopsyDate = dtExistingBiopsyDate;
			//TES 8/2/2010 - PLID 38776 - Remember the type
			li.eLabType = eLabType;
			//TES 5/3/2011 - PLID 43536 - Remember the Order Status
			li.strExistingOrderStatus = strExistingOrderStatus;
			//TES 5/5/2011 - PLID 43575 - And the Filler Order Number
			li.strExistingFillerOrderNumber = strExistingFillerOrderNumber;
			//TES 5/11/2011 - PLID 43658 - And the Comments
			li.strExistingComments = strExistingComments;
			// (r.gonet 03/07/2013) - PLID 55489 - This will be the lab procedure we will use in creating the lab (if we do that)
			li.nLabProcedureID = nLabProcedureID;
			// (r.gonet 2015-11-17 23:37) - PLID 67590 - Set the lab's location from what we found to be either the existing order's location
			// or a related lab order's location if this one doesn't exist. It is still possible for this to be -1 though if there is no existing
			// matching lab order nor an existing related lab order.
			li.nLocationID = nLocationID;

			//TES 4/1/2010 - PLID 37500 - Now remember where this lab begins and ends so we only pull related results.
			int nOBRBegin = FindNthOccurrence(Message.strMessage, "\rOBR", nLab);
			if(nOBRBegin == -1) {
				nOBRBegin = FindNthOccurrence(Message.strMessage, "\nOBR", nLab);
			}
			li.nStart = nOBRBegin;
			int nOBREnd = FindNthOccurrence(Message.strMessage, "\rOBR", nLab+1);
			if(nOBREnd == -1) {
				//TES 5/7/2010 - PLID 37500 - Assign the result to the variable.
				nOBREnd = FindNthOccurrence(Message.strMessage, "\nOBR", nLab+1);
			}
			if(nOBREnd == -1) {
				li.nEnd = Message.strMessage.GetLength();
			}
			else {
				li.nEnd = nOBREnd;
			}

			arLabs.Add(li);

			nLab++;
		}

		// (c.haag 2010-09-14 15:06) - PLID 40176 - Before we add the labs to data, and before we try to
		// auto-assign them specimens, deal with specimen parsing.
		int i;
		if (1 == nLabSpecimenParseMethod) 
		{
			// For each lab, see if there's a specimen embedded in the order number. If there is, remove it from
			// the order number and put it in the specimen field.
			for(i = 0; i < arLabs.GetSize(); i++) 
			{
				LabIdentifier& li = arLabs[i];
				strOrderNumber = li.strOrderNumber;
				// See if the order number is big enough to hold a specimen
				if (strOrderNumber.GetLength() > 2) 
				{
					// (j.kuziel 2014-04-29) - PLID 60396 - Optimized this section to use any <form number> - <specimen ID> pattern.
					CString strSplitFormNumber;
					CString strSplitSpecimenID;
					if (SplitFormNumberAndSpecimen(strOrderNumber, strSplitFormNumber, strSplitSpecimenID)) {
						// If we get here, then we consider this to have a specimen. Take the specimen out of the
						// order number and move it into the specimen field.
						li.strSpecimen = strSplitSpecimenID;
						li.strOrderNumber = strSplitFormNumber;
					}
				}
			}
		}

		// (r.gonet 2011-04-08) - PLID 42655 - Duplicate a singular result to all specimens within the lab
		if(bCopyResult && arLabs.GetSize() == 1) {
			// (r.gonet 2011-03-10) - PLID 42655 - Find others labs with this form number
			// (d.thompson 2013-07-09) - PLID 56806 - The initial implementation of this query would only copy results
			//	if there was 1 OBR, but also if the destination specimens had no existing results.  Noone is sure why
			//	that requirement is in here, and we can't come up with a totally logical explanation for it.  It seems
			//	like if we're going to offer to copy singular OBR's, we should always do so.  I removed the requirement
			//	for having no lab results.
			_RecordsetPtr rsCopyToLabs = CreateParamRecordset(
					"SELECT A.ID, A.Specimen FROM LabsT A JOIN LabsT B ON A.FormNumberTextID = B.FormNumberTextID "
					"WHERE B.ID = {INT} AND "
						"A.ID <> B.ID AND "
						"A.Deleted = 0 "
						// (d.thompson 2013-07-09) - PLID 56806
						//"AND A.ID NOT IN (SELECT LabID FROM LabResultsT WHERE Deleted = 0) "
					, arLabs[0].nLabID);
			while(!rsCopyToLabs->eof) {
				// (r.gonet 2011-03-10) - PLID 42655 - Now create a duplicate result for those labs found
				long nLabID = VarLong(rsCopyToLabs->Fields->Item["ID"]->Value);
				CString strSpecimen = VarString(rsCopyToLabs->Fields->Item["Specimen"]->Value, "");
				LabIdentifier liTemp = arLabs[0];
				liTemp.nLabID = nLabID;
				liTemp.strSpecimen = strSpecimen;		
				arLabs.Add(liTemp);
				rsCopyToLabs->MoveNext();
			}
			rsCopyToLabs->Close();
		} else if(bCopyResult) {
			// (r.gonet 2011-04-08) - PLID 42655 - Copy Result should not be turned on if the message has multiple OBRs. If it were to be turned on, then behavior would be undefined.
			bCopyResult = FALSE;
			ASSERT(FALSE);
			// (r.gonet 05/01/2014) - PLID 61843 - Log a warning. This should be impossible though since we turned off the setting above if they have more than 1 OBR.
			LogHL7Warning("The setting \"Copy singular results to all specimens\" is enabled but there are multiple OBR segments in the message. The setting should not "
				"be enabled when the lab company sends multiple OBRs.");
		}

		//TES 4/5/2010 - PLID 37500 - Now, go through all our labs which we will be creating, and if they have a -1 LabID, and there is more
		// than one lab with the same form number, make sure there's a specimen.
		for(i = 0; i < arLabs.GetSize(); i++) {
			if(arLabs[i].nLabID == -1 && arLabs[i].strSpecimen.IsEmpty()) {
				bool bIsOtherLab = false;
				bool bOtherLabExists = false;
				for(int j = 0; j < arLabs.GetSize(); j++) {
					if(i != j && arLabs[i].strOrderNumber == arLabs[j].strOrderNumber) {
						bIsOtherLab = true;
						if(!arLabs[j].strSpecimen.IsEmpty()) {
							bOtherLabExists = true;
						}
					}
				}
				if(bIsOtherLab) {
					long nSpecimen = -1;
					bool bIsSpecimenAlpha = true;
					bool bZeroPrefix = false;
					if(bOtherLabExists) {
						//TES 4/5/2010 - PLID 37500 - Pull the highest specimen for this form number, then go through and increment.
						// (r.gonet 03/07/2013) - PLID 55490 - We were counting deleted labs. Don't do that now, that's an unusal bug.
						_RecordsetPtr rsExistingSpecimens = CreateParamRecordset("SELECT Max(Specimen) AS Specimen FROM LabsT WHERE PatientID = {INT} and FormNumberTextID = {STRING} AND Deleted = 0",
							nPersonID, arLabs[i].strOrderNumber);
						if(!rsExistingSpecimens->eof) {
							CString strSpecimen = AdoFldString(rsExistingSpecimens, "Specimen","");
							// (j.kuziel 2014-05-02) - PLID 60396 - Specimen IDs can be alphanumeric, so only then will we increment it.
							int nSpecimenLength = strSpecimen.GetLength();
							if (nSpecimenLength == 1 && isalpha(strSpecimen.GetAt(0))) {
								bIsSpecimenAlpha = true;
								nSpecimen = (char)strSpecimen.GetAt(0);
								nSpecimen++;
							}
							else if (nSpecimenLength > 0 && nSpecimenLength <= 2 && IsNumeric(strSpecimen)) {
								bIsSpecimenAlpha = false;
								nSpecimen = atol(strSpecimen);
								nSpecimen++;

								if (nSpecimenLength == 2 && strSpecimen.GetAt(0) == '0') {
									bZeroPrefix = true;
								}
							}
						}
					}
					else {
						nSpecimen = (char)'A';
						bIsSpecimenAlpha = true;
					}

					if(nSpecimen != -1) {
						for(int j = 0; j < arLabs.GetSize(); j++) {
							if(arLabs[i].strOrderNumber == arLabs[j].strOrderNumber) {
								if(arLabs[j].strSpecimen.IsEmpty()) {

									if (bIsSpecimenAlpha) {
										if (isupper(nSpecimen) || islower(nSpecimen)) {
											arLabs[j].strSpecimen = (char)nSpecimen;
											nSpecimen++;
										}

									} else	{
										if (nSpecimen <= 99) {
											CString strSpecimen;
											if (bZeroPrefix) {
												strSpecimen.Format("%02d", nSpecimen);
											}
											else {
												strSpecimen.Format("%d", nSpecimen);
											}
											
											arLabs[j].strSpecimen = strSpecimen;
											nSpecimen++;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		CArray<long, long> arMailIDs;
		// (r.gonet 09/02/2014) - PLID 63221 - Remember the patients who had labs created for them
		std::set<long> setPatientsWithNewLabsCreated;

		//TES 4/1/2010 - PLID 37500 - Now go through each of the labs and create them.
		for(int i = 0; i < arLabs.GetSize(); i++) {
			LabIdentifier li = arLabs[i];

			//TES 12/1/2008 - PLID 32192 - Pull the OBR-25, Result Status field, use it in case any of our OBX's don't
			// have a status.
			CString strMasterStatus;
			GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", i+1, 25, 1, 1, strMasterStatus);

			//TES 9/18/2008 - PLID 21093 - INTERMEDIATE: Variables to hold the parsed data, we will pop them up in a message box
			// for now, rather than creating any actual records.
			//TES 10/14/2008 - PLID 31637 - We now keep a link between created results and their associated HL7MessageQueueT
			// records, which contain the entire message.  Therefore, if we wind up needing any information that we don't
			// currently parse, we'll be able to get it, so for now only parse the things we have a place in data for.
	//#pragma TODO("TES 10/8/2008 - PLID 21093 - Figure out what to do with those fields that we don't have any place in data for (the ones below plus result fields like Responsible Observer).")

			CString strTest, strProviderID, strProviderLast, strProviderFirst, strProviderMiddle, strProviderPersonID, strDateObserved;

			// (a.walling 2010-02-25 13:27) - PLID 37534 - Lab date performed
			// OBR-22
			_variant_t varDatePerformed = g_cvarNull;
			//TES 4/28/2011 - PLID 43426 - Date Received By Lab
			// OBR-14
			_variant_t varDateReceivedByLab = g_cvarNull;

			//TES 11/13/2014 - PLID 55674 - Set up our struct to remember all the information to commit this lab
			LabCommitInfo lci;
			lci.pAryParams = NULL;
			lci.pArPendingLabAudits = NULL;
			lci.pArPendingResultAudits = NULL;
			lci.bHL7IDLinkT_NeedAudit = false;
			lci.psaResults = NULL;
			lci.nTodoPriority = 1;
			aryLabCommits.Add(lci);

			CString strSqlBatch;
			//TES 11/13/2014 - PLID 55674 - Make this a pointer (we'll be using it outside this loop), store it in our struct
			CNxParamSqlArray *pAryParams = new CNxParamSqlArray;
			lci.pAryParams = pAryParams;
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @nLabID INT;");
			//TES 10/13/2008 - PLID 21093 - For auditing, we need a variable @nNewLabID, which will have the LabsT.ID iff
			// we newly create a LabsT record.  @nLabID will have it regardless.
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @nNewLabID INT;");
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nNewLabID = -1;");
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @nMailID INT;");
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nMailID = NULL");

			//TES 10/13/2008 - PLID 21093 - Keep an array of all the Lab-level things we want to audit (we can't actually audit 
			// them until we get the LabsT.ID)
			CArray<CPendingAuditInfo,CPendingAuditInfo&> arPendingLabAudits;

			//TES 10/13/2008 - PLID 21093 - Get the patient name for auditing.
			_RecordsetPtr rsPatientName = CreateParamRecordset("SELECT First, Middle, Last FROM PersonT WHERE ID = {INT}", 
					nPersonID);
			CString strFirst = AdoFldString(rsPatientName, "First");
			CString strMiddle = AdoFldString(rsPatientName, "Middle");
			CString strLast = AdoFldString(rsPatientName, "Last");
			rsPatientName->Close();
			CString strPersonName = strLast + ", " + strFirst + " " + strMiddle;

			//TES 2/23/2010 - PLID 37501 - Track some Receiving Lab information that may be stored in the OBX records.
			//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			ReceivingLabLocation  rllReceivingLabLocation = (ReceivingLabLocation)GetHL7SettingInt(nHL7GroupID, "ReceivingLabLocation");
			CString strCurrentLabCode, strCurrentLabName, strCurrentLabAddress1, strCurrentLabAddress2, strCurrentLabCity, strCurrentLabState, strCurrentLabZip;

			bool bHL7IDLinkT_NeedAudit = false;

			//TES 2/25/2013 - PLID 54876 - Try and map the Performing Lab based on the OBR-21 information.  If the field is required, then prompt the user, 
			// and fail if nothing ends up getting mapped.  Otherwise, only map if we can silently find a match.
			//TES 5/17/2013 - PLID 56286 - We can now read Performing Lab information out of OBX-15 instead of OBR-21, so skip if we'll be reading from OBX
			// (d.singleton 2013-10-31 16:47) - PLID 59181 - can now also look at obx23 so new setting is an enum,  changed code to support that
			long nPerformingLabID = -1;
			long nPerformingLabIDObr21 = GetHL7SettingInt(nHL7GroupID, "PerformingLab_OBR_21");
			if(nPerformingLabIDObr21 == (long)plcOBR21) {
				BOOL bRequirePerformingLab = GetHL7SettingBit(nHL7GroupID, "RequirePerformingLab");
				LogHL7Debug("The setting \"Require using X as the Performing Lab ID\" is set to \"OBR-21\". Attempting to match OBR-21 (Performing Lab) in the %s OBR with an existing Nextech lab location.", GetOrdinalFromInteger(i + 1));
				nPerformingLabID = GetPracticeLabLocation(nHL7GroupID, li.strPerformingLabID, li.strPerformingLabName, 
								li.strPerformingLabAddress, "", li.strPerformingLabCity, li.strPerformingLabState, li.strPerformingLabZip, 
								!bRequirePerformingLab, "Performing Lab");
				if(bRequirePerformingLab && nPerformingLabID == -1) {
					//TES 2/25/2013 - PLID 54876 - GetPracticeLabLocation() will already have notified them of the failure.
					// (r.gonet 05/01/2014) - PLID 61843 - Log an error
					LogHL7Error("Could not match the HL7 performing lab location with an existing Nextech lab location.");
					// (r.gonet 05/01/2014) - PLID 61843 - Log a possible resolution
					LogHL7Resolution("Ensure that the lab company actually sends a performing lab somewhere in the message. Some do not. Nextech has the ability to read the "
						"performing lab ID from OBX-15, OBR-21, or OBX-23, which can be configured on the Links->Receive Labs->Configure HL7 Settings->Labs->Advanced screen. "
						"Ensure that setting is set correctly.");
					return FALSE;
				}
			}

			if(li.nLabID == -1)
			{
				// (z.manning 2014-06-27 10:51) - PLID 62456 - Check and make sure this form number is not in use by another patient.
				_RecordsetPtr prsExistingFormNumber = CreateParamRecordset(
					"SELECT TOP 1 P.FullName \r\n"
					"FROM LabsT L \r\n"
					"INNER JOIN PersonT P ON P.ID = L.PatientID \r\n"
					"WHERE L.Deleted = 0 AND L.FormNumberTextID = {STRING} AND L.PatientID <> {INT} \r\n"
					, li.strOrderNumber, nPersonID);
				if (!prsExistingFormNumber->eof)
				{
					// (z.manning 2014-06-27 11:15) - PLID 62456 - There is already a lab for another patient with this form number
					// so clean up auditing, log the error, and do not try to create this lab.
					for (int i = 0; i<arPendingResultAudits.GetSize(); i++) {
						CArray<CPendingAuditInfo, CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo, CPendingAuditInfo&>*)arPendingResultAudits.GetAt(i);
						if (parAudits) {
							delete parAudits;
							parAudits = NULL;
						}
					}
					arPendingResultAudits.RemoveAll();

					CString strExistingPatName = AdoFldString(prsExistingFormNumber, "FullName", "");
					LogHL7Error("Failed to import lab '%s' for patient '%s' because this form number is used by an existing lab for another patient (%s)."
						, li.strOrderNumber, strPatientName, strExistingPatName);
					LogHL7Resolution("The lab company will need to re-send this lab result with a unique order number");
					return FALSE;
				}

				//TES 9/18/2008 - PLID 21093 - Couldn't link to a lab record, so we're going to create it.
				//TES 9/18/2008 - PLID 21093 - Get the test (dermatopathology, urine screening, whatever) out of OBR-4
				if(COMPONENT_NOT_FOUND == GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 4, 1, 2, strTest)) {
					//TES 9/18/2008 - PLID 21093 - Some lab companies (incorrectly) just send a single component, 
					// so try pulling that one.
					GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 4, 1, 1, strTest);
				}

				if(!li.strTestCodeDescription.IsEmpty()) {
					strTest = li.strTestCodeDescription;
				}
				//TES 5/11/2011 - PLID 43634 - Check the setting for whether to append the OBR-20.5 value to the name (Quest does this).
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				if(GetHL7SettingBit(nHL7GroupID, "AppendOBR20")) {
					CString strExtraName;
					GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 20, 1, 5, strExtraName);
					if(!strExtraName.IsEmpty()) {
						strTest += "\r\n" + strExtraName;
					}
				}

				// (r.gonet 03/07/2013) - PLID 55490 - To be explicit, get OBR-4.1 as the test code and OBR-4.2 as the test description.
				CString strTestCode;
				CString strTestDescription;
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 4, 1, 1, strTestCode);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 4, 1, 2, strTestDescription);

				/*//TES 9/18/2008 - PLID 21093 - Get the identity of the collector out of OBR-10
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 10, 1, 1, strCollectorID);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 10, 1, 2, strCollectorLast);

				//Relevant clinical information - OBR-13
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 13, 1, 1, strRelevantClinicalInformation);*/

				//Ordering Provider, OBR-16 
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 16, 1, 1, strProviderID);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 16, 1, 2, strProviderLast);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 16, 1, 3, strProviderFirst);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 16, 1, 4, strProviderMiddle);
				//TES 4/8/2010 - PLID 37527 - If the provider information is blank, just set it to NULL, the saving code down below
				// will translate that to the patient's MainPhysician.
				if(strProviderID.IsEmpty() && strProviderLast.IsEmpty() && strProviderFirst.IsEmpty() && strProviderMiddle.IsEmpty()) {
					strProviderPersonID = "NULL";
				}
				else {
					// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
					LogHL7Debug("Attempting to match OBR-16 (Ordering Provider) from the %s OBR with an existing Nextech provider.", GetOrdinalFromInteger(i + 1));
					//TES 10/21/2008 - PLID 31414 - Pass in our param array.
					// (j.jones 2010-05-13 10:43) - PLID 36527 - added a parameter indicating that the HL7IDLinkT needs audited					
					strProviderPersonID = GetNextechProvider(strProviderID, strProviderFirst, strProviderMiddle, strProviderLast, nHL7GroupID, &strSqlBatch, pAryParams, "Lab Result", "Ordering Provider", NULL, &bHL7IDLinkT_NeedAudit);
				}

				/*//Placer fields (custom), OBR-18 and OBR-19
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 18, 1, 1, strPlacerField1);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 19, 1, 1, strPlacerField2);

				//Filler fields (custom), OBR-20 and OBR-21
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 20, 1, 1, strFillerField1);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 21, 1, 1, strFillerField2);

				//Date requested, OBR-6
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", 1, 6, 1, 1, strDateRequested);
				if(!strDateRequested.IsEmpty()) {
					COleDateTime dt;
					if(ParseHL7Date(strDateRequested, dt)) {
						strDateRequested = FormatDateTimeForInterface(dt, 0, dtoDateTime);
					}
				}*/
				
				//Date of observation, OBR-7
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 7, 1, 1, strDateObserved);
				COleDateTime dtObserved;
				_variant_t varObserved = g_cvarNull;
				dtObserved.SetStatus(COleDateTime::invalid);
				if(!strDateObserved.IsEmpty()) {
					// (a.walling 2010-02-25 14:17) - PLID 37543 - Include time (even if we don't end up using it in the interface, better to have more info than less)
					if(ParseHL7DateTime(strDateObserved, dtObserved)) {
						strDateObserved = FormatDateTimeForInterface(dtObserved, 0, dtoDateTime);
						varObserved = COleVariant(dtObserved);
					}
				}

				//TES 5/3/2011 - PLID 43536 - Order Status, ORC-5 (note that there's only one ORC segment per message).
				CString strOrderStatusFlag;
				GetHL7MessageComponent(strMessage, nHL7GroupID, "ORC", 1, 5, 1, 1, strOrderStatusFlag);
				CString strOrderStatusDescription = HL7GetLabOrderStatusDescription(GetRemoteData(), strOrderStatusFlag);

				//Sending facility, MSH-4
				CString strSendingFacilityID;
				if(COMPONENT_NOT_FOUND == GetHL7MessageComponent(strMessage, nHL7GroupID, "MSH", 1, 4, 1, 2, strSendingFacilityID)) {
					//TES 9/18/2008 - PLID 21093 - Some lab companies (incorrectly) just send a single component, 
					// so try pulling that one.
					GetHL7MessageComponent(strMessage, nHL7GroupID, "MSH", 1, 4, 1, 1, strSendingFacilityID);
				}
				long nLabLocationID = -1;
				//TES 10/8/2008 - PLID 21093 - Made a new function for tying HL7 Location Codes to Nextech Lab-type LocationsT records.
				_variant_t varLabLocationID = g_cvarNull;
				//TES 4/29/2011 - PLID 43424 - Get the Lab Location, assuming we're not pulling it from OBX
				if(rllReceivingLabLocation != rllOBX23) {
					//TES 4/29/2011 - PLID 43424 - First, try OBR-21, if that's what we're set to do
					if(rllReceivingLabLocation == rllOBR21) {
						//TES 2/25/2013 - PLID 54876 - If we already mapped the OBR-21 to a PerformingLabID, then use it here, otherwise call
						// GetPracticeLabLocation like before.
						if(nPerformingLabID == -1) {
							// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
							LogHL7Debug("The setting \"Use the Receiving Lab information from\" is set to \"OBR-21\". Attempting to match OBR-21 in the %s OBR with an existing Nextech lab location.", GetOrdinalFromInteger(i+1));
							nLabLocationID = GetPracticeLabLocation(nHL7GroupID, li.strPerformingLabID, li.strPerformingLabName, 
								li.strPerformingLabAddress, "", li.strPerformingLabCity, li.strPerformingLabState, li.strPerformingLabZip,
								FALSE, "Receiving Lab");
						}
						else {
							nLabLocationID = nPerformingLabID;
						}
					}
					//TES 4/29/2011 - PLID 43424 - Now check MSH-4, if either that's our setting, or if the OBR-21 field was empty.
					if(rllReceivingLabLocation == rllMSH4 || nLabLocationID == -1) {
						// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
						LogHL7Debug("The setting \"Use the Receiving Lab information from\" is set to \"MSH-4\" or Nextech has failed to find a match with the setting set to \"OBR-21\". Attempting to match MSH-4 with an existing Nextech lab location.");
						GetHL7LabLocationID(Message, strSendingFacilityID, nLabLocationID);
					}
					if(nLabLocationID != -1) {
						varLabLocationID = nLabLocationID;
					}
					else {
						//TES 12/3/2008 - PLID 32301 - If there's no location, we should just fail.  We used to say that we 
						// would "use the system defaults", but a.) we didn't, it just failed, and b.) the "system default"
						// is to arbitrarily select a location, which is ok on a dialog where they can see and change it,
						// but I think unacceptable for a situation like this where the user doesn't see which arbitrary
						// selection is being used.

						// (j.jones 2010-04-08 09:03) - PLID 38101 - if we didn't process the result,
						// clear any audits we may have loaded into memory
						for(int i=0; i<arPendingResultAudits.GetSize(); i++) {
							CArray<CPendingAuditInfo,CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo,CPendingAuditInfo&>*)arPendingResultAudits.GetAt(i);
							if(parAudits) {
								delete parAudits;
								parAudits = NULL;
							}
						}
						arPendingResultAudits.RemoveAll();

						// (r.gonet 05/01/2014) - PLID 61843 - Log an error
						LogHL7Error("The HL7 receiving lab location matched no existing Nextech lab location. Nextech requires a lab location in order to set the receiving lab field "
							"on the newly created lab.");
						LogHL7Resolution("Please ensure the HL7 setting \"Use the Receiving Lab information from\" in Links->Receive Labs->Configure HL7 Settings->Labs->Advanced is correct and that the "
							"lab is sending a value in that field.");
						return FALSE;
					}
				}
				
				//TES 10/10/2008 - PLID 21093 - Determine which "procedure" (ladder, basically) to assign to this Lab.
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				// (r.gonet 03/07/2013) - PLID 55489 - It is now possible to have the lab procedure already assigned (reflexes will do this)
				if(li.nLabProcedureID == -1) {
					long nLabProcedureID = GetHL7SettingInt(nHL7GroupID, "DefaultLabProcedureID");
					if(nLabProcedureID == -1) {
						//TES 10/10/2008 - PLID 21093 - They don't have a default, we'll have to prompt them.
						CSingleSelectDlg dlg(NULL);
						//TES 10/13/2008 - PLID 21093 - Pass in true to force them to select a record.
						// (z.manning 2011-04-28 17:09) - PLID 43495 - Filter out deleted lab procedures
						if(IDOK == dlg.Open("LabProceduresT", "Inactive = 0", "ID", "Name", 
							"Please select a Lab Procedure for this newly created Lab record.", true)) {
								nLabProcedureID = dlg.GetSelectedID();
						}
						else {

							// (j.jones 2010-04-08 09:03) - PLID 38101 - if we didn't process the result,
							// clear any audits we may have loaded into memory
							for(int i=0; i<arPendingResultAudits.GetSize(); i++) {
								CArray<CPendingAuditInfo,CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo,CPendingAuditInfo&>*)arPendingResultAudits.GetAt(i);
								if(parAudits) {
									delete parAudits;
									parAudits = NULL;
								}
							}
							arPendingResultAudits.RemoveAll();

							// (r.gonet 05/01/2014) - PLID 61843 - Log an error
							LogHL7Error("Lab procedure selection has been cancelled. Nextech cannot import the HL7 lab order without a lab procedure.");
							LogHL7Resolution("To avoid having to select a lab procedure, set the Default Lab Procedure in Links->Receive Labs->Configure HL7 Settings->Labs");
							return FALSE;
						}
					}
					// (r.gonet 03/07/2013) - PLID 55490 - Set the lab's procedure id
					li.nLabProcedureID = nLabProcedureID;
				} else {
					// (r.gonet 03/07/2013) - PLID 55489 - We already have a lab procedure, probably because we are a reflexed test.
				}

				// (r.gonet 03/07/2013) - PLID 55489 - It is now possible to have the lab procedure type already set (in the case of reflexes)
				if(li.eLabType == ltInvalid) {
					// (z.manning 2009-06-15 11:07) - PLID 34623 - We need to get the lab procedure type
					LabType eLabType = GetHL7LabProcedureType(nHL7GroupID);
					if(eLabType == ltInvalid) {
						// (z.manning 2009-06-15 11:08) - PLID 34623 - We didn't get the lab type (probably
						// because they don't have a default procedure) so we need to load it from data.
						_RecordsetPtr prsLabType = CreateParamRecordset(
							"SELECT Type FROM LabProceduresT WHERE ID = {INT}"
							, li.nLabProcedureID);
						if(!prsLabType->eof) {
							eLabType = (LabType)AdoFldByte(prsLabType->GetFields(), "Type");
						}
						else {
							// (z.manning 2009-06-15 11:10) - 34623 - This shouldn't be possible, but just in case
							eLabType = ltBiopsy;
						}
					}
					//TES 8/2/2010 - PLID 38776 - We need to remember the lab type we're using.
					li.eLabType = eLabType;
				} else {
					// (r.gonet 03/07/2013) - PLID 55489 - We already have a lab type, probably because we are a reflexed test.
				}
				arLabs.SetAt(i, li);

				//TES 4/5/2010 - PLID 38040 - Parse the anatomic location, if we have one.
				AnatomySide as = asNone;
				long nAnatomicQualifierID = -1;
				long nAnatomicLocationID = -1;
				CString strAnatomicQualifier, strAnatomicLocationName;
				//TES 4/5/2010 - PLID 38040 - Don't parse this out unless this is a biopsy; otherwise they won't be able to see it anyway.
				if(li.eLabType == ltBiopsy && !li.strAnatomicLocation.IsEmpty()) {
					//TES 6/1/2010 - PLID 38066 - This is configurable now, so pass in nHL7GroupID
					ParseAnatomicLocation(GetRemoteData(), li.strAnatomicLocation, nHL7GroupID, as, nAnatomicQualifierID, strAnatomicQualifier, nAnatomicLocationID, strAnatomicLocationName);
				}

				// (d.singleton 2013-06-03 17:03) - PLID 57061 - Update HL7 lab messages to support latest MU requirements.
				HL7_SPMFields SPM;
				ParseSPMSegment(Message.strMessage, Message.nHL7GroupID, SPM);

				// (d.singleton 2013-08-08 16:44) - PLID 57598 - support import of TQ1 segment ( only starttime and endtime ) for lab results
				HL7_TQ1Fields TQ1;
				ParseTQ1Segment(Message.strMessage, Message.nHL7GroupID, TQ1);

				// (r.gonet 2015-11-17 23:51) - PLID 67590 - If we still have no location for this lab, ie it is a new lab order that we'll be creating,
				// use the currently logged in location since they are importing from there, that would be a better bet than the gen 2 location or 
				// the default patient location. Plus it is never null.
				if (li.nLocationID == -1) {
					// If the lab has no location, like if it is totally unsolicited, use the current location
					li.nLocationID = GetCurrentLocationID();
				}

				CString strLocationName;
				// (r.gonet 2015-11-17 23:51) - PLID 67590 - Now, since we may either be using an existing lab's location or the current location,
				// get the name of the location to audit with when we go to create a new lab order.
				_RecordsetPtr rsLocation = CreateParamRecordset(GetRemoteDataSnapshot(),
					"SELECT LocationsT.Name "
					"FROM LocationsT "
					"WHERE LocationsT.ID = {INT}", li.nLocationID);
				if (!rsLocation->eof) {
					strLocationName = AdoFldString(rsLocation->Fields, "Name", "");
				}

				//TES 10/13/2008 - PLID 21093 - OK, now audit that we created the lab, and all the fields that we're filling in.
				CString strLabDescription;
				// (a.walling 2010-01-15 16:37) - PLID 36905 - Include the test (format consistent with auditing in the lab requistion dialog)
				//TES 4/5/2010 - PLID 38040 - If this is a biopsy, then format using the anatomic location instead (note, just like in 
				// the lab requisition dialog, this only uses the location, not the side or qualifier).
				if(li.eLabType == ltBiopsy) {
					strLabDescription.Format("New Lab Created - %s - %s", li.strOrderNumber, strAnatomicLocationName);
				}
				else {
					strLabDescription.Format("New Lab Created - %s - %s", li.strOrderNumber, strTest);
				}
				arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabNew, -1, "", strLabDescription, aepMedium, aetCreated));
				arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabFormNum, -1, "", li.strOrderNumber, aepMedium, aetCreated));
				if(li.eLabType == ltBiopsy) {
					//TES 4/5/2010 - PLID 38040 - Audit the anatomic location information.
					if(!strAnatomicLocationName.IsEmpty()) {
						arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabAnatomicLocation, -1, "", strAnatomicLocationName, aepMedium, aetCreated));
					}
					if(as != asNone) {
						arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabAnatomicSide, -1, "", as == asLeft ? "Left" : "Right", aepMedium, aetCreated));
					}
					if(!strAnatomicQualifier.IsEmpty()) {
						arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabAnatomicLocationQualifier, -1, "", strAnatomicQualifier, aepMedium, aetCreated));
					}
				}
				else {
					// (a.walling 2010-01-15 16:37) - PLID 36905 - Audit the test
					if (!strTest.IsEmpty()) {
						arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabToBeOrdered, -1, "", strTest, aepMedium, aetCreated));
					}
				}
				// (r.gonet 2015-11-17 23:51) - PLID 67590 - Use the name of the location we are creating the new lab under, which may be either
				// the name of a location of a related, existing lab order or the current location.
				arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabLocation, -1, "", strLocationName, aepMedium, aetCreated));
				if(dtObserved.GetStatus() == COleDateTime::valid) {
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabBiopsyDate, -1, "", FormatDateTimeForInterface(dtObserved), aepMedium, aetCreated));
				}
				arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabFirst, -1, "", strFirst, aepMedium, aetCreated));
				arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabMiddle, -1, "", strMiddle, aepMedium, aetCreated));
				arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabLast, -1, "", strLast, aepMedium, aetCreated));

				//TES 5/3/2011 - PLID 43536 - Audit the Order Status, if any
				if(!strOrderStatusDescription.IsEmpty()) {
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabOrderStatus, -1, "", strOrderStatusDescription, aepMedium, aetCreated));
				}

				//TES 5/5/2011 - PLID 43575 - Audit the Filler Order Number, if any
				if(!strFillerOrderNumber.IsEmpty()) {
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabFillerOrderNumber, -1, "", strFillerOrderNumber, aepMedium, aetCreated));
				}

				//TES 5/11/2011 - PLID 43658 - Audit the report comments, if any
				if(!strReportComment.IsEmpty()) {
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabClinicalData, -1, "", strReportComment, aepMedium, aetCreated));
				}

				// (z.manning 2008-10-30 11:40) - PLID 31864 - Set labs created here as type biopsy
				// (z.manning 2009-06-15 11:11) - PLID 34623 - I must've been been really dumb 8 months ago.
				// We now use the type of the lab procedure that they select instead of always using biopsy as the type.
				//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
				//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
				// (a.walling 2010-01-15 16:38) - PLID 36905 - Include the test (ToBeOrdered)
				//TES 4/5/2010 - PLID 38040 - Added AnatomyID, AnatomySide, AnatomyQualifierID
				//TES 4/5/2010 - PLID 37500 - Added Specimen
				//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
				//TES 5/3/2011 - PLID 43536 - Add in the Order Status
				//TES 5/5/2011 - PLID 43575 - Added Filler Order Number
				//TES 5/11/2011 - PLID 43658 - Added "Report Comment", stored in the ClinicalData (Comments) field.
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @nOrderStatusID INT");
				if(!strOrderStatusDescription.IsEmpty()) {
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nOrderStatusID = ID FROM LabOrderStatusT "
						"WHERE Description = {STRING}", strOrderStatusDescription);
				}
				//TES 5/3/2011 - PLID 43536 - The OrderStatusID needs to be the same for all labs with the same patient and order number.
				// If we don't have a description, and there's already a lab with this patient and order number, use that lab's order status
				// for the new lab.
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "IF @nOrderStatusID Is Null BEGIN \r\n"
					"SELECT TOP 1 @nOrderStatusID = OrderStatusID FROM LabsT WHERE LabsT.FormNumberTextID = {STRING} AND LabsT.PatientID = {INT} AND LabsT.Deleted = 0\r\n"
					"END", li.strOrderNumber, nPersonID);

				// (d.singleton 2013-11-25 17:34) - PLID 59379 - need to expand the funtionality of reflex tests				
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @nParentResultID INT");
				if(li.HasParentResult()) {
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nParentResultID = ResultID FROM LabResultsT "
						"WHERE LOINC = {STRING} AND Name = {STRING} AND ObservationSubID = {STRING}", li.strParentObservationID, 
						li.strParentObservationIDText, li.strParentResultSubID);	
				}
					
				if(rllReceivingLabLocation == rllOBX23) {
					//TES 2/23/2010 - PLID 37501 - If we're pulling the location from the OBX segments, then we don't know what it is yet, so
					// just put a placeholder in there that we'll replace before executing the batch.
					//TES 2/25/2013 - PLID 54876 - Added PerformingLabID
					//TES 2/26/2013 - PLID 55321 - Added MedicalDirector
					// (r.gonet 03/07/2013) - PLID 55490 - Need to set the LOINC values from the universal service identifier
					//TES 5/17/2013 - PLID 56286 - Moved PerformingLabID to LabResultsT
					// (d.singleton 2013-07-02 15:12) - PLID 57061 - Need to be able to parse and import SPM segments in lab result ORU messages
					// (d.singleton 2013-11-25 17:34) - PLID 59379 - need to expand the funtionality of reflex tests
					// (r.gonet 2015-11-17 23:51) - PLID 67590 - Use the lab's assigned location rather than the current logged in location for the ordering location.
					// This may either be the location of a lab order on the same requisition (which seems more accurate to use) or the name of the logged in location
					// if there is no related, existing lab order.
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "INSERT INTO LabsT (FormNumberTextID, PatientID, LocationID, BiopsyDate, "
						"InputDate, LabLocationID, PatientFirst, PatientMiddle, PatientLast, Deleted, LabProcedureID, Type, ToBeOrdered, LOINC_Code, LOINC_Description, "
						"AnatomyID, AnatomySide, AnatomyQualifierID, Specimen, OrderStatusID, FillerOrderNumber, ClinicalData, "
						"MedicalDirector, SpecimenID, SpecimenIdText, SpecimenStartTime, SpecimenEndTime, SpecimenRejectReason, SpecimenCondition, "
						"ServiceStartTime, ServiceEndTime, ParentResultID) "
						"VALUES ({STRING}, {INT}, {INT}, {VT_DATE}, getdate(), LAB_LOCATION_PLACEHOLDER, {STRING}, {STRING}, {STRING}, 0, {INT}, {INT}, {STRING}, {STRING}, {STRING}, "
						"{VT_I4}, {INT}, {VT_I4}, {STRING}, @nOrderStatusID, {STRING}, {STRING}, "
						"{VT_BSTR}, {STRING}, {STRING}, {VT_DATE}, {VT_DATE}, {STRING}, {STRING}, {VT_DATE}, {VT_DATE}, @nParentResultID) ",
						li.strOrderNumber, nPersonID, li.nLocationID, 
						varObserved, strFirst, strMiddle, strLast, li.nLabProcedureID, li.eLabType, strTest, strTestCode, strTestDescription,
						nAnatomicLocationID == -1 ? g_cvarNull : _variant_t(nAnatomicLocationID), as, nAnatomicQualifierID == -1 ? g_cvarNull : _variant_t(nAnatomicQualifierID),
						li.strSpecimen, strFillerOrderNumber, strReportComment,
						li.strMedicalDirector.IsEmpty() ? g_cvarNull : _bstr_t(li.strMedicalDirector), SPM.strIdentifier, SPM.strIDText, SPM.varStartTime, SPM.varEndTime,
						SPM.strRejectReason, SPM.strCondition, TQ1.varStartTime, TQ1.varEndTime);
				}
				else {
					//TES 2/25/2013 - PLID 54876 - Added PerformingLabID
					//TES 2/26/2013 - PLID 55321 - Added MedicalDirector
					// (r.gonet 03/07/2013) - PLID 55490 - Need to set the LOINC values from the universal service identifier
					//TES 5/17/2013 - PLID 56286 - Moved PerformingLabID to LabResultsT
					// (d.singleton 2013-07-02 15:12) - PLID 57061 - Need to be able to parse and import SPM segments in lab result ORU messages
					// (d.singleton 2013-11-25 17:34) - PLID 59379 - need to expand the funtionality of reflex tests
					// (r.gonet 2015-11-17 23:51) - PLID 67590 - Use the lab's assigned location rather than the current logged in location for the ordering location.
					// This may either be the location of a lab order on the same requisition (which seems more accurate to use) or the name of the logged in location
					// if there is no related, existing lab order.
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "INSERT INTO LabsT (FormNumberTextID, PatientID, LocationID, BiopsyDate, "
						"InputDate, LabLocationID, PatientFirst, PatientMiddle, PatientLast, Deleted, LabProcedureID, Type, ToBeOrdered, LOINC_Code, LOINC_Description, "
						"AnatomyID, AnatomySide, AnatomyQualifierID, Specimen, OrderStatusID, FillerOrderNumber, ClinicalData, "
						"MedicalDirector, SpecimenID, SpecimenIdText, SpecimenStartTime, SpecimenEndTime, SpecimenRejectReason, SpecimenCondition, "
						"ServiceStartTime, ServiceEndTime, ParentResultID) "
						"VALUES ({STRING}, {INT}, {INT}, {VT_DATE}, getdate(), {VT_I4}, {STRING}, {STRING}, {STRING}, 0, {INT}, {INT}, {STRING}, {STRING}, {STRING}, "
						"{VT_I4}, {INT}, {VT_I4}, {STRING}, @nOrderStatusID, {STRING}, {STRING}, "
						"{VT_BSTR}, {STRING}, {STRING}, {VT_DATE}, {VT_DATE}, {STRING}, {STRING}, {VT_DATE}, {VT_DATE}, @nParentResultID) ",
						li.strOrderNumber, nPersonID, li.nLocationID, 
						varObserved, varLabLocationID, strFirst, strMiddle, strLast, li.nLabProcedureID, li.eLabType, strTest, strTestCode, strTestDescription,
						nAnatomicLocationID == -1 ? g_cvarNull : _variant_t(nAnatomicLocationID), as, nAnatomicQualifierID == -1 ? g_cvarNull : _variant_t(nAnatomicQualifierID),
						li.strSpecimen, strFillerOrderNumber, strReportComment,
						li.strMedicalDirector.IsEmpty() ? g_cvarNull : _bstr_t(li.strMedicalDirector), SPM.strIdentifier, SPM.strIDText, SPM.varStartTime, SPM.varEndTime,
						SPM.strRejectReason, SPM.strCondition, TQ1.varStartTime, TQ1.varEndTime);
				}
				//TES 5/3/2011 - PLID 43536 - All labs with the same form number and patient need to have the same order status
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET OrderStatusID = @nOrderStatusID "
					"WHERE FormNumberTextID = {STRING} AND PatientID = {INT} AND Deleted = 0", li.strOrderNumber, nPersonID);
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nLabID = SCOPE_IDENTITY();");
				if(strProviderPersonID == "NULL") {
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @nProviderID INT;");
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nProviderID = COALESCE(MainPhysician,-1) FROM PatientsT WHERE PersonID = {INT}", nPersonID);
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "IF @nProviderID <> -1\r\n"
						"BEGIN\r\n"
						"INSERT INTO LabMultiProviderT (LabID, ProviderID) VALUES (@nLabID, @nProviderID)\r\n"
						"END");
				}
				else {
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, FormatString("INSERT INTO LabMultiProviderT (LabID, ProviderID) "
						"VALUES (@nLabID, %s)", strProviderPersonID));
				}

				//TES 10/10/2008 - PLID 21093 - Add all the steps for the procedure we're using.
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "INSERT INTO LabStepsT (LabProcedureStepID, LabID, "
					" StepOrder, Name) "
					" SELECT StepID, @nLabID, StepOrder, LabProcedureStepsT.Name "
					" FROM LabProcedureStepsT "
					" LEFT JOIN LabProceduresT ON LabProcedureStepsT.LabProcedureID = LabProceduresT.ID "
					" WHERE LabProceduresT.ID = {INT} AND LabProcedureStepsT.Inactive = 0; ", 
					li.nLabProcedureID);

				//TES 10/13/2008 - PLID 21093 - This is new, so update the @nNewLabID variable.
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nNewLabID = @nLabID");
				
			}
			else {
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "SELECT @nLabID = {INT}", li.nLabID);
				
				//TES 7/16/2010 - PLID 39642 - For existing labs, we want to update the BiopsyDate to include the time from the incoming
				// message if a.) it doesn't have a time already, and b.) the date is either null, or the same as the incoming time.

				//Date of observation, OBR-7
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 7, 1, 1, strDateObserved);
				COleDateTime dtObserved;
				_variant_t varObserved = g_cvarNull;
				dtObserved.SetStatus(COleDateTime::invalid);
				if(!strDateObserved.IsEmpty()) {
					// (a.walling 2010-02-25 14:17) - PLID 37543 - Include time (even if we don't end up using it in the interface, better to have more info than less)
					if(!ParseHL7DateTime(strDateObserved, dtObserved)) {
						dtObserved.SetStatus(COleDateTime::invalid);
					}
				}

				//TES 7/16/2010 - PLID 39642 - Is this a date that includes a valid time?
				// (If they take a biopsy at the stroke of midnight, they're out of luck, but if they do that, they're probably
				// a mad scientist anyway, and don't deserve a properly functioning EMR system.)
				if(dtObserved.GetStatus() == COleDateTime::valid && 
					(dtObserved.GetHour() != 0 || dtObserved.GetMinute() != 0 || dtObserved.GetSecond() != 0)) {
					COleDateTime dtObservedDate;
					dtObservedDate.SetDate(dtObserved.GetYear(), dtObserved.GetMonth(), dtObserved.GetDay());
					if(li.dtExistingBiopsyDate == dtNull || li.dtExistingBiopsyDate == dtObservedDate) {
						AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET BiopsyDate = {OLEDATETIME} "
							"WHERE ID = @nLabID", dtObserved);
						//TES 7/16/2010 - PLID 39642 - Audit the change.
						CString strOld;
						if(li.dtExistingBiopsyDate == dtNull) {
							strOld = "";
						}
						else {
							strOld = FormatDateTimeForInterface(li.dtExistingBiopsyDate);
						}
						CString strNew = FormatDateTimeForInterface(dtObserved);
						arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabBiopsyDate, li.nLabID, strOld, strNew, aepMedium, aetCreated));

					}
				}

				//TES 5/3/2011 - PLID 43536 - Order Status, ORC-5 (note that there's only one ORC segment per message).
				CString strOrderStatusFlag;
				GetHL7MessageComponent(strMessage, nHL7GroupID, "ORC", 1, 5, 1, 1, strOrderStatusFlag);
				CString strOrderStatusDescription = HL7GetLabOrderStatusDescription(GetRemoteData(), strOrderStatusFlag);
				//TES 5/3/2011 - PLID 43536 - Has the status changed (to something non-empty)?
				if(strOrderStatusDescription != li.strExistingOrderStatus && !strOrderStatusDescription.IsEmpty()) {
					//TES 5/3/2011 - PLID 43536 - Yes, update the record.
					//TES 5/3/2011 - PLID 43536 - NOTE: Make sure we update all labs for the current order number and pateint
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET OrderStatusID = "
						"(SELECT ID FROM LabOrderStatusT WHERE Description = {STRING}) "
						"WHERE LabsT.FormNumberTextID = (SELECT FormNumberTextID FROM LabsT WHERE ID = @nLabID) "
						"AND LabsT.PatientID = (SELECT PatientID FROM LabsT WHERE ID = @nLabID) "
						"AND LabsT.Deleted = 0", strOrderStatusDescription);

					//TES 5/3/2011 - PLID 43536 - And audit the change.
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabOrderStatus, li.nLabID, 
						li.strOrderNumber + " - " + li.strSpecimen + ": " + li.strExistingOrderStatus, strOrderStatusDescription, 
						aepMedium, aetCreated));
				}

				//TES 5/5/2011 - PLID 43575 - Has the Filler Order Number changed (to something non-empty?
				if(strFillerOrderNumber != li.strExistingFillerOrderNumber && !strFillerOrderNumber.IsEmpty()) {
					//TES 5/5/2011 - PLID 43575 - If the existing Filler Order Number isn't empty, that's very suspicious!  Let's warn them.
					if(!li.strExistingFillerOrderNumber.IsEmpty()) {
						//TES 5/5/2011 - PLID 43575 - Just in case we do find ourselves in a position someday where the Filler Order Numbers
						// are unavoidably variable, let's check a hidden ConfigRT setting that will prevent this warning.
						if(!GetRemotePropertyInt("HL7_AllowMismatchedFillerOrderNumbers", 0, 0, "<None>", false)) {
							// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
							LogHL7Warning("Filler order numbers don't match for the %s OBR. HL7 Message's filler order number is \"%s\" and the "
								"existing lab's filler order number is \"%s\". Ordinarily, this may indicate a problem with the result being attached to the wrong lab.",
								GetOrdinalFromInteger(nLab), strFillerOrderNumber, li.strExistingFillerOrderNumber);
							if(IDYES == MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "WARNING: The result message you are importing has an order number, assigned by "
								"the lab, which is different from the order number on the lab record to which this result is being attached.  This may lead to "
								"the result information being attached to an incorrect lab record!  It is highly recommended that you cancel this import "
								"and contact NexTech Technical Support at 888-417-8464.\r\n"
								"\r\n"
								"Existing Order Number: %s\r\n"
								"New Order Number: %s\r\n"
								"\r\n"
								"Would you like to cancel this import?", li.strExistingFillerOrderNumber, strFillerOrderNumber)) {
									// (r.gonet 05/01/2014) - PLID 61843 - Log an error
									LogHL7Error("The HL7 lab result import has been cancelled due to the filler order numbers mismatching.");
									LogHL7Resolution("Check with the lab company and have them ensure that this HL7 message has no mistakes with the patient ID, demographics, or order number. "
										"This error can also occur if the wrong result was imported to the Nextech lab order previously.");
									return FALSE;
							} else {
								// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
								LogHL7Warning("User continued the HL7 lab result import even though the filler order numbers mismatch.");
							}
						}
					}
					//TES 5/5/2011 - PLID 43575 - They said OK, so update it.
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET FillerOrderNumber = {STRING} WHERE ID = @nLabID",
						strFillerOrderNumber);
					//TES 5/5/2011 - PLID 43575 - And audit
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabFillerOrderNumber, li.nLabID, li.strExistingFillerOrderNumber, strFillerOrderNumber, aepMedium, aetCreated));					
				}

				//TES 5/11/2011 - PLID 43658 - Do we have a Report Comment to add?
				if(!strReportComment.IsEmpty()) {
					//TES 5/11/2011 - PLID 43658 - Append it to the existing comment.
					CString strNewComments;
					if(!li.strExistingComments.IsEmpty()) {
						strNewComments = li.strExistingComments + "\r\n";
					}
					strNewComments += strReportComment;

					//TES 5/11/2011 - PLID 43658 - Update
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET ClinicalData = {STRING} WHERE ID = @nLabID",
						strNewComments);
					//TES 5/11/2011 - PLID 43658 - And audit
					arPendingLabAudits.Add(CPendingAuditInfo(nPersonID, strPersonName, aeiPatientLabClinicalData, li.nLabID, li.strExistingComments, strNewComments, aepMedium, aetCreated));
				}

				//TES 2/26/2013 - PLID 55321 - Similarly with the MedicalDirector
				if(!li.strMedicalDirector.IsEmpty()) {
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET MedicalDirector = {STRING} "
						"WHERE ID = @nLabID AND MedicalDirector Is Null", li.strMedicalDirector);
				}

				// (d.singleton 2013-07-02 15:12) - PLID 57061 - Need to be able to parse and import SPM segments in lab result ORU messages
				HL7_SPMFields SPM;
				ParseSPMSegment(Message.strMessage, Message.nHL7GroupID, SPM);
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET SpecimenID = {STRING}, SpecimenIdText = {STRING}, SpecimenStartTime = {VT_DATE}, "
					"SpecimenEndTime = {VT_DATE}, SpecimenRejectReason = {STRING}, SpecimenCondition = {STRING} "
					"WHERE ID = @nLabID", SPM.strIdentifier, SPM.strIDText, SPM.varStartTime, SPM.varEndTime, SPM.strRejectReason, SPM.strCondition);

				// (d.singleton 2013-07-16 16:43) - PLID 57598 - support import of TQ1 segment ( only starttime and endtime ) for lab results
				HL7_TQ1Fields TQ1;
				ParseTQ1Segment(Message.strMessage, Message.nHL7GroupID, TQ1);
				AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET ServiceStartTime = {VT_DATE}, ServiceEndTime = {VT_DATE} "
					"WHERE ID = @nLabID", TQ1.varStartTime, TQ1.varEndTime);

			}

			//TES 7/30/2010 - PLID 39908 - Pull the value in OBR-24, we may use it for result processing.
			CString strObr24Value;
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 24, 1, 1, strObr24Value);

			//TES 9/18/2008 - PLID 21093 - Now, we need to go through the OBX or NTE segments, to get the actual results.
			
			//TES 10/8/2008 - PLID 31646 - Keep track of results for putting in notes of ToDo Alarm
			CStringArray saResults;
			//TES 8/6/2013 - PLID 51147 - Load the default todo for labs, we will override it if we find any results with abnormal flags set
			int nTodoPriority = GetRemotePropertyInt("Lab_DefaultTodoPriority", 1, 0, "<None>");
			BOOL bFlagFound = FALSE;

			//TES 10/13/2008 - PLID 21093 - We need to track everything we want to audit for each of the results, so store
			// that in an array of arrays of audits.  Also, keep track in our query of each new LabResultsT.ID along with
			// that record's index in our array of arrays of audits.
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @NewLabResultsT TABLE ( \r\n"
				"	LabResultID int, \r\n"
				"	ArrayIndex int) \r\n");	
			//TES 9/10/2013 - PLID 54288 - We also need to keep track of the current ResultID
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @CurrentResultID INT \r\n");
			//TES 11/05/2013 - PLID 59207 - For some reason this was never declared here, only in NxServer. Fixing under 59207 
			// since I came across it there.
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "DECLARE @LinkedLabID INT \r\n");

			//TES 10/14/2008 - PLID 21093 - Track the information for the result we're currently processing.
			HL7LabResultInfo hlriCurrentResult;
			hlriCurrentResult.strName = li.strLabName;

			// (a.walling 2010-02-25 13:28) - PLID 37534 - Although Date Performed is part of OBR which seems more Lab-related, Date Performed
			// seems more conceptually related to results. Although technically it seems possible to get multiple OBR for a single lab, we don't
			// support it now, and Tom says we would probably treat them as separate labs anyway. Regardless, Date Performed will be recorded
			// in LabResultsT for simplicity.
			//Date Performed, OBR-22
			{
				CString strDatePerformed;
				COleDateTime dtDatePerformed;
				dtDatePerformed.SetStatus(COleDateTime::invalid);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 22, 1, 1, strDatePerformed);

				if(!strDatePerformed.IsEmpty()) {
					if(ParseHL7DateTime(strDatePerformed, dtDatePerformed)) {
						strDatePerformed = FormatDateTimeForInterface(dtDatePerformed, 0, dtoDateTime);
						varDatePerformed = COleVariant(dtDatePerformed);
					}
				}
			}
						
			// (a.walling 2010-02-25 13:41) - PLID 37534 - Date Performed
			hlriCurrentResult.varDatePerformed = varDatePerformed;
			// (d.singleton 2013-11-26 12:38) - PLID 59836 - need to add the "date the order was rejected" to the requisition tab if it was rejected.  this comes from the obr 22 value and only if obr 25 is x.
			CString strResultStatus;
			GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 25, 1, 1, strResultStatus);
			if(strResultStatus.CompareNoCase("X") == 0) {
				//ok this has been rejected,  need to specify in the order tab the date.  use comments section as this is a fringe case
				//our date is already good to go so lets use it.
				if(varDatePerformed.vt != VT_NULL) {
					CString strStatusChangeDate = "Date Order Status Changed: " + AsString(varDatePerformed);
					AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE LabsT SET ClinicalData = {STRING} WHERE ID = @nLabID; \r\n", strStatusChangeDate);
				}
			}

			//TES 4/28/2011 - PLID 43426 - Date Received By Lab, same logic as Date Performed.  OBR-14
			{
				CString strDateReceivedByLab;
				COleDateTime dtReceivedByLab;
				dtReceivedByLab.SetStatus(COleDateTime::invalid);
				GetHL7MessageComponent(strMessage, nHL7GroupID, "OBR", i+1, 14, 1, 1, strDateReceivedByLab);

				if(!strDateReceivedByLab.IsEmpty()) {
					if(ParseHL7DateTime(strDateReceivedByLab, dtReceivedByLab)) {
						strDateReceivedByLab = FormatDateTimeForInterface(dtReceivedByLab, 0, dtoDateTime);
						varDateReceivedByLab = COleVariant(dtReceivedByLab);
					}
				}
			}

			//TES 4/28/2011 - PLID 43426 - Date Received By Lab
			hlriCurrentResult.varDateReceivedByLab = varDateReceivedByLab;

			//TES 4/29/2011 - PLID 43484 - ClientAccountNumber, MSH-6
			CString strClientAccountNumber;
			GetHL7MessageComponent(Message.strMessage, nHL7GroupID, "MSH", 1, 6, 1, 1, strClientAccountNumber);
			hlriCurrentResult.strClientAccountNumber = strClientAccountNumber;

			//TES 5/17/2013 - PLID 56286 - If we read the PerformingLabID out of OBR-21, set it on the current result
			if(nPerformingLabIDObr21 == (long)plcOBR21) {
				hlriCurrentResult.varPerformingLabID = (nPerformingLabID == -1 ? g_cvarNull : nPerformingLabID);
			}
			
			//TES 9/15/2009 - PLID 35557 - Sadly, the notes are now more complicated, as Caris has multiple notes associated
			// with OBX results.  We need to track the notes as we go, put any that are associated with OBX segments in the
			// relevant result, and then, if necessary, append a master note at the end.
			//TES 9/15/2009 - PLID 35557 - First, go through any notes that come before the first OBX, those will definitely
			// be part of our master note
			CString strMasterNote, strNote;
			//TES 4/5/2010 - PLID 37500 - Here and throughout the OBX processing, make sure we just search within the correct lab.
			int nFirstOBX = FindNthOccurrence(Message.strMessage, "\rOBX", 1, li.nStart, li.nEnd);
			if(nFirstOBX == -1) {
				//I bet they did it the non-standard way
				nFirstOBX = FindNthOccurrence(Message.strMessage, "\nOBX", 1, li.nStart, li.nEnd);
			}
			int nNoteLine = 1;
			//TES 9/17/2009 - PLID 35531 - This was looping as long as the function returned COMPONENT_FOUND.  However,
			// in some cases, the NTE segment exists without component 3, and then is followed by some more, valid NTE
			// segments.  Those valid NTE segments were therefore never getting processed.  So, we now loop until
			// there are no more NTE segments, regardless of whether they have component 3.
			//TES 5/7/2010 - PLID 37500 - Start at the beginning of this lab, not the beginning of the message
			while(SEGMENT_NOT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "NTE", nNoteLine, 3, 1, 1, strNote, li.nStart, nFirstOBX)) {
				//(s.dhole 2014-06-04 10:32) PLID 47478 - Aplly DeformatHL7Text to note
				if(!strMasterNote.IsEmpty()) strMasterNote += "\r\n";
				strMasterNote += DeformatHL7Text(strNote);
				nNoteLine++;
				strNote = "";
			}
			
			//TES 9/18/2008 - PLID 21093 - Now, go through all the OBX segments.
			int nOBXLine = 1;
			//TES 9/18/2008 - PLID 21093 - Start by getting the sub-group ID out of OBX-4, that will tell us whether we've moved on to 
			// a new result.
			//TES 6/29/2010 - PLID 39418 - Similar to the NTE logic just above, we need to loop until we run out of OBX segments, NOT
			// until we fail to find OBX-4.  An OBX segment without a 4th field is still an OBX segment.
			//TES 10/15/2010 - PLID 40951 - Moved all the OBX parsing into its own function, in HL7ParseUtils
			HL7_OBXSegment OBX;
			//TES 4/25/2011 - PLID 43423 - Pass in the setting for whether to use component 5 instead of 2
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			//TES 8/16/2011 - PLID 44262 - Pass in the cache, and therefore don't pass in the UseObx3_5 setting
			while(ParseOBXSegment(GetHL7SettingsCache(), strMessage, nHL7GroupID, nOBXLine, li.nStart, li.nEnd, OBX)) {
				// (r.gonet 2011-04-08) - PLID 42655 - Don't copy PDFs if we are copying results
				if(bCopyResult && i > 0 && !OBX.strFileData.IsEmpty()) {
					nOBXLine++;
					continue;
				}
				
				//TES 10/22/2010 - PLID 40951 - We've got all the information out of the OBX segment parsed into our struct, now update 
				// the status if necessary.
				if(OBX.strStatus.IsEmpty()) {
					//Use the master status that we pulled up above from OBR-25.
					OBX.strStatus = strMasterStatus;
					//TES 9/10/2013 - PLID 54288 - Track that we used the status from OBR
					OBX.bUsedMasterStatus = true;
				}

				//TES 2/8/2010 - PLID 37268 - Pull the diagnosis out of OBX-13, if we're configured to do so.
				//TES 10/22/2010 - PLID 40951 - This should be in ParseOBXSegment(), but HL7ParseUtils doesn't have access to the
				// HL7 Settings at the present time.
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				if(GetHL7SettingBit(nHL7GroupID, "UseOBX13")) {
					if(hlriCurrentResult.strDiagnosis.IsEmpty()) {						
						hlriCurrentResult.strDiagnosis += OBX.strDiagnosis;
					}
				}

				//TES 5/17/2013 - PLID 56286 - If we're reading the Performing Lab out of OBX-15, calculate the ID now
				// (d.singleton 2013-11-06 15:02) - PLID 59337 - need the city, state, zip,  parish code and country added to preforming lab for hl7 labs
				//TES 5/19/2015 - PLID 60125 - Don't check hlriCurrentResult here, that's the last result we processed, not the result we're currently on, which we
				// know we haven't calculated the nPerformingLabID for yet
				if(/*hlriCurrentResult.varPerformingLabID.vt == VT_NULL && */nPerformingLabIDObr21 != plcOBR21) {
					BOOL bRequirePerformingLab = GetHL7SettingBit(nHL7GroupID, "RequirePerformingLab");
					if (nPerformingLabIDObr21 == plcOBX15) {
						// (r.gonet 05/01/2014) - PLID 62013 - Log a diagnostic message
						LogHL7Debug("The setting \"Require using X as the Performing Lab ID\" is set to \"OBX-15\". Attempting to match OBX-15 with an existing Nextech lab location.");
					} else if (nPerformingLabIDObr21 == plcOBX23) {
						// (r.gonet 05/01/2014) - PLID 62013 - Log a diagnostic message
						LogHL7Debug("The setting \"Require using X as the Performing Lab ID\" is set to \"OBX-23\". Attempting to match OBX-23 with an existing Nextech lab location.");
					}
					OBX.nPerformingLabID = GetPracticeLabLocation(nHL7GroupID, OBX.strPerformingLabID, OBX.strPerformingLabName, 
								OBX.strPerformingLabAddress, "", OBX.strPerformingLabCity, OBX.strPerformingLabState, OBX.strPerformingLabZip, 
								!bRequirePerformingLab, "Performing Lab", OBX.strPerformingLabCountry, OBX.strPerformingLabParish);
					if(bRequirePerformingLab && OBX.nPerformingLabID == -1) {
						//TES 5/17/2013 - PLID 56286 - GetPracticeLabLocation() will already have notified them of the failure.
						// (r.gonet 05/01/2014) - PLID 62013 - Log an error
						LogHL7Error("No performing lab location could be found or matched from the HL7 message. Nextech requires a performing lab to be able to import this message.");
						// (r.gonet 05/01/2014) - PLID 62013 - Log a diagnostic message
						LogHL7Resolution("Ensure that the lab company actually sends a performing lab somewhere in the message. Some do not. Nextech has the ability to read the "
							"performing lab ID from OBX-15, OBR-21, or OBX-23, which can be configured on the Links->Receive Labs->Configure HL7 Settings->Labs->Advanced screen. "
							"Ensure that setting is set correctly.");
						return FALSE;
					}
					if(OBX.nPerformingLabID != -1) {
						hlriCurrentResult.varPerformingLabID = OBX.nPerformingLabID;
					}
				}

				//TES 10/22/2010 - PLID 40951 - Now remember the location, if we're using the OBX location
				//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
				if(rllReceivingLabLocation == rllOBX23) {					
					//TES 2/23/2010 - PLID 37501 - Now, if we have OBXs with different (non-empty) lab locations, then we can't handle that, 
					// so make sure that's not the case.
					if((!OBX.strLabName.IsEmpty() && !strCurrentLabName.IsEmpty() && OBX.strLabName != strCurrentLabName) ||
						(!OBX.strLabCode.IsEmpty() && !strCurrentLabCode.IsEmpty() && OBX.strLabCode != strCurrentLabCode) ||
						(!OBX.strLabAddress1.IsEmpty() && !strCurrentLabAddress1.IsEmpty() && OBX.strLabAddress1 != strCurrentLabAddress1) ||
						(!OBX.strLabAddress2.IsEmpty() && !strCurrentLabAddress2.IsEmpty() && OBX.strLabAddress2 != strCurrentLabAddress2) ||
						(!OBX.strLabCity.IsEmpty() && !strCurrentLabCity.IsEmpty() && OBX.strLabCity != strCurrentLabCity) ||
						(!OBX.strLabState.IsEmpty() && !strCurrentLabState.IsEmpty() && OBX.strLabState != strCurrentLabState) ||
						(!OBX.strLabZip.IsEmpty() && !strCurrentLabZip.IsEmpty() && OBX.strLabZip != strCurrentLabZip)) {
							// (r.gonet 05/01/2014) - PLID 61843 - Log an error
							LogHL7Error("This OBX's receiving lab location values from OBX-23 do not match the other OBX-23 values for the current OBR.");
							// (r.gonet 05/01/2014) - PLID 61843 - Log a possible resolution
							LogHL7Resolution("When \"OBX-23\" is set for the \"Use the Receiving Lab information from\" setting, OBX-23 needs to be consistent for all OBXs under the OBR. "
								"Please ensure that \"OBX-23\" is the setting that should be enabled.");
							//TES 2/23/2010 - PLID 37501 - FAIL!
							ASSERT(FALSE);
							AfxMessageBox("This HL7 Message could not be imported (Error: Inconsistent Receiving Lab information found in OBX segments)");

							// (j.jones 2010-04-08 09:03) - PLID 38101 - if we didn't process the result,
							// clear any audits we may have loaded into memory
							for(int i=0; i<arPendingResultAudits.GetSize(); i++) {
								CArray<CPendingAuditInfo,CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo,CPendingAuditInfo&>*)arPendingResultAudits.GetAt(i);
								if(parAudits) {
									delete parAudits;
									parAudits = NULL;
								}
							}
							arPendingResultAudits.RemoveAll();
							return FALSE;
					}
					else if(strCurrentLabName.IsEmpty() && !OBX.strLabName.IsEmpty()) {
						//TES 2/23/2010 - PLID 37501 - If this is the first lab location we've found, remember it.
						strCurrentLabName = OBX.strLabName;
						strCurrentLabCode = OBX.strLabCode;
						strCurrentLabAddress1 = OBX.strLabAddress1;
						strCurrentLabAddress2 = OBX.strLabAddress2;
						strCurrentLabCity = OBX.strLabCity;
						strCurrentLabState = OBX.strLabState;
						strCurrentLabZip = OBX.strLabZip;
					}

				}	

				//TES 10/22/2010 - PLID 40951 - Now, determine whether we are combining the OBX segment into our existing result; all this
				// logic has been consolidated into a single function.
				BOOL bCombine = CanCombineResult(OBX, hlriCurrentResult, strObr24Value, nHL7GroupID);

				//TES 10/22/2010 - PLID 40951 - If we're not combining, then if our existing result isn't empty, we need to commit the current
				// result and start a new one.
				if(!bCombine) {
					//TES 10/22/2010 - PLID 40951 - First, check if the current result even has anything in it.
					//TES 5/17/2013 - PLID 56286 - Pass in whether the Performing Lab is part of the OBX segment
					// (d.singleton 2013-10-31 16:47) - PLID 59181 - performing lab can also be in obx23 changed code for that
					if(!hlriCurrentResult.IsEmpty(!(nPerformingLabIDObr21 == (long)plcOBR21))) {
						// (r.gonet 07/18/2011) - PLID 37997 - Added in the dafault mail category ID
						//TES 8/6/2013 - PLID 51147 - Pass in nTodoPriority and bFlagFound, this will update them based on the result
						// (r.gonet 2015-11-17 23:51) - PLID 67590 - No code change, just a comment. The current logged in location is fine to use here because it is
						// only going to be used for the location of the mail sent record, if this result has a PDF.
						CommitLabResult(GetRemoteData(), hlriCurrentResult, nPersonID, strPersonName, Message, strSqlBatch, *pAryParams, saResults, nTodoPriority, bFlagFound, &arPendingResultAudits, GetCurrentLocationID(), GetCurrentUserName(), nLabAttachmentsDefaultCategory);
						//TES 11/4/2008 - PLID 21093 - Start a new one.
						hlriCurrentResult.Reset();
						hlriCurrentResult.strName = li.strLabName;
						// (a.walling 2010-02-25 13:41) - PLID 37534 - Date Performed
						hlriCurrentResult.varDatePerformed = varDatePerformed;
						//TES 4/28/2011 - PLID 43426 - Date Received By Lab
						hlriCurrentResult.varDateReceivedByLab = varDateReceivedByLab;
						//TES 4/29/2011 - PLID 43484 - Client Account Number
						hlriCurrentResult.strClientAccountNumber = strClientAccountNumber;
					}
				}
				//TES 10/22/2010 - PLID 40951 - Finally, add the OBX information to our current result
				//TES 6/4/2010 - PLID 38776 - Moved this code to a new function, since it may not actually get appended to the value.
				//TES 8/2/2010 - PLID 38776 - Pass in the lab type.
				AddObservationToResult(hlriCurrentResult, OBX, li.eLabType, Message.nMessageID);
			
				nOBXLine++;
			}

			//TES 6/4/2010 - PLID 38776 - Commit the result if any of the fields have been filled, not just the value.
			//TES 5/9/2011 - PLID 43633 - Call the IsEmpty() function rather than just checking the value-related fields.  If the value
			// field wasn't filled in, but something else (i.e., the Status field) was, we still want to make sure that we store that information.
			// The IsEmpty() function looks at all the fields which are filled from the OBX segment.
			//TES 5/17/2013 - PLID 56286 - Pass in whether the Performing Lab is part of the OBX segment
			if(!hlriCurrentResult.IsEmpty(!(nPerformingLabIDObr21 == (long)plcOBR21))) {
				// (r.gonet 07/18/2011) - PLID 37997 - Added in the dafault mail category ID
				//TES 8/6/2013 - PLID 51147 - Pass in nTodoPriority and bFlagFound, this will update them based on the result
				// (r.gonet 2015-11-17 23:51) - PLID 67590 - No code change, just a comment. The current logged in location is fine to use here because it is
				// only going to be used for the location of the mail sent record, if this result has a PDF.
				CommitLabResult(GetRemoteData(), hlriCurrentResult, nPersonID, strPersonName, Message, strSqlBatch, *pAryParams, saResults, nTodoPriority, bFlagFound, &arPendingResultAudits, GetCurrentLocationID(), GetCurrentUserName(), nLabAttachmentsDefaultCategory);
			}

			//TES 9/15/2009 - PLID 35557 - If we wound up with a "master" note for the result, commit it now.
			if(!strMasterNote.IsEmpty()) {
				// (b.spivey, March 29, 2012) - PLID 49313 - We need to reset this to prevent residual values. 
				hlriCurrentResult.Reset(); 
				//TES 10/14/2008 - PLID 21093 - Commit this as a result named "Notes"
				hlriCurrentResult.strName = "Notes";
				hlriCurrentResult.strValue = strMasterNote;
				//TES 12/1/2008 - PLID 32192 - Notes don't have their own status, use the master status, OBR-25, which
				// we've already pulled
				hlriCurrentResult.strStatusFlag = strMasterStatus;
				//TES 9/10/2013 - PLID 54288 - Track that we used the OBR status
				hlriCurrentResult.bUsedMasterStatus = true;
				// (r.gonet 07/18/2011) - PLID 37997 - Added in the dafault mail category ID
				//TES 8/6/2013 - PLID 51147 - Pass in nTodoPriority and bFlagFound, this will update them based on the result
				CommitLabResult(GetRemoteData(), hlriCurrentResult, nPersonID, strPersonName, Message, strSqlBatch, *pAryParams, saResults, nTodoPriority, bFlagFound, &arPendingResultAudits, GetCurrentLocationID(), GetCurrentUserName(), nLabAttachmentsDefaultCategory);
				hlriCurrentResult.Reset();
				hlriCurrentResult.strName = li.strLabName;			
				// (a.walling 2010-02-25 13:41) - PLID 37534 - Date Performed
				hlriCurrentResult.varDatePerformed = varDatePerformed;
				//TES 4/28/2011 - PLID 43426 - Date Received By Lab
				hlriCurrentResult.varDateReceivedByLab = varDateReceivedByLab;
				//TES 4/29/2011 - PLID 43484 - Client Account Number
				hlriCurrentResult.strClientAccountNumber = strClientAccountNumber;
			}

			//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
			if(rllReceivingLabLocation == rllOBX23) {
				// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
				LogHL7Debug("The setting \"Use the Receiving Lab information from\" is set to \"OBX-23\". Attempting to match OBX-23.10 with an existing Nextech lab location.");
				//TES 2/23/2010 - PLID 37501 - Call our function to turn the information we've gathered into a LabID.
				//TES 4/29/2011 - PLID 43424 - Renamed from GetPracticeLocationFromOBX to GetPracticeLabLocation, the parameters for this function
				// could be coming from OBR or OBX
				//TES 2/25/2013 - PLID 54876 - Specify that this is for a Receiving Lab, not a Performing Lab
				long nPracticeLabLocationID = GetPracticeLabLocation(nHL7GroupID, strCurrentLabCode, strCurrentLabName, strCurrentLabAddress1, strCurrentLabAddress2, strCurrentLabCity, strCurrentLabState, strCurrentLabZip,
					FALSE, "Receiving Lab");
				if(nPracticeLabLocationID == -1) {
					//TES 2/23/2010 - PLID 37501 - The user will already have been notified as to why this failed, so just return.

					// (j.jones 2010-04-08 09:03) - PLID 38101 - if we didn't process the result,
					// clear any audits we may have loaded into memory
					for(int i=0; i<arPendingResultAudits.GetSize(); i++) {
						CArray<CPendingAuditInfo,CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo,CPendingAuditInfo&>*)arPendingResultAudits.GetAt(i);
						if(parAudits) {
							delete parAudits;
							parAudits = NULL;
						}
					}
					arPendingResultAudits.RemoveAll();

					// (r.gonet 05/01/2014) - PLID 61843 - Log an error
					LogHL7Error("Could not match the HL7 lab location found in OBX-23.10 with an existing Nextech lab location.");
					LogHL7Resolution("Please ensure that the HL7 setting \"Use the Receiving Lab information from\" is correct and that the lab is sending a value in that field.");
					return FALSE;
				}
				//TES 2/23/2010 - PLID 37501 - I know this messes up the advantages of parameterization, but a.) this function's parameterization
				// is already somewhat messed up, and b.) we don't at this point actually anticipate any clients ever using this setting, it's
				// just for CCHIT.
				strSqlBatch.Replace("LAB_LOCATION_PLACEHOLDER", AsString(nPracticeLabLocationID));
			}
			
			//TES 10/14/2008 - PLID 21093 - Flag this message as committed (because of the weird way we do auditing,
			// we can't just call FinalizeHL7Event).
			AddParamStatementToSqlBatch(strSqlBatch, *pAryParams, "UPDATE HL7MessageQueueT SET ActionID = {INT} WHERE ID = {INT}", (long)mqaCommit, Message.nMessageID);

			// (j.jones 2010-05-13 10:59) - PLID 36527 - if we don't have a provider ID,
			// we will have to select it
			CString strProvRecordset = "";
			if(bHL7IDLinkT_NeedAudit && (strProviderPersonID == "NULL" || strProviderPersonID == "@nProviderID")) {
				strProvRecordset = "SELECT @nProviderID AS NewProviderID \r\n";
			}
			
			//TES 11/13/2014 - PLID 55674 - Remember all the info we need to commit this to data.
			lci.strSqlBatch = strSqlBatch;
			lci.pArPendingLabAudits = new CArray < CPendingAuditInfo, CPendingAuditInfo& >() ;
			lci.pArPendingLabAudits->Copy(arPendingLabAudits);
			lci.pArPendingResultAudits = new CArray < CArray<CPendingAuditInfo, CPendingAuditInfo&>*, CArray<CPendingAuditInfo, CPendingAuditInfo&>* > ;
			lci.pArPendingResultAudits->Copy(arPendingResultAudits);
			lci.strProviderPersonID = strProviderPersonID;
			lci.strProvRecordset = strProvRecordset;
			lci.bHL7IDLinkT_NeedAudit = bHL7IDLinkT_NeedAudit;
			lci.strProviderPersonID = strProviderID;
			lci.psaResults = new CStringArray;
			lci.psaResults->Copy(saResults);
			lci.nTodoPriority = nTodoPriority;
			aryLabCommits.SetAt(aryLabCommits.GetSize() - 1, lci);
			
		} // for(int i = 0; i < arLabs.GetSize(); i++) {

		//TES 11/13/2014 - PLID 55674 - Now commit everything to data, in a transaction
		//TES 1/14/2015 - PLID 55674 - Track which todos change, send tablecheckers after committing
		//TES 1/23/2015 - PLID 55674 - Added logging around transactions; the logs won't get rolled back so we want to know which are associated with rolled-back vs. committed transactions
		LogHL7Debug("Beginning lab import transaction");
		std::list<ChangedTodo> listChangedTodos;
		commitTrans.Begin();
		for (int nLabCommit = 0; nLabCommit < aryLabCommits.GetSize(); nLabCommit++)
		{
			LabCommitInfo lci = aryLabCommits[nLabCommit];
			//TES 10/8/2008 - PLID 21093 - OK, we're all set.  Execute!
			CString strRecordset;
			strRecordset.Format(
				"SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n"
				"%s "
				"COMMIT TRAN \r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT @nNewLabID AS NewLabID, @nLabID AS LabID \r\n"
				"SELECT LabResultID, ArrayIndex FROM @NewLabResultsT \r\n"
				"%s",
				lci.strSqlBatch, lci.strProvRecordset);

			//TES 10/13/2008 - PLID 21093 - OK, our first recordset will be the new LabsT.ID (-1 if we didn't actually
			// create a LabsT record).
			// (e.lally 2009-06-21) PLID 34680 - Fixed to use create recordset function.
			//TES 1/14/2015 - PLID 55674 - We're already in a transaction, so don't use the batch version which would add an additional transaction
			_RecordsetPtr rsResults = CreateParamRecordsetBatch(GetRemoteData(), strRecordset, *(lci.pAryParams), CommandCache::Auto, false);
			long nNewLabID = AdoFldLong(rsResults, "NewLabID");
			const long nLabID = AdoFldLong(rsResults->GetFields(), "LabID");
			//TES 7/16/2010 - PLID 39642 - We may have auditing even on existing labs now.
			//if(nNewLabID != -1) {
			//TES 10/13/2008 - PLID 21093 - OK, go through and actually audit everything we changed for this lab, now
			// that we have a record id.
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginNewAuditEvent();
			}
			for (int i = 0; i < lci.pArPendingLabAudits->GetSize(); i++) {
				CPendingAuditInfo pai = lci.pArPendingLabAudits->GetAt(i);
				AuditEvent(pai.m_nPatientID, pai.m_strPersonName, nAuditTransactionID, pai.m_aeiItem,
					pai.m_nRecordID == -1 ? nNewLabID : pai.m_nRecordID, pai.m_strOldValue, pai.m_strNewValue, pai.m_aepPriority,
					pai.m_aetType);
			}
			//}
			rsResults = rsResults->NextRecordset(NULL);
			//TES 10/13/2008 - PLID 21093 - The next records is the table that maps LabsResultsT.IDs to indexes in our master array.
			// (j.gruber 2010-02-24 10:23) - PLID 37510 - add to dword array
			CDWordArray dwaryResultIDs;
			while (!rsResults->eof) {
				//TES 10/13/2008 - PLID 21093 - Get the index in our master array.
				long nIndex = AdoFldLong(rsResults, "ArrayIndex");
				CArray<CPendingAuditInfo, CPendingAuditInfo&> *parAudits = lci.pArPendingResultAudits->GetAt(nIndex);
				//TES 10/13/2008 - PLID 21093 - Get the record ID.
				long nRecordID = AdoFldLong(rsResults, "LabResultID");
				dwaryResultIDs.Add(nRecordID);
				if (nAuditTransactionID == -1) {
					nAuditTransactionID = BeginNewAuditEvent();
				}
				//TES 10/13/2008 - PLID 21093 - Now go through all the audits in this array, and actually audit them.
				for (int i = 0; i < parAudits->GetSize(); i++) {
					CPendingAuditInfo pai = parAudits->GetAt(i);
					AuditEvent(pai.m_nPatientID, pai.m_strPersonName, nAuditTransactionID, pai.m_aeiItem, nRecordID, pai.m_strOldValue,
						pai.m_strNewValue, pai.m_aepPriority, pai.m_aetType);
				}
				//TES 10/13/2008 - PLID 21093 - Clean up this array (this will throw an exception if we somehow wind up with
				// duplicated indexes in our @NewLabResultsT table, which is fine).
				delete parAudits;
				parAudits = NULL;
				lci.pArPendingResultAudits->SetAt(nIndex, NULL);
				rsResults->MoveNext();
			}
			lci.pArPendingResultAudits->RemoveAll();

			// (j.jones 2010-05-13 10:59) - PLID 36527 - now, do we need to audit?
			if (lci.bHL7IDLinkT_NeedAudit) {
				long nProviderID = 0;
				if ((lci.strProviderPersonID == "NULL" || lci.strProviderPersonID == "@nProviderID") && !lci.strProvRecordset.IsEmpty()) {

					rsResults = rsResults->NextRecordset(NULL);
					if (!rsResults->eof) {
						nProviderID = AdoFldLong(rsResults, "NewProviderID", 0);
					}
				}
				else {
					nProviderID = atoi(lci.strProviderPersonID);
				}

				if (nProviderID > 0) {
					//finally audit
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginNewAuditEvent();
					}
					CString strOld, strNew, strName;
					_RecordsetPtr rsProv = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", nProviderID);
					if (!rsProv->eof) {
						strName = AdoFldString(rsProv, "Name", "");
					}
					rsProv->Close();
					strOld.Format("Provider Code '%s' (HL7 Group '%s')", lci.strProviderID, Message.strHL7GroupName);
					strNew.Format("Linked to Provider '%s'", strName);
					AuditEvent(nProviderID, strName, nAuditTransactionID, aeiHL7ProviderLink, nProviderID, strOld, strNew, aepLow, aetCreated);
				}
			}

			//TES 10/14/2008 - PLID 21093 - Audit that we've processed this message (because of the weird way we do auditing,
			// we can't just call FinalizeHL7Event).
			if (nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPersonID, Message.strPatientName, nAuditTransactionID, aeiHL7MessageProcessed, Message.nMessageID,
				"Pending (" + Message.strHL7GroupName + " - " + GetActionDescriptionForHL7Event(Message.strMessage, Message.nHL7GroupID) + ")",
				"Committed", aepMedium, aetChanged);

			if (nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
			}

			// (j.dinatale 2011-10-04 11:15) - PLID 38934 - Obey the spawn lab todo preference
			BOOL bCreateTodo = GetRemotePropertyInt("LabSpawnToDo", 1, 0, "<None>");

			if (bCreateTodo){
				//TES 10/10/2008 - PLID 31646 - Create a ToDo Alarm so they know the results for this lab have changed.
				// (z.manning 2010-05-12 15:07) - PLID 37405 - Added parameter for lab ID
				//TES 8/6/2013 - PLID 51147 - Pass in the priority we calculated
				long nTaskID = TodoCreateForLab(nPersonID, nLabID, *(lci.psaResults), true, (TodoPriority)lci.nTodoPriority);

				if (nTaskID != -1) {
					//TES 1/14/2015 - PLID 55674 - Don't send the tablechecker, just store the info for later
					ChangedTodo ct;
					ct.nTaskID = nTaskID;
					ct.nPatientID = nPersonID;
					ct.nAssignedTo = GetCurrentUserID();
					ct.tsStatus = TableCheckerDetailIndex::tddisAdded;
					listChangedTodos.push_back(ct);
				}
			}

			if (dwaryResultIDs.GetSize() > 0) {
				// (c.haag 2010-09-21 12:48) - PLID 40612 - Replaced dwaryResultIDs with nPersonID
				//TES 10/31/2013 - PLID 59251 - Pass in the array our caller gave us
				UpdateDecisionRules(GetRemoteData(), nPersonID, arNewCDSInterventions);
			}

			//TES 1/4/2011 - PLID 37877 - Now find any steps for this lab that are flagged to be marked as complete when an HL7 result 
			// comes in (that aren't already completed), and complete them.
			long nRecordsAffected = -1;
			ExecuteParamSql(GetRemoteData(), &nRecordsAffected,
				"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
				"UPDATE LabStepsT SET StepCompletedDate = getdate(), StepCompletedBy = {INT} "
				"FROM LabStepsT LEFT JOIN LabProcedureStepsT ON LabStepsT.LabProcedureStepID = LabProcedureStepsT.StepID "
				"WHERE LabID = {INT} AND StepCompletedDate Is Null AND LabProcedureStepsT.CompletedByHL7 = 1",
				GetCurrentUserID(), nLabID);
			//TES 1/4/2011 - PLID 37877 - If we marked any steps complete, make sure any associated todo alarms are up to date.
			if (nRecordsAffected > 0) {
				//TES 1/4/2011 - PLID 37877 - Pass in the patient ID
				//TES 1/14/2015 - PLID 55674 - Pass in our list of Todos that need tablecheckers
				SyncTodoWithLab(nLabID, nPersonID, &listChangedTodos);
			}
		}
		//TES 11/13/2014 - PLID 55674 - Commit
		//TES 1/23/2015 - PLID 55674 - Added logging around transactions; the logs won't get rolled back so we want to know which are associated with rolled-back vs. committed transactions
		LogHL7Debug("Commiting lab import transaction");
		commitTrans.Commit();


		// (r.gonet 09/02/2014) - PLID 63221 - This patient had a lab created for them. We'll
		// want to send a table checker for the patient.
		setPatientsWithNewLabsCreated.insert(nPersonID);

		// (r.gonet 08/25/2014) - PLID 63221 - Firing a general table checker for labs is detrimental to performance due to
		// the requerying on the labs table. Fire one for each patient that was affected instead.
		for each(long nPatientID in setPatientsWithNewLabsCreated)
		{
			CClient::RefreshLabsTable(nPatientID, -1);
		}
		//TES 1/14/2015 - PLID 55674 - Now send all the tablecheckers for changed Todos
		for each(ChangedTodo ct in listChangedTodos)
		{
			CClient::RefreshTodoTable(ct.nTaskID, ct.nPatientID, ct.nAssignedTo, ct.tsStatus);
		}
		
		// (r.gonet 05/01/2014) - PLID 61843 - Log an information message 
		LogHL7Info("Successfully committed the HL7 lab result message.");

		//TES 11/13/2014 - PLID 55674 - Cleanup
		for (int nLabCommit = 0; nLabCommit < aryLabCommits.GetSize(); nLabCommit++) {
			LabCommitInfo lci = aryLabCommits[nLabCommit];
			if (lci.pAryParams) {
				delete lci.pAryParams;
			}
			if (lci.pArPendingResultAudits) {
				for (int i = 0; i < lci.pArPendingResultAudits->GetSize(); i++) {
					CArray<CPendingAuditInfo, CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo, CPendingAuditInfo&>*)lci.pArPendingResultAudits->GetAt(i);
					if (parAudits) {
						delete parAudits;
						parAudits = NULL;
					}
				}
				delete lci.pArPendingResultAudits;
			}
			if (lci.pArPendingLabAudits) {
				delete lci.pArPendingLabAudits;
			}
			if (lci.psaResults) {
				delete lci.psaResults;
			}
		}
		return TRUE;

	// (r.gonet 05/01/2014) - PLID 61843 - Log any exceptions to the HL7 Log
	} NxCatchAllHL7("Error in ImportHL7LabResult()");

	if(nAuditTransactionID != -1) {
		RollbackAuditTransaction(nAuditTransactionID);
	}

	//TES 11/13/2014 - PLID 55674 - Cleanup
	if (commitTrans.InProgress()) {
		//TES 1/23/2015 - PLID 55674 - Added logging around transactions; the logs won't get rolled back so we want to know which are associated with rolled-back vs. committed transactions
		LogHL7Debug("Rolling back lab import transaction");
		commitTrans.Rollback();
	}

	//TES 11/13/2014 - PLID 55674 - Cleanup
	for (int nLabCommit = 0; nLabCommit < aryLabCommits.GetSize(); nLabCommit++) {
		LabCommitInfo lci = aryLabCommits[nLabCommit];
		if (lci.pAryParams) {
			delete lci.pAryParams;
		}
		if (lci.pArPendingLabAudits) {
			for (int i = 0; i < lci.pArPendingResultAudits->GetSize(); i++) {
				CArray<CPendingAuditInfo, CPendingAuditInfo&> *parAudits = (CArray<CPendingAuditInfo, CPendingAuditInfo&>*)lci.pArPendingResultAudits->GetAt(i);
				if (parAudits) {
					delete parAudits;
					parAudits = NULL;
				}
			}
			delete lci.pArPendingLabAudits;
		}
		if (lci.pArPendingResultAudits) {
			delete lci.pArPendingResultAudits;
		}
		if (lci.psaResults) {
			delete lci.psaResults;
		}
	}

	return FALSE;
}

//TES 2/23/2010 - PLID 37501 - Attempts to match the existing information to an existing lab.  Will either return a valid LocationsT.ID,
// or -1, in which case the user will have been notified that the message will not be imported.
//TES 4/29/2011 - PLID 43424 - Renamed to GetPracticeLabLocation, this might be coming from OBX-23 or OBR-21
//TES 2/25/2013 - PLID 54876 - Added bSilent and strLabType.  bSilent controls whether it will prompt the user if no existing mapping is found,
// and strLabType will be used in any messages ("Receiving Lab" and "Performing Lab" are the two current options).  If bSilent is FALSE, then it is possible
// to return -1 without the user having been notified.
// (d.singleton 2013-11-05 17:10) - PLID 59337 - need the parish code and country added to preforming lab for hl7 labs
long GetPracticeLabLocation(long nHL7GroupID, const CString &strThirdPartyID, const CString &strName, const CString &strAddress1, const CString &strAddress2, const CString &strCity, const CString &strState, const CString &strZip,
							BOOL bSilent, const CString &strLabType, const CString &strCountry/*=""*/, const CString &strParish/*=""*/)
{
	//TES 5/20/2011 - PLID 43424 - Don't link blank IDs
	if(strThirdPartyID.IsEmpty()) {
		//TES 6/21/2013 - PLID 55658 - We need to notify the user of the reason for the failure, unless bSilent is set.
		if(!bSilent) {
			MsgBox("No %s was found in the message. This message will not be imported.", strLabType);
		}
		// (r.gonet 05/01/2014) - PLID 61843 - Log a diagnostic message
		LogHL7Debug("Lab location match failed. The HL7 third party lab location ID is blank.");
		return -1;
	}
	long nLabLocationID = -1;
	//TES 2/23/2010 - PLID 37501 - Try and silently match it.
	HL7MatchResult hmr = FindMatchingLabLocation(GetRemoteData(), strThirdPartyID, strName, nHL7GroupID, nLabLocationID);
	switch(hmr) {
	case hmrSuccess:
		{
			//TES 2/23/2010 - PLID 37501 - Got it, so we can just go ahead and return it.
			return nLabLocationID;
		}
		break;

	case hmrNoMatchFound:
		{
			if(bSilent) {
				return -1;
			}
			else {
				CString strHL7GroupName = "";

				//TES 2/23/2010 - PLID 37501 - Nope, so prompt the user.
				CSelectDlg dlg(NULL);
				dlg.m_strTitle = "Select " + strLabType;
				dlg.m_strCaption = "You are attempting to import a Lab Result with the " + strLabType + " code '" + strThirdPartyID + "' (Name: " + strName + ").\r\n"
					"Please select a Practice Lab Location record that should correspond with this code (you may need to contact your software vendor for assistance).\r\n\r\n"
					"If you cancel this dialog, the Lab Result will not be imported", 
				//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
				dlg.m_strFromClause.Format("(SELECT ID, Name FROM LocationsT WHERE TypeID = 2"
					"OR ID IN "
					"(SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %i) "
					"UNION SELECT -1, '<Create New Lab Location>') AS LabLocationsQ ", hclrtLocation);
				dlg.AddColumn("ID", "ID", FALSE, FALSE);
				DatalistColumn dcID = dlg.m_arColumns[0];
				dcID.nSortPriority = 0;
				dlg.m_arColumns.SetAt(0,dcID);
				dlg.AddColumn("Name", "Lab Location", TRUE, TRUE);
				if(dlg.DoModal() == IDOK) {
					ASSERT(dlg.m_arSelectedValues.GetSize() == 2);
					nLabLocationID = VarLong(dlg.m_arSelectedValues[0]);
					if(nLabLocationID == -1) {
						//TES 2/23/2010 - PLID 37501 - They chose to create a new lab, so let's do it.
						//TES 2/23/2010 - PLID 37501 - Does this exist as a non-lab Location?  Because we can't have duplicate LocationsT.Name values
						CString strNewName = strName;
						_RecordsetPtr rsExistingLocation = CreateParamRecordset(GetRemoteData(), "SELECT ID FROM LocationsT WHERE Name = {STRING}", strNewName);
						while(!rsExistingLocation->eof) {
							//TES 2/23/2010 - PLID 37501 - It does, prompt them for a new name.
							if(IDOK != InputBox(NULL, "The imported " + strLabType + " has the same name as an existing, non-Lab Location record.  Please enter a new name to use for the " + strLabType + ".  If you cancel this dialog, the lab will not be imported.", strNewName, "")) {
								// (r.gonet 05/01/2014) - PLID 61843 - Log a warning
								LogHL7Warning("The new lab location's name conflicted with an existing Nextech lab location. The user cancelled when prompted to rename the new lab location.");
								return FALSE;
							}
							else {
								rsExistingLocation = CreateParamRecordset(GetRemoteData(), "SELECT ID FROM LocationsT WHERE Name = {STRING}", strNewName);
							}
						}
						//TES 2/23/2010 - PLID 37501 - OK, go ahead and create our new lab.
						// (d.singleton 2013-11-05 17:10) - PLID 59337 - need the parish code and country added to preforming lab for hl7 labs
						CString strSqlBatch;
						CNxParamSqlArray aryParams;
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DECLARE @nLocationID int;");
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SELECT @nLocationID = Max(ID)+1 FROM LocationsT;");
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO LocationsT (ID, Name, Address1, Address2, City, State, Zip, TypeID, Country, ParishCode) "
							"VALUES (@nLocationID, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, 2, {STRING}, {STRING})",
							_T(strNewName), _T(strAddress1), _T(strAddress2), _T(strCity), _T(strState), _T(strZip), _T(strCountry), _T(strParish));
						if(!strThirdPartyID.IsEmpty()) {
							//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
							// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
							CreateNewHL7CodeLinkT_WithVariableName(nHL7GroupID, hclrtLocation, strThirdPartyID, "@nLocationID", strSqlBatch, &aryParams);
						}
						// (j.jones 2010-06-25 11:53) - PLID 39185 - link with all labs to be ordered records
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO LabsToBeOrderedLocationLinkT (LocationID, LabsToBeOrderedID) "
							"SELECT @nLocationID, ID FROM LabsToBeOrderedT");
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SELECT @nLocationID AS NewLocationID");

						//TES 2/23/2010 - PLID 37501 - Now pull the ID of the new record.
						_RecordsetPtr rsNewLocation = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, aryParams);
						_variant_t v;
						//TES 2/23/2010 - PLID 37501 - Skip past the two INSERTs.
						rsNewLocation = rsNewLocation->NextRecordset(&v);					
						//TES 6/15/2010 - PLID 38907 - The second INSERT only happens if we have a ThirdPartyID
						if(!strThirdPartyID.IsEmpty()) {
							rsNewLocation = rsNewLocation->NextRecordset(&v);
						}
						// (j.jones 2010-08-09 16:12) - PLID 39185 - skip the third insert for LabsToBeOrderedLocationLinkT
						rsNewLocation = rsNewLocation->NextRecordset(&v);
						long nNewLocationID = AdoFldLong(rsNewLocation, "NewLocationID");
						CClient::RefreshTable(NetUtils::LocationsT, nNewLocationID);

						// (j.jones 2010-05-12 11:07) - PLID 36527 - audit this
						if(!strThirdPartyID.IsEmpty()) {
							long nAuditID = BeginNewAuditEvent();

							if(strHL7GroupName.IsEmpty()) {
								//TES 6/22/2011 - PLID 44261 - This is in a global cache now
								strHL7GroupName = GetHL7GroupName(nHL7GroupID);
							}

							CString strOld, strNew;
							strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, strHL7GroupName);
							strNew.Format("Linked to %s", strNewName);
							AuditEvent(-1, "", nAuditID, aeiHL7LocationLink, nHL7GroupID, strOld, strNew, aepLow, aetCreated);
						}

						return nNewLocationID;
					}
					else {
						//TES 2/23/2010 - PLID 37501 - They selected an existing location, store the ID mapping.
						if(!strThirdPartyID.IsEmpty()) {
							//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
							// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
							ExecuteParamSql("{SQL}", CreateNewHL7CodeLinkT(nHL7GroupID, hclrtLocation, strThirdPartyID, nLabLocationID));

							// (j.jones 2010-05-12 11:07) - PLID 36527 - audit this
							long nAuditID = BeginNewAuditEvent();

							if(strHL7GroupName.IsEmpty()) {
								//TES 6/22/2011 - PLID 44261 - This is in a global cache now
								strHL7GroupName = GetHL7GroupName(nHL7GroupID);
							}

							CString strLocationName;
							if(nLabLocationID == -1) {
								strLocationName = "<Use Practice Default>";
							}
							else {
								_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLabLocationID);
								if(!rsLocation->eof) {
									strLocationName = AdoFldString(rsLocation, "Name");
								}
								rsLocation->Close();
							}

							CString strOld, strNew;
							strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, strHL7GroupName);
							strNew.Format("Linked to %s", strLocationName);
							AuditEvent(-1, "", nAuditID, aeiHL7LocationLink, nHL7GroupID, strOld, strNew, aepLow, aetCreated);
						}
						//TES 2/23/2010 - PLID 37501 - Now, return the selected ID.
						return nLabLocationID;
					}
				}
				else {
					//TES 2/23/2010 - PLID 37501 - They cancelled.
					return -1;
				}
			}
		}
		break;

	case hmrMultipleMatchesFound:
		//TES 2/23/2010 - PLID 37501 - Since location names are unique, the function should never return this enum, 
		// so let the default error handling take it.
	default:
		AfxThrowNxException("Invalid result %i returned in GetPracticeLocation()", hmr);
		//TES 4/17/2008 - PLID 29595 - Need a return value to prevent compiler warnings.
		return -1;
		break;
	}
}

// (z.manning 2010-05-21 13:16) - PLID 38638 - Function to get the audit text when auditing changes
// in the required fields for auto importing lab results
CString GetLabImportMatchingFieldAuditText(const DWORD dwLabImportMatchingFieldFlags)
{
	CString strAudit;
	if((dwLabImportMatchingFieldFlags & limfPatientID) == limfPatientID) { strAudit += "Patient ID, "; }
	if((dwLabImportMatchingFieldFlags & limfPatientName) == limfPatientName) { strAudit += "Patient Name, "; }
	if((dwLabImportMatchingFieldFlags & limfFormNumber) == limfFormNumber) { strAudit += "Form Number, "; }
	if((dwLabImportMatchingFieldFlags & limfSSN) == limfSSN) { strAudit += "SSN, "; }
	if((dwLabImportMatchingFieldFlags & limfBirthDate) == limfBirthDate) { strAudit += "Birth Date, "; }
	// (r.gonet 03/07/2013) - PLID 55527 - We now consider the specimen a separate entitity from the form number.
	if((dwLabImportMatchingFieldFlags & limfSpecimen) == limfSpecimen) { strAudit += "Specimen, "; }
	// (r.gonet 03/07/2013) - PLID 55527 - We now allow matching on the test code / universal service identifier.
	if((dwLabImportMatchingFieldFlags & limfTestCode) == limfTestCode) { strAudit += "Test Code, "; }
	strAudit.TrimRight(", ");

	return strAudit;
}

// (z.manning 2010-07-01 13:45) - PLID 39422
BOOL HandleHL7SchedulerMessage(const HL7Message &message, CWnd *pwndParent)
{
	BOOL bSuccess = FALSE;
	HL7_Appointment hl7appt;
	// (b.spivey, July 09, 2012) - PLID 49170 - Pass GetRemoteData() for ParsePIDSegment()
	ParseSIUMessage(message.strMessage, message.nHL7GroupID, hl7appt, GetRemoteData());
	
	long nApptID = -1;
	FindMatchingAppointment(GetRemoteData(), message.nHL7GroupID, hl7appt.strHL7ID, nApptID);

	// (z.manning 2010-07-14 12:47) - PLID 39422 - Support non-patient appts
	long nPatientID = -1;
	if(hl7appt.PID.strHL7ID.IsEmpty()) {
		nPatientID = -25;
	}
	else {
		// (z.manning 2010-07-01 11:00) - PLID 39422 - Get the patient ID from the message
		ENotFoundResult nfr;
		ENotFoundBehavior nfb;
		// (z.manning 2010-08-09 10:22) - PLID 39985 - Added an option to auto-create patients from SIU messages
		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		BOOL bCreatePatient = GetHL7SettingBit(message.nHL7GroupID, "AutoCreatePatientsFromScheduleMessages");
		if(bCreatePatient) {
			// (z.manning 2010-08-09 10:23) - PLID 39985 - Don't prompt to create a patient since it's set to do
			// so automatically.
			nfb = nfbSkip;
		}
		else {
			// (z.manning 2010-08-09 10:24) - PLID 39985 - Prompt to create the patient
			nfb = nfbPromptToLinkAndCreate;
		}
		nPatientID = GetPatientFromHL7Message(message.strMessage, message.nHL7GroupID, pwndParent, nfb, &nfr);
		if(nPatientID == -1 && (nfr == nfrCreateNew || bCreatePatient)) {
			if(!AddHL7PatientToData(message, TRUE, &nPatientID)) {
				return FALSE;
			}
		}
		if(nPatientID == -1) {
			// (z.manning 2010-07-01 11:01) - PLID 39422 - We can't import an appointment without a valid patient ID
			return FALSE;
		}
	}

	// (z.manning 2010-07-01 11:03) - PLID 39422 - Get the location from the message
	long nLocationID = -1;
	if(hl7appt.strLocationID.IsEmpty()) {
		// (z.manning 2010-07-01 11:04) - PLID 39422 - If they didn't give a location let's just default to the
		// currently logged in location.
		// (z.manning 2010-07-15 09:29) - But only do this for new appts. No need to possibly change the location
		// of existing appts to the current location.
		if(nApptID == -1) {
			nLocationID = GetCurrentLocationID();
		}
	}
	else {
		// (j.dinatale 2011-10-19 16:10) - PLID 44823 - need to call the new function to get appointment location
		if(!GetHL7ApptLocation(message, hl7appt.strLocationID, nLocationID)) {
			return FALSE;
		}
	}

	// (j.dinatale 2013-01-04 11:02) - PLID 54446 - we need to pay attention to referring phys now
	// (j.dinatale 2013-01-15 14:10) - PLID 54631 - we now have a setting to determine if we want to import ref phys from appt messages
	long nRefPhysID = APPT_REFPHYS_IMPORT_DISABLED;
	if(GetHL7SettingBit(message.nHL7GroupID, "IMPORTAPPTREFPHYS")){
		nRefPhysID = -1;
		HL7ReferringPhysician hrp;
		hrp.strThirdPartyID = hl7appt.PID.strRefPhysThirdPartyID;
		hrp.strFirst = hl7appt.PID.strRefPhysFirst;
		hrp.strLast = hl7appt.PID.strRefPhysLast;
		hrp.strDegree = hl7appt.PID.strRefPhysDegree;
		//TES 4/22/2015 - PLID 61147 - Added support for the prefix
		hrp.strPrefix = hl7appt.PID.strRefPhysPrefix;

		// (j.dinatale 2013-01-29 09:14) - PLID 54901 - need prompts to handle when we have duplicate ref phys and also if the user wants to create
		//		a ref phys when we cant match one up
		HL7MatchResult hl7RefPhysMatch = FindMatchingRefPhys(GetRemoteData(), hrp, message.nHL7GroupID, nRefPhysID);
		if(hl7RefPhysMatch != hmrSuccess) {
			switch(hl7RefPhysMatch){
				case hmrMultipleMatchesFound:	// multiples
					{
						CHL7DuplicateRefPhysDlg dlg(NULL);
						long nNewRefPhysID = dlg.Open(message.nHL7GroupID, hrp.strThirdPartyID, hrp.strFirst, hrp.strLast, hrp.strDegree, false, "record");
						if(nNewRefPhysID > 0){
							// if they selected one
							if(!hrp.strThirdPartyID.IsEmpty()){
								// and if we have some form of 3rd party ID, we can link with it
								CreateHL7IDLinkRecord(GetRemoteData(), message.nHL7GroupID, hrp.strThirdPartyID, nNewRefPhysID, hilrtReferringPhysician, hrp.strFirst, hrp.strLast, "");

								// audit the changes
								CString strOld;
								strOld.Format("Referring Physician Code '%s' (HL7 Group %s)", hrp.strThirdPartyID, message.strHL7GroupName);
								CString strNew;
								strNew.Format("Linked to %s, %s %s", hrp.strLast, hrp.strFirst, hrp.strMiddle);
								long nAuditID = BeginNewAuditEvent();
								AuditEvent(-1, "", nAuditID, aeiHL7ReferringPhysicianLink, -1, strOld, strNew, aepMedium);
							}

							nRefPhysID = nNewRefPhysID;	// we now want to import with this Ref Phys
							CClient::RefreshTable(NetUtils::RefPhys, nRefPhysID);
						}
					}
					break;
				case hmrNoMatchFound:	// no match
					{
						// well we dont have the referring phys
						if(IDNO == MsgBox(MB_YESNO, "This message is attempting to update a referring physician '%s %s' that could not be found "
							"in Practice.  Would you like to create this referring physician?\n"
							"If Yes, the referring physician will be created.  If No, the import will be cancelled.", hrp.strFirst, hrp.strLast)) {
								// if the user said no, we dont let them import
								return FALSE;
						}
						
						// user approved a new ref phys 
						//TES 4/22/2015 - PLID 61147 - Added support for the prefix
						CSqlFragment sql("DECLARE @nRefPhysPrefixID int;");
						if (hrp.strPrefix != "") {
							sql += CSqlFragment("SELECT @nRefPhysPrefixID = ID FROM PrefixT WHERE Prefix = {STRING}; "
								"IF @nRefPhysPrefixID Is Null BEGIN "
								"	SELECT @nRefPhysPrefixID = COALESCE(Max(ID),0)+1 FROM PrefixT; "
								"	INSERT INTO PrefixT (ID, Prefix, Gender, InformID) "
								"	VALUES (@nRefPhysPrefixID, {STRING}, 0, 0) "
								"END", hrp.strPrefix, hrp.strPrefix);
						}
						sql += CSqlFragment(
							"DECLARE @nRefPhysID INT;"
							"SET @nRefPhysID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM PersonT);"
							"INSERT INTO PersonT (ID, First, Middle, Last, Title, "
							"WorkPhone, Address1, Address2, City, State, Zip, PrefixID) "
							"VALUES (@nRefPhysID, {STRING}, {STRING}, {STRING}, {STRING}, "
							"{STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, @nRefPhysPrefixID); \r\n"
							"INSERT INTO ReferringPhysT (PersonID) VALUES (@nRefPhysID); \r\n",
							hrp.strFirst, hrp.strMiddle, hrp.strLast, hrp.strDegree,
							FormatPhoneForSql(hrp.strPhone), hrp.strAddress1, hrp.strAddress2, hrp.strCity, hrp.strState, hrp.strZip
						);

						// if we have a 3rd party ID, then we can link it while we are at it
						if(!hrp.strThirdPartyID.IsEmpty()){
							// Eliminate the possibility of duplicate links by deleting before inserting.
							sql += GetDeleteHL7IDLinkSql(message.nHL7GroupID, hilrtReferringPhysician, hrp.strThirdPartyID);
							sql += GetCreateHL7IDLinkSql(message.nHL7GroupID, hilrtReferringPhysician, hrp.strThirdPartyID, "", "@nRefPhysID", hrp.strFirst, hrp.strLast);
						}

						sql += "SELECT @nRefPhysID AS RefPhysID; \r\n";

						_RecordsetPtr rsRefPhys = CreateParamRecordset(
							"BEGIN TRAN; SET NOCOUNT ON; {SQL} SET NOCOUNT OFF; COMMIT TRAN;", sql);

						nRefPhysID = AdoFldLong(rsRefPhys, "RefPhysID", -1);

						CAuditTransaction auditTrans;
						AuditEvent(-1, hrp.strLast + ", " + hrp.strFirst + " " + hrp.strMiddle, auditTrans, aeiRefPhysCreated, -1, "", hrp.strLast + ", " + hrp.strFirst + " " + hrp.strMiddle, aepMedium);

						// if we had a 3rd party ID, we linked it, so audit that
						if(!hrp.strThirdPartyID.IsEmpty()){
							CString strOld;
							strOld.Format("Referring Physician Code '%s' (HL7 Group %s)", hrp.strThirdPartyID, message.strHL7GroupName);
							CString strNew;
							strNew.Format("Linked to %s, %s %s", hrp.strLast, hrp.strFirst, hrp.strMiddle);
							long nAuditID = BeginNewAuditEvent();
							AuditEvent(-1, "", auditTrans, aeiHL7ReferringPhysicianLink, -1, strOld, strNew, aepMedium);
						}

						auditTrans.Commit();
						CClient::RefreshTable(NetUtils::RefPhys, nRefPhysID);
					}
					break;
				default:
					// well we didnt expect that value
					AfxThrowNxException("Invalid HL7MatchResult result %i returned in HandleHL7SchedulerMessage()", hrp);
					break;
			}
		}
	}

	// (z.manning 2010-07-01 11:04) - PLID 39422 - Now go through all the resources in the message and
	// map them to a NexTech resource.
	CArray<long,long> arynResourceIDs;
	for(int nResourceIndex = 0; nResourceIndex < hl7appt.aryResources.GetSize();  nResourceIndex++) {
		HL7_Resource hl7resource = hl7appt.aryResources.GetAt(nResourceIndex);
		long nResourceID = GetNextechResource(message, hl7resource.strHL7ID, hl7resource.strName);
		if(nResourceID == -1) {
			// (z.manning 2010-07-01 11:05) - PLID 39422 - We were unable to map to a NexTech resource so
			// we can't continue.
			return FALSE;
		}
		if(!IsIDInArray(nResourceID, arynResourceIDs)) {
			arynResourceIDs.Add(nResourceID);
		}
	}

	CString strEvent;
	GetHL7MessageComponent(message.strMessage, message.nHL7GroupID, "MSH", 1, 9, 1, 2, strEvent);
	
	BYTE nStatus;
	long nShowState;
	GetStatusAndShowStateFromStatusText(hl7appt.strStatusCode, nStatus, nShowState);

	CString strError;
	CAuditTransaction auditTran;
	CArray<CommonAuditData,CommonAuditData&> aryAuditData;

	// (d.singleton 2012-08-08 15:50) - PLID 51938 import, update, assign ins party per appt
	CNxParamSqlArray aryInsPartyParams;
	CString strInsPartySql = BeginSqlBatch();
	// (r.gonet 05/14/2013) - PLID 56675 - Added an output variable to tell us if the insurance info has been changed.
	bool bInsuranceChanged = false;
	if(!HandleInsuredParty(message, GetExistingPatientName(nPatientID), nPatientID, nApptID, auditTran, bInsuranceChanged, strInsPartySql, strError, TRUE, aryInsPartyParams)) {
		AfxMessageBox("Failed to import message, could not match insurance info");
		return FALSE;
	}

	CString strApptInfo = FormatString("%s, %s at %s\r\n%s", hl7appt.PID.strLast, hl7appt.PID.strFirst, FormatDateTimeForInterface(hl7appt.dtStart,DTF_STRIP_SECONDS), hl7appt.GetNotes());
	BOOL bCreateAppt = FALSE, bCancelAppt = FALSE;
	if(strEvent == "S12") // New appointment
	{
		if(nApptID != -1) {
			// (z.manning 2010-07-07 09:57) - PLID 39422 - They are trying to create a new appointment but this appt
			// ID already exists. We can't safely import this appointment.
			AfxMessageBox("The following new appointment...\r\n\r\n" + strApptInfo + "\r\n\r\n...matches an existing appointment in NexTech and cannot be imported.", MB_OK|MB_ICONERROR);
			return FALSE;
		}

		bCreateAppt = TRUE;
	}
	else if(strEvent == "S13"	// Appointment rescheduled
		 || strEvent == "S14")	// Appointment modified
	{
		if(nApptID == -1) {
			if(strEvent == "S13") {
				// (z.manning 2010-08-11 17:48) - PLID 39422 - S13 are rescheduled appointments which per the HL7 specs means
				// the old appt may be cancelled and a new one created. So let's allow these to be silently created.
				bCreateAppt = TRUE;
			}
			else {
				// (z.manning 2010-07-07 10:21) - PLID 39422 - We don't have a matching appointment. Prompt to create a new one
				int nResult = AfxMessageBox("Could not update the following appointment...\r\n\r\n" + strApptInfo + "\r\n\r\n...because it does not exist in NexTech. Would you like to create a new appointment?", MB_YESNO|MB_ICONQUESTION);
				if(nResult != IDYES) {
					return FALSE;
				}
				else {
					bCreateAppt = TRUE;
				}
			}
		}
		else {
			// (r.gonet 05/14/2013) - PLID 56675 - Tell the update function that the insurance needs saved (so it doesn't short circuit)
			bSuccess = UpdateHL7Appointment(GetRemoteData(), message, nApptID, nPatientID, nLocationID, nRefPhysID, arynResourceIDs, bInsuranceChanged, aryAuditData, strError, strInsPartySql, aryInsPartyParams);
		}
	}
	else if(strEvent == "S15"	// Appointment cancelled
		 || strEvent == "S17")	// Appointment deleted
	{
		if(nApptID == -1) {
			// (z.manning 2010-07-07 10:47) - PLID 39422 - They're trying to cancel an appointment that does not yet exist.
			// Let's silently create it and then we'll cancel it.
			bCreateAppt = TRUE;
		}
		bCancelAppt = TRUE;
	}

	if(bCreateAppt) {
		nApptID = CreateHL7Appointment(GetRemoteData(), message, nPatientID, nLocationID, nRefPhysID, arynResourceIDs, aryAuditData, strError, strInsPartySql, aryInsPartyParams);
		if(nApptID != -1) {
			bSuccess = TRUE;
			// (z.manning 2010-07-01 17:32) - PLID 39422 - Remember the link between the 3rd party appt and the NexTech
			// appt we just created.
			// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
			ExecuteParamSql("{SQL}", CreateNewHL7CodeLinkT(message.nHL7GroupID, hclrtAppointment, hl7appt.strHL7ID, nApptID));

			CString strOld, strNew;
			strOld.Format("Appiontment Code '%s' (HL7 Group '%s')", hl7appt.strHL7ID, message.strHL7GroupName);
			strNew.Format("Linked to %s", strApptInfo);
			AuditEvent(-1, "", auditTran, aeiHL7AppointmentLink, message.nHL7GroupID, strOld, strNew, aepLow, aetCreated);
		}
	}

	if(bCancelAppt && nApptID != -1) {
		bSuccess = CancelHL7Appointment(GetRemoteData(), message, nApptID, nPatientID, nLocationID, nRefPhysID, arynResourceIDs, aryAuditData, strError, strInsPartySql, aryInsPartyParams);
		nStatus = 4;
	}

	if(bSuccess)
	{
		// (z.manning 2010-07-01 14:25) - PLID 39422 - Send a table checker
		// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID
		CString strResourceIDs = ArrayAsString(arynResourceIDs);
		//this is space-delimited
		strResourceIDs.Replace(",", " ");
		CClient::RefreshAppointmentTable(nApptID, nPatientID, hl7appt.dtStart, hl7appt.GetEndTime(), nStatus, nShowState, nLocationID, strResourceIDs);

		for(int nAuditIndex = 0; nAuditIndex < aryAuditData.GetSize(); nAuditIndex++) {
			CommonAuditData auditData = aryAuditData.GetAt(nAuditIndex);
			AuditEvent(nPatientID, GetExistingPatientName(nPatientID), auditTran, auditData.eEvent, nApptID, auditData.strOld, auditData.strNew, aepMedium, auditData.eType);
		}

		CNxParamSqlArray aryParams;
		CString strSql = BeginSqlBatch();
		//TES 7/17/2010 - PLID 39518 - FinalizeHL7Event() now takes a parameter to the AuditID
		long nAuditID = auditTran;
		//TES 6/10/2011 - PLID 41353 - The appointment was already created, so there's no output variable here
		FinalizeHL7Event(nPatientID, message, strSql, TRUE, &aryParams, &nAuditID);

		PPCModifyAppt(nApptID);

		// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
		GetMainFrame()->HandleRecallChanged();
	}
	else {
		CString strMessage = "Failed to import the following appointment...\r\n" + strApptInfo;
		if(!strError.IsEmpty()) {
			strMessage += "\r\n\r\ndue to the following reason...\r\n" + strError;
		}
		AfxMessageBox(strMessage);
	}

	auditTran.Commit();

	return bSuccess;
}

// (d.singleton 2012-08-08 11:19) - PLID 51938 use apptid and message to assign , create, update insured parties per appt
//  most of the code has been copied from importing patient code
// (r.gonet 05/14/2013) - PLID 56675 - Added an output variable to tell us if the insurance info has been changed.
BOOL HandleInsuredParty(const HL7Message &Message, CString strPersonName, long nPersonID, long nApptID, long nAuditID, OUT bool &bInsuranceChanged, CString &strSql, CString &strError, BOOL bIsAppt, IN OUT CNxParamSqlArray &aryParams)
{	
	try {
		HL7_PIDFields hl7pID;
		ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, hl7pID, GetRemoteData());
		CArray<HL7_InsuranceFields, HL7_InsuranceFields&> arInsuranceSegments;
		// (r.gonet 05/14/2013) - PLID 56675 - Retain the old insurance so we can compare and see if the insurance is changed.
		CArray<long, long> arOldApptInsuredParties;
		bInsuranceChanged = false;
		ParseInsuranceSegments(Message.strMessage, Message.nHL7GroupID, arInsuranceSegments);

		if(bIsAppt) {
			// (r.gonet 05/14/2013) - PLID 56675 - Store in memory the old insurance values, if they exist.
			_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(), 
				"SELECT InsuredPartyID, Placement "
				"FROM AppointmentInsuredPartyT "
				"WHERE AppointmentID = {INT} "
				"ORDER BY Placement ASC",
				nApptID);
			while(!prs->eof) {
				arOldApptInsuredParties.Add(AdoFldLong(prs->Fields, "InsuredPartyID"));
				prs->MoveNext();
			}

			// (d.singleton 2012-08-16 11:51) - PLID 51938 add declarations only if this is an appt hl7 message
			// (d.singleton 2012-10-09 11:44) - PLID 53032 added variable for resptype priority
			AddStatementToSqlBatch(strSql, "DECLARE @nPriority INT");
			AddStatementToSqlBatch(strSql, "DECLARE @nNextPersonID INT");
			AddStatementToSqlBatch(strSql, "SELECT @nNextPersonID = COALESCE(Max(ID),0)+1 FROM PersonT");
			AddStatementToSqlBatch(strSql, "DECLARE @nNextInsPlanID INT");
			AddStatementToSqlBatch(strSql, "SELECT @nNextInsPlanID = COALESCE(Max(ID),0)+1 FROM InsurancePlansT");
			AddStatementToSqlBatch(strSql, "DECLARE @nPatientID INT");
			AddStatementToSqlBatch(strSql, "SET @nPatientID = @nNextPersonID");
			AddStatementToSqlBatch(strSql, "SET @nNextPersonID = @nNextPersonID+1");
			AddStatementToSqlBatch(strSql, "DECLARE @strExistingApptIns nvarchar(150)");
			AddStatementToSqlBatch(strSql, "DECLARE @ApptInsuredPartyAuditInfo TABLE(OldName nvarchar(150), NewName nvarchar(150), Type INT)");
			AddStatementToSqlBatch(strSql, "DECLARE @b_DefaultDeductiblePerPayGroup INT");
			AddStatementToSqlBatch(strSql, "DECLARE @money_InsCoDefTotalDeductible MONEY");
			AddStatementToSqlBatch(strSql, "DECLARE @money_InsCoDefTotalOOP MONEY");

			// (r.gonet 05/14/2013) - PLID 56675 - Clear out the existing insurance associated with this appointment. Save the names in SQL first though so we can audit.
			AddStatementToSqlBatch(strSql, "DECLARE @OldAppointmentInsuredPartyT TABLE (Placement INT, OldName NVARCHAR(150))");
			AddStatementToSqlBatch(strSql, 
				"INSERT INTO @OldAppointmentInsuredPartyT (Placement, OldName)\r\n"
				"SELECT AppointmentInsuredPartyT.Placement, InsuranceCoT.Name\r\n"
				"FROM AppointmentInsuredPartyT\r\n"
				"LEFT JOIN InsuredPartyT ON AppointmentInsuredPartyT.InsuredPartyID = InsuredPartyT.PersonID\r\n"
				"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID\r\n"
				"WHERE AppointmentInsuredPartyT.AppointmentID = @nApptID");
			AddStatementToSqlBatch(strSql, "DELETE FROM AppointmentInsuredPartyT WHERE AppointmentID = @nApptID");
		}

		if(!AssignInsCoIDs(hl7pID, nPersonID, arInsuranceSegments, Message.nHL7GroupID, nAuditID, strSql)) {
			strError == "Could not map insurance info";
			return FALSE;
		}
		//TES 7/15/2010 - PLID 39518 - We're going to need all the RespType names for auditing, grab them all now.
		CMapStringToString mapRespTypes;	
		if(arInsuranceSegments.GetSize()) {
			//TES 8/11/2010 - PLID 39518 - Map the Priorities, not the IDs
			_RecordsetPtr rsRespTypes = CreateParamRecordset("SELECT Priority, TypeName FROM RespTypeT");
			while(!rsRespTypes->eof) {
				mapRespTypes.SetAt(AsString(AdoFldLong(rsRespTypes, "Priority")), AdoFldString(rsRespTypes, "TypeName"));
				rsRespTypes->MoveNext();
			}
		}
		// (d.singleton 2012-08-27 10:49) - PLID 51938 need to grab the ins set numbers and map to the nextech ins types, default to medical
		CMapStringToString mapInsSetNumbers;
		if(arInsuranceSegments.GetSize()) {
			_RecordsetPtr rsInsSetNumbers = CreateParamRecordset("SELECT ThirdPartyCode, CONVERT(NVARCHAR,PracticeID) AS PracticeID FROM HL7CodeLinkT WHERE TYPE = {INT} AND HL7GroupID = {INT}", hclrtInsuranceCategoryType, Message.nHL7GroupID);
			while(!rsInsSetNumbers->eof) {
				mapInsSetNumbers.SetAt(AdoFldString(rsInsSetNumbers, "ThirdPartyCode", ""), AdoFldString(rsInsSetNumbers, "PracticeID", "1"));
				rsInsSetNumbers->MoveNext();
			}
		}

		//TES 7/19/2010 - PLID 39518 - We need to track any HL7 Relation mappings we prompt the user for, so that if they have the same
		// unmapped code twice in the same message, they're only prompted for it once.
		CMapStringToString mapUserMappedRelations;

		//TES 7/15/2010 - PLID 39518 - Track which segments we've mapped to an existing InsuredParty
		CArray<long,long> arMappedInsPartyIDs;
		for(int i = 0; i < arInsuranceSegments.GetSize(); i++) {
			arMappedInsPartyIDs.Add(-1);
		}

		//TES 7/15/2010 - PLID 39518 - Now, go through all our existing insured parties, and update them appropriately.
		// (j.gruber 2010-08-03 16:35) - PLID 39977 - changed insurance structure
		//TES 8/11/2010 - PLID 39518 - Pull the RespType Priority, not the RespTypeID
		_RecordsetPtr rsExistingParties = CreateParamRecordset("SELECT InsuredPartyT.PersonID, InsuredPartyT.InsuranceCoID, "
			"InsuredPartyT.InsuranceContactID, InsuredPartyT.InsPlan, InsuredPartyT.PolicyGroupNum, InsuredPartyT.IDForInsurance, "
			"InsuredPartyT.EffectiveDate, InsuredPartyT.ExpireDate, InsuredPartyT.RelationToPatient, RespTypeT.Priority AS RespTypePriority, "
			"InsuredPartyT.Employer, InsuredPartyT.CoPay,InsuredPartyT.CopayPercent, InsuredPartyT.PayGroupID, "
			"PersonT.First, PersonT.Middle, PersonT.Last, PersonT.BirthDate, PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, "
			"PersonT.Zip, PersonT.SocialSecurity, PersonT.HomePhone, PersonT.Gender, "
			"ContactPerson.First + ' ' + ContactPerson.Last AS ContactName, InsurancePlansT.PlanName "
			"FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay, PayGroupID FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage, ServicePayGroupsT.ID As PayGroupID FROM "
			"(SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
			" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
			" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID ) InsuredPartyT INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"INNER JOIN PersonT ContactPerson ON InsuredPartyT.InsuranceContactID = ContactPerson.ID "
			"INNER JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PatientID = {INT} "
			"ORDER BY CASE WHEN RespTypeT.Priority = -1 THEN 99 ELSE RespTypeT.Priority END ASC", nPersonID);
		while(!rsExistingParties->eof) {
			//TES 7/15/2010 - PLID 39518 - Get the company ID and placement.
			long nInsCoID = AdoFldLong(rsExistingParties, "InsuranceCoID");
			long nPartyID = AdoFldLong(rsExistingParties, "PersonID");
			long nPlacement = AdoFldLong(rsExistingParties, "RespTypePriority");
			//TES 7/15/2010 - PLID 39518 - Now, see if any incoming insurances have this company, and if so, see where they're placed.
			bool bMapped = false;
			// (d.singleton 2012-10-09 14:04) - PLID 53032 need to keep track of how many resptypeT records we have added
			long nAddedRespType = 0;
			//(j.camacho 2013-07-24 17:27) - PLID 57597 - Prevent duplicate entries if there are two insurances with the same company from being entered
			for(int i = 0; i < arInsuranceSegments.GetSize() && !bMapped; i++) {
				//TES 7/15/2010 - PLID 39518 - Only check segments we haven't already mapped.
				if(arMappedInsPartyIDs[i] == -1) {
					HL7_InsuranceFields Insurance = arInsuranceSegments[i];
					if(Insurance.nNextechInsCoID == nInsCoID ) {
						//TES 7/15/2010 - PLID 39518 - OK, we got one, let's updated it with the HL7 information.
						arMappedInsPartyIDs.SetAt(i, nPartyID);
						bMapped = true;	

						// (d.singleton 2012-08-27 12:03) - PLID 51938 grab the resptypet.categorytype and cross check the ones on the message with the matched insured party
						CString strInsCategoryType;
						long nInsCategoryType;
						if(!mapInsSetNumbers.Lookup(Insurance.strInsSetNumber, strInsCategoryType)) {
							strInsCategoryType = "1";
						}
						nInsCategoryType = atoi(strInsCategoryType);

						//see if a RespType record already exists
						// (d.singleton 2012-09-17 16:06) - PLID 51938 had to change the way the below code worked as we now accept an insurance category type
						_RecordsetPtr prsPriority = CreateParamRecordset("SELECT Priority FROM RespTypeT WHERE CategoryType = {INT} AND CategoryPlacement = {INT};\r\n"
							"SELECT Max(Priority) + 1 AS NextPriority FROM RespTypeT ", nInsCategoryType, i+1);
						long nPriority; 
						if(!prsPriority->eof) {
							nPriority = AdoFldLong(prsPriority, "Priority");	
							AddStatementToSqlBatch(strSql, "SET @nPriority = %li", nPriority);
						}
						else {
							// (d.singleton 2012-10-04 12:03) - PLID 53032 added a sql param to track priority
							prsPriority = prsPriority->NextRecordset(NULL);
							nPriority = AdoFldLong(prsPriority, "NextPriority");
							//TES 7/20/2010 - PLID 39518 - Ensure the RespType exists.
							//TES 8/11/2010 - PLID 39518 - Ensure the Priority, not the ID.
							AddStatementToSqlBatch(strSql, "SET @nPriority = (SELECT MAX(Priority) + 1 FROM RespTypeT)");
							AddStatementToSqlBatch(strSql, "INSERT INTO RespTypeT (ID, TypeName, Priority, CategoryType, CategoryPlacement) "
								"SELECT (SELECT Max(ID)+1 FROM RespTypeT), %li, @nPriority, %li, %li",
								i+1, nInsCategoryType, i+1);
							
							nPriority += nAddedRespType;
							nAddedRespType++;
						}
						if(nPlacement != nPriority) {
							// (r.gonet 06/10/2013) - PLID 57105 - update the insuredparty.
							// (d.singleton 2012-10-04 12:03) - PLID 53032 added a sql param to track priority
							AddStatementToSqlBatch(strSql, "UPDATE InsuredPartyT SET RespTypeID = (SELECT ID FROM RespTypeT WHERE Priority = @nPriority) WHERE PersonID = %li", nPartyID);

							// (r.gonet 06/10/2013) - PLID 57105 - Make sure that the priority change gets saved.
							bInsuranceChanged = true;

							CString strOldName;
							if(!mapRespTypes.Lookup(AsString(nPlacement), strOldName)) {
								//TES 7/15/2010 - PLID 39518 - This must have just been created.
								strOldName = AsString(nPlacement);
							}
							CString strNewName;
							if(!mapRespTypes.Lookup(AsString((long)i+1), strNewName)) {
								strNewName = AsString((long)i+1);
							}
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsuredPartyInsType, nPartyID, strOldName, strNewName, aepMedium);
						}

						CString strUpdateInsParty = "UPDATE InsuredPartyT SET ";
						//TES 7/15/2010 - PLID 39518 - Now, update the rest of the fields.
						//InsuranceContactID
						long nExistingContactID = AdoFldLong(rsExistingParties, "InsuranceContactID");
						if(nExistingContactID != Insurance.nNextechInsContactID) {
							CString strExistingContactName = AdoFldString(rsExistingParties, "ContactName");
							strUpdateInsParty += FormatString("InsuranceContactID = %li, ", Insurance.nNextechInsContactID);
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyContact, nPartyID, strExistingContactName, Insurance.strNextechInsContactName, aepMedium);
						}

						//InsPlan
						long nExistingPlanID = AdoFldLong(rsExistingParties, "InsPlan");
						if(nExistingPlanID != Insurance.nNextechInsPlanID) {
							strUpdateInsParty += FormatString("InsPlan = %li, ", Insurance.nNextechInsPlanID);

							//Changing the plan isn't audited
						}

						//PolicyGroupNum
						CString strGroupNum = AdoFldString(rsExistingParties, "PolicyGroupNum");
						if(!Insurance.strGroupNum.IsEmpty() && strGroupNum != Insurance.strGroupNum) {
							strUpdateInsParty += FormatString("PolicyGroupNum = '%s', ", _Q(Insurance.strGroupNum));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyPolicyGroupNum, nPartyID, strGroupNum, Insurance.strGroupNum, aepMedium);
						}

						//IDForInsurance
						CString strInsID = AdoFldString(rsExistingParties, "IDForInsurance");
						if(!Insurance.strIDForInsurance.IsEmpty() && strInsID != Insurance.strIDForInsurance) {
							strUpdateInsParty += FormatString("IDForInsurance = '%s', ", _Q(Insurance.strIDForInsurance));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyIDForIns, nPartyID, strInsID, Insurance.strIDForInsurance, aepMedium);
						}

						//EffectiveDate
						COleDateTime dtEffective;
						_variant_t varEffective = rsExistingParties->Fields->GetItem("EffectiveDate")->Value;
						if(varEffective.vt == VT_DATE) {
							dtEffective = VarDateTime(varEffective);
						}
						else {
							dtEffective.SetStatus(COleDateTime::invalid);
						}
						if(Insurance.dtEffective.GetStatus() == COleDateTime::valid && dtEffective != Insurance.dtEffective) {
							strUpdateInsParty += FormatString("EffectiveDate = '%s', ", _Q(FormatDateTimeForSql(Insurance.dtEffective)));
							CString strOldEffective;
							if(dtEffective.GetStatus()) {
								strOldEffective = FormatDateTimeForInterface(dtEffective);
							}
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyEffectiveDate, nPartyID, strOldEffective, FormatDateTimeForInterface(Insurance.dtEffective), aepMedium);
						}

						//ExpireDate
						COleDateTime dtExpire;
						_variant_t varExpire = rsExistingParties->Fields->GetItem("ExpireDate")->GetValue();
						if(varExpire.vt == VT_DATE) {
							dtExpire = VarDateTime(varExpire);
						}
						else {
							dtExpire.SetStatus(COleDateTime::invalid);
						}
						if(Insurance.dtExpires.GetStatus() == COleDateTime::valid && dtExpire != Insurance.dtExpires) {
							strUpdateInsParty += FormatString("ExpireDate = '%s', ", _Q(FormatDateTimeForSql(Insurance.dtExpires)));
							CString strOldExpire;
							if(dtExpire.GetStatus()) {
								strOldExpire = FormatDateTimeForInterface(dtExpire);
							}
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyInactiveDate, nPartyID, strOldExpire, FormatDateTimeForInterface(Insurance.dtExpires), aepMedium);
						}

						//RelationToPatient
						CString strRelation;
						//TES 7/19/2010 - PLID 39610 - First check whether the user's mapped this relation already.
						if(!mapUserMappedRelations.Lookup(Insurance.strInsPartyRelation, strRelation)) {
							if(!GetHL7Relation(GetRemoteData(), Insurance.strInsPartyRelation, strRelation, Message.nHL7GroupID)) {
								if(!PromptForHL7Relation(Insurance.strInsPartyRelation, Message.nHL7GroupID, Message.strHL7GroupName, strRelation, nAuditID, strSql)) {
									return FALSE;
								}
								else {
									//TES 7/19/2010 - PLID 39610 - Remember what the user chose.
									mapUserMappedRelations.SetAt(Insurance.strInsPartyRelation, strRelation);
								}
							}
						}
						CString strExistingRelation = AdoFldString(rsExistingParties, "RelationToPatient");
						if(strRelation != strExistingRelation) {
							strUpdateInsParty += FormatString("RelationToPatient = '%s', ", _Q(strRelation));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyRelation, nPartyID, strExistingRelation, strRelation, aepMedium);
						}

						//Employer
						CString strEmployer = AdoFldString(rsExistingParties, "Employer");
						if(!Insurance.strEmployer.IsEmpty() && strEmployer != Insurance.strEmployer) {
							strUpdateInsParty += FormatString("Employer = '%s', ", _Q(Insurance.strEmployer));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyEmployer, nPartyID, strEmployer, Insurance.strEmployer, aepMedium);
						}


						//CoPay						
						if(!Insurance.strCoPay.IsEmpty()) {
							CString strPayGroupSql;
							// (j.gruber 2010-08-04 08:37) - PLID 39977 - change to new copay structure
							_variant_t varCopay, varPercent;
							varCopay = rsExistingParties->Fields->Item["Copay"]->Value;
							varPercent = rsExistingParties->Fields->Item["CopayPercent"]->Value;
							COleCurrency cyNewCoPay;
							cyNewCoPay.ParseCurrency(Insurance.strCoPay);
							long nPayGroupID = AdoFldLong(rsExistingParties, "PayGroupID", -1);							

							enum CopayStatus {
								csCurrency = 0,
								csPercent,
								csOther,
							};

							BOOL bCurrencyChanged = FALSE;
							long nPercent = -1;
							CopayStatus csStatus;
							if (varCopay.vt == VT_CY) {
								csStatus = csCurrency;
								if (VarCurrency(varCopay) != cyNewCoPay) {
									bCurrencyChanged = TRUE;
								}
							}
							else if (varPercent.vt == VT_I4) {
								//they have a percent, so we'll need to change it
								nPercent = VarLong(varPercent);
								csStatus = csPercent;
							}
							else {
								//its null or invalid, so we are just adding
								csStatus = csOther;
							}														

							if(cyNewCoPay.GetStatus() == COleCurrency::valid && (csStatus == csPercent || csStatus == csOther || bCurrencyChanged)) {

								// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
								RoundCurrency(cyNewCoPay);

								//CoPayType
								if(csStatus == csPercent) {
									strPayGroupSql.Format("UPDATE InsuredPartyPayGroupsT SET CopayPercentage = NULL, CopayMoney = convert(money, '%s') WHERE InsuredPartyID = %li AND PayGroupID = %li",
										FormatCurrencyForSql(cyNewCoPay), nPartyID, nPayGroupID);
									AddStatementToSqlBatch(strSql, strPayGroupSql);
									AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyPayGroupCopayPercent, nPartyID, "Pay Group: Copay Value: " + AsString(nPercent), "Pay Group: Copay Value: ", aepMedium);
									AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyPayGroupCopayMoney, nPartyID, "Pay Group: Copay Value: ", "Pay Group: Copay Value: " + FormatCurrencyForInterface(cyNewCoPay), aepMedium);
								}
								else if (csStatus == csOther) {
									//there is no copay, so the record might exist, but it might not
									strPayGroupSql.Format(" DECLARE @nInsPartyID INT; \r\n"
										" DECLARE @nPayGroupID INT; \r\n "
										" DECLARE @cyCopay money; \r\n "
										" SET @nInsPartyID = %li; \r\n "
										" SET @nPayGroupID = %li; \r\n "
										" IF @nPayGroupID = -1 BEGIN \r\n "
										" SET @nPayGroupID = (SELECT ID FROM ServicePayGroupsT WHERE Name = 'Copay'); \r\n "
										" END \r\n "
										" SET @cyCopay = CONVERT(money, '%s'); \r\n "
										" DECLARE @nInsPartyPayGroupID INT; \r\n "
										" SET @nInsPartyPayGroupID = (SELECT ID FROM InsuredPartyPayGroupsT WHERE InsuredPartyID = @nInsPartyID AND PayGroupID = @nPayGroupID); \r\n "
										"  \r\n "
										" IF @nInsPartyPayGroupID IS NULL BEGIN \r\n "										
										" 	INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PayGroupID, CopayMoney) VALUES (@nInsPartyID, @nPayGroupID, @cyCopay) \r\n "
										" END \r\n "
										" ELSE BEGIN \r\n "
										" 	UPDATE InsuredPartyPayGroupsT SET CopayMoney = @cyCopay WHERE ID = @nInsPartyPayGroupID \r\n "
										" END", nPartyID, nPayGroupID, FormatCurrencyForSql(cyNewCoPay) );
									AddStatementToSqlBatch(strSql, strPayGroupSql);
									AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyPayGroupCopayMoney, nPartyID, "Pay Group: Copay Value: " + AsString(varCopay), "Pay Group: Copay Value: " + FormatCurrencyForInterface(cyNewCoPay), aepMedium);

								}
								else {
									//the copay changed
									strPayGroupSql.Format("UPDATE InsuredPartyPayGroupsT SET CopayMoney = convert(money, '%s') WHERE InsuredPartyID = %li AND PayGroupID = %li", FormatCurrencyForSql(cyNewCoPay), nPartyID, nPayGroupID);									
									AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyPayGroupCopayMoney, nPartyID, "Pay Group: Copay Value: " + AsString(varCopay), "Pay Group: Copay Value: " + FormatCurrencyForInterface(cyNewCoPay), aepMedium);
									AddStatementToSqlBatch(strSql, strPayGroupSql);
								}
							}
						}

						//TES 7/15/2010 - PLID 39518 - OK, did we actually update anything?
						if(strUpdateInsParty != "UPDATE InsuredPartyT SET ") {
							//TES 7/15/2010 - PLID 39518 - Trim the last comma
							strUpdateInsParty = strUpdateInsParty.Left(strUpdateInsParty.GetLength()-2);
							strUpdateInsParty += FormatString(" WHERE PersonID = %li", nPartyID);
							AddStatementToSqlBatch(strSql, strUpdateInsParty);
						}

						//TES 7/15/2010 - PLID 39518 - Now for PersonT
						CString strUpdatePerson = "UPDATE PersonT SET ";

						//First
						CString strFirst = AdoFldString(rsExistingParties, "First");
						if(!Insurance.strInsPartyFirst.IsEmpty() && strFirst != Insurance.strInsPartyFirst) {
							strUpdatePerson += FormatString("First = '%s', ", _Q(Insurance.strInsPartyFirst));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyFirst, nPartyID, strFirst, Insurance.strInsPartyFirst, aepMedium);
						}

						//Middle
						CString strMiddle = AdoFldString(rsExistingParties, "Middle");
						if(!Insurance.strInsPartyMiddle.IsEmpty() && strMiddle != Insurance.strInsPartyMiddle) {
							strUpdatePerson += FormatString("Middle = '%s', ", _Q(Insurance.strInsPartyMiddle));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyMiddle, nPartyID, strMiddle, Insurance.strInsPartyMiddle, aepMedium);
						}

						//Last
						CString strLast = AdoFldString(rsExistingParties, "Last");
						if(!Insurance.strInsPartyLast.IsEmpty() && strLast != Insurance.strInsPartyLast) {
							strUpdatePerson += FormatString("Last = '%s', ", _Q(Insurance.strInsPartyLast));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyLast, nPartyID, strLast, Insurance.strInsPartyLast, aepMedium);
						}

						//BirthDate
						COleDateTime dtBirth;
						_variant_t varBirth = rsExistingParties->Fields->GetItem("BirthDate")->GetValue();
						if(varBirth.vt == VT_DATE) {
							dtBirth = VarDateTime(varBirth);
						}
						else {
							dtBirth.SetStatus(COleDateTime::invalid);
						}
						if(Insurance.dtInsPartyBirthDate.GetStatus() == COleDateTime::valid && dtBirth != Insurance.dtInsPartyBirthDate) {
							strUpdatePerson += FormatString("BirthDate = '%s', ", _Q(FormatDateTimeForSql(Insurance.dtInsPartyBirthDate)));
							CString strOldBirth;
							if(dtBirth.GetStatus()) {
								strOldBirth = FormatDateTimeForInterface(dtBirth);
							}
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyDateofBirth, nPartyID, strOldBirth, FormatDateTimeForInterface(Insurance.dtInsPartyBirthDate), aepMedium);
						}

						//Address1
						CString strAddress1 = AdoFldString(rsExistingParties, "Address1");
						if(!Insurance.strInsPartyAddress1.IsEmpty() && strAddress1 != Insurance.strInsPartyAddress1) {
							strUpdatePerson += FormatString("Address1 = '%s', ", _Q(Insurance.strInsPartyAddress1));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyAddress1, nPartyID, strAddress1, Insurance.strInsPartyAddress1, aepMedium);
						}

						//Address2
						CString strAddress2 = AdoFldString(rsExistingParties, "Address2");
						if(!Insurance.strInsPartyAddress2.IsEmpty() && strAddress2 != Insurance.strInsPartyAddress2) {
							strUpdatePerson += FormatString("Address2 = '%s', ", _Q(Insurance.strInsPartyAddress2));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyAddress2, nPartyID, strAddress2, Insurance.strInsPartyAddress2, aepMedium);
						}

						//City
						CString strCity = AdoFldString(rsExistingParties, "City");
						if(!Insurance.strInsPartyCity.IsEmpty() && strCity != Insurance.strInsPartyCity) {
							strUpdatePerson += FormatString("City = '%s', ", _Q(Insurance.strInsPartyCity));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyCity, nPartyID, strCity, Insurance.strInsPartyCity, aepMedium);
						}

						//State
						CString strState = AdoFldString(rsExistingParties, "State");
						if(!Insurance.strInsPartyState.IsEmpty() && strState != Insurance.strInsPartyState) {
							strUpdatePerson += FormatString("State = '%s', ", _Q(Insurance.strInsPartyState));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyState, nPartyID, strState, Insurance.strInsPartyState, aepMedium);
						}

						//Zip
						CString strZip = AdoFldString(rsExistingParties, "Zip");
						if(!Insurance.strInsPartyZip.IsEmpty() && strZip != Insurance.strInsPartyZip) {
							strUpdatePerson += FormatString("Zip = '%s', ", _Q(Insurance.strInsPartyZip));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyZip, nPartyID, strZip, Insurance.strInsPartyZip, aepMedium);
						}

						//SocialSecurity
						// (j.dinatale 2010-11-24) - PLID 39609 - need to format the insurance SSNs as well, the reason it is being done this
						//		way is to ensure that any incorrectly formatted SSNs in the DB are replaced with properly formatted SSNs
						CString strSSN = AdoFldString(rsExistingParties, "SocialSecurity");
						CString strNewSSNNums = FormatSSN(Insurance.strInsPartySSN);			

						if(!strNewSSNNums.IsEmpty() && strSSN != strNewSSNNums) {
							strUpdatePerson += FormatString("SocialSecurity = '%s', ", _Q(strNewSSNNums));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartySSN, nPartyID, strSSN, strNewSSNNums, aepMedium);
						}

						//HomePhone
						// (j.dinatale 2010-11-24) - PLID 39609 - need to format the insurance home phone as well, the reason it is being done this
						//		way is to ensure that any incorrectly formatted numbers in the DB are replaced with properly formatted numbers
						CString strHomePhone = AdoFldString(rsExistingParties, "HomePhone");
						CString strNewHomePhoneNums = FormatPhoneForSql(Insurance.strInsPartyHomePhone);

						if(!strNewHomePhoneNums.IsEmpty() && strHomePhone != strNewHomePhoneNums) {
							strUpdatePerson += FormatString("HomePhone = '%s', ", _Q(strNewHomePhoneNums));
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyPhone, nPartyID, strHomePhone, strNewHomePhoneNums, aepMedium);
						}

						//Gender
						BYTE bGender = AdoFldByte(rsExistingParties, "Gender");
						if(Insurance.nInsPartyGender > 0 && bGender != Insurance.nInsPartyGender) {
							strUpdatePerson += FormatString("Gender = %li, ", Insurance.nInsPartyGender);
							CString strOldGender;
							if(bGender == 1) {
								strOldGender = "Male";
							}
							else if(bGender == 2) {
								strOldGender = "Female";
							}
							CString strNewGender;
							if(Insurance.nInsPartyGender == 1) {
								strNewGender = "Male";
							}
							else if(Insurance.nInsPartyGender == 2) {
								strNewGender = "Female";
							}
							AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsPartyGender, nPartyID, strOldGender, strNewGender, aepMedium);
						}

						//TES 7/15/2010 - PLID 39518 - OK, did we actually update anything?
						if(strUpdatePerson != "UPDATE PersonT SET ") {
							//TES 7/15/2010 - PLID 39518 - Trim the last comma
							strUpdatePerson = strUpdatePerson.Left(strUpdatePerson.GetLength()-2);
							strUpdatePerson += FormatString(" WHERE ID = %li", nPartyID);
							AddStatementToSqlBatch(strSql, strUpdatePerson);
						}

						// (d.singleton 2012-08-10 14:39) - PLID 51938 add new resp party to appt, only if appt message
						if(bIsAppt) {							
							// (a.walling 2013-05-29 15:15) - PLID 56917 - Insurance.strNextechInsCoName was not escaped
							CString strApptInsParty;
							// (r.gonet 05/14/2013) - PLID 56675 - Altered to account for the fact that we have cleared out the insurance ahead of time. Now we always insert and never update.
							strApptInsParty.Format(
								"SELECT @strExistingApptIns = OldAppointmentInsuredPartyT.OldName\r\n"
								"FROM @OldAppointmentInsuredPartyT OldAppointmentInsuredPartyT\r\n"
								"WHERE OldAppointmentInsuredPartyT.Placement = %li\r\n"
								"IF LEN(@strExistingApptIns) > 0 BEGIN\r\n"
								"	INSERT INTO @ApptInsuredPartyAuditInfo(NewName, OldName, Type) VALUES('%s', @strExistingApptIns, 1);\r\n"
								// (r.gonet 05/14/2013) - PLID 56675 - Remove the old insurance because it is now useless and we can look later at this temp table to see if any insurance was replaced with null.
								"	DELETE FROM @OldAppointmentInsuredPartyT WHERE Placement = %li\r\n"
								"END\r\n"
								"ELSE\r\n"
								"BEGIN\r\n"
								"	INSERT INTO @ApptInsuredPartyAuditInfo(NewName, OldName, Type) VALUES('%s', @strExistingApptIns, 3);\r\n"
								"END\r\n"
								"INSERT INTO AppointmentInsuredPartyT(AppointmentID, InsuredPartyID, Placement) VALUES(@nApptID, %li, %li)\r\n"
								, i+1, _Q(Insurance.strNextechInsCoName), i+1, _Q(Insurance.strNextechInsCoName), nPartyID, i+1);
							AddStatementToSqlBatch(strSql, strApptInsParty);
						}
					}
				}
			}
			if(!bMapped) {
				//TES 7/15/2010 - PLID 39518 - We couldn't map to an incoming company.  Do any of the incoming companies have our placement?
				if(nPlacement != -1 && nPlacement <= arInsuranceSegments.GetSize()) {
					//TES 7/15/2010 - PLID 39518 - Yup, so we need to inactivate this one.
					AddStatementToSqlBatch(strSql, "UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PersonID = %li", nPartyID);

					// (r.gonet 06/10/2013) - PLID 57105 - Make sure we save the inactivation.
					bInsuranceChanged = true;

					CString strOldName;
					if(!mapRespTypes.Lookup(AsString(nPlacement), strOldName)) {
						//TES 7/15/2010 - PLID 39518 - This must have just been created.
						strOldName = AsString(nPlacement);
					}
					AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsuredPartyInsType, nPartyID, strOldName, "Inactive", aepMedium);
				}
			}

			rsExistingParties->MoveNext();
		}

		//TES 7/15/2010 - PLID 39518 - At this point, all the existing insurance parties have been updated.  Next, go through all the
		// segments, and if they haven't been mapped to a party, create a new party (the placement has already been cleared out for them).
		AddStatementToSqlBatch(strSql, "DECLARE @nInsuredPartyID INT");
		for(int i = 0; i < arInsuranceSegments.GetSize(); i++) {
			if(arMappedInsPartyIDs[i] == -1) {
				// (r.gonet 06/10/2013) - PLID 57105 - Make sure we save the creation of a new insured party.
				bInsuranceChanged = true;
				//TES 7/14/2010 - PLID 39635 - AssignInsCoIDs() will have set these variables in the batch.
				CString strInsCoID;
				strInsCoID.Format("@nInsuranceCoID%i", i);
				CString strInsContactID;
				strInsContactID.Format("@nInsuranceContactID%i", i);
				CString strInsPlanID;
				strInsPlanID.Format("@nInsurancePlanID%i", i);

				HL7_InsuranceFields Insurance = arInsuranceSegments[i];
				AddStatementToSqlBatch(strSql, "SET @nInsuredPartyID = @nNextPersonID");
				AddStatementToSqlBatch(strSql, "SET @nNextPersonID = @nNextPersonID+1");
				CString strBirthDate = "NULL";
				if(Insurance.dtInsPartyBirthDate.GetStatus() == COleDateTime::valid) {
					strBirthDate = "'" + FormatDateTimeForSql(Insurance.dtInsPartyBirthDate) + "'";
				}
				AddStatementToSqlBatch(strSql, "INSERT INTO PersonT (ID, First, Middle, Last, BirthDate, Address1, Address2, City, State, Zip, "
					"SocialSecurity, HomePhone, Gender) "
					"VALUES (@nInsuredPartyID, '%s', '%s', '%s', %s, '%s', '%s', '%s', '%s', '%s', "
					"'%s', '%s', %li)", 
					_Q(Insurance.strInsPartyFirst), _Q(Insurance.strInsPartyMiddle), _Q(Insurance.strInsPartyLast), strBirthDate, 
					_Q(Insurance.strInsPartyAddress1), _Q(Insurance.strInsPartyAddress2), _Q(Insurance.strInsPartyCity), 
					_Q(Insurance.strInsPartyState), _Q(Insurance.strInsPartyZip), 
					_Q(Insurance.strInsPartySSN), _Q(Insurance.strInsPartyHomePhone), 
					Insurance.nInsPartyGender == -1 ? 0 : Insurance.nInsPartyGender);
				CString strEffectiveDate = "NULL";
				if(Insurance.dtEffective.GetStatus() == COleDateTime::valid) {
					strEffectiveDate = "'" + FormatDateTimeForSql(Insurance.dtEffective) + "'";
				}
				CString strExpiresDate = "NULL";
				if(Insurance.dtExpires.GetStatus() == COleDateTime::valid) {
					strExpiresDate = "'" + FormatDateTimeForSql(Insurance.dtExpires) + "'";
				}
				//TES 7/12/2010 - PLID 39610 - Find out what relationship this code is mapped to (if it isn't mapped to anything, the
				// GetHL7Relation() function will tell the user, so we need to just exit).
				CString strRelation;
				//TES 7/19/2010 - PLID 39610 - First check whether the user's mapped this relation already.
				if(!mapUserMappedRelations.Lookup(Insurance.strInsPartyRelation, strRelation)) {
					if(!GetHL7Relation(GetRemoteData(), Insurance.strInsPartyRelation, strRelation, Message.nHL7GroupID)) {
						if(!PromptForHL7Relation(Insurance.strInsPartyRelation, Message.nHL7GroupID, Message.strHL7GroupName, strRelation, nAuditID, strSql)) {
							return FALSE;
						}
						else {
							//TES 7/19/2010 - PLID 39610 - Remember what the user chose.
							mapUserMappedRelations.SetAt(Insurance.strInsPartyRelation, strRelation);
						}
					}
				}
				// (d.singleton 2012-08-27 12:03) - PLID 51938 grab the resptypet.categorytype
				CString strInsCategoryType;
				long nInsCategoryType;
				if(!mapInsSetNumbers.Lookup(Insurance.strInsSetNumber, strInsCategoryType)) {
					strInsCategoryType = "1";
				}
				nInsCategoryType = atoi(strInsCategoryType);

				//see if a RespType record already exists
				_RecordsetPtr prsPriority = CreateParamRecordset("SELECT Priority FROM RespTypeT WHERE CategoryType = {INT} AND CategoryPlacement = {INT} " , nInsCategoryType, i+1);
				long nPriority; 
				if(!prsPriority->eof) {
					nPriority = AdoFldLong(prsPriority, "Priority");
					// (d.singleton 2012-10-09 14:50) - PLID 53032 need to use sql variable to ensure we are inserting the correct priority
					AddStatementToSqlBatch(strSql, "SET @nPriority = %li", nPriority);
				}
				else {
					// (d.singleton 2012-10-04 12:18) - PLID 53032 took out NextRecordset added sql param to track priority and re-order params in below query					
					//TES 7/20/2010 - PLID 39518 - Ensure the RespType exists.
					//TES 8/11/2010 - PLID 39518 - Ensure the Priority, not the ID.
					AddStatementToSqlBatch(strSql, "SET @nPriority = (SELECT MAX(Priority) + 1 FROM RespTypeT)");
					AddStatementToSqlBatch(strSql, "INSERT INTO RespTypeT (ID, TypeName, Priority, CategoryType, CategoryPlacement) "
						"SELECT (SELECT Max(ID)+1 FROM RespTypeT), %li, (SELECT Max(Priority)+1 FROM RespTypeT), %li, %li",
						i+1, nInsCategoryType, i+1);					
				}

				// (j.jones 2011-06-24 12:35) - PLID 29885 - added a default secondary reason code
				CString strSecondaryReasonCode = GetRemotePropertyText("DefaultMedicareSecondaryReasonCode", "47", 0, "<None>", true);
				if(strSecondaryReasonCode.IsEmpty()) {
					strSecondaryReasonCode = "47";
				}
				// (j.gruber 2010-08-04 10:03) - PLID 39977 - change copay structure
				//TES 8/11/2010 - PLID 39518 - We have the RespType Priority, but not the ID, so make sure we use the correct ID.
				// (d.singleton 2012-10-04 12:18) - PLID 53032 added sql param to track priority					
				AddStatementToSqlBatch(strSql, "INSERT INTO InsuredPartyT (PersonID, InsuranceCoID, InsuranceContactID, InsPlan, RespTypeID, "
					"PatientID, "
					"PolicyGroupNum, IDForInsurance, EffectiveDate, ExpireDate, RelationToPatient, Employer, "
					"AssignDate, SecondaryReasonCode) "
					"SELECT @nInsuredPartyID, %s, %s, %s, ID, %li, '%s', '%s', %s, %s, '%s', '%s', "
					"getdate(), '%s' FROM RespTypeT WHERE Priority = @nPriority",
					strInsCoID, strInsContactID, strInsPlanID, nPersonID, 
					_Q(Insurance.strGroupNum), _Q(Insurance.strIDForInsurance), strEffectiveDate, strExpiresDate, _Q(strRelation),
					_Q(Insurance.strEmployer), _Q(strSecondaryReasonCode));
				AuditEvent(nPersonID, strPersonName, nAuditID, aeiInsuredPartyAdded, nPersonID, "", Insurance.strNextechInsCoName, aepMedium, aetCreated);

				// (d.singleton 2012-08-10 14:39) - PLID 51938 add new resp party to appt
				if(bIsAppt) {
					// (a.walling 2013-05-29 15:15) - PLID 56917 - Insurance.strNextechInsCoName was not escaped
					CString strApptInsParty;
					// (r.gonet 05/14/2013) - PLID 56675 - Altered to account for the fact that we have cleared out the insurance ahead of time. Now we always insert and never update.
					strApptInsParty.Format(
						"SELECT @strExistingApptIns = OldAppointmentInsuredPartyT.OldName\r\n"
						"FROM @OldAppointmentInsuredPartyT OldAppointmentInsuredPartyT\r\n"
						"WHERE OldAppointmentInsuredPartyT.Placement = %li\r\n"
						"IF LEN(@strExistingApptIns) > 0 BEGIN\r\n"
						"	INSERT INTO @ApptInsuredPartyAuditInfo(NewName, OldName, Type) VALUES('%s', @strExistingApptIns, 1);\r\n"
						"	DELETE FROM @OldAppointmentInsuredPartyT WHERE Placement = %li\r\n"
						"END\r\n"
						"ELSE\r\n"
						"BEGIN\r\n"
						"	INSERT INTO @ApptInsuredPartyAuditInfo(NewName, OldName, Type) VALUES('%s', @strExistingApptIns, 3);\r\n"
						"END\r\n"
						"INSERT INTO AppointmentInsuredPartyT(AppointmentID, InsuredPartyID, Placement) VALUES(@nApptID, @nInsuredPartyID, %li)\r\n"
						, i+1, _Q(Insurance.strNextechInsCoName), i+1, _Q(Insurance.strNextechInsCoName), i+1);
					AddStatementToSqlBatch(strSql, strApptInsParty);					
				}

				//r.wilson PLID 52222 [HL7]
				CSqlFragment sqlFrag;

				// (d.singleton 2012-09-21 10:54) - PLID 51938 - duplicate declarations down here, removed them
				sqlFrag += CSqlFragment(
				" SELECT @b_DefaultDeductiblePerPayGroup = DefaultDeductiblePerPayGroup, "
				" @money_InsCoDefTotalDeductible = DefaultTotalDeductible, "
				" @money_InsCoDefTotalOOP = DefaultTotalOOP "
				" FROM InsuranceCoT WHERE PersonID = "+ strInsCoID +" ; \r\n"
				" "
				" INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PaygroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP) "
				" SELECT  @nInsuredPartyID AS InsuredPartyID, ServicePaygroupsT.ID AS PayGroupID , CopayMoney, CopayPercentage, CoInsurance, "
				" CASE WHEN @b_DefaultDeductiblePerPayGroup = 1 THEN PayGroupDefaultsT.TotalDeductible "
				" 	 ELSE @money_InsCoDefTotalDeductible "
				"	 END AS TotalDeductible, "
				" CASE WHEN @b_DefaultDeductiblePerPayGroup = 1 THEN PayGroupDefaultsT.TotalOOP "
				"	 ELSE @money_InsCoDefTotalOOP "
				"	 END AS TotalOOP "
				" FROM ServicePaygroupsT "
				" INNER JOIN (SELECT ID, PayGroupID, CopayMoney, CopayPercentage, CoInsurance, TotalDeductible, TotalOOP "
				"			  FROM InsuranceCoPayGroupsDefaultsT  "
				"			  WHERE InsuranceCoID = "+ strInsCoID +") "
				" AS PayGroupDefaultsT ON ServicePaygroupsT.ID = PayGroupDefaultsT.PaygroupID; \r\n"
				" "
				" IF @b_DefaultDeductiblePerPayGroup = 0 "
				" BEGIN "
				" UPDATE InsuredPartyT "
				" SET "
				"     TotalDeductible = @money_InsCoDefTotalDeductible, TotalOOP = @money_InsCoDefTotalOOP, DeductiblePerPayGroup = @b_DefaultDeductiblePerPayGroup, LastModifiedDate = GetDate() "
				" WHERE "
				" PersonID = @nInsuredPartyID AND InsuranceCoID = "+ strInsCoID +"; \r\n"
				" END \r\n"
				" ELSE "
				" BEGIN "
				" UPDATE InsuredPartyT "
				" SET "
				" DeductiblePerPayGroup = @b_DefaultDeductiblePerPayGroup "
				" WHERE "
				" PersonID = @nInsuredPartyID AND InsuranceCoID = "+ strInsCoID +"; \r\n"
				" END \r\n"

				);
				AddStatementToSqlBatch(strSql, sqlFrag.Flatten());

				//TES 7/22/2010 - PLID 39518 - Format the copay correctly.
				// (j.gruber 2010-08-04 10:03) - PLID 39977 - change copay structure - moved to after the ins party insert
				CString strCoPay = "NULL";
				COleCurrency cyCoPay;
				cyCoPay.ParseCurrency(Insurance.strCoPay);
				if(cyCoPay.GetStatus() == COleCurrency::valid) {
					// (j.jones 2015-11-05 11:39) - PLID 63866 - ensured CopayMoney is rounded
					RoundCurrency(cyCoPay);
					AddStatementToSqlBatch(strSql, "DECLARE @nPayGrpID INT");
					AddStatementToSqlBatch(strSql, "SET @nPayGrpID = (SELECT ID FROM ServicePayGroupsT WHERE Name = 'Copay');");
					AddStatementToSqlBatch(strSql,"INSERT INTO InsuredPartyPayGroupsT (InsuredPartyID, PayGroupID, CopayMoney) VALUES "
						" (@nInsuredPartyID, @nPayGrpID, CONVERT(money, '%s'))",FormatCurrencyForSql(cyCoPay));					
				}
			}
		}

		if(bIsAppt) {
			// (r.gonet 05/14/2013) - PLID 56675 - Anything remaining in the temp table of the old insurances for this appointment has been replaced with null in AppointmentInsuredPartiesT. So audit these remaining as deleted.
			AddStatementToSqlBatch(strSql,
				"INSERT INTO @ApptInsuredPartyAuditInfo (NewName, OldName, Type)\r\n"
				"SELECT '', OldName, %li\r\n"
				"FROM @OldAppointmentInsuredPartyT",
				aetDeleted);
			
			if(!bInsuranceChanged) {
				// (r.gonet 05/14/2013) - PLID 56675 - See if any insurance has changed
				if(arOldApptInsuredParties.GetSize() != arMappedInsPartyIDs.GetSize()) {
					bInsuranceChanged = true;
				} else {
					for(int i = 0; i < arOldApptInsuredParties.GetSize(); i++) {
						if(arOldApptInsuredParties[i] != arMappedInsPartyIDs[i]) {
							bInsuranceChanged = true;
							break;
						}
					}
				}
			}
		}
		return TRUE;
	}NxCatchAll(__FUNCTION__);
	return FALSE;
}

//TES 5/23/2011 - PLID 41353 - Added support for referring physician messages (MFN^M02)
BOOL HandleHL7RefPhys(const HL7Message &Message, BOOL bSendHL7Tablechecker)
{
	//TES 5/23/2011 - PLID 41353 - First, let's make sure that this is in fact a referring physician message.  For Athena, that means that
	// the MFI-1 field (Master File Identifier) is PRA, and the STF-4 field (Staff Type) is RD
	CString strMFI1;
	GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MFI", 1, 1, 1, 1, strMFI1);
	if(strMFI1 != "PRA") {
		MsgBox(MB_ICONERROR, "Unexpected Master File Identifier found in HL7 Message!  Expected value: PRA, Actual value: %s", strMFI1);
		return FALSE;
	}
	CString strSTF4;
	GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "STF", 1, 4, 1, 1, strSTF4);
	if(strSTF4 != "RD") {
		MsgBox(MB_ICONERROR, "Unexpected Staff Type found in HL7 Message!  Expected value: RD, Actual value: %s", strSTF4);
		return FALSE;
	}

	//TES 5/23/2011 - PLID 41353 - Gather all the information about the physician.
	HL7ReferringPhysician hrp;
	ParseReferringPhysicianMessage(Message, hrp);

	//TES 5/23/2011 - PLID 41353 - Next, are we updating, or deleting?  This is determined by MFE-1, Record-Level Event Code, MAD = Add,
	// MUP = Update
	CString strMFE1;
	GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MFE", 1, 1, 1, 1, strMFE1);
	if(strMFE1 != "MAD" && strMFE1 != "MUP") {
		MsgBox(MB_ICONERROR, "Unexpected Record-Level Event Code %s found in HL7 Message!", strMFE1);
		return FALSE;
	}

	//TES 5/23/2011 - PLID 41353 - OK, let's get started building our query
	CString strSql;
	CNxParamSqlArray aryParams;
	CArray<CommonAuditData,CommonAuditData&> aryAuditData;
	long nAuditID = BeginAuditTransaction();

	bool bAdd = false;
	CString strRefPhysID;

	if(strMFE1 == "MAD") {
		//TES 6/8/2011 - PLID 41353 - First, try and match the referring physician (on ID only).  If we can, then we'll update, otherwise, 
		// we'll add
		long nRefPhysID = -1;
		if(hmrSuccess == FindMatchingRefPhys(GetRemoteData(), hrp, Message.nHL7GroupID, nRefPhysID, true)) {
			//TES 6/8/2011 - PLID 41353 - If this is an Add message with the same ID as an existing physician, we run the risk of overwriting
			// one record with another (which may affect an indefinite number of patients/bills/etc.).  If the name is different, we need
			// to get confirmation from the user
			_RecordsetPtr rsExistingRefPhys = CreateParamRecordset("SELECT First, Last FROM PersonT WHERE ID = {INT}", nRefPhysID);
			ASSERT(!rsExistingRefPhys->eof); //FindMatchingRefPhys must have given us a valid ID
			CString strExistingFirst = AdoFldString(rsExistingRefPhys, "First");
			CString strExistingLast = AdoFldString(rsExistingRefPhys, "Last");
			if(strExistingFirst.CompareNoCase(hrp.strFirst) != 0 || strExistingLast.CompareNoCase(hrp.strLast) != 0) {
				if(IDYES == MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "WARNING: This message is attempting to update the existing referring "
					"physician '%s %s' with information for a physician named '%s %s.'  This change will affect ALL records, including patients "
					"and bills, tied to the existing referring physician, and CANNOT be undone.  It is recommended that you cancel this import, and contact "
					"NexTech Technical Support for assistance in resolving this issue.  Would you like to do so now?",
					strExistingFirst, strExistingLast, hrp.strFirst, hrp.strLast)) {
					return FALSE;
				}
			}
			strRefPhysID.Format("%li", nRefPhysID);
			bAdd = false;
		}
		else {
			bAdd = true;
		}
	}

	if(bAdd) {
		//TES 5/23/2011 - PLID 41353 - Create it
		CreateHL7ReferringPhysician(GetRemoteData(), Message, hrp, strSql, aryParams, aryAuditData);
		//TES 6/10/2011 - PLID 41353 - CreateHL7ReferringPhysician always puts the new ID in a variable named @nRefPhysID
		strRefPhysID = "@nRefPhysID";
	}
	else {
		//TES 5/23/2011 - PLID 41353 - Find the matching record
		AddParamStatementToSqlBatch(strSql, aryParams, "DECLARE @nNextPersonID INT");
		AddParamStatementToSqlBatch(strSql, aryParams, "SELECT @nNextPersonID = COALESCE(Max(ID),0)+1 FROM PersonT");
		//TES 6/8/2011 - PLID 41353 - We may have already determined the ID
		if(strRefPhysID.IsEmpty()) {
			strRefPhysID = GetNextechRefPhys(hrp, false, Message, nAuditID, strSql, &aryParams);
		}
		//TES 5/23/2011 - PLID 41353 - If the function returned @nRefPhysID, that means it created a new record, 
		// in which case we don't need to do anything else
		if(strRefPhysID != "@nRefPhysID") {
			//TES 5/23/2011 - PLID 41353 - If the function returned "NULL", the user was prompted and cancelled, so abort
			if(strRefPhysID == "NULL") {
				return FALSE;
			}
			//TES 5/23/2011 - PLID 41353 - Otherwise, it must be a valid ID.  Call our function to generate the update query
			long nRefPhysID = atol(strRefPhysID);
			ASSERT(nRefPhysID > 0);
			UpdateHL7ReferringPhysician(GetRemoteData(), nRefPhysID, hrp, strSql, aryParams, aryAuditData);
		
		}
	}

	//TES 5/23/2011 - PLID 41353 - Audit everything we've been told to audit
	for(int i = 0; i < aryAuditData.GetSize(); i++) {
		CommonAuditData cad = aryAuditData[i];
		AuditEvent(-1, hrp.strLast + ", " + hrp.strFirst + " " + hrp.strMiddle, nAuditID, cad.eEvent, cad.nRecordID, cad.strOld, cad.strNew, aepMedium, cad.eType);
	}
	//TES 5/23/2011 - PLID 41353 - Finalize the event, pass in -2 as the patient ID to indicate that this is not patient-related
	long nPatientID = -2;
	//TES 6/10/2011 - PLID 41353 - If we don't have a referring physician ID yet, get it from this function 
	long nRefPhysID = -1;
	if(strRefPhysID != "@nRefPhysID") {
		nRefPhysID = atol(strRefPhysID);
		ASSERT(nRefPhysID > 0);
	}
	if(!FinalizeHL7Event(nPatientID, Message, strSql, bSendHL7Tablechecker, &aryParams, &nAuditID, nRefPhysID == -1 ? strRefPhysID : "", nRefPhysID == -1 ? &nRefPhysID : NULL)) {
		return FALSE;
	}
	else {
		//TES 6/10/2011 - PLID 41353 - Send a tablechecker.
		CClient::RefreshTable(NetUtils::RefPhys, nRefPhysID);
		CClient::RefreshTable(NetUtils::CustomContacts, nRefPhysID);
		return TRUE;
	}

}

// (z.manning 2011-06-15 14:45) - PLID 40903
void GetHL7MessageQueueMap(IN const CArray<long,long> &arynMessageIDs, OUT CMap<long,long,CString,LPCTSTR> &mapMessageIDToMessage)
{
	if(arynMessageIDs.GetSize() == 0) {
		return;
	}

	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, Message \r\n"
		"FROM HL7MessageQueueT \r\n"
		"WHERE ID IN ({INTARRAY}) \r\n"
		, arynMessageIDs);
	for(; !prs->eof; prs->MoveNext())
	{
		long nMessageID = AdoFldLong(prs, "ID");
		CString strMessage = AdoFldString(prs, "Message", "");
		mapMessageIDToMessage.SetAt(nMessageID, strMessage);
	}
}

// (z.manning 2011-06-15 14:45) - PLID 40903
void GetHL7MessageLogMap(IN const CArray<long,long> &arynMessageIDs, OUT CMap<long,long,CString,LPCTSTR> &mapMessageIDToMessage)
{
	if(arynMessageIDs.GetSize() == 0) {
		return;
	}

	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT MessageID, Message \r\n"
		"FROM HL7MessageLogT \r\n"
		"WHERE MessageID IN ({INTARRAY}) \r\n"
		, arynMessageIDs);
	for(; !prs->eof; prs->MoveNext())
	{
		long nMessageID = AdoFldLong(prs, "MessageID");
		CString strMessage = AdoFldString(prs, "Message", "");
		mapMessageIDToMessage.SetAt(nMessageID, strMessage);
	}
}

// (z.manning 2011-07-08 16:22) - PLID 38753
// (r.goldschmidt 2015-10-28 18:54) - PLID 66437 - Update everywhere in Practice that displays HL7 messages to handle purged messages
void ViewHL7ImportMessage(const long nMessageID)
{
	_RecordsetPtr prs = CreateParamRecordset("SELECT Message, CASE WHEN PurgeDate IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS WasPurged, InputDate, PurgeDate FROM HL7MessageQueueT WHERE ID = {INT}", nMessageID);
	if(!prs->eof) {
		CString strMessage = AdoFldString(prs, "Message");
		if (!!AdoFldBool(prs, "WasPurged")) {
			strMessage.Format("[This HL7 message was purged.]\r\nMessage Input Date: %s\r\nMessage Purge Date: %s"
				, FormatDateTimeForInterface(AdoFldDateTime(prs, "InputDate"), NULL, dtoDate)
				, FormatDateTimeForInterface(AdoFldDateTime(prs, "PurgeDate"), NULL, dtoDate));
		}
		DisplayHL7Message(strMessage);
	}
}

// (z.manning 2011-07-08 16:22) - PLID 38753
// (r.goldschmidt 2015-10-28 18:54) - PLID 66437 - Update everywhere in Practice that displays HL7 messages to handle purged messages
void ViewHL7ExportMessage(const long nMessageID)
{
	_RecordsetPtr prs = CreateParamRecordset("SELECT Message, CASE WHEN PurgeDate IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS WasPurged, CreateDate, PurgeDate FROM HL7MessageLogT WHERE MessageID = {INT}", nMessageID);
	if(!prs->eof) {
		CString strMessage = AdoFldString(prs, "Message");
		if (!!AdoFldBool(prs, "WasPurged")) {
			strMessage.Format("[This HL7 message was purged.]\r\nMessage Create Date: %s\r\nMessage Purge Date: %s"
				, FormatDateTimeForInterface(AdoFldDateTime(prs, "CreateDate"), NULL, dtoDate)
				, FormatDateTimeForInterface(AdoFldDateTime(prs, "PurgeDate"), NULL, dtoDate));
		}
		DisplayHL7Message(strMessage);
	}

}

void DisplayHL7Message(CString strMessage)
{
	// (z.manning 2011-07-08 16:43) - PLID 38753 - In HL7 carriage returns are the standard, but let's format
	// this such that it's more readable.
	strMessage.Replace("\r\n", "\r");
	strMessage.Replace("\r", "\r\n");

	// (r.gonet 05/01/2014) - PLID 49432 - Disable word wrap on the HL7 message preview.
	extern CPracticeApp theApp;
	CMsgBox dlg(NULL);
	dlg.msg = strMessage;
	dlg.m_bWordWrap = false;
	dlg.m_pFont = theApp.GetPracticeFont(CPracticeApp::pftFixed);
	dlg.DoModal();
}

// (r.gonet 02/26/2013) - PLID 48419 - Dismisses a message in HL7MessageQueueT, audits, and sends a table checker.
// - nMessageID is the ID of the message in HL7MessageQueueT to be dismissed.
// Returns true if dismissal was successful. false otherwise.
// Throws NxException in case of bad data.
bool DismissImportedHL7Message(long nMessageID)
{	
	// (r.gonet 02/26/2013) - PLID 48419 - Can't dismiss an invalid message.	
	if(nMessageID <= 0) {
		return false;
	}

	// (r.gonet 02/26/2013) - PLID 48419 - Update the action and get some info about what we are updating.
	CParamSqlBatch sqlBatch;
	sqlBatch.Add(
		"SET NOCOUNT ON; \r\n"
		"DECLARE @DismissedMessagesVar TABLE ( \r\n"
		"	ID INT NOT NULL, \r\n"
		"	OldActionID INT NULL, \r\n"
		"	NewActionID INT NULL \r\n"
		"); \r\n"
		" \r\n"	
		"UPDATE HL7MessageQueueT SET ActionID = {INT} \r\n"
		"OUTPUT inserted.ID, deleted.ActionID, inserted.ActionID \r\n"
		"INTO @DismissedMessagesVar \r\n"
		"WHERE ID = {INT}; \r\n"
		"SET NOCOUNT OFF; \r\n"
		" \r\n"
		"SELECT ID \r\n"
		"FROM @DismissedMessagesVar; \r\n", 
		mqaDismiss, nMessageID);
	sqlBatch.Add(
		"SELECT DismissedMessagesT.OldActionID, DismissedMessagesT.NewActionID, HL7MessageQueueT.Message, HL7MessageQueueT.PatientName, HL7SettingsT.ID AS GroupID, HL7SettingsT.Name AS GroupName \r\n"
		"FROM HL7MessageQueueT \r\n"
		"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID \r\n"
		"INNER JOIN @DismissedMessagesVar DismissedMessagesT ON HL7MessageQueueT.ID = DismissedMessagesT.ID \r\n");

	_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());
	if(prs->eof) {
		// (r.gonet 02/26/2013) - PLID 48419 - Possible if we are passed an ID to an invalid message record.
		return false;
	} else {
		// (r.gonet 02/26/2013) - PLID 48419 - We dismissed the message. Now we can proceed to auditing.
	}

	prs = prs->NextRecordset(NULL);
	if(prs->eof) {
		// (r.gonet 02/26/2013) - PLID 48419 - This is bad data. If there was an HL7MessageQueueT, by a foreign key constraint, there must be an HL7SettingsT record.
		ThrowNxException("%s : Could not obtain the HL7 settings group while dismissing the HL7 message with ID = %li", __FUNCTION__, nMessageID);
	} else {
		// (r.gonet 02/26/2013) - PLID 48419 - We can now get the patient name and HL7 settings group name that the message is associated with, which we need to audit.
	}

	long nOldActionID = AdoFldLong(prs->Fields, "OldActionID", -1);
	long nNewActionID = AdoFldLong(prs->Fields, "NewActionID", -1);
	CString strMessage = AdoFldString(prs->Fields, "Message", "");
	CString strPatientName = AdoFldString(prs->Fields, "PatientName", "");
	long nHL7GroupID = AdoFldLong(prs->Fields, "GroupID", -1); 
	CString strHL7GroupName = AdoFldString(prs->Fields, "GroupName", "");
	prs->Close();

	// (r.gonet 02/26/2013) - PLID 48419 - See if we need to audit.
	if(nOldActionID == nNewActionID) {
		// (r.gonet 02/26/2013) - PLID 48419 - Well, it IS dismissed.
		return true;
	}

	//TES 4/21/2008 - PLID 29721 - Audit that we've processed this message.
	// (a.walling 2010-01-25 08:24) - PLID 37023 - Patient ID is not available here; this patient name is simply extracted from the message and has not been matched yet.
	// (a.walling 2010-01-25 16:33) - PLID 37023 - Must include the patient name in the old/new value since we can't display it
	AuditEvent(-1, "", BeginNewAuditEvent(), aeiHL7MessageProcessed, nMessageID, 
		"Pending (" + strHL7GroupName + " - " + GetActionDescriptionForHL7Event(strMessage, nHL7GroupID) + " for " + strPatientName + ")",
		"Dismissed", aepMedium, aetChanged);

	// (r.gonet 02/26/2013) - PLID 48419 - Send a table checker.
	CClient::RefreshTable(NetUtils::HL7MessageQueueT, nMessageID);
	return true;
}

// (r.gonet 02/26/2013) - PLID 47534 - Dismisses a message in HL7MessageLogT, audits, and sends a table checker.
// - nMessageID is the ID of the message in HL7MessageLogT to be dismissed.
// Returns true if dismissal was successful. false otherwise.
// Throws NxException in case of bad data.
bool DismissExportedHL7Message(long nMessageID)
{	
	// (r.gonet 02/26/2013) - PLID 47534 - Invalid messages can't be dismissed.
	if(nMessageID <= 0) {
		return false;
	}

	// (r.gonet 02/26/2013) - PLID 47534 - Update the dismissed flag and get info about what was dismissed.
	CParamSqlBatch sqlBatch;
	sqlBatch.Add(
		"SET NOCOUNT ON; \r\n"
		"DECLARE @DismissedMessagesVar TABLE ( \r\n"
		"	ID INT NOT NULL, \r\n"
		"	OldDismissed BIT NOT NULL, \r\n"
		"	NewDismissed BIT NOT NULL \r\n"
		"); \r\n"
		" \r\n"	
		"UPDATE HL7MessageLogT SET Dismissed = 1 \r\n"
		"OUTPUT inserted.MessageID, deleted.Dismissed, inserted.Dismissed \r\n"
		"INTO @DismissedMessagesVar \r\n"
		"WHERE MessageID = {INT}; \r\n"
		"SET NOCOUNT OFF; \r\n"
		" \r\n"
		"SELECT ID \r\n"
		"FROM @DismissedMessagesVar; \r\n", 
		nMessageID);
	sqlBatch.Add(
		"SELECT DismissedMessagesT.OldDismissed, DismissedMessagesT.NewDismissed, HL7MessageLogT.Message, HL7MessageLogT.UserDefinedID, PersonT.FullName AS PatientName, HL7SettingsT.Name AS GroupName, HL7MessageLogT.MessageType \r\n"
		"FROM HL7MessageLogT \r\n"
		"INNER JOIN HL7SettingsT ON HL7MessageLogT.GroupID = HL7SettingsT.ID \r\n"
		"INNER JOIN @DismissedMessagesVar DismissedMessagesT ON HL7MessageLogT.MessageID = DismissedMessagesT.ID \r\n"
		// (r.gonet 02/26/2013) - PLID 47534 - No FK constraint here, so left join
		"LEFT JOIN PatientsT ON PatientsT.UserDefinedID = HL7MessageLogT.UserDefinedID \r\n"
		"LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID; \r\n");

	_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());
	if(prs->eof) {
		// (r.gonet 02/26/2013) - PLID 47534 - Possible if we are passed an ID to an invalid message record.
		return false;
	} else {
		// (r.gonet 02/26/2013) - PLID 47534 - We dismissed the message. Now we can proceed to auditing.
	}

	prs = prs->NextRecordset(NULL);
	if(prs->eof) {
		// (r.gonet 02/26/2013) - PLID 47534 - This is bad data. If there was an HL7MessageQueueT, by a foreign key constraint, there must be an HL7SettingsT record.
		ThrowNxException("%s : Could not obtain the HL7 settings group while dismissing the HL7 message with ID = %li", __FUNCTION__, nMessageID);
	} else {
		// (r.gonet 02/26/2013) - PLID 47534 - We can now get the patient name and HL7 settings group name that the message is associated with, which we need to audit.
	}

	BOOL bOldDismissed = AdoFldBool(prs->Fields, "OldDismissed");
	BOOL bNewDismissed = AdoFldBool(prs->Fields, "NewDismissed");
	CString strMessage = AdoFldString(prs->Fields, "Message", "");
	long nUserDefinedID = AdoFldLong(prs->Fields, "UserDefinedID", -1);
	CString strPatientName = AdoFldString(prs->Fields, "PatientName", "");
	CString strHL7GroupName = AdoFldString(prs->Fields, "GroupName", "");
	HL7ExportMessageType hemtExportMessageType = (HL7ExportMessageType)AdoFldLong(prs->Fields, "MessageType");
	prs->Close();

	if(bOldDismissed == bNewDismissed) {
		// (r.gonet 02/26/2013) - PLID 47534 - Uh, return true since the message IS in fact dismissed.
		return true;
	}
	// (r.gonet 02/26/2013) - PLID 47534 - Since HL7MessageLogT.UserDefinedID doesn't have any FK constraint, we need to do something special here
	//  in case the patient was deleted.
	CString strPatientDescription;
	if(strPatientName.IsEmpty()) {
		// (r.gonet 02/26/2013) - PLID 47534 - Even for patients with no first and last, we'll still have a comma.
		strPatientDescription.Format("Patient with ID = %li", nUserDefinedID);
	} else {
		strPatientDescription = strPatientName; 
	}

	// (r.gonet 02/26/2013) - PLID 47534 - Audit that we dismissed the message.
	AuditEvent(-1, "", BeginNewAuditEvent(), aeiHL7ExportedMessageDismissed, nMessageID, 
		"Pending (" + strHL7GroupName + " - " + GetHL7ExportMessageTypeDescription(hemtExportMessageType) + " for " + strPatientDescription + ")",
		"Dismissed", aepMedium, aetChanged);

	// (r.gonet 02/26/2013) - PLID 47534 - Notify any listeners.
	CClient::RefreshTable(NetUtils::HL7MessageLogT, nMessageID);
	return true;
}

//TES 8/16/2011 - PLID 44262 - Added an accessor for the cache
CHL7SettingsCache* GetHL7SettingsCache()
{
	EnsureHL7SettingsCache();
	return g_pHL7SettingsCache;
}

//TES 9/13/2011 - PLID 45465 - Loads all the custom segments for the given group, message type, and scope into an array
// NOTE: Caller is responsible for deleting the CustomSegment pointers allocated in this function
void LoadCustomSegmentInfo(long nHL7GroupID, HL7MessageType hmt, HL7CustomSegmentScope hcss, OUT CArray<CustomSegment*,CustomSegment*> &arCustomSegments)
{
	//TES 9/13/2011 - PLID 45465 - Function copied from CHL7CustomSegmentsDlg, although our version is simpler since we're only loading
	// one scope.
	_RecordsetPtr rsCustomSegments = CreateParamRecordset("SELECT HL7CustomSegmentsT.ID, HL7CustomSegmentsT.SegmentName, "
		"HL7CustomSegmentsT.OffsetFromSegment, HL7CustomSegmentsT.OffsetAmount, HL7CustomSegmentFieldsT.FieldValue, "
		"HL7CustomSegmentFieldsT.FieldType, HL7CustomSegmentFieldsT.DataFieldID, "
		"HL7CustomSegmentsT.CriteriaFieldID AS SegmentCriteriaField, HL7CustomSegmentFieldsT.CriteriaFieldID AS FieldCriteriaField, "
		"HL7CustomSegmentFieldsT.TextLimit "
		"FROM HL7CustomSegmentsT LEFT JOIN HL7CustomSegmentFieldsT ON HL7CustomSegmentsT.ID = HL7CustomSegmentFieldsT.HL7CustomSegmentID "
		"WHERE HL7CustomSegmentsT.HL7GroupID = {INT} AND HL7CustomSegmentsT.MessageType = {INT} AND HL7CustomSegmentsT.Scope = {INT} "
		"ORDER BY HL7CustomSegmentsT.OffsetFromSegment, HL7CustomSegmentsT.OffsetAmount, HL7CustomSegmentFieldsT.OrderIndex ", 
		nHL7GroupID, hmt, hcss);
	long nCurrentSegmentID = -1;
	CustomSegment *pcsCurrent = NULL;
	while(!rsCustomSegments->eof) {
		//TES 9/13/2011 - PLID 45465 - Are we on a new segment?
		long nSegmentID = AdoFldLong(rsCustomSegments, "ID");
		if(nSegmentID != nCurrentSegmentID) {
			//TES 9/13/2011 - PLID 45465 - Add our current segment, and start a new one
			if(nCurrentSegmentID != -1) {
				arCustomSegments.Add(pcsCurrent);
			}
			nCurrentSegmentID = nSegmentID;
			pcsCurrent = new CustomSegment;
			pcsCurrent->nID = nSegmentID;
			pcsCurrent->strName = AdoFldString(rsCustomSegments, "SegmentName");
			pcsCurrent->hsOffsetFrom = (HL7Segment)AdoFldLong(rsCustomSegments, "OffsetFromSegment");
			pcsCurrent->nOffsetBy = AdoFldLong(rsCustomSegments, "OffsetAmount");
			pcsCurrent->nCriteriaFieldID = AdoFldLong(rsCustomSegments, "SegmentCriteriaField", 0);
			pcsCurrent->parFields = new CArray<CustomSegmentField,CustomSegmentField&>;
		}
		//TES 9/13/2011 - PLID 45465 - Add the field
		CustomSegmentField csf;
		csf.type = (CustomSegmentFieldType)AdoFldLong(rsCustomSegments, "FieldType");
		csf.nFieldID = AdoFldLong(rsCustomSegments, "DataFieldID", 0);
		csf.strValue = AdoFldString(rsCustomSegments, "FieldValue", pcsCurrent->strName + "|");
		csf.nCriteriaFieldID = AdoFldLong(rsCustomSegments, "FieldCriteriaField", 0);
		csf.nTextLimit = AdoFldLong(rsCustomSegments, "TextLimit", -1);
		pcsCurrent->parFields->Add(csf);

		rsCustomSegments->MoveNext();
	}
	//TES 9/13/2011 - PLID 45465 - Add our last segment (if any)
	if(nCurrentSegmentID != -1) {
		arCustomSegments.Add(pcsCurrent);
	}
}

//r.wilson (8/23/2012) PLID 52222 [HL7]
CString	DeclareDefaultDeductibleVars()
{

	CString strQry = "";					
	strQry += " DECLARE @b_DefaultDeductiblePerPayGroup BIT\r\n";
	strQry += " DECLARE @money_InsCoDefTotalDeductible Money \r\n";
	strQry += " DECLARE @money_InsCoDefTotalOOP Money \r\n";

	return strQry;
}

// (r.gonet 12/11/2012) - PLID 54117 - Get a description of a message type.
CString GetHL7ExportMessageTypeDescription(HL7ExportMessageType hemt)
{
	switch(hemt) {
	case hemtAcknowledgement:
		return "Acknowledgement";
	case hemtAddNewPatient:
		return "New Patient";
	case hemtUpdatePatient:
		return "Patient Update";
	case hemtNewAppt:
		return "New Appointment";
	case hemtUpdateAppt:
		return "Appointment Update";
	case hemtCancelAppt:
		return "Appointment Cancellation";
	case hemtNewEmnBill:
		return "EMN Bill";
	case hemtNewLab:
		return "Lab Order";
	case hemtNewLabResult:
		return "Lab Result";
	case hemtLockedEMN:
		return "Locked EMN";
		// (b.savon 2013-10-01 15:49) - PLID 53171 - Connectathon - Support new HL7 message type A01^ADT_A01 - Admit patient
	case hemtAdmitPatient:
		return "Admit Patient";
		// (b.savon 2013-10-01 15:50) - PLID 53174 - Connectathon - Support new HL7 message type A11^ADT_A09 - Cancel admit patient
	case hemtCancelAdmitPatient:
		return "Cancel Admit Patient";
		// (b.savon 2013-10-01 15:50) - PLID 53175 - Connectathon - Support new HL7 message type A03^ADT_A03 - Discharge patient
	case hemtDischargePatient:
		return "Discharge Patient";
		// (b.savon 2013-10-01 15:50) - PLID 53176 - Connectathon - Support new HL7 message type A13^ADT_A01 - Cancel discharge patient
	case hemtCancelDischargePatient:
		return "Cancel Discharge Patient";
		// (b.savon 2013-10-02 11:58) - PLID 58850 - Connectathon - Support new HL7 message type ADT^A40^ADT_A39 - Merge patient
	case hemtMergePatient:
		return "Merge Patient";
		// (b.savon 2014-12-23 09:55) - PLID 64471 - Add support to export MFN HL7 messages - Construct HL7 Message
	case hemtReferringPhysician:
		return "Referring Physician";
		// (s.tullis 2015-06-23 14:17) - PLID 66197 - Patient Reminder
	case hemtPatientReminder: 
		return "Patient Reminder";
	default:
		// Unhandled export message type.
		ASSERT(FALSE);
		return "";
	}
}

// (d.singleton 2012-10-08 17:49) - PLID 53097 need to support importing MDM messages
BOOL HandleHL7Notification(const HL7Message &Message, CWnd *pwndParent)
{
	HL7_PIDFields PID;
	HL7_OBXSegment OBX;

	ParsePIDSegment(Message.strMessage, Message.nHL7GroupID, PID, GetRemoteData());
	long nOBXLine = 1;
	CString strNote;
	while(ParseOBXSegment(GetHL7SettingsCache(), Message.strMessage, Message.nHL7GroupID, nOBXLine, -1, -1, OBX)) {
		strNote += DeformatHL7Text(OBX.strValue) + "\r\n\r\n";
		nOBXLine++;
	}
	
	//return false if the note is blank
	if(strNote.IsEmpty()) {
		return FALSE;
	}

	//get patient id and return false if failure
	long nPersonID = GetPatientFromHL7Message(Message.strMessage, Message.nHL7GroupID, pwndParent);
	if(nPersonID == -1) {
		return FALSE;
	}

	CString strSqlBatch;
	CNxParamSqlArray aryParams;
	// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO Notes (PersonID, Date, Note, Priority) "
		"SELECT {INT}, GetDate(), {STRING}, 3", nPersonID, strNote);

	if(!FinalizeHL7Event(nPersonID, Message, strSqlBatch, FALSE, &aryParams)) {
		return FALSE;
	}
	// (d.singleton 2013-01-29 15:03) - PLID 54235 audit creation of note
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientNote, -1, "", strNote, aepHigh, aetCreated);

	return TRUE;
}

// (d.singleton 2013-01-17 15:18) - PLID 54781 function to handle ORU messages ( images vs lab results ) 
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL HandleORUMessage(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker, IN OUT CDWordArray &arNewCDSInterventions)
{
	BOOL bSuccess = FALSE;

	// (b.spivey, January 23, 2013) - PLID 54751 - Grab these settings. 
	//bit 1 == true; bit 0 == false; 
	bool bPatHistoryImportFlag = (GetHL7SettingBit(Message.nHL7GroupID, "UsePatientHistoryImportMSH_3") == 1 ? true : false);
	
	//If the flag is on, import an image. 
	if(bPatHistoryImportFlag) {
		bSuccess = ImportHL7Image(Message, pMessageParent, bSendHL7Tablechecker);
	}
	else {
		//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
		bSuccess = ImportHL7LabResult(Message, pMessageParent, bSendHL7Tablechecker, arNewCDSInterventions);
		if(!bSuccess && GetHL7Logger() != NULL && !GetHL7Logger()->ErrorLogged()) {
			// (r.gonet 05/01/2014) - PLID 61842 - The import of a lab result message failed
			// but no error was logged. All lab result import failures must log at least one error,
			// which will be user-visible.
			// You can log an error by calling LogHL7Error("This is the reason for the failure");
			ASSERT(FALSE);
		}
	}

	return bSuccess;
}

// (d.singleton 2013-01-17 15:18) - PLID 54781
BOOL ImportHL7Image(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker)
{
	HL7_OBXSegment OBX;

	long nPersonID = -1;
	nPersonID = GetPatientFromHL7Message(Message.strMessage, Message.nHL7GroupID, pMessageParent);
	if(nPersonID == -1) {
		return FALSE;
	}

	CString strImageFileName;
	if(COMPONENT_NOT_FOUND == GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", 1, 4, 1, 2, strImageFileName)) {
		// (d.singleton 2013-01-17 17:39) - PLID 54781 maybe they only sent one component
		GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", 1, 4, 1, 1, strImageFileName);
	}
	strImageFileName = DeformatHL7Text(strImageFileName);

	// (d.singleton 2013-01-18 14:18) - PLID 54781 grab the file date
	CString strFileDate;
	COleDateTime dtFileDate;
	GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "OBR", 1, 7, 1, 1, strFileDate);
	if(!strFileDate.IsEmpty()) {
		if(!ParseHL7Date(strFileDate, dtFileDate)) {
			dtFileDate.SetStatus(COleDateTime::invalid);
		}
	}

	ParseOBXSegment(GetHL7SettingsCache(), Message.strMessage, Message.nHL7GroupID, 1, -1, -1, OBX);
	if(OBX.strFileData.IsEmpty()) {
		return FALSE;
	}

	// (d.singleton 2013-01-23 10:48) - PLID 54781 Get the notes if there are any
	CString strFileComment;
	CString strFileCommentLine;
	int nFileCommentLine = 1;
	while(SEGMENT_NOT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "NTE", nFileCommentLine, 3, 1, 1, strFileCommentLine)) {
		//(s.dhole 2014-06-04 10:32) PLID 47478 - Aplly DeformatHL7Text to comment
		if(!strFileComment.IsEmpty()) strFileComment += "\r\n";
		strFileComment += DeformatHL7Text(strFileCommentLine);
		nFileCommentLine++;
		strFileCommentLine = "";
	}

	// (d.singleton 2013-01-21 15:23) - PLID 54781 decode the ED
	CFile ImageFile;
	CString strFileName = GetPatientDocumentName(GetRemoteData(), nPersonID, OBX.strFileExt.MakeLower());
	CString strFilePath = GetPatientDocumentPath(GetRemoteData(), nPersonID);
	CString strFullFileName = strFilePath ^ strFileName;
	if (!FileUtils::EnsureDirectory(strFilePath)) {
		CString strErr;
		FormatLastError(strErr, "Could not create or access the folder '%s'", strFilePath);
		ThrowNxException(strErr);
	}
	DecodeEncapsulatedData(OBX.strFileData, OBX.strFileEncodingType, strFullFileName, ImageFile);

	// (d.singleton 2013-01-21 15:50) - PLID 54781 now we need to attach to history and audit changes
	CString strSqlBatch;
	CNxParamSqlArray aryParams;
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DECLARE @nMailID INT");
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DECLARE @nMailBatchID INT");		
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SELECT @nMailBatchID = COALESCE(MAX(MailBatchID), 0) + 1 FROM MailSent WITH (UPDLOCK, HOLDLOCK);");
	// (j.armen 2014-01-30 10:38) - PLID 55225 - Idenitate MailSent
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, 
		"INSERT INTO MailSent (PersonID, Selection, PathName, Subject, Sender, MailBatchID, Date, TemplateID, ServiceDate, IsPhoto) "
		"SELECT {INT}, 'BITMAP:FILE', {STRING}, {STRING}, {STRING}, @nMailBatchID, GetDate(), NULL, {OLEDATETIME}, {INT}", 
		nPersonID, strFileName, strImageFileName, GetCurrentUserName(), dtFileDate, IsImageFile(strFullFileName) ? 1 : 0);
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SET @nMailID = SCOPE_IDENTITY()");
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO MailSentNotesT (MailID, Note) VALUES (@nMailID, {STRING})", strFileComment);

	if(!FinalizeHL7Event(nPersonID, Message, strSqlBatch, FALSE, &aryParams)) {
		return FALSE;
	}
	//we know finalize ran fine so we can now audit
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(nPersonID, GetExistingPatientName(nPersonID), nAuditID, aeiPatientImageImported, -1, "", strFileName, aepHigh, aetCreated);

	return TRUE;
}

//TES 10/16/2015 - PLID 66204 - Custom type for MDI optical prescriptions
BOOL ImportOpticalPrescription(const HL7Message &Message, CWnd *pMessageParent, BOOL bSendHL7Tablechecker)
{
	long nPersonID = -1;
	nPersonID = GetPatientFromHL7Message(Message.strMessage, Message.nHL7GroupID, pMessageParent);
	if (nPersonID == -1) {
		return FALSE;
	}

	CString strEncodingType;
	GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "ZRX", 1, 1, 1, 4, strEncodingType);
	if (strEncodingType.CompareNoCase("Base64") != 0) {
		ThrowNxException("Optical Prescription found with invalid encoding type");
	}
	CString strEncodedXml;
	GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "ZRX", 1, 1, 1, 5, strEncodedXml);
	BYTE *pBytes = NULL;
	int nLength = -1;
	CBase64::Decode(strEncodedXml, pBytes, nLength, true);
	CString strXml = (LPCTSTR)pBytes;
	delete[] pBytes;
	
	//TES 10/16/2015 - PLID 66372 - Now process the XML
	CString strSqlBatch;
	CNxParamSqlArray aryParams;

	MSXML2::IXMLDOMDocument2Ptr pDoc = NxXMLUtils::LoadXMLDocumentFromString(strXml);
	MSXML2::IXMLDOMNodePtr pTopNode = FindChildNode(pDoc, "NTOpticalShopRefractionInformation");

	//TES 10/16/2015 - PLID 66372 - Pull the type and date
	CString strRefracType;
	MSXML2::IXMLDOMNodePtr pRxInfo = FindChildNode(pTopNode, "RxInfo");
	MSXML2::IXMLDOMNodePtr pRefracType = FindChildNode(pRxInfo, "RefracType");
	strRefracType = AsString(pRefracType->text);
	MSXML2::IXMLDOMNodePtr pRxDate = FindChildNode(pRxInfo, "RxDate");
	COleDateTime dtRxDate = AsDateTime(pRxDate->text);

	MSXML2::IXMLDOMNodePtr pRefraction = FindChildNode(pTopNode, "PatientRefractionInformation");

	//TES 10/16/2015 - PLID 66372 - OD information
	_variant_t varODSphere = g_cvarNull;
	_variant_t varODCyl = g_cvarNull;
	_variant_t varODAxis = g_cvarNull;
	_variant_t varODDiameter = g_cvarNull;
	_variant_t varODAdd = g_cvarNull;
	_variant_t varODBC = g_cvarNull;
	_variant_t varODPrism1 = g_cvarNull;
	_variant_t varODBase1Num = g_cvarNull; 
	_variant_t varODBase1Text = g_cvarNull;
	_variant_t varODPrism2 = g_cvarNull;
	_variant_t varODBase2Num = g_cvarNull;
	_variant_t varODBase2Text = g_cvarNull;;
	CString strODBase1, strODBase2;
	_variant_t varODBrand = g_cvarNull; //TES 12/2/2015 - PLID 67671
	_variant_t varODPD = g_cvarNull;

	//TES 12/2/2015 - PLID 67670 - If the value is empty or "Plano", treat it as 0
	MSXML2::IXMLDOMNodePtr pODSphere = FindChildNode(pRefraction, "ODsph1");
	CString strODSphere = pODSphere == NULL ? "" : AsString(pODSphere->text);
	if (pODSphere == NULL) {
		varODSphere = g_cvarNull;
	}
	else if (strODSphere.CompareNoCase("Plano") == 0 || strODSphere.IsEmpty()) {
		varODSphere = _variant_t((double)0);
	}
	else {
		varODSphere = _variant_t(AsDouble(pODSphere->text));
	}
	//TES 12/2/2015 - PLID 67670 - If the value is empty or "Sph", treat it as 0
	MSXML2::IXMLDOMNodePtr pODCyl = FindChildNode(pRefraction, "ODcyl1");
	CString strODCyl = pODCyl == NULL ? "" : AsString(pODCyl->text);
	if (pODCyl == NULL) {
		varODCyl = g_cvarNull;
	}
	else if (strODCyl.CompareNoCase("Sph") == 0 || strODCyl.IsEmpty()) {
		varODCyl = _variant_t((double)0);
	}
	else {
		varODCyl = _variant_t(AsDouble(pODCyl->text));
	}
	MSXML2::IXMLDOMNodePtr pODAxis= FindChildNode(pRefraction, "ODaxis1");
	varODAxis = (pODAxis == NULL || AsString(pODAxis->text).IsEmpty()) ? g_cvarNull : _variant_t(AsLong(pODAxis->text));
	MSXML2::IXMLDOMNodePtr pODDiameter = FindChildNode(pRefraction, "ODdia");
	varODDiameter = (pODDiameter == NULL || AsString(pODDiameter->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pODDiameter->text));
	MSXML2::IXMLDOMNodePtr pODAdd = FindChildNode(pRefraction, "ODadd1");
	varODAdd = (pODAdd == NULL || AsString(pODAdd->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pODAdd->text));
	MSXML2::IXMLDOMNodePtr pODBC = FindChildNode(pRefraction, "ODbc1");
	if (pODBC == NULL) {
		pODBC = FindChildNode(pRefraction, "ODBCmm");
	}
	varODBC = (pODBC == NULL || AsString(pODBC->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pODBC->text));
	MSXML2::IXMLDOMNodePtr pODPrism1 = FindChildNode(pRefraction, "ODhprism");
	varODPrism1 = (pODPrism1 == NULL || AsString(pODPrism1->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pODPrism1->text));
	MSXML2::IXMLDOMNodePtr pODBase1 = FindChildNode(pRefraction, "ODhbase");
	strODBase1 = (pODBase1 == NULL) ? "" : AsPrescriptionNumber(AsString(pODBase1->text), pnfBase);
	strODBase1.Replace(" ", "");
	if (!strODBase1.IsEmpty()) {
		long nTemp = atol(strODBase1);
		if ((nTemp != 0 || strODBase1.CompareNoCase("0") == 0) && (AsString(nTemp).GetLength() == strODBase1.GetLength()))
		{
			varODBase1Num = atol(strODBase1);
		}
		else if (strODBase1.GetLength()>0) {
			varODBase1Text = _bstr_t(strODBase1);
		}
	}
	MSXML2::IXMLDOMNodePtr pODPrism2 = FindChildNode(pRefraction, "ODvprism");
	varODPrism2 = (pODPrism2 == NULL || AsString(pODPrism2->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pODPrism2->text));
	MSXML2::IXMLDOMNodePtr pODBase2 = FindChildNode(pRefraction, "ODvbase");
	strODBase2 = (pODBase2 == NULL) ? "" : AsPrescriptionNumber(AsString(pODBase2->text), pnfBase);
	strODBase2.Replace(" ", "");
	if (!strODBase2.IsEmpty()) {
		long nTemp = atol(strODBase2);
		if ((nTemp != 0 || strODBase2.CompareNoCase("0") == 0) && (AsString(nTemp).GetLength() == strODBase2.GetLength()))
		{
			varODBase2Num = atol(strODBase2);
		}
		else if (strODBase2.GetLength() > 0) {
			varODBase2Text = _bstr_t(strODBase2);
		}
	}
	//TES 12/2/2015 - PLID 67671 - Get the Brand
	MSXML2::IXMLDOMNodePtr pODBrand = FindChildNode(pRefraction, "ODBrand");
	varODBrand = pODBrand == NULL ? "" : pODBrand->text;
	MSXML2::IXMLDOMNodePtr pODPD = FindChildNode(pRefraction, "ODpd");
	CString strODPD = pODPD == NULL ? "" : AsString(pODPD->text);
	StripNonNumericChars(strODPD);
	varODPD = strODPD.IsEmpty() ? g_cvarNull : _variant_t(AsDouble(_bstr_t(strODPD)));

	//TES 10/16/2015 - PLID 66372 - OS information
	_variant_t varOSSphere = g_cvarNull;
	_variant_t varOSCyl = g_cvarNull;
	_variant_t varOSAxis = g_cvarNull;
	_variant_t varOSDiameter = g_cvarNull;
	_variant_t varOSAdd = g_cvarNull;
	_variant_t varOSBC = g_cvarNull;
	_variant_t varOSPrism1 = g_cvarNull;
	_variant_t varOSBase1Num = g_cvarNull;
	_variant_t varOSBase1Text = g_cvarNull;
	_variant_t varOSPrism2 = g_cvarNull;
	_variant_t varOSBase2Num = g_cvarNull;
	_variant_t varOSBase2Text = g_cvarNull;;
	CString strOSBase1, strOSBase2;
	_variant_t varOSBrand = g_cvarNull; //TES 12/2/2015 - PLID 67671
	_variant_t varOSPD = g_cvarNull;

	//TES 12/2/2015 - PLID 67670 - If the value is empty or "Plano", treat it as 0
	MSXML2::IXMLDOMNodePtr pOSSphere = FindChildNode(pRefraction, "OSsph1");
	CString strOSSphere = pOSSphere == NULL ? "" : AsString(pOSSphere->text);
	if (pOSSphere == NULL) {
		varOSSphere = g_cvarNull;
	}
	else if (strOSSphere.CompareNoCase("Plano") == 0 || strOSSphere.IsEmpty()) {
		varOSSphere = _variant_t((double)0);
	}
	else {
		varOSSphere = _variant_t(AsDouble(pOSSphere->text));
	}
	//TES 12/2/2015 - PLID 67670 - If the value is empty or "Sph", treat it as 0
	MSXML2::IXMLDOMNodePtr pOSCyl = FindChildNode(pRefraction, "OScyl1");
	CString strOSCyl = pOSCyl == NULL ? "" : AsString(pOSCyl->text);
	if (pOSCyl == NULL) {
		varOSCyl = g_cvarNull;
	}
	else if (strOSCyl.CompareNoCase("Sph") == 0 || strOSCyl.IsEmpty()) {
		varOSCyl = _variant_t((double)0);
	}
	else {
		varOSCyl = _variant_t(AsDouble(pOSCyl->text));
	}
	MSXML2::IXMLDOMNodePtr pOSAxis = FindChildNode(pRefraction, "OSaxis1");
	varOSAxis = (pOSAxis == NULL || AsString(pOSAxis->text).IsEmpty()) ? g_cvarNull : _variant_t(AsLong(pOSAxis->text));
	MSXML2::IXMLDOMNodePtr pOSDiameter = FindChildNode(pRefraction, "OSdia");
	varOSDiameter = (pOSDiameter == NULL || AsString(pOSDiameter->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pOSDiameter->text));
	MSXML2::IXMLDOMNodePtr pOSAdd = FindChildNode(pRefraction, "OSadd1");
	varOSAdd = (pOSAdd == NULL || AsString(pOSAdd->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pOSAdd->text));
	MSXML2::IXMLDOMNodePtr pOSBC = FindChildNode(pRefraction, "OSbc1");
	if (pOSBC == NULL) {
		pOSBC = FindChildNode(pRefraction, "OSBCmm");
	}
	varOSBC = (pOSBC == NULL || AsString(pOSBC->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pOSBC->text));
	MSXML2::IXMLDOMNodePtr pOSPrism1 = FindChildNode(pRefraction, "OShprism");
	varOSPrism1 = (pOSPrism1 == NULL || AsString(pOSPrism1->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pOSPrism1->text));
	MSXML2::IXMLDOMNodePtr pOSBase1 = FindChildNode(pRefraction, "OShbase");
	strOSBase1 = (pOSBase1 == NULL) ? "" : AsPrescriptionNumber(AsString(pOSBase1->text), pnfBase);
	strOSBase1.Replace(" ", "");
	if (!strOSBase1.IsEmpty()) {
		long nTemp = atol(strOSBase1);
		if ((nTemp != 0 || strOSBase1.CompareNoCase("0") == 0) && (AsString(nTemp).GetLength() == strOSBase1.GetLength()))
		{
			varOSBase1Num = atol(strOSBase1);
		}
		else if (strOSBase1.GetLength()>0) {
			varOSBase1Text = _bstr_t(strOSBase1);
		}
	}
	MSXML2::IXMLDOMNodePtr pOSPrism2 = FindChildNode(pRefraction, "OSvprism");
	varOSPrism2 = (pOSPrism2 == NULL || AsString(pOSPrism2->text).IsEmpty()) ? g_cvarNull : _variant_t(AsDouble(pOSPrism2->text));
	MSXML2::IXMLDOMNodePtr pOSBase2 = FindChildNode(pRefraction, "OSvbase");
	strOSBase2 = (pOSBase2 == NULL) ? "" : AsPrescriptionNumber(AsString(pOSBase2->text), pnfBase);
	strOSBase2.Replace(" ", "");
	if (!strOSBase2.IsEmpty()) {
		long nTemp = atol(strOSBase2);
		if ((nTemp != 0 || strOSBase2.CompareNoCase("0") == 0) && (AsString(nTemp).GetLength() == strOSBase2.GetLength()))
		{
			varOSBase2Num = atol(strOSBase2);
		}
		else if (strOSBase2.GetLength() > 0) {
			varOSBase2Text = _bstr_t(strOSBase2);
		}
	}
	//TES 12/2/2015 - PLID 67671 - Get the Brand
	MSXML2::IXMLDOMNodePtr pOSBrand = FindChildNode(pRefraction, "OSBrand");
	varOSBrand = pOSBrand == NULL ? "" : pOSBrand->text;
	MSXML2::IXMLDOMNodePtr pOSPD = FindChildNode(pRefraction, "OSpd");
	CString strOSPD = pOSPD == NULL ? "" : AsString(pOSPD->text);
	StripNonNumericChars(strOSPD);
	varOSPD = strOSPD.IsEmpty() ? g_cvarNull : _variant_t(AsDouble(_bstr_t(strOSPD)));

	//TES 10/16/2015 - PLID 66372 - Now commit it all to data
	long nRxType = 0;
	if (strRefracType == "RGP" || strRefracType == "Soft CL") {
		nRxType = 2;
	}
	else if (strRefracType == "Spec Rx") {
		nRxType = 1;
	}
	else {
		ThrowNxException("Unexpected RefracType value %s found in message", strRefracType);
	}
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nLensRxID INT; "
		"DECLARE @nRightRxDetailID INT; "
		"DECLARE @nLeftRxDetailID INT; ");
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO LensRxT (PersonID, RxDate,CreateDate, HL7MessageID, RxType) "
		"VALUES ({INT}, {OLEDATETIME}, getdate(), {INT}, {INT}) \r\n"
		"SELECT @nLensRxID = CONVERT(int, SCOPE_IDENTITY())",
		nPersonID, dtRxDate, Message.nMessageID, nRxType);

	//TES 12/2/2015 - PLID 67671 - Put the Brand in the Note field
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO LensDetailRxT "
		"(PrescriptionSphere, CylinderValue, CylinderAxis, AdditionValue, PrismValue, "
		"SecondaryPrismValue, PrismAxis, PrismAxisStr, PrismAxis2, PrismAxisStr2, Note, "
		"BC, Diameter, FarHalfPd) "
		"VALUES ({VT_R8}, {VT_R8}, {VT_I4}, {VT_R8}, {VT_R8}, "
		"{VT_R8}, {VT_I4}, {VT_BSTR}, {VT_I4}, {VT_BSTR}, {VT_BSTR}, "
		"{VT_R8}, {VT_R8}, {VT_R8}) \r\n"
		"SELECT @nRightRxDetailID = CONVERT(int, SCOPE_IDENTITY())",
		varODSphere, varODCyl, varODAxis, varODAdd, varODPrism1,
		varODPrism2, varODBase1Num, varODBase1Text, varODBase2Num, varODBase2Text, varODBrand,
		varODBC, varODDiameter, varODPD);

	//TES 12/2/2015 - PLID 67671 - Put the Brand in the Note field
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO LensDetailRxT "
		"(PrescriptionSphere, CylinderValue, CylinderAxis, AdditionValue, PrismValue, "
		"SecondaryPrismValue, PrismAxis, PrismAxisStr, PrismAxis2, PrismAxisStr2, Note, "
		"BC, Diameter, FarHalfPd) "
		"VALUES ({VT_R8}, {VT_R8}, {VT_I4}, {VT_R8}, {VT_R8}, "
		"{VT_R8}, {VT_I4}, {VT_BSTR}, {VT_I4}, {VT_BSTR}, {VT_BSTR}, "
		"{VT_R8}, {VT_R8}, {VT_R8}) \r\n"
		"SELECT @nLeftRxDetailID = CONVERT(int, SCOPE_IDENTITY())",
		varOSSphere, varOSCyl, varOSAxis, varOSAdd, varOSPrism1,
		varOSPrism2, varOSBase1Num, varOSBase1Text, varOSBase2Num, varOSBase2Text, varOSBrand,
		varOSBC, varOSDiameter, varOSPD);

	AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE LensRxT SET LeftLensDetailRxID = @nLeftRxDetailID, RightLensDetailRxID = @nRightRxDetailID "
		"WHERE ID = @nLensRxID");

	if (!FinalizeHL7Event(nPersonID, Message, strSqlBatch, FALSE, &aryParams)) {
		return FALSE;
	}

	return TRUE;
}

// (d.singleton 2013-01-25 15:30) - PLID 54781 copied code here to decode any ED data regardless of message type
void DecodeEncapsulatedData(const CString &strFileData, const CString &strEncodingType, const CString &strFullFileName, CFile &ImageFile)
{	
	CFileException fe;
	if(!ImageFile.Open(strFullFileName, CFile::modeWrite|CFile::modeCreate|CFile::shareExclusive, &fe)) {
		CString strError;
		fe.GetErrorMessage(strError.GetBuffer(4096), 4095);
		strError.ReleaseBuffer();
		AfxThrowNxException("HL7Utils::DecodeEncapsulatedData\r\nFailed to create %s:\r\n%s", strFullFileName, strError);
	}

	//TES 9/18/2008 - PLID 31413 - Now, decode the data based on the encoding type.
	// (b.savon 2013-05-07 15:38) - PLID 55242 - Let's fix them all do ignore the case, starting with this one.
	if(strEncodingType.CompareNoCase("A") == 0) {
		//TES 9/18/2008 - PLID 31413 - It's not encoded, sweet, just write it.
		CString strFile = strFileData;
		ImageFile.Write(strFile.GetBuffer(strFile.GetLength()), strFile.GetLength());
		strFile.ReleaseBuffer();
	}
	else if(strEncodingType.CompareNoCase("Base64") == 0) { // (b.savon 2013-05-07 15:38) - PLID 55242 - Then the one that inspired this item.
		//TES 9/18/2008 - PLID 31413 - Decode it (just call our utility function).
		BYTE *pBytes = NULL;
		int nLength = -1;
		//TES 10/14/2008 - PLID 31413 - Moved utility function to the CBase64 class, it should have been
		// there all along.
		CBase64::Decode(strFileData, pBytes, nLength);

		ImageFile.Write(pBytes, nLength);

		delete [] pBytes;
	}
	else if(strEncodingType.CompareNoCase("Hex") == 0) { // (b.savon 2013-05-07 15:41) - PLID 55242 - And finally our last supported format (at this time)
		//TES 9/18/2008 - PLID 31413 - OK, each 2-character couplet is just a hex byte, so go through and write
		// those bytes to our data.
		BYTE *pBytes = new BYTE[strFileData.GetLength()/2];
		int nIndex = 0;
		for(int i = 0; i < strFileData.GetLength()-1; i+= 2) {
			CString strByte = strFileData.Mid(i,2);
			long nByte;
			_stscanf(strByte, "%x", &nByte);
			pBytes[nIndex] = (BYTE)nByte;
			nIndex++;
		}

		ImageFile.Write(pBytes, strFileData.GetLength()/2);

		delete [] pBytes;
	}
	else {
		ThrowNxException("Invalid Encoding Type %s found in imported Image Result!", strEncodingType);
	}

	ImageFile.Close();
}

// (a.wilson 2013-06-12 09:41) - PLID 57117 - a function to commit an array of EMNIDs when an HL7 message was exported for an emn bill.
void MarkHL7EMNBillsAsSent(const CArray<long>& aryEMNIDs)
{
	//generate an xml object for ease of use in sql
	CString strXml = "<ArrayOfEMNIDs>";
	for (int i = 0; i < aryEMNIDs.GetCount(); i++) {
		strXml += FormatString("<EMNID>%li</EMNID>", aryEMNIDs.GetAt(i));
	}
	strXml += "</ArrayOfEMNIDs>";

	//execute a query to insert records where they do not exist in the table already.
	ExecuteParamSql(
		"DECLARE @EMNIDXml XML "
		"SET @EMNIDXml = {STRING} "
		"INSERT	INTO EMNBillsSentToHL7T (EMNID) "
		"SELECT	Q.ID FROM "
		"( "
		"	SELECT	EMNIDs.Record.value('(.)[1]', 'INT') ID "
		"	FROM	@EMNIDXml.nodes('ArrayOfEMNIDs/EMNID') AS EMNIDs(Record) "
		") Q "
		"WHERE Q.ID NOT IN (SELECT EMNID FROM EMNBillsSentToHL7T) ", 
		strXml);
}

//TES 1/23/2015 - PLID 55674 - Copied from NxServer's HL7Support, used to prevent logging queries from getting rolled back with failed transactions.
CNxAdoConnection GetNewAdoConnection(const CString strDatabase)
{
	CString strRegBase = GetRegistryBase();
	CNxAdoConnection con;
	BOOL bIsLan = strcmp(NxRegUtils::ReadString(strRegBase + "ConnectionType"), "IP");

	// (b.savon 2016-05-18 13:19) - NX-100670 - Check the flag to use Windows Auth for SQL connection 
	CString strSubkey = NxConnectionUtils::GetSubkey(strDatabase);
	con.Init(GetSqlServerName(), bIsLan, strDatabase, NxConnectionUtils::UseSqlIntegratedSecurity(strSubkey));
	return con;
}

// (j.jones 2015-11-16 11:01) - PLID 67491 - Modular function to create a SQL fragment that adds a new entry in HL7CodeLinkT,
// intended for use when we think no link exists, but will handle cases where a NULL practice ID exists.
// It's the caller's responsibility to audit the creation. It will still be a creation audit even if we replace a NULL entry.
CSqlFragment CreateNewHL7CodeLinkT(long nHL7GroupID, HL7CodeLink_RecordType eType, CString strThirdPartyCode, long nPracticeID)
{
	_variant_t varPracticeID = (long)nPracticeID;
	//TES 6/10/2010 - PLID 38066 - -1 is a valid PracticeID for hclrtLabSideAndQual.  Not the best decision I've ever made, 
	// but that ship has sailed.
	if (nPracticeID == -1 && eType != hclrtLabSideAndQual) {
		//if not hclrtLabSideAndQual, use NULL, not -1
		varPracticeID = g_cvarNull;
	}

	return CSqlFragment("IF EXISTS (SELECT HL7GroupID FROM HL7CodeLinkT WHERE HL7GroupID = {INT} AND ThirdPartyCode = {STRING} AND Type = {INT}) \r\n"
		"BEGIN \r\n"
		"	UPDATE HL7CodeLinkT SET PracticeID = {VT_I4} \r\n"
		"	WHERE HL7GroupID = {INT} AND ThirdPartyCode = {STRING} AND Type = {INT} \r\n" 
		"END \r\n"
		"ELSE BEGIN \r\n"
		"	INSERT INTO HL7CodeLinkT (HL7GroupID, ThirdPartyCode, PracticeID, Type) \r\n"
		"	VALUES ({INT}, {STRING}, {VT_I4}, {INT}) \r\n"
		"END \r\n",
		nHL7GroupID, strThirdPartyCode, eType,
		varPracticeID, nHL7GroupID, strThirdPartyCode, eType,
		nHL7GroupID, strThirdPartyCode, varPracticeID, eType);
}

// (j.jones 2015-11-16 11:01) - PLID 67491 - version that takes in a variable name that is an INT, like @PracticeID,
// strictly for use in batches
void CreateNewHL7CodeLinkT_WithVariableName(long nHL7GroupID, HL7CodeLink_RecordType eType, CString strThirdPartyCode, CString strPracticeIDVariableName,
	IN OUT CString &strSqlBatch, OPTIONAL IN OUT CNxParamSqlArray *paryParams /*= NULL*/)
{
	CSqlFragment sql(FormatString("IF EXISTS (SELECT HL7GroupID FROM HL7CodeLinkT WHERE HL7GroupID = {INT} AND ThirdPartyCode = {STRING} AND Type = {INT}) \r\n"
		"BEGIN \r\n"
		"	UPDATE HL7CodeLinkT SET PracticeID = %s \r\n"
		"	WHERE HL7GroupID = {INT} AND ThirdPartyCode = {STRING} AND Type = {INT} \r\n"
		"END \r\n"
		"ELSE BEGIN \r\n"
		"	INSERT INTO HL7CodeLinkT (HL7GroupID, ThirdPartyCode, PracticeID, Type) \r\n"
		"	VALUES ({INT}, {STRING}, %s, {INT}) \r\n"
		"END \r\n", strPracticeIDVariableName, strPracticeIDVariableName),
		nHL7GroupID, strThirdPartyCode, eType,
		nHL7GroupID, strThirdPartyCode, eType,
		nHL7GroupID, strThirdPartyCode, eType);

	if (paryParams) {
		AddParamStatementToSqlBatch(strSqlBatch, *paryParams, "{SQL} ", sql);
	}
	else {
		// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
		AddStatementToSqlBatch(strSqlBatch, "%s ", sql.Flatten());
	}
}