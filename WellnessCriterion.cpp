
#include "stdafx.h"
#include "WellnessCriterion.h"
#include "WellnessDataUtils.h"

// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS

using namespace Intervention;

WellnessCriterion::WellnessCriterion(long nID,	long nRuleID, CString strName, CString strType, WellnessTemplateCriterionType wtctTypeID,
									_variant_t varRecordID, InterventionCriteriaOperator icoOperator, CString strValue, CString strDisplayValue, 
									long nLastXDays, ::EmrInfoType eitEmrInfoType)
{
	m_nID = nID;
	m_nRuleID = nRuleID;
	m_strName = strName;
	m_strType = strType;
	m_wtctTypeID = wtctTypeID;
	m_varRecordID = varRecordID;
	m_icoOperator = icoOperator;
	m_strValue = strValue;
	m_strDisplayValue = strDisplayValue;
	m_nLastXDays = nLastXDays;
	m_eitEmrInfoType = eitEmrInfoType;

	RefreshCalculatedValues();
}

void WellnessCriterion::RefreshCalculatedValues()
{
	//TES 6/2/2009 - PLID 34302 - Don't say "Select" if they can't select.
	m_bValueIsList = ((m_wtctTypeID == wtctEmrItem && m_eitEmrInfoType == eitMultiList) 
		&& (m_icoOperator != icoFilledIn && m_icoOperator != icoNotFilledIn && m_icoOperator != icoExists && m_icoOperator != icoDoesNotExist));

	NXDATALIST2Lib::IFormatSettingsPtr m_pOperatorFormat = GetOperatorFormatSettings(m_wtctTypeID);
	NXDATALIST2Lib::IFormatSettingsPtr m_pValueFormat = GetValueFormatSettings(m_wtctTypeID, m_eitEmrInfoType, VarLong(m_varRecordID, -1), (WellnessTemplateCriteriaOperator)m_icoOperator);
}

long WellnessCriterion::GetID()
{
	return m_nID;
}

long WellnessCriterion::GetRuleID()
{
	return m_nRuleID;
}

CString WellnessCriterion::GetName()
{
	return m_strName;
}

CString WellnessCriterion::GetType()
{
	return m_strType;
}

BYTE WellnessCriterion::GetTypeID()
{
	return (BYTE)m_wtctTypeID;
}

_variant_t WellnessCriterion::GetRecordID()
{
	return m_varRecordID;
}

InterventionCriteriaOperator WellnessCriterion::GetOperator()
{
	return m_icoOperator;
}

