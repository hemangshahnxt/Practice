// (d.thompson 2010-01-18) - PLID 36927 - Created
#pragma once

// (j.gruber 2010-09-10 14:19) - PLID 40487 - make it a type in case we have to add more in the future
enum CCHITReportConfigType {
	crctNone = 0,
	crctEMRDataGroup = 1,
	crctEMRItem,
	crctEMRMultiItems, // (j.gruber 2010-09-14 11:34) - PLID 40514 - Multiple items
	crctMailsentCat, // (j.gruber 2010-09-21 13:47) - PLID 40617 - mailsent Categories
	crctEMRDataGroupOrSlider,	// (j.jones 2014-02-05 09:49) - PLID 60530 - vitals signs allow sliders in addition to emr data groups
};

// (j.gruber 2011-05-16 16:46) - PLID 43694
enum CCHITReportType {
	crtMU = 0,
	crtCCHIT,
};


//Used to track information on reports being generated
class CCCHITReportInfo {
public:
	
	CCCHITReportInfo();

	// (j.jones 2011-04-21 10:48) - PLID 43285 - added flag for whether the query allows use of the snapshot connection
	bool m_bCanUseSnapshot;

	CString m_strInternalName;		// (j.gruber 2011-11-04 12:43) - PLID 45692
	long m_nInternalID;				//(e.lally 2012-04-03) PLID 48264
	CString m_strDisplayName;		//Name of the report for display purposes only // (j.gruber 2011-11-04 12:48) - PLID 45692 - changed the name
	CString GetHelpGeneral();		//Accessor for m_strHelpGeneral
	CString GetHelpNum();			//Accessor for m_strHelpNum
	CString GetHelpDenom();			//Accessor for m_strHelpDenom

	// (j.gruber 2011-05-12 15:23) - PLID 43676 - provider filter
	// (j.gruber 2011-05-18 10:51) - PLID 43758 - sdded location list
	// (j.gruber 2011-09-27 10:28) - PLID 45616 - added bExcludeSecondaries
	// (j.gruber 2011-10-27 16:50) - PLID 46160 - suported sending in default denominator
	// (j.gruber 2011-11-01 11:54) - PLID 46219 - add connection
	// (j.gruber 2011-11-07 12:23) - PLID 45365 - exclusiong list
	//(e.lally 2012-02-24) PLID 48268 - Added nPatientID to filter on a single patient
	void Calculate(ADODB::_ConnectionPtr pCon, COleDateTime dtFrom, COleDateTime dtTo, CString strProviderList, CString strLocationList, CString strExclusionsList, BOOL bExcludeSecondaries, long nDefaultDenominator = -1, long nPatientID = -1);		//Calculates the num and denom given the queries and using the dates filtered

	double GetPercentage();			//Gets the percentage of this report.  Will call calculate if necessary
	long GetNumerator();			//Gets the numerator value, if it has been calculated.  Will call calculate if necessary
	long GetDenominator();			//Gets the denominator value, if it has been calculated.  Will call calculate if necessary
	// (j.gruber 2011-10-27 16:53) - PLID 46160
	BOOL UseDefaultDenominator() { return m_bUseDefaultDenominator;}

	//When declaring your report, provide queries to generate the data.
	//	The value selected must be called 'NumeratorValue' and 'DenominatorValue'
	void SetNumeratorSql(CString strSql) { m_strNumSql = strSql; }
	void SetDenominatorSql(CString strSql) { m_strDenomSql = strSql; }
	// (j.gruber 2011-10-27 16:20) - PLID 46160 - use the same denominator query where applicable
	void SetDefaultDenominatorSql();

	//(e.lally 2012-04-24) PLID 48266 - Moved sql declarations into separate function
	CString GetFilterDeclarations();

	// (r.gonet 06/12/2013) - PLID 55151 - Retrieves the initialization sql that must be run before the num and denom queries.
	// (c.haag 2015-08-31) - PLID 65056 - We can now apply filters to the initialization sql
	CString GetInitializationSql(COleDateTime dtFrom, COleDateTime dtTo, CString strProviderList, CString strLocationList, CString strExclusionsList, BOOL bExcludeSecondaries, long nPatientID = -1);
	// (r.gonet 06/12/2013) - PLID 55151 - Retrieves the cleanup sql that must be run before the num and denom queries.
	// (c.haag 2015-09-11) - PLID 66976 - Now returns a SQL fragment
	CSqlFragment GetCleanupSql();

	// (c.haag 2015-09-11) - PLID 65056 - Returns the SQL for dropping a temp table
	// (c.haag 2015-09-11) - PLID 66976 - Now returns a SQL fragment
	CSqlFragment GetDropTempTableSql(const CString& strTempTableName);

	// (j.gruber 2011-11-09 10:12) - PLID 45689
	//(e.lally 2012-02-24) PLID 48268 - Added nPatientID to filter on a single patient
	CString ApplyFilters(CString strSql, COleDateTime dtFrom, COleDateTime dtTo, CString strProviderList, CString strLocationList, CString strExclusionsList, BOOL bExcludeSecondaries, long nPatientID = -1);

	//Sets the help text to be used when more info is requested
	void SetHelpText(CString strGeneral, CString strNum, CString strDenom);

	//States that this report must be configured to pick a specific EMR data element (select list, row/column) for 
	//	the numerator.
	void SetConfigureType(CCHITReportConfigType crctType);
	CCHITReportConfigType GetConfigureType	();

