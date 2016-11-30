#if !defined(AFX_SCHEDULINGPRODUCTIVITYDLG_H__A0F6C02A_7D42_4D5D_8445_36412877EE4D__INCLUDED_)
#define AFX_SCHEDULINGPRODUCTIVITYDLG_H__A0F6C02A_7D42_4D5D_8445_36412877EE4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SchedulingProductivityDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSchedulingProductivityDlg dialog

class CSchedulingProductivityDlg : public CNxDialog
{
// Construction
public:
	CSchedulingProductivityDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSchedulingProductivityDlg)
	enum { IDD = IDD_SCHEDULING_PRODUCTIVITY_DLG };
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxStatic	m_nxstaticInquiries;
	CNxStatic	m_nxstaticNewProspects;
	CNxStatic	m_nxstaticConsults;
	CNxStatic	m_nxstaticProceduresPerformed;
	CNxStatic	m_nxstaticPrepaymentsEntered;
	NxButton	m_btnProductivityGroupbox;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSchedulingProductivityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Refresh();
	NXDATALISTLib::_DNxDataListPtr m_pDateOptions;

	// Generated message map functions
	//{{AFX_MSG(CSchedulingProductivityDlg)
	afx_msg void OnChangeProductivityFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeProductivityTo(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenDateFilterOptions(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULINGPRODUCTIVITYDLG_H__A0F6C02A_7D42_4D5D_8445_36412877EE4D__INCLUDED_)
