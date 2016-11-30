#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:19) - PLID 53526
class CMUMeasure_CORE_09 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_09(void);
	~CMUMeasure_CORE_09(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.CORE.09 - Patients who have smoking status reported."; }
	virtual CString GetShortName() { return "MU.CORE.09"; }
	virtual CString GetInternalName() { return "MU.10 - Patients who have smoking status reported."; }

};
