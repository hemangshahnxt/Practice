// (r.gonet 08/22/2011) - PLID 45555 - Initial Commit

#pragma once

#include "PatientsRc.h"
#include "LabCustomField.h"
#include "LabCustomFieldControlManager.h"

// CLabCustomFieldsView dialog

// (r.gonet 09/21/2011) - PLID 45555 - CLabCustomFieldsView defines a window that can take the fields from a lab custom fields template instance 
//  and draw their controls to it. It allows for syncing of the controls back to the fields when Save is called.
//  Scrolling is also allowed since we can have an arbitrary number of controls.
class CLabCustomFieldsView : public CNxDialog
{
	DECLARE_DYNAMIC(CLabCustomFieldsView)
private:
	// (r.gonet 09/21/2011) - PLID 45555 - A go between between our window and the instance's fields.
	CLabCustomFieldControlManager *m_pControlManager;
	// (r.gonet 09/21/2011) - PLID 45555 - The template instance that houses the fields that will be drawn to our window.
	CCFTemplateInstance *m_pTemplateInstance;
	// (r.gonet 09/21/2011) - PLID 45555 - The current scroll position in ticks.
	CSize m_szScrollPos;
	// (r.gonet 09/21/2011) - PLID 45555 - The current edit mode, how things are synced back to the fields.
	CLabCustomFieldControlManager::EEditMode m_emEditMode;
	// (r.gonet 09/21/2011) - PLID 45555 - The number of pixels per tick of scrolling.
	CSize m_szPixelsPerScrollTick;
	// (r.gonet 10/20/2011) - PLID 45555 - Don't show the fields until we are told we are done loading.
	bool m_bShowControls;

public:
	CLabCustomFieldsView(CWnd* pParent, CCFTemplateInstance *pTemplateInstance, CLabCustomFieldControlManager::EEditMode emEditMode = CLabCustomFieldControlManager::emNone);   // standard constructor
	virtual ~CLabCustomFieldsView();

// Dialog Data
	enum { IDD = IDD_LAB_CUSTOM_FIELDS_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	// Generated message map functions
	virtual BOOL OnInitDialog();

	BOOL IsScrollBarVisible(HWND hWndParent, int nBar);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabCustomFieldsView)
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
private:
	void ScrollBy(long nDeltaX, long nDeltaY);
	void ScrollTo(long nX, long nY);
public:
	void SetTemplateInstance(CCFTemplateInstance *pTemplateInstance);
	void RefreshFields(bool bRedrawOnly = false);
	bool SyncControlsToFields();
	void EnsureFieldInView(CLabCustomFieldPtr pField);
	void ShowControls(bool bShow);

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
