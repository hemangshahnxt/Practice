// (r.gonet 2016-02-17 16:05) - PLID 68404 - Added a search list provider for searching pharmacies
#include "stdafx.h"
#include "PharmacySearch_DatalistProvider.h"
#include "NxAPI.h"
#include "PrescriptionUtilsAPI.h"
#include "FirstDataBankUtils.h"
#include "NxUILib\ColorUtils.h"
using namespace ADODB;

// (r.gonet 2016-02-17 16:31) - PLID 68404 - Creates a searchable pharmacy list provider.
// If nPatientMedication is not -1, the results list will include, at the top, the pharmacy selected for that prescription,
// if the pharmacy matches the search. If nPatientID is not -1, then the patient's favorite pharmacies will appear below the selected pharmacy.
// (r.gonet 2016-02-23 12:47) - PLID 67964 - Added hFavoriteIcon. Caller is responsible for freeing resources.
CPharmacySearch_DatalistProvider::CPharmacySearch_DatalistProvider(ADODB::_ConnectionPtr pCon, HICON hFavoriteIcon, long nPatientPersonID/* = -1*/, long nPatientMedicationID/*= -1*/)
	: LookupSearch::CSearch_Api_DataListProvider()
{
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Init the favorite icon.
	m_hUnownedFavoriteIcon = hFavoriteIcon;
	m_nPatientPersonID = nPatientPersonID;
	m_nPatientMedicationID = nPatientMedicationID;
}

// (r.gonet 2016-02-23 01:26) - PLID 68404
CPharmacySearch_DatalistProvider::~CPharmacySearch_DatalistProvider()
{
}

// (r.gonet 2016-02-23 01:26) - PLID 68404 - Conducts the search for data.
SAFEARRAY *CPharmacySearch_DatalistProvider::Search(BSTR bstrSearchString)
{
	CWaitCursor pWait;
	
	NexTech_Accessor::_PharmacySearchResultsListPtr pResults = GetAPI()->SearchPharmacies(GetAPISubkey(), GetAPILoginToken(), 
		bstrSearchString, 
		_bstr_t(m_nPatientPersonID != -1 ? FormatString("%li", m_nPatientPersonID) : ""), 
		_bstr_t(m_nPatientMedicationID != -1 ? FormatString("%li", m_nPatientMedicationID) : ""));

	// Get the results array
	if (pResults != NULL) {
		return pResults->Results;
	} else {
		return NULL;
	}
}

