#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-10-21 12:21) - PLID 59576 - Created, copied from MU.MENU.06

class CMUMeasure2_CORE_13 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_CORE_13(void);
	~CMUMeasure2_CORE_13(void);

	double GetRequirePrecent(){ return 10.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.CORE.13 - Patients with Educational Resources Sent"; }
	virtual CString GetShortName() { return "MU2.CORE.13"; }
	virtual CString GetInternalName() { return "MU2.C13 - Patients with Educational Resources Sent"; }
};