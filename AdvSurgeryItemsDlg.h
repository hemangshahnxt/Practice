#if !defined(AFX_ADVSURGERYITEMSDLG_H__51BEF972_E743_49DC_85CB_51041625810A__INCLUDED_)
#define AFX_ADVSURGERYITEMSDLG_H__51BEF972_E743_49DC_85CB_51041625810A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvSurgeryItemsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvSurgeryItemsDlg dialog

class CAdvSurgeryItemsDlg : public CNxDialog
{
// Construction
public:
	CAdvSurgeryItemsDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UnselectedSurgeries, m_SelectedSurgeries,
				m_UnselectedCPTs, m_SelectedCPTs,
				m_UnselectedInv, m_SelectedInv;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButton for close
// Dialog Data
	//{{AFX_DATA(CAdvSurgeryItemsDlg)
	enum { IDD = IDD_ADV_SURGERY_ITEMS_DLG };
	CNxIconButton	m_btnUnselectOneSurgery;
	CNxIconButton	m_btnUnselectOneInv;
	CNxIconButton	m_btnUnselectAllSurgeries;
	CNxIconButton	m_btnUnselectAllInv;
	CNxIconButton	m_btnSelectOneSurgery;
	CNxIconButton	m_btnSelectOneInv;
	CNxIconButton	m_btnSelectAllSurgeries;
	CNxIconButton	m_btnSelectAllInv;
	CNxIconButton	m_btnUnselectAllCPT;
	CNxIconButton	m_btnUnselectOneCPT;
	CNxIconButton	m_btnSelectOneCPT;
	CNxIconButton	m_btnSelectAllCPT;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnApply;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvSurgeryItemsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvSurgeryItemsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	afx_msg void OnSelectOneSurgery();
	afx_msg void OnSelectAllSurgeries();
	afx_msg void OnUnselectOneSurgery();
	afx_msg void OnUnselectAllSurgeries();
	afx_msg void OnSelectOneCpt();
	afx_msg void OnSelectAllCpt();
	afx_msg void OnUnselectOneCpt();
	afx_msg void OnUnselectAllCpt();
	afx_msg void OnSelectOneInv();
	afx_msg void OnSelectAllInv();
	afx_msg void OnUnselectOneInv();
	afx_msg void OnUnselectAllInv();
	afx_msg void OnDblClickCellUnselectedSurgeryList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedSurgeryList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselectedCptList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedCptList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselectedInvList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedInvList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVSURGERYITEMSDLG_H__51BEF972_E743_49DC_85CB_51041625810A__INCLUDED_)
