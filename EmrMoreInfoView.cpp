// EmrMoreInfoView.cpp : implementation file
//

#include "stdafx.h"
#include "EmrMoreInfoView.h"

#include "EmrFrameWnd.h"


// CEmrMoreInfoView

// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view

IMPLEMENT_DYNCREATE(CEmrMoreInfoView, CNexTechDialogView)

CWnd* CEmrMoreInfoView::CreateChildWnd()
{
	m_moreInfoDlg.Create(IDD_EMN_MORE_INFO_DLG, this);
	return &m_moreInfoDlg;
}

// (a.walling 2012-07-03 10:56) - PLID 51284 - Ensure the appropriate EMN is activated and highlighted in the tree when this view becomes active
void CEmrMoreInfoView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	try {
		if (bActivate && this == pActivateView) {
			if (CEMN* pEMN = GetAttachedEMN()) {
				GetEmrFrameWnd()->ActivateEMN(pEMN, NULL, false);
			}
			// (a.walling 2012-07-03 10:56) - PLID 51284 - Update the title (now requires pointer passed into it for AttachedEMNImpl)
			// although actually we will probably just keep this title as more info anyway for now
			//UpdateTitle(this);
		}

		__super::OnActivateView(bActivate, pActivateView, pDeactiveView);
	} NxCatchAll(__FUNCTION__);
}

