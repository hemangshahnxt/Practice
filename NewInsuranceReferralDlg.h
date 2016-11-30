#if !defined(AFX_NEWINSURANCEREFERRALDLG_H__6D0D9B6B_C3ED_481F_A607_AE0D3FA64B4C__INCLUDED_)
#define AFX_NEWINSURANCEREFERRALDLG_H__6D0D9B6B_C3ED_481F_A607_AE0D3FA64B4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewInsuranceReferralDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewInsuranceReferralDlg dialog

class CNewInsuranceReferralDlg : public CNxDialog
{
// Construction
public:
	void DisplayDiagInfo();
	void DisplayCPTInfo();
	CNewInsuranceReferralDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr 
					m_CPTCodeCombo,
					m_ProviderCombo,
					m_LocationCombo;

    // (a.levy 2014-02-20 11:15) - PLID - 60768  - Added diagnosis search control & List
    NXDATALIST2Lib::_DNxDataListPtr m_diagInsSearch;
    NXDATALIST2Lib::_DNxDataListPtr m_InsDiagList;

  
	long m_InsuredPartyID;
	long m_ID;
	long m_LocationID;
	CString m_strAuthNum;
	COleDateTime m_dtStart;
	COleDateTime m_dtEnd;
	void Load();
	void OnMultiDiag();
	void OnMultiCPT();

	CString m_strCPTIDs, m_strDiagIDs;

	bool m_bEditing, m_bAllowChange;

// Dialog Data
	//{{AFX_DATA(CNewInsuranceReferralDlg)
	enum { IDD = IDD_NEW_INSURANCE_REFERRAL };
	CDateTimePicker	m_dtStartDate;
	CDateTimePicker	m_dtEndDate;
	CNxEdit	m_nxeditAuthNum;
	CNxEdit	m_nxeditNumVisits;
	CNxEdit	m_nxeditComments;
    // (a.levy 2014-02-20 11:15) - PLID - 60768 Remove this for 
	//CNxStatic	m_nxstaticMultiDiagLabel;
	CNxStatic	m_nxstaticMultiCptLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewInsuranceReferralDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void EnableControls(bool bEnable);
	CString m_strDiagList, m_strCPTList;

	void DrawDiagLabel(CDC *pdc);
	void DrawCPTLabel(CDC *pdc);
	CString GetStringOfCPTCodes();
	CString GetStringOfDiagCodes();
    // (a.levy 2014-02-20 11:15) - PLID 60768  - Added Diagnosis search box
    void OnSelectInsuranceRefDiagnosisSearch(LPDISPATCH lpRow);
    // (a.levy 2014-02-20 11:15) - PLID 60768 - update the Datalist 
    void UpdateDiagList(long DiagID,CString strDiagCode, CString strDiagDesc);
    void UpdateDiagList(); //overload
    // (a.levy 2014-03-05 12:22) - PLID 60868 - Right Click Functionality to remove Diag Codes from list
    void OnRightClickDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	BOOL m_bIsCPTListHidden, m_bIsDiagListHidden;

	void DoClickHyperlink(UINT nFlags, CPoint point);

	CRect m_rcMultiDiagLabel, m_rcMultiCPTLabel;
 
  

	// Generated message map functions
	//{{AFX_MSG(CNewInsuranceReferralDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenCptcodeCombo(long nRow);
	afx_msg void OnSelChosenDiagcodeCombo(long nRow);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRequeryFinishedCptcodeCombo(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWINSURANCEREFERRALDLG_H__6D0D9B6B_C3ED_481F_A607_AE0D3FA64B4C__INCLUDED_)
