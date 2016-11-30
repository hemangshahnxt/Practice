// (d.thompson 2010-01-18) - PLID 36927 - Created
#pragma once
#include "ReportsRc.h"
#include "MUMeasureBase.h"
// (r.gonet 06/12/2013) - PLID 55151 - Removed a header include and added a forward declaration.
class CCCHITReportInfo;

// CCCHITReportsDlg dialog
class CCCHITReportsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCCHITReportsDlg)

public:
	CCCHITReportsDlg(CWnd* pParent);   // standard constructor
	virtual ~CCCHITReportsDlg();

// Dialog Data
	enum { IDD = IDD_CCHIT_REPORTS_DLG };

private:
	CWinThread* m_pThread;
	HANDLE m_hStopThread;

protected:
	//Interface / overrides
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();

	//Internal functions
	void LoadAllReportData();
	void LoadReportsToInterface();
	void EnsureColumns();

	// (j.gruber 2011-11-02 09:55) - PLID 46222
	void CCCHITReportsDlg::KillThread();

	//Members
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	// (j.gruber 2011-05-13 11:58) - PLID 43694
	NXDATALIST2Lib::_DNxDataListPtr m_pCCHITList;
	CArray<CCCHITReportInfo, CCCHITReportInfo&> m_aryReports;
	CNxIconButton m_btnClose;
	CDateTimePicker m_to;
	CDateTimePicker m_from;
	NxButton	m_btnConfigOptions;
	CNxIconButton m_nxbOpenPqri; //TES 9/7/2012 - PLID 52515

// (j.gruber 2011-05-13 11:58) - PLID 43676
	CString m_strProviderList;
	CString GetProviderNamesFromIDString(CString strIDs);
	CNxLabel	m_nxlProviderLabel;
	CDWordArray m_dwProvIDList;
	long m_nCurrentProviderID;
	CString m_strProviderFilterString;	
	BOOL SelectMultiProviders();
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderList;	


	// (j.gruber 2011-05-18 10:36) - PLID 43758
	CString m_strLocationList;
	CString GetLocationNamesFromIDString(CString strIDs);
	CNxLabel	m_nxlLocationLabel;
	CDWordArray m_dwLocIDList;
	long m_nCurrentLocationID;	
	BOOL SelectMultiLocations();
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;
	// (r.farnworth 2013-10-14 15:33) - PLID 58995 - used to determine the MU Stage
	MU::Stage m_eMeaningfulUseStage;

	// (j.gruber 2011-05-13 13:26) - PLID 43695
	void PositionControls();

	// (j.gruber 2011-11-03 16:49) - PLID 44993 - variable for if we are loading	
	bool m_bDoneLoading;
	CString m_strLastProvsRun;
	CString m_strLastLocsRun;
	COleDateTime m_dtLastDateToRun;
	COleDateTime m_dtLastDateFromRun;

	// (j.gruber 2011-11-09 11:00) - PLID 45689 - need more
	CString m_strLastRunExclusionsList;
	BOOL m_bLastRunExcludeSecondaries;
	
	CString m_strExclusionsNameList;
	CString m_strLastRunExclusionsNameList;

	// (j.gruber 2011-11-07 11:23) - PLID 45365 - exclusions
	CString m_strExclusionsList;
	CDWordArray m_dwExcludedTemplateIDs;
	void LoadExclusions();
	CNxIconButton m_btnExclusions;
	

	// (j.gruber 2011-11-02 13:49) - PLID 46222	
	long m_nTotalMeasures;
	long m_nCurrentMeasureCount;
	void CloseWindow();
	BOOL m_bDialogClosing;

	// (j.gruber 2011-11-16 11:44) - PLID 45689
	CString GetTemplateNamesFromIDString(CString strIDs);

	// (b.savon 2014-05-14 08:58) - PLID 62140 - Rename MUS1 Core 12 to “[OBSOLETE] Electronic Copy of their Health Information”. Move the measure to the very bottom of the list.
	void AddRowToInterface(CCCHITReportInfo &riReport, bool bAtTheEnd, OLE_COLOR color);
	/// <summary>
	/// Sets the internal Meaningful Use stage indicator to the passed in stage, and clears the measures list if loading has completed.
	/// </summary>
	/// <param name="eStage">The stage enum to set the internal stage indicator to.</param>
	void HandleStageChange(MU::Stage eStage);

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void LeftClickReportsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.gruber 2011-05-13 11:58) - PLID 43694
	void LeftClickCCHITReportsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedCchitLoad();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedCchitShowConfig();
	// (j.gruber 2011-11-03 16:46) - PLID 44993
	afx_msg void OnBnClickedMUPreview();
	// (j.gruber 2011-11-04 16:39) - PLID 45365
	afx_msg void OnBnClickedCCHITExclusions();

// (j.gruber 2011-05-13 11:58) - PLID 43676
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void RequeryFinishedProviderList(short nFlags);
	afx_msg void CCCHITReportsDlg::OnCancel();


	// (j.gruber 2011-11-08 11:40) - PLID 45689
	void PreviewReport(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol);
	long FindIndexFromName(CString strInternalMeasureName);

	// (j.gruber 2011-05-18 10:37) - PLID 43758
	afx_msg void RequeryFinishedLocationList(short nFlags);
	void SelChosenCchitLocationList(LPDISPATCH lpRow);
	// (j.gruber 2011-09-27 10:02) - PLID 45616 - added checkbox
	void PositionLists(long nListID, long nLabelID, long nMultiSelectBoxID, long nPlaceHolderID, long nCheckBoxID = -1);

	// (j.gruber 2011-11-01 13:02) - PLID 46222
	afx_msg LRESULT OnProcessingAddRow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProcessingFinished(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);


// (j.gruber 2011-05-13 11:58) - PLID 43676
	void SelChosenCchitProvList(LPDISPATCH lpRow);
	afx_msg void OnOpenPqri();
	afx_msg void OnBnClickedRadioStage1();
	afx_msg void OnBnClickedRadioStage2();
	afx_msg void OnBnClickedRadioModStage2();
};
