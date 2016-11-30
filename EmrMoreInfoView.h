#pragma once

#include <NexTechDialogView.h>
#include "EmrHelpers.h"
#include "EMNMoreInfoDlg.h"


// CEmrMoreInfoView view

// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view

class CEmrMoreInfoView : public CNexTechDialogView, public Emr::AttachedEMNImpl, public Emr::InterfaceAccessImpl<CEmrMoreInfoView>
{
public:
	DECLARE_DYNCREATE(CEmrMoreInfoView)

	CEmrMoreInfoView() : m_moreInfoDlg(this) {}

	CEMNMoreInfoDlg* GetMoreInfoDlg()
	{
		if (!this) return NULL;
		return &m_moreInfoDlg;
	}

protected:
	virtual CWnd* CreateChildWnd();

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Ensure the appropriate EMN is activated and highlighted in the tree when this view becomes active
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	CEMNMoreInfoDlg m_moreInfoDlg;
};


