#if !defined(AFX_ANESTHESIASETUPDLG_H__B47B66DB_5573_43B1_B602_F2C57B499FF2__INCLUDED_)
#define AFX_ANESTHESIASETUPDLG_H__B47B66DB_5573_43B1_B602_F2C57B499FF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnesthesiaSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaSetupDlg dialog

class CAnesthesiaSetupDlg : public CNxDialog
{
// Construction
public:
	CAnesthesiaSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_POSCombo;
	NXDATALISTLib::_DNxDataListPtr m_IncrementBaseCombo, m_IncrementAdditionalCombo;
	NXDATALISTLib::_DNxDataListPtr m_AnesthFeeTable;

	// (j.jones 2007-10-15 10:36) - PLID 27757 - added ServiceID & ServiceCode
	long m_nServiceID;
	CString m_strServiceCode;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CAnesthesiaSetupDlg)
	enum { IDD = IDD_ANESTHESIA_SETUP_DLG };	
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
	NxButton	m_radioUnitBased;
	CNxEdit	m_nxeditEditFlatAnesthFee;
	CNxEdit	m_nxeditEditAnesthIncrementalBaseFee;
	CNxEdit	m_nxeditEditAnesthIncrementalAdditionalFee;
	CNxEdit	m_nxeditEditUnitCost;
	CNxEdit	m_nxeditEditTimeUnits;
	CNxEdit	m_nxeditAnesFeePaidTo;
	CNxStatic	m_nxstaticAnesthServiceLabel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnEditInsuranceCoSetup;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnesthesiaSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;

	long m_nLocationID;

	void Load();
	BOOL Save();

	void OnFeeRadioChanged();

	// (j.jones 2007-10-15 13:36) - PLID 27757 - added AnesthesiaSetupID
	long m_nAnesthesiaSetupID;

	// Generated message map functions
	//{{AFX_MSG(CAnesthesiaSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnEditInsuranceCoSetup();
	afx_msg void OnSelChosenPosCombo(long nRow);
	afx_msg void OnRadioFlatFee();
	afx_msg void OnRadioUseIncrementalFee();
	afx_msg void OnRadioUseSpecificFeeSchedule();
	afx_msg void OnRadioUseUnitBased();
	afx_msg void OnKillfocusEditFlatAnesthFee();
	afx_msg void OnKillfocusEditIncrementalBaseFee();
	afx_msg void OnKillfocusEditIncrementalAdditionalFee();
	afx_msg void OnBtnAddNewFeeOption();
	afx_msg void OnBtnDeleteFeeOption();
	afx_msg void OnEditingFinishingAnesthFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedAnesthFeeTable(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnKillfocusEditUnitCost();
	afx_msg void OnKillfocusEditTimeUnits();
	// (j.jones 2007-10-16 08:48) - PLID 27760 - added ability to copy the setup between service codes
	afx_msg void OnBtnCopyAnesthToCode();
	// (j.jones 2007-10-16 10:48) - PLID 27761 - added ability to copy the setup between places of service
	afx_msg void OnBtnCopyAnesthToPos();
	afx_msg void OnRequeryFinishedPosAnesthCombo(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANESTHESIASETUPDLG_H__B47B66DB_5573_43B1_B602_F2C57B499FF2__INCLUDED_)