void WellnessCriterion::PutOperator(InterventionCriteriaOperator value)
{
	m_icoOperator = value;
	m_bValueIsList = false;
	InterventionCriteriaOperator icoOld = m_icoOperator;
	CString strBatchSql = "";
	CNxParamSqlArray arryParams;

	//TES 5/26/2009 - PLID 34302 - Track whether we escape this value for a LIKE clause.
	bool bEscaped = false;
	if(m_icoOperator == icoFilledIn || 
		m_icoOperator == icoNotFilledIn || 
		m_icoOperator == icoExists || 
		m_icoOperator == icoDoesNotExist) 
	{
		//TES 5/22/2009 - PLID 34302 - The value is not used with this operator, so clear it out.
		if(!m_strValue.IsEmpty()) 
		{
			::AddParamStatementToSqlBatch(strBatchSql, arryParams, 
				"UPDATE WellnessTemplateCriterionT SET Value = '' WHERE ID = {INT} ",
				m_nID);

			m_strValue = "";
		}

		//TES 6/2/2009 - PLID 34302 - Always clear out the display value (sometimes it's non-blank 
		// even when the real value is blank).
		m_strDisplayValue = "";
	}
	else 
	{
		//TES 6/8/2009 - PLID 34509 - Make sure this is actually an EMR-Item-based criterion
		if(m_wtctTypeID == wtctEmrItem && 
			m_eitEmrInfoType == eitMultiList) 
		{
			if(m_strValue.IsEmpty()) {
				//TES 6/2/2009 - PLID 34302 - Restore the Select option
				m_strDisplayValue = "<Select...>";
				m_bValueIsList = true;
			}
		}
		if(m_icoOperator != icoEqual && 
			m_icoOperator != icoNotEqual) 
		{
			//TES 5/22/2009 - PLID 34302 - With any operator other than equal or not equal, they
			// can only have one selected item in the Value field, so truncate the list to one entry.
			m_strValue.TrimLeft();
			int nSpace = m_strValue.Find(" ");
			if(nSpace != -1) {
				CString strNewValue = m_strValue.Left(nSpace);
				::AddParamStatementToSqlBatch(strBatchSql, arryParams,
					"UPDATE WellnessTemplateCriterionT SET Value = {STRING} WHERE ID = {INT}",
					strNewValue, m_nID);

				m_strValue = strNewValue;
				m_strDisplayValue = GetTableField("EmrDataT INNER JOIN EmrInfoMasterT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID",
					"Data", "EmrDataGroupID", atol(strNewValue));
			}
		}
		else if(m_icoOperator == icoContains || 
			m_icoOperator == icoDoesNotContain) 
		{
			if(m_wtctTypeID == wtctEmrProblemList || 
				(m_wtctTypeID == wtctEmrItem && m_eitEmrInfoType == eitText)) 
			{

				//TES 5/26/2009 - PLID 34302 - With this operator/type combination, we want to
				// store the text escaped for a LIKE clause
				bEscaped = true;

				CString strNewValue = FormatForLikeClause(m_strValue);
				::AddParamStatementToSqlBatch(strBatchSql, arryParams,
					"UPDATE WellnessTemplateCriterionT SET Value = {STRING} WHERE ID = {INT}",
					strNewValue, m_nID);

				m_strValue = strNewValue;
				m_strDisplayValue = strNewValue;
			}
		}
	}
	
	if( (m_wtctTypeID == wtctEmrProblemList || (m_wtctTypeID == wtctEmrItem && m_eitEmrInfoType == eitText)) && 
		(icoOld == icoContains || icoOld == icoDoesNotContain) &&
		!bEscaped) 
	{
			//TES 5/26/2009 - PLID 34302 - This used to be escaped, but isn't any more.  Update the
			// database and the datalist.
			CString strOldValue = m_strValue;
			CString strNewValue = UnformatFromLikeClause(m_strValue);
			::AddParamStatementToSqlBatch(strBatchSql, arryParams,
				"UPDATE WellnessTemplateCriterionT SET Value = {STRING} WHERE ID = {INT}",
				strNewValue, m_nID);
			m_strValue = strNewValue;
	}
	// (s.tullis 2015-05-19 09:39) - PLID 61879 - Need to actually update the operator and execute the sql update statement
	::AddParamStatementToSqlBatch(strBatchSql, arryParams,
		"UPDATE WellnessTemplateCriterionT SET Operator = {INT} WHERE ID = {INT} ", m_icoOperator, m_nID);
	ExecuteParamSqlBatch(GetRemoteData(),strBatchSql, arryParams);
	RefreshCalculatedValues();
}

CString WellnessCriterion::GetValue()
{
	return m_strValue;
}

CString WellnessCriterion::GetDisplayValue()
{
	return m_strDisplayValue;
}

void WellnessCriterion::PutDisplayValue(CString &value)
{
	CString strValue;

	_variant_t varValue = _bstr_t(value);

	if( (m_icoOperator == icoContains || m_icoOperator == icoDoesNotContain) && 
		((m_wtctTypeID == wtctEmrItem && m_eitEmrInfoType == eitText) || m_wtctTypeID == wtctEmrProblemList)) 
	{
		varValue = _bstr_t(FormatForLikeClause(value));
	}

	//TES 6/12/2009 - PLID 34567 - If they are using the Age (Years) option, then we want
	// to copy the value in in months (i.e., the entered value * 12) to the "real" value field.							
	else if (m_wtctTypeID == wtctAge && m_strName == "Age (Years)") {
		if(!value.IsEmpty()) {
			long nNewValue = atol(value);
			if(nNewValue > 0) {
				varValue = nNewValue*12;
			}
		}
	}

	//TES 5/26/2009 - PLID 34302 - Single- and multi-select lists need a leading space
	if(m_wtctTypeID == wtctEmrItem && (m_eitEmrInfoType == eitSingleList || m_eitEmrInfoType == eitMultiList)) {
		strValue = " " + AsString(varValue);
	}
	else {
		strValue = AsString(varValue);
	}

	m_strValue = AsString(varValue);
	m_strDisplayValue = value;

	ExecuteParamSql("UPDATE WellnessTemplateCriterionT SET Value = {STRING} WHERE ID = {INT}", strValue, m_nID);
}

