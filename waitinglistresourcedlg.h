#if !defined(AFX_WAITINGLISTRESOURCEDLG_H__256FB0F7_B9AB_4D4D_86FF_67BCD8AF937A__INCLUDED_)
#define AFX_WAITINGLISTRESOURCEDLG_H__256FB0F7_B9AB_4D4D_86FF_67BCD8AF937A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaitingListResourceDlg.h : header file
//

#include "WaitingListUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CWaitingListResourceDlg dialog

// (d.moore 2007-05-23 11:31) - PLID 4013

class CWaitingListResourceDlg : public CNxDialog
{
// Construction
public:
	
	CWaitingListResourceDlg(CWnd* pParent);   // standard constructor

	~CWaitingListResourceDlg();

	bool m_bDataChanged; // Used to prevent updating the database until some change has been made.
	
	void SetLineItemData(const WaitListLineItem &wlItem);

	void GetLineItemData(WaitListLineItem &wlData);

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CWaitingListResourceDlg)
	enum { IDD = IDD_WAITING_LIST_RESOURCE_DLG };
	NxButton	m_btnSelectResources;
	NxButton	m_btnAllResources;
	NxButton	m_btnWednesday;
	NxButton	m_btnTuesday;
	NxButton	m_btnThursday;
	NxButton	m_btnSunday;
	NxButton	m_btnSaturday;
	NxButton	m_btnFriday;
	NxButton	m_btnMonday;
	CNxStatic	m_nxstaticResultsLabel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnDatesGroupbox;
	NxButton	m_btnTimeGroupbox;
	NxButton	m_btnIncludeFrame;
	NxButton	m_btnResourceGroupbox;
	NxButton	m_btnResults;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitingListResourceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	WaitListLineItem m_LineItem; // Used to store the state of the form.

	// (a.walling 2008-05-13 14:56) - PLID 27591 - Use CDateTimePicker
	CDateTimePicker m_dtcStartDate;
	CDateTimePicker m_dtcEndDate;

	NXTIMELib::_DNxTimePtr m_nxtStartTime;
	NXTIMELib::_DNxTimePtr m_nxtEndTime;

	NXDATALIST2Lib::_DNxDataListPtr m_pResources;

	// Sets control values using m_pLineItem.
	void LoadFormValues();
	
	// Sets the day checkboxes to match pItem->rgDayIDs.
	void SetDayCheckboxVals(const WaitListLineItem &wlItem);

	// Updates pItem->rgDayIDs to match the day checkboxes.
	void GetDayCheckboxVals(WaitListLineItem &wlItem);

	// Sets the resource checkboxes to match pItem->rgResourceIDs.
	void SetResourceListVals(const WaitListLineItem &wlItem);

	// Updates pItem->rgResourceIDs to match the resource checkboxes.
	void GetResourceListVals(WaitListLineItem &wlItem);


	// Updates m_pLineItem to match the current values for the controls.
	void RefreshLineItemData(WaitListLineItem &wlItem);

	// Updates the result area to match the state of the controls.
	void RefreshLineItemText(const WaitListLineItem &wlItem);

	// Returns false if dates or times are improperly entered.
	bool ValidateData(const WaitListLineItem &wlItem);

	// Generated message map functions
	// (a.walling 2008-05-13 14:58) - PLID 27591 - Use notify handlers for datetimepicker events
	//{{AFX_MSG(CWaitingListResourceDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeDateStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDateEnd(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMondayCheck();
	afx_msg void OnTuesdayCheck();
	afx_msg void OnWednesdayCheck();
	afx_msg void OnThursdayCheck();
	afx_msg void OnFridayCheck();
	afx_msg void OnSaturdayCheck();
	afx_msg void OnSundayCheck();
	afx_msg void OnRadioAllResources();
	afx_msg void OnRadioSelectResources();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedWlResources(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnChangedWlEndTime();
	afx_msg void OnChangedWlStartTime();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAITINGLISTRESOURCEDLG_H__256FB0F7_B9AB_4D4D_86FF_67BCD8AF937A__INCLUDED_)
