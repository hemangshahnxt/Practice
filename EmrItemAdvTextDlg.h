#if !defined(AFX_EMRITEMADVTEXTDLG_H__C8674EEB_4400_4E47_97EC_59EC5F61866E__INCLUDED_)
#define AFX_EMRITEMADVTEXTDLG_H__C8674EEB_4400_4E47_97EC_59EC5F61866E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemAdvTextDlg.h : header file
//

#include "EmrItemAdvDlg.h"

//TES 1/29/2008 - PLID 28673 - Define IDCs for any controls we create.
#define	EDIT_IDC	1000
#define SPELL_CHECK_IDC	1001
#define LAB_BUTTON_IDC	1002
/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvTextDlg dialog

// (c.haag 2010-01-07 13:41) - PLID 36794 - Special class that gives us control over
// the context menu appearance
class CEmrItemAdvTextEdit : public CEdit
{
public:
	CEmrItemAdvTextEdit() {};

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

	DECLARE_MESSAGE_MAP()
};

class CEmrItemAdvTextDlg : public CEmrItemAdvDlg
{
// Construction
public:
	CEmrItemAdvTextDlg(class CEMNDetail *pDetail);
	// (b.cardillo 2006-11-22 15:19) - PLID 23632 - Needed a destructor so we could free the 
	// font used on the spell-check button.
	~CEmrItemAdvTextDlg();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemAdvTextDlg)
	//}}AFX_VIRTUAL

public:
	virtual void ReflectCurrentState();
	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);
	//(s.dhole 12/19/2014 1:45 PM ) - PLID 63571 handel escape key
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

	virtual void OpenLabEntryDialog(); // (z.manning 2009-09-23 09:16) - PLID 33612

// Implementation
protected:
	CEmrItemAdvTextEdit m_edit; // (c.haag 2010-01-07 13:41) - PLID 36794 - Don't use CEdit any longer

	// (b.cardillo 2006-11-22 15:17) - PLID 23632 - Added a button for spell-checking the text.  
	// This also requires a font member, to store the font used on that button.
	CNxIconButton m_btnSpellCheck;
	// (z.manning 2008-10-08 12:02) - PLID 31613 - Added a button to open lab
	CNxIconButton m_btnOpenLab;

	// Generated message map functions
	//{{AFX_MSG(CEmrItemAdvTextDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnChangeEdit();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG

	// (b.cardillo 2006-11-22 10:08) - PLID 23632 - Added a spell-check handler, to be called 
	// whenever the user should be given a spell-check dialog for this detail.
	afx_msg void OnSpellCheck();

	// (z.manning 2008-10-08 12:24) - PLID 31613 - Added button to open lab
	afx_msg void OnLabButton();

	// (c.haag 2010-01-11 12:35) - PLID 31924 - This function will have the user choose a macro, and then
	// insert the text of that macro where the cursor is.
	LRESULT OnPasteMacroText(WPARAM wParam, LPARAM lParam);

	LRESULT OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMADVTEXTDLG_H__C8674EEB_4400_4E47_97EC_59EC5F61866E__INCLUDED_)

