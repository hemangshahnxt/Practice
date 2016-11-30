#pragma once


// CEmrItemSelectStampsDlg dialog
// (z.manning 2011-10-24 10:21) - PLID 46082 - Created

class CEmrItemStampExclusions;

class CEmrItemSelectStampsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrItemSelectStampsDlg)

public:
	CEmrItemSelectStampsDlg(CEmrItemStampExclusions *pStampExclusions, CWnd* pParent);   // standard constructor
	virtual ~CEmrItemSelectStampsDlg();

// Dialog Data
	enum { IDD = IDD_EMR_ITEM_STAMP_SETUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CEmrItemStampExclusions *m_pStampExclusions;

	NXDATALIST2Lib::_DNxDataListPtr m_pdlStamps;
	enum StampListColumns {
		slcCheck = 0,
		slcID,
		slcName,
	};

	CNxColor m_nxcolor;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;

	void Load();
	void OnOK();

	void SelectAll(BOOL bSelect);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
public:
	afx_msg void OnBnClickedEmrItemStampSelectAll();
	afx_msg void OnBnClickedEmrItemStampSelectNone();
};
