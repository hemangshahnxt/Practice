#pragma once
#include "PracticeRc.h"

// CConfigureReferralSourcePhoneNumbersDlg dialog


// (j.gruber 2010-01-12 11:11) - PLID 36647 - created for

class CConfigureReferralSourcePhoneNumbersDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConfigureReferralSourcePhoneNumbersDlg)

public:
	CConfigureReferralSourcePhoneNumbersDlg(CWnd* pParent);   // standard constructor
	virtual ~CConfigureReferralSourcePhoneNumbersDlg();

// Dialog Data
	enum { IDD = IDD_CONFIGURE_REFERRAL_SOURCE_PHONE_NUMBERS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	long m_nReferralID;

	NXDATALIST2Lib::_DNxDataListPtr m_pReferralSourceList;

	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;
	BOOL m_bFormattingField;
	void SaveToMemory(long nID);

	CNxEdit m_edtPhoneNumber;
	// (j.gruber 2010-03-04 10:05) - PLID 37622 - added new number field
	CNxEdit m_edtHomePhoneNumber;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

	// (j.gruber 2010-03-04 10:05) - PLID 37622 - added new number field
	void EnKillFocus(long nID);
	void EnSetFocus(long nID);
	void EnChange(long nID);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();	
	afx_msg void OnEnSetfocusCrsPhoneNumber();
	afx_msg void OnEnKillfocusCrsPhoneNumber();
	DECLARE_EVENTSINK_MAP()	
	void RequeryFinishedCrsReferralSourceList(short nFlags);
	void SelChosenCrsReferralSourceList(LPDISPATCH lpRow);
	afx_msg void OnEnChangeCrsPhoneNumber();
	afx_msg void OnEnChangeCrsHomePhoneNumber();
	afx_msg void OnEnKillfocusCrsHomePhoneNumber();
	afx_msg void OnEnSetfocusCrsHomePhoneNumber();
	void SelChangingCrsReferralSourceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
