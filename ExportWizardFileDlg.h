#if !defined(AFX_EXPORTWIZARDFILEDLG_H__9B6934B9_71BF_4608_8346_8F0BEC937846__INCLUDED_)
#define AFX_EXPORTWIZARDFILEDLG_H__9B6934B9_71BF_4608_8346_8F0BEC937846__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportWizardFileDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFileDlg dialog

class CExportWizardFileDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CExportWizardFileDlg)

// Construction
public:
	CExportWizardFileDlg();
	~CExportWizardFileDlg();

// Dialog Data
	//{{AFX_DATA(CExportWizardFileDlg)
	enum { IDD = IDD_EXPORT_WIZARD_FILE };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	CNxEdit	m_nxeditExportFileName;
	CNxStatic	m_nxstaticFilenameHelp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExportWizardFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CExportWizardFileDlg)
	afx_msg void OnExportFilePrompt();
	afx_msg void OnExportFileSpecific();
	afx_msg void OnChangeExportFileName();
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTWIZARDFILEDLG_H__9B6934B9_71BF_4608_8346_8F0BEC937846__INCLUDED_)
