
#include "stdafx.h"
#include "WellnessManager.h"
#include "WellnessDataUtils.h"
#include "WellnessUtils.h"
#include "WellnessCriterion.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

using namespace Intervention;

//Wellness Manager

WellnessManager::WellnessManager()
{
	m_pSelectedTemplate.reset();
}

WellnessManager::~WellnessManager()
{

}

CString WellnessManager::GetDlgName()
{
	return "Patient Wellness Templates";
}

bool WellnessManager::ShowCompletionList()
{
	return true;
}

IInterventionTemplatePtr WellnessManager::GetNewTemplate()
{
	IInterventionTemplatePtr pTemplate;
	pTemplate.reset();

	//TES 5/22/2009 - PLID 34302 - give them a InputBox and have them input a new name for the datalist
	CString strNewName;
	if (IDOK == InputBoxLimited(NULL, FormatString("Please enter a name for the new Wellness Template"), strNewName, "",200,false,false,NULL)) {

		ADODB::_RecordsetPtr rs;
		//TES 6/3/2009 - PLID 34446 - Don't check deleted templates, or patient-specific ones (it shouldn't have been checking those all along).
		// (s.tullis 2015-05-19 14:14) - PLID 61879 - moved rs->Close() throwing null error
		if(ReturnsRecordsParam("SELECT Name FROM WellnessTemplateT WHERE Name = {STRING} AND Deleted = 0 AND SpecificToPatientID IS NULL", strNewName))
		{
			MessageBox(NULL, "There is already a Wellness Template with this name. Please choose another name.", "Practice", MB_OK | MB_ICONEXCLAMATION);
			return pTemplate;
		}
		
		//TES 5/22/2009 - PLID 34302 - Add to data
		rs = CreateParamRecordset("SET NOCOUNT OFF\r\n"
			"INSERT INTO WellnessTemplateT (Name, Guideline, Reference) VALUES({STRING}, '', '')\r\n"
			"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID", strNewName);
		_variant_t v;
		if (NULL == (rs = rs->NextRecordset(&v))) {
			AfxThrowNxException("Failed to get new ID for Wellness Template");
		}
		long nNewID = AdoFldLong(rs, "ID");

		rs->Close();

		pTemplate.reset(new WellnessTemplate(strNewName, nNewID));
	}

	return pTemplate;
}

void WellnessManager::SetSelectedTemplate(IInterventionTemplatePtr pTemplate)
{
	m_pSelectedTemplate = pTemplate;
}

bool WellnessManager::DeleteSelectedTemplate()
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

CString WellnessManager::GetTemplateListFromClause()
{
	return "(SELECT ID, Name, NULL AS ToDoMessage, OriginalWellnessTemplateID, Deleted "
		"FROM WellnessTemplateT) TemplateQ";
}

CString WellnessManager::GetTemplateListWhereClause()
{
	return "OriginalWellnessTemplateID Is Null AND Deleted = 0";
}

CString WellnessManager::GetCriteriaListFromClause()
{
	return
"(SELECT 'Age (Months)' AS Name, 'Demographics' AS Type, convert(tinyint,1) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION SELECT 'Age (Years)' AS Name, 'Demographics' AS Type, convert(tinyint,1) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION SELECT 'Gender' AS Name, 'Demographics' AS Type, convert(tinyint,2) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION SELECT EmrInfoT.Name, 'EMR Detail' AS Type, convert(tinyint,3) AS TypeID, EmrInfoMasterT.ID AS RecordID, EmrInfoT.DataType AS EmrInfoType "
"FROM EmrInfoMasterT "
"INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
"WHERE EmrInfoMasterT.Inactive = 0 AND EMRInfoT.DataSubType <> 3 "
"UNION SELECT 'Active Problems' AS Name, 'EMR Problem List' AS Type, convert(tinyint,4) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION SELECT Type AS Name, 'Immunization' AS Type, convert(tinyint,5) AS TypeID, ID AS RecordID, NULL AS EmrInfoType FROM ImmunizationsT "
"UNION SELECT 'Patient Is In' AS Name, 'Demographics' AS Type, convert(tinyint,6) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION SELECT 'Result Name' AS Name, 'Lab Result Name' AS Type, convert(tinyint,7) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType "
"UNION SELECT 'Result Value' AS Name, 'Lab Result Value' AS Type, convert(tinyint,8) AS TypeID, NULL AS RecordID, NULL AS EmrInfoType) AS AvailableCriteriaQ"
		;
}

