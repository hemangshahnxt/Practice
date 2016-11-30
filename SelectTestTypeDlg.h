#if !defined(AFX_SELECTTESTTYPEDLG_H__1C08155B_F3A1_46C9_80F3_1875A26C4CB3__INCLUDED_)
#define AFX_SELECTTESTTYPEDLG_H__1C08155B_F3A1_46C9_80F3_1875A26C4CB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectTestTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectTestTypeDlg dialog

class CSelectTestTypeDlg : public CNxDialog
{
// Construction
public:
	CSelectTestTypeDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_pTestsList;
	long m_nSelectedTest;

// Dialog Data
	//{{AFX_DATA(CSelectTestTypeDlg)
	enum { IDD = IDD_SELECT_TEST_TYPE };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticTestTypeLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectTestTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectTestTypeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnModifyTestTypesBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTTESTTYPEDLG_H__1C08155B_F3A1_46C9_80F3_1875A26C4CB3__INCLUDED_)
