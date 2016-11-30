//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_RESOURCEORDERDLG_H__5F352FD6_5265_11D2_80D3_00104B2FE914__INCLUDED_)
#define AFX_RESOURCEORDERDLG_H__5F352FD6_5265_11D2_80D3_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourceOrderDlg.h : header file
//
#include "resourcepurptypedlg.h"

/////////////////////////////////////////////////////////////////////////////
// CResourceOrderDlg dialog

class CResourceOrderDlg : public CNxDialog
{
// Construction
public:
	CResourceOrderDlg(CWnd* pParent);   // standard constructor
	// (z.manning, 02/20/2007) - PLID 24167 - Added destructor.
	~CResourceOrderDlg();

	void ResolveButtons(long nIndex);
	void CommitDeletedResources(CString& strSqlBatch);
	void CommitDeletedViews(CString& strSqlBatch);
	long SaveViewName(long nSaveViewIndex);
	BOOL SaveResourceList(long nSaveViewIndex);
	void LoadResourceList();
	long UniquifyRelevences();
	BOOL HasWritePermissionForResource(long nResourceID);

// Dialog Data
	//{{AFX_DATA(CResourceOrderDlg)
	enum { IDD = IDD_RESOURCE_ORDER_DLG };
	CNxIconButton	m_downButton;
	CNxIconButton	m_upButton;
	CNxIconButton	m_btnAddResource;
	CNxIconButton	m_btnDeleteResource;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAllowedPurposes;
	CNxIconButton	m_btnProviderLinking;
	// (j.jones 2012-04-11 11:23) - PLID 44174 - added ability to merge resources
	CNxIconButton	m_btnMergeResources;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourceOrderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	// in/out member variable; on init it loads this id as the current view; on 
	// out this variable contains the currently selected view.
	long m_nCurResourceViewID;

	//TES 7/27/2010 - PLID 39445 - Output only, tells the caller what the default location filter for m_nCurResourceViewID is.
	long m_nCurLocationID;

protected:
	// (j.jones 2010-04-20 09:14) - PLID 38273 - removed in favor of the standardized batch functions
	//void AddToSqlBatchStatement(CString& strSqlBatch, LPCTSTR strFmt, ...);

protected:
	// (c.haag 2010-05-04 10:29) - PLID 37263 - When the user presses "Copy to Users...", this is
	// the index to the view that the user had selected
	long m_nCopyResourcesSrcViewIndex;

	//TES 7/26/2010 - PLID 39445 - Used when the default location is inactive.
	long m_nPendingLocationID;

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_lstResource;
	NXDATALISTLib::_DNxDataListPtr m_lstViews;
	NXDATALIST2Lib::_DNxDataListPtr m_pDefaultLocation;
	CDWordArray m_aryDeleteViews;
	CDWordArray m_aryDeleteResources;
	long m_nLastChosenRow;
	BOOL m_bNeedToSave;
	BOOL m_bSaveResourcePurpTypes;
	CArray<CResourcePurpTypeDlg::CCombination*, CResourcePurpTypeDlg::CCombination*> m_apSavedResPurpTypeCombinations;
	CArray<CResourcePurpTypeDlg::CCombination*, CResourcePurpTypeDlg::CCombination*> m_apChanged;
	bool ValidateResourceName(const IN CString &strNewName, const IN CString &strOldName = "");
	BOOL SaveResourcePurpTypes();
	// Generated message map functions
	//{{AFX_MSG(CResourceOrderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnMoveUpBtn();
	afx_msg void OnMoveDownBtn();
	virtual void OnOK();
	afx_msg void OnEditingFinishedResourceList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangedResourceList(long nNewSel);
	afx_msg void OnNewResourceBtn();
	afx_msg void OnDeleteResourceBtn();
	afx_msg void OnEditingFinishingResourceList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelChosenViewList(long nRow);
	afx_msg void OnEditviewBtn();
	afx_msg void OnViewAdd();
	afx_msg void OnViewEdit();
	afx_msg void OnViewDelete();
	afx_msg void OnBtnProviderLinking();
	afx_msg void OnEditingStartingResourceList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnCopyResourceSettings();
	afx_msg void OnBtnResourceappttypes();
	afx_msg void OnDestroy();
	afx_msg void OnRequeryFinishedResourceList(short nFlags);
	afx_msg void OnCopyResourcesToMultipleUsers();
	afx_msg void OnCopyResourceSettingsToMultipleUsers();
	afx_msg void OnCopyResourceSettingsToAllUsers();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnSelChangingDefaultViewLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenDefaultViewLocation(LPDISPATCH lpRow);
	void OnTrySetSelFinishedDefaultViewLocation(long nRowEnum, long nFlags);
	// (j.jones 2012-04-11 11:22) - PLID 44174 - added ability to merge resources
	afx_msg void OnBtnMergeResources();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCEORDERDLG_H__5F352FD6_5265_11D2_80D3_00104B2FE914__INCLUDED_)
