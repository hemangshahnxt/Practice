#ifndef HL7SETTINGS_H
#define HL7SETTINGS_H

#pragma once



#include <afxtempl.h>
#include "GlobalLabUtils.h"
#include "HL7ParseUtils.h"

class CHL7Settings
{
private:
	struct HL7Group {
		long nID;
		CString strGroupName;
		
		long nExportType;
		CString strExportAddr;
		CString strExportPort;
		CString strExportFile;
		CString strExportBeginChars;
		CString strExportEndChars;

		long nImportType;
		CString strImportAddr;
		CString strImportPort;
		CString strImportFile;
		CString strImportExtension;

		BOOL bExportNewPatients;
		BOOL bExpectACK;
		BOOL bSendACK;
		BOOL bUseBillDate;
		BOOL bUseImportedLocation;
		BOOL bUseImportedLocationAsPOS;

		// (j.gruber 2007-08-23 17:32) - PLID 24628 - Auto Update
		BOOL bExportUpdatedPatients;

		// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
		BOOL bBatchImports;
		BOOL bBatchExports;

		// (z.manning 2008-07-16 10:05) - PLID 30753 - Added option for exporting appointments
		BOOL bExportAppts;

		//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
		BOOL bExportEmnBills;

		//TES 10/10/2008 - PLID 21093 - Added option for default Lab Procedure
		long nDefaultLabProcedureID;

		// (z.manning 2008-12-22 16:37) - PLID 32250 - Option to export insurance
		BOOL bExportInsurance;

		// (z.manning 2009-06-15 10:54) - PLID 34623 - Keep track of lab procedure type too
		LabType eLabType;

		//TES 2/8/2010 - PLID 37268 - Added setting for whether to pull the result diagnosis from OBX-13.
		BOOL bUseOBX13;

		//TES 2/23/2010 - PLID 37503 - Added setting to pull the Receiving Lab from OBX
		//BOOL bUseOBXLocation;
		//TES 4/29/2011 - PLID 43424 - Replaced UseOBXLocation with an enum for the possible places
		// to pull the Receiving Lab from
		ReceivingLabLocation rllReceivingLabLocation;


		//TES 4/2/2010 - PLID 38040 - Added setting to pull Anatomic Location from OBR-15
		long nUseOBR15Component;

		// (a.vengrofski 2010-05-11 17:05) - PLID <38547>
		BOOL bAutoExoprtLabs;

		// (a.vengrofski 2010-05-11 17:05) - PLID <38547>
		BOOL nDefaultLabID;

		// (z.manning 2010-05-21 09:47) - PLID 38638
		BOOL bAutoImportLabs;
		DWORD nLabImportMatchingFieldFlags;

		//TES 5/21/2010 - PLID 38785 - Added a setting to combine text segments into a single result
		//TES 6/14/2010 - PLID 39135 - Changed from a BOOL to an enum.
		CombineTextSegmentsOptions ctsoCombineTextSegments;

		//TES 6/11/2010 - PLID 38541 - Setting for the Facility ID on new lab orders
		CString strLabFacilityID;

		//TES 7/19/2010 - PLID 39720 - Setting for whether to include insurance information when importing demographics.
		BOOL bImportInsurance;

		//TES 7/30/2010 - PLID 39908 - Setting for values in OBR-24 that represent text segments that should be combined into
		// a single result.
		CString strOBR24Values;

		//TES 8/2/2010 - PLID 39935 - Setting for exporting patient demographics as A28/A31 instead of A04/A08
		BOOL bSendA2831;

		// (z.manning 2010-08-09 09:44) - PLID 39985 - Auto create patients from scheduler messages
		BOOL bAutoCreatePatientsFromSiu;

		// (c.haag 2010-09-14 15:06) - PLID 40176
		long nLabSpecimenParseMethod;

		// (r.gonet 2011-03-09 11:29) - PLID 42655
		BOOL bCopyResult;

		//TES 9/20/2010 - PLID 40595
		BOOL bExcludeProspects;

		// (z.manning 2010-10-01 15:22) - PLID 40654
		CString strFilenamePrefix;

		// (z.manning 2010-10-04 16:54) - PLID 40795
		BOOL bA28AddNewOnly;

		//TES 3/10/2011 - PLID 41912
		BOOL bForceFacilityIDMatch;

		// (z.manning 2011-04-21 12:02) - PLID 43361
		CString strScheduleExportVersion;

