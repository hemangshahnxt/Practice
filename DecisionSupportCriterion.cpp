
#include "stdafx.h"
#include "DecisionSupportCriterion.h"
#include "DecisionRuleUtils.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

using namespace Intervention;

DecisionSupportCriterion::DecisionSupportCriterion(long nID, CString strName, CString strType, DecisionRuleCriterionType drctTypeID,
			_variant_t varRecordID, InterventionCriteriaOperator icoOperator, CString strValue, CString strDisplayValue,
			::EmrInfoType eitEmrInfoType)
{
	m_nID = nID;
	m_strName = strName;
	m_strType = strType;
	m_drctTypeID = drctTypeID;
	m_varRecordID = varRecordID;
	m_icoOperator = icoOperator;
	m_strValue = strValue;
	m_strDisplayValue = strDisplayValue;
	m_eitEmrInfoType = eitEmrInfoType;

	RefreshCalculatedValues();
}

void DecisionSupportCriterion::RefreshCalculatedValues()
{
	m_bValueIsList = ((m_drctTypeID == drctEmrItem && m_eitEmrInfoType == eitMultiList) 
		&& (m_icoOperator != icoFilledIn && m_icoOperator != icoNotFilledIn && m_icoOperator != icoExists && m_icoOperator != icoDoesNotExist));

	//TES 12/2/2013 - PLID 59532 - Pass in our EmrInfoType
	NXDATALIST2Lib::IFormatSettingsPtr m_pOperatorFormat = GetOperatorFormatSettings(m_drctTypeID, m_eitEmrInfoType);
	NXDATALIST2Lib::IFormatSettingsPtr m_pValueFormat = GetValueFormatSettings(m_drctTypeID, m_eitEmrInfoType, VarLong(m_varRecordID, -1), m_icoOperator);
}

long DecisionSupportCriterion::GetID()
{
	return m_nID;
}

CString DecisionSupportCriterion::GetName()
{
	return m_strName;
}

CString DecisionSupportCriterion::GetType()
{
	return m_strType;
}

BYTE DecisionSupportCriterion::GetTypeID()
{
	return (BYTE)m_drctTypeID;
}

_variant_t DecisionSupportCriterion::GetRecordID()
{
	return m_varRecordID;
}

InterventionCriteriaOperator DecisionSupportCriterion::GetOperator()
{
	return m_icoOperator;
}

void DecisionSupportCriterion::PutOperator(InterventionCriteriaOperator value)
{
	CNxParamSqlArray sqlParams;
	CString strSqlBatch = "";
	CString strCurrentValue = m_strValue;
	strCurrentValue.TrimLeft();

	//Update our copy of the opperator
	m_icoOperator = value;

	//Refresh anthing that might change from this
	RefreshCalculatedValues();

	m_strValue = "";

	if(m_bValueIsList)
	{
		//Its a list so we can have multiple selected
		m_strDisplayValue = "<Select>";
	}
	else
	{
		m_strDisplayValue = "";
	}

		////Its not a list, just use the first item in the list
		//int nSpace = strCurrentValue.Find(" ");
		//if(nSpace != -1) {
		//	CString strNewValue = strCurrentValue.Left(nSpace);
		//	AddParamStatementToSqlBatch(strSqlBatch, sqlParams, "UPDATE DecisionRulesCriterionT SET Value = {STRING} WHERE ID = {INT}",
		//		strNewValue, m_nID);

		//	m_strValue = strNewValue;
		//	m_strDisplayValue = GetTableField("LabsToBeOrderedT", "Description", "ID", atol(strNewValue));
		//}
	//}

	AddParamStatementToSqlBatch(strSqlBatch, sqlParams, "UPDATE DecisionRulesCriterionT SET Operator = {INT}, Value = '' WHERE ID = {INT}", (BYTE)value, m_nID);

	ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, sqlParams);
}

CString DecisionSupportCriterion::GetValue()
{
	return m_strValue;
}

CString DecisionSupportCriterion::GetDisplayValue()
{
	return m_strDisplayValue;
}

