#if !defined(AFX_ASDDLG_H__9B0E2D2E_6A24_4C45_ACC5_549A8A58AE71__INCLUDED_)
#define AFX_ASDDLG_H__9B0E2D2E_6A24_4C45_ACC5_549A8A58AE71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ASDDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CASDDlg dialog
// (a.walling 2008-07-03 15:44) - PLID 30498 - Generic dialog to warn and require a checkbox be selected before continuing

class CASDDlg : public CNxDialog
{
// Construction
public:
	CASDDlg(CWnd* pParent);   // standard constructor

	// (a.walling 2010-01-13 14:16) - PLID 31253 - Option to allow cancel
	// (a.walling 2010-01-13 14:51) - PLID 31253 - Set parameters for this dialog
	void SetParams(LPCTSTR strText, LPCTSTR strWindowTitle, LPCTSTR strTitle, LPCTSTR strAgree = NULL, bool bAllowCancel = true);

	CString m_strText;
	CString m_strTitle;
	CString m_strWindowTitle;
	CString m_strAgree;

// Dialog Data
	//{{AFX_DATA(CASDDlg)
	enum { IDD = IDD_ASD_DLG };
	CNxIconButton	m_nxibOK;
	CNxIconButton	m_nxibCancel;
	// (a.walling 2010-01-13 14:16) - PLID 31253 - 'Other' button
	CNxIconButton	m_nxibOther;
	CNxStatic	m_nxsText;
	CNxStatic	m_nxsTitle;
	NxButton	m_nxbAgree;
	CNxColor	m_nxcolor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CASDDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.walling 2010-01-13 14:47) - PLID 31253 - Update the other button text / style
	virtual void UpdateOtherButton();
	// (a.walling 2010-01-13 14:55) - PLID 31253 - Other button was clicked
	virtual void OnOtherButtonClicked(); 

	// (a.walling 2010-01-13 14:16) - PLID 31253 - Option to allow cancel
	bool m_bAllowCancel;

	// Generated message map functions
	//{{AFX_MSG(CASDDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnOther(); // (a.walling 2010-01-13 14:55) - PLID 31253 - Other button was clicked
	afx_msg void OnCheckAgree();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASDDLG_H__9B0E2D2E_6A24_4C45_ACC5_549A8A58AE71__INCLUDED_)
