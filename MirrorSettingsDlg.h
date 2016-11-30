#if !defined(AFX_MIRRORSETTINGSDLG_H__B8DFDA7D_2270_433F_B7BC_B1A6A0DD6D5B__INCLUDED_)
#define AFX_MIRRORSETTINGSDLG_H__B8DFDA7D_2270_433F_B7BC_B1A6A0DD6D5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MirrorSettingsDlg.h : header file
//
#include "PracticeRc.h"

/////////////////////////////////////////////////////////////////////////////
// CMirrorSettingsDlg dialog

class CMirrorSettingsDlg : public CNxDialog
{
// Construction
public:
	CMirrorSettingsDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_ProviderCombo;

// Dialog Data
	//{{AFX_DATA(CMirrorSettingsDlg)
	enum { IDD = IDD_MIRROR_SETTINGS };
	NxButton   m_btnNewPat;
	NxButton   m_btnLinkSSN;
	NxButton   m_btnLinkMRN;
	NxButton   m_btnImportOverwrite;
	NxButton   m_btnAssignProv;
	NxButton   m_btnNoImages;
	NxButton   m_btnFullColor;
	NxButton   m_btn256Color;
	NxButton   m_btnGrayscale;
	NxButton   m_btnHiRes;
	NxButton   m_btnDisable;
	NxButton   m_btnHideMirror;
	NxButton   m_btnAssignProvExist;
	NxButton   m_btnExportOverwrite;
	NxButton   m_btnForce60;
	BOOL	m_bAllowImportOverwrite;
	BOOL	m_bAllowExportOverwrite;
	BOOL	m_bAssignProvider;
	BOOL	m_bAssignProviderExisting;
	BOOL	m_bLinkPatIDToMRN;
	BOOL	m_bLinkPatIDToSSN;
	CString	m_strImagePath;
	CString	m_strDataPath;
	BOOL	m_bAutoExport;
	int		m_iImageDisplay;
	BOOL	m_bHiResImages;
	BOOL	m_bDisableLink;
	BOOL	m_bShowMirror;
	BOOL	m_bForceMirror60;
	CNxEdit	m_nxeditMirrorDataPath;
	CNxEdit	m_nxeditImageOverride;
	CNxStatic	m_nxstaticMirrordata;
	CNxStatic	m_nxstaticMirrorimages;
	NxButton	m_btnImageGroupbox;
	NxButton	m_btnMirror60Groupbox;
	NxButton	m_btnExportGroupbox;
	NxButton	m_btnVisibility;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	BOOL	m_bFullImageInPhotoViewerDlg;
	NxButton	m_btnFullImageInPhotoViewerDlg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Load();
	void Save();

	// Generated message map functions
	//{{AFX_MSG(CMirrorSettingsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnCheckAssignProvider();
	afx_msg void OnBtnMirrordataBrowse();
	afx_msg void OnBtnMirrorimageBrowse();
	afx_msg void OnCheckDisable();
	afx_msg void OnCheckAllowImportOverwrite();
	afx_msg void OnCheckForceMirror60();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIRRORSETTINGSDLG_H__B8DFDA7D_2270_433F_B7BC_B1A6A0DD6D5B__INCLUDED_)
