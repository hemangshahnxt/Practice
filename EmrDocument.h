#pragma once

#include "EmrHelpers.h"
#include "EmrTopicView.h"
#include "EmrMoreInfoView.h"
#include "EmrCodesView.h"

class CEMN;

// (a.walling 2011-10-20 21:22) - PLID 46078 - Facelift - EMR Document / View / Command routing etc
// (a.walling 2013-02-13 09:30) - PLID 56125 - Topic and more info view creation handling now within the doctemplate

class CEmrDocument : public CDocument, public Emr::AttachedEMNImpl
{
	DECLARE_DYNCREATE(CEmrDocument)
public:
	CEmrDocument()
	{
	}

	virtual ~CEmrDocument()
	{
	}

	virtual BOOL IsModified();

	class CEmrDocTemplate* GetEmrDocTemplate() const;

	template<typename ViewType>
	ViewType* FindView()
	{
		ViewType* pView = NULL;
		POSITION pos = GetFirstViewPosition();
		while (pos && !pView) {
			pView = dynamic_cast<ViewType*>(GetNextView(pos));
		}
		return pView;
	}

	// (a.walling 2013-02-13 09:30) - PLID 56125 - Allow setting a path name directly without attempting filesystem resolution
	void SetPathNameDirect(CString strPath)
	{
		m_strPathName = strPath;
	}
protected:
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);
	virtual BOOL SaveModified(); // return TRUE if ok to continue
	virtual void PreCloseFrame(CFrameWnd* pFrame);

	virtual void OnIdle();
};
