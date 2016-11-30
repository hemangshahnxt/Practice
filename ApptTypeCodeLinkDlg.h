#pragma once
#include "AdministratorRc.h"

// CApptTypeCodeLinkDlg dialog
// (j.gruber 2010-07-21 10:46) - PLID 30481 - created
class CApptTypeCodeLinkDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptTypeCodeLinkDlg)

public:
	CApptTypeCodeLinkDlg(long nApptTypeID, CString strApptType, CWnd* pParent);   // standard constructor
	virtual ~CApptTypeCodeLinkDlg();

// Dialog Data
	enum { IDD = IDD_APPT_TYPE_CODE_LINK_DLG };

protected:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_rdService;
	NxButton m_rdInventory;
	NXDATALIST2Lib::_DNxDataListPtr m_pServiceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pInvList;
	NXDATALIST2Lib::_DNxDataListPtr  m_pSelectList;
	CNxStatic m_stDesc;
	void SetAvailList();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void Save();
	long m_nApptTypeID;
	CString m_strApptType;

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
public:
	afx_msg void OnBnClickedRdServiceCodes();
	afx_msg void OnBnClickedRdInvItems();
	DECLARE_EVENTSINK_MAP()
	void SelChosenCatcCodeList(LPDISPATCH lpRow);
	void SelChosenCatcInvList(LPDISPATCH lpRow);
	void RButtonUpCatcSelectedList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
