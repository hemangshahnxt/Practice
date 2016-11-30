#ifndef MARKETUTILS_H
#define MARKETUTILS_H

#pragma once



//TES 11/5/2007 - PLID 27978 - VS 2008 - VS 2008 doesn't recognize this file, which is fine, because nothing
// appears to actually need it.
//#include <fstream.h>
#include <afxtempl.h>
#include "client.h"
#include <afxdtctl.h>

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

// Callback function called by CTreeLong::EnumDescendents().
typedef BOOL (CALLBACK *ENUMDESCENDENTSPROC)(long nID, LPVOID pParam);

class GraphDescript;

////////////a.walling PLID 20695 5/18/06/////////////////////////
// CMarketRenderButtonStatus is a simple function that changes the Render button
// while rendering and then reverts the changes when it goes out of scope.
/////////////////////////////////////////////////////////////////
class CMarketRenderButtonStatus {
public:
	CWnd *wnd;
	bool m_bManualHide;

	CMarketRenderButtonStatus(CWnd *twnd, bool bManualHide = false);
	~CMarketRenderButtonStatus();

	void HideGo();

	static void ManualHide(CWnd *lwnd);
};

/////////////////////////////////////////////////////////////////
// CTreeLong: this data structure class represents a hierarchical tree where each 
// node is a long integer and can have any number of child nodes.
/////////////////////////////////////////////////////////////////
class CTreeLong 
{
public:
	CTreeLong(long nValue = 0, CPtrArray *paryptlChildren = NULL);
	~CTreeLong();

public:
	void Clear();
	void AddNewChild(long nValue = 0, CPtrArray *paryptlChildren = NULL);
	void AddChild(CTreeLong *pChild);

public:
	POSITION GetFirstChildPosition() const;
	BOOL GetNextChild(IN OUT POSITION *pp, OUT const CTreeLong **ppChild) const;

public:
	 BOOL EnumDescendents(ENUMDESCENDENTSPROC lpEnumDescendentsProc, LPVOID pParam, BOOL bIncludeSelf) const;

public:
	long m_nValue;

protected:
	CPtrArray *m_paryptlChildren;
};

// CMapLongToTreePtr is a handy class that is essentially a "mapped tree".  It lets you jump to 
// any spot in a CTreeLong by ID so that you don't have to search the whole tree in order to get 
// to a certain branch of it.
// NOTE: On destruction this class will call delete on the root of the tree (identified by the 
// key long in the constructor).  It is assumed that all entries you add to this map will either 
// be manually deleted by you, or (more likely) are children in the tree rooted at that key long.
class CMapLongToTreePtr : public CMap<long,long,CTreeLong*,CTreeLong*> 
{
public:
	CMapLongToTreePtr(long nRootKey);
	~CMapLongToTreePtr();

public:
	// Notice, these are virtual only for DERIVED classes, they are NOT virtual in our base class, 
	// and therefore if you call them on a base-class pointer, our function won't be called which 
	// is a shame.
	virtual BOOL RemoveKey(long nKey);
	virtual void RemoveAll();

protected:
	long m_nRootKey;
};

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


int clamp (int min, int max, int value);

CString Descendants(int id, CString field = "Name");
void Descendants(CArray<int,int> &array);
//(e.lally 2009-09-18) PLID 35300 - Added bIncludeNoReferralEntry, defaulted to false
void Descendants(long nID, const CTreeLong **pptlTreeLong, BOOL bIncludeNoReferralEntry = FALSE);
//TES 7/23/2009 - PLID 34227 - Added an overload that takes an array, for use when filtering the Marketing->Referral
// tab on multiple referral sources.
CString Descendants(const CDWordArray &arIDs, CString field = "Name");
void CategoryDescendants(CArray<int,int> &array);
CString CategoryDescendants(int id, CString field ="Name");

enum MarketGraphType{
	CNVConsToSurgByRefSour,			//Referrals Tab, Consult to Procedure button
	CNVConstoSurgByDate,
	CNVInqToConsByProc,
	CNVProsToConsByProc,
	CNVProsToSurgByProc,
	CNVInqToConsByStaff,
	CNVConsToSurgByPatCoord,		//Coordinator Tab, Consult to Procedure button
	EFFMonByRefSour,
	EFFMonByProc,
	EFFProfByProc,
	EFFProfByPatCoord,
	EFFMonByPatCoord,
	EFFRefEffect,
	EFFMonByCategory,
	NUMPatByRefSour,
	NUMPatNoShowsByProc,
	NUMInqByProc,
	NUMNoShowByProc,
	NUMCanByPatCoord,
	NUMNoShowbyPatCoord,
	NUMCanByProc,
	NUMCanByReason,
	NUMNoShowByReason,
	NUMConWithProsByPatCoord,
	NUMConsByPatCoord,
	NUMClosedByProc,
	NUMPerfByProc,
	DATECloseByProc,
	DATEPerfByProcs,
	DATECountByProcs,
	DATERevByDate,
	DATEApptToCharge, // (j.gruber 2011-05-03 16:01) - PLID 38153
	REFInqByReferral,
	REFNoShowByReferral,
	DATENoShowCancel,
	COORDPatients,
	DATEPatients,
	PROCConsultToSurg,
	PROCPatients,
	REFInqtoCons,
	DATEInqToCons,
	REFProsToCons,
	DATEProsToCons,
	COORDProsToCons,
	REFSchedVClosed,
	PROCSchedVClosed,
	COORDSchedVClosed,
	RETENTIONGraph,
	TRENDSGraph,
	ZIPGraph,
	BASELINEGraph,
	COSTS,
	INTERNAL_IncPerCat,
	INTERNAL_IncPerPerson,
	INTERNAL_IncPerClient,
	INTERNAL_OpenPerWeek,
	
};

