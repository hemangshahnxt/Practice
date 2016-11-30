#pragma once
#include "ReschedulingUtils.h"
#include <vector>

// (r.goldschmidt 2014-12-23 18:14) - PLID 64383 - Created
// CReschedulingCancelAppt dialog

class CReschedulingCancelAppt : public CNxDialog
{
	DECLARE_DYNAMIC(CReschedulingCancelAppt)

public:
	CReschedulingCancelAppt(CWnd* pParent = NULL);   // standard constructor
	virtual ~CReschedulingCancelAppt();

	void SetAppointmentListFromClause(const Nx::Scheduler::RescheduleAppointmentsInfo& info);

	const std::vector<long>& GetAppointmentIDsNotCancelled()
	{
		return m_vecAppointmentIDsNotCancelled;
	}

// Dialog Data
	enum { IDD = IDD_RESCHEDULING_CANCEL_APPT };

	void ShowAll()
	{
		m_bShowAll = true;
	}

protected:

	bool m_bShowAll = false;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxColor m_background;

	NXDATALIST2Lib::_DNxDataListPtr m_pListAppointments;

	NxButton m_radioShowAllAppts;
	NxButton m_radioShowApptsWithOtherResources;

	CString m_strAppointmentListFromClause;
	std::vector<long> m_vecAppointmentIDsNotCancelled;

	virtual BOOL OnInitDialog();
	void SecureControls();
	void ToggleRows();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedUncheckall();
	afx_msg void OnBnClickedCheckall();
	afx_msg void OnBnClickedOk();
	afx_msg void OnCancel();
	afx_msg void OnBnClickedRadioShowApptsWithMultipleResources();
	afx_msg void OnBnClickedRadioShowAllAppts();
};
