#pragma once

// (j.jones 2011-07-05 14:57) - PLID 43603 - created

// CEMNStatusConfigDlg dialog

#include "AdministratorRc.h"

class CEMNStatusConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMNStatusConfigDlg)

public:
	CEMNStatusConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CEMNStatusConfigDlg();
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;

// Dialog Data
	enum { IDD = IDD_EMN_STATUS_CONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	CArray<int, int> m_aryDeletedStatuses;

	BOOL DoesStatusExist(const CString& strStatus, NXDATALIST2Lib::IRowSettingsPtr pRowToSkip);

	DECLARE_MESSAGE_MAP()
	BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnBtnAddEmnStatus();
	afx_msg void OnBtnRemoveEmnStatus();
	DECLARE_EVENTSINK_MAP()
	void OnEditingStartingEmnStatusList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingEmnStatusList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnSelChangedEmnStatusList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
