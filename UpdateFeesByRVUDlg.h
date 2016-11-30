#if !defined(AFX_UPDATEFEESBYRVUDLG_H__9863040A_C281_41E4_9DCE_5F9851A8B065__INCLUDED_)
#define AFX_UPDATEFEESBYRVUDLG_H__9863040A_C281_41E4_9DCE_5F9851A8B065__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdateFeesByRVUDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdateFeesByRVUDlg dialog

class CUpdateFeesByRVUDlg : public CNxDialog
{
// Construction
public:
	CUpdateFeesByRVUDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UnselectedList, m_SelectedList;

	long m_FeeScheduleID;

	double m_dblConversionFactor;

	void ClearUnselectedNewFees();

// Dialog Data
	//{{AFX_DATA(CUpdateFeesByRVUDlg)
	enum { IDD = IDD_UPDATE_FEES_BY_RVU_DLG };
	CNxIconButton	m_btnCalculatePrices;
	CNxIconButton	m_btnApplyNewFees;
	CNxIconButton	m_btnMoveOneCPTUp;
	CNxIconButton	m_btnMoveOneCPTDown;
	CNxIconButton	m_btnMoveAllCPTUp;
	CNxIconButton	m_btnMoveAllCPTDown;
	CNxEdit	m_nxeditConversionFactor;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateFeesByRVUDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdateFeesByRVUDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnMoveOneCptDown();
	afx_msg void OnBtnMoveAllCptDown();
	afx_msg void OnMoveOneCptUp();
	afx_msg void OnBtnMoveAllCptUp();
	afx_msg void OnDblClickCellUnselectedCptRvuList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedCptRvuList(long nRowIndex, short nColIndex);
	afx_msg void OnBtnCalculatePrices();
	afx_msg void OnBtnApplyNewFees();
	afx_msg void OnChangeConversionFactor();
	afx_msg void OnKillfocusConversionFactor();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEFEESBYRVUDLG_H__9863040A_C281_41E4_9DCE_5F9851A8B065__INCLUDED_)
