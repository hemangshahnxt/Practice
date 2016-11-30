#include <afxcmn.h>

#include "client.h"
#if !defined(AFX_MIRRORLINK_H__A35DDAD3_48E8_11D3_A382_00C04F42E33B__INCLUDED_)
#define AFX_MIRRORLINK_H__A35DDAD3_48E8_11D3_A382_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MirrorLink.h : header file

namespace MirrorCross
{
	typedef struct
	{	NXDATALISTLib::_DNxDataListPtr mirrorList;
		NXDATALISTLib::_DNxDataListPtr nextechList;
		NXDATALISTLib::_DNxDataListPtr exportList;
		NXDATALISTLib::_DNxDataListPtr importList;
		HWND			statusBar;
		bool			stopRequest;
	} CROSSREF;
};

/////////////////////////////////////////////////////////////////////////////
// CMirrorLink dialog

// (c.haag 2008-02-06 10:09) - PLID 28622 - Added OnBtnLinkCommonPatients

class CMirrorLink : public CNxDialog
{
// Construction
public:
	CMirrorLink();
	virtual ~CMirrorLink();

	void CMirrorLink::DoFakeModal();

	HRESULT RequeryMirrorList61();
	void SetOtherUserChanged();
	void UpdateCount();

	MirrorCross::CROSSREF m_cross;
	bool m_bMirrorFinishedRequerying;
	long m_nStep;

	NXDATALISTLib::_DNxDataListPtr m_nextechList,
					m_mirrorList,
					m_importList,
					m_exportList;

// Dialog Data
	//{{AFX_DATA(CMirrorLink)
	enum { IDD = IDD_MIRROR_LINK };
	CNxIconButton	m_fixbadlinks;
	CNxIconButton	m_btnReindex;
	CNxIconButton	m_refreshMirrorButton;
	CNxIconButton	m_linkButton;
	CNxIconButton	m_unlinkButton;
	CNxIconButton	m_import;
	CNxIconButton	m_export;
	CNxIconButton	m_pracRemAll;
	CNxIconButton	m_pracRem;
	CNxIconButton	m_pracAdd;
	CNxIconButton	m_mirRemAll;
	CNxIconButton	m_mirRem;
	CNxIconButton	m_mirAdd;
	CNxStatic	m_nxstaticNextechCount;
	CNxStatic	m_nxstaticNextechCount2;
	CNxStatic	m_nxstaticMirrorCount;
	CNxStatic	m_nxstaticMirrorCount2;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorLink)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2008-03-05 10:46) - PLID 28622 - We now track whether the user
	// cancelled the Mirror requery
	bool m_bForcedRequeryStop;

	bool	m_failedToOpen;

	CTableChecker	m_pathChecker;

	CStatusBarCtrl m_statusBar;
	CProgressCtrl m_Progress;

	CWinThread		*m_pThread;

	bool m_otherUserChanged,
		 m_exporting,
		 m_bCancel61Requery;
	long m_nMirrorPatients;

	BOOL RefreshLink();
	void LoadMirrorList();
	void CrossRef();
	void IsThreadRunning();
	void KillThread();
	void Close();
	int GetImproperlyLinkedCount();
	void RepositionProgressBar();

	// Generated message map functions
	//{{AFX_MSG(CMirrorLink)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnDblClickCellNextech(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellExport(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellImport(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellMirror(long nRowIndex, short nColIndex);
	afx_msg void OnPracAdd();
	afx_msg void OnPracRemove();
	afx_msg void OnPracRemoveAll();
	afx_msg void OnMirAdd();
	afx_msg void OnMirRemove();
	afx_msg void OnMirRemoveAll();
	afx_msg void OnKillfocusOverride();
	afx_msg void OnExportBtn();
	afx_msg void OnImportBtn();
	afx_msg void OnRequeryFinishedNextech(short nFlags);
	afx_msg void OnRequeryFinishedMirror(short nFlags);
	afx_msg void OnUnlink();
	afx_msg void OnLink();
	afx_msg void OnBtnLinkCommonPatients();
	afx_msg void OnRefreshMirror();
	afx_msg void OnHelp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnRButtonUpNexTech(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnReindexMirror();
	afx_msg void OnBtnMirrorAdvopt();
	afx_msg void OnBtnFixbadlinks();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnStopMirrorLoad();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIRRORLINK_H__A35DDAD3_48E8_11D3_A382_00C04F42E33B__INCLUDED_)
