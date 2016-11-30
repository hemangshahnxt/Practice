#pragma once

#include "PracticeRC.h"
#include <NxUILib/SafeMsgProc.h>
#include <boost/scoped_ptr.hpp>
#include <NxAdvancedUILib/NxHtmlEditor.h>

// (b.cardillo 2013-03-11 18:00) - PLID 55576 - Dialog for easy WYSIWYG editing of html

// CHtmlEditorDlg dialog

class CHtmlEditorDlg : public SafeMsgProc<CNxDialog>
{
	DECLARE_DYNAMIC(CHtmlEditorDlg)

public:
	// (b.cardillo 2013-03-12 14:06) - PLID 55576 supplemental - Since we're sizable, support storing/recalling our size and position
	// Simple as possible: just call this static function to edit some html
	// Always updates strHtmlInOut with the final html even if the user clicked the Cancel button.
	// Returns IDOK or IDCANCEL depending on what the user clicked to dismiss the dialog.
	static int InputHtmlBox(CWnd* pParent, const CString &strTitle, const CString &strSizingConfigRT, IN OUT CString &strHtmlInOut);

public:
	CHtmlEditorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHtmlEditorDlg();

// Dialog Data
	enum { IDD = IDD_HTML_EDITOR_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

public:
	CString m_strTitle;
	CString m_strSizingConfigRT;
	CString m_strHtml;

public:
	virtual BOOL IncludeChildPosition(IN HWND hwnd);
	virtual int GetControlPositions();
	virtual int SetControlPositions();

protected:
	boost::scoped_ptr<class CNxHtmlEditor> m_pHtmlEditor;

protected:
	// Info about the dialog and certain controls so we can position everything appropriately
	int m_nSizePos_OKandCancelDistanceBetween;
	int m_nSizePos_OKandCancelTop;
	int m_nSizePos_OKandCancelHeight;
	int m_nSizePos_OKWidth;
	int m_nSizePos_CancelWidth;
	int m_nSizePos_WindowMargin;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);

	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
