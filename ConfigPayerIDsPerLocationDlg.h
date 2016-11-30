#pragma once

// CConfigPayerIDsPerLocationDlg dialog

// (j.jones 2009-08-04 13:15) - PLID 34467 - created

#include "PatientsRc.h"

class CConfigPayerIDsPerLocationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigPayerIDsPerLocationDlg)

public:
	CConfigPayerIDsPerLocationDlg(CWnd* pParent);   // standard constructor

	long m_nInsuranceCoID;
	CString m_strInsuranceCoName;
	OLE_COLOR m_nColor;

// Dialog Data
	enum { IDD = IDD_CONFIG_PAYER_ID_PER_LOCATION_DLG };
	CNxIconButton m_btnClose;
	CNxIconButton m_btnEditPayerList;
	CNxStatic m_nxstaticInsCoLabel;
	CNxColor m_bkg;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ClaimPayerIDCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_EligibilityPayerIDCombo;
	// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
	NXDATALIST2Lib::_DNxDataListPtr m_UBPayerIDCombo;

	long m_nCurLocationID;

	//for auditing
	long m_nOldClaimPayerID;
	CString m_strOldClaimPayerCode;
	long m_nOldEligibilityPayerID;
	CString m_strOldEligibilityPayerCode;
	// (j.jones 2009-12-16 16:03) - PLID 36237 - added UB Payer IDs
	long m_nOldUBPayerID;
	CString m_strOldUBPayerCode;

	void Load();
	BOOL Save();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnClose();
	afx_msg void OnEditPayerList();
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenLocationCombo(LPDISPATCH lpRow);
};
