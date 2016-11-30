#if !defined(AFX_EMRTEMPLATEEDITORDLG_H__46274928_E2B8_4ED4_A574_2A8811E81087__INCLUDED_)
#define AFX_EMRTEMPLATEEDITORDLG_H__46274928_E2B8_4ED4_A574_2A8811E81087__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTemplateEditorDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEmrTemplateEditorDlg dialog

#include "EmrTreeWnd.h"
#include "EmrEditorBase.h"
#include "EmrTemplateFrameWnd.h"

// (a.walling 2012-02-29 06:42) - PLID 46644 - Moved some CEmrTemplateEditorDlg logic to EmrTemplateFrameWnd; removed a lot

class CEmrTemplateEditorDlg : public CEmrEditorBase
{
// Construction
public:
	CEmrTemplateEditorDlg();   // standard constructor


// Implementation
protected:

	// (a.walling 2012-02-28 08:27) - PLID 48429 - EmrEditorBase abstraction for EmrEditor and EmrTemplateEditor
	virtual void LoadEMRObject();
	void OnCloseTemplate();

	virtual void Initialize();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTEMPLATEEDITORDLG_H__46274928_E2B8_4ED4_A574_2A8811E81087__INCLUDED_)
