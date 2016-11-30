#if !defined(AFX_EXPORTDLG_H__D78BFDD0_6041_4344_845E_9575174668D9__INCLUDED_)
#define AFX_EXPORTDLG_H__D78BFDD0_6041_4344_845E_9575174668D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

enum StoredExportColumn {
	secID = 0,
	secName = 1,
	secLastExportDate = 2,
	secBasedOn = 3,
	secFilterType = 4,
	secIncludeFieldNames = 5,
	secManualSort = 6,
	secAllowOtherTemplates = 7,
	secExtraEmnFilter = 8,
	secFilterFlags, // (z.manning 2009-12-14 12:32) - PLID 36576
};

class CExportDlg : public CNxDialog
{
// Construction
public:
	CExportDlg(CWnd* pParent);   // standard constructor

	void LoadExport(long nExportID);

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (j.jones 2008-05-08 09:15) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
	CNxIconButton	m_btnExport;
	CNxIconButton	m_btnASCExport;
	// (r.gonet 09/27/2011) - PLID 45717
	CNxIconButton	m_btnThirdPartyExports;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnEdit;
	CNxIconButton	m_btnNew;
	CNxIconButton	m_nxbDown;
	CNxIconButton	m_nxbUp;
	CNxIconButton	m_btnRemoveOne;
	CNxIconButton	m_btnRemoveAll;
	CNxIconButton	m_btnAddAll;
	CNxIconButton	m_btnAddOne;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pStoredExportList, m_pAvail, m_pSelect, m_pDateFilters;

	CString GetDateFilterWhereClause();

	CString GetCurrentExtraEmnFilter();
	CString GetCurrentExtraHistoryFilter();

	CList<long, long> m_lExpectedTableCheckers;

	void HandleSelChangedStoredExportList(long nNewSel, BOOL bReloadDateFilters); // (z.manning 2009-12-15 14:30) - PLID 36576

	// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	afx_msg void OnNewExport();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditExport();
	afx_msg void OnDeleteExport();
	afx_msg void OnSelChangedStoredExportList(long nNewSel);
	afx_msg void OnSelChosenExportDateFilterOptions(long nRow);
	afx_msg void OnChangeExportDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeExportDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddAllRecords();
	afx_msg void OnAddOneRecord();
	afx_msg void OnRemoveAllRecords();
	afx_msg void OnRemoveOneRecord();
	afx_msg void OnSelChangedAvailableRecords(long nNewSel);
	afx_msg void OnDblClickCellAvailableRecords(long nRowIndex, short nColIndex);
	afx_msg void OnSelChangedRecordsToBeExported(long nNewSel);
	afx_msg void OnDblClickCellRecordsToBeExported(long nRowIndex, short nColIndex);
	afx_msg void OnExport();
	afx_msg void OnManualSortUp();
	afx_msg void OnManualSortDown();
	afx_msg void OnDblClickCellStoredExportList(long nRowIndex, short nColIndex);
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnSystemExport();
	afx_msg void OnAHCAExport();
	// (j.jones 2007-07-03 09:08) - PLID 25493 - created to support the VA ASC export
	afx_msg void OnVHIExport();
	// (c.haag 2014-09-10) - PLID 63612 - Perform a KY IPOP export
	afx_msg void OnKYIPOPExport();
	// (b.spivey, December 18th, 2014) - PLID 64158 - Illinois IDPH Export
	afx_msg void OnILIDPHExport();
	// (b.cardillo 2007-02-16 14:56) - PLID 24791 - Added RequeryFinished event handler.  See implementation comments for more info.
	afx_msg void OnRequeryFinishedAvailableRecords(short nFlags);
	// (b.spivey, January 07, 2013) - PLID 54538 - For launching the PASRA dialog. 
	afx_msg void OnPASRAExport(); 
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (a.walling 2010-10-05 13:12) - PLID 40822 - Temporarily lift patient export restrictions
	afx_msg void OnBnClickedExportLiftPatientRestrictions();
	// (r.gonet 09/27/2011) - PLID 45717
	afx_msg void OnBnClickedBtnThirdPartyExports();
	// (r.gonet 09/27/2011) - PLID 45717
	afx_msg void OnLabCorpInsuranceExport();
	// (j.dinatale 2013-01-14 12:11) - PLID 54602 - export ref phys for hl7
	afx_msg void OnExportRefPhys();
	// (z.manning 2015-05-13 12:02) - PLID 66048 - Added UT, OR, and PA exports
	afx_msg void OnUTDHExport();
	afx_msg void OnOHPRExport();
	afx_msg void OnPHC4Export();
	// (s.tullis 2015-05-15 14:23) - PLID 65996 
	afx_msg void OnOKDHExport();
	// (z.manning 2015-11-23 09:54) - PLID 67569
	afx_msg void OnNYASCExport();
	afx_msg void OnTNASTCExport(); // (a.walling 2016-01-20 16:43) - PLID 68013 - Tennessee / TNASTC exporter
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__D78BFDD0_6041_4344_845E_9575174668D9__INCLUDED_)
