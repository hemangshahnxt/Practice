#if !defined(AFX_AUDITING_H__1B350021_D5A7_11D4_A760_0001024317D6__INCLUDED_)
#define AFX_AUDITING_H__1B350021_D5A7_11D4_A760_0001024317D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Auditing.h : header file
//
#include "NxSocketUtils.h"
/////////////////////////////////////////////////////////////////////////////
// CAuditing dialog

class CAuditing : public CNxDialog
{
// Construction
public:
	~CAuditing();
	CString GetItemDescription(long ItemID);
	void RebuildList();
	void PurgeCurrent();
	void PurgeAll();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	CAuditing(CWnd* pParent);   // standard constructor
	void OnDateRange();
	BOOL m_bPurgeDBEnsured;
	// (a.walling 2012-02-17 15:43) - PLID 32916 - The query param is now only for filtering on an auditdetailst.id
	// also returns # affected
	long PurgeToDisk(const CString& strAuditDetailsFilter);
	long PurgeToDisk();

	// (s.dhole 2011-07-15 15:50) - PLID 44593 Set IsRequerying Flag 
	void EnableControls(BOOL bIsRequerying = TRUE);
	void ChooseFirstCategory();

	BOOL CanPurgeCurrentCategory(BOOL bCheckPermission);

	CString m_strCellString;

	short m_iFilterColumnID;
	_variant_t m_varFilterColumnData;

	BOOL m_bFiltered;

	// (c.haag 2010-09-07 16:48) - PLID 40439 - Changed for DL2
	NXDATALIST2Lib::_DNxDataListPtr m_AuditList;
	// (c.haag 2010-09-09 10:42) - PLID 40198 - We don't want the user to see the results of the
	// requery until after the list is fully populated. The reason for that is when they see partial results,
	// they will think the requery is finished. There's also some odd behavior at the time of this writing
	// where off-screen results are not accessible by scrollbar; and if you cancel the requery, they never
	// appear at all. Adam told me he's making general improvements to the datalist in the future. For now,
	// we will work around things using a "decoy list"; which is an always-empty version of the audit list.
	// During a requery, the user will only see the decoy list.
	NXDATALIST2Lib::_DNxDataListPtr m_DecoyList;

	long m_iRow;
	// (a.walling 2008-05-28 11:24) - PLID 27591 - Use CDateTimePicker
// Dialog Data
	//{{AFX_DATA(CAuditing)
	enum { IDD = IDD_AUDITING };
	CNxIconButton	m_btnPurge;
	CNxIconButton	m_btnAdvancedOptions; // (a.walling 2009-12-18 10:46) - PLID 36626
	CNxIconButton	m_btnCopyOutput; // (b.cardillo 2010-01-07 13:26) - PLID 35780
	CNxIconButton	m_btnAuditValidation;
	NxButton	m_radioAllDates;
	NxButton	m_radioDateRange;
	NxButton	m_financialButton;
	NxButton	m_scheduleButton;
	NxButton	m_patientButton;
	NxButton	m_inventoryButton;
	NxButton	m_contactsButton;
	NxButton	m_miscButton;
	NxButton	m_allButton;
	NxButton	m_highButton;
	NxButton	m_topButton;
	NxButton	m_palmButton;
	NxButton	m_emrButton;
	NxButton	m_insuranceButton;
	NxButton	m_backupButton;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxStatic	m_staticProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuditing)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	// (c.haag 2010-09-08 13:01) - PLID 40198 - This will store connection settings for the datalist
	ADODB::_ConnectionPtr m_pConAudit;

// Implementation
public:
	// (c.haag 2010-09-07 17:41) - PLID 40198 - This is the dialog-level override for getting the description
	// of the audit item. For efficiency purposes, we do lookups in a variant map before deferring to the global
	// version.
	_variant_t GetAuditItemDescription(long nItemID);

protected:
	// (c.haag 2010-09-07 17:21) - PLID 40198 - This should be used in place of GetValue whenever we want to
	// fetch a value from a row
	_variant_t GetRowValue(NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol);
	// (c.haag 2010-09-07 17:21) - PLID 40198 - This function is used to build the query components that will be
	// used in the audit list requery. This function will also update column titles appropriately.
	void PrepareListForRebuild(OUT CString& strFrom, OUT CString& strWhere, OUT CString& strSection,
											OUT AuditEventItems& aeiWhatWasViewed);
	// (c.haag 2010-09-07 17:41) - PLID 40198 - Clears the map of cached audit descriptions used during painting
	void ClearAuditItemDescriptionMap();

