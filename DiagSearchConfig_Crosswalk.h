#pragma once
#include "DiagSearchConfig.h"

// (j.jones 2014-02-10 09:09) - PLID 60666 - Derived class of CDiagSearchConfig.  See it for the majority of the
//	system documentation.  We implement creation and parsing of results from diag search lists.
class CDiagSearchConfig_Crosswalk : public CDiagSearchConfig
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearchConfig_Crosswalk(bool bIncludePCS);
	CDiagSearchResults ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);
	void CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon);
};
