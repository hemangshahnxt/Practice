#if !defined(AFX_PROCEDURESECTIONEDITDLG_H__782701D1_C9FA_45FE_AB66_22F48F0C8974__INCLUDED_)
#define AFX_PROCEDURESECTIONEDITDLG_H__782701D1_C9FA_45FE_AB66_22F48F0C8974__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureSectionEditDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcedureSectionEditDlg dialog
#include "NxRichEditCtrl.h"

struct ProcSection {
	CString strDisplayName;
	CString strFieldName;
	CString strText;
	bool bChanged;
	long nCustomFieldID;
	//DRT 7/7/2004 - PLID 13349 - Review button
	CString strReviewFieldName;
	BOOL bNeedsReview;
};

class CProcedureSectionEditDlg : public CNxDialog
{
// Construction
public:
	// (d.moore 2007-06-22 09:00) - PLID 23861 - Added the ability to pass in a procedure ID.
	CProcedureSectionEditDlg(CWnd* pParent = NULL, long nProcedureID = -1);   // standard constructor
	~CProcedureSectionEditDlg();

	// (d.moore 2007-07-02 09:37) - PLID 23863 - Pass in a Procedure ID from outside.
	//  Will update the procedure selection and data if the dialog is initialized.
	void SetProcedure(long nID);

	// (z.manning, 11/29/2007) - PLID 28232 - The formatting buttons are push-buttons, which is not supported
	// by the NxIconButton class at this point.
// Dialog Data
	//{{AFX_DATA(CProcedureSectionEditDlg)
	enum { IDD = IDD_PROCEDURE_SECTION_DLG };
	CNxIconButton	m_btnSpellCheck;
	CNxIconButton	m_btnPreviewItem;
	CNxIconButton	m_btnCopySections;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnNext;
	CNxIconButton	m_btnBack;
	CButton	m_btnItalic;
	CButton	m_btnSuperScript;
	CButton	m_btnSubScript;
	CButton	m_btnBullets;
	CButton	m_btnAlignRight;
	CButton	m_btnAlignCenter;
	CButton	m_btnAlignLeft;
	CButton	m_btnUnderline;
	CButton	m_btnBold;
	CNxIconButton	m_btnUndo;
	CNxRichEditCtrl	m_richeditSectionText;
	CNxEdit	m_nxeditProcSectionFontHeight;
	CNxStatic	m_nxstaticProcedure;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnFilterWithContent;
	NxButton	m_btnFilterNeedReviewed;
	NxButton	m_btnNeedsReviewed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureSectionEditDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (d.moore 2007-06-19 16:55) - PLID 23861 - Modified m_dlSectionCombo to a NxDataList2.
	NXDATALIST2Lib::_DNxDataListPtr m_dlSectionCombo;
	NXDATALISTLib::_DNxDataListPtr m_pFontList;
	NXDATALISTLib::_DNxDataListPtr m_pColorList;
	// (d.moore 2007-06-19 11:01) - PLID 23861 - Adding the ability to select procedures from within this dialog.
	NXDATALIST2Lib::_DNxDataListPtr m_pProcedureList;

	bool m_bInitializing;
	// (d.moore 2007-06-21 16:34) - PLID 23861 - Need to block certain functionality when moving between procedures or sections.
	bool m_bNavigating;
	// (d.moore 2007-07-05 11:06) - PLID 23861 - We need to track when the 'Needs Reviewed' checkbox is clicked on. This
	//  is not the same thing as the section having its bNeedsReview flag set. What we need to test for is if the sections
	//  bNeedsReview flag is set, but the user did not set it during this viewing.
	bool m_bNeedsReviewedWasSet;

	ProcSection *m_arSections;

	void InitSectionArray();

	long m_nProcedureID;
	long m_nCurrentSection;

protected:
	BOOL Save();
	BOOL ValidateAndSave();

