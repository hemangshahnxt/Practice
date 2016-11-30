#include "client.h"
#if !defined(AFX_BILLING2DLG_H__B91C7DA0_044B_4114_BF8F_F11C74DF502D__INCLUDED_)
#define AFX_BILLING2DLG_H__B91C7DA0_044B_4114_BF8F_F11C74DF502D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Billing2Dlg.h : header file
//

enum BillingAccessType;

/////////////////////////////////////////////////////////////////////////////
// CBilling2Dlg dialog

class CBilling2Dlg : public CNxDialog
{
// Construction
public:

	// (j.gruber 2011-09-28 14:07) - PLID 45356
	enum AffiliateListColumns {
		aflcID = 0,
		aflcName,
		aflcNPI,
		aflcAddress1,
		aflcAddress2,
		aflcCity,
		aflcState,
		aflcZip,
	};

	// Bill description edit box
	CNxEdit* m_peditBillDescription;

	// Bill date edit box
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	CDateTimePicker* m_peditBillDate;

	// (r.gonet 07/02/2014) - PLID 62567 - Initialize to NULL.
	CWnd* m_pBillingModuleWnd = NULL;
	// (a.walling 2008-05-05 13:20) - PLID 29897 - Patient Name
	CString GetBillPatientName();

	// (j.jones 2011-11-01 09:39) - PLID 41558 - removed the unnecessary anesth/facility hours & minutes lists
	NXDATALIST2Lib::_DNxDataListPtr m_DischargeStatusCombo;

	// (j.jones 2007-05-02 15:24) - PLID 25883 - supported DischargeStatusT tablechecker
	// (j.jones 2008-12-11 09:50) - PLID 32407 - supported ProvidersT tablechecker
	// (j.gruber 2011-09-29 15:25) - PLID 45356 - RefPhys checker
	CTableChecker m_DischargeStatusChecker, m_ProviderChecker, m_RefPhysChecker;

	NXTIMELib::_DNxTimePtr	m_nxtAnesthStart, m_nxtAnesthEnd;
	NXTIMELib::_DNxTimePtr	m_nxtFacilityStart, m_nxtFacilityEnd;
	// (j.jones 2011-11-01 11:11) - PLID 41558 - added Assisting controls (for OHIP)
	NXTIMELib::_DNxTimePtr	m_nxtAssistingStart, m_nxtAssistingEnd;

	BOOL m_boInitialized;

	// (j.jones 2011-01-21 09:58) - PLID 42156 - changed to an enum
	BillingAccessType m_eHasAccess;
	void ChangeAccess();

	//Returns SCR_ABORT_SAVE, SCR_NOT_SAVED, or SCR_SAVED
	int SaveChanges();

	enum BillEntryType m_EntryType;	// (j.armen 2014-08-06 10:06) - PLID 63161 - Use an enum

	OLE_COLOR m_nColor;
	CBrush m_brush;

	long m_nCurTotalAnesthMinutes;
	long m_nCurTotalFacilityMinutes;
	long m_nCurTotalAssistingMinutes;

	CString GetAnesthStartTime();
	CString GetAnesthEndTime();
	CString GetFacilityStartTime();
	CString GetFacilityEndTime();
	long GetAnesthMinutes();
	long GetFacilityMinutes();

	void SetAnesthStartTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAnesthEndTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetFacilityStartTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetFacilityEndTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAnesthMinutes(long nMinutes, BOOL bUpdateCharges = FALSE);
	void SetFacilityMinutes(long nMinutes, BOOL bUpdateCharges = FALSE);

	// (j.jones 2011-11-01 08:55) - PLID 41558 - added assisting times
	CString GetAssistingStartTime();
	CString GetAssistingEndTime();
	long GetAssistingMinutes();
	void SetAssistingStartTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAssistingEndTime(CString strTime, BOOL bUpdateCharges = FALSE);
	void SetAssistingMinutes(long nMinutes, BOOL bUpdateCharges = FALSE);

	void ClearData();
	void ReadBillData();

