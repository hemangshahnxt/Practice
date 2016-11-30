#if !defined(AFX_INACTIVEMEDICATIONSDLG_H__2B882FA5_B15E_4763_B4CC_FA085BAEE6D7__INCLUDED_)
#define AFX_INACTIVEMEDICATIONSDLG_H__2B882FA5_B15E_4763_B4CC_FA085BAEE6D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InactiveMedicationsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInactiveMedicationsDlg dialog

class CInactiveMedicationsDlg : public CNxDialog
{
// Construction
public:
	CInactiveMedicationsDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pInactiveList;

// Dialog Data
	//{{AFX_DATA(CInactiveMedicationsDlg)
	enum { IDD = IDD_INACTIVE_MEDICATIONS };
	CNxIconButton m_btnActivate;
	CNxIconButton m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInactiveMedicationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2007-01-31 11:40) -  PLID 24422 - This function is called to activate a medication
	// in the DrugList table. We must update the EMR data as well.
	void ExecuteActivation(long nDrugID);

protected:

	// Generated message map functions
	//{{AFX_MSG(CInactiveMedicationsDlg)
	afx_msg void OnActivateMedication();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INACTIVEMEDICATIONSDLG_H__2B882FA5_B15E_4763_B4CC_FA085BAEE6D7__INCLUDED_)
