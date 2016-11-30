#pragma once
#include "MUMeasureBase.h"

// (j.dinatale 2012-10-24 14:38) - PLID 53507 - eRx measure

class CMUMeasure_CORE_04 : public CMUMeasureBase
{
public:
	CMUMeasure_CORE_04(void);
	~CMUMeasure_CORE_04(void);

	double GetRequirePrecent(){ return 40.0; }

	virtual CString GetFullName() { return "MU.CORE.04 - Prescriptions sent electronically."; }
	virtual CString GetShortName() { return "MU.CORE.04"; }
	virtual CString GetInternalName() { return "MU.08 - Prescriptions sent electronically."; }

	virtual CSqlFragment GetMeasureSql();
	virtual MU::MeasureData GetMeasureInfo();
};