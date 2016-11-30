#include "stdafx.h"
#include "MUMeasureFilter.h"

// (j.dinatale 2012-10-24 11:32) - PLID 53503 - Measure Filter Object which can be used to generate sql filters.

MUMeasureFilter::MUMeasureFilter(void)
{
	m_bUseDates = false;
	m_bExcludeSecondaries = FALSE;

	//for lack of better option, default to today
	m_dtFromDate = COleDateTime::GetCurrentTime();
	m_dtToDate = COleDateTime::GetCurrentTime();

	//Start both blank
	m_aryLocationList.RemoveAll();
	m_aryProviderList.RemoveAll();
}

MUMeasureFilter::MUMeasureFilter(const MUMeasureFilter &src)
{
	m_bUseDates = src.m_bUseDates;
	m_dtFromDate = src.m_dtFromDate;
	m_dtToDate = src.m_dtToDate;

	m_aryLocationList.RemoveAll();
	m_aryLocationList.Append(src.m_aryLocationList);

	m_aryProviderList.RemoveAll();
	m_aryProviderList.Append(src.m_aryProviderList);

	m_strExcludedTemplateIDs = src.m_strExcludedTemplateIDs;
	m_bExcludeSecondaries = src.m_bExcludeSecondaries;
}

MUMeasureFilter::~MUMeasureFilter(void)
{
}

//Are any of the possible filters in use?
// bEMRFilter is used to determine we are using this as an EMR filter or not
bool MUMeasureFilter::AreAnyActive(bool bEMRFilter /*= false*/)
{
	return m_bUseDates || (m_aryLocationList.GetSize() > 0) || (m_aryProviderList.GetSize() > 0) || (bEMRFilter && !m_strExcludedTemplateIDs.IsEmpty());
}

//ASSUMES
// - emrmastert
// - emrproviderst
// - emrsecondaryproviderst
CSqlFragment MUMeasureFilter::GenerateEMRFilter_ForPatientBaseMeasures(BOOL bIncludeDate /*=TRUE*/, BOOL bIncludeProvider /*=TRUE*/, BOOL bIncludeLocation /*=TRUE*/)
{
	CSqlFragment filter;

	//TODO:  Ensure this matches 100% with CCCHITReportInfo::ApplyFilters()
	if(AreAnyActive(true)) {
		if(bIncludeDate){
			filter += GenerateEMRDateFilter();
		}

		if(bIncludeLocation){
			filter += GenerateEMRLocationFilter();
		}
		
		if (bIncludeProvider) {
			filter += GenerateEMRProviderFilter();
		}

		//add the exclusions
		filter += GenerateEMRTemplateExclusions();
	}
	else {
		//No active filters, so we don't need to query a thing
	}

	return filter;
}

CSqlFragment MUMeasureFilter::GenerateEMRTemplateExclusions()
{
	CString strExcludedFilter = "";	

	if (!m_strExcludedTemplateIDs.IsEmpty()) {
		strExcludedFilter.Format(" AND (EMRMasterT.TemplateID IS NULL OR EMRMasterT.TemplateID NOT IN %s)", m_strExcludedTemplateIDs);
	}

	return CSqlFragment(strExcludedFilter);
}

CSqlFragment MUMeasureFilter::GenerateDateFilter(const CString &strDateField)
{
	CSqlFragment filter;

	if(m_bUseDates) {
		filter += CSqlFragment(
			"AND {CONST_STR} >= dbo.AsDateNoTime({OLEDATETIME}) AND "
			"{CONST_STR} < DATEADD(dd, 1, dbo.AsDateNoTime({OLEDATETIME})) ", 
			strDateField, m_dtFromDate, strDateField, m_dtToDate);
	}

	return filter;
}

CSqlFragment MUMeasureFilter::GenerateLocationFilter(const CString &strLocField)
{
	CSqlFragment filter;

	if(m_aryLocationList.GetCount() > 0) {
		filter += CSqlFragment("AND {CONST_STR} IN ({INTARRAY}) ", strLocField, m_aryLocationList);
	}

	return filter;
}

