// EmrMoveEmnDlg.h: interface for the CEmrMoveEmnDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMRMOVEEMNDLG_H__F31C0FD8_C81E_4573_A83E_5005BAB315FF__INCLUDED_)
#define AFX_EMRMOVEEMNDLG_H__F31C0FD8_C81E_4573_A83E_5005BAB315FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrMoveEmnDlg.h : header file
//

#include "EmrRc.h"

enum EmrListColumns
{
	elcID = 0,
	elcDescription,
	elcInputDate,
};

// (j.jones 2009-09-24 15:15) - PLID 31672 - patient columns
// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
/*
enum PatientListColumns
{
	plcID = 0,
	plcUserDefinedID,
	plcName,
	plcBirthDate,
	plcGender,
	plcLast,
	plcFirst,
	plcMiddle,
};
*/

/////////////////////////////////////////////////////////////////////////////
// CEmrMoveEmnDlg dialog

class CEmrMoveEmnDlg : public CNxDialog
{
// Construction
public:
	// (j.jones 2009-09-24 15:04) - PLID 31672 - added ability to move between patients,
	// as an alternative to moving between EMRs
	// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
	CEmrMoveEmnDlg(long nEmnID, /*BOOL bMoveToPatient,*/ CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CEmrMoveEmnDlg)
	enum { IDD = IDD_EMR_MOVE_EMN };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticMoveEmnInstructions;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrMoveEmnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CEmrMoveEmnDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2009-09-24 14:51) - PLID 31672 - Ths dialog has dual use,
	// moving between EMRs, and moving between patients. If the latter,
	// this boolean will be TRUE.
	// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
	//BOOL m_bMoveToPatient;

	long m_nEmnID;
	long m_nPatientID;
	CString m_strPatientName;
	COleDateTime m_dtEMNDate;
	CString m_strEmnDescription;
	CString m_strOldEmrDescription;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlEmrList;
	// (j.jones 2009-09-24 15:10) - PLID 31672 - added a patient list
	// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
	//NXDATALIST2Lib::_DNxDataListPtr m_pdlPatientList;

	void RequeryEmrList();
	// (j.jones 2009-09-24 15:08) - PLID 31672 - displays all active patients but this one
	// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
	//void RequeryPatientList();

	// Generated message map functions
	//{{AFX_MSG(CEmrMoveEmnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnOK();
	afx_msg void OnDblClickCellEmrList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRequeryFinishedEmrList(short nFlags);
	// (j.jones 2009-09-24 14:57) - PLID 31672 - added ability to move EMNs between patients
	// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this feature
	//afx_msg void OnDblClickCellMoveToPatientList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRMOVEEMNDLG_H__89537872_330E_4FC7_89A0_528A03D9C92A__INCLUDED_)
