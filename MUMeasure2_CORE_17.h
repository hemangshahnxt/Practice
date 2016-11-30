#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-10-25 12:23) - PLID 59580 - Implement MU.CORE.17 for Stage 2
class CMUMeasure2_CORE_17 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_17(void);
	~CMUMeasure2_CORE_17(void);

	double GetRequirePrecent(){ return 5.0; }

	virtual CString GetFullName() { return "MU.CORE.17 - Patients who have sent secure electronic messages."; }
	virtual CString GetShortName() { return "MU.CORE.17"; }
	virtual CString GetInternalName() { return "MU2.CORE.17 - Patients who have sent secure electronic messages."; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
