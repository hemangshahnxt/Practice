#pragma once

// CSecondaryANSIAllowedAdjConfigDlg dialog

// (j.jones 2010-02-03 08:47) - PLID 37159 - created

#include "AdministratorRc.h"

class CSecondaryANSIAllowedAdjConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSecondaryANSIAllowedAdjConfigDlg)

public:
	CSecondaryANSIAllowedAdjConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CSecondaryANSIAllowedAdjConfigDlg();

	long m_nGroupID;
	BOOL m_bIsUB;

// Dialog Data
	enum { IDD = IDD_SECONDARY_ANSI_ALLOWED_ADJ_CONFIG_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRemove;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	CArray<long, long> m_aryDeletedCodes;
	CArray<long, long> m_aryChangedCodes;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	afx_msg void OnBtnAddAdjCode();
	afx_msg void OnBtnRemoveAdjCode();
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishedAllowedAmtCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnRButtonDownAllowedAmtCodesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
