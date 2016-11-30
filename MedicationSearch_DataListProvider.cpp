#include "stdafx.h"
#include "MedicationSearch_DataListProvider.h"
#include "NxAPI.h"
#include "PrescriptionUtilsAPI.h"
#include "FirstDataBankUtils.h"

//(s.dhole 2/4/2015 11:24 AM ) - PLID 64559
using namespace ADODB;


#define ERX_NORMAL_COLOR RGB(255, 255, 255);

// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
CMedicationSearch_DataListProvider::CMedicationSearch_DataListProvider(ADODB::_ConnectionPtr pCon, bool bFormulary, bool bIncludeFDBMedsOnly)
{	
	m_hIcon = NULL;
	m_bFormulary = bFormulary;
	m_bIncludeFDBMedsOnly = bIncludeFDBMedsOnly;
}


CMedicationSearch_DataListProvider::~CMedicationSearch_DataListProvider()
{
	if (m_hIcon != NULL) {
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}
}



//(s.dhole 2/3/2015 9:41 AM ) - PLID 64612 Abstarct class implimnetation , Override  function
HICON CMedicationSearch_DataListProvider::GetIcon()
{
	if (m_hIcon == NULL) {
		m_hIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_NEXFORMULARY), IMAGE_ICON, 16, 16, 0);
	}
	return m_hIcon;
}

//(s.dhole 3/6/2015 2:12 PM ) - PLID  64612 Check base class .h implimentation
SAFEARRAY *CMedicationSearch_DataListProvider::Search(BSTR bstrSearchString)
{

	if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if (!FirstDataBank::IsDatabaseValid())
			{
				//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
				return NULL;
			}
		}
	// Call the API to do the medication Search.  We must covert the SAFEARRAY to a CArray before we can do anything useful with it.
	NexTech_Accessor::MedicationStatus medStatus = NexTech_Accessor::MedicationStatus_Active;
	// filter
	NexTech_Accessor::_FDBMedicationFilterPtr pFilter(__uuidof(NexTech_Accessor::FDBMedicationFilter));
	pFilter->NameLike = bstrSearchString;
	pFilter->_MedicationStatus = medStatus;
	pFilter->IncludeMonograph = FALSE;

	// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
	NexTech_Accessor::_NullableBoolPtr pIncludeFDBMedsOnly(__uuidof(NexTech_Accessor::NullableBool));
	pIncludeFDBMedsOnly->SetBool(m_bIncludeFDBMedsOnly ? VARIANT_TRUE : VARIANT_FALSE);
	pFilter->IncludeFDBMedsOnly = pIncludeFDBMedsOnly;

	NexTech_Accessor::_FDBMedicationArrayPtr pResults = GetAPI()->MedicationSearch(GetAPISubkey(), GetAPILoginToken(), pFilter);
	// Get the results array
	if (pResults != NULL) {
		return pResults->FDBMedications;
	}
	else {
		return NULL;
	}
}


//(s.dhole 3/6/2015 2:13 PM ) - PLID 64559  Check base class .h implimentation
void CMedicationSearch_DataListProvider::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	NexTech_Accessor::_FDBMedicationPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varMedicationID = g_cvarNull;
	_variant_t varMedicationName = g_cvarNull;
	_variant_t varFDBID = g_cvarNull;
	_variant_t varFDBOutOfDate = g_cvarNull;
	_variant_t varNexFormulary = g_cvarNull;
	_variant_t varBackColor = g_cvarNull;


	//convert our results to variants, with Null for no value
	varMedicationID = pResult->DrugListID;
	//varMedicationName = pResult->MedName.Detach();
	
	varMedicationName.vt = VT_BSTR;
	varMedicationName.bstrVal = pResult->MedName.Detach();

	varFDBID = pResult->MedID;
	// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
	varFDBOutOfDate = pResult->FDBOutOfDate && VarLong(varFDBID, 0) > 0 ? g_cvarTrue : g_cvarFalse;
	varBackColor = (long)ERX_NORMAL_COLOR;


	//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
	if (pResult->MedID > 0){
		if (pResult->FDBOutOfDate) {
			varBackColor = (long)ERX_IMPORTED_OUTOFDATE_COLOR;
		}
		else {
			varBackColor = ERX_IMPORTED_COLOR;
			if (m_bFormulary){
				varNexFormulary = _variant_t((long)GetIcon());
			}
		}
	}

	//add all our values into a safearray
	// Speed boost: create all elements in the array at once

	{
		// Write values to the safearray elements
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varMedicationID.Detach();
		aryFieldValues[countValues++] = varMedicationName.Detach();
		aryFieldValues[countValues++] = varFDBID.Detach();
		aryFieldValues[countValues++] = _variant_t((long)varFDBOutOfDate, VT_I4).Detach();
		aryFieldValues[countValues++] = _variant_t((long)varBackColor, VT_I4).Detach();
		if (m_bFormulary){
			aryFieldValues[countValues++] = varNexFormulary.Detach();
		}
		ASSERT(countValues == nFieldValueCount);
	}

}


//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
HRESULT CMedicationSearch_DataListProvider::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[1] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[4] = _variant_t((long)ERX_NO_RESULTS_COLOR, VT_I4).Detach();
	aryFieldValues[0] = _variant_t((long)NO_RESULTS_ROW, VT_I4).Detach(); //MedicationID
	aryFieldValues[2] = _variant_t(-1L, VT_I4).Detach(); //FDBID
	return S_OK;
}

