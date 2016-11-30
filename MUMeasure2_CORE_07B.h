#pragma once
#include "mumeasurebase.h"
// (s.dhole 2014-05-06 16:22) - PLID 59572 - Implement Detailed Reporting for MU.CORE.07.B for Stage 2


class CMUMeasure2_CORE_07B :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_07B(void);
	~CMUMeasure2_CORE_07B(void);

	double GetRequirePrecent(){ return 5.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.07B - Patients provided with timely Electronic Access "; }
	virtual CString GetShortName() { return "MU2.CORE.07B"; }
	virtual CString GetInternalName() { return "MU2.C07B - Patients provided with timely Electronic Access "; }
};
