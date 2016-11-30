#if !defined(AFX_PROCESSRPTDLG_H__82A10A53_A1C2_11D3_9508_00C04F4C8415__INCLUDED_)
#define AFX_PROCESSRPTDLG_H__82A10A53_A1C2_11D3_9508_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcessRptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcessRptDlg dialog

class CProcessRptDlg : public CDialog
{
// Construction
public:
	CProcessRptDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcessRptDlg)
	enum { IDD = IDD_PROCESSINGDLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticDescriptionLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessRptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessRptDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESSRPTDLG_H__82A10A53_A1C2_11D3_9508_00C04F4C8415__INCLUDED_)
