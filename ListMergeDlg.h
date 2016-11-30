#if !defined(AFX_LISTMERGEDLG_H__1F091846_F4A5_4A14_AC57_CF2795B29E83__INCLUDED_)
#define AFX_LISTMERGEDLG_H__1F091846_F4A5_4A14_AC57_CF2795B29E83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ManageInsContactsDlg.h : header file
//

	//(e.lally 2011-07-07) PLID 31585
	enum MergeListType
	{
		mltInvalid,
		mltInsuranceCompanies,
		mltNoteCategories,
		// (j.jones 2012-04-10 16:39) - PLID 44174 - added ability to merge resources
		mltSchedulerResources,
		mltProviders, // (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
		mltLocations, // (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
	};

/////////////////////////////////////////////////////////////////////////////
// CManageInsContactsDlg dialog

class CListMergeDlg : public CNxDialog
{
// Construction
public:
	CListMergeDlg(CWnd* pParent);   // standard constructor

	//(e.lally 2011-07-07) PLID 31585 - changed to DL2 controls
	NXDATALIST2Lib::_DNxDataListPtr m_pUnselectedList, m_pSelectedList;

	BOOL m_bCombined;
	MergeListType m_eListType;

// Dialog Data
	//{{AFX_DATA(CListMergeDlg)
	enum { IDD = IDD_LIST_MERGE_DLG };
	CNxIconButton	m_btnCombine;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticTopWarningLabel;
	CNxStatic	m_nxstaticBottomWarningLabel;
	CNxStatic	m_nxstaticUnselectedLabel;
	CNxStatic	m_nxstaticSelectedLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListMergeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CListMergeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSelectOne();
	afx_msg void OnBtnUnselectOne();
	afx_msg void OnBtnUnselectAll();
	afx_msg void OnDblClickCellSelectedMergeList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellUnselectedMergeList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnCombine();
	afx_msg void OnEditingFinishedSelectedMergeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	//(e.lally 2011-07-07) PLID 31585
	void EnsureControls();
	void MergeInsuranceCompanies(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep);
	void MergeNoteCategories(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep);
	// (j.jones 2012-04-10 17:16) - PLID 44174 - added ability to merge resources
	void MergeSchedulerResources(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep);
	// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
	void MergeProviders(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep);
	// (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
	void MergeLocations(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTMERGEDLG_H__1F091846_F4A5_4A14_AC57_CF2795B29E83__INCLUDED_)
