#if !defined(AFX_CHANGETYPEDLG_H__9E48BF78_F7D5_48CA_B10D_8DE8B8C8F667__INCLUDED_)
#define AFX_CHANGETYPEDLG_H__9E48BF78_F7D5_48CA_B10D_8DE8B8C8F667__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangeTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangeTypeDlg dialog

class CChangeTypeDlg : public CNxDialog
{
// Construction
public:
	int nStatus;
	CChangeTypeDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangeTypeDlg)
	enum { IDD = IDD_CHANGE_TYPE };
	NxButton	m_btnProvider;
	NxButton	m_btnUser;
	NxButton	m_btnSupplier;
	NxButton	m_btnOther;
	NxButton	m_btnRefPhys;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangeTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangeTypeDlg)
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnOtherBtn();
	afx_msg void OnProviderBtn();
	afx_msg void OnRefphysBtn();
	afx_msg void OnSupplierBtn();
	afx_msg void OnUserBtn();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGETYPEDLG_H__9E48BF78_F7D5_48CA_B10D_8DE8B8C8F667__INCLUDED_)
