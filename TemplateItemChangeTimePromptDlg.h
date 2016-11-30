#if !defined(AFX_TEMPLATEITEMCHANGETIMEPROMPT_H__4A86E1E1_526F_4D63_9D80_A3089566642E__INCLUDED_)
#define AFX_TEMPLATEITEMCHANGETIMEPROMPT_H__4A86E1E1_526F_4D63_9D80_A3089566642E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateItemChangeTimePrompt.h : header file
//

// Just make sure these aren't equal to IDCANCEL
#define ID_CHANGE_ONCE		IDYES
#define ID_CHANGE_LINE_ITEM	IDNO

#include "TemplateLineItemInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CTemplateItemChangeTimePromptDlg dialog

// (z.manning, 11/15/2006) - PLID 23555 - This is basically just a message box with
// speically labeled buttons.
class CTemplateItemChangeTimePromptDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2015-01-30 16:18) - PLID 64230 - Added dialog type
	CTemplateItemChangeTimePromptDlg(CTemplateLineItemInfo* pLineItem, ESchedulerTemplateEditorType eType, CWnd* pParent);

	void SetChangeType(CString strChangeType);

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CTemplateItemChangeTimePromptDlg)
	enum { IDD = IDD_TEMPLATE_ITEM_CHANGE_TIME_PROMPT };
	CNxStatic	m_nxstaticChangeTimePrompt;
	CNxIconButton	m_btnChangeLineItem;
	CNxIconButton	m_btnChangeOnce;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateItemChangeTimePromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CTemplateItemChangeTimePromptDlg(CWnd* pParent);   // standard constructor

	CTemplateLineItemInfo* m_pLineItem;

	CString m_strChangeType;
	// (z.manning, 04/29/2008) - PLID 29814 - The button style can differ depending on whether we're prompting
	// to change or delete
	NXB_TYPE m_eButtonStyle;

	// (z.manning 2015-01-30 16:19) - PLID 64230
	ESchedulerTemplateEditorType m_eEditorType;

	// Generated message map functions
	//{{AFX_MSG(CTemplateItemChangeTimePromptDlg)
	afx_msg void OnChangeLineItem();
	afx_msg void OnChangeOnce();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEITEMCHANGETIMEPROMPT_H__4A86E1E1_526F_4D63_9D80_A3089566642E__INCLUDED_)
