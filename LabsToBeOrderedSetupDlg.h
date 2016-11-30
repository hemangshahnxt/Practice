#pragma once

// (j.jones 2010-06-25 16:01) - PLID 39185 - created

// CLabsToBeOrderedSetupDlg dialog

#include "AdministratorRc.h"

class CLabsToBeOrderedSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabsToBeOrderedSetupDlg)

public:
	CLabsToBeOrderedSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabsToBeOrderedSetupDlg();

	long m_nDefaultLocationID;

// Dialog Data
	enum { IDD = IDD_LABS_TO_BE_ORDERED_SETUP_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnEditToBeOrderedList;

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_Combo;
	NXDATALIST2Lib::_DNxDataListPtr m_List;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void LoadLocationList(long nLocationID);

	//returns TRUE if any linked checkbox doesn't match what is saved
	BOOL HasSelectionsChanged();

	BOOL Save(long nLocationID);

	//cache the last selected location, since you can't do this cleanly in OnSelChanging
	long m_nLastSelLocationID;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBtnEditToBeOrderedList();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenLabToBeOrderedLocationCombo(LPDISPATCH lpRow);
	void OnSelChangingLabToBeOrderedLocationCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
