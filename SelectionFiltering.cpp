// (r.gonet 10/23/2013) - PLID 56236 - Added. Comments to follow.

#include "stdafx.h"
#include "Groups.h"
#include "Filter.h"
#include "FilterDetail.h"
#include "SelectionFiltering.h"
using namespace ADODB;

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new empty CSelectedField
CSelectedField::CSelectedField()
{
	this->m_strInternalFieldName = "";
	this->m_strUnaffixedFieldName = "";
	this->m_nSuffixNumber = 0;
	this->m_nFilterFieldInfoID = -1;
	this->m_fboType = fboPerson;
	this->m_bIsKey = false;
	this->m_nSelectOrderIndex = 0;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSelectedField
CSelectedField::CSelectedField(CString strInternalFieldName, CString strUnaffixedFieldName, long nSuffixNumber, 
	FilterBasedOnEnum fboType, long nFilterFieldInfoID, bool bIsKey, long nSelectOrderIndex)
{
	this->m_strInternalFieldName = strInternalFieldName;
	this->m_strUnaffixedFieldName = strUnaffixedFieldName;
	this->m_nSuffixNumber = nSuffixNumber;
	this->m_nFilterFieldInfoID = nFilterFieldInfoID;
	this->m_fboType = fboType;
	this->m_bIsKey = bIsKey;
	this->m_nSelectOrderIndex = nSelectOrderIndex;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a copy of a CSelectedField
CSelectedField::CSelectedField(const CSelectedField &other)
{
	this->m_strInternalFieldName = other.m_strInternalFieldName;
	this->m_strUnaffixedFieldName = other.m_strUnaffixedFieldName;
	this->m_nSuffixNumber = other.m_nSuffixNumber;
	this->m_nFilterFieldInfoID = other.m_nFilterFieldInfoID;
	this->m_fboType = other.m_fboType;
	this->m_bIsKey = other.m_bIsKey;
	this->m_nSelectOrderIndex = other.m_nSelectOrderIndex;
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets the internal name of the field. This may be a qualified database field name, or it might be something more complex like a case statement
CString CSelectedField::GetInternalFieldName()
{
	return m_strInternalFieldName;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets the internal name of the selected field.
void CSelectedField::SetInternalFieldName(CString &strInternalFieldName)
{
	m_strInternalFieldName = strInternalFieldName;
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets the apparent field name without any prefix or suffix
CString CSelectedField::GetUnaffixedFieldName()
{
	return m_strUnaffixedFieldName;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets the apparent field name without any prefix or suffix
void CSelectedField::SetUnaffixedFieldName(CString &strUnaffixedFieldName)
{
	m_strUnaffixedFieldName = strUnaffixedFieldName;
}
	
// (r.gonet 11/27/2013) - PLID 59833 - Gets the name of the field as it will appear in recordset. This may
// be prefixed or suffixed to avoid ambiguity.
CString CSelectedField::GetApparentFieldName()
{
	CString strApparentFieldName;
	if(!m_bIsKey) {
		// Add a prefix
		strApparentFieldName = GetPrefix();
	}
	if(m_nSuffixNumber != 0) {
		// Add a suffix since there are more than one fields by the same name in the select list. Field names have to be unique.
		// The suffix is actually applied to the prefix 
		strApparentFieldName += FormatString("(%li)", m_nSuffixNumber);
	}
	if(!strApparentFieldName.IsEmpty()) {
		strApparentFieldName += " ";
	}
	strApparentFieldName += m_strUnaffixedFieldName;
	
	return strApparentFieldName;
	
}

// (r.gonet 11/27/2013) - PLID 59833 - Get the name of the base type
CString CSelectedField::GetPrefix()
{
	switch(m_fboType)
	{
	case fboPerson:
		return "Patient";
	case fboEMN:
		return "EMN";
	case fboAppointment:
		return "Appointment";
	case fboTodo:
		return "To-Do";
	case fboEMR:
		return "EMR";
	case fboPayment:
		return "Payment";
	case fboLabResult:
		return "Lab Result";
	case fboImmunization:
		return "Immunization";
	case fboEmrProblem:
		return "Problem";
	case fboMedication:
		return "Medication";
	case fboAllergy:
		return "Allergy";
	case fboLab:
		return "Lab";
	default:
		return "";
	}
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets the suffix that will make this field's name unique in the select list.
// A suffix of 0 is the default suffix and should not be displayed.
long CSelectedField::GetSuffixNumber()
{
	return m_nSuffixNumber;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets the suffix that will make this field's name unique in the select list.
// A suffix of 0 is the default suffix and should not be displayed.
void CSelectedField::SetSuffixNumber(long nSuffixNumber)
{
	m_nSuffixNumber = nSuffixNumber;
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets the filter field base type that this selected field represents
FilterBasedOnEnum CSelectedField::GetBaseType()
{
	return m_fboType;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets the filter field base type that this selected field represents
void CSelectedField::SetBaseType(FilterBasedOnEnum fboType)
{
	m_fboType = fboType;
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets the id of the corresponding filter field
long CSelectedField::GetFilterFieldInfoID()
{
	return m_nFilterFieldInfoID;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets the id of the corresponding filter field
void CSelectedField::SetFilterFieldInfoID(long nFilterFieldInfoID)
{
	m_nFilterFieldInfoID = nFilterFieldInfoID;
}


// (r.gonet 11/27/2013) - PLID 59833 - Returns true if this selected field is a key in the containing query. Keys are not propegated to the parent query's select list.
bool CSelectedField::GetIsKey()
{
	return m_bIsKey;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets whether this is a key in the containing query. Keys are not propegated to the parent query's select list.
void CSelectedField::SetIsKey(bool bIsKey)
{
	m_bIsKey = bIsKey;
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets the order in which this selected field appears in the select list
long CSelectedField::GetSelectOrderIndex()
{
	return m_nSelectOrderIndex;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sets the order in which tihs selected field appears in the select list
void CSelectedField::SetSelectOrderIndex(long nSelectOrderIndex)
{
	m_nSelectOrderIndex = nSelectOrderIndex;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs an empty CSqlPredicate
CSubQuery::CSqlPredicate::CSqlPredicate()
{
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlPredicate of the form LHSSchema.LHSObject <Operator> RHSSchema.RHSObject
CSubQuery::CSqlPredicate::CSqlPredicate(CString strLHSSchema, CString strLHSObject, CString strOperator, CString strRHSSchema, CString strRHSObject, CString strRHSAlias)
{
	m_strLHSSchema = strLHSSchema;
	m_strLHSObject = strLHSObject;
	m_strOperator = strOperator;
	m_strRHSSchema = strRHSSchema;
	m_strRHSObject = strRHSObject;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlPredicate of the form <Operator> RHSObject
CSubQuery::CSqlPredicate::CSqlPredicate(CString strOperator, CString strRHSObject)
{
	m_strOperator = strOperator;
	m_strRHSObject = strRHSObject;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlPredicate from an unstructured SQL expression
CSubQuery::CSqlPredicate::CSqlPredicate(CString strRawSql)
{
	m_strRHSObject = strRawSql;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a copy of a CSqlPredicate.
CSubQuery::CSqlPredicate::CSqlPredicate(const CSqlPredicate &other)
{
	m_strLHSSchema = other.m_strLHSSchema;
	m_strLHSObject = other.m_strLHSObject;
	m_strOperator = other.m_strOperator;
	m_strRHSSchema = other.m_strRHSSchema;
	m_strRHSObject = other.m_strRHSObject;
	m_strRHSAlias = other.m_strRHSAlias;
}

// (r.gonet 12/02/2013) - PLID 59820 - Generates the SQL string that represents this predicate.
CString CSubQuery::CSqlPredicate::ToString()
{
	CString strPredicate;
	if(!m_strLHSSchema.IsEmpty()) {
		strPredicate += FormatString("%s.", m_strLHSSchema);
	}
	if(!m_strLHSObject.IsEmpty()) {
		strPredicate += m_strLHSObject;
	}
	if(!m_strOperator.IsEmpty()) {
		strPredicate += FormatString(" %s ", m_strOperator);
	}
	if(!m_strRHSSchema.IsEmpty()) {
		strPredicate += FormatString("%s.", m_strRHSSchema);
	}
	if(m_strRHSAlias.IsEmpty()) {
		if(!m_strRHSObject.IsEmpty()) {
			strPredicate += m_strRHSObject;
		}
	} else {
		strPredicate += m_strRHSAlias;
	}
	return strPredicate;
}

// (r.gonet 10/23/2013) - PLID 59833 - Gets the LHS Schema
CString CSubQuery::CSqlPredicate::GetLHSSchema()
{
	return m_strLHSSchema;
}

// (r.gonet 10/23/2013) - PLID 59833 - Sets the LHS Schema
void CSubQuery::CSqlPredicate::SetLHSSchema(CString &strLHSSchema)
{
	m_strLHSSchema = strLHSSchema;
}

// (r.gonet 10/23/2013) - PLID 59833 - Gets the LHS Object
CString CSubQuery::CSqlPredicate::GetLHSObject()
{
	return m_strLHSObject;
}

// (r.gonet 10/23/2013) - PLID 59833 - Sets the LHS Object
void CSubQuery::CSqlPredicate::SetLHSObject(CString &strLHSObject)
{
	m_strLHSObject = strLHSObject;
}

// (r.gonet 10/23/2013) - PLID 59833 - Gets the Operator
CString CSubQuery::CSqlPredicate::GetOperator()
{
	return m_strOperator;
}

// (r.gonet 10/23/2013) - PLID 59833 - Sets the Operator
void CSubQuery::CSqlPredicate::SetOperator(CString &strOperator)
{
	m_strOperator = strOperator;
}

// (r.gonet 10/23/2013) - PLID 59833 - Gets the RHS Schema
CString CSubQuery::CSqlPredicate::GetRHSSchema()
{
	return m_strRHSSchema;
}

// (r.gonet 10/23/2013) - PLID 59833 - Sets the RHS Schema
void CSubQuery::CSqlPredicate::SetRHSSchema(CString &strRHSSchema)
{
	m_strRHSSchema = strRHSSchema;
}

// (r.gonet 10/23/2013) - PLID 59833 - Gets the RHS Object
CString CSubQuery::CSqlPredicate::GetRHSObject()
{
	return m_strRHSObject;
}

// (r.gonet 10/23/2013) - PLID 59833 - Sets the RHS Object
void CSubQuery::CSqlPredicate::SetRHSObject(CString &strRHSObject)
{
	m_strRHSObject = strRHSObject;
}

// (r.gonet 10/23/2013) - PLID 59833 - Gets the RHS Alias
CString CSubQuery::CSqlPredicate::GetRHSAlias()
{
	return m_strRHSAlias;
}

// (r.gonet 10/23/2013) - PLID 59833 - Sets the RHS Alias
void CSubQuery::CSqlPredicate::SetRHSAlias(CString &strRHSAlias)
{
	m_strRHSAlias = strRHSAlias;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlCondition that just contains a single predicate
CSubQuery::CSqlCondition::CSqlCondition(CSqlPredicatePtr pPredicate)
{
	m_esctConditionType = esctPredicate;
	m_pPredicate = pPredicate;
	m_bNegated = false;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a new CSqlCondition that contains multiple subconditions joined by a certain operator
// specified in the condition type
CSubQuery::CSqlCondition::CSqlCondition(ESqlConditionType esctConditionType)
{
	m_esctConditionType = esctConditionType;
	m_pPredicate.reset();
	m_bNegated = false;
}

// (r.gonet 11/27/2013) - PLID 59833 - Adds a child condition node to this condition.
bool CSubQuery::CSqlCondition::AddChildCondition(CSqlConditionPtr pChildCondition)
{
	if(m_esctConditionType != esctPredicate) {
		m_vecChildConditions.push_back(pChildCondition);
		return true;
	} else {
		return false;
	}
}

// (r.gonet 11/27/2013) - PLID 59833 - Adds a leaf condition that contains a predicate.
bool CSubQuery::CSqlCondition::AddPredicate(CSqlPredicatePtr pPredicate)
{
	if(m_esctConditionType != esctPredicate) {
		CSqlConditionPtr pPredicateCondition(new CSqlCondition(pPredicate));
		m_vecChildConditions.push_back(pPredicateCondition);
		return true;
	} else {
		return false;
	}
}

// (r.gonet 12/02/2013) - PLID 59820 - Gets the SQL string representation of this condition. Walks the condition tree.
CString CSubQuery::CSqlCondition::ToString()
{
	CString strFlatCondition;
	if(m_esctConditionType == esctPredicate && m_pPredicate != NULL) {
		strFlatCondition = m_pPredicate->ToString();
	} else {
		foreach(CSqlConditionPtr pChildCondition, m_vecChildConditions) {
			CString strChildCondition = pChildCondition->ToString();
			CString strAndOr;
			if(m_esctConditionType == esctAnd) {
				strAndOr = " AND ";
			} else if(m_esctConditionType == esctOr) {
				strAndOr = " OR ";
			}
			if(strChildCondition != "") {
				if(!strFlatCondition.IsEmpty()) {
					strFlatCondition += strAndOr;
				}
				strFlatCondition += FormatString("(%s)", strChildCondition);
			}
		}
	}
	if(strFlatCondition != "" && m_bNegated) {
		strFlatCondition = FormatString("CASE WHEN NOT (%s) THEN 1 ELSE 0 END = 1", strFlatCondition);	
	}
	return strFlatCondition;
}

// (r.gonet 11/27/2013) - PLID 59833 - Negates this current condition node
void CSubQuery::CSqlCondition::Negate()
{
	m_bNegated = !m_bNegated;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a table source from a unstructured SQL that hopefully represents a subquery or table.
CSubQuery::CSqlTableSource::CSqlTableSource(CString strRawSql)
{
	m_eJoinType = etsjtNone;
	m_strTableName = strRawSql;
	m_pSubQuery.reset();
	m_strTableSourceAlias = "";
	m_pJoinCondition.reset();
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a table source that represents a simple joined table.
CSubQuery::CSqlTableSource::CSqlTableSource(ETableSourceJoinType eJoinType, CString strTableName, CString strTableSourceAlias, CSqlConditionPtr pJoinCondition)
{
	m_eJoinType = eJoinType;
	m_strTableName = strTableName;
	m_pSubQuery.reset();
	m_strTableSourceAlias = strTableSourceAlias;
	m_pJoinCondition = pJoinCondition;
}

// (r.gonet 11/27/2013) - PLID 59833 - Constructs a table source that represens a joined subquery.
CSubQuery::CSqlTableSource::CSqlTableSource(ETableSourceJoinType eJoinType, CSubQueryPtr pSubQuery, CSqlConditionPtr pJoinCondition)
{
	m_eJoinType = eJoinType;
	m_pSubQuery = pSubQuery;
	m_strTableSourceAlias = pSubQuery->GetAlias();
	m_pJoinCondition = pJoinCondition;
}

// (r.gonet 12/02/2013) - PLID 59820 - Gets the SQL string that represents this table source.
CString CSubQuery::CSqlTableSource::ToString()
{
	CString strTableSource;
	if(m_eJoinType == etsjtInnerJoin) {
		strTableSource += "INNER JOIN ";
	} else if(m_eJoinType == etsjtLeftJoin) {
		strTableSource += "LEFT JOIN ";
	}
	if(m_pSubQuery != NULL) {
		strTableSource += FormatString(
			"( \r\n"
			"%s \r\n"
			") ", Tabify(m_pSubQuery->ToString(), 1));
	} else {
		strTableSource += FormatString("%s ", m_strTableName);
	}
	if(!m_strTableSourceAlias.IsEmpty()) {
		strTableSource += FormatString("%s ", m_strTableSourceAlias);
	}
	if(m_pJoinCondition != NULL) {
		strTableSource += FormatString("ON %s ", m_pJoinCondition->ToString());
	}
	return strTableSource;
}

// (r.gonet 11/27/2013) - PLID 59869 - Constructs an empty CFilterDetailSqlInfo
CSubQuery::CFilterDetailSqlInfo::CFilterDetailSqlInfo()
{
	m_bSelected = false;
}

// (r.gonet 11/27/2013) - PLID 59869 - Gets the ID field
CString CSubQuery::CFilterDetailSqlInfo::GetIDField()
{
	return m_strIDField;
}

// (r.gonet 11/27/2013) - PLID 59869 - Sets the ID field
void CSubQuery::CFilterDetailSqlInfo::SetIDField(CString &strIDField)
{
	m_strIDField = strIDField;
}

// (r.gonet 11/27/2013) - PLID 59869 - Gets the value field
CString CSubQuery::CFilterDetailSqlInfo::GetValueField()
{
	return m_strValueField;
}

// (r.gonet 11/27/2013) - PLID 59869 - Sets the value field
void CSubQuery::CFilterDetailSqlInfo::SetValueField(CString &strValueField)
{
	m_strValueField = strValueField;
}

// Returns true if the corresponding detail has been added to the select list. Returns false if the corresponding detail has not been added to the select list. 
bool CSubQuery::CFilterDetailSqlInfo::GetSelected()
{
	return m_bSelected;
}

// (r.gonet 11/27/2013) - PLID 59869 - Sets whether the corresponding detail has been added to the select list. 
// True if the corresponding detail has been added to the select list. False if the corresponding detail has not been added to the select list. 
void CSubQuery::CFilterDetailSqlInfo::SetSelected(bool bSelected)
{
	m_bSelected = bSelected;
}

// (r.gonet 11/27/2013) - PLID 59833 - Sort comparator for selected fields.
bool CSubQuery::SelectedFieldComparator::operator()(CSelectedFieldPtr &a, CSelectedFieldPtr &b) 
{ 
	// Compare the type of the selected field first. Certain base types always go before others
	int nATypeOrder = GetTypeOrder(a->GetBaseType());
	int nBTypeOrder = GetTypeOrder(b->GetBaseType());

	if(nATypeOrder < nBTypeOrder) {
		return true;
	}
	if(nBTypeOrder < nATypeOrder) {
		return false;
	}

	// Otherwise respect the defined order of the fields
	if(a->GetSelectOrderIndex() < b->GetSelectOrderIndex()) {
		return true;
	}
	if(b->GetSelectOrderIndex() < a->GetSelectOrderIndex()) {
		return false;
	}

	return false;
}

// (r.gonet 11/27/2013) - PLID 59833 - Gets whether one filter base type should come after the other.
int CSubQuery::SelectedFieldComparator::GetTypeOrder(FilterBasedOnEnum fbo)
{
	// Person fields are always first.
	switch(fbo) {
	case fboPerson:
		return 0;
	default:
		return 99;
	}
}

// (r.gonet 11/27/2013) - PLID 59829 - Constructs a new CSubFilterFrame
CSubQuery::CSubFilterFrame::CSubFilterFrame(CFilterPtr pFilter)
{
	m_pFilter = pFilter;
	m_bIsNegated = false;
	m_pParentFrame.reset();
	m_pParentFilterDetail = NULL;
}

// (r.gonet 11/27/2013) - PLID 59829 - Constructs a new CSubFilterFrame 
CSubQuery::CSubFilterFrame::CSubFilterFrame(CFilterPtr pFilter, bool bIsNegated, CSubFilterFramePtr pParentFrame, CFilterDetail *pParentFilterDetail)
{
	m_pFilter = pFilter;
	m_bIsNegated = bIsNegated;
	m_pParentFrame = pParentFrame;
	m_pParentFilterDetail = pParentFilterDetail;
}

// (r.gonet 11/27/2013) - PLID 59829 - Gets the filter being evaluated.
CFilterPtr CSubQuery::CSubFilterFrame::GetFilter()
{
	return m_pFilter;
}

// (r.gonet 11/27/2013) - PLID 59829 - Gets whether the filter associated with this frame is negated.
bool CSubQuery::CSubFilterFrame::GetIsNegated()
{
	return m_bIsNegated;
}

// (r.gonet 11/27/2013) - PLID 59829 - Gets the frame that this subfilter frame was created in. May be null if top level.
CSubQuery::CSubFilterFramePtr CSubQuery::CSubFilterFrame::GetParentFrame()
{
	return m_pParentFrame;
}

// (r.gonet 11/27/2013) - PLID 59829 - Gets the detail that specifies this subfilter. May be null if top level.
CFilterDetail * CSubQuery::CSubFilterFrame::GetParentFilterDetail()
{
	return m_pParentFilterDetail;
}

// (r.gonet 11/27/2013) - PLID 59829 - Constructs a new CSubQuery with the intention of it being a top level query
CSubQuery::CSubQuery(CFilteredSelect& batch, CFilterPtr pFilter)
: m_batch(batch)
{
	m_pWhereClause.reset(new CSqlCondition(CSqlCondition::esctOr));
	m_stackFrames.push(CSubFilterFramePtr(new CSubFilterFrame(pFilter)));
}

// (r.gonet 11/27/2013) - PLID 59202 - Constructs a new CSubQuery with the intention of it being a child subquery
CSubQuery::CSubQuery(CFilteredSelect& batch, CFilterPtr pFilter, bool bIsNegated, CSubFilterFramePtr &pParentFrame, CFilterDetail *pParentFilterDetail, CString &strAlias)
: m_batch(batch)
{
	m_pWhereClause.reset(new CSqlCondition(CSqlCondition::esctOr));
	m_stackFrames.push(CSubFilterFramePtr(new CSubFilterFrame(pFilter, bIsNegated, pParentFrame, pParentFilterDetail)));
	m_strAlias = strAlias;
}

// (r.gonet 11/27/2013) - PLID 59828 - Gets the alias of the current subquery within the parent query
CString CSubQuery::GetAlias()
{
	return m_strAlias;
}

// (r.gonet 11/27/2013) - PLID 59829 - Gets the current subfilter evaluation context frame
CSubQuery::CSubFilterFramePtr CSubQuery::GetCurrentFrame()
{
	if(m_stackFrames.empty()) {
		return CSubFilterFramePtr();
	} else {
		return m_stackFrames.top();
	}
}

// (r.gonet 11/27/2013) - PLID 59869 - Looks up SQL Info for a filter detail using the filter detail as a key
CSubQuery::CFilterDetailSqlInfoPtr CSubQuery::GetFilterDetailSqlInfo(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	CString strKey;
	pFilterDetail->GetDetailString(strKey);
	CFilterDetailSqlInfoPtr pValue;
	m_mapFilterDetailSqlInfo.Lookup(strKey, pValue);
	return pValue;
}

// (r.gonet 11/27/2013) - PLID 59869 - Caches SQL Info for a filter detail for later lookup using the filter detail as a key
void CSubQuery::StoreFilterDetailSqlInfo(CFilterDetail *pFilterDetail, CFilterDetailSqlInfoPtr pFilterDetailSqlInfo)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	CString strKey;
	pFilterDetail->GetDetailString(strKey);
	m_mapFilterDetailSqlInfo.SetAt(strKey, pFilterDetailSqlInfo);
}

// (r.gonet 11/27/2013) - PLID 59828 - Looks up a subquery using the filter detail as a key
CSubQueryPtr CSubQuery::GetChildSubQuery(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	CString strKey;
	pFilterDetail->GetDetailString(strKey);
	CSubQueryPtr pValue;
	m_mapDetailStringToSubQuery.Lookup(strKey, pValue);
	return pValue;
}

// (r.gonet 11/27/2013) - PLID 59828 - Caches a subquery for later lookup using the filter detail as a key
void CSubQuery::StoreChildSubQuery(CFilterDetail *pFilterDetail, CSubQueryPtr pChildSubQuery)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	CString strKey;
	pFilterDetail->GetDetailString(strKey);
	m_mapDetailStringToSubQuery.SetAt(strKey, pChildSubQuery);
}

// (r.gonet 11/27/2013) - PLID 59202 - Constructs a new CFilteredSelect. pFilter is the root filter.
CFilteredSelect::CFilteredSelect(CFilterPtr pFilter)
{
	m_pTopLevelQuery.reset(new CSubQuery(*this, pFilter));
}

// (r.gonet 11/27/2013) - PLID 59823 - Takes a filter field and map of joins and adds to the query's table sources the dependencies of the filter field.
bool CSubQuery::MoveJoinFromMapToClause(const CFilterFieldInfo *pffi, CMapStringToPtr &mapJoins)
{
	if (pffi) {
		// Make sure it hasn't already been added
		void *pbuf; // Unused variable for temp storage in lookup
		if (mapJoins.Lookup(pffi->m_strJoinClause, pbuf)) {
			bool bIsBase = false;
			// First add any dependencies
			if (pffi->m_nJoinDependsOn == FILTER_DEPENDS_ON_NONE) {
				//They don't want to add any joins, that's fine.
				bIsBase = true;
			}
			else if (pffi->m_nJoinDependsOn == FILTER_DEPENDS_ON_BASE) {
				long nIndex = CFilterFieldInfo::GetInfoIndex(pffi->m_nFilterType, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
				if((nIndex >= 0) && (nIndex <= CFilterDetail::g_nBaseDependencyCount)) {
					//Got the valid base dependency, so add it
					if (!this->MoveJoinFromMapToClause(&(CFilterDetail::g_BaseDependencies[nIndex]), mapJoins)) {
						// If this add failed, we failed
						return false;
					}
				} else {
					// There was a base dependency but it wasn't a valid filter field
					return false;
				}
			}
			else {
				long nIndex = CFilterFieldInfo::GetInfoIndex(pffi->m_nJoinDependsOn, (CFilterFieldInfo*)CFilterDetail::g_FilterFields, CFilterDetail::g_nFilterFieldCount);
				//We need to make sure that it's valid AND of the correct type.
				if ((nIndex >= 0) && (nIndex <= CFilterDetail::g_nFilterFieldCount)
					&& (pffi->m_nFilterType == CFilterDetail::g_FilterFields[nIndex].m_nFilterType)) {
					// Got the valid filter field dependency, so add it
					if (!this->MoveJoinFromMapToClause(&(CFilterDetail::g_FilterFields[nIndex]), mapJoins)) {
						// If this add failed, we failed
						return false;
					}
				} else {
					// There was a join dependency but it wasn't a valid filter field, or was the wrong type.
					ASSERT(FALSE);
					return false;
				}
			}
			// Use the filter-field's table/alias name and join-on string to add a join
			if (!pffi->m_strJoinClause.IsEmpty()) {
				if(bIsBase) {
					// This is a base table
					CSqlTableSourcePtr pTableSource(new CSqlTableSource(pffi->m_strJoinClause));
					this->AddTableSource(pTableSource);
					
				}
				else {
					// Add the join
					CSqlTableSourcePtr pTableSource(new CSqlTableSource("LEFT JOIN " + pffi->m_strJoinClause));
					this->AddTableSource(pTableSource);
				}
			}

			// We've added the string, so now remove it from the map
			mapJoins.RemoveKey(pffi->m_strJoinClause);
		}

		// Done
		return true;
	}

	// If we made it to here we failed
	return false;
}

// (r.gonet 11/27/2013) - PLID 59828 - Gets a filter from a subfilter filter detail.
CFilterPtr CSubQuery::GetFilterFromDetail(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	long nSubFilterID;
	CString strSubFilterText;
	filterFieldInfo.ParseSubFilterValue(pFilterDetail->GetDetailValue(), nSubFilterID, strSubFilterText, TRUE);
	CFilterPtr pSubFilter(new CFilter(nSubFilterID, filterFieldInfo.m_nSubfilterType, CGroups::IsActionSupported, CGroups::CommitSubfilterAction, NULL));
	pSubFilter->SetFilterString(strSubFilterText);
	return pSubFilter;
}

// (r.gonet 11/27/2013) - PLID 59823 - Adds the current filter's base type's dependency table sources to the subquery's table sources.
bool CSubQuery::AddBaseTableSources()
{
	CSubFilterFramePtr pFrame = this->GetCurrentFrame();
	CMapStringToPtr mapJoins;
	CMapPtrToWord mapExcludeParentJoins;
	if(pFrame->GetFilter()->GetJoinDependencies(mapJoins, mapExcludeParentJoins)) {
		const CFilterFieldInfo *pffi;
		// Build the full FROM clause based on the list of joins
		POSITION pos;
		CString strJoinClause;
		//ALL base tables should be included in the filterfieldinfo, we have no "base join" anymore.
		for (pos=mapJoins.GetStartPosition(); pos; ) {
			// Get the filter field info
			mapJoins.GetNextAssoc(pos, strJoinClause, (void *&)pffi);
			if (pffi) {
				// Try to add the clause to the string, and if it fails, we fail
				if (!this->MoveJoinFromMapToClause(pffi, mapJoins)) {
					// If we couldn't handle this one, the whole thing fails
					return false;
				}
			}
			// Now begin loop again because items have been removed
			pos = mapJoins.GetStartPosition();
		}
	}
	return true;
}

// (r.gonet 11/27/2013) - PLID 59826 - Adds the table source for combo values type fields
void CSubQuery::AddComboValuesTableSource(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];	
	CFilterDetailSqlInfoPtr pFilterDetailSqlInfo(new CFilterDetailSqlInfo);

	// (r.gonet 11/27/2013) - PLID 59826 - These are just a set of pre-defined values that the user can select from. Create a subquery for them
	// and union them all.
	CString strComboValueSubQuery = "";
	LPCTSTR pstrParamName = filterFieldInfo.GetNextParam(NULL), pstrParamData;
	long j = 0;
	for (j=0; pstrParamName && *pstrParamName; j++) {
		pstrParamData = filterFieldInfo.GetNextParam(pstrParamName);
		if (pstrParamData && *pstrParamData) {
			// (r.gonet 11/27/2013) - PLID 59826 - Put the value in the dropdown
			if(!strComboValueSubQuery.IsEmpty()) {
				strComboValueSubQuery += "\r\nUNION ALL \r\n";
			}
			strComboValueSubQuery += FormatString("SELECT '%s' AS ID, '%s' AS Name ", _Q(pstrParamData), _Q(pstrParamName));
		} else {
			// (r.gonet 11/27/2013) - PLID 59826 - There wasn't a data parameter for this name parameter
			ThrowNxException("%s : No data parameter was supplied for a name parameter in a combo filter field.", __FUNCTION__);
		}
		// (r.gonet 11/27/2013) - PLID 59826 - Find the next item
		pstrParamName = filterFieldInfo.GetNextParam(pstrParamData);
	}
	// (r.gonet 11/27/2013) - PLID 59826 - Now join this list of values to the outer query.
	if(!strComboValueSubQuery.IsEmpty()) {
		CString strSubQueryAlias = FormatString("ComboSubQ_InfoID_%li", filterFieldInfo.m_nInfoId);
		CString strValueField = FormatString("%s.Name", strSubQueryAlias);
		pFilterDetailSqlInfo->SetValueField(strValueField);

		strComboValueSubQuery = FormatString(
			"( \r\n"
			"%s \r\n"
			")",
			Tabify(strComboValueSubQuery, 1));

		if(!filterFieldInfo.m_strExistsSource.IsEmpty()) {
			// (r.gonet 11/27/2013) - PLID 59825 - This filter field uses a an exists clause which contains a subquery that references the outer query.
			// This require's special handling that normal letter writing fields do not require.
			CString strExistsSubQueryAlias, strExistsSubQueryObject;
			JoinExistsTableSource(pFilterDetail, strExistsSubQueryAlias, strExistsSubQueryObject);
			CSqlPredicatePtr pJoinPredicate(new CSqlPredicate(strSubQueryAlias, "ID", "=", strExistsSubQueryAlias, strExistsSubQueryObject, ""));
			CSqlConditionPtr pJoinCondition(new CSqlCondition(pJoinPredicate));
			CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, strComboValueSubQuery, strSubQueryAlias, pJoinCondition));
			this->AddTableSource(pTableSource);
			CString strIDField = FormatString("%s.%s", strExistsSubQueryAlias, strExistsSubQueryObject);
			pFilterDetailSqlInfo->SetIDField(strIDField);
		} else {
			// (r.gonet 11/27/2013) - PLID 59826 - Just join the subquery of unioned descriptive values to the user selected value.
			CSqlPredicatePtr pSubQueryJoinPredicate(new CSqlPredicate(strSubQueryAlias, "ID", "=", "", FormatString("(%s)", filterFieldInfo.m_strFieldNameInternal), ""));
			CSqlConditionPtr pSubQueryJoinCondition(new CSqlCondition(pSubQueryJoinPredicate));
			CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, strComboValueSubQuery, strSubQueryAlias, pSubQueryJoinCondition));
			this->AddTableSource(pTableSource);
			CString strIDField = FormatString("%s.ID", strSubQueryAlias);
			pFilterDetailSqlInfo->SetIDField(strIDField);
		}
		this->StoreFilterDetailSqlInfo(pFilterDetail, pFilterDetailSqlInfo);
	} else {
		// (r.gonet 11/27/2013) - PLID 59826 - No values! Odd. Probably won't ever happen.
	}
}

// (r.gonet 11/27/2013) - PLID 59824 - Create (or reuse) a temp table which will hold the dynamic ids and friendly names of values the user chose from when setting up the filter detail.
CString CSubQuery::GetComboSelectTempTable(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];

	// (r.gonet 11/27/2013) - PLID 59824 - This key just combines the field with whatever the dynamic ID was. The operator or record value does not affect
	// the temp table's values.
	CString strTempTableName;
	if(!m_batch.GetTempTableName(pFilterDetail, strTempTableName)) {
		CString strComboSelectSubQuery = filterFieldInfo.m_pstrParameters;
		if(pFilterDetail->GetDynamicRecordID() != INVALID_DYNAMIC_ID) {
			strTempTableName = FormatString("SelectedComboValues_InfoID_%li_DynamicID_%li", pFilterDetail->GetDetailInfoId(), pFilterDetail->GetDynamicRecordID());

			CString strDynamicID;
			strDynamicID.Format("%li", pFilterDetail->GetDynamicRecordID());
			strComboSelectSubQuery.Replace("{DYNAMIC_ID}", strDynamicID);
		} else {
			strTempTableName = FormatString("SelectedComboValues_InfoID_%li", pFilterDetail->GetDetailInfoId());
		}
		m_batch.StoreTempTableName(pFilterDetail, strTempTableName);

		// (r.gonet 11/27/2013) - PLID 59824 - Execute the selection query to see how the select list is ordered and also what the data type of the ID field is.
		_RecordsetPtr prsCombo = CreateRecordset(strComboSelectSubQuery);
		bool bIDFirst = true;
		CString strTypeName;
		for(long j = 0; j < prsCombo->Fields->Count; j++) {
			CString strName = (LPCTSTR)prsCombo->Fields->Item[j]->Name;
			ADODB::DataTypeEnum adDataType = prsCombo->Fields->Item[j]->Type;
			strName = strName.MakeLower();
			if(strName == "id") {
				if(j == 0) {
					bIDFirst = true;
				}
				if(adDataType == adTinyInt || adDataType == adSmallInt || adDataType == adInteger || 
					adDataType == adBigInt || adDataType == adUnsignedTinyInt || adDataType == adUnsignedSmallInt ||
					adDataType == adUnsignedInt || adDataType == adUnsignedBigInt)
				{
					strTypeName = "INT";
				}
				else if(adDataType == adBoolean)
				{
					strTypeName = "BIT";
				}
				else
				{
					strTypeName = "NVARCHAR(MAX)";
				}
			} else if(strName == "name") {
				if(j == 0) {
					bIDFirst = false;
				}
			}
		}
		prsCombo->Close();
		
		// (r.gonet 11/27/2013) - PLID 59824 - Add the table to the SQL executed before the record selection query is run,
		// which will guarantee the temp table exists when we need it.
		m_batch.AppendPreSelect(FormatString(
			"CREATE TABLE #%s \r\n"
			"( \r\n"
			"	ID %s, \r\n"
			"	Name NVARCHAR(MAX) \r\n"
			") \r\n"
			"INSERT INTO #%s (%s) \r\n"
			"%s",
			strTempTableName, 
			strTypeName,
			strTempTableName, (bIDFirst ? "ID, Name" : "Name, ID"), 
			strComboSelectSubQuery));
		if(filterFieldInfo.m_nInfoId == 181) {
			// (r.gonet 11/27/2013) - PLID 59824 - This field's IDs are strange and it is not just me saying that. But it is legacy and we can't do anything about it.
			m_batch.AppendPreSelect(FormatString(
				"UPDATE #%s SET ID = SUBSTRING(ID, 2, LEN(ID) - 2);\r\n",
				strTempTableName));
		}
		// (r.gonet 11/27/2013) - PLID 59824 - Make sure we clean up the temp table
		m_batch.PrependPostSelect(FormatString(
			"DROP TABLE #%s;",
			strTempTableName));
	} else {
		// (r.gonet 11/27/2013) - PLID 59824 - We've already made a temp table for this combo filter detail.
	}

	return strTempTableName;
}

// (r.gonet 11/27/2013) - PLID 59824 - Add the table sources required by combo select type fields such where the user has selected some value when setting up the filter detail from a dynamic set of values
// retrieved by some SQL statement.
void CSubQuery::AddComboSelectTableSource(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];

	// (r.gonet 11/27/2013) - PLID 59824 - Create (or reuse) a temp table which will hold the dynamic ids and friendly names of values the user chose from when setting up the filter detail.
	CString strTempTableName = GetComboSelectTempTable(pFilterDetail);
	// (r.gonet 11/27/2013) - PLID 59824 - While we may reuse the temp table for multiple details, we need a unique table source name for it.
	CString strTempTableAlias = FormatString("TempT_%s", NewUUID(true));

	CFilterDetailSqlInfoPtr pFilterDetailSqlInfo(new CFilterDetailSqlInfo);
	// (r.gonet 11/27/2013) - PLID 59824 - The field name which will will be selected for this detail and shown to the user. More user friendly then the ID field
	CString strValueField = FormatString("%s.Name", strTempTableAlias);
	pFilterDetailSqlInfo->SetValueField(strValueField);
	
	// (r.gonet 11/27/2013) - PLID 59824 - Add the table source
	if(filterFieldInfo.m_strExistsSource == "") {
		CSqlPredicatePtr pJoinPredicate;
		if(filterFieldInfo.m_bUseValueFormat) {
			// (r.gonet 11/27/2013) - PLID 59824 - Multi value
			CString strValueFormat = filterFieldInfo.m_strValueFormat;
			strValueFormat.Replace("{UserEnteredValue}", FormatString("%s.ID", strTempTableAlias));
			pJoinPredicate.reset(new CSqlPredicate("", filterFieldInfo.m_strFieldNameInternal, "IN", "", FormatString("(%s)", strValueFormat), ""));
			CString strIDField = filterFieldInfo.m_strFieldNameInternal;
			pFilterDetailSqlInfo->SetIDField(strIDField);
		} else {
			// (r.gonet 11/27/2013) - PLID 59824 - Single value
			pJoinPredicate.reset(new CSqlPredicate(strTempTableAlias, "ID", "=", "", FormatString("(%s)", filterFieldInfo.m_strFieldNameInternal), ""));
			CString strIDField = FormatString("%s.ID", strTempTableAlias);
			pFilterDetailSqlInfo->SetIDField(strIDField);
		}
		CSqlConditionPtr pJoinCondition(new CSqlCondition(pJoinPredicate));
		CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, FormatString("#%s", strTempTableName), strTempTableAlias, pJoinCondition));
		this->AddTableSource(pTableSource);
	} else {
		// (r.gonet 11/27/2013) - PLID 59825 - This filter field uses an exists clause which can reference fields in the outer query. 
		// We have special handling for that.
		CString strSubQueryAlias, strSubQueryObject;
		JoinExistsTableSource(pFilterDetail, strSubQueryAlias, strSubQueryObject);
		CSqlPredicatePtr pJoinPredicate(new CSqlPredicate(strTempTableAlias, "ID", "=", strSubQueryAlias, strSubQueryObject, ""));
		CSqlConditionPtr pJoinCondition(new CSqlCondition(pJoinPredicate));
		CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, FormatString("#%s", strTempTableName), strTempTableAlias, pJoinCondition));
		this->AddTableSource(pTableSource);
		CString strIDField = FormatString("%s.%s", strSubQueryAlias, strSubQueryObject);
		pFilterDetailSqlInfo->SetIDField(strIDField);
	}
	// (r.gonet 11/27/2013) - PLID 59824 - Cache the SQL info related to this detail.
	this->StoreFilterDetailSqlInfo(pFilterDetail, pFilterDetailSqlInfo);
}

// (r.gonet 11/27/2013) - PLID 59827 - Add the table sources required by simple fields such as text, number, etc. that all depend on the base type's dependencies.
void CSubQuery::AddSimpleTypeTableSource(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	CFilterDetailSqlInfoPtr pFilterDetailSqlInfo(new CFilterDetailSqlInfo);

	CString strFieldName;
	if(!filterFieldInfo.m_strExistsSource.IsEmpty()) {
		// (r.gonet 11/27/2013) - PLID 59825 - Very rarely these simple field types have an exist source. For instance info id = 294.
		// We have special handling for when the field uses an exists clause. We must transform that exists clause into a subquery.
		CString strSubQueryAlias, strSubQueryObject;
		JoinExistsTableSource(pFilterDetail, strSubQueryAlias, strSubQueryObject);
		strFieldName = FormatString("%s.%s", strSubQueryAlias, strSubQueryObject);
	} else {
		// (r.gonet 11/27/2013) - PLID 59827 - These are just basic fields such as Person Last Name, Person Birthdate, Appointment Date, etc.
		strFieldName = filterFieldInfo.m_strFieldNameInternal;
	}
	// (r.gonet 11/27/2013) - PLID 59827 - Cache the simple detail's SQL info.
	pFilterDetailSqlInfo->SetIDField(strFieldName);
	pFilterDetailSqlInfo->SetValueField(strFieldName);
	this->StoreFilterDetailSqlInfo(pFilterDetail, pFilterDetailSqlInfo);
}

// (r.gonet 11/27/2013) - PLID 59828 - Add the table sources required by a subfilter type field.
bool CSubQuery::AddSubFilterTableSource(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	if(filterFieldInfo.m_nSubfilterType == filterFieldInfo.m_nFilterType) {
		// Here's the deal with subfilter frames. We don't have a one to one relationship between SQL subqueries 
		// and subfilters. We add a new SQL subquery only if the subfilter has a different base type than the 
		// containing filter. If they have the same base type, then we "flatten" the subfilter into the parent. 
		// We do this to avoid repeating static fields.
		CSubFilterFramePtr pNewFrame = this->NewFrame(pFilterDetail);
		this->m_stackFrames.push(pNewFrame);
		this->AddTableSources();
		this->m_stackFrames.pop();
	} else {
		// The subfilter gets added as a new subquery since it has a different base type.
		CSubQueryPtr pSubQuery = this->NewSubQuery(pFilterDetail);
		if(!pSubQuery->Create()) {
			return false;
		}
		// We always left join. The where clause will take care of the cases where we actually care about if this returned any values.
		this->AddSubQuery(pSubQuery);
		this->StoreChildSubQuery(pFilterDetail, pSubQuery);
	}

	return true;
}

// (r.gonet 12/09/2013) - PLID 59869 - Advanced filters may reference columns without qualification. In order to prevent
// ambiguous column references, isolate the advanced filter in a subquery.
bool CSubQuery::AddAdvancedFilterTableSource(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	
	CFilterPtr pFilter = GetCurrentFrame()->GetFilter();
	if(!pFilter) {
		ThrowNxException("%s : pFilter is NULL.", __FUNCTION__);
	}

	// (r.gonet 12/09/2013) - PLID 59869 - The subquery will represent a filter without any details. A dummy subfilter.
	CFilterPtr pDummySubFilter(new CFilter(FILTER_ID_TEMPORARY, pFilter->GetFilterBase(), NULL, NULL, NULL, GetRemoteConnection()));

	// (r.gonet 12/09/2013) - PLID 59869 - We need a unique alias for the subfilter's subquery. Since one time subfilters can be used, we can't really make this name
	// meaningful in any way. Just use a GUID.
	CString strSubQueryAlias = FormatString("SubQ_%s", NewUUID(true));

	// (r.gonet 12/09/2013) - PLID 59869 - Construct the Advanced filter field subquery
	CSubQueryPtr pSubQuery(new CSubQuery(m_batch, pDummySubFilter, GetCurrentFrame()->GetIsNegated(), this->GetCurrentFrame(), pFilterDetail, strSubQueryAlias));

	// (r.gonet 12/09/2013) - PLID 59869 - The advanced filter expects some base tables to be available. Make them available.
	long nBaseDependencyIndex = CFilterFieldInfo::GetInfoIndex(filterFieldInfo.m_nFilterType, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
	CMapStringToPtr mapJoins;
	CMapPtrToWord mapExcludeParents;
	filterFieldInfo.AddJoinToMap(pFilterDetail, mapJoins, mapExcludeParents);
	CString strSubQueryBaseTableSources;
	::MoveJoinFromMapToClause(&filterFieldInfo, mapJoins, strSubQueryBaseTableSources);
	CSqlTableSourcePtr pBaseTableSource(new CSqlTableSource(strSubQueryBaseTableSources));
	pSubQuery->AddTableSource(pBaseTableSource);

	// (r.gonet 12/09/2013) - PLID 59869 - Fill the selection list.
	pSubQuery->AddKeyFields();

	// (r.gonet 12/09/2013) - PLID 59869 - Fill the where clause.
	CSqlConditionPtr pSubQueryWhereCondition(new CSqlCondition(CSqlCondition::esctOr));
	pSubQuery->AppendWhereCondition(pFilterDetail, pSubQueryWhereCondition);
	pSubQuery->SetWhereClause(pSubQueryWhereCondition);

	// (r.gonet 12/09/2013) - PLID 59869 - Join the subquery to the outer query.
	CSqlPredicatePtr pSubQueryJoinPredicate = GetSubFilterJoinPredicate((FilterBasedOnEnum)filterFieldInfo.m_nFilterType, (FilterBasedOnEnum)filterFieldInfo.m_nFilterType);
	// The join predicate is mostly correct with the exception of the RHS Schema, which is the subquery's alias.
	pSubQueryJoinPredicate->SetRHSSchema(pSubQuery->GetAlias());
	CSqlConditionPtr pJoinCondition(new CSqlCondition(pSubQueryJoinPredicate));
	CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, pSubQuery, pJoinCondition));
	if(!this->AddTableSource(pTableSource)) {
		return false;
	}
	this->StoreChildSubQuery(pFilterDetail, pSubQuery);
	return true;
}

// (r.gonet 11/27/2013) - PLID 59869 - Each detail references fields from some table(s), so add those tables to the query. 
bool CSubQuery::AddTableSource(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];

	if(this->GetFilterDetailSqlInfo(pFilterDetail) != NULL) {
		// We already added the table source for this filter detail.
		return true;
	} else {
		// We need to add the table source for this filter detail.
	}

	if(ExcludeFromSelection(pFilterDetail->GetDetailInfoId())) {
		// (r.gonet 11/27/2013) - PLID 59869 - For some reason, we can't select this field, so don't add its table sources.
	} else if(filterFieldInfo.m_ftFieldType == ftComboValues) {
		// (r.gonet 11/27/2013) - PLID 59826 - Combo Values fields use a pre-defined set of values that the user can choose from. Since we need to select the filtered fields, we need to get the
		// friendly value name. This entails adding a subquery composed of unions of those values.
		AddComboValuesTableSource(pFilterDetail);
	} else if(filterFieldInfo.m_ftFieldType == ftComboSelect) {
		// (r.gonet 11/27/2013) - PLID 59824 - Combo Select fields use a dynamic set of values that the user can choose from. These values come from a SQL query. We'll create a temp table for those
		// dynamic values and in-place subquery to handle this type of field.
		AddComboSelectTableSource(pFilterDetail);
	} else if(filterFieldInfo.m_ftFieldType == ftText || filterFieldInfo.m_ftFieldType == ftDate || filterFieldInfo.m_ftFieldType == ftNumber || 
		filterFieldInfo.m_ftFieldType == ftTime || filterFieldInfo.m_ftFieldType == ftPhoneNumber || filterFieldInfo.m_ftFieldType == ftCurrency) 
	{ 
		// (r.gonet 11/27/2013) - PLID 59827 - These are simple fields that just depend on the base type's dependencies.
		AddSimpleTypeTableSource(pFilterDetail);
	} else if(filterFieldInfo.m_ftFieldType == ftSubFilter || filterFieldInfo.m_ftFieldType == ftSubFilterEditable) {
		// (r.gonet 11/27/2013) - PLID 59828 - Ah, a subfilter. For subfilters, we create a subquery usually to handle them and then use that as the table source.
		if(!AddSubFilterTableSource(pFilterDetail)) {
			return false;
		}
	} else if(filterFieldInfo.m_ftFieldType == ftAdvanced) {
		if(!AddAdvancedFilterTableSource(pFilterDetail)) {
			return false;
		}
	} else {
		// Any other type, don't add. In fact, assert since the type may be new and may need to be added to the above handling.
		ASSERT(FALSE);
	}
	return true;
}

// (r.gonet 11/27/2013) - PLID 59869 - Adds the table sources that all the filter's details depend on.
bool CSubQuery::AddTableSources()
{
	// Add the the base type's dependency table sources. For instance, if this is filter is base type Person, add the PersonT table source.
	if(!AddBaseTableSources()) {
		return false;
	}
	
	// Add the regular table sources that each filter detail is dependent on.
	CSubFilterFramePtr pFrame = this->GetCurrentFrame();
	for(long i = 0; i < pFrame->GetFilter()->GetDetailAryItemCount(); i++) {
		CFilterDetail *pFilterDetail = pFrame->GetFilter()->GetFilterDetail(i);
		if(!AddTableSource(pFilterDetail)) {
			return false;
		}
	}
	return true;
}

// (r.gonet 11/27/2013) - PLID 59829 - Creates a new subfilter frame from a filter detail. OK, here's the deal with subfilter frames. We don't have a
// one to one relationship between SQL subqueries and subfilters. We add a new SQL subquery only if the subfilter
// has a different base type than the containing filter. If they have the same base type, then we "flatten"
// the subfilter into the parent. We do this to avoid repeating static fields. A subfilter frame represents the the evaluation
// of a same type subfilter in the same SQL subquery's nest level.
CSubQuery::CSubFilterFramePtr CSubQuery::NewFrame(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	if(filterFieldInfo.m_ftFieldType != ftSubFilter && filterFieldInfo.m_ftFieldType != ftSubFilterEditable) {
		ThrowNxException("%s : Invalid type of filter detail.", __FUNCTION__);
	}

	CFilterPtr pSubFilter = GetFilterFromDetail(pFilterDetail);
	bool bNegateSubFilter = false;
	if(pFilterDetail->GetDetailOperator() == foNotIn) {
		// The frame is negated.
		bNegateSubFilter = !(GetCurrentFrame()->GetIsNegated());
	}
	// Construct the new frame. Tell it what parent frame it came from and if it is negated.
	CSubFilterFramePtr pNewFrame(new CSubFilterFrame(pSubFilter, bNegateSubFilter, GetCurrentFrame(), pFilterDetail));

	return pNewFrame;
}

// (r.gonet 11/27/2013) - PLID 59828 - Generates a child subquery that will represent a given filter detail
CSubQueryPtr CSubQuery::NewSubQuery(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : Filter detail is NULL.", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	if(filterFieldInfo.m_ftFieldType != ftSubFilter && filterFieldInfo.m_ftFieldType != ftSubFilterEditable) {
		ThrowNxException("%s : Invalid type of filter detail.", __FUNCTION__);
	}
	
	// The detail is a subfilter detail and thus has a filter string of its own. Get the CFilter from that string.
	CFilterPtr pSubFilter = GetFilterFromDetail(pFilterDetail);
	bool bNegateSubQuery = false;
	if(pFilterDetail->GetDetailOperator() == foNotIn) {
		// The detail specifies that we do not want filtered records to occur within the subfilter. This is easy to say
		// in LW where clauses, but since we actually have to select the fields that are being filtered on, we instead completely
		// negate subquery, forcing it to return the inverse of what it would return left unnegated
		bNegateSubQuery = !(GetCurrentFrame()->GetIsNegated());
	}
	// We need a unique alias for the subfilter's subquery. Since one time subfilters can be used, we can't really make this name
	// meaningful in any way. Just use a GUID.
	CString strSubQueryAlias = FormatString("SubQ_%s", NewUUID(true));

	// Construct the subquery and return it.
	CSubQueryPtr pSubQuery(new CSubQuery(m_batch, pSubFilter, bNegateSubQuery, this->GetCurrentFrame(), pFilterDetail, strSubQueryAlias));
	return pSubQuery;
}

// (r.gonet 11/27/2013) - PLID 59869 - Gets the where clause of a SQL query.
CSubQuery::CSqlConditionPtr CSubQuery::GetWhereClause()
{
	return m_pWhereClause;
}

// (r.gonet 11/27/2013) - PLID 59869 - Sets the where clause of the SQL query.
void CSubQuery::SetWhereClause(CSqlConditionPtr pWhereClause)
{
	m_pWhereClause = pWhereClause;
}

// (r.gonet 12/02/2013) - PLID 59823 - Some fields are always selected when we filter on a particular base type. We always select key fields if this
// query is a child subquery of another, which are fields necessary to join this subquery with the outer query.
// We also select so-called static fields. Where as all the rest of the fields are dynamically added to the select
// list if the filter field is being used by one of the filter's details, the static fields are always added to the
// select list.
void CSubQuery::AddBaseSelectFields()
{
	CSubFilterFramePtr pFrame = this->GetCurrentFrame();
	if(this->m_stackFrames.size() == 1 && pFrame->GetParentFrame() != NULL) {
		// (r.gonet 12/02/2013) - PLID 59823 - Add the key fields needed for joins on this subquery.
		this->AddKeyFields();
	} else {
		// (r.gonet 12/02/2013) - PLID 59823 - The key fields are not needed or desired, so don't select them.
	}
	this->AddStaticFields();
}

// (r.gonet 11/27/2013) - PLID 59202 - Some filter fields have in their SQL definitions placeholders for DYNAMIC_NAME. Particularly EMR related fields.
// The detail gives us the information necessary in order to get the real apparent name of the filter detail.
// This function gets that dynamic name.
CString CSubQuery::GetDynamicFieldName(CFilterDetail *pFilterDetail)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail was NULL");
	}
	if(pFilterDetail->GetDetailFieldNameApparent().Left(13) != "DYNAMIC_FIELD") {
		ThrowNxException("%s : pFilterDetail is not a dynamic field.");
	}

	CString strDynamicFieldName;
	CString strDynamicFieldQuery = FormatString(
		"SELECT ID, Name \r\n"
		"FROM (%s) SubQ \r\n"
		"WHERE ID = %li",
		pFilterDetail->GetDetailFieldNameApparent().Mid(14),
		pFilterDetail->GetDynamicRecordID());
	_RecordsetPtr rsDynamicList = CreateRecordsetStd(strDynamicFieldQuery);
	if(!rsDynamicList->eof) {
		strDynamicFieldName = AdoFldString(rsDynamicList->Fields, "Name");
	}
	rsDynamicList->Close();
	return strDynamicFieldName;
}

// (r.gonet 11/27/2013) - PLID 59827 - Populate the select list with the fields from the current filter's details.
void CSubQuery::AddSelectFields()
{
	// (r.gonet 12/02/2013) - PLID 59823 - Add the always-selected fields to the select list.
	AddBaseSelectFields();

	// (r.gonet 11/27/2013) - PLID 59827 - Go through all the details in the current filter and add the details to the SQL select list
	CSubFilterFramePtr pFrame = GetCurrentFrame();
	for(long i = 0; i < pFrame->GetFilter()->GetDetailAryItemCount(); i++) {
		// (r.gonet 11/27/2013) - PLID 59827 - Get the filter field for this detail.
		CFilterDetail *pFilterDetail = pFrame->GetFilter()->GetFilterDetail(i);
		const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];

		// (r.gonet 11/27/2013) - PLID 59827 - The internal apparent field 
		CString strValueField;
		CString strInternalFieldName;
		CString strApparentFieldName = filterFieldInfo.GetFieldNameApparent();

		CFilterDetailSqlInfoPtr pFilterDetailSqlInfo;
		if(filterFieldInfo.m_ftFieldType != ftSubFilter && filterFieldInfo.m_ftFieldType != ftSubFilterEditable) {
			if((pFilterDetailSqlInfo = this->GetFilterDetailSqlInfo(pFilterDetail)) != NULL) {
				if(pFilterDetailSqlInfo->GetSelected()) {
					// (r.gonet 11/27/2013) - PLID 59827 - This field has already been added to our select list.
				} else {
					if(strApparentFieldName.Left(13) == "DYNAMIC_FIELD") {
						// (r.gonet 11/27/2013) - PLID 59827 - Use the actual name of the field rather than the placeholder
						strApparentFieldName = GetDynamicFieldName(pFilterDetail);
					} else {
						// (r.gonet 11/27/2013) - PLID 59827 - The field does not have any dynamic placeholders
					}
					// (r.gonet 11/27/2013) - PLID 59827 - Select the field
					this->AddFieldToSelectList(pFilterDetailSqlInfo->GetValueField(), strApparentFieldName, filterFieldInfo.m_nInfoId, (FilterBasedOnEnum)filterFieldInfo.m_nFilterType);
					// (r.gonet 11/27/2013) - PLID 59827 - Mark it as selected so we only add it to the select list once
					pFilterDetailSqlInfo->SetSelected(true);
				}
			} else {
				// (r.gonet 11/27/2013) - PLID 59827 - This detail does not get added to the select list.
			}
		} else {
			// This is a subfilter. 
			if(filterFieldInfo.m_nSubfilterType == filterFieldInfo.m_nFilterType) {
				// This detail is a subfilter with the same type as the current filter. We flatten these types of subfilters into the parent subquery.
				// Make that the current subfilter so to merge its select list with the current filter's.
				CSubFilterFramePtr pChildFrame = this->NewFrame(pFilterDetail);
				this->m_stackFrames.push(pChildFrame);
				this->AddSelectFields();
				this->m_stackFrames.pop();
			} else {
				// We already imported the fields when we retrieved the table sources.
			}
		}
	}
}

// (r.gonet 11/27/2013) - PLID 59869 - Takes a filter detail belonging to some filter and creates the SQL where clause for the detail, then appends it to a growing where condition.
void CSubQuery::AppendWhereCondition(CFilterDetail *pFilterDetail, IN OUT CSqlConditionPtr pWhereCondition)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];

	CSubQueryPtr pChildSubQuery;
	// Handle regular fields and subfilters differently, as always.
	if((pChildSubQuery = this->GetChildSubQuery(pFilterDetail)) != NULL) {
		// This subfilter is a different base type and its subquery was put at a greater nest depth within the overall SQL query. We left join 
		// all subqueries though because only some of the filter products (from the SplitProducts function) require the join to return something.
		// This detail requires that that join. We have selected in the subquery a dummy value called the NotNullBit which is never null. By forcing 
		// the left joined subquery to always have its NotNullBit NOT NULL, we can make the left join as an inner join just for this product.
		// This accomplishes the <FilterField> IS IN <SubFilter> behavior (the NOT IN case being handled by the negation case statement in CreateWhereCondition)
		pWhereCondition->AddPredicate(CSqlPredicatePtr(new CSqlPredicate(pChildSubQuery->GetAlias(), "NotNullBit", "IS NOT", "", "NULL", "")));
	} else {
		if(filterFieldInfo.m_nSubfilterType == filterFieldInfo.m_nFilterType) {
			// The where condition for a same type subfilter is put into the current subfilter's where clause.
			CSubFilterFramePtr pChildFrame = this->NewFrame(pFilterDetail);
			this->m_stackFrames.push(pChildFrame);
			CSqlConditionPtr pSubFilterWhereCondition = this->CreateWhereCondition();
			this->m_stackFrames.pop();
			pWhereCondition->AddChildCondition(pSubFilterWhereCondition);
		} else {
			CString strIDField;
			CFilterDetailSqlInfoPtr pFilterDetailSqlInfo;
			if((pFilterDetailSqlInfo = this->GetFilterDetailSqlInfo(pFilterDetail)) != NULL) {
				// This detail will be in the select list and the where clause. We use the internal field name that we found or created when retrieving table sources.
				// It is important to use our CFilterDetailSqlInfo's internal field rather than the CFilterFieldInfo's m_strFieldNameInternal because for combo selects 
				// or combo values fields, we differ due to using in place subqueries.
				strIDField = pFilterDetailSqlInfo->GetIDField();
			} else {
				// This filter detail will not be in the select list, but it will be in the where clause. Examples are excluded fields and advanced filters.
				strIDField = filterFieldInfo.m_strFieldNameInternal;
			}

			// Let the normal LW filtering figure out the where predicate for the detail. Pass it our internal field name as an override internal field name in case it differs from
			// the defined internal field name.
			CMapStringToString mapUsedFilters;
			CString strDetailWhereCondition, strPrefix, strExistsBefore, strExistsAfter;
			if(!filterFieldInfo.FormatWhereCondition(pFilterDetail->GetDetailOperator(), pFilterDetail->GetDetailValue(), strDetailWhereCondition, mapUsedFilters, strPrefix, strExistsBefore, strExistsAfter, TRUE, GetRemoteConnection(), strIDField)) {
				ThrowNxException("%s : Could not get the where clause from a filter detail.", __FUNCTION__);
			}
			// Add the where predicate for the detail to the overall where condition.
			pWhereCondition->AddPredicate(CSqlPredicatePtr(new CSqlPredicate(Trim(strDetailWhereCondition))));
		}
	}
}

// (r.gonet 11/27/2013) - PLID 59869 - Returns the where condition for the current frame's filter.
CSubQuery::CSqlConditionPtr CSubQuery::CreateWhereCondition()
{
	CSubFilterFramePtr pFrame = GetCurrentFrame();
	// Add the where clauses
	CSqlConditionPtr pWhereCondition(new CSqlCondition(CSqlCondition::esctOr));
	if(pFrame->GetIsNegated()) {
		// The current subfilter is negated. Negate the where condition so as to return the opposite results.
		// This only occurs in NOT IN subfilters.
		pWhereCondition->Negate();
	}
	//this->GetWhereClause()->AddChildCondition(pWhereCondition);

	std::list<CFilterPtr> lstProductFilters;
	// OK. Here's the deal. A filter can be A AND B OR C AND D AND E OR F (ie AB+CDE+F). In boolean logic, we call that a sum of products
	// form. We need to get the individual products so we can properly construct the where clause SQL. Note the nesting is not necessary
	// for SQL execution but it is for our query generation code.
	SplitProducts(pFrame->GetFilter(), lstProductFilters);
	foreach(CFilterPtr pProductFilter, lstProductFilters) {
		CSqlConditionPtr pProductCondition(new CSqlCondition(CSqlCondition::esctAnd));

		// Build up the product's where condition from the filter details.
		for(long i = 0; i < pProductFilter->GetDetailAryItemCount(); i++) {
			CFilterDetail *pFilterDetail = pProductFilter->GetFilterDetail(i);
			AppendWhereCondition(pFilterDetail, pProductCondition);
		}
		// Add the product's where condition to the sum's where condition.
		pWhereCondition->AddChildCondition(pProductCondition);
	}
	return pWhereCondition;
}

// (r.gonet 11/27/2013) - PLID 59202 - Creates an in memory query for a CFilter object passed in the constructor. Can later be converted to a SQL string.
bool CSubQuery::Create()
{
	// (r.gonet 12/02/2013) - PLID 59869 - Add the FROM clause and the dependency JOINS, including an child subqueries.
	if(!this->AddTableSources()) {
		return false;
	}
	// Very simply, we now add the fields that the filter details are filtering on to the SELECT list.
	this->AddSelectFields();
	// (r.gonet 12/02/2013) - PLID 59869 - Now the above will get us the filter's base type's records in the database, but we only want some since this is a filter... The filter details
	// specify which records to select, so turn the filter details into a WHERE clause and add that to the query.
	CSqlConditionPtr pWhereCondition = this->CreateWhereCondition();
	if(pWhereCondition != NULL) {
		this->SetWhereClause(pWhereCondition);
	}
	
	return true;
}

// (r.gonet 11/27/2013) - PLID 59825 - Joins a filter field that uses an EXISTS clause which contains a subquery that references the outer query.
// For Letter Writing filtering, that's fine since all that does not need to select any of the fields it is filtering on. But here we do need to
// select all the fields we are filtering on. EXISTS is kind of black box though. It can be a query of its own. We need to somehow get 
// the table sources used in the EXISTS and add them onto the current query (which LW does not do) as JOINs.
void CSubQuery::JoinExistsTableSource(CFilterDetail *pFilterDetail, OUT CString &strSubQueryAlias, OUT CString &strSubQueryObject)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}
	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pFilterDetail->GetDetailInfoIndex()];
	if(filterFieldInfo.m_strExistsSource.IsEmpty()) {
		// Nothing to do.
		return;
	}

	// This filter detail is dependent on some base like Person or Appointment or Labs. Find the base's index.
	long nBaseDependencyIndex = CFilterFieldInfo::GetInfoIndex(filterFieldInfo.m_nFilterType, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
	// Get the base's primary key field. The BasePKField is what we can use to join the CSubQuery to this in-place subquery. If this filter detail is ultimately dependent on PersonT, the BasePKField will be PersonT.ID. If on AppointmentsT, the BasePKField will be AppointmentsT.ID, etc.
	CString strBasePKField = CFilterDetail::g_BaseDependencies[nBaseDependencyIndex].m_strFieldNameInternal;
	// (r.gonet 11/27/2013) - PLID 59825 - Give this in-place subquery all the table sources it depends on so it can reference them and not get a SQL error about missing table.
	CMapStringToPtr mapJoins;
	CMapPtrToWord mapExcludeParents;
	filterFieldInfo.AddJoinToMap(pFilterDetail, mapJoins, mapExcludeParents);
	CString strSubQueryBaseTableSources;
	::MoveJoinFromMapToClause(&filterFieldInfo, mapJoins, strSubQueryBaseTableSources);

	// (r.gonet 11/27/2013) - PLID 59825 - The exists clause may reference some dynamic id, so replace it with the actual record's id.
	CString strExistsSource = filterFieldInfo.m_strExistsSource;
	strExistsSource.Replace("{DYNAMIC_ID}", FormatString("%li", pFilterDetail->GetDynamicRecordID()));
	CString strExistsBaseWhere = filterFieldInfo.m_strExistsBaseWhere;
	strExistsBaseWhere.Replace("{DYNAMIC_ID}", FormatString("%li", pFilterDetail->GetDynamicRecordID()));

	// (r.gonet 11/27/2013) - PLID 59825 - OK. Here's the subquery SQL. We'll need to join this soon using the filter detail's internal field,
	// but that can vary from field to field. Heck, it doesn'n even need to be a field name, could be a case statement or something. To keep things constant,
	// alias the internal field as BasePKID so we can join on it in a bit. 
	CString strSubQuery = FormatString(
		"( \r\n"
		"	SELECT DISTINCT %s AS BasePKID, %s AS Value \r\n"
		"	FROM %s \r\n"
		"	INNER JOIN %s ON %s "
		")",
		strBasePKField, filterFieldInfo.m_strFieldNameInternal,
		Trim(Tabify(strSubQueryBaseTableSources, 2)),
		Trim(Tabify(strExistsSource, 2)),
		Trim(Tabify(strExistsBaseWhere, 2)));
	
	// Phew. Create an alias for the subquery.
	strSubQueryAlias = FormatString("SubQ_%s", NewUUID(true));
	strSubQueryObject = "Value";

	// Now join the EXISTS subquery to the current query. With these filter details that use EXISTS, the base of the detail will be the base of the current filter
	// so the join is known to us. eg if the filter detail has a base of Person, then the join condition will be SubQAlias.BasePKID = PersonT.ID
	CSqlPredicatePtr pJoinPredicate(new CSqlPredicate(strSubQueryAlias, "BasePKID", "=", "", strBasePKField, ""));
	CSqlConditionPtr pJoinCondition(new CSqlCondition(pJoinPredicate));
	// Join the in-place subquery onto the current query's table sources using a left join.
	CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, strSubQuery, strSubQueryAlias, pJoinCondition));
	this->AddTableSource(pTableSource);
}

