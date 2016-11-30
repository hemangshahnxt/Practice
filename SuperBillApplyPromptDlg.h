#if !defined(AFX_SUPERBILLAPPLYPROMPTDLG_H__58788FEA_01A8_4569_8FF7_079768931AB3__INCLUDED_)
#define AFX_SUPERBILLAPPLYPROMPTDLG_H__58788FEA_01A8_4569_8FF7_079768931AB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SuperBillApplyPromptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSuperBillApplyPromptDlg dialog

class CSuperBillApplyPromptDlg : public CNxDialog
{
// Construction
public:
	CSuperBillApplyPromptDlg(CWnd* pParent);   // standard constructor

	long m_nPatientID;	//id of the patient we are viewing
	NXDATALISTLib::_DNxDataListPtr m_listIDs;
	long m_nFinalID;	//id to be returned from the dialog

// Dialog Data
	//{{AFX_DATA(CSuperBillApplyPromptDlg)
	enum { IDD = IDD_SUPERBILL_APPLY_PROMPT };
	CNxIconButton	m_btnOK;
	NxButton m_checkIncludeApplies;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSuperBillApplyPromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSuperBillApplyPromptDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnIncludeApplies();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedSuperbillList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUPERBILLAPPLYPROMPTDLG_H__58788FEA_01A8_4569_8FF7_079768931AB3__INCLUDED_)
