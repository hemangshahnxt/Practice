
#include "stdafx.h"
#include "DecisionSupportTemplate.h"
#include "DecisionSupportCriterion.h"
#include "InterventionUtils.h"
#include "DecisionRuleUtils.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

using namespace Intervention;

//DecisionSupport Template

DecisionSupportTemplate::DecisionSupportTemplate(CString &strName, long nID, CString &strToDoMessage)
{
	m_strName = strName;
	m_nID = nID;
	m_strToDoMessage = strToDoMessage;
	m_bQueried = false;
}

CString DecisionSupportTemplate::GetName()
{
	return m_strName;
}

long DecisionSupportTemplate::GetID()
{
	return m_nID;
}

CString DecisionSupportTemplate::GetMaterials()
{
	if(!m_bQueried)
	{
		LoadData();
	}
	return m_strMaterials;
}

CString DecisionSupportTemplate::GetGuidelines() 
{
	if(!m_bQueried)
	{
		LoadData();
	}
	return m_strGuidelines;
}

void DecisionSupportTemplate::LoadData()
{
	ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT ReferenceInformation, Citation,Developer,FundingSource,ReleaseDate,RevisionDate "
		"FROM DecisionRulesT  WHERE ID = {INT}", m_nID);

	if(!prs->eof)
	{
		m_strMaterials = AdoFldString(prs->GetFields(), "ReferenceInformation","");
		m_strGuidelines = AdoFldString(prs->GetFields(), "Citation","");
		// (s.dhole 2013-11-01 11:01) - PLID 56237 Added Extra filds to load
		m_strDeveloper= AdoFldString(prs->GetFields(), "Developer","");
		m_strFunding= AdoFldString(prs->GetFields(), "FundingSource","");
		variant_t varReleaseDate = prs->Fields->Item["ReleaseDate"]->Value;
		if(varReleaseDate.vt == VT_DATE) {
			m_dtRelease= VarDateTime(varReleaseDate );
		}
		variant_t varRevisionDate = prs->Fields->Item["RevisionDate"]->Value;
		if(varRevisionDate.vt == VT_DATE) {
			m_dtRevision = VarDateTime(varRevisionDate );
		}
	}

	m_bQueried = true;
}

