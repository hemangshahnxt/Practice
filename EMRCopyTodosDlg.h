#if !defined(AFX_EMRCOPYTODOSDLG_H__4651FA2C_161A_425A_9F7B_73461FDF094A__INCLUDED_)
#define AFX_EMRCOPYTODOSDLG_H__4651FA2C_161A_425A_9F7B_73461FDF094A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRCopyTodosDlg.h : header file
//
// (c.haag 2008-07-16 11:54) - PLID 30752 - Initial implementation
//
#include "patientsrc.h"

/////////////////////////////////////////////////////////////////////////////
// CEMRCopyTodosDlg dialog

class CEMRCopyTodosDlg : public CNxDialog
{
// Construction
public:
	CEMRCopyTodosDlg(CArray<long,long>& anTodoIDs, CStringArray& astrTodoDetailNames, long nSourceEMNID, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRCopyTodosDlg)
	enum { IDD = IDD_EMR_COPY_TODOS };
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRCopyTodosDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CArray<long,long>& m_anTodoIDs;
	CStringArray& m_astrTodoDetailNames;
	long m_nSourceEMNID;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlList;

protected:

	// Generated message map functions
	//{{AFX_MSG(CEMRCopyTodosDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRCOPYTODOSDLG_H__4651FA2C_161A_425A_9F7B_73461FDF094A__INCLUDED_)
