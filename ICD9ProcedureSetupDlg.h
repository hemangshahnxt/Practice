#if !defined(AFX_ICD9PROCEDURESETUPDLG_H__36D45802_5E01_45BB_950C_41E4B8697CA6__INCLUDED_)
#define AFX_ICD9PROCEDURESETUPDLG_H__36D45802_5E01_45BB_950C_41E4B8697CA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (a.walling 2007-06-20 12:09) - PLID 26412 - Dialog to set up the ICD9 Procedure Codes

// ICD9ProcedureSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CICD9ProcedureSetupDlg dialog

class CICD9ProcedureSetupDlg : public CNxDialog
{
// Construction
public:
	CICD9ProcedureSetupDlg(CWnd* pParent);   // standard constructor

	enum EICD9ProcedureListColumns {
		eipID,
		eipCode,
		eipDescription
	};

	// (z.manning, 05/01/2008) - PLID 29860 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CICD9ProcedureSetupDlg)
	enum { IDD = IDD_ICD9V3_SETUP };
	CNxEdit	m_nxeditEditIcd9v3Code;
	CNxEdit	m_nxeditEditIcd9v3Description;
	CNxIconButton	m_btnIcd9v3Add;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnIcd9v3LinkToMultiple;
	CNxIconButton	m_btnIcd9v3LinkToUnlinked;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CICD9ProcedureSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

	BOOL VerifyCode(CString strCode, long nID = -1);
	void LinkToMultiple(BOOL bUnlinkedOnly = FALSE);

	// Generated message map functions
	//{{AFX_MSG(CICD9ProcedureSetupDlg)
	afx_msg void OnCurSelWasSetList();
	// (r.gonet 03/21/2014) - PLID 61240 - Removed unused event handler OnRequeryFinishedList
	afx_msg void OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnLinkToMultiple();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeEditIcd9v3Code();
	afx_msg void OnRButtonDownListIcd9v3Setup(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBtnIcd9v3LinkToUnlinked();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ICD9PROCEDURESETUPDLG_H__36D45802_5E01_45BB_950C_41E4B8697CA6__INCLUDED_)
