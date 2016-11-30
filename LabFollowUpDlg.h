#if !defined(AFX_LABFOLLOWUPDLG_H__3647D732_0B0A_4D45_BAD1_69AA20505D42__INCLUDED_)
#define AFX_LABFOLLOWUPDLG_H__3647D732_0B0A_4D45_BAD1_69AA20505D42__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LabFollowUpDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLabFollowUpDlg dialog

class CLabFollowUpDlg : public CNxDialog
{
// Construction
public:
	CLabFollowUpDlg(CWnd* pParent);   // standard constructor

	// (j.dinatale 2011-01-05) - PLID 41818 
	~CLabFollowUpDlg();

	bool HasDataChanged(); // returns true if there have been any modifications to the data

	// (r.galicki 2008-10-28 10:50) - PLID 27214 - Added specimen label controls
	// (c.haag 2010-09-09 16:03) - PLID 40461 - Added discontinued toggle
// Dialog Data
	//{{AFX_DATA(CLabFollowUpDlg)
	enum { IDD = IDD_LAB_FOLLOW_UP_DLG };
	CNxIconButton	m_nxibOK;
	NxButton	m_nxbRememberColumns;
	CNxIconButton m_nxibShowAll;
	NxButton		m_chkSpecimenLabel;
	CNxIconButton m_nxibPrintLabels;
	// (j.jones 2010-04-16 16:42) - PLID 37875 - added option to auto-open the next lab
	NxButton	m_checkAutoOpenNextLab;
	// (j.gruber 2010-06-08 12:36) - PLID 36844 - added option to only show labs without results
	NxButton m_chkLabNoResults;
	NxButton	m_checkDiscontinuedLabs;
	// (j.gruber 2010-12-22 14:55) - PLID 38740 - added radio buttons 
	NxButton m_rdShowByReq;
	NxButton m_rdShowByResult;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabFollowUpDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pLabList;
	NXDATALIST2Lib::_DNxDataListPtr m_pProv;
	NXDATALIST2Lib::_DNxDataListPtr m_pProc;
	NXDATALIST2Lib::_DNxDataListPtr m_pStep;

	void ReloadList();

	// (a.walling 2006-07-12 15:57) - PLID 21073 Save and restore column settings
	void SaveColumns();
	void RestoreColumns();
	void GotoPatient(long nPatID); // switch to the patient in the mainframe
	
	// this will highlight labs which have all steps complete but are not marked complete
	// and alternate coloring for consecutive labs
	void ColorLabList();

	// (j.dinatale 2011-01-05) - PLID 41818 - sets up the note icon on the list for each row
	void SetupNoteIcons();

	// (j.dinatale 2011-01-05) - PLID 41818 - sets up the note icon for a given row.
	void SetRowNoteIcon(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bHasNotes);

	// (j.dinatale 2011-01-05) - PLID 41818 - the "Has Notes" red icon
	HICON m_hNotes;

	// (r.galicki 2008-11-06 11:39) - PLID 27214 - Minimize lab followup (when previewing specimen label)
	void Minimize();

	// (z.manning 2010-01-07 14:14) - PLID 36796
	//(r.farnworth 2013-03-25) PLID 51115 - Changed to bool to check if a requery was needed
	// (b.savon 2013-09-06 13:00) - PLID 58426 - Added bUseMultiSelectDlg
	bool HandleSelChosenListProcedure(LPDISPATCH lpRow, BOOL bUseMultiSelectDlg = TRUE);
	void ConcatenateWhere(NXDATALIST2Lib::IRowSettingsPtr pRow, int ColumnID, CStringArray &ListIDs, CString TableName, CString &strWhere);

	bool m_bDataChanged; // true if data has changed at all
	COLORREF colorHighlight, colorStandard;

	// (a.walling 2008-07-31 10:33) - PLID 25755 - Added variable to keep track of last selected ProviderID,
	// and also a button to refresh and a better way to handle inactive providers that disappear from the list
	// due to their labs being completed.
	long m_nLastProviderID;

	// (r.farnworth 2013-03-25) - PLID 51115 -  Keep track of the last selected StepID and ProcedureID
	long m_nLastStepID;
	long m_nLastProcedureID;

	CVariantArray m_varProcIDs; //(r.farnworth 2013-03-21) - PLID 51115
	CVariantArray m_varStepIDs; //(r.farnworth 2013-03-21) - PLID 51115
	CStringArray m_ProcIDs; //(r.farnworth 2013-03-19) - PLID 51115
	CStringArray m_StepIDs; //(r.farnworth 2013-03-19) - PLID 51115

	CVariantArray m_varProvIDs;	// (v.maida 2016-04-13 12:58) - NX-100191
	std::vector<long> m_vecProvIDs;	// (v.maida 2016-04-13 12:58) - NX-100191
	
	CString m_StepFilter;   //(r.farnworth 2013-03-19) - PLID 51115

	// (b.savon 2013-09-06 12:36) - PLID 58426 - In Labs Needing Attention, the lab procedure filter doesn't remember per user
	void PopulateProcedureDefaults();
	void GetProcedureWhereClause(CString &strWhere);
	void SaveProcedureDefaults();

	// (j.jones 2015-08-27 14:30) - PLID 57837 - we now remember multiple steps per user
	void PopulateStepDefaults();
	void SaveStepDefaults();

	// (v.maida 2016-04-14 9:00) - NX-100192 - Remember multiple providers.
	void PopulateProviderDefaults();
	void SaveProviderDefaults();

	// Generated message map functions
	//{{AFX_MSG(CLabFollowUpDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnRememberColumnSettings();
	afx_msg void OnRequeryFinishedLabList(short nFlags);
	afx_msg void OnRButtonDownLabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellLabList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnGotopatient();
	afx_msg void OnLabsMarkcomplete();
	afx_msg void OnLabsMarkstepcomplete();
	afx_msg void OnLabsOpenlab();
	afx_msg void OnLeftClickLabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnChangeColumnSortFinishedLabList(short nOldSortCol, BOOL bOldSortAscending, short nNewSortCol, BOOL bNewSortAscending);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelChosenListProviders(LPDISPATCH lpRow);
	afx_msg void OnSelChosenListStep(LPDISPATCH lpRow);
	afx_msg void OnSelChosenListProcedure(LPDISPATCH lpRow);
	afx_msg void OnLabsShowall();
	afx_msg void OnBtnRefresh();
	afx_msg void OnTrySetSelFinishedListProviders(long nRowEnum, long nFlags);
	afx_msg void OnSelChangingListProviders(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingListProcedure(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingListStep(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	virtual void PostNcDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCheckSpecimen();
	afx_msg void OnBnClickedPrintSpecimen();
	// (j.jones 2010-04-16 16:42) - PLID 37875 - added option to auto-open the next lab
	afx_msg void OnCheckAutoOpenNextLab();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	// (j.gruber 2010-06-08 12:36) - PLID 36844
	afx_msg void OnBnClickedCheckLabsNoResults();
	afx_msg LRESULT OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedCheckShowDiscontinued();
	afx_msg void OnBnClickedLfuRequisition(); // (j.gruber 2010-12-23 12:30) - PLID 38740
	afx_msg void OnBnClickedLfuResult(); // (j.gruber 2010-12-23 12:30) - PLID 38740
	afx_msg void OnColumnClickingLabList(short nCol, BOOL* bAllowSort); //TES 8/1/2013 - PLID 57823
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABFOLLOWUPDLG_H__3647D732_0B0A_4D45_BAD1_69AA20505D42__INCLUDED_)
