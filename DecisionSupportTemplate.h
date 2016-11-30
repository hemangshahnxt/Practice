
#pragma once

#include "IInterventionTemplate.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

namespace Intervention
{
	class DecisionSupportTemplate : public IInterventionTemplate
	{
	public:
		DecisionSupportTemplate(CString &strName, long nID, CString &strToDoMessage);
		
	public:
		virtual CString GetName();
		virtual long GetID();
		virtual CString GetMaterials();
		virtual CString GetGuidelines();

		virtual bool SetName(CString &strName);
		virtual bool SetMaterials(CString &strText);
		virtual bool SetGuidelines(CString &strText);

		virtual bool Delete();

		virtual long AddCompletionItem(long nID, BYTE nType);
		// (s.dhole 2013-10-31 16:17) - PLID 
		virtual CString GetDeveloper();
		virtual CString GetFundingInfo();
		virtual COleDateTime GetReleaseDate();
		virtual COleDateTime GetRevisionDate();
		virtual bool SetDeveloper(CString &strText);
		virtual bool SetFundingInfo(CString &strText);
		virtual bool SetReleaseDate(COleDateTime &Dt);
		virtual bool SetRevisionDate(COleDateTime &Dt);


	


		virtual IInterventionCriterionPtr AddCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
		virtual void LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	private:
		void LoadData();

	private:
		CString m_strName;
		long m_nID;
		CString m_strToDoMessage;
		CString m_strMaterials;
		CString m_strGuidelines;
		CString m_strDeveloper;
		CString m_strFunding;
		COleDateTime m_dtRelease;
		COleDateTime m_dtRevision;

		bool m_bQueried;

	};
}