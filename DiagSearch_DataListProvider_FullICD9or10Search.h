#pragma once
#include "DiagSearch_DataListProvider.h"
#include "Search_DataListProvider.h"

// (j.jones 2014-02-06 11:50) - PLID 60710 - Created

class CDiagSearch_DataListProvider_FullICD9or10Search :
	public CDiagSearch_DataListProvider
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearch_DataListProvider_FullICD9or10Search(ADODB::_ConnectionPtr pCon, bool bIncludePCS);

	//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResult);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);


};
