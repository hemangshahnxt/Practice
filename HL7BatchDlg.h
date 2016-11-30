#if !defined(AFX_HL7BATCHDLG_H__3ED5F884_1EF7_4D28_9197_C55F1414EAC6__INCLUDED_)
#define AFX_HL7BATCHDLG_H__3ED5F884_1EF7_4D28_9197_C55F1414EAC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7BatchDlg.h : header file
//

// (j.jones 2008-04-08 15:28) - PLID 29587 - created

#include "Client.h"

/////////////////////////////////////////////////////////////////////////////
// CHL7BatchDlg dialog

class CHL7BatchDlg : public CNxDialog
{
// Construction
public:
	CHL7BatchDlg(CWnd* pParent);   // standard constructor
	~CHL7BatchDlg();

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	//{{AFX_DATA(CHL7BatchDlg)
	enum { IDD = IDD_HL7_BATCH_DLG };
	CNxIconButton	m_btnExportPatients;
	NxButton	m_nxbShowProcessed;
	CNxIconButton	m_btnConfigSettings;
	CNxIconButton	m_btnCheckForNewMessages;
	CNxIconButton	m_btnUnselectOneImport;
	CNxIconButton	m_btnUnselectOneExport;
	CNxIconButton	m_btnSelectOneImport;
	CNxIconButton	m_btnSelectOneExport;
	CNxIconButton	m_btnUnselectAllImport;
	CNxIconButton	m_btnUnselectAllExport;
	CNxIconButton	m_btnSelectAllImport;
	CNxIconButton	m_btnSelectAllExport;
	CNxIconButton	m_btnImport;
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnExportAppts;
	CNxIconButton	m_btnExportEmnBills;
	CNxIconButton	m_btnExportSyndromic; // (a.walling 2010-02-22 10:45) - PLID 37154
	CNxIconButton	m_btnExportEMNs;
	//}}AFX_DATA

	//TES 6/24/2009 - PLID 34283 - Set this if you are popping up the dialog modally.
	bool m_bIsModal;
	// (s.tullis 2015-06-02 16:09) - PLID 66211
	BOOL m_bEditedDataListLength;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7BatchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CTableChecker m_HL7SettingsChecker;

	// (c.haag 2010-08-11 09:55) - PLID 39799
	HANDLE  m_hIconRedX;
	
	// (j.jones 2008-05-19 16:11) - PLID 30110 - added member tablecheckers
	// so we do not receive our own tablechecker sends
	CTableChecker m_HL7MessageQueueTChecker, m_HL7MessageLogTChecker;

	// (r.gonet 12/11/2012) - PLID 54114 - Client managing HL7 sends between Practice and NxServer
	// (z.manning 2013-05-20 11:17) - PLID 56777 - Renamed to CHL7Client_Practice
	CHL7Client_Practice *m_pHL7Client;

	NXDATALIST2Lib::_DNxDataListPtr m_ExportUnselectedList,
									m_ExportSelectedList,
									m_ImportUnselectedList,
									m_ImportSelectedList;

	NXDATALIST2Lib::_DNxDataListPtr m_ImportTypeFilterCombo,
									m_ExportGroupFilterCombo;

	void RequeryImportFilterTypes();
	void RequeryImportLists();
	void RequeryExportLists();
	

	// (j.jones 2008-04-11 14:55) - PLID 29596 - will process all files for all HL7 groups,
	// bSilent will suppress all messages if TRUE
	void ProcessHL7Files(BOOL bSilent);

	// (j.jones 2008-04-11 14:59) - PLID 29596 - if TRUE, will try to silently process any pending
	// HL7 files, but not actually commit to data, only populate our unselected list
	BOOL m_bAutoProcessHL7Files;

	//set after right-clicking an import list, if TRUE they right clicked the import selected list,
	//if FALSE they right-clicked the import unselected list
	// (z.manning 2011-07-08 15:49) - PLID 38753 - Just keep track of the last right clicked datalist pointer
	NXDATALIST2Lib::_DNxDataListPtr m_pdlLastRightClickedHL7List;
	BOOL m_bLastRightClickListWasImport;
	void SetLastRightClickList(NXDATALIST2Lib::_DNxDataListPtr pdl, BOOL bImportList);

