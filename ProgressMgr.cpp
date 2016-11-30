#include "stdafx.h"
#include "ProgressMgr.h"

//////////////////////////////////////////////////////////////////
// CProgressTimeTracker
//////////////////////////////////////////////////////////////////

void CProgressTimeTracker::Clear()
{
	m_arydwTransitionTimes.RemoveAll();
	m_arystrTransitionTexts.RemoveAll();
}

void CProgressTimeTracker::ReportResults(CWnd *pwndParent, BOOL bCollapseDuplicates /*= FALSE*/)
{
	if (m_arydwTransitionTimes.GetSize() > 1) {
		// No parent, so give the final message
		CMapStringToPtr mapDurations, mapCounts;
		CStringArray arystrMapKeys;
		if (bCollapseDuplicates) {
			for (long i=1; i<m_arydwTransitionTimes.GetSize(); i++) {
				CString strEntryName = m_arystrTransitionTexts.GetAt(i-1);
				const DWORD dwThisEntryDuration = m_arydwTransitionTimes.GetAt(i) - m_arydwTransitionTimes.GetAt(i-1);
				DWORD dwThisEntryNameDuration;
				long nThisEntryNameCount;
				if (mapDurations.Lookup(strEntryName, ((void *&)dwThisEntryNameDuration))) {
					dwThisEntryNameDuration += dwThisEntryDuration;
					nThisEntryNameCount = (long)(mapCounts[strEntryName]) + 1;
				} else {
					dwThisEntryNameDuration = dwThisEntryDuration;
					nThisEntryNameCount = 1;
					arystrMapKeys.Add(strEntryName);
				}
				mapDurations.SetAt(strEntryName, (void *)dwThisEntryNameDuration);
				mapCounts.SetAt(strEntryName, (void *)nThisEntryNameCount);
			}
		}

		CString strMsg;
		strMsg.Format("Total Entries: %li\nTotal Time: %lu\n\n", 
			m_arydwTransitionTimes.GetSize() - 1, m_arydwTransitionTimes.GetAt(m_arydwTransitionTimes.GetSize() - 1) - m_arydwTransitionTimes.GetAt(0));
		long nLen = arystrMapKeys.GetSize() ? arystrMapKeys.GetSize() : m_arydwTransitionTimes.GetSize();
		for (long i=(arystrMapKeys.GetSize()?0:1); i<nLen; i++) {
			CString strText = arystrMapKeys.GetSize() ? arystrMapKeys.GetAt(i) : m_arystrTransitionTexts.GetAt(i-1);
			long nSpaceCount = 60-2*strText.GetLength();
			if (nSpaceCount < 1) {
				nSpaceCount = 1;
			}
			CString str;
			str.Format("%s: %*s \t\t%lu\n", 
				arystrMapKeys.GetSize() ? strText + " (" + AsString((long)mapCounts[strText]) + ")" : strText, nSpaceCount, " ", arystrMapKeys.GetSize() ? (DWORD)(mapDurations[strText]) : (m_arydwTransitionTimes.GetAt(i) - m_arydwTransitionTimes.GetAt(i-1)));
			strMsg += str;
		}

		pwndParent->MessageBox(strMsg, NULL, MB_OK|MB_ICONINFORMATION);
	}
}

void CProgressTimeTracker::AddResult(const CString &strText)
{
	AddResult(GetTickCount(), strText);
}

void CProgressTimeTracker::AddResult(DWORD dwTickCount, const CString &strText)
{
	m_arydwTransitionTimes.Add(dwTickCount);
	m_arystrTransitionTexts.Add(strText);
}

void CProgressTimeTracker::AddResults(const CProgressTimeTracker *ppttCopyFromTimeTracker)
{
	AddResults(ppttCopyFromTimeTracker->m_arystrTransitionTexts, ppttCopyFromTimeTracker->m_arydwTransitionTimes);
}

void CProgressTimeTracker::AddResults(const CStringArray &aryTTexts, const CDWordArray &aryTTimes)
{
	for (long i=0; i<aryTTexts.GetSize(); i++) {
		m_arystrTransitionTexts.Add(aryTTexts.GetAt(i));
		m_arydwTransitionTimes.Add(aryTTimes.GetAt(i));
	}
}

//////////////////////////////////////////////////////////////////







//////////////////////////////////////////////////////////////////
// CProgressTimeTracker
//////////////////////////////////////////////////////////////////

long CProgressMgr::m_nTimeTrackerIndentCount = 0;

CProgressMgr::CProgressMgr()
	: m_dlgProgress(NULL)
{
	m_nTimeTrackerIndentCount++;
	m_ppttTimeTracker = NULL;
}

