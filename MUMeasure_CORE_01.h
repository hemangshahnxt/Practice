#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:11) - PLID 53531
class CMUMeasure_CORE_01 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_01(void);
	~CMUMeasure_CORE_01(void);

	double GetRequirePrecent(){ return 30.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.01 - Patients who have medication orders entered through CPOE"; }
	virtual CString GetShortName() { return "MU.CORE.01"; }
	virtual CString GetInternalName() { return "MU.07 - Patients who have medication orders entered through CPOE"; }

};
