#pragma once

#include "CommonDialog.h"

// (z.manning 2014-12-04 15:38) - PLID 64217 - Created
// CNameAndColorEntryDlg dialog

class CNameAndColorEntryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNameAndColorEntryDlg)

public:
	CNameAndColorEntryDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNameAndColorEntryDlg();

	CString m_strName;
	COLORREF m_nColor;

	CString m_strWindowTitle;
	UINT m_nTextLimit;
	CString m_strSqlTable, m_strSqlColumn;

// Dialog Data
	enum { IDD = IDD_NAME_AND_COLOR_ENTRY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CCommonDialog60	m_ctrlColorPicker;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	BOOL Validate();

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedNameColorEntryChooseColor();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
};
