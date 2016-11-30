#pragma once

// CConfigExcludableAuditEventsDlg dialog

// (j.jones 2010-01-08 10:46) - PLID 35778 - created

#include "AdministratorRc.h"

class CConfigExcludableAuditEventsDlg : public CNxDialog
{

public:
	CConfigExcludableAuditEventsDlg(CWnd* pParent);   // standard constructor
	virtual ~CConfigExcludableAuditEventsDlg();

// Dialog Data
	enum { IDD = IDD_CONFIG_EXCLUDABLE_AUDIT_EVENTS_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxColor m_bkg;
	CNxIconButton m_btnSelectOne;
	CNxIconButton m_btnUnselectOne;
	CNxIconButton m_btnUnselectAll;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedList;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellUnselectedExcludableAuditList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedExcludableAuditList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnSelectOneAuditItem();
	afx_msg void OnBtnUnselectOneAuditItem();
	afx_msg void OnBtnUnselectAllAuditItems();
};
