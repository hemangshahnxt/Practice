#ifndef REPORT_INFO_H
#define REPORT_INFO_H

#pragma once


#include "NxReportLib/SharedReportInfo.h" // (c.haag 2015-02-23) - PLID 63751 - CReportInfo now inherits from CSharedReportInfo
#include "GlobalAuditUtils.h" // (j.dinatale 2011-03-29 14:34) - PLID 42982 - wasnt included here before, really should have been. Now needs to be
#include "ComplexReportQuery.h" // (c.haag 2016-04-01) - PLID 68251

//this is declared in GlobalReportUtils.h
struct CRParameterInfo;

// Use for listing all the reports and their various information
#define BEGIN_ADD_REPORTS(reps)																		const CReportInfo reps[] = {
#define ADD_REPORT																						CReportInfo
#define END_ADD_REPORTS(reps, repcnt)																}; const long repcnt = sizeof(reps) / sizeof(CReportInfo)

// Class that stores a report and all its various information
// (c.haag 2015-02-23) - PLID 63751 - We now inherit from a class that maintains the business logic for reports used in other projects
class CReportInfo : public CSharedReportInfo
{
public:
	friend class CNxReportJob;

public:
	// Constructor
	CReportInfo(long nID, LPCTSTR strPrintName, long nRepIDForRepGroup, LPCTSTR strReportName, LPCTSTR strRecordSource, LPCTSTR strCategory, LPCTSTR strReportFile, BOOL bCreateGroup, short nDetail, long nProvider, long nLocation, long nPatient, 
		short	nDateRange, short nDateFilter, BOOL bOneDate, LPCTSTR strDateCaption, BOOL bExternal, LPCTSTR strReportSpecificName, LPCTSTR strListBoxFormat, 
		LPCTSTR strListBoxWidths, LPCTSTR strListBoxSQL, LPCTSTR strFilterField, BOOL bExtended, LPCTSTR strExtraText, LPCTSTR strExtraField, LPCTSTR strExtraValue, BOOL bUseGroupOption, 
		short nExtendedStyle, LPCTSTR strExtendedSql, /*BOOL bPhaseOut = FALSE,*/ BOOL bEditable, long nVersion, CString strDateOptions, long nOneDateOption, long nSupportedSubfilter = -1, BOOL bAllowMultipleExtended = TRUE);
	CReportInfo();
	~CReportInfo();

	// (a.walling 2013-08-30 09:01) - PLID 57998 - Let the compiler create our copy constructor and assignment operator

	static long GetInfoIndex(long nID);
	static const long REPORT_NEXT_INFO_ID;

	typedef enum {NXR_RUN, NXR_PRINT} NxOutputType;

	
	// standard setup for all reports
	// upon adding a report, create an instance of this struct,
	// and pull all data you might need from the table ReportsT here.  
	// then, use this instead of the DAO

	CString	strPrintName;			// the name displayed to the user
	long	nRepIDForRepGroup;		// the nID of another report similar to this report in terms of what report groups it should belong to by default
	CString	strReportName;			// the report name in Access
	CString	strRecordSource;		// name of the final query in Practice.mdb (RC - I don't like but if so it is going to be the alias)
	CString	strCategory;
	BOOL	bCreateGroup;			// are they allowed to create a group out of this report?

	// Filter Data
	short	nDetail;				// disabled = 0; detail = 1; summary = 2
	long	nProvider;				// disabled = 0; all provs = -1; else provider id
	long	nPatient;				// disabled = 0; all pats = -1; else patient id
	long    nLocation;              // disabled = 0; all pats = -1; else location id
	long	nExtraID;

	short	nDateRange;				// disabled = 0; all dates = -1; date range = 2
	short	nDateFilter;			// disabled = 0; otherwise, ID of date option
	BOOL	bOneDate;				//		does this report only use one date?
	COleDateTime	DateFrom;		// if so, from date
	COleDateTime	DateTo;			//		and to date
	CString	strDateCaption;			//		and what dates its filtering on
	BOOL	bExternal;				// using an external form?
	CString strReportSpecificName;	// TODO: Add documentation
	CString strListBoxFormat;			// TODO: Add documentation
	CString strListBoxWidths;			// TODO: Add documentation
	CString strListBoxSQL;				// TODO: Add documentation
	CString strFilterField;				// TODO: Add documentation
	BOOL	bExtended;				// using the extended filter?
	CString strExtraField;			// RC -- this is the field name of the extended filter
	//TES 8/22/2005 - There can now be multiple extra values, and while I'm at it, let's make this object-oriented, shall we?
	//CString strExtraValue;			// and this is the currently selected value

