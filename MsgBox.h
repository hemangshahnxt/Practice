#if !defined(AFX_MSGBOX_H__BE3F53E4_9C32_11D3_9506_00C04F4C8415__INCLUDED_)
#define AFX_MSGBOX_H__BE3F53E4_9C32_11D3_9506_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMsgBox dialog

class CMsgBox : public CNxDialog
{
// Construction
public:
	CString msg;
	CString m_strWindowText;
	// (r.gonet 05/01/2014) - PLID 49432 - true if the message box should word wrap the text.
	// false if it should not. true by default.
	bool m_bWordWrap;
	// (r.gonet 05/01/2014) - PLID 49432 - The font that the message box will use in displaying
	// the text. NULL to use the default font. Caller is responsible for memory deallocation. 
	CFont *m_pFont;
	// (r.gonet 05/16/2014) - PLID 49432 - Need to retain in order to free the edit box if we dynamically create it.
	CEdit *m_pDynamicEdit;
	CMsgBox(CWnd* pParent);   // standard constructor
	// (r.gonet 05/16/2014) - PLID 49432 - Destructor
	virtual ~CMsgBox();
	BOOL m_bAllowCancel;

// Dialog Data
	// (a.walling 2009-04-20 15:06) - PLID 33951 - Changed from CNxEdit back to CEdit
	//{{AFX_DATA(CMsgBox)
	enum { IDD = IDD_MSGDLG };
	CEdit	m_nxeditMsgbox;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (z.manning, 05/16/2008) - PLID 30050 - Added OnCtlColor
	// Generated message map functions
	//{{AFX_MSG(CMsgBox)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGBOX_H__BE3F53E4_9C32_11D3_9506_00C04F4C8415__INCLUDED_)
