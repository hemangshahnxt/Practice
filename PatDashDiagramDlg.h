#if !defined(AFX_PATDASHDIAGRAMDLG_H__7B8A3FB3_D585_4712_9F52_6D11789CB0E0__INCLUDED_)
#define AFX_PATDASHDIAGRAM_H__7B8A3FB3_D585_4712_9F52_6D11789CB0E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PatDashDiagramDlg.h : header file
//
#include "patientsrc.h"

// (j.gruber 2012-07-02 09:45) - PLID 51210 - created for
/////////////////////////////////////////////////////////////////////////////
// CPatDashDiagramDlg dialog

class CPatDashDiagramDlg : public CNxDialog
{
// Construction
public:
	CPatDashDiagramDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPatDashDiagramDlg)
	enum { IDD = IDD_PAT_DASH_COL_DIAGRAM };
	CNxIconButton	m_btnClose;	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatDashDiagramDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HBITMAP m_image;
	
	// Generated message map functions
	//{{AFX_MSG(CPatDashDiagramDlg)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATDASHDIAGRAM_H__7B8A3FB3_D585_4712_9F52_6D11789CB0E0__INCLUDED_)
