//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_EXTERNALFORM_H__E4A73A21_E681_11D2_B68F_0000C0832801__INCLUDED_)
#define AFX_EXTERNALFORM_H__E4A73A21_E681_11D2_B68F_0000C0832801__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExternalForm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExternalForm dialog

class CExternalForm : public CNxDialog
{
// Construction
public:
	CString	m_Caption, 
				m_SQL, 
				m_ColWidths, 
				m_ColFormat, 
				m_SortBy,
				m_RepID,
				m_FilterField,
				*m_Filter;

	short		m_BoundCol;
	COleVariant	nVarIndex,
				varTmp;

	CExternalForm(CWnd* pParent);   // standard constructor

	// (a.walling 2010-02-24 10:15) - PLID 37483
	bool m_bRequireSelection;
	bool m_bDefaultRemember;

private:
	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButtons for OK and Cancel
// Dialog Data
	//{{AFX_DATA(CExternalForm)
	enum { IDD = IDD_EXTERNALFORM };
	CNxIconButton	m_remOneButton;
	CNxIconButton	m_remAllButton;
	CNxIconButton	m_addOneButton;
	CNxIconButton	m_addAllButton;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic		m_nxsNote; // (a.walling 2010-02-24 10:14) - PLID 37483
	NxButton	m_checkRememberExternalList;
	//}}AFX_DATA

	// (j.gruber 2014-03-05 16:00) - PLID 61203 - switch to datalist2
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailableList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;
	


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExternalForm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2015-01-22) - PLID 64646 - Gets the connection to use for the creation and insertion of temp table records.
	ADODB::_ConnectionPtr GetTempTableConnection();
	CString CreateTempReportIDTable(NXDATALIST2Lib::_DNxDataListPtr pDataList, short nIDColIndex, OUT long *pnRecordCount, OUT CStringArray &arSelected);
	// (d.thompson 2012-06-19) - PLID 50834
	void HandleSpecialChargeCategoryRecursion(CString strTempTable);

	// (j.gruber 2014-03-06 09:52) - PLID 61201 - for filtering	
	std::map<CString, NXDATALIST2Lib::IRowSettingsPtr> m_mapAvailList;
	void FilterAvailableList(CiString strFilter);
	void ResetAvailableList();
	BOOL m_bIsFiltered;	
	void AddRowFromMap(NXDATALIST2Lib::IRowSettingsPtr pRowFromMap);
	
	// Generated message map functions
	//{{AFX_MSG(CExternalForm)
	virtual BOOL OnInitDialog();
	virtual void OnOK();	
	afx_msg void OnAddOne();
	afx_msg void OnRemoveOne();
	afx_msg void OnAddAll();
	afx_msg void OnRemoveAll();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeFilter();	
	void DblClickCellAvailableList(LPDISPATCH lpRow, short nColIndex); // (j.gruber 2014-03-05 16:26) - PLID 61203
	void DblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex); // (j.gruber 2014-03-05 16:26) - PLID 61203
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXTERNALFORM_H__E4A73A21_E681_11D2_B68F_0000C0832801__INCLUDED_)