void DecisionSupportCriterion::PutDisplayValue(CString &value)
{
	CString strValue;

	_variant_t varValue = _bstr_t(value);

	if( (m_icoOperator == icoContains || m_icoOperator == icoDoesNotContain) && 
		(m_drctTypeID == drctEmrItem && m_eitEmrInfoType == eitText)) 
	{
		varValue = _bstr_t(FormatForLikeClause(value));
	}

	if(m_drctTypeID == drctEmrItem && (m_eitEmrInfoType == eitSingleList || m_eitEmrInfoType == eitMultiList)) {
		strValue = " " + AsString(varValue);
	}
	else {
		strValue = AsString(varValue);
	}

	m_strValue = AsString(varValue);
	m_strDisplayValue = value;

	ExecuteParamSql("UPDATE DecisionRulesCriterionT SET Value = {STRING} WHERE ID = {INT}", value, m_nID);
}

long DecisionSupportCriterion::GetLastXDays()
{
	return -1;
}

void DecisionSupportCriterion::PutLastXDays(long value)
{
	//Do nothing
}

EmrInfoType DecisionSupportCriterion::GetEmrInfoType()
{
	return m_eitEmrInfoType;
}

bool DecisionSupportCriterion::IsValueAList()
{
	return m_bValueIsList;
}

NXDATALIST2Lib::IFormatSettingsPtr DecisionSupportCriterion::GetOperatorFormatOverride()
{
	return m_pOperatorFormat;
}

NXDATALIST2Lib::IFormatSettingsPtr DecisionSupportCriterion::GetValueFormatOverride()
{
	return m_pValueFormat;
}

bool DecisionSupportCriterion::Delete()
{
	try
	{
		ExecuteParamSql("DELETE FROM DecisionRulesCriterionT WHERE ID = {INT}", m_nID);

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

void DecisionSupportCriterion::LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	pRow->PutValue(clcID, m_nID);
	pRow->PutValue(clcName, _bstr_t(m_strName));
	pRow->PutValue(clcType, _bstr_t(m_strType));
	pRow->PutValue(clcTypeID, (BYTE)m_drctTypeID);
	pRow->PutValue(clcRecordID, m_varRecordID);
	pRow->PutValue(clcOperator, (BYTE)m_icoOperator);
	pRow->PutValue(clcValue, _bstr_t(m_strValue));
	pRow->PutValue(clcDisplayValue, _bstr_t(m_strDisplayValue));
	pRow->PutValue(clcLastXDays, g_cvarNull);
	pRow->PutValue(clcEmrInfoType, (BYTE)m_eitEmrInfoType);

	//TES 5/22/2009 - PLID 34302 - Now, set the available operators according to type.
	//TES 12/2/2013 - PLID 59532 - Pass in our EmrInfoType
	pRow->PutRefCellFormatOverride(clcOperator, GetOperatorFormatSettings(m_drctTypeID, m_eitEmrInfoType));
	//TES 5/22/2009 - PLID 34302 - Likewise, set up the Value field based on the type (and EMR Item, if any) of this row.
	pRow->PutRefCellFormatOverride(clcDisplayValue, GetValueFormatSettings(m_drctTypeID, m_eitEmrInfoType, VarLong(m_varRecordID,-1), m_icoOperator));
}

bool DecisionSupportCriterion::IsValid()
{
	if(m_strValue.IsEmpty() && m_icoOperator != icoFilledIn 
		&& m_icoOperator != icoNotFilledIn && m_icoOperator != icoExists 
		&& m_icoOperator != icoDoesNotExist) 
	{
		MsgBox("The criterion for %s does not have a value entered.  Please enter a value for this criterion to be compared to.", 
			m_strName);

		return false;
	}

	return true;
}

bool DecisionSupportCriterion::ValidateValue(CString &strValue, VARIANT* pvarNewValue)
{
	return true;
}

bool DecisionSupportCriterion::IsValueEnabled()
{
	//TES 5/22/2009 - PLID 34302 - They can't set the value if the operator is Filled In or Not Filled In
	if(m_icoOperator == icoFilledIn || m_icoOperator == icoNotFilledIn || 
		m_icoOperator == icoExists || m_icoOperator == icoDoesNotExist) {
		return false;
	}
	return true;
}

bool DecisionSupportCriterion::IsLastXDaysEnabled()
{
	return false;
}

void DecisionSupportCriterion::OnClickValue()
{
	//Do nothing
}