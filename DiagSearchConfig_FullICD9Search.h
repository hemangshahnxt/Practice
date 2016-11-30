#pragma once
#include "DiagSearchConfig.h"

// (j.jones 2015-09-01 14:37) - PLID 66993 - Derived class of CDiagSearchConfig.  See it for the majority of the
//	system documentation.  We implement creation and parsing of results from diag search lists.
class CDiagSearchConfig_FullICD9Search : public CDiagSearchConfig
{
public:
	CDiagSearchConfig_FullICD9Search(bool bIncludePCS);
	CDiagSearchResults ConvertRowToSearchResults(NXDATALIST2Lib::IRowSettingsPtr pSearchRow);
	void CreateResultList(NXDATALIST2Lib::_DNxDataListPtr pDataList, ADODB::_ConnectionPtr pCon);
};
