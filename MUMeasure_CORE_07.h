#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 10:08) - PLID 53527
class CMUMeasure_CORE_07 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_07(void);
	~CMUMeasure_CORE_07(void);

	double GetRequirePrecent(){ return 50.0; }

	virtual CString GetFullName() { return "MU.CORE.07 - Patients who have all Demographic fields filled out."; }
	virtual CString GetShortName() { return "MU.CORE.07"; }
	virtual CString GetInternalName() { return "MU.04 - Patients who have all Demographic fields filled out."; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
