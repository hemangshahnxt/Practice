#pragma once

#include "GlobalReportUtils.h"

/// <summary>
/// Builds a patient statement report query.
/// </summary>
class CStatementSqlBuilder
{
protected:
	/// <summary>
	/// Connection to use when querying data or, more importantly, creating temp tables.
	/// </summary>
	ADODB::_ConnectionPtr m_pConnection;
	/// <summary>
	/// Which patient statement report to create the SQL for.
	/// </summary>
	CReportInfo *m_pReport;
	/// <summary>
	/// Report sub-level.
	/// </summary>
	long m_nSubLevel;
	/// <summary>
	/// The sub-report number.
	/// </summary>
	long m_nSubRepNum;
	/// <summary>
	/// Whether to generate the report SQL for e-statements or not.
	/// </summary>
	BOOL m_bIsEStatement;
public:
	/// <summary>
	/// Creates a new instance of the CStatementSqlBuilder class. 
	/// </summary>
	/// <param name="pConnection">Connection to use for all queries and temp table creation.</param>
	/// <param name="pReport">Which patient statement report to create the SQL for.</param>
	/// <param name="nSubLevel">Report sub-level.</param>
	/// <param name="nSubRepNum">Sub-report number.</param>
	CStatementSqlBuilder(ADODB::_ConnectionPtr pConnection, CReportInfo *pReport, long nSubLevel, long nSubRepNum);
	/// <summary>
	/// Creates a new instance of the CStatementSqlBuilder class with the option of generating the SQL for e-statements.
	/// </summary>
	/// <param name="pConnection">Connection to use for all queries and temp table creation.</param>
	/// <param name="pReport">Which patient statement report to create the SQL for.</param>
	/// <param name="nSubLevel">Report sub-level.</param>
	/// <param name="nSubRepNum">Sub-report number.</param>
	/// <param name="bIsEStatement">Whether to generate the report SQL for e-statements or not.</param>
	CStatementSqlBuilder(ADODB::_ConnectionPtr pConnection, CReportInfo *pReport, long nSubLevel, long nSubRepNum, BOOL bIsEStatement);
	// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, return a CComplexReportQuery object
	CComplexReportQuery GetStatementSql();
private:
	inline ADODB::_ConnectionPtr GetConnection() const
	{
		return m_pConnection;
	}

	// (j.gruber 2008-07-01 15:28) - PLID 30322 - needed to split this out since I reached the heap limit
	CString GetStatementByLocationSql(CString strRespFilter);
	//changed to be just for location statements
	CString GetLastLocationPaymentInformationSql(CString strPatTempTable, long nPatientID);
	// (j.gruber 2008-07-02 16:44) - PLID 29533 - added a by provider function because of compiler heap limit
	CString GetLastProviderPaymentInformationSql(CString strPatTempTable, long nPatientID);
	CString GetMoreStatementSql();
	CString CreateStatementFilterTable();
	CString GetStatementUnAppliedPrePaymentsString();
	CString GetStatementChargeDescription();
	CString GetStatementProvString(long nTransProvFormat);
	//TES 7/17/2014 - PLID 62565 - Added; in some cases two differently aliased chargeback tables both need to be suppressed, so this function requires an input.
	CString GetStatementChargebackString(const CString &strChargebacksTAlias);
};
