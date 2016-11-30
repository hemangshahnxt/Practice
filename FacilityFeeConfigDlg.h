#if !defined(AFX_FACILITYFEECONFIGDLG_H__FE05063B_8B94_4C83_A229_C9E008E92241__INCLUDED_)
#define AFX_FACILITYFEECONFIGDLG_H__FE05063B_8B94_4C83_A229_C9E008E92241__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FacilityFeeConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFacilityFeeConfigDlg dialog

class CFacilityFeeConfigDlg : public CNxDialog
{
// Construction
public:
	CFacilityFeeConfigDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_POSCombo;
	NXDATALISTLib::_DNxDataListPtr m_IncrementBaseCombo, m_IncrementAdditionalCombo;
	NXDATALISTLib::_DNxDataListPtr m_FacilityFeeTable;

	// (j.jones 2007-10-15 10:36) - PLID 27757 - added ServiceID & ServiceCode
	long m_nServiceID;
	CString m_strServiceCode;

	// (z.manning, 04/30/2008) - PLID 29860 - Added NxIconButtons for OK and Cancel
// Dialog Data
	//{{AFX_DATA(CFacilityFeeConfigDlg)
	enum { IDD = IDD_FACILITY_FEE_CONFIG_DLG };	
	// (j.jones 2007-10-16 10:48) - PLID 27761 - added ability to copy the setup between places of service
	CNxIconButton	m_btnCopyToPOS;
	// (j.jones 2007-10-16 08:48) - PLID 27760 - added ability to copy the setup between service codes
	CNxIconButton	m_btnCopyToCode;
	CNxIconButton	m_btnDeleteFeeOption;
	CNxIconButton	m_btnAddFeeOption;
	NxButton	m_checkQuoteAsOutsideFee;
	NxButton	m_radioUseMinutes;
	NxButton	m_radioUseBeginEndTimes;
	NxButton	m_radioUseLesserTime;
	NxButton	m_radioUseGreaterTime;
	NxButton	m_radioFlatFee;
	NxButton	m_radioIncrementalFee;
	NxButton	m_radioSpecificFeeTable;
	NxButton	m_checkEnableFacilityFeeConfig;
	CNxEdit	m_nxeditEditFlatFacilityFee;
	CNxEdit	m_nxeditEditIncrementalBaseFee;
	CNxEdit	m_nxeditEditIncrementalAdditionalFee;
	CNxEdit	m_nxeditFacilityFeePaidTo;
	CNxStatic	m_nxstaticFacilityServiceLabel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFacilityFeeConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	long m_nLocationID;

	void Load();
	BOOL Save();

	void OnFeeRadioChanged();

	// (j.jones 2007-10-15 13:36) - PLID 27757 - added FacilityFeeSetupID
	long m_nFacilityFeeSetupID;

	// Generated message map functions
	//{{AFX_MSG(CFacilityFeeConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckEnableFacilityFeeConfig();
	afx_msg void OnSelChosenPosCombo(long nRow);
	afx_msg void OnRadioFlatFee();
	afx_msg void OnRadioUseIncrementalFee();
	afx_msg void OnRadioUseSpecificFeeSchedule();
	afx_msg void OnKillfocusEditFlatFacilityFee();
	afx_msg void OnKillfocusEditIncrementalBaseFee();
	afx_msg void OnKillfocusEditIncrementalAdditionalFee();
	afx_msg void OnBtnAddNewFeeOption();
	afx_msg void OnBtnDeleteFeeOption();
	afx_msg void OnEditingFinishingFacilityFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedFacilityFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	// (j.jones 2007-10-16 08:48) - PLID 27760 - added ability to copy the setup between service codes
	afx_msg void OnBtnCopyFacfeeToCode();
	// (j.jones 2007-10-16 10:48) - PLID 27761 - added ability to copy the setup between places of service
	afx_msg void OnBtnCopyFacfeeToPos();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FACILITYFEECONFIGDLG_H__FE05063B_8B94_4C83_A229_C9E008E92241__INCLUDED_)
