#pragma once

#include "PracticeRc.h"

// CViewWebinarDlg dialog
// (b.savon 2011-12-19 17:12) - PLID 47074 - Created

enum WebinarAnchor{
	ewaBasics = 0,
	ewaBilling,
	ewaModules,
	ewaCommonTasks,
	ewaUncommonTasks,
	ewaWhatsNew,
	ewaEMR,
	ewaMeaningfulUse,
	ewaNone
};

class CViewWebinarDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CViewWebinarDlg)

public:
	CViewWebinarDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CViewWebinarDlg();

	int DoModal(const CString &strText, const CString &strName, const CString &strTitle /*= ""*/, const CString &strURLDisplay, const WebinarAnchor ewaAnchor);

	void ViewWebinar();

// Dialog Data
	enum { IDD = IDD_VIEW_WEBINAR };

private:
	CString m_strText;
	CString m_strName;
	CString m_strTitle;
	CString m_strURLDisplay;

	CNxStatic m_nxstaticMessage;
	CNxStatic m_nxstaticWebinar;

	CNxIconButton	m_btnOk;

	BOOL m_bDontRemind;

	WebinarAnchor m_ewaAnchor;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CString GetWebinarAnchor();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStnClickedWebinar();
	afx_msg void OnBnClickedDontRemind();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedOk();
};
