#pragma once


// CEMRAnalysisDlg dialog

// (j.jones 2008-10-14 16:29) - PLID 14411 - created

#include "EmrUtils.h"

// (c.haag 2009-02-24 17:55) - PLID 33187 - Limit to the number of dynamic columns in the
// analysis result list
#define MAX_ANALYSIS_DYNAMIC_COLUMNS		200

// (a.wilson 2013-05-14 15:11) - PLID 55963 - add EMRFilterTextData for contains operator.
//used to cache entries from EMRAnalysisConfigDetailsT
struct CachedConfigDetail {

	long nActiveEMRInfoID;
	CString strEMRInfoName;
	EmrInfoType eDataType;
	long nEMRInfoMasterID;
	EMRAnalysisDataOperatorType eOperator;
	long nEMRDataGroupID;
	long nEMRDataID;
	CString strEMRDataName;
	long nListType;
	CString strTextFilterData;
};

//used to cache a list of EMR details that potentially match our search
struct MatchingEMRDetails {

	long nPatientID;
	long nUserDefinedID;
	CString strPatientName;
	CString strEMRDesc;
	CString strEMNDesc;
	long nEMRID;
	long nEMNID;
	long nPicID;
	COleDateTime dtEMNDate;
	long nEMRInfoMasterID;
	EmrInfoType eDataType;
	long nEMRDetailID;
	BOOL bHasData;	//this is only used in FilterCandidateDetails
	CString strSentenceFormat;
	// (j.jones 2009-04-09 14:46) - PLID 33916 - added spawning information, separate from the sentence format
	CString strSpawningInformation;
	CString strName;	// (c.haag 2009-02-24 16:11) - PLID 33187 - The name of the detail
	// (j.jones 2009-04-08 14:32) - PLID 33915 - added provider & secondary provider
	CString strProviderNames;
	CString strSecondaryProviderNames;
	// (j.jones 2009-04-09 17:13) - PLID 33916 - added SourceDetailID
	long nSourceDetailID;
};

// (b.savon 2011-11-01 08:56) - PLID 43205 - The EMR Analysis should be able to export individual 
// table rows as their own cells
struct DynColumns
{
	CString strHeader;
	CString strData;
	short nColumn;
	long nPatientID;
	long nEMNID;
	long nEMRID;
	COleDateTime dtEMNDate;
};

