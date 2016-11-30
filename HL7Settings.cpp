//HL7Settings.cpp
#include "stdafx.h"
#include "HL7Settings.h"
#include "GlobalLabUtils.h"

//The one and only actual CHL7Settings object, all the public functions are static, and reference this global object.
CHL7Settings g_HL7Settings;

int CHL7Settings::GetHL7GroupIndex(long nHL7GroupID)
{
	//Have we already loaded it?
	for(int i = 0; i < g_HL7Settings.m_arCachedGroups.GetSize(); i++) {
		if(g_HL7Settings.m_arCachedGroups[i].nID == nHL7GroupID) return i;
	}
	//Nope, load it.
	// (z.manning 2009-06-15 10:58) - PLID 34623 - Also load the lab procedure type
	ADODB::_RecordsetPtr rsHL7Settings = CreateParamRecordset(
		"SELECT HL7SettingsT.*, LabProceduresT.Type AS LabProcedureType \r\n"
		"FROM HL7SettingsT \r\n"
		"LEFT JOIN LabProceduresT ON HL7SettingsT.DefaultLabProcedureID = LabProceduresT.ID \r\n"
		"WHERE HL7SettingsT.ID = {INT} \r\n"
		, nHL7GroupID);
	if(rsHL7Settings->eof) {
		AfxThrowNxException("Invalid HL7SettingsT.ID passed to GetHL7GroupIndex()");
		return -1;
	}
	ADODB::FieldsPtr f = rsHL7Settings->Fields;
	HL7Group group;
	group.nID = nHL7GroupID;
	group.strGroupName = AdoFldString(f, "Name", "");
	
	group.nExportType = AdoFldLong(f, "ExportType");
	group.strExportAddr = AdoFldString(f, "ExportAddr", "");
	group.strExportPort = AdoFldString(f, "ExportPort", "");
	group.strExportFile = AdoFldString(f, "ExportFile", "");
	group.strExportBeginChars = AdoFldString(f, "ExportBeginChars", "");
	group.strExportEndChars = AdoFldString(f, "ExportEndChars", "");
	
	group.nImportType = AdoFldLong(f, "ImportType");
	group.strImportAddr = AdoFldString(f, "ImportAddr", "");
	group.strImportPort = AdoFldString(f, "ImportPort", "");
	group.strImportFile = AdoFldString(f, "ImportFile", "");
	group.strImportExtension = AdoFldString(f, "ImportExtension","");

	group.bExportNewPatients = AdoFldBool(f, "ExportNewPatients");
	group.bExpectACK = AdoFldBool(f, "ExpectACK");
	group.bSendACK = AdoFldBool(f, "SendACK");
	group.bUseBillDate = AdoFldBool(f, "UseBillDate");
	group.bUseImportedLocation = AdoFldBool(f, "UseImportedLocation");
	group.bUseImportedLocationAsPOS = AdoFldBool(f, "UseImportedLocationAsPOS");

	// (j.gruber 2007-08-23 17:31) - PLID 24628 - auto update
	group.bExportUpdatedPatients = AdoFldBool(f, "ExportUpdatedPatients");

	// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
	group.bBatchImports = AdoFldBool(f, "BatchImports");
	group.bBatchExports = AdoFldBool(f, "BatchExports");

	// (z.manning 2008-07-16 10:07) - PLID 30753 - Added option to export appts
	group.bExportAppts = AdoFldBool(f, "ExportAppts");

	//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
	group.bExportEmnBills = AdoFldBool(f, "ExportEmnBills");

	//TES 10/10/2008 - PLID 21093 - Added option for default Lab Procedure
	group.nDefaultLabProcedureID = AdoFldLong(f, "DefaultLabProcedureID", -1);

	// (z.manning 2008-12-22 16:37) - PLID 32250 - Option to export insurance
	group.bExportInsurance = AdoFldBool(f, "ExportInsurance");

	// (z.manning 2009-06-15 10:59) - PLID 34623 - Lab procedure type
	_variant_t varLabType = f->GetItem("LabProcedureType")->GetValue();
	if(varLabType.vt == VT_NULL) {
		group.eLabType = ltInvalid;
	}
	else {
		group.eLabType = (LabType)VarByte(varLabType);
	}

	//TES 2/8/2010 - PLID 37268 - Added setting for whether to pull the result diagnosis from OBX-13
	group.bUseOBX13 = AdoFldBool(f, "UseOBX13");

	//TES 2/23/2010 - PLID 37503 - Added setting to pull the Receiving Lab from OBX
	//group.bUseOBXLocation = AdoFldBool(f, "UseOBXLocation");
	//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
	group.rllReceivingLabLocation = (ReceivingLabLocation)AdoFldLong(f, "ReceivingLabLocation");

	//TES 4/2/2010 - PLID 38040 - Added setting to pull Anatomic Location from OBR-15
	group.nUseOBR15Component = AdoFldLong(f, "AnatomicLocationComponent", -1);

	// (a.vengrofski 2010-05-11 17:07) - PLID <38547> - Added setting to Automatically Export Labs
	group.bAutoExoprtLabs = AdoFldBool(f, "AutoExportLabs");

	// (a.vengrofski 2010-05-11 17:07) - PLID <38547> - Added setting to Automatically Export Labs
	group.nDefaultLabID = AdoFldLong(f, "DefaultLabID", (long)-1);

	// (z.manning 2010-05-21 10:01) - PLID 38638
	group.bAutoImportLabs = AdoFldBool(f, "AutoImportLabs");
	group.nLabImportMatchingFieldFlags = AdoFldLong(f, "LabImportMatchingFieldFlags");

	//TES 5/21/2010 - PLID 38785 - Added a setting to combine text segments into a single result
	//TES 6/14/2010 - PLID 39135 - Changed to an enum
	group.ctsoCombineTextSegments = (CombineTextSegmentsOptions)AdoFldLong(f, "CombineTextSegments");

	//TES 6/11/2010 - PLID 38541 - Added a setting for the Lab facility ID
	group.strLabFacilityID = AdoFldString(f, "LabFacilityID");

	//TES 7/19/2010 - PLID 39720 - Added a setting for whether to include insurance information when importing demographics.
	group.bImportInsurance = AdoFldBool(f, "ImportInsurance");

	//TES 7/30/2010 - PLID 39908
	group.strOBR24Values = AdoFldString(f, "OBR24Values");

	//TES 8/2/2010 - PLID 39935
	group.bSendA2831 = AdoFldBool(f, "SendA2831");

	// (z.manning 2010-08-09 09:50) - PLID 39985
	group.bAutoCreatePatientsFromSiu = AdoFldBool(f, "AutoCreatePatientsFromScheduleMessages");

	// (c.haag 2010-09-16 12:27) - PLID 40176
	group.nLabSpecimenParseMethod = AdoFldLong(f, "LabSpecimenParseMethod");

	// (r.gonet 2011-03-09 11:32) - PLID 42655
	group.bCopyResult = AdoFldBool(f, "CopyResult");

	//TES 9/20/2010 - PLID 40595
	group.bExcludeProspects = AdoFldBool(f, "ExcludeProspects");

	// (z.manning 2010-10-01 15:26) - PLID 40654
	group.strFilenamePrefix = AdoFldString(f, "FilenamePrefix");

	// (z.manning 2010-10-04 16:55) - PLID 40795
	group.bA28AddNewOnly = AdoFldBool(f, "A28AddNewOnly");

	//TES 3/10/2011 - PLID 41912
	group.bForceFacilityIDMatch = AdoFldBool(f, "ForceFacilityIDMatch");

	// (z.manning 2011-04-21 12:04) - PLID 43361
	group.strScheduleExportVersion = AdoFldString(f, "ScheduleExportVersion");

	//TES 4/25/2011 - PLID 43423
	group.bUseOBX3_5 = AdoFldBool(f, "UseOBX3_5");

	//TES 5/11/2011 - PLID 43634
	group.bAppendObr20 = AdoFldBool(f, "AppendObr20");

	g_HL7Settings.m_arCachedGroups.Add(group);
	return g_HL7Settings.m_arCachedGroups.GetSize()-1;	
}

