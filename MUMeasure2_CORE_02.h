#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-11-18 12:30) - PLID 59566 - Implement Detailed Reporting for MU.CORE.02 for Stage 2

class CMUMeasure2_CORE_02 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_02(void);
	~CMUMeasure2_CORE_02(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.02 - Prescriptions sent electronically"; }
	virtual CString GetShortName() { return "MU2.CORE.02"; }
	virtual CString GetInternalName() { return "MU2.C02 - Prescriptions sent electronically."; }
};
