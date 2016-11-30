#pragma once

class CEMN;

class CPicContainerDlg;
class CEmrFrameWnd;
class CEMRPreviewCtrlDlg;
class CEmrTreeWnd;
class CEmrEditorDlg;
class CEmrEditorBase;

class CEmrTopicView;

namespace Emr
{

class AttachedEMNImpl
{
public:
	AttachedEMNImpl()
		: m_pAttachedEMN(NULL)
	{
	}

	CEMN* m_pAttachedEMN;

	void SetAttachedEMN(CEMN* pEMN)
	{
		ASSERT(!m_pAttachedEMN || (m_pAttachedEMN == pEMN));
		m_pAttachedEMN = pEMN;
	}

	CEMN* GetAttachedEMN() const
	{
		return m_pAttachedEMN;
	}
	
	// (a.walling 2012-07-03 10:56) - PLID 51284 - Update the title (now requires pointer)
	void UpdateTitle(CWnd* pThis);
};

// (a.walling 2011-10-20 14:23) - PLID 46071 - Accesses the top-level windows
template<typename T>
class InterfaceAccessImpl
{
public:
	CPicContainerDlg* GetPicContainer()
	{
		return dynamic_cast<CPicContainerDlg*>(static_cast<T*>(this)->GetTopLevelFrame());
	}

	CEmrFrameWnd* GetEmrFrameWnd()
	{
		return dynamic_cast<CEmrFrameWnd*>(static_cast<T*>(this)->GetTopLevelFrame());
	}

	CEMRPreviewCtrlDlg* GetEmrPreviewCtrl()
	{
		CEmrFrameWnd* pEmrFrameWnd = GetEmrFrameWnd();

		if (!pEmrFrameWnd) return NULL;

		return pEmrFrameWnd->GetEmrPreviewCtrl();
	}

	CEmrTreeWnd* GetEmrTreeWnd()
	{
		CEmrFrameWnd* pEmrFrameWnd = GetEmrFrameWnd();

		if (!pEmrFrameWnd) return NULL;

		return pEmrFrameWnd->GetEmrTreeWnd();
	}

	CEmrEditorDlg* GetEmrEditor()
	{
		CPicContainerDlg* pPicContainer = GetPicContainer();

		if (!pPicContainer) return NULL;

		return pPicContainer->GetEmrEditor();
	}

	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor
	CEmrEditorBase* GetEmrEditorBase()
	{
		CEmrFrameWnd* pEmrFrameWnd = GetEmrFrameWnd();

		if (!pEmrFrameWnd) return NULL;

		return pEmrFrameWnd->GetEmrEditorBase();
	}
};

// (a.walling 2011-10-20 14:23) - PLID 46071 - Accesses topic information for items seomwhere within a topic view
template<typename T>
class TopicInterfaceAccessImpl : public InterfaceAccessImpl<T>
{
public:
	CEmrTopicView* GetEmrTopicView()
	{
		CWnd* pParent = static_cast<T*>(this)->GetParent();

		CEmrTopicView* pTopicView = NULL;
		while (pParent && !pParent->IsFrameWnd()) {
			pTopicView = dynamic_cast<CEmrTopicView*>(pParent);

			if (pTopicView) {
				return pTopicView;
			}

			pParent = pParent->GetParent();
		}

		return NULL;
	}
};

}
