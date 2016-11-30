#if !defined(AFX_OFFICEVISITCONFIGDLG_H__100C5266_E26A_43A6_AAD0_DE88FCDC951C__INCLUDED_)
#define AFX_OFFICEVISITCONFIGDLG_H__100C5266_E26A_43A6_AAD0_DE88FCDC951C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OfficeVisitConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COfficeVisitConfigDlg dialog

class COfficeVisitConfigDlg : public CNxDialog
{
// Construction
public:
	COfficeVisitConfigDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconbutton
// Dialog Data
	//{{AFX_DATA(COfficeVisitConfigDlg)
	enum { IDD = IDD_OFFICE_VISIT_CONFIG_DLG };
	NxButton	m_checkEnableOfficeVisits;
	CNxLabel	m_nxlCategory5;
	CNxLabel	m_nxlCategory4;
	CNxLabel	m_nxlCategory3;
	CNxLabel	m_nxlCategory2;
	CNxEdit	m_nxeditDefaultLevel;
	CNxEdit	m_nxedit2NumberItems;
	CNxEdit	m_nxedit3NumberItems;
	CNxEdit	m_nxedit4NumberItems;
	CNxEdit	m_nxedit5NumberItems;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COfficeVisitConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pDefault2, m_pDefault3, m_pDefault4, m_pDefault5, 
		m_pCategories2, m_pCategories3, m_pCategories4, m_pCategories5;

	CArray <int, int> m_arCategories2, m_arCategories3, m_arCategories4, m_arCategories5;

	CBrush m_brush;

	void EnableControls();

	// Generated message map functions
	//{{AFX_MSG(COfficeVisitConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusDefaultLevel();
	virtual void OnOK();
	afx_msg void OnClose();
	afx_msg void OnKillfocus2NumberItems();
	afx_msg void OnKillfocus3NumberItems();
	afx_msg void OnKillfocus4NumberItems();
	afx_msg void OnKillfocus5NumberItems();
	afx_msg void OnSelChanging2Default(long FAR* nNewSel);
	afx_msg void OnSelChosen2Default(long nRow);
	afx_msg void OnSelChanging3Default(long FAR* nNewSel);
	afx_msg void OnSelChosen3Default(long nRow);
	afx_msg void OnSelChanging4Default(long FAR* nNewSel);
	afx_msg void OnSelChosen4Default(long nRow);
	afx_msg void OnSelChanging5Default(long FAR* nNewSel);
	afx_msg void OnSelChosen5Default(long nRow);
	afx_msg void OnSelChanging2CategoryDropdown(long FAR* nNewSel);
	afx_msg void OnSelChanging3CategoryDropdown(long FAR* nNewSel);
	afx_msg void OnSelChanging4CategoryDropdown(long FAR* nNewSel);
	afx_msg void OnSelChanging5CategoryDropdown(long FAR* nNewSel);
	afx_msg void OnSelChosen2CategoryDropdown(long nRow);
	afx_msg void OnSelChosen3CategoryDropdown(long nRow);
	afx_msg void OnSelChosen4CategoryDropdown(long nRow);
	afx_msg void OnSelChosen5CategoryDropdown(long nRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void On2CategoryText();
	afx_msg void On3CategoryText();
	afx_msg void On4CategoryText();
	afx_msg void On5CategoryText();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCheckEnableOfficeVisitIncrements();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OFFICEVISITCONFIGDLG_H__100C5266_E26A_43A6_AAD0_DE88FCDC951C__INCLUDED_)
