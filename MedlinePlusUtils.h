#pragma once

// (j.jones 2013-10-17 09:00) - PLID 57503 - checks to see if the user has been warned
// about the Medline Plus content, and if not, warns them
void CheckWarnMedlinePlusInfo(CWnd *pParentWnd);

// (j.jones 2013-10-17 09:02) - PLID 57503 - Used in the call to LookupMedlinePlusInformation,
// this is not saved in data.
enum MedlinePlusIDType
{
	mlpDiagCodeID = 0,
	mlpDrugListID,
	mlpLabResultID,
	mlpLabID,
};
// (r.gonet 10/30/2013) - PLID 58980 - Method of retrieval for patient education resources.
enum MedlinePlusMethod
{
	// (r.gonet 10/30/2013) - PLID 58980 - By the HL7 InfoButton standard method.
	mlpmInfoButton = 0,
	// (r.gonet 10/30/2013) - PLID 58980 - By the alternate MedlinePlus Search method.
	mlpmSearch,
};

// (r.gonet 10/30/2013) - PLID 58980 - Looks up patient education resources by using the MedlinePlus search web service.
// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which we will try to lookup patient education with
// in the case where the primary record ID fails to find any results.
void LookupMedlinePlusInformationViaSearch(CWnd *pParentWnd, MedlinePlusIDType eIDType, long nID, const CArray<long, long>& aryAlternateRecordIDs = CArray<long, long>());
// (j.jones 2013-10-17 09:01) - PLID 57503 - given a record ID and an enum that
// identifies the record type (such as a DiagCodes.ID, DrugList.ID, or LabResultsT.ID),
// this function will have the API generate the proper Medline Plus query and end it
// to the system web browser
// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which we will try to lookup patient education with
// in the case where the primary record ID fails to find any results.
void LookupMedlinePlusInformationViaURL(CWnd *pParentWnd, MedlinePlusIDType eIDType, long nID, const CArray<long, long>& aryAlternateRecordIDs = CArray<long, long>());
// (r.gonet 10/30/2013) - PLID 58980 - Extracted method. Added a way to use either the InfoButton method or the Search method
// when getting patient education resources.
// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which we will try to lookup patient education with
// in the case where the primary record ID fails to find any results.
void LookupMedlinePlusInformation(CWnd *pParentWnd, MedlinePlusIDType mlpIDType, long nID, const CArray<long, long>& aryAlternateRecordIDs, MedlinePlusMethod mlpmMethod);