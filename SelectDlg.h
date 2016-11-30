#if !defined(AFX_SELECTDLG_H__BCAE0736_5033_4C57_ADA0_2DCCF52BC5F1__INCLUDED_)
#define AFX_SELECTDLG_H__BCAE0736_5033_4C57_ADA0_2DCCF52BC5F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectDlg dialog

struct DatalistColumn {
	DatalistColumn() {
		strField = _bstr_t("");
		strTitle = _bstr_t("");
		nWidth = 0;
		nStyle = NXDATALISTLib::csVisible|NXDATALISTLib::csFixedWidth;
		nSortPriority = -1;
		bSortAsc = TRUE;
		bWordWrap = FALSE;
		// (r.gonet 03/05/2014) - PLID 61187 - By default, columns don't define the back color.
		bRowBackColor = FALSE;
	}

	_bstr_t strField;
    _bstr_t strTitle;
    long nWidth;
    long nStyle;
	int nSortPriority;
	BOOL bSortAsc;
	BOOL bWordWrap;
	// (r.gonet 03/05/2014) - PLID 61187 - If set to TRUE, then the DWORD value of this column (from RGB(r,g,b)) sets the row background color.
	BOOL bRowBackColor;
};

class CSelectDlg : public CNxDialog
{
// Construction
public:
	CSelectDlg(CWnd* pParent);   // standard constructor

	CString m_strTitle;
	CString m_strCaption;
	CString m_strFromClause;
	CString m_strWhereClause;
	CString m_strGroupByClause;

	variant_t m_varPreSelectedID;
	short m_nPreSelectColumn;
	void SetPreSelectedID(short nColumn, _variant_t varID);

	// (a.walling 2013-02-13 11:41) - PLID 55148 - Allow no selection
	bool m_bAllowNoSelection;

	//TES 8/1/2006 - This defaulted to true, but the m_arSelectedValues have never returned more than one row's worth of values,
	// and most places that use this dialog should not allow multiple selection anyway. So, I'm commenting this out (it will now 
	// always behave as if m_bAllowMultiSelect were false), we can comment this back in someday if we're prepared to actually 
	// implement multi-selection.
	//Defaults to true.
	//bool m_bAllowMultiSelect;

	//Input
	CArray<DatalistColumn, DatalistColumn&> m_arColumns;
	// (a.walling 2009-04-14 17:58) - PLID 33951 - Data width style
	// (j.jones 2009-08-28 11:54) - PLID 29185 - added data width size and optional sort priorities
	void AddColumn(CString strField, CString strTitle, BOOL bVisible, BOOL bWordWrap, BOOL bDataWidth = FALSE,
		long nDataWidthSize = -1, long nSortPriority = -1, BOOL bSortAscending = TRUE);
	// (r.gonet 03/05/2014) - PLID 61187 - Add a hidden column which dictates the color of the row background.
	void AddRowBackColorColumn(CString strField); 

	//Output, will correspond with m_arColumns.
	CArray<_variant_t, _variant_t> m_arSelectedValues;

public:
	// (c.haag 2009-08-24 16:07) - PLID 29310 - TRUE if we allow adding new records to the list on the fly
	BOOL m_bAllowAddRecord;
	// (c.haag 2009-08-24 16:07) - PLID 29310 - The display name for the type of record being added (i.e. "Collection", "Category")
	CString m_strRecordType;
	// (c.haag 2009-08-24 16:08) - PLID 29310 - The table for adding the new record
	CString m_strRecordTable;
	// (c.haag 2009-08-24 16:08) - PLID 29310 - The name field for the new record
	CString m_strRecordField;
	// (c.haag 2009-08-24 16:09) - PLID 29310 - The SQL syntax for inserting the record in data. A sample
	// format is "SET NOCOUNT ON;insert into mytable (name) values ({STRING});SET NOCOUNT OFF; SELECT Convert(int, SCOPE_IDENTITY()) AS NewID"
	CString m_strParamSqlRecordAdd;
	// (c.haag 2009-08-24 16:33) - PLID 29310 - The ID column
	short m_nRecordIDColumn;
	// (c.haag 2009-08-24 16:33) - PLID 29310 - The name column
	short m_nRecordNameColumn;

public:
	// (c.haag 2009-08-24 16:50) - PLID 29310 - Set to TRUE if at least one record was added
	BOOL m_bWereRecordsAdded;

protected:
// Dialog Data
	//{{AFX_DATA(CSelectDlg)
	enum { IDD = IDD_SELECT_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticSelectCaption;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pList;

	// Generated message map functions
	//{{AFX_MSG(CSelectDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChangedList(long nNewSel);
	afx_msg void OnDblClickCellList(long nRowIndex, short nColIndex);
	afx_msg void OnTrySetSelFinishedList(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnAdd();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTDLG_H__BCAE0736_5033_4C57_ADA0_2DCCF52BC5F1__INCLUDED_)