class CEMRAnalysisDlg : public CNxDialog
{

public:
	CEMRAnalysisDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_EMR_ANALYSIS_DLG };
	CNxColor	m_bkg;
	// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnEdit;
	CNxIconButton	m_btnDelete;
	NxButton	m_radioAllDates;
	NxButton	m_radioEMNDate;
	NxButton	m_radioGroupByPatient;
	NxButton	m_radioGroupByDate;
	NxButton	m_radioGroupByEMN;
	NxButton	m_radioGroupByEMR;
	CDateTimePicker	m_dtTo;
	CDateTimePicker	m_dtFrom;
	CNxIconButton	m_btnReloadResults;
	CNxIconButton	m_btnCancelSearch;
	CProgressCtrl	m_progress;
	CNxEdit			m_nxeditProgressStatus;	
	NxButton	m_radioColumnGroupByItem;
	NxButton	m_radioColumnGroupByCondensed;
	// (j.jones 2009-04-08 14:39) - PLID 33915 - added checkboxes to show/hide columns
	NxButton	m_checkShowProviderColumn;
	NxButton	m_checkShowSecondaryProviderColumn;
	NxButton	m_checkShowEMNDescColumn;
	NxButton	m_checkShowEMRDescColumn;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr	m_ConfigCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_ResultsList;

	HICON m_hIconPreview; // (a.walling 2010-01-11 12:11) - PLID 31482
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;

	// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
	void ShowPreview(long nPatID, long nEMNID);

	long m_nCurrentConfigID;

	//this function will disable everything but the cancel button when loading,
	//and enable everything but the cancel button when not loading
	void ToggleInterface();
	//this function will re-enable the interface if the user cancelled
	void ResetInterfaceOnCancel();
	BOOL m_bIsLoading;

	void DisableAllFilters();
	void ClearResultsList();

	void UpdateColumnWidths();

	void ClearCachedConfigDetails(CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails);
	void ClearMatchingEMRDetails(CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails);

	void LoadResults();
	//this function will load the information from EMRAnalysisConfigDetailsT into aryCachedConfigDetails
	void LoadConfigDetailInfo(long nEMRAnalysisConfigID, CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails);
	//this function will load all details from data that potentially match our filter - it will not be able to
	//100% match "has data" filters on narratives and tables, nor check the "filter on all items" setting,
	//but will load all details that potentially may be in our final resultset
	// (j.jones 2009-03-27 11:05) - PLID 33703 - added nFilterByTemplateID
	void LoadAllCandidateDetails(BOOL bUseDateFilter, COleDateTime dtFrom, COleDateTime dtTo, long nFilterByTemplateID,
		CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails,
		CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails);
	//this function will take in existing matching details and load an EMN for each one,
	//filling in strSentenceFormat and bHasData
	// (j.jones 2009-04-09 14:45) - PLID 33916 - passed in bIncludeSpawningInfo
	// (b.savon 2011-11-01 15:12) - PLID 43205 - passed in arydcTableColumns
	void FillCandidateDetailsWithEMNData(CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails, 
										 BOOL bIncludeSpawningInfo, 
										 CArray<DynColumns*, DynColumns*> &arydcTableColumns);
	//this function will take in existing matching details that have already had their EMN data loaded,
	//re-check all the "Has Data" statuses when requested, and check the bFilterOnAllItems status
	void FinalFilterCandidateDetails(BOOL bFilterOnAllItems, EMRAnalysisRecordFilterType eRecordFilter,
		CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails,
		CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails);
	// (c.haag 2009-02-24 13:19) - PLID 33187 - Returns the first non-populated column of a row in m_ResultsList.
	// If none is available, the column will equal the number of columns in the datalist.
	short CalculateFirstAvailableDynamicColumn(NXDATALIST2Lib::IRowSettingsPtr& pRow);
	//this function will take in the final detail list and display them on the result list,
	//based on the group by setting
	// (c.haag 2009-02-24 16:41) - PLID 33187 - Added eColumnGroupBy
	void AddDetailsToResultList(EMRAnalysisGroupByType eGroupBy, EMRAnalysisColumnGroupByType eColumnGroupBy,
		CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails,
		CArray<MatchingEMRDetails*, MatchingEMRDetails*> &aryMatchingEMRDetails);
	//this function will determine which datalist column represents the info item referenced in the passed-in detail
	short CalcResultColumnIndexForDetail(MatchingEMRDetails *pMatchingDetail, CArray<CachedConfigDetail*, CachedConfigDetail*> &aryCachedConfigDetails);

	//TrySaveFilterChanges will attempt to save our current filters to the selected configuration
	//after prompting, if those filters changed. Only returns FALSE if the user cancels or gets an error.
	BOOL TrySaveFilterChanges(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void MinimizeWindow();

	// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
	CString CalcResultsetAsCsv();
	CString CalcExportTextValue(IN const CString &strValue);

	// (j.jones 2009-04-08 17:43) - PLID 33915 - added ability to show/hide certain columns
	void ToggleColumns();

	// (j.jones 2009-04-09 16:59) - PLID 33916 - fill the spawning information in pDetail->strSpawningInformation,
	// by finding the sentence format for the spawning item, and prepending it to strSpawningInformation
	// (a.walling 2010-06-02 13:31) - PLID 39007
	void LoadSpawningInformation(MatchingEMRDetails *pMatchingDetail, long nSpawningDetailID, CEMN* &pEMN);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnBtnEmrAnalysisClose();
	afx_msg void OnBtnAddEmrAnalysisConfig();
	afx_msg void OnBtnEditEmrAnalysisConfig();
	afx_msg void OnBtnDeleteEmrAnalysisConfig();	
	afx_msg void OnSelChosenEmrAnalysisConfigCombo(LPDISPATCH lpRow);
	afx_msg void OnLeftClickEmrAnalysisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedRadioAllEmnDatesAnalysis();
	afx_msg void OnBnClickedRadioEmnDateAnalysis();
	afx_msg void OnBnClickedRadioGroupByPatientAnalysis();
	afx_msg void OnBnClickedRadioGroupByDateAnalysis();
	afx_msg void OnBnClickedRadioGroupByEmnAnalysis();
	afx_msg void OnBnClickedRadioGroupByEmrAnalysis();
	afx_msg void OnDtnDatetimechangeEmnFromDateAnalysis(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeEmnToDateAnalysis(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnReloadResults();
	afx_msg void OnSelChangingEmrAnalysisConfigCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedBtnCancelSearch();
	// (j.jones 2008-10-22 16:17) - PLID 31790 - added ability to export as csv
	afx_msg void OnBtnEmrAnalysisExport();
	// (j.jones 2009-04-08 16:14) - PLID 33915 - added checkboxes to show/hide columns
	afx_msg void OnCheckShowProviderName();
	afx_msg void OnCheckShowSecProviderName();
	afx_msg void OnCheckShowEmnDescription();
	afx_msg void OnShowEmrDescription();
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	afx_msg void OnDestroy();
};