	// (r.gonet 12/18/2012) - PLID 53629 - I don't like doing this, but we must save 
	//  the output types for statement reports at the time of the report being run so 
	//  we can properly pass them along to WriteToHistory. This prevents races where 
	//  the properties change while the the report is running.
	long nStatementType;
	long nOutputType;
	long nCategoryType; // (r.goldschmidt 2014-08-05 14:06) - PLID 62717
protected:
	BOOL bAllowMultipleExtended;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Utility functions that Practice should override
protected:
	// (c.haag 2015-02-23) - PLID 63751 - Gets the allowed locations for a field.
	CString GetAllowedLocationClause(const CString &strField) const;
	// (c.haag 2015-02-23) - PLID 63751 - Gets the allowed locations for a field.
	CSqlFragment GetAllowedLocationClause_Param(const CString &strField) const;

//DRT 10/26/2005 - PLID 18085 - Clean up some memory leaks around here by removing the ability for anyone and their brother to 
//	manipulate this array without worrying about memory.
protected:
	// (a.walling 2013-08-30 09:01) - PLID 57998 - Now using a map of strings for name->value.
	// Previously this used a CPtrArray, which was needlessly complicated, and for some reason used dynamically-allocated
	// memory as well, yikes. Still have to support it for backwards compatibility, but for now I am simply
	// using this map.
	std::map<CString, CString> m_params;
public:
	void AddToParameterList(CRParameterInfo*& prpi);
	void ClearParameterList();

public:
	//These can be used by things that do know that multiple values are possible.
	void SetAllowMultipleExtended(BOOL bAllow) {bAllowMultipleExtended = bAllow;}
	BOOL GetAllowMultipleExtended() {return bAllowMultipleExtended;}

public:
	BOOL	bUseGroupOption;		// 
	BOOL	bUseGroup;
	long		nGroup;
	BOOL    bUseFilter;
	long    nFilterID;
	long	nSupportedSubfilter;	//Any supported subfilter type for this report (-1=none, patients is always assumed).
	

	//TS 12/20/02: It is NOT unused right now, whoever said that.  It is used right now, but in a completely
	//random and unpredictable way because somebody did a half-assed job of commenting out.  NOW it is unused.
	//BOOL bPhaseOut;				// unused right now

	short nExtendedStyle;		// TODO: Add documentation

	CString strExtendedSql;		// TODO: Add documentation

	BOOL bEditable;

// TODO: A bunch (but not all) of the above member variables really should be protected
protected:
//	EBuiltInObjectIDs m_bioBuiltInObjectID;


// Fields that store additional filter criteria
public:
	CString strExternalFilter; // Filter to be used if bExternal is TRUE
	CString strGroupFilter; // Filter to be used for group filtering
	CString strFilterString; //string for the filter

	CString strDateOptions; //string containing a semicolon-delimited list with an indefinite number of
							//four-field-long rows: ID (1, 2; 1-based index unique within this string); 
							//						Display Name ("Service Date", "Input Date", shown on the screen)
							//						Field Name ("TDate", "IDate", used in generating the where clause)
							//						Report Suffix ("Service", "Input", used in generating .rpt filename, can be empty string)
	CString strDateFilterField; //TDate, IDate, Date, or whatever.

	long nOneDateOption;	//Valid iff bOneDate = TRUE; 0 = Less than or equal to given date, 1 = equal to given date.

	// (a.walling 2013-08-30 09:01) - PLID 57998 - Make copyable collection
	Nx::MFC::Copyable<CDWordArray> m_dwProviders;
	Nx::MFC::Copyable<CDWordArray> m_dwLocations;


