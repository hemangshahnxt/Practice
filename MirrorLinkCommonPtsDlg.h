#if !defined(AFX_MirrorLinkCommonPtsDlg_H__1E1825DD_46A4_487B_B4DE_EC230F852AA5__INCLUDED_)
#define AFX_MirrorLinkCommonPtsDlg_H__1E1825DD_46A4_487B_B4DE_EC230F852AA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MirrorLinkCommonPtsDlg.h : header file
//
// (c.haag 2008-02-06 09:57) - PLID 28622 - This class allows the user to
// determine which patients can be linked between both softwares, and then
// link them together.
//
// For inputs, we are given two other datalists; one with Practice fields,
// and one with Mirror data fields. The fields include: Last/First name,
// Middle Name, SSN, Home Phone, Birthdate, Address 1, Address 2, City, State,
// Zip. We also get the full name and the chart number in each respective system
// for display purposes.
//

/////////////////////////////////////////////////////////////////////////////
// CMirrorLinkCommonPtsDlg dialog

class CMirrorLinkCommonPtsDlg : public CNxDialog
{
// Construction
public:
	CMirrorLinkCommonPtsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMirrorLinkCommonPtsDlg)
	enum { IDD = IDD_MIRROR_LINK_COMMON_PTS };
	NxButton	m_btnUseMiddleName;
	NxButton	m_btnUseSSN;
	NxButton	m_btnUseHPhone;
	NxButton	m_btnUseBDate;
	NxButton	m_btnUseAddr1;
	NxButton	m_btnUseAddr2;
	NxButton	m_btnUseCity;
	NxButton	m_btnUseState;
	NxButton	m_btnUseZip;
	NxButton	m_btnShowSSN;
	NxButton	m_btnShowHPhone;
	NxButton	m_btnShowBDate;
	NxButton	m_btnPatIDToSSN;
	NxButton	m_btnPatIDToMRN;
	CProgressCtrl	m_ProgressRequery;
	BOOL	m_bUseMiddleName;
	BOOL	m_bUseAddress1;
	BOOL	m_bUseAddress2;
	BOOL	m_bUseBirthdate;
	BOOL	m_bUseCity;
	BOOL	m_bUseHomePhone;
	BOOL	m_bUseSSN;
	BOOL	m_bUseState;
	BOOL	m_bUseZip;
	BOOL	m_bLinkPatIDToMRN;
	BOOL	m_bLinkPatIDToSSN;
	NxButton	m_btnLinkRemotePtsFilter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorLinkCommonPtsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
	// Input lists given to this object by the caller
	NXDATALISTLib::_DNxDataListPtr m_dlInput_Practice;
	NXDATALISTLib::_DNxDataListPtr m_dlInput_Remote;

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dl2Qualify;
	NXDATALIST2Lib::_DNxDataListPtr m_dl2DisqualifyPractice;
	NXDATALIST2Lib::_DNxDataListPtr m_dl2DisqualifyRemote;

private:
	CNxIconButton	m_btnStop;
	CNxIconButton	m_btnCalculate;
	CNxIconButton	m_btnLink;
	CNxIconButton	m_btnClose;

private:
	// (c.haag 2008-02-13 12:44) - Before we start linking patients, we retain
	// the initial size of the qualify list for progress display purposes
	long m_nInitialQualifyListSize;

private:
	// (c.haag 2008-01-15 16:23) - When requerying or linking, this is the counter
	// that tells us what patient to populate next
	long m_nProgressIteration;

	// (c.haag 2008-01-31 10:45) - When linking patients between the two systems, we
	// do so on a timer. This is a pointer to the current row in the qualifying list
	// which we reference in OnTimer.
	NXDATALIST2Lib::IRowSettingsPtr m_pCurrentLinkRow;

private:
	// (c.haag 2008-02-04 10:04) - These variables track the widths of optionally
	// visible columns when they are invisible
	long m_nPracSSNColLen;
	long m_nPracHomePhoneColLen;
	long m_nPracBirthdateColLen;

	long m_nRemoteSSNColLen;
	long m_nRemoteHomePhoneColLen;
	long m_nRemoteBirthdateColLen;

private:
	// (c.haag 2008-02-04 10:30) - When calculating qualifying patients, patients
	// who are already linked are ignored. We need the user to know this, so we
	// keep a count of the number of linked patients
	long m_nExistingLinkedPts;

