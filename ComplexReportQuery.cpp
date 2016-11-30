// (c.haag 2016-04-01) - PLID 68251 - For optimization purposes, some reports may require a CTE to operate.
// Whenever necessary, the reports engine should use this object rather than a string to pass a query around
// in case there are more components to the query than just simple SQL.

#include "StdAfx.h"
#include "ComplexReportQuery.h"
#include "GlobalStringUtils.h"

CComplexReportQuery::CComplexReportQuery()
{
	m_strCTE = "";
	m_strSQL = "";
}

CComplexReportQuery::CComplexReportQuery(const CString& strSQL)
{
	m_strCTE = "";
	m_strSQL = strSQL;
}

CComplexReportQuery::CComplexReportQuery(const CString& strCTE, const CString& strSQL)
{
	m_strCTE = strCTE;
	m_strSQL = strSQL;
}

void CComplexReportQuery::operator =(IN const CString& strSQL)
{
	m_strCTE = "";
	m_strSQL = strSQL;
}

// Returns true if this object contains the elements of a valid query
bool CComplexReportQuery::IsValid()
{
	// Just check for an empty query. Don't worry about whitespace or malformed SQL; garbage in garbage out
	return m_strSQL.IsEmpty() ? false : true;
}

// Returns the patient statement query in the form "[CTE] SELECT * FROM ([SQL]) SubQ"
CString CComplexReportQuery::Flatten(const CString& strAlias) const
{
	return FormatString("%s SELECT * FROM (%s) %s", m_strCTE, m_strSQL, strAlias);
}