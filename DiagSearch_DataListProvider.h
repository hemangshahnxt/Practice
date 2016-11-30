#pragma once

#include "Search_DataListProvider.h"

// (d.thompson 2014-01-31) - PLID 60607:  This is the base class for all IDataListProvider implementations.  This 
//	class implements the base requirements and handles the messy IUnknown functions/reference counting that we
//	have to implement ourselves.  Please derive from this class instead of trying to derive from IDataListProvider
//	directly.
//
//	Usage:  Derive a class, implement a suitable LoadListData() function, and set it as the DataListProvider member
//	of any search based datalist.

class CDiagSearch_DataListProvider : public LookupSearch::CSearch_Api_DataListProvider
{
public:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	CDiagSearch_DataListProvider(bool bIncludePCS);
	//(s.dhole 2/27/2015 10:54 AM ) - PLID 64612 this should be virtual function
	virtual ~CDiagSearch_DataListProvider(void);

protected:
	// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
	bool m_bIncludePCS;

protected:
	// (z.manning 2014-03-05 10:55) - PLID 61140
	HICON GetQuicklistIcon();

private:
	// (z.manning 2014-03-03 13:48) - PLID 61140 - Icon for showing which items are in the user's quick list
	HICON m_hQuicklistIcon;
};
