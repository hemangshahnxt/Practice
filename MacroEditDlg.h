#if !defined(AFX_MACROEDITDLG_H__035677D9_9C25_4586_A4C9_531CACEE3332__INCLUDED_)
#define AFX_MACROEDITDLG_H__035677D9_9C25_4586_A4C9_531CACEE3332__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MacroEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMacroEditDlg dialog

class CMacroEditDlg : public CNxDialog
{
// Construction
public:
	CMacroEditDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMacroEditDlg)
	enum { IDD = IDD_MACRO_EDIT };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditMacroDesc;
	CNxEdit	m_nxeditMacroNotes;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAddMacro;
	CNxIconButton m_btnDeleteMacro;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMacroEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_dlMacroCombo;
	NXDATALISTLib::_DNxDataListPtr m_dlCategoryCombo;
	long m_ID;

	void SaveCurrentMacro();
	bool CheckFields();
	void LoadCurrentlySelectedMacro();
	void EnableAppropriateFields();
	bool SomethingHasChanged();
	bool IsMacroDescValid(CString strDescToValidate, CString strOldDesc ="");

	// Generated message map functions
	//{{AFX_MSG(CMacroEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddMacro();
	virtual void OnOK();
	afx_msg void OnSelChosenMacroListCombo(long nRow);
	afx_msg void OnDeleteMacro();
	afx_msg void OnRequeryFinishedCategoryCombolist(short nFlags);
	afx_msg void OnKillfocusMacroDesc();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MACROEDITDLG_H__035677D9_9C25_4586_A4C9_531CACEE3332__INCLUDED_)
