#pragma once

// CERemitImportFilteringDlg dialog

// (j.jones 2010-02-08 15:00) - PLID 37174 - created

#include "FinancialRc.h"

class CERemitImportFilteringDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CERemitImportFilteringDlg)

public:
	CERemitImportFilteringDlg(CWnd* pParent);   // standard constructor
	virtual ~CERemitImportFilteringDlg();

// Dialog Data
	enum { IDD = IDD_EREMIT_IMPORT_FILTERING_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAddEOBID;
	CNxIconButton	m_btnDeleteEOBID;
	// (j.jones 2010-02-09 13:44) - PLID 37254 - added claim filtering
	CNxIconButton	m_btnAddClaimID;
	CNxIconButton	m_btnDeleteClaimID;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_EOBIDList;
	// (j.jones 2010-02-09 13:44) - PLID 37254 - added claim filtering
	NXDATALIST2Lib::_DNxDataListPtr m_ClaimIDList;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnClose();
	afx_msg void OnBtnAddEobID();
	afx_msg void OnBtnDeleteEobID();
	DECLARE_EVENTSINK_MAP()
	void OnRButtonDownEobIDsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingEobIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedEobIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (j.jones 2010-02-09 14:00) - PLID 37254 - added claim filtering
	afx_msg void OnBtnAddClaimID();
	afx_msg void OnBtnDeleteClaimID();
	void OnRButtonDownClaimIDsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingClaimIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedClaimIDsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
