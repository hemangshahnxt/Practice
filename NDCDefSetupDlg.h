#pragma once
#include "AdministratorRc.h"

// CNDCDefSetupDlg dialog

// (j.dinatale 2012-06-12 11:07) - PLID 32795 - Created

class CNDCDefSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNDCDefSetupDlg)

public:
	CNDCDefSetupDlg(long nServiceID, bool bFromInventoryMod = false, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNDCDefSetupDlg();

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Dialog Data
	enum { IDD = IDD_NDC_DEF_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxColor m_nxcolorBackground;
	bool m_bFromInvModule;

	NXDATALIST2Lib::_DNxDataListPtr m_dlUnitType;

	void SetupUnitTypeList();
	void RestrictFieldLengths();
	void LoadInfo();
	long m_nServiceID;

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChangingNdcDefUnitType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
