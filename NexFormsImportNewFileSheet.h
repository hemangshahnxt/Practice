#if !defined(AFX_NEXFORMSIMPORTNEWFILESHEET_H__F5529169_973F_4DB3_A96B_D9C15E4D6EAC__INCLUDED_)
#define AFX_NEXFORMSIMPORTNEWFILESHEET_H__F5529169_973F_4DB3_A96B_D9C15E4D6EAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexFormsImportNewFileSheet.h : header file
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
// CNexFormsImportNewFileSheet dialog

class CNexFormsImportNewFileSheet : public CNexFormsImportWizardSheet
{
// Construction
public:
	CNexFormsImportNewFileSheet(CNexFormsImportWizardMasterDlg* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNexFormsImportNewFileSheet)
	enum { IDD = IDD_NEXFORMS_IMPORT_NEW_FILE_SHEET };
	NxButton	m_btnTrackingOnly;
	NxButton	m_btnNeither;
	NxButton	m_btnBoth;
	CNxIconButton	m_btnDownload;
	CNxIconButton	m_btnBrowse;
	CNxEdit	m_nxeditNexformsContentFilename;
	//}}AFX_DATA

	virtual BOOL Validate();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexFormsImportNewFileSheet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bNeedToLoadContent;

	// (z.manning, 07/19/2007) - PLID 26729 - Variable to store the file size if we're downloading a file for
	// progress bar purposes.
	DWORD m_dwCompressedFileSize;

	void EnableDownloadButtons(BOOL bEnable);

	// Generated message map functions
	//{{AFX_MSG(CNexFormsImportNewFileSheet)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseNexformsContentFile();
	afx_msg void OnDownloadNexformsContent(); // (z.manning, 07/31/2007) - PLID 25715
	afx_msg LRESULT OnTransferBegin(WPARAM wParam, LPARAM lParam); // (z.manning, 07/19/2007) - PLID 26729
	afx_msg LRESULT OnTransferProgress(WPARAM wParam, LPARAM lParam); // (z.manning, 07/19/2007) - PLID 26729
	afx_msg LRESULT OnTransferEnd(WPARAM wParam, LPARAM lParam); // (z.manning, 07/19/2007) - PLID 26729
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXFORMSIMPORTNEWFILESHEET_H__F5529169_973F_4DB3_A96B_D9C15E4D6EAC__INCLUDED_)
