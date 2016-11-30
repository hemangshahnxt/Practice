#pragma once

// MultiSelectDlg.h : header file
//

#include <afxtempl.h>
#include "practicerc.h"
#include "stdafx.h"

class CMultiSelectDlg;
typedef BOOL (CALLBACK* MULTISELECTDLG_CONTEXTMENUPROC)(IN CMultiSelectDlg *pwndMultSelDlg, IN LPARAM pParam, IN NXDATALIST2Lib::IRowSettings *lpRow, IN CWnd* pContextWnd, IN const CPoint &point, IN OUT CArray<long, long> &m_aryOtherIDs);
/////////////////////////////////////////////////////////////////////////////
// CMultiSelectDlg dialog

struct MultiSelectRow 
{
	_variant_t varID;
	_variant_t varName;
};

class CMultiSelectDlg : public CNxDialog
{
// Construction
public:
	~CMultiSelectDlg();

	HRESULT Open(CString strFrom, CString strWhere,
		CString strIDField, CString strValueField, CString strDescription,
		unsigned long nMinSelections = 0, unsigned long nMaxSelections = 0xFFFFFFFF,
		CStringArray *straryExtraColumnFields = NULL, CStringArray *straryExtraColumnNames = NULL, BOOL bAutoCheckMultipleSelections = FALSE);
	
	//Give it a datalist, and the IDs of any rows (like {Multiple} or {None}) which shouldn't be included in the list.
	// (j.jones 2009-08-11 17:59) - PLID 35189 - renamed the datalist parameter to make it more obvious that it is a datalist1
	HRESULT Open(NXDATALISTLib::_DNxDataList * pDataList1, CVariantArray &vaIDsToSkip, CString strDescription, unsigned long nMinSelections = 0, unsigned long nMaxSelections = 0xFFFFFFFF); //Column 0 = ID, Column 1 = Description
	// (r.gonet 05/22/2014) - PLID 62250 - Added the ability to open a datalist 2. Also with the ability to have extra columns.
	HRESULT OpenWithDataList2(NXDATALIST2Lib::_DNxDataListPtr pDataList2, CVariantArray &vaIDsToSkip, CString strDescription, unsigned long nMinSelections = 0, unsigned long nMaxSelections = 0xFFFFFFFF, 
		short nSourceIDColumnIndex = 0, short nSourceNameColumnIndex = 1, CArray<short, short> *parySourceExtraColumnIndices = NULL, 
		BOOL bAutoCheckMultipleSelections = FALSE);
	// (c.haag 2011-03-18 16:58) - PLID 42908 - Added the ability to open based on an array of ID's and names
	HRESULT Open(CArray<long,long>& anIDs, CStringArray& astrNames, CString strDescription, unsigned long nMinSelections = 0, unsigned long nMaxSelections = 0xFFFFFFFF);
	// (z.manning, 07/18/2007) - PLID 26727 - Added the ability to populate the list based on a semicolon
	// delimited list from a datalist's embedded combo source.
	HRESULT OpenWithDelimitedComboSource(_bstr_t bstrEmbeddedComboSource, CVariantArray &vaIDsToSkip, CString strDescription, unsigned long nMinSelections = 0, unsigned long nMaxSelections = 0xFFFFFFFF); //Column 0 = ID, Column 1 = Description
	// (c.haag 2010-11-22 16:50) - PLID 41588 - Added the ability to open the list pulling from a simple string array.
	HRESULT OpenWithStringArray(CStringArray& astrSource, CString strDescription, unsigned long nMinSelections = 0, unsigned long nMaxSelections = 0xFFFFFFFF);

	// Used to just fill an an array with selected ID's
	void FillArrayWithIDs(CDWordArray& adw);
	void FillArrayWithIDs(CArray<long,long> &arIDs);
	void FillArrayWithIDs(CVariantArray &arIDs);
	void FillArrayWithIDs(std::set<long>& arIDs); // (c.haag 2015-01-05) - PLID 64257
	void FillArrayWithNames(CVariantArray &arNames);

	// (z.manning 2011-10-25 14:09) - PLID 39401 - If you want the unselected IDs
	void FillArrayWithUnselectedIDs(CArray<long,long> *parynIDs);
	// (r.gonet 2016-01-22 15:05) - PLID 68041 - Gets the selected IDs as a variant array.
	void FillArrayWithUnselectedIDs(CVariantArray *paryIDs);

	// (a.walling 2007-03-27 11:58) - PLID 25367 - Need a simple way to set IDs to skip
	void SkipIDsInArray(CDWordArray &adwIDs);
	void SkipIDsInArray(CArray<long,long> &arIDs);
	void SkipIDsInArray(CVariantArray &arIDs);

	void PreSelect(long ID);
	void PreSelect(CArray<long,long> &arIDs);
	//TES 4/10/2009 - PLID 33889 - Added (useful because this is what GetRemotePropertyArray() uses)
	void PreSelect(CArray<int,int> &arIDs);
	void PreSelect(CDWordArray& adwIDs);
	void PreSelect(CVariantArray &arIDs);
	// (j.gruber 2011-11-07 14:24) - PLID 46316 - added bool to say whether to use as IDs
	void PreSelect(const CString& strIDs, BOOL bUseAsIDS = FALSE); // Whitespace-separated
	void PreSelectByName(CString strName);		//DRT 1/12/2007 - PLID 24223 - Allow preselection by the name column

	// (j.jones 2007-03-14 15:18) - PLID 25195 - if filled, m_nIDToSelect can be assigned an ID,
	// and the dialog will set the corresponding row as the current selection
	long m_nIDToSelect;

	// (j.armen 2012-06-07 16:34) - PLID 49607 - Sizing ConfigRT entry
	CMultiSelectDlg(CWnd* pParent, const CString& strSizingConfigRT);   // standard constructor
	
	//for auditing
	CArray<_variant_t, _variant_t&> m_aryNewName, m_aryOldName;

	CString GetMultiSelectString(CString strDelimiter = ", ");
	CString GetMultiSelectIDString(const CString& strSeparator = " ");

	CString m_strNameColTitle;

	// Set this string to the text you'd like to appear on a third button.  If this text is set, the third 
	// button will show up in between the OK and Cancel buttons and will have the given text, otherwise the 
	// button won't be there at all.
	CString m_strOtherButtonText;
	// (z.manning, 04/30/2008) - PLID 29845 - Similarly, you can set a button style for the Other button
	NXB_TYPE m_eOtherButtonStyle;

	// (z.manning 2009-08-19 17:21) - PLID 17932 - Added a filter button. Set this to true to display it.
	BOOL m_bShowFilterButton;
	BOOL m_bFiltered;

	// If non-NULL m_pfnContextMenuProc will be called when the user right-clicks on an entry in the list.
	MULTISELECTDLG_CONTEXTMENUPROC m_pfnContextMenuProc;
	LPARAM m_nContextMenuProcParam;

	//for EMR, and perhaps other future uses
	CArray<long, long> m_aryOtherChangedMasterIDs;

	// (j.jones 2009-08-12 08:45) - PLID 35189 - changed the parameter to be a row
	BOOL SetIfOnlyItem(NXDATALIST2Lib::IRowSettingsPtr pSelRow);

	// (j.jones 2007-05-15 16:32) - PLID 26012 - this function is not used, but if we do ever use it,
	// we should really remove the WaitForRequery option inside it unless it is truly required
	//void Requery();

	//Set this to true if you want the primary column added from the Open() function to allow text wrapping.
	bool m_bWrapText;

	// (c.haag 2007-01-31 11:05) - PLID 24428 - You may now define row colors based on the
	// input data. This is the field definition for the column that this dialog looks at
	// to define row colors. (e.g. to color a record whose ID is 100 with the color red,
	// the string should be "CASE WHEN ID = 100 THEN red ELSE NULL END". Nulls are treated
	// as -1 values, which means no color
	CString m_strColorField;

	// (z.manning, 07/26/2007) - PLID 14579 - Pre-selectw all values by default if true.
	BOOL m_bPreSelectAll;

	// (c.haag 2008-02-13 16:08) - PLID 28855 - We now allow the sort order of the list to
	// be dictated from outside the dialog. The following arrays must always be exactly the same
	// size. For every element, a hidden column is added and given a sort priority and direction.
	//
	CStringArray m_astrOrderColumns;		// List of data columns to sort
	CArray<BOOL,BOOL> m_abSortAscending;	// Sort ascending/descending (corresponds 1:1 to m_astrOrderColumns)

	// (z.manning 2009-03-19 11:59) - PLID 33576 - Added an option to not sort;
	BOOL m_bNoSort;

	// (z.manning 2011-08-04 10:43) - PLID 44347 - Added an option to sort by selections first
	BOOL m_bPutSelectionsAtTop;
	// (s.dhole 2014-03-11 10:51) - PLID 61318
	BOOL m_bSetEqualColumnWidth;


public:
	// (j.jones 2009-08-12 09:45) - PLID 35189 - renamed this function to make it clear that it is a datalist2
	inline NXDATALIST2Lib::_DNxDataListPtr GetDataList2() { return m_dlList; }

public:
	enum EReturnValue {
		// These two enum values are just here to demonstrate that this dialog can also return 
		// some standard values.  Feel free to use these enum values or the standard IDOK/IDCANCEL.
		rvOk = IDOK,
		rvCancel = IDCANCEL,
		// The rest of the enum values should try to stay far away from the standard values (which
		// tend to be numbers between 1 and 20).  So we start at 1000 and go up from there.
		rvOtherBtn = 1000,
	};

public:
	enum EMultiSelectListColumns {
		mslcID,
		mslcSelected,
		mslcName,
	};

public:

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CMultiSelectDlg)
	enum { IDD = IDD_MULTISELECT_DLG };
	CNxStatic	m_nxstaticDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOther;
	CNxIconButton	m_btnFilter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiSelectDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CArray<MultiSelectRow, MultiSelectRow&> m_arySelectedRows;
	CArray<MultiSelectRow, MultiSelectRow&> m_aryUnselectedRows; // (z.manning 2011-10-25 14:11) - PLID 39401
	CString m_strFrom, m_strWhere, m_strIDField, m_strValueField, m_strDescription;
	// (r.gonet 05/22/2014) - PLID 62250 - The source datalist column index for the column we'll use as the ID.
	short m_nSourceIDColumnIndex = -1;
	// (r.gonet 05/22/2014) - PLID 62250 - The source datalist column index for the column we'll use as the Name.
	short m_nSourceNameColumnIndex = -1;
	// (r.gonet 05/22/2014) - PLID 62250 - Stores the source datalist column indices for columns we want to display in addition to the ID and Name columns.
	CArray<short, short> m_arySourceExtraColumnIndices;
	CStringArray m_straryExtraColumnFields;
	CStringArray m_straryExtraColumnNames;
	NXDATALISTLib::_DNxDataListPtr m_pSourceList;
	// (r.gonet 05/22/2014) - PLID 62250 - NxDataList2 that we can load the multi-select list from.
	NXDATALIST2Lib::_DNxDataListPtr m_pSourceList2 = NULL;
	CVariantArray m_vaIDsToSkip;
	CVariantArray m_arPreSelect;
	CStringArray m_arPreSelectByName;		//DRT 1/12/2007 - PLID 24223 - Allow preselect by name
	DWORD m_nMinSelections;
	DWORD m_nMaxSelections;
	// (j.jones 2009-08-11 17:59) - PLID 35189 - converted to use a datalist 2
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;
	BOOL m_bAutoCheckMultipleSelections; // (a.walling 2007-06-13 17:05) - PLID 26211
	CString m_strEmbeddedComboSource; // (z.manning, 07/18/2007) - PLID 26727
	CStringArray m_astrStringArraySource; // (c.haag 2010-11-22 16:50) - PLID 41588
	CArray<long,long> m_astrNameIDSource_IDs; // (c.haag 2011-03-18 16:58) - PLID 42908 - Added the ability to open based on an array of ID's and names
	CStringArray m_astrNameIDSource_Names; // (c.haag 2011-03-18 16:58) - PLID 42908 - Added the ability to open based on an array of ID's and names

protected:
	BOOL ValidateAndStore();

protected:
	// Generated message map functions
	//{{AFX_MSG(CMultiSelectDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnOtherBtn();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	// (j.jones 2009-08-12 08:29) - PLID 35189 - converted to a datalist2
	void OnRButtonDownMultiSelectList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnDblClickCellMultiSelectList(LPDISPATCH lpRow, short nColIndex);
	void OnRequeryFinishedMultiSelectList(short nFlags);
	afx_msg void OnFilterBtn();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
private:
	// (j.armen 2012-06-06 12:50) - PLID 49607 - Functions for setting the sizing ConfigRT entry
	CString m_strSizingConfigRT;
public:
	void SetSizingConfigRT(const CString& strSizingConfigRT) { m_strSizingConfigRT = strSizingConfigRT; }

protected:
	// (j.jones 2015-03-02 15:48) - PLID 65093 - detect when they checked a box
	void OnEditingFinishedMultiSelectList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
