//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_QUOTENOTES_H__8ABC1941_D8BC_11D2_936C_00104BD3573F__INCLUDED_)
#define AFX_QUOTENOTES_H__8ABC1941_D8BC_11D2_936C_00104BD3573F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QuoteNotes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQuoteNotes dialog

class CQuoteNotes : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_NoteCombo;

	CString m_strSpecificNotes;
	CQuoteNotes(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQuoteNotes)
	enum { IDD = IDD_QUOTE_NOTES };
	CNxIconButton	m_cancelButton;
	CNxIconButton	m_adminButton;
	CNxIconButton	m_saveButton;
	CNxEdit			m_editNotes;
	CNxStatic	m_nxstaticLabel142;
	CNxStatic	m_nxstaticLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQuoteNotes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CQuoteNotes)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdministrator();
	afx_msg void OnSelChosenQuoteNotesCombo(long nRow);
	afx_msg void OnEditQuoteNotes();
	afx_msg void OnSave();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUOTENOTES_H__8ABC1941_D8BC_11D2_936C_00104BD3573F__INCLUDED_)