		//TES 4/25/2011 - PLID 43423
		BOOL bUseOBX3_5;

		//TES 5/11/2011 - PLID 43634
		BOOL bAppendObr20;

		HL7Group() {
			nID = -1;
			nExportType = nImportType = 0;
			nDefaultLabProcedureID = -1;
			bExportNewPatients = bExpectACK = bSendACK = bUseBillDate = bUseImportedLocation = bUseImportedLocationAsPOS = FALSE;
			// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
			bBatchImports = TRUE;
			bBatchExports = FALSE;
			eLabType = ltInvalid;
			bUseOBX13 = TRUE;
			//bUseOBXLocation = FALSE;
			//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
			rllReceivingLabLocation = rllMSH4;
			//TES 4/6/2010 - PLID 38040 - Default to pulling the anatomic location from the first component of OBR-15.
			nUseOBR15Component = 1;
			// (a.vengrofski 2010-05-11 17:23) - PLID <38547>
			bAutoExoprtLabs = FALSE;
			// (a.vengrofski 2010-05-11 17:23) - PLID <38547>
			nDefaultLabID = -1;
			bAutoImportLabs = FALSE;
			nLabImportMatchingFieldFlags = 0;
			//TES 5/21/2010 - PLID 38785 - Default to off (current behavior).
			//TES 6/14/2010 - PLID 39135 - Changed to an enum
			ctsoCombineTextSegments = ctsoCheckSubGroupID;
			//TES 7/19/2010 - PLID 39720
			bImportInsurance = TRUE;
			//TES 8/2/2010 - PLID 39935
			bSendA2831 = FALSE;
			bAutoCreatePatientsFromSiu = FALSE; // (z.manning 2010-08-09 09:49) - PLID 39985
			// (c.haag 2010-09-14 15:06) - PLID 40176
			nLabSpecimenParseMethod = 1;
			// (r.gonet 2011-03-09 11:30) - PLID 42655
			bCopyResult = FALSE;
			bExcludeProspects = FALSE; //TES 9/20/2010 - PLID 40595
			bA28AddNewOnly = FALSE;
			//TES 3/10/2011 - PLID 41912
			bForceFacilityIDMatch = FALSE;
			//TES 4/25/2011 - PLID 43423
			bUseOBX3_5 = FALSE;
			//TES 5/11/2011 - PLID 43634
			bAppendObr20 = FALSE;
		}
	};

	CArray<HL7Group,HL7Group&> m_arCachedGroups;

	//Returns the index in data of the specified group, loading it first if necessary.
	static int GetHL7GroupIndex(long nHL7GroupID);

	//Accessors, non-static.
	CString GetGroupNamePrivate(long nHL7GroupID);
	
	long GetExportTypePrivate(long nHL7GroupID);	
	CString GetExportAddrPrivate(long nHL7GroupID);
	CString GetExportPortPrivate(long nHL7GroupID);
	CString GetExportFilePrivate(long nHL7GroupID);
	CString GetExportBeginCharsPrivate(long nHL7GroupID);
	CString GetExportEndCharsPrivate(long nHL7GroupID);
	
	long GetImportTypePrivate(long nHL7GroupID);	
	CString GetImportAddrPrivate(long nHL7GroupID);
	CString GetImportPortPrivate(long nHL7GroupID);
	CString GetImportFilePrivate(long nHL7GroupID);
	CString GetImportExtensionPrivate(long nHL7GroupID);

	BOOL GetExportNewPatientsPrivate(long nHL7GroupID);
	BOOL GetExpectACKPrivate(long nHL7GroupID);
	BOOL GetSendACKPrivate(long nHL7GroupID);
	BOOL GetUseBillDatePrivate(long nHL7GroupID);
	BOOL GetUseImportedLocationPrivate(long nHL7GroupID);
	BOOL GetUseImportedLocationAsPOSPrivate(long nHL7GroupID);

	BOOL GetExportUpdatedPatientsPrivate(long nHL7GroupID);

	// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
	BOOL GetBatchImportsPrivate(long nHL7GroupID);
	BOOL GetBatchExportsPrivate(long nHL7GroupID);

	// (z.manning 2008-07-16 10:09) - PLID 30753
	BOOL GetExportApptsPrivate(long nHL7GroupID);

	//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
	BOOL GetExportEmnBillsPrivate(long nHL7GroupID);

