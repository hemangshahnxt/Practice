//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_INSURANCEGROUPSDLG1_H__3EBFE561_16A7_11D4_BA41_0050DA0F2D8A__INCLUDED_)
#define AFX_INSURANCEGROUPSDLG1_H__3EBFE561_16A7_11D4_BA41_0050DA0F2D8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsuranceGroupsDlg1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInsuranceGroupsDlg dialog

class CInsuranceGroupsDlg : public CNxDialog
{
// Construction
public:
	CString m_strInsCo;
	CInsuranceGroupsDlg(CWnd* pParent);   // standard constructor
	int m_iInsuranceCoID;
	int m_iProviderID;

// Dialog Data
	//{{AFX_DATA(CInsuranceGroupsDlg)
	enum { IDD = IDD_INSURANCE_GROUPS_DLG };
	CString	m_GRP;
	CNxEdit	m_nxeditEditGrp;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAdvGrpEdit;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_pDocList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsuranceGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsuranceGroupsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnKillfocusEditGrp();
	afx_msg void OnSelChangedInsDocs(long nNewSel);
	afx_msg void OnAdvGrpEdit();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void LoadGRPNumber();
	void SaveGRPNumber();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSURANCEGROUPSDLG1_H__3EBFE561_16A7_11D4_BA41_0050DA0F2D8A__INCLUDED_)
