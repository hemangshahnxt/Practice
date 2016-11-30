#if !defined(AFX_ELIGIBILITYREQUESTDLG_H__02EA9E65_BDDD_4AAC_A134_CD3C7DD52568__INCLUDED_)
#define AFX_ELIGIBILITYREQUESTDLG_H__02EA9E65_BDDD_4AAC_A134_CD3C7DD52568__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EligibilityRequestDlg.h : header file
//

#include "Color.h"

// (j.jones 2007-05-21 15:07) - PLID 8993 - created

/////////////////////////////////////////////////////////////////////////////
// CEligibilityRequestDlg dialog

class CEligibilityRequestDlg : public CNxDialog
{
// Construction
public:
	CEligibilityRequestDlg(CWnd* pParent);   // standard constructor

	//allow the caller to choose a background color
	CColor m_clrBackground;

	long m_nID;

	// (j.jones 2010-07-19 10:53) - PLID 31082 - allow passing in a default insured party ID
	long m_nDefaultInsuredPartyID;

	// (j.jones 2010-07-08 08:58) - PLID 39515 - allow passing in which format to export in
	long m_nFormatID;

	// (j.jones 2008-05-07 15:38) - PLID 29854 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CEligibilityRequestDlg)
	enum { IDD = IDD_ELIGIBILITY_REQUEST_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton	m_radioBenefitCategory;
	NxButton	m_radioServiceCode;	
	CNxColor	m_bkg;
	//}}AFX_DATA

	// (s.dhole 07/23/2012) PLID 48693 
	BOOL m_bModeless;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEligibilityRequestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_PatientCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_InsuredPartyCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ServiceCombo;

	NXDATALIST2Lib::_DNxDataListPtr m_ModifierCombo1;
	NXDATALIST2Lib::_DNxDataListPtr m_ModifierCombo2;
	NXDATALIST2Lib::_DNxDataListPtr m_ModifierCombo3;
	NXDATALIST2Lib::_DNxDataListPtr m_ModifierCombo4;

	// (j.jones 2014-02-27 15:14) - PLID 60767 - added diag search and code list
	NXDATALIST2Lib::_DNxDataListPtr m_DiagSearchList;
	NXDATALIST2Lib::_DNxDataListPtr m_DiagCodeList;

	NXDATALIST2Lib::_DNxDataListPtr m_POSCombo; // (c.haag 2010-10-15 9:15) - PLID 40352 - Place of Service type

	// (j.jones 2009-09-16 12:44) - PLID 26481 - moved to globalfinancialutils
	//void BuildCategoryCombo();
	//void AddToCategoryCombo(CString strCode, CString strCategory);

	void OnRequestTypeChanged();

	CBrush m_brush;

	//used to determine if we've requeried the service code list yet
	BOOL m_bNeedServiceRequery;

	long m_nPendingPatientID;
	long m_nPendingInsuredPartyID;
	long m_nPendingProviderID;
	long m_nPendingLocationID;
	long m_nPendingServiceID;
	CString m_strPendingModifier1;
	CString m_strPendingModifier2;
	CString m_strPendingModifier3;
	CString m_strPendingModifier4;
	long m_nPendingPOSID; // (c.haag 2010-10-15 9:15) - PLID 40352

	// (j.jones 2007-06-21 12:08) - PLID 26387 - in order to prompt to re-batch a request,
	// we need to know if it is unbatched
	BOOL m_bIsBatched;
	
	// Generated message map functions
	//{{AFX_MSG(CEligibilityRequestDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenEligibilityPatientCombo(LPDISPATCH lpRow);
	afx_msg void OnRadioReqBenefitCategory();
	afx_msg void OnRadioReqServiceCode();
	afx_msg void OnTrySetSelFinishedEligibilityPatientCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligibilityInsuredPartyCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligibilityProviderCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligibilityLocationCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligibilityServiceCodeCombo(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligModifierCombo1(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligModifierCombo2(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligModifierCombo3(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedEligModifierCombo4(long nRowEnum, long nFlags);
	// (j.jones 2014-02-27 15:14) - PLID 60767 - added diag search and code list
	afx_msg void OnSelChosenEligDiagSearchList(LPDISPATCH lpRow);
	// (j.jones 2014-02-28 11:25) - PLID 60867 - added right click ability to remove default codes
	afx_msg void OnRButtonDownEligDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ELIGIBILITYREQUESTDLG_H__02EA9E65_BDDD_4AAC_A134_CD3C7DD52568__INCLUDED_)
