#include "StdAfx.h"
#include "DiagSearch_DataListProvider_Crosswalk.h"
#include "NxException.h"
#include "DiagSearchUtils.h"
#include <NxUILib\GlobalDrawingUtils.h>
#include "NxAPI.h"
#include "DiagSearchConfig_Crosswalk.h"

// (j.jones 2014-02-10 09:13) - PLID 60679 - Created

using namespace DiagSearchUtils;
using namespace ADODB;

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearch_DataListProvider_Crosswalk::CDiagSearch_DataListProvider_Crosswalk(ADODB::_ConnectionPtr pCon, bool bIncludePCS)
	: CDiagSearch_DataListProvider(bIncludePCS)
{
}

//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
SAFEARRAY *CDiagSearch_DataListProvider_Crosswalk::Search(BSTR bstrSearchString)
{
	//give our un-altered search string to the API, which will trim spaces and break it up into individual search terms
	//Practice does not want to limit matches, so we pass in -1 for maxMatches.
	// (j.jones 2014-03-03 11:56) - PLID 61136 - added PCS flag
	NexTech_Accessor::_DiagSearchResultsPtr pCrosswalkResults =
		GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::DiagSearchType_Crosswalk, m_bIncludePCS, bstrSearchString, -1);

	// Get the results array
	if (pCrosswalkResults != NULL) {
		return pCrosswalkResults->SearchResults;
	} else {
		return NULL;
	}
}

//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
void CDiagSearch_DataListProvider_Crosswalk::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	NexTech_Accessor::_DiagSearchResultPtr pResult = pResultUnk;

	//convert our results to variants, with Null for no value
	_variant_t varICD9Code = g_cvarNull;
	_variant_t varICD9Description = g_cvarNull;
	_variant_t varICD9_DiagCodesID = g_cvarNull;
	_variant_t varICD10Code = g_cvarNull;
	_variant_t varICD10Description = g_cvarNull;
	_variant_t varICD10_DiagCodesID = g_cvarNull;
	_variant_t varMatchType = g_cvarNull;
	// (z.manning 2014-03-03 14:22) - PLID 61140 - Set the quick list icon if this result is in the 
	// current user's quick list
	_variant_t varIsQuicklist = pResult->InCurrentUserQuickList ? _variant_t((long)GetQuicklistIcon()) : g_cvarNull;
	// (j.jones 2014-03-04 09:19) - PLID 61136 - added PCS flag
	_variant_t varPCS = pResult->PCS ? g_cvarTrue : g_cvarFalse;

	NexTech_Accessor::_SearchDiagCodePtr pICD9Code = pResult->ICD9Code;
	if (pICD9Code != NULL) {

		// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Speed boost: avoid copying the strings

		V_VT(&varICD9Code) = VT_BSTR;
		V_BSTR(&varICD9Code) = pICD9Code->Code.Detach();

		V_VT(&varICD9Description) = VT_BSTR;
		V_BSTR(&varICD9Description) = pICD9Code->description.Detach();

		_bstr_t id = pICD9Code->DiagCodesID;
		if (id.length() > 0) {
			long nDiagCodeID = atoi(id);
			if (nDiagCodeID > 0) {
				varICD9_DiagCodesID = nDiagCodeID;
			}
		}
	} else {
		varICD9Description = "<No Matching Code>";
	}

	NexTech_Accessor::_SearchDiagCodePtr pICD10Code = pResult->ICD10Code;
	if (pICD10Code) {

		// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Speed boost: avoid copying the strings

		V_VT(&varICD10Code) = VT_BSTR;
		V_BSTR(&varICD10Code) = pICD10Code->Code.Detach();

		V_VT(&varICD10Description) = VT_BSTR;
		V_BSTR(&varICD10Description) = pICD10Code->description.Detach();

		_bstr_t id = pICD10Code->DiagCodesID;
		if (id.length() > 0) {
			long nDiagCodeID = atoi(id);
			if (nDiagCodeID > 0) {
				varICD10_DiagCodesID = nDiagCodeID;
			}
		}
	} else {
		varICD10Description = "<No Matching Code>";
	}

	//convert MatchType to our Practice-side enum
	DiagSearchUtils::EDiagCrosswalkMatchType eMatchType = DiagSearchUtils::NoMatch;
	switch (pResult->MatchType) {
	case NexTech_Accessor::DiagCrosswalkMatchType_ExactMatch:
		eMatchType = DiagSearchUtils::ExactMatch;
		break;
	case NexTech_Accessor::DiagCrosswalkMatchType_ApproximateMatch:
		eMatchType = DiagSearchUtils::ApproximateMatch;
		break;
	case NexTech_Accessor::DiagCrosswalkMatchType_Combination:
		eMatchType = DiagSearchUtils::Combination;
		break;
	case NexTech_Accessor::DiagCrosswalkMatchType_NoMatch:
		eMatchType = DiagSearchUtils::NoMatch;
		break;
	// (c.haag 2015-07-14) - PLID 66018
	case NexTech_Accessor::DiagCrosswalkMatchType_CustomMatch:
		eMatchType = DiagSearchUtils::CustomMatch;
		break;
	case NexTech_Accessor::DiagCrosswalkMatchType_NotApplicable:
		//Not Applicable should never have been returned in
		//a crosswalk search. Change it to No Match.
		ASSERT(FALSE);
		eMatchType = DiagSearchUtils::NoMatch;
		break;
	default:
		//should not be possible					
		eMatchType = DiagSearchUtils::NoMatch;
		break;
	}

	// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Speed boost: calc the color efficiently from the bstr so we 
	// don't need to mess with keeping a map for speed.
	long nBackColor = ConvertHexStringToCOLORREF(pResult->Color);

	// Finally return our results
	{
		int countValues = 0;

		// (b.cardillo 2014-03-27 14:13) - PLID 61564 - Speed boost: avoid copying the variants
		aryFieldValues[countValues++] = varICD9Code.Detach();
		aryFieldValues[countValues++] = varICD9Description.Detach();
		aryFieldValues[countValues++] = varICD9_DiagCodesID.Detach();
		aryFieldValues[countValues++] = varICD10Code.Detach();
		aryFieldValues[countValues++] = varICD10Description.Detach();
		aryFieldValues[countValues++] = varICD10_DiagCodesID.Detach();
		aryFieldValues[countValues++] = _variant_t((long)eMatchType, VT_I4).Detach();
		aryFieldValues[countValues++] = _variant_t(nBackColor, VT_I4).Detach();
		aryFieldValues[countValues++] = varIsQuicklist.Detach();
		aryFieldValues[countValues++] = varPCS.Detach();

		ASSERT(countValues == nFieldValueCount);
	}
}

//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
HRESULT CDiagSearch_DataListProvider_Crosswalk::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[1] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[7] = _variant_t((long)RGB(255, 255, 255), VT_I4).Detach();
	aryFieldValues[2] = _variant_t(-1L, VT_I4).Detach(); //DiagCodesID
	aryFieldValues[5] = _variant_t(-1L, VT_I4).Detach(); //DiagCodesID
	return S_OK;
}

