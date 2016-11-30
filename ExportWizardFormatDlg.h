#if !defined(AFX_EXPORTWIZARDFORMATDLG_H__BABC0497_A2A5_489F_A5F9_4B6CB30A201E__INCLUDED_)
#define AFX_EXPORTWIZARDFORMATDLG_H__BABC0497_A2A5_489F_A5F9_4B6CB30A201E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportWizardFormatDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExportWizardFormatDlg dialog

class CExportWizardFormatDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CExportWizardFormatDlg)

// Construction
public:
	CExportWizardFormatDlg();
	~CExportWizardFormatDlg();

	// (j.jones 2008-05-08 09:19) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CExportWizardFormatDlg)
	enum { IDD = IDD_EXPORT_WIZARD_FORMAT };
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	CNxEdit	m_editRecordReplace;
	CNxEdit	m_editDelimiterReplace;
	CNxEdit	m_editSeparatorReplace;
	CNxEdit	m_editDelimiter;
	CNxEdit	m_editSeparator;
	CNxEdit	m_nxeditOtherSeparatorText;
	CNxStatic	m_nxstaticSpecialCharactersDescription;
	NxButton	m_btnExportOutputGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CExportWizardFormatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pSpecialChars;
	bool m_bEditingDatalist;
	// Generated message map functions
	//{{AFX_MSG(CExportWizardFormatDlg)
	afx_msg void OnCharacterSeparated();
	afx_msg void OnFixedWidth();
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	afx_msg void OnSelChangedSpecialChars(long nNewSel);
	afx_msg void OnEditingFinishingSpecialChars(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedSpecialChars(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAddSpecialChar();
	afx_msg void OnRemoveSpecialChar();
	afx_msg void OnReplaceSeparator();
	afx_msg void OnReplaceDelimiter();
	virtual BOOL OnKillActive();
	afx_msg void OnEditingStartingSpecialChars(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnCarriageReturn();
	afx_msg void OnLineFeed();
	afx_msg void OnOtherSeparator();
	afx_msg void OnPair();
	afx_msg void OnReplaceRecordSeparator();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTWIZARDFORMATDLG_H__BABC0497_A2A5_489F_A5F9_4B6CB30A201E__INCLUDED_)
