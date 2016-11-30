#pragma once

// (j.jones 2010-08-02 12:04) - PLID 39912 - created

// CAdvServiceCodePayGroupSetupDlg dialog

#include "AdministratorRc.h"

class CAdvServiceCodePayGroupSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAdvServiceCodePayGroupSetupDlg)

public:
	CAdvServiceCodePayGroupSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CAdvServiceCodePayGroupSetupDlg();

// Dialog Data
	enum { IDD = IDD_ADV_SERVICE_CODE_PAY_GROUP_SETUP_DLG };
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnEditPayGroups;
	CNxIconButton	m_btnApply;

protected:

	NXDATALIST2Lib::_DNxDataListPtr	m_InsCoCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_PayGroupCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_UnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr	m_SelectedList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSelectOneCode();
	afx_msg void OnBtnSelectAllCodes();
	afx_msg void OnBtnUnselectOneCode();
	afx_msg void OnBtnUnselectAllCodes();
	afx_msg void OnBtnClose();
	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellUnselectedCptList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedCptList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnEditPayGroups();
	void OnSelChosenInscoCombo(LPDISPATCH lpRow);
	void OnSelChosenPayGroupCombo(LPDISPATCH lpRow);
	void OnSelChangingInscoCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingPayGroupCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBtnApplyPayGroups();
};