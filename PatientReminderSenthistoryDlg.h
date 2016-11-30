#pragma once

#include "PatientsRc.h"
//(s.dhole 8/28/2014 3:20 PM ) - PLID 62751 Added new Class
// CPatientReminderSenthistoryDlg dialog

class CPatientReminderSenthistoryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientReminderSenthistoryDlg)

public:
	CPatientReminderSenthistoryDlg(CWnd* pParent = NULL, long nPatientI=-1, CString sPatientName="");   // standard constructor
	virtual ~CPatientReminderSenthistoryDlg();

// Dialog Data
	enum { IDD = IDD_REMINDER_SENT_HISTORY_DLG };
protected:
	long m_nPatientId;
	CString  m_sPatientName;
	HICON m_hExtraNotesIcon;
	NXDATALIST2Lib::_DNxDataListPtr m_pReminderHistoryList;
	CNxIconButton  m_btn_Close;
	CNxIconButton  m_btn_New;
	CNxColor	m_bkg;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnInitDialog();
	void LoadData();
	DECLARE_MESSAGE_MAP()
	void OnDeleteRecord();
	void AddRowToList(ADODB::_RecordsetPtr rsPtr);
	
public:
	DECLARE_EVENTSINK_MAP()
	void RButtonUpReminderSentHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LButtonDownReminderSentHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedBtnRemindersent();
	afx_msg void OnMenuAction(UINT nID);
	afx_msg void OnBnClickedBtnCloseReminderHistory();
};
