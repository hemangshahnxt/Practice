#pragma once
#include "PatientsRc.h"

// (j.fouts 2012-08-10 09:44) - PLID 52090 - Added. 

// CMonographDlg dialog

class CMonographDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CMonographDlg)

public:
	CMonographDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMonographDlg();

	CString m_strHTML;
	//TES 11/21/2013 - PLID 59399 - Added a label that should only be shown for drug-drug interactions
	bool m_bShowDeveloper;
	
// Dialog Data
	enum { IDD = IDD_MONOGRAPH_DLG };

private:
	//Loads the html from the string passed by the API
	void LoadCustomHtml();

protected:
	IWebBrowser2Ptr m_pBrowser;
	// (j.fouts 2012-08-16 17:55) - PLID 52194 - Added buttons for print/preview
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnPrintPreview;
	CNxIconButton m_btnOk;
	// (j.fouts 2013-02-13 11:32) - PLID 55146 - Added a member for the NxColor
	CNxColor m_bkg;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// (j.fouts 2012-08-16 17:55) - PLID 52194 - Added buttons for print/preview
	afx_msg void OnBnClickedPrintMonograph();
	afx_msg void OnBnClickedPrintPreviewMonograph();
	afx_msg void DocumentComplete(LPDISPATCH pDisp, VARIANT* URL);

	DECLARE_EVENTSINK_MAP()
};
