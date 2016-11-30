#if !defined(AFX_ADVFACILITYIDEDITDLG_H__3C94B4CE_D936_4971_915F_D3B5BB4A2C36__INCLUDED_)
#define AFX_ADVFACILITYIDEDITDLG_H__3C94B4CE_D936_4971_915F_D3B5BB4A2C36__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvFacilityIDEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvFacilityIDEditDlg dialog

class CAdvFacilityIDEditDlg : public CNxDialog
{
// Construction
public:
	CAdvFacilityIDEditDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UnselectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr m_UnselectedLocList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedLocList;

// Dialog Data
	//{{AFX_DATA(CAdvFacilityIDEditDlg)
	enum { IDD = IDD_ADV_FACILITY_ID_EDIT_DLG };
	CNxIconButton	m_btnUnselectOneInsCo;
	CNxIconButton	m_btnUnselectOneLoc;
	CNxIconButton	m_btnUnselectAllLoc;
	CNxIconButton	m_btnUnselectAllInsCo;
	CNxIconButton	m_btnSelectOneLoc;
	CNxIconButton	m_btnSelectOneInsCo;
	CNxIconButton	m_btnSelectAllLoc;
	CNxIconButton	m_btnSelectAllInsCo;
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnOK;
	CNxEdit	m_nxeditNewFacilityIdQual;
	CNxEdit	m_nxeditNewFacilityId;
	NxButton	m_btnInsLocGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvFacilityIDEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvFacilityIDEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellUnselectedInsList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedInsList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselectedLocationsList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedLocationsList(long nRowIndex, short nColIndex);
	afx_msg void OnSelectOneInsco();
	afx_msg void OnSelectAllInsco();
	afx_msg void OnUnselectOneInsco();
	afx_msg void OnUnselectAllInsco();
	afx_msg void OnSelectOneLoc();
	afx_msg void OnSelectAllLoc();
	afx_msg void OnUnselectOneLoc();
	afx_msg void OnUnselectAllLoc();
	afx_msg void OnApply();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVFACILITYIDEDITDLG_H__3C94B4CE_D936_4971_915F_D3B5BB4A2C36__INCLUDED_)
