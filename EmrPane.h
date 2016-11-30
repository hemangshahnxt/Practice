#pragma once
#include "EmrHelpers.h" //(e.lally 2012-02-15) PLID 48065
#include <NxAdvancedUILib/NxDockablePane.h>
#include <NxUILib/SafeMsgProc.h>

// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes

// CEmrPane

// (a.walling 2012-04-17 16:56) - PLID 49750 - Use CNxDockablePane
// (a.walling 2012-07-05 10:54) - PLID 50853 - Use SafeMsgProc to implement try/catch around window messages and commands
class CEmrPane : public SafeMsgProc<CNxDockablePane>, public Emr::InterfaceAccessImpl<CEmrPane>
{
	DECLARE_DYNAMIC(CEmrPane)

public:
	CEmrPane();

	class CEmrFrameWnd* GetEmrFrameWnd() const;

	virtual BOOL LoadState(LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);
	virtual BOOL SaveState(LPCTSTR lpszProfileName = NULL, int nIndex = -1, UINT uiID = (UINT) -1);

	// (a.walling 2012-04-17 16:56) - PLID 49750 - Most moved to shared CNxDockablePane

protected:
	DECLARE_MESSAGE_MAP()
};
