#if !defined(AFX_NEXWEBEDITOBJECTSDLG_H__D92B8AE2_B2EC_4A8A_80B8_9DA3F9A72BFA__INCLUDED_)
#define AFX_NEXWEBEDITOBJECTSDLG_H__D92B8AE2_B2EC_4A8A_80B8_9DA3F9A72BFA__INCLUDED_


#include "NexWebPatientInfoDlg.h"
#include "NexWebExtraPatientInfoDlg.h"
#include "NexWebApptListDlg.h"
#include "NexWebPatientNotesDlg.h"
#include "NexWebToDoTaskDlg.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebEditObjectsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNexWebEditObjectsDlg dialog

class CNexWebEditObjectsDlg : public CNxDialog
{
// Construction
public:
	CNexWebEditObjectsDlg(long nPersonID, BOOL bIsNewPatient, CWnd* pParent);   // standard constructor
	// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - Need to specify namespace
	NxTab::_DNxTabPtr m_tab;
	CNexWebPatientInfoDlg m_PatientInfoDlg;
	CNexWebExtraPatientInfoDlg m_ExtraPatientInfoDlg;
	CNexWebApptListDlg m_ApptListDlg;
	CNexWebPatientNotesDlg m_PatientNotesDlg;
	CNexWebToDoTaskDlg m_ToDoTaskDlg;
	long GetPersonID();
	

// Dialog Data
	//{{AFX_DATA(CNexWebEditObjectsDlg)
	enum { IDD = IDD_NEXWEB_EDIT_DLG };
	CNxIconButton	m_Preview;
	CNxIconButton	m_Cancel;
	CNxIconButton	m_Delete;
	CNxIconButton	m_Import;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebEditObjectsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	long m_nPersonID;
	BOOL m_bIsNewPatient;

	//(e.lally 2009-09-22) PLID 15116
	BOOL CommitNexWebTransaction();

	// Generated message map functions
	//{{AFX_MSG(CNexWebEditObjectsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectTabNexWebTab(short newTab, short oldTab);
	afx_msg void OnNexwebImport();
	afx_msg void OnNexwebDelete();
	afx_msg void OnNexwebPreview();
	afx_msg void OnNexwebCancel();
	//}}AFX_MSG
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBEDITOBJECTSDLG_H__D92B8AE2_B2EC_4A8A_80B8_9DA3F9A72BFA__INCLUDED_)
