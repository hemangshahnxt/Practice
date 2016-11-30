#pragma once
#include "Search_DataListProvider.h"

//(s.dhole 2/4/2015 11:24 AM ) - PLID 64559
class CMedicationSearch_DataListProvider : public LookupSearch::CSearch_Api_DataListProvider
{
public:
	// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
	CMedicationSearch_DataListProvider(ADODB::_ConnectionPtr pCon, bool bFormulary, bool bIncludeFDBMedsOnly);
	~CMedicationSearch_DataListProvider();
	HICON m_hIcon;
protected:
	HICON GetIcon();
protected:
	//I went with ADODB connection ptr instead of CNxAdoConnection since this is likely to be in a lib and a little more generalized
	//	code available to all projects.
	
	bool m_bFormulary;

	// (j.jones 2016-01-21 08:52) - PLID 68020 - added option to filter out non-FDB drugs
	bool m_bIncludeFDBMedsOnly;

	//(s.dhole 3/6/2015 10:32 AM ) - PLID 64610 Check base class .h implimentation
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);

};