long WellnessCriterion::GetLastXDays()
{
	return m_nLastXDays;
}
// (s.tullis 2015-05-19 14:29) - PLID 61879 - Need to update Last days
void WellnessCriterion::PutLastXDays(long value)
{
	m_nLastXDays = value;

	ExecuteParamSql("UPDATE WellnessTemplateCriterionT SET LastXDays = {INT} WHERE ID = {INT}", m_nLastXDays, m_nID);
}

EmrInfoType WellnessCriterion::GetEmrInfoType()
{
	return m_eitEmrInfoType;
}

bool WellnessCriterion::IsValueAList()
{
	return m_bValueIsList;
}

NXDATALIST2Lib::IFormatSettingsPtr WellnessCriterion::GetOperatorFormatOverride()
{
	return m_pOperatorFormat;
}

NXDATALIST2Lib::IFormatSettingsPtr WellnessCriterion::GetValueFormatOverride()
{
	return m_pValueFormat;
}

bool WellnessCriterion::Delete()
{
	try
	{
		//TES 5/22/2009 - PLID 34302 - Remove from data
		//TES 6/2/2009 - PLID 34302 - Pre-qualifications as well
		ExecuteParamSql("DELETE FROM WellnessPatientQualificationT WHERE WellnessTemplateCriterionID = {INT}", m_nID);
		ExecuteParamSql("DELETE FROM WellnessTemplateCriterionT WHERE ID = {INT}", m_nID);

		return true;

	} NxCatchAll(__FUNCTION__);

	return false;
}

void WellnessCriterion::LoadIntoRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	pRow->PutValue(clcID, m_nID);
	pRow->PutValue(clcName, _bstr_t(m_strName));
	pRow->PutValue(clcType, _bstr_t(m_strType));
	pRow->PutValue(clcTypeID, (BYTE)m_wtctTypeID);
	pRow->PutValue(clcRecordID, m_varRecordID);
	pRow->PutValue(clcOperator, (BYTE)m_icoOperator);
	pRow->PutValue(clcValue, _bstr_t(m_strValue));
	pRow->PutValue(clcDisplayValue, _bstr_t(m_strDisplayValue));
	pRow->PutValue(clcLastXDays, m_nLastXDays);
	pRow->PutValue(clcEmrInfoType, (BYTE)m_eitEmrInfoType);

	//TES 5/22/2009 - PLID 34302 - Now, set the available operators according to type.
	pRow->PutRefCellFormatOverride(clcOperator, GetOperatorFormatSettings(m_wtctTypeID));
	//TES 5/22/2009 - PLID 34302 - Likewise, set up the Value field based on the type (and EMR Item, if any) of this row.
	pRow->PutRefCellFormatOverride(clcDisplayValue, GetValueFormatSettings(m_wtctTypeID, m_eitEmrInfoType, VarLong(m_varRecordID,-1), (WellnessTemplateCriteriaOperator)m_icoOperator));
}

bool WellnessCriterion::IsValid()
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

bool WellnessCriterion::ValidateValue(CString &strValue, VARIANT* pvarNewValue)
{
	switch(m_wtctTypeID) {
		case wtctAge:
			{
				//TES 5/22/2009 - PLID 34302 - Force a positive integer
				long nValue = atol(strValue);
				if(nValue <= 0) {
					*pvarNewValue = g_cvarNull;
				}
				// (b.cardillo 2009-06-11 18:01) - PLID 34609 - Eventually SQL will prevent super-extreme 
				// values, but we also protect from plain nonsense values.
				else if(nValue > 400) {
					MsgBox("You cannot enter a value in the 'Value' column greater than 400 years");
					return false;
				}
				else {
					*pvarNewValue = _variant_t(nValue);
				}
			}
			break;
		case wtctEmrItem:
			{
				switch(m_eitEmrInfoType) {
					case eitSlider:
						{
							//TES 5/22/2009 - PLID 34302 - Force a valid number
							double dValue = atof(strValue);
							*pvarNewValue = _variant_t(dValue);
						}
						break;
				}
			}
			break;
	}
	return true;
}

