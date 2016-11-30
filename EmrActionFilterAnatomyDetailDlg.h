#pragma once

// (a.walling 2014-08-06 15:13) - PLID 62686 - Laterality - Setup

// (a.walling 2014-08-06 16:39) - PLID 62690 - Laterality - CEmrActionAnatomyFilterDetailDlg - Side / Qualifier Selection

// CEmrActionFilterAnatomyDetailDlg dialog

class CEmrActionFilterAnatomyDetailDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrActionFilterAnatomyDetailDlg)

public:
	CEmrActionFilterAnatomyDetailDlg(CWnd* pParent, const Emr::ActionFilter& filter, long anatomicLocationID, CString anatomicLocationName);   // standard constructor
	virtual ~CEmrActionFilterAnatomyDetailDlg();

// Dialog Data
	enum { IDD = IDD_EMR_ACTION_FILTER_ANATOMY_DETAIL_DLG };
	
	Emr::ActionFilter m_filter;
	long m_anatomicLocationID = -1;
	CString m_anatomicLocationName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void RButtonDownList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void ShowContextMenuList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue);
	virtual BOOL OnInitDialog();
};
