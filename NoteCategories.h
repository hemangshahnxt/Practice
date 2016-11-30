#if !defined(AFX_NOTECATEGORIES_H__E55C9603_E5DE_11D2_B6B9_00104B2FE914__INCLUDED_)
#define AFX_NOTECATEGORIES_H__E55C9603_E5DE_11D2_B6B9_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NoteCategories.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNoteCategories dialog

class CNoteCategories : public CNxDialog
{
// Construction
public:
	CNoteCategories(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr  m_pNxDlNoteCats;

	bool m_bEditingNoteCat;
	bool m_bEditingFollowUpCat;
	bool m_bEditingHistoryCat;

// Dialog Data
	//{{AFX_DATA(CNoteCategories)
	enum { IDD = IDD_NOTE_CATEGORIES };
	CNxIconButton	m_advEmrMergeSetupButton;
	CNxIconButton	m_okButton;
	CNxIconButton	m_deleteButton;
	CNxIconButton	m_newButton;
	CNxIconButton	m_btnCombineCategories;
	COleVariant m_varBoundItem;
	// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
	NxButton		m_checkSortAlphabetically;
	CNxIconButton	m_btnMoveUp;
	CNxIconButton	m_btnMoveDown;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNoteCategories)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetAsDefault();
	void RemoveDefault();
	BOOL IsDefaultSelected();
	bool GetCurrentPropertyName(CString& strName);

	// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
	void ReflectAlphabeticSorting(BOOL bSortAlphabetically);
	void UpdateArrowButtons();
	
	// Generated message map functions
	//{{AFX_MSG(CNoteCategories)
	virtual BOOL OnInitDialog();
	afx_msg void OnLeftClickNotecats(const VARIANT FAR& varBoundValue, long iColumn);
	afx_msg void OnRightClickNotecats(const VARIANT FAR& varBoundValue, long iColumn);
	afx_msg void OnPopupSelectionNotecats(long iItemID);
	afx_msg void OnRepeatedLeftClickNotecats(const VARIANT FAR& varBoundValue, long iColumn, long nClicks);
	virtual void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnLButtonUpNxdlnotecats(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedNxdlnotecats(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonUpNxdlnotecats(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnNew();
	afx_msg void OnDelete();
	afx_msg void OnRequeryFinishedNxdlnotecats(short nFlags);
	afx_msg void OnSelChangedNxdlnotecats(long nNewSel);
	afx_msg void OnAdvEmrMergeSetup();
	afx_msg void OnCombineCategories();
	//(e.lally 2011-11-18) PLID 46539 - Added
	afx_msg void OnEditingFinishingNxdlnotecats(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2012-04-17 16:43) - PLID 13109 - added sorting options
	afx_msg void OnCheckSortNotecatsAlphabetically();
	afx_msg void OnBtnMoveNotecatUp();
	afx_msg void OnBtnMoveNotecatDown();
	void OnDragEndNxdlnotecats(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
	void OnDragBeginNxdlnotecats(BOOL* pbShowDrag, long nRow, short nCol, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOTECATEGORIES_H__E55C9603_E5DE_11D2_B6B9_00104B2FE914__INCLUDED_)
