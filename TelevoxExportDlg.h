#if !defined(AFX_TELEVOXEXPORTDLG_H__7DC08300_7CEE_4995_AB1B_4A9592211217__INCLUDED_)
#define AFX_TELEVOXEXPORTDLG_H__7DC08300_7CEE_4995_AB1B_4A9592211217__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TelevoxExportDlg.h : header file
//

#include "PracticeRc.h"
/////////////////////////////////////////////////////////////////////////////
// CTelevoxExportDlg dialog

// (b.savon 2014-08-28 11:28) - PLID 62790 - Add three new option set "Sent Reminder" to TeleVox Export
enum ReminderSentStatus{
	rssNothing = 0,
	rssAllPatients,
	rssNoFutureAppts,
};

class CTelevoxExportDlg : public CNxDialog
{
// Construction
public:
	CTelevoxExportDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTelevoxExportDlg)
	enum { IDD = IDD_TELEVOX_EXPORT };
	NxButton	m_btnLocation;
	NxButton	m_btnExcludePriv;
	NxButton	m_btnUsePurpose;
	NxButton	m_btnSendEmail;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxIconButton	m_btnConfigureFields;
	NxButton	m_btnFailIfNot10Digits;
	NxButton	m_btnNoCellNoExport;
	NxButton	m_btnNoCellIfNoTextMsg;
	CNxIconButton	m_btnSelectType;
	CNxIconButton	m_btnUnselectType;
	CNxIconButton	m_btnSelectResource;
	CNxIconButton	m_btnUnselectResource;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTelevoxExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pLocations;
	NXDATALISTLib::_DNxDataListPtr m_pUnselTypes;
	NXDATALISTLib::_DNxDataListPtr m_pSelTypes;
	NXDATALISTLib::_DNxDataListPtr m_pUnselRes;
	NXDATALISTLib::_DNxDataListPtr m_pSelRes;

	// (b.savon 2014-08-28 11:29) - PLID 62790 - Holds the current status
	ReminderSentStatus m_rssPatientReminder;
	void CreatePatientReminder(CString sql); //Creates the reminders
	void CommitPatientReminders(const CSqlFragment &sql);

	// Generated message map functions
	//{{AFX_MSG(CTelevoxExportDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCheckLocation();
	afx_msg void OnSelectType();
	afx_msg void OnUnselectType();
	afx_msg void OnSelectResource();
	afx_msg void OnUnselectResource();
	afx_msg void OnGeneratePatients();
	afx_msg void OnSelChangingLocationFilterList(long FAR* nNewSel);
	afx_msg void OnDblClickCellUnselType(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselResource(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelType(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelResource(long nRowIndex, short nColIndex);
	afx_msg void OnConfigureTelevoxFields();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadioDoNothing();
	afx_msg void OnBnClickedRadioAddAllPatients();
	afx_msg void OnBnClickedRadioAddNoAppts();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TELEVOXEXPORTDLG_H__7DC08300_7CEE_4995_AB1B_4A9592211217__INCLUDED_)
