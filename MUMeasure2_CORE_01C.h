#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-11-18 12:28) - PLID 59565 - Implement Detailed Reporting for MU.CORE.01.C for Stage 2
class CMUMeasure2_CORE_01C :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_01C(void);
	~CMUMeasure2_CORE_01C(void);

	double GetRequirePrecent(){ return 30.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.01.C - Laboratory orders which have been entered through CPOE"; }
	virtual CString GetShortName() { return "MU2.CORE.01.C"; }
	virtual CString GetInternalName() { return "MU2.C01C - Laboratory orders which have been entered through CPOE"; }

};