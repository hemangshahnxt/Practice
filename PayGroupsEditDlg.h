#pragma once

// CPayGroupsEditDlg dialog

// (j.jones 2010-07-30 10:29) - PLID 39728 - created

#include "AdministratorRc.h"

class CPayGroupsEditDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPayGroupsEditDlg)

public:
	CPayGroupsEditDlg(CWnd* pParent);   // standard constructor
	virtual ~CPayGroupsEditDlg();

// Dialog Data
	enum { IDD = IDD_PAY_GROUPS_EDIT_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_PayGroupList;

	CArray<long,long> m_aryGroupsToDelete;

	// (j.jones 2010-08-04 16:16) - PLID 39991 - moved delete button enabling to its own function
	void UpdateDeleteButtonEnabledState();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddPayGroup();
	afx_msg void OnBtnDeletePayGroup();
	afx_msg void OnOk();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangedPayGroupsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnEditingFinishingPayGroupsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
public:
	void OnEditingStartingPayGroupsList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	afx_msg void OnBnClickedCancel();
};
