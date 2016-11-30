#pragma once

// ToDoAlarmDlg.h : header file
//

#include "client.h"
#include "GlobalTodoUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CToDoAlarmDlg dialog

// (j.armen 2012-05-30 11:50) - PLID 49854 - Removed OnPaint handler for drag handler - now handled by CNexTechDialog
// (j.armen 2012-06-06 12:38) - PLID 50830 - Removed GetMinMaxInfo

class CToDoAlarmDlg : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_List;
	NXDATALISTLib::_DNxDataListPtr m_Combo;
	NXDATALISTLib::_DNxDataListPtr m_CategoryFilter;
//	long GetTimerInterval();//int nRemindOption = -1);
	CToDoAlarmDlg(CWnd* pParent);   // standard constructor
	CTableChecker m_tblCheckTask, m_UserChecker;
	CScrollBar m_sizegrip;
	RECT m_rPrev;

	BOOL SelectTodoListUser(long nUserID);

	// (a.walling 2007-05-04 09:53) - PLID 4850 - Initialize the variables (to allow for re-initializing the window when switching users)
	void Init();

	// (j.jones 2014-08-12 11:49) - PLID 63187 - cache if we need a requery upon OnShowWindow
	bool m_bNeedsRequery;

	// (j.jones 2008-09-30 11:17) - PLID 31331 - added controls for the patient appt. filter options
// Dialog Data
	//{{AFX_DATA(CToDoAlarmDlg)
	enum { IDD = IDD_TODO_ALARM_DLG };
	NxButton	m_btnPatientFilterGroup;
	NxButton	m_radioApptsMarkedIn;
	NxButton	m_radioTodaysPatients;
	NxButton	m_radioAllPatients;
	NxButton	m_btnIncomplete;
	NxButton	m_btnComplete;
	NxButton	m_btnAll;
	NxButton	m_btnHigh;
	NxButton	m_btnMedium;
	NxButton	m_btnLow;
	NxButton	m_btnRemember;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnPrintPreview;
	CSpinButtonCtrl	m_hourSpin;
	NxButton m_minuteBtn;
	NxButton m_hourBtn;
	NxButton m_neverBtn;
	CSpinButtonCtrl	m_minuteSpin;
	CDateTimePicker	m_DateTo;
	CDateTimePicker	m_DateFrom;
	CNxEdit	m_nxeditOtherTime;
	CNxEdit	m_nxeditOtherHTime;
	CNxStatic	m_nxstaticLabelDate;
	NxButton	m_btnRemindMeGroup;
	NxButton	m_btnStatusGroupbox;
	NxButton	m_btnPriorityGroupbox;
	NxButton	m_btnStartRemindGroupbox;
	//}}AFX_DATA


	// (j.jones 2008-09-30 14:24) - PLID 31331 - added OnTableChangedEx
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToDoAlarmDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

public:
	void Save();

	//(a.walling 2006-08-14 10:08) - PLID 21755
	void RecolorList(); //refreshes the coloring of the list, does not requery

// Variables used for auto-sizing the dialog controls
protected:
	long m_nListWidthAdj;
	long m_nListHeightAdj;
	long m_nOkayXOffsetFromRight;
	long m_nOkayYOffsetFromTop;
	long m_nTitleLabelWidth;
	long m_nTitleLabelYFromTop;

	long m_bDateDrop;

// Implementation
protected:
	CString GetCategoryFilterText();
	// (z.manning 2008-11-21 10:47) - PLID 31893 - Added a parameter for tab. If -1 the user's default is used.
	void LoadPatientScreen(long nID, long nTab = -1);
	void LoadContactScreen(long nID);
	// (c.haag 2008-02-07 13:34) - PLID 28853 - Included a parameter for consignment
	void LoadInventoryScreen(long nID, BOOL bConsignment);
	void LoadPersonScreen(long nRow);
	void LoadTodoListScreen(int nID);
	void LoadExportScreen(long nExportID);
	//(e.lally 2010-05-06) PLID 36567
	// (z.manning 2010-05-12 16:28) - PLID 37405 - Added regarding type
	void LoadLabEntryScreen(long nPatientID, TodoType eRegardingType, long nRegardingID);
	//int GetTodoType(long nPersonID, long nRow = 0);
	void SetColumnSizes();
	void ResetColumnSizes();

	// (z.manning 2008-11-21 10:25) - PLID 31893 - Function to handle clicking on links for EMR to-dos
	void HandleEmrTodoLink(const TodoType eType, const long nRegardingID, const long nPersonID);
	// (c.haag 2010-05-24 9:48) - PLID 38731 - Handling for MailSent tasks
	void HandleMailSentTodoLink(const long nRegardingID, const long nPersonID);

	CString m_strWhere;
	bool GenerateWhereClause(); // return false if where clause is the same as before.
	void ColorizeList(OPTIONAL long nRow = -1); // colorize the todo list accordingly.
	void ColorizeItem(NXDATALISTLib::IRowSettingsPtr &pRow); // colorize a single item
	COLORREF m_colorComplete;
	COLORREF m_colorIncompleteHigh;
	COLORREF m_colorIncompleteMedium;
	COLORREF m_colorIncompleteLow;
	bool m_bListColored; // true if the list has already been coloured
	bool RefreshColors(); // return true if colors have changed
	void UpdateColorField(); // sets BackColor field to a SQL Case statement that returns the appropriate backcolor of the row

	long m_rightClicked;
	CString m_strOriginalColSizes;

	// (j.jones 2008-09-30 14:34) - PLID 31331 - added TryUpdateTodoListByAppointmentID, which will
	// see if the patient associated with the appointment has any active todos that need to be displayed
	// in the list, given that the appt. qualifies in the current filter setup. If so, it will ensure
	// the todos are displayed, either manually or through a requery.
	void TryUpdateTodoListByAppointmentID(long nAppointmentID);

	//(c.copits 2011-02-18) PLID 40794 - Permissions for individual todo alarm fields
	BOOL CheckIndividualPermissions(short nCol);

	// (j.jones 2014-08-12 11:53) - PLID 63187 - unified duplicated code into one function
	void TryRequeryList();

	// (j.jones 2014-09-02 09:32) - PLID 63187 - unified todo tablechecker code into one function,
	// pDetails will be null for non-EX tablecheckers
	void HandleTodoTablechecker(long nTaskID, CTableCheckerDetails* pDetails);

	// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - OnNcHitTest should return an LRESULT
	// (j.jones 2008-09-30 11:17) - PLID 31331 - added functions for the patient appt. filter options
	// Generated message map functions
	//{{AFX_MSG(CToDoAlarmDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeMinuteSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDownList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnChangeHourSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClickCellList(long nRowIndex, short nColIndex);
	afx_msg void OnClickRadioMinute();
	afx_msg void OnClickRadioHour();
	afx_msg void OnClickRadioDontremind();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDestroy();
	afx_msg void OnRequeryFinishedList(short nFlags);
	afx_msg void OnSelChosenCombo(long nRow);
	afx_msg void OnBtnPrintTodos();
	afx_msg void OnRememberColumnSettings();
	afx_msg void OnColumnSizingFinishedList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnCheckTodoHigh();
	afx_msg void OnCheckTodoLow();
	afx_msg void OnCheckTodoMedium();
	afx_msg void OnRadioTodoAll();
	afx_msg void OnRadioTodoComplete();
	afx_msg void OnRadioTodoIncomplete();
	afx_msg void OnChangeTodoDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTodoDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownTodoDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpTodoDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownTodoDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpTodoDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadioTodoAllPatients();
	afx_msg void OnRadioTodoApptsMarkedIn();
	afx_msg void OnRadioTodoTodaysAppts();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SelChosenTodoCategoryFilter(long nRow);
	void SelChangingTodoCategoryFilter(long* nNewSel);
};
