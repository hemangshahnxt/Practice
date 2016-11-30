#pragma once

#include "ReschedulingUtils.h"

// CRescheduleAppointmentsDlg dialog

// (a.walling 2015-01-05 16:24) - PLID 64373 - Reschedule appointments dialog

class CRescheduleAppointmentsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CRescheduleAppointmentsDlg)

public:
	CRescheduleAppointmentsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRescheduleAppointmentsDlg();

// Dialog Data
	enum { IDD = IDD_RESCHEDULE_APPOINTMENTS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXTIMELib::_DNxTimePtr m_pTimeFrom;
	NXTIMELib::_DNxTimePtr m_pTimeTo;

	NXDATALIST2Lib::_DNxDataListPtr m_pResources;
	NXDATALIST2Lib::_DNxDataListPtr m_pReason;
	NXDATALIST2Lib::_DNxDataListPtr m_pTemplate;
	
	// (a.walling 2015-01-06 14:37) - PLID 64375 - Start and End date
	CDateTimeCtrl m_dtpFrom;
	CDateTimeCtrl m_dtpTo;

	Nx::Scheduler::RescheduleAppointmentsInfo GenerateInfo() const;

	Nx::Scheduler::RescheduleAppointmentsInfo ValidateInfo();

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedAllResources();
	afx_msg void OnBnClickedSelectResources();
	afx_msg void OnBnClickedEditCancellationReasons();
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRescheduleAll();

	// (a.walling 2015-01-06 14:37) - PLID 64375 - Start and End date
	afx_msg void OnDtnDatetimechangeDateStart(NMHDR *pNMHDR, LRESULT *pResult);
};

