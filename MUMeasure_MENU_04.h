#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:14) - PLID  53523
class CMUMeasure_MENU_04 :
	public CMUMeasureBase
{
public:
	CMUMeasure_MENU_04(void);
	~CMUMeasure_MENU_04(void);

	double GetRequirePrecent(){ return 20.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.MENU.04 - Patients who were sent appropriate reminders"; }
	virtual CString GetShortName() { return "MU.MENU.04"; }
	virtual CString GetInternalName() { return "MU.14 - Patients who were sent appropriate reminders"; }

};
