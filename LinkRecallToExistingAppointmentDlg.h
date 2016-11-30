#pragma once


// CLinkRecallToExistingAppointmentDlg dialog
// (b.savon 2012-03-01 11:56) - PLID 48486 - Created

class CLinkRecallToExistingAppointmentDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLinkRecallToExistingAppointmentDlg)

public:
	// (b.savon 2012-03-20 12:03) - PLID 48471 - Add param for audting and make code more sound
	CLinkRecallToExistingAppointmentDlg(const long &nPatientID, const CString &strPatientName, const long &nRecallID, const CString &strTemplateName, CWnd* pParent = NULL);   // standard constructor
	virtual ~CLinkRecallToExistingAppointmentDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_RECALL_LINK_EXISTING_APPOINTMENT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CNxIconButton m_btnCreate;
	CNxIconButton m_btnCancel;

	CNxColor m_cBackground;

	CNxStatic m_lblDialogHeading;

	NXDATALIST2Lib::_DNxDataListPtr m_pnxdlExistingAppointments;
	enum ExistingAppointmentsColumn{
		eacID = 0,
		eacDate,
		eacTime,
		eacStatus,
		eacPurpose,
		eacResource,
		eacNotes,
	};

	long m_nPatientID;
	long m_nRecallID;
	
	CString m_strPatientName;
	CString m_strTemplateName;

	NXDATALIST2Lib::IRowSettingsPtr m_pSelectedRow;

	void UpdateButtons();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelSetNxdlPtExistingAppts(LPDISPATCH lpSel);
	afx_msg void OnBnClickedCheckShowPrevious();
};