	// (j.jones 2008-04-17 16:11) - PLID 29701 - this function will update the given imported message ID on the screen
	void ProcessChangedHL7MessageQueueID(long nID);

	// (j.jones 2008-04-29 14:49) - PLID 29813 - this function will update the given message log ID on the screen
	void ProcessChangedHL7MessageLogID(long nID);

	// (j.jones 2008-04-17 16:20) - PLID 29701 - these functions will update the filter list based on the message & event type
	void TryAddImportFilter(CString strMessageType, CString strEventType);
	void TryRemoveImportFilter(CString strMessageType, CString strEventType);

	

	// (z.manning 2011-07-08 16:36) - PLID 38753
	void ShowExportListMenu();

	// (j.jones 2008-04-11 14:53) - PLID 29596 - added OnBtnCheckForMessages
	// Generated message map functions
	// (j.jones 2008-04-17 15:41) - PLID 29701 - added OnTableChanged, so we can
	// catch any incoming HL7 messages
	// (z.manning 2008-07-18 14:38) - PLID 30782 - Added a button to export appointments
	//{{AFX_MSG(CHL7BatchDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectOneHl7Export();
	afx_msg void OnSelectAllHl7Export();
	afx_msg void OnUnselectOneHl7Export();
	afx_msg void OnUnselectAllHl7Export();
	afx_msg void OnSelectOneHl7Import();
	afx_msg void OnSelectAllHl7Import();
	afx_msg void OnUnselectOneHl7Import();
	afx_msg void OnUnselectAllHl7Import();
	afx_msg void OnBtnHl7Export();
	afx_msg void OnBtnExportHL7Click();// (a.vengrofski 2010-05-11 10:08) - PLID <38547> - This will handle the context menu.
	afx_msg void OnBtnHl7Import();
	afx_msg void OnDblClickCellHl7ImportUnselectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellHl7ImportSelectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellHl7ExportUnselectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellHl7ExportSelectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnCheckForMessages();
	afx_msg void OnSelChosenImportTypeFilter(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownHl7ImportUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownHl7ImportSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnImportCommit();
	afx_msg void OnImportDismiss();
	// (r.gonet 02/26/2013) - PLID 47534 - Handles the context menu option to dismiss a message pending export.
	afx_msg void OnExportDismiss();
	afx_msg void OnBtnHl7Settings();
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowProcessed();
	afx_msg void OnSelChosenHl7ExportGroupFilter(LPDISPATCH lpRow);
	afx_msg void OnBtnExportPatients();
	afx_msg void OnBtnExportRefPhys();
	afx_msg void OnBtnExportAppts();
	afx_msg void OnBnClickedBtnExportEMNs();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnExportEmnBills();
	afx_msg void OnBtnExportSyndromicData(); // (a.walling 2010-02-22 10:46) - PLID 37154
	void LeftClickHl7ExportUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickHl7ExportSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnViewHL7Message();
	void RButtonDownHl7ExportUnselectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RButtonDownHl7ExportSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (s.tullis 2015-06-02 16:09) - PLID 66211
	CString GetDefaultColumnLengthsImport();
	CString GetDefaultColumnLengthsExport();
	void LoadDatalistColumnLength();
	void SaveDatalistColumnLength();
	void PutDataListColumnWidth(NXDATALIST2Lib::_DNxDataListPtr pdl, std::vector<long> arrWidths);
	std::vector<long> GetDatalistColumnIntLengths(CString strDatalistColumnWidths);
	std::vector<CString> GetDatalistColumnLengths(CString strDatalistColumnWidths);
	CString GetstrDataColumnWidths(NXDATALIST2Lib::_DNxDataListPtr pdl);
	void ColumnSizingFinishedHl7List(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void FocusLostHl7List();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7BATCHDLG_H__3ED5F884_1EF7_4D28_9197_C55F1414EAC6__INCLUDED_)
