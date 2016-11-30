#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 10:14) - PLID 53530
class CMUMeasure_CORE_03 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_03(void);
	~CMUMeasure_CORE_03(void);

	double GetRequirePrecent(){ return 80.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.03 - Patients who have current or up to date diagnoses"; }
	virtual CString GetShortName() { return "MU.CORE.03"; }
	virtual CString GetInternalName() { return "MU.01 - Patients who have current or up to date diagnoses"; }

};
