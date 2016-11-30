#pragma once
// (a.wilson 2014-07-10 09:58) - PLID 62526 - created.

#include "Financialrc.h"

// CConfigureColumnsDlg dialog

// (a.wilson 2014-07-10 10:01) - PLID 62526 - struct to easily keep track of a column.
struct ConfigureColumn {
	long nColumnID;
	CString strName;
	long nOrderIndex;

	ConfigureColumn() {}
	ConfigureColumn(long _nColumnID, CString _strName, long _nOrderIndex)
	{
		nColumnID = _nColumnID;
		strName = _strName;
		nOrderIndex = _nOrderIndex;
	}
};

class CConfigureColumnsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureColumnsDlg)

public:
	bool m_bOrderChanged;

	CConfigureColumnsDlg(CWnd* pParent = NULL);   // standard constructor
	CConfigureColumnsDlg(const CArray<ConfigureColumn>& aryColumns, CWnd* pParent = NULL);
	virtual ~CConfigureColumnsDlg();

	CArray<ConfigureColumn>& GetOrderedColumns();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_COLUMNS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CArray<ConfigureColumn> m_aryConfigureColumns;
	NXDATALIST2Lib::_DNxDataListPtr m_pColumnList;
	CNxIconButton	m_btnUp, m_btnDown;
	CNxIconButton	m_btnOk, m_btnCancel;

	virtual BOOL OnInitDialog();
	void SwitchRowOrder(NXDATALIST2Lib::IRowSettingsPtr pCurrentRow, NXDATALIST2Lib::IRowSettingsPtr pOtherRow);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedConfigureColumnUp();
	afx_msg void OnBnClickedConfigureColumnDown();
	DECLARE_EVENTSINK_MAP()
	void SelSetConfigureColumnList(LPDISPATCH lpSel);
};
