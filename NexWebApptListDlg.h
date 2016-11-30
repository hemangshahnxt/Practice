#if !defined(AFX_NEXWEBAPPTLISTDLG_H__B55B7C04_F93D_48FB_B67F_6EB79738654E__INCLUDED_)
#define AFX_NEXWEBAPPTLISTDLG_H__B55B7C04_F93D_48FB_B67F_6EB79738654E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebApptListDlg.h : header file
//

struct InsuranceInfo;
typedef CMap<long, long, InsuranceInfo*, InsuranceInfo*> AppointmentInsuranceMap;
struct SchedAuditItems;
class SchedulerMixRule;

/////////////////////////////////////////////////////////////////////////////
// CNexWebApptListDlg dialog

struct ApptImport {
	long nID;
	BOOL bSelected;
	COleDateTime dtApptDate;
	COleDateTime dtStartTime;
	COleDateTime dtEndTime;
	long nLocationID;
	bool bMoveUp;
	CString strNotes;
	long nPatientID;
	long nTypeID;
	long nStatus;
	CDWordArray dwResourceList;
	CDWordArray dwPurposeList;
	BOOL bHasBeenSaved;
	BOOL bIsNew;
	// (j.jones 2014-11-26 14:56) - PLID 64169 - added insurance placements
	AppointmentInsuranceMap mapInsPlacements;
	// (j.jones 2014-12-16 10:37) - PLID 64312 - if they overrode the appointment, we need to save
	// this information for when the appointment is saved
	std::vector<SchedulerMixRule> overriddenMixRules;
};

class CNexWebApptListDlg : public CNxDialog
{
// Construction
public:
	CNexWebApptListDlg(CWnd* pParent);   // standard constructor
	// (j.gruber 2007-02-23 11:22) - PLID 24722 - clean up memory leaks
	~CNexWebApptListDlg();
	void SetPersonID(long nPersonID, BOOL bIsNewPatient);
	long m_nPersonID;
	NXDATALISTLib::_DNxDataListPtr  m_pApptList;
	void LoadApptList();
	BOOL SaveInfo(long nPersonID = -1);
	BOOL CheckExistingApptData();
	BOOL ValidateData();
	BOOL m_bIsNewPatient;
	void GenerateApptList();
	CString GenerateResourceString(long nApptID);
	// (j.gruber 2006-11-13 14:39) - PLID 23154 - added Purpose function to save purposes right away
	void GeneratePurposeArray(long nApptID, CDWordArray *pdw, BOOL bIsNewAppt);
	void GenerateResourceArray(long nApptID, CDWordArray *pdw);
	CString m_strError;
	void GenerateAuditItem(SchedAuditItems &item, ApptImport *pAppt);
	void RefreshList(ApptImport *pAppt);

// Dialog Data
	//{{AFX_DATA(CNexWebApptListDlg)
	enum { IDD = IDD_NEXWEB_APPT_LIST_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebApptListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	CArray<ApptImport*, ApptImport*> m_ApptList;

	// Generated message map functions
	//{{AFX_MSG(CNexWebApptListDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellNexwebApptList(long nRowIndex, short nColIndex);
	afx_msg void OnLButtonDownNexwebApptList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedNexwebApptList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBAPPTLISTDLG_H__B55B7C04_F93D_48FB_B67F_6EB79738654E__INCLUDED_)
