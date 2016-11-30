#if !defined(AFX_EMRSUMMARYCONFIGDLG_H__C8A5008B_51AF_401C_90AA_472DA35A0EFF__INCLUDED_)
#define AFX_EMRSUMMARYCONFIGDLG_H__C8A5008B_51AF_401C_90AA_472DA35A0EFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRSummaryConfigDlg.h : header file
//

// (j.jones 2008-06-19 09:20) - PLID 30436 - created

/////////////////////////////////////////////////////////////////////////////
// CEMRSummaryConfigDlg dialog

class CEMRSummaryConfigDlg : public CNxDialog
{
// Construction
public:
	CEMRSummaryConfigDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRSummaryConfigDlg)
	enum { IDD = IDD_EMR_SUMMARY_CONFIG_DLG };
	CNxIconButton	m_btnRemoveCategory;
	CNxIconButton	m_btnAddCategory;
	NxButton	m_checkEMRDocuments;
	NxButton	m_checkProblemList;
	NxButton	m_checkPatientEmrProblems;
	NxButton	m_checkPatientNonEmrProblems;
	NxButton	m_checkLabs;
	NxButton	m_checkPrescribedMeds;
	NxButton	m_checkCurrentMeds;
	NxButton	m_checkAllergies;
	CNxIconButton	m_btnMoveDown;
	CNxIconButton	m_btnMoveUp;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRSummaryConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_CategoryList;

	// Generated message map functions
	//{{AFX_MSG(CEMRSummaryConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnMoveCategoryUp();
	afx_msg void OnBtnMoveCategoryDown();
	afx_msg void OnBtnAddEmrCategory();
	afx_msg void OnBtnRemoveEmrCategory();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRSUMMARYCONFIGDLG_H__C8A5008B_51AF_401C_90AA_472DA35A0EFF__INCLUDED_)
