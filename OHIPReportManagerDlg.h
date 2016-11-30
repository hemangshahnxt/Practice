#pragma once

// COHIPReportManagerDlg dialog

// (j.jones 2008-12-17 08:59) - PLID 31900 - created

class COHIPReportManagerDlg : public CNxDialog
{

public:
	COHIPReportManagerDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2009-03-10 10:40) - PLID 33419 - m_bAutoScanForReports determines
	// if we scan for new reports upon creation
	BOOL m_bAutoScanForReports;

	// (j.jones 2008-12-17 10:09) - PLID 32488 - added m_btnScanReports
// Dialog Data
	enum { IDD = IDD_OHIP_REPORT_MANAGER_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnScanReports;
	CNxIconButton	m_btnBrowseReportFolder;
	CNxEdit			m_nxeditReportFolder;
	NxButton		m_radioAllDates;
	NxButton		m_radioDateRange;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COHIPReportManagerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_ReportHistoryList;

	// (j.jones 2008-12-17 12:39) - PLID 32488 - added ReconcileFileNames, which will
	// take in an array of filenames found in the scanned folder, compare them with
	// tracked data (updating paths when necessary), and reduce the array of filenames
	// down to only the filenames not currently tracked
	void ReconcileFileNames(CStringArray &arystrFileNames, CString strFolderPath);
	// (j.jones 2008-12-17 14:52) - PLID 32488 - used to parse data from the OHIP files
	CString ParseElement(CString strLine, long nStart, long nLength, BOOL bDoNotTrim = FALSE);

	void RefilterList();

	// (j.jones 2009-03-10 10:40) - PLID 33419 - ScanForReports is called by
	//OnBtnScanReports and if m_bAutoScanReports is true, it should have its
	//path validated before being called
	void ScanForReports();

	// (j.jones 2008-12-17 10:09) - PLID 32488 - added OnBtnScanReports
	// (j.jones 2009-03-10 10:47) - PLID 33419 - added OnAutoScanReports and OnShowWindow
	// Generated message map functions
	//{{AFX_MSG(COHIPReportManagerDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();	
	afx_msg void OnLeftClickOhipReportHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBtnScanReports();
	afx_msg void OnBtnBrowseReports();
	afx_msg void OnRadioOhipRmAllDates();
	afx_msg void OnRadioOhipRmDateRange();
	afx_msg void OnCloseupOhipRmFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCloseupOhipRmToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDatetimechangeOhipRmFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDatetimechangeOhipRmToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnAutoScanReports(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()	
};