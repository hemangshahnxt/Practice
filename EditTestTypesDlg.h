#if !defined(AFX_EDITTESTTYPESDLG_H__874A3923_D5A0_4B2F_ACF8_CA79C5F5574C__INCLUDED_)
#define AFX_EDITTESTTYPESDLG_H__874A3923_D5A0_4B2F_ACF8_CA79C5F5574C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditTestTypesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditTestTypesDlg dialog

class CEditTestTypesDlg : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_pTestsList;

	CEditTestTypesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditTestTypesDlg)
	enum { IDD = IDD_TEST_TYPES_DLG };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditTestTypesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditTestTypesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEditingFinishedTestTypesList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITTESTTYPESDLG_H__874A3923_D5A0_4B2F_ACF8_CA79C5F5574C__INCLUDED_)
