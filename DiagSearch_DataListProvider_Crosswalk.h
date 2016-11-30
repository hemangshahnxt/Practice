#pragma once
#include "DiagSearch_DataListProvider.h"
#include "Search_DataListProvider.h"

// (j.jones 2014-02-10 09:13) - PLID 60679 - This is the icd9 to icd10 crosswalk specific implementation for the
//	search list provider.

class CDiagSearch_DataListProvider_Crosswalk :
	public CDiagSearch_DataListProvider
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearch_DataListProvider_Crosswalk(ADODB::_ConnectionPtr pCon, bool bIncludePCS);
	//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);
};