	//TES 10/10/2008 - PLID 21093
	long GetDefaultLabProcedureIDPrivate(long nHL7GroupID);

	// (z.manning 2008-12-22 16:40) - PLID 32250
	BOOL GetExportInsurancePrivate(long nHL7GroupID);

	// (z.manning 2009-06-15 11:04) - PLID 34623
	LabType GetLabProcedureTypePrivate(long nHL7GroupID);

	//TES 2/8/2010 - PLID 37268
	BOOL GetUseOBX13Private(long nHL7GroupID);

	////TES 2/23/2010 - PLID 37503
	//BOOL GetUseOBXLocationPrivate(long nHL7GroupID);
	//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
	ReceivingLabLocation GetReceivingLabLocationPrivate(long nHL7GroupID);

	//TES 4/2/2010 - PLID 38040
	long GetUseOBR15ComponentPrivate(long nHL7GroupID);

	// (a.vengrofski 2010-05-11 17:00) - PLID <38547>
	BOOL GetAutoExoprtLabsPrivate(long nHL7GroupID);

	// (a.vengrofski 2010-05-11 17:00) - PLID <38547>
	long GetDefaultLabIDPrivate(long nHL7GroupID);

	// (z.manning 2010-05-21 09:50) - PLID 38638
	BOOL GetAutoImportLabsPrivate(long nHL7GroupID);
	DWORD GetLabImportMatchingFieldFlagsPrivate(long nHL7GroupID);

	//TES 5/21/2010 - PLID 38785
	//TES 6/14/2010 - PLID 39135 - Changed to an enum
	CombineTextSegmentsOptions GetCombineTextSegmentsPrivate(long nHL7GroupID);

	//TES 6/11/2010 - PLID 38541
	CString GetLabFacilityIDPrivate(long nHL7GroupID);

	//TES 7/19/2010 - PLID 39720
	BOOL GetImportInsurancePrivate(long nHL7GroupID);

	//TES 7/30/2010 - PLID 39908
	CString GetOBR24ValuesPrivate(long nHL7GroupID);

	//TES 8/2/2010 - PLID 39935
	BOOL GetSendA2831Private(long nHL7GroupID);

	// (z.manning 2010-08-09 09:49) - PLID 39985
	BOOL GetAutoCreatePatientsFromSiuPrivate(long nHL7GroupID);

	// (c.haag 2010-09-14 15:06) - PLID 40176
	long GetLabSpecimenParseMethodPrivate(long nHL7GroupID);

	// (r.gonet 2011-03-09 11:31) - PLID 42655
	BOOL GetCopyResultPrivate(long nHL7GroupID);

	//TES 9/20/2010 - PLID 40595
	BOOL GetExcludeProspectsPrivate(long nHL7GroupID);

	// (z.manning 2010-10-01 15:23) - PLID 40654
	CString GetFilenamePrefixPrivate(long nHL7GroupID);

	// (z.manning 2010-10-04 16:55) - PLID 40795
	BOOL GetA28AddNewOnlyPrivate(long nHL7GroupID);

	//TES 3/10/2011 - PLID 41912
	BOOL GetForceFacilityIDMatchPrivate(long nHL7GroupID);

	// (z.manning 2011-04-21 12:02) - PLID 43361
	CString GetScheduleExportVersionPrivate(const long nHL7GroupID);

	//TES 4/25/2011 - PLID 43423
	BOOL GetUseOBX3_5Private(long nHL7GroupID);

	//TES 5/11/2011 - PLID 43634
	BOOL GetAppendOBR20Private(long nHL7GroupID);



public:
	//Static accessors, these all pull from the global CHL7Settings object.
	static CString GetGroupName(long nHL7GroupID);
	
	static long GetExportType(long nHL7GroupID);	
	static CString GetExportAddr(long nHL7GroupID);
	static CString GetExportPort(long nHL7GroupID);
	static CString GetExportFile(long nHL7GroupID);
	static CString GetExportBeginChars(long nHL7GroupID);
	static CString GetExportEndChars(long nHL7GroupID);
	
	static long GetImportType(long nHL7GroupID);	
	static CString GetImportAddr(long nHL7GroupID);
	static CString GetImportPort(long nHL7GroupID);
	static CString GetImportFile(long nHL7GroupID);
	static CString GetImportExtension(long nHL7GroupID);

