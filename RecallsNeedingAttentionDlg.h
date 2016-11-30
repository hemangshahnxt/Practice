#pragma once

// CRecallsNeedingAttentionDlg

// (j.armen 2012-02-24 16:20) - PLID 48303 - Created

class CRecallsNeedingAttentionDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRecallsNeedingAttentionDlg)

public:
	BOOL m_bUsePatientID;					// (j.armen 2012-02-28 14:53) - PLID 48452
	CArray<long, long> m_aryProviderIDs;	// (j.armen 2012-02-28 15:55) - PLID 48460
	CArray<long, long> m_aryTemplateIDs;	// (j.armen 2012-02-28 16:06) - PLID 48462
	CArray<long, long> m_aryLocationIDs;	// (j.armen 2012-03-01 09:35) - PLID 48483
	CArray<long, long> m_aryStatusIDs;		// (j.armen 2012-03-01 09:36) - PLID 48546
	COleDateTime m_dtStart;					// (j.armen 2012-02-28 16:30) - PLID 48463
	COleDateTime m_dtEnd;					// (j.armen 2012-02-28 16:30) - PLID 48463

	CRecallsNeedingAttentionDlg(CWnd* pParent);
	virtual ~CRecallsNeedingAttentionDlg();

	enum { IDD = IDD_RECALLS_NEEDING_ATTENTION_DLG };

// Public Functions
public:
	void DoFilter();

	// (z.manning 2015-11-05 12:30) - PLID 57109
	void SetNeedsRefresh();

// Protected Functions
protected:
	// (a.walling 2014-01-27 14:58) - PLID 59993 - Update UI with multiple selections etc
	void UpdateDatalistSelectionUI(NXDATALIST2Lib::_DNxDataListPtr pList, short colID, short colName, CNxLabel& label, UINT labelID, const CArray<long, long>& arIDs);
	void DlgCtrlMultiSelEnable(NXDATALIST2Lib::_DNxDataListPtr& pdl2, UINT dl2ID, CNxLabel& nxl, bool bEnable = true);
	void UpdateFilter();
	void ShowSelectMultiProvider();	// (j.armen 2012-02-28 16:00) - PLID 48460
	void ShowSelectMultiTemplate();	// (j.armen 2012-02-28 16:06) - PLID 48462
	void ShowSelectMultiLocation();	// (j.armen 2012-03-01 09:36) - PLID 48483
	void ShowSelectMultiStatus();	// (j.armen 2012-03-01 09:36) - PLID 48546
	void GotoPatient(long nPatID);
	void Minimize();
	bool GoToMonthTab(const COleDateTime& dtDate);
	void Merge();	// (b.savon 2012-03-02 16:41) - PLID 48474 - Add ability to Merge To Word from Recall Dlg
	void SetRowNoteIcon(NXDATALIST2Lib::IRowSettingsPtr& pRow, BOOL bHasNotes);	// (j.armen 2012-03-19 09:08) - PLID 48780

	// (b.savon 2012-03-08 14:33) - PLID 48716 - Remember column widths
	void RestoreColumnWidths();
	void SaveColumnWidths();

	// (z.manning 2015-11-05 13:49) - PLID 57109
	void RequeryRecallList();

// Protected Dialog Controls
protected:
	CNxColor m_nxcRecallBkg;
	NxButton m_btnCurrentPat;
	NxButton m_btnShowDiscontinued;
	NxButton m_btnShowCompleted;	// (j.armen 2012-03-09 16:40) - PLID 48774
	CNxLabel m_nxlProviders;		// (j.armen 2012-03-12 09:42) - PLID 48460
	CNxLabel m_nxlTemplates;		// (j.armen 2012-03-12 09:41) - PLID 48462
	CNxLabel m_nxlLocations;		// (j.armen 2012-03-12 09:41) - PLID 48483
	CNxLabel m_nxlStatus;			// (j.armen 2012-03-09 16:44) - PLID 48546
	NXDATALIST2Lib::_DNxDataListPtr m_pdlProviderList;	// (j.armen 2012-03-12 09:41) - PLID 48460
	NXDATALIST2Lib::_DNxDataListPtr m_pdlTemplateList;	// (j.armen 2012-03-09 17:55) - PLID 48462
	NXDATALIST2Lib::_DNxDataListPtr m_pdlLocationList;	// (j.armen 2012-03-12 09:41) - PLID 48483
	NXDATALIST2Lib::_DNxDataListPtr m_pdlStatusList;	// (j.armen 2012-03-09 16:44) - PLID 48546

	// (b.savon 2012-03-19 09:01) - PLID 48716
	NxButton m_btnRememberColumns;

	// (b.savon 2014-09-02 13:16) - PLID 62791 - Add Patient Reminder
	NxButton m_btnAddPatientReminder;

	// (b.savon 2012-03-02 16:41) - PLID 48474 - Add ability to Merge To Word from Recall Dlg
	CNxIconButton m_btnMergeToWord;
	CNxIconButton m_btnCreateRecall;
	CNxIconButton m_btnCreateMergeGroup;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlRecallList;
	CDateTimeCtrl m_cdtStart;
	CDateTimeCtrl m_cdtEnd;
	CNxIconButton m_btnClose;

	enum eSpecialID {
		eIDAll = -1,
		eIDMultiple = -2,
	};

	HICON m_hNotes;	// (j.armen 2012-03-19 09:08) - PLID 48780

	// (z.manning 2015-11-05 12:30) - PLID 57109
	BOOL m_bNeedsRefresh;

// Protected Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Private Message Map
private:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDtnDatetimechangeRecallDaterange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedRecallCurrentPat();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnScheduler();
	afx_msg void OnFFA();
	afx_msg void OnSwapDiscontinueStatus();	// (j.armen 2012-02-28 15:28) - PLID 48457
	afx_msg void OnLinkExistingAppointment();  // (b.savon 2012-03-01 12:32) - PLID 48486
	afx_msg void OnUnlinkExistingAppointment(); // (b.savon 2012-03-06 10:55) - PLID 48635
	afx_msg void OnAppointment();
	afx_msg void OnBnClickedBtnMergeToWord();
	afx_msg void OnBnClickedBtnCreatemergegroup();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClose();

// Private Event Sink Map
private:
	DECLARE_EVENTSINK_MAP()
	void SelChosenRecallProviders(LPDISPATCH lpRow);	// (j.armen 2012-02-28 16:02) - PLID 48460
	void SelChosenRecallTemplates(LPDISPATCH lpRow);	// (j.armen 2012-02-28 16:27) - PLID 48462
	void SelChosenRecallLocations(LPDISPATCH lpRow);	// (j.armen 2012-03-01 09:42) - PLID 48483
	void SelChosenRecallStatus(LPDISPATCH lpRow);		// (j.armen 2012-03-01 09:42) - PLID 48546
	void OnLeftClickRecallList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnRightClickRecallList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);	
	afx_msg void OnBnClickedAddPtReminderRecall();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};