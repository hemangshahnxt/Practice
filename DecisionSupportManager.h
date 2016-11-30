
#pragma once

#include "IInterventionConfigurationManager.h"
#include "DecisionSupportTemplate.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	class DecisionSupportManager : public IInterventionConfigurationManager
	{
	public:	
		DecisionSupportManager();
		~DecisionSupportManager();

	private:
		IInterventionTemplatePtr m_pSelectedTemplate;

	//Implementation Overrides
	public:
		virtual IInterventionTemplatePtr GetSelectedTemplate();
		virtual inline CString GetDlgName();
		virtual inline bool ShowCompletionList();
		virtual IInterventionTemplatePtr GetNewTemplate();
		virtual IInterventionTemplatePtr GetTemplatePtr(CString &strName, long nID);
		virtual void SetSelectedTemplate(IInterventionTemplatePtr pTemplate);
		virtual bool DeleteSelectedTemplate();
		virtual CString GetTemplateListFromClause();
		virtual CString GetTemplateListWhereClause();
		virtual CString GetCriteriaListFromClause();
		virtual CString GetCriteriaListWhereClause();
		virtual CString GetTemplateCriteriaFromClause();
		virtual CString GetTemplateCriteriaWhereClause();
		virtual CString GetCompletionListWhereClause();
		virtual bool RemoveCompletionItem(long nID);
		virtual inline long GetNameConstraint();
		virtual inline bool ShowLastXDays();
		virtual IInterventionTemplatePtr LoadTemplateFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
		virtual IInterventionCriterionPtr LoadCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	};
}