enum MarketFilterType {
	mftDate,
	mftLocation,
	mftProvider,
};

enum MarketFilter {
	mfUnknown,
	mfFirstContactDate,
	mfChargeDate,
	mfPaymentDate,
	mfApptInputDate,
	mfApptDate,
	mfEffectivenessDate,
	mfConsultDate,
	mfConsultInputDate,
	mfPatientLocation,
	mfTransLocation,
	mfApptLocation,
	mfCostLocation,
	//****These are for the revenue by referral source tab
	mfPatCostLocation,
	mfTransCostLocation,
	mfPatNoCostLocation,
	mfTransNoCostLocation,
	//******************************
	mfPatApptLocation,
	mfNoPatApptLocation,
	mfPatNoApptLocation,
	mfChargeLocation,
	mfPayLocation,
	//*************** Market Baseline Graph
	mfGraphDependant,
	//*********************Provider Filters
	mfPatientProvider,
	mfTransProvider,
	mfApptProvider, //Resource
	mfNoPatApptProvider,
	mfPatNoApptProvider,
	mfChargeProvider,
	mfPayProvider,
	mfDependantProvider,
	mfCostDatePaid,
	//DRT 5/8/2008 - PLID 29966 - Referral date
	mfReferralDate,
};


enum MarketDateOptionType {
	mdotAll,
	mdotOneYear,
	mdotCustom,
	mdotToday,
	mdotThisWeek,
	mdotThisMonth,
	mdotThisQuarter,
	mdotYear,
	mdotThisMTD,
	mdotThisQTD,
	mdotThisYTD,
	mdotLastWeek,
	mdotLastMonth,
	mdotLastQuarter,
	mdotLastYear,
};

//(e.lally 2009-09-24) PLID 35526 - Created enum of color for readability
enum MarketingGraphColors {
	mgcBrightRed=0,
	mgcBrightGreen,
	mgcBrightBlue,
	mgcBrightPurple,
	mgcBrightOrange,
	mgcDarkRed,
	mgcDarkGreen,
	mgcDarkBlue,
	mgcLightRed,
	mgcLightGreen,
	mgcLightBlue,
	mgcLightPurple,
	mgcLightOrange,
	mgcTurquoise,
	mgcTan,
};

OLE_COLOR GetMarketGraphColor(MarketingGraphColors mgcColor);

bool DoesGraphSupportFilter(MarketGraphType graph, MarketFilter filter, MarketFilterType mft = mftDate);

CString GetFieldFromEnum(MarketFilter mf);
CString GetDisplayNameFromEnum(MarketFilter mf);

//TES 6/4/2008 - PLID 30206 - Moved some functions to CMarketingDlg.

int GetType();

#define INVALID_DATETIME "Invalid DateTime."

