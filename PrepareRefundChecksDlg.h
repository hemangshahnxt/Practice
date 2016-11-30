#if !defined(AFX_PREPAREREFUNDCHECKSDLG_H__9DDDAEE5_7426_4DA3_ACA2_D3053EF7155F__INCLUDED_)
#define AFX_PREPAREREFUNDCHECKSDLG_H__9DDDAEE5_7426_4DA3_ACA2_D3053EF7155F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrepareRefundChecksDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrepareRefundChecksDlg dialog

class CPrepareRefundChecksDlg : public CNxDialog
{
// Construction
public:
	CPrepareRefundChecksDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrepareRefundChecksDlg)
	enum { IDD = IDD_PREPARE_REFUND_CHECKS_DLG };
	CNxIconButton	m_btnSaveAndPreview;
	CNxIconButton	m_btnSaveAndClose;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnClear;
	CNxIconButton	m_btnAutoNumber;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectOne;
	CNxEdit	m_nxeditStartingCheckNo;
	CNxStatic	m_nxstaticUnselectedTotal;
	CNxStatic	m_nxstaticSelectedTotal;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_UnselectedList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrepareRefundChecksDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL Save(BOOL bPreviewReport);

	void UpdateTotals();

	// Generated message map functions
	//{{AFX_MSG(CPrepareRefundChecksDlg)
	afx_msg void OnDblClickCellUnselectedRefundList(long nRowIndex, short nColIndex);
	afx_msg void OnEditingFinishingUnselectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedUnselectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelectOneRefund();
	afx_msg void OnSelectAllRefunds();
	afx_msg void OnUnselectOneRefund();
	afx_msg void OnUnselectAllRefunds();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedUnselectedRefundList(short nFlags);
	afx_msg void OnDblClickCellSelectedRefundList(long nRowIndex, short nColIndex);
	afx_msg void OnEditingFinishingSelectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedSelectedRefundList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnBtnAutonumber();
	afx_msg void OnBtnClear();
	afx_msg void OnBtnSaveAndClose();
	afx_msg void OnBtnSaveAndPreview();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREPAREREFUNDCHECKSDLG_H__9DDDAEE5_7426_4DA3_ACA2_D3053EF7155F__INCLUDED_)
