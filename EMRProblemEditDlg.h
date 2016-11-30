#if !defined(AFX_EMRPROBLEMEDITDLG_H__BE64B009_BD9B_4D8B_844D_DA08C79A9AF2__INCLUDED_)
#define AFX_EMRPROBLEMEDITDLG_H__BE64B009_BD9B_4D8B_844D_DA08C79A9AF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRProblemEditDlg.h : header file
//

class CDiagCode;

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemEditDlg dialog

// (j.jones 2009-05-22 10:02) - PLID 34250 - track info. for all linked items
struct LinkedObjectInfo {

	long nEMRProblemLinkID;
	CEmrProblemLink *pEmrProblemLink;	//optional, nullable
	EMRProblemRegardingTypes eType;
	CString strName;
	CString strValue;
	long nRegardingID;
	long nEMRDataID;
	BOOL bDeleted;
};

class CEMRProblemEditDlg : public CNxDialog
{
// Construction
public:
	CEMRProblemEditDlg(CWnd* pParent);   // standard constructor
	// (j.jones 2009-05-22 10:05) - PLID 34250 - for memory cleanup
	~CEMRProblemEditDlg();

// Dialog Data
	//{{AFX_DATA(CEMRProblemEditDlg)
	enum { IDD = IDD_EMR_PROBLEM_EDIT_DLG };
	CString	m_strCtrlDesc;
	CNxEdit	m_nxeditEditProblemDescription;
	CNxIconButton m_btnSave;
	CNxIconButton m_btnCancel;
	// (j.jones 2013-11-05 14:49) - PLID 58982 - added patient education
	CNxIconButton m_btnPatientEducation;
	CNxLabel m_nxlabelPatientEducation;
	// (s.tullis 2015-02-23 15:15) - PLID 64723 - Problem Checkbox: Add a checkbox at the top left of the EMR problem below onset date for “Do not show on CCDA”. This box is defaulted unchecked.
	NxButton m_checkDoNotShowOnCCDA;
	// (r.gonet 2015-03-06 09:37) - PLID 65008 - Added the checkbox control for Do not show on problem prompt.
	NxButton m_checkDoNotShowOnProblemPrompt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRProblemEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

protected:
	BOOL IsDateTimeValid(const COleDateTime& dt) const;

public:
	// (c.haag 2008-07-23 10:13) - PLID 30727 - Assigns the type and description of the source
	// object that owns this problem
	// (j.jones 2008-07-25 10:18) - PLID 30841 - nOwnerID and nEMRDataID are required for saving new problems
	// (j.jones 2009-05-21 16:42) - PLID 34250 - renamed this function in lieu of two new ones
	//void SetSourceInfo(EMRProblemRegardingTypes type, const CString& strDescription, long nOwnerID, long nEMRDataID = -1);
	
	// (j.jones 2009-05-21 16:42) - PLID 34250 - AddLinkedObjectInfo adds direct info. regarding a problem link,
	// can be called multiple times
	// pEmrProblemLink is NULL if a memory object is not available
	void AddLinkedObjectInfo(long nEMRProblemLinkID, EMRProblemRegardingTypes eType, const CString& strName, const CString& strValue, long nRegardingID, CEmrProblemLink *pEmrProblemLink = NULL, long nEMRDataID = -1);
	// (j.jones 2009-05-21 16:42) - PLID 34250 - GenerateLinkedObjectInfo will crunch data from a passed-in array of problem links,
	// and call AddLinkedObjectInfo for each problem link
	// Note: not currently used, it just so happens that all places that call this dialog can more efficiently use AddLinkedObjectInfo
	//void GenerateLinkedObjectInfo(CArray<CEmrProblemLink*, CEmrProblemLink*> &apEmrProblemLinks);

	void SetProblemID(long nProblemID);
	// (j.jones 2008-07-21 09:48) - PLID 30779 - added GetProblemID
	long GetProblemID();
	void SetProblemDesc(const CString& strDescription);
	CString GetProblemDesc();
	void SetProblemStatusID(long nStatusID);
	long GetProblemStatusID();
	void SetPatientID(long nPatientID);
	// (c.haag 2008-07-23 09:44) - PLID 30727 - Added support for the problem onset date
	COleDateTime GetOnsetDate();
	void SetOnsetDate(const COleDateTime& vOnsetDate);
	void SetSNOMEDCodeID(const long nSNOMEDCodeID); // (b.spivey September 24, 2013) - PLID 58677 
	
