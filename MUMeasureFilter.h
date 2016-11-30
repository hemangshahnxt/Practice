#pragma once
// (j.dinatale 2012-10-24 11:32) - PLID 53503 - Measure Filter Object which can be used to generate sql filters.

class MUMeasureFilter
{
public:
	MUMeasureFilter(void);
	MUMeasureFilter(const MUMeasureFilter &src);
	~MUMeasureFilter(void);

	bool AreAnyActive(bool bEMRFilter = false);
	CSqlFragment GenerateEMRFilter_ForPatientBaseMeasures(BOOL bIncludeDate = TRUE, BOOL bIncludeProvider = TRUE, BOOL bIncludeLocation = TRUE);
	CSqlFragment GenerateFieldFilter(const CString &strDateField, const CString &strProviderField, const CString &strLocationField);
	CSqlFragment GenerateDateFilter(const CString &strDateField);
	CSqlFragment GenerateLocationFilter(const CString &strLocField);
	CSqlFragment GenerateProviderFilter(const CString &strProvField);

	CSqlFragment GenerateEMRDateFilter();
	CSqlFragment GenerateEMRLocationFilter();
	CSqlFragment GenerateEMRProviderFilter();
	// (d.singleton 2014-09-05 10:59) - PLID 63457 - MU2 Core 12 (Reminders) - Bills not filtering on Provider - Detailed
	CSqlFragment GenerateEMRInnerProviderFilter();
	CSqlFragment GenerateEMRTemplateExclusions();

	void operator=(const MUMeasureFilter &source){
		m_bUseDates = source.m_bUseDates;
		m_dtFromDate = source.m_dtFromDate;
		m_dtToDate = source.m_dtToDate;
		
		m_aryLocationList.RemoveAll();
		m_aryLocationList.Append(source.m_aryLocationList);

		m_aryProviderList.RemoveAll();
		m_aryProviderList.Append(source.m_aryProviderList);

		m_strExcludedTemplateIDs = source.m_strExcludedTemplateIDs;
		m_bExcludeSecondaries = source.m_bExcludeSecondaries;
	}

	//If false, use all dates
	bool m_bUseDates;

	//Otherwise, use these dates
	COleDateTime m_dtFromDate;
	COleDateTime m_dtToDate;

	//If no elements, use all providers
	CDWordArray m_aryLocationList;

	//If no elements, use all providers
	CDWordArray m_aryProviderList;

	CString m_strExcludedTemplateIDs;
	BOOL m_bExcludeSecondaries;

};