	static BOOL GetExportNewPatients(long nHL7GroupID);
	static BOOL GetExpectACK(long nHL7GroupID);
	static BOOL GetSendACK(long nHL7GroupID);
	static BOOL GetUseBillDate(long nHL7GroupID);
	static BOOL GetUseImportedLocation(long nHL7GroupID);
	static BOOL GetUseImportedLocationAsPOS(long nHL7GroupID);

	static BOOL GetExportUpdatedPatients(long nHL7GroupID);

	// (j.jones 2008-04-24 14:11) - PLID 29600 - added bBatchImports and bBatchExports
	static BOOL GetBatchImports(long nHL7GroupID);
	static BOOL GetBatchExports(long nHL7GroupID);

	// (z.manning 2008-07-16 10:09) - PLID 30753
	static BOOL GetExportAppts(long nHL7GroupID);

	//TES 7/10/2009 - PLID 34856 - Added option for exporting billing from EMR
	static BOOL GetExportEmnBills(long nHL7GroupID);

	//TES 10/10/2008 - PLID 21093
	static long GetDefaultLabProcedureID(long nHL7GroupID);

	// (z.manning 2008-12-22 16:40) - PLID 32550
	static BOOL GetExportInsurance(long nHL7GroupID);

	static LabType GetLabProcedureType(long nHL7GroupID);

	//TES 2/8/2010 - PLID 37268
	static BOOL GetUseOBX13(long nHL7GroupID);

	//TES 2/23/2010 - PLID 37503
	//static BOOL GetUseOBXLocation(long nHL7GroupID);
	//TES 4/29/2011 - PLID 43424 - Replaced bUseOBXLocation with rllReceivingLabLocation
	static ReceivingLabLocation GetReceivingLabLocation(long nHL7GroupID);

	//TES 4/2/2010 - PLID 38040
	static long GetUseOBR15Component(long nHL7GroupID);

	// (a.vengrofski 2010-05-11 17:00) - PLID <38547>
	static BOOL GetAutoExoprtLabs(long nHL7GroupID);

	// (a.vengrofski 2010-05-11 17:00) - PLID <38547>
	static long GetDefaultLabID(long nHL7GroupID);

	// (z.manning 2010-05-21 09:51) - PLID 38638
	static BOOL GetAutoImportLabs(long nHL7GroupID);
	static DWORD GetLabImportMatchingFieldFlags(long nHL7GroupID);

	//TES 5/21/2010 - PLID 38785
	//TES 6/14/2010 - PLID 39135 - Changed to an enum
	static CombineTextSegmentsOptions GetCombineTextSegments(long nHL7GroupID);

	//TES 6/11/2010 - PLID 38541
	static CString GetLabFacilityID(long nHL7GroupID);

	//TES 7/19/2010 - PLID 39720
	static BOOL GetImportInsurance(long nHL7GroupID);

	//TES 7/30/2010 - PLID 39908
	static void GetOBR24Values(long nHL7GroupID, CStringArray &saOBR24Fields);

	//TES 8/2/2010 - PLID 39935
	static BOOL GetSendA2831(long nHL7GroupID);

	// (z.manning 2010-08-09 09:50) - PLID 39985
	static BOOL GetAutoCreatePatientsFromSiu(long nHL7GroupID);

	// (c.haag 2010-09-14 15:06) - PLID 40176
	static long GetLabSpecimenParseMethod(long nHL7GroupID);

	// (r.gonet 2011-03-09 11:31) - PLID 42655
	static BOOL GetCopyResult(long nHl7GroupID);

	//TES 9/20/2010 - PLID 40595
	static BOOL GetExcludeProspects(long nHL7GroupID);

	// (z.manning 2010-10-01 15:24) - PLID 40654
	static CString GetFilenamePrefix(long nHL7GroupID);

	// (z.manning 2010-10-04 16:55) - PLID 40795
	static BOOL GetA28AddNewOnly(long nHL7GroupID);

	//TES 3/10/2011 - PLID 41912
	static BOOL GetForceFacilityIDMatch(long nHL7GroupID);

	// (z.manning 2011-04-21 12:03) - PLID 43361
	static CString GetScheduleExportVersion(const long nHL7GroupID);

	//TES 4/25/2011 - PLID 43423
	static BOOL GetUseOBX3_5(long nHL7GroupID);

	//TES 5/11/2011 - PLID 43634
	static BOOL GetAppendOBR20(long nHL7GroupID);

	//Clears the cache for a given setting.
	static void UnloadGroup(long nHL7GroupID);
};







#endif