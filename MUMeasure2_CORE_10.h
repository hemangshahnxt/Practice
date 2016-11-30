#pragma once
#include "mumeasurebase.h"
// (b.savon 2014-05-06 12:24) - PLID 59574 - Created, copied from MU.MENU.02

class CMUMeasure2_CORE_10 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_10(void);
	~CMUMeasure2_CORE_10(void);

	double GetRequirePrecent(){ return 55.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.10 - Clinical Labs having results in structured data"; }
	virtual CString GetShortName() { return "MU.CORE.10"; }
	// (b.savon 2014-05-06 12:24) - PLID 59574 - Implement Detailed Reporting for MU.CORE.10 for Stage 2
	virtual CString GetInternalName() { return "MU2.C10 - Clinical Labs having results in structured data."; }
};