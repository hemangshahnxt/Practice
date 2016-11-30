#pragma once
#include "Search_DataListProvider.h"

// (r.gonet 2016-02-23 01:12) - PLID 68404 - IDs of special rows in the pharmacy search results list.
enum EPharmacySearchSpecialRow {
	SpacerFooter = -5,
	InstructionsFooter = -4,
	AddFromPharmacyDirectory = -3,
	NoResultsFound = -1,
};

// (r.gonet 2016-02-17 16:05) - PLID 68404 - Added a search list provider for searching pharmacies
class CPharmacySearch_DatalistProvider : public LookupSearch::CSearch_Api_DataListProvider {
public:
	// (r.gonet 2016-02-17 16:31) - PLID 68404 - Create a searchable pharmacy list provider.
	// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
	// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added the favorite icon handle so the caller can pass in the icon to be used as the favorite. Caller is responsible for
	// freeing the icon resources.
	CPharmacySearch_DatalistProvider(ADODB::_ConnectionPtr pCon, HICON hFavoriteIcon, long nPatientPersonID = -1, long nPatientMedicationID = -1);
	// (r.gonet 2016-02-23 01:26) - PLID 68404
	virtual ~CPharmacySearch_DatalistProvider();
protected:
	// (r.gonet 2016-02-23 01:26) - PLID 68404 - The patient we are filtered on for showing favorite pharmacies.
	long m_nPatientPersonID = -1;
	// (r.gonet 2016-02-23 01:26) - PLID 68404 - The prescription we are filtered on for showing the selected pharmacy.
	long m_nPatientMedicationID = -1;
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Handle to a favorite icon resource. This icon is unowned and will be destroyed externally.
	HICON m_hUnownedFavoriteIcon;
protected:
	// (r.gonet 2016-02-23 01:26) - PLID 68404 - Overrides of the abstract base class.
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);

};