// (r.gonet 2016-02-23 01:26) - PLID 68404 - Fills a variant array with the search result values returned.
void CPharmacySearch_DatalistProvider::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	NexTech_Accessor::_PharmacySearchResultPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varID = _variant_t(atol(pResult->ID), VT_I4);
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Set the leading icon to the favorite icon if the result is a favorite pharmacy. Otherwise no icon.
	_variant_t varLeadingIcon = pResult->IsFavorite != VARIANT_FALSE ? (long)m_hUnownedFavoriteIcon : (long)0;
	_variant_t varName = _variant_t(pResult->Name);
	CString strAddress((LPCTSTR)pResult->Address1);
	strAddress += " ";
	strAddress += (LPCTSTR)pResult->Address2;
	_variant_t varAddress = _bstr_t(strAddress);
	_variant_t varCity = _variant_t(pResult->City);
	_variant_t varState = _variant_t(pResult->State);
	_variant_t varZip = _variant_t(pResult->Zip);
	// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
	_variant_t varCrossStreet = _variant_t(pResult->CrossStreet);
	_variant_t varOrderIndex = !pResult->FavoritePharmacyOrderIndex->IsNull() ? pResult->FavoritePharmacyOrderIndex->GetValue() : g_cvarNull;
	_variant_t varPhone = _variant_t(pResult->Phone);
	_variant_t varNCPDPID = _variant_t(pResult->NCPDPID);
	_variant_t varFax = _variant_t(pResult->Fax);
	bool bEPrescribingReady = pResult->NCPDPID.length() != 0;
	_variant_t varEPrescribingReady = bEPrescribingReady ? g_cvarTrue : g_cvarFalse;
	_variant_t varSpec1 = _variant_t(pResult->SpecialtyType1);
	_variant_t varSpec2 = _variant_t(pResult->SpecialtyType2);
	_variant_t varSpec3 = _variant_t(pResult->SpecialtyType3);
	_variant_t varSpec4 = _variant_t(pResult->SpecialtyType4);
	// (r.gonet 2016-02-23 11:02) - PLID 67962 - Added color columns. Light gray as the row background for non-erx pharmacies. 
	// Product thought 192,192,192 was too dark. That's light gray in MS Paint. Going lighter.
	DWORD dwRowBackColor = bEPrescribingReady ? GetSysColor(COLOR_WINDOW) : RGB(220, 220, 220);
	DWORD dwRowBackSelColor = bEPrescribingReady ? GetSysColor(COLOR_HIGHLIGHT) : ColorUtils::CalcDarkerColor(dwRowBackColor);
	DWORD dwRowForeColor = bEPrescribingReady ? GetSysColor(COLOR_WINDOWTEXT) : RGB(0, 0, 0);
	DWORD dwRowForeSelColor = bEPrescribingReady ? GetSysColor(COLOR_HIGHLIGHTTEXT) : ColorUtils::CalcDarkerColor(dwRowForeColor);
	
	_variant_t varRowBackColor = (long)dwRowBackColor;
	_variant_t varRowBackSelColor = (long)dwRowBackSelColor;
	_variant_t varRowForeColor = (long)dwRowForeColor;
	_variant_t varRowForeSelColor = (long)dwRowForeSelColor;

	// (r.gonet 2016-02-23 01:26) - PLID 68404 - Combine the specialty types into a CSV.
	int nSpecAllLength = pResult->SpecialtyType1.length() + 2 + pResult->SpecialtyType2.length() + 2 + pResult->SpecialtyType3.length() + 2 + pResult->SpecialtyType4.length();
	CString strSpecAll;
	strSpecAll.Preallocate(nSpecAllLength);

	strSpecAll += (LPCTSTR)pResult->SpecialtyType1;

	if (pResult->SpecialtyType2.length() > 0) {
		strSpecAll += ", ";
		strSpecAll += (LPCTSTR)pResult->SpecialtyType2;
	}
	if (pResult->SpecialtyType3.length() > 0) {
		strSpecAll += ", ";
		strSpecAll += (LPCTSTR)pResult->SpecialtyType3;
	}
	if (pResult->SpecialtyType4.length() > 0) {
		strSpecAll += ", ";
		strSpecAll += (LPCTSTR)pResult->SpecialtyType4;
	}
	_variant_t varSpecAll = _variant_t(_bstr_t(strSpecAll));

	//add all our values into a safearray
	{
		// Write values to the safearray elements
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		//Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varID.Detach();
		// (r.gonet 2016-02-23 12:47) - PLID 67964 - Set the leading icon.
		aryFieldValues[countValues++] = varLeadingIcon.Detach();
		aryFieldValues[countValues++] = varName.Detach();
		aryFieldValues[countValues++] = varAddress.Detach();
		aryFieldValues[countValues++] = varCity.Detach();
		aryFieldValues[countValues++] = varState.Detach();
		aryFieldValues[countValues++] = varZip.Detach();
		// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
		aryFieldValues[countValues++] = varCrossStreet.Detach();
		aryFieldValues[countValues++] = varOrderIndex.Detach();
		aryFieldValues[countValues++] = varPhone.Detach();
		aryFieldValues[countValues++] = varNCPDPID.Detach();
		aryFieldValues[countValues++] = varFax.Detach();
		aryFieldValues[countValues++] = varEPrescribingReady.Detach();
		aryFieldValues[countValues++] = varSpec1.Detach();
		aryFieldValues[countValues++] = varSpec2.Detach();
		aryFieldValues[countValues++] = varSpec3.Detach();
		aryFieldValues[countValues++] = varSpec4.Detach();
		aryFieldValues[countValues++] = varSpecAll.Detach();
		aryFieldValues[countValues++] = varRowBackColor.Detach();
		aryFieldValues[countValues++] = varRowBackSelColor.Detach();
		aryFieldValues[countValues++] = varRowForeColor.Detach();
		aryFieldValues[countValues++] = varRowForeSelColor.Detach();
		ASSERT(countValues == nFieldValueCount);
	}
}

