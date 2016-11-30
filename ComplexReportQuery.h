#pragma once

// (c.haag 2016-04-01) - PLID 68251 - For optimization purposes, some reports may require a CTE to operate.
// Whenever necessary, the reports engine should use this object rather than a string to pass a query around
// in case there are more components to the query than just simple SQL.
class CComplexReportQuery
{
public:
	CString m_strCTE;
	CString m_strSQL;

public:
	CComplexReportQuery();
	CComplexReportQuery(const CString& strSQL);
	CComplexReportQuery(const CString& strCTE, const CString& strSQL);

	// Takes in simple SQL and assigns it to this query
	void operator =(IN const CString& strSQL);

public:
	// Returns true if this object contains the elements of a valid query
	bool IsValid();
	// Returns the patient statement query in the form "[CTE] SELECT * FROM ([SQL]) [Alias]"
	CString Flatten(const CString& strAlias = "SubQ") const;
};