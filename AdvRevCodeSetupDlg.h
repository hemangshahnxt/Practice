#if !defined(AFX_ADVREVCODESETUPDLG_H__D5AF2FEC_8E48_4A5E_BE25_ED4D2E8A867E__INCLUDED_)
#define AFX_ADVREVCODESETUPDLG_H__D5AF2FEC_8E48_4A5E_BE25_ED4D2E8A867E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvRevCodeSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvRevCodeSetupDlg dialog

class CAdvRevCodeSetupDlg : public CNxDialog
{
// Construction
public:
	CAdvRevCodeSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_UnselectedServiceList, m_SelectedServiceList;
	NXDATALISTLib::_DNxDataListPtr m_UnselectedInsCoList, m_SelectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr m_RevCodeCombo;

	BOOL m_bIsInv;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CAdvRevCodeSetupDlg)
	enum { IDD = IDD_ADV_REVCODE_SETUP_DLG };
	NxButton	m_btnInv;
	NxButton	m_btnCpt;
	CNxIconButton	m_btnUnselectOneInsCo;
	CNxIconButton	m_btnUnselectAllInsCos;
	CNxIconButton	m_btnSelectAllInsCos;
	CNxIconButton	m_btnSelectOneInsCo;
	CNxIconButton	m_btnUnselectAllItems;
	CNxIconButton	m_btnUnselectOneItem;
	CNxIconButton	m_btnSelectAllItems;
	CNxIconButton	m_btnSelectOneItem;
	CNxStatic	m_nxstaticUnselectedItemsLabel;
	CNxStatic	m_nxstaticSelectedItemsLabel;
	CNxIconButton	m_btnApplyRevCodes;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvRevCodeSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvRevCodeSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectOneServiceItem();
	afx_msg void OnSelectAllServiceItems();
	afx_msg void OnUnselectOneServiceItem();
	afx_msg void OnUnselectAllServiceItems();
	afx_msg void OnApplyRevCodes();
	afx_msg void OnRadioCpt();
	afx_msg void OnRadioInv();
	afx_msg void OnDblClickCellUnselectedServiceItemList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedServiceItemList(long nRowIndex, short nColIndex);
	afx_msg void OnSelectOneInsCo();
	afx_msg void OnSelectAllInscos();
	afx_msg void OnUnselectOneInsCo();
	afx_msg void OnUnselectAllInscos();
	afx_msg void OnDblClickCellUnselectedInscoList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedInscoList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVREVCODESETUPDLG_H__D5AF2FEC_8E48_4A5E_BE25_ED4D2E8A867E__INCLUDED_)
