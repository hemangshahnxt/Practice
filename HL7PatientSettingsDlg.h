#pragma once


// CHL7PatientSettingsDlg dialog
// (z.manning 2010-10-04 17:26) - PLID 40795 - Created

class CHL7PatientSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7PatientSettingsDlg)

public:
	CHL7PatientSettingsDlg(long nHL7GroupID, CWnd* pParent);   // standard constructor
	virtual ~CHL7PatientSettingsDlg();
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Dialog Data
	enum { IDD = IDD_HL7_PATIENT_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	long m_nHL7GroupID;

	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	NxButton m_nxbPID18UseOverride, m_nxbUseIn1_3_2, m_nxbWorkersCompIn1_31;
	CNxEdit m_nxePID18Override;
	// (d.thompson 2012-06-05) - PLID 50551
	NxButton m_nxbRequirePatientIDMap;

	//TES 9/16/2011 - PLID 45537 - Disable the override field if the box to use it is unchecked
	void ReflectPid18UseOverride();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPid18UseOverride();
	// (d.thompson 2012-08-21) - PLID 52048
	afx_msg void OnBnClickedConfigEthnicity();
	// (d.thompson 2012-08-22) - PLID 52047
	afx_msg void OnBnClickedConfigRace();
	// (d.thompson 2012-08-23) - PLID 52049
	afx_msg void OnBnClickedConfigLanguage();
};
