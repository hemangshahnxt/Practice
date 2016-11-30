#if !defined(AFX_REFPHYSPROCSDLG_H__9F70AD8D_EC2F_45BD_96B7_EF12A49E9A82__INCLUDED_)
#define AFX_REFPHYSPROCSDLG_H__9F70AD8D_EC2F_45BD_96B7_EF12A49E9A82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RefPhysProcsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRefPhysProcsDlg dialog

class CRefPhysProcsDlg : public CNxDialog
{
// Construction
public:
	CRefPhysProcsDlg(CWnd* pParent);   // standard constructor

	long m_nRefPhysID;

// Dialog Data
	//{{AFX_DATA(CRefPhysProcsDlg)
	enum { IDD = IDD_REF_PHYS_PROCS_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditProcWarningText;
	CNxStatic	m_nxstaticProcListCaption;
	NxButton	m_btnShowProcWarning;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRefPhysProcsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pProcList;
	// Generated message map functions
	//{{AFX_MSG(CRefPhysProcsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnShowProcWarning();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFPHYSPROCSDLG_H__9F70AD8D_EC2F_45BD_96B7_EF12A49E9A82__INCLUDED_)