bool DecisionSupportTemplate::SetName(CString &strName)
{
	try
	{
		if(ReturnsRecordsParam("SELECT Name FROM DecisionRulesT WHERE Name = {STRING} AND ID <> {INT}", strName, m_nID))
		{
			MsgBox("The name you have entered has already been used for another Clinical Decision Support Template.");
			return false;
		}

		ExecuteParamSql("UPDATE DecisionRulesT SET Name = {STRING} WHERE ID = {INT}", strName, m_nID);
		m_strName = strName;

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

bool DecisionSupportTemplate::SetMaterials(CString &strText)
{
	try
	{
		// (s.dhole 2013-11-01 11:01) - PLID 56237 Change filed
		ExecuteParamSql("UPDATE DecisionRulesT SET ReferenceInformation  = {STRING} WHERE ID = {INT}", strText, m_nID);
		m_strMaterials = strText;

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

bool DecisionSupportTemplate::SetGuidelines(CString &strText)
{
	try
	{
		// (s.dhole 2013-11-01 11:01) - PLID 56237 Change filed
		ExecuteParamSql("UPDATE DecisionRulesT SET Citation  = {STRING} WHERE ID = {INT}", strText, m_nID);
		m_strGuidelines = strText;

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

bool DecisionSupportTemplate::Delete()
{
	try
	{
		//TES 11/27/2013 - PLID 59848 - Check to see if there has been an intervention for this rule
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ID FROM DecisionRuleInterventionsT WHERE DecisionRuleID = {INT}", m_nID);
		if(rs->eof) {
			//TES 11/27/2013 - PLID 59848 - OK, we can delete it.
			if (IDYES == MsgBox(MB_YESNO, "Are you sure you want to delete this rule?")) {
				//keep going
			
				//delete the criteria first
				ExecuteParamSql("DELETE FROM DecisionRulesCriterionT WHERE RuleID = {INT}", m_nID);
				ExecuteParamSql("DELETE FROM DecisionRulesT WHERE ID = {INT}", m_nID);
				return true;
			}
		}
		else {
			//TES 11/27/2013 - PLID 59848 - We can't delete it, see if they want to mark it Inactive
			if(IDYES == MsgBox(MB_YESNO, "This rule can not be deleted, because interventions have been created for it. Would you like to mark it Inactive instead? "
				"Inactive rules will not create any new interventions.")) {
				ExecuteParamSql("UPDATE DecisionRulesT SET Inactive = 1 WHERE ID = {INT}", m_nID);
				return true;
			}
		}
		
	}NxCatchAll(__FUNCTION__)

	return false;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
CString DecisionSupportTemplate::GetDeveloper()
{
	if(!m_bQueried){
		LoadData();
	}
	return m_strDeveloper;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
CString DecisionSupportTemplate::GetFundingInfo()
{
	if(!m_bQueried){
		LoadData();
	}
	return m_strFunding;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
COleDateTime DecisionSupportTemplate::GetReleaseDate()
{
	if(!m_bQueried){
		LoadData();
	}
	return m_dtRelease;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
COleDateTime DecisionSupportTemplate::GetRevisionDate()
{
	if(!m_bQueried){
		LoadData();
	}
	return m_dtRevision;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
bool DecisionSupportTemplate::SetDeveloper(CString &strText)
{
	try
	{
		if (m_strDeveloper!=strText)
		{
			ExecuteParamSql("UPDATE DecisionRulesT SET Developer   = {STRING} WHERE ID = {INT}", strText, m_nID);
			m_strDeveloper = strText;
		}
		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
bool DecisionSupportTemplate::SetFundingInfo(CString &strText)
{
	try
	{
		if (m_strFunding!=strText)
		{
			ExecuteParamSql("UPDATE DecisionRulesT SET FundingSource   = {STRING} WHERE ID = {INT}", strText, m_nID);
			m_strFunding = strText;
		}
		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
bool DecisionSupportTemplate::SetReleaseDate(COleDateTime &Dt)
{
	try
	{
		_variant_t  varDate=g_cvarNull ;
		COleDateTime  dttemp;
		dttemp.ParseDateTime("01/01/1800");
		if (Dt.GetStatus() == COleDateTime::valid && Dt.GetStatus() != COleDateTime::null && Dt!=NULL   &&  Dt.m_dt > dttemp.m_dt){
			varDate =COleVariant(Dt);
		}
		if (m_dtRelease !=varDate )
		{
			ExecuteParamSql("UPDATE DecisionRulesT SET ReleaseDate ={VT_DATE} WHERE ID = {INT}", varDate, m_nID);
			m_dtRelease  = varDate ;
		}

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

// (s.dhole 2013-10-31 16:17) - PLID 56237
bool DecisionSupportTemplate::SetRevisionDate(COleDateTime &Dt)
{
	try
	{
		_variant_t  varDate=g_cvarNull ;
		COleDateTime  dttemp;
		dttemp.ParseDateTime("01/01/1800");
		if (Dt.GetStatus() == COleDateTime::valid && Dt.GetStatus() != COleDateTime::null && Dt!=NULL  &&  Dt.m_dt > dttemp.m_dt ){
			varDate =COleVariant(Dt);
		}
		if (m_dtRelease !=varDate )
		{
			ExecuteParamSql("UPDATE DecisionRulesT SET RevisionDate   = {VT_DATE} WHERE ID = {INT}", varDate, m_nID);
			m_dtRevision  = varDate ;
		}

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}


// (s.dhole 2013-10-31 16:17) - PLID 56237
long DecisionSupportTemplate::AddCompletionItem(long nID, BYTE nType)
{
	return -1;
}

IInterventionCriterionPtr DecisionSupportTemplate::AddCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	BYTE nNewType = VarByte(pRow->GetValue(aclcTypeID));		
	_variant_t varNewRecordID = pRow->GetValue(aclcRecordID);

	InterventionCriteriaOperator ico = GetDefaultOperator((DecisionRuleCriterionType)nNewType);

	//add to data
	long nNewID = NewNumber("DecisionRulesCriterionT", "ID");
	ExecuteParamSql("INSERT INTO DecisionRulesCriterionT (ID, RuleID, Type, RecordID, Operator, Value) "
		" VALUES({INT}, {INT}, {INT}, {VT_I4}, {INT}, '') "
		, nNewID, m_nID, nNewType, varNewRecordID, ico);


	IInterventionCriterionPtr pCriterion;
	pCriterion.reset(new DecisionSupportCriterion(nNewID, AsString(pRow->GetValue(aclcName)), AsString(pRow->GetValue(aclcType)), 
		(DecisionRuleCriterionType)nNewType, varNewRecordID, ico, "", "", (EmrInfoType)VarByte(pRow->GetValue(aclcEmrInfoType),-1)));
	return pCriterion;
}

void DecisionSupportTemplate::LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	pRow->PutValue(ilcID, m_nID);
	pRow->PutValue(ilcName, _bstr_t(m_strName));
	pRow->PutValue(ilcToDoMessage, _bstr_t(m_strToDoMessage));
}
