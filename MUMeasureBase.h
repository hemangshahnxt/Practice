#pragma once
#include "MUMeasureFilter.h"

// (j.dinatale 2012-10-24 11:44) - PLID 53495 - the new MU data structure

#define WM_MEASURE_PRELOAD_COMPLETE	(WM_APP + 1)
#define WM_MEASURE_COMPLETE			(WM_APP + 2)
#define WM_ALL_MEASURES_COMPLETE	(WM_APP + 3)
#define WM_MEASURE_LOAD_CANCEL		(WM_APP + 4)

namespace MU
{
	// Measure IDs so we can identify which measure our returned data belongs to.
	enum MeasureNames{
		MU_DEM_00 = 0,	// Initial demographics set, this is a fake measure
		MU_CORE_01,
		MU_CORE_03,
		MU_CORE_04,
		MU_CORE_05,
		MU_CORE_06,
		MU_CORE_07,
		MU_CORE_08,		
		MU_CORE_09,
		MU_CORE_12,
		MU_CORE_13,
		MU_MENU_02,
		MU_MENU_04,
		MU_MENU_05,
		MU_MENU_06,
		MU_MENU_07,
		MU_MENU_08,

		//TES 10/15/2013 - PLID 58993 - Stage 2 measures go here
		MU2_CORE_01A, // (r.farnworth 2013-11-18 12:22) - PLID 59563 - Implement Detailed Report for MU.CORE.01.A for Stage 2
		MU2_CORE_01B, // (r.farnworth 2013-11-18 12:26) - PLID 59564 - Implement Detailed Reporting for MU.CORE.01.B for Stage 2
		MU2_CORE_01C, // (r.farnworth 2013-11-18 12:28) - PLID 59565 - Implement Detailed Reporting for MU.CORE.01.C for Stage 2
		MU2_CORE_02, // (r.farnworth 2013-11-18 12:30) - PLID 59566 - Implement Detailed Reporting for MU.CORE.02 for Stage 2
		MU2_CORE_03, //TES 10/15/2013 - PLID 59567 - MU.CORE.03
		MU2_CORE_04, //TES 10/15/2013 - PLID 59568 - MU.CORE.04
		MU2_CORE_05, //TES 10/15/2013 - PLID 59569 - MU.CORE.05
		MU2_CORE_07A, // (r.farnworth 2013-11-13 11:24) - PLID 59571 - MU.CORE.07.A
		MU2_CORE_07B, // (r.farnworth 2013-11-14 16:19) - PLID 59572 - MU.CORE.07.B
		MU2_CORE_08, // (r.farnworth 2013-10-15 14:47) - PLID 59573 - MU.CORE.08
		MU2_CORE_10, // (r.farnworth 2013-10-16 11:43) - PLID 59574 - MU.CORE.10
		MU2_CORE_12, // (r.farnworth 2013-10-16 15:53) - PLID 59575 - MU.CORE.12
		MU2_CORE_13, // (r.farnworth 2013-10-21 10:12) - PLID 59576 - MU.CORE.13
		MU2_CORE_14, // (r.farnworth 2013-10-30 15:12) - PLID 59577 - MU.CORE.14
		MU2_CORE_15A, // (r.farnworth 2013-11-14 11:17) - PLID 59578 - MU.CORE.15.A
		MU2_CORE_15B, // (r.farnworth 2013-11-14 14:07) - PLID 59579 - MU.CORE.15.B
		MU2_CORE_17, // (r.farnworth 2013-10-24 15:18) - PLID 59580 - MU.CORE.17

		MU2_MENU_02, // (r.farnworth 2013-10-29 10:38) - PLID 59581 - MU.MENU.02
		MU2_MENU_03, // (r.farnworth 2013-10-30 13:39) - PLID 59582 - MU.MENU.03
		MU2_MENU_04, //TES 10/16/2013 - PLID 59583 - MU.MENU.04
		Count,
	};

	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - We can determine what kind of query we need to run and union accordingly.
	enum UnionType {
		EMR,				//Doesn't require a special join
		AllStage1,				//For Dem_00. Will hit all S1 Unions
		AllStage2,				//For Dem_00. Will hit all S2 Unions
		ePrescribe,
		CPOEMedicationsStage2,	//History Docs
		CPOELaboratories,	//History Docs
		CPOERadiologies,	//History Docs
		LabResults,			//Labs and Docs
		ImagingResults,
		RemindersStage1,
		RemindersStage2
	};

	// this is the type of each individual data point on PersonMeasureData.
	enum DataPointType{
		Unknown = -1,
		RawMeasure = 0,	// type which specifies if the individual patient met the requirements of the measure
		LastName,
		PatientID,
		Location,
		EMNDate,
		Height,
		Weight,
		eRx,
		Language,
		Gender,
		Race,
		Ethnicity,
		BirthDate,
		BloodPressure,
		CPOE,
		SmokingStatus,
		Labs,
		FamilyHistory, //TES 10/16/2013 - PLID 59583 - MU.MENU.04
	};

	enum Stage {
		Stage1 = 0,
		Stage2,
		ModStage2,
	};

	// this struct spells out the possible data points a person may have for a measure.
	struct DataPointInfo{
		DataPointType DataType;	// When adding data points to measures, make sure the MeasureID, DataPointType is unique
		CString strName;	// visible name to the user for this data point
		long nNumerator;	// the numerator total for this portion of the measure
		long nDenominator;	// the total denominator for this portion of the measure

