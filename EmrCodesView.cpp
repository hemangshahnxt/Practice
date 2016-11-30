// EmrCodesView.cpp : implementation file
//

#include "stdafx.h"
#include "EmrCodesView.h"

#include "EmrFrameWnd.h"

#define IDT_SEARCHCODESREFRESH 3401

// CEmrCodesView

//TES 2/11/2014 - PLID 60740 - New view for <Codes> topic

IMPLEMENT_DYNCREATE(CEmrCodesView, CNexTechDialogView)

BEGIN_MESSAGE_MAP(CEmrCodesView, CNexTechDialogView)
	ON_WM_TIMER()
END_MESSAGE_MAP()



CWnd* CEmrCodesView::CreateChildWnd()
{
	m_codesDlg.Create(IDD_EMR_CODES_DLG, this);
	return &m_codesDlg;
}

//TES 2/11/2014 - PLID 60740 - Ensure the appropriate EMN is activated and highlighted in the tree when this view becomes active
void CEmrCodesView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	try {
		//make sure that the dialog is not currently "saving" and that the codes dialog is whats being activated.
		if (bActivate && this == pActivateView && !m_codesDlg.m_bIsTemplate && !m_codesDlg.m_bIsSaving) {
			//(j.camacho 10/27/2014 - plid 62641 - use a timer that allows us to cancel message calls and better handle concurrent calls.
			if (m_timerEmrSetSearchQueue && !bActivate) {
				
				KillTimer(m_timerEmrSetSearchQueue);
				m_timerEmrSetSearchQueue = 0;
			}
			else if (!m_timerEmrSetSearchQueue && bActivate) {
				
				m_timerEmrSetSearchQueue = SetTimer(IDT_SEARCHCODESREFRESH, 0, NULL);
			}

		}
		__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
	} NxCatchAll(__FUNCTION__);
}

//j.camacho 10/27/2014 - plid 62641 - handle timers
void CEmrCodesView::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == IDT_SEARCHCODESREFRESH)
	{
		try
		{
			//(j.camacho 8/11/2014) plid 62641 - catch here and call save function
			if (CEMN* pEMN = GetAttachedEMN()) {
				KillTimer(m_timerEmrSetSearchQueue);
				m_timerEmrSetSearchQueue = 0;
				//update the codes dlg ANYWAYS whether they saved or not. In case another source saved and we haven't updated our list yet.
				m_codesDlg.SendMessage(NXM_EMR_SET_SEARCH_QUEUE);
				GetEmrFrameWnd()->ActivateEMN(pEMN, NULL, false);
			}
		}NxCatchAll(__FUNCTION__);
	}
	else
	{
		__super::OnTimer(nIDEvent);
	}
	return;
}