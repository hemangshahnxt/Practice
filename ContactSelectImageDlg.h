#if !defined(AFX_CONTACTSELECTIMAGEDLG_H__2DAFD6DA_AF24_44D3_9598_7D0A790E5859__INCLUDED_)
#define AFX_CONTACTSELECTIMAGEDLG_H__2DAFD6DA_AF24_44D3_9598_7D0A790E5859__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ContactSelectImageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CContactSelectImageDlg dialog

// (z.manning, 06/07/2007) - PLID 23862 - Created class to select an image for a provider.
class CContactSelectImageDlg : public CNxDialog
{
// Construction
public:
	CContactSelectImageDlg(long nProviderID, CWnd* pParent);

	// (z.manning, 06/07/2007) - PLID 23862 - Using a CMirrorImageButton because that's what the EMRItemEntryDlg uses.
// Dialog Data
	//{{AFX_DATA(CContactSelectImageDlg)
	enum { IDD = IDD_CONTACT_SELECT_IMAGE };
	CMirrorImageButton	m_btnImage;
	CNxIconButton	m_btnSelectImage;
	CNxIconButton	m_btnClearImage;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA

	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	BOOL Save();

	// (z.manning, 06/13/2007) - PLID 26255 - Moved GetContactImagePath to GlobalUtils.

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactSelectImageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (z.manning, 06/07/2007) - PLID 23862 - Force developers to use constructor with provider ID.
	CContactSelectImageDlg(CWnd* pParent);   // standard constructor

	long m_nProviderID;

	CString m_strImageFile;

	// Generated message map functions
	//{{AFX_MSG(CContactSelectImageDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectImage();
	afx_msg void OnClearImage();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTACTSELECTIMAGEDLG_H__2DAFD6DA_AF24_44D3_9598_7D0A790E5859__INCLUDED_)