	// (j.gruber 2008-07-11 15:47) - PLID 28976 - Add All Years setting
	BOOL bUseAllYears;

public:
	// Actually open the report given the reportinfo's current settings
	//TS 3/9/01:  In order to display the print options dialog, a CPrintInfo pointer needs to be passed
	//to these functions (if you're in an OnPreparePrinting function, you should have it.)
	BOOL ViewReport(const CString &strTitle, const CString &strFile, BOOL bPreview, CWnd *pParentWnd, CPrintInfo* pInfo = 0) const;
	//Another ViewReport function that takes a Parameter list
	// (a.walling 2013-08-30 09:01) - PLID 57998 - Handle map of params as well
	BOOL ViewReport(const CString &strTitle, const CString &strFile, const std::map<CString, CString>& paramList, BOOL bPreview, CWnd *pParentWnd, CPrintInfo* pInfo = 0) const;
	BOOL ViewReport(const CString &strTitle, const CString &strFile, CPtrArray* pParamList, BOOL bPreview, CWnd *pParentWnd, CPrintInfo* pInfo = 0) const;
	// Actually generate a new group based on this report
	// TS 7/17/03: This now handles prompting the user for a name, and returns whether a group was actually
	// created or not.
	BOOL CreateGroup() const;
	// Returns the Crystal filter for this report
	CString GetBaseFilter() const;
	//Gets the suffix field from the date option record with an id of nOption.
	CString GetDateSuffix(long nOption) const;
	//Gets the date filter field from the date option record with an id of nOption
	CString GetDateField(long nOption) const;
	//Gets the display name from the date option record with an id of nOption 
	CString GetDateName(long nOption) const;
	
	// (j.gruber 2010-12-01 10:30) - PLID 41540 - added a map for excluded patientIDs
	// (r.gonet 12/18/2012) - PLID 53629 - Pass along the saved statement output types.
	// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass along category type also
	static const void WriteToHistory(long nReportInfoID, long nSubLevel, long nSubRepNum, ADODB::_RecordsetPtr pRS, NxOutputType nxOutType, CMap<long, long, long, long> *mapExcludedPatientIDs = NULL, long nStatementType = -1, long nOutputType = -1, long nCategoryType = -1);

	// (j.gruber 2011-10-11 15:42) - PLID 45937 - need a way to determine when the report is running if it should write to history
	BOOL GetWriteToHistoryStatus() const;
	// (j.gruber 2011-10-13 10:04) - PLID 45916
	static const void UpdateAffiliateStatusToPaid(ADODB::_RecordsetPtr pRS);
	//(s.tullis 2016-05-18 17:41) - NX-100491
	/// <summary>
	/// Gets whether or not a report will use server side cursors.
	/// </summary>
	/// <param name="nReportID">ID of the report that may or may not use server side cursors.</param>
	/// <param name="bCalledFromTempTable">If true, then the caller is utilizing a temp table in the report query,
	/// which affects whether server side cursors can be used.</param>
	/// <returns>True if the report will use server side cursors. False otherwise.</returns>
	static BOOL IsServerSide(long nReportID, bool bCalledFromTempTableGeneration = false);
		
public:
	// Convert a crystal filter to a regular sql filter (makes use of this reportinfo's current settings)
	void ConvertCrystalToSql(IN const CString &strSql, IN OUT CString &strFilter) const;
	// Generic way of generating the report's filter
	CString GetFilter(long nSubLevel, long nSubRepNum) const;
protected:
	// Generic way of making last minute changes to a report query before opening it
	void FinalizeReportSqlQuery(long nSubLevel, long nSubRepNum, CString& strSql) const;
protected:
	// Generic way of opening a report's recordset (calls GetDefaultSql)
	ADODB::_RecordsetPtr OpenDefaultRecordset(long nSubLevel, long nSubRepNum, BOOL bForReportVerify) const;

protected:
	// Generates the list of patients that this report will result in (TODO: This uses GetDefaultSql but should actually use GetRecordset)
	// TS 7/17/03: This now handles prompting the user for a name, and returns whether a group was actually
	// created or not.
	BOOL CreateDefaultGroup(LPCTSTR strFilter, long nSubLevel, long nSubRepNum) const;

	// Default filtering functions
	CString DefGetProviderFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetLocationFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetPatientFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetDateFilterField(long nSubLevel, long nSubRepNum) const;
	CString DefGetDateFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetExtraFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetExternalFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetGroupFilter(long nSubLevel, long nSubRepNum) const;
	CString DefGetFilter(long nSubLevel, long nSubRepNum) const;
	//EBuiltInObjectIDs DefGetBuiltInObjectID() const;
	CString DefGetExtraField() const;
	CString DefGetDateSuffix(long nOption) const;
	CString DefGetDateField(long nOption) const;
	CString DefGetDateName(long nOption) const;
	
	//Ttx File functions
	BOOL DefCreateTtxFile(CString strPath) const;
	CString GetTtxFileContents(long nID, long nSubreport = 0, long nSubReportID = 0) const;
	BOOL CreateTtxFile(CReportInfo *pRep, CString strPath) const;

public:
	// (j.gruber 2007-02-22 16:47) - PLID 24832 - added to support subreports
	void DeleteTtxFiles() const;
	
