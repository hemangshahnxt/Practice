#pragma once

// (b.savon 2014-12-01 10:24) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter

#include "PatientsRc.h"

// CPatientEncounterDlg dialog

class CPatientEncounterDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientEncounterDlg)
private:
	CNxColor m_nxcBackground;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnAdmit;
	CNxIconButton m_btnCancelAdmit;
	CNxIconButton m_btnDischarge;
	CNxIconButton m_btnCancelDischarge;
	CNxStatic m_nxsEncounterStatus;

public:
	CPatientEncounterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPatientEncounterDlg();
	virtual BOOL OnInitDialog();

	// Dialog Data
	enum { IDD = IDD_PATIENT_ENCOUNTER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnAdmitPatient();
	afx_msg void OnBnClickedBtnCancelAdmitPatient();
	afx_msg void OnBnClickedBtnDischargePatient();
	afx_msg void OnBnClickedBtnCancelDischargePatient();
};