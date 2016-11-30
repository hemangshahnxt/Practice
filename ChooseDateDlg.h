#if !defined(AFX_CHOOSEDATEDLG_H__4667BAD1_73E0_43C4_BCA1_D243BA4AF834__INCLUDED_)
#define AFX_CHOOSEDATEDLG_H__4667BAD1_73E0_43C4_BCA1_D243BA4AF834__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseDateDlg.h : header file
//
#include "practicerc.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseDateDlg dialog
//
// (c.haag 2006-11-20 09:21) - PLID 23589 - This dialog was originally
// created to let users choose a template line item exception date
//

class CChooseDateDlg : public CNxDialog
{
protected:
	COleDateTime m_dtDate;

// Construction
public:
	CChooseDateDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseDateDlg)
	enum { IDD = IDD_CHOOSE_DATE_DLG };
	CDateTimePicker	m_dtpDate;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseDateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	COleDateTime Open(const COleDateTime& dtDefaultDate);

protected:

	// Generated message map functions
	//{{AFX_MSG(CChooseDateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEDATEDLG_H__4667BAD1_73E0_43C4_BCA1_D243BA4AF834__INCLUDED_)
