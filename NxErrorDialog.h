// NxErrorDialog.h: interface for the CNxErrorDialog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NXERRORDIALOG_H__651CB9A3_7CD5_11D3_AD72_00104B318376__INCLUDED_)
#define AFX_NXERRORDIALOG_H__651CB9A3_7CD5_11D3_AD72_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (j.armen 2014-09-09 11:09) - PLID 63594 - Changed to an enum class
// (j.armen 2014-09-09 11:09) - PLID 63594 - Added API Warning, API Error
enum class ErrorLevel {
	WARNING = 0,		// Normal warning
	ROUTINE_ERROR = 20, // let them know something went wrong, but its something that we know can happend, and doesn't cause problems(ie record was deleted by another user)
	NETWORK_ERROR = 21, // a routine error that network related - unplugged network cable
	BUG_ERROR = 30,		// this is our fault, only use for things that should never happen
	CRITICAL_ERROR = 40,// This is a big deal, data could be corrupted, something bad happend, this needs special attention
	API_WARNING = 50,	// This is a warning from the API.  Should not be reported to Nextech
	API_ERROR = 51,		// This is an error from the API.  Should be allowed to report to Nextech
};

class CNxErrorDialog : public CDialog  
{
public:
	CNxErrorDialog();
	virtual ~CNxErrorDialog();
	virtual int DoModal(const CString &message,
						const CString &title, 
						const ErrorLevel errorLevel,
						OPTIONAL const CString &strBtnText_OK, 
						OPTIONAL const CString &strBtnText_Retry,
						OPTIONAL const CString &strBtnText_Cancel,
						OPTIONAL const CString &strManualLocation,
						OPTIONAL const CString &strManualBookmark);

	virtual BOOL Create(CWnd* pParent,
						const CString &message,
						const CString &title, 
						const ErrorLevel errorLevel,
						OPTIONAL const CString &strBtnText_OK, 
						OPTIONAL const CString &strBtnText_Retry,
						OPTIONAL const CString &strBtnText_Cancel,
						OPTIONAL const CString &strManualLocation,
						OPTIONAL const CString &strManualBookmark);

protected:
	// The buttons on the messagebox
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnRetry;
	CNxIconButton m_btnHelp;
	
	// The static label that contains the text of the messagebox
	CNxStatic	 m_text;

	///////
	// These are set by the DoModal (caller can pass the desired values via DoModal's parameters)
	///////
	// Text that appears to the user
	CString  m_messageText;			// The main text of the messagebox
	CString  m_titleText;			// The text appearing in the titlebar of the messagebox
	CString	 m_strBtnText_OK;		// The text that will appear on the OK button in the messagebox
	CString  m_strBtnText_Retry;	// The text that will appear on the Retry button in the messagebox
	CString  m_strBtnText_Cancel;	// The text that will appear on the Cancel button in the messagebox
	// Values used to call the help if necessary (if these are blank the help button won't show)
	CString  m_strManualLocation;	// The location in the help (usually an htm file)
	CString  m_strManualBookmark;	// The bookmark within that location
	// Value that controls the messagebox's background color and message font
	ErrorLevel m_errorLevel;
	///////

protected:
	BOOL m_bModal; // True if this is a modal window

protected:
	long l_cnButtonWidth;
	long l_cnButtonXBuffer;
	long l_cnButtonHeight;
	long l_cnButtonYBuffer;

protected:
	DWORD GetColor();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	CBrush  *m_pbrush;
	CFont	m_fntMessageBox;
	CFont   m_font;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnBtnHelp();
	afx_msg void OnBtnRetry();
	// (a.walling 2014-04-29 16:42) - PLID 61964
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_NXERRORDIALOG_H__651CB9A3_7CD5_11D3_AD72_00104B318376__INCLUDED_)
