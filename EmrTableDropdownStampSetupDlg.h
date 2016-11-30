#pragma once


// CEmrTableDropdownStampSetupDlg dialog
// (z.manning 2011-09-28 12:29) - PLID 45729 - Created

class CEmrTableDropdownStampSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrTableDropdownStampSetupDlg)

public:
	CEmrTableDropdownStampSetupDlg(CEmrTableDropDownItem *pDrowpdownItem, CWnd* pParent);   // standard constructor
	virtual ~CEmrTableDropdownStampSetupDlg();

	// (j.jones 2012-11-28 10:15) - PLID 53144 - added array of all dropdowns, so we can
	// confirm if a stamp is filtered in any other dropdown
	CEmrTableDropDownItemArray *m_arypEMRDropDownList;

// Dialog Data
	enum { IDD = IDD_EMR_TABLE_DROPDOWN_STAMP_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	// (j.jones 2012-11-26 17:43) - PLID 53144 - stamp filters and defaults are now tracked separately
	BOOL m_bChangedFilter;
	BOOL m_bChangedDefaults;

	CEmrTableDropDownItem *m_pDropdownItem;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlStamps;
	enum EStampListColumns {
		slcCheck = 0,
		slcID,
		slcStamp,
		slcDefault, // (z.manning 2011-10-12 09:59) - PLID 45728
	};

	CNxColor m_nxcolor;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	void SelectAll(BOOL bSelect);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDropdownStampSelectAll();
	afx_msg void OnBnClickedDropdownStampSelectNone();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedEmrDropdownStampList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
