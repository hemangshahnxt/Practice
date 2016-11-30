#pragma once

#include "billingRc.h"

// (s.dhole 2011-05-31 15:04) - PLID 46357 Added Service code selection Dialog
// CBillChangeServiceCodeDlg dialog

class CBillChangeServiceCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBillChangeServiceCodeDlg)

public:
	CBillChangeServiceCodeDlg(CWnd* pParent);   // standard constructor
	virtual ~CBillChangeServiceCodeDlg();
	long m_nServiceCodeID;
	
// Dialog Data
	enum { IDD = IDD_CHANGE_BILL_SEVICE_CODE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	NXDATALIST2Lib::_DNxDataListPtr	 m_CPTListCombo; 
	
	afx_msg void OnSelChangingCptCodeCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel); //
	afx_msg void OnSelChosenCptCodeCombo(LPDISPATCH lpRow);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CNxIconButton	m_nxbtnOk;
	CNxIconButton	m_nxbtnCancel;
	
	DECLARE_EVENTSINK_MAP()

	DECLARE_MESSAGE_MAP()


};
