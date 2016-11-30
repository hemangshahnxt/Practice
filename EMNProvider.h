#pragma once

class EMNProvider {
public:
	long nID;
	BOOL bIsNew;
	CString strName;
	// (j.gruber 2009-05-07 16:45) - PLID 33688 - adding type ID
	long nTypeID;
	CString strTypeName;
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added FloatEMRData
	BOOL bFloatEMRData;

	EMNProvider() {
		nID = -1;
		bIsNew = TRUE;
		nTypeID = -1;		
		// (j.jones 2011-04-28 14:39) - PLID 43122 - added FloatEMRData
		bFloatEMRData = FALSE;
	}
};