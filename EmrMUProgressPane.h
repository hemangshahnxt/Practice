#pragma once
#include "EmrProgressPane.h"
#include "CCHITReportInfoListing.h"

//(e.lally 2012-02-28) PLID 48016 - Created
class CEmrMUProgressPane :	public CEMRProgressPane
{
	DECLARE_DYNCREATE(CEmrMUProgressPane)
public:
	CEmrMUProgressPane();
	~CEmrMUProgressPane();
	// (e.lally 2012-03-14) PLID 48891
	void RecalculateMeasures(bool bForceRefresh = false);

protected:
	CCHITReportInfoListing m_cchitReportListing;
	long m_nPatientMeassuresComplete, m_nPatientMeasuresTotal;
	HANDLE m_hStopThread;
	CWinThread* m_pLoadingThread;
	bool m_bIsLoading;
	CArray<long, long&> m_aryReportIDs; //(e.lally 2012-04-04) PLID 49378
	DWORD m_dwCurrentTickcountID; //(e.lally 2012-04-04) PLID 49378 - Used to identify which posted messages belong to the current thread

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnProcessingAddMeasureResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnProcessingMeasuresFinished(WPARAM wParam, LPARAM lParam);

	////
	/// UI State overrides
	afx_msg void OnUpdateStatusLabel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowDetails(CCmdUI* pCmdUI); //(e.lally 2012-04-23) PLID 48016

	void KillLoadingThread();
	void OnShowMUPatientMeasures();//(e.lally 2012-02-28) PLID 48265
	void OnConfigure(); //(e.lally 2012-03-26) PLID 48264


	DECLARE_MESSAGE_MAP()
};