	// (a.walling 2009-05-01 17:56) - PLID 33751
	long GetProblemChronicityID() {
		return m_nChronicityID;
	};
	void SetProblemChronicityID(long nChronicityID) {
		m_nChronicityID = nChronicityID;
	}
	// (b.spivey, October 22, 2013) - PLID 58677 - refactored to work in EMR. 
	long GetProblemCodeID() {
		return m_nCodeID;
	};
	void SetProblemCodeID(long nCodeID) {
		m_nCodeID = nCodeID;
	}

	// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	void GetProblemDiagCodeIDs(long &nDiagICD9CodeID, long &nDiagICD10CodeID)
	{
		nDiagICD9CodeID = m_nDiagICD9CodeID;
		nDiagICD10CodeID = m_nDiagICD10CodeID;
	}
	void SetProblemDiagCodeIDs(long nDiagICD9CodeID, long nDiagICD10CodeID)
	{
		m_nDiagICD9CodeID = nDiagICD9CodeID;
		m_nDiagICD10CodeID = nDiagICD10CodeID;
	}

	// (s.tullis 2015-02-23 15:36) - PLID 64723 
	BOOL GetProblemDoNotShowOnCCDA()
	{
		return m_bDoNotShowOnCCDA;
	}

	// (s.tullis 2015-02-23 15:36) - PLID 64723 
	void SetProblemDoNotShowOnCCDA(BOOL bDoNotShowOnCCDA)
	{
		m_bDoNotShowOnCCDA=	bDoNotShowOnCCDA;
	}

	// (r.gonet 2015-03-06 10:05) - PLID 65008 - Sets the EMR Problem's Do Not Show On Problem Prompt flag.
	void SetProblemDoNotShowOnProblemPrompt(BOOL bDoNotShowOnProblemPrompt);
	// (r.gonet 2015-03-06 10:05) - PLID 65008 - Gets the EMR Problem's Do Not Show On Problem Prompt flag.
	BOOL GetProblemDoNotShowOnProblemPrompt() const;
public:
	// (c.haag 2006-11-13 13:38) - PLID 22052 - Set to true if you want to save the
	// changes to data in this dialog (default is off)
	void SetWriteToData(BOOL);

	// (a.walling 2008-06-12 10:22) - PLID 23138 - Set to true to be readonly
	void SetReadOnly(BOOL bReadOnly) {
		m_bReadOnly = bReadOnly;
	};

public:
	BOOL ProblemWasDeleted() const;
	BOOL IsModified() const;

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlStatus;
	NXDATALIST2Lib::_DNxDataListPtr m_dlHistory;

	// (b.spivey September 24, 2013) - PLID 58677 - list for snomed codes
	NXDATALIST2Lib::_DNxDataListPtr m_dlSNOMEDList; 

	// (j.jones 2014-02-24 11:14) - PLID 60781 - diagnosis codes now have
	// a search list, and then a list of the selected codes, maximum of one row
	NXDATALIST2Lib::_DNxDataListPtr m_DiagSearchList,
									m_DiagCodeList;

	// (a.walling 2009-05-01 17:56) - PLID 33751
	NXDATALIST2Lib::_DNxDataListPtr m_dlChronicity;

	// (j.jones 2009-05-21 17:53) - PLID 34250 - added list of linked objects
	NXDATALIST2Lib::_DNxDataListPtr m_dlLinkedObjectsList;

protected:
	CNxColor m_nxcTop;

protected:
	// (c.haag 2008-07-23 10:14) - PLID 30727 - Instead of a detail name, we now
	// track the type of owning EMR object, and a description for it
	//EMRProblemRegardingTypes m_OwnerType;
	//CString m_strOwnerDescription;
	// (j.jones 2008-07-25 10:18) - PLID 30841 - required for saving new problems
	//long m_nOwnerID;
	//long m_nEMRDataID;

public:
	// (j.jones 2009-05-21 16:42) - PLID 34250 - now we track an array of problem link information
	CArray<LinkedObjectInfo*, LinkedObjectInfo*> m_arypLinkedObjects;

protected:
	// (c.haag 2008-07-23 10:45) - PLID 30727 - Onset date
	NXTIMELib::_DNxTimePtr	m_nxtOnsetDate;

protected:
	// (c.haag 2006-11-13 13:38) - PLID 22052 - Set to true to make this dialog
	// write to data (default is off). This must be false if m_nProblemID <> -1
	BOOL m_bWriteToData;

