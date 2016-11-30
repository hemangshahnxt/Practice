#pragma once
#include "MUMeasureBase.h"

// (j.dinatale 2012-10-24 14:34) - PLID 53504 - Measure to provide our base demographic info and other misc info

class CMUMeasure_DEM_00 : public CMUMeasureBase
{
public:
	CMUMeasure_DEM_00(MU::Stage eMeaningfulUseStage);
	~CMUMeasure_DEM_00(void);

	double GetRequirePrecent(){ return 0.0; }

	virtual CString GetFullName() { return "MU.DEM.00 - Basic Demographics."; }
	virtual CString GetShortName() { return "MU.DEM.00"; }
	virtual CString GetInternalName() { return "MU.DEM.00 - Basic Demographics."; }

	virtual CSqlFragment GetMeasureSql();
	virtual MU::MeasureData GetMeasureInfo();
protected:
	MU::Stage m_eMeaningfulUseStage;
};