
#if !defined(AFX_EMREDITORDLG_H__58DE9A5D_2019_42B8_BE9D_18A00C0C6580__INCLUDED_)
#define AFX_EMREDITORDLG_H__58DE9A5D_2019_42B8_BE9D_18A00C0C6580__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrEditorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrEditorDlg dialog
class CEMR;
class CEMN;
class CEmrMedAllergyViewerDlg;
#include "EmrTreeWnd.h"
#include "EmrTopicWnd.h"
#include "EMRBarcodeDlg.h"
#include "EmrEditorBase.h"
#include "EMNProcedure.h"

// (a.walling 2011-12-09 17:16) - PLID 46643 - Moving tools and etc into the CEmrFrameWnd

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor
// moved lots of common operations into CEmrEditorBase


//DRT 7/13/2007 - Clarification:  This structure is used primarily for the EMR description tracking features.  We 
//	need to be able to detect a generated description, and to do that, we need to be able to track a copy of
//	the previous procedures selected.  That is why we copy things here, instead of using the pointer to always
//	get current.
struct LocalEmn {//Just the EMN info we care about, plus the pointer for reference.
	CEMN *pEMN;
	// (z.manning 2010-01-13 11:10) - PLID 22672 - Age is now a string
	CString strPatientAge;
	BYTE nPatientGender;

	//DRT 7/13/2007 - PLID 26671 - The EMN keeps a list of procedures with lots of info -- we want that, not
	//	just the ID.  That lets us more capably look at information without having to query the database
	//	for procedure names.
	//CArray<long,long> arProcedureIDs;
	CArray<EMNProcedure, EMNProcedure> arProcedures;

	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - requires a return type
	void operator =(LocalEmn &leSource) {
		pEMN = leSource.pEMN;
		strPatientAge = leSource.strPatientAge;
		nPatientGender = leSource.nPatientGender;
		arProcedures.RemoveAll();
		for(int i = 0; i < leSource.arProcedures.GetSize(); i++) {
			arProcedures.Add(leSource.arProcedures.GetAt(i));
		}
	}
};

class CEMN;
class CEMRTopic;

class CEmrEditorDlg : public CEmrEditorBase
{
// Construction
public:
	CEmrEditorDlg();   // standard constructor

	// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
	class CPicContainerDlg* const GetPicContainer() const;

	//Edit an existing EMR, optionally starting on an EMN.
	//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
	void SetEmr(long nEMRID, long nPicID, long nStartingEMNID = -1);
	//Add a new EMN to an existing EMR, and edit that new EMN.
	//TES 11/22/2010 - PLID 41582 - Added nPicID (only really needed for new EMRs)
	void SetEmrWithNewEmn(long nEMRID, long nPicID, long nTemplateID);

	long GetEmrID();
	
	void TryGenerateAndApplyEMRDescription();

	//TES 5/20/2008 - PLID 27905 - This is sometimes called when procedures are about to be deleted; if so, pass in the EMN
	// they're being deleted from and this will only return true if the procedure is on some other EMN.
	BOOL IsProcedureInEMR(long nProcedureID, CEMN *pExcludingEmn = NULL);

	// (c.haag 2010-08-02 09:27) - PLID 38928 - Called when EMR content is saved
	void Commit();

public:

	// (a.walling 2008-06-27 13:57) - PLID 30496
	CEMN* GetEMNByID(int nID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrEditorDlg)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2010-01-14 11:42) - PLID 26036 - The total amount of time paused in this session
	COleDateTimeSpan m_dtsPreviousTimePaused;
	// (c.haag 2010-01-14 11:42) - PLID 26036 - The last time that we paused this session in LOCAL time
	COleDateTime m_dtLastPaused_Local;

	virtual void LoadEMRObject();

	// (j.jones 2008-06-04 10:51) - PLID 28073 - tracks when we are loading the EMR,
	// *not* creating a new EMR from a template
	BOOL m_bIsLoadingExistingEMR;

	long m_nEMRID;
	//TES 11/22/2010 - PLID 41582 - Added m_nPicID so that new EMRs will save to the correct PIC
	long m_nPicID;
	long m_nStartingEMNID;
	long m_nNewEmnTemplateID;

	CArray<LocalEmn, LocalEmn&> m_arEmnInfo;
	// (j.jones 2010-04-01 15:58) - PLID 34915 - this function tries to update the dialog text,
	// but dialog has no menu bar, it's part of the PIC!
	//void DisplayEmnInfo();//Call when m_arEmnInfo changes, will update the screen accordingly.

	CString GenerateEMRDescription();

	//Just so we don't have to keep looking it up.
	// (a.walling 2012-05-18 17:15) - PLID 50546 - No longer necessary
	//CString m_strPatientName;

	CMap<long,long,CString,CString> m_mapProcedureNames;
	CString GetProcedureName(long nProcedureID);

	CEMRBarcodeDlg *m_pBarcodeDlg;	// (j.dinatale 2011-07-26 17:41) - PLID 44702 - barcode dialog to display this emr's barcode

	// (j.jones 2007-01-31 13:37) - PLID 24493 - store the open time, for the clock
	COleDateTime m_dtCurrentTimeOpened;
	//store the total time opened in the past
	COleDateTimeSpan m_dtHistoricalTimeOpenedSpan;
	//store the offset between the server and workstation
	COleDateTimeSpan m_dtOffset;

	// (a.walling 2011-12-09 17:16) - PLID 46643 - Time tracking
	int m_nTotalSecondsOpen;
	int m_nCurrentSecondsOpen;

