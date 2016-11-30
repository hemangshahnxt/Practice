// SupportExpiredDlg.h: interface for the CSupportExpiredDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SUPPORTEXPIREDDLG_H__9B68DA51_F755_48B8_B562_E8219731C9B3__INCLUDED_)
#define AFX_SUPPORTEXPIREDDLG_H__9B68DA51_F755_48B8_B562_E8219731C9B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SizeableTextDlg.h"

class CSupportExpiredDlg : public CSizeableTextDlg  
{
public:
	// Dialog Data
	//{{AFX_DATA(CDontShowDlg)
	enum { IDD = IDD_SUPPORT_EXPIRED };//	(a.vengrofski 2009-11-04 12:15) - PLID <36091> - Added new dialog.
	NxButton	m_btnDontRemind;
	CNxStatic	m_nxstaticTextEdit;
	CNxStatic	m_nxstaticTextEdit2;//(a.vengrofski 2009-11-05 10:28) - PLID <36091> - Added new dialog.
	CNxStatic	m_nxstaticTextEdit3;//(a.vengrofski 2009-11-05 10:28) - PLID <36091> - Added new dialog.
	CNxStatic	m_nxstaticTextEdit4;//(a.vengrofski 2009-11-05 10:28) - PLID <36091> - Added new dialog.
	CNxLabel	m_nxlstaticWebSite;	//(a.vengrofski 2009-11-04 17:49) - PLID <36091> - Added new dialog.
	CNxLabel	m_nxlstaticEmail;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnUpdate;
	CString		m_strTextEdit;	//(a.vengrofski 2009-11-05 12:19) - PLID <36091> - First text box string
	CString		m_strTextEdit2;	//(a.vengrofski 2009-11-05 12:19) - PLID <36091> - Second text box string
	CString		m_strTextEdit3;	//(a.vengrofski 2009-11-05 12:19) - PLID <36091> - Third text box string
	CString		m_strTextEdit4;	//(a.vengrofski 2009-11-05 12:19) - PLID <36091> - Fourth text box string

	//}}AFX_DATA

	CSupportExpiredDlg(CWnd* pParent);
	virtual ~CSupportExpiredDlg();

	//Shows the dialog as appropriate.
	void Show(IN const COleDateTime &dtExpirationDate);
	BOOL UserNeedsRemind();

	//This dialog needs access to a license, and g_pLicense may not be set yet.
	// (z.manning 2015-05-20 10:00) - PLID 65971 - Changed to CPracticeLicense
	CPracticeLicense* m_pLicense;

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSupportExpiredDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
protected:
	COleDateTime m_dtExpirationDate;

	// Generated message map functions
	//{{AFX_MSG(CSupportExpiredDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNo();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);// (a.vengrofski 2009-11-05 14:53) - PLID <36091> - HAndle the hover over links
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);// (a.vengrofski 2009-11-05 14:53) - PLID <36091> - Handle the clicking
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // !defined(AFX_SUPPORTEXPIREDDLG_H__9B68DA51_F755_48B8_B562_E8219731C9B3__INCLUDED_)
