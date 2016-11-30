#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2014-05-09 10:07) - PLID 59579 - Implement Detailed Reporting for MU.CORE.15.B for Stage 2

class CMUMeasure2_CORE_15B :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_15B(void);
	~CMUMeasure2_CORE_15B(void);

	double GetRequirePrecent(){ return 10.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.15.B - Transitions of Care have transmitted summary of care."; }
	virtual CString GetShortName() { return "MU2.CORE.15B"; }
	// (b.savon 2014-05-06 10:01) - PLID 59573 - Implement Detailed Reporting for MU.CORE.08 for Stage 2
	// Yes, I know it says 3 business days, but it is wrong in the Summary report too and the configrt value
	// is saved off of the wrong value.  So, if I change it, whoever configured it before this release would lose
	// the configrt value they setup
	virtual CString GetInternalName() { return "MU2.C15B - Transitions of Care have transmitted summary of care."; }
};