protected:	
	CMap<long,long,_variant_t,_variant_t> m_mapCachedAuditDescriptions; // (c.haag 2010-09-07 17:41) - PLID 40198 - The map of cached audit descriptions used during a requery
	class CAuditing_JITCellValue* m_pJITItemListValue; // (c.haag 2010-09-07 16:10) - PLID 40198 - Details below
	long m_nRequeryDurationInSeconds; // (c.haag 2010-09-09 10:59) - PLID 40198 - The number of seconds that have elapsed since the requery started
	NxSocketUtils::HCLIENT m_hSocket;		//For NxServer
	BOOL m_bShowSysComponent;
	void EnsurePurgeDatabaseInNxServer(const CString& strPurgeTo);
	void AddDatabaseToNxServer(char* szList, const CString& strNewDatabase);
	void EnsureSystemComponentVisibility();
	//(r.wilson 11/13/2013) PLID 59468 - is the Purge button hidden or not
	BOOL m_bPurgeBtnHidden;

	// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
	// Generated message map functions
	//{{AFX_MSG(CAuditing)
	virtual BOOL OnInitDialog();
	afx_msg void OnClickBtnScheduler();
	afx_msg void OnFinancial();
	afx_msg void OnSchedule();
	afx_msg void OnPatient();
	afx_msg void OnAll();
	afx_msg void OnHigh();
	afx_msg void OnTop();
	afx_msg void OnRadioAllDates();
	afx_msg void OnRadioDateRange();
	afx_msg void OnChangeAuditDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeAuditDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPurge();
	afx_msg void OnAdvancedOptions(); // (a.walling 2009-12-18 10:46) - PLID 36626
	afx_msg void OnCopyOutputBtn(); // (b.cardillo 2010-01-07 13:26) - PLID 35780
	afx_msg void OnAuditMiscRadio();
	afx_msg void OnAuditPalmRadio();
	afx_msg void OnAuditInsuranceRadio();
	afx_msg void OnRButtonDownAuditList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnAuditInvRadio();
	afx_msg void OnAuditContactsRadio();
	afx_msg void OnAuditingRefresh();
	afx_msg void OnAuditingFilterToday();
	afx_msg void OnAuditEmrRadio();
	afx_msg void OnAuditBackupRadio();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void RButtonDownAuditList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RequeryFinishedAuditList(short nFlags);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBnClickedBtnAuditValidation();
};

// (c.haag 2010-09-07 17:32) - PLID 40198 - This class is used to enable us to fill a datalist
// column with hard-coded values as the list is visibly populated real-time
class CAuditing_JITCellValue : public NXDATALIST2Lib::IJustInTimeCellValue
{
public:
	CAuditing_JITCellValue(class CAuditing* pDlgOwner)
	{
		m_pDlgOwner = pDlgOwner;
		m_nRefCount = 1;
	}
	~CAuditing_JITCellValue()
	{
	}

public:
	CAuditing* m_pDlgOwner; // The owner dialog (in this case, CAuditing)
	LONG m_nRefCount; // Reference count

public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		if (riid == IID_IUnknown || riid == __uuidof(NXDATALIST2Lib::IJustInTimeCellValue)) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		} else {
			return E_NOINTERFACE;
		}
	}
    virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return InterlockedIncrement(&m_nRefCount); 
	}
    virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		if (m_nRefCount == 0) {
			return 0;
		}
		LONG lResult = InterlockedDecrement(&m_nRefCount);
		if (lResult == 0) {
			delete this;
		}
		return lResult;
	}

	// Our IJustInTimeCellValue interface
    virtual HRESULT __stdcall raw_GetValue(VARIANT * pvValue, NXDATALIST2Lib::EJITGetValueReason egvrReason, NXDATALIST2Lib::IRowSettings * pRow, NXDATALIST2Lib::IColumnSettings * pCol);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDITING_H__1B350021_D5A7_11D4_A760_0001024317D6__INCLUDED_)
