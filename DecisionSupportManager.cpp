
#include "stdafx.h"
#include "DecisionSupportManager.h"
#include "DecisionSupportCriterion.h"
#include "AuditTrail.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

using namespace Intervention;

//DecisionSupport Manager

DecisionSupportManager::DecisionSupportManager()
{
	m_pSelectedTemplate.reset();
}

DecisionSupportManager::~DecisionSupportManager()
{

}

CString DecisionSupportManager::GetDlgName()
{
	return "Configure Clinical Decision Support Rules";
}

bool DecisionSupportManager::ShowCompletionList()
{
	return false;
}

IInterventionTemplatePtr DecisionSupportManager::GetNewTemplate()
{
	IInterventionTemplatePtr pTemplate;
	pTemplate.reset();

	CString strNewName;
	if (IDOK == InputBoxLimited(NULL, FormatString("Please enter a name for the new Clinical Decision Support Rule"), strNewName, "",255,false,false,NULL)) {

		ADODB::_RecordsetPtr rs;		
		rs = CreateParamRecordset("SELECT Name FROM DecisionRulesT WHERE Name = {STRING}", strNewName);
		if(!rs->eof)
		{
			//TES 11/27/2013 - PLID 59848 - Rules can be inactive now
			MessageBox(NULL, "There is already a Clinical Decision Support Rule with this name (it may be Inactive). Please choose another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
			rs->Close();
			return pTemplate;
		}
		rs->Close();
		
		
		long nNewID = NewNumber("DecisionRulesT", "ID");
		ExecuteParamSql("INSERT INTO DecisionRulesT (ID, Name, ToDoMessage) VALUES({INT}, {STRING}, '')",
			nNewID, strNewName);

		//TES 11/14/2013 - PLID 59491 - CDS Rule auditing (the test script only requires auditing creation, so for now that's all we audit
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiDecisionRuleCreated, nNewID, "", strNewName, aepMedium, aetCreated);
			
		pTemplate.reset(new DecisionSupportTemplate(strNewName, nNewID, CString("")));
	}

	return pTemplate;
}

void DecisionSupportManager::SetSelectedTemplate(IInterventionTemplatePtr pTemplate)
{
	m_pSelectedTemplate = pTemplate;
}

bool DecisionSupportManager::DeleteSelectedTemplate()
{
	if(m_pSelectedTemplate != NULL)
	{
		if(m_pSelectedTemplate->Delete())
		{
			m_pSelectedTemplate.reset();
			return true;
		}
	}
	return false;
}

CString DecisionSupportManager::GetTemplateListFromClause()
{
	//TES 11/27/2013 - PLID 59848 - Filter out Inactive rules
	return "(SELECT ID, Name, ToDoMessage FROM DecisionRulesT WHERE Inactive = 0) TemplateListQ";
}

CString DecisionSupportManager::GetTemplateListWhereClause()
{
	return "";
}

CString DecisionSupportManager::GetCriteriaListFromClause()
{
	//TES 11/15/2013 - PLID 59532 - Changed to only show EMR Items that are Text, List, or Slider types
	return
"(SELECT Name, TypeID, Type, RecordID, EmrInfoType "
"FROM "
"(SELECT 'Age (Years)' AS Name, 'Demographics' AS Type, convert(tinyint,1) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT 'Result Name' AS Name, 'Lab Result Name' AS Type, convert(tinyint,2) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT 'Result Value' AS Name, 'Lab Result Value' AS Type, convert(tinyint,3) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT 'Patient' AS Name, 'Demographics' AS Type, convert(tinyint,4) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT 'Problem Name' AS Name, 'Problem List' AS Type, convert(tinyint,5) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT 'Medication Name' AS Name, 'Medication List' AS Type, convert(tinyint,6) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT 'Allergy Name' AS Name, 'Allergy List' AS Type, convert(tinyint,7) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION "
"SELECT EmrInfoT.Name, 'EMR Detail' AS Type, convert(tinyint,8) AS TypeID, EmrInfoMasterT.ID AS RecordID, EmrInfoT.DataType AS EmrInfoType "
"FROM EmrInfoMasterT "
"INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
"WHERE EmrInfoMasterT.Inactive = 0 AND EMRInfoT.DataSubType <> 3 AND EmrInfoT.DataType IN (1,2,3,5) "
") InnerQ) AvailableCriteriaQ";
}

CString DecisionSupportManager::GetCriteriaListWhereClause()
{
	return "";
}

