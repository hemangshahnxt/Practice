#if !defined(AFX_INVALLOCATIONSDLG_H__75B242E7_32DB_4D5C_A7F2_0E0ABF48F722__INCLUDED_)
#define AFX_INVALLOCATIONSDLG_H__75B242E7_32DB_4D5C_A7F2_0E0ABF48F722__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvAllocationsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvAllocationsDlg dialog

// (j.jones 2007-11-06 11:12) - PLID 28003 - created

class CInvAllocationsDlg : public CNxDialog
{
// Construction
public:
	CInvAllocationsDlg(CWnd* pParent);   // standard constructor

	virtual void Refresh();

	// (c.haag 2008-01-07 16:18) - PLID 28084 - Calculate the filters for the Allocation List report
	void GetCurrentFilters(long& nStatus);

// Dialog Data
	//{{AFX_DATA(CInvAllocationsDlg)
	enum { IDD = IDD_INV_ALLOCATIONS_DLG };
	CNxIconButton	m_nxbConfigureRequiredAllocations;
	CNxIconButton	m_btnCompleteMultipleAllocations;
	NxButton	m_radioCompleteNotBilled;
	NxButton	m_radioCompleteBilled;
	NxButton	m_radioActive;
	CNxIconButton	m_btnCreateNew;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvAllocationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	NXDATALIST2Lib::_DNxDataListPtr m_AllocationsList;	//list of all allocations in the system

	// Generated message map functions
	//{{AFX_MSG(CInvAllocationsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnCreateNewAllocation();
	afx_msg void OnLeftClickAllocationsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownAllocationsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRadioActiveAllocations();
	afx_msg void OnRadioCompletedAndBilled();
	afx_msg void OnRadioCompletedNotBilled();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBtnCompleteMultipleAllocations();
	afx_msg void OnConfigureRequiredAllocations();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVALLOCATIONSDLG_H__75B242E7_32DB_4D5C_A7F2_0E0ABF48F722__INCLUDED_)
