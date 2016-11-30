#pragma once

// COHIPPremiumCodesDlg dialog

// (j.jones 2009-04-03 17:08) - PLID 32324 - created

class COHIPPremiumCodesDlg : public CNxDialog
{

public:
	COHIPPremiumCodesDlg(CWnd* pParent);   // standard constructor
	virtual ~COHIPPremiumCodesDlg();

	long m_nDefaultServiceID;

// Dialog Data
	enum { IDD = IDD_OHIP_PREMIUM_CODES_DLG };
	CNxIconButton m_btnClose;
	NxButton	m_checkIsPremiumCode;
	NxButton	m_radioPercent;
	NxButton	m_radioFlatFee;
	NxButton	m_radioAddOnce;
	NxButton	m_radioAddMultiple;
	CNxEdit		m_editPercent;
	CNxEdit		m_editFlatFee;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_CodeCombo;

	void Load();
	BOOL Save();

	BOOL m_bUsePercent;
	BOOL m_bPercentChanged;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckIsPremiumCode();
	afx_msg void OnRadioAddPercent();
	afx_msg void OnRadioAddFlatFee();
	afx_msg void OnBtnClose();
	afx_msg void OnEnKillfocusEditAddPercent();
	afx_msg void OnRadioAddOnce();
	afx_msg void OnRadioAddMultiple();	
	void OnSelChangingServiceCodeCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenServiceCodeCombo(LPDISPATCH lpRow);
	afx_msg void OnEnChangeEditAddPercent();
};
