#if !defined(AFX_CONFIGURESUPERBILLADDFILEDLG_H__BC2E2453_F642_4A62_A697_2B307E61C785__INCLUDED_)
#define AFX_CONFIGURESUPERBILLADDFILEDLG_H__BC2E2453_F642_4A62_A697_2B307E61C785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureSuperbillAddFileDlg.h : header file
//

//DRT 6/6/2008 - PLID 30306 - Created.

/////////////////////////////////////////////////////////////////////////////
// CConfigureSuperbillAddFileDlg dialog

class CConfigureSuperbillAddFileDlg : public CNxDialog
{
// Construction
public:
	CConfigureSuperbillAddFileDlg(CWnd* pParent);   // standard constructor

	//This is the full file path that was selected, filled only when the user presses OK
	CString m_strResults_FullPath;
	//This is the path starting after the \Templates\Forms level.
	CString m_strResults_SuperbillPath;

	// (d.thompson 2009-10-20) - PLID 36007 - Allow users to specify text to prompt the user
	//	what they're selecting.  Not required.
	OPTIONAL CString m_strPromptText;

	// (d.thompson 2009-10-20) - PLID 36007 - Allow users to specify the list of templates that
	//	can be chosen.  This overrides any requerying functionality and just uses the 
	//	provided.  Paths should still be local to SharedPath + templates + forms...
	void OverrideTemplateList(CStringArray *paryLocalPaths);

// Dialog Data
	//{{AFX_DATA(CConfigureSuperbillAddFileDlg)
	enum { IDD = IDD_CONFIGURE_SUPERBILL_ADD_FILE };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxStatic	m_nxstaticDescText;
	CNxStatic	m_nxstaticPromptText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureSuperbillAddFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	// (d.thompson 2009-10-20) - PLID 36007 - used for override
	bool m_bOverrideTemplates;
	CStringArray *m_paryOverrideList;

	void FillSuperBillList(CString strPath);
	void FillSuperBillOverrideList();				// (d.thompson 2009-10-20) - PLID 36007

	// Generated message map functions
	//{{AFX_MSG(CConfigureSuperbillAddFileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGURESUPERBILLADDFILEDLG_H__BC2E2453_F642_4A62_A697_2B307E61C785__INCLUDED_)