	// (j.jones 2013-07-10 16:13) - PLID 57148 - invoice number is not
	// editable unless the user has access and a valid bill ID exists
	void TryEnableInvoiceNumber();
	// (j.jones 2013-07-10 16:39) - PLID 57148 - given an invoice ID and provider,
	// update the saved invoice fields, the current invoice fields, and the edit box
	void SetSavedInvoiceNumber(long nInvoiceID, long nProviderID);
	// (j.jones 2013-07-10 15:31) - PLID 57148 - Ensures that the invoice number
	// is valid and available for the current provider.
	// If bIsSaving is false, it's assumed this was called from OnEnKillfocusEditInvoiceNumber.
	bool ValidateInvoiceNumber(bool bIsSaving);

	// (a.walling 2007-05-24 09:14) - PLID 26114
	void UpdateRewardPointsDisplay();

	CString  m_strDeductLeftToMeet;
	CString m_strOOPLeftToMeet;

	// (j.gruber 2011-09-28 15:48) - PLID 45356
	long m_nOldAffiliatePhysID;
	CString m_strOldAffiliatePhysName;
	long m_nCurrentAffiliatePhysID;
	CString m_strCurrentAffiliatePhysName;
	CString m_strOldAffiliateAmount;
	CString m_strOldAffiliateNote;
	long m_nOldStatusType;
	NXDATALIST2Lib::_DNxDataListPtr m_pAffiliatePhysList;
	// (j.gruber 2011-09-28 15:50) - PLID 45358
	long m_nOldAffiliateStatusID;
	CString m_strOldAffiliateStatusDescription;
	//CString m_strOldAffiliateStatusDate;
	//COleDateTime m_dtOldAffiliateStatus;
	NXDATALIST2Lib::_DNxDataListPtr m_pAffiliateStatusList;



	CBilling2Dlg(CWnd* pParent);   // standard constructor

	// (j.jones 2008-05-28 10:35) - PLID 28782 - added m_btnExtraChargeInfo
	// (j.jones 2008-12-11 09:50) - PLID 32407 - added m_nxstaticSupervisingProviderLabel
// Dialog Data
	//{{AFX_DATA(CBilling2Dlg)
	enum { IDD = IDD_BILLING_2_DLG };
	CNxStatic	m_nxstaticSupervisingProviderLabel;
	CNxIconButton	m_btnExtraChargeInfo;
	CNxEdit	m_nxeditEditAnesthHours;
	CNxEdit	m_nxeditEditAnesthMinutes;
	CNxEdit	m_nxeditEditTotalAnesthMinutes;
	CNxEdit	m_nxeditEditFacilityHours;
	CNxEdit	m_nxeditEditFacilityMinutes;
	CNxEdit	m_nxeditEditTotalFacilityMinutes;
	CNxEdit	m_nxeditDeductLeftToMeet;
	CNxEdit	m_nxeditPatCoInsurance;
	CNxEdit	m_nxeditOopLeftToMeet;
	CNxEdit	m_nxeditRewardPointsTotalEdit;
	CNxStatic	m_nxstaticDeductLeftToMeetStatic;
	CNxStatic	m_nxstaticPatCoInsuranceStatic;
	CNxStatic	m_nxstaticOopLeftToMeetStatic;
	CNxStatic	m_nxstaticRewardPointsTotalStatic;
	// (j.jones 2009-08-10 16:19) - PLID 35142 - added case history labels
	CNxStatic	m_nxstaticCaseHistoryLabel;
	CNxLabel	m_nxlCaseHistoryLinkLabel;
	// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
	CNxStatic	m_nxstaticAssistingLabel1;
	CNxStatic	m_nxstaticAssistingLabel2;
	CNxStatic	m_nxstaticAssistingLabel3;
	CNxStatic	m_nxstaticAssistingLabel4;
	CNxStatic	m_nxstaticAssistingLabel5;
	CNxStatic	m_nxstaticAssistingLabel6;
	CNxStatic	m_nxstaticAssistingLabel7;
	CNxEdit m_nxeditEditAssistingHours;
	CNxEdit m_nxeditEditAssistingMinutes;
	CNxEdit m_nxeditEditTotalAssistingMinutes;
	CNxEdit m_nxeditAffiliatePhysNote; // (j.gruber 2012-10-17 13:05) - PLID 53227
	// (j.jones 2013-07-10 15:14) - PLID 57148 - added invoice number
	CNxStatic	m_nxstaticLabelInvoiceNumber;
	CNxEdit		m_nxeditInvoiceNumber;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBilling2Dlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	long GetBillID();	

