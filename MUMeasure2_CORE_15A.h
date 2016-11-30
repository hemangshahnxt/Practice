#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2014-05-09 09:14) - PLID 59578 - Implement Detailed Reporting for MU.CORE.15.A for Stage 2

class CMUMeasure2_CORE_15A :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_15A(void);
	~CMUMeasure2_CORE_15A(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.15.A - Transitions of Care are provided summary of care."; }
	virtual CString GetShortName() { return "MU2.CORE.15A"; }
	// (b.savon 2014-05-06 10:01) - PLID 59573 - Implement Detailed Reporting for MU.CORE.08 for Stage 2
	// Yes, I know it says 3 business days, but it is wrong in the Summary report too and the configrt value
	// is saved off of the wrong value.  So, if I change it, whoever configured it before this release would lose
	// the configrt value they setup
	virtual CString GetInternalName() { return "MU2.C15A - Transitions of Care are provided summary of care."; }
};
