#ifndef NX_PROGRESS_MGR_H
#define NX_PROGRESS_MGR_H

#pragma once



#include "BackupProgessDlg.h"

class CProgressTimeTracker
{
public:
	void Clear();
	void AddResult(const CString &strText);
	void AddResult(DWORD dwTickCount, const CString &strText);
	void AddResults(const CProgressTimeTracker *ppttCopyFromTimeTracker);
	void AddResults(const CStringArray &aryTTexts, const CDWordArray &aryTTimes);
public:
	void ReportResults(CWnd *pwndParent, BOOL bCollapseDuplicates = FALSE);

protected:
	CStringArray m_arystrTransitionTexts;
	CDWordArray m_arydwTransitionTimes;
};




class CProgressMgr
{
public:
	CProgressMgr();
	//TES 12/21/2006 - PLID 23957 - Added optional strExtraProgress; if it exists, it will display in a line above the other text.
	CProgressMgr(IN const CString &strTitle, OPTIONAL IN const CString &strAutoStartInitText = "", OPTIONAL IN const long nAutoStartMin = 0, OPTIONAL IN const long nAutoStartMax = 100, CProgressTimeTracker *ppttTimeTracker = NULL, OPTIONAL IN const CString &strExtraProgress = "");
	~CProgressMgr();

public:
	void StartProgress(IN const CString &strTitle, OPTIONAL IN const CString &strInitText = "", OPTIONAL IN const long nMin = 0, OPTIONAL IN const long nMax = 100, OPTIONAL IN const CString &strExtraProgress = "");
	BOOL IsCancelRequested();
	void SetProgress(OPTIONAL IN LPCTSTR strText = NULL, OPTIONAL IN const long nProgress = -1, BOOL bAutoBringToTop = TRUE);

protected:
	// (z.manning, 05/16/2008) - PLID 30050 - Converted to NxDialog
	CBackupProgessDlg m_dlgProgress;
	
protected:
	CProgressTimeTracker *m_ppttTimeTracker;
	static long m_nTimeTrackerIndentCount;
	CString m_strExtraProgress;

};


#endif