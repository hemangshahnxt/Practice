#if !defined(AFX_MARKETRANGECONFIGDLG_H__72E8D4A8_AF3E_41E5_82C1_00285F40C56D__INCLUDED_)
#define AFX_MARKETRANGECONFIGDLG_H__72E8D4A8_AF3E_41E5_82C1_00285F40C56D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MarketRangeConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMarketRangeConfigDlg dialog

class CMarketRangeConfigDlg : public CNxDialog
{
// Construction
public:
	CMarketRangeConfigDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pRangeList;
	long GetNewIDFromList(long nCol);
	CDWordArray m_aryAddList;
	CDWordArray m_aryRemoveList;
	CDWordArray m_aryChangeList;
	BOOL m_bNeedToSave;
	BOOL Save();
	BOOL IDInList(CDWordArray *pAry, long nID);
	void Resort();
	BOOL IsValidData();
	
	

// Dialog Data
	//{{AFX_DATA(CMarketRangeConfigDlg)
	enum { IDD = IDD_MARKET_RETENTION_RANGE_CONFIG_DLG };
	CNxIconButton	m_MoveUp;
	CNxIconButton	m_MoveDown;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnRestoreDefaults;
	CNxIconButton	m_btnAddRange;
	CNxIconButton	m_btnRemoveRange;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMarketRangeConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMarketRangeConfigDlg)
	afx_msg void OnAddRange();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	afx_msg void OnRemoveRange();
	afx_msg void OnEditingFinishingRangeListEdit(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedRangeListEdit(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRestoreDefaults();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MARKETRANGECONFIGDLG_H__72E8D4A8_AF3E_41E5_82C1_00285F40C56D__INCLUDED_)
