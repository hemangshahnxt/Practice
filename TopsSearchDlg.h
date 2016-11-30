#pragma once

// CTopsSearchDlg dialog
//(e.lally 2009-10-07) PLID 35803 - Created

// (j.gruber 2009-11-23 12:47) - PLID 36139 - get TOPS values

struct TOPSSelection {
		long nSelectID;
		CString strSelection;				
};

struct TOPSField {
		long nItemID;
		CString strRow;		
		CArray<TOPSSelection*, TOPSSelection*> *parySelections;
	};

struct TOPLevel {
		CString strCode;
		CString strCodeDesc;
		BOOL bAllowMultiples;
		BOOL bIsCCDField;
		CArray<TOPSField*, TOPSField*> *aryTOPS;
	};

class CTopsSearchDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CTopsSearchDlg)

public:
	CTopsSearchDlg(CWnd* pParent);   // standard constructor
	virtual ~CTopsSearchDlg();

// Dialog Data
	enum { IDD = IDD_TOPS };
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxIconButton m_btnSearch;
	CNxIconButton m_btnSend;
	NxButton	m_checkShowMasterProcs;
	CNxStatic	m_nxstaticSearchResults;
	CNxLabel	m_nxlMultiProviders;
	CNxLabel	m_nxlMultiProcedures;
	// (f.dinatale 2010-09-09) - PLID 38192 - added place of service filter
	CNxLabel	m_nxlMultiLocations;
	// (j.jones 2009-11-03 15:07) - PLID 36137 - added submission filters
	NxButton	m_radioShowSubmitted;
	NxButton	m_radioShowUnsubmitted;
	NxButton	m_radioShowAll;
	// (d.thompson 2009-11-04) - PLID 36136 - Added username/password/proxied
	CNxEdit		m_nxeditUsername;
	CNxEdit		m_nxeditPassword;
	CNxEdit		m_nxeditProxiedUser;
	//(e.lally 2009-11-24) PLID 36415 - added billing vs scheduler searches
	NxButton	m_radioBillingSearch;
	NxButton	m_radioAppointmentSearch;
	CNxIconButton m_btnTopsOptions; //(e.lally 2010-02-16) PLID 36849 
	

protected:
	// (f.dinatale 2010-09-09) - PLID 38192 - Location filtering
	NXDATALIST2Lib::_DNxDataListPtr m_pTopsSearchList, m_pProviderFilter, m_pProcedureFilter, m_pLocationFilter;
	CArray<long,long> m_arProviderFilterIDList;
	CArray<long,long> m_arProcFilterIDList;
	CArray<long,long> m_arLocFilterIDList;
	BOOL m_bNotifyOnce;
	BOOL m_bDateFromDown;
	BOOL m_bDateToDown;
	//Not yet supported
	//CGenericBrowserDlg* m_pBrowserDlg;
	//(e.lally 2010-09-15) PLID 40532
	BOOL m_bFilterShowInactiveProviders, m_bFilterShowInactiveLocations;
	CTableChecker m_doctorChecker, m_resourceChecker, m_locationChecker;

	void SetProcedureFilterWhere(BOOL bShowOnlyMasterProcedures);
	void EnsureControls();
	void ClearResults();//(e.lally 2009-11-25) PLID 35803
	void RequeryProviderList(); //(e.lally 2009-11-25) PLID 36415
	CString FormatTopsHiddenElement(CString strElementName, CString strData);
	CString FormatTopsTextAreaElement(CString strElementName, CString strData);
	CString CreateFieldValuePairForHttpPost(CString strField, CString strValue);
	void PostFromTempHtmlFile(CString strUrl, CString strUsername, CString strPW, CString strProxiedUsername, CString strFormID, CString strPrepopXml);
	CString GenerateRawTopsPostData(CString strUsername, CString strPW, CString strProxiedUsername, CString strFormID, CString strPrepopXml);
	void IEDirectPostToUrl(CString strUrl, CString strRawPostData);
	void PostFromBuiltInBrowser(CString strUrl, CString strRawPostData);
	void SelectMultiProviders();
	void SelectMultiProcedures();
	void SelectMultiLocations(); // (f.dinatale 2010-09-09) - PLID 38192
	void EnsureValidSearchDateRange();//(e.lally 2009-11-25) PLID 35803
	void OnBnClickedOptions(); //(e.lally 2010-02-16) PLID 36849 
	void ConfigureServiceCodeLinks(); //(e.lally 2010-02-16) PLID 36849 
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true); //(e.lally 2010-09-15) PLID 40532

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRefreshSearch();
	afx_msg void OnBnClickedSendToTops();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedSearchList(short nFlags);
	void OnRequeryFinishedProviderFilter(short nFlags);
	void OnRequeryFinishedProcedureFilter(short nFlags);
	// (f.dinatale 2010-09-08) - PLID 38192 - Location filter added
	void OnRequeryFinishedLocationList(short nFlags);
	void RequeryLocationList();
	void OnSelChosenLocationFilter(LPDISPATCH lpRow);
	void OnSelChosenProviderFilter(LPDISPATCH lpRow);
	void OnSelChosenProcedureFilter(LPDISPATCH lpRow);
	afx_msg void OnBnClickedTopsOnlyMasterProcedures();
	// (j.jones 2009-11-03 15:07) - PLID 36137 - added submission filters
	afx_msg void OnShowUnsubmitted();
	afx_msg void OnShowSubmitted();
	afx_msg void OnShowAll();
	//(e.lally 2009-11-24) PLID 36415 - Added option for billing vs scheduler data searches
	afx_msg void OnClickedBillingSearch();
	afx_msg void OnClickedApptSearch();
	//(e.lally 2009-11-25) PLID 35803 - Added for our date range control message handling
	afx_msg void OnChangeSearchFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeSearchToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdownSearchFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDropdownSearchToDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnCloseupSearchFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnCloseupSearchToDate(NMHDR *pNMHDR, LRESULT *pResult);
	
	// (j.gruber 2009-11-23 12:47) - PLID 36139 - send TOPS fields
	CString GenerateTopsFields(long nPatientID, COleDateTime dtProcDate, CString strPatientName, CString &strCCDExcludedItems, CString &strCCDExcludedDetails);
	CString GenerateStringFromMemory(CArray<TOPLevel*, TOPLevel*> *aryTopLevel, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapSelectedValues );
	void GetValuesFromArray(CArray<TOPSField*, TOPSField*> *aryTOPS, CStringArray *strAry, long nItemID);
	void GetExcludedCCDFields(CArray<TOPLevel*, TOPLevel*> *aryTopLevel, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapSelectedValues, CString &strExcludedDetails, CString &strExcludedItems );
	CString GetFooterForMultiItem(CString strCode);
	CString GetHeaderForMultiItem(CString strCode);
	CString GetEndTOPSXMLFromCode(CString strCode);
	CString GetStartTOPSXMLFromCode(CString strCode);
	

	
public:
	afx_msg void OnEnKillfocusTopsUsername();
	afx_msg void OnEnKillfocusTopsProxiedName();
};
