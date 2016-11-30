#if !defined(AFX_UPDATEPASTPENDINGAPPTSDLG_H__C3467C46_AAE7_4333_B71A_13E94E9CF155__INCLUDED_)
#define AFX_UPDATEPASTPENDINGAPPTSDLG_H__C3467C46_AAE7_4333_B71A_13E94E9CF155__INCLUDED_

#include "practicerc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdatePastPendingApptsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdatePastPendingApptsDlg dialog

class CUpdatePastPendingApptsDlg : public CNxDialog
{
// Construction
public:
	CUpdatePastPendingApptsDlg(CWnd* pParent);   // standard constructor

// (c.haag 2010-01-12 15:44) - PLID 31157 - We now standardize auditing of mass appointment updates
	static void AuditMassUpdate(const CString& strFrom, const CString& strTo,
		const COleDateTime& dtFrom, const COleDateTime& dtTo);

	void UpdateCaption();

// Dialog Data
	//{{AFX_DATA(CUpdatePastPendingApptsDlg)
	enum { IDD = IDD_UPDATE_PAST_PENDING_APPTS };
	COleDateTime	m_dtFrom;
	COleDateTime	m_dtTo;
	CNxStatic	m_nxstaticAptCount;
	CNxIconButton	m_btnPendingToNoShow;
	CNxIconButton	m_btnPendingToOut;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdatePastPendingApptsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdatePastPendingApptsDlg)
	afx_msg void OnBtnPendingToNoshow();
	afx_msg void OnBtnPendingToOut();
	afx_msg void OnDatetimechangeFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeToDate(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEPASTPENDINGAPPTSDLG_H__C3467C46_AAE7_4333_B71A_13E94E9CF155__INCLUDED_)
