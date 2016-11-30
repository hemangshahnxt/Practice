#if !defined(AFX_IMPLEMENTATIONEDITSPECIALISTDLG_H__18C3DD07_F496_4534_9D87_7D307AE6F8F2__INCLUDED_)
#define AFX_IMPLEMENTATIONEDITSPECIALISTDLG_H__18C3DD07_F496_4534_9D87_7D307AE6F8F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImplementationEditSpecialistDlg.h : header file
// (j.gruber 2008-04-02 16:50) - PLID 28979 - created for
//
/////////////////////////////////////////////////////////////////////////////
// CImplementationEditSpecialistDlg dialog

class CImplementationEditSpecialistDlg : public CNxDialog
{
// Construction
public:
	CImplementationEditSpecialistDlg(CWnd* pParent);   // standard constructor
	NXDATALIST2Lib::_DNxDataListPtr m_pUsersList;

// Dialog Data
	//{{AFX_DATA(CImplementationEditSpecialistDlg)
	enum { IDD = IDD_IMPLEMENTATION_EDIT_SPECIALISTS };
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImplementationEditSpecialistDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImplementationEditSpecialistDlg)
	afx_msg void OnEditingFinishedUsersList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingUsersList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnOK();
	void OnCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPLEMENTATIONEDITSPECIALISTDLG_H__18C3DD07_F496_4534_9D87_7D307AE6F8F2__INCLUDED_)
