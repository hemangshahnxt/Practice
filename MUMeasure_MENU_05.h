#pragma once
#include "mumeasurebase.h"

// (j.dinatale 2012-10-24 14:41) - PLID 53501 - Timely Electronic Access

class CMUMeasure_MENU_05 :
	public CMUMeasureBase
{
public:
	CMUMeasure_MENU_05(void);
	~CMUMeasure_MENU_05(void);

	// (b.savon 2014-06-06 14:20) - PLID 62344 - Move the top & bottom columns in the stage 1 detailed report for "Elect. Access" between "Smoking Status" and "Clin. Summary".  Also update the percentage to 50.
	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	// (b.savon 2014-05-14 07:34) - PLID 62128 - Rename MUS1 Menu 5 to MUS1 Core 11 and move between MU Core 9 and MU Core 12
	virtual CString GetFullName() { return "MU.CORE.11 - Patients provided with timely Electronic Access."; }
	virtual CString GetShortName() { return "MU.CORE.11"; }
	virtual CString GetInternalName() { return "MU.06 - Patients provided with timely Electronic Access "; }

};
