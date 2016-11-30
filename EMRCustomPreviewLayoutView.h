#pragma once

#include <NxAdvancedUILib/NxHtmlMDIView.h>

// CEMRCustomPreviewLayoutView

class CEMRCustomPreviewLayoutView : public CNxHtmlMDIView
{
	DECLARE_DYNCREATE(CEMRCustomPreviewLayoutView)

public:
	CEMRCustomPreviewLayoutView();
	virtual ~CEMRCustomPreviewLayoutView();

protected:
	// (j.armen 2013-01-18 16:23) - PLID 54686 - Set the editor location
	virtual CString GetEditorUrl() override;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRenameLayout();
	afx_msg void OnDeleteLayout();
	afx_msg void OnFileSaveCurrentLayout();
	afx_msg void OnShowSource();
	afx_msg void OnUpdateShowSource(CCmdUI* pCmdUI);
};


