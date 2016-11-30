#if !defined(AFX_ZIPCODES_H__0E677AA6_DE04_11D2_B6B7_00104B2FE914__INCLUDED_)
#define AFX_ZIPCODES_H__0E677AA6_DE04_11D2_B6B7_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

#include "client.h"
#include "ZipcodeUtils.h"
#include "CityZipUtils.h"

// Zipcodes.h : header file
//
#define ID_ZIP_ADD		32768
#define ID_ZIP_DELETE	32769
#define ID_ZIP_PRIMARY	32770
#define ID_REM_ZIP_PRIMARY 32771
/////////////////////////////////////////////////////////////////////////////
// CZipcodes dialog

class CZipcodes : public CNxDialog
{
// Construction
public:
	CZipcodes(CWnd* pParent);   // standard constructor

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void LoadZiplist();
	//TES 9/15/03: I'm taking out this function because it is far slower than just calling m_pZipCodes->Clear();
	//void ClearZipBox();
	//void TrimZiplist();
	CBrush m_brush;

	// (z.manning, 04/16/2008) - PLID 29566 - Set button styles
// Dialog Data
	//{{AFX_DATA(CZipcodes)
	enum { IDD = IDD_ZIP_CODES };
	NxButton	m_btnPromptPrimary;
	NxButton	m_btnSearchZip;
	NxButton	m_btnSearchArea;
	NxButton	m_btnSearchCity;
	NxButton	m_btnSearchModifiedZips;
	NxButton	m_btnSearchAllZips;
	CNxIconButton	m_btnAddZipCode;
	CNxIconButton	m_btnMakePrimary;
	CNxStatic	m_lblSearch;
	COleVariant m_varBoundItem;
	CNxEdit	m_nxeditZipSearch;
	CNxEdit	m_nxeditDefaultAreaCode;
	CNxStatic	m_nxstaticZipstatus;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_pZipCodes;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZipcodes)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CTableChecker m_zipChecker, g_ZipCodePrimaryChecker;
	//_ConnectionPtr m_pMDB;
	//long m_nOldSize;	//for the edit control
	long m_nSearchType;	//1 = ZipCode; 2 = AreaCode; 3 = City; 4 = All Modified; 5 = All
	void FilterSearch();

	// (a.walling 2006-12-06 09:39) - PLID 23754
	CWinThread* m_pLoadZipCodesThread;	// our Zip query thread
	HANDLE m_hStopLoadingZips;			// our event to tell the thread to stop and exit
	bool m_bDestroying;					// true if the window is being destroyed, so we can clean up mem.

	// (a.walling 2006-12-06 09:39) - PLID 23754
	DWORD StopZipRequery(long nWait = 0); // stops the thread by setting event and posting message
	void StartZipRequery(ZipcodeUtils::EZipcodeField ezfField, CString strCriteria); // starts a new query thread, and stops any existing ones.

	static CString GetDatabaseField(ZipcodeUtils::EZipcodeField ezfField);
	static CString GetDatabaseField(CityZipUtils::ECityZipField ezfField);

	// (j.gruber 2009-10-07 12:34) - PLID 35827 - make dialog work with city file
	void StartCityRequery(CityZipUtils::ECityZipField ezfField, CString strCriteria); // starts a new query thread, and stops any existing ones.
	
	// Generated message map functions
	//{{AFX_MSG(CZipcodes)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnRButtonDownZipCodes(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingZipcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedZipcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnMakePrimary();
	afx_msg void OnRemPrimary();
	afx_msg void OnPrompt();
	afx_msg void OnModZip();
	afx_msg void OnDelete();
	afx_msg void OnClose();
	afx_msg void OnAddZip();
	afx_msg void OnRequeryFinishedZipcodes(short nFlags);
	afx_msg void OnSearchZip();
	afx_msg void OnSearchArea();
	afx_msg void OnSearchCity();
	afx_msg void OnBtnSearch();
	afx_msg void OnKillfocusDefaultAreaCode();
	afx_msg void OnAllZips();
	afx_msg LRESULT OnZipCodesLoaded(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZIPCODES_H__0E677AA6_DE04_11D2_B6B7_00104B2FE914__INCLUDED_)
