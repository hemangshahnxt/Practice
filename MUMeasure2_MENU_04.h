#pragma once
#include "mumeasurebase.h"
//TES 10/16/2013 - PLID 59583 - MU.MENU.04 - Created
class CMUMeasure2_MENU_04 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_MENU_04(void);
	~CMUMeasure2_MENU_04(void);

	double GetRequirePrecent(){ return 20.0; }

	virtual CString GetFullName() { return "MU.MENU.04 - Patients who have family history reported."; }
	virtual CString GetShortName() { return "MU.MENU.04"; }
	// (s.dhole 2014-05-19 10:30) - PLID 59583  Change internal name
	virtual CString GetInternalName() { return "MU2.M04 - Patients who have family history reported."; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();
};
