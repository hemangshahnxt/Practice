#pragma once

// (a.walling 2014-08-06 15:13) - PLID 62686 - Laterality - Setup

// (a.walling 2014-08-06 16:39) - PLID 62688 - Laterality - CEmrActionAnatomyFilterDlg

// CEmrActionFilterAnatomyDlg dialog

class CEmrActionFilterAnatomyDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrActionFilterAnatomyDlg)

public:
	CEmrActionFilterAnatomyDlg(CWnd* pParent, const Emr::ActionFilter& filter);   // standard constructor
	virtual ~CEmrActionFilterAnatomyDlg();

// Dialog Data
	enum { IDD = IDD_EMR_ACTION_FILTER_ANATOMY_DLG };

	// (a.walling 2014-08-06 16:37) - PLID 62687 - Laterality - CEmrActionDlg - needs to support new data for filters
	Emr::ActionFilter m_filter;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	Emr::AnatomyQualifierMap m_qualifierMap;

	void UpdateDetailsColumn(NXDATALIST2Lib::IRowSettingsPtr pRow);

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void LeftClickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	virtual BOOL OnInitDialog();
	virtual void OnOK() override;
};
