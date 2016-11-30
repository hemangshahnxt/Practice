#pragma once
#include "mumeasurebase.h"

// (j.dinatale 2012-10-24 14:44) - PLID 53499 - Transition of Care

class CMUMeasure_MENU_08 :
	public CMUMeasureBase
{
public:
	CMUMeasure_MENU_08(void);
	~CMUMeasure_MENU_08(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	// (b.savon 2014-05-14 07:36) - PLID 62132 - Rename MUS1 Menu 8 to MUS1 Menu 7 and update the numerator text
	virtual CString GetFullName() { return "MU.MENU.07 - Transitions of Care are provided summary of care."; }
	virtual CString GetShortName() { return "MU.MENU.07"; }
	virtual CString GetInternalName() { return "MU.16 - Transitions of Care are provided summary of care."; }
};