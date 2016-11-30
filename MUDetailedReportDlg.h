// MUDetailedReportDlg.h : header file
//

// (j.dinatale 2012-10-24 09:49) - PLID 53502 - Created.
#pragma once

// (r.gonet 06/12/2013) - PLID 55151 - Removed an unneeded header include.
#include "MUCalculator.h"

typedef std::map<MU::DataPointType, short> DataTypeMap;
typedef std::map<MU::MeasureNames, DataTypeMap> MeasureMap;
typedef std::pair<MU::DataPointType, short> DataTypeMapValue;
typedef std::pair<MU::MeasureNames, DataTypeMap> MeasureMapValue;

/////////////////////////////////////////////////////////////////////////////
// CMUDetailedReportDlg dialog

class CMUDetailedReportDlg : public CNxDialog
{
// Construction
public:
	
	CMUDetailedReportDlg(CWnd* pParent);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CMUDetailedReportDlg)
	enum { IDD = IDD_MU_DETAILED_RPT_DLG };
	
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMUDetailedReportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support		
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	NXDATALIST2Lib::_DNxDataListPtr m_pReport;	
	NXDATALIST2Lib::_DNxDataListPtr m_pSummary;	
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;
	NXDATALIST2Lib::_DNxDataListPtr m_pProviders;
	NXDATALIST2Lib::_DNxDataListPtr m_pSeenLocationList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSeenProviders;

	MU::CCalculator m_muCalculator;

	CString m_strCurrProvName;
	bool m_bShowProvCol;
	bool m_bDialogVisible;

	CNxStatic m_nxlProgressLabel;

	CNxLabel m_nxlLocationLabel;
	CNxLabel m_nxlLocationSeenLabel;
	CNxLabel m_nxlProviderLabel;

	CNxIconButton m_btnClose;
	CNxIconButton m_btnLoad;
	CNxIconButton m_btnExclusions;	// (j.dinatale 2012-10-24 12:58) - PLID 53497
	CNxIconButton m_btnExport;
	CNxIconButton m_btnLeft; // (v.maida - 2014-04-28 10:54) - PLID 61904
	CNxIconButton m_btnRight; // (v.maida - 2014-04-28 10:54) - PLID 61904

	CDateTimePicker m_SeenFrom;
	CDateTimePicker m_SeenTo;
	CDateTimePicker m_MUFrom;
	CDateTimePicker m_MUTo;

	CDWordArray m_dwSeenLocIDList;
	long m_nCurrentSeenLocationID;	
	CString m_strSeenLocationList;
	BOOL SelectMultiSeenLocations();

	CDWordArray m_dwLocIDList;
	long m_nCurrentLocationID;	
	CString m_strLocationList;
	BOOL SelectMultiLocations();

	CDWordArray m_dwProvIDList;
	long m_nCurrentProvID;	
	CString m_strProviderList;
	BOOL SelectMultiProviders();

	void EnableSeenFilter(BOOL bEnable);
	void EnableMUFilter(BOOL bEnable);
	void EnableUtilityButtons(BOOL bEnable);

	CString GetNamesFromIDString(CString strIDs, NXDATALIST2Lib::_DNxDataListPtr pList, short nIDCol, short nNameCol, std::vector<std::pair<long, CString>> & IDsAndNames = std::vector<std::pair<long, CString>>());
	MeasureMap SetUpLists(std::vector<MU::MeasureData> MeasureDatum);

	/// <summary>
	/// Sets the internal Meaningful Use stage indicator to the passed in stage, and clears the measures datalists.
	/// </summary>
	/// <param name="eStage">The stage enum to set the internal stage indicator to.</param>
	void HandleStageChange(MU::Stage eStage);
	/// <summary>
	/// Clears out the measures-related (main measures list and summary) datalists.
	/// </summary>
	void ClearMeasuresDatalists();
	/// <summary>
	/// Resets the internal MU calculator.
	/// </summary>
	void ResetMUCalculator();
	
	// Generated message map functions
	//{{AFX_MSG(CMUDetailedReportDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedMuLocationList(short nFlags);
	void SelChangingMuLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenMuLocationList(LPDISPATCH lpRow);

	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedMuDetPreview();	// (j.dinatale 2012-10-24 14:27) - PLID 53498 - allow for our detailed list to be exported
	afx_msg void OnBnClickedLoad();
	afx_msg LRESULT OnMeasurePreload(WPARAM wParam, LPARAM lParam);	// (j.dinatale 2012-10-24 14:22) - PLID 53508
	afx_msg LRESULT OnAllMeasuresComplete(WPARAM wParam, LPARAM lParam);	// (j.dinatale 2012-10-24 14:22) - PLID 53494
	afx_msg LRESULT OnMeasureComplete(WPARAM wParam, LPARAM lParam);	// (j.dinatale 2012-10-24 14:22) - PLID 53508
	afx_msg LRESULT OnMeasureLoadCancel(WPARAM wParam, LPARAM lParam);	// (j.dinatale 2012-10-24 14:22) - PLID 53508
	void RequeryFinishedMuDtlProviderList(short nFlags);
	void RequeryFinishedMuDtlSeenProviderList(short nFlags);
	void RequeryFinishedMuDtlLocationList(short nFlags);
	void SelChosenMuDtlLocationList(LPDISPATCH lpRow);
	void SelChangingMuDtlLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingMuDtlProviderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingMuDtlSeenProviderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenMuDtlSeenProviderList(LPDISPATCH lpRow);
	void SelChosenMuDtlProviderList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedMuDtlDisableSeenFilter();
	afx_msg void OnBnClickedMuDtlExclusions();	// (j.dinatale 2012-10-24 12:58) - PLID 53497
	afx_msg void OnBnClickedOk();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRadioDstage1();
	afx_msg void OnBnClickedRadioDstage2();	
	void OnRButtonDownReportList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedMoveLeft();
	afx_msg void OnBnClickedMoveRight();
	afx_msg void OnBnClickedRadioModDstage2();
};

