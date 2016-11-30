#pragma once

#include "NxAPI.h"


// CICCPPaymentProfileDlg dialog
// (z.manning 2015-07-22 10:02) - PLID 67241 - Created

class CICCPPaymentProfileDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CICCPPaymentProfileDlg)

public:
	CICCPPaymentProfileDlg(const long nPatientPersonID, CWnd* pParent);   // standard constructor
	virtual ~CICCPPaymentProfileDlg();

// Dialog Data
	enum { IDD = IDD_ICCP_PAYMENT_PROFILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog() override;
	virtual void UpdateView(bool bForceRefresh = true) override;

	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnClose;

	CNxColor m_nxclrBackground;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlPaymentProfiles;
	enum EPaymentProfileColumns {
		ppcProfileID = 0,
		ppcCardType,
		ppcLastFour,
		ppcExpDate,
		ppcExpDateText,
		ppcDefault,
	};

	long m_nPatientPersonID;

	// (z.manning 2015-07-23 09:18) - PLID 67237
	void UpdateButtons();
	// (z.manning 2015-07-23 09:58) - PLID 67254
	void UpdateVisibleRows();

	void LoadProfiles();
	NXDATALIST2Lib::IRowSettingsPtr GetNewRowFromProfile(NexTech_Accessor::_ICCPPaymentProfilePtr pProfile);

	void AddNewPaymentProfile(BOOL bCardPresent);
	// (z.manning 2015-08-17 15:16) - PLID 67251
	BOOL UpdatePaymentProfileFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedICCPPaymentProfileList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void SelChangedICCPPaymentProfileList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedPaymentProfileHideExpired();
	// (z.manning 2015-08-04 11:34) - PLID 67247
	afx_msg void OnBnClickedAddICCPPaymentProfile();
	afx_msg void OnBnClickedDeleteICCPPaymentProfile();
	void EditingStartingIccpPaymentProfileList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void RButtonDownIccpPaymentProfileList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
