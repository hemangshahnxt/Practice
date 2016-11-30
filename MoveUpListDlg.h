#pragma once

// MoveUpListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMoveUpListDlg dialog

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;

// (j.armen 2012-06-06 12:39) - PLID 50830 - Removed GetMinMaxInfo
class CMoveUpListDlg : public CNxDialog
{
// Construction
public:
	CMoveUpListDlg(CWnd* pParent);   // standard constructor
	NXDATALIST2Lib::_DNxDataListPtr  m_pMoveUpList;
	NXDATALIST2Lib::_DNxDataListPtr  m_pMoveUpListSort;
	NXDATALIST2Lib::_DNxDataListPtr  m_pTypeList;
	NXDATALIST2Lib::_DNxDataListPtr  m_pPurposeList;
	NXDATALIST2Lib::_DNxDataListPtr	m_pResourceList;
	NXTIMELib::_DNxTimePtr m_nxtMoveUp;

	// (a.walling 2008-05-13 14:56) - PLID 27591 - Use CDateTimePicker
	CDateTimePicker m_dtcMoveUp;
	CDateTimePicker m_dtcStartDate; // (d.moore 2007-05-14 09:39) - PLID 4013 - 
	CDateTimePicker m_dtcEndDate;   //  Used to filter the start and end dates for the waiting list.

	void SwitchToAppointment();
	BOOL m_bMoveUpRequeryEnabled;
	void SetAppointmentColors();
	void SetMoveUpDateTime(const COleDateTime& dt);
	void SetDateRangeFilter(COleDateTime dtStart, COleDateTime dtEnd);
	void SetResourceFilter(const CDWordArray& adwResourceFilter);
	void EnableRequery(BOOL bEnable);
	// (d.moore 2007-05-11 11:31) - PLID 4013 - Fills the waiting list and sets row colors.
	void Requery();

protected:
	CString m_strResourceFilter;
	
	void QueryLineItemCollection();
	void RemoveFromWaitList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void MoveUpAppointment();
	void TryScheduleNewAppt();

	// (j.jones 2014-08-22 10:53) - PLID 63186 - if the start date filter is today's date,
	// it will always filter as of the current time, not the date
	COleDateTime GetCurStartDate();
	// (j.jones 2014-08-22 10:53) - PLID 63186 - if the end date filter is today's date,
	// it will always filter through the end of the day
	COleDateTime GetCurEndDate();

	CString FormatQueryFilter();

	void OpenResourceMultiList();
	//(e.lally 2009-08-14) PLID 35226 - Used for manually sorting the records since the datalist does not inherently
	//	support the sorting of the tree style.
	void LoadSortOptions();
	void RefreshSortPriority();

public:

	// (z.manning, 04/30/2008) - PLID 29814 - Added NxIconButtons
	//(e.lally 2009-08-13) PLID 28591 - Added ShowApptStart checkbox.
// Dialog Data
	//{{AFX_DATA(CMoveUpListDlg)
	enum { IDD = IDD_SHOW_MOVEUP_LIST };
	CNxLabel	m_nxlMultipleResourceLabel;
	CNxStatic	m_nxstaticMoveupdatetime;
	CNxIconButton	m_btnAddWaitListPatient;
	CNxIconButton	m_btnMoveUp;
	CNxIconButton	m_btnPreviewMoveUpList;
	CNxIconButton	m_btnClose;
	NxButton		m_btnShowInputDate;
	NxButton		m_btnShowApptStart;
	NxButton		m_radioSortAsc;
	NxButton		m_radioSortDesc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoveUpListDlg)	
	protected:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-21 17:19) - PLID 63186 - added an Ex handler
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2009-08-07 09:55) - PLID 11699 - This function will see which columns
	// the user wants visible, and update the appropriate checkboxes
	void LoadVisibleColumnDefaults();
	// (c.haag 2009-08-07 10:02) - PLID 11699 - This function will ensure a single optional column
	// is at its appropriate size.
	void EnsureVisibleColumnWidth(UINT nCheckID, short col, long nWidth);
	// (c.haag 2009-08-07 09:59) - PLID 11699 - This function will update the visible columns to
	// ensure they are the proper sizes.
	void EnsureAllVisibleColumnWidths();

protected:
	// Generated message map functions
	// (a.walling 2008-05-13 14:58) - PLID 27591 - Use notify handlers for datetimepicker events
	//{{AFX_MSG(CMoveUpListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedMoveuplist(short nFlags);
	afx_msg void OnGotoappointment();
	afx_msg void OnMoveUp();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPreviewMoveupList();
	afx_msg void OnAddWaitListPatient();
	afx_msg void OnRButtonDownMoveuplist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnChangeDateStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDateEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenApptTypeList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenApptPurpose(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedApptTypeList(short nFlags);
	afx_msg void OnRequeryFinishedApptPurpose(short nFlags);
	afx_msg void OnSelChosenApptResource(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedApptResource(short nFlags);
	afx_msg void OnSelChangedMoveuplist(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellMoveuplist(LPDISPATCH lpRow, short nColIndex);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnCheckShowInputDate();
	afx_msg void OnCheckShowApptStart();
	afx_msg void OnRadioSortMoveUpListAscending();
	afx_msg void OnRadioSortMoveUpListDescending();
	afx_msg void OnSelChangedMoveupListSort(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChosenMoveupListSort(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};