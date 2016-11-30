#if !defined(AFX_TRACKINGCONVERSIONCONFIGDLG_H__069F8115_FF1A_47B3_A853_3CF4712E519B__INCLUDED_)
#define AFX_TRACKINGCONVERSIONCONFIGDLG_H__069F8115_FF1A_47B3_A853_3CF4712E519B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrackingConversionConfigDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CTrackingConversionConfigDlg dialog

// (j.gruber 2007-08-17 15:18) - PLID 27091 - Created For
class CTrackingConversionConfigDlg : public CNxDialog
{
// Construction
public:
	CTrackingConversionConfigDlg(CWnd* pParent);   // standard constructor
	CBrush m_brush;

	NXDATALIST2Lib::_DNxDataListPtr m_pConversionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLadderList;
	NXDATALIST2Lib::_DNxDataListPtr m_pBeginStepList;
	NXDATALIST2Lib::_DNxDataListPtr m_pEndStepList;

	void CheckButtonStatus();


// Dialog Data
	//{{AFX_DATA(CTrackingConversionConfigDlg)
	enum { IDD = IDD_TRACKING_CONVERSION_CONFIG_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrackingConversionConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTrackingConversionConfigDlg)
	afx_msg void OnTrackConfigAdd();
	afx_msg void OnRButtonUpTrackConfigConversions(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenTrackConfigLadderList(LPDISPATCH lpRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedTrackConfigLadderList(short nFlags);
	afx_msg void OnEditingFinishedTrackConfigConversions(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingTrackConfigConversions(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedTrackConfigStartStep(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnTrackConfigDown();
	afx_msg void OnTrackConfigUp();
	afx_msg void OnSelChangedTrackConfigConversions(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRemoveTrackingConversion();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRACKINGCONVERSIONCONFIGDLG_H__069F8115_FF1A_47B3_A853_3CF4712E519B__INCLUDED_)
