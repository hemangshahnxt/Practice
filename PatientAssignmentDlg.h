#pragma once


class CPatientAssignmentDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientAssignmentDlg)

public:

	//Add purposes here as uses for the dialog grow
	typedef enum {
		apOnlineVisit = 0,
	} AssignmentPurpose;

	CPatientAssignmentDlg(AssignmentPurpose apPurpose, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPatientAssignmentDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PATIENT_ASSIGNMENT_DLG };
#endif

protected:
	long m_nAssignedPatientID;
	NXDATALIST2Lib::_DNxDataListPtr m_pPatientList;
	CNxIconButton m_btnAssign, m_btnCancel, m_btnCreateNew;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	afx_msg void OnBnClickedAssignPatient();
	afx_msg void OnBnClickedCreatePatient();
	void SelSetPatientLinkList(LPDISPATCH lpSel);

public:
	AssignmentPurpose m_apPurpose;
	CString m_strFirst;
	CString m_strLast;
	CString m_strGender;
	CString m_strEmail;
	CString m_strAddress1;
	CString m_strAddress2;
	CString m_strCity;
	CString m_strState;
	CString m_strZip;
	CString m_strHomePhone;
	CString m_strCellPhone;
	COleDateTime m_dtBirthdate;

	void Prepopulate(CString strFirst, CString strLast, CString strGender, CString strEmail, CString strAddress1, CString strAddress2,
		CString strCity, CString strState, CString strZip, CString strHomePhone, CString strCellPhone, COleDateTime dtBirthdate);

	long GetAssignedPatientID();

};
