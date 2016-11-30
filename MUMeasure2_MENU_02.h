#pragma once
#include "mumeasurebase.h"
// (r.farnworth 2013-10-29 14:52) - PLID 59581 - Implement Stage 2 MU.MENU.02
class CMUMeasure2_MENU_02 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_MENU_02(void);
	~CMUMeasure2_MENU_02(void);

	double GetRequirePrecent(){ return 30.0; }

	virtual CString GetFullName() { return "MU.MENU.02 - Patients who have at least one electronic progress note as text searchable data."; }
	virtual CString GetShortName() { return "MU.MENU.02"; }
	virtual CString GetInternalName() { return "MU2.MENU.02 - Patients who have at least one electronic progress note as text searchable data."; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
