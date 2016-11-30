#pragma once
#include "mumeasurebase.h"
//TES 10/15/2013 - PLID 59568 - MU.CORE.04 - Created
class CMUMeasure2_CORE_04 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_04(void);
	~CMUMeasure2_CORE_04(void);

	double GetRequirePrecent(){ return 80.0; }

	virtual CString GetFullName() { return "MU2.CORE.04 - Patients with Vitals recorded."; }
	virtual CString GetShortName() { return "MU2.CORE.04"; }
	//TES 10/15/2013 - PLID 59568 - MU.CORE.04 - Put MU2 in the internal name to distinguish, it shouldn't be displayed to the user
	virtual CString GetInternalName() { return "MU2.C04 - Patients with Height, Weight, and Blood Pressure recorded"; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
