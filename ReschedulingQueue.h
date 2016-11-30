#pragma once

// (a.walling 2014-12-22 14:24) - PLID 64370

// CReschedulingQueue dialog

class CNxOleDataSource;

class CReschedulingQueue 
	: public CNxDialog
{
	DECLARE_DYNAMIC(CReschedulingQueue)

public:
	static bool ShouldDock();

	bool IsDocked() const;

	CReschedulingQueue(CWnd* pParent);   // standard constructor
	virtual ~CReschedulingQueue();

	// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
	void Refresh(long nApptID);
	virtual void Refresh() override 
	{
		Refresh(-1);
	}

	// (a.walling 2015-01-29 13:06) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
	void RefreshEventually(UINT nTimeout);

// Dialog Data
	enum { IDD = IDD_RESCHEDULING_QUEUE };

protected:
	boost::intrusive_ptr<CNxOleDataSource> m_pDataSource;

	// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
	void ResetFilters();
	long m_nPendingApptID = -1;
	bool m_bRefreshOnShow = false;
	

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pListResource;
	NXDATALIST2Lib::_DNxDataListPtr m_pListLocation;
	NXDATALIST2Lib::_DNxDataListPtr m_pListReason;
	NXDATALIST2Lib::_DNxDataListPtr m_pListType;
	NXDATALIST2Lib::_DNxDataListPtr m_pListPurpose;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	// (r.goldschmidt 2015-01-06 10:36) - PLID 64372 - Secure controls based on relevant permissions
	void SecureControls();

	// thunk required due to the way MFC's event maps handle member function pointers for a class with more than one base
	void RequireDataListSel(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
	{
		CNxDialog::RequireDataListSel(lpOldSel, lppNewSel);
	}

	// (b.spivey, January 6th, 2015) PLID 64389 - Remember column widths
	NxButton m_checkRememberColumnWidths; 

	void SaveColumnWidths(NXDATALIST2Lib::_DNxDataListPtr pList, CString strRememberedWidthsConfigRTName);
	void OnColumnSizingFinishedAppointmentList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void UpdateVisibleAppointmentColumns(NXDATALIST2Lib::_DNxDataListPtr pList, CString strRememberedWidthsConfigRTName);

	bool m_bRememberColumns = true; 
	void OnBnClickedRememberColumnWidths(); 

	// (b.spivey, January 7th, 2015) PLID 64397 - Helper functions to select multiple resources, toggle labels, 
	//	handle clicking labels, and changing the cursor icon when scrolling over labels. 
	void MultiSelectResource(); 
	void ToggleLabels(); 
	CArray<long, long> m_naryResourceIDs, m_naryLocationIDs;
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	CNxLabel m_nxstaticMultiResourceLabel, m_nxstaticMultiLocationLabel, m_nxstaticAppointmentsShowLabel;

	void MultiSelectLocations();

	// (b.spivey, January 16, 2015) PLID 64608
	void GoToPatient(long nPatientID);

	// (b.spivey, January 8th, 2015) PLID 64390 
	HANDLE m_hIconNotes, m_hIconHasNotes;

	// (b.spivey, January 9th, 2015) PLID 64390 - need to be able to open up the notes dialog and add more notes
	afx_msg void OnLeftClickAppointmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	// (b.spivey, January 12, 2015) PLID 64395 - 
	afx_msg void OnShowContextMenuAppointmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, VARIANT_BOOL* pbContinue);

	// (b.spivey, January 13, 2015) PLID 64402 - update functions to keep the label and list up to date.
	void UpdateAppointmentList();
	void UpdateAppointmentsShownLabel();

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog() override;
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRescheduleAppointments();
	afx_msg void OnBnClickedToggleDock(); // (a.walling 2015-01-12 16:42) - PLID 64570 - Toggle dock

	// (a.walling 2015-01-29 13:06) - PLID 64586 - Rescheduling Queue - Refresh, table checkers
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_EVENTSINK_MAP()
	void SelChosenListResource(LPDISPATCH lpRow);
	void SelChosenListLocation(LPDISPATCH lpRow);
	void SelChosenListReason(LPDISPATCH lpRow);
	void SelChosenListType(LPDISPATCH lpRow);
	void SelChosenListPurpose(LPDISPATCH lpRow);
	void DragInitListAppointments(BOOL* pbCancel, LPDISPATCH lpRow, short nCol, long nFlags);
	void OnDblClickCellAppointmentsList(LPDISPATCH lpRow, short nColIndex); // (a.walling 2015-01-26 14:06) - PLID 64686
	//(s.dhole 6/4/2015 2:21 PM ) - PLID 65638
	void LoadResourceIDStringIntoArray(CString strResourceIDs, CDWordArray& adwIDs);
	bool LoadFindFirstAvailableAppt(long nAppointmentID);
};