	void LoadSectionList();
	// (d.moore 2007-06-21 17:10) - PLID 23861 - Loads text and other attributes for a given section.
	void LoadDataForSection(long nIndex);
	// (d.moore 2007-06-20 17:07) - PLID 23861 - Load the category dropdown with correct filters.
	void LoadProcedures();
	// (d.moore 2007-06-21 14:16) - PLID 13861 - Enable or disable all controls not related to navigation.
	void EnableControls(BOOL bState);
	// (d.moore 2007-06-21 14:16) - PLID 13861 - These functions are just to make selecting the right section
	//  or procedure easier and clearer in the code. They return the index for the section of procedure that actually got set.
	long SetProcedureSelection(long nID);
	long SetProcedureSelectionToFirst();
	long SetProcedureSelectionToLast();
	long SetSectionSelection(long nIndex);
	long SetSectionSelectionToFirst();
	long SetSectionSelectionToLast();
	// (d.moore 2007-06-27 17:47) - PLID 13861 - Store the text for a section in the m_arSections.
	void StoreSectionText();

	//Sets the formatting controls (font, boldness, etc.) to reflect the currently selected text.
	void SyncControlsWithText();
	void ColorSectionRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (d.moore 2007-06-20 14:16) - PLID 23861 - Color all rows marked 'Needs Reviewed'.
	void ColorSectionMarkedRows();
	// (d.moore 2007-06-21 16:47) - PLID 23861 - Make sure that next and back buttons are properly enabled.
	void EnsureNextBackButtonState();

	// (z.manning, 08/03/2007) - PLID 18359 - Move the functions to add & remove the need review text to  NexFormsUtils.

	// (d.moore 2007-07-05 11:23) - PLID 23863 - Check to see if the user wants to remove the 'Needs Reviewed' setting for the section.
	void PromptRemoveNeedsReviewed(long nSectionIndex);

	// (d.moore 2007-09-26) - PLID 23863 - When switching between Procedures lets prompt them that the data is about
	//  to be saved. This should eliminate some confusion.
	BOOL PromptForSaveOnProcedureChange();

	void PreviewNexFormsReport(BOOL bShowCurrentFieldOnly);
	
protected:

	// (z.manning, 05/02/2008) - PLID 29867 - Added OnCancel handler
	// Generated message map functions
	//{{AFX_MSG(CProcedureSectionEditDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSectionEdit();
	afx_msg void OnRenameBtn();
	afx_msg void OnCopySections();
	afx_msg void OnSelchangeSectionEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnProcSectionBold();
	afx_msg void OnProcSectionItalic();
	afx_msg void OnProcSectionUnderline();
	afx_msg void OnSelChosenProcSectionFontList(long nRow);
	afx_msg void OnChangeProcSectionFontHeight();
	afx_msg void OnKillfocusProcSectionFontHeight();
	afx_msg void OnDeltaposFontHeightSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenProcSectionColor(long nRow);
	afx_msg void OnUndo();
	afx_msg void OnProcSectionAlignCenter();
	afx_msg void OnProcSectionAlignLeft();
	afx_msg void OnProcSectionAlignRight();
	afx_msg void OnProcSectionBullets();
	afx_msg void OnRclickSectionEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCut();
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnProcSectionSubscript();
	afx_msg void OnProcSectionSuperscript();
	afx_msg void OnNeedsReviewed();
	afx_msg void OnPreviewItem();
	afx_msg void OnPreviewField();
	afx_msg void OnPreviewProc();
	afx_msg void OnButtonBack();
	afx_msg void OnButtonNext();
	afx_msg void OnSelChosenProcedureContentsList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedProcedureContentsList(short nFlags);
	afx_msg void OnSelChosenCustomSectionCombo(LPDISPATCH lpRow);
	afx_msg void OnCheckFilterNeedReviewed();
	afx_msg void OnCheckFilterSectionsWithContent();
	afx_msg void OnClose();
	afx_msg void OnProcSpellCheck();
	afx_msg void OnProcedureSectionMenuUpdateFont(); // (z.manning, 10/11/2007) - PLID 27719
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDURESECTIONEDITDLG_H__782701D1_C9FA_45FE_AB66_22F48F0C8974__INCLUDED_)
