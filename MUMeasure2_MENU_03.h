#pragma once
#include "mumeasurebase.h"
// (b.savon 2014-05-14 11:14) - PLID 59582 - Implement Detailed Reporting for MU.MENU.03 for Stage 2
class CMUMeasure2_MENU_03 :
	public CMUMeasureBase
{
public:
	CMUMeasure2_MENU_03(void);
	~CMUMeasure2_MENU_03(void);

	double GetRequirePrecent(){ return 10.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	virtual CString GetFullName() { return "MU2.MENU_03 - Number of test whose result is one or more images that were received"; }
	virtual CString GetShortName() { return "MU2.MENU_03"; }
	virtual CString GetInternalName() { return "MU2.M03 - Number of test whose result is one or more images that were received"; }

};