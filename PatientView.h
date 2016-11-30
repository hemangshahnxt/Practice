// PatientView.h : interface of the CPatientView class
//
/////////////////////////////////////////////////////////////////////////////

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

#if !defined(AFX_PATIENTVIEW_H__F2B94DB7_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
#define AFX_PATIENTVIEW_H__F2B94DB7_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CBillingModuleDlg;

// (j.jones 2013-05-07 15:57) - PLID 56591 - removed the .h files for the child tabs
class CGeneral1Dlg;
class CGeneral2Dlg;
class CCustom1Dlg;
class CFollowUpDlg;
class CAppointmentsDlg;
class CHistoryDlg;
class CNexPhotoDlg;
class CFollowUpDlg;
class CNotesDlg;
class CInsuranceDlg;
class CFinancialDlg;
class CQuotesDlg;
class CMedicationDlg;
class CPatientProcedureDlg;
class CSupportDlg;
class CSalesDlg;
class CRefractiveDlg;
class CCustomRecordsTabDlg;
class CPatientNexEMRDlg;
class CPatientLabsDlg;
class CImplementationDlg;
class CPatientDashboardDlg;
class CProcInfoCenterDlg;
struct AppointmentInfo;

class CPatientView : public CNxTabView
{

	friend class CBillingModuleDlg;
	friend class CFinancialDlg;
	friend class CMainFrame;

public:

protected: // create from serialization only
	CPatientView();
	DECLARE_DYNCREATE(CPatientView)

public:
	void Delete	(void);

	int m_iWarningPatientID;
	//TES 1/6/2010 - PLID 36761 - Need to remember the current status, otherwise occasionally the color won't update properly.
	short m_nCurrentStatus;
	
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh (also restore virtual qualifier)
	virtual void UpdateView(bool bForceRefresh = true);

	void PromptPatientWarning(); // Checks to see if there is a
								// warning for the patient, and
								// displays it if necessary

	void NeedToDropPatientList(bool Set = false);

	CBillingModuleDlg * GetBillingDlg();

	CFinancialDlg* GetFinancialDlg(); // (z.manning 2008-09-22 16:51) - PLID 31252

	//Event Tracking
	void OpenBill(int nBillID);
	void OpenQuote(int nQuoteID);
	void OpenLetter(int nMailID);
	void OpenPacket(int nMergedPacketID);
	void OpenPayment(int nPayID);
	void OpenEMR(int nEmrID);

	virtual void SetDefaultControlFocus();

	int ShowPrefsDlg();
	// (a.wilson 2012-07-06 15:48) - PLID 51332 - adding silent flag for default tab preference.
	// (j.gruber 2012-06-19 14:23) - PLID 51067
	BOOL CheckIEVersion(BOOL bSilent = FALSE);

	//{{AFX_DATA(CPatientView)
	//}}AFX_DATA

// Attributes
public:
	void SetColor();
// Operations
public:
	BOOL CheckPermissions();
	void ShowTabs();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual void OnSelectTab(short newTab, short oldTab);//used for the new NxTab
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnDraw(CDC* pDC);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (j.gruber 2012-06-11 16:02) - PLID 50225
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	virtual void OnInitialUpdate();
	LRESULT OnPreviewClosed(WPARAM wParam, LPARAM lParam);
	LRESULT OnPreviewPrinted(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	//(a.wilson 2012-1-13) PLID 47485
	virtual LRESULT OnOPOSBarcodeScannerEvent(WPARAM wParam, LPARAM lParam);
	// (a.walling 2008-09-10 13:31) - PLID 31334
	afx_msg LRESULT OnWIAEvent(WPARAM wParam, LPARAM lParam);
	// (a.walling 2009-06-05 12:52) - PLID 34496
	afx_msg LRESULT OnPostOnDraw(WPARAM wParam, LPARAM lParam);
	// (a.walling 2009-12-22 17:49) - PLID 7002 - Prevent closing if a bill is active
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam); 
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPatientView();
	virtual int Hotkey(int key);
	// (d.singleton 2012-05-17 15:40) - PLID 50471 in order to print pre op calendar from PIC
	BOOL m_bIsPicPreOpCalendar;
	BOOL bMedSchedPrintInfoLoaded;
	long m_tempMedSchedID;
	CArray<AppointmentInfo, AppointmentInfo> *m_arAppointmentInfo;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	// (a.walling 2008-05-07 13:56) - PLID 29951 - Called from OnSelectTab; handles most everything.
	void HandleSelectTab(short newTab, short oldTab);

