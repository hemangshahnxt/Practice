#pragma once

#include "ContactsRc.h"
#include "NxAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>

// CNexERxRegisterPrescriberDlg dialog
// (b.savon 2013-06-06 15:51) - PLID 56840 - Created

struct NexERxPrescriber{
	CString strID;
	CString strFirst;
	CString strLast;
	CString strMiddle;
	CString strNPI;
	CString strStateLicense;
	// (b.savon 2013-08-02 14:21) - PLID 57747 - DEA
	CString strDEA;
	CString strWork;
	CString strExt;
	CString strFax;
	CString strEmail;

	NexERxPrescriber()
	{
		strID = "";
		strFirst = "";
		strLast = "";
		strMiddle = "";
		strNPI = "";
		strStateLicense = "";
		// (b.savon 2013-08-02 14:21) - PLID 57747 - DEA
		strDEA = "";
		strWork = "";
		strExt = "";
		strFax = "";
		strEmail = "";	
	}
};

class CNexERxRegisterPrescriberDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexERxRegisterPrescriberDlg)
private:
	CNxColor		m_nxcTop;
	CNxColor		m_nxcBottom;

	CNxIconButton	m_btnRegister;
	CNxIconButton	m_btnCancel;

	CNxStatic		m_nxstaticInfo;

	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditWorkPhoneBox;
	CNxEdit	m_nxeditExtPhoneBox;
	CNxEdit	m_nxeditFaxBox;
	CNxEdit	m_nxeditEmailBox;
	CNxEdit	m_nxeditNpiBox;
	CNxEdit	m_nxeditLicenseBox;
	// (b.savon 2013-08-02 14:21) - PLID 57747 - DEA
	CNxEdit m_nxeditDEA;

	NxButton m_chkSelectAllLocations;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlLocationServiceLevel;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlServiceLevelDD;

	const NexERxPrescriber &m_prescriber;

	long m_nCountSelected;

	BOOL m_bRegistered;

	void PrepareControls();
	void LoadDatalist();
	void LoadPrescriberInfo();
	void SetTextBoxColors();
	void EnableRegistration();

	void HandleRegistrationResults(NexTech_Accessor::_ERxPrescriberRegistrationPtr results);

	// (b.savon 2013-06-09 13:40) - PLID 56867
	NXDATALIST2Lib::IFormatSettingsPtr GetReadOnlyServiceLevelFormat();

public:
	CNexERxRegisterPrescriberDlg(const NexERxPrescriber &prescriber, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexERxRegisterPrescriberDlg();
	virtual BOOL OnInitDialog();

	inline BOOL IsRegistered(){ return m_bRegistered; };

// Dialog Data
	enum { IDD = IDD_NEXERX_REGISTER_PRESCRIBER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedOk();
	void EditingFinishedNxdlRegisterPrescriberLocations(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedCheckAllRxLocations();
	void SelChosenNxdlApplyServicelevel(LPDISPATCH lpRow);
	void SelChangingNxdlApplyServicelevel(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