		DataPointInfo()
			: DataType(Unknown)
			, nNumerator(0)
			, nDenominator(0)
		{
		}

		DataPointInfo(CString strName, DataPointType DataType)
			: strName(strName)
			, DataType(DataType)
			, nNumerator(0)
			, nDenominator(0)
		{
		}

		void operator =(const DataPointInfo &cSource)
		{
			DataType = cSource.DataType;
			strName = cSource.strName;
		}

		// calculates the percentage for the measure
		double GetPercentage()
		{
			if(nDenominator == 0){
				return 0.0f;
			}

			return 100.0f * ((double)nNumerator) / nDenominator;
		}
	};

	// data point which is used to specify a data point for a patient
	struct DataPoint{
		DataPointType DataType;	// type of the datapoint
		long nDenominator;
		long nNumerator;
		CString strVisibleData;	// data in string format that will be visible to the user

		DataPoint()
			: DataType(Unknown)
			, nDenominator(0)
			, nNumerator(0)
		{
		}

		void operator =(const DataPoint &cSource)
		{
			DataType = cSource.DataType;
			nDenominator = cSource.nDenominator;
			nNumerator = cSource.nNumerator;
			strVisibleData = cSource.strVisibleData;
		}

		// calculates the percentage for the measure
		double GetPercentage()
		{
			if(nDenominator == 0){
				return 0.0f;
			}

			return 100.0f * ((double)nNumerator) / nDenominator;
		}
	};

	// patient data for this measure data
	struct PersonMeasureData{
		long nPersonID;	// patient ID of patient contributing to the measure
		long nNumerator;	// the amount the patient is contributing to the overall numerator
		long nDenominator;	// the amount the patient is contributing to the overall denominator
		std::vector<DataPoint> DataPoints;

		PersonMeasureData() 
			: nNumerator(0)
			, nDenominator(0)
			, nPersonID(-1)
		{
		}

		void operator =(const PersonMeasureData &cSource)
		{
			nPersonID = cSource.nPersonID;
			nNumerator = cSource.nNumerator;
			nDenominator = cSource.nDenominator;
			DataPoints.clear();
			std::copy(cSource.DataPoints.begin(), cSource.DataPoints.end(), DataPoints.begin());
		}
	};
	
	// collection of data for a measure
	struct MeasureData{
		// general numerator and denominator info for this measure
		long nNumerator;
		long nDenominator;

		// which measure is this?
		MeasureNames MeasureID;
		CString strFullName;
		CString strShortName;
		CString strInternalName;

		// percent required to pass
		double dblRequiredPercent;

		// detailed measure information per patient
		std::vector<PersonMeasureData> MeasureInfo;

		// this is the collection of potential data points. This will help dialogs construct lists to display data
		std::vector<DataPointInfo> DataPointInfo;

		MeasureData() 
			: nNumerator(0)
			, nDenominator(0)
			, dblRequiredPercent(0.0f)
			, MeasureID(MU_DEM_00)
		{
		}

		void operator =(const MeasureData &cSource)
		{
			nNumerator = cSource.nNumerator;
			nDenominator = cSource.nDenominator;
			MeasureID = cSource.MeasureID;
			dblRequiredPercent = cSource.dblRequiredPercent;

			MeasureInfo.clear();
			std::copy(cSource.MeasureInfo.begin(), cSource.MeasureInfo.end(), MeasureInfo.begin());

			DataPointInfo.clear();
			std::copy(cSource.DataPointInfo.begin(), cSource.DataPointInfo.end(), DataPointInfo.begin());
		}
	};
};

// (j.dinatale 2012-10-24 11:44) - PLID 53495 - Our base measure object, all measure objects must derive from this
class CMUMeasureBase
{
public:
	CMUMeasureBase(void);
	~CMUMeasureBase(void);

	MUMeasureFilter m_filterPatients;
	MUMeasureFilter m_filterMURange;

	virtual MU::MeasureData GetMeasureInfo() = 0;

private:
	virtual CSqlFragment GetMeasureSql() = 0;

	virtual CString GetFullName() = 0;
	virtual CString GetShortName() = 0;
	virtual CString GetInternalName() = 0;

protected:
	CSqlFragment GetPatientBaseQuery();
	CSqlFragment GetPatientEMRBaseQuery();
	//(s.dhole 8/1/2014 3:53 PM ) - PLID 63044 
	CSqlFragment GetPatientEMRIDBaseQuery();
	CSqlFragment GetPatientEmrIDBaseQuery();
	// (r.farnworth 2014-05-19 08:10) - PLID 59575 - Added TwoyearsPrior
	CSqlFragment GetAllMUPatients(MU::UnionType ToJoin = MU::EMR);

	// (r.farnworth 2014-06-04 16:37) - PLID 62325 - All functions needed to make the 00_DEM query work
	CSqlFragment UnioneRx();
	CSqlFragment UnionS1CPOEMeds();
	CSqlFragment UnionS2CPOEMeds();
	CSqlFragment UnionCPOERads();
	CSqlFragment UnionCPOELabs();
	CSqlFragment UnionLabResults();
	CSqlFragment UnionImagingResults();
	CSqlFragment UnionS1Reminders();
	CSqlFragment UnionS2Reminders();

	CString GetCommonDemographicsTableDefs();
	CString GetCommonDemographicsFieldNames();
	//(s.dhole 9/26/2014 8:37 AM ) - PLID 63765
	CSqlFragment GetClinicalSummaryCommonDenominatorSQL(CString strCodes);
	CSqlFragment GetClinicalNumeratorFilterSQL();
};