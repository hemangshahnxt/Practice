#if !defined(AFX_ADVPAYERIDDLG_H__D5300835_6226_4900_8C1D_289D35719728__INCLUDED_)
#define AFX_ADVPAYERIDDLG_H__D5300835_6226_4900_8C1D_289D35719728__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvPayerIDDlg.h : header file
//

#include "PatientsRc.h"

enum InsuranceTypeCode;

/////////////////////////////////////////////////////////////////////////////
// CAdvPayerIDDlg dialog

class CAdvPayerIDDlg : public CNxDialog
{
// Construction
public:
	CAdvPayerIDDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-09-18 10:00) - PLID 31138 - added m_btnApplyEligibility
// Dialog Data
	//{{AFX_DATA(CAdvPayerIDDlg)
	enum { IDD = IDD_ADV_PAYER_ID_DLG };
	CNxIconButton	m_btnApplyEligibility;
	CNxIconButton	m_btnUnselectAllInsCo;
	CNxIconButton	m_btnUnselectOneInsCo;
	CNxIconButton	m_btnSelectAllInsCo;
	CNxIconButton	m_btnSelectOneInsCo;
	CNxIconButton	m_btnApplyPayer;
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//CNxIconButton	m_btnApplyThin;
	CNxIconButton	m_btnApplyInsType;
	CNxIconButton	m_btnOK;
	NxButton	m_btnInsGroupbox;
	// (j.jones 2009-08-05 12:20) - PLID 35109 - added abilities to apply per location
	CNxIconButton	m_btnApplyClaimLoc;
	CNxIconButton	m_btnApplyEligibilityLoc;
	// (j.jones 2009-12-16 16:40) - PLID 36621 - added UB payer ID
	CNxIconButton	m_btnApplyUB;
	CNxIconButton	m_btnApplyUBLoc;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvPayerIDDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr	m_SelectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr	m_UnselectedInsCoList;
	NXDATALISTLib::_DNxDataListPtr	m_pEnvoyList;
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//NXDATALISTLib::_DNxDataListPtr	m_pTHINList;
	// (j.jones 2008-09-09 10:46) - PLID 18695 - converted to InsType list
	NXDATALISTLib::_DNxDataListPtr	m_pInsTypeList;
	// (j.jones 2008-09-18 10:00) - PLID 31138 - added eligibility list
	NXDATALISTLib::_DNxDataListPtr	m_pEligibilityList;
	// (j.jones 2009-08-05 12:23) - PLID 35109 - added location combo
	NXDATALIST2Lib::_DNxDataListPtr	m_pLocationCombo;
	// (j.jones 2009-12-16 16:40) - PLID 36621 - added UB payer ID
	NXDATALISTLib::_DNxDataListPtr	m_pUBPayerList;

	// (j.jones 2008-09-09 11:00) - PLID 18695 - this function will add a new row to m_pInsuranceTypeList
	void AddNewRowToInsuranceTypeList(InsuranceTypeCode eCode, BOOL bColorize = FALSE);

	// (j.jones 2008-09-18 10:01) - PLID 31138 - added OnApplyEligibility
	// Generated message map functions
	//{{AFX_MSG(CAdvPayerIDDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectOneInsco();
	afx_msg void OnSelectAllInsco();
	afx_msg void OnUnselectOneInsco();
	afx_msg void OnUnselectAllInsco();
	afx_msg void OnDblClickCellUnselectedInsList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedInsList(long nRowIndex, short nColIndex);
	afx_msg void OnApplyPayer();
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//afx_msg void OnApplyThin();
	afx_msg void OnApplyInsType();
	afx_msg void OnApplyEligibility();
	// (j.jones 2009-08-05 12:20) - PLID 35109 - added abilities to apply per location
	afx_msg void OnApplyPayerLoc();
	afx_msg void OnApplyEligibilityLoc();
	// (j.jones 2009-12-16 16:40) - PLID 36621 - added UB payer ID
	afx_msg void OnBnClickedApplyUbPayer();
	afx_msg void OnBnClickedApplyUbPayerLoc();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()		
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVPAYERIDDLG_H__D5300835_6226_4900_8C1D_289D35719728__INCLUDED_)
