#pragma once
// (s.dhole 2011-03-31 14:12) - PLID 43077 Create new Dialog to change Glasses order status
#include "InventoryRc.h"
// CInvGlassesOrderStatusDlg dialog

class CInvGlassesOrderStatusDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvGlassesOrderStatusDlg)

public:
	CInvGlassesOrderStatusDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvGlassesOrderStatusDlg();
	bool m_IsVisionWeb;
	long m_nOrderStatus;
	CString  m_strOrderStatusMsg, m_strCaption;
// Dialog Data
	enum { IDD = IDD_INV_EDIT_GLASSES_ORDER_STATUS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebOrderStatusCombo, m_pStandardNotes;
	virtual BOOL OnInitDialog();
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit  m_nxMsg;
	CNxIconButton m_btnEditStandardNotes;
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChangingOrderStatusList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	void OnSelChosenStandardHistoryNotes(LPDISPATCH lpRow);
	afx_msg void OnEditStandardNotes();
};
