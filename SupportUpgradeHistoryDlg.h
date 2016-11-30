#pragma once

// (f.dinatale 2010-05-19) - PLID 38842 - Added the Support_Upgrade_History_Dlg which
// displays all version changes to the currently selected client.
// CSupportUpgradeHistoryDlg dialog

class CSupportUpgradeHistoryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSupportUpgradeHistoryDlg)

public:
	CSupportUpgradeHistoryDlg(CWnd* pParent = NULL, long nClientID = -1);   // standard constructor
	virtual ~CSupportUpgradeHistoryDlg();

// Dialog Data
	enum { IDD = IDD_SUPPORT_UPGRADE_HISTORY_DLG };

protected:
	// (f.dinatale 2010-05-19) - PLID 38842 - Pointer to the Upgrade History datalist
	NXDATALIST2Lib::_DNxDataListPtr m_pUpgradehist;
	long m_nClientID;
	CNxIconButton m_nxbClose;

	BOOL OnInitDialog();
	void OnRequeryFinished(short nFlags);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	DECLARE_EVENTSINK_MAP()
};
