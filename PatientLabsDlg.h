#if !defined(AFX_PATIENTLABSDLG_H__C86FF85B_90C5_466D_B996_62C3DDA378F6__INCLUDED_)
#define AFX_PATIENTLABSDLG_H__C86FF85B_90C5_466D_B996_62C3DDA378F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "nxtwain.h"
#include "PatientsRc.h"

// PatientLabsDlg.h : header file
//

//TES 2/10/2010 - PLID 37296 - Moved a couple enums and a function to GlobalUtils.h

/////////////////////////////////////////////////////////////////////////////
// CPatientLabsDlg dialog

class CPatientLabsDlg : public CNxDialog
{
protected:
	// (c.haag 2010-09-15 12:20) - PLID 40512 - This class is used to ultimately have
	// this dialog not refresh in response to table checkers during list events.
	class CPatientLabsIgnoreTableChecker
	{
	private:
		CPatientLabsDlg* m_pDlg;
	public:
		CPatientLabsIgnoreTableChecker(CPatientLabsDlg* pDlg)
		{
			// Ignore table checkers
			m_pDlg = pDlg;
			if (m_pDlg) { 
				m_pDlg->m_nIgnoreTableCheckersRefCnt++;
				m_pDlg->m_bGotIgnoredTableChecker = false;
			}
		}
		//d.thompson 10/6/2015 - PLID 67289 - In vs2015, destructors are assumed not to throw.  Since this one does, we need to explicitly state it.
		~CPatientLabsIgnoreTableChecker() throw(...)
		{
			if (m_pDlg) {
				if (m_pDlg->m_nIgnoreTableCheckersRefCnt > 0) {
					m_pDlg->m_nIgnoreTableCheckersRefCnt--;  
				} else {
					// The reference count should always be positive here
					m_pDlg->m_nIgnoreTableCheckersRefCnt = 0;
					ThrowNxException("Failed to properly derefrence m_nIgnoreTableCheckersRefCnt!");
				}
				if (m_pDlg->m_bGotIgnoredTableChecker &&
					0 == m_pDlg->m_nIgnoreTableCheckersRefCnt) {
					// We can't refresh the list now; we still may be in a datalist event. Post a message
					// to update next chance we get.
					if (IsWindow(m_pDlg->GetSafeHwnd())) {
						m_pDlg->PostMessage(NXM_UPDATEVIEW);
					}
				}
			}
		}
	};

	// (r.gonet 09/03/2014) - PLID 63540 - Small utility class to encapsulate calls to table checkers and store a value identifying
	// the sender of a tablechecker message.
	class CTableRefresher {
	private:
		CString m_strSenderID;
	public:
		// (r.gonet 09/03/2014) - PLID 63540 - Construct the CTableRefresher
		CTableRefresher();
		// (r.gonet 09/03/2014) - PLID 63540 - Refreshes the labs table with the ability to tell recipients to ignore the message
		// if it was sent from a particular sender ID (used to ignore tablecheckers sent to the dialog owning this refresher).
		void RefreshLabsTable(long nPersonID, long nLabID, BOOL bIgnoreSelf = FALSE);
		// (r.gonet 09/03/2014) - PLID 63540 - Returns the sender ID, which identifies this table refresher object.
		CString GetSenderID();
	};


// Construction
public:
	CPatientLabsDlg(CWnd* pParent);   // standard constructor

	virtual void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (m.hancock 2006-06-27 10:11) - PLID 21070 - Returns the string value for the passed number.
	// 0 is ""{No Selection}, 1 is "Left", 2 is "Right"
	//TES 11/10/2009 - PLID 36128 - This is no longer an enum, but a configurable table, so the description should be loaded along with the ID.
	//TES 12/8/2009 - PLID 36470 - AnatomySide is back!
	CString GetAnatomySideString(long nSide);

	NXDATALIST2Lib::_DNxDataListPtr m_pLabsList;

	//bool m_bInPICContainer; // (r.galicki 2008-10-07 12:12) - PLID 31555
	// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
	//bool m_bInPICContainer;
	class CPicContainerDlg* GetPicContainer() const
	{
		return m_pPicContainer;
	}

