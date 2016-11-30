#pragma once


// CConfigurePermissionGroupsDlg dialog
// (j.gruber 2010-04-13 16:51) - PLID 37948 - created for
// (d.thompson 2013-03-26) - PLID 55847 - There were invisible Move All Right & Left buttons with code that noone is
//	sure ever worked.  I deleted all traces.

class CConfigurePermissionGroupsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigurePermissionGroupsDlg)

public:
	CConfigurePermissionGroupsDlg(BOOL bConfigureUsers, long nUserGroupID, CWnd* pParent);   // standard constructor
	virtual ~CConfigurePermissionGroupsDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_PERMISSION_TEMPLATES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL m_bConfigureUser;
	BOOL CConfigurePermissionGroupsDlg::OnInitDialog();
	void LoadLists();
	void ReloadDialog();

	CNxIconButton m_btnOneLeft;
	CNxIconButton m_btnOneRight;
	CNxIconButton m_btnSwitch;
	CNxIconButton m_btnClose;

	NXDATALIST2Lib::_DNxDataListPtr m_pAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPickList;

	void MoveOneLeft(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void MoveOneRight(NXDATALIST2Lib::IRowSettingsPtr pRow);

	long m_nUserGroupID;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPermsOneRight();
	afx_msg void OnBnClickedPermsOneLeft();
	DECLARE_EVENTSINK_MAP()
	void SelChosenPermsPickList(LPDISPATCH lpRow);
	afx_msg void OnBnClickedGotoOther();
	void RequeryFinishedPermsPickList(short nFlags);
	void DblClickCellPermsAvailList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellPermsSelList(LPDISPATCH lpRow, short nColIndex);
	virtual void OnOK();
	virtual void OnCancel();
};