// (r.gonet 2016-02-23 01:26) - PLID 68404 - Get the variants for the No Results row.
HRESULT CPharmacySearch_DatalistProvider::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	//convert our results to variants, with Null for no value
	_variant_t varID = g_cvarNull;
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Init the leading icon.
	_variant_t varLeadingIcon = g_cvarNull;
	_variant_t varName = g_cvarNull;
	_variant_t varAddress = g_cvarNull;
	_variant_t varCity = g_cvarNull;
	_variant_t varState = g_cvarNull;
	_variant_t varZip = g_cvarNull;
	// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
	_variant_t varCrossStreet = g_cvarNull;
	_variant_t varOrderIndex = g_cvarNull;
	_variant_t varPhone = g_cvarNull;
	_variant_t varNCPDPID = g_cvarNull;
	_variant_t varFax = g_cvarNull;
	_variant_t varEPrescribingReady = g_cvarNull;
	_variant_t varSpec1 = g_cvarNull;
	_variant_t varSpec2 = g_cvarNull;
	_variant_t varSpec3 = g_cvarNull;
	_variant_t varSpec4 = g_cvarNull;
	_variant_t varSpecAll = g_cvarNull;
	// (r.gonet 2016-02-23 11:02) - PLID 67962 - Added color columns.
	_variant_t varRowBackColor = g_cvarNull;
	_variant_t varRowBackSelColor = g_cvarNull;
	_variant_t varRowForeColor = g_cvarNull;
	_variant_t varRowForeSelColor = g_cvarNull;

	// (r.gonet 2016-02-23 01:26) - PLID 68404 - Only display if the user actually searched for something.
	varID = (long)EPharmacySearchSpecialRow::NoResultsFound;
	// (r.gonet 2016-02-23 12:47) - PLID 67964 - Set the leading icon to nothing.
	varLeadingIcon = (long)0;
	varName = AsVariant("< No Results Found >");
	varAddress = AsVariant("");
	varCity = AsVariant("");
	varState = AsVariant("");
	varZip = AsVariant("");
	varCrossStreet = AsVariant("");
	varOrderIndex = (long)1;
	varPhone = AsVariant("");
	varNCPDPID = AsVariant("");

	//add all our values into a safearray
	{
		// Write values to the safearray elements
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		//Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varID.Detach();
		// (r.gonet 2016-02-23 12:47) - PLID 67964 - Set the leading icon.
		aryFieldValues[countValues++] = varLeadingIcon.Detach();
		aryFieldValues[countValues++] = varName.Detach();
		aryFieldValues[countValues++] = varAddress.Detach();
		aryFieldValues[countValues++] = varCity.Detach();
		aryFieldValues[countValues++] = varState.Detach();
		aryFieldValues[countValues++] = varZip.Detach();
		// (r.gonet 2016-02-24 11:00) - PLID 68418 - Added cross street
		aryFieldValues[countValues++] = varCrossStreet.Detach();
		aryFieldValues[countValues++] = varOrderIndex.Detach();
		aryFieldValues[countValues++] = varPhone.Detach();
		aryFieldValues[countValues++] = varNCPDPID.Detach();
		aryFieldValues[countValues++] = varFax.Detach();
		aryFieldValues[countValues++] = varEPrescribingReady.Detach();
		aryFieldValues[countValues++] = varSpec1.Detach();
		aryFieldValues[countValues++] = varSpec2.Detach();
		aryFieldValues[countValues++] = varSpec3.Detach();
		aryFieldValues[countValues++] = varSpec4.Detach();
		aryFieldValues[countValues++] = varSpecAll.Detach();
		// (r.gonet 2016-02-23 11:02) - PLID 67962 - Added color columns.
		aryFieldValues[countValues++] = varRowBackColor.Detach();
		aryFieldValues[countValues++] = varRowBackSelColor.Detach();
		aryFieldValues[countValues++] = varRowForeColor.Detach();
		aryFieldValues[countValues++] = varRowForeSelColor.Detach();
		ASSERT(countValues == nFieldValueCount);
	}
	return S_OK;
}