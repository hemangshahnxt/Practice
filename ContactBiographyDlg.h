#if !defined(AFX_CONTACTBIOGRAPHYDLG_H__8C61B6FC_379C_403C_A2AF_0D1B796C70CF__INCLUDED_)
#define AFX_CONTACTBIOGRAPHYDLG_H__8C61B6FC_379C_403C_A2AF_0D1B796C70CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContactBiographyDlg.h : header file
//

#import "RichTextEditor.tlb"

/////////////////////////////////////////////////////////////////////////////
// CContactBiographyDlg dialog

// (z.manning, 06/07/2007) - PLID 23862 - Created dialog to edit a provider's biography text.
class CContactBiographyDlg : public CNxDialog
{
// Construction
public:
	CContactBiographyDlg(long nProviderID, CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CContactBiographyDlg)
	enum { IDD = IDD_CONTACT_BIOGRAPHY };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	BOOL Save();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactBiographyDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (z.manning, 06/07/2007) - PLID 23862 - Force users to construct this dialog with a provider ID.
	CContactBiographyDlg(CWnd* pParent);   // standard constructor

	RICHTEXTEDITORLib::_DRichTextEditorPtr m_RichEditCtrl;

	long m_nProviderID;

	// Generated message map functions
	//{{AFX_MSG(CContactBiographyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTBIOGRAPHYDLG_H__8C61B6FC_379C_403C_A2AF_0D1B796C70CF__INCLUDED_)
