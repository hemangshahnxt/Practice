#if !defined(AFX_SELECTRECIPIENTSDLG_H__F8DE72D4_3A6C_11D5_B877_00C04F4C8415__INCLUDED_)
#define AFX_SELECTRECIPIENTSDLG_H__F8DE72D4_3A6C_11D5_B877_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectRecipientsDlg.h : header file
//
#include <afxtempl.h>
/////////////////////////////////////////////////////////////////////////////
// CSelectRecipientsDlg dialog

class CSelectRecipientsDlg : public CNxDialog
{
// Construction
public:
	CSelectRecipientsDlg(CWnd* pParent);   // standard constructor
	CDWordArray m_dwRecipientsAry;
	CStringArray m_strRecipientsAry;

// Dialog Data
	//{{AFX_DATA(CSelectRecipientsDlg)
	enum { IDD = IDD_SELECT_RECIPIENTS };
	CNxIconButton	m_buLLeft;
	CNxIconButton	m_buRRight;
	CNxIconButton	m_buRight;
	CNxIconButton	m_buLeft;
	NXDATALISTLib::_DNxDataListPtr m_pAvail;
	NXDATALISTLib::_DNxDataListPtr m_pSelect;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectRecipientsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectRecipientsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRight();
	afx_msg void OnLeft();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDblClickCellAvail(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelect(long nRowIndex, short nColIndex);
	afx_msg void OnRright();
	afx_msg void OnLleft();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTRECIPIENTSDLG_H__F8DE72D4_3A6C_11D5_B877_00C04F4C8415__INCLUDED_)
