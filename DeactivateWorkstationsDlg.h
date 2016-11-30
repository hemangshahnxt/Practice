#if !defined(AFX_DEACTIVATEWORKSTATIONSDLG_H__0E2449F3_014F_44A9_B79B_E3229B6B0750__INCLUDED_)
#define AFX_DEACTIVATEWORKSTATIONSDLG_H__0E2449F3_014F_44A9_B79B_E3229B6B0750__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeactivateWorkstationsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeactivateWorkstationsDlg dialog

class CDeactivateWorkstationsDlg : public CNxDialog
{
// Construction
public:
	CDeactivateWorkstationsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeactivateWorkstationsDlg)
	enum { IDD = IDD_DEACTIVATE_WORKSTATIONS_DLG };
	CNxIconButton	m_nxbDeactivateWorkstation;
	CNxStatic	m_nxstaticDeactivateWorkstationsLabel;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeactivateWorkstationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pUsedList, m_pInactiveList;
	enum EUsedListColumns {
		ulcText = 0,
		ulcIsIpad,
		ulcFullText,
	};
	enum EInactiveListColumns {
		ilcText = 0,
	};

	// Generated message map functions
	//{{AFX_MSG(CDeactivateWorkstationsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDeactivateWorkstation();
	afx_msg void OnSelChangedUsedWorkstations(long nNewSel);
	afx_msg void OnDblClickCellUsedWorkstations(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEACTIVATEWORKSTATIONSDLG_H__0E2449F3_014F_44A9_B79B_E3229B6B0750__INCLUDED_)