#define IMPLEMENT_ACCESSOR(type, name, variable)	type CHL7Settings::Get##name##Private(long nHL7GroupID) \
{ \
	HL7Group group = m_arCachedGroups[GetHL7GroupIndex(nHL7GroupID)]; \
	return group.variable;\
}\
type CHL7Settings::Get##name##(long nHL7GroupID) \
{ \
	return g_HL7Settings.Get##name##Private(nHL7GroupID); \
}

IMPLEMENT_ACCESSOR(CString, GroupName, strGroupName)
IMPLEMENT_ACCESSOR(long, ExportType, nExportType)
IMPLEMENT_ACCESSOR(CString, ExportAddr, strExportAddr)
IMPLEMENT_ACCESSOR(CString, ExportPort, strExportPort)
IMPLEMENT_ACCESSOR(CString, ExportFile, strExportFile)
IMPLEMENT_ACCESSOR(CString, ExportBeginChars, strExportBeginChars)
IMPLEMENT_ACCESSOR(CString, ExportEndChars, strExportEndChars)

IMPLEMENT_ACCESSOR(long, ImportType, nImportType)
IMPLEMENT_ACCESSOR(CString, ImportAddr, strImportAddr)
IMPLEMENT_ACCESSOR(CString, ImportPort, strImportPort)
IMPLEMENT_ACCESSOR(CString, ImportFile, strImportFile)
IMPLEMENT_ACCESSOR(CString, ImportExtension, strImportExtension)

