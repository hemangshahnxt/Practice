#if !defined(AFX_WAITINGLISTMATCHES_H__EEDC7835_8CD5_4B93_81DE_FFA5F0B84AC2__INCLUDED_)
#define AFX_WAITINGLISTMATCHES_H__EEDC7835_8CD5_4B93_81DE_FFA5F0B84AC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaitingListMatches.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaitingListMatches dialog

// (d.moore 2007-07-12 10:36) - PLID 26546 - This dialog displays a set of matches
//  in the waitng list bases on the patient ID, resources selected, date, and time.

class CWaitingListMatches : public CNxDialog
{
// Construction
public:
	// Pass in the appointment ID and a list of waiting list ID values
	//  that may be matches to the appointment. This dialog doesn't
	//  actually find matches, but instead displays them in a clear way
	//  to allow the user to delete them from the list of edit them.
	CWaitingListMatches(long nAppointmentID, 
		const CArray<long, long> &arWaitingListIDs, 
		CWnd* pParent);   

// Dialog Data
	//{{AFX_DATA(CWaitingListMatches)
	enum { IDD = IDD_WAITING_LIST_MATCH_DLG };
	CNxIconButton	m_btnDeleteMatches;
	CNxIconButton	m_btnEditMatch;
	CNxIconButton	m_btnClose;
	CNxStatic	m_nxstaticWlMatchLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaitingListMatches)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Don't allow calling the default constructor.
	CWaitingListMatches(CWnd* pParent);

	long m_nAppointmentID;
	CArray<long, long> m_arWaitingListIDs;
	CString m_strCommaSeparatedIdList;

	long m_nMode; // Used to track the type of behavior for the dialog.
	enum m_eModes {
		emNoMatch,
		emAppointmentMatch, // Single entry that is linked to the m_nAppointmentID.
		emSingleMatch,
		emMultipleMatches
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pMatchList;

	// The following three functions just seperate out functionality for
	//  setting up the buttons and labels of the dialog.
	void ConfigureForNoMatch();
	void ConfigureForSingleMatch();
	void ConfigureForMultipleMatch();

	void QueryMatchList();
	void QueryLineItemCollection();
	void SetAppointmentColors();
	void RemoveFromWaitList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void RemoveFromWaitList(const CArray<NXDATALIST2Lib::IRowSettings*, NXDATALIST2Lib::IRowSettings*> &arIdList);

	// Generated message map functions
	//{{AFX_MSG(CWaitingListMatches)
	afx_msg void OnWlBtnDeleteMatches();
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedWaitingListMatches(short nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAITINGLISTMATCHES_H__EEDC7835_8CD5_4B93_81DE_FFA5F0B84AC2__INCLUDED_)
