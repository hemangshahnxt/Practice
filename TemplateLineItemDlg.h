#if !defined(AFX_TEMPLATELINEITEMDLG_H__0FC55234_59F5_11D2_80D7_00104B2FE914__INCLUDED_)
#define AFX_TEMPLATELINEITEMDLG_H__0FC55234_59F5_11D2_80D7_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateLineItemDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;

#include "TemplateLineItemInfo.h"
/////////////////////////////////////////////////////////////////////////////
// CTemplateLineItemDlg dialog

class CTemplateLineItemDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2014-12-11 09:18) - PLID 64230 - Added editor type to constructor
	CTemplateLineItemDlg(ESchedulerTemplateEditorType eEditorType, CWnd* pParent);   // standard constructor
	~CTemplateLineItemDlg();

	//TES 6/19/2010 - PLID 39262 - Added a parameter for whether we're editing a Resource Availability template
	long ZoomLineItem(CTemplateLineItemInfo *pLineItem);
	// (z.manning 2014-12-11 12:24) - PLID 64230
	int ZoomCollectionApply(CTemplateLineItemInfo *pLineItem);

	void ApplyChanges(BOOL bForceRefresh = TRUE);
	void ShowScaleChanges();
	void ShowPeriodChanges();
	CString GetScaleWord(bool bNormalCase = true, bool bSingular = true);
	void ShowIncludeChanges();
	void ShowFrame(int nFrameID, BOOL bShow);
	void DisplayResults();
	CString GetDayNumberText(bool bNormalCase = true);
	void EnsureDialogData(BOOL bFullEnsure = FALSE);
	void SetDayCheck(int nDayIndex, int nCheckVal = 1);

	// (c.haag 2006-11-13 09:45) - PLID 23336 - These have been moved to TemplateLineItemInfo.h
	//CString GetPreTimeRangeText(bool &bNormalCase);
	//CString GetPreIncludeText(bool &bNormalCase);
	//CString GetPeriodText(bool &bNormalCase);
	//CString GetPostIncludeText(bool &bNormalCase);
	//CString GetPostTimeRangeText(bool &bNormalCase);
	//CString GetDateRangeText(bool &bNormalCase /* = true */);
	//CString GetDaysText(UINT nCheckVal);
	//CString GetDaysTextEx(UINT& nIncluded);

	void GetDays(int &nDays, int &nDayCnt, UINT nCheckVal = 1);
	void GetDaysEx(int &nDays, int &nDayCnt, UINT& nIncluded);

	void LoadCurrentState();
	bool Validate();
	void WriteContentToLineItem(CTemplateLineItemInfo *pLineItem);


	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data	
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CTemplateLineItemDlg)
	enum { IDD = IDD_TEMPLATE_LINE_ITEM_DLG };
	NxButton	m_btnEvery1;
	NxButton	m_btnEvery2;
	NxButton	m_btnEveryX;
	NxButton	m_btnSelectResources;
	NxButton	m_btnShowAsPrecisionTemplate;
	NxButton	m_btnAllResources;
	NxButton	m_btnWednesday;
	NxButton	m_btnTuesday;
	NxButton	m_btnThursday;
	NxButton	m_btnSunday;
	NxButton	m_btnSaturday;
	NxButton	m_btnMonday;
	NxButton	m_btnFriday;
	NxButton	m_btnToDate;
	CString	m_strResults;
	CDateTimePicker	m_dtpStartDate;
	CDateTimePicker	m_dtpEndDate;
	CNxEdit	m_nxeditEveryXText;
	CNxEdit	m_nxeditDayNumberEdit;
	CNxStatic	m_nxstaticEveryXStatic;
	CNxStatic	m_nxstaticResultsLabel;
	CNxStatic	m_nxstaticByLabel;
	CNxStatic	m_nxstaticDayNumberLabel;
	CNxStatic	m_nxstaticForever;
	CNxIconButton	m_btnAddLineItemException;
	CNxIconButton	m_btnEditLineItemException;
	CNxIconButton	m_btnRemoveLineItemException;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnTimeGroupbox;
	NxButton	m_btnPeriodFrame;
	NxButton	m_btnByDateFrame;
	NxButton	m_btnResultsGroupbox;
	NxButton	m_btnAppliesGroupbox;
	NxButton	m_btnIncludeFrame;
	NxButton	m_btnAppearanceGroupbox;
	NxButton	m_btnDatesGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateLineItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_bGotDialogData;
	bool m_bReady;
	int m_nPeriod;
	EScale m_esScale;
	EMonthBy m_embBy;
	int m_nPatternOrdinal;
	int m_nDayNumber;
	int m_nInclude;

	// (z.manning 2014-12-11 09:18) - PLID 64230
	ESchedulerTemplateEditorType m_eEditorType;

	// (c.haag 2006-11-03 10:52) - PLID 23336 - Template line items no longer have
	// pivot dates. The field is superceded by m_dtStartDate.
	//COleDateTime m_dtPivotDate;

	// (c.haag 2006-11-02 16:28) - PLID 23336 - The start and end date fields have moved from
	// the template scope to the template line item scope
	COleDateTime m_dtStartDate;
	COleDateTime m_dtEndDate;

	CTemplateLineItemInfo *m_pLineItem;
	// (z.manning 2015-01-06 09:25) - PLID 64521 - Keep track of the original line item
	CTemplateLineItemInfo *m_pOriginalLineItem;

	// (b.cardillo 2005-10-14 16:50) - PLID 17954 - Use the COleDateTime directly rather than 
	// converting back and forth to CString, so as to avoid any problems from regional settings.
	COleDateTime m_dtStartTime;
	COleDateTime m_dtEndTime;

	NXTIMELib::_DNxTimePtr m_nxtEnd;
	NXTIMELib::_DNxTimePtr m_nxtStart;

	const CRect m_rectApplyDateLabel;
	const CRect m_rectApplyDate;

	NXDATALIST2Lib::_DNxDataListPtr m_dlResources;
	NXDATALIST2Lib::_DNxDataListPtr m_dlExceptionDates;
	// (z.manning 2014-12-11 11:31) - PLID 64230 - We only support one resource for collection applies
	// so we have a 2nd resource datalist that is a combo box.
	NXDATALIST2Lib::_DNxDataListPtr m_pdlApplyResource;
	enum ApplyResourceColumns {
		arcID = 0,
		arcName,
		arcInactive,
	};

protected:
	void ReflectCurrentStateOnBtns();

protected:
	_variant_t ToDateVariant(const COleDateTime&);

protected:
	// Generated message map functions
	//{{AFX_MSG(CTemplateLineItemDlg)
	virtual BOOL OnInitDialog();
	virtual void OnDestroy();
	afx_msg void OnSelchangeScaleCombo();
	afx_msg void OnEveryXRadio();
	afx_msg void OnEvery2Radio();
	afx_msg void OnEvery1Radio();
	afx_msg void OnSelchangeByCombo();
	afx_msg void OnMondayCheck();
	afx_msg void OnTuesdayCheck();
	afx_msg void OnWednesdayCheck();
	afx_msg void OnThursdayCheck();
	afx_msg void OnSundayCheck();
	afx_msg void OnFridayCheck();
	afx_msg void OnSaturdayCheck();
	afx_msg void OnChangeDayNumberEdit();
	afx_msg void OnSelchangeStartTimeCombo();
	afx_msg void OnSelchangeEndTimeCombo();
	virtual void OnOK();
	afx_msg void OnSelchangePatternOrdinalCombo();
	afx_msg void OnChangeEveryXText();
	/*afx_msg void OnChangeStartDate();*/
	/*afx_msg void OnChangeEndDate();*/
	afx_msg void OnKillFocusStartTime();
	afx_msg void OnKillFocusEndTime();
	afx_msg void OnFromDateCheck();
	afx_msg void OnToDateCheck();
	afx_msg void OnAllResourcesBtn();
	afx_msg void OnSelectResourcesBtn();
	afx_msg void OnRequeryFinishedResourceList(short nFlags);
	afx_msg void OnRequeryFinishedResourceDropdown(short nFlags);
	afx_msg void OnEditingFinishedResourceList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAddLineItemException();
	afx_msg void OnEditLineItemException();
	afx_msg void OnRemoveLineItemException();
	afx_msg void OnSelchangedExceptionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellExceptionList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChosenResourceDropdown(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckBlockAppts();
	// (d.singleton 2011-10-13 13:50) - PLID 45945 added these so the OnChangeStartDate and OnChangeEndDate actually fire
	afx_msg void OnChangeStartDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnChangeEndDate(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATELINEITEMDLG_H__0FC55234_59F5_11D2_80D7_00104B2FE914__INCLUDED_)
