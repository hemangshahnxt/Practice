#pragma once

// CAssistingCodesSetupDlg dialog

// (j.jones 2010-11-22 10:07) - PLID 41564 - created

#include "AdministratorRc.h"

class CAssistingCodesSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAssistingCodesSetupDlg)

public:
	CAssistingCodesSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CAssistingCodesSetupDlg();

// Dialog Data
	enum { IDD = IDD_ASSISTING_CODES_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	NxButton		m_radioTimes;
	NxButton		m_radioMinutes;
	CNxEdit			m_editUnitFee;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_UnitList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//this function will validate the fee, warn if it is invalid,
	//and replace the value on screen with what will eventually be
	//saved if not corrected, returns TRUE if the fee is valid,
	//returns FALSE if a warning was given
	BOOL GetValidatedUnitFee(OUT CString &strFormattedUnitFee);

	BOOL m_bUnitTableChanged;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnAddNewAssistingUnitOption();
	afx_msg void OnBtnDeleteAssistingUnitOption();
	afx_msg void OnEnKillfocusEditAssistingUnitFee();
	afx_msg void OnEditingFinishingUnitList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedUnitList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownUnitList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
