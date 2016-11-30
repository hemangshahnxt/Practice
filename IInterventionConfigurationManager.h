
#pragma once

#include "IInterventionTemplate.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	typedef boost::shared_ptr<class IInterventionConfigurationManager> IInterventionConfigurationManagerPtr;

	class IInterventionConfigurationManager
	{
	public:	
		virtual IInterventionTemplatePtr GetSelectedTemplate() = 0;

		//Dlg Configuration Settings
		virtual inline CString GetDlgName() = 0;
		virtual inline long GetNameConstraint() = 0;
		virtual inline bool ShowCompletionList() = 0;
		virtual inline bool ShowLastXDays() = 0;

		//Template and State Changing Operations
		virtual IInterventionTemplatePtr GetNewTemplate() = 0;
		virtual IInterventionTemplatePtr GetTemplatePtr(CString &strName, long nID) = 0;
		virtual IInterventionTemplatePtr LoadTemplateFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow) = 0;
		virtual void SetSelectedTemplate(IInterventionTemplatePtr pTemplate) = 0;
		virtual bool DeleteSelectedTemplate() = 0;

		//Datalist Queries
		virtual CString GetTemplateListFromClause() = 0;
		virtual CString GetTemplateListWhereClause() = 0;
		virtual CString GetCriteriaListFromClause() = 0;
		virtual CString GetCriteriaListWhereClause() = 0;
		virtual CString GetTemplateCriteriaFromClause() = 0;
		virtual CString GetTemplateCriteriaWhereClause() = 0;
		virtual CString GetCompletionListWhereClause() { return ""; }


		virtual bool RemoveCompletionItem(long nID) { return false; }
		virtual IInterventionCriterionPtr LoadCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow) = 0;
	};
}