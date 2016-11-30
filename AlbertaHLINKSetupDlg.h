#pragma once

// CAlbertaHLINKSetupDlg dialog

// (j.jones 2010-10-12 14:52) - PLID 40901 - created

#include "FinancialRc.h"

class CAlbertaHLINKSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAlbertaHLINKSetupDlg)

public:
	CAlbertaHLINKSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CAlbertaHLINKSetupDlg();

	// (j.jones 2010-11-08 14:05) - PLID 39620 - we now cache the Health Number field
	// and tell the caller if it changed
	long m_nHealthNumberCustomField;
	BOOL m_bHealthNumberCustomFieldChanged;

// Dialog Data
	enum { IDD = IDD_ALBERTA_HLINK_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxEdit			m_nxeditSubmitterPrefix;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_PatientULICombo;	
	NXDATALIST2Lib::_DNxDataListPtr m_RegistrationNumberCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderBAIDCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_PayToCodeCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_SubmitterPrefixCombo;	// (j.dinatale 2012-12-28 10:00) - PLID 54370 - allow users to choose the submitter prefix

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenAlbertaSubmitprefixCombo(LPDISPATCH lpRow);
	void RequeryFinishedAlbertaSubmitprefixCombo(short nFlags);
	afx_msg void OnBnClickedHlinkSetupHelp();
};
