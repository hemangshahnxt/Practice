#if !defined(AFX_NEXFORMSIMPORTWIZARDMASTERDLG_H__7F4A3640_979C_4F54_A823_337F481888B4__INCLUDED_)
#define AFX_NEXFORMSIMPORTWIZARDMASTERDLG_H__7F4A3640_979C_4F54_A823_337F481888B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportWizardMasterDlg.h : header file
//

#include "NexFormsUtils.h"
#include "NexFormsImportInfo.h"

/********************************************************************************/
/*																				*/
/* For documentation on the NexForms importer/exporter, see:					*/
/*																				*/
/*		http://192.168.1.2/developers/doku.php?id=nexforms_importer_exporter	*/
/*																				*/
/********************************************************************************/

// (z.manning, 08/30/2007) - PLID 18359 - Created a new importer for NexForms.
/////////////////////////////////////////////////////////////////////////////
// CNexFormsImportWizardMasterDlg dialog

class CNexFormsImportWizardSheet;

class CNexFormsImportWizardMasterDlg : public CNxDialog
{
// Construction
public:
	CNexFormsImportWizardMasterDlg(CWnd* pParent);   // standard constructor
	~CNexFormsImportWizardMasterDlg();

// Dialog Data
	//{{AFX_DATA(CNexFormsImportWizardMasterDlg)
	enum { IDD = IDD_NEXFORMS_IMPORT_WIZARD_MASTER };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnNext;
	CNxIconButton	m_btnImportNexForms;
	CNxIconButton	m_btnBack;
	NxButton	m_btnSheetPlaceholder;
	//}}AFX_DATA

	NexFormsImportInfo m_ImportInfo;

	// (z.manning, 10/09/2007) - PLID 27706 - Map to easily lookup custom field names.
	CMap<CString,LPCTSTR,CString,LPCTSTR> m_mapCustomFieldNames;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportWizardMasterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CArray<CNexFormsImportWizardSheet*, CNexFormsImportWizardSheet*> m_arypWizardSheets;

	long m_nActiveSheetIndex;

	void SetActiveSheet(int nSheetIndex);

	void RefreshButtons();

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportWizardMasterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNext();
	afx_msg void OnBack();
	afx_msg void OnImportNexforms();
	afx_msg void OnDestroy();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTWIZARDMASTERDLG_H__7F4A3640_979C_4F54_A823_337F481888B4__INCLUDED_)