bool WellnessCriterion::IsValueEnabled()
{
	//TES 5/22/2009 - PLID 34302 - They can't set the value if the operator is Filled In or Not Filled In
	if(m_icoOperator == icoFilledIn || m_icoOperator == icoNotFilledIn || 
		m_icoOperator == icoExists || m_icoOperator == icoDoesNotExist) {
		return false;
	}
	return true;
}

bool WellnessCriterion::IsLastXDaysEnabled()
{
	//TES 5/22/2009 - PLID 34302 - They can't set the Last X Days if the criteria is Age, Gender, or Problem List
	if(m_wtctTypeID == wtctAge || m_wtctTypeID == wtctGender || m_wtctTypeID == wtctEmrProblemList) {
		return false;
	}
	return true;
}

void WellnessCriterion::OnClickValue()
{
	//TES 6/8/2009 - PLID 34509 - Make sure this is actually an EMR-Item-based criterion
	if(m_wtctTypeID == wtctEmrItem && m_eitEmrInfoType == eitMultiList) 
	{
		//TES 5/22/2009 - PLID 34302 - We're going to pop up a dialog.
		
		CString strValue = m_strValue;

		//TES 5/26/2009 - PLID 34302 - Trim the leading space
		strValue.TrimLeft(" ");

		//TES 5/22/2009 - PLID 34302 - Load the pre-selected values.
		CArray<long,long> arDataGroupIDs;
		int nSpace = strValue.Find(" ");
		while(nSpace != -1) {
			arDataGroupIDs.Add(atol(strValue.Left(nSpace)));
			strValue = strValue.Mid(nSpace+1);
			nSpace = strValue.Find(" ");
		}
		arDataGroupIDs.Add(atol(strValue));

		//TES 5/22/2009 - PLID 34302 - Prepare the dialog, force only one selection unless the operator
		// is Equal or Not Equal
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(NULL, "EmrDataT");
		dlg.PreSelect(arDataGroupIDs);
		unsigned long nMaxSelections = 1;
		CString strValueLabel = "value";
		if(m_icoOperator == wtcoEqual || m_icoOperator == wtcoNotEqual) {
			nMaxSelections = 0xFFFFFFFF;
			strValueLabel = "values";
		}
		if(IDOK == dlg.Open("EmrDataT", 
			// (j.gruber 2009-07-02 16:27) - PLID 34350 - take out inactives and labels
			// (j.jones 2009-07-15 17:56) - PLID 34916 - ensure we filter on list items only, incase any table columns exist
			FormatString("EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0 "
				"AND EMRDataT.ListType = 1 "
				"AND EmrInfoID = (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID = %li)", VarLong(m_varRecordID, -1)),
			"EmrDataGroupID", "Data", 
			"Please select the " + strValueLabel + " you wish this criteria to use for this Wellness Template.",
			0, nMaxSelections)) {
			//TES 5/22/2009 - PLID 34302 - OK, they've selected some new values, put them in the data.
			dlg.FillArrayWithIDs(arDataGroupIDs);
			//TES 5/26/2009 - PLID 34302 - Start it off with a space
			CString strNewValue = " ";
			for(int i = 0; i < arDataGroupIDs.GetSize(); i++) {
				strNewValue += AsString(arDataGroupIDs[i]) + " ";
			}
			strNewValue.TrimRight(" ");

			ExecuteParamSql("UPDATE WellnessTemplateCriterionT SET Value = {STRING} WHERE ID = {INT}",
				strNewValue, m_nID);

			//TES 6/1/2009 - PLID 34302 - Update pre-qualifications
			UpdatePatientWellnessQualification_TemplateCriteria(GetRemoteData(), m_nID);
			//TES 5/22/2009 - PLID 34302 - Update the "real" value field.
			m_strValue = strNewValue;

			//TES 5/22/2009 - PLID 34302 - Now update the display value field with the names of the
			// selected items.
			CVariantArray vaNewNames;
			dlg.FillArrayWithNames(vaNewNames);
			CString strNewNames;
			for(i = 0; i < vaNewNames.GetSize(); i++) {
				strNewNames += AsString(vaNewNames[i]) + "; ";
			}
			if(strNewNames.IsEmpty()) {
				m_strDisplayValue = "<Select...>";
			}
			else {
				strNewNames = strNewNames.Left(strNewNames.GetLength() - 2);
				m_strDisplayValue = strNewNames;
			}
		}
	}
}