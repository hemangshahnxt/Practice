#if !defined(AFX_DOCBAR_H__D1C25B83_015A_11D3_9447_00C04F4C8415__INCLUDED_)
#define AFX_DOCBAR_H__D1C25B83_015A_11D3_9447_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DocBar.h : header file
//

#include <afxdtctl.h>
#include "Marketutils.h"

#define IDC_MAIN_SEARCH		187
#define IDC_REFERING_SEARCH 188
#define IDC_EMPLOYEE_SEARCH 189
#define IDC_OTHER_SEARCH	190
#define IDC_DocS_SEARCH		191
/////////////////////////////////////////////////////////////////////////////
// CDocBar window

#define DBF_LOCATION	0x0001
#define DBF_PROVIDER	0x0002
#define DBF_FILTER		0x0004
#define DBF_DATES		0x0008
#define DBF_CATEGORY	0x0010
#define DBF_RESP		0x0020
//(a.wilson 2011-11-18) PLID 38789 - added for to and from filter changes
#define DBF_TO			0x0021
#define DBF_FROM		0x0022

// (j.gruber 2010-09-08 10:48) - PLID 37425 - changed to be an enum
enum EMarketingDateValues {
	emdvSeparator =  -2,
	emdvAll = -1,
	emdvOneYear = 1,
	emdvCustom,
	emdvToday,	
	emdvThisWeek,
	emdvThisMonth,
	emdvThisQuarter,
	emdvThisYear,
	emdvThisMonthToDate,
	emdvThisQuarterToDate,
	emdvThisYearToDate,
	emdvYesterday,
	emdvLastWeek,
	emdvLastMonth,
	emdvLastQuarter,
	emdvLastYear,
};

class CDocBar : public CDialogBar
{
public:
// Construction
	CDocBar();
	virtual ~CDocBar();

// Attributes
public:
	void SetDoc(long id);
	int GetDoc();
	int GetLoc();
	int SetLoc(long id);
	void SetCategory(long id);
	int GetDocCount();
	int GetDocIndex();
	void SetType(int mktType);//Can be -1
	int GetType();
	void SetFilter(MarketFilter mktFilter, MarketFilterType mfType = mftDate);
	MarketFilter GetFilterType(MarketFilterType mft);
	void InvalidateDlgItem(int nID, bool bErase = TRUE);
	afx_msg void OnUpdatePatFilterButton(CCmdUI* pCmdUI); //j.camacho 7/8/2014 - plid 58876

	CString GetFromDate();
	CString GetToDate();
	CString GetProviderString();
	CString GetLocationString();
	// (j.jones 2010-07-19 15:24) - PLID 39053 - require a connection pointer
	CString GetPatientFilterString(ADODB::_ConnectionPtr pCon, OUT CString &strPatientTempTable);

	// (a.walling 2006-10-11 09:57) - PLID 22764
	CString GetRespFilterString();
	CString GetCategoryFilterString();

	// (a.walling 2006-10-11 09:41) - PLID 22764 - Show the filter criteria in marketing reports
	CString GetDescription(); // gets an English sentence filter description for use on reports.

	CString GetFilterField(MarketFilterType mft);
	BOOL IsFilteringAllDates();
	long GetCategory();
	long GetResp();
	CString GetDisplayNameFromEnum(MarketFilter mdf);
	void EnsureFilter(MarketFilterType mft);
	void SetFromDate(COleDateTime dt);
	void SetToDate(COleDateTime dt);
	//False if All Dates is selected.
	BOOL UseFilter(MarketFilterType mft);
	void SetAllowedFilters(CDWordArray &dwFilters, MarketFilterType mft); //For when the type is -1.

	CString GetLocationFilterString();
	CString GetProviderFilterString();
	CString GetPatFilterFilterString();
	CString GetDateFilterString();
	

// Operations
public:
	void RequeryDoc(BOOL bKeepSelection);
	void RequeryLoc();
	void RequeryCategory();
	BOOL CreateComboBox();
	void ScrollDoc(int i);
	//Pass in a DWORD with all the filters you want to HIDE piped together (see the #define DBF_s above).  All others will be shown.
	void SetHiddenFilters(DWORD dwHidden);
	//these are passed to the marketview
	DWORD m_dwHidden; // (a.walling 2006-10-11 16:13) - PLID 22764 - reliable way to check for hidden filters
	long m_nUseRetention;
	long m_nUseMarketing;
	BOOL CheckProvRequery(MarketFilter mfFilterOld, MarketFilter mfFilterNew);
	void SetDateOptionCombo(MarketDateOptionType mdotSelection);
	void ChangeSelectionFilteredDateOptionList(long nRow, BOOL bIsEvent);

