#if !defined(AFX_CPTDIAGNOSISLINKINGDLG_H__DCF70F46_1561_4F55_9E35_9292F2560CEA__INCLUDED_)
#define AFX_CPTDIAGNOSISLINKINGDLG_H__DCF70F46_1561_4F55_9E35_9292F2560CEA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CPTDiagnosisLinkingDlg.h : header file
// (r.gonet 02/20/2014) - PLID 60778 - Renamed the file to remove the reference to ICD-9. Also renamed the class,
//     functions, and member variables to remove references to ICD-9. I have not commented on the simple
//     name changes because there are a ton of them and they are not helpful.
//
#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CCPTDiagnosisLinkingDlg dialog

class CCPTDiagnosisLinkingDlg : public CNxDialog
{
// Construction
public:
	CCPTDiagnosisLinkingDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_CPTCombo, m_DiagnosisCombo, m_ServiceList, m_DiagnosisList;
	// (j.jones 2013-04-09 16:43) - PLID 56126 - added product combo
	NXDATALIST2Lib::_DNxDataListPtr m_ProductCombo;

	void OnRadioCPTDiagnosisChange(BOOL bChangedCPTDiagnosis);

	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CCPTDiagnosisLinkingDlg)
	enum { IDD = IDD_CPT_DIAGNOSIS_LINKING_DLG };
	NxButton	m_btnLinkCPT;
	NxButton	m_btnLinkDiagnosis;
	NxButton	m_btnViewLinked;
	NxButton	m_btnViewBlocked;
	NxButton	m_btnEnableLinking;
	// (r.gonet 02/20/2014) - PLID 60778 - Radio button controlling what codesystem the diagnosis combo shows. 
	// If this is checked, the combo will show ICD-9 codes.
	NxButton	m_radioShowICD9Codes;
	// (r.gonet 02/20/2014) - PLID 60778 - Radio button controlling what codesystem the diagnosis combo shows. 
	// If this is checked, the combo will show ICd-10 codes.
	NxButton	m_radioShowICD10Codes;
	CNxStatic	m_nxstaticLabelCptDiagnosisDropdown;
	CNxStatic	m_nxstaticLinkedCodeListLabel;
	CNxIconButton	m_btnAddNewCptDiagnosisLink;
	CNxIconButton	m_btnDeleteCptDiagnosisLink;
	CNxIconButton	m_btnClose;
	// (j.jones 2012-12-12 15:52) - PLID 47773 - added ability to toggle the prompt in the bill
	NxButton	m_checkAllowDynamicLinking;
	// (j.jones 2013-04-09 17:24) - PLID 56126 - added radio buttons for services/products
	NxButton	m_radioSelectCPTCodes;
	NxButton	m_radioSelectProducts;
	CNxStatic	m_staticProductInfoLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCPTDiagnosisLinkingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (j.jones 2013-04-10 09:30) - PLID 56126 - track if they have the inventory license
	bool m_bHasInventory;
	// (r.gonet 02/20/2014) - PLID 60778 - Reloads the content of the diagnosis combo box.
	// Fills with ICD-10 codes or ICD-9 codes depending on which radio button is checked above it.
	void ReloadDiagnosisCombo();

	// Generated message map functions
	//{{AFX_MSG(CCPTDiagnosisLinkingDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddNewCptDiagnosisLink();
	afx_msg void OnBtnDeleteCptDiagnosisLink();
	afx_msg void OnRButtonDownLinkedCptList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownLinkedDiagnosisList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChosenCptLinkSelectCombo(long nRow);
	afx_msg void OnSelChosenDiagnosisLinkSelectCombo(long nRow);
	afx_msg void OnLinkcodes();
	// (r.gonet 02/20/2014) - PLID 60778 - Handler for when the user clicks the ICD-9 radio
// button above the diagnosis combo. Forces the diagnosis combo to load only ICD-9 codes
	afx_msg void OnRadioShowICD9Codes();
	// (r.gonet 02/20/2014) - PLID 60778 - Handler for when the user clicks the ICD-10 radio
// button above the diagnosis combo. Forces the diagnosis combo to load only ICD-10 codes
	afx_msg void OnRadioShowICD10Codes();
	afx_msg void OnRadioCptLink();
	afx_msg void OnRadioDiagnosisLink();
	afx_msg void OnRadioViewLinkedCodes();
	afx_msg void OnRadioViewBlockedCodes();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2012-12-12 15:52) - PLID 47773 - added ability to toggle the prompt in the bill
	afx_msg void OnCheckAllowDynamicLinking();
	// (j.jones 2013-04-09 16:43) - PLID 56126 - added product combo
	afx_msg void OnSelChosenProductLinkSelectCombo(LPDISPATCH lpRow);
	// (j.jones 2013-04-09 17:24) - PLID 56126 - added radio buttons for services/products
	afx_msg void OnRadioProductLinkSelect();
	afx_msg void OnRadioCptLinkSelect();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPTDIAGNOSISLINKINGDLG_H__DCF70F46_1561_4F55_9E35_9292F2560CEA__INCLUDED_)
