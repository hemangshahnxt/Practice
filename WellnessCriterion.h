
#pragma once

#include "IInterventionCriterion.h"
#include "WellnessUtils.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	class WellnessCriterion : public IInterventionCriterion
	{
	public:
		WellnessCriterion(long nID,	long nRuleID, CString strName, CString strType, WellnessTemplateCriterionType wtctTypeID,
			_variant_t varRecordID, InterventionCriteriaOperator icoOperator, CString strValue, CString strDisplayValue, 
			long nLastXDays, ::EmrInfoType eitEmrInfoType);

	public:

		virtual long GetID();
		virtual long GetRuleID();
		virtual CString GetName();
		virtual CString GetType();
		virtual BYTE GetTypeID();
		virtual _variant_t GetRecordID();
		virtual InterventionCriteriaOperator GetOperator();
		virtual void PutOperator(InterventionCriteriaOperator value);
		virtual CString GetValue();
		virtual void PutDisplayValue(CString &value);
		virtual CString GetDisplayValue();
		virtual long GetLastXDays();
		virtual void PutLastXDays(long value);
		virtual ::EmrInfoType GetEmrInfoType();

		virtual bool IsValueAList();
		virtual NXDATALIST2Lib::IFormatSettingsPtr GetOperatorFormatOverride();
		virtual NXDATALIST2Lib::IFormatSettingsPtr GetValueFormatOverride();

		virtual bool Delete();
		virtual void LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
		virtual bool IsValid();
		virtual bool ValidateValue(CString &strValue, VARIANT* pvarNewValue);
		virtual bool IsValueEnabled();
		virtual bool IsLastXDaysEnabled();

		virtual void OnClickValue();

	private:
		void RefreshCalculatedValues();

	private:
		long m_nID;
		long m_nRuleID;
		CString m_strName;
		CString m_strType;
		WellnessTemplateCriterionType m_wtctTypeID;
		_variant_t m_varRecordID;
		InterventionCriteriaOperator m_icoOperator;
		CString m_strValue;
		CString m_strDisplayValue;
		long m_nLastXDays;
		::EmrInfoType m_eitEmrInfoType;

		bool m_bValueIsList;
		NXDATALIST2Lib::IFormatSettingsPtr m_pOperatorFormat;
		NXDATALIST2Lib::IFormatSettingsPtr m_pValueFormat;
	};
}