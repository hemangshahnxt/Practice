#pragma once
#include "mumeasurebase.h"

// (b.savon 2014-05-06 12:09) - PLID 59577 - Implement MU.CORE.14 for Stage 2

class CMUMeasure2_CORE_14 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_14(void);
	~CMUMeasure2_CORE_14(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.14 - Referrals that have medication reconciliation performed."; }
	virtual CString GetShortName() { return "MU.CORE.14"; }
	// (b.savon 2014-05-06 12:09) - PLID 59577 - Use the correct internal name so that we get the correct configrt 
	// value that was saved from the summary report
	virtual CString GetInternalName() { return "MU2.C14 - Referrals that have medication reconciliation performed."; }
};