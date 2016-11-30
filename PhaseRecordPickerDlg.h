#if !defined(AFX_PHASERECORDPICKERDLG_H__005382B0_AF00_4173_8F63_EF3A5468D01F__INCLUDED_)
#define AFX_PHASERECORDPICKERDLG_H__005382B0_AF00_4173_8F63_EF3A5468D01F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhaseRecordPickerDlg.h : header file
#include "practicerc.h"
//

#include "PhaseTracking.h"
/////////////////////////////////////////////////////////////////////////////
// CPhaseRecordPickerDlg dialog

class CPhaseRecordPickerDlg : public CNxDialog
{
// Construction
public:
	BOOL IsAppointmentAction();
	CPhaseRecordPickerDlg(CWnd* pParent);   // standard constructor
	int Open(const CArray<PhaseTracking::TrackingEvent,PhaseTracking::TrackingEvent&>& arEvents, const CString& strProcedureName, long nAction, const CString& strPersonName);
	PhaseTracking::TrackingEvent GetSelectedEvent();

// Dialog Data
	//{{AFX_DATA(CPhaseRecordPickerDlg)
	enum { IDD = IDD_PHASE_RECORD_PICKER };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticPickerText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhaseRecordPickerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr	m_List;
	CArray<PhaseTracking::TrackingEvent,PhaseTracking::TrackingEvent&> m_arEvents;
	CString m_strProcedureName;
	CString m_strPersonName;
	PhaseTracking::TrackingEvent m_SelectedEvent;
	long m_nAction;
	void OnOK();

	// Generated message map functions
	//{{AFX_MSG(CPhaseRecordPickerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedListPhaserecordpicker(short nFlags);
	afx_msg void OnDblClickCellListPhaserecordpicker(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHASERECORDPICKERDLG_H__005382B0_AF00_4173_8F63_EF3A5468D01F__INCLUDED_)
