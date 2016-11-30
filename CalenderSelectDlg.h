#pragma once


// (d.singleton 2012-05-17 16:37) - PLID added new dialog

// CCalenderSelectDlg dialog

class CCalenderSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCalenderSelectDlg)

public:
	CCalenderSelectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCalenderSelectDlg();

	CString m_strWhere;
	long m_nCalendarID;

// Dialog Data
	enum { IDD = IDD_SELECT_PRE_OP_CALENDAR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	NXDATALIST2Lib::_DNxDataListPtr m_dlCalendarList;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void DblClickCellPreOpCalendarList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedOk();
};