	void SetPicContainer(class CPicContainerDlg* pPicContainer)
	{
		ASSERT(!m_pPicContainer || !pPicContainer);
		m_pPicContainer = pPicContainer;
	}

protected:
	class CPicContainerDlg* m_pPicContainer;
	CTableRefresher m_labsTableRefresher;

protected:
	// (z.manning 2011-07-08 15:10) - PLID 42746 - Added an enum for the lab radio button remembering preference
	// NOTE: Saved to data, do not change.
	enum ELabRadioPreference {
		lrpOpen = 1,
		lrpClosed = 2,
		lrpAll = 3,
	};

	// (r.gonet 09/02/2014) - PLID 63538 - Reload the labs datalist
	void ReloadLabList();
	// (r.gonet 09/02/2014) - PLID 63538 - Get the patient's labs, lab steps, and lab step related documents.
	ADODB::_RecordsetPtr GetLabRowsRecordset();
	// (r.gonet 09/02/2014) - PLID 63538 - Construct the where clause for the labs list.
	CSqlFragment GetLabsListWhereClause();
	// (r.gonet 09/02/2014) - PLID 63538 - Given fields for the lab, adds a lab row and spacer row to the labs list.
	NXDATALIST2Lib::IRowSettingsPtr AddLabRow(ADODB::FieldsPtr pFields);
	// (r.gonet 09/02/2014) - PLID 63538 - Given fields for the lab step and the parent lab row, adds a lab step row to the labs list.
	NXDATALIST2Lib::IRowSettingsPtr AddLabStepRow(ADODB::FieldsPtr pFields, NXDATALIST2Lib::IRowSettingsPtr pParentLabRow);
	// (r.gonet 09/02/2014) - PLID 63538 - Given fields for the mailsent record and the parent step row, adds a document row to the labs list.
	NXDATALIST2Lib::IRowSettingsPtr AddDocumentRow(ADODB::FieldsPtr pFields, NXDATALIST2Lib::IRowSettingsPtr pParentLabStepRow);
	

	// (c.haag 2010-01-19) - PLID 41825 - We no longer allow reopening labs based on step completions
	void ModifyStepCompletion(LPDISPATCH lpRow, BOOL bComplete);//, bool bReopenLab = false);
	void MarkEntireLabComplete(LPDISPATCH lpLabRow, LPCTSTR strLabIDs = NULL);
	// (c.haag 2010-01-19) - PLID 41825 -This function now "reopens" the lab by having the user select individual,
	// completed results to mark incomplete. Returns FALSE if the user cancelled the operation. Returns TRUE if they
	// did not, which means they must have marked at least one result as incomplete. We also no longer accept an
// array of labs; this only operates one lab at a time.
	BOOL ReopenLab(LPDISPATCH lpLabRow);
	bool IsLabComplete(long nLabID);
	COLORREF CalculateLabColor(bool bComplete, bool bStepRow);
	// (c.haag 2010-01-19) - PLID 41825 - Return FALSE if the operation was cancelled or failed
	BOOL MarkLabIncomplete(LPDISPATCH lpLabRow);
	
	// (c.haag 2010-05-05 17:36) - PLID 36096 - Utility function for acquiring documents from a TWAIN device
	void DoTWAINAcquisition(NXTWAINlib::EScanTargetFormat eScanTargetFormat);

	// (m.hancock 2006-06-23 15:24) - PLID 21196 - set bDetachHistory to true
// to also detach from the patient's history, and set bDelete to true to also delete the file.
	void DetachFile(bool bDetachHistory, bool bDelete);

	// (m.hancock 2006-06-28 11:28) - PLID 21205 - call this funtion to enable or disable the appropriate buttons on screen
	void EnableAppropriateButtons();

	int GetPicID();
	long GetPatientID();
	CString GetPatientName();

	// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
	static void WINAPI CALLBACK OnNxTwainPreCompress(const LPBITMAPINFO pDib, BOOL &bAttach, BOOL &bAttachChecksum, long &nChecksum, ADODB::_Connection* lpCon);

	// (z.manning 2009-05-28 15:59) - PLID 34340 - Problem icons
	HICON m_hIconHasProblemFlag;
	HICON m_hIconHadProblemFlag;

	// (c.haag 2010-09-15 12:20) - PLID 40512 - Set to non-zero when we want to ignore table checkers
	int m_nIgnoreTableCheckersRefCnt;
	// (c.haag 2010-09-15 12:20) - PLID 40512 - Set to true if we got a table checker while we were ignoring them
	bool m_bGotIgnoredTableChecker;

	// (r.gonet 09/02/2014) - PLID 63539 - Whether we are waiting for more tablecheckers before reloading the labs list.
	bool m_bCoalescingTableCheckers;

	//TES 8/2/2011 - PLID 44814 - Moved code to get the default category for lab attachments into this function, so it can also
	// check the permissions for category
	long GetDefaultAttachmentCategory();
public:

// Dialog Data
	// (j.gruber 2009-05-07 11:21) - PLID 28556 - created result graph preview button
	// (c.haag 2010-09-09 13:46) - PLID 40461 - Added checkbox for discontinued labs
	//{{AFX_DATA(CPatientLabsDlg)
	enum { IDD = IDD_PATIENT_LABS };
	CNxIconButton	m_newLab;
	CNxIconButton	m_newAttachment;
	CNxIconButton	m_labsNeedingAttention;
	CNxIconButton	m_detach;
	CNxIconButton	m_deleteLab;
	CNxIconButton m_btnResultGraphPreview;
	CNxColor	m_bkg;
	NxButton		m_openLabsRad;
	NxButton		m_closedLabsRad;
	NxButton		m_allLabsRad;
	NxButton		m_checkFilterOnPIC;
	NxButton		m_checkDiscontinuedLabs;
	CNxIconButton m_btnShowAllResults;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPatientLabsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.walling 2010-10-14 17:04) - PLID 40978
	long m_id;

	// (r.galicki 2008-10-08 13:17) - PLID 31555 - 	Added OnCheckFilterOnPIC()
	// (c.haag 2010-07-19 13:40) - PLID 30894 - Handle table checkers
	// Generated message map functions
	//{{AFX_MSG(CPatientLabsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewLab();
	afx_msg void OnDeleteLab();
	afx_msg void OnLabsNeedingAttention();
	afx_msg void OnNewAttachment();
	afx_msg void OnDetach();
	afx_msg void OnOpenLabs();
	afx_msg void OnClosedLabs();
	afx_msg void OnAllLabs();
	afx_msg void OnEditingFinishedLabsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownLabsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnMarkdone();
	afx_msg void OnLeftClickLabsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDetachFromLab();
	afx_msg void OnDetachFromLabAndHistory();
	afx_msg void OnDetachAndDeleteFile();
	afx_msg void OnAttachExistingFile();
	afx_msg void OnAttachExistingFolder();	
	afx_msg void OnImportAndAttachExistingFile();
	afx_msg void OnImportFromScanAsPDF();
	afx_msg void OnImportFromScanAsMulti();
	afx_msg void OnImportFromScannerCamera();
	afx_msg void OnEditingStartingLabsList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnPrintAttachment();
	afx_msg void OnDblClickCellLabsList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChangedLabsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishingLabsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnOpenLab();
	afx_msg void OnDiscontinueLab();
	afx_msg void OnCheckFilterOnPIC();
	afx_msg LRESULT OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedResultGraphPreview();
	afx_msg void OnShowAllResults();
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (r.gonet 09/02/2014) - PLID 63221 - Added
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCheckDiscontinuedLabs();
	afx_msg LRESULT OnUpdateView(WPARAM wParam, LPARAM lParam);
	// (r.gonet 09/02/2014) - PLID 63539 - Added
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATIENTLABSDLG_H__C86FF85B_90C5_466D_B996_62C3DDA378F6__INCLUDED_)
