#if !defined(AFX_BLOCKTIMEWARNINGDLG_H__6D9A12D3_309A_45D3_9584_3D8735C94476__INCLUDED_)
#define AFX_BLOCKTIMEWARNINGDLG_H__6D9A12D3_309A_45D3_9584_3D8735C94476__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BlockTimeWarningDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBlockTimeWarningDlg dialog

class CBlockTimeWarningDlg : public CNxDialog
{
// Construction
public:
	CBlockTimeWarningDlg(CWnd* pParent);   // standard constructor

	long m_nAffectedCount, m_nTypeID;
	CString m_strTypeName;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CBlockTimeWarningDlg)
	enum { IDD = IDD_BLOCK_TIME_WARNING };
	CNxStatic	m_nxstaticBlockWarning;
	CNxIconButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBlockTimeWarningDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBlockTimeWarningDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BLOCKTIMEWARNINGDLG_H__6D9A12D3_309A_45D3_9584_3D8735C94476__INCLUDED_)
