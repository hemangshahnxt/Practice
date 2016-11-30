#pragma once
#include "DiagSearchConfig.h"

// (j.jones 2014-02-10 09:03) - PLID 60667 - Derived class of CDiagSearchConfig.  See it for the majority of the
//	system documentation.  We implement creation and parsing of results from crosswalk enabled diag search lists.
class CDiagSearchConfig_ManagedICD10Search : public CDiagSearchConfig
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference,
	// though this search does not utilize it
	CDiagSearchConfig_ManagedICD10Search(bool bIncludePCS);
	CDiagSearchResults ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);
	void CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon);
};
