#pragma once


// CLinksView view
// (d.thompson 2009-11-16) - PLID 36134 - Created

// (j.jones 2013-05-08 08:33) - PLID 56591 - removed the .h files for the child tabs
// (r.farnworth 2016-02-25 09:33) - PLID 68396 - Added COnlineVisitsDlg
class CExportDlg;
class CHL7BatchDlg;
class CTopsSearchDlg;
class CBoldLinkDlg;
class CDeviceConfigTabDlg;
class CSendLabsDlg;
class CReceiveLabsDlg;
class CDirectMessageReceivedDlg;
class CCancerCasesDlg;
class COnlineVisitsDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

class CLinksView : public CNxTabView
{
public:
	CLinksView();           // protected constructor used by dynamic creation
	~CLinksView();
	DECLARE_DYNCREATE(CLinksView)

	BOOL CheckPermissions();
	virtual	int Hotkey(int key);

#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void OnSelectTab(short newTab, short oldTab);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-08-07 14:22) - PLID 63179 - added an Ex handler
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	virtual void OnDraw(CDC* pDC);
	void ShowTabs();
	int ShowPrefsDlg();

	// (j.jones 2013-05-08 08:36) - PLID 56591 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	// (d.thompson 2009-11-16) - PLID 36301 - Moved Export, HL7, TOPS from financial module
	CExportDlg			&m_dlgExport;
	// (j.jones 2008-04-08 15:01) - PLID 29587 - added HL7 tab
	CHL7BatchDlg		&m_HL7BatchDlg;
	//(e.lally 2009-10-07) PLID 35803
	CTopsSearchDlg		&m_TopsSearchDlg;
	// (j.gruber 2010-04-28 12:27) - PLID 38337 - BOLD tab
	CBoldLinkDlg		&m_dlgBoldLink;
	// (d.lange 2010-05-07 15:36) - PLID 38536 - Devices tab
	CDeviceConfigTabDlg	&m_dlgDeviceConfigTab;
	// (a.vengrofski 2010-05-28 09:35) - PLID <38919> - Sending Labs
	CSendLabsDlg		&m_dlgSendLabsTab;
	// (a.vengrofski 2010-07-22 17:11) - PLID <38919> - Receive Labs
	CReceiveLabsDlg		&m_dlgReceiveLabsTab;
	// (j.camacho 2013-10-17 16:32) - PLID 
	CDirectMessageReceivedDlg &m_dlgDirectMessageTab;
	//TES 4/23/2014 - PLID 61854
	CCancerCasesDlg &m_dlgCancerCasesTab;
	// (r.farnworth 2016-02-25 09:32) - PLID 68396 - Create a new tab in the Links module that is for Online Visits
	COnlineVisitsDlg &m_dlgOnlineVisitsTab;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
