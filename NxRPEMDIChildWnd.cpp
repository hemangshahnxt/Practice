#include "stdafx.h"
#include "NxRPEMDIChildWnd.h"

#include "NxReportJob.h"
#include "crpe.h"
#include "reports.h"

// Implementation source file for the CNxRPEMDIChildWnd class
//////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CNxRPEMDIChildWnd, CRPEMDIChildWnd)

// Constructor
CNxRPEMDIChildWnd::CNxRPEMDIChildWnd(CRPEJob *pJob /*= NULL*/) : CRPEMDIChildWnd(), m_pJob(pJob)
{
	m_objPrintInfo.m_pPD->m_pd.hDevNames = NULL;
	m_objPrintInfo.m_pPD->m_pd.hDevMode = NULL;
};

// Destructor
CNxRPEMDIChildWnd::~CNxRPEMDIChildWnd()
{
	// (j.jones 2005-09-08 17:08) - this is one function that will be hit
	// when a report is closed by the user
	//
	// When a report is closed, 
	//    CNxRPEMDIChildWnd::~CNxRPEMDIChildWnd() is fired,
	// which deletes the report job, causing both
	//    CNxReportJob::~CNxReportJob() and
	//    CRPEJob::~CRPEJob() to fire in succession
	// and ~CRPEJob will call CRPEngine::RemoveJob()
	//

	// (j.jones 2005-09-09 10:49) - free memory that was allocated
	if(m_objPrintInfo.m_pPD->m_pd.hDevMode) {
		GlobalFree(m_objPrintInfo.m_pPD->m_pd.hDevMode);
		m_objPrintInfo.m_pPD->m_pd.hDevMode = NULL;
	}

	if(m_objPrintInfo.m_pPD->m_pd.hDevNames) {
		GlobalFree(m_objPrintInfo.m_pPD->m_pd.hDevNames);
		m_objPrintInfo.m_pPD->m_pd.hDevNames = NULL;
	}

	if (m_pJob) {
		delete m_pJob;
		m_pJob = NULL;
	}
}

// Message map
BEGIN_MESSAGE_MAP(CNxRPEMDIChildWnd, CRPEMDIChildWnd)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

// Handler for the ID_FILE_PRINT command (menu item and toolbar button)
void CNxRPEMDIChildWnd::OnFilePrint()
{
	CNxReportJob *pJob = reinterpret_cast<CNxReportJob *>(m_pJob);
	if (pJob) {
		try {
			pJob->OutputToPrinter(1, &m_objPrintInfo);
			PEStartPrintJob(pJob->GetJobHandle(), TRUE);
		} NxCatchAll("CMainFrame::OnCommand:ID_FILE_PRINT");
	}
}

void CNxRPEMDIChildWnd::OnClose()
{
	CMainFrame *pMainFrm = GetMainFrame();
	if(pMainFrm) {
		if(pMainFrm->m_bIsReportRunning) {
			MessageBox("To stop a report, please click the stop button.");
			return;
		}
	}
		

	CRPEMDIChildWnd::OnClose();
}