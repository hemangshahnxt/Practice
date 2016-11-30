#if !defined(AFX_IMPORTWIZARDTYPEDLG_H__7A29DB01_A664_4AB4_BD7C_AC0E3C1DCAA4__INCLUDED_)
#define AFX_IMPORTWIZARDTYPEDLG_H__7A29DB01_A664_4AB4_BD7C_AC0E3C1DCAA4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportWizardTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportWizardTypeDlg dialog

#include "PracticeRc.h"

// (b.savon 2015-04-28 15:19) - PLID 65485 - Derive from our CNxPropertyPage
class CImportWizardTypeDlg  : public CNxPropertyPage
{
	DECLARE_DYNCREATE(CImportWizardTypeDlg)
// Construction
public:
	CImportWizardTypeDlg();   // standard constructor
	// (j.politis 2015-04-09 15:58) - PLID 65232 - Limit all note imports to a maximum of 100k records.
	// Made it more modular...BECAUSE Brad MADE ME DO IT!!!  Plus it was a good idea...
	BOOL IsFileValid(const CString &strFileName);
	BOOL DoesRecordTypeHaveFileLengthLimit();
	long GetFileLengthLimitForRecordType();
	BOOL IsFileLengthValid(CFile &fImportFile);
	CString GetFileLengthLimitWarningMessage(long nMaxLimit);

// Dialog Data
	//{{AFX_DATA(CImportWizardTypeDlg)
	enum { IDD = IDD_IMPORT_WIZARD_TYPE };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditImportFileName;
	CNxEdit	m_nxeditImportFieldDelimiter;
	CNxEdit	m_nxeditImportTextQualifier;
	CNxStatic	m_nxstaticImportDelimiterLabel;
	NxButton	m_btnImportTypeGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportWizardTypeDlg)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//(s.dhole 3/30/2015 11:02 AM ) - PLID 65224
	BOOL HandleRadioButtonEx(UINT RedioButtonId, ImportRecordType RecordType, EBuiltInObjectIDs oBuiltInObjectID, ESecurityPermissionType SecurityPermissionType);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	BOOL   m_bNeedInit;
	CSize  m_ClientSize;

	NXDATALIST2Lib::_DNxDataListPtr m_pFileType;

	void UpdateWizardButtons();
	void HandleRadioButton();
	BOOL GetImportFileName(CString &strFileName);
	void HandleSelectedFileType(long nFileType);

	// Generated message map functions
	//{{AFX_MSG(CImportWizardTypeDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	afx_msg void OnImportPatientFile();
	afx_msg void OnImportProviderFile();
	afx_msg void OnImportRefPhysFile();
	afx_msg void OnImportUserFile();
	afx_msg void OnImportSupplierFile();
	afx_msg void OnImportOtherContactFile();
	afx_msg void OnImportMedinotesFile();
	// (j.jones 2010-04-05 10:07) - PLID 16717 - supported service codes
	afx_msg void OnImportServiceCodeFile();
	afx_msg void OnImportResourceCodeFile(); // (r.farnworth 2015-03-16 16:52) - PLID 65197
	afx_msg void OnImportProductCodeFile(); // (r.farnworth 2015-03-19 09:17) - PLID 65238
	afx_msg void OnImportInsCoCodeFile(); // (r.farnworth 2015-04-01 12:45) - PLID 65166
	afx_msg void OnChangeImportFileName();
	afx_msg void OnImportFileBrowse();
	afx_msg void OnSelChangedImportFileType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnSelChosenImportFileType(LPDISPATCH lpRow);
	afx_msg void OnImportUseTextQualifier();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CImportWizardTypeDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	afx_msg void OnImportmportPatientNoteFile();
	afx_msg void OnBnClickedImportRecallFile();
	afx_msg void OnBnClickedImportApptsFile();
	afx_msg void OnBnClickedImportInsuredParties();
	// (j.politis 2015-04-27 14:12) - PLID 65524 - Allow the import wizard to be resizable
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedImportRacesFile();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTWIZARDTYPEDLG_H__7A29DB01_A664_4AB4_BD7C_AC0E3C1DCAA4__INCLUDED_)
