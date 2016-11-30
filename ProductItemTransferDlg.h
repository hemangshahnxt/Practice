#if !defined(AFX_PRODUCTITEMTRANSFERDLG_H__C810AFE5_6276_4DC6_B230_FB043E20DAD0__INCLUDED_)
#define AFX_PRODUCTITEMTRANSFERDLG_H__C810AFE5_6276_4DC6_B230_FB043E20DAD0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProductItemTransferDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProductItemTransferDlg dialog

class CProductItemTransferDlg : public CNxDialog
{
// Construction
public:
	CProductItemTransferDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UnselectedList, m_SelectedList, m_LocationCombo;

	long m_ProductID;	//the ProductT.ID
	BOOL m_bUseSerial, m_bUseExpDate;

// Dialog Data
	//{{AFX_DATA(CProductItemTransferDlg)
	enum { IDD = IDD_PRODUCT_ITEM_TRANSFER_DLG };
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnApplyLoc;
	CNxStatic		m_nxsTransferWarning;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProductItemTransferDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	BOOL Save();
	void SetColumnWidths();

	// Generated message map functions
	//{{AFX_MSG(CProductItemTransferDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblClickCellUnselectedItemList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedItemList(long nRowIndex, short nColIndex);
	afx_msg void OnBtnSelectOne();
	afx_msg void OnBtnSelectAll();
	afx_msg void OnBtnUnselectOne();
	afx_msg void OnBtnUnselectAll();
	afx_msg void OnRequeryFinishedUnselectedItemList(short nFlags);
	afx_msg void OnBtnApplyLoc();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRODUCTITEMTRANSFERDLG_H__C810AFE5_6276_4DC6_B230_FB043E20DAD0__INCLUDED_)
