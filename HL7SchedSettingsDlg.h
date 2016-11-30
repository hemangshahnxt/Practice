#pragma once


// CHL7SchedSettingsDlg dialog
// (z.manning 2011-04-21 10:51) - PLID 43361 - Created

class CHL7SchedSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7SchedSettingsDlg)

public:
	CHL7SchedSettingsDlg(const long nHL7GroupID, CWnd* pParent);   // standard constructor
	virtual ~CHL7SchedSettingsDlg();

// Dialog Data
	enum { IDD = IDD_HL7_SCHED_SETTINGS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	long m_nHL7GroupID;

	// (b.savon 2014-12-09 13:36) - PLID 64320 - Remove the Schedule Export HL7 Version dropdown from Advanced Settings for Appts. and make this into a hidden setting.

	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	BOOL ValidateAndSave();

	virtual BOOL OnInitDialog();
	virtual void OnOK();
};
