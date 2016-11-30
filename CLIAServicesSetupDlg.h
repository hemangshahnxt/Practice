#pragma once

// (j.jones 2011-10-19 09:37) - PLID 46023 - created

// CCLIAServicesSetupDlg dialog

#include "AdministratorRc.h"

class CCLIAServicesSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCLIAServicesSetupDlg)

public:
	CCLIAServicesSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CCLIAServicesSetupDlg();

	OLE_COLOR m_nBkgColor;

// Dialog Data
	enum { IDD = IDD_CLIA_SERVICES_SETUP_DLG };
	CNxIconButton	m_btnUnselectOneCPT;
	CNxIconButton	m_btnUnselectAllCPT;
	CNxIconButton	m_btnSelectOneCPT;
	CNxIconButton	m_btnSelectAllCPT;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxColor		m_bkg1;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedCPTList, m_SelectedCPTList;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSelectOneCpt();
	afx_msg void OnBtnSelectAllCpt();
	afx_msg void OnBtnUnselectOneCpt();
	afx_msg void OnBtnUnselectAllCpt();
	void OnDblClickCellUnselectedCliaServiceList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedCliaServiceList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnOk();
	afx_msg void OnCancel();
};
