#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-10-17 10:42) - PLID 59575 - Created, copied from stage one MU.MENU.04

class CMUMeasure2_CORE_12 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_12(void);
	~CMUMeasure2_CORE_12(void);

	double GetRequirePrecent(){ return 10.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.12 - Patients with at least 2 office visits in the last 2 years that recieved a reminder"; }
	virtual CString GetShortName() { return "MU.CORE.12"; }
	virtual CString GetInternalName() { return "MU2.C12 - Patients who were sent appropriate reminders"; }
};
