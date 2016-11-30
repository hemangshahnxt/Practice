#pragma once
#include "mumeasurebase.h"

// (j.dinatale 2012-10-24 14:43) - PLID 53500 - Medication Reconciliation

class CMUMeasure_MENU_07 :
	public CMUMeasureBase
{
public:
	CMUMeasure_MENU_07(void);
	~CMUMeasure_MENU_07(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	// (b.savon 2014-05-14 07:32) - PLID 62131 - Rename MUS1 Menu 7 to MUS1 Menu 6 and update the numerator text
	virtual CString GetFullName() { return "MU.MENU.06 - Referrals that have medication reconciliation performed."; }
	virtual CString GetShortName() { return "MU.MENU.06"; }
	virtual CString GetInternalName() { return "MU.15 - Referrals that have medication reconciliation performed."; }
};