// (r.gonet 11/27/2013) - PLID 59869 - Returns true if a field should be excluded from the select list. False if it should not.
bool CSubQuery::ExcludeFromSelection(long nFieldInfoID)
{
	switch(nFieldInfoID) {
		case 545: // Has NexWeb EMN, which is a hack and does not lend itself to being done as a selection.
			return true;
		default:
			return false;
	}
}

// (r.gonet 12/02/2013) - PLID 59823 - Add the always selected key fields, which are fields necessary to join this subquery with the outer query.
void CSubQuery::AddKeyFields()
{
	CSubFilterFramePtr pFrame = GetCurrentFrame();
	// Only add the key fields if this is the first evaluation context of a subquery.
	if(this->m_stackFrames.size() > 1 || pFrame->GetParentFrame() == NULL) {
		// Don't add them twice
		return;
	} else {
		// They need added.
	}

	CSubFilterFramePtr pParentFrame = pFrame->GetParentFrame();
	FilterBasedOnEnum fboParent = pParentFrame->GetFilter()->GetFilterBase();
	FilterBasedOnEnum fboChild = pFrame->GetFilter()->GetFilterBase();
	// We need to select the fields that are referenced in the join clause that our parent is using to join us.
	CSqlPredicatePtr pJoinPredicate = this->GetSubFilterJoinPredicate(fboParent, fboChild);
	
	// Select a dummy field that we can always reference in the outer query to check for existence of rows.
	this->AddFieldToSelectList("CONVERT(BIT, 1)", "NotNullBit", -1, fboChild, true);
	// Select the primary key field
	long nBaseDependencyIndex = CFilterFieldInfo::GetInfoIndex((unsigned long)fboChild, (CFilterFieldInfo*)CFilterDetail::g_BaseDependencies, CFilterDetail::g_nBaseDependencyCount);
	CString strPrimaryKeyField = CFilterDetail::g_BaseDependencies[nBaseDependencyIndex].m_strFieldNameInternal;
	this->AddFieldToSelectList(strPrimaryKeyField, strPrimaryKeyField, -1, fboChild, true);
	// Select the field that the outer query will be referencing to join this subquery on.
	if(pJoinPredicate != NULL) {
		CString strInternalFieldName;
		if(!pJoinPredicate->GetRHSSchema().IsEmpty()) {
			strInternalFieldName = FormatString("%s.%s", pJoinPredicate->GetRHSSchema(), pJoinPredicate->GetRHSObject());
		} else {
			strInternalFieldName = FormatString("%s", pJoinPredicate->GetRHSObject());
		}
		CString strApparentFieldName;
		if(!pJoinPredicate->GetRHSAlias().IsEmpty()) {
			strApparentFieldName = pJoinPredicate->GetRHSAlias();
		} else {
			strApparentFieldName = pJoinPredicate->GetRHSObject();
		}
		this->AddFieldToSelectList(strInternalFieldName, strApparentFieldName, -1, fboChild, true);
	}
}

// (r.gonet 12/02/2013) - PLID 59823 - Certain fields we always want to select depending on the base type. These are things that better identify the record selected.
void CSubQuery::AddStaticFields()
{
	if(m_stackFrames.size() > 1) {
		// Don't add the static fields twice.
		return;
	} else {
		// Need to add them
	}

	FilterBasedOnEnum fbo = GetCurrentFrame()->GetFilter()->GetFilterBase();
	if(fbo == fboAllergy) {
		this->AddFieldToSelectList("PatientAllergyT.AllergyID", "Allergy ID", -1, fbo, true);
		this->AddFieldToSelectList("PatientAllergyT.Data", "Allergy Name", 2005, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(PatientAllergyT.EnteredDate)", "Allergy Entered Date", 1502, fbo);
	} else if(fbo == fboAppointment) {
		this->AddFieldToSelectList("AppointmentsT.ID", "Appointment ID", -1, fbo, true);
		this->AddFieldToSelectList("dbo.AsDateNoTime(AppointmentsT.Date)", "Appointment Date", 278, fbo);
		this->AddFieldToSelectList("AppointmentsT.StartTime", "Appointment Start Time", 279, fbo);
	} else if(fbo == fboEMN) {
		this->AddFieldToSelectList("EMRMasterT.ID", "EMN ID", -1, fbo, true);
		this->AddFieldToSelectList("EMRMasterT.Description", "EMN Description", -1, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(EMRMasterT.Date)", "EMN Date", -1, fbo);
	} else if(fbo == fboEMR) {
		this->AddFieldToSelectList("EMRGroupsT.ID", "EMR ID", -1, fbo, true);
		this->AddFieldToSelectList("EMRGroupsT.Description", "EMR Description", -1, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(EMRGroupsT.InputDate)", "EMR Input Date", -1, fbo);
	} else if(fbo == fboEmrProblem) {
		this->AddFieldToSelectList("EMRProblemsT.ID", "EMR Problem ID", -1, fbo, true);
		this->AddFieldToSelectList("EMRProblemsT.Description", "Problem Description", 459, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(EMRProblemsT.EnteredDate)", "Problem Input Date", 460, fbo);
	} else if(fbo == fboImmunization) {
		this->AddFieldToSelectList("PatientImmunizationsT.ID", "Immunization ID", -1, fbo, true);
		this->AddFieldToSelectList("dbo.AsDateNoTime(PatientImmunizationsT.DateAdministered)", "Immunization Date Administered", 451, fbo);
	} else if(fbo == fboLab) {
		this->AddFieldToSelectList("LabsT.ID", "Lab ID", -1, fbo, true);
		this->AddFieldToSelectList("LabsT.FormNumberTextID", "Lab Form Number", 2005, fbo);
		this->AddFieldToSelectList("LabsT.Specimen", "Lab Specimen Label", 2006, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(LabsT.InputDate)", "Lab Input Date", 2003, fbo);
	} else if(fbo == fboLabResult) {
		this->AddFieldToSelectList("LabResultsT.ResultID", "Lab Result ID", -1, fbo, true);
		this->AddFieldToSelectList("LabResultsT.Name", "Lab Result Name", 433, fbo);
		this->AddFieldToSelectList("LabResultsT.Value", "Lab Result Value", 439, fbo);
		this->AddFieldToSelectList("LabResultsT.Units", "Lab Result Units", 443, fbo);
		this->AddFieldToSelectList("LabResultFlagsT_Static.Name", "Lab Result Flag", 438, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(LabResultsT.DateReceived)", "Lab Result Date Received", 434, fbo);
		CSqlPredicatePtr pPredicate(new CSqlPredicate("LabResultsT", "FlagID", "=", "LabResultFlagsT_Static", "ID", ""));
		CSqlConditionPtr pJoinCondition(new CSqlCondition(pPredicate));
		this->AddTableSource(CSqlTableSourcePtr(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, "LabResultFlagsT", "LabResultFlagsT_Static", pJoinCondition)));
	} else if(fbo == fboMedication) {
		this->AddFieldToSelectList("CurrentPatientMedsT.MedicationID", "Patient Medication ID", -1, fbo, true);
		this->AddFieldToSelectList("CurrentPatientMedsT.Data", "Medication Name", 1002, fbo);
		this->AddFieldToSelectList("dbo.AsDateNoTime(CurrentPatientMedsT.StartDate)", "Medication Start Date", 1003, fbo);
	} else if(fbo == fboPerson) {
		this->AddFieldToSelectList("PersonT.ID", "Patient Internal ID", -1, fbo, true);
		this->AddFieldToSelectList("PersonT.Last", "Patient Last Name", 79, fbo);
		this->AddFieldToSelectList("PersonT.First", "Patient First Name", 77, fbo);
		this->AddFieldToSelectList("PersonT.Middle", "Patient Middle Name", 78, fbo);
		this->AddFieldToSelectList("PatientsT_Static.UserDefinedID", "Patient ID", 168, fbo);
		this->AddFieldToSelectList("PersonT.BirthDate", "Patient Birth Date", 93, fbo);
		this->AddFieldToSelectList("PersonT.SocialSecurity", "Patient SS Number", 92, fbo);
		CSqlConditionPtr pJoinCondition(new CSqlCondition(CSqlCondition::esctAnd));
		pJoinCondition->AddPredicate(CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "PatientsT_Static", "PersonID", "")));
		pJoinCondition->AddPredicate(CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", ">", "", "0", "")));
		pJoinCondition->AddPredicate(CSqlPredicatePtr(new CSqlPredicate("PatientsT_Static", "CurrentStatus", "<>", "", "4", "")));
		this->AddTableSource(CSqlTableSourcePtr(new CSqlTableSource(CSqlTableSource::etsjtInnerJoin, "PatientsT", "PatientsT_Static", pJoinCondition)));
	} else if(fbo == fboPayment) {
		this->AddFieldToSelectList("LineItemT.ID", "Payment ID", -1, fbo, true);
		this->AddFieldToSelectList("LineItemT.Date", "Payment Date", 376, fbo);
	} else if(fbo == fboTodo) {
		this->AddFieldToSelectList("ToDoList.TaskID", "To-Do ID", -1, fbo, true);
		this->AddFieldToSelectList("dbo.AsDateNoTime(ToDoList.Deadline)", "To-Do Deadline", 332, fbo);
	}
}

// (r.gonet 11/27/2013) - PLID 59828 - We need to know how to join a parent subquery to a child subquery of different base types.
// These joins are defined nowhere in LW filtering. Define them here.
CSubQuery::CSqlPredicatePtr CSubQuery::GetSubFilterJoinPredicate(FilterBasedOnEnum fboParent, FilterBasedOnEnum fboChild)
{
	if(fboParent == fboPerson) {
		if(fboChild == fboPerson) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "PersonT", "ID", "Patient Internal ID"));
		} else if(fboChild == fboAllergy) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "PatientAllergyT", "PersonID", "Patient Internal ID"));
		} else if(fboChild == fboAppointment) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "AppointmentsT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboEMN) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "EmrMasterT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboEMR) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "EmrGroupsT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboEmrProblem) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "EmrProblemsT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboImmunization) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "PatientImmunizationsT", "PersonID", "Patient Internal ID"));
		} else if(fboChild == fboLab) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "LabsT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboLabResult) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "LabsT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboMedication) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "CurrentPatientMedsT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboPayment) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "LineItemT", "PatientID", "Patient Internal ID"));
		} else if(fboChild == fboTodo) {
			return CSqlPredicatePtr(new CSqlPredicate("PersonT", "ID", "=", "ToDoList", "PersonID", "Patient Internal ID"));
		}
	} else if(fboParent == fboAllergy) {
		if(fboChild == fboAllergy) {
			return CSqlPredicatePtr(new CSqlPredicate("PatientAllergyT", "ID", "=", "PatientAllergyT", "ID", "Patient Allergy ID"));
		}
	} else if(fboParent == fboAppointment) {
		if(fboChild == fboAppointment) {
			return CSqlPredicatePtr(new CSqlPredicate("AppointmentsT", "ID", "=", "AppointmentsT", "ID", "Appointment ID"));
		} else if(fboChild == fboPerson) {
			return CSqlPredicatePtr(new CSqlPredicate("AppointmentsT", "PatientID", "=", "PersonT", "ID", "Patient Internal ID"));
		}
	} else if(fboParent == fboEMN) {
		if(fboChild == fboEMN) {
			return CSqlPredicatePtr(new CSqlPredicate("EmrMasterT", "ID", "=", "EmrMasterT", "ID", "EMN ID"));
		}
	} else if(fboParent == fboEMR) {
		if(fboChild == fboEMR) {
			return CSqlPredicatePtr(new CSqlPredicate("EmrGroupsT", "ID", "=", "EmrGroupsT", "ID", "EMR ID"));
		}
	} else if(fboParent == fboEmrProblem) {
		if(fboChild == fboEmrProblem) {
			return CSqlPredicatePtr(new CSqlPredicate("EmrProblemsT", "ID", "=", "EmrProblemsT", "ID", "EMR Problem ID"));
		}
	} else if(fboParent == fboImmunization) {
		if(fboChild == fboImmunization) {
			return CSqlPredicatePtr(new CSqlPredicate("PatientImmunizationsT", "ID", "=", "PatientImmunizationsT", "ID", "Immunization ID"));
		}
	} else if(fboParent == fboLab) {
		if(fboChild == fboLab) {
			return CSqlPredicatePtr(new CSqlPredicate("LabsT", "ID", "=", "LabsT", "ID", "Lab ID"));
		} else if(fboChild == fboLabResult) {
			return CSqlPredicatePtr(new CSqlPredicate("LabsT", "ID", "=", "LabResultsT", "LabID", "Lab ID"));
		}
	} else if(fboParent == fboLabResult) {
		if(fboChild == fboLabResult) {
			return CSqlPredicatePtr(new CSqlPredicate("LabResultsT", "ID", "=", "LabResultsT", "ID", "Lab Result ID"));
		}
	} else if(fboParent == fboMedication) {
		if(fboChild == fboMedication) {
			return CSqlPredicatePtr(new CSqlPredicate("CurrentPatientMedsT", "MedicationID", "=", "CurrentPatientMedsT", "MedicationID", "Patient Medication ID"));
		}
	} else if(fboParent == fboPayment) {
		if(fboChild == fboPayment) {
			return CSqlPredicatePtr(new CSqlPredicate("LineItemT", "ID", "=", "LineItemT", "ID", "Payment ID"));
		}
	} else if(fboParent == fboTodo) {
		if(fboChild == fboTodo) {
			return CSqlPredicatePtr(new CSqlPredicate("ToDoList", "TaskID", "=", "ToDoList", "TaskID", "To-Do ID"));
		}
	}
	return CSqlPredicatePtr();
}

