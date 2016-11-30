
#include "stdafx.h"
#include "WellnessTemplate.h"
#include "WellnessDataUtils.h"
#include "WellnessUtils.h"
#include "WellnessCriterion.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

using namespace Intervention;

//Wellness Template

WellnessTemplate::WellnessTemplate(CString &strName, long nID)
{
	m_strName = strName;
	m_nID = nID;
	m_bQueried = false;
}

CString WellnessTemplate::GetName()
{
	return m_strName;
}

long WellnessTemplate::GetID()
{
	return m_nID;
}

CString WellnessTemplate::GetMaterials()
{
	if(!m_bQueried)
	{
		LoadData();
	}
	return m_strMaterials;
}

CString WellnessTemplate::GetGuidelines() 
{
	if(!m_bQueried)
	{
		LoadData();
	}
	return m_strGuidelines;
}

void WellnessTemplate::LoadData()
{
	ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT Reference, Guideline "
		"FROM WellnessTemplateT WHERE ID = {INT}", m_nID);

	if(!prs->eof)
	{
		m_strMaterials = AdoFldString(prs->GetFields(), "Reference");
		m_strGuidelines = AdoFldString(prs->GetFields(), "Guideline");
	}

	m_bQueried = true;
}

bool WellnessTemplate::SetName(CString &strName)
{
	try
	{
		//TES 6/3/2009 - PLID 34446 - Don't check deleted templates, or patient-specific ones (it shouldn't have been checking those all along).
		ADODB::_RecordsetPtr rsDup = CreateParamRecordset(
			"SELECT Name "
			"FROM WellnessTemplateT "
			"WHERE Name = {STRING} "
			"AND Deleted = 0 "
			"AND ID <> {INT} "
			"AND Deleted = 0 "
			"AND SpecificToPatientID Is Null ",
			strName, m_nID);
		if(!rsDup->eof) {
			MsgBox("The name you have entered has already been used for another Wellness Template.");
			return false;
		}
		//TES 5/22/2009 - PLID 34302 - Update the data
		ExecuteParamSql("UPDATE WellnessTemplateT SET Name = {STRING} WHERE ID = {INT}", strName, m_nID);
		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

bool WellnessTemplate::SetMaterials(CString &strText)
{
	try
	{
		//TES 5/22/2009 - PLID 34302 - Put the new text in data.
		ExecuteParamSql("UPDATE WellnessTemplateT SET Reference = {STRING} WHERE ID = {INT}", strText, m_nID);
		m_strMaterials = strText;

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

bool WellnessTemplate::SetGuidelines(CString &strText)
{
	try
	{
		//TES 5/22/2009 - PLID 34302 - Put the new text in data.
		ExecuteParamSql("UPDATE WellnessTemplateT SET Guideline = {STRING} WHERE ID = {INT}", strText, m_nID);
		m_strGuidelines = strText;

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

bool WellnessTemplate::Delete()
{
	try
	{
		// (j.jones 2009-07-16 10:26) - PLID 34917 - calculate how many patient-specific templates there are for this template
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountPatient FROM WellnessTemplateT WHERE Deleted = 0 AND OriginalWellnessTemplateID = {INT}", m_nID);
		long nCountPatientSpecific = 0;
		if(!rs->eof) {
			nCountPatientSpecific = AdoFldLong(rs, "CountPatient", 0);
		}
		rs->Close();

		CString strWarn, strPatient;

		if(nCountPatientSpecific == 1) {
			strPatient = "In addition, there is 1 patient-specific template created from this template that will also be deleted. "
			"Existing Wellness Alerts for the patient will be unaffected.\n\n";
		}
		else if(nCountPatientSpecific > 0) {
			strPatient.Format("In addition, there are %li patient-specific templates created from this template that will also be deleted. "
				"Existing Wellness Alerts for these patients will be unaffected.\n\n", nCountPatientSpecific);
		}

		strWarn.Format("This action will PERMANENTLY delete this template.  You will no longer receive any "
			"notifications related to %s, though any previously viewed notifications will remain.  This action cannot be undone.\n\n"
			"%s"
			"Are you sure you wish to continue?", m_strName, strPatient);

		if(IDYES != MsgBox(MB_YESNO, strWarn)) {
			return false;
		}
		
		//TES 6/2/2009 - PLID 34446 - First, clear out all pre-qualifications, then clear out the sub-componenents,
		// because nothing else references them anyway.  Finally, set the Deleted flag on this template.
		// (j.jones 2009-07-16 10:26) - PLID 34917 - flag patient-specific children as deleted
		ExecuteParamSql("DELETE FROM WellnessPatientQualificationT WHERE WellnessTemplateCriterionID IN "
			"(SELECT ID FROM WellnessTemplateCriterionT "
			"WHERE WellnessTemplateID = {INT} "
				"OR WellnessTemplateID IN (SELECT ID FROM WellnessTemplateT WHERE OriginalWellnessTemplateID = {INT})) "
			"DELETE FROM WellnessTemplateCriterionT "
			"WHERE WellnessTemplateID = {INT} "
				"OR WellnessTemplateID IN (SELECT ID FROM WellnessTemplateT WHERE OriginalWellnessTemplateID = {INT}) "
			"DELETE FROM WellnessTemplateCompletionItemT "
			"WHERE WellnessTemplateID = {INT} "
				"OR WellnessTemplateID IN (SELECT ID FROM WellnessTemplateT WHERE OriginalWellnessTemplateID = {INT}) "
			"UPDATE WellnessTemplateT SET Deleted = 1 WHERE ID = {INT} OR OriginalWellnessTemplateID = {INT}",
			m_nID, m_nID, m_nID, m_nID, m_nID, m_nID, m_nID, m_nID);

		return true;
	}NxCatchAll(__FUNCTION__)

	return false;
}

long WellnessTemplate::AddCompletionItem(long nID, BYTE nType)
{
	try
	{
		//TES 5/22/2009 - PLID 34302 - Add to data
		//TES 6/8/2009 - PLID 34504 - Added RecordType
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT OFF\r\n"
			"INSERT INTO WellnessTemplateCompletionItemT (WellnessTemplateID, RecordID, RecordType) VALUES({INT}, {INT}, {INT})\r\n"
			"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID", m_nID, nID, nType);
		_variant_t v;
		if (NULL == (rs = rs->NextRecordset(&v))) {
			AfxThrowNxException("Failed to get new ID for Completion Item");
		}

		return AdoFldLong(rs, "ID");

	}NxCatchAll(__FUNCTION__);

	return -1;
}

IInterventionCriterionPtr WellnessTemplate::AddCriterionFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	BYTE nNewType = VarByte(pRow->GetValue(aclcTypeID));
	//TES 6/8/2009 - PLID 34509 - Renamed EmrInfoMasterID to RecordID
	_variant_t varNewRecordID = pRow->GetValue(aclcRecordID);
	//TES 5/22/2009 - PLID 34302 - Set the operator to the default for this type.
	InterventionCriteriaOperator ico = (InterventionCriteriaOperator)GetDefaultOperator((WellnessTemplateCriterionType)nNewType);

	//TES 5/22/2009 - PLID 34302 - Add to data
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT OFF\r\n"
	"INSERT INTO WellnessTemplateCriterionT (WellnessTemplateID, Type, RecordID, Operator) VALUES({INT}, {INT}, {VT_I4}, {INT})\r\n"
		"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID", m_nID, nNewType, varNewRecordID, ico);
	_variant_t v;
	if (NULL == (rs = rs->NextRecordset(&v))) {
		AfxThrowNxException("Failed to get new ID for Wellness Template Criterion");
	}
	long nNewID = AdoFldLong(rs, "ID");

	IInterventionCriterionPtr pCriterion;

	UpdatePatientWellnessQualification_TemplateCriteria(GetRemoteData(), nNewID);
	
	pCriterion.reset(new WellnessCriterion(nNewID, -1, VarString(pRow->GetValue(aclcName), ""),
		VarString(pRow->GetValue(aclcType)), (WellnessTemplateCriterionType)nNewType,
		varNewRecordID, ico, "", "", -1, (EmrInfoType)VarByte(pRow->GetValue(aclcEmrInfoType),-1)));

	return pCriterion;
	//TODO:
	/*
	EmrInfoType eit = (EmrInfoType)VarByte(pRow->GetValue(aclcEmrInfoType),-1);
	//TES 6/2/2009 - PLID 34302 - Don't say "Select" if they can't select.
	if(((WellnessTemplateCriterionType)nNewType == wtctEmrItem && eit == eitMultiList) && (wtco != wtcoFilledIn && wtco != wtcoNotFilledIn && wtco != wtcoExists && wtco != wtcoDoesNotExist)) {
		//TES 5/22/2009 - PLID 34302 - Let them know they need to click on this row to change it.
		pRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
	}
	*/
}

// (s.dhole 2013-10-31 16:17) - PLID 

void WellnessTemplate::LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	pRow->PutValue(ilcID, m_nID);
	pRow->PutValue(ilcName, _bstr_t(m_strName));
}

CString WellnessTemplate::GetDeveloper()
{
	return ""; 
}

// (s.dhole 2013-10-31 16:17) - PLID 

CString WellnessTemplate::GetFundingInfo()
{
	return ""; 
}

// (s.dhole 2013-10-31 16:17) - PLID 

COleDateTime WellnessTemplate::GetReleaseDate()
{
	COleDateTime dt;
	dt.SetStatus(COleDateTime::invalid);
	return dt; 
}
// (s.dhole 2013-10-31 16:17) - PLID 

COleDateTime WellnessTemplate::GetRevisionDate()
{
	COleDateTime dt;
	dt.SetStatus(COleDateTime::invalid);
	return dt; 
}


// (s.dhole 2013-10-31 16:17) - PLID 
bool WellnessTemplate::SetDeveloper(CString &strText)
{
	return false;
}


// (s.dhole 2013-10-31 16:17) - PLID 
bool WellnessTemplate::SetFundingInfo(CString &strText)
{
	return false;
}

// (s.dhole 2013-10-31 16:17) - PLID 
bool WellnessTemplate::SetReleaseDate(COleDateTime &Dt)
{
	return false;
}

// (s.dhole 2013-10-31 16:17) - PLID 
bool WellnessTemplate::SetRevisionDate(COleDateTime &Dt)
{
	return false;
}