	void OnUserChanged();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDocBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:

	// Generated message map functions
protected:
	// (a.walling 2008-04-21 15:24) - PLID 29642 - Added handler for EraseBkgrnd; delegates to parent
	// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint, Paint)
	//{{AFX_MSG(CDocBar)
	afx_msg void OnSelectionChangeCombo(long iNewRow);
	afx_msg void OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnPullupFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPullupTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelChosenPatFilterList(long nRow);
	afx_msg void OnSelChosenFilteredProviderList(long nRow);
	afx_msg void OnSelChosenFilteredLocationList(long nRow);
	afx_msg void OnSelChosenFilteredDateOptionList(long nRow);
	afx_msg void OnSelChosenFilteredCategoryList(long nRow);
	afx_msg void OnSelChosenFilteredRespList(long nRow);
	afx_msg void OnRequeryFinishedLocationCombo(short nFlags);
	afx_msg void OnRequeryFinishedDocCombo(short nFlags);
	afx_msg void OnRequeryFinishedPatFilterCombo(short nFlags);
	afx_msg void OnRequeryFinishedCategoryCombo(short nFlags);
	afx_msg void OnChangeFilteredToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeFilteredFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditFilter();
	afx_msg void OnSelChangingFilteredDateOptionList(long FAR* nNewSel);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNcPaint();
	afx_msg void OnPaint();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	// (a.walling 2008-04-21 15:24) - PLID 29642 - Use NxButton, CNxStatic
	CNxIconButton m_main, 
			m_referring, 
			m_employee, 
			m_other, 
			m_all;
	CNxStatic m_searchText;
	CFont m_searchButtonFont;
	CFont *m_pFont;
	bool m_fromChanged;
	bool m_toChanged;
	CNxLabel m_Loc, m_Prov;
	CNxStatic  m_PatFilter, m_From, m_To, m_Category, m_Resp, m_blank, m_blank2, m_blank3, m_blank4;
	CNxLabel m_Date;
	CNxLabel m_nxlProviderLabel;
	CNxLabel m_nxlLocationLabel;
	CNxLabel m_nxlPatFilterLabel;
	int m_Type; //Either a MarketGraphType, or -1
	MarketFilter m_DateFilter;
	MarketFilter m_LocFilter;
	MarketFilter m_ProvFilter;
	
	CWnd m_wndDocCombo;
	CWnd m_wndLocCombo;
	CWnd m_wndCatCombo;
	CWnd m_wndPatFilterCombo;
	CWnd m_wndDateCombo;
	CWnd m_wndRespCombo;
	NXDATALISTLib::_DNxDataListPtr m_docCombo;
	NXDATALISTLib::_DNxDataListPtr m_locCombo;
	NXDATALISTLib::_DNxDataListPtr m_categoryCombo;
	NXDATALISTLib::_DNxDataListPtr m_PatFilterCombo;
	NXDATALISTLib::_DNxDataListPtr m_DateCombo;
	NXDATALISTLib::_DNxDataListPtr m_RespCombo;
	// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
	CDateTimePicker	m_FromDate;
	CDateTimePicker	m_ToDate;
	CNxStatic			m_fromText;
	CNxStatic			m_toText;
	CNxStatic			m_lblBlank;
	CNxIconButton m_btnPatFilter;
	

	BOOL OnMultiSelectProviders();
	BOOL OnMultiSelectLocations();
	BOOL AddNewFilter();
	
	


	//Used to keep track of which row to select when we requery.
	long m_nDocCurSel;
	long m_nLocCurSel;
	long m_nCatCurSel;

	CDWordArray m_dwProvIDs;
	CDWordArray m_dwLocIDs;
	long m_nPatFilterID;

	CString m_strMultiProvString;
	CString m_strMultiLocString;
	
	//For when the "type" is -1.
	CDWordArray m_dwAllowedDateFilters;
	CDWordArray m_dwAllowedLocationFilters;
	CDWordArray m_dwAllowedProviderFilters;



};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DocBAR_H__D1C25B83_015A_11D3_9447_00C04F4C8415__INCLUDED_)

