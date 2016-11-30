#pragma once
#include "PatientsRc.h"

// CPrescriptionDosingDlg dialog
// (s.dhole 2012-10-23 ) - PLID 51718   New Dialog Dosing information
class CPrescriptionDosingDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPrescriptionDosingDlg)

public:
	CPrescriptionDosingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPrescriptionDosingDlg();

// Dialog Data
	enum { IDD = IDD_PRESCRIPTION_DOSING };
	CNxIconButton	m_btnCancel;
	CNxLabel m_nxlabelDoseUnit,m_nxlabelDoseCalc;
	CNxEdit m_nxtxtWeight,m_nxtxtDoseValue;
	NxButton m_nxbIsKg, m_nxbIsLb;
	CString  m_strMedicationName;
	unsigned long m_bkgColor;
	CNxColor	m_bkg;
	NXDATALIST2Lib::_DNxDataListPtr m_DosingList;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
protected:
	
	virtual BOOL OnInitDialog();
	void LoadData();
	void ShowDataFiled( BOOL bShowData);
	void CalculateDosage();
	
public:

	long m_nMedID,m_nAgeInDays;
	DECLARE_MESSAGE_MAP()	

	afx_msg void OnBnClickedPrescriptiondosecancel();
	DECLARE_EVENTSINK_MAP()
	void SelChosenPrescriptionDoasingList(long nRow);
	afx_msg void OnEnChangeTxtDosePatientWeight();
	afx_msg void OnEnChangeTxtDoseValue();
	afx_msg void OnBnClickedRadioIsLb();
	afx_msg void OnBnClickedRadioIsKg();
	void SelChangedPrescriptionDosingList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
