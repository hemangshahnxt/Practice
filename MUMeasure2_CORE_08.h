#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-10-15 14:56) - PLID 59573 - Created, copied from stage one MU.CORE.13

class CMUMeasure2_CORE_08 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_08(void);
	~CMUMeasure2_CORE_08(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.08 - Clinical summaries provided within 1 business day"; }
	virtual CString GetShortName() { return "MU.CORE.08"; }
	// (b.savon 2014-05-06 10:01) - PLID 59573 - Implement Detailed Reporting for MU.CORE.08 for Stage 2
	// Yes, I know it says 3 business days, but it is wrong in the Summary report too and the configrt value
	// is saved off of the wrong value.  So, if I change it, whoever configured it before this release would lose
	// the configrt value they setup
	virtual CString GetInternalName() { return "MU2.C08 - Clinical summaries provided within 3 business days"; }
};
