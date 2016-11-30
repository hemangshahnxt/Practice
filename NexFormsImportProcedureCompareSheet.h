#if !defined(AFX_NEXFORMSIMPORTPROCEDURECOMPARESHEET_H__C8D63648_ABA3_4FEA_8286_BCD9337F024D__INCLUDED_)
#define AFX_NEXFORMSIMPORTPROCEDURECOMPARESHEET_H__C8D63648_ABA3_4FEA_8286_BCD9337F024D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportProcedureCompareSheet.h : header file
//

#import "RichTextEditor.tlb"
#include "NexFormsImportWizardSheet.h"

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportProcedureCompareSheet dialog

class CNexFormsImportProcedureCompareSheet : public CNexFormsImportWizardSheet
{
// Construction
public:
	CNexFormsImportProcedureCompareSheet(CNexFormsImportWizardMasterDlg* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNexFormsImportProcedureCompareSheet)
	enum { IDD = IDD_NEXFORMS_IMPORT_PROCEDURE_COMPARE_SHEET };
	NxButton	m_btnNeedReview;
	NxButton	m_btnOverwrite;
	CNxIconButton	m_btnPreviousField;
	CNxIconButton	m_btnNextField;
	CRichEditCtrl	m_ctrlRichEdit;
	CNxEdit	m_nxeditOldContentPlain;
	CNxEdit	m_nxeditNewContentPlain;
	//}}AFX_DATA

	virtual void Load();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportProcedureCompareSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlProcedures;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlFields;

	RICHTEXTEDITORLib::_DRichTextEditorPtr m_pOldRichText;
	RICHTEXTEDITORLib::_DRichTextEditorPtr m_pNewRichText;

	CString m_strSelecedProcedure;
	CString m_strSelectedField;

	void RefreshFieldMovementButtons();

	void SaveNewContent(LPDISPATCH lpFieldRow);

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportProcedureCompareSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedProcedureSelect(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChangedNexformsFieldSelect(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnPreviousNexformsField();
	afx_msg void OnNextNexformsField();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnOverwriteField();
	afx_msg void OnNeedsReview();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTPROCEDURECOMPARESHEET_H__C8D63648_ABA3_4FEA_8286_BCD9337F024D__INCLUDED_)
