// SizeableTextDlg.h : header file
//

#if !defined(AFX_SIZEABLETEXT_H__72BFB617_F73D_11D2_A820_00C04F4C852A__INCLUDED_)
#define AFX_SIZEABLETEXT_H__72BFB617_F73D_11D2_A820_00C04F4C852A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PracticeRc.h"

/////////////////////////////////////////////////////////////////////////////
// CSizeableTextDlg dialog

class CSizeableTextDlg : public CNxDialog
{
// Construction
public:
	CSizeableTextDlg(CWnd* pParentWnd = NULL);
	CSizeableTextDlg(UINT nIDTemplate, CWnd* pParentWnd = NULL); // Used for derived classes

// Member functions
public:
	virtual BOOL Init();

	BOOL DoModeless();
	int DoModal(); // Derived from CDialog

public: 
	// These enum values are to be ORed together in the m_nStyle member
	// Choose one from each Enum
	enum EnumMessageStyle {
		msDynamicFontText				= 0x0000, // Default
		msFixedFontText					= 0x0001,
		msMsgBoxText					= 0x0002,
	};
	enum EnumTextSource {
		tsFromString					= 0x0000, // Default
		tsFromFile						= 0x0004,
	};
	enum EnumDontRemindStyle {
		drsDontRemindBool				= 0x0000, // Default
		drsDontRemindUntil				= 0x0008,
		drsDontRemindUntilFileChange	= 0x0010, 
		drsIgnoreDontRemind				= 0x0020,
	};

	enum EnumButtonStyle {
		bsOKOnly						= 0x0040,  
		bsYesNoOnly						= 0x0080,
		bsOKPrint						= 0x0100,
		bsYesNoPrint					= 0x0200,
		bsPrintOnly						= 0x0400,
		bsYesNoCancel					= 0x0800,	//DRT 6/9/2008 - PLID 30331 - Added cancel option
	};


public:
	BOOL m_nStyle; // OR together one or more of the enum values above
	CString m_strCurUser; // Set this to the name of the desired user; leave blank to use the true CurrentUser (calls GetCurrentUser())
	CString m_strTextOrPath; // Either the actual text to display, or the path to the text file to load from
	CString m_strName; // A short clean name to be used in ConfigRT for DontShow stuff
	CString m_strTitle; // Whatever you want to be displayed in the title bar.  If empy, the app's name will be used
	COleDateTime m_dtOverrideCurrentTime; // Ignore this if you just want to use the current system time for the drsDontReminUntil compare
	COleDateTime m_dtNextTime; // This is what the times are compared to.  If "current time" is >= m_dtNextTime and "last time" is < m_dtNexTime then we show the dialog.  If the user checks "don't remind me again", then the "last time" gets set to "current time".

public:
// Dialog Data
	//{{AFX_DATA(CSizeableTextDlg)
	enum { IDD = IDD_SIZEABLE_TEXT_DLG };
	CNxEdit	m_nxeditTextEdit;
	CNxIconButton	m_btnPrint;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSizeableTextDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CSize m_szTextEdit;
	CPoint m_ptDontShowCheck; // (b.cardillo 2005-05-02 14:52) - PLID 15477 - .x is the width of the checkbox, so it can be horizontally centered when dialog is sized; .y is the distance from the bottom, so it can be position when dialog is sized.
	CSize m_szOKBtn;
	CSize m_szPrintBtn;
	CSize m_szYesBtn;
	CSize m_szNoBtn;
	CSize m_szCancelBtn;		//DRT 6/9/2008 - PLID 30331 - Added cancel option

protected:
	virtual BOOL UserNeedsRemind();

	//TES 8/12/2009 - PLID 35201 - Stores whether the "Don't Show Me Again" box is checked, so that we can return
	// that information to the caller.
	BOOL m_bDontShowChecked;

	// (z.manning, 05/16/2008) - PLID 30050 - Added OnCtlColor
	// Generated message map functions
	//{{AFX_MSG(CSizeableTextDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPrintBtn();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNo();
	afx_msg void OnYes();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIZEABLETEXT_H__72BFB617_F73D_11D2_A820_00C04F4C852A__INCLUDED_)