public:

	int GetTotalSecondsOpen() const
	{
		return m_nTotalSecondsOpen;
	}

	int GetCurrentSecondsOpen() const
	{
		return m_nCurrentSecondsOpen;
	}

public:

public:
	// (a.walling 2012-05-18 17:18) - PLID 50546 - No longer necessary
	//void OnParentClosing();
	// (j.jones 2007-01-30 11:21) - PLID 24353 - we commit times when the dialog is closed
	void StopTrackingEMNTimes();

	// (j.jones 2007-02-06 15:17) - PLID 24493 - calculate the offset of time between the
	// server and workstation
	void CalculateLocalTimeOffset();
	//track when the user changes the workstation time
	void FireTimeChanged();

public:
	// (c.haag 2007-03-07 16:33) - PLID 25110 - Patient ID and name functions
	long GetPatientID() const;
	CString GetPatientName() const;

public:
	// (c.haag 2008-07-09 12:41) - PLID 30648 - This is called when cancelling out of an
	// unsaved EMR. We need to see if any todo alarms were spawned or unspawned for unsaved
	// topics. If so, the user needs to be made aware of them, and resolve them by either
	// getting rid of them, or leaving them alone.
	// (c.haag 2008-07-10 16:42) - Returns TRUE if the user accepted any changes, or there
	// were no known changes to make. Returns FALSE if they cancelled out and changed their mind.
	BOOL FindAndResolveChangedEMRTodoAlarms();

protected:
	// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "pause" the clock timer that tracks how long
	// someone has had the chart open
	void PauseClock();

	// (c.haag 2010-01-14 11:28) - PLID 26036 - Called to "resume" the clock timer that tracks how long
	// someone has had the chart open
	void ResumeClock();

	// (c.haag 2010-01-14 12:04) - PLID 26036 - Update our states which track
	// whether the clock is paused or not
	void UpdateClockPauseState();

	// (c.haag 2010-01-14 11:35) - PLID 26036 - Returns the total amount of time paused since the
	// EMR was opened
	COleDateTimeSpan GetTotalTimeClockWasPaused() const;

public:
	// (j.jones 2012-03-27 14:59) - PLID 44763 - added global period warning
	void CheckWarnGlobalPeriod_EMR(COleDateTime dtToCheck);

protected:
	// (j.jones 2012-03-27 14:59) - PLID 44763 - tracks if we have warned about a global period in this EMR session
	BOOL m_bCheckWarnedGlobalPeriod;

protected:	
	// (a.walling 2008-06-25 12:44) - PLID 30496
	// (a.walling 2008-06-27 17:30) - PLID 30482 - Added OnRefreshInfo
	// Generated message map functions
	//{{AFX_MSG(CEmrEditorDlg)
	virtual void Initialize();

	afx_msg LRESULT OnEmnAdded(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEmnRemoved(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnNonClinicalProceduresAdded(WPARAM wParam, LPARAM lParam);
	// (a.walling 2012-05-18 17:18) - PLID 50546 - No longer necessary
	//afx_msg LRESULT OnRefreshInfo(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-03-31 17:01) - PLID 37980 - added ability to tell the EMR to add a given image
	afx_msg LRESULT OnAddImageToEMR(WPARAM wParam, LPARAM lParam);
	// (j.jones 2010-06-21 10:22) - PLID 39010 - added ability to add a generic table to the EMR
	afx_msg LRESULT OnAddGenericTableToEMR(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (d.lange 2010-06-29 18:01) - PLID 39202 - added ability to launch the CDeviceImportDlg dialog
	afx_msg void OnBnClickedBtnDeviceInfo();
	afx_msg void OnBtnShowDeviceImport();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREDITORDLG_H__58DE9A5D_2019_42B8_BE9D_18A00C0C6580__INCLUDED_)