CSqlFragment MUMeasureFilter::GenerateProviderFilter(const CString &strProvField)
{
	CSqlFragment filter;

	if(m_aryProviderList.GetCount() > 0) {
		filter += CSqlFragment("AND {CONST_STR} IN ({INTARRAY}) ", strProvField, m_aryProviderList);
	}

	return filter;
}

CSqlFragment MUMeasureFilter::GenerateEMRDateFilter()
{
	return GenerateDateFilter("EMRMasterT.Date");
}

CSqlFragment MUMeasureFilter::GenerateEMRLocationFilter()
{
	return GenerateLocationFilter("EMRMasterT.LocationID");
}

CSqlFragment MUMeasureFilter::GenerateEMRProviderFilter()
{
	CSqlFragment filter;

	if(m_aryProviderList.GetSize() > 0) {
		//TODO:  Ensure this matches 100% with CCCHITReportInfo::ApplyFilters()			
		if (m_bExcludeSecondaries) {
			filter += CSqlFragment(" AND (EMRMasterT.ID IN (SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN ({INTARRAY}) AND EMRProvidersT.DELETED = 0) "
				" AND EMRMasterT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE EMRSecondaryProvidersT.DELETED = 0))", m_aryProviderList);
		}
		else {
			filter += CSqlFragment("AND EMRMasterT.ID IN "
				"(SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN ({INTARRAY}) AND EMRProvidersT.Deleted = 0 "
				"UNION SELECT EMRID FROM EMRSecondaryProvidersT WHERE ProviderID IN ({INTARRAY}) AND EMRSecondaryProvidersT.Deleted = 0) ", m_aryProviderList, m_aryProviderList);
		}
	} else {
		if (m_bExcludeSecondaries) {
			filter += CSqlFragment(" AND (EMRMasterT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE EMRSecondaryProvidersT.DELETED = 0)) ");
		}
	}

	return filter;
}

// (d.singleton 2014-09-05 10:59) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
CSqlFragment MUMeasureFilter::GenerateEMRInnerProviderFilter()
{
	CSqlFragment filter;

	if (m_aryProviderList.GetSize() > 0) {
		//TODO:  Ensure this matches 100% with CCCHITReportInfo::ApplyFilters()			
		if (m_bExcludeSecondaries) {
			filter += CSqlFragment(" AND (EMRInnerT.ID IN (SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN ({INTARRAY}) AND EMRProvidersT.DELETED = 0) "
				" AND EMRInnerT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE EMRSecondaryProvidersT.DELETED = 0))", m_aryProviderList);
		}
		else {
			filter += CSqlFragment("AND EMRInnerT.ID IN "
				"(SELECT EMRID FROM EMRProvidersT WHERE ProviderID IN ({INTARRAY}) AND EMRProvidersT.Deleted = 0 "
				"UNION SELECT EMRID FROM EMRSecondaryProvidersT WHERE ProviderID IN ({INTARRAY}) AND EMRSecondaryProvidersT.Deleted = 0) ", m_aryProviderList, m_aryProviderList);
		}
	}
	else {
		if (m_bExcludeSecondaries) {
			filter += CSqlFragment(" AND (EMRInnerT.ID NOT IN (SELECT EMRID FROM EMRSecondaryProvidersT WHERE EMRSecondaryProvidersT.DELETED = 0)) ");
		}
	}

	return filter;
}

// this returns a filter based on the fields passed in
CSqlFragment MUMeasureFilter::GenerateFieldFilter(const CString &strDateField, const CString &strProviderField, const CString &strLocationField)
{
	CSqlFragment filter;

	if(AreAnyActive()) {
		filter += GenerateDateFilter(strDateField);

		filter += GenerateLocationFilter(strLocationField);

		filter += GenerateProviderFilter(strProviderField);
	}
	else {
		//No active filters, so we don't need to query a thing
	}

	return filter;
}