#if !defined(AFX_EMRPROBLEMLISTDLG_H__3A941DB7_2196_4E7D_ADEE_B36838162A1D__INCLUDED_)
#define AFX_EMRPROBLEMLISTDLG_H__3A941DB7_2196_4E7D_ADEE_B36838162A1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRProblemListDlg.h : header file
//

#include "pracprops.h"
#include "EmrUtils.h"

class CLabRequisitionDlg;

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemListDlg dialog

class CEMRProblemListDlg : public CNxDialog
{
public:
	// (c.haag 2009-02-09 15:09) - PLID 32976 - Structures for data exchanges
	// between the dialog and the thread
	struct ProblemListThreadValue
	{
		// Dialog
		CEMRProblemListDlg* pDlg;
		// Input
		// (j.jones 2009-05-28 14:00) - PLID 34298 - supported problem links
		long nProblemID;
		long nProblemLinkID;
		long nDetailID;
		long nEMNID;
		EmrInfoType DataType;
		// Output
		CString strDetailValue;
	};

// Construction
public:
	CEMRProblemListDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-07-21 15:09) - PLID 30730 - m_bIsOwnedByMainframe tells us
	// if we are modeless or not
	BOOL m_bIsOwnedByMainframe;

	// (j.jones 2008-07-21 15:09) - PLID 30773 - pMessageWnd is the window that
	//will receive messages if problems change, it is often CEMRTopicWnd, or CEMRTreeWnd
	CWnd *m_pMessageWnd;

	// (j.jones 2008-07-15 14:35) - PLID 30731 - added date controls
// Dialog Data
	//{{AFX_DATA(CEMRProblemListDlg)
	enum { IDD = IDD_EMR_PROBLEM_LIST_DLG };
	CDateTimePicker	m_dtTo;
	CDateTimePicker	m_dtFrom;
	NxButton	m_btnShowEMRName;
	NxButton	m_btnShowEMNName;
	NxButton	m_btnShowTopicName;
	NxButton	m_btnShowDetailName;
	NxButton	m_btnShowDetailValue;
	NxButton	m_btnShowProblemDesc;
	NxButton	m_btnShowDiagCode;
	NxButton	m_btnShowChronicity; // (a.walling 2009-05-04 12:16) - PLID 33751
	BOOL	m_bShowProblemDesc;
	BOOL	m_bShowDetailData;
	BOOL	m_bShowDetailName;
	BOOL	m_bShowEMNName;
	BOOL	m_bShowEMRName;
	BOOL	m_bShowTopicName;
	BOOL	m_bShowChronicity;  // (a.walling 2009-05-04 12:16) - PLID 33751
	BOOL	m_bShowDiagCode;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnNewProblem;
	CNxIconButton	m_btnEditProblem;
	CNxIconButton	m_btnDeleteProblem;
	CNxIconButton	m_btnPrintPreview;
	//}}AFX_DATA


	// (j.jones 2008-07-17 09:10) - PLID 30730 - added OnTableChanged
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRProblemListDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	//}}AFX_VIRTUAL

public:
	// (j.jones 2008-07-17 16:18) - PLID 30731 - renamed SetPatientID to SetDefaultFilter,
	// so that we can fill in m_eprtDefaultRegardingType and m_nDefaultRegardingID
	void SetDefaultFilter(long nPatientID, EMRProblemRegardingTypes eprtDefaultRegardingType = eprtInvalid, long nDefaultRegardingID = -1, CString strRegardingDesc = "");

	// (j.jones 2008-07-18 14:35) - PLID 30773 - added ability to pass in an array of problem pointers
	// (c.haag 2009-05-21 12:40) - PLID 34298 - We now pass in problem links
	void LoadFromProblemList(CWnd* pMessageWnd, CArray<CEmrProblemLink*, CEmrProblemLink*> *papEMRProblemLinks);

	// (j.jones 2008-08-20 10:39) - PLID 30773 - added boolean so loading from memory doesn't load twice
	BOOL m_bIsRefilteringFromMemory;

	long GetPatientID();
	EMRProblemRegardingTypes GetRegardingType();

	void SetEMRID();

	void SetLabRequisitionDlg(CLabRequisitionDlg *pdlgLabRequisition); // (z.manning 2009-05-29 09:59) - PLID 34345

	// (z.manning 2009-07-01 15:59) - PLID 34765 - Added flag to make it optional to exclude current user
	// when checking problems against EMN locks.
	BOOL m_bExcludeCurrentUserFromEmnConcurrencyCheck;

	// (z.manning 2009-07-01 17:41) - PLID 34765 - When opening this dialog from the lab entry dialog, we
	// also want to do concurrency checks against EMR level problems (even though locking is done no higher
	// than the EMR level) to be safe.
	BOOL m_bIncludeEmrLevelProblemsInConcurrencyCheck;

protected:
	// (c.haag 2009-02-09 15:26) - PLID 32976 - These should not be public
	NXDATALIST2Lib::_DNxDataListPtr m_dlProblems;
	NXDATALIST2Lib::_DNxDataListPtr m_dlHistory;

