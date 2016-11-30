#if !defined(AFX_INVORDERDATEFILTERPICKERDLG_H__6C973321_30F1_4EEC_9D84_8FAEED5E33A9__INCLUDED_)
#define AFX_INVORDERDATEFILTERPICKERDLG_H__6C973321_30F1_4EEC_9D84_8FAEED5E33A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvOrderDateFilterPickerDlg.h : header file
//

// (j.gruber 2008-06-30 10:01) - PLID 29205 - created for
/////////////////////////////////////////////////////////////////////////////
// CInvOrderDateFilterPickerDlg dialog

enum {
	DATE_CREATED = 1,
	DATE_RECEIVED = 2,
};

class CInvOrderDateFilterPickerDlg : public CNxDialog
{
// Construction
public:
	CInvOrderDateFilterPickerDlg(long nFilterID, CView* pParent = NULL);   // standard constructor
	void SetFilterID(long nID);
	long GetFilterID();

// Dialog Data
	//{{AFX_DATA(CInvOrderDateFilterPickerDlg)
	enum { IDD = IDD_INV_ORDER_DATE_FILTER_PICKER_DLG };
	CNxIconButton	m_CancelBtn;
	CNxIconButton	m_OKBtn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvOrderDateFilterPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pFilterList;
	long m_nFilterID;
	long m_nOrigFilterID;

	// Generated message map functions
	//{{AFX_MSG(CInvOrderDateFilterPickerDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenInvOrderDateFilterList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingInvOrderDateFilterList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVORDERDATEFILTERPICKERDLG_H__6C973321_30F1_4EEC_9D84_8FAEED5E33A9__INCLUDED_)
