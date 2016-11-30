#if !defined(AFX_EMRTEXTMACRODLG_H__C6E2BE7F_B393_4963_9EF2_13C49C62B767__INCLUDED_)
#define AFX_EMRTEXTMACRODLG_H__C6E2BE7F_B393_4963_9EF2_13C49C62B767__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTextMacroDlg.h : header file
//
// (c.haag 2008-06-05 17:24) - PLID 26038 - Initial implementation
//
#include "EMRTextMacroBoxDlg.h"
#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CEmrTextMacroDlg dialog

class CEmrTextMacroDlg : public CNxDialog
{
// Construction
public:
	CEmrTextMacroDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrTextMacroDlg)
	enum { IDD = IDD_EMR_TEXT_MACRO };
	CNxEdit	m_nxeditDetailName;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnNewMacro;
	CNxIconButton	m_btnDeleteMacro;
	CNxIconButton	m_btnAddAndSave;
	CNxStatic	m_nxstaticPreview;
	CString	m_strDetailName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrTextMacroDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// The outputs of this dialog
	CString m_strResultName;
	CString m_strResultTextData;

protected:
	// Member variables used to compare with the form content to determine if
	// the form content has changed
	CString m_strOldDetailName;
	CString m_strOldTextData;

protected:
	// Used to have the dialog ignore EN_CHANGE-related messages when loading
	// form content
	BOOL m_bIgnoreEditChanges;

protected:
	BOOL IsNewRow(NXDATALIST2Lib::IRowSettingsPtr& pRow);
	BOOL IsContentEmpty(NXDATALIST2Lib::IRowSettingsPtr& pRow);
	void CreateAndSelectEmptyRow();
	void UpdateAppearance(NXDATALIST2Lib::IRowSettingsPtr& pRow);
	BOOL NeedToSave();
	BOOL ValidateContent(BOOL bCheckData); // (c.haag 2008-06-26 17:19) - Added a flag for querying data for validation
	void SaveContent();
	void DeleteMacro();
	void HandleListSelectionChange(NXDATALIST2Lib::IRowSettingsPtr& pRow);
	int SaveOrDiscardCurrentSelection();
	void UpdateButtons(NXDATALIST2Lib::IRowSettingsPtr& pSel);

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlMacroList;
	NXDATALIST2Lib::IRowSettingsPtr m_pRowToDelete;
	CEmrTextMacroBoxDlg m_wndPreview;

public:
	void OnOK();
	void OnCancel();

protected:
	// Generated message map functions
	//{{AFX_MSG(CEmrTextMacroDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellEmrTextMacroList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnAddAndSave();
	afx_msg void OnChangeEditDetailName();
	afx_msg void OnSelChangingEmrTextMacroList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg LRESULT OnChangePreviewEdit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBtnNewMacro();
	afx_msg void OnBtnDeleteMacro();
	afx_msg void OnRButtonUpEmrTextMacroList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnPopupDeleteMacro();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTEXTMACRODLG_H__C6E2BE7F_B393_4963_9EF2_13C49C62B767__INCLUDED_)
