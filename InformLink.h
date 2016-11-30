//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_INFORMLINK_H__5D07C783_832D_11D3_AD73_00104B318376__INCLUDED_)
#define AFX_INFORMLINK_H__5D07C783_832D_11D3_AD73_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InformLink.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

typedef struct
{	NXDATALISTLib::_DNxDataListPtr informList;
	NXDATALISTLib::_DNxDataListPtr practiceList;
	HWND			statusBar;
	bool			stopRequest;
} InformCrossRef;

/////////////////////////////////////////////////////////////////////////////
// CInformLink dialog

class CInformLink : public CNxDialog
{
// Construction
public:
	//TES 5/24/2013 - PLID 56860 - We've only been checking the Inform version when first browsing to the database, in other cases IsFullVersion was
	// never being initialized.  Track whether we've checked the Inform version
	BOOL m_bCheckedVersion;
	BOOL IsFullVersion;
	BOOL CheckInformVersion();
	BOOL m_InformRequeryDone;
	BOOL m_PracticeRequeryDone;

	CString m_informPath;

	ADODB::_ConnectionPtr informDB;
	NXDATALISTLib::_DNxDataListPtr m_practiceList;
	NXDATALISTLib::_DNxDataListPtr m_informList;
	NXDATALISTLib::_DNxDataListPtr m_practiceSelected;
	NXDATALISTLib::_DNxDataListPtr m_informSelected;
	CInformLink();
	~CInformLink();
	void KillThread();
	BOOL EnsureData();
	BOOL TryExport(long practiceID, CString &informPath, long &ChartNum, long &newID, BOOL &bStop);//doesn't open informs mdb
// Dialog Data
	//{{AFX_DATA(CInformLink)
	enum { IDD = IDD_INFORM_LINK };
	CNxIconButton	m_btnLinkPrefixes;
	CNxIconButton	m_link;
	CNxIconButton	m_Close;
	CNxIconButton	m_unlink;
	CNxIconButton	m_getInformPath;
	CNxIconButton	m_SendToInform;
	CNxIconButton	m_SendToPractice;
	CNxIconButton	m_infRemoveAll;
	CNxIconButton	m_infRemove;
	CNxIconButton	m_infAdd;
	CNxIconButton	m_pracRemoveAll;
	CNxIconButton	m_pracRemove;
	CNxIconButton	m_pracAdd;
	CNxStatic	m_nxstaticLabel2;
	CNxStatic	m_nxstaticPracCount;
	CNxStatic	m_nxstaticLabel8;
	CNxStatic	m_nxstaticPracSelectCount;
	CNxStatic	m_nxstaticLabel7;
	CNxStatic	m_nxstaticInfCount;
	CNxStatic	m_nxstaticLabel1;
	CNxStatic	m_nxstaticInfSelectCount;
	CNxStatic	m_nxstaticLabel3;
	NxButton	m_btnDisable;
	NxButton	m_btnNewPat;
	NxButton	m_btnShowAdvanced;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInformLink)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CStatusBarCtrl m_statusBar;

	InformCrossRef m_cross;

	CWinThread		*m_pThread;

	CBrush m_pracBrush;
	CBrush m_informBrush;

	void CrossReference();
	// Generated message map functions
	//{{AFX_MSG(CInformLink)
	afx_msg void OnInfAdd();
	afx_msg void OnInfRemAll();
	afx_msg void OnInfRem();
	afx_msg void OnPracRemAll();
	afx_msg void OnPracAdd();
	afx_msg void OnPracRem();
	afx_msg void OnExport();
	afx_msg void OnImport();
	virtual BOOL OnInitDialog();
	afx_msg void GetInformPath();
	afx_msg void OnRequeryFinishedPracPatients(short nFlags);
	afx_msg void OnRequeryFinishedInformPatients(short nFlags);
	afx_msg void OnDblClickCellInformPatients(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellPracPatients(long nRowIndex, short nColIndex);
	afx_msg void OnShowAdvanced();
	afx_msg void OnLink();
	afx_msg void OnUnlink();
	afx_msg void OnDblClickCellPracPatientsSelected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellInformPatientsSelected(long nRowIndex, short nColIndex);
	//afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNewPatCheck();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLinkPrefixes();
	afx_msg void OnDisableLinkCheck();
	afx_msg LRESULT OnEnsureData(WPARAM wparam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFORMLINK_H__5D07C783_832D_11D3_AD73_00104B318376__INCLUDED_)
