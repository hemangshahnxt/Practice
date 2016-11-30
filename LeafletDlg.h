#pragma once
#include "PatientsRc.h"

// (j.luckoski 2013-01-15 15:03) - PLID 53042 - Added. 

// CLeafletDlg dialog

class CLeafletDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLeafletDlg)

public:
	CLeafletDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLeafletDlg();

	CString m_strHTML;
	
// Dialog Data
	enum { IDD = IDD_LEAFLET_DLG };
	CNxColor	m_bkg;

private:
	//Loads the html from the string passed by the API
	void LoadCustomHtml();

protected:
	IWebBrowser2Ptr m_pBrowser;
	/// (j.luckoski 2013-01-15 15:03) - PLID 54638 - Added buttons for print/preview
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnPrintPreview;
	CNxIconButton m_btnOk;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// (j.luckoski 2013-01-15 15:03) - PLID 54638 - Added buttons for print/preview
	afx_msg void OnBnClickedPrintLeaflet();
	afx_msg void OnBnClickedPrintPreviewLeaflet();
	afx_msg void DocumentComplete(LPDISPATCH pDisp, VARIANT* URL);

	DECLARE_EVENTSINK_MAP()
};
