#pragma once
#include "mumeasurebase.h"

// (j.dinatale 2012-10-24 14:39) - PLID 53506 - Electronic health info

class CMUMeasure_CORE_12 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_12(void);
	~CMUMeasure_CORE_12(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	// (b.savon 2014-05-14 08:00) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”.  Move the measure to the very bottom of the list.
	virtual CString GetFullName() { return "[OBSOLETE] - Patients who request electronic copies of health information were provided it within 3 business days"; }
	virtual CString GetShortName() { return "MU.CORE.12.[OBSOLETE]"; }
	virtual CString GetInternalName() { return "MU.12 - Patients who request electronic copies of health information were provided it within 3 business days"; }

};
