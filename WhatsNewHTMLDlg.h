#if !defined(AFX_WHATSNEWHTMLDLG_H__D447ED2E_8DA4_4BC3_A1D8_31E724FFDBD4__INCLUDED_)
#define AFX_WHATSNEWHTMLDLG_H__D447ED2E_8DA4_4BC3_A1D8_31E724FFDBD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WhatsNewHTMLDlg.h : header file
//

//TES 11/7/2007 - PLID 27979 - VS2008 -  FindText is already a macro, and we don't use the imported FindText anyway.
#import "shdocvw.dll" exclude("OLECMDID", "OLECMDF", "OLECMDEXECOPT", "tagREADYSTATE") rename("FindText", "SHDOCVW_FindText")

/////////////////////////////////////////////////////////////////////////////
// CWhatsNewHTMLDlg dialog

class CWhatsNewHTMLDlg : public CNxDialog
{
// Construction
public:
	CWhatsNewHTMLDlg(CWnd* pParent);   // standard constructor
	~CWhatsNewHTMLDlg(); //destructor

// Member functions
public:
	BOOL DoModeless(BOOL bForceWhatsNew = FALSE);// (a.vengrofski 2009-11-02 14:38) - PLID <36104> - Added the bool to Force the DLG to show.
	bool NeedToDisplay();

//Member variables
private:
	bool m_bFileExists;
	COleDateTime m_dtOverrideTime;
protected:
	IWebBrowser2Ptr m_pBrowser;

// Dialog Data
	//{{AFX_DATA(CWhatsNewHTMLDlg)
	enum { IDD = IDD_WHATS_NEW_HTML_DLG };
	NxButton	m_btnDontShow;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnPrint;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWhatsNewHTMLDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWhatsNewHTMLDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnPrintBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WHATSNEWHTMLDLG_H__D447ED2E_8DA4_4BC3_A1D8_31E724FFDBD4__INCLUDED_)
