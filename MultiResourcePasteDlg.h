//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_MULTIRESOURCEPASTEDLG_H__129527E9_6A3B_473E_968E_22AF7D23E498__INCLUDED_)
#define AFX_MULTIRESOURCEPASTEDLG_H__129527E9_6A3B_473E_968E_22AF7D23E498__INCLUDED_

#include "Schedulerrc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiResourcePasteDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiResourcePasteDlg dialog

class CMultiResourcePasteDlg : public CNxDialog
{
// Construction
public:
	CDWordArray m_adwSelected;
	// (a.walling 2015-01-29 09:05) - PLID 64413 - Override title and text
	CMultiResourcePasteDlg(CWnd* pParent, CString overrideTitle = CString(), CString overrideText = CString());   // standard constructor

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CMultiResourcePasteDlg)
	enum { IDD = IDD_MULTIRESOURCE_PASTE };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiResourcePasteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlList;

	CString m_overrideText;
	CString m_overrideTitle;

	// Generated message map functions
	//{{AFX_MSG(CMultiResourcePasteDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIRESOURCEPASTEDLG_H__129527E9_6A3B_473E_968E_22AF7D23E498__INCLUDED_)
