#if !defined(AFX_APPTSREQUIRINGALLOCATIONSDLG_H__CFE063B6_F2BA_42D4_885F_2BFE95958F11__INCLUDED_)
#define AFX_APPTSREQUIRINGALLOCATIONSDLG_H__CFE063B6_F2BA_42D4_885F_2BFE95958F11__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ApptsRequiringAllocationsDlg.h : header file
//

//TES 6/12/2008 - PLID 28078 - Created
/////////////////////////////////////////////////////////////////////////////
// CApptsRequiringAllocationsDlg dialog

class CApptsRequiringAllocationsDlg : public CNxDialog
{
// Construction
public:
	CApptsRequiringAllocationsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CApptsRequiringAllocationsDlg)
	enum { IDD = IDD_APPTS_REQUIRING_ALLOCATIONS_DLG };
	CNxEdit	m_nxeRequiredDays;
	NxButton	m_nxbRemindMe;
	CNxIconButton	m_nxbEdit;
	CNxIconButton	m_nxbDelete;
	CNxIconButton	m_nxbClose;
	CNxIconButton	m_nxbAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApptsRequiringAllocationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	//TES 8/4/2008 - PLID 28078 - Split out to be called from the DblClick handler as well as the button handler.
	void EditRequirement(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// Generated message map functions
	//{{AFX_MSG(CApptsRequiringAllocationsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedRequirementsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnAddRequirement();
	afx_msg void OnCloseApptsRequiringAllocations();
	afx_msg void OnDeleteRequirement();
	afx_msg void OnEditRequirement();
	afx_msg void OnRemindMe();
	afx_msg void OnKillfocusRequirementDays();
	afx_msg void OnDblClickCellRequirementsList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPTSREQUIRINGALLOCATIONSDLG_H__CFE063B6_F2BA_42D4_885F_2BFE95958F11__INCLUDED_)