IMPLEMENT_ACCESSOR(BOOL, ExportNewPatients, bExportNewPatients)
IMPLEMENT_ACCESSOR(BOOL, ExpectACK, bExpectACK)
IMPLEMENT_ACCESSOR(BOOL, SendACK, bSendACK)
IMPLEMENT_ACCESSOR(BOOL, UseBillDate, bUseBillDate)
IMPLEMENT_ACCESSOR(BOOL, UseImportedLocation, bUseImportedLocation)
IMPLEMENT_ACCESSOR(BOOL, UseImportedLocationAsPOS, bUseImportedLocationAsPOS)

// (j.gruber 2007-08-23 17:30) - PLID 24628 - automatically update patients
IMPLEMENT_ACCESSOR(BOOL, ExportUpdatedPatients, bExportUpdatedPatients)

// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
IMPLEMENT_ACCESSOR(BOOL, BatchImports, bBatchImports)
IMPLEMENT_ACCESSOR(BOOL, BatchExports, bBatchExports)

// (z.manning 2008-07-16 10:08) - PLID 30753
IMPLEMENT_ACCESSOR(BOOL, ExportAppts, bExportAppts)

//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
IMPLEMENT_ACCESSOR(BOOL, ExportEmnBills, bExportEmnBills)

//TES 10/10/2008 - PLID 21093
IMPLEMENT_ACCESSOR(long, DefaultLabProcedureID, nDefaultLabProcedureID)

// (z.manning 2008-12-22 16:38) - PLID 32550
IMPLEMENT_ACCESSOR(BOOL, ExportInsurance, bExportInsurance)

// (z.manning 2009-06-15 11:01) - PLID 34623
IMPLEMENT_ACCESSOR(LabType, LabProcedureType, eLabType)

//TES 2/8/2010 - PLID 37268
IMPLEMENT_ACCESSOR(BOOL, UseOBX13, bUseOBX13)

//TES 2/23/2010 - PLID 37503
//IMPLEMENT_ACCESSOR(BOOL, UseOBXLocation, bUseOBXLocation)
//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
IMPLEMENT_ACCESSOR(ReceivingLabLocation, ReceivingLabLocation, rllReceivingLabLocation);

//TES 4/2/2010 - PLID 38040
IMPLEMENT_ACCESSOR(long, UseOBR15Component, nUseOBR15Component);

// (a.vengrofski 2010-05-11 16:57) - PLID <38547>
IMPLEMENT_ACCESSOR(BOOL, AutoExoprtLabs, bAutoExoprtLabs)

// (a.vengrofski 2010-05-11 16:57) - PLID <38547>
IMPLEMENT_ACCESSOR(long, DefaultLabID, nDefaultLabID)

// (z.manning 2010-05-21 10:02) - PLID 38639
IMPLEMENT_ACCESSOR(BOOL, AutoImportLabs, bAutoImportLabs);
IMPLEMENT_ACCESSOR(DWORD, LabImportMatchingFieldFlags, nLabImportMatchingFieldFlags);

