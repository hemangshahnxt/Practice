#if !defined(AFX_SCHEDULERDURATIONDLG_H__08B39391_8E9D_4CC7_8C79_113B9C656557__INCLUDED_)
#define AFX_SCHEDULERDURATIONDLG_H__08B39391_8E9D_4CC7_8C79_113B9C656557__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SchedulerDurationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSchedulerDurationDlg dialog

class CSchedulerDurationDlg : public CNxDialog
{
// Construction
public:
	CSchedulerDurationDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSchedulerDurationDlg)
	enum { IDD = IDD_SCHEDULER_PROVIDERAPTDURATIONS };
	NxButton	m_btnAddDefaultDurations;
	NxButton	m_btnComboPurpose;
	NxButton	m_btnIndivPurpose;
	CNxStatic	m_nxstaticMultiprocnotice;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAddProcCombination;
	CNxIconButton	m_btnDeleteProcCombination;
	CNxIconButton	m_btnRemoveDurations;
	CNxIconButton	m_btnChangeDurations;
	CNxIconButton	m_btnCopyDurations;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSchedulerDurationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2004-06-22 08:59) - This is used to store values
	// the user may have changed while navigating in the dialog.
	// Every time the provider, type or a purpose has changed,
	// we call RememberDurations. In the end, we call SaveDurations
	// to write all of our memorized changes to data.
	class CDurationSet
	{
	public:
		long m_nSetID;
		long m_nProviderID;
		long m_nAptTypeID;
		CDWordArray m_adwPurposeIDs;
		BOOL m_bEnforced;

		long m_nDefaultLength;
		long m_nMinimumLength;

		//TES 3/29/2010 - PLID 37893
		BOOL operator ==(const CDurationSet &s) {
			if(m_nProviderID == s.m_nProviderID && m_nAptTypeID == s.m_nAptTypeID && ArraysMatch(m_adwPurposeIDs, s.m_adwPurposeIDs)
				&& m_nDefaultLength == s.m_nDefaultLength && m_nMinimumLength == s.m_nMinimumLength) {
					return TRUE;
			}
			else {
				return FALSE;
			}
		}

		//TES 3/25/2010 - PLID 37893 - Initialize m_nSetID
		CDurationSet() {m_nSetID = -1; m_bEnforced = TRUE;};
		CDurationSet(const CDurationSet& s)
		{
			Copy(s);
		}
		~CDurationSet() {};
		void Copy(const CDurationSet& s)
		{
			m_nSetID = s.m_nSetID;
			m_nProviderID = s.m_nProviderID;
			m_nAptTypeID = s.m_nAptTypeID;
			m_adwPurposeIDs.RemoveAll();
			m_bEnforced = s.m_bEnforced;
			for (long i=0; i < s.m_adwPurposeIDs.GetSize(); i++)
				m_adwPurposeIDs.Add(s.m_adwPurposeIDs[i]);
			m_nDefaultLength = s.m_nDefaultLength;
			m_nMinimumLength = s.m_nMinimumLength;
		}
	};
	typedef enum { eIndividual, eCombinations } ECurMode;
	CArray<CDurationSet*, CDurationSet*> m_aDurations;
	//TES 3/29/2010 - PLID 37893 - Array used to try and diagnose any issues with the saving.
	CArray<CDurationSet*, CDurationSet*> m_aOriginalDurations;
	CMap<int, int, CString, CString> m_mapPurpName;

	//TES 3/25/2010 - PLID 37893 - Keep track of any changes that are made.
	class ChangedDuration {
	public:
		long nSetID;
		long nProviderID;
		long nAptTypeID;
		CDWordArray adwPurposeIDs;
		long nOldDefaultLength;
		long nNewDefaultLength;
		long nOldMinimumLength;
		long nNewMinimumLength;
		//TES 3/25/2010 - PLID 37893 - Initialize the "identifier" variables based on this CDurationSet
		ChangedDuration(CDurationSet s) {
			nSetID = s.m_nSetID;
			nProviderID = s.m_nProviderID;
			nAptTypeID = s.m_nAptTypeID;
			adwPurposeIDs.Append(s.m_adwPurposeIDs);
			//TES 3/25/2010 - PLID 37893 - We don't pull the lengths.
			nOldDefaultLength = nNewDefaultLength = nOldMinimumLength = nNewMinimumLength = -1;
		}
		ChangedDuration() {
			nSetID = nProviderID = nAptTypeID = nOldDefaultLength = nNewDefaultLength = nOldMinimumLength = nNewMinimumLength = -1;
		}
		void operator =(const ChangedDuration &cd) {
			nSetID = cd.nSetID;
			nProviderID = cd.nProviderID;
			nAptTypeID = cd.nAptTypeID;
			adwPurposeIDs.RemoveAll();
			adwPurposeIDs.Append(cd.adwPurposeIDs);
			nOldDefaultLength = cd.nOldDefaultLength;
			nNewDefaultLength = cd.nNewDefaultLength;
			nOldMinimumLength = cd.nOldMinimumLength;
			nNewMinimumLength = cd.nNewMinimumLength;
		}

		ChangedDuration(const ChangedDuration &cd) {
			*this = cd;
		}
	};
	//TES 3/25/2010 - PLID 37893 - All the changes that have been made.
	CArray<ChangedDuration,ChangedDuration&> m_aChangedDurations;
	//TES 3/25/2010 - PLID 37893 - Find a ChangedDuration that matches the given CDurationSet.  Note that this does NOT look at SetID, but
	// matches on the provider, type, and purposes.
	// (b.spivey - February 4th, 2014) - PLID 60379 - Added bFindAbsoluteDurationSet to find by ID instead of updating a possible duplicate.
	long FindChangedDuration(CDurationSet s, bool bFindAbsoluteDurationSet = false);

	NXDATALISTLib::_DNxDataListPtr m_dlProviders, m_dlTypes;
	NXDATALISTLib::_DNxDataListPtr m_dlPurposes, m_dlPurposeCombinations;
	BOOL m_bModified;

	void LoadDurations();
	void SaveDurations();
	void ReflectDurations();
	BOOL WarnOfDuplicates();

	long GetSelectedProviderID();
	long GetSelectedAptTypeID();
	// (b.spivey - February 4th, 2014) - PLID 60379 - Added bFindAbsoluteDurationSet to find by ID instead of updating a possible duplicate.
	long FindDurationSet(const CDurationSet& s, long nExclude = -1, bool bFindAbsoluteDurationSet = false);
	static BOOL ArraysMatch(const CDWordArray& a1, const CDWordArray& a2);

	void RequeryPurposes();

	ECurMode GetMode();

	void EnsureButtons();
	void EnsureList();

	// (b.spivey - February 4th, 2014) - PLID 60379 - Updates duration sets when reloading the list. 
	bool UpdateSingleDurationSet(CDurationSet* pds);

	void OnOK();

	// (c.haag 2008-09-02 10:38) - PLID 23981 - Copies a duration set to a provider
	void CopyDurationSet(CDurationSet* pSrc, long nDestProviderID);

	//TES 3/26/2010 - PLID 33555 - We need to not requery the purposes until both the provider and type have finished requerying
	bool m_bProvidersRequeried, m_bTypesRequeried;

	// Generated message map functions
	//{{AFX_MSG(CSchedulerDurationDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedDoctorCombo(short nFlags);
	afx_msg void OnRequeryFinishedTypeCombo(short nFlags);
	afx_msg void OnRequeryFinishedPurposeCombo(short nFlags);
	afx_msg void OnSelChosenDoctorCombo(long nNewSel);
	afx_msg void OnSelChosenTypeCombo(long nNewSel);
	afx_msg void OnDestroy();
	afx_msg void OnBtnChangedurations();
	afx_msg void OnSelSetMultipurposeDurationList(long nRow);
	afx_msg void OnBtnRemovedurations();
	afx_msg void OnEditingFinishedMultipurposeDurationList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRadioDoindividualpurposes();
	afx_msg void OnRadioDopurposecombinations();
	afx_msg void OnLeftClickMultipurposeDurationCombinationList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedMultipurposeDurationCombinationList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnBtnAddproccombination();
	afx_msg void OnBtnDeleteproccombination();
	afx_msg void OnSelSetMultipurposeDurationCombinationList(long nRow);
	afx_msg void OnAddDefaultDurations();
	afx_msg void OnCopyDurations();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULERDURATIONDLG_H__08B39391_8E9D_4CC7_8C79_113B9C656557__INCLUDED_)
