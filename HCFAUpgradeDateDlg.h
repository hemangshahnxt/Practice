#pragma once

// CHCFAUpgradeDateDlg dialog

// (j.jones 2013-08-02 15:23) - PLID 57805 - created

#include "AdministratorRc.h"

class CHCFAUpgradeDateDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHCFAUpgradeDateDlg)

public:
	CHCFAUpgradeDateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHCFAUpgradeDateDlg();

	//if bFromHCFAGroup is true, nID is a HCFASetupT.ID,
	//if it is false, nID is InsuranceCoT.PersonID
	virtual int DoModal(long nID, bool bFromHCFAGroup, OLE_COLOR nColor);

// Dialog Data
	enum { IDD = IDD_HCFA_UPGRADE_DATE_DLG };

protected:
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxColor		m_bkg;
	CDateTimePicker m_dtUpgradeDate;
	NxButton		m_checkOldFormAllowed;
	CNxStatic		m_nxstaticLabel;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	bool m_bFromHCFAGroup;	//true if updating an entire HCFA group, false if just one insurance company
	long m_nID;				//if m_bFromHCFAGroup is true, this is a HCFASetupT.ID, otherwise it's an InsuranceCoT.PersonID
	OLE_COLOR m_nColor;		//the color to use on the background
	CString m_strName;		//the name of the company or the HCFA group

	//cache the old values, used for individual companies only
	COleDateTime m_dtOldUpgradeDate;
	bool m_bOldAllowed;

	void TryUpdateInsuranceCompany(CString &strSqlBatch, CNxParamSqlArray &aryParams, long &nAuditTransactionID,
		const long nInsuranceCoID, const CString strInsuranceCoName,
		const COleDateTime dtOldUpgradeDate, const COleDateTime dtNewUpgradeDate,
		const bool bOldFormAllowed, const bool bNewFormAllowed);

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
};
