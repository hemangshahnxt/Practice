#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2014-05-06 15:22) - PLID 59571 - Implement Detailed Reporting for MU.CORE.07.A for Stage 2

class CMUMeasure2_CORE_07A :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_07A(void);
	~CMUMeasure2_CORE_07A(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.07A - Patients provided with timely Electronic Access "; }
	virtual CString GetShortName() { return "MU2.CORE.07A"; }
	virtual CString GetInternalName() { return "MU2.C07A - Patients provided with timely Electronic Access "; }
};
