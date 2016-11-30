#if !defined(AFX_NEXFORMSIMPORTPROCEDURECOLUMNSSHEET_H__C4FEB0B3_9FDF_4CF0_A906_C674D8199A1D__INCLUDED_)
#define AFX_NEXFORMSIMPORTPROCEDURECOLUMNSSHEET_H__C4FEB0B3_9FDF_4CF0_A906_C674D8199A1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportProcedureColumnsSheet.h : header file
//

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
// CNexFormsImportProcedureColumnsSheet dialog

class CNexFormsImportProcedureColumnsSheet : public CNexFormsImportWizardSheet
{
// Construction
public:
	CNexFormsImportProcedureColumnsSheet(CNexFormsImportWizardMasterDlg* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNexFormsImportProcedureColumnsSheet)
	enum { IDD = IDD_NEXFORMS_IMPORT_PROCEDURE_COLUMNS_SHEET };
	CNxIconButton	m_btnSelectNothing;
	CNxIconButton	m_btnSelectEverything;
	//}}AFX_DATA

	virtual void Load();
	virtual BOOL Validate(); // (z.manning, 10/11/2007) - PLID 27719

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportProcedureColumnsSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pdlProcedureContent;
	NXDATALIST2Lib::_DNxDataListPtr m_pdlNexFormsContent;

	void HandleColumnClick(NXDATALIST2Lib::_DNxDataListPtr pdl, short nCol);

	void HandleProcedureContentChange(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void HandleNexFormsContentChange(NXDATALIST2Lib::IRowSettingsPtr pRow);

	void SelectAll(BOOL bSelect);

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportProcedureColumnsSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnColumnClickingNexformsContent(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnColumnClickingProcedureContent(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnEditingFinishedNexformsContent(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishedProcedureContent(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelectEverything();
	afx_msg void OnSelectNothing();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTPROCEDURECOLUMNSSHEET_H__C4FEB0B3_9FDF_4CF0_A906_C674D8199A1D__INCLUDED_)
