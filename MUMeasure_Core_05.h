#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 10:12) - PLID 53529
class CMUMeasure_CORE_05 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_05(void);
	~CMUMeasure_CORE_05(void);

	double GetRequirePrecent(){ return 80.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.05 - Patients who have active Current Medications."; }
	virtual CString GetShortName() { return "MU.CORE.05"; }
	virtual CString GetInternalName() { return "MU.02 - Patients who have active Current Medications."; }

};
