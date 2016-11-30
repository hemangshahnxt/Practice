#pragma once
#include "Search_DataListProvider.h"

// (j.jones 2015-03-23 13:57) - PLID 65281 - created

class CGiftCertificateSearch_DataListProvider_Payments : public LookupSearch::CSearch_Api_DataListProvider
{
public:
	//if nOnlyGCsForPatientID is not -1, the displayed GC will be those assigned to this patient
	//and any GCs not assigned to any patient
	CGiftCertificateSearch_DataListProvider_Payments(ADODB::_ConnectionPtr pCon, long nOnlyGCsForPatientID);
	~CGiftCertificateSearch_DataListProvider_Payments();
protected:

protected:
	//I went with ADODB connection ptr instead of CNxAdoConnection since this is likely to be in a lib and a little more generalized
	//	code available to all projects.
	/*override*/ SAFEARRAY *Search(BSTR bstrSearchString);
	/*override*/ void GetFieldValues(VARIANT aryFieldValues[], size_t nFieldValueCount, IUnknownPtr pResultUnk);
	/*override*/ HRESULT GetFieldValues_IfNoResults(VARIANT aryFieldValues[], size_t nFieldValueCount);

	//if not -1, the displayed GC will be those assigned to this patient
	//and any GCs not assigned to any patient
	long m_nOnlyGCsForPatientID;
};
