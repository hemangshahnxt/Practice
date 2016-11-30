#if !defined(AFX_ATTENDANCEOPTIONSDLG_H__2EA60E67_501E_48F8_8FB4_FBAAEECA6C1A__INCLUDED_)
#define AFX_ATTENDANCEOPTIONSDLG_H__2EA60E67_501E_48F8_8FB4_FBAAEECA6C1A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AttendanceOptionsDlg.h : header file
//

#include "commondialog.h"

/////////////////////////////////////////////////////////////////////////////
// CAttendanceOptionsDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29139 - Created

class CAttendanceOptionsDlg : public CNxDialog
{
// Construction
public:
	CAttendanceOptionsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAttendanceOptionsDlg)
	enum { IDD = IDD_ATTENDANCE_OPTIONS };
	NxButton	m_btnShowGridlines;
	NxButton	m_btnEmailOnDeny;
	NxButton	m_btnEmailOnApprove;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOk;
	CNxEdit	m_nxeditDefaultVacationAllowance;
	CNxEdit	m_nxeditDefaultSickAllowance;
	CCommonDialog60	m_ctrlColorPicker;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttendanceOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (z.manning, 01/11/2008) - PLID 28600 - Added an option for default to-do category
	NXDATALIST2Lib::_DNxDataListPtr m_pdlTodoCategory;
	// (z.manning, 02/18/2008) - PLID 28909 - Can now associate attendance appointments with schedule types.
	NXDATALIST2Lib::_DNxDataListPtr m_pdlVacationType;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlSickType;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlOtherType;
	// (z.manning 2008-11-13 13:42) - PLID 31831 - Paid/Unpaid
	NXDATALIST2Lib::_DNxDataListPtr m_pdlPaidType;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlUnpaidType;

	// (z.manning 2012-03-27 11:52) - PLID 49227 - Added row color options
	OLE_COLOR m_nAccruedRowColor;
	OLE_COLOR m_nAvailableRowColor;
	OLE_COLOR m_nTotalUsedRowColor;
	OLE_COLOR m_nBalanceRowColor;

	void Save();

	// (z.manning 2012-03-27 12:07) - PLID 49227
	void SelectRowColorOption(UINT nCtrlIDButton, IN OUT OLE_COLOR &nColor);

	// Generated message map functions
	//{{AFX_MSG(CAttendanceOptionsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnRequeryFinishedAttendanceTodoCategory(short nFlags);
	afx_msg void OnRequeryFinishedAttendanceVacationType(short nFlags);
	afx_msg void OnRequeryFinishedAttendanceSickType(short nFlags);
	afx_msg void OnRequeryFinishedAttendanceOtherType(short nFlags);
	afx_msg void OnRequeryFinishedAttendancePaidType(short nFlags);
	afx_msg void OnRequeryFinishedAttendanceUnpaidType(short nFlags);
	void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedAccruedRowColor();
	afx_msg void OnBnClickedAvailableRowColor();
	afx_msg void OnBnClickedTotalUsedRowColor();
	afx_msg void OnBnClickedBalanceRowColor();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTENDANCEOPTIONSDLG_H__2EA60E67_501E_48F8_8FB4_FBAAEECA6C1A__INCLUDED_)
