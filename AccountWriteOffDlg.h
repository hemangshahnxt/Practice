#if !defined(AFX_ACCOUNTWRITEOFFDLG_H__9BDB82CC_D662_4F81_8D19_F750C1C19575__INCLUDED_)
#define AFX_ACCOUNTWRITEOFFDLG_H__9BDB82CC_D662_4F81_8D19_F750C1C19575__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AccountWriteOffDlg.h : header file
//

// (j.jones 2008-06-27 13:37) - PLID 27647 - created

/////////////////////////////////////////////////////////////////////////////
// CAccountWriteOffDlg dialog

class CAccountWriteOffDlg : public CNxDialog
{
// Construction
public:
	CAccountWriteOffDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAccountWriteOffDlg)
	enum { IDD = IDD_ACCOUNT_WRITE_OFF_DLG };
	CNxIconButton	m_btnEditPayCat;
	CNxEdit	m_editAdjDesc;
	CNxStatic	m_nxstaticPatientTotalLabel;
	CNxStatic	m_nxstaticResultsInfoLabel;
	NxButton	m_radioAllBalances;
	NxButton	m_radioPatBalances;
	NxButton	m_radioInsBalances;
	CNxStatic	m_nxstaticPatientTotal;
	CDateTimePicker	m_dtTo;
	CDateTimePicker	m_dtFrom;
	CNxStatic m_nxstaticSearchLabel;
	CNxStatic m_nxstaticAdjInfoLabel;
	CNxEdit	m_editLessThanAmount;
	CNxEdit	m_editDaysOld;
	NxButton	m_radioLastSentDate;
	NxButton	m_radioBillServiceDate;
	NxButton	m_radioBillInputDate;
	NxButton	m_radioLastPaymentDate;
	NxButton	m_radioGreaterThanDays;
	NxButton	m_radioDateRange;
	NxButton	m_checkUseDateFilters;
	NxButton	m_checkBalanceLessThan;
	CNxIconButton	m_btnEditAdjDesc;
	NxButton	m_radioSearchByBill;
	NxButton	m_radioSearchByAccount;
	CNxIconButton	m_btnDisplayResults;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnAdjust;
	// (j.jones 2013-08-19 16:59) - PLID 58088 - added an assignment date filter
	NxButton	m_radioAssignmentDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccountWriteOffDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr	m_ResultsList,
								m_DescriptionCombo,
								m_CategoryCombo,
								m_GroupCodeCombo,
								m_ReasonCodeCombo;

	BOOL m_bAdjustmentsMade;	//tracks whether any adjustments were made in the dialog session

	void OnChangeSearchOptions();
	void OnChangeDateFilterRadio();

	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
	scoped_ptr<CIncreaseCommandTimeout> m_pIncreaseCommandTimeout;

	CString GetFromClauseByAccount();
	CString GetFromClauseByBill();

	void UpdatePatientTotalLabel();

	void CreateMergeGroup(CString strPatientIDs);

	// Generated message map functions
	//{{AFX_MSG(CAccountWriteOffDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAdjust();
	afx_msg void OnBtnWriteOffClose();
	afx_msg void OnBtnDisplayResults();
	afx_msg void OnRadioWriteOffByAccount();
	afx_msg void OnRadioWriteOffByBill();
	afx_msg void OnEditAdjDesc();
	afx_msg void OnCheckUseDateRange();
	afx_msg void OnCheckUseBalanceLessThan();
	afx_msg void OnRadioWriteOffGreaterThanDays();
	afx_msg void OnRadioWriteOffBetweenDateRange();
	afx_msg void OnRequeryFinishedWriteOffList(short nFlags);
	afx_msg void OnRadioWriteOffAllBalances();
	afx_msg void OnRadioWriteOffPatientBalances();
	afx_msg void OnRadioWriteOffInsuranceBalances();
	afx_msg void OnRButtonDownWriteOffList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChosenAdjDescription(LPDISPATCH lpRow);
	afx_msg void OnBtnEditPayCat();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACCOUNTWRITEOFFDLG_H__9BDB82CC_D662_4F81_8D19_F750C1C19575__INCLUDED_)
