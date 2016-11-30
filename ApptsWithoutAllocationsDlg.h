#if !defined(AFX_APPTSWITHOUTALLOCATIONSDLG_H__4459C418_73CA_46CD_AF49_356167C8B1A4__INCLUDED_)
#define AFX_APPTSWITHOUTALLOCATIONSDLG_H__4459C418_73CA_46CD_AF49_356167C8B1A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ApptsWithoutAllocationsDlg.h : header file
//

//TES 6/16/2008 - PLID 30394 - Created.
/////////////////////////////////////////////////////////////////////////////
// CApptsWithoutAllocationsDlg dialog

class CApptsWithoutAllocationsDlg : public CNxDialog
{
// Construction
public:
	CApptsWithoutAllocationsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CApptsWithoutAllocationsDlg)
	enum { IDD = IDD_APPOINTMENTS_WITHOUT_ALLOCATIONS_DLG };
	CNxIconButton	m_nxbPreview;
	CNxIconButton	m_nxbCreateOrder;
	CNxIconButton	m_nxbConfigureRequirements;
	CNxIconButton	m_nxbCreateAllocation;
	CNxIconButton	m_nxbClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApptsWithoutAllocationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pApptsList;

	//TES 6/16/2008 - PLID 30394 - Updates the Create Allocation and Create Order buttons, based on permissions and the currently
	// selected appointment.
	void EnableButtons();

	CString GetWhereClause();

	// Generated message map functions
	//{{AFX_MSG(CApptsWithoutAllocationsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedAppointmentsList(short nFlags);
	afx_msg void OnCloseApptsWoAllocs();
	afx_msg void OnConfigureRequirements();
	afx_msg void OnCreateAllocationWo();
	afx_msg void OnCreateOrderWo();
	afx_msg void OnSelChangedAppointmentsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRButtonUpAppointmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnPreviewAppts();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPTSWITHOUTALLOCATIONSDLG_H__4459C418_73CA_46CD_AF49_356167C8B1A4__INCLUDED_)
