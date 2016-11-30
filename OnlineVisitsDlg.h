#pragma once

#include "NxAPI.h"

class COnlineVisitsDlg : public CNxDialog
{

	DECLARE_DYNAMIC(COnlineVisitsDlg)

protected:
	HICON m_hMagnifyingGlass;

public:
	COnlineVisitsDlg(CWnd* pParent);
	virtual ~COnlineVisitsDlg();

// Dialog Data
	enum { IDD = IDD_ONLINE_VISITS_DLG };


protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void FilterVisitsList(); // (r.farnworth 2016-03-02 15:14) - PLID 68455
	void DisplayPDF(NXDATALIST2Lib::IRowSettingsPtr pRow); // (r.farnworth 2016-03-08 11:42) - PLID 68396
	void SpawnAssignmentDlg(NXDATALIST2Lib::IRowSettingsPtr pRow); // (r.farnworth 2016-03-09 07:50) - PLID 68400
	void GenerateFile(NXDATALIST2Lib::IRowSettingsPtr pRow, CString strFullFileName); // (r.farnworth 2016-03-07 08:35) - PLID 68401
	long AttachToHistory(const CString &strFilePath, const CString &strProvider, const CString &strDescription, const COleDateTime &dtCreated, long nPatientID, int nRowID); // (r.farnworth 2016-03-07 08:35) - PLID 68401
	void PopulateVisitsListWithResults(NexTech_Accessor::_OnlineVisitsPtr pOnlineVisits); // (r.farnworth 2016-03-08 11:42) - PLID 68396
	void EnableImportButton(); // (r.farnworth 2016-03-07 08:35) - PLID 68401
	void PopulateProviderDropdown(); // (r.farnworth 2016-03-09 12:11) - PLID 68455
	_variant_t ValidateVarDate(_variant_t vtDate);

	CMap<int, int, bool, bool> m_mapCheckedRows;
	bool m_bHistoryWritePerm; // (r.farnworth 2016-03-07 08:35) - PLID 68401

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

public:
	afx_msg void OnBnClickedRefreshButton();
	afx_msg void OnBnClickedConfigureButton();
	afx_msg void OnBnClickedImportButton();
	void OnLeftClickOnlineVisitsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingFinishedOnlineVistsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnSelChosenProviderCombo(LPDISPATCH lpRow);
	void OnSelChangingProviderCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

	CNxIconButton	m_btnRefresh;
	CNxIconButton	m_btnConfigure;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pOnlineVisits;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProviderCombo; // (r.farnworth 2016-03-02 15:14) - PLID 68455
		
};