// (r.gonet 12/02/2013) - PLID 59820 - Adds a number of tabs to each line in str
CString Tabify(CString str, int nNumTabs)
{
	if(nNumTabs <= 0) {
		return str;
	}
	CString strTabs('\t', nNumTabs);
	str.Replace("\r\n", "\r\n" + strTabs + "");
	str = strTabs + str;
	return str;
}

// (r.gonet 11/27/2013) - PLID 59869 - Takes a filter in the sum of products form of A*B+A*C+A+B*C*D and splits it into its individual products A*B, A*C, A, B*C*D
void CSubQuery::SplitProducts(CFilterPtr pFilter, OUT std::list<CFilterPtr> &lstProductFilters)
{
	boost::shared_ptr<CFilter> pProductFilter;
	for(long i = 0; i < pFilter->GetDetailAryItemCount(); i++) {
		CFilterDetail *pFilterDetail = pFilter->GetFilterDetail(i);
		bool bUseOr = pFilterDetail->GetDetailUseOr();
		if(bUseOr || i == 0) {
			// The current product is finished and a new one begins.
			pProductFilter.reset(new CFilter(FILTER_ID_TEMPORARY, pFilter->GetFilterBase(), NULL, NULL, NULL, GetRemoteConnection()));
			lstProductFilters.push_back(pProductFilter);
		} else {
			// Continue appending details to the curent product.
		}
		pProductFilter->AddDetail(pFilterDetail->GetDetailInfoIndex(), pFilterDetail->GetDetailOperator(), pFilterDetail->GetDetailValue(), pFilterDetail->GetDetailUseOr(), pFilterDetail->GetDynamicRecordID());
	}
}

// (r.gonet 11/27/2013) - PLID 59202 - Adds a field to the select list of the query. The internal field name represents the source of the field. It may be qualified with a table name or even be an expression.
// The apparent name is what we select as the alias for the field. nSuffixNumber is a number which makes this field unique within the query's select list. 0 is the default.
void CSubQuery::AddFieldToSelectList(CString strInternalFieldName, CString strApparentFieldName, long nFilterFieldInfoID, FilterBasedOnEnum fboType, bool bIsKey/*=false*/, long nSuffixNumber/*=0*/)
{
	// We need to add this field to the select clause
	CSelectedFieldPtr pSelectedField(new CSelectedField(
		strInternalFieldName, strApparentFieldName, nSuffixNumber, 
		fboType, nFilterFieldInfoID, bIsKey, m_mapSelectList.GetSize()));

	if (pSelectedField->GetUnaffixedFieldName().Find("REPLACE_FIELD_NAME") != -1) {
		CString strQuery = pSelectedField->GetUnaffixedFieldName().Right(pSelectedField->GetUnaffixedFieldName().GetLength() - (pSelectedField->GetUnaffixedFieldName().Find("):") + 2));
		_RecordsetPtr rsName = CreateRecordset(strQuery);
		if(!rsName->eof) {
			pSelectedField->SetUnaffixedFieldName(AdoFldString(rsName, "Name", ""));
		}
		rsName->Close();
	}

	if(!pSelectedField->GetIsKey()) {
		CString strTypePrefix = pSelectedField->GetPrefix();
		int nPrefixIndex = pSelectedField->GetUnaffixedFieldName().Find(strTypePrefix + " "); 
		if(nPrefixIndex == 0) {
			// The field already has the prefix in its field name field, so 
			// adding it again would make it doubled. Remove it.
			CString strUnaffixedFieldName = pSelectedField->GetUnaffixedFieldName().Mid(CString(strTypePrefix + " ").GetLength());
			pSelectedField->SetUnaffixedFieldName(strUnaffixedFieldName);
		}
	}

	if(m_mapSelectList.PLookup(pSelectedField->GetApparentFieldName()) != NULL) {
		// Already added. Don't add the field again.
	} else {
		m_mapSelectList.SetAt(pSelectedField->GetApparentFieldName(), pSelectedField);
	}
}

// (r.gonet 11/27/2013) - PLID 59828 - Adds the select list from a subquery to the current query. Ensures the field names remain unique.
void CSubQuery::ImportSelectListFields(CSubQueryPtr pFromQuery)
{
	std::vector<CSelectedFieldPtr> vecSelectList;
	this->SortSelectList(vecSelectList);
	std::vector<CSelectedFieldPtr> vecChildSelectList;
	pFromQuery->SortSelectList(vecChildSelectList);
	
	// Get the maximum suffix for each of the base types already selected in the current query.
	CMap<FilterBasedOnEnum, FilterBasedOnEnum, long, long> mapMaxSuffixes;
	for(size_t i = 0; i < vecSelectList.size(); i++) {
		FilterBasedOnEnum fboType = vecSelectList[i]->GetBaseType();
		long nSuffixNumber = vecSelectList[i]->GetSuffixNumber();
		long nMaxSuffixNumber = -1;
		mapMaxSuffixes.Lookup(fboType, nMaxSuffixNumber);
		if(nSuffixNumber > nMaxSuffixNumber) {
			mapMaxSuffixes.SetAt(fboType, nSuffixNumber);
		}
	}

	// Go through all the pFromQuery's select list fields and add them to the current query's select list.
	// Ensure field names are unique.
	for(size_t i = 0; i < vecChildSelectList.size(); i++) {
		CSelectedFieldPtr pSelectedField = vecChildSelectList[i];
		if(pSelectedField->GetIsKey()) {
			// Do not select child subqueries' key fields.
			continue;
		}
		CString strInternalFieldName;
		// The internal name in the outer query will be the apparent name of the subquery's field.
		if(pFromQuery->GetAlias() != "") {
			strInternalFieldName = FormatString("%s.[%s]", pFromQuery->GetAlias(), pSelectedField->GetApparentFieldName());
		} else {
			strInternalFieldName = pSelectedField->GetInternalFieldName();
		}

		long nMaxSuffixNumber; 
		if(!mapMaxSuffixes.Lookup(pSelectedField->GetBaseType(), nMaxSuffixNumber)) {
			nMaxSuffixNumber = -1;
		}

		AddFieldToSelectList(strInternalFieldName, pSelectedField->GetUnaffixedFieldName(),
			pSelectedField->GetFilterFieldInfoID(), pSelectedField->GetBaseType(), false, 
			nMaxSuffixNumber + pSelectedField->GetSuffixNumber() + 1);
	}
}

