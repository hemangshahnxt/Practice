#include "stdafx.h"
#include "MedlinePlusUtils.h"
#include "DontShowDlg.h"
#include "NxAPI.h"

// (j.jones 2013-10-17 09:00) - PLID 57503 - checks to see if the user has been warned
// about the Medline Plus content, and if not, warns them
void CheckWarnMedlinePlusInfo(CWnd *pParentWnd)
{
	//this should never be null
	if(pParentWnd == NULL) {
		ASSERT(FALSE); //for debugging
		ThrowNxException("CheckWarnMedlinePlusInfo called with a null pParentWnd.");
	}

	DontShowMeAgain(pParentWnd, "MedlinePlus is a free service of the National Library of Medicine (NLM), "
		"National Institutes of Health (NIH), and the Department of Health and Human Services (HHS).\n\n"
		"NexTech is not responsible for the content of this website. It is the responsibility of the user "
		"to trust and verify the information provided by MedlinePlus.", "MedlinePlusWarning");
}

// (r.gonet 10/30/2013) - PLID 58980 - Looks up patient education resources by using the MedlinePlus search web service.
// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which we will try to lookup patient education with
// in the case where the primary record ID fails to find any results.
void LookupMedlinePlusInformationViaSearch(CWnd *pParentWnd, MedlinePlusIDType mlpIDType, long nID, const CArray<long, long>& aryAlternateRecordIDs/* = CArray<long, long>()*/)
{
	LookupMedlinePlusInformation(pParentWnd, mlpIDType, nID, aryAlternateRecordIDs, mlpmSearch);
}

// (j.jones 2013-10-17 09:01) - PLID 57503 - given a record ID and an enum that
// identifies the record type (such as a DiagCodes.ID, DrugList.ID, or LabResultsT.ID),
// this function will have the API generate the proper Medline Plus query and end it
// to the system web browser
// (r.gonet 10/30/2013) - PLID 58980 - Extracted body to use in a common function.
// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which we will try to lookup patient education with
// in the case where the primary record ID fails to find any results.
void LookupMedlinePlusInformationViaURL(CWnd *pParentWnd, MedlinePlusIDType mlpIDType, long nID, const CArray<long, long>& aryAlternateRecordIDs/* = CArray<long, long>()*/)
{
	LookupMedlinePlusInformation(pParentWnd, mlpIDType, nID, aryAlternateRecordIDs, mlpmInfoButton);
}

// (r.gonet 10/30/2013) - PLID 58980 - Extracted method. Added a way to use either the InfoButton method or the Search method
// when getting patient education resources.
// (r.gonet 03/04/2014) - PLID 60756 - Added aryAlternateRecordIDs, which we will try to lookup patient education with
// in the case where the primary record ID fails to find any results.
void LookupMedlinePlusInformation(CWnd *pParentWnd, MedlinePlusIDType mlpIDType, long nID, const CArray<long, long>& aryAlternateRecordIDs, MedlinePlusMethod mlpmMethod)
{
	try {

		//this should never be null
		if(pParentWnd == NULL) {
			ASSERT(FALSE); //for debugging
			ThrowNxException("LookupMedlinePlusInformation called with a null pParentWnd.");
		}

		NexTech_Accessor::_PracticeMethodsPtr pApi = GetAPI();
		if(pApi == NULL) {
			ThrowNxException("Could not lookup Medline Plus information due to an invalid API.");
		}

		//give our liability warning
		CheckWarnMedlinePlusInfo(pParentWnd);

		//The enum he passed in is not the API type, for the purposes of not having
		//to include the API in every calling dialog.
		//We now have to convert it to the proper API enum.
		NexTech_Accessor::MedlinePlusIDType eAPIIDType;
		switch(mlpIDType) {
			case mlpDiagCodeID:
				eAPIIDType = NexTech_Accessor::MedlinePlusIDType_DiagCodeID;
				break;
			case mlpDrugListID:
				eAPIIDType = NexTech_Accessor::MedlinePlusIDType_DrugListID;
				break;
			case mlpLabResultID:
				eAPIIDType = NexTech_Accessor::MedlinePlusIDType_LabResultID;
				break;
			case mlpLabID:
				eAPIIDType = NexTech_Accessor::MedlinePlusIDType_LabID;
				break;
			default:
				//this should not be possible
				ASSERT(FALSE); //for debugging
				ThrowNxException("Invalid ID type encountered.");
		}
		
		// (r.gonet 03/04/2014) - PLID 60756 - Convert aryAlternateRecordIDs to a BSTR safearray.
		CArray<CString,LPCTSTR> arysAlternateRecordIDs;
		for(int i = 0; i < aryAlternateRecordIDs.GetSize(); i++) {
			arysAlternateRecordIDs.Add(AsString(aryAlternateRecordIDs.GetAt(i)));
		}
		Nx::SafeArray<BSTR> saryAlternateRecordIDs = Nx::SafeArray<BSTR>::From(arysAlternateRecordIDs);

		// (r.gonet 10/30/2013) - PLID 58980 - Get the patient education resources by either
		// the HL7 InfoButton method or by the MedlinePlus web service search.
		if(mlpmMethod == mlpmInfoButton) {
			NexTech_Accessor::_MedlinePlusWebAddressResultPtr result = pApi->GetMedlinePlusWebAddress(GetAPISubkey(), GetAPILoginToken(), eAPIIDType, _bstr_t(AsString(nID)), saryAlternateRecordIDs);
			if(result == NULL) {
				//this should not be possible, the API should at least return the base Medline Plus website
				ThrowNxException("GetMedlinePlusWebAddress did not return a result.");
			}
			CString strWebAddress = (LPCTSTR)result->webaddress;
			if(strWebAddress.IsEmpty()) {
				//this should not be possible, the API should at least return the base Medline Plus website
				ThrowNxException("GetMedlinePlusWebAddress did not return a webaddress.");
			}

			//pass the address to the system web browser
			ShellExecute(NULL, NULL, strWebAddress, NULL, NULL, SW_SHOW);

		} else if(mlpmMethod == mlpmSearch) {
			// (r.gonet 11/05/2013) - PLID 58980 - Get the URL of the search results page to open on MedlinePlus
			NexTech_Accessor::_MedlinePlusSearchResultPtr pResult = pApi->GetMedlinePlusSearchResult(GetAPISubkey(), GetAPILoginToken(), eAPIIDType, _bstr_t(AsString(nID)), saryAlternateRecordIDs);
			if(pResult == NULL) {
				ThrowNxException("GetMedlinePlusSearchResult did not return a result.");
			}
			CString strWebAddress = (LPCTSTR)pResult->FirstResultPageUrl;
			if(strWebAddress.IsEmpty()) {
				// OK. No results. Return the search page which has a nice looking No Results message on it.
				strWebAddress = (LPCTSTR)pResult->SearchResultsPageUrl;
			}
			if(strWebAddress.IsEmpty()) {
				//this should not be possible, the API should at least return the base Medline Plus website
				ThrowNxException("GetMedlinePlusSearchResult did not return a website.");
			}

			//pass the address to the system web browser
			ShellExecute(NULL, NULL, strWebAddress, NULL, NULL, SW_SHOW);
		}

		// (r.farnworth 2014-05-14 11:57) - PLID 62151 - Write data to EducationResourceAccessT whenever Education Resources are accessed.
		ExecuteParamSql("INSERT INTO EducationResourceAccessT (PatientID, AccessTime) "
			"VALUES ({INT}, GETDATE())", GetActivePatientID());

	}NxCatchAll(__FUNCTION__);	
}