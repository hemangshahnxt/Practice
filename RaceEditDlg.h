#pragma once

// CRaceEditDlg dialog

// (j.jones 2009-10-15 09:02) - PLID 34327 - created

#include "PatientsRc.h"

class CRaceEditDlg : public CNxDialog
{

public:
	CRaceEditDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_RACE_EDIT_DLG };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_RaceList;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnAddRace();
	afx_msg void OnBtnDeleteRace();
	afx_msg void OnBtnCloseRace();
	DECLARE_EVENTSINK_MAP()
	void OnEditingFinishingRaceList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedRaceList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
