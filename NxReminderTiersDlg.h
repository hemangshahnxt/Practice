#include "UIFormChangesTracker.h"
#pragma once

// (f.dinatale 2010-10-22) - PLID 40827 - Dialog created
// CNxReminderTiersDlg dialog

class CNxReminderTiersDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNxReminderTiersDlg)

public:
	CNxReminderTiersDlg(CWnd* pParent);   // standard constructor
	virtual ~CNxReminderTiersDlg();

// Dialog Data
	enum { IDD = IDD_NXREMINDER_TIERS_DLG };

	enum dlNxReminderTiers{
		dlTiersID = 0,
		dlTiersName,
		dlTiersMessages,
		dlTiersPrice,
	};

protected:
	// (f.dinatale 2010-10-25) - PLID 40827 - Member variables to help keep track of changes.
	CString m_strTierName;
	CString m_strMessages;
	COleCurrency m_curPrice;

	BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	NXDATALIST2Lib::_DNxDataListPtr m_dlTiers;
	CUIFormChangesTracker m_uifctTiers;

	afx_msg void OnTiersSaveclose();
	afx_msg void OnTiersAdd();
	DECLARE_EVENTSINK_MAP()
	void OnSelChangedTiersList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