	// (j.jones 2008-12-11 09:50) - PLID 32407 - added m_SupervisingProviderCombo
	NXDATALIST2Lib::_DNxDataListPtr m_SupervisingProviderCombo;
	long m_nOldSupervisingProviderID;
	CString m_strOldSupervisingProviderName;

	// (j.jones 2009-08-10 16:19) - PLID 35142 - added case history combo
	NXDATALIST2Lib::_DNxDataListPtr m_CaseHistoryCombo;

	// (j.jones 2009-08-11 12:22) - PLID 35142 - supported multiple case histories
	BOOL m_bIsSurgeryCenter;
	void OnMultipleCaseHistories();
	void ToggleCaseHistoryDisplay();
	BOOL m_bIsCaseHistoryComboHidden;
	CString GetStringOfCaseHistories();

	// (j.jones 2013-07-10 15:31) - PLID 57148 - the last invoice number & provider saved to data
	long m_nSavedInvoiceNumber;
	long m_nSavedInvoiceProviderID;
	// (j.jones 2013-07-10 15:31) - PLID 57148 - the last valid invoice number typed in,
	// it may or may not have been saved to data yet
	long m_nCurrentInvoiceNumber;
	// (j.jones 2013-07-12 09:02) - PLID 57148 - the provider ID the current invoice number is for
	long m_nCurrentInvoiceProviderID;

	// (j.jones 2013-07-10 16:39) - PLID 57148 - given an invoice ID,
	// update the edit box, m_nCurrentInvoiceNumber, and m_nCurrentInvoiceProviderID
	void SetCurrentInvoiceNumber(long nInvoiceID, long nProviderID);

	// (j.jones 2008-05-28 10:35) - PLID 28782 - added OnBtnExtraChargeInfo
	// Generated message map functions
	//{{AFX_MSG(CBilling2Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEditAnesthHours();
	afx_msg void OnKillfocusEditAnesthMinutes();
	afx_msg void OnKillfocusEditFacilityHours();
	afx_msg void OnKillfocusEditFacilityMinutes();
	afx_msg void OnKillfocusEditTotalAnesthMinutes();
	afx_msg void OnKillfocusEditTotalFacilityMinutes();
	afx_msg void OnKillFocusAnesthStartTime();
	afx_msg void OnKillFocusAnesthEndTime();
	afx_msg void OnKillFocusFacilityStartTime();
	afx_msg void OnKillFocusFacilityEndTime();
	afx_msg void OnDestroy();
	afx_msg void OnBtnEditDischargeStatus();
	afx_msg void OnRequeryFinishedDischargeStatusCombo(short nFlags);
	afx_msg void OnKillfocusDeductLeftToMeet();
	afx_msg void OnKillfocusOopLeftToMeet();
	afx_msg void OnKillfocusPatCoInsurance();
	afx_msg void OnBtnExtraChargeInfo();
	// (j.jones 2009-08-11 12:22) - PLID 35142 - supported multiple case histories
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	void OnSelChosenCaseHistoryBillCombo(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void SelChangingBillAffiliateList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenBillAffiliateList(LPDISPATCH lpRow);
	void RequeryFinishedBillAffiliateList(short nFlags);
	void SelChangingAffilStatus(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenAffilStatus(LPDISPATCH lpRow);
	afx_msg void OnEnKillfocusAffilAmt();
	void TrySetSelFinishedBillAffiliateList(long nRowEnum, long nFlags);
	// (j.jones 2011-11-01 10:02) - PLID 41558 - added Assisting controls (for OHIP)
	afx_msg void OnEnKillfocusEditAssistingHours();
	afx_msg void OnEnKillfocusEditAssistingMinutes();
	afx_msg void OnEnKillfocusEditTotalAssistingMinutes();
	afx_msg void OnKillFocusAssistingStartTime();
	afx_msg void OnKillFocusAssistingEndTime();
	// (j.jones 2013-07-10 15:31) - PLID 57148 - added invoice number
	afx_msg void OnEnKillfocusEditInvoiceNumber();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BILLING2DLG_H__B91C7DA0_044B_4114_BF8F_F11C74DF502D__INCLUDED_)