CProgressMgr::CProgressMgr(IN const CString &strTitle, OPTIONAL IN const CString &strAutoStartInitText /*= ""*/, OPTIONAL IN const long nAutoStartMin /*= 0*/, OPTIONAL IN const long nAutoStartMax /*= 100*/, CProgressTimeTracker *ppttTimeTracker /*= NULL*/, OPTIONAL IN const CString &strExtraProgress /*= ""*/)
	: m_dlgProgress(NULL)
{
	m_nTimeTrackerIndentCount++;
	m_ppttTimeTracker = ppttTimeTracker;
	
	StartProgress(strTitle, strAutoStartInitText, nAutoStartMin, nAutoStartMax, strExtraProgress);
}

CProgressMgr::~CProgressMgr()
{
	m_nTimeTrackerIndentCount--;
}

void CProgressMgr::StartProgress(IN const CString &strTitle, OPTIONAL IN const CString &strInitText /*= ""*/, OPTIONAL IN const long nMin /*= 0*/, OPTIONAL IN const long nMax /*= 100*/, OPTIONAL IN const CString &strExtraProgress /*= ""*/)
{
	// Create
	m_dlgProgress.Create(IDD_BACKUP_PROGRESS, CWnd::GetDesktopWindow());
	
	// Init
	m_dlgProgress.SetWindowText(strTitle);

	//TES 12/21/2006 - PLID 23957 - If we have an extra progress indicator, put it on top, otherwise put the InitText on top.
	m_strExtraProgress = strExtraProgress;
	if(m_strExtraProgress.IsEmpty()) {
		m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG, strInitText);
		m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG2, "");
	}
	else {
		m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG, m_strExtraProgress);
		m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG2, strInitText);
	}
	CWnd *pProgCtrl = m_dlgProgress.GetDlgItem(IDC_PROGRESS);
	ASSERT(pProgCtrl->GetSafeHwnd());
	pProgCtrl->SendMessage(PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);
	pProgCtrl->SendMessage(PBM_SETPOS, nMin);
	
	// Show
	m_dlgProgress.SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
	m_dlgProgress.BringWindowToTop();
	m_dlgProgress.SetForegroundWindow();

	if (m_ppttTimeTracker) {
		m_ppttTimeTracker->AddResult(GetTickCount(), CString(' ', m_nTimeTrackerIndentCount) + strInitText);
	}
}

BOOL CProgressMgr::IsCancelRequested()
{
	return FALSE;
}

void CProgressMgr::SetProgress(OPTIONAL IN LPCTSTR strText /*= NULL*/, OPTIONAL IN const long nProgress /*= -1*/, BOOL bAutoBringToTop /*= TRUE*/)
{
	if (bAutoBringToTop) {
		m_dlgProgress.BringWindowToTop();
		// (b.cardillo 2004-09-22 14:53) - PLID 7445 - BringWindowToTop is all we want to do, we don't actually want to 
		// SetForegroundWindow (which is what we used to do right here after the BringWindowToTop).  The problem is, 
		// BringWindowToTop doesn't always work like we want it to (which is why we had the SetForegroundWindow here 
		// before).  So now what we do here after BringWindowToTop instead of SetForegroundWindow is we check to see if 
		// the window was actually brought to the top, and if not, then we use SetWindowPos to put our window's z-order 
		// immediately above the current top window.  But since this might make us a topmost window, we then immediately 
		// change our z-order to be non-topmost.
		HWND hWnd = ::GetTopWindow(NULL);
		if (hWnd != m_dlgProgress.m_hWnd) {
			::SetWindowPos(m_dlgProgress.m_hWnd, hWnd, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
			::SetWindowPos(m_dlgProgress.m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
		}
	}

	if (strText) {
		//TES 12/21/2006 - PLID 29357 - If we have extra progress text in the top line, put this text in the bottom line.
		if(m_strExtraProgress.IsEmpty()) {
			m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG, strText);
			m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG2, "");
		}
		else {
			m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG, m_strExtraProgress);
			m_dlgProgress.SetDlgItemText(IDC_STATIC_LAST_MSG2, strText);
		}
	}
	if (nProgress != -1) {
		m_dlgProgress.GetDlgItem(IDC_PROGRESS)->SendMessage(PBM_SETPOS, nProgress);
	}

	if (m_ppttTimeTracker) {
		m_ppttTimeTracker->AddResult(GetTickCount(), CString(' ', m_nTimeTrackerIndentCount) + strText);
	}
}

//////////////////////////////////////////////////////////////////
