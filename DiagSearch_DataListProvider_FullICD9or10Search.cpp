// (j.jones 2014-02-06 11:50) - PLID 60710 - Created
#include "StdAfx.h"
#include "DiagSearch_DataListProvider_FullICD9or10Search.h"
#include "NxException.h"
#include "DiagSearchUtils.h"
#include <NxUILib\GlobalDrawingUtils.h>
#include "NxAPI.h"

using namespace DiagSearchUtils;
using namespace ADODB;

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearch_DataListProvider_FullICD9or10Search::CDiagSearch_DataListProvider_FullICD9or10Search(ADODB::_ConnectionPtr pCon, bool bIncludePCS)
	: CDiagSearch_DataListProvider(bIncludePCS)
{

}

//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
SAFEARRAY *CDiagSearch_DataListProvider_FullICD9or10Search::Search(BSTR bstrSearchString)
{
	//give our un-altered search string to the API, which will trim spaces and break it up into individual search terms
	//Practice does not want to limit matches, so we pass in -1 for maxMatches.
	NexTech_Accessor::_DiagSearchResultsPtr pICD910Results;

	// (j.jones 2014-02-28 16:15) - PLID 61113 - We now will look at the cached global preference,
	// if it is crosswalk then this is really the full 9 and 10 list. If their preference is for
	// ICD-10s only, just return ICD10 codes. But when in ICD-9 mode, return managed 9s.
	// So when not in ICD-9 mode, lists are always the full lists no matter what, and never the managed lists.
	// When in ICD-9 mode, it's only the managed list, and never a full list.
	// (j.jones 2014-03-03 11:56) - PLID 61136 - added PCS flag
	DiagCodeSearchStyle eSearchStyle = GetPreferenceSearchStyle();
	if (eSearchStyle == eManagedICD9_Search) {
		//search managed ICD-9 codes only
		//the PCS code parameter has no effect in managed searches, so just pass false here
		pICD910Results = GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::DiagSearchType_ManagedICD9Only, false, bstrSearchString, -1);
	}
	else if (eSearchStyle == eManagedICD10_Search) {
		//search ICD-10 codes only
		pICD910Results = GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::DiagSearchType_FullICD10Only, m_bIncludePCS, bstrSearchString, -1);
	}
	else {
		//this really is the full 9 and 10 search		
		pICD910Results = GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::DiagSearchType_FullICD9or10, m_bIncludePCS, bstrSearchString, -1);
	}

	// Get the results array
	if (pICD910Results != NULL) {
		return pICD910Results->SearchResults;
	}
	else {
		return NULL;
	}
}

//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
void CDiagSearch_DataListProvider_FullICD9or10Search::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	
	NexTech_Accessor::_DiagSearchResultPtr pResult = pResultUnk;
	//convert our results to variants, with Null for no value
	_variant_t varIsICD10 = g_cvarNull;
	_variant_t varCode = g_cvarNull;
	_variant_t varDescription = g_cvarNull;
	_variant_t varDiagCodesID = g_cvarNull;
	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS flag
	_variant_t varPCS = pResult->PCS ? g_cvarTrue : g_cvarFalse;

	//the result is either an ICD-9 or ICD-10, mutually exclusive
	NexTech_Accessor::_SearchDiagCodePtr pICD9Code = pResult->ICD9Code;
	NexTech_Accessor::_SearchDiagCodePtr pICD10Code = pResult->ICD10Code;
	if(pICD9Code != NULL && pICD10Code != NULL) {
		//should be impossible, the API should not have returned
		//both codes in one record
		ASSERT(FALSE);
	}

	if(pICD9Code != NULL) {

		varIsICD10 = g_cvarFalse;

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
	else if(pICD10Code) {

		varIsICD10 = g_cvarTrue;

		// (b.cardillo 2014-04-02 17:04) - PLID 61564 - Speed boost: avoid copying the strings

		V_VT(&varCode) = VT_BSTR;
		V_BSTR(&varCode) = pICD10Code->Code.Detach();

		V_VT(&varDescription) = VT_BSTR;
		V_BSTR(&varDescription) = pICD10Code->description.Detach();

		_bstr_t id = pICD10Code->DiagCodesID;
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
			
	{
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		// (b.cardillo 2014-04-02 17:04) - PLID 61564 - Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varIsICD10.Detach();
		aryFieldValues[countValues++] = varCode.Detach();
		aryFieldValues[countValues++] = varDescription.Detach();
		aryFieldValues[countValues++] = varDiagCodesID.Detach();
		aryFieldValues[countValues++] = _variant_t(nBackColor, VT_I4).Detach();
		aryFieldValues[countValues++] = varPCS.Detach();
		ASSERT(countValues == nFieldValueCount);
		// Hand the elements into the safe array
	}
}


//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
HRESULT CDiagSearch_DataListProvider_FullICD9or10Search::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[2] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[3] = _variant_t(-1L, VT_I4).Detach(); //DiagCodesID
	aryFieldValues[4] = _variant_t((long)RGB(255, 255, 255), VT_I4).Detach();
	
	return S_OK;
}