void Save (CString data);
long ReferralID(const CString &source);
//CString OleToSQL (const COleDateTime &dtTime);
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
void GetDateText(CDateTimePicker &dt, CString &str);
void GetDateText(CDateTimeCtrl &dt, CString &str);
void GetDateTextFrom(CString &str);
void GetDateTextTo(CString &str);
//VARIANT GetDateFrom();
//VARIANT GetDateTo();
ADODB::_RecordsetPtr GetReferralData();
ADODB::_RecordsetPtr GetCategoryData();
void EnsureReferralData(bool force = false);
void EnsureCategoryData(bool force = false);
void ReferralMultiRefresh(CTableChecker **p, long id = -1);
void CategoryMultiRefresh(CTableChecker **p, long id = -1);	//same code, that's why it's here
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
CString GetGraphSql(MarketGraphType mktGraph, long nFlags, long nFilter, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
void GetParameters(CString &from, CString &to, CString &prov, CString &loc,CString &strPatCoord, CString &strDateField, CString &strLocationField, CString &strProvField, int &nCategory, int &nResp, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);


BOOL ParseMultiFilter(CString strCompleteFilter, CString &strFilter1, CString &strFilter2);
BOOL HasMultiFilter(CString strLocationField);

//(e.lally 2009-09-01) PLID 35429 - Used in the Consult to Procedure Conversion Rate base query as placeholders for the
	//consult and surgery appointment type ID list filtering.
#define CONSULT_TYPE_PLACEHOLDER_1	CString("[ConsultTypeList1]")
#define CONSULT_TYPE_PLACEHOLDER_2	CString("[ConsultTypeList2]")
#define SURGERY_TYPE_PLACEHOLDER	CString("[SurgeryTypeList]")
//(e.lally 2009-09-11) PLID 35521 - Used in the Consult to Procedure Conversion Rate base query as a placeholder for the
	//consult appointment procedure filtering.
#define PROCEDURE_FILTER_PLACEHOLDER CString("[ConsultProcedures]")
//(e.lally 2009-09-28) PLID 35594 - Used in the referrals tab for filtering on a set of referral sources
#define PRIMARY_REFERRAL_FILTER_PLACEHOLDER CString("[PatientPrimaryReferralSources]")
#define MULTI_REFERRAL_FILTER_PLACEHOLDER CString("[PatientMultiReferralSources]")
//(e.lally 2009-09-18) PLID 35300 - Used in the marketing referrals tab as the default ID when the patient's referral ID is null
#define NOREFERRAL_SENTINEL_ID	-4

//(e.lally 2009-08-24) PLID 35297 - Returns the SQL string to be used as the base query for the consult to procedure
	//conversion rate graphs and reports. It contains string placeholders for the advanced filtering available 
	//in the marketing module.
//(e.lally 2009-09-24) PLID 35593 - Added paramter for including the multi referrals table
CString GetConsultToProcedureConversionRateBaseSql(BOOL bUseMultiReferrals = FALSE);

//(e.lally 2009-09-01) PLID 35429 - Gets a standardized list of statistics fields using the placeholders for the
	//consult and surgery appointment type ID list filtering.
//(e.lally 2009-09-24) PLID 35593 - Added paramter for using the referral rate fields stats
CString GetConsultToProcedureGraphStatFields(BOOL bUseReferralRateFields = FALSE);

//(e.lally 2009-08-25) PLID 35297 - Applies the filtering parameters to be used by the conversation rate graphs.
	//Eventually all graphs could be migrated to use this function instead
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
void ApplyDocbarGraphFilters(IN OUT CString &strSql, MarketGraphType mktGraph, ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable);
//(e.lally 2009-08-25) PLID 35297 - Used by the PP reports to send in parameters for the filters used.
	//The reports must extract these filters in the same order they are added here
// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
void BuildDocbarFilterArray(OUT CStringArray& saryFilters, MarketGraphType mktGraph, ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable);
//(e.lally 2009-09-08) PLID 35429 - Sets the comma delimited appointment type ID lists for the userdefined
	//New Patient Consults, Established Consults, and Procedure appt types to be used in marketing
void GetUserDefinedConsToProcAptTypeLists(CString& strConsultTypeList1, CString& strConsultTypeList2, CString& strSurgeryTypeList);
// (z.manning 2009-09-08 17:22) - PLID 35051 - Gets the labels for the conversion rate graphs
void GetUserDefinedConsToProcLabels(OUT CString &strCons1Label, OUT CString &strCons2Label, OUT CString &strSurgeryLabel);
// (z.manning 2009-09-08 17:47) - PLID 35051
BOOL IsConsToProcSetupValid();
// (z.manning 2009-09-09 09:40) - PLID 35051 - Moved the logic for setting up consult to procedure graph
// from date dialog here so that other tabs can use it too.
void AddConsToProcDataToGraphDesc(const CString strSummarySql, GraphDescript *pDesc, BOOL bShowNumbers, BOOL bShowPercentages, BOOL bHandlePercentsManually = FALSE);

//(e.lally 2009-08-25) PLID 35297 - Functions to build the filter string for individual placeholders in the sql base queries
//Right now only the conversion rate graphs and reports use these.

// (j.jones 2010-07-19 15:27) - PLID 39053 - require a connection pointer
CString GetDocbarPatientLWFilter(MarketGraphType mktGraph, ADODB::_ConnectionPtr pCon, CString &strPatientTempTable);
CString GetDocbarPatientFilterDateFrom(MarketGraphType mktGraph);
CString GetDocbarPatientFilterDateTo(MarketGraphType mktGraph);
CString GetDocbarPatientLocationFilter(MarketGraphType mktGraph);
CString GetDocbarPatientProviderFilter(MarketGraphType mktGraph);
CString GetDocbarConsultDateFromFilter(MarketGraphType mktGraph);
CString GetDocbarConsultDateToFilter(MarketGraphType mktGraph);
CString GetDocbarConsultLocation(MarketGraphType mktGraph);
CString GetDocbarConsultResource(MarketGraphType mktGraph);
//(e.lally 2009-09-24) PLID 35592 - added multi-referral date filters
CString GetDocbarReferralDateFromFilter(MarketGraphType mktGraph);
CString GetDocbarReferralDateToFilter(MarketGraphType mktGraph);

#endif