#if !defined(AFX_APTBOOKALARMLISTDLG_H__B9D9E8EB_4D27_4958_9914_0E7EE74D98A2__INCLUDED_)
#define AFX_APTBOOKALARMLISTDLG_H__B9D9E8EB_4D27_4958_9914_0E7EE74D98A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AptBookAlarmListDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmListDlg dialog

class CAptBookAlarmListDlg : public CNxDialog
{
// Construction
public:
	CAptBookAlarmListDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_AlarmList;

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CAptBookAlarmListDlg)
	enum { IDD = IDD_APT_BOOK_ALARM_LIST_DLG };
	CNxIconButton	m_btnAddAlarm;
	CNxIconButton	m_btnEditAlarm;
	CNxIconButton	m_btnRemoveAlarm;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAptBookAlarmListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void RefreshButtons();

	// Generated message map functions
	//{{AFX_MSG(CAptBookAlarmListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddAlarmBtn();
	afx_msg void OnEditAlarmBtn();
	afx_msg void OnRemoveAlarmBtn();
	afx_msg void OnDblClickCellAptBookingAlarmList(long nRowIndex, short nColIndex);
	afx_msg void OnSelChangedAptBookingAlarmList(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APTBOOKALARMLISTDLG_H__B9D9E8EB_4D27_4958_9914_0E7EE74D98A2__INCLUDED_)
