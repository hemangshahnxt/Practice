#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-11-18 12:22) - PLID 59563 - Implement Detailed Report for MU.CORE.01.A for Stage 2
class CMUMeasure2_CORE_01A :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_01A(void);
	~CMUMeasure2_CORE_01A(void);

	double GetRequirePrecent(){ return 60.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.01.A - Medication orders which have been entered through CPOE"; }
	virtual CString GetShortName() { return "MU2.CORE.01.A"; }
	virtual CString GetInternalName() { return "MU2.C01A - Medications orders which have been entered through CPOE"; }

};
