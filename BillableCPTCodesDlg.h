#pragma once

// CBillableCPTCodesDlg dialog

// (j.jones 2011-03-28 10:31) - PLID 43012 - created

#include "AdministratorRc.h"

class CBillableCPTCodesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillableCPTCodesDlg)

public:
	CBillableCPTCodesDlg(CWnd* pParent);   // standard constructor
	virtual ~CBillableCPTCodesDlg();

// Dialog Data
	enum { IDD = IDD_BILLABLE_CPT_CODES_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedList;

	//validates whether the codes can be marked non-billable, due to a given rule
	BOOL ValidateCodes(CSqlFragment sqlQuery, CString strWarningStart, CString strWarningEnd, BOOL bCanSave);

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellUnselectedServiceCodes(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedServiceCodes(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnSelectOneCpt();
	afx_msg void OnBtnSelectAllCpt();
	afx_msg void OnBtnUnselectOneCpt();
	afx_msg void OnBtnUnselectAllCpt();
	afx_msg void OnOK();
};
