#pragma once
#include "PatientsRc.h"

// CInsuranceDeductOOPDlg dialog
// (j.gruber 2010-07-30 11:08) - PLID 39727 - created

class CInsuranceDeductOOPDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInsuranceDeductOOPDlg)

public:
	CInsuranceDeductOOPDlg(long nInsuredPartyID, long nPatientID, CString strName, CWnd* pParent);   // standard constructor
	CInsuranceDeductOOPDlg(long nInsuredPartyID, long nPatientID, CString strName, long nColor, CWnd* pParent);   // standard constructor
	virtual ~CInsuranceDeductOOPDlg();

	// (j.jones 2011-12-23 14:54) - PLID 47013 - set to TRUE if coinsurance changed for any pay group
	BOOL m_bCoinsuranceChanged;

// Dialog Data
	enum { IDD = IDD_INSURANCE_DEDUCT_OOP_DLG };
	// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
	NxButton	m_radioAllPayGroups;
	NxButton	m_radioPerPayGroup;
	CNxStatic	m_nxstaticDeductibleRemainLabel;
	CNxStatic	m_nxstaticTotalDeductibleLabel;
	CNxStatic	m_nxstaticOOPRemainLabel;
	CNxStatic	m_nxstaticTotalOOPLabel;
	CNxStatic	m_nxstaticLastModifiedLabel;
	CNxColor	m_bkg1;
	CNxColor	m_bkg2;

protected:
	long m_nPatientID;
	long m_nInsuredPartyID;
	CString m_strName;
	CString m_strDeductRemain;
	CString m_strDeductTotal;
	CString m_strOOPRemain;
	CString m_strOOPTotal;
	// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
	BOOL m_bDeductiblePerPayGroup;

	long m_nColor;
	
	// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
	NXDATALIST2Lib::_DNxDataListPtr m_PayGroupList;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void SetCurrencyBox(long nID); 
	_variant_t GetCurrencyBox(long nID, CString &strBox);
	void Load();

	// (j.jones 2011-12-22 15:27) - PLID 47013 - based on the radio button settings,
	// this will show/hide either the pay group datalist or the text fields
	void ToggleDisplay();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnEnKillfocusDeductRemaining();
	afx_msg void OnEnKillfocusDeductTotal();
	afx_msg void OnEnKillfocusOopRemaining();
	afx_msg void OnEnKillfocusOopTotal();
	// (j.jones 2011-12-22 15:27) - PLID 47013 - supported configuring deductibles/OOP per pay group
	afx_msg void OnDeductibleAll();
	afx_msg void OnDeductiblePerPayGroup();
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishingDedPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};