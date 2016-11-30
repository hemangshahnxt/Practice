#if !defined(AFX_TEMPLATEEXCEPTIONDLG_H__9EEF3639_B2DB_47E1_BB46_9B61176D7CAA__INCLUDED_)
#define AFX_TEMPLATEEXCEPTIONDLG_H__9EEF3639_B2DB_47E1_BB46_9B61176D7CAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TemplateExceptionDlg.h : header file
//
// (c.haag 2006-11-13 10:02) - PLID 5993 - Initial implementation
//
#include "SchedulerRc.h"

class CTemplateExceptionInfo;

/////////////////////////////////////////////////////////////////////////////
// CTemplateExceptionDlg dialog

class CTemplateExceptionDlg : public CNxDialog
{
protected:
	CTemplateExceptionInfo* m_pException;

	// (c.haag 2006-11-13 12:24) - For validation, we need a list of all exceptions
	// for the current template
	CArray<CTemplateExceptionInfo*, CTemplateExceptionInfo*> m_apTemplateExceptions;

	//TES 6/19/2010 - PLID 39262
	bool m_bUseResourceAvailTemplates;

// Construction
public:
	CTemplateExceptionDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 04/29/2008) - PLID 29814 - Added NxIconButtons
// Dialog Data
	// (a.walling 2008-05-13 10:39) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CTemplateExceptionDlg)
	enum { IDD = IDD_TEMPLATE_EXCEPTION_DLG };
	CString	m_strDescription;
	CDateTimePicker	m_dtpEndDate;
	CDateTimePicker	m_dtpStartDate;
	CNxEdit	m_nxeditEditExceptionDescription;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnDatesGroupbox;
	NxButton	m_btnExceptionTypeGroupbox;
	NxButton	m_btnDescGroupbox;
	NxButton	m_checkTopPriority;
	NxButton	m_checkIgnoreTemplate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTemplateExceptionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	//TES 6/19/2010 - PLID 39262 - Added a parameter for whether we're using the Resource Availability templates
	int ZoomException(bool bUseResourceAvailTemplates, CTemplateExceptionInfo *pException, CArray<CTemplateExceptionInfo*, CTemplateExceptionInfo*>& apTemplateExceptions);

protected:
	BOOL Load();
	BOOL Validate();
	BOOL Save();

protected:
	void SetFlags(long);
	long GetFlags();

protected:
	// Generated message map functions
	//{{AFX_MSG(CTemplateExceptionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEMPLATEEXCEPTIONDLG_H__9EEF3639_B2DB_47E1_BB46_9B61176D7CAA__INCLUDED_)