	// (j.jones 2011-03-16 10:24) - PLID 21559 - made this a public function
	// Callback style function that is used to open the report recordset differently on a per-report basis
	ADODB::_RecordsetPtr GetRecordset(long nSubLevel, long nSubRepNum, BOOL bForReportVerify) const;

protected:
	
	// (j.jones 2009-11-17 16:13) - PLID 36326 - added functions for building the Provider Commissions recordsets
	ADODB::_RecordsetPtr GetProviderCommissionsChargesRecordset(long nSubLevel, long nSubRepNum, BOOL bForReportVerify) const;
	ADODB::_RecordsetPtr GetProviderCommissionsPaymentsRecordset(long nSubLevel, long nSubRepNum, BOOL bForReportVerify) const;

	// (j.jones 2009-11-23 10:08) - PLID 36326 - This function calculates tiered provider commissions
	// based on the content in the provided temp table, for all rules that are active on dtEndDate.
	// This function assumes the temp table has the following fields:
	// ChargeAmount, NoTaxTotal, NoShopFeeAmount, NoTaxNoShopFeeAmount, ProvID,
	// ChargeTypeID, TotalProviderCommissions, TotalProviderCommissionsNoTax
	//TES 8/20/2015 - PLID 65984 - The Payments report calculates slightly differently, added bPaymentsReport so it knows which calculation to use
	void CalculateProviderCommissions(ADODB::_ConnectionPtr pCon, CString strTempTableName, COleDateTime dtStartDate, COleDateTime dtEndDate, bool bPaymentsReport) const;

	// (j.gruber 2010-12-01 10:31) - PLID 41540 - added a map of excluded patient IDs
	// (r.gonet 12/18/2012) - PLID 53629 - Pass along the saved statement output types.
	// (r.goldschmidt 2014-08-05 14:06) - PLID 62717 - Pass along category type also
	static const void StatementWriteToHistory(ADODB::_RecordsetPtr pRS, long nType, NxOutputType nxOutType, CMap<long, long, long, long> *mapExcludedPatientIDs = NULL, long nStatementType = -1, long nOutputType = -1, long nCategoryType = -1);
	static const void TracerLetterWriteToHistory(ADODB::_RecordsetPtr pRS, NxOutputType nxOutType, CString strNote);
	//(e.lally 2010-09-10) PLID 40488
	static const void WellnessWriteToHistory(ADODB::_RecordsetPtr pRS, NxOutputType nxOutType, CString strNote);
	// (c.haag 2010-01-27 12:30) - PLID 36271 - This appears to now be dead code. If it weren't, I would have had to
	// make a change for this PL item.
	//static const void DefaultWriteToHistory(ADODB::_RecordsetPtr pRS, NxOutputType nxOutType, CString strNote);

	// (j.jones 2014-08-04 11:57) - PLID 63150 - added a modular function for saving to history
	static const void WriteTempTableToHistory(CString strTempTableName, long nCategoryID);	
	
	//CString GetStatementProvString(long nFormat) const; 
	//CString GetStatementChargeDescription() const;
	//CString GetStatementUnAppliedPrePaymentsString() const;
	CString GetQuoteEmpName() const;
	static const CString CreateWriteToHistoryTempTable_80(CMap<long, long, CString, CString> *mapPatients);
	// (j.gruber 2008-06-09 10:09) - PLID 29399 - added string version of function
	static const CString CreateWriteToHistoryTempTable_80WithStringKey(CMap<CString, LPCTSTR, CString, CString> *map);
	
	// Callback style function.  Listing of the default sql for each report or subreport (used by OpenDefaultRecordset)
	//TODO:  Pull the queries out of a .dat file instead of storing them in source code.  
	//See, once a function gets over 6000 lines (approximately), it can't be compiled.  So we had
	//to split this up into two functions, and everywhere it was called, we have to decide which 
	//version to call.  AR by Amount and AR by Patient are in GetSqlLow because AR is in GetSqlLow and they use the same
	//query.
	
