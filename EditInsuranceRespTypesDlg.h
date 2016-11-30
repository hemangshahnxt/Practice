#if !defined(AFX_EDITINSURANCERESPTYPESDLG_H__5F962D82_8ACF_4CC7_8402_96BFB013A073__INCLUDED_)
#define AFX_EDITINSURANCERESPTYPESDLG_H__5F962D82_8ACF_4CC7_8402_96BFB013A073__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditInsuranceRespTypesDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CEditInsuranceRespTypesDlg dialog

#include "PatientsRc.h"

class CEditInsuranceRespTypesDlg : public CNxDialog
{
// Construction
public:
	CEditInsuranceRespTypesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditInsuranceRespTypesDlg)
	enum { IDD = IDD_EDIT_INSURANCE_RESP_TYPE_DLG };
	CNxIconButton	m_btnMoveDown;
	CNxIconButton	m_btnMoveUp;
	CNxIconButton	m_btnNewResp;
	CNxIconButton	m_btnDeleteResp;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditInsuranceRespTypesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2011-03-30 11:08) - PLID 43044 - changed to a datalist 2
	NXDATALIST2Lib::_DNxDataListPtr m_List;
	void EnsurePriorities();

	// (j.jones 2011-03-31 09:07) - PLID 43044 - this function will
	// keep RespTypeT.CategoryPlacement up to date
	void EnsureCategoryPlacement();

	// Generated message map functions
	//{{AFX_MSG(CEditInsuranceRespTypesDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnNewBtn();
	afx_msg void OnDeleteBtn();
	afx_msg void OnMoveUpBtn();
	afx_msg void OnMoveDownBtn();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedListRespTypes(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	// (j.jones 2010-06-16 13:22) - PLID 39190 - ensured you can't change HasBillingColumn for Primary/Secondary
	afx_msg void OnEditingStartingListRespTypes(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	// (j.jones 2011-03-31 09:06) - PLID 43044 - added OnEditingFinished
	afx_msg void OnEditingFinishedListRespTypes(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (j.jones 2011-03-31 09:15) - PLID 43044 - added coloring to the placement column
	afx_msg void OnRequeryFinishedListRespTypes(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINSURANCERESPTYPESDLG_H__5F962D82_8ACF_4CC7_8402_96BFB013A073__INCLUDED_)
