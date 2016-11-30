#if !defined(AFX_SELECTBILLDLG_H__63ABCC41_6FDD_46E8_AB15_8A3B2286D2E4__INCLUDED_)
#define AFX_SELECTBILLDLG_H__63ABCC41_6FDD_46E8_AB15_8A3B2286D2E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectBillDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectBillDlg dialog

class CSelectBillDlg : public CNxDialog
{
// Construction
public:
	CSelectBillDlg(CWnd* pParent);   // standard constructor
	CString m_strWhere;
	long m_nBillID;

// Dialog Data
	//{{AFX_DATA(CSelectBillDlg)
	enum { IDD = IDD_SELECT_BILL };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectBillDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pBillList;
	// Generated message map functions
	//{{AFX_MSG(CSelectBillDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRequeryFinishedBillList(short nFlags);
	afx_msg void OnSelChosenBillList(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTBILLDLG_H__63ABCC41_6FDD_46E8_AB15_8A3B2286D2E4__INCLUDED_)
