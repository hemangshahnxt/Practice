#if !defined(AFX_BACKUPPROGESSDLG_H__4973A060_D7DE_4559_B743_1D646E31C7D3__INCLUDED_)
#define AFX_BACKUPPROGESSDLG_H__4973A060_D7DE_4559_B743_1D646E31C7D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BackupProgessDlg.h : header file
//

// (z.manning, 05/21/2008) - PLID 30050 - Created
/////////////////////////////////////////////////////////////////////////////
// CBackupProgessDlg dialog

class CBackupProgessDlg : public CDialog
{
// Construction
public:
	CBackupProgessDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CBackupProgessDlg)
	enum { IDD = IDD_BACKUP_PROGRESS };
	CNxStatic	m_nxstaticLastMsg;
	CNxStatic	m_nxstaticLastMsg2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBackupProgessDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CBackupProgessDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BACKUPPROGESSDLG_H__4973A060_D7DE_4559_B743_1D646E31C7D3__INCLUDED_)