CString WellnessManager::GetCriteriaListWhereClause()
{
	return "";
}
// (s.tullis 2015-05-19 13:55) - PLID 61879 - Added Another condition to the Value statement 
//<select > should not be shown if the Operator is icoFilledIn, icoNotFilledIn, icoExist, icoDoesNotExist)
CString WellnessManager::GetTemplateCriteriaFromClause()
{
	return 
"( "
	"SELECT "
		"WellnessTemplateCriterionT.ID, "
		"CASE "
			"WHEN WellnessTemplateCriterionT.Type = 1 "
			"THEN "
				"CASE "
					"WHEN  convert(int,WellnessTemplateCriterionT.Value)%12 = 0 "
					"AND convert(int,WellnessTemplateCriterionT.Value) >= 36 "
				"THEN "
					"'Age (Years)' ELSE 'Age (Months)' "
				"END "
			"WHEN WellnessTemplateCriterionT.Type = 2 "
			"THEN 'Gender' "
			"WHEN WellnessTemplateCriterionT.Type = 4 "
			"THEN 'Active Problems' "
			"WHEN WellnessTemplateCriterionT.Type = 3 "
			"THEN EmrInfoT.Name "
			"WHEN WellnessTemplateCriterionT.Type = 5 "
			"THEN ImmunizationsT.Type "
			"WHEN WellnessTemplateCriterionT.Type = 6 "
			"THEN 'Patient Is In' "
			"WHEN WellnessTemplateCriterionT.Type = 7 "
			"THEN 'Result Name' "
			"WHEN WellnessTemplateCriterionT.Type = 8 "
			"THEN 'Result Value' END AS Name, "
			"CASE WHEN WellnessTemplateCriterionT.Type IN (1,2,6) "
			"THEN 'Demographics' "
			"WHEN WellnessTemplateCriterionT.Type = 3 "
			"THEN 'EMR Detail' "
			"WHEN WellnessTemplateCriterionT.Type = 4 "
			"THEN 'EMR Problem List' "
			"WHEN WellnessTemplateCriterionT.Type = 5 "
			"THEN 'Immunization' "
			"WHEN WellnessTemplateCriterionT.Type = 7 "
			"THEN 'Lab Result Name' "
			"WHEN WellnessTemplateCriterionT.Type = 8 "
			"THEN 'Lab Result Value' "
		"END AS Type, "
		"WellnessTemplateCriterionT.Type AS TypeID, "
		"WellnessTemplateCriterionT.RecordID, "
		"WellnessTemplateCriterionT.Operator, "
		"WellnessTemplateCriterionT.Value, "
		"CASE "
			"WHEN WellnessTemplateCriterionT.Type = 1 "
			"THEN "
				"CASE "
					"WHEN IsNumeric(WellnessTemplateCriterionT.Value) = 1 "
					"THEN "
						"CASE "
							"WHEN WellnessTemplateCriterionT.Value%12 = 0 "
							"AND convert(int,WellnessTemplateCriterionT.Value) >= 36 "
							"THEN "
								"convert(nvarchar(3500),WellnessTemplateCriterionT.Value/12) "
							"ELSE WellnessTemplateCriterionT.Value "
						"END "
					"ELSE WellnessTemplateCriterionT.Value "
				"END "
			"WHEN WellnessTemplateCriterionT.Type = 3 "
			"AND EmrInfoT.DataType = 3 AND WellnessTemplateCriterionT.Operator < 9 "
			"THEN '<Loading...>' "
			"ELSE WellnessTemplateCriterionT.Value "
		"END AS DisplayValue, "
		"WellnessTemplateCriterionT.LastXDays, "
		"WellnessTemplateCriterionT.WellnessTemplateID, "
		"EmrInfoT.DataType AS EmrInfoType "
	"FROM WellnessTemplateCriterionT "
	"LEFT JOIN "
		"(EmrInfoMasterT "
		"INNER JOIN EmrInfoT "
		"ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID) "
	"ON WellnessTemplateCriterionT.RecordID = EmrInfoMasterT.ID "
	"LEFT JOIN ImmunizationsT "
	"ON WellnessTemplateCriterionT.RecordID = ImmunizationsT.ID "
") AS WellnessTemplateCriteriaQ"
		;
}

CString WellnessManager::GetTemplateCriteriaWhereClause()
{
	return FormatString("WellnessTemplateID = %d", m_pSelectedTemplate->GetID());
}

IInterventionTemplatePtr WellnessManager::GetTemplatePtr(CString &strName, long nID)
{
	IInterventionTemplatePtr pTemplate(new WellnessTemplate(strName, nID));
	return pTemplate;
}

IInterventionTemplatePtr WellnessManager::GetSelectedTemplate()
{
	return m_pSelectedTemplate;
}

CString WellnessManager::GetCompletionListWhereClause()
{
	return FormatString("WellnessTemplateCompletionItemT.WellnessTemplateID = %li", m_pSelectedTemplate->GetID());
}

bool WellnessManager::RemoveCompletionItem(long nID)
{
	try
	{
		//TES 5/22/2009 - PLID 34302 - Remove from data
		ExecuteParamSql("DELETE FROM WellnessTemplateCompletionItemT WHERE ID = {INT}", nID);
		return true;
	} NxCatchAll(__FUNCTION__);

	return false;
}

inline long WellnessManager::GetNameConstraint()
{
	return 200;
}

IInterventionTemplatePtr WellnessManager::LoadTemplateFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	IInterventionTemplatePtr pTemplate;
	pTemplate.reset(new WellnessTemplate(VarString(pRow->GetValue(ilcName)), VarLong(pRow->GetValue(ilcID))));
	return pTemplate;
}

IInterventionCriterionPtr WellnessManager::LoadCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	IInterventionCriterionPtr pCriterion;
	pCriterion.reset(new WellnessCriterion(VarLong(pRow->GetValue(clcID)), -1, VarString(pRow->GetValue(clcName)), 
		VarString(pRow->GetValue(clcType)), (WellnessTemplateCriterionType)VarByte(pRow->GetValue(clcTypeID)), pRow->GetValue(clcRecordID), 
		(InterventionCriteriaOperator)VarByte(pRow->GetValue(clcOperator)), AsString(pRow->GetValue(clcValue)), AsString(pRow->GetValue(clcDisplayValue)), 
		AsLong(pRow->GetValue(clcLastXDays)), (EmrInfoType)VarByte(pRow->GetValue(clcEmrInfoType),-1)));
	return pCriterion;
}

bool WellnessManager::ShowLastXDays()
{
	return true;
}
