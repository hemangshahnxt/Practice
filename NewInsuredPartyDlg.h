#if !defined(AFX_NEWINSUREDPARTYDLG_H__3F9526C8_CA28_4017_A349_33D89C74FC3B__INCLUDED_)
#define AFX_NEWINSUREDPARTYDLG_H__3F9526C8_CA28_4017_A349_33D89C74FC3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewInsuredPartyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewInsuredPartyDlg dialog

#include "PatientsRc.h"

class CNewInsuredPartyDlg : public CNxDialog
{
// Construction
public:
	CNewInsuredPartyDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2010-08-17 14:48) - PLID 40128 - this dialog has been completely re-worked,
	// now it lets the user pick a responsibility, and and can handle default values

	NXDATALIST2Lib::_DNxDataListPtr m_InsCoCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_RespTypeList;

	//input values
	long m_nPatientID;
	long m_nDefaultInsCoID;
	long m_nDefaultRespTypeID;

	//output values
	long m_nSelectedInsuranceCoID;
	long m_nSelectedRespTypeID;

// Dialog Data
	//{{AFX_DATA(CNewInsuredPartyDlg)
	enum { IDD = IDD_NEW_INSURED_PARTY_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewInsuredPartyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2010-08-17 15:50) - PLID 40128 - this function will try to
	//select the first available resp type, though it will also take into
	//account the preferred default resp ID
	void TrySelectDefaultResp();

	// Generated message map functions
	//{{AFX_MSG(CNewInsuredPartyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWINSUREDPARTYDLG_H__3F9526C8_CA28_4017_A349_33D89C74FC3B__INCLUDED_)