//TES 5/21/2010 - PLID 38785
//TES 6/14/2010 - PLID 39135 - Changed to an enum
IMPLEMENT_ACCESSOR(CombineTextSegmentsOptions, CombineTextSegments, ctsoCombineTextSegments);

//TES 6/11/2010 - PLID 38541
IMPLEMENT_ACCESSOR(CString, LabFacilityID, strLabFacilityID);

//TES 7/19/2010 - PLID 39720
IMPLEMENT_ACCESSOR(BOOL, ImportInsurance, bImportInsurance);

//TES 7/30/2010 - PLID 39908 - We can't use the usual accessor for this, because we store as a |-delimited string, but
// we actually output a CStringArray.
CString CHL7Settings::GetOBR24ValuesPrivate(long nHL7GroupID) 
{ 
	HL7Group group = m_arCachedGroups[GetHL7GroupIndex(nHL7GroupID)]; 
	return group.strOBR24Values;
}

void CHL7Settings::GetOBR24Values(long nHL7GroupID, CStringArray &saOBR24Values) 
{ 
	CString strOBR24Values = g_HL7Settings.GetOBR24ValuesPrivate(nHL7GroupID);
	//TES 7/30/2010 - PLID 39908 - Transform the |-delimited string into a CStringArray
	int nTok = strOBR24Values.Find("|");
	while(nTok != -1) {
		CString strField = strOBR24Values.Left(nTok);
		saOBR24Values.Add(strField);
		strOBR24Values = strOBR24Values.Mid(nTok+1);
		nTok = strOBR24Values.Find("|");
	}
	if(!strOBR24Values.IsEmpty()) {
		saOBR24Values.Add(strOBR24Values);
	}
}

//TES 8/2/2010 - PLID 39935
IMPLEMENT_ACCESSOR(BOOL, SendA2831, bSendA2831);

// (z.manning 2010-08-09 09:50) - PLID 39985
IMPLEMENT_ACCESSOR(BOOL, AutoCreatePatientsFromSiu, bAutoCreatePatientsFromSiu);

//TES 3/10/2011 - PLID 41912
IMPLEMENT_ACCESSOR(BOOL, ForceFacilityIDMatch, bForceFacilityIDMatch);

//TES 4/25/2011 - PLID 43423
IMPLEMENT_ACCESSOR(BOOL, UseOBX3_5, bUseOBX3_5);

//TES 5/11/2011 - PLID 43634
IMPLEMENT_ACCESSOR(BOOL, AppendOBR20, bAppendObr20);

void CHL7Settings::UnloadGroup(long nHL7GroupID)
{
	if(nHL7GroupID == -1) {
		g_HL7Settings.m_arCachedGroups.RemoveAll();
		return;
	}

	for(int i = 0; i < g_HL7Settings.m_arCachedGroups.GetSize(); i++) {
		if(g_HL7Settings.m_arCachedGroups[i].nID == nHL7GroupID) {
			g_HL7Settings.m_arCachedGroups.RemoveAt(i);
			return;
		}
	}
}

// (c.haag 2010-09-14 15:06) - PLID 40176
IMPLEMENT_ACCESSOR(long, LabSpecimenParseMethod, nLabSpecimenParseMethod)

// (r.gonet 2011-03-09 11:32) - PLID 42655
IMPLEMENT_ACCESSOR(BOOL, CopyResult, bCopyResult)

//TES 9/20/2010 - PLID 40595
IMPLEMENT_ACCESSOR(BOOL, ExcludeProspects, bExcludeProspects)

// (z.manning 2010-10-01 15:29) - PLID 40654 
IMPLEMENT_ACCESSOR(CString, FilenamePrefix, strFilenamePrefix)

// (z.manning 2010-10-04 16:56) - PLID 40795
IMPLEMENT_ACCESSOR(BOOL, A28AddNewOnly, bA28AddNewOnly)

// (z.manning 2011-04-21 12:04) - PLID 43361
IMPLEMENT_ACCESSOR(CString, ScheduleExportVersion, strScheduleExportVersion)