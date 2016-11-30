#pragma once

#include "AdministratorRc.h"
// CClaimFormLocationInsuranceSetup dialog

typedef long InsuranceID;
typedef long ClaimFormID;

struct ClaimTypeConfig
{
	bool bIsSetupRecord;
	long nClaimFormType;

	ClaimTypeConfig(bool IsSetupRecord,long ClaimFormID) {

		bIsSetupRecord = IsSetupRecord;
		nClaimFormType = ClaimFormID;
	}
};

class CClaimFormLocationInsuranceSetup : public CNxDialog
{
	DECLARE_DYNAMIC(CClaimFormLocationInsuranceSetup)

public:
	CClaimFormLocationInsuranceSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClaimFormLocationInsuranceSetup();

	std::map<InsuranceID, ClaimTypeConfig > m_mInsuranceClaimFormMap;
	CNxColor	m_nxc;
	OLE_COLOR m_nColor;
	virtual BOOL OnInitDialog();
	void InitializeClaimFormList();
// Dialog Data
	enum { IDD = IDD_CLAIMFORM_LOCATION_INSURANCE_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	long m_nCurLocationID;
	CNxIconButton m_buttonApplyAll;
	CNxIconButton m_buttonClose;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;
	NXDATALIST2Lib::_DNxDataListPtr m_pClaimFormList;
public:
	afx_msg void OnBnClickedClaimApplyAll();
	DECLARE_EVENTSINK_MAP()
	CString GetInsuranceClaimListFROMSQL();
	void SaveEdited();
	void AddUpdateEditedItems(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void SelChosenClaimLocation(LPDISPATCH lpRow);
	afx_msg void OnBnClickedOk();
	void EditingFinishedClaimInsuranceCompany(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnClose();
};
