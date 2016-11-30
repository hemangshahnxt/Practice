// (j.jones 2014-02-10 09:12) - PLID 60676 - Created
#include "StdAfx.h"
#include "DiagSearch_DataListProvider_ManagedICD9Search.h"
#include "NxException.h"
#include "DiagSearchUtils.h"
#include <NxUILib\GlobalDrawingUtils.h>
#include "NxAPI.h"

using namespace DiagSearchUtils;
using namespace ADODB;

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference,
// though this search does not utilize it
CDiagSearch_DataListProvider_ManagedICD9Search::CDiagSearch_DataListProvider_ManagedICD9Search(ADODB::_ConnectionPtr pCon, bool bIncludePCS)
	: CDiagSearch_DataListProvider(bIncludePCS)
{
}


//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
SAFEARRAY *CDiagSearch_DataListProvider_ManagedICD9Search::Search(BSTR bstrSearchString)
{
	//give our un-altered search string to the API, which will trim spaces and break it up into individual search terms
	//Practice does not want to limit matches, so we pass in -1 for maxMatches.
	// (j.jones 2014-03-03 11:56) - PLID 61136 - added PCS flag, always false for managed lists
	NexTech_Accessor::_DiagSearchResultsPtr pICD9Results =
		GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::DiagSearchType_ManagedICD9Only, false, bstrSearchString, -1);

	// Get the results array
	if (pICD9Results != NULL) {
		return pICD9Results->SearchResults;
	}
	else {
		return NULL;
	}
}



//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
void CDiagSearch_DataListProvider_ManagedICD9Search::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{

	NexTech_Accessor::_DiagSearchResultPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varCode = g_cvarNull;
	_variant_t varDescription = g_cvarNull;
	_variant_t varDiagCodesID = g_cvarNull;
	// (z.manning 2014-03-03 14:22) - PLID 61140 - Set the quick list icon if this result is in the 
	// current user's quick list
	_variant_t varIsQuicklist = pResult->InCurrentUserQuickList ? _variant_t((long)GetQuicklistIcon()) : g_cvarNull;

	NexTech_Accessor::_SearchDiagCodePtr pICD9Code = pResult->ICD9Code;
	if(pICD9Code == NULL) {
		//should be impossible, we only asked the API for ICD-9 codes
		ASSERT(FALSE);
	}

	if(pICD9Code) {

		// (b.cardillo 2014-04-02 17:04) - PLID 61564 - Speed boost: avoid copying the strings

		V_VT(&varCode) = VT_BSTR;
		V_BSTR(&varCode) = pICD9Code->Code.Detach();

		V_VT(&varDescription) = VT_BSTR;
		V_BSTR(&varDescription) = pICD9Code->description.Detach();

		_bstr_t id = pICD9Code->DiagCodesID;
		if(id.length() > 0) {
			long nDiagCodeID = atoi(id);
			if(nDiagCodeID > 0) {
				varDiagCodesID = nDiagCodeID;
			}
		}
	}

	// (b.cardillo 2014-04-02 17:04) - PLID 61564 - Speed boost: calc the color efficiently from the bstr so we 
	// don't need to mess with keeping a map for speed.
	long nBackColor = ConvertHexStringToCOLORREF(pResult->Color);

	//add all our values into a safearray
	// (b.cardillo 2014-04-02 17:04) - PLID 61564 - Speed boost: create all elements in the array at once
	 {
		// Write values to the safearray elements
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		// (b.cardillo 2014-04-02 17:04) - PLID 61564 - Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varCode.Detach();
		aryFieldValues[countValues++] = varDescription.Detach();
		aryFieldValues[countValues++] = varDiagCodesID.Detach();
		aryFieldValues[countValues++] = _variant_t(nBackColor, VT_I4).Detach();
		aryFieldValues[countValues++] = varIsQuicklist.Detach();
		ASSERT(nFieldValueCount == countValues);
		// Hand the elements into the safe array
	}
}


//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
HRESULT CDiagSearch_DataListProvider_ManagedICD9Search::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[1] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[3] = _variant_t((long)RGB(255, 255, 255), VT_I4).Detach();
	aryFieldValues[2] = _variant_t(-1L, VT_I4).Detach(); //DiagCodesID
	return S_OK;
}