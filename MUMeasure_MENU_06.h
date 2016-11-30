#pragma once
#include "mumeasurebase.h"
// (j.gruber 2012-10-25 09:08) - PLID 53522
class CMUMeasure_MENU_06 :
	public CMUMeasureBase
{
public:
	CMUMeasure_MENU_06(void);
	~CMUMeasure_MENU_06(void);

	double GetRequirePrecent(){ return 10.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	// (b.savon 2014-05-14 07:33) - PLID 62129 - Rename MUS1 Menu 6 to MUS1 Menu 5 and update the numerator text
	virtual CString GetFullName() { return "MU.MENU.05 - Patients with Educational Resources Sent"; }
	virtual CString GetShortName() { return "MU.MENU.05"; }
	virtual CString GetInternalName() { return "MU.05 - Patients with Educational Resources Sent"; }

};
