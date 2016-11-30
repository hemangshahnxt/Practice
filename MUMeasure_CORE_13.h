#pragma once
#include "mumeasurebase.h"

// (j.dinatale 2012-10-24 14:40) - PLID 53505 - Clinical Summaries

class CMUMeasure_CORE_13 :
	public CMUMeasureBase
{
public:
	CMUMeasure_CORE_13(void);
	~CMUMeasure_CORE_13(void);

	double GetRequirePrecent(){ return 50.0; }

	CSqlFragment GetMeasureSql();
	MU::MeasureData GetMeasureInfo();

	// (b.savon 2014-05-14 07:51) - PLID 62127 - Rename MUS1 Core 13 to Core 12.  Also update the numerator text
	virtual CString GetFullName() { return "MU.CORE.12 - Clinical summaries provided within 3 business days"; }
	virtual CString GetShortName() { return "MU.CORE.12"; }
	virtual CString GetInternalName() { return "MU.13 - Clinical summaries provided within 3 business days"; }
};
