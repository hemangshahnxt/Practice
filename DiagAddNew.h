#if !defined(AFX_DIAGADDNEW_H__F85BE538_3614_11D3_A36A_00C04F42E33B__INCLUDED_)
#define AFX_DIAGADDNEW_H__F85BE538_3614_11D3_A36A_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiagAddNew.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CDiagAddNew dialog

class CDiagAddNew : public CNxDialog
{
// Construction
public:
	CDiagAddNew(CWnd* pParent);   // standard constructor
	CString strCode, strDesc;

	long m_ID;

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CDiagAddNew)
	enum { IDD = IDD_DIAG_ADDNEW };
	CNxEdit	m_nxeditCode;
	CNxEdit	m_nxeditDesc;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiagAddNew)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDiagAddNew)
	virtual void OnOK();
	afx_msg void OnOkBtn();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIAGADDNEW_H__F85BE538_3614_11D3_A36A_00C04F42E33B__INCLUDED_)
