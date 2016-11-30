#if !defined(AFX_EMRSETUPDLG_H__BA753DAD_E5BB_4012_AB7B_7A1DEB35F534__INCLUDED_)
#define AFX_EMRSETUPDLG_H__BA753DAD_E5BB_4012_AB7B_7A1DEB35F534__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMESetupDlg.h : header file
//
#include "MirrorImageButton.h"

#import "RichTextEditor.tlb"

// Prompts the user for various criteria and returns a filter sufficient for producing a 
// reasonably small set of EMR Info items to be included in a given mergeinfo for use in 
// editing a Word template.  Returns TRUE for success, FALSE if the user canceled.
BOOL GetEMRFilter(CString &strOutEMRFilter);

/////////////////////////////////////////////////////////////////////////////
// CEMRSetupDlg dialog

class CEMRSetupDlg : public CNxDialog
{
// Construction
public:
	CEMRSetupDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	CTableChecker m_EMRTemplateChecker, m_EMRInfoChecker;

	long m_nTemplateFilterID;
	bool m_bAutoFilter;

	// (z.manning, 04/16/2008) - PLID 29566 - Set button styles
	// (d.thompson 2009-03-03) - PLID 33103 - Added filter for details button.
// Dialog Data
	//{{AFX_DATA(CEMRSetupDlg)
	enum { IDD = IDD_EMR_SETUP };
	NxButton	m_btnShowInactive;
	CNxIconButton	m_btnAddEmrItem;
	CNxIconButton	m_btnDeleteEmrItem;
	CNxIconButton	m_btnEditEmnTemplates;
	CNxIconButton	m_btnManageCollections;
	CNxIconButton	m_btnEditWordTemplates;
	CNxIconButton	m_btnEditLinkedItems;
	CNxIconButton	m_btnEditEmrVisitTypes;
	CNxIconButton	m_btnEditEmChecklists;
	// (j.jones 2011-07-05 12:06) - PLID 43603 - renamed to just 'status lists'
	CNxIconButton	m_btnStatusLists;
	CNxIconButton	m_btnOfficeVisit;
	CNxIconButton	m_btnEditSignature;
	CNxIconButton	m_btnFilterDetails;
	CNxIconButton	m_btnEditWellnessTemplates;
	// (j.jones 2010-02-10 17:01) - PLID 37224 - added ability to edit image stamps from Admin.
	CNxIconButton	m_btnEditImageStamps;
	CNxIconButton	m_btnProviderConfig; // (z.manning 2011-01-31 10:03) - PLID 42334
	CNxIconButton	m_btnEmrCoding; // (z.manning 2011-07-05 15:22) - PLID 44421
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRSetupDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pdlItemList;
	enum EItemListColumns {
		ilcID,
		ilcName,
		ilcDataType,
		ilcInactive,
		ilcInputDate,
		ilcEmrInfoMasterID,
		ilcDataSubType, // (c.haag 2007-01-31 10:20) - PLID 24428 - We now query the DataSubType field for coloring
	};

	NXDATALISTLib::_DNxDataListPtr m_pTemplateFilter;
	BOOL m_bSendTableCheckers;
	bool m_bFiltering;

	//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"
	void RefreshTable(NetUtils::ERefreshTable table, DWORD id = -1);

	void EnableAppropriateFields();

	void AddProductToDataList(long nProductID, CString strProductName, BOOL bAddAsAction);

	void OpenEmrItemEntryDlg(long nRowIndex);

	CString CalcDataListWhereClause();

	CString CalcInfoFilteredWhereClause(long nTemplateID, CString strIDName);

	// (d.thompson 2009-03-03) - needed for PLID 33103
	void UnfilterView();

	RICHTEXTEDITORLib::_DRichTextEditorPtr m_RichEditCtrl;

	class CEmrTemplateManagerDlg* m_pdlgTemplateManager;

	// (j.jones 2007-12-11 10:44) - PLID 28321 - removes the given
	// selection, and tries to reselect a nearby row
	void RemoveItemRow(long nCurSel);

	// (z.manning 2008-06-13 09:31) - PLID 30384 - Will auto-size the item name column based on the ideal width.
	void UpdateColumnWidths();

	// Generated message map functions
	//{{AFX_MSG(CEMRSetupDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnAddInfoItem();
	afx_msg void OnDeleteInfoItem();
	afx_msg void OnSetupHeader();
	afx_msg void OnEditTemplate();
	afx_msg void OnManageEmrcollections();
	afx_msg void OnOfficeVisit();
	afx_msg void OnSelChosenEmrTemplateSelectList(long nRow);
	afx_msg void OnRequeryFinishedEmrTemplateSelectList(short nFlags);
	// (j.jones 2011-07-05 12:06) - PLID 43603 - renamed to just 'status lists'
	afx_msg void OnEditStatusLists();
	afx_msg void OnEditMintTemplate();
	afx_msg void OnSelChangedEmrInfoList(long nNewSel);
	afx_msg void OnDblClickCellEmrInfoList(long nRowIndex, short nColIndex);
	afx_msg void OnRequeryFinishedEmrInfoList(short nFlags);
	afx_msg LRESULT OnEditEMRTemplate(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRButtonDownEmrInfoList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEMRSetupEdit();
	afx_msg void OnEMRSetupCopy();
	afx_msg void OnShowInactiveItems();
	afx_msg void OnDestroy();
	afx_msg void OnInactivateItem();
	afx_msg void OnActivateItem();
	afx_msg void OnEditLinkedItems();
	// (j.jones 2007-08-16 08:36) - PLID 27054 - added Visit Types
	afx_msg void OnEditEmrVisitTypes();
	// (j.jones 2007-08-16 10:48) - PLID 27055 - added E/M Checklists
	afx_msg void OnEditEmrEmChecklists();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedEditEmrSignature(); // (z.manning 2008-10-22 16:06) - PLID 21082
	afx_msg void OnFilterEMRDetails();
	afx_msg void OnEditWellnessTemplates();
	// (j.jones 2010-02-10 17:01) - PLID 37224 - added ability to edit image stamps from Admin.
	afx_msg void OnBtnEditImageStamps();
	// (z.manning 2011-01-31 10:01) - PLID 42334
	afx_msg void OnBnClickedEmrProviderConfig();
	afx_msg void OnBnClickedEmrCodingSetupButton(); // (z.manning 2011-07-05 15:12) - PLID 44421
	afx_msg void OnSelChosenEMRInfoList(long nRow); // (b.savon 2014-07-08 07:55) - PLID 62766
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRSETUPDLG_H__BA753DAD_E5BB_4012_AB7B_7A1DEB35F534__INCLUDED_)
