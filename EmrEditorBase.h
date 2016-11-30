#pragma once

#include "EmrTreeWnd.h"

// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor

class CEmrEditorBase : public CWnd
{
public:
	CEmrEditorBase();

	CEmrTreeWnd* GetEmrTreeWnd()
	{
		return &m_wndEmrTree;
	}
	
	CEmrTreeWnd m_wndEmrTree;

	// (j.jones 2012-10-11 17:58) - PLID 52820 - added bIsClosing, TRUE if the user picked Save & Close
	EmrSaveStatus Save(BOOL bShowProgressBar = TRUE, BOOL bIsClosing = FALSE);

	//Is this an empty EMR (i.e., one with no EMNs)?
	BOOL IsEmpty();
	
	BOOL IsEMRUnsaved();

	// (a.walling 2009-11-23 12:32) - PLID 36404 - Are we printing?
	bool IsPrinting();

	// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
	// and returns TRUE if any Image detail on this EMR references it
	BOOL IsImageFileInUseOnEMR(const CString strFileName);

	int GetEMNCount();

	CEMN* GetEMN(int nIndex);
		// (m.hancock 2006-11-28 11:10) - PLID 22302 - Returns a pointer containing m_pCurEMN.
	CEMN* GetCurrentEMN();

	// (a.walling 2008-07-07 11:32) - PLID 30496 - Get the current EMN from the treewnd
	CEMN* GetCurrentEMNFromTreeWnd();

	// (a.walling 2008-06-27 15:49) - PLID 30482 - Get the EMR
	CEMR* GetEMR();	

	// (j.jones 2010-04-01 11:58) - PLID 37980 - gets the pointer to the active EMN, if there is one
	CEMN* GetActiveEMN();

	// (j.jones 2010-03-31 17:33) - PLID 37980 - returns TRUE if this EMR has an EMN opened to a topic that is writeable
	BOOL HasWriteableEMRTopicOpen();

protected:	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual void Initialize() = 0;
	virtual void LoadEMRObject() = 0;
	
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//TES 8/13/2014 - PLID 63519 - Added support for EX tablecheckers
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

