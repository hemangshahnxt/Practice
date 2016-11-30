#if !defined(AFX_REASONDLG_H__8DD54D47_0EA5_486D_A596_476F0BF8868C__INCLUDED_)
#define AFX_REASONDLG_H__8DD54D47_0EA5_486D_A596_476F0BF8868C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CancelApptDlg.h : header file
//

#include "PatientDialog.h"

namespace Nx
{
	namespace Scheduler
	{
		struct Reason;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CReasonDlg dialog

class CReasonDlg : public CNxDialog
{
// Construction
public:
	CReasonDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_ReasonCombo;
	bool m_bNoShow;//True, gets no-show reason, false, gets cancel reason.
	BOOL OnInitDialog();
	CString GetReason(long &ReasonID);
	Nx::Scheduler::Reason GetReason();
	int IsCustomReason();
	int m_nIsChecked;
	long m_nApptID;
	// (d.singleton 2011-12-27 08:54) - PLID 47110 - added mem variable to store the preference to require a reason so i dont have to keep checking it from other dialogs
	BOOL m_bReqReason;
	bool m_bReschedule = false;
	bool m_bDisableReschedulingQueue = false; // (a.walling 2015-02-04 09:12) - PLID 64412 - hide checkbox to add to rescheduling queue

// Dialog Data
	//{{AFX_DATA(CReasonDlg)
	enum { IDD = IDD_REASON_DLG };
	NxButton	m_customReason;
	CString	m_strText;
	CString m_strReason;
	CNxEdit	m_nxeditCustomReasonEdit;
	CNxEdit	m_nxeditReasonPatient;
	CNxEdit	m_nxeditReasonStartTime;
	CNxEdit	m_nxeditReasonEndTime;
	CNxEdit	m_nxeditReasonDate;
	CNxEdit	m_nxeditReasonText;
	CNxEdit	m_nxeditReasonType;
	CNxEdit	m_nxeditReasonPurpose;
	CNxStatic	m_nxstaticReasonCaption;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnReasonCombo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReasonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EditCustomList(NXDATALISTLib::_DNxDataListPtr &list, long listID);

	// Generated message map functions
	//{{AFX_MSG(CReasonDlg)
	afx_msg void OnCustomReason();
	afx_msg void OnEditReasonCombo();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CANCELAPPTDLG_H__8DD54D47_0EA5_486D_A596_476F0BF8868C__INCLUDED_)
