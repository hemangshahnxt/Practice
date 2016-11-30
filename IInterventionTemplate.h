
#pragma once

#include "IInterventionCriterion.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	typedef boost::shared_ptr<class IInterventionTemplate> IInterventionTemplatePtr;

	class IInterventionTemplate
	{
	public:
		virtual CString GetName() = 0;
		virtual long GetID() = 0;
		virtual CString GetMaterials() = 0;
		virtual CString GetGuidelines() = 0;

		virtual bool SetName(CString &strName) = 0;
		virtual bool SetMaterials(CString &strText) = 0;
		virtual bool SetGuidelines(CString &strText) = 0;

		virtual bool Delete() = 0;
		// (s.dhole 2013-10-31 16:17) - PLID 
		virtual CString GetDeveloper() = 0;
		virtual CString GetFundingInfo() = 0;
		virtual COleDateTime GetReleaseDate() = 0;
		virtual COleDateTime GetRevisionDate() = 0;
		virtual bool SetDeveloper(CString &strText) = 0;
		virtual bool SetFundingInfo(CString &strText) = 0;
		virtual bool SetReleaseDate(COleDateTime &Dt) = 0;
		virtual bool SetRevisionDate(COleDateTime &Dt) = 0;



		virtual IInterventionCriterionPtr AddCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow) = 0;

		virtual long AddCompletionItem(long nID, BYTE nType) { return -1; }

		virtual void LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow) = 0;
	};
}