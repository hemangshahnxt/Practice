#pragma once
#include "DiagSearch_DataListProvider.h"
#include "Search_DataListProvider.h"

// (j.jones 2015-09-01 14:37) - PLID 66993 - Created

class CDiagSearch_DataListProvider_FullICD9Search :
	public CDiagSearch_DataListProvider
{
public:
	CDiagSearch_DataListProvider_FullICD9Search(ADODB::_ConnectionPtr pCon, bool bIncludePCS);

	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResult);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);
};