protected:
	CNxColor m_nxcTop;

	long m_nPatientID;

	// (j.jones 2008-11-20 16:29) - PLID 32119 - added problem status tablechecker
	CTableChecker m_ProblemStatusChecker;

	void RequeryStatusFilter(BOOL bRefilterList);

	// (j.jones 2008-07-17 16:18) - PLID 30731 - if m_eprtDefaultRegardingType is eprtInvalid,
	// then we are only filtering on a patient, otherwise we are filtering on a specific type
	// and all its children, specified by m_nDefaultRegardingID
	EMRProblemRegardingTypes m_eprtDefaultRegardingType;
	long m_nDefaultRegardingID;
	CString m_strRegardingDesc;

	// (j.jones 2008-07-18 14:37) - PLID 30773 - added array of problem pointers
	// (c.haag 2009-05-21 12:42) - PLID 34298 - We now maintain an array of problem links
	CArray<CEmrProblemLink*, CEmrProblemLink*> *m_papEMRProblemLinks;

	// (z.manning 2009-05-29 09:58) - PLID 34345 - Need when opening this dialog from lab entry
	//TES 11/25/2009 - PLID 36191 - Changed from a CLabEntryDlg to a CLabRequisitionDlg
	CLabRequisitionDlg *m_pdlgLabRequisition;

	CWinThread* m_pPopulateDetailValuesThread;

	// (j.jones 2008-07-15 14:34) - PLID 30731 - added filters
	NXDATALIST2Lib::_DNxDataListPtr m_StatusCombo,
								m_TypeCombo,
								m_DateCombo;

	//dynamically builds the Type combo based on m_eprtDefaultRegardingType
	void BuildTypeCombo();

	// (j.jones 2008-07-16 10:43) - PLID 30731 - This function will filter the list accordingly and requery.
	// bForceRequery is set to TRUE if the caller wants the list to requery even if the filter does not change.
	void RefilterList(BOOL bForceRequery = FALSE);

	// (j.jones 2008-07-18 14:40) - PLID 30773 - RefilterList() will call RefilterListFromMemory()
	// if we have a list of problem pointers, otherwise it will pull from the database
	void RefilterListFromSql(BOOL bForceRequery);
	void RefilterListFromMemory();

	//tries to kill the active PopulateDetailValuesThread thread
	void KillThread();

	void RequeryHistory(long nProblemID);

	void ResizeColumns();

	// (c.haag 2009-02-09 15:04) - PLID 32976 - Utility wrapper for clearing the problem list
	void ClearProblemList();

	// (j.jones 2008-07-17 11:07) - PLID 30730 - used to flash the window when minimized or inactive
	void Flash();

	// (j.jones 2008-07-17 16:28) - PLID 30730 - added function to set the window text
	void UpdateProblemListWindowText();

	// (j.jones 2008-07-18 16:10) - PLID 30773 - Made separate functions for editing
	// a problem from a memory object, or simply from the SQL-generated list
	// (c.haag 2009-05-22 16:50) - PLID 34298 - No need to pass in a problem or problem
	// link object; it's already in the row
	void EditProblemFromMemory(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void EditProblemFromData(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.jones 2008-08-07 11:35) - PLID 30773 - this function will update the EMR appropriately
	// when a problem memory object has been changed
	// (c.haag 2009-05-26 15:51) - PLID 34298 - We now pass in problem links. We no longer need the EMN lock flag;
	// we calculate it in the function now
	// (j.jones 2009-06-05 09:16) - PLID 34487 - moved to EmrUtils
	//void UpdateEMRInterface(CEmrProblemLink *pLink);

	// (e.lally 2008-07-23) PLID 30732 - Pulled out the logic to generate where clauses based on the selected filters
		//into their own functions.
	CString GetWhereClauseFromStatusFilter();
	CString GetWhereClauseFromTypeFilter();
	CString GetWhereClauseFromDateFilter();

	// (j.jones 2014-02-26 14:30) - PLID 60764 - standalone functions for updating diagnosis columns
	void UpdateProblemListDiagnosisColumns();
	void UpdateHistoryListDiagnosisColumns();

	// Generated message map functions
	//{{AFX_MSG(CEMRProblemListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedListEmrProblems(short nFlags);
	afx_msg void OnSelChangedListEmrProblems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnCheckShowProblemDesc();
	afx_msg void OnCheckShowDetailData();
	afx_msg void OnCheckShowDetailName();
	afx_msg void OnCheckShowTopicName();
	afx_msg void OnCheckShowEmnName();
	afx_msg void OnCheckShowEmrName();
	afx_msg void OnDestroy();
	afx_msg void OnBtnNewProblem();
	afx_msg void OnBtnEditProblem();
	afx_msg void OnBtnDeleteProblem();
	afx_msg void OnDblClickCellListEmrProblems(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChosenProblemStatusFilter(LPDISPATCH lpRow);
	afx_msg void OnSelChosenProblemObjectTypeFilter(LPDISPATCH lpRow);
	afx_msg void OnSelChosenProblemDateFilter(LPDISPATCH lpRow);
	afx_msg void OnCloseupProblemFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseupProblemToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeProblemFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeProblemToDate(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPrintPreview();
	afx_msg LRESULT OnEMRProblemListDetailFromThread(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCheckShowDiagnosis();
	afx_msg void OnBnClickedCheckShowChronicity();
	// (j.jones 2014-02-26 14:30) - PLID 60764 - requerying the history may need to show/hide diagnosis columns
	afx_msg void OnRequeryFinishedListEmrProblemHistory(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPROBLEMLISTDLG_H__3A941DB7_2196_4E7D_ADEE_B36838162A1D__INCLUDED_)
