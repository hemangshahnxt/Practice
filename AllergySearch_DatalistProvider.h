#pragma once
#include "Search_DataListProvider.h"
//(s.dhole 2/4/2015 11:24 AM ) - PLID 64563 added new class
class CAllergySearch_DatalistProvider : public LookupSearch::CSearch_Api_DataListProvider
{
public:
	// (j.jones 2016-01-21 09:18) - PLID 68021 - added an option to include only FDB allergies
	CAllergySearch_DatalistProvider(ADODB::_ConnectionPtr pCon, bool bIncludeFDBAllergiesOnly);
	~CAllergySearch_DatalistProvider();
protected:
	
protected:

	// (j.jones 2016-01-21 09:18) - PLID 68021 - added an option to include only FDB allergies
	bool m_bIncludeFDBAllergiesOnly;

	//I went with ADODB connection ptr instead of CNxAdoConnection since this is likely to be in a lib and a little more generalized
	//	code available to all projects.
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);

};

