#pragma once
#include "DiagSearchConfig.h"

// (d.thompson 2014-02-04) - PLID 60668 - Derived class of CDiagSearchConfig.  See it for the majority of the
//	system documentation.  We implement creation and parsing of results from diag search lists.
class CDiagSearchConfig_DiagCodesDropdown : public CDiagSearchConfig
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference,
	// though this search does not utilize it
	CDiagSearchConfig_DiagCodesDropdown(bool bIncludePCS);
	CDiagSearchResults ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);
	void CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon);
};
