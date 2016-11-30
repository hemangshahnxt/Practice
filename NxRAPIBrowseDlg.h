#if !defined(AFX_NXRAPIBROWSEDLG_H__FCAF5703_71A0_41AD_8ADF_E17DAE6E339A__INCLUDED_)
#define AFX_NXRAPIBROWSEDLG_H__FCAF5703_71A0_41AD_8ADF_E17DAE6E339A__INCLUDED_

#include "practicerc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxRAPIBrowseDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxRAPIBrowseDlg dialog

class CNxRAPIBrowseDlg : public CNxDialog
{
// Construction
public:
	CString GetPathName();
	int GetIconIndex(CString strFPath, DWORD dwFileAttr);
	CNxRAPIBrowseDlg(class CNxRAPI* pRAPI, BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL, DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);

// Dialog Data
	//{{AFX_DATA(CNxRAPIBrowseDlg)
	enum { IDD = IDD_RAPI_BROWSE };
	CComboBox	m_cmbFileMask;
	CListCtrl	m_List;
	CString	m_strSelectedFileName;
	CString	m_strCurrentPath;
	CNxEdit	m_nxeditRapiBrowseEditFilename;
	CNxEdit	m_nxeditEditPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxRAPIBrowseDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void PopulateFileList();

	CImageList		m_SmallImgList;		// System small image list
	class CNxRAPI*  m_pRAPI;

	// Generated message map functions
	//{{AFX_MSG(CNxRAPIBrowseDlg)
	afx_msg void OnBtnOpen();
	afx_msg void OnBtnUp();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnItemchangedRapiBrowseList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkRapiBrowseList(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnRclickRapiBrowseList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRClickOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXRAPIBROWSEDLG_H__FCAF5703_71A0_41AD_8ADF_E17DAE6E339A__INCLUDED_)
