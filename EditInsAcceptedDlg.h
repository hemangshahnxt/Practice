#if !defined(AFX_EDITINSACCEPTEDDLG_H__4DA2A04B_E0B8_48DD_9085_812BBA2E5276__INCLUDED_)
#define AFX_EDITINSACCEPTEDDLG_H__4DA2A04B_E0B8_48DD_9085_812BBA2E5276__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditInsAcceptedDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditInsAcceptedDlg dialog

#include "PatientsRc.h"

class CEditInsAcceptedDlg : public CNxDialog
{
// Construction
public:

	CString m_strInsCo;
	NXDATALISTLib::_DNxDataListPtr m_ProviderList;
	long m_iInsuranceCoID;

	CEditInsAcceptedDlg(CWnd* pParent);   // standard constructor

	void LoadAcceptedInfo();

// Dialog Data
	//{{AFX_DATA(CEditInsAcceptedDlg)
	enum { IDD = IDD_EDIT_INS_ACCEPTED_DLG };
	CNxIconButton	m_btnOK;
	// (j.jones 2010-07-23 09:10) - PLID 25217 - moved preference features into Practice
	NxButton		m_radioAlwaysAccept;
	NxButton		m_radioNeverAccept;	
	// (j.jones 2010-07-23 17:18) - PLID 34105 - added warning label & button for assignment of benefits
	CNxStatic	m_nxstaticAssignmentOfBenefitsWarningLabel;
	CNxIconButton	m_btnAssignmentOfBenefitsWarning;
	// (j.jones 2010-07-30 14:34) - PLID 39917 - moved the ability to mark all accepted / not accepted
	// into a separate, permissioned dialog
	CNxIconButton	m_btnUpdateAllAccepted;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditInsAcceptedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2010-07-23 17:16) - PLID 34105 - called to show/hide the assignment of benefits warning label
	void UpdateAssignmentOfBenefitsWarningLabel();

	// Generated message map functions
	//{{AFX_MSG(CEditInsAcceptedDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedProviderAcceptedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedProviderAcceptedList(short nFlags);
	afx_msg void OnRButtonDownProviderAcceptedList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2010-07-23 09:10) - PLID 25217 - moved preference features into Practice	
	afx_msg void OnRadioAcceptAllIns();
	afx_msg void OnRadioAcceptNoIns();
	// (j.jones 2010-07-23 17:17) - PLID 34105 - only shown if assignment of benefits can be blank,
	// clicking this button should explain why the warning is displayed
	afx_msg void OnBtnWarnAssignmentOfBenefitsAcceptAssignment();
	// (j.jones 2010-07-30 14:34) - PLID 39917 - moved the ability to mark all accepted / not accepted
	// into a separate, permissioned dialog
	afx_msg void OnBtnUpdateAllAccepted();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINSACCEPTEDDLG_H__4DA2A04B_E0B8_48DD_9085_812BBA2E5276__INCLUDED_)