CString DecisionSupportManager::GetTemplateCriteriaFromClause()
{
	//TES 11/14/2013 - PLID 59506 - This was not loading EmrInfoT.DataType for some reason, I added it back in
	//TES 11/21/2013 - PLID 59506 - Fixed "Alergy" to "Allergy"
	//TES 12/2/2013 - PLID 59532 - Changed multi-select lists to work like single-select lists, there's no more hyperlink or ability to select multiple
	return 
"(SELECT DecisionRulesCriterionT.ID, DecisionRulesCriterionT.RuleID, "
"CASE WHEN DecisionRulesCriterionT.Type = 1 THEN 'Age (Years)' "
"WHEN DecisionRulesCriterionT.Type = 2 THEN 'Lab Result Name' "
"WHEN DecisionRulesCriterionT.Type = 3 THEN 'Result Value' "
"WHEN DecisionRulesCriterionT.Type = 4 THEN 'Patient' "
"WHEN DecisionRulesCriterionT.Type = 5 THEN 'Problem Name' "
"WHEN DecisionRulesCriterionT.Type = 6 THEN 'Medication Name' "
"WHEN DecisionRulesCriterionT.Type = 7 THEN 'Allergy Name' "
"WHEN DecisionRulesCriterionT.Type = 8 THEN EmrInfoT.Name "
"END AS Name, "
"CASE WHEN DecisionRulesCriterionT.Type = 1 THEN 'Demographics' "
"WHEN DecisionRulesCriterionT.Type = 2 THEN 'Lab Result Name' "
"WHEN DecisionRulesCriterionT.Type = 3 THEN 'Lab Result' "
"WHEN DecisionRulesCriterionT.Type = 4 THEN 'Demographics' "
"WHEN DecisionRulesCriterionT.Type = 5 THEN 'Problem List' "
"WHEN DecisionRulesCriterionT.Type = 6 THEN 'Medication List' "
"WHEN DecisionRulesCriterionT.Type = 7 THEN 'Allergy List' "
"WHEN DecisionRulesCriterionT.Type = 8 THEN 'EMR Detail' "
"END AS Type, "
"CONVERT(TINYINT, DecisionRulesCriterionT.Type) AS TypeID, DecisionRulesCriterionT.RecordID, "
"CONVERT(tinyint, DecisionRulesCriterionT.Operator) as Operator, DecisionRulesCriterionT.Value, CASE WHEN "
"DecisionRulesCriterionT.Type = 1 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 2 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 3 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 4 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 5 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 6 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 7 THEN DecisionRulesCriterionT.Value "
"WHEN DecisionRulesCriterionT.Type = 8 THEN DecisionRulesCriterionT.Value "
"END AS DisplayValue, "
"NULL AS LastXDays, "
"EmrInfoT.DataType AS EmrInfoType "
"FROM DecisionRulesCriterionT "
	"LEFT JOIN "
		"(EmrInfoMasterT "
		"INNER JOIN EmrInfoT "
		"ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID) "
	"ON DecisionRulesCriterionT.RecordID = EmrInfoMasterT.ID "
") DecisionRuleCriteriaQ"
		;
}

CString DecisionSupportManager::GetTemplateCriteriaWhereClause()
{
	return FormatString("DecisionRuleCriteriaQ.RuleID = %d", m_pSelectedTemplate->GetID());
}

IInterventionTemplatePtr DecisionSupportManager::GetTemplatePtr(CString &strName, long nID)
{
	IInterventionTemplatePtr pTemplate(new DecisionSupportTemplate(strName, nID, CString("")));
	return pTemplate;
}

IInterventionTemplatePtr DecisionSupportManager::GetSelectedTemplate()
{
	return m_pSelectedTemplate;
}

CString DecisionSupportManager::GetCompletionListWhereClause()
{
	return "";
}

bool DecisionSupportManager::RemoveCompletionItem(long nID)
{
	return true;
}

inline long DecisionSupportManager::GetNameConstraint()
{
	return 255;
}

IInterventionTemplatePtr DecisionSupportManager::LoadTemplateFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	IInterventionTemplatePtr pTemplate;
	pTemplate.reset(new DecisionSupportTemplate(VarString(pRow->GetValue(ilcName)), VarLong(pRow->GetValue(ilcID)), CString("")));
	return pTemplate;
}

IInterventionCriterionPtr DecisionSupportManager::LoadCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	IInterventionCriterionPtr pCriterion;
	pCriterion.reset(new DecisionSupportCriterion(VarLong(pRow->GetValue(clcID)), VarString(pRow->GetValue(clcName)), 
		VarString(pRow->GetValue(clcType)), (DecisionRuleCriterionType)VarByte(pRow->GetValue(clcTypeID)), pRow->GetValue(clcRecordID), 
		(InterventionCriteriaOperator)VarByte(pRow->GetValue(clcOperator)), AsString(pRow->GetValue(clcValue)), AsString(pRow->GetValue(clcDisplayValue)), 
		(EmrInfoType)VarByte(pRow->GetValue(clcEmrInfoType),-1)));
	return pCriterion;
}

bool DecisionSupportManager::ShowLastXDays()
{
	return false;
}