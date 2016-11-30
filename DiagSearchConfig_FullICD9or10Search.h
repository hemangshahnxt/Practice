#pragma once
#include "DiagSearchConfig.h"

// (j.jones 2014-02-06 11:50) - PLID 60685 - Derived class of CDiagSearchConfig.  See it for the majority of the
//	system documentation.  We implement creation and parsing of results from diag search lists.
class CDiagSearchConfig_FullICD9or10Search : public CDiagSearchConfig
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearchConfig_FullICD9or10Search(bool bIncludePCS);
	CDiagSearchResults ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);
	void CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon);
};
