#pragma once

#include "EmrTopicView.h"
#include "EmrDocument.h"
#include "EmrCodesView.h"

class CEmrDocTemplate : public CMultiDocTemplate
{
	DECLARE_DYNAMIC(CEmrDocTemplate)
public:
	CEmrDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
		CRuntimeClass* pFrameClass, CWnd* pOverrideFrameParent)
		: CMultiDocTemplate(nIDResource, pDocClass, pFrameClass, NULL)
		, m_pOverrideFrameParent(pOverrideFrameParent)
	{
	}

	//CEmrDocument* GetEmrDocument(CEMN* pEMN, bool bAutoCreate);

	CFrameWnd* CreateNewEmrFrame(CDocument* pDoc, CRuntimeClass* pViewClass);

	virtual void SetDefaultTitle(CDocument* pDocument);

	// (a.walling 2013-02-13 09:30) - PLID 56125 - Topic and more info view creation handling now within the doctemplate

	CEmrTopicView* GetEmrTopicView(CEMN* pEMN, bool bAutoCreate);
	CEmrTopicView* CreateEmrTopicView(CEMN* pEMN = NULL);
	// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
	CEmrMoreInfoView* GetEmrMoreInfoView(CEMN* pEMN, bool bAutoCreate);
	CEmrMoreInfoView* CreateEmrMoreInfoView(CEMN* pEMN = NULL);
	//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
	CEmrCodesView* GetEmrCodesView(CEMN* pEMN, bool bAutoCreate);
	CEmrCodesView* CreateEmrCodesView(CEMN* pEMN = NULL);

protected:
	CWnd* m_pOverrideFrameParent;
};
