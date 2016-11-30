#if !defined(AFX_SELECTSENDERDLG_H__3069A627_1DCF_4CE1_94FC_19C7562CA23D__INCLUDED_)
#define AFX_SELECTSENDERDLG_H__3069A627_1DCF_4CE1_94FC_19C7562CA23D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectSenderDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CSelectSenderDlg dialog

class CSelectSenderDlg : public CNxDialog
{
// Construction
public:
	CSelectSenderDlg(CWnd* pParent);   // standard constructor

	//return fields
	CString m_strLast;
	CString m_strFirst;
	CString m_strMiddle;
	CString m_strEmail;
	CString m_strTitle;
	CString m_strSubjectMatter;

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CSelectSenderDlg)
	enum { IDD = IDD_SELECT_SENDER_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditSubjectMatter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectSenderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pUserList;

	// Generated message map functions
	//{{AFX_MSG(CSelectSenderDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTSENDERDLG_H__3069A627_1DCF_4CE1_94FC_19C7562CA23D__INCLUDED_)
