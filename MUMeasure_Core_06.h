#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:40) - PLID 53528
class CMUMeasure_CORE_06 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_06(void);
	~CMUMeasure_CORE_06(void);

	double GetRequirePrecent(){ return 80.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.06 - Patients who have active Allergies."; }
	virtual CString GetShortName() { return "MU.CORE.06"; }
	virtual CString GetInternalName() { return "MU.03 - Patients who have active Allergies."; }

};
