#if !defined(AFX_EMRDEBUGDLG_H__B32DFD75_985B_456F_910F_490C570371A9__INCLUDED_)
#define AFX_EMRDEBUGDLG_H__B32DFD75_985B_456F_910F_490C570371A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrDebugDlg.h : header file
//
// (c.haag 2007-08-04 10:29) - PLID 26946 - This dialog is used for
// developers to look for discrepancies or data problems in EMN's.
//
// (c.haag 2007-09-17 12:51) - PLID 27406 - Added support for path
// length validation and a checkbox for comparing files
//
/////////////////////////////////////////////////////////////////////////////
// CEmrDebugDlg dialog

#include "PatientsRc.h"

class CEMR;
class CEMN;
class CEMRTopic;

class CEmrDebugDlg : public CNxDialog
{
private:
	// (c.haag 2007-09-17 10:19) - PLID 27401 - Defines the output method for reporting information
	typedef enum { eEMRDebugOutput_Console, eEMRDebugOutput_TextFile } EOutputMethod;

private:
	// (c.haag 2007-09-17 10:48) - PLID 27401 - Output file
	CStdioFile m_fileOutput;

// Construction
public:
	CEmrDebugDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrDebugDlg)
	enum { IDD = IDD_EMR_DEBUG };
	NxButton	m_btn8300;
	NxButton	m_btnConsole;
	NxButton	m_btnText;
	NxButton	m_btnNarrative;
	NxButton	m_btnTree;
	NxButton	m_btnDiff;
	NxButton	m_btnAllDetails;
	NxButton	m_btnIncludeMemAddress;
	NxButton	m_btnAllTopics;
	CString	m_strDiffFile;
	CString	m_strDiffApp;
	CString	m_strFilePath;
	CNxEdit	m_nxeditEditEmnName;
	CNxEdit	m_nxeditEditFilePath;
	CNxEdit	m_nxeditEditDiffFile;
	CNxEdit	m_nxeditEditDiffApp;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton	m_btnInfoGroupbox;
	NxButton	m_btnFiltersGroupbox;
	NxButton	m_btnOutputGroupbox;
	NxButton	m_btnComparisonsGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrDebugDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
	CEMR* m_pEMR; // The EMR we are looking at
	CEMN* m_pActiveEMN; // The active EMN the user has on the screen

private:
	// (c.haag 2007-09-17 13:26) - PLID 27408 - Added bAllowAddresses
	void DumpEMNTopicsRecurse(BOOL bOnlyInclude8300Fields, BOOL bAllowAddresses, CEMRTopic* pTopic);
	void DumpEMNTreeRecurse(CEMRTopic* pTopic, long nDepth);
	// (c.haag 2007-11-26 10:45) - PLID 28170 - Added support for reporting narrative field lists
	void DumpEMNNarrativeFields();

private:
	void UpdateButtonStates();

public:
	void SetEMR(CEMR* pEMR); // Sets the EMR we are looking at
	void SetActiveEMN(CEMN* pActiveEMN); // Sets the active EMN

private:
	// (c.haag 2007-09-17 10:17) - PLID 27401 - Returns the method in which we report information
	EOutputMethod GetOutputMethod();
	
public:
	// (c.haag 2007-09-17 10:21) - PLID 27401 - Actually does the outputting
	void Output(const CString&);

protected:

	// (z.manning, 05/16/2008) - PLID 30050 - Added OnCtlColor
	// Generated message map functions
	//{{AFX_MSG(CEmrDebugDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckAllDetails();
	afx_msg void OnCheckAllTopics();
	afx_msg void OnCheckTree();
	afx_msg void OnCheckConsole();
	afx_msg void OnCheckTextFile();
	afx_msg void OnCheckRunDiff();
	afx_msg void OnCheckNarrativeFieldLists();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRDEBUGDLG_H__B32DFD75_985B_456F_910F_490C570371A9__INCLUDED_)