	CBillingModuleDlg *m_BillingDlg;
	short m_CurrentTab;
	HANDLE m_hCheckPatientWarning;
	bool m_bNeedToDropPatientList;
	
	// (j.jones 2013-05-07 15:58) - PLID 56591 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CGeneral1Dlg			&m_General1Sheet;
	CGeneral2Dlg			&m_General2Sheet;
	CCustom1Dlg				&m_CustomSheet;
	CInsuranceDlg			&m_InsuranceSheet;
	CNotesDlg				&m_NotesSheet;
	CFollowUpDlg			&m_FollowUpSheet;
	CAppointmentsDlg		&m_AppointmentsSheet;
	CHistoryDlg				&m_HistorySheet;
	CMedicationDlg			&m_MedicationSheet;
	CFinancialDlg			&m_FinancialSheet;
	CQuotesDlg				&m_QuotesSheet;
	CPatientProcedureDlg	&m_ProcedureSheet;
	CCustomRecordsTabDlg	&m_CustomRecordsSheet;
	CPatientNexEMRDlg		&m_NexEMRSheet;
	CSupportDlg				&m_SupportSheet;
	CSalesDlg				&m_SalesSheet;
	CRefractiveDlg			&m_RefractiveSheet;
	CPatientLabsDlg			&m_LabSheet;
	// (j.gruber 2007-11-07 11:11) - PLID 28023 - created tab
	CImplementationDlg		&m_ImplementationSheet;
	// (c.haag 2009-08-19 12:45) - PLID 35231 - NxPhotoTab
	CNexPhotoDlg			&m_NexPhotoSheet;
	// (a.wilson 2012-07-05 11:03) - PLID 51332 - renamed to follow naming paradigm.
	CPatientDashboardDlg	&m_PatientDashboardSheet;

	// (d.singleton 2012-04-05 17:27) - PLID 50471
	CStringArray m_astrMedSchedEntries[367];
	CDWordArray m_adwMedSchedColors[367];
	long m_nMedSchedDays;
	void LoadMedSchedPrintInfo();
	void OnBeginPreOpCalendarPrinting(CDC* pDC, CPrintInfo* pInfo);
	void PrintPreOpCalendar(CDC *pDC, CPrintInfo *pInfo);
	long DrawPageHeader(CDC *pDC, LPRECT rcPage, long nProcId);
	long DrawDayNumber(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey);
	long DrawDayHeader(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey);
	BOOL DoesDayOutputFit(CDC *pDC, CStringArray *pastrNames, long nBoxWidth, long nBoxHeight);

	//{{AFX_MSG(CPatientView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSuperbill();
	afx_msg void OnPatGroupDlg();
	afx_msg LRESULT OnImageProcessingCompleted(WPARAM wParam, LPARAM lParam);
	// (j.jones 2015-03-16 15:08) - PLID 64926 - renamed to reflect that this
	// hides all billing tool windows
	afx_msg LRESULT OnHideBillingToolWindows(WPARAM wParam, LPARAM lParam);
	afx_msg void OnReferralOrders(); // (z.manning 2009-05-05 10:08) - PLID 34172
	afx_msg void OnOrderSets(); // (z.manning 2009-05-07 11:04) - PLID 28554
	afx_msg void OnImmunizations();
	afx_msg void OnExportSummaryofCare(); // (j.gruber 2013-11-05 12:27) - PLID 59323
	afx_msg void OnAdvanceDirectives(); // (a.walling 2009-06-01 11:47) - PLID 34410
	afx_msg void OnGraphPatientEMRData();
	// (j.fouts 2012-06-06 11:08) - PLID 49611 - Handle the changing of preferences
	afx_msg LRESULT OnPreferenceUpdated(WPARAM wParam, LPARAM lParam);
	// (r.gonet 2016-05-24 15:54) - NX-100732 - Handle when the connected system's name changes, ie
	// a remote desktop session has just ended or started. Pass it along to listeners.
	afx_msg LRESULT OnSystemNameUpdated(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnFilePrintPreview();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTVIEW_H__F2B94DB7_9A7D_11D1_B2C7_00001B4B970B__INCLUDED_)
