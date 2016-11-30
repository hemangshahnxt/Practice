#if !defined(AFX_REQUIREDALLOCATIONSDETAILDLG_H__9C20B9B4_B49A_4440_9FF7_ADD42DBBE460__INCLUDED_)
#define AFX_REQUIREDALLOCATIONSDETAILDLG_H__9C20B9B4_B49A_4440_9FF7_ADD42DBBE460__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RequiredAllocationsDetailDlg.h : header file
//

//TES 6/12/2008 - PLID 28078 - Created
/////////////////////////////////////////////////////////////////////////////
// CRequiredAllocationsDetailDlg dialog

class CRequiredAllocationsDetailDlg : public CNxDialog
{
// Construction
public:
	CRequiredAllocationsDetailDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRequiredAllocationsDetailDlg)
	enum { IDD = IDD_REQUIRED_ALLOCATIONS_DETAIL_DLG };
	CNxIconButton	m_nxbOK;
	CNxIconButton	m_nxbCancel;
	CNxEdit	m_nxeDescription;
	CNxIconButton	m_nxbRemoveType;
	CNxIconButton	m_nxbRemovePurpose;
	CNxIconButton	m_nxbRemoveAllPurposes;
	CNxIconButton	m_nxbAddType;
	CNxIconButton	m_nxbAddPurpose;
	//}}AFX_DATA

	//TES 6/12/2008 - PLID 28078 - In/Out parameters.
	long m_nID;
	CString m_strDescription;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRequiredAllocationsDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailTypes, m_pSelectedTypes, m_pAvailPurposes, m_pSelectedPurposes;

	void AddCurrentPurpose();
	void RemoveCurrentPurpose();
	void AddCurrentType();
	void RemoveCurrentType();

	// Generated message map functions
	//{{AFX_MSG(CRequiredAllocationsDetailDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAddRequiredType();
	afx_msg void OnAddRequiredPurpose();
	afx_msg void OnRemoveAllRequiredPurposes();
	afx_msg void OnRemoveRequiredPurpose();
	afx_msg void OnRemoveRequiredType();
	afx_msg void OnDblClickCellAvailablePurposesList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellAvailableTypes(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellSelectedPurposesList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellSelectedTypes(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REQUIREDALLOCATIONSDETAILDLG_H__9C20B9B4_B49A_4440_9FF7_ADD42DBBE460__INCLUDED_)
