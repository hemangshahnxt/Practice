#if !defined(AFX_CLIASETUP_H__CEB683C7_E122_4F6E_B3FB_4AB8BF22DE48__INCLUDED_)
#define AFX_CLIASETUP_H__CEB683C7_E122_4F6E_B3FB_4AB8BF22DE48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CLIASetup.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CCLIASetup dialog

#include "AdministratorRc.h"

class CCLIASetup : public CNxDialog
{
// Construction
public:
	CCLIASetup(CWnd* pParent);   // standard constructor

	// (j.jones 2011-04-05 11:00) - PLID 42372 - added nInsuranceCoID, required
	// (j.jones 2011-10-20 10:31) - PLID 45784 - renamed to nDefaultInsuranceCoID, as this 
	// is all it is used for now
	virtual int DoModal(int nDefaultInsuranceCoID, OLE_COLOR nBkgColor);

	void LoadCLIANumber();
	void SaveCLIANumber();

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
	// (j.jones 2009-02-23 10:30) - PLID 33167 - added billing/rendering provider radio buttons
	// (j.jones 2011-04-05 10:42) - PLID 42732 - removed insurance company controls
// Dialog Data
	//{{AFX_DATA(CCLIASetup)
	enum { IDD = IDD_CLIA_SETUP };
	CNxColor	m_bkg1;
	CNxColor	m_bkg2;
	CNxColor	m_bkg3;
	NxButton	m_btnRefPhys;
	NxButton	m_btnBox32;
	NxButton	m_btnBox23;
	NxButton	m_btnUseModifier;
	// (j.jones 2011-10-19 09:38) - PLID 46023 - removed service code controls
	CNxEdit	m_nxeditEditCliaNumber;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_radioBillingProv;
	NxButton	m_radioRenderingProv;
	// (j.jones 2011-10-19 10:14) - PLID 46023 - moved the service code setup to its own dialog
	CNxIconButton	m_btnConfigServices;
	// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
	CNxIconButton	m_btnUnselectOneInsCo;
	CNxIconButton	m_btnUnselectAllInsCo;
	CNxIconButton	m_btnSelectOneInsCo;
	CNxIconButton	m_btnSelectAllInsCo;
	CNxStatic	m_nxstaticInsuranceCoNameLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCLIASetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2011-10-19 09:38) - PLID 46023 - removed service code controls
	NXDATALISTLib::_DNxDataListPtr m_ComboProviders, m_ComboModifiers,
			m_ComboLocations;	// (j.jones 2009-02-06 15:45) - PLID 32951 - added location support

	// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedInsCoList, m_SelectedInsCoList;

	BOOL SetModifierSelectionFromData(CString strModifierID);

	// (z.manning, 05/02/2007) - PLID 16623 - Used to keep track of the modifier so we can use it in the
	// TrySetSelFinished handler for the modifier dropdown.
	CString m_strOriginalModifier;

	// (j.jones 2011-04-05 11:00) - PLID 42372 - added m_nInsuranceCoID, required
	// (j.jones 2011-10-20 10:31) - PLID 45784 - renamed to m_nDefaultInsuranceCoID, as this 
	// is all it is used for now
	long m_nDefaultInsuranceCoID;
	OLE_COLOR m_nBkgColor;

	// (j.jones 2009-02-06 15:46) - PLID 32951 - added OnSelChosenComboLocations
	// Generated message map functions
	//{{AFX_MSG(CCLIASetup)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenComboProviders(long nRow);
	afx_msg void OnSelChosenComboLocations(long nRow);
	// (j.jones 2011-10-19 09:38) - PLID 46023 - removed service code controls
	afx_msg void OnKillfocusEditCliaNumber();
	afx_msg void OnCheckUseModifier();
	afx_msg void OnTrySetSelFinishedComboModifiers(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2011-10-19 10:14) - PLID 46023 - moved the service code setup to its own dialog
	afx_msg void OnBtnCliaServices();
	// (j.jones 2011-10-19 17:29) - PLID 45784 - added insurance lists
	void OnDblClickCellUnselectedCliaInscoList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedCliaInscoList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnSelectOneInsco();
	afx_msg void OnBtnSelectAllInscos();
	afx_msg void OnBtnUnselectOneInsco();
	afx_msg void OnBtnUnselectAllInscos();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIASETUP_H__CEB683C7_E122_4F6E_B3FB_4AB8BF22DE48__INCLUDED_)
