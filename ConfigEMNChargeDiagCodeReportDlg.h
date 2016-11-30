#pragma once

// (j.gruber 2010-03-10 10:34) - PLID 37660 - created for

#include "Reportsrc.h"
// CConfigEMNChargeDiagCodeReportDlg dialog

class CConfigEMNChargeDiagCodeReportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigEMNChargeDiagCodeReportDlg)

public:
	CConfigEMNChargeDiagCodeReportDlg(CWnd* pParent);   // standard constructor
	virtual ~CConfigEMNChargeDiagCodeReportDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_EMN_REPORT };

protected:
	CNxIconButton m_btnUp;
	CNxIconButton m_btnDown;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	NXDATALIST2Lib::_DNxDataListPtr m_pSelCatList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCatList;

	void CheckButtonStatus();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedConfigEmnReportUp();
	afx_msg void OnBnClickedConfigEmnReportDown();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnRemoveCategory();
	DECLARE_EVENTSINK_MAP()
	void SelChosenConfigEmnReportCategoryList(LPDISPATCH lpRow);
	void RButtonUpConfigEmnReportSelectList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChangedConfigEmnReportSelectList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