// (r.gonet 11/27/2013) - PLID 59869 - Adds a table source to the query's list of table sources.
bool CSubQuery::AddTableSource(CSqlTableSourcePtr pTableSource)
{
	CString strTableSource = pTableSource->ToString();
	foreach(CSqlTableSourcePtr pExistingTableSource, m_lstTableSources) {
		// Ensure we don't add the same table source twice. This turns out to be impossible without just comparing the raw SQL strings.
		if(pExistingTableSource->ToString() == strTableSource) {
			return false;
		}
	}
	m_lstTableSources.push_back(pTableSource);
	return true;
}

// (r.gonet 11/27/2013) - PLID 59828 - Adds a subquery to the current query's list of table sources
bool CSubQuery::AddSubQuery(CSubQueryPtr pSubQuery)
{
	// Bubble-up the subquery's select list
	this->ImportSelectListFields(pSubQuery);
	CFilterDetail *pParentFilterDetail = pSubQuery->GetCurrentFrame()->GetParentFilterDetail();
	if(pParentFilterDetail == NULL) {
		ThrowNxException("%s : pParentFilterDetail is NULL!", __FUNCTION__);
	}
	const CFilterFieldInfo &filterFieldInfo = CFilterDetail::g_FilterFields[pParentFilterDetail->GetDetailInfoIndex()];
	CSqlPredicatePtr pSubQueryJoinPredicate = GetSubFilterJoinPredicate((FilterBasedOnEnum)filterFieldInfo.m_nFilterType, (FilterBasedOnEnum)filterFieldInfo.m_nSubfilterType);
	// The join predicate is mostly correct with the exception of the RHS Schema, which is the subquery's alias.
	pSubQueryJoinPredicate->SetRHSSchema(pSubQuery->GetAlias());
	CSqlConditionPtr pJoinCondition(new CSqlCondition(pSubQueryJoinPredicate));
	CSqlTableSourcePtr pTableSource(new CSqlTableSource(CSqlTableSource::etsjtLeftJoin, pSubQuery, pJoinCondition));
	if(!this->AddTableSource(pTableSource)) {
		return false;
	}
	
	return true;
}

