#if !defined(AFX_LOCATIONBIOGRAPHYDLG_H__7058B616_109F_4FD8_9C67_8A2BB9272A70__INCLUDED_)
#define AFX_LOCATIONBIOGRAPHYDLG_H__7058B616_109F_4FD8_9C67_8A2BB9272A70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocationBiographyDlg.h : header file
//

#import "RichTextEditor.tlb"

/////////////////////////////////////////////////////////////////////////////
// CLocationBiographyDlg dialog

// (d.moore 2007-07-18 17:35) - PLID 14799 - Created a dialog to add biography text to locations.

class CLocationBiographyDlg : public CNxDialog
{
// Construction
public:
	
	CLocationBiographyDlg(long nLocationID, CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CLocationBiographyDlg)
	enum { IDD = IDD_LOCATION_BIOGRAPHY };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	BOOL Save();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocationBiographyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CLocationBiographyDlg(CWnd* pParent);   // standard constructor

	RICHTEXTEDITORLib::_DRichTextEditorPtr m_RichEditCtrl;

	long m_nLocationID;

	// Generated message map functions
	//{{AFX_MSG(CLocationBiographyDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATIONBIOGRAPHYDLG_H__7058B616_109F_4FD8_9C67_8A2BB9272A70__INCLUDED_)
