// (j.jones 2015-09-01 14:37) - PLID 66993 - Created
#include "StdAfx.h"
#include "DiagSearch_DataListProvider_FullICD9Search.h"
#include "NxException.h"
#include "DiagSearchUtils.h"
#include <NxUILib\GlobalDrawingUtils.h>
#include "NxAPI.h"

using namespace DiagSearchUtils;
using namespace ADODB;

CDiagSearch_DataListProvider_FullICD9Search::CDiagSearch_DataListProvider_FullICD9Search(ADODB::_ConnectionPtr pCon, bool bIncludePCS)
	: CDiagSearch_DataListProvider(bIncludePCS)
{

}

SAFEARRAY *CDiagSearch_DataListProvider_FullICD9Search::Search(BSTR bstrSearchString)
{
	//give our un-altered search string to the API, which will trim spaces and break it up into individual search terms
	//Practice does not want to limit matches, so we pass in -1 for maxMatches.
	NexTech_Accessor::_DiagSearchResultsPtr pICD9Results = GetAPI()->GetDiagSearchResultsForSearchType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::DiagSearchType_FullICD9Only, m_bIncludePCS, bstrSearchString, -1);

	// Get the results array
	if (pICD9Results != NULL) {
		return pICD9Results->SearchResults;
	}
	else {
		return NULL;
	}
}

void CDiagSearch_DataListProvider_FullICD9Search::GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk)
{
	
	NexTech_Accessor::_DiagSearchResultPtr pResult = pResultUnk;
	//convert our results to variants, with Null for no value
	_variant_t varCode = g_cvarNull;
	_variant_t varDescription = g_cvarNull;
	_variant_t varDiagCodesID = g_cvarNull;
	_variant_t varPCS = pResult->PCS ? g_cvarTrue : g_cvarFalse;

	//always an ICD-9 code
	NexTech_Accessor::_SearchDiagCodePtr pICD9Code = pResult->ICD9Code;
	if(pICD9Code == NULL) {
		//should be impossible, the API should have always returned a code
		ASSERT(FALSE);
	}

	if(pICD9Code != NULL) {
		
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

	long nBackColor = ConvertHexStringToCOLORREF(pResult->Color);
			
	{
		int countValues = 0;
		// THE ORDER HERE IS CRITICAL: It must exactly match the recordset's field order.
		aryFieldValues[countValues++] = varCode.Detach();
		aryFieldValues[countValues++] = varDescription.Detach();
		aryFieldValues[countValues++] = varDiagCodesID.Detach();
		aryFieldValues[countValues++] = _variant_t(nBackColor, VT_I4).Detach();
		aryFieldValues[countValues++] = varPCS.Detach();
		ASSERT(countValues == nFieldValueCount);
		// Hand the elements into the safe array
	}
}

HRESULT CDiagSearch_DataListProvider_FullICD9Search::GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount)
{
	aryFieldValues[1] = AsVariant("< No Results Found >").Detach();
	aryFieldValues[2] = _variant_t(-1L, VT_I4).Detach(); //DiagCodesID
	aryFieldValues[3] = _variant_t((long)RGB(255, 255, 255), VT_I4).Detach();
	
	return S_OK;
}
