//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_APPLYLISTDLG_H__65102141_410A_11D2_AB75_00A0246CDDA1__INCLUDED_)
#define AFX_APPLYLISTDLG_H__65102141_410A_11D2_AB75_00A0246CDDA1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ApplyListDlg.h : header file
//
#define APPLY_LIST_COMBO_SEL_EXISTING	0
#define APPLY_LIST_COMBO_SEL_AVAILABLE	1

/////////////////////////////////////////////////////////////////////////////
// CApplyListDlg dialog

class CApplyListDlg : public CNxDialog
{
// Construction
public:
	~CApplyListDlg();
	NXDATALISTLib::_DNxDataListPtr m_List;
	CApplyListDlg(CWnd* pParent);   // standard constructor

	CString m_strClickedType;
	int m_iDestID; // Could be a bill or charge

// Dialog Data
	//{{AFX_DATA(CApplyListDlg)
	enum { IDD = IDD_APPLY_LIST_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnNewPay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApplyListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CApplyListDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLButtonDownList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnAddPayment();
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void OnOK();
	void FillAppliesList();

	CBrush m_brush;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLYLISTDLG_H__65102141_410A_11D2_AB75_00A0246CDDA1__INCLUDED_)
