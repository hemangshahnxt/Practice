#pragma once

#include <NexTechDialogView.h>
#include "EmrHelpers.h"
#include "EmrCodesDlg.h"
#include "EMN.h"

class CEMN;

// CEmrCodesView view

//TES 2/11/2014 - PLID 60740 - New view for <Codes> topic

class CEmrCodesView : public CNexTechDialogView, public Emr::AttachedEMNImpl, public Emr::InterfaceAccessImpl<CEmrCodesView>
{
public:
	DECLARE_DYNCREATE(CEmrCodesView)
	
	CEmrCodesView() : m_codesDlg(this) {}

	CEmrCodesDlg* GetEmrCodesDlg()
	{
		if (!this) return NULL;
		return &m_codesDlg;
	}

	//j.camacho 10/27/2014 - plid 62641 - add timer flag
	long m_timerEmrSetSearchQueue = 0;
	DECLARE_MESSAGE_MAP()

protected:
	virtual CWnd* CreateChildWnd();

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Ensure the appropriate EMN is activated and highlighted in the tree when this view becomes active
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	CEmrCodesDlg m_codesDlg;

	afx_msg void OnTimer(UINT nIDEvents);
	
};


