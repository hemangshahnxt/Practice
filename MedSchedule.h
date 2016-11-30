#if !defined(AFX_MEDSCHEDULE_H__CA0A4141_BA2A_47D5_A509_80CD98940FF9__INCLUDED_)
#define AFX_MEDSCHEDULE_H__CA0A4141_BA2A_47D5_A509_80CD98940FF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MedSchedule.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

#include "ProcInfoCenterDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CMedSchedule dialog

class CMedSchedule : public CNxDialog
{
// Construction
public:
	BOOL m_bPrintOnClose;
	void SaveTemplate(CString strName);
	BOOL Save(BOOL bTestName = TRUE);
	void Load();
	void DeleteSchedule(long ID);
	CMedSchedule(CWnd* pParent, BOOL bIsPreOpSched = FALSE);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_DetailList,
					m_ProcedureList,
					m_TemplateList;

	long m_MedSchedID;

	BOOL m_bIsNew;
	// (d.singleton 2012-04-05 10:40) - PLID 50436 Modify Medication Schedule feature to work for preoperative surgical calendar
	BOOL m_bIsPreOpSchedule;
	long m_nProcedureID;

	// (d.singleton 2012-04-05 15:04) - PLID 50436 array to load the appts into
	CArray<AppointmentInfo, AppointmentInfo> m_arAppointmentInfo;

// Dialog Data
	//{{AFX_DATA(CMedSchedule)
	enum { IDD = IDD_MEDSCHEDULE_DLG };
	NxButton	m_btnApplyStartDate;
	CNxIconButton	m_btnMoveDetailDown;
	CNxIconButton	m_btnMoveDetailUp;
	CDateTimePicker	m_dtApplyDate;
	CNxEdit	m_nxeditMedschedName;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnDeleteSched;
	CNxIconButton	m_btnPrintMedSched;
	CNxIconButton	m_btnAddSchedDetail;
	CNxIconButton	m_btnEditSchedDetail;
	CNxIconButton	m_btnRemoveSchedDetail;
	CNxIconButton	m_btnSaveToTemplate;
	CNxIconButton	m_btnSaveNewTemplate;
	CNxIconButton	m_btnDeleteTemplate;
	NxButton	m_btnTemplatesGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMedSchedule)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMedSchedule)
	void CalculateApptDays(COleDateTime dt);
	virtual BOOL OnInitDialog();
	afx_msg void OnAddSchedDetail();
	afx_msg void OnEditSchedDetail();
	afx_msg void OnRemoveSchedDetail();
	afx_msg void OnApplyOnDate();
	afx_msg void OnMoveDetailUp();
	afx_msg void OnMoveDetailDown();
	afx_msg void OnDblClickCellMedscheddetailList(long nRowIndex, short nColIndex);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnDeleteSched();
	afx_msg void OnRequeryFinishedMedscheddetailList(short nFlags);
	afx_msg void OnPrintMedsched();
	afx_msg void OnLoadFromTemplate();
	afx_msg void OnSaveToTemplate();
	afx_msg void OnSaveNewTemplate();
	afx_msg void OnDeleteTemplate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDSCHEDULE_H__CA0A4141_BA2A_47D5_A509_80CD98940FF9__INCLUDED_)
