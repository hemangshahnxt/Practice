#if !defined(AFX_IMPLEMENTATIONLADDERPICKERDLG_H__6705E117_1802_4B9A_89D8_6EFC7AFD94CF__INCLUDED_)
#define AFX_IMPLEMENTATIONLADDERPICKERDLG_H__6705E117_1802_4B9A_89D8_6EFC7AFD94CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImplementationLadderPickerDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImplementationLadderPickerDlg dialog

class CImplementationLadderPickerDlg : public CNxDialog
{
// Construction
public:
	CImplementationLadderPickerDlg(CWnd* pParent);   // standard constructor
	long m_nLadderID;

// Dialog Data
	//{{AFX_DATA(CImplementationLadderPickerDlg)
	enum { IDD = IDD_IMPLEMENTATION_LADDER_PICKER_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImplementationLadderPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pLadderList;
	

	// Generated message map functions
	//{{AFX_MSG(CImplementationLadderPickerDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelChangingImplementationLadderList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPLEMENTATIONLADDERPICKERDLG_H__6705E117_1802_4B9A_89D8_6EFC7AFD94CF__INCLUDED_)
