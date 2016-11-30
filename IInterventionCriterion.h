
#pragma once

#include "InterventionUtils.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	typedef boost::shared_ptr<class IInterventionCriterion> IInterventionCriterionPtr;

	class IInterventionCriterion
	{
	public:

		//ID - Read
		__declspec(property(get=GetID)) long ID;
		virtual long GetID() = 0;

		//Name - Read
		__declspec(property(get=GetName)) CString Name;
		virtual CString GetName() = 0;

		//Type - Read
		__declspec(property(get=GetType)) CString Type;
		virtual CString GetType() = 0;

		//TypeID - Read
		__declspec(property(get=GetTypeID)) BYTE TypeID;
		virtual BYTE GetTypeID() = 0;

		//RecordID - Read
		__declspec(property(get=GetRecordID)) _variant_t RecordID;
		virtual _variant_t GetRecordID() = 0;

		//Operator - Read/Write
		__declspec(property(get=GetOperator,put=PutOperator)) InterventionCriteriaOperator Operator;
		virtual InterventionCriteriaOperator GetOperator() = 0;
		virtual void PutOperator(InterventionCriteriaOperator value) = 0;

		//Value - Read/Write
		__declspec(property(get=GetValue)) CString Value;
		virtual CString GetValue() = 0;

		//DisplayValue - Read
		__declspec(property(get=GetDisplayValue,put=PutDisplayValue)) CString DisplayValue;
		virtual CString GetDisplayValue() = 0;
		virtual void PutDisplayValue(CString &value) = 0;

		//LastXDays - Read/Write
		__declspec(property(get=GetLastXDays,put=PutLastXDays)) long LastXDays;
		virtual long GetLastXDays() = 0;
		virtual void PutLastXDays(long value) = 0;

		//EMRInfoType - Read
		__declspec(property(get=GetEmrInfoType)) ::EmrInfoType EmrInfoType;
		virtual ::EmrInfoType GetEmrInfoType() = 0;

		virtual bool IsValueAList() = 0;

		virtual bool Delete() = 0;

		virtual NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatOverride() = 0;
		virtual NXDATALIST2Lib::IFormatSettingsPtr GetValueFormatOverride() = 0;

		virtual void LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow) = 0;

		virtual bool IsValid() = 0;
		virtual bool ValidateValue(CString &strValue, VARIANT* pvarNewValue) = 0;
		virtual bool IsValueEnabled() = 0;
		virtual bool IsLastXDaysEnabled() = 0;

		virtual void OnClickValue() = 0;
	};
}