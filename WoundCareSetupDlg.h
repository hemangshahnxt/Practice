// (r.gonet 08/03/2012) - PLID 51947 - Added

#pragma once

#include "AdministratorRc.h"
#include "WoundCareCalculator.h"

// CWoundCareSetupDlg dialog

class CWoundCareSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CWoundCareSetupDlg)

private:
	enum EWoundCareConditionalActionsColumns
	{
		wccacID,
		wccacCondition,
		wccacActions,
	};

	CNxStatic m_nxstaticHeaderText;
	NXDATALIST2Lib::_DNxDataListPtr m_pConditionalActionList;
	CNxIconButton m_btnClose;
	CNxIconButton m_btnResetToDefaults;

public:
	CWoundCareSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWoundCareSetupDlg();

	// (r.gonet 08/03/2012) - PLID 51947 - Fills the conditional actions datalist
	void ReloadConditionalActionsDataList();
	// (r.gonet 08/03/2012) - PLID 51947 - Checks if the user has setup the conditional actions yet
	//  Static so it can be called elsewhere
	static bool IsConfigured();
	// (r.gonet 08/03/2012) - PLID 51947 - Attempts to set the conditional actions up like how
	//  we think wound care coding should work.
	//  Static so it can be called elsewhere.
	static bool AutoConfigure();

// Dialog Data
	enum { IDD = IDD_WOUND_CARE_CODING_SETUP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	DECLARE_EVENTSINK_MAP()
	void LeftClickWoundCareCodingActionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedWcDefaultsBtn();
};
