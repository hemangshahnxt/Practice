#include "stdafx.h"
#include "AllergySearch_DatalistProvider.h"
#include "NxAPI.h"
#include "PrescriptionUtilsAPI.h"
#include "FirstDataBankUtils.h"

//(s.dhole 2/4/2015 11:24 AM ) - PLID 64563 added new class
using namespace ADODB;


#define ERX_NORMAL_COLOR RGB(255, 255, 255);

// (j.jones 2016-01-21 09:18) - PLID 68021 - added an option to include only FDB allergies
CAllergySearch_DatalistProvider::CAllergySearch_DatalistProvider(ADODB::_ConnectionPtr pCon, bool bIncludeFDBAllergiesOnly)
{
	m_bIncludeFDBAllergiesOnly = bIncludeFDBAllergiesOnly;
}


CAllergySearch_DatalistProvider::~CAllergySearch_DatalistProvider()
{
}


//(s.dhole 3/6/2015 2:12 PM ) - PLID  64563 Check base class .h implimentation
SAFEARRAY *CAllergySearch_DatalistProvider::Search(BSTR bstrSearchString)
{

	// (j.fouts 2013-03-19 3:10) - PLID 64563 - Ensure database before we do trying to use FDB
	if (g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent))
	{
		//Also check that the database exists
		if (!FirstDataBank::IsDatabaseValid())
		{
			//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
			return NULL;
		}
	}

	// (j.jones 2016-01-21 09:18) - PLID 68021 - added an option to include only FDB allergies
	NexTech_Accessor::_NullableBoolPtr pIncludeFDBAllergiesOnly(__uuidof(NexTech_Accessor::NullableBool));
	pIncludeFDBAllergiesOnly->SetBool(m_bIncludeFDBAllergiesOnly ? VARIANT_TRUE : VARIANT_FALSE);

	NexTech_Accessor::_FDBAllergyArrayPtr pResults = GetAPI()->AllergySearch(GetAPISubkey(), GetAPILoginToken(), bstrSearchString, pIncludeFDBAllergiesOnly);

	// Get the results array
	if (pResults != NULL) {
		return pResults->FDBAllergies;
	}
	else {
		return NULL;
	}
}



//(s.dhole 3/6/2015 2:13 PM ) - PLID 64563  Check base class .h implimentation
void CAllergySearch_DatalistProvider::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	NexTech_Accessor::_FDBAllergyPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varAllergyID = g_cvarNull;
	_variant_t varAllergyName = g_cvarNull;
	_variant_t varStatus = g_cvarNull;
	_variant_t varConceptID = g_cvarNull;
	_variant_t varConceptTypeID = g_cvarNull;
	_variant_t varFDBOutOfDate = g_cvarNull;
	_variant_t varBackColor = g_cvarNull;


	//convert our results to variants, with Null for no value
	varAllergyID = pResult->AllergyID;
	
	varAllergyName.vt = VT_BSTR;
	varAllergyName.bstrVal = pResult->Name.Detach();
	varStatus = pResult->ConceptID != -1 && pResult->ConceptTypeID != -1 ? 1 : 0;
	varConceptID = pResult->ConceptID;
	varConceptTypeID = pResult->ConceptTypeID;
	varFDBOutOfDate = pResult->FDBOutOfDate ? g_cvarTrue : g_cvarFalse;;
	varBackColor = (long)ERX_NORMAL_COLOR;
	

	//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
	if (pResult->ConceptID != -1 && pResult->ConceptTypeID != -1){
		if (pResult->FDBOutOfDate) {
			varBackColor = (long)ERX_IMPORTED_OUTOFDATE_COLOR;
		}
		else {
			varBackColor = ERX_IMPORTED_COLOR;
		}
	}

	//add all our values into a safearray
	// Speed boost: create all elements in the array at once

	{
		// Write values to the safearray elements
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		//Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varAllergyID.Detach();
		aryFieldValues[countValues++] = varAllergyName.Detach();
		aryFieldValues[countValues++] = _variant_t((long)varStatus, VT_I4).Detach(); 		
		aryFieldValues[countValues++] = varConceptID.Detach();
		aryFieldValues[countValues++] = varConceptTypeID.Detach();
		aryFieldValues[countValues++] = _variant_t((long)varFDBOutOfDate, VT_I4).Detach();
		aryFieldValues[countValues++] = _variant_t((long)varBackColor, VT_I4).Detach();
		ASSERT(countValues == nFieldValueCount);
	}
}


//(s.dhole 3/6/2015 10:32 AM ) - PLID 64563 Check base class .h implimentation
HRESULT CAllergySearch_DatalistProvider::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[1] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[4] = _variant_t((long)ERX_NO_RESULTS_COLOR, VT_I4).Detach();
	aryFieldValues[0] = _variant_t((long)NO_RESULTS_ROW, VT_I4).Detach(); //AllergyID
	return S_OK;
}