private:
	// (c.haag 2008-02-19 17:28) - When scanning for patients who qualify for linking,
	// any improperly linked patient ordinals in the m_dlInput_Practice list are stored
	// in this array.
	CArray<long,long> m_anImproperlyLinkedPracticeOrdinals;

private:
	// TRUE if we actually changed any data
	BOOL m_bChangedData;

public:
	// Returns TRUE if any data was changed by this dialog
	BOOL GetChangedData() const { return m_bChangedData; }

public:
	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function invokes the
	// dialog.
	int Open(NXDATALISTLib::_DNxDataListPtr dlPractice,
		NXDATALISTLib::_DNxDataListPtr dlRemote);

private:
	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function causes all
	// the lists on the form to be requeried based on the current linking
	// criteria.
	void RequeryAll();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function is called when
	// we want to reset the progress bar and m_nProgressIteration
	void ResetProgressCount();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function is called when
	// a timed action is either completed or interrupted
	void StopTimedAction();

private:
	// (c.haag 2008-02-06 09:57) - PLID 28622 - Assigns the width of the
	// SSN columns based on the state of the "Show SSN" checkbox
	void UpdateSSNColumnWidths();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - Assigns the width of the
	// SSN columns based on the state of the "Show Home Phone" checkbox
	void UpdateHomePhoneColumnWidths();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - Assigns the width of the
	// SSN columns based on the state of the "Show Birthdate" checkbox
	void UpdateBirthdateColumnWidths();

private:
	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function encapsulates the
	// global variant compare function
	HRESULT VariantCompare(_variant_t v1, _variant_t v2);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function takes in
	// a variant that should contain an SSN, and tries to format it.
	_variant_t CleanSSN(_variant_t v);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function compares two SSN's
	HRESULT CompareSSNs(_variant_t v1, _variant_t v2);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function takes in
	// a variant that should contain a phone number, and tries to format it
	_variant_t CleanPhoneNumber(_variant_t v);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function compares two
	// phone numbers
	HRESULT ComparePhones(_variant_t v1, _variant_t v2);

	// (c.haag 2008-03-11 11:15) - PLID 28622 - This function takes in
	// a variant that should contain a zip code, and ensures there are no
	// trailing dashes or spaces.
	_variant_t CleanZip(_variant_t v);

	// (c.haag 2008-03-11 11:13) - PLID 28622 - This function compares two
	// zip codes
	HRESULT CompareZips(_variant_t v1, _variant_t v2);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function is called if a record in
	// the Practice input list failed to qualify to link with another Mirror record. The
	// interface will be reflected to update this fact.
	NXDATALIST2Lib::IRowSettingsPtr FailQualifyPracticeRecord(long nOrdinal, LPCTSTR szReason, BOOL bAddSorted = TRUE);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function is called if a record in
	// the remote input list failed to qualify to link with another Practice record. The
	// interface will be reflected to update this fact.
	void FailQualifyMirrorRecord(long nOrdinal, LPCTSTR szReason);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function will examine the
	// record in m_dlInput_Practice for the given ordinal, and either add it to
	// the m_dl2Qualify list, or add it to m_dl2DisqualifyPractice based on
	// whether it can be matched up with exactly one remote record based on the
	// user criteria
	void TryQualifyPracticeRecord(long nOrdinal);

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function will examine the
	// record in m_dlInput_Remote for the given ordinal. This is done after we
	// test Practice records for qualification. Any Mirror records that could not
	// be carried over to the qualification list will be added to m_dl2DisqualifyRemote.
	void TryQualifyRemoteRecord(long nOrdinal);

private:
	// (c.haag 2008-02-06 09:57) - PLID 28622 - This begins the process of linking
	// qualifying patients (the ones in the top list)
	void OnBtnLinkQualifyingPatients();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function will dismiss this dialog
	void OnBtnClose();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This function will begin the process
	// of searching for patients in both systems who meet the criteria for being linked
	void OnBtnCalculateLinkPts();

	// (c.haag 2008-02-06 09:57) - PLID 28622 - This will halt any operation in progress
	void OnBtnStop();

private:
	void OnOK();
	void OnCancel();

protected:
	// Generated message map functions
	//{{AFX_MSG(CMirrorLinkCommonPtsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnSSNMRNHelp();
	afx_msg void OnCheckShowSSN();
	afx_msg void OnCheckShowHomePhone();
	afx_msg void OnCheckShowBirthdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MirrorLinkCommonPtsDlg_H__1E1825DD_46A4_487B_B4DE_EC230F852AA5__INCLUDED_)
