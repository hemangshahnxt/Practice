#if !defined(AFX_DONTSHOWDLG_H__923D0123_ACE3_11D3_A3BE_00C04F42E33B__INCLUDED_)
#define AFX_DONTSHOWDLG_H__923D0123_ACE3_11D3_A3BE_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DontShowDlg.h : header file
//

#include "SizeableTextDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CDontShowDlg dialog

class CDontShowDlg : public CSizeableTextDlg
{
	// strText is the message you want to display
	// strName is the property name to be used for ConfigRT and the registry
	// strTitle is optional, the title to be used for the dialog
	//DRT 6/9/2008 - PLID 30331 - Added an option for a YesNoCancel
	friend int DontShowMeAgain(CWnd* pParent, const CString &strText, const CString &strName, const CString &strTitle = "", BOOL bIgnoreUser = FALSE, BOOL bUseYesNo = FALSE, BOOL bUseCancelWithYesNo = FALSE);

	// (a.walling 2007-02-26 15:57) - PLID 24421 - DontShowMeAgains need to support a parent window
	//	so they can be used in modeless dialogs.
	//TES 8/12/2009 - PLID 35201 - Added an overload that returns a BOOL for whether the "Don't Show Me Again" box was
	// checked, rather than a property name.
	friend int DontShowMeAgain(CWnd* pParent, const CString &strText, OUT BOOL &bDontShowChecked, const CString &strTitle = "", BOOL bIgnoreUser = FALSE, BOOL bUseYesNo = FALSE);

	// strText is the message you want to display
	// strName is the property name to be used for ConfigRT and the registry
	// strFilePath is the path to the file whose date you want to compare to when deciding whether to show the message or not
	// strTitle is optional, the title to be used for the dialog
	friend int DontShowMeAgainByFileDate(CWnd* pParent, const CString &strText, const CString &strName, const CString &strFilePath, const CString &strTitle = "", BOOL bIgnoreUser = FALSE, BOOL bUseYesNo = FALSE);

// Construction
public:
	CDontShowDlg(CWnd* pParent);   // standard constructor

public:
	// strText is the message you want to display
	// strName is the property name to be used for ConfigRT and the registry
	// strTitle is optional, the title to be used for the dialog
	//DRT 6/9/2008 - PLID 30331 - Added an option for a YesNoCancel
	int DoModal(const CString &strText, const CString &strName, const CString &strTitle = "", const BOOL bUseYesNo = FALSE, BOOL bUseCancelWithYesNo = FALSE);
	//TES 8/12/2009 - PLID 35201 - Added an overload that returns a BOOL for whether the "Don't Show Me Again" box was
	// checked, rather than a property name.
	int DoModal(const CString &strText, OUT BOOL &bDontShowChecked, const CString &strTitle = "", const BOOL bUseYesNo = FALSE, BOOL bUseCancelWithYesNo = FALSE);
	virtual BOOL Init();

public:
// Dialog Data
	//{{AFX_DATA(CDontShowDlg)
	enum { IDD = IDD_DONT_SHOW_ME };
	NxButton	m_btnDontShow;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnYes;
	CNxIconButton	m_btnNo;
	CNxStatic	m_nxstaticTextEdit;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDontShowDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strByFileDate;
	BOOL m_bUseYesNo;
	//DRT 7/29/2008 - PLID 30331 - This is only allowed to be used if you also use YesNo.  Don't set bUseYesNo to false
	//	and this to true.
	BOOL m_bUseCancelWithYesNo;

	//TES 8/12/2009 - PLID 35201 - Store whether the caller is handling the "Don't Show Me Again" functionality.
	BOOL m_bIgnoreDontShow;

	// Generated message map functions
	//{{AFX_MSG(CDontShowDlg)
		//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DONTSHOWDLG_H__923D0123_ACE3_11D3_A3BE_00C04F42E33B__INCLUDED_)