	// (j.gruber 2011-05-13 11:57) - PLID 43694
	void SetReportType(CCHITReportType crtType);
	CCHITReportType GetReportType();
	CCHITReportType m_crtType;

	// (j.gruber 2011-11-08 11:01) - PLID 45689 - setReportInformation
	void SetReportInfo(CString strSelect, CString strNumFrom, CString strDenomFrom);
	CString GetReportSelect() { return m_strReportSelect; }
	CString GetReportNumFrom() { return m_strReportNumFrom; }
	CString GetReportDenomFrom() { return m_strReportDenomFrom; }
	
	// (j.jones 2014-11-07 11:38) - PLID 63983 - added ability for additional SQL declarations,
	// for SQL member variables, temp. tables, etc.
	CString GetInitializationSQL() { return m_strInitializationSQL; }
	void SetInitializationSQL(CString strSQL) { m_strInitializationSQL = strSQL; }

	//(e.lally 2012-02-28) PLID 48265
	bool HasBeenCalculated() { return m_bHasCalculated; }

	//(e.lally 2012-03-21) PLID 48707 - Get/Set what percentage the results have to be over in order to pass
	//	Right now this is only used by the MU Progress bar and set on selective measures that don't use a patient count
	int GetPercentToPass() { return m_nPercentToPass; }
	void SetPercentToPass(int nPercent) { m_nPercentToPass = nPercent; }

	// (r.gonet 06/12/2013) - PLID 55151 - Gets the unique name for the patient age temp table.
	CString EnsurePatientAgeTempTableName();

	// (c.haag 2015-09-11) - PLID 65056 - Ensures we have temp tables names used in clinical summary calculations
	void EnsureClinicalSummaryTempTableNames();

protected:
	bool m_bHasCalculated;			//Flag to determine if we've run the calculation queries yet
	long m_nNumerator;				//If flag is set, this is the numerator value pulled from the query.
	long m_nDenominator;			//If flag is set, this is the denominator value pulled from the query

	CString m_strNumSql;			//SQL for the numerator set by SetNumeratorSql
	CString m_strDenomSql;			//SQL for the denominator set by SetDenominatorSql

	CString m_strHelpGeneral;		//General help text about the report, when the user clicks More Info
	CString m_strHelpNum;			//Numerator specific help text about the report, when the user clicks More Info
	CString m_strHelpDenom;			//Denominator specific help text about the report, when the user clicks More Info
	BOOL m_bUseDefaultDenominator;  // (j.gruber 2011-10-27 16:54) - PLID 46160 - support default possibilities

	//bool m_bIsConfigurableEMR;		//Report must be configured to have an EMR element to control the numerator
	CCHITReportConfigType m_crctConfigType;		//if the report is configurable, we have different types it could be

	//(e.lally 2012-03-21) PLID 48707 - I would have made this a float so it's easier to use in calculations,
	//	but an integer better clues devs in that it is used as a whole number
	int m_nPercentToPass; //Given as percent between 0-100

	// (j.jones 2014-11-07 11:38) - PLID 63983 - added ability for additional SQL declarations,
	// for SQL member variables, temp. tables, etc.
	CString m_strInitializationSQL;

	// (j.gruber 2011-11-08 10:56) - PLID 45689 - members for reporting
	CString m_strReportSelect;
	CString m_strReportNumFrom;
	CString m_strReportDenomFrom;	

	// (r.gonet 06/12/2013) - PLID 55151 - Has the patient age temp table been named yet?
	bool m_bPatientAgeTempTableDeclared;
	// (r.gonet 06/12/2013) - PLID 55151 - The name of the patient age temp table.
	CString m_strPatientAgeTempTableName;

	// (r.gonet 06/12/2013) - PLID 55151 - Gets the sql that creates and fills the patient age temp table.
	CString GetPatientAgeTempTableSql();

	// (c.haag 2015-09-11) - PLID 65056 - The names of the clinical summary temp table names for MU timely clinical summary calculations
	CString m_strClinicalSummaryNumeratorTempTableName;
	CString m_strClinicalSummaryDenominatorTempTableName;

public:
	// (c.haag 2015-09-11) - PLID 66976 - Add and modify the result of this function to a query to perform MU timely clinical summary calculations.
	// The caller is expected to replace the square-bracketed fields with data or {SQL} parameters, and to generate a CSqlFragment object with it.
	CString GetUnparameterizedClinicalSummaryTempTablesSql();
	// (c.haag 2015-09-11) - PLID 65056 - Add the result of this function to a query to perform MU timely clinical summary calculations
	// Takes in the number of business days as a parameter since different reports have different metrics
	// Only one temp table may exist per CCHITReportInfo object, so choose your business days wisely
	// (c.haag 2015-09-11) - PLID 66976 - We now require the caller to pass in the code filter as well
	CString GetClinicalSummaryTempTablesSql(const CString& strCodes, long nBusinessDays);
	// (c.haag 2015-09-11) - PLID 65056 - Returns the timely clinical summary numerator temp table name
	CString GetClinicalSummaryNumeratorTempTableName(); 
	// (c.haag 2015-09-11) - PLID 65056 - Returns the timely clinical summary denominator temp table name
	CString GetClinicalSummaryDenominatorTempTableName();
};

