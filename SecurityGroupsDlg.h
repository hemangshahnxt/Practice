#pragma once

//TES 1/5/2010 - PLID 35774 - Created, inspired by CPatientGroupsDlg.
// CSecurityGroupsDlg dialog

class CSecurityGroupsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSecurityGroupsDlg)

public:
	// (j.gruber 2011-01-13 15:07) - PLID 40415 - take a variable as to whether to update our toolbar
	CSecurityGroupsDlg(CWnd* pParent, BOOL bGreyList = FALSE, BOOL bUpdateCurrentToolbar = FALSE);   // standard constructor
	virtual ~CSecurityGroupsDlg();

	long m_nPatientID;

// Dialog Data
	enum { IDD = IDD_SECURITY_GROUPS_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pGroupsList;
	NXCOLORLib::_DNxColorPtr m_pBkg;

	//TES 1/5/2010 - PLID 35774 - Track the initial checked groups so we can audit appropriately.
	CStringArray m_saInitialGroups;
	bool m_bInitialGroupsLoaded;

	CNxIconButton m_nxbOK, m_nxbCancel, m_nxbConfigureGroups;
	CNxStatic m_nxsLabel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();

	BOOL m_bGreyList; // (j.gruber 2010-10-26 13:53) - PLID 40416
	BOOL m_bUpdateCurrentToolbar; // (j.gruber 2011-01-13 15:10) - PLID 40415

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOK();
	afx_msg void OnConfigureGroups();
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedSecurityGroups(short nFlags);
};
