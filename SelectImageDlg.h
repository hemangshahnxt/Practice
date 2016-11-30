#if !defined(AFX_SELECTIMAGEDLG_H__C2C79DFE_16F5_44BA_9A99_8EF611CBC702__INCLUDED_)
#define AFX_SELECTIMAGEDLG_H__C2C79DFE_16F5_44BA_9A99_8EF611CBC702__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectImageDlg.h : header file
//
#include "PhotoViewerCtrl.h"
/////////////////////////////////////////////////////////////////////////////
// CSelectImageDlg dialog
//Written to data, DON'T TOUCH!!!!!
enum eImageType {
	itUndefined = -1,	// (c.haag 2007-02-09 15:36) - PLID 23365 - itUndefined 
						// means there is no image at all. This won't break existing
						// data because none of the other ordinals were changed.
	itAbsolutePath = 0,
	itPatientDocument = 1,
	itDiagram = 2,
	itMirrorImage = 3,
	itSignature = 4, // (z.manning 2008-11-03 09:52) - PLID 31890
	itForcedBlank = 5,	// (d.thompson 2009-03-05 09:45) - PLID 32891
};

class CSelectImageDlg : public CNxDialog
{
// Construction
public:
	CSelectImageDlg(CWnd* pParent);   // standard constructor
	virtual ~CSelectImageDlg();
	long m_nPatientID;
	// (j.jones 2013-09-19 15:19) - PLID 58547 - added a pointer to the pic
	class CPicContainerDlg* m_pPicContainer;
	WORD m_wCloseCommand;
	// (z.manning, 06/07/2007) - PLID 23862 - We can now specify differnt image paths.
	CString m_strImagePath;
	// (z.manning, 06/07/2007) - PLID 23862 - Can also specify the name of the main tab.
	CString m_strMainTabName;

	//"Return" values.
	CString m_strFileName;
	eImageType m_nImageType;

// Dialog Data
	//{{AFX_DATA(CSelectImageDlg)
	enum { IDD = IDD_SELECT_IMAGE_DLG };
	CPhotoViewerCtrl	m_PatientPhotoViewer;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic	m_nxstaticPatientPhotos;
	NxButton m_btnShowInterface;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectImageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
	NxTab::_DNxTabPtr m_tab;

	// (a.walling 2008-09-23 12:58) - PLID 31486 - Import from TWAIN or WIA
	void ImportFromTWAIN();
	void ImportFromWIA();

	// (a.walling 2008-09-23 11:22) - PLID 31479 - This is really not necessary
	//HANDLE m_hevShuttingDown;

	// Generated message map functions
	//{{AFX_MSG(CSelectImageDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelectTabImageType(short newTab, short oldTab);
	afx_msg void OnTwainImport();
	afx_msg LRESULT OnTwainXferdone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowInterface();
	afx_msg void OnImportFile();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg LRESULT OnPhotoLoaded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTIMAGEDLG_H__C2C79DFE_16F5_44BA_9A99_8EF611CBC702__INCLUDED_)
