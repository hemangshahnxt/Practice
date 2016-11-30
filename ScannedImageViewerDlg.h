#if !defined(AFX_SCANNEDIMAGEVIEWERDLG_H__553AB2A2_AC63_4F0E_A1A7_194A8DE92597__INCLUDED_)
#define AFX_SCANNEDIMAGEVIEWERDLG_H__553AB2A2_AC63_4F0E_A1A7_194A8DE92597__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScannedImageViewerDlg.h : header file
//

// (a.walling 2008-07-25 12:26) - PLID 30836 - Made this dialog modeless and resizable
/////////////////////////////////////////////////////////////////////////////
// CScannedImageViewerDlg dialog

class CScannedImageViewerDlg : public CNxDialog
{
// Construction
public:
	CScannedImageViewerDlg(CString strFileName, CWnd* pParent);   // standard constructor
	~CScannedImageViewerDlg();
	
	void SetImage(const CString& strPath);
	
// Dialog Data
	//{{AFX_DATA(CScannedImageViewerDlg)
	enum { IDD = IDD_SCANNED_IMAGE_VIEWER_DLG };
	CNxStatic	m_lblCaption;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA

	// (a.walling 2008-09-11 08:56) - PLID 31334 - Allow a mode that has a prompt and OK/Cancel buttons
	BOOL m_bPromptMode;
	CString m_strPrompt;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScannedImageViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FreeImage();
	void PrepareImage();

	CString m_strFilePath;
	HBITMAP m_hImage;

	// Generated message map functions
	//{{AFX_MSG(CScannedImageViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg void OnCancelDlg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCANNEDIMAGEVIEWERDLG_H__553AB2A2_AC63_4F0E_A1A7_194A8DE92597__INCLUDED_)