// (r.gonet 12/02/2013) - PLID 59820 - Generate the SQL string for this query. If strOutputTempTableName is specified, then we'll output the records to a temp table.
CString CSubQuery::ToString(LPCTSTR strOutputTempTableName/*=NULL*/)
{
	CString strQuery, strSelectList, strFromClause, strWhereClause;

	// Construct the select list, respecting the order of the fields.
	std::vector<CSelectedFieldPtr> vecSortedFields;
	this->SortSelectList(vecSortedFields);
	for(std::vector<CSelectedFieldPtr>::iterator sortedFieldsIterator = vecSortedFields.begin(); sortedFieldsIterator != vecSortedFields.end(); sortedFieldsIterator++)
	{
		if(!strSelectList.IsEmpty()) {
			strSelectList += ", \r\n";
		}
		CSelectedFieldPtr pField = *sortedFieldsIterator;
		strSelectList += FormatString("%s AS [%s]", pField->GetInternalFieldName(), pField->GetApparentFieldName());
	}

	// Get the joins.
	for(std::list<CSqlTableSourcePtr>::iterator tableSourceIterator = m_lstTableSources.begin(); tableSourceIterator != m_lstTableSources.end(); tableSourceIterator++)
	{
		CSqlTableSourcePtr pTableSource = *tableSourceIterator;
		if(!strFromClause.IsEmpty()) {
			strFromClause += "\r\n";
		}
		strFromClause += FormatString("%s", pTableSource->ToString()); 
	}

	// Either just select the records or select them into a temp table
	if(strOutputTempTableName != NULL) {
		strQuery = FormatString(
			"SELECT \r\n"
			"%s \r\n"
			"INTO #%s \r\n"
			"FROM %s \r\n",
			Tabify(strSelectList, 1),
			strOutputTempTableName, 
			strFromClause);
	} else {
		strQuery = FormatString(
			"SELECT \r\n"
			"%s \r\n"
			"FROM %s \r\n",
			Tabify(strSelectList, 1), 
			strFromClause);
	}
	// Add the where clause
	if(m_pWhereClause != NULL) {
		strWhereClause = m_pWhereClause->ToString();
		if(!strWhereClause.IsEmpty()) {
			strQuery += FormatString("WHERE %s ", Trim(Tabify(strWhereClause, 1)));
		}
	}
		
	return strQuery;
}

// (r.gonet 11/27/2013) - PLID 58821 - Returns the map containing all the fields selected in the query
CMapStringToField& CSubQuery::GetSelectedFieldsMap()
{
	return m_mapSelectList;
}

// (r.gonet 11/27/2013) - PLID 59820 - Gets the columns marked as key columns in the select list
void CSubQuery::GetKeyColumns(OUT std::vector<CSelectedFieldPtr> &vecKeyColumns)
{
	CString strKeyColumns;
	std::vector<CSelectedFieldPtr> vecSortedFields;
	this->SortSelectList(vecSortedFields);
	for(std::vector<CSelectedFieldPtr>::iterator sortedFieldsIterator = vecSortedFields.begin(); sortedFieldsIterator != vecSortedFields.end(); sortedFieldsIterator++)
	{
		CSelectedFieldPtr pSelectedField = *sortedFieldsIterator;
		if(pSelectedField->GetIsKey()) {
			vecKeyColumns.push_back(pSelectedField);
		}
	}
}

// (r.gonet 11/27/2013) - PLID 59202 - Creates a filtered select using the filter from the constructor.
bool CFilteredSelect::Create()
{
	return m_pTopLevelQuery->Create();
}

// (r.gonet 12/02/2013) - PLID 59820 - Generates the SQL for the filtered select
CString CFilteredSelect::ToString()
{
	// Select the filtered results into a temp table
	CString strOutputTempTableName = FormatString("TempT_%s", NewUUID(true));
	CString strQuery = FormatString(
		"SET ANSI_WARNINGS OFF; \r\n"
		"SET NOCOUNT ON; \r\n"
		"%s",
		m_pTopLevelQuery->ToString(strOutputTempTableName));

	// Select the results from the temp table
	strQuery += FormatString(
		"\r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT * \r\n"
		"FROM #%s \r\n",
		strOutputTempTableName);

	// Get a select list of the key columns
	std::vector<CSelectedFieldPtr> vecKeyColumns;
	m_pTopLevelQuery->GetKeyColumns(vecKeyColumns);
	CString strKeyColumns;
	for(std::vector<CSelectedFieldPtr>::iterator keyColumnIterator = vecKeyColumns.begin(); keyColumnIterator != vecKeyColumns.end(); keyColumnIterator++)
	{
		CSelectedFieldPtr pField = *keyColumnIterator;
		if(!strKeyColumns.IsEmpty()) {
			strKeyColumns += ", ";
		}
		strKeyColumns += FormatString("[%s]", pField->GetUnaffixedFieldName());
	}

	// Find out how many records of the top level filter's base type there are and select that.
	strQuery += FormatString(
		"SELECT COUNT(*) AS TopLevelCount \r\n"
		"FROM \r\n"
		"( \r\n"
		"	SELECT DISTINCT %s \r\n"
		"	FROM #%s %s \r\n"
		") CountSubQ \r\n",
		strKeyColumns,
		strOutputTempTableName, strOutputTempTableName);
	strQuery += FormatString(
		"SET NOCOUNT ON; \r\n"
		"DROP TABLE #%s; \r\n"
		"SET NOCOUNT OFF;",
		strOutputTempTableName);

	// Run any SQL that must be run before the selection query.
	if(!m_strPreSelect.IsEmpty()) {
		strQuery = FormatString(
			"SET NOCOUNT ON;\r\n"
			"%s\r\n"
			"SET NOCOUNT OFF;\r\n"
			"%s",
			m_strPreSelect, strQuery);
	}
	// Run any SQL that must be run after the selection query.
	if(!m_strPostSelect.IsEmpty()) {
		strQuery = FormatString(
			"%s\r\n"
			"SET NOCOUNT ON;\r\n"
			"%s\r\n"
			"SET NOCOUNT OFF;\r\n",
			strQuery, m_strPostSelect);
	}

	strQuery += "SET ANSI_WARNINGS ON; ";
		
	return strQuery;
}

// (r.gonet 12/02/2013) - PLID 58821 - Gets a map of all the selected fields
CMapStringToField& CFilteredSelect::GetSelectedFieldsMap()
{
	return m_pTopLevelQuery->GetSelectedFieldsMap();
}

// (r.gonet 11/27/2013) - PLID 59202 - Sorts the selected fields by position and type so that they will appear in the select list more agreeably.
void CSubQuery::SortSelectList(OUT std::vector<CSelectedFieldPtr> &vecSortedSelectedFields)
{
	POSITION pos = m_mapSelectList.GetStartPosition();
	while(pos){
		CString strSelectedFieldName;
		CSelectedFieldPtr pSelectedField;
		m_mapSelectList.GetNextAssoc(pos, strSelectedFieldName, pSelectedField);
		vecSortedSelectedFields.push_back(pSelectedField);
	}
	CSubQuery::SelectedFieldComparator comparator;
	std::sort(vecSortedSelectedFields.begin(), vecSortedSelectedFields.end(), comparator);
}

// (r.gonet 11/27/2013) - PLID 59202 - Adds raw SQL before the selection query.
void CFilteredSelect::AppendPreSelect(CString strPreSelect)
{
	if(!m_strPreSelect.IsEmpty()) {
		m_strPreSelect += "\r\n";
	}
	m_strPreSelect += strPreSelect;
}

// (r.gonet 11/27/2013) - PLID 59202 - Adds raw SQL after the selection query.
void CFilteredSelect::PrependPostSelect(CString strPostSelect)
{
	if(!m_strPostSelect.IsEmpty()) {
		m_strPostSelect = "\r\n" + m_strPostSelect;
	}
	m_strPostSelect = strPostSelect + m_strPostSelect;
}

// (r.gonet 11/27/2013) - PLID 59202 - Looks up a temp table by filter detail. Returns true if found and false if not found
bool CFilteredSelect::GetTempTableName(CFilterDetail *pFilterDetail, CString &strTempTableName)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	CString strTempTableKey = FormatString("%li|%li", pFilterDetail->GetDetailInfoId(), pFilterDetail->GetDynamicRecordID());
	return m_mapTempTables.Lookup(strTempTableKey, strTempTableName) ? true : false;
}

// (r.gonet 11/27/2013) - PLID 59202 - Caches a temp table name by a detail key.
void CFilteredSelect::StoreTempTableName(CFilterDetail *pFilterDetail, CString &strTempTableName)
{
	if(pFilterDetail == NULL) {
		ThrowNxException("%s : pFilterDetail is NULL!", __FUNCTION__);
	}

	CString strTempTableKey = FormatString("%li|%li", pFilterDetail->GetDetailInfoId(), pFilterDetail->GetDynamicRecordID());
	m_mapTempTables.SetAt(strTempTableKey, strTempTableName);
}