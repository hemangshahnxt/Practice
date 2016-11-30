#pragma once
#include "EmrRc.h"
#include "CCHITReportInfoListing.h"

// CEmrMUProgressSetupDlg dialog
//(e.lally 2012-03-21) PLID 48264 - Created
class CEmrMUProgressSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrMUProgressSetupDlg)

public:
	//(e.lally 2012-04-03) PLID 48264
	//This is stored in data, do not change the values
	enum EMuDateValues {
		mudvCustom = 1,
		mudvThisMonthToDate = 2,
		mudvThisQuarterToDate = 3,
		mudvThisYearToDate = 4,
	};

	CEmrMUProgressSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEmrMUProgressSetupDlg(){}

	void SetInitialProviderID(long nProviderID);

// Dialog Data
	enum { IDD = IDD_EMR_MU_PROGRESS_SETUP_DLG };

protected:
	CNxIconButton m_btnClose;
	bool m_bIsStartDateDroppedDown;
	//To be used as the default provider being loaded on initialization only
	long m_nInitialProviderID;

	CCHITReportInfoListing m_cchitReportListing;
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderList, m_pMeasureList, m_pDateOptionList;
	CDateTimePicker m_dtStart;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	virtual void OnChangeStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnCloseUpStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnDropDownStartDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnExcludeSecProvider();

	DECLARE_EVENTSINK_MAP()
	void OnSelChosenProvider(LPDISPATCH lpRow);
	void OnSelChosenDateOption(LPDISPATCH lpRow);
	void OnEditingFinishedMeasures(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	void OnRequeryFinishedProviders(short nFlags);

	void Load(long nProviderID);
	void SaveStartDate();
	void EnsureControls();
	long GetCurrentProviderID();
	void ResetMeasureList();
};
