#pragma once

// (j.jones 2009-01-14 11:17) - PLID 32707 - created

// CInvReconcileHistoryDlg dialog

class CInvReconcileHistoryDlg : public CNxDialog
{

public:
	CInvReconcileHistoryDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_INV_RECONCILE_HISTORY_DLG };
	CNxIconButton m_btnClose;
	CNxIconButton m_btnNew;
	NxButton m_checkShowCancelled;
	CNxIconButton m_btnEditActive;
	CNxIconButton m_btnViewClosed;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_CurrentList;
	NXDATALIST2Lib::_DNxDataListPtr m_HistoryList;
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;

	long GetLocationID();

	void ReloadCurrentList();
	void ReloadHistoryList();

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnCloseInvRec();
	afx_msg void OnBtnNewReconciliation();		
	afx_msg void OnCheckShowCancelledInvRec();
	afx_msg void OnSelChosenReconcileLocation(LPDISPATCH lpRow);
	afx_msg void OnDblClickCellReconcileCurrentList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellReconcileHistoryList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnEditCurInvRec();
	afx_msg void OnBtnViewHistInvRec();
};
