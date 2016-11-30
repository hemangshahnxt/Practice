#pragma once

#include "PracticeRc.h"
#include "NexERxLicense.h"
#include "NexERxSetupDlg.h"

// CNexERxConfigureLicenses dialog
// (b.savon 2013-01-31 14:18) - PLID 54964 - Created

class CNexERxConfigureLicenses : public CNxDialog
{
	DECLARE_DYNAMIC(CNexERxConfigureLicenses)
private:
	CNxIconButton m_btnClose;
	CNxIconButton m_btnLicensedPrescriberRequest;
	CNxIconButton m_btnLicensedPrescriberDeactivate;
	CNxIconButton m_btnMidlevelPrescriberRequest;
	CNxIconButton m_btnMidlevelPrescriberDeactivate;
	CNxIconButton m_btnNurseStaffRequest;
	CNxIconButton m_btnNurseStaffDeactivate;

	CNxColor m_nxcBack;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlLicensedPrescriberAvailable;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlLicensedPrescriberLicensed;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlLicensedPrescriberInactive;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlMidlevelPrescriberAvailable;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlMidlevelPrescriberLicensed;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlMidlevelPrescriberInactive;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlNurseStaffPrescriberAvailable;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlNurseStaffPrescriberLicensed;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlNurseStaffPrescriberInactive;

	void LoadDatalists();
	void LoadLicensedPrescribersNexERx();
	void LoadMidlevelPrescribersNexERx();
	void LoadNurseStaffNexERx();

	void EnableLicensedPrescriberButtons();
	void EnableMidlevelPrescriberButtons();
	void EnableNurseStaffButtons();

	void UpdateInfoLabel();

	BOOL m_bLicensedPrescriberAllReady;
	BOOL m_bLicensedPrescriberUsedReady;
	BOOL m_bLicensedPrescriberInactiveReady;
	BOOL m_bMidlevelPrescriberAllReady;
	BOOL m_bMidlevelPrescriberUsedReady;
	BOOL m_bMidlevelPrescriberInactiveReady;
	BOOL m_bNurseStaffAllReady;
	BOOL m_bNurseStaffUsedReady;
	BOOL m_bNurseStaffInactiveReady;

	CNexERxLicense m_lNexERxLicense;

	CNexERxSetupDlg* m_pSetup;
	BOOL DetermineSupervisorsAndMidlevelsLicensedForUser(long nPersonID);
	BOOL DetermineSupervisorsForUser(long nPersonID);
	BOOL CheckMidlevelForError(const CSimpleArray<long> &aryMidlevel, long nPersonID);
	BOOL CheckSupervisorForError(const CSimpleArray<long> &arySupervising, long nPersonID);


public:
	CNexERxConfigureLicenses(CNexERxSetupDlg* pCaller, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexERxConfigureLicenses();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_NEXERX_MANAGE_LICENSES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedNxdlAvailableLp(short nFlags);
	void RequeryFinishedNxdlLicensedLp(short nFlags);
	void RequeryFinishedNxdlInactiveLp(short nFlags);
	void RequeryFinishedNxdlAvailableMl(short nFlags);
	void RequeryFinishedNxdlLicensedMl(short nFlags);
	void RequeryFinishedNxdlInactiveMl(short nFlags);
	void RequeryFinishedNxdlAvailableNs(short nFlags);
	void RequeryFinishedNxdlLicensedNs(short nFlags);
	void RequeryFinishedNxdlInactiveNs(short nFlags);
	afx_msg void OnBnClickedLpRequest();
	afx_msg void OnBnClickedLpDeactivate();
	afx_msg void OnBnClickedMlRequest();
	afx_msg void OnBnClickedMlDeactivate();
	afx_msg void OnBnClickedNsRequest();
	afx_msg void OnBnClickedNsDeactivate();
};
