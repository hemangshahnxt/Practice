#pragma once
#include "mumeasurebase.h"
//TES 10/16/2013 - PLID 59569 - MU.CORE.05 - Created
class CMUMeasure2_CORE_05 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_05(void);
	~CMUMeasure2_CORE_05(void);

	double GetRequirePrecent(){ return 80.0; }

	virtual CString GetFullName() { return "MU.CORE.05 - Patients who have smoking status reported."; }
	virtual CString GetShortName() { return "MU.CORE.05"; }
	//TES 10/16/2013 - PLID 59569 - MU.CORE.05 - Keep the same internal name as CMUMeasure_CORE_09, so the configuration will use the same item
	//(s.dhole 2014-05-06) PLID  59569
	virtual CString GetInternalName() { return "MU2.C05 - Patients who have smoking status reported."; }
	
	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
