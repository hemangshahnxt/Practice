#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-11-18 12:26) - PLID 59564 - Implement Detailed Reporting for MU.CORE.01.B for Stage 2
class CMUMeasure2_CORE_01B :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_01B(void);
	~CMUMeasure2_CORE_01B(void);

	double GetRequirePrecent(){ return 30.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.01.B - Radiology orders which have been entered through CPOE"; }
	virtual CString GetShortName() { return "MU2.CORE.01.B"; }
	virtual CString GetInternalName() { return "MU2.C01B - Radiology orders which have been entered through CPOE"; }

};