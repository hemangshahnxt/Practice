#pragma once

#include "PracticeRc.h"

// CSingleSelectMultiColumnDlg dialog

// (b.savon 2013-02-27 13:23) - PLID 54713 - Created

class CSingleSelectMultiColumnDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSingleSelectMultiColumnDlg)

private:
	CString m_strFrom;
	CString m_strWhere;
	CString m_strDescription;
	CString m_strTitle;
	CString m_strDisplayColumn;

	CStringArray m_aryColumns;
	CStringArray m_aryColumnHeaders;
	CSimpleArray<short> m_larySortOrder;
	// (j.jones 2013-08-01 13:45) - PLID 53317 - added optional sort ascending list
	CSimpleArray<bool> m_barySortAscending;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlSingleMultiColumn;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	CVariantArray m_varySelectedValues;

	void LoadDatalist();

public:
	CSingleSelectMultiColumnDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSingleSelectMultiColumnDlg();
	virtual BOOL OnInitDialog();

	// (j.jones 2013-08-01 13:45) - PLID 53317 - added constructor for an optional sort ascending list,
	// where a value of true means ascending, false means descending
	HRESULT Open(const CString &strFrom, const CString &strWhere, const CStringArray &aryColumns, const CStringArray &aryColumnHeaders, 
				 const CSimpleArray<short> &arySortOrder, OPTIONAL CSimpleArray<bool> &arySortAscending, const CString &strDisplayColumn, 
				 const CString &strDescription, const CString &strTitle);
	// (j.jones 2013-08-01 13:45) - PLID 53317 - use this constructor if no columns sort or if all columns
	// sort ascending, and none need to sort descending
	HRESULT Open(const CString &strFrom, const CString &strWhere, const CStringArray &aryColumns, const CStringArray &aryColumnHeaders, 
				 const CSimpleArray<short> &arySortOrder, const CString &strDisplayColumn, 
				 const CString &strDescription, const CString &strTitle);

	void GetSelectedValues(CVariantArray &varySelectedValues);

// Dialog Data
	enum { IDD = IDD_SINGLE_SELECT_MULTI_COLUMN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChosenNxdlSingleMulti(LPDISPATCH lpRow);
};