	// (a.walling 2008-06-12 10:22) - PLID 23138 - Set to true to be readonly
	BOOL m_bReadOnly;

protected:
	long m_nProblemID;
	long m_nStatusID;
	long m_nPatientID;
	// (a.walling 2009-05-01 17:56) - PLID 33751
	long m_nChronicityID;

	// (j.jones 2014-02-24 11:49) - PLID 60781 - EMR problems now have
	// an ICD-9 code and/or an ICD-10 code, no more than one pair.
	long m_nDiagICD9CodeID;
	long m_nDiagICD10CodeID;
	// (s.tullis 2015-02-23 15:36) - PLID 64723 
	BOOL m_bDoNotShowOnCCDA;
	// (r.gonet 2015-03-06 09:58) - PLID 65008 - Added a member variable to hold the value of EMRProblemsT.DoNotShowOnProblemPrompt. Controlled by the Do Not Show On Problem Prompt checkbox.
	BOOL m_bDoNotShowOnProblemPrompt;

	// (j.jones 2014-02-24 12:32) - PLID 60781 - refreshes the datalist display
	// to show the codes for the current member variables
	void ReflectCurrentDiagnosisCodes();

	// (j.jones 2014-02-24 12:32) - PLID 60781 - resizes ICD-9 or 10 columns to
	// show/hide based on the search preference and current content
	void UpdateDiagnosisListColumnSizes();

	CString m_strProblemDesc;
	BOOL m_bProblemWasDeleted;
	BOOL m_bModified;
	COleDateTime m_dtOnsetDate; // (c.haag 2008-07-23 09:44) - PLID 30727

	// (b.spivey September 24, 2013) - PLID 58677
	long m_nCodeID;

	// (a.walling 2009-05-01 17:40) - PLID 28495 - SelChanged handler for the diag code
	// (a.walling 2009-05-01 17:40) - PLID 33751 - SelChanged handler for the chronicity, and edit list button
	// Generated message map functions
	//{{AFX_MSG(CEMRProblemEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedListProblemStatus(short nFlags);
	afx_msg void OnSelChangedListProblemStatus(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBtnEditStatusList();
	afx_msg void OnButtonSave();
	afx_msg void OnKillFocusDateOnset();
	afx_msg void OnBnClickedBtnEditChronicityList();
	void SelChangedListProblemChronicity(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	// (a.walling 2008-07-28 15:21) - PLID 30855 - This can fail gracefully if write access is not available
	BOOL WriteToData();

	// (j.jones 2014-03-06 15:46) - PLID 60781 - standalone function for updating diagnosis columns in the history
	void UpdateHistoryListDiagnosisColumns();

protected:
	void OnOK();
	void RequeryFinishedListProblemDiag(short nFlags);
	void RequeryFinishedListProblemChronicity(short nFlags);
	// (j.jones 2009-05-22 13:30) - PLID 34250 - supported a right click menu in the list of linked objects
	void OnRButtonDownEmrProblemLinkedObjectsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (b.spivey September 24, 2013) - PLID 58677 - event handlers for the interface. 
	void SelChangedListProblemSNOMED(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangingListProblemSNOMED(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RequeryFinishedListProblemSNOMEDCode(short nFlags);
	// (b.spivey, October 21, 2013) - PLID 58677 - message handler for the new button. 
	void OnBnClickedOpenUmls();
	// (j.jones 2013-11-05 14:49) - PLID 58982 - added patient education
	afx_msg void OnBtnPtEducation();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	// (j.jones 2014-02-24 12:27) - PLID 60781 - added diagnosis search
	void OnSelChosenProblemDiagSearchList(LPDISPATCH lpRow);
	void OnRButtonDownProblemDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2014-03-06 15:46) - PLID 60781 - resize the diag columns
	void OnRequeryFinishedListEmrProblemHistory(short nFlags);
public:
	afx_msg void OnBnClickedCheckNoshowonccda();
	// (r.gonet 2015-03-09 18:21) - PLID 65008 - Handles the event when the Do Not Show On Problem Prompt checkbox is checked.
	afx_msg void CEMRProblemEditDlg::OnBnClickedCheckDoNotShowOnProblemPrompt();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPROBLEMEDITDLG_H__BE64B009_BD9B_4D8B_844D_DA08C79A9AF2__INCLUDED_)
