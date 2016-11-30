#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:16) - PLID 53524
class CMUMeasure_MENU_02 :
	public CMUMeasureBase
{
public:
	CMUMeasure_MENU_02(void);
	~CMUMeasure_MENU_02(void);

	double GetRequirePrecent(){ return 40.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU.MENU.02 - Clinical Labs having results in structured data."; }
	virtual CString GetShortName() { return "MU.MENU.02"; }
	virtual CString GetInternalName() { return "MU.11 - Clinical Labs having results in structured data."; }

};
