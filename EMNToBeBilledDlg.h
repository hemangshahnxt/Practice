#if !defined(AFX_EMNTOBEBILLEDDLG_H__6ACA5F2C_57CF_4E8D_BDDE_234684B72C54__INCLUDED_)
#define AFX_EMNTOBEBILLEDDLG_H__6ACA5F2C_57CF_4E8D_BDDE_234684B72C54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMNToBeBilledDlg.h : header file
//

class CBillingModuleDlg;

/////////////////////////////////////////////////////////////////////////////
// CEMNToBeBilledDlg dialog

class CEMNToBeBilledDlg : public CNxDialog
{
// Construction
public:
	CEMNToBeBilledDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-06-05 12:37) - PLID 30255 - converted to be a datalist2
	NXDATALIST2Lib::_DNxDataListPtr m_List;

	CBillingModuleDlg *m_BillingDlg;

	// (j.jones 2008-07-03 15:48) - PLID 18354 - added m_btnPreview
// Dialog Data
	//{{AFX_DATA(CEMNToBeBilledDlg)
	enum { IDD = IDD_EMN_TO_BE_BILLED_DLG };
	CNxIconButton	m_btnPreview;
	CNxIconButton	m_btnOK;
	// (j.jones 2010-07-07 15:22) - PLID 36682 - added a refresh button
	CNxIconButton	m_btnRefresh;
	// (a.wilson 2013-06-07 15:24) - PLID 56784 - new control objects for sending to hl7.
	CNxIconButton	m_btnSendToHL7;
	CNxLabel		m_lblCheckAll, m_lblUncheckAll;
	// (a.wilson 2013-06-11 12:09) - PLID 57117 - checkbox to hide bills with only patient resp.
	NxButton		m_chkHidePatientResp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMNToBeBilledDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.wilson 2013-06-11 16:51) - PLID 57117
	scoped_ptr<class CHL7Client_Practice> m_pHL7Client;

	HICON m_hIconPreview; // (a.walling 2010-01-11 12:11) - PLID 31482
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;
	
	// (j.jones 2008-06-05 12:55) - PLID 30255 - made these functions require a row
	void GoToPatient(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void BillThisEMN(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (a.walling 2010-04-26 13:01) - PLID 35921
	void GoToEMN(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (a.walling 2010-04-26 14:18) - PLID 38364 - Is data dirty?
	// (j.jones 2010-07-07 16:15) - PLID 36682 - dirtiness is obsolete (aren't you glad you used Dial?)
	//bool m_bDirty;
	bool m_bShowState;
	void Refresh();

	BOOL m_bCreated;

	// (j.dinatale 2012-02-02 11:20) - PLID 47846 - member variable to store the last EMN selection before a refresh
	long m_nEMNIDToSelAfterRef;

	// (j.dinatale 2012-01-11 17:36) - PLID 47483 - show the charge split dialog
	long ShowChargeRespDialog(long nPatID, long nEMNID);

	// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
	// (z.manning 2012-09-10 15:22) - PLID 52543 - Added modified date
	void ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate);

	// (j.jones 2009-06-24 15:04) - PLID 24076 - added ability to link to existing bill
	void LinkWithExistingBill(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.jones 2009-09-15 10:02) - PLID 34717 - added ability to remove from the list
	void RemoveEMNFromList(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.jones 2008-06-05 12:37) - PLID 30255 - converted functions to use a datalist2
	// (j.jones 2008-07-03 15:48) - PLID 18354 - added OnBtnEmnToBeBilledPreview
	// Generated message map functions
	//{{AFX_MSG(CEMNToBeBilledDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnLeftClickUnbilledEmnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownUnbilledEmnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedUnbilledEmnList(short nFlags);
	afx_msg void OnBtnEmnToBeBilledPreview();
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	// (j.jones 2010-07-07 15:22) - PLID 36682 - added a refresh button
	afx_msg void OnBtnEmnToBeBilledRefresh();
	// (j.jones 2010-07-07 15:50) - PLID 36682 - track when the window is sized
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnSendBillToHl7();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedHidePatientRespCheck();
	void UpdateEMNBillsListFromClause(bool bForceRequery = false);
};

struct HL7EMNCharges
{
	long nEMNID;
	CString strEMNDescription, strPatientName;
	COleDateTime dtEMNDate;

	HL7EMNCharges()
	{
		nEMNID = 0;
		strEMNDescription = "";
		strPatientName = "";
		dtEMNDate = g_cdtNull;
	}

	HL7EMNCharges(long _nEMNID, CString _strEMNDescription, CString _strPatientName, COleDateTime _dtEMNDate)
	{
		nEMNID = _nEMNID;
		strEMNDescription = _strEMNDescription;
		strPatientName = _strPatientName;
		dtEMNDate = _dtEMNDate;
	}
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMNTOBEBILLEDDLG_H__6ACA5F2C_57CF_4E8D_BDDE_234684B72C54__INCLUDED_)
