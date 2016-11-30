#if !defined(AFX_SELECTSOURCEPROCEDUREDLG_H__8B0B4B9D_8D65_4BF1_ADF6_CCF2EC719100__INCLUDED_)
#define AFX_SELECTSOURCEPROCEDUREDLG_H__8B0B4B9D_8D65_4BF1_ADF6_CCF2EC719100__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectSourceProcedureDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectSourceProcedureDlg dialog

class CSelectSourceProcedureDlg : public CNxDialog
{
// Construction
public:
	CSelectSourceProcedureDlg(CWnd* pParent);   // standard constructor
	long m_nSelectedId;

// Dialog Data
	//{{AFX_DATA(CSelectSourceProcedureDlg)
	enum { IDD = IDD_SELECT_SOURCE_PROCEDURE };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectSourceProcedureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pProcList;

	// Generated message map functions
	//{{AFX_MSG(CSelectSourceProcedureDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTSOURCEPROCEDUREDLG_H__8B0B4B9D_8D65_4BF1_ADF6_CCF2EC719100__INCLUDED_)
