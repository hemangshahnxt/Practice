#pragma once

// EditComboBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditComboBox dialog

class CEditComboBox : public CNxDialog
{
// Construction
public:
	// (j.armen 2012-06-06 15:46) - PLID 49856 - Created Constructors using the most commonly used items
	CEditComboBox(CWnd* pParent, long nListID, const CString& strTitle);
	CEditComboBox(CWnd* pParent, long nListID, NXDATALISTLib::_DNxDataListPtr pFromListV1, const CString& strTitle);
	CEditComboBox(CWnd* pParent, long nListID, NXDATALIST2Lib::_DNxDataListPtr pFromListV2, const CString& strTitle);
	NXDATALISTLib::_DNxDataListPtr m_pList;
private:
	long m_listID;
public:

	BOOL m_bAllowDefault;

	void SetAsDefault();
	void RemoveDefault();
	
	BOOL IsDefaultSelected();

	// (j.jones 2007-07-20 12:11) - PLID 26749 - added variable to track the current selection
	long m_nCurIDInUse;

	// (a.walling 2010-01-18 13:17) - PLID 36955 - the last selected ID, as well as the initial selection to attempt to select after requery
	long m_nLastSelID;
	_variant_t m_varInitialSel;

// Dialog Data
	//{{AFX_DATA(CEditComboBox)
	enum { IDD = IDD_EDITCOMBO };
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnEdit;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel; // (a.walling 2010-01-18 13:00) - PLID 36955 - Cancel button, if needed
	//}}AFX_DATA

	// (a.walling 2015-01-06 15:35) - PLID 64528 - CEditComboBox override where and from clauses
	CString m_strOverrideFromClause;
	CString m_strOverrideWhereClause;

	CEditComboBox& OverrideFrom(const CString& str)
	{
		m_strOverrideFromClause = str;
		return *this;
	}

	CEditComboBox& OverrideWhere(const CString& str)
	{
		m_strOverrideWhereClause = str;
		return *this;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditComboBox)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pFromListV1;
	NXDATALIST2Lib::_DNxDataListPtr m_pFromListV2;
	const CString m_strTitle;
	void EnableAppropriateButtons();
	_bstr_t GetFieldName(short nColumn);
	_bstr_t GetColumnTitle(short nColumn);

	// Generated message map functions
	//{{AFX_MSG(CEditComboBox)
	afx_msg void OnAdd();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnRButtonDownEditList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEditList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedEditList(short nFlags);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChangedEditList(long nNewSel);
	// (j.jones 2007-07-19 12:03) - PLID 26749 - added ability to disallow changes to data
	afx_msg void OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};