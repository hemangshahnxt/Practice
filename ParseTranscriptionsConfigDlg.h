#if !defined(AFX_PARSETRANSCRIPTIONSCONFIGDLG_H__0B604424_5390_4CDB_8456_019C27273601__INCLUDED_)
#define AFX_PARSETRANSCRIPTIONSCONFIGDLG_H__0B604424_5390_4CDB_8456_019C27273601__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParseTranscriptionsConfigDlg.h : header file
//

#include "ParseTranscriptionsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CParseTranscriptionsConfigDlg dialog
// (a.walling 2008-07-14 15:41) - PLID 30720 - Dialog to configure parsing options

class CParseTranscriptionsConfigDlg : public CNxDialog
{
// Construction
public:
	CParseTranscriptionsConfigDlg(const CParseTranscriptionsDlg::CParseOptions& opt, CWnd* pParent);   // standard constructor

	CParseTranscriptionsDlg::CParseOptions& GetParseOptions();

// Dialog Data
	//{{AFX_DATA(CParseTranscriptionsConfigDlg)
	enum { IDD = IDD_PARSE_TRANSCRIPTIONS_CONFIG_DLG };
	NxButton	m_nxbCreateWordDocs;
	NxButton	m_nxbIgnoreSegmentReview;	// (a.wilson 2012-10-15 14:21) - PLID 53129
	CNxIconButton	m_nxibReset;
	CNxEdit	m_editTitle;
	CNxEdit	m_editCat;
	CNxStatic	m_lblTitle;
	CNxStatic	m_lblCat;
	CNxStatic	m_lblPatID;
	NxButton	m_nxbGroup;
	CNxStatic	m_lblDate;
	CNxStatic	m_lblBreak;
	CNxEdit	m_editSection;
	CNxEdit	m_editDateLine;
	CNxEdit	m_editPatIDLine;
	CEdit	m_editInfo; // purposefully NOT an NxEdit
			 // - because it is readonly and does not need focus hints, which also can slow down things if the text is very long.
	NxButton	m_nxbBreakOnPatID;
	CNxIconButton	m_nxibOK;
	CNxIconButton	m_nxibCancel;
	CNxColor	m_nxcolor;
	// (a.wilson 2012-07-24 11:29) - PLID 51753 - new varaibles to control added options.
	NxButton m_radioHistory;
	NxButton m_radioEMN;
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplateCombo;
	// (a.wilson 2012-08-01 10:11) - PLID 51901 - add provider parsing.  edit control object.
	CNxEdit m_editProviderLine;
	// (j.jones 2013-02-28 13:05) - PLID 55123 - added ability to permit inactive providers
	NxButton	m_checkIncludeInactiveProviders;
	// (j.jones 2013-02-28 15:53) - PLID 55121 - added ability to force errors to be fixed before they can be imported
	NxButton	m_checkRequireErrorsFixed;
	// (j.jones 2013-03-01 09:38) - PLID 55122 - added default EMN status
	CNxStatic	m_nxstaticDefaultEMNStatusLabel;
	NXDATALIST2Lib::_DNxDataListPtr m_EMNStatusCombo;
	// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
	NxButton	m_checkMoveImportedFiles;
	CNxEdit		m_editPostImportFolder;
	CNxIconButton	m_btnSelectPostImportFolder;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParseTranscriptionsConfigDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Load();

	CParseTranscriptionsDlg::CParseOptions m_opt;

	// Generated message map functions
	//{{AFX_MSG(CParseTranscriptionsConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChkBreakAtPatidline();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnImportDestinationChanged();
	// (j.jones 2013-03-01 12:56) - PLID 55120 - added ability to move imported files to a folder
	afx_msg void OnCheckMoveImportedFiles();
	afx_msg void OnBtnSelectPostImportFolder();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARSETRANSCRIPTIONSCONFIGDLG_H__0B604424_5390_4CDB_8456_019C27273601__INCLUDED_)
