#if !defined(AFX_UPDATEPENDINGCONFIGDLG_H__089FB312_2CB1_4319_9EA7_8B32A48CA22D__INCLUDED_)
#define AFX_UPDATEPENDINGCONFIGDLG_H__089FB312_2CB1_4319_9EA7_8B32A48CA22D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdatePendingConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdatePendingConfigDlg dialog

class CUpdatePendingConfigDlg : public CNxDialog
{
// Construction
public:
	CUpdatePendingConfigDlg(CWnd* pParent);   // standard constructor
	int m_nUpdateType; //1=Out, 2=NoShow

// Dialog Data
	//{{AFX_DATA(CUpdatePendingConfigDlg)
	enum { IDD = IDD_UPDATE_PENDING_CONFIG };
	NxButton	m_btnRemember;
	CNxIconButton	m_btnOut;
	CNxIconButton	m_btnNoShow;
	CNxIconButton	m_btnDoNothing;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdatePendingConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdatePendingConfigDlg)
	virtual void OnOK();
	afx_msg void OnNoshow();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEPENDINGCONFIGDLG_H__089FB312_2CB1_4319_9EA7_8B32A48CA22D__INCLUDED_)
