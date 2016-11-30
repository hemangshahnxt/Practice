#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:17) - PLID 53525
class CMUMeasure_CORE_08 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_08(void);
	~CMUMeasure_CORE_08(void);

	double GetRequirePrecent(){ return 50.0; }
	
	virtual CString GetFullName() { return "MU.CORE.08 - Patients with Vitals recorded."; }
	virtual CString GetShortName() { return "MU.CORE.08"; }
	virtual CString GetInternalName() { return "MU.09 - Patients with Height, Weight, and Blood Pressure recorded"; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