	//these should probably be implemented soon because I think they are a lot better to use than 
	//the current high/Low system
	CString GetSqlPatients(long nSubLevel, long nSubRepNum) const;
	CString GetSqlScheduler(long nSubLevel, long nSubRepNum) const;
	CString GetSqlInventory(long nSubLevel, long nSubRepNum) const;
	CString GetSqlMarketing(long nSubLevel, long nSubRepNum) const;
	CString GetSqlContacts(long nSubLevel, long nSubRepNum) const;
	CString GetSqlCharges(long nSubLevel, long nSubRepNum) const;
	CString GetSqlPayments(long nSubLevel, long nSubRepNum) const;
	CString GetSqlFinancial(long nSubLevel, long nSubRepNum) const;
	CString GetSqlASC(long nSubLevel, long nSubRepNum) const;
	CString GetSqlAdministration(long nSubLevel, long nSubRepNum) const;
	CString GetSqlOther(long nSubLevel, long nSubRepNum) const;
	CString GetSqlPrintPreview(long nSubLevel, long nSubRepNum) const;
	CString GetMoreSqlPrintPreview(long nSubLevel, long nSubRepNum) const;
	CString GetSqlAR(long nSubLevel, long nSubRepNum) const;
	// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, return a CComplexReportQuery object
	CComplexReportQuery GetSqlStatement(ADODB::_ConnectionPtr pConnection, long nSubLevel, long nSubRepNum) const;
	// (j.gruber 2008-07-14 16:35) - PLID 28976 - added Practice Analysis tab
	CString GetSqlPracticeAnalysis(long nSubLevel, long nSubRepNum) const;
	//CString GetSqlHigh(long nSubLevel, long nSubRepNum) const;
	//CString GetSqlLow(long nSubLevel, long nSubRepNum) const;
	//CString GetSqlPatStatement(long nSubLevel, long nSubRepNum) const;
	// (j.gruber 2008-09-17 13:01) - PLID 30284 - added sublevel and subreport number
	CString GetSqlDailySchedule(bool bIncludeCancelled, long nSubLevel, long nSubRepNum) const;
	CString GetSqlFinancialActivity(long nSubLevel, long nSubRepNum) const;
	// (s.dhole 2012-04-19 17:16) - PLID 49341
	CString GetSqlOptical(long nSubLevel, long nSubRepNum) const;

	
	//TS:  Let's make this public, because CReports needs to set the tooltip to this, and besides, it's just an accessor function.
public:
	// Callback style function.  Listing of each report's description (this function is only changed when reports are added because all the report descriptions are stored in resources)
	CString GetDescription() const;

public:
	//EBuiltInObjectIDs GetBuiltInObjectID() const;

public:
	//DRT 7/3/2007 - PLID 11920 - Special filtering of the WHERE clause
	CString GetLocationWhereClause() const;

	// Filtering functions
	CString GetProviderFilter(long nSubLevel, long nSubRepNum) const;
	CString GetLocationFilter(long nSubLevel, long nSubRepNum) const;
	CString GetPatientFilter(long nSubLevel, long nSubRepNum) const;
	CString GetDateFilterField(long nSubLevel, long nSubRepNum) const;
	CString GetDateFilter(long nSubLevel, long nSubRepNum) const;
	CString GetExtraFilter(long nSubLevel, long nSubRepNum) const;
	CString GetExternalFilter(long nSubLevel, long nSubRepNum) const;
	CString GetGroupFilter(long nSubLevel, long nSubRepNum) const;
	CString GetFilterFilter(long nSubLevel, long nSubRepNum) const;
	CString GetExtraField() const;

public:
	//Function to create .ttx file
	bool CreateTtxFile() const;
	BOOL CreateTtxFile(CString strPath) const;

	// (a.walling 2007-06-19 13:50) - PLID 19405 - Verify the custom report without having to load an interface
	BOOL VerifyCustomReport() const;
	// (a.walling 2007-09-11 11:17) - PLID 19405 - Save the report to a temp file (must be renamed/moved)
	BOOL SaveCustomReport(LPDISPATCH lpReportDisp, const CString &strFileName) const;

	// (a.walling 2007-06-19 13:50) - PLID 19405 - Helper class to increase the global command timeout and restore
	// (a.walling 2009-08-11 13:48) - PLID 35178 - For any connection (moved to PracProps.h)

protected:
	// (d.thompson 2010-03-16) - PLID 37721 - Audit that the auditing report was executed.
	void AuditingReportExecuted(AuditEventItems aeiWhichReport) const;
};

/////////////////////////////////
//TS:  Defines and functions for loading p2smon.dll

typedef UINT (CALLBACK* CreateDefFnType) (LPUNKNOWN FAR * ,LPCSTR, BOOL);



bool EnsureP2smonDlls();
void FreeP2smonDlls();


static HMODULE l_hP2smonDll = NULL;
static bool l_bP2smonDllsLoaded;
static bool l_bP2smonDllsEnsured = false;

#endif
