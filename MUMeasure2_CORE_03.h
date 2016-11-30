#pragma once
#include "mumeasurebase.h"
//TES 10/15/2013 - PLID 59567 - MU.CORE.03 - Created, copied from stage one MU.CORE.07
class CMUMeasure2_CORE_03 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_03(void);
	~CMUMeasure2_CORE_03(void);

	double GetRequirePrecent(){ return 80.0; }

	virtual CString GetFullName() { return "MU2.CORE.03 - Patients who have all Demographic fields filled out."; }
	virtual CString GetShortName() { return "MU2.CORE.03"; }
	//TES 10/15/2013 - PLID 59567 - MU.CORE.03 - Put MU2 in the internal name to distinguish, it shouldn't be displayed to the user
	virtual CString GetInternalName() { return "MU2.C03 - Patients who have all Demographic fields filled out."; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
