#pragma once
#include "Search_DataListProvider.h"

// (j.jones 2015-03-25 14:10) - PLID 65390 - created

class CGiftCertificateSearch_DataListProvider_General : public LookupSearch::CSearch_Api_DataListProvider
{
public:
	CGiftCertificateSearch_DataListProvider_General(ADODB::_ConnectionPtr pCon, bool bHideZeroBalances, bool bHideExpired, bool bHideVoided);
	~CGiftCertificateSearch_DataListProvider_General();
protected:

protected:
	//I went with ADODB connection ptr instead of CNxAdoConnection since this is likely to be in a lib and a little more generalized
	//	code available to all projects.
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);

	bool m_bHideZeroBalances;
	bool m_bHideExpired;
	bool m_bHideVoided;
